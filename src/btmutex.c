/*
** 2007 August 27
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
** This file contains code used to implement mutexes on Btree objects.
** This code really belongs in btree.c.  But btree.c is getting too
** big and we want to break it down some.  This packaged seemed like
** a good breakout.
*/
#ಅನ್ತರ್ಭಾವ "btreeInt.h"
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
#ಆದರೆ SQLITE_THREADSAFE

/*
** Obtain the BtShared mutex associated with B-Tree handle p. Also,
** set BtShared.db to the database handle associated with p and the
** p->locked boolean to true.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಬೀಗ_Btree_Mutex(Btree *p){
  ಪ್ರತಿಜ್ಞಾ( p->locked==0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_notheld(p->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->db->mutex) );

  sqlite3_mutex_enter(p->pBt->mutex);
  p->pBt->db = p->db;
  p->locked = 1;
}

/*
** Release the BtShared mutex associated with B-Tree handle p and
** clear the p->locked boolean.
*/
ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_NOINLINE unlockBtreeMutex(Btree *p){
  BtShared *pBt = p->pBt;
  ಪ್ರತಿಜ್ಞಾ( p->locked==1 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( p->db==pBt->db );

  sqlite3_mutex_leave(pBt->mutex);
  p->locked = 0;
}

/* Forward reference */
ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_NOINLINE btree_ಬೀಗ_Carefully(Btree *p);

/*
** Enter a mutex on the given BTree object.
**
** If the object is not sharable, then no mutex is ever required
** and this routine is a no-op.  The underlying mutex is non-recursive.
** But we keep a reference count in Btree.wantToLock so the behavior
** of this interface is recursive.
**
** To avoid deadlocks, multiple Btrees are locked in the same order
** by all database connections.  The p->pNext is a list of other
** Btrees belonging to the same database connection as the p Btree
** which need to be locked after p.  If we cannot get a lock on
** p, then first unlock all of the others on p->pNext, then wait
** for the lock to become available on p, then relock all of the
** subsequent Btrees that desire a lock.
*/
ಶೂನ್ಯ sqlite3BtreeEnter(Btree *p){
  /* Some basic sanity checking on the Btree.  The list of Btrees
  ** connected by pNext and pPrev should be in sorted order by
  ** Btree.pBt value. All elements of the list should belong to
  ** the same connection. Only shared Btrees are on the list. */
  ಪ್ರತಿಜ್ಞಾ( p->p_ಮುಂದಿನ==0 || p->p_ಮುಂದಿನ->pBt>p->pBt );
  ಪ್ರತಿಜ್ಞಾ( p->pPrev==0 || p->pPrev->pBt<p->pBt );
  ಪ್ರತಿಜ್ಞಾ( p->p_ಮುಂದಿನ==0 || p->p_ಮುಂದಿನ->db==p->db );
  ಪ್ರತಿಜ್ಞಾ( p->pPrev==0 || p->pPrev->db==p->db );
  ಪ್ರತಿಜ್ಞಾ( p->sharable || (p->p_ಮುಂದಿನ==0 && p->pPrev==0) );

  /* Check for locking consistency */
  ಪ್ರತಿಜ್ಞಾ( !p->locked || p->want_ಗಮ್ಯ_ಬೀಗ>0 );
  ಪ್ರತಿಜ್ಞಾ( p->sharable || p->want_ಗಮ್ಯ_ಬೀಗ==0 );

  /* We should already hold a lock on the database connection */
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->db->mutex) );

  /* Unless the database is sharable and unlocked, then BtShared.db
  ** should already be set correctly. */
  ಪ್ರತಿಜ್ಞಾ( (p->locked==0 && p->sharable) || p->pBt->db==p->db );

  ಆದರೆ( !p->sharable ) ಹಿಂತಿರುಗಿ;
  p->want_ಗಮ್ಯ_ಬೀಗ++;
  ಆದರೆ( p->locked ) ಹಿಂತಿರುಗಿ;
  btree_ಬೀಗ_Carefully(p);
}

