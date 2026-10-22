/*
** 2015 January 12
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
** This file contains code that is specific to MSVC.
*/
#که_نه_تعریف_شوی SQLITE_MSVC_H
#تعریف_کړه SQLITE_MSVC_H

#إذا defined(_MSC_VER)
#لارښوونه warning(disable : 4054)
#لارښوونه warning(disable : 4055)
#لارښوونه warning(disable : 4100)
#لارښوونه warning(disable : 4127)
#لارښوونه warning(disable : 4130)
#لارښوونه warning(disable : 4152)
#لارښوونه warning(disable : 4189)
#لارښوونه warning(disable : 4206)
#لارښوونه warning(disable : 4210)
#لارښوونه warning(disable : 4232)
#لارښوونه warning(disable : 4244)
#لارښوونه warning(disable : 4305)
#لارښوونه warning(disable : 4306)
#لارښوونه warning(disable : 4702)
#لارښوونه warning(disable : 4706)
#د_که_پای /* defined(_MSC_VER) */

#إذا defined(_MSC_VER) && !defined(_WIN64)
#تعریف_لغوه SQLITE_4_BYTE_ALIGNED_MALLOC
#تعریف_کړه SQLITE_4_BYTE_ALIGNED_MALLOC
#د_که_پای /* defined(_MSC_VER) && !defined(_WIN64) */

#إذا !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800
#تعریف_کړه HAVE_LOG2 0
#د_که_پای /* !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800 */

#د_که_پای /* SQLITE_MSVC_H */
