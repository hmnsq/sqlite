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
#إذا defined(__RTP__) || defined(_WRS_KERNEL)
/* This is VxWorks.  Set up things specially for that OS
*/
#ضمن <vxWorks.h>
#ضمن <pthread.h>  /* amalgamator: dontcache */
#تعریف_کرو OS_VXWORKS 1
#تعریف_کرو SQLITE_OS_OTHER 0
#تعریف_کرو SQLITE_HOMEGROWN_RECURSIVE_MUTEX 1
#تعریف_کرو SQLITE_OMIT_لدو_EXTENSION 1
#تعریف_کرو SQLITE_ENABLE_LOCKING_STYLE 0
#تعریف_کرو HAVE_UTIME 1
#وإلا
/* This is not VxWorks. */
#جے_بنا_تعریف OS_VXWORKS
#  define OS_VXWORKS 0
#جے_انت
#تعریف_کرو HAVE_FCHOWN 1
#تعریف_کرو HAVE_READLINK 1
#تعریف_کرو HAVE_LSTAT 1
#جے_انت /* defined(_WRS_KERNEL) */
