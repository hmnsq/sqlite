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
#अगर_अघोषित SQLITE_MSVC_H
#घोषणा SQLITE_MSVC_H

#अगर defined(_MSC_VER)
#आशय warning(disable : 4054)
#आशय warning(disable : 4055)
#आशय warning(disable : 4100)
#आशय warning(disable : 4127)
#आशय warning(disable : 4130)
#आशय warning(disable : 4152)
#आशय warning(disable : 4189)
#आशय warning(disable : 4206)
#आशय warning(disable : 4210)
#आशय warning(disable : 4232)
#आशय warning(disable : 4244)
#आशय warning(disable : 4305)
#आशय warning(disable : 4306)
#आशय warning(disable : 4702)
#आशय warning(disable : 4706)
#पूर्ण_अगर /* defined(_MSC_VER) */

#अगर defined(_MSC_VER) && !defined(_WIN64)
#अघोषित SQLITE_4_BYTE_ALIGNED_MALLOC
#घोषणा SQLITE_4_BYTE_ALIGNED_MALLOC
#पूर्ण_अगर /* defined(_MSC_VER) && !defined(_WIN64) */

#अगर !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800
#घोषणा HAVE_LOG2 0
#पूर्ण_अगर /* !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800 */

#पूर्ण_अगर /* SQLITE_MSVC_H */
