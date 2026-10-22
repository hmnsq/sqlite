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
#अगर_अघोषित _OS_COMMON_H_
#घोषणा _OS_COMMON_H_

/*
** At least two bugs have slipped in because we changed the MEMORY_DEBUG
** macro to SQLITE_DEBUG and some older makefiles have not yet made the
** switch.  The following code should catch this problem at compile-time.
*/
#अगर_घोषित MEMORY_DEBUG
# error "The MEMORY_DEBUG macro is obsolete.  Use SQLITE_DEBUG instead."
#पूर्ण_अगर

/*
** Macros for performance tracing.  Normally turned off.  Only works
** on i486 hardware.
*/
#अगर_घोषित SQLITE_PERFORMANCE_TRACE

अटल sqlite_uint64 g_start;
अटल sqlite_uint64 g_elapsed;
#घोषणा TIMER_START       g_start=sqlite3Hwtime()
#घोषणा TIMER_END         g_elapsed=sqlite3Hwtime()-g_start
#घोषणा TIMER_ELAPSED     g_elapsed
#अन्यथा
#घोषणा TIMER_START
#घोषणा TIMER_END
#घोषणा TIMER_ELAPSED     ((sqlite_uint64)0)
#पूर्ण_अगर

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#अगर defined(SQLITE_TEST)
बाह्य पूर्णांक sqlite3_io_error_hit;
बाह्य पूर्णांक sqlite3_io_error_hardhit;
बाह्य पूर्णांक sqlite3_io_error_pending;
बाह्य पूर्णांक sqlite3_io_error_persist;
बाह्य पूर्णांक sqlite3_io_error_benign;
बाह्य पूर्णांक sqlite3_diskfull_pending;
बाह्य पूर्णांक sqlite3_diskfull;
#घोषणा SimulateIOErrorBenign(X) sqlite3_io_error_benign=(X)
#घोषणा SimulateIOError(CODE)  \
  अगर( (sqlite3_io_error_persist && sqlite3_io_error_hit) \
       || sqlite3_io_error_pending-- == 1 )  \
              { local_ioerr(); CODE; }
अटल व्योम local_ioerr(){
  IOTRACE(("IOERR\n"));
  sqlite3_io_error_hit++;
  अगर( !sqlite3_io_error_benign ) sqlite3_io_error_hardhit++;
}
#घोषणा SimulateDiskfullError(CODE) \
   अगर( sqlite3_diskfull_pending ){ \
     अगर( sqlite3_diskfull_pending == 1 ){ \
       local_ioerr(); \
       sqlite3_diskfull = 1; \
       sqlite3_io_error_hit = 1; \
       CODE; \
     }अन्यथा{ \
       sqlite3_diskfull_pending--; \
     } \
   }
#अन्यथा
#घोषणा SimulateIOErrorBenign(X)
#घोषणा SimulateIOError(A)
#घोषणा SimulateDiskfullError(A)
#पूर्ण_अगर /* defined(SQLITE_TEST) */

/*
** When testing, keep a count of the number of open files.
*/
#अगर defined(SQLITE_TEST)
बाह्य पूर्णांक sqlite3_open_file_गिनती;
#घोषणा Open_गणक(X)  sqlite3_open_file_गिनती+=(X)
#अन्यथा
#घोषणा Open_गणक(X)
#पूर्ण_अगर /* defined(SQLITE_TEST) */

#पूर्ण_अगर /* !defined(_OS_COMMON_H_) */
