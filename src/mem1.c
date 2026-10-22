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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_SYSTEM_MALLOC
#ಆದರೆ defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)

/*
** Use the zone allocator available on apple products unless the
** SQLITE_WITHOUT_ZONEMALLOC symbol is defined.
*/
#ಅನ್ತರ್ಭಾವ <sys/sysctl.h>
#ಅನ್ತರ್ಭಾವ <malloc/malloc.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MIGHT_BE_SINGLE_CORE
#ಅನ್ತರ್ಭಾವ <libkern/OSAtomic.h>
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_MIGHT_BE_SINGLE_CORE */
ಸ್ಥಿರ malloc_zone_t* _sqliteZone_;
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MALLOC(x) malloc_zone_malloc(_sqliteZone_, (x))
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FREE(x) malloc_zone_free(_sqliteZone_, (x));
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_REALLOC(x,y) malloc_zone_realloc(_sqliteZone_, (x), (y))
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MALLOCSIZE(x) \
        (_sqliteZone_ ? _sqliteZone_->ಗಾತ್ರ_2(_sqliteZone_,x) : malloc_ಗಾತ್ರ(x))

#ಇಲ್ಲದಿದ್ದರೆ /* if not __APPLE__ */

/*
** Use standard C library malloc and free on non-Apple systems.  
** Also used by Apple systems if SQLITE_WITHOUT_ZONEMALLOC is defined.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MALLOC(x)             ಸ್ಮರಣೆ_ಕೊಡು(x)
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FREE(x)               ಬಿಡುಗಡೆ(x)
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_REALLOC(x,y)          realloc((x),(y))

/*
** The malloc.h header file is needed for malloc_usable_size() function
** on some systems (e.g. Linux).
*/
#ಆದರೆ HAVE_MALLOC_H && HAVE_MALLOC_USABLE_ಗಾತ್ರ
#  define SQLITE_USE_MALLOC_H 1
#  define SQLITE_USE_MALLOC_USABLE_ಗಾತ್ರ 1
/*
** The MSVCRT has malloc_usable_size(), but it is called _msize().  The
** use of _msize() is automatic, but can be disabled by compiling with
** -DSQLITE_WITHOUT_MSIZE.  Using the _msize() function also requires
** the malloc.h header file.
*/
#ಅಥವಾಆದರೆ defined(_MSC_VER) && !defined(SQLITE_WITHOUT_MSIZE)
#  define SQLITE_USE_MALLOC_H
#  define SQLITE_USE_MSIZE
#ಆದರೆ_ಅಂತ್ಯ

/*
** Include the malloc.h header file, if necessary.  Also set define macro
** SQLITE_MALLOCSIZE to the appropriate function name, which is _msize()
** for MSVC and malloc_usable_size() for most other systems (e.g. Linux).
** The memory size function can always be overridden manually by defining
** the macro SQLITE_MALLOCSIZE to the desired function name.
*/
#ಆದರೆ defined(SQLITE_USE_MALLOC_H)
#  include <malloc.h>
#  ಆದರೆ defined(SQLITE_USE_MALLOC_USABLE_ಗಾತ್ರ)
#    ಆದರೆ !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE(x)   malloc_usable_ಗಾತ್ರ(x)
#    endif
#  elif defined(SQLITE_USE_MSIZE)
#    ಆದರೆ !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE      _msize
#    endif
#  endif
#ಆದರೆ_ಅಂತ್ಯ /* defined(SQLITE_USE_MALLOC_H) */

#ಆದರೆ_ಅಂತ್ಯ /* __APPLE__ or not __APPLE__ */

