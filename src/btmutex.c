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
#ਅਨ੍ਤਰ੍ਭਾਵ "btreeInt.h"
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
#ਜੇ SQLITE_THREADSAFE

/*
** Obtain the BtShared mutex associated with B-Tree handle p. Also,
** set BtShared.db to the database handle associated with p and the
** p->locked boolean to true.
*/
ਸਥਿਰ ਖਾਲੀ ਤਾਲਾ_Btree_Mutex(Btree *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->locked==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_notheld(p->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->db->mutex) );

  sqlite3_mutex_enter(p->pBt->mutex);
  p->pBt->db = p->db;
  p->locked = 1;
}

/*
** Release the BtShared mutex associated with B-Tree handle p and
** clear the p->locked boolean.
*/
ਸਥਿਰ ਖਾਲੀ SQLITE_NOINLINE unlockBtreeMutex(Btree *p){
  BtShared *pBt = p->pBt;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->locked==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->db==pBt->db );

  sqlite3_mutex_leave(pBt->mutex);
  p->locked = 0;
}

/* Forward reference */
ਸਥਿਰ ਖਾਲੀ SQLITE_NOINLINE btree_ਤਾਲਾ_Carefully(Btree *p);

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
ਖਾਲੀ sqlite3BtreeEnter(Btree *p){
  /* Some basic sanity checking on the Btree.  The list of Btrees
  ** connected by pNext and pPrev should be in sorted order by
  ** Btree.pBt value. All elements of the list should belong to
  ** the same connection. Only shared Btrees are on the list. */
  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਅਗਲਾ==0 || p->p_ਅਗਲਾ->pBt>p->pBt );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pPrev==0 || p->pPrev->pBt<p->pBt );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਅਗਲਾ==0 || p->p_ਅਗਲਾ->db==p->db );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pPrev==0 || p->pPrev->db==p->db );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->sharable || (p->p_ਅਗਲਾ==0 && p->pPrev==0) );

  /* Check for locking consistency */
  ਪ੍ਰਤਿਜ੍ਞਾ( !p->locked || p->want_ਮੰਜ਼ਿਲ_ਤਾਲਾ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->sharable || p->want_ਮੰਜ਼ਿਲ_ਤਾਲਾ==0 );

  /* We should already hold a lock on the database connection */
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->db->mutex) );

  /* Unless the database is sharable and unlocked, then BtShared.db
  ** should already be set correctly. */
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->locked==0 && p->sharable) || p->pBt->db==p->db );

  ਜੇ( !p->sharable ) ਵਾਪਸ;
  p->want_ਮੰਜ਼ਿਲ_ਤਾਲਾ++;
  ਜੇ( p->locked ) ਵਾਪਸ;
  btree_ਤਾਲਾ_Carefully(p);
}

/* This is a helper function for sqlite3BtreeLock(). By moving
** complex, but seldom used logic, out of sqlite3BtreeLock() and
** into this routine, we avoid unnecessary stack pointer changes
** and thus help the sqlite3BtreeLock() routine to run much faster
** in the common case.
*/
ਸਥਿਰ ਖਾਲੀ SQLITE_NOINLINE btree_ਤਾਲਾ_Carefully(Btree *p){
  Btree *pLater;

  /* In most cases, we should be able to acquire the lock we
  ** want without having to go through the ascending lock
  ** procedure that follows.  Just be sure not to block.
  */
  ਜੇ( sqlite3_mutex_try(p->pBt->mutex)==SQLITE_OK ){
    p->pBt->db = p->db;
    p->locked = 1;
    ਵਾਪਸ;
  }

  /* To avoid deadlock, first release all locks with a larger
  ** BtShared address.  Then acquire our lock.  Then reacquire
  ** the other BtShared locks that we used to hold in ascending
  ** order.
  */
  ਲਈ(pLater=p->p_ਅਗਲਾ; pLater; pLater=pLater->p_ਅਗਲਾ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pLater->sharable );
    ਪ੍ਰਤਿਜ੍ਞਾ( pLater->p_ਅਗਲਾ==0 || pLater->p_ਅਗਲਾ->pBt>pLater->pBt );
    ਪ੍ਰਤਿਜ੍ਞਾ( !pLater->locked || pLater->want_ਮੰਜ਼ਿਲ_ਤਾਲਾ>0 );
    ਜੇ( pLater->locked ){
      unlockBtreeMutex(pLater);
    }
  }
  ਤਾਲਾ_Btree_Mutex(p);
  ਲਈ(pLater=p->p_ਅਗਲਾ; pLater; pLater=pLater->p_ਅਗਲਾ){
    ਜੇ( pLater->want_ਮੰਜ਼ਿਲ_ਤਾਲਾ ){
      ਤਾਲਾ_Btree_Mutex(pLater);
    }
  }
}


