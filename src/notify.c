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
#شامل "sqliteInt.h"
#شامل "btreeInt.h"

/* Omit this entire file if SQLITE_ENABLE_UNLOCK_NOTIFY is not defined. */
#اگر_تعریف_شدہ SQLITE_ENABLE_UNLOCK_NOTIFY

/*
** Public interfaces:
**
**   sqlite3ConnectionBlocked()
**   sqlite3ConnectionUnlocked()
**   sqlite3ConnectionClosed()
**   sqlite3_unlock_notify()
*/

#تعریف_کرو assertMutexHeld() \
  أكد( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN)) )

/*
** Head of a linked list of all sqlite3 objects created by this process
** for which either sqlite3.pBlockingConnection or sqlite3.pUnlockConnection
** is not NULL. This variable may only accessed while the STATIC_MAIN
** mutex is held.
*/
ساکن sqlite3 *SQLITE_WSD sqlite3BlockedList = 0;

#اگر_غیر_تعریف_شدہ NDEBUG
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
ساکن خالی checkListProperties(sqlite3 *db){
  sqlite3 *p;
  برائے(p=sqlite3BlockedList; p; p=p->p_اگلا_Blocked){
    صحیح نظر_آیا = 0;
    sqlite3 *p2;

    /* Verify property (1) */
    أكد( p->pUnlockConnection || p->pBlockingConnection );

    /* Verify property (2) */
    برائے(p2=sqlite3BlockedList; p2!=p; p2=p2->p_اگلا_Blocked){
      اگر( p2->xUnlockNotify==p->xUnlockNotify ) نظر_آیا = 1;
      أكد( p2->xUnlockNotify==p->xUnlockNotify || !نظر_آیا );
      أكد( db==0 || p->pUnlockConnection!=db );
      أكد( db==0 || p->pBlockingConnection!=db );
    }
  }
}
#ورنہ
# define checkListProperties(x)
#اگر_ختم

/*
** Remove connection db from the blocked connections list. If connection
** db is not currently a part of the list, this function is a no-op.
*/
ساکن خالی removeFromBlockedList(sqlite3 *db){
  sqlite3 **pp;
  assertMutexHeld();
  برائے(pp=&sqlite3BlockedList; *pp; pp = &(*pp)->p_اگلا_Blocked){
    اگر( *pp==db ){
      *pp = (*pp)->p_اگلا_Blocked;
      توڑو;
    }
  }
}

/*
** Add connection db to the blocked connections list. It is assumed
** that it is not already a part of the list.
*/
ساکن خالی جوڑو_مقصد_Blocked_List(sqlite3 *db){
  sqlite3 **pp;
  assertMutexHeld();
  برائے(
    pp=&sqlite3BlockedList; 
    *pp && (*pp)->xUnlockNotify!=db->xUnlockNotify; 
    pp=&(*pp)->p_اگلا_Blocked
  );
  db->p_اگلا_Blocked = *pp;
  *pp = db;
}

/*
** Obtain the STATIC_MAIN mutex.
*/
ساکن خالی enterMutex(خالی){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN));
  checkListProperties(0);
}

