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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OS_WIN_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_OS_WIN_H

/*
** Include the primary Windows SDK header file.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "windows.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __CYGWIN__
# include <sys/cygwin.h>
# include <sys/stat.h> /* amalgamator: dontcache */
# include <unistd.h> /* amalgamator: dontcache */
# include <errno.h> /* amalgamator: dontcache */
#ਜੇ_ਅੰਤ

/*
** For some Windows sub-platforms, the _beginthreadex() / _endthreadex()
** functions are not available (e.g. those not using MSVC, Cygwin, etc).
*/
#ਜੇ SQLITE_OS_WIN && SQLITE_THREADSAFE>0 && !defined(__CYGWIN__)
# define SQLITE_OS_WIN_ਧਾਗੇ 1
#ਨਹੀਂਤਾਂ
# define SQLITE_OS_WIN_ਧਾਗੇ 0
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* SQLITE_OS_WIN_H */
