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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_SYSTEM_MALLOC
#ਜੇ defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)

/*
** Use the zone allocator available on apple products unless the
** SQLITE_WITHOUT_ZONEMALLOC symbol is defined.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/sysctl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc/malloc.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MIGHT_BE_SINGLE_CORE
#ਅਨ੍ਤਰ੍ਭਾਵ <libkern/OSAtomic.h>
#ਜੇ_ਅੰਤ /* SQLITE_MIGHT_BE_SINGLE_CORE */
ਸਥਿਰ malloc_zone_t* _sqliteZone_;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MALLOC(x) malloc_zone_malloc(_sqliteZone_, (x))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FREE(x) malloc_zone_free(_sqliteZone_, (x));
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_REALLOC(x,y) malloc_zone_realloc(_sqliteZone_, (x), (y))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MALLOCSIZE(x) \
        (_sqliteZone_ ? _sqliteZone_->ਮਾਪ(_sqliteZone_,x) : malloc_ਮਾਪ(x))

#ਨਹੀਂਤਾਂ /* if not __APPLE__ */

/*
** Use standard C library malloc and free on non-Apple systems.  
** Also used by Apple systems if SQLITE_WITHOUT_ZONEMALLOC is defined.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MALLOC(x)             ਸਮ੍ਰਿਤੀ_ਦਿਓ(x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FREE(x)               ਮੁਕਤ(x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_REALLOC(x,y)          realloc((x),(y))

/*
** The malloc.h header file is needed for malloc_usable_size() function
** on some systems (e.g. Linux).
*/
#ਜੇ HAVE_MALLOC_H && HAVE_MALLOC_USABLE_ਮਾਪ
#  define SQLITE_USE_MALLOC_H 1
#  define SQLITE_USE_MALLOC_USABLE_ਮਾਪ 1
/*
** The MSVCRT has malloc_usable_size(), but it is called _msize().  The
** use of _msize() is automatic, but can be disabled by compiling with
** -DSQLITE_WITHOUT_MSIZE.  Using the _msize() function also requires
** the malloc.h header file.
*/
#ਜਾਂਜੇ defined(_MSC_VER) && !defined(SQLITE_WITHOUT_MSIZE)
#  define SQLITE_USE_MALLOC_H
#  define SQLITE_USE_MSIZE
#ਜੇ_ਅੰਤ

/*
** Include the malloc.h header file, if necessary.  Also set define macro
** SQLITE_MALLOCSIZE to the appropriate function name, which is _msize()
** for MSVC and malloc_usable_size() for most other systems (e.g. Linux).
** The memory size function can always be overridden manually by defining
** the macro SQLITE_MALLOCSIZE to the desired function name.
*/
#ਜੇ defined(SQLITE_USE_MALLOC_H)
#  include <malloc.h>
#  ਜੇ defined(SQLITE_USE_MALLOC_USABLE_ਮਾਪ)
#    ਜੇ !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE(x)   malloc_usable_ਮਾਪ(x)
#    endif
#  elif defined(SQLITE_USE_MSIZE)
#    ਜੇ !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE      _msize
#    endif
#  endif
#ਜੇ_ਅੰਤ /* defined(SQLITE_USE_MALLOC_H) */

#ਜੇ_ਅੰਤ /* __APPLE__ or not __APPLE__ */