/*
** Release the STATIC_MAIN mutex.
*/
ساکن خالی leaveMutex(خالی){
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
صحیح sqlite3_unlock_notify(
  sqlite3 *db,
  خالی (*xNotify)(خالی **, صحیح),
  خالی *pArg
){
  صحیح rc = SQLITE_OK;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);
  enterMutex();

  اگر( xNotify==0 ){
    removeFromBlockedList(db);
    db->pBlockingConnection = 0;
    db->pUnlockConnection = 0;
    db->xUnlockNotify = 0;
    db->pUnlockArg = 0;
  }ورنہ اگر( 0==db->pBlockingConnection ){
    /* The blocking transaction has been concluded. Or there never was a 
    ** blocking transaction. In either case, invoke the notify callback
    ** immediately. 
    */
    xNotify(&pArg, 1);
  }ورنہ{
    sqlite3 *p;

    برائے(p=db->pBlockingConnection; p && p!=db; p=p->pUnlockConnection){}
    اگر( p ){
      rc = SQLITE_LOCKED;              /* Deadlock detected. */
    }ورنہ{
      db->pUnlockConnection = db->pBlockingConnection;
      db->xUnlockNotify = xNotify;
      db->pUnlockArg = pArg;
      removeFromBlockedList(db);
      جوڑو_مقصد_Blocked_List(db);
    }
  }

  leaveMutex();
  أكد( !db->mallocFailed );
  sqlite3_Error_With_پیغام(db, rc, (rc?"database is deadlocked":0));
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

/*
** This function is called while stepping or preparing a statement 
** associated with connection db. The operation will return SQLITE_LOCKED
** to the user because it requires a lock that will not be available
** until connection pBlocker concludes its current transaction.
*/
خالی sqlite3ConnectionBlocked(sqlite3 *db, sqlite3 *pBlocker){
  enterMutex();
  اگر( db->pBlockingConnection==0 && db->pUnlockConnection==0 ){
    جوڑو_مقصد_Blocked_List(db);
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
خالی sqlite3ConnectionUnlocked(sqlite3 *db){
  خالی (*xUnlockNotify)(خالی **, صحیح) = 0; /* Unlock-notify cb to invoke */
  صحیح nArg = 0;                            /* Number of entries in aArg[] */
  sqlite3 **pp;                            /* Iterator variable */
  خالی **aArg;               /* Arguments to the unlock callback */
  خالی **aDyn = 0;           /* Dynamically allocated space for aArg[] */
  خالی *aStatic[16];         /* Starter space for aArg[].  No malloc required */

  aArg = aStatic;
  enterMutex();         /* Enter STATIC_MAIN mutex */

  /* This loop runs once for each entry in the blocked-connections list. */
  برائے(pp=&sqlite3BlockedList; *pp; /* no-op */ ){
    sqlite3 *p = *pp;

    /* Step 1. */
    اگر( p->pBlockingConnection==db ){
      p->pBlockingConnection = 0;
    }

    /* Step 2. */
    اگر( p->pUnlockConnection==db ){
      أكد( p->xUnlockNotify );
      اگر( p->xUnlockNotify!=xUnlockNotify && nArg!=0 ){
        xUnlockNotify(aArg, nArg);
        nArg = 0;
      }

      sqlite3BeginBenignMalloc();
      أكد( aArg==aDyn || (aDyn==0 && aArg==aStatic) );
      أكد( nArg<=(صحیح)Array_سائز(aStatic) || aArg==aDyn );
      اگر( (!aDyn && nArg==(صحیح)Array_سائز(aStatic))
       || (aDyn && nArg==(صحیح)(sqlite3_Malloc_سائز(aDyn)/حجم(خالی*)))
      ){
        /* The aArg[] array needs to grow. */
        خالی **pNew = (خالی **)sqlite3Malloc(nArg*حجم(خالی *)*2);
        اگر( pNew ){
          حافظہ_نقل(pNew, aArg, nArg*حجم(خالی *));
          sqlite3_free(aDyn);
          aDyn = aArg = pNew;
        }ورنہ{
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
      sqlite3_انتہا_Benign_Malloc();

      aArg[nArg++] = p->pUnlockArg;
      xUnlockNotify = p->xUnlockNotify;
      p->pUnlockConnection = 0;
      p->xUnlockNotify = 0;
      p->pUnlockArg = 0;
    }

    /* Step 3. */
    اگر( p->pBlockingConnection==0 && p->pUnlockConnection==0 ){
      /* Remove connection p from the blocked connections list. */
      *pp = p->p_اگلا_Blocked;
      p->p_اگلا_Blocked = 0;
    }ورنہ{
      pp = &p->p_اگلا_Blocked;
    }
  }

  اگر( nArg!=0 ){
    xUnlockNotify(aArg, nArg);
  }
  sqlite3_free(aDyn);
  leaveMutex();         /* Leave STATIC_MAIN mutex */
}

/*
** This is called when the database connection passed as an argument is 
** being closed. The connection is removed from the blocked list.
*/
خالی sqlite3ConnectionClosed(sqlite3 *db){
  sqlite3ConnectionUnlocked(db);
  enterMutex();
  removeFromBlockedList(db);
  checkListProperties(db);
  leaveMutex();
}
#اگر_ختم
