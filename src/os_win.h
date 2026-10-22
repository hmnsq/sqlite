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
#నిర్వచితం_కాకపోతే SQLITE_OS_WIN_H
#నిర్వచించు SQLITE_OS_WIN_H

/*
** Include the primary Windows SDK header file.
*/
#అన్తర్భావ "windows.h"

#నిర్వచితమైతే __CYGWIN__
# include <sys/cygwin.h>
# include <sys/stat.h> /* amalgamator: dontcache */
# include <unistd.h> /* amalgamator: dontcache */
# include <errno.h> /* amalgamator: dontcache */
#అయితే_ముగింపు

/*
** For some Windows sub-platforms, the _beginthreadex() / _endthreadex()
** functions are not available (e.g. those not using MSVC, Cygwin, etc).
*/
#ఒకవేళ SQLITE_OS_WIN && SQLITE_THREADSAFE>0 && !defined(__CYGWIN__)
# define SQLITE_OS_WIN_దారాలు 1
#లేకపోతే
# define SQLITE_OS_WIN_దారాలు 0
#అయితే_ముగింపు

#అయితే_ముగింపు /* SQLITE_OS_WIN_H */
