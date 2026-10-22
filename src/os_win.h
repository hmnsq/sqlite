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
#यदि_अपरिभाषितम् SQLITE_OS_WIN_H
#परिभाषय SQLITE_OS_WIN_H

/*
** Include the primary Windows SDK header file.
*/
#अन्तर्भाव "windows.h"

#यदि_परिभाषितम् __CYGWIN__
# include <sys/cygwin.h>
# include <sys/stat.h> /* amalgamator: dontcache */
# include <unistd.h> /* amalgamator: dontcache */
# include <errno.h> /* amalgamator: dontcache */
#यदि_अन्तः

/*
** For some Windows sub-platforms, the _beginthreadex() / _endthreadex()
** functions are not available (e.g. those not using MSVC, Cygwin, etc).
*/
#यदि SQLITE_OS_WIN && SQLITE_THREADSAFE>0 && !defined(__CYGWIN__)
# define SQLITE_OS_WIN_तन्तवः 1
#अन्यथा
# define SQLITE_OS_WIN_तन्तवः 0
#यदि_अन्तः

#यदि_अन्तः /* SQLITE_OS_WIN_H */
