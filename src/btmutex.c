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
#شامل "btreeInt.h"
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
#اگر SQLITE_THREADSAFE

/*
** Obtain the BtShared mutex associated with B-Tree handle p. Also,
** set BtShared.db to the database handle associated with p and the
** p->locked boolean to true.
*/
ساکن خالی تالا_Btree_Mutex(Btree *p){
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
ساکن خالی SQLITE_NOINLINE unlockBtreeMutex(Btree *p){
  BtShared *pBt = p->pBt;
  أكد( p->locked==1 );
  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( sqlite3_mutex_held(p->db->mutex) );
  أكد( p->db==pBt->db );

  sqlite3_mutex_leave(pBt->mutex);
  p->locked = 0;
}

/* Forward reference */
ساکن خالی SQLITE_NOINLINE btree_تالا_Carefully(Btree *p);

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
خالی sqlite3BtreeEnter(Btree *p){
  /* Some basic sanity checking on the Btree.  The list of Btrees
  ** connected by pNext and pPrev should be in sorted order by
  ** Btree.pBt value. All elements of the list should belong to
  ** the same connection. Only shared Btrees are on the list. */
  أكد( p->p_اگلا==0 || p->p_اگلا->pBt>p->pBt );
  أكد( p->p_پچھلا==0 || p->p_پچھلا->pBt<p->pBt );
  أكد( p->p_اگلا==0 || p->p_اگلا->db==p->db );
  أكد( p->p_پچھلا==0 || p->p_پچھلا->db==p->db );
  أكد( p->sharable || (p->p_اگلا==0 && p->p_پچھلا==0) );

  /* Check for locking consistency */
  أكد( !p->locked || p->want_مقصد_تالا>0 );
  أكد( p->sharable || p->want_مقصد_تالا==0 );

  /* We should already hold a lock on the database connection */
  أكد( sqlite3_mutex_held(p->db->mutex) );

  /* Unless the database is sharable and unlocked, then BtShared.db
  ** should already be set correctly. */
  أكد( (p->locked==0 && p->sharable) || p->pBt->db==p->db );

  اگر( !p->sharable ) واپس;
  p->want_مقصد_تالا++;
  اگر( p->locked ) واپس;
  btree_تالا_Carefully(p);
}

/* This is a helper function for sqlite3BtreeLock(). By moving
** complex, but seldom used logic, out of sqlite3BtreeLock() and
** into this routine, we avoid unnecessary stack pointer changes
** and thus help the sqlite3BtreeLock() routine to run much faster
** in the common case.
*/
ساکن خالی SQLITE_NOINLINE btree_تالا_Carefully(Btree *p){
  Btree *pLater;

  /* In most cases, we should be able to acquire the lock we
  ** want without having to go through the ascending lock
  ** procedure that follows.  Just be sure not to block.
  */
  اگر( sqlite3_mutex_try(p->pBt->mutex)==SQLITE_OK ){
    p->pBt->db = p->db;
    p->locked = 1;
    واپس;
  }

  /* To avoid deadlock, first release all locks with a larger
  ** BtShared address.  Then acquire our lock.  Then reacquire
  ** the other BtShared locks that we used to hold in ascending
  ** order.
  */
  برائے(pLater=p->p_اگلا; pLater; pLater=pLater->p_اگلا){
    أكد( pLater->sharable );
    أكد( pLater->p_اگلا==0 || pLater->p_اگلا->pBt>pLater->pBt );
    أكد( !pLater->locked || pLater->want_مقصد_تالا>0 );
    اگر( pLater->locked ){
      unlockBtreeMutex(pLater);
    }
  }
  تالا_Btree_Mutex(p);
  برائے(pLater=p->p_اگلا; pLater; pLater=pLater->p_اگلا){
    اگر( pLater->want_مقصد_تالا ){
      تالا_Btree_Mutex(pLater);
    }
  }
}


/*
** Exit the recursive mutex on a Btree.
*/
خالی sqlite3BtreeLeave(Btree *p){
  أكد( sqlite3_mutex_held(p->db->mutex) );
  اگر( p->sharable ){
    أكد( p->want_مقصد_تالا>0 );
    p->want_مقصد_تالا--;
    اگر( p->want_مقصد_تالا==0 ){
      unlockBtreeMutex(p);
    }
  }
}

#اگر_غیر_تعریف_شدہ NDEBUG
/*
** Return true if the BtShared mutex is held on the btree, or if the
** B-Tree is not marked as sharable.
**
** This routine is used only from within assert() statements.
*/
صحیح sqlite3BtreeHoldsMutex(Btree *p){
  أكد( p->sharable==0 || p->locked==0 || p->want_مقصد_تالا>0 );
  أكد( p->sharable==0 || p->locked==0 || p->db==p->pBt->db );
  أكد( p->sharable==0 || p->locked==0 || sqlite3_mutex_held(p->pBt->mutex) );
  أكد( p->sharable==0 || p->locked==0 || sqlite3_mutex_held(p->db->mutex) );

  واپس (p->sharable==0 || p->locked);
}
#اگر_ختم


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
ساکن خالی SQLITE_NOINLINE btreeEnterAll(sqlite3 *db){
  صحیح i;
  u8 skipOk = 1;
  Btree *p;
  أكد( sqlite3_mutex_held(db->mutex) );
  برائے(i=0; i<db->nDb; i++){
    p = db->aDb[i].pBt;
    اگر( p && p->sharable ){
      sqlite3BtreeEnter(p);
      skipOk = 0;
    }
  }
  db->noSharedCache = skipOk;
}
خالی sqlite3BtreeEnterAll(sqlite3 *db){
  اگر( db->noSharedCache==0 ) btreeEnterAll(db);
}
ساکن خالی SQLITE_NOINLINE btreeLeaveAll(sqlite3 *db){
  صحیح i;
  Btree *p;
  أكد( sqlite3_mutex_held(db->mutex) );
  برائے(i=0; i<db->nDb; i++){
    p = db->aDb[i].pBt;
    اگر( p ) sqlite3BtreeLeave(p);
  }
}
خالی sqlite3BtreeLeaveAll(sqlite3 *db){
  اگر( db->noSharedCache==0 ) btreeLeaveAll(db);
}

#اگر_غیر_تعریف_شدہ NDEBUG
/*
** Return true if the current thread holds the database connection
** mutex and all required BtShared mutexes.
**
** This routine is used inside assert() statements only.
*/
صحیح sqlite3BtreeHoldsAllMutexes(sqlite3 *db){
  صحیح i;
  اگر( !sqlite3_mutex_held(db->mutex) ){
    واپس 0;
  }
  برائے(i=0; i<db->nDb; i++){
    Btree *p;
    p = db->aDb[i].pBt;
    اگر( p && p->sharable &&
         (p->want_مقصد_تالا==0 || !sqlite3_mutex_held(p->pBt->mutex)) ){
      واپس 0;
    }
  }
  واپس 1;
}
#اگر_ختم /* NDEBUG */

#اگر_غیر_تعریف_شدہ NDEBUG
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
صحیح sqlite3SchemaMutexHeld(sqlite3 *db, صحیح iDb, Schema *pSchema){
  Btree *p;
  أكد( db!=0 );
  اگر( db->pVfs==0 && db->nDb==0 ) واپس 1;
  اگر( pSchema ) iDb = sqlite3_Schema_مقصد_اشاریہ(db, pSchema);
  أكد( iDb>=0 && iDb<db->nDb );
  اگر( !sqlite3_mutex_held(db->mutex) ) واپس 0;
  اگر( iDb==1 ) واپس 1;
  p = db->aDb[iDb].pBt;
  أكد( p!=0 );
  واپس p->sharable==0 || p->locked==1;
}
#اگر_ختم /* NDEBUG */

#ورنہ /* SQLITE_THREADSAFE>0 above.  SQLITE_THREADSAFE==0 below */
/*
** The following are special cases for mutex enter routines for use
** in single threaded applications that use shared cache.  Except for
** these two routines, all mutex operations are no-ops in that case and
** are null #defines in btree.h.
**
** If shared cache is disabled, then all btree mutex routines, including
** the ones below, are no-ops and are null #defines in btree.h.
*/

خالی sqlite3BtreeEnter(Btree *p){
  p->pBt->db = p->db;
}
خالی sqlite3BtreeEnterAll(sqlite3 *db){
  صحیح i;
  u8 skipOk = 1;
  /* As in the threadsafe build: set db->noSharedCache when no attached
  ** Btree is sharable, so that schema and table-lock checks are skipped */
  اگر( db->noSharedCache ) واپس;
  برائے(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    اگر( p ){
      p->pBt->db = p->db;
      اگر( p->sharable ) skipOk = 0;
    }
  }
  db->noSharedCache = skipOk;
}
#اگر_ختم /* if SQLITE_THREADSAFE */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
/*
** Enter a mutex on a Btree given a cursor owned by that Btree. 
**
** These entry points are used by incremental I/O only. Enter() is required 
** any time OMIT_SHARED_CACHE is not defined, regardless of whether or not 
** the build is threadsafe. Leave() is only required by threadsafe builds.
*/
خالی sqlite3BtreeEnterCursor(BtCursor *pCur){
  sqlite3BtreeEnter(pCur->pBtree);
}
# اگر SQLITE_THREADSAFE
خالی sqlite3BtreeLeaveCursor(BtCursor *pCur){
  sqlite3BtreeLeave(pCur->pBtree);
}
# endif
#اگر_ختم /* ifndef SQLITE_OMIT_INCRBLOB */

#اگر_ختم /* ifndef SQLITE_OMIT_SHARED_CACHE */
