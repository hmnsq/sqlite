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
#ਜੇ defined(__RTP__) || defined(_WRS_KERNEL)
/* This is VxWorks.  Set up things specially for that OS
*/
#ਅਨ੍ਤਰ੍ਭਾਵ <vxWorks.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pthread.h>  /* amalgamator: dontcache */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OS_VXWORKS 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_OS_OTHER 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_HOMEGROWN_RECURSIVE_MUTEX 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_OMIT_ਲੱਦੋ_EXTENSION 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ENABLE_LOCKING_STYLE 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_UTIME 1
#ਨਹੀਂਤਾਂ
/* This is not VxWorks. */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ OS_VXWORKS
#  define OS_VXWORKS 0
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_FCHOWN 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_READLINK 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_LSTAT 1
#ਜੇ_ਅੰਤ /* defined(_WRS_KERNEL) */
