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
#అన్తర్భావ "btreeInt.h"
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
#ఒకవేళ SQLITE_THREADSAFE

/*
** Obtain the BtShared mutex associated with B-Tree handle p. Also,
** set BtShared.db to the database handle associated with p and the
** p->locked boolean to true.
*/
స్థిర శూన్య తాళం_Btree_Mutex(Btree *p){
  ప్రతిజ్ఞా( p->locked==0 );
  ప్రతిజ్ఞా( sqlite3_mutex_notheld(p->pBt->mutex) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p->db->mutex) );

  sqlite3_mutex_enter(p->pBt->mutex);
  p->pBt->db = p->db;
  p->locked = 1;
}

/*
** Release the BtShared mutex associated with B-Tree handle p and
** clear the p->locked boolean.
*/
స్థిర శూన్య SQLITE_NOINLINE unlockBtreeMutex(Btree *p){
  BtShared *pBt = p->pBt;
  ప్రతిజ్ఞా( p->locked==1 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p->db->mutex) );
  ప్రతిజ్ఞా( p->db==pBt->db );

  sqlite3_mutex_leave(pBt->mutex);
  p->locked = 0;
}

/* Forward reference */
స్థిర శూన్య SQLITE_NOINLINE btree_తాళం_Carefully(Btree *p);

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
శూన్య sqlite3BtreeEnter(Btree *p){
  /* Some basic sanity checking on the Btree.  The list of Btrees
  ** connected by pNext and pPrev should be in sorted order by
  ** Btree.pBt value. All elements of the list should belong to
  ** the same connection. Only shared Btrees are on the list. */
  ప్రతిజ్ఞా( p->p_తదుపరి==0 || p->p_తదుపరి->pBt>p->pBt );
  ప్రతిజ్ఞా( p->pPrev==0 || p->pPrev->pBt<p->pBt );
  ప్రతిజ్ఞా( p->p_తదుపరి==0 || p->p_తదుపరి->db==p->db );
  ప్రతిజ్ఞా( p->pPrev==0 || p->pPrev->db==p->db );
  ప్రతిజ్ఞా( p->sharable || (p->p_తదుపరి==0 && p->pPrev==0) );

  /* Check for locking consistency */
  ప్రతిజ్ఞా( !p->locked || p->want_గమ్యం_తాళం>0 );
  ప్రతిజ్ఞా( p->sharable || p->want_గమ్యం_తాళం==0 );

  /* We should already hold a lock on the database connection */
  ప్రతిజ్ఞా( sqlite3_mutex_held(p->db->mutex) );

  /* Unless the database is sharable and unlocked, then BtShared.db
  ** should already be set correctly. */
  ప్రతిజ్ఞా( (p->locked==0 && p->sharable) || p->pBt->db==p->db );

  ఒకవేళ( !p->sharable ) తిరిగి;
  p->want_గమ్యం_తాళం++;
  ఒకవేళ( p->locked ) తిరిగి;
  btree_తాళం_Carefully(p);
}

/* This is a helper function for sqlite3BtreeLock(). By moving
** complex, but seldom used logic, out of sqlite3BtreeLock() and
** into this routine, we avoid unnecessary stack pointer changes
** and thus help the sqlite3BtreeLock() routine to run much faster
** in the common case.
*/
స్థిర శూన్య SQLITE_NOINLINE btree_తాళం_Carefully(Btree *p){
  Btree *pLater;

  /* In most cases, we should be able to acquire the lock we
  ** want without having to go through the ascending lock
  ** procedure that follows.  Just be sure not to block.
  */
  ఒకవేళ( sqlite3_mutex_try(p->pBt->mutex)==SQLITE_OK ){
    p->pBt->db = p->db;
    p->locked = 1;
    తిరిగి;
  }

  /* To avoid deadlock, first release all locks with a larger
  ** BtShared address.  Then acquire our lock.  Then reacquire
  ** the other BtShared locks that we used to hold in ascending
  ** order.
  */
  కొరకు(pLater=p->p_తదుపరి; pLater; pLater=pLater->p_తదుపరి){
    ప్రతిజ్ఞా( pLater->sharable );
    ప్రతిజ్ఞా( pLater->p_తదుపరి==0 || pLater->p_తదుపరి->pBt>pLater->pBt );
    ప్రతిజ్ఞా( !pLater->locked || pLater->want_గమ్యం_తాళం>0 );
    ఒకవేళ( pLater->locked ){
      unlockBtreeMutex(pLater);
    }
  }
  తాళం_Btree_Mutex(p);
  కొరకు(pLater=p->p_తదుపరి; pLater; pLater=pLater->p_తదుపరి){
    ఒకవేళ( pLater->want_గమ్యం_తాళం ){
      తాళం_Btree_Mutex(pLater);
    }
  }
}


/*
** Exit the recursive mutex on a Btree.
*/
శూన్య sqlite3BtreeLeave(Btree *p){
  ప్రతిజ్ఞా( sqlite3_mutex_held(p->db->mutex) );
  ఒకవేళ( p->sharable ){
    ప్రతిజ్ఞా( p->want_గమ్యం_తాళం>0 );
    p->want_గమ్యం_తాళం--;
    ఒకవేళ( p->want_గమ్యం_తాళం==0 ){
      unlockBtreeMutex(p);
    }
  }
}

