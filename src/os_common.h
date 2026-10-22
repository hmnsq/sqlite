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
#اگر_تعریف_نشده _OS_COMMON_H_
#تعریف_کن _OS_COMMON_H_

/*
** At least two bugs have slipped in because we changed the MEMORY_DEBUG
** macro to SQLITE_DEBUG and some older makefiles have not yet made the
** switch.  The following code should catch this problem at compile-time.
*/
#اگر_تعریف_شده حافظه_DEBUG
# error "The MEMORY_DEBUG macro is obsolete.  Use SQLITE_DEBUG instead."
#پایان_اگر

/*
** Macros for performance tracing.  Normally turned off.  Only works
** on i486 hardware.
*/
#اگر_تعریف_شده SQLITE_PERFORMANCE_TRACE

ساكن sqlite_uint64 g_شروع;
ساكن sqlite_uint64 g_elapsed;
#تعریف_کن TIMER_شروع       g_شروع=sqlite3Hwtime()
#تعریف_کن TIMER_انتها         g_elapsed=sqlite3Hwtime()-g_شروع
#تعریف_کن TIMER_ELAPSED     g_elapsed
#وإلا
#تعریف_کن TIMER_شروع
#تعریف_کن TIMER_انتها
#تعریف_کن TIMER_ELAPSED     ((sqlite_uint64)0)
#پایان_اگر

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#إذا defined(SQLITE_TEST)
خارجي صحيح sqlite3_io_error_اصابت;
خارجي صحيح sqlite3_io_error_hardhit;
خارجي صحيح sqlite3_io_error_pending;
خارجي صحيح sqlite3_io_error_persist;
خارجي صحيح sqlite3_io_error_benign;
خارجي صحيح sqlite3_diskfull_pending;
خارجي صحيح sqlite3_diskfull;
#تعریف_کن SimulateIOErrorBenign(X) sqlite3_io_error_benign=(X)
#تعریف_کن SimulateIOError(کد_2)  \
  إذا( (sqlite3_io_error_persist && sqlite3_io_error_اصابت) \
       || sqlite3_io_error_pending-- == 1 )  \
              { local_ioerr(); کد_2; }
ساكن فراغ local_ioerr(){
  IOTRACE(("IOERR\n"));
  sqlite3_io_error_اصابت++;
  إذا( !sqlite3_io_error_benign ) sqlite3_io_error_hardhit++;
}
#تعریف_کن SimulateDiskfullError(کد_2) \
   إذا( sqlite3_diskfull_pending ){ \
     إذا( sqlite3_diskfull_pending == 1 ){ \
       local_ioerr(); \
       sqlite3_diskfull = 1; \
       sqlite3_io_error_اصابت = 1; \
       کد_2; \
     }وإلا{ \
       sqlite3_diskfull_pending--; \
     } \
   }
#وإلا
#تعریف_کن SimulateIOErrorBenign(X)
#تعریف_کن SimulateIOError(A)
#تعریف_کن SimulateDiskfullError(A)
#پایان_اگر /* defined(SQLITE_TEST) */

/*
** When testing, keep a count of the number of open files.
*/
#إذا defined(SQLITE_TEST)
خارجي صحيح sqlite3_open_file_شمار;
#تعریف_کن OpenCounter(X)  sqlite3_open_file_شمار+=(X)
#وإلا
#تعریف_کن OpenCounter(X)
#پایان_اگر /* defined(SQLITE_TEST) */

#پایان_اگر /* !defined(_OS_COMMON_H_) */
