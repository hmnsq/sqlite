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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MSVC_H
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MSVC_H

#ಆದರೆ defined(_MSC_VER)
#ಸೂಚನೆ warning(disable : 4054)
#ಸೂಚನೆ warning(disable : 4055)
#ಸೂಚನೆ warning(disable : 4100)
#ಸೂಚನೆ warning(disable : 4127)
#ಸೂಚನೆ warning(disable : 4130)
#ಸೂಚನೆ warning(disable : 4152)
#ಸೂಚನೆ warning(disable : 4189)
#ಸೂಚನೆ warning(disable : 4206)
#ಸೂಚನೆ warning(disable : 4210)
#ಸೂಚನೆ warning(disable : 4232)
#ಸೂಚನೆ warning(disable : 4244)
#ಸೂಚನೆ warning(disable : 4305)
#ಸೂಚನೆ warning(disable : 4306)
#ಸೂಚನೆ warning(disable : 4702)
#ಸೂಚನೆ warning(disable : 4706)
#ಆದರೆ_ಅಂತ್ಯ /* defined(_MSC_VER) */

#ಆದರೆ defined(_MSC_VER) && !defined(_WIN64)
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SQLITE_4_BYTE_ALIGNED_MALLOC
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_4_BYTE_ALIGNED_MALLOC
#ಆದರೆ_ಅಂತ್ಯ /* defined(_MSC_VER) && !defined(_WIN64) */

#ಆದರೆ !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800
#ವ್ಯಾಖ್ಯಾನಿಸು HAVE_LOG2 0
#ಆದರೆ_ಅಂತ್ಯ /* !defined(HAVE_LOG2) && defined(_MSC_VER) && _MSC_VER<1800 */

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_MSVC_H */
