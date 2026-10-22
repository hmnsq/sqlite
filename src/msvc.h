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
#यदि_अपरिभाषितम् SQLITE_MSVC_H
#परिभाषय SQLITE_MSVC_H

#यदि defined(_MSC_VER)
#निर्देशः warning(disable : 4054)
#निर्देशः warning(disable : 4055)
#निर्देशः warning(disable : 4100)
#निर्देशः warning(disable : 4127)
#निर्देशः warning(disable : 4130)
#निर्देशः warning(disable : 4152)
#निर्देशः warning(disable : 4189)
#निर्देशः warning(disable : 4206)
#निर्देशः warning(disable : 4210)
#निर्देशः warning(disable : 4232)
#निर्देशः warning(disable : 4244)
#निर्देशः warning(disable : 4305)
#निर्देशः warning(disable : 4306)
#निर्देशः warning(disable : 4702)
#निर्देशः warning(disable : 4706)
#यदि_अन्तः /* defined(_MSC_VER) */

#यदि defined(_MSC_VER) && !defined(_WIN64)
#अपरिभाषय SQLITE_4_BYTE_ALIGNED_MALLOC
#परिभाषय SQLITE_4_BYTE_ALIGNED_MALLOC
#यदि_अन्तः /* defined(_MSC_VER) && !defined(_WIN64) */

#यदि !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800
#परिभाषय HAVE_LOG2 0
#यदि_अन्तः /* !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800 */

#यदि_अन्तः /* SQLITE_MSVC_H */
