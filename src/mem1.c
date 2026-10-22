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
#ضمن "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#جيڪڏهن_وصف_ٿيل SQLITE_SYSTEM_MALLOC
#إذا defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)

/*
** Use the zone allocator available on apple products unless the
** SQLITE_WITHOUT_ZONEMALLOC symbol is defined.
*/
#ضمن <sys/sysctl.h>
#ضمن <malloc/malloc.h>
#جيڪڏهن_وصف_ٿيل SQLITE_MIGHT_BE_SINGLE_CORE
#ضمن <libkern/OSAtomic.h>
#جيڪڏهن_پورو /* SQLITE_MIGHT_BE_SINGLE_CORE */
ساكن malloc_zone_t* _sqliteZone_;
#وصف_ڪريو SQLITE_MALLOC(x) malloc_zone_malloc(_sqliteZone_, (x))
#وصف_ڪريو SQLITE_FREE(x) malloc_zone_free(_sqliteZone_, (x));
#وصف_ڪريو SQLITE_REALLOC(x,y) malloc_zone_realloc(_sqliteZone_, (x), (y))
#وصف_ڪريو SQLITE_MALLOCSIZE(x) \
        (_sqliteZone_ ? _sqliteZone_->سائيز(_sqliteZone_,x) : malloc_سائيز(x))

#وإلا /* if not __APPLE__ */

/*
** Use standard C library malloc and free on non-Apple systems.  
** Also used by Apple systems if SQLITE_WITHOUT_ZONEMALLOC is defined.
*/
#وصف_ڪريو SQLITE_MALLOC(x)             يادگيري_ڏيو(x)
#وصف_ڪريو SQLITE_FREE(x)               حرر(x)
#وصف_ڪريو SQLITE_REALLOC(x,y)          realloc((x),(y))

/*
** The malloc.h header file is needed for malloc_usable_size() function
** on some systems (e.g. Linux).
*/
#إذا HAVE_MALLOC_H && HAVE_MALLOC_USABLE_سائيز
#  define SQLITE_USE_MALLOC_H 1
#  define SQLITE_USE_MALLOC_USABLE_سائيز 1
/*
** The MSVCRT has malloc_usable_size(), but it is called _msize().  The
** use of _msize() is automatic, but can be disabled by compiling with
** -DSQLITE_WITHOUT_MSIZE.  Using the _msize() function also requires
** the malloc.h header file.
*/
#وإلاإذا defined(_MSC_VER) && !defined(SQLITE_WITHOUT_MSIZE)
#  define SQLITE_USE_MALLOC_H
#  define SQLITE_USE_MSIZE
#جيڪڏهن_پورو

/*
** Include the malloc.h header file, if necessary.  Also set define macro
** SQLITE_MALLOCSIZE to the appropriate function name, which is _msize()
** for MSVC and malloc_usable_size() for most other systems (e.g. Linux).
** The memory size function can always be overridden manually by defining
** the macro SQLITE_MALLOCSIZE to the desired function name.
*/
#إذا defined(SQLITE_USE_MALLOC_H)
#  include <malloc.h>
#  إذا defined(SQLITE_USE_MALLOC_USABLE_سائيز)
#    إذا !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE(x)   malloc_usable_سائيز(x)
#    endif
#  elif defined(SQLITE_USE_MSIZE)
#    إذا !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE      _msize
#    endif
#  endif
#جيڪڏهن_پورو /* defined(SQLITE_USE_MALLOC_H) */

#جيڪڏهن_پورو /* __APPLE__ or not __APPLE__ */

