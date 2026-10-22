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
#تعریف_کن OS_VXWORKS 1
#تعریف_کن SQLITE_OS_OTHER 0
#تعریف_کن SQLITE_HOMEGROWN_RECURSIVE_MUTEX 1
#تعریف_کن SQLITE_OMIT_بارگیری_EXTENSION 1
#تعریف_کن SQLITE_ENABLE_LOCKING_STYLE 0
#تعریف_کن HAVE_UTIME 1
#وإلا
/* This is not VxWorks. */
#اگر_تعریف_نشده OS_VXWORKS
#  define OS_VXWORKS 0
#پایان_اگر
#تعریف_کن HAVE_FCHOWN 1
#تعریف_کن HAVE_READLINK 1
#تعریف_کن HAVE_LSTAT 1
#پایان_اگر /* defined(_WRS_KERNEL) */
