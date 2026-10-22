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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITEINT_H
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITEINT_H

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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_TCLAPI
#  define SQLITE_TCLAPI
#ಆದರೆ_ಅಂತ್ಯ

/*
** Include the header file used to customize the compiler options for MSVC.
** This should be done first so that it can successfully prevent spurious
** compiler warnings due to subsequent content in this file and other files
** that are included by this file.
*/
#ಅನ್ತರ್ಭಾವ "msvc.h"

/*
** Special setup for VxWorks
*/
#ಅನ್ತರ್ಭಾವ "vxworks.h"

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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DISABLE_LFS
# define _LARGE_FILE       1
# ifndef FILE_ಸ್ಥಾನಾಂತರ_ಬಿಟ್ಟುಗಳು
#   define FILE_ಸ್ಥಾನಾಂತರ_ಬಿಟ್ಟುಗಳು 64
# endif
# define LARGEFILE_ಮೂಲ 1
#ಆದರೆ_ಅಂತ್ಯ

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
#ಆದರೆ defined(__GNUC__) && !defined(SQLITE_DISABLE_INTRINSIC)
# define GCC_ಆವೃತ್ತಿ (__GNUC__*1000000+__GNUC_MINOR__*1000+__GNUC_PATCHLEVEL__)
#ಇಲ್ಲದಿದ್ದರೆ
# define GCC_ಆವೃತ್ತಿ 0
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(_MSC_VER) && !defined(SQLITE_DISABLE_INTRINSIC)
# define MSVC_ಆವೃತ್ತಿ _MSC_VER
#ಇಲ್ಲದಿದ್ದರೆ
# define MSVC_ಆವೃತ್ತಿ 0
#ಆದರೆ_ಅಂತ್ಯ

/*
** Some C99 functions in "math.h" are only present for MSVC when its version
** is associated with Visual Studio 2013 or higher.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_HAVE_C99_MATH_FUNCS
# ಆದರೆ MSVC_ಆವೃತ್ತಿ==0 || MSVC_ಆವೃತ್ತಿ>=1800
#  define SQLITE_HAVE_C99_MATH_FUNCS (1)
# ಇಲ್ಲದಿದ್ದರೆ
#  define SQLITE_HAVE_C99_MATH_FUNCS (0)
# endif
#ಆದರೆ_ಅಂತ್ಯ

/* Needed for various definitions... */
#ಆದರೆ defined(__GNUC__) && !defined(GNU_ಮೂಲ)
# define GNU_ಮೂಲ
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__OpenBSD__) && !defined(BSD_ಮೂಲ)
# define BSD_ಮೂಲ
#ಆದರೆ_ಅಂತ್ಯ

/*
** Macro to disable warnings about missing "break" at the end of a "case".
*/
#ಆದರೆ defined(__has_attribute)
#  ಆದರೆ __has_attribute(fallthrough)
#    define deliberate_fall_through __attribute__((fallthrough));
#  endif
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(deliberate_fall_through)
#  define deliberate_fall_through
#ಆದರೆ_ಅಂತ್ಯ

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
#ಆದರೆ defined(_HAVE_MINGW_H)
# include "mingw.h"
#ಅಥವಾಆದರೆ defined(_HAVE__MINGW_H)
# include "_mingw.h"
#ಆದರೆ_ಅಂತ್ಯ

/*
** For MinGW version 4.x (and higher), check to see if the _USE_32BIT_TIME_T
** define is required to maintain binary compatibility with the MSVC runtime
** library in use (e.g. for Windows XP).
*/
#ಆದರೆ !defined(USE_32_ಬಿಟ್_ಸಮಯ_T) && !defined(USE_64_ಬಿಟ್_ಸಮಯ_T) && \
    defined(_WIN32) && !defined(_WIN64) && \
    defined(__MINGW_MAJOR_VERSION) && __MINGW_MAJOR_VERSION >= 4 && \
    defined(__MSVCRT__)
# define USE_32_ಬಿಟ್_ಸಮಯ_T
#ಆದರೆ_ಅಂತ್ಯ

/* Optionally #include a user-defined header, whereby compilation options
** may be set prior to where they take effect, but after platform setup.
** If SQLITE_CUSTOM_INCLUDE=? is defined, its value names the #include
** file.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_CUSTOM_INCLUDE
# define INC_STRINGIFY_(f) #f
# define INC_STRINGIFY(f) INC_STRINGIFY_(f)
# include INC_STRINGIFY(SQLITE_CUSTOM_INCLUDE)
#ಆದರೆ_ಅಂತ್ಯ

/* The public SQLite interface.  The _FILE_OFFSET_BITS macro must appear
** first in QNX.  Also, the _USE_32BIT_TIME_T macro must appear first for
** MinGW.
*/
#ಅನ್ತರ್ಭಾವ "sqlite3.h"

/*
** Reuse the STATIC_VFS1 for mutex access to sqlite3_temp_directory.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MUTEX_STATIC_TEMPDIR SQLITE_MUTEX_STATIC_VFS1

/*
** Include the configuration header output by 'configure' if we're using the
** autoconf-based build
*/
#ಆದರೆ defined(_HAVE_SQLITE_CONFIG_H) && !defined(SQLITECONFIG_H)
#ಅನ್ತರ್ಭಾವ "sqlite_cfg.h"
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITECONFIG_H 1
#ಆದರೆ_ಅಂತ್ಯ

#ಅನ್ತರ್ಭಾವ "sqliteLimit.h"

/* Disable nuisance warnings on Borland compilers */
#ಆದರೆ defined(__BORLANDC__)
#ಸೂಚನೆ warn -rch /* unreachable code */
#ಸೂಚನೆ warn -ccc /* Condition is always true or false */
#ಸೂಚನೆ warn -aus /* Assigned value is never used */
#ಸೂಚನೆ warn -csu /* Comparing signed and unsigned */
#ಸೂಚನೆ warn -spa /* Suspicious pointer arithmetic */
#ಆದರೆ_ಅಂತ್ಯ

/*
** A few places in the code require atomic load/store of aligned
** integer values.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __has_extension
# define __has_extension(x) 0     /* compatibility with non-clang compilers */
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ GCC_ಆವೃತ್ತಿ>=4007000 || __has_extension(c_atomic)
# define SQLITE_ATOMIC_INTRINSICS 1
# define Atomic_ತುಂಬು(PTR)       __atomic_load_n((PTR),__ATOMIC_RELAXED)
# define AtomicStore(PTR,VAL)  __atomic_store_n((PTR),(VAL),__ATOMIC_RELAXED)
#ಇಲ್ಲದಿದ್ದರೆ
# define SQLITE_ATOMIC_INTRINSICS 0
# define Atomic_ತುಂಬು(PTR)       (*(PTR))
# define AtomicStore(PTR,VAL)  (*(PTR) = (VAL))
#ಆದರೆ_ಅಂತ್ಯ

/*
** Include standard header files as necessary
*/
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_INTTYPES_H
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಆದರೆ_ಅಂತ್ಯ

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
#ಆದರೆ defined(HAVE_STDINT_H)   /* Use this case if we have ANSI headers */
# define SQLITE_INT_ಗಮ್ಯ_PTR(X)  ((ಶೂನ್ಯ*)(intptr_t)(X))
# define SQLITE_PTR_ಗಮ್ಯ_INT(X)  ((ಪೂರ್ಣಾಂಕ)(intptr_t)(X))
#ಅಥವಾಆದರೆ defined(__PTRDIFF_TYPE__)  /* This case should work for GCC */
# define SQLITE_INT_ಗಮ್ಯ_PTR(X)  ((ಶೂನ್ಯ*)(__PTRDIFF_TYPE__)(X))
# define SQLITE_PTR_ಗಮ್ಯ_INT(X)  ((ಪೂರ್ಣಾಂಕ)(__PTRDIFF_TYPE__)(X))
#ಅಥವಾಆದರೆ !defined(__GNUC__)       /* Works for compilers other than LLVM */
# define SQLITE_INT_ಗಮ್ಯ_PTR(X)  ((ಶೂನ್ಯ*)&((ಅಕ್ಷರ*)0)[X])
# define SQLITE_PTR_ಗಮ್ಯ_INT(X)  ((ಪೂರ್ಣಾಂಕ)(((ಅಕ್ಷರ*)X)-(ಅಕ್ಷರ*)0))
#ಇಲ್ಲದಿದ್ದರೆ                          /* Generates a warning - but it always works */
# define SQLITE_INT_ಗಮ್ಯ_PTR(X)  ((ಶೂನ್ಯ*)(X))
# define SQLITE_PTR_ಗಮ್ಯ_INT(X)  ((ಪೂರ್ಣಾಂಕ)(X))
#ಆದರೆ_ಅಂತ್ಯ

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
#ಆದರೆ defined(SQLITE_DISABLE_INLINE)
#  define SQLITE_NOINLINE
#  define SQLITE_INLINE
#  define SQLITE_OPT_INLINE
#  define SQLITE_ಬಳಸುತ್ತದೆ_INLINE 0
#ಅಥವಾಆದರೆ defined(__GNUC__)
#  define SQLITE_NOINLINE    __attribute__((noinline))
#  define SQLITE_INLINE      __attribute__((always_inline)) ಸಾಲೊಳಗೆ
#  define SQLITE_OPT_INLINE  __attribute__((always_inline)) ಸಾಲೊಳಗೆ
#  define SQLITE_ಬಳಸುತ್ತದೆ_INLINE 1
#ಅಥವಾಆದರೆ defined(_MSC_VER) && _MSC_VER>=1310
#  define SQLITE_NOINLINE    __declspec(noinline)
#  define SQLITE_INLINE      __forceinline
#  define SQLITE_OPT_INLINE  __forceinline
#  define SQLITE_ಬಳಸುತ್ತದೆ_INLINE 1
#ಇಲ್ಲದಿದ್ದರೆ
#  define SQLITE_NOINLINE
#  define SQLITE_INLINE
#  define SQLITE_OPT_INLINE
#  define SQLITE_ಬಳಸುತ್ತದೆ_INLINE 0
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_COVERAGE_TEST) || defined(__STRICT_ANSI__)
# undef SQLITE_INLINE
# define SQLITE_INLINE
# undef SQLITE_OPT_INLINE
# define SQLITE_OPT_INLINE
# undef SQLITE_ಬಳಸುತ್ತದೆ_INLINE
# define SQLITE_ಬಳಸುತ್ತದೆ_INLINE 0
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(SQLITE_AMALGAMATION)
# undef SQLITE_OPT_INLINE
# define SQLITE_OPT_INLINE
#ಆದರೆ_ಅಂತ್ಯ

/*
** Make sure that the compiler intrinsics we desire are enabled when
** compiling with an appropriate version of MSVC unless prevented by
** the SQLITE_DISABLE_INTRINSIC define.
*/
#ಆದರೆ !defined(SQLITE_DISABLE_INTRINSIC)
#  ಆದರೆ defined(_MSC_VER) && _MSC_VER>=1400
#    ಆದರೆ !defined(_WIN32_WCE)
#      include <intrin.h>
#      pragma intrinsic(_byteswap_ushort)
#      pragma intrinsic(_byteswap_ulong)
#      pragma intrinsic(_byteswap_uint64)
#      pragma intrinsic(_ReadWriteBarrier)
#    ಇಲ್ಲದಿದ್ದರೆ
#      include <cmnintrin.h>
#    endif
#  endif
#ಆದರೆ_ಅಂತ್ಯ

/*
** Enable SQLITE_USE_SEH by default on MSVC builds.  Only omit
** SEH support if the -DSQLITE_OMIT_SEH option is given.
*/
#ಆದರೆ defined(_MSC_VER) && !defined(SQLITE_OMIT_SEH)
# define SQLITE_USE_SEH 1
#ಇಲ್ಲದಿದ್ದರೆ
# undef SQLITE_USE_SEH
#ಆದರೆ_ಅಂತ್ಯ

/*
** Enable SQLITE_DIRECT_OVERFLOW_READ, unless the build explicitly
** disables it using -DSQLITE_DIRECT_OVERFLOW_READ=0
*/
#ಆದರೆ defined(SQLITE_ನೇರ_ಅತಿಪ್ರವಾಹ_READ) && SQLITE_ನೇರ_ಅತಿಪ್ರವಾಹ_READ+1==1
  /* Disable if -DSQLITE_DIRECT_OVERFLOW_READ=0 */
# undef SQLITE_ನೇರ_ಅತಿಪ್ರವಾಹ_READ
#ಇಲ್ಲದಿದ್ದರೆ
  /* In all other cases, enable */
# define SQLITE_ನೇರ_ಅತಿಪ್ರವಾಹ_READ 1
#ಆದರೆ_ಅಂತ್ಯ


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
#ಆದರೆ !defined(SQLITE_THREADSAFE)
# ಆದರೆ defined(THREADSAFE)
#   define SQLITE_THREADSAFE THREADSAFE
# ಇಲ್ಲದಿದ್ದರೆ
#   define SQLITE_THREADSAFE 1 /* IMP: R-07272-22309 */
# endif
#ಆದರೆ_ಅಂತ್ಯ

/*
** Powersafe overwrite is on by default.  But can be turned off using
** the -DSQLITE_POWERSAFE_OVERWRITE=0 command-line option.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_POWERSAFE_OVERWRITE
# define SQLITE_POWERSAFE_OVERWRITE 1
#ಆದರೆ_ಅಂತ್ಯ

/*
** EVIDENCE-OF: R-25715-37072 Memory allocation statistics are enabled by
** default unless SQLite is compiled with SQLITE_DEFAULT_MEMSTATUS=0 in
** which case memory allocation statistics are disabled by default.
*/
#ಆದರೆ !defined(SQLITE_DEFAULT_MEMSTATUS)
# define SQLITE_DEFAULT_MEMSTATUS 1
#ಆದರೆ_ಅಂತ್ಯ

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
#ಆದರೆ defined(SQLITE_SYSTEM_MALLOC) \
  + defined(SQLITE_WIN32_MALLOC) \
  + defined(SQLITE_ZERO_MALLOC) \
  + defined(SQLITE_MEMDEBUG)>1
# error "Two or ಇನ್ನಷ್ಟು ದ the following compile-ಸಮಯ configuration options\
 are defined but at most one is allowed:\
 SQLITE_SYSTEM_MALLOC, SQLITE_WIN32_MALLOC, SQLITE_MEMDEBUG,\
 SQLITE_ZERO_MALLOC"
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_SYSTEM_MALLOC) \
  + defined(SQLITE_WIN32_MALLOC) \
  + defined(SQLITE_ZERO_MALLOC) \
  + defined(SQLITE_MEMDEBUG)==0
# define SQLITE_SYSTEM_MALLOC 1
#ಆದರೆ_ಅಂತ್ಯ

/*
** If SQLITE_MALLOC_SOFT_LIMIT is not zero, then try to keep the
** sizes of memory allocations below this value where possible.
*/
#ಆದರೆ !defined(SQLITE_MALLOC_SOFT_ಎಲ್ಲೆ)
# define SQLITE_MALLOC_SOFT_ಎಲ್ಲೆ 1024
#ಆದರೆ_ಅಂತ್ಯ

/*
** We need to define _XOPEN_SOURCE as follows in order to enable
** recursive mutexes on most Unix systems and fchmod() on OpenBSD.
** But _XOPEN_SOURCE define causes problems for Mac OS X, so omit
** it.
*/
#ಆದರೆ !defined(XOPEN_ಮೂಲ) && !defined(__DARWIN__) && !defined(__APPLE__)
#  define XOPEN_ಮೂಲ 600
#ಆದರೆ_ಅಂತ್ಯ

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
#ಆದರೆ !defined(NDEBUG) && !defined(SQLITE_DEBUG)
# define NDEBUG 1
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(NDEBUG) && defined(SQLITE_DEBUG)
# undef NDEBUG
#ಆದರೆ_ಅಂತ್ಯ

/*
** Enable SQLITE_ENABLE_EXPLAIN_COMMENTS if SQLITE_DEBUG is turned on.
*/
#ಆದರೆ !defined(SQLITE_ENABLE_EXPLAIN_COMMENTS) && defined(SQLITE_DEBUG)
# define SQLITE_ENABLE_EXPLAIN_COMMENTS 1
#ಆದರೆ_ಅಂತ್ಯ

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
#ಆದರೆ defined(SQLITE_MUTATION_TEST)
# define testcase(X)
#ಅಥವಾಆದರೆ defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
# ifndef SQLITE_AMALGAMATION
    ಬಾಹ್ಯ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ sqlite3CoverageCounter;
# endif
# define testcase(X)  ಆದರೆ( X ){ sqlite3CoverageCounter += (ಚಿಹ್ನೆರಹಿತ)__LINE__; }
#ಇಲ್ಲದಿದ್ದರೆ
# define testcase(X)
#ಆದರೆ_ಅಂತ್ಯ

/*
** The TESTONLY macro is used to enclose variable declarations or
** other bits of code that are needed to support the arguments
** within testcase() and assert() macros.
*/
#ಆದರೆ !defined(NDEBUG) || defined(SQLITE_COVERAGE_TEST)
# define TESTONLY(X)  X
#ಇಲ್ಲದಿದ್ದರೆ
# define TESTONLY(X)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Sometimes we need a small amount of code such as a variable initialization
** to setup for a later assert() statement.  We do not want this code to
** appear when assert() is disabled.  The following macro is therefore
** used to contain that setup code.  The "VVA" acronym stands for
** "Verification, Validation, and Accreditation".  In other words, the
** code within VVA_ONLY() will only run during verification processes.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
# define VVA_ONLY(X)  X
#ಇಲ್ಲದಿದ್ದರೆ
# define VVA_ONLY(X)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Disable ALWAYS() and NEVER() (make them pass-throughs) for coverage
** and mutation testing
*/
#ಆದರೆ defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_MUTATION_TEST)
# define SQLITE_OMIT_AUXILIARY_SAFETY_CHECKS  1
#ಆದರೆ_ಅಂತ್ಯ

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
#ಆದರೆ defined(SQLITE_OMIT_AUXILIARY_SAFETY_CHECKS)
# define ALWAYS(X)      (1)
# define NEVER(X)       (0)
#ಅಥವಾಆದರೆ !defined(NDEBUG)
# define ALWAYS(X)      ((X)?1:(ಪ್ರತಿಜ್ಞಾ(0),0))
# define NEVER(X)       ((X)?(ಪ್ರತಿಜ್ಞಾ(0),1):0)
#ಇಲ್ಲದಿದ್ದರೆ
# define ALWAYS(X)      (X)
# define NEVER(X)       (X)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Some conditionals are optimizations only.  In other words, if the
** conditionals are replaced with a constant 1 (true) or 0 (false) then
** the correct answer is still obtained, though perhaps not as quickly.
**
** The following macros mark these optimizations conditionals.
*/
#ಆದರೆ defined(SQLITE_MUTATION_TEST)
# define OK_IF_ALWAYS_TRUE(X)  (1)
# define OK_IF_ALWAYS_FALSE(X) (0)
#ಇಲ್ಲದಿದ್ದರೆ
# define OK_IF_ALWAYS_TRUE(X)  (X)
# define OK_IF_ALWAYS_FALSE(X) (X)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Some malloc failures are only possible if SQLITE_TEST_REALLOC_STRESS is
** defined.  We need to defend against those failures when testing with
** SQLITE_TEST_REALLOC_STRESS, but we don't want the unreachable branches
** during a normal build.  The following macro can be used to disable tests
** that are always false except when SQLITE_TEST_REALLOC_STRESS is set.
*/
#ಆದರೆ defined(SQLITE_TEST_REALLOC_STRESS)
# define ONLY_IF_REALLOC_STRESS(X)  (X)
#ಅಥವಾಆದರೆ !defined(NDEBUG)
# define ONLY_IF_REALLOC_STRESS(X)  ((X)?(ಪ್ರತಿಜ್ಞಾ(0),1):0)
#ಇಲ್ಲದಿದ್ದರೆ
# define ONLY_IF_REALLOC_STRESS(X)  (0)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Declarations used for tracing the operating system interfaces.
*/
#ಆದರೆ defined(SQLITE_FORCE_OS_TRACE) || defined(SQLITE_TEST) || \
    (defined(SQLITE_DEBUG) && SQLITE_OS_WIN)
  ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3OSTrace;
# define OSTRACE(X)          ಆದರೆ( sqlite3OSTrace ) sqlite3DebugPrintf X
# define SQLITE_HAVE_OS_TRACE
#ಇಲ್ಲದಿದ್ದರೆ
# define OSTRACE(X)
# undef  SQLITE_HAVE_OS_TRACE
#ಆದರೆ_ಅಂತ್ಯ

/*
** Is the sqlite3ErrName() function needed in the build?  Currently,
** it is needed by "mutex_w32.c" (when debugging), "os_win.c" (when
** OSTRACE is enabled), and by several "test*.c" files (which are
** compiled using SQLITE_TEST).
*/
#ಆದರೆ defined(SQLITE_HAVE_OS_TRACE) || defined(SQLITE_TEST) || \
    (defined(SQLITE_DEBUG) && SQLITE_OS_WIN)
# define SQLITE_NEED_ERR_NAME
#ಇಲ್ಲದಿದ್ದರೆ
# undef  SQLITE_NEED_ERR_NAME
#ಆದರೆ_ಅಂತ್ಯ

/*
** SQLITE_ENABLE_EXPLAIN_COMMENTS is incompatible with SQLITE_OMIT_EXPLAIN
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_EXPLAIN
# undef SQLITE_ENABLE_EXPLAIN_COMMENTS
#ಆದರೆ_ಅಂತ್ಯ

/*
** SQLITE_OMIT_VIRTUALTABLE implies SQLITE_OMIT_ALTERTABLE
*/
#ಆದರೆ defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_ALTERTABLE)
# define SQLITE_OMIT_ALTERTABLE
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_DIGIT_SEPARATOR '_'

/*
** Return true (non-zero) if the input is an integer that is too large
** to fit in 32-bits.  This macro is used inside of various testcase()
** macros to verify that we have tested SQLite for large-file support.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು IS_BIG_INT(X)  (((X)&~(i64)0xffffffff)!=0)

/*
** The macro unlikely() is a hint that surrounds a boolean
** expression that is usually false.  Macro likely() surrounds
** a boolean expression that is usually true.  These hints could,
** in theory, be used by the compiler to generate better code, but
** currently they are just comments for human readers.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು likely(X)    (X)
#ವ್ಯಾಖ್ಯಾನಿಸು unlikely(X)  (X)

#ಅನ್ತರ್ಭಾವ "hash.h"
#ಅನ್ತರ್ಭಾವ "parse.h"
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>

/*
** Use a macro to replace memcpy() if compiled with SQLITE_INLINE_MEMCPY.
** This allows better measurements of where memcpy() is used when running
** cachegrind.  But this macro version of memcpy() is very slow so it
** should not be used in production.  This is a performance measurement
** hack only.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_INLINE_MEMCPY
# define ಸ್ಮರಣೆ_ನಕಲು(D,S,N) {ಅಕ್ಷರ*xxd=(ಅಕ್ಷರ*)(D);ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*xxs=(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)(S);\
                        ಪೂರ್ಣಾಂಕ xxn=(N);ಆಗ(xxn-->0)*(xxd++)=*(xxs++);}
#ಆದರೆ_ಅಂತ್ಯ

/*
** If compiling for a processor that lacks floating point support,
** substitute integer for floating-point
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
# define ದ್ವಿಗುಣ sqlite_int64
# define ದಶಮಾಂಶ sqlite_int64
# define fabs(X) ((X)<0?-(X):(X))
# define sqlite3_Is_ಅತಿಪ್ರವಾಹ(X) 0
# define INFINITY (9223372036854775807LL)
# ifndef SQLITE_BIG_DBL
#   define SQLITE_BIG_DBL (((sqlite3_int64)1)<<50)
# endif
# define SQLITE_OMIT_DATETIME_FUNCS 1
# define SQLITE_OMIT_TRACE 1
# undef SQLITE_MIXED_ENDIAN_64_ಬಿಟ್_FLOAT
# undef SQLITE_HAVE_ISNAN
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_BIG_DBL
# define SQLITE_BIG_DBL (1e99)
#ಆದರೆ_ಅಂತ್ಯ

/*
** OMIT_TEMPDB is set to 1 if SQLITE_OMIT_TEMPDB is defined, or 0
** afterward. Having this macro allows us to cause the C compiler
** to omit code used by TEMP tables without messy #ifndef statements.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_TEMPDB
#ವ್ಯಾಖ್ಯಾನಿಸು OMIT_TEMPDB 1
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು OMIT_TEMPDB 0
#ಆದರೆ_ಅಂತ್ಯ

/*
** The "file format" number is an integer that is incremented whenever
** the VDBE-level file format changes.  The following macros define the
** the default file format for new databases and the maximum file format
** that the library can read.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MAX_FILE_FORMAT 4
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEFAULT_FILE_FORMAT
# define SQLITE_DEFAULT_FILE_FORMAT 4
#ಆದರೆ_ಅಂತ್ಯ

/*
** Determine whether triggers are recursive by default.  This can be
** changed at run-time using a pragma.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEFAULT_RECURSIVE_TRIGGERS
# define SQLITE_DEFAULT_RECURSIVE_TRIGGERS 0
#ಆದರೆ_ಅಂತ್ಯ

/*
** Provide a default value for SQLITE_TEMP_STORE in case it is not specified
** on the command-line
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ತಾತ್ಕಾಲಿಕ_STORE
# define SQLITE_ತಾತ್ಕಾಲಿಕ_STORE 1
#ಆದರೆ_ಅಂತ್ಯ

/*
** If no value has been provided for SQLITE_MAX_WORKER_THREADS, or if
** SQLITE_TEMP_STORE is set to 3 (never use temporary files), set it
** to zero.
*/
#ಆದರೆ SQLITE_ತಾತ್ಕಾಲಿಕ_STORE==3 || SQLITE_THREADSAFE==0
# undef SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು
# define SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು
# define SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು 8
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEFAULT_ಕೆಲಸಗಾರ_ಎಳೆಗಳು
# define SQLITE_DEFAULT_ಕೆಲಸಗಾರ_ಎಳೆಗಳು 0
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_DEFAULT_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು
# undef SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು
# define SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು SQLITE_DEFAULT_ಕೆಲಸಗಾರ_ಎಳೆಗಳು
#ಆದರೆ_ಅಂತ್ಯ

/*
** The default initial allocation for the pagecache when using separate
** pagecaches for each database connection.  A positive number is the
** number of pages.  A negative number N translations means that a buffer
** of -1024*N bytes is allocated and used for as many pages as it will hold.
**
** The default value of "20" was chosen to minimize the run-time of the
** speedtest1 test program with options: --shrink-memory --reprepare
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEFAULT_PCACHE_INITSZ
# define SQLITE_DEFAULT_PCACHE_INITSZ 20
#ಆದರೆ_ಅಂತ್ಯ

/*
** Default value for the SQLITE_CONFIG_SORTERREF_SIZE option.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEFAULT_SORTERREF_ಗಾತ್ರ
# define SQLITE_DEFAULT_SORTERREF_ಗಾತ್ರ 0x7fffffff
#ಆದರೆ_ಅಂತ್ಯ

/*
** The compile-time options SQLITE_MMAP_READWRITE and
** SQLITE_ENABLE_BATCH_ATOMIC_WRITE are not compatible with one another.
** You must choose one or the other (or neither) but not both.
*/
#ಆದರೆ defined(SQLITE_MMAP_READWRITE) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
#ದೋಷ Cannot use both SQLITE_MMAP_READWRITE and SQLITE_ENABLE_BATCH_ATOMIC_WRITE
#ಆದರೆ_ಅಂತ್ಯ

/*
** GCC does not define the offsetof() macro so we'll have to do it
** ourselves.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ offsetof
# define offsetof(ST,M) ((size_t)((ಅಕ್ಷರ*)&((ST*)0)->M - (ಅಕ್ಷರ*)0))
#ಆದರೆ_ಅಂತ್ಯ

/*
** sizeof64() is like sizeof(), but always returns a 64-bit value, even
** on 32-bit builds. This can help to avoid overflow by ensuring 64-bit
** arithmetic is used consistently in both 32-bit and 64-bit builds.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು sizeof64(X) ((sqlite3_int64)ಗಾತ್ರ(X))

/*
** Work around C99 "flex-array" syntax for pre-C99 compilers, so as
** to avoid complaints from -fsanitize=strict-bounds.
*/
#ಆದರೆ defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define FLEXARRAY
#ಇಲ್ಲದಿದ್ದರೆ
# define FLEXARRAY 1
#ಆದರೆ_ಅಂತ್ಯ

/*
** Macros to compute minimum and maximum of two numbers.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ MIN
# define MIN(A,B) ((A)<(B)?(A):(B))
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ MAX
# define MAX(A,B) ((A)>(B)?(A):(B))
#ಆದರೆ_ಅಂತ್ಯ

/*
** Swap two objects of type TYPE.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಅದಲುಬದಲು(TYPE,A,B) {TYPE t=A; A=B; B=t;}

/*
** Check to see if this machine uses EBCDIC.  (Yes, believe it or
** not, there are still machines out there that use EBCDIC.)
*/
#ಆದರೆ 'A' == '\301'
# define SQLITE_EBCDIC 1
#ಇಲ್ಲದಿದ್ದರೆ
# define SQLITE_ASCII 1
#ಆದರೆ_ಅಂತ್ಯ

/*
** Integers of known sizes.  These typedefs might change for architectures
** where the sizes very.  Preprocessor macros are available so that the
** types can be conveniently redefined at compile-type.  Like this:
**
**         cc '-DUINTPTR_TYPE=long long int' ...
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ UINT32_TYPE
# ifdef HAVE_UINT32_T
#  define UINT32_TYPE uint32_t
# ಇಲ್ಲದಿದ್ದರೆ
#  define UINT32_TYPE ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ
# endif
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ UINT16_TYPE
# ifdef HAVE_UINT16_T
#  define UINT16_TYPE uint16_t
# ಇಲ್ಲದಿದ್ದರೆ
#  define UINT16_TYPE ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ ಪೂರ್ಣಾಂಕ
# endif
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ INT16_TYPE
# ifdef HAVE_INT16_T
#  define INT16_TYPE int16_t
# ಇಲ್ಲದಿದ್ದರೆ
#  define INT16_TYPE ಗಿಡ್ಡ ಪೂರ್ಣಾಂಕ
# endif
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ UINT8_TYPE
# ifdef HAVE_UINT8_T
#  define UINT8_TYPE uint8_t
# ಇಲ್ಲದಿದ್ದರೆ
#  define UINT8_TYPE ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ
# endif
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ INT8_TYPE
# ifdef HAVE_INT8_T
#  define INT8_TYPE int8_t
# ಇಲ್ಲದಿದ್ದರೆ
#  define INT8_TYPE ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ
# endif
#ಆದರೆ_ಅಂತ್ಯ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ sqlite_int64 i64;          /* 8-byte signed integer */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ sqlite_uint64 u64;         /* 8-byte unsigned integer */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ UINT32_TYPE u32;           /* 4-byte unsigned integer */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ UINT16_TYPE u16;           /* 2-byte unsigned integer */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ INT16_TYPE i16;            /* 2-byte signed integer */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ UINT8_TYPE u8;             /* 1-byte unsigned integer */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ INT8_TYPE i8;              /* 1-byte signed integer */

/* A bitfield type for use inside of structures.  Always follow with :N where
** N is the number of bits.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ bft;  /* Bit Field Type */

/*
** SQLITE_MAX_U32 is a u64 constant that is the maximum u64 value
** that can be stored in a u32 without loss of data.  The value
** is 0x00000000ffffffff.  But because of quirks of some compilers, we
** have to specify the value in the less intuitive manner shown:
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MAX_U32  ((((u64)1)<<32)-1)

/*
** The datatype used to store estimates of the number of rows in a
** table or index.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ u64 tRowcnt;

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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ INT16_TYPE LogEst;
#ವ್ಯಾಖ್ಯಾನಿಸು LOGEST_MIN (-32768)
#ವ್ಯಾಖ್ಯಾನಿಸು LOGEST_MAX (32767)

/*
** Set the SQLITE_PTRSIZE macro to the number of bytes in a pointer
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_PTRSIZE
# ಆದರೆ defined(__SIZEOF_POINTER__)
#   define SQLITE_PTRSIZE __SIZEOF_POINTER__
# elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
       defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
      (defined(__APPLE__) && defined(__ppc__)) ||                         \
      (defined(__TOS_AIX__) && !defined(__64BIT__))
#   define SQLITE_PTRSIZE 4
# ಇಲ್ಲದಿದ್ದರೆ
#   define SQLITE_PTRSIZE 8
# endif
#ಆದರೆ_ಅಂತ್ಯ

/* The uptr type is an unsigned integer large enough to hold a pointer
*/
#ಆದರೆ defined(HAVE_STDINT_H)
  ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ uintptr_t uptr;
#ಅಥವಾಆದರೆ SQLITE_PTRSIZE==4
  ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ u32 uptr;
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ u64 uptr;
#ಆದರೆ_ಅಂತ್ಯ

/*
** The SQLITE_WITHIN(P,S,E) macro checks to see if pointer P points to
** something between S (inclusive) and E (exclusive).
**
** In other words, S is a buffer and E is a pointer to the first byte after
** the end of buffer S.  This macro returns true if P points to something
** contained within the buffer S.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_WITHIN(P,S,E)   (((uptr)(P)>=(uptr)(S))&&((uptr)(P)<(uptr)(E)))

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
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಅತಿಪ್ರವಾಹ(P,S,E) (((uptr)(S)<(uptr)(P))&&((uptr)(E)>(uptr)(P)))

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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_BYTEORDER  /* Replicate changes at tag-20230904a */
# ಆದರೆ defined(__BYTE_ORDER__) && __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
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
# ಇಲ್ಲದಿದ್ದರೆ
#   define SQLITE_BYTEORDER 0
# endif
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_BYTEORDER==4321
# define SQLITE_BIGENDIAN    1
# define SQLITE_LITTLEENDIAN 0
# define SQLITE_UTF16NATIVE  SQLITE_UTF16BE
#ಅಥವಾಆದರೆ SQLITE_BYTEORDER==1234
# define SQLITE_BIGENDIAN    0
# define SQLITE_LITTLEENDIAN 1
# define SQLITE_UTF16NATIVE  SQLITE_UTF16LE
#ಇಲ್ಲದಿದ್ದರೆ
# ifdef SQLITE_AMALGAMATION
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ sqlite3one = 1;
# ಇಲ್ಲದಿದ್ದರೆ
  ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ sqlite3one;
# endif
# define SQLITE_BIGENDIAN    (*(ಅಕ್ಷರ *)(&sqlite3one)==0)
# define SQLITE_LITTLEENDIAN (*(ಅಕ್ಷರ *)(&sqlite3one)==1)
# define SQLITE_UTF16NATIVE  (SQLITE_BIGENDIAN?SQLITE_UTF16BE:SQLITE_UTF16LE)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Constants for the largest and smallest possible 64-bit signed integers.
** These macros are designed to work correctly on both 32-bit and 64-bit
** compilers.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಅತಿದೊಡ್ಡ_INT64  (0xffffffff|(((i64)0x7fffffff)<<32))
#ವ್ಯಾಖ್ಯಾನಿಸು ಅತಿದೊಡ್ಡ_UINT64 (0xffffffff|(((u64)0xffffffff)<<32))
#ವ್ಯಾಖ್ಯಾನಿಸು ಅತಿಚಿಕ್ಕ_INT64 (((i64)-1) - ಅತಿದೊಡ್ಡ_INT64)

/*
** Macro SMXV(n) return the maximum value that can be held in variable n,
** assuming n is a signed integer type.  UMXV(n) is similar for unsigned
** integer types.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SMXV(n) ((((i64)1)<<(ಗಾತ್ರ(n)*8-1))-1)
#ವ್ಯಾಖ್ಯಾನಿಸು UMXV(n) ((((i64)1)<<(ಗಾತ್ರ(n)*8))-1)

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
#ವ್ಯಾಖ್ಯಾನಿಸು ROUND8(x)     (((x)+7)&~7)
#ಆದರೆ SQLITE_PTRSIZE==8
# define ROUND8P(x)   (x)
#ಇಲ್ಲದಿದ್ದರೆ
# define ROUND8P(x)   (((x)+7)&~7)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Round down to the nearest multiple of 8
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ROUNDDOWN8(x) ((x)&~7)

/*
** Assert that the pointer X is aligned to an 8-byte boundary.  This
** macro is used only within assert() to verify that the code gets
** all alignment restrictions correct.
**
** Except, if SQLITE_4_BYTE_ALIGNED_MALLOC is defined, then the
** underlying malloc() implementation might return us 4-byte aligned
** pointers.  In that case, only verify 4-byte alignment.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_4_BYTE_ALIGNED_MALLOC
# define EIGHT_BYTE_ALIGNMENT(X)   ((((uptr)(X) - (uptr)0)&3)==0)
#ಇಲ್ಲದಿದ್ದರೆ
# define EIGHT_BYTE_ALIGNMENT(X)   ((((uptr)(X) - (uptr)0)&7)==0)
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು TWO_BYTE_ALIGNMENT(X)      ((((uptr)(X) - (uptr)0)&1)==0)

/*
** Disable MMAP on platforms where it is known to not work
*/
#ಆದರೆ defined(__OpenBSD__) || defined(__QNXNTO__)
# undef SQLITE_MAX_MMAP_ಗಾತ್ರ
# define SQLITE_MAX_MMAP_ಗಾತ್ರ 0
#ಆದರೆ_ಅಂತ್ಯ

