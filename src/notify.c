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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "btreeInt.h"

/* Omit this entire file if SQLITE_ENABLE_UNLOCK_NOTIFY is not defined. */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UNLOCK_NOTIFY

/*
** Public interfaces:
**
**   sqlite3ConnectionBlocked()
**   sqlite3ConnectionUnlocked()
**   sqlite3ConnectionClosed()
**   sqlite3_unlock_notify()
*/

#ವ್ಯಾಖ್ಯಾನಿಸು assertMutexHeld() \
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) )

/*
** Head of a linked list of all sqlite3 objects created by this process
** for which either sqlite3.pBlockingConnection or sqlite3.pUnlockConnection
** is not NULL. This variable may only accessed while the STATIC_MAIN
** mutex is held.
*/
ಸ್ಥಿರ sqlite3 *SQLITE_WSD sqlite3BlockedList = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
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
ಸ್ಥಿರ ಶೂನ್ಯ checkListProperties(sqlite3 *db){
  sqlite3 *p;
  ಗಾಗಿ(p=sqlite3BlockedList; p; p=p->p_ಮುಂದಿನ_Blocked){
    ಪೂರ್ಣಾಂಕ ನೋಡಿದ = 0;
    sqlite3 *p2;

    /* Verify property (1) */
    ಪ್ರತಿಜ್ಞಾ( p->pUnlockConnection || p->pBlockingConnection );

    /* Verify property (2) */
    ಗಾಗಿ(p2=sqlite3BlockedList; p2!=p; p2=p2->p_ಮುಂದಿನ_Blocked){
      ಆದರೆ( p2->xUnlockNotify==p->xUnlockNotify ) ನೋಡಿದ = 1;
      ಪ್ರತಿಜ್ಞಾ( p2->xUnlockNotify==p->xUnlockNotify || !ನೋಡಿದ );
      ಪ್ರತಿಜ್ಞಾ( db==0 || p->pUnlockConnection!=db );
      ಪ್ರತಿಜ್ಞಾ( db==0 || p->pBlockingConnection!=db );
    }
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define checkListProperties(x)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Remove connection db from the blocked connections list. If connection
** db is not currently a part of the list, this function is a no-op.
*/
ಸ್ಥಿರ ಶೂನ್ಯ removeFromBlockedList(sqlite3 *db){
  sqlite3 **pp;
  assertMutexHeld();
  ಗಾಗಿ(pp=&sqlite3BlockedList; *pp; pp = &(*pp)->p_ಮುಂದಿನ_Blocked){
    ಆದರೆ( *pp==db ){
      *pp = (*pp)->p_ಮುಂದಿನ_Blocked;
      ನಿಲ್ಲಿಸು;
    }
  }
}

/*
** Add connection db to the blocked connections list. It is assumed
** that it is not already a part of the list.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಕೂಡಿಸು_ಗಮ್ಯ_Blocked_List(sqlite3 *db){
  sqlite3 **pp;
  assertMutexHeld();
  ಗಾಗಿ(
    pp=&sqlite3BlockedList; 
    *pp && (*pp)->xUnlockNotify!=db->xUnlockNotify; 
    pp=&(*pp)->p_ಮುಂದಿನ_Blocked
  );
  db->p_ಮುಂದಿನ_Blocked = *pp;
  *pp = db;
}

/*
** Obtain the STATIC_MAIN mutex.
*/
ಸ್ಥಿರ ಶೂನ್ಯ enterMutex(ಶೂನ್ಯ){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN));
  checkListProperties(0);
}

