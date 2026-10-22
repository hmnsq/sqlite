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
#ಆದರೆ !SQLITE_THREADSAFE
# define SQLITE_MUTEX_OMIT
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_THREADSAFE && !defined(SQLITE_MUTEX_NOOP)
#  ಆದರೆ SQLITE_OS_UNIX || defined(__CYGWIN__)
#    define SQLITE_MUTEX_PTHREADS
#  elif SQLITE_OS_WIN
#    define SQLITE_MUTEX_W32
#  ಇಲ್ಲದಿದ್ದರೆ
#    define SQLITE_MUTEX_NOOP
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MUTEX_OMIT
/*
** If this is a no-op implementation, implement everything as macros.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_free(X)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_enter(X)    
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_try(X)      SQLITE_OK
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_leave(X)    
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_held(X)     ((ಶೂನ್ಯ)(X),1)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_mutex_notheld(X)  ((ಶೂನ್ಯ)(X),1)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3MutexAlloc(X)      ((sqlite3_mutex*)8)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3MutexInit()        SQLITE_OK
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_Mutex_ಕೊನೆ()
#ವ್ಯಾಖ್ಯಾನಿಸು MUTEX_LOGIC(X)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು MUTEX_LOGIC(X)            X
ಪೂರ್ಣಾಂಕ sqlite3_mutex_held(sqlite3_mutex*);
#ಆದರೆ_ಅಂತ್ಯ /* defined(SQLITE_MUTEX_OMIT) */
