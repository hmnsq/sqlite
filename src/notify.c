/*
** 2009 March 3
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
** This file contains the implementation of the sqlite3_unlock_notify()
** API method and its associated functionality.
*/
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "btreeInt.h"

/* Omit this entire file if SQLITE_ENABLE_UNLOCK_NOTIFY is not defined. */
#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY

/*
** Public interfaces:
**
**   sqlite3ConnectionBlocked()
**   sqlite3ConnectionUnlocked()
**   sqlite3ConnectionClosed()
**   sqlite3_unlock_notify()
*/

#define assertMutexHeld() \
  പ്രതിജ്ഞാ( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) )

/*
** Head of a linked list of all sqlite3 objects created by this process
** for which either sqlite3.pBlockingConnection or sqlite3.pUnlockConnection
** is not NULL. This variable may only accessed while the STATIC_MAIN
** mutex is held.
*/
സ്ഥിതം sqlite3 *SQLITE_WSD sqlite3BlockedList = 0;

#ifndef NDEBUG
/*
** This function is a complex assert() that verifies the following 
** properties of the blocked connections list:
**
**   1) Each entry in the list has a non-NULL value for either 
**      pUnlockConnection or pBlockingConnection, or both.
**
**   2) All entries in the list that share a common value for 
**      xUnlockNotify are grouped together.
**
**   3) If the argument db is not NULL, then none of the entries in the
**      blocked connections list have pUnlockConnection or pBlockingConnection
**      set to db. This is used when closing connection db.
*/
സ്ഥിതം ശൂന്യം checkListProperties(sqlite3 *db){
  sqlite3 *p;
  വേണ്ടി(p=sqlite3BlockedList; p; p=p->pNextBlocked){
    പൂർണ്ണസംഖ്യ seen = 0;
    sqlite3 *p2;

    /* Verify property (1) */
    പ്രതിജ്ഞാ( p->pUnlockConnection || p->pBlockingConnection );

    /* Verify property (2) */
    വേണ്ടി(p2=sqlite3BlockedList; p2!=p; p2=p2->pNextBlocked){
      എങ്കിൽ( p2->xUnlockNotify==p->xUnlockNotify ) seen = 1;
      പ്രതിജ്ഞാ( p2->xUnlockNotify==p->xUnlockNotify || !seen );
      പ്രതിജ്ഞാ( db==0 || p->pUnlockConnection!=db );
      പ്രതിജ്ഞാ( db==0 || p->pBlockingConnection!=db );
    }
  }
}
#അല്ലെങ്കിൽ
# define checkListProperties(x)
#endif

/*
** Remove connection db from the blocked connections list. If connection
** db is not currently a part of the list, this function is a no-op.
*/
സ്ഥിതം ശൂന്യം removeFromBlockedList(sqlite3 *db){
  sqlite3 **pp;
  assertMutexHeld();
  വേണ്ടി(pp=&sqlite3BlockedList; *pp; pp = &(*pp)->pNextBlocked){
    എങ്കിൽ( *pp==db ){
      *pp = (*pp)->pNextBlocked;
      നിർത്തുക;
    }
  }
}

/*
** Add connection db to the blocked connections list. It is assumed
** that it is not already a part of the list.
*/
സ്ഥിതം ശൂന്യം addToBlockedList(sqlite3 *db){
  sqlite3 **pp;
  assertMutexHeld();
  വേണ്ടി(
    pp=&sqlite3BlockedList; 
    *pp && (*pp)->xUnlockNotify!=db->xUnlockNotify; 
    pp=&(*pp)->pNextBlocked
  );
  db->pNextBlocked = *pp;
  *pp = db;
}

/*
** Obtain the STATIC_MAIN mutex.
*/
സ്ഥിതം ശൂന്യം enterMutex(ശൂന്യം){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN));
  checkListProperties(0);
}

/*
** Release the STATIC_MAIN mutex.
*/
സ്ഥിതം ശൂന്യം leaveMutex(ശൂന്യം){
  assertMutexHeld();
  checkListProperties(0);
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN));
}

