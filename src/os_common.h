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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _OS_COMMON_H_
#ವ್ಯಾಖ್ಯಾನಿಸು _OS_COMMON_H_

/*
** At least two bugs have slipped in because we changed the MEMORY_DEBUG
** macro to SQLITE_DEBUG and some older makefiles have not yet made the
** switch.  The following code should catch this problem at compile-time.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ಸ್ಮರಣೆ_DEBUG
# error "The MEMORY_DEBUG macro is obsolete.  Use SQLITE_DEBUG instead."
#ಆದರೆ_ಅಂತ್ಯ

/*
** Macros for performance tracing.  Normally turned off.  Only works
** on i486 hardware.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_PERFORMANCE_TRACE

ಸ್ಥಿರ sqlite_uint64 g_ಆರಂಭ;
ಸ್ಥಿರ sqlite_uint64 g_elapsed;
#ವ್ಯಾಖ್ಯಾನಿಸು TIMER_ಆರಂಭ       g_ಆರಂಭ=sqlite3Hwtime()
#ವ್ಯಾಖ್ಯಾನಿಸು TIMER_ಕೊನೆ         g_elapsed=sqlite3Hwtime()-g_ಆರಂಭ
#ವ್ಯಾಖ್ಯಾನಿಸು TIMER_ELAPSED     g_elapsed
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು TIMER_ಆರಂಭ
#ವ್ಯಾಖ್ಯಾನಿಸು TIMER_ಕೊನೆ
#ವ್ಯಾಖ್ಯಾನಿಸು TIMER_ELAPSED     ((sqlite_uint64)0)
#ಆದರೆ_ಅಂತ್ಯ

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#ಆದರೆ defined(SQLITE_TEST)
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_io_error_ದೊರಕಿತು;
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_io_error_hardhit;
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_io_error_pending;
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_io_error_persist;
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_io_error_benign;
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_diskfull_pending;
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_diskfull;
#ವ್ಯಾಖ್ಯಾನಿಸು SimulateIOErrorBenign(X) sqlite3_io_error_benign=(X)
#ವ್ಯಾಖ್ಯಾನಿಸು SimulateIOError(ಸಂಕೇತ_3)  \
  ಆದರೆ( (sqlite3_io_error_persist && sqlite3_io_error_ದೊರಕಿತು) \
       || sqlite3_io_error_pending-- == 1 )  \
              { local_ioerr(); ಸಂಕೇತ_3; }
ಸ್ಥಿರ ಶೂನ್ಯ local_ioerr(){
  IOTRACE(("IOERR\n"));
  sqlite3_io_error_ದೊರಕಿತು++;
  ಆದರೆ( !sqlite3_io_error_benign ) sqlite3_io_error_hardhit++;
}
#ವ್ಯಾಖ್ಯಾನಿಸು SimulateDiskfullError(ಸಂಕೇತ_3) \
   ಆದರೆ( sqlite3_diskfull_pending ){ \
     ಆದರೆ( sqlite3_diskfull_pending == 1 ){ \
       local_ioerr(); \
       sqlite3_diskfull = 1; \
       sqlite3_io_error_ದೊರಕಿತು = 1; \
       ಸಂಕೇತ_3; \
     }ಇಲ್ಲದಿದ್ದರೆ{ \
       sqlite3_diskfull_pending--; \
     } \
   }
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು SimulateIOErrorBenign(X)
#ವ್ಯಾಖ್ಯಾನಿಸು SimulateIOError(A)
#ವ್ಯಾಖ್ಯಾನಿಸು SimulateDiskfullError(A)
#ಆದರೆ_ಅಂತ್ಯ /* defined(SQLITE_TEST) */

/*
** When testing, keep a count of the number of open files.
*/
#ಆದರೆ defined(SQLITE_TEST)
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3_open_file_ಎಣಿಕೆ;
#ವ್ಯಾಖ್ಯಾನಿಸು OpenCounter(X)  sqlite3_open_file_ಎಣಿಕೆ+=(X)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು OpenCounter(X)
#ಆದರೆ_ಅಂತ್ಯ /* defined(SQLITE_TEST) */

#ಆದರೆ_ಅಂತ್ಯ /* !defined(_OS_COMMON_H_) */
