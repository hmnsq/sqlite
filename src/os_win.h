/*
** 2013 November 25
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
** This file contains code that is specific to Windows.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OS_WIN_H
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_OS_WIN_H

/*
** Include the primary Windows SDK header file.
*/
#ಅನ್ತರ್ಭಾವ "windows.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __CYGWIN__
# include <sys/cygwin.h>
# include <sys/stat.h> /* amalgamator: dontcache */
# include <unistd.h> /* amalgamator: dontcache */
# include <errno.h> /* amalgamator: dontcache */
#ಆದರೆ_ಅಂತ್ಯ

/*
** For some Windows sub-platforms, the _beginthreadex() / _endthreadex()
** functions are not available (e.g. those not using MSVC, Cygwin, etc).
*/
#ಆದರೆ SQLITE_OS_WIN && SQLITE_THREADSAFE>0 && !defined(__CYGWIN__)
# define SQLITE_OS_WIN_ಎಳೆಗಳು 1
#ಇಲ್ಲದಿದ್ದರೆ
# define SQLITE_OS_WIN_ಎಳೆಗಳು 0
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OS_WIN_H */
