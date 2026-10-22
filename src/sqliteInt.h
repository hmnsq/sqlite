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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITEINT_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITEINT_H

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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_TCLAPI
#  define SQLITE_TCLAPI
#ਜੇ_ਅੰਤ

/*
** Include the header file used to customize the compiler options for MSVC.
** This should be done first so that it can successfully prevent spurious
** compiler warnings due to subsequent content in this file and other files
** that are included by this file.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "msvc.h"

/*
** Special setup for VxWorks
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "vxworks.h"

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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DISABLE_LFS
# define _LARGE_FILE       1
# ifndef FILE_ਵਿੱਥ_ਬਿੱਟ
#   define FILE_ਵਿੱਥ_ਬਿੱਟ 64
# endif
# define LARGEFILE_ਸਰੋਤ 1
#ਜੇ_ਅੰਤ

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
#ਜੇ defined(__GNUC__) && !defined(SQLITE_DISABLE_INTRINSIC)
# define GCC_ਸੰਸਕਰਣ (__GNUC__*1000000+__GNUC_MINOR__*1000+__GNUC_PATCHLEVEL__)
#ਨਹੀਂਤਾਂ
# define GCC_ਸੰਸਕਰਣ 0
#ਜੇ_ਅੰਤ
#ਜੇ defined(_MSC_VER) && !defined(SQLITE_DISABLE_INTRINSIC)
# define MSVC_ਸੰਸਕਰਣ _MSC_VER
#ਨਹੀਂਤਾਂ
# define MSVC_ਸੰਸਕਰਣ 0
#ਜੇ_ਅੰਤ

/*
** Some C99 functions in "math.h" are only present for MSVC when its version
** is associated with Visual Studio 2013 or higher.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_HAVE_C99_MATH_FUNCS
# ਜੇ MSVC_ਸੰਸਕਰਣ==0 || MSVC_ਸੰਸਕਰਣ>=1800
#  define SQLITE_HAVE_C99_MATH_FUNCS (1)
# ਨਹੀਂਤਾਂ
#  define SQLITE_HAVE_C99_MATH_FUNCS (0)
# endif
#ਜੇ_ਅੰਤ

/* Needed for various definitions... */
#ਜੇ defined(__GNUC__) && !defined(GNU_ਸਰੋਤ)
# define GNU_ਸਰੋਤ
#ਜੇ_ਅੰਤ

#ਜੇ defined(__OpenBSD__) && !defined(BSD_ਸਰੋਤ)
# define BSD_ਸਰੋਤ
#ਜੇ_ਅੰਤ

/*
** Macro to disable warnings about missing "break" at the end of a "case".
*/
#ਜੇ defined(__has_attribute)
#  ਜੇ __has_attribute(fallthrough)
#    define deliberate_fall_through __attribute__((fallthrough));
#  endif
#ਜੇ_ਅੰਤ
#ਜੇ !defined(deliberate_fall_through)
#  define deliberate_fall_through
#ਜੇ_ਅੰਤ

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
#ਜੇ defined(_HAVE_MINGW_H)
# include "mingw.h"
#ਜਾਂਜੇ defined(_HAVE__MINGW_H)
# include "_mingw.h"
#ਜੇ_ਅੰਤ

/*
** For MinGW version 4.x (and higher), check to see if the _USE_32BIT_TIME_T
** define is required to maintain binary compatibility with the MSVC runtime
** library in use (e.g. for Windows XP).
*/
#ਜੇ !defined(USE_32_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸਮਾਂ_T) && !defined(USE_64_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸਮਾਂ_T) && \
    defined(_WIN32) && !defined(_WIN64) && \
    defined(__MINGW_MAJOR_VERSION) && __MINGW_MAJOR_VERSION >= 4 && \
    defined(__MSVCRT__)
# define USE_32_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸਮਾਂ_T
#ਜੇ_ਅੰਤ

/* Optionally #include a user-defined header, whereby compilation options
** may be set prior to where they take effect, but after platform setup.
** If SQLITE_CUSTOM_INCLUDE=? is defined, its value names the #include
** file.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_CUSTOM_INCLUDE
# define INC_STRINGIFY_(f) #f
# define INC_STRINGIFY(f) INC_STRINGIFY_(f)
# include INC_STRINGIFY(SQLITE_CUSTOM_INCLUDE)
#ਜੇ_ਅੰਤ

/* The public SQLite interface.  The _FILE_OFFSET_BITS macro must appear
** first in QNX.  Also, the _USE_32BIT_TIME_T macro must appear first for
** MinGW.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"

/*
** Reuse the STATIC_VFS1 for mutex access to sqlite3_temp_directory.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MUTEX_STATIC_TEMPDIR SQLITE_MUTEX_STATIC_VFS1

/*
** Include the configuration header output by 'configure' if we're using the
** autoconf-based build
*/
#ਜੇ defined(_HAVE_SQLITE_CONFIG_H) && !defined(SQLITECONFIG_H)
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite_cfg.h"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITECONFIG_H 1
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteLimit.h"

/* Disable nuisance warnings on Borland compilers */
#ਜੇ defined(__BORLANDC__)
#ਹਦਾਇਤ warn -rch /* unreachable code */
#ਹਦਾਇਤ warn -ccc /* Condition is always true or false */
#ਹਦਾਇਤ warn -aus /* Assigned value is never used */
#ਹਦਾਇਤ warn -csu /* Comparing signed and unsigned */
#ਹਦਾਇਤ warn -spa /* Suspicious pointer arithmetic */
#ਜੇ_ਅੰਤ

/*
** A few places in the code require atomic load/store of aligned
** integer values.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __has_extension
# define __has_extension(x) 0     /* compatibility with non-clang compilers */
#ਜੇ_ਅੰਤ
#ਜੇ GCC_ਸੰਸਕਰਣ>=4007000 || __has_extension(c_atomic)
# define SQLITE_ATOMIC_INTRINSICS 1
# define Atomic_ਲੱਦੋ(PTR)       __atomic_load_n((PTR),__ATOMIC_RELAXED)
# define AtomicStore(PTR,VAL)  __atomic_store_n((PTR),(VAL),__ATOMIC_RELAXED)
#ਨਹੀਂਤਾਂ
# define SQLITE_ATOMIC_INTRINSICS 0
# define Atomic_ਲੱਦੋ(PTR)       (*(PTR))
# define AtomicStore(PTR,VAL)  (*(PTR) = (VAL))
#ਜੇ_ਅੰਤ

/*
** Include standard header files as necessary
*/
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_INTTYPES_H
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਜੇ_ਅੰਤ

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
#ਜੇ defined(HAVE_STDINT_H)   /* Use this case if we have ANSI headers */
# define SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(X)  ((ਖਾਲੀ*)(intptr_t)(X))
# define SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(X)  ((ਪੂਰਨ)(intptr_t)(X))
#ਜਾਂਜੇ defined(__PTRDIFF_TYPE__)  /* This case should work for GCC */
# define SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(X)  ((ਖਾਲੀ*)(__PTRDIFF_TYPE__)(X))
# define SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(X)  ((ਪੂਰਨ)(__PTRDIFF_TYPE__)(X))
#ਜਾਂਜੇ !defined(__GNUC__)       /* Works for compilers other than LLVM */
# define SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(X)  ((ਖਾਲੀ*)&((ਅੱਖਰ*)0)[X])
# define SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(X)  ((ਪੂਰਨ)(((ਅੱਖਰ*)X)-(ਅੱਖਰ*)0))
#ਨਹੀਂਤਾਂ                          /* Generates a warning - but it always works */
# define SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(X)  ((ਖਾਲੀ*)(X))
# define SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(X)  ((ਪੂਰਨ)(X))
#ਜੇ_ਅੰਤ

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
#ਜੇ defined(SQLITE_DISABLE_INLINE)
#  define SQLITE_NOINLINE
#  define SQLITE_INLINE
#  define SQLITE_OPT_INLINE
#  define SQLITE_ਵਰਤਦਾ_INLINE 0
#ਜਾਂਜੇ defined(__GNUC__)
#  define SQLITE_NOINLINE    __attribute__((noinline))
#  define SQLITE_INLINE      __attribute__((always_inline)) ਸਤਰਅੰਦਰ
#  define SQLITE_OPT_INLINE  __attribute__((always_inline)) ਸਤਰਅੰਦਰ
#  define SQLITE_ਵਰਤਦਾ_INLINE 1
#ਜਾਂਜੇ defined(_MSC_VER) && _MSC_VER>=1310
#  define SQLITE_NOINLINE    __declspec(noinline)
#  define SQLITE_INLINE      __forceinline
#  define SQLITE_OPT_INLINE  __forceinline
#  define SQLITE_ਵਰਤਦਾ_INLINE 1
#ਨਹੀਂਤਾਂ
#  define SQLITE_NOINLINE
#  define SQLITE_INLINE
#  define SQLITE_OPT_INLINE
#  define SQLITE_ਵਰਤਦਾ_INLINE 0
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_COVERAGE_TEST) || defined(__STRICT_ANSI__)
# undef SQLITE_INLINE
# define SQLITE_INLINE
# undef SQLITE_OPT_INLINE
# define SQLITE_OPT_INLINE
# undef SQLITE_ਵਰਤਦਾ_INLINE
# define SQLITE_ਵਰਤਦਾ_INLINE 0
#ਜੇ_ਅੰਤ
#ਜੇ !defined(SQLITE_AMALGAMATION)
# undef SQLITE_OPT_INLINE
# define SQLITE_OPT_INLINE
#ਜੇ_ਅੰਤ

/*
** Make sure that the compiler intrinsics we desire are enabled when
** compiling with an appropriate version of MSVC unless prevented by
** the SQLITE_DISABLE_INTRINSIC define.
*/
#ਜੇ !defined(SQLITE_DISABLE_INTRINSIC)
#  ਜੇ defined(_MSC_VER) && _MSC_VER>=1400
#    ਜੇ !defined(_WIN32_WCE)
#      include <intrin.h>
#      pragma intrinsic(_byteswap_ushort)
#      pragma intrinsic(_byteswap_ulong)
#      pragma intrinsic(_byteswap_uint64)
#      pragma intrinsic(_ReadWriteBarrier)
#    ਨਹੀਂਤਾਂ
#      include <cmnintrin.h>
#    endif
#  endif
#ਜੇ_ਅੰਤ

/*
** Enable SQLITE_USE_SEH by default on MSVC builds.  Only omit
** SEH support if the -DSQLITE_OMIT_SEH option is given.
*/
#ਜੇ defined(_MSC_VER) && !defined(SQLITE_OMIT_SEH)
# define SQLITE_USE_SEH 1
#ਨਹੀਂਤਾਂ
# undef SQLITE_USE_SEH
#ਜੇ_ਅੰਤ

/*
** Enable SQLITE_DIRECT_OVERFLOW_READ, unless the build explicitly
** disables it using -DSQLITE_DIRECT_OVERFLOW_READ=0
*/
#ਜੇ defined(SQLITE_ਸਿੱਧਾ_ਉਛਾਲ_READ) && SQLITE_ਸਿੱਧਾ_ਉਛਾਲ_READ+1==1
  /* Disable if -DSQLITE_DIRECT_OVERFLOW_READ=0 */
# undef SQLITE_ਸਿੱਧਾ_ਉਛਾਲ_READ
#ਨਹੀਂਤਾਂ
  /* In all other cases, enable */
# define SQLITE_ਸਿੱਧਾ_ਉਛਾਲ_READ 1
#ਜੇ_ਅੰਤ


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
#ਜੇ !defined(SQLITE_THREADSAFE)
# ਜੇ defined(THREADSAFE)
#   define SQLITE_THREADSAFE THREADSAFE
# ਨਹੀਂਤਾਂ
#   define SQLITE_THREADSAFE 1 /* IMP: R-07272-22309 */
# endif
#ਜੇ_ਅੰਤ

/*
** Powersafe overwrite is on by default.  But can be turned off using
** the -DSQLITE_POWERSAFE_OVERWRITE=0 command-line option.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_POWERSAFE_OVERWRITE
# define SQLITE_POWERSAFE_OVERWRITE 1
#ਜੇ_ਅੰਤ

/*
** EVIDENCE-OF: R-25715-37072 Memory allocation statistics are enabled by
** default unless SQLite is compiled with SQLITE_DEFAULT_MEMSTATUS=0 in
** which case memory allocation statistics are disabled by default.
*/
#ਜੇ !defined(SQLITE_DEFAULT_MEMSTATUS)
# define SQLITE_DEFAULT_MEMSTATUS 1
#ਜੇ_ਅੰਤ

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
#ਜੇ defined(SQLITE_SYSTEM_MALLOC) \
  + defined(SQLITE_WIN32_MALLOC) \
  + defined(SQLITE_ZERO_MALLOC) \
  + defined(SQLITE_MEMDEBUG)>1
# error "Two or ਹੋਰ ਦਾ the following compile-ਸਮਾਂ configuration options\
 are defined but at most one is allowed:\
 SQLITE_SYSTEM_MALLOC, SQLITE_WIN32_MALLOC, SQLITE_MEMDEBUG,\
 SQLITE_ZERO_MALLOC"
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_SYSTEM_MALLOC) \
  + defined(SQLITE_WIN32_MALLOC) \
  + defined(SQLITE_ZERO_MALLOC) \
  + defined(SQLITE_MEMDEBUG)==0
# define SQLITE_SYSTEM_MALLOC 1
#ਜੇ_ਅੰਤ

/*
** If SQLITE_MALLOC_SOFT_LIMIT is not zero, then try to keep the
** sizes of memory allocations below this value where possible.
*/
#ਜੇ !defined(SQLITE_MALLOC_SOFT_ਹੱਦਬੰਦੀ)
# define SQLITE_MALLOC_SOFT_ਹੱਦਬੰਦੀ 1024
#ਜੇ_ਅੰਤ

/*
** We need to define _XOPEN_SOURCE as follows in order to enable
** recursive mutexes on most Unix systems and fchmod() on OpenBSD.
** But _XOPEN_SOURCE define causes problems for Mac OS X, so omit
** it.
*/
#ਜੇ !defined(XOPEN_ਸਰੋਤ) && !defined(__DARWIN__) && !defined(__APPLE__)
#  define XOPEN_ਸਰੋਤ 600
#ਜੇ_ਅੰਤ

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
#ਜੇ !defined(NDEBUG) && !defined(SQLITE_DEBUG)
# define NDEBUG 1
#ਜੇ_ਅੰਤ
#ਜੇ defined(NDEBUG) && defined(SQLITE_DEBUG)
# undef NDEBUG
#ਜੇ_ਅੰਤ

/*
** Enable SQLITE_ENABLE_EXPLAIN_COMMENTS if SQLITE_DEBUG is turned on.
*/
#ਜੇ !defined(SQLITE_ENABLE_EXPLAIN_COMMENTS) && defined(SQLITE_DEBUG)
# define SQLITE_ENABLE_EXPLAIN_COMMENTS 1
#ਜੇ_ਅੰਤ

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
#ਜੇ defined(SQLITE_MUTATION_TEST)
# define testcase(X)
#ਜਾਂਜੇ defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
# ifndef SQLITE_AMALGAMATION
    ਬਾਹਰੀ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ sqlite3CoverageCounter;
# endif
# define testcase(X)  ਜੇ( X ){ sqlite3CoverageCounter += (ਬਿਨਾਨਿਸ਼ਾਨੀ)__LINE__; }
#ਨਹੀਂਤਾਂ
# define testcase(X)
#ਜੇ_ਅੰਤ

/*
** The TESTONLY macro is used to enclose variable declarations or
** other bits of code that are needed to support the arguments
** within testcase() and assert() macros.
*/
#ਜੇ !defined(NDEBUG) || defined(SQLITE_COVERAGE_TEST)
# define TESTONLY(X)  X
#ਨਹੀਂਤਾਂ
# define TESTONLY(X)
#ਜੇ_ਅੰਤ

/*
** Sometimes we need a small amount of code such as a variable initialization
** to setup for a later assert() statement.  We do not want this code to
** appear when assert() is disabled.  The following macro is therefore
** used to contain that setup code.  The "VVA" acronym stands for
** "Verification, Validation, and Accreditation".  In other words, the
** code within VVA_ONLY() will only run during verification processes.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
# define VVA_ONLY(X)  X
#ਨਹੀਂਤਾਂ
# define VVA_ONLY(X)
#ਜੇ_ਅੰਤ

/*
** Disable ALWAYS() and NEVER() (make them pass-throughs) for coverage
** and mutation testing
*/
#ਜੇ defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_MUTATION_TEST)
# define SQLITE_OMIT_AUXILIARY_SAFETY_CHECKS  1
#ਜੇ_ਅੰਤ

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
#ਜੇ defined(SQLITE_OMIT_AUXILIARY_SAFETY_CHECKS)
# define ALWAYS(X)      (1)
# define NEVER(X)       (0)
#ਜਾਂਜੇ !defined(NDEBUG)
# define ALWAYS(X)      ((X)?1:(ਪ੍ਰਤਿਜ੍ਞਾ(0),0))
# define NEVER(X)       ((X)?(ਪ੍ਰਤਿਜ੍ਞਾ(0),1):0)
#ਨਹੀਂਤਾਂ
# define ALWAYS(X)      (X)
# define NEVER(X)       (X)
#ਜੇ_ਅੰਤ

/*
** Some conditionals are optimizations only.  In other words, if the
** conditionals are replaced with a constant 1 (true) or 0 (false) then
** the correct answer is still obtained, though perhaps not as quickly.
**
** The following macros mark these optimizations conditionals.
*/
#ਜੇ defined(SQLITE_MUTATION_TEST)
# define OK_IF_ALWAYS_TRUE(X)  (1)
# define OK_IF_ALWAYS_FALSE(X) (0)
#ਨਹੀਂਤਾਂ
# define OK_IF_ALWAYS_TRUE(X)  (X)
# define OK_IF_ALWAYS_FALSE(X) (X)
#ਜੇ_ਅੰਤ

/*
** Some malloc failures are only possible if SQLITE_TEST_REALLOC_STRESS is
** defined.  We need to defend against those failures when testing with
** SQLITE_TEST_REALLOC_STRESS, but we don't want the unreachable branches
** during a normal build.  The following macro can be used to disable tests
** that are always false except when SQLITE_TEST_REALLOC_STRESS is set.
*/
#ਜੇ defined(SQLITE_TEST_REALLOC_STRESS)
# define ONLY_IF_REALLOC_STRESS(X)  (X)
#ਜਾਂਜੇ !defined(NDEBUG)
# define ONLY_IF_REALLOC_STRESS(X)  ((X)?(ਪ੍ਰਤਿਜ੍ਞਾ(0),1):0)
#ਨਹੀਂਤਾਂ
# define ONLY_IF_REALLOC_STRESS(X)  (0)
#ਜੇ_ਅੰਤ

/*
** Declarations used for tracing the operating system interfaces.
*/
#ਜੇ defined(SQLITE_FORCE_OS_TRACE) || defined(SQLITE_TEST) || \
    (defined(SQLITE_DEBUG) && SQLITE_OS_WIN)
  ਬਾਹਰੀ ਪੂਰਨ sqlite3OSTrace;
# define OSTRACE(X)          ਜੇ( sqlite3OSTrace ) sqlite3DebugPrintf X
# define SQLITE_HAVE_OS_TRACE
#ਨਹੀਂਤਾਂ
# define OSTRACE(X)
# undef  SQLITE_HAVE_OS_TRACE
#ਜੇ_ਅੰਤ

/*
** Is the sqlite3ErrName() function needed in the build?  Currently,
** it is needed by "mutex_w32.c" (when debugging), "os_win.c" (when
** OSTRACE is enabled), and by several "test*.c" files (which are
** compiled using SQLITE_TEST).
*/
#ਜੇ defined(SQLITE_HAVE_OS_TRACE) || defined(SQLITE_TEST) || \
    (defined(SQLITE_DEBUG) && SQLITE_OS_WIN)
# define SQLITE_NEED_ERR_NAME
#ਨਹੀਂਤਾਂ
# undef  SQLITE_NEED_ERR_NAME
#ਜੇ_ਅੰਤ

/*
** SQLITE_ENABLE_EXPLAIN_COMMENTS is incompatible with SQLITE_OMIT_EXPLAIN
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_EXPLAIN
# undef SQLITE_ENABLE_EXPLAIN_COMMENTS
#ਜੇ_ਅੰਤ

/*
** SQLITE_OMIT_VIRTUALTABLE implies SQLITE_OMIT_ALTERTABLE
*/
#ਜੇ defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_ALTERTABLE)
# define SQLITE_OMIT_ALTERTABLE
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_DIGIT_SEPARATOR '_'

/*
** Return true (non-zero) if the input is an integer that is too large
** to fit in 32-bits.  This macro is used inside of various testcase()
** macros to verify that we have tested SQLite for large-file support.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_BIG_INT(X)  (((X)&~(i64)0xffffffff)!=0)

/*
** The macro unlikely() is a hint that surrounds a boolean
** expression that is usually false.  Macro likely() surrounds
** a boolean expression that is usually true.  These hints could,
** in theory, be used by the compiler to generate better code, but
** currently they are just comments for human readers.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ likely(X)    (X)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unlikely(X)  (X)

#ਅਨ੍ਤਰ੍ਭਾਵ "hash.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "parse.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>

/*
** Use a macro to replace memcpy() if compiled with SQLITE_INLINE_MEMCPY.
** This allows better measurements of where memcpy() is used when running
** cachegrind.  But this macro version of memcpy() is very slow so it
** should not be used in production.  This is a performance measurement
** hack only.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_INLINE_MEMCPY
# define ਸਮ੍ਰਿਤੀ_ਨਕਲ(D,S,N) {ਅੱਖਰ*xxd=(ਅੱਖਰ*)(D);ਅਚਲ ਅੱਖਰ*xxs=(ਅਚਲ ਅੱਖਰ*)(S);\
                        ਪੂਰਨ xxn=(N);ਜਦੋਂ(xxn-->0)*(xxd++)=*(xxs++);}
#ਜੇ_ਅੰਤ

/*
** If compiling for a processor that lacks floating point support,
** substitute integer for floating-point
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
# define ਦੁੱਗਣਾ sqlite_int64
# define ਦਸ਼ਮਲਵ sqlite_int64
# define fabs(X) ((X)<0?-(X):(X))
# define sqlite3_Is_ਉਛਾਲ(X) 0
# define INFINITY (9223372036854775807LL)
# ifndef SQLITE_BIG_DBL
#   define SQLITE_BIG_DBL (((sqlite3_int64)1)<<50)
# endif
# define SQLITE_OMIT_DATETIME_FUNCS 1
# define SQLITE_OMIT_TRACE 1
# undef SQLITE_MIXED_ENDIAN_64_ਦੋਆਧਾਰੀ_ਅੰਕ_FLOAT
# undef SQLITE_HAVE_ISNAN
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_BIG_DBL
# define SQLITE_BIG_DBL (1e99)
#ਜੇ_ਅੰਤ

/*
** OMIT_TEMPDB is set to 1 if SQLITE_OMIT_TEMPDB is defined, or 0
** afterward. Having this macro allows us to cause the C compiler
** to omit code used by TEMP tables without messy #ifndef statements.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TEMPDB
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OMIT_TEMPDB 1
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OMIT_TEMPDB 0
#ਜੇ_ਅੰਤ

/*
** The "file format" number is an integer that is incremented whenever
** the VDBE-level file format changes.  The following macros define the
** the default file format for new databases and the maximum file format
** that the library can read.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MAX_FILE_FORMAT 4
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_FILE_FORMAT
# define SQLITE_DEFAULT_FILE_FORMAT 4
#ਜੇ_ਅੰਤ

/*
** Determine whether triggers are recursive by default.  This can be
** changed at run-time using a pragma.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_RECURSIVE_TRIGGERS
# define SQLITE_DEFAULT_RECURSIVE_TRIGGERS 0
#ਜੇ_ਅੰਤ

/*
** Provide a default value for SQLITE_TEMP_STORE in case it is not specified
** on the command-line
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ਆਰਜ਼ੀ_STORE
# define SQLITE_ਆਰਜ਼ੀ_STORE 1
#ਜੇ_ਅੰਤ

/*
** If no value has been provided for SQLITE_MAX_WORKER_THREADS, or if
** SQLITE_TEMP_STORE is set to 3 (never use temporary files), set it
** to zero.
*/
#ਜੇ SQLITE_ਆਰਜ਼ੀ_STORE==3 || SQLITE_THREADSAFE==0
# undef SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ
# define SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ
# define SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ 8
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_ਕਾਮਾ_ਧਾਗੇ
# define SQLITE_DEFAULT_ਕਾਮਾ_ਧਾਗੇ 0
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_DEFAULT_ਕਾਮਾ_ਧਾਗੇ>SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ
# undef SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ
# define SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ SQLITE_DEFAULT_ਕਾਮਾ_ਧਾਗੇ
#ਜੇ_ਅੰਤ

/*
** The default initial allocation for the pagecache when using separate
** pagecaches for each database connection.  A positive number is the
** number of pages.  A negative number N translations means that a buffer
** of -1024*N bytes is allocated and used for as many pages as it will hold.
**
** The default value of "20" was chosen to minimize the run-time of the
** speedtest1 test program with options: --shrink-memory --reprepare
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_PCACHE_INITSZ
# define SQLITE_DEFAULT_PCACHE_INITSZ 20
#ਜੇ_ਅੰਤ

/*
** Default value for the SQLITE_CONFIG_SORTERREF_SIZE option.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_SORTERREF_ਮਾਪ
# define SQLITE_DEFAULT_SORTERREF_ਮਾਪ 0x7fffffff
#ਜੇ_ਅੰਤ

/*
** The compile-time options SQLITE_MMAP_READWRITE and
** SQLITE_ENABLE_BATCH_ATOMIC_WRITE are not compatible with one another.
** You must choose one or the other (or neither) but not both.
*/
#ਜੇ defined(SQLITE_MMAP_READWRITE) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
#ਗਲਤੀ Cannot use both SQLITE_MMAP_READWRITE and SQLITE_ENABLE_BATCH_ATOMIC_WRITE
#ਜੇ_ਅੰਤ

/*
** GCC does not define the offsetof() macro so we'll have to do it
** ourselves.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ offsetof
# define offsetof(ST,M) ((size_t)((ਅੱਖਰ*)&((ST*)0)->M - (ਅੱਖਰ*)0))
#ਜੇ_ਅੰਤ

/*
** sizeof64() is like sizeof(), but always returns a 64-bit value, even
** on 32-bit builds. This can help to avoid overflow by ensuring 64-bit
** arithmetic is used consistently in both 32-bit and 64-bit builds.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sizeof64(X) ((sqlite3_int64)ਆਕਾਰ(X))

/*
** Work around C99 "flex-array" syntax for pre-C99 compilers, so as
** to avoid complaints from -fsanitize=strict-bounds.
*/
#ਜੇ defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define FLEXARRAY
#ਨਹੀਂਤਾਂ
# define FLEXARRAY 1
#ਜੇ_ਅੰਤ

/*
** Macros to compute minimum and maximum of two numbers.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MIN
# define MIN(A,B) ((A)<(B)?(A):(B))
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MAX
# define MAX(A,B) ((A)>(B)?(A):(B))
#ਜੇ_ਅੰਤ

/*
** Swap two objects of type TYPE.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਦਲਾ_ਬਦਲੀ(TYPE,A,B) {TYPE t=A; A=B; B=t;}

/*
** Check to see if this machine uses EBCDIC.  (Yes, believe it or
** not, there are still machines out there that use EBCDIC.)
*/
#ਜੇ 'A' == '\301'
# define SQLITE_EBCDIC 1
#ਨਹੀਂਤਾਂ
# define SQLITE_ASCII 1
#ਜੇ_ਅੰਤ

/*
** Integers of known sizes.  These typedefs might change for architectures
** where the sizes very.  Preprocessor macros are available so that the
** types can be conveniently redefined at compile-type.  Like this:
**
**         cc '-DUINTPTR_TYPE=long long int' ...
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ UINT32_TYPE
# ifdef HAVE_UINT32_T
#  define UINT32_TYPE uint32_t
# ਨਹੀਂਤਾਂ
#  define UINT32_TYPE ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ
# endif
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ UINT16_TYPE
# ifdef HAVE_UINT16_T
#  define UINT16_TYPE uint16_t
# ਨਹੀਂਤਾਂ
#  define UINT16_TYPE ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਪੂਰਨ
# endif
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ INT16_TYPE
# ifdef HAVE_INT16_T
#  define INT16_TYPE int16_t
# ਨਹੀਂਤਾਂ
#  define INT16_TYPE ਛੋਟਾ ਪੂਰਨ
# endif
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ UINT8_TYPE
# ifdef HAVE_UINT8_T
#  define UINT8_TYPE uint8_t
# ਨਹੀਂਤਾਂ
#  define UINT8_TYPE ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ
# endif
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ INT8_TYPE
# ifdef HAVE_INT8_T
#  define INT8_TYPE int8_t
# ਨਹੀਂਤਾਂ
#  define INT8_TYPE ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ
# endif
#ਜੇ_ਅੰਤ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ sqlite_int64 i64;          /* 8-byte signed integer */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ sqlite_uint64 u64;         /* 8-byte unsigned integer */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ UINT32_TYPE u32;           /* 4-byte unsigned integer */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ UINT16_TYPE u16;           /* 2-byte unsigned integer */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ INT16_TYPE i16;            /* 2-byte signed integer */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ UINT8_TYPE u8;             /* 1-byte unsigned integer */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ INT8_TYPE i8;              /* 1-byte signed integer */

/* A bitfield type for use inside of structures.  Always follow with :N where
** N is the number of bits.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ bft;  /* Bit Field Type */

/*
** SQLITE_MAX_U32 is a u64 constant that is the maximum u64 value
** that can be stored in a u32 without loss of data.  The value
** is 0x00000000ffffffff.  But because of quirks of some compilers, we
** have to specify the value in the less intuitive manner shown:
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MAX_U32  ((((u64)1)<<32)-1)

/*
** The datatype used to store estimates of the number of rows in a
** table or index.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ u64 tRowcnt;

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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ INT16_TYPE LogEst;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOGEST_MIN (-32768)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOGEST_MAX (32767)

/*
** Set the SQLITE_PTRSIZE macro to the number of bytes in a pointer
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_PTRSIZE
# ਜੇ defined(__SIZEOF_POINTER__)
#   define SQLITE_PTRSIZE __SIZEOF_POINTER__
# elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
       defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
      (defined(__APPLE__) && defined(__ppc__)) ||                         \
      (defined(__TOS_AIX__) && !defined(__64BIT__))
#   define SQLITE_PTRSIZE 4
# ਨਹੀਂਤਾਂ
#   define SQLITE_PTRSIZE 8
# endif
#ਜੇ_ਅੰਤ

/* The uptr type is an unsigned integer large enough to hold a pointer
*/
#ਜੇ defined(HAVE_STDINT_H)
  ਕਿਸਮਪਰਿਭਾਸ਼ਾ uintptr_t uptr;
#ਜਾਂਜੇ SQLITE_PTRSIZE==4
  ਕਿਸਮਪਰਿਭਾਸ਼ਾ u32 uptr;
#ਨਹੀਂਤਾਂ
  ਕਿਸਮਪਰਿਭਾਸ਼ਾ u64 uptr;
#ਜੇ_ਅੰਤ

/*
** The SQLITE_WITHIN(P,S,E) macro checks to see if pointer P points to
** something between S (inclusive) and E (exclusive).
**
** In other words, S is a buffer and E is a pointer to the first byte after
** the end of buffer S.  This macro returns true if P points to something
** contained within the buffer S.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_WITHIN(P,S,E)   (((uptr)(P)>=(uptr)(S))&&((uptr)(P)<(uptr)(E)))

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਉਛਾਲ(P,S,E) (((uptr)(S)<(uptr)(P))&&((uptr)(E)>(uptr)(P)))

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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_BYTEORDER  /* Replicate changes at tag-20230904a */
# ਜੇ defined(__BYTE_ORDER__) && __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
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
# ਨਹੀਂਤਾਂ
#   define SQLITE_BYTEORDER 0
# endif
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_BYTEORDER==4321
# define SQLITE_BIGENDIAN    1
# define SQLITE_LITTLEENDIAN 0
# define SQLITE_UTF16NATIVE  SQLITE_UTF16BE
#ਜਾਂਜੇ SQLITE_BYTEORDER==1234
# define SQLITE_BIGENDIAN    0
# define SQLITE_LITTLEENDIAN 1
# define SQLITE_UTF16NATIVE  SQLITE_UTF16LE
#ਨਹੀਂਤਾਂ
# ifdef SQLITE_AMALGAMATION
  ਅਚਲ ਪੂਰਨ sqlite3one = 1;
# ਨਹੀਂਤਾਂ
  ਬਾਹਰੀ ਅਚਲ ਪੂਰਨ sqlite3one;
# endif
# define SQLITE_BIGENDIAN    (*(ਅੱਖਰ *)(&sqlite3one)==0)
# define SQLITE_LITTLEENDIAN (*(ਅੱਖਰ *)(&sqlite3one)==1)
# define SQLITE_UTF16NATIVE  (SQLITE_BIGENDIAN?SQLITE_UTF16BE:SQLITE_UTF16LE)
#ਜੇ_ਅੰਤ

/*
** Constants for the largest and smallest possible 64-bit signed integers.
** These macros are designed to work correctly on both 32-bit and 64-bit
** compilers.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64  (0xffffffff|(((i64)0x7fffffff)<<32))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਭ_ਤੋਂ_ਵੱਡਾ_UINT64 (0xffffffff|(((u64)0xffffffff)<<32))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਭ_ਤੋਂ_ਛੋਟਾ_INT64 (((i64)-1) - ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64)

/*
** Macro SMXV(n) return the maximum value that can be held in variable n,
** assuming n is a signed integer type.  UMXV(n) is similar for unsigned
** integer types.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMXV(n) ((((i64)1)<<(ਆਕਾਰ(n)*8-1))-1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UMXV(n) ((((i64)1)<<(ਆਕਾਰ(n)*8))-1)

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ROUND8(x)     (((x)+7)&~7)
#ਜੇ SQLITE_PTRSIZE==8
# define ROUND8P(x)   (x)
#ਨਹੀਂਤਾਂ
# define ROUND8P(x)   (((x)+7)&~7)
#ਜੇ_ਅੰਤ

/*
** Round down to the nearest multiple of 8
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ROUNDDOWN8(x) ((x)&~7)

/*
** Assert that the pointer X is aligned to an 8-byte boundary.  This
** macro is used only within assert() to verify that the code gets
** all alignment restrictions correct.
**
** Except, if SQLITE_4_BYTE_ALIGNED_MALLOC is defined, then the
** underlying malloc() implementation might return us 4-byte aligned
** pointers.  In that case, only verify 4-byte alignment.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_4_BYTE_ALIGNED_MALLOC
# define EIGHT_BYTE_ALIGNMENT(X)   ((((uptr)(X) - (uptr)0)&3)==0)
#ਨਹੀਂਤਾਂ
# define EIGHT_BYTE_ALIGNMENT(X)   ((((uptr)(X) - (uptr)0)&7)==0)
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TWO_BYTE_ALIGNMENT(X)      ((((uptr)(X) - (uptr)0)&1)==0)

/*
** Disable MMAP on platforms where it is known to not work
*/
#ਜੇ defined(__OpenBSD__) || defined(__QNXNTO__)
# undef SQLITE_MAX_MMAP_ਮਾਪ
# define SQLITE_MAX_MMAP_ਮਾਪ 0
#ਜੇ_ਅੰਤ

/*
** Default maximum size of memory used by memory-mapped I/O in the VFS
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __APPLE__
# include <TargetConditionals.h>
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MAX_MMAP_ਮਾਪ
# ਜੇ defined(__linux__) \
  || defined(_WIN32) \
  || (defined(__APPLE__) && defined(__MACH__)) \
  || defined(__sun) \
  || defined(__FreeBSD__) \
  || defined(__DragonFly__)
#   define SQLITE_MAX_MMAP_ਮਾਪ 0x7fff0000  /* 2147418112 */
# ਨਹੀਂਤਾਂ
#   define SQLITE_MAX_MMAP_ਮਾਪ 0
# endif
#ਜੇ_ਅੰਤ

