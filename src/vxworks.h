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
#यदि defined(__RTP__) || defined(_WRS_KERNEL)
/* This is VxWorks.  Set up things specially for that OS
*/
#अन्तर्भाव <vxWorks.h>
#अन्तर्भाव <pthread.h>  /* amalgamator: dontcache */
#परिभाषय OS_VXWORKS 1
#परिभाषय SQLITE_OS_OTHER 0
#परिभाषय SQLITE_HOMEGROWN_RECURSIVE_MUTEX 1
#परिभाषय SQLITE_OMIT_आरोपय_EXTENSION 1
#परिभाषय SQLITE_ENABLE_LOCKING_STYLE 0
#परिभाषय HAVE_UTIME 1
#अन्यथा
/* This is not VxWorks. */
#यदि_अपरिभाषितम् OS_VXWORKS
#  define OS_VXWORKS 0
#यदि_अन्तः
#परिभाषय HAVE_FCHOWN 1
#परिभाषय HAVE_READLINK 1
#परिभाषय HAVE_LSTAT 1
#यदि_अन्तः /* defined(_WRS_KERNEL) */
