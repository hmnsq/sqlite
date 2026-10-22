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
#شامل "sqliteInt.h"


/*
** Target size for allocation chunks.
*/
#تعریف_کرو ROWSET_تخصیص_سائز 1024

/*
** The number of rowset entries per allocation chunk.
*/
#تعریف_کرو ROWSET_اندراج_فی_CHUNK  \
                       ((ROWSET_تخصیص_سائز-8)/حجم(ساخت صف_سیٹ_اندراج))

/*
** Each entry in a RowSet is an instance of the following object.
**
** This same object is reused to store a linked list of trees of RowSetEntry
** objects.  In that alternative use, pRight points to the next entry
** in the list, pLeft points to the tree, and v is unused.  The
** RowSet.pForest value points to the head of this forest list.
*/
ساخت صف_سیٹ_اندراج {           
  i64 v;                        /* ROWID value for this entry */
  ساخت صف_سیٹ_اندراج *p_دایاں;   /* Right subtree (larger entries) or list */
  ساخت صف_سیٹ_اندراج *p_بایاں;    /* Left subtree (smaller entries) */
};

/*
** RowSetEntry objects are allocated in large chunks (instances of the
** following structure) to reduce memory allocation overhead.  The
** chunks are kept on a linked list so that they can be deallocated
** when the RowSet is destroyed.
*/
ساخت صف_سیٹ_Chunk {
  ساخت صف_سیٹ_Chunk *p_اگلا_Chunk;        /* Next chunk on list of them all */
  ساخت صف_سیٹ_اندراج a_اندراج[ROWSET_اندراج_فی_CHUNK]; /* Allocated entries */
};

/*
** A RowSet in an instance of the following structure.
**
** A typedef of this structure if found in sqliteInt.h.
*/
ساخت صف_سیٹ {
  ساخت صف_سیٹ_Chunk *pChunk;    /* List of all chunk allocations */
  sqlite3 *db;                   /* The database connection */
  ساخت صف_سیٹ_اندراج *p_اندراج;    /* List of entries using pRight */
  ساخت صف_سیٹ_اندراج *p_آخری;     /* Last entry on the pEntry list */
  ساخت صف_سیٹ_اندراج *pFresh;    /* Source of new entry objects */
  ساخت صف_سیٹ_اندراج *pForest;   /* List of binary trees of entries */
  u16 nFresh;                    /* Number of objects on pFresh */
  u16 rsFlags;                   /* Various flags */
  صحیح iBatch;                    /* Current insert batch */
};

/*
** Allowed values for RowSet.rsFlags
*/
#تعریف_کرو ROWSET_SORTED  0x01   /* True if RowSet.pEntry is sorted */
#تعریف_کرو ROWSET_اگلا    0x02   /* True if sqlite3RowSetNext() has been called */