/*
** The default MMAP_SIZE is zero on all platforms.  Or, even if a larger
** default MMAP_SIZE is specified at compile-time, make sure that it does
** not exceed the maximum mmap size.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_MMAP_ਮਾਪ
# define SQLITE_DEFAULT_MMAP_ਮਾਪ 0
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_DEFAULT_MMAP_ਮਾਪ>SQLITE_MAX_MMAP_ਮਾਪ
# undef SQLITE_DEFAULT_MMAP_ਮਾਪ
# define SQLITE_DEFAULT_MMAP_ਮਾਪ SQLITE_MAX_MMAP_ਮਾਪ
#ਜੇ_ਅੰਤ

/*
** TREETRACE_ENABLED will be either 1 or 0 depending on whether or not
** the Abstract Syntax Tree tracing logic is turned on.
*/
#ਜੇ !defined(SQLITE_AMALGAMATION)
ਬਾਹਰੀ u32 sqlite3_ਰੁੱਖ_Trace;
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_DEBUG) \
    && (defined(SQLITE_TEST) || defined(SQLITE_ENABLE_SELECTTRACE) \
                             || defined(SQLITE_ENABLE_TREETRACE))
# define TREETRACE_ENABLED 1
# define TREETRACE(K,P,S,X)  \
  ਜੇ(sqlite3_ਰੁੱਖ_Trace&(K))   \
    sqlite3DebugPrintf("%u/%d/%p: ",(S)->selId,(P)->addrExplain,(S)),\
    sqlite3DebugPrintf X
#ਨਹੀਂਤਾਂ
# define TREETRACE(K,P,S,X)
# define TREETRACE_ENABLED 0
#ਜੇ_ਅੰਤ

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
ਬਾਹਰੀ u32 sqlite3WhereTrace;
#ਜੇ defined(SQLITE_DEBUG) \
    && (defined(SQLITE_TEST) || defined(SQLITE_ENABLE_WHERETRACE))
