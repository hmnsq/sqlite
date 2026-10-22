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

#समावेश "sqliteInt.h"

#अगर_अघोषित SQLITE_UNTESTABLE

/*
** Global variables.
*/
प्रकार काष्ठा BenignMallocHooks BenignMallocHooks;
अटल SQLITE_WSD काष्ठा BenignMallocHooks {
  व्योम (*xBenignBegin)(व्योम);
  व्योम (*xBenignEnd)(व्योम);
} sqlite3Hooks = { 0, 0 };

/* The "wsdHooks" macro will resolve to the appropriate BenignMallocHooks
** structure.  If writable static data is unsupported on the target,
** we have to locate the state vector at run-time.  In the more common
** case where writable static data is supported, wsdHooks can refer directly
** to the "sqlite3Hooks" state vector declared above.
*/
#अगर_घोषित SQLITE_OMIT_WSD
# define wsdHooksInit \
  BenignMallocHooks *x = &GLOBAL(BenignMallocHooks,sqlite3Hooks)
# define wsdHooks x[0]
#अन्यथा
# define wsdHooksInit
# define wsdHooks sqlite3Hooks
#पूर्ण_अगर


/*
** Register hooks to call when sqlite3BeginBenignMalloc() and
** sqlite3EndBenignMalloc() are called, respectively.
*/
व्योम sqlite3BenignMallocHooks(
  व्योम (*xBenignBegin)(व्योम),
  व्योम (*xBenignEnd)(व्योम)
){
  wsdHooksInit;
  wsdHooks.xBenignBegin = xBenignBegin;
  wsdHooks.xBenignEnd = xBenignEnd;
}

/*
** This (sqlite3EndBenignMalloc()) is called by SQLite code to indicate that
** subsequent malloc failures are benign. A call to sqlite3EndBenignMalloc()
** indicates that subsequent malloc failures are non-benign.
*/
व्योम sqlite3BeginBenignMalloc(व्योम){
  wsdHooksInit;
  अगर( wsdHooks.xBenignBegin ){
    wsdHooks.xBenignBegin();
  }
}
व्योम sqlite3EndBenignMalloc(व्योम){
  wsdHooksInit;
  अगर( wsdHooks.xBenignEnd ){
    wsdHooks.xBenignEnd();
  }
}

#पूर्ण_अगर   /* #ifndef SQLITE_UNTESTABLE */
