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
#اگر defined(__RTP__) || defined(_WRS_KERNEL)
/* This is VxWorks.  Set up things specially for that OS
*/
#شامل <vxWorks.h>
#شامل <pthread.h>  /* amalgamator: dontcache */
#تعریف_کرو OS_VXWORKS 1
#تعریف_کرو SQLITE_OS_OTHER 0
#تعریف_کرو SQLITE_HOMEGROWN_RECURSIVE_MUTEX 1
#تعریف_کرو SQLITE_OMIT_بھرو_EXTENSION 1
#تعریف_کرو SQLITE_ENABLE_LOCKING_STYLE 0
#تعریف_کرو HAVE_UTIME 1
#ورنہ
/* This is not VxWorks. */
#اگر_غیر_تعریف_شدہ OS_VXWORKS
#  define OS_VXWORKS 0
#اگر_ختم
#تعریف_کرو HAVE_FCHOWN 1
#تعریف_کرو HAVE_READLINK 1
#تعریف_کرو HAVE_LSTAT 1
#اگر_ختم /* defined(_WRS_KERNEL) */
