/*
** 2013 November 25
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains pre-processor directives related to operating system
** detection and/or setup.
*/
#నిర్వచితం_కాకపోతే SQLITE_OS_అమరిక_H
#నిర్వచించు SQLITE_OS_అమరిక_H

/*
** Figure out if we are dealing with Unix, Windows, or some other operating
** system.
**
** After the following block of preprocess macros, all of 
**
**    SQLITE_OS_KV
**    SQLITE_OS_OTHER
**    SQLITE_OS_UNIX
**    SQLITE_OS_WIN
**
** will defined to either 1 or 0. One of them will be 1. The others will be 0.
** If none of the macros are initially defined, then select either
** SQLITE_OS_UNIX or SQLITE_OS_WIN depending on the target platform.
**
** If SQLITE_OS_OTHER=1 is specified at compile-time, then the application
** must provide its own VFS implementation together with sqlite3_os_init()
** and sqlite3_os_end() routines.
*/
#ఒకవేళ SQLITE_OS_KV+1<=1  && SQLITE_OS_OTHER+1<=1 &&  \
    SQLITE_OS_WIN+1<=1 && SQLITE_OS_UNIX+1<=1 
#  ఒకవేళ defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || \
          defined(__MINGW32__) || defined(__BORLANDC__)
#    define SQLITE_OS_WIN 1
#    define SQLITE_OS_UNIX 0
#  లేకపోతే
#    define SQLITE_OS_WIN 0
#    define SQLITE_OS_UNIX 1
#  endif
#అయితే_ముగింపు
#ఒకవేళ SQLITE_OS_OTHER+1>1
#  undef SQLITE_OS_KV
#  define SQLITE_OS_KV 0
#  undef SQLITE_OS_UNIX
#  define SQLITE_OS_UNIX 0
#  undef SQLITE_OS_WIN
#  define SQLITE_OS_WIN 0
#అయితే_ముగింపు
#ఒకవేళ SQLITE_OS_KV+1>1
#  undef SQLITE_OS_OTHER
#  define SQLITE_OS_OTHER 0
#  undef SQLITE_OS_UNIX
#  define SQLITE_OS_UNIX 0
#  undef SQLITE_OS_WIN
#  define SQLITE_OS_WIN 0
#  define SQLITE_OMIT_ఎక్కించు_EXTENSION 1
#  define SQLITE_OMIT_WAL 1
#  define SQLITE_OMIT_DEPRECATED 1
#  undef SQLITE_తాత్కాలిక_STORE
#  define SQLITE_తాత్కాలిక_STORE 3  /* Always use memory for temporary storage */
#  define SQLITE_DQS 0
#  define SQLITE_OMIT_SHARED_CACHE 1
#  define SQLITE_OMIT_AUTOINIT 1
#అయితే_ముగింపు
#ఒకవేళ SQLITE_OS_UNIX+1>1
#  undef SQLITE_OS_KV
#  define SQLITE_OS_KV 0
#  undef SQLITE_OS_OTHER
#  define SQLITE_OS_OTHER 0
#  undef SQLITE_OS_WIN
#  define SQLITE_OS_WIN 0
#అయితే_ముగింపు
#ఒకవేళ SQLITE_OS_WIN+1>1
#  undef SQLITE_OS_KV
#  define SQLITE_OS_KV 0
#  undef SQLITE_OS_OTHER
#  define SQLITE_OS_OTHER 0
#  undef SQLITE_OS_UNIX
#  define SQLITE_OS_UNIX 0
#అయితే_ముగింపు


#అయితే_ముగింపు /* SQLITE_OS_SETUP_H */
