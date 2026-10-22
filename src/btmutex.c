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
#समावेश "btreeInt.h"
#अगर_अघोषित SQLITE_OMIT_SHARED_CACHE
#अगर SQLITE_THREADSAFE

/*
** Obtain the BtShared mutex associated with B-Tree handle p. Also,
** set BtShared.db to the database handle associated with p and the
** p->locked boolean to true.
*/
अटल व्योम lockBtreeMutex(Btree *p){
  निश्चित( p->locked==0 );
  निश्चित( sqlite3_mutex_notheld(p->pBt->mutex) );
  निश्चित( sqlite3_mutex_held(p->db->mutex) );

  sqlite3_mutex_enter(p->pBt->mutex);
  p->pBt->db = p->db;
  p->locked = 1;
}

/*
** Release the BtShared mutex associated with B-Tree handle p and
** clear the p->locked boolean.
*/
अटल व्योम SQLITE_NOINLINE unlockBtreeMutex(Btree *p){
  BtShared *pBt = p->pBt;
  निश्चित( p->locked==1 );
  निश्चित( sqlite3_mutex_held(pBt->mutex) );
  निश्चित( sqlite3_mutex_held(p->db->mutex) );
  निश्चित( p->db==pBt->db );

  sqlite3_mutex_leave(pBt->mutex);
  p->locked = 0;
}

/* Forward reference */
अटल व्योम SQLITE_NOINLINE btreeLockCarefully(Btree *p);

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
व्योम sqlite3BtreeEnter(Btree *p){
  /* Some basic sanity checking on the Btree.  The list of Btrees
  ** connected by pNext and pPrev should be in sorted order by
  ** Btree.pBt value. All elements of the list should belong to
  ** the same connection. Only shared Btrees are on the list. */
  निश्चित( p->p_अग्र==0 || p->p_अग्र->pBt>p->pBt );
  निश्चित( p->p_पूर्व==0 || p->p_पूर्व->pBt<p->pBt );
  निश्चित( p->p_अग्र==0 || p->p_अग्र->db==p->db );
  निश्चित( p->p_पूर्व==0 || p->p_पूर्व->db==p->db );
  निश्चित( p->sharable || (p->p_अग्र==0 && p->p_पूर्व==0) );

  /* Check for locking consistency */
  निश्चित( !p->locked || p->wantToLock>0 );
  निश्चित( p->sharable || p->wantToLock==0 );

  /* We should already hold a lock on the database connection */
  निश्चित( sqlite3_mutex_held(p->db->mutex) );

  /* Unless the database is sharable and unlocked, then BtShared.db
  ** should already be set correctly. */
  निश्चित( (p->locked==0 && p->sharable) || p->pBt->db==p->db );

  अगर( !p->sharable ) वापस;
  p->wantToLock++;
  अगर( p->locked ) वापस;
  btreeLockCarefully(p);
}

/* This is a helper function for sqlite3BtreeLock(). By moving
** complex, but seldom used logic, out of sqlite3BtreeLock() and
** into this routine, we avoid unnecessary stack pointer changes
** and thus help the sqlite3BtreeLock() routine to run much faster
** in the common case.
*/
अटल व्योम SQLITE_NOINLINE btreeLockCarefully(Btree *p){
  Btree *pLater;

  /* In most cases, we should be able to acquire the lock we
  ** want without having to go through the ascending lock
  ** procedure that follows.  Just be sure not to block.
  */
  अगर( sqlite3_mutex_try(p->pBt->mutex)==SQLITE_OK ){
    p->pBt->db = p->db;
    p->locked = 1;
    वापस;
  }

  /* To avoid deadlock, first release all locks with a larger
  ** BtShared address.  Then acquire our lock.  Then reacquire
  ** the other BtShared locks that we used to hold in ascending
  ** order.
  */
  क्रम(pLater=p->p_अग्र; pLater; pLater=pLater->p_अग्र){
    निश्चित( pLater->sharable );
    निश्चित( pLater->p_अग्र==0 || pLater->p_अग्र->pBt>pLater->pBt );
    निश्चित( !pLater->locked || pLater->wantToLock>0 );
    अगर( pLater->locked ){
      unlockBtreeMutex(pLater);
    }
  }
  lockBtreeMutex(p);
  क्रम(pLater=p->p_अग्र; pLater; pLater=pLater->p_अग्र){
    अगर( pLater->wantToLock ){
      lockBtreeMutex(pLater);
    }
  }
}


/*
** Exit the recursive mutex on a Btree.
*/
व्योम sqlite3BtreeLeave(Btree *p){
  निश्चित( sqlite3_mutex_held(p->db->mutex) );
  अगर( p->sharable ){
    निश्चित( p->wantToLock>0 );
    p->wantToLock--;
    अगर( p->wantToLock==0 ){
      unlockBtreeMutex(p);
    }
  }
}