/*
** Default maximum size of memory used by memory-mapped I/O in the VFS
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
# include <TargetConditionals.h>
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ
# ಆದರೆ defined(__linux__) \
  || defined(_WIN32) \
  || (defined(__APPLE__) && defined(__MACH__)) \
  || defined(__sun) \
  || defined(__FreeBSD__) \
  || defined(__DragonFly__)
#   define SQLITE_MAX_MMAP_ಗಾತ್ರ 0x7fff0000  /* 2147418112 */
# ಇಲ್ಲದಿದ್ದರೆ
#   define SQLITE_MAX_MMAP_ಗಾತ್ರ 0
# endif
#ಆದರೆ_ಅಂತ್ಯ

/*
** The default MMAP_SIZE is zero on all platforms.  Or, even if a larger
** default MMAP_SIZE is specified at compile-time, make sure that it does
** not exceed the maximum mmap size.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEFAULT_MMAP_ಗಾತ್ರ
# define SQLITE_DEFAULT_MMAP_ಗಾತ್ರ 0
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ SQLITE_DEFAULT_MMAP_ಗಾತ್ರ>SQLITE_MAX_MMAP_ಗಾತ್ರ
# undef SQLITE_DEFAULT_MMAP_ಗಾತ್ರ
# define SQLITE_DEFAULT_MMAP_ಗಾತ್ರ SQLITE_MAX_MMAP_ಗಾತ್ರ
#ಆದರೆ_ಅಂತ್ಯ

/*
** TREETRACE_ENABLED will be either 1 or 0 depending on whether or not
** the Abstract Syntax Tree tracing logic is turned on.
*/
#ಆದರೆ !defined(SQLITE_AMALGAMATION)
ಬಾಹ್ಯ u32 sqlite3_ವೃಕ್ಷ_Trace;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_DEBUG) \
    && (defined(SQLITE_TEST) || defined(SQLITE_ENABLE_SELECTTRACE) \
                             || defined(SQLITE_ENABLE_TREETRACE))
# define TREETRACE_ENABLED 1
# define TREETRACE(K,P,S,X)  \
  ಆದರೆ(sqlite3_ವೃಕ್ಷ_Trace&(K))   \
    sqlite3DebugPrintf("%u/%d/%p: ",(S)->selId,(P)->addrExplain,(S)),\
    sqlite3DebugPrintf X
#ಇಲ್ಲದಿದ್ದರೆ
# define TREETRACE(K,P,S,X)
# define TREETRACE_ENABLED 0
#ಆದರೆ_ಅಂತ್ಯ

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
ಬಾಹ್ಯ u32 sqlite3WhereTrace;
#ಆದರೆ defined(SQLITE_DEBUG) \
    && (defined(SQLITE_TEST) || defined(SQLITE_ENABLE_WHERETRACE))
# define WHERETRACE(K,X)  ಆದರೆ(sqlite3WhereTrace&(K)) sqlite3DebugPrintf X
# define WHERETRACE_ENABLED 1
#ಇಲ್ಲದಿದ್ದರೆ
# define WHERETRACE(K,X)
#ಆದರೆ_ಅಂತ್ಯ

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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ BusyHandler BusyHandler;
ರಚನೆ BusyHandler {
  ಪೂರ್ಣಾಂಕ (*xBusyHandler)(ಶೂನ್ಯ *,ಪೂರ್ಣಾಂಕ);  /* The busy callback */
  ಶೂನ್ಯ *pBusyArg;                   /* First arg to busy callback */
  ಪೂರ್ಣಾಂಕ nBusy;                        /* Incremented with each busy call */
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
#ವ್ಯಾಖ್ಯಾನಿಸು LEGACY_SCHEMA_ಕೋಷ್ಟಕ          "sqlite_master"
#ವ್ಯಾಖ್ಯಾನಿಸು LEGACY_ತಾತ್ಕಾಲಿಕ_SCHEMA_ಕೋಷ್ಟಕ     "sqlite_temp_master"
#ವ್ಯಾಖ್ಯಾನಿಸು PREFERRED_SCHEMA_ಕೋಷ್ಟಕ       "sqlite_schema"
#ವ್ಯಾಖ್ಯಾನಿಸು PREFERRED_ತಾತ್ಕಾಲಿಕ_SCHEMA_ಕೋಷ್ಟಕ  "sqlite_temp_schema"


/*
** The root-page of the schema table.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SCHEMA_ಬೇರು    1

/*
** The name of the schema table.  The name is different for TEMP.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SCHEMA_ಕೋಷ್ಟಕ(x) \
    ((!OMIT_TEMPDB)&&(x==1)?LEGACY_ತಾತ್ಕಾಲಿಕ_SCHEMA_ಕೋಷ್ಟಕ:LEGACY_SCHEMA_ಕೋಷ್ಟಕ)

/*
** A convenience macro that returns the number of elements in
** an array.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು Array_ಗಾತ್ರ(X)    ((ಪೂರ್ಣಾಂಕ)(ಗಾತ್ರ(X)/ಗಾತ್ರ(X[0])))

/*
** Determine if the argument is a power of two
*/
#ವ್ಯಾಖ್ಯಾನಿಸು Is_Power_ದ_Two(X) (((X)&((X)-1))==0)

/*
** The following value as a destructor means to use sqlite3DbFree().
** The sqlite3DbFree() routine requires two parameters instead of the
** one parameter that destructors normally want.  So we have to introduce
** this magic value that the code knows to handle differently.  Any
** pointer will work here as long as it is distinct from SQLITE_STATIC
** and SQLITE_TRANSIENT.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_DYNAMIC   ((sqlite3_destructor_type)sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Clear)

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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_WSD
  #ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_WSD ಸ್ಥಿರಾಂಕ
  #ವ್ಯಾಖ್ಯಾನಿಸು GLOBAL(t,v) (*(t*)sqlite3_wsd_ಕಂಡುಹಿಡಿ((ಶೂನ್ಯ*)&(v), ಗಾತ್ರ(v)))
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3GlobalConfig GLOBAL(ರಚನೆ Sqlite3Config, sqlite3Config)
  ಪೂರ್ಣಾಂಕ sqlite3_wsd_init(ಪೂರ್ಣಾಂಕ N, ಪೂರ್ಣಾಂಕ J);
  ಶೂನ್ಯ *sqlite3_wsd_ಕಂಡುಹಿಡಿ(ಶೂನ್ಯ *K, ಪೂರ್ಣಾಂಕ L);
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_WSD
  #ವ್ಯಾಖ್ಯಾನಿಸು GLOBAL(t,v) v
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3GlobalConfig sqlite3Config
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಳಸದ_PARAMETER(x) (ಶೂನ್ಯ)(x)
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಳಸದ_PARAMETER2(x,y) ಬಳಸದ_PARAMETER(x),ಬಳಸದ_PARAMETER(y)

/*
** Forward references to structures
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ AggInfo AggInfo;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ AuthContext AuthContext;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ AutoincInfo AutoincInfo;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Bitvec Bitvec;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ CollSeq CollSeq;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಸ್ತಂಭ ಸ್ತಂಭ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Cte Cte;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ CteUse CteUse;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Db Db;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Db_Client_ದತ್ತಾಂಶ Db_Client_ದತ್ತಾಂಶ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ DbFixer DbFixer;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Schema Schema;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ ಅಭಿವ್ಯಕ್ತಿ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List ಅಭಿವ್ಯಕ್ತಿ_List;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ FKey FKey;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ FpDecode FpDecode;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ FuncDestructor FuncDestructor;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ FuncDef FuncDef;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ FuncDefHash FuncDefHash;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ IdList IdList;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಸೂಚ್ಯಂಕ ಸೂಚ್ಯಂಕ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಸೂಚ್ಯಂಕ_Sample ಸೂಚ್ಯಂಕ_Sample;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಕೀಲಿ_Class ಕೀಲಿ_Class;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಕೀಲಿ_Info ಕೀಲಿ_Info;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Lookaside Lookaside;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Lookaside_ಸ್ಥಾನ Lookaside_ಸ್ಥಾನ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Module Module;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ NameContext NameContext;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ OnOrUsing OnOrUsing;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Parse Parse;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ParseCleanup ParseCleanup;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ PreUpdate PreUpdate;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ PrintfArguments PrintfArguments;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ RCStr RCStr;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ RenameToken RenameToken;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Returning Returning;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಅಡ್ಡಸಾಲು_ಗಣ ಅಡ್ಡಸಾಲು_ಗಣ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Savepoint Savepoint;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Select Select;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ SQLiteThread SQLiteThread;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ SelectDest SelectDest;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Subquery Subquery;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Src_ಅಂಶ Src_ಅಂಶ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ SrcList SrcList;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ sqlite3_str StrAccum; /* Internal alias for sqlite3_str */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಕೋಷ್ಟಕ ಕೋಷ್ಟಕ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಕೋಷ್ಟಕ_ಬೀಗ ಕೋಷ್ಟಕ_ಬೀಗ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Token Token;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ವೃಕ್ಷ_View ವೃಕ್ಷ_View;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Trigger Trigger;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ TriggerPrg TriggerPrg;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Trigger_ಹೆಜ್ಜೆ Trigger_ಹೆಜ್ಜೆ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Unpacked_ದಾಖಲೆ Unpacked_ದಾಖಲೆ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Upsert Upsert;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ VTable VTable;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ VtabCtx VtabCtx;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Walker Walker;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ WhereInfo WhereInfo;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಕಿಟಕಿ ಕಿಟಕಿ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ With With;


/*
** The bitmask datatype defined below is used for various optimizations.
**
** Changing this from a 64-bit to a 32-bit type limits the number of
** tables in a join to 32 instead of 64.  But it also reduces the size
** of the library by 738 bytes on ix86.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_BITMASK_TYPE
  ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ SQLITE_BITMASK_TYPE Bitmask;
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ u64 Bitmask;
#ಆದರೆ_ಅಂತ್ಯ

/*
** The number of bits in a Bitmask.  "BMS" means "BitMask Size".
*/
#ವ್ಯಾಖ್ಯಾನಿಸು BMS  ((ಪೂರ್ಣಾಂಕ)(ಗಾತ್ರ(Bitmask)*8))

/*
** A bit in a Bitmask
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MASKBIT(n)    (((Bitmask)1)<<(n))
#ವ್ಯಾಖ್ಯಾನಿಸು MASKBIT64(n)  (((u64)1)<<(n))
#ವ್ಯಾಖ್ಯಾನಿಸು MASKBIT32(n)  (((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)1)<<(n))
#ವ್ಯಾಖ್ಯಾನಿಸು SMASKBIT32(n) ((n)<=31?((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)1)<<(n):0)
#ವ್ಯಾಖ್ಯಾನಿಸು ALLBITS       ((Bitmask)-1)
#ವ್ಯಾಖ್ಯಾನಿಸು TOPBIT        (((Bitmask)1)<<(BMS-1))

/* A VList object records a mapping between parameters/variables/wildcards
** in the SQL statement (such as $abc, @pqr, or :xyz) and the integer
** variable number associated with that parameter.  See the format description
** on the sqlite3VListAdd() routine for more information.  A VList is really
** just an array of integers.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ VList;

/*
** Defer sourcing vdbe.h and btree.h until after the "u8" and
** "BusyHandler" typedefs. vdbe.h also requires a few of the opaque
** pointer types (i.e. FuncDef) defined above.
*/
#ಅನ್ತರ್ಭಾವ "os.h"
#ಅನ್ತರ್ಭಾವ "pager.h"
#ಅನ್ತರ್ಭಾವ "btree.h"
#ಅನ್ತರ್ಭಾವ "vdbe.h"
#ಅನ್ತರ್ಭಾವ "pcache.h"
#ಅನ್ತರ್ಭಾವ "mutex.h"

/* The SQLITE_EXTRA_DURABLE compile-time option used to set the default
** synchronous setting to EXTRA.  It is no longer supported.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EXTRA_DURABLE
# warning Use SQLITE_DEFAULT_SYNCHRONOUS=3 instead ದ SQLITE_EXTRA_DURABLE
# define SQLITE_DEFAULT_SYNCHRONOUS 3
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEFAULT_SYNCHRONOUS
# define SQLITE_DEFAULT_SYNCHRONOUS 2
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEFAULT_WAL_SYNCHRONOUS
# define SQLITE_DEFAULT_WAL_SYNCHRONOUS SQLITE_DEFAULT_SYNCHRONOUS
#ಆದರೆ_ಅಂತ್ಯ

/*
** Each database file to be accessed by the system is an instance
** of the following structure.  There are normally two of these structures
** in the sqlite.aDb[] array.  aDb[0] is the main database file and
** aDb[1] is the database file used to hold temporary tables.  Additional
** databases may be attached.
*/
ರಚನೆ Db {
  ಅಕ್ಷರ *zDbSName;      /* Name of this database. (schema name, not filename) */
  Btree *pBt;          /* The B*Tree structure for this database file */
  u8 safety_level;     /* How aggressive at syncing data to disk */
  u8 b_Sync_ಗಣ;         /* True if "PRAGMA synchronous=N" has been run */
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
ರಚನೆ Schema {
  ಪೂರ್ಣಾಂಕ schema_cookie;   /* Database schema version number for this file */
  ಪೂರ್ಣಾಂಕ iGeneration;     /* Generation counter.  Incremented with each change */
  Hash tblHash;        /* All tables indexed by name */
  Hash idxHash;        /* All (named) indices indexed by name */
  Hash trigHash;       /* All triggers indexed by name */
  Hash fkeyHash;       /* All foreign keys by referenced table name */
  ಕೋಷ್ಟಕ *pSeqTab;      /* The sqlite_sequence table used by AUTOINCREMENT */
  u8 file_format;      /* Schema format version for this file */
  u8 enc;              /* Text encoding used by this database */
  u16 schemaFlags;     /* Flags associated with this schema */
  ಪೂರ್ಣಾಂಕ cache_ಗಾತ್ರ;      /* Number of pages to use in the cache */
};

/*
** These macros can be used to test, set, or clear bits in the
** Db.pSchema->flags field.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು DbHasProperty(D,I,P)     (((D)->aDb[I].pSchema->schemaFlags&(P))==(P))
#ವ್ಯಾಖ್ಯಾನಿಸು DbHasAnyProperty(D,I,P)  (((D)->aDb[I].pSchema->schemaFlags&(P))!=0)
#ವ್ಯಾಖ್ಯಾನಿಸು Db_ಗಣ_Property(D,I,P)     (D)->aDb[I].pSchema->schemaFlags|=(P)
#ವ್ಯಾಖ್ಯಾನಿಸು DbClearProperty(D,I,P)   (D)->aDb[I].pSchema->schemaFlags&=~(P)

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
#ವ್ಯಾಖ್ಯಾನಿಸು DB_SchemaLoaded    0x0001  /* The schema has been loaded */
#ವ್ಯಾಖ್ಯಾನಿಸು DB_UnresetViews    0x0002  /* Some views have defined column names */
#ವ್ಯಾಖ್ಯಾನಿಸು DB_ResetWanted     0x0008  /* Reset the schema when nSchemaLock==0 */

/*
** The number of different kinds of things that can be limited
** using the sqlite3_limit() interface.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_N_ಎಲ್ಲೆ (SQLITE_ಎಲ್ಲೆ_CTES+1)

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
ರಚನೆ Lookaside {
  u32 bDisable;           /* Only operate the lookaside when zero */
  u16 sz;                 /* Size of each buffer in bytes */
  u16 szTrue;             /* True value of sz, even if disabled */
  u8 bMalloced;           /* True if pStart obtained from sqlite3_malloc() */
  u32 n_ಸ್ಥಾನ;              /* Number of lookaside slots allocated */
  u32 anStat[3];          /* 0: hits.  1: size misses.  2: full misses */
  Lookaside_ಸ್ಥಾನ *pInit;   /* List of buffers not previously used */
  Lookaside_ಸ್ಥಾನ *pFree;   /* List of available buffers */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TWOSIZE_LOOKASIDE
  Lookaside_ಸ್ಥಾನ *pSmallInit; /* List of small buffers not previously used */
  Lookaside_ಸ್ಥಾನ *pSmallFree; /* List of available small buffers */
  ಶೂನ್ಯ *p_ಮಧ್ಯ;          /* First byte past end of full-size buffers and
                          ** the first byte of LOOKASIDE_SMALL buffers */
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  ಶೂನ್ಯ *p_ಆರಂಭ;           /* First byte of available memory space */
  ಶೂನ್ಯ *p_ಕೊನೆ;             /* First byte past end of available space */
  ಶೂನ್ಯ *p_True_ಕೊನೆ;         /* True value of pEnd, when db->pnBytesFreed!=0 */
};
ರಚನೆ Lookaside_ಸ್ಥಾನ {
  Lookaside_ಸ್ಥಾನ *p_ಮುಂದಿನ;    /* Next buffer in the list of free buffers */
};

#ವ್ಯಾಖ್ಯಾನಿಸು DisableLookaside  db->lookaside.bDisable++;db->lookaside.sz=0
#ವ್ಯಾಖ್ಯಾನಿಸು EnableLookaside   db->lookaside.bDisable--;\
   db->lookaside.sz=db->lookaside.bDisable?0:db->lookaside.szTrue

/* Size of the smaller allocations in two-size lookaside */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_TWOSIZE_LOOKASIDE
#  define LOOKASIDE_SMALL           0
#ಇಲ್ಲದಿದ್ದರೆ
#  define LOOKASIDE_SMALL         128
#ಆದರೆ_ಅಂತ್ಯ

/*
** A hash table for built-in function definitions.  (Application-defined
** functions use a regular table table from hash.h.)
**
** Hash each FuncDef structure into one of the FuncDefHash.a[] slots.
** Collisions are on the FuncDef.u.pHash chain.  Use the SQLITE_FUNC_HASH()
** macro to compute a hash on the function name.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_HASH_SZ 23
ರಚನೆ FuncDefHash {
  FuncDef *a[SQLITE_FUNC_HASH_SZ];       /* Hash table for functions */
};
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_HASH(C,L) (((C)+(L))%SQLITE_FUNC_HASH_SZ)

/*
** typedef for the authorization callback function.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ (*sqlite3_xauth)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,
                             ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
/* This is an extra SQLITE_TRACE macro that indicates "legacy" tracing
** in the style of sqlite3_trace()
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_TRACE_LEGACY          0x40     /* Use the legacy xTrace */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_TRACE_XPROFILE        0x80     /* Use the legacy xProfile */
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_TRACE_LEGACY          0
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_TRACE_XPROFILE        0
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_DEPRECATED */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_TRACE_NONLEGACY_ಮುಖವಾಡ  0x0f     /* Normal flags */