/* This is a helper function for sqlite3BtreeLock(). By moving
** complex, but seldom used logic, out of sqlite3BtreeLock() and
** into this routine, we avoid unnecessary stack pointer changes
** and thus help the sqlite3BtreeLock() routine to run much faster
** in the common case.
*/
ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_NOINLINE btree_ಬೀಗ_Carefully(Btree *p){
  Btree *pLater;

  /* In most cases, we should be able to acquire the lock we
  ** want without having to go through the ascending lock
  ** procedure that follows.  Just be sure not to block.
  */
  ಆದರೆ( sqlite3_mutex_try(p->pBt->mutex)==SQLITE_OK ){
    p->pBt->db = p->db;
    p->locked = 1;
    ಹಿಂತಿರುಗಿ;
  }

  /* To avoid deadlock, first release all locks with a larger
  ** BtShared address.  Then acquire our lock.  Then reacquire
  ** the other BtShared locks that we used to hold in ascending
  ** order.
  */
  ಗಾಗಿ(pLater=p->p_ಮುಂದಿನ; pLater; pLater=pLater->p_ಮುಂದಿನ){
    ಪ್ರತಿಜ್ಞಾ( pLater->sharable );
    ಪ್ರತಿಜ್ಞಾ( pLater->p_ಮುಂದಿನ==0 || pLater->p_ಮುಂದಿನ->pBt>pLater->pBt );
    ಪ್ರತಿಜ್ಞಾ( !pLater->locked || pLater->want_ಗಮ್ಯ_ಬೀಗ>0 );
    ಆದರೆ( pLater->locked ){
      unlockBtreeMutex(pLater);
    }
  }
  ಬೀಗ_Btree_Mutex(p);
  ಗಾಗಿ(pLater=p->p_ಮುಂದಿನ; pLater; pLater=pLater->p_ಮುಂದಿನ){
    ಆದರೆ( pLater->want_ಗಮ್ಯ_ಬೀಗ ){
      ಬೀಗ_Btree_Mutex(pLater);
    }
  }
}