#अगर_अघोषित न_संशोधन
/*
** Return true if the BtShared mutex is held on the btree, or if the
** B-Tree is not marked as sharable.
**
** This routine is used only from within assert() statements.
*/
पूर्णांक sqlite3BtreeHoldsMutex(Btree *p){
  निश्चित( p->sharable==0 || p->locked==0 || p->wantToLock>0 );
  निश्चित( p->sharable==0 || p->locked==0 || p->db==p->pBt->db );
  निश्चित( p->sharable==0 || p->locked==0 || sqlite3_mutex_held(p->pBt->mutex) );
  निश्चित( p->sharable==0 || p->locked==0 || sqlite3_mutex_held(p->db->mutex) );

  वापस (p->sharable==0 || p->locked);
}
#पूर्ण_अगर


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
अटल व्योम SQLITE_NOINLINE btreeEnterAll(sqlite3 *db){
  पूर्णांक i;
  u8 skipOk = 1;
  Btree *p;
  निश्चित( sqlite3_mutex_held(db->mutex) );
  क्रम(i=0; i<db->nDb; i++){
    p = db->aDb[i].pBt;
    अगर( p && p->sharable ){
      sqlite3BtreeEnter(p);
      skipOk = 0;
    }
  }
  db->noSharedCache = skipOk;
}
व्योम sqlite3BtreeEnterAll(sqlite3 *db){
  अगर( db->noSharedCache==0 ) btreeEnterAll(db);
}
अटल व्योम SQLITE_NOINLINE btreeLeaveAll(sqlite3 *db){
  पूर्णांक i;
  Btree *p;
  निश्चित( sqlite3_mutex_held(db->mutex) );
  क्रम(i=0; i<db->nDb; i++){
    p = db->aDb[i].pBt;
    अगर( p ) sqlite3BtreeLeave(p);
  }
}
व्योम sqlite3BtreeLeaveAll(sqlite3 *db){
  अगर( db->noSharedCache==0 ) btreeLeaveAll(db);
}

#अगर_अघोषित न_संशोधन
/*
** Return true if the current thread holds the database connection
** mutex and all required BtShared mutexes.
**
** This routine is used inside assert() statements only.
*/
पूर्णांक sqlite3BtreeHoldsAllMutexes(sqlite3 *db){
  पूर्णांक i;
  अगर( !sqlite3_mutex_held(db->mutex) ){
    वापस 0;
  }
  क्रम(i=0; i<db->nDb; i++){
    Btree *p;
    p = db->aDb[i].pBt;
    अगर( p && p->sharable &&
         (p->wantToLock==0 || !sqlite3_mutex_held(p->pBt->mutex)) ){
      वापस 0;
    }
  }
  वापस 1;
}
#पूर्ण_अगर /* NDEBUG */

#अगर_अघोषित न_संशोधन
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
पूर्णांक sqlite3SchemaMutexHeld(sqlite3 *db, पूर्णांक iDb, Schema *pSchema){
  Btree *p;
  निश्चित( db!=0 );
  अगर( db->pVfs==0 && db->nDb==0 ) वापस 1;
  अगर( pSchema ) iDb = sqlite3_Schema_To_सूचकांक(db, pSchema);
  निश्चित( iDb>=0 && iDb<db->nDb );
  अगर( !sqlite3_mutex_held(db->mutex) ) वापस 0;
  अगर( iDb==1 ) वापस 1;
  p = db->aDb[iDb].pBt;
  निश्चित( p!=0 );
  वापस p->sharable==0 || p->locked==1;
}
#पूर्ण_अगर /* NDEBUG */

#अन्यथा /* SQLITE_THREADSAFE>0 above.  SQLITE_THREADSAFE==0 below */
/*
** The following are special cases for mutex enter routines for use
** in single threaded applications that use shared cache.  Except for
** these two routines, all mutex operations are no-ops in that case and
** are null #defines in btree.h.
**
** If shared cache is disabled, then all btree mutex routines, including
** the ones below, are no-ops and are null #defines in btree.h.
*/

व्योम sqlite3BtreeEnter(Btree *p){
  p->pBt->db = p->db;
}
व्योम sqlite3BtreeEnterAll(sqlite3 *db){
  पूर्णांक i;
  u8 skipOk = 1;
  /* As in the threadsafe build: set db->noSharedCache when no attached
  ** Btree is sharable, so that schema and table-lock checks are skipped */
  अगर( db->noSharedCache ) वापस;
  क्रम(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    अगर( p ){
      p->pBt->db = p->db;
      अगर( p->sharable ) skipOk = 0;
    }
  }
  db->noSharedCache = skipOk;
}
#पूर्ण_अगर /* if SQLITE_THREADSAFE */

#अगर_अघोषित SQLITE_OMIT_INCRBLOB
/*
** Enter a mutex on a Btree given a cursor owned by that Btree. 
**
** These entry points are used by incremental I/O only. Enter() is required 
** any time OMIT_SHARED_CACHE is not defined, regardless of whether or not 
** the build is threadsafe. Leave() is only required by threadsafe builds.
*/
व्योम sqlite3BtreeEnterCursor(BtCursor *pCur){
  sqlite3BtreeEnter(pCur->pBtree);
}
# अगर SQLITE_THREADSAFE
व्योम sqlite3BtreeLeaveCursor(BtCursor *pCur){
  sqlite3BtreeLeave(pCur->pBtree);
}
# endif
#पूर्ण_अगर /* ifndef SQLITE_OMIT_INCRBLOB */

#पूर्ण_अगर /* ifndef SQLITE_OMIT_SHARED_CACHE */
