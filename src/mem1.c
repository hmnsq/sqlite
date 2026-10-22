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
#अन्तर्भाव "sqliteInt.h"

/*
** This version of the memory allocator is the default.  It is
** used when no other memory allocator is specified using compile-time
** macros.
*/
#यदि_परिभाषितम् SQLITE_SYSTEM_MALLOC
#यदि defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)

/*
** Use the zone allocator available on apple products unless the
** SQLITE_WITHOUT_ZONEMALLOC symbol is defined.
*/
#अन्तर्भाव <sys/sysctl.h>
#अन्तर्भाव <malloc/malloc.h>
#यदि_परिभाषितम् SQLITE_MIGHT_BE_SINGLE_CORE
#अन्तर्भाव <libkern/OSAtomic.h>
#यदि_अन्तः /* SQLITE_MIGHT_BE_SINGLE_CORE */
स्थिर malloc_zone_t* _sqliteZone_;
#परिभाषय SQLITE_MALLOC(x) malloc_zone_malloc(_sqliteZone_, (x))
#परिभाषय SQLITE_FREE(x) malloc_zone_free(_sqliteZone_, (x));
#परिभाषय SQLITE_REALLOC(x,y) malloc_zone_realloc(_sqliteZone_, (x), (y))
#परिभाषय SQLITE_MALLOCSIZE(x) \
        (_sqliteZone_ ? _sqliteZone_->आकार(_sqliteZone_,x) : malloc_आकार(x))

#अन्यथा /* if not __APPLE__ */

/*
** Use standard C library malloc and free on non-Apple systems.  
** Also used by Apple systems if SQLITE_WITHOUT_ZONEMALLOC is defined.
*/
#परिभाषय SQLITE_MALLOC(x)             स्मृतिं_देहि(x)
#परिभाषय SQLITE_FREE(x)               मुक्त(x)
#परिभाषय SQLITE_REALLOC(x,y)          realloc((x),(y))

/*
** The malloc.h header file is needed for malloc_usable_size() function
** on some systems (e.g. Linux).
*/
#यदि HAVE_MALLOC_H && HAVE_MALLOC_USABLE_आकार
#  define SQLITE_USE_MALLOC_H 1
#  define SQLITE_USE_MALLOC_USABLE_आकार 1
/*
** The MSVCRT has malloc_usable_size(), but it is called _msize().  The
** use of _msize() is automatic, but can be disabled by compiling with
** -DSQLITE_WITHOUT_MSIZE.  Using the _msize() function also requires
** the malloc.h header file.
*/
#नोचेद्यदि defined(_MSC_VER) && !defined(SQLITE_WITHOUT_MSIZE)
#  define SQLITE_USE_MALLOC_H
#  define SQLITE_USE_MSIZE
#यदि_अन्तः

/*
** Include the malloc.h header file, if necessary.  Also set define macro
** SQLITE_MALLOCSIZE to the appropriate function name, which is _msize()
** for MSVC and malloc_usable_size() for most other systems (e.g. Linux).
** The memory size function can always be overridden manually by defining
** the macro SQLITE_MALLOCSIZE to the desired function name.
*/
#यदि defined(SQLITE_USE_MALLOC_H)
#  include <malloc.h>
#  यदि defined(SQLITE_USE_MALLOC_USABLE_आकार)
#    यदि !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE(x)   malloc_usable_आकार(x)
#    endif
#  elif defined(SQLITE_USE_MSIZE)
#    यदि !defined(SQLITE_MALLOCSIZE)
#      define SQLITE_MALLOCSIZE      _msize
#    endif
#  endif
#यदि_अन्तः /* defined(SQLITE_USE_MALLOC_H) */

#यदि_अन्तः /* __APPLE__ or not __APPLE__ */

/*
** Like malloc(), but remember the size of the allocation
** so that we can find it later using sqlite3MemSize().
**
** For this low-level routine, we are guaranteed that nByte>0 because
** cases of nByte<=0 will be intercepted and dealt with by higher level
** routines.
*/
स्थिर शून्य *sqlite3MemMalloc(पूर्णाङ्क nByte){
#यदि_परिभाषितम् SQLITE_MALLOCSIZE
  शून्य *p;
  testcase( ROUND8(nByte)==nByte );
  p = SQLITE_MALLOC( nByte );
  यदि( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  प्रत्यागम p;
#अन्यथा
  sqlite3_int64 *p;
  प्रतिज्ञा( nByte>0 );
  testcase( ROUND8(nByte)!=nByte );
  p = SQLITE_MALLOC( nByte+8 );
  यदि( p ){
    p[0] = nByte;
    p++;
  }अन्यथा{
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM, "failed to allocate %u bytes of memory", nByte);
  }
  प्रत्यागम (शून्य *)p;
#यदि_अन्तः
}

