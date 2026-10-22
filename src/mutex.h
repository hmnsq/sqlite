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
#إذا !SQLITE_THREADSAFE
# define SQLITE_MUTEX_OMIT
#جے_انت
#إذا SQLITE_THREADSAFE && !defined(SQLITE_MUTEX_NOOP)
#  إذا SQLITE_OS_UNIX || defined(__CYGWIN__)
#    define SQLITE_MUTEX_PTHREADS
#  elif SQLITE_OS_WIN
#    define SQLITE_MUTEX_W32
#  وإلا
#    define SQLITE_MUTEX_NOOP
#  endif
#جے_انت

#جے_تعریف_شدہ SQLITE_MUTEX_OMIT
/*
** If this is a no-op implementation, implement everything as macros.
*/
#تعریف_کرو sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#تعریف_کرو sqlite3_mutex_free(X)
#تعریف_کرو sqlite3_mutex_enter(X)    
#تعریف_کرو sqlite3_mutex_try(X)      SQLITE_OK
#تعریف_کرو sqlite3_mutex_leave(X)    
#تعریف_کرو sqlite3_mutex_held(X)     ((فراغ)(X),1)
#تعریف_کرو sqlite3_mutex_notheld(X)  ((فراغ)(X),1)
#تعریف_کرو sqlite3MutexAlloc(X)      ((sqlite3_mutex*)8)
#تعریف_کرو sqlite3MutexInit()        SQLITE_OK
#تعریف_کرو sqlite3_Mutex_سرا()
#تعریف_کرو MUTEX_LOGIC(X)
#وإلا
#تعریف_کرو MUTEX_LOGIC(X)            X
صحيح sqlite3_mutex_held(sqlite3_mutex*);
#جے_انت /* defined(SQLITE_MUTEX_OMIT) */
