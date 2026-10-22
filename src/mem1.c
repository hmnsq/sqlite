/*
** 2007 August 14
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains low-level memory allocation drivers for when
** SQLite will use the standard C-library malloc/realloc/free interface
** to obtain the memory it needs.
**
** This file contains implementations of the low-level memory allocation
** routines specified in the sqlite3_mem_methods object.  The content of
** this file is only used if SQLITE_SYSTEM_MALLOC is defined.  The
** SQLITE_SYSTEM_MALLOC macro is defined automatically if neither the
** SQLITE_MEMDEBUG nor the SQLITE_WIN32_MALLOC macros are defined.  The
** default configuration is to use memory allocation routines in this
** file.
**
** C-preprocessor macro summary:
**
**    HAVE_MALLOC_USABLE_SIZE     The configure script sets this symbol if
**                                the malloc_usable_size() interface exists
**                                on the target platform.  Or, this symbol
**                                can be set manually, if desired.
**                                If an equivalent interface exists by
**                                a different name, using a separate -D
**                                option to rename it.
**
**    SQLITE_WITHOUT_ZONEMALLOC   Some older macs lack support for the zone
**                                memory allocator.  Set this symbol to enable
**                                building on older macs.
**
**    SQLITE_WITHOUT_MSIZE        Set this symbol to disable the use of
**                                _msize() on windows systems.  This might
**                                be necessary when compiling for Delphi,
**                                for example.
*/
#شامل "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#ifdef SQLITE_SYSTEM_MALLOC
#اگر defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)

/*
** Use the zone allocator available on apple products unless the
** SQLITE_WITHOUT_ZONEMALLOC symbol is defined.
*/
#شامل <sys/sysctl.h>
#شامل <malloc/malloc.h>
#ifdef SQLITE_MIGHT_BE_SINGLE_CORE
#شامل <libkern/OSAtomic.h>
#endif /* SQLITE_MIGHT_BE_SINGLE_CORE */
ایستا malloc_zone_t* _sqliteZone_;
#define SQLITE_MALLOC(x) malloc_zone_malloc(_sqliteZone_, (x))
#define SQLITE_FREE(x) malloc_zone_free(_sqliteZone_, (x));
#define SQLITE_REALLOC(x,y) malloc_zone_realloc(_sqliteZone_, (x), (y))
#define SQLITE_MALLOCSIZE(x) \
        (_sqliteZone_ ? _sqliteZone_->size(_sqliteZone_,x) : malloc_size(x))

#وگرنه /* if not __APPLE__ */

/*
** Use standard C library malloc and free on non-Apple systems.  
** Also used by Apple systems if SQLITE_WITHOUT_ZONEMALLOC is defined.
*/
#define SQLITE_MALLOC(x)             malloc(x)
#define SQLITE_FREE(x)               آزاد(x)
#define SQLITE_REALLOC(x,y)          realloc((x),(y))

/*
** The malloc.h header file is needed for malloc_usable_size() function
** on some systems (e.g. Linux).
*/
#اگر HAVE_MALLOC_H && HAVE_MALLOC_USABLE_SIZE
#  define SQLITE_USE_MALLOC_H 1
#  define SQLITE_USE_MALLOC_USABLE_SIZE 1
/*
** The MSVCRT has malloc_usable_size(), but it is called _msize().  The
** use of _msize() is automatic, but can be disabled by compiling with
** -DSQLITE_WITHOUT_MSIZE.  Using the _msize() function also requires
** the malloc.h header file.
*/
#وگرنهاگر defined(_MSC_VER) && !defined(SQLITE_WITHOUT_MSIZE)
#  define SQLITE_USE_MALLOC_H
#  define SQLITE_USE_MSIZE
#endif

/*
** Include the malloc.h header file, if necessary.  Also set define macro
** SQLITE_MALLOCSIZE to the appropriate function name, which is _msize()
** for MSVC and malloc_usable_size() for most other systems (e.g. Linux).
** The memory size function can always be overridden manually by defining
** the macro SQLITE_MALLOCSIZE to the desired function name.
*/
#اگر defined(SQLITE_USE_MALLOC_H)
#  include <malloc.h>
#  اگر defined(SQLITE_USE_MALLOC_USABLE_SIZE)
#    اگر !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE(x)   malloc_usable_size(x)
#    endif
#  elif defined(SQLITE_USE_MSIZE)
#    اگر !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE      _msize
#    endif
#  endif
#endif /* defined(SQLITE_USE_MALLOC_H) */

#endif /* __APPLE__ or not __APPLE__ */

