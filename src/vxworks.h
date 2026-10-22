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
#وصف_ڪريو OS_VXWORKS 1
#وصف_ڪريو SQLITE_OS_OTHER 0
#وصف_ڪريو SQLITE_HOMEGROWN_RECURSIVE_MUTEX 1
#وصف_ڪريو SQLITE_OMIT_ڀريو_EXTENSION 1
#وصف_ڪريو SQLITE_ENABLE_LOCKING_STYLE 0
#وصف_ڪريو HAVE_UTIME 1
#وإلا
/* This is not VxWorks. */
#جيڪڏهن_وصف_نه_ٿيل OS_VXWORKS
#  define OS_VXWORKS 0
#جيڪڏهن_پورو
#وصف_ڪريو HAVE_FCHOWN 1
#وصف_ڪريو HAVE_READLINK 1
#وصف_ڪريو HAVE_LSTAT 1
#جيڪڏهن_پورو /* defined(_WRS_KERNEL) */