/*
** Like malloc(), but remember the size of the allocation
** so that we can find it later using sqlite3MemSize().
**
** For this low-level routine, we are guaranteed that nByte>0 because
** cases of nByte<=0 will be intercepted and dealt with by higher level
** routines.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *sqlite3MemMalloc(ಪೂರ್ಣಾಂಕ nByte){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MALLOCSIZE
  ಶೂನ್ಯ *p;
  testcase( ROUND8(nByte)==nByte );
  p = SQLITE_MALLOC( nByte );
  ಆದರೆ( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  ಹಿಂತಿರುಗಿ p;
#ಇಲ್ಲದಿದ್ದರೆ
  sqlite3_int64 *p;
  ಪ್ರತಿಜ್ಞಾ( nByte>0 );
  testcase( ROUND8(nByte)!=nByte );
  p = SQLITE_MALLOC( nByte+8 );
  ಆದರೆ( p ){
    p[0] = nByte;
    p++;
  }ಇಲ್ಲದಿದ್ದರೆ{
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)p;
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Like free() but works for allocations obtained from sqlite3MemMalloc()
** or sqlite3MemRealloc().
**
** For this low-level routine, we already know that pPrior!=0 since
** cases where pPrior==0 will have been intercepted and dealt with
** by higher-level routines.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3MemFree(ಶೂನ್ಯ *p_ಪೂರ್ವ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MALLOCSIZE
  SQLITE_FREE(p_ಪೂರ್ವ);
#ಇಲ್ಲದಿದ್ದರೆ
  sqlite3_int64 *p = (sqlite3_int64*)p_ಪೂರ್ವ;
  ಪ್ರತಿಜ್ಞಾ( p_ಪೂರ್ವ!=0 );
  p--;
  SQLITE_FREE(p);
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Report the allocated size of a prior return from xMalloc()
** or xRealloc().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3_Mem_ಗಾತ್ರ(ಶೂನ್ಯ *p_ಪೂರ್ವ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MALLOCSIZE
  ಪ್ರತಿಜ್ಞಾ( p_ಪೂರ್ವ!=0 );
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)SQLITE_MALLOCSIZE(p_ಪೂರ್ವ);
#ಇಲ್ಲದಿದ್ದರೆ
  sqlite3_int64 *p;
  ಪ್ರತಿಜ್ಞಾ( p_ಪೂರ್ವ!=0 );
  p = (sqlite3_int64*)p_ಪೂರ್ವ;
  p--;
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)p[0];
#ಆದರೆ_ಅಂತ್ಯ
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
ಸ್ಥಿರ ಶೂನ್ಯ *sqlite3MemRealloc(ಶೂನ್ಯ *p_ಪೂರ್ವ, ಪೂರ್ಣಾಂಕ nByte){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MALLOCSIZE
  ಶೂನ್ಯ *p = SQLITE_REALLOC(p_ಪೂರ್ವ, nByte);
  ಆದರೆ( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      SQLITE_MALLOCSIZE(p_ಪೂರ್ವ), nByte);
  }
  ಹಿಂತಿರುಗಿ p;
#ಇಲ್ಲದಿದ್ದರೆ
  sqlite3_int64 *p = (sqlite3_int64*)p_ಪೂರ್ವ;
  ಪ್ರತಿಜ್ಞಾ( p_ಪೂರ್ವ!=0 && nByte>0 );
  ಪ್ರತಿಜ್ಞಾ( nByte==ROUND8(nByte) ); /* EV: R-46199-30249 */
  p--;
  p = SQLITE_REALLOC(p, nByte+8 );
  ಆದರೆ( p ){
    p[0] = nByte;
    p++;
  }ಇಲ್ಲದಿದ್ದರೆ{
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      sqlite3_Mem_ಗಾತ್ರ(p_ಪೂರ್ವ), nByte);
  }
  ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)p;
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Round up a request size to the next valid allocation size.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3MemRoundup(ಪೂರ್ಣಾಂಕ n){
  ಹಿಂತಿರುಗಿ ROUND8(n);
}

/*
** Initialize this module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3MemInit(ಶೂನ್ಯ *Not_ಬಳಸಿದ){
#ಆದರೆ defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)
  ಪೂರ್ಣಾಂಕ cpu_ಎಣಿಕೆ;
  size_t len;
  ಆದರೆ( _sqliteZone_ ){
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  len = ಗಾತ್ರ(cpu_ಎಣಿಕೆ);
  /* One usually wants to use hw.activecpu for MT decisions, but not here */
  sysctlbyname("hw.ncpu", &cpu_ಎಣಿಕೆ, &len, ಖಾಲಿ, 0);
  ಆದರೆ( cpu_ಎಣಿಕೆ>1 ){
    /* defer MT decisions to system malloc */
    _sqliteZone_ = malloc_default_zone();
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* only 1 core, use our own zone to contention over global locks,
    ** e.g. we have our own dedicated locks */
    _sqliteZone_ = malloc_create_zone(4096, 0);
    malloc_ಗಣ_zone_name(_sqliteZone_, "Sqlite_Heap");
  }
#ಆದರೆ_ಅಂತ್ಯ /*  defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC) */
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3MemShutdown(ಶೂನ್ಯ *Not_ಬಳಸಿದ){
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  ಹಿಂತಿರುಗಿ;
}

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
ಶೂನ್ಯ sqlite3_Mem_ಗಣ_Default(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_ಗಾತ್ರ,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_SYSTEM_MALLOC */
