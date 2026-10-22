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
#ಆದರೆ defined(__RTP__) || defined(_WRS_KERNEL)
/* This is VxWorks.  Set up things specially for that OS
*/
#ಅನ್ತರ್ಭಾವ <vxWorks.h>
#ಅನ್ತರ್ಭಾವ <pthread.h>  /* amalgamator: dontcache */
#ವ್ಯಾಖ್ಯಾನಿಸು OS_VXWORKS 1
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_OS_OTHER 0
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_HOMEGROWN_RECURSIVE_MUTEX 1
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_OMIT_ತುಂಬು_EXTENSION 1
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ENABLE_LOCKING_STYLE 0
#ವ್ಯಾಖ್ಯಾನಿಸು HAVE_UTIME 1
#ಇಲ್ಲದಿದ್ದರೆ
/* This is not VxWorks. */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ OS_VXWORKS
#  define OS_VXWORKS 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು HAVE_FCHOWN 1
#ವ್ಯಾಖ್ಯಾನಿಸು HAVE_READLINK 1
#ವ್ಯಾಖ್ಯಾನಿಸು HAVE_LSTAT 1
#ಆದರೆ_ಅಂತ್ಯ /* defined(_WRS_KERNEL) */
