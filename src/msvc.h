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
#اگر_تعریف_نشده SQLITE_MSVC_H
#تعریف_کن SQLITE_MSVC_H

#إذا defined(_MSC_VER)
#رهنمود warning(disable : 4054)
#رهنمود warning(disable : 4055)
#رهنمود warning(disable : 4100)
#رهنمود warning(disable : 4127)
#رهنمود warning(disable : 4130)
#رهنمود warning(disable : 4152)
#رهنمود warning(disable : 4189)
#رهنمود warning(disable : 4206)
#رهنمود warning(disable : 4210)
#رهنمود warning(disable : 4232)
#رهنمود warning(disable : 4244)
#رهنمود warning(disable : 4305)
#رهنمود warning(disable : 4306)
#رهنمود warning(disable : 4702)
#رهنمود warning(disable : 4706)
#پایان_اگر /* defined(_MSC_VER) */

#إذا defined(_MSC_VER) && !defined(_WIN64)
#لغو_تعریف SQLITE_4_BYTE_ALIGNED_MALLOC
#تعریف_کن SQLITE_4_BYTE_ALIGNED_MALLOC
#پایان_اگر /* defined(_MSC_VER) && !defined(_WIN64) */

#إذا !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800
#تعریف_کن HAVE_LOG2 0
#پایان_اگر /* !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800 */

#پایان_اگر /* SQLITE_MSVC_H */
