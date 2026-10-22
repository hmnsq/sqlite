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
#ifndef SQLITE_HWTIME_H
#define SQLITE_HWTIME_H

#எனில் defined(_MSC_VER) && defined(_WIN32)

  #அந்தர்பாவ "windows.h"
  #அந்தர்பாவ <profileapi.h>

  __inline sqlite3_uint64 sqlite3Hwtime(வெறுமை){
    LARGE_INTEGER tm;
    QueryPerformanceCounter(&tm);
    திரும்பு (sqlite3_uint64)tm.QuadPart;
  }

#அல்லதெனில் !defined(__STRICT_ANSI__) && defined(__GNUC__) && \
    (defined(i586) || defined(__i586__) || defined(_M_IX86))

  __inline__ sqlite_uint64 sqlite3Hwtime(வெறுமை){
     unsigned முழு lo, hi;
     __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
     திரும்பு (sqlite_uint64)hi << 32 | lo;
  }

#அல்லதெனில் !defined(__STRICT_ANSI__) && (defined(__GNUC__) && defined(__x86_64__))

  __inline__ sqlite_uint64 sqlite3Hwtime(வெறுமை){
     unsigned முழு lo, hi;
     __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
     திரும்பு (sqlite_uint64)hi << 32 | lo;
  }

#அல்லதெனில் !defined(__STRICT_ANSI__) && defined(__GNUC__) &&  defined(__aarch64__)

  __inline__ sqlite_uint64 sqlite3Hwtime(வெறுமை){
     sqlite3_uint64 cnt;
     __asm__ __volatile__ ("mrs %0, cntvct_el0" : "=r" (cnt));
     திரும்பு cnt;
  }
 
#அல்லதெனில் !defined(__STRICT_ANSI__) && (defined(__GNUC__) && defined(__ppc__))

  __inline__ sqlite_uint64 sqlite3Hwtime(வெறுமை){
      unsigned long long retval;
      unsigned long junk;
      __asm__ __volatile__ ("\n\
          1:      mftbu   %1\n\
                  mftb    %L0\n\
                  mftbu   %0\n\
                  cmpw    %0,%1\n\
                  bne     1b"
                  : "=r" (retval), "=r" (junk));
      திரும்பு retval;
  }

#இல்லையெனில்

  /*
  ** asm() is needed for hardware timing support.  Without asm(),
  ** disable the sqlite3Hwtime() routine.
  **
  ** sqlite3Hwtime() is only used for some obscure debugging
  ** and analysis configurations, not in any deliverable, so this
  ** should not be a great loss.
  */
  sqlite_uint64 sqlite3Hwtime(வெறுமை){ திரும்பு ((sqlite_uint64)0); }

#endif

#endif /* !defined(SQLITE_HWTIME_H) */