/*
** Exit the recursive mutex on a Btree.
*/
ಶೂನ್ಯ sqlite3BtreeLeave(Btree *p){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->db->mutex) );
  ಆದರೆ( p->sharable ){
    ಪ್ರತಿಜ್ಞಾ( p->want_ಗಮ್ಯ_ಬೀಗ>0 );
    p->want_ಗಮ್ಯ_ಬೀಗ--;
    ಆದರೆ( p->want_ಗಮ್ಯ_ಬೀಗ==0 ){
      unlockBtreeMutex(p);
    }
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
/*
** Return true if the BtShared mutex is held on the btree, or if the
** B-Tree is not marked as sharable.
**
** This routine is used only from within assert() statements.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeHoldsMutex(Btree *p){
  ಪ್ರತಿಜ್ಞಾ( p->sharable==0 || p->locked==0 || p->want_ಗಮ್ಯ_ಬೀಗ>0 );
  ಪ್ರತಿಜ್ಞಾ( p->sharable==0 || p->locked==0 || p->db==p->pBt->db );
  ಪ್ರತಿಜ್ಞಾ( p->sharable==0 || p->locked==0 || sqlite3_mutex_held(p->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( p->sharable==0 || p->locked==0 || sqlite3_mutex_held(p->db->mutex) );

  ಹಿಂತಿರುಗಿ (p->sharable==0 || p->locked);
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** Enter the mutex on every Btree associated with a database
** connection.  This is needed (for example) prior to parsing
** a statement since we will be comparing table and column names
** against all schemas and we do not want those schemas being
** reset out from under us.
**
** There is a corresponding leave-all procedures.
**
** Enter the mutexes in ascending order by BtShared pointer address
** to avoid the possibility of deadlock when two threads with
** two or more btrees in common both try to lock all their btrees
** at the same instant.
*/
ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_NOINLINE btreeEnterAll(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ i;
  u8 skipOk = 1;
  Btree *p;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಗಾಗಿ(i=0; i<db->nDb; i++){
    p = db->aDb[i].pBt;
    ಆದರೆ( p && p->sharable ){
      sqlite3BtreeEnter(p);
      skipOk = 0;
    }
  }
  db->noSharedCache = skipOk;
}
ಶೂನ್ಯ sqlite3BtreeEnterAll(sqlite3 *db){
  ಆದರೆ( db->noSharedCache==0 ) btreeEnterAll(db);
}
ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_NOINLINE btreeLeaveAll(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ i;
  Btree *p;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಗಾಗಿ(i=0; i<db->nDb; i++){
    p = db->aDb[i].pBt;
    ಆದರೆ( p ) sqlite3BtreeLeave(p);
  }
}
ಶೂನ್ಯ sqlite3BtreeLeaveAll(sqlite3 *db){
  ಆದರೆ( db->noSharedCache==0 ) btreeLeaveAll(db);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
/*
** Return true if the current thread holds the database connection
** mutex and all required BtShared mutexes.
**
** This routine is used inside assert() statements only.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeHoldsAllMutexes(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( !sqlite3_mutex_held(db->mutex) ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಗಾಗಿ(i=0; i<db->nDb; i++){
    Btree *p;
    p = db->aDb[i].pBt;
    ಆದರೆ( p && p->sharable &&
         (p->want_ಗಮ್ಯ_ಬೀಗ==0 || !sqlite3_mutex_held(p->pBt->mutex)) ){
      ಹಿಂತಿರುಗಿ 0;
    }
  }
  ಹಿಂತಿರುಗಿ 1;
}
#ಆದರೆ_ಅಂತ್ಯ /* NDEBUG */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
/*
** Return true if the correct mutexes are held for accessing the
** db->aDb[iDb].pSchema structure.  The mutexes required for schema
** access are:
**
**   (1) The mutex on db
**   (2) if iDb!=1, then the mutex on db->aDb[iDb].pBt.
**
** If pSchema is not NULL, then iDb is computed from pSchema and
** db using sqlite3SchemaToIndex().
*/
ಪೂರ್ಣಾಂಕ sqlite3SchemaMutexHeld(sqlite3 *db, ಪೂರ್ಣಾಂಕ iDb, Schema *pSchema){
  Btree *p;
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಆದರೆ( db->pVfs==0 && db->nDb==0 ) ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ( pSchema ) iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pSchema);
  ಪ್ರತಿಜ್ಞಾ( iDb>=0 && iDb<db->nDb );
  ಆದರೆ( !sqlite3_mutex_held(db->mutex) ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( iDb==1 ) ಹಿಂತಿರುಗಿ 1;
  p = db->aDb[iDb].pBt;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಹಿಂತಿರುಗಿ p->sharable==0 || p->locked==1;
}
#ಆದರೆ_ಅಂತ್ಯ /* NDEBUG */

#ಇಲ್ಲದಿದ್ದರೆ /* SQLITE_THREADSAFE>0 above.  SQLITE_THREADSAFE==0 below */
/*
** The following are special cases for mutex enter routines for use
** in single threaded applications that use shared cache.  Except for
** these two routines, all mutex operations are no-ops in that case and
** are null #defines in btree.h.
**
** If shared cache is disabled, then all btree mutex routines, including
** the ones below, are no-ops and are null #defines in btree.h.
*/

ಶೂನ್ಯ sqlite3BtreeEnter(Btree *p){
  p->pBt->db = p->db;
}
ಶೂನ್ಯ sqlite3BtreeEnterAll(sqlite3 *db){
  ಪೂರ್ಣಾಂಕ i;
  u8 skipOk = 1;
  /* As in the threadsafe build: set db->noSharedCache when no attached
  ** Btree is sharable, so that schema and table-lock checks are skipped */
  ಆದರೆ( db->noSharedCache ) ಹಿಂತಿರುಗಿ;
  ಗಾಗಿ(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    ಆದರೆ( p ){
      p->pBt->db = p->db;
      ಆದರೆ( p->sharable ) skipOk = 0;
    }
  }
  db->noSharedCache = skipOk;
}
#ಆದರೆ_ಅಂತ್ಯ /* if SQLITE_THREADSAFE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB
/*
** Enter a mutex on a Btree given a cursor owned by that Btree. 
**
** These entry points are used by incremental I/O only. Enter() is required 
** any time OMIT_SHARED_CACHE is not defined, regardless of whether or not 
** the build is threadsafe. Leave() is only required by threadsafe builds.
*/
ಶೂನ್ಯ sqlite3BtreeEnterCursor(BtCursor *pCur){
  sqlite3BtreeEnter(pCur->pBtree);
}
# ಆದರೆ SQLITE_THREADSAFE
ಶೂನ್ಯ sqlite3BtreeLeaveCursor(BtCursor *pCur){
  sqlite3BtreeLeave(pCur->pBtree);
}
# endif
#ಆದರೆ_ಅಂತ್ಯ /* ifndef SQLITE_OMIT_INCRBLOB */

#ಆದರೆ_ಅಂತ್ಯ /* ifndef SQLITE_OMIT_SHARED_CACHE */