/*
** Allocate a RowSet object.  Return NULL if a memory allocation
** error occurs.
*/
صف_سیٹ *sqlite3_صف_سیٹ_Init(sqlite3 *db){
  صف_سیٹ *p = sqlite3DbMallocRawNN(db, حجم(*p));
  اگر( p ){
    صحیح N = sqlite3_Db_Malloc_سائز(db, p);
    p->pChunk = 0;
    p->db = db;
    p->p_اندراج = 0;
    p->p_آخری = 0;
    p->pForest = 0;
    p->pFresh = (ساخت صف_سیٹ_اندراج*)(ROUND8(حجم(*p)) + (حرف*)p);
    p->nFresh = (u16)((N - ROUND8(حجم(*p)))/حجم(ساخت صف_سیٹ_اندراج));
    p->rsFlags = ROWSET_SORTED;
    p->iBatch = 0;
  }
  واپس p;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
خالی sqlite3_صف_سیٹ_Clear(خالی *pArg){
  صف_سیٹ *p = (صف_سیٹ*)pArg;
  ساخت صف_سیٹ_Chunk *pChunk, *p_اگلا_Chunk;
  برائے(pChunk=p->pChunk; pChunk; pChunk = p_اگلا_Chunk){
    p_اگلا_Chunk = pChunk->p_اگلا_Chunk;
    sqlite3DbFree(p->db, pChunk);
  }
  p->pChunk = 0;
  p->nFresh = 0;
  p->p_اندراج = 0;
  p->p_آخری = 0;
  p->pForest = 0;
  p->rsFlags = ROWSET_SORTED;
}

/*
** Deallocate all chunks from a RowSet.  This frees all memory that
** the RowSet has allocated over its lifetime.  This routine is
** the destructor for the RowSet.
*/
خالی sqlite3_صف_سیٹ_Delete(خالی *pArg){
  sqlite3_صف_سیٹ_Clear(pArg);
  sqlite3DbFree(((صف_سیٹ*)pArg)->db, pArg);
}

/*
** Allocate a new RowSetEntry object that is associated with the
** given RowSet.  Return a pointer to the new and completely uninitialized
** object.
**
** In an OOM situation, the RowSet.db->mallocFailed flag is set and this
** routine returns NULL.
*/
ساکن ساخت صف_سیٹ_اندراج *صف_سیٹ_اندراج_Alloc(صف_سیٹ *p){
  أكد( p!=0 );
  اگر( p->nFresh==0 ){  /*OPTIMIZATION-IF-FALSE*/
    /* We could allocate a fresh RowSetEntry each time one is needed, but it
    ** is more efficient to pull a preallocated entry from the pool */
    ساخت صف_سیٹ_Chunk *pNew;
    pNew = sqlite3DbMallocRawNN(p->db, حجم(*pNew));
    اگر( pNew==0 ){
      واپس 0;
    }
    pNew->p_اگلا_Chunk = p->pChunk;
    p->pChunk = pNew;
    p->pFresh = pNew->a_اندراج;
    p->nFresh = ROWSET_اندراج_فی_CHUNK;
  }
  p->nFresh--;
  واپس p->pFresh++;
}

/*
** Insert a new value into a RowSet.
**
** The mallocFailed flag of the database connection is set if a
** memory allocation fails.
*/
خالی sqlite3_صف_سیٹ_داخل_کرو(صف_سیٹ *p, i64 rowid){
  ساخت صف_سیٹ_اندراج *p_اندراج;  /* The new entry */
  ساخت صف_سیٹ_اندراج *p_آخری;   /* The last prior entry */

  /* This routine is never called after sqlite3RowSetNext() */
  أكد( p!=0 && (p->rsFlags & ROWSET_اگلا)==0 );

  p_اندراج = صف_سیٹ_اندراج_Alloc(p);
  اگر( p_اندراج==0 ) واپس;
  p_اندراج->v = rowid;
  p_اندراج->p_دایاں = 0;
  p_آخری = p->p_آخری;
  اگر( p_آخری ){
    اگر( rowid<=p_آخری->v ){  /*OPTIMIZATION-IF-FALSE*/
      /* Avoid unnecessary sorts by preserving the ROWSET_SORTED flags
      ** where possible */
      p->rsFlags &= ~ROWSET_SORTED;
    }
    p_آخری->p_دایاں = p_اندراج;
  }ورنہ{
    p->p_اندراج = p_اندراج;
  }
  p->p_آخری = p_اندراج;
}

/*
** Merge two lists of RowSetEntry objects.  Remove duplicates.
**
** The input lists are connected via pRight pointers and are
** assumed to each already be in sorted order.
*/
ساکن ساخت صف_سیٹ_اندراج *صف_سیٹ_اندراج_ملاؤ(
  ساخت صف_سیٹ_اندراج *pA,    /* First sorted list to be merged */
  ساخت صف_سیٹ_اندراج *pB     /* Second sorted list to be merged */
){
  ساخت صف_سیٹ_اندراج سر;
  ساخت صف_سیٹ_اندراج *p_دم;

  p_دم = &سر;
  أكد( pA!=0 && pB!=0 );
  برائے(;;){
    أكد( pA->p_دایاں==0 || pA->v<=pA->p_دایاں->v );
    أكد( pB->p_دایاں==0 || pB->v<=pB->p_دایاں->v );
    اگر( pA->v<=pB->v ){
      اگر( pA->v<pB->v ) p_دم = p_دم->p_دایاں = pA;
      pA = pA->p_دایاں;
      اگر( pA==0 ){
        p_دم->p_دایاں = pB;
        توڑو;
      }
    }ورنہ{
      p_دم = p_دم->p_دایاں = pB;
      pB = pB->p_دایاں;
      اگر( pB==0 ){
        p_دم->p_دایاں = pA;
        توڑو;
      }
    }
  }
  واپس سر.p_دایاں;
}

/*
** Sort all elements on the list of RowSetEntry objects into order of
** increasing v.
*/
ساکن ساخت صف_سیٹ_اندراج *صف_سیٹ_اندراج_ترتیب_دو(ساخت صف_سیٹ_اندراج *p_اندر){
  غیرعلامتی صحیح i;
  ساخت صف_سیٹ_اندراج *p_اگلا, *a_بالٹی[40];

  memset(a_بالٹی, 0, حجم(a_بالٹی));
  جب( p_اندر ){
    p_اگلا = p_اندر->p_دایاں;
    p_اندر->p_دایاں = 0;
    برائے(i=0; a_بالٹی[i]; i++){
      p_اندر = صف_سیٹ_اندراج_ملاؤ(a_بالٹی[i], p_اندر);
      a_بالٹی[i] = 0;
    }
    a_بالٹی[i] = p_اندر;
    p_اندر = p_اگلا;
  }
  p_اندر = a_بالٹی[0];
  برائے(i=1; i<حجم(a_بالٹی)/حجم(a_بالٹی[0]); i++){
    اگر( a_بالٹی[i]==0 ) جاری;
    p_اندر = p_اندر ? صف_سیٹ_اندراج_ملاؤ(p_اندر, a_بالٹی[i]) : a_بالٹی[i];
  }
  واپس p_اندر;
}


/*
** The input, pIn, is a binary tree (or subtree) of RowSetEntry objects.
** Convert this tree into a linked list connected by the pRight pointers
** and return pointers to the first and last elements of the new list.
*/
ساکن خالی صف_سیٹ_درخت_مقصد_List(
  ساخت صف_سیٹ_اندراج *p_اندر,         /* Root of the input tree */
  ساخت صف_سیٹ_اندراج **pp_پہلا,    /* Write head of the output list here */
  ساخت صف_سیٹ_اندراج **pp_آخری      /* Write tail of the output list here */
){
  أكد( p_اندر!=0 );
  اگر( p_اندر->p_بایاں ){
    ساخت صف_سیٹ_اندراج *p;
    صف_سیٹ_درخت_مقصد_List(p_اندر->p_بایاں, pp_پہلا, &p);
    p->p_دایاں = p_اندر;
  }ورنہ{
    *pp_پہلا = p_اندر;
  }
  اگر( p_اندر->p_دایاں ){
    صف_سیٹ_درخت_مقصد_List(p_اندر->p_دایاں, &p_اندر->p_دایاں, pp_آخری);
  }ورنہ{
    *pp_آخری = p_اندر;
  }
  أكد( (*pp_آخری)->p_دایاں==0 );
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
ساکن ساخت صف_سیٹ_اندراج *صف_سیٹ_NDeep_درخت(
  ساخت صف_سیٹ_اندراج **ppList,
  صحیح i_گہرائی
){
  ساخت صف_سیٹ_اندراج *p;         /* Root of the new tree */
  ساخت صف_سیٹ_اندراج *p_بایاں;     /* Left subtree */
  اگر( *ppList==0 ){ /*OPTIMIZATION-IF-TRUE*/
    /* Prevent unnecessary deep recursion when we run out of entries */
    واپس 0;
  }
  اگر( i_گہرائی>1 ){   /*OPTIMIZATION-IF-TRUE*/
    /* This branch causes a *balanced* tree to be generated.  A valid tree
    ** is still generated without this branch, but the tree is wildly
    ** unbalanced and inefficient. */
    p_بایاں = صف_سیٹ_NDeep_درخت(ppList, i_گہرائی-1);
    p = *ppList;
    اگر( p==0 ){     /*OPTIMIZATION-IF-FALSE*/
      /* It is safe to always return here, but the resulting tree
      ** would be unbalanced */
      واپس p_بایاں;
    }
    p->p_بایاں = p_بایاں;
    *ppList = p->p_دایاں;
    p->p_دایاں = صف_سیٹ_NDeep_درخت(ppList, i_گہرائی-1);
  }ورنہ{
    p = *ppList;
    *ppList = p->p_دایاں;
    p->p_بایاں = p->p_دایاں = 0;
  }
  واپس p;
}

/*
** Convert a sorted list of elements into a binary tree. Make the tree
** as deep as it needs to be in order to contain the entire list.
*/
ساکن ساخت صف_سیٹ_اندراج *صف_سیٹ_List_مقصد_درخت(ساخت صف_سیٹ_اندراج *pList){
  صحیح i_گہرائی;           /* Depth of the tree so far */
  ساخت صف_سیٹ_اندراج *p;       /* Current tree root */
  ساخت صف_سیٹ_اندراج *p_بایاں;   /* Left subtree */

  أكد( pList!=0 );
  p = pList;
  pList = p->p_دایاں;
  p->p_بایاں = p->p_دایاں = 0;
  برائے(i_گہرائی=1; pList; i_گہرائی++){
    p_بایاں = p;
    p = pList;
    pList = p->p_دایاں;
    p->p_بایاں = p_بایاں;
    p->p_دایاں = صف_سیٹ_NDeep_درخت(&pList, i_گہرائی);
  }
  واپس p;
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
صحیح sqlite3_صف_سیٹ_اگلا(صف_سیٹ *p, i64 *pRowid){
  أكد( p!=0 );
  أكد( p->pForest==0 );  /* Cannot be used with sqlite3RowSetText() */

  /* Merge the forest into a single sorted list on first call */
  اگر( (p->rsFlags & ROWSET_اگلا)==0 ){  /*OPTIMIZATION-IF-FALSE*/
    اگر( (p->rsFlags & ROWSET_SORTED)==0 ){  /*OPTIMIZATION-IF-FALSE*/
      p->p_اندراج = صف_سیٹ_اندراج_ترتیب_دو(p->p_اندراج);
    }
    p->rsFlags |= ROWSET_SORTED|ROWSET_اگلا;
  }

  /* Return the next entry on the list */
  اگر( p->p_اندراج ){
    *pRowid = p->p_اندراج->v;
    p->p_اندراج = p->p_اندراج->p_دایاں;
    اگر( p->p_اندراج==0 ){ /*OPTIMIZATION-IF-TRUE*/
      /* Free memory immediately, rather than waiting on sqlite3_finalize() */
      sqlite3_صف_سیٹ_Clear(p);
    }
    واپس 1;
  }ورنہ{
    واپس 0;
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
صحیح sqlite3_صف_سیٹ_Test(صف_سیٹ *p_صف_سیٹ, صحیح iBatch, sqlite3_int64 iRowid){
  ساخت صف_سیٹ_اندراج *p, *p_درخت;

  /* This routine is never called after sqlite3RowSetNext() */
  أكد( p_صف_سیٹ!=0 && (p_صف_سیٹ->rsFlags & ROWSET_اگلا)==0 );

  /* Sort entries into the forest on the first test of a new batch.
  ** To save unnecessary work, only do this when the batch number changes.
  */
  اگر( iBatch!=p_صف_سیٹ->iBatch ){  /*OPTIMIZATION-IF-FALSE*/
    p = p_صف_سیٹ->p_اندراج;
    اگر( p ){
      ساخت صف_سیٹ_اندراج **pp_پچھلا_درخت = &p_صف_سیٹ->pForest;
      اگر( (p_صف_سیٹ->rsFlags & ROWSET_SORTED)==0 ){ /*OPTIMIZATION-IF-FALSE*/
        /* Only sort the current set of entries if they need it */
        p = صف_سیٹ_اندراج_ترتیب_دو(p);
      }
      برائے(p_درخت = p_صف_سیٹ->pForest; p_درخت; p_درخت=p_درخت->p_دایاں){
        pp_پچھلا_درخت = &p_درخت->p_دایاں;
        اگر( p_درخت->p_بایاں==0 ){
          p_درخت->p_بایاں = صف_سیٹ_List_مقصد_درخت(p);
          توڑو;
        }ورنہ{
          ساخت صف_سیٹ_اندراج *pAux, *p_دم;
          صف_سیٹ_درخت_مقصد_List(p_درخت->p_بایاں, &pAux, &p_دم);
          p_درخت->p_بایاں = 0;
          p = صف_سیٹ_اندراج_ملاؤ(pAux, p);
        }
      }
      اگر( p_درخت==0 ){
        *pp_پچھلا_درخت = p_درخت = صف_سیٹ_اندراج_Alloc(p_صف_سیٹ);
        اگر( p_درخت ){
          p_درخت->v = 0;
          p_درخت->p_دایاں = 0;
          p_درخت->p_بایاں = صف_سیٹ_List_مقصد_درخت(p);
        }
      }
      p_صف_سیٹ->p_اندراج = 0;
      p_صف_سیٹ->p_آخری = 0;
      p_صف_سیٹ->rsFlags |= ROWSET_SORTED;
    }
    p_صف_سیٹ->iBatch = iBatch;
  }

  /* Test to see if the iRowid value appears anywhere in the forest.
  ** Return 1 if it does and 0 if not.
  */
  برائے(p_درخت = p_صف_سیٹ->pForest; p_درخت; p_درخت=p_درخت->p_دایاں){
    p = p_درخت->p_بایاں;
    جب( p ){
      اگر( p->v<iRowid ){
        p = p->p_دایاں;
      }ورنہ اگر( p->v>iRowid ){
        p = p->p_بایاں;
      }ورنہ{
        واپس 1;
      }
    }
  }
  واپس 0;
}
