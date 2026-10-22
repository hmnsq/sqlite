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
#अगर_अघोषित SQLITE_OS_WIN_H
#घोषणा SQLITE_OS_WIN_H

/*
** Include the primary Windows SDK header file.
*/
#समावेश "windows.h"

#अगर_घोषित __CYGWIN__
# include <sys/cygwin.h>
# include <sys/stat.h> /* amalgamator: dontcache */
# include <unistd.h> /* amalgamator: dontcache */
# include <errno.h> /* amalgamator: dontcache */
#पूर्ण_अगर

/*
** For some Windows sub-platforms, the _beginthreadex() / _endthreadex()
** functions are not available (e.g. those not using MSVC, Cygwin, etc).
*/
#अगर SQLITE_OS_WIN && SQLITE_THREADSAFE>0 && !defined(__CYGWIN__)
# define SQLITE_OS_WIN_THREADS 1
#अन्यथा
# define SQLITE_OS_WIN_THREADS 0
#पूर्ण_अगर

#पूर्ण_अगर /* SQLITE_OS_WIN_H */
