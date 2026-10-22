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
#تعریف_کن ROWSET_تخصیص_اندازه 1024

/*
** The number of rowset entries per allocation chunk.
*/
#تعریف_کن ROWSET_مدخل_در_هر_CHUNK  \
                       ((ROWSET_تخصیص_اندازه-8)/حجم(بنية ردیف_مجموعه_مدخل))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
بنية ردیف_مجموعه_مدخل {           
  i64 v;                        /* ROWID value for this entry */
  بنية ردیف_مجموعه_مدخل *p_راست;   /* Right subtree (larger entries) or list */
  بنية ردیف_مجموعه_مدخل *p_چپ;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
بنية ردیف_مجموعه_Chunk {
  بنية ردیف_مجموعه_Chunk *p_بعدی_Chunk;        /* Next chunk on list of them all */
  بنية ردیف_مجموعه_مدخل a_مدخل[ROWSET_مدخل_در_هر_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
بنية ردیف_مجموعه {
  بنية ردیف_مجموعه_Chunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  بنية ردیف_مجموعه_مدخل *p_مدخل;    /* List of entries using pRight */
  بنية ردیف_مجموعه_مدخل *p_آخرین;     /* Last entry on the pEntry list */
  بنية ردیف_مجموعه_مدخل *pFresh;    /* Source of new entry objects */
  بنية ردیف_مجموعه_مدخل *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  صحيح iBatch;                    /* Current insert batch */
};

/*
** Allowed values for RowSet.rsFlags
*/
#تعریف_کن ROWSET_SORTED  0x01   /* True if RowSet.pEntry is sorted */
#تعریف_کن ROWSET_بعدی    0x02   /* True if sqlite3RowSetNext() has been called */

/*
** Allocate a RowSet object.  Return NULL if a memory allocation
** error occurs.
*/
ردیف_مجموعه *sqlite3_ردیف_مجموعه_Init(sqlite3 *db){
  ردیف_مجموعه *p = sqlite3DbMallocRawNN(db, حجم(*p));
  إذا( p ){
    صحيح N = sqlite3_Db_Malloc_اندازه(db, p);
    p->pChunk = 0;
    p->db = db;
    p->p_مدخل = 0;
    p->p_آخرین = 0;
    p->pForest = 0;
    p->pFresh = (بنية ردیف_مجموعه_مدخل*)(ROUND8(حجم(*p)) + (حرف*)p);
    p->nFresh = (u16)((N - ROUND8(حجم(*p)))/حجم(بنية ردیف_مجموعه_مدخل));
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
فراغ sqlite3_ردیف_مجموعه_Clear(فراغ *pArg){
  ردیف_مجموعه *p = (ردیف_مجموعه*)pArg;
  بنية ردیف_مجموعه_Chunk *pChunk, *p_بعدی_Chunk;
  لكل(pChunk=p->pChunk; pChunk; pChunk = p_بعدی_Chunk){
    p_بعدی_Chunk = pChunk->p_بعدی_Chunk;
    sqlite3DbFree(p->db, pChunk);
  }
  p->pChunk = 0;
  p->nFresh = 0;
  p->p_مدخل = 0;
  p->p_آخرین = 0;
  p->pForest = 0;
  p->rsFlags = ROWSET_SORTED;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
فراغ sqlite3_ردیف_مجموعه_Delete(فراغ *pArg){
  sqlite3_ردیف_مجموعه_Clear(pArg);
  sqlite3DbFree(((ردیف_مجموعه*)pArg)->db, pArg);
}

/*
** Allocate a new RowSetEntry object that is associated with the
** given RowSet.  Return a pointer to the new and completely uninitialized
** object.
**
** In an OOM situation, the RowSet.db->mallocFailed flag is set and this
** routine returns NULL.
*/
ساكن بنية ردیف_مجموعه_مدخل *ردیف_مجموعه_مدخل_Alloc(ردیف_مجموعه *p){
  أكد( p!=0 );
  إذا( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    بنية ردیف_مجموعه_Chunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, حجم(*pNew));
    إذا( pNew==0 ){
      أرجع 0;
    }
    pNew->p_بعدی_Chunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->a_مدخل;
    p->nFresh = ROWSET_مدخل_در_هر_CHUNK;
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
فراغ sqlite3_ردیف_مجموعه_درج(ردیف_مجموعه *p, i64 rowid){
  بنية ردیف_مجموعه_مدخل *p_مدخل;  /* The new entry */
  بنية ردیف_مجموعه_مدخل *p_آخرین;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  أكد( p!=0 && (p->rsFlags & ROWSET_بعدی)==0 );

  p_مدخل = ردیف_مجموعه_مدخل_Alloc(p);
  إذا( p_مدخل==0 ) أرجع;
  p_مدخل->v = rowid;
  p_مدخل->p_راست = 0;
  p_آخرین = p->p_آخرین;
  إذا( p_آخرین ){
    إذا( rowid<=p_آخرین->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    p_آخرین->p_راست = p_مدخل;
  }وإلا{
    p->p_مدخل = p_مدخل;
  }
  p->p_آخرین = p_مدخل;
}

/*
** Merge two lists of RowSetEntry objects.  Remove duplicates.
**
** The input lists are connected via pRight pointers and are
** assumed to each already be in sorted order.
*/
ساكن بنية ردیف_مجموعه_مدخل *ردیف_مجموعه_مدخل_ادغام(
  بنية ردیف_مجموعه_مدخل *pA,    /* First sorted list to be merged */
  بنية ردیف_مجموعه_مدخل *pB     /* Second sorted list to be merged */
){
  بنية ردیف_مجموعه_مدخل سر;
  بنية ردیف_مجموعه_مدخل *p_دم;

  p_دم = &سر;
  أكد( pA!=0 && pB!=0 );
  لكل(;;){
    أكد( pA->p_راست==0 || pA->v<=pA->p_راست->v );
    أكد( pB->p_راست==0 || pB->v<=pB->p_راست->v );
    إذا( pA->v<=pB->v ){
      إذا( pA->v<pB->v ) p_دم = p_دم->p_راست = pA;
      pA = pA->p_راست;
      إذا( pA==0 ){
        p_دم->p_راست = pB;
        اكسر;
      }
    }وإلا{
      p_دم = p_دم->p_راست = pB;
      pB = pB->p_راست;
      إذا( pB==0 ){
        p_دم->p_راست = pA;
        اكسر;
      }
    }
  }
  أرجع سر.p_راست;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
ساكن بنية ردیف_مجموعه_مدخل *ردیف_مجموعه_مدخل_مرتب_سازی(بنية ردیف_مجموعه_مدخل *p_درون){
  بیعلامت صحيح i;
  بنية ردیف_مجموعه_مدخل *p_بعدی, *a_سطل[40];

  memset(a_سطل, 0, حجم(a_سطل));
  طالما( p_درون ){
    p_بعدی = p_درون->p_راست;
    p_درون->p_راست = 0;
    لكل(i=0; a_سطل[i]; i++){
      p_درون = ردیف_مجموعه_مدخل_ادغام(a_سطل[i], p_درون);
      a_سطل[i] = 0;
    }
    a_سطل[i] = p_درون;
    p_درون = p_بعدی;
  }
  p_درون = a_سطل[0];
  لكل(i=1; i<حجم(a_سطل)/حجم(a_سطل[0]); i++){
    إذا( a_سطل[i]==0 ) استمر;
    p_درون = p_درون ? ردیف_مجموعه_مدخل_ادغام(p_درون, a_سطل[i]) : a_سطل[i];
  }
  أرجع p_درون;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
ساكن فراغ ردیف_مجموعه_درخت_مقصد_List(
  بنية ردیف_مجموعه_مدخل *p_درون,         /* Root of the input tree */
  بنية ردیف_مجموعه_مدخل **pp_اولی,    /* Write head of the output list here */
  بنية ردیف_مجموعه_مدخل **pp_آخرین      /* Write tail of the output list here */
){
  أكد( p_درون!=0 );
  إذا( p_درون->p_چپ ){
    بنية ردیف_مجموعه_مدخل *p;
    ردیف_مجموعه_درخت_مقصد_List(p_درون->p_چپ, pp_اولی, &p);
    p->p_راست = p_درون;
  }وإلا{
    *pp_اولی = p_درون;
  }
  إذا( p_درون->p_راست ){
    ردیف_مجموعه_درخت_مقصد_List(p_درون->p_راست, &p_درون->p_راست, pp_آخرین);
  }وإلا{
    *pp_آخرین = p_درون;
  }
  أكد( (*pp_آخرین)->p_راست==0 );
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
ساكن بنية ردیف_مجموعه_مدخل *ردیف_مجموعه_NDeep_درخت(
  بنية ردیف_مجموعه_مدخل **ppList,
  صحيح i_عمق
){
  بنية ردیف_مجموعه_مدخل *p;         /* Root of the new tree */
  بنية ردیف_مجموعه_مدخل *p_چپ;     /* Left subtree */
  إذا( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    أرجع 0;
  }
  إذا( i_عمق>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_چپ = ردیف_مجموعه_NDeep_درخت(ppList, i_عمق-1);
    p = *ppList;
    إذا( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      أرجع p_چپ;
    }
    p->p_چپ = p_چپ;
    *ppList = p->p_راست;
    p->p_راست = ردیف_مجموعه_NDeep_درخت(ppList, i_عمق-1);
  }وإلا{
    p = *ppList;
    *ppList = p->p_راست;
    p->p_چپ = p->p_راست = 0;
  }
  أرجع p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
ساكن بنية ردیف_مجموعه_مدخل *ردیف_مجموعه_List_مقصد_درخت(بنية ردیف_مجموعه_مدخل *pList){
  صحيح i_عمق;           /* Depth of the tree so far */
  بنية ردیف_مجموعه_مدخل *p;       /* Current tree root */
  بنية ردیف_مجموعه_مدخل *p_چپ;   /* Left subtree */

  أكد( pList!=0 );
  p = pList;
  pList = p->p_راست;
  p->p_چپ = p->p_راست = 0;
  لكل(i_عمق=1; pList; i_عمق++){
    p_چپ = p;
    p = pList;
    pList = p->p_راست;
    p->p_چپ = p_چپ;
    p->p_راست = ردیف_مجموعه_NDeep_درخت(&pList, i_عمق);
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
صحيح sqlite3_ردیف_مجموعه_بعدی(ردیف_مجموعه *p, i64 *pRowid){
  أكد( p!=0 );
  أكد( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  إذا( (p->rsFlags & ROWSET_بعدی)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    إذا( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->p_مدخل = ردیف_مجموعه_مدخل_مرتب_سازی(p->p_مدخل);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_بعدی;
  }

  /* Return the next entry on the list */
  إذا( p->p_مدخل ){
    *pRowid = p->p_مدخل->v;
    p->p_مدخل = p->p_مدخل->p_راست;
    إذا( p->p_مدخل==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3_ردیف_مجموعه_Clear(p);
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
صحيح sqlite3_ردیف_مجموعه_Test(ردیف_مجموعه *p_ردیف_مجموعه, صحيح iBatch, sqlite3_int64 iRowid){
  بنية ردیف_مجموعه_مدخل *p, *p_درخت;

  /* This routine is never called after sqlite3RowSetNext() */
  أكد( p_ردیف_مجموعه!=0 && (p_ردیف_مجموعه->rsFlags & ROWSET_بعدی)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  إذا( iBatch!=p_ردیف_مجموعه->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = p_ردیف_مجموعه->p_مدخل;
    إذا( p ){
      بنية ردیف_مجموعه_مدخل **pp_Prev_درخت = &p_ردیف_مجموعه->pForest;
      إذا( (p_ردیف_مجموعه->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = ردیف_مجموعه_مدخل_مرتب_سازی(p);
      }
      لكل(p_درخت = p_ردیف_مجموعه->pForest; p_درخت; p_درخت=p_درخت->p_راست){
        pp_Prev_درخت = &p_درخت->p_راست;
        إذا( p_درخت->p_چپ==0 ){
          p_درخت->p_چپ = ردیف_مجموعه_List_مقصد_درخت(p);
          اكسر;
        }وإلا{
          بنية ردیف_مجموعه_مدخل *pAux, *p_دم;
          ردیف_مجموعه_درخت_مقصد_List(p_درخت->p_چپ, &pAux, &p_دم);
          p_درخت->p_چپ = 0;
          p = ردیف_مجموعه_مدخل_ادغام(pAux, p);
        }
      }
      إذا( p_درخت==0 ){
        *pp_Prev_درخت = p_درخت = ردیف_مجموعه_مدخل_Alloc(p_ردیف_مجموعه);
        إذا( p_درخت ){
          p_درخت->v = 0;
          p_درخت->p_راست = 0;
          p_درخت->p_چپ = ردیف_مجموعه_List_مقصد_درخت(p);
        }
      }
      p_ردیف_مجموعه->p_مدخل = 0;
      p_ردیف_مجموعه->p_آخرین = 0;
      p_ردیف_مجموعه->rsFlags |= ROWSET_SORTED;
    }
    p_ردیف_مجموعه->iBatch = iBatch;
  }

  /* Test to see if the iRowid value appears anywhere in the forest.
  ** Return 1 if it does and 0 if not.
  */
  لكل(p_درخت = p_ردیف_مجموعه->pForest; p_درخت; p_درخت=p_درخت->p_راست){
    p = p_درخت->p_چپ;
    طالما( p ){
      إذا( p->v<iRowid ){
        p = p->p_راست;
      }وإلا إذا( p->v>iRowid ){
        p = p->p_چپ;
      }وإلا{
        أرجع 1;
      }
    }
  }
  أرجع 0;
}
