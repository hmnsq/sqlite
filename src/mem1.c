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
#అన్తర్భావ "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#నిర్వచితమైతే SQLITE_SYSTEM_MALLOC
#ఒకవేళ defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)

/*
** Use the zone allocator available on apple products unless the
** SQLITE_WITHOUT_ZONEMALLOC symbol is defined.
*/
#అన్తర్భావ <sys/sysctl.h>
#అన్తర్భావ <malloc/malloc.h>
#నిర్వచితమైతే SQLITE_MIGHT_BE_SINGLE_CORE
#అన్తర్భావ <libkern/OSAtomic.h>
#అయితే_ముగింపు /* SQLITE_MIGHT_BE_SINGLE_CORE */
స్థిర malloc_zone_t* _sqliteZone_;
#నిర్వచించు SQLITE_MALLOC(x) malloc_zone_malloc(_sqliteZone_, (x))
#నిర్వచించు SQLITE_FREE(x) malloc_zone_free(_sqliteZone_, (x));
#నిర్వచించు SQLITE_REALLOC(x,y) malloc_zone_realloc(_sqliteZone_, (x), (y))
#నిర్వచించు SQLITE_MALLOCSIZE(x) \
        (_sqliteZone_ ? _sqliteZone_->కొలత(_sqliteZone_,x) : malloc_కొలత(x))

#లేకపోతే /* if not __APPLE__ */

/*
** Use standard C library malloc and free on non-Apple systems.  
** Also used by Apple systems if SQLITE_WITHOUT_ZONEMALLOC is defined.
*/
#నిర్వచించు SQLITE_MALLOC(x)             జ్ఞాపకం_ఇవ్వు(x)
#నిర్వచించు SQLITE_FREE(x)               విడుదల(x)
#నిర్వచించు SQLITE_REALLOC(x,y)          realloc((x),(y))

/*
** The malloc.h header file is needed for malloc_usable_size() function
** on some systems (e.g. Linux).
*/
#ఒకవేళ HAVE_MALLOC_H && HAVE_MALLOC_USABLE_కొలత
#  define SQLITE_USE_MALLOC_H 1
#  define SQLITE_USE_MALLOC_USABLE_కొలత 1
/*
** The MSVCRT has malloc_usable_size(), but it is called _msize().  The
** use of _msize() is automatic, but can be disabled by compiling with
** -DSQLITE_WITHOUT_MSIZE.  Using the _msize() function also requires
** the malloc.h header file.
*/
#లేదావేళ defined(_MSC_VER) && !defined(SQLITE_WITHOUT_MSIZE)
#  define SQLITE_USE_MALLOC_H
#  define SQLITE_USE_MSIZE
#అయితే_ముగింపు

/*
** Include the malloc.h header file, if necessary.  Also set define macro
** SQLITE_MALLOCSIZE to the appropriate function name, which is _msize()
** for MSVC and malloc_usable_size() for most other systems (e.g. Linux).
** The memory size function can always be overridden manually by defining
** the macro SQLITE_MALLOCSIZE to the desired function name.
*/
#ఒకవేళ defined(SQLITE_USE_MALLOC_H)
#  include <malloc.h>
#  ఒకవేళ defined(SQLITE_USE_MALLOC_USABLE_కొలత)
#    ఒకవేళ !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE(x)   malloc_usable_కొలత(x)
#    endif
#  elif defined(SQLITE_USE_MSIZE)
#    ఒకవేళ !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE      _msize
#    endif
#  endif
#అయితే_ముగింపు /* defined(SQLITE_USE_MALLOC_H) */

#అయితే_ముగింపు /* __APPLE__ or not __APPLE__ */