/*
** Exit the recursive mutex on a Btree.
*/
ਖਾਲੀ sqlite3BtreeLeave(Btree *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->db->mutex) );
  ਜੇ( p->sharable ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->want_ਮੰਜ਼ਿਲ_ਤਾਲਾ>0 );
    p->want_ਮੰਜ਼ਿਲ_ਤਾਲਾ--;
    ਜੇ( p->want_ਮੰਜ਼ਿਲ_ਤਾਲਾ==0 ){
      unlockBtreeMutex(p);
    }
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
/*
** Return true if the BtShared mutex is held on the btree, or if the
** B-Tree is not marked as sharable.
**
** This routine is used only from within assert() statements.
*/
ਪੂਰਨ sqlite3BtreeHoldsMutex(Btree *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->sharable==0 || p->locked==0 || p->want_ਮੰਜ਼ਿਲ_ਤਾਲਾ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->sharable==0 || p->locked==0 || p->db==p->pBt->db );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->sharable==0 || p->locked==0 || sqlite3_mutex_held(p->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->sharable==0 || p->locked==0 || sqlite3_mutex_held(p->db->mutex) );

  ਵਾਪਸ (p->sharable==0 || p->locked);
}
#ਜੇ_ਅੰਤ


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
ਸਥਿਰ ਖਾਲੀ SQLITE_NOINLINE btreeEnterAll(sqlite3 *db){
  ਪੂਰਨ i;
  u8 skipOk = 1;
  Btree *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਲਈ(i=0; i<db->nDb; i++){
    p = db->aDb[i].pBt;
    ਜੇ( p && p->sharable ){
      sqlite3BtreeEnter(p);
      skipOk = 0;
    }
  }
  db->noSharedCache = skipOk;
}
ਖਾਲੀ sqlite3BtreeEnterAll(sqlite3 *db){
  ਜੇ( db->noSharedCache==0 ) btreeEnterAll(db);
}
ਸਥਿਰ ਖਾਲੀ SQLITE_NOINLINE btreeLeaveAll(sqlite3 *db){
  ਪੂਰਨ i;
  Btree *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਲਈ(i=0; i<db->nDb; i++){
    p = db->aDb[i].pBt;
    ਜੇ( p ) sqlite3BtreeLeave(p);
  }
}
ਖਾਲੀ sqlite3BtreeLeaveAll(sqlite3 *db){
  ਜੇ( db->noSharedCache==0 ) btreeLeaveAll(db);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
/*
** Return true if the current thread holds the database connection
** mutex and all required BtShared mutexes.
**
** This routine is used inside assert() statements only.
*/
ਪੂਰਨ sqlite3BtreeHoldsAllMutexes(sqlite3 *db){
  ਪੂਰਨ i;
  ਜੇ( !sqlite3_mutex_held(db->mutex) ){
    ਵਾਪਸ 0;
  }
  ਲਈ(i=0; i<db->nDb; i++){
    Btree *p;
    p = db->aDb[i].pBt;
    ਜੇ( p && p->sharable &&
         (p->want_ਮੰਜ਼ਿਲ_ਤਾਲਾ==0 || !sqlite3_mutex_held(p->pBt->mutex)) ){
      ਵਾਪਸ 0;
    }
  }
  ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ /* NDEBUG */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
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
ਪੂਰਨ sqlite3SchemaMutexHeld(sqlite3 *db, ਪੂਰਨ iDb, Schema *pSchema){
  Btree *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਜੇ( db->pVfs==0 && db->nDb==0 ) ਵਾਪਸ 1;
  ਜੇ( pSchema ) iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pSchema);
  ਪ੍ਰਤਿਜ੍ਞਾ( iDb>=0 && iDb<db->nDb );
  ਜੇ( !sqlite3_mutex_held(db->mutex) ) ਵਾਪਸ 0;
  ਜੇ( iDb==1 ) ਵਾਪਸ 1;
  p = db->aDb[iDb].pBt;
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਵਾਪਸ p->sharable==0 || p->locked==1;
}
#ਜੇ_ਅੰਤ /* NDEBUG */

#ਨਹੀਂਤਾਂ /* SQLITE_THREADSAFE>0 above.  SQLITE_THREADSAFE==0 below */
/*
** The following are special cases for mutex enter routines for use
** in single threaded applications that use shared cache.  Except for
** these two routines, all mutex operations are no-ops in that case and
** are null #defines in btree.h.
**
** If shared cache is disabled, then all btree mutex routines, including
** the ones below, are no-ops and are null #defines in btree.h.
*/

ਖਾਲੀ sqlite3BtreeEnter(Btree *p){
  p->pBt->db = p->db;
}
ਖਾਲੀ sqlite3BtreeEnterAll(sqlite3 *db){
  ਪੂਰਨ i;
  u8 skipOk = 1;
  /* As in the threadsafe build: set db->noSharedCache when no attached
  ** Btree is sharable, so that schema and table-lock checks are skipped */
  ਜੇ( db->noSharedCache ) ਵਾਪਸ;
  ਲਈ(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    ਜੇ( p ){
      p->pBt->db = p->db;
      ਜੇ( p->sharable ) skipOk = 0;
    }
  }
  db->noSharedCache = skipOk;
}
#ਜੇ_ਅੰਤ /* if SQLITE_THREADSAFE */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
/*
** Enter a mutex on a Btree given a cursor owned by that Btree. 
**
** These entry points are used by incremental I/O only. Enter() is required 
** any time OMIT_SHARED_CACHE is not defined, regardless of whether or not 
** the build is threadsafe. Leave() is only required by threadsafe builds.
*/
ਖਾਲੀ sqlite3BtreeEnterCursor(BtCursor *pCur){
  sqlite3BtreeEnter(pCur->pBtree);
}
# ਜੇ SQLITE_THREADSAFE
ਖਾਲੀ sqlite3BtreeLeaveCursor(BtCursor *pCur){
  sqlite3BtreeLeave(pCur->pBtree);
}
# endif
#ਜੇ_ਅੰਤ /* ifndef SQLITE_OMIT_INCRBLOB */

#ਜੇ_ਅੰਤ /* ifndef SQLITE_OMIT_SHARED_CACHE */
