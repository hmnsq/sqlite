/*
** 2007 August 28
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
** This file contains the common header for all mutex implementations.
** The sqliteInt.h header #includes this file so that it is available
** to all source files.  We break it out in an effort to keep the code
** better organized.
**
** NOTE:  source files should *not* #include this header file directly.
** Source files should #include the sqliteInt.h file and let that file
** include this one indirectly.
*/


/*
** Figure out what version of the code to use.  The choices are
**
**   SQLITE_MUTEX_OMIT         No mutex logic.  Not even stubs.  The
**                             mutexes implementation cannot be overridden
**                             at start-time.
**
**   SQLITE_MUTEX_NOOP         For single-threaded applications.  No
**                             mutual exclusion is provided.  But this
**                             implementation can be overridden at
**                             start-time.
**
**   SQLITE_MUTEX_PTHREADS     For multi-threaded applications on Unix.
**
**   SQLITE_MUTEX_W32          For multi-threaded applications on Win32.
*/
#अगर !SQLITE_THREADSAFE
# define SQLITE_MUTEX_OMIT
#पूर्ण_अगर
#अगर SQLITE_THREADSAFE && !defined(SQLITE_MUTEX_NOOP)
#  अगर SQLITE_OS_UNIX || defined(__CYGWIN__)
#    define SQLITE_MUTEX_PTHREADS
#  elif SQLITE_OS_WIN
#    define SQLITE_MUTEX_W32
#  अन्यथा
#    define SQLITE_MUTEX_NOOP
#  endif
#पूर्ण_अगर

#अगर_घोषित SQLITE_MUTEX_OMIT
/*
** If this is a no-op implementation, implement everything as macros.
*/
#घोषणा sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#घोषणा sqlite3_mutex_free(X)
#घोषणा sqlite3_mutex_enter(X)    
#घोषणा sqlite3_mutex_try(X)      SQLITE_OK
#घोषणा sqlite3_mutex_leave(X)    
#घोषणा sqlite3_mutex_held(X)     ((व्योम)(X),1)
#घोषणा sqlite3_mutex_notheld(X)  ((व्योम)(X),1)
#घोषणा sqlite3MutexAlloc(X)      ((sqlite3_mutex*)8)
#घोषणा sqlite3MutexInit()        SQLITE_OK
#घोषणा sqlite3MutexEnd()
#घोषणा MUTEX_LOGIC(X)
#अन्यथा
#घोषणा MUTEX_LOGIC(X)            X
पूर्णांक sqlite3_mutex_held(sqlite3_mutex*);
#पूर्ण_अगर /* defined(SQLITE_MUTEX_OMIT) */
