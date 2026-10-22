/*
** 2004 May 22
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
** This file contains macros and a little bit of code that is common to
** all of the platform-specific files (os_*.c) and is #included into those
** files.
**
** This file should be #included by the os_*.c files only.  It is not a
** general purpose header file.
*/
#ifndef _OS_COMMON_H_
#define _OS_COMMON_H_

/*
** At least two bugs have slipped in because we changed the MEMORY_DEBUG
** macro to SQLITE_DEBUG and some older makefiles have not yet made the
** switch.  The following code should catch this problem at compile-time.
*/
#ifdef MEMORY_DEBUG
# error "The MEMORY_DEBUG macro is obsolete.  Use SQLITE_DEBUG instead."
#endif

/*
** Macros for performance tracing.  Normally turned off.  Only works
** on i486 hardware.
*/
#ifdef SQLITE_PERFORMANCE_TRACE

স্থির sqlite_uint64 g_start;
স্থির sqlite_uint64 g_elapsed;
#define TIMER_START       g_start=sqlite3Hwtime()
#define TIMER_END         g_elapsed=sqlite3Hwtime()-g_start
#define TIMER_ELAPSED     g_elapsed
#অন্যথা
#define TIMER_START
#define TIMER_END
#define TIMER_ELAPSED     ((sqlite_uint64)0)
#endif

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#যদি defined(SQLITE_TEST)
বাহ্য পূর্ণাঙ্ক sqlite3_io_error_hit;
বাহ্য পূর্ণাঙ্ক sqlite3_io_error_hardhit;
বাহ্য পূর্ণাঙ্ক sqlite3_io_error_pending;
বাহ্য পূর্ণাঙ্ক sqlite3_io_error_persist;
বাহ্য পূর্ণাঙ্ক sqlite3_io_error_benign;
বাহ্য পূর্ণাঙ্ক sqlite3_diskfull_pending;
বাহ্য পূর্ণাঙ্ক sqlite3_diskfull;
#define SimulateIOErrorBenign(X) sqlite3_io_error_benign=(X)
#define SimulateIOError(CODE)  \
  যদি( (sqlite3_io_error_persist && sqlite3_io_error_hit) \
       || sqlite3_io_error_pending-- == 1 )  \
              { local_ioerr(); CODE; }
স্থির শূন্য local_ioerr(){
  IOTRACE(("IOERR\n"));
  sqlite3_io_error_hit++;
  যদি( !sqlite3_io_error_benign ) sqlite3_io_error_hardhit++;
}
#define SimulateDiskfullError(CODE) \
   যদি( sqlite3_diskfull_pending ){ \
     যদি( sqlite3_diskfull_pending == 1 ){ \
       local_ioerr(); \
       sqlite3_diskfull = 1; \
       sqlite3_io_error_hit = 1; \
       CODE; \
     }অন্যথা{ \
       sqlite3_diskfull_pending--; \
     } \
   }
#অন্যথা
#define SimulateIOErrorBenign(X)
#define SimulateIOError(A)
#define SimulateDiskfullError(A)
#endif /* defined(SQLITE_TEST) */

/*
** When testing, keep a count of the number of open files.
*/
#যদি defined(SQLITE_TEST)
বাহ্য পূর্ণাঙ্ক sqlite3_open_file_গুনতি;
#define OpenCounter(X)  sqlite3_open_file_গুনতি+=(X)
#অন্যথা
#define OpenCounter(X)
#endif /* defined(SQLITE_TEST) */

#endif /* !defined(_OS_COMMON_H_) */
