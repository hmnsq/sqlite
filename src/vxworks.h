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
#अगर defined(__RTP__) || defined(_WRS_KERNEL)
/* This is VxWorks.  Set up things specially for that OS
*/
#समावेश <vxWorks.h>
#समावेश <pthread.h>  /* amalgamator: dontcache */
#घोषणा OS_VXWORKS 1
#घोषणा SQLITE_OS_OTHER 0
#घोषणा SQLITE_HOMEGROWN_RECURSIVE_MUTEX 1
#घोषणा SQLITE_OMIT_LOAD_EXTENSION 1
#घोषणा SQLITE_ENABLE_LOCKING_STYLE 0
#घोषणा HAVE_UTIME 1
#अन्यथा
/* This is not VxWorks. */
#अगर_अघोषित OS_VXWORKS
#  define OS_VXWORKS 0
#पूर्ण_अगर
#घोषणा HAVE_FCHOWN 1
#घोषणा HAVE_READLINK 1
#घोषणा HAVE_LSTAT 1
#पूर्ण_अगर /* defined(_WRS_KERNEL) */
