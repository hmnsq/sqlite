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
#অন্তর্ভাব "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#ifdef SQLITE_SYSTEM_MALLOC
#যদি defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)

/*
** Use the zone allocator available on apple products unless the
** SQLITE_WITHOUT_ZONEMALLOC symbol is defined.
*/
#অন্তর্ভাব <sys/sysctl.h>
#অন্তর্ভাব <malloc/malloc.h>
#ifdef SQLITE_MIGHT_BE_SINGLE_CORE
#অন্তর্ভাব <libkern/OSAtomic.h>
#endif /* SQLITE_MIGHT_BE_SINGLE_CORE */
স্থির malloc_zone_t* _sqliteZone_;
#define SQLITE_MALLOC(x) malloc_zone_malloc(_sqliteZone_, (x))
#define SQLITE_FREE(x) malloc_zone_free(_sqliteZone_, (x));
#define SQLITE_REALLOC(x,y) malloc_zone_realloc(_sqliteZone_, (x), (y))
#define SQLITE_MALLOCSIZE(x) \
        (_sqliteZone_ ? _sqliteZone_->আকার_2(_sqliteZone_,x) : malloc_আকার(x))

#অন্যথা /* if not __APPLE__ */

/*
** Use standard C library malloc and free on non-Apple systems.  
** Also used by Apple systems if SQLITE_WITHOUT_ZONEMALLOC is defined.
*/
#define SQLITE_MALLOC(x)             malloc(x)
#define SQLITE_FREE(x)               মুক্ত(x)
#define SQLITE_REALLOC(x,y)          realloc((x),(y))

/*
** The malloc.h header file is needed for malloc_usable_size() function
** on some systems (e.g. Linux).
*/
#যদি HAVE_MALLOC_H && HAVE_MALLOC_USABLE_আকার
#  define SQLITE_USE_MALLOC_H 1
#  define SQLITE_USE_MALLOC_USABLE_আকার 1
/*
** The MSVCRT has malloc_usable_size(), but it is called _msize().  The
** use of _msize() is automatic, but can be disabled by compiling with
** -DSQLITE_WITHOUT_MSIZE.  Using the _msize() function also requires
** the malloc.h header file.
*/
#নতুবা_যদি defined(_MSC_VER) && !defined(SQLITE_WITHOUT_MSIZE)
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
#যদি defined(SQLITE_USE_MALLOC_H)
#  include <malloc.h>
#  যদি defined(SQLITE_USE_MALLOC_USABLE_আকার)
#    যদি !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE(x)   malloc_usable_আকার(x)
#    endif
#  elif defined(SQLITE_USE_MSIZE)
#    যদি !defined(SQLITE_MALLOCSIZE)
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
স্থির শূন্য *sqlite3MemMalloc(পূর্ণসংখ্যা nByte){
#ifdef SQLITE_MALLOCSIZE
  শূন্য *p;
  testcase( ROUND8(nByte)==nByte );
  p = SQLITE_MALLOC( nByte );
  যদি( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  ফেরত p;
#অন্যথা
  sqlite3_int64 *p;
  প্রতিজ্ঞা( nByte>0 );
  testcase( ROUND8(nByte)!=nByte );
  p = SQLITE_MALLOC( nByte+8 );
  যদি( p ){
    p[0] = nByte;
    p++;
  }অন্যথা{
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  ফেরত (শূন্য *)p;
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
স্থির শূন্য sqlite3MemFree(শূন্য *pPrior){
#ifdef SQLITE_MALLOCSIZE
  SQLITE_FREE(pPrior);
#অন্যথা
  sqlite3_int64 *p = (sqlite3_int64*)pPrior;
  প্রতিজ্ঞা( pPrior!=0 );
  p--;
  SQLITE_FREE(p);
#endif
}

/*
** Report the allocated size of a prior return from xMalloc()
** or xRealloc().
*/
স্থির পূর্ণসংখ্যা sqlite3_Mem_আকার(শূন্য *pPrior){
#ifdef SQLITE_MALLOCSIZE
  প্রতিজ্ঞা( pPrior!=0 );
  ফেরত (পূর্ণসংখ্যা)SQLITE_MALLOCSIZE(pPrior);
#অন্যথা
  sqlite3_int64 *p;
  প্রতিজ্ঞা( pPrior!=0 );
  p = (sqlite3_int64*)pPrior;
  p--;
  ফেরত (পূর্ণসংখ্যা)p[0];
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
স্থির শূন্য *sqlite3MemRealloc(শূন্য *pPrior, পূর্ণসংখ্যা nByte){
#ifdef SQLITE_MALLOCSIZE
  শূন্য *p = SQLITE_REALLOC(pPrior, nByte);
  যদি( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      SQLITE_MALLOCSIZE(pPrior), nByte);
  }
  ফেরত p;
#অন্যথা
  sqlite3_int64 *p = (sqlite3_int64*)pPrior;
  প্রতিজ্ঞা( pPrior!=0 && nByte>0 );
  প্রতিজ্ঞা( nByte==ROUND8(nByte) ); /* EV: R-46199-30249 */
  p--;
  p = SQLITE_REALLOC(p, nByte+8 );
  যদি( p ){
    p[0] = nByte;
    p++;
  }অন্যথা{
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      sqlite3_Mem_আকার(pPrior), nByte);
  }
  ফেরত (শূন্য*)p;
#endif
}

/*
** Round up a request size to the next valid allocation size.
*/
স্থির পূর্ণসংখ্যা sqlite3MemRoundup(পূর্ণসংখ্যা n){
  ফেরত ROUND8(n);
}

/*
** Initialize this module.
*/
স্থির পূর্ণসংখ্যা sqlite3MemInit(শূন্য *NotUsed){
#যদি defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)
  পূর্ণসংখ্যা cpu_গুনতি;
  size_t len;
  যদি( _sqliteZone_ ){
    ফেরত SQLITE_OK;
  }
  len = আকার(cpu_গুনতি);
  /* One usually wants to use hw.activecpu for MT decisions, but not here */
  sysctlbyname("hw.ncpu", &cpu_গুনতি, &len, ফাঁকা, 0);
  যদি( cpu_গুনতি>1 ){
    /* defer MT decisions to system malloc */
    _sqliteZone_ = malloc_default_zone();
  }অন্যথা{
    /* only 1 core, use our own zone to contention over global locks,
    ** e.g. we have our own dedicated locks */
    _sqliteZone_ = malloc_create_zone(4096, 0);
    malloc_set_zone_name(_sqliteZone_, "Sqlite_Heap");
  }
#endif /*  defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC) */
  UNUSED_PARAMETER(NotUsed);
  ফেরত SQLITE_OK;
}

/*
** Deinitialize this module.
*/
স্থির শূন্য sqlite3MemShutdown(শূন্য *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  ফেরত;
}

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
শূন্য sqlite3MemSetDefault(শূন্য){
  স্থির ধ্রুবক sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_আকার,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#endif /* SQLITE_SYSTEM_MALLOC */
