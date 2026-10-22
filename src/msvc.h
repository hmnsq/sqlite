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
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MSVC_H
#وصف_ڪريو SQLITE_MSVC_H

#إذا defined(_MSC_VER)
#هدايت warning(disable : 4054)
#هدايت warning(disable : 4055)
#هدايت warning(disable : 4100)
#هدايت warning(disable : 4127)
#هدايت warning(disable : 4130)
#هدايت warning(disable : 4152)
#هدايت warning(disable : 4189)
#هدايت warning(disable : 4206)
#هدايت warning(disable : 4210)
#هدايت warning(disable : 4232)
#هدايت warning(disable : 4244)
#هدايت warning(disable : 4305)
#هدايت warning(disable : 4306)
#هدايت warning(disable : 4702)
#هدايت warning(disable : 4706)
#جيڪڏهن_پورو /* defined(_MSC_VER) */

#إذا defined(_MSC_VER) && !defined(_WIN64)
#وصف_رد SQLITE_4_BYTE_ALIGNED_MALLOC
#وصف_ڪريو SQLITE_4_BYTE_ALIGNED_MALLOC
#جيڪڏهن_پورو /* defined(_MSC_VER) && !defined(_WIN64) */

#إذا !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800
#وصف_ڪريو HAVE_LOG2 0
#جيڪڏهن_پورو /* !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800 */

#جيڪڏهن_پورو /* SQLITE_MSVC_H */
