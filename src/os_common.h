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
#यदि_अपरिभाषितम् _OS_COMMON_H_
#परिभाषय _OS_COMMON_H_

/*
** At least two bugs have slipped in because we changed the MEMORY_DEBUG
** macro to SQLITE_DEBUG and some older makefiles have not yet made the
** switch.  The following code should catch this problem at compile-time.
*/
#यदि_परिभाषितम् स्मृतिः_DEBUG
# error "The MEMORY_DEBUG macro is obsolete.  Use SQLITE_DEBUG instead."
#यदि_अन्तः

/*
** Macros for performance tracing.  Normally turned off.  Only works
** on i486 hardware.
*/
#यदि_परिभाषितम् SQLITE_PERFORMANCE_TRACE

स्थिर sqlite_uint64 g_start;
स्थिर sqlite_uint64 g_elapsed;
#परिभाषय TIMER_START       g_start=sqlite3Hwtime()
#परिभाषय TIMER_अन्तः         g_elapsed=sqlite3Hwtime()-g_start
#परिभाषय TIMER_ELAPSED     g_elapsed
#अन्यथा
#परिभाषय TIMER_START
#परिभाषय TIMER_अन्तः
#परिभाषय TIMER_ELAPSED     ((sqlite_uint64)0)
#यदि_अन्तः

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#यदि defined(SQLITE_TEST)
बाह्य पूर्णाङ्क sqlite3_io_error_लब्धम्;
बाह्य पूर्णाङ्क sqlite3_io_error_hardhit;
बाह्य पूर्णाङ्क sqlite3_io_error_pending;
बाह्य पूर्णाङ्क sqlite3_io_error_persist;
बाह्य पूर्णाङ्क sqlite3_io_error_benign;
बाह्य पूर्णाङ्क sqlite3_diskfull_pending;
बाह्य पूर्णाङ्क sqlite3_diskfull;
#परिभाषय SimulateIOErrorBenign(X) sqlite3_io_error_benign=(X)
#परिभाषय SimulateIOError(संहिता_2)  \
  यदि( (sqlite3_io_error_persist && sqlite3_io_error_लब्धम्) \
       || sqlite3_io_error_pending-- == 1 )  \
              { local_ioerr(); संहिता_2; }
स्थिर शून्य local_ioerr(){
  IOTRACE(("IOERR\n"));
  sqlite3_io_error_लब्धम्++;
  यदि( !sqlite3_io_error_benign ) sqlite3_io_error_hardhit++;
}
#परिभाषय SimulateDiskfullError(संहिता_2) \
   यदि( sqlite3_diskfull_pending ){ \
     यदि( sqlite3_diskfull_pending == 1 ){ \
       local_ioerr(); \
       sqlite3_diskfull = 1; \
       sqlite3_io_error_लब्धम् = 1; \
       संहिता_2; \
     }अन्यथा{ \
       sqlite3_diskfull_pending--; \
     } \
   }
#अन्यथा
#परिभाषय SimulateIOErrorBenign(X)
#परिभाषय SimulateIOError(A)
#परिभाषय SimulateDiskfullError(A)
#यदि_अन्तः /* defined(SQLITE_TEST) */

/*
** When testing, keep a count of the number of open files.
*/
#यदि defined(SQLITE_TEST)
बाह्य पूर्णाङ्क sqlite3_open_file_गणनम्;
#परिभाषय Open_गणक(X)  sqlite3_open_file_गणनम्+=(X)
#अन्यथा
#परिभाषय Open_गणक(X)
#यदि_अन्तः /* defined(SQLITE_TEST) */

#यदि_अन्तः /* !defined(_OS_COMMON_H_) */
