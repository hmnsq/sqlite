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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _OS_COMMON_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _OS_COMMON_H_

/*
** At least two bugs have slipped in because we changed the MEMORY_DEBUG
** macro to SQLITE_DEBUG and some older makefiles have not yet made the
** switch.  The following code should catch this problem at compile-time.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ਸਮ੍ਰਿਤੀ_DEBUG
# error "The MEMORY_DEBUG macro is obsolete.  Use SQLITE_DEBUG instead."
#ਜੇ_ਅੰਤ

/*
** Macros for performance tracing.  Normally turned off.  Only works
** on i486 hardware.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_PERFORMANCE_TRACE

ਸਥਿਰ sqlite_uint64 g_ਸ਼ੁਰੂ;
ਸਥਿਰ sqlite_uint64 g_elapsed;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIMER_ਸ਼ੁਰੂ       g_ਸ਼ੁਰੂ=sqlite3Hwtime()
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIMER_ਸਿਰਾ         g_elapsed=sqlite3Hwtime()-g_ਸ਼ੁਰੂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIMER_ELAPSED     g_elapsed
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIMER_ਸ਼ੁਰੂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIMER_ਸਿਰਾ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TIMER_ELAPSED     ((sqlite_uint64)0)
#ਜੇ_ਅੰਤ

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#ਜੇ defined(SQLITE_TEST)
ਬਾਹਰੀ ਪੂਰਨ sqlite3_io_error_ਪ੍ਰਾਪਤੀ;
ਬਾਹਰੀ ਪੂਰਨ sqlite3_io_error_hardhit;
ਬਾਹਰੀ ਪੂਰਨ sqlite3_io_error_pending;
ਬਾਹਰੀ ਪੂਰਨ sqlite3_io_error_persist;
ਬਾਹਰੀ ਪੂਰਨ sqlite3_io_error_benign;
ਬਾਹਰੀ ਪੂਰਨ sqlite3_diskfull_pending;
ਬਾਹਰੀ ਪੂਰਨ sqlite3_diskfull;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SimulateIOErrorBenign(X) sqlite3_io_error_benign=(X)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SimulateIOError(ਕੋਡ_2)  \
  ਜੇ( (sqlite3_io_error_persist && sqlite3_io_error_ਪ੍ਰਾਪਤੀ) \
       || sqlite3_io_error_pending-- == 1 )  \
              { local_ioerr(); ਕੋਡ_2; }
ਸਥਿਰ ਖਾਲੀ local_ioerr(){
  IOTRACE(("IOERR\n"));
  sqlite3_io_error_ਪ੍ਰਾਪਤੀ++;
  ਜੇ( !sqlite3_io_error_benign ) sqlite3_io_error_hardhit++;
}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SimulateDiskfullError(ਕੋਡ_2) \
   ਜੇ( sqlite3_diskfull_pending ){ \
     ਜੇ( sqlite3_diskfull_pending == 1 ){ \
       local_ioerr(); \
       sqlite3_diskfull = 1; \
       sqlite3_io_error_ਪ੍ਰਾਪਤੀ = 1; \
       ਕੋਡ_2; \
     }ਨਹੀਂਤਾਂ{ \
       sqlite3_diskfull_pending--; \
     } \
   }
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SimulateIOErrorBenign(X)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SimulateIOError(A)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SimulateDiskfullError(A)
#ਜੇ_ਅੰਤ /* defined(SQLITE_TEST) */

/*
** When testing, keep a count of the number of open files.
*/
#ਜੇ defined(SQLITE_TEST)
ਬਾਹਰੀ ਪੂਰਨ sqlite3_open_file_ਗਿਣਤੀ;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OpenCounter(X)  sqlite3_open_file_ਗਿਣਤੀ+=(X)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OpenCounter(X)
#ਜੇ_ਅੰਤ /* defined(SQLITE_TEST) */

#ਜੇ_ਅੰਤ /* !defined(_OS_COMMON_H_) */