/*
** Register an unlock-notify callback.
**
** This is called after connection "db" has attempted some operation
** but has received an SQLITE_LOCKED error because another connection
** (call it pOther) in the same process was busy using the same shared
** cache.  pOther is found by looking at db->pBlockingConnection.
**
** If there is no blocking connection, the callback is invoked immediately,
** before this routine returns.
**
** If pOther is already blocked on db, then report SQLITE_LOCKED, to indicate
** a deadlock.
**
** Otherwise, make arrangements to invoke xNotify when pOther drops
** its locks.
**
** Each call to this routine overrides any prior callbacks registered
** on the same "db".  If xNotify==0 then any prior callbacks are immediately
** cancelled.
*/
പൂർണ്ണസംഖ്യ sqlite3_unlock_notify(
  sqlite3 *db,
  ശൂന്യം (*xNotify)(ശൂന്യം **, പൂർണ്ണസംഖ്യ),
  ശൂന്യം *pArg
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;

#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( !sqlite3SafetyCheckOk(db) ) തിരികെ SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  enterMutex();

  എങ്കിൽ( xNotify==0 ){
    removeFromBlockedList(db);
    db->pBlockingConnection = 0;
    db->pUnlockConnection = 0;
    db->xUnlockNotify = 0;
    db->pUnlockArg = 0;
  }അല്ലെങ്കിൽ എങ്കിൽ( 0==db->pBlockingConnection ){
    /* The blocking transaction has been concluded. Or there never was a 
    ** blocking transaction. In either case, invoke the notify callback
    ** immediately. 
    */
    xNotify(&pArg, 1);
  }അല്ലെങ്കിൽ{
    sqlite3 *p;

    വേണ്ടി(p=db->pBlockingConnection; p && p!=db; p=p->pUnlockConnection){}
    എങ്കിൽ( p ){
      rc = SQLITE_LOCKED;              /* Deadlock detected. */
    }അല്ലെങ്കിൽ{
      db->pUnlockConnection = db->pBlockingConnection;
      db->xUnlockNotify = xNotify;
      db->pUnlockArg = pArg;
      removeFromBlockedList(db);
      addToBlockedList(db);
    }
  }

  leaveMutex();
  പ്രതിജ്ഞാ( !db->mallocFailed );
  sqlite3_Error_With_സന്ദേശം(db, rc, (rc?"database is deadlocked":0));
  sqlite3_mutex_leave(db->mutex);
  തിരികെ rc;
}

/*
** This function is called while stepping or preparing a statement 
** associated with connection db. The operation will return SQLITE_LOCKED
** to the user because it requires a lock that will not be available
** until connection pBlocker concludes its current transaction.
*/
ശൂന്യം sqlite3ConnectionBlocked(sqlite3 *db, sqlite3 *pBlocker){
  enterMutex();
  എങ്കിൽ( db->pBlockingConnection==0 && db->pUnlockConnection==0 ){
    addToBlockedList(db);
  }
  db->pBlockingConnection = pBlocker;
  leaveMutex();
}

