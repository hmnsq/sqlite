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
#جے_بنا_تعریف SQLITE_OS_WIN_H
#تعریف_کرو SQLITE_OS_WIN_H

/*
** Include the primary Windows SDK header file.
*/
#ضمن "windows.h"

#جے_تعریف_شدہ __CYGWIN__
# include <sys/cygwin.h>
# include <sys/stat.h> /* amalgamator: dontcache */
# include <unistd.h> /* amalgamator: dontcache */
# include <errno.h> /* amalgamator: dontcache */
#جے_انت

/*
** For some Windows sub-platforms, the _beginthreadex() / _endthreadex()
** functions are not available (e.g. those not using MSVC, Cygwin, etc).
*/
#إذا SQLITE_OS_WIN && SQLITE_THREADSAFE>0 && !defined(__CYGWIN__)
# define SQLITE_OS_WIN_دھاگے 1
#وإلا
# define SQLITE_OS_WIN_دھاگے 0
#جے_انت

#جے_انت /* SQLITE_OS_WIN_H */