/*
** Like malloc(), but remember the size of the allocation
** so that we can find it later using sqlite3MemSize().
**
** For this low-level routine, we are guaranteed that nByte>0 because
** cases of nByte<=0 will be intercepted and dealt with by higher level
** routines.
*/
ایستا تهی *sqlite3MemMalloc(صحیح nByte){
#ifdef SQLITE_MALLOCSIZE
  تهی *p;
  testcase( ROUND8(nByte)==nByte );
  p = SQLITE_MALLOC( nByte );
  اگر( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  بازگشت p;
#وگرنه
  sqlite3_int64 *p;
  أكد( nByte>0 );
  testcase( ROUND8(nByte)!=nByte );
  p = SQLITE_MALLOC( nByte+8 );
  اگر( p ){
    p[0] = nByte;
    p++;
  }وگرنه{
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  بازگشت (تهی *)p;
#endif
}

/*
** Like free() but works for allocations obtained from sqlite3MemMalloc()
** or sqlite3MemRealloc().
**
** For this low-level routine, we already know that pPrior!=0 since
** cases where pPrior==0 will have been intercepted and dealt with
** by higher-level routines.
*/
ایستا تهی sqlite3MemFree(تهی *pPrior){
#ifdef SQLITE_MALLOCSIZE
  SQLITE_FREE(pPrior);
#وگرنه
  sqlite3_int64 *p = (sqlite3_int64*)pPrior;
  أكد( pPrior!=0 );
  p--;
  SQLITE_FREE(p);
#endif
}

/*
** Report the allocated size of a prior return from xMalloc()
** or xRealloc().
*/
ایستا صحیح sqlite3MemSize(تهی *pPrior){
#ifdef SQLITE_MALLOCSIZE
  أكد( pPrior!=0 );
  بازگشت (صحیح)SQLITE_MALLOCSIZE(pPrior);
#وگرنه
  sqlite3_int64 *p;
  أكد( pPrior!=0 );
  p = (sqlite3_int64*)pPrior;
  p--;
  بازگشت (صحیح)p[0];
#endif
}

/*
** Like realloc().  Resize an allocation previously obtained from
** sqlite3MemMalloc().
**
** For this low-level interface, we know that pPrior!=0.  Cases where
** pPrior==0 while have been intercepted by higher-level routine and
** redirected to xMalloc.  Similarly, we know that nByte>0 because
** cases where nByte<=0 will have been intercepted by higher-level
** routines and redirected to xFree.
*/
ایستا تهی *sqlite3MemRealloc(تهی *pPrior, صحیح nByte){
#ifdef SQLITE_MALLOCSIZE
  تهی *p = SQLITE_REALLOC(pPrior, nByte);
  اگر( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      SQLITE_MALLOCSIZE(pPrior), nByte);
  }
  بازگشت p;
#وگرنه
  sqlite3_int64 *p = (sqlite3_int64*)pPrior;
  أكد( pPrior!=0 && nByte>0 );
  أكد( nByte==ROUND8(nByte) ); /* EV: R-46199-30249 */
  p--;
  p = SQLITE_REALLOC(p, nByte+8 );
  اگر( p ){
    p[0] = nByte;
    p++;
  }وگرنه{
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      sqlite3MemSize(pPrior), nByte);
  }
  بازگشت (تهی*)p;
#endif
}

/*
** Round up a request size to the next valid allocation size.
*/
ایستا صحیح sqlite3MemRoundup(صحیح n){
  بازگشت ROUND8(n);
}

/*
** Initialize this module.
*/
ایستا صحیح sqlite3MemInit(تهی *NotUsed){
#اگر defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)
  صحیح cpu_شمار;
  size_t len;
  اگر( _sqliteZone_ ){
    بازگشت SQLITE_OK;
  }
  len = اندازه(cpu_شمار);
  /* One usually wants to use hw.activecpu for MT decisions, but not here */
  sysctlbyname("hw.ncpu", &cpu_شمار, &len, پوچ, 0);
  اگر( cpu_شمار>1 ){
    /* defer MT decisions to system malloc */
    _sqliteZone_ = malloc_default_zone();
  }وگرنه{
    /* only 1 core, use our own zone to contention over global locks,
    ** e.g. we have our own dedicated locks */
    _sqliteZone_ = malloc_create_zone(4096, 0);
    malloc_set_zone_name(_sqliteZone_, "Sqlite_Heap");
  }
#endif /*  defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC) */
  UNUSED_PARAMETER(NotUsed);
  بازگشت SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ایستا تهی sqlite3MemShutdown(تهی *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  بازگشت;
}

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
تهی sqlite3MemSetDefault(تهی){
  ایستا ثابت sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3MemSize,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#endif /* SQLITE_SYSTEM_MALLOC */
