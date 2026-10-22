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
#ضمن "btreeInt.h"
#ifndef SQLITE_OMIT_SHARED_CACHE
#إذا SQLITE_THREADSAFE

/*
** Obtain the BtShared mutex associated with B-Tree handle p. Also,
** set BtShared.db to the database handle associated with p and the
** p->locked boolean to true.
*/
ساكن فراغ lockBtreeMutex(Btree *p){
  أكد( p->locked==0 );
  أكد( sqlite3_mutex_notheld(p->pBt->mutex) );
  أكد( sqlite3_mutex_held(p->db->mutex) );

  sqlite3_mutex_enter(p->pBt->mutex);
  p->pBt->db = p->db;
  p->locked = 1;
}

/*
** Release the BtShared mutex associated with B-Tree handle p and
** clear the p->locked boolean.
*/
ساكن فراغ SQLITE_NOINLINE unlockBtreeMutex(Btree *p){
  BtShared *pBt = p->pBt;
  أكد( p->locked==1 );
  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( sqlite3_mutex_held(p->db->mutex) );
  أكد( p->db==pBt->db );

  sqlite3_mutex_leave(pBt->mutex);
  p->locked = 0;
}

/* Forward reference */
ساكن فراغ SQLITE_NOINLINE btreeLockCarefully(Btree *p);

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
فراغ sqlite3BtreeEnter(Btree *p){
  /* Some basic sanity checking on the Btree.  The list of Btrees
  ** connected by pNext and pPrev should be in sorted order by
  ** Btree.pBt value. All elements of the list should belong to
  ** the same connection. Only shared Btrees are on the list. */
  أكد( p->pNext==0 || p->pNext->pBt>p->pBt );
  أكد( p->pPrev==0 || p->pPrev->pBt<p->pBt );
  أكد( p->pNext==0 || p->pNext->db==p->db );
  أكد( p->pPrev==0 || p->pPrev->db==p->db );
  أكد( p->sharable || (p->pNext==0 && p->pPrev==0) );

  /* Check for locking consistency */
  أكد( !p->locked || p->wantToLock>0 );
  أكد( p->sharable || p->wantToLock==0 );

  /* We should already hold a lock on the database connection */
  أكد( sqlite3_mutex_held(p->db->mutex) );

  /* Unless the database is sharable and unlocked, then BtShared.db
  ** should already be set correctly. */
  أكد( (p->locked==0 && p->sharable) || p->pBt->db==p->db );

  إذا( !p->sharable ) إرجاع;
  p->wantToLock++;
  إذا( p->locked ) إرجاع;
  btreeLockCarefully(p);
}

/* This is a helper function for sqlite3BtreeLock(). By moving
** complex, but seldom used logic, out of sqlite3BtreeLock() and
** into this routine, we avoid unnecessary stack pointer changes
** and thus help the sqlite3BtreeLock() routine to run much faster
** in the common case.
*/
ساكن فراغ SQLITE_NOINLINE btreeLockCarefully(Btree *p){
  Btree *pLater;

  /* In most cases, we should be able to acquire the lock we
  ** want without having to go through the ascending lock
  ** procedure that follows.  Just be sure not to block.
  */
  إذا( sqlite3_mutex_try(p->pBt->mutex)==SQLITE_OK ){
    p->pBt->db = p->db;
    p->locked = 1;
    إرجاع;
  }

  /* To avoid deadlock, first release all locks with a larger
  ** BtShared address.  Then acquire our lock.  Then reacquire
  ** the other BtShared locks that we used to hold in ascending
  ** order.
  */
  لكل(pLater=p->pNext; pLater; pLater=pLater->pNext){
    أكد( pLater->sharable );
    أكد( pLater->pNext==0 || pLater->pNext->pBt>pLater->pBt );
    أكد( !pLater->locked || pLater->wantToLock>0 );
    إذا( pLater->locked ){
      unlockBtreeMutex(pLater);
    }
  }
  lockBtreeMutex(p);
  لكل(pLater=p->pNext; pLater; pLater=pLater->pNext){
    إذا( pLater->wantToLock ){
      lockBtreeMutex(pLater);
    }
  }
}


/*
** Exit the recursive mutex on a Btree.
*/
فراغ sqlite3BtreeLeave(Btree *p){
  أكد( sqlite3_mutex_held(p->db->mutex) );
  إذا( p->sharable ){
    أكد( p->wantToLock>0 );
    p->wantToLock--;
    إذا( p->wantToLock==0 ){
      unlockBtreeMutex(p);
    }
  }
}

