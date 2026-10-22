/*
** 2008 Jan 22
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
** This file contains code to support the concept of "benign" 
** malloc failures (when the xMalloc() or xRealloc() method of the
** sqlite3_mem_methods structure fails to allocate a block of memory
** and returns 0). 
**
** Most malloc failures are non-benign. After they occur, SQLite
** abandons the current operation and returns an error code (usually
** SQLITE_NOMEM) to the user. However, sometimes a fault is not necessarily
** fatal. For example, if a malloc fails while resizing a hash table, this 
** is completely recoverable simply by not carrying out the resize. The 
** hash table will continue to function normally.  So a malloc failure 
** during a hash table resize is a benign fault.
*/

#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_UNTESTABLE

/*
** Global variables.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ BenignMallocHooks BenignMallocHooks;
ਸਥਿਰ SQLITE_WSD ਬਣਤਰ BenignMallocHooks {
  ਖਾਲੀ (*xBenignBegin)(ਖਾਲੀ);
  ਖਾਲੀ (*x_Benign_ਸਿਰਾ)(ਖਾਲੀ);
} sqlite3Hooks = { 0, 0 };

/* The "wsdHooks" macro will resolve to the appropriate BenignMallocHooks
** structure.  If writable static data is unsupported on the target,
** we have to locate the state vector at run-time.  In the more common
** case where writable static data is supported, wsdHooks can refer directly
** to the "sqlite3Hooks" state vector declared above.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WSD
# define wsdHooksInit \
  BenignMallocHooks *x = &GLOBAL(BenignMallocHooks,sqlite3Hooks)
# define wsdHooks x[0]
#ਨਹੀਂਤਾਂ
# define wsdHooksInit
# define wsdHooks sqlite3Hooks
#ਜੇ_ਅੰਤ


/*
** Register hooks to call when sqlite3BeginBenignMalloc() and
** sqlite3EndBenignMalloc() are called, respectively.
*/
ਖਾਲੀ sqlite3BenignMallocHooks(
  ਖਾਲੀ (*xBenignBegin)(ਖਾਲੀ),
  ਖਾਲੀ (*x_Benign_ਸਿਰਾ)(ਖਾਲੀ)
){
  wsdHooksInit;
  wsdHooks.xBenignBegin = xBenignBegin;
  wsdHooks.x_Benign_ਸਿਰਾ = x_Benign_ਸਿਰਾ;
}

/*
** This (sqlite3EndBenignMalloc()) is called by SQLite code to indicate that
** subsequent malloc failures are benign. A call to sqlite3EndBenignMalloc()
** indicates that subsequent malloc failures are non-benign.
*/
ਖਾਲੀ sqlite3BeginBenignMalloc(ਖਾਲੀ){
  wsdHooksInit;
  ਜੇ( wsdHooks.xBenignBegin ){
    wsdHooks.xBenignBegin();
  }
}
ਖਾਲੀ sqlite3_ਸਿਰਾ_Benign_Malloc(ਖਾਲੀ){
  wsdHooksInit;
  ਜੇ( wsdHooks.x_Benign_ਸਿਰਾ ){
    wsdHooks.x_Benign_ਸਿਰਾ();
  }
}

#ਜੇ_ਅੰਤ   /* #ifndef SQLITE_UNTESTABLE */
