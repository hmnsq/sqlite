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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MSVC_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MSVC_H

#ਜੇ defined(_MSC_VER)
#ਹਦਾਇਤ warning(disable : 4054)
#ਹਦਾਇਤ warning(disable : 4055)
#ਹਦਾਇਤ warning(disable : 4100)
#ਹਦਾਇਤ warning(disable : 4127)
#ਹਦਾਇਤ warning(disable : 4130)
#ਹਦਾਇਤ warning(disable : 4152)
#ਹਦਾਇਤ warning(disable : 4189)
#ਹਦਾਇਤ warning(disable : 4206)
#ਹਦਾਇਤ warning(disable : 4210)
#ਹਦਾਇਤ warning(disable : 4232)
#ਹਦਾਇਤ warning(disable : 4244)
#ਹਦਾਇਤ warning(disable : 4305)
#ਹਦਾਇਤ warning(disable : 4306)
#ਹਦਾਇਤ warning(disable : 4702)
#ਹਦਾਇਤ warning(disable : 4706)
#ਜੇ_ਅੰਤ /* defined(_MSC_VER) */

#ਜੇ defined(_MSC_VER) && !defined(_WIN64)
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SQLITE_4_BYTE_ALIGNED_MALLOC
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_4_BYTE_ALIGNED_MALLOC
#ਜੇ_ਅੰਤ /* defined(_MSC_VER) && !defined(_WIN64) */

#ਜੇ !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_LOG2 0
#ਜੇ_ਅੰਤ /* !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800 */

#ਜੇ_ਅੰਤ /* SQLITE_MSVC_H */
