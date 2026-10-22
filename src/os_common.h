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
#اگر_غیر_تعریف_شدہ _OS_COMMON_H_
#تعریف_کرو _OS_COMMON_H_

/*
** At least two bugs have slipped in because we changed the MEMORY_DEBUG
** macro to SQLITE_DEBUG and some older makefiles have not yet made the
** switch.  The following code should catch this problem at compile-time.
*/
#اگر_تعریف_شدہ حافظہ_DEBUG
# error "The MEMORY_DEBUG macro is obsolete.  Use SQLITE_DEBUG instead."
#اگر_ختم

/*
** Macros for performance tracing.  Normally turned off.  Only works
** on i486 hardware.
*/
#اگر_تعریف_شدہ SQLITE_PERFORMANCE_TRACE

ساکن sqlite_uint64 g_آغاز;
ساکن sqlite_uint64 g_elapsed;
#تعریف_کرو TIMER_آغاز       g_آغاز=sqlite3Hwtime()
#تعریف_کرو TIMER_انتہا         g_elapsed=sqlite3Hwtime()-g_آغاز
#تعریف_کرو TIMER_ELAPSED     g_elapsed
#ورنہ
#تعریف_کرو TIMER_آغاز
#تعریف_کرو TIMER_انتہا
#تعریف_کرو TIMER_ELAPSED     ((sqlite_uint64)0)
#اگر_ختم

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#اگر defined(SQLITE_TEST)
خارجی صحیح sqlite3_io_error_کامیابی;
خارجی صحیح sqlite3_io_error_hardhit;
خارجی صحیح sqlite3_io_error_pending;
خارجی صحیح sqlite3_io_error_persist;
خارجی صحیح sqlite3_io_error_benign;
خارجی صحیح sqlite3_diskfull_pending;
خارجی صحیح sqlite3_diskfull;
#تعریف_کرو SimulateIOErrorBenign(X) sqlite3_io_error_benign=(X)
#تعریف_کرو SimulateIOError(رمز_2)  \
  اگر( (sqlite3_io_error_persist && sqlite3_io_error_کامیابی) \
       || sqlite3_io_error_pending-- == 1 )  \
              { local_ioerr(); رمز_2; }
ساکن خالی local_ioerr(){
  IOTRACE(("IOERR\n"));
  sqlite3_io_error_کامیابی++;
  اگر( !sqlite3_io_error_benign ) sqlite3_io_error_hardhit++;
}
#تعریف_کرو SimulateDiskfullError(رمز_2) \
   اگر( sqlite3_diskfull_pending ){ \
     اگر( sqlite3_diskfull_pending == 1 ){ \
       local_ioerr(); \
       sqlite3_diskfull = 1; \
       sqlite3_io_error_کامیابی = 1; \
       رمز_2; \
     }ورنہ{ \
       sqlite3_diskfull_pending--; \
     } \
   }
#ورنہ
#تعریف_کرو SimulateIOErrorBenign(X)
#تعریف_کرو SimulateIOError(A)
#تعریف_کرو SimulateDiskfullError(A)
#اگر_ختم /* defined(SQLITE_TEST) */

/*
** When testing, keep a count of the number of open files.
*/
#اگر defined(SQLITE_TEST)
خارجی صحیح sqlite3_open_file_گنتی;
#تعریف_کرو Open_شمار(X)  sqlite3_open_file_گنتی+=(X)
#ورنہ
#تعریف_کرو Open_شمار(X)
#اگر_ختم /* defined(SQLITE_TEST) */

#اگر_ختم /* !defined(_OS_COMMON_H_) */
