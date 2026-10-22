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
#నిర్వచితం_కాకపోతే SQLITE_MSVC_H
#నిర్వచించు SQLITE_MSVC_H

#ఒకవేళ defined(_MSC_VER)
#సూచన warning(disable : 4054)
#సూచన warning(disable : 4055)
#సూచన warning(disable : 4100)
#సూచన warning(disable : 4127)
#సూచన warning(disable : 4130)
#సూచన warning(disable : 4152)
#సూచన warning(disable : 4189)
#సూచన warning(disable : 4206)
#సూచన warning(disable : 4210)
#సూచన warning(disable : 4232)
#సూచన warning(disable : 4244)
#సూచన warning(disable : 4305)
#సూచన warning(disable : 4306)
#సూచన warning(disable : 4702)
#సూచన warning(disable : 4706)
#అయితే_ముగింపు /* defined(_MSC_VER) */

#ఒకవేళ defined(_MSC_VER) && !defined(_WIN64)
#నిర్వచనం_రద్దు SQLITE_4_BYTE_ALIGNED_MALLOC
#నిర్వచించు SQLITE_4_BYTE_ALIGNED_MALLOC
#అయితే_ముగింపు /* defined(_MSC_VER) && !defined(_WIN64) */

#ఒకవేళ !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800
#నిర్వచించు HAVE_LOG2 0
#అయితే_ముగింపు /* !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800 */

#అయితే_ముగింపు /* SQLITE_MSVC_H */
