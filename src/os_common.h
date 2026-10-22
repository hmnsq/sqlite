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
#جيڪڏهن_وصف_نه_ٿيل _OS_COMMON_H_
#وصف_ڪريو _OS_COMMON_H_

/*
** At least two bugs have slipped in because we changed the MEMORY_DEBUG
** macro to SQLITE_DEBUG and some older makefiles have not yet made the
** switch.  The following code should catch this problem at compile-time.
*/
#جيڪڏهن_وصف_ٿيل يادگيري_DEBUG
# error "The MEMORY_DEBUG macro is obsolete.  Use SQLITE_DEBUG instead."
#جيڪڏهن_پورو

/*
** Macros for performance tracing.  Normally turned off.  Only works
** on i486 hardware.
*/
#جيڪڏهن_وصف_ٿيل SQLITE_PERFORMANCE_TRACE

ساكن sqlite_uint64 g_شروعات;
ساكن sqlite_uint64 g_elapsed;
#وصف_ڪريو TIMER_شروعات       g_شروعات=sqlite3Hwtime()
#وصف_ڪريو TIMER_پڇاڙي         g_elapsed=sqlite3Hwtime()-g_شروعات
#وصف_ڪريو TIMER_ELAPSED     g_elapsed
#وإلا
#وصف_ڪريو TIMER_شروعات
#وصف_ڪريو TIMER_پڇاڙي
#وصف_ڪريو TIMER_ELAPSED     ((sqlite_uint64)0)
#جيڪڏهن_پورو

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#إذا defined(SQLITE_TEST)
خارجي صحيح sqlite3_io_error_ڪاميابي;
خارجي صحيح sqlite3_io_error_hardhit;
خارجي صحيح sqlite3_io_error_pending;
خارجي صحيح sqlite3_io_error_persist;
خارجي صحيح sqlite3_io_error_benign;
خارجي صحيح sqlite3_diskfull_pending;
خارجي صحيح sqlite3_diskfull;
#وصف_ڪريو SimulateIOErrorBenign(X) sqlite3_io_error_benign=(X)
#وصف_ڪريو SimulateIOError(ڪوڊ_2)  \
  إذا( (sqlite3_io_error_persist && sqlite3_io_error_ڪاميابي) \
       || sqlite3_io_error_pending-- == 1 )  \
              { local_ioerr(); ڪوڊ_2; }
ساكن فراغ local_ioerr(){
  IOTRACE(("IOERR\n"));
  sqlite3_io_error_ڪاميابي++;
  إذا( !sqlite3_io_error_benign ) sqlite3_io_error_hardhit++;
}
#وصف_ڪريو SimulateDiskfullError(ڪوڊ_2) \
   إذا( sqlite3_diskfull_pending ){ \
     إذا( sqlite3_diskfull_pending == 1 ){ \
       local_ioerr(); \
       sqlite3_diskfull = 1; \
       sqlite3_io_error_ڪاميابي = 1; \
       ڪوڊ_2; \
     }وإلا{ \
       sqlite3_diskfull_pending--; \
     } \
   }
#وإلا
#وصف_ڪريو SimulateIOErrorBenign(X)
#وصف_ڪريو SimulateIOError(A)
#وصف_ڪريو SimulateDiskfullError(A)
#جيڪڏهن_پورو /* defined(SQLITE_TEST) */

/*
** When testing, keep a count of the number of open files.
*/
#إذا defined(SQLITE_TEST)
خارجي صحيح sqlite3_open_file_ڳڻپ;
#وصف_ڪريو OpenCounter(X)  sqlite3_open_file_ڳڻپ+=(X)
#وإلا
#وصف_ڪريو OpenCounter(X)
#جيڪڏهن_پورو /* defined(SQLITE_TEST) */

#جيڪڏهن_پورو /* !defined(_OS_COMMON_H_) */