/*
** This function is called when
** the transaction opened by database db has just finished. Locks held 
** by database connection db have been released.
**
** This function loops through each entry in the blocked connections
** list and does the following:
**
**   1) If the sqlite3.pBlockingConnection member of a list entry is
**      set to db, then set pBlockingConnection=0.
**
**   2) If the sqlite3.pUnlockConnection member of a list entry is
**      set to db, then invoke the configured unlock-notify callback and
**      set pUnlockConnection=0.
**
**   3) If the two steps above mean that pBlockingConnection==0 and
**      pUnlockConnection==0, remove the entry from the blocked connections
**      list.
*/
ശൂന്യം sqlite3ConnectionUnlocked(sqlite3 *db){
  ശൂന്യം (*xUnlockNotify)(ശൂന്യം **, പൂർണ്ണസംഖ്യ) = 0; /* Unlock-notify cb to invoke */
  പൂർണ്ണസംഖ്യ nArg = 0;                            /* Number of entries in aArg[] */
  sqlite3 **pp;                            /* Iterator variable */
  ശൂന്യം **aArg;               /* Arguments to the unlock callback */
  ശൂന്യം **aDyn = 0;           /* Dynamically allocated space for aArg[] */
  ശൂന്യം *aStatic[16];         /* Starter space for aArg[].  No malloc required */

  aArg = aStatic;
  enterMutex();         /* Enter STATIC_MAIN mutex */

  /* This loop runs once for each entry in the blocked-connections list. */
  വേണ്ടി(pp=&sqlite3BlockedList; *pp; /* no-op */ ){
    sqlite3 *p = *pp;

    /* Step 1. */
    എങ്കിൽ( p->pBlockingConnection==db ){
      p->pBlockingConnection = 0;
    }

    /* Step 2. */
    എങ്കിൽ( p->pUnlockConnection==db ){
      പ്രതിജ്ഞാ( p->xUnlockNotify );
      എങ്കിൽ( p->xUnlockNotify!=xUnlockNotify && nArg!=0 ){
        xUnlockNotify(aArg, nArg);
        nArg = 0;
      }

      sqlite3BeginBenignMalloc();
      പ്രതിജ്ഞാ( aArg==aDyn || (aDyn==0 && aArg==aStatic) );
      പ്രതിജ്ഞാ( nArg<=(പൂർണ്ണസംഖ്യ)ArraySize(aStatic) || aArg==aDyn );
      എങ്കിൽ( (!aDyn && nArg==(പൂർണ്ണസംഖ്യ)ArraySize(aStatic))
       || (aDyn && nArg==(പൂർണ്ണസംഖ്യ)(sqlite3MallocSize(aDyn)/വലുപ്പം(ശൂന്യം*)))
      ){
        /* The aArg[] array needs to grow. */
        ശൂന്യം **pNew = (ശൂന്യം **)sqlite3Malloc(nArg*വലുപ്പം(ശൂന്യം *)*2);
        എങ്കിൽ( pNew ){
          memcpy(pNew, aArg, nArg*വലുപ്പം(ശൂന്യം *));
          sqlite3_free(aDyn);
          aDyn = aArg = pNew;
        }അല്ലെങ്കിൽ{
          /* This occurs when the array of context pointers that need to
          ** be passed to the unlock-notify callback is larger than the
          ** aStatic[] array allocated on the stack and the attempt to 
          ** allocate a larger array from the heap has failed.
          **
          ** This is a difficult situation to handle. Returning an error
          ** code to the caller is insufficient, as even if an error code
          ** is returned the transaction on connection db will still be
          ** closed and the unlock-notify callbacks on blocked connections
          ** will go unissued. This might cause the application to wait
          ** indefinitely for an unlock-notify callback that will never 
          ** arrive.
          **
          ** Instead, invoke the unlock-notify callback with the context
          ** array already accumulated. We can then clear the array and
          ** begin accumulating any further context pointers without 
          ** requiring any dynamic allocation. This is sub-optimal because
          ** it means that instead of one callback with a large array of
          ** context pointers the application will receive two or more
          ** callbacks with smaller arrays of context pointers, which will
          ** reduce the applications ability to prioritize multiple 
          ** connections. But it is the best that can be done under the
          ** circumstances.
          */
          xUnlockNotify(aArg, nArg);
          nArg = 0;
        }
      }
      sqlite3EndBenignMalloc();

      aArg[nArg++] = p->pUnlockArg;
      xUnlockNotify = p->xUnlockNotify;
      p->pUnlockConnection = 0;
      p->xUnlockNotify = 0;
      p->pUnlockArg = 0;
    }

    /* Step 3. */
    എങ്കിൽ( p->pBlockingConnection==0 && p->pUnlockConnection==0 ){
      /* Remove connection p from the blocked connections list. */
      *pp = p->pNextBlocked;
      p->pNextBlocked = 0;
    }അല്ലെങ്കിൽ{
      pp = &p->pNextBlocked;
    }
  }

  എങ്കിൽ( nArg!=0 ){
    xUnlockNotify(aArg, nArg);
  }
  sqlite3_free(aDyn);
  leaveMutex();         /* Leave STATIC_MAIN mutex */
}

/*
** This is called when the database connection passed as an argument is 
** being closed. The connection is removed from the blocked list.
*/
ശൂന്യം sqlite3ConnectionClosed(sqlite3 *db){
  sqlite3ConnectionUnlocked(db);
  enterMutex();
  removeFromBlockedList(db);
  checkListProperties(db);
  leaveMutex();
}
#endif