/*
** Like malloc(), but remember the size of the allocation
** so that we can find it later using sqlite3MemSize().
**
** For this low-level routine, we are guaranteed that nByte>0 because
** cases of nByte<=0 will be intercepted and dealt with by higher level
** routines.
*/
స్థిర శూన్య *sqlite3MemMalloc(పూర్ణాంకం nByte){
#నిర్వచితమైతే SQLITE_MALLOCSIZE
  శూన్య *p;
  testcase( ROUND8(nByte)==nByte );
  p = SQLITE_MALLOC( nByte );
  ఒకవేళ( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  తిరిగి p;
#లేకపోతే
  sqlite3_int64 *p;
  ప్రతిజ్ఞా( nByte>0 );
  testcase( ROUND8(nByte)!=nByte );
  p = SQLITE_MALLOC( nByte+8 );
  ఒకవేళ( p ){
    p[0] = nByte;
    p++;
  }లేకపోతే{
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  తిరిగి (శూన్య *)p;
#అయితే_ముగింపు
}

/*
** Like free() but works for allocations obtained from sqlite3MemMalloc()
** or sqlite3MemRealloc().
**
** For this low-level routine, we already know that pPrior!=0 since
** cases where pPrior==0 will have been intercepted and dealt with
** by higher-level routines.
*/
స్థిర శూన్య sqlite3MemFree(శూన్య *p_పూర్వ){
#నిర్వచితమైతే SQLITE_MALLOCSIZE
  SQLITE_FREE(p_పూర్వ);
#లేకపోతే
  sqlite3_int64 *p = (sqlite3_int64*)p_పూర్వ;
  ప్రతిజ్ఞా( p_పూర్వ!=0 );
  p--;
  SQLITE_FREE(p);
#అయితే_ముగింపు
}

/*
** Report the allocated size of a prior return from xMalloc()
** or xRealloc().
*/
స్థిర పూర్ణాంకం sqlite3_Mem_కొలత(శూన్య *p_పూర్వ){
#నిర్వచితమైతే SQLITE_MALLOCSIZE
  ప్రతిజ్ఞా( p_పూర్వ!=0 );
  తిరిగి (పూర్ణాంకం)SQLITE_MALLOCSIZE(p_పూర్వ);
#లేకపోతే
  sqlite3_int64 *p;
  ప్రతిజ్ఞా( p_పూర్వ!=0 );
  p = (sqlite3_int64*)p_పూర్వ;
  p--;
  తిరిగి (పూర్ణాంకం)p[0];
#అయితే_ముగింపు
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
స్థిర శూన్య *sqlite3MemRealloc(శూన్య *p_పూర్వ, పూర్ణాంకం nByte){
#నిర్వచితమైతే SQLITE_MALLOCSIZE
  శూన్య *p = SQLITE_REALLOC(p_పూర్వ, nByte);
  ఒకవేళ( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      SQLITE_MALLOCSIZE(p_పూర్వ), nByte);
  }
  తిరిగి p;
#లేకపోతే
  sqlite3_int64 *p = (sqlite3_int64*)p_పూర్వ;
  ప్రతిజ్ఞా( p_పూర్వ!=0 && nByte>0 );
  ప్రతిజ్ఞా( nByte==ROUND8(nByte) ); /* EV: R-46199-30249 */
  p--;
  p = SQLITE_REALLOC(p, nByte+8 );
  ఒకవేళ( p ){
    p[0] = nByte;
    p++;
  }లేకపోతే{
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      sqlite3_Mem_కొలత(p_పూర్వ), nByte);
  }
  తిరిగి (శూన్య*)p;
#అయితే_ముగింపు
}

/*
** Round up a request size to the next valid allocation size.
*/
స్థిర పూర్ణాంకం sqlite3MemRoundup(పూర్ణాంకం n){
  తిరిగి ROUND8(n);
}

/*
** Initialize this module.
*/
స్థిర పూర్ణాంకం sqlite3MemInit(శూన్య *Not_వాడిన){
#ఒకవేళ defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)
  పూర్ణాంకం cpu_లెక్క;
  size_t len;
  ఒకవేళ( _sqliteZone_ ){
    తిరిగి SQLITE_OK;
  }
  len = పరిమాణం(cpu_లెక్క);
  /* One usually wants to use hw.activecpu for MT decisions, but not here */
  sysctlbyname("hw.ncpu", &cpu_లెక్క, &len, ఖాళీ, 0);
  ఒకవేళ( cpu_లెక్క>1 ){
    /* defer MT decisions to system malloc */
    _sqliteZone_ = malloc_default_zone();
  }లేకపోతే{
    /* only 1 core, use our own zone to contention over global locks,
    ** e.g. we have our own dedicated locks */
    _sqliteZone_ = malloc_create_zone(4096, 0);
    malloc_సమితి_zone_name(_sqliteZone_, "Sqlite_Heap");
  }
#అయితే_ముగింపు /*  defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC) */
  వాడని_PARAMETER(Not_వాడిన);
  తిరిగి SQLITE_OK;
}

/*
** Deinitialize this module.
*/
స్థిర శూన్య sqlite3MemShutdown(శూన్య *Not_వాడిన){
  వాడని_PARAMETER(Not_వాడిన);
  తిరిగి;
}

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
శూన్య sqlite3_Mem_సమితి_Default(శూన్య){
  స్థిర స్థిరరాశి sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_కొలత,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#అయితే_ముగింపు /* SQLITE_SYSTEM_MALLOC */
