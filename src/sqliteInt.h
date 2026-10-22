/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Internal interface definitions for SQLite.
**
*/
#నిర్వచితం_కాకపోతే SQLITEINT_H
#నిర్వచించు SQLITEINT_H

/* Special Comments:
**
** Some comments have special meaning to the tools that measure test
** coverage:
**
**    NO_TEST                     - The branches on this line are not
**                                  measured by branch coverage.  This is
**                                  used on lines of code that actually
**                                  implement parts of coverage testing.
**
**    OPTIMIZATION-IF-TRUE        - This branch is allowed to always be false
**                                  and the correct answer is still obtained,
**                                  though perhaps more slowly.
**
**    OPTIMIZATION-IF-FALSE       - This branch is allowed to always be true
**                                  and the correct answer is still obtained,
**                                  though perhaps more slowly.
**
**    PREVENTS-HARMLESS-OVERREAD  - This branch prevents a buffer overread
**                                  that would be harmless and undetectable
**                                  if it did occur.
**
** In all cases, the special comment must be enclosed in the usual
** slash-asterisk...asterisk-slash comment marks, with no spaces between the
** asterisks and the comment text.
*/

/*
** Make sure the Tcl calling convention macro is defined.  This macro is
** only used by test code and Tcl integration code.
*/
#నిర్వచితం_కాకపోతే SQLITE_TCLAPI
#  define SQLITE_TCLAPI
#అయితే_ముగింపు

/*
** Include the header file used to customize the compiler options for MSVC.
** This should be done first so that it can successfully prevent spurious
** compiler warnings due to subsequent content in this file and other files
** that are included by this file.
*/
#అన్తర్భావ "msvc.h"

/*
** Special setup for VxWorks
*/
#అన్తర్భావ "vxworks.h"

/*
** These #defines should enable >2GB file support on POSIX if the
** underlying operating system supports it.  If the OS lacks
** large file support, or if the OS is windows, these should be no-ops.
**
** Ticket #2739:  The _LARGEFILE_SOURCE macro must appear before any
** system #includes.  Hence, this block of code must be the very first
** code in all source files.
**
** Large file support can be disabled using the -DSQLITE_DISABLE_LFS switch
** on the compiler command line.  This is necessary if you are compiling
** on a recent machine (ex: Red Hat 7.2) but you want your code to work
** on an older machine (ex: Red Hat 6.0).  If you compile on Red Hat 7.2
** without this option, LFS is enable.  But LFS does not exist in the kernel
** in Red Hat 6.0, so the code won't work.  Hence, for maximum binary
** portability you should omit LFS.
**
** The previous paragraph was written in 2005.  (This paragraph is written
** on 2008-11-28.) These days, all Linux kernels support large files, so
** you should probably leave LFS enabled.  But some embedded platforms might
** lack LFS in which case the SQLITE_DISABLE_LFS macro might still be useful.
**
** Similar is true for Mac OS X.  LFS is only supported on Mac OS X 9 and later.
*/
#నిర్వచితం_కాకపోతే SQLITE_DISABLE_LFS
# define _LARGE_FILE       1
# ifndef FILE_స్థానభ్రంశం_బిట్లు
#   define FILE_స్థానభ్రంశం_బిట్లు 64
# endif
# define LARGEFILE_ఆధారం 1
#అయితే_ముగింపు

/* The GCC_VERSION and MSVC_VERSION macros are used to
** conditionally include optimizations for each of these compilers.  A
** value of 0 means that compiler is not being used.  The
** SQLITE_DISABLE_INTRINSIC macro means do not use any compiler-specific
** optimizations, and hence set all compiler macros to 0
**
** There was once also a CLANG_VERSION macro.  However, we learn that the
** version numbers in clang are for "marketing" only and are inconsistent
** and unreliable.  Fortunately, all versions of clang also recognize the
** gcc version numbers and have reasonable settings for gcc version numbers,
** so the GCC_VERSION macro will be set to a correct non-zero value even
** when compiling with clang.
*/
#ఒకవేళ defined(__GNUC__) && !defined(SQLITE_DISABLE_INTRINSIC)
# define GCC_సంస్కరణ (__GNUC__*1000000+__GNUC_MINOR__*1000+__GNUC_PATCHLEVEL__)
#లేకపోతే
# define GCC_సంస్కరణ 0
#అయితే_ముగింపు
#ఒకవేళ defined(_MSC_VER) && !defined(SQLITE_DISABLE_INTRINSIC)
# define MSVC_సంస్కరణ _MSC_VER
#లేకపోతే
# define MSVC_సంస్కరణ 0
#అయితే_ముగింపు

/*
** Some C99 functions in "math.h" are only present for MSVC when its version
** is associated with Visual Studio 2013 or higher.
*/
#నిర్వచితం_కాకపోతే SQLITE_HAVE_C99_MATH_FUNCS
# ఒకవేళ MSVC_సంస్కరణ==0 || MSVC_సంస్కరణ>=1800
#  define SQLITE_HAVE_C99_MATH_FUNCS (1)
# లేకపోతే
#  define SQLITE_HAVE_C99_MATH_FUNCS (0)
# endif
#అయితే_ముగింపు

/* Needed for various definitions... */
#ఒకవేళ defined(__GNUC__) && !defined(GNU_ఆధారం)
# define GNU_ఆధారం
#అయితే_ముగింపు

#ఒకవేళ defined(__OpenBSD__) && !defined(BSD_ఆధారం)
# define BSD_ఆధారం
#అయితే_ముగింపు

/*
** Macro to disable warnings about missing "break" at the end of a "case".
*/
#ఒకవేళ defined(__has_attribute)
#  ఒకవేళ __has_attribute(fallthrough)
#    define deliberate_fall_through __attribute__((fallthrough));
#  endif
#అయితే_ముగింపు
#ఒకవేళ !defined(deliberate_fall_through)
#  define deliberate_fall_through
#అయితే_ముగింపు

/*
** For MinGW, check to see if we can include the header file containing its
** version information, among other things.  Normally, this internal MinGW
** header file would [only] be included automatically by other MinGW header
** files; however, the contained version information is now required by this
** header file to work around binary compatibility issues (see below) and
** this is the only known way to reliably obtain it.  This entire #if block
** would be completely unnecessary if there was any other way of detecting
** MinGW via their preprocessor (e.g. if they customized their GCC to define
** some MinGW-specific macros).  When compiling for MinGW, either the
** _HAVE_MINGW_H or _HAVE__MINGW_H (note the extra underscore) macro must be
** defined; otherwise, detection of conditions specific to MinGW will be
** disabled.
*/
#ఒకవేళ defined(_HAVE_MINGW_H)
# include "mingw.h"
#లేదావేళ defined(_HAVE__MINGW_H)
# include "_mingw.h"
#అయితే_ముగింపు

/*
** For MinGW version 4.x (and higher), check to see if the _USE_32BIT_TIME_T
** define is required to maintain binary compatibility with the MSVC runtime
** library in use (e.g. for Windows XP).
*/
#ఒకవేళ !defined(USE_32_బిట్టు_సమయం_T) && !defined(USE_64_బిట్టు_సమయం_T) && \
    defined(_WIN32) && !defined(_WIN64) && \
    defined(__MINGW_MAJOR_VERSION) && __MINGW_MAJOR_VERSION >= 4 && \
    defined(__MSVCRT__)
# define USE_32_బిట్టు_సమయం_T
#అయితే_ముగింపు

/* Optionally #include a user-defined header, whereby compilation options
** may be set prior to where they take effect, but after platform setup.
** If SQLITE_CUSTOM_INCLUDE=? is defined, its value names the #include
** file.
*/
#నిర్వచితమైతే SQLITE_CUSTOM_INCLUDE
# define INC_STRINGIFY_(f) #f
# define INC_STRINGIFY(f) INC_STRINGIFY_(f)
# include INC_STRINGIFY(SQLITE_CUSTOM_INCLUDE)
#అయితే_ముగింపు

/* The public SQLite interface.  The _FILE_OFFSET_BITS macro must appear
** first in QNX.  Also, the _USE_32BIT_TIME_T macro must appear first for
** MinGW.
*/
#అన్తర్భావ "sqlite3.h"

/*
** Reuse the STATIC_VFS1 for mutex access to sqlite3_temp_directory.
*/
#నిర్వచించు SQLITE_MUTEX_STATIC_TEMPDIR SQLITE_MUTEX_STATIC_VFS1

/*
** Include the configuration header output by 'configure' if we're using the
** autoconf-based build
*/
#ఒకవేళ defined(_HAVE_SQLITE_CONFIG_H) && !defined(SQLITECONFIG_H)
#అన్తర్భావ "sqlite_cfg.h"
#నిర్వచించు SQLITECONFIG_H 1
#అయితే_ముగింపు

#అన్తర్భావ "sqliteLimit.h"

/* Disable nuisance warnings on Borland compilers */
#ఒకవేళ defined(__BORLANDC__)
#సూచన warn -rch /* unreachable code */
#సూచన warn -ccc /* Condition is always true or false */
#సూచన warn -aus /* Assigned value is never used */
#సూచన warn -csu /* Comparing signed and unsigned */
#సూచన warn -spa /* Suspicious pointer arithmetic */
#అయితే_ముగింపు

/*
** A few places in the code require atomic load/store of aligned
** integer values.
*/
#నిర్వచితం_కాకపోతే __has_extension
# define __has_extension(x) 0     /* compatibility with non-clang compilers */
#అయితే_ముగింపు
#ఒకవేళ GCC_సంస్కరణ>=4007000 || __has_extension(c_atomic)
# define SQLITE_ATOMIC_INTRINSICS 1
# define Atomic_ఎక్కించు(PTR)       __atomic_load_n((PTR),__ATOMIC_RELAXED)
# define AtomicStore(PTR,VAL)  __atomic_store_n((PTR),(VAL),__ATOMIC_RELAXED)
#లేకపోతే
# define SQLITE_ATOMIC_INTRINSICS 0
# define Atomic_ఎక్కించు(PTR)       (*(PTR))
# define AtomicStore(PTR,VAL)  (*(PTR) = (VAL))
#అయితే_ముగింపు

/*
** Include standard header files as necessary
*/
#అన్తర్భావ <stdint.h>
#నిర్వచితమైతే HAVE_INTTYPES_H
#అన్తర్భావ <inttypes.h>
#అయితే_ముగింపు

/*
** The following macros are used to cast pointers to integers and
** integers to pointers.  The way you do this varies from one compiler
** to the next, so we have developed the following set of #if statements
** to generate appropriate macros for a wide range of compilers.
**
** The correct "ANSI" way to do this is to use the intptr_t type.
** Unfortunately, that typedef is not available on all compilers, or
** if it is available, it requires an #include of specific headers
** that vary from one machine to the next.
**
** Ticket #3860:  The llvm-gcc-4.2 compiler from Apple chokes on
** the ((void*)&((char*)0)[X]) construct.  But MSVC chokes on ((void*)(X)).
** So we have to define the macros in different ways depending on the
** compiler.
*/
#ఒకవేళ defined(HAVE_STDINT_H)   /* Use this case if we have ANSI headers */
# define SQLITE_INT_గమ్యం_PTR(X)  ((శూన్య*)(intptr_t)(X))
# define SQLITE_PTR_గమ్యం_INT(X)  ((పూర్ణాంకం)(intptr_t)(X))
#లేదావేళ defined(__PTRDIFF_TYPE__)  /* This case should work for GCC */
# define SQLITE_INT_గమ్యం_PTR(X)  ((శూన్య*)(__PTRDIFF_TYPE__)(X))
# define SQLITE_PTR_గమ్యం_INT(X)  ((పూర్ణాంకం)(__PTRDIFF_TYPE__)(X))
#లేదావేళ !defined(__GNUC__)       /* Works for compilers other than LLVM */
# define SQLITE_INT_గమ్యం_PTR(X)  ((శూన్య*)&((అక్షరం*)0)[X])
# define SQLITE_PTR_గమ్యం_INT(X)  ((పూర్ణాంకం)(((అక్షరం*)X)-(అక్షరం*)0))
#లేకపోతే                          /* Generates a warning - but it always works */
# define SQLITE_INT_గమ్యం_PTR(X)  ((శూన్య*)(X))
# define SQLITE_PTR_గమ్యం_INT(X)  ((పూర్ణాంకం)(X))
#అయితే_ముగింపు

/*
** Hint to the compiler that a function should or should not be inlined:
**
**    SQLITE_NOINLINE         Never in-line this function
**
**    SQLITE_INLINE           Strive to in-line this function
**
**    SQLITE_OPT_INLINE       In-line this function if building the
**                            amalgamation.
**
**    SQLITE_USES_INLINE      Set to 1 or 0 according to whether or not
**                            inline functions are supported.
*/
#ఒకవేళ defined(SQLITE_DISABLE_INLINE)
#  define SQLITE_NOINLINE
#  define SQLITE_INLINE
#  define SQLITE_OPT_INLINE
#  define SQLITE_ఉపయోగిస్తుంది_INLINE 0
#లేదావేళ defined(__GNUC__)
#  define SQLITE_NOINLINE    __attribute__((noinline))
#  define SQLITE_INLINE      __attribute__((always_inline)) అంతఃవరుస
#  define SQLITE_OPT_INLINE  __attribute__((always_inline)) అంతఃవరుస
#  define SQLITE_ఉపయోగిస్తుంది_INLINE 1
#లేదావేళ defined(_MSC_VER) && _MSC_VER>=1310
#  define SQLITE_NOINLINE    __declspec(noinline)
#  define SQLITE_INLINE      __forceinline
#  define SQLITE_OPT_INLINE  __forceinline
#  define SQLITE_ఉపయోగిస్తుంది_INLINE 1
#లేకపోతే
#  define SQLITE_NOINLINE
#  define SQLITE_INLINE
#  define SQLITE_OPT_INLINE
#  define SQLITE_ఉపయోగిస్తుంది_INLINE 0
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_COVERAGE_TEST) || defined(__STRICT_ANSI__)
# undef SQLITE_INLINE
# define SQLITE_INLINE
# undef SQLITE_OPT_INLINE
# define SQLITE_OPT_INLINE
# undef SQLITE_ఉపయోగిస్తుంది_INLINE
# define SQLITE_ఉపయోగిస్తుంది_INLINE 0
#అయితే_ముగింపు
#ఒకవేళ !defined(SQLITE_AMALGAMATION)
# undef SQLITE_OPT_INLINE
# define SQLITE_OPT_INLINE
#అయితే_ముగింపు

/*
** Make sure that the compiler intrinsics we desire are enabled when
** compiling with an appropriate version of MSVC unless prevented by
** the SQLITE_DISABLE_INTRINSIC define.
*/
#ఒకవేళ !defined(SQLITE_DISABLE_INTRINSIC)
#  ఒకవేళ defined(_MSC_VER) && _MSC_VER>=1400
#    ఒకవేళ !defined(_WIN32_WCE)
#      include <intrin.h>
#      pragma intrinsic(_byteswap_ushort)
#      pragma intrinsic(_byteswap_ulong)
#      pragma intrinsic(_byteswap_uint64)
#      pragma intrinsic(_ReadWriteBarrier)
#    లేకపోతే
#      include <cmnintrin.h>
#    endif
#  endif
#అయితే_ముగింపు

/*
** Enable SQLITE_USE_SEH by default on MSVC builds.  Only omit
** SEH support if the -DSQLITE_OMIT_SEH option is given.
*/
#ఒకవేళ defined(_MSC_VER) && !defined(SQLITE_OMIT_SEH)
# define SQLITE_USE_SEH 1
#లేకపోతే
# undef SQLITE_USE_SEH
#అయితే_ముగింపు

/*
** Enable SQLITE_DIRECT_OVERFLOW_READ, unless the build explicitly
** disables it using -DSQLITE_DIRECT_OVERFLOW_READ=0
*/
#ఒకవేళ defined(SQLITE_ప్రత్యక్ష_అధికప్రవాహం_READ) && SQLITE_ప్రత్యక్ష_అధికప్రవాహం_READ+1==1
  /* Disable if -DSQLITE_DIRECT_OVERFLOW_READ=0 */
# undef SQLITE_ప్రత్యక్ష_అధికప్రవాహం_READ
#లేకపోతే
  /* In all other cases, enable */
# define SQLITE_ప్రత్యక్ష_అధికప్రవాహం_READ 1
#అయితే_ముగింపు


/*
** The SQLITE_THREADSAFE macro must be defined as 0, 1, or 2.
** 0 means mutexes are permanently disable and the library is never
** threadsafe.  1 means the library is serialized which is the highest
** level of threadsafety.  2 means the library is multithreaded - multiple
** threads can use SQLite as long as no two threads try to use the same
** database connection at the same time.
**
** Older versions of SQLite used an optional THREADSAFE macro.
** We support that for legacy.
**
** To ensure that the correct value of "THREADSAFE" is reported when querying
** for compile-time options at runtime (e.g. "PRAGMA compile_options"), this
** logic is partially replicated in ctime.c. If it is updated here, it should
** also be updated there.
*/
#ఒకవేళ !defined(SQLITE_THREADSAFE)
# ఒకవేళ defined(THREADSAFE)
#   define SQLITE_THREADSAFE THREADSAFE
# లేకపోతే
#   define SQLITE_THREADSAFE 1 /* IMP: R-07272-22309 */
# endif
#అయితే_ముగింపు

/*
** Powersafe overwrite is on by default.  But can be turned off using
** the -DSQLITE_POWERSAFE_OVERWRITE=0 command-line option.
*/
#నిర్వచితం_కాకపోతే SQLITE_POWERSAFE_OVERWRITE
# define SQLITE_POWERSAFE_OVERWRITE 1
#అయితే_ముగింపు

/*
** EVIDENCE-OF: R-25715-37072 Memory allocation statistics are enabled by
** default unless SQLite is compiled with SQLITE_DEFAULT_MEMSTATUS=0 in
** which case memory allocation statistics are disabled by default.
*/
#ఒకవేళ !defined(SQLITE_DEFAULT_MEMSTATUS)
# define SQLITE_DEFAULT_MEMSTATUS 1
#అయితే_ముగింపు

/*
** Exactly one of the following macros must be defined in order to
** specify which memory allocation subsystem to use.
**
**     SQLITE_SYSTEM_MALLOC          // Use normal system malloc()
**     SQLITE_WIN32_MALLOC           // Use Win32 native heap API
**     SQLITE_ZERO_MALLOC            // Use a stub allocator that always fails
**     SQLITE_MEMDEBUG               // Debugging version of system malloc()
**
** On Windows, if the SQLITE_WIN32_MALLOC_VALIDATE macro is defined and the
** assert() macro is enabled, each call into the Win32 native heap subsystem
** will cause HeapValidate to be called.  If heap validation should fail, an
** assertion will be triggered.
**
** If none of the above are defined, then set SQLITE_SYSTEM_MALLOC as
** the default.
*/
#ఒకవేళ defined(SQLITE_SYSTEM_MALLOC) \
  + defined(SQLITE_WIN32_MALLOC) \
  + defined(SQLITE_ZERO_MALLOC) \
  + defined(SQLITE_MEMDEBUG)>1
# error "Two or మరిన్ని యొక్క the following compile-సమయం configuration options\
 are defined but at most one is allowed:\
 SQLITE_SYSTEM_MALLOC, SQLITE_WIN32_MALLOC, SQLITE_MEMDEBUG,\
 SQLITE_ZERO_MALLOC"
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_SYSTEM_MALLOC) \
  + defined(SQLITE_WIN32_MALLOC) \
  + defined(SQLITE_ZERO_MALLOC) \
  + defined(SQLITE_MEMDEBUG)==0
# define SQLITE_SYSTEM_MALLOC 1
#అయితే_ముగింపు

/*
** If SQLITE_MALLOC_SOFT_LIMIT is not zero, then try to keep the
** sizes of memory allocations below this value where possible.
*/
#ఒకవేళ !defined(SQLITE_MALLOC_SOFT_హద్దు)
# define SQLITE_MALLOC_SOFT_హద్దు 1024
#అయితే_ముగింపు

/*
** We need to define _XOPEN_SOURCE as follows in order to enable
** recursive mutexes on most Unix systems and fchmod() on OpenBSD.
** But _XOPEN_SOURCE define causes problems for Mac OS X, so omit
** it.
*/
#ఒకవేళ !defined(XOPEN_ఆధారం) && !defined(__DARWIN__) && !defined(__APPLE__)
#  define XOPEN_ఆధారం 600
#అయితే_ముగింపు

/*
** NDEBUG and SQLITE_DEBUG are opposites.  It should always be true that
** defined(NDEBUG)==!defined(SQLITE_DEBUG).  If this is not currently true,
** make it true by defining or undefining NDEBUG.
**
** Setting NDEBUG makes the code smaller and faster by disabling the
** assert() statements in the code.  So we want the default action
** to be for NDEBUG to be set and NDEBUG to be undefined only if SQLITE_DEBUG
** is set.  Thus NDEBUG becomes an opt-in rather than an opt-out
** feature.
*/
#ఒకవేళ !defined(NDEBUG) && !defined(SQLITE_DEBUG)
# define NDEBUG 1
#అయితే_ముగింపు
#ఒకవేళ defined(NDEBUG) && defined(SQLITE_DEBUG)
# undef NDEBUG
#అయితే_ముగింపు

/*
** Enable SQLITE_ENABLE_EXPLAIN_COMMENTS if SQLITE_DEBUG is turned on.
*/
#ఒకవేళ !defined(SQLITE_ENABLE_EXPLAIN_COMMENTS) && defined(SQLITE_DEBUG)
# define SQLITE_ENABLE_EXPLAIN_COMMENTS 1
#అయితే_ముగింపు

/*
** The testcase() macro is used to aid in coverage testing.  When
** doing coverage testing, the condition inside the argument to
** testcase() must be evaluated both true and false in order to
** get full branch coverage.  The testcase() macro is inserted
** to help ensure adequate test coverage in places where simple
** condition/decision coverage is inadequate.  For example, testcase()
** can be used to make sure boundary values are tested.  For
** bitmask tests, testcase() can be used to make sure each bit
** is significant and used at least once.  On switch statements
** where multiple cases go to the same block of code, testcase()
** can insure that all cases are evaluated.
*/
#ఒకవేళ defined(SQLITE_MUTATION_TEST)
# define testcase(X)
#లేదావేళ defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
# ifndef SQLITE_AMALGAMATION
    బాహ్య గుర్తులేని పూర్ణాంకం sqlite3CoverageCounter;
# endif
# define testcase(X)  ఒకవేళ( X ){ sqlite3CoverageCounter += (గుర్తులేని)__LINE__; }
#లేకపోతే
# define testcase(X)
#అయితే_ముగింపు

/*
** The TESTONLY macro is used to enclose variable declarations or
** other bits of code that are needed to support the arguments
** within testcase() and assert() macros.
*/
#ఒకవేళ !defined(NDEBUG) || defined(SQLITE_COVERAGE_TEST)
# define TESTONLY(X)  X
#లేకపోతే
# define TESTONLY(X)
#అయితే_ముగింపు

/*
** Sometimes we need a small amount of code such as a variable initialization
** to setup for a later assert() statement.  We do not want this code to
** appear when assert() is disabled.  The following macro is therefore
** used to contain that setup code.  The "VVA" acronym stands for
** "Verification, Validation, and Accreditation".  In other words, the
** code within VVA_ONLY() will only run during verification processes.
*/
#నిర్వచితం_కాకపోతే NDEBUG
# define VVA_ONLY(X)  X
#లేకపోతే
# define VVA_ONLY(X)
#అయితే_ముగింపు

/*
** Disable ALWAYS() and NEVER() (make them pass-throughs) for coverage
** and mutation testing
*/
#ఒకవేళ defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_MUTATION_TEST)
# define SQLITE_OMIT_AUXILIARY_SAFETY_CHECKS  1
#అయితే_ముగింపు

/*
** The ALWAYS and NEVER macros surround boolean expressions which
** are intended to always be true or false, respectively.  Such
** expressions could be omitted from the code completely.  But they
** are included in a few cases in order to enhance the resilience
** of SQLite to unexpected behavior - to make the code "self-healing"
** or "ductile" rather than being "brittle" and crashing at the first
** hint of unplanned behavior.
**
** In other words, ALWAYS and NEVER are added for defensive code.
**
** When doing coverage testing ALWAYS and NEVER are hard-coded to
** be true and false so that the unreachable code they specify will
** not be counted as untested code.
*/
#ఒకవేళ defined(SQLITE_OMIT_AUXILIARY_SAFETY_CHECKS)
# define ALWAYS(X)      (1)
# define NEVER(X)       (0)
#లేదావేళ !defined(NDEBUG)
# define ALWAYS(X)      ((X)?1:(ప్రతిజ్ఞా(0),0))
# define NEVER(X)       ((X)?(ప్రతిజ్ఞా(0),1):0)
#లేకపోతే
# define ALWAYS(X)      (X)
# define NEVER(X)       (X)
#అయితే_ముగింపు

/*
** Some conditionals are optimizations only.  In other words, if the
** conditionals are replaced with a constant 1 (true) or 0 (false) then
** the correct answer is still obtained, though perhaps not as quickly.
**
** The following macros mark these optimizations conditionals.
*/
#ఒకవేళ defined(SQLITE_MUTATION_TEST)
# define OK_IF_ALWAYS_TRUE(X)  (1)
# define OK_IF_ALWAYS_FALSE(X) (0)
#లేకపోతే
# define OK_IF_ALWAYS_TRUE(X)  (X)
# define OK_IF_ALWAYS_FALSE(X) (X)
#అయితే_ముగింపు

/*
** Some malloc failures are only possible if SQLITE_TEST_REALLOC_STRESS is
** defined.  We need to defend against those failures when testing with
** SQLITE_TEST_REALLOC_STRESS, but we don't want the unreachable branches
** during a normal build.  The following macro can be used to disable tests
** that are always false except when SQLITE_TEST_REALLOC_STRESS is set.
*/
#ఒకవేళ defined(SQLITE_TEST_REALLOC_STRESS)
# define ONLY_IF_REALLOC_STRESS(X)  (X)
#లేదావేళ !defined(NDEBUG)
# define ONLY_IF_REALLOC_STRESS(X)  ((X)?(ప్రతిజ్ఞా(0),1):0)
#లేకపోతే
# define ONLY_IF_REALLOC_STRESS(X)  (0)
#అయితే_ముగింపు

/*
** Declarations used for tracing the operating system interfaces.
*/
#ఒకవేళ defined(SQLITE_FORCE_OS_TRACE) || defined(SQLITE_TEST) || \
    (defined(SQLITE_DEBUG) && SQLITE_OS_WIN)
  బాహ్య పూర్ణాంకం sqlite3OSTrace;
# define OSTRACE(X)          ఒకవేళ( sqlite3OSTrace ) sqlite3DebugPrintf X
# define SQLITE_HAVE_OS_TRACE
#లేకపోతే
# define OSTRACE(X)
# undef  SQLITE_HAVE_OS_TRACE
#అయితే_ముగింపు

/*
** Is the sqlite3ErrName() function needed in the build?  Currently,
** it is needed by "mutex_w32.c" (when debugging), "os_win.c" (when
** OSTRACE is enabled), and by several "test*.c" files (which are
** compiled using SQLITE_TEST).
*/
#ఒకవేళ defined(SQLITE_HAVE_OS_TRACE) || defined(SQLITE_TEST) || \
    (defined(SQLITE_DEBUG) && SQLITE_OS_WIN)
# define SQLITE_NEED_ERR_NAME
#లేకపోతే
# undef  SQLITE_NEED_ERR_NAME
#అయితే_ముగింపు

/*
** SQLITE_ENABLE_EXPLAIN_COMMENTS is incompatible with SQLITE_OMIT_EXPLAIN
*/
#నిర్వచితమైతే SQLITE_OMIT_EXPLAIN
# undef SQLITE_ENABLE_EXPLAIN_COMMENTS
#అయితే_ముగింపు

/*
** SQLITE_OMIT_VIRTUALTABLE implies SQLITE_OMIT_ALTERTABLE
*/
#ఒకవేళ defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_ALTERTABLE)
# define SQLITE_OMIT_ALTERTABLE
#అయితే_ముగింపు

#నిర్వచించు SQLITE_DIGIT_SEPARATOR '_'

/*
** Return true (non-zero) if the input is an integer that is too large
** to fit in 32-bits.  This macro is used inside of various testcase()
** macros to verify that we have tested SQLite for large-file support.
*/
#నిర్వచించు IS_BIG_INT(X)  (((X)&~(i64)0xffffffff)!=0)

/*
** The macro unlikely() is a hint that surrounds a boolean
** expression that is usually false.  Macro likely() surrounds
** a boolean expression that is usually true.  These hints could,
** in theory, be used by the compiler to generate better code, but
** currently they are just comments for human readers.
*/
#నిర్వచించు likely(X)    (X)
#నిర్వచించు unlikely(X)  (X)

#అన్తర్భావ "hash.h"
#అన్తర్భావ "parse.h"
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <ctype.h>

/*
** Use a macro to replace memcpy() if compiled with SQLITE_INLINE_MEMCPY.
** This allows better measurements of where memcpy() is used when running
** cachegrind.  But this macro version of memcpy() is very slow so it
** should not be used in production.  This is a performance measurement
** hack only.
*/
#నిర్వచితమైతే SQLITE_INLINE_MEMCPY
# define జ్ఞాపకం_నకలు(D,S,N) {అక్షరం*xxd=(అక్షరం*)(D);స్థిరరాశి అక్షరం*xxs=(స్థిరరాశి అక్షరం*)(S);\
                        పూర్ణాంకం xxn=(N);అయితే(xxn-->0)*(xxd++)=*(xxs++);}
#అయితే_ముగింపు

/*
** If compiling for a processor that lacks floating point support,
** substitute integer for floating-point
*/
#నిర్వచితమైతే SQLITE_OMIT_FLOATING_బిందువు
# define ద్విగుణ sqlite_int64
# define దశాంశం sqlite_int64
# define fabs(X) ((X)<0?-(X):(X))
# define sqlite3_Is_అధికప్రవాహం(X) 0
# define INFINITY (9223372036854775807LL)
# ifndef SQLITE_BIG_DBL
#   define SQLITE_BIG_DBL (((sqlite3_int64)1)<<50)
# endif
# define SQLITE_OMIT_DATETIME_FUNCS 1
# define SQLITE_OMIT_TRACE 1
# undef SQLITE_MIXED_ENDIAN_64_బిట్టు_FLOAT
# undef SQLITE_HAVE_ISNAN
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_BIG_DBL
# define SQLITE_BIG_DBL (1e99)
#అయితే_ముగింపు

/*
** OMIT_TEMPDB is set to 1 if SQLITE_OMIT_TEMPDB is defined, or 0
** afterward. Having this macro allows us to cause the C compiler
** to omit code used by TEMP tables without messy #ifndef statements.
*/
#నిర్వచితమైతే SQLITE_OMIT_TEMPDB
#నిర్వచించు OMIT_TEMPDB 1
#లేకపోతే
#నిర్వచించు OMIT_TEMPDB 0
#అయితే_ముగింపు

/*
** The "file format" number is an integer that is incremented whenever
** the VDBE-level file format changes.  The following macros define the
** the default file format for new databases and the maximum file format
** that the library can read.
*/
#నిర్వచించు SQLITE_MAX_FILE_FORMAT 4
#నిర్వచితం_కాకపోతే SQLITE_DEFAULT_FILE_FORMAT
# define SQLITE_DEFAULT_FILE_FORMAT 4
#అయితే_ముగింపు

/*
** Determine whether triggers are recursive by default.  This can be
** changed at run-time using a pragma.
*/
#నిర్వచితం_కాకపోతే SQLITE_DEFAULT_RECURSIVE_TRIGGERS
# define SQLITE_DEFAULT_RECURSIVE_TRIGGERS 0
#అయితే_ముగింపు

/*
** Provide a default value for SQLITE_TEMP_STORE in case it is not specified
** on the command-line
*/
#నిర్వచితం_కాకపోతే SQLITE_తాత్కాలిక_STORE
# define SQLITE_తాత్కాలిక_STORE 1
#అయితే_ముగింపు

/*
** If no value has been provided for SQLITE_MAX_WORKER_THREADS, or if
** SQLITE_TEMP_STORE is set to 3 (never use temporary files), set it
** to zero.
*/
#ఒకవేళ SQLITE_తాత్కాలిక_STORE==3 || SQLITE_THREADSAFE==0
# undef SQLITE_MAX_కార్మికుడు_దారాలు
# define SQLITE_MAX_కార్మికుడు_దారాలు 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_MAX_కార్మికుడు_దారాలు
# define SQLITE_MAX_కార్మికుడు_దారాలు 8
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_DEFAULT_కార్మికుడు_దారాలు
# define SQLITE_DEFAULT_కార్మికుడు_దారాలు 0
#అయితే_ముగింపు
#ఒకవేళ SQLITE_DEFAULT_కార్మికుడు_దారాలు>SQLITE_MAX_కార్మికుడు_దారాలు
# undef SQLITE_MAX_కార్మికుడు_దారాలు
# define SQLITE_MAX_కార్మికుడు_దారాలు SQLITE_DEFAULT_కార్మికుడు_దారాలు
#అయితే_ముగింపు

/*
** The default initial allocation for the pagecache when using separate
** pagecaches for each database connection.  A positive number is the
** number of pages.  A negative number N translations means that a buffer
** of -1024*N bytes is allocated and used for as many pages as it will hold.
**
** The default value of "20" was chosen to minimize the run-time of the
** speedtest1 test program with options: --shrink-memory --reprepare
*/
#నిర్వచితం_కాకపోతే SQLITE_DEFAULT_PCACHE_INITSZ
# define SQLITE_DEFAULT_PCACHE_INITSZ 20
#అయితే_ముగింపు

/*
** Default value for the SQLITE_CONFIG_SORTERREF_SIZE option.
*/
#నిర్వచితం_కాకపోతే SQLITE_DEFAULT_SORTERREF_కొలత
# define SQLITE_DEFAULT_SORTERREF_కొలత 0x7fffffff
#అయితే_ముగింపు

/*
** The compile-time options SQLITE_MMAP_READWRITE and
** SQLITE_ENABLE_BATCH_ATOMIC_WRITE are not compatible with one another.
** You must choose one or the other (or neither) but not both.
*/
#ఒకవేళ defined(SQLITE_MMAP_READWRITE) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
#దోషం Cannot use both SQLITE_MMAP_READWRITE and SQLITE_ENABLE_BATCH_ATOMIC_WRITE
#అయితే_ముగింపు

/*
** GCC does not define the offsetof() macro so we'll have to do it
** ourselves.
*/
#నిర్వచితం_కాకపోతే offsetof
# define offsetof(ST,M) ((size_t)((అక్షరం*)&((ST*)0)->M - (అక్షరం*)0))
#అయితే_ముగింపు

/*
** sizeof64() is like sizeof(), but always returns a 64-bit value, even
** on 32-bit builds. This can help to avoid overflow by ensuring 64-bit
** arithmetic is used consistently in both 32-bit and 64-bit builds.
*/
#నిర్వచించు sizeof64(X) ((sqlite3_int64)పరిమాణం(X))

/*
** Work around C99 "flex-array" syntax for pre-C99 compilers, so as
** to avoid complaints from -fsanitize=strict-bounds.
*/
#ఒకవేళ defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define FLEXARRAY
#లేకపోతే
# define FLEXARRAY 1
#అయితే_ముగింపు

/*
** Macros to compute minimum and maximum of two numbers.
*/
#నిర్వచితం_కాకపోతే MIN
# define MIN(A,B) ((A)<(B)?(A):(B))
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే MAX
# define MAX(A,B) ((A)>(B)?(A):(B))
#అయితే_ముగింపు

/*
** Swap two objects of type TYPE.
*/
#నిర్వచించు మార్పిడి(TYPE,A,B) {TYPE t=A; A=B; B=t;}

/*
** Check to see if this machine uses EBCDIC.  (Yes, believe it or
** not, there are still machines out there that use EBCDIC.)
*/
#ఒకవేళ 'A' == '\301'
# define SQLITE_EBCDIC 1
#లేకపోతే
# define SQLITE_ASCII 1
#అయితే_ముగింపు

/*
** Integers of known sizes.  These typedefs might change for architectures
** where the sizes very.  Preprocessor macros are available so that the
** types can be conveniently redefined at compile-type.  Like this:
**
**         cc '-DUINTPTR_TYPE=long long int' ...
*/
#నిర్వచితం_కాకపోతే UINT32_TYPE
# ifdef HAVE_UINT32_T
#  define UINT32_TYPE uint32_t
# లేకపోతే
#  define UINT32_TYPE గుర్తులేని పూర్ణాంకం
# endif
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే UINT16_TYPE
# ifdef HAVE_UINT16_T
#  define UINT16_TYPE uint16_t
# లేకపోతే
#  define UINT16_TYPE గుర్తులేని పొట్టి పూర్ణాంకం
# endif
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే INT16_TYPE
# ifdef HAVE_INT16_T
#  define INT16_TYPE int16_t
# లేకపోతే
#  define INT16_TYPE పొట్టి పూర్ణాంకం
# endif
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే UINT8_TYPE
# ifdef HAVE_UINT8_T
#  define UINT8_TYPE uint8_t
# లేకపోతే
#  define UINT8_TYPE గుర్తులేని అక్షరం
# endif
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే INT8_TYPE
# ifdef HAVE_INT8_T
#  define INT8_TYPE int8_t
# లేకపోతే
#  define INT8_TYPE గుర్తుగల అక్షరం
# endif
#అయితే_ముగింపు
రకనిర్వచనం sqlite_int64 i64;          /* 8-byte signed integer */
రకనిర్వచనం sqlite_uint64 u64;         /* 8-byte unsigned integer */
రకనిర్వచనం UINT32_TYPE u32;           /* 4-byte unsigned integer */
రకనిర్వచనం UINT16_TYPE u16;           /* 2-byte unsigned integer */
రకనిర్వచనం INT16_TYPE i16;            /* 2-byte signed integer */
రకనిర్వచనం UINT8_TYPE u8;             /* 1-byte unsigned integer */
రకనిర్వచనం INT8_TYPE i8;              /* 1-byte signed integer */

/* A bitfield type for use inside of structures.  Always follow with :N where
** N is the number of bits.
*/
రకనిర్వచనం గుర్తులేని bft;  /* Bit Field Type */

/*
** SQLITE_MAX_U32 is a u64 constant that is the maximum u64 value
** that can be stored in a u32 without loss of data.  The value
** is 0x00000000ffffffff.  But because of quirks of some compilers, we
** have to specify the value in the less intuitive manner shown:
*/
#నిర్వచించు SQLITE_MAX_U32  ((((u64)1)<<32)-1)

/*
** The datatype used to store estimates of the number of rows in a
** table or index.
*/
రకనిర్వచనం u64 tRowcnt;

/*
** Estimated quantities used for query planning are stored as 16-bit
** logarithms.  For quantity X, the value stored is 10*log2(X).  This
** gives a possible range of values of approximately 1.0e986 to 1e-986.
** But the allowed values are "grainy".  Not every value is representable.
** For example, quantities 16 and 17 are both represented by a LogEst
** of 40.  However, since LogEst quantities are suppose to be estimates,
** not exact values, this imprecision is not a problem.
**
** "LogEst" is short for "Logarithmic Estimate".
**
** Examples:
**      1 -> 0              20 -> 43          10000 -> 132
**      2 -> 10             25 -> 46          25000 -> 146
**      3 -> 16            100 -> 66        1000000 -> 199
**      4 -> 20           1000 -> 99        1048576 -> 200
**     10 -> 33           1024 -> 100    4294967296 -> 320
**
** The LogEst can be negative to indicate fractional values.
** Examples:
**
**    0.5 -> -10           0.1 -> -33        0.0625 -> -40
*/
రకనిర్వచనం INT16_TYPE LogEst;
#నిర్వచించు LOGEST_MIN (-32768)
#నిర్వచించు LOGEST_MAX (32767)

/*
** Set the SQLITE_PTRSIZE macro to the number of bytes in a pointer
*/
#నిర్వచితం_కాకపోతే SQLITE_PTRSIZE
# ఒకవేళ defined(__SIZEOF_POINTER__)
#   define SQLITE_PTRSIZE __SIZEOF_POINTER__
# elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
       defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
      (defined(__APPLE__) && defined(__ppc__)) ||                         \
      (defined(__TOS_AIX__) && !defined(__64BIT__))
#   define SQLITE_PTRSIZE 4
# లేకపోతే
#   define SQLITE_PTRSIZE 8
# endif
#అయితే_ముగింపు

/* The uptr type is an unsigned integer large enough to hold a pointer
*/
#ఒకవేళ defined(HAVE_STDINT_H)
  రకనిర్వచనం uintptr_t uptr;
#లేదావేళ SQLITE_PTRSIZE==4
  రకనిర్వచనం u32 uptr;
#లేకపోతే
  రకనిర్వచనం u64 uptr;
#అయితే_ముగింపు

/*
** The SQLITE_WITHIN(P,S,E) macro checks to see if pointer P points to
** something between S (inclusive) and E (exclusive).
**
** In other words, S is a buffer and E is a pointer to the first byte after
** the end of buffer S.  This macro returns true if P points to something
** contained within the buffer S.
*/
#నిర్వచించు SQLITE_WITHIN(P,S,E)   (((uptr)(P)>=(uptr)(S))&&((uptr)(P)<(uptr)(E)))

/*
** P is one byte past the end of a large buffer. Return true if a span of bytes
** between S..E crosses the end of that buffer.  In other words, return true
** if the sub-buffer S..E-1 overflows the buffer whose last byte is P-1.
**
** S is the start of the span.  E is one byte past the end of end of span.
**
**                        P
**     |-----------------|                FALSE
**               |-------|
**               S        E
**
**                        P
**     |-----------------|
**                    |-------|           TRUE
**                    S        E
**
**                        P
**     |-----------------|               
**                        |-------|       FALSE
**                        S        E
*/
#నిర్వచించు SQLITE_అధికప్రవాహం(P,S,E) (((uptr)(S)<(uptr)(P))&&((uptr)(E)>(uptr)(P)))

/*
** Macros to determine whether the machine is big or little endian,
** and whether or not that determination is run-time or compile-time.
**
** For best performance, an attempt is made to guess at the byte-order
** using C-preprocessor macros.  If that is unsuccessful, or if
** -DSQLITE_BYTEORDER=0 is set, then byte-order is determined
** at run-time.
**
** If you are building SQLite on some obscure platform for which the
** following ifdef magic does not work, you can always include either:
**
**    -DSQLITE_BYTEORDER=1234
**
** or
**
**    -DSQLITE_BYTEORDER=4321
**
** to cause the build to work for little-endian or big-endian processors,
** respectively.
*/
#నిర్వచితం_కాకపోతే SQLITE_BYTEORDER  /* Replicate changes at tag-20230904a */
# ఒకవేళ defined(__BYTE_ORDER__) && __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
#   define SQLITE_BYTEORDER 4321
# elif defined(__BYTE_ORDER__) && __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
#   define SQLITE_BYTEORDER 1234
# elif defined(__BIG_ENDIAN__) && __BIG_ENDIAN__==1
#   define SQLITE_BYTEORDER 4321
# elif defined(i386)    || defined(__i386__)      || defined(_M_IX86) ||    \
     defined(__x86_64)  || defined(__x86_64__)    || defined(_M_X64)  ||    \
     defined(_M_AMD64)  || defined(_M_ARM)        || defined(__x86)   ||    \
     defined(__ARMEL__) || defined(__AARCH64EL__) || defined(_M_ARM64)
#   define SQLITE_BYTEORDER 1234
# elif defined(sparc)   || defined(__ARMEB__)     || defined(__AARCH64EB__)
#   define SQLITE_BYTEORDER 4321
# లేకపోతే
#   define SQLITE_BYTEORDER 0
# endif
#అయితే_ముగింపు
#ఒకవేళ SQLITE_BYTEORDER==4321
# define SQLITE_BIGENDIAN    1
# define SQLITE_LITTLEENDIAN 0
# define SQLITE_UTF16NATIVE  SQLITE_UTF16BE
#లేదావేళ SQLITE_BYTEORDER==1234
# define SQLITE_BIGENDIAN    0
# define SQLITE_LITTLEENDIAN 1
# define SQLITE_UTF16NATIVE  SQLITE_UTF16LE
#లేకపోతే
# ifdef SQLITE_AMALGAMATION
  స్థిరరాశి పూర్ణాంకం sqlite3one = 1;
# లేకపోతే
  బాహ్య స్థిరరాశి పూర్ణాంకం sqlite3one;
# endif
# define SQLITE_BIGENDIAN    (*(అక్షరం *)(&sqlite3one)==0)
# define SQLITE_LITTLEENDIAN (*(అక్షరం *)(&sqlite3one)==1)
# define SQLITE_UTF16NATIVE  (SQLITE_BIGENDIAN?SQLITE_UTF16BE:SQLITE_UTF16LE)
#అయితే_ముగింపు

/*
** Constants for the largest and smallest possible 64-bit signed integers.
** These macros are designed to work correctly on both 32-bit and 64-bit
** compilers.
*/
#నిర్వచించు అతిపెద్ద_INT64  (0xffffffff|(((i64)0x7fffffff)<<32))
#నిర్వచించు అతిపెద్ద_UINT64 (0xffffffff|(((u64)0xffffffff)<<32))
#నిర్వచించు అతిచిన్న_INT64 (((i64)-1) - అతిపెద్ద_INT64)

/*
** Macro SMXV(n) return the maximum value that can be held in variable n,
** assuming n is a signed integer type.  UMXV(n) is similar for unsigned
** integer types.
*/
#నిర్వచించు SMXV(n) ((((i64)1)<<(పరిమాణం(n)*8-1))-1)
#నిర్వచించు UMXV(n) ((((i64)1)<<(పరిమాణం(n)*8))-1)

/*
** Round up a number to the next larger multiple of 8.  This is used
** to force 8-byte alignment on 64-bit architectures.
**
** ROUND8() always does the rounding, for any argument.
**
** ROUND8P() assumes that the argument is already an integer number of
** pointers in size, and so it is a no-op on systems where the pointer
** size is 8.
*/
#నిర్వచించు ROUND8(x)     (((x)+7)&~7)
#ఒకవేళ SQLITE_PTRSIZE==8
# define ROUND8P(x)   (x)
#లేకపోతే
# define ROUND8P(x)   (((x)+7)&~7)
#అయితే_ముగింపు

/*
** Round down to the nearest multiple of 8
*/
#నిర్వచించు ROUNDDOWN8(x) ((x)&~7)

/*
** Assert that the pointer X is aligned to an 8-byte boundary.  This
** macro is used only within assert() to verify that the code gets
** all alignment restrictions correct.
**
** Except, if SQLITE_4_BYTE_ALIGNED_MALLOC is defined, then the
** underlying malloc() implementation might return us 4-byte aligned
** pointers.  In that case, only verify 4-byte alignment.
*/
#నిర్వచితమైతే SQLITE_4_BYTE_ALIGNED_MALLOC
# define EIGHT_BYTE_ALIGNMENT(X)   ((((uptr)(X) - (uptr)0)&3)==0)
#లేకపోతే
# define EIGHT_BYTE_ALIGNMENT(X)   ((((uptr)(X) - (uptr)0)&7)==0)
#అయితే_ముగింపు
#నిర్వచించు TWO_BYTE_ALIGNMENT(X)      ((((uptr)(X) - (uptr)0)&1)==0)

/*
** Disable MMAP on platforms where it is known to not work
*/
#ఒకవేళ defined(__OpenBSD__) || defined(__QNXNTO__)
# undef SQLITE_MAX_MMAP_కొలత
# define SQLITE_MAX_MMAP_కొలత 0
#అయితే_ముగింపు

/*
** Default maximum size of memory used by memory-mapped I/O in the VFS
*/
#నిర్వచితమైతే __APPLE__
# include <TargetConditionals.h>
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_MAX_MMAP_కొలత
# ఒకవేళ defined(__linux__) \
  || defined(_WIN32) \
  || (defined(__APPLE__) && defined(__MACH__)) \
  || defined(__sun) \
  || defined(__FreeBSD__) \
  || defined(__DragonFly__)
#   define SQLITE_MAX_MMAP_కొలత 0x7fff0000  /* 2147418112 */
# లేకపోతే
#   define SQLITE_MAX_MMAP_కొలత 0
# endif
#అయితే_ముగింపు

/*
** The default MMAP_SIZE is zero on all platforms.  Or, even if a larger
** default MMAP_SIZE is specified at compile-time, make sure that it does
** not exceed the maximum mmap size.
*/
#నిర్వచితం_కాకపోతే SQLITE_DEFAULT_MMAP_కొలత
# define SQLITE_DEFAULT_MMAP_కొలత 0
#అయితే_ముగింపు
#ఒకవేళ SQLITE_DEFAULT_MMAP_కొలత>SQLITE_MAX_MMAP_కొలత
# undef SQLITE_DEFAULT_MMAP_కొలత
# define SQLITE_DEFAULT_MMAP_కొలత SQLITE_MAX_MMAP_కొలత
#అయితే_ముగింపు

/*
** TREETRACE_ENABLED will be either 1 or 0 depending on whether or not
** the Abstract Syntax Tree tracing logic is turned on.
*/
#ఒకవేళ !defined(SQLITE_AMALGAMATION)
బాహ్య u32 sqlite3_వృక్షం_Trace;
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_DEBUG) \
    && (defined(SQLITE_TEST) || defined(SQLITE_ENABLE_SELECTTRACE) \
                             || defined(SQLITE_ENABLE_TREETRACE))
# define TREETRACE_ENABLED 1
# define TREETRACE(K,P,S,X)  \
  ఒకవేళ(sqlite3_వృక్షం_Trace&(K))   \
    sqlite3DebugPrintf("%u/%d/%p: ",(S)->selId,(P)->addrExplain,(S)),\
    sqlite3DebugPrintf X
#లేకపోతే
# define TREETRACE(K,P,S,X)
# define TREETRACE_ENABLED 0
#అయితే_ముగింపు

/* TREETRACE flag meanings:
**
**   0x00000001     Beginning and end of SELECT processing
**   0x00000002     WHERE clause processing
**   0x00000004     Query flattener
**   0x00000008     Result-set wildcard expansion
**   0x00000010     Query name resolution
**   0x00000020     Aggregate analysis
**   0x00000040     Window functions
**   0x00000080     Generated column names
**   0x00000100     Move HAVING terms into WHERE
**   0x00000200     Count-of-view optimization
**   0x00000400     Compound SELECT processing
**   0x00000800     Drop superfluous ORDER BY
**   0x00001000     LEFT JOIN simplifies to JOIN
**   0x00002000     Constant propagation
**   0x00004000     Push-down optimization
**   0x00008000     After all FROM-clause analysis
**   0x00010000     Beginning of DELETE/INSERT/UPDATE processing
**   0x00020000     Transform DISTINCT into GROUP BY
**   0x00040000     SELECT tree dump after all code has been generated
**   0x00080000     NOT NULL strength reduction
**   0x00100000     Pointers are all shown as zero
**   0x00200000     EXISTS-to-JOIN optimization
*/

/*
** Macros for "wheretrace"
*/
బాహ్య u32 sqlite3WhereTrace;
#ఒకవేళ defined(SQLITE_DEBUG) \
    && (defined(SQLITE_TEST) || defined(SQLITE_ENABLE_WHERETRACE))
# define WHERETRACE(K,X)  ఒకవేళ(sqlite3WhereTrace&(K)) sqlite3DebugPrintf X
# define WHERETRACE_ENABLED 1
#లేకపోతే
# define WHERETRACE(K,X)
#అయితే_ముగింపు

/*
** Bits for the sqlite3WhereTrace mask:
**
** (---any--)   Top-level block structure
** 0x-------F   High-level debug messages
** 0x----FFF-   More detail
** 0xFFFF----   Low-level debug messages
**
** 0x00000001   Code generation
** 0x00000002   Solver (Use 0x40000 for less detail)
** 0x00000004   Solver costs
** 0x00000008   WhereLoop inserts
**
** 0x00000010   Display sqlite3_index_info xBestIndex calls
** 0x00000020   Range an equality scan metrics
** 0x00000040   IN operator decisions
** 0x00000080   WhereLoop cost adjustments
** 0x00000100
** 0x00000200   Covering index decisions
** 0x00000400   OR optimization
** 0x00000800   Index scanner
** 0x00001000   More details associated with code generation
** 0x00002000
** 0x00004000   Show all WHERE terms at key points
** 0x00008000   Show the full SELECT statement at key places
**
** 0x00010000   Show more detail when printing WHERE terms
** 0x00020000   Show WHERE terms returned from whereScanNext()
** 0x00040000   Solver overview messages
** 0x00080000   Star-query heuristic
** 0x00100000   Pointers are all shown as zero
*/


/*
** An instance of the following structure is used to store the busy-handler
** callback for a given sqlite handle.
**
** The sqlite.busyHandler member of the sqlite struct contains the busy
** callback for the database handle. Each pager opened via the sqlite
** handle is passed a pointer to sqlite.busyHandler. The busy-handler
** callback is currently invoked only from within pager.c.
*/
రకనిర్వచనం నిర్మాణం BusyHandler BusyHandler;
నిర్మాణం BusyHandler {
  పూర్ణాంకం (*xBusyHandler)(శూన్య *,పూర్ణాంకం);  /* The busy callback */
  శూన్య *pBusyArg;                   /* First arg to busy callback */
  పూర్ణాంకం nBusy;                        /* Incremented with each busy call */
};

/*
** Name of table that holds the database schema.
**
** The PREFERRED names are used wherever possible.  But LEGACY is also
** used for backwards compatibility.
**
**  1.  Queries can use either the PREFERRED or the LEGACY names
**  2.  The sqlite3_set_authorizer() callback uses the LEGACY name
**  3.  The PRAGMA table_list statement uses the PREFERRED name
**
** The LEGACY names are stored in the internal symbol hash table
** in support of (2).  Names are translated using sqlite3PreferredTableName()
** for (3).  The sqlite3FindTable() function takes care of translating
** names for (1).
**
** Note that "sqlite_temp_schema" can also be called "temp.sqlite_schema".
*/
#నిర్వచించు LEGACY_SCHEMA_పట్టిక          "sqlite_master"
#నిర్వచించు LEGACY_తాత్కాలిక_SCHEMA_పట్టిక     "sqlite_temp_master"
#నిర్వచించు PREFERRED_SCHEMA_పట్టిక       "sqlite_schema"
#నిర్వచించు PREFERRED_తాత్కాలిక_SCHEMA_పట్టిక  "sqlite_temp_schema"


/*
** The root-page of the schema table.
*/
#నిర్వచించు SCHEMA_మూలం    1

/*
** The name of the schema table.  The name is different for TEMP.
*/
#నిర్వచించు SCHEMA_పట్టిక(x) \
    ((!OMIT_TEMPDB)&&(x==1)?LEGACY_తాత్కాలిక_SCHEMA_పట్టిక:LEGACY_SCHEMA_పట్టిక)

/*
** A convenience macro that returns the number of elements in
** an array.
*/
#నిర్వచించు Array_కొలత(X)    ((పూర్ణాంకం)(పరిమాణం(X)/పరిమాణం(X[0])))

/*
** Determine if the argument is a power of two
*/
#నిర్వచించు Is_Power_యొక్క_Two(X) (((X)&((X)-1))==0)

/*
** The following value as a destructor means to use sqlite3DbFree().
** The sqlite3DbFree() routine requires two parameters instead of the
** one parameter that destructors normally want.  So we have to introduce
** this magic value that the code knows to handle differently.  Any
** pointer will work here as long as it is distinct from SQLITE_STATIC
** and SQLITE_TRANSIENT.
*/
#నిర్వచించు SQLITE_DYNAMIC   ((sqlite3_destructor_type)sqlite3_అడ్డువరుస_సమితి_Clear)

/*
** When SQLITE_OMIT_WSD is defined, it means that the target platform does
** not support Writable Static Data (WSD) such as global and static variables.
** All variables must either be on the stack or dynamically allocated from
** the heap.  When WSD is unsupported, the variable declarations scattered
** throughout the SQLite code must become constants instead.  The SQLITE_WSD
** macro is used for this purpose.  And instead of referencing the variable
** directly, we use its constant as a key to lookup the run-time allocated
** buffer that holds real variable.  The constant is also the initializer
** for the run-time allocated buffer.
**
** In the usual case where WSD is supported, the SQLITE_WSD and GLOBAL
** macros become no-ops and have zero performance impact.
*/
#నిర్వచితమైతే SQLITE_OMIT_WSD
  #నిర్వచించు SQLITE_WSD స్థిరరాశి
  #నిర్వచించు GLOBAL(t,v) (*(t*)sqlite3_wsd_కనుగొను((శూన్య*)&(v), పరిమాణం(v)))
  #నిర్వచించు sqlite3GlobalConfig GLOBAL(నిర్మాణం Sqlite3Config, sqlite3Config)
  పూర్ణాంకం sqlite3_wsd_init(పూర్ణాంకం N, పూర్ణాంకం J);
  శూన్య *sqlite3_wsd_కనుగొను(శూన్య *K, పూర్ణాంకం L);
#లేకపోతే
  #నిర్వచించు SQLITE_WSD
  #నిర్వచించు GLOBAL(t,v) v
  #నిర్వచించు sqlite3GlobalConfig sqlite3Config
#అయితే_ముగింపు

/*
** The following macros are used to suppress compiler warnings and to
** make it clear to human readers when a function parameter is deliberately
** left unused within the body of a function. This usually happens when
** a function is called via a function pointer. For example the
** implementation of an SQL aggregate step callback may not use the
** parameter indicating the number of arguments passed to the aggregate,
** if it knows that this is enforced elsewhere.
**
** When a function parameter is not used at all within the body of a function,
** it is generally named "NotUsed" or "NotUsed2" to make things even clearer.
** However, these macros may also be used to suppress warnings related to
** parameters that may or may not be used depending on compilation options.
** For example those parameters only used in assert() statements. In these
** cases the parameters are named as per the usual conventions.
*/
#నిర్వచించు వాడని_PARAMETER(x) (శూన్య)(x)
#నిర్వచించు వాడని_PARAMETER2(x,y) వాడని_PARAMETER(x),వాడని_PARAMETER(y)

/*
** Forward references to structures
*/
రకనిర్వచనం నిర్మాణం AggInfo AggInfo;
రకనిర్వచనం నిర్మాణం AuthContext AuthContext;
రకనిర్వచనం నిర్మాణం AutoincInfo AutoincInfo;
రకనిర్వచనం నిర్మాణం Bitvec Bitvec;
రకనిర్వచనం నిర్మాణం CollSeq CollSeq;
రకనిర్వచనం నిర్మాణం స్తంభం స్తంభం;
రకనిర్వచనం నిర్మాణం Cte Cte;
రకనిర్వచనం నిర్మాణం CteUse CteUse;
రకనిర్వచనం నిర్మాణం Db Db;
రకనిర్వచనం నిర్మాణం Db_Client_దత్తాంశం Db_Client_దత్తాంశం;
రకనిర్వచనం నిర్మాణం DbFixer DbFixer;
రకనిర్వచనం నిర్మాణం Schema Schema;
రకనిర్వచనం నిర్మాణం సమాసం సమాసం;
రకనిర్వచనం నిర్మాణం సమాసం_List సమాసం_List;
రకనిర్వచనం నిర్మాణం FKey FKey;
రకనిర్వచనం నిర్మాణం FpDecode FpDecode;
రకనిర్వచనం నిర్మాణం FuncDestructor FuncDestructor;
రకనిర్వచనం నిర్మాణం FuncDef FuncDef;
రకనిర్వచనం నిర్మాణం FuncDefHash FuncDefHash;
రకనిర్వచనం నిర్మాణం IdList IdList;
రకనిర్వచనం నిర్మాణం సూచీ సూచీ;
రకనిర్వచనం నిర్మాణం సూచీకృత_సమాసం సూచీకృత_సమాసం;
రకనిర్వచనం నిర్మాణం సూచీ_Sample సూచీ_Sample;
రకనిర్వచనం నిర్మాణం కీలకం_Class కీలకం_Class;
రకనిర్వచనం నిర్మాణం కీలకం_Info కీలకం_Info;
రకనిర్వచనం నిర్మాణం Lookaside Lookaside;
రకనిర్వచనం నిర్మాణం Lookaside_స్థానం Lookaside_స్థానం;
రకనిర్వచనం నిర్మాణం Module Module;
రకనిర్వచనం నిర్మాణం NameContext NameContext;
రకనిర్వచనం నిర్మాణం OnOrUsing OnOrUsing;
రకనిర్వచనం నిర్మాణం Parse Parse;
రకనిర్వచనం నిర్మాణం ParseCleanup ParseCleanup;
రకనిర్వచనం నిర్మాణం PreUpdate PreUpdate;
రకనిర్వచనం నిర్మాణం PrintfArguments PrintfArguments;
రకనిర్వచనం నిర్మాణం RCStr RCStr;
రకనిర్వచనం నిర్మాణం RenameToken RenameToken;
రకనిర్వచనం నిర్మాణం Returning Returning;
రకనిర్వచనం నిర్మాణం అడ్డువరుస_సమితి అడ్డువరుస_సమితి;
రకనిర్వచనం నిర్మాణం Savepoint Savepoint;
రకనిర్వచనం నిర్మాణం Select Select;
రకనిర్వచనం నిర్మాణం SQLiteThread SQLiteThread;
రకనిర్వచనం నిర్మాణం SelectDest SelectDest;
రకనిర్వచనం నిర్మాణం Subquery Subquery;
రకనిర్వచనం నిర్మాణం Src_అంశం Src_అంశం;
రకనిర్వచనం నిర్మాణం SrcList SrcList;
రకనిర్వచనం నిర్మాణం sqlite3_str StrAccum; /* Internal alias for sqlite3_str */
రకనిర్వచనం నిర్మాణం పట్టిక పట్టిక;
రకనిర్వచనం నిర్మాణం పట్టిక_తాళం పట్టిక_తాళం;
రకనిర్వచనం నిర్మాణం Token Token;
రకనిర్వచనం నిర్మాణం వృక్షం_View వృక్షం_View;
రకనిర్వచనం నిర్మాణం Trigger Trigger;
రకనిర్వచనం నిర్మాణం TriggerPrg TriggerPrg;
రకనిర్వచనం నిర్మాణం Trigger_అడుగు Trigger_అడుగు;
రకనిర్వచనం నిర్మాణం Unpacked_రికార్డు Unpacked_రికార్డు;
రకనిర్వచనం నిర్మాణం Upsert Upsert;
రకనిర్వచనం నిర్మాణం VTable VTable;
రకనిర్వచనం నిర్మాణం VtabCtx VtabCtx;
రకనిర్వచనం నిర్మాణం Walker Walker;
రకనిర్వచనం నిర్మాణం WhereInfo WhereInfo;
రకనిర్వచనం నిర్మాణం కిటికీ కిటికీ;
రకనిర్వచనం నిర్మాణం With With;


/*
** The bitmask datatype defined below is used for various optimizations.
**
** Changing this from a 64-bit to a 32-bit type limits the number of
** tables in a join to 32 instead of 64.  But it also reduces the size
** of the library by 738 bytes on ix86.
*/
#నిర్వచితమైతే SQLITE_BITMASK_TYPE
  రకనిర్వచనం SQLITE_BITMASK_TYPE Bitmask;
#లేకపోతే
  రకనిర్వచనం u64 Bitmask;
#అయితే_ముగింపు

/*
** The number of bits in a Bitmask.  "BMS" means "BitMask Size".
*/
#నిర్వచించు BMS  ((పూర్ణాంకం)(పరిమాణం(Bitmask)*8))

/*
** A bit in a Bitmask
*/
#నిర్వచించు MASKBIT(n)    (((Bitmask)1)<<(n))
#నిర్వచించు MASKBIT64(n)  (((u64)1)<<(n))
#నిర్వచించు MASKBIT32(n)  (((గుర్తులేని పూర్ణాంకం)1)<<(n))
#నిర్వచించు SMASKBIT32(n) ((n)<=31?((గుర్తులేని పూర్ణాంకం)1)<<(n):0)
#నిర్వచించు ALLBITS       ((Bitmask)-1)
#నిర్వచించు TOPBIT        (((Bitmask)1)<<(BMS-1))

/* A VList object records a mapping between parameters/variables/wildcards
** in the SQL statement (such as $abc, @pqr, or :xyz) and the integer
** variable number associated with that parameter.  See the format description
** on the sqlite3VListAdd() routine for more information.  A VList is really
** just an array of integers.
*/
రకనిర్వచనం పూర్ణాంకం VList;

/*
** Defer sourcing vdbe.h and btree.h until after the "u8" and
** "BusyHandler" typedefs. vdbe.h also requires a few of the opaque
** pointer types (i.e. FuncDef) defined above.
*/
#అన్తర్భావ "os.h"
#అన్తర్భావ "pager.h"
#అన్తర్భావ "btree.h"
#అన్తర్భావ "vdbe.h"
#అన్తర్భావ "pcache.h"
#అన్తర్భావ "mutex.h"

/* The SQLITE_EXTRA_DURABLE compile-time option used to set the default
** synchronous setting to EXTRA.  It is no longer supported.
*/
#నిర్వచితమైతే SQLITE_EXTRA_DURABLE
# warning Use SQLITE_DEFAULT_SYNCHRONOUS=3 instead యొక్క SQLITE_EXTRA_DURABLE
# define SQLITE_DEFAULT_SYNCHRONOUS 3
#అయితే_ముగింపు

/*
** Default synchronous levels.
**
** Note that (for historical reasons) the PAGER_SYNCHRONOUS_* macros differ
** from the SQLITE_DEFAULT_SYNCHRONOUS value by 1.
**
**           PAGER_SYNCHRONOUS       DEFAULT_SYNCHRONOUS
**   OFF           1                         0
**   NORMAL        2                         1
**   FULL          3                         2
**   EXTRA         4                         3
**
** The "PRAGMA synchronous" statement also uses the zero-based numbers.
** In other words, the zero-based numbers are used for all external interfaces
** and the one-based values are used internally.
*/
#నిర్వచితం_కాకపోతే SQLITE_DEFAULT_SYNCHRONOUS
# define SQLITE_DEFAULT_SYNCHRONOUS 2
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_DEFAULT_WAL_SYNCHRONOUS
# define SQLITE_DEFAULT_WAL_SYNCHRONOUS SQLITE_DEFAULT_SYNCHRONOUS
#అయితే_ముగింపు

/*
** Each database file to be accessed by the system is an instance
** of the following structure.  There are normally two of these structures
** in the sqlite.aDb[] array.  aDb[0] is the main database file and
** aDb[1] is the database file used to hold temporary tables.  Additional
** databases may be attached.
*/
నిర్మాణం Db {
  అక్షరం *zDbSName;      /* Name of this database. (schema name, not filename) */
  Btree *pBt;          /* The B*Tree structure for this database file */
  u8 safety_level;     /* How aggressive at syncing data to disk */
  u8 b_Sync_సమితి;         /* True if "PRAGMA synchronous=N" has been run */
  Schema *pSchema;     /* Pointer to database schema (possibly shared) */
};

/*
** An instance of the following structure stores a database schema.
**
** Most Schema objects are associated with a Btree.  The exception is
** the Schema for the TEMP database (sqlite3.aDb[1]) which is free-standing.
** In shared cache mode, a single Schema object can be shared by multiple
** Btrees that refer to the same underlying BtShared object.
**
** Schema objects are automatically deallocated when the last Btree that
** references them is destroyed.   The TEMP Schema is manually freed by
** sqlite3_close().
*
** A thread must be holding a mutex on the corresponding Btree in order
** to access Schema content.  This implies that the thread must also be
** holding a mutex on the sqlite3 connection pointer that owns the Btree.
** For a TEMP Schema, only the connection mutex is required.
*/
నిర్మాణం Schema {
  పూర్ణాంకం schema_cookie;   /* Database schema version number for this file */
  పూర్ణాంకం iGeneration;     /* Generation counter.  Incremented with each change */
  Hash tblHash;        /* All tables indexed by name */
  Hash idxHash;        /* All (named) indices indexed by name */
  Hash trigHash;       /* All triggers indexed by name */
  Hash fkeyHash;       /* All foreign keys by referenced table name */
  పట్టిక *pSeqTab;      /* The sqlite_sequence table used by AUTOINCREMENT */
  u8 file_format;      /* Schema format version for this file */
  u8 enc;              /* Text encoding used by this database */
  u16 schemaFlags;     /* Flags associated with this schema */
  పూర్ణాంకం cache_కొలత;      /* Number of pages to use in the cache */
};

/*
** These macros can be used to test, set, or clear bits in the
** Db.pSchema->flags field.
*/
#నిర్వచించు DbHasProperty(D,I,P)     (((D)->aDb[I].pSchema->schemaFlags&(P))==(P))
#నిర్వచించు DbHasAnyProperty(D,I,P)  (((D)->aDb[I].pSchema->schemaFlags&(P))!=0)
#నిర్వచించు Db_సమితి_Property(D,I,P)     (D)->aDb[I].pSchema->schemaFlags|=(P)
#నిర్వచించు DbClearProperty(D,I,P)   (D)->aDb[I].pSchema->schemaFlags&=~(P)

/*
** Allowed values for the DB.pSchema->flags field.
**
** The DB_SchemaLoaded flag is set after the database schema has been
** read into internal hash tables.
**
** DB_UnresetViews means that one or more views have column names that
** have been filled out.  If the schema changes, these column names might
** changes and so the view will need to be reset.
*/
#నిర్వచించు DB_SchemaLoaded    0x0001  /* The schema has been loaded */
#నిర్వచించు DB_UnresetViews    0x0002  /* Some views have defined column names */
#నిర్వచించు DB_ResetWanted     0x0008  /* Reset the schema when nSchemaLock==0 */

/*
** The number of different kinds of things that can be limited
** using the sqlite3_limit() interface.
*/
#నిర్వచించు SQLITE_N_హద్దు (SQLITE_హద్దు_CTES+1)

/*
** Lookaside malloc is a set of fixed-size buffers that can be used
** to satisfy small transient memory allocation requests for objects
** associated with a particular database connection.  The use of
** lookaside malloc provides a significant performance enhancement
** (approx 10%) by avoiding numerous malloc/free requests while parsing
** SQL statements.
**
** The Lookaside structure holds configuration information about the
** lookaside malloc subsystem.  Each available memory allocation in
** the lookaside subsystem is stored on a linked list of LookasideSlot
** objects.
**
** Lookaside allocations are only allowed for objects that are associated
** with a particular database connection.  Hence, schema information cannot
** be stored in lookaside because in shared cache mode the schema information
** is shared by multiple database connections.  Therefore, while parsing
** schema information, the Lookaside.bEnabled flag is cleared so that
** lookaside allocations are not used to construct the schema objects.
**
** New lookaside allocations are only allowed if bDisable==0.  When
** bDisable is greater than zero, sz is set to zero which effectively
** disables lookaside without adding a new test for the bDisable flag
** in a performance-critical path.  sz should be set by to szTrue whenever
** bDisable changes back to zero.
**
** Lookaside buffers are initially held on the pInit list.  As they are
** used and freed, they are added back to the pFree list.  New allocations
** come off of pFree first, then pInit as a fallback.  This dual-list
** allows use to compute a high-water mark - the maximum number of allocations
** outstanding at any point in the past - by subtracting the number of
** allocations on the pInit list from the total number of allocations.
**
** Enhancement on 2019-12-12:  Two-size-lookaside
** The default lookaside configuration is 100 slots of 1200 bytes each.
** The larger slot sizes are important for performance, but they waste
** a lot of space, as most lookaside allocations are less than 128 bytes.
** The two-size-lookaside enhancement breaks up the lookaside allocation
** into two pools:  One of 128-byte slots and the other of the default size
** (1200-byte) slots.   Allocations are filled from the small-pool first,
** failing over to the full-size pool if that does not work.  Thus more
** lookaside slots are available while also using less memory.
** This enhancement can be omitted by compiling with
** SQLITE_OMIT_TWOSIZE_LOOKASIDE.
*/
నిర్మాణం Lookaside {
  u32 bDisable;           /* Only operate the lookaside when zero */
  u16 sz;                 /* Size of each buffer in bytes */
  u16 szTrue;             /* True value of sz, even if disabled */
  u8 bMalloced;           /* True if pStart obtained from sqlite3_malloc() */
  u32 n_స్థానం;              /* Number of lookaside slots allocated */
  u32 anStat[3];          /* 0: hits.  1: size misses.  2: full misses */
  Lookaside_స్థానం *pInit;   /* List of buffers not previously used */
  Lookaside_స్థానం *pFree;   /* List of available buffers */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TWOSIZE_LOOKASIDE
  Lookaside_స్థానం *pSmallInit; /* List of small buffers not previously used */
  Lookaside_స్థానం *pSmallFree; /* List of available small buffers */
  శూన్య *p_మధ్య;          /* First byte past end of full-size buffers and
                          ** the first byte of LOOKASIDE_SMALL buffers */
#అయితే_ముగింపు /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  శూన్య *p_ప్రారంభం;           /* First byte of available memory space */
  శూన్య *p_చివర;             /* First byte past end of available space */
  శూన్య *p_True_చివర;         /* True value of pEnd, when db->pnBytesFreed!=0 */
};
నిర్మాణం Lookaside_స్థానం {
  Lookaside_స్థానం *p_తదుపరి;    /* Next buffer in the list of free buffers */
};

#నిర్వచించు DisableLookaside  db->lookaside.bDisable++;db->lookaside.sz=0
#నిర్వచించు EnableLookaside   db->lookaside.bDisable--;\
   db->lookaside.sz=db->lookaside.bDisable?0:db->lookaside.szTrue

/* Size of the smaller allocations in two-size lookaside */
#నిర్వచితమైతే SQLITE_OMIT_TWOSIZE_LOOKASIDE
#  define LOOKASIDE_SMALL           0
#లేకపోతే
#  define LOOKASIDE_SMALL         128
#అయితే_ముగింపు

/*
** A hash table for built-in function definitions.  (Application-defined
** functions use a regular table table from hash.h.)
**
** Hash each FuncDef structure into one of the FuncDefHash.a[] slots.
** Collisions are on the FuncDef.u.pHash chain.  Use the SQLITE_FUNC_HASH()
** macro to compute a hash on the function name.
*/
#నిర్వచించు SQLITE_FUNC_HASH_SZ 23
నిర్మాణం FuncDefHash {
  FuncDef *a[SQLITE_FUNC_HASH_SZ];       /* Hash table for functions */
};
#నిర్వచించు SQLITE_FUNC_HASH(C,L) (((C)+(L))%SQLITE_FUNC_HASH_SZ)

/*
** typedef for the authorization callback function.
*/
రకనిర్వచనం పూర్ణాంకం (*sqlite3_xauth)(శూన్య*,పూర్ణాంకం,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,
                             స్థిరరాశి అక్షరం*);

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
/* This is an extra SQLITE_TRACE macro that indicates "legacy" tracing
** in the style of sqlite3_trace()
*/
#నిర్వచించు SQLITE_TRACE_LEGACY          0x40     /* Use the legacy xTrace */
#నిర్వచించు SQLITE_TRACE_XPROFILE        0x80     /* Use the legacy xProfile */
#లేకపోతే
#నిర్వచించు SQLITE_TRACE_LEGACY          0
#నిర్వచించు SQLITE_TRACE_XPROFILE        0
#అయితే_ముగింపు /* SQLITE_OMIT_DEPRECATED */
#నిర్వచించు SQLITE_TRACE_NONLEGACY_ముసుగు  0x0f     /* Normal flags */

/*
** Maximum number of sqlite3.aDb[] entries.  This is the number of attached
** databases plus 2 for "main" and "temp".
*/
#నిర్వచించు SQLITE_MAX_DB (SQLITE_MAX_ATTACHED+2)

/*
** Each database connection is an instance of the following structure.
*/
నిర్మాణం sqlite3 {
  sqlite3_vfs *pVfs;            /* OS Interface */
  నిర్మాణం Vdbe *pVdbe;           /* List of active virtual machines */
  CollSeq *pDfltColl;           /* BINARY collseq for the database encoding */
  sqlite3_mutex *mutex;         /* Connection mutex */
  Db *aDb;                      /* All backends */
  పూర్ణాంకం nDb;                      /* Number of backends currently in use */
  u32 mDbFlags;                 /* flags recording internal state */
  u64 flags;                    /* flags settable by pragmas. See below */
  i64 చివరి_Rowid;                /* ROWID of most recent insert (see above) */
  i64 szMmap;                   /* Default mmap_size setting */
  u32 n_Schema_తాళం;              /* Do not reset the schema when non-zero */
  గుర్తులేని పూర్ణాంకం openFlags;       /* Flags passed to sqlite3_vfs.xOpen() */
  పూర్ణాంకం err_సంకేతం;                  /* Most recent error code (SQLITE_*) */
  పూర్ణాంకం err_Byte_స్థానభ్రంశం;            /* Byte offset of error in SQL statement */
  పూర్ణాంకం err_ముసుగు;                  /* & result codes with this before returning */
  పూర్ణాంకం iSysErrno;                /* Errno value from last system error */
  u8 enc;                       /* Text encoding */
  u8 autoCommit;                /* The auto-commit flag. */
  u8 తాత్కాలిక_store;                /* 1: file 2: memory 0: default */
  u8 mallocFailed;              /* True if we have seen a malloc failure */
  u8 bBenignMalloc;             /* Do not require OOMs if true */
  u8 dflt_తాళం_బహుళకం;              /* Default locking-mode for attached dbs */
  గుర్తుగల అక్షరం తదుపరి_Autovac;      /* Autovac setting after VACUUM if >=0 */
  u8 suppressErr;               /* Do not issue error messages if true */
  u8 vtabOnConflict;            /* Value to return for s3_vtab_on_conflict() */
  u8 isTransactionSavepoint;    /* True if the outermost savepoint is a TS */
  u8 mTrace;                    /* zero or more SQLITE_TRACE flags */
  u8 noSharedCache;             /* True if no shared-cache backends */
  u8 nSqlExec;                  /* Number of pending OP_SqlExec opcodes */
  u8 eOpenState;                /* Current condition of the connection */
  u8 nFpDigit;                  /* Significant digits to keep on double->text */
  పూర్ణాంకం తదుపరి_Pagesize;             /* Pagesize after VACUUM if >0 */
  i64 nChange;                  /* Value returned by sqlite3_changes() */
  i64 n_మొత్తం_Change;             /* Value returned by sqlite3_total_changes() */
  u32 dbOptFlags;               /* Flags to enable/disable optimizations */
  పూర్ణాంకం a_హద్దు[SQLITE_N_హద్దు];   /* Limits */
  పూర్ణాంకం nMaxSorterMmap;           /* Maximum size of regions mapped by sorter */
  నిర్మాణం sqlite3InitInfo {      /* Information used during initialization */
    Pgno newTnum;               /* Rootpage of table being initialized */
    u8 iDb;                     /* Which db file is being initialized */
    u8 busy;                    /* TRUE if initing. 2 if due to ADD COLUMN */
    గుర్తులేని orphanTrigger : 1; /* Last statement is orphaned TEMP trigger */
    గుర్తులేని imposter_పట్టిక : 2; /* Building an imposter table */
    గుర్తులేని reopenMemdb : 1;   /* ATTACH is really a reopen using MemDB */
    స్థిరరాశి అక్షరం **azInit;        /* "type", "name", and "tbl_name" columns */
  } init;
  పూర్ణాంకం n_Vdbe_చురుకైన;              /* Number of VDBEs currently running */
  పూర్ణాంకం nVdbeRead;                /* Number of active VDBEs that read or write */
  పూర్ణాంకం nVdbeWrite;               /* Number of active VDBEs that read and write */
  పూర్ణాంకం nVdbeExec;                /* Number of nested calls to VdbeExec() */
  పూర్ణాంకం nVDestroy;                /* Number of active OP_VDestroy operations */
  పూర్ణాంకం nExtension;               /* Number of loaded extensions */
  శూన్య **aExtension;            /* Array of shared library handles */
  సంఘం {
    శూన్య (*xLegacy)(శూన్య*,స్థిరరాశి అక్షరం*);   /* mTrace==SQLITE_TRACE_LEGACY */
    పూర్ణాంకం (*xV2)(u32,శూన్య*,శూన్య*,శూన్య*);    /* All other mTrace values */
  } trace;
  శూన్య *pTraceArg;                        /* Argument to the trace function */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
  శూన్య (*xProfile)(శూన్య*,స్థిరరాశి అక్షరం*,u64);  /* Profiling function */
  శూన్య *pProfileArg;                        /* Argument to profile function */
#అయితే_ముగింపు
  శూన్య *pCommitArg;                 /* Argument to xCommitCallback() */
  పూర్ణాంకం (*xCommitCallback)(శూన్య*);    /* Invoked at every commit. */
  శూన్య *pRollbackArg;               /* Argument to xRollbackCallback() */
  శూన్య (*xRollbackCallback)(శూన్య*); /* Invoked at every commit. */
  శూన్య *pUpdateArg;
  శూన్య (*xUpdateCallback)(శూన్య*,పూర్ణాంకం, స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,sqlite_int64);
  శూన్య *p_Autovac_పుటలు_Arg;           /* Client argument to autovac_pages */
  శూన్య (*xAutovacDestr)(శూన్య*);     /* Destructor for pAutovacPAgesArg */
  గుర్తులేని పూర్ణాంకం (*x_Autovac_పుటలు)(శూన్య*,స్థిరరాశి అక్షరం*,u32,u32,u32);
  Parse *pParse;                /* Current parse */
#నిర్వచితమైతే SQLITE_ENABLE_PREUPDATE_HOOK
  శూన్య *pPreUpdateArg;          /* First argument to xPreUpdateCallback */
  శూన్య (*xPreUpdateCallback)(   /* Registered using sqlite3_preupdate_hook() */
    శూన్య*,sqlite3*,పూర్ణాంకం,అక్షరం స్థిరరాశి*,అక్షరం స్థిరరాశి*,sqlite3_int64,sqlite3_int64
  );
  PreUpdate *pPreUpdate;        /* Context for active pre-update callback */
#అయితే_ముగింపు /* SQLITE_ENABLE_PREUPDATE_HOOK */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  పూర్ణాంకం (*xWalCallback)(శూన్య *, sqlite3 *, స్థిరరాశి అక్షరం *, పూర్ణాంకం);
  శూన్య *pWalArg;
#అయితే_ముగింపు
  శూన్య(*xCollNeeded)(శూన్య*,sqlite3*,పూర్ణాంకం e_పాఠ్యం_Rep,స్థిరరాశి అక్షరం*);
  శూన్య(*xCollNeeded16)(శూన్య*,sqlite3*,పూర్ణాంకం e_పాఠ్యం_Rep,స్థిరరాశి శూన్య*);
  శూన్య *pCollNeededArg;
  sqlite3_విలువ *pErr;          /* Most recent error message */
  సంఘం {
    అస్థిర పూర్ణాంకం isInterrupted; /* True if sqlite3_interrupt has been called */
    ద్విగుణ notUsed1;            /* Spacer */
  } u1;
  Lookaside lookaside;          /* Lookaside malloc configuration */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth;          /* Access authorization function */
  శూన్య *pAuthArg;               /* 1st argument to the access auth function */
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_పురోగతి_CALLBACK
  పూర్ణాంకం (*x_పురోగతి)(శూన్య *);     /* The progress callback */
  శూన్య *p_పురోగతి_Arg;           /* Argument to the progress callback */
  గుర్తులేని n_పురోగతి_Ops;        /* Number of opcodes for progress callback */
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  పూర్ణాంకం nVTrans;                  /* Allocated size of aVTrans */
  Hash aModule;                 /* populated by sqlite3_create_module() */
  VtabCtx *pVtabCtx;            /* Context for active vtab connect/create */
  VTable **aVTrans;             /* Virtual tables with open transactions */
  VTable *pDisconnect;          /* Disconnect these in next sqlite3_prepare() */
#అయితే_ముగింపు
  Hash aFunc;                   /* Hash table of connection functions */
  Hash aCollSeq;                /* All collating sequences */
  BusyHandler busyHandler;      /* Busy callback */
  Db aDbStatic[2];              /* Static space for the 2 default backends */
  Savepoint *pSavepoint;        /* List of active savepoints */
  పూర్ణాంకం n_Analysis_హద్దు;           /* Number of index rows to ANALYZE */
  పూర్ణాంకం busyTimeout;              /* Busy handler timeout, in msec */
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  పూర్ణాంకం setlkTimeout;             /* Blocking lock timeout, in msec. -1 -> inf. */
  పూర్ణాంకం setlkFlags;               /* Flags passed to setlk_timeout() */
#అయితే_ముగింపు
  పూర్ణాంకం nSavepoint;               /* Number of non-transaction savepoints */
  పూర్ణాంకం nStatement;               /* Number of nested statement-transactions  */
  i64 nDeferredCons;            /* Net deferred constraints this transaction. */
  i64 nDeferredImmCons;         /* Net deferred immediate constraints */
  పూర్ణాంకం *pn_బైట్లు_Freed;            /* If not NULL, increment this in DbFree() */
  Db_Client_దత్తాంశం *p_Db_దత్తాంశం;        /* sqlite3_set_clientdata() content */
  u64 nSpill;                   /* TEMP content spilled to disk */
#నిర్వచితమైతే SQLITE_ENABLE_UNLOCK_NOTIFY
  /* The following variables are all protected by the STATIC_MAIN
  ** mutex, not by sqlite3.mutex. They are used by code in notify.c.
  **
  ** When X.pUnlockConnection==Y, that means that X is waiting for Y to
  ** unlock so that it can proceed.
  **
  ** When X.pBlockingConnection==Y, that means that something that X tried
  ** tried to do recently failed with an SQLITE_LOCKED error due to locks
  ** held by Y.
  */
  sqlite3 *pBlockingConnection; /* Connection that caused SQLITE_LOCKED */
  sqlite3 *pUnlockConnection;           /* Connection to watch for unlock */
  శూన్య *pUnlockArg;                     /* Argument to xUnlockNotify */
  శూన్య (*xUnlockNotify)(శూన్య **, పూర్ణాంకం);  /* Unlock notify callback */
  sqlite3 *p_తదుపరి_Blocked;        /* Next in list of all blocked connections */
#అయితే_ముగింపు
};

/*
** A macro to discover the encoding of a database.
*/
#నిర్వచించు SCHEMA_ENC(db) ((db)->aDb[0].pSchema->enc)
#నిర్వచించు ENC(db)        ((db)->enc)

/*
** A u64 constant where the lower 32 bits are all zeros.  Only the
** upper 32 bits are included in the argument.  Necessary because some
** C-compilers still do not accept LL integer literals.
*/
#నిర్వచించు HI(X)  ((u64)(X)<<32)

/*
** Possible values for the sqlite3.flags.
**
** Value constraints (enforced via assert()):
**      SQLITE_FullFSync     == PAGER_FULLFSYNC
**      SQLITE_CkptFullFSync == PAGER_CKPT_FULLFSYNC
**      SQLITE_CacheSpill    == PAGER_CACHE_SPILL
*/
#నిర్వచించు SQLITE_WriteSchema    0x00000001  /* OK to update SQLITE_SCHEMA */
#నిర్వచించు SQLITE_LegacyFileFmt  0x00000002  /* Create new databases in format 1 */
#నిర్వచించు SQLITE_నిండిన_నిలువు_పేర్లు   0x00000004  /* Show full column names on SELECT */
#నిర్వచించు SQLITE_నిండిన_FSync      0x00000008  /* Use full fsync on the backend */
#నిర్వచించు SQLITE_Ckpt_నిండిన_FSync  0x00000010  /* Use full fsync for checkpoint */
#నిర్వచించు SQLITE_CacheSpill     0x00000020  /* OK to spill pager cache */
#నిర్వచించు SQLITE_Short_నిలువు_పేర్లు  0x00000040  /* Show short columns names */
#నిర్వచించు SQLITE_TrustedSchema  0x00000080  /* Allow unsafe functions and
                                          ** vtabs in the schema definition */
#నిర్వచించు SQLITE_NullCallback   0x00000100  /* Invoke the callback once if the */
                                          /*   result set is empty */
#నిర్వచించు SQLITE_IgnoreChecks   0x00000200  /* Do not enforce check constraints */
#నిర్వచించు SQLITE_Stmt_పరిశీలన_స్థితి 0x00000400  /* Enable stmt_scanstats() counters */
#నిర్వచించు SQLITE_NoCkptOnClose  0x00000800  /* No checkpoint on close()/DETACH */
#నిర్వచించు SQLITE_తిరగేయి_క్రమం   0x00001000  /* Reverse unordered SELECTs */
#నిర్వచించు SQLITE_RecTriggers    0x00002000  /* Enable recursive triggers */
#నిర్వచించు SQLITE_Foreign_కీలకాలు    0x00004000  /* Enforce foreign key constraints  */
#నిర్వచించు SQLITE_Auto_సూచీ      0x00008000  /* Enable automatic indexes */
#నిర్వచించు SQLITE_ఎక్కించు_Extension  0x00010000  /* Enable load_extension */
#నిర్వచించు SQLITE_ఎక్కించు_Ext_Func    0x00020000  /* Enable load_extension() SQL func */
#నిర్వచించు SQLITE_EnableTrigger  0x00040000  /* True to enable triggers */
#నిర్వచించు SQLITE_DeferFKs       0x00080000  /* Defer all FK constraints */
#నిర్వచించు SQLITE_QueryOnly      0x00100000  /* Disable database changes */
#నిర్వచించు SQLITE_Cell_కొలత_Ck     0x00200000  /* Check btree cell sizes on load */
#నిర్వచించు SQLITE_Fts3Tokenizer  0x00400000  /* Enable fts3_tokenizer(2) */
#నిర్వచించు SQLITE_EnableQPSG     0x00800000  /* Query Planner Stability Guarantee*/
#నిర్వచించు SQLITE_TriggerEQP     0x01000000  /* Show trigger EXPLAIN QUERY PLAN */
#నిర్వచించు SQLITE_ResetDatabase  0x02000000  /* Reset the database */
#నిర్వచించు SQLITE_LegacyAlter    0x04000000  /* Legacy ALTER TABLE behaviour */
#నిర్వచించు SQLITE_NoSchemaError  0x08000000  /* Do not report schema parse errors*/
#నిర్వచించు SQLITE_Defensive      0x10000000  /* Input SQL is likely hostile */
#నిర్వచించు SQLITE_DqsDDL         0x20000000  /* dbl-quoted strings allowed in DDL*/
#నిర్వచించు SQLITE_DqsDML         0x40000000  /* dbl-quoted strings allowed in DML*/
#నిర్వచించు SQLITE_EnableView     0x80000000  /* Enable the use of views */
#నిర్వచించు SQLITE_లెక్క_అడ్డువరుసలు      HI(0x00001) /* Count rows changed by INSERT, */
                                          /*   DELETE, or UPDATE and return */
                                          /*   the count using a callback. */
#నిర్వచించు SQLITE_CorruptRdOnly  HI(0x00002) /* Prohibit writes due to error */
#నిర్వచించు SQLITE_ReadUncommit   HI(0x00004) /* READ UNCOMMITTED in shared-cache */
#నిర్వచించు SQLITE_FkNoAction     HI(0x00008) /* Treat all FK as NO ACTION */
#నిర్వచించు SQLITE_AttachCreate   HI(0x00010) /* ATTACH allowed to create new dbs */
#నిర్వచించు SQLITE_AttachWrite    HI(0x00020) /* ATTACH allowed to open for write */
#నిర్వచించు SQLITE_Comments       HI(0x00040) /* Enable SQL comments */

/* Flags used only if debugging */
#నిర్వచితమైతే SQLITE_DEBUG
#నిర్వచించు SQLITE_SqlTrace       HI(0x0100000) /* Debug print SQL as it executes */
#నిర్వచించు SQLITE_VdbeListing    HI(0x0200000) /* Debug listings of VDBE progs */
#నిర్వచించు SQLITE_VdbeTrace      HI(0x0400000) /* True to trace VDBE execution */
#నిర్వచించు SQLITE_VdbeAddopTrace HI(0x0800000) /* Trace sqlite3VdbeAddOp() calls */
#నిర్వచించు SQLITE_VdbeEQP        HI(0x1000000) /* Debug EXPLAIN QUERY PLAN */
#నిర్వచించు SQLITE_ParserTrace    HI(0x2000000) /* PRAGMA parser_trace=ON */
#అయితే_ముగింపు

/*
** Allowed values for sqlite3.mDbFlags
*/
#నిర్వచించు DBFLAG_SchemaChange   0x0001  /* Uncommitted Hash table changes */
#నిర్వచించు DBFLAG_PreferBuiltin  0x0002  /* Preference to built-in funcs */
#నిర్వచించు DBFLAG_Vacuum         0x0004  /* Currently in a VACUUM */
#నిర్వచించు DBFLAG_VacuumInto     0x0008  /* Currently running VACUUM INTO */
#నిర్వచించు DBFLAG_Schema_తెలిసిన_Ok  0x0010  /* Schema is known to be valid */
#నిర్వచించు DBFLAG_అంతర్గత_Func   0x0020  /* Allow use of internal functions */
#నిర్వచించు DBFLAG_EncodingFixed  0x0040  /* No longer possible to change enc. */

/*
** Bits of the sqlite3.dbOptFlags field that are used by the
** sqlite3_test_control(SQLITE_TESTCTRL_OPTIMIZATIONS,...) interface to
** selectively disable various optimizations. The numbers in each macro
** are the one-based bit position (for improved readability).
**
** Keep these values in sync with the ext/misc/optname.c extension.
*/
#నిర్వచించు SQLITE_QueryFlattener  1 /* Query flattening */
#నిర్వచించు SQLITE_కిటికీ_Func      2 /* Use xInverse for window functions */
#నిర్వచించు SQLITE_సమూహం_By_క్రమం    3 /* GROUPBY cover of ORDERBY */
#నిర్వచించు SQLITE_Join_తప్పిపోవు        4 /* Unwind join loops on a lookup miss*/
#నిర్వచించు SQLITE_DistinctOpt     5 /* DISTINCT using indexes */
#నిర్వచించు SQLITE_Cover_Idx_పరిశీలన    6 /* Covering index scans */
#నిర్వచించు SQLITE_క్రమం_By_Idx_Join  7 /* ORDER BY of joins via index */
#నిర్వచించు SQLITE_సంక్రమణ      8 /* Transitive constraints */
#నిర్వచించు SQLITE_OmitNoopJoin    9 /* Omit unused tables in joins */
#నిర్వచించు SQLITE_లెక్క_యొక్క_View    10 /* The count-of-view optimization */
#నిర్వచించు SQLITE_CursorHints    11 /* Add OP_CursorHint opcodes */
#నిర్వచించు SQLITE_Stat4          12 /* Use STAT4 data */
#నిర్వచించు SQLITE_నెట్టు_కిందికి       13 /* WHERE-clause push-down opt */
#నిర్వచించు SQLITE_SimplifyJoin   14 /* Convert LEFT JOIN to JOIN */
#నిర్వచించు SQLITE_Skip_పరిశీలన       15 /* Skip-scans */
#నిర్వచించు SQLITE_PropagateConst 16 /* The constant propagation opt */
#నిర్వచించు SQLITE_MinMaxOpt      17 /* The min/max optimization */
#నిర్వచించు SQLITE_Seek_పరిశీలన       18 /* The OP_SeekScan optimization */
#నిర్వచించు SQLITE_Omit_క్రమం_By    19 /* Omit pointless ORDER BY */
#నిర్వచించు SQLITE_BloomFilter    20 /* Use a Bloom filters */
                          /*  21 -- Available for reuse */
#నిర్వచించు SQLITE_సమతుల్య_విలీనం  22 /* Balance multi-way merges */
#నిర్వచించు SQLITE_ReleaseReg     23 /* Use OP_ReleaseReg for testing */
#నిర్వచించు SQLITE_FlttnUnionAll  24 /* Disable the UNION ALL flattener */
#నిర్వచించు SQLITE_సూచీకృత_సమాసం    25 /* Pull exprs from index when able */
#నిర్వచించు SQLITE_Coroutines     26 /* Co-routines for subqueries */
#నిర్వచించు SQLITE_Null_వాడని_నిలువుల_సంఖ్య 27 /* NULL unused columns in subqueries */
#నిర్వచించు SQLITE_OnePass        28 /* Single-pass DELETE and UPDATE */
#నిర్వచించు SQLITE_క్రమం_By_Subq    29 /* ORDER BY in subquery helps outer */
#నిర్వచించు SQLITE_నక్షత్రం_Query      30 /* Heurists for star queries */
#నిర్వచించు SQLITE_Exists_గమ్యం_Join   31 /* The EXISTS-to-JOIN optimization */
#నిర్వచించు SQLITE_Union_హద్దు     32 /* Optimizations for UNION + LIMIT */

/*
** Macros for testing whether or not optimizations are enabled or disabled.
*/
#నిర్వచించు OptimizationDisabled(D,M)  (((D)->dbOptFlags&(1ul<<((M)-1)))!=0)
#నిర్వచించు OptimizationEnabled(D,M)   (((D)->dbOptFlags&(1ul<<((M)-1)))==0)

/*
** Return true if it OK to factor constant expressions into the initialization
** code. The argument is a Parse object for the code generator.
*/
#నిర్వచించు Const_గుణకం_Ok(P) ((P)->ok_Const_గుణకం)

/* Possible values for the sqlite3.eOpenState field.
** The numbers are randomly selected such that a minimum of three bits must
** change to convert any number to another or to zero
*/
#నిర్వచించు SQLITE_STATE_OPEN     0x76  /* Database is open */
#నిర్వచించు SQLITE_STATE_CLOSED   0xce  /* Database is closed */
#నిర్వచించు SQLITE_STATE_SICK     0xba  /* Error and awaiting close */
#నిర్వచించు SQLITE_STATE_BUSY     0x6d  /* Database currently in use */
#నిర్వచించు SQLITE_STATE_ERROR    0xd5  /* An SQLITE_MISUSE error occurred */
#నిర్వచించు SQLITE_STATE_ZOMBIE   0xa7  /* Close with last statement close */

/*
** Each SQL function is defined by an instance of the following
** structure.  For global built-in functions (ex: substr(), max(), count())
** a pointer to this structure is held in the sqlite3BuiltinFunctions object.
** For per-connection application-defined functions, a pointer to this
** structure is held in the db->aHash hash table.
**
** The u.pHash field is used by the global built-ins.  The u.pDestructor
** field is used by per-connection app-def functions.
*/
నిర్మాణం FuncDef {
  i16 nArg;            /* Number of arguments.  -1 means unlimited */
  u32 funcFlags;       /* Some combination of SQLITE_FUNC_* */
  శూన్య *p_User_దత్తాంశం;     /* User data parameter */
  FuncDef *p_తదుపరి;      /* Next function with same name */
  శూన్య (*xSFunc)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**); /* func or agg-step */
  శూన్య (*xFinalize)(sqlite3_context*);                  /* Agg finalizer */
  శూన్య (*x_విలువ)(sqlite3_context*);                     /* Current agg value */
  శూన్య (*xInverse)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ**); /* inverse agg-step */
  స్థిరరాశి అక్షరం *zName;   /* SQL name of the function. */
  సంఘం {
    FuncDef *pHash;      /* Next with a different name but the same hash */
    FuncDestructor *pDestructor;   /* Reference counted destructor function */
  } u; /* pHash if SQLITE_FUNC_BUILTIN, pDestructor otherwise */
};

/*
** This structure encapsulates a user-function destructor callback (as
** configured using create_function_v2()) and a reference counter. When
** create_function_v2() is called to create a function with a destructor,
** a single object of this type is allocated. FuncDestructor.nRef is set to
** the number of FuncDef objects created (either 1 or 3, depending on whether
** or not the specified encoding is SQLITE_ANY). The FuncDef.pDestructor
** member of each of the new FuncDef objects is set to point to the allocated
** FuncDestructor.
**
** Thereafter, when one of the FuncDef objects is deleted, the reference
** count on this object is decremented. When it reaches 0, the destructor
** is invoked and the FuncDestructor structure freed.
*/
నిర్మాణం FuncDestructor {
  పూర్ణాంకం nRef;
  శూన్య (*xDestroy)(శూన్య *);
  శూన్య *p_User_దత్తాంశం;
};

/*
** Possible values for FuncDef.flags.  Note that the _LENGTH and _TYPEOF
** values must correspond to OPFLAG_LENGTHARG and OPFLAG_TYPEOFARG.  And
** SQLITE_FUNC_CONSTANT must be the same as SQLITE_DETERMINISTIC.  There
** are assert() statements in the code to verify this.
**
** Value constraints (enforced via assert()):
**     SQLITE_FUNC_MINMAX      ==  NC_MinMaxAgg      == SF_MinMaxAgg
**     SQLITE_FUNC_ANYORDER    ==  NC_OrderAgg       == SF_OrderByReqd
**     SQLITE_FUNC_LENGTH      ==  OPFLAG_LENGTHARG
**     SQLITE_FUNC_TYPEOF      ==  OPFLAG_TYPEOFARG
**     SQLITE_FUNC_BYTELEN     ==  OPFLAG_BYTELENARG
**     SQLITE_FUNC_CONSTANT    ==  SQLITE_DETERMINISTIC from the API
**     SQLITE_FUNC_DIRECT      ==  SQLITE_DIRECTONLY from the API
**     SQLITE_FUNC_UNSAFE      ==  SQLITE_INNOCUOUS  -- opposite meanings!!!
**     SQLITE_FUNC_ENCMASK   depends on SQLITE_UTF* macros in the API
**
** Note that even though SQLITE_FUNC_UNSAFE and SQLITE_INNOCUOUS have the
** same bit value, their meanings are inverted.  SQLITE_FUNC_UNSAFE is
** used internally and if set means that the function has side effects.
** SQLITE_INNOCUOUS is used by application code and means "not unsafe".
** See multiple instances of tag-20230109-1.
*/
#నిర్వచించు SQLITE_FUNC_ENCMASK  0x0003 /* SQLITE_UTF8, SQLITE_UTF16BE or UTF16LE */
#నిర్వచించు SQLITE_FUNC_LIKE     0x0004 /* Candidate for the LIKE optimization */
#నిర్వచించు SQLITE_FUNC_CASE     0x0008 /* Case-sensitive LIKE-type function */
#నిర్వచించు SQLITE_FUNC_EPHEM    0x0010 /* Ephemeral.  Delete with VDBE */
#నిర్వచించు SQLITE_FUNC_NEEDCOLL 0x0020 /* sqlite3GetFuncCollSeq() might be called*/
#నిర్వచించు SQLITE_FUNC_నిడివి   0x0040 /* Built-in length() function */
#నిర్వచించు SQLITE_FUNC_TYPEOF   0x0080 /* Built-in typeof() function */
#నిర్వచించు SQLITE_FUNC_BYTELEN  0x00c0 /* Built-in octet_length() function */
#నిర్వచించు SQLITE_FUNC_లెక్క    0x0100 /* Built-in count(*) aggregate */
/*                           0x0200 -- available for reuse */
#నిర్వచించు SQLITE_FUNC_UNLIKELY 0x0400 /* Built-in unlikely() function */
#నిర్వచించు SQLITE_FUNC_CONSTANT 0x0800 /* Constant inputs give a constant output */
#నిర్వచించు SQLITE_FUNC_MINMAX   0x1000 /* True for min() and max() aggregates */
#నిర్వచించు SQLITE_FUNC_SLOCHNG  0x2000 /* "Slow Change". Value constant during a
                                    ** single query - might change over time */
#నిర్వచించు SQLITE_FUNC_TEST     0x4000 /* Built-in testing functions */
#నిర్వచించు SQLITE_FUNC_RUNONLY  0x8000 /* Cannot be used by valueFromFunction */
#నిర్వచించు SQLITE_FUNC_కిటికీ   0x00010000 /* Built-in window-only function */
#నిర్వచించు SQLITE_FUNC_అంతర్గత 0x00040000 /* For use by NestedParse() only */
#నిర్వచించు SQLITE_FUNC_ప్రత్యక్ష   0x00080000 /* Not for use in TRIGGERs or VIEWs */
/* SQLITE_SUBTYPE            0x00100000 // Consumer of subtypes */
#నిర్వచించు SQLITE_FUNC_UNSAFE   0x00200000 /* Function has side effects */
#నిర్వచించు SQLITE_FUNC_INLINE   0x00400000 /* Functions implemented in-line */
#నిర్వచించు SQLITE_FUNC_BUILTIN  0x00800000 /* This is a built-in function */
/*  SQLITE_RESULT_SUBTYPE    0x01000000 // Generator of subtypes */
#నిర్వచించు SQLITE_FUNC_ANYORDER 0x08000000 /* count/min/max aggregate */

/* Identifier numbers for each in-line function */
#నిర్వచించు INLINEFUNC_coalesce             0
#నిర్వచించు INLINEFUNC_సూచిస్తుంది_nonnull_అడ్డువరుస  1
#నిర్వచించు INLINEFUNC_సమాసం_సూచిస్తుంది_సమాసం    2
#నిర్వచించు INLINEFUNC_సమాసం_compare         3
#నిర్వచించు INLINEFUNC_affinity             4
#నిర్వచించు INLINEFUNC_iif                  5
#నిర్వచించు INLINEFUNC_sqlite_స్థానభ్రంశం        6
#నిర్వచించు INLINEFUNC_unlikely            99  /* Default case */

/*
** The following three macros, FUNCTION(), LIKEFUNC() and AGGREGATE() are
** used to create the initializers for the FuncDef structures.
**
**   FUNCTION(zName, nArg, iArg, bNC, xFunc)
**     Used to create a scalar function definition of a function zName
**     implemented by C function xFunc that accepts nArg arguments. The
**     value passed as iArg is cast to a (void*) and made available
**     as the user-data (sqlite3_user_data()) for the function. If
**     argument bNC is true, then the SQLITE_FUNC_NEEDCOLL flag is set.
**
**   VFUNCTION(zName, nArg, iArg, bNC, xFunc)
**     Like FUNCTION except it omits the SQLITE_FUNC_CONSTANT flag.
**
**   SFUNCTION(zName, nArg, iArg, bNC, xFunc)
**     Like FUNCTION except it omits the SQLITE_FUNC_CONSTANT flag and
**     adds the SQLITE_DIRECTONLY flag.
**
**   INLINE_FUNC(zName, nArg, iFuncId, mFlags)
**     zName is the name of a function that is implemented by in-line
**     byte code rather than by the usual callbacks. The iFuncId
**     parameter determines the function id.  The mFlags parameter is
**     optional SQLITE_FUNC_ flags for this function.
**
**   TEST_FUNC(zName, nArg, iFuncId, mFlags)
**     zName is the name of a test-only function implemented by in-line
**     byte code rather than by the usual callbacks. The iFuncId
**     parameter determines the function id.  The mFlags parameter is
**     optional SQLITE_FUNC_ flags for this function.
**
**   DFUNCTION(zName, nArg, iArg, bNC, xFunc)
**     Like FUNCTION except it omits the SQLITE_FUNC_CONSTANT flag and
**     adds the SQLITE_FUNC_SLOCHNG flag.  Used for date & time functions
**     and functions like sqlite_version() that can change, but not during
**     a single query.  The iArg is ignored.  The user-data is always set
**     to a NULL pointer.  The bNC parameter is not used.
**
**   MFUNCTION(zName, nArg, xPtr, xFunc)
**     For math-library functions.  xPtr is an arbitrary pointer.
**
**   PURE_DATE(zName, nArg, iArg, bNC, xFunc)
**     Used for "pure" date/time functions, this macro is like DFUNCTION
**     except that it does set the SQLITE_FUNC_CONSTANT flags.  iArg is
**     ignored and the user-data for these functions is set to an
**     arbitrary non-NULL pointer.  The bNC parameter is not used.
**
**   AGGREGATE(zName, nArg, iArg, bNC, xStep, xFinal)
**     Used to create an aggregate function definition implemented by
**     the C functions xStep and xFinal. The first four parameters
**     are interpreted in the same way as the first 4 parameters to
**     FUNCTION().
**
**   WAGGREGATE(zName, nArg, iArg, xStep, xFinal, xValue, xInverse)
**     Used to create an aggregate function definition implemented by
**     the C functions xStep and xFinal. The first four parameters
**     are interpreted in the same way as the first 4 parameters to
**     FUNCTION().
**
**   LIKEFUNC(zName, nArg, pArg, flags)
**     Used to create a scalar function definition of a function zName
**     that accepts nArg arguments and is implemented by a call to C
**     function likeFunc. Argument pArg is cast to a (void *) and made
**     available as the function user-data (sqlite3_user_data()). The
**     FuncDef.flags variable is set to the value passed as the flags
**     parameter.
*/
#నిర్వచించు FUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_CONSTANT|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_గమ్యం_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#నిర్వచించు VFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_గమ్యం_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#నిర్వచించు SFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|SQLITE_DIRECTONLY|SQLITE_FUNC_UNSAFE, \
   SQLITE_INT_గమ్యం_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#నిర్వచించు MFUNCTION(zName, nArg, xPtr, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_CONSTANT|SQLITE_UTF8, \
   xPtr, 0, xFunc, 0, 0, 0, #zName, {0} }
#నిర్వచించు JFUNCTION(zName, nArg, bUseCache, bWS, bRS, bJsonB, iArg, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_DETERMINISTIC|SQLITE_FUNC_CONSTANT|\
   SQLITE_UTF8|((bUseCache)*SQLITE_FUNC_RUNONLY)|\
   ((bRS)*SQLITE_SUBTYPE)|((bWS)*SQLITE_ఫలితం_SUBTYPE), \
   SQLITE_INT_గమ్యం_PTR(iArg|((bJsonB)*JSON_BLOB)),0,xFunc,0, 0, 0, #zName, {0} }
#నిర్వచించు INLINE_FUNC(zName, nArg, iArg, mFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_UTF8|SQLITE_FUNC_INLINE|SQLITE_FUNC_CONSTANT|(mFlags), \
   SQLITE_INT_గమ్యం_PTR(iArg), 0, noopFunc, 0, 0, 0, #zName, {0} }
#నిర్వచించు TEST_FUNC(zName, nArg, iArg, mFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
         SQLITE_UTF8|SQLITE_FUNC_అంతర్గత|SQLITE_FUNC_TEST| \
         SQLITE_FUNC_INLINE|SQLITE_FUNC_CONSTANT|(mFlags), \
   SQLITE_INT_గమ్యం_PTR(iArg), 0, noopFunc, 0, 0, 0, #zName, {0} }
#నిర్వచించు DFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_SLOCHNG|SQLITE_UTF8, \
   0, 0, xFunc, 0, 0, 0, #zName, {0} }
#నిర్వచించు శుద్ధ_DATE(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
         SQLITE_FUNC_SLOCHNG|SQLITE_UTF8|SQLITE_FUNC_CONSTANT, \
   (శూన్య*)&sqlite3Config, 0, xFunc, 0, 0, 0, #zName, {0} }
#నిర్వచించు FUNCTION2(zName, nArg, iArg, bNC, xFunc, extraFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_CONSTANT|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL)|extraFlags,\
   SQLITE_INT_గమ్యం_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#నిర్వచించు STR_FUNCTION(zName, nArg, pArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_SLOCHNG|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   pArg, 0, xFunc, 0, 0, 0, #zName, {0} }
#నిర్వచించు LIKEFUNC(zName, nArg, arg, flags) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_CONSTANT|SQLITE_UTF8|flags, \
   (శూన్య *)arg, 0, likeFunc, 0, 0, 0, #zName, {0} }
#నిర్వచించు WAGGREGATE(zName, nArg, arg, nc, x_అడుగు, xFinal, x_విలువ, xInverse, f) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|(nc*SQLITE_FUNC_NEEDCOLL)|f, \
   SQLITE_INT_గమ్యం_PTR(arg), 0, x_అడుగు,xFinal,x_విలువ,xInverse,#zName, {0}}
#నిర్వచించు అంతర్గత_FUNCTION(zName, nArg, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_అంతర్గత|SQLITE_UTF8|SQLITE_FUNC_CONSTANT, \
   0, 0, xFunc, 0, 0, 0, #zName, {0} }


/*
** All current savepoints are stored in a linked list starting at
** sqlite3.pSavepoint. The first element in the list is the most recently
** opened savepoint. Savepoints are added to the list by the vdbe
** OP_Savepoint instruction.
*/
నిర్మాణం Savepoint {
  అక్షరం *zName;                        /* Savepoint name (nul-terminated) */
  i64 nDeferredCons;                  /* Number of deferred fk violations */
  i64 nDeferredImmCons;               /* Number of deferred imm fk. */
  Savepoint *p_తదుపరి;                   /* Parent savepoint (if any) */
};

/*
** The following are used as the second parameter to sqlite3Savepoint(),
** and as the P1 argument to the OP_Savepoint instruction.
*/
#నిర్వచించు SAVEPOINT_BEGIN      0
#నిర్వచించు SAVEPOINT_RELEASE    1
#నిర్వచించు SAVEPOINT_ROLLBACK   2


/*
** Each SQLite module (virtual table definition) is defined by an
** instance of the following structure, stored in the sqlite3.aModule
** hash table.
*/
నిర్మాణం Module {
  స్థిరరాశి sqlite3_module *pModule;       /* Callback pointers */
  స్థిరరాశి అక్షరం *zName;                   /* Name passed to create_module() */
  పూర్ణాంకం nRefModule;                      /* Number of pointers to this object */
  శూన్య *pAux;                          /* pAux passed to create_module() */
  శూన్య (*xDestroy)(శూన్య *);            /* Module destructor function */
  పట్టిక *pEpoTab;                      /* Eponymous table for this module */
};

/*
** Information about each column of an SQL table is held in an instance
** of the Column structure, in the Table.aCol[] array.
**
** Definitions:
**
**   "table column index"     This is the index of the column in the
**                            Table.aCol[] array, and also the index of
**                            the column in the original CREATE TABLE stmt.
**
**   "storage column index"   This is the index of the column in the
**                            record BLOB generated by the OP_MakeRecord
**                            opcode.  The storage column index is less than
**                            or equal to the table column index.  It is
**                            equal if and only if there are no VIRTUAL
**                            columns to the left.
**
** Notes on zCnName:
** The zCnName field stores the name of the column, the datatype of the
** column, and the collating sequence for the column, in that order, all in
** a single allocation.  Each string is 0x00 terminated.  The datatype
** is only included if the COLFLAG_HASTYPE bit of colFlags is set and the
** collating sequence name is only included if the COLFLAG_HASCOLL bit is
** set.
*/
నిర్మాణం స్తంభం {
  అక్షరం *zCnName;        /* Name of this column */
  గుర్తులేని notNull :4;  /* An OE_ code for handling a NOT NULL constraint */
  గుర్తులేని eCType :4;   /* One of the standard types */
  అక్షరం affinity;        /* One of the SQLITE_AFF_... values */
  u8 szEst;             /* Est size of value in this column. sizeof(INT)==1 */
  u8 hName;             /* Column name hash for faster lookup */
  u16 iDflt;            /* 1-based index of DEFAULT.  0 means "none" */
  u16 నిలువు_Flags;         /* Boolean properties.  See COLFLAG_ defines below */
};

/* Allowed values for Column.eCType.
**
** Values must match entries in the global constant arrays
** sqlite3StdTypeLen[] and sqlite3StdType[].  Each value is one more
** than the offset into these arrays for the corresponding name.
** Adjust the SQLITE_N_STDTYPE value if adding or removing entries.
*/
#నిర్వచించు COLTYPE_CUSTOM      0   /* Type appended to zName */
#నిర్వచించు COLTYPE_ANY         1
#నిర్వచించు COLTYPE_BLOB        2
#నిర్వచించు COLTYPE_INT         3
#నిర్వచించు COLTYPE_INTEGER     4
#నిర్వచించు COLTYPE_REAL        5
#నిర్వచించు COLTYPE_పాఠ్యం        6
#నిర్వచించు SQLITE_N_STDTYPE    6  /* Number of standard types */

/* Allowed values for Column.colFlags.
**
** Constraints:
**         TF_HasVirtual == COLFLAG_VIRTUAL
**         TF_HasStored  == COLFLAG_STORED
**         TF_HasHidden  == COLFLAG_HIDDEN
*/
#నిర్వచించు COLFLAG_PRIMKEY   0x0001   /* Column is part of the primary key */
#నిర్వచించు COLFLAG_HIDDEN    0x0002   /* A hidden column in a virtual table */
#నిర్వచించు COLFLAG_HASTYPE   0x0004   /* Type name follows column name */
#నిర్వచించు COLFLAG_UNIQUE    0x0008   /* Column def contains "UNIQUE" or "PK" */
#నిర్వచించు COLFLAG_SORTERREF 0x0010   /* Use sorter-refs with this column */
#నిర్వచించు COLFLAG_VIRTUAL   0x0020   /* GENERATED ALWAYS AS ... VIRTUAL */
#నిర్వచించు COLFLAG_STORED    0x0040   /* GENERATED ALWAYS AS ... STORED */
#నిర్వచించు COLFLAG_NOTAVAIL  0x0080   /* STORED column not yet calculated */
#నిర్వచించు COLFLAG_BUSY      0x0100   /* Blocks recursion on GENERATED columns */
#నిర్వచించు COLFLAG_HASCOLL   0x0200   /* Has collating sequence name in zCnName */
#నిర్వచించు COLFLAG_NOEXPAND  0x0400   /* Omit this column when expanding "*" */
#నిర్వచించు COLFLAG_GENERATED 0x0060   /* Combo: _STORED, _VIRTUAL */
#నిర్వచించు COLFLAG_NOINSERT  0x0062   /* Combo: _HIDDEN, _STORED, _VIRTUAL */

/*
** A "Collating Sequence" is defined by an instance of the following
** structure. Conceptually, a collating sequence consists of a name and
** a comparison routine that defines the order of that sequence.
**
** If CollSeq.xCmp is NULL, it means that the
** collating sequence is undefined.  Indices built on an undefined
** collating sequence may not be read or written.
*/
నిర్మాణం CollSeq {
  అక్షరం *zName;          /* Name of the collating sequence, UTF-8 encoded */
  u8 enc;               /* Text encoding handled by xCmp() */
  శూన్య *pUser;          /* First argument to xCmp() */
  పూర్ణాంకం (*xCmp)(శూన్య*,పూర్ణాంకం, స్థిరరాశి శూన్య*, పూర్ణాంకం, స్థిరరాశి శూన్య*);
  శూన్య (*xDel)(శూన్య*);  /* Destructor for pUser */
};

/*
** A sort order can be either ASC or DESC.
*/
#నిర్వచించు SQLITE_SO_ASC       0  /* Sort in ascending order */
#నిర్వచించు SQLITE_SO_DESC      1  /* Sort in ascending order */
#నిర్వచించు SQLITE_SO_UNDEFINED -1 /* No sort order specified */

/*
** Column affinity types.
**
** These used to have mnemonic name like 'i' for SQLITE_AFF_INTEGER and
** 't' for SQLITE_AFF_TEXT.  But we can save a little space and improve
** the speed a little by numbering the values consecutively.
**
** But rather than start with 0 or 1, we begin with 'A'.  That way,
** when multiple affinity types are concatenated into a string and
** used as the P4 operand, they will be more readable.
**
** Note also that the numeric types are grouped together so that testing
** for a numeric type is a single comparison.  And the BLOB type is first.
*/
#నిర్వచించు SQLITE_AFF_NONE     0x40  /* '@' */
#నిర్వచించు SQLITE_AFF_BLOB     0x41  /* 'A' */
#నిర్వచించు SQLITE_AFF_పాఠ్యం     0x42  /* 'B' */
#నిర్వచించు SQLITE_AFF_NUMERIC  0x43  /* 'C' */
#నిర్వచించు SQLITE_AFF_INTEGER  0x44  /* 'D' */
#నిర్వచించు SQLITE_AFF_REAL     0x45  /* 'E' */
#నిర్వచించు SQLITE_AFF_FLEXNUM  0x46  /* 'F' */
#నిర్వచించు SQLITE_AFF_DEFER    0x58  /* 'X'  - defer computation until later */

#నిర్వచించు sqlite3IsNumericAffinity(X)  ((X)>=SQLITE_AFF_NUMERIC)

/*
** The SQLITE_AFF_MASK values masks off the significant bits of an
** affinity value.
*/
#నిర్వచించు SQLITE_AFF_ముసుగు     0x47

/*
** Additional bit values that can be ORed with an affinity without
** changing the affinity.
**
** The SQLITE_NOTNULL flag is a combination of NULLEQ and JUMPIFNULL.
** It causes an assert() to fire if either operand to a comparison
** operator is NULL.  It is added to certain comparison operators to
** prove that the operands are always NOT NULL.
*/
#నిర్వచించు SQLITE_JUMPIFNULL   0x10  /* jumps if either operand is NULL */
#నిర్వచించు SQLITE_NULLEQ       0x80  /* NULL=NULL */
#నిర్వచించు SQLITE_NOTNULL      0x90  /* Assert that operands are never NULL */

/*
** An object of this type is created for each virtual table present in
** the database schema.
**
** If the database schema is shared, then there is one instance of this
** structure for each database connection (sqlite3*) that uses the shared
** schema. This is because each database connection requires its own unique
** instance of the sqlite3_vtab* handle used to access the virtual table
** implementation. sqlite3_vtab* handles can not be shared between
** database connections, even when the rest of the in-memory database
** schema is shared, as the implementation often stores the database
** connection handle passed to it via the xConnect() or xCreate() method
** during initialization internally. This database connection handle may
** then be used by the virtual table implementation to access real tables
** within the database. So that they appear as part of the callers
** transaction, these accesses need to be made via the same database
** connection as that used to execute SQL operations on the virtual table.
**
** All VTable objects that correspond to a single table in a shared
** database schema are initially stored in a linked-list pointed to by
** the Table.pVTable member variable of the corresponding Table object.
** When an sqlite3_prepare() operation is required to access the virtual
** table, it searches the list for the VTable that corresponds to the
** database connection doing the preparing so as to use the correct
** sqlite3_vtab* handle in the compiled query.
**
** When an in-memory Table object is deleted (for example when the
** schema is being reloaded for some reason), the VTable objects are not
** deleted and the sqlite3_vtab* handles are not xDisconnect()ed
** immediately. Instead, they are moved from the Table.pVTable list to
** another linked list headed by the sqlite3.pDisconnect member of the
** corresponding sqlite3 structure. They are then deleted/xDisconnected
** next time a statement is prepared using said sqlite3*. This is done
** to avoid deadlock issues involving multiple sqlite3.mutex mutexes.
** Refer to comments above function sqlite3VtabUnlockList() for an
** explanation as to why it is safe to add an entry to an sqlite3.pDisconnect
** list without holding the corresponding sqlite3.mutex mutex.
**
** The memory for objects of this type is always allocated by
** sqlite3DbMalloc(), using the connection handle stored in VTable.db as
** the first argument.
*/
నిర్మాణం VTable {
  sqlite3 *db;              /* Database connection associated with this table */
  Module *pMod;             /* Pointer to module implementation */
  sqlite3_vtab *pVtab;      /* Pointer to vtab instance */
  పూర్ణాంకం nRef;                 /* Number of pointers to this structure */
  u8 bConstraint;           /* True if constraints are supported */
  u8 bAllSchemas;           /* True if might use any attached schema */
  u8 eVtabRisk;             /* Riskiness of allowing hacker access */
  పూర్ణాంకం iSavepoint;           /* Depth of the SAVEPOINT stack */
  VTable *p_తదుపరి;            /* Next in linked list (see above) */
};

/* Allowed values for VTable.eVtabRisk
*/
#నిర్వచించు SQLITE_VTABRISK_దిగువ          0
#నిర్వచించు SQLITE_VTABRISK_సాధారణ       1
#నిర్వచించు SQLITE_VTABRISK_ఎగువ         2

/*
** The schema for each SQL table, virtual table, and view is represented
** in memory by an instance of the following structure.
*/
నిర్మాణం పట్టిక {
  అక్షరం *zName;         /* Name of the table or view */
  స్తంభం *a_నిలువు;        /* Information about each column */
  సూచీ *p_సూచీ;       /* List of SQL indexes on this table. */
  అక్షరం *z_నిలువు_Aff;       /* String defining the affinity of each column */
  సమాసం_List *pCheck;    /* All CHECK constraints */
                       /*   ... also used as column name list in a VIEW */
  Pgno tnum;           /* Root BTree page for this table */
  u32 nTabRef;         /* Number of pointers to this Table */
  u32 tabFlags;        /* Mask of TF_* values */
  i16 iPKey;           /* If not negative, use aCol[iPKey] as the rowid */
  i16 n_నిలువు;            /* Number of columns in this table */
  i16 nNVCol;          /* Number of columns that are not VIRTUAL */
  LogEst n_అడ్డువరుస_Log_Est;   /* Estimated rows in table - from sqlite_stat1 table */
  LogEst sz_Tab_అడ్డువరుస;     /* Estimated size of each table row in bytes */
#నిర్వచితమైతే SQLITE_ENABLE_COSTMULT
  LogEst ఖర్చు_Mult;     /* Cost multiplier for using this table */
#అయితే_ముగింపు
  u8 కీలకం_Conf;          /* What to do in case of uniqueness conflict on iPKey */
  u8 eTabType;         /* 0: normal, 1: virtual, 2: view */
  సంఘం {
    నిర్మాణం {             /* Used by ordinary tables: */
      పూర్ణాంకం కూడు_నిలువు_స్థానభ్రంశం;    /* Offset in CREATE TABLE stmt to add a new column */
      FKey *pFKey;         /* Linked list of all foreign keys in this table */
      సమాసం_List *pDfltList; /* DEFAULT clauses on various columns.
                           ** Or the AS clause for generated columns. */
    } tab;
    నిర్మాణం {             /* Used by views: */
      Select *pSelect;     /* View definition */
    } view;
    నిర్మాణం {             /* Used by virtual tables only: */
      పూర్ణాంకం nArg;            /* Number of arguments to the module */
      అక్షరం **azArg;        /* 0: module 1: schema 2: vtab name 3...: args */
      VTable *p;           /* List of VTable objects. */
    } vtab;
  } u;
  Trigger *pTrigger;   /* List of triggers on this object */
  Schema *pSchema;     /* Schema that contains this table */
  u8 aHx[16];          /* Column aHt[K%sizeof(aHt)] might have hash K */
};

/*
** Allowed values for Table.tabFlags.
**
** TF_OOOHidden applies to tables or view that have hidden columns that are
** followed by non-hidden columns.  Example:  "CREATE VIRTUAL TABLE x USING
** vtab1(a HIDDEN, b);".  Since "b" is a non-hidden column but "a" is hidden,
** the TF_OOOHidden attribute would apply in this case.  Such tables require
** special handling during INSERT processing. The "OOO" means "Out Of Order".
**
** Constraints:
**
**         TF_HasVirtual == COLFLAG_VIRTUAL
**         TF_HasStored  == COLFLAG_STORED
**         TF_HasHidden  == COLFLAG_HIDDEN
*/
#నిర్వచించు TF_Readonly       0x00000001 /* Read-only system table */
#నిర్వచించు TF_HasHidden      0x00000002 /* Has one or more hidden columns */
#నిర్వచించు TF_Has_Primary_కీలకం  0x00000004 /* Table has a primary key */
#నిర్వచించు TF_Autoincrement  0x00000008 /* Integer primary key is autoincrement */
#నిర్వచించు TF_HasStat1       0x00000010 /* nRowLogEst set from sqlite_stat1 */
#నిర్వచించు TF_HasVirtual     0x00000020 /* Has one or more VIRTUAL columns */
#నిర్వచించు TF_HasStored      0x00000040 /* Has one or more STORED columns */
#నిర్వచించు TF_HasGenerated   0x00000060 /* Combo: HasVirtual + HasStored */
#నిర్వచించు TF_WithoutRowid   0x00000080 /* No rowid.  PRIMARY KEY is the key */
#నిర్వచించు TF_MaybeReanalyze 0x00000100 /* Maybe run ANALYZE on this table */
#నిర్వచించు TF_NoVisibleRowid 0x00000200 /* No user-visible "rowid" column */
#నిర్వచించు TF_OOOHidden      0x00000400 /* Out-of-Order hidden columns */
#నిర్వచించు TF_HasNotNull     0x00000800 /* Contains NOT NULL constraints */
#నిర్వచించు TF_Shadow         0x00001000 /* True for a shadow table */
#నిర్వచించు TF_HasStat4       0x00002000 /* STAT4 info available for this table */
#నిర్వచించు TF_Ephemeral      0x00004000 /* An ephemeral table */
#నిర్వచించు TF_Eponymous      0x00008000 /* An eponymous virtual table */
#నిర్వచించు TF_Strict         0x00010000 /* STRICT mode */
#నిర్వచించు TF_Imposter       0x00020000 /* An imposter table */

/*
** Allowed values for Table.eTabType
*/
#నిర్వచించు TABTYP_NORM      0     /* Ordinary table */
#నిర్వచించు TABTYP_VTAB      1     /* Virtual table */
#నిర్వచించు TABTYP_VIEW      2     /* A view */

#నిర్వచించు IsView(X)           ((X)->eTabType==TABTYP_VIEW)
#నిర్వచించు Is_Ordinary_పట్టిక(X)  ((X)->eTabType==TABTYP_NORM)

/*
** Test to see whether or not a table is a virtual table.  This is
** done as a macro so that it will be optimized out when virtual
** table support is omitted from the build.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
#  define IsVirtual(X)      ((X)->eTabType==TABTYP_VTAB)
#  define సమాసం_Is_Vtab(X)  \
   ((X)->op==TK_స్తంభం && (X)->y.pTab->eTabType==TABTYP_VTAB)
#లేకపోతే
#  define IsVirtual(X)      0
#  define సమాసం_Is_Vtab(X)     0
#అయితే_ముగింపు

/*
** Macros to determine if a column is hidden.  IsOrdinaryHiddenColumn()
** only works for non-virtual tables (ordinary tables and views) and is
** always false unless SQLITE_ENABLE_HIDDEN_COLUMNS is defined.  The
** IsHiddenColumn() macro is general purpose.
*/
#ఒకవేళ defined(SQLITE_ENABLE_HIDDEN_COLUMNS)
#  define Is_Hidden_స్తంభం(X)         (((X)->నిలువు_Flags & COLFLAG_HIDDEN)!=0)
#  define Is_Ordinary_Hidden_స్తంభం(X) (((X)->నిలువు_Flags & COLFLAG_HIDDEN)!=0)
#లేదావేళ !defined(SQLITE_OMIT_VIRTUALTABLE)
#  define Is_Hidden_స్తంభం(X)         (((X)->నిలువు_Flags & COLFLAG_HIDDEN)!=0)
#  define Is_Ordinary_Hidden_స్తంభం(X) 0
#లేకపోతే
#  define Is_Hidden_స్తంభం(X)         0
#  define Is_Ordinary_Hidden_స్తంభం(X) 0
#అయితే_ముగింపు


/* Does the table have a rowid */
#నిర్వచించు HasRowid(X)     (((X)->tabFlags & TF_WithoutRowid)==0)
#నిర్వచించు VisibleRowid(X) (((X)->tabFlags & TF_NoVisibleRowid)==0)

/* Macro is true if the SQLITE_ALLOW_ROWID_IN_VIEW (mis-)feature is
** available.  By default, this macro is false
*/
#నిర్వచితం_కాకపోతే SQLITE_ALLOW_ROWID_లోపల_VIEW
# define ViewCanHaveRowid     0
#లేకపోతే
# define ViewCanHaveRowid     (sqlite3Config.mNoVisibleRowid==0)
#అయితే_ముగింపు

/*
** Each foreign key constraint is an instance of the following structure.
**
** A foreign key is associated with two tables.  The "from" table is
** the table that contains the REFERENCES clause that creates the foreign
** key.  The "to" table is the table that is named in the REFERENCES clause.
** Consider this example:
**
**     CREATE TABLE ex1(
**       a INTEGER PRIMARY KEY,
**       b INTEGER CONSTRAINT fk1 REFERENCES ex2(x)
**     );
**
** For foreign key "fk1", the from-table is "ex1" and the to-table is "ex2".
** Equivalent names:
**
**     from-table == child-table
**       to-table == parent-table
**
** Each REFERENCES clause generates an instance of the following structure
** which is attached to the from-table.  The to-table need not exist when
** the from-table is created.  The existence of the to-table is not checked.
**
** The list of all parents for child Table X is held at X.pFKey.
**
** A list of all children for a table named Z (which might not even exist)
** is held in Schema.fkeyHash with a hash key of Z.
*/
నిర్మాణం FKey {
  పట్టిక *pFrom;     /* Table containing the REFERENCES clause (aka: Child) */
  FKey *p_తదుపరి_From;  /* Next FKey with the same in pFrom. Next parent of pFrom */
  అక్షరం *z_గమ్యం;        /* Name of table that the key points to (aka: Parent) */
  FKey *p_తదుపరి_గమ్యం;    /* Next with the same zTo. Next child of zTo. */
  FKey *p_Prev_గమ్యం;    /* Previous with the same zTo */
  పూర్ణాంకం n_నిలువు;         /* Number of columns in this key */
  /* EV: R-30323-21917 */
  u8 isDeferred;       /* True if constraint checking is deferred till COMMIT */
  u8 aAction[2];        /* ON DELETE and ON UPDATE actions, respectively */
  Trigger *apTrigger[2];/* Triggers for aAction[] actions */
  నిర్మాణం s_నిలువు_Map {      /* Mapping of columns in pFrom to columns in zTo */
    పూర్ణాంకం iFrom;            /* Index of column in pFrom */
    అక్షరం *z_నిలువు;           /* Name of column in zTo.  If NULL use PRIMARY KEY */
  } a_నిలువు[FLEXARRAY];      /* One entry for each of nCol columns */
};

/* The size (in bytes) of an FKey object holding N columns.  The answer
** does NOT include space to hold the zTo name. */
#నిర్వచించు SZ_FKEY(N)  (offsetof(FKey,a_నిలువు)+(N)*పరిమాణం(నిర్మాణం s_నిలువు_Map))

/*
** SQLite supports many different ways to resolve a constraint
** error.  ROLLBACK processing means that a constraint violation
** causes the operation in process to fail and for the current transaction
** to be rolled back.  ABORT processing means the operation in process
** fails and any prior changes from that one operation are backed out,
** but the transaction is not rolled back.  FAIL processing means that
** the operation in progress stops and returns an error code.  But prior
** changes due to the same operation are not backed out and no rollback
** occurs.  IGNORE means that the particular row that caused the constraint
** error is not inserted or updated.  Processing continues and no error
** is returned.  REPLACE means that preexisting database rows that caused
** a UNIQUE constraint violation are removed so that the new insert or
** update can proceed.  Processing continues and no error is reported.
** UPDATE applies to insert operations only and means that the insert
** is omitted and the DO UPDATE clause of an upsert is run instead.
**
** RESTRICT, SETNULL, SETDFLT, and CASCADE actions apply only to foreign keys.
** RESTRICT is the same as ABORT for IMMEDIATE foreign keys and the
** same as ROLLBACK for DEFERRED keys.  SETNULL means that the foreign
** key is set to NULL.  SETDFLT means that the foreign key is set
** to its default value.  CASCADE means that a DELETE or UPDATE of the
** referenced table row is propagated into the row that holds the
** foreign key.
**
** The OE_Default value is a place holder that means to use whatever
** conflict resolution algorithm is required from context.
**
** The following symbolic values are used to record which type
** of conflict resolution action to take.
*/
#నిర్వచించు OE_None     0   /* There is no constraint to check */
#నిర్వచించు OE_Rollback 1   /* Fail the operation and rollback the transaction */
#నిర్వచించు OE_Abort    2   /* Back out changes but do no rollback transaction */
#నిర్వచించు OE_Fail     3   /* Stop the operation but leave all prior changes */
#నిర్వచించు OE_Ignore   4   /* Ignore the error. Do not do the INSERT or UPDATE */
#నిర్వచించు OE_Replace  5   /* Delete existing record, then do INSERT or UPDATE */
#నిర్వచించు OE_Update   6   /* Process as a DO UPDATE in an upsert */
#నిర్వచించు OE_Restrict 7   /* OE_Abort for IMMEDIATE, OE_Rollback for DEFERRED */
#నిర్వచించు OE_సమితి_Null  8   /* Set the foreign key value to NULL */
#నిర్వచించు OE_సమితి_Dflt  9   /* Set the foreign key value to its default */
#నిర్వచించు OE_Cascade  10  /* Cascade the changes */
#నిర్వచించు OE_Default  11  /* Do whatever the default action is */


/*
** An instance of the following structure is passed as the first
** argument to sqlite3VdbeKeyCompare and is used to control the
** comparison of the two index keys.
**
** The aSortOrder[] and aColl[] arrays have nAllField slots each. There
** are nKeyField slots for the columns of an index then extra slots
** for the rowid or key at the end.  The aSortOrder array is located after
** the aColl[] array.
**
** If SQLITE_ENABLE_PREUPDATE_HOOK is defined, then aSortFlags might be NULL
** to indicate that this object is for use by a preupdate hook.  When aSortFlags
** is NULL, then nAllField is uninitialized and no space is allocated for
** aColl[], so those fields may not be used.
*/
నిర్మాణం కీలకం_Info {
  u32 nRef;           /* Number of references to this KeyInfo object */
  u8 enc;             /* Text encoding - one of the SQLITE_UTF* values */
  u16 n_కీలకం_క్షేత్రం;      /* Number of key columns in the index */
  u16 n_All_క్షేత్రం;      /* Total columns, including key plus others */
  sqlite3 *db;        /* The database connection */
  u8 *a_క్రమీకరించు_Flags;     /* Sort order for each column. */
  CollSeq *aColl[FLEXARRAY]; /* Collating sequence for each term of the key */
};

/* The size (in bytes) of a KeyInfo object with up to N fields.  This includes
** the main body of the KeyInfo object and the aColl[] array of N elements,
** but does not count the memory used to hold aSortFlags[]. */
#నిర్వచించు SZ_KEYINFO(N)  (offsetof(కీలకం_Info,aColl) + (N)*పరిమాణం(CollSeq*))

/* The size of a bare KeyInfo with no aColl[] entries */
#ఒకవేళ FLEXARRAY+1 > 1
# define SZ_KEYINFO_0   offsetof(కీలకం_Info,aColl)
#లేకపోతే
# define SZ_KEYINFO_0   పరిమాణం(కీలకం_Info)
#అయితే_ముగింపు

/*
** Allowed bit values for entries in the KeyInfo.aSortFlags[] array.
*/
#నిర్వచించు KEYINFO_క్రమం_DESC    0x01    /* DESC sort order */
#నిర్వచించు KEYINFO_క్రమం_BIGNULL 0x02    /* NULL is larger than any other value */

/*
** This object holds a record which has been parsed out into individual
** fields, for the purposes of doing a comparison.
**
** A record is an object that contains one or more fields of data.
** Records are used to store the content of a table row and to store
** the key of an index.  A blob encoding of a record is created by
** the OP_MakeRecord opcode of the VDBE and is disassembled by the
** OP_Column opcode.
**
** An instance of this object serves as a "key" for doing a search on
** an index b+tree. The goal of the search is to find the entry that
** is closest to the key described by this object.  This object might hold
** just a prefix of the key.  The number of fields is given by nField.
**
** The r1 and r2 fields are the values to return if this key is less than
** or greater than a key in the btree, respectively.  These are normally
** -1 and +1 respectively, but might be inverted to +1 and -1 if the b-tree
** is in DESC order.
**
** The key comparison functions actually return default_rc when they find
** an equals comparison.  default_rc can be -1, 0, or +1.  If there are
** multiple entries in the b-tree with the same key (when only looking
** at the first nField elements) then default_rc can be set to -1 to
** cause the search to find the last match, or +1 to cause the search to
** find the first match.
**
** The key comparison functions will set eqSeen to true if they ever
** get and equal results when comparing this structure to a b-tree record.
** When default_rc!=0, the search might end up on the record immediately
** before the first match or immediately after the last match.  The
** eqSeen field will indicate whether or not an exact match exists in the
** b-tree.
*/
నిర్మాణం Unpacked_రికార్డు {
  కీలకం_Info *p_కీలకం_Info;  /* Comparison info for the index that is unpacked */
  Mem *aMem;          /* Values for columns of the index */
  సంఘం {
    అక్షరం *z;            /* Cache of aMem[0].z for vdbeRecordCompareString() */
    i64 i;              /* Cache of aMem[0].u.i for vdbeRecordCompareInt() */
  } u;
  పూర్ణాంకం n;              /* Cache of aMem[0].n used by vdbeRecordCompareString() */
  u16 n_క్షేత్రం;         /* Number of entries in apMem[] */
  i8 default_rc;      /* Comparison result if keys are equal */
  u8 err_సంకేతం;         /* Error detected by xRecordCompare (CORRUPT or NOMEM) */
  i8 r1;              /* Value to return if (lhs < rhs) */
  i8 r2;              /* Value to return if (lhs > rhs) */
  u8 eq_చూసిన;          /* True if an equality comparison has been seen */
};


/*
** Each SQL index is represented in memory by an
** instance of the following structure.
**
** The columns of the table that are to be indexed are described
** by the aiColumn[] field of this structure.  For example, suppose
** we have the following table and index:
**
**     CREATE TABLE Ex1(c1 int, c2 int, c3 text);
**     CREATE INDEX Ex2 ON Ex1(c3,c1);
**
** In the Table structure describing Ex1, nCol==3 because there are
** three columns in the table.  In the Index structure describing
** Ex2, nColumn==2 since 2 of the 3 columns of Ex1 are indexed.
** The value of aiColumn is {2, 0}.  aiColumn[0]==2 because the
** first column to be indexed (c3) has an index of 2 in Ex1.aCol[].
** The second column to be indexed (c1) has an index of 0 in
** Ex1.aCol[], hence Ex2.aiColumn[1]==0.
**
** The Index.onError field determines whether or not the indexed columns
** must be unique and what to do if they are not.  When Index.onError=OE_None,
** it means this is not a unique index.  Otherwise it is a unique index
** and the value of Index.onError indicates which conflict resolution
** algorithm to employ when an attempt is made to insert a non-unique
** element.
**
** The colNotIdxed bitmask is used in combination with SrcItem.colUsed
** for a fast test to see if an index can serve as a covering index.
** colNotIdxed has a 1 bit for every column of the original table that
** is *not* available in the index.  Thus the expression
** "colUsed & colNotIdxed" will be non-zero if the index is not a
** covering index.  The most significant bit of of colNotIdxed will always
** be true (note-20221022-a).  If a column beyond the 63rd column of the
** table is used, the "colUsed & colNotIdxed" test will always be non-zero
** and we have to assume either that the index is not covering, or use
** an alternative (slower) algorithm to determine whether or not
** the index is covering.
**
** While parsing a CREATE TABLE or CREATE INDEX statement in order to
** generate VDBE code (as opposed to parsing one read from an sqlite_schema
** table as part of parsing an existing database schema), transient instances
** of this structure may be created. In this case the Index.tnum variable is
** used to store the address of a VDBE instruction, not a database page
** number (it cannot - the database page is not allocated until the VDBE
** program is executed). See convertToWithoutRowidTable() for details.
*/
నిర్మాణం సూచీ {
  అక్షరం *zName;             /* Name of this index */
  i16 *ai_స్తంభం;           /* Which columns are used by this index.  1st is 0 */
  LogEst *ai_అడ్డువరుస_Log_Est;     /* From ANALYZE: Est. rows selected by each column */
  పట్టిక *p_పట్టిక;           /* The SQL table being indexed */
  అక్షరం *z_నిలువు_Aff;           /* String defining the affinity of each column */
  సూచీ *p_తదుపరి;            /* The next index associated with the same table */
  Schema *pSchema;         /* Schema containing this index */
  u8 *a_క్రమీకరించు_క్రమం;          /* for each column: True==DESC, False==ASC */
  స్థిరరాశి అక్షరం **azColl;     /* Array of collation sequence names for index */
  సమాసం *pPartIdxWhere;     /* WHERE clause for partial indices */
  సమాసం_List *a_నిలువు_సమాసం;      /* Column expressions */
  Pgno tnum;               /* DB Page containing root of this index */
  LogEst sz_Idx_అడ్డువరుస;         /* Estimated average row size in bytes */
  u16 n_కీలకం_నిలువు;             /* Number of columns forming the key */
  u16 n_స్తంభం;             /* Nr columns in btree. Can be 2*Table.nCol */
  u8 onError;              /* OE_Abort, OE_Ignore, OE_Replace, or OE_None */
  గుర్తులేని idxType:2;      /* 0:Normal 1:UNIQUE, 2:PRIMARY KEY, 3:IPK */
  గుర్తులేని bUnordered:1;   /* Use this index for == or IN queries only */
  గుర్తులేని uniqNotNull:1;  /* True if UNIQUE and NOT NULL for all columns */
  గుర్తులేని isResized:1;    /* True if resizeIndexObject() has been called */
  గుర్తులేని isCovering:1;   /* True if this is a covering index */
  గుర్తులేని no_Skip_పరిశీలన:1;   /* Do not try to use skip-scan if true */
  గుర్తులేని hasStat1:1;     /* aiRowLogEst values come from sqlite_stat1 */
  గుర్తులేని bNoQuery:1;     /* Do not use this index to optimize queries */
  గుర్తులేని b_Asc_కీలకం_Bug:1;   /* True if the bba7b69f9849b5bf bug applies */
  గుర్తులేని bHasVCol:1;     /* Index references one or more VIRTUAL columns */
  గుర్తులేని b_Has_సమాసం:1;     /* Index contains an expression, either a literal
                           ** expression, or a reference to a VIRTUAL column */
#నిర్వచితమైతే SQLITE_ENABLE_STAT4
  పూర్ణాంకం nSample;             /* Number of elements in aSample[] */
  పూర్ణాంకం mxSample;            /* Number of slots allocated to aSample[] */
  పూర్ణాంకం n_Sample_నిలువు;          /* Size of IndexSample.anEq[] and so on */
  tRowcnt *aAvgEq;         /* Average nEq values for keys not in aSample */
  సూచీ_Sample *aSample;    /* Samples of the left-most key */
  tRowcnt *ai_అడ్డువరుస_Est;       /* Non-logarithmic stat1 data for this index */
  tRowcnt n_అడ్డువరుస_Est0;        /* Non-logarithmic number of rows in the index */
#అయితే_ముగింపు
  Bitmask నిలువు_Not_Idxed;     /* Unindexed columns in pTab */
};

/*
** Allowed values for Index.idxType
*/
#నిర్వచించు SQLITE_IDXTYPE_APPDEF      0   /* Created using CREATE INDEX */
#నిర్వచించు SQLITE_IDXTYPE_UNIQUE      1   /* Implements a UNIQUE constraint */
#నిర్వచించు SQLITE_IDXTYPE_PRIMARYKEY  2   /* Is the PRIMARY KEY for the table */
#నిర్వచించు SQLITE_IDXTYPE_IPK         3   /* INTEGER PRIMARY KEY index */

/* Return true if index X is a PRIMARY KEY index */
#నిర్వచించు Is_Primary_కీలకం_సూచీ(X)  ((X)->idxType==SQLITE_IDXTYPE_PRIMARYKEY)

/* Return true if index X is a UNIQUE index */
#నిర్వచించు Is_Unique_సూచీ(X)      ((X)->onError!=OE_None)

/* The Index.aiColumn[] values are normally positive integer.  But
** there are some negative values that have special meaning:
*/
#నిర్వచించు XN_ROWID     (-1)     /* Indexed column is the rowid */
#నిర్వచించు XN_సమాసం      (-2)     /* Indexed column is an expression */

/*
** Each sample stored in the sqlite_stat4 table is represented in memory
** using a structure of this type.  See documentation at the top of the
** analyze.c source file for additional information.
*/
నిర్మాణం సూచీ_Sample {
  శూన్య *p;          /* Pointer to sampled record */
  పూర్ణాంకం n;            /* Size of record in bytes */
  tRowcnt *anEq;    /* Est. number of rows where the key equals this sample */
  tRowcnt *anLt;    /* Est. number of rows where key is less than this sample */
  tRowcnt *anDLt;   /* Est. number of distinct keys less than this sample */
};

/*
** Possible values to use within the flags argument to sqlite3GetToken().
*/
#నిర్వచించు SQLITE_TOKEN_QUOTED    0x1 /* Token is a quoted identifier. */
#నిర్వచించు SQLITE_TOKEN_KEYWORD   0x2 /* Token is a keyword. */

/*
** Each token coming out of the lexer is an instance of
** this structure.  Tokens are also used as part of an expression.
**
** The memory that "z" points to is owned by other objects.  Take care
** that the owner of the "z" string does not deallocate the string before
** the Token goes out of scope!  Very often, the "z" points to some place
** in the middle of the Parse.zSql text.  But it might also point to a
** static string.
*/
నిర్మాణం Token {
  స్థిరరాశి అక్షరం *z;     /* Text of the token.  Not NULL-terminated! */
  గుర్తులేని పూర్ణాంకం n;    /* Number of characters in this token */
};

/*
** An instance of this structure contains information needed to generate
** code for a SELECT that contains aggregate functions.
**
** If Expr.op==TK_AGG_COLUMN or TK_AGG_FUNCTION then Expr.pAggInfo is a
** pointer to this structure.  The Expr.iAgg field is the index in
** AggInfo.aCol[] or AggInfo.aFunc[] of information needed to generate
** code for that node.
**
** AggInfo.pGroupBy and AggInfo.aFunc.pExpr point to fields within the
** original Select structure that describes the SELECT statement.  These
** fields do not need to be freed when deallocating the AggInfo structure.
*/
నిర్మాణం AggInfo {
  u8 ప్రత్యక్ష_బహుళకం;          /* Direct rendering mode means take data directly
                          ** from source tables rather than from accumulators */
  u8 use_క్రమీకరణ_Idx;       /* In direct mode, reference the sorting index rather
                          ** than the source table */
  u32 n_క్రమీకరణ_స్తంభం;     /* Number of columns in the sorting index */
  పూర్ణాంకం క్రమీకరణ_Idx;         /* Cursor number of the sorting index */
  పూర్ణాంకం క్రమీకరణ_Idx_PTab;     /* Cursor number of pseudo-table */
  పూర్ణాంకం i_మొదటి_Reg;          /* First register in range for aCol[] and aFunc[] */
  సమాసం_List *p_సమూహం_By;     /* The group by clause */
  నిర్మాణం Agg_Info_నిలువు {    /* For each column used in source tables */
    పట్టిక *pTab;             /* Source table */
    సమాసం *pCExpr;            /* The original expression */
    పూర్ణాంకం i_పట్టిక;              /* Cursor number of the source table */
    పూర్ణాంకం i_స్తంభం;             /* Column number within the source table */
    పూర్ణాంకం i_Sorter_స్తంభం;       /* Column number in the sorting index */
  } *a_నిలువు;
  పూర్ణాంకం n_స్తంభం;            /* Number of used entries in aCol[] */
  పూర్ణాంకం n_సంచాయకం;       /* Number of columns that show through to the output.
                          ** Additional columns are used only as parameters to
                          ** aggregate functions */
  నిర్మాణం AggInfo_func {   /* For each aggregate function */
    సమాసం *pFExpr;            /* Expression encoding the function */
    FuncDef *pFunc;          /* The aggregate function implementation */
    పూర్ణాంకం iDistinct;           /* Ephemeral table used to enforce DISTINCT */
    పూర్ణాంకం iDistAddr;           /* Address of OP_OpenEphemeral */
    పూర్ణాంకం iOBTab;              /* Ephemeral table to implement ORDER BY */
    u8 bOBPayload;           /* iOBTab has payload columns separate from key */
    u8 bOBUnique;            /* Enforce uniqueness on iOBTab keys */
    u8 bUseSubtype;          /* Transfer subtype info through sorter */
  } *aFunc;
  పూర్ణాంకం nFunc;              /* Number of entries in aFunc[] */
  u32 selId;              /* Select to which this AggInfo belongs */
#నిర్వచితమైతే SQLITE_DEBUG
  Select *pSelect;        /* SELECT statement that this AggInfo supports */
#అయితే_ముగింపు
};

/*
** Macros to compute aCol[] and aFunc[] register numbers.
**
** These macros should not be used prior to the call to
** assignAggregateRegisters() that computes the value of pAggInfo->iFirstReg.
** The assert()s that are part of this macro verify that constraint.
*/
#నిర్వచితం_కాకపోతే NDEBUG
#నిర్వచించు Agg_Info_స్తంభం_Reg(A,I)  (ప్రతిజ్ఞా((A)->i_మొదటి_Reg),(A)->i_మొదటి_Reg+(I))
#నిర్వచించు AggInfoFuncReg(A,I)    \
                      (ప్రతిజ్ఞా((A)->i_మొదటి_Reg),(A)->i_మొదటి_Reg+(A)->n_స్తంభం+(I))
#లేకపోతే
#నిర్వచించు Agg_Info_స్తంభం_Reg(A,I)  ((A)->i_మొదటి_Reg+(I))
#నిర్వచించు AggInfoFuncReg(A,I)    \
                      ((A)->i_మొదటి_Reg+(A)->n_స్తంభం+(I))
#అయితే_ముగింపు

/*
** The datatype ynVar is a signed integer, either 16-bit or 32-bit.
** Usually it is 16-bits.  But if SQLITE_MAX_VARIABLE_NUMBER is greater
** than 32767 we have to make it 32-bit.  16-bit is preferred because
** it uses less memory in the Expr object, which is a big memory user
** in systems with lots of prepared statements.  And few applications
** need more than about 10 or 20 variables.  But some extreme users want
** to have prepared statements with over 32766 variables, and for them
** the option is available (at compile-time).
*/
#ఒకవేళ SQLITE_MAX_VARIABLE_సంఖ్య<32767
రకనిర్వచనం i16 ynVar;
#లేకపోతే
రకనిర్వచనం పూర్ణాంకం ynVar;
#అయితే_ముగింపు

/*
** Each node of an expression in the parse tree is an instance
** of this structure.
**
** Expr.op is the opcode. The integer parser token codes are reused
** as opcodes here. For example, the parser defines TK_GE to be an integer
** code representing the ">=" operator. This same integer code is reused
** to represent the greater-than-or-equal-to operator in the expression
** tree.
**
** If the expression is an SQL literal (TK_INTEGER, TK_FLOAT, TK_BLOB,
** or TK_STRING), then Expr.u.zToken contains the text of the SQL literal. If
** the expression is a variable (TK_VARIABLE), then Expr.u.zToken contains the
** variable name. Finally, if the expression is an SQL function (TK_FUNCTION),
** then Expr.u.zToken contains the name of the function.
**
** Expr.pRight and Expr.pLeft are the left and right subexpressions of a
** binary operator. Either or both may be NULL.
**
** Expr.x.pList is a list of arguments if the expression is an SQL function,
** a CASE expression or an IN expression of the form "<lhs> IN (<y>, <z>...)".
** Expr.x.pSelect is used if the expression is a sub-select or an expression of
** the form "<lhs> IN (SELECT ...)". If the EP_xIsSelect bit is set in the
** Expr.flags mask, then Expr.x.pSelect is valid. Otherwise, Expr.x.pList is
** valid.
**
** An expression of the form ID or ID.ID refers to a column in a table.
** For such expressions, Expr.op is set to TK_COLUMN and Expr.iTable is
** the integer cursor number of a VDBE cursor pointing to that table and
** Expr.iColumn is the column number for the specific column.  If the
** expression is used as a result in an aggregate SELECT, then the
** value is also stored in the Expr.iAgg column in the aggregate so that
** it can be accessed after all aggregates are computed.
**
** If the expression is an unbound variable marker (a question mark
** character '?' in the original SQL) then the Expr.iTable holds the index
** number for that variable.
**
** If the expression is a subquery then Expr.iColumn holds an integer
** register number containing the result of the subquery.  If the
** subquery gives a constant result, then iTable is -1.  If the subquery
** gives a different answer at different times during statement processing
** then iTable is the address of a subroutine that computes the subquery.
**
** If the Expr is of type OP_Column, and the table it is selecting from
** is a disk table or the "old.*" pseudo-table, then pTab points to the
** corresponding table definition.
**
** ALLOCATION NOTES:
**
** Expr objects can use a lot of memory space in database schema.  To
** help reduce memory requirements, sometimes an Expr object will be
** truncated.  And to reduce the number of memory allocations, sometimes
** two or more Expr objects will be stored in a single memory allocation,
** together with Expr.u.zToken strings.
**
** If the EP_Reduced and EP_TokenOnly flags are set when
** an Expr object is truncated.  When EP_Reduced is set, then all
** the child Expr objects in the Expr.pLeft and Expr.pRight subtrees
** are contained within the same memory allocation.  Note, however, that
** the subtrees in Expr.x.pList or Expr.x.pSelect are always separately
** allocated, regardless of whether or not EP_Reduced is set.
*/
నిర్మాణం సమాసం {
  u8 op;                 /* Operation performed by this node */
  అక్షరం aff_సమాసం;          /* affinity, or RAISE type */
  u8 op2;                /* TK_REGISTER/TK_TRUTH: original value of Expr.op
                         ** TK_COLUMN: the value of p5 for OP_Column
                         ** TK_AGG_FUNCTION: nesting depth
                         ** TK_FUNCTION: NC_SelfRef flag if needs OP_PureFunc */
#నిర్వచితమైతే SQLITE_DEBUG
  u8 vvaFlags;           /* Verification flags. */
#అయితే_ముగింపు
  u32 flags;             /* Various flags.  EP_* See below */
  సంఘం {
    అక్షరం *zToken;          /* Token value. Zero terminated and dequoted */
    పూర్ణాంకం i_విలువ;            /* Non-negative integer value if EP_IntValue */
  } u;

  /* If the EP_TokenOnly flag is set in the Expr.flags mask, then no
  ** space is allocated for the fields below this point. An attempt to
  ** access them will result in a segfault or malfunction.
  *********************************************************************/

  సమాసం *p_ఎడమ;           /* Left subnode */
  సమాసం *p_కుడి;          /* Right subnode */
  సంఘం {
    సమాసం_List *pList;     /* op = IN, EXISTS, SELECT, CASE, FUNCTION, BETWEEN */
    Select *pSelect;     /* EP_xIsSelect and op = IN, EXISTS, SELECT */
  } x;

  /* If the EP_Reduced flag is set in the Expr.flags mask, then no
  ** space is allocated for the fields below this point. An attempt to
  ** access them will result in a segfault or malfunction.
  *********************************************************************/

#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
  పూర్ణాంకం n_ఎత్తు;           /* Height of the tree headed by this node */
#అయితే_ముగింపు
  పూర్ణాంకం i_పట్టిక;            /* TK_COLUMN: cursor number of table holding column
                         ** TK_REGISTER: register number
                         ** TK_TRIGGER: 1 -> new, 0 -> old
                         ** EP_Unlikely:  134217728 times likelihood
                         ** TK_IN: ephemeral table holding RHS
                         ** TK_SELECT_COLUMN: Number of columns on the LHS
                         ** TK_SELECT: 1st register of result vector */
  ynVar i_స్తంభం;         /* TK_COLUMN: column index.  -1 for rowid.
                         ** TK_VARIABLE: variable number (always >= 1).
                         ** TK_SELECT_COLUMN: column of the result vector */
  i16 iAgg;              /* Which entry in pAggInfo->aCol[] or ->aFunc[] */
  సంఘం {
    పూర్ణాంకం iJoin;             /* If EP_OuterON or EP_InnerON, the right table */
    పూర్ణాంకం iOfst;             /* else: start of token from start of statement */
  } w;
  AggInfo *pAggInfo;     /* Used by TK_AGG_COLUMN and TK_AGG_FUNCTION */
  సంఘం {
    పట్టిక *pTab;           /* TK_COLUMN: Table containing column. Can be NULL
                           ** for a column of an index on an expression */
    కిటికీ *pWin;          /* EP_WinFunc: Window/Filter defn for a function */
    పూర్ణాంకం nReg;              /* TK_NULLS: Number of registers to NULL out */
    నిర్మాణం {               /* TK_IN, TK_SELECT, and TK_EXISTS */
      పూర్ణాంకం iAddr;             /* Subroutine entry address */
      పూర్ణాంకం regReturn;         /* Register used to hold return address */
    } sub;
  } y;
};

/* The following are the meanings of bits in the Expr.flags field.
** Value restrictions:
**
**          EP_Agg == NC_HasAgg == SF_HasAgg
**          EP_Win == NC_HasWin
*/
#నిర్వచించు EP_OuterON    0x000001 /* Originates in ON/USING clause of outer join */
#నిర్వచించు EP_InnerON    0x000002 /* Originates in ON/USING of an inner join */
#నిర్వచించు EP_Distinct   0x000004 /* Aggregate function with DISTINCT keyword */
#నిర్వచించు EP_HasFunc    0x000008 /* Contains one or more functions of any kind */
#నిర్వచించు EP_Agg        0x000010 /* Contains one or more aggregate functions */
#నిర్వచించు EP_Fixed_నిలువు   0x000020 /* TK_Column with a known fixed value */
#నిర్వచించు EP_VarSelect  0x000040 /* pSelect is correlated, not constant */
#నిర్వచించు EP_DblQuoted  0x000080 /* token.z was originally in "..." */
#నిర్వచించు EP_InfixFunc  0x000100 /* True for an infix function: LIKE, GLOB, etc */
#నిర్వచించు EP_Collate    0x000200 /* Tree contains a TK_COLLATE operator */
#నిర్వచించు EP_Commuted   0x000400 /* Comparison operator has been commuted */
#నిర్వచించు EP_Int_విలువ   0x000800 /* Integer value contained in u.iValue */
#నిర్వచించు EP_xIsSelect  0x001000 /* x.pSelect is valid (otherwise x.pList is) */
#నిర్వచించు EP_Skip       0x002000 /* Operator does not contribute to affinity */
#నిర్వచించు EP_Reduced    0x004000 /* Expr struct EXPR_REDUCEDSIZE bytes only */
#నిర్వచించు EP_Win        0x008000 /* Contains window functions */
#నిర్వచించు EP_TokenOnly  0x010000 /* Expr struct EXPR_TOKENONLYSIZE bytes only */
#నిర్వచించు EP_నిండిన_కొలత   0x020000 /* Expr structure must remain full sized */
#నిర్వచించు EP_If_Null_అడ్డువరుస  0x040000 /* The TK_IF_NULL_ROW opcode */
#నిర్వచించు EP_Unlikely   0x080000 /* unlikely() or likelihood() function */
#నిర్వచించు EP_ConstFunc  0x100000 /* A SQLITE_FUNC_CONSTANT or _SLOCHNG function */
#నిర్వచించు EP_CanBeNull  0x200000 /* Can be null despite NOT NULL constraint */
#నిర్వచించు EP_Subquery   0x400000 /* Tree contains a TK_SELECT operator */
#నిర్వచించు EP_ఆకు       0x800000 /* Expr.pLeft, .pRight, .u.pSelect all NULL */
#నిర్వచించు EP_WinFunc   0x1000000 /* TK_FUNCTION with Expr.y.pWin set */
#నిర్వచించు EP_Subrtn    0x2000000 /* Uses Expr.y.sub. TK_IN, _SELECT, or _EXISTS */
#నిర్వచించు EP_Quoted    0x4000000 /* TK_ID was originally quoted */
#నిర్వచించు EP_Static    0x8000000 /* Held in memory not obtained from malloc() */
#నిర్వచించు EP_IsTrue   0x10000000 /* Always has boolean value of TRUE */
#నిర్వచించు EP_IsFalse  0x20000000 /* Always has boolean value of FALSE */
#నిర్వచించు EP_FromDDL  0x40000000 /* Originates from sqlite_schema */
#నిర్వచించు EP_SubtArg  0x80000000 /* Is argument to SQLITE_SUBTYPE function */

/* The EP_Propagate mask is a set of properties that automatically propagate
** upwards into parent nodes.
*/
#నిర్వచించు EP_Propagate (EP_Collate|EP_Subquery|EP_HasFunc)

/* Macros can be used to test, set, or clear bits in the
** Expr.flags field.
*/
#నిర్వచించు సమాసం_Has_Property(E,P)     (((E)->flags&(u32)(P))!=0)
#నిర్వచించు సమాసం_Has_All_Property(E,P)  (((E)->flags&(u32)(P))==(u32)(P))
#నిర్వచించు సమాసం_సమితి_Property(E,P)     (E)->flags|=(u32)(P)
#నిర్వచించు సమాసం_Clear_Property(E,P)   (E)->flags&=~(u32)(P)
#నిర్వచించు సమాసం_Always_True(E)   (((E)->flags&(EP_OuterON|EP_IsTrue))==EP_IsTrue)
#నిర్వచించు సమాసం_Always_False(E)  (((E)->flags&(EP_OuterON|EP_IsFalse))==EP_IsFalse)
#నిర్వచించు సమాసం_Is_నిండిన_కొలత(E)   (((E)->flags&(EP_Reduced|EP_TokenOnly))==0)

/* Macros used to ensure that the correct members of unions are accessed
** in Expr.
*/
#నిర్వచించు సమాసం_Use_UToken(E)    (((E)->flags&EP_Int_విలువ)==0)
#నిర్వచించు సమాసం_Use_UValue(E)    (((E)->flags&EP_Int_విలువ)!=0)
#నిర్వచించు సమాసం_Use_WOfst(E)     (((E)->flags&(EP_InnerON|EP_OuterON))==0)
#నిర్వచించు సమాసం_Use_WJoin(E)     (((E)->flags&(EP_InnerON|EP_OuterON))!=0)
#నిర్వచించు సమాసం_Use_XList(E)     (((E)->flags&EP_xIsSelect)==0)
#నిర్వచించు సమాసం_Use_XSelect(E)   (((E)->flags&EP_xIsSelect)!=0)
#నిర్వచించు సమాసం_Use_YTab(E)      (((E)->flags&(EP_WinFunc|EP_Subrtn))==0)
#నిర్వచించు సమాసం_Use_YWin(E)      (((E)->flags&EP_WinFunc)!=0)
#నిర్వచించు సమాసం_Use_YSub(E)      (((E)->flags&EP_Subrtn)!=0)

/* Flags for use with Expr.vvaFlags
*/
#నిర్వచించు EP_NoReduce   0x01  /* Cannot EXPRDUP_REDUCE this Expr */
#నిర్వచించు EP_Immutable  0x02  /* Do not change this Expr node */

/* The ExprSetVVAProperty() macro is used for Verification, Validation,
** and Accreditation only.  It works like ExprSetProperty() during VVA
** processes but is a no-op for delivery.
*/
#నిర్వచితమైతే SQLITE_DEBUG
# define సమాసం_సమితి_VVAProperty(E,P)   (E)->vvaFlags|=(P)
# define సమాసం_Has_VVAProperty(E,P)   (((E)->vvaFlags&(P))!=0)
# define సమాసం_Clear_VVAProperties(E) (E)->vvaFlags = 0
#లేకపోతే
# define సమాసం_సమితి_VVAProperty(E,P)
# define సమాసం_Has_VVAProperty(E,P)   0
# define సమాసం_Clear_VVAProperties(E)
#అయితే_ముగింపు

/*
** Macros to determine the number of bytes required by a normal Expr
** struct, an Expr struct with the EP_Reduced flag set in Expr.flags
** and an Expr struct with the EP_TokenOnly flag set.
*/
#నిర్వచించు సమాసం_FULLSIZE           పరిమాణం(సమాసం)           /* Full size */
#నిర్వచించు సమాసం_REDUCEDSIZE        offsetof(సమాసం,i_పట్టిక)  /* Common features */
#నిర్వచించు సమాసం_TOKENONLYSIZE      offsetof(సమాసం,p_ఎడమ)   /* Fewer features */

/*
** Flags passed to the sqlite3ExprDup() function. See the header comment
** above sqlite3ExprDup() for details.
*/
#నిర్వచించు EXPRDUP_REDUCE         0x0001  /* Used reduced-size Expr nodes */

/*
** True if the expression passed as an argument was a function with
** an OVER() clause (a window function).
*/
#నిర్వచితమైతే SQLITE_OMIT_WINDOWFUNC
# define Is_కిటికీ_Func(p) 0
#లేకపోతే
# define Is_కిటికీ_Func(p) ( \
    సమాసం_Has_Property((p), EP_WinFunc) && p->y.pWin->eFrmType!=TK_FILTER \
 )
#అయితే_ముగింపు

/*
** A list of expressions.  Each expression may optionally have a
** name.  An expr/name combination can be used in several ways, such
** as the list of "expr AS ID" fields following a "SELECT" or in the
** list of "ID = expr" items in an UPDATE.  A list of expressions can
** also be used as the argument to a function, in which case the a.zName
** field is not used.
**
** In order to try to keep memory usage down, the Expr.a.zEName field
** is used for multiple purposes:
**
**     eEName          Usage
**    ----------       -------------------------
**    ENAME_NAME       (1) the AS of result set column
**                     (2) COLUMN= of an UPDATE
**
**    ENAME_TAB        DB.TABLE.NAME used to resolve names
**                     of subqueries
**
**    ENAME_SPAN       Text of the original result set
**                     expression.
*/
నిర్మాణం సమాసం_List {
  పూర్ణాంకం n_సమాసం;             /* Number of expressions on the list */
  పూర్ణాంకం nAlloc;            /* Number of a[] slots allocated */
  నిర్మాణం సమాసం_List_అంశం { /* For each expression in the list */
    సమాసం *p_సమాసం;            /* The parse tree for this expression */
    అక్షరం *zEName;           /* Token associated with this expression */
    నిర్మాణం {
      u8 క్రమీకరించు_Flags;           /* Mask of KEYINFO_ORDER_* flags */
      గుర్తులేని eEName :2;     /* Meaning of zEName */
      గుర్తులేని పూర్తి :1;       /* Indicates when processing is finished */
      గుర్తులేని reusable :1;   /* Constant expression is reusable */
      గుర్తులేని bSorterRef :1; /* Defer evaluation until after sorting */
      గుర్తులేని bNulls :1;     /* True if explicit "NULLS FIRST/LAST" */
      గుర్తులేని b_వాడిన :1;      /* This column used in a SF_NestedFrom subquery */
      గుర్తులేని b_Using_పదం:1;  /* Term from the USING clause of a NestedFrom */
      గుర్తులేని bNoExpand: 1;  /* Term is an auxiliary in NestedFrom and should
                              ** not be expanded by "*" in parent queries */
    } fg;
    సంఘం {
      నిర్మాణం {             /* Used by any ExprList other than Parse.pConsExpr */
        u16 i_క్రమం_By_నిలువు;      /* For ORDER BY, column number in result set */
        u16 iAlias;           /* Index into Parse.aAlias[] for zName */
      } x;
      పూర్ణాంకం i_Const_సమాసం_Reg;   /* Register in which Expr value is cached. Used only
                           ** by Parse.pConstExpr */
    } u;
  } a[FLEXARRAY];          /* One slot for each expression in the list */
};

/* The size (in bytes) of an ExprList object that is big enough to hold
** as many as N expressions. */
#నిర్వచించు SZ_EXPRLIST(N)  \
             (offsetof(సమాసం_List,a) + (N)*పరిమాణం(నిర్మాణం సమాసం_List_అంశం))

/*
** Allowed values for Expr.a.eEName
*/
#నిర్వచించు ENAME_NAME  0       /* The AS clause of a result set */
#నిర్వచించు ENAME_పరిధి  1       /* Complete text of the result set expression */
#నిర్వచించు ENAME_TAB   2       /* "DB.TABLE.NAME" for the result set */
#నిర్వచించు ENAME_ROWID 3       /* "DB.TABLE._rowid_" for * expansion of rowid */

/*
** An instance of this structure can hold a simple list of identifiers,
** such as the list "a,b,c" in the following statements:
**
**      INSERT INTO t(a,b,c) VALUES ...;
**      CREATE INDEX idx ON t(a,b,c);
**      CREATE TRIGGER trig BEFORE UPDATE ON t(a,b,c) ...;
**
** The IdList.a.idx field is used when the IdList represents the list of
** column names after a table name in an INSERT statement.  In the statement
**
**     INSERT INTO t(a,b,c) ...
**
** If "a" is the k-th column of table "t", then IdList.a[0].idx==k.
*/
నిర్మాణం IdList {
  పూర్ణాంకం nId;         /* Number of identifiers on the list */
  నిర్మాణం Id_List_అంశం {
    అక్షరం *zName;      /* Name of the identifier */
  } a[FLEXARRAY];
};

/* The size (in bytes) of an IdList object that can hold up to N IDs. */
#నిర్వచించు SZ_IDLIST(N)  (offsetof(IdList,a)+(N)*పరిమాణం(నిర్మాణం Id_List_అంశం))

/*
** Allowed values for IdList.eType, which determines which value of the a.u4
** is valid.
*/
#నిర్వచించు EU4_NONE   0   /* Does not use IdList.a.u4 */
#నిర్వచించు EU4_IDX    1   /* Uses IdList.a.u4.idx */
#నిర్వచించు EU4_సమాసం   2   /* Uses IdList.a.u4.pExpr -- NOT CURRENTLY USED */

/*
** Details of the implementation of a subquery.
*/
నిర్మాణం Subquery {
  Select *pSelect;  /* A SELECT statement used in place of a table name */
  పూర్ణాంకం addrFillSub;  /* Address of subroutine to initialize a subquery */
  పూర్ణాంకం regReturn;    /* Register holding return address of addrFillSub */
  పూర్ణాంకం reg_ఫలితం;    /* Registers holding results of a co-routine */
};

/*
** The SrcItem object represents a single term in the FROM clause of a query.
** The SrcList object is mostly an array of SrcItems.
**
** The jointype starts out showing the join type between the current table
** and the next table on the list.  The parser builds the list this way.
** But sqlite3SrcListShiftJoinType() later shifts the jointypes so that each
** jointype expresses the join between the table and the previous table.
**
** In the colUsed field, the high-order bit (bit 63) is set if the table
** contains more than 63 columns and the 64-th or later column is used.
**
** Aggressive use of "union" helps keep the size of the object small.  This
** has been shown to boost performance, in addition to saving memory.
** Access to union elements is gated by the following rules which should
** always be checked, either by an if-statement or by an assert().
**
**    Field              Only access if this is true
**    ---------------    -----------------------------------
**    u1.zIndexedBy      fg.isIndexedBy
**    u1.pFuncArg        fg.isTabFunc
**    u1.nRow            !fg.isTabFunc  && !fg.isIndexedBy
**
**    u2.pIBIndex        fg.isIndexedBy
**    u2.pCteUse         fg.isCte
**
**    u3.pOn             !fg.isUsing
**    u3.pUsing          fg.isUsing
**
**    u4.zDatabase       !fg.fixedSchema && !fg.isSubquery
**    u4.pSchema         fg.fixedSchema
**    u4.pSubq           fg.isSubquery
**
** See also the sqlite3SrcListDelete() routine for assert() statements that
** check invariants on the fields of this object, especially the flags
** inside the fg struct.
*/
నిర్మాణం Src_అంశం {
  అక్షరం *zName;      /* Name of the table */
  అక్షరం *zAlias;     /* The "B" part of a "A AS B" phrase.  zName is the "A" */
  పట్టిక *pSTab;     /* Table object for zName. Mnemonic: Srcitem-TABle */
  నిర్మాణం {
    u8 jointype;      /* Type of join between this table and the previous */
    గుర్తులేని not_సూచీకృత :1;    /* True if there is a NOT INDEXED clause */
    గుర్తులేని is_సూచీకృత_By :1;   /* True if there is an INDEXED BY clause */
    గుర్తులేని isSubquery :1;    /* True if this term is a subquery */
    గుర్తులేని isTabFunc :1;     /* True if table-valued-function syntax */
    గుర్తులేని isCorrelated :1;  /* True if sub-query is correlated */
    గుర్తులేని isMaterialized:1; /* This is a materialized view */
    గుర్తులేని ద్వారా_Coroutine :1;  /* Implemented as a co-routine */
    గుర్తులేని isRecursive :1;   /* True for recursive reference in WITH */
    గుర్తులేని fromDDL :1;       /* Comes from sqlite_schema */
    గుర్తులేని isCte :1;         /* This is a CTE */
    గుర్తులేని notCte :1;        /* This item may not match a CTE */
    గుర్తులేని isUsing :1;       /* u3.pUsing is valid */
    గుర్తులేని isOn :1;          /* u3.pOn was once valid and non-NULL */
    గుర్తులేని isSynthUsing :1;  /* u3.pUsing is synthesized from NATURAL */
    గుర్తులేని isNestedFrom :1;  /* pSelect is a SF_NestedFrom subquery */
    గుర్తులేని rowid_వాడిన :1;     /* The ROWID of this table is referenced */
    గుర్తులేని fixedSchema :1;   /* Uses u4.pSchema, not u4.zDatabase */
    గుర్తులేని hadSchema :1;     /* Had u4.zDatabase before u4.pSchema */
    గుర్తులేని fromExists :1;    /* Comes from WHERE EXISTS(...) */
  } fg;
  పూర్ణాంకం iCursor;      /* The VDBE cursor number used to access this table */
  Bitmask నిలువు_వాడిన;  /* Bit N set if column N used. Details above for N>62 */
  సంఘం {
    అక్షరం *z_సూచీకృత_By;    /* Identifier from "INDEXED BY <zIndex>" clause */
    సమాసం_List *pFuncArg;  /* Arguments to table-valued-function */
    u32 n_అడ్డువరుస;            /* Number of rows in a VALUES clause */
  } u1;
  సంఘం {
    సూచీ *pIBIndex;  /* Index structure corresponding to u1.zIndexedBy */
    CteUse *pCteUse;  /* CTE Usage info when fg.isCte is true */
  } u2;
  సంఘం {
    సమాసం *pOn;        /* fg.isUsing==0 =>  The ON clause of a join */
    IdList *pUsing;   /* fg.isUsing==1 =>  The USING clause of a join */
  } u3;
  సంఘం {
    Schema *pSchema;  /* Schema to which this item is fixed */
    అక్షరం *zDatabase;  /* Name of database holding this table */
    Subquery *pSubq;  /* Description of a subquery */
  } u4;
};

/*
** The OnOrUsing object represents either an ON clause or a USING clause.
** It can never be both at the same time, but it can be neither.
*/
నిర్మాణం OnOrUsing {
  సమాసం *pOn;         /* The ON clause of a join */
  IdList *pUsing;    /* The USING clause of a join */
};

/*
** This object represents one or more tables that are the source of
** content for an SQL statement.  For example, a single SrcList object
** is used to hold the FROM clause of a SELECT statement.  SrcList also
** represents the target tables for DELETE, INSERT, and UPDATE statements.
**
*/
నిర్మాణం SrcList {
  పూర్ణాంకం nSrc;             /* Number of tables or subqueries in the FROM clause */
  u32 nAlloc;           /* Number of entries allocated in a[] below */
  Src_అంశం a[FLEXARRAY]; /* One entry for each identifier on the list */
};

/* Size (in bytes) of a SrcList object that can hold as many as N
** SrcItem objects. */
#నిర్వచించు SZ_SRCLIST(N) (offsetof(SrcList,a)+(N)*పరిమాణం(Src_అంశం))

/* Size (in bytes( of a SrcList object that holds 1 SrcItem.  This is a
** special case of SZ_SRCITEM(1) that comes up often. */
#నిర్వచించు SZ_SRCLIST_1  (offsetof(SrcList,a)+పరిమాణం(Src_అంశం))

/*
** Permitted values of the SrcList.a.jointype field
*/
#నిర్వచించు JT_INNER     0x01    /* Any kind of inner or cross join */
#నిర్వచించు JT_CROSS     0x02    /* Explicit use of the CROSS keyword */
#నిర్వచించు JT_NATURAL   0x04    /* True for a "natural" join */
#నిర్వచించు JT_ఎడమ      0x08    /* Left outer join */
#నిర్వచించు JT_కుడి     0x10    /* Right outer join */
#నిర్వచించు JT_OUTER     0x20    /* The "OUTER" keyword is present */
#నిర్వచించు JT_LTORJ     0x40    /* One of the LEFT operands of a RIGHT JOIN
                             ** Mnemonic: Left Table Of Right Join */
#నిర్వచించు JT_ERROR     0x80    /* unknown or unsupported join type */

/*
** Flags appropriate for the wctrlFlags parameter of sqlite3WhereBegin()
** and the WhereInfo.wctrlFlags member.
**
** Value constraints (enforced via assert()):
**     WHERE_USE_LIMIT  == SF_FixedLimit
*/
#నిర్వచించు WHERE_ORDERBY_సాధారణ   0x0000 /* No-op */
#నిర్వచించు WHERE_ORDERBY_MIN      0x0001 /* ORDER BY processing for min() func */
#నిర్వచించు WHERE_ORDERBY_MAX      0x0002 /* ORDER BY processing for max() func */
#నిర్వచించు WHERE_ONEPASS_DESIRED  0x0004 /* Want to do one-pass UPDATE/DELETE */
#నిర్వచించు WHERE_ONEPASS_MULTIROW 0x0008 /* ONEPASS is ok with multiple rows */
#నిర్వచించు WHERE_DUPLICATES_OK    0x0010 /* Ok to return a row more than once */
#నిర్వచించు WHERE_OR_SUBCLAUSE     0x0020 /* Processing a sub-WHERE as part of
                                      ** the OR optimization  */
#నిర్వచించు WHERE_GROUPBY          0x0040 /* pOrderBy is really a GROUP BY */
#నిర్వచించు WHERE_DISTINCTBY       0x0080 /* pOrderby is really a DISTINCT clause */
#నిర్వచించు WHERE_WANT_DISTINCT    0x0100 /* All output needs to be distinct */
#నిర్వచించు WHERE_SORTBYGROUP      0x0200 /* Support sqlite3WhereIsSorted() */
#నిర్వచించు WHERE_AGG_DISTINCT     0x0400 /* Query is "SELECT agg(DISTINCT ...)" */
#నిర్వచించు WHERE_ORDERBY_హద్దు    0x0800 /* ORDERBY+LIMIT on the inner loop */
#నిర్వచించు WHERE_కుడి_JOIN       0x1000 /* Processing a RIGHT JOIN */
#నిర్వచించు WHERE_KEEP_ALL_JOINS   0x2000 /* Do not do the omit-noop-join opt */
#నిర్వచించు WHERE_USE_హద్దు        0x4000 /* Use the LIMIT in cost estimates */
                        /*     0x8000    not currently used */

/* Allowed return values from sqlite3WhereIsDistinct()
*/
#నిర్వచించు WHERE_DISTINCT_NOOP      0  /* DISTINCT keyword not used */
#నిర్వచించు WHERE_DISTINCT_UNIQUE    1  /* No duplicates */
#నిర్వచించు WHERE_DISTINCT_ORDERED   2  /* All duplicates are adjacent */
#నిర్వచించు WHERE_DISTINCT_UNORDERED 3  /* Duplicates are scattered */

/*
** A NameContext defines a context in which to resolve table and column
** names.  The context consists of a list of tables (the pSrcList) field and
** a list of named expression (pEList).  The named expression list may
** be NULL.  The pSrc corresponds to the FROM clause of a SELECT or
** to the table being operated on by INSERT, UPDATE, or DELETE.  The
** pEList corresponds to the result set of a SELECT and is NULL for
** other statements.
**
** NameContexts can be nested.  When resolving names, the inner-most
** context is searched first.  If no match is found, the next outer
** context is checked.  If there is still no match, the next context
** is checked.  This process continues until either a match is found
** or all contexts are check.  When a match is found, the nRef member of
** the context containing the match is incremented.
**
** Each subquery gets a new NameContext.  The pNext field points to the
** NameContext in the parent query.  Thus the process of scanning the
** NameContext list corresponds to searching through successively outer
** subqueries looking for a match.
*/
నిర్మాణం NameContext {
  Parse *pParse;       /* The parser */
  SrcList *pSrcList;   /* One or more tables used to resolve names */
  సంఘం {
    సమాసం_List *pEList;    /* Optional list of result-set columns */
    AggInfo *pAggInfo;   /* Information about aggregates at this level */
    Upsert *pUpsert;     /* ON CONFLICT clause information from an upsert */
    పూర్ణాంకం i_పునాది_Reg;        /* For TK_REGISTER when parsing RETURNING */
  } uNC;
  NameContext *p_తదుపరి;  /* Next outer name context.  NULL for outermost */
  పూర్ణాంకం nRef;            /* Number of names resolved by this context */
  పూర్ణాంకం nNcErr;          /* Number of errors encountered while resolving names */
  పూర్ణాంకం ncFlags;         /* Zero or more NC_* flags defined below */
  u32 nNestedSelect;   /* Number of nested selects using this NC */
  Select *pWinSelect;  /* SELECT statement for any window functions */
};

/*
** Allowed values for the NameContext, ncFlags field.
**
** Value constraints (all checked via assert()):
**    NC_HasAgg    == SF_HasAgg       == EP_Agg
**    NC_MinMaxAgg == SF_MinMaxAgg    == SQLITE_FUNC_MINMAX
**    NC_OrderAgg  == SF_OrderByReqd  == SQLITE_FUNC_ANYORDER
**    NC_HasWin    == EP_Win
**
*/
#నిర్వచించు NC_AllowAgg  0x000001 /* Aggregate functions are allowed here */
#నిర్వచించు NC_PartIdx   0x000002 /* True if resolving a partial index WHERE */
#నిర్వచించు NC_IsCheck   0x000004 /* True if resolving a CHECK constraint */
#నిర్వచించు NC_Gen_నిలువు    0x000008 /* True for a GENERATED ALWAYS AS clause */
#నిర్వచించు NC_HasAgg    0x000010 /* One or more aggregate functions seen */
#నిర్వచించు NC_Idx_సమాసం   0x000020 /* True if resolving columns of CREATE INDEX */
#నిర్వచించు NC_SelfRef   0x00002e /* Combo: PartIdx, isCheck, GenCol, and IdxExpr */
#నిర్వచించు NC_Subquery  0x000040 /* A subquery has been seen */
#నిర్వచించు NC_UEList    0x000080 /* True if uNC.pEList is used */
#నిర్వచించు NC_UAggInfo  0x000100 /* True if uNC.pAggInfo is used */
#నిర్వచించు NC_UUpsert   0x000200 /* True if uNC.pUpsert is used */
#నిర్వచించు NC_UBaseReg  0x000400 /* True if uNC.iBaseReg is used */
#నిర్వచించు NC_MinMaxAgg 0x001000 /* min/max aggregates seen.  See note above */
/*                   0x002000 // available for reuse */
#నిర్వచించు NC_AllowWin  0x004000 /* Window functions are allowed here */
#నిర్వచించు NC_HasWin    0x008000 /* One or more window functions seen */
#నిర్వచించు NC_IsDDL     0x010000 /* Resolving names in a CREATE statement */
#నిర్వచించు NC_లోపల_Agg_Func 0x020000 /* True if analyzing arguments to an agg func */
#నిర్వచించు NC_FromDDL   0x040000 /* SQL text comes from sqlite_schema */
#నిర్వచించు NC_NoSelect  0x080000 /* Do not descend into sub-selects */
#నిర్వచించు NC_Where     0x100000 /* Processing WHERE clause of a SELECT */
#నిర్వచించు NC_క్రమం_Agg 0x8000000 /* Has an aggregate other than count/min/max */

/*
** An instance of the following object describes a single ON CONFLICT
** clause in an upsert.
**
** The pUpsertTarget field is only set if the ON CONFLICT clause includes
** conflict-target clause.  (In "ON CONFLICT(a,b)" the "(a,b)" is the
** conflict-target clause.)  The pUpsertTargetWhere is the optional
** WHERE clause used to identify partial unique indexes.
**
** pUpsertSet is the list of column=expr terms of the UPDATE statement.
** The pUpsertSet field is NULL for a ON CONFLICT DO NOTHING.  The
** pUpsertWhere is the WHERE clause for the UPDATE and is NULL if the
** WHERE clause is omitted.
*/
నిర్మాణం Upsert {
  సమాసం_List *p_Upsert_లక్ష్యం;  /* Optional description of conflict target */
  సమాసం *p_Upsert_లక్ష్యం_Where; /* WHERE clause for partial index targets */
  సమాసం_List *p_Upsert_సమితి;     /* The SET clause from an ON CONFLICT UPDATE */
  సమాసం *pUpsertWhere;       /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_తదుపరి_Upsert;      /* Next ON CONFLICT clause in the list */
  u8 isDoUpdate;            /* True for DO UPDATE.  False for DO NOTHING */
  u8 isDup;                 /* True if 2nd or later with same pUpsertIdx */
  /* Above this point is the parse tree for the ON CONFLICT clauses.
  ** The next group of fields stores intermediate data. */
  శూన్య *p_గమ్యం_Free;            /* Free memory when deleting the Upsert object */
  /* All fields above are owned by the Upsert object and must be freed
  ** when the Upsert is destroyed.  The fields below are used to transfer
  ** information from the INSERT processing down into the UPDATE processing
  ** while generating code.  The fields below are owned by the INSERT
  ** statement and will be freed by INSERT processing. */
  సూచీ *pUpsertIdx;        /* UNIQUE constraint specified by pUpsertTarget */
  SrcList *pUpsertSrc;      /* Table to be updated */
  పూర్ణాంకం reg_దత్తాంశం;              /* First register holding array of VALUES */
  పూర్ణాంకం i_దత్తాంశం_Cur;             /* Index of the data cursor */
  పూర్ణాంకం iIdxCur;              /* Index of the first index cursor */
};

/*
** An instance of the following structure contains all information
** needed to generate code for a single SELECT statement.
*/
నిర్మాణం Select {
  u8 op;                 /* One of: TK_UNION TK_ALL TK_INTERSECT TK_EXCEPT */
  LogEst n_Select_అడ్డువరుస;     /* Estimated number of result rows */
  u32 selFlags;          /* Various SF_* values */
  పూర్ణాంకం i_హద్దు, i_స్థానభ్రంశం;   /* Memory registers holding LIMIT & OFFSET counters */
  u32 selId;             /* Unique identifier number for this SELECT */
  సమాసం_List *pEList;      /* The fields of the result */
  SrcList *pSrc;         /* The FROM clause */
  సమాసం *pWhere;          /* The WHERE clause */
  సమాసం_List *p_సమూహం_By;    /* The GROUP BY clause */
  సమాసం *pHaving;         /* The HAVING clause */
  సమాసం_List *p_క్రమం_By;    /* The ORDER BY clause */
  Select *p_పూర్వ;        /* Prior select in a compound select statement */
  Select *p_తదుపరి;         /* Next select to the left in a compound */
  సమాసం *p_హద్దు;          /* LIMIT expression. NULL means not used. */
  With *pWith;           /* WITH clause attached to this select. Or NULL. */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
  కిటికీ *pWin;          /* List of window functions */
  కిటికీ *pWinDefn;      /* List of named window definitions */
#అయితే_ముగింపు
};

/*
** Allowed values for Select.selFlags.  The "SF" prefix stands for
** "Select Flag".
**
** Value constraints (all checked via assert())
**     SF_HasAgg      == NC_HasAgg
**     SF_MinMaxAgg   == NC_MinMaxAgg     == SQLITE_FUNC_MINMAX
**     SF_OrderByReqd == NC_OrderAgg      == SQLITE_FUNC_ANYORDER
**     SF_FixedLimit  == WHERE_USE_LIMIT
*/
#నిర్వచించు SF_Distinct      0x0000001 /* Output should be DISTINCT */
#నిర్వచించు SF_All           0x0000002 /* Includes the ALL keyword */
#నిర్వచించు SF_Resolved      0x0000004 /* Identifiers have been resolved */
#నిర్వచించు SF_Aggregate     0x0000008 /* Contains agg functions or a GROUP BY */
#నిర్వచించు SF_HasAgg        0x0000010 /* Contains aggregate functions */
#నిర్వచించు SF_Cloned_Rhs_లోపల   0x0000020 /* Cloned RHS of an IN operator */
#నిర్వచించు SF_Expanded      0x0000040 /* sqlite3SelectExpand() called on this */
#నిర్వచించు SF_HasTypeInfo   0x0000080 /* FROM subqueries have Table metadata */
#నిర్వచించు SF_Compound      0x0000100 /* Part of a compound query */
#నిర్వచించు SF_విలువలు        0x0000200 /* Synthesized from VALUES clause */
#నిర్వచించు SF_Multi_విలువ    0x0000400 /* Single VALUES term with multiple rows */
#నిర్వచించు SF_NestedFrom    0x0000800 /* Part of a parenthesized FROM clause */
#నిర్వచించు SF_MinMaxAgg     0x0001000 /* Aggregate containing min() or max() */
#నిర్వచించు SF_Recursive     0x0002000 /* The recursive part of a recursive CTE */
#నిర్వచించు SF_Fixed_హద్దు    0x0004000 /* nSelectRow set by a constant LIMIT */
/*                       0x0008000 // available for reuse */
#నిర్వచించు SF_Converted     0x0010000 /* By convertCompoundSelectToSubquery() */
#నిర్వచించు SF_IncludeHidden 0x0020000 /* Include hidden columns in output */
#నిర్వచించు SF_Complex_ఫలితం 0x0040000 /* Result contains subquery or function */
#నిర్వచించు SF_WhereBegin    0x0080000 /* Really a WhereBegin() call.  Debug Only */
#నిర్వచించు SF_WinRewrite    0x0100000 /* Window function rewrite accomplished */
#నిర్వచించు SF_View          0x0200000 /* SELECT statement is a view */
/*                       0x0400000 // available for reuse */
#నిర్వచించు SF_UFSrcCheck    0x0800000 /* Check pSrc as required by UPDATE...FROM */
#నిర్వచించు SF_నెట్టు_కిందికి      0x1000000 /* Modified by WHERE-clause push-down opt */
#నిర్వచించు SF_MultiPart     0x2000000 /* Has multiple incompatible PARTITIONs */
#నిర్వచించు SF_CopyCte       0x4000000 /* SELECT statement is a copy of a CTE */
#నిర్వచించు SF_క్రమం_By_Reqd   0x8000000 /* The ORDER BY clause may not be omitted */
#నిర్వచించు SF_UpdateFrom   0x10000000 /* Query originates with UPDATE FROM */
#నిర్వచించు SF_Correlated   0x20000000 /* True if references the outer context */
#నిర్వచించు SF_On_గమ్యం_Where    0x40000000 /* One or more ON clauses moved to WHERE */

/* True if SrcItem X is a subquery that has SF_NestedFrom */
#నిర్వచించు IsNestedFrom(X) \
   ((X)->fg.isSubquery && \
    ((X)->u4.pSubq->pSelect->selFlags&SF_NestedFrom)!=0)

/*
** The results of a SELECT can be distributed in several ways, as defined
** by one of the following macros.  The "SRT" prefix means "SELECT Result
** Type".
**
**     SRT_Exists      Store a 1 in memory cell pDest->iSDParm if the result
**                     set is not empty.
**
**     SRT_Discard     Throw the results away.  This is used by SELECT
**                     statements within triggers whose only purpose is
**                     the side-effects of functions.
**
**     SRT_Output      Generate a row of output (using the OP_ResultRow
**                     opcode) for each row in the result set.
**
**     SRT_Mem         Only valid if the result is a single column.
**                     Store the first column of the first result row
**                     in register pDest->iSDParm then abandon the rest
**                     of the query.  This destination implies "LIMIT 1".
**
**     SRT_Set         The result must be a single column.  Store each
**                     row of result as the key in table pDest->iSDParm.
**                     Apply the affinity pDest->affSdst before storing
**                     results.  if pDest->iSDParm2 is positive, then it is
**                     a register holding a Bloom filter for the IN operator
**                     that should be populated in addition to the
**                     pDest->iSDParm table.  This SRT is used to
**                     implement "IN (SELECT ...)".
**
**     SRT_EphemTab    Create an temporary table pDest->iSDParm and store
**                     the result there. The cursor is left open after
**                     returning.  This is like SRT_Table except that
**                     this destination uses OP_OpenEphemeral to create
**                     the table first.
**
**     SRT_Coroutine   Generate a co-routine that returns a new row of
**                     results each time it is invoked.  The entry point
**                     of the co-routine is stored in register pDest->iSDParm
**                     and the result row is stored in pDest->nDest registers
**                     starting with pDest->iSdst.
**
**     SRT_Table       Store results in temporary table pDest->iSDParm.
**     SRT_Fifo        This is like SRT_EphemTab except that the table
**                     is assumed to already be open.  SRT_Fifo has
**                     the additional property of being able to ignore
**                     the ORDER BY clause.
**
**     SRT_DistFifo    Store results in a temporary table pDest->iSDParm.
**                     But also use temporary table pDest->iSDParm+1 as
**                     a record of all prior results and ignore any duplicate
**                     rows.  Name means:  "Distinct Fifo".
**
**     SRT_Queue       Store results in priority queue pDest->iSDParm (really
**                     an index).  Append a sequence number so that all entries
**                     are distinct.
**
**     SRT_DistQueue   Store results in priority queue pDest->iSDParm only if
**                     the same record has never been stored before.  The
**                     index at pDest->iSDParm+1 hold all prior stores.
**
**     SRT_Upfrom      Store results in the temporary table already opened by
**                     pDest->iSDParm. If (pDest->iSDParm<0), then the temp
**                     table is an intkey table - in this case the first
**                     column returned by the SELECT is used as the integer
**                     key. If (pDest->iSDParm>0), then the table is an index
**                     table. (pDest->iSDParm) is the number of key columns in
**                     each index record in this case.
*/
#నిర్వచించు SRT_Exists       1  /* Store 1 if the result is not empty */
#నిర్వచించు SRT_Discard      2  /* Do not save the results anywhere */
#నిర్వచించు SRT_DistFifo     3  /* Like SRT_Fifo, but unique results only */
#నిర్వచించు SRT_Dist_వరుస    4  /* Like SRT_Queue, but unique results only */

/* The DISTINCT clause is ignored for all of the above.  Not that
** IgnorableDistinct() implies IgnorableOrderby() */
#నిర్వచించు IgnorableDistinct(X) ((X->eDest)<=SRT_Dist_వరుస)

#నిర్వచించు SRT_వరుస        5  /* Store result in an queue */
#నిర్వచించు SRT_Fifo         6  /* Store result as data with an automatic rowid */

/* The ORDER BY clause is ignored for all of the above */
#నిర్వచించు IgnorableOrderby(X) ((X->eDest)<=SRT_Fifo)

#నిర్వచించు SRT_Output       7  /* Output each row of result */
#నిర్వచించు SRT_Mem          8  /* Store result in a memory cell */
#నిర్వచించు SRT_సమితి          9  /* Store results as keys in an index */
#నిర్వచించు SRT_EphemTab    10  /* Create transient tab and store like SRT_Table */
#నిర్వచించు SRT_Coroutine   11  /* Generate a single row of result */
#నిర్వచించు SRT_పట్టిక       12  /* Store result as data with an automatic rowid */
#నిర్వచించు SRT_Upfrom      13  /* Store result as data with rowid */

/*
** An instance of this object describes where to put of the results of
** a SELECT statement.
*/
నిర్మాణం SelectDest {
  u8 eDest;            /* How to dispose of the results.  One of SRT_* above. */
  పూర్ణాంకం iSDParm;         /* A parameter used by the eDest disposal method */
  పూర్ణాంకం iSDParm2;        /* A second parameter for the eDest disposal method */
  పూర్ణాంకం iSdst;           /* Base register where results are written */
  పూర్ణాంకం nSdst;           /* Number of registers allocated */
  అక్షరం *zAffSdst;      /* Affinity used for SRT_Set */
  సమాసం_List *p_క్రమం_By;  /* Key columns for SRT_Queue and SRT_DistQueue */
};

/*
** During code generation of statements that do inserts into AUTOINCREMENT
** tables, the following information is attached to the Table.u.autoInc.p
** pointer of each autoincrement table to record some side information that
** the code generator needs.  We have to keep per-table autoincrement
** information in case inserts are done within triggers.  Triggers do not
** normally coordinate their activities, but we do need to coordinate the
** loading and saving of autoincrement information.
*/
నిర్మాణం AutoincInfo {
  AutoincInfo *p_తదుపరి;   /* Next info block in a list of them all */
  పట్టిక *pTab;          /* Table this info block refers to */
  పూర్ణాంకం iDb;              /* Index in sqlite3.aDb[] of database holding pTab */
  పూర్ణాంకం regCtr;           /* Memory register holding the rowid counter */
};

/*
** At least one instance of the following structure is created for each
** trigger that may be fired while parsing an INSERT, UPDATE or DELETE
** statement. All such objects are stored in the linked list headed at
** Parse.pTriggerPrg and deleted once statement compilation has been
** completed.
**
** A Vdbe sub-program that implements the body and WHEN clause of trigger
** TriggerPrg.pTrigger, assuming a default ON CONFLICT clause of
** TriggerPrg.orconf, is stored in the TriggerPrg.pProgram variable.
** The Parse.pTriggerPrg list never contains two entries with the same
** values for both pTrigger and orconf.
**
** The TriggerPrg.aColmask[0] variable is set to a mask of old.* columns
** accessed (or set to 0 for triggers fired as a result of INSERT
** statements). Similarly, the TriggerPrg.aColmask[1] variable is set to
** a mask of new.* columns used by the program.
*/
నిర్మాణం TriggerPrg {
  Trigger *pTrigger;      /* Trigger this program was coded from */
  TriggerPrg *p_తదుపరి;      /* Next entry in Parse.pTriggerPrg list */
  Sub_కార్యక్రమం *p_కార్యక్రమం;   /* Program implementing pTrigger/orconf */
  పూర్ణాంకం orconf;             /* Default ON CONFLICT policy */
  u32 aColmask[2];        /* Masks of old.*, new.* columns accessed */
};

/*
** The yDbMask datatype for the bitmask of all attached databases.
*/
#ఒకవేళ SQLITE_MAX_ATTACHED>30
  రకనిర్వచనం గుర్తులేని అక్షరం y_Db_ముసుగు[(SQLITE_MAX_ATTACHED+9)/8];
# define Db_ముసుగు_Test(M,I)    (((M)[(I)/8]&(1<<((I)&7)))!=0)
# define Db_ముసుగు_Zero(M)      memset((M),0,పరిమాణం(M))
# define Db_ముసుగు_సమితి(M,I)     (M)[(I)/8]|=(1<<((I)&7))
# define Db_ముసుగు_All_Zero(M)   sqlite3_Db_ముసుగు_All_Zero(M)
# define Db_ముసుగు_Non_Zero(M)   (sqlite3_Db_ముసుగు_All_Zero(M)==0)
#లేకపోతే
  రకనిర్వచనం గుర్తులేని పూర్ణాంకం y_Db_ముసుగు;
# define Db_ముసుగు_Test(M,I)    (((M)&(((y_Db_ముసుగు)1)<<(I)))!=0)
# define Db_ముసుగు_Zero(M)      ((M)=0)
# define Db_ముసుగు_సమితి(M,I)     ((M)|=(((y_Db_ముసుగు)1)<<(I)))
# define Db_ముసుగు_All_Zero(M)   ((M)==0)
# define Db_ముసుగు_Non_Zero(M)   ((M)!=0)
#అయితే_ముగింపు

/*
** For each index X that has as one of its arguments either an expression
** or the name of a virtual generated column, and if X is in scope such that
** the value of the expression can simply be read from the index, then
** there is an instance of this object on the Parse.pIdxExpr list.
**
** During code generation, while generating code to evaluate expressions,
** this list is consulted and if a matching expression is found, the value
** is read from the index rather than being recomputed.
*/
నిర్మాణం సూచీకృత_సమాసం {
  సమాసం *p_సమాసం;            /* The expression contained in the index */
  పూర్ణాంకం i_దత్తాంశం_Cur;           /* The data cursor associated with the index */
  పూర్ణాంకం iIdxCur;            /* The index cursor */
  పూర్ణాంకం i_Idx_నిలువు;            /* The index column that contains value of pExpr */
  u8 b_Maybe_Null_అడ్డువరుస;       /* True if we need an OP_IfNullRow check */
  u8 aff;                 /* Affinity of the pExpr expression */
  సూచీకృత_సమాసం *pIENext;   /* Next in a list of all indexed expressions */
#నిర్వచితమైతే SQLITE_ENABLE_EXPLAIN_COMMENTS
  స్థిరరాశి అక్షరం *zIdxName;   /* Name of index, used only for bytecode comments */
#అయితే_ముగింపు
};

/*
** An instance of the ParseCleanup object specifies an operation that
** should be performed after parsing to deallocation resources obtained
** during the parse and which are no longer needed.
*/
నిర్మాణం ParseCleanup {
  ParseCleanup *p_తదుపరి;               /* Next cleanup task */
  శూన్య *pPtr;                        /* Pointer to object to deallocate */
  శూన్య (*xCleanup)(sqlite3*,శూన్య*);  /* Deallocation routine */
};

/*
** Number of 64-bit entried in the variable number bitmap cache (VNBMC).
*/
#నిర్వచితం_కాకపోతే SQLITE_VNBMC
# define SQLITE_VNBMC 2
#అయితే_ముగింపు

/*
** An SQL parser context.  A copy of this structure is passed through
** the parser and down into all the parser action routine in order to
** carry around information that is global to the entire parse.
**
** The structure is divided into two parts.  When the parser and code
** generate call themselves recursively, the first part of the structure
** is constant but the second part is reset at the beginning and end of
** each recursion.
**
** The nTableLock and aTableLock variables are only used if the shared-cache
** feature is enabled (if sqlite3Tsd()->useSharedData is true). They are
** used to store the set of table-locks required by the statement being
** compiled. Function sqlite3TableLock() is used to add entries to the
** list.
*/
నిర్మాణం Parse {
  sqlite3 *db;         /* The main database structure */
  అక్షరం *z_Err_సందేశం;       /* An error message */
  Vdbe *pVdbe;         /* An engine for executing database bytecode */
  పూర్ణాంకం rc;              /* Return code from execution */
  LogEst nQueryLoop;   /* Est number of iterations of a query (10*log2(N)) */
  u8 nested;           /* Number of nested calls to the parser/code generator */
  u8 n_తాత్కాలిక_Reg;         /* Number of temporary registers in aTempReg[] */
  u8 isMultiWrite;     /* True if statement may modify/insert multiple rows */
  u8 disableLookaside; /* Number of times lookaside has been disabled */
  u8 prepFlags;        /* SQLITE_PREPARE_* flags */
  u8 withinRJSubrtn;   /* Nesting level for RIGHT JOIN body subroutines */
  u8 mSubrtnSig;       /* mini Bloom filter on available SubrtnSig.selId */
  u8 eTriggerOp;       /* TK_UPDATE, TK_INSERT or TK_DELETE */
  u8 eOrconf;          /* Default ON CONFLICT policy for trigger steps */
#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
  u8 earlyCleanup;     /* OOM inside sqlite3ParserAddCleanup() */
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_DEBUG
  u8 ifNotExists;      /* Might be true if IF NOT EXISTS.  Assert()s only */
  u8 isCreate;         /* CREATE TABLE, INDEX, or VIEW (but not TRIGGER)
                       ** and ALTER TABLE ADD COLUMN. */
#అయితే_ముగింపు
  bft disableTriggers:1; /* True to disable triggers */
  bft mayAbort :1;     /* True if statement may throw an ABORT exception */
  bft hasCompound :1;  /* Need to invoke convertCompoundSelectToSubquery() */
  bft bReturning :1;   /* Coding a RETURNING trigger */
  bft bHasExists :1;   /* Has a correlated "EXISTS (SELECT ....)" expression */
  bft నిలువు_పేర్లు_సమితి :1;  /* TRUE after OP_ColumnName has been issued to pVdbe */
  bft bHasWith :1;     /* True if statement contains WITH */
  bft ok_Const_గుణకం:1; /* OK to factor out constants */
  bft checkSchema :1;  /* Causes schema cookie check after an error */
  bft ఉపయోగిస్తుంది_Ainc :1;     /* True if pAinc is valid */
  పూర్ణాంకం nRangeReg;       /* Size of the temporary register block */
  పూర్ణాంకం iRangeReg;       /* First register in temporary register block */
  పూర్ణాంకం nErr;            /* Number of errors seen */
  పూర్ణాంకం nTab;            /* Number of previously allocated VDBE cursors */
  పూర్ణాంకం nMem;            /* Number of memory cells used so far */
  పూర్ణాంకం szOpAlloc;       /* Bytes of memory space allocated for Vdbe.aOp[] */
  పూర్ణాంకం iSelfTab;        /* Table associated with an index on expr, or negative
                       ** of the base register during check-constraint eval */
  పూర్ణాంకం nNestSel;        /* Number of nested SELECT statements and/or VIEWs */
  పూర్ణాంకం nLabel;          /* The *negative* of the number of labels used */
  పూర్ణాంకం nLabelAlloc;     /* Number of slots in aLabel */
  పూర్ణాంకం *aLabel;         /* Space to hold the labels */
  సమాసం_List *p_Const_సమాసం;/* Constant expressions */
  సూచీకృత_సమాసం *pIdxEpr;/* List of expressions used by active indexes */
  సూచీకృత_సమాసం *p_Idx_Part_సమాసం; /* Exprs constrained by index WHERE clauses */
  y_Db_ముసుగు write_ముసుగు;   /* Start a write transaction on these databases */
  y_Db_ముసుగు cookie_ముసుగు;  /* Bitmask of schema verified databases */
  పూర్ణాంకం nMaxArg;         /* Max args to xUpdate and xFilter vtab methods */
  పూర్ణాంకం nSelect;         /* Number of SELECT stmts. Counter for Select.selId */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_పురోగతి_CALLBACK
  u32 n_పురోగతి_అడుగులు;  /* xProgress steps taken during sqlite3_prepare() */
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
  పూర్ణాంకం n_పట్టిక_తాళం;        /* Number of locks in aTableLock */
#అయితే_ముగింపు
  Parse *pToplevel;    /* Parse structure for main program (or NULL) */
  పట్టిక *pTriggerTab;  /* Table triggers are being coded for */
  TriggerPrg *pTriggerPrg;  /* Linked list of coded triggers */
  ParseCleanup *pCleanup;   /* List of cleanup operations to run after parse */
  Trigger *pTrigCoded;      /* Trigger whose sub-program is being coded */

  /**************************************************************************
  ** Fields above must be initialized to zero.  The fields that follow,
  ** down to the beginning of the recursive section, do not need to be
  ** initialized as they will be set before being used.  The boundary is
  ** determined by offsetof(Parse,aTempReg).
  **************************************************************************/

  పూర్ణాంకం a_తాత్కాలిక_Reg[8];        /* Holding area for temporary registers */
  Parse *pOuterParse;     /* Outer Parse object when nested */
  Token sNameToken;       /* Token with unqualified schema object name */
  u32 oldmask;            /* Mask of old.* columns referenced */
  u32 newmask;            /* Mask of new.* columns referenced */
  సంఘం {
    నిర్మాణం {  /* These fields available when isCreate is true */
      పూర్ణాంకం addrCrTab;        /* Address of OP_CreateBtree on CREATE TABLE */
      పూర్ణాంకం regRowid;         /* Register holding rowid of CREATE TABLE entry */
      పూర్ణాంకం reg_మూలం;          /* Register holding root page for new objects */
      Token constraintName; /* Name of the constraint currently being parsed */
    } cr;
    నిర్మాణం {  /* These fields available to all other statements */
      Returning *pReturning; /* The RETURNING clause */
    } d;
  } u1;
  AutoincInfo *pAinc;     /* Information about AUTOINCREMENT counters. Only
                          ** valid if Parse.usesAinc is true */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
  పట్టిక_తాళం *a_పట్టిక_తాళం;  /* Required table locks for shared-cache mode. Only
                          ** valid if Parse.nTableLock>0 */
#అయితే_ముగింపు


  /************************************************************************
  ** Above is constant between recursions.  Below is reset before and after
  ** each recursion.  The boundary between these two regions is determined
  ** using offsetof(Parse,sLastToken) so the sLastToken field must be the
  ** first field in the recursive region.
  ************************************************************************/

  Token s_చివరి_Token;       /* The last token parsed */
  ynVar nVar;               /* Number of '?' variables seen in the SQL so far */
  u64 aVnbmc[SQLITE_VNBMC]; /* Varible Number Bitmap Cache */
  u8 i_Pk_క్రమీకరించు_క్రమం;          /* ASC or DESC for INTEGER PRIMARY KEY */
  u8 explain;               /* True if the EXPLAIN flag is found on the query */
  u8 e_Parse_బహుళకం;            /* PARSE_MODE_XXX constant */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  పూర్ణాంకం n_Vtab_తాళం;            /* Number of virtual tables to lock */
#అయితే_ముగింపు
  పూర్ణాంకం n_ఎత్తు;              /* Expression tree height of current sub-select */
  పూర్ణాంకం addrExplain;          /* Address of current OP_Explain opcode */
  VList *pVList;            /* Mapping between variable names and numbers */
  Vdbe *pReprepare;         /* VM being reprepared (sqlite3Reprepare()) */
  స్థిరరాశి అక్షరం *z_తోక;        /* All SQL text past the last semicolon parsed */
  పట్టిక *p_New_పట్టిక;         /* A table being constructed by CREATE TABLE */
  సూచీ *p_New_సూచీ;         /* An index being constructed by CREATE INDEX.
                            ** Also used to hold redundant UNIQUE constraints
                            ** during a RENAME COLUMN */
  Trigger *pNewTrigger;     /* Trigger under construct by a CREATE TRIGGER */
  స్థిరరాశి అక్షరం *zAuthContext; /* The 6th parameter to db->xAuth callbacks */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  Token sArg;               /* Complete text of a module argument */
  పట్టిక **ap_Vtab_తాళం;       /* Pointer to virtual tables needing locking */
#అయితే_ముగింపు
  With *pWith;              /* Current WITH clause, or NULL */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ALTERTABLE
  RenameToken *pRename;     /* Tokens subject to renaming by ALTER TABLE */
#అయితే_ముగింపు
};

/* Allowed values for Parse.eParseMode
*/
#నిర్వచించు PARSE_బహుళకం_సాధారణ        0
#నిర్వచించు PARSE_బహుళకం_DECLARE_VTAB  1
#నిర్వచించు PARSE_బహుళకం_RENAME        2
#నిర్వచించు PARSE_బహుళకం_UNMAP         3

/*
** Sizes and pointers of various parts of the Parse object.
*/
#నిర్వచించు PARSE_HDR(X)  (((అక్షరం*)(X))+offsetof(Parse,z_Err_సందేశం))
#నిర్వచించు PARSE_HDR_SZ (offsetof(Parse,a_తాత్కాలిక_Reg)-offsetof(Parse,z_Err_సందేశం)) /* Recursive part w/o aColCache*/
#నిర్వచించు PARSE_RECURSE_SZ offsetof(Parse,s_చివరి_Token)    /* Recursive part */
#నిర్వచించు PARSE_తోక_SZ (పరిమాణం(Parse)-PARSE_RECURSE_SZ) /* Non-recursive part */
#నిర్వచించు PARSE_తోక(X) (((అక్షరం*)(X))+PARSE_RECURSE_SZ)  /* Pointer to tail */

/*
** Return true if currently inside an sqlite3_declare_vtab() call.
*/
#నిర్వచితమైతే SQLITE_OMIT_VIRTUALTABLE
  #నిర్వచించు లోపల_DECLARE_VTAB 0
#లేకపోతే
  #నిర్వచించు లోపల_DECLARE_VTAB (pParse->e_Parse_బహుళకం==PARSE_బహుళకం_DECLARE_VTAB)
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_OMIT_ALTERTABLE)
  #నిర్వచించు లోపల_RENAME_OBJECT 0
#లేకపోతే
  #నిర్వచించు లోపల_RENAME_OBJECT (pParse->e_Parse_బహుళకం>=PARSE_బహుళకం_RENAME)
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_OMIT_ALTERTABLE)
  #నిర్వచించు లోపల_SPECIAL_PARSE 0
#లేకపోతే
  #నిర్వచించు లోపల_SPECIAL_PARSE (pParse->e_Parse_బహుళకం!=PARSE_బహుళకం_సాధారణ)
#అయితే_ముగింపు

/*
** An instance of the following structure can be declared on a stack and used
** to save the Parse.zAuthContext value so that it can be restored later.
*/
నిర్మాణం AuthContext {
  స్థిరరాశి అక్షరం *zAuthContext;   /* Put saved Parse.zAuthContext here */
  Parse *pParse;              /* The Parse structure */
};

/*
** Bitfield flags for P5 value in various opcodes.
**
** Value constraints (enforced via assert()):
**    OPFLAG_LENGTHARG    == SQLITE_FUNC_LENGTH
**    OPFLAG_TYPEOFARG    == SQLITE_FUNC_TYPEOF
**    OPFLAG_BULKCSR      == BTREE_BULKLOAD
**    OPFLAG_SEEKEQ       == BTREE_SEEK_EQ
**    OPFLAG_FORDELETE    == BTREE_FORDELETE
**    OPFLAG_SAVEPOSITION == BTREE_SAVEPOSITION
**    OPFLAG_AUXDELETE    == BTREE_AUXDELETE
*/
#నిర్వచించు OPFLAG_NCHANGE       0x01    /* OP_Insert: Set to update db->nChange */
                                     /* Also used in P2 (not P5) of OP_Delete */
#నిర్వచించు OPFLAG_NOCHNG        0x01    /* OP_VColumn nochange for UPDATE */
#నిర్వచించు OPFLAG_EPHEM         0x01    /* OP_Column: Ephemeral output is ok */
#నిర్వచించు OPFLAG_LASTROWID     0x20    /* Set to update db->lastRowid */
#నిర్వచించు OPFLAG_ISUPDATE      0x04    /* This OP_Insert is an sql UPDATE */
#నిర్వచించు OPFLAG_APPEND        0x08    /* This is likely to be an append */
#నిర్వచించు OPFLAG_USESEEKRESULT 0x10    /* Try to avoid a seek in BtreeInsert() */
#నిర్వచించు OPFLAG_ISNOOP        0x40    /* OP_Delete does pre-update-hook only */
#నిర్వచించు OPFLAG_LENGTHARG     0x40    /* OP_Column only used for length() */
#నిర్వచించు OPFLAG_TYPEOFARG     0x80    /* OP_Column only used for typeof() */
#నిర్వచించు OPFLAG_BYTELENARG    0xc0    /* OP_Column only for octet_length() */
#నిర్వచించు OPFLAG_BULKCSR       0x01    /* OP_Open** used to open bulk cursor */
#నిర్వచించు OPFLAG_SEEKEQ        0x02    /* OP_Open** cursor uses EQ seek only */
#నిర్వచించు OPFLAG_FORDELETE     0x08    /* OP_Open should use BTREE_FORDELETE */
#నిర్వచించు OPFLAG_P2ISREG       0x10    /* P2 to OP_Open** is a register number */
#నిర్వచించు OPFLAG_క్రమమార్పు       0x01    /* OP_Compare: use the permutation */
#నిర్వచించు OPFLAG_SAVEPOSITION  0x02    /* OP_Delete/Insert: save cursor pos */
#నిర్వచించు OPFLAG_AUXDELETE     0x04    /* OP_Delete: index in a DELETE op */
#నిర్వచించు OPFLAG_NOCHNG_MAGIC  0x6d    /* OP_MakeRecord: serialtype 10 is ok */
#నిర్వచించు OPFLAG_PREFORMAT     0x80    /* OP_Insert uses preformatted cell */

/*
** Each trigger present in the database schema is stored as an instance of
** struct Trigger.
**
** Pointers to instances of struct Trigger are stored in two ways.
** 1. In the "trigHash" hash table (part of the sqlite3* that represents the
**    database). This allows Trigger structures to be retrieved by name.
** 2. All triggers associated with a single table form a linked list, using the
**    pNext member of struct Trigger. A pointer to the first element of the
**    linked list is stored as the "pTrigger" member of the associated
**    struct Table.
**
** The "step_list" member points to the first element of a linked list
** containing the SQL statements specified as the trigger program.
*/
నిర్మాణం Trigger {
  అక్షరం *zName;            /* The name of the trigger                        */
  అక్షరం *పట్టిక_2;            /* The table or view to which the trigger applies */
  u8 op;                  /* One of TK_DELETE, TK_UPDATE, TK_INSERT         */
  u8 tr_tm;               /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  u8 bReturning;          /* This trigger implements a RETURNING clause */
  సమాసం *pWhen;            /* The WHEN clause of the expression (may be NULL) */
  IdList *pColumns;       /* If this is an UPDATE OF <column-list> trigger,
                             the <column-list> is stored here */
  Schema *pSchema;        /* Schema containing the trigger */
  Schema *pTabSchema;     /* Schema containing the table */
  Trigger_అడుగు *అడుగు_list; /* Link list of trigger program steps             */
  Trigger *p_తదుపరి;         /* Next trigger associated with the table */
};

/*
** A trigger is either a BEFORE or an AFTER trigger.  The following constants
** determine which.
**
** If there are multiple triggers, you might of some BEFORE and some AFTER.
** In that cases, the constants below can be ORed together.
*/
#నిర్వచించు TRIGGER_BEFORE  1
#నిర్వచించు TRIGGER_AFTER   2

/*
** An instance of struct TriggerStep is used to store a single SQL statement
** that is a part of a trigger-program.
**
** Instances of struct TriggerStep are stored in a singly linked list (linked
** using the "pNext" member) referenced by the "step_list" member of the
** associated struct Trigger instance. The first element of the linked list is
** the first step of the trigger-program.
**
** The "op" member indicates whether this is a "DELETE", "INSERT", "UPDATE" or
** "SELECT" statement. The meanings of the other members is determined by the
** value of "op" as follows:
**
** (op == TK_INSERT)
** orconf    -> stores the ON CONFLICT algorithm
** pSelect   -> The content to be inserted - either a SELECT statement or
**              a VALUES clause.
** pSrc      -> Table to insert into.
** pIdList   -> If this is an INSERT INTO ... (<column-names>) VALUES ...
**              statement, then this stores the column-names to be
**              inserted into.
** pUpsert   -> The ON CONFLICT clauses for an Upsert
**
** (op == TK_DELETE)
** pSrc      -> Table to delete from
** pWhere    -> The WHERE clause of the DELETE statement if one is specified.
**              Otherwise NULL.
**
** (op == TK_UPDATE)
** pSrc      -> Table to update, followed by any FROM clause tables.
** pWhere    -> The WHERE clause of the UPDATE statement if one is specified.
**              Otherwise NULL.
** pExprList -> A list of the columns to update and the expressions to update
**              them to. See sqlite3Update() documentation of "pChanges"
**              argument.
**
** (op == TK_SELECT)
** pSelect   -> The SELECT statement
**
** (op == TK_RETURNING)
** pExprList -> The list of expressions that follow the RETURNING keyword.
**
*/
నిర్మాణం Trigger_అడుగు {
  u8 op;               /* One of TK_DELETE, TK_UPDATE, TK_INSERT, TK_SELECT,
                       ** or TK_RETURNING */
  u8 orconf;           /* OE_Rollback etc. */
  Trigger *pTrig;      /* The trigger that this step is a part of */
  Select *pSelect;     /* SELECT statement or RHS of INSERT INTO SELECT ... */
  SrcList *pSrc;       /* Table to insert/update/delete */
  సమాసం *pWhere;        /* The WHERE clause for DELETE or UPDATE steps */
  సమాసం_List *p_సమాసం_List; /* SET clause for UPDATE, or RETURNING clause */
  IdList *pIdList;     /* Column names for INSERT */
  Upsert *pUpsert;     /* Upsert clauses on an INSERT */
  అక్షరం *z_పరిధి;         /* Original SQL text of this command */
  Trigger_అడుగు *p_తదుపరి;  /* Next in the link-list */
  Trigger_అడుగు *p_చివరి;  /* Last element in link-list. Valid for 1st elem only */
};

/*
** Information about a RETURNING clause
*/
నిర్మాణం Returning {
  Parse *pParse;        /* The parse that includes the RETURNING clause */
  సమాసం_List *pReturnEL;  /* List of expressions to return */
  Trigger retTrig;      /* The transient trigger that implements RETURNING */
  Trigger_అడుగు retTStep; /* The trigger step */
  పూర్ణాంకం iRetCur;          /* Transient table holding RETURNING results */
  పూర్ణాంకం n_Ret_నిలువు;          /* Number of in pReturnEL after expansion */
  పూర్ణాంకం iRetReg;          /* Register array for holding a row of RETURNING */
  అక్షరం zName[40];       /* Name of trigger: "sqlite_returning_%p" */
};

/*
** An object used to accumulate the text of a string where we
** do not necessarily know how big the string will be in the end.
*/
నిర్మాణం sqlite3_str {
  sqlite3 *db;         /* Optional database for lookaside.  Can be NULL */
  అక్షరం *z_పాఠ్యం;         /* The string collected so far */
  u32  nAlloc;         /* Amount of space allocated in zText */
  u32  mxAlloc;        /* Maximum allowed allocation.  0 for no malloc usage */
  u32  nChar;          /* Length of the string so far */
  u8   accError;       /* SQLITE_NOMEM or SQLITE_TOOBIG */
  u8   printfFlags;    /* SQLITE_PRINTF flags below */
};
#నిర్వచించు SQLITE_PRINTF_అంతర్గత 0x01  /* Internal-use-only converters allowed */
#నిర్వచించు SQLITE_PRINTF_SQLFUNC  0x02  /* SQL function arguments to VXPrintf */
#నిర్వచించు SQLITE_PRINTF_MALLOCED 0x04  /* True if zText is allocated space */

#నిర్వచించు isMalloced(X)  (((X)->printfFlags & SQLITE_PRINTF_MALLOCED)!=0)

/*
** The following object is the header for an "RCStr" or "reference-counted
** string".  An RCStr is passed around and used like any other char*
** that has been dynamically allocated.  The important interface
** differences:
**
**   1.  RCStr strings are reference counted.  They are deallocated
**       when the reference count reaches zero.
**
**   2.  Use sqlite3RCStrUnref() to free an RCStr string rather than
**       sqlite3_free()
**
**   3.  Make a (read-only) copy of a read-only RCStr string using
**       sqlite3RCStrRef().
**
** "String" is in the name, but an RCStr object can also be used to hold
** binary data.
*/
నిర్మాణం RCStr {
  u64 nRCRef;            /* Number of references */
  /* Total structure size should be a multiple of 8 bytes for alignment */
};

/*
** A pointer to this structure is used to communicate information
** from sqlite3Init and OP_ParseSchema into the sqlite3InitCallback.
*/
రకనిర్వచనం నిర్మాణం {
  sqlite3 *db;        /* The database being initialized */
  అక్షరం **pz_Err_సందేశం;    /* Error message stored here */
  పూర్ణాంకం iDb;            /* 0 for main database.  1 for TEMP, 2.. for ATTACHed */
  పూర్ణాంకం rc;             /* Result code stored here */
  u32 mInitFlags;     /* Flags controlling error messages */
  u32 n_Init_అడ్డువరుస;       /* Number of rows processed */
  Pgno mx_పుట;        /* Maximum page number.  0 for no limit. */
} Init_దత్తాంశం;

/*
** Allowed values for mInitFlags
*/
#నిర్వచించు INITFLAG_Alter_ముసుగు     0x0007  /* Types of ALTER */
#నిర్వచించు INITFLAG_AlterRename   0x0001  /* Reparse after a RENAME */
#నిర్వచించు INITFLAG_AlterDrop     0x0002  /* Reparse after a DROP COLUMN */
#నిర్వచించు INITFLAG_Alter_కూడు      0x0003  /* Reparse after an ADD COLUMN */
#నిర్వచించు INITFLAG_AlterDropCons 0x0004  /* Reparse after a DROP CONSTRAINT */

/* Tuning parameters are set using SQLITE_TESTCTRL_TUNE and are controlled
** on debug-builds of the CLI using ".testctrl tune ID VALUE".  Tuning
** parameters are for temporary use during development, to help find
** optimal values for parameters in the query planner.  The should not
** be used on trunk check-ins.  They are a temporary mechanism available
** for transient development builds only.
**
** Tuning parameters are numbered starting with 1.
*/
#నిర్వచించు SQLITE_NTUNE  6             /* Should be zero for all trunk check-ins */
#నిర్వచితమైతే SQLITE_DEBUG
# define Tuning(X)  (sqlite3Config.aTune[(X)-1])
#లేకపోతే
# define Tuning(X)  0
#అయితే_ముగింపు

/*
** Structure containing global configuration data for the SQLite library.
**
** This structure also contains some state information.
*/
నిర్మాణం Sqlite3Config {
  పూర్ణాంకం bMemstat;                     /* True to enable memory status */
  u8 bCoreMutex;                    /* True to enable core mutexing */
  u8 b_నిండిన_Mutex;                    /* True to enable full mutexing */
  u8 bOpenUri;                      /* True to interpret filenames as URIs */
  u8 bUseCis;                       /* Use covering indices for full-scans */
  u8 bSmallMalloc;                  /* Avoid large memory allocations if true */
  u8 bExtraSchemaChecks;            /* Verify type,name,tbl_name in schema */
#నిర్వచితమైతే SQLITE_DEBUG
  u8 bJsonSelfcheck;                /* Double-check JSON parsing */
#అయితే_ముగింపు
  పూర్ణాంకం mxStrlen;                     /* Maximum string length */
  పూర్ణాంకం neverCorrupt;                 /* Database is always well-formed */
  పూర్ణాంకం szLookaside;                  /* Default lookaside buffer size */
  పూర్ణాంకం nLookaside;                   /* Default lookaside buffer count */
  పూర్ణాంకం nStmtSpill;                   /* Stmt-journal spill-to-disk threshold */
  sqlite3_mem_methods m;            /* Low-level memory allocation interface */
  sqlite3_mutex_methods mutex;      /* Low-level mutex interface */
  sqlite3_pcache_methods2 pcache2;  /* Low-level page-cache interface */
  శూన్య *p_కుప్ప;                      /* Heap storage space */
  పూర్ణాంకం n_కుప్ప;                        /* Size of pHeap[] */
  పూర్ణాంకం mnReq, mxReq;                 /* Min and max heap requests sizes */
  sqlite3_int64 szMmap;             /* mmap() space per open file */
  sqlite3_int64 mxMmap;             /* Maximum value for szMmap */
  శూన్య *p_పుట;                      /* Page cache memory */
  పూర్ణాంకం sz_పుట;                       /* Size of each page in pPage[] */
  పూర్ణాంకం n_పుట;                        /* Number of pages in pPage[] */
  పూర్ణాంకం mx_Parser_దొంతర;                /* maximum depth of the parser stack */
  పూర్ణాంకం sharedCacheEnabled;           /* true if shared-cache mode enabled */
  u32 szPma;                        /* Maximum Sorter PMA size */
  /* The above might be initialized to non-zero.  The following need to always
  ** initially be zero, however. */
  పూర్ణాంకం isInit;                       /* True after initialization has finished */
  పూర్ణాంకం లోపల_పురోగతి;                   /* True while initialization in progress */
  పూర్ణాంకం isMutexInit;                  /* True after mutexes are initialized */
  పూర్ణాంకం isMallocInit;                 /* True after malloc is initialized */
  పూర్ణాంకం isPCacheInit;                 /* True after malloc is initialized */
  పూర్ణాంకం nRefInitMutex;                /* Number of users of pInitMutex */
  sqlite3_mutex *pInitMutex;        /* Mutex used by sqlite3_initialize() */
  శూన్య (*xLog)(శూన్య*,పూర్ణాంకం,స్థిరరాశి అక్షరం*); /* Function for logging */
  శూన్య *pLogArg;                       /* First argument to xLog() */
#నిర్వచితమైతే SQLITE_ENABLE_SQLLOG
  శూన్య(*xSqllog)(శూన్య*,sqlite3*,స్థిరరాశి అక్షరం*, పూర్ణాంకం);
  శూన్య *pSqllogArg;
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_VDBE_COVERAGE
  /* The following callback (if not NULL) is invoked on every VDBE branch
  ** operation.  Set the callback using SQLITE_TESTCTRL_VDBE_COVERAGE.
  */
  శూన్య (*xVdbeBranch)(శూన్య*,గుర్తులేని i_Src_పంక్తి,u8 eThis,u8 eMx);  /* Callback */
  శూన్య *pVdbeBranchArg;                                     /* 1st argument */
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DESERIALIZE
  sqlite3_int64 mx_Memdb_కొలత;        /* Default max memdb size */
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_UNTESTABLE
  పూర్ణాంకం (*xTestCallback)(పూర్ణాంకం);        /* Invoked by sqlite3FaultSim() */
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ALLOW_ROWID_లోపల_VIEW
  u32 mNoVisibleRowid;              /* TF_NoVisibleRowid if the ROWID_IN_VIEW
                                    ** feature is disabled.  0 if rowids can
                                    ** occur in views. */
#అయితే_ముగింపు
  పూర్ణాంకం bLocaltimeFault;              /* True to fail localtime() calls */
  పూర్ణాంకం (*xAltLocaltime)(స్థిరరాశి శూన్య*,శూన్య*); /* Alternative localtime() routine */
  పూర్ణాంకం i_Once_Reset_పరిమితి;          /* When to reset OP_Once counters */
  u32 szSorterRef;                  /* Min size in bytes to use sorter-refs */
  గుర్తులేని పూర్ణాంకం i_Prng_విత్తనం;           /* Alternative fixed seed for the PRNG */
  /* vvvv--- must be last ---vvv */
#నిర్వచితమైతే SQLITE_DEBUG
  sqlite3_int64 aTune[SQLITE_NTUNE]; /* Tuning parameters */
#అయితే_ముగింపు
};

/*
** This macro is used inside of assert() statements to indicate that
** the assert is only valid on a well-formed database.  Instead of:
**
**     assert( X );
**
** One writes:
**
**     assert( X || CORRUPT_DB );
**
** CORRUPT_DB is true during normal operation.  CORRUPT_DB does not indicate
** that the database is definitely corrupt, only that it might be corrupt.
** For most test cases, CORRUPT_DB is set to false using a special
** sqlite3_test_control().  This enables assert() statements to prove
** things that are always true for well-formed databases.
*/
#నిర్వచించు CORRUPT_DB  (sqlite3Config.neverCorrupt==0)

/*
** Context pointer passed down through the tree-walk.
*/
నిర్మాణం Walker {
  Parse *pParse;                            /* Parser context.  */
  పూర్ణాంకం (*x_సమాసం_Callback)(Walker*, సమాసం*);     /* Callback for expressions */
  పూర్ణాంకం (*xSelectCallback)(Walker*,Select*);  /* Callback for SELECTs */
  శూన్య (*xSelectCallback2)(Walker*,Select*);/* Second callback for SELECTs */
  పూర్ణాంకం walker_లోతు;                          /* Number of subqueries */
  u16 e_సంకేతం;                                /* A small processing code */
  u16 mWFlags;                              /* Use-dependent flags */
  సంఘం {                                   /* Extra data for callback */
    NameContext *pNC;                         /* Naming context */
    పూర్ణాంకం n;                                    /* A counter */
    పూర్ణాంకం iCur;                                 /* A cursor number */
    పూర్ణాంకం sz;                                   /* String literal length */
    SrcList *pSrcList;                        /* FROM clause */
    నిర్మాణం CCurHint *pCCurHint;               /* Used by codeCursorHint() */
    నిర్మాణం RefSrcList *pRefSrcList;           /* sqlite3ReferencesSrcList() */
    పూర్ణాంకం *ai_నిలువు;                               /* array of column indexes */
    నిర్మాణం IdxCover *pIdxCover;               /* Check for index coverage */
    సమాసం_List *p_సమూహం_By;                       /* GROUP BY clause */
    Select *pSelect;                          /* HAVING to WHERE clause ctx */
    నిర్మాణం కిటికీ_Rewrite *pRewrite;           /* Window rewrite context */
    నిర్మాణం WhereConst *pConst;                /* WHERE clause constants */
    నిర్మాణం RenameCtx *pRename;                /* RENAME COLUMN context */
    నిర్మాణం పట్టిక *pTab;                       /* Table of generated column */
    నిర్మాణం Covering_సూచీ_Check *pCovIdxCk;     /* Check for covering index */
    Src_అంశం *p_Src_అంశం;                        /* A single FROM clause item */
    DbFixer *pFix;                            /* See sqlite3FixSelect() */
    Mem *aMem;                                /* See sqlite3BtreeCursorHint() */
    నిర్మాణం CheckOnCtx *pCheckOnCtx;           /* See selectCheckOnClauses() */
  } u;
};

/*
** The following structure contains information used by the sqliteFix...
** routines as they walk the parse tree to make database references
** explicit.
*/
నిర్మాణం DbFixer {
  Parse *pParse;      /* The parsing context.  Error messages written here */
  Walker w;           /* Walker object */
  Schema *pSchema;    /* Fix items to this schema */
  u8 b_తాత్కాలిక;           /* True for TEMP schema entries */
  స్థిరరాశి అక్షరం *zDb;    /* Make sure all objects are contained in this database */
  స్థిరరాశి అక్షరం *zType;  /* Type of the container - used for error messages */
  స్థిరరాశి Token *pName; /* Name of the container - used for error messages */
};

/* Forward declarations */
పూర్ణాంకం sqlite3_Walk_సమాసం(Walker*, సమాసం*);
పూర్ణాంకం sqlite3_Walk_సమాసం_NN(Walker*, సమాసం*);
పూర్ణాంకం sqlite3_Walk_సమాసం_List(Walker*, సమాసం_List*);
పూర్ణాంకం sqlite3WalkSelect(Walker*, Select*);
పూర్ణాంకం sqlite3_Walk_Select_సమాసం(Walker*, Select*);
పూర్ణాంకం sqlite3WalkSelectFrom(Walker*, Select*);
పూర్ణాంకం sqlite3_సమాసం_Walk_Noop(Walker*, సమాసం*);
పూర్ణాంకం sqlite3SelectWalkNoop(Walker*, Select*);
పూర్ణాంకం sqlite3SelectWalkFail(Walker*, Select*);
పూర్ణాంకం sqlite3_Walker_లోతు_Increase(Walker*,Select*);
శూన్య sqlite3_Walker_లోతు_Decrease(Walker*,Select*);
శూన్య sqlite3WalkWinDefnDummyCallback(Walker*,Select*);

#నిర్వచితమైతే SQLITE_DEBUG
శూన్య sqlite3SelectWalkAssert2(Walker*, Select*);
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_CTE
శూన్య sqlite3_Select_తీయి_With(Walker*, Select*);
#లేకపోతే
# define sqlite3_Select_తీయి_With 0
#అయితే_ముగింపు

/*
** Return code from the parse-tree walking primitives and their
** callbacks.
*/
#నిర్వచించు WRC_Continue    0   /* Continue down into children */
#నిర్వచించు WRC_Prune       1   /* Omit children but continue walking siblings */
#నిర్వచించు WRC_Abort       2   /* Abandon the tree walk */

/*
** A single common table expression
*/
నిర్మాణం Cte {
  అక్షరం *zName;            /* Name of this CTE */
  సమాసం_List *p_నిలువుల_సంఖ్య;        /* List of explicit column names, or NULL */
  Select *pSelect;        /* The definition of this CTE */
  స్థిరరాశి అక్షరం *zCteErr;    /* Error message for circular references */
  CteUse *pUse;           /* Usage information for this CTE */
  u8 eM10d;               /* The MATERIALIZED flag */
};

/*
** Allowed values for the materialized flag (eM10d):
*/
#నిర్వచించు M10d_Yes       0  /* AS MATERIALIZED */
#నిర్వచించు M10d_Any       1  /* Not specified.  Query planner's choice */
#నిర్వచించు M10d_No        2  /* AS NOT MATERIALIZED */

/*
** An instance of the With object represents a WITH clause containing
** one or more CTEs (common table expressions).
*/
నిర్మాణం With {
  పూర్ణాంకం nCte;               /* Number of CTEs in the WITH clause */
  పూర్ణాంకం bView;              /* Belongs to the outermost Select of a view */
  With *pOuter;           /* Containing WITH clause, or NULL */
  Cte a[FLEXARRAY];       /* For each CTE in the WITH clause.... */
};

/* The size (in bytes) of a With object that can hold as many
** as N different CTEs. */
#నిర్వచించు SZ_WITH(N)  (offsetof(With,a) + (N)*పరిమాణం(Cte))

/*
** The Cte object is not guaranteed to persist for the entire duration
** of code generation.  (The query flattener or other parser tree
** edits might delete it.)  The following object records information
** about each Common Table Expression that must be preserved for the
** duration of the parse.
**
** The CteUse objects are freed using sqlite3ParserAddCleanup() rather
** than sqlite3SelectDelete(), which is what enables them to persist
** until the end of code generation.
*/
నిర్మాణం CteUse {
  పూర్ణాంకం nUse;              /* Number of users of this CTE */
  పూర్ణాంకం addrM9e;           /* Start of subroutine to compute materialization */
  పూర్ణాంకం regRtn;            /* Return address register for addrM9e subroutine */
  పూర్ణాంకం iCur;              /* Ephemeral table holding the materialization */
  LogEst n_అడ్డువరుస_Est;        /* Estimated number of rows in the table */
  u8 eM10d;              /* The MATERIALIZED flag */
};


/* Client data associated with sqlite3_set_clientdata() and
** sqlite3_get_clientdata().
*/
నిర్మాణం Db_Client_దత్తాంశం {
  Db_Client_దత్తాంశం *p_తదుపరి;        /* Next in a linked list */
  శూన్య *p_దత్తాంశం;                /* The data */
  శూన్య (*xDestructor)(శూన్య*); /* Destructor.  Might be NULL */
  అక్షరం zName[FLEXARRAY];      /* Name of this client data. MUST BE LAST */
};

/* The size (in bytes) of a DbClientData object that can has a name
** that is N bytes long, including the zero-terminator. */
#నిర్వచించు SZ_DBCLIENTDATA(N) (offsetof(Db_Client_దత్తాంశం,zName)+(N))

#నిర్వచితమైతే SQLITE_DEBUG
/*
** An instance of the TreeView object is used for printing the content of
** data structures on sqlite3DebugPrintf() using a tree-like view.
*/
నిర్మాణం వృక్షం_View {
  పూర్ణాంకం iLevel;             /* Which level of the tree we are on */
  u8  b_పంక్తి[100];         /* Draw vertical in column i if bLine[i] is true */
};
#అయితే_ముగింపు /* SQLITE_DEBUG */

/*
** This object is used in various ways, most (but not all) related to window
** functions.
**
**   (1) A single instance of this structure is attached to the
**       the Expr.y.pWin field for each window function in an expression tree.
**       This object holds the information contained in the OVER clause,
**       plus additional fields used during code generation.
**
**   (2) All window functions in a single SELECT form a linked-list
**       attached to Select.pWin.  The Window.pFunc and Window.pExpr
**       fields point back to the expression that is the window function.
**
**   (3) The terms of the WINDOW clause of a SELECT are instances of this
**       object on a linked list attached to Select.pWinDefn.
**
**   (4) For an aggregate function with a FILTER clause, an instance
**       of this object is stored in Expr.y.pWin with eFrmType set to
**       TK_FILTER. In this case the only field used is Window.pFilter.
**
** The uses (1) and (2) are really the same Window object that just happens
** to be accessible in two different ways.  Use case (3) are separate objects.
*/
నిర్మాణం కిటికీ {
  అక్షరం *zName;            /* Name of window (may be NULL) */
  అక్షరం *z_పునాది;            /* Name of base window for chaining (may be NULL) */
  సమాసం_List *p_విభజన;   /* PARTITION BY clause */
  సమాసం_List *p_క్రమం_By;     /* ORDER BY clause */
  u8 eFrmType;            /* TK_RANGE, TK_GROUPS, TK_ROWS, or 0 */
  u8 e_ప్రారంభం;              /* UNBOUNDED, CURRENT, PRECEDING or FOLLOWING */
  u8 e_చివర;                /* UNBOUNDED, CURRENT, PRECEDING or FOLLOWING */
  u8 b_Implicit_చట్రం;      /* True if frame was implicitly specified */
  u8 eExclude;            /* TK_NO, TK_CURRENT, TK_TIES, TK_GROUP, or 0 */
  సమాసం *p_ప్రారంభం;           /* Expression for "<expr> PRECEDING" */
  సమాసం *p_చివర;             /* Expression for "<expr> FOLLOWING" */
  కిటికీ **ppThis;        /* Pointer to this object in Select.pWin list */
  కిటికీ *p_తదుపరి_Win;       /* Next window function belonging to this SELECT */
  సమాసం *pFilter;          /* The FILTER expression */
  FuncDef *pWFunc;        /* The function */
  పూర్ణాంకం iEphCsr;            /* Partition buffer or Peer buffer */
  పూర్ణాంకం regAccum;           /*సంచాయకం*/
  పూర్ణాంకం reg_ఫలితం;          /* Interim result */
  పూర్ణాంకం csrApp;             /* Function cursor (used by min/max) */
  పూర్ణాంకం regApp;             /* Function register (also used by min/max) */
  పూర్ణాంకం regPart;            /* Array of registers for PARTITION BY values */
  సమాసం *pOwner;           /* Expression object this window is attached to */
  పూర్ణాంకం n_బఫరు_నిలువు;         /* Number of columns in buffer table */
  పూర్ణాంకం i_Arg_నిలువు;            /* Offset of first argument for this function */
  పూర్ణాంకం regOne;             /* Register containing constant value 1 */
  పూర్ణాంకం reg_ప్రారంభం_Rowid;
  పూర్ణాంకం reg_చివర_Rowid;
  u8 b_సమాసం_వాదనలు;           /* Defer evaluation of window function arguments
                          ** due to the SQLITE_SUBTYPE flag */
};

Select *sqlite3_Multi_విలువలు(Parse *pParse, Select *p_ఎడమ, సమాసం_List *p_అడ్డువరుస);
శూన్య sqlite3_Multi_విలువలు_చివర(Parse *pParse, Select *pVal);

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
శూన్య sqlite3_కిటికీ_Delete(sqlite3*, కిటికీ*);
శూన్య sqlite3_కిటికీ_Unlink_From_Select(కిటికీ*);
శూన్య sqlite3_కిటికీ_List_Delete(sqlite3 *db, కిటికీ *p);
కిటికీ *sqlite3_కిటికీ_Alloc(Parse*, పూర్ణాంకం, పూర్ణాంకం, సమాసం*, పూర్ణాంకం , సమాసం*, u8);
శూన్య sqlite3_కిటికీ_Attach(Parse*, సమాసం*, కిటికీ*);
శూన్య sqlite3_కిటికీ_Link(Select *pSel, కిటికీ *pWin);
పూర్ణాంకం sqlite3_కిటికీ_Compare(స్థిరరాశి Parse*, స్థిరరాశి కిటికీ*, స్థిరరాశి కిటికీ*, పూర్ణాంకం);
శూన్య sqlite3_కిటికీ_సంకేతం_Init(Parse*, Select*);
శూన్య sqlite3_కిటికీ_సంకేతం_అడుగు(Parse*, Select*, WhereInfo*, పూర్ణాంకం, పూర్ణాంకం);
పూర్ణాంకం sqlite3_కిటికీ_Rewrite(Parse*, Select*);
శూన్య sqlite3_కిటికీ_Update(Parse*, కిటికీ*, కిటికీ*, FuncDef*);
కిటికీ *sqlite3_కిటికీ_Dup(sqlite3 *db, సమాసం *pOwner, కిటికీ *p);
కిటికీ *sqlite3_కిటికీ_List_Dup(sqlite3 *db, కిటికీ *p);
శూన్య sqlite3_కిటికీ_Functions(శూన్య);
శూన్య sqlite3_కిటికీ_Chain(Parse*, కిటికీ*, కిటికీ*);
కిటికీ *sqlite3_కిటికీ_Assemble(Parse*, కిటికీ*, సమాసం_List*, సమాసం_List*, Token*);
#లేకపోతే
# define sqlite3_కిటికీ_Delete(a,b)
# define sqlite3_కిటికీ_Functions()
# define sqlite3_కిటికీ_Attach(a,b,c)
#అయితే_ముగింపు

/*
** Assuming zIn points to the first byte of a UTF-8 character,
** advance zIn to point to the first byte of the next UTF-8 character.
**
** # Dividing malformed UTF-8 into characters (tag-20260418-01)
**
** If a text input is malformed UTF-8, SQLite does not make any guarantees
** about how the bytes are divided up into characters.  The system promises
** to not overflow an array or cause other memory errors when presented
** with malformed UTF-8.  And it promises to preserve the specific
** sequence of bytes as long as no conversion occur.  But beyond that,
** there are no guarantees.  Results can vary from one version to the
** next.
**
** The SQLITE_SKIP_UTF8 macro below is one technique for dividing UTF-8
** into characters.  The length() and substr() SQL functions use a
** different technique when searching across multiple characters, a
** technique that exchanges a subtraction for comparison of z and results
** in faster machine code on some compilers and architectures.  The code
** in substr() to skip over p1 characters goes something like this:
**
**    for( ; p1>0; p1--){
**                     // vvvv--- tag-20260418-01
**      if( (u8)(z[0]-1)<(0x80-1) ){
**        z++;
**      }else if( z[0]==0 ){
**        break;
**      }else{
**        do{ z++; }while( (z[0]&0xc0)==0x80 );
**      }
**    }
**
** In valid UTF-8, multibyte characters always begin with a byte with the
** two most significant bits set and that is followed by one or more bytes
** for which the two most significant bits are 10.  In other words:
**
**     First byte:        (BYTE & 0xc0)==0xc0
**     Following bytes:   (BYTE & 0xc0)==0x80
**
** What to do if the input byte sequence contain a "following byte" that
** is not preceded by a "first byte"?  How many characters are in the
** byte sequence:  0x61 0x81 0x82 0x7a?  3 or 4 or something else?
** 
** If you use the macro below, the answer will be 4.  If you use the code
** snippet demonstrated at tag-20260418-01, then answer is 3.  If you
** use a variant of tag-20260418-01 where the constant of comparison is
** 0xc0-1 instead of 0x80-1 then the answer is again 4.  The key point is
** that because the input is malformed UTF-8, so is no "correct" answer.
** SQLite is free to use either value.
**
** It turns out that GCC 13.3.0 is able to generate faster code (at least
** on x86-64) if the constant at tag-20260418-01 is (0x80-1).  If you make
** that constant (0xc0-1) instead, gcc 13.3.0 generates code that runs slower.
** So the (0x80-1) constant is used for substr() and length().
*/
#నిర్వచించు SQLITE_SKIP_UTF8(z_లోపల) {                        \
  ఒకవేళ( (*(z_లోపల++))>=0xc0 ){                              \
    అయితే( (*z_లోపల & 0xc0)==0x80 ){ z_లోపల++; }             \
  }                                                    \
}

/*
** The SQLITE_*_BKPT macros are substitutes for the error codes with
** the same name but without the _BKPT suffix.  These macros invoke
** routines that report the line-number on which the error originated
** using sqlite3_log().  The routines also provide a convenient place
** to set a debugger breakpoint.
*/
పూర్ణాంకం sqlite3ReportError(పూర్ణాంకం iErr, పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *zType);
పూర్ణాంకం sqlite3CorruptError(పూర్ణాంకం);
పూర్ణాంకం sqlite3MisuseError(పూర్ణాంకం);
పూర్ణాంకం sqlite3CantopenError(పూర్ణాంకం);
#నిర్వచించు SQLITE_CORRUPT_BKPT sqlite3CorruptError(__LINE__)
#నిర్వచించు SQLITE_MISUSE_BKPT sqlite3MisuseError(__LINE__)
#నిర్వచించు SQLITE_CANTOPEN_BKPT sqlite3CantopenError(__LINE__)
#నిర్వచితమైతే SQLITE_DEBUG
  పూర్ణాంకం sqlite3NomemError(పూర్ణాంకం);
  పూర్ణాంకం sqlite3IoerrnomemError(పూర్ణాంకం);
# define SQLITE_NOMEM_BKPT sqlite3NomemError(__LINE__)
# define SQLITE_IOERR_NOMEM_BKPT sqlite3IoerrnomemError(__LINE__)
#లేకపోతే
# define SQLITE_NOMEM_BKPT SQLITE_NOMEM
# define SQLITE_IOERR_NOMEM_BKPT SQLITE_IOERR_NOMEM
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_CORRUPT_PGNO)
  పూర్ణాంకం sqlite3CorruptPgnoError(పూర్ణాంకం,Pgno);
# define SQLITE_CORRUPT_PGNO(P) sqlite3CorruptPgnoError(__LINE__,(P))
#లేకపోతే
# define SQLITE_CORRUPT_PGNO(P) sqlite3CorruptError(__LINE__)
#అయితే_ముగింపు

/*
** FTS3 and FTS4 both require virtual table support
*/
#ఒకవేళ defined(SQLITE_OMIT_VIRTUALTABLE)
# undef SQLITE_ENABLE_FTS3
# undef SQLITE_ENABLE_FTS4
#అయితే_ముగింపు

/*
** FTS4 is really an extension for FTS3.  It is enabled using the
** SQLITE_ENABLE_FTS3 macro.  But to avoid confusion we also call
** the SQLITE_ENABLE_FTS4 macro to serve as an alias for SQLITE_ENABLE_FTS3.
*/
#ఒకవేళ defined(SQLITE_ENABLE_FTS4) && !defined(SQLITE_ENABLE_FTS3)
# define SQLITE_ENABLE_FTS3 1
#అయితే_ముగింపు

/*
** The following macros mimic the standard library functions toupper(),
** isspace(), isalnum(), isdigit() and isxdigit(), respectively. The
** sqlite versions only work for ASCII characters, regardless of locale.
*/
#నిర్వచితమైతే SQLITE_ASCII
# define sqlite3Toupper(x)  ((x)&~(sqlite3CtypeMap[(గుర్తులేని అక్షరం)(x)]&0x20))
# define sqlite3Isspace(x)   (sqlite3CtypeMap[(గుర్తులేని అక్షరం)(x)]&0x01)
# define sqlite3Isalnum(x)   (sqlite3CtypeMap[(గుర్తులేని అక్షరం)(x)]&0x06)
# define sqlite3Isalpha(x)   (sqlite3CtypeMap[(గుర్తులేని అక్షరం)(x)]&0x02)
# define sqlite3Isdigit(x)   (sqlite3CtypeMap[(గుర్తులేని అక్షరం)(x)]&0x04)
# define sqlite3Isxdigit(x)  (sqlite3CtypeMap[(గుర్తులేని అక్షరం)(x)]&0x08)
# define sqlite3Tolower(x)   (sqlite3_పై_గమ్యం_క్రింది[(గుర్తులేని అక్షరం)(x)])
# define sqlite3Isquote(x)   (sqlite3CtypeMap[(గుర్తులేని అక్షరం)(x)]&0x80)
# define sqlite3JsonId1(x)   (sqlite3CtypeMap[(గుర్తులేని అక్షరం)(x)]&0x42)
# define sqlite3JsonId2(x)   (sqlite3CtypeMap[(గుర్తులేని అక్షరం)(x)]&0x46)
#లేకపోతే
# define sqlite3Toupper(x)   పెద్ద_అక్షరం((గుర్తులేని అక్షరం)(x))
# define sqlite3Isspace(x)   ఖాళీనా((గుర్తులేని అక్షరం)(x))
# define sqlite3Isalnum(x)   అక్షరమా_అంకెనా((గుర్తులేని అక్షరం)(x))
# define sqlite3Isalpha(x)   అక్షరమా((గుర్తులేని అక్షరం)(x))
# define sqlite3Isdigit(x)   అంకెనా((గుర్తులేని అక్షరం)(x))
# define sqlite3Isxdigit(x)  isxdigit((గుర్తులేని అక్షరం)(x))
# define sqlite3Tolower(x)   tolower((గుర్తులేని అక్షరం)(x))
# define sqlite3Isquote(x)   ((x)=='"'||(x)=='\''||(x)=='['||(x)=='`')
# define sqlite3JsonId1(x)   (sqlite3IsIdChar(x)&&(x)<'0')
# define sqlite3JsonId2(x)   sqlite3IsIdChar(x)
#అయితే_ముగింపు
పూర్ణాంకం sqlite3IsIdChar(u8);

/*
** Internal function prototypes
*/
పూర్ణాంకం sqlite3StrICmp(స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*);
పూర్ణాంకం sqlite3Strlen30(స్థిరరాశి అక్షరం*);
#నిర్వచించు sqlite3Strlen30NN(C) (అక్షరశ్రేణి_పొడవు(C)&0x3fffffff)
అక్షరం *sqlite3_స్తంభం_Type(స్తంభం*,అక్షరం*);
#నిర్వచించు sqlite3StrNICmp sqlite3_strnicmp

పూర్ణాంకం sqlite3MallocInit(శూన్య);
శూన్య sqlite3_Malloc_చివర(శూన్య);
శూన్య *sqlite3Malloc(u64);
శూన్య *sqlite3MallocZero(u64);
శూన్య *sqlite3DbMallocZero(sqlite3*, u64);
శూన్య *sqlite3DbMallocRaw(sqlite3*, u64);
శూన్య *sqlite3DbMallocRawNN(sqlite3*, u64);
అక్షరం *sqlite3DbStrDup(sqlite3*,స్థిరరాశి అక్షరం*);
అక్షరం *sqlite3DbStrNDup(sqlite3*,స్థిరరాశి అక్షరం*, u64);
అక్షరం *sqlite3_Db_పరిధి_Dup(sqlite3*,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*);
శూన్య *sqlite3Realloc(శూన్య*, u64);
శూన్య *sqlite3DbReallocOrFree(sqlite3 *, శూన్య *, u64);
శూన్య *sqlite3DbRealloc(sqlite3 *, శూన్య *, u64);
శూన్య sqlite3DbFree(sqlite3*, శూన్య*);
శూన్య sqlite3DbFreeNN(sqlite3*, శూన్య*);
శూన్య sqlite3DbNNFreeNN(sqlite3*, శూన్య*);
పూర్ణాంకం sqlite3_Malloc_కొలత(స్థిరరాశి శూన్య*);
పూర్ణాంకం sqlite3_Db_Malloc_కొలత(sqlite3*, స్థిరరాశి శూన్య*);
శూన్య *sqlite3_పుట_Malloc(పూర్ణాంకం);
శూన్య sqlite3_పుట_Free(శూన్య*);
శూన్య sqlite3_Mem_సమితి_Default(శూన్య);
#నిర్వచితం_కాకపోతే SQLITE_UNTESTABLE
శూన్య sqlite3BenignMallocHooks(శూన్య (*)(శూన్య), శూన్య (*)(శూన్య));
#అయితే_ముగింపు
పూర్ణాంకం sqlite3_కుప్ప_Nearly_నిండిన(శూన్య);

/*
** On systems with ample stack space and that support alloca(), make
** use of alloca() to obtain space for large automatic objects.  By default,
** obtain space from malloc().
**
** The alloca() routine never returns NULL.  This will cause code paths
** that deal with sqlite3StackAlloc() failures to be unreachable.
*/
#నిర్వచితమైతే SQLITE_USE_ALLOCA
# define sqlite3_దొంతర_Alloc_Raw(D,N)   alloca(N)
# define sqlite3_దొంతర_Alloc_Raw_NN(D,N) alloca(N)
# define sqlite3_దొంతర_Free(D,P)
# define sqlite3_దొంతర_Free_NN(D,P)
#లేకపోతే
# define sqlite3_దొంతర_Alloc_Raw(D,N)   sqlite3DbMallocRaw(D,N)
# define sqlite3_దొంతర_Alloc_Raw_NN(D,N) sqlite3DbMallocRawNN(D,N)
# define sqlite3_దొంతర_Free(D,P)       sqlite3DbFree(D,P)
# define sqlite3_దొంతర_Free_NN(D,P)     sqlite3DbFreeNN(D,P)
#అయితే_ముగింపు

/* Do not allow both MEMSYS5 and MEMSYS3 to be defined together.  If they
** are, disable MEMSYS3
*/
#నిర్వచితమైతే SQLITE_ENABLE_MEMSYS5
స్థిరరాశి sqlite3_mem_methods *sqlite3MemGetMemsys5(శూన్య);
#నిర్వచనం_రద్దు SQLITE_ENABLE_MEMSYS3
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_MEMSYS3
స్థిరరాశి sqlite3_mem_methods *sqlite3MemGetMemsys3(శూన్య);
#అయితే_ముగింపు


#నిర్వచితం_కాకపోతే SQLITE_MUTEX_OMIT
  sqlite3_mutex_methods స్థిరరాశి *sqlite3DefaultMutex(శూన్య);
  sqlite3_mutex_methods స్థిరరాశి *sqlite3NoopMutex(శూన్య);
  sqlite3_mutex *sqlite3MutexAlloc(పూర్ణాంకం);
  పూర్ణాంకం sqlite3MutexInit(శూన్య);
  పూర్ణాంకం sqlite3_Mutex_చివర(శూన్య);
#అయితే_ముగింపు
#ఒకవేళ !defined(SQLITE_MUTEX_OMIT) && !defined(SQLITE_MUTEX_NOOP)
  శూన్య sqlite3_జ్ఞాపకం_Barrier(శూన్య);
#లేకపోతే
# define sqlite3_జ్ఞాపకం_Barrier()
#అయితే_ముగింపు

sqlite3_int64 sqlite3_స్థితి_విలువ(పూర్ణాంకం);
శూన్య sqlite3_స్థితి_పైకి(పూర్ణాంకం, పూర్ణాంకం);
శూన్య sqlite3_స్థితి_కిందికి(పూర్ణాంకం, పూర్ణాంకం);
శూన్య sqlite3_స్థితి_Highwater(పూర్ణాంకం, పూర్ణాంకం);
పూర్ణాంకం sqlite3_Lookaside_వాడిన(sqlite3*,పూర్ణాంకం*);

/* Access to mutexes used by sqlite3_status() */
sqlite3_mutex *sqlite3Pcache1Mutex(శూన్య);
sqlite3_mutex *sqlite3MallocMutex(శూన్య);


/* The SQLITE_THREAD_MISUSE_WARNINGS compile-time option used to be called
** SQLITE_ENABLE_MULTITHREADED_CHECKS.  Keep that older macro for backwards
** compatibility, at least for a while... */
#నిర్వచితమైతే SQLITE_ENABLE_MULTITHREADED_CHECKS
# define SQLITE_THREAD_MISUSE_WARNINGS 1
#అయితే_ముగింపు

/* SQLITE_THREAD_MISUSE_ABORT implies SQLITE_THREAD_MISUSE_WARNINGS */
#నిర్వచితమైతే SQLITE_THREAD_MISUSE_ABORT
# define SQLITE_THREAD_MISUSE_WARNINGS 1
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_THREAD_MISUSE_WARNINGS) && !defined(SQLITE_MUTEX_OMIT)
శూన్య sqlite3MutexWarnOnContention(sqlite3_mutex*);
#లేకపోతే
# define sqlite3MutexWarnOnContention(x)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
# define EXP754 (((u64)0x7ff)<<52)
# define MAN754 ((((u64)1)<<52)-1)
# define IsNaN(X) (((X)&EXP754)==EXP754 && ((X)&MAN754)!=0)
# define IsOvfl(X) (((X)&EXP754)==EXP754)
  పూర్ణాంకం sqlite3IsNaN(ద్విగుణ);
  పూర్ణాంకం sqlite3_Is_అధికప్రవాహం(ద్విగుణ);
#లేకపోతే
# define IsNaN(X)             0
# define sqlite3IsNaN(X)      0
# define sqlite3_Is_అధికప్రవాహం_2(X) 0
#అయితే_ముగింపు

/*
** An instance of the following structure holds information about SQL
** functions arguments that are the parameters to the printf() function.
*/
నిర్మాణం PrintfArguments {
  పూర్ణాంకం nArg;                /* Total number of arguments */
  పూర్ణాంకం n_వాడిన;               /* Number of arguments used so far */
  sqlite3_విలువ **apArg;   /* The argument values */
};

/*
** Maxium number of base-10 digits in an unsigned 64-bit integer
*/
#నిర్వచించు SQLITE_U64_DIGITS 20

/*
** An instance of this object receives the decoding of a floating point
** value into an approximate decimal representation.
*/
నిర్మాణం FpDecode {
  పూర్ణాంకం n;                           /* Significant digits in the decode */
  పూర్ణాంకం iDP;                         /* Location of the decimal point */
  అక్షరం *z;                         /* Start of significant digits */
  అక్షరం zBuf[SQLITE_U64_DIGITS+1];  /* Storage for significant digits */
  అక్షరం sign;                       /* '+' or '-' */
  అక్షరం isSpecial;                  /* 1: Infinity  2: NaN */
};

శూన్య sqlite3FpDecode(FpDecode*,ద్విగుణ,పూర్ణాంకం,పూర్ణాంకం);
అక్షరం *sqlite3MPrintf(sqlite3*,స్థిరరాశి అక్షరం*, ...);
అక్షరం *sqlite3VMPrintf(sqlite3*,స్థిరరాశి అక్షరం*, va_list);
#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  శూన్య sqlite3DebugPrintf(స్థిరరాశి అక్షరం*, ...);
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_TEST)
  శూన్య *sqlite3_Test_పాఠ్యం_గమ్యం_Ptr(స్థిరరాశి అక్షరం*);
  స్థిరరాశి అక్షరం *sqlite3_Test_Ptr_గమ్యం_పాఠ్యం(శూన్య*);
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_DEBUG)
  శూన్య sqlite3_వృక్షం_View_పంక్తి(వృక్షం_View*, స్థిరరాశి అక్షరం *zFormat, ...);
  శూన్య sqlite3_వృక్షం_View_సమాసం(వృక్షం_View*, స్థిరరాశి సమాసం*, u8);
  శూన్య sqlite3_వృక్షం_View_Bare_సమాసం_List(వృక్షం_View*, స్థిరరాశి సమాసం_List*, స్థిరరాశి అక్షరం*);
  శూన్య sqlite3_వృక్షం_View_సమాసం_List(వృక్షం_View*, స్థిరరాశి సమాసం_List*, u8, స్థిరరాశి అక్షరం*);
  శూన్య sqlite3_వృక్షం_View_Bare_Id_List(వృక్షం_View*, స్థిరరాశి IdList*, స్థిరరాశి అక్షరం*);
  శూన్య sqlite3_వృక్షం_View_Id_List(వృక్షం_View*, స్థిరరాశి IdList*, u8, స్థిరరాశి అక్షరం*);
  శూన్య sqlite3_వృక్షం_View_స్తంభం_List(వృక్షం_View*, స్థిరరాశి స్తంభం*, పూర్ణాంకం, u8);
  శూన్య sqlite3_వృక్షం_View_Src_List(వృక్షం_View*, స్థిరరాశి SrcList*);
  శూన్య sqlite3_వృక్షం_View_Select(వృక్షం_View*, స్థిరరాశి Select*, u8);
  శూన్య sqlite3_వృక్షం_View_With(వృక్షం_View*, స్థిరరాశి With*, u8);
  శూన్య sqlite3_వృక్షం_View_Upsert(వృక్షం_View*, స్థిరరాశి Upsert*, u8);
#ఒకవేళ TREETRACE_ENABLED
  శూన్య sqlite3_వృక్షం_View_Delete(స్థిరరాశి With*, స్థిరరాశి SrcList*, స్థిరరాశి సమాసం*,
                             స్థిరరాశి సమాసం_List*,స్థిరరాశి సమాసం*, స్థిరరాశి Trigger*);
  శూన్య sqlite3_వృక్షం_View_చొప్పించు(స్థిరరాశి With*, స్థిరరాశి SrcList*,
                             స్థిరరాశి IdList*, స్థిరరాశి Select*, స్థిరరాశి సమాసం_List*,
                             పూర్ణాంకం, స్థిరరాశి Upsert*, స్థిరరాశి Trigger*);
  శూన్య sqlite3_వృక్షం_View_Update(స్థిరరాశి With*, స్థిరరాశి SrcList*, స్థిరరాశి సమాసం_List*,
                             స్థిరరాశి సమాసం*, పూర్ణాంకం, స్థిరరాశి సమాసం_List*, స్థిరరాశి సమాసం*,
                             స్థిరరాశి Upsert*, స్థిరరాశి Trigger*);
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
  శూన్య sqlite3_వృక్షం_View_Trigger_అడుగు(వృక్షం_View*, స్థిరరాశి Trigger_అడుగు*, u8, u8);
  శూన్య sqlite3_వృక్షం_View_Trigger(వృక్షం_View*, స్థిరరాశి Trigger*, u8, u8);
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
  శూన్య sqlite3_వృక్షం_View_కిటికీ(వృక్షం_View*, స్థిరరాశి కిటికీ*, u8);
  శూన్య sqlite3_వృక్షం_View_Win_Func(వృక్షం_View*, స్థిరరాశి కిటికీ*, u8);
#అయితే_ముగింపు
  శూన్య sqlite3_Show_సమాసం(స్థిరరాశి సమాసం*);
  శూన్య sqlite3_Show_సమాసం_List(స్థిరరాశి సమాసం_List*);
  శూన్య sqlite3ShowIdList(స్థిరరాశి IdList*);
  శూన్య sqlite3ShowSrcList(స్థిరరాశి SrcList*);
  శూన్య sqlite3ShowSelect(స్థిరరాశి Select*);
  శూన్య sqlite3ShowWith(స్థిరరాశి With*);
  శూన్య sqlite3ShowUpsert(స్థిరరాశి Upsert*);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
  శూన్య sqlite3_Show_Trigger_అడుగు(స్థిరరాశి Trigger_అడుగు*);
  శూన్య sqlite3_Show_Trigger_అడుగు_List(స్థిరరాశి Trigger_అడుగు*);
  శూన్య sqlite3ShowTrigger(స్థిరరాశి Trigger*);
  శూన్య sqlite3ShowTriggerList(స్థిరరాశి Trigger*);
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
  శూన్య sqlite3_Show_కిటికీ(స్థిరరాశి కిటికీ*);
  శూన్య sqlite3ShowWinFunc(స్థిరరాశి కిటికీ*);
#అయితే_ముగింపు
  శూన్య sqlite3ShowBitvec(Bitvec*);
#అయితే_ముగింపు

శూన్య sqlite3_సమితి_String(అక్షరం **, sqlite3*, స్థిరరాశి అక్షరం*);
శూన్య sqlite3_పురోగతి_Check(Parse*);
శూన్య sqlite3_Error_సందేశం(Parse*, స్థిరరాశి అక్షరం*, ...);
పూర్ణాంకం sqlite3_Error_గమ్యం_Parser(sqlite3*,పూర్ణాంకం);
శూన్య sqlite3Dequote(అక్షరం*);
శూన్య sqlite3_Dequote_సమాసం(సమాసం*);
శూన్య sqlite3DequoteToken(Token*);
శూన్య sqlite3_Dequote_సంఖ్య(Parse*, సమాసం*);
శూన్య sqlite3TokenInit(Token*,అక్షరం*);
పూర్ణాంకం sqlite3_Keyword_సంకేతం(స్థిరరాశి గుర్తులేని అక్షరం*, పూర్ణాంకం);
పూర్ణాంకం sqlite3_నిరంతరం_Parser(Parse*, స్థిరరాశి అక్షరం*);
శూన్య sqlite3_ముగింపు_Coding(Parse*);
పూర్ణాంకం sqlite3_Get_తాత్కాలిక_Reg(Parse*);
శూన్య sqlite3_Release_తాత్కాలిక_Reg(Parse*,పూర్ణాంకం);
పూర్ణాంకం sqlite3_Get_తాత్కాలిక_Range(Parse*,పూర్ణాంకం);
శూన్య sqlite3_Release_తాత్కాలిక_Range(Parse*,పూర్ణాంకం,పూర్ణాంకం);
శూన్య sqlite3_Clear_తాత్కాలిక_Reg_Cache(Parse*);
శూన్య sqlite3_Touch_రిజిస్టరు(Parse*,పూర్ణాంకం);
#ఒకవేళ defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
పూర్ణాంకం sqlite3_మొదటి_అందుబాటు_రిజిస్టరు(Parse*,పూర్ణాంకం);
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_DEBUG
పూర్ణాంకం sqlite3_No_Temps_లోపల_Range(Parse*,పూర్ణాంకం,పూర్ణాంకం);
#అయితే_ముగింపు
సమాసం *sqlite3_సమాసం_Alloc(sqlite3*,పూర్ణాంకం,స్థిరరాశి Token*,పూర్ణాంకం);
సమాసం *sqlite3_సమాసం(sqlite3*,పూర్ణాంకం,స్థిరరాశి అక్షరం*);
సమాసం *sqlite3_సమాసం_Int32(sqlite3*,పూర్ణాంకం);
శూన్య sqlite3_సమాసం_Attach_Subtrees(sqlite3*,సమాసం*,సమాసం*,సమాసం*);
సమాసం *sqlite3PExpr(Parse*, పూర్ణాంకం, సమాసం*, సమాసం*);
శూన్య sqlite3_PExpr_కూడు_Select(Parse*, సమాసం*, Select*);
సమాసం *sqlite3_సమాసం_And(Parse*,సమాసం*, సమాసం*);
సమాసం *sqlite3_సమాసం_Simplified_And_Or(సమాసం*);
సమాసం *sqlite3_సమాసం_Function(Parse*,సమాసం_List*, స్థిరరాశి Token*, పూర్ణాంకం);
శూన్య sqlite3_సమాసం_కూడు_Function_క్రమం_By(Parse*,సమాసం*,సమాసం_List*);
శూన్య sqlite3_సమాసం_క్రమం_By_Aggregate_Error(Parse*,సమాసం*);
శూన్య sqlite3_సమాసం_Function_Usable(Parse*,స్థిరరాశి సమాసం*,స్థిరరాశి FuncDef*);
శూన్య sqlite3_సమాసం_కేటాయించు_Var_సంఖ్య(Parse*, సమాసం*, u32);
శూన్య sqlite3_సమాసం_Delete(sqlite3*, సమాసం*);
శూన్య sqlite3_సమాసం_Delete_Generic(sqlite3*,శూన్య*);
పూర్ణాంకం sqlite3_సమాసం_Deferred_Delete(Parse*, సమాసం*);
శూన్య sqlite3_సమాసం_Unmap_And_Delete(Parse*, సమాసం*);
సమాసం_List *sqlite3_సమాసం_List_Append(Parse*,సమాసం_List*,సమాసం*);
సమాసం_List *sqlite3_సమాసం_List_Append_సదిశ(Parse*,సమాసం_List*,IdList*,సమాసం*);
Select *sqlite3_సమాసం_List_గమ్యం_విలువలు(Parse*, పూర్ణాంకం, సమాసం_List*);
శూన్య sqlite3_సమాసం_List_సమితి_క్రమీకరించు_క్రమం(సమాసం_List*,పూర్ణాంకం,పూర్ణాంకం);
శూన్య sqlite3_సమాసం_List_సమితి_Name(Parse*,సమాసం_List*,స్థిరరాశి Token*,పూర్ణాంకం);
శూన్య sqlite3_సమాసం_List_సమితి_పరిధి(Parse*,సమాసం_List*,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*);
శూన్య sqlite3_సమాసం_List_Delete(sqlite3*, సమాసం_List*);
శూన్య sqlite3_సమాసం_List_Delete_Generic(sqlite3*,శూన్య*);
పూర్ణాంకం sqlite3_సమాసం_Can_Return_Subtype(Parse*,సమాసం*);
u32 sqlite3_సమాసం_List_Flags(స్థిరరాశి సమాసం_List*);
పూర్ణాంకం sqlite3_సూచీ_Has_Duplicate_మూలం_పుట(సూచీ*);
పూర్ణాంకం sqlite3Init(sqlite3*, అక్షరం**);
పూర్ణాంకం sqlite3InitCallback(శూన్య*, పూర్ణాంకం, అక్షరం**, అక్షరం**);
పూర్ణాంకం sqlite3InitOne(sqlite3*, పూర్ణాంకం, అక్షరం**, u32);
శూన్య sqlite3Pragma(Parse*,Token*,Token*,Token*,పూర్ణాంకం);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
Module *sqlite3_Pragma_Vtab_రిజిస్టరు(sqlite3*,స్థిరరాశి అక్షరం *zName);
#అయితే_ముగింపు
శూన్య sqlite3_Reset_All_Schemas_యొక్క_Connection(sqlite3*);
శూన్య sqlite3ResetOneSchema(sqlite3*,పూర్ణాంకం);
శూన్య sqlite3CollapseDatabaseArray(sqlite3*);
శూన్య sqlite3_Commit_అంతర్గత_Changes(sqlite3*);
శూన్య sqlite3_స్తంభం_సమితి_సమాసం(Parse*,పట్టిక*,స్తంభం*,సమాసం*);
సమాసం *sqlite3_స్తంభం_సమాసం(పట్టిక*,స్తంభం*);
సమాసం *sqlite3_స్తంభం_సమాసం_Auth(పట్టిక*,స్తంభం*,Parse*);
శూన్య sqlite3_స్తంభం_సమితి_Coll(sqlite3*,స్తంభం*,స్థిరరాశి అక్షరం*zColl);
స్థిరరాశి అక్షరం *sqlite3_స్తంభం_Coll(స్తంభం*);
శూన్య sqlite3_Delete_స్తంభం_పేర్లు(sqlite3*,పట్టిక*);
శూన్య sqlite3_Generate_స్తంభం_పేర్లు(Parse *pParse, Select *pSelect);
పూర్ణాంకం sqlite3_Columns_From_సమాసం_List(Parse*,సమాసం_List*,i16*,స్తంభం**);
శూన్య sqlite3_Subquery_స్తంభం_Types(Parse*,పట్టిక*,Select*,అక్షరం);
పట్టిక *sqlite3_ఫలితం_సమితి_యొక్క_Select(Parse*,Select*,అక్షరం);
శూన్య sqlite3_Open_Schema_పట్టిక(Parse *, పూర్ణాంకం);
సూచీ *sqlite3_Primary_కీలకం_సూచీ(పట్టిక*);
పూర్ణాంకం sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(సూచీ*, పూర్ణాంకం);
#నిర్వచితమైతే SQLITE_OMIT_GENERATED_COLUMNS
# define sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(T,X) (X)  /* No-op pass-through */
# define sqlite3_Storage_స్తంభం_గమ్యం_పట్టిక(T,X) (X)  /* No-op pass-through */
#లేకపోతే
  i16 sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(పట్టిక*, i16);
  i16 sqlite3_Storage_స్తంభం_గమ్యం_పట్టిక(పట్టిక*, i16);
#అయితే_ముగింపు
శూన్య sqlite3_ప్రారంభం_పట్టిక(Parse*,Token*,Token*,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం);
#ఒకవేళ SQLITE_ENABLE_HIDDEN_COLUMNS
  శూన్య sqlite3_స్తంభం_Properties_From_Name(పట్టిక*, స్తంభం*);
#లేకపోతే
# define sqlite3_స్తంభం_Properties_From_Name(T,C) /* no-op */
#అయితే_ముగింపు
శూన్య sqlite3_కూడు_స్తంభం(Parse*,Token,Token);
శూన్య sqlite3_కూడు_Not_Null(Parse*, పూర్ణాంకం);
శూన్య sqlite3_కూడు_Primary_కీలకం(Parse*, సమాసం_List*, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం);
శూన్య sqlite3_కూడు_Check_Constraint(Parse*, సమాసం*, స్థిరరాశి అక్షరం*, స్థిరరాశి అక్షరం*);
శూన్య sqlite3_కూడు_Default_విలువ(Parse*,సమాసం*,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*);
శూన్య sqlite3_కూడు_Collate_Type(Parse*, Token*);
శూన్య sqlite3_కూడు_Generated(Parse*,సమాసం*,Token*);
శూన్య sqlite3_చివర_పట్టిక(Parse*,Token*,Token*,u32,Select*);
శూన్య sqlite3_కూడు_Returning(Parse*,సమాసం_List*);
పూర్ణాంకం sqlite3ParseUri(స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,గుర్తులేని పూర్ణాంకం*,
                    sqlite3_vfs**,అక్షరం**,అక్షరం **);
#నిర్వచించు sqlite3CodecQueryParameters(A,B,C) 0
Btree *sqlite3_Db_Name_గమ్యం_Btree(sqlite3*,స్థిరరాశి అక్షరం*);

#నిర్వచితమైతే SQLITE_UNTESTABLE
# define sqlite3FaultSim(X) SQLITE_OK
#లేకపోతే
  పూర్ణాంకం sqlite3FaultSim(పూర్ణాంకం);
#అయితే_ముగింపు

Bitvec *sqlite3BitvecCreate(u32);
పూర్ణాంకం sqlite3BitvecTest(Bitvec*, u32);
పూర్ణాంకం sqlite3BitvecTestNotNull(Bitvec*, u32);
పూర్ణాంకం sqlite3_Bitvec_సమితి(Bitvec*, u32);
శూన్య sqlite3BitvecClear(Bitvec*, u32, శూన్య*);
శూన్య sqlite3BitvecDestroy(Bitvec*);
u32 sqlite3_Bitvec_కొలత(Bitvec*);
#నిర్వచితం_కాకపోతే SQLITE_UNTESTABLE
పూర్ణాంకం sqlite3BitvecBuiltinTest(పూర్ణాంకం,పూర్ణాంకం*);
#అయితే_ముగింపు

అడ్డువరుస_సమితి *sqlite3_అడ్డువరుస_సమితి_Init(sqlite3*);
శూన్య sqlite3_అడ్డువరుస_సమితి_Delete(శూన్య*);
శూన్య sqlite3_అడ్డువరుస_సమితి_Clear(శూన్య*);
శూన్య sqlite3_అడ్డువరుస_సమితి_చొప్పించు(అడ్డువరుస_సమితి*, i64);
పూర్ణాంకం sqlite3_అడ్డువరుస_సమితి_Test(అడ్డువరుస_సమితి*, పూర్ణాంకం iBatch, i64);
పూర్ణాంకం sqlite3_అడ్డువరుస_సమితి_తదుపరి(అడ్డువరుస_సమితి*, i64*);

శూన్య sqlite3CreateView(Parse*,Token*,Token*,Token*,సమాసం_List*,Select*,పూర్ణాంకం,పూర్ణాంకం);

#ఒకవేళ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
  పూర్ణాంకం sqlite3_View_Get_స్తంభం_పేర్లు(Parse*,పట్టిక*);
#లేకపోతే
# define sqlite3_View_Get_స్తంభం_పేర్లు(A,B) 0
#అయితే_ముగింపు

#ఒకవేళ SQLITE_MAX_ATTACHED>30
  పూర్ణాంకం sqlite3_Db_ముసుగు_All_Zero(y_Db_ముసుగు);
#అయితే_ముగింపు
శూన్య sqlite3_Drop_పట్టిక(Parse*, SrcList*, పూర్ణాంకం, పూర్ణాంకం);
శూన్య sqlite3_సంకేతం_Drop_పట్టిక(Parse*, పట్టిక*, పూర్ణాంకం, పూర్ణాంకం);
శూన్య sqlite3_Delete_పట్టిక(sqlite3*, పట్టిక*);
శూన్య sqlite3_Delete_పట్టిక_Generic(sqlite3*, శూన్య*);
శూన్య sqlite3_Free_సూచీ(sqlite3*, సూచీ*);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINCREMENT
  శూన్య sqlite3AutoincrementBegin(Parse *pParse);
  శూన్య sqlite3_Autoincrement_చివర(Parse *pParse);
#లేకపోతే
# define sqlite3AutoincrementBegin(X)
# define sqlite3_Autoincrement_చివర(X)
#అయితే_ముగింపు
శూన్య sqlite3_చొప్పించు(Parse*, SrcList*, Select*, IdList*, పూర్ణాంకం, Upsert*);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
  శూన్య sqlite3ComputeGeneratedColumns(Parse*, పూర్ణాంకం, పట్టిక*);
#అయితే_ముగింపు
శూన్య *sqlite3ArrayAllocate(sqlite3*,శూన్య*,పూర్ణాంకం,పూర్ణాంకం*,పూర్ణాంకం*);
IdList *sqlite3IdListAppend(Parse*, IdList*, Token*);
పూర్ణాంకం sqlite3_Id_List_సూచీ(IdList*,స్థిరరాశి అక్షరం*);
SrcList *sqlite3SrcListEnlarge(Parse*, SrcList*, పూర్ణాంకం, పూర్ణాంకం);
SrcList *sqlite3SrcListAppendList(Parse *pParse, SrcList *p1, SrcList *p2);
SrcList *sqlite3SrcListAppend(Parse*, SrcList*, Token*, Token*);
శూన్య sqlite3SubqueryDelete(sqlite3*,Subquery*);
Select *sqlite3SubqueryDetach(sqlite3*,Src_అంశం*);
పూర్ణాంకం sqlite3_Src_అంశం_Attach_Subquery(Parse*, Src_అంశం*, Select*, పూర్ణాంకం);
SrcList *sqlite3_Src_List_Append_From_పదం(Parse*, SrcList*, Token*, Token*,
                                      Token*, Select*, OnOrUsing*);
శూన్య sqlite3_Src_List_సూచీకృత_By(Parse *, SrcList *, Token *);
శూన్య sqlite3_Src_List_Func_వాదనలు(Parse*, SrcList*, సమాసం_List*);
పూర్ణాంకం sqlite3_సూచీకృత_By_Lookup(Parse *, Src_అంశం *);
శూన్య sqlite3_Src_List_జరుపు_Join_Type(Parse*,SrcList*);
శూన్య sqlite3_Src_List_కేటాయించు_Cursors(Parse*, SrcList*);
శూన్య sqlite3IdListDelete(sqlite3*, IdList*);
శూన్య sqlite3ClearOnOrUsing(sqlite3*, OnOrUsing*);
శూన్య sqlite3SrcListDelete(sqlite3*, SrcList*);
సూచీ *sqlite3_Allocate_సూచీ_Object(sqlite3*,పూర్ణాంకం,పూర్ణాంకం,అక్షరం**);
పూర్ణాంకం sqlite3_సూచీ_Bloomable(స్థిరరాశి సూచీ*,పూర్ణాంకం);
శూన్య sqlite3_Create_సూచీ(Parse*,Token*,Token*,SrcList*,సమాసం_List*,పూర్ణాంకం,Token*,
                          సమాసం*, పూర్ణాంకం, పూర్ణాంకం, u8);
శూన్య sqlite3_Drop_సూచీ(Parse*, SrcList*, పూర్ణాంకం);
పూర్ణాంకం sqlite3Select(Parse*, Select*, SelectDest*);
Select *sqlite3SelectNew(Parse*,సమాసం_List*,SrcList*,సమాసం*,సమాసం_List*,
                         సమాసం*,సమాసం_List*,u32,సమాసం*);
శూన్య sqlite3SelectDelete(sqlite3*, Select*);
శూన్య sqlite3SelectDeleteGeneric(sqlite3*,శూన్య*);
శూన్య sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect);
పట్టిక *sqlite3SrcListLookup(Parse*, SrcList*);
పూర్ణాంకం sqlite3IsReadOnly(Parse*, పట్టిక*, Trigger*);
శూన్య sqlite3_Open_పట్టిక(Parse*, పూర్ణాంకం iCur, పూర్ణాంకం iDb, పట్టిక*, పూర్ణాంకం);
#ఒకవేళ defined(SQLITE_ENABLE_UPDATE_DELETE_హద్దు) && !defined(SQLITE_OMIT_SUBQUERY)
సమాసం *sqlite3_హద్దు_Where(Parse*,SrcList*,సమాసం*,సమాసం_List*,సమాసం*,అక్షరం*);
#అయితే_ముగింపు
శూన్య sqlite3_సంకేతం_Change_లెక్క(Vdbe*,పూర్ణాంకం,స్థిరరాశి అక్షరం*);
శూన్య sqlite3DeleteFrom(Parse*, SrcList*, సమాసం*, సమాసం_List*, సమాసం*);
శూన్య sqlite3Update(Parse*, SrcList*, సమాసం_List*,సమాసం*,పూర్ణాంకం,సమాసం_List*,సమాసం*,
                   Upsert*);
WhereInfo *sqlite3WhereBegin(Parse*,SrcList*,సమాసం*,సమాసం_List*,
                             సమాసం_List*,Select*,u16,పూర్ణాంకం);
శూన్య sqlite3_Where_చివర(WhereInfo*);
LogEst sqlite3_Where_Output_అడ్డువరుస_లెక్క(WhereInfo*);
పూర్ణాంకం sqlite3WhereIsDistinct(WhereInfo*);
పూర్ణాంకం sqlite3WhereIsOrdered(WhereInfo*);
పూర్ణాంకం sqlite3_Where_క్రమం_By_హద్దు_Opt_Label(WhereInfo*);
శూన్య sqlite3_Where_Min_Max_Opt_Early_బహిర్గతం(Vdbe*,WhereInfo*);
పూర్ణాంకం sqlite3WhereIsSorted(WhereInfo*);
పూర్ణాంకం sqlite3WhereContinueLabel(WhereInfo*);
పూర్ణాంకం sqlite3WhereBreakLabel(WhereInfo*);
పూర్ణాంకం sqlite3WhereOkOnePass(WhereInfo*, పూర్ణాంకం*);
#నిర్వచించు ONEPASS_OFF      0        /* Use of ONEPASS not allowed */
#నిర్వచించు ONEPASS_SINGLE   1        /* ONEPASS valid for a single row update */
#నిర్వచించు ONEPASS_MULTI    2        /* ONEPASS is valid for multiple rows */
పూర్ణాంకం sqlite3_Where_ఉపయోగిస్తుంది_Deferred_Seek(WhereInfo*);
శూన్య sqlite3_సమాసం_సంకేతం_ఎక్కించు_సూచీ_స్తంభం(Parse*, సూచీ*, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం);
పూర్ణాంకం sqlite3_సమాసం_సంకేతం_Get_స్తంభం(Parse*, పట్టిక*, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం, u8);
శూన్య sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(Vdbe*, పట్టిక*, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం);
శూన్య sqlite3_సమాసం_సంకేతం_Move(Parse*, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం);
శూన్య sqlite3_సమాసం_గమ్యం_రిజిస్టరు(సమాసం *p_సమాసం, పూర్ణాంకం iReg);
శూన్య sqlite3_సమాసం_సంకేతం(Parse*, సమాసం*, పూర్ణాంకం);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
శూన్య sqlite3_సమాసం_సంకేతం_Generated_స్తంభం(Parse*, పట్టిక*, స్తంభం*, పూర్ణాంకం);
#అయితే_ముగింపు
శూన్య sqlite3_సమాసం_సంకేతం_Copy(Parse*, సమాసం*, పూర్ణాంకం);
శూన్య sqlite3_సమాసం_సంకేతం_Factorable(Parse*, సమాసం*, పూర్ణాంకం);
పూర్ణాంకం sqlite3_సమాసం_సంకేతం_నిరంతరం_Just_Once(Parse*, సమాసం*, పూర్ణాంకం);
శూన్య sqlite3_సమాసం_Null_రిజిస్టరు_Range(Parse*, పూర్ణాంకం, పూర్ణాంకం);
పూర్ణాంకం sqlite3_సమాసం_సంకేతం_తాత్కాలిక(Parse*, సమాసం*, పూర్ణాంకం*);
పూర్ణాంకం sqlite3_సమాసం_సంకేతం_లక్ష్యం(Parse*, సమాసం*, పూర్ణాంకం);
పూర్ణాంకం sqlite3_సమాసం_సంకేతం_సమాసం_List(Parse*, సమాసం_List*, పూర్ణాంకం, పూర్ణాంకం, u8);
#నిర్వచించు SQLITE_ECEL_DUP      0x01  /* Deep, not shallow copies */
#నిర్వచించు SQLITE_ECEL_గుణకం   0x02  /* Factor out constant terms */
#నిర్వచించు SQLITE_ECEL_REF      0x04  /* Use ExprList.u.x.iOrderByCol */
#నిర్వచించు SQLITE_ECEL_OMITREF  0x08  /* Omit if ExprList.u.x.iOrderByCol */
శూన్య sqlite3_సమాసం_If_True(Parse*, సమాసం*, పూర్ణాంకం, పూర్ణాంకం);
శూన్య sqlite3_సమాసం_If_False(Parse*, సమాసం*, పూర్ణాంకం, పూర్ణాంకం);
శూన్య sqlite3_సమాసం_If_False_Dup(Parse*, సమాసం*, పూర్ణాంకం, పూర్ణాంకం);
పట్టిక *sqlite3_కనుగొను_పట్టిక(sqlite3*,స్థిరరాశి అక్షరం*, స్థిరరాశి అక్షరం*);
#నిర్వచించు LOCATE_VIEW    0x01
#నిర్వచించు LOCATE_NOERR   0x02
పట్టిక *sqlite3_Locate_పట్టిక(Parse*,u32 flags,స్థిరరాశి అక్షరం*, స్థిరరాశి అక్షరం*);
స్థిరరాశి అక్షరం *sqlite3_Preferred_పట్టిక_Name(స్థిరరాశి అక్షరం*);
పట్టిక *sqlite3_Locate_పట్టిక_అంశం(Parse*,u32 flags,Src_అంశం *);
సూచీ *sqlite3_కనుగొను_సూచీ(sqlite3*,స్థిరరాశి అక్షరం*, స్థిరరాశి అక్షరం*);
శూన్య sqlite3_Unlink_And_Delete_పట్టిక(sqlite3*,పూర్ణాంకం,స్థిరరాశి అక్షరం*);
శూన్య sqlite3_Unlink_And_Delete_సూచీ(sqlite3*,పూర్ణాంకం,స్థిరరాశి అక్షరం*);
శూన్య sqlite3Vacuum(Parse*,Token*,సమాసం*);
పూర్ణాంకం sqlite3_నిరంతరం_Vacuum(అక్షరం**, sqlite3*, పూర్ణాంకం, sqlite3_విలువ*);
అక్షరం *sqlite3NameFromToken(sqlite3*, స్థిరరాశి Token*);
పూర్ణాంకం sqlite3_సమాసం_Compare(స్థిరరాశి Parse*,స్థిరరాశి సమాసం*,స్థిరరాశి సమాసం*, పూర్ణాంకం);
పూర్ణాంకం sqlite3_సమాసం_Compare_Skip(సమాసం*,సమాసం*,పూర్ణాంకం);
పూర్ణాంకం sqlite3_సమాసం_List_Compare(స్థిరరాశి సమాసం_List*,స్థిరరాశి సమాసం_List*, పూర్ణాంకం);
పూర్ణాంకం sqlite3_సమాసం_సూచిస్తుంది_సమాసం(స్థిరరాశి Parse*,స్థిరరాశి సమాసం*,స్థిరరాశి సమాసం*, పూర్ణాంకం);
పూర్ణాంకం sqlite3_సమాసం_సూచిస్తుంది_Non_Null_అడ్డువరుస(సమాసం*,పూర్ణాంకం,పూర్ణాంకం);
శూన్య sqlite3AggInfoPersistWalkerInit(Walker*,Parse*);
శూన్య sqlite3_సమాసం_Analyze_Aggregates(NameContext*, సమాసం*);
శూన్య sqlite3_సమాసం_Analyze_Agg_List(NameContext*,సమాసం_List*);
పూర్ణాంకం sqlite3_సమాసం_Covered_By_సూచీ(సమాసం*, పూర్ణాంకం iCur, సూచీ *pIdx);
పూర్ణాంకం sqlite3ReferencesSrcList(Parse*, సమాసం*, SrcList*);
Vdbe *sqlite3GetVdbe(Parse*);
#నిర్వచితం_కాకపోతే SQLITE_UNTESTABLE
శూన్య sqlite3PrngSaveState(శూన్య);
శూన్య sqlite3PrngRestoreState(శూన్య);
#అయితే_ముగింపు
శూన్య sqlite3RollbackAll(sqlite3*,పూర్ణాంకం);
శూన్య sqlite3_సంకేతం_Verify_Schema(Parse*, పూర్ణాంకం);
శూన్య sqlite3_సంకేతం_Verify_Named_Schema(Parse*, స్థిరరాశి అక్షరం *zDb);
శూన్య sqlite3BeginTransaction(Parse*, పూర్ణాంకం);
శూన్య sqlite3_చివర_Transaction(Parse*,పూర్ణాంకం);
శూన్య sqlite3Savepoint(Parse*, పూర్ణాంకం, Token*);
శూన్య sqlite3CloseSavepoints(sqlite3 *);
శూన్య sqlite3LeaveMutexAndCloseZombie(sqlite3*);
u32 sqlite3IsTrueOrFalse(స్థిరరాశి అక్షరం*);
పూర్ణాంకం sqlite3_సమాసం_Id_గమ్యం_True_False(సమాసం*);
పూర్ణాంకం sqlite3_సమాసం_Truth_విలువ(స్థిరరాశి సమాసం*);
పూర్ణాంకం sqlite3_సమాసం_Is_Constant(Parse*,సమాసం*);
పూర్ణాంకం sqlite3_సమాసం_Is_Constant_Or_Function(సమాసం*, u8);
పూర్ణాంకం sqlite3_సమాసం_Is_Constant_Or_సమూహం_By(Parse*, సమాసం*, సమాసం_List*);
పూర్ణాంకం sqlite3_సమాసం_Is_పట్టిక_Constant(సమాసం*,పూర్ణాంకం,పూర్ణాంకం);
పూర్ణాంకం sqlite3_సమాసం_Is_Single_పట్టిక_Constraint(సమాసం*,స్థిరరాశి SrcList*,పూర్ణాంకం,పూర్ణాంకం);
పూర్ణాంకం sqlite3_సమాసం_List_Is_Constant(Parse *pParse, సమాసం_List *pList, పూర్ణాంకం bNoIs);
పూర్ణాంకం sqlite3_సమాసం_కలిగియుంది_Subquery(సమాసం*);
పూర్ణాంకం sqlite3_సమాసం_Is_Integer(స్థిరరాశి సమాసం*, పూర్ణాంకం*, Parse*, పూర్ణాంకం);
పూర్ణాంకం sqlite3_సమాసం_Can_Be_Null(స్థిరరాశి సమాసం*);
పూర్ణాంకం sqlite3_సమాసం_Needs_No_Affinity_Change(స్థిరరాశి సమాసం*, అక్షరం);
పూర్ణాంకం sqlite3_సమాసం_Is_Like_Operator(స్థిరరాశి సమాసం*);
పూర్ణాంకం sqlite3IsRowid(స్థిరరాశి అక్షరం*);
స్థిరరాశి అక్షరం *sqlite3RowidAlias(పట్టిక *pTab);
శూన్య sqlite3_Generate_అడ్డువరుస_Delete(
    Parse*,పట్టిక*,Trigger*,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం,i16,u8,u8,u8,పూర్ణాంకం);
శూన్య sqlite3_Generate_అడ్డువరుస_సూచీ_Delete(Parse*, పట్టిక*, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం*, పూర్ణాంకం);
పూర్ణాంకం sqlite3_Generate_సూచీ_కీలకం(Parse*, సూచీ*, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం*,సూచీ*,పూర్ణాంకం);
శూన్య sqlite3ResolvePartIdxLabel(Parse*,పూర్ణాంకం);
పూర్ణాంకం sqlite3_సమాసం_References_Updated_స్తంభం(సమాసం*,పూర్ణాంకం*,పూర్ణాంకం);
శూన్య sqlite3GenerateConstraintChecks(Parse*,పట్టిక*,పూర్ణాంకం*,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం,
                                     u8,u8,పూర్ణాంకం,పూర్ణాంకం*,పూర్ణాంకం*,Upsert*);
#నిర్వచితమైతే SQLITE_ENABLE_NULL_TRIM
  శూన్య sqlite3_సమితి_Make_రికార్డు_P5(Vdbe*,పట్టిక*);
#లేకపోతే
# define sqlite3_సమితి_Make_రికార్డు_P5(A,B)
#అయితే_ముగింపు
శూన్య sqlite3CompleteInsertion(Parse*,పట్టిక*,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం*,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం);
పూర్ణాంకం sqlite3_Open_పట్టిక_And_Indices(Parse*, పట్టిక*, పూర్ణాంకం, u8, పూర్ణాంకం, u8*, పూర్ణాంకం*, పూర్ణాంకం*);
శూన్య sqlite3BeginWriteOperation(Parse*, పూర్ణాంకం, పూర్ణాంకం);
శూన్య sqlite3MultiWrite(Parse*);
శూన్య sqlite3MayAbort(Parse*);
శూన్య sqlite3HaltConstraint(Parse*, పూర్ణాంకం, పూర్ణాంకం, అక్షరం*, i8, u8);
శూన్య sqlite3UniqueConstraint(Parse*, పూర్ణాంకం, సూచీ*);
శూన్య sqlite3RowidConstraint(Parse*, పూర్ణాంకం, పట్టిక*);
సమాసం *sqlite3_సమాసం_Dup(sqlite3*,స్థిరరాశి సమాసం*,పూర్ణాంకం);
సమాసం_List *sqlite3_సమాసం_List_Dup(sqlite3*,స్థిరరాశి సమాసం_List*,పూర్ణాంకం);
SrcList *sqlite3SrcListDup(sqlite3*,స్థిరరాశి SrcList*,పూర్ణాంకం);
IdList *sqlite3IdListDup(sqlite3*,స్థిరరాశి IdList*);
Select *sqlite3SelectDup(sqlite3*,స్థిరరాశి Select*,పూర్ణాంకం);
FuncDef *sqlite3_Function_శోధన(పూర్ణాంకం,స్థిరరాశి అక్షరం*);
శూన్య sqlite3_చొప్పించు_Builtin_Funcs(FuncDef*,పూర్ణాంకం);
FuncDef *sqlite3_కనుగొను_Function(sqlite3*,స్థిరరాశి అక్షరం*,పూర్ణాంకం,u8,u8);
శూన్య sqlite3_Quote_విలువ(StrAccum*,sqlite3_విలువ*,పూర్ణాంకం);
పూర్ణాంకం sqlite3AppendOneUtf8Character(అక్షరం*, u32);
శూన్య sqlite3_రిజిస్టరు_Builtin_Functions(శూన్య);
శూన్య sqlite3_రిజిస్టరు_Date_సమయం_Functions(శూన్య);
శూన్య sqlite3_రిజిస్టరు_Json_Functions(శూన్య);
శూన్య sqlite3_రిజిస్టరు_చొప్పున_Connection_Builtin_Functions(sqlite3*);
#ఒకవేళ !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
  Module *sqlite3_Json_Vtab_రిజిస్టరు(sqlite3*,స్థిరరాశి అక్షరం*);
#అయితే_ముగింపు
పూర్ణాంకం sqlite3SafetyCheckOk(sqlite3*);
పూర్ణాంకం sqlite3SafetyCheckSickOrOk(sqlite3*);
శూన్య sqlite3ChangeCookie(Parse*, పూర్ణాంకం);
With *sqlite3WithDup(sqlite3 *db, With *p);

#ఒకవేళ !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
  Module *sqlite3_Carray_రిజిస్టరు(sqlite3*);
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
శూన్య sqlite3MaterializeView(Parse*, పట్టిక*, సమాసం*, సమాసం_List*,సమాసం*,పూర్ణాంకం);
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
  శూన్య sqlite3BeginTrigger(Parse*, Token*,Token*,పూర్ణాంకం,పూర్ణాంకం,IdList*,SrcList*,
                           సమాసం*,పూర్ణాంకం, పూర్ణాంకం);
  శూన్య sqlite3_ముగింపు_Trigger(Parse*, Trigger_అడుగు*, Token*);
  శూన్య sqlite3DropTrigger(Parse*, SrcList*, పూర్ణాంకం);
  శూన్య sqlite3DropTriggerPtr(Parse*, Trigger*);
  Trigger *sqlite3TriggersExist(Parse *, పట్టిక*, పూర్ణాంకం, సమాసం_List*, పూర్ణాంకం *p_ముసుగు);
  Trigger *sqlite3TriggerList(Parse *, పట్టిక *);
  శూన్య sqlite3_సంకేతం_అడ్డువరుస_Trigger(Parse*, Trigger *, పూర్ణాంకం, సమాసం_List*, పూర్ణాంకం, పట్టిక *,
                            పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం);
  శూన్య sqlite3_సంకేతం_అడ్డువరుస_Trigger_ప్రత్యక్ష(Parse *, Trigger *, పట్టిక *, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం);
  శూన్య sqliteViewTriggers(Parse*, పట్టిక*, సమాసం*, పూర్ణాంకం, సమాసం_List*);
  శూన్య sqlite3_Delete_Trigger_అడుగు(sqlite3*, Trigger_అడుగు*);
  Trigger_అడుగు *sqlite3_Trigger_Select_అడుగు(sqlite3*,Select*,
                                        స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*);
  Trigger_అడుగు *sqlite3_Trigger_చొప్పించు_అడుగు(Parse*,SrcList*, IdList*,
                                        Select*,u8,Upsert*,
                                        స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*);
  Trigger_అడుగు *sqlite3_Trigger_Update_అడుగు(Parse*,SrcList*,SrcList*,సమాసం_List*,
                                        సమాసం*, u8, స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*);
  Trigger_అడుగు *sqlite3_Trigger_Delete_అడుగు(Parse*,SrcList*, సమాసం*,
                                        స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*);
  శూన్య sqlite3DeleteTrigger(sqlite3*, Trigger*);
  శూన్య sqlite3UnlinkAndDeleteTrigger(sqlite3*,పూర్ణాంకం,స్థిరరాశి అక్షరం*);
  u32 sqlite3TriggerColmask(Parse*,Trigger*,సమాసం_List*,పూర్ణాంకం,పూర్ణాంకం,పట్టిక*,పూర్ణాంకం);
# define sqlite3ParseToplevel(p) ((p)->pToplevel ? (p)->pToplevel : (p))
# define sqlite3IsToplevel(p) ((p)->pToplevel==0)
#లేకపోతే
# define sqlite3TriggersExist(B,C,D,E,F) 0
# define sqlite3DeleteTrigger(A,B)
# define sqlite3DropTriggerPtr(A,B)
# define sqlite3UnlinkAndDeleteTrigger(A,B,C)
# define sqlite3_సంకేతం_అడ్డువరుస_Trigger(A,B,C,D,E,F,G,H,I)
# define sqlite3_సంకేతం_అడ్డువరుస_Trigger_ప్రత్యక్ష(A,B,C,D,E,F)
# define sqlite3TriggerList(X, Y) 0
# define sqlite3ParseToplevel(p) p
# define sqlite3IsToplevel(p) 1
# define sqlite3TriggerColmask(A,B,C,D,E,F,G) 0
#అయితే_ముగింపు

పూర్ణాంకం sqlite3JoinType(Parse*, Token*, Token*, Token*);
పూర్ణాంకం sqlite3_స్తంభం_సూచీ(పట్టిక *pTab, స్థిరరాశి అక్షరం *z_నిలువు);
శూన్య sqlite3_Src_అంశం_స్తంభం_వాడిన(Src_అంశం*,పూర్ణాంకం);
శూన్య sqlite3_సమితి_Join_సమాసం(సమాసం*,పూర్ణాంకం,u32);
శూన్య sqlite3_Create_Foreign_కీలకం(Parse*, సమాసం_List*, Token*, సమాసం_List*, పూర్ణాంకం);
శూన్య sqlite3_Defer_Foreign_కీలకం(Parse*, పూర్ణాంకం);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
  శూన్య sqlite3AuthRead(Parse*,సమాసం*,Schema*,SrcList*);
  పూర్ణాంకం sqlite3AuthCheck(Parse*,పూర్ణాంకం, స్థిరరాశి అక్షరం*, స్థిరరాశి అక్షరం*, స్థిరరాశి అక్షరం*);
  శూన్య sqlite3_Auth_Context_నెట్టు(Parse*, AuthContext*, స్థిరరాశి అక్షరం*);
  శూన్య sqlite3_Auth_Context_తీయి(AuthContext*);
  పూర్ణాంకం sqlite3_Auth_Read_నిలువు(Parse*, స్థిరరాశి అక్షరం *, స్థిరరాశి అక్షరం *, పూర్ణాంకం);
#లేకపోతే
# define sqlite3AuthRead(a,b,c,d)
# define sqlite3AuthCheck(a,b,c,d,e)    SQLITE_OK
# define sqlite3_Auth_Context_నెట్టు(a,b,c)
# define sqlite3_Auth_Context_తీయి(a)  ((శూన్య)(a))
#అయితే_ముగింపు
పూర్ణాంకం sqlite3DbIsNamed(sqlite3 *db, పూర్ణాంకం iDb, స్థిరరాశి అక్షరం *zName);
శూన్య sqlite3Attach(Parse*, సమాసం*, సమాసం*, సమాసం*);
శూన్య sqlite3Detach(Parse*, సమాసం*);
శూన్య sqlite3FixInit(DbFixer*, Parse*, పూర్ణాంకం, స్థిరరాశి అక్షరం*, స్థిరరాశి Token*);
పూర్ణాంకం sqlite3FixSrcList(DbFixer*, SrcList*);
పూర్ణాంకం sqlite3FixSelect(DbFixer*, Select*);
పూర్ణాంకం sqlite3_Fix_సమాసం(DbFixer*, సమాసం*);
పూర్ణాంకం sqlite3_Fix_Trigger_అడుగు(DbFixer*, Trigger_అడుగు*);

పూర్ణాంకం sqlite3RealSameAsInt(ద్విగుణ,sqlite3_int64);
i64 sqlite3_Real_గమ్యం_I64(ద్విగుణ);
పూర్ణాంకం sqlite3_Int64_గమ్యం_పాఠ్యం(i64,అక్షరం*);
పూర్ణాంకం sqlite3AtoF(స్థిరరాశి అక్షరం *z, ద్విగుణ*);
పూర్ణాంకం sqlite3GetInt32(స్థిరరాశి అక్షరం *, పూర్ణాంకం*);
పూర్ణాంకం sqlite3GetUInt32(స్థిరరాశి అక్షరం*, u32*);
పూర్ణాంకం sqlite3Atoi(స్థిరరాశి అక్షరం*);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
పూర్ణాంకం sqlite3Utf16ByteLen(స్థిరరాశి శూన్య *p_దత్తాంశం, పూర్ణాంకం nByte, పూర్ణాంకం nChar);
#అయితే_ముగింపు
పూర్ణాంకం sqlite3Utf8CharLen(స్థిరరాశి అక్షరం *p_దత్తాంశం, పూర్ణాంకం nByte);
u32 sqlite3Utf8Read(స్థిరరాశి u8**);
పూర్ణాంకం sqlite3Utf8ReadLimited(స్థిరరాశి u8*, పూర్ణాంకం, u32*);
LogEst sqlite3LogEst(u64);
LogEst sqlite3_Log_Est_కూడు(LogEst,LogEst);
LogEst sqlite3LogEstFromDouble(ద్విగుణ);
u64 sqlite3_Log_Est_గమ్యం_Int(LogEst);
VList *sqlite3_VList_కూడు(sqlite3*,VList*,స్థిరరాశి అక్షరం*,పూర్ణాంకం,పూర్ణాంకం);
స్థిరరాశి అక్షరం *sqlite3_VList_అంకె_గమ్యం_Name(VList*,పూర్ణాంకం);
పూర్ణాంకం sqlite3_VList_Name_గమ్యం_అంకె(VList*,స్థిరరాశి అక్షరం*,పూర్ణాంకం);

/*
** Routines to read and write variable-length integers.  These used to
** be defined locally, but now we use the varint routines in the util.c
** file.
*/
పూర్ణాంకం sqlite3PutVarint(గుర్తులేని అక్షరం*, u64);
u8 sqlite3GetVarint(స్థిరరాశి గుర్తులేని అక్షరం *, u64 *);
i64 sqlite3_Varint_విలువ(స్థిరరాశి గుర్తులేని అక్షరం*);
u8 sqlite3GetVarint32(స్థిరరాశి గుర్తులేని అక్షరం *, u32 *);
పూర్ణాంకం sqlite3VarintLen(u64 v);

/*
** The common case is for a varint to be a single byte.  They following
** macros handle the common case without a procedure call, but then call
** the procedure for larger varints.
*/
#నిర్వచించు getVarint32(A,B)  \
  (u8)((*(A)<(u8)0x80)?((B)=(u32)*(A)),1:sqlite3GetVarint32((A),(u32 *)&(B)))
#నిర్వచించు getVarint32NR(A,B) \
  B=(u32)*(A);ఒకవేళ(B>=0x80)sqlite3GetVarint32((A),(u32*)&(B))
#నిర్వచించు putVarint32(A,B)  \
  (u8)(((u32)(B)<(u32)0x80)?(*(A)=(గుర్తులేని అక్షరం)(B)),1:\
  sqlite3PutVarint((A),(B)))

స్థిరరాశి అక్షరం *sqlite3_సూచీ_Affinity_Str(sqlite3*, సూచీ*);
అక్షరం *sqlite3_పట్టిక_Affinity_Str(sqlite3*,స్థిరరాశి పట్టిక*);
శూన్య sqlite3_పట్టిక_Affinity(Vdbe*, పట్టిక*, పూర్ణాంకం);
అక్షరం sqlite3CompareAffinity(స్థిరరాశి సమాసం *p_సమాసం, అక్షరం aff2);
పూర్ణాంకం sqlite3_సూచీ_Affinity_Ok(స్థిరరాశి సమాసం *p_సమాసం, అక్షరం idx_affinity);
అక్షరం sqlite3_పట్టిక_స్తంభం_Affinity(స్థిరరాశి పట్టిక*,పూర్ణాంకం);
అక్షరం sqlite3_సమాసం_Affinity(స్థిరరాశి సమాసం *p_సమాసం);
పూర్ణాంకం sqlite3_సమాసం_దత్తాంశం_Type(స్థిరరాశి సమాసం *p_సమాసం);
పూర్ణాంకం sqlite3Atoi64(స్థిరరాశి అక్షరం*, i64*, పూర్ణాంకం, u8);
పూర్ణాంకం sqlite3_Dec_Or_Hex_గమ్యం_I64(స్థిరరాశి అక్షరం*, i64*);
శూన్య sqlite3_Error_With_సందేశం(sqlite3*, పూర్ణాంకం, స్థిరరాశి అక్షరం*,...);
శూన్య sqlite3Error(sqlite3*,పూర్ణాంకం);
శూన్య sqlite3ErrorClear(sqlite3*);
శూన్య sqlite3SystemError(sqlite3*,పూర్ణాంకం);
#ఒకవేళ !defined(SQLITE_OMIT_BLOB_LITERAL)
శూన్య *sqlite3_Hex_గమ్యం_Blob(sqlite3*, స్థిరరాశి అక్షరం *z, పూర్ణాంకం n);
#అయితే_ముగింపు
u8 sqlite3_Hex_గమ్యం_Int(పూర్ణాంకం h);
పూర్ణాంకం sqlite3TwoPartName(Parse *, Token *, Token *, Token **);

#ఒకవేళ defined(SQLITE_NEED_ERR_NAME)
స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DESERIALIZE
పూర్ణాంకం sqlite3MemdbInit(శూన్య);
పూర్ణాంకం sqlite3IsMemdb(స్థిరరాశి sqlite3_vfs*);
#లేకపోతే
# define sqlite3IsMemdb(X) 0
#అయితే_ముగింపు

స్థిరరాశి అక్షరం *sqlite3ErrStr(పూర్ణాంకం);
పూర్ణాంకం sqlite3ReadSchema(Parse *pParse);
CollSeq *sqlite3_కనుగొను_Coll_Seq(sqlite3*,u8 enc, స్థిరరాశి అక్షరం*,పూర్ణాంకం);
పూర్ణాంకం sqlite3_ద్విమాన_Compare(శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*);
#ఒకవేళ SQLITE_ఉపయోగిస్తుంది_INLINE
స్థిర SQLITE_INLINE పూర్ణాంకం sqlite3_Is_ద్విమాన(స్థిరరాశి CollSeq *p){
  ప్రతిజ్ఞా( p==0 || p->xCmp!=sqlite3_ద్విమాన_Compare
       || strcmp(p->zName,"BINARY")==0 );
  తిరిగి p==0 || p->xCmp==sqlite3_ద్విమాన_Compare;
}
#లేకపోతే
పూర్ణాంకం sqlite3_Is_ద్విమాన(స్థిరరాశి CollSeq*);
#అయితే_ముగింపు
CollSeq *sqlite3LocateCollSeq(Parse *pParse, స్థిరరాశి అక్షరం*zName);
శూన్య sqlite3_సమితి_పాఠ్యం_Encoding(sqlite3 *db, u8);
CollSeq *sqlite3_సమాసం_Coll_Seq(Parse *pParse, స్థిరరాశి సమాసం *p_సమాసం);
CollSeq *sqlite3_సమాసం_NNColl_Seq(Parse *pParse, స్థిరరాశి సమాసం *p_సమాసం);
పూర్ణాంకం sqlite3_సమాసం_Coll_Seq_సరిపోలిక(Parse*,స్థిరరాశి సమాసం*,స్థిరరాశి సమాసం*);
సమాసం *sqlite3_సమాసం_కూడు_Collate_Token(Parse *pParse, సమాసం*, స్థిరరాశి Token*, పూర్ణాంకం);
సమాసం *sqlite3_సమాసం_కూడు_Collate_String(Parse*,సమాసం*,స్థిరరాశి అక్షరం*);
సమాసం *sqlite3_సమాసం_Skip_Collate(సమాసం*);
సమాసం *sqlite3_సమాసం_Skip_Collate_And_Likely(సమాసం*);
పూర్ణాంకం sqlite3CheckCollSeq(Parse *, CollSeq *);
పూర్ణాంకం sqlite3WritableSchema(sqlite3*);
పూర్ణాంకం sqlite3CheckObjectName(Parse*, స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*,స్థిరరాశి అక్షరం*);
శూన్య sqlite3_Vdbe_సమితి_Changes(sqlite3 *, i64);
పూర్ణాంకం sqlite3_కూడు_Int64(i64*,i64);
పూర్ణాంకం sqlite3SubInt64(i64*,i64);
పూర్ణాంకం sqlite3MulInt64(i64*,i64);
పూర్ణాంకం sqlite3AbsInt32(పూర్ణాంకం);
#నిర్వచితమైతే SQLITE_ENABLE_8_3_పేర్లు
శూన్య sqlite3FileSuffix3(స్థిరరాశి అక్షరం*, అక్షరం*);
#లేకపోతే
# define sqlite3FileSuffix3(X,Y)
#అయితే_ముగింపు
u8 sqlite3GetBoolean(స్థిరరాశి అక్షరం *z,u8);

స్థిరరాశి శూన్య *sqlite3_విలువ_పాఠ్యం_2(sqlite3_విలువ*, u8);
పూర్ణాంకం sqlite3_విలువ_Is_యొక్క_Class(స్థిరరాశి sqlite3_విలువ*, శూన్య(*)(శూన్య*));
పూర్ణాంకం sqlite3_విలువ_బైట్లు_2(sqlite3_విలువ*, u8);
శూన్య sqlite3_విలువ_సమితి_Str(sqlite3_విలువ*, పూర్ణాంకం, స్థిరరాశి శూన్య *,u8,
                        శూన్య(*)(శూన్య*));
శూన్య sqlite3_విలువ_సమితి_Null(sqlite3_విలువ*);
శూన్య sqlite3_విలువ_Free(sqlite3_విలువ*);
#నిర్వచితం_కాకపోతే SQLITE_UNTESTABLE
శూన్య sqlite3_ఫలితం_Int_Real(sqlite3_context*);
#అయితే_ముగింపు
sqlite3_విలువ *sqlite3_విలువ_New(sqlite3 *);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UTF16
అక్షరం *sqlite3Utf16to8(sqlite3 *, స్థిరరాశి శూన్య*, పూర్ణాంకం, u8);
#అయితే_ముగింపు
పూర్ణాంకం sqlite3_విలువ_From_సమాసం(sqlite3 *, స్థిరరాశి సమాసం *, u8, u8, sqlite3_విలువ **);
శూన్య sqlite3_విలువ_Apply_Affinity(sqlite3_విలువ *, u8, u8);
#నిర్వచితం_కాకపోతే SQLITE_AMALGAMATION
బాహ్య స్థిరరాశి గుర్తులేని అక్షరం sqlite3OpcodeProperty[];
బాహ్య స్థిరరాశి అక్షరం sqlite3_Str_ద్విమాన[];
బాహ్య స్థిరరాశి గుర్తులేని అక్షరం sqlite3StdTypeLen[];
బాహ్య స్థిరరాశి అక్షరం sqlite3StdTypeAffinity[];
బాహ్య స్థిరరాశి అక్షరం *sqlite3StdType[];
బాహ్య స్థిరరాశి గుర్తులేని అక్షరం sqlite3_పై_గమ్యం_క్రింది[];
బాహ్య స్థిరరాశి గుర్తులేని అక్షరం *sqlite3aLTb;
బాహ్య స్థిరరాశి గుర్తులేని అక్షరం *sqlite3aEQb;
బాహ్య స్థిరరాశి గుర్తులేని అక్షరం *sqlite3aGTb;
బాహ్య స్థిరరాశి గుర్తులేని అక్షరం sqlite3CtypeMap[];
బాహ్య SQLITE_WSD నిర్మాణం Sqlite3Config sqlite3Config;
బాహ్య FuncDefHash sqlite3BuiltinFunctions;
బాహ్య స్థిరరాశి sqlite3_str sqlite3OomStr;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WSD
బాహ్య పూర్ణాంకం sqlite3PendingByte;
#అయితే_ముగింపు
#అయితే_ముగింపు /* SQLITE_AMALGAMATION */
#నిర్వచితమైతే VDBE_PROFILE
బాహ్య sqlite3_uint64 sqlite3NProfileCnt;
#అయితే_ముగింపు
శూన్య sqlite3_మూలం_పుట_Moved(sqlite3*, పూర్ణాంకం, Pgno, Pgno);
శూన్య sqlite3Reindex(Parse*, Token*, Token*);
శూన్య sqlite3AlterFunctions(శూన్య);
శూన్య sqlite3_Alter_Rename_పట్టిక(Parse*, SrcList*, Token*);
శూన్య sqlite3_Alter_Rename_స్తంభం(Parse*, SrcList*, Token*, Token*);
శూన్య sqlite3AlterDropConstraint(Parse*,SrcList*,Token*,Token*);
శూన్య sqlite3_Alter_కూడు_Constraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *p_మొదటి,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  స్థిరరాశి అక్షరం *z_సమాసం,       /* Text of CHECK expression */
  పూర్ణాంకం n_సమాసం,               /* Size of pExpr in bytes */
  సమాసం *p_సమాసం              /* The parsed CHECK expression */
);
శూన్య sqlite3_Alter_సమితి_Not_Null(Parse*, SrcList*, Token*, Token*);
i64 sqlite3GetToken(స్థిరరాశి గుర్తులేని అక్షరం *, పూర్ణాంకం *);
శూన్య sqlite3NestedParse(Parse*, స్థిరరాశి అక్షరం*, ...);
శూన్య sqlite3ExpirePreparedStatements(sqlite3*, పూర్ణాంకం);
శూన్య sqlite3_సంకేతం_Rhs_యొక్క_లోపల(Parse*, సమాసం*, పూర్ణాంకం, పూర్ణాంకం);
పూర్ణాంకం sqlite3_సంకేతం_Subselect(Parse*, సమాసం*);
శూన్య sqlite3SelectPrep(Parse*, Select*, NameContext*);
పూర్ణాంకం sqlite3ExpandSubquery(Parse*, Src_అంశం*);
శూన్య sqlite3_Select_Wrong_అంకె_Terms_Error(Parse *pParse, Select *p);
పూర్ణాంకం sqlite3_సరిపోలిక_EName(
  స్థిరరాశి నిర్మాణం సమాసం_List_అంశం*,
  స్థిరరాశి అక్షరం*,
  స్థిరరాశి అక్షరం*,
  స్థిరరాశి అక్షరం*,
  పూర్ణాంకం*
);
Bitmask sqlite3_సమాసం_నిలువు_వాడిన(సమాసం*);
u8 sqlite3StrIHash(స్థిరరాశి అక్షరం*);
పూర్ణాంకం sqlite3_Resolve_సమాసం_పేర్లు(NameContext*, సమాసం*);
పూర్ణాంకం sqlite3_Resolve_సమాసం_List_పేర్లు(NameContext*, సమాసం_List*);
శూన్య sqlite3_Resolve_Select_పేర్లు(Parse*, Select*, NameContext*);
పూర్ణాంకం sqlite3ResolveSelfReference(Parse*,పట్టిక*,పూర్ణాంకం,సమాసం*,సమాసం_List*);
పూర్ణాంకం sqlite3_Resolve_క్రమం_సమూహం_By(Parse*, Select*, సమాసం_List*, స్థిరరాశి అక్షరం*);
శూన్య sqlite3_స్తంభం_Default(Vdbe *, పట్టిక *, పూర్ణాంకం, పూర్ణాంకం);
శూన్య sqlite3_Alter_ముగింపు_కూడు_స్తంభం(Parse *, Token *);
శూన్య sqlite3_Alter_Begin_కూడు_స్తంభం(Parse *, SrcList *);
శూన్య sqlite3_Alter_Drop_స్తంభం(Parse*, SrcList*, స్థిరరాశి Token*);
స్థిరరాశి శూన్య *sqlite3RenameTokenMap(Parse*, స్థిరరాశి శూన్య*, స్థిరరాశి Token*);
శూన్య sqlite3RenameTokenRemap(Parse*, స్థిరరాశి శూన్య *p_గమ్యం, స్థిరరాశి శూన్య *pFrom);
శూన్య sqlite3_Rename_సమాసం_Unmap(Parse*, సమాసం*);
శూన్య sqlite3RenameExprlistUnmap(Parse*, సమాసం_List*);
CollSeq *sqlite3GetCollSeq(Parse*, u8, CollSeq *, స్థిరరాశి అక్షరం*);
అక్షరం sqlite3AffinityType(స్థిరరాశి అక్షరం*, స్తంభం*);
శూన్య sqlite3Analyze(Parse*, Token*, Token*);
పూర్ణాంకం sqlite3InvokeBusyHandler(BusyHandler*);
పూర్ణాంకం sqlite3_కనుగొను_Db(sqlite3*, Token*);
పూర్ణాంకం sqlite3_కనుగొను_Db_Name(sqlite3 *, స్థిరరాశి అక్షరం *);
పూర్ణాంకం sqlite3_Analysis_ఎక్కించు(sqlite3*,పూర్ణాంకం iDB);
శూన్య sqlite3_Delete_సూచీ_Samples(sqlite3*,సూచీ*);
శూన్య sqlite3_Default_అడ్డువరుస_Est(సూచీ*);
శూన్య sqlite3_రిజిస్టరు_Like_Functions(sqlite3*, పూర్ణాంకం);
పూర్ణాంకం sqlite3IsLikeFunction(sqlite3*,సమాసం*,పూర్ణాంకం*,అక్షరం*);
శూన్య sqlite3SchemaClear(శూన్య *);
Schema *sqlite3SchemaGet(sqlite3 *, Btree *);
పూర్ణాంకం sqlite3_Schema_గమ్యం_సూచీ(sqlite3 *db, Schema *);
కీలకం_Info *sqlite3_కీలకం_Info_Alloc(sqlite3*,పూర్ణాంకం,పూర్ణాంకం);
శూన్య sqlite3_కీలకం_Info_Unref(కీలకం_Info*);
కీలకం_Info *sqlite3_కీలకం_Info_Ref(కీలకం_Info*);
కీలకం_Info *sqlite3_కీలకం_Info_యొక్క_సూచీ(Parse*, సూచీ*);
కీలకం_Info *sqlite3_కీలకం_Info_From_సమాసం_List(Parse*, సమాసం_List*, పూర్ణాంకం, పూర్ణాంకం);
స్థిరరాశి అక్షరం *sqlite3SelectOpName(పూర్ణాంకం);
పూర్ణాంకం sqlite3HasExplicitNulls(Parse*, సమాసం_List*);

#నిర్వచితమైతే SQLITE_DEBUG
పూర్ణాంకం sqlite3_కీలకం_Info_Is_Writeable(కీలకం_Info*);
#అయితే_ముగింపు
పూర్ణాంకం sqlite3CreateFunc(sqlite3 *, స్థిరరాశి అక్షరం *, పూర్ణాంకం, పూర్ణాంకం, శూన్య *,
  శూన్య (*)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ **),
  శూన్య (*)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ **),
  శూన్య (*)(sqlite3_context*),
  శూన్య (*)(sqlite3_context*),
  శూన్య (*)(sqlite3_context*,పూర్ణాంకం,sqlite3_విలువ **),
  FuncDestructor *pDestructor
);
శూన్య sqlite3NoopDestructor(శూన్య*);
శూన్య *sqlite3OomFault(sqlite3*);
శూన్య sqlite3OomClear(sqlite3*);
పూర్ణాంకం sqlite3ApiExit(sqlite3 *db, పూర్ణాంకం);
పూర్ణాంకం sqlite3_Open_తాత్కాలిక_Database(Parse *);

అక్షరం *sqlite3RCStrRef(అక్షరం*);
శూన్య sqlite3RCStrUnref(శూన్య*);
అక్షరం *sqlite3RCStrNew(u64);
అక్షరం *sqlite3RCStrResize(అక్షరం*,u64);

శూన్య sqlite3StrAccumInit(StrAccum*, sqlite3*, అక్షరం*, పూర్ణాంకం, పూర్ణాంకం);
పూర్ణాంకం sqlite3StrAccumEnlarge(StrAccum*, i64);
పూర్ణాంకం sqlite3StrAccumEnlargeIfNeeded(StrAccum*, i64);
అక్షరం *sqlite3_Str_Accum_ముగింపు(StrAccum*);
శూన్య sqlite3_Str_Accum_సమితి_Error(StrAccum*, u8);
శూన్య sqlite3SelectDestInit(SelectDest*,పూర్ణాంకం,పూర్ణాంకం);
సమాసం *sqlite3_Create_స్తంభం_సమాసం(sqlite3 *, SrcList *, పూర్ణాంకం, పూర్ణాంకం);
శూన్య sqlite3_రికార్డు_Error_Byte_స్థానభ్రంశం(sqlite3*,స్థిరరాశి అక్షరం*);
శూన్య sqlite3_రికార్డు_Error_స్థానభ్రంశం_యొక్క_సమాసం(sqlite3*,స్థిరరాశి సమాసం*);

శూన్య sqlite3BackupRestart(sqlite3_backup *);
శూన్య sqlite3BackupUpdate(sqlite3_backup *, Pgno, స్థిరరాశి u8 *);

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
పూర్ణాంకం sqlite3_సమాసం_Check_లోపల(Parse*, సమాసం*);
#లేకపోతే
# define sqlite3_సమాసం_Check_లోపల(x,y) SQLITE_OK
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ENABLE_STAT4
పూర్ణాంకం sqlite3_Stat4_Probe_సమితి_విలువ(
    Parse*,సూచీ*,Unpacked_రికార్డు**,సమాసం*,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం*);
పూర్ణాంకం sqlite3_Stat4_విలువ_From_సమాసం(Parse*, సమాసం*, u8, sqlite3_విలువ**);
శూన్య sqlite3Stat4ProbeFree(Unpacked_రికార్డు*);
పూర్ణాంకం sqlite3_Stat4_స్తంభం(sqlite3*, స్థిరరాశి శూన్య*, పూర్ణాంకం, పూర్ణాంకం, sqlite3_విలువ**);
అక్షరం sqlite3_సూచీ_స్తంభం_Affinity(sqlite3*, సూచీ*, పూర్ణాంకం);
#అయితే_ముగింపు

/*
** The interface to the LEMON-generated parser
*/
#నిర్వచితం_కాకపోతే SQLITE_AMALGAMATION
  శూన్య *sqlite3ParserAlloc(శూన్య*(*)(u64), Parse*);
  శూన్య sqlite3ParserFree(శూన్య*, శూన్య(*)(శూన్య*));
#అయితే_ముగింపు
శూన్య sqlite3Parser(శూన్య*, పూర్ణాంకం, Token);
పూర్ణాంకం sqlite3ParserFallback(పూర్ణాంకం);
#నిర్వచితమైతే YYTRACKMAXSTACKDEPTH
  పూర్ణాంకం sqlite3_Parser_దొంతర_Peak(శూన్య*);
#అయితే_ముగింపు

శూన్య sqlite3_Auto_ఎక్కించు_Extensions(sqlite3*);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
  శూన్య sqlite3CloseExtensions(sqlite3*);
#లేకపోతే
# define sqlite3CloseExtensions(X)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
  శూన్య sqlite3_పట్టిక_తాళం(Parse *, పూర్ణాంకం, Pgno, u8, స్థిరరాశి అక్షరం *);
#లేకపోతే
  #నిర్వచించు sqlite3_పట్టిక_తాళం(v,w,x,y,z)
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_TEST
  పూర్ణాంకం sqlite3Utf8To8(గుర్తులేని అక్షరం*);
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_OMIT_VIRTUALTABLE
#  define sqlite3VtabClear(D,T)
#  define sqlite3VtabSync(X,Y) SQLITE_OK
#  define sqlite3VtabRollback(X)
#  define sqlite3VtabCommit(X)
#  define sqlite3_Vtab_లోపల_Sync(db) 0
#  define sqlite3_Vtab_తాళం(X)
#  define sqlite3VtabUnlock(X)
#  define sqlite3VtabModuleUnref(D,X)
#  define sqlite3VtabUnlockList(X)
#  define sqlite3VtabSavepoint(X, Y, Z) SQLITE_OK
#  define sqlite3GetVTable(X,Y)  ((VTable*)0)
#లేకపోతే
   శూన్య sqlite3VtabClear(sqlite3 *db, పట్టిక*);
   శూన్య sqlite3VtabDisconnect(sqlite3 *db, పట్టిక *p);
   పూర్ణాంకం sqlite3VtabSync(sqlite3 *db, Vdbe*);
   పూర్ణాంకం sqlite3VtabRollback(sqlite3 *db);
   పూర్ణాంకం sqlite3VtabCommit(sqlite3 *db);
   శూన్య sqlite3_Vtab_తాళం(VTable *);
   శూన్య sqlite3VtabUnlock(VTable *);
   శూన్య sqlite3VtabModuleUnref(sqlite3*,Module*);
   శూన్య sqlite3VtabUnlockList(sqlite3*);
   పూర్ణాంకం sqlite3VtabSavepoint(sqlite3 *, పూర్ణాంకం, పూర్ణాంకం);
   శూన్య sqlite3VtabImportErrmsg(Vdbe*, sqlite3_vtab*);
   VTable *sqlite3GetVTable(sqlite3*, పట్టిక*);
   Module *sqlite3VtabCreateModule(
     sqlite3*,
     స్థిరరాశి అక్షరం*,
     స్థిరరాశి sqlite3_module*,
     శూన్య*,
     శూన్య(*)(శూన్య*)
   );
#  define sqlite3_Vtab_లోపల_Sync(db) ((db)->nVTrans>0 && (db)->aVTrans==0)
#అయితే_ముగింపు
పూర్ణాంకం sqlite3ReadOnlyShadowTables(sqlite3 *db);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  పూర్ణాంకం sqlite3_Shadow_పట్టిక_Name(sqlite3 *db, స్థిరరాశి అక్షరం *zName);
  పూర్ణాంకం sqlite3_Is_Shadow_పట్టిక_యొక్క(sqlite3*,పట్టిక*,స్థిరరాశి అక్షరం*);
  శూన్య sqlite3_Mark_All_Shadow_Tables_యొక్క(sqlite3*, పట్టిక*);
  శూన్య sqlite3_Vtab_Eponymous_పట్టిక_Clear_All(sqlite3*);
#లేకపోతే
# define sqlite3_Shadow_పట్టిక_Name(A,B) 0
# define sqlite3_Is_Shadow_పట్టిక_యొక్క(A,B,C) 0
# define sqlite3_Mark_All_Shadow_Tables_యొక్క(A,B)
# define sqlite3_Vtab_Eponymous_పట్టిక_Clear_All(X)
#అయితే_ముగింపు
పూర్ణాంకం sqlite3_Vtab_Eponymous_పట్టిక_Init(Parse*,Module*);
శూన్య sqlite3_Vtab_Eponymous_పట్టిక_Clear(sqlite3*,Module*);
శూన్య sqlite3VtabMakeWritable(Parse*,పట్టిక*);
శూన్య sqlite3VtabBeginParse(Parse*, Token*, Token*, Token*, పూర్ణాంకం);
శూన్య sqlite3_Vtab_ముగింపు_Parse(Parse*, Token*);
శూన్య sqlite3VtabArgInit(Parse*);
శూన్య sqlite3VtabArgExtend(Parse*, Token*);
పూర్ణాంకం sqlite3VtabCallCreate(sqlite3*, పూర్ణాంకం, స్థిరరాశి అక్షరం *, అక్షరం **);
పూర్ణాంకం sqlite3VtabCallConnect(Parse*, పట్టిక*);
పూర్ణాంకం sqlite3VtabCallDestroy(sqlite3*, పూర్ణాంకం, స్థిరరాశి అక్షరం *);
పూర్ణాంకం sqlite3VtabBegin(sqlite3 *, VTable *);

FuncDef *sqlite3VtabOverloadFunction(sqlite3 *,FuncDef*, పూర్ణాంకం nArg, సమాసం*);
శూన్య sqlite3_Vtab_ఉపయోగిస్తుంది_All_Schemas(Parse*);
sqlite3_int64 sqlite3_Stmt_Current_సమయం(sqlite3_context*);
పూర్ణాంకం sqlite3_Vdbe_Parameter_సూచీ(Vdbe*, స్థిరరాశి అక్షరం*, పూర్ణాంకం);
పూర్ణాంకం sqlite3TransferBindings(sqlite3_stmt *, sqlite3_stmt *);
శూన్య sqlite3ParseObjectInit(Parse*,sqlite3*);
శూన్య sqlite3ParseObjectReset(Parse*);
శూన్య *sqlite3_Parser_కూడు_Cleanup(Parse*,శూన్య(*)(sqlite3*,శూన్య*),శూన్య*);
#నిర్వచితమైతే SQLITE_ENABLE_NORMALIZE
అక్షరం *sqlite3Normalize(Vdbe*, స్థిరరాశి అక్షరం*);
#అయితే_ముగింపు
పూర్ణాంకం sqlite3Reprepare(Vdbe*);
శూన్య sqlite3_సమాసం_List_Check_నిడివి(Parse*, సమాసం_List*, స్థిరరాశి అక్షరం*);
CollSeq *sqlite3_సమాసం_Compare_Coll_Seq(Parse*,స్థిరరాశి సమాసం*);
CollSeq *sqlite3_ద్విమాన_Compare_Coll_Seq(Parse *, స్థిరరాశి సమాసం*, స్థిరరాశి సమాసం*);
పూర్ణాంకం sqlite3_తాత్కాలిక_లోపల_జ్ఞాపకం(స్థిరరాశి sqlite3*);
స్థిరరాశి అక్షరం *sqlite3JournalModename(పూర్ణాంకం);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  పూర్ణాంకం sqlite3Checkpoint(sqlite3*, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం*, పూర్ణాంకం*);
  పూర్ణాంకం sqlite3WalDefaultHook(శూన్య*,sqlite3*,స్థిరరాశి అక్షరం*,పూర్ణాంకం);
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_CTE
  Cte *sqlite3CteNew(Parse*,Token*,సమాసం_List*,Select*,u8);
  శూన్య sqlite3CteDelete(sqlite3*,Cte*);
  With *sqlite3_With_కూడు(Parse*,With*,Cte*);
  శూన్య sqlite3WithDelete(sqlite3*,With*);
  శూన్య sqlite3WithDeleteGeneric(sqlite3*,శూన్య*);
  With *sqlite3_With_నెట్టు(Parse*, With*, u8);
#లేకపోతే
# define sqlite3CteNew(P,T,E,S)   ((శూన్య*)0)
# define sqlite3CteDelete(D,C)
# define sqlite3_Cte_With_కూడు(P,W,C) ((శూన్య*)0)
# define sqlite3WithDelete(x,y)
# define sqlite3_With_నెట్టు(x,y,z) ((శూన్య*)0)
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UPSERT
  Upsert *sqlite3UpsertNew(sqlite3*,సమాసం_List*,సమాసం*,సమాసం_List*,సమాసం*,Upsert*);
  శూన్య sqlite3UpsertDelete(sqlite3*,Upsert*);
  Upsert *sqlite3UpsertDup(sqlite3*,Upsert*);
  పూర్ణాంకం sqlite3_Upsert_Analyze_లక్ష్యం(Parse*,SrcList*,Upsert*,Upsert*);
  శూన్య sqlite3UpsertDoUpdate(Parse*,Upsert*,పట్టిక*,సూచీ*,పూర్ణాంకం);
  Upsert *sqlite3_Upsert_యొక్క_సూచీ(Upsert*,సూచీ*);
  పూర్ణాంకం sqlite3_Upsert_తదుపరి_Is_IPK(Upsert*);
#లేకపోతే
#నిర్వచించు sqlite3UpsertNew(u,v,w,x,y,z) ((Upsert*)0)
#నిర్వచించు sqlite3UpsertDelete(x,y)
#నిర్వచించు sqlite3UpsertDup(x,y)         ((Upsert*)0)
#నిర్వచించు sqlite3_Upsert_యొక్క_సూచీ(x,y)     ((Upsert*)0)
#నిర్వచించు sqlite3_Upsert_తదుపరి_Is_IPK(x)     0
#అయితే_ముగింపు


/* Declarations for functions in fkey.c. All of these are replaced by
** no-op macros if OMIT_FOREIGN_KEY is defined. In this case no foreign
** key functionality is available. If OMIT_TRIGGER is defined but
** OMIT_FOREIGN_KEY is not, only some of the functions are no-oped. In
** this case foreign keys are parsed, but no other functionality is
** provided (enforcement of FK constraints requires the triggers sub-system).
*/
#ఒకవేళ !defined(SQLITE_OMIT_FOREIGN_కీలకం) && !defined(SQLITE_OMIT_TRIGGER)
  శూన్య sqlite3FkCheck(Parse*, పట్టిక*, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం*, పూర్ణాంకం);
  శూన్య sqlite3_Fk_Drop_పట్టిక(Parse*, SrcList *, పట్టిక*);
  శూన్య sqlite3FkActions(Parse*, పట్టిక*, సమాసం_List*, పూర్ణాంకం, పూర్ణాంకం*, పూర్ణాంకం);
  పూర్ణాంకం sqlite3FkRequired(Parse*, పట్టిక*, పూర్ణాంకం*, పూర్ణాంకం);
  u32 sqlite3FkOldmask(Parse*, పట్టిక*);
  FKey *sqlite3FkReferences(పట్టిక *);
  శూన్య sqlite3FkClearTriggerCache(sqlite3*,పూర్ణాంకం);
#లేకపోతే
  #నిర్వచించు sqlite3FkActions(a,b,c,d,e,f)
  #నిర్వచించు sqlite3FkCheck(a,b,c,d,e,f)
  #నిర్వచించు sqlite3_Fk_Drop_పట్టిక(a,b,c)
  #నిర్వచించు sqlite3FkOldmask(a,b)         0
  #నిర్వచించు sqlite3FkRequired(a,b,c,d)    0
  #నిర్వచించు sqlite3FkReferences(a)        0
  #నిర్వచించు sqlite3FkClearTriggerCache(a,b)
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FOREIGN_కీలకం
  శూన్య sqlite3FkDelete(sqlite3 *, పట్టిక*);
  పూర్ణాంకం sqlite3_Fk_Locate_సూచీ(Parse*,పట్టిక*,FKey*,సూచీ**,పూర్ణాంకం**);
#లేకపోతే
  #నిర్వచించు sqlite3FkDelete(a,b)
  #నిర్వచించు sqlite3_Fk_Locate_సూచీ(a,b,c,d,e)
#అయితే_ముగింపు


/*
** Available fault injectors.  Should be numbered beginning with 0.
*/
#నిర్వచించు SQLITE_FAULTINJECTOR_MALLOC     0
#నిర్వచించు SQLITE_FAULTINJECTOR_లెక్క      1

/*
** The interface to the code in fault.c used for identifying "benign"
** malloc failures. This is only present if SQLITE_UNTESTABLE
** is not defined.
*/
#నిర్వచితం_కాకపోతే SQLITE_UNTESTABLE
  శూన్య sqlite3BeginBenignMalloc(శూన్య);
  శూన్య sqlite3_చివర_Benign_Malloc(శూన్య);
#లేకపోతే
  #నిర్వచించు sqlite3BeginBenignMalloc()
  #నిర్వచించు sqlite3_చివర_Benign_Malloc()
#అయితే_ముగింపు

/*
** Allowed return values from sqlite3FindInIndex()
*/
#నిర్వచించు లోపల_సూచీ_ROWID        1   /* Search the rowid of the table */
#నిర్వచించు లోపల_సూచీ_EPH          2   /* Search an ephemeral b-tree */
#నిర్వచించు లోపల_సూచీ_సూచీ_ASC    3   /* Existing index ASCENDING */
#నిర్వచించు లోపల_సూచీ_సూచీ_DESC   4   /* Existing index DESCENDING */
#నిర్వచించు లోపల_సూచీ_NOOP         5   /* No table available. Use comparisons */
/*
** Allowed flags for the 3rd parameter to sqlite3FindInIndex().
*/
#నిర్వచించు లోపల_సూచీ_NOOP_OK     0x0001  /* OK to return IN_INDEX_NOOP */
#నిర్వచించు లోపల_సూచీ_MEMBERSHIP  0x0002  /* IN operator used for membership test */
#నిర్వచించు లోపల_సూచీ_LOOP        0x0004  /* IN operator used as a loop */
పూర్ణాంకం sqlite3_కనుగొను_లోపల_సూచీ(Parse *, సమాసం *, u32, పూర్ణాంకం*, పూర్ణాంకం*, పూర్ణాంకం*);

పూర్ణాంకం sqlite3JournalOpen(sqlite3_vfs *, స్థిరరాశి అక్షరం *, sqlite3_file *, పూర్ణాంకం, పూర్ణాంకం);
పూర్ణాంకం sqlite3_Journal_కొలత(sqlite3_vfs *);
#ఒకవేళ defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
  పూర్ణాంకం sqlite3JournalCreate(sqlite3_file *);
#అయితే_ముగింపు

పూర్ణాంకం sqlite3_Journal_Is_లోపల_జ్ఞాపకం(sqlite3_file *p);
శూన్య sqlite3MemJournalOpen(sqlite3_file *);

శూన్య sqlite3_సమాసం_సమితి_ఎత్తు_And_Flags(Parse *pParse, సమాసం *p);
#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
  పూర్ణాంకం sqlite3_Select_సమాసం_ఎత్తు(స్థిరరాశి Select *);
  పూర్ణాంకం sqlite3_సమాసం_Check_ఎత్తు(Parse*, పూర్ణాంకం);
#లేకపోతే
  #నిర్వచించు sqlite3_Select_సమాసం_ఎత్తు(x) 0
  #నిర్వచించు sqlite3_సమాసం_Check_ఎత్తు(x,y)
#అయితే_ముగింపు
శూన్య sqlite3_సమాసం_సమితి_Error_స్థానభ్రంశం(సమాసం*,పూర్ణాంకం);

u32 sqlite3Get4byte(స్థిరరాశి u8*);
SQLITE_OPT_INLINE u64 sqlite3Get8byte(స్థిరరాశి u8*);
#ఒకవేళ SQLITE_BYTEORDER!=4321
SQLITE_OPT_INLINE u64 sqlite3BSwap64(u64);
#అయితే_ముగింపు
శూన్య sqlite3Put4byte(u8*, u32);

#నిర్వచితమైతే SQLITE_ENABLE_UNLOCK_NOTIFY
  శూన్య sqlite3ConnectionBlocked(sqlite3 *, sqlite3 *);
  శూన్య sqlite3ConnectionUnlocked(sqlite3 *db);
  శూన్య sqlite3ConnectionClosed(sqlite3 *db);
#లేకపోతే
  #నిర్వచించు sqlite3ConnectionBlocked(x,y)
  #నిర్వచించు sqlite3ConnectionUnlocked(x)
  #నిర్వచించు sqlite3ConnectionClosed(x)
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_DEBUG
  శూన్య sqlite3ParserTrace(FILE*, అక్షరం *);
#అయితే_ముగింపు
#ఒకవేళ defined(YYCOVERAGE)
  పూర్ణాంకం sqlite3ParserCoverage(FILE*);
#అయితే_ముగింపు

/*
** If the SQLITE_ENABLE IOTRACE exists then the global variable
** sqlite3IoTrace is a pointer to a printf-like routine used to
** print I/O tracing messages.
*/
#నిర్వచితమైతే SQLITE_ENABLE_IOTRACE
# define IOTRACE(A)  ఒకవేళ( sqlite3IoTrace ){ sqlite3IoTrace A; }
  శూన్య sqlite3VdbeIOTraceSql(Vdbe*);
SQLITE_API SQLITE_EXTERN శూన్య (SQLITE_CDECL *sqlite3IoTrace)(స్థిరరాశి అక్షరం*,...);
#లేకపోతే
# define IOTRACE(A)
# define sqlite3VdbeIOTraceSql(X)
#అయితే_ముగింపు

/*
** These routines are available for the mem2.c debugging memory allocator
** only.  They are used to verify that different "types" of memory
** allocations are properly tracked by the system.
**
** sqlite3MemdebugSetType() sets the "type" of an allocation to one of
** the MEMTYPE_* macros defined below.  The type must be a bitmask with
** a single bit set.
**
** sqlite3MemdebugHasType() returns true if any of the bits in its second
** argument match the type set by the previous sqlite3MemdebugSetType().
** sqlite3MemdebugHasType() is intended for use inside assert() statements.
**
** sqlite3MemdebugNoType() returns true if none of the bits in its second
** argument match the type set by the previous sqlite3MemdebugSetType().
**
** Perhaps the most important point is the difference between MEMTYPE_HEAP
** and MEMTYPE_LOOKASIDE.  If an allocation is MEMTYPE_LOOKASIDE, that means
** it might have been allocated by lookaside, except the allocation was
** too large or lookaside was already full.  It is important to verify
** that allocations that might have been satisfied by lookaside are not
** passed back to non-lookaside free() routines.  Asserts such as the
** example above are placed on the non-lookaside free() routines to verify
** this constraint.
**
** All of this is no-op for a production build.  It only comes into
** play when the SQLITE_MEMDEBUG compile-time option is used.
*/
#నిర్వచితమైతే SQLITE_MEMDEBUG
  శూన్య sqlite3_Memdebug_సమితి_Type(శూన్య*,u8);
  పూర్ణాంకం sqlite3MemdebugHasType(స్థిరరాశి శూన్య*,u8);
  పూర్ణాంకం sqlite3MemdebugNoType(స్థిరరాశి శూన్య*,u8);
#లేకపోతే
# define sqlite3_Memdebug_సమితి_Type(X,Y)  /* no-op */
# define sqlite3MemdebugHasType(X,Y)  1
# define sqlite3MemdebugNoType(X,Y)   1
#అయితే_ముగింపు
#నిర్వచించు MEMTYPE_కుప్ప       0x01  /* General heap allocations */
#నిర్వచించు MEMTYPE_LOOKASIDE  0x02  /* Heap that might have been lookaside */
#నిర్వచించు MEMTYPE_PCACHE     0x04  /* Page cache allocations */

/*
** Threading interface
*/
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
పూర్ణాంకం sqlite3ThreadCreate(SQLiteThread**,శూన్య*(*)(శూన్య*),శూన్య*);
పూర్ణాంకం sqlite3ThreadJoin(SQLiteThread*, శూన్య**);
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)
పూర్ణాంకం sqlite3_Dbpage_రిజిస్టరు(sqlite3*);
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)
పూర్ణాంకం sqlite3_Dbstat_రిజిస్టరు(sqlite3*);
#అయితే_ముగింపు

పూర్ణాంకం sqlite3_సమాసం_సదిశ_కొలత(స్థిరరాశి సమాసం *p_సమాసం);
పూర్ణాంకం sqlite3_సమాసం_Is_సదిశ(స్థిరరాశి సమాసం *p_సమాసం);
సమాసం *sqlite3_సదిశ_క్షేత్రం_Subexpr(సమాసం*, పూర్ణాంకం);
సమాసం *sqlite3_సమాసం_For_సదిశ_క్షేత్రం(Parse*,సమాసం*,పూర్ణాంకం,పూర్ణాంకం);
శూన్య sqlite3_సదిశ_Error_సందేశం(Parse*, సమాసం*);

#నిర్వచితం_కాకపోతే SQLITE_OMIT_COMPILEOPTION_DIAGS
స్థిరరాశి అక్షరం **sqlite3CompileOptions(పూర్ణాంకం *pnOpt);
#అయితే_ముగింపు

#ఒకవేళ SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
పూర్ణాంకం sqlite3KvvfsInit(శూన్య);
#అయితే_ముగింపు

#ఒకవేళ defined(VDBE_PROFILE) \
 || defined(SQLITE_PERFORMANCE_TRACE) \
 || defined(SQLITE_ENABLE_STMT_SCANSTATUS)
sqlite3_uint64 sqlite3Hwtime(శూన్య);
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
# define IS_STMT_SCANSTATUS(db) (db->flags & SQLITE_Stmt_పరిశీలన_స్థితి)
#లేకపోతే
# define IS_STMT_SCANSTATUS(db) 0
#అయితే_ముగింపు

#అయితే_ముగింపు /* SQLITEINT_H */