# define WHERETRACE(K,X)  ਜੇ(sqlite3WhereTrace&(K)) sqlite3DebugPrintf X
# define WHERETRACE_ENABLED 1
#ਨਹੀਂਤਾਂ
# define WHERETRACE(K,X)
#ਜੇ_ਅੰਤ

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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ BusyHandler BusyHandler;
ਬਣਤਰ BusyHandler {
  ਪੂਰਨ (*xBusyHandler)(ਖਾਲੀ *,ਪੂਰਨ);  /* The busy callback */
  ਖਾਲੀ *pBusyArg;                   /* First arg to busy callback */
  ਪੂਰਨ nBusy;                        /* Incremented with each busy call */
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LEGACY_SCHEMA_ਸਾਰਣੀ          "sqlite_master"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LEGACY_ਆਰਜ਼ੀ_SCHEMA_ਸਾਰਣੀ     "sqlite_temp_master"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PREFERRED_SCHEMA_ਸਾਰਣੀ       "sqlite_schema"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PREFERRED_ਆਰਜ਼ੀ_SCHEMA_ਸਾਰਣੀ  "sqlite_temp_schema"


/*
** The root-page of the schema table.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCHEMA_ਜੜ੍ਹ    1

/*
** The name of the schema table.  The name is different for TEMP.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCHEMA_ਸਾਰਣੀ(x) \
    ((!OMIT_TEMPDB)&&(x==1)?LEGACY_ਆਰਜ਼ੀ_SCHEMA_ਸਾਰਣੀ:LEGACY_SCHEMA_ਸਾਰਣੀ)

/*
** A convenience macro that returns the number of elements in
** an array.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Array_ਮਾਪ(X)    ((ਪੂਰਨ)(ਆਕਾਰ(X)/ਆਕਾਰ(X[0])))

/*
** Determine if the argument is a power of two
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Is_Power_ਦਾ_Two(X) (((X)&((X)-1))==0)

/*
** The following value as a destructor means to use sqlite3DbFree().
** The sqlite3DbFree() routine requires two parameters instead of the
** one parameter that destructors normally want.  So we have to introduce
** this magic value that the code knows to handle differently.  Any
** pointer will work here as long as it is distinct from SQLITE_STATIC
** and SQLITE_TRANSIENT.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_DYNAMIC   ((sqlite3_destructor_type)sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_Clear)

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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WSD
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_WSD ਅਚਲ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GLOBAL(t,v) (*(t*)sqlite3_wsd_ਲੱਭੋ((ਖਾਲੀ*)&(v), ਆਕਾਰ(v)))
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3GlobalConfig GLOBAL(ਬਣਤਰ Sqlite3Config, sqlite3Config)
  ਪੂਰਨ sqlite3_wsd_init(ਪੂਰਨ N, ਪੂਰਨ J);
  ਖਾਲੀ *sqlite3_wsd_ਲੱਭੋ(ਖਾਲੀ *K, ਪੂਰਨ L);
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_WSD
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GLOBAL(t,v) v
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3GlobalConfig sqlite3Config
#ਜੇ_ਅੰਤ

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਣਵਰਤਿਆ_PARAMETER(x) (ਖਾਲੀ)(x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਣਵਰਤਿਆ_PARAMETER2(x,y) ਅਣਵਰਤਿਆ_PARAMETER(x),ਅਣਵਰਤਿਆ_PARAMETER(y)

/*
** Forward references to structures
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ AggInfo AggInfo;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ AuthContext AuthContext;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ AutoincInfo AutoincInfo;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Bitvec Bitvec;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ CollSeq CollSeq;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਕਾਲਮ ਕਾਲਮ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Cte Cte;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ CteUse CteUse;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Db Db;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Db_Client_ਅੰਕੜੇ Db_Client_ਅੰਕੜੇ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ DbFixer DbFixer;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Schema Schema;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਵਿਅੰਜਕ ਵਿਅੰਜਕ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਵਿਅੰਜਕ_List ਵਿਅੰਜਕ_List;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ FKey FKey;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ FpDecode FpDecode;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ FuncDestructor FuncDestructor;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ FuncDef FuncDef;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ FuncDefHash FuncDefHash;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ IdList IdList;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਸੂਚਕਾਂਕ ਸੂਚਕਾਂਕ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਸੂਚਕਾਂਕ_Sample ਸੂਚਕਾਂਕ_Sample;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਕੁੰਜੀ_Class ਕੁੰਜੀ_Class;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਕੁੰਜੀ_Info ਕੁੰਜੀ_Info;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Lookaside Lookaside;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Lookaside_ਥਾਂ Lookaside_ਥਾਂ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Module Module;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ NameContext NameContext;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ OnOrUsing OnOrUsing;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Parse Parse;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ParseCleanup ParseCleanup;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ PreUpdate PreUpdate;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ PrintfArguments PrintfArguments;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ RCStr RCStr;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ RenameToken RenameToken;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Returning Returning;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਪੰਕਤੀ_ਸੈੱਟ ਪੰਕਤੀ_ਸੈੱਟ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Savepoint Savepoint;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Select Select;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ SQLiteThread SQLiteThread;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ SelectDest SelectDest;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Subquery Subquery;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Src_ਵਸਤੂ Src_ਵਸਤੂ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ SrcList SrcList;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ sqlite3_str StrAccum; /* Internal alias for sqlite3_str */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਸਾਰਣੀ ਸਾਰਣੀ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਸਾਰਣੀ_ਤਾਲਾ ਸਾਰਣੀ_ਤਾਲਾ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Token Token;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਰੁੱਖ_View ਰੁੱਖ_View;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Trigger Trigger;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ TriggerPrg TriggerPrg;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Trigger_ਕਦਮ Trigger_ਕਦਮ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Unpacked_ਰਿਕਾਰਡ Unpacked_ਰਿਕਾਰਡ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Upsert Upsert;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ VTable VTable;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ VtabCtx VtabCtx;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Walker Walker;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ WhereInfo WhereInfo;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਖਿੜਕੀ ਖਿੜਕੀ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ With With;


/*
** The bitmask datatype defined below is used for various optimizations.
**
** Changing this from a 64-bit to a 32-bit type limits the number of
** tables in a join to 32 instead of 64.  But it also reduces the size
** of the library by 738 bytes on ix86.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_BITMASK_TYPE
  ਕਿਸਮਪਰਿਭਾਸ਼ਾ SQLITE_BITMASK_TYPE Bitmask;
#ਨਹੀਂਤਾਂ
  ਕਿਸਮਪਰਿਭਾਸ਼ਾ u64 Bitmask;
#ਜੇ_ਅੰਤ

/*
** The number of bits in a Bitmask.  "BMS" means "BitMask Size".
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BMS  ((ਪੂਰਨ)(ਆਕਾਰ(Bitmask)*8))

/*
** A bit in a Bitmask
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MASKBIT(n)    (((Bitmask)1)<<(n))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MASKBIT64(n)  (((u64)1)<<(n))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MASKBIT32(n)  (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)1)<<(n))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMASKBIT32(n) ((n)<=31?((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)1)<<(n):0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALLBITS       ((Bitmask)-1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TOPBIT        (((Bitmask)1)<<(BMS-1))

/* A VList object records a mapping between parameters/variables/wildcards
** in the SQL statement (such as $abc, @pqr, or :xyz) and the integer
** variable number associated with that parameter.  See the format description
** on the sqlite3VListAdd() routine for more information.  A VList is really
** just an array of integers.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ VList;

/*
** Defer sourcing vdbe.h and btree.h until after the "u8" and
** "BusyHandler" typedefs. vdbe.h also requires a few of the opaque
** pointer types (i.e. FuncDef) defined above.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "os.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pager.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "btree.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "vdbe.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pcache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "mutex.h"

/* The SQLITE_EXTRA_DURABLE compile-time option used to set the default
** synchronous setting to EXTRA.  It is no longer supported.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EXTRA_DURABLE
# warning Use SQLITE_DEFAULT_SYNCHRONOUS=3 instead ਦਾ SQLITE_EXTRA_DURABLE
# define SQLITE_DEFAULT_SYNCHRONOUS 3
#ਜੇ_ਅੰਤ

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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_SYNCHRONOUS
# define SQLITE_DEFAULT_SYNCHRONOUS 2
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_WAL_SYNCHRONOUS
# define SQLITE_DEFAULT_WAL_SYNCHRONOUS SQLITE_DEFAULT_SYNCHRONOUS
#ਜੇ_ਅੰਤ

/*
** Each database file to be accessed by the system is an instance
** of the following structure.  There are normally two of these structures
** in the sqlite.aDb[] array.  aDb[0] is the main database file and
** aDb[1] is the database file used to hold temporary tables.  Additional
** databases may be attached.
*/
ਬਣਤਰ Db {
  ਅੱਖਰ *zDbSName;      /* Name of this database. (schema name, not filename) */
  Btree *pBt;          /* The B*Tree structure for this database file */
  u8 safety_level;     /* How aggressive at syncing data to disk */
  u8 b_Sync_ਸੈੱਟ;         /* True if "PRAGMA synchronous=N" has been run */
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
ਬਣਤਰ Schema {
  ਪੂਰਨ schema_cookie;   /* Database schema version number for this file */
  ਪੂਰਨ iGeneration;     /* Generation counter.  Incremented with each change */
  Hash tblHash;        /* All tables indexed by name */
  Hash idxHash;        /* All (named) indices indexed by name */
  Hash trigHash;       /* All triggers indexed by name */
  Hash fkeyHash;       /* All foreign keys by referenced table name */
  ਸਾਰਣੀ *pSeqTab;      /* The sqlite_sequence table used by AUTOINCREMENT */
  u8 file_format;      /* Schema format version for this file */
  u8 enc;              /* Text encoding used by this database */
  u16 schemaFlags;     /* Flags associated with this schema */
  ਪੂਰਨ cache_ਮਾਪ;      /* Number of pages to use in the cache */
};

/*
** These macros can be used to test, set, or clear bits in the
** Db.pSchema->flags field.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DbHasProperty(D,I,P)     (((D)->aDb[I].pSchema->schemaFlags&(P))==(P))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DbHasAnyProperty(D,I,P)  (((D)->aDb[I].pSchema->schemaFlags&(P))!=0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Db_ਸੈੱਟ_Property(D,I,P)     (D)->aDb[I].pSchema->schemaFlags|=(P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DbClearProperty(D,I,P)   (D)->aDb[I].pSchema->schemaFlags&=~(P)

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DB_SchemaLoaded    0x0001  /* The schema has been loaded */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DB_UnresetViews    0x0002  /* Some views have defined column names */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DB_ResetWanted     0x0008  /* Reset the schema when nSchemaLock==0 */

/*
** The number of different kinds of things that can be limited
** using the sqlite3_limit() interface.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_N_ਹੱਦਬੰਦੀ (SQLITE_ਹੱਦਬੰਦੀ_CTES+1)

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
ਬਣਤਰ Lookaside {
  u32 bDisable;           /* Only operate the lookaside when zero */
  u16 sz;                 /* Size of each buffer in bytes */
  u16 szTrue;             /* True value of sz, even if disabled */
  u8 bMalloced;           /* True if pStart obtained from sqlite3_malloc() */
  u32 n_ਥਾਂ;              /* Number of lookaside slots allocated */
  u32 anStat[3];          /* 0: hits.  1: size misses.  2: full misses */
  Lookaside_ਥਾਂ *pInit;   /* List of buffers not previously used */
  Lookaside_ਥਾਂ *pFree;   /* List of available buffers */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TWOSIZE_LOOKASIDE
  Lookaside_ਥਾਂ *pSmallInit; /* List of small buffers not previously used */
  Lookaside_ਥਾਂ *pSmallFree; /* List of available small buffers */
  ਖਾਲੀ *p_ਵਿਚਕਾਰ;          /* First byte past end of full-size buffers and
                          ** the first byte of LOOKASIDE_SMALL buffers */
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  ਖਾਲੀ *p_ਸ਼ੁਰੂ;           /* First byte of available memory space */
  ਖਾਲੀ *p_ਸਿਰਾ;             /* First byte past end of available space */
  ਖਾਲੀ *p_True_ਸਿਰਾ;         /* True value of pEnd, when db->pnBytesFreed!=0 */
};
ਬਣਤਰ Lookaside_ਥਾਂ {
  Lookaside_ਥਾਂ *p_ਅਗਲਾ;    /* Next buffer in the list of free buffers */
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DisableLookaside  db->lookaside.bDisable++;db->lookaside.sz=0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EnableLookaside   db->lookaside.bDisable--;\
   db->lookaside.sz=db->lookaside.bDisable?0:db->lookaside.szTrue

/* Size of the smaller allocations in two-size lookaside */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TWOSIZE_LOOKASIDE
#  define LOOKASIDE_SMALL           0
#ਨਹੀਂਤਾਂ
#  define LOOKASIDE_SMALL         128
#ਜੇ_ਅੰਤ

/*
** A hash table for built-in function definitions.  (Application-defined
** functions use a regular table table from hash.h.)
**
** Hash each FuncDef structure into one of the FuncDefHash.a[] slots.
** Collisions are on the FuncDef.u.pHash chain.  Use the SQLITE_FUNC_HASH()
** macro to compute a hash on the function name.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_HASH_SZ 23
ਬਣਤਰ FuncDefHash {
  FuncDef *a[SQLITE_FUNC_HASH_SZ];       /* Hash table for functions */
};
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_HASH(C,L) (((C)+(L))%SQLITE_FUNC_HASH_SZ)

/*
** typedef for the authorization callback function.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ (*sqlite3_xauth)(ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,
                             ਅਚਲ ਅੱਖਰ*);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
/* This is an extra SQLITE_TRACE macro that indicates "legacy" tracing
** in the style of sqlite3_trace()
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_TRACE_LEGACY          0x40     /* Use the legacy xTrace */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_TRACE_XPROFILE        0x80     /* Use the legacy xProfile */
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_TRACE_LEGACY          0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_TRACE_XPROFILE        0
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_DEPRECATED */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_TRACE_NONLEGACY_ਨਕਾਬ  0x0f     /* Normal flags */

/*
** Maximum number of sqlite3.aDb[] entries.  This is the number of attached
** databases plus 2 for "main" and "temp".
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MAX_DB (SQLITE_MAX_ATTACHED+2)

/*
** Each database connection is an instance of the following structure.
*/
ਬਣਤਰ sqlite3 {
  sqlite3_vfs *pVfs;            /* OS Interface */
  ਬਣਤਰ Vdbe *pVdbe;           /* List of active virtual machines */
  CollSeq *pDfltColl;           /* BINARY collseq for the database encoding */
  sqlite3_mutex *mutex;         /* Connection mutex */
  Db *aDb;                      /* All backends */
  ਪੂਰਨ nDb;                      /* Number of backends currently in use */
  u32 mDbFlags;                 /* flags recording internal state */
  u64 flags;                    /* flags settable by pragmas. See below */
  i64 ਆਖਰੀ_Rowid;                /* ROWID of most recent insert (see above) */
  i64 szMmap;                   /* Default mmap_size setting */
  u32 n_Schema_ਤਾਲਾ;              /* Do not reset the schema when non-zero */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ openFlags;       /* Flags passed to sqlite3_vfs.xOpen() */
  ਪੂਰਨ err_ਕੋਡ;                  /* Most recent error code (SQLITE_*) */
  ਪੂਰਨ err_Byte_ਵਿੱਥ;            /* Byte offset of error in SQL statement */
  ਪੂਰਨ err_ਨਕਾਬ;                  /* & result codes with this before returning */
  ਪੂਰਨ iSysErrno;                /* Errno value from last system error */
  u8 enc;                       /* Text encoding */
  u8 autoCommit;                /* The auto-commit flag. */
  u8 ਆਰਜ਼ੀ_store;                /* 1: file 2: memory 0: default */
  u8 mallocFailed;              /* True if we have seen a malloc failure */
  u8 bBenignMalloc;             /* Do not require OOMs if true */
  u8 dflt_ਤਾਲਾ_ਬਹੁਲਕ;              /* Default locking-mode for attached dbs */
  ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ ਅਗਲਾ_Autovac;      /* Autovac setting after VACUUM if >=0 */
  u8 suppressErr;               /* Do not issue error messages if true */
  u8 vtabOnConflict;            /* Value to return for s3_vtab_on_conflict() */
  u8 isTransactionSavepoint;    /* True if the outermost savepoint is a TS */
  u8 mTrace;                    /* zero or more SQLITE_TRACE flags */
  u8 noSharedCache;             /* True if no shared-cache backends */
  u8 nSqlExec;                  /* Number of pending OP_SqlExec opcodes */
  u8 eOpenState;                /* Current condition of the connection */
  u8 nFpDigit;                  /* Significant digits to keep on double->text */
  ਪੂਰਨ ਅਗਲਾ_Pagesize;             /* Pagesize after VACUUM if >0 */
  i64 nChange;                  /* Value returned by sqlite3_changes() */
  i64 n_ਕੁੱਲ_Change;             /* Value returned by sqlite3_total_changes() */
  u32 dbOptFlags;               /* Flags to enable/disable optimizations */
  ਪੂਰਨ a_ਹੱਦਬੰਦੀ[SQLITE_N_ਹੱਦਬੰਦੀ];   /* Limits */
  ਪੂਰਨ nMaxSorterMmap;           /* Maximum size of regions mapped by sorter */
  ਬਣਤਰ sqlite3InitInfo {      /* Information used during initialization */
    Pgno newTnum;               /* Rootpage of table being initialized */
    u8 iDb;                     /* Which db file is being initialized */
    u8 busy;                    /* TRUE if initing. 2 if due to ADD COLUMN */
    ਬਿਨਾਨਿਸ਼ਾਨੀ orphanTrigger : 1; /* Last statement is orphaned TEMP trigger */
    ਬਿਨਾਨਿਸ਼ਾਨੀ imposter_ਸਾਰਣੀ : 2; /* Building an imposter table */
    ਬਿਨਾਨਿਸ਼ਾਨੀ reopenMemdb : 1;   /* ATTACH is really a reopen using MemDB */
    ਅਚਲ ਅੱਖਰ **azInit;        /* "type", "name", and "tbl_name" columns */
  } init;
  ਪੂਰਨ n_Vdbe_ਸਰਗਰਮ;              /* Number of VDBEs currently running */
  ਪੂਰਨ nVdbeRead;                /* Number of active VDBEs that read or write */
  ਪੂਰਨ nVdbeWrite;               /* Number of active VDBEs that read and write */
  ਪੂਰਨ nVdbeExec;                /* Number of nested calls to VdbeExec() */
  ਪੂਰਨ nVDestroy;                /* Number of active OP_VDestroy operations */
  ਪੂਰਨ nExtension;               /* Number of loaded extensions */
  ਖਾਲੀ **aExtension;            /* Array of shared library handles */
  ਸੰਘ {
    ਖਾਲੀ (*xLegacy)(ਖਾਲੀ*,ਅਚਲ ਅੱਖਰ*);   /* mTrace==SQLITE_TRACE_LEGACY */
    ਪੂਰਨ (*xV2)(u32,ਖਾਲੀ*,ਖਾਲੀ*,ਖਾਲੀ*);    /* All other mTrace values */
  } trace;
  ਖਾਲੀ *pTraceArg;                        /* Argument to the trace function */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
  ਖਾਲੀ (*xProfile)(ਖਾਲੀ*,ਅਚਲ ਅੱਖਰ*,u64);  /* Profiling function */
  ਖਾਲੀ *pProfileArg;                        /* Argument to profile function */
#ਜੇ_ਅੰਤ
  ਖਾਲੀ *pCommitArg;                 /* Argument to xCommitCallback() */
  ਪੂਰਨ (*xCommitCallback)(ਖਾਲੀ*);    /* Invoked at every commit. */
  ਖਾਲੀ *pRollbackArg;               /* Argument to xRollbackCallback() */
  ਖਾਲੀ (*xRollbackCallback)(ਖਾਲੀ*); /* Invoked at every commit. */
  ਖਾਲੀ *pUpdateArg;
  ਖਾਲੀ (*xUpdateCallback)(ਖਾਲੀ*,ਪੂਰਨ, ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,sqlite_int64);
  ਖਾਲੀ *p_Autovac_ਪੰਨੇ_Arg;           /* Client argument to autovac_pages */
  ਖਾਲੀ (*xAutovacDestr)(ਖਾਲੀ*);     /* Destructor for pAutovacPAgesArg */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ (*x_Autovac_ਪੰਨੇ)(ਖਾਲੀ*,ਅਚਲ ਅੱਖਰ*,u32,u32,u32);
  Parse *pParse;                /* Current parse */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_PREUPDATE_HOOK
  ਖਾਲੀ *pPreUpdateArg;          /* First argument to xPreUpdateCallback */
  ਖਾਲੀ (*xPreUpdateCallback)(   /* Registered using sqlite3_preupdate_hook() */
    ਖਾਲੀ*,sqlite3*,ਪੂਰਨ,ਅੱਖਰ ਅਚਲ*,ਅੱਖਰ ਅਚਲ*,sqlite3_int64,sqlite3_int64
  );
  PreUpdate *pPreUpdate;        /* Context for active pre-update callback */
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_PREUPDATE_HOOK */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  ਪੂਰਨ (*xWalCallback)(ਖਾਲੀ *, sqlite3 *, ਅਚਲ ਅੱਖਰ *, ਪੂਰਨ);
  ਖਾਲੀ *pWalArg;
#ਜੇ_ਅੰਤ
  ਖਾਲੀ(*xCollNeeded)(ਖਾਲੀ*,sqlite3*,ਪੂਰਨ e_ਪਾਠ_Rep,ਅਚਲ ਅੱਖਰ*);
  ਖਾਲੀ(*xCollNeeded16)(ਖਾਲੀ*,sqlite3*,ਪੂਰਨ e_ਪਾਠ_Rep,ਅਚਲ ਖਾਲੀ*);
  ਖਾਲੀ *pCollNeededArg;
  sqlite3_ਮੁੱਲ *pErr;          /* Most recent error message */
  ਸੰਘ {
    ਅਸਥਿਰ ਪੂਰਨ isInterrupted; /* True if sqlite3_interrupt has been called */
    ਦੁੱਗਣਾ notUsed1;            /* Spacer */
  } u1;
  Lookaside lookaside;          /* Lookaside malloc configuration */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth;          /* Access authorization function */
  ਖਾਲੀ *pAuthArg;               /* 1st argument to the access auth function */
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਤਰੱਕੀ_CALLBACK
  ਪੂਰਨ (*x_ਤਰੱਕੀ)(ਖਾਲੀ *);     /* The progress callback */
  ਖਾਲੀ *p_ਤਰੱਕੀ_Arg;           /* Argument to the progress callback */
  ਬਿਨਾਨਿਸ਼ਾਨੀ n_ਤਰੱਕੀ_Ops;        /* Number of opcodes for progress callback */
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਪੂਰਨ nVTrans;                  /* Allocated size of aVTrans */
  Hash aModule;                 /* populated by sqlite3_create_module() */
  VtabCtx *pVtabCtx;            /* Context for active vtab connect/create */
  VTable **aVTrans;             /* Virtual tables with open transactions */
  VTable *pDisconnect;          /* Disconnect these in next sqlite3_prepare() */
#ਜੇ_ਅੰਤ
  Hash aFunc;                   /* Hash table of connection functions */
  Hash aCollSeq;                /* All collating sequences */
  BusyHandler busyHandler;      /* Busy callback */
  Db aDbStatic[2];              /* Static space for the 2 default backends */
  Savepoint *pSavepoint;        /* List of active savepoints */
  ਪੂਰਨ n_Analysis_ਹੱਦਬੰਦੀ;           /* Number of index rows to ANALYZE */
  ਪੂਰਨ busyTimeout;              /* Busy handler timeout, in msec */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
  ਪੂਰਨ setlkTimeout;             /* Blocking lock timeout, in msec. -1 -> inf. */
  ਪੂਰਨ setlkFlags;               /* Flags passed to setlk_timeout() */
#ਜੇ_ਅੰਤ
  ਪੂਰਨ nSavepoint;               /* Number of non-transaction savepoints */
  ਪੂਰਨ nStatement;               /* Number of nested statement-transactions  */
  i64 nDeferredCons;            /* Net deferred constraints this transaction. */
  i64 nDeferredImmCons;         /* Net deferred immediate constraints */
  ਪੂਰਨ *pn_ਬਾਈਟਾਂ_Freed;            /* If not NULL, increment this in DbFree() */
  Db_Client_ਅੰਕੜੇ *p_Db_ਅੰਕੜੇ;        /* sqlite3_set_clientdata() content */
  u64 nSpill;                   /* TEMP content spilled to disk */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UNLOCK_NOTIFY
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
  ਖਾਲੀ *pUnlockArg;                     /* Argument to xUnlockNotify */
  ਖਾਲੀ (*xUnlockNotify)(ਖਾਲੀ **, ਪੂਰਨ);  /* Unlock notify callback */
  sqlite3 *p_ਅਗਲਾ_Blocked;        /* Next in list of all blocked connections */
#ਜੇ_ਅੰਤ
};

/*
** A macro to discover the encoding of a database.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SCHEMA_ENC(db) ((db)->aDb[0].pSchema->enc)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENC(db)        ((db)->enc)

/*
** A u64 constant where the lower 32 bits are all zeros.  Only the
** upper 32 bits are included in the argument.  Necessary because some
** C-compilers still do not accept LL integer literals.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HI(X)  ((u64)(X)<<32)

/*
** Possible values for the sqlite3.flags.
**
** Value constraints (enforced via assert()):
**      SQLITE_FullFSync     == PAGER_FULLFSYNC
**      SQLITE_CkptFullFSync == PAGER_CKPT_FULLFSYNC
**      SQLITE_CacheSpill    == PAGER_CACHE_SPILL
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_WriteSchema    0x00000001  /* OK to update SQLITE_SCHEMA */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_LegacyFileFmt  0x00000002  /* Create new databases in format 1 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਭਰਿਆ_ਖਾਨਾ_ਨਾਂ   0x00000004  /* Show full column names on SELECT */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਭਰਿਆ_FSync      0x00000008  /* Use full fsync on the backend */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Ckpt_ਭਰਿਆ_FSync  0x00000010  /* Use full fsync for checkpoint */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_CacheSpill     0x00000020  /* OK to spill pager cache */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Short_ਖਾਨਾ_ਨਾਂ  0x00000040  /* Show short columns names */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_TrustedSchema  0x00000080  /* Allow unsafe functions and
                                          ** vtabs in the schema definition */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_NullCallback   0x00000100  /* Invoke the callback once if the */
                                          /*   result set is empty */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_IgnoreChecks   0x00000200  /* Do not enforce check constraints */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Stmt_ਜਾਂਚ_ਸਥਿਤੀ 0x00000400  /* Enable stmt_scanstats() counters */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_NoCkptOnClose  0x00000800  /* No checkpoint on close()/DETACH */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਉਲਟ_ਕ੍ਰਮ   0x00001000  /* Reverse unordered SELECTs */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_RecTriggers    0x00002000  /* Enable recursive triggers */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Foreign_ਕੁੰਜੀਆਂ    0x00004000  /* Enforce foreign key constraints  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Auto_ਸੂਚਕਾਂਕ      0x00008000  /* Enable automatic indexes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਲੱਦੋ_Extension  0x00010000  /* Enable load_extension */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਲੱਦੋ_Ext_Func    0x00020000  /* Enable load_extension() SQL func */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_EnableTrigger  0x00040000  /* True to enable triggers */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_DeferFKs       0x00080000  /* Defer all FK constraints */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_QueryOnly      0x00100000  /* Disable database changes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Cell_ਮਾਪ_Ck     0x00200000  /* Check btree cell sizes on load */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Fts3Tokenizer  0x00400000  /* Enable fts3_tokenizer(2) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_EnableQPSG     0x00800000  /* Query Planner Stability Guarantee*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_TriggerEQP     0x01000000  /* Show trigger EXPLAIN QUERY PLAN */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ResetDatabase  0x02000000  /* Reset the database */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_LegacyAlter    0x04000000  /* Legacy ALTER TABLE behaviour */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_NoSchemaError  0x08000000  /* Do not report schema parse errors*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Defensive      0x10000000  /* Input SQL is likely hostile */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_DqsDDL         0x20000000  /* dbl-quoted strings allowed in DDL*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_DqsDML         0x40000000  /* dbl-quoted strings allowed in DML*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_EnableView     0x80000000  /* Enable the use of views */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਗਿਣਤੀ_ਪੰਕਤੀਆਂ      HI(0x00001) /* Count rows changed by INSERT, */
                                          /*   DELETE, or UPDATE and return */
                                          /*   the count using a callback. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_CorruptRdOnly  HI(0x00002) /* Prohibit writes due to error */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ReadUncommit   HI(0x00004) /* READ UNCOMMITTED in shared-cache */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FkNoAction     HI(0x00008) /* Treat all FK as NO ACTION */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_AttachCreate   HI(0x00010) /* ATTACH allowed to create new dbs */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_AttachWrite    HI(0x00020) /* ATTACH allowed to open for write */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Comments       HI(0x00040) /* Enable SQL comments */

/* Flags used only if debugging */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_SqlTrace       HI(0x0100000) /* Debug print SQL as it executes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_VdbeListing    HI(0x0200000) /* Debug listings of VDBE progs */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_VdbeTrace      HI(0x0400000) /* True to trace VDBE execution */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_VdbeAddopTrace HI(0x0800000) /* Trace sqlite3VdbeAddOp() calls */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_VdbeEQP        HI(0x1000000) /* Debug EXPLAIN QUERY PLAN */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ParserTrace    HI(0x2000000) /* PRAGMA parser_trace=ON */
#ਜੇ_ਅੰਤ

/*
** Allowed values for sqlite3.mDbFlags
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBFLAG_SchemaChange   0x0001  /* Uncommitted Hash table changes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBFLAG_PreferBuiltin  0x0002  /* Preference to built-in funcs */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBFLAG_Vacuum         0x0004  /* Currently in a VACUUM */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBFLAG_VacuumInto     0x0008  /* Currently running VACUUM INTO */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBFLAG_Schema_ਗਿਆਤ_Ok  0x0010  /* Schema is known to be valid */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBFLAG_ਅੰਦਰੂਨੀ_Func   0x0020  /* Allow use of internal functions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBFLAG_EncodingFixed  0x0040  /* No longer possible to change enc. */

/*
** Bits of the sqlite3.dbOptFlags field that are used by the
** sqlite3_test_control(SQLITE_TESTCTRL_OPTIMIZATIONS,...) interface to
** selectively disable various optimizations. The numbers in each macro
** are the one-based bit position (for improved readability).
**
** Keep these values in sync with the ext/misc/optname.c extension.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_QueryFlattener  1 /* Query flattening */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਖਿੜਕੀ_Func      2 /* Use xInverse for window functions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਸਮੂਹ_By_ਕ੍ਰਮ    3 /* GROUPBY cover of ORDERBY */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Join_ਖੁੰਝ        4 /* Unwind join loops on a lookup miss*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_DistinctOpt     5 /* DISTINCT using indexes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Cover_Idx_ਜਾਂਚ    6 /* Covering index scans */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਕ੍ਰਮ_By_Idx_Join  7 /* ORDER BY of joins via index */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਸੰਕ੍ਰਾਮਕ      8 /* Transitive constraints */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_OmitNoopJoin    9 /* Omit unused tables in joins */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਗਿਣਤੀ_ਦਾ_View    10 /* The count-of-view optimization */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_CursorHints    11 /* Add OP_CursorHint opcodes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Stat4          12 /* Use STAT4 data */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਧੱਕੋ_ਥੱਲੇ       13 /* WHERE-clause push-down opt */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_SimplifyJoin   14 /* Convert LEFT JOIN to JOIN */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Skip_ਜਾਂਚ       15 /* Skip-scans */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_PropagateConst 16 /* The constant propagation opt */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MinMaxOpt      17 /* The min/max optimization */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Seek_ਜਾਂਚ       18 /* The OP_SeekScan optimization */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Omit_ਕ੍ਰਮ_By    19 /* Omit pointless ORDER BY */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_BloomFilter    20 /* Use a Bloom filters */
                          /*  21 -- Available for reuse */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਸੰਤੁਲਿਤ_ਮਿਲਾਓ  22 /* Balance multi-way merges */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ReleaseReg     23 /* Use OP_ReleaseReg for testing */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FlttnUnionAll  24 /* Disable the UNION ALL flattener */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ    25 /* Pull exprs from index when able */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Coroutines     26 /* Co-routines for subqueries */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Null_ਅਣਵਰਤਿਆ_ਖਾਨੇ_ਗਿਣਤੀ 27 /* NULL unused columns in subqueries */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_OnePass        28 /* Single-pass DELETE and UPDATE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਕ੍ਰਮ_By_Subq    29 /* ORDER BY in subquery helps outer */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ਤਾਰਾ_Query      30 /* Heurists for star queries */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Exists_ਮੰਜ਼ਿਲ_Join   31 /* The EXISTS-to-JOIN optimization */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_Union_ਹੱਦਬੰਦੀ     32 /* Optimizations for UNION + LIMIT */

/*
** Macros for testing whether or not optimizations are enabled or disabled.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OptimizationDisabled(D,M)  (((D)->dbOptFlags&(1ul<<((M)-1)))!=0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OptimizationEnabled(D,M)   (((D)->dbOptFlags&(1ul<<((M)-1)))==0)

/*
** Return true if it OK to factor constant expressions into the initialization
** code. The argument is a Parse object for the code generator.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Const_ਗੁਣਕ_Ok(P) ((P)->ok_Const_ਗੁਣਕ)

/* Possible values for the sqlite3.eOpenState field.
** The numbers are randomly selected such that a minimum of three bits must
** change to convert any number to another or to zero
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_STATE_OPEN     0x76  /* Database is open */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_STATE_CLOSED   0xce  /* Database is closed */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_STATE_SICK     0xba  /* Error and awaiting close */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_STATE_BUSY     0x6d  /* Database currently in use */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_STATE_ERROR    0xd5  /* An SQLITE_MISUSE error occurred */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_STATE_ZOMBIE   0xa7  /* Close with last statement close */

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
ਬਣਤਰ FuncDef {
  i16 nArg;            /* Number of arguments.  -1 means unlimited */
  u32 funcFlags;       /* Some combination of SQLITE_FUNC_* */
  ਖਾਲੀ *p_User_ਅੰਕੜੇ;     /* User data parameter */
  FuncDef *p_ਅਗਲਾ;      /* Next function with same name */
  ਖਾਲੀ (*xSFunc)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**); /* func or agg-step */
  ਖਾਲੀ (*xFinalize)(sqlite3_context*);                  /* Agg finalizer */
  ਖਾਲੀ (*x_ਮੁੱਲ)(sqlite3_context*);                     /* Current agg value */
  ਖਾਲੀ (*xInverse)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ**); /* inverse agg-step */
  ਅਚਲ ਅੱਖਰ *zName;   /* SQL name of the function. */
  ਸੰਘ {
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
ਬਣਤਰ FuncDestructor {
  ਪੂਰਨ nRef;
  ਖਾਲੀ (*xDestroy)(ਖਾਲੀ *);
  ਖਾਲੀ *p_User_ਅੰਕੜੇ;
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_ENCMASK  0x0003 /* SQLITE_UTF8, SQLITE_UTF16BE or UTF16LE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_LIKE     0x0004 /* Candidate for the LIKE optimization */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_CASE     0x0008 /* Case-sensitive LIKE-type function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_EPHEM    0x0010 /* Ephemeral.  Delete with VDBE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_NEEDCOLL 0x0020 /* sqlite3GetFuncCollSeq() might be called*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_ਲੰਮਾਈ   0x0040 /* Built-in length() function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_TYPEOF   0x0080 /* Built-in typeof() function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_BYTELEN  0x00c0 /* Built-in octet_length() function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_ਗਿਣਤੀ    0x0100 /* Built-in count(*) aggregate */
/*                           0x0200 -- available for reuse */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_UNLIKELY 0x0400 /* Built-in unlikely() function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_CONSTANT 0x0800 /* Constant inputs give a constant output */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_MINMAX   0x1000 /* True for min() and max() aggregates */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_SLOCHNG  0x2000 /* "Slow Change". Value constant during a
                                    ** single query - might change over time */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_TEST     0x4000 /* Built-in testing functions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_RUNONLY  0x8000 /* Cannot be used by valueFromFunction */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_ਖਿੜਕੀ   0x00010000 /* Built-in window-only function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_ਅੰਦਰੂਨੀ 0x00040000 /* For use by NestedParse() only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_ਸਿੱਧਾ   0x00080000 /* Not for use in TRIGGERs or VIEWs */
/* SQLITE_SUBTYPE            0x00100000 // Consumer of subtypes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_UNSAFE   0x00200000 /* Function has side effects */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_INLINE   0x00400000 /* Functions implemented in-line */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_BUILTIN  0x00800000 /* This is a built-in function */
/*  SQLITE_RESULT_SUBTYPE    0x01000000 // Generator of subtypes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FUNC_ANYORDER 0x08000000 /* count/min/max aggregate */

/* Identifier numbers for each in-line function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INLINEFUNC_coalesce             0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INLINEFUNC_ਸਿੱਟਾ_ਦਿੰਦਾ_nonnull_ਪੰਕਤੀ  1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INLINEFUNC_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_ਵਿਅੰਜਕ    2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INLINEFUNC_ਵਿਅੰਜਕ_compare         3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INLINEFUNC_affinity             4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INLINEFUNC_iif                  5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INLINEFUNC_sqlite_ਵਿੱਥ        6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INLINEFUNC_unlikely            99  /* Default case */

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_CONSTANT|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|SQLITE_DIRECTONLY|SQLITE_FUNC_UNSAFE, \
   SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MFUNCTION(zName, nArg, xPtr, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_CONSTANT|SQLITE_UTF8, \
   xPtr, 0, xFunc, 0, 0, 0, #zName, {0} }
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JFUNCTION(zName, nArg, bUseCache, bWS, bRS, bJsonB, iArg, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_DETERMINISTIC|SQLITE_FUNC_CONSTANT|\
   SQLITE_UTF8|((bUseCache)*SQLITE_FUNC_RUNONLY)|\
   ((bRS)*SQLITE_SUBTYPE)|((bWS)*SQLITE_ਨਤੀਜਾ_SUBTYPE), \
   SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(iArg|((bJsonB)*JSON_BLOB)),0,xFunc,0, 0, 0, #zName, {0} }
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INLINE_FUNC(zName, nArg, iArg, mFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_UTF8|SQLITE_FUNC_INLINE|SQLITE_FUNC_CONSTANT|(mFlags), \
   SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(iArg), 0, noopFunc, 0, 0, 0, #zName, {0} }
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TEST_FUNC(zName, nArg, iArg, mFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
         SQLITE_UTF8|SQLITE_FUNC_ਅੰਦਰੂਨੀ|SQLITE_FUNC_TEST| \
         SQLITE_FUNC_INLINE|SQLITE_FUNC_CONSTANT|(mFlags), \
   SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(iArg), 0, noopFunc, 0, 0, 0, #zName, {0} }
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_SLOCHNG|SQLITE_UTF8, \
   0, 0, xFunc, 0, 0, 0, #zName, {0} }
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸ਼ੁੱਧ_DATE(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
         SQLITE_FUNC_SLOCHNG|SQLITE_UTF8|SQLITE_FUNC_CONSTANT, \
   (ਖਾਲੀ*)&sqlite3Config, 0, xFunc, 0, 0, 0, #zName, {0} }
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FUNCTION2(zName, nArg, iArg, bNC, xFunc, extraFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_CONSTANT|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL)|extraFlags,\
   SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STR_FUNCTION(zName, nArg, pArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_SLOCHNG|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   pArg, 0, xFunc, 0, 0, 0, #zName, {0} }
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LIKEFUNC(zName, nArg, arg, flags) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_CONSTANT|SQLITE_UTF8|flags, \
   (ਖਾਲੀ *)arg, 0, likeFunc, 0, 0, 0, #zName, {0} }
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WAGGREGATE(zName, nArg, arg, nc, x_ਕਦਮ, xFinal, x_ਮੁੱਲ, xInverse, f) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|(nc*SQLITE_FUNC_NEEDCOLL)|f, \
   SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(arg), 0, x_ਕਦਮ,xFinal,x_ਮੁੱਲ,xInverse,#zName, {0}}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਦਰੂਨੀ_FUNCTION(zName, nArg, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_ਅੰਦਰੂਨੀ|SQLITE_UTF8|SQLITE_FUNC_CONSTANT, \
   0, 0, xFunc, 0, 0, 0, #zName, {0} }


/*
** All current savepoints are stored in a linked list starting at
** sqlite3.pSavepoint. The first element in the list is the most recently
** opened savepoint. Savepoints are added to the list by the vdbe
** OP_Savepoint instruction.
*/
ਬਣਤਰ Savepoint {
  ਅੱਖਰ *zName;                        /* Savepoint name (nul-terminated) */
  i64 nDeferredCons;                  /* Number of deferred fk violations */
  i64 nDeferredImmCons;               /* Number of deferred imm fk. */
  Savepoint *p_ਅਗਲਾ;                   /* Parent savepoint (if any) */
};

/*
** The following are used as the second parameter to sqlite3Savepoint(),
** and as the P1 argument to the OP_Savepoint instruction.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SAVEPOINT_BEGIN      0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SAVEPOINT_RELEASE    1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SAVEPOINT_ROLLBACK   2


/*
** Each SQLite module (virtual table definition) is defined by an
** instance of the following structure, stored in the sqlite3.aModule
** hash table.
*/
ਬਣਤਰ Module {
  ਅਚਲ sqlite3_module *pModule;       /* Callback pointers */
  ਅਚਲ ਅੱਖਰ *zName;                   /* Name passed to create_module() */
  ਪੂਰਨ nRefModule;                      /* Number of pointers to this object */
  ਖਾਲੀ *pAux;                          /* pAux passed to create_module() */
  ਖਾਲੀ (*xDestroy)(ਖਾਲੀ *);            /* Module destructor function */
  ਸਾਰਣੀ *pEpoTab;                      /* Eponymous table for this module */
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
ਬਣਤਰ ਕਾਲਮ {
  ਅੱਖਰ *zCnName;        /* Name of this column */
  ਬਿਨਾਨਿਸ਼ਾਨੀ notNull :4;  /* An OE_ code for handling a NOT NULL constraint */
  ਬਿਨਾਨਿਸ਼ਾਨੀ eCType :4;   /* One of the standard types */
  ਅੱਖਰ affinity;        /* One of the SQLITE_AFF_... values */
  u8 szEst;             /* Est size of value in this column. sizeof(INT)==1 */
  u8 hName;             /* Column name hash for faster lookup */
  u16 iDflt;            /* 1-based index of DEFAULT.  0 means "none" */
  u16 ਖਾਨਾ_Flags;         /* Boolean properties.  See COLFLAG_ defines below */
};

/* Allowed values for Column.eCType.
**
** Values must match entries in the global constant arrays
** sqlite3StdTypeLen[] and sqlite3StdType[].  Each value is one more
** than the offset into these arrays for the corresponding name.
** Adjust the SQLITE_N_STDTYPE value if adding or removing entries.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLTYPE_CUSTOM      0   /* Type appended to zName */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLTYPE_ANY         1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLTYPE_BLOB        2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLTYPE_INT         3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLTYPE_INTEGER     4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLTYPE_REAL        5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLTYPE_ਪਾਠ        6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_N_STDTYPE    6  /* Number of standard types */

/* Allowed values for Column.colFlags.
**
** Constraints:
**         TF_HasVirtual == COLFLAG_VIRTUAL
**         TF_HasStored  == COLFLAG_STORED
**         TF_HasHidden  == COLFLAG_HIDDEN
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLFLAG_PRIMKEY   0x0001   /* Column is part of the primary key */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLFLAG_HIDDEN    0x0002   /* A hidden column in a virtual table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLFLAG_HASTYPE   0x0004   /* Type name follows column name */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLFLAG_UNIQUE    0x0008   /* Column def contains "UNIQUE" or "PK" */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLFLAG_SORTERREF 0x0010   /* Use sorter-refs with this column */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLFLAG_VIRTUAL   0x0020   /* GENERATED ALWAYS AS ... VIRTUAL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLFLAG_STORED    0x0040   /* GENERATED ALWAYS AS ... STORED */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLFLAG_NOTAVAIL  0x0080   /* STORED column not yet calculated */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLFLAG_BUSY      0x0100   /* Blocks recursion on GENERATED columns */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLFLAG_HASCOLL   0x0200   /* Has collating sequence name in zCnName */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLFLAG_NOEXPAND  0x0400   /* Omit this column when expanding "*" */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLFLAG_GENERATED 0x0060   /* Combo: _STORED, _VIRTUAL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ COLFLAG_NOINSERT  0x0062   /* Combo: _HIDDEN, _STORED, _VIRTUAL */

/*
** A "Collating Sequence" is defined by an instance of the following
** structure. Conceptually, a collating sequence consists of a name and
** a comparison routine that defines the order of that sequence.
**
** If CollSeq.xCmp is NULL, it means that the
** collating sequence is undefined.  Indices built on an undefined
** collating sequence may not be read or written.
*/
ਬਣਤਰ CollSeq {
  ਅੱਖਰ *zName;          /* Name of the collating sequence, UTF-8 encoded */
  u8 enc;               /* Text encoding handled by xCmp() */
  ਖਾਲੀ *pUser;          /* First argument to xCmp() */
  ਪੂਰਨ (*xCmp)(ਖਾਲੀ*,ਪੂਰਨ, ਅਚਲ ਖਾਲੀ*, ਪੂਰਨ, ਅਚਲ ਖਾਲੀ*);
  ਖਾਲੀ (*xDel)(ਖਾਲੀ*);  /* Destructor for pUser */
};

/*
** A sort order can be either ASC or DESC.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_SO_ASC       0  /* Sort in ascending order */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_SO_DESC      1  /* Sort in ascending order */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_SO_UNDEFINED -1 /* No sort order specified */

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_AFF_NONE     0x40  /* '@' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_AFF_BLOB     0x41  /* 'A' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_AFF_ਪਾਠ     0x42  /* 'B' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_AFF_NUMERIC  0x43  /* 'C' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_AFF_INTEGER  0x44  /* 'D' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_AFF_REAL     0x45  /* 'E' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_AFF_FLEXNUM  0x46  /* 'F' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_AFF_DEFER    0x58  /* 'X'  - defer computation until later */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3IsNumericAffinity(X)  ((X)>=SQLITE_AFF_NUMERIC)

/*
** The SQLITE_AFF_MASK values masks off the significant bits of an
** affinity value.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_AFF_ਨਕਾਬ     0x47

/*
** Additional bit values that can be ORed with an affinity without
** changing the affinity.
**
** The SQLITE_NOTNULL flag is a combination of NULLEQ and JUMPIFNULL.
** It causes an assert() to fire if either operand to a comparison
** operator is NULL.  It is added to certain comparison operators to
** prove that the operands are always NOT NULL.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_JUMPIFNULL   0x10  /* jumps if either operand is NULL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_NULLEQ       0x80  /* NULL=NULL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_NOTNULL      0x90  /* Assert that operands are never NULL */

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
ਬਣਤਰ VTable {
  sqlite3 *db;              /* Database connection associated with this table */
  Module *pMod;             /* Pointer to module implementation */
  sqlite3_vtab *pVtab;      /* Pointer to vtab instance */
  ਪੂਰਨ nRef;                 /* Number of pointers to this structure */
  u8 bConstraint;           /* True if constraints are supported */
  u8 bAllSchemas;           /* True if might use any attached schema */
  u8 eVtabRisk;             /* Riskiness of allowing hacker access */
  ਪੂਰਨ iSavepoint;           /* Depth of the SAVEPOINT stack */
  VTable *p_ਅਗਲਾ;            /* Next in linked list (see above) */
};

/* Allowed values for VTable.eVtabRisk
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_VTABRISK_ਨੀਵਾਂ          0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_VTABRISK_ਸਾਧਾਰਨ       1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_VTABRISK_ਉੱਚਾ         2

/*
** The schema for each SQL table, virtual table, and view is represented
** in memory by an instance of the following structure.
*/
ਬਣਤਰ ਸਾਰਣੀ {
  ਅੱਖਰ *zName;         /* Name of the table or view */
  ਕਾਲਮ *a_ਖਾਨਾ;        /* Information about each column */
  ਸੂਚਕਾਂਕ *p_ਸੂਚਕਾਂਕ;       /* List of SQL indexes on this table. */
  ਅੱਖਰ *z_ਖਾਨਾ_Aff;       /* String defining the affinity of each column */
  ਵਿਅੰਜਕ_List *pCheck;    /* All CHECK constraints */
                       /*   ... also used as column name list in a VIEW */
  Pgno tnum;           /* Root BTree page for this table */
  u32 nTabRef;         /* Number of pointers to this Table */
  u32 tabFlags;        /* Mask of TF_* values */
  i16 iPKey;           /* If not negative, use aCol[iPKey] as the rowid */
  i16 n_ਖਾਨਾ;            /* Number of columns in this table */
  i16 nNVCol;          /* Number of columns that are not VIRTUAL */
  LogEst n_ਪੰਕਤੀ_Log_Est;   /* Estimated rows in table - from sqlite_stat1 table */
  LogEst sz_Tab_ਪੰਕਤੀ;     /* Estimated size of each table row in bytes */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_COSTMULT
  LogEst ਲਾਗਤ_Mult;     /* Cost multiplier for using this table */
#ਜੇ_ਅੰਤ
  u8 ਕੁੰਜੀ_Conf;          /* What to do in case of uniqueness conflict on iPKey */
  u8 eTabType;         /* 0: normal, 1: virtual, 2: view */
  ਸੰਘ {
    ਬਣਤਰ {             /* Used by ordinary tables: */
      ਪੂਰਨ ਜੋੜੋ_ਖਾਨਾ_ਵਿੱਥ;    /* Offset in CREATE TABLE stmt to add a new column */
      FKey *pFKey;         /* Linked list of all foreign keys in this table */
      ਵਿਅੰਜਕ_List *pDfltList; /* DEFAULT clauses on various columns.
                           ** Or the AS clause for generated columns. */
    } tab;
    ਬਣਤਰ {             /* Used by views: */
      Select *pSelect;     /* View definition */
    } view;
    ਬਣਤਰ {             /* Used by virtual tables only: */
      ਪੂਰਨ nArg;            /* Number of arguments to the module */
      ਅੱਖਰ **azArg;        /* 0: module 1: schema 2: vtab name 3...: args */
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_Readonly       0x00000001 /* Read-only system table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_HasHidden      0x00000002 /* Has one or more hidden columns */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_Has_Primary_ਕੁੰਜੀ  0x00000004 /* Table has a primary key */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_Autoincrement  0x00000008 /* Integer primary key is autoincrement */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_HasStat1       0x00000010 /* nRowLogEst set from sqlite_stat1 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_HasVirtual     0x00000020 /* Has one or more VIRTUAL columns */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_HasStored      0x00000040 /* Has one or more STORED columns */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_HasGenerated   0x00000060 /* Combo: HasVirtual + HasStored */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_WithoutRowid   0x00000080 /* No rowid.  PRIMARY KEY is the key */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_MaybeReanalyze 0x00000100 /* Maybe run ANALYZE on this table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_NoVisibleRowid 0x00000200 /* No user-visible "rowid" column */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_OOOHidden      0x00000400 /* Out-of-Order hidden columns */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_HasNotNull     0x00000800 /* Contains NOT NULL constraints */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_Shadow         0x00001000 /* True for a shadow table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_HasStat4       0x00002000 /* STAT4 info available for this table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_Ephemeral      0x00004000 /* An ephemeral table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_Eponymous      0x00008000 /* An eponymous virtual table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_Strict         0x00010000 /* STRICT mode */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TF_Imposter       0x00020000 /* An imposter table */

/*
** Allowed values for Table.eTabType
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TABTYP_NORM      0     /* Ordinary table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TABTYP_VTAB      1     /* Virtual table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TABTYP_VIEW      2     /* A view */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IsView(X)           ((X)->eTabType==TABTYP_VIEW)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Is_Ordinary_ਸਾਰਣੀ(X)  ((X)->eTabType==TABTYP_NORM)

/*
** Test to see whether or not a table is a virtual table.  This is
** done as a macro so that it will be optimized out when virtual
** table support is omitted from the build.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
#  define IsVirtual(X)      ((X)->eTabType==TABTYP_VTAB)
#  define ਵਿਅੰਜਕ_Is_Vtab(X)  \
   ((X)->op==TK_ਕਾਲਮ && (X)->y.pTab->eTabType==TABTYP_VTAB)
#ਨਹੀਂਤਾਂ
#  define IsVirtual(X)      0
#  define ਵਿਅੰਜਕ_Is_Vtab(X)     0
#ਜੇ_ਅੰਤ

/*
** Macros to determine if a column is hidden.  IsOrdinaryHiddenColumn()
** only works for non-virtual tables (ordinary tables and views) and is
** always false unless SQLITE_ENABLE_HIDDEN_COLUMNS is defined.  The
** IsHiddenColumn() macro is general purpose.
*/
#ਜੇ defined(SQLITE_ENABLE_HIDDEN_COLUMNS)
#  define Is_Hidden_ਕਾਲਮ(X)         (((X)->ਖਾਨਾ_Flags & COLFLAG_HIDDEN)!=0)
#  define Is_Ordinary_Hidden_ਕਾਲਮ(X) (((X)->ਖਾਨਾ_Flags & COLFLAG_HIDDEN)!=0)
#ਜਾਂਜੇ !defined(SQLITE_OMIT_VIRTUALTABLE)
#  define Is_Hidden_ਕਾਲਮ(X)         (((X)->ਖਾਨਾ_Flags & COLFLAG_HIDDEN)!=0)
#  define Is_Ordinary_Hidden_ਕਾਲਮ(X) 0
#ਨਹੀਂਤਾਂ
#  define Is_Hidden_ਕਾਲਮ(X)         0
#  define Is_Ordinary_Hidden_ਕਾਲਮ(X) 0
#ਜੇ_ਅੰਤ


/* Does the table have a rowid */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HasRowid(X)     (((X)->tabFlags & TF_WithoutRowid)==0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VisibleRowid(X) (((X)->tabFlags & TF_NoVisibleRowid)==0)

/* Macro is true if the SQLITE_ALLOW_ROWID_IN_VIEW (mis-)feature is
** available.  By default, this macro is false
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ALLOW_ROWID_ਵਿੱਚ_VIEW
# define ViewCanHaveRowid     0
#ਨਹੀਂਤਾਂ
# define ViewCanHaveRowid     (sqlite3Config.mNoVisibleRowid==0)
#ਜੇ_ਅੰਤ

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
ਬਣਤਰ FKey {
  ਸਾਰਣੀ *pFrom;     /* Table containing the REFERENCES clause (aka: Child) */
  FKey *p_ਅਗਲਾ_From;  /* Next FKey with the same in pFrom. Next parent of pFrom */
  ਅੱਖਰ *z_ਮੰਜ਼ਿਲ;        /* Name of table that the key points to (aka: Parent) */
  FKey *p_ਅਗਲਾ_ਮੰਜ਼ਿਲ;    /* Next with the same zTo. Next child of zTo. */
  FKey *p_Prev_ਮੰਜ਼ਿਲ;    /* Previous with the same zTo */
  ਪੂਰਨ n_ਖਾਨਾ;         /* Number of columns in this key */
  /* EV: R-30323-21917 */
  u8 isDeferred;       /* True if constraint checking is deferred till COMMIT */
  u8 aAction[2];        /* ON DELETE and ON UPDATE actions, respectively */
  Trigger *apTrigger[2];/* Triggers for aAction[] actions */
  ਬਣਤਰ s_ਖਾਨਾ_Map {      /* Mapping of columns in pFrom to columns in zTo */
    ਪੂਰਨ iFrom;            /* Index of column in pFrom */
    ਅੱਖਰ *z_ਖਾਨਾ;           /* Name of column in zTo.  If NULL use PRIMARY KEY */
  } a_ਖਾਨਾ[FLEXARRAY];      /* One entry for each of nCol columns */
};

/* The size (in bytes) of an FKey object holding N columns.  The answer
** does NOT include space to hold the zTo name. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SZ_FKEY(N)  (offsetof(FKey,a_ਖਾਨਾ)+(N)*ਆਕਾਰ(ਬਣਤਰ s_ਖਾਨਾ_Map))

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OE_None     0   /* There is no constraint to check */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OE_Rollback 1   /* Fail the operation and rollback the transaction */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OE_Abort    2   /* Back out changes but do no rollback transaction */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OE_Fail     3   /* Stop the operation but leave all prior changes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OE_Ignore   4   /* Ignore the error. Do not do the INSERT or UPDATE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OE_Replace  5   /* Delete existing record, then do INSERT or UPDATE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OE_Update   6   /* Process as a DO UPDATE in an upsert */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OE_Restrict 7   /* OE_Abort for IMMEDIATE, OE_Rollback for DEFERRED */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OE_ਸੈੱਟ_Null  8   /* Set the foreign key value to NULL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OE_ਸੈੱਟ_Dflt  9   /* Set the foreign key value to its default */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OE_Cascade  10  /* Cascade the changes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OE_Default  11  /* Do whatever the default action is */


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
ਬਣਤਰ ਕੁੰਜੀ_Info {
  u32 nRef;           /* Number of references to this KeyInfo object */
  u8 enc;             /* Text encoding - one of the SQLITE_UTF* values */
  u16 n_ਕੁੰਜੀ_ਖੇਤਰ;      /* Number of key columns in the index */
  u16 n_All_ਖੇਤਰ;      /* Total columns, including key plus others */
  sqlite3 *db;        /* The database connection */
  u8 *a_ਛਾਂਟੋ_Flags;     /* Sort order for each column. */
  CollSeq *aColl[FLEXARRAY]; /* Collating sequence for each term of the key */
};

/* The size (in bytes) of a KeyInfo object with up to N fields.  This includes
** the main body of the KeyInfo object and the aColl[] array of N elements,
** but does not count the memory used to hold aSortFlags[]. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SZ_KEYINFO(N)  (offsetof(ਕੁੰਜੀ_Info,aColl) + (N)*ਆਕਾਰ(CollSeq*))

/* The size of a bare KeyInfo with no aColl[] entries */
#ਜੇ FLEXARRAY+1 > 1
# define SZ_KEYINFO_0   offsetof(ਕੁੰਜੀ_Info,aColl)
#ਨਹੀਂਤਾਂ
# define SZ_KEYINFO_0   ਆਕਾਰ(ਕੁੰਜੀ_Info)
#ਜੇ_ਅੰਤ

/*
** Allowed bit values for entries in the KeyInfo.aSortFlags[] array.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KEYINFO_ਕ੍ਰਮ_DESC    0x01    /* DESC sort order */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ KEYINFO_ਕ੍ਰਮ_BIGNULL 0x02    /* NULL is larger than any other value */

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
ਬਣਤਰ Unpacked_ਰਿਕਾਰਡ {
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info;  /* Comparison info for the index that is unpacked */
  Mem *aMem;          /* Values for columns of the index */
  ਸੰਘ {
    ਅੱਖਰ *z;            /* Cache of aMem[0].z for vdbeRecordCompareString() */
    i64 i;              /* Cache of aMem[0].u.i for vdbeRecordCompareInt() */
  } u;
  ਪੂਰਨ n;              /* Cache of aMem[0].n used by vdbeRecordCompareString() */
  u16 n_ਖੇਤਰ;         /* Number of entries in apMem[] */
  i8 default_rc;      /* Comparison result if keys are equal */
  u8 err_ਕੋਡ;         /* Error detected by xRecordCompare (CORRUPT or NOMEM) */
  i8 r1;              /* Value to return if (lhs < rhs) */
  i8 r2;              /* Value to return if (lhs > rhs) */
  u8 eq_ਵੇਖਿਆ;          /* True if an equality comparison has been seen */
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
ਬਣਤਰ ਸੂਚਕਾਂਕ {
  ਅੱਖਰ *zName;             /* Name of this index */
  i16 *ai_ਕਾਲਮ;           /* Which columns are used by this index.  1st is 0 */
  LogEst *ai_ਪੰਕਤੀ_Log_Est;     /* From ANALYZE: Est. rows selected by each column */
  ਸਾਰਣੀ *p_ਸਾਰਣੀ;           /* The SQL table being indexed */
  ਅੱਖਰ *z_ਖਾਨਾ_Aff;           /* String defining the affinity of each column */
  ਸੂਚਕਾਂਕ *p_ਅਗਲਾ;            /* The next index associated with the same table */
  Schema *pSchema;         /* Schema containing this index */
  u8 *a_ਛਾਂਟੋ_ਕ੍ਰਮ;          /* for each column: True==DESC, False==ASC */
  ਅਚਲ ਅੱਖਰ **azColl;     /* Array of collation sequence names for index */
  ਵਿਅੰਜਕ *pPartIdxWhere;     /* WHERE clause for partial indices */
  ਵਿਅੰਜਕ_List *a_ਖਾਨਾ_ਵਿਅੰਜਕ;      /* Column expressions */
  Pgno tnum;               /* DB Page containing root of this index */
  LogEst sz_Idx_ਪੰਕਤੀ;         /* Estimated average row size in bytes */
  u16 n_ਕੁੰਜੀ_ਖਾਨਾ;             /* Number of columns forming the key */
  u16 n_ਕਾਲਮ;             /* Nr columns in btree. Can be 2*Table.nCol */
  u8 onError;              /* OE_Abort, OE_Ignore, OE_Replace, or OE_None */
  ਬਿਨਾਨਿਸ਼ਾਨੀ idxType:2;      /* 0:Normal 1:UNIQUE, 2:PRIMARY KEY, 3:IPK */
  ਬਿਨਾਨਿਸ਼ਾਨੀ bUnordered:1;   /* Use this index for == or IN queries only */
  ਬਿਨਾਨਿਸ਼ਾਨੀ uniqNotNull:1;  /* True if UNIQUE and NOT NULL for all columns */
  ਬਿਨਾਨਿਸ਼ਾਨੀ isResized:1;    /* True if resizeIndexObject() has been called */
  ਬਿਨਾਨਿਸ਼ਾਨੀ isCovering:1;   /* True if this is a covering index */
  ਬਿਨਾਨਿਸ਼ਾਨੀ no_Skip_ਜਾਂਚ:1;   /* Do not try to use skip-scan if true */
  ਬਿਨਾਨਿਸ਼ਾਨੀ hasStat1:1;     /* aiRowLogEst values come from sqlite_stat1 */
  ਬਿਨਾਨਿਸ਼ਾਨੀ bNoQuery:1;     /* Do not use this index to optimize queries */
  ਬਿਨਾਨਿਸ਼ਾਨੀ b_Asc_ਕੁੰਜੀ_Bug:1;   /* True if the bba7b69f9849b5bf bug applies */
  ਬਿਨਾਨਿਸ਼ਾਨੀ bHasVCol:1;     /* Index references one or more VIRTUAL columns */
  ਬਿਨਾਨਿਸ਼ਾਨੀ b_Has_ਵਿਅੰਜਕ:1;     /* Index contains an expression, either a literal
                           ** expression, or a reference to a VIRTUAL column */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
  ਪੂਰਨ nSample;             /* Number of elements in aSample[] */
  ਪੂਰਨ mxSample;            /* Number of slots allocated to aSample[] */
  ਪੂਰਨ n_Sample_ਖਾਨਾ;          /* Size of IndexSample.anEq[] and so on */
  tRowcnt *aAvgEq;         /* Average nEq values for keys not in aSample */
  ਸੂਚਕਾਂਕ_Sample *aSample;    /* Samples of the left-most key */
  tRowcnt *ai_ਪੰਕਤੀ_Est;       /* Non-logarithmic stat1 data for this index */
  tRowcnt n_ਪੰਕਤੀ_Est0;        /* Non-logarithmic number of rows in the index */
#ਜੇ_ਅੰਤ
  Bitmask ਖਾਨਾ_Not_Idxed;     /* Unindexed columns in pTab */
};

/*
** Allowed values for Index.idxType
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_IDXTYPE_APPDEF      0   /* Created using CREATE INDEX */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_IDXTYPE_UNIQUE      1   /* Implements a UNIQUE constraint */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_IDXTYPE_PRIMARYKEY  2   /* Is the PRIMARY KEY for the table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_IDXTYPE_IPK         3   /* INTEGER PRIMARY KEY index */

/* Return true if index X is a PRIMARY KEY index */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(X)  ((X)->idxType==SQLITE_IDXTYPE_PRIMARYKEY)

/* Return true if index X is a UNIQUE index */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Is_Unique_ਸੂਚਕਾਂਕ(X)      ((X)->onError!=OE_None)

/* The Index.aiColumn[] values are normally positive integer.  But
** there are some negative values that have special meaning:
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XN_ROWID     (-1)     /* Indexed column is the rowid */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ XN_ਵਿਅੰਜਕ      (-2)     /* Indexed column is an expression */

/*
** Each sample stored in the sqlite_stat4 table is represented in memory
** using a structure of this type.  See documentation at the top of the
** analyze.c source file for additional information.
*/
ਬਣਤਰ ਸੂਚਕਾਂਕ_Sample {
  ਖਾਲੀ *p;          /* Pointer to sampled record */
  ਪੂਰਨ n;            /* Size of record in bytes */
  tRowcnt *anEq;    /* Est. number of rows where the key equals this sample */
  tRowcnt *anLt;    /* Est. number of rows where key is less than this sample */
  tRowcnt *anDLt;   /* Est. number of distinct keys less than this sample */
};

/*
** Possible values to use within the flags argument to sqlite3GetToken().
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_TOKEN_QUOTED    0x1 /* Token is a quoted identifier. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_TOKEN_KEYWORD   0x2 /* Token is a keyword. */

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
ਬਣਤਰ Token {
  ਅਚਲ ਅੱਖਰ *z;     /* Text of the token.  Not NULL-terminated! */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n;    /* Number of characters in this token */
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
ਬਣਤਰ AggInfo {
  u8 ਸਿੱਧਾ_ਬਹੁਲਕ;          /* Direct rendering mode means take data directly
                          ** from source tables rather than from accumulators */
  u8 use_ਛਾਂਟੀ_Idx;       /* In direct mode, reference the sorting index rather
                          ** than the source table */
  u32 n_ਛਾਂਟੀ_ਕਾਲਮ;     /* Number of columns in the sorting index */
  ਪੂਰਨ ਛਾਂਟੀ_Idx;         /* Cursor number of the sorting index */
  ਪੂਰਨ ਛਾਂਟੀ_Idx_PTab;     /* Cursor number of pseudo-table */
  ਪੂਰਨ i_ਪਹਿਲਾ_Reg;          /* First register in range for aCol[] and aFunc[] */
  ਵਿਅੰਜਕ_List *p_ਸਮੂਹ_By;     /* The group by clause */
  ਬਣਤਰ Agg_Info_ਖਾਨਾ {    /* For each column used in source tables */
    ਸਾਰਣੀ *pTab;             /* Source table */
    ਵਿਅੰਜਕ *pCExpr;            /* The original expression */
    ਪੂਰਨ i_ਸਾਰਣੀ;              /* Cursor number of the source table */
    ਪੂਰਨ i_ਕਾਲਮ;             /* Column number within the source table */
    ਪੂਰਨ i_Sorter_ਕਾਲਮ;       /* Column number in the sorting index */
  } *a_ਖਾਨਾ;
  ਪੂਰਨ n_ਕਾਲਮ;            /* Number of used entries in aCol[] */
  ਪੂਰਨ n_ਸੰਚਾਇਕ;       /* Number of columns that show through to the output.
                          ** Additional columns are used only as parameters to
                          ** aggregate functions */
  ਬਣਤਰ AggInfo_func {   /* For each aggregate function */
    ਵਿਅੰਜਕ *pFExpr;            /* Expression encoding the function */
    FuncDef *pFunc;          /* The aggregate function implementation */
    ਪੂਰਨ iDistinct;           /* Ephemeral table used to enforce DISTINCT */
    ਪੂਰਨ iDistAddr;           /* Address of OP_OpenEphemeral */
    ਪੂਰਨ iOBTab;              /* Ephemeral table to implement ORDER BY */
    u8 bOBPayload;           /* iOBTab has payload columns separate from key */
    u8 bOBUnique;            /* Enforce uniqueness on iOBTab keys */
    u8 bUseSubtype;          /* Transfer subtype info through sorter */
  } *aFunc;
  ਪੂਰਨ nFunc;              /* Number of entries in aFunc[] */
  u32 selId;              /* Select to which this AggInfo belongs */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  Select *pSelect;        /* SELECT statement that this AggInfo supports */
#ਜੇ_ਅੰਤ
};

/*
** Macros to compute aCol[] and aFunc[] register numbers.
**
** These macros should not be used prior to the call to
** assignAggregateRegisters() that computes the value of pAggInfo->iFirstReg.
** The assert()s that are part of this macro verify that constraint.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Agg_Info_ਕਾਲਮ_Reg(A,I)  (ਪ੍ਰਤਿਜ੍ਞਾ((A)->i_ਪਹਿਲਾ_Reg),(A)->i_ਪਹਿਲਾ_Reg+(I))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AggInfoFuncReg(A,I)    \
                      (ਪ੍ਰਤਿਜ੍ਞਾ((A)->i_ਪਹਿਲਾ_Reg),(A)->i_ਪਹਿਲਾ_Reg+(A)->n_ਕਾਲਮ+(I))
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Agg_Info_ਕਾਲਮ_Reg(A,I)  ((A)->i_ਪਹਿਲਾ_Reg+(I))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AggInfoFuncReg(A,I)    \
                      ((A)->i_ਪਹਿਲਾ_Reg+(A)->n_ਕਾਲਮ+(I))
#ਜੇ_ਅੰਤ

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
#ਜੇ SQLITE_MAX_VARIABLE_ਸੰਖਿਆ<32767
ਕਿਸਮਪਰਿਭਾਸ਼ਾ i16 ynVar;
#ਨਹੀਂਤਾਂ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ ynVar;
#ਜੇ_ਅੰਤ

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
ਬਣਤਰ ਵਿਅੰਜਕ {
  u8 op;                 /* Operation performed by this node */
  ਅੱਖਰ aff_ਵਿਅੰਜਕ;          /* affinity, or RAISE type */
  u8 op2;                /* TK_REGISTER/TK_TRUTH: original value of Expr.op
                         ** TK_COLUMN: the value of p5 for OP_Column
                         ** TK_AGG_FUNCTION: nesting depth
                         ** TK_FUNCTION: NC_SelfRef flag if needs OP_PureFunc */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  u8 vvaFlags;           /* Verification flags. */
#ਜੇ_ਅੰਤ
  u32 flags;             /* Various flags.  EP_* See below */
  ਸੰਘ {
    ਅੱਖਰ *zToken;          /* Token value. Zero terminated and dequoted */
    ਪੂਰਨ i_ਮੁੱਲ;            /* Non-negative integer value if EP_IntValue */
  } u;

  /* If the EP_TokenOnly flag is set in the Expr.flags mask, then no
  ** space is allocated for the fields below this point. An attempt to
  ** access them will result in a segfault or malfunction.
  *********************************************************************/

  ਵਿਅੰਜਕ *p_ਖੱਬਾ;           /* Left subnode */
  ਵਿਅੰਜਕ *p_ਸੱਜਾ;          /* Right subnode */
  ਸੰਘ {
    ਵਿਅੰਜਕ_List *pList;     /* op = IN, EXISTS, SELECT, CASE, FUNCTION, BETWEEN */
    Select *pSelect;     /* EP_xIsSelect and op = IN, EXISTS, SELECT */
  } x;

  /* If the EP_Reduced flag is set in the Expr.flags mask, then no
  ** space is allocated for the fields below this point. An attempt to
  ** access them will result in a segfault or malfunction.
  *********************************************************************/

#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
  ਪੂਰਨ n_ਉਚਾਈ;           /* Height of the tree headed by this node */
#ਜੇ_ਅੰਤ
  ਪੂਰਨ i_ਸਾਰਣੀ;            /* TK_COLUMN: cursor number of table holding column
                         ** TK_REGISTER: register number
                         ** TK_TRIGGER: 1 -> new, 0 -> old
                         ** EP_Unlikely:  134217728 times likelihood
                         ** TK_IN: ephemeral table holding RHS
                         ** TK_SELECT_COLUMN: Number of columns on the LHS
                         ** TK_SELECT: 1st register of result vector */
  ynVar i_ਕਾਲਮ;         /* TK_COLUMN: column index.  -1 for rowid.
                         ** TK_VARIABLE: variable number (always >= 1).
                         ** TK_SELECT_COLUMN: column of the result vector */
  i16 iAgg;              /* Which entry in pAggInfo->aCol[] or ->aFunc[] */
  ਸੰਘ {
    ਪੂਰਨ iJoin;             /* If EP_OuterON or EP_InnerON, the right table */
    ਪੂਰਨ iOfst;             /* else: start of token from start of statement */
  } w;
  AggInfo *pAggInfo;     /* Used by TK_AGG_COLUMN and TK_AGG_FUNCTION */
  ਸੰਘ {
    ਸਾਰਣੀ *pTab;           /* TK_COLUMN: Table containing column. Can be NULL
                           ** for a column of an index on an expression */
    ਖਿੜਕੀ *pWin;          /* EP_WinFunc: Window/Filter defn for a function */
    ਪੂਰਨ nReg;              /* TK_NULLS: Number of registers to NULL out */
    ਬਣਤਰ {               /* TK_IN, TK_SELECT, and TK_EXISTS */
      ਪੂਰਨ iAddr;             /* Subroutine entry address */
      ਪੂਰਨ regReturn;         /* Register used to hold return address */
    } sub;
  } y;
};

/* The following are the meanings of bits in the Expr.flags field.
** Value restrictions:
**
**          EP_Agg == NC_HasAgg == SF_HasAgg
**          EP_Win == NC_HasWin
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_OuterON    0x000001 /* Originates in ON/USING clause of outer join */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_InnerON    0x000002 /* Originates in ON/USING of an inner join */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Distinct   0x000004 /* Aggregate function with DISTINCT keyword */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_HasFunc    0x000008 /* Contains one or more functions of any kind */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Agg        0x000010 /* Contains one or more aggregate functions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Fixed_ਖਾਨਾ   0x000020 /* TK_Column with a known fixed value */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_VarSelect  0x000040 /* pSelect is correlated, not constant */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_DblQuoted  0x000080 /* token.z was originally in "..." */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_InfixFunc  0x000100 /* True for an infix function: LIKE, GLOB, etc */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Collate    0x000200 /* Tree contains a TK_COLLATE operator */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Commuted   0x000400 /* Comparison operator has been commuted */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Int_ਮੁੱਲ   0x000800 /* Integer value contained in u.iValue */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_xIsSelect  0x001000 /* x.pSelect is valid (otherwise x.pList is) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Skip       0x002000 /* Operator does not contribute to affinity */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Reduced    0x004000 /* Expr struct EXPR_REDUCEDSIZE bytes only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Win        0x008000 /* Contains window functions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_TokenOnly  0x010000 /* Expr struct EXPR_TOKENONLYSIZE bytes only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_ਭਰਿਆ_ਮਾਪ   0x020000 /* Expr structure must remain full sized */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_If_Null_ਪੰਕਤੀ  0x040000 /* The TK_IF_NULL_ROW opcode */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Unlikely   0x080000 /* unlikely() or likelihood() function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_ConstFunc  0x100000 /* A SQLITE_FUNC_CONSTANT or _SLOCHNG function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_CanBeNull  0x200000 /* Can be null despite NOT NULL constraint */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Subquery   0x400000 /* Tree contains a TK_SELECT operator */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_ਪੱਤੀ       0x800000 /* Expr.pLeft, .pRight, .u.pSelect all NULL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_WinFunc   0x1000000 /* TK_FUNCTION with Expr.y.pWin set */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Subrtn    0x2000000 /* Uses Expr.y.sub. TK_IN, _SELECT, or _EXISTS */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Quoted    0x4000000 /* TK_ID was originally quoted */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Static    0x8000000 /* Held in memory not obtained from malloc() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_IsTrue   0x10000000 /* Always has boolean value of TRUE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_IsFalse  0x20000000 /* Always has boolean value of FALSE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_FromDDL  0x40000000 /* Originates from sqlite_schema */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_SubtArg  0x80000000 /* Is argument to SQLITE_SUBTYPE function */

/* The EP_Propagate mask is a set of properties that automatically propagate
** upwards into parent nodes.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Propagate (EP_Collate|EP_Subquery|EP_HasFunc)

/* Macros can be used to test, set, or clear bits in the
** Expr.flags field.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Has_Property(E,P)     (((E)->flags&(u32)(P))!=0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Has_All_Property(E,P)  (((E)->flags&(u32)(P))==(u32)(P))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_ਸੈੱਟ_Property(E,P)     (E)->flags|=(u32)(P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Clear_Property(E,P)   (E)->flags&=~(u32)(P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Always_True(E)   (((E)->flags&(EP_OuterON|EP_IsTrue))==EP_IsTrue)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Always_False(E)  (((E)->flags&(EP_OuterON|EP_IsFalse))==EP_IsFalse)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Is_ਭਰਿਆ_ਮਾਪ(E)   (((E)->flags&(EP_Reduced|EP_TokenOnly))==0)

/* Macros used to ensure that the correct members of unions are accessed
** in Expr.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Use_UToken(E)    (((E)->flags&EP_Int_ਮੁੱਲ)==0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Use_UValue(E)    (((E)->flags&EP_Int_ਮੁੱਲ)!=0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Use_WOfst(E)     (((E)->flags&(EP_InnerON|EP_OuterON))==0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Use_WJoin(E)     (((E)->flags&(EP_InnerON|EP_OuterON))!=0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Use_XList(E)     (((E)->flags&EP_xIsSelect)==0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Use_XSelect(E)   (((E)->flags&EP_xIsSelect)!=0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Use_YTab(E)      (((E)->flags&(EP_WinFunc|EP_Subrtn))==0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Use_YWin(E)      (((E)->flags&EP_WinFunc)!=0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_Use_YSub(E)      (((E)->flags&EP_Subrtn)!=0)

/* Flags for use with Expr.vvaFlags
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_NoReduce   0x01  /* Cannot EXPRDUP_REDUCE this Expr */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EP_Immutable  0x02  /* Do not change this Expr node */

/* The ExprSetVVAProperty() macro is used for Verification, Validation,
** and Accreditation only.  It works like ExprSetProperty() during VVA
** processes but is a no-op for delivery.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
# define ਵਿਅੰਜਕ_ਸੈੱਟ_VVAProperty(E,P)   (E)->vvaFlags|=(P)
# define ਵਿਅੰਜਕ_Has_VVAProperty(E,P)   (((E)->vvaFlags&(P))!=0)
# define ਵਿਅੰਜਕ_Clear_VVAProperties(E) (E)->vvaFlags = 0
#ਨਹੀਂਤਾਂ
# define ਵਿਅੰਜਕ_ਸੈੱਟ_VVAProperty(E,P)
# define ਵਿਅੰਜਕ_Has_VVAProperty(E,P)   0
# define ਵਿਅੰਜਕ_Clear_VVAProperties(E)
#ਜੇ_ਅੰਤ

/*
** Macros to determine the number of bytes required by a normal Expr
** struct, an Expr struct with the EP_Reduced flag set in Expr.flags
** and an Expr struct with the EP_TokenOnly flag set.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_FULLSIZE           ਆਕਾਰ(ਵਿਅੰਜਕ)           /* Full size */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_REDUCEDSIZE        offsetof(ਵਿਅੰਜਕ,i_ਸਾਰਣੀ)  /* Common features */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿਅੰਜਕ_TOKENONLYSIZE      offsetof(ਵਿਅੰਜਕ,p_ਖੱਬਾ)   /* Fewer features */

/*
** Flags passed to the sqlite3ExprDup() function. See the header comment
** above sqlite3ExprDup() for details.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXPRDUP_REDUCE         0x0001  /* Used reduced-size Expr nodes */

/*
** True if the expression passed as an argument was a function with
** an OVER() clause (a window function).
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
# define Is_ਖਿੜਕੀ_Func(p) 0
#ਨਹੀਂਤਾਂ
# define Is_ਖਿੜਕੀ_Func(p) ( \
    ਵਿਅੰਜਕ_Has_Property((p), EP_WinFunc) && p->y.pWin->eFrmType!=TK_FILTER \
 )
#ਜੇ_ਅੰਤ

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
ਬਣਤਰ ਵਿਅੰਜਕ_List {
  ਪੂਰਨ n_ਵਿਅੰਜਕ;             /* Number of expressions on the list */
  ਪੂਰਨ nAlloc;            /* Number of a[] slots allocated */
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ { /* For each expression in the list */
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ;            /* The parse tree for this expression */
    ਅੱਖਰ *zEName;           /* Token associated with this expression */
    ਬਣਤਰ {
      u8 ਛਾਂਟੋ_Flags;           /* Mask of KEYINFO_ORDER_* flags */
      ਬਿਨਾਨਿਸ਼ਾਨੀ eEName :2;     /* Meaning of zEName */
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਾ :1;       /* Indicates when processing is finished */
      ਬਿਨਾਨਿਸ਼ਾਨੀ reusable :1;   /* Constant expression is reusable */
      ਬਿਨਾਨਿਸ਼ਾਨੀ bSorterRef :1; /* Defer evaluation until after sorting */
      ਬਿਨਾਨਿਸ਼ਾਨੀ bNulls :1;     /* True if explicit "NULLS FIRST/LAST" */
      ਬਿਨਾਨਿਸ਼ਾਨੀ b_ਵਰਤਿਆ :1;      /* This column used in a SF_NestedFrom subquery */
      ਬਿਨਾਨਿਸ਼ਾਨੀ b_Using_ਪਦ:1;  /* Term from the USING clause of a NestedFrom */
      ਬਿਨਾਨਿਸ਼ਾਨੀ bNoExpand: 1;  /* Term is an auxiliary in NestedFrom and should
                              ** not be expanded by "*" in parent queries */
    } fg;
    ਸੰਘ {
      ਬਣਤਰ {             /* Used by any ExprList other than Parse.pConsExpr */
        u16 i_ਕ੍ਰਮ_By_ਖਾਨਾ;      /* For ORDER BY, column number in result set */
        u16 iAlias;           /* Index into Parse.aAlias[] for zName */
      } x;
      ਪੂਰਨ i_Const_ਵਿਅੰਜਕ_Reg;   /* Register in which Expr value is cached. Used only
                           ** by Parse.pConstExpr */
    } u;
  } a[FLEXARRAY];          /* One slot for each expression in the list */
};

/* The size (in bytes) of an ExprList object that is big enough to hold
** as many as N expressions. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SZ_EXPRLIST(N)  \
             (offsetof(ਵਿਅੰਜਕ_List,a) + (N)*ਆਕਾਰ(ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ))

/*
** Allowed values for Expr.a.eEName
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENAME_NAME  0       /* The AS clause of a result set */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENAME_ਵਿਸਤਾਰ  1       /* Complete text of the result set expression */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENAME_TAB   2       /* "DB.TABLE.NAME" for the result set */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENAME_ROWID 3       /* "DB.TABLE._rowid_" for * expansion of rowid */

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
ਬਣਤਰ IdList {
  ਪੂਰਨ nId;         /* Number of identifiers on the list */
  ਬਣਤਰ Id_List_ਵਸਤੂ {
    ਅੱਖਰ *zName;      /* Name of the identifier */
  } a[FLEXARRAY];
};

/* The size (in bytes) of an IdList object that can hold up to N IDs. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SZ_IDLIST(N)  (offsetof(IdList,a)+(N)*ਆਕਾਰ(ਬਣਤਰ Id_List_ਵਸਤੂ))

/*
** Allowed values for IdList.eType, which determines which value of the a.u4
** is valid.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EU4_NONE   0   /* Does not use IdList.a.u4 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EU4_IDX    1   /* Uses IdList.a.u4.idx */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EU4_ਵਿਅੰਜਕ   2   /* Uses IdList.a.u4.pExpr -- NOT CURRENTLY USED */

/*
** Details of the implementation of a subquery.
*/
ਬਣਤਰ Subquery {
  Select *pSelect;  /* A SELECT statement used in place of a table name */
  ਪੂਰਨ addrFillSub;  /* Address of subroutine to initialize a subquery */
  ਪੂਰਨ regReturn;    /* Register holding return address of addrFillSub */
  ਪੂਰਨ reg_ਨਤੀਜਾ;    /* Registers holding results of a co-routine */
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
ਬਣਤਰ Src_ਵਸਤੂ {
  ਅੱਖਰ *zName;      /* Name of the table */
  ਅੱਖਰ *zAlias;     /* The "B" part of a "A AS B" phrase.  zName is the "A" */
  ਸਾਰਣੀ *pSTab;     /* Table object for zName. Mnemonic: Srcitem-TABle */
  ਬਣਤਰ {
    u8 jointype;      /* Type of join between this table and the previous */
    ਬਿਨਾਨਿਸ਼ਾਨੀ not_ਸੂਚਕਾਂਕਿਤ :1;    /* True if there is a NOT INDEXED clause */
    ਬਿਨਾਨਿਸ਼ਾਨੀ is_ਸੂਚਕਾਂਕਿਤ_By :1;   /* True if there is an INDEXED BY clause */
    ਬਿਨਾਨਿਸ਼ਾਨੀ isSubquery :1;    /* True if this term is a subquery */
    ਬਿਨਾਨਿਸ਼ਾਨੀ isTabFunc :1;     /* True if table-valued-function syntax */
    ਬਿਨਾਨਿਸ਼ਾਨੀ isCorrelated :1;  /* True if sub-query is correlated */
    ਬਿਨਾਨਿਸ਼ਾਨੀ isMaterialized:1; /* This is a materialized view */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਰਾਹੀਂ_Coroutine :1;  /* Implemented as a co-routine */
    ਬਿਨਾਨਿਸ਼ਾਨੀ isRecursive :1;   /* True for recursive reference in WITH */
    ਬਿਨਾਨਿਸ਼ਾਨੀ fromDDL :1;       /* Comes from sqlite_schema */
    ਬਿਨਾਨਿਸ਼ਾਨੀ isCte :1;         /* This is a CTE */
    ਬਿਨਾਨਿਸ਼ਾਨੀ notCte :1;        /* This item may not match a CTE */
    ਬਿਨਾਨਿਸ਼ਾਨੀ isUsing :1;       /* u3.pUsing is valid */
    ਬਿਨਾਨਿਸ਼ਾਨੀ isOn :1;          /* u3.pOn was once valid and non-NULL */
    ਬਿਨਾਨਿਸ਼ਾਨੀ isSynthUsing :1;  /* u3.pUsing is synthesized from NATURAL */
    ਬਿਨਾਨਿਸ਼ਾਨੀ isNestedFrom :1;  /* pSelect is a SF_NestedFrom subquery */
    ਬਿਨਾਨਿਸ਼ਾਨੀ rowid_ਵਰਤਿਆ :1;     /* The ROWID of this table is referenced */
    ਬਿਨਾਨਿਸ਼ਾਨੀ fixedSchema :1;   /* Uses u4.pSchema, not u4.zDatabase */
    ਬਿਨਾਨਿਸ਼ਾਨੀ hadSchema :1;     /* Had u4.zDatabase before u4.pSchema */
    ਬਿਨਾਨਿਸ਼ਾਨੀ fromExists :1;    /* Comes from WHERE EXISTS(...) */
  } fg;
  ਪੂਰਨ iCursor;      /* The VDBE cursor number used to access this table */
  Bitmask ਖਾਨਾ_ਵਰਤਿਆ;  /* Bit N set if column N used. Details above for N>62 */
  ਸੰਘ {
    ਅੱਖਰ *z_ਸੂਚਕਾਂਕਿਤ_By;    /* Identifier from "INDEXED BY <zIndex>" clause */
    ਵਿਅੰਜਕ_List *pFuncArg;  /* Arguments to table-valued-function */
    u32 n_ਪੰਕਤੀ;            /* Number of rows in a VALUES clause */
  } u1;
  ਸੰਘ {
    ਸੂਚਕਾਂਕ *pIBIndex;  /* Index structure corresponding to u1.zIndexedBy */
    CteUse *pCteUse;  /* CTE Usage info when fg.isCte is true */
  } u2;
  ਸੰਘ {
    ਵਿਅੰਜਕ *pOn;        /* fg.isUsing==0 =>  The ON clause of a join */
    IdList *pUsing;   /* fg.isUsing==1 =>  The USING clause of a join */
  } u3;
  ਸੰਘ {
    Schema *pSchema;  /* Schema to which this item is fixed */
    ਅੱਖਰ *zDatabase;  /* Name of database holding this table */
    Subquery *pSubq;  /* Description of a subquery */
  } u4;
};

/*
** The OnOrUsing object represents either an ON clause or a USING clause.
** It can never be both at the same time, but it can be neither.
*/
ਬਣਤਰ OnOrUsing {
  ਵਿਅੰਜਕ *pOn;         /* The ON clause of a join */
  IdList *pUsing;    /* The USING clause of a join */
};

/*
** This object represents one or more tables that are the source of
** content for an SQL statement.  For example, a single SrcList object
** is used to hold the FROM clause of a SELECT statement.  SrcList also
** represents the target tables for DELETE, INSERT, and UPDATE statements.
**
*/
ਬਣਤਰ SrcList {
  ਪੂਰਨ nSrc;             /* Number of tables or subqueries in the FROM clause */
  u32 nAlloc;           /* Number of entries allocated in a[] below */
  Src_ਵਸਤੂ a[FLEXARRAY]; /* One entry for each identifier on the list */
};

/* Size (in bytes) of a SrcList object that can hold as many as N
** SrcItem objects. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SZ_SRCLIST(N) (offsetof(SrcList,a)+(N)*ਆਕਾਰ(Src_ਵਸਤੂ))

/* Size (in bytes( of a SrcList object that holds 1 SrcItem.  This is a
** special case of SZ_SRCITEM(1) that comes up often. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SZ_SRCLIST_1  (offsetof(SrcList,a)+ਆਕਾਰ(Src_ਵਸਤੂ))

/*
** Permitted values of the SrcList.a.jointype field
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JT_INNER     0x01    /* Any kind of inner or cross join */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JT_CROSS     0x02    /* Explicit use of the CROSS keyword */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JT_NATURAL   0x04    /* True for a "natural" join */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JT_ਖੱਬਾ      0x08    /* Left outer join */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JT_ਸੱਜਾ     0x10    /* Right outer join */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JT_OUTER     0x20    /* The "OUTER" keyword is present */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JT_LTORJ     0x40    /* One of the LEFT operands of a RIGHT JOIN
                             ** Mnemonic: Left Table Of Right Join */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ JT_ERROR     0x80    /* unknown or unsupported join type */

/*
** Flags appropriate for the wctrlFlags parameter of sqlite3WhereBegin()
** and the WhereInfo.wctrlFlags member.
**
** Value constraints (enforced via assert()):
**     WHERE_USE_LIMIT  == SF_FixedLimit
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ORDERBY_ਸਾਧਾਰਨ   0x0000 /* No-op */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ORDERBY_MIN      0x0001 /* ORDER BY processing for min() func */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ORDERBY_MAX      0x0002 /* ORDER BY processing for max() func */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ONEPASS_DESIRED  0x0004 /* Want to do one-pass UPDATE/DELETE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ONEPASS_MULTIROW 0x0008 /* ONEPASS is ok with multiple rows */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_DUPLICATES_OK    0x0010 /* Ok to return a row more than once */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_OR_SUBCLAUSE     0x0020 /* Processing a sub-WHERE as part of
                                      ** the OR optimization  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_GROUPBY          0x0040 /* pOrderBy is really a GROUP BY */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_DISTINCTBY       0x0080 /* pOrderby is really a DISTINCT clause */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_WANT_DISTINCT    0x0100 /* All output needs to be distinct */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_SORTBYGROUP      0x0200 /* Support sqlite3WhereIsSorted() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_AGG_DISTINCT     0x0400 /* Query is "SELECT agg(DISTINCT ...)" */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ORDERBY_ਹੱਦਬੰਦੀ    0x0800 /* ORDERBY+LIMIT on the inner loop */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_ਸੱਜਾ_JOIN       0x1000 /* Processing a RIGHT JOIN */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_KEEP_ALL_JOINS   0x2000 /* Do not do the omit-noop-join opt */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_USE_ਹੱਦਬੰਦੀ        0x4000 /* Use the LIMIT in cost estimates */
                        /*     0x8000    not currently used */

/* Allowed return values from sqlite3WhereIsDistinct()
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_DISTINCT_NOOP      0  /* DISTINCT keyword not used */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_DISTINCT_UNIQUE    1  /* No duplicates */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_DISTINCT_ORDERED   2  /* All duplicates are adjacent */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WHERE_DISTINCT_UNORDERED 3  /* Duplicates are scattered */

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
ਬਣਤਰ NameContext {
  Parse *pParse;       /* The parser */
  SrcList *pSrcList;   /* One or more tables used to resolve names */
  ਸੰਘ {
    ਵਿਅੰਜਕ_List *pEList;    /* Optional list of result-set columns */
    AggInfo *pAggInfo;   /* Information about aggregates at this level */
    Upsert *pUpsert;     /* ON CONFLICT clause information from an upsert */
    ਪੂਰਨ i_ਅਧਾਰ_Reg;        /* For TK_REGISTER when parsing RETURNING */
  } uNC;
  NameContext *p_ਅਗਲਾ;  /* Next outer name context.  NULL for outermost */
  ਪੂਰਨ nRef;            /* Number of names resolved by this context */
  ਪੂਰਨ nNcErr;          /* Number of errors encountered while resolving names */
  ਪੂਰਨ ncFlags;         /* Zero or more NC_* flags defined below */
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_AllowAgg  0x000001 /* Aggregate functions are allowed here */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_PartIdx   0x000002 /* True if resolving a partial index WHERE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_IsCheck   0x000004 /* True if resolving a CHECK constraint */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_Gen_ਖਾਨਾ    0x000008 /* True for a GENERATED ALWAYS AS clause */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_HasAgg    0x000010 /* One or more aggregate functions seen */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_Idx_ਵਿਅੰਜਕ   0x000020 /* True if resolving columns of CREATE INDEX */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_SelfRef   0x00002e /* Combo: PartIdx, isCheck, GenCol, and IdxExpr */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_Subquery  0x000040 /* A subquery has been seen */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_UEList    0x000080 /* True if uNC.pEList is used */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_UAggInfo  0x000100 /* True if uNC.pAggInfo is used */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_UUpsert   0x000200 /* True if uNC.pUpsert is used */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_UBaseReg  0x000400 /* True if uNC.iBaseReg is used */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_MinMaxAgg 0x001000 /* min/max aggregates seen.  See note above */
/*                   0x002000 // available for reuse */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_AllowWin  0x004000 /* Window functions are allowed here */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_HasWin    0x008000 /* One or more window functions seen */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_IsDDL     0x010000 /* Resolving names in a CREATE statement */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_ਵਿੱਚ_Agg_Func 0x020000 /* True if analyzing arguments to an agg func */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_FromDDL   0x040000 /* SQL text comes from sqlite_schema */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_NoSelect  0x080000 /* Do not descend into sub-selects */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_Where     0x100000 /* Processing WHERE clause of a SELECT */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NC_ਕ੍ਰਮ_Agg 0x8000000 /* Has an aggregate other than count/min/max */

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
ਬਣਤਰ Upsert {
  ਵਿਅੰਜਕ_List *p_Upsert_ਨਿਸ਼ਾਨਾ;  /* Optional description of conflict target */
  ਵਿਅੰਜਕ *p_Upsert_ਨਿਸ਼ਾਨਾ_Where; /* WHERE clause for partial index targets */
  ਵਿਅੰਜਕ_List *p_Upsert_ਸੈੱਟ;     /* The SET clause from an ON CONFLICT UPDATE */
  ਵਿਅੰਜਕ *pUpsertWhere;       /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_ਅਗਲਾ_Upsert;      /* Next ON CONFLICT clause in the list */
  u8 isDoUpdate;            /* True for DO UPDATE.  False for DO NOTHING */
  u8 isDup;                 /* True if 2nd or later with same pUpsertIdx */
  /* Above this point is the parse tree for the ON CONFLICT clauses.
  ** The next group of fields stores intermediate data. */
  ਖਾਲੀ *p_ਮੰਜ਼ਿਲ_Free;            /* Free memory when deleting the Upsert object */
  /* All fields above are owned by the Upsert object and must be freed
  ** when the Upsert is destroyed.  The fields below are used to transfer
  ** information from the INSERT processing down into the UPDATE processing
  ** while generating code.  The fields below are owned by the INSERT
  ** statement and will be freed by INSERT processing. */
  ਸੂਚਕਾਂਕ *pUpsertIdx;        /* UNIQUE constraint specified by pUpsertTarget */
  SrcList *pUpsertSrc;      /* Table to be updated */
  ਪੂਰਨ reg_ਅੰਕੜੇ;              /* First register holding array of VALUES */
  ਪੂਰਨ i_ਅੰਕੜੇ_Cur;             /* Index of the data cursor */
  ਪੂਰਨ iIdxCur;              /* Index of the first index cursor */
};

/*
** An instance of the following structure contains all information
** needed to generate code for a single SELECT statement.
*/
ਬਣਤਰ Select {
  u8 op;                 /* One of: TK_UNION TK_ALL TK_INTERSECT TK_EXCEPT */
  LogEst n_Select_ਪੰਕਤੀ;     /* Estimated number of result rows */
  u32 selFlags;          /* Various SF_* values */
  ਪੂਰਨ i_ਹੱਦਬੰਦੀ, i_ਵਿੱਥ;   /* Memory registers holding LIMIT & OFFSET counters */
  u32 selId;             /* Unique identifier number for this SELECT */
  ਵਿਅੰਜਕ_List *pEList;      /* The fields of the result */
  SrcList *pSrc;         /* The FROM clause */
  ਵਿਅੰਜਕ *pWhere;          /* The WHERE clause */
  ਵਿਅੰਜਕ_List *p_ਸਮੂਹ_By;    /* The GROUP BY clause */
  ਵਿਅੰਜਕ *pHaving;         /* The HAVING clause */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By;    /* The ORDER BY clause */
  Select *p_ਪੂਰਵ;        /* Prior select in a compound select statement */
  Select *p_ਅਗਲਾ;         /* Next select to the left in a compound */
  ਵਿਅੰਜਕ *p_ਹੱਦਬੰਦੀ;          /* LIMIT expression. NULL means not used. */
  With *pWith;           /* WITH clause attached to this select. Or NULL. */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
  ਖਿੜਕੀ *pWin;          /* List of window functions */
  ਖਿੜਕੀ *pWinDefn;      /* List of named window definitions */
#ਜੇ_ਅੰਤ
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_Distinct      0x0000001 /* Output should be DISTINCT */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_All           0x0000002 /* Includes the ALL keyword */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_Resolved      0x0000004 /* Identifiers have been resolved */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_Aggregate     0x0000008 /* Contains agg functions or a GROUP BY */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_HasAgg        0x0000010 /* Contains aggregate functions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_Cloned_Rhs_ਵਿੱਚ   0x0000020 /* Cloned RHS of an IN operator */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_Expanded      0x0000040 /* sqlite3SelectExpand() called on this */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_HasTypeInfo   0x0000080 /* FROM subqueries have Table metadata */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_Compound      0x0000100 /* Part of a compound query */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_ਮੁੱਲਾਂ        0x0000200 /* Synthesized from VALUES clause */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_Multi_ਮੁੱਲ    0x0000400 /* Single VALUES term with multiple rows */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_NestedFrom    0x0000800 /* Part of a parenthesized FROM clause */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_MinMaxAgg     0x0001000 /* Aggregate containing min() or max() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_Recursive     0x0002000 /* The recursive part of a recursive CTE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_Fixed_ਹੱਦਬੰਦੀ    0x0004000 /* nSelectRow set by a constant LIMIT */
/*                       0x0008000 // available for reuse */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_Converted     0x0010000 /* By convertCompoundSelectToSubquery() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_IncludeHidden 0x0020000 /* Include hidden columns in output */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_Complex_ਨਤੀਜਾ 0x0040000 /* Result contains subquery or function */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_WhereBegin    0x0080000 /* Really a WhereBegin() call.  Debug Only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_WinRewrite    0x0100000 /* Window function rewrite accomplished */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_View          0x0200000 /* SELECT statement is a view */
/*                       0x0400000 // available for reuse */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_UFSrcCheck    0x0800000 /* Check pSrc as required by UPDATE...FROM */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_ਧੱਕੋ_ਥੱਲੇ      0x1000000 /* Modified by WHERE-clause push-down opt */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_MultiPart     0x2000000 /* Has multiple incompatible PARTITIONs */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_CopyCte       0x4000000 /* SELECT statement is a copy of a CTE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_ਕ੍ਰਮ_By_Reqd   0x8000000 /* The ORDER BY clause may not be omitted */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_UpdateFrom   0x10000000 /* Query originates with UPDATE FROM */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_Correlated   0x20000000 /* True if references the outer context */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SF_On_ਮੰਜ਼ਿਲ_Where    0x40000000 /* One or more ON clauses moved to WHERE */

/* True if SrcItem X is a subquery that has SF_NestedFrom */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IsNestedFrom(X) \
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRT_Exists       1  /* Store 1 if the result is not empty */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRT_Discard      2  /* Do not save the results anywhere */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRT_DistFifo     3  /* Like SRT_Fifo, but unique results only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRT_Dist_ਕਤਾਰ    4  /* Like SRT_Queue, but unique results only */

/* The DISTINCT clause is ignored for all of the above.  Not that
** IgnorableDistinct() implies IgnorableOrderby() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IgnorableDistinct(X) ((X->eDest)<=SRT_Dist_ਕਤਾਰ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRT_ਕਤਾਰ        5  /* Store result in an queue */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRT_Fifo         6  /* Store result as data with an automatic rowid */

/* The ORDER BY clause is ignored for all of the above */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IgnorableOrderby(X) ((X->eDest)<=SRT_Fifo)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRT_Output       7  /* Output each row of result */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRT_Mem          8  /* Store result in a memory cell */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRT_ਸੈੱਟ          9  /* Store results as keys in an index */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRT_EphemTab    10  /* Create transient tab and store like SRT_Table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRT_Coroutine   11  /* Generate a single row of result */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRT_ਸਾਰਣੀ       12  /* Store result as data with an automatic rowid */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRT_Upfrom      13  /* Store result as data with rowid */

/*
** An instance of this object describes where to put of the results of
** a SELECT statement.
*/
ਬਣਤਰ SelectDest {
  u8 eDest;            /* How to dispose of the results.  One of SRT_* above. */
  ਪੂਰਨ iSDParm;         /* A parameter used by the eDest disposal method */
  ਪੂਰਨ iSDParm2;        /* A second parameter for the eDest disposal method */
  ਪੂਰਨ iSdst;           /* Base register where results are written */
  ਪੂਰਨ nSdst;           /* Number of registers allocated */
  ਅੱਖਰ *zAffSdst;      /* Affinity used for SRT_Set */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By;  /* Key columns for SRT_Queue and SRT_DistQueue */
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
ਬਣਤਰ AutoincInfo {
  AutoincInfo *p_ਅਗਲਾ;   /* Next info block in a list of them all */
  ਸਾਰਣੀ *pTab;          /* Table this info block refers to */
  ਪੂਰਨ iDb;              /* Index in sqlite3.aDb[] of database holding pTab */
  ਪੂਰਨ regCtr;           /* Memory register holding the rowid counter */
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
ਬਣਤਰ TriggerPrg {
  Trigger *pTrigger;      /* Trigger this program was coded from */
  TriggerPrg *p_ਅਗਲਾ;      /* Next entry in Parse.pTriggerPrg list */
  Sub_ਪ੍ਰੋਗਰਾਮ *p_ਪ੍ਰੋਗਰਾਮ;   /* Program implementing pTrigger/orconf */
  ਪੂਰਨ orconf;             /* Default ON CONFLICT policy */
  u32 aColmask[2];        /* Masks of old.*, new.* columns accessed */
};

/*
** The yDbMask datatype for the bitmask of all attached databases.
*/
#ਜੇ SQLITE_MAX_ATTACHED>30
  ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ y_Db_ਨਕਾਬ[(SQLITE_MAX_ATTACHED+9)/8];
# define Db_ਨਕਾਬ_Test(M,I)    (((M)[(I)/8]&(1<<((I)&7)))!=0)
# define Db_ਨਕਾਬ_Zero(M)      memset((M),0,ਆਕਾਰ(M))
# define Db_ਨਕਾਬ_ਸੈੱਟ(M,I)     (M)[(I)/8]|=(1<<((I)&7))
# define Db_ਨਕਾਬ_All_Zero(M)   sqlite3_Db_ਨਕਾਬ_All_Zero(M)
# define Db_ਨਕਾਬ_Non_Zero(M)   (sqlite3_Db_ਨਕਾਬ_All_Zero(M)==0)
#ਨਹੀਂਤਾਂ
  ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ y_Db_ਨਕਾਬ;
# define Db_ਨਕਾਬ_Test(M,I)    (((M)&(((y_Db_ਨਕਾਬ)1)<<(I)))!=0)
# define Db_ਨਕਾਬ_Zero(M)      ((M)=0)
# define Db_ਨਕਾਬ_ਸੈੱਟ(M,I)     ((M)|=(((y_Db_ਨਕਾਬ)1)<<(I)))
# define Db_ਨਕਾਬ_All_Zero(M)   ((M)==0)
# define Db_ਨਕਾਬ_Non_Zero(M)   ((M)!=0)
#ਜੇ_ਅੰਤ

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
ਬਣਤਰ ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ {
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ;            /* The expression contained in the index */
  ਪੂਰਨ i_ਅੰਕੜੇ_Cur;           /* The data cursor associated with the index */
  ਪੂਰਨ iIdxCur;            /* The index cursor */
  ਪੂਰਨ i_Idx_ਖਾਨਾ;            /* The index column that contains value of pExpr */
  u8 b_Maybe_Null_ਪੰਕਤੀ;       /* True if we need an OP_IfNullRow check */
  u8 aff;                 /* Affinity of the pExpr expression */
  ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ *pIENext;   /* Next in a list of all indexed expressions */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_EXPLAIN_COMMENTS
  ਅਚਲ ਅੱਖਰ *zIdxName;   /* Name of index, used only for bytecode comments */
#ਜੇ_ਅੰਤ
};

/*
** An instance of the ParseCleanup object specifies an operation that
** should be performed after parsing to deallocation resources obtained
** during the parse and which are no longer needed.
*/
ਬਣਤਰ ParseCleanup {
  ParseCleanup *p_ਅਗਲਾ;               /* Next cleanup task */
  ਖਾਲੀ *pPtr;                        /* Pointer to object to deallocate */
  ਖਾਲੀ (*xCleanup)(sqlite3*,ਖਾਲੀ*);  /* Deallocation routine */
};

/*
** Number of 64-bit entried in the variable number bitmap cache (VNBMC).
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_VNBMC
# define SQLITE_VNBMC 2
#ਜੇ_ਅੰਤ

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
ਬਣਤਰ Parse {
  sqlite3 *db;         /* The main database structure */
  ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ;       /* An error message */
  Vdbe *pVdbe;         /* An engine for executing database bytecode */
  ਪੂਰਨ rc;              /* Return code from execution */
  LogEst nQueryLoop;   /* Est number of iterations of a query (10*log2(N)) */
  u8 nested;           /* Number of nested calls to the parser/code generator */
  u8 n_ਆਰਜ਼ੀ_Reg;         /* Number of temporary registers in aTempReg[] */
  u8 isMultiWrite;     /* True if statement may modify/insert multiple rows */
  u8 disableLookaside; /* Number of times lookaside has been disabled */
  u8 prepFlags;        /* SQLITE_PREPARE_* flags */
  u8 withinRJSubrtn;   /* Nesting level for RIGHT JOIN body subroutines */
  u8 mSubrtnSig;       /* mini Bloom filter on available SubrtnSig.selId */
  u8 eTriggerOp;       /* TK_UPDATE, TK_INSERT or TK_DELETE */
  u8 eOrconf;          /* Default ON CONFLICT policy for trigger steps */
#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
  u8 earlyCleanup;     /* OOM inside sqlite3ParserAddCleanup() */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  u8 ifNotExists;      /* Might be true if IF NOT EXISTS.  Assert()s only */
  u8 isCreate;         /* CREATE TABLE, INDEX, or VIEW (but not TRIGGER)
                       ** and ALTER TABLE ADD COLUMN. */
#ਜੇ_ਅੰਤ
  bft disableTriggers:1; /* True to disable triggers */
  bft mayAbort :1;     /* True if statement may throw an ABORT exception */
  bft hasCompound :1;  /* Need to invoke convertCompoundSelectToSubquery() */
  bft bReturning :1;   /* Coding a RETURNING trigger */
  bft bHasExists :1;   /* Has a correlated "EXISTS (SELECT ....)" expression */
  bft ਖਾਨਾ_ਨਾਂ_ਸੈੱਟ :1;  /* TRUE after OP_ColumnName has been issued to pVdbe */
  bft bHasWith :1;     /* True if statement contains WITH */
  bft ok_Const_ਗੁਣਕ:1; /* OK to factor out constants */
  bft checkSchema :1;  /* Causes schema cookie check after an error */
  bft ਵਰਤਦਾ_Ainc :1;     /* True if pAinc is valid */
  ਪੂਰਨ nRangeReg;       /* Size of the temporary register block */
  ਪੂਰਨ iRangeReg;       /* First register in temporary register block */
  ਪੂਰਨ nErr;            /* Number of errors seen */
  ਪੂਰਨ nTab;            /* Number of previously allocated VDBE cursors */
  ਪੂਰਨ nMem;            /* Number of memory cells used so far */
  ਪੂਰਨ szOpAlloc;       /* Bytes of memory space allocated for Vdbe.aOp[] */
  ਪੂਰਨ iSelfTab;        /* Table associated with an index on expr, or negative
                       ** of the base register during check-constraint eval */
  ਪੂਰਨ nNestSel;        /* Number of nested SELECT statements and/or VIEWs */
  ਪੂਰਨ nLabel;          /* The *negative* of the number of labels used */
  ਪੂਰਨ nLabelAlloc;     /* Number of slots in aLabel */
  ਪੂਰਨ *aLabel;         /* Space to hold the labels */
  ਵਿਅੰਜਕ_List *p_Const_ਵਿਅੰਜਕ;/* Constant expressions */
  ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ *pIdxEpr;/* List of expressions used by active indexes */
  ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ *p_Idx_Part_ਵਿਅੰਜਕ; /* Exprs constrained by index WHERE clauses */
  y_Db_ਨਕਾਬ write_ਨਕਾਬ;   /* Start a write transaction on these databases */
  y_Db_ਨਕਾਬ cookie_ਨਕਾਬ;  /* Bitmask of schema verified databases */
  ਪੂਰਨ nMaxArg;         /* Max args to xUpdate and xFilter vtab methods */
  ਪੂਰਨ nSelect;         /* Number of SELECT stmts. Counter for Select.selId */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਤਰੱਕੀ_CALLBACK
  u32 n_ਤਰੱਕੀ_ਕਦਮਾਂ;  /* xProgress steps taken during sqlite3_prepare() */
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
  ਪੂਰਨ n_ਸਾਰਣੀ_ਤਾਲਾ;        /* Number of locks in aTableLock */
#ਜੇ_ਅੰਤ
  Parse *pToplevel;    /* Parse structure for main program (or NULL) */
  ਸਾਰਣੀ *pTriggerTab;  /* Table triggers are being coded for */
  TriggerPrg *pTriggerPrg;  /* Linked list of coded triggers */
  ParseCleanup *pCleanup;   /* List of cleanup operations to run after parse */
  Trigger *pTrigCoded;      /* Trigger whose sub-program is being coded */

  /**************************************************************************
  ** Fields above must be initialized to zero.  The fields that follow,
  ** down to the beginning of the recursive section, do not need to be
  ** initialized as they will be set before being used.  The boundary is
  ** determined by offsetof(Parse,aTempReg).
  **************************************************************************/

  ਪੂਰਨ a_ਆਰਜ਼ੀ_Reg[8];        /* Holding area for temporary registers */
  Parse *pOuterParse;     /* Outer Parse object when nested */
  Token sNameToken;       /* Token with unqualified schema object name */
  u32 oldmask;            /* Mask of old.* columns referenced */
  u32 newmask;            /* Mask of new.* columns referenced */
  ਸੰਘ {
    ਬਣਤਰ {  /* These fields available when isCreate is true */
      ਪੂਰਨ addrCrTab;        /* Address of OP_CreateBtree on CREATE TABLE */
      ਪੂਰਨ regRowid;         /* Register holding rowid of CREATE TABLE entry */
      ਪੂਰਨ reg_ਜੜ੍ਹ;          /* Register holding root page for new objects */
      Token constraintName; /* Name of the constraint currently being parsed */
    } cr;
    ਬਣਤਰ {  /* These fields available to all other statements */
      Returning *pReturning; /* The RETURNING clause */
    } d;
  } u1;
  AutoincInfo *pAinc;     /* Information about AUTOINCREMENT counters. Only
                          ** valid if Parse.usesAinc is true */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
  ਸਾਰਣੀ_ਤਾਲਾ *a_ਸਾਰਣੀ_ਤਾਲਾ;  /* Required table locks for shared-cache mode. Only
                          ** valid if Parse.nTableLock>0 */
#ਜੇ_ਅੰਤ


  /************************************************************************
  ** Above is constant between recursions.  Below is reset before and after
  ** each recursion.  The boundary between these two regions is determined
  ** using offsetof(Parse,sLastToken) so the sLastToken field must be the
  ** first field in the recursive region.
  ************************************************************************/

  Token s_ਆਖਰੀ_Token;       /* The last token parsed */
  ynVar nVar;               /* Number of '?' variables seen in the SQL so far */
  u64 aVnbmc[SQLITE_VNBMC]; /* Varible Number Bitmap Cache */
  u8 i_Pk_ਛਾਂਟੋ_ਕ੍ਰਮ;          /* ASC or DESC for INTEGER PRIMARY KEY */
  u8 explain;               /* True if the EXPLAIN flag is found on the query */
  u8 e_Parse_ਬਹੁਲਕ;            /* PARSE_MODE_XXX constant */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਪੂਰਨ n_Vtab_ਤਾਲਾ;            /* Number of virtual tables to lock */
#ਜੇ_ਅੰਤ
  ਪੂਰਨ n_ਉਚਾਈ;              /* Expression tree height of current sub-select */
  ਪੂਰਨ addrExplain;          /* Address of current OP_Explain opcode */
  VList *pVList;            /* Mapping between variable names and numbers */
  Vdbe *pReprepare;         /* VM being reprepared (sqlite3Reprepare()) */
  ਅਚਲ ਅੱਖਰ *z_ਪੂਛ;        /* All SQL text past the last semicolon parsed */
  ਸਾਰਣੀ *p_New_ਸਾਰਣੀ;         /* A table being constructed by CREATE TABLE */
  ਸੂਚਕਾਂਕ *p_New_ਸੂਚਕਾਂਕ;         /* An index being constructed by CREATE INDEX.
                            ** Also used to hold redundant UNIQUE constraints
                            ** during a RENAME COLUMN */
  Trigger *pNewTrigger;     /* Trigger under construct by a CREATE TRIGGER */
  ਅਚਲ ਅੱਖਰ *zAuthContext; /* The 6th parameter to db->xAuth callbacks */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  Token sArg;               /* Complete text of a module argument */
  ਸਾਰਣੀ **ap_Vtab_ਤਾਲਾ;       /* Pointer to virtual tables needing locking */
#ਜੇ_ਅੰਤ
  With *pWith;              /* Current WITH clause, or NULL */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ALTERTABLE
  RenameToken *pRename;     /* Tokens subject to renaming by ALTER TABLE */
#ਜੇ_ਅੰਤ
};

/* Allowed values for Parse.eParseMode
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PARSE_ਬਹੁਲਕ_ਸਾਧਾਰਨ        0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PARSE_ਬਹੁਲਕ_DECLARE_VTAB  1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PARSE_ਬਹੁਲਕ_RENAME        2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PARSE_ਬਹੁਲਕ_UNMAP         3

/*
** Sizes and pointers of various parts of the Parse object.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PARSE_HDR(X)  (((ਅੱਖਰ*)(X))+offsetof(Parse,z_Err_ਸੁਨੇਹਾ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PARSE_HDR_SZ (offsetof(Parse,a_ਆਰਜ਼ੀ_Reg)-offsetof(Parse,z_Err_ਸੁਨੇਹਾ)) /* Recursive part w/o aColCache*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PARSE_RECURSE_SZ offsetof(Parse,s_ਆਖਰੀ_Token)    /* Recursive part */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PARSE_ਪੂਛ_SZ (ਆਕਾਰ(Parse)-PARSE_RECURSE_SZ) /* Non-recursive part */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PARSE_ਪੂਛ(X) (((ਅੱਖਰ*)(X))+PARSE_RECURSE_SZ)  /* Pointer to tail */

/*
** Return true if currently inside an sqlite3_declare_vtab() call.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_DECLARE_VTAB 0
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_DECLARE_VTAB (pParse->e_Parse_ਬਹੁਲਕ==PARSE_ਬਹੁਲਕ_DECLARE_VTAB)
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_OMIT_ALTERTABLE)
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_RENAME_OBJECT 0
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_RENAME_OBJECT (pParse->e_Parse_ਬਹੁਲਕ>=PARSE_ਬਹੁਲਕ_RENAME)
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_OMIT_ALTERTABLE)
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_SPECIAL_PARSE 0
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_SPECIAL_PARSE (pParse->e_Parse_ਬਹੁਲਕ!=PARSE_ਬਹੁਲਕ_ਸਾਧਾਰਨ)
#ਜੇ_ਅੰਤ

/*
** An instance of the following structure can be declared on a stack and used
** to save the Parse.zAuthContext value so that it can be restored later.
*/
ਬਣਤਰ AuthContext {
  ਅਚਲ ਅੱਖਰ *zAuthContext;   /* Put saved Parse.zAuthContext here */
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_NCHANGE       0x01    /* OP_Insert: Set to update db->nChange */
                                     /* Also used in P2 (not P5) of OP_Delete */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_NOCHNG        0x01    /* OP_VColumn nochange for UPDATE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_EPHEM         0x01    /* OP_Column: Ephemeral output is ok */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_LASTROWID     0x20    /* Set to update db->lastRowid */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_ISUPDATE      0x04    /* This OP_Insert is an sql UPDATE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_APPEND        0x08    /* This is likely to be an append */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_USESEEKRESULT 0x10    /* Try to avoid a seek in BtreeInsert() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_ISNOOP        0x40    /* OP_Delete does pre-update-hook only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_LENGTHARG     0x40    /* OP_Column only used for length() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_TYPEOFARG     0x80    /* OP_Column only used for typeof() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_BYTELENARG    0xc0    /* OP_Column only for octet_length() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_BULKCSR       0x01    /* OP_Open** used to open bulk cursor */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_SEEKEQ        0x02    /* OP_Open** cursor uses EQ seek only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_FORDELETE     0x08    /* OP_Open should use BTREE_FORDELETE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_P2ISREG       0x10    /* P2 to OP_Open** is a register number */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_ਕ੍ਰਮ_ਬਦਲੋ       0x01    /* OP_Compare: use the permutation */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_SAVEPOSITION  0x02    /* OP_Delete/Insert: save cursor pos */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_AUXDELETE     0x04    /* OP_Delete: index in a DELETE op */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_NOCHNG_MAGIC  0x6d    /* OP_MakeRecord: serialtype 10 is ok */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ OPFLAG_PREFORMAT     0x80    /* OP_Insert uses preformatted cell */

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
ਬਣਤਰ Trigger {
  ਅੱਖਰ *zName;            /* The name of the trigger                        */
  ਅੱਖਰ *ਸਾਰਣੀ_2;            /* The table or view to which the trigger applies */
  u8 op;                  /* One of TK_DELETE, TK_UPDATE, TK_INSERT         */
  u8 tr_tm;               /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  u8 bReturning;          /* This trigger implements a RETURNING clause */
  ਵਿਅੰਜਕ *pWhen;            /* The WHEN clause of the expression (may be NULL) */
  IdList *pColumns;       /* If this is an UPDATE OF <column-list> trigger,
                             the <column-list> is stored here */
  Schema *pSchema;        /* Schema containing the trigger */
  Schema *pTabSchema;     /* Schema containing the table */
  Trigger_ਕਦਮ *ਕਦਮ_list; /* Link list of trigger program steps             */
  Trigger *p_ਅਗਲਾ;         /* Next trigger associated with the table */
};

/*
** A trigger is either a BEFORE or an AFTER trigger.  The following constants
** determine which.
**
** If there are multiple triggers, you might of some BEFORE and some AFTER.
** In that cases, the constants below can be ORed together.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRIGGER_BEFORE  1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TRIGGER_AFTER   2

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
ਬਣਤਰ Trigger_ਕਦਮ {
  u8 op;               /* One of TK_DELETE, TK_UPDATE, TK_INSERT, TK_SELECT,
                       ** or TK_RETURNING */
  u8 orconf;           /* OE_Rollback etc. */
  Trigger *pTrig;      /* The trigger that this step is a part of */
  Select *pSelect;     /* SELECT statement or RHS of INSERT INTO SELECT ... */
  SrcList *pSrc;       /* Table to insert/update/delete */
  ਵਿਅੰਜਕ *pWhere;        /* The WHERE clause for DELETE or UPDATE steps */
  ਵਿਅੰਜਕ_List *p_ਵਿਅੰਜਕ_List; /* SET clause for UPDATE, or RETURNING clause */
  IdList *pIdList;     /* Column names for INSERT */
  Upsert *pUpsert;     /* Upsert clauses on an INSERT */
  ਅੱਖਰ *z_ਵਿਸਤਾਰ;         /* Original SQL text of this command */
  Trigger_ਕਦਮ *p_ਅਗਲਾ;  /* Next in the link-list */
  Trigger_ਕਦਮ *p_ਆਖਰੀ;  /* Last element in link-list. Valid for 1st elem only */
};

/*
** Information about a RETURNING clause
*/
ਬਣਤਰ Returning {
  Parse *pParse;        /* The parse that includes the RETURNING clause */
  ਵਿਅੰਜਕ_List *pReturnEL;  /* List of expressions to return */
  Trigger retTrig;      /* The transient trigger that implements RETURNING */
  Trigger_ਕਦਮ retTStep; /* The trigger step */
  ਪੂਰਨ iRetCur;          /* Transient table holding RETURNING results */
  ਪੂਰਨ n_Ret_ਖਾਨਾ;          /* Number of in pReturnEL after expansion */
  ਪੂਰਨ iRetReg;          /* Register array for holding a row of RETURNING */
  ਅੱਖਰ zName[40];       /* Name of trigger: "sqlite_returning_%p" */
};

/*
** An object used to accumulate the text of a string where we
** do not necessarily know how big the string will be in the end.
*/
ਬਣਤਰ sqlite3_str {
  sqlite3 *db;         /* Optional database for lookaside.  Can be NULL */
  ਅੱਖਰ *z_ਪਾਠ;         /* The string collected so far */
  u32  nAlloc;         /* Amount of space allocated in zText */
  u32  mxAlloc;        /* Maximum allowed allocation.  0 for no malloc usage */
  u32  nChar;          /* Length of the string so far */
  u8   accError;       /* SQLITE_NOMEM or SQLITE_TOOBIG */
  u8   printfFlags;    /* SQLITE_PRINTF flags below */
};
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_PRINTF_ਅੰਦਰੂਨੀ 0x01  /* Internal-use-only converters allowed */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_PRINTF_SQLFUNC  0x02  /* SQL function arguments to VXPrintf */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_PRINTF_MALLOCED 0x04  /* True if zText is allocated space */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isMalloced(X)  (((X)->printfFlags & SQLITE_PRINTF_MALLOCED)!=0)

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
ਬਣਤਰ RCStr {
  u64 nRCRef;            /* Number of references */
  /* Total structure size should be a multiple of 8 bytes for alignment */
};

/*
** A pointer to this structure is used to communicate information
** from sqlite3Init and OP_ParseSchema into the sqlite3InitCallback.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  sqlite3 *db;        /* The database being initialized */
  ਅੱਖਰ **pz_Err_ਸੁਨੇਹਾ;    /* Error message stored here */
  ਪੂਰਨ iDb;            /* 0 for main database.  1 for TEMP, 2.. for ATTACHed */
  ਪੂਰਨ rc;             /* Result code stored here */
  u32 mInitFlags;     /* Flags controlling error messages */
  u32 n_Init_ਪੰਕਤੀ;       /* Number of rows processed */
  Pgno mx_ਪੰਨਾ;        /* Maximum page number.  0 for no limit. */
} Init_ਅੰਕੜੇ;

/*
** Allowed values for mInitFlags
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITFLAG_Alter_ਨਕਾਬ     0x0007  /* Types of ALTER */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITFLAG_AlterRename   0x0001  /* Reparse after a RENAME */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITFLAG_AlterDrop     0x0002  /* Reparse after a DROP COLUMN */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITFLAG_Alter_ਜੋੜੋ      0x0003  /* Reparse after an ADD COLUMN */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INITFLAG_AlterDropCons 0x0004  /* Reparse after a DROP CONSTRAINT */

/* Tuning parameters are set using SQLITE_TESTCTRL_TUNE and are controlled
** on debug-builds of the CLI using ".testctrl tune ID VALUE".  Tuning
** parameters are for temporary use during development, to help find
** optimal values for parameters in the query planner.  The should not
** be used on trunk check-ins.  They are a temporary mechanism available
** for transient development builds only.
**
** Tuning parameters are numbered starting with 1.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_NTUNE  6             /* Should be zero for all trunk check-ins */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
# define Tuning(X)  (sqlite3Config.aTune[(X)-1])
#ਨਹੀਂਤਾਂ
# define Tuning(X)  0
#ਜੇ_ਅੰਤ

/*
** Structure containing global configuration data for the SQLite library.
**
** This structure also contains some state information.
*/
ਬਣਤਰ Sqlite3Config {
  ਪੂਰਨ bMemstat;                     /* True to enable memory status */
  u8 bCoreMutex;                    /* True to enable core mutexing */
  u8 b_ਭਰਿਆ_Mutex;                    /* True to enable full mutexing */
  u8 bOpenUri;                      /* True to interpret filenames as URIs */
  u8 bUseCis;                       /* Use covering indices for full-scans */
  u8 bSmallMalloc;                  /* Avoid large memory allocations if true */
  u8 bExtraSchemaChecks;            /* Verify type,name,tbl_name in schema */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  u8 bJsonSelfcheck;                /* Double-check JSON parsing */
#ਜੇ_ਅੰਤ
  ਪੂਰਨ mxStrlen;                     /* Maximum string length */
  ਪੂਰਨ neverCorrupt;                 /* Database is always well-formed */
  ਪੂਰਨ szLookaside;                  /* Default lookaside buffer size */
  ਪੂਰਨ nLookaside;                   /* Default lookaside buffer count */
  ਪੂਰਨ nStmtSpill;                   /* Stmt-journal spill-to-disk threshold */
  sqlite3_mem_methods m;            /* Low-level memory allocation interface */
  sqlite3_mutex_methods mutex;      /* Low-level mutex interface */
  sqlite3_pcache_methods2 pcache2;  /* Low-level page-cache interface */
  ਖਾਲੀ *p_ਢੇਰ;                      /* Heap storage space */
  ਪੂਰਨ n_ਢੇਰ;                        /* Size of pHeap[] */
  ਪੂਰਨ mnReq, mxReq;                 /* Min and max heap requests sizes */
  sqlite3_int64 szMmap;             /* mmap() space per open file */
  sqlite3_int64 mxMmap;             /* Maximum value for szMmap */
  ਖਾਲੀ *p_ਪੰਨਾ;                      /* Page cache memory */
  ਪੂਰਨ sz_ਪੰਨਾ;                       /* Size of each page in pPage[] */
  ਪੂਰਨ n_ਪੰਨਾ;                        /* Number of pages in pPage[] */
  ਪੂਰਨ mx_Parser_ਢੇਰ;                /* maximum depth of the parser stack */
  ਪੂਰਨ sharedCacheEnabled;           /* true if shared-cache mode enabled */
  u32 szPma;                        /* Maximum Sorter PMA size */
  /* The above might be initialized to non-zero.  The following need to always
  ** initially be zero, however. */
  ਪੂਰਨ isInit;                       /* True after initialization has finished */
  ਪੂਰਨ ਵਿੱਚ_ਤਰੱਕੀ;                   /* True while initialization in progress */
  ਪੂਰਨ isMutexInit;                  /* True after mutexes are initialized */
  ਪੂਰਨ isMallocInit;                 /* True after malloc is initialized */
  ਪੂਰਨ isPCacheInit;                 /* True after malloc is initialized */
  ਪੂਰਨ nRefInitMutex;                /* Number of users of pInitMutex */
  sqlite3_mutex *pInitMutex;        /* Mutex used by sqlite3_initialize() */
  ਖਾਲੀ (*xLog)(ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*); /* Function for logging */
  ਖਾਲੀ *pLogArg;                       /* First argument to xLog() */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SQLLOG
  ਖਾਲੀ(*xSqllog)(ਖਾਲੀ*,sqlite3*,ਅਚਲ ਅੱਖਰ*, ਪੂਰਨ);
  ਖਾਲੀ *pSqllogArg;
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_VDBE_COVERAGE
  /* The following callback (if not NULL) is invoked on every VDBE branch
  ** operation.  Set the callback using SQLITE_TESTCTRL_VDBE_COVERAGE.
  */
  ਖਾਲੀ (*xVdbeBranch)(ਖਾਲੀ*,ਬਿਨਾਨਿਸ਼ਾਨੀ i_Src_ਸਤਰ,u8 eThis,u8 eMx);  /* Callback */
  ਖਾਲੀ *pVdbeBranchArg;                                     /* 1st argument */
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DESERIALIZE
  sqlite3_int64 mx_Memdb_ਮਾਪ;        /* Default max memdb size */
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_UNTESTABLE
  ਪੂਰਨ (*xTestCallback)(ਪੂਰਨ);        /* Invoked by sqlite3FaultSim() */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ALLOW_ROWID_ਵਿੱਚ_VIEW
  u32 mNoVisibleRowid;              /* TF_NoVisibleRowid if the ROWID_IN_VIEW
                                    ** feature is disabled.  0 if rowids can
                                    ** occur in views. */
#ਜੇ_ਅੰਤ
  ਪੂਰਨ bLocaltimeFault;              /* True to fail localtime() calls */
  ਪੂਰਨ (*xAltLocaltime)(ਅਚਲ ਖਾਲੀ*,ਖਾਲੀ*); /* Alternative localtime() routine */
  ਪੂਰਨ i_Once_Reset_ਹੱਦ;          /* When to reset OP_Once counters */
  u32 szSorterRef;                  /* Min size in bytes to use sorter-refs */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i_Prng_ਬੀਜ;           /* Alternative fixed seed for the PRNG */
  /* vvvv--- must be last ---vvv */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  sqlite3_int64 aTune[SQLITE_NTUNE]; /* Tuning parameters */
#ਜੇ_ਅੰਤ
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CORRUPT_DB  (sqlite3Config.neverCorrupt==0)

/*
** Context pointer passed down through the tree-walk.
*/
ਬਣਤਰ Walker {
  Parse *pParse;                            /* Parser context.  */
  ਪੂਰਨ (*x_ਵਿਅੰਜਕ_Callback)(Walker*, ਵਿਅੰਜਕ*);     /* Callback for expressions */
  ਪੂਰਨ (*xSelectCallback)(Walker*,Select*);  /* Callback for SELECTs */
  ਖਾਲੀ (*xSelectCallback2)(Walker*,Select*);/* Second callback for SELECTs */
  ਪੂਰਨ walker_ਡੂੰਘਾਈ;                          /* Number of subqueries */
  u16 e_ਕੋਡ;                                /* A small processing code */
  u16 mWFlags;                              /* Use-dependent flags */
  ਸੰਘ {                                   /* Extra data for callback */
    NameContext *pNC;                         /* Naming context */
    ਪੂਰਨ n;                                    /* A counter */
    ਪੂਰਨ iCur;                                 /* A cursor number */
    ਪੂਰਨ sz;                                   /* String literal length */
    SrcList *pSrcList;                        /* FROM clause */
    ਬਣਤਰ CCurHint *pCCurHint;               /* Used by codeCursorHint() */
    ਬਣਤਰ RefSrcList *pRefSrcList;           /* sqlite3ReferencesSrcList() */
    ਪੂਰਨ *ai_ਖਾਨਾ;                               /* array of column indexes */
    ਬਣਤਰ IdxCover *pIdxCover;               /* Check for index coverage */
    ਵਿਅੰਜਕ_List *p_ਸਮੂਹ_By;                       /* GROUP BY clause */
    Select *pSelect;                          /* HAVING to WHERE clause ctx */
    ਬਣਤਰ ਖਿੜਕੀ_Rewrite *pRewrite;           /* Window rewrite context */
    ਬਣਤਰ WhereConst *pConst;                /* WHERE clause constants */
    ਬਣਤਰ RenameCtx *pRename;                /* RENAME COLUMN context */
    ਬਣਤਰ ਸਾਰਣੀ *pTab;                       /* Table of generated column */
    ਬਣਤਰ Covering_ਸੂਚਕਾਂਕ_Check *pCovIdxCk;     /* Check for covering index */
    Src_ਵਸਤੂ *p_Src_ਵਸਤੂ;                        /* A single FROM clause item */
    DbFixer *pFix;                            /* See sqlite3FixSelect() */
    Mem *aMem;                                /* See sqlite3BtreeCursorHint() */
    ਬਣਤਰ CheckOnCtx *pCheckOnCtx;           /* See selectCheckOnClauses() */
  } u;
};

/*
** The following structure contains information used by the sqliteFix...
** routines as they walk the parse tree to make database references
** explicit.
*/
ਬਣਤਰ DbFixer {
  Parse *pParse;      /* The parsing context.  Error messages written here */
  Walker w;           /* Walker object */
  Schema *pSchema;    /* Fix items to this schema */
  u8 b_ਆਰਜ਼ੀ;           /* True for TEMP schema entries */
  ਅਚਲ ਅੱਖਰ *zDb;    /* Make sure all objects are contained in this database */
  ਅਚਲ ਅੱਖਰ *zType;  /* Type of the container - used for error messages */
  ਅਚਲ Token *pName; /* Name of the container - used for error messages */
};

/* Forward declarations */
ਪੂਰਨ sqlite3_Walk_ਵਿਅੰਜਕ(Walker*, ਵਿਅੰਜਕ*);
ਪੂਰਨ sqlite3_Walk_ਵਿਅੰਜਕ_NN(Walker*, ਵਿਅੰਜਕ*);
ਪੂਰਨ sqlite3_Walk_ਵਿਅੰਜਕ_List(Walker*, ਵਿਅੰਜਕ_List*);
ਪੂਰਨ sqlite3WalkSelect(Walker*, Select*);
ਪੂਰਨ sqlite3_Walk_Select_ਵਿਅੰਜਕ(Walker*, Select*);
ਪੂਰਨ sqlite3WalkSelectFrom(Walker*, Select*);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Walk_Noop(Walker*, ਵਿਅੰਜਕ*);
ਪੂਰਨ sqlite3SelectWalkNoop(Walker*, Select*);
ਪੂਰਨ sqlite3SelectWalkFail(Walker*, Select*);
ਪੂਰਨ sqlite3_Walker_ਡੂੰਘਾਈ_Increase(Walker*,Select*);
ਖਾਲੀ sqlite3_Walker_ਡੂੰਘਾਈ_Decrease(Walker*,Select*);
ਖਾਲੀ sqlite3WalkWinDefnDummyCallback(Walker*,Select*);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਖਾਲੀ sqlite3SelectWalkAssert2(Walker*, Select*);
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CTE
ਖਾਲੀ sqlite3_Select_ਕੱਢੋ_With(Walker*, Select*);
#ਨਹੀਂਤਾਂ
# define sqlite3_Select_ਕੱਢੋ_With 0
#ਜੇ_ਅੰਤ

/*
** Return code from the parse-tree walking primitives and their
** callbacks.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRC_Continue    0   /* Continue down into children */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRC_Prune       1   /* Omit children but continue walking siblings */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WRC_Abort       2   /* Abandon the tree walk */

/*
** A single common table expression
*/
ਬਣਤਰ Cte {
  ਅੱਖਰ *zName;            /* Name of this CTE */
  ਵਿਅੰਜਕ_List *p_ਖਾਨੇ_ਗਿਣਤੀ;        /* List of explicit column names, or NULL */
  Select *pSelect;        /* The definition of this CTE */
  ਅਚਲ ਅੱਖਰ *zCteErr;    /* Error message for circular references */
  CteUse *pUse;           /* Usage information for this CTE */
  u8 eM10d;               /* The MATERIALIZED flag */
};

/*
** Allowed values for the materialized flag (eM10d):
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M10d_Yes       0  /* AS MATERIALIZED */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M10d_Any       1  /* Not specified.  Query planner's choice */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ M10d_No        2  /* AS NOT MATERIALIZED */

/*
** An instance of the With object represents a WITH clause containing
** one or more CTEs (common table expressions).
*/
ਬਣਤਰ With {
  ਪੂਰਨ nCte;               /* Number of CTEs in the WITH clause */
  ਪੂਰਨ bView;              /* Belongs to the outermost Select of a view */
  With *pOuter;           /* Containing WITH clause, or NULL */
  Cte a[FLEXARRAY];       /* For each CTE in the WITH clause.... */
};

/* The size (in bytes) of a With object that can hold as many
** as N different CTEs. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SZ_WITH(N)  (offsetof(With,a) + (N)*ਆਕਾਰ(Cte))

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
ਬਣਤਰ CteUse {
  ਪੂਰਨ nUse;              /* Number of users of this CTE */
  ਪੂਰਨ addrM9e;           /* Start of subroutine to compute materialization */
  ਪੂਰਨ regRtn;            /* Return address register for addrM9e subroutine */
  ਪੂਰਨ iCur;              /* Ephemeral table holding the materialization */
  LogEst n_ਪੰਕਤੀ_Est;        /* Estimated number of rows in the table */
  u8 eM10d;              /* The MATERIALIZED flag */
};


/* Client data associated with sqlite3_set_clientdata() and
** sqlite3_get_clientdata().
*/
ਬਣਤਰ Db_Client_ਅੰਕੜੇ {
  Db_Client_ਅੰਕੜੇ *p_ਅਗਲਾ;        /* Next in a linked list */
  ਖਾਲੀ *p_ਅੰਕੜੇ;                /* The data */
  ਖਾਲੀ (*xDestructor)(ਖਾਲੀ*); /* Destructor.  Might be NULL */
  ਅੱਖਰ zName[FLEXARRAY];      /* Name of this client data. MUST BE LAST */
};

/* The size (in bytes) of a DbClientData object that can has a name
** that is N bytes long, including the zero-terminator. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SZ_DBCLIENTDATA(N) (offsetof(Db_Client_ਅੰਕੜੇ,zName)+(N))

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** An instance of the TreeView object is used for printing the content of
** data structures on sqlite3DebugPrintf() using a tree-like view.
*/
ਬਣਤਰ ਰੁੱਖ_View {
  ਪੂਰਨ iLevel;             /* Which level of the tree we are on */
  u8  b_ਸਤਰ[100];         /* Draw vertical in column i if bLine[i] is true */
};
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */

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
ਬਣਤਰ ਖਿੜਕੀ {
  ਅੱਖਰ *zName;            /* Name of window (may be NULL) */
  ਅੱਖਰ *z_ਅਧਾਰ;            /* Name of base window for chaining (may be NULL) */
  ਵਿਅੰਜਕ_List *p_ਹਿੱਸੇ_ਕਰੋ;   /* PARTITION BY clause */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By;     /* ORDER BY clause */
  u8 eFrmType;            /* TK_RANGE, TK_GROUPS, TK_ROWS, or 0 */
  u8 e_ਸ਼ੁਰੂ;              /* UNBOUNDED, CURRENT, PRECEDING or FOLLOWING */
  u8 e_ਸਿਰਾ;                /* UNBOUNDED, CURRENT, PRECEDING or FOLLOWING */
  u8 b_Implicit_ਚੌਖਟਾ;      /* True if frame was implicitly specified */
  u8 eExclude;            /* TK_NO, TK_CURRENT, TK_TIES, TK_GROUP, or 0 */
  ਵਿਅੰਜਕ *p_ਸ਼ੁਰੂ;           /* Expression for "<expr> PRECEDING" */
  ਵਿਅੰਜਕ *p_ਸਿਰਾ;             /* Expression for "<expr> FOLLOWING" */
  ਖਿੜਕੀ **ppThis;        /* Pointer to this object in Select.pWin list */
  ਖਿੜਕੀ *p_ਅਗਲਾ_Win;       /* Next window function belonging to this SELECT */
  ਵਿਅੰਜਕ *pFilter;          /* The FILTER expression */
  FuncDef *pWFunc;        /* The function */
  ਪੂਰਨ iEphCsr;            /* Partition buffer or Peer buffer */
  ਪੂਰਨ regAccum;           /*ਸੰਚਾਇਕ*/
  ਪੂਰਨ reg_ਨਤੀਜਾ;          /* Interim result */
  ਪੂਰਨ csrApp;             /* Function cursor (used by min/max) */
  ਪੂਰਨ regApp;             /* Function register (also used by min/max) */
  ਪੂਰਨ regPart;            /* Array of registers for PARTITION BY values */
  ਵਿਅੰਜਕ *pOwner;           /* Expression object this window is attached to */
  ਪੂਰਨ n_ਬਫਰ_ਖਾਨਾ;         /* Number of columns in buffer table */
  ਪੂਰਨ i_Arg_ਖਾਨਾ;            /* Offset of first argument for this function */
  ਪੂਰਨ regOne;             /* Register containing constant value 1 */
  ਪੂਰਨ reg_ਸ਼ੁਰੂ_Rowid;
  ਪੂਰਨ reg_ਸਿਰਾ_Rowid;
  u8 b_ਵਿਅੰਜਕ_ਦਲੀਲਾਂ;           /* Defer evaluation of window function arguments
                          ** due to the SQLITE_SUBTYPE flag */
};

Select *sqlite3_Multi_ਮੁੱਲਾਂ(Parse *pParse, Select *p_ਖੱਬਾ, ਵਿਅੰਜਕ_List *p_ਪੰਕਤੀ);
ਖਾਲੀ sqlite3_Multi_ਮੁੱਲਾਂ_ਸਿਰਾ(Parse *pParse, Select *pVal);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
ਖਾਲੀ sqlite3_ਖਿੜਕੀ_Delete(sqlite3*, ਖਿੜਕੀ*);
ਖਾਲੀ sqlite3_ਖਿੜਕੀ_Unlink_From_Select(ਖਿੜਕੀ*);
ਖਾਲੀ sqlite3_ਖਿੜਕੀ_List_Delete(sqlite3 *db, ਖਿੜਕੀ *p);
ਖਿੜਕੀ *sqlite3_ਖਿੜਕੀ_Alloc(Parse*, ਪੂਰਨ, ਪੂਰਨ, ਵਿਅੰਜਕ*, ਪੂਰਨ , ਵਿਅੰਜਕ*, u8);
ਖਾਲੀ sqlite3_ਖਿੜਕੀ_Attach(Parse*, ਵਿਅੰਜਕ*, ਖਿੜਕੀ*);
ਖਾਲੀ sqlite3_ਖਿੜਕੀ_Link(Select *pSel, ਖਿੜਕੀ *pWin);
ਪੂਰਨ sqlite3_ਖਿੜਕੀ_Compare(ਅਚਲ Parse*, ਅਚਲ ਖਿੜਕੀ*, ਅਚਲ ਖਿੜਕੀ*, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਖਿੜਕੀ_ਕੋਡ_Init(Parse*, Select*);
ਖਾਲੀ sqlite3_ਖਿੜਕੀ_ਕੋਡ_ਕਦਮ(Parse*, Select*, WhereInfo*, ਪੂਰਨ, ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਖਿੜਕੀ_Rewrite(Parse*, Select*);
ਖਾਲੀ sqlite3_ਖਿੜਕੀ_Update(Parse*, ਖਿੜਕੀ*, ਖਿੜਕੀ*, FuncDef*);
ਖਿੜਕੀ *sqlite3_ਖਿੜਕੀ_Dup(sqlite3 *db, ਵਿਅੰਜਕ *pOwner, ਖਿੜਕੀ *p);
ਖਿੜਕੀ *sqlite3_ਖਿੜਕੀ_List_Dup(sqlite3 *db, ਖਿੜਕੀ *p);
ਖਾਲੀ sqlite3_ਖਿੜਕੀ_Functions(ਖਾਲੀ);
ਖਾਲੀ sqlite3_ਖਿੜਕੀ_Chain(Parse*, ਖਿੜਕੀ*, ਖਿੜਕੀ*);
ਖਿੜਕੀ *sqlite3_ਖਿੜਕੀ_Assemble(Parse*, ਖਿੜਕੀ*, ਵਿਅੰਜਕ_List*, ਵਿਅੰਜਕ_List*, Token*);
#ਨਹੀਂਤਾਂ
# define sqlite3_ਖਿੜਕੀ_Delete(a,b)
# define sqlite3_ਖਿੜਕੀ_Functions()
# define sqlite3_ਖਿੜਕੀ_Attach(a,b,c)
#ਜੇ_ਅੰਤ

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_SKIP_UTF8(z_ਵਿੱਚ) {                        \
  ਜੇ( (*(z_ਵਿੱਚ++))>=0xc0 ){                              \
    ਜਦੋਂ( (*z_ਵਿੱਚ & 0xc0)==0x80 ){ z_ਵਿੱਚ++; }             \
  }                                                    \
}

/*
** The SQLITE_*_BKPT macros are substitutes for the error codes with
** the same name but without the _BKPT suffix.  These macros invoke
** routines that report the line-number on which the error originated
** using sqlite3_log().  The routines also provide a convenient place
** to set a debugger breakpoint.
*/
ਪੂਰਨ sqlite3ReportError(ਪੂਰਨ iErr, ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *zType);
ਪੂਰਨ sqlite3CorruptError(ਪੂਰਨ);
ਪੂਰਨ sqlite3MisuseError(ਪੂਰਨ);
ਪੂਰਨ sqlite3CantopenError(ਪੂਰਨ);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_CORRUPT_BKPT sqlite3CorruptError(__LINE__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MISUSE_BKPT sqlite3MisuseError(__LINE__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_CANTOPEN_BKPT sqlite3CantopenError(__LINE__)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਪੂਰਨ sqlite3NomemError(ਪੂਰਨ);
  ਪੂਰਨ sqlite3IoerrnomemError(ਪੂਰਨ);
# define SQLITE_NOMEM_BKPT sqlite3NomemError(__LINE__)
# define SQLITE_IOERR_NOMEM_BKPT sqlite3IoerrnomemError(__LINE__)
#ਨਹੀਂਤਾਂ
# define SQLITE_NOMEM_BKPT SQLITE_NOMEM
# define SQLITE_IOERR_NOMEM_BKPT SQLITE_IOERR_NOMEM
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_CORRUPT_PGNO)
  ਪੂਰਨ sqlite3CorruptPgnoError(ਪੂਰਨ,Pgno);
# define SQLITE_CORRUPT_PGNO(P) sqlite3CorruptPgnoError(__LINE__,(P))
#ਨਹੀਂਤਾਂ
# define SQLITE_CORRUPT_PGNO(P) sqlite3CorruptError(__LINE__)
#ਜੇ_ਅੰਤ

/*
** FTS3 and FTS4 both require virtual table support
*/
#ਜੇ defined(SQLITE_OMIT_VIRTUALTABLE)
# undef SQLITE_ENABLE_FTS3
# undef SQLITE_ENABLE_FTS4
#ਜੇ_ਅੰਤ

/*
** FTS4 is really an extension for FTS3.  It is enabled using the
** SQLITE_ENABLE_FTS3 macro.  But to avoid confusion we also call
** the SQLITE_ENABLE_FTS4 macro to serve as an alias for SQLITE_ENABLE_FTS3.
*/
#ਜੇ defined(SQLITE_ENABLE_FTS4) && !defined(SQLITE_ENABLE_FTS3)
# define SQLITE_ENABLE_FTS3 1
#ਜੇ_ਅੰਤ

/*
** The following macros mimic the standard library functions toupper(),
** isspace(), isalnum(), isdigit() and isxdigit(), respectively. The
** sqlite versions only work for ASCII characters, regardless of locale.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ASCII
# define sqlite3Toupper(x)  ((x)&~(sqlite3CtypeMap[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x)]&0x20))
# define sqlite3Isspace(x)   (sqlite3CtypeMap[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x)]&0x01)
# define sqlite3Isalnum(x)   (sqlite3CtypeMap[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x)]&0x06)
# define sqlite3Isalpha(x)   (sqlite3CtypeMap[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x)]&0x02)
# define sqlite3Isdigit(x)   (sqlite3CtypeMap[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x)]&0x04)
# define sqlite3Isxdigit(x)  (sqlite3CtypeMap[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x)]&0x08)
# define sqlite3Tolower(x)   (sqlite3_ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x)])
# define sqlite3Isquote(x)   (sqlite3CtypeMap[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x)]&0x80)
# define sqlite3JsonId1(x)   (sqlite3CtypeMap[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x)]&0x42)
# define sqlite3JsonId2(x)   (sqlite3CtypeMap[(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x)]&0x46)
#ਨਹੀਂਤਾਂ
# define sqlite3Toupper(x)   ਵੱਡਾ_ਅੱਖਰ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x))
# define sqlite3Isspace(x)   ਖਾਲੀ_ਥਾਂ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x))
# define sqlite3Isalnum(x)   ਅੱਖਰ_ਜਾਂ_ਅੰਕ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x))
# define sqlite3Isalpha(x)   ਅੱਖਰ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x))
# define sqlite3Isdigit(x)   ਅੰਕ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x))
# define sqlite3Isxdigit(x)  isxdigit((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x))
# define sqlite3Tolower(x)   tolower((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(x))
# define sqlite3Isquote(x)   ((x)=='"'||(x)=='\''||(x)=='['||(x)=='`')
# define sqlite3JsonId1(x)   (sqlite3IsIdChar(x)&&(x)<'0')
# define sqlite3JsonId2(x)   sqlite3IsIdChar(x)
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3IsIdChar(u8);

/*
** Internal function prototypes
*/
ਪੂਰਨ sqlite3StrICmp(ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*);
ਪੂਰਨ sqlite3Strlen30(ਅਚਲ ਅੱਖਰ*);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3Strlen30NN(C) (ਲੜੀ_ਲੰਬਾਈ(C)&0x3fffffff)
ਅੱਖਰ *sqlite3_ਕਾਲਮ_Type(ਕਾਲਮ*,ਅੱਖਰ*);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3StrNICmp sqlite3_strnicmp

ਪੂਰਨ sqlite3MallocInit(ਖਾਲੀ);
ਖਾਲੀ sqlite3_Malloc_ਸਿਰਾ(ਖਾਲੀ);
ਖਾਲੀ *sqlite3Malloc(u64);
ਖਾਲੀ *sqlite3MallocZero(u64);
ਖਾਲੀ *sqlite3DbMallocZero(sqlite3*, u64);
ਖਾਲੀ *sqlite3DbMallocRaw(sqlite3*, u64);
ਖਾਲੀ *sqlite3DbMallocRawNN(sqlite3*, u64);
ਅੱਖਰ *sqlite3DbStrDup(sqlite3*,ਅਚਲ ਅੱਖਰ*);
ਅੱਖਰ *sqlite3DbStrNDup(sqlite3*,ਅਚਲ ਅੱਖਰ*, u64);
ਅੱਖਰ *sqlite3_Db_ਵਿਸਤਾਰ_Dup(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*);
ਖਾਲੀ *sqlite3Realloc(ਖਾਲੀ*, u64);
ਖਾਲੀ *sqlite3DbReallocOrFree(sqlite3 *, ਖਾਲੀ *, u64);
ਖਾਲੀ *sqlite3DbRealloc(sqlite3 *, ਖਾਲੀ *, u64);
ਖਾਲੀ sqlite3DbFree(sqlite3*, ਖਾਲੀ*);
ਖਾਲੀ sqlite3DbFreeNN(sqlite3*, ਖਾਲੀ*);
ਖਾਲੀ sqlite3DbNNFreeNN(sqlite3*, ਖਾਲੀ*);
ਪੂਰਨ sqlite3_Malloc_ਮਾਪ(ਅਚਲ ਖਾਲੀ*);
ਪੂਰਨ sqlite3_Db_Malloc_ਮਾਪ(sqlite3*, ਅਚਲ ਖਾਲੀ*);
ਖਾਲੀ *sqlite3_ਪੰਨਾ_Malloc(ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਪੰਨਾ_Free(ਖਾਲੀ*);
ਖਾਲੀ sqlite3_Mem_ਸੈੱਟ_Default(ਖਾਲੀ);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_UNTESTABLE
ਖਾਲੀ sqlite3BenignMallocHooks(ਖਾਲੀ (*)(ਖਾਲੀ), ਖਾਲੀ (*)(ਖਾਲੀ));
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3_ਢੇਰ_Nearly_ਭਰਿਆ(ਖਾਲੀ);

/*
** On systems with ample stack space and that support alloca(), make
** use of alloca() to obtain space for large automatic objects.  By default,
** obtain space from malloc().
**
** The alloca() routine never returns NULL.  This will cause code paths
** that deal with sqlite3StackAlloc() failures to be unreachable.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_USE_ALLOCA
# define sqlite3_ਢੇਰ_Alloc_Raw(D,N)   alloca(N)
# define sqlite3_ਢੇਰ_Alloc_Raw_NN(D,N) alloca(N)
# define sqlite3_ਢੇਰ_Free(D,P)
# define sqlite3_ਢੇਰ_Free_NN(D,P)
#ਨਹੀਂਤਾਂ
# define sqlite3_ਢੇਰ_Alloc_Raw(D,N)   sqlite3DbMallocRaw(D,N)
# define sqlite3_ਢੇਰ_Alloc_Raw_NN(D,N) sqlite3DbMallocRawNN(D,N)
# define sqlite3_ਢੇਰ_Free(D,P)       sqlite3DbFree(D,P)
# define sqlite3_ਢੇਰ_Free_NN(D,P)     sqlite3DbFreeNN(D,P)
#ਜੇ_ਅੰਤ

/* Do not allow both MEMSYS5 and MEMSYS3 to be defined together.  If they
** are, disable MEMSYS3
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_MEMSYS5
ਅਚਲ sqlite3_mem_methods *sqlite3MemGetMemsys5(ਖਾਲੀ);
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SQLITE_ENABLE_MEMSYS3
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_MEMSYS3
ਅਚਲ sqlite3_mem_methods *sqlite3MemGetMemsys3(ਖਾਲੀ);
#ਜੇ_ਅੰਤ


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MUTEX_OMIT
  sqlite3_mutex_methods ਅਚਲ *sqlite3DefaultMutex(ਖਾਲੀ);
  sqlite3_mutex_methods ਅਚਲ *sqlite3NoopMutex(ਖਾਲੀ);
  sqlite3_mutex *sqlite3MutexAlloc(ਪੂਰਨ);
  ਪੂਰਨ sqlite3MutexInit(ਖਾਲੀ);
  ਪੂਰਨ sqlite3_Mutex_ਸਿਰਾ(ਖਾਲੀ);
#ਜੇ_ਅੰਤ
#ਜੇ !defined(SQLITE_MUTEX_OMIT) && !defined(SQLITE_MUTEX_NOOP)
  ਖਾਲੀ sqlite3_ਸਮ੍ਰਿਤੀ_Barrier(ਖਾਲੀ);
#ਨਹੀਂਤਾਂ
# define sqlite3_ਸਮ੍ਰਿਤੀ_Barrier()
#ਜੇ_ਅੰਤ

sqlite3_int64 sqlite3_ਸਥਿਤੀ_ਮੁੱਲ(ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਸਥਿਤੀ_ਉੱਪਰ(ਪੂਰਨ, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਸਥਿਤੀ_ਥੱਲੇ(ਪੂਰਨ, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਸਥਿਤੀ_Highwater(ਪੂਰਨ, ਪੂਰਨ);
ਪੂਰਨ sqlite3_Lookaside_ਵਰਤਿਆ(sqlite3*,ਪੂਰਨ*);

/* Access to mutexes used by sqlite3_status() */
sqlite3_mutex *sqlite3Pcache1Mutex(ਖਾਲੀ);
sqlite3_mutex *sqlite3MallocMutex(ਖਾਲੀ);


/* The SQLITE_THREAD_MISUSE_WARNINGS compile-time option used to be called
** SQLITE_ENABLE_MULTITHREADED_CHECKS.  Keep that older macro for backwards
** compatibility, at least for a while... */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_MULTITHREADED_CHECKS
# define SQLITE_THREAD_MISUSE_WARNINGS 1
#ਜੇ_ਅੰਤ

/* SQLITE_THREAD_MISUSE_ABORT implies SQLITE_THREAD_MISUSE_WARNINGS */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_THREAD_MISUSE_ABORT
# define SQLITE_THREAD_MISUSE_WARNINGS 1
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_THREAD_MISUSE_WARNINGS) && !defined(SQLITE_MUTEX_OMIT)
ਖਾਲੀ sqlite3MutexWarnOnContention(sqlite3_mutex*);
#ਨਹੀਂਤਾਂ
# define sqlite3MutexWarnOnContention(x)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
# define EXP754 (((u64)0x7ff)<<52)
# define MAN754 ((((u64)1)<<52)-1)
# define IsNaN(X) (((X)&EXP754)==EXP754 && ((X)&MAN754)!=0)
# define IsOvfl(X) (((X)&EXP754)==EXP754)
  ਪੂਰਨ sqlite3IsNaN(ਦੁੱਗਣਾ);
  ਪੂਰਨ sqlite3_Is_ਉਛਾਲ(ਦੁੱਗਣਾ);
#ਨਹੀਂਤਾਂ
# define IsNaN(X)             0
# define sqlite3IsNaN(X)      0
# define sqlite3_Is_ਉਛਾਲ_2(X) 0
#ਜੇ_ਅੰਤ

/*
** An instance of the following structure holds information about SQL
** functions arguments that are the parameters to the printf() function.
*/
ਬਣਤਰ PrintfArguments {
  ਪੂਰਨ nArg;                /* Total number of arguments */
  ਪੂਰਨ n_ਵਰਤਿਆ;               /* Number of arguments used so far */
  sqlite3_ਮੁੱਲ **apArg;   /* The argument values */
};

/*
** Maxium number of base-10 digits in an unsigned 64-bit integer
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_U64_DIGITS 20

/*
** An instance of this object receives the decoding of a floating point
** value into an approximate decimal representation.
*/
ਬਣਤਰ FpDecode {
  ਪੂਰਨ n;                           /* Significant digits in the decode */
  ਪੂਰਨ iDP;                         /* Location of the decimal point */
  ਅੱਖਰ *z;                         /* Start of significant digits */
  ਅੱਖਰ zBuf[SQLITE_U64_DIGITS+1];  /* Storage for significant digits */
  ਅੱਖਰ sign;                       /* '+' or '-' */
  ਅੱਖਰ isSpecial;                  /* 1: Infinity  2: NaN */
};

ਖਾਲੀ sqlite3FpDecode(FpDecode*,ਦੁੱਗਣਾ,ਪੂਰਨ,ਪੂਰਨ);
ਅੱਖਰ *sqlite3MPrintf(sqlite3*,ਅਚਲ ਅੱਖਰ*, ...);
ਅੱਖਰ *sqlite3VMPrintf(sqlite3*,ਅਚਲ ਅੱਖਰ*, va_list);
#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  ਖਾਲੀ sqlite3DebugPrintf(ਅਚਲ ਅੱਖਰ*, ...);
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_TEST)
  ਖਾਲੀ *sqlite3_Test_ਪਾਠ_ਮੰਜ਼ਿਲ_Ptr(ਅਚਲ ਅੱਖਰ*);
  ਅਚਲ ਅੱਖਰ *sqlite3_Test_Ptr_ਮੰਜ਼ਿਲ_ਪਾਠ(ਖਾਲੀ*);
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_DEBUG)
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਸਤਰ(ਰੁੱਖ_View*, ਅਚਲ ਅੱਖਰ *zFormat, ...);
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(ਰੁੱਖ_View*, ਅਚਲ ਵਿਅੰਜਕ*, u8);
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Bare_ਵਿਅੰਜਕ_List(ਰੁੱਖ_View*, ਅਚਲ ਵਿਅੰਜਕ_List*, ਅਚਲ ਅੱਖਰ*);
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(ਰੁੱਖ_View*, ਅਚਲ ਵਿਅੰਜਕ_List*, u8, ਅਚਲ ਅੱਖਰ*);
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Bare_Id_List(ਰੁੱਖ_View*, ਅਚਲ IdList*, ਅਚਲ ਅੱਖਰ*);
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Id_List(ਰੁੱਖ_View*, ਅਚਲ IdList*, u8, ਅਚਲ ਅੱਖਰ*);
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਕਾਲਮ_List(ਰੁੱਖ_View*, ਅਚਲ ਕਾਲਮ*, ਪੂਰਨ, u8);
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Src_List(ਰੁੱਖ_View*, ਅਚਲ SrcList*);
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Select(ਰੁੱਖ_View*, ਅਚਲ Select*, u8);
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_With(ਰੁੱਖ_View*, ਅਚਲ With*, u8);
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Upsert(ਰੁੱਖ_View*, ਅਚਲ Upsert*, u8);
#ਜੇ TREETRACE_ENABLED
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Delete(ਅਚਲ With*, ਅਚਲ SrcList*, ਅਚਲ ਵਿਅੰਜਕ*,
                             ਅਚਲ ਵਿਅੰਜਕ_List*,ਅਚਲ ਵਿਅੰਜਕ*, ਅਚਲ Trigger*);
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਪਾਓ(ਅਚਲ With*, ਅਚਲ SrcList*,
                             ਅਚਲ IdList*, ਅਚਲ Select*, ਅਚਲ ਵਿਅੰਜਕ_List*,
                             ਪੂਰਨ, ਅਚਲ Upsert*, ਅਚਲ Trigger*);
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Update(ਅਚਲ With*, ਅਚਲ SrcList*, ਅਚਲ ਵਿਅੰਜਕ_List*,
                             ਅਚਲ ਵਿਅੰਜਕ*, ਪੂਰਨ, ਅਚਲ ਵਿਅੰਜਕ_List*, ਅਚਲ ਵਿਅੰਜਕ*,
                             ਅਚਲ Upsert*, ਅਚਲ Trigger*);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Trigger_ਕਦਮ(ਰੁੱਖ_View*, ਅਚਲ Trigger_ਕਦਮ*, u8, u8);
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Trigger(ਰੁੱਖ_View*, ਅਚਲ Trigger*, u8, u8);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਖਿੜਕੀ(ਰੁੱਖ_View*, ਅਚਲ ਖਿੜਕੀ*, u8);
  ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Win_Func(ਰੁੱਖ_View*, ਅਚਲ ਖਿੜਕੀ*, u8);
#ਜੇ_ਅੰਤ
  ਖਾਲੀ sqlite3_Show_ਵਿਅੰਜਕ(ਅਚਲ ਵਿਅੰਜਕ*);
  ਖਾਲੀ sqlite3_Show_ਵਿਅੰਜਕ_List(ਅਚਲ ਵਿਅੰਜਕ_List*);
  ਖਾਲੀ sqlite3ShowIdList(ਅਚਲ IdList*);
  ਖਾਲੀ sqlite3ShowSrcList(ਅਚਲ SrcList*);
  ਖਾਲੀ sqlite3ShowSelect(ਅਚਲ Select*);
  ਖਾਲੀ sqlite3ShowWith(ਅਚਲ With*);
  ਖਾਲੀ sqlite3ShowUpsert(ਅਚਲ Upsert*);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
  ਖਾਲੀ sqlite3_Show_Trigger_ਕਦਮ(ਅਚਲ Trigger_ਕਦਮ*);
  ਖਾਲੀ sqlite3_Show_Trigger_ਕਦਮ_List(ਅਚਲ Trigger_ਕਦਮ*);
  ਖਾਲੀ sqlite3ShowTrigger(ਅਚਲ Trigger*);
  ਖਾਲੀ sqlite3ShowTriggerList(ਅਚਲ Trigger*);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
  ਖਾਲੀ sqlite3_Show_ਖਿੜਕੀ(ਅਚਲ ਖਿੜਕੀ*);
  ਖਾਲੀ sqlite3ShowWinFunc(ਅਚਲ ਖਿੜਕੀ*);
#ਜੇ_ਅੰਤ
  ਖਾਲੀ sqlite3ShowBitvec(Bitvec*);
#ਜੇ_ਅੰਤ

ਖਾਲੀ sqlite3_ਸੈੱਟ_String(ਅੱਖਰ **, sqlite3*, ਅਚਲ ਅੱਖਰ*);
ਖਾਲੀ sqlite3_ਤਰੱਕੀ_Check(Parse*);
ਖਾਲੀ sqlite3_Error_ਸੁਨੇਹਾ(Parse*, ਅਚਲ ਅੱਖਰ*, ...);
ਪੂਰਨ sqlite3_Error_ਮੰਜ਼ਿਲ_Parser(sqlite3*,ਪੂਰਨ);
ਖਾਲੀ sqlite3Dequote(ਅੱਖਰ*);
ਖਾਲੀ sqlite3_Dequote_ਵਿਅੰਜਕ(ਵਿਅੰਜਕ*);
ਖਾਲੀ sqlite3DequoteToken(Token*);
ਖਾਲੀ sqlite3_Dequote_ਸੰਖਿਆ(Parse*, ਵਿਅੰਜਕ*);
ਖਾਲੀ sqlite3TokenInit(Token*,ਅੱਖਰ*);
ਪੂਰਨ sqlite3_Keyword_ਕੋਡ(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*, ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਲੜੀਵਾਰ_Parser(Parse*, ਅਚਲ ਅੱਖਰ*);
ਖਾਲੀ sqlite3_ਅੰਤ_Coding(Parse*);
ਪੂਰਨ sqlite3_Get_ਆਰਜ਼ੀ_Reg(Parse*);
ਖਾਲੀ sqlite3_Release_ਆਰਜ਼ੀ_Reg(Parse*,ਪੂਰਨ);
ਪੂਰਨ sqlite3_Get_ਆਰਜ਼ੀ_Range(Parse*,ਪੂਰਨ);
ਖਾਲੀ sqlite3_Release_ਆਰਜ਼ੀ_Range(Parse*,ਪੂਰਨ,ਪੂਰਨ);
ਖਾਲੀ sqlite3_Clear_ਆਰਜ਼ੀ_Reg_Cache(Parse*);
ਖਾਲੀ sqlite3_Touch_ਰਜਿਸਟਰ(Parse*,ਪੂਰਨ);
#ਜੇ defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
ਪੂਰਨ sqlite3_ਪਹਿਲਾ_ਉਪਲਬਧ_ਰਜਿਸਟਰ(Parse*,ਪੂਰਨ);
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਪੂਰਨ sqlite3_No_Temps_ਵਿੱਚ_Range(Parse*,ਪੂਰਨ,ਪੂਰਨ);
#ਜੇ_ਅੰਤ
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_Alloc(sqlite3*,ਪੂਰਨ,ਅਚਲ Token*,ਪੂਰਨ);
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ(sqlite3*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*);
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_Int32(sqlite3*,ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Attach_Subtrees(sqlite3*,ਵਿਅੰਜਕ*,ਵਿਅੰਜਕ*,ਵਿਅੰਜਕ*);
ਵਿਅੰਜਕ *sqlite3PExpr(Parse*, ਪੂਰਨ, ਵਿਅੰਜਕ*, ਵਿਅੰਜਕ*);
ਖਾਲੀ sqlite3_PExpr_ਜੋੜੋ_Select(Parse*, ਵਿਅੰਜਕ*, Select*);
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_And(Parse*,ਵਿਅੰਜਕ*, ਵਿਅੰਜਕ*);
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_Simplified_And_Or(ਵਿਅੰਜਕ*);
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_Function(Parse*,ਵਿਅੰਜਕ_List*, ਅਚਲ Token*, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Function_ਕ੍ਰਮ_By(Parse*,ਵਿਅੰਜਕ*,ਵਿਅੰਜਕ_List*);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕ੍ਰਮ_By_Aggregate_Error(Parse*,ਵਿਅੰਜਕ*);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Function_Usable(Parse*,ਅਚਲ ਵਿਅੰਜਕ*,ਅਚਲ FuncDef*);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਸੌਂਪੋ_Var_ਸੰਖਿਆ(Parse*, ਵਿਅੰਜਕ*, u32);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Delete(sqlite3*, ਵਿਅੰਜਕ*);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Delete_Generic(sqlite3*,ਖਾਲੀ*);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Deferred_Delete(Parse*, ਵਿਅੰਜਕ*);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Unmap_And_Delete(Parse*, ਵਿਅੰਜਕ*);
ਵਿਅੰਜਕ_List *sqlite3_ਵਿਅੰਜਕ_List_Append(Parse*,ਵਿਅੰਜਕ_List*,ਵਿਅੰਜਕ*);
ਵਿਅੰਜਕ_List *sqlite3_ਵਿਅੰਜਕ_List_Append_ਸਦਿਸ਼(Parse*,ਵਿਅੰਜਕ_List*,IdList*,ਵਿਅੰਜਕ*);
Select *sqlite3_ਵਿਅੰਜਕ_List_ਮੰਜ਼ਿਲ_ਮੁੱਲਾਂ(Parse*, ਪੂਰਨ, ਵਿਅੰਜਕ_List*);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_List_ਸੈੱਟ_ਛਾਂਟੋ_ਕ੍ਰਮ(ਵਿਅੰਜਕ_List*,ਪੂਰਨ,ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_List_ਸੈੱਟ_Name(Parse*,ਵਿਅੰਜਕ_List*,ਅਚਲ Token*,ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_List_ਸੈੱਟ_ਵਿਸਤਾਰ(Parse*,ਵਿਅੰਜਕ_List*,ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_List_Delete(sqlite3*, ਵਿਅੰਜਕ_List*);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_List_Delete_Generic(sqlite3*,ਖਾਲੀ*);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Can_Return_Subtype(Parse*,ਵਿਅੰਜਕ*);
u32 sqlite3_ਵਿਅੰਜਕ_List_Flags(ਅਚਲ ਵਿਅੰਜਕ_List*);
ਪੂਰਨ sqlite3_ਸੂਚਕਾਂਕ_Has_Duplicate_ਜੜ੍ਹ_ਪੰਨਾ(ਸੂਚਕਾਂਕ*);
ਪੂਰਨ sqlite3Init(sqlite3*, ਅੱਖਰ**);
ਪੂਰਨ sqlite3InitCallback(ਖਾਲੀ*, ਪੂਰਨ, ਅੱਖਰ**, ਅੱਖਰ**);
ਪੂਰਨ sqlite3InitOne(sqlite3*, ਪੂਰਨ, ਅੱਖਰ**, u32);
ਖਾਲੀ sqlite3Pragma(Parse*,Token*,Token*,Token*,ਪੂਰਨ);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
Module *sqlite3_Pragma_Vtab_ਰਜਿਸਟਰ(sqlite3*,ਅਚਲ ਅੱਖਰ *zName);
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3_Reset_All_Schemas_ਦਾ_Connection(sqlite3*);
ਖਾਲੀ sqlite3ResetOneSchema(sqlite3*,ਪੂਰਨ);
ਖਾਲੀ sqlite3CollapseDatabaseArray(sqlite3*);
ਖਾਲੀ sqlite3_Commit_ਅੰਦਰੂਨੀ_Changes(sqlite3*);
ਖਾਲੀ sqlite3_ਕਾਲਮ_ਸੈੱਟ_ਵਿਅੰਜਕ(Parse*,ਸਾਰਣੀ*,ਕਾਲਮ*,ਵਿਅੰਜਕ*);
ਵਿਅੰਜਕ *sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ(ਸਾਰਣੀ*,ਕਾਲਮ*);
ਵਿਅੰਜਕ *sqlite3_ਕਾਲਮ_ਵਿਅੰਜਕ_Auth(ਸਾਰਣੀ*,ਕਾਲਮ*,Parse*);
ਖਾਲੀ sqlite3_ਕਾਲਮ_ਸੈੱਟ_Coll(sqlite3*,ਕਾਲਮ*,ਅਚਲ ਅੱਖਰ*zColl);
ਅਚਲ ਅੱਖਰ *sqlite3_ਕਾਲਮ_Coll(ਕਾਲਮ*);
ਖਾਲੀ sqlite3_Delete_ਕਾਲਮ_ਨਾਂ(sqlite3*,ਸਾਰਣੀ*);
ਖਾਲੀ sqlite3_Generate_ਕਾਲਮ_ਨਾਂ(Parse *pParse, Select *pSelect);
ਪੂਰਨ sqlite3_Columns_From_ਵਿਅੰਜਕ_List(Parse*,ਵਿਅੰਜਕ_List*,i16*,ਕਾਲਮ**);
ਖਾਲੀ sqlite3_Subquery_ਕਾਲਮ_Types(Parse*,ਸਾਰਣੀ*,Select*,ਅੱਖਰ);
ਸਾਰਣੀ *sqlite3_ਨਤੀਜਾ_ਸੈੱਟ_ਦਾ_Select(Parse*,Select*,ਅੱਖਰ);
ਖਾਲੀ sqlite3_Open_Schema_ਸਾਰਣੀ(Parse *, ਪੂਰਨ);
ਸੂਚਕਾਂਕ *sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(ਸਾਰਣੀ*);
ਪੂਰਨ sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(ਸੂਚਕਾਂਕ*, ਪੂਰਨ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
# define sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(T,X) (X)  /* No-op pass-through */
# define sqlite3_Storage_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸਾਰਣੀ(T,X) (X)  /* No-op pass-through */
#ਨਹੀਂਤਾਂ
  i16 sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(ਸਾਰਣੀ*, i16);
  i16 sqlite3_Storage_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸਾਰਣੀ(ਸਾਰਣੀ*, i16);
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3_ਸ਼ੁਰੂ_ਸਾਰਣੀ(Parse*,Token*,Token*,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ);
#ਜੇ SQLITE_ENABLE_HIDDEN_COLUMNS
  ਖਾਲੀ sqlite3_ਕਾਲਮ_Properties_From_Name(ਸਾਰਣੀ*, ਕਾਲਮ*);
#ਨਹੀਂਤਾਂ
# define sqlite3_ਕਾਲਮ_Properties_From_Name(T,C) /* no-op */
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3_ਜੋੜੋ_ਕਾਲਮ(Parse*,Token,Token);
ਖਾਲੀ sqlite3_ਜੋੜੋ_Not_Null(Parse*, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਜੋੜੋ_Primary_ਕੁੰਜੀ(Parse*, ਵਿਅੰਜਕ_List*, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਜੋੜੋ_Check_Constraint(Parse*, ਵਿਅੰਜਕ*, ਅਚਲ ਅੱਖਰ*, ਅਚਲ ਅੱਖਰ*);
ਖਾਲੀ sqlite3_ਜੋੜੋ_Default_ਮੁੱਲ(Parse*,ਵਿਅੰਜਕ*,ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*);
ਖਾਲੀ sqlite3_ਜੋੜੋ_Collate_Type(Parse*, Token*);
ਖਾਲੀ sqlite3_ਜੋੜੋ_Generated(Parse*,ਵਿਅੰਜਕ*,Token*);
ਖਾਲੀ sqlite3_ਸਿਰਾ_ਸਾਰਣੀ(Parse*,Token*,Token*,u32,Select*);
ਖਾਲੀ sqlite3_ਜੋੜੋ_Returning(Parse*,ਵਿਅੰਜਕ_List*);
ਪੂਰਨ sqlite3ParseUri(ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*,
                    sqlite3_vfs**,ਅੱਖਰ**,ਅੱਖਰ **);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3CodecQueryParameters(A,B,C) 0
Btree *sqlite3_Db_Name_ਮੰਜ਼ਿਲ_Btree(sqlite3*,ਅਚਲ ਅੱਖਰ*);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UNTESTABLE
# define sqlite3FaultSim(X) SQLITE_OK
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ sqlite3FaultSim(ਪੂਰਨ);
#ਜੇ_ਅੰਤ

Bitvec *sqlite3BitvecCreate(u32);
ਪੂਰਨ sqlite3BitvecTest(Bitvec*, u32);
ਪੂਰਨ sqlite3BitvecTestNotNull(Bitvec*, u32);
ਪੂਰਨ sqlite3_Bitvec_ਸੈੱਟ(Bitvec*, u32);
ਖਾਲੀ sqlite3BitvecClear(Bitvec*, u32, ਖਾਲੀ*);
ਖਾਲੀ sqlite3BitvecDestroy(Bitvec*);
u32 sqlite3_Bitvec_ਮਾਪ(Bitvec*);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_UNTESTABLE
ਪੂਰਨ sqlite3BitvecBuiltinTest(ਪੂਰਨ,ਪੂਰਨ*);
#ਜੇ_ਅੰਤ

ਪੰਕਤੀ_ਸੈੱਟ *sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_Init(sqlite3*);
ਖਾਲੀ sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_Delete(ਖਾਲੀ*);
ਖਾਲੀ sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_Clear(ਖਾਲੀ*);
ਖਾਲੀ sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_ਪਾਓ(ਪੰਕਤੀ_ਸੈੱਟ*, i64);
ਪੂਰਨ sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_Test(ਪੰਕਤੀ_ਸੈੱਟ*, ਪੂਰਨ iBatch, i64);
ਪੂਰਨ sqlite3_ਪੰਕਤੀ_ਸੈੱਟ_ਅਗਲਾ(ਪੰਕਤੀ_ਸੈੱਟ*, i64*);

ਖਾਲੀ sqlite3CreateView(Parse*,Token*,Token*,Token*,ਵਿਅੰਜਕ_List*,Select*,ਪੂਰਨ,ਪੂਰਨ);

#ਜੇ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
  ਪੂਰਨ sqlite3_View_Get_ਕਾਲਮ_ਨਾਂ(Parse*,ਸਾਰਣੀ*);
#ਨਹੀਂਤਾਂ
# define sqlite3_View_Get_ਕਾਲਮ_ਨਾਂ(A,B) 0
#ਜੇ_ਅੰਤ

#ਜੇ SQLITE_MAX_ATTACHED>30
  ਪੂਰਨ sqlite3_Db_ਨਕਾਬ_All_Zero(y_Db_ਨਕਾਬ);
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3_Drop_ਸਾਰਣੀ(Parse*, SrcList*, ਪੂਰਨ, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਕੋਡ_Drop_ਸਾਰਣੀ(Parse*, ਸਾਰਣੀ*, ਪੂਰਨ, ਪੂਰਨ);
ਖਾਲੀ sqlite3_Delete_ਸਾਰਣੀ(sqlite3*, ਸਾਰਣੀ*);
ਖਾਲੀ sqlite3_Delete_ਸਾਰਣੀ_Generic(sqlite3*, ਖਾਲੀ*);
ਖਾਲੀ sqlite3_Free_ਸੂਚਕਾਂਕ(sqlite3*, ਸੂਚਕਾਂਕ*);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINCREMENT
  ਖਾਲੀ sqlite3AutoincrementBegin(Parse *pParse);
  ਖਾਲੀ sqlite3_Autoincrement_ਸਿਰਾ(Parse *pParse);
#ਨਹੀਂਤਾਂ
# define sqlite3AutoincrementBegin(X)
# define sqlite3_Autoincrement_ਸਿਰਾ(X)
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3_ਪਾਓ(Parse*, SrcList*, Select*, IdList*, ਪੂਰਨ, Upsert*);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
  ਖਾਲੀ sqlite3ComputeGeneratedColumns(Parse*, ਪੂਰਨ, ਸਾਰਣੀ*);
#ਜੇ_ਅੰਤ
ਖਾਲੀ *sqlite3ArrayAllocate(sqlite3*,ਖਾਲੀ*,ਪੂਰਨ,ਪੂਰਨ*,ਪੂਰਨ*);
IdList *sqlite3IdListAppend(Parse*, IdList*, Token*);
ਪੂਰਨ sqlite3_Id_List_ਸੂਚਕਾਂਕ(IdList*,ਅਚਲ ਅੱਖਰ*);
SrcList *sqlite3SrcListEnlarge(Parse*, SrcList*, ਪੂਰਨ, ਪੂਰਨ);
SrcList *sqlite3SrcListAppendList(Parse *pParse, SrcList *p1, SrcList *p2);
SrcList *sqlite3SrcListAppend(Parse*, SrcList*, Token*, Token*);
ਖਾਲੀ sqlite3SubqueryDelete(sqlite3*,Subquery*);
Select *sqlite3SubqueryDetach(sqlite3*,Src_ਵਸਤੂ*);
ਪੂਰਨ sqlite3_Src_ਵਸਤੂ_Attach_Subquery(Parse*, Src_ਵਸਤੂ*, Select*, ਪੂਰਨ);
SrcList *sqlite3_Src_List_Append_From_ਪਦ(Parse*, SrcList*, Token*, Token*,
                                      Token*, Select*, OnOrUsing*);
ਖਾਲੀ sqlite3_Src_List_ਸੂਚਕਾਂਕਿਤ_By(Parse *, SrcList *, Token *);
ਖਾਲੀ sqlite3_Src_List_Func_ਦਲੀਲਾਂ(Parse*, SrcList*, ਵਿਅੰਜਕ_List*);
ਪੂਰਨ sqlite3_ਸੂਚਕਾਂਕਿਤ_By_Lookup(Parse *, Src_ਵਸਤੂ *);
ਖਾਲੀ sqlite3_Src_List_ਖਿਸਕਾਓ_Join_Type(Parse*,SrcList*);
ਖਾਲੀ sqlite3_Src_List_ਸੌਂਪੋ_Cursors(Parse*, SrcList*);
ਖਾਲੀ sqlite3IdListDelete(sqlite3*, IdList*);
ਖਾਲੀ sqlite3ClearOnOrUsing(sqlite3*, OnOrUsing*);
ਖਾਲੀ sqlite3SrcListDelete(sqlite3*, SrcList*);
ਸੂਚਕਾਂਕ *sqlite3_Allocate_ਸੂਚਕਾਂਕ_Object(sqlite3*,ਪੂਰਨ,ਪੂਰਨ,ਅੱਖਰ**);
ਪੂਰਨ sqlite3_ਸੂਚਕਾਂਕ_Bloomable(ਅਚਲ ਸੂਚਕਾਂਕ*,ਪੂਰਨ);
ਖਾਲੀ sqlite3_Create_ਸੂਚਕਾਂਕ(Parse*,Token*,Token*,SrcList*,ਵਿਅੰਜਕ_List*,ਪੂਰਨ,Token*,
                          ਵਿਅੰਜਕ*, ਪੂਰਨ, ਪੂਰਨ, u8);
ਖਾਲੀ sqlite3_Drop_ਸੂਚਕਾਂਕ(Parse*, SrcList*, ਪੂਰਨ);
ਪੂਰਨ sqlite3Select(Parse*, Select*, SelectDest*);
Select *sqlite3SelectNew(Parse*,ਵਿਅੰਜਕ_List*,SrcList*,ਵਿਅੰਜਕ*,ਵਿਅੰਜਕ_List*,
                         ਵਿਅੰਜਕ*,ਵਿਅੰਜਕ_List*,u32,ਵਿਅੰਜਕ*);
ਖਾਲੀ sqlite3SelectDelete(sqlite3*, Select*);
ਖਾਲੀ sqlite3SelectDeleteGeneric(sqlite3*,ਖਾਲੀ*);
ਖਾਲੀ sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect);
ਸਾਰਣੀ *sqlite3SrcListLookup(Parse*, SrcList*);
ਪੂਰਨ sqlite3IsReadOnly(Parse*, ਸਾਰਣੀ*, Trigger*);
ਖਾਲੀ sqlite3_Open_ਸਾਰਣੀ(Parse*, ਪੂਰਨ iCur, ਪੂਰਨ iDb, ਸਾਰਣੀ*, ਪੂਰਨ);
#ਜੇ defined(SQLITE_ENABLE_UPDATE_DELETE_ਹੱਦਬੰਦੀ) && !defined(SQLITE_OMIT_SUBQUERY)
ਵਿਅੰਜਕ *sqlite3_ਹੱਦਬੰਦੀ_Where(Parse*,SrcList*,ਵਿਅੰਜਕ*,ਵਿਅੰਜਕ_List*,ਵਿਅੰਜਕ*,ਅੱਖਰ*);
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3_ਕੋਡ_Change_ਗਿਣਤੀ(Vdbe*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*);
ਖਾਲੀ sqlite3DeleteFrom(Parse*, SrcList*, ਵਿਅੰਜਕ*, ਵਿਅੰਜਕ_List*, ਵਿਅੰਜਕ*);
ਖਾਲੀ sqlite3Update(Parse*, SrcList*, ਵਿਅੰਜਕ_List*,ਵਿਅੰਜਕ*,ਪੂਰਨ,ਵਿਅੰਜਕ_List*,ਵਿਅੰਜਕ*,
                   Upsert*);
WhereInfo *sqlite3WhereBegin(Parse*,SrcList*,ਵਿਅੰਜਕ*,ਵਿਅੰਜਕ_List*,
                             ਵਿਅੰਜਕ_List*,Select*,u16,ਪੂਰਨ);
ਖਾਲੀ sqlite3_Where_ਸਿਰਾ(WhereInfo*);
LogEst sqlite3_Where_Output_ਪੰਕਤੀ_ਗਿਣਤੀ(WhereInfo*);
ਪੂਰਨ sqlite3WhereIsDistinct(WhereInfo*);
ਪੂਰਨ sqlite3WhereIsOrdered(WhereInfo*);
ਪੂਰਨ sqlite3_Where_ਕ੍ਰਮ_By_ਹੱਦਬੰਦੀ_Opt_Label(WhereInfo*);
ਖਾਲੀ sqlite3_Where_Min_Max_Opt_Early_ਬਾਹਰਲਾ(Vdbe*,WhereInfo*);
ਪੂਰਨ sqlite3WhereIsSorted(WhereInfo*);
ਪੂਰਨ sqlite3WhereContinueLabel(WhereInfo*);
ਪੂਰਨ sqlite3WhereBreakLabel(WhereInfo*);
ਪੂਰਨ sqlite3WhereOkOnePass(WhereInfo*, ਪੂਰਨ*);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ONEPASS_OFF      0        /* Use of ONEPASS not allowed */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ONEPASS_SINGLE   1        /* ONEPASS valid for a single row update */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ONEPASS_MULTI    2        /* ONEPASS is valid for multiple rows */
ਪੂਰਨ sqlite3_Where_ਵਰਤਦਾ_Deferred_Seek(WhereInfo*);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਲੱਦੋ_ਸੂਚਕਾਂਕ_ਕਾਲਮ(Parse*, ਸੂਚਕਾਂਕ*, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ(Parse*, ਸਾਰਣੀ*, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ, u8);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(Vdbe*, ਸਾਰਣੀ*, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Move(Parse*, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ(ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ iReg);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(Parse*, ਵਿਅੰਜਕ*, ਪੂਰਨ);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Generated_ਕਾਲਮ(Parse*, ਸਾਰਣੀ*, ਕਾਲਮ*, ਪੂਰਨ);
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Copy(Parse*, ਵਿਅੰਜਕ*, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Factorable(Parse*, ਵਿਅੰਜਕ*, ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਲੜੀਵਾਰ_Just_Once(Parse*, ਵਿਅੰਜਕ*, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Null_ਰਜਿਸਟਰ_Range(Parse*, ਪੂਰਨ, ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(Parse*, ਵਿਅੰਜਕ*, ਪੂਰਨ*);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(Parse*, ਵਿਅੰਜਕ*, ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿਅੰਜਕ_List(Parse*, ਵਿਅੰਜਕ_List*, ਪੂਰਨ, ਪੂਰਨ, u8);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ECEL_DUP      0x01  /* Deep, not shallow copies */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ECEL_ਗੁਣਕ   0x02  /* Factor out constant terms */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ECEL_REF      0x04  /* Use ExprList.u.x.iOrderByCol */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_ECEL_OMITREF  0x08  /* Omit if ExprList.u.x.iOrderByCol */
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_If_True(Parse*, ਵਿਅੰਜਕ*, ਪੂਰਨ, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_If_False(Parse*, ਵਿਅੰਜਕ*, ਪੂਰਨ, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_If_False_Dup(Parse*, ਵਿਅੰਜਕ*, ਪੂਰਨ, ਪੂਰਨ);
ਸਾਰਣੀ *sqlite3_ਲੱਭੋ_ਸਾਰਣੀ(sqlite3*,ਅਚਲ ਅੱਖਰ*, ਅਚਲ ਅੱਖਰ*);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOCATE_VIEW    0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOCATE_NOERR   0x02
ਸਾਰਣੀ *sqlite3_Locate_ਸਾਰਣੀ(Parse*,u32 flags,ਅਚਲ ਅੱਖਰ*, ਅਚਲ ਅੱਖਰ*);
ਅਚਲ ਅੱਖਰ *sqlite3_Preferred_ਸਾਰਣੀ_Name(ਅਚਲ ਅੱਖਰ*);
ਸਾਰਣੀ *sqlite3_Locate_ਸਾਰਣੀ_ਵਸਤੂ(Parse*,u32 flags,Src_ਵਸਤੂ *);
ਸੂਚਕਾਂਕ *sqlite3_ਲੱਭੋ_ਸੂਚਕਾਂਕ(sqlite3*,ਅਚਲ ਅੱਖਰ*, ਅਚਲ ਅੱਖਰ*);
ਖਾਲੀ sqlite3_Unlink_And_Delete_ਸਾਰਣੀ(sqlite3*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*);
ਖਾਲੀ sqlite3_Unlink_And_Delete_ਸੂਚਕਾਂਕ(sqlite3*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*);
ਖਾਲੀ sqlite3Vacuum(Parse*,Token*,ਵਿਅੰਜਕ*);
ਪੂਰਨ sqlite3_ਲੜੀਵਾਰ_Vacuum(ਅੱਖਰ**, sqlite3*, ਪੂਰਨ, sqlite3_ਮੁੱਲ*);
ਅੱਖਰ *sqlite3NameFromToken(sqlite3*, ਅਚਲ Token*);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Compare(ਅਚਲ Parse*,ਅਚਲ ਵਿਅੰਜਕ*,ਅਚਲ ਵਿਅੰਜਕ*, ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Compare_Skip(ਵਿਅੰਜਕ*,ਵਿਅੰਜਕ*,ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_List_Compare(ਅਚਲ ਵਿਅੰਜਕ_List*,ਅਚਲ ਵਿਅੰਜਕ_List*, ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_ਵਿਅੰਜਕ(ਅਚਲ Parse*,ਅਚਲ ਵਿਅੰਜਕ*,ਅਚਲ ਵਿਅੰਜਕ*, ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Non_Null_ਪੰਕਤੀ(ਵਿਅੰਜਕ*,ਪੂਰਨ,ਪੂਰਨ);
ਖਾਲੀ sqlite3AggInfoPersistWalkerInit(Walker*,Parse*);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Analyze_Aggregates(NameContext*, ਵਿਅੰਜਕ*);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_Analyze_Agg_List(NameContext*,ਵਿਅੰਜਕ_List*);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Covered_By_ਸੂਚਕਾਂਕ(ਵਿਅੰਜਕ*, ਪੂਰਨ iCur, ਸੂਚਕਾਂਕ *pIdx);
ਪੂਰਨ sqlite3ReferencesSrcList(Parse*, ਵਿਅੰਜਕ*, SrcList*);
Vdbe *sqlite3GetVdbe(Parse*);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_UNTESTABLE
ਖਾਲੀ sqlite3PrngSaveState(ਖਾਲੀ);
ਖਾਲੀ sqlite3PrngRestoreState(ਖਾਲੀ);
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3RollbackAll(sqlite3*,ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਕੋਡ_Verify_Schema(Parse*, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਕੋਡ_Verify_Named_Schema(Parse*, ਅਚਲ ਅੱਖਰ *zDb);
ਖਾਲੀ sqlite3BeginTransaction(Parse*, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਸਿਰਾ_Transaction(Parse*,ਪੂਰਨ);
ਖਾਲੀ sqlite3Savepoint(Parse*, ਪੂਰਨ, Token*);
ਖਾਲੀ sqlite3CloseSavepoints(sqlite3 *);
ਖਾਲੀ sqlite3LeaveMutexAndCloseZombie(sqlite3*);
u32 sqlite3IsTrueOrFalse(ਅਚਲ ਅੱਖਰ*);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Id_ਮੰਜ਼ਿਲ_True_False(ਵਿਅੰਜਕ*);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Truth_ਮੁੱਲ(ਅਚਲ ਵਿਅੰਜਕ*);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_Constant(Parse*,ਵਿਅੰਜਕ*);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_Constant_Or_Function(ਵਿਅੰਜਕ*, u8);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_Constant_Or_ਸਮੂਹ_By(Parse*, ਵਿਅੰਜਕ*, ਵਿਅੰਜਕ_List*);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_ਸਾਰਣੀ_Constant(ਵਿਅੰਜਕ*,ਪੂਰਨ,ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_Single_ਸਾਰਣੀ_Constraint(ਵਿਅੰਜਕ*,ਅਚਲ SrcList*,ਪੂਰਨ,ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_List_Is_Constant(Parse *pParse, ਵਿਅੰਜਕ_List *pList, ਪੂਰਨ bNoIs);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਸ਼ਾਮਲ_ਹੈ_Subquery(ਵਿਅੰਜਕ*);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_Integer(ਅਚਲ ਵਿਅੰਜਕ*, ਪੂਰਨ*, Parse*, ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Can_Be_Null(ਅਚਲ ਵਿਅੰਜਕ*);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Needs_No_Affinity_Change(ਅਚਲ ਵਿਅੰਜਕ*, ਅੱਖਰ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_Like_Operator(ਅਚਲ ਵਿਅੰਜਕ*);
ਪੂਰਨ sqlite3IsRowid(ਅਚਲ ਅੱਖਰ*);
ਅਚਲ ਅੱਖਰ *sqlite3RowidAlias(ਸਾਰਣੀ *pTab);
ਖਾਲੀ sqlite3_Generate_ਪੰਕਤੀ_Delete(
    Parse*,ਸਾਰਣੀ*,Trigger*,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ,i16,u8,u8,u8,ਪੂਰਨ);
ਖਾਲੀ sqlite3_Generate_ਪੰਕਤੀ_ਸੂਚਕਾਂਕ_Delete(Parse*, ਸਾਰਣੀ*, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ*, ਪੂਰਨ);
ਪੂਰਨ sqlite3_Generate_ਸੂਚਕਾਂਕ_ਕੁੰਜੀ(Parse*, ਸੂਚਕਾਂਕ*, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ*,ਸੂਚਕਾਂਕ*,ਪੂਰਨ);
ਖਾਲੀ sqlite3ResolvePartIdxLabel(Parse*,ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_References_Updated_ਕਾਲਮ(ਵਿਅੰਜਕ*,ਪੂਰਨ*,ਪੂਰਨ);
ਖਾਲੀ sqlite3GenerateConstraintChecks(Parse*,ਸਾਰਣੀ*,ਪੂਰਨ*,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ,
                                     u8,u8,ਪੂਰਨ,ਪੂਰਨ*,ਪੂਰਨ*,Upsert*);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_NULL_TRIM
  ਖਾਲੀ sqlite3_ਸੈੱਟ_Make_ਰਿਕਾਰਡ_P5(Vdbe*,ਸਾਰਣੀ*);
#ਨਹੀਂਤਾਂ
# define sqlite3_ਸੈੱਟ_Make_ਰਿਕਾਰਡ_P5(A,B)
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3CompleteInsertion(Parse*,ਸਾਰਣੀ*,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ*,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ);
ਪੂਰਨ sqlite3_Open_ਸਾਰਣੀ_And_Indices(Parse*, ਸਾਰਣੀ*, ਪੂਰਨ, u8, ਪੂਰਨ, u8*, ਪੂਰਨ*, ਪੂਰਨ*);
ਖਾਲੀ sqlite3BeginWriteOperation(Parse*, ਪੂਰਨ, ਪੂਰਨ);
ਖਾਲੀ sqlite3MultiWrite(Parse*);
ਖਾਲੀ sqlite3MayAbort(Parse*);
ਖਾਲੀ sqlite3HaltConstraint(Parse*, ਪੂਰਨ, ਪੂਰਨ, ਅੱਖਰ*, i8, u8);
ਖਾਲੀ sqlite3UniqueConstraint(Parse*, ਪੂਰਨ, ਸੂਚਕਾਂਕ*);
ਖਾਲੀ sqlite3RowidConstraint(Parse*, ਪੂਰਨ, ਸਾਰਣੀ*);
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_Dup(sqlite3*,ਅਚਲ ਵਿਅੰਜਕ*,ਪੂਰਨ);
ਵਿਅੰਜਕ_List *sqlite3_ਵਿਅੰਜਕ_List_Dup(sqlite3*,ਅਚਲ ਵਿਅੰਜਕ_List*,ਪੂਰਨ);
SrcList *sqlite3SrcListDup(sqlite3*,ਅਚਲ SrcList*,ਪੂਰਨ);
IdList *sqlite3IdListDup(sqlite3*,ਅਚਲ IdList*);
Select *sqlite3SelectDup(sqlite3*,ਅਚਲ Select*,ਪੂਰਨ);
FuncDef *sqlite3_Function_ਖੋਜ(ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*);
ਖਾਲੀ sqlite3_ਪਾਓ_Builtin_Funcs(FuncDef*,ਪੂਰਨ);
FuncDef *sqlite3_ਲੱਭੋ_Function(sqlite3*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ,u8,u8);
ਖਾਲੀ sqlite3_Quote_ਮੁੱਲ(StrAccum*,sqlite3_ਮੁੱਲ*,ਪੂਰਨ);
ਪੂਰਨ sqlite3AppendOneUtf8Character(ਅੱਖਰ*, u32);
ਖਾਲੀ sqlite3_ਰਜਿਸਟਰ_Builtin_Functions(ਖਾਲੀ);
ਖਾਲੀ sqlite3_ਰਜਿਸਟਰ_Date_ਸਮਾਂ_Functions(ਖਾਲੀ);
ਖਾਲੀ sqlite3_ਰਜਿਸਟਰ_Json_Functions(ਖਾਲੀ);
ਖਾਲੀ sqlite3_ਰਜਿਸਟਰ_ਪ੍ਰਤੀ_Connection_Builtin_Functions(sqlite3*);
#ਜੇ !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
  Module *sqlite3_Json_Vtab_ਰਜਿਸਟਰ(sqlite3*,ਅਚਲ ਅੱਖਰ*);
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3SafetyCheckOk(sqlite3*);
ਪੂਰਨ sqlite3SafetyCheckSickOrOk(sqlite3*);
ਖਾਲੀ sqlite3ChangeCookie(Parse*, ਪੂਰਨ);
With *sqlite3WithDup(sqlite3 *db, With *p);

#ਜੇ !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
  Module *sqlite3_Carray_ਰਜਿਸਟਰ(sqlite3*);
#ਜੇ_ਅੰਤ

#ਜੇ !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
ਖਾਲੀ sqlite3MaterializeView(Parse*, ਸਾਰਣੀ*, ਵਿਅੰਜਕ*, ਵਿਅੰਜਕ_List*,ਵਿਅੰਜਕ*,ਪੂਰਨ);
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
  ਖਾਲੀ sqlite3BeginTrigger(Parse*, Token*,Token*,ਪੂਰਨ,ਪੂਰਨ,IdList*,SrcList*,
                           ਵਿਅੰਜਕ*,ਪੂਰਨ, ਪੂਰਨ);
  ਖਾਲੀ sqlite3_ਅੰਤ_Trigger(Parse*, Trigger_ਕਦਮ*, Token*);
  ਖਾਲੀ sqlite3DropTrigger(Parse*, SrcList*, ਪੂਰਨ);
  ਖਾਲੀ sqlite3DropTriggerPtr(Parse*, Trigger*);
  Trigger *sqlite3TriggersExist(Parse *, ਸਾਰਣੀ*, ਪੂਰਨ, ਵਿਅੰਜਕ_List*, ਪੂਰਨ *p_ਨਕਾਬ);
  Trigger *sqlite3TriggerList(Parse *, ਸਾਰਣੀ *);
  ਖਾਲੀ sqlite3_ਕੋਡ_ਪੰਕਤੀ_Trigger(Parse*, Trigger *, ਪੂਰਨ, ਵਿਅੰਜਕ_List*, ਪੂਰਨ, ਸਾਰਣੀ *,
                            ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ);
  ਖਾਲੀ sqlite3_ਕੋਡ_ਪੰਕਤੀ_Trigger_ਸਿੱਧਾ(Parse *, Trigger *, ਸਾਰਣੀ *, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ);
  ਖਾਲੀ sqliteViewTriggers(Parse*, ਸਾਰਣੀ*, ਵਿਅੰਜਕ*, ਪੂਰਨ, ਵਿਅੰਜਕ_List*);
  ਖਾਲੀ sqlite3_Delete_Trigger_ਕਦਮ(sqlite3*, Trigger_ਕਦਮ*);
  Trigger_ਕਦਮ *sqlite3_Trigger_Select_ਕਦਮ(sqlite3*,Select*,
                                        ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*);
  Trigger_ਕਦਮ *sqlite3_Trigger_ਪਾਓ_ਕਦਮ(Parse*,SrcList*, IdList*,
                                        Select*,u8,Upsert*,
                                        ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*);
  Trigger_ਕਦਮ *sqlite3_Trigger_Update_ਕਦਮ(Parse*,SrcList*,SrcList*,ਵਿਅੰਜਕ_List*,
                                        ਵਿਅੰਜਕ*, u8, ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*);
  Trigger_ਕਦਮ *sqlite3_Trigger_Delete_ਕਦਮ(Parse*,SrcList*, ਵਿਅੰਜਕ*,
                                        ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*);
  ਖਾਲੀ sqlite3DeleteTrigger(sqlite3*, Trigger*);
  ਖਾਲੀ sqlite3UnlinkAndDeleteTrigger(sqlite3*,ਪੂਰਨ,ਅਚਲ ਅੱਖਰ*);
  u32 sqlite3TriggerColmask(Parse*,Trigger*,ਵਿਅੰਜਕ_List*,ਪੂਰਨ,ਪੂਰਨ,ਸਾਰਣੀ*,ਪੂਰਨ);
# define sqlite3ParseToplevel(p) ((p)->pToplevel ? (p)->pToplevel : (p))
# define sqlite3IsToplevel(p) ((p)->pToplevel==0)
#ਨਹੀਂਤਾਂ
# define sqlite3TriggersExist(B,C,D,E,F) 0
# define sqlite3DeleteTrigger(A,B)
# define sqlite3DropTriggerPtr(A,B)
# define sqlite3UnlinkAndDeleteTrigger(A,B,C)
# define sqlite3_ਕੋਡ_ਪੰਕਤੀ_Trigger(A,B,C,D,E,F,G,H,I)
# define sqlite3_ਕੋਡ_ਪੰਕਤੀ_Trigger_ਸਿੱਧਾ(A,B,C,D,E,F)
# define sqlite3TriggerList(X, Y) 0
# define sqlite3ParseToplevel(p) p
# define sqlite3IsToplevel(p) 1
# define sqlite3TriggerColmask(A,B,C,D,E,F,G) 0
#ਜੇ_ਅੰਤ

ਪੂਰਨ sqlite3JoinType(Parse*, Token*, Token*, Token*);
ਪੂਰਨ sqlite3_ਕਾਲਮ_ਸੂਚਕਾਂਕ(ਸਾਰਣੀ *pTab, ਅਚਲ ਅੱਖਰ *z_ਖਾਨਾ);
ਖਾਲੀ sqlite3_Src_ਵਸਤੂ_ਕਾਲਮ_ਵਰਤਿਆ(Src_ਵਸਤੂ*,ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਸੈੱਟ_Join_ਵਿਅੰਜਕ(ਵਿਅੰਜਕ*,ਪੂਰਨ,u32);
ਖਾਲੀ sqlite3_Create_Foreign_ਕੁੰਜੀ(Parse*, ਵਿਅੰਜਕ_List*, Token*, ਵਿਅੰਜਕ_List*, ਪੂਰਨ);
ਖਾਲੀ sqlite3_Defer_Foreign_ਕੁੰਜੀ(Parse*, ਪੂਰਨ);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
  ਖਾਲੀ sqlite3AuthRead(Parse*,ਵਿਅੰਜਕ*,Schema*,SrcList*);
  ਪੂਰਨ sqlite3AuthCheck(Parse*,ਪੂਰਨ, ਅਚਲ ਅੱਖਰ*, ਅਚਲ ਅੱਖਰ*, ਅਚਲ ਅੱਖਰ*);
  ਖਾਲੀ sqlite3_Auth_Context_ਧੱਕੋ(Parse*, AuthContext*, ਅਚਲ ਅੱਖਰ*);
  ਖਾਲੀ sqlite3_Auth_Context_ਕੱਢੋ(AuthContext*);
  ਪੂਰਨ sqlite3_Auth_Read_ਖਾਨਾ(Parse*, ਅਚਲ ਅੱਖਰ *, ਅਚਲ ਅੱਖਰ *, ਪੂਰਨ);
#ਨਹੀਂਤਾਂ
# define sqlite3AuthRead(a,b,c,d)
# define sqlite3AuthCheck(a,b,c,d,e)    SQLITE_OK
# define sqlite3_Auth_Context_ਧੱਕੋ(a,b,c)
# define sqlite3_Auth_Context_ਕੱਢੋ(a)  ((ਖਾਲੀ)(a))
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3DbIsNamed(sqlite3 *db, ਪੂਰਨ iDb, ਅਚਲ ਅੱਖਰ *zName);
ਖਾਲੀ sqlite3Attach(Parse*, ਵਿਅੰਜਕ*, ਵਿਅੰਜਕ*, ਵਿਅੰਜਕ*);
ਖਾਲੀ sqlite3Detach(Parse*, ਵਿਅੰਜਕ*);
ਖਾਲੀ sqlite3FixInit(DbFixer*, Parse*, ਪੂਰਨ, ਅਚਲ ਅੱਖਰ*, ਅਚਲ Token*);
ਪੂਰਨ sqlite3FixSrcList(DbFixer*, SrcList*);
ਪੂਰਨ sqlite3FixSelect(DbFixer*, Select*);
ਪੂਰਨ sqlite3_Fix_ਵਿਅੰਜਕ(DbFixer*, ਵਿਅੰਜਕ*);
ਪੂਰਨ sqlite3_Fix_Trigger_ਕਦਮ(DbFixer*, Trigger_ਕਦਮ*);

ਪੂਰਨ sqlite3RealSameAsInt(ਦੁੱਗਣਾ,sqlite3_int64);
i64 sqlite3_Real_ਮੰਜ਼ਿਲ_I64(ਦੁੱਗਣਾ);
ਪੂਰਨ sqlite3_Int64_ਮੰਜ਼ਿਲ_ਪਾਠ(i64,ਅੱਖਰ*);
ਪੂਰਨ sqlite3AtoF(ਅਚਲ ਅੱਖਰ *z, ਦੁੱਗਣਾ*);
ਪੂਰਨ sqlite3GetInt32(ਅਚਲ ਅੱਖਰ *, ਪੂਰਨ*);
ਪੂਰਨ sqlite3GetUInt32(ਅਚਲ ਅੱਖਰ*, u32*);
ਪੂਰਨ sqlite3Atoi(ਅਚਲ ਅੱਖਰ*);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਪੂਰਨ sqlite3Utf16ByteLen(ਅਚਲ ਖਾਲੀ *p_ਅੰਕੜੇ, ਪੂਰਨ nByte, ਪੂਰਨ nChar);
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3Utf8CharLen(ਅਚਲ ਅੱਖਰ *p_ਅੰਕੜੇ, ਪੂਰਨ nByte);
u32 sqlite3Utf8Read(ਅਚਲ u8**);
ਪੂਰਨ sqlite3Utf8ReadLimited(ਅਚਲ u8*, ਪੂਰਨ, u32*);
LogEst sqlite3LogEst(u64);
LogEst sqlite3_Log_Est_ਜੋੜੋ(LogEst,LogEst);
LogEst sqlite3LogEstFromDouble(ਦੁੱਗਣਾ);
u64 sqlite3_Log_Est_ਮੰਜ਼ਿਲ_Int(LogEst);
VList *sqlite3_VList_ਜੋੜੋ(sqlite3*,VList*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ,ਪੂਰਨ);
ਅਚਲ ਅੱਖਰ *sqlite3_VList_ਅੰਕ_ਮੰਜ਼ਿਲ_Name(VList*,ਪੂਰਨ);
ਪੂਰਨ sqlite3_VList_Name_ਮੰਜ਼ਿਲ_ਅੰਕ(VList*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ);

/*
** Routines to read and write variable-length integers.  These used to
** be defined locally, but now we use the varint routines in the util.c
** file.
*/
ਪੂਰਨ sqlite3PutVarint(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*, u64);
u8 sqlite3GetVarint(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *, u64 *);
i64 sqlite3_Varint_ਮੁੱਲ(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*);
u8 sqlite3GetVarint32(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *, u32 *);
ਪੂਰਨ sqlite3VarintLen(u64 v);

/*
** The common case is for a varint to be a single byte.  They following
** macros handle the common case without a procedure call, but then call
** the procedure for larger varints.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getVarint32(A,B)  \
  (u8)((*(A)<(u8)0x80)?((B)=(u32)*(A)),1:sqlite3GetVarint32((A),(u32 *)&(B)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getVarint32NR(A,B) \
  B=(u32)*(A);ਜੇ(B>=0x80)sqlite3GetVarint32((A),(u32*)&(B))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ putVarint32(A,B)  \
  (u8)(((u32)(B)<(u32)0x80)?(*(A)=(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(B)),1:\
  sqlite3PutVarint((A),(B)))

ਅਚਲ ਅੱਖਰ *sqlite3_ਸੂਚਕਾਂਕ_Affinity_Str(sqlite3*, ਸੂਚਕਾਂਕ*);
ਅੱਖਰ *sqlite3_ਸਾਰਣੀ_Affinity_Str(sqlite3*,ਅਚਲ ਸਾਰਣੀ*);
ਖਾਲੀ sqlite3_ਸਾਰਣੀ_Affinity(Vdbe*, ਸਾਰਣੀ*, ਪੂਰਨ);
ਅੱਖਰ sqlite3CompareAffinity(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਅੱਖਰ aff2);
ਪੂਰਨ sqlite3_ਸੂਚਕਾਂਕ_Affinity_Ok(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਅੱਖਰ idx_affinity);
ਅੱਖਰ sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_Affinity(ਅਚਲ ਸਾਰਣੀ*,ਪੂਰਨ);
ਅੱਖਰ sqlite3_ਵਿਅੰਜਕ_Affinity(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਅੰਕੜੇ_Type(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ);
ਪੂਰਨ sqlite3Atoi64(ਅਚਲ ਅੱਖਰ*, i64*, ਪੂਰਨ, u8);
ਪੂਰਨ sqlite3_Dec_Or_Hex_ਮੰਜ਼ਿਲ_I64(ਅਚਲ ਅੱਖਰ*, i64*);
ਖਾਲੀ sqlite3_Error_With_ਸੁਨੇਹਾ(sqlite3*, ਪੂਰਨ, ਅਚਲ ਅੱਖਰ*,...);
ਖਾਲੀ sqlite3Error(sqlite3*,ਪੂਰਨ);
ਖਾਲੀ sqlite3ErrorClear(sqlite3*);
ਖਾਲੀ sqlite3SystemError(sqlite3*,ਪੂਰਨ);
#ਜੇ !defined(SQLITE_OMIT_BLOB_LITERAL)
ਖਾਲੀ *sqlite3_Hex_ਮੰਜ਼ਿਲ_Blob(sqlite3*, ਅਚਲ ਅੱਖਰ *z, ਪੂਰਨ n);
#ਜੇ_ਅੰਤ
u8 sqlite3_Hex_ਮੰਜ਼ਿਲ_Int(ਪੂਰਨ h);
ਪੂਰਨ sqlite3TwoPartName(Parse *, Token *, Token *, Token **);

#ਜੇ defined(SQLITE_NEED_ERR_NAME)
ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DESERIALIZE
ਪੂਰਨ sqlite3MemdbInit(ਖਾਲੀ);
ਪੂਰਨ sqlite3IsMemdb(ਅਚਲ sqlite3_vfs*);
#ਨਹੀਂਤਾਂ
# define sqlite3IsMemdb(X) 0
#ਜੇ_ਅੰਤ

ਅਚਲ ਅੱਖਰ *sqlite3ErrStr(ਪੂਰਨ);
ਪੂਰਨ sqlite3ReadSchema(Parse *pParse);
CollSeq *sqlite3_ਲੱਭੋ_Coll_Seq(sqlite3*,u8 enc, ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਦੁਆਧਾਰੀ_Compare(ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*);
#ਜੇ SQLITE_ਵਰਤਦਾ_INLINE
ਸਥਿਰ SQLITE_INLINE ਪੂਰਨ sqlite3_Is_ਦੁਆਧਾਰੀ(ਅਚਲ CollSeq *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p==0 || p->xCmp!=sqlite3_ਦੁਆਧਾਰੀ_Compare
       || strcmp(p->zName,"BINARY")==0 );
  ਵਾਪਸ p==0 || p->xCmp==sqlite3_ਦੁਆਧਾਰੀ_Compare;
}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ sqlite3_Is_ਦੁਆਧਾਰੀ(ਅਚਲ CollSeq*);
#ਜੇ_ਅੰਤ
CollSeq *sqlite3LocateCollSeq(Parse *pParse, ਅਚਲ ਅੱਖਰ*zName);
ਖਾਲੀ sqlite3_ਸੈੱਟ_ਪਾਠ_Encoding(sqlite3 *db, u8);
CollSeq *sqlite3_ਵਿਅੰਜਕ_Coll_Seq(Parse *pParse, ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ);
CollSeq *sqlite3_ਵਿਅੰਜਕ_NNColl_Seq(Parse *pParse, ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Coll_Seq_ਮੇਲ(Parse*,ਅਚਲ ਵਿਅੰਜਕ*,ਅਚਲ ਵਿਅੰਜਕ*);
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Collate_Token(Parse *pParse, ਵਿਅੰਜਕ*, ਅਚਲ Token*, ਪੂਰਨ);
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Collate_String(Parse*,ਵਿਅੰਜਕ*,ਅਚਲ ਅੱਖਰ*);
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_Skip_Collate(ਵਿਅੰਜਕ*);
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(ਵਿਅੰਜਕ*);
ਪੂਰਨ sqlite3CheckCollSeq(Parse *, CollSeq *);
ਪੂਰਨ sqlite3WritableSchema(sqlite3*);
ਪੂਰਨ sqlite3CheckObjectName(Parse*, ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*,ਅਚਲ ਅੱਖਰ*);
ਖਾਲੀ sqlite3_Vdbe_ਸੈੱਟ_Changes(sqlite3 *, i64);
ਪੂਰਨ sqlite3_ਜੋੜੋ_Int64(i64*,i64);
ਪੂਰਨ sqlite3SubInt64(i64*,i64);
ਪੂਰਨ sqlite3MulInt64(i64*,i64);
ਪੂਰਨ sqlite3AbsInt32(ਪੂਰਨ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_8_3_ਨਾਂ
ਖਾਲੀ sqlite3FileSuffix3(ਅਚਲ ਅੱਖਰ*, ਅੱਖਰ*);
#ਨਹੀਂਤਾਂ
# define sqlite3FileSuffix3(X,Y)
#ਜੇ_ਅੰਤ
u8 sqlite3GetBoolean(ਅਚਲ ਅੱਖਰ *z,u8);

ਅਚਲ ਖਾਲੀ *sqlite3_ਮੁੱਲ_ਪਾਠ_2(sqlite3_ਮੁੱਲ*, u8);
ਪੂਰਨ sqlite3_ਮੁੱਲ_Is_ਦਾ_Class(ਅਚਲ sqlite3_ਮੁੱਲ*, ਖਾਲੀ(*)(ਖਾਲੀ*));
ਪੂਰਨ sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ_2(sqlite3_ਮੁੱਲ*, u8);
ਖਾਲੀ sqlite3_ਮੁੱਲ_ਸੈੱਟ_Str(sqlite3_ਮੁੱਲ*, ਪੂਰਨ, ਅਚਲ ਖਾਲੀ *,u8,
                        ਖਾਲੀ(*)(ਖਾਲੀ*));
ਖਾਲੀ sqlite3_ਮੁੱਲ_ਸੈੱਟ_Null(sqlite3_ਮੁੱਲ*);
ਖਾਲੀ sqlite3_ਮੁੱਲ_Free(sqlite3_ਮੁੱਲ*);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_UNTESTABLE
ਖਾਲੀ sqlite3_ਨਤੀਜਾ_Int_Real(sqlite3_context*);
#ਜੇ_ਅੰਤ
sqlite3_ਮੁੱਲ *sqlite3_ਮੁੱਲ_New(sqlite3 *);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UTF16
ਅੱਖਰ *sqlite3Utf16to8(sqlite3 *, ਅਚਲ ਖਾਲੀ*, ਪੂਰਨ, u8);
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3_ਮੁੱਲ_From_ਵਿਅੰਜਕ(sqlite3 *, ਅਚਲ ਵਿਅੰਜਕ *, u8, u8, sqlite3_ਮੁੱਲ **);
ਖਾਲੀ sqlite3_ਮੁੱਲ_Apply_Affinity(sqlite3_ਮੁੱਲ *, u8, u8);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_AMALGAMATION
ਬਾਹਰੀ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ sqlite3OpcodeProperty[];
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ sqlite3_Str_ਦੁਆਧਾਰੀ[];
ਬਾਹਰੀ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ sqlite3StdTypeLen[];
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ sqlite3StdTypeAffinity[];
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3StdType[];
ਬਾਹਰੀ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ sqlite3_ਉੱਤਲਾ_ਮੰਜ਼ਿਲ_ਹੇਠਲਾ[];
ਬਾਹਰੀ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *sqlite3aLTb;
ਬਾਹਰੀ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *sqlite3aEQb;
ਬਾਹਰੀ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *sqlite3aGTb;
ਬਾਹਰੀ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ sqlite3CtypeMap[];
ਬਾਹਰੀ SQLITE_WSD ਬਣਤਰ Sqlite3Config sqlite3Config;
ਬਾਹਰੀ FuncDefHash sqlite3BuiltinFunctions;
ਬਾਹਰੀ ਅਚਲ sqlite3_str sqlite3OomStr;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WSD
ਬਾਹਰੀ ਪੂਰਨ sqlite3PendingByte;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* SQLITE_AMALGAMATION */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ VDBE_PROFILE
ਬਾਹਰੀ sqlite3_uint64 sqlite3NProfileCnt;
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3_ਜੜ੍ਹ_ਪੰਨਾ_Moved(sqlite3*, ਪੂਰਨ, Pgno, Pgno);
ਖਾਲੀ sqlite3Reindex(Parse*, Token*, Token*);
ਖਾਲੀ sqlite3AlterFunctions(ਖਾਲੀ);
ਖਾਲੀ sqlite3_Alter_Rename_ਸਾਰਣੀ(Parse*, SrcList*, Token*);
ਖਾਲੀ sqlite3_Alter_Rename_ਕਾਲਮ(Parse*, SrcList*, Token*, Token*);
ਖਾਲੀ sqlite3AlterDropConstraint(Parse*,SrcList*,Token*,Token*);
ਖਾਲੀ sqlite3_Alter_ਜੋੜੋ_Constraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *p_ਪਹਿਲਾ,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  ਅਚਲ ਅੱਖਰ *z_ਵਿਅੰਜਕ,       /* Text of CHECK expression */
  ਪੂਰਨ n_ਵਿਅੰਜਕ,               /* Size of pExpr in bytes */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ              /* The parsed CHECK expression */
);
ਖਾਲੀ sqlite3_Alter_ਸੈੱਟ_Not_Null(Parse*, SrcList*, Token*, Token*);
i64 sqlite3GetToken(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *, ਪੂਰਨ *);
ਖਾਲੀ sqlite3NestedParse(Parse*, ਅਚਲ ਅੱਖਰ*, ...);
ਖਾਲੀ sqlite3ExpirePreparedStatements(sqlite3*, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਕੋਡ_Rhs_ਦਾ_ਵਿੱਚ(Parse*, ਵਿਅੰਜਕ*, ਪੂਰਨ, ਪੂਰਨ);
ਪੂਰਨ sqlite3_ਕੋਡ_Subselect(Parse*, ਵਿਅੰਜਕ*);
ਖਾਲੀ sqlite3SelectPrep(Parse*, Select*, NameContext*);
ਪੂਰਨ sqlite3ExpandSubquery(Parse*, Src_ਵਸਤੂ*);
ਖਾਲੀ sqlite3_Select_Wrong_ਅੰਕ_Terms_Error(Parse *pParse, Select *p);
ਪੂਰਨ sqlite3_ਮੇਲ_EName(
  ਅਚਲ ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ*,
  ਅਚਲ ਅੱਖਰ*,
  ਅਚਲ ਅੱਖਰ*,
  ਅਚਲ ਅੱਖਰ*,
  ਪੂਰਨ*
);
Bitmask sqlite3_ਵਿਅੰਜਕ_ਖਾਨਾ_ਵਰਤਿਆ(ਵਿਅੰਜਕ*);
u8 sqlite3StrIHash(ਅਚਲ ਅੱਖਰ*);
ਪੂਰਨ sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(NameContext*, ਵਿਅੰਜਕ*);
ਪੂਰਨ sqlite3_Resolve_ਵਿਅੰਜਕ_List_ਨਾਂ(NameContext*, ਵਿਅੰਜਕ_List*);
ਖਾਲੀ sqlite3_Resolve_Select_ਨਾਂ(Parse*, Select*, NameContext*);
ਪੂਰਨ sqlite3ResolveSelfReference(Parse*,ਸਾਰਣੀ*,ਪੂਰਨ,ਵਿਅੰਜਕ*,ਵਿਅੰਜਕ_List*);
ਪੂਰਨ sqlite3_Resolve_ਕ੍ਰਮ_ਸਮੂਹ_By(Parse*, Select*, ਵਿਅੰਜਕ_List*, ਅਚਲ ਅੱਖਰ*);
ਖਾਲੀ sqlite3_ਕਾਲਮ_Default(Vdbe *, ਸਾਰਣੀ *, ਪੂਰਨ, ਪੂਰਨ);
ਖਾਲੀ sqlite3_Alter_ਅੰਤ_ਜੋੜੋ_ਕਾਲਮ(Parse *, Token *);
ਖਾਲੀ sqlite3_Alter_Begin_ਜੋੜੋ_ਕਾਲਮ(Parse *, SrcList *);
ਖਾਲੀ sqlite3_Alter_Drop_ਕਾਲਮ(Parse*, SrcList*, ਅਚਲ Token*);
ਅਚਲ ਖਾਲੀ *sqlite3RenameTokenMap(Parse*, ਅਚਲ ਖਾਲੀ*, ਅਚਲ Token*);
ਖਾਲੀ sqlite3RenameTokenRemap(Parse*, ਅਚਲ ਖਾਲੀ *p_ਮੰਜ਼ਿਲ, ਅਚਲ ਖਾਲੀ *pFrom);
ਖਾਲੀ sqlite3_Rename_ਵਿਅੰਜਕ_Unmap(Parse*, ਵਿਅੰਜਕ*);
ਖਾਲੀ sqlite3RenameExprlistUnmap(Parse*, ਵਿਅੰਜਕ_List*);
CollSeq *sqlite3GetCollSeq(Parse*, u8, CollSeq *, ਅਚਲ ਅੱਖਰ*);
ਅੱਖਰ sqlite3AffinityType(ਅਚਲ ਅੱਖਰ*, ਕਾਲਮ*);
ਖਾਲੀ sqlite3Analyze(Parse*, Token*, Token*);
ਪੂਰਨ sqlite3InvokeBusyHandler(BusyHandler*);
ਪੂਰਨ sqlite3_ਲੱਭੋ_Db(sqlite3*, Token*);
ਪੂਰਨ sqlite3_ਲੱਭੋ_Db_Name(sqlite3 *, ਅਚਲ ਅੱਖਰ *);
ਪੂਰਨ sqlite3_Analysis_ਲੱਦੋ(sqlite3*,ਪੂਰਨ iDB);
ਖਾਲੀ sqlite3_Delete_ਸੂਚਕਾਂਕ_Samples(sqlite3*,ਸੂਚਕਾਂਕ*);
ਖਾਲੀ sqlite3_Default_ਪੰਕਤੀ_Est(ਸੂਚਕਾਂਕ*);
ਖਾਲੀ sqlite3_ਰਜਿਸਟਰ_Like_Functions(sqlite3*, ਪੂਰਨ);
ਪੂਰਨ sqlite3IsLikeFunction(sqlite3*,ਵਿਅੰਜਕ*,ਪੂਰਨ*,ਅੱਖਰ*);
ਖਾਲੀ sqlite3SchemaClear(ਖਾਲੀ *);
Schema *sqlite3SchemaGet(sqlite3 *, Btree *);
ਪੂਰਨ sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(sqlite3 *db, Schema *);
ਕੁੰਜੀ_Info *sqlite3_ਕੁੰਜੀ_Info_Alloc(sqlite3*,ਪੂਰਨ,ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਕੁੰਜੀ_Info_Unref(ਕੁੰਜੀ_Info*);
ਕੁੰਜੀ_Info *sqlite3_ਕੁੰਜੀ_Info_Ref(ਕੁੰਜੀ_Info*);
ਕੁੰਜੀ_Info *sqlite3_ਕੁੰਜੀ_Info_ਦਾ_ਸੂਚਕਾਂਕ(Parse*, ਸੂਚਕਾਂਕ*);
ਕੁੰਜੀ_Info *sqlite3_ਕੁੰਜੀ_Info_From_ਵਿਅੰਜਕ_List(Parse*, ਵਿਅੰਜਕ_List*, ਪੂਰਨ, ਪੂਰਨ);
ਅਚਲ ਅੱਖਰ *sqlite3SelectOpName(ਪੂਰਨ);
ਪੂਰਨ sqlite3HasExplicitNulls(Parse*, ਵਿਅੰਜਕ_List*);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਪੂਰਨ sqlite3_ਕੁੰਜੀ_Info_Is_Writeable(ਕੁੰਜੀ_Info*);
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3CreateFunc(sqlite3 *, ਅਚਲ ਅੱਖਰ *, ਪੂਰਨ, ਪੂਰਨ, ਖਾਲੀ *,
  ਖਾਲੀ (*)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ **),
  ਖਾਲੀ (*)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ **),
  ਖਾਲੀ (*)(sqlite3_context*),
  ਖਾਲੀ (*)(sqlite3_context*),
  ਖਾਲੀ (*)(sqlite3_context*,ਪੂਰਨ,sqlite3_ਮੁੱਲ **),
  FuncDestructor *pDestructor
);
ਖਾਲੀ sqlite3NoopDestructor(ਖਾਲੀ*);
ਖਾਲੀ *sqlite3OomFault(sqlite3*);
ਖਾਲੀ sqlite3OomClear(sqlite3*);
ਪੂਰਨ sqlite3ApiExit(sqlite3 *db, ਪੂਰਨ);
ਪੂਰਨ sqlite3_Open_ਆਰਜ਼ੀ_Database(Parse *);

ਅੱਖਰ *sqlite3RCStrRef(ਅੱਖਰ*);
ਖਾਲੀ sqlite3RCStrUnref(ਖਾਲੀ*);
ਅੱਖਰ *sqlite3RCStrNew(u64);
ਅੱਖਰ *sqlite3RCStrResize(ਅੱਖਰ*,u64);

ਖਾਲੀ sqlite3StrAccumInit(StrAccum*, sqlite3*, ਅੱਖਰ*, ਪੂਰਨ, ਪੂਰਨ);
ਪੂਰਨ sqlite3StrAccumEnlarge(StrAccum*, i64);
ਪੂਰਨ sqlite3StrAccumEnlargeIfNeeded(StrAccum*, i64);
ਅੱਖਰ *sqlite3_Str_Accum_ਅੰਤ(StrAccum*);
ਖਾਲੀ sqlite3_Str_Accum_ਸੈੱਟ_Error(StrAccum*, u8);
ਖਾਲੀ sqlite3SelectDestInit(SelectDest*,ਪੂਰਨ,ਪੂਰਨ);
ਵਿਅੰਜਕ *sqlite3_Create_ਕਾਲਮ_ਵਿਅੰਜਕ(sqlite3 *, SrcList *, ਪੂਰਨ, ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਰਿਕਾਰਡ_Error_Byte_ਵਿੱਥ(sqlite3*,ਅਚਲ ਅੱਖਰ*);
ਖਾਲੀ sqlite3_ਰਿਕਾਰਡ_Error_ਵਿੱਥ_ਦਾ_ਵਿਅੰਜਕ(sqlite3*,ਅਚਲ ਵਿਅੰਜਕ*);

ਖਾਲੀ sqlite3BackupRestart(sqlite3_backup *);
ਖਾਲੀ sqlite3BackupUpdate(sqlite3_backup *, Pgno, ਅਚਲ u8 *);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Check_ਵਿੱਚ(Parse*, ਵਿਅੰਜਕ*);
#ਨਹੀਂਤਾਂ
# define sqlite3_ਵਿਅੰਜਕ_Check_ਵਿੱਚ(x,y) SQLITE_OK
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STAT4
ਪੂਰਨ sqlite3_Stat4_Probe_ਸੈੱਟ_ਮੁੱਲ(
    Parse*,ਸੂਚਕਾਂਕ*,Unpacked_ਰਿਕਾਰਡ**,ਵਿਅੰਜਕ*,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ*);
ਪੂਰਨ sqlite3_Stat4_ਮੁੱਲ_From_ਵਿਅੰਜਕ(Parse*, ਵਿਅੰਜਕ*, u8, sqlite3_ਮੁੱਲ**);
ਖਾਲੀ sqlite3Stat4ProbeFree(Unpacked_ਰਿਕਾਰਡ*);
ਪੂਰਨ sqlite3_Stat4_ਕਾਲਮ(sqlite3*, ਅਚਲ ਖਾਲੀ*, ਪੂਰਨ, ਪੂਰਨ, sqlite3_ਮੁੱਲ**);
ਅੱਖਰ sqlite3_ਸੂਚਕਾਂਕ_ਕਾਲਮ_Affinity(sqlite3*, ਸੂਚਕਾਂਕ*, ਪੂਰਨ);
#ਜੇ_ਅੰਤ

/*
** The interface to the LEMON-generated parser
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_AMALGAMATION
  ਖਾਲੀ *sqlite3ParserAlloc(ਖਾਲੀ*(*)(u64), Parse*);
  ਖਾਲੀ sqlite3ParserFree(ਖਾਲੀ*, ਖਾਲੀ(*)(ਖਾਲੀ*));
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3Parser(ਖਾਲੀ*, ਪੂਰਨ, Token);
ਪੂਰਨ sqlite3ParserFallback(ਪੂਰਨ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ YYTRACKMAXSTACKDEPTH
  ਪੂਰਨ sqlite3_Parser_ਢੇਰ_Peak(ਖਾਲੀ*);
#ਜੇ_ਅੰਤ

ਖਾਲੀ sqlite3_Auto_ਲੱਦੋ_Extensions(sqlite3*);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
  ਖਾਲੀ sqlite3CloseExtensions(sqlite3*);
#ਨਹੀਂਤਾਂ
# define sqlite3CloseExtensions(X)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
  ਖਾਲੀ sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(Parse *, ਪੂਰਨ, Pgno, u8, ਅਚਲ ਅੱਖਰ *);
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(v,w,x,y,z)
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  ਪੂਰਨ sqlite3Utf8To8(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
#  define sqlite3VtabClear(D,T)
#  define sqlite3VtabSync(X,Y) SQLITE_OK
#  define sqlite3VtabRollback(X)
#  define sqlite3VtabCommit(X)
#  define sqlite3_Vtab_ਵਿੱਚ_Sync(db) 0
#  define sqlite3_Vtab_ਤਾਲਾ(X)
#  define sqlite3VtabUnlock(X)
#  define sqlite3VtabModuleUnref(D,X)
#  define sqlite3VtabUnlockList(X)
#  define sqlite3VtabSavepoint(X, Y, Z) SQLITE_OK
#  define sqlite3GetVTable(X,Y)  ((VTable*)0)
#ਨਹੀਂਤਾਂ
   ਖਾਲੀ sqlite3VtabClear(sqlite3 *db, ਸਾਰਣੀ*);
   ਖਾਲੀ sqlite3VtabDisconnect(sqlite3 *db, ਸਾਰਣੀ *p);
   ਪੂਰਨ sqlite3VtabSync(sqlite3 *db, Vdbe*);
   ਪੂਰਨ sqlite3VtabRollback(sqlite3 *db);
   ਪੂਰਨ sqlite3VtabCommit(sqlite3 *db);
   ਖਾਲੀ sqlite3_Vtab_ਤਾਲਾ(VTable *);
   ਖਾਲੀ sqlite3VtabUnlock(VTable *);
   ਖਾਲੀ sqlite3VtabModuleUnref(sqlite3*,Module*);
   ਖਾਲੀ sqlite3VtabUnlockList(sqlite3*);
   ਪੂਰਨ sqlite3VtabSavepoint(sqlite3 *, ਪੂਰਨ, ਪੂਰਨ);
   ਖਾਲੀ sqlite3VtabImportErrmsg(Vdbe*, sqlite3_vtab*);
   VTable *sqlite3GetVTable(sqlite3*, ਸਾਰਣੀ*);
   Module *sqlite3VtabCreateModule(
     sqlite3*,
     ਅਚਲ ਅੱਖਰ*,
     ਅਚਲ sqlite3_module*,
     ਖਾਲੀ*,
     ਖਾਲੀ(*)(ਖਾਲੀ*)
   );
#  define sqlite3_Vtab_ਵਿੱਚ_Sync(db) ((db)->nVTrans>0 && (db)->aVTrans==0)
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3ReadOnlyShadowTables(sqlite3 *db);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਪੂਰਨ sqlite3_Shadow_ਸਾਰਣੀ_Name(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zName);
  ਪੂਰਨ sqlite3_Is_Shadow_ਸਾਰਣੀ_ਦਾ(sqlite3*,ਸਾਰਣੀ*,ਅਚਲ ਅੱਖਰ*);
  ਖਾਲੀ sqlite3_Mark_All_Shadow_Tables_ਦਾ(sqlite3*, ਸਾਰਣੀ*);
  ਖਾਲੀ sqlite3_Vtab_Eponymous_ਸਾਰਣੀ_Clear_All(sqlite3*);
#ਨਹੀਂਤਾਂ
# define sqlite3_Shadow_ਸਾਰਣੀ_Name(A,B) 0
# define sqlite3_Is_Shadow_ਸਾਰਣੀ_ਦਾ(A,B,C) 0
# define sqlite3_Mark_All_Shadow_Tables_ਦਾ(A,B)
# define sqlite3_Vtab_Eponymous_ਸਾਰਣੀ_Clear_All(X)
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3_Vtab_Eponymous_ਸਾਰਣੀ_Init(Parse*,Module*);
ਖਾਲੀ sqlite3_Vtab_Eponymous_ਸਾਰਣੀ_Clear(sqlite3*,Module*);
ਖਾਲੀ sqlite3VtabMakeWritable(Parse*,ਸਾਰਣੀ*);
ਖਾਲੀ sqlite3VtabBeginParse(Parse*, Token*, Token*, Token*, ਪੂਰਨ);
ਖਾਲੀ sqlite3_Vtab_ਅੰਤ_Parse(Parse*, Token*);
ਖਾਲੀ sqlite3VtabArgInit(Parse*);
ਖਾਲੀ sqlite3VtabArgExtend(Parse*, Token*);
ਪੂਰਨ sqlite3VtabCallCreate(sqlite3*, ਪੂਰਨ, ਅਚਲ ਅੱਖਰ *, ਅੱਖਰ **);
ਪੂਰਨ sqlite3VtabCallConnect(Parse*, ਸਾਰਣੀ*);
ਪੂਰਨ sqlite3VtabCallDestroy(sqlite3*, ਪੂਰਨ, ਅਚਲ ਅੱਖਰ *);
ਪੂਰਨ sqlite3VtabBegin(sqlite3 *, VTable *);

FuncDef *sqlite3VtabOverloadFunction(sqlite3 *,FuncDef*, ਪੂਰਨ nArg, ਵਿਅੰਜਕ*);
ਖਾਲੀ sqlite3_Vtab_ਵਰਤਦਾ_All_Schemas(Parse*);
sqlite3_int64 sqlite3_Stmt_Current_ਸਮਾਂ(sqlite3_context*);
ਪੂਰਨ sqlite3_Vdbe_Parameter_ਸੂਚਕਾਂਕ(Vdbe*, ਅਚਲ ਅੱਖਰ*, ਪੂਰਨ);
ਪੂਰਨ sqlite3TransferBindings(sqlite3_stmt *, sqlite3_stmt *);
ਖਾਲੀ sqlite3ParseObjectInit(Parse*,sqlite3*);
ਖਾਲੀ sqlite3ParseObjectReset(Parse*);
ਖਾਲੀ *sqlite3_Parser_ਜੋੜੋ_Cleanup(Parse*,ਖਾਲੀ(*)(sqlite3*,ਖਾਲੀ*),ਖਾਲੀ*);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_NORMALIZE
ਅੱਖਰ *sqlite3Normalize(Vdbe*, ਅਚਲ ਅੱਖਰ*);
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3Reprepare(Vdbe*);
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_List_Check_ਲੰਮਾਈ(Parse*, ਵਿਅੰਜਕ_List*, ਅਚਲ ਅੱਖਰ*);
CollSeq *sqlite3_ਵਿਅੰਜਕ_Compare_Coll_Seq(Parse*,ਅਚਲ ਵਿਅੰਜਕ*);
CollSeq *sqlite3_ਦੁਆਧਾਰੀ_Compare_Coll_Seq(Parse *, ਅਚਲ ਵਿਅੰਜਕ*, ਅਚਲ ਵਿਅੰਜਕ*);
ਪੂਰਨ sqlite3_ਆਰਜ਼ੀ_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ(ਅਚਲ sqlite3*);
ਅਚਲ ਅੱਖਰ *sqlite3JournalModename(ਪੂਰਨ);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  ਪੂਰਨ sqlite3Checkpoint(sqlite3*, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ*, ਪੂਰਨ*);
  ਪੂਰਨ sqlite3WalDefaultHook(ਖਾਲੀ*,sqlite3*,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CTE
  Cte *sqlite3CteNew(Parse*,Token*,ਵਿਅੰਜਕ_List*,Select*,u8);
  ਖਾਲੀ sqlite3CteDelete(sqlite3*,Cte*);
  With *sqlite3_With_ਜੋੜੋ(Parse*,With*,Cte*);
  ਖਾਲੀ sqlite3WithDelete(sqlite3*,With*);
  ਖਾਲੀ sqlite3WithDeleteGeneric(sqlite3*,ਖਾਲੀ*);
  With *sqlite3_With_ਧੱਕੋ(Parse*, With*, u8);
#ਨਹੀਂਤਾਂ
# define sqlite3CteNew(P,T,E,S)   ((ਖਾਲੀ*)0)
# define sqlite3CteDelete(D,C)
# define sqlite3_Cte_With_ਜੋੜੋ(P,W,C) ((ਖਾਲੀ*)0)
# define sqlite3WithDelete(x,y)
# define sqlite3_With_ਧੱਕੋ(x,y,z) ((ਖਾਲੀ*)0)
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UPSERT
  Upsert *sqlite3UpsertNew(sqlite3*,ਵਿਅੰਜਕ_List*,ਵਿਅੰਜਕ*,ਵਿਅੰਜਕ_List*,ਵਿਅੰਜਕ*,Upsert*);
  ਖਾਲੀ sqlite3UpsertDelete(sqlite3*,Upsert*);
  Upsert *sqlite3UpsertDup(sqlite3*,Upsert*);
  ਪੂਰਨ sqlite3_Upsert_Analyze_ਨਿਸ਼ਾਨਾ(Parse*,SrcList*,Upsert*,Upsert*);
  ਖਾਲੀ sqlite3UpsertDoUpdate(Parse*,Upsert*,ਸਾਰਣੀ*,ਸੂਚਕਾਂਕ*,ਪੂਰਨ);
  Upsert *sqlite3_Upsert_ਦਾ_ਸੂਚਕਾਂਕ(Upsert*,ਸੂਚਕਾਂਕ*);
  ਪੂਰਨ sqlite3_Upsert_ਅਗਲਾ_Is_IPK(Upsert*);
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3UpsertNew(u,v,w,x,y,z) ((Upsert*)0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3UpsertDelete(x,y)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3UpsertDup(x,y)         ((Upsert*)0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_Upsert_ਦਾ_ਸੂਚਕਾਂਕ(x,y)     ((Upsert*)0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_Upsert_ਅਗਲਾ_Is_IPK(x)     0
#ਜੇ_ਅੰਤ


/* Declarations for functions in fkey.c. All of these are replaced by
** no-op macros if OMIT_FOREIGN_KEY is defined. In this case no foreign
** key functionality is available. If OMIT_TRIGGER is defined but
** OMIT_FOREIGN_KEY is not, only some of the functions are no-oped. In
** this case foreign keys are parsed, but no other functionality is
** provided (enforcement of FK constraints requires the triggers sub-system).
*/
#ਜੇ !defined(SQLITE_OMIT_FOREIGN_ਕੁੰਜੀ) && !defined(SQLITE_OMIT_TRIGGER)
  ਖਾਲੀ sqlite3FkCheck(Parse*, ਸਾਰਣੀ*, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ*, ਪੂਰਨ);
  ਖਾਲੀ sqlite3_Fk_Drop_ਸਾਰਣੀ(Parse*, SrcList *, ਸਾਰਣੀ*);
  ਖਾਲੀ sqlite3FkActions(Parse*, ਸਾਰਣੀ*, ਵਿਅੰਜਕ_List*, ਪੂਰਨ, ਪੂਰਨ*, ਪੂਰਨ);
  ਪੂਰਨ sqlite3FkRequired(Parse*, ਸਾਰਣੀ*, ਪੂਰਨ*, ਪੂਰਨ);
  u32 sqlite3FkOldmask(Parse*, ਸਾਰਣੀ*);
  FKey *sqlite3FkReferences(ਸਾਰਣੀ *);
  ਖਾਲੀ sqlite3FkClearTriggerCache(sqlite3*,ਪੂਰਨ);
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3FkActions(a,b,c,d,e,f)
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3FkCheck(a,b,c,d,e,f)
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_Fk_Drop_ਸਾਰਣੀ(a,b,c)
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3FkOldmask(a,b)         0
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3FkRequired(a,b,c,d)    0
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3FkReferences(a)        0
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3FkClearTriggerCache(a,b)
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FOREIGN_ਕੁੰਜੀ
  ਖਾਲੀ sqlite3FkDelete(sqlite3 *, ਸਾਰਣੀ*);
  ਪੂਰਨ sqlite3_Fk_Locate_ਸੂਚਕਾਂਕ(Parse*,ਸਾਰਣੀ*,FKey*,ਸੂਚਕਾਂਕ**,ਪੂਰਨ**);
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3FkDelete(a,b)
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_Fk_Locate_ਸੂਚਕਾਂਕ(a,b,c,d,e)
#ਜੇ_ਅੰਤ


/*
** Available fault injectors.  Should be numbered beginning with 0.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FAULTINJECTOR_MALLOC     0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FAULTINJECTOR_ਗਿਣਤੀ      1

/*
** The interface to the code in fault.c used for identifying "benign"
** malloc failures. This is only present if SQLITE_UNTESTABLE
** is not defined.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_UNTESTABLE
  ਖਾਲੀ sqlite3BeginBenignMalloc(ਖਾਲੀ);
  ਖਾਲੀ sqlite3_ਸਿਰਾ_Benign_Malloc(ਖਾਲੀ);
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3BeginBenignMalloc()
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਸਿਰਾ_Benign_Malloc()
#ਜੇ_ਅੰਤ

/*
** Allowed return values from sqlite3FindInIndex()
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_ਸੂਚਕਾਂਕ_ROWID        1   /* Search the rowid of the table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_ਸੂਚਕਾਂਕ_EPH          2   /* Search an ephemeral b-tree */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_ਸੂਚਕਾਂਕ_ਸੂਚਕਾਂਕ_ASC    3   /* Existing index ASCENDING */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_ਸੂਚਕਾਂਕ_ਸੂਚਕਾਂਕ_DESC   4   /* Existing index DESCENDING */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_ਸੂਚਕਾਂਕ_NOOP         5   /* No table available. Use comparisons */
/*
** Allowed flags for the 3rd parameter to sqlite3FindInIndex().
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_ਸੂਚਕਾਂਕ_NOOP_OK     0x0001  /* OK to return IN_INDEX_NOOP */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_ਸੂਚਕਾਂਕ_MEMBERSHIP  0x0002  /* IN operator used for membership test */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਵਿੱਚ_ਸੂਚਕਾਂਕ_LOOP        0x0004  /* IN operator used as a loop */
ਪੂਰਨ sqlite3_ਲੱਭੋ_ਵਿੱਚ_ਸੂਚਕਾਂਕ(Parse *, ਵਿਅੰਜਕ *, u32, ਪੂਰਨ*, ਪੂਰਨ*, ਪੂਰਨ*);

ਪੂਰਨ sqlite3JournalOpen(sqlite3_vfs *, ਅਚਲ ਅੱਖਰ *, sqlite3_file *, ਪੂਰਨ, ਪੂਰਨ);
ਪੂਰਨ sqlite3_Journal_ਮਾਪ(sqlite3_vfs *);
#ਜੇ defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
  ਪੂਰਨ sqlite3JournalCreate(sqlite3_file *);
#ਜੇ_ਅੰਤ

ਪੂਰਨ sqlite3_Journal_Is_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ(sqlite3_file *p);
ਖਾਲੀ sqlite3MemJournalOpen(sqlite3_file *);

ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਸੈੱਟ_ਉਚਾਈ_And_Flags(Parse *pParse, ਵਿਅੰਜਕ *p);
#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
  ਪੂਰਨ sqlite3_Select_ਵਿਅੰਜਕ_ਉਚਾਈ(ਅਚਲ Select *);
  ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Check_ਉਚਾਈ(Parse*, ਪੂਰਨ);
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_Select_ਵਿਅੰਜਕ_ਉਚਾਈ(x) 0
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3_ਵਿਅੰਜਕ_Check_ਉਚਾਈ(x,y)
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3_ਵਿਅੰਜਕ_ਸੈੱਟ_Error_ਵਿੱਥ(ਵਿਅੰਜਕ*,ਪੂਰਨ);

u32 sqlite3Get4byte(ਅਚਲ u8*);
SQLITE_OPT_INLINE u64 sqlite3Get8byte(ਅਚਲ u8*);
#ਜੇ SQLITE_BYTEORDER!=4321
SQLITE_OPT_INLINE u64 sqlite3BSwap64(u64);
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3Put4byte(u8*, u32);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UNLOCK_NOTIFY
  ਖਾਲੀ sqlite3ConnectionBlocked(sqlite3 *, sqlite3 *);
  ਖਾਲੀ sqlite3ConnectionUnlocked(sqlite3 *db);
  ਖਾਲੀ sqlite3ConnectionClosed(sqlite3 *db);
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3ConnectionBlocked(x,y)
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3ConnectionUnlocked(x)
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3ConnectionClosed(x)
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਖਾਲੀ sqlite3ParserTrace(FILE*, ਅੱਖਰ *);
#ਜੇ_ਅੰਤ
#ਜੇ defined(YYCOVERAGE)
  ਪੂਰਨ sqlite3ParserCoverage(FILE*);
#ਜੇ_ਅੰਤ

/*
** If the SQLITE_ENABLE IOTRACE exists then the global variable
** sqlite3IoTrace is a pointer to a printf-like routine used to
** print I/O tracing messages.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_IOTRACE
# define IOTRACE(A)  ਜੇ( sqlite3IoTrace ){ sqlite3IoTrace A; }
  ਖਾਲੀ sqlite3VdbeIOTraceSql(Vdbe*);
SQLITE_API SQLITE_EXTERN ਖਾਲੀ (SQLITE_CDECL *sqlite3IoTrace)(ਅਚਲ ਅੱਖਰ*,...);
#ਨਹੀਂਤਾਂ
# define IOTRACE(A)
# define sqlite3VdbeIOTraceSql(X)
#ਜੇ_ਅੰਤ

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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MEMDEBUG
  ਖਾਲੀ sqlite3_Memdebug_ਸੈੱਟ_Type(ਖਾਲੀ*,u8);
  ਪੂਰਨ sqlite3MemdebugHasType(ਅਚਲ ਖਾਲੀ*,u8);
  ਪੂਰਨ sqlite3MemdebugNoType(ਅਚਲ ਖਾਲੀ*,u8);
#ਨਹੀਂਤਾਂ
# define sqlite3_Memdebug_ਸੈੱਟ_Type(X,Y)  /* no-op */
# define sqlite3MemdebugHasType(X,Y)  1
# define sqlite3MemdebugNoType(X,Y)   1
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEMTYPE_ਢੇਰ       0x01  /* General heap allocations */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEMTYPE_LOOKASIDE  0x02  /* Heap that might have been lookaside */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEMTYPE_PCACHE     0x04  /* Page cache allocations */

/*
** Threading interface
*/
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
ਪੂਰਨ sqlite3ThreadCreate(SQLiteThread**,ਖਾਲੀ*(*)(ਖਾਲੀ*),ਖਾਲੀ*);
ਪੂਰਨ sqlite3ThreadJoin(SQLiteThread*, ਖਾਲੀ**);
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)
ਪੂਰਨ sqlite3_Dbpage_ਰਜਿਸਟਰ(sqlite3*);
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)
ਪੂਰਨ sqlite3_Dbstat_ਰਜਿਸਟਰ(sqlite3*);
#ਜੇ_ਅੰਤ

ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ);
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ);
ਵਿਅੰਜਕ *sqlite3_ਸਦਿਸ਼_ਖੇਤਰ_Subexpr(ਵਿਅੰਜਕ*, ਪੂਰਨ);
ਵਿਅੰਜਕ *sqlite3_ਵਿਅੰਜਕ_For_ਸਦਿਸ਼_ਖੇਤਰ(Parse*,ਵਿਅੰਜਕ*,ਪੂਰਨ,ਪੂਰਨ);
ਖਾਲੀ sqlite3_ਸਦਿਸ਼_Error_ਸੁਨੇਹਾ(Parse*, ਵਿਅੰਜਕ*);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_COMPILEOPTION_DIAGS
ਅਚਲ ਅੱਖਰ **sqlite3CompileOptions(ਪੂਰਨ *pnOpt);
#ਜੇ_ਅੰਤ

#ਜੇ SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
ਪੂਰਨ sqlite3KvvfsInit(ਖਾਲੀ);
#ਜੇ_ਅੰਤ

#ਜੇ defined(VDBE_PROFILE) \
 || defined(SQLITE_PERFORMANCE_TRACE) \
 || defined(SQLITE_ENABLE_STMT_SCANSTATUS)
sqlite3_uint64 sqlite3Hwtime(ਖਾਲੀ);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
# define IS_STMT_SCANSTATUS(db) (db->flags & SQLITE_Stmt_ਜਾਂਚ_ਸਥਿਤੀ)
#ਨਹੀਂਤਾਂ
# define IS_STMT_SCANSTATUS(db) 0
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* SQLITEINT_H */