/*
** Release the STATIC_MAIN mutex.
*/
ಸ್ಥಿರ ಶೂನ್ಯ leaveMutex(ಶೂನ್ಯ){
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
ಪೂರ್ಣಾಂಕ sqlite3_unlock_notify(
  sqlite3 *db,
  ಶೂನ್ಯ (*xNotify)(ಶೂನ್ಯ **, ಪೂರ್ಣಾಂಕ),
  ಶೂನ್ಯ *pArg
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);
  enterMutex();

  ಆದರೆ( xNotify==0 ){
    removeFromBlockedList(db);
    db->pBlockingConnection = 0;
    db->pUnlockConnection = 0;
    db->xUnlockNotify = 0;
    db->pUnlockArg = 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( 0==db->pBlockingConnection ){
    /* The blocking transaction has been concluded. Or there never was a 
    ** blocking transaction. In either case, invoke the notify callback
    ** immediately. 
    */
    xNotify(&pArg, 1);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3 *p;

    ಗಾಗಿ(p=db->pBlockingConnection; p && p!=db; p=p->pUnlockConnection){}
    ಆದರೆ( p ){
      rc = SQLITE_LOCKED;              /* Deadlock detected. */
    }ಇಲ್ಲದಿದ್ದರೆ{
      db->pUnlockConnection = db->pBlockingConnection;
      db->xUnlockNotify = xNotify;
      db->pUnlockArg = pArg;
      removeFromBlockedList(db);
      ಕೂಡಿಸು_ಗಮ್ಯ_Blocked_List(db);
    }
  }

  leaveMutex();
  ಪ್ರತಿಜ್ಞಾ( !db->mallocFailed );
  sqlite3_Error_With_ಸಂದೇಶ(db, rc, (rc?"database is deadlocked":0));
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This function is called while stepping or preparing a statement 
** associated with connection db. The operation will return SQLITE_LOCKED
** to the user because it requires a lock that will not be available
** until connection pBlocker concludes its current transaction.
*/
ಶೂನ್ಯ sqlite3ConnectionBlocked(sqlite3 *db, sqlite3 *pBlocker){
  enterMutex();
  ಆದರೆ( db->pBlockingConnection==0 && db->pUnlockConnection==0 ){
    ಕೂಡಿಸು_ಗಮ್ಯ_Blocked_List(db);
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
ಶೂನ್ಯ sqlite3ConnectionUnlocked(sqlite3 *db){
  ಶೂನ್ಯ (*xUnlockNotify)(ಶೂನ್ಯ **, ಪೂರ್ಣಾಂಕ) = 0; /* Unlock-notify cb to invoke */
  ಪೂರ್ಣಾಂಕ nArg = 0;                            /* Number of entries in aArg[] */
  sqlite3 **pp;                            /* Iterator variable */
  ಶೂನ್ಯ **aArg;               /* Arguments to the unlock callback */
  ಶೂನ್ಯ **aDyn = 0;           /* Dynamically allocated space for aArg[] */
  ಶೂನ್ಯ *aStatic[16];         /* Starter space for aArg[].  No malloc required */

  aArg = aStatic;
  enterMutex();         /* Enter STATIC_MAIN mutex */

  /* This loop runs once for each entry in the blocked-connections list. */
  ಗಾಗಿ(pp=&sqlite3BlockedList; *pp; /* no-op */ ){
    sqlite3 *p = *pp;

    /* Step 1. */
    ಆದರೆ( p->pBlockingConnection==db ){
      p->pBlockingConnection = 0;
    }

    /* Step 2. */
    ಆದರೆ( p->pUnlockConnection==db ){
      ಪ್ರತಿಜ್ಞಾ( p->xUnlockNotify );
      ಆದರೆ( p->xUnlockNotify!=xUnlockNotify && nArg!=0 ){
        xUnlockNotify(aArg, nArg);
        nArg = 0;
      }

      sqlite3BeginBenignMalloc();
      ಪ್ರತಿಜ್ಞಾ( aArg==aDyn || (aDyn==0 && aArg==aStatic) );
      ಪ್ರತಿಜ್ಞಾ( nArg<=(ಪೂರ್ಣಾಂಕ)Array_ಗಾತ್ರ(aStatic) || aArg==aDyn );
      ಆದರೆ( (!aDyn && nArg==(ಪೂರ್ಣಾಂಕ)Array_ಗಾತ್ರ(aStatic))
       || (aDyn && nArg==(ಪೂರ್ಣಾಂಕ)(sqlite3_Malloc_ಗಾತ್ರ(aDyn)/ಗಾತ್ರ(ಶೂನ್ಯ*)))
      ){
        /* The aArg[] array needs to grow. */
        ಶೂನ್ಯ **pNew = (ಶೂನ್ಯ **)sqlite3Malloc(nArg*ಗಾತ್ರ(ಶೂನ್ಯ *)*2);
        ಆದರೆ( pNew ){
          ಸ್ಮರಣೆ_ನಕಲು(pNew, aArg, nArg*ಗಾತ್ರ(ಶೂನ್ಯ *));
          sqlite3_free(aDyn);
          aDyn = aArg = pNew;
        }ಇಲ್ಲದಿದ್ದರೆ{
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
      sqlite3_ಕೊನೆ_Benign_Malloc();

      aArg[nArg++] = p->pUnlockArg;
      xUnlockNotify = p->xUnlockNotify;
      p->pUnlockConnection = 0;
      p->xUnlockNotify = 0;
      p->pUnlockArg = 0;
    }

    /* Step 3. */
    ಆದರೆ( p->pBlockingConnection==0 && p->pUnlockConnection==0 ){
      /* Remove connection p from the blocked connections list. */
      *pp = p->p_ಮುಂದಿನ_Blocked;
      p->p_ಮುಂದಿನ_Blocked = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pp = &p->p_ಮುಂದಿನ_Blocked;
    }
  }

  ಆದರೆ( nArg!=0 ){
    xUnlockNotify(aArg, nArg);
  }
  sqlite3_free(aDyn);
  leaveMutex();         /* Leave STATIC_MAIN mutex */
}

/*
** This is called when the database connection passed as an argument is 
** being closed. The connection is removed from the blocked list.
*/
ಶೂನ್ಯ sqlite3ConnectionClosed(sqlite3 *db){
  sqlite3ConnectionUnlocked(db);
  enterMutex();
  removeFromBlockedList(db);
  checkListProperties(db);
  leaveMutex();
}
#ಆದರೆ_ಅಂತ್ಯ