#నిర్వచితం_కాకపోతే NDEBUG
/*
** Return true if the BtShared mutex is held on the btree, or if the
** B-Tree is not marked as sharable.
**
** This routine is used only from within assert() statements.
*/
పూర్ణాంకం sqlite3BtreeHoldsMutex(Btree *p){
  ప్రతిజ్ఞా( p->sharable==0 || p->locked==0 || p->want_గమ్యం_తాళం>0 );
  ప్రతిజ్ఞా( p->sharable==0 || p->locked==0 || p->db==p->pBt->db );
  ప్రతిజ్ఞా( p->sharable==0 || p->locked==0 || sqlite3_mutex_held(p->pBt->mutex) );
  ప్రతిజ్ఞా( p->sharable==0 || p->locked==0 || sqlite3_mutex_held(p->db->mutex) );

  తిరిగి (p->sharable==0 || p->locked);
}
#అయితే_ముగింపు


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
స్థిర శూన్య SQLITE_NOINLINE btreeEnterAll(sqlite3 *db){
  పూర్ణాంకం i;
  u8 skipOk = 1;
  Btree *p;
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  కొరకు(i=0; i<db->nDb; i++){
    p = db->aDb[i].pBt;
    ఒకవేళ( p && p->sharable ){
      sqlite3BtreeEnter(p);
      skipOk = 0;
    }
  }
  db->noSharedCache = skipOk;
}
శూన్య sqlite3BtreeEnterAll(sqlite3 *db){
  ఒకవేళ( db->noSharedCache==0 ) btreeEnterAll(db);
}
స్థిర శూన్య SQLITE_NOINLINE btreeLeaveAll(sqlite3 *db){
  పూర్ణాంకం i;
  Btree *p;
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  కొరకు(i=0; i<db->nDb; i++){
    p = db->aDb[i].pBt;
    ఒకవేళ( p ) sqlite3BtreeLeave(p);
  }
}
శూన్య sqlite3BtreeLeaveAll(sqlite3 *db){
  ఒకవేళ( db->noSharedCache==0 ) btreeLeaveAll(db);
}

#నిర్వచితం_కాకపోతే NDEBUG
/*
** Return true if the current thread holds the database connection
** mutex and all required BtShared mutexes.
**
** This routine is used inside assert() statements only.
*/
పూర్ణాంకం sqlite3BtreeHoldsAllMutexes(sqlite3 *db){
  పూర్ణాంకం i;
  ఒకవేళ( !sqlite3_mutex_held(db->mutex) ){
    తిరిగి 0;
  }
  కొరకు(i=0; i<db->nDb; i++){
    Btree *p;
    p = db->aDb[i].pBt;
    ఒకవేళ( p && p->sharable &&
         (p->want_గమ్యం_తాళం==0 || !sqlite3_mutex_held(p->pBt->mutex)) ){
      తిరిగి 0;
    }
  }
  తిరిగి 1;
}
#అయితే_ముగింపు /* NDEBUG */

#నిర్వచితం_కాకపోతే NDEBUG
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
పూర్ణాంకం sqlite3SchemaMutexHeld(sqlite3 *db, పూర్ణాంకం iDb, Schema *pSchema){
  Btree *p;
  ప్రతిజ్ఞా( db!=0 );
  ఒకవేళ( db->pVfs==0 && db->nDb==0 ) తిరిగి 1;
  ఒకవేళ( pSchema ) iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pSchema);
  ప్రతిజ్ఞా( iDb>=0 && iDb<db->nDb );
  ఒకవేళ( !sqlite3_mutex_held(db->mutex) ) తిరిగి 0;
  ఒకవేళ( iDb==1 ) తిరిగి 1;
  p = db->aDb[iDb].pBt;
  ప్రతిజ్ఞా( p!=0 );
  తిరిగి p->sharable==0 || p->locked==1;
}
#అయితే_ముగింపు /* NDEBUG */

#లేకపోతే /* SQLITE_THREADSAFE>0 above.  SQLITE_THREADSAFE==0 below */
/*
** The following are special cases for mutex enter routines for use
** in single threaded applications that use shared cache.  Except for
** these two routines, all mutex operations are no-ops in that case and
** are null #defines in btree.h.
**
** If shared cache is disabled, then all btree mutex routines, including
** the ones below, are no-ops and are null #defines in btree.h.
*/

శూన్య sqlite3BtreeEnter(Btree *p){
  p->pBt->db = p->db;
}
శూన్య sqlite3BtreeEnterAll(sqlite3 *db){
  పూర్ణాంకం i;
  u8 skipOk = 1;
  /* As in the threadsafe build: set db->noSharedCache when no attached
  ** Btree is sharable, so that schema and table-lock checks are skipped */
  ఒకవేళ( db->noSharedCache ) తిరిగి;
  కొరకు(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    ఒకవేళ( p ){
      p->pBt->db = p->db;
      ఒకవేళ( p->sharable ) skipOk = 0;
    }
  }
  db->noSharedCache = skipOk;
}
#అయితే_ముగింపు /* if SQLITE_THREADSAFE */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_INCRBLOB
/*
** Enter a mutex on a Btree given a cursor owned by that Btree. 
**
** These entry points are used by incremental I/O only. Enter() is required 
** any time OMIT_SHARED_CACHE is not defined, regardless of whether or not 
** the build is threadsafe. Leave() is only required by threadsafe builds.
*/
శూన్య sqlite3BtreeEnterCursor(BtCursor *pCur){
  sqlite3BtreeEnter(pCur->pBtree);
}
# ఒకవేళ SQLITE_THREADSAFE
శూన్య sqlite3BtreeLeaveCursor(BtCursor *pCur){
  sqlite3BtreeLeave(pCur->pBtree);
}
# endif
#అయితే_ముగింపు /* ifndef SQLITE_OMIT_INCRBLOB */

#అయితే_ముగింపు /* ifndef SQLITE_OMIT_SHARED_CACHE */
