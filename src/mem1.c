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
#جے_تعریف_شدہ SQLITE_SYSTEM_MALLOC
#إذا defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)

/*
** Use the zone allocator available on apple products unless the
** SQLITE_WITHOUT_ZONEMALLOC symbol is defined.
*/
#ضمن <sys/sysctl.h>
#ضمن <malloc/malloc.h>
#جے_تعریف_شدہ SQLITE_MIGHT_BE_SINGLE_CORE
#ضمن <libkern/OSAtomic.h>
#جے_انت /* SQLITE_MIGHT_BE_SINGLE_CORE */
ساكن malloc_zone_t* _sqliteZone_;
#تعریف_کرو SQLITE_MALLOC(x) malloc_zone_malloc(_sqliteZone_, (x))
#تعریف_کرو SQLITE_FREE(x) malloc_zone_free(_sqliteZone_, (x));
#تعریف_کرو SQLITE_REALLOC(x,y) malloc_zone_realloc(_sqliteZone_, (x), (y))
#تعریف_کرو SQLITE_MALLOCSIZE(x) \
        (_sqliteZone_ ? _sqliteZone_->ناپ(_sqliteZone_,x) : malloc_ناپ(x))

#وإلا /* if not __APPLE__ */

/*
** Use standard C library malloc and free on non-Apple systems.  
** Also used by Apple systems if SQLITE_WITHOUT_ZONEMALLOC is defined.
*/
#تعریف_کرو SQLITE_MALLOC(x)             حافظہ_دیو(x)
#تعریف_کرو SQLITE_FREE(x)               حرر(x)
#تعریف_کرو SQLITE_REALLOC(x,y)          realloc((x),(y))

/*
** The malloc.h header file is needed for malloc_usable_size() function
** on some systems (e.g. Linux).
*/
#إذا HAVE_MALLOC_H && HAVE_MALLOC_USABLE_ناپ
#  define SQLITE_USE_MALLOC_H 1
#  define SQLITE_USE_MALLOC_USABLE_ناپ 1
/*
** The MSVCRT has malloc_usable_size(), but it is called _msize().  The
** use of _msize() is automatic, but can be disabled by compiling with
** -DSQLITE_WITHOUT_MSIZE.  Using the _msize() function also requires
** the malloc.h header file.
*/
#وإلاإذا defined(_MSC_VER) && !defined(SQLITE_WITHOUT_MSIZE)
#  define SQLITE_USE_MALLOC_H
#  define SQLITE_USE_MSIZE
#جے_انت

/*
** Include the malloc.h header file, if necessary.  Also set define macro
** SQLITE_MALLOCSIZE to the appropriate function name, which is _msize()
** for MSVC and malloc_usable_size() for most other systems (e.g. Linux).
** The memory size function can always be overridden manually by defining
** the macro SQLITE_MALLOCSIZE to the desired function name.
*/
#إذا defined(SQLITE_USE_MALLOC_H)
#  include <malloc.h>
#  إذا defined(SQLITE_USE_MALLOC_USABLE_ناپ)
#    إذا !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE(x)   malloc_usable_ناپ(x)
#    endif
#  elif defined(SQLITE_USE_MSIZE)
#    إذا !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE      _msize
#    endif
#  endif
#جے_انت /* defined(SQLITE_USE_MALLOC_H) */

#جے_انت /* __APPLE__ or not __APPLE__ */

/*
** Like malloc(), but remember the size of the allocation
** so that we can find it later using sqlite3MemSize().
**
** For this low-level routine, we are guaranteed that nByte>0 because
** cases of nByte<=0 will be intercepted and dealt with by higher level
** routines.
*/
ساكن فراغ *sqlite3MemMalloc(صحيح nByte){
#جے_تعریف_شدہ SQLITE_MALLOCSIZE
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
#جے_انت
}

/*
** Like free() but works for allocations obtained from sqlite3MemMalloc()
** or sqlite3MemRealloc().
**
** For this low-level routine, we already know that pPrior!=0 since
** cases where pPrior==0 will have been intercepted and dealt with
** by higher-level routines.
*/
ساكن فراغ sqlite3MemFree(فراغ *p_پہلوں){
#جے_تعریف_شدہ SQLITE_MALLOCSIZE
  SQLITE_FREE(p_پہلوں);
#وإلا
  sqlite3_int64 *p = (sqlite3_int64*)p_پہلوں;
  أكد( p_پہلوں!=0 );
  p--;
  SQLITE_FREE(p);
#جے_انت
}

/*
** Report the allocated size of a prior return from xMalloc()
** or xRealloc().
*/
ساكن صحيح sqlite3_Mem_ناپ(فراغ *p_پہلوں){
#جے_تعریف_شدہ SQLITE_MALLOCSIZE
  أكد( p_پہلوں!=0 );
  أرجع (صحيح)SQLITE_MALLOCSIZE(p_پہلوں);
#وإلا
  sqlite3_int64 *p;
  أكد( p_پہلوں!=0 );
  p = (sqlite3_int64*)p_پہلوں;
  p--;
  أرجع (صحيح)p[0];
#جے_انت
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
ساكن فراغ *sqlite3MemRealloc(فراغ *p_پہلوں, صحيح nByte){
#جے_تعریف_شدہ SQLITE_MALLOCSIZE
  فراغ *p = SQLITE_REALLOC(p_پہلوں, nByte);
  إذا( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      SQLITE_MALLOCSIZE(p_پہلوں), nByte);
  }
  أرجع p;
#وإلا
  sqlite3_int64 *p = (sqlite3_int64*)p_پہلوں;
  أكد( p_پہلوں!=0 && nByte>0 );
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
      sqlite3_Mem_ناپ(p_پہلوں), nByte);
  }
  أرجع (فراغ*)p;
#جے_انت
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
ساكن صحيح sqlite3MemInit(فراغ *Not_ورتیا){
#إذا defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)
  صحيح cpu_گنتی;
  size_t len;
  إذا( _sqliteZone_ ){
    أرجع SQLITE_OK;
  }
  len = حجم(cpu_گنتی);
  /* One usually wants to use hw.activecpu for MT decisions, but not here */
  sysctlbyname("hw.ncpu", &cpu_گنتی, &len, عدم, 0);
  إذا( cpu_گنتی>1 ){
    /* defer MT decisions to system malloc */
    _sqliteZone_ = malloc_default_zone();
  }وإلا{
    /* only 1 core, use our own zone to contention over global locks,
    ** e.g. we have our own dedicated locks */
    _sqliteZone_ = malloc_create_zone(4096, 0);
    malloc_سیٹ_zone_name(_sqliteZone_, "Sqlite_Heap");
  }
#جے_انت /*  defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC) */
  بن_ورتیا_PARAMETER(Not_ورتیا);
  أرجع SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ساكن فراغ sqlite3MemShutdown(فراغ *Not_ورتیا){
  بن_ورتیا_PARAMETER(Not_ورتیا);
  أرجع;
}

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
فراغ sqlite3_Mem_سیٹ_Default(فراغ){
  ساكن ثابت sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_ناپ,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#جے_انت /* SQLITE_SYSTEM_MALLOC */
