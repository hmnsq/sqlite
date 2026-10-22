/*
** 2008 May 27
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
** This file contains inline asm code for retrieving "high-performance"
** counters for x86 and x86_64 class CPUs.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_HWTIME_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_HWTIME_H

#ਜੇ defined(_MSC_VER) && defined(_WIN32)

  #ਅਨ੍ਤਰ੍ਭਾਵ "windows.h"
  #ਅਨ੍ਤਰ੍ਭਾਵ <profileapi.h>

  __inline sqlite3_uint64 sqlite3Hwtime(ਖਾਲੀ){
    LARGE_INTEGER tm;
    QueryPerformanceCounter(&tm);
    ਵਾਪਸ (sqlite3_uint64)tm.QuadPart;
  }

#ਜਾਂਜੇ !defined(__STRICT_ANSI__) && defined(__GNUC__) && \
    (defined(i586) || defined(__i586__) || defined(_M_IX86))

  __inline__ sqlite_uint64 sqlite3Hwtime(ਖਾਲੀ){
     ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ lo, hi;
     __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
     ਵਾਪਸ (sqlite_uint64)hi << 32 | lo;
  }

#ਜਾਂਜੇ !defined(__STRICT_ANSI__) && (defined(__GNUC__) && defined(__x86_64__))

  __inline__ sqlite_uint64 sqlite3Hwtime(ਖਾਲੀ){
     ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ lo, hi;
     __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
     ਵਾਪਸ (sqlite_uint64)hi << 32 | lo;
  }

#ਜਾਂਜੇ !defined(__STRICT_ANSI__) && defined(__GNUC__) &&  defined(__aarch64__)

  __inline__ sqlite_uint64 sqlite3Hwtime(ਖਾਲੀ){
     sqlite3_uint64 cnt;
     __asm__ __volatile__ ("mrs %0, cntvct_el0" : "=r" (cnt));
     ਵਾਪਸ cnt;
  }
 
#ਜਾਂਜੇ !defined(__STRICT_ANSI__) && (defined(__GNUC__) && defined(__ppc__))

  __inline__ sqlite_uint64 sqlite3Hwtime(ਖਾਲੀ){
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ retval;
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ junk;
      __asm__ __volatile__ ("\n\
          1:      mftbu   %1\n\
                  mftb    %L0\n\
                  mftbu   %0\n\
                  cmpw    %0,%1\n\
                  bne     1b"
                  : "=r" (retval), "=r" (junk));
      ਵਾਪਸ retval;
  }

#ਨਹੀਂਤਾਂ

  /*
  ** asm() is needed for hardware timing support.  Without asm(),
  ** disable the sqlite3Hwtime() routine.
  **
  ** sqlite3Hwtime() is only used for some obscure debugging
  ** and analysis configurations, not in any deliverable, so this
  ** should not be a great loss.
  */
  sqlite_uint64 sqlite3Hwtime(ਖਾਲੀ){ ਵਾਪਸ ((sqlite_uint64)0); }

#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* !defined(SQLITE_HWTIME_H) */