#ifndef NDEBUG
/*
** Return true if the BtShared mutex is held on the btree, or if the
** B-Tree is not marked as sharable.
**
** This routine is used only from within assert() statements.
*/
صحيح sqlite3BtreeHoldsMutex(Btree *p){
  أكد( p->sharable==0 || p->locked==0 || p->wantToLock>0 );
  أكد( p->sharable==0 || p->locked==0 || p->db==p->pBt->db );
  أكد( p->sharable==0 || p->locked==0 || sqlite3_mutex_held(p->pBt->mutex) );
  أكد( p->sharable==0 || p->locked==0 || sqlite3_mutex_held(p->db->mutex) );

  إرجاع (p->sharable==0 || p->locked);
}
#endif


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
ساكن فراغ SQLITE_NOINLINE btreeEnterAll(sqlite3 *db){
  صحيح i;
  u8 skipOk = 1;
  Btree *p;
  أكد( sqlite3_mutex_held(db->mutex) );
  لكل(i=0; i<db->nDb; i++){
    p = db->aDb[i].pBt;
    إذا( p && p->sharable ){
      sqlite3BtreeEnter(p);
      skipOk = 0;
    }
  }
  db->noSharedCache = skipOk;
}
فراغ sqlite3BtreeEnterAll(sqlite3 *db){
  إذا( db->noSharedCache==0 ) btreeEnterAll(db);
}
ساكن فراغ SQLITE_NOINLINE btreeLeaveAll(sqlite3 *db){
  صحيح i;
  Btree *p;
  أكد( sqlite3_mutex_held(db->mutex) );
  لكل(i=0; i<db->nDb; i++){
    p = db->aDb[i].pBt;
    إذا( p ) sqlite3BtreeLeave(p);
  }
}
فراغ sqlite3BtreeLeaveAll(sqlite3 *db){
  إذا( db->noSharedCache==0 ) btreeLeaveAll(db);
}

#ifndef NDEBUG
/*
** Return true if the current thread holds the database connection
** mutex and all required BtShared mutexes.
**
** This routine is used inside assert() statements only.
*/
صحيح sqlite3BtreeHoldsAllMutexes(sqlite3 *db){
  صحيح i;
  إذا( !sqlite3_mutex_held(db->mutex) ){
    إرجاع 0;
  }
  لكل(i=0; i<db->nDb; i++){
    Btree *p;
    p = db->aDb[i].pBt;
    إذا( p && p->sharable &&
         (p->wantToLock==0 || !sqlite3_mutex_held(p->pBt->mutex)) ){
      إرجاع 0;
    }
  }
  إرجاع 1;
}
#endif /* NDEBUG */

#ifndef NDEBUG
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
صحيح sqlite3SchemaMutexHeld(sqlite3 *db, صحيح iDb, Schema *pSchema){
  Btree *p;
  أكد( db!=0 );
  إذا( db->pVfs==0 && db->nDb==0 ) إرجاع 1;
  إذا( pSchema ) iDb = sqlite3SchemaToIndex(db, pSchema);
  أكد( iDb>=0 && iDb<db->nDb );
  إذا( !sqlite3_mutex_held(db->mutex) ) إرجاع 0;
  إذا( iDb==1 ) إرجاع 1;
  p = db->aDb[iDb].pBt;
  أكد( p!=0 );
  إرجاع p->sharable==0 || p->locked==1;
}
#endif /* NDEBUG */

#وإلا /* SQLITE_THREADSAFE>0 above.  SQLITE_THREADSAFE==0 below */
/*
** The following are special cases for mutex enter routines for use
** in single threaded applications that use shared cache.  Except for
** these two routines, all mutex operations are no-ops in that case and
** are null #defines in btree.h.
**
** If shared cache is disabled, then all btree mutex routines, including
** the ones below, are no-ops and are null #defines in btree.h.
*/

فراغ sqlite3BtreeEnter(Btree *p){
  p->pBt->db = p->db;
}
فراغ sqlite3BtreeEnterAll(sqlite3 *db){
  صحيح i;
  u8 skipOk = 1;
  /* As in the threadsafe build: set db->noSharedCache when no attached
  ** Btree is sharable, so that schema and table-lock checks are skipped */
  إذا( db->noSharedCache ) إرجاع;
  لكل(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    إذا( p ){
      p->pBt->db = p->db;
      إذا( p->sharable ) skipOk = 0;
    }
  }
  db->noSharedCache = skipOk;
}
#endif /* if SQLITE_THREADSAFE */

#ifndef SQLITE_OMIT_INCRBLOB
/*
** Enter a mutex on a Btree given a cursor owned by that Btree. 
**
** These entry points are used by incremental I/O only. Enter() is required 
** any time OMIT_SHARED_CACHE is not defined, regardless of whether or not 
** the build is threadsafe. Leave() is only required by threadsafe builds.
*/
فراغ sqlite3BtreeEnterCursor(BtCursor *pCur){
  sqlite3BtreeEnter(pCur->pBtree);
}
# إذا SQLITE_THREADSAFE
فراغ sqlite3BtreeLeaveCursor(BtCursor *pCur){
  sqlite3BtreeLeave(pCur->pBtree);
}
# endif
#endif /* ifndef SQLITE_OMIT_INCRBLOB */

#endif /* ifndef SQLITE_OMIT_SHARED_CACHE */
