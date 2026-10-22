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
#جے_بنا_تعریف SQLITE_MSVC_H
#تعریف_کرو SQLITE_MSVC_H

#إذا defined(_MSC_VER)
#ہدایت warning(disable : 4054)
#ہدایت warning(disable : 4055)
#ہدایت warning(disable : 4100)
#ہدایت warning(disable : 4127)
#ہدایت warning(disable : 4130)
#ہدایت warning(disable : 4152)
#ہدایت warning(disable : 4189)
#ہدایت warning(disable : 4206)
#ہدایت warning(disable : 4210)
#ہدایت warning(disable : 4232)
#ہدایت warning(disable : 4244)
#ہدایت warning(disable : 4305)
#ہدایت warning(disable : 4306)
#ہدایت warning(disable : 4702)
#ہدایت warning(disable : 4706)
#جے_انت /* defined(_MSC_VER) */

#إذا defined(_MSC_VER) && !defined(_WIN64)
#تعریف_رد SQLITE_4_BYTE_ALIGNED_MALLOC
#تعریف_کرو SQLITE_4_BYTE_ALIGNED_MALLOC
#جے_انت /* defined(_MSC_VER) && !defined(_WIN64) */

#إذا !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800
#تعریف_کرو HAVE_LOG2 0
#جے_انت /* !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800 */

#جے_انت /* SQLITE_MSVC_H */