/*
** Like malloc(), but remember the size of the allocation
** so that we can find it later using sqlite3MemSize().
**
** For this low-level routine, we are guaranteed that nByte>0 because
** cases of nByte<=0 will be intercepted and dealt with by higher level
** routines.
*/
ਸਥਿਰ ਖਾਲੀ *sqlite3MemMalloc(ਪੂਰਨ nByte){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MALLOCSIZE
  ਖਾਲੀ *p;
  testcase( ROUND8(nByte)==nByte );
  p = SQLITE_MALLOC( nByte );
  ਜੇ( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  ਵਾਪਸ p;
#ਨਹੀਂਤਾਂ
  sqlite3_int64 *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( nByte>0 );
  testcase( ROUND8(nByte)!=nByte );
  p = SQLITE_MALLOC( nByte+8 );
  ਜੇ( p ){
    p[0] = nByte;
    p++;
  }ਨਹੀਂਤਾਂ{
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  ਵਾਪਸ (ਖਾਲੀ *)p;
#ਜੇ_ਅੰਤ
}

/*
** Like free() but works for allocations obtained from sqlite3MemMalloc()
** or sqlite3MemRealloc().
**
** For this low-level routine, we already know that pPrior!=0 since
** cases where pPrior==0 will have been intercepted and dealt with
** by higher-level routines.
*/
ਸਥਿਰ ਖਾਲੀ sqlite3MemFree(ਖਾਲੀ *p_ਪੂਰਵ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MALLOCSIZE
  SQLITE_FREE(p_ਪੂਰਵ);
#ਨਹੀਂਤਾਂ
  sqlite3_int64 *p = (sqlite3_int64*)p_ਪੂਰਵ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੂਰਵ!=0 );
  p--;
  SQLITE_FREE(p);
#ਜੇ_ਅੰਤ
}

/*
** Report the allocated size of a prior return from xMalloc()
** or xRealloc().
*/
ਸਥਿਰ ਪੂਰਨ sqlite3_Mem_ਮਾਪ(ਖਾਲੀ *p_ਪੂਰਵ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MALLOCSIZE
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੂਰਵ!=0 );
  ਵਾਪਸ (ਪੂਰਨ)SQLITE_MALLOCSIZE(p_ਪੂਰਵ);
#ਨਹੀਂਤਾਂ
  sqlite3_int64 *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੂਰਵ!=0 );
  p = (sqlite3_int64*)p_ਪੂਰਵ;
  p--;
  ਵਾਪਸ (ਪੂਰਨ)p[0];
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ ਖਾਲੀ *sqlite3MemRealloc(ਖਾਲੀ *p_ਪੂਰਵ, ਪੂਰਨ nByte){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MALLOCSIZE
  ਖਾਲੀ *p = SQLITE_REALLOC(p_ਪੂਰਵ, nByte);
  ਜੇ( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      SQLITE_MALLOCSIZE(p_ਪੂਰਵ), nByte);
  }
  ਵਾਪਸ p;
#ਨਹੀਂਤਾਂ
  sqlite3_int64 *p = (sqlite3_int64*)p_ਪੂਰਵ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੂਰਵ!=0 && nByte>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( nByte==ROUND8(nByte) ); /* EV: R-46199-30249 */
  p--;
  p = SQLITE_REALLOC(p, nByte+8 );
  ਜੇ( p ){
    p[0] = nByte;
    p++;
  }ਨਹੀਂਤਾਂ{
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      sqlite3_Mem_ਮਾਪ(p_ਪੂਰਵ), nByte);
  }
  ਵਾਪਸ (ਖਾਲੀ*)p;
#ਜੇ_ਅੰਤ
}

/*
** Round up a request size to the next valid allocation size.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3MemRoundup(ਪੂਰਨ n){
  ਵਾਪਸ ROUND8(n);
}

/*
** Initialize this module.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3MemInit(ਖਾਲੀ *Not_ਵਰਤਿਆ){
#ਜੇ defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)
  ਪੂਰਨ cpu_ਗਿਣਤੀ;
  size_t len;
  ਜੇ( _sqliteZone_ ){
    ਵਾਪਸ SQLITE_OK;
  }
  len = ਆਕਾਰ(cpu_ਗਿਣਤੀ);
  /* One usually wants to use hw.activecpu for MT decisions, but not here */
  sysctlbyname("hw.ncpu", &cpu_ਗਿਣਤੀ, &len, ਨਿਰਾ, 0);
  ਜੇ( cpu_ਗਿਣਤੀ>1 ){
    /* defer MT decisions to system malloc */
    _sqliteZone_ = malloc_default_zone();
  }ਨਹੀਂਤਾਂ{
    /* only 1 core, use our own zone to contention over global locks,
    ** e.g. we have our own dedicated locks */
    _sqliteZone_ = malloc_create_zone(4096, 0);
    malloc_ਸੈੱਟ_zone_name(_sqliteZone_, "Sqlite_Heap");
  }
#ਜੇ_ਅੰਤ /*  defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC) */
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ਸਥਿਰ ਖਾਲੀ sqlite3MemShutdown(ਖਾਲੀ *Not_ਵਰਤਿਆ){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਵਾਪਸ;
}

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
ਖਾਲੀ sqlite3_Mem_ਸੈੱਟ_Default(ਖਾਲੀ){
  ਸਥਿਰ ਅਚਲ sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_ਮਾਪ,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#ਜੇ_ਅੰਤ /* SQLITE_SYSTEM_MALLOC */