/*
** Like free() but works for allocations obtained from sqlite3MemMalloc()
** or sqlite3MemRealloc().
**
** For this low-level routine, we already know that pPrior!=0 since
** cases where pPrior==0 will have been intercepted and dealt with
** by higher-level routines.
*/
स्थिर शून्य sqlite3MemFree(शून्य *p_प्राक्){
#यदि_परिभाषितम् SQLITE_MALLOCSIZE
  SQLITE_FREE(p_प्राक्);
#अन्यथा
  sqlite3_int64 *p = (sqlite3_int64*)p_प्राक्;
  प्रतिज्ञा( p_प्राक्!=0 );
  p--;
  SQLITE_FREE(p);
#यदि_अन्तः
}

/*
** Report the allocated size of a prior return from xMalloc()
** or xRealloc().
*/
स्थिर पूर्णाङ्क sqlite3_Mem_आकार(शून्य *p_प्राक्){
#यदि_परिभाषितम् SQLITE_MALLOCSIZE
  प्रतिज्ञा( p_प्राक्!=0 );
  प्रत्यागम (पूर्णाङ्क)SQLITE_MALLOCSIZE(p_प्राक्);
#अन्यथा
  sqlite3_int64 *p;
  प्रतिज्ञा( p_प्राक्!=0 );
  p = (sqlite3_int64*)p_प्राक्;
  p--;
  प्रत्यागम (पूर्णाङ्क)p[0];
#यदि_अन्तः
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
स्थिर शून्य *sqlite3MemRealloc(शून्य *p_प्राक्, पूर्णाङ्क nByte){
#यदि_परिभाषितम् SQLITE_MALLOCSIZE
  शून्य *p = SQLITE_REALLOC(p_प्राक्, nByte);
  यदि( p==0 ){
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      SQLITE_MALLOCSIZE(p_प्राक्), nByte);
  }
  प्रत्यागम p;
#अन्यथा
  sqlite3_int64 *p = (sqlite3_int64*)p_प्राक्;
  प्रतिज्ञा( p_प्राक्!=0 && nByte>0 );
  प्रतिज्ञा( nByte==ROUND8(nByte) ); /* EV: R-46199-30249 */
  p--;
  p = SQLITE_REALLOC(p, nByte+8 );
  यदि( p ){
    p[0] = nByte;
    p++;
  }अन्यथा{
    testcase( sqlite3GlobalConfig.xLog!=0 );
    sqlite3_log(SQLITE_NOMEM,
      "failed memory resize %u to %u bytes",
      sqlite3_Mem_आकार(p_प्राक्), nByte);
  }
  प्रत्यागम (शून्य*)p;
#यदि_अन्तः
}

/*
** Round up a request size to the next valid allocation size.
*/
स्थिर पूर्णाङ्क sqlite3MemRoundup(पूर्णाङ्क n){
  प्रत्यागम ROUND8(n);
}

/*
** Initialize this module.
*/
स्थिर पूर्णाङ्क sqlite3MemInit(शून्य *Not_प्रयुक्तम्){
#यदि defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC)
  पूर्णाङ्क cpu_गणनम्;
  size_t len;
  यदि( _sqliteZone_ ){
    प्रत्यागम SQLITE_OK;
  }
  len = परिमाण(cpu_गणनम्);
  /* One usually wants to use hw.activecpu for MT decisions, but not here */
  sysctlbyname("hw.ncpu", &cpu_गणनम्, &len, रिक्त, 0);
  यदि( cpu_गणनम्>1 ){
    /* defer MT decisions to system malloc */
    _sqliteZone_ = malloc_default_zone();
  }अन्यथा{
    /* only 1 core, use our own zone to contention over global locks,
    ** e.g. we have our own dedicated locks */
    _sqliteZone_ = malloc_create_zone(4096, 0);
    malloc_समुच्चयः_zone_name(_sqliteZone_, "Sqlite_Heap");
  }
#यदि_अन्तः /*  defined(__APPLE__) && !defined(SQLITE_WITHOUT_ZONEMALLOC) */
  अप्रयुक्तम्_PARAMETER(Not_प्रयुक्तम्);
  प्रत्यागम SQLITE_OK;
}

/*
** Deinitialize this module.
*/
स्थिर शून्य sqlite3MemShutdown(शून्य *Not_प्रयुक्तम्){
  अप्रयुक्तम्_PARAMETER(Not_प्रयुक्तम्);
  प्रत्यागम;
}

/*
** This routine is the only routine in this file with external linkage.
**
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file.
*/
शून्य sqlite3_Mem_समुच्चयः_Default(शून्य){
  स्थिर ध्रुव sqlite3_mem_methods defaultMethods = {
     sqlite3MemMalloc,
     sqlite3MemFree,
     sqlite3MemRealloc,
     sqlite3_Mem_आकार,
     sqlite3MemRoundup,
     sqlite3MemInit,
     sqlite3MemShutdown,
     0
  };
  sqlite3_config(SQLITE_CONFIG_MALLOC, &defaultMethods);
}

#यदि_अन्तः /* SQLITE_SYSTEM_MALLOC */