/*
** Maximum number of sqlite3.aDb[] entries.  This is the number of attached
** databases plus 2 for "main" and "temp".
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MAX_DB (SQLITE_MAX_ATTACHED+2)

/*
** Each database connection is an instance of the following structure.
*/
ರಚನೆ sqlite3 {
  sqlite3_vfs *pVfs;            /* OS Interface */
  ರಚನೆ Vdbe *pVdbe;           /* List of active virtual machines */
  CollSeq *pDfltColl;           /* BINARY collseq for the database encoding */
  sqlite3_mutex *mutex;         /* Connection mutex */
  Db *aDb;                      /* All backends */
  ಪೂರ್ಣಾಂಕ nDb;                      /* Number of backends currently in use */
  u32 mDbFlags;                 /* flags recording internal state */
  u64 flags;                    /* flags settable by pragmas. See below */
  i64 ಕೊನೆಯ_Rowid;                /* ROWID of most recent insert (see above) */
  i64 szMmap;                   /* Default mmap_size setting */
  u32 n_Schema_ಬೀಗ;              /* Do not reset the schema when non-zero */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ openFlags;       /* Flags passed to sqlite3_vfs.xOpen() */
  ಪೂರ್ಣಾಂಕ err_ಸಂಕೇತ;                  /* Most recent error code (SQLITE_*) */
  ಪೂರ್ಣಾಂಕ err_Byte_ಸ್ಥಾನಾಂತರ;            /* Byte offset of error in SQL statement */
  ಪೂರ್ಣಾಂಕ err_ಮುಖವಾಡ;                  /* & result codes with this before returning */
  ಪೂರ್ಣಾಂಕ iSysErrno;                /* Errno value from last system error */
  u8 enc;                       /* Text encoding */
  u8 autoCommit;                /* The auto-commit flag. */
  u8 ತಾತ್ಕಾಲಿಕ_store;                /* 1: file 2: memory 0: default */
  u8 mallocFailed;              /* True if we have seen a malloc failure */
  u8 bBenignMalloc;             /* Do not require OOMs if true */
  u8 dflt_ಬೀಗ_ಬಹುಲಕ;              /* Default locking-mode for attached dbs */
  ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ ಮುಂದಿನ_Autovac;      /* Autovac setting after VACUUM if >=0 */
  u8 suppressErr;               /* Do not issue error messages if true */
  u8 vtabOnConflict;            /* Value to return for s3_vtab_on_conflict() */
  u8 isTransactionSavepoint;    /* True if the outermost savepoint is a TS */
  u8 mTrace;                    /* zero or more SQLITE_TRACE flags */
  u8 noSharedCache;             /* True if no shared-cache backends */
  u8 nSqlExec;                  /* Number of pending OP_SqlExec opcodes */
  u8 eOpenState;                /* Current condition of the connection */
  u8 nFpDigit;                  /* Significant digits to keep on double->text */
  ಪೂರ್ಣಾಂಕ ಮುಂದಿನ_Pagesize;             /* Pagesize after VACUUM if >0 */
  i64 nChange;                  /* Value returned by sqlite3_changes() */
  i64 n_ಒಟ್ಟು_Change;             /* Value returned by sqlite3_total_changes() */
  u32 dbOptFlags;               /* Flags to enable/disable optimizations */
  ಪೂರ್ಣಾಂಕ a_ಎಲ್ಲೆ[SQLITE_N_ಎಲ್ಲೆ];   /* Limits */
  ಪೂರ್ಣಾಂಕ nMaxSorterMmap;           /* Maximum size of regions mapped by sorter */
  ರಚನೆ sqlite3InitInfo {      /* Information used during initialization */
    Pgno newTnum;               /* Rootpage of table being initialized */
    u8 iDb;                     /* Which db file is being initialized */
    u8 busy;                    /* TRUE if initing. 2 if due to ADD COLUMN */
    ಚಿಹ್ನೆರಹಿತ orphanTrigger : 1; /* Last statement is orphaned TEMP trigger */
    ಚಿಹ್ನೆರಹಿತ imposter_ಕೋಷ್ಟಕ : 2; /* Building an imposter table */
    ಚಿಹ್ನೆರಹಿತ reopenMemdb : 1;   /* ATTACH is really a reopen using MemDB */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **azInit;        /* "type", "name", and "tbl_name" columns */
  } init;
  ಪೂರ್ಣಾಂಕ n_Vdbe_ಸಕ್ರಿಯ;              /* Number of VDBEs currently running */
  ಪೂರ್ಣಾಂಕ nVdbeRead;                /* Number of active VDBEs that read or write */
  ಪೂರ್ಣಾಂಕ nVdbeWrite;               /* Number of active VDBEs that read and write */
  ಪೂರ್ಣಾಂಕ nVdbeExec;                /* Number of nested calls to VdbeExec() */
  ಪೂರ್ಣಾಂಕ nVDestroy;                /* Number of active OP_VDestroy operations */
  ಪೂರ್ಣಾಂಕ nExtension;               /* Number of loaded extensions */
  ಶೂನ್ಯ **aExtension;            /* Array of shared library handles */
  ಒಕ್ಕೂಟ {
    ಶೂನ್ಯ (*xLegacy)(ಶೂನ್ಯ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);   /* mTrace==SQLITE_TRACE_LEGACY */
    ಪೂರ್ಣಾಂಕ (*xV2)(u32,ಶೂನ್ಯ*,ಶೂನ್ಯ*,ಶೂನ್ಯ*);    /* All other mTrace values */
  } trace;
  ಶೂನ್ಯ *pTraceArg;                        /* Argument to the trace function */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DEPRECATED
  ಶೂನ್ಯ (*xProfile)(ಶೂನ್ಯ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,u64);  /* Profiling function */
  ಶೂನ್ಯ *pProfileArg;                        /* Argument to profile function */
#ಆದರೆ_ಅಂತ್ಯ
  ಶೂನ್ಯ *pCommitArg;                 /* Argument to xCommitCallback() */
  ಪೂರ್ಣಾಂಕ (*xCommitCallback)(ಶೂನ್ಯ*);    /* Invoked at every commit. */
  ಶೂನ್ಯ *pRollbackArg;               /* Argument to xRollbackCallback() */
  ಶೂನ್ಯ (*xRollbackCallback)(ಶೂನ್ಯ*); /* Invoked at every commit. */
  ಶೂನ್ಯ *pUpdateArg;
  ಶೂನ್ಯ (*xUpdateCallback)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,sqlite_int64);
  ಶೂನ್ಯ *p_Autovac_ಪುಟಗಳು_Arg;           /* Client argument to autovac_pages */
  ಶೂನ್ಯ (*xAutovacDestr)(ಶೂನ್ಯ*);     /* Destructor for pAutovacPAgesArg */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ (*x_Autovac_ಪುಟಗಳು)(ಶೂನ್ಯ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,u32,u32,u32);
  Parse *pParse;                /* Current parse */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_PREUPDATE_HOOK
  ಶೂನ್ಯ *pPreUpdateArg;          /* First argument to xPreUpdateCallback */
  ಶೂನ್ಯ (*xPreUpdateCallback)(   /* Registered using sqlite3_preupdate_hook() */
    ಶೂನ್ಯ*,sqlite3*,ಪೂರ್ಣಾಂಕ,ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ*,ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ*,sqlite3_int64,sqlite3_int64
  );
  PreUpdate *pPreUpdate;        /* Context for active pre-update callback */
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_PREUPDATE_HOOK */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  ಪೂರ್ಣಾಂಕ (*xWalCallback)(ಶೂನ್ಯ *, sqlite3 *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ *pWalArg;
#ಆದರೆ_ಅಂತ್ಯ
  ಶೂನ್ಯ(*xCollNeeded)(ಶೂನ್ಯ*,sqlite3*,ಪೂರ್ಣಾಂಕ e_ಪಠ್ಯ_Rep,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಶೂನ್ಯ(*xCollNeeded16)(ಶೂನ್ಯ*,sqlite3*,ಪೂರ್ಣಾಂಕ e_ಪಠ್ಯ_Rep,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*);
  ಶೂನ್ಯ *pCollNeededArg;
  sqlite3_ಮೌಲ್ಯ *pErr;          /* Most recent error message */
  ಒಕ್ಕೂಟ {
    ಅಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isInterrupted; /* True if sqlite3_interrupt has been called */
    ದ್ವಿಗುಣ notUsed1;            /* Spacer */
  } u1;
  Lookaside lookaside;          /* Lookaside malloc configuration */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth;          /* Access authorization function */
  ಶೂನ್ಯ *pAuthArg;               /* 1st argument to the access auth function */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ಪ್ರಗತಿ_CALLBACK
  ಪೂರ್ಣಾಂಕ (*x_ಪ್ರಗತಿ)(ಶೂನ್ಯ *);     /* The progress callback */
  ಶೂನ್ಯ *p_ಪ್ರಗತಿ_Arg;           /* Argument to the progress callback */
  ಚಿಹ್ನೆರಹಿತ n_ಪ್ರಗತಿ_Ops;        /* Number of opcodes for progress callback */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಪೂರ್ಣಾಂಕ nVTrans;                  /* Allocated size of aVTrans */
  Hash aModule;                 /* populated by sqlite3_create_module() */
  VtabCtx *pVtabCtx;            /* Context for active vtab connect/create */
  VTable **aVTrans;             /* Virtual tables with open transactions */
  VTable *pDisconnect;          /* Disconnect these in next sqlite3_prepare() */
#ಆದರೆ_ಅಂತ್ಯ
  Hash aFunc;                   /* Hash table of connection functions */
  Hash aCollSeq;                /* All collating sequences */
  BusyHandler busyHandler;      /* Busy callback */
  Db aDbStatic[2];              /* Static space for the 2 default backends */
  Savepoint *pSavepoint;        /* List of active savepoints */
  ಪೂರ್ಣಾಂಕ n_Analysis_ಎಲ್ಲೆ;           /* Number of index rows to ANALYZE */
  ಪೂರ್ಣಾಂಕ busyTimeout;              /* Busy handler timeout, in msec */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
  ಪೂರ್ಣಾಂಕ setlkTimeout;             /* Blocking lock timeout, in msec. -1 -> inf. */
  ಪೂರ್ಣಾಂಕ setlkFlags;               /* Flags passed to setlk_timeout() */
#ಆದರೆ_ಅಂತ್ಯ
  ಪೂರ್ಣಾಂಕ nSavepoint;               /* Number of non-transaction savepoints */
  ಪೂರ್ಣಾಂಕ nStatement;               /* Number of nested statement-transactions  */
  i64 nDeferredCons;            /* Net deferred constraints this transaction. */
  i64 nDeferredImmCons;         /* Net deferred immediate constraints */
  ಪೂರ್ಣಾಂಕ *pn_ಬೈಟುಗಳು_Freed;            /* If not NULL, increment this in DbFree() */
  Db_Client_ದತ್ತಾಂಶ *p_Db_ದತ್ತಾಂಶ;        /* sqlite3_set_clientdata() content */
  u64 nSpill;                   /* TEMP content spilled to disk */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UNLOCK_NOTIFY
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
  ಶೂನ್ಯ *pUnlockArg;                     /* Argument to xUnlockNotify */
  ಶೂನ್ಯ (*xUnlockNotify)(ಶೂನ್ಯ **, ಪೂರ್ಣಾಂಕ);  /* Unlock notify callback */
  sqlite3 *p_ಮುಂದಿನ_Blocked;        /* Next in list of all blocked connections */
#ಆದರೆ_ಅಂತ್ಯ
};

/*
** A macro to discover the encoding of a database.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SCHEMA_ENC(db) ((db)->aDb[0].pSchema->enc)
#ವ್ಯಾಖ್ಯಾನಿಸು ENC(db)        ((db)->enc)

/*
** A u64 constant where the lower 32 bits are all zeros.  Only the
** upper 32 bits are included in the argument.  Necessary because some
** C-compilers still do not accept LL integer literals.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು HI(X)  ((u64)(X)<<32)

/*
** Possible values for the sqlite3.flags.
**
** Value constraints (enforced via assert()):
**      SQLITE_FullFSync     == PAGER_FULLFSYNC
**      SQLITE_CkptFullFSync == PAGER_CKPT_FULLFSYNC
**      SQLITE_CacheSpill    == PAGER_CACHE_SPILL
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_WriteSchema    0x00000001  /* OK to update SQLITE_SCHEMA */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_LegacyFileFmt  0x00000002  /* Create new databases in format 1 */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ತುಂಬಿದ_ಲಂಬಸಾಲು_ಹೆಸರುಗಳು   0x00000004  /* Show full column names on SELECT */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ತುಂಬಿದ_FSync      0x00000008  /* Use full fsync on the backend */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Ckpt_ತುಂಬಿದ_FSync  0x00000010  /* Use full fsync for checkpoint */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_CacheSpill     0x00000020  /* OK to spill pager cache */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Short_ಲಂಬಸಾಲು_ಹೆಸರುಗಳು  0x00000040  /* Show short columns names */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_TrustedSchema  0x00000080  /* Allow unsafe functions and
                                          ** vtabs in the schema definition */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_NullCallback   0x00000100  /* Invoke the callback once if the */
                                          /*   result set is empty */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_IgnoreChecks   0x00000200  /* Do not enforce check constraints */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Stmt_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ 0x00000400  /* Enable stmt_scanstats() counters */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_NoCkptOnClose  0x00000800  /* No checkpoint on close()/DETACH */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಹಿಮ್ಮುಖ_ಕ್ರಮ   0x00001000  /* Reverse unordered SELECTs */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_RecTriggers    0x00002000  /* Enable recursive triggers */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Foreign_ಕೀಲಿಗಳು    0x00004000  /* Enforce foreign key constraints  */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Auto_ಸೂಚ್ಯಂಕ      0x00008000  /* Enable automatic indexes */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ತುಂಬು_Extension  0x00010000  /* Enable load_extension */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ತುಂಬು_Ext_Func    0x00020000  /* Enable load_extension() SQL func */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_EnableTrigger  0x00040000  /* True to enable triggers */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_DeferFKs       0x00080000  /* Defer all FK constraints */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_QueryOnly      0x00100000  /* Disable database changes */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Cell_ಗಾತ್ರ_Ck     0x00200000  /* Check btree cell sizes on load */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Fts3Tokenizer  0x00400000  /* Enable fts3_tokenizer(2) */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_EnableQPSG     0x00800000  /* Query Planner Stability Guarantee*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_TriggerEQP     0x01000000  /* Show trigger EXPLAIN QUERY PLAN */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ResetDatabase  0x02000000  /* Reset the database */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_LegacyAlter    0x04000000  /* Legacy ALTER TABLE behaviour */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_NoSchemaError  0x08000000  /* Do not report schema parse errors*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Defensive      0x10000000  /* Input SQL is likely hostile */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_DqsDDL         0x20000000  /* dbl-quoted strings allowed in DDL*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_DqsDML         0x40000000  /* dbl-quoted strings allowed in DML*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_EnableView     0x80000000  /* Enable the use of views */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಎಣಿಕೆ_ಅಡ್ಡಸಾಲುಗಳು      HI(0x00001) /* Count rows changed by INSERT, */
                                          /*   DELETE, or UPDATE and return */
                                          /*   the count using a callback. */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_CorruptRdOnly  HI(0x00002) /* Prohibit writes due to error */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ReadUncommit   HI(0x00004) /* READ UNCOMMITTED in shared-cache */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FkNoAction     HI(0x00008) /* Treat all FK as NO ACTION */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_AttachCreate   HI(0x00010) /* ATTACH allowed to create new dbs */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_AttachWrite    HI(0x00020) /* ATTACH allowed to open for write */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Comments       HI(0x00040) /* Enable SQL comments */

/* Flags used only if debugging */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_SqlTrace       HI(0x0100000) /* Debug print SQL as it executes */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_VdbeListing    HI(0x0200000) /* Debug listings of VDBE progs */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_VdbeTrace      HI(0x0400000) /* True to trace VDBE execution */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_VdbeAddopTrace HI(0x0800000) /* Trace sqlite3VdbeAddOp() calls */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_VdbeEQP        HI(0x1000000) /* Debug EXPLAIN QUERY PLAN */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ParserTrace    HI(0x2000000) /* PRAGMA parser_trace=ON */
#ಆದರೆ_ಅಂತ್ಯ

/*
** Allowed values for sqlite3.mDbFlags
*/
#ವ್ಯಾಖ್ಯಾನಿಸು DBFLAG_SchemaChange   0x0001  /* Uncommitted Hash table changes */
#ವ್ಯಾಖ್ಯಾನಿಸು DBFLAG_PreferBuiltin  0x0002  /* Preference to built-in funcs */
#ವ್ಯಾಖ್ಯಾನಿಸು DBFLAG_Vacuum         0x0004  /* Currently in a VACUUM */
#ವ್ಯಾಖ್ಯಾನಿಸು DBFLAG_VacuumInto     0x0008  /* Currently running VACUUM INTO */
#ವ್ಯಾಖ್ಯಾನಿಸು DBFLAG_Schema_ತಿಳಿದ_Ok  0x0010  /* Schema is known to be valid */
#ವ್ಯಾಖ್ಯಾನಿಸು DBFLAG_ಆಂತರಿಕ_Func   0x0020  /* Allow use of internal functions */
#ವ್ಯಾಖ್ಯಾನಿಸು DBFLAG_EncodingFixed  0x0040  /* No longer possible to change enc. */

/*
** Bits of the sqlite3.dbOptFlags field that are used by the
** sqlite3_test_control(SQLITE_TESTCTRL_OPTIMIZATIONS,...) interface to
** selectively disable various optimizations. The numbers in each macro
** are the one-based bit position (for improved readability).
**
** Keep these values in sync with the ext/misc/optname.c extension.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_QueryFlattener  1 /* Query flattening */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಕಿಟಕಿ_Func      2 /* Use xInverse for window functions */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಗುಂಪು_By_ಕ್ರಮ    3 /* GROUPBY cover of ORDERBY */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Join_ತಪ್ಪಿದ        4 /* Unwind join loops on a lookup miss*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_DistinctOpt     5 /* DISTINCT using indexes */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Cover_Idx_ಪರಿಶೀಲನೆ    6 /* Covering index scans */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಕ್ರಮ_By_Idx_Join  7 /* ORDER BY of joins via index */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಸಂಕ್ರಾಮಿಕ      8 /* Transitive constraints */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_OmitNoopJoin    9 /* Omit unused tables in joins */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಎಣಿಕೆ_ದ_View    10 /* The count-of-view optimization */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_CursorHints    11 /* Add OP_CursorHint opcodes */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Stat4          12 /* Use STAT4 data */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ತಳ್ಳು_ಕೆಳಕ್ಕೆ       13 /* WHERE-clause push-down opt */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_SimplifyJoin   14 /* Convert LEFT JOIN to JOIN */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Skip_ಪರಿಶೀಲನೆ       15 /* Skip-scans */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_PropagateConst 16 /* The constant propagation opt */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MinMaxOpt      17 /* The min/max optimization */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Seek_ಪರಿಶೀಲನೆ       18 /* The OP_SeekScan optimization */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Omit_ಕ್ರಮ_By    19 /* Omit pointless ORDER BY */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_BloomFilter    20 /* Use a Bloom filters */
                          /*  21 -- Available for reuse */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಸಮತೋಲಿತ_ವಿಲೀನ  22 /* Balance multi-way merges */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ReleaseReg     23 /* Use OP_ReleaseReg for testing */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FlttnUnionAll  24 /* Disable the UNION ALL flattener */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ    25 /* Pull exprs from index when able */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Coroutines     26 /* Co-routines for subqueries */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Null_ಬಳಸದ_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ 27 /* NULL unused columns in subqueries */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_OnePass        28 /* Single-pass DELETE and UPDATE */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ಕ್ರಮ_By_Subq    29 /* ORDER BY in subquery helps outer */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ನಕ್ಷತ್ರ_Query      30 /* Heurists for star queries */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Exists_ಗಮ್ಯ_Join   31 /* The EXISTS-to-JOIN optimization */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_Union_ಎಲ್ಲೆ     32 /* Optimizations for UNION + LIMIT */

/*
** Macros for testing whether or not optimizations are enabled or disabled.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು OptimizationDisabled(D,M)  (((D)->dbOptFlags&(1ul<<((M)-1)))!=0)
#ವ್ಯಾಖ್ಯಾನಿಸು OptimizationEnabled(D,M)   (((D)->dbOptFlags&(1ul<<((M)-1)))==0)

/*
** Return true if it OK to factor constant expressions into the initialization
** code. The argument is a Parse object for the code generator.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು Const_ಗುಣಕ_Ok(P) ((P)->ok_Const_ಗುಣಕ)

/* Possible values for the sqlite3.eOpenState field.
** The numbers are randomly selected such that a minimum of three bits must
** change to convert any number to another or to zero
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_STATE_OPEN     0x76  /* Database is open */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_STATE_CLOSED   0xce  /* Database is closed */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_STATE_SICK     0xba  /* Error and awaiting close */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_STATE_BUSY     0x6d  /* Database currently in use */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_STATE_ERROR    0xd5  /* An SQLITE_MISUSE error occurred */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_STATE_ZOMBIE   0xa7  /* Close with last statement close */

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
ರಚನೆ FuncDef {
  i16 nArg;            /* Number of arguments.  -1 means unlimited */
  u32 funcFlags;       /* Some combination of SQLITE_FUNC_* */
  ಶೂನ್ಯ *p_User_ದತ್ತಾಂಶ;     /* User data parameter */
  FuncDef *p_ಮುಂದಿನ;      /* Next function with same name */
  ಶೂನ್ಯ (*xSFunc)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**); /* func or agg-step */
  ಶೂನ್ಯ (*xFinalize)(sqlite3_context*);                  /* Agg finalizer */
  ಶೂನ್ಯ (*x_ಮೌಲ್ಯ)(sqlite3_context*);                     /* Current agg value */
  ಶೂನ್ಯ (*xInverse)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ**); /* inverse agg-step */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;   /* SQL name of the function. */
  ಒಕ್ಕೂಟ {
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
ರಚನೆ FuncDestructor {
  ಪೂರ್ಣಾಂಕ nRef;
  ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ *);
  ಶೂನ್ಯ *p_User_ದತ್ತಾಂಶ;
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
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_ENCMASK  0x0003 /* SQLITE_UTF8, SQLITE_UTF16BE or UTF16LE */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_LIKE     0x0004 /* Candidate for the LIKE optimization */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_CASE     0x0008 /* Case-sensitive LIKE-type function */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_EPHEM    0x0010 /* Ephemeral.  Delete with VDBE */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_NEEDCOLL 0x0020 /* sqlite3GetFuncCollSeq() might be called*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_ಉದ್ದಳತೆ   0x0040 /* Built-in length() function */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_TYPEOF   0x0080 /* Built-in typeof() function */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_BYTELEN  0x00c0 /* Built-in octet_length() function */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_ಎಣಿಕೆ    0x0100 /* Built-in count(*) aggregate */
/*                           0x0200 -- available for reuse */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_UNLIKELY 0x0400 /* Built-in unlikely() function */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_CONSTANT 0x0800 /* Constant inputs give a constant output */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_MINMAX   0x1000 /* True for min() and max() aggregates */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_SLOCHNG  0x2000 /* "Slow Change". Value constant during a
                                    ** single query - might change over time */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_TEST     0x4000 /* Built-in testing functions */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_RUNONLY  0x8000 /* Cannot be used by valueFromFunction */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_ಕಿಟಕಿ   0x00010000 /* Built-in window-only function */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_ಆಂತರಿಕ 0x00040000 /* For use by NestedParse() only */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_ನೇರ   0x00080000 /* Not for use in TRIGGERs or VIEWs */
/* SQLITE_SUBTYPE            0x00100000 // Consumer of subtypes */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_UNSAFE   0x00200000 /* Function has side effects */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_INLINE   0x00400000 /* Functions implemented in-line */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_BUILTIN  0x00800000 /* This is a built-in function */
/*  SQLITE_RESULT_SUBTYPE    0x01000000 // Generator of subtypes */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FUNC_ANYORDER 0x08000000 /* count/min/max aggregate */

/* Identifier numbers for each in-line function */
#ವ್ಯಾಖ್ಯಾನಿಸು INLINEFUNC_coalesce             0
#ವ್ಯಾಖ್ಯಾನಿಸು INLINEFUNC_ಸೂಚಿಸುತ್ತದೆ_nonnull_ಅಡ್ಡಸಾಲು  1
#ವ್ಯಾಖ್ಯಾನಿಸು INLINEFUNC_ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_ಅಭಿವ್ಯಕ್ತಿ    2
#ವ್ಯಾಖ್ಯಾನಿಸು INLINEFUNC_ಅಭಿವ್ಯಕ್ತಿ_compare         3
#ವ್ಯಾಖ್ಯಾನಿಸು INLINEFUNC_affinity             4
#ವ್ಯಾಖ್ಯಾನಿಸು INLINEFUNC_iif                  5
#ವ್ಯಾಖ್ಯಾನಿಸು INLINEFUNC_sqlite_ಸ್ಥಾನಾಂತರ        6
#ವ್ಯಾಖ್ಯಾನಿಸು INLINEFUNC_unlikely            99  /* Default case */

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
#ವ್ಯಾಖ್ಯಾನಿಸು FUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_CONSTANT|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_ಗಮ್ಯ_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#ವ್ಯಾಖ್ಯಾನಿಸು VFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_ಗಮ್ಯ_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#ವ್ಯಾಖ್ಯಾನಿಸು SFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|SQLITE_DIRECTONLY|SQLITE_FUNC_UNSAFE, \
   SQLITE_INT_ಗಮ್ಯ_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#ವ್ಯಾಖ್ಯಾನಿಸು MFUNCTION(zName, nArg, xPtr, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_CONSTANT|SQLITE_UTF8, \
   xPtr, 0, xFunc, 0, 0, 0, #zName, {0} }
#ವ್ಯಾಖ್ಯಾನಿಸು JFUNCTION(zName, nArg, bUseCache, bWS, bRS, bJsonB, iArg, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_DETERMINISTIC|SQLITE_FUNC_CONSTANT|\
   SQLITE_UTF8|((bUseCache)*SQLITE_FUNC_RUNONLY)|\
   ((bRS)*SQLITE_SUBTYPE)|((bWS)*SQLITE_ಫಲಿತಾಂಶ_SUBTYPE), \
   SQLITE_INT_ಗಮ್ಯ_PTR(iArg|((bJsonB)*JSON_BLOB)),0,xFunc,0, 0, 0, #zName, {0} }
#ವ್ಯಾಖ್ಯಾನಿಸು INLINE_FUNC(zName, nArg, iArg, mFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_UTF8|SQLITE_FUNC_INLINE|SQLITE_FUNC_CONSTANT|(mFlags), \
   SQLITE_INT_ಗಮ್ಯ_PTR(iArg), 0, noopFunc, 0, 0, 0, #zName, {0} }
#ವ್ಯಾಖ್ಯಾನಿಸು TEST_FUNC(zName, nArg, iArg, mFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
         SQLITE_UTF8|SQLITE_FUNC_ಆಂತರಿಕ|SQLITE_FUNC_TEST| \
         SQLITE_FUNC_INLINE|SQLITE_FUNC_CONSTANT|(mFlags), \
   SQLITE_INT_ಗಮ್ಯ_PTR(iArg), 0, noopFunc, 0, 0, 0, #zName, {0} }
#ವ್ಯಾಖ್ಯಾನಿಸು DFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_SLOCHNG|SQLITE_UTF8, \
   0, 0, xFunc, 0, 0, 0, #zName, {0} }
#ವ್ಯಾಖ್ಯಾನಿಸು ಶುದ್ಧ_DATE(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
         SQLITE_FUNC_SLOCHNG|SQLITE_UTF8|SQLITE_FUNC_CONSTANT, \
   (ಶೂನ್ಯ*)&sqlite3Config, 0, xFunc, 0, 0, 0, #zName, {0} }
#ವ್ಯಾಖ್ಯಾನಿಸು FUNCTION2(zName, nArg, iArg, bNC, xFunc, extraFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_CONSTANT|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL)|extraFlags,\
   SQLITE_INT_ಗಮ್ಯ_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#ವ್ಯಾಖ್ಯಾನಿಸು STR_FUNCTION(zName, nArg, pArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_SLOCHNG|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   pArg, 0, xFunc, 0, 0, 0, #zName, {0} }
#ವ್ಯಾಖ್ಯಾನಿಸು LIKEFUNC(zName, nArg, arg, flags) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_CONSTANT|SQLITE_UTF8|flags, \
   (ಶೂನ್ಯ *)arg, 0, likeFunc, 0, 0, 0, #zName, {0} }
#ವ್ಯಾಖ್ಯಾನಿಸು WAGGREGATE(zName, nArg, arg, nc, x_ಹೆಜ್ಜೆ, xFinal, x_ಮೌಲ್ಯ, xInverse, f) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|(nc*SQLITE_FUNC_NEEDCOLL)|f, \
   SQLITE_INT_ಗಮ್ಯ_PTR(arg), 0, x_ಹೆಜ್ಜೆ,xFinal,x_ಮೌಲ್ಯ,xInverse,#zName, {0}}
#ವ್ಯಾಖ್ಯಾನಿಸು ಆಂತರಿಕ_FUNCTION(zName, nArg, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_ಆಂತರಿಕ|SQLITE_UTF8|SQLITE_FUNC_CONSTANT, \
   0, 0, xFunc, 0, 0, 0, #zName, {0} }


/*
** All current savepoints are stored in a linked list starting at
** sqlite3.pSavepoint. The first element in the list is the most recently
** opened savepoint. Savepoints are added to the list by the vdbe
** OP_Savepoint instruction.
*/
ರಚನೆ Savepoint {
  ಅಕ್ಷರ *zName;                        /* Savepoint name (nul-terminated) */
  i64 nDeferredCons;                  /* Number of deferred fk violations */
  i64 nDeferredImmCons;               /* Number of deferred imm fk. */
  Savepoint *p_ಮುಂದಿನ;                   /* Parent savepoint (if any) */
};

/*
** The following are used as the second parameter to sqlite3Savepoint(),
** and as the P1 argument to the OP_Savepoint instruction.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SAVEPOINT_BEGIN      0
#ವ್ಯಾಖ್ಯಾನಿಸು SAVEPOINT_RELEASE    1
#ವ್ಯಾಖ್ಯಾನಿಸು SAVEPOINT_ROLLBACK   2


/*
** Each SQLite module (virtual table definition) is defined by an
** instance of the following structure, stored in the sqlite3.aModule
** hash table.
*/
ರಚನೆ Module {
  ಸ್ಥಿರಾಂಕ sqlite3_module *pModule;       /* Callback pointers */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;                   /* Name passed to create_module() */
  ಪೂರ್ಣಾಂಕ nRefModule;                      /* Number of pointers to this object */
  ಶೂನ್ಯ *pAux;                          /* pAux passed to create_module() */
  ಶೂನ್ಯ (*xDestroy)(ಶೂನ್ಯ *);            /* Module destructor function */
  ಕೋಷ್ಟಕ *pEpoTab;                      /* Eponymous table for this module */
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
ರಚನೆ ಸ್ತಂಭ {
  ಅಕ್ಷರ *zCnName;        /* Name of this column */
  ಚಿಹ್ನೆರಹಿತ notNull :4;  /* An OE_ code for handling a NOT NULL constraint */
  ಚಿಹ್ನೆರಹಿತ eCType :4;   /* One of the standard types */
  ಅಕ್ಷರ affinity;        /* One of the SQLITE_AFF_... values */
  u8 szEst;             /* Est size of value in this column. sizeof(INT)==1 */
  u8 hName;             /* Column name hash for faster lookup */
  u16 iDflt;            /* 1-based index of DEFAULT.  0 means "none" */
  u16 ಲಂಬಸಾಲು_Flags;         /* Boolean properties.  See COLFLAG_ defines below */
};

/* Allowed values for Column.eCType.
**
** Values must match entries in the global constant arrays
** sqlite3StdTypeLen[] and sqlite3StdType[].  Each value is one more
** than the offset into these arrays for the corresponding name.
** Adjust the SQLITE_N_STDTYPE value if adding or removing entries.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು COLTYPE_CUSTOM      0   /* Type appended to zName */
#ವ್ಯಾಖ್ಯಾನಿಸು COLTYPE_ANY         1
#ವ್ಯಾಖ್ಯಾನಿಸು COLTYPE_BLOB        2
#ವ್ಯಾಖ್ಯಾನಿಸು COLTYPE_INT         3
#ವ್ಯಾಖ್ಯಾನಿಸು COLTYPE_INTEGER     4
#ವ್ಯಾಖ್ಯಾನಿಸು COLTYPE_REAL        5
#ವ್ಯಾಖ್ಯಾನಿಸು COLTYPE_ಪಠ್ಯ        6
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_N_STDTYPE    6  /* Number of standard types */

/* Allowed values for Column.colFlags.
**
** Constraints:
**         TF_HasVirtual == COLFLAG_VIRTUAL
**         TF_HasStored  == COLFLAG_STORED
**         TF_HasHidden  == COLFLAG_HIDDEN
*/
#ವ್ಯಾಖ್ಯಾನಿಸು COLFLAG_PRIMKEY   0x0001   /* Column is part of the primary key */
#ವ್ಯಾಖ್ಯಾನಿಸು COLFLAG_HIDDEN    0x0002   /* A hidden column in a virtual table */
#ವ್ಯಾಖ್ಯಾನಿಸು COLFLAG_HASTYPE   0x0004   /* Type name follows column name */
#ವ್ಯಾಖ್ಯಾನಿಸು COLFLAG_UNIQUE    0x0008   /* Column def contains "UNIQUE" or "PK" */
#ವ್ಯಾಖ್ಯಾನಿಸು COLFLAG_SORTERREF 0x0010   /* Use sorter-refs with this column */
#ವ್ಯಾಖ್ಯಾನಿಸು COLFLAG_VIRTUAL   0x0020   /* GENERATED ALWAYS AS ... VIRTUAL */
#ವ್ಯಾಖ್ಯಾನಿಸು COLFLAG_STORED    0x0040   /* GENERATED ALWAYS AS ... STORED */
#ವ್ಯಾಖ್ಯಾನಿಸು COLFLAG_NOTAVAIL  0x0080   /* STORED column not yet calculated */
#ವ್ಯಾಖ್ಯಾನಿಸು COLFLAG_BUSY      0x0100   /* Blocks recursion on GENERATED columns */
#ವ್ಯಾಖ್ಯಾನಿಸು COLFLAG_HASCOLL   0x0200   /* Has collating sequence name in zCnName */
#ವ್ಯಾಖ್ಯಾನಿಸು COLFLAG_NOEXPAND  0x0400   /* Omit this column when expanding "*" */
#ವ್ಯಾಖ್ಯಾನಿಸು COLFLAG_GENERATED 0x0060   /* Combo: _STORED, _VIRTUAL */
#ವ್ಯಾಖ್ಯಾನಿಸು COLFLAG_NOINSERT  0x0062   /* Combo: _HIDDEN, _STORED, _VIRTUAL */

/*
** A "Collating Sequence" is defined by an instance of the following
** structure. Conceptually, a collating sequence consists of a name and
** a comparison routine that defines the order of that sequence.
**
** If CollSeq.xCmp is NULL, it means that the
** collating sequence is undefined.  Indices built on an undefined
** collating sequence may not be read or written.
*/
ರಚನೆ CollSeq {
  ಅಕ್ಷರ *zName;          /* Name of the collating sequence, UTF-8 encoded */
  u8 enc;               /* Text encoding handled by xCmp() */
  ಶೂನ್ಯ *pUser;          /* First argument to xCmp() */
  ಪೂರ್ಣಾಂಕ (*xCmp)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*);
  ಶೂನ್ಯ (*xDel)(ಶೂನ್ಯ*);  /* Destructor for pUser */
};

/*
** A sort order can be either ASC or DESC.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_SO_ASC       0  /* Sort in ascending order */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_SO_DESC      1  /* Sort in ascending order */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_SO_UNDEFINED -1 /* No sort order specified */

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
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_AFF_NONE     0x40  /* '@' */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_AFF_BLOB     0x41  /* 'A' */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_AFF_ಪಠ್ಯ     0x42  /* 'B' */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_AFF_NUMERIC  0x43  /* 'C' */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_AFF_INTEGER  0x44  /* 'D' */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_AFF_REAL     0x45  /* 'E' */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_AFF_FLEXNUM  0x46  /* 'F' */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_AFF_DEFER    0x58  /* 'X'  - defer computation until later */

#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3IsNumericAffinity(X)  ((X)>=SQLITE_AFF_NUMERIC)

/*
** The SQLITE_AFF_MASK values masks off the significant bits of an
** affinity value.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_AFF_ಮುಖವಾಡ     0x47

/*
** Additional bit values that can be ORed with an affinity without
** changing the affinity.
**
** The SQLITE_NOTNULL flag is a combination of NULLEQ and JUMPIFNULL.
** It causes an assert() to fire if either operand to a comparison
** operator is NULL.  It is added to certain comparison operators to
** prove that the operands are always NOT NULL.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_JUMPIFNULL   0x10  /* jumps if either operand is NULL */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_NULLEQ       0x80  /* NULL=NULL */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_NOTNULL      0x90  /* Assert that operands are never NULL */

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
ರಚನೆ VTable {
  sqlite3 *db;              /* Database connection associated with this table */
  Module *pMod;             /* Pointer to module implementation */
  sqlite3_vtab *pVtab;      /* Pointer to vtab instance */
  ಪೂರ್ಣಾಂಕ nRef;                 /* Number of pointers to this structure */
  u8 bConstraint;           /* True if constraints are supported */
  u8 bAllSchemas;           /* True if might use any attached schema */
  u8 eVtabRisk;             /* Riskiness of allowing hacker access */
  ಪೂರ್ಣಾಂಕ iSavepoint;           /* Depth of the SAVEPOINT stack */
  VTable *p_ಮುಂದಿನ;            /* Next in linked list (see above) */
};

/* Allowed values for VTable.eVtabRisk
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_VTABRISK_ಕೆಳಗಿನ          0
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_VTABRISK_ಸಾಮಾನ್ಯ       1
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_VTABRISK_ಮೇಲಿನ         2

/*
** The schema for each SQL table, virtual table, and view is represented
** in memory by an instance of the following structure.
*/
ರಚನೆ ಕೋಷ್ಟಕ {
  ಅಕ್ಷರ *zName;         /* Name of the table or view */
  ಸ್ತಂಭ *a_ಲಂಬಸಾಲು;        /* Information about each column */
  ಸೂಚ್ಯಂಕ *p_ಸೂಚ್ಯಂಕ;       /* List of SQL indexes on this table. */
  ಅಕ್ಷರ *z_ಲಂಬಸಾಲು_Aff;       /* String defining the affinity of each column */
  ಅಭಿವ್ಯಕ್ತಿ_List *pCheck;    /* All CHECK constraints */
                       /*   ... also used as column name list in a VIEW */
  Pgno tnum;           /* Root BTree page for this table */
  u32 nTabRef;         /* Number of pointers to this Table */
  u32 tabFlags;        /* Mask of TF_* values */
  i16 iPKey;           /* If not negative, use aCol[iPKey] as the rowid */
  i16 n_ಲಂಬಸಾಲು;            /* Number of columns in this table */
  i16 nNVCol;          /* Number of columns that are not VIRTUAL */
  LogEst n_ಅಡ್ಡಸಾಲು_Log_Est;   /* Estimated rows in table - from sqlite_stat1 table */
  LogEst sz_Tab_ಅಡ್ಡಸಾಲು;     /* Estimated size of each table row in bytes */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_COSTMULT
  LogEst ವೆಚ್ಚ_Mult;     /* Cost multiplier for using this table */
#ಆದರೆ_ಅಂತ್ಯ
  u8 ಕೀಲಿ_Conf;          /* What to do in case of uniqueness conflict on iPKey */
  u8 eTabType;         /* 0: normal, 1: virtual, 2: view */
  ಒಕ್ಕೂಟ {
    ರಚನೆ {             /* Used by ordinary tables: */
      ಪೂರ್ಣಾಂಕ ಕೂಡಿಸು_ಲಂಬಸಾಲು_ಸ್ಥಾನಾಂತರ;    /* Offset in CREATE TABLE stmt to add a new column */
      FKey *pFKey;         /* Linked list of all foreign keys in this table */
      ಅಭಿವ್ಯಕ್ತಿ_List *pDfltList; /* DEFAULT clauses on various columns.
                           ** Or the AS clause for generated columns. */
    } tab;
    ರಚನೆ {             /* Used by views: */
      Select *pSelect;     /* View definition */
    } view;
    ರಚನೆ {             /* Used by virtual tables only: */
      ಪೂರ್ಣಾಂಕ nArg;            /* Number of arguments to the module */
      ಅಕ್ಷರ **azArg;        /* 0: module 1: schema 2: vtab name 3...: args */
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
#ವ್ಯಾಖ್ಯಾನಿಸು TF_Readonly       0x00000001 /* Read-only system table */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_HasHidden      0x00000002 /* Has one or more hidden columns */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_Has_Primary_ಕೀಲಿ  0x00000004 /* Table has a primary key */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_Autoincrement  0x00000008 /* Integer primary key is autoincrement */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_HasStat1       0x00000010 /* nRowLogEst set from sqlite_stat1 */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_HasVirtual     0x00000020 /* Has one or more VIRTUAL columns */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_HasStored      0x00000040 /* Has one or more STORED columns */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_HasGenerated   0x00000060 /* Combo: HasVirtual + HasStored */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_WithoutRowid   0x00000080 /* No rowid.  PRIMARY KEY is the key */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_MaybeReanalyze 0x00000100 /* Maybe run ANALYZE on this table */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_NoVisibleRowid 0x00000200 /* No user-visible "rowid" column */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_OOOHidden      0x00000400 /* Out-of-Order hidden columns */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_HasNotNull     0x00000800 /* Contains NOT NULL constraints */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_Shadow         0x00001000 /* True for a shadow table */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_HasStat4       0x00002000 /* STAT4 info available for this table */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_Ephemeral      0x00004000 /* An ephemeral table */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_Eponymous      0x00008000 /* An eponymous virtual table */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_Strict         0x00010000 /* STRICT mode */
#ವ್ಯಾಖ್ಯಾನಿಸು TF_Imposter       0x00020000 /* An imposter table */

/*
** Allowed values for Table.eTabType
*/
#ವ್ಯಾಖ್ಯಾನಿಸು TABTYP_NORM      0     /* Ordinary table */
#ವ್ಯಾಖ್ಯಾನಿಸು TABTYP_VTAB      1     /* Virtual table */
#ವ್ಯಾಖ್ಯಾನಿಸು TABTYP_VIEW      2     /* A view */

#ವ್ಯಾಖ್ಯಾನಿಸು IsView(X)           ((X)->eTabType==TABTYP_VIEW)
#ವ್ಯಾಖ್ಯಾನಿಸು Is_Ordinary_ಕೋಷ್ಟಕ(X)  ((X)->eTabType==TABTYP_NORM)

/*
** Test to see whether or not a table is a virtual table.  This is
** done as a macro so that it will be optimized out when virtual
** table support is omitted from the build.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
#  define IsVirtual(X)      ((X)->eTabType==TABTYP_VTAB)
#  define ಅಭಿವ್ಯಕ್ತಿ_Is_Vtab(X)  \
   ((X)->op==TK_ಸ್ತಂಭ && (X)->y.pTab->eTabType==TABTYP_VTAB)
#ಇಲ್ಲದಿದ್ದರೆ
#  define IsVirtual(X)      0
#  define ಅಭಿವ್ಯಕ್ತಿ_Is_Vtab(X)     0
#ಆದರೆ_ಅಂತ್ಯ

/*
** Macros to determine if a column is hidden.  IsOrdinaryHiddenColumn()
** only works for non-virtual tables (ordinary tables and views) and is
** always false unless SQLITE_ENABLE_HIDDEN_COLUMNS is defined.  The
** IsHiddenColumn() macro is general purpose.
*/
#ಆದರೆ defined(SQLITE_ENABLE_HIDDEN_COLUMNS)
#  define Is_Hidden_ಸ್ತಂಭ(X)         (((X)->ಲಂಬಸಾಲು_Flags & COLFLAG_HIDDEN)!=0)
#  define Is_Ordinary_Hidden_ಸ್ತಂಭ(X) (((X)->ಲಂಬಸಾಲು_Flags & COLFLAG_HIDDEN)!=0)
#ಅಥವಾಆದರೆ !defined(SQLITE_OMIT_VIRTUALTABLE)
#  define Is_Hidden_ಸ್ತಂಭ(X)         (((X)->ಲಂಬಸಾಲು_Flags & COLFLAG_HIDDEN)!=0)
#  define Is_Ordinary_Hidden_ಸ್ತಂಭ(X) 0
#ಇಲ್ಲದಿದ್ದರೆ
#  define Is_Hidden_ಸ್ತಂಭ(X)         0
#  define Is_Ordinary_Hidden_ಸ್ತಂಭ(X) 0
#ಆದರೆ_ಅಂತ್ಯ


/* Does the table have a rowid */
#ವ್ಯಾಖ್ಯಾನಿಸು HasRowid(X)     (((X)->tabFlags & TF_WithoutRowid)==0)
#ವ್ಯಾಖ್ಯಾನಿಸು VisibleRowid(X) (((X)->tabFlags & TF_NoVisibleRowid)==0)

/* Macro is true if the SQLITE_ALLOW_ROWID_IN_VIEW (mis-)feature is
** available.  By default, this macro is false
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ALLOW_ROWID_ಒಳ_VIEW
# define ViewCanHaveRowid     0
#ಇಲ್ಲದಿದ್ದರೆ
# define ViewCanHaveRowid     (sqlite3Config.mNoVisibleRowid==0)
#ಆದರೆ_ಅಂತ್ಯ

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
ರಚನೆ FKey {
  ಕೋಷ್ಟಕ *pFrom;     /* Table containing the REFERENCES clause (aka: Child) */
  FKey *p_ಮುಂದಿನ_From;  /* Next FKey with the same in pFrom. Next parent of pFrom */
  ಅಕ್ಷರ *z_ಗಮ್ಯ;        /* Name of table that the key points to (aka: Parent) */
  FKey *p_ಮುಂದಿನ_ಗಮ್ಯ;    /* Next with the same zTo. Next child of zTo. */
  FKey *p_Prev_ಗಮ್ಯ;    /* Previous with the same zTo */
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;         /* Number of columns in this key */
  /* EV: R-30323-21917 */
  u8 isDeferred;       /* True if constraint checking is deferred till COMMIT */
  u8 aAction[2];        /* ON DELETE and ON UPDATE actions, respectively */
  Trigger *apTrigger[2];/* Triggers for aAction[] actions */
  ರಚನೆ s_ಲಂಬಸಾಲು_Map {      /* Mapping of columns in pFrom to columns in zTo */
    ಪೂರ್ಣಾಂಕ iFrom;            /* Index of column in pFrom */
    ಅಕ್ಷರ *z_ಲಂಬಸಾಲು;           /* Name of column in zTo.  If NULL use PRIMARY KEY */
  } a_ಲಂಬಸಾಲು[FLEXARRAY];      /* One entry for each of nCol columns */
};

/* The size (in bytes) of an FKey object holding N columns.  The answer
** does NOT include space to hold the zTo name. */
#ವ್ಯಾಖ್ಯಾನಿಸು SZ_FKEY(N)  (offsetof(FKey,a_ಲಂಬಸಾಲು)+(N)*ಗಾತ್ರ(ರಚನೆ s_ಲಂಬಸಾಲು_Map))

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
#ವ್ಯಾಖ್ಯಾನಿಸು OE_None     0   /* There is no constraint to check */
#ವ್ಯಾಖ್ಯಾನಿಸು OE_Rollback 1   /* Fail the operation and rollback the transaction */
#ವ್ಯಾಖ್ಯಾನಿಸು OE_Abort    2   /* Back out changes but do no rollback transaction */
#ವ್ಯಾಖ್ಯಾನಿಸು OE_Fail     3   /* Stop the operation but leave all prior changes */
#ವ್ಯಾಖ್ಯಾನಿಸು OE_Ignore   4   /* Ignore the error. Do not do the INSERT or UPDATE */
#ವ್ಯಾಖ್ಯಾನಿಸು OE_Replace  5   /* Delete existing record, then do INSERT or UPDATE */
#ವ್ಯಾಖ್ಯಾನಿಸು OE_Update   6   /* Process as a DO UPDATE in an upsert */
#ವ್ಯಾಖ್ಯಾನಿಸು OE_Restrict 7   /* OE_Abort for IMMEDIATE, OE_Rollback for DEFERRED */
#ವ್ಯಾಖ್ಯಾನಿಸು OE_ಗಣ_Null  8   /* Set the foreign key value to NULL */
#ವ್ಯಾಖ್ಯಾನಿಸು OE_ಗಣ_Dflt  9   /* Set the foreign key value to its default */
#ವ್ಯಾಖ್ಯಾನಿಸು OE_Cascade  10  /* Cascade the changes */
#ವ್ಯಾಖ್ಯಾನಿಸು OE_Default  11  /* Do whatever the default action is */


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
ರಚನೆ ಕೀಲಿ_Info {
  u32 nRef;           /* Number of references to this KeyInfo object */
  u8 enc;             /* Text encoding - one of the SQLITE_UTF* values */
  u16 n_ಕೀಲಿ_ಕ್ಷೇತ್ರ;      /* Number of key columns in the index */
  u16 n_All_ಕ್ಷೇತ್ರ;      /* Total columns, including key plus others */
  sqlite3 *db;        /* The database connection */
  u8 *a_ವಿಂಗಡಿಸು_Flags;     /* Sort order for each column. */
  CollSeq *aColl[FLEXARRAY]; /* Collating sequence for each term of the key */
};

/* The size (in bytes) of a KeyInfo object with up to N fields.  This includes
** the main body of the KeyInfo object and the aColl[] array of N elements,
** but does not count the memory used to hold aSortFlags[]. */
#ವ್ಯಾಖ್ಯಾನಿಸು SZ_KEYINFO(N)  (offsetof(ಕೀಲಿ_Info,aColl) + (N)*ಗಾತ್ರ(CollSeq*))

/* The size of a bare KeyInfo with no aColl[] entries */
#ಆದರೆ FLEXARRAY+1 > 1
# define SZ_KEYINFO_0   offsetof(ಕೀಲಿ_Info,aColl)
#ಇಲ್ಲದಿದ್ದರೆ
# define SZ_KEYINFO_0   ಗಾತ್ರ(ಕೀಲಿ_Info)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Allowed bit values for entries in the KeyInfo.aSortFlags[] array.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು KEYINFO_ಕ್ರಮ_DESC    0x01    /* DESC sort order */
#ವ್ಯಾಖ್ಯಾನಿಸು KEYINFO_ಕ್ರಮ_BIGNULL 0x02    /* NULL is larger than any other value */

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
ರಚನೆ Unpacked_ದಾಖಲೆ {
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;  /* Comparison info for the index that is unpacked */
  Mem *aMem;          /* Values for columns of the index */
  ಒಕ್ಕೂಟ {
    ಅಕ್ಷರ *z;            /* Cache of aMem[0].z for vdbeRecordCompareString() */
    i64 i;              /* Cache of aMem[0].u.i for vdbeRecordCompareInt() */
  } u;
  ಪೂರ್ಣಾಂಕ n;              /* Cache of aMem[0].n used by vdbeRecordCompareString() */
  u16 n_ಕ್ಷೇತ್ರ;         /* Number of entries in apMem[] */
  i8 default_rc;      /* Comparison result if keys are equal */
  u8 err_ಸಂಕೇತ;         /* Error detected by xRecordCompare (CORRUPT or NOMEM) */
  i8 r1;              /* Value to return if (lhs < rhs) */
  i8 r2;              /* Value to return if (lhs > rhs) */
  u8 eq_ನೋಡಿದ;          /* True if an equality comparison has been seen */
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
ರಚನೆ ಸೂಚ್ಯಂಕ {
  ಅಕ್ಷರ *zName;             /* Name of this index */
  i16 *ai_ಸ್ತಂಭ;           /* Which columns are used by this index.  1st is 0 */
  LogEst *ai_ಅಡ್ಡಸಾಲು_Log_Est;     /* From ANALYZE: Est. rows selected by each column */
  ಕೋಷ್ಟಕ *p_ಕೋಷ್ಟಕ;           /* The SQL table being indexed */
  ಅಕ್ಷರ *z_ಲಂಬಸಾಲು_Aff;           /* String defining the affinity of each column */
  ಸೂಚ್ಯಂಕ *p_ಮುಂದಿನ;            /* The next index associated with the same table */
  Schema *pSchema;         /* Schema containing this index */
  u8 *a_ವಿಂಗಡಿಸು_ಕ್ರಮ;          /* for each column: True==DESC, False==ASC */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **azColl;     /* Array of collation sequence names for index */
  ಅಭಿವ್ಯಕ್ತಿ *pPartIdxWhere;     /* WHERE clause for partial indices */
  ಅಭಿವ್ಯಕ್ತಿ_List *a_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ;      /* Column expressions */
  Pgno tnum;               /* DB Page containing root of this index */
  LogEst sz_Idx_ಅಡ್ಡಸಾಲು;         /* Estimated average row size in bytes */
  u16 n_ಕೀಲಿ_ಲಂಬಸಾಲು;             /* Number of columns forming the key */
  u16 n_ಸ್ತಂಭ;             /* Nr columns in btree. Can be 2*Table.nCol */
  u8 onError;              /* OE_Abort, OE_Ignore, OE_Replace, or OE_None */
  ಚಿಹ್ನೆರಹಿತ idxType:2;      /* 0:Normal 1:UNIQUE, 2:PRIMARY KEY, 3:IPK */
  ಚಿಹ್ನೆರಹಿತ bUnordered:1;   /* Use this index for == or IN queries only */
  ಚಿಹ್ನೆರಹಿತ uniqNotNull:1;  /* True if UNIQUE and NOT NULL for all columns */
  ಚಿಹ್ನೆರಹಿತ isResized:1;    /* True if resizeIndexObject() has been called */
  ಚಿಹ್ನೆರಹಿತ isCovering:1;   /* True if this is a covering index */
  ಚಿಹ್ನೆರಹಿತ no_Skip_ಪರಿಶೀಲನೆ:1;   /* Do not try to use skip-scan if true */
  ಚಿಹ್ನೆರಹಿತ hasStat1:1;     /* aiRowLogEst values come from sqlite_stat1 */
  ಚಿಹ್ನೆರಹಿತ bNoQuery:1;     /* Do not use this index to optimize queries */
  ಚಿಹ್ನೆರಹಿತ b_Asc_ಕೀಲಿ_Bug:1;   /* True if the bba7b69f9849b5bf bug applies */
  ಚಿಹ್ನೆರಹಿತ bHasVCol:1;     /* Index references one or more VIRTUAL columns */
  ಚಿಹ್ನೆರಹಿತ b_Has_ಅಭಿವ್ಯಕ್ತಿ:1;     /* Index contains an expression, either a literal
                           ** expression, or a reference to a VIRTUAL column */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
  ಪೂರ್ಣಾಂಕ nSample;             /* Number of elements in aSample[] */
  ಪೂರ್ಣಾಂಕ mxSample;            /* Number of slots allocated to aSample[] */
  ಪೂರ್ಣಾಂಕ n_Sample_ಲಂಬಸಾಲು;          /* Size of IndexSample.anEq[] and so on */
  tRowcnt *aAvgEq;         /* Average nEq values for keys not in aSample */
  ಸೂಚ್ಯಂಕ_Sample *aSample;    /* Samples of the left-most key */
  tRowcnt *ai_ಅಡ್ಡಸಾಲು_Est;       /* Non-logarithmic stat1 data for this index */
  tRowcnt n_ಅಡ್ಡಸಾಲು_Est0;        /* Non-logarithmic number of rows in the index */
#ಆದರೆ_ಅಂತ್ಯ
  Bitmask ಲಂಬಸಾಲು_Not_Idxed;     /* Unindexed columns in pTab */
};

/*
** Allowed values for Index.idxType
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_IDXTYPE_APPDEF      0   /* Created using CREATE INDEX */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_IDXTYPE_UNIQUE      1   /* Implements a UNIQUE constraint */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_IDXTYPE_PRIMARYKEY  2   /* Is the PRIMARY KEY for the table */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_IDXTYPE_IPK         3   /* INTEGER PRIMARY KEY index */

/* Return true if index X is a PRIMARY KEY index */
#ವ್ಯಾಖ್ಯಾನಿಸು Is_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(X)  ((X)->idxType==SQLITE_IDXTYPE_PRIMARYKEY)

/* Return true if index X is a UNIQUE index */
#ವ್ಯಾಖ್ಯಾನಿಸು Is_Unique_ಸೂಚ್ಯಂಕ(X)      ((X)->onError!=OE_None)

/* The Index.aiColumn[] values are normally positive integer.  But
** there are some negative values that have special meaning:
*/
#ವ್ಯಾಖ್ಯಾನಿಸು XN_ROWID     (-1)     /* Indexed column is the rowid */
#ವ್ಯಾಖ್ಯಾನಿಸು XN_ಅಭಿವ್ಯಕ್ತಿ      (-2)     /* Indexed column is an expression */

/*
** Each sample stored in the sqlite_stat4 table is represented in memory
** using a structure of this type.  See documentation at the top of the
** analyze.c source file for additional information.
*/
ರಚನೆ ಸೂಚ್ಯಂಕ_Sample {
  ಶೂನ್ಯ *p;          /* Pointer to sampled record */
  ಪೂರ್ಣಾಂಕ n;            /* Size of record in bytes */
  tRowcnt *anEq;    /* Est. number of rows where the key equals this sample */
  tRowcnt *anLt;    /* Est. number of rows where key is less than this sample */
  tRowcnt *anDLt;   /* Est. number of distinct keys less than this sample */
};

/*
** Possible values to use within the flags argument to sqlite3GetToken().
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_TOKEN_QUOTED    0x1 /* Token is a quoted identifier. */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_TOKEN_KEYWORD   0x2 /* Token is a keyword. */

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
ರಚನೆ Token {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;     /* Text of the token.  Not NULL-terminated! */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n;    /* Number of characters in this token */
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
ರಚನೆ AggInfo {
  u8 ನೇರ_ಬಹುಲಕ;          /* Direct rendering mode means take data directly
                          ** from source tables rather than from accumulators */
  u8 use_ವಿಂಗಡಣೆ_Idx;       /* In direct mode, reference the sorting index rather
                          ** than the source table */
  u32 n_ವಿಂಗಡಣೆ_ಸ್ತಂಭ;     /* Number of columns in the sorting index */
  ಪೂರ್ಣಾಂಕ ವಿಂಗಡಣೆ_Idx;         /* Cursor number of the sorting index */
  ಪೂರ್ಣಾಂಕ ವಿಂಗಡಣೆ_Idx_PTab;     /* Cursor number of pseudo-table */
  ಪೂರ್ಣಾಂಕ i_ಮೊದಲ_Reg;          /* First register in range for aCol[] and aFunc[] */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಗುಂಪು_By;     /* The group by clause */
  ರಚನೆ Agg_Info_ಲಂಬಸಾಲು {    /* For each column used in source tables */
    ಕೋಷ್ಟಕ *pTab;             /* Source table */
    ಅಭಿವ್ಯಕ್ತಿ *pCExpr;            /* The original expression */
    ಪೂರ್ಣಾಂಕ i_ಕೋಷ್ಟಕ;              /* Cursor number of the source table */
    ಪೂರ್ಣಾಂಕ i_ಸ್ತಂಭ;             /* Column number within the source table */
    ಪೂರ್ಣಾಂಕ i_Sorter_ಸ್ತಂಭ;       /* Column number in the sorting index */
  } *a_ಲಂಬಸಾಲು;
  ಪೂರ್ಣಾಂಕ n_ಸ್ತಂಭ;            /* Number of used entries in aCol[] */
  ಪೂರ್ಣಾಂಕ n_ಸಂಚಯಕ;       /* Number of columns that show through to the output.
                          ** Additional columns are used only as parameters to
                          ** aggregate functions */
  ರಚನೆ AggInfo_func {   /* For each aggregate function */
    ಅಭಿವ್ಯಕ್ತಿ *pFExpr;            /* Expression encoding the function */
    FuncDef *pFunc;          /* The aggregate function implementation */
    ಪೂರ್ಣಾಂಕ iDistinct;           /* Ephemeral table used to enforce DISTINCT */
    ಪೂರ್ಣಾಂಕ iDistAddr;           /* Address of OP_OpenEphemeral */
    ಪೂರ್ಣಾಂಕ iOBTab;              /* Ephemeral table to implement ORDER BY */
    u8 bOBPayload;           /* iOBTab has payload columns separate from key */
    u8 bOBUnique;            /* Enforce uniqueness on iOBTab keys */
    u8 bUseSubtype;          /* Transfer subtype info through sorter */
  } *aFunc;
  ಪೂರ್ಣಾಂಕ nFunc;              /* Number of entries in aFunc[] */
  u32 selId;              /* Select to which this AggInfo belongs */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  Select *pSelect;        /* SELECT statement that this AggInfo supports */
#ಆದರೆ_ಅಂತ್ಯ
};

/*
** Macros to compute aCol[] and aFunc[] register numbers.
**
** These macros should not be used prior to the call to
** assignAggregateRegisters() that computes the value of pAggInfo->iFirstReg.
** The assert()s that are part of this macro verify that constraint.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
#ವ್ಯಾಖ್ಯಾನಿಸು Agg_Info_ಸ್ತಂಭ_Reg(A,I)  (ಪ್ರತಿಜ್ಞಾ((A)->i_ಮೊದಲ_Reg),(A)->i_ಮೊದಲ_Reg+(I))
#ವ್ಯಾಖ್ಯಾನಿಸು AggInfoFuncReg(A,I)    \
                      (ಪ್ರತಿಜ್ಞಾ((A)->i_ಮೊದಲ_Reg),(A)->i_ಮೊದಲ_Reg+(A)->n_ಸ್ತಂಭ+(I))
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು Agg_Info_ಸ್ತಂಭ_Reg(A,I)  ((A)->i_ಮೊದಲ_Reg+(I))
#ವ್ಯಾಖ್ಯಾನಿಸು AggInfoFuncReg(A,I)    \
                      ((A)->i_ಮೊದಲ_Reg+(A)->n_ಸ್ತಂಭ+(I))
#ಆದರೆ_ಅಂತ್ಯ

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
#ಆದರೆ SQLITE_MAX_VARIABLE_ಸಂಖ್ಯೆ<32767
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ i16 ynVar;
#ಇಲ್ಲದಿದ್ದರೆ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ ynVar;
#ಆದರೆ_ಅಂತ್ಯ

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
ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ {
  u8 op;                 /* Operation performed by this node */
  ಅಕ್ಷರ aff_ಅಭಿವ್ಯಕ್ತಿ;          /* affinity, or RAISE type */
  u8 op2;                /* TK_REGISTER/TK_TRUTH: original value of Expr.op
                         ** TK_COLUMN: the value of p5 for OP_Column
                         ** TK_AGG_FUNCTION: nesting depth
                         ** TK_FUNCTION: NC_SelfRef flag if needs OP_PureFunc */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  u8 vvaFlags;           /* Verification flags. */
#ಆದರೆ_ಅಂತ್ಯ
  u32 flags;             /* Various flags.  EP_* See below */
  ಒಕ್ಕೂಟ {
    ಅಕ್ಷರ *zToken;          /* Token value. Zero terminated and dequoted */
    ಪೂರ್ಣಾಂಕ i_ಮೌಲ್ಯ;            /* Non-negative integer value if EP_IntValue */
  } u;

  /* If the EP_TokenOnly flag is set in the Expr.flags mask, then no
  ** space is allocated for the fields below this point. An attempt to
  ** access them will result in a segfault or malfunction.
  *********************************************************************/

  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ;           /* Left subnode */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ;          /* Right subnode */
  ಒಕ್ಕೂಟ {
    ಅಭಿವ್ಯಕ್ತಿ_List *pList;     /* op = IN, EXISTS, SELECT, CASE, FUNCTION, BETWEEN */
    Select *pSelect;     /* EP_xIsSelect and op = IN, EXISTS, SELECT */
  } x;

  /* If the EP_Reduced flag is set in the Expr.flags mask, then no
  ** space is allocated for the fields below this point. An attempt to
  ** access them will result in a segfault or malfunction.
  *********************************************************************/

#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
  ಪೂರ್ಣಾಂಕ n_ಎತ್ತರ;           /* Height of the tree headed by this node */
#ಆದರೆ_ಅಂತ್ಯ
  ಪೂರ್ಣಾಂಕ i_ಕೋಷ್ಟಕ;            /* TK_COLUMN: cursor number of table holding column
                         ** TK_REGISTER: register number
                         ** TK_TRIGGER: 1 -> new, 0 -> old
                         ** EP_Unlikely:  134217728 times likelihood
                         ** TK_IN: ephemeral table holding RHS
                         ** TK_SELECT_COLUMN: Number of columns on the LHS
                         ** TK_SELECT: 1st register of result vector */
  ynVar i_ಸ್ತಂಭ;         /* TK_COLUMN: column index.  -1 for rowid.
                         ** TK_VARIABLE: variable number (always >= 1).
                         ** TK_SELECT_COLUMN: column of the result vector */
  i16 iAgg;              /* Which entry in pAggInfo->aCol[] or ->aFunc[] */
  ಒಕ್ಕೂಟ {
    ಪೂರ್ಣಾಂಕ iJoin;             /* If EP_OuterON or EP_InnerON, the right table */
    ಪೂರ್ಣಾಂಕ iOfst;             /* else: start of token from start of statement */
  } w;
  AggInfo *pAggInfo;     /* Used by TK_AGG_COLUMN and TK_AGG_FUNCTION */
  ಒಕ್ಕೂಟ {
    ಕೋಷ್ಟಕ *pTab;           /* TK_COLUMN: Table containing column. Can be NULL
                           ** for a column of an index on an expression */
    ಕಿಟಕಿ *pWin;          /* EP_WinFunc: Window/Filter defn for a function */
    ಪೂರ್ಣಾಂಕ nReg;              /* TK_NULLS: Number of registers to NULL out */
    ರಚನೆ {               /* TK_IN, TK_SELECT, and TK_EXISTS */
      ಪೂರ್ಣಾಂಕ iAddr;             /* Subroutine entry address */
      ಪೂರ್ಣಾಂಕ regReturn;         /* Register used to hold return address */
    } sub;
  } y;
};

/* The following are the meanings of bits in the Expr.flags field.
** Value restrictions:
**
**          EP_Agg == NC_HasAgg == SF_HasAgg
**          EP_Win == NC_HasWin
*/
#ವ್ಯಾಖ್ಯಾನಿಸು EP_OuterON    0x000001 /* Originates in ON/USING clause of outer join */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_InnerON    0x000002 /* Originates in ON/USING of an inner join */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Distinct   0x000004 /* Aggregate function with DISTINCT keyword */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_HasFunc    0x000008 /* Contains one or more functions of any kind */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Agg        0x000010 /* Contains one or more aggregate functions */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Fixed_ಲಂಬಸಾಲು   0x000020 /* TK_Column with a known fixed value */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_VarSelect  0x000040 /* pSelect is correlated, not constant */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_DblQuoted  0x000080 /* token.z was originally in "..." */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_InfixFunc  0x000100 /* True for an infix function: LIKE, GLOB, etc */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Collate    0x000200 /* Tree contains a TK_COLLATE operator */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Commuted   0x000400 /* Comparison operator has been commuted */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Int_ಮೌಲ್ಯ   0x000800 /* Integer value contained in u.iValue */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_xIsSelect  0x001000 /* x.pSelect is valid (otherwise x.pList is) */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Skip       0x002000 /* Operator does not contribute to affinity */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Reduced    0x004000 /* Expr struct EXPR_REDUCEDSIZE bytes only */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Win        0x008000 /* Contains window functions */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_TokenOnly  0x010000 /* Expr struct EXPR_TOKENONLYSIZE bytes only */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_ತುಂಬಿದ_ಗಾತ್ರ   0x020000 /* Expr structure must remain full sized */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_If_Null_ಅಡ್ಡಸಾಲು  0x040000 /* The TK_IF_NULL_ROW opcode */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Unlikely   0x080000 /* unlikely() or likelihood() function */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_ConstFunc  0x100000 /* A SQLITE_FUNC_CONSTANT or _SLOCHNG function */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_CanBeNull  0x200000 /* Can be null despite NOT NULL constraint */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Subquery   0x400000 /* Tree contains a TK_SELECT operator */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_ಎಲೆ       0x800000 /* Expr.pLeft, .pRight, .u.pSelect all NULL */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_WinFunc   0x1000000 /* TK_FUNCTION with Expr.y.pWin set */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Subrtn    0x2000000 /* Uses Expr.y.sub. TK_IN, _SELECT, or _EXISTS */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Quoted    0x4000000 /* TK_ID was originally quoted */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Static    0x8000000 /* Held in memory not obtained from malloc() */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_IsTrue   0x10000000 /* Always has boolean value of TRUE */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_IsFalse  0x20000000 /* Always has boolean value of FALSE */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_FromDDL  0x40000000 /* Originates from sqlite_schema */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_SubtArg  0x80000000 /* Is argument to SQLITE_SUBTYPE function */

/* The EP_Propagate mask is a set of properties that automatically propagate
** upwards into parent nodes.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Propagate (EP_Collate|EP_Subquery|EP_HasFunc)

/* Macros can be used to test, set, or clear bits in the
** Expr.flags field.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Has_Property(E,P)     (((E)->flags&(u32)(P))!=0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Has_All_Property(E,P)  (((E)->flags&(u32)(P))==(u32)(P))
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(E,P)     (E)->flags|=(u32)(P)
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(E,P)   (E)->flags&=~(u32)(P)
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Always_True(E)   (((E)->flags&(EP_OuterON|EP_IsTrue))==EP_IsTrue)
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Always_False(E)  (((E)->flags&(EP_OuterON|EP_IsFalse))==EP_IsFalse)
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Is_ತುಂಬಿದ_ಗಾತ್ರ(E)   (((E)->flags&(EP_Reduced|EP_TokenOnly))==0)

/* Macros used to ensure that the correct members of unions are accessed
** in Expr.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Use_UToken(E)    (((E)->flags&EP_Int_ಮೌಲ್ಯ)==0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Use_UValue(E)    (((E)->flags&EP_Int_ಮೌಲ್ಯ)!=0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Use_WOfst(E)     (((E)->flags&(EP_InnerON|EP_OuterON))==0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Use_WJoin(E)     (((E)->flags&(EP_InnerON|EP_OuterON))!=0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Use_XList(E)     (((E)->flags&EP_xIsSelect)==0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(E)   (((E)->flags&EP_xIsSelect)!=0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(E)      (((E)->flags&(EP_WinFunc|EP_Subrtn))==0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Use_YWin(E)      (((E)->flags&EP_WinFunc)!=0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_Use_YSub(E)      (((E)->flags&EP_Subrtn)!=0)

/* Flags for use with Expr.vvaFlags
*/
#ವ್ಯಾಖ್ಯಾನಿಸು EP_NoReduce   0x01  /* Cannot EXPRDUP_REDUCE this Expr */
#ವ್ಯಾಖ್ಯಾನಿಸು EP_Immutable  0x02  /* Do not change this Expr node */

/* The ExprSetVVAProperty() macro is used for Verification, Validation,
** and Accreditation only.  It works like ExprSetProperty() during VVA
** processes but is a no-op for delivery.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
# define ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_VVAProperty(E,P)   (E)->vvaFlags|=(P)
# define ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(E,P)   (((E)->vvaFlags&(P))!=0)
# define ಅಭಿವ್ಯಕ್ತಿ_Clear_VVAProperties(E) (E)->vvaFlags = 0
#ಇಲ್ಲದಿದ್ದರೆ
# define ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_VVAProperty(E,P)
# define ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(E,P)   0
# define ಅಭಿವ್ಯಕ್ತಿ_Clear_VVAProperties(E)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Macros to determine the number of bytes required by a normal Expr
** struct, an Expr struct with the EP_Reduced flag set in Expr.flags
** and an Expr struct with the EP_TokenOnly flag set.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_FULLSIZE           ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ)           /* Full size */
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_REDUCEDSIZE        offsetof(ಅಭಿವ್ಯಕ್ತಿ,i_ಕೋಷ್ಟಕ)  /* Common features */
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಭಿವ್ಯಕ್ತಿ_TOKENONLYSIZE      offsetof(ಅಭಿವ್ಯಕ್ತಿ,p_ಎಡ)   /* Fewer features */

/*
** Flags passed to the sqlite3ExprDup() function. See the header comment
** above sqlite3ExprDup() for details.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು EXPRDUP_REDUCE         0x0001  /* Used reduced-size Expr nodes */

/*
** True if the expression passed as an argument was a function with
** an OVER() clause (a window function).
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_WINDOWFUNC
# define Is_ಕಿಟಕಿ_Func(p) 0
#ಇಲ್ಲದಿದ್ದರೆ
# define Is_ಕಿಟಕಿ_Func(p) ( \
    ಅಭಿವ್ಯಕ್ತಿ_Has_Property((p), EP_WinFunc) && p->y.pWin->eFrmType!=TK_FILTER \
 )
#ಆದರೆ_ಅಂತ್ಯ

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
ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List {
  ಪೂರ್ಣಾಂಕ n_ಅಭಿವ್ಯಕ್ತಿ;             /* Number of expressions on the list */
  ಪೂರ್ಣಾಂಕ nAlloc;            /* Number of a[] slots allocated */
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ { /* For each expression in the list */
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ;            /* The parse tree for this expression */
    ಅಕ್ಷರ *zEName;           /* Token associated with this expression */
    ರಚನೆ {
      u8 ವಿಂಗಡಿಸು_Flags;           /* Mask of KEYINFO_ORDER_* flags */
      ಚಿಹ್ನೆರಹಿತ eEName :2;     /* Meaning of zEName */
      ಚಿಹ್ನೆರಹಿತ ಮುಗಿದ :1;       /* Indicates when processing is finished */
      ಚಿಹ್ನೆರಹಿತ reusable :1;   /* Constant expression is reusable */
      ಚಿಹ್ನೆರಹಿತ bSorterRef :1; /* Defer evaluation until after sorting */
      ಚಿಹ್ನೆರಹಿತ bNulls :1;     /* True if explicit "NULLS FIRST/LAST" */
      ಚಿಹ್ನೆರಹಿತ b_ಬಳಸಿದ :1;      /* This column used in a SF_NestedFrom subquery */
      ಚಿಹ್ನೆರಹಿತ b_Using_ಪದ:1;  /* Term from the USING clause of a NestedFrom */
      ಚಿಹ್ನೆರಹಿತ bNoExpand: 1;  /* Term is an auxiliary in NestedFrom and should
                              ** not be expanded by "*" in parent queries */
    } fg;
    ಒಕ್ಕೂಟ {
      ರಚನೆ {             /* Used by any ExprList other than Parse.pConsExpr */
        u16 i_ಕ್ರಮ_By_ಲಂಬಸಾಲು;      /* For ORDER BY, column number in result set */
        u16 iAlias;           /* Index into Parse.aAlias[] for zName */
      } x;
      ಪೂರ್ಣಾಂಕ i_Const_ಅಭಿವ್ಯಕ್ತಿ_Reg;   /* Register in which Expr value is cached. Used only
                           ** by Parse.pConstExpr */
    } u;
  } a[FLEXARRAY];          /* One slot for each expression in the list */
};

/* The size (in bytes) of an ExprList object that is big enough to hold
** as many as N expressions. */
#ವ್ಯಾಖ್ಯಾನಿಸು SZ_EXPRLIST(N)  \
             (offsetof(ಅಭಿವ್ಯಕ್ತಿ_List,a) + (N)*ಗಾತ್ರ(ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ))

/*
** Allowed values for Expr.a.eEName
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ENAME_NAME  0       /* The AS clause of a result set */
#ವ್ಯಾಖ್ಯಾನಿಸು ENAME_ಹರವು  1       /* Complete text of the result set expression */
#ವ್ಯಾಖ್ಯಾನಿಸು ENAME_TAB   2       /* "DB.TABLE.NAME" for the result set */
#ವ್ಯಾಖ್ಯಾನಿಸು ENAME_ROWID 3       /* "DB.TABLE._rowid_" for * expansion of rowid */

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
ರಚನೆ IdList {
  ಪೂರ್ಣಾಂಕ nId;         /* Number of identifiers on the list */
  ರಚನೆ Id_List_ಅಂಶ {
    ಅಕ್ಷರ *zName;      /* Name of the identifier */
  } a[FLEXARRAY];
};

/* The size (in bytes) of an IdList object that can hold up to N IDs. */
#ವ್ಯಾಖ್ಯಾನಿಸು SZ_IDLIST(N)  (offsetof(IdList,a)+(N)*ಗಾತ್ರ(ರಚನೆ Id_List_ಅಂಶ))

/*
** Allowed values for IdList.eType, which determines which value of the a.u4
** is valid.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು EU4_NONE   0   /* Does not use IdList.a.u4 */
#ವ್ಯಾಖ್ಯಾನಿಸು EU4_IDX    1   /* Uses IdList.a.u4.idx */
#ವ್ಯಾಖ್ಯಾನಿಸು EU4_ಅಭಿವ್ಯಕ್ತಿ   2   /* Uses IdList.a.u4.pExpr -- NOT CURRENTLY USED */

/*
** Details of the implementation of a subquery.
*/
ರಚನೆ Subquery {
  Select *pSelect;  /* A SELECT statement used in place of a table name */
  ಪೂರ್ಣಾಂಕ addrFillSub;  /* Address of subroutine to initialize a subquery */
  ಪೂರ್ಣಾಂಕ regReturn;    /* Register holding return address of addrFillSub */
  ಪೂರ್ಣಾಂಕ reg_ಫಲಿತಾಂಶ;    /* Registers holding results of a co-routine */
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
ರಚನೆ Src_ಅಂಶ {
  ಅಕ್ಷರ *zName;      /* Name of the table */
  ಅಕ್ಷರ *zAlias;     /* The "B" part of a "A AS B" phrase.  zName is the "A" */
  ಕೋಷ್ಟಕ *pSTab;     /* Table object for zName. Mnemonic: Srcitem-TABle */
  ರಚನೆ {
    u8 jointype;      /* Type of join between this table and the previous */
    ಚಿಹ್ನೆರಹಿತ not_ಸೂಚ್ಯಂಕಿತ :1;    /* True if there is a NOT INDEXED clause */
    ಚಿಹ್ನೆರಹಿತ is_ಸೂಚ್ಯಂಕಿತ_By :1;   /* True if there is an INDEXED BY clause */
    ಚಿಹ್ನೆರಹಿತ isSubquery :1;    /* True if this term is a subquery */
    ಚಿಹ್ನೆರಹಿತ isTabFunc :1;     /* True if table-valued-function syntax */
    ಚಿಹ್ನೆರಹಿತ isCorrelated :1;  /* True if sub-query is correlated */
    ಚಿಹ್ನೆರಹಿತ isMaterialized:1; /* This is a materialized view */
    ಚಿಹ್ನೆರಹಿತ ಮೂಲಕ_Coroutine :1;  /* Implemented as a co-routine */
    ಚಿಹ್ನೆರಹಿತ isRecursive :1;   /* True for recursive reference in WITH */
    ಚಿಹ್ನೆರಹಿತ fromDDL :1;       /* Comes from sqlite_schema */
    ಚಿಹ್ನೆರಹಿತ isCte :1;         /* This is a CTE */
    ಚಿಹ್ನೆರಹಿತ notCte :1;        /* This item may not match a CTE */
    ಚಿಹ್ನೆರಹಿತ isUsing :1;       /* u3.pUsing is valid */
    ಚಿಹ್ನೆರಹಿತ isOn :1;          /* u3.pOn was once valid and non-NULL */
    ಚಿಹ್ನೆರಹಿತ isSynthUsing :1;  /* u3.pUsing is synthesized from NATURAL */
    ಚಿಹ್ನೆರಹಿತ isNestedFrom :1;  /* pSelect is a SF_NestedFrom subquery */
    ಚಿಹ್ನೆರಹಿತ rowid_ಬಳಸಿದ :1;     /* The ROWID of this table is referenced */
    ಚಿಹ್ನೆರಹಿತ fixedSchema :1;   /* Uses u4.pSchema, not u4.zDatabase */
    ಚಿಹ್ನೆರಹಿತ hadSchema :1;     /* Had u4.zDatabase before u4.pSchema */
    ಚಿಹ್ನೆರಹಿತ fromExists :1;    /* Comes from WHERE EXISTS(...) */
  } fg;
  ಪೂರ್ಣಾಂಕ iCursor;      /* The VDBE cursor number used to access this table */
  Bitmask ಲಂಬಸಾಲು_ಬಳಸಿದ;  /* Bit N set if column N used. Details above for N>62 */
  ಒಕ್ಕೂಟ {
    ಅಕ್ಷರ *z_ಸೂಚ್ಯಂಕಿತ_By;    /* Identifier from "INDEXED BY <zIndex>" clause */
    ಅಭಿವ್ಯಕ್ತಿ_List *pFuncArg;  /* Arguments to table-valued-function */
    u32 n_ಅಡ್ಡಸಾಲು;            /* Number of rows in a VALUES clause */
  } u1;
  ಒಕ್ಕೂಟ {
    ಸೂಚ್ಯಂಕ *pIBIndex;  /* Index structure corresponding to u1.zIndexedBy */
    CteUse *pCteUse;  /* CTE Usage info when fg.isCte is true */
  } u2;
  ಒಕ್ಕೂಟ {
    ಅಭಿವ್ಯಕ್ತಿ *pOn;        /* fg.isUsing==0 =>  The ON clause of a join */
    IdList *pUsing;   /* fg.isUsing==1 =>  The USING clause of a join */
  } u3;
  ಒಕ್ಕೂಟ {
    Schema *pSchema;  /* Schema to which this item is fixed */
    ಅಕ್ಷರ *zDatabase;  /* Name of database holding this table */
    Subquery *pSubq;  /* Description of a subquery */
  } u4;
};

/*
** The OnOrUsing object represents either an ON clause or a USING clause.
** It can never be both at the same time, but it can be neither.
*/
ರಚನೆ OnOrUsing {
  ಅಭಿವ್ಯಕ್ತಿ *pOn;         /* The ON clause of a join */
  IdList *pUsing;    /* The USING clause of a join */
};

/*
** This object represents one or more tables that are the source of
** content for an SQL statement.  For example, a single SrcList object
** is used to hold the FROM clause of a SELECT statement.  SrcList also
** represents the target tables for DELETE, INSERT, and UPDATE statements.
**
*/
ರಚನೆ SrcList {
  ಪೂರ್ಣಾಂಕ nSrc;             /* Number of tables or subqueries in the FROM clause */
  u32 nAlloc;           /* Number of entries allocated in a[] below */
  Src_ಅಂಶ a[FLEXARRAY]; /* One entry for each identifier on the list */
};

/* Size (in bytes) of a SrcList object that can hold as many as N
** SrcItem objects. */
#ವ್ಯಾಖ್ಯಾನಿಸು SZ_SRCLIST(N) (offsetof(SrcList,a)+(N)*ಗಾತ್ರ(Src_ಅಂಶ))

/* Size (in bytes( of a SrcList object that holds 1 SrcItem.  This is a
** special case of SZ_SRCITEM(1) that comes up often. */
#ವ್ಯಾಖ್ಯಾನಿಸು SZ_SRCLIST_1  (offsetof(SrcList,a)+ಗಾತ್ರ(Src_ಅಂಶ))

/*
** Permitted values of the SrcList.a.jointype field
*/
#ವ್ಯಾಖ್ಯಾನಿಸು JT_INNER     0x01    /* Any kind of inner or cross join */
#ವ್ಯಾಖ್ಯಾನಿಸು JT_CROSS     0x02    /* Explicit use of the CROSS keyword */
#ವ್ಯಾಖ್ಯಾನಿಸು JT_NATURAL   0x04    /* True for a "natural" join */
#ವ್ಯಾಖ್ಯಾನಿಸು JT_ಎಡ      0x08    /* Left outer join */
#ವ್ಯಾಖ್ಯಾನಿಸು JT_ಬಲ     0x10    /* Right outer join */
#ವ್ಯಾಖ್ಯಾನಿಸು JT_OUTER     0x20    /* The "OUTER" keyword is present */
#ವ್ಯಾಖ್ಯಾನಿಸು JT_LTORJ     0x40    /* One of the LEFT operands of a RIGHT JOIN
                             ** Mnemonic: Left Table Of Right Join */
#ವ್ಯಾಖ್ಯಾನಿಸು JT_ERROR     0x80    /* unknown or unsupported join type */

/*
** Flags appropriate for the wctrlFlags parameter of sqlite3WhereBegin()
** and the WhereInfo.wctrlFlags member.
**
** Value constraints (enforced via assert()):
**     WHERE_USE_LIMIT  == SF_FixedLimit
*/
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ORDERBY_ಸಾಮಾನ್ಯ   0x0000 /* No-op */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ORDERBY_MIN      0x0001 /* ORDER BY processing for min() func */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ORDERBY_MAX      0x0002 /* ORDER BY processing for max() func */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ONEPASS_DESIRED  0x0004 /* Want to do one-pass UPDATE/DELETE */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ONEPASS_MULTIROW 0x0008 /* ONEPASS is ok with multiple rows */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_DUPLICATES_OK    0x0010 /* Ok to return a row more than once */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_OR_SUBCLAUSE     0x0020 /* Processing a sub-WHERE as part of
                                      ** the OR optimization  */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_GROUPBY          0x0040 /* pOrderBy is really a GROUP BY */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_DISTINCTBY       0x0080 /* pOrderby is really a DISTINCT clause */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_WANT_DISTINCT    0x0100 /* All output needs to be distinct */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_SORTBYGROUP      0x0200 /* Support sqlite3WhereIsSorted() */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_AGG_DISTINCT     0x0400 /* Query is "SELECT agg(DISTINCT ...)" */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ORDERBY_ಎಲ್ಲೆ    0x0800 /* ORDERBY+LIMIT on the inner loop */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_ಬಲ_JOIN       0x1000 /* Processing a RIGHT JOIN */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_KEEP_ALL_JOINS   0x2000 /* Do not do the omit-noop-join opt */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_USE_ಎಲ್ಲೆ        0x4000 /* Use the LIMIT in cost estimates */
                        /*     0x8000    not currently used */

/* Allowed return values from sqlite3WhereIsDistinct()
*/
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_DISTINCT_NOOP      0  /* DISTINCT keyword not used */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_DISTINCT_UNIQUE    1  /* No duplicates */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_DISTINCT_ORDERED   2  /* All duplicates are adjacent */
#ವ್ಯಾಖ್ಯಾನಿಸು WHERE_DISTINCT_UNORDERED 3  /* Duplicates are scattered */

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
ರಚನೆ NameContext {
  Parse *pParse;       /* The parser */
  SrcList *pSrcList;   /* One or more tables used to resolve names */
  ಒಕ್ಕೂಟ {
    ಅಭಿವ್ಯಕ್ತಿ_List *pEList;    /* Optional list of result-set columns */
    AggInfo *pAggInfo;   /* Information about aggregates at this level */
    Upsert *pUpsert;     /* ON CONFLICT clause information from an upsert */
    ಪೂರ್ಣಾಂಕ i_ತಳ_Reg;        /* For TK_REGISTER when parsing RETURNING */
  } uNC;
  NameContext *p_ಮುಂದಿನ;  /* Next outer name context.  NULL for outermost */
  ಪೂರ್ಣಾಂಕ nRef;            /* Number of names resolved by this context */
  ಪೂರ್ಣಾಂಕ nNcErr;          /* Number of errors encountered while resolving names */
  ಪೂರ್ಣಾಂಕ ncFlags;         /* Zero or more NC_* flags defined below */
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
#ವ್ಯಾಖ್ಯಾನಿಸು NC_AllowAgg  0x000001 /* Aggregate functions are allowed here */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_PartIdx   0x000002 /* True if resolving a partial index WHERE */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_IsCheck   0x000004 /* True if resolving a CHECK constraint */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_Gen_ಲಂಬಸಾಲು    0x000008 /* True for a GENERATED ALWAYS AS clause */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_HasAgg    0x000010 /* One or more aggregate functions seen */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_Idx_ಅಭಿವ್ಯಕ್ತಿ   0x000020 /* True if resolving columns of CREATE INDEX */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_SelfRef   0x00002e /* Combo: PartIdx, isCheck, GenCol, and IdxExpr */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_Subquery  0x000040 /* A subquery has been seen */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_UEList    0x000080 /* True if uNC.pEList is used */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_UAggInfo  0x000100 /* True if uNC.pAggInfo is used */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_UUpsert   0x000200 /* True if uNC.pUpsert is used */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_UBaseReg  0x000400 /* True if uNC.iBaseReg is used */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_MinMaxAgg 0x001000 /* min/max aggregates seen.  See note above */
/*                   0x002000 // available for reuse */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_AllowWin  0x004000 /* Window functions are allowed here */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_HasWin    0x008000 /* One or more window functions seen */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_IsDDL     0x010000 /* Resolving names in a CREATE statement */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_ಒಳ_Agg_Func 0x020000 /* True if analyzing arguments to an agg func */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_FromDDL   0x040000 /* SQL text comes from sqlite_schema */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_NoSelect  0x080000 /* Do not descend into sub-selects */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_Where     0x100000 /* Processing WHERE clause of a SELECT */
#ವ್ಯಾಖ್ಯಾನಿಸು NC_ಕ್ರಮ_Agg 0x8000000 /* Has an aggregate other than count/min/max */

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
ರಚನೆ Upsert {
  ಅಭಿವ್ಯಕ್ತಿ_List *p_Upsert_ಗುರಿ;  /* Optional description of conflict target */
  ಅಭಿವ್ಯಕ್ತಿ *p_Upsert_ಗುರಿ_Where; /* WHERE clause for partial index targets */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_Upsert_ಗಣ;     /* The SET clause from an ON CONFLICT UPDATE */
  ಅಭಿವ್ಯಕ್ತಿ *pUpsertWhere;       /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_ಮುಂದಿನ_Upsert;      /* Next ON CONFLICT clause in the list */
  u8 isDoUpdate;            /* True for DO UPDATE.  False for DO NOTHING */
  u8 isDup;                 /* True if 2nd or later with same pUpsertIdx */
  /* Above this point is the parse tree for the ON CONFLICT clauses.
  ** The next group of fields stores intermediate data. */
  ಶೂನ್ಯ *p_ಗಮ್ಯ_Free;            /* Free memory when deleting the Upsert object */
  /* All fields above are owned by the Upsert object and must be freed
  ** when the Upsert is destroyed.  The fields below are used to transfer
  ** information from the INSERT processing down into the UPDATE processing
  ** while generating code.  The fields below are owned by the INSERT
  ** statement and will be freed by INSERT processing. */
  ಸೂಚ್ಯಂಕ *pUpsertIdx;        /* UNIQUE constraint specified by pUpsertTarget */
  SrcList *pUpsertSrc;      /* Table to be updated */
  ಪೂರ್ಣಾಂಕ reg_ದತ್ತಾಂಶ;              /* First register holding array of VALUES */
  ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur;             /* Index of the data cursor */
  ಪೂರ್ಣಾಂಕ iIdxCur;              /* Index of the first index cursor */
};

/*
** An instance of the following structure contains all information
** needed to generate code for a single SELECT statement.
*/
ರಚನೆ Select {
  u8 op;                 /* One of: TK_UNION TK_ALL TK_INTERSECT TK_EXCEPT */
  LogEst n_Select_ಅಡ್ಡಸಾಲು;     /* Estimated number of result rows */
  u32 selFlags;          /* Various SF_* values */
  ಪೂರ್ಣಾಂಕ i_ಎಲ್ಲೆ, i_ಸ್ಥಾನಾಂತರ;   /* Memory registers holding LIMIT & OFFSET counters */
  u32 selId;             /* Unique identifier number for this SELECT */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;      /* The fields of the result */
  SrcList *pSrc;         /* The FROM clause */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere;          /* The WHERE clause */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಗುಂಪು_By;    /* The GROUP BY clause */
  ಅಭಿವ್ಯಕ್ತಿ *pHaving;         /* The HAVING clause */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By;    /* The ORDER BY clause */
  Select *p_ಪೂರ್ವ;        /* Prior select in a compound select statement */
  Select *p_ಮುಂದಿನ;         /* Next select to the left in a compound */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಲ್ಲೆ;          /* LIMIT expression. NULL means not used. */
  With *pWith;           /* WITH clause attached to this select. Or NULL. */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
  ಕಿಟಕಿ *pWin;          /* List of window functions */
  ಕಿಟಕಿ *pWinDefn;      /* List of named window definitions */
#ಆದರೆ_ಅಂತ್ಯ
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
#ವ್ಯಾಖ್ಯಾನಿಸು SF_Distinct      0x0000001 /* Output should be DISTINCT */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_All           0x0000002 /* Includes the ALL keyword */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_Resolved      0x0000004 /* Identifiers have been resolved */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_Aggregate     0x0000008 /* Contains agg functions or a GROUP BY */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_HasAgg        0x0000010 /* Contains aggregate functions */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_Cloned_Rhs_ಒಳ   0x0000020 /* Cloned RHS of an IN operator */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_Expanded      0x0000040 /* sqlite3SelectExpand() called on this */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_HasTypeInfo   0x0000080 /* FROM subqueries have Table metadata */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_Compound      0x0000100 /* Part of a compound query */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_ಮೌಲ್ಯಗಳು        0x0000200 /* Synthesized from VALUES clause */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_Multi_ಮೌಲ್ಯ    0x0000400 /* Single VALUES term with multiple rows */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_NestedFrom    0x0000800 /* Part of a parenthesized FROM clause */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_MinMaxAgg     0x0001000 /* Aggregate containing min() or max() */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_Recursive     0x0002000 /* The recursive part of a recursive CTE */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_Fixed_ಎಲ್ಲೆ    0x0004000 /* nSelectRow set by a constant LIMIT */
/*                       0x0008000 // available for reuse */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_Converted     0x0010000 /* By convertCompoundSelectToSubquery() */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_IncludeHidden 0x0020000 /* Include hidden columns in output */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_Complex_ಫಲಿತಾಂಶ 0x0040000 /* Result contains subquery or function */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_WhereBegin    0x0080000 /* Really a WhereBegin() call.  Debug Only */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_WinRewrite    0x0100000 /* Window function rewrite accomplished */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_View          0x0200000 /* SELECT statement is a view */
/*                       0x0400000 // available for reuse */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_UFSrcCheck    0x0800000 /* Check pSrc as required by UPDATE...FROM */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_ತಳ್ಳು_ಕೆಳಕ್ಕೆ      0x1000000 /* Modified by WHERE-clause push-down opt */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_MultiPart     0x2000000 /* Has multiple incompatible PARTITIONs */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_CopyCte       0x4000000 /* SELECT statement is a copy of a CTE */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_ಕ್ರಮ_By_Reqd   0x8000000 /* The ORDER BY clause may not be omitted */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_UpdateFrom   0x10000000 /* Query originates with UPDATE FROM */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_Correlated   0x20000000 /* True if references the outer context */
#ವ್ಯಾಖ್ಯಾನಿಸು SF_On_ಗಮ್ಯ_Where    0x40000000 /* One or more ON clauses moved to WHERE */

/* True if SrcItem X is a subquery that has SF_NestedFrom */
#ವ್ಯಾಖ್ಯಾನಿಸು IsNestedFrom(X) \
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
#ವ್ಯಾಖ್ಯಾನಿಸು SRT_Exists       1  /* Store 1 if the result is not empty */
#ವ್ಯಾಖ್ಯಾನಿಸು SRT_Discard      2  /* Do not save the results anywhere */
#ವ್ಯಾಖ್ಯಾನಿಸು SRT_DistFifo     3  /* Like SRT_Fifo, but unique results only */
#ವ್ಯಾಖ್ಯಾನಿಸು SRT_Dist_ಸರತಿ    4  /* Like SRT_Queue, but unique results only */

/* The DISTINCT clause is ignored for all of the above.  Not that
** IgnorableDistinct() implies IgnorableOrderby() */
#ವ್ಯಾಖ್ಯಾನಿಸು IgnorableDistinct(X) ((X->eDest)<=SRT_Dist_ಸರತಿ)

#ವ್ಯಾಖ್ಯಾನಿಸು SRT_ಸರತಿ        5  /* Store result in an queue */
#ವ್ಯಾಖ್ಯಾನಿಸು SRT_Fifo         6  /* Store result as data with an automatic rowid */

/* The ORDER BY clause is ignored for all of the above */
#ವ್ಯಾಖ್ಯಾನಿಸು IgnorableOrderby(X) ((X->eDest)<=SRT_Fifo)

#ವ್ಯಾಖ್ಯಾನಿಸು SRT_Output       7  /* Output each row of result */
#ವ್ಯಾಖ್ಯಾನಿಸು SRT_Mem          8  /* Store result in a memory cell */
#ವ್ಯಾಖ್ಯಾನಿಸು SRT_ಗಣ          9  /* Store results as keys in an index */
#ವ್ಯಾಖ್ಯಾನಿಸು SRT_EphemTab    10  /* Create transient tab and store like SRT_Table */
#ವ್ಯಾಖ್ಯಾನಿಸು SRT_Coroutine   11  /* Generate a single row of result */
#ವ್ಯಾಖ್ಯಾನಿಸು SRT_ಕೋಷ್ಟಕ       12  /* Store result as data with an automatic rowid */
#ವ್ಯಾಖ್ಯಾನಿಸು SRT_Upfrom      13  /* Store result as data with rowid */

/*
** An instance of this object describes where to put of the results of
** a SELECT statement.
*/
ರಚನೆ SelectDest {
  u8 eDest;            /* How to dispose of the results.  One of SRT_* above. */
  ಪೂರ್ಣಾಂಕ iSDParm;         /* A parameter used by the eDest disposal method */
  ಪೂರ್ಣಾಂಕ iSDParm2;        /* A second parameter for the eDest disposal method */
  ಪೂರ್ಣಾಂಕ iSdst;           /* Base register where results are written */
  ಪೂರ್ಣಾಂಕ nSdst;           /* Number of registers allocated */
  ಅಕ್ಷರ *zAffSdst;      /* Affinity used for SRT_Set */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By;  /* Key columns for SRT_Queue and SRT_DistQueue */
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
ರಚನೆ AutoincInfo {
  AutoincInfo *p_ಮುಂದಿನ;   /* Next info block in a list of them all */
  ಕೋಷ್ಟಕ *pTab;          /* Table this info block refers to */
  ಪೂರ್ಣಾಂಕ iDb;              /* Index in sqlite3.aDb[] of database holding pTab */
  ಪೂರ್ಣಾಂಕ regCtr;           /* Memory register holding the rowid counter */
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
ರಚನೆ TriggerPrg {
  Trigger *pTrigger;      /* Trigger this program was coded from */
  TriggerPrg *p_ಮುಂದಿನ;      /* Next entry in Parse.pTriggerPrg list */
  Sub_ಕಾರ್ಯಕ್ರಮ *p_ಕಾರ್ಯಕ್ರಮ;   /* Program implementing pTrigger/orconf */
  ಪೂರ್ಣಾಂಕ orconf;             /* Default ON CONFLICT policy */
  u32 aColmask[2];        /* Masks of old.*, new.* columns accessed */
};

/*
** The yDbMask datatype for the bitmask of all attached databases.
*/
#ಆದರೆ SQLITE_MAX_ATTACHED>30
  ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ y_Db_ಮುಖವಾಡ[(SQLITE_MAX_ATTACHED+9)/8];
# define Db_ಮುಖವಾಡ_Test(M,I)    (((M)[(I)/8]&(1<<((I)&7)))!=0)
# define Db_ಮುಖವಾಡ_Zero(M)      memset((M),0,ಗಾತ್ರ(M))
# define Db_ಮುಖವಾಡ_ಗಣ(M,I)     (M)[(I)/8]|=(1<<((I)&7))
# define Db_ಮುಖವಾಡ_All_Zero(M)   sqlite3_Db_ಮುಖವಾಡ_All_Zero(M)
# define Db_ಮುಖವಾಡ_Non_Zero(M)   (sqlite3_Db_ಮುಖವಾಡ_All_Zero(M)==0)
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ y_Db_ಮುಖವಾಡ;
# define Db_ಮುಖವಾಡ_Test(M,I)    (((M)&(((y_Db_ಮುಖವಾಡ)1)<<(I)))!=0)
# define Db_ಮುಖವಾಡ_Zero(M)      ((M)=0)
# define Db_ಮುಖವಾಡ_ಗಣ(M,I)     ((M)|=(((y_Db_ಮುಖವಾಡ)1)<<(I)))
# define Db_ಮುಖವಾಡ_All_Zero(M)   ((M)==0)
# define Db_ಮುಖವಾಡ_Non_Zero(M)   ((M)!=0)
#ಆದರೆ_ಅಂತ್ಯ

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
ರಚನೆ ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ {
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ;            /* The expression contained in the index */
  ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ_Cur;           /* The data cursor associated with the index */
  ಪೂರ್ಣಾಂಕ iIdxCur;            /* The index cursor */
  ಪೂರ್ಣಾಂಕ i_Idx_ಲಂಬಸಾಲು;            /* The index column that contains value of pExpr */
  u8 b_Maybe_Null_ಅಡ್ಡಸಾಲು;       /* True if we need an OP_IfNullRow check */
  u8 aff;                 /* Affinity of the pExpr expression */
  ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ *pIENext;   /* Next in a list of all indexed expressions */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_EXPLAIN_COMMENTS
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zIdxName;   /* Name of index, used only for bytecode comments */
#ಆದರೆ_ಅಂತ್ಯ
};

/*
** An instance of the ParseCleanup object specifies an operation that
** should be performed after parsing to deallocation resources obtained
** during the parse and which are no longer needed.
*/
ರಚನೆ ParseCleanup {
  ParseCleanup *p_ಮುಂದಿನ;               /* Next cleanup task */
  ಶೂನ್ಯ *pPtr;                        /* Pointer to object to deallocate */
  ಶೂನ್ಯ (*xCleanup)(sqlite3*,ಶೂನ್ಯ*);  /* Deallocation routine */
};

/*
** Number of 64-bit entried in the variable number bitmap cache (VNBMC).
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_VNBMC
# define SQLITE_VNBMC 2
#ಆದರೆ_ಅಂತ್ಯ

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
ರಚನೆ Parse {
  sqlite3 *db;         /* The main database structure */
  ಅಕ್ಷರ *z_Err_ಸಂದೇಶ;       /* An error message */
  Vdbe *pVdbe;         /* An engine for executing database bytecode */
  ಪೂರ್ಣಾಂಕ rc;              /* Return code from execution */
  LogEst nQueryLoop;   /* Est number of iterations of a query (10*log2(N)) */
  u8 nested;           /* Number of nested calls to the parser/code generator */
  u8 n_ತಾತ್ಕಾಲಿಕ_Reg;         /* Number of temporary registers in aTempReg[] */
  u8 isMultiWrite;     /* True if statement may modify/insert multiple rows */
  u8 disableLookaside; /* Number of times lookaside has been disabled */
  u8 prepFlags;        /* SQLITE_PREPARE_* flags */
  u8 withinRJSubrtn;   /* Nesting level for RIGHT JOIN body subroutines */
  u8 mSubrtnSig;       /* mini Bloom filter on available SubrtnSig.selId */
  u8 eTriggerOp;       /* TK_UPDATE, TK_INSERT or TK_DELETE */
  u8 eOrconf;          /* Default ON CONFLICT policy for trigger steps */
#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
  u8 earlyCleanup;     /* OOM inside sqlite3ParserAddCleanup() */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  u8 ifNotExists;      /* Might be true if IF NOT EXISTS.  Assert()s only */
  u8 isCreate;         /* CREATE TABLE, INDEX, or VIEW (but not TRIGGER)
                       ** and ALTER TABLE ADD COLUMN. */
#ಆದರೆ_ಅಂತ್ಯ
  bft disableTriggers:1; /* True to disable triggers */
  bft mayAbort :1;     /* True if statement may throw an ABORT exception */
  bft hasCompound :1;  /* Need to invoke convertCompoundSelectToSubquery() */
  bft bReturning :1;   /* Coding a RETURNING trigger */
  bft bHasExists :1;   /* Has a correlated "EXISTS (SELECT ....)" expression */
  bft ಲಂಬಸಾಲು_ಹೆಸರುಗಳು_ಗಣ :1;  /* TRUE after OP_ColumnName has been issued to pVdbe */
  bft bHasWith :1;     /* True if statement contains WITH */
  bft ok_Const_ಗುಣಕ:1; /* OK to factor out constants */
  bft checkSchema :1;  /* Causes schema cookie check after an error */
  bft ಬಳಸುತ್ತದೆ_Ainc :1;     /* True if pAinc is valid */
  ಪೂರ್ಣಾಂಕ nRangeReg;       /* Size of the temporary register block */
  ಪೂರ್ಣಾಂಕ iRangeReg;       /* First register in temporary register block */
  ಪೂರ್ಣಾಂಕ nErr;            /* Number of errors seen */
  ಪೂರ್ಣಾಂಕ nTab;            /* Number of previously allocated VDBE cursors */
  ಪೂರ್ಣಾಂಕ nMem;            /* Number of memory cells used so far */
  ಪೂರ್ಣಾಂಕ szOpAlloc;       /* Bytes of memory space allocated for Vdbe.aOp[] */
  ಪೂರ್ಣಾಂಕ iSelfTab;        /* Table associated with an index on expr, or negative
                       ** of the base register during check-constraint eval */
  ಪೂರ್ಣಾಂಕ nNestSel;        /* Number of nested SELECT statements and/or VIEWs */
  ಪೂರ್ಣಾಂಕ nLabel;          /* The *negative* of the number of labels used */
  ಪೂರ್ಣಾಂಕ nLabelAlloc;     /* Number of slots in aLabel */
  ಪೂರ್ಣಾಂಕ *aLabel;         /* Space to hold the labels */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_Const_ಅಭಿವ್ಯಕ್ತಿ;/* Constant expressions */
  ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ *pIdxEpr;/* List of expressions used by active indexes */
  ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ *p_Idx_Part_ಅಭಿವ್ಯಕ್ತಿ; /* Exprs constrained by index WHERE clauses */
  y_Db_ಮುಖವಾಡ write_ಮುಖವಾಡ;   /* Start a write transaction on these databases */
  y_Db_ಮುಖವಾಡ cookie_ಮುಖವಾಡ;  /* Bitmask of schema verified databases */
  ಪೂರ್ಣಾಂಕ nMaxArg;         /* Max args to xUpdate and xFilter vtab methods */
  ಪೂರ್ಣಾಂಕ nSelect;         /* Number of SELECT stmts. Counter for Select.selId */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ಪ್ರಗತಿ_CALLBACK
  u32 n_ಪ್ರಗತಿ_ಹೆಜ್ಜೆಗಳು;  /* xProgress steps taken during sqlite3_prepare() */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
  ಪೂರ್ಣಾಂಕ n_ಕೋಷ್ಟಕ_ಬೀಗ;        /* Number of locks in aTableLock */
#ಆದರೆ_ಅಂತ್ಯ
  Parse *pToplevel;    /* Parse structure for main program (or NULL) */
  ಕೋಷ್ಟಕ *pTriggerTab;  /* Table triggers are being coded for */
  TriggerPrg *pTriggerPrg;  /* Linked list of coded triggers */
  ParseCleanup *pCleanup;   /* List of cleanup operations to run after parse */
  Trigger *pTrigCoded;      /* Trigger whose sub-program is being coded */

  /**************************************************************************
  ** Fields above must be initialized to zero.  The fields that follow,
  ** down to the beginning of the recursive section, do not need to be
  ** initialized as they will be set before being used.  The boundary is
  ** determined by offsetof(Parse,aTempReg).
  **************************************************************************/

  ಪೂರ್ಣಾಂಕ a_ತಾತ್ಕಾಲಿಕ_Reg[8];        /* Holding area for temporary registers */
  Parse *pOuterParse;     /* Outer Parse object when nested */
  Token sNameToken;       /* Token with unqualified schema object name */
  u32 oldmask;            /* Mask of old.* columns referenced */
  u32 newmask;            /* Mask of new.* columns referenced */
  ಒಕ್ಕೂಟ {
    ರಚನೆ {  /* These fields available when isCreate is true */
      ಪೂರ್ಣಾಂಕ addrCrTab;        /* Address of OP_CreateBtree on CREATE TABLE */
      ಪೂರ್ಣಾಂಕ regRowid;         /* Register holding rowid of CREATE TABLE entry */
      ಪೂರ್ಣಾಂಕ reg_ಬೇರು;          /* Register holding root page for new objects */
      Token constraintName; /* Name of the constraint currently being parsed */
    } cr;
    ರಚನೆ {  /* These fields available to all other statements */
      Returning *pReturning; /* The RETURNING clause */
    } d;
  } u1;
  AutoincInfo *pAinc;     /* Information about AUTOINCREMENT counters. Only
                          ** valid if Parse.usesAinc is true */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
  ಕೋಷ್ಟಕ_ಬೀಗ *a_ಕೋಷ್ಟಕ_ಬೀಗ;  /* Required table locks for shared-cache mode. Only
                          ** valid if Parse.nTableLock>0 */
#ಆದರೆ_ಅಂತ್ಯ


  /************************************************************************
  ** Above is constant between recursions.  Below is reset before and after
  ** each recursion.  The boundary between these two regions is determined
  ** using offsetof(Parse,sLastToken) so the sLastToken field must be the
  ** first field in the recursive region.
  ************************************************************************/

  Token s_ಕೊನೆಯ_Token;       /* The last token parsed */
  ynVar nVar;               /* Number of '?' variables seen in the SQL so far */
  u64 aVnbmc[SQLITE_VNBMC]; /* Varible Number Bitmap Cache */
  u8 i_Pk_ವಿಂಗಡಿಸು_ಕ್ರಮ;          /* ASC or DESC for INTEGER PRIMARY KEY */
  u8 explain;               /* True if the EXPLAIN flag is found on the query */
  u8 e_Parse_ಬಹುಲಕ;            /* PARSE_MODE_XXX constant */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಪೂರ್ಣಾಂಕ n_Vtab_ಬೀಗ;            /* Number of virtual tables to lock */
#ಆದರೆ_ಅಂತ್ಯ
  ಪೂರ್ಣಾಂಕ n_ಎತ್ತರ;              /* Expression tree height of current sub-select */
  ಪೂರ್ಣಾಂಕ addrExplain;          /* Address of current OP_Explain opcode */
  VList *pVList;            /* Mapping between variable names and numbers */
  Vdbe *pReprepare;         /* VM being reprepared (sqlite3Reprepare()) */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಬಾಲ;        /* All SQL text past the last semicolon parsed */
  ಕೋಷ್ಟಕ *p_New_ಕೋಷ್ಟಕ;         /* A table being constructed by CREATE TABLE */
  ಸೂಚ್ಯಂಕ *p_New_ಸೂಚ್ಯಂಕ;         /* An index being constructed by CREATE INDEX.
                            ** Also used to hold redundant UNIQUE constraints
                            ** during a RENAME COLUMN */
  Trigger *pNewTrigger;     /* Trigger under construct by a CREATE TRIGGER */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zAuthContext; /* The 6th parameter to db->xAuth callbacks */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  Token sArg;               /* Complete text of a module argument */
  ಕೋಷ್ಟಕ **ap_Vtab_ಬೀಗ;       /* Pointer to virtual tables needing locking */
#ಆದರೆ_ಅಂತ್ಯ
  With *pWith;              /* Current WITH clause, or NULL */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ALTERTABLE
  RenameToken *pRename;     /* Tokens subject to renaming by ALTER TABLE */
#ಆದರೆ_ಅಂತ್ಯ
};

/* Allowed values for Parse.eParseMode
*/
#ವ್ಯಾಖ್ಯಾನಿಸು PARSE_ಬಹುಲಕ_ಸಾಮಾನ್ಯ        0
#ವ್ಯಾಖ್ಯಾನಿಸು PARSE_ಬಹುಲಕ_DECLARE_VTAB  1
#ವ್ಯಾಖ್ಯಾನಿಸು PARSE_ಬಹುಲಕ_RENAME        2
#ವ್ಯಾಖ್ಯಾನಿಸು PARSE_ಬಹುಲಕ_UNMAP         3

/*
** Sizes and pointers of various parts of the Parse object.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು PARSE_HDR(X)  (((ಅಕ್ಷರ*)(X))+offsetof(Parse,z_Err_ಸಂದೇಶ))
#ವ್ಯಾಖ್ಯಾನಿಸು PARSE_HDR_SZ (offsetof(Parse,a_ತಾತ್ಕಾಲಿಕ_Reg)-offsetof(Parse,z_Err_ಸಂದೇಶ)) /* Recursive part w/o aColCache*/
#ವ್ಯಾಖ್ಯಾನಿಸು PARSE_RECURSE_SZ offsetof(Parse,s_ಕೊನೆಯ_Token)    /* Recursive part */
#ವ್ಯಾಖ್ಯಾನಿಸು PARSE_ಬಾಲ_SZ (ಗಾತ್ರ(Parse)-PARSE_RECURSE_SZ) /* Non-recursive part */
#ವ್ಯಾಖ್ಯಾನಿಸು PARSE_ಬಾಲ(X) (((ಅಕ್ಷರ*)(X))+PARSE_RECURSE_SZ)  /* Pointer to tail */

/*
** Return true if currently inside an sqlite3_declare_vtab() call.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_VIRTUALTABLE
  #ವ್ಯಾಖ್ಯಾನಿಸು ಒಳ_DECLARE_VTAB 0
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು ಒಳ_DECLARE_VTAB (pParse->e_Parse_ಬಹುಲಕ==PARSE_ಬಹುಲಕ_DECLARE_VTAB)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(SQLITE_OMIT_ALTERTABLE)
  #ವ್ಯಾಖ್ಯಾನಿಸು ಒಳ_RENAME_OBJECT 0
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು ಒಳ_RENAME_OBJECT (pParse->e_Parse_ಬಹುಲಕ>=PARSE_ಬಹುಲಕ_RENAME)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_OMIT_ALTERTABLE)
  #ವ್ಯಾಖ್ಯಾನಿಸು ಒಳ_SPECIAL_PARSE 0
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು ಒಳ_SPECIAL_PARSE (pParse->e_Parse_ಬಹುಲಕ!=PARSE_ಬಹುಲಕ_ಸಾಮಾನ್ಯ)
#ಆದರೆ_ಅಂತ್ಯ

/*
** An instance of the following structure can be declared on a stack and used
** to save the Parse.zAuthContext value so that it can be restored later.
*/
ರಚನೆ AuthContext {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zAuthContext;   /* Put saved Parse.zAuthContext here */
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
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_NCHANGE       0x01    /* OP_Insert: Set to update db->nChange */
                                     /* Also used in P2 (not P5) of OP_Delete */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_NOCHNG        0x01    /* OP_VColumn nochange for UPDATE */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_EPHEM         0x01    /* OP_Column: Ephemeral output is ok */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_LASTROWID     0x20    /* Set to update db->lastRowid */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_ISUPDATE      0x04    /* This OP_Insert is an sql UPDATE */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_APPEND        0x08    /* This is likely to be an append */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_USESEEKRESULT 0x10    /* Try to avoid a seek in BtreeInsert() */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_ISNOOP        0x40    /* OP_Delete does pre-update-hook only */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_LENGTHARG     0x40    /* OP_Column only used for length() */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_TYPEOFARG     0x80    /* OP_Column only used for typeof() */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_BYTELENARG    0xc0    /* OP_Column only for octet_length() */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_BULKCSR       0x01    /* OP_Open** used to open bulk cursor */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_SEEKEQ        0x02    /* OP_Open** cursor uses EQ seek only */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_FORDELETE     0x08    /* OP_Open should use BTREE_FORDELETE */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_P2ISREG       0x10    /* P2 to OP_Open** is a register number */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_ಕ್ರಮಬದಲಾವಣೆ       0x01    /* OP_Compare: use the permutation */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_SAVEPOSITION  0x02    /* OP_Delete/Insert: save cursor pos */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_AUXDELETE     0x04    /* OP_Delete: index in a DELETE op */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_NOCHNG_MAGIC  0x6d    /* OP_MakeRecord: serialtype 10 is ok */
#ವ್ಯಾಖ್ಯಾನಿಸು OPFLAG_PREFORMAT     0x80    /* OP_Insert uses preformatted cell */

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
ರಚನೆ Trigger {
  ಅಕ್ಷರ *zName;            /* The name of the trigger                        */
  ಅಕ್ಷರ *ಕೋಷ್ಟಕ_2;            /* The table or view to which the trigger applies */
  u8 op;                  /* One of TK_DELETE, TK_UPDATE, TK_INSERT         */
  u8 tr_tm;               /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  u8 bReturning;          /* This trigger implements a RETURNING clause */
  ಅಭಿವ್ಯಕ್ತಿ *pWhen;            /* The WHEN clause of the expression (may be NULL) */
  IdList *pColumns;       /* If this is an UPDATE OF <column-list> trigger,
                             the <column-list> is stored here */
  Schema *pSchema;        /* Schema containing the trigger */
  Schema *pTabSchema;     /* Schema containing the table */
  Trigger_ಹೆಜ್ಜೆ *ಹೆಜ್ಜೆ_list; /* Link list of trigger program steps             */
  Trigger *p_ಮುಂದಿನ;         /* Next trigger associated with the table */
};

/*
** A trigger is either a BEFORE or an AFTER trigger.  The following constants
** determine which.
**
** If there are multiple triggers, you might of some BEFORE and some AFTER.
** In that cases, the constants below can be ORed together.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು TRIGGER_BEFORE  1
#ವ್ಯಾಖ್ಯಾನಿಸು TRIGGER_AFTER   2

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
ರಚನೆ Trigger_ಹೆಜ್ಜೆ {
  u8 op;               /* One of TK_DELETE, TK_UPDATE, TK_INSERT, TK_SELECT,
                       ** or TK_RETURNING */
  u8 orconf;           /* OE_Rollback etc. */
  Trigger *pTrig;      /* The trigger that this step is a part of */
  Select *pSelect;     /* SELECT statement or RHS of INSERT INTO SELECT ... */
  SrcList *pSrc;       /* Table to insert/update/delete */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere;        /* The WHERE clause for DELETE or UPDATE steps */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಅಭಿವ್ಯಕ್ತಿ_List; /* SET clause for UPDATE, or RETURNING clause */
  IdList *pIdList;     /* Column names for INSERT */
  Upsert *pUpsert;     /* Upsert clauses on an INSERT */
  ಅಕ್ಷರ *z_ಹರವು;         /* Original SQL text of this command */
  Trigger_ಹೆಜ್ಜೆ *p_ಮುಂದಿನ;  /* Next in the link-list */
  Trigger_ಹೆಜ್ಜೆ *p_ಕೊನೆಯ;  /* Last element in link-list. Valid for 1st elem only */
};

/*
** Information about a RETURNING clause
*/
ರಚನೆ Returning {
  Parse *pParse;        /* The parse that includes the RETURNING clause */
  ಅಭಿವ್ಯಕ್ತಿ_List *pReturnEL;  /* List of expressions to return */
  Trigger retTrig;      /* The transient trigger that implements RETURNING */
  Trigger_ಹೆಜ್ಜೆ retTStep; /* The trigger step */
  ಪೂರ್ಣಾಂಕ iRetCur;          /* Transient table holding RETURNING results */
  ಪೂರ್ಣಾಂಕ n_Ret_ಲಂಬಸಾಲು;          /* Number of in pReturnEL after expansion */
  ಪೂರ್ಣಾಂಕ iRetReg;          /* Register array for holding a row of RETURNING */
  ಅಕ್ಷರ zName[40];       /* Name of trigger: "sqlite_returning_%p" */
};

/*
** An object used to accumulate the text of a string where we
** do not necessarily know how big the string will be in the end.
*/
ರಚನೆ sqlite3_str {
  sqlite3 *db;         /* Optional database for lookaside.  Can be NULL */
  ಅಕ್ಷರ *z_ಪಠ್ಯ;         /* The string collected so far */
  u32  nAlloc;         /* Amount of space allocated in zText */
  u32  mxAlloc;        /* Maximum allowed allocation.  0 for no malloc usage */
  u32  nChar;          /* Length of the string so far */
  u8   accError;       /* SQLITE_NOMEM or SQLITE_TOOBIG */
  u8   printfFlags;    /* SQLITE_PRINTF flags below */
};
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_PRINTF_ಆಂತರಿಕ 0x01  /* Internal-use-only converters allowed */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_PRINTF_SQLFUNC  0x02  /* SQL function arguments to VXPrintf */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_PRINTF_MALLOCED 0x04  /* True if zText is allocated space */

#ವ್ಯಾಖ್ಯಾನಿಸು isMalloced(X)  (((X)->printfFlags & SQLITE_PRINTF_MALLOCED)!=0)

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
ರಚನೆ RCStr {
  u64 nRCRef;            /* Number of references */
  /* Total structure size should be a multiple of 8 bytes for alignment */
};

/*
** A pointer to this structure is used to communicate information
** from sqlite3Init and OP_ParseSchema into the sqlite3InitCallback.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
  sqlite3 *db;        /* The database being initialized */
  ಅಕ್ಷರ **pz_Err_ಸಂದೇಶ;    /* Error message stored here */
  ಪೂರ್ಣಾಂಕ iDb;            /* 0 for main database.  1 for TEMP, 2.. for ATTACHed */
  ಪೂರ್ಣಾಂಕ rc;             /* Result code stored here */
  u32 mInitFlags;     /* Flags controlling error messages */
  u32 n_Init_ಅಡ್ಡಸಾಲು;       /* Number of rows processed */
  Pgno mx_ಪುಟ;        /* Maximum page number.  0 for no limit. */
} Init_ದತ್ತಾಂಶ;

/*
** Allowed values for mInitFlags
*/
#ವ್ಯಾಖ್ಯಾನಿಸು INITFLAG_Alter_ಮುಖವಾಡ     0x0007  /* Types of ALTER */
#ವ್ಯಾಖ್ಯಾನಿಸು INITFLAG_AlterRename   0x0001  /* Reparse after a RENAME */
#ವ್ಯಾಖ್ಯಾನಿಸು INITFLAG_AlterDrop     0x0002  /* Reparse after a DROP COLUMN */
#ವ್ಯಾಖ್ಯಾನಿಸು INITFLAG_Alter_ಕೂಡಿಸು      0x0003  /* Reparse after an ADD COLUMN */
#ವ್ಯಾಖ್ಯಾನಿಸು INITFLAG_AlterDropCons 0x0004  /* Reparse after a DROP CONSTRAINT */

/* Tuning parameters are set using SQLITE_TESTCTRL_TUNE and are controlled
** on debug-builds of the CLI using ".testctrl tune ID VALUE".  Tuning
** parameters are for temporary use during development, to help find
** optimal values for parameters in the query planner.  The should not
** be used on trunk check-ins.  They are a temporary mechanism available
** for transient development builds only.
**
** Tuning parameters are numbered starting with 1.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_NTUNE  6             /* Should be zero for all trunk check-ins */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
# define Tuning(X)  (sqlite3Config.aTune[(X)-1])
#ಇಲ್ಲದಿದ್ದರೆ
# define Tuning(X)  0
#ಆದರೆ_ಅಂತ್ಯ

/*
** Structure containing global configuration data for the SQLite library.
**
** This structure also contains some state information.
*/
ರಚನೆ Sqlite3Config {
  ಪೂರ್ಣಾಂಕ bMemstat;                     /* True to enable memory status */
  u8 bCoreMutex;                    /* True to enable core mutexing */
  u8 b_ತುಂಬಿದ_Mutex;                    /* True to enable full mutexing */
  u8 bOpenUri;                      /* True to interpret filenames as URIs */
  u8 bUseCis;                       /* Use covering indices for full-scans */
  u8 bSmallMalloc;                  /* Avoid large memory allocations if true */
  u8 bExtraSchemaChecks;            /* Verify type,name,tbl_name in schema */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  u8 bJsonSelfcheck;                /* Double-check JSON parsing */
#ಆದರೆ_ಅಂತ್ಯ
  ಪೂರ್ಣಾಂಕ mxStrlen;                     /* Maximum string length */
  ಪೂರ್ಣಾಂಕ neverCorrupt;                 /* Database is always well-formed */
  ಪೂರ್ಣಾಂಕ szLookaside;                  /* Default lookaside buffer size */
  ಪೂರ್ಣಾಂಕ nLookaside;                   /* Default lookaside buffer count */
  ಪೂರ್ಣಾಂಕ nStmtSpill;                   /* Stmt-journal spill-to-disk threshold */
  sqlite3_mem_methods m;            /* Low-level memory allocation interface */
  sqlite3_mutex_methods mutex;      /* Low-level mutex interface */
  sqlite3_pcache_methods2 pcache2;  /* Low-level page-cache interface */
  ಶೂನ್ಯ *p_ರಾಶಿ;                      /* Heap storage space */
  ಪೂರ್ಣಾಂಕ n_ರಾಶಿ;                        /* Size of pHeap[] */
  ಪೂರ್ಣಾಂಕ mnReq, mxReq;                 /* Min and max heap requests sizes */
  sqlite3_int64 szMmap;             /* mmap() space per open file */
  sqlite3_int64 mxMmap;             /* Maximum value for szMmap */
  ಶೂನ್ಯ *p_ಪುಟ;                      /* Page cache memory */
  ಪೂರ್ಣಾಂಕ sz_ಪುಟ;                       /* Size of each page in pPage[] */
  ಪೂರ್ಣಾಂಕ n_ಪುಟ;                        /* Number of pages in pPage[] */
  ಪೂರ್ಣಾಂಕ mx_Parser_ರಾಶಿ;                /* maximum depth of the parser stack */
  ಪೂರ್ಣಾಂಕ sharedCacheEnabled;           /* true if shared-cache mode enabled */
  u32 szPma;                        /* Maximum Sorter PMA size */
  /* The above might be initialized to non-zero.  The following need to always
  ** initially be zero, however. */
  ಪೂರ್ಣಾಂಕ isInit;                       /* True after initialization has finished */
  ಪೂರ್ಣಾಂಕ ಒಳ_ಪ್ರಗತಿ;                   /* True while initialization in progress */
  ಪೂರ್ಣಾಂಕ isMutexInit;                  /* True after mutexes are initialized */
  ಪೂರ್ಣಾಂಕ isMallocInit;                 /* True after malloc is initialized */
  ಪೂರ್ಣಾಂಕ isPCacheInit;                 /* True after malloc is initialized */
  ಪೂರ್ಣಾಂಕ nRefInitMutex;                /* Number of users of pInitMutex */
  sqlite3_mutex *pInitMutex;        /* Mutex used by sqlite3_initialize() */
  ಶೂನ್ಯ (*xLog)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*); /* Function for logging */
  ಶೂನ್ಯ *pLogArg;                       /* First argument to xLog() */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SQLLOG
  ಶೂನ್ಯ(*xSqllog)(ಶೂನ್ಯ*,sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ *pSqllogArg;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_VDBE_COVERAGE
  /* The following callback (if not NULL) is invoked on every VDBE branch
  ** operation.  Set the callback using SQLITE_TESTCTRL_VDBE_COVERAGE.
  */
  ಶೂನ್ಯ (*xVdbeBranch)(ಶೂನ್ಯ*,ಚಿಹ್ನೆರಹಿತ i_Src_ಸಾಲು,u8 eThis,u8 eMx);  /* Callback */
  ಶೂನ್ಯ *pVdbeBranchArg;                                     /* 1st argument */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DESERIALIZE
  sqlite3_int64 mx_Memdb_ಗಾತ್ರ;        /* Default max memdb size */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_UNTESTABLE
  ಪೂರ್ಣಾಂಕ (*xTestCallback)(ಪೂರ್ಣಾಂಕ);        /* Invoked by sqlite3FaultSim() */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ALLOW_ROWID_ಒಳ_VIEW
  u32 mNoVisibleRowid;              /* TF_NoVisibleRowid if the ROWID_IN_VIEW
                                    ** feature is disabled.  0 if rowids can
                                    ** occur in views. */
#ಆದರೆ_ಅಂತ್ಯ
  ಪೂರ್ಣಾಂಕ bLocaltimeFault;              /* True to fail localtime() calls */
  ಪೂರ್ಣಾಂಕ (*xAltLocaltime)(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಶೂನ್ಯ*); /* Alternative localtime() routine */
  ಪೂರ್ಣಾಂಕ i_Once_Reset_ಮಿತಿ;          /* When to reset OP_Once counters */
  u32 szSorterRef;                  /* Min size in bytes to use sorter-refs */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i_Prng_ಬೀಜ;           /* Alternative fixed seed for the PRNG */
  /* vvvv--- must be last ---vvv */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  sqlite3_int64 aTune[SQLITE_NTUNE]; /* Tuning parameters */
#ಆದರೆ_ಅಂತ್ಯ
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
#ವ್ಯಾಖ್ಯಾನಿಸು CORRUPT_DB  (sqlite3Config.neverCorrupt==0)

/*
** Context pointer passed down through the tree-walk.
*/
ರಚನೆ Walker {
  Parse *pParse;                            /* Parser context.  */
  ಪೂರ್ಣಾಂಕ (*x_ಅಭಿವ್ಯಕ್ತಿ_Callback)(Walker*, ಅಭಿವ್ಯಕ್ತಿ*);     /* Callback for expressions */
  ಪೂರ್ಣಾಂಕ (*xSelectCallback)(Walker*,Select*);  /* Callback for SELECTs */
  ಶೂನ್ಯ (*xSelectCallback2)(Walker*,Select*);/* Second callback for SELECTs */
  ಪೂರ್ಣಾಂಕ walker_ಆಳ;                          /* Number of subqueries */
  u16 e_ಸಂಕೇತ;                                /* A small processing code */
  u16 mWFlags;                              /* Use-dependent flags */
  ಒಕ್ಕೂಟ {                                   /* Extra data for callback */
    NameContext *pNC;                         /* Naming context */
    ಪೂರ್ಣಾಂಕ n;                                    /* A counter */
    ಪೂರ್ಣಾಂಕ iCur;                                 /* A cursor number */
    ಪೂರ್ಣಾಂಕ sz;                                   /* String literal length */
    SrcList *pSrcList;                        /* FROM clause */
    ರಚನೆ CCurHint *pCCurHint;               /* Used by codeCursorHint() */
    ರಚನೆ RefSrcList *pRefSrcList;           /* sqlite3ReferencesSrcList() */
    ಪೂರ್ಣಾಂಕ *ai_ಲಂಬಸಾಲು;                               /* array of column indexes */
    ರಚನೆ IdxCover *pIdxCover;               /* Check for index coverage */
    ಅಭಿವ್ಯಕ್ತಿ_List *p_ಗುಂಪು_By;                       /* GROUP BY clause */
    Select *pSelect;                          /* HAVING to WHERE clause ctx */
    ರಚನೆ ಕಿಟಕಿ_Rewrite *pRewrite;           /* Window rewrite context */
    ರಚನೆ WhereConst *pConst;                /* WHERE clause constants */
    ರಚನೆ RenameCtx *pRename;                /* RENAME COLUMN context */
    ರಚನೆ ಕೋಷ್ಟಕ *pTab;                       /* Table of generated column */
    ರಚನೆ Covering_ಸೂಚ್ಯಂಕ_Check *pCovIdxCk;     /* Check for covering index */
    Src_ಅಂಶ *p_Src_ಅಂಶ;                        /* A single FROM clause item */
    DbFixer *pFix;                            /* See sqlite3FixSelect() */
    Mem *aMem;                                /* See sqlite3BtreeCursorHint() */
    ರಚನೆ CheckOnCtx *pCheckOnCtx;           /* See selectCheckOnClauses() */
  } u;
};

/*
** The following structure contains information used by the sqliteFix...
** routines as they walk the parse tree to make database references
** explicit.
*/
ರಚನೆ DbFixer {
  Parse *pParse;      /* The parsing context.  Error messages written here */
  Walker w;           /* Walker object */
  Schema *pSchema;    /* Fix items to this schema */
  u8 b_ತಾತ್ಕಾಲಿಕ;           /* True for TEMP schema entries */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb;    /* Make sure all objects are contained in this database */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType;  /* Type of the container - used for error messages */
  ಸ್ಥಿರಾಂಕ Token *pName; /* Name of the container - used for error messages */
};

/* Forward declarations */
ಪೂರ್ಣಾಂಕ sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(Walker*, ಅಭಿವ್ಯಕ್ತಿ*);
ಪೂರ್ಣಾಂಕ sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_NN(Walker*, ಅಭಿವ್ಯಕ್ತಿ*);
ಪೂರ್ಣಾಂಕ sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(Walker*, ಅಭಿವ್ಯಕ್ತಿ_List*);
ಪೂರ್ಣಾಂಕ sqlite3WalkSelect(Walker*, Select*);
ಪೂರ್ಣಾಂಕ sqlite3_Walk_Select_ಅಭಿವ್ಯಕ್ತಿ(Walker*, Select*);
ಪೂರ್ಣಾಂಕ sqlite3WalkSelectFrom(Walker*, Select*);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Walk_Noop(Walker*, ಅಭಿವ್ಯಕ್ತಿ*);
ಪೂರ್ಣಾಂಕ sqlite3SelectWalkNoop(Walker*, Select*);
ಪೂರ್ಣಾಂಕ sqlite3SelectWalkFail(Walker*, Select*);
ಪೂರ್ಣಾಂಕ sqlite3_Walker_ಆಳ_Increase(Walker*,Select*);
ಶೂನ್ಯ sqlite3_Walker_ಆಳ_Decrease(Walker*,Select*);
ಶೂನ್ಯ sqlite3WalkWinDefnDummyCallback(Walker*,Select*);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
ಶೂನ್ಯ sqlite3SelectWalkAssert2(Walker*, Select*);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CTE
ಶೂನ್ಯ sqlite3_Select_ತೆಗೆ_With(Walker*, Select*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_Select_ತೆಗೆ_With 0
#ಆದರೆ_ಅಂತ್ಯ

/*
** Return code from the parse-tree walking primitives and their
** callbacks.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು WRC_Continue    0   /* Continue down into children */
#ವ್ಯಾಖ್ಯಾನಿಸು WRC_Prune       1   /* Omit children but continue walking siblings */
#ವ್ಯಾಖ್ಯಾನಿಸು WRC_Abort       2   /* Abandon the tree walk */

/*
** A single common table expression
*/
ರಚನೆ Cte {
  ಅಕ್ಷರ *zName;            /* Name of this CTE */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;        /* List of explicit column names, or NULL */
  Select *pSelect;        /* The definition of this CTE */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zCteErr;    /* Error message for circular references */
  CteUse *pUse;           /* Usage information for this CTE */
  u8 eM10d;               /* The MATERIALIZED flag */
};

/*
** Allowed values for the materialized flag (eM10d):
*/
#ವ್ಯಾಖ್ಯಾನಿಸು M10d_Yes       0  /* AS MATERIALIZED */
#ವ್ಯಾಖ್ಯಾನಿಸು M10d_Any       1  /* Not specified.  Query planner's choice */
#ವ್ಯಾಖ್ಯಾನಿಸು M10d_No        2  /* AS NOT MATERIALIZED */

/*
** An instance of the With object represents a WITH clause containing
** one or more CTEs (common table expressions).
*/
ರಚನೆ With {
  ಪೂರ್ಣಾಂಕ nCte;               /* Number of CTEs in the WITH clause */
  ಪೂರ್ಣಾಂಕ bView;              /* Belongs to the outermost Select of a view */
  With *pOuter;           /* Containing WITH clause, or NULL */
  Cte a[FLEXARRAY];       /* For each CTE in the WITH clause.... */
};

/* The size (in bytes) of a With object that can hold as many
** as N different CTEs. */
#ವ್ಯಾಖ್ಯಾನಿಸು SZ_WITH(N)  (offsetof(With,a) + (N)*ಗಾತ್ರ(Cte))

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
ರಚನೆ CteUse {
  ಪೂರ್ಣಾಂಕ nUse;              /* Number of users of this CTE */
  ಪೂರ್ಣಾಂಕ addrM9e;           /* Start of subroutine to compute materialization */
  ಪೂರ್ಣಾಂಕ regRtn;            /* Return address register for addrM9e subroutine */
  ಪೂರ್ಣಾಂಕ iCur;              /* Ephemeral table holding the materialization */
  LogEst n_ಅಡ್ಡಸಾಲು_Est;        /* Estimated number of rows in the table */
  u8 eM10d;              /* The MATERIALIZED flag */
};


/* Client data associated with sqlite3_set_clientdata() and
** sqlite3_get_clientdata().
*/
ರಚನೆ Db_Client_ದತ್ತಾಂಶ {
  Db_Client_ದತ್ತಾಂಶ *p_ಮುಂದಿನ;        /* Next in a linked list */
  ಶೂನ್ಯ *p_ದತ್ತಾಂಶ;                /* The data */
  ಶೂನ್ಯ (*xDestructor)(ಶೂನ್ಯ*); /* Destructor.  Might be NULL */
  ಅಕ್ಷರ zName[FLEXARRAY];      /* Name of this client data. MUST BE LAST */
};

/* The size (in bytes) of a DbClientData object that can has a name
** that is N bytes long, including the zero-terminator. */
#ವ್ಯಾಖ್ಯಾನಿಸು SZ_DBCLIENTDATA(N) (offsetof(Db_Client_ದತ್ತಾಂಶ,zName)+(N))

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** An instance of the TreeView object is used for printing the content of
** data structures on sqlite3DebugPrintf() using a tree-like view.
*/
ರಚನೆ ವೃಕ್ಷ_View {
  ಪೂರ್ಣಾಂಕ iLevel;             /* Which level of the tree we are on */
  u8  b_ಸಾಲು[100];         /* Draw vertical in column i if bLine[i] is true */
};
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */

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
ರಚನೆ ಕಿಟಕಿ {
  ಅಕ್ಷರ *zName;            /* Name of window (may be NULL) */
  ಅಕ್ಷರ *z_ತಳ;            /* Name of base window for chaining (may be NULL) */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ವಿಭಜನೆ;   /* PARTITION BY clause */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By;     /* ORDER BY clause */
  u8 eFrmType;            /* TK_RANGE, TK_GROUPS, TK_ROWS, or 0 */
  u8 e_ಆರಂಭ;              /* UNBOUNDED, CURRENT, PRECEDING or FOLLOWING */
  u8 e_ಕೊನೆ;                /* UNBOUNDED, CURRENT, PRECEDING or FOLLOWING */
  u8 b_Implicit_ಚೌಕಟ್ಟು;      /* True if frame was implicitly specified */
  u8 eExclude;            /* TK_NO, TK_CURRENT, TK_TIES, TK_GROUP, or 0 */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಆರಂಭ;           /* Expression for "<expr> PRECEDING" */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಕೊನೆ;             /* Expression for "<expr> FOLLOWING" */
  ಕಿಟಕಿ **ppThis;        /* Pointer to this object in Select.pWin list */
  ಕಿಟಕಿ *p_ಮುಂದಿನ_Win;       /* Next window function belonging to this SELECT */
  ಅಭಿವ್ಯಕ್ತಿ *pFilter;          /* The FILTER expression */
  FuncDef *pWFunc;        /* The function */
  ಪೂರ್ಣಾಂಕ iEphCsr;            /* Partition buffer or Peer buffer */
  ಪೂರ್ಣಾಂಕ regAccum;           /*ಸಂಚಯಕ*/
  ಪೂರ್ಣಾಂಕ reg_ಫಲಿತಾಂಶ;          /* Interim result */
  ಪೂರ್ಣಾಂಕ csrApp;             /* Function cursor (used by min/max) */
  ಪೂರ್ಣಾಂಕ regApp;             /* Function register (also used by min/max) */
  ಪೂರ್ಣಾಂಕ regPart;            /* Array of registers for PARTITION BY values */
  ಅಭಿವ್ಯಕ್ತಿ *pOwner;           /* Expression object this window is attached to */
  ಪೂರ್ಣಾಂಕ n_ಬಫರ್_ಲಂಬಸಾಲು;         /* Number of columns in buffer table */
  ಪೂರ್ಣಾಂಕ i_Arg_ಲಂಬಸಾಲು;            /* Offset of first argument for this function */
  ಪೂರ್ಣಾಂಕ regOne;             /* Register containing constant value 1 */
  ಪೂರ್ಣಾಂಕ reg_ಆರಂಭ_Rowid;
  ಪೂರ್ಣಾಂಕ reg_ಕೊನೆ_Rowid;
  u8 b_ಅಭಿವ್ಯಕ್ತಿ_ವಾದಗಳು;           /* Defer evaluation of window function arguments
                          ** due to the SQLITE_SUBTYPE flag */
};

Select *sqlite3_Multi_ಮೌಲ್ಯಗಳು(Parse *pParse, Select *p_ಎಡ, ಅಭಿವ್ಯಕ್ತಿ_List *p_ಅಡ್ಡಸಾಲು);
ಶೂನ್ಯ sqlite3_Multi_ಮೌಲ್ಯಗಳು_ಕೊನೆ(Parse *pParse, Select *pVal);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
ಶೂನ್ಯ sqlite3_ಕಿಟಕಿ_Delete(sqlite3*, ಕಿಟಕಿ*);
ಶೂನ್ಯ sqlite3_ಕಿಟಕಿ_Unlink_From_Select(ಕಿಟಕಿ*);
ಶೂನ್ಯ sqlite3_ಕಿಟಕಿ_List_Delete(sqlite3 *db, ಕಿಟಕಿ *p);
ಕಿಟಕಿ *sqlite3_ಕಿಟಕಿ_Alloc(Parse*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ , ಅಭಿವ್ಯಕ್ತಿ*, u8);
ಶೂನ್ಯ sqlite3_ಕಿಟಕಿ_Attach(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, ಕಿಟಕಿ*);
ಶೂನ್ಯ sqlite3_ಕಿಟಕಿ_Link(Select *pSel, ಕಿಟಕಿ *pWin);
ಪೂರ್ಣಾಂಕ sqlite3_ಕಿಟಕಿ_Compare(ಸ್ಥಿರಾಂಕ Parse*, ಸ್ಥಿರಾಂಕ ಕಿಟಕಿ*, ಸ್ಥಿರಾಂಕ ಕಿಟಕಿ*, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಕಿಟಕಿ_ಸಂಕೇತ_Init(Parse*, Select*);
ಶೂನ್ಯ sqlite3_ಕಿಟಕಿ_ಸಂಕೇತ_ಹೆಜ್ಜೆ(Parse*, Select*, WhereInfo*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಕಿಟಕಿ_Rewrite(Parse*, Select*);
ಶೂನ್ಯ sqlite3_ಕಿಟಕಿ_Update(Parse*, ಕಿಟಕಿ*, ಕಿಟಕಿ*, FuncDef*);
ಕಿಟಕಿ *sqlite3_ಕಿಟಕಿ_Dup(sqlite3 *db, ಅಭಿವ್ಯಕ್ತಿ *pOwner, ಕಿಟಕಿ *p);
ಕಿಟಕಿ *sqlite3_ಕಿಟಕಿ_List_Dup(sqlite3 *db, ಕಿಟಕಿ *p);
ಶೂನ್ಯ sqlite3_ಕಿಟಕಿ_Functions(ಶೂನ್ಯ);
ಶೂನ್ಯ sqlite3_ಕಿಟಕಿ_Chain(Parse*, ಕಿಟಕಿ*, ಕಿಟಕಿ*);
ಕಿಟಕಿ *sqlite3_ಕಿಟಕಿ_Assemble(Parse*, ಕಿಟಕಿ*, ಅಭಿವ್ಯಕ್ತಿ_List*, ಅಭಿವ್ಯಕ್ತಿ_List*, Token*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_ಕಿಟಕಿ_Delete(a,b)
# define sqlite3_ಕಿಟಕಿ_Functions()
# define sqlite3_ಕಿಟಕಿ_Attach(a,b,c)
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_SKIP_UTF8(z_ಒಳ) {                        \
  ಆದರೆ( (*(z_ಒಳ++))>=0xc0 ){                              \
    ಆಗ( (*z_ಒಳ & 0xc0)==0x80 ){ z_ಒಳ++; }             \
  }                                                    \
}

/*
** The SQLITE_*_BKPT macros are substitutes for the error codes with
** the same name but without the _BKPT suffix.  These macros invoke
** routines that report the line-number on which the error originated
** using sqlite3_log().  The routines also provide a convenient place
** to set a debugger breakpoint.
*/
ಪೂರ್ಣಾಂಕ sqlite3ReportError(ಪೂರ್ಣಾಂಕ iErr, ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType);
ಪೂರ್ಣಾಂಕ sqlite3CorruptError(ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3MisuseError(ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3CantopenError(ಪೂರ್ಣಾಂಕ);
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_CORRUPT_BKPT sqlite3CorruptError(__LINE__)
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MISUSE_BKPT sqlite3MisuseError(__LINE__)
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_CANTOPEN_BKPT sqlite3CantopenError(__LINE__)
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಪೂರ್ಣಾಂಕ sqlite3NomemError(ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ sqlite3IoerrnomemError(ಪೂರ್ಣಾಂಕ);
# define SQLITE_NOMEM_BKPT sqlite3NomemError(__LINE__)
# define SQLITE_IOERR_NOMEM_BKPT sqlite3IoerrnomemError(__LINE__)
#ಇಲ್ಲದಿದ್ದರೆ
# define SQLITE_NOMEM_BKPT SQLITE_NOMEM
# define SQLITE_IOERR_NOMEM_BKPT SQLITE_IOERR_NOMEM
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_CORRUPT_PGNO)
  ಪೂರ್ಣಾಂಕ sqlite3CorruptPgnoError(ಪೂರ್ಣಾಂಕ,Pgno);
# define SQLITE_CORRUPT_PGNO(P) sqlite3CorruptPgnoError(__LINE__,(P))
#ಇಲ್ಲದಿದ್ದರೆ
# define SQLITE_CORRUPT_PGNO(P) sqlite3CorruptError(__LINE__)
#ಆದರೆ_ಅಂತ್ಯ

/*
** FTS3 and FTS4 both require virtual table support
*/
#ಆದರೆ defined(SQLITE_OMIT_VIRTUALTABLE)
# undef SQLITE_ENABLE_FTS3
# undef SQLITE_ENABLE_FTS4
#ಆದರೆ_ಅಂತ್ಯ

/*
** FTS4 is really an extension for FTS3.  It is enabled using the
** SQLITE_ENABLE_FTS3 macro.  But to avoid confusion we also call
** the SQLITE_ENABLE_FTS4 macro to serve as an alias for SQLITE_ENABLE_FTS3.
*/
#ಆದರೆ defined(SQLITE_ENABLE_FTS4) && !defined(SQLITE_ENABLE_FTS3)
# define SQLITE_ENABLE_FTS3 1
#ಆದರೆ_ಅಂತ್ಯ

/*
** The following macros mimic the standard library functions toupper(),
** isspace(), isalnum(), isdigit() and isxdigit(), respectively. The
** sqlite versions only work for ASCII characters, regardless of locale.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ASCII
# define sqlite3Toupper(x)  ((x)&~(sqlite3CtypeMap[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x)]&0x20))
# define sqlite3Isspace(x)   (sqlite3CtypeMap[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x)]&0x01)
# define sqlite3Isalnum(x)   (sqlite3CtypeMap[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x)]&0x06)
# define sqlite3Isalpha(x)   (sqlite3CtypeMap[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x)]&0x02)
# define sqlite3Isdigit(x)   (sqlite3CtypeMap[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x)]&0x04)
# define sqlite3Isxdigit(x)  (sqlite3CtypeMap[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x)]&0x08)
# define sqlite3Tolower(x)   (sqlite3_ಊರ್ಧ್ವ_ಗಮ್ಯ_ಅಧೋ[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x)])
# define sqlite3Isquote(x)   (sqlite3CtypeMap[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x)]&0x80)
# define sqlite3JsonId1(x)   (sqlite3CtypeMap[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x)]&0x42)
# define sqlite3JsonId2(x)   (sqlite3CtypeMap[(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x)]&0x46)
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3Toupper(x)   ದೊಡ್ಡ_ಅಕ್ಷರ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x))
# define sqlite3Isspace(x)   ಖಾಲಿಯೇ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x))
# define sqlite3Isalnum(x)   ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x))
# define sqlite3Isalpha(x)   ಅಕ್ಷರವೇ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x))
# define sqlite3Isdigit(x)   ಅಂಕಿಯೇ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x))
# define sqlite3Isxdigit(x)  isxdigit((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x))
# define sqlite3Tolower(x)   tolower((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(x))
# define sqlite3Isquote(x)   ((x)=='"'||(x)=='\''||(x)=='['||(x)=='`')
# define sqlite3JsonId1(x)   (sqlite3IsIdChar(x)&&(x)<'0')
# define sqlite3JsonId2(x)   sqlite3IsIdChar(x)
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3IsIdChar(u8);

/*
** Internal function prototypes
*/
ಪೂರ್ಣಾಂಕ sqlite3StrICmp(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಪೂರ್ಣಾಂಕ sqlite3Strlen30(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3Strlen30NN(C) (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(C)&0x3fffffff)
ಅಕ್ಷರ *sqlite3_ಸ್ತಂಭ_Type(ಸ್ತಂಭ*,ಅಕ್ಷರ*);
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3StrNICmp sqlite3_strnicmp

ಪೂರ್ಣಾಂಕ sqlite3MallocInit(ಶೂನ್ಯ);
ಶೂನ್ಯ sqlite3_Malloc_ಕೊನೆ(ಶೂನ್ಯ);
ಶೂನ್ಯ *sqlite3Malloc(u64);
ಶೂನ್ಯ *sqlite3MallocZero(u64);
ಶೂನ್ಯ *sqlite3DbMallocZero(sqlite3*, u64);
ಶೂನ್ಯ *sqlite3DbMallocRaw(sqlite3*, u64);
ಶೂನ್ಯ *sqlite3DbMallocRawNN(sqlite3*, u64);
ಅಕ್ಷರ *sqlite3DbStrDup(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಅಕ್ಷರ *sqlite3DbStrNDup(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, u64);
ಅಕ್ಷರ *sqlite3_Db_ಹರವು_Dup(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಶೂನ್ಯ *sqlite3Realloc(ಶೂನ್ಯ*, u64);
ಶೂನ್ಯ *sqlite3DbReallocOrFree(sqlite3 *, ಶೂನ್ಯ *, u64);
ಶೂನ್ಯ *sqlite3DbRealloc(sqlite3 *, ಶೂನ್ಯ *, u64);
ಶೂನ್ಯ sqlite3DbFree(sqlite3*, ಶೂನ್ಯ*);
ಶೂನ್ಯ sqlite3DbFreeNN(sqlite3*, ಶೂನ್ಯ*);
ಶೂನ್ಯ sqlite3DbNNFreeNN(sqlite3*, ಶೂನ್ಯ*);
ಪೂರ್ಣಾಂಕ sqlite3_Malloc_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*);
ಪೂರ್ಣಾಂಕ sqlite3_Db_Malloc_ಗಾತ್ರ(sqlite3*, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*);
ಶೂನ್ಯ *sqlite3_ಪುಟ_Malloc(ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಪುಟ_Free(ಶೂನ್ಯ*);
ಶೂನ್ಯ sqlite3_Mem_ಗಣ_Default(ಶೂನ್ಯ);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_UNTESTABLE
ಶೂನ್ಯ sqlite3BenignMallocHooks(ಶೂನ್ಯ (*)(ಶೂನ್ಯ), ಶೂನ್ಯ (*)(ಶೂನ್ಯ));
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3_ರಾಶಿ_Nearly_ತುಂಬಿದ(ಶೂನ್ಯ);

/*
** On systems with ample stack space and that support alloca(), make
** use of alloca() to obtain space for large automatic objects.  By default,
** obtain space from malloc().
**
** The alloca() routine never returns NULL.  This will cause code paths
** that deal with sqlite3StackAlloc() failures to be unreachable.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_USE_ALLOCA
# define sqlite3_ರಾಶಿ_Alloc_Raw(D,N)   alloca(N)
# define sqlite3_ರಾಶಿ_Alloc_Raw_NN(D,N) alloca(N)
# define sqlite3_ರಾಶಿ_Free(D,P)
# define sqlite3_ರಾಶಿ_Free_NN(D,P)
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_ರಾಶಿ_Alloc_Raw(D,N)   sqlite3DbMallocRaw(D,N)
# define sqlite3_ರಾಶಿ_Alloc_Raw_NN(D,N) sqlite3DbMallocRawNN(D,N)
# define sqlite3_ರಾಶಿ_Free(D,P)       sqlite3DbFree(D,P)
# define sqlite3_ರಾಶಿ_Free_NN(D,P)     sqlite3DbFreeNN(D,P)
#ಆದರೆ_ಅಂತ್ಯ

/* Do not allow both MEMSYS5 and MEMSYS3 to be defined together.  If they
** are, disable MEMSYS3
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_MEMSYS5
ಸ್ಥಿರಾಂಕ sqlite3_mem_methods *sqlite3MemGetMemsys5(ಶೂನ್ಯ);
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SQLITE_ENABLE_MEMSYS3
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_MEMSYS3
ಸ್ಥಿರಾಂಕ sqlite3_mem_methods *sqlite3MemGetMemsys3(ಶೂನ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MUTEX_OMIT
  sqlite3_mutex_methods ಸ್ಥಿರಾಂಕ *sqlite3DefaultMutex(ಶೂನ್ಯ);
  sqlite3_mutex_methods ಸ್ಥಿರಾಂಕ *sqlite3NoopMutex(ಶೂನ್ಯ);
  sqlite3_mutex *sqlite3MutexAlloc(ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ sqlite3MutexInit(ಶೂನ್ಯ);
  ಪೂರ್ಣಾಂಕ sqlite3_Mutex_ಕೊನೆ(ಶೂನ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(SQLITE_MUTEX_OMIT) && !defined(SQLITE_MUTEX_NOOP)
  ಶೂನ್ಯ sqlite3_ಸ್ಮರಣೆ_Barrier(ಶೂನ್ಯ);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_ಸ್ಮರಣೆ_Barrier()
#ಆದರೆ_ಅಂತ್ಯ

sqlite3_int64 sqlite3_ಸ್ಥಿತಿ_ಮೌಲ್ಯ(ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಸ್ಥಿತಿ_ಮೇಲೆ(ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಸ್ಥಿತಿ_ಕೆಳಕ್ಕೆ(ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಸ್ಥಿತಿ_Highwater(ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_Lookaside_ಬಳಸಿದ(sqlite3*,ಪೂರ್ಣಾಂಕ*);

/* Access to mutexes used by sqlite3_status() */
sqlite3_mutex *sqlite3Pcache1Mutex(ಶೂನ್ಯ);
sqlite3_mutex *sqlite3MallocMutex(ಶೂನ್ಯ);


/* The SQLITE_THREAD_MISUSE_WARNINGS compile-time option used to be called
** SQLITE_ENABLE_MULTITHREADED_CHECKS.  Keep that older macro for backwards
** compatibility, at least for a while... */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_MULTITHREADED_CHECKS
# define SQLITE_THREAD_MISUSE_WARNINGS 1
#ಆದರೆ_ಅಂತ್ಯ

/* SQLITE_THREAD_MISUSE_ABORT implies SQLITE_THREAD_MISUSE_WARNINGS */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_THREAD_MISUSE_ABORT
# define SQLITE_THREAD_MISUSE_WARNINGS 1
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(SQLITE_THREAD_MISUSE_WARNINGS) && !defined(SQLITE_MUTEX_OMIT)
ಶೂನ್ಯ sqlite3MutexWarnOnContention(sqlite3_mutex*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3MutexWarnOnContention(x)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
# define EXP754 (((u64)0x7ff)<<52)
# define MAN754 ((((u64)1)<<52)-1)
# define IsNaN(X) (((X)&EXP754)==EXP754 && ((X)&MAN754)!=0)
# define IsOvfl(X) (((X)&EXP754)==EXP754)
  ಪೂರ್ಣಾಂಕ sqlite3IsNaN(ದ್ವಿಗುಣ);
  ಪೂರ್ಣಾಂಕ sqlite3_Is_ಅತಿಪ್ರವಾಹ(ದ್ವಿಗುಣ);
#ಇಲ್ಲದಿದ್ದರೆ
# define IsNaN(X)             0
# define sqlite3IsNaN(X)      0
# define sqlite3_Is_ಅತಿಪ್ರವಾಹ_2(X) 0
#ಆದರೆ_ಅಂತ್ಯ

/*
** An instance of the following structure holds information about SQL
** functions arguments that are the parameters to the printf() function.
*/
ರಚನೆ PrintfArguments {
  ಪೂರ್ಣಾಂಕ nArg;                /* Total number of arguments */
  ಪೂರ್ಣಾಂಕ n_ಬಳಸಿದ;               /* Number of arguments used so far */
  sqlite3_ಮೌಲ್ಯ **apArg;   /* The argument values */
};

/*
** Maxium number of base-10 digits in an unsigned 64-bit integer
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_U64_DIGITS 20

/*
** An instance of this object receives the decoding of a floating point
** value into an approximate decimal representation.
*/
ರಚನೆ FpDecode {
  ಪೂರ್ಣಾಂಕ n;                           /* Significant digits in the decode */
  ಪೂರ್ಣಾಂಕ iDP;                         /* Location of the decimal point */
  ಅಕ್ಷರ *z;                         /* Start of significant digits */
  ಅಕ್ಷರ zBuf[SQLITE_U64_DIGITS+1];  /* Storage for significant digits */
  ಅಕ್ಷರ sign;                       /* '+' or '-' */
  ಅಕ್ಷರ isSpecial;                  /* 1: Infinity  2: NaN */
};

ಶೂನ್ಯ sqlite3FpDecode(FpDecode*,ದ್ವಿಗುಣ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
ಅಕ್ಷರ *sqlite3MPrintf(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ...);
ಅಕ್ಷರ *sqlite3VMPrintf(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, va_list);
#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  ಶೂನ್ಯ sqlite3DebugPrintf(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ...);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_TEST)
  ಶೂನ್ಯ *sqlite3_Test_ಪಠ್ಯ_ಗಮ್ಯ_Ptr(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_Test_Ptr_ಗಮ್ಯ_ಪಠ್ಯ(ಶೂನ್ಯ*);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(SQLITE_DEBUG)
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, ...);
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*, u8);
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Bare_ಅಭಿವ್ಯಕ್ತಿ_List(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List*, u8, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Bare_Id_List(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ IdList*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Id_List(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ IdList*, u8, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಸ್ತಂಭ_List(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ ಸ್ತಂಭ*, ಪೂರ್ಣಾಂಕ, u8);
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Src_List(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ SrcList*);
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Select(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ Select*, u8);
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_With(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ With*, u8);
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Upsert(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ Upsert*, u8);
#ಆದರೆ TREETRACE_ENABLED
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Delete(ಸ್ಥಿರಾಂಕ With*, ಸ್ಥಿರಾಂಕ SrcList*, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*,
                             ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List*,ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*, ಸ್ಥಿರಾಂಕ Trigger*);
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಸೇರಿಸು(ಸ್ಥಿರಾಂಕ With*, ಸ್ಥಿರಾಂಕ SrcList*,
                             ಸ್ಥಿರಾಂಕ IdList*, ಸ್ಥಿರಾಂಕ Select*, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List*,
                             ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ Upsert*, ಸ್ಥಿರಾಂಕ Trigger*);
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Update(ಸ್ಥಿರಾಂಕ With*, ಸ್ಥಿರಾಂಕ SrcList*, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List*,
                             ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List*, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*,
                             ಸ್ಥಿರಾಂಕ Upsert*, ಸ್ಥಿರಾಂಕ Trigger*);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Trigger_ಹೆಜ್ಜೆ(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ Trigger_ಹೆಜ್ಜೆ*, u8, u8);
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Trigger(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ Trigger*, u8, u8);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಕಿಟಕಿ(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ ಕಿಟಕಿ*, u8);
  ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Win_Func(ವೃಕ್ಷ_View*, ಸ್ಥಿರಾಂಕ ಕಿಟಕಿ*, u8);
#ಆದರೆ_ಅಂತ್ಯ
  ಶೂನ್ಯ sqlite3_Show_ಅಭಿವ್ಯಕ್ತಿ(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*);
  ಶೂನ್ಯ sqlite3_Show_ಅಭಿವ್ಯಕ್ತಿ_List(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List*);
  ಶೂನ್ಯ sqlite3ShowIdList(ಸ್ಥಿರಾಂಕ IdList*);
  ಶೂನ್ಯ sqlite3ShowSrcList(ಸ್ಥಿರಾಂಕ SrcList*);
  ಶೂನ್ಯ sqlite3ShowSelect(ಸ್ಥಿರಾಂಕ Select*);
  ಶೂನ್ಯ sqlite3ShowWith(ಸ್ಥಿರಾಂಕ With*);
  ಶೂನ್ಯ sqlite3ShowUpsert(ಸ್ಥಿರಾಂಕ Upsert*);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
  ಶೂನ್ಯ sqlite3_Show_Trigger_ಹೆಜ್ಜೆ(ಸ್ಥಿರಾಂಕ Trigger_ಹೆಜ್ಜೆ*);
  ಶೂನ್ಯ sqlite3_Show_Trigger_ಹೆಜ್ಜೆ_List(ಸ್ಥಿರಾಂಕ Trigger_ಹೆಜ್ಜೆ*);
  ಶೂನ್ಯ sqlite3ShowTrigger(ಸ್ಥಿರಾಂಕ Trigger*);
  ಶೂನ್ಯ sqlite3ShowTriggerList(ಸ್ಥಿರಾಂಕ Trigger*);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
  ಶೂನ್ಯ sqlite3_Show_ಕಿಟಕಿ(ಸ್ಥಿರಾಂಕ ಕಿಟಕಿ*);
  ಶೂನ್ಯ sqlite3ShowWinFunc(ಸ್ಥಿರಾಂಕ ಕಿಟಕಿ*);
#ಆದರೆ_ಅಂತ್ಯ
  ಶೂನ್ಯ sqlite3ShowBitvec(Bitvec*);
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ sqlite3_ಗಣ_String(ಅಕ್ಷರ **, sqlite3*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3_ಪ್ರಗತಿ_Check(Parse*);
ಶೂನ್ಯ sqlite3_Error_ಸಂದೇಶ(Parse*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ...);
ಪೂರ್ಣಾಂಕ sqlite3_Error_ಗಮ್ಯ_Parser(sqlite3*,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3Dequote(ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3_Dequote_ಅಭಿವ್ಯಕ್ತಿ(ಅಭಿವ್ಯಕ್ತಿ*);
ಶೂನ್ಯ sqlite3DequoteToken(Token*);
ಶೂನ್ಯ sqlite3_Dequote_ಸಂಖ್ಯೆ(Parse*, ಅಭಿವ್ಯಕ್ತಿ*);
ಶೂನ್ಯ sqlite3TokenInit(Token*,ಅಕ್ಷರ*);
ಪೂರ್ಣಾಂಕ sqlite3_Keyword_ಸಂಕೇತ(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಸತತ_Parser(Parse*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3_ಮುಕ್ತಾಯ_Coding(Parse*);
ಪೂರ್ಣಾಂಕ sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(Parse*);
ಶೂನ್ಯ sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(Parse*,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(Parse*,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(Parse*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_Clear_ತಾತ್ಕಾಲಿಕ_Reg_Cache(Parse*);
ಶೂನ್ಯ sqlite3_Touch_ರಿಜಿಸ್ಟರ್(Parse*,ಪೂರ್ಣಾಂಕ);
#ಆದರೆ defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
ಪೂರ್ಣಾಂಕ sqlite3_ಮೊದಲ_ಲಭ್ಯ_ರಿಜಿಸ್ಟರ್(Parse*,ಪೂರ್ಣಾಂಕ);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
ಪೂರ್ಣಾಂಕ sqlite3_No_Temps_ಒಳ_Range(Parse*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
#ಆದರೆ_ಅಂತ್ಯ
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(sqlite3*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ Token*,ಪೂರ್ಣಾಂಕ);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ(sqlite3*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(sqlite3*,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Attach_Subtrees(sqlite3*,ಅಭಿವ್ಯಕ್ತಿ*,ಅಭಿವ್ಯಕ್ತಿ*,ಅಭಿವ್ಯಕ್ತಿ*);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3PExpr(Parse*, ಪೂರ್ಣಾಂಕ, ಅಭಿವ್ಯಕ್ತಿ*, ಅಭಿವ್ಯಕ್ತಿ*);
ಶೂನ್ಯ sqlite3_PExpr_ಕೂಡಿಸು_Select(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, Select*);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(Parse*,ಅಭಿವ್ಯಕ್ತಿ*, ಅಭಿವ್ಯಕ್ತಿ*);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Simplified_And_Or(ಅಭಿವ್ಯಕ್ತಿ*);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Function(Parse*,ಅಭಿವ್ಯಕ್ತಿ_List*, ಸ್ಥಿರಾಂಕ Token*, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Function_ಕ್ರಮ_By(Parse*,ಅಭಿವ್ಯಕ್ತಿ*,ಅಭಿವ್ಯಕ್ತಿ_List*);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕ್ರಮ_By_Aggregate_Error(Parse*,ಅಭಿವ್ಯಕ್ತಿ*);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Function_Usable(Parse*,ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*,ಸ್ಥಿರಾಂಕ FuncDef*);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ನಿಯೋಜಿಸು_Var_ಸಂಖ್ಯೆ(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, u32);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(sqlite3*, ಅಭಿವ್ಯಕ್ತಿ*);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete_Generic(sqlite3*,ಶೂನ್ಯ*);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Deferred_Delete(Parse*, ಅಭಿವ್ಯಕ್ತಿ*);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Unmap_And_Delete(Parse*, ಅಭಿವ್ಯಕ್ತಿ*);
ಅಭಿವ್ಯಕ್ತಿ_List *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(Parse*,ಅಭಿವ್ಯಕ್ತಿ_List*,ಅಭಿವ್ಯಕ್ತಿ*);
ಅಭಿವ್ಯಕ್ತಿ_List *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append_ಸದಿಶ(Parse*,ಅಭಿವ್ಯಕ್ತಿ_List*,IdList*,ಅಭಿವ್ಯಕ್ತಿ*);
Select *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_ಗಮ್ಯ_ಮೌಲ್ಯಗಳು(Parse*, ಪೂರ್ಣಾಂಕ, ಅಭಿವ್ಯಕ್ತಿ_List*);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_ಗಣ_ವಿಂಗಡಿಸು_ಕ್ರಮ(ಅಭಿವ್ಯಕ್ತಿ_List*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_ಗಣ_Name(Parse*,ಅಭಿವ್ಯಕ್ತಿ_List*,ಸ್ಥಿರಾಂಕ Token*,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_ಗಣ_ಹರವು(Parse*,ಅಭಿವ್ಯಕ್ತಿ_List*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(sqlite3*, ಅಭಿವ್ಯಕ್ತಿ_List*);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete_Generic(sqlite3*,ಶೂನ್ಯ*);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Return_Subtype(Parse*,ಅಭಿವ್ಯಕ್ತಿ*);
u32 sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Flags(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List*);
ಪೂರ್ಣಾಂಕ sqlite3_ಸೂಚ್ಯಂಕ_Has_Duplicate_ಬೇರು_ಪುಟ(ಸೂಚ್ಯಂಕ*);
ಪೂರ್ಣಾಂಕ sqlite3Init(sqlite3*, ಅಕ್ಷರ**);
ಪೂರ್ಣಾಂಕ sqlite3InitCallback(ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ**, ಅಕ್ಷರ**);
ಪೂರ್ಣಾಂಕ sqlite3InitOne(sqlite3*, ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ**, u32);
ಶೂನ್ಯ sqlite3Pragma(Parse*,Token*,Token*,Token*,ಪೂರ್ಣಾಂಕ);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
Module *sqlite3_Pragma_Vtab_ರಿಜಿಸ್ಟರ್(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName);
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3_Reset_All_Schemas_ದ_Connection(sqlite3*);
ಶೂನ್ಯ sqlite3ResetOneSchema(sqlite3*,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3CollapseDatabaseArray(sqlite3*);
ಶೂನ್ಯ sqlite3_Commit_ಆಂತರಿಕ_Changes(sqlite3*);
ಶೂನ್ಯ sqlite3_ಸ್ತಂಭ_ಗಣ_ಅಭಿವ್ಯಕ್ತಿ(Parse*,ಕೋಷ್ಟಕ*,ಸ್ತಂಭ*,ಅಭಿವ್ಯಕ್ತಿ*);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(ಕೋಷ್ಟಕ*,ಸ್ತಂಭ*);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ_Auth(ಕೋಷ್ಟಕ*,ಸ್ತಂಭ*,Parse*);
ಶೂನ್ಯ sqlite3_ಸ್ತಂಭ_ಗಣ_Coll(sqlite3*,ಸ್ತಂಭ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*zColl);
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_ಸ್ತಂಭ_Coll(ಸ್ತಂಭ*);
ಶೂನ್ಯ sqlite3_Delete_ಸ್ತಂಭ_ಹೆಸರುಗಳು(sqlite3*,ಕೋಷ್ಟಕ*);
ಶೂನ್ಯ sqlite3_Generate_ಸ್ತಂಭ_ಹೆಸರುಗಳು(Parse *pParse, Select *pSelect);
ಪೂರ್ಣಾಂಕ sqlite3_Columns_From_ಅಭಿವ್ಯಕ್ತಿ_List(Parse*,ಅಭಿವ್ಯಕ್ತಿ_List*,i16*,ಸ್ತಂಭ**);
ಶೂನ್ಯ sqlite3_Subquery_ಸ್ತಂಭ_Types(Parse*,ಕೋಷ್ಟಕ*,Select*,ಅಕ್ಷರ);
ಕೋಷ್ಟಕ *sqlite3_ಫಲಿತಾಂಶ_ಗಣ_ದ_Select(Parse*,Select*,ಅಕ್ಷರ);
ಶೂನ್ಯ sqlite3_Open_Schema_ಕೋಷ್ಟಕ(Parse *, ಪೂರ್ಣಾಂಕ);
ಸೂಚ್ಯಂಕ *sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(ಕೋಷ್ಟಕ*);
ಪೂರ್ಣಾಂಕ sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(ಸೂಚ್ಯಂಕ*, ಪೂರ್ಣಾಂಕ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_GENERATED_COLUMNS
# define sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(T,X) (X)  /* No-op pass-through */
# define sqlite3_Storage_ಸ್ತಂಭ_ಗಮ್ಯ_ಕೋಷ್ಟಕ(T,X) (X)  /* No-op pass-through */
#ಇಲ್ಲದಿದ್ದರೆ
  i16 sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(ಕೋಷ್ಟಕ*, i16);
  i16 sqlite3_Storage_ಸ್ತಂಭ_ಗಮ್ಯ_ಕೋಷ್ಟಕ(ಕೋಷ್ಟಕ*, i16);
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3_ಆರಂಭ_ಕೋಷ್ಟಕ(Parse*,Token*,Token*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
#ಆದರೆ SQLITE_ENABLE_HIDDEN_COLUMNS
  ಶೂನ್ಯ sqlite3_ಸ್ತಂಭ_Properties_From_Name(ಕೋಷ್ಟಕ*, ಸ್ತಂಭ*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_ಸ್ತಂಭ_Properties_From_Name(T,C) /* no-op */
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3_ಕೂಡಿಸು_ಸ್ತಂಭ(Parse*,Token,Token);
ಶೂನ್ಯ sqlite3_ಕೂಡಿಸು_Not_Null(Parse*, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಕೂಡಿಸು_Primary_ಕೀಲಿ(Parse*, ಅಭಿವ್ಯಕ್ತಿ_List*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಕೂಡಿಸು_Check_Constraint(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3_ಕೂಡಿಸು_Default_ಮೌಲ್ಯ(Parse*,ಅಭಿವ್ಯಕ್ತಿ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3_ಕೂಡಿಸು_Collate_Type(Parse*, Token*);
ಶೂನ್ಯ sqlite3_ಕೂಡಿಸು_Generated(Parse*,ಅಭಿವ್ಯಕ್ತಿ*,Token*);
ಶೂನ್ಯ sqlite3_ಕೊನೆ_ಕೋಷ್ಟಕ(Parse*,Token*,Token*,u32,Select*);
ಶೂನ್ಯ sqlite3_ಕೂಡಿಸು_Returning(Parse*,ಅಭಿವ್ಯಕ್ತಿ_List*);
ಪೂರ್ಣಾಂಕ sqlite3ParseUri(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ*,
                    sqlite3_vfs**,ಅಕ್ಷರ**,ಅಕ್ಷರ **);
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3CodecQueryParameters(A,B,C) 0
Btree *sqlite3_Db_Name_ಗಮ್ಯ_Btree(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UNTESTABLE
# define sqlite3FaultSim(X) SQLITE_OK
#ಇಲ್ಲದಿದ್ದರೆ
  ಪೂರ್ಣಾಂಕ sqlite3FaultSim(ಪೂರ್ಣಾಂಕ);
#ಆದರೆ_ಅಂತ್ಯ

Bitvec *sqlite3BitvecCreate(u32);
ಪೂರ್ಣಾಂಕ sqlite3BitvecTest(Bitvec*, u32);
ಪೂರ್ಣಾಂಕ sqlite3BitvecTestNotNull(Bitvec*, u32);
ಪೂರ್ಣಾಂಕ sqlite3_Bitvec_ಗಣ(Bitvec*, u32);
ಶೂನ್ಯ sqlite3BitvecClear(Bitvec*, u32, ಶೂನ್ಯ*);
ಶೂನ್ಯ sqlite3BitvecDestroy(Bitvec*);
u32 sqlite3_Bitvec_ಗಾತ್ರ(Bitvec*);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_UNTESTABLE
ಪೂರ್ಣಾಂಕ sqlite3BitvecBuiltinTest(ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ*);
#ಆದರೆ_ಅಂತ್ಯ

ಅಡ್ಡಸಾಲು_ಗಣ *sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Init(sqlite3*);
ಶೂನ್ಯ sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Delete(ಶೂನ್ಯ*);
ಶೂನ್ಯ sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Clear(ಶೂನ್ಯ*);
ಶೂನ್ಯ sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_ಸೇರಿಸು(ಅಡ್ಡಸಾಲು_ಗಣ*, i64);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_Test(ಅಡ್ಡಸಾಲು_ಗಣ*, ಪೂರ್ಣಾಂಕ iBatch, i64);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಡ್ಡಸಾಲು_ಗಣ_ಮುಂದಿನ(ಅಡ್ಡಸಾಲು_ಗಣ*, i64*);

ಶೂನ್ಯ sqlite3CreateView(Parse*,Token*,Token*,Token*,ಅಭಿವ್ಯಕ್ತಿ_List*,Select*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);

#ಆದರೆ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
  ಪೂರ್ಣಾಂಕ sqlite3_View_Get_ಸ್ತಂಭ_ಹೆಸರುಗಳು(Parse*,ಕೋಷ್ಟಕ*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_View_Get_ಸ್ತಂಭ_ಹೆಸರುಗಳು(A,B) 0
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ SQLITE_MAX_ATTACHED>30
  ಪೂರ್ಣಾಂಕ sqlite3_Db_ಮುಖವಾಡ_All_Zero(y_Db_ಮುಖವಾಡ);
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3_Drop_ಕೋಷ್ಟಕ(Parse*, SrcList*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಸಂಕೇತ_Drop_ಕೋಷ್ಟಕ(Parse*, ಕೋಷ್ಟಕ*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_Delete_ಕೋಷ್ಟಕ(sqlite3*, ಕೋಷ್ಟಕ*);
ಶೂನ್ಯ sqlite3_Delete_ಕೋಷ್ಟಕ_Generic(sqlite3*, ಶೂನ್ಯ*);
ಶೂನ್ಯ sqlite3_Free_ಸೂಚ್ಯಂಕ(sqlite3*, ಸೂಚ್ಯಂಕ*);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINCREMENT
  ಶೂನ್ಯ sqlite3AutoincrementBegin(Parse *pParse);
  ಶೂನ್ಯ sqlite3_Autoincrement_ಕೊನೆ(Parse *pParse);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3AutoincrementBegin(X)
# define sqlite3_Autoincrement_ಕೊನೆ(X)
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3_ಸೇರಿಸು(Parse*, SrcList*, Select*, IdList*, ಪೂರ್ಣಾಂಕ, Upsert*);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
  ಶೂನ್ಯ sqlite3ComputeGeneratedColumns(Parse*, ಪೂರ್ಣಾಂಕ, ಕೋಷ್ಟಕ*);
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ *sqlite3ArrayAllocate(sqlite3*,ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ*,ಪೂರ್ಣಾಂಕ*);
IdList *sqlite3IdListAppend(Parse*, IdList*, Token*);
ಪೂರ್ಣಾಂಕ sqlite3_Id_List_ಸೂಚ್ಯಂಕ(IdList*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
SrcList *sqlite3SrcListEnlarge(Parse*, SrcList*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
SrcList *sqlite3SrcListAppendList(Parse *pParse, SrcList *p1, SrcList *p2);
SrcList *sqlite3SrcListAppend(Parse*, SrcList*, Token*, Token*);
ಶೂನ್ಯ sqlite3SubqueryDelete(sqlite3*,Subquery*);
Select *sqlite3SubqueryDetach(sqlite3*,Src_ಅಂಶ*);
ಪೂರ್ಣಾಂಕ sqlite3_Src_ಅಂಶ_Attach_Subquery(Parse*, Src_ಅಂಶ*, Select*, ಪೂರ್ಣಾಂಕ);
SrcList *sqlite3_Src_List_Append_From_ಪದ(Parse*, SrcList*, Token*, Token*,
                                      Token*, Select*, OnOrUsing*);
ಶೂನ್ಯ sqlite3_Src_List_ಸೂಚ್ಯಂಕಿತ_By(Parse *, SrcList *, Token *);
ಶೂನ್ಯ sqlite3_Src_List_Func_ವಾದಗಳು(Parse*, SrcList*, ಅಭಿವ್ಯಕ್ತಿ_List*);
ಪೂರ್ಣಾಂಕ sqlite3_ಸೂಚ್ಯಂಕಿತ_By_Lookup(Parse *, Src_ಅಂಶ *);
ಶೂನ್ಯ sqlite3_Src_List_ಸರಿಸು_Join_Type(Parse*,SrcList*);
ಶೂನ್ಯ sqlite3_Src_List_ನಿಯೋಜಿಸು_Cursors(Parse*, SrcList*);
ಶೂನ್ಯ sqlite3IdListDelete(sqlite3*, IdList*);
ಶೂನ್ಯ sqlite3ClearOnOrUsing(sqlite3*, OnOrUsing*);
ಶೂನ್ಯ sqlite3SrcListDelete(sqlite3*, SrcList*);
ಸೂಚ್ಯಂಕ *sqlite3_Allocate_ಸೂಚ್ಯಂಕ_Object(sqlite3*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಅಕ್ಷರ**);
ಪೂರ್ಣಾಂಕ sqlite3_ಸೂಚ್ಯಂಕ_Bloomable(ಸ್ಥಿರಾಂಕ ಸೂಚ್ಯಂಕ*,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_Create_ಸೂಚ್ಯಂಕ(Parse*,Token*,Token*,SrcList*,ಅಭಿವ್ಯಕ್ತಿ_List*,ಪೂರ್ಣಾಂಕ,Token*,
                          ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, u8);
ಶೂನ್ಯ sqlite3_Drop_ಸೂಚ್ಯಂಕ(Parse*, SrcList*, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3Select(Parse*, Select*, SelectDest*);
Select *sqlite3SelectNew(Parse*,ಅಭಿವ್ಯಕ್ತಿ_List*,SrcList*,ಅಭಿವ್ಯಕ್ತಿ*,ಅಭಿವ್ಯಕ್ತಿ_List*,
                         ಅಭಿವ್ಯಕ್ತಿ*,ಅಭಿವ್ಯಕ್ತಿ_List*,u32,ಅಭಿವ್ಯಕ್ತಿ*);
ಶೂನ್ಯ sqlite3SelectDelete(sqlite3*, Select*);
ಶೂನ್ಯ sqlite3SelectDeleteGeneric(sqlite3*,ಶೂನ್ಯ*);
ಶೂನ್ಯ sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect);
ಕೋಷ್ಟಕ *sqlite3SrcListLookup(Parse*, SrcList*);
ಪೂರ್ಣಾಂಕ sqlite3IsReadOnly(Parse*, ಕೋಷ್ಟಕ*, Trigger*);
ಶೂನ್ಯ sqlite3_Open_ಕೋಷ್ಟಕ(Parse*, ಪೂರ್ಣಾಂಕ iCur, ಪೂರ್ಣಾಂಕ iDb, ಕೋಷ್ಟಕ*, ಪೂರ್ಣಾಂಕ);
#ಆದರೆ defined(SQLITE_ENABLE_UPDATE_DELETE_ಎಲ್ಲೆ) && !defined(SQLITE_OMIT_SUBQUERY)
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಎಲ್ಲೆ_Where(Parse*,SrcList*,ಅಭಿವ್ಯಕ್ತಿ*,ಅಭಿವ್ಯಕ್ತಿ_List*,ಅಭಿವ್ಯಕ್ತಿ*,ಅಕ್ಷರ*);
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3_ಸಂಕೇತ_Change_ಎಣಿಕೆ(Vdbe*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3DeleteFrom(Parse*, SrcList*, ಅಭಿವ್ಯಕ್ತಿ*, ಅಭಿವ್ಯಕ್ತಿ_List*, ಅಭಿವ್ಯಕ್ತಿ*);
ಶೂನ್ಯ sqlite3Update(Parse*, SrcList*, ಅಭಿವ್ಯಕ್ತಿ_List*,ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ,ಅಭಿವ್ಯಕ್ತಿ_List*,ಅಭಿವ್ಯಕ್ತಿ*,
                   Upsert*);
WhereInfo *sqlite3WhereBegin(Parse*,SrcList*,ಅಭಿವ್ಯಕ್ತಿ*,ಅಭಿವ್ಯಕ್ತಿ_List*,
                             ಅಭಿವ್ಯಕ್ತಿ_List*,Select*,u16,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_Where_ಕೊನೆ(WhereInfo*);
LogEst sqlite3_Where_Output_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ(WhereInfo*);
ಪೂರ್ಣಾಂಕ sqlite3WhereIsDistinct(WhereInfo*);
ಪೂರ್ಣಾಂಕ sqlite3WhereIsOrdered(WhereInfo*);
ಪೂರ್ಣಾಂಕ sqlite3_Where_ಕ್ರಮ_By_ಎಲ್ಲೆ_Opt_Label(WhereInfo*);
ಶೂನ್ಯ sqlite3_Where_Min_Max_Opt_Early_ಹೊರಹೋಗು(Vdbe*,WhereInfo*);
ಪೂರ್ಣಾಂಕ sqlite3WhereIsSorted(WhereInfo*);
ಪೂರ್ಣಾಂಕ sqlite3WhereContinueLabel(WhereInfo*);
ಪೂರ್ಣಾಂಕ sqlite3WhereBreakLabel(WhereInfo*);
ಪೂರ್ಣಾಂಕ sqlite3WhereOkOnePass(WhereInfo*, ಪೂರ್ಣಾಂಕ*);
#ವ್ಯಾಖ್ಯಾನಿಸು ONEPASS_OFF      0        /* Use of ONEPASS not allowed */
#ವ್ಯಾಖ್ಯಾನಿಸು ONEPASS_SINGLE   1        /* ONEPASS valid for a single row update */
#ವ್ಯಾಖ್ಯಾನಿಸು ONEPASS_MULTI    2        /* ONEPASS is valid for multiple rows */
ಪೂರ್ಣಾಂಕ sqlite3_Where_ಬಳಸುತ್ತದೆ_Deferred_Seek(WhereInfo*);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತುಂಬು_ಸೂಚ್ಯಂಕ_ಸ್ತಂಭ(Parse*, ಸೂಚ್ಯಂಕ*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ(Parse*, ಕೋಷ್ಟಕ*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, u8);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(Vdbe*, ಕೋಷ್ಟಕ*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Move(Parse*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ iReg);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Generated_ಸ್ತಂಭ(Parse*, ಕೋಷ್ಟಕ*, ಸ್ತಂಭ*, ಪೂರ್ಣಾಂಕ);
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Copy(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Factorable(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಸತತ_Just_Once(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Null_ರಿಜಿಸ್ಟರ್_Range(Parse*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ*);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_List(Parse*, ಅಭಿವ್ಯಕ್ತಿ_List*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, u8);
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ECEL_DUP      0x01  /* Deep, not shallow copies */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ECEL_ಗುಣಕ   0x02  /* Factor out constant terms */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ECEL_REF      0x04  /* Use ExprList.u.x.iOrderByCol */
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_ECEL_OMITREF  0x08  /* Omit if ExprList.u.x.iOrderByCol */
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_True(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False_Dup(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಕೋಷ್ಟಕ *sqlite3_ಕಂಡುಹಿಡಿ_ಕೋಷ್ಟಕ(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
#ವ್ಯಾಖ್ಯಾನಿಸು LOCATE_VIEW    0x01
#ವ್ಯಾಖ್ಯಾನಿಸು LOCATE_NOERR   0x02
ಕೋಷ್ಟಕ *sqlite3_Locate_ಕೋಷ್ಟಕ(Parse*,u32 flags,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_Preferred_ಕೋಷ್ಟಕ_Name(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಕೋಷ್ಟಕ *sqlite3_Locate_ಕೋಷ್ಟಕ_ಅಂಶ(Parse*,u32 flags,Src_ಅಂಶ *);
ಸೂಚ್ಯಂಕ *sqlite3_ಕಂಡುಹಿಡಿ_ಸೂಚ್ಯಂಕ(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3_Unlink_And_Delete_ಕೋಷ್ಟಕ(sqlite3*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3_Unlink_And_Delete_ಸೂಚ್ಯಂಕ(sqlite3*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3Vacuum(Parse*,Token*,ಅಭಿವ್ಯಕ್ತಿ*);
ಪೂರ್ಣಾಂಕ sqlite3_ಸತತ_Vacuum(ಅಕ್ಷರ**, sqlite3*, ಪೂರ್ಣಾಂಕ, sqlite3_ಮೌಲ್ಯ*);
ಅಕ್ಷರ *sqlite3NameFromToken(sqlite3*, ಸ್ಥಿರಾಂಕ Token*);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(ಸ್ಥಿರಾಂಕ Parse*,ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*,ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare_Skip(ಅಭಿವ್ಯಕ್ತಿ*,ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Compare(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List*,ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List*, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_ಅಭಿವ್ಯಕ್ತಿ(ಸ್ಥಿರಾಂಕ Parse*,ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*,ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Non_Null_ಅಡ್ಡಸಾಲು(ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3AggInfoPersistWalkerInit(Walker*,Parse*);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Aggregates(NameContext*, ಅಭಿವ್ಯಕ್ತಿ*);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Agg_List(NameContext*,ಅಭಿವ್ಯಕ್ತಿ_List*);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Covered_By_ಸೂಚ್ಯಂಕ(ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ iCur, ಸೂಚ್ಯಂಕ *pIdx);
ಪೂರ್ಣಾಂಕ sqlite3ReferencesSrcList(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, SrcList*);
Vdbe *sqlite3GetVdbe(Parse*);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_UNTESTABLE
ಶೂನ್ಯ sqlite3PrngSaveState(ಶೂನ್ಯ);
ಶೂನ್ಯ sqlite3PrngRestoreState(ಶೂನ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3RollbackAll(sqlite3*,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಸಂಕೇತ_Verify_Schema(Parse*, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಸಂಕೇತ_Verify_Named_Schema(Parse*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb);
ಶೂನ್ಯ sqlite3BeginTransaction(Parse*, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಕೊನೆ_Transaction(Parse*,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3Savepoint(Parse*, ಪೂರ್ಣಾಂಕ, Token*);
ಶೂನ್ಯ sqlite3CloseSavepoints(sqlite3 *);
ಶೂನ್ಯ sqlite3LeaveMutexAndCloseZombie(sqlite3*);
u32 sqlite3IsTrueOrFalse(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Id_ಗಮ್ಯ_True_False(ಅಭಿವ್ಯಕ್ತಿ*);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(Parse*,ಅಭಿವ್ಯಕ್ತಿ*);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Or_Function(ಅಭಿವ್ಯಕ್ತಿ*, u8);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Or_ಗುಂಪು_By(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, ಅಭಿವ್ಯಕ್ತಿ_List*);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಕೋಷ್ಟಕ_Constant(ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Single_ಕೋಷ್ಟಕ_Constraint(ಅಭಿವ್ಯಕ್ತಿ*,ಸ್ಥಿರಾಂಕ SrcList*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Is_Constant(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ_List *pList, ಪೂರ್ಣಾಂಕ bNoIs);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಒಳಗೊಂಡಿದೆ_Subquery(ಅಭಿವ್ಯಕ್ತಿ*);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ*, Parse*, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Needs_No_Affinity_Change(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*, ಅಕ್ಷರ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Like_Operator(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*);
ಪೂರ್ಣಾಂಕ sqlite3IsRowid(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3RowidAlias(ಕೋಷ್ಟಕ *pTab);
ಶೂನ್ಯ sqlite3_Generate_ಅಡ್ಡಸಾಲು_Delete(
    Parse*,ಕೋಷ್ಟಕ*,Trigger*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,i16,u8,u8,u8,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_Generate_ಅಡ್ಡಸಾಲು_ಸೂಚ್ಯಂಕ_Delete(Parse*, ಕೋಷ್ಟಕ*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ*, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_Generate_ಸೂಚ್ಯಂಕ_ಕೀಲಿ(Parse*, ಸೂಚ್ಯಂಕ*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ*,ಸೂಚ್ಯಂಕ*,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3ResolvePartIdxLabel(Parse*,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_References_Updated_ಸ್ತಂಭ(ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ*,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3GenerateConstraintChecks(Parse*,ಕೋಷ್ಟಕ*,ಪೂರ್ಣಾಂಕ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,
                                     u8,u8,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ*,ಪೂರ್ಣಾಂಕ*,Upsert*);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_NULL_TRIM
  ಶೂನ್ಯ sqlite3_ಗಣ_Make_ದಾಖಲೆ_P5(Vdbe*,ಕೋಷ್ಟಕ*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_ಗಣ_Make_ದಾಖಲೆ_P5(A,B)
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3CompleteInsertion(Parse*,ಕೋಷ್ಟಕ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_Open_ಕೋಷ್ಟಕ_And_Indices(Parse*, ಕೋಷ್ಟಕ*, ಪೂರ್ಣಾಂಕ, u8, ಪೂರ್ಣಾಂಕ, u8*, ಪೂರ್ಣಾಂಕ*, ಪೂರ್ಣಾಂಕ*);
ಶೂನ್ಯ sqlite3BeginWriteOperation(Parse*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3MultiWrite(Parse*);
ಶೂನ್ಯ sqlite3MayAbort(Parse*);
ಶೂನ್ಯ sqlite3HaltConstraint(Parse*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ*, i8, u8);
ಶೂನ್ಯ sqlite3UniqueConstraint(Parse*, ಪೂರ್ಣಾಂಕ, ಸೂಚ್ಯಂಕ*);
ಶೂನ್ಯ sqlite3RowidConstraint(Parse*, ಪೂರ್ಣಾಂಕ, ಕೋಷ್ಟಕ*);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ);
ಅಭಿವ್ಯಕ್ತಿ_List *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List*,ಪೂರ್ಣಾಂಕ);
SrcList *sqlite3SrcListDup(sqlite3*,ಸ್ಥಿರಾಂಕ SrcList*,ಪೂರ್ಣಾಂಕ);
IdList *sqlite3IdListDup(sqlite3*,ಸ್ಥಿರಾಂಕ IdList*);
Select *sqlite3SelectDup(sqlite3*,ಸ್ಥಿರಾಂಕ Select*,ಪೂರ್ಣಾಂಕ);
FuncDef *sqlite3_Function_ಹುಡುಕು(ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3_ಸೇರಿಸು_Builtin_Funcs(FuncDef*,ಪೂರ್ಣಾಂಕ);
FuncDef *sqlite3_ಕಂಡುಹಿಡಿ_Function(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ,u8,u8);
ಶೂನ್ಯ sqlite3_Quote_ಮೌಲ್ಯ(StrAccum*,sqlite3_ಮೌಲ್ಯ*,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3AppendOneUtf8Character(ಅಕ್ಷರ*, u32);
ಶೂನ್ಯ sqlite3_ರಿಜಿಸ್ಟರ್_Builtin_Functions(ಶೂನ್ಯ);
ಶೂನ್ಯ sqlite3_ರಿಜಿಸ್ಟರ್_Date_ಸಮಯ_Functions(ಶೂನ್ಯ);
ಶೂನ್ಯ sqlite3_ರಿಜಿಸ್ಟರ್_Json_Functions(ಶೂನ್ಯ);
ಶೂನ್ಯ sqlite3_ರಿಜಿಸ್ಟರ್_ಪ್ರತಿ_Connection_Builtin_Functions(sqlite3*);
#ಆದರೆ !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
  Module *sqlite3_Json_Vtab_ರಿಜಿಸ್ಟರ್(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3SafetyCheckOk(sqlite3*);
ಪೂರ್ಣಾಂಕ sqlite3SafetyCheckSickOrOk(sqlite3*);
ಶೂನ್ಯ sqlite3ChangeCookie(Parse*, ಪೂರ್ಣಾಂಕ);
With *sqlite3WithDup(sqlite3 *db, With *p);

#ಆದರೆ !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
  Module *sqlite3_Carray_ರಿಜಿಸ್ಟರ್(sqlite3*);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
ಶೂನ್ಯ sqlite3MaterializeView(Parse*, ಕೋಷ್ಟಕ*, ಅಭಿವ್ಯಕ್ತಿ*, ಅಭಿವ್ಯಕ್ತಿ_List*,ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
  ಶೂನ್ಯ sqlite3BeginTrigger(Parse*, Token*,Token*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,IdList*,SrcList*,
                           ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ sqlite3_ಮುಕ್ತಾಯ_Trigger(Parse*, Trigger_ಹೆಜ್ಜೆ*, Token*);
  ಶೂನ್ಯ sqlite3DropTrigger(Parse*, SrcList*, ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ sqlite3DropTriggerPtr(Parse*, Trigger*);
  Trigger *sqlite3TriggersExist(Parse *, ಕೋಷ್ಟಕ*, ಪೂರ್ಣಾಂಕ, ಅಭಿವ್ಯಕ್ತಿ_List*, ಪೂರ್ಣಾಂಕ *p_ಮುಖವಾಡ);
  Trigger *sqlite3TriggerList(Parse *, ಕೋಷ್ಟಕ *);
  ಶೂನ್ಯ sqlite3_ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger(Parse*, Trigger *, ಪೂರ್ಣಾಂಕ, ಅಭಿವ್ಯಕ್ತಿ_List*, ಪೂರ್ಣಾಂಕ, ಕೋಷ್ಟಕ *,
                            ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ sqlite3_ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger_ನೇರ(Parse *, Trigger *, ಕೋಷ್ಟಕ *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ sqliteViewTriggers(Parse*, ಕೋಷ್ಟಕ*, ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ, ಅಭಿವ್ಯಕ್ತಿ_List*);
  ಶೂನ್ಯ sqlite3_Delete_Trigger_ಹೆಜ್ಜೆ(sqlite3*, Trigger_ಹೆಜ್ಜೆ*);
  Trigger_ಹೆಜ್ಜೆ *sqlite3_Trigger_Select_ಹೆಜ್ಜೆ(sqlite3*,Select*,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  Trigger_ಹೆಜ್ಜೆ *sqlite3_Trigger_ಸೇರಿಸು_ಹೆಜ್ಜೆ(Parse*,SrcList*, IdList*,
                                        Select*,u8,Upsert*,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  Trigger_ಹೆಜ್ಜೆ *sqlite3_Trigger_Update_ಹೆಜ್ಜೆ(Parse*,SrcList*,SrcList*,ಅಭಿವ್ಯಕ್ತಿ_List*,
                                        ಅಭಿವ್ಯಕ್ತಿ*, u8, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  Trigger_ಹೆಜ್ಜೆ *sqlite3_Trigger_Delete_ಹೆಜ್ಜೆ(Parse*,SrcList*, ಅಭಿವ್ಯಕ್ತಿ*,
                                        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಶೂನ್ಯ sqlite3DeleteTrigger(sqlite3*, Trigger*);
  ಶೂನ್ಯ sqlite3UnlinkAndDeleteTrigger(sqlite3*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  u32 sqlite3TriggerColmask(Parse*,Trigger*,ಅಭಿವ್ಯಕ್ತಿ_List*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಕೋಷ್ಟಕ*,ಪೂರ್ಣಾಂಕ);
# define sqlite3ParseToplevel(p) ((p)->pToplevel ? (p)->pToplevel : (p))
# define sqlite3IsToplevel(p) ((p)->pToplevel==0)
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3TriggersExist(B,C,D,E,F) 0
# define sqlite3DeleteTrigger(A,B)
# define sqlite3DropTriggerPtr(A,B)
# define sqlite3UnlinkAndDeleteTrigger(A,B,C)
# define sqlite3_ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger(A,B,C,D,E,F,G,H,I)
# define sqlite3_ಸಂಕೇತ_ಅಡ್ಡಸಾಲು_Trigger_ನೇರ(A,B,C,D,E,F)
# define sqlite3TriggerList(X, Y) 0
# define sqlite3ParseToplevel(p) p
# define sqlite3IsToplevel(p) 1
# define sqlite3TriggerColmask(A,B,C,D,E,F,G) 0
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ sqlite3JoinType(Parse*, Token*, Token*, Token*);
ಪೂರ್ಣಾಂಕ sqlite3_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(ಕೋಷ್ಟಕ *pTab, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಲಂಬಸಾಲು);
ಶೂನ್ಯ sqlite3_Src_ಅಂಶ_ಸ್ತಂಭ_ಬಳಸಿದ(Src_ಅಂಶ*,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಗಣ_Join_ಅಭಿವ್ಯಕ್ತಿ(ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ,u32);
ಶೂನ್ಯ sqlite3_Create_Foreign_ಕೀಲಿ(Parse*, ಅಭಿವ್ಯಕ್ತಿ_List*, Token*, ಅಭಿವ್ಯಕ್ತಿ_List*, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_Defer_Foreign_ಕೀಲಿ(Parse*, ಪೂರ್ಣಾಂಕ);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
  ಶೂನ್ಯ sqlite3AuthRead(Parse*,ಅಭಿವ್ಯಕ್ತಿ*,Schema*,SrcList*);
  ಪೂರ್ಣಾಂಕ sqlite3AuthCheck(Parse*,ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಶೂನ್ಯ sqlite3_Auth_Context_ತಳ್ಳು(Parse*, AuthContext*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಶೂನ್ಯ sqlite3_Auth_Context_ತೆಗೆ(AuthContext*);
  ಪೂರ್ಣಾಂಕ sqlite3_Auth_Read_ಲಂಬಸಾಲು(Parse*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3AuthRead(a,b,c,d)
# define sqlite3AuthCheck(a,b,c,d,e)    SQLITE_OK
# define sqlite3_Auth_Context_ತಳ್ಳು(a,b,c)
# define sqlite3_Auth_Context_ತೆಗೆ(a)  ((ಶೂನ್ಯ)(a))
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3DbIsNamed(sqlite3 *db, ಪೂರ್ಣಾಂಕ iDb, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName);
ಶೂನ್ಯ sqlite3Attach(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, ಅಭಿವ್ಯಕ್ತಿ*, ಅಭಿವ್ಯಕ್ತಿ*);
ಶೂನ್ಯ sqlite3Detach(Parse*, ಅಭಿವ್ಯಕ್ತಿ*);
ಶೂನ್ಯ sqlite3FixInit(DbFixer*, Parse*, ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ಥಿರಾಂಕ Token*);
ಪೂರ್ಣಾಂಕ sqlite3FixSrcList(DbFixer*, SrcList*);
ಪೂರ್ಣಾಂಕ sqlite3FixSelect(DbFixer*, Select*);
ಪೂರ್ಣಾಂಕ sqlite3_Fix_ಅಭಿವ್ಯಕ್ತಿ(DbFixer*, ಅಭಿವ್ಯಕ್ತಿ*);
ಪೂರ್ಣಾಂಕ sqlite3_Fix_Trigger_ಹೆಜ್ಜೆ(DbFixer*, Trigger_ಹೆಜ್ಜೆ*);

ಪೂರ್ಣಾಂಕ sqlite3RealSameAsInt(ದ್ವಿಗುಣ,sqlite3_int64);
i64 sqlite3_Real_ಗಮ್ಯ_I64(ದ್ವಿಗುಣ);
ಪೂರ್ಣಾಂಕ sqlite3_Int64_ಗಮ್ಯ_ಪಠ್ಯ(i64,ಅಕ್ಷರ*);
ಪೂರ್ಣಾಂಕ sqlite3AtoF(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, ದ್ವಿಗುಣ*);
ಪೂರ್ಣಾಂಕ sqlite3GetInt32(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ*);
ಪೂರ್ಣಾಂಕ sqlite3GetUInt32(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, u32*);
ಪೂರ್ಣಾಂಕ sqlite3Atoi(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಪೂರ್ಣಾಂಕ sqlite3Utf16ByteLen(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p_ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ nByte, ಪೂರ್ಣಾಂಕ nChar);
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3Utf8CharLen(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p_ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ nByte);
u32 sqlite3Utf8Read(ಸ್ಥಿರಾಂಕ u8**);
ಪೂರ್ಣಾಂಕ sqlite3Utf8ReadLimited(ಸ್ಥಿರಾಂಕ u8*, ಪೂರ್ಣಾಂಕ, u32*);
LogEst sqlite3LogEst(u64);
LogEst sqlite3_Log_Est_ಕೂಡಿಸು(LogEst,LogEst);
LogEst sqlite3LogEstFromDouble(ದ್ವಿಗುಣ);
u64 sqlite3_Log_Est_ಗಮ್ಯ_Int(LogEst);
VList *sqlite3_VList_ಕೂಡಿಸು(sqlite3*,VList*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_VList_ಅಂಕೆ_ಗಮ್ಯ_Name(VList*,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_VList_Name_ಗಮ್ಯ_ಅಂಕೆ(VList*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ);

/*
** Routines to read and write variable-length integers.  These used to
** be defined locally, but now we use the varint routines in the util.c
** file.
*/
ಪೂರ್ಣಾಂಕ sqlite3PutVarint(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*, u64);
u8 sqlite3GetVarint(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *, u64 *);
i64 sqlite3_Varint_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*);
u8 sqlite3GetVarint32(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *, u32 *);
ಪೂರ್ಣಾಂಕ sqlite3VarintLen(u64 v);

/*
** The common case is for a varint to be a single byte.  They following
** macros handle the common case without a procedure call, but then call
** the procedure for larger varints.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು getVarint32(A,B)  \
  (u8)((*(A)<(u8)0x80)?((B)=(u32)*(A)),1:sqlite3GetVarint32((A),(u32 *)&(B)))
#ವ್ಯಾಖ್ಯಾನಿಸು getVarint32NR(A,B) \
  B=(u32)*(A);ಆದರೆ(B>=0x80)sqlite3GetVarint32((A),(u32*)&(B))
#ವ್ಯಾಖ್ಯಾನಿಸು putVarint32(A,B)  \
  (u8)(((u32)(B)<(u32)0x80)?(*(A)=(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(B)),1:\
  sqlite3PutVarint((A),(B)))

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3_ಸೂಚ್ಯಂಕ_Affinity_Str(sqlite3*, ಸೂಚ್ಯಂಕ*);
ಅಕ್ಷರ *sqlite3_ಕೋಷ್ಟಕ_Affinity_Str(sqlite3*,ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ*);
ಶೂನ್ಯ sqlite3_ಕೋಷ್ಟಕ_Affinity(Vdbe*, ಕೋಷ್ಟಕ*, ಪೂರ್ಣಾಂಕ);
ಅಕ್ಷರ sqlite3CompareAffinity(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಅಕ್ಷರ aff2);
ಪೂರ್ಣಾಂಕ sqlite3_ಸೂಚ್ಯಂಕ_Affinity_Ok(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಅಕ್ಷರ idx_affinity);
ಅಕ್ಷರ sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_Affinity(ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ*,ಪೂರ್ಣಾಂಕ);
ಅಕ್ಷರ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ದತ್ತಾಂಶ_Type(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ);
ಪೂರ್ಣಾಂಕ sqlite3Atoi64(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, i64*, ಪೂರ್ಣಾಂಕ, u8);
ಪೂರ್ಣಾಂಕ sqlite3_Dec_Or_Hex_ಗಮ್ಯ_I64(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, i64*);
ಶೂನ್ಯ sqlite3_Error_With_ಸಂದೇಶ(sqlite3*, ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,...);
ಶೂನ್ಯ sqlite3Error(sqlite3*,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3ErrorClear(sqlite3*);
ಶೂನ್ಯ sqlite3SystemError(sqlite3*,ಪೂರ್ಣಾಂಕ);
#ಆದರೆ !defined(SQLITE_OMIT_BLOB_LITERAL)
ಶೂನ್ಯ *sqlite3_Hex_ಗಮ್ಯ_Blob(sqlite3*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z, ಪೂರ್ಣಾಂಕ n);
#ಆದರೆ_ಅಂತ್ಯ
u8 sqlite3_Hex_ಗಮ್ಯ_Int(ಪೂರ್ಣಾಂಕ h);
ಪೂರ್ಣಾಂಕ sqlite3TwoPartName(Parse *, Token *, Token *, Token **);

#ಆದರೆ defined(SQLITE_NEED_ERR_NAME)
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DESERIALIZE
ಪೂರ್ಣಾಂಕ sqlite3MemdbInit(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ sqlite3IsMemdb(ಸ್ಥಿರಾಂಕ sqlite3_vfs*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3IsMemdb(X) 0
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrStr(ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3ReadSchema(Parse *pParse);
CollSeq *sqlite3_ಕಂಡುಹಿಡಿ_Coll_Seq(sqlite3*,u8 enc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ದ್ವಿಮಾನ_Compare(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*);
#ಆದರೆ SQLITE_ಬಳಸುತ್ತದೆ_INLINE
ಸ್ಥಿರ SQLITE_INLINE ಪೂರ್ಣಾಂಕ sqlite3_Is_ದ್ವಿಮಾನ(ಸ್ಥಿರಾಂಕ CollSeq *p){
  ಪ್ರತಿಜ್ಞಾ( p==0 || p->xCmp!=sqlite3_ದ್ವಿಮಾನ_Compare
       || strcmp(p->zName,"BINARY")==0 );
  ಹಿಂತಿರುಗಿ p==0 || p->xCmp==sqlite3_ದ್ವಿಮಾನ_Compare;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ sqlite3_Is_ದ್ವಿಮಾನ(ಸ್ಥಿರಾಂಕ CollSeq*);
#ಆದರೆ_ಅಂತ್ಯ
CollSeq *sqlite3LocateCollSeq(Parse *pParse, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*zName);
ಶೂನ್ಯ sqlite3_ಗಣ_ಪಠ್ಯ_Encoding(sqlite3 *db, u8);
CollSeq *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(Parse *pParse, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ);
CollSeq *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_NNColl_Seq(Parse *pParse, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq_ಹೊಂದಿಕೆ(Parse*,ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*,ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Collate_Token(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ*, ಸ್ಥಿರಾಂಕ Token*, ಪೂರ್ಣಾಂಕ);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Collate_String(Parse*,ಅಭಿವ್ಯಕ್ತಿ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate(ಅಭಿವ್ಯಕ್ತಿ*);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(ಅಭಿವ್ಯಕ್ತಿ*);
ಪೂರ್ಣಾಂಕ sqlite3CheckCollSeq(Parse *, CollSeq *);
ಪೂರ್ಣಾಂಕ sqlite3WritableSchema(sqlite3*);
ಪೂರ್ಣಾಂಕ sqlite3CheckObjectName(Parse*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3_Vdbe_ಗಣ_Changes(sqlite3 *, i64);
ಪೂರ್ಣಾಂಕ sqlite3_ಕೂಡಿಸು_Int64(i64*,i64);
ಪೂರ್ಣಾಂಕ sqlite3SubInt64(i64*,i64);
ಪೂರ್ಣಾಂಕ sqlite3MulInt64(i64*,i64);
ಪೂರ್ಣಾಂಕ sqlite3AbsInt32(ಪೂರ್ಣಾಂಕ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_8_3_ಹೆಸರುಗಳು
ಶೂನ್ಯ sqlite3FileSuffix3(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಅಕ್ಷರ*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3FileSuffix3(X,Y)
#ಆದರೆ_ಅಂತ್ಯ
u8 sqlite3GetBoolean(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z,u8);

ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ_2(sqlite3_ಮೌಲ್ಯ*, u8);
ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_Is_ದ_Class(ಸ್ಥಿರಾಂಕ sqlite3_ಮೌಲ್ಯ*, ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು_2(sqlite3_ಮೌಲ್ಯ*, u8);
ಶೂನ್ಯ sqlite3_ಮೌಲ್ಯ_ಗಣ_Str(sqlite3_ಮೌಲ್ಯ*, ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *,u8,
                        ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
ಶೂನ್ಯ sqlite3_ಮೌಲ್ಯ_ಗಣ_Null(sqlite3_ಮೌಲ್ಯ*);
ಶೂನ್ಯ sqlite3_ಮೌಲ್ಯ_Free(sqlite3_ಮೌಲ್ಯ*);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_UNTESTABLE
ಶೂನ್ಯ sqlite3_ಫಲಿತಾಂಶ_Int_Real(sqlite3_context*);
#ಆದರೆ_ಅಂತ್ಯ
sqlite3_ಮೌಲ್ಯ *sqlite3_ಮೌಲ್ಯ_New(sqlite3 *);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UTF16
ಅಕ್ಷರ *sqlite3Utf16to8(sqlite3 *, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ, u8);
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(sqlite3 *, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *, u8, u8, sqlite3_ಮೌಲ್ಯ **);
ಶೂನ್ಯ sqlite3_ಮೌಲ್ಯ_Apply_Affinity(sqlite3_ಮೌಲ್ಯ *, u8, u8);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_AMALGAMATION
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ sqlite3OpcodeProperty[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ sqlite3_Str_ದ್ವಿಮಾನ[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ sqlite3StdTypeLen[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ sqlite3StdTypeAffinity[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3StdType[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ sqlite3_ಊರ್ಧ್ವ_ಗಮ್ಯ_ಅಧೋ[];
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *sqlite3aLTb;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *sqlite3aEQb;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *sqlite3aGTb;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ sqlite3CtypeMap[];
ಬಾಹ್ಯ SQLITE_WSD ರಚನೆ Sqlite3Config sqlite3Config;
ಬಾಹ್ಯ FuncDefHash sqlite3BuiltinFunctions;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ sqlite3_str sqlite3OomStr;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WSD
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ sqlite3PendingByte;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_AMALGAMATION */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ VDBE_PROFILE
ಬಾಹ್ಯ sqlite3_uint64 sqlite3NProfileCnt;
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3_ಬೇರು_ಪುಟ_Moved(sqlite3*, ಪೂರ್ಣಾಂಕ, Pgno, Pgno);
ಶೂನ್ಯ sqlite3Reindex(Parse*, Token*, Token*);
ಶೂನ್ಯ sqlite3AlterFunctions(ಶೂನ್ಯ);
ಶೂನ್ಯ sqlite3_Alter_Rename_ಕೋಷ್ಟಕ(Parse*, SrcList*, Token*);
ಶೂನ್ಯ sqlite3_Alter_Rename_ಸ್ತಂಭ(Parse*, SrcList*, Token*, Token*);
ಶೂನ್ಯ sqlite3AlterDropConstraint(Parse*,SrcList*,Token*,Token*);
ಶೂನ್ಯ sqlite3_Alter_ಕೂಡಿಸು_Constraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *p_ಮೊದಲ,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಅಭಿವ್ಯಕ್ತಿ,       /* Text of CHECK expression */
  ಪೂರ್ಣಾಂಕ n_ಅಭಿವ್ಯಕ್ತಿ,               /* Size of pExpr in bytes */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ              /* The parsed CHECK expression */
);
ಶೂನ್ಯ sqlite3_Alter_ಗಣ_Not_Null(Parse*, SrcList*, Token*, Token*);
i64 sqlite3GetToken(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ *);
ಶೂನ್ಯ sqlite3NestedParse(Parse*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ...);
ಶೂನ್ಯ sqlite3ExpirePreparedStatements(sqlite3*, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಸಂಕೇತ_Rhs_ದ_ಒಳ(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_ಸಂಕೇತ_Subselect(Parse*, ಅಭಿವ್ಯಕ್ತಿ*);
ಶೂನ್ಯ sqlite3SelectPrep(Parse*, Select*, NameContext*);
ಪೂರ್ಣಾಂಕ sqlite3ExpandSubquery(Parse*, Src_ಅಂಶ*);
ಶೂನ್ಯ sqlite3_Select_Wrong_ಅಂಕೆ_Terms_Error(Parse *pParse, Select *p);
ಪೂರ್ಣಾಂಕ sqlite3_ಹೊಂದಿಕೆ_EName(
  ಸ್ಥಿರಾಂಕ ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ*,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,
  ಪೂರ್ಣಾಂಕ*
);
Bitmask sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಲಂಬಸಾಲು_ಬಳಸಿದ(ಅಭಿವ್ಯಕ್ತಿ*);
u8 sqlite3StrIHash(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಪೂರ್ಣಾಂಕ sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(NameContext*, ಅಭಿವ್ಯಕ್ತಿ*);
ಪೂರ್ಣಾಂಕ sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_ಹೆಸರುಗಳು(NameContext*, ಅಭಿವ್ಯಕ್ತಿ_List*);
ಶೂನ್ಯ sqlite3_Resolve_Select_ಹೆಸರುಗಳು(Parse*, Select*, NameContext*);
ಪೂರ್ಣಾಂಕ sqlite3ResolveSelfReference(Parse*,ಕೋಷ್ಟಕ*,ಪೂರ್ಣಾಂಕ,ಅಭಿವ್ಯಕ್ತಿ*,ಅಭಿವ್ಯಕ್ತಿ_List*);
ಪೂರ್ಣಾಂಕ sqlite3_Resolve_ಕ್ರಮ_ಗುಂಪು_By(Parse*, Select*, ಅಭಿವ್ಯಕ್ತಿ_List*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3_ಸ್ತಂಭ_Default(Vdbe *, ಕೋಷ್ಟಕ *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_Alter_ಮುಕ್ತಾಯ_ಕೂಡಿಸು_ಸ್ತಂಭ(Parse *, Token *);
ಶೂನ್ಯ sqlite3_Alter_Begin_ಕೂಡಿಸು_ಸ್ತಂಭ(Parse *, SrcList *);
ಶೂನ್ಯ sqlite3_Alter_Drop_ಸ್ತಂಭ(Parse*, SrcList*, ಸ್ಥಿರಾಂಕ Token*);
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3RenameTokenMap(Parse*, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*, ಸ್ಥಿರಾಂಕ Token*);
ಶೂನ್ಯ sqlite3RenameTokenRemap(Parse*, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p_ಗಮ್ಯ, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pFrom);
ಶೂನ್ಯ sqlite3_Rename_ಅಭಿವ್ಯಕ್ತಿ_Unmap(Parse*, ಅಭಿವ್ಯಕ್ತಿ*);
ಶೂನ್ಯ sqlite3RenameExprlistUnmap(Parse*, ಅಭಿವ್ಯಕ್ತಿ_List*);
CollSeq *sqlite3GetCollSeq(Parse*, u8, CollSeq *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಅಕ್ಷರ sqlite3AffinityType(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಸ್ತಂಭ*);
ಶೂನ್ಯ sqlite3Analyze(Parse*, Token*, Token*);
ಪೂರ್ಣಾಂಕ sqlite3InvokeBusyHandler(BusyHandler*);
ಪೂರ್ಣಾಂಕ sqlite3_ಕಂಡುಹಿಡಿ_Db(sqlite3*, Token*);
ಪೂರ್ಣಾಂಕ sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(sqlite3 *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
ಪೂರ್ಣಾಂಕ sqlite3_Analysis_ತುಂಬು(sqlite3*,ಪೂರ್ಣಾಂಕ iDB);
ಶೂನ್ಯ sqlite3_Delete_ಸೂಚ್ಯಂಕ_Samples(sqlite3*,ಸೂಚ್ಯಂಕ*);
ಶೂನ್ಯ sqlite3_Default_ಅಡ್ಡಸಾಲು_Est(ಸೂಚ್ಯಂಕ*);
ಶೂನ್ಯ sqlite3_ರಿಜಿಸ್ಟರ್_Like_Functions(sqlite3*, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3IsLikeFunction(sqlite3*,ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ*,ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3SchemaClear(ಶೂನ್ಯ *);
Schema *sqlite3SchemaGet(sqlite3 *, Btree *);
ಪೂರ್ಣಾಂಕ sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(sqlite3 *db, Schema *);
ಕೀಲಿ_Info *sqlite3_ಕೀಲಿ_Info_Alloc(sqlite3*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಕೀಲಿ_Info_Unref(ಕೀಲಿ_Info*);
ಕೀಲಿ_Info *sqlite3_ಕೀಲಿ_Info_Ref(ಕೀಲಿ_Info*);
ಕೀಲಿ_Info *sqlite3_ಕೀಲಿ_Info_ದ_ಸೂಚ್ಯಂಕ(Parse*, ಸೂಚ್ಯಂಕ*);
ಕೀಲಿ_Info *sqlite3_ಕೀಲಿ_Info_From_ಅಭಿವ್ಯಕ್ತಿ_List(Parse*, ಅಭಿವ್ಯಕ್ತಿ_List*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3SelectOpName(ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3HasExplicitNulls(Parse*, ಅಭಿವ್ಯಕ್ತಿ_List*);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
ಪೂರ್ಣಾಂಕ sqlite3_ಕೀಲಿ_Info_Is_Writeable(ಕೀಲಿ_Info*);
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3CreateFunc(sqlite3 *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಶೂನ್ಯ *,
  ಶೂನ್ಯ (*)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ **),
  ಶೂನ್ಯ (*)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ **),
  ಶೂನ್ಯ (*)(sqlite3_context*),
  ಶೂನ್ಯ (*)(sqlite3_context*),
  ಶೂನ್ಯ (*)(sqlite3_context*,ಪೂರ್ಣಾಂಕ,sqlite3_ಮೌಲ್ಯ **),
  FuncDestructor *pDestructor
);
ಶೂನ್ಯ sqlite3NoopDestructor(ಶೂನ್ಯ*);
ಶೂನ್ಯ *sqlite3OomFault(sqlite3*);
ಶೂನ್ಯ sqlite3OomClear(sqlite3*);
ಪೂರ್ಣಾಂಕ sqlite3ApiExit(sqlite3 *db, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_Open_ತಾತ್ಕಾಲಿಕ_Database(Parse *);

ಅಕ್ಷರ *sqlite3RCStrRef(ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3RCStrUnref(ಶೂನ್ಯ*);
ಅಕ್ಷರ *sqlite3RCStrNew(u64);
ಅಕ್ಷರ *sqlite3RCStrResize(ಅಕ್ಷರ*,u64);

ಶೂನ್ಯ sqlite3StrAccumInit(StrAccum*, sqlite3*, ಅಕ್ಷರ*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3StrAccumEnlarge(StrAccum*, i64);
ಪೂರ್ಣಾಂಕ sqlite3StrAccumEnlargeIfNeeded(StrAccum*, i64);
ಅಕ್ಷರ *sqlite3_Str_Accum_ಮುಕ್ತಾಯ(StrAccum*);
ಶೂನ್ಯ sqlite3_Str_Accum_ಗಣ_Error(StrAccum*, u8);
ಶೂನ್ಯ sqlite3SelectDestInit(SelectDest*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_Create_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(sqlite3 *, SrcList *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ದಾಖಲೆ_Error_Byte_ಸ್ಥಾನಾಂತರ(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಶೂನ್ಯ sqlite3_ದಾಖಲೆ_Error_ಸ್ಥಾನಾಂತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ(sqlite3*,ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*);

ಶೂನ್ಯ sqlite3BackupRestart(sqlite3_backup *);
ಶೂನ್ಯ sqlite3BackupUpdate(sqlite3_backup *, Pgno, ಸ್ಥಿರಾಂಕ u8 *);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_ಒಳ(Parse*, ಅಭಿವ್ಯಕ್ತಿ*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_ಒಳ(x,y) SQLITE_OK
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STAT4
ಪೂರ್ಣಾಂಕ sqlite3_Stat4_Probe_ಗಣ_ಮೌಲ್ಯ(
    Parse*,ಸೂಚ್ಯಂಕ*,Unpacked_ದಾಖಲೆ**,ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ*);
ಪೂರ್ಣಾಂಕ sqlite3_Stat4_ಮೌಲ್ಯ_From_ಅಭಿವ್ಯಕ್ತಿ(Parse*, ಅಭಿವ್ಯಕ್ತಿ*, u8, sqlite3_ಮೌಲ್ಯ**);
ಶೂನ್ಯ sqlite3Stat4ProbeFree(Unpacked_ದಾಖಲೆ*);
ಪೂರ್ಣಾಂಕ sqlite3_Stat4_ಸ್ತಂಭ(sqlite3*, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, sqlite3_ಮೌಲ್ಯ**);
ಅಕ್ಷರ sqlite3_ಸೂಚ್ಯಂಕ_ಸ್ತಂಭ_Affinity(sqlite3*, ಸೂಚ್ಯಂಕ*, ಪೂರ್ಣಾಂಕ);
#ಆದರೆ_ಅಂತ್ಯ

/*
** The interface to the LEMON-generated parser
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_AMALGAMATION
  ಶೂನ್ಯ *sqlite3ParserAlloc(ಶೂನ್ಯ*(*)(u64), Parse*);
  ಶೂನ್ಯ sqlite3ParserFree(ಶೂನ್ಯ*, ಶೂನ್ಯ(*)(ಶೂನ್ಯ*));
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3Parser(ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ, Token);
ಪೂರ್ಣಾಂಕ sqlite3ParserFallback(ಪೂರ್ಣಾಂಕ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ YYTRACKMAXSTACKDEPTH
  ಪೂರ್ಣಾಂಕ sqlite3_Parser_ರಾಶಿ_Peak(ಶೂನ್ಯ*);
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ sqlite3_Auto_ತುಂಬು_Extensions(sqlite3*);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
  ಶೂನ್ಯ sqlite3CloseExtensions(sqlite3*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3CloseExtensions(X)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
  ಶೂನ್ಯ sqlite3_ಕೋಷ್ಟಕ_ಬೀಗ(Parse *, ಪೂರ್ಣಾಂಕ, Pgno, u8, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಕೋಷ್ಟಕ_ಬೀಗ(v,w,x,y,z)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  ಪೂರ್ಣಾಂಕ sqlite3Utf8To8(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_VIRTUALTABLE
#  define sqlite3VtabClear(D,T)
#  define sqlite3VtabSync(X,Y) SQLITE_OK
#  define sqlite3VtabRollback(X)
#  define sqlite3VtabCommit(X)
#  define sqlite3_Vtab_ಒಳ_Sync(db) 0
#  define sqlite3_Vtab_ಬೀಗ(X)
#  define sqlite3VtabUnlock(X)
#  define sqlite3VtabModuleUnref(D,X)
#  define sqlite3VtabUnlockList(X)
#  define sqlite3VtabSavepoint(X, Y, Z) SQLITE_OK
#  define sqlite3GetVTable(X,Y)  ((VTable*)0)
#ಇಲ್ಲದಿದ್ದರೆ
   ಶೂನ್ಯ sqlite3VtabClear(sqlite3 *db, ಕೋಷ್ಟಕ*);
   ಶೂನ್ಯ sqlite3VtabDisconnect(sqlite3 *db, ಕೋಷ್ಟಕ *p);
   ಪೂರ್ಣಾಂಕ sqlite3VtabSync(sqlite3 *db, Vdbe*);
   ಪೂರ್ಣಾಂಕ sqlite3VtabRollback(sqlite3 *db);
   ಪೂರ್ಣಾಂಕ sqlite3VtabCommit(sqlite3 *db);
   ಶೂನ್ಯ sqlite3_Vtab_ಬೀಗ(VTable *);
   ಶೂನ್ಯ sqlite3VtabUnlock(VTable *);
   ಶೂನ್ಯ sqlite3VtabModuleUnref(sqlite3*,Module*);
   ಶೂನ್ಯ sqlite3VtabUnlockList(sqlite3*);
   ಪೂರ್ಣಾಂಕ sqlite3VtabSavepoint(sqlite3 *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
   ಶೂನ್ಯ sqlite3VtabImportErrmsg(Vdbe*, sqlite3_vtab*);
   VTable *sqlite3GetVTable(sqlite3*, ಕೋಷ್ಟಕ*);
   Module *sqlite3VtabCreateModule(
     sqlite3*,
     ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,
     ಸ್ಥಿರಾಂಕ sqlite3_module*,
     ಶೂನ್ಯ*,
     ಶೂನ್ಯ(*)(ಶೂನ್ಯ*)
   );
#  define sqlite3_Vtab_ಒಳ_Sync(db) ((db)->nVTrans>0 && (db)->aVTrans==0)
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3ReadOnlyShadowTables(sqlite3 *db);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಪೂರ್ಣಾಂಕ sqlite3_Shadow_ಕೋಷ್ಟಕ_Name(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName);
  ಪೂರ್ಣಾಂಕ sqlite3_Is_Shadow_ಕೋಷ್ಟಕ_ದ(sqlite3*,ಕೋಷ್ಟಕ*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
  ಶೂನ್ಯ sqlite3_Mark_All_Shadow_Tables_ದ(sqlite3*, ಕೋಷ್ಟಕ*);
  ಶೂನ್ಯ sqlite3_Vtab_Eponymous_ಕೋಷ್ಟಕ_Clear_All(sqlite3*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_Shadow_ಕೋಷ್ಟಕ_Name(A,B) 0
# define sqlite3_Is_Shadow_ಕೋಷ್ಟಕ_ದ(A,B,C) 0
# define sqlite3_Mark_All_Shadow_Tables_ದ(A,B)
# define sqlite3_Vtab_Eponymous_ಕೋಷ್ಟಕ_Clear_All(X)
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3_Vtab_Eponymous_ಕೋಷ್ಟಕ_Init(Parse*,Module*);
ಶೂನ್ಯ sqlite3_Vtab_Eponymous_ಕೋಷ್ಟಕ_Clear(sqlite3*,Module*);
ಶೂನ್ಯ sqlite3VtabMakeWritable(Parse*,ಕೋಷ್ಟಕ*);
ಶೂನ್ಯ sqlite3VtabBeginParse(Parse*, Token*, Token*, Token*, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_Vtab_ಮುಕ್ತಾಯ_Parse(Parse*, Token*);
ಶೂನ್ಯ sqlite3VtabArgInit(Parse*);
ಶೂನ್ಯ sqlite3VtabArgExtend(Parse*, Token*);
ಪೂರ್ಣಾಂಕ sqlite3VtabCallCreate(sqlite3*, ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಅಕ್ಷರ **);
ಪೂರ್ಣಾಂಕ sqlite3VtabCallConnect(Parse*, ಕೋಷ್ಟಕ*);
ಪೂರ್ಣಾಂಕ sqlite3VtabCallDestroy(sqlite3*, ಪೂರ್ಣಾಂಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
ಪೂರ್ಣಾಂಕ sqlite3VtabBegin(sqlite3 *, VTable *);

FuncDef *sqlite3VtabOverloadFunction(sqlite3 *,FuncDef*, ಪೂರ್ಣಾಂಕ nArg, ಅಭಿವ್ಯಕ್ತಿ*);
ಶೂನ್ಯ sqlite3_Vtab_ಬಳಸುತ್ತದೆ_All_Schemas(Parse*);
sqlite3_int64 sqlite3_Stmt_Current_ಸಮಯ(sqlite3_context*);
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Parameter_ಸೂಚ್ಯಂಕ(Vdbe*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3TransferBindings(sqlite3_stmt *, sqlite3_stmt *);
ಶೂನ್ಯ sqlite3ParseObjectInit(Parse*,sqlite3*);
ಶೂನ್ಯ sqlite3ParseObjectReset(Parse*);
ಶೂನ್ಯ *sqlite3_Parser_ಕೂಡಿಸು_Cleanup(Parse*,ಶೂನ್ಯ(*)(sqlite3*,ಶೂನ್ಯ*),ಶೂನ್ಯ*);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_NORMALIZE
ಅಕ್ಷರ *sqlite3Normalize(Vdbe*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3Reprepare(Vdbe*);
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Check_ಉದ್ದಳತೆ(Parse*, ಅಭಿವ್ಯಕ್ತಿ_List*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
CollSeq *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare_Coll_Seq(Parse*,ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*);
CollSeq *sqlite3_ದ್ವಿಮಾನ_Compare_Coll_Seq(Parse *, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ*);
ಪೂರ್ಣಾಂಕ sqlite3_ತಾತ್ಕಾಲಿಕ_ಒಳ_ಸ್ಮರಣೆ(ಸ್ಥಿರಾಂಕ sqlite3*);
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3JournalModename(ಪೂರ್ಣಾಂಕ);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  ಪೂರ್ಣಾಂಕ sqlite3Checkpoint(sqlite3*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ*, ಪೂರ್ಣಾಂಕ*);
  ಪೂರ್ಣಾಂಕ sqlite3WalDefaultHook(ಶೂನ್ಯ*,sqlite3*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CTE
  Cte *sqlite3CteNew(Parse*,Token*,ಅಭಿವ್ಯಕ್ತಿ_List*,Select*,u8);
  ಶೂನ್ಯ sqlite3CteDelete(sqlite3*,Cte*);
  With *sqlite3_With_ಕೂಡಿಸು(Parse*,With*,Cte*);
  ಶೂನ್ಯ sqlite3WithDelete(sqlite3*,With*);
  ಶೂನ್ಯ sqlite3WithDeleteGeneric(sqlite3*,ಶೂನ್ಯ*);
  With *sqlite3_With_ತಳ್ಳು(Parse*, With*, u8);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3CteNew(P,T,E,S)   ((ಶೂನ್ಯ*)0)
# define sqlite3CteDelete(D,C)
# define sqlite3_Cte_With_ಕೂಡಿಸು(P,W,C) ((ಶೂನ್ಯ*)0)
# define sqlite3WithDelete(x,y)
# define sqlite3_With_ತಳ್ಳು(x,y,z) ((ಶೂನ್ಯ*)0)
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UPSERT
  Upsert *sqlite3UpsertNew(sqlite3*,ಅಭಿವ್ಯಕ್ತಿ_List*,ಅಭಿವ್ಯಕ್ತಿ*,ಅಭಿವ್ಯಕ್ತಿ_List*,ಅಭಿವ್ಯಕ್ತಿ*,Upsert*);
  ಶೂನ್ಯ sqlite3UpsertDelete(sqlite3*,Upsert*);
  Upsert *sqlite3UpsertDup(sqlite3*,Upsert*);
  ಪೂರ್ಣಾಂಕ sqlite3_Upsert_Analyze_ಗುರಿ(Parse*,SrcList*,Upsert*,Upsert*);
  ಶೂನ್ಯ sqlite3UpsertDoUpdate(Parse*,Upsert*,ಕೋಷ್ಟಕ*,ಸೂಚ್ಯಂಕ*,ಪೂರ್ಣಾಂಕ);
  Upsert *sqlite3_Upsert_ದ_ಸೂಚ್ಯಂಕ(Upsert*,ಸೂಚ್ಯಂಕ*);
  ಪೂರ್ಣಾಂಕ sqlite3_Upsert_ಮುಂದಿನ_Is_IPK(Upsert*);
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3UpsertNew(u,v,w,x,y,z) ((Upsert*)0)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3UpsertDelete(x,y)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3UpsertDup(x,y)         ((Upsert*)0)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_Upsert_ದ_ಸೂಚ್ಯಂಕ(x,y)     ((Upsert*)0)
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_Upsert_ಮುಂದಿನ_Is_IPK(x)     0
#ಆದರೆ_ಅಂತ್ಯ


/* Declarations for functions in fkey.c. All of these are replaced by
** no-op macros if OMIT_FOREIGN_KEY is defined. In this case no foreign
** key functionality is available. If OMIT_TRIGGER is defined but
** OMIT_FOREIGN_KEY is not, only some of the functions are no-oped. In
** this case foreign keys are parsed, but no other functionality is
** provided (enforcement of FK constraints requires the triggers sub-system).
*/
#ಆದರೆ !defined(SQLITE_OMIT_FOREIGN_ಕೀಲಿ) && !defined(SQLITE_OMIT_TRIGGER)
  ಶೂನ್ಯ sqlite3FkCheck(Parse*, ಕೋಷ್ಟಕ*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ*, ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ sqlite3_Fk_Drop_ಕೋಷ್ಟಕ(Parse*, SrcList *, ಕೋಷ್ಟಕ*);
  ಶೂನ್ಯ sqlite3FkActions(Parse*, ಕೋಷ್ಟಕ*, ಅಭಿವ್ಯಕ್ತಿ_List*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ*, ಪೂರ್ಣಾಂಕ);
  ಪೂರ್ಣಾಂಕ sqlite3FkRequired(Parse*, ಕೋಷ್ಟಕ*, ಪೂರ್ಣಾಂಕ*, ಪೂರ್ಣಾಂಕ);
  u32 sqlite3FkOldmask(Parse*, ಕೋಷ್ಟಕ*);
  FKey *sqlite3FkReferences(ಕೋಷ್ಟಕ *);
  ಶೂನ್ಯ sqlite3FkClearTriggerCache(sqlite3*,ಪೂರ್ಣಾಂಕ);
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3FkActions(a,b,c,d,e,f)
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3FkCheck(a,b,c,d,e,f)
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_Fk_Drop_ಕೋಷ್ಟಕ(a,b,c)
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3FkOldmask(a,b)         0
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3FkRequired(a,b,c,d)    0
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3FkReferences(a)        0
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3FkClearTriggerCache(a,b)
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FOREIGN_ಕೀಲಿ
  ಶೂನ್ಯ sqlite3FkDelete(sqlite3 *, ಕೋಷ್ಟಕ*);
  ಪೂರ್ಣಾಂಕ sqlite3_Fk_Locate_ಸೂಚ್ಯಂಕ(Parse*,ಕೋಷ್ಟಕ*,FKey*,ಸೂಚ್ಯಂಕ**,ಪೂರ್ಣಾಂಕ**);
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3FkDelete(a,b)
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_Fk_Locate_ಸೂಚ್ಯಂಕ(a,b,c,d,e)
#ಆದರೆ_ಅಂತ್ಯ


/*
** Available fault injectors.  Should be numbered beginning with 0.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FAULTINJECTOR_MALLOC     0
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_FAULTINJECTOR_ಎಣಿಕೆ      1

/*
** The interface to the code in fault.c used for identifying "benign"
** malloc failures. This is only present if SQLITE_UNTESTABLE
** is not defined.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_UNTESTABLE
  ಶೂನ್ಯ sqlite3BeginBenignMalloc(ಶೂನ್ಯ);
  ಶೂನ್ಯ sqlite3_ಕೊನೆ_Benign_Malloc(ಶೂನ್ಯ);
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3BeginBenignMalloc()
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಕೊನೆ_Benign_Malloc()
#ಆದರೆ_ಅಂತ್ಯ

/*
** Allowed return values from sqlite3FindInIndex()
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಒಳ_ಸೂಚ್ಯಂಕ_ROWID        1   /* Search the rowid of the table */
#ವ್ಯಾಖ್ಯಾನಿಸು ಒಳ_ಸೂಚ್ಯಂಕ_EPH          2   /* Search an ephemeral b-tree */
#ವ್ಯಾಖ್ಯಾನಿಸು ಒಳ_ಸೂಚ್ಯಂಕ_ಸೂಚ್ಯಂಕ_ASC    3   /* Existing index ASCENDING */
#ವ್ಯಾಖ್ಯಾನಿಸು ಒಳ_ಸೂಚ್ಯಂಕ_ಸೂಚ್ಯಂಕ_DESC   4   /* Existing index DESCENDING */
#ವ್ಯಾಖ್ಯಾನಿಸು ಒಳ_ಸೂಚ್ಯಂಕ_NOOP         5   /* No table available. Use comparisons */
/*
** Allowed flags for the 3rd parameter to sqlite3FindInIndex().
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಒಳ_ಸೂಚ್ಯಂಕ_NOOP_OK     0x0001  /* OK to return IN_INDEX_NOOP */
#ವ್ಯಾಖ್ಯಾನಿಸು ಒಳ_ಸೂಚ್ಯಂಕ_MEMBERSHIP  0x0002  /* IN operator used for membership test */
#ವ್ಯಾಖ್ಯಾನಿಸು ಒಳ_ಸೂಚ್ಯಂಕ_LOOP        0x0004  /* IN operator used as a loop */
ಪೂರ್ಣಾಂಕ sqlite3_ಕಂಡುಹಿಡಿ_ಒಳ_ಸೂಚ್ಯಂಕ(Parse *, ಅಭಿವ್ಯಕ್ತಿ *, u32, ಪೂರ್ಣಾಂಕ*, ಪೂರ್ಣಾಂಕ*, ಪೂರ್ಣಾಂಕ*);

ಪೂರ್ಣಾಂಕ sqlite3JournalOpen(sqlite3_vfs *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, sqlite3_file *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_Journal_ಗಾತ್ರ(sqlite3_vfs *);
#ಆದರೆ defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
  ಪೂರ್ಣಾಂಕ sqlite3JournalCreate(sqlite3_file *);
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ sqlite3_Journal_Is_ಒಳ_ಸ್ಮರಣೆ(sqlite3_file *p);
ಶೂನ್ಯ sqlite3MemJournalOpen(sqlite3_file *);

ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_ಎತ್ತರ_And_Flags(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p);
#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
  ಪೂರ್ಣಾಂಕ sqlite3_Select_ಅಭಿವ್ಯಕ್ತಿ_ಎತ್ತರ(ಸ್ಥಿರಾಂಕ Select *);
  ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_ಎತ್ತರ(Parse*, ಪೂರ್ಣಾಂಕ);
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_Select_ಅಭಿವ್ಯಕ್ತಿ_ಎತ್ತರ(x) 0
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_ಎತ್ತರ(x,y)
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Error_ಸ್ಥಾನಾಂತರ(ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ);

u32 sqlite3Get4byte(ಸ್ಥಿರಾಂಕ u8*);
SQLITE_OPT_INLINE u64 sqlite3Get8byte(ಸ್ಥಿರಾಂಕ u8*);
#ಆದರೆ SQLITE_BYTEORDER!=4321
SQLITE_OPT_INLINE u64 sqlite3BSwap64(u64);
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3Put4byte(u8*, u32);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UNLOCK_NOTIFY
  ಶೂನ್ಯ sqlite3ConnectionBlocked(sqlite3 *, sqlite3 *);
  ಶೂನ್ಯ sqlite3ConnectionUnlocked(sqlite3 *db);
  ಶೂನ್ಯ sqlite3ConnectionClosed(sqlite3 *db);
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3ConnectionBlocked(x,y)
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3ConnectionUnlocked(x)
  #ವ್ಯಾಖ್ಯಾನಿಸು sqlite3ConnectionClosed(x)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಶೂನ್ಯ sqlite3ParserTrace(FILE*, ಅಕ್ಷರ *);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(YYCOVERAGE)
  ಪೂರ್ಣಾಂಕ sqlite3ParserCoverage(FILE*);
#ಆದರೆ_ಅಂತ್ಯ

/*
** If the SQLITE_ENABLE IOTRACE exists then the global variable
** sqlite3IoTrace is a pointer to a printf-like routine used to
** print I/O tracing messages.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_IOTRACE
# define IOTRACE(A)  ಆದರೆ( sqlite3IoTrace ){ sqlite3IoTrace A; }
  ಶೂನ್ಯ sqlite3VdbeIOTraceSql(Vdbe*);
SQLITE_API SQLITE_EXTERN ಶೂನ್ಯ (SQLITE_CDECL *sqlite3IoTrace)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,...);
#ಇಲ್ಲದಿದ್ದರೆ
# define IOTRACE(A)
# define sqlite3VdbeIOTraceSql(X)
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MEMDEBUG
  ಶೂನ್ಯ sqlite3_Memdebug_ಗಣ_Type(ಶೂನ್ಯ*,u8);
  ಪೂರ್ಣಾಂಕ sqlite3MemdebugHasType(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,u8);
  ಪೂರ್ಣಾಂಕ sqlite3MemdebugNoType(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,u8);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_Memdebug_ಗಣ_Type(X,Y)  /* no-op */
# define sqlite3MemdebugHasType(X,Y)  1
# define sqlite3MemdebugNoType(X,Y)   1
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು MEMTYPE_ರಾಶಿ       0x01  /* General heap allocations */
#ವ್ಯಾಖ್ಯಾನಿಸು MEMTYPE_LOOKASIDE  0x02  /* Heap that might have been lookaside */
#ವ್ಯಾಖ್ಯಾನಿಸು MEMTYPE_PCACHE     0x04  /* Page cache allocations */

/*
** Threading interface
*/
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
ಪೂರ್ಣಾಂಕ sqlite3ThreadCreate(SQLiteThread**,ಶೂನ್ಯ*(*)(ಶೂನ್ಯ*),ಶೂನ್ಯ*);
ಪೂರ್ಣಾಂಕ sqlite3ThreadJoin(SQLiteThread*, ಶೂನ್ಯ**);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)
ಪೂರ್ಣಾಂಕ sqlite3_Dbpage_ರಿಜಿಸ್ಟರ್(sqlite3*);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)
ಪೂರ್ಣಾಂಕ sqlite3_Dbstat_ರಿಜಿಸ್ಟರ್(sqlite3*);
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ);
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಸದಿಶ(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಸದಿಶ_ಕ್ಷೇತ್ರ_Subexpr(ಅಭಿವ್ಯಕ್ತಿ*, ಪೂರ್ಣಾಂಕ);
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_ಅಭಿವ್ಯಕ್ತಿ_For_ಸದಿಶ_ಕ್ಷೇತ್ರ(Parse*,ಅಭಿವ್ಯಕ್ತಿ*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_ಸದಿಶ_Error_ಸಂದೇಶ(Parse*, ಅಭಿವ್ಯಕ್ತಿ*);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_COMPILEOPTION_DIAGS
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **sqlite3CompileOptions(ಪೂರ್ಣಾಂಕ *pnOpt);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
ಪೂರ್ಣಾಂಕ sqlite3KvvfsInit(ಶೂನ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(VDBE_PROFILE) \
 || defined(SQLITE_PERFORMANCE_TRACE) \
 || defined(SQLITE_ENABLE_STMT_SCANSTATUS)
sqlite3_uint64 sqlite3Hwtime(ಶೂನ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
# define IS_STMT_SCANSTATUS(db) (db->flags & SQLITE_Stmt_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ)
#ಇಲ್ಲದಿದ್ದರೆ
# define IS_STMT_SCANSTATUS(db) 0
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* SQLITEINT_H */