/*
** Like malloc(), but remember the size of the allocation
** so that we can find it later using sqlite3MemSize().
**
** For this low-level routine, we are guaranteed that nByte>0 because
** cases of nByte<=0 will be intercepted and dealt with by higher level
** routines.
*/
ساكن فراغ *sqlite3MemMalloc(صحيح nByte){
#جيڪڏهن_وصف_ٿيل SQLITE_MALLOCSIZE
  فراغ *p;
  testcase( ROUND8(nByte)==nByte );
  p = SQLITE_MALLOC( nByte );
  إذا( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  أرجع p;
#وإلا
  sqlite3_int64 *p;
  أكد( nByte>0 );
  testcase( ROUND8(nByte)!=nByte );
  p = SQLITE_MALLOC( nByte+8 );
  إذا( p ){
    p[0] = nByte;
    p++;
  }وإلا{
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  أرجع (فراغ *)p;
#جيڪڏهن_پورو
}

/*
** Like free() but works for allocations obtained from sqlite3MemMalloc()
** or sqlite3MemRealloc().
**
** For this low-level routine, we already know that pPrior!=0 since
** cases where pPrior==0 will have been intercepted and dealt with
** by higher-level routines.
*/
ساكن فراغ sqlite3MemFree(فراغ *p_اڳواٽ){
#جيڪڏهن_وصف_ٿيل SQLITE_MALLOCSIZE
  SQLITE_FREE(p_اڳواٽ);
#وإلا
  sqlite3_int64 *p = (sqlite3_int64*)p_اڳواٽ;
  أكد( p_اڳواٽ!=0 );
  p--;
  SQLITE_FREE(p);
#جيڪڏهن_پورو
}

/*
** Report the allocated size of a prior return from xMalloc()
** or xRealloc().
*/
ساكن صحيح sqlite3_Mem_سائيز(فراغ *p_اڳواٽ){
#جيڪڏهن_وصف_ٿيل SQLITE_MALLOCSIZE
  أكد( p_اڳواٽ!=0 );
  أرجع (صحيح)SQLITE_MALLOCSIZE(p_اڳواٽ);
#وإلا
  sqlite3_int64 *p;
  أكد( p_اڳواٽ!=0 );
  p = (sqlite3_int64*)p_اڳواٽ;
  p--;
  أرجع (صحيح)p[0];
#جيڪڏهن_پورو
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
ساكن فراغ *sqlite3MemRealloc(فراغ *p_اڳواٽ, صحيح nByte){
#جيڪڏهن_وصف_ٿيل SQLITE_MALLOCSIZE
  فراغ *p = SQLITE_REALLOC(p_اڳواٽ, nByte);
  إذا( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      SQLITE_MALLOCSIZE(p_اڳواٽ), nByte);
  }
  أرجع p;
#وإلا
  sqlite3_int64 *p = (sqlite3_int64*)p_اڳواٽ;
  أكد( p_اڳواٽ!=0 && nByte>0 );
  أكد( nByte==ROUND8(nByte) ); /* EV: R-46199-30249 */
  p--;
  p = SQLITE_REALLOC(p, nByte+8 );
  إذا( p ){
    p[0] = nByte;
    p++;
  }وإلا{
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      sqlite3_Mem_سائيز(p_اڳواٽ), nByte);
  }
  أرجع (فراغ*)p;
#جيڪڏهن_پورو
}

/*
** Round up a request size to the next valid allocation size.
*/
ساكن صحيح sqlite3MemRoundup(صحيح n){
  أرجع ROUND8(n);
}

/*
** Initialize this module.
*/
ساكن صحيح sqlite3MemInit(فراغ *Not_استعمال_ٿيل){
#إذا defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)
  صحيح cpu_ڳڻپ;
  size_t len;
  إذا( _sqliteZone_ ){
    أرجع SQLITE_OK;
  }
  len = حجم(cpu_ڳڻپ);
  /* One usually wants to use hw.activecpu for MT decisions, but not here */
  sysctlbyname("hw.ncpu", &cpu_ڳڻپ, &len, عدم, 0);
  إذا( cpu_ڳڻپ>1 ){
    /* defer MT decisions to system malloc */
    _sqliteZone_ = malloc_default_zone();
  }وإلا{
    /* only 1 core, use our own zone to contention over global locks,
    ** e.g. we have our own dedicated locks */
    _sqliteZone_ = malloc_create_zone(4096, 0);
    malloc_سيٽ_zone_name(_sqliteZone_, "Sqlite_Heap");
  }
#جيڪڏهن_پورو /*  defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC) */
  اڻ_استعمال_ٿيل_PARAMETER(Not_استعمال_ٿيل);
  أرجع SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ساكن فراغ sqlite3MemShutdown(فراغ *Not_استعمال_ٿيل){
  اڻ_استعمال_ٿيل_PARAMETER(Not_استعمال_ٿيل);
  أرجع;
}

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
فراغ sqlite3_Mem_سيٽ_Default(فراغ){
  ساكن ثابت sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_سائيز,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#جيڪڏهن_پورو /* SQLITE_SYSTEM_MALLOC */
