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
#నిర్వచితం_కాకపోతే _OS_COMMON_H_
#నిర్వచించు _OS_COMMON_H_

/*
** At least two bugs have slipped in because we changed the MEMORY_DEBUG
** macro to SQLITE_DEBUG and some older makefiles have not yet made the
** switch.  The following code should catch this problem at compile-time.
*/
#నిర్వచితమైతే జ్ఞాపకం_DEBUG
# error "The MEMORY_DEBUG macro is obsolete.  Use SQLITE_DEBUG instead."
#అయితే_ముగింపు

/*
** Macros for performance tracing.  Normally turned off.  Only works
** on i486 hardware.
*/
#నిర్వచితమైతే SQLITE_PERFORMANCE_TRACE

స్థిర sqlite_uint64 g_ప్రారంభం;
స్థిర sqlite_uint64 g_elapsed;
#నిర్వచించు TIMER_ప్రారంభం       g_ప్రారంభం=sqlite3Hwtime()
#నిర్వచించు TIMER_చివర         g_elapsed=sqlite3Hwtime()-g_ప్రారంభం
#నిర్వచించు TIMER_ELAPSED     g_elapsed
#లేకపోతే
#నిర్వచించు TIMER_ప్రారంభం
#నిర్వచించు TIMER_చివర
#నిర్వచించు TIMER_ELAPSED     ((sqlite_uint64)0)
#అయితే_ముగింపు

/*
** If we compile with the SQLITE_TEST macro set, then the following block
** of code will give us the ability to simulate a disk I/O error.  This
** is used for testing the I/O recovery logic.
*/
#ఒకవేళ defined(SQLITE_TEST)
బాహ్య పూర్ణాంకం sqlite3_io_error_తగిలింది;
బాహ్య పూర్ణాంకం sqlite3_io_error_hardhit;
బాహ్య పూర్ణాంకం sqlite3_io_error_pending;
బాహ్య పూర్ణాంకం sqlite3_io_error_persist;
బాహ్య పూర్ణాంకం sqlite3_io_error_benign;
బాహ్య పూర్ణాంకం sqlite3_diskfull_pending;
బాహ్య పూర్ణాంకం sqlite3_diskfull;
#నిర్వచించు SimulateIOErrorBenign(X) sqlite3_io_error_benign=(X)
#నిర్వచించు SimulateIOError(సంకేతం_3)  \
  ఒకవేళ( (sqlite3_io_error_persist && sqlite3_io_error_తగిలింది) \
       || sqlite3_io_error_pending-- == 1 )  \
              { local_ioerr(); సంకేతం_3; }
స్థిర శూన్య local_ioerr(){
  IOTRACE(("IOERR\n"));
  sqlite3_io_error_తగిలింది++;
  ఒకవేళ( !sqlite3_io_error_benign ) sqlite3_io_error_hardhit++;
}
#నిర్వచించు SimulateDiskfullError(సంకేతం_3) \
   ఒకవేళ( sqlite3_diskfull_pending ){ \
     ఒకవేళ( sqlite3_diskfull_pending == 1 ){ \
       local_ioerr(); \
       sqlite3_diskfull = 1; \
       sqlite3_io_error_తగిలింది = 1; \
       సంకేతం_3; \
     }లేకపోతే{ \
       sqlite3_diskfull_pending--; \
     } \
   }
#లేకపోతే
#నిర్వచించు SimulateIOErrorBenign(X)
#నిర్వచించు SimulateIOError(A)
#నిర్వచించు SimulateDiskfullError(A)
#అయితే_ముగింపు /* defined(SQLITE_TEST) */

/*
** When testing, keep a count of the number of open files.
*/
#ఒకవేళ defined(SQLITE_TEST)
బాహ్య పూర్ణాంకం sqlite3_open_file_లెక్క;
#నిర్వచించు OpenCounter(X)  sqlite3_open_file_లెక్క+=(X)
#లేకపోతే
#నిర్వచించు OpenCounter(X)
#అయితే_ముగింపు /* defined(SQLITE_TEST) */

#అయితే_ముగింపు /* !defined(_OS_COMMON_H_) */
