/*
** 2015-03-02
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
** This file contains code that is specific to Wind River's VxWorks
*/
#ఒకవేళ defined(__RTP__) || defined(_WRS_KERNEL)
/* This is VxWorks.  Set up things specially for that OS
*/
#అన్తర్భావ <vxWorks.h>
#అన్తర్భావ <pthread.h>  /* amalgamator: dontcache */
#నిర్వచించు OS_VXWORKS 1
#నిర్వచించు SQLITE_OS_OTHER 0
#నిర్వచించు SQLITE_HOMEGROWN_RECURSIVE_MUTEX 1
#నిర్వచించు SQLITE_OMIT_ఎక్కించు_EXTENSION 1
#నిర్వచించు SQLITE_ENABLE_LOCKING_STYLE 0
#నిర్వచించు HAVE_UTIME 1
#లేకపోతే
/* This is not VxWorks. */
#నిర్వచితం_కాకపోతే OS_VXWORKS
#  define OS_VXWORKS 0
#అయితే_ముగింపు
#నిర్వచించు HAVE_FCHOWN 1
#నిర్వచించు HAVE_READLINK 1
#నిర్వచించు HAVE_LSTAT 1
#అయితే_ముగింపు /* defined(_WRS_KERNEL) */
