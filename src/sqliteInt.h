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
#अगर_अघोषित SQLITEINT_H
#घोषणा SQLITEINT_H

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
#अगर_अघोषित SQLITE_TCLAPI
#  define SQLITE_TCLAPI
#पूर्ण_अगर

/*
** Include the header file used to customize the compiler options for MSVC.
** This should be done first so that it can successfully prevent spurious
** compiler warnings due to subsequent content in this file and other files
** that are included by this file.
*/
#समावेश "msvc.h"

/*
** Special setup for VxWorks
*/
#समावेश "vxworks.h"

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
#अगर_अघोषित SQLITE_DISABLE_LFS
# define _LARGE_FILE       1
# ifndef _FILE_OFFSET_BITS
#   define _FILE_OFFSET_BITS 64
# endif
# define _LARGEFILE_SOURCE 1
#पूर्ण_अगर

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
#अगर defined(__GNUC__) && !defined(SQLITE_DISABLE_INTRINSIC)
# define GCC_VERSION (__GNUC__*1000000+__GNUC_MINOR__*1000+__GNUC_PATCHLEVEL__)
#अन्यथा
# define GCC_VERSION 0
#पूर्ण_अगर
#अगर defined(_MSC_VER) && !defined(SQLITE_DISABLE_INTRINSIC)
# define MSVC_VERSION _MSC_VER
#अन्यथा
# define MSVC_VERSION 0
#पूर्ण_अगर

/*
** Some C99 functions in "math.h" are only present for MSVC when its version
** is associated with Visual Studio 2013 or higher.
*/
#अगर_अघोषित SQLITE_HAVE_C99_MATH_FUNCS
# अगर MSVC_VERSION==0 || MSVC_VERSION>=1800
#  define SQLITE_HAVE_C99_MATH_FUNCS (1)
# अन्यथा
#  define SQLITE_HAVE_C99_MATH_FUNCS (0)
# endif
#पूर्ण_अगर

/* Needed for various definitions... */
#अगर defined(__GNUC__) && !defined(_GNU_SOURCE)
# define _GNU_SOURCE
#पूर्ण_अगर

#अगर defined(__OpenBSD__) && !defined(_BSD_SOURCE)
# define _BSD_SOURCE
#पूर्ण_अगर

/*
** Macro to disable warnings about missing "break" at the end of a "case".
*/
#अगर defined(__has_attribute)
#  अगर __has_attribute(fallthrough)
#    define deliberate_fall_through __attribute__((fallthrough));
#  endif
#पूर्ण_अगर
#अगर !defined(deliberate_fall_through)
#  define deliberate_fall_through
#पूर्ण_अगर

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
#अगर defined(_HAVE_MINGW_H)
# include "mingw.h"
#या_अगर defined(_HAVE__MINGW_H)
# include "_mingw.h"
#पूर्ण_अगर

/*
** For MinGW version 4.x (and higher), check to see if the _USE_32BIT_TIME_T
** define is required to maintain binary compatibility with the MSVC runtime
** library in use (e.g. for Windows XP).
*/
#अगर !defined(_USE_32BIT_TIME_T) && !defined(_USE_64BIT_TIME_T) && \
    defined(_WIN32) && !defined(_WIN64) && \
    defined(__MINGW_MAJOR_VERSION) && __MINGW_MAJOR_VERSION >= 4 && \
    defined(__MSVCRT__)
# define _USE_32BIT_TIME_T
#पूर्ण_अगर

/* Optionally #include a user-defined header, whereby compilation options
** may be set prior to where they take effect, but after platform setup.
** If SQLITE_CUSTOM_INCLUDE=? is defined, its value names the #include
** file.
*/
#अगर_घोषित SQLITE_CUSTOM_INCLUDE
# define INC_STRINGIFY_(f) #f
# define INC_STRINGIFY(f) INC_STRINGIFY_(f)
# include INC_STRINGIFY(SQLITE_CUSTOM_INCLUDE)
#पूर्ण_अगर

/* The public SQLite interface.  The _FILE_OFFSET_BITS macro must appear
** first in QNX.  Also, the _USE_32BIT_TIME_T macro must appear first for
** MinGW.
*/
#समावेश "sqlite3.h"

/*
** Reuse the STATIC_VFS1 for mutex access to sqlite3_temp_directory.
*/
#घोषणा SQLITE_MUTEX_STATIC_TEMPDIR SQLITE_MUTEX_STATIC_VFS1

/*
** Include the configuration header output by 'configure' if we're using the
** autoconf-based build
*/
#अगर defined(_HAVE_SQLITE_CONFIG_H) && !defined(SQLITECONFIG_H)
#समावेश "sqlite_cfg.h"
#घोषणा SQLITECONFIG_H 1
#पूर्ण_अगर

#समावेश "sqliteLimit.h"

/* Disable nuisance warnings on Borland compilers */
#अगर defined(__BORLANDC__)
#आशय warn -rch /* unreachable code */
#आशय warn -ccc /* Condition is always true or false */
#आशय warn -aus /* Assigned value is never used */
#आशय warn -csu /* Comparing signed and unsigned */
#आशय warn -spa /* Suspicious pointer arithmetic */
#पूर्ण_अगर

/*
** A few places in the code require atomic load/store of aligned
** integer values.
*/
#अगर_अघोषित __has_extension
# define __has_extension(x) 0     /* compatibility with non-clang compilers */
#पूर्ण_अगर
#अगर GCC_VERSION>=4007000 || __has_extension(c_atomic)
# define SQLITE_ATOMIC_INTRINSICS 1
# define AtomicLoad(PTR)       __atomic_load_n((PTR),__ATOMIC_RELAXED)
# define AtomicStore(PTR,VAL)  __atomic_store_n((PTR),(VAL),__ATOMIC_RELAXED)
#अन्यथा
# define SQLITE_ATOMIC_INTRINSICS 0
# define AtomicLoad(PTR)       (*(PTR))
# define AtomicStore(PTR,VAL)  (*(PTR) = (VAL))
#पूर्ण_अगर

/*
** Include standard header files as necessary
*/
#समावेश <stdint.h>
#अगर_घोषित HAVE_INTTYPES_H
#समावेश <inttypes.h>
#पूर्ण_अगर

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
#अगर defined(HAVE_STDINT_H)   /* Use this case if we have ANSI headers */
# define SQLITE_INT_TO_PTR(X)  ((व्योम*)(intptr_t)(X))
# define SQLITE_PTR_TO_INT(X)  ((पूर्णांक)(intptr_t)(X))
#या_अगर defined(__PTRDIFF_TYPE__)  /* This case should work for GCC */
# define SQLITE_INT_TO_PTR(X)  ((व्योम*)(__PTRDIFF_TYPE__)(X))
# define SQLITE_PTR_TO_INT(X)  ((पूर्णांक)(__PTRDIFF_TYPE__)(X))
#या_अगर !defined(__GNUC__)       /* Works for compilers other than LLVM */
# define SQLITE_INT_TO_PTR(X)  ((व्योम*)&((अक्षर*)0)[X])
# define SQLITE_PTR_TO_INT(X)  ((पूर्णांक)(((अक्षर*)X)-(अक्षर*)0))
#अन्यथा                          /* Generates a warning - but it always works */
# define SQLITE_INT_TO_PTR(X)  ((व्योम*)(X))
# define SQLITE_PTR_TO_INT(X)  ((पूर्णांक)(X))
#पूर्ण_अगर

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
#अगर defined(SQLITE_DISABLE_INLINE)
#  define SQLITE_NOINLINE
#  define SQLITE_INLINE
#  define SQLITE_OPT_INLINE
#  define SQLITE_USES_INLINE 0
#या_अगर defined(__GNUC__)
#  define SQLITE_NOINLINE    __attribute__((noinline))
#  define SQLITE_INLINE      __attribute__((always_inline)) अंतरभूत
#  define SQLITE_OPT_INLINE  __attribute__((always_inline)) अंतरभूत
#  define SQLITE_USES_INLINE 1
#या_अगर defined(_MSC_VER) && _MSC_VER>=1310
#  define SQLITE_NOINLINE    __declspec(noinline)
#  define SQLITE_INLINE      __forceinline
#  define SQLITE_OPT_INLINE  __forceinline
#  define SQLITE_USES_INLINE 1
#अन्यथा
#  define SQLITE_NOINLINE
#  define SQLITE_INLINE
#  define SQLITE_OPT_INLINE
#  define SQLITE_USES_INLINE 0
#पूर्ण_अगर
#अगर defined(SQLITE_COVERAGE_TEST) || defined(__STRICT_ANSI__)
# undef SQLITE_INLINE
# define SQLITE_INLINE
# undef SQLITE_OPT_INLINE
# define SQLITE_OPT_INLINE
# undef SQLITE_USES_INLINE
# define SQLITE_USES_INLINE 0
#पूर्ण_अगर
#अगर !defined(SQLITE_AMALGAMATION)
# undef SQLITE_OPT_INLINE
# define SQLITE_OPT_INLINE
#पूर्ण_अगर

/*
** Make sure that the compiler intrinsics we desire are enabled when
** compiling with an appropriate version of MSVC unless prevented by
** the SQLITE_DISABLE_INTRINSIC define.
*/
#अगर !defined(SQLITE_DISABLE_INTRINSIC)
#  अगर defined(_MSC_VER) && _MSC_VER>=1400
#    अगर !defined(_WIN32_WCE)
#      include <intrin.h>
#      pragma intrinsic(_byteswap_ushort)
#      pragma intrinsic(_byteswap_ulong)
#      pragma intrinsic(_byteswap_uint64)
#      pragma intrinsic(_ReadWriteBarrier)
#    अन्यथा
#      include <cmnintrin.h>
#    endif
#  endif
#पूर्ण_अगर

/*
** Enable SQLITE_USE_SEH by default on MSVC builds.  Only omit
** SEH support if the -DSQLITE_OMIT_SEH option is given.
*/
#अगर defined(_MSC_VER) && !defined(SQLITE_OMIT_SEH)
# define SQLITE_USE_SEH 1
#अन्यथा
# undef SQLITE_USE_SEH
#पूर्ण_अगर

/*
** Enable SQLITE_DIRECT_OVERFLOW_READ, unless the build explicitly
** disables it using -DSQLITE_DIRECT_OVERFLOW_READ=0
*/
#अगर defined(SQLITE_DIRECT_OVERFLOW_READ) && SQLITE_DIRECT_OVERFLOW_READ+1==1
  /* Disable if -DSQLITE_DIRECT_OVERFLOW_READ=0 */
# undef SQLITE_DIRECT_OVERFLOW_READ
#अन्यथा
  /* In all other cases, enable */
# define SQLITE_DIRECT_OVERFLOW_READ 1
#पूर्ण_अगर


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
#अगर !defined(SQLITE_THREADSAFE)
# अगर defined(THREADSAFE)
#   define SQLITE_THREADSAFE THREADSAFE
# अन्यथा
#   define SQLITE_THREADSAFE 1 /* IMP: R-07272-22309 */
# endif
#पूर्ण_अगर

/*
** Powersafe overwrite is on by default.  But can be turned off using
** the -DSQLITE_POWERSAFE_OVERWRITE=0 command-line option.
*/
#अगर_अघोषित SQLITE_POWERSAFE_OVERWRITE
# define SQLITE_POWERSAFE_OVERWRITE 1
#पूर्ण_अगर

/*
** EVIDENCE-OF: R-25715-37072 Memory allocation statistics are enabled by
** default unless SQLite is compiled with SQLITE_DEFAULT_MEMSTATUS=0 in
** which case memory allocation statistics are disabled by default.
*/
#अगर !defined(SQLITE_DEFAULT_MEMSTATUS)
# define SQLITE_DEFAULT_MEMSTATUS 1
#पूर्ण_अगर

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
#अगर defined(SQLITE_SYSTEM_MALLOC) \
  + defined(SQLITE_WIN32_MALLOC) \
  + defined(SQLITE_ZERO_MALLOC) \
  + defined(SQLITE_MEMDEBUG)>1
# error "Two or more of the following compile-समय configuration options\
 are defined but at most one is allowed:\
 SQLITE_SYSTEM_MALLOC, SQLITE_WIN32_MALLOC, SQLITE_MEMDEBUG,\
 SQLITE_ZERO_MALLOC"
#पूर्ण_अगर
#अगर defined(SQLITE_SYSTEM_MALLOC) \
  + defined(SQLITE_WIN32_MALLOC) \
  + defined(SQLITE_ZERO_MALLOC) \
  + defined(SQLITE_MEMDEBUG)==0
# define SQLITE_SYSTEM_MALLOC 1
#पूर्ण_अगर

/*
** If SQLITE_MALLOC_SOFT_LIMIT is not zero, then try to keep the
** sizes of memory allocations below this value where possible.
*/
#अगर !defined(SQLITE_MALLOC_SOFT_LIMIT)
# define SQLITE_MALLOC_SOFT_LIMIT 1024
#पूर्ण_अगर

/*
** We need to define _XOPEN_SOURCE as follows in order to enable
** recursive mutexes on most Unix systems and fchmod() on OpenBSD.
** But _XOPEN_SOURCE define causes problems for Mac OS X, so omit
** it.
*/
#अगर !defined(_XOPEN_SOURCE) && !defined(__DARWIN__) && !defined(__APPLE__)
#  define _XOPEN_SOURCE 600
#पूर्ण_अगर

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
#अगर !defined(न_संशोधन) && !defined(SQLITE_DEBUG)
# define न_संशोधन 1
#पूर्ण_अगर
#अगर defined(न_संशोधन) && defined(SQLITE_DEBUG)
# undef न_संशोधन
#पूर्ण_अगर

/*
** Enable SQLITE_ENABLE_EXPLAIN_COMMENTS if SQLITE_DEBUG is turned on.
*/
#अगर !defined(SQLITE_ENABLE_EXPLAIN_COMMENTS) && defined(SQLITE_DEBUG)
# define SQLITE_ENABLE_EXPLAIN_COMMENTS 1
#पूर्ण_अगर

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
#अगर defined(SQLITE_MUTATION_TEST)
# define testcase(X)
#या_अगर defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
# ifndef SQLITE_AMALGAMATION
    बाह्य अचिन्हित पूर्णांक sqlite3_Coverage_गणक;
# endif
# define testcase(X)  अगर( X ){ sqlite3_Coverage_गणक += (अचिन्हित)__LINE__; }
#अन्यथा
# define testcase(X)
#पूर्ण_अगर

/*
** The TESTONLY macro is used to enclose variable declarations or
** other bits of code that are needed to support the arguments
** within testcase() and assert() macros.
*/
#अगर !defined(न_संशोधन) || defined(SQLITE_COVERAGE_TEST)
# define TESTONLY(X)  X
#अन्यथा
# define TESTONLY(X)
#पूर्ण_अगर

/*
** Sometimes we need a small amount of code such as a variable initialization
** to setup for a later assert() statement.  We do not want this code to
** appear when assert() is disabled.  The following macro is therefore
** used to contain that setup code.  The "VVA" acronym stands for
** "Verification, Validation, and Accreditation".  In other words, the
** code within VVA_ONLY() will only run during verification processes.
*/
#अगर_अघोषित न_संशोधन
# define VVA_ONLY(X)  X
#अन्यथा
# define VVA_ONLY(X)
#पूर्ण_अगर

/*
** Disable ALWAYS() and NEVER() (make them pass-throughs) for coverage
** and mutation testing
*/
#अगर defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_MUTATION_TEST)
# define SQLITE_OMIT_AUXILIARY_SAFETY_CHECKS  1
#पूर्ण_अगर

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
#अगर defined(SQLITE_OMIT_AUXILIARY_SAFETY_CHECKS)
# define ALWAYS(X)      (1)
# define NEVER(X)       (0)
#या_अगर !defined(न_संशोधन)
# define ALWAYS(X)      ((X)?1:(निश्चित(0),0))
# define NEVER(X)       ((X)?(निश्चित(0),1):0)
#अन्यथा
# define ALWAYS(X)      (X)
# define NEVER(X)       (X)
#पूर्ण_अगर

/*
** Some conditionals are optimizations only.  In other words, if the
** conditionals are replaced with a constant 1 (true) or 0 (false) then
** the correct answer is still obtained, though perhaps not as quickly.
**
** The following macros mark these optimizations conditionals.
*/
#अगर defined(SQLITE_MUTATION_TEST)
# define OK_IF_ALWAYS_TRUE(X)  (1)
# define OK_IF_ALWAYS_FALSE(X) (0)
#अन्यथा
# define OK_IF_ALWAYS_TRUE(X)  (X)
# define OK_IF_ALWAYS_FALSE(X) (X)
#पूर्ण_अगर

/*
** Some malloc failures are only possible if SQLITE_TEST_REALLOC_STRESS is
** defined.  We need to defend against those failures when testing with
** SQLITE_TEST_REALLOC_STRESS, but we don't want the unreachable branches
** during a normal build.  The following macro can be used to disable tests
** that are always false except when SQLITE_TEST_REALLOC_STRESS is set.
*/
#अगर defined(SQLITE_TEST_REALLOC_STRESS)
# define ONLY_IF_REALLOC_STRESS(X)  (X)
#या_अगर !defined(न_संशोधन)
# define ONLY_IF_REALLOC_STRESS(X)  ((X)?(निश्चित(0),1):0)
#अन्यथा
# define ONLY_IF_REALLOC_STRESS(X)  (0)
#पूर्ण_अगर

/*
** Declarations used for tracing the operating system interfaces.
*/
#अगर defined(SQLITE_FORCE_OS_TRACE) || defined(SQLITE_TEST) || \
    (defined(SQLITE_DEBUG) && SQLITE_OS_WIN)
  बाह्य पूर्णांक sqlite3OSTrace;
# define OSTRACE(X)          अगर( sqlite3OSTrace ) sqlite3DebugPrintf X
# define SQLITE_HAVE_OS_TRACE
#अन्यथा
# define OSTRACE(X)
# undef  SQLITE_HAVE_OS_TRACE
#पूर्ण_अगर

/*
** Is the sqlite3ErrName() function needed in the build?  Currently,
** it is needed by "mutex_w32.c" (when debugging), "os_win.c" (when
** OSTRACE is enabled), and by several "test*.c" files (which are
** compiled using SQLITE_TEST).
*/
#अगर defined(SQLITE_HAVE_OS_TRACE) || defined(SQLITE_TEST) || \
    (defined(SQLITE_DEBUG) && SQLITE_OS_WIN)
# define SQLITE_NEED_ERR_NAME
#अन्यथा
# undef  SQLITE_NEED_ERR_NAME
#पूर्ण_अगर

/*
** SQLITE_ENABLE_EXPLAIN_COMMENTS is incompatible with SQLITE_OMIT_EXPLAIN
*/
#अगर_घोषित SQLITE_OMIT_EXPLAIN
# undef SQLITE_ENABLE_EXPLAIN_COMMENTS
#पूर्ण_अगर

/*
** SQLITE_OMIT_VIRTUALTABLE implies SQLITE_OMIT_ALTERTABLE
*/
#अगर defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_ALTERTABLE)
# define SQLITE_OMIT_ALTERTABLE
#पूर्ण_अगर

#घोषणा SQLITE_DIGIT_SEPARATOR '_'

/*
** Return true (non-zero) if the input is an integer that is too large
** to fit in 32-bits.  This macro is used inside of various testcase()
** macros to verify that we have tested SQLite for large-file support.
*/
#घोषणा IS_BIG_INT(X)  (((X)&~(i64)0xffffffff)!=0)

/*
** The macro unlikely() is a hint that surrounds a boolean
** expression that is usually false.  Macro likely() surrounds
** a boolean expression that is usually true.  These hints could,
** in theory, be used by the compiler to generate better code, but
** currently they are just comments for human readers.
*/
#घोषणा likely(X)    (X)
#घोषणा unlikely(X)  (X)

#समावेश "hash.h"
#समावेश "parse.h"
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <assert.h>
#समावेश <stddef.h>
#समावेश <ctype.h>

/*
** Use a macro to replace memcpy() if compiled with SQLITE_INLINE_MEMCPY.
** This allows better measurements of where memcpy() is used when running
** cachegrind.  But this macro version of memcpy() is very slow so it
** should not be used in production.  This is a performance measurement
** hack only.
*/
#अगर_घोषित SQLITE_INLINE_MEMCPY
# define स_नकल(D,S,N) {अक्षर*xxd=(अक्षर*)(D);स्थिर अक्षर*xxs=(स्थिर अक्षर*)(S);\
                        पूर्णांक xxn=(N);जबतक(xxn-->0)*(xxd++)=*(xxs++);}
#पूर्ण_अगर

/*
** If compiling for a processor that lacks floating point support,
** substitute integer for floating-point
*/
#अगर_घोषित SQLITE_OMIT_FLOATING_POINT
# define द्विगुन sqlite_int64
# define भग्न sqlite_int64
# define भ_असल(X) ((X)<0?-(X):(X))
# define sqlite3IsOverflow(X) 0
# define अनन्त (9223372036854775807LL)
# ifndef SQLITE_BIG_DBL
#   define SQLITE_BIG_DBL (((sqlite3_int64)1)<<50)
# endif
# define SQLITE_OMIT_DATETIME_FUNCS 1
# define SQLITE_OMIT_TRACE 1
# undef SQLITE_MIXED_ENDIAN_64BIT_FLOAT
# undef SQLITE_HAVE_ISNAN
#पूर्ण_अगर
#अगर_अघोषित SQLITE_BIG_DBL
# define SQLITE_BIG_DBL (1e99)
#पूर्ण_अगर

/*
** OMIT_TEMPDB is set to 1 if SQLITE_OMIT_TEMPDB is defined, or 0
** afterward. Having this macro allows us to cause the C compiler
** to omit code used by TEMP tables without messy #ifndef statements.
*/
#अगर_घोषित SQLITE_OMIT_TEMPDB
#घोषणा OMIT_TEMPDB 1
#अन्यथा
#घोषणा OMIT_TEMPDB 0
#पूर्ण_अगर

/*
** The "file format" number is an integer that is incremented whenever
** the VDBE-level file format changes.  The following macros define the
** the default file format for new databases and the maximum file format
** that the library can read.
*/
#घोषणा SQLITE_MAX_FILE_FORMAT 4
#अगर_अघोषित SQLITE_DEFAULT_FILE_FORMAT
# define SQLITE_DEFAULT_FILE_FORMAT 4
#पूर्ण_अगर

/*
** Determine whether triggers are recursive by default.  This can be
** changed at run-time using a pragma.
*/
#अगर_अघोषित SQLITE_DEFAULT_RECURSIVE_TRIGGERS
# define SQLITE_DEFAULT_RECURSIVE_TRIGGERS 0
#पूर्ण_अगर

/*
** Provide a default value for SQLITE_TEMP_STORE in case it is not specified
** on the command-line
*/
#अगर_अघोषित SQLITE_अस्थायी_STORE
# define SQLITE_अस्थायी_STORE 1
#पूर्ण_अगर

/*
** If no value has been provided for SQLITE_MAX_WORKER_THREADS, or if
** SQLITE_TEMP_STORE is set to 3 (never use temporary files), set it
** to zero.
*/
#अगर SQLITE_अस्थायी_STORE==3 || SQLITE_THREADSAFE==0
# undef SQLITE_MAX_WORKER_THREADS
# define SQLITE_MAX_WORKER_THREADS 0
#पूर्ण_अगर
#अगर_अघोषित SQLITE_MAX_WORKER_THREADS
# define SQLITE_MAX_WORKER_THREADS 8
#पूर्ण_अगर
#अगर_अघोषित SQLITE_DEFAULT_WORKER_THREADS
# define SQLITE_DEFAULT_WORKER_THREADS 0
#पूर्ण_अगर
#अगर SQLITE_DEFAULT_WORKER_THREADS>SQLITE_MAX_WORKER_THREADS
# undef SQLITE_MAX_WORKER_THREADS
# define SQLITE_MAX_WORKER_THREADS SQLITE_DEFAULT_WORKER_THREADS
#पूर्ण_अगर

/*
** The default initial allocation for the pagecache when using separate
** pagecaches for each database connection.  A positive number is the
** number of pages.  A negative number N translations means that a buffer
** of -1024*N bytes is allocated and used for as many pages as it will hold.
**
** The default value of "20" was chosen to minimize the run-time of the
** speedtest1 test program with options: --shrink-memory --reprepare
*/
#अगर_अघोषित SQLITE_DEFAULT_PCACHE_INITSZ
# define SQLITE_DEFAULT_PCACHE_INITSZ 20
#पूर्ण_अगर

/*
** Default value for the SQLITE_CONFIG_SORTERREF_SIZE option.
*/
#अगर_अघोषित SQLITE_DEFAULT_SORTERREF_आकार
# define SQLITE_DEFAULT_SORTERREF_आकार 0x7fffffff
#पूर्ण_अगर

/*
** The compile-time options SQLITE_MMAP_READWRITE and
** SQLITE_ENABLE_BATCH_ATOMIC_WRITE are not compatible with one another.
** You must choose one or the other (or neither) but not both.
*/
#अगर defined(SQLITE_MMAP_READWRITE) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
#त्रुटि Cannot use both SQLITE_MMAP_READWRITE and SQLITE_ENABLE_BATCH_ATOMIC_WRITE
#पूर्ण_अगर

/*
** GCC does not define the offsetof() macro so we'll have to do it
** ourselves.
*/
#अगर_अघोषित दुरत्व
# define दुरत्व(ST,M) ((माप_प्रकार)((अक्षर*)&((ST*)0)->M - (अक्षर*)0))
#पूर्ण_अगर

/*
** sizeof64() is like sizeof(), but always returns a 64-bit value, even
** on 32-bit builds. This can help to avoid overflow by ensuring 64-bit
** arithmetic is used consistently in both 32-bit and 64-bit builds.
*/
#घोषणा sizeof64(X) ((sqlite3_int64)माप(X))

/*
** Work around C99 "flex-array" syntax for pre-C99 compilers, so as
** to avoid complaints from -fsanitize=strict-bounds.
*/
#अगर defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define FLEXARRAY
#अन्यथा
# define FLEXARRAY 1
#पूर्ण_अगर

/*
** Macros to compute minimum and maximum of two numbers.
*/
#अगर_अघोषित MIN
# define MIN(A,B) ((A)<(B)?(A):(B))
#पूर्ण_अगर
#अगर_अघोषित MAX
# define MAX(A,B) ((A)>(B)?(A):(B))
#पूर्ण_अगर

/*
** Swap two objects of type TYPE.
*/
#घोषणा SWAP(TYPE,A,B) {TYPE t=A; A=B; B=t;}

/*
** Check to see if this machine uses EBCDIC.  (Yes, believe it or
** not, there are still machines out there that use EBCDIC.)
*/
#अगर 'A' == '\301'
# define SQLITE_EBCDIC 1
#अन्यथा
# define SQLITE_ASCII 1
#पूर्ण_अगर

/*
** Integers of known sizes.  These typedefs might change for architectures
** where the sizes very.  Preprocessor macros are available so that the
** types can be conveniently redefined at compile-type.  Like this:
**
**         cc '-DUINTPTR_TYPE=long long int' ...
*/
#अगर_अघोषित UINT32_TYPE
# ifdef HAVE_UINT32_T
#  define UINT32_TYPE uint32_t
# अन्यथा
#  define UINT32_TYPE अचिन्हित पूर्णांक
# endif
#पूर्ण_अगर
#अगर_अघोषित UINT16_TYPE
# ifdef HAVE_UINT16_T
#  define UINT16_TYPE uint16_t
# अन्यथा
#  define UINT16_TYPE अचिन्हित लघु पूर्णांक
# endif
#पूर्ण_अगर
#अगर_अघोषित INT16_TYPE
# ifdef HAVE_INT16_T
#  define INT16_TYPE int16_t
# अन्यथा
#  define INT16_TYPE लघु पूर्णांक
# endif
#पूर्ण_अगर
#अगर_अघोषित UINT8_TYPE
# ifdef HAVE_UINT8_T
#  define UINT8_TYPE uint8_t
# अन्यथा
#  define UINT8_TYPE अचिन्हित अक्षर
# endif
#पूर्ण_अगर
#अगर_अघोषित INT8_TYPE
# ifdef HAVE_INT8_T
#  define INT8_TYPE int8_t
# अन्यथा
#  define INT8_TYPE चिन्हित अक्षर
# endif
#पूर्ण_अगर
प्रकार sqlite_int64 i64;          /* 8-byte signed integer */
प्रकार sqlite_uint64 u64;         /* 8-byte unsigned integer */
प्रकार UINT32_TYPE u32;           /* 4-byte unsigned integer */
प्रकार UINT16_TYPE u16;           /* 2-byte unsigned integer */
प्रकार INT16_TYPE i16;            /* 2-byte signed integer */
प्रकार UINT8_TYPE u8;             /* 1-byte unsigned integer */
प्रकार INT8_TYPE i8;              /* 1-byte signed integer */

/* A bitfield type for use inside of structures.  Always follow with :N where
** N is the number of bits.
*/
प्रकार अचिन्हित bft;  /* Bit Field Type */

/*
** SQLITE_MAX_U32 is a u64 constant that is the maximum u64 value
** that can be stored in a u32 without loss of data.  The value
** is 0x00000000ffffffff.  But because of quirks of some compilers, we
** have to specify the value in the less intuitive manner shown:
*/
#घोषणा SQLITE_MAX_U32  ((((u64)1)<<32)-1)

/*
** The datatype used to store estimates of the number of rows in a
** table or index.
*/
प्रकार u64 tRowcnt;

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
प्रकार INT16_TYPE LogEst;
#घोषणा LOGEST_MIN (-32768)
#घोषणा LOGEST_MAX (32767)

/*
** Set the SQLITE_PTRSIZE macro to the number of bytes in a pointer
*/
#अगर_अघोषित SQLITE_PTRSIZE
# अगर defined(__SIZEOF_POINTER__)
#   define SQLITE_PTRSIZE __SIZEOF_POINTER__
# elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
       defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
      (defined(__APPLE__) && defined(__ppc__)) ||                         \
      (defined(__TOS_AIX__) && !defined(__64BIT__))
#   define SQLITE_PTRSIZE 4
# अन्यथा
#   define SQLITE_PTRSIZE 8
# endif
#पूर्ण_अगर

/* The uptr type is an unsigned integer large enough to hold a pointer
*/
#अगर defined(HAVE_STDINT_H)
  प्रकार uintptr_t uptr;
#या_अगर SQLITE_PTRSIZE==4
  प्रकार u32 uptr;
#अन्यथा
  प्रकार u64 uptr;
#पूर्ण_अगर

/*
** The SQLITE_WITHIN(P,S,E) macro checks to see if pointer P points to
** something between S (inclusive) and E (exclusive).
**
** In other words, S is a buffer and E is a pointer to the first byte after
** the end of buffer S.  This macro returns true if P points to something
** contained within the buffer S.
*/
#घोषणा SQLITE_WITHIN(P,S,E)   (((uptr)(P)>=(uptr)(S))&&((uptr)(P)<(uptr)(E)))

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
#घोषणा SQLITE_OVERFLOW(P,S,E) (((uptr)(S)<(uptr)(P))&&((uptr)(E)>(uptr)(P)))

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
#अगर_अघोषित SQLITE_BYTEORDER  /* Replicate changes at tag-20230904a */
# अगर defined(__BYTE_ORDER__) && __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
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
# अन्यथा
#   define SQLITE_BYTEORDER 0
# endif
#पूर्ण_अगर
#अगर SQLITE_BYTEORDER==4321
# define SQLITE_BIGENDIAN    1
# define SQLITE_LITTLEENDIAN 0
# define SQLITE_UTF16NATIVE  SQLITE_UTF16BE
#या_अगर SQLITE_BYTEORDER==1234
# define SQLITE_BIGENDIAN    0
# define SQLITE_LITTLEENDIAN 1
# define SQLITE_UTF16NATIVE  SQLITE_UTF16LE
#अन्यथा
# ifdef SQLITE_AMALGAMATION
  स्थिर पूर्णांक sqlite3one = 1;
# अन्यथा
  बाह्य स्थिर पूर्णांक sqlite3one;
# endif
# define SQLITE_BIGENDIAN    (*(अक्षर *)(&sqlite3one)==0)
# define SQLITE_LITTLEENDIAN (*(अक्षर *)(&sqlite3one)==1)
# define SQLITE_UTF16NATIVE  (SQLITE_BIGENDIAN?SQLITE_UTF16BE:SQLITE_UTF16LE)
#पूर्ण_अगर

/*
** Constants for the largest and smallest possible 64-bit signed integers.
** These macros are designed to work correctly on both 32-bit and 64-bit
** compilers.
*/
#घोषणा LARGEST_INT64  (0xffffffff|(((i64)0x7fffffff)<<32))
#घोषणा LARGEST_UINT64 (0xffffffff|(((u64)0xffffffff)<<32))
#घोषणा SMALLEST_INT64 (((i64)-1) - LARGEST_INT64)

/*
** Macro SMXV(n) return the maximum value that can be held in variable n,
** assuming n is a signed integer type.  UMXV(n) is similar for unsigned
** integer types.
*/
#घोषणा SMXV(n) ((((i64)1)<<(माप(n)*8-1))-1)
#घोषणा UMXV(n) ((((i64)1)<<(माप(n)*8))-1)

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
#घोषणा ROUND8(x)     (((x)+7)&~7)
#अगर SQLITE_PTRSIZE==8
# define ROUND8P(x)   (x)
#अन्यथा
# define ROUND8P(x)   (((x)+7)&~7)
#पूर्ण_अगर

/*
** Round down to the nearest multiple of 8
*/
#घोषणा ROUNDDOWN8(x) ((x)&~7)

/*
** Assert that the pointer X is aligned to an 8-byte boundary.  This
** macro is used only within assert() to verify that the code gets
** all alignment restrictions correct.
**
** Except, if SQLITE_4_BYTE_ALIGNED_MALLOC is defined, then the
** underlying malloc() implementation might return us 4-byte aligned
** pointers.  In that case, only verify 4-byte alignment.
*/
#अगर_घोषित SQLITE_4_BYTE_ALIGNED_MALLOC
# define EIGHT_BYTE_ALIGNMENT(X)   ((((uptr)(X) - (uptr)0)&3)==0)
#अन्यथा
# define EIGHT_BYTE_ALIGNMENT(X)   ((((uptr)(X) - (uptr)0)&7)==0)
#पूर्ण_अगर
#घोषणा TWO_BYTE_ALIGNMENT(X)      ((((uptr)(X) - (uptr)0)&1)==0)

/*
** Disable MMAP on platforms where it is known to not work
*/
#अगर defined(__OpenBSD__) || defined(__QNXNTO__)
# undef SQLITE_MAX_MMAP_आकार
# define SQLITE_MAX_MMAP_आकार 0
#पूर्ण_अगर

/*
** Default maximum size of memory used by memory-mapped I/O in the VFS
*/
#अगर_घोषित __APPLE__
# include <TargetConditionals.h>
#पूर्ण_अगर
#अगर_अघोषित SQLITE_MAX_MMAP_आकार
# अगर defined(__linux__) \
  || defined(_WIN32) \
  || (defined(__APPLE__) && defined(__MACH__)) \
  || defined(__sun) \
  || defined(__FreeBSD__) \
  || defined(__DragonFly__)
#   define SQLITE_MAX_MMAP_आकार 0x7fff0000  /* 2147418112 */
# अन्यथा
#   define SQLITE_MAX_MMAP_आकार 0
# endif
#पूर्ण_अगर

/*
** The default MMAP_SIZE is zero on all platforms.  Or, even if a larger
** default MMAP_SIZE is specified at compile-time, make sure that it does
** not exceed the maximum mmap size.
*/
#अगर_अघोषित SQLITE_DEFAULT_MMAP_आकार
# define SQLITE_DEFAULT_MMAP_आकार 0
#पूर्ण_अगर
#अगर SQLITE_DEFAULT_MMAP_आकार>SQLITE_MAX_MMAP_आकार
# undef SQLITE_DEFAULT_MMAP_आकार
# define SQLITE_DEFAULT_MMAP_आकार SQLITE_MAX_MMAP_आकार
#पूर्ण_अगर

/*
** TREETRACE_ENABLED will be either 1 or 0 depending on whether or not
** the Abstract Syntax Tree tracing logic is turned on.
*/
#अगर !defined(SQLITE_AMALGAMATION)
बाह्य u32 sqlite3_वृक्ष_Trace;
#पूर्ण_अगर
#अगर defined(SQLITE_DEBUG) \
    && (defined(SQLITE_TEST) || defined(SQLITE_ENABLE_SELECTTRACE) \
                             || defined(SQLITE_ENABLE_TREETRACE))
# define TREETRACE_ENABLED 1
# define TREETRACE(K,P,S,X)  \
  अगर(sqlite3_वृक्ष_Trace&(K))   \
    sqlite3DebugPrintf("%u/%d/%p: ",(S)->selId,(P)->addrExplain,(S)),\
    sqlite3DebugPrintf X
#अन्यथा
# define TREETRACE(K,P,S,X)
# define TREETRACE_ENABLED 0
#पूर्ण_अगर

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
बाह्य u32 sqlite3WhereTrace;
#अगर defined(SQLITE_DEBUG) \
    && (defined(SQLITE_TEST) || defined(SQLITE_ENABLE_WHERETRACE))
# define WHERETRACE(K,X)  अगर(sqlite3WhereTrace&(K)) sqlite3DebugPrintf X
# define WHERETRACE_ENABLED 1
#अन्यथा
# define WHERETRACE(K,X)
#पूर्ण_अगर

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
प्रकार काष्ठा BusyHandler BusyHandler;
काष्ठा BusyHandler {
  पूर्णांक (*xBusyHandler)(व्योम *,पूर्णांक);  /* The busy callback */
  व्योम *pBusyArg;                   /* First arg to busy callback */
  पूर्णांक nBusy;                        /* Incremented with each busy call */
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
#घोषणा LEGACY_SCHEMA_TABLE          "sqlite_master"
#घोषणा LEGACY_अस्थायी_SCHEMA_TABLE     "sqlite_temp_master"
#घोषणा PREFERRED_SCHEMA_TABLE       "sqlite_schema"
#घोषणा PREFERRED_अस्थायी_SCHEMA_TABLE  "sqlite_temp_schema"


/*
** The root-page of the schema table.
*/
#घोषणा SCHEMA_मूल    1

/*
** The name of the schema table.  The name is different for TEMP.
*/
#घोषणा SCHEMA_TABLE(x) \
    ((!OMIT_TEMPDB)&&(x==1)?LEGACY_अस्थायी_SCHEMA_TABLE:LEGACY_SCHEMA_TABLE)

/*
** A convenience macro that returns the number of elements in
** an array.
*/
#घोषणा Array_आकार(X)    ((पूर्णांक)(माप(X)/माप(X[0])))

/*
** Determine if the argument is a power of two
*/
#घोषणा IsPowerOfTwo(X) (((X)&((X)-1))==0)

/*
** The following value as a destructor means to use sqlite3DbFree().
** The sqlite3DbFree() routine requires two parameters instead of the
** one parameter that destructors normally want.  So we have to introduce
** this magic value that the code knows to handle differently.  Any
** pointer will work here as long as it is distinct from SQLITE_STATIC
** and SQLITE_TRANSIENT.
*/
#घोषणा SQLITE_DYNAMIC   ((sqlite3_destructor_type)sqlite3RowSetClear)

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
#अगर_घोषित SQLITE_OMIT_WSD
  #घोषणा SQLITE_WSD स्थिर
  #घोषणा GLOBAL(t,v) (*(t*)sqlite3_wsd_find((व्योम*)&(v), माप(v)))
  #घोषणा sqlite3GlobalConfig GLOBAL(काष्ठा Sqlite3Config, sqlite3Config)
  पूर्णांक sqlite3_wsd_init(पूर्णांक N, पूर्णांक J);
  व्योम *sqlite3_wsd_find(व्योम *K, पूर्णांक L);
#अन्यथा
  #घोषणा SQLITE_WSD
  #घोषणा GLOBAL(t,v) v
  #घोषणा sqlite3GlobalConfig sqlite3Config
#पूर्ण_अगर

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
#घोषणा UNUSED_PARAMETER(x) (व्योम)(x)
#घोषणा UNUSED_PARAMETER2(x,y) UNUSED_PARAMETER(x),UNUSED_PARAMETER(y)

/*
** Forward references to structures
*/
प्रकार काष्ठा AggInfo AggInfo;
प्रकार काष्ठा AuthContext AuthContext;
प्रकार काष्ठा AutoincInfo AutoincInfo;
प्रकार काष्ठा Bitvec Bitvec;
प्रकार काष्ठा CollSeq CollSeq;
प्रकार काष्ठा Column Column;
प्रकार काष्ठा Cte Cte;
प्रकार काष्ठा CteUse CteUse;
प्रकार काष्ठा Db Db;
प्रकार काष्ठा Db_Client_तथ्य Db_Client_तथ्य;
प्रकार काष्ठा DbFixer DbFixer;
प्रकार काष्ठा Schema Schema;
प्रकार काष्ठा व्यंजक व्यंजक;
प्रकार काष्ठा व्यंजक_List व्यंजक_List;
प्रकार काष्ठा FKey FKey;
प्रकार काष्ठा FpDecode FpDecode;
प्रकार काष्ठा FuncDestructor FuncDestructor;
प्रकार काष्ठा FuncDef FuncDef;
प्रकार काष्ठा FuncDefHash FuncDefHash;
प्रकार काष्ठा IdList IdList;
प्रकार काष्ठा सूचकांक सूचकांक;
प्रकार काष्ठा Indexed_व्यंजक Indexed_व्यंजक;
प्रकार काष्ठा सूचकांक_Sample सूचकांक_Sample;
प्रकार काष्ठा कुंजी_Class कुंजी_Class;
प्रकार काष्ठा कुंजी_Info कुंजी_Info;
प्रकार काष्ठा Lookaside Lookaside;
प्रकार काष्ठा LookasideSlot LookasideSlot;
प्रकार काष्ठा Module Module;
प्रकार काष्ठा NameContext NameContext;
प्रकार काष्ठा OnOrUsing OnOrUsing;
प्रकार काष्ठा Parse Parse;
प्रकार काष्ठा ParseCleanup ParseCleanup;
प्रकार काष्ठा PreUpdate PreUpdate;
प्रकार काष्ठा PrintfArguments PrintfArguments;
प्रकार काष्ठा RCStr RCStr;
प्रकार काष्ठा RenameToken RenameToken;
प्रकार काष्ठा Returning Returning;
प्रकार काष्ठा RowSet RowSet;
प्रकार काष्ठा Savepoint Savepoint;
प्रकार काष्ठा Select Select;
प्रकार काष्ठा SQLiteThread SQLiteThread;
प्रकार काष्ठा SelectDest SelectDest;
प्रकार काष्ठा Subquery Subquery;
प्रकार काष्ठा SrcItem SrcItem;
प्रकार काष्ठा SrcList SrcList;
प्रकार काष्ठा sqlite3_str StrAccum; /* Internal alias for sqlite3_str */
प्रकार काष्ठा Table Table;
प्रकार काष्ठा TableLock TableLock;
प्रकार काष्ठा Token Token;
प्रकार काष्ठा वृक्ष_View वृक्ष_View;
प्रकार काष्ठा Trigger Trigger;
प्रकार काष्ठा TriggerPrg TriggerPrg;
प्रकार काष्ठा TriggerStep TriggerStep;
प्रकार काष्ठा UnpackedRecord UnpackedRecord;
प्रकार काष्ठा Upsert Upsert;
प्रकार काष्ठा VTable VTable;
प्रकार काष्ठा VtabCtx VtabCtx;
प्रकार काष्ठा Walker Walker;
प्रकार काष्ठा WhereInfo WhereInfo;
प्रकार काष्ठा Window Window;
प्रकार काष्ठा With With;


/*
** The bitmask datatype defined below is used for various optimizations.
**
** Changing this from a 64-bit to a 32-bit type limits the number of
** tables in a join to 32 instead of 64.  But it also reduces the size
** of the library by 738 bytes on ix86.
*/
#अगर_घोषित SQLITE_BITMASK_TYPE
  प्रकार SQLITE_BITMASK_TYPE Bitmask;
#अन्यथा
  प्रकार u64 Bitmask;
#पूर्ण_अगर

/*
** The number of bits in a Bitmask.  "BMS" means "BitMask Size".
*/
#घोषणा BMS  ((पूर्णांक)(माप(Bitmask)*8))

/*
** A bit in a Bitmask
*/
#घोषणा MASKBIT(n)    (((Bitmask)1)<<(n))
#घोषणा MASKBIT64(n)  (((u64)1)<<(n))
#घोषणा MASKBIT32(n)  (((अचिन्हित पूर्णांक)1)<<(n))
#घोषणा SMASKBIT32(n) ((n)<=31?((अचिन्हित पूर्णांक)1)<<(n):0)
#घोषणा ALLBITS       ((Bitmask)-1)
#घोषणा TOPBIT        (((Bitmask)1)<<(BMS-1))

/* A VList object records a mapping between parameters/variables/wildcards
** in the SQL statement (such as $abc, @pqr, or :xyz) and the integer
** variable number associated with that parameter.  See the format description
** on the sqlite3VListAdd() routine for more information.  A VList is really
** just an array of integers.
*/
प्रकार पूर्णांक VList;

/*
** Defer sourcing vdbe.h and btree.h until after the "u8" and
** "BusyHandler" typedefs. vdbe.h also requires a few of the opaque
** pointer types (i.e. FuncDef) defined above.
*/
#समावेश "os.h"
#समावेश "pager.h"
#समावेश "btree.h"
#समावेश "vdbe.h"
#समावेश "pcache.h"
#समावेश "mutex.h"

/* The SQLITE_EXTRA_DURABLE compile-time option used to set the default
** synchronous setting to EXTRA.  It is no longer supported.
*/
#अगर_घोषित SQLITE_EXTRA_DURABLE
# warning Use SQLITE_DEFAULT_SYNCHRONOUS=3 instead of SQLITE_EXTRA_DURABLE
# define SQLITE_DEFAULT_SYNCHRONOUS 3
#पूर्ण_अगर

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
#अगर_अघोषित SQLITE_DEFAULT_SYNCHRONOUS
# define SQLITE_DEFAULT_SYNCHRONOUS 2
#पूर्ण_अगर
#अगर_अघोषित SQLITE_DEFAULT_WAL_SYNCHRONOUS
# define SQLITE_DEFAULT_WAL_SYNCHRONOUS SQLITE_DEFAULT_SYNCHRONOUS
#पूर्ण_अगर

/*
** Each database file to be accessed by the system is an instance
** of the following structure.  There are normally two of these structures
** in the sqlite.aDb[] array.  aDb[0] is the main database file and
** aDb[1] is the database file used to hold temporary tables.  Additional
** databases may be attached.
*/
काष्ठा Db {
  अक्षर *zDbSName;      /* Name of this database. (schema name, not filename) */
  Btree *pBt;          /* The B*Tree structure for this database file */
  u8 safety_level;     /* How aggressive at syncing data to disk */
  u8 bSyncSet;         /* True if "PRAGMA synchronous=N" has been run */
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
काष्ठा Schema {
  पूर्णांक schema_cookie;   /* Database schema version number for this file */
  पूर्णांक iGeneration;     /* Generation counter.  Incremented with each change */
  Hash tblHash;        /* All tables indexed by name */
  Hash idxHash;        /* All (named) indices indexed by name */
  Hash trigHash;       /* All triggers indexed by name */
  Hash fkeyHash;       /* All foreign keys by referenced table name */
  Table *pSeqTab;      /* The sqlite_sequence table used by AUTOINCREMENT */
  u8 file_format;      /* Schema format version for this file */
  u8 enc;              /* Text encoding used by this database */
  u16 schemaFlags;     /* Flags associated with this schema */
  पूर्णांक cache_आकार;      /* Number of pages to use in the cache */
};

/*
** These macros can be used to test, set, or clear bits in the
** Db.pSchema->flags field.
*/
#घोषणा DbHasProperty(D,I,P)     (((D)->aDb[I].pSchema->schemaFlags&(P))==(P))
#घोषणा DbHasAnyProperty(D,I,P)  (((D)->aDb[I].pSchema->schemaFlags&(P))!=0)
#घोषणा DbSetProperty(D,I,P)     (D)->aDb[I].pSchema->schemaFlags|=(P)
#घोषणा DbClearProperty(D,I,P)   (D)->aDb[I].pSchema->schemaFlags&=~(P)

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
#घोषणा DB_SchemaLoaded    0x0001  /* The schema has been loaded */
#घोषणा DB_UnresetViews    0x0002  /* Some views have defined column names */
#घोषणा DB_ResetWanted     0x0008  /* Reset the schema when nSchemaLock==0 */

/*
** The number of different kinds of things that can be limited
** using the sqlite3_limit() interface.
*/
#घोषणा SQLITE_N_LIMIT (SQLITE_LIMIT_CTES+1)

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
काष्ठा Lookaside {
  u32 bDisable;           /* Only operate the lookaside when zero */
  u16 sz;                 /* Size of each buffer in bytes */
  u16 szTrue;             /* True value of sz, even if disabled */
  u8 bMalloced;           /* True if pStart obtained from sqlite3_malloc() */
  u32 nSlot;              /* Number of lookaside slots allocated */
  u32 anStat[3];          /* 0: hits.  1: size misses.  2: full misses */
  LookasideSlot *pInit;   /* List of buffers not previously used */
  LookasideSlot *pFree;   /* List of available buffers */
#अगर_अघोषित SQLITE_OMIT_TWOSIZE_LOOKASIDE
  LookasideSlot *pSmallInit; /* List of small buffers not previously used */
  LookasideSlot *pSmallFree; /* List of available small buffers */
  व्योम *p_मध्य;          /* First byte past end of full-size buffers and
                          ** the first byte of LOOKASIDE_SMALL buffers */
#पूर्ण_अगर /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  व्योम *pStart;           /* First byte of available memory space */
  व्योम *pEnd;             /* First byte past end of available space */
  व्योम *pTrueEnd;         /* True value of pEnd, when db->pnBytesFreed!=0 */
};
काष्ठा LookasideSlot {
  LookasideSlot *p_अग्र;    /* Next buffer in the list of free buffers */
};

#घोषणा DisableLookaside  db->lookaside.bDisable++;db->lookaside.sz=0
#घोषणा EnableLookaside   db->lookaside.bDisable--;\
   db->lookaside.sz=db->lookaside.bDisable?0:db->lookaside.szTrue

/* Size of the smaller allocations in two-size lookaside */
#अगर_घोषित SQLITE_OMIT_TWOSIZE_LOOKASIDE
#  define LOOKASIDE_SMALL           0
#अन्यथा
#  define LOOKASIDE_SMALL         128
#पूर्ण_अगर

/*
** A hash table for built-in function definitions.  (Application-defined
** functions use a regular table table from hash.h.)
**
** Hash each FuncDef structure into one of the FuncDefHash.a[] slots.
** Collisions are on the FuncDef.u.pHash chain.  Use the SQLITE_FUNC_HASH()
** macro to compute a hash on the function name.
*/
#घोषणा SQLITE_FUNC_HASH_SZ 23
काष्ठा FuncDefHash {
  FuncDef *a[SQLITE_FUNC_HASH_SZ];       /* Hash table for functions */
};
#घोषणा SQLITE_FUNC_HASH(C,L) (((C)+(L))%SQLITE_FUNC_HASH_SZ)

/*
** typedef for the authorization callback function.
*/
प्रकार पूर्णांक (*sqlite3_xauth)(व्योम*,पूर्णांक,स्थिर अक्षर*,स्थिर अक्षर*,स्थिर अक्षर*,
                             स्थिर अक्षर*);

#अगर_अघोषित SQLITE_OMIT_DEPRECATED
/* This is an extra SQLITE_TRACE macro that indicates "legacy" tracing
** in the style of sqlite3_trace()
*/
#घोषणा SQLITE_TRACE_LEGACY          0x40     /* Use the legacy xTrace */
#घोषणा SQLITE_TRACE_XPROFILE        0x80     /* Use the legacy xProfile */
#अन्यथा
#घोषणा SQLITE_TRACE_LEGACY          0
#घोषणा SQLITE_TRACE_XPROFILE        0
#पूर्ण_अगर /* SQLITE_OMIT_DEPRECATED */
#घोषणा SQLITE_TRACE_NONLEGACY_MASK  0x0f     /* Normal flags */

/*
** Maximum number of sqlite3.aDb[] entries.  This is the number of attached
** databases plus 2 for "main" and "temp".
*/
#घोषणा SQLITE_MAX_DB (SQLITE_MAX_ATTACHED+2)

/*
** Each database connection is an instance of the following structure.
*/
काष्ठा sqlite3 {
  sqlite3_vfs *pVfs;            /* OS Interface */
  काष्ठा Vdbe *pVdbe;           /* List of active virtual machines */
  CollSeq *pDfltColl;           /* BINARY collseq for the database encoding */
  sqlite3_mutex *mutex;         /* Connection mutex */
  Db *aDb;                      /* All backends */
  पूर्णांक nDb;                      /* Number of backends currently in use */
  u32 mDbFlags;                 /* flags recording internal state */
  u64 flags;                    /* flags settable by pragmas. See below */
  i64 lastRowid;                /* ROWID of most recent insert (see above) */
  i64 szMmap;                   /* Default mmap_size setting */
  u32 nSchemaLock;              /* Do not reset the schema when non-zero */
  अचिन्हित पूर्णांक openFlags;       /* Flags passed to sqlite3_vfs.xOpen() */
  पूर्णांक errCode;                  /* Most recent error code (SQLITE_*) */
  पूर्णांक errByteOffset;            /* Byte offset of error in SQL statement */
  पूर्णांक errMask;                  /* & result codes with this before returning */
  पूर्णांक iSysErrno;                /* Errno value from last system error */
  u8 enc;                       /* Text encoding */
  u8 autoCommit;                /* The auto-commit flag. */
  u8 अस्थायी_store;                /* 1: file 2: memory 0: default */
  u8 mallocFailed;              /* True if we have seen a malloc failure */
  u8 bBenignMalloc;             /* Do not require OOMs if true */
  u8 dfltLockMode;              /* Default locking-mode for attached dbs */
  चिन्हित अक्षर अग्र_Autovac;      /* Autovac setting after VACUUM if >=0 */
  u8 suppressErr;               /* Do not issue error messages if true */
  u8 vtabOnConflict;            /* Value to return for s3_vtab_on_conflict() */
  u8 isTransactionSavepoint;    /* True if the outermost savepoint is a TS */
  u8 mTrace;                    /* zero or more SQLITE_TRACE flags */
  u8 noSharedCache;             /* True if no shared-cache backends */
  u8 nSqlExec;                  /* Number of pending OP_SqlExec opcodes */
  u8 eOpenState;                /* Current condition of the connection */
  u8 nFpDigit;                  /* Significant digits to keep on double->text */
  पूर्णांक अग्र_Pagesize;             /* Pagesize after VACUUM if >0 */
  i64 nChange;                  /* Value returned by sqlite3_changes() */
  i64 n_योग_Change;             /* Value returned by sqlite3_total_changes() */
  u32 dbOptFlags;               /* Flags to enable/disable optimizations */
  पूर्णांक aLimit[SQLITE_N_LIMIT];   /* Limits */
  पूर्णांक nMaxSorterMmap;           /* Maximum size of regions mapped by sorter */
  काष्ठा sqlite3InitInfo {      /* Information used during initialization */
    Pgno newTnum;               /* Rootpage of table being initialized */
    u8 iDb;                     /* Which db file is being initialized */
    u8 busy;                    /* TRUE if initing. 2 if due to ADD COLUMN */
    अचिन्हित orphanTrigger : 1; /* Last statement is orphaned TEMP trigger */
    अचिन्हित imposterTable : 2; /* Building an imposter table */
    अचिन्हित reopenMemdb : 1;   /* ATTACH is really a reopen using MemDB */
    स्थिर अक्षर **azInit;        /* "type", "name", and "tbl_name" columns */
  } init;
  पूर्णांक nVdbeActive;              /* Number of VDBEs currently running */
  पूर्णांक nVdbeRead;                /* Number of active VDBEs that read or write */
  पूर्णांक nVdbeWrite;               /* Number of active VDBEs that read and write */
  पूर्णांक nVdbeExec;                /* Number of nested calls to VdbeExec() */
  पूर्णांक nVDestroy;                /* Number of active OP_VDestroy operations */
  पूर्णांक nExtension;               /* Number of loaded extensions */
  व्योम **aExtension;            /* Array of shared library handles */
  जोड़ {
    व्योम (*xLegacy)(व्योम*,स्थिर अक्षर*);   /* mTrace==SQLITE_TRACE_LEGACY */
    पूर्णांक (*xV2)(u32,व्योम*,व्योम*,व्योम*);    /* All other mTrace values */
  } trace;
  व्योम *pTraceArg;                        /* Argument to the trace function */
#अगर_अघोषित SQLITE_OMIT_DEPRECATED
  व्योम (*xProfile)(व्योम*,स्थिर अक्षर*,u64);  /* Profiling function */
  व्योम *pProfileArg;                        /* Argument to profile function */
#पूर्ण_अगर
  व्योम *pCommitArg;                 /* Argument to xCommitCallback() */
  पूर्णांक (*xCommitCallback)(व्योम*);    /* Invoked at every commit. */
  व्योम *pRollbackArg;               /* Argument to xRollbackCallback() */
  व्योम (*xRollbackCallback)(व्योम*); /* Invoked at every commit. */
  व्योम *pUpdateArg;
  व्योम (*xUpdateCallback)(व्योम*,पूर्णांक, स्थिर अक्षर*,स्थिर अक्षर*,sqlite_int64);
  व्योम *pAutovacPagesArg;           /* Client argument to autovac_pages */
  व्योम (*xAutovacDestr)(व्योम*);     /* Destructor for pAutovacPAgesArg */
  अचिन्हित पूर्णांक (*xAutovacPages)(व्योम*,स्थिर अक्षर*,u32,u32,u32);
  Parse *pParse;                /* Current parse */
#अगर_घोषित SQLITE_ENABLE_PREUPDATE_HOOK
  व्योम *pPreUpdateArg;          /* First argument to xPreUpdateCallback */
  व्योम (*xPreUpdateCallback)(   /* Registered using sqlite3_preupdate_hook() */
    व्योम*,sqlite3*,पूर्णांक,अक्षर स्थिर*,अक्षर स्थिर*,sqlite3_int64,sqlite3_int64
  );
  PreUpdate *pPreUpdate;        /* Context for active pre-update callback */
#पूर्ण_अगर /* SQLITE_ENABLE_PREUPDATE_HOOK */
#अगर_अघोषित SQLITE_OMIT_WAL
  पूर्णांक (*xWalCallback)(व्योम *, sqlite3 *, स्थिर अक्षर *, पूर्णांक);
  व्योम *pWalArg;
#पूर्ण_अगर
  व्योम(*xCollNeeded)(व्योम*,sqlite3*,पूर्णांक eTextRep,स्थिर अक्षर*);
  व्योम(*xCollNeeded16)(व्योम*,sqlite3*,पूर्णांक eTextRep,स्थिर व्योम*);
  व्योम *pCollNeededArg;
  sqlite3_मूल्य *pErr;          /* Most recent error message */
  जोड़ {
    अस्थिर पूर्णांक isInterrupted; /* True if sqlite3_interrupt has been called */
    द्विगुन notUsed1;            /* Spacer */
  } u1;
  Lookaside lookaside;          /* Lookaside malloc configuration */
#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth;          /* Access authorization function */
  व्योम *pAuthArg;               /* 1st argument to the access auth function */
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_PROGRESS_CALLBACK
  पूर्णांक (*xProgress)(व्योम *);     /* The progress callback */
  व्योम *pProgressArg;           /* Argument to the progress callback */
  अचिन्हित nProgressOps;        /* Number of opcodes for progress callback */
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  पूर्णांक nVTrans;                  /* Allocated size of aVTrans */
  Hash aModule;                 /* populated by sqlite3_create_module() */
  VtabCtx *pVtabCtx;            /* Context for active vtab connect/create */
  VTable **aVTrans;             /* Virtual tables with open transactions */
  VTable *pDisconnect;          /* Disconnect these in next sqlite3_prepare() */
#पूर्ण_अगर
  Hash aFunc;                   /* Hash table of connection functions */
  Hash aCollSeq;                /* All collating sequences */
  BusyHandler busyHandler;      /* Busy callback */
  Db aDbStatic[2];              /* Static space for the 2 default backends */
  Savepoint *pSavepoint;        /* List of active savepoints */
  पूर्णांक nAnalysisLimit;           /* Number of index rows to ANALYZE */
  पूर्णांक busyTimeout;              /* Busy handler timeout, in msec */
#अगर_घोषित SQLITE_ENABLE_SETLK_TIMEOUT
  पूर्णांक setlkTimeout;             /* Blocking lock timeout, in msec. -1 -> inf. */
  पूर्णांक setlkFlags;               /* Flags passed to setlk_timeout() */
#पूर्ण_अगर
  पूर्णांक nSavepoint;               /* Number of non-transaction savepoints */
  पूर्णांक nStatement;               /* Number of nested statement-transactions  */
  i64 nDeferredCons;            /* Net deferred constraints this transaction. */
  i64 nDeferredImmCons;         /* Net deferred immediate constraints */
  पूर्णांक *pnBytesFreed;            /* If not NULL, increment this in DbFree() */
  Db_Client_तथ्य *p_Db_तथ्य;        /* sqlite3_set_clientdata() content */
  u64 nSpill;                   /* TEMP content spilled to disk */
#अगर_घोषित SQLITE_ENABLE_UNLOCK_NOTIFY
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
  व्योम *pUnlockArg;                     /* Argument to xUnlockNotify */
  व्योम (*xUnlockNotify)(व्योम **, पूर्णांक);  /* Unlock notify callback */
  sqlite3 *p_अग्र_Blocked;        /* Next in list of all blocked connections */
#पूर्ण_अगर
};

/*
** A macro to discover the encoding of a database.
*/
#घोषणा SCHEMA_ENC(db) ((db)->aDb[0].pSchema->enc)
#घोषणा ENC(db)        ((db)->enc)

/*
** A u64 constant where the lower 32 bits are all zeros.  Only the
** upper 32 bits are included in the argument.  Necessary because some
** C-compilers still do not accept LL integer literals.
*/
#घोषणा HI(X)  ((u64)(X)<<32)

/*
** Possible values for the sqlite3.flags.
**
** Value constraints (enforced via assert()):
**      SQLITE_FullFSync     == PAGER_FULLFSYNC
**      SQLITE_CkptFullFSync == PAGER_CKPT_FULLFSYNC
**      SQLITE_CacheSpill    == PAGER_CACHE_SPILL
*/
#घोषणा SQLITE_WriteSchema    0x00000001  /* OK to update SQLITE_SCHEMA */
#घोषणा SQLITE_LegacyFileFmt  0x00000002  /* Create new databases in format 1 */
#घोषणा SQLITE_FullColNames   0x00000004  /* Show full column names on SELECT */
#घोषणा SQLITE_FullFSync      0x00000008  /* Use full fsync on the backend */
#घोषणा SQLITE_CkptFullFSync  0x00000010  /* Use full fsync for checkpoint */
#घोषणा SQLITE_CacheSpill     0x00000020  /* OK to spill pager cache */
#घोषणा SQLITE_ShortColNames  0x00000040  /* Show short columns names */
#घोषणा SQLITE_TrustedSchema  0x00000080  /* Allow unsafe functions and
                                          ** vtabs in the schema definition */
#घोषणा SQLITE_NullCallback   0x00000100  /* Invoke the callback once if the */
                                          /*   result set is empty */
#घोषणा SQLITE_IgnoreChecks   0x00000200  /* Do not enforce check constraints */
#घोषणा SQLITE_StmtScanStatus 0x00000400  /* Enable stmt_scanstats() counters */
#घोषणा SQLITE_NoCkptOnClose  0x00000800  /* No checkpoint on close()/DETACH */
#घोषणा SQLITE_ReverseOrder   0x00001000  /* Reverse unordered SELECTs */
#घोषणा SQLITE_RecTriggers    0x00002000  /* Enable recursive triggers */
#घोषणा SQLITE_Foreign_कुंजियाँ    0x00004000  /* Enforce foreign key constraints  */
#घोषणा SQLITE_Auto_सूचकांक      0x00008000  /* Enable automatic indexes */
#घोषणा SQLITE_LoadExtension  0x00010000  /* Enable load_extension */
#घोषणा SQLITE_LoadExtFunc    0x00020000  /* Enable load_extension() SQL func */
#घोषणा SQLITE_EnableTrigger  0x00040000  /* True to enable triggers */
#घोषणा SQLITE_DeferFKs       0x00080000  /* Defer all FK constraints */
#घोषणा SQLITE_QueryOnly      0x00100000  /* Disable database changes */
#घोषणा SQLITE_Cell_आकार_Ck     0x00200000  /* Check btree cell sizes on load */
#घोषणा SQLITE_Fts3Tokenizer  0x00400000  /* Enable fts3_tokenizer(2) */
#घोषणा SQLITE_EnableQPSG     0x00800000  /* Query Planner Stability Guarantee*/
#घोषणा SQLITE_TriggerEQP     0x01000000  /* Show trigger EXPLAIN QUERY PLAN */
#घोषणा SQLITE_ResetDatabase  0x02000000  /* Reset the database */
#घोषणा SQLITE_LegacyAlter    0x04000000  /* Legacy ALTER TABLE behaviour */
#घोषणा SQLITE_NoSchemaError  0x08000000  /* Do not report schema parse errors*/
#घोषणा SQLITE_Defensive      0x10000000  /* Input SQL is likely hostile */
#घोषणा SQLITE_DqsDDL         0x20000000  /* dbl-quoted strings allowed in DDL*/
#घोषणा SQLITE_DqsDML         0x40000000  /* dbl-quoted strings allowed in DML*/
#घोषणा SQLITE_EnableView     0x80000000  /* Enable the use of views */
#घोषणा SQLITE_गिनती_Rows      HI(0x00001) /* Count rows changed by INSERT, */
                                          /*   DELETE, or UPDATE and return */
                                          /*   the count using a callback. */
#घोषणा SQLITE_CorruptRdOnly  HI(0x00002) /* Prohibit writes due to error */
#घोषणा SQLITE_ReadUncommit   HI(0x00004) /* READ UNCOMMITTED in shared-cache */
#घोषणा SQLITE_FkNoAction     HI(0x00008) /* Treat all FK as NO ACTION */
#घोषणा SQLITE_AttachCreate   HI(0x00010) /* ATTACH allowed to create new dbs */
#घोषणा SQLITE_AttachWrite    HI(0x00020) /* ATTACH allowed to open for write */
#घोषणा SQLITE_Comments       HI(0x00040) /* Enable SQL comments */

/* Flags used only if debugging */
#अगर_घोषित SQLITE_DEBUG
#घोषणा SQLITE_SqlTrace       HI(0x0100000) /* Debug print SQL as it executes */
#घोषणा SQLITE_VdbeListing    HI(0x0200000) /* Debug listings of VDBE progs */
#घोषणा SQLITE_VdbeTrace      HI(0x0400000) /* True to trace VDBE execution */
#घोषणा SQLITE_VdbeAddopTrace HI(0x0800000) /* Trace sqlite3VdbeAddOp() calls */
#घोषणा SQLITE_VdbeEQP        HI(0x1000000) /* Debug EXPLAIN QUERY PLAN */
#घोषणा SQLITE_ParserTrace    HI(0x2000000) /* PRAGMA parser_trace=ON */
#पूर्ण_अगर

/*
** Allowed values for sqlite3.mDbFlags
*/
#घोषणा DBFLAG_SchemaChange   0x0001  /* Uncommitted Hash table changes */
#घोषणा DBFLAG_PreferBuiltin  0x0002  /* Preference to built-in funcs */
#घोषणा DBFLAG_Vacuum         0x0004  /* Currently in a VACUUM */
#घोषणा DBFLAG_VacuumInto     0x0008  /* Currently running VACUUM INTO */
#घोषणा DBFLAG_SchemaKnownOk  0x0010  /* Schema is known to be valid */
#घोषणा DBFLAG_InternalFunc   0x0020  /* Allow use of internal functions */
#घोषणा DBFLAG_EncodingFixed  0x0040  /* No longer possible to change enc. */

/*
** Bits of the sqlite3.dbOptFlags field that are used by the
** sqlite3_test_control(SQLITE_TESTCTRL_OPTIMIZATIONS,...) interface to
** selectively disable various optimizations. The numbers in each macro
** are the one-based bit position (for improved readability).
**
** Keep these values in sync with the ext/misc/optname.c extension.
*/
#घोषणा SQLITE_QueryFlattener  1 /* Query flattening */
#घोषणा SQLITE_WindowFunc      2 /* Use xInverse for window functions */
#घोषणा SQLITE_GroupByOrder    3 /* GROUPBY cover of ORDERBY */
#घोषणा SQLITE_JoinMiss        4 /* Unwind join loops on a lookup miss*/
#घोषणा SQLITE_DistinctOpt     5 /* DISTINCT using indexes */
#घोषणा SQLITE_CoverIdxScan    6 /* Covering index scans */
#घोषणा SQLITE_OrderByIdxJoin  7 /* ORDER BY of joins via index */
#घोषणा SQLITE_Transitive      8 /* Transitive constraints */
#घोषणा SQLITE_OmitNoopJoin    9 /* Omit unused tables in joins */
#घोषणा SQLITE_गिनती_Of_View    10 /* The count-of-view optimization */
#घोषणा SQLITE_CursorHints    11 /* Add OP_CursorHint opcodes */
#घोषणा SQLITE_Stat4          12 /* Use STAT4 data */
#घोषणा SQLITE_धकेलो_Down       13 /* WHERE-clause push-down opt */
#घोषणा SQLITE_SimplifyJoin   14 /* Convert LEFT JOIN to JOIN */
#घोषणा SQLITE_SkipScan       15 /* Skip-scans */
#घोषणा SQLITE_PropagateConst 16 /* The constant propagation opt */
#घोषणा SQLITE_MinMaxOpt      17 /* The min/max optimization */
#घोषणा SQLITE_SeekScan       18 /* The OP_SeekScan optimization */
#घोषणा SQLITE_OmitOrderBy    19 /* Omit pointless ORDER BY */
#घोषणा SQLITE_BloomFilter    20 /* Use a Bloom filters */
                          /*  21 -- Available for reuse */
#घोषणा SQLITE_BalancedMerge  22 /* Balance multi-way merges */
#घोषणा SQLITE_ReleaseReg     23 /* Use OP_ReleaseReg for testing */
#घोषणा SQLITE_FlttnUnionAll  24 /* Disable the UNION ALL flattener */
#घोषणा SQLITE_Indexed_व्यंजक    25 /* Pull exprs from index when able */
#घोषणा SQLITE_Coroutines     26 /* Co-routines for subqueries */
#घोषणा SQLITE_NullUnusedCols 27 /* NULL unused columns in subqueries */
#घोषणा SQLITE_OnePass        28 /* Single-pass DELETE and UPDATE */
#घोषणा SQLITE_OrderBySubq    29 /* ORDER BY in subquery helps outer */
#घोषणा SQLITE_StarQuery      30 /* Heurists for star queries */
#घोषणा SQLITE_ExistsToJoin   31 /* The EXISTS-to-JOIN optimization */
#घोषणा SQLITE_UnionLimit     32 /* Optimizations for UNION + LIMIT */

/*
** Macros for testing whether or not optimizations are enabled or disabled.
*/
#घोषणा OptimizationDisabled(D,M)  (((D)->dbOptFlags&(1ul<<((M)-1)))!=0)
#घोषणा OptimizationEnabled(D,M)   (((D)->dbOptFlags&(1ul<<((M)-1)))==0)

/*
** Return true if it OK to factor constant expressions into the initialization
** code. The argument is a Parse object for the code generator.
*/
#घोषणा ConstFactorOk(P) ((P)->okConstFactor)

/* Possible values for the sqlite3.eOpenState field.
** The numbers are randomly selected such that a minimum of three bits must
** change to convert any number to another or to zero
*/
#घोषणा SQLITE_STATE_OPEN     0x76  /* Database is open */
#घोषणा SQLITE_STATE_CLOSED   0xce  /* Database is closed */
#घोषणा SQLITE_STATE_SICK     0xba  /* Error and awaiting close */
#घोषणा SQLITE_STATE_BUSY     0x6d  /* Database currently in use */
#घोषणा SQLITE_STATE_ERROR    0xd5  /* An SQLITE_MISUSE error occurred */
#घोषणा SQLITE_STATE_ZOMBIE   0xa7  /* Close with last statement close */

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
काष्ठा FuncDef {
  i16 nArg;            /* Number of arguments.  -1 means unlimited */
  u32 funcFlags;       /* Some combination of SQLITE_FUNC_* */
  व्योम *p_User_तथ्य;     /* User data parameter */
  FuncDef *p_अग्र;      /* Next function with same name */
  व्योम (*xSFunc)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**); /* func or agg-step */
  व्योम (*xFinalize)(sqlite3_context*);                  /* Agg finalizer */
  व्योम (*x_मूल्य)(sqlite3_context*);                     /* Current agg value */
  व्योम (*xInverse)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य**); /* inverse agg-step */
  स्थिर अक्षर *zName;   /* SQL name of the function. */
  जोड़ {
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
काष्ठा FuncDestructor {
  पूर्णांक nRef;
  व्योम (*xDestroy)(व्योम *);
  व्योम *p_User_तथ्य;
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
#घोषणा SQLITE_FUNC_ENCMASK  0x0003 /* SQLITE_UTF8, SQLITE_UTF16BE or UTF16LE */
#घोषणा SQLITE_FUNC_LIKE     0x0004 /* Candidate for the LIKE optimization */
#घोषणा SQLITE_FUNC_CASE     0x0008 /* Case-sensitive LIKE-type function */
#घोषणा SQLITE_FUNC_EPHEM    0x0010 /* Ephemeral.  Delete with VDBE */
#घोषणा SQLITE_FUNC_NEEDCOLL 0x0020 /* sqlite3GetFuncCollSeq() might be called*/
#घोषणा SQLITE_FUNC_लंबाई   0x0040 /* Built-in length() function */
#घोषणा SQLITE_FUNC_TYPEOF   0x0080 /* Built-in typeof() function */
#घोषणा SQLITE_FUNC_BYTELEN  0x00c0 /* Built-in octet_length() function */
#घोषणा SQLITE_FUNC_गिनती    0x0100 /* Built-in count(*) aggregate */
/*                           0x0200 -- available for reuse */
#घोषणा SQLITE_FUNC_UNLIKELY 0x0400 /* Built-in unlikely() function */
#घोषणा SQLITE_FUNC_CONSTANT 0x0800 /* Constant inputs give a constant output */
#घोषणा SQLITE_FUNC_MINMAX   0x1000 /* True for min() and max() aggregates */
#घोषणा SQLITE_FUNC_SLOCHNG  0x2000 /* "Slow Change". Value constant during a
                                    ** single query - might change over time */
#घोषणा SQLITE_FUNC_TEST     0x4000 /* Built-in testing functions */
#घोषणा SQLITE_FUNC_RUNONLY  0x8000 /* Cannot be used by valueFromFunction */
#घोषणा SQLITE_FUNC_WINDOW   0x00010000 /* Built-in window-only function */
#घोषणा SQLITE_FUNC_INTERNAL 0x00040000 /* For use by NestedParse() only */
#घोषणा SQLITE_FUNC_DIRECT   0x00080000 /* Not for use in TRIGGERs or VIEWs */
/* SQLITE_SUBTYPE            0x00100000 // Consumer of subtypes */
#घोषणा SQLITE_FUNC_UNSAFE   0x00200000 /* Function has side effects */
#घोषणा SQLITE_FUNC_INLINE   0x00400000 /* Functions implemented in-line */
#घोषणा SQLITE_FUNC_BUILTIN  0x00800000 /* This is a built-in function */
/*  SQLITE_RESULT_SUBTYPE    0x01000000 // Generator of subtypes */
#घोषणा SQLITE_FUNC_ANYORDER 0x08000000 /* count/min/max aggregate */

/* Identifier numbers for each in-line function */
#घोषणा INLINEFUNC_coalesce             0
#घोषणा INLINEFUNC_implies_nonnull_row  1
#घोषणा INLINEFUNC_व्यंजक_implies_व्यंजक    2
#घोषणा INLINEFUNC_व्यंजक_compare         3
#घोषणा INLINEFUNC_affinity             4
#घोषणा INLINEFUNC_iif                  5
#घोषणा INLINEFUNC_sqlite_offset        6
#घोषणा INLINEFUNC_unlikely            99  /* Default case */

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
#घोषणा FUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_CONSTANT|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_TO_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#घोषणा VFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_TO_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#घोषणा SFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|SQLITE_DIRECTONLY|SQLITE_FUNC_UNSAFE, \
   SQLITE_INT_TO_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#घोषणा MFUNCTION(zName, nArg, xPtr, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_CONSTANT|SQLITE_UTF8, \
   xPtr, 0, xFunc, 0, 0, 0, #zName, {0} }
#घोषणा JFUNCTION(zName, nArg, bUseCache, bWS, bRS, bJsonB, iArg, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_DETERMINISTIC|SQLITE_FUNC_CONSTANT|\
   SQLITE_UTF8|((bUseCache)*SQLITE_FUNC_RUNONLY)|\
   ((bRS)*SQLITE_SUBTYPE)|((bWS)*SQLITE_परिणाम_SUBTYPE), \
   SQLITE_INT_TO_PTR(iArg|((bJsonB)*JSON_BLOB)),0,xFunc,0, 0, 0, #zName, {0} }
#घोषणा INLINE_FUNC(zName, nArg, iArg, mFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_UTF8|SQLITE_FUNC_INLINE|SQLITE_FUNC_CONSTANT|(mFlags), \
   SQLITE_INT_TO_PTR(iArg), 0, noopFunc, 0, 0, 0, #zName, {0} }
#घोषणा TEST_FUNC(zName, nArg, iArg, mFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
         SQLITE_UTF8|SQLITE_FUNC_INTERNAL|SQLITE_FUNC_TEST| \
         SQLITE_FUNC_INLINE|SQLITE_FUNC_CONSTANT|(mFlags), \
   SQLITE_INT_TO_PTR(iArg), 0, noopFunc, 0, 0, 0, #zName, {0} }
#घोषणा DFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_SLOCHNG|SQLITE_UTF8, \
   0, 0, xFunc, 0, 0, 0, #zName, {0} }
#घोषणा PURE_DATE(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
         SQLITE_FUNC_SLOCHNG|SQLITE_UTF8|SQLITE_FUNC_CONSTANT, \
   (व्योम*)&sqlite3Config, 0, xFunc, 0, 0, 0, #zName, {0} }
#घोषणा FUNCTION2(zName, nArg, iArg, bNC, xFunc, extraFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_CONSTANT|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL)|extraFlags,\
   SQLITE_INT_TO_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#घोषणा STR_FUNCTION(zName, nArg, pArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_SLOCHNG|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   pArg, 0, xFunc, 0, 0, 0, #zName, {0} }
#घोषणा LIKEFUNC(zName, nArg, arg, flags) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_CONSTANT|SQLITE_UTF8|flags, \
   (व्योम *)arg, 0, likeFunc, 0, 0, 0, #zName, {0} }
#घोषणा WAGGREGATE(zName, nArg, arg, nc, xStep, xFinal, x_मूल्य, xInverse, f) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|(nc*SQLITE_FUNC_NEEDCOLL)|f, \
   SQLITE_INT_TO_PTR(arg), 0, xStep,xFinal,x_मूल्य,xInverse,#zName, {0}}
#घोषणा INTERNAL_FUNCTION(zName, nArg, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_INTERNAL|SQLITE_UTF8|SQLITE_FUNC_CONSTANT, \
   0, 0, xFunc, 0, 0, 0, #zName, {0} }


/*
** All current savepoints are stored in a linked list starting at
** sqlite3.pSavepoint. The first element in the list is the most recently
** opened savepoint. Savepoints are added to the list by the vdbe
** OP_Savepoint instruction.
*/
काष्ठा Savepoint {
  अक्षर *zName;                        /* Savepoint name (nul-terminated) */
  i64 nDeferredCons;                  /* Number of deferred fk violations */
  i64 nDeferredImmCons;               /* Number of deferred imm fk. */
  Savepoint *p_अग्र;                   /* Parent savepoint (if any) */
};

/*
** The following are used as the second parameter to sqlite3Savepoint(),
** and as the P1 argument to the OP_Savepoint instruction.
*/
#घोषणा SAVEPOINT_BEGIN      0
#घोषणा SAVEPOINT_RELEASE    1
#घोषणा SAVEPOINT_ROLLBACK   2


/*
** Each SQLite module (virtual table definition) is defined by an
** instance of the following structure, stored in the sqlite3.aModule
** hash table.
*/
काष्ठा Module {
  स्थिर sqlite3_module *pModule;       /* Callback pointers */
  स्थिर अक्षर *zName;                   /* Name passed to create_module() */
  पूर्णांक nRefModule;                      /* Number of pointers to this object */
  व्योम *pAux;                          /* pAux passed to create_module() */
  व्योम (*xDestroy)(व्योम *);            /* Module destructor function */
  Table *pEpoTab;                      /* Eponymous table for this module */
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
काष्ठा Column {
  अक्षर *zCnName;        /* Name of this column */
  अचिन्हित notNull :4;  /* An OE_ code for handling a NOT NULL constraint */
  अचिन्हित eCType :4;   /* One of the standard types */
  अक्षर affinity;        /* One of the SQLITE_AFF_... values */
  u8 szEst;             /* Est size of value in this column. sizeof(INT)==1 */
  u8 hName;             /* Column name hash for faster lookup */
  u16 iDflt;            /* 1-based index of DEFAULT.  0 means "none" */
  u16 colFlags;         /* Boolean properties.  See COLFLAG_ defines below */
};

/* Allowed values for Column.eCType.
**
** Values must match entries in the global constant arrays
** sqlite3StdTypeLen[] and sqlite3StdType[].  Each value is one more
** than the offset into these arrays for the corresponding name.
** Adjust the SQLITE_N_STDTYPE value if adding or removing entries.
*/
#घोषणा COLTYPE_CUSTOM      0   /* Type appended to zName */
#घोषणा COLTYPE_ANY         1
#घोषणा COLTYPE_BLOB        2
#घोषणा COLTYPE_INT         3
#घोषणा COLTYPE_INTEGER     4
#घोषणा COLTYPE_REAL        5
#घोषणा COLTYPE_TEXT        6
#घोषणा SQLITE_N_STDTYPE    6  /* Number of standard types */

/* Allowed values for Column.colFlags.
**
** Constraints:
**         TF_HasVirtual == COLFLAG_VIRTUAL
**         TF_HasStored  == COLFLAG_STORED
**         TF_HasHidden  == COLFLAG_HIDDEN
*/
#घोषणा COLFLAG_PRIMKEY   0x0001   /* Column is part of the primary key */
#घोषणा COLFLAG_HIDDEN    0x0002   /* A hidden column in a virtual table */
#घोषणा COLFLAG_HASTYPE   0x0004   /* Type name follows column name */
#घोषणा COLFLAG_UNIQUE    0x0008   /* Column def contains "UNIQUE" or "PK" */
#घोषणा COLFLAG_SORTERREF 0x0010   /* Use sorter-refs with this column */
#घोषणा COLFLAG_VIRTUAL   0x0020   /* GENERATED ALWAYS AS ... VIRTUAL */
#घोषणा COLFLAG_STORED    0x0040   /* GENERATED ALWAYS AS ... STORED */
#घोषणा COLFLAG_NOTAVAIL  0x0080   /* STORED column not yet calculated */
#घोषणा COLFLAG_BUSY      0x0100   /* Blocks recursion on GENERATED columns */
#घोषणा COLFLAG_HASCOLL   0x0200   /* Has collating sequence name in zCnName */
#घोषणा COLFLAG_NOEXPAND  0x0400   /* Omit this column when expanding "*" */
#घोषणा COLFLAG_GENERATED 0x0060   /* Combo: _STORED, _VIRTUAL */
#घोषणा COLFLAG_NOINSERT  0x0062   /* Combo: _HIDDEN, _STORED, _VIRTUAL */

/*
** A "Collating Sequence" is defined by an instance of the following
** structure. Conceptually, a collating sequence consists of a name and
** a comparison routine that defines the order of that sequence.
**
** If CollSeq.xCmp is NULL, it means that the
** collating sequence is undefined.  Indices built on an undefined
** collating sequence may not be read or written.
*/
काष्ठा CollSeq {
  अक्षर *zName;          /* Name of the collating sequence, UTF-8 encoded */
  u8 enc;               /* Text encoding handled by xCmp() */
  व्योम *pUser;          /* First argument to xCmp() */
  पूर्णांक (*xCmp)(व्योम*,पूर्णांक, स्थिर व्योम*, पूर्णांक, स्थिर व्योम*);
  व्योम (*xDel)(व्योम*);  /* Destructor for pUser */
};

/*
** A sort order can be either ASC or DESC.
*/
#घोषणा SQLITE_SO_ASC       0  /* Sort in ascending order */
#घोषणा SQLITE_SO_DESC      1  /* Sort in ascending order */
#घोषणा SQLITE_SO_UNDEFINED -1 /* No sort order specified */

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
#घोषणा SQLITE_AFF_NONE     0x40  /* '@' */
#घोषणा SQLITE_AFF_BLOB     0x41  /* 'A' */
#घोषणा SQLITE_AFF_TEXT     0x42  /* 'B' */
#घोषणा SQLITE_AFF_NUMERIC  0x43  /* 'C' */
#घोषणा SQLITE_AFF_INTEGER  0x44  /* 'D' */
#घोषणा SQLITE_AFF_REAL     0x45  /* 'E' */
#घोषणा SQLITE_AFF_FLEXNUM  0x46  /* 'F' */
#घोषणा SQLITE_AFF_DEFER    0x58  /* 'X'  - defer computation until later */

#घोषणा sqlite3IsNumericAffinity(X)  ((X)>=SQLITE_AFF_NUMERIC)

/*
** The SQLITE_AFF_MASK values masks off the significant bits of an
** affinity value.
*/
#घोषणा SQLITE_AFF_MASK     0x47

/*
** Additional bit values that can be ORed with an affinity without
** changing the affinity.
**
** The SQLITE_NOTNULL flag is a combination of NULLEQ and JUMPIFNULL.
** It causes an assert() to fire if either operand to a comparison
** operator is NULL.  It is added to certain comparison operators to
** prove that the operands are always NOT NULL.
*/
#घोषणा SQLITE_JUMPIFNULL   0x10  /* jumps if either operand is NULL */
#घोषणा SQLITE_NULLEQ       0x80  /* NULL=NULL */
#घोषणा SQLITE_NOTNULL      0x90  /* Assert that operands are never NULL */

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
काष्ठा VTable {
  sqlite3 *db;              /* Database connection associated with this table */
  Module *pMod;             /* Pointer to module implementation */
  sqlite3_vtab *pVtab;      /* Pointer to vtab instance */
  पूर्णांक nRef;                 /* Number of pointers to this structure */
  u8 bConstraint;           /* True if constraints are supported */
  u8 bAllSchemas;           /* True if might use any attached schema */
  u8 eVtabRisk;             /* Riskiness of allowing hacker access */
  पूर्णांक iSavepoint;           /* Depth of the SAVEPOINT stack */
  VTable *p_अग्र;            /* Next in linked list (see above) */
};

/* Allowed values for VTable.eVtabRisk
*/
#घोषणा SQLITE_VTABRISK_निचला          0
#घोषणा SQLITE_VTABRISK_Normal       1
#घोषणा SQLITE_VTABRISK_ऊपरी         2

/*
** The schema for each SQL table, virtual table, and view is represented
** in memory by an instance of the following structure.
*/
काष्ठा Table {
  अक्षर *zName;         /* Name of the table or view */
  Column *aCol;        /* Information about each column */
  सूचकांक *p_सूचकांक;       /* List of SQL indexes on this table. */
  अक्षर *zColAff;       /* String defining the affinity of each column */
  व्यंजक_List *pCheck;    /* All CHECK constraints */
                       /*   ... also used as column name list in a VIEW */
  Pgno tnum;           /* Root BTree page for this table */
  u32 nTabRef;         /* Number of pointers to this Table */
  u32 tabFlags;        /* Mask of TF_* values */
  i16 iPKey;           /* If not negative, use aCol[iPKey] as the rowid */
  i16 nCol;            /* Number of columns in this table */
  i16 nNVCol;          /* Number of columns that are not VIRTUAL */
  LogEst nRowLogEst;   /* Estimated rows in table - from sqlite_stat1 table */
  LogEst szTabRow;     /* Estimated size of each table row in bytes */
#अगर_घोषित SQLITE_ENABLE_COSTMULT
  LogEst costMult;     /* Cost multiplier for using this table */
#पूर्ण_अगर
  u8 कुंजी_Conf;          /* What to do in case of uniqueness conflict on iPKey */
  u8 eTabType;         /* 0: normal, 1: virtual, 2: view */
  जोड़ {
    काष्ठा {             /* Used by ordinary tables: */
      पूर्णांक addColOffset;    /* Offset in CREATE TABLE stmt to add a new column */
      FKey *pFKey;         /* Linked list of all foreign keys in this table */
      व्यंजक_List *pDfltList; /* DEFAULT clauses on various columns.
                           ** Or the AS clause for generated columns. */
    } tab;
    काष्ठा {             /* Used by views: */
      Select *pSelect;     /* View definition */
    } view;
    काष्ठा {             /* Used by virtual tables only: */
      पूर्णांक nArg;            /* Number of arguments to the module */
      अक्षर **azArg;        /* 0: module 1: schema 2: vtab name 3...: args */
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
#घोषणा TF_Readonly       0x00000001 /* Read-only system table */
#घोषणा TF_HasHidden      0x00000002 /* Has one or more hidden columns */
#घोषणा TF_Has_Primary_कुंजी  0x00000004 /* Table has a primary key */
#घोषणा TF_Autoincrement  0x00000008 /* Integer primary key is autoincrement */
#घोषणा TF_HasStat1       0x00000010 /* nRowLogEst set from sqlite_stat1 */
#घोषणा TF_HasVirtual     0x00000020 /* Has one or more VIRTUAL columns */
#घोषणा TF_HasStored      0x00000040 /* Has one or more STORED columns */
#घोषणा TF_HasGenerated   0x00000060 /* Combo: HasVirtual + HasStored */
#घोषणा TF_WithoutRowid   0x00000080 /* No rowid.  PRIMARY KEY is the key */
#घोषणा TF_MaybeReanalyze 0x00000100 /* Maybe run ANALYZE on this table */
#घोषणा TF_NoVisibleRowid 0x00000200 /* No user-visible "rowid" column */
#घोषणा TF_OOOHidden      0x00000400 /* Out-of-Order hidden columns */
#घोषणा TF_HasNotNull     0x00000800 /* Contains NOT NULL constraints */
#घोषणा TF_Shadow         0x00001000 /* True for a shadow table */
#घोषणा TF_HasStat4       0x00002000 /* STAT4 info available for this table */
#घोषणा TF_Ephemeral      0x00004000 /* An ephemeral table */
#घोषणा TF_Eponymous      0x00008000 /* An eponymous virtual table */
#घोषणा TF_Strict         0x00010000 /* STRICT mode */
#घोषणा TF_Imposter       0x00020000 /* An imposter table */

/*
** Allowed values for Table.eTabType
*/
#घोषणा TABTYP_NORM      0     /* Ordinary table */
#घोषणा TABTYP_VTAB      1     /* Virtual table */
#घोषणा TABTYP_VIEW      2     /* A view */

#घोषणा IsView(X)           ((X)->eTabType==TABTYP_VIEW)
#घोषणा IsOrdinaryTable(X)  ((X)->eTabType==TABTYP_NORM)

/*
** Test to see whether or not a table is a virtual table.  This is
** done as a macro so that it will be optimized out when virtual
** table support is omitted from the build.
*/
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
#  define IsVirtual(X)      ((X)->eTabType==TABTYP_VTAB)
#  define व्यंजक_Is_Vtab(X)  \
   ((X)->op==TK_COLUMN && (X)->y.pTab->eTabType==TABTYP_VTAB)
#अन्यथा
#  define IsVirtual(X)      0
#  define व्यंजक_Is_Vtab(X)     0
#पूर्ण_अगर

/*
** Macros to determine if a column is hidden.  IsOrdinaryHiddenColumn()
** only works for non-virtual tables (ordinary tables and views) and is
** always false unless SQLITE_ENABLE_HIDDEN_COLUMNS is defined.  The
** IsHiddenColumn() macro is general purpose.
*/
#अगर defined(SQLITE_ENABLE_HIDDEN_COLUMNS)
#  define IsHiddenColumn(X)         (((X)->colFlags & COLFLAG_HIDDEN)!=0)
#  define IsOrdinaryHiddenColumn(X) (((X)->colFlags & COLFLAG_HIDDEN)!=0)
#या_अगर !defined(SQLITE_OMIT_VIRTUALTABLE)
#  define IsHiddenColumn(X)         (((X)->colFlags & COLFLAG_HIDDEN)!=0)
#  define IsOrdinaryHiddenColumn(X) 0
#अन्यथा
#  define IsHiddenColumn(X)         0
#  define IsOrdinaryHiddenColumn(X) 0
#पूर्ण_अगर


/* Does the table have a rowid */
#घोषणा HasRowid(X)     (((X)->tabFlags & TF_WithoutRowid)==0)
#घोषणा VisibleRowid(X) (((X)->tabFlags & TF_NoVisibleRowid)==0)

/* Macro is true if the SQLITE_ALLOW_ROWID_IN_VIEW (mis-)feature is
** available.  By default, this macro is false
*/
#अगर_अघोषित SQLITE_ALLOW_ROWID_IN_VIEW
# define ViewCanHaveRowid     0
#अन्यथा
# define ViewCanHaveRowid     (sqlite3Config.mNoVisibleRowid==0)
#पूर्ण_अगर

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
काष्ठा FKey {
  Table *pFrom;     /* Table containing the REFERENCES clause (aka: Child) */
  FKey *p_अग्र_From;  /* Next FKey with the same in pFrom. Next parent of pFrom */
  अक्षर *zTo;        /* Name of table that the key points to (aka: Parent) */
  FKey *p_अग्र_To;    /* Next with the same zTo. Next child of zTo. */
  FKey *p_पूर्व_To;    /* Previous with the same zTo */
  पूर्णांक nCol;         /* Number of columns in this key */
  /* EV: R-30323-21917 */
  u8 isDeferred;       /* True if constraint checking is deferred till COMMIT */
  u8 aAction[2];        /* ON DELETE and ON UPDATE actions, respectively */
  Trigger *apTrigger[2];/* Triggers for aAction[] actions */
  काष्ठा sColMap {      /* Mapping of columns in pFrom to columns in zTo */
    पूर्णांक iFrom;            /* Index of column in pFrom */
    अक्षर *zCol;           /* Name of column in zTo.  If NULL use PRIMARY KEY */
  } aCol[FLEXARRAY];      /* One entry for each of nCol columns */
};

/* The size (in bytes) of an FKey object holding N columns.  The answer
** does NOT include space to hold the zTo name. */
#घोषणा SZ_FKEY(N)  (दुरत्व(FKey,aCol)+(N)*माप(काष्ठा sColMap))

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
#घोषणा OE_None     0   /* There is no constraint to check */
#घोषणा OE_Rollback 1   /* Fail the operation and rollback the transaction */
#घोषणा OE_Abort    2   /* Back out changes but do no rollback transaction */
#घोषणा OE_Fail     3   /* Stop the operation but leave all prior changes */
#घोषणा OE_Ignore   4   /* Ignore the error. Do not do the INSERT or UPDATE */
#घोषणा OE_Replace  5   /* Delete existing record, then do INSERT or UPDATE */
#घोषणा OE_Update   6   /* Process as a DO UPDATE in an upsert */
#घोषणा OE_Restrict 7   /* OE_Abort for IMMEDIATE, OE_Rollback for DEFERRED */
#घोषणा OE_SetNull  8   /* Set the foreign key value to NULL */
#घोषणा OE_SetDflt  9   /* Set the foreign key value to its default */
#घोषणा OE_Cascade  10  /* Cascade the changes */
#घोषणा OE_Default  11  /* Do whatever the default action is */


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
काष्ठा कुंजी_Info {
  u32 nRef;           /* Number of references to this KeyInfo object */
  u8 enc;             /* Text encoding - one of the SQLITE_UTF* values */
  u16 n_कुंजी_Field;      /* Number of key columns in the index */
  u16 nAllField;      /* Total columns, including key plus others */
  sqlite3 *db;        /* The database connection */
  u8 *aSortFlags;     /* Sort order for each column. */
  CollSeq *aColl[FLEXARRAY]; /* Collating sequence for each term of the key */
};

/* The size (in bytes) of a KeyInfo object with up to N fields.  This includes
** the main body of the KeyInfo object and the aColl[] array of N elements,
** but does not count the memory used to hold aSortFlags[]. */
#घोषणा SZ_KEYINFO(N)  (दुरत्व(कुंजी_Info,aColl) + (N)*माप(CollSeq*))

/* The size of a bare KeyInfo with no aColl[] entries */
#अगर FLEXARRAY+1 > 1
# define SZ_KEYINFO_0   दुरत्व(कुंजी_Info,aColl)
#अन्यथा
# define SZ_KEYINFO_0   माप(कुंजी_Info)
#पूर्ण_अगर

/*
** Allowed bit values for entries in the KeyInfo.aSortFlags[] array.
*/
#घोषणा KEYINFO_ORDER_DESC    0x01    /* DESC sort order */
#घोषणा KEYINFO_ORDER_BIGNULL 0x02    /* NULL is larger than any other value */

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
काष्ठा UnpackedRecord {
  कुंजी_Info *p_कुंजी_Info;  /* Comparison info for the index that is unpacked */
  Mem *aMem;          /* Values for columns of the index */
  जोड़ {
    अक्षर *z;            /* Cache of aMem[0].z for vdbeRecordCompareString() */
    i64 i;              /* Cache of aMem[0].u.i for vdbeRecordCompareInt() */
  } u;
  पूर्णांक n;              /* Cache of aMem[0].n used by vdbeRecordCompareString() */
  u16 nField;         /* Number of entries in apMem[] */
  i8 default_rc;      /* Comparison result if keys are equal */
  u8 errCode;         /* Error detected by xRecordCompare (CORRUPT or NOMEM) */
  i8 r1;              /* Value to return if (lhs < rhs) */
  i8 r2;              /* Value to return if (lhs > rhs) */
  u8 eqSeen;          /* True if an equality comparison has been seen */
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
काष्ठा सूचकांक {
  अक्षर *zName;             /* Name of this index */
  i16 *aiColumn;           /* Which columns are used by this index.  1st is 0 */
  LogEst *aiRowLogEst;     /* From ANALYZE: Est. rows selected by each column */
  Table *pTable;           /* The SQL table being indexed */
  अक्षर *zColAff;           /* String defining the affinity of each column */
  सूचकांक *p_अग्र;            /* The next index associated with the same table */
  Schema *pSchema;         /* Schema containing this index */
  u8 *aSortOrder;          /* for each column: True==DESC, False==ASC */
  स्थिर अक्षर **azColl;     /* Array of collation sequence names for index */
  व्यंजक *pPartIdxWhere;     /* WHERE clause for partial indices */
  व्यंजक_List *a_Col_व्यंजक;      /* Column expressions */
  Pgno tnum;               /* DB Page containing root of this index */
  LogEst szIdxRow;         /* Estimated average row size in bytes */
  u16 n_कुंजी_Col;             /* Number of columns forming the key */
  u16 nColumn;             /* Nr columns in btree. Can be 2*Table.nCol */
  u8 onError;              /* OE_Abort, OE_Ignore, OE_Replace, or OE_None */
  अचिन्हित idxType:2;      /* 0:Normal 1:UNIQUE, 2:PRIMARY KEY, 3:IPK */
  अचिन्हित bUnordered:1;   /* Use this index for == or IN queries only */
  अचिन्हित uniqNotNull:1;  /* True if UNIQUE and NOT NULL for all columns */
  अचिन्हित isResized:1;    /* True if resizeIndexObject() has been called */
  अचिन्हित isCovering:1;   /* True if this is a covering index */
  अचिन्हित noSkipScan:1;   /* Do not try to use skip-scan if true */
  अचिन्हित hasStat1:1;     /* aiRowLogEst values come from sqlite_stat1 */
  अचिन्हित bNoQuery:1;     /* Do not use this index to optimize queries */
  अचिन्हित b_Asc_कुंजी_Bug:1;   /* True if the bba7b69f9849b5bf bug applies */
  अचिन्हित bHasVCol:1;     /* Index references one or more VIRTUAL columns */
  अचिन्हित b_Has_व्यंजक:1;     /* Index contains an expression, either a literal
                           ** expression, or a reference to a VIRTUAL column */
#अगर_घोषित SQLITE_ENABLE_STAT4
  पूर्णांक nSample;             /* Number of elements in aSample[] */
  पूर्णांक mxSample;            /* Number of slots allocated to aSample[] */
  पूर्णांक nSampleCol;          /* Size of IndexSample.anEq[] and so on */
  tRowcnt *aAvgEq;         /* Average nEq values for keys not in aSample */
  सूचकांक_Sample *aSample;    /* Samples of the left-most key */
  tRowcnt *aiRowEst;       /* Non-logarithmic stat1 data for this index */
  tRowcnt nRowEst0;        /* Non-logarithmic number of rows in the index */
#पूर्ण_अगर
  Bitmask colNotIdxed;     /* Unindexed columns in pTab */
};

/*
** Allowed values for Index.idxType
*/
#घोषणा SQLITE_IDXTYPE_APPDEF      0   /* Created using CREATE INDEX */
#घोषणा SQLITE_IDXTYPE_UNIQUE      1   /* Implements a UNIQUE constraint */
#घोषणा SQLITE_IDXTYPE_PRIMARYKEY  2   /* Is the PRIMARY KEY for the table */
#घोषणा SQLITE_IDXTYPE_IPK         3   /* INTEGER PRIMARY KEY index */

/* Return true if index X is a PRIMARY KEY index */
#घोषणा Is_Primary_कुंजी_सूचकांक(X)  ((X)->idxType==SQLITE_IDXTYPE_PRIMARYKEY)

/* Return true if index X is a UNIQUE index */
#घोषणा Is_Unique_सूचकांक(X)      ((X)->onError!=OE_None)

/* The Index.aiColumn[] values are normally positive integer.  But
** there are some negative values that have special meaning:
*/
#घोषणा XN_ROWID     (-1)     /* Indexed column is the rowid */
#घोषणा XN_व्यंजक      (-2)     /* Indexed column is an expression */

/*
** Each sample stored in the sqlite_stat4 table is represented in memory
** using a structure of this type.  See documentation at the top of the
** analyze.c source file for additional information.
*/
काष्ठा सूचकांक_Sample {
  व्योम *p;          /* Pointer to sampled record */
  पूर्णांक n;            /* Size of record in bytes */
  tRowcnt *anEq;    /* Est. number of rows where the key equals this sample */
  tRowcnt *anLt;    /* Est. number of rows where key is less than this sample */
  tRowcnt *anDLt;   /* Est. number of distinct keys less than this sample */
};

/*
** Possible values to use within the flags argument to sqlite3GetToken().
*/
#घोषणा SQLITE_TOKEN_QUOTED    0x1 /* Token is a quoted identifier. */
#घोषणा SQLITE_TOKEN_KEYWORD   0x2 /* Token is a keyword. */

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
काष्ठा Token {
  स्थिर अक्षर *z;     /* Text of the token.  Not NULL-terminated! */
  अचिन्हित पूर्णांक n;    /* Number of characters in this token */
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
काष्ठा AggInfo {
  u8 directMode;          /* Direct rendering mode means take data directly
                          ** from source tables rather than from accumulators */
  u8 useSortingIdx;       /* In direct mode, reference the sorting index rather
                          ** than the source table */
  u32 nSortingColumn;     /* Number of columns in the sorting index */
  पूर्णांक sortingIdx;         /* Cursor number of the sorting index */
  पूर्णांक sortingIdxPTab;     /* Cursor number of pseudo-table */
  पूर्णांक iFirstReg;          /* First register in range for aCol[] and aFunc[] */
  व्यंजक_List *pGroupBy;     /* The group by clause */
  काष्ठा AggInfo_col {    /* For each column used in source tables */
    Table *pTab;             /* Source table */
    व्यंजक *pCExpr;            /* The original expression */
    पूर्णांक iTable;              /* Cursor number of the source table */
    पूर्णांक iColumn;             /* Column number within the source table */
    पूर्णांक iSorterColumn;       /* Column number in the sorting index */
  } *aCol;
  पूर्णांक nColumn;            /* Number of used entries in aCol[] */
  पूर्णांक nAccumulator;       /* Number of columns that show through to the output.
                          ** Additional columns are used only as parameters to
                          ** aggregate functions */
  काष्ठा AggInfo_func {   /* For each aggregate function */
    व्यंजक *pFExpr;            /* Expression encoding the function */
    FuncDef *pFunc;          /* The aggregate function implementation */
    पूर्णांक iDistinct;           /* Ephemeral table used to enforce DISTINCT */
    पूर्णांक iDistAddr;           /* Address of OP_OpenEphemeral */
    पूर्णांक iOBTab;              /* Ephemeral table to implement ORDER BY */
    u8 bOBPayload;           /* iOBTab has payload columns separate from key */
    u8 bOBUnique;            /* Enforce uniqueness on iOBTab keys */
    u8 bUseSubtype;          /* Transfer subtype info through sorter */
  } *aFunc;
  पूर्णांक nFunc;              /* Number of entries in aFunc[] */
  u32 selId;              /* Select to which this AggInfo belongs */
#अगर_घोषित SQLITE_DEBUG
  Select *pSelect;        /* SELECT statement that this AggInfo supports */
#पूर्ण_अगर
};

/*
** Macros to compute aCol[] and aFunc[] register numbers.
**
** These macros should not be used prior to the call to
** assignAggregateRegisters() that computes the value of pAggInfo->iFirstReg.
** The assert()s that are part of this macro verify that constraint.
*/
#अगर_अघोषित न_संशोधन
#घोषणा AggInfoColumnReg(A,I)  (निश्चित((A)->iFirstReg),(A)->iFirstReg+(I))
#घोषणा AggInfoFuncReg(A,I)    \
                      (निश्चित((A)->iFirstReg),(A)->iFirstReg+(A)->nColumn+(I))
#अन्यथा
#घोषणा AggInfoColumnReg(A,I)  ((A)->iFirstReg+(I))
#घोषणा AggInfoFuncReg(A,I)    \
                      ((A)->iFirstReg+(A)->nColumn+(I))
#पूर्ण_अगर

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
#अगर SQLITE_MAX_VARIABLE_संख्या<32767
प्रकार i16 ynVar;
#अन्यथा
प्रकार पूर्णांक ynVar;
#पूर्ण_अगर

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
काष्ठा व्यंजक {
  u8 op;                 /* Operation performed by this node */
  अक्षर aff_व्यंजक;          /* affinity, or RAISE type */
  u8 op2;                /* TK_REGISTER/TK_TRUTH: original value of Expr.op
                         ** TK_COLUMN: the value of p5 for OP_Column
                         ** TK_AGG_FUNCTION: nesting depth
                         ** TK_FUNCTION: NC_SelfRef flag if needs OP_PureFunc */
#अगर_घोषित SQLITE_DEBUG
  u8 vvaFlags;           /* Verification flags. */
#पूर्ण_अगर
  u32 flags;             /* Various flags.  EP_* See below */
  जोड़ {
    अक्षर *zToken;          /* Token value. Zero terminated and dequoted */
    पूर्णांक i_मूल्य;            /* Non-negative integer value if EP_IntValue */
  } u;

  /* If the EP_TokenOnly flag is set in the Expr.flags mask, then no
  ** space is allocated for the fields below this point. An attempt to
  ** access them will result in a segfault or malfunction.
  *********************************************************************/

  व्यंजक *p_बायाँ;           /* Left subnode */
  व्यंजक *p_दायाँ;          /* Right subnode */
  जोड़ {
    व्यंजक_List *pList;     /* op = IN, EXISTS, SELECT, CASE, FUNCTION, BETWEEN */
    Select *pSelect;     /* EP_xIsSelect and op = IN, EXISTS, SELECT */
  } x;

  /* If the EP_Reduced flag is set in the Expr.flags mask, then no
  ** space is allocated for the fields below this point. An attempt to
  ** access them will result in a segfault or malfunction.
  *********************************************************************/

#अगर SQLITE_MAX_व्यंजक_DEPTH>0
  पूर्णांक nHeight;           /* Height of the tree headed by this node */
#पूर्ण_अगर
  पूर्णांक iTable;            /* TK_COLUMN: cursor number of table holding column
                         ** TK_REGISTER: register number
                         ** TK_TRIGGER: 1 -> new, 0 -> old
                         ** EP_Unlikely:  134217728 times likelihood
                         ** TK_IN: ephemeral table holding RHS
                         ** TK_SELECT_COLUMN: Number of columns on the LHS
                         ** TK_SELECT: 1st register of result vector */
  ynVar iColumn;         /* TK_COLUMN: column index.  -1 for rowid.
                         ** TK_VARIABLE: variable number (always >= 1).
                         ** TK_SELECT_COLUMN: column of the result vector */
  i16 iAgg;              /* Which entry in pAggInfo->aCol[] or ->aFunc[] */
  जोड़ {
    पूर्णांक iJoin;             /* If EP_OuterON or EP_InnerON, the right table */
    पूर्णांक iOfst;             /* else: start of token from start of statement */
  } w;
  AggInfo *pAggInfo;     /* Used by TK_AGG_COLUMN and TK_AGG_FUNCTION */
  जोड़ {
    Table *pTab;           /* TK_COLUMN: Table containing column. Can be NULL
                           ** for a column of an index on an expression */
    Window *pWin;          /* EP_WinFunc: Window/Filter defn for a function */
    पूर्णांक nReg;              /* TK_NULLS: Number of registers to NULL out */
    काष्ठा {               /* TK_IN, TK_SELECT, and TK_EXISTS */
      पूर्णांक iAddr;             /* Subroutine entry address */
      पूर्णांक regReturn;         /* Register used to hold return address */
    } sub;
  } y;
};

/* The following are the meanings of bits in the Expr.flags field.
** Value restrictions:
**
**          EP_Agg == NC_HasAgg == SF_HasAgg
**          EP_Win == NC_HasWin
*/
#घोषणा EP_OuterON    0x000001 /* Originates in ON/USING clause of outer join */
#घोषणा EP_InnerON    0x000002 /* Originates in ON/USING of an inner join */
#घोषणा EP_Distinct   0x000004 /* Aggregate function with DISTINCT keyword */
#घोषणा EP_HasFunc    0x000008 /* Contains one or more functions of any kind */
#घोषणा EP_Agg        0x000010 /* Contains one or more aggregate functions */
#घोषणा EP_FixedCol   0x000020 /* TK_Column with a known fixed value */
#घोषणा EP_VarSelect  0x000040 /* pSelect is correlated, not constant */
#घोषणा EP_DblQuoted  0x000080 /* token.z was originally in "..." */
#घोषणा EP_InfixFunc  0x000100 /* True for an infix function: LIKE, GLOB, etc */
#घोषणा EP_Collate    0x000200 /* Tree contains a TK_COLLATE operator */
#घोषणा EP_Commuted   0x000400 /* Comparison operator has been commuted */
#घोषणा EP_Int_मूल्य   0x000800 /* Integer value contained in u.iValue */
#घोषणा EP_xIsSelect  0x001000 /* x.pSelect is valid (otherwise x.pList is) */
#घोषणा EP_Skip       0x002000 /* Operator does not contribute to affinity */
#घोषणा EP_Reduced    0x004000 /* Expr struct EXPR_REDUCEDSIZE bytes only */
#घोषणा EP_Win        0x008000 /* Contains window functions */
#घोषणा EP_TokenOnly  0x010000 /* Expr struct EXPR_TOKENONLYSIZE bytes only */
#घोषणा EP_Full_आकार   0x020000 /* Expr structure must remain full sized */
#घोषणा EP_IfNullRow  0x040000 /* The TK_IF_NULL_ROW opcode */
#घोषणा EP_Unlikely   0x080000 /* unlikely() or likelihood() function */
#घोषणा EP_ConstFunc  0x100000 /* A SQLITE_FUNC_CONSTANT or _SLOCHNG function */
#घोषणा EP_CanBeNull  0x200000 /* Can be null despite NOT NULL constraint */
#घोषणा EP_Subquery   0x400000 /* Tree contains a TK_SELECT operator */
#घोषणा EP_Leaf       0x800000 /* Expr.pLeft, .pRight, .u.pSelect all NULL */
#घोषणा EP_WinFunc   0x1000000 /* TK_FUNCTION with Expr.y.pWin set */
#घोषणा EP_Subrtn    0x2000000 /* Uses Expr.y.sub. TK_IN, _SELECT, or _EXISTS */
#घोषणा EP_Quoted    0x4000000 /* TK_ID was originally quoted */
#घोषणा EP_Static    0x8000000 /* Held in memory not obtained from malloc() */
#घोषणा EP_IsTrue   0x10000000 /* Always has boolean value of TRUE */
#घोषणा EP_IsFalse  0x20000000 /* Always has boolean value of FALSE */
#घोषणा EP_FromDDL  0x40000000 /* Originates from sqlite_schema */
#घोषणा EP_SubtArg  0x80000000 /* Is argument to SQLITE_SUBTYPE function */

/* The EP_Propagate mask is a set of properties that automatically propagate
** upwards into parent nodes.
*/
#घोषणा EP_Propagate (EP_Collate|EP_Subquery|EP_HasFunc)

/* Macros can be used to test, set, or clear bits in the
** Expr.flags field.
*/
#घोषणा व्यंजक_Has_Property(E,P)     (((E)->flags&(u32)(P))!=0)
#घोषणा व्यंजक_Has_All_Property(E,P)  (((E)->flags&(u32)(P))==(u32)(P))
#घोषणा व्यंजक_Set_Property(E,P)     (E)->flags|=(u32)(P)
#घोषणा व्यंजक_Clear_Property(E,P)   (E)->flags&=~(u32)(P)
#घोषणा व्यंजक_Always_True(E)   (((E)->flags&(EP_OuterON|EP_IsTrue))==EP_IsTrue)
#घोषणा व्यंजक_Always_False(E)  (((E)->flags&(EP_OuterON|EP_IsFalse))==EP_IsFalse)
#घोषणा व्यंजक_Is_Full_आकार(E)   (((E)->flags&(EP_Reduced|EP_TokenOnly))==0)

/* Macros used to ensure that the correct members of unions are accessed
** in Expr.
*/
#घोषणा व्यंजक_Use_UToken(E)    (((E)->flags&EP_Int_मूल्य)==0)
#घोषणा व्यंजक_Use_UValue(E)    (((E)->flags&EP_Int_मूल्य)!=0)
#घोषणा व्यंजक_Use_WOfst(E)     (((E)->flags&(EP_InnerON|EP_OuterON))==0)
#घोषणा व्यंजक_Use_WJoin(E)     (((E)->flags&(EP_InnerON|EP_OuterON))!=0)
#घोषणा व्यंजक_Use_XList(E)     (((E)->flags&EP_xIsSelect)==0)
#घोषणा व्यंजक_Use_XSelect(E)   (((E)->flags&EP_xIsSelect)!=0)
#घोषणा व्यंजक_Use_YTab(E)      (((E)->flags&(EP_WinFunc|EP_Subrtn))==0)
#घोषणा व्यंजक_Use_YWin(E)      (((E)->flags&EP_WinFunc)!=0)
#घोषणा व्यंजक_Use_YSub(E)      (((E)->flags&EP_Subrtn)!=0)

/* Flags for use with Expr.vvaFlags
*/
#घोषणा EP_NoReduce   0x01  /* Cannot EXPRDUP_REDUCE this Expr */
#घोषणा EP_Immutable  0x02  /* Do not change this Expr node */

/* The ExprSetVVAProperty() macro is used for Verification, Validation,
** and Accreditation only.  It works like ExprSetProperty() during VVA
** processes but is a no-op for delivery.
*/
#अगर_घोषित SQLITE_DEBUG
# define व्यंजक_Set_VVAProperty(E,P)   (E)->vvaFlags|=(P)
# define व्यंजक_Has_VVAProperty(E,P)   (((E)->vvaFlags&(P))!=0)
# define व्यंजक_Clear_VVAProperties(E) (E)->vvaFlags = 0
#अन्यथा
# define व्यंजक_Set_VVAProperty(E,P)
# define व्यंजक_Has_VVAProperty(E,P)   0
# define व्यंजक_Clear_VVAProperties(E)
#पूर्ण_अगर

/*
** Macros to determine the number of bytes required by a normal Expr
** struct, an Expr struct with the EP_Reduced flag set in Expr.flags
** and an Expr struct with the EP_TokenOnly flag set.
*/
#घोषणा व्यंजक_FULLSIZE           माप(व्यंजक)           /* Full size */
#घोषणा व्यंजक_REDUCEDSIZE        दुरत्व(व्यंजक,iTable)  /* Common features */
#घोषणा व्यंजक_TOKENONLYSIZE      दुरत्व(व्यंजक,p_बायाँ)   /* Fewer features */

/*
** Flags passed to the sqlite3ExprDup() function. See the header comment
** above sqlite3ExprDup() for details.
*/
#घोषणा EXPRDUP_REDUCE         0x0001  /* Used reduced-size Expr nodes */

/*
** True if the expression passed as an argument was a function with
** an OVER() clause (a window function).
*/
#अगर_घोषित SQLITE_OMIT_WINDOWFUNC
# define IsWindowFunc(p) 0
#अन्यथा
# define IsWindowFunc(p) ( \
    व्यंजक_Has_Property((p), EP_WinFunc) && p->y.pWin->eFrmType!=TK_FILTER \
 )
#पूर्ण_अगर

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
काष्ठा व्यंजक_List {
  पूर्णांक n_व्यंजक;             /* Number of expressions on the list */
  पूर्णांक nAlloc;            /* Number of a[] slots allocated */
  काष्ठा व्यंजक_List_item { /* For each expression in the list */
    व्यंजक *p_व्यंजक;            /* The parse tree for this expression */
    अक्षर *zEName;           /* Token associated with this expression */
    काष्ठा {
      u8 sortFlags;           /* Mask of KEYINFO_ORDER_* flags */
      अचिन्हित eEName :2;     /* Meaning of zEName */
      अचिन्हित done :1;       /* Indicates when processing is finished */
      अचिन्हित reusable :1;   /* Constant expression is reusable */
      अचिन्हित bSorterRef :1; /* Defer evaluation until after sorting */
      अचिन्हित bNulls :1;     /* True if explicit "NULLS FIRST/LAST" */
      अचिन्हित bUsed :1;      /* This column used in a SF_NestedFrom subquery */
      अचिन्हित bUsingTerm:1;  /* Term from the USING clause of a NestedFrom */
      अचिन्हित bNoExpand: 1;  /* Term is an auxiliary in NestedFrom and should
                              ** not be expanded by "*" in parent queries */
    } fg;
    जोड़ {
      काष्ठा {             /* Used by any ExprList other than Parse.pConsExpr */
        u16 iOrderByCol;      /* For ORDER BY, column number in result set */
        u16 iAlias;           /* Index into Parse.aAlias[] for zName */
      } x;
      पूर्णांक i_Const_व्यंजक_Reg;   /* Register in which Expr value is cached. Used only
                           ** by Parse.pConstExpr */
    } u;
  } a[FLEXARRAY];          /* One slot for each expression in the list */
};

/* The size (in bytes) of an ExprList object that is big enough to hold
** as many as N expressions. */
#घोषणा SZ_EXPRLIST(N)  \
             (दुरत्व(व्यंजक_List,a) + (N)*माप(काष्ठा व्यंजक_List_item))

/*
** Allowed values for Expr.a.eEName
*/
#घोषणा ENAME_NAME  0       /* The AS clause of a result set */
#घोषणा ENAME_SPAN  1       /* Complete text of the result set expression */
#घोषणा ENAME_TAB   2       /* "DB.TABLE.NAME" for the result set */
#घोषणा ENAME_ROWID 3       /* "DB.TABLE._rowid_" for * expansion of rowid */

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
काष्ठा IdList {
  पूर्णांक nId;         /* Number of identifiers on the list */
  काष्ठा IdList_item {
    अक्षर *zName;      /* Name of the identifier */
  } a[FLEXARRAY];
};

/* The size (in bytes) of an IdList object that can hold up to N IDs. */
#घोषणा SZ_IDLIST(N)  (दुरत्व(IdList,a)+(N)*माप(काष्ठा IdList_item))

/*
** Allowed values for IdList.eType, which determines which value of the a.u4
** is valid.
*/
#घोषणा EU4_NONE   0   /* Does not use IdList.a.u4 */
#घोषणा EU4_IDX    1   /* Uses IdList.a.u4.idx */
#घोषणा EU4_व्यंजक   2   /* Uses IdList.a.u4.pExpr -- NOT CURRENTLY USED */

/*
** Details of the implementation of a subquery.
*/
काष्ठा Subquery {
  Select *pSelect;  /* A SELECT statement used in place of a table name */
  पूर्णांक addrFillSub;  /* Address of subroutine to initialize a subquery */
  पूर्णांक regReturn;    /* Register holding return address of addrFillSub */
  पूर्णांक reg_परिणाम;    /* Registers holding results of a co-routine */
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
काष्ठा SrcItem {
  अक्षर *zName;      /* Name of the table */
  अक्षर *zAlias;     /* The "B" part of a "A AS B" phrase.  zName is the "A" */
  Table *pSTab;     /* Table object for zName. Mnemonic: Srcitem-TABle */
  काष्ठा {
    u8 jointype;      /* Type of join between this table and the previous */
    अचिन्हित notIndexed :1;    /* True if there is a NOT INDEXED clause */
    अचिन्हित isIndexedBy :1;   /* True if there is an INDEXED BY clause */
    अचिन्हित isSubquery :1;    /* True if this term is a subquery */
    अचिन्हित isTabFunc :1;     /* True if table-valued-function syntax */
    अचिन्हित isCorrelated :1;  /* True if sub-query is correlated */
    अचिन्हित isMaterialized:1; /* This is a materialized view */
    अचिन्हित viaCoroutine :1;  /* Implemented as a co-routine */
    अचिन्हित isRecursive :1;   /* True for recursive reference in WITH */
    अचिन्हित fromDDL :1;       /* Comes from sqlite_schema */
    अचिन्हित isCte :1;         /* This is a CTE */
    अचिन्हित notCte :1;        /* This item may not match a CTE */
    अचिन्हित isUsing :1;       /* u3.pUsing is valid */
    अचिन्हित isOn :1;          /* u3.pOn was once valid and non-NULL */
    अचिन्हित isSynthUsing :1;  /* u3.pUsing is synthesized from NATURAL */
    अचिन्हित isNestedFrom :1;  /* pSelect is a SF_NestedFrom subquery */
    अचिन्हित rowidUsed :1;     /* The ROWID of this table is referenced */
    अचिन्हित fixedSchema :1;   /* Uses u4.pSchema, not u4.zDatabase */
    अचिन्हित hadSchema :1;     /* Had u4.zDatabase before u4.pSchema */
    अचिन्हित fromExists :1;    /* Comes from WHERE EXISTS(...) */
  } fg;
  पूर्णांक iCursor;      /* The VDBE cursor number used to access this table */
  Bitmask colUsed;  /* Bit N set if column N used. Details above for N>62 */
  जोड़ {
    अक्षर *zIndexedBy;    /* Identifier from "INDEXED BY <zIndex>" clause */
    व्यंजक_List *pFuncArg;  /* Arguments to table-valued-function */
    u32 nRow;            /* Number of rows in a VALUES clause */
  } u1;
  जोड़ {
    सूचकांक *pIBIndex;  /* Index structure corresponding to u1.zIndexedBy */
    CteUse *pCteUse;  /* CTE Usage info when fg.isCte is true */
  } u2;
  जोड़ {
    व्यंजक *pOn;        /* fg.isUsing==0 =>  The ON clause of a join */
    IdList *pUsing;   /* fg.isUsing==1 =>  The USING clause of a join */
  } u3;
  जोड़ {
    Schema *pSchema;  /* Schema to which this item is fixed */
    अक्षर *zDatabase;  /* Name of database holding this table */
    Subquery *pSubq;  /* Description of a subquery */
  } u4;
};

/*
** The OnOrUsing object represents either an ON clause or a USING clause.
** It can never be both at the same time, but it can be neither.
*/
काष्ठा OnOrUsing {
  व्यंजक *pOn;         /* The ON clause of a join */
  IdList *pUsing;    /* The USING clause of a join */
};

/*
** This object represents one or more tables that are the source of
** content for an SQL statement.  For example, a single SrcList object
** is used to hold the FROM clause of a SELECT statement.  SrcList also
** represents the target tables for DELETE, INSERT, and UPDATE statements.
**
*/
काष्ठा SrcList {
  पूर्णांक nSrc;             /* Number of tables or subqueries in the FROM clause */
  u32 nAlloc;           /* Number of entries allocated in a[] below */
  SrcItem a[FLEXARRAY]; /* One entry for each identifier on the list */
};

/* Size (in bytes) of a SrcList object that can hold as many as N
** SrcItem objects. */
#घोषणा SZ_SRCLIST(N) (दुरत्व(SrcList,a)+(N)*माप(SrcItem))

/* Size (in bytes( of a SrcList object that holds 1 SrcItem.  This is a
** special case of SZ_SRCITEM(1) that comes up often. */
#घोषणा SZ_SRCLIST_1  (दुरत्व(SrcList,a)+माप(SrcItem))

/*
** Permitted values of the SrcList.a.jointype field
*/
#घोषणा JT_INNER     0x01    /* Any kind of inner or cross join */
#घोषणा JT_CROSS     0x02    /* Explicit use of the CROSS keyword */
#घोषणा JT_NATURAL   0x04    /* True for a "natural" join */
#घोषणा JT_बायाँ      0x08    /* Left outer join */
#घोषणा JT_दायाँ     0x10    /* Right outer join */
#घोषणा JT_OUTER     0x20    /* The "OUTER" keyword is present */
#घोषणा JT_LTORJ     0x40    /* One of the LEFT operands of a RIGHT JOIN
                             ** Mnemonic: Left Table Of Right Join */
#घोषणा JT_ERROR     0x80    /* unknown or unsupported join type */

/*
** Flags appropriate for the wctrlFlags parameter of sqlite3WhereBegin()
** and the WhereInfo.wctrlFlags member.
**
** Value constraints (enforced via assert()):
**     WHERE_USE_LIMIT  == SF_FixedLimit
*/
#घोषणा WHERE_ORDERBY_NORMAL   0x0000 /* No-op */
#घोषणा WHERE_ORDERBY_MIN      0x0001 /* ORDER BY processing for min() func */
#घोषणा WHERE_ORDERBY_MAX      0x0002 /* ORDER BY processing for max() func */
#घोषणा WHERE_ONEPASS_DESIRED  0x0004 /* Want to do one-pass UPDATE/DELETE */
#घोषणा WHERE_ONEPASS_MULTIROW 0x0008 /* ONEPASS is ok with multiple rows */
#घोषणा WHERE_DUPLICATES_OK    0x0010 /* Ok to return a row more than once */
#घोषणा WHERE_OR_SUBCLAUSE     0x0020 /* Processing a sub-WHERE as part of
                                      ** the OR optimization  */
#घोषणा WHERE_GROUPBY          0x0040 /* pOrderBy is really a GROUP BY */
#घोषणा WHERE_DISTINCTBY       0x0080 /* pOrderby is really a DISTINCT clause */
#घोषणा WHERE_WANT_DISTINCT    0x0100 /* All output needs to be distinct */
#घोषणा WHERE_SORTBYGROUP      0x0200 /* Support sqlite3WhereIsSorted() */
#घोषणा WHERE_AGG_DISTINCT     0x0400 /* Query is "SELECT agg(DISTINCT ...)" */
#घोषणा WHERE_ORDERBY_LIMIT    0x0800 /* ORDERBY+LIMIT on the inner loop */
#घोषणा WHERE_दायाँ_JOIN       0x1000 /* Processing a RIGHT JOIN */
#घोषणा WHERE_KEEP_ALL_JOINS   0x2000 /* Do not do the omit-noop-join opt */
#घोषणा WHERE_USE_LIMIT        0x4000 /* Use the LIMIT in cost estimates */
                        /*     0x8000    not currently used */

/* Allowed return values from sqlite3WhereIsDistinct()
*/
#घोषणा WHERE_DISTINCT_NOOP      0  /* DISTINCT keyword not used */
#घोषणा WHERE_DISTINCT_UNIQUE    1  /* No duplicates */
#घोषणा WHERE_DISTINCT_ORDERED   2  /* All duplicates are adjacent */
#घोषणा WHERE_DISTINCT_UNORDERED 3  /* Duplicates are scattered */

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
काष्ठा NameContext {
  Parse *pParse;       /* The parser */
  SrcList *pSrcList;   /* One or more tables used to resolve names */
  जोड़ {
    व्यंजक_List *pEList;    /* Optional list of result-set columns */
    AggInfo *pAggInfo;   /* Information about aggregates at this level */
    Upsert *pUpsert;     /* ON CONFLICT clause information from an upsert */
    पूर्णांक iBaseReg;        /* For TK_REGISTER when parsing RETURNING */
  } uNC;
  NameContext *p_अग्र;  /* Next outer name context.  NULL for outermost */
  पूर्णांक nRef;            /* Number of names resolved by this context */
  पूर्णांक nNcErr;          /* Number of errors encountered while resolving names */
  पूर्णांक ncFlags;         /* Zero or more NC_* flags defined below */
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
#घोषणा NC_AllowAgg  0x000001 /* Aggregate functions are allowed here */
#घोषणा NC_PartIdx   0x000002 /* True if resolving a partial index WHERE */
#घोषणा NC_IsCheck   0x000004 /* True if resolving a CHECK constraint */
#घोषणा NC_GenCol    0x000008 /* True for a GENERATED ALWAYS AS clause */
#घोषणा NC_HasAgg    0x000010 /* One or more aggregate functions seen */
#घोषणा NC_Idx_व्यंजक   0x000020 /* True if resolving columns of CREATE INDEX */
#घोषणा NC_SelfRef   0x00002e /* Combo: PartIdx, isCheck, GenCol, and IdxExpr */
#घोषणा NC_Subquery  0x000040 /* A subquery has been seen */
#घोषणा NC_UEList    0x000080 /* True if uNC.pEList is used */
#घोषणा NC_UAggInfo  0x000100 /* True if uNC.pAggInfo is used */
#घोषणा NC_UUpsert   0x000200 /* True if uNC.pUpsert is used */
#घोषणा NC_UBaseReg  0x000400 /* True if uNC.iBaseReg is used */
#घोषणा NC_MinMaxAgg 0x001000 /* min/max aggregates seen.  See note above */
/*                   0x002000 // available for reuse */
#घोषणा NC_AllowWin  0x004000 /* Window functions are allowed here */
#घोषणा NC_HasWin    0x008000 /* One or more window functions seen */
#घोषणा NC_IsDDL     0x010000 /* Resolving names in a CREATE statement */
#घोषणा NC_InAggFunc 0x020000 /* True if analyzing arguments to an agg func */
#घोषणा NC_FromDDL   0x040000 /* SQL text comes from sqlite_schema */
#घोषणा NC_NoSelect  0x080000 /* Do not descend into sub-selects */
#घोषणा NC_Where     0x100000 /* Processing WHERE clause of a SELECT */
#घोषणा NC_OrderAgg 0x8000000 /* Has an aggregate other than count/min/max */

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
काष्ठा Upsert {
  व्यंजक_List *pUpsertTarget;  /* Optional description of conflict target */
  व्यंजक *pUpsertTargetWhere; /* WHERE clause for partial index targets */
  व्यंजक_List *pUpsertSet;     /* The SET clause from an ON CONFLICT UPDATE */
  व्यंजक *pUpsertWhere;       /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_अग्र_Upsert;      /* Next ON CONFLICT clause in the list */
  u8 isDoUpdate;            /* True for DO UPDATE.  False for DO NOTHING */
  u8 isDup;                 /* True if 2nd or later with same pUpsertIdx */
  /* Above this point is the parse tree for the ON CONFLICT clauses.
  ** The next group of fields stores intermediate data. */
  व्योम *pToFree;            /* Free memory when deleting the Upsert object */
  /* All fields above are owned by the Upsert object and must be freed
  ** when the Upsert is destroyed.  The fields below are used to transfer
  ** information from the INSERT processing down into the UPDATE processing
  ** while generating code.  The fields below are owned by the INSERT
  ** statement and will be freed by INSERT processing. */
  सूचकांक *pUpsertIdx;        /* UNIQUE constraint specified by pUpsertTarget */
  SrcList *pUpsertSrc;      /* Table to be updated */
  पूर्णांक reg_तथ्य;              /* First register holding array of VALUES */
  पूर्णांक i_तथ्य_Cur;             /* Index of the data cursor */
  पूर्णांक iIdxCur;              /* Index of the first index cursor */
};

/*
** An instance of the following structure contains all information
** needed to generate code for a single SELECT statement.
*/
काष्ठा Select {
  u8 op;                 /* One of: TK_UNION TK_ALL TK_INTERSECT TK_EXCEPT */
  LogEst nSelectRow;     /* Estimated number of result rows */
  u32 selFlags;          /* Various SF_* values */
  पूर्णांक iLimit, iOffset;   /* Memory registers holding LIMIT & OFFSET counters */
  u32 selId;             /* Unique identifier number for this SELECT */
  व्यंजक_List *pEList;      /* The fields of the result */
  SrcList *pSrc;         /* The FROM clause */
  व्यंजक *pWhere;          /* The WHERE clause */
  व्यंजक_List *pGroupBy;    /* The GROUP BY clause */
  व्यंजक *pHaving;         /* The HAVING clause */
  व्यंजक_List *pOrderBy;    /* The ORDER BY clause */
  Select *pPrior;        /* Prior select in a compound select statement */
  Select *p_अग्र;         /* Next select to the left in a compound */
  व्यंजक *pLimit;          /* LIMIT expression. NULL means not used. */
  With *pWith;           /* WITH clause attached to this select. Or NULL. */
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
  Window *pWin;          /* List of window functions */
  Window *pWinDefn;      /* List of named window definitions */
#पूर्ण_अगर
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
#घोषणा SF_Distinct      0x0000001 /* Output should be DISTINCT */
#घोषणा SF_All           0x0000002 /* Includes the ALL keyword */
#घोषणा SF_Resolved      0x0000004 /* Identifiers have been resolved */
#घोषणा SF_Aggregate     0x0000008 /* Contains agg functions or a GROUP BY */
#घोषणा SF_HasAgg        0x0000010 /* Contains aggregate functions */
#घोषणा SF_ClonedRhsIn   0x0000020 /* Cloned RHS of an IN operator */
#घोषणा SF_Expanded      0x0000040 /* sqlite3SelectExpand() called on this */
#घोषणा SF_HasTypeInfo   0x0000080 /* FROM subqueries have Table metadata */
#घोषणा SF_Compound      0x0000100 /* Part of a compound query */
#घोषणा SF_मूल्यावली        0x0000200 /* Synthesized from VALUES clause */
#घोषणा SF_Multi_मूल्य    0x0000400 /* Single VALUES term with multiple rows */
#घोषणा SF_NestedFrom    0x0000800 /* Part of a parenthesized FROM clause */
#घोषणा SF_MinMaxAgg     0x0001000 /* Aggregate containing min() or max() */
#घोषणा SF_Recursive     0x0002000 /* The recursive part of a recursive CTE */
#घोषणा SF_FixedLimit    0x0004000 /* nSelectRow set by a constant LIMIT */
/*                       0x0008000 // available for reuse */
#घोषणा SF_Converted     0x0010000 /* By convertCompoundSelectToSubquery() */
#घोषणा SF_IncludeHidden 0x0020000 /* Include hidden columns in output */
#घोषणा SF_Complex_परिणाम 0x0040000 /* Result contains subquery or function */
#घोषणा SF_WhereBegin    0x0080000 /* Really a WhereBegin() call.  Debug Only */
#घोषणा SF_WinRewrite    0x0100000 /* Window function rewrite accomplished */
#घोषणा SF_View          0x0200000 /* SELECT statement is a view */
/*                       0x0400000 // available for reuse */
#घोषणा SF_UFSrcCheck    0x0800000 /* Check pSrc as required by UPDATE...FROM */
#घोषणा SF_धकेलो_Down      0x1000000 /* Modified by WHERE-clause push-down opt */
#घोषणा SF_MultiPart     0x2000000 /* Has multiple incompatible PARTITIONs */
#घोषणा SF_CopyCte       0x4000000 /* SELECT statement is a copy of a CTE */
#घोषणा SF_OrderByReqd   0x8000000 /* The ORDER BY clause may not be omitted */
#घोषणा SF_UpdateFrom   0x10000000 /* Query originates with UPDATE FROM */
#घोषणा SF_Correlated   0x20000000 /* True if references the outer context */
#घोषणा SF_OnToWhere    0x40000000 /* One or more ON clauses moved to WHERE */

/* True if SrcItem X is a subquery that has SF_NestedFrom */
#घोषणा IsNestedFrom(X) \
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
#घोषणा SRT_Exists       1  /* Store 1 if the result is not empty */
#घोषणा SRT_Discard      2  /* Do not save the results anywhere */
#घोषणा SRT_DistFifo     3  /* Like SRT_Fifo, but unique results only */
#घोषणा SRT_DistQueue    4  /* Like SRT_Queue, but unique results only */

/* The DISTINCT clause is ignored for all of the above.  Not that
** IgnorableDistinct() implies IgnorableOrderby() */
#घोषणा IgnorableDistinct(X) ((X->eDest)<=SRT_DistQueue)

#घोषणा SRT_Queue        5  /* Store result in an queue */
#घोषणा SRT_Fifo         6  /* Store result as data with an automatic rowid */

/* The ORDER BY clause is ignored for all of the above */
#घोषणा IgnorableOrderby(X) ((X->eDest)<=SRT_Fifo)

#घोषणा SRT_Output       7  /* Output each row of result */
#घोषणा SRT_Mem          8  /* Store result in a memory cell */
#घोषणा SRT_Set          9  /* Store results as keys in an index */
#घोषणा SRT_EphemTab    10  /* Create transient tab and store like SRT_Table */
#घोषणा SRT_Coroutine   11  /* Generate a single row of result */
#घोषणा SRT_Table       12  /* Store result as data with an automatic rowid */
#घोषणा SRT_Upfrom      13  /* Store result as data with rowid */

/*
** An instance of this object describes where to put of the results of
** a SELECT statement.
*/
काष्ठा SelectDest {
  u8 eDest;            /* How to dispose of the results.  One of SRT_* above. */
  पूर्णांक iSDParm;         /* A parameter used by the eDest disposal method */
  पूर्णांक iSDParm2;        /* A second parameter for the eDest disposal method */
  पूर्णांक iSdst;           /* Base register where results are written */
  पूर्णांक nSdst;           /* Number of registers allocated */
  अक्षर *zAffSdst;      /* Affinity used for SRT_Set */
  व्यंजक_List *pOrderBy;  /* Key columns for SRT_Queue and SRT_DistQueue */
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
काष्ठा AutoincInfo {
  AutoincInfo *p_अग्र;   /* Next info block in a list of them all */
  Table *pTab;          /* Table this info block refers to */
  पूर्णांक iDb;              /* Index in sqlite3.aDb[] of database holding pTab */
  पूर्णांक regCtr;           /* Memory register holding the rowid counter */
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
काष्ठा TriggerPrg {
  Trigger *pTrigger;      /* Trigger this program was coded from */
  TriggerPrg *p_अग्र;      /* Next entry in Parse.pTriggerPrg list */
  SubProgram *pProgram;   /* Program implementing pTrigger/orconf */
  पूर्णांक orconf;             /* Default ON CONFLICT policy */
  u32 aColmask[2];        /* Masks of old.*, new.* columns accessed */
};

/*
** The yDbMask datatype for the bitmask of all attached databases.
*/
#अगर SQLITE_MAX_ATTACHED>30
  प्रकार अचिन्हित अक्षर yDbMask[(SQLITE_MAX_ATTACHED+9)/8];
# define DbMaskTest(M,I)    (((M)[(I)/8]&(1<<((I)&7)))!=0)
# define DbMaskZero(M)      स_रखो((M),0,माप(M))
# define DbMaskSet(M,I)     (M)[(I)/8]|=(1<<((I)&7))
# define DbMaskAllZero(M)   sqlite3DbMaskAllZero(M)
# define DbMaskNonZero(M)   (sqlite3DbMaskAllZero(M)==0)
#अन्यथा
  प्रकार अचिन्हित पूर्णांक yDbMask;
# define DbMaskTest(M,I)    (((M)&(((yDbMask)1)<<(I)))!=0)
# define DbMaskZero(M)      ((M)=0)
# define DbMaskSet(M,I)     ((M)|=(((yDbMask)1)<<(I)))
# define DbMaskAllZero(M)   ((M)==0)
# define DbMaskNonZero(M)   ((M)!=0)
#पूर्ण_अगर

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
काष्ठा Indexed_व्यंजक {
  व्यंजक *p_व्यंजक;            /* The expression contained in the index */
  पूर्णांक i_तथ्य_Cur;           /* The data cursor associated with the index */
  पूर्णांक iIdxCur;            /* The index cursor */
  पूर्णांक iIdxCol;            /* The index column that contains value of pExpr */
  u8 bMaybeNullRow;       /* True if we need an OP_IfNullRow check */
  u8 aff;                 /* Affinity of the pExpr expression */
  Indexed_व्यंजक *pIENext;   /* Next in a list of all indexed expressions */
#अगर_घोषित SQLITE_ENABLE_EXPLAIN_COMMENTS
  स्थिर अक्षर *zIdxName;   /* Name of index, used only for bytecode comments */
#पूर्ण_अगर
};

/*
** An instance of the ParseCleanup object specifies an operation that
** should be performed after parsing to deallocation resources obtained
** during the parse and which are no longer needed.
*/
काष्ठा ParseCleanup {
  ParseCleanup *p_अग्र;               /* Next cleanup task */
  व्योम *pPtr;                        /* Pointer to object to deallocate */
  व्योम (*xCleanup)(sqlite3*,व्योम*);  /* Deallocation routine */
};

/*
** Number of 64-bit entried in the variable number bitmap cache (VNBMC).
*/
#अगर_अघोषित SQLITE_VNBMC
# define SQLITE_VNBMC 2
#पूर्ण_अगर

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
काष्ठा Parse {
  sqlite3 *db;         /* The main database structure */
  अक्षर *z_Err_संदेश;       /* An error message */
  Vdbe *pVdbe;         /* An engine for executing database bytecode */
  पूर्णांक rc;              /* Return code from execution */
  LogEst nQueryLoop;   /* Est number of iterations of a query (10*log2(N)) */
  u8 nested;           /* Number of nested calls to the parser/code generator */
  u8 n_अस्थायी_Reg;         /* Number of temporary registers in aTempReg[] */
  u8 isMultiWrite;     /* True if statement may modify/insert multiple rows */
  u8 disableLookaside; /* Number of times lookaside has been disabled */
  u8 prepFlags;        /* SQLITE_PREPARE_* flags */
  u8 withinRJSubrtn;   /* Nesting level for RIGHT JOIN body subroutines */
  u8 mSubrtnSig;       /* mini Bloom filter on available SubrtnSig.selId */
  u8 eTriggerOp;       /* TK_UPDATE, TK_INSERT or TK_DELETE */
  u8 eOrconf;          /* Default ON CONFLICT policy for trigger steps */
#अगर defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
  u8 earlyCleanup;     /* OOM inside sqlite3ParserAddCleanup() */
#पूर्ण_अगर
#अगर_घोषित SQLITE_DEBUG
  u8 ifNotExists;      /* Might be true if IF NOT EXISTS.  Assert()s only */
  u8 isCreate;         /* CREATE TABLE, INDEX, or VIEW (but not TRIGGER)
                       ** and ALTER TABLE ADD COLUMN. */
#पूर्ण_अगर
  bft disableTriggers:1; /* True to disable triggers */
  bft mayAbort :1;     /* True if statement may throw an ABORT exception */
  bft hasCompound :1;  /* Need to invoke convertCompoundSelectToSubquery() */
  bft bReturning :1;   /* Coding a RETURNING trigger */
  bft bHasExists :1;   /* Has a correlated "EXISTS (SELECT ....)" expression */
  bft colNamesSet :1;  /* TRUE after OP_ColumnName has been issued to pVdbe */
  bft bHasWith :1;     /* True if statement contains WITH */
  bft okConstFactor:1; /* OK to factor out constants */
  bft checkSchema :1;  /* Causes schema cookie check after an error */
  bft usesAinc :1;     /* True if pAinc is valid */
  पूर्णांक nRangeReg;       /* Size of the temporary register block */
  पूर्णांक iRangeReg;       /* First register in temporary register block */
  पूर्णांक nErr;            /* Number of errors seen */
  पूर्णांक nTab;            /* Number of previously allocated VDBE cursors */
  पूर्णांक nMem;            /* Number of memory cells used so far */
  पूर्णांक szOpAlloc;       /* Bytes of memory space allocated for Vdbe.aOp[] */
  पूर्णांक iSelfTab;        /* Table associated with an index on expr, or negative
                       ** of the base register during check-constraint eval */
  पूर्णांक nNestSel;        /* Number of nested SELECT statements and/or VIEWs */
  पूर्णांक nLabel;          /* The *negative* of the number of labels used */
  पूर्णांक nLabelAlloc;     /* Number of slots in aLabel */
  पूर्णांक *aLabel;         /* Space to hold the labels */
  व्यंजक_List *p_Const_व्यंजक;/* Constant expressions */
  Indexed_व्यंजक *pIdxEpr;/* List of expressions used by active indexes */
  Indexed_व्यंजक *p_Idx_Part_व्यंजक; /* Exprs constrained by index WHERE clauses */
  yDbMask writeMask;   /* Start a write transaction on these databases */
  yDbMask cookieMask;  /* Bitmask of schema verified databases */
  पूर्णांक nMaxArg;         /* Max args to xUpdate and xFilter vtab methods */
  पूर्णांक nSelect;         /* Number of SELECT stmts. Counter for Select.selId */
#अगर_अघोषित SQLITE_OMIT_PROGRESS_CALLBACK
  u32 nProgressSteps;  /* xProgress steps taken during sqlite3_prepare() */
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_SHARED_CACHE
  पूर्णांक nTableLock;        /* Number of locks in aTableLock */
#पूर्ण_अगर
  Parse *pToplevel;    /* Parse structure for main program (or NULL) */
  Table *pTriggerTab;  /* Table triggers are being coded for */
  TriggerPrg *pTriggerPrg;  /* Linked list of coded triggers */
  ParseCleanup *pCleanup;   /* List of cleanup operations to run after parse */
  Trigger *pTrigCoded;      /* Trigger whose sub-program is being coded */

  /**************************************************************************
  ** Fields above must be initialized to zero.  The fields that follow,
  ** down to the beginning of the recursive section, do not need to be
  ** initialized as they will be set before being used.  The boundary is
  ** determined by offsetof(Parse,aTempReg).
  **************************************************************************/

  पूर्णांक a_अस्थायी_Reg[8];        /* Holding area for temporary registers */
  Parse *pOuterParse;     /* Outer Parse object when nested */
  Token sNameToken;       /* Token with unqualified schema object name */
  u32 oldmask;            /* Mask of old.* columns referenced */
  u32 newmask;            /* Mask of new.* columns referenced */
  जोड़ {
    काष्ठा {  /* These fields available when isCreate is true */
      पूर्णांक addrCrTab;        /* Address of OP_CreateBtree on CREATE TABLE */
      पूर्णांक regRowid;         /* Register holding rowid of CREATE TABLE entry */
      पूर्णांक reg_मूल;          /* Register holding root page for new objects */
      Token constraintName; /* Name of the constraint currently being parsed */
    } cr;
    काष्ठा {  /* These fields available to all other statements */
      Returning *pReturning; /* The RETURNING clause */
    } d;
  } u1;
  AutoincInfo *pAinc;     /* Information about AUTOINCREMENT counters. Only
                          ** valid if Parse.usesAinc is true */
#अगर_अघोषित SQLITE_OMIT_SHARED_CACHE
  TableLock *aTableLock;  /* Required table locks for shared-cache mode. Only
                          ** valid if Parse.nTableLock>0 */
#पूर्ण_अगर


  /************************************************************************
  ** Above is constant between recursions.  Below is reset before and after
  ** each recursion.  The boundary between these two regions is determined
  ** using offsetof(Parse,sLastToken) so the sLastToken field must be the
  ** first field in the recursive region.
  ************************************************************************/

  Token sLastToken;       /* The last token parsed */
  ynVar nVar;               /* Number of '?' variables seen in the SQL so far */
  u64 aVnbmc[SQLITE_VNBMC]; /* Varible Number Bitmap Cache */
  u8 iPkSortOrder;          /* ASC or DESC for INTEGER PRIMARY KEY */
  u8 explain;               /* True if the EXPLAIN flag is found on the query */
  u8 eParseMode;            /* PARSE_MODE_XXX constant */
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  पूर्णांक nVtabLock;            /* Number of virtual tables to lock */
#पूर्ण_अगर
  पूर्णांक nHeight;              /* Expression tree height of current sub-select */
  पूर्णांक addrExplain;          /* Address of current OP_Explain opcode */
  VList *pVList;            /* Mapping between variable names and numbers */
  Vdbe *pReprepare;         /* VM being reprepared (sqlite3Reprepare()) */
  स्थिर अक्षर *z_पुच्छ;        /* All SQL text past the last semicolon parsed */
  Table *pNewTable;         /* A table being constructed by CREATE TABLE */
  सूचकांक *p_New_सूचकांक;         /* An index being constructed by CREATE INDEX.
                            ** Also used to hold redundant UNIQUE constraints
                            ** during a RENAME COLUMN */
  Trigger *pNewTrigger;     /* Trigger under construct by a CREATE TRIGGER */
  स्थिर अक्षर *zAuthContext; /* The 6th parameter to db->xAuth callbacks */
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  Token sArg;               /* Complete text of a module argument */
  Table **apVtabLock;       /* Pointer to virtual tables needing locking */
#पूर्ण_अगर
  With *pWith;              /* Current WITH clause, or NULL */
#अगर_अघोषित SQLITE_OMIT_ALTERTABLE
  RenameToken *pRename;     /* Tokens subject to renaming by ALTER TABLE */
#पूर्ण_अगर
};

/* Allowed values for Parse.eParseMode
*/
#घोषणा PARSE_MODE_NORMAL        0
#घोषणा PARSE_MODE_DECLARE_VTAB  1
#घोषणा PARSE_MODE_RENAME        2
#घोषणा PARSE_MODE_UNMAP         3

/*
** Sizes and pointers of various parts of the Parse object.
*/
#घोषणा PARSE_HDR(X)  (((अक्षर*)(X))+दुरत्व(Parse,z_Err_संदेश))
#घोषणा PARSE_HDR_SZ (दुरत्व(Parse,a_अस्थायी_Reg)-दुरत्व(Parse,z_Err_संदेश)) /* Recursive part w/o aColCache*/
#घोषणा PARSE_RECURSE_SZ दुरत्व(Parse,sLastToken)    /* Recursive part */
#घोषणा PARSE_पुच्छ_SZ (माप(Parse)-PARSE_RECURSE_SZ) /* Non-recursive part */
#घोषणा PARSE_पुच्छ(X) (((अक्षर*)(X))+PARSE_RECURSE_SZ)  /* Pointer to tail */

/*
** Return true if currently inside an sqlite3_declare_vtab() call.
*/
#अगर_घोषित SQLITE_OMIT_VIRTUALTABLE
  #घोषणा IN_DECLARE_VTAB 0
#अन्यथा
  #घोषणा IN_DECLARE_VTAB (pParse->eParseMode==PARSE_MODE_DECLARE_VTAB)
#पूर्ण_अगर

#अगर defined(SQLITE_OMIT_ALTERTABLE)
  #घोषणा IN_RENAME_OBJECT 0
#अन्यथा
  #घोषणा IN_RENAME_OBJECT (pParse->eParseMode>=PARSE_MODE_RENAME)
#पूर्ण_अगर

#अगर defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_OMIT_ALTERTABLE)
  #घोषणा IN_SPECIAL_PARSE 0
#अन्यथा
  #घोषणा IN_SPECIAL_PARSE (pParse->eParseMode!=PARSE_MODE_NORMAL)
#पूर्ण_अगर

/*
** An instance of the following structure can be declared on a stack and used
** to save the Parse.zAuthContext value so that it can be restored later.
*/
काष्ठा AuthContext {
  स्थिर अक्षर *zAuthContext;   /* Put saved Parse.zAuthContext here */
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
#घोषणा OPFLAG_NCHANGE       0x01    /* OP_Insert: Set to update db->nChange */
                                     /* Also used in P2 (not P5) of OP_Delete */
#घोषणा OPFLAG_NOCHNG        0x01    /* OP_VColumn nochange for UPDATE */
#घोषणा OPFLAG_EPHEM         0x01    /* OP_Column: Ephemeral output is ok */
#घोषणा OPFLAG_LASTROWID     0x20    /* Set to update db->lastRowid */
#घोषणा OPFLAG_ISUPDATE      0x04    /* This OP_Insert is an sql UPDATE */
#घोषणा OPFLAG_APPEND        0x08    /* This is likely to be an append */
#घोषणा OPFLAG_USESEEKRESULT 0x10    /* Try to avoid a seek in BtreeInsert() */
#घोषणा OPFLAG_ISNOOP        0x40    /* OP_Delete does pre-update-hook only */
#घोषणा OPFLAG_LENGTHARG     0x40    /* OP_Column only used for length() */
#घोषणा OPFLAG_TYPEOFARG     0x80    /* OP_Column only used for typeof() */
#घोषणा OPFLAG_BYTELENARG    0xc0    /* OP_Column only for octet_length() */
#घोषणा OPFLAG_BULKCSR       0x01    /* OP_Open** used to open bulk cursor */
#घोषणा OPFLAG_SEEKEQ        0x02    /* OP_Open** cursor uses EQ seek only */
#घोषणा OPFLAG_FORDELETE     0x08    /* OP_Open should use BTREE_FORDELETE */
#घोषणा OPFLAG_P2ISREG       0x10    /* P2 to OP_Open** is a register number */
#घोषणा OPFLAG_PERMUTE       0x01    /* OP_Compare: use the permutation */
#घोषणा OPFLAG_SAVEPOSITION  0x02    /* OP_Delete/Insert: save cursor pos */
#घोषणा OPFLAG_AUXDELETE     0x04    /* OP_Delete: index in a DELETE op */
#घोषणा OPFLAG_NOCHNG_MAGIC  0x6d    /* OP_MakeRecord: serialtype 10 is ok */
#घोषणा OPFLAG_PREFORMAT     0x80    /* OP_Insert uses preformatted cell */

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
काष्ठा Trigger {
  अक्षर *zName;            /* The name of the trigger                        */
  अक्षर *table;            /* The table or view to which the trigger applies */
  u8 op;                  /* One of TK_DELETE, TK_UPDATE, TK_INSERT         */
  u8 tr_tm;               /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  u8 bReturning;          /* This trigger implements a RETURNING clause */
  व्यंजक *pWhen;            /* The WHEN clause of the expression (may be NULL) */
  IdList *pColumns;       /* If this is an UPDATE OF <column-list> trigger,
                             the <column-list> is stored here */
  Schema *pSchema;        /* Schema containing the trigger */
  Schema *pTabSchema;     /* Schema containing the table */
  TriggerStep *step_list; /* Link list of trigger program steps             */
  Trigger *p_अग्र;         /* Next trigger associated with the table */
};

/*
** A trigger is either a BEFORE or an AFTER trigger.  The following constants
** determine which.
**
** If there are multiple triggers, you might of some BEFORE and some AFTER.
** In that cases, the constants below can be ORed together.
*/
#घोषणा TRIGGER_BEFORE  1
#घोषणा TRIGGER_AFTER   2

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
काष्ठा TriggerStep {
  u8 op;               /* One of TK_DELETE, TK_UPDATE, TK_INSERT, TK_SELECT,
                       ** or TK_RETURNING */
  u8 orconf;           /* OE_Rollback etc. */
  Trigger *pTrig;      /* The trigger that this step is a part of */
  Select *pSelect;     /* SELECT statement or RHS of INSERT INTO SELECT ... */
  SrcList *pSrc;       /* Table to insert/update/delete */
  व्यंजक *pWhere;        /* The WHERE clause for DELETE or UPDATE steps */
  व्यंजक_List *p_व्यंजक_List; /* SET clause for UPDATE, or RETURNING clause */
  IdList *pIdList;     /* Column names for INSERT */
  Upsert *pUpsert;     /* Upsert clauses on an INSERT */
  अक्षर *zSpan;         /* Original SQL text of this command */
  TriggerStep *p_अग्र;  /* Next in the link-list */
  TriggerStep *pLast;  /* Last element in link-list. Valid for 1st elem only */
};

/*
** Information about a RETURNING clause
*/
काष्ठा Returning {
  Parse *pParse;        /* The parse that includes the RETURNING clause */
  व्यंजक_List *pReturnEL;  /* List of expressions to return */
  Trigger retTrig;      /* The transient trigger that implements RETURNING */
  TriggerStep retTStep; /* The trigger step */
  पूर्णांक iRetCur;          /* Transient table holding RETURNING results */
  पूर्णांक nRetCol;          /* Number of in pReturnEL after expansion */
  पूर्णांक iRetReg;          /* Register array for holding a row of RETURNING */
  अक्षर zName[40];       /* Name of trigger: "sqlite_returning_%p" */
};

/*
** An object used to accumulate the text of a string where we
** do not necessarily know how big the string will be in the end.
*/
काष्ठा sqlite3_str {
  sqlite3 *db;         /* Optional database for lookaside.  Can be NULL */
  अक्षर *zText;         /* The string collected so far */
  u32  nAlloc;         /* Amount of space allocated in zText */
  u32  mxAlloc;        /* Maximum allowed allocation.  0 for no malloc usage */
  u32  nChar;          /* Length of the string so far */
  u8   accError;       /* SQLITE_NOMEM or SQLITE_TOOBIG */
  u8   printfFlags;    /* SQLITE_PRINTF flags below */
};
#घोषणा SQLITE_PRINTF_INTERNAL 0x01  /* Internal-use-only converters allowed */
#घोषणा SQLITE_PRINTF_SQLFUNC  0x02  /* SQL function arguments to VXPrintf */
#घोषणा SQLITE_PRINTF_MALLOCED 0x04  /* True if zText is allocated space */

#घोषणा isMalloced(X)  (((X)->printfFlags & SQLITE_PRINTF_MALLOCED)!=0)

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
काष्ठा RCStr {
  u64 nRCRef;            /* Number of references */
  /* Total structure size should be a multiple of 8 bytes for alignment */
};

/*
** A pointer to this structure is used to communicate information
** from sqlite3Init and OP_ParseSchema into the sqlite3InitCallback.
*/
प्रकार काष्ठा {
  sqlite3 *db;        /* The database being initialized */
  अक्षर **pz_Err_संदेश;    /* Error message stored here */
  पूर्णांक iDb;            /* 0 for main database.  1 for TEMP, 2.. for ATTACHed */
  पूर्णांक rc;             /* Result code stored here */
  u32 mInitFlags;     /* Flags controlling error messages */
  u32 nInitRow;       /* Number of rows processed */
  Pgno mxPage;        /* Maximum page number.  0 for no limit. */
} Init_तथ्य;

/*
** Allowed values for mInitFlags
*/
#घोषणा INITFLAG_AlterMask     0x0007  /* Types of ALTER */
#घोषणा INITFLAG_AlterRename   0x0001  /* Reparse after a RENAME */
#घोषणा INITFLAG_AlterDrop     0x0002  /* Reparse after a DROP COLUMN */
#घोषणा INITFLAG_AlterAdd      0x0003  /* Reparse after an ADD COLUMN */
#घोषणा INITFLAG_AlterDropCons 0x0004  /* Reparse after a DROP CONSTRAINT */

/* Tuning parameters are set using SQLITE_TESTCTRL_TUNE and are controlled
** on debug-builds of the CLI using ".testctrl tune ID VALUE".  Tuning
** parameters are for temporary use during development, to help find
** optimal values for parameters in the query planner.  The should not
** be used on trunk check-ins.  They are a temporary mechanism available
** for transient development builds only.
**
** Tuning parameters are numbered starting with 1.
*/
#घोषणा SQLITE_NTUNE  6             /* Should be zero for all trunk check-ins */
#अगर_घोषित SQLITE_DEBUG
# define Tuning(X)  (sqlite3Config.aTune[(X)-1])
#अन्यथा
# define Tuning(X)  0
#पूर्ण_अगर

/*
** Structure containing global configuration data for the SQLite library.
**
** This structure also contains some state information.
*/
काष्ठा Sqlite3Config {
  पूर्णांक bMemstat;                     /* True to enable memory status */
  u8 bCoreMutex;                    /* True to enable core mutexing */
  u8 bFullMutex;                    /* True to enable full mutexing */
  u8 bOpenUri;                      /* True to interpret filenames as URIs */
  u8 bUseCis;                       /* Use covering indices for full-scans */
  u8 bSmallMalloc;                  /* Avoid large memory allocations if true */
  u8 bExtraSchemaChecks;            /* Verify type,name,tbl_name in schema */
#अगर_घोषित SQLITE_DEBUG
  u8 bJsonSelfcheck;                /* Double-check JSON parsing */
#पूर्ण_अगर
  पूर्णांक mxStrlen;                     /* Maximum string length */
  पूर्णांक neverCorrupt;                 /* Database is always well-formed */
  पूर्णांक szLookaside;                  /* Default lookaside buffer size */
  पूर्णांक nLookaside;                   /* Default lookaside buffer count */
  पूर्णांक nStmtSpill;                   /* Stmt-journal spill-to-disk threshold */
  sqlite3_mem_methods m;            /* Low-level memory allocation interface */
  sqlite3_mutex_methods mutex;      /* Low-level mutex interface */
  sqlite3_pcache_methods2 pcache2;  /* Low-level page-cache interface */
  व्योम *pHeap;                      /* Heap storage space */
  पूर्णांक nHeap;                        /* Size of pHeap[] */
  पूर्णांक mnReq, mxReq;                 /* Min and max heap requests sizes */
  sqlite3_int64 szMmap;             /* mmap() space per open file */
  sqlite3_int64 mxMmap;             /* Maximum value for szMmap */
  व्योम *pPage;                      /* Page cache memory */
  पूर्णांक szPage;                       /* Size of each page in pPage[] */
  पूर्णांक nPage;                        /* Number of pages in pPage[] */
  पूर्णांक mx_Parser_ढेर;                /* maximum depth of the parser stack */
  पूर्णांक sharedCacheEnabled;           /* true if shared-cache mode enabled */
  u32 szPma;                        /* Maximum Sorter PMA size */
  /* The above might be initialized to non-zero.  The following need to always
  ** initially be zero, however. */
  पूर्णांक isInit;                       /* True after initialization has finished */
  पूर्णांक inProgress;                   /* True while initialization in progress */
  पूर्णांक isMutexInit;                  /* True after mutexes are initialized */
  पूर्णांक isMallocInit;                 /* True after malloc is initialized */
  पूर्णांक isPCacheInit;                 /* True after malloc is initialized */
  पूर्णांक nRefInitMutex;                /* Number of users of pInitMutex */
  sqlite3_mutex *pInitMutex;        /* Mutex used by sqlite3_initialize() */
  व्योम (*xLog)(व्योम*,पूर्णांक,स्थिर अक्षर*); /* Function for logging */
  व्योम *pLogArg;                       /* First argument to xLog() */
#अगर_घोषित SQLITE_ENABLE_SQLLOG
  व्योम(*xSqllog)(व्योम*,sqlite3*,स्थिर अक्षर*, पूर्णांक);
  व्योम *pSqllogArg;
#पूर्ण_अगर
#अगर_घोषित SQLITE_VDBE_COVERAGE
  /* The following callback (if not NULL) is invoked on every VDBE branch
  ** operation.  Set the callback using SQLITE_TESTCTRL_VDBE_COVERAGE.
  */
  व्योम (*xVdbeBranch)(व्योम*,अचिन्हित i_Src_पंक्ति,u8 eThis,u8 eMx);  /* Callback */
  व्योम *pVdbeBranchArg;                                     /* 1st argument */
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_DESERIALIZE
  sqlite3_int64 mx_Memdb_आकार;        /* Default max memdb size */
#पूर्ण_अगर
#अगर_अघोषित SQLITE_UNTESTABLE
  पूर्णांक (*xTestCallback)(पूर्णांक);        /* Invoked by sqlite3FaultSim() */
#पूर्ण_अगर
#अगर_घोषित SQLITE_ALLOW_ROWID_IN_VIEW
  u32 mNoVisibleRowid;              /* TF_NoVisibleRowid if the ROWID_IN_VIEW
                                    ** feature is disabled.  0 if rowids can
                                    ** occur in views. */
#पूर्ण_अगर
  पूर्णांक bLocaltimeFault;              /* True to fail localtime() calls */
  पूर्णांक (*xAltLocaltime)(स्थिर व्योम*,व्योम*); /* Alternative localtime() routine */
  पूर्णांक iOnceResetThreshold;          /* When to reset OP_Once counters */
  u32 szSorterRef;                  /* Min size in bytes to use sorter-refs */
  अचिन्हित पूर्णांक iPrngSeed;           /* Alternative fixed seed for the PRNG */
  /* vvvv--- must be last ---vvv */
#अगर_घोषित SQLITE_DEBUG
  sqlite3_int64 aTune[SQLITE_NTUNE]; /* Tuning parameters */
#पूर्ण_अगर
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
#घोषणा CORRUPT_DB  (sqlite3Config.neverCorrupt==0)

/*
** Context pointer passed down through the tree-walk.
*/
काष्ठा Walker {
  Parse *pParse;                            /* Parser context.  */
  पूर्णांक (*x_व्यंजक_Callback)(Walker*, व्यंजक*);     /* Callback for expressions */
  पूर्णांक (*xSelectCallback)(Walker*,Select*);  /* Callback for SELECTs */
  व्योम (*xSelectCallback2)(Walker*,Select*);/* Second callback for SELECTs */
  पूर्णांक walkerDepth;                          /* Number of subqueries */
  u16 eCode;                                /* A small processing code */
  u16 mWFlags;                              /* Use-dependent flags */
  जोड़ {                                   /* Extra data for callback */
    NameContext *pNC;                         /* Naming context */
    पूर्णांक n;                                    /* A counter */
    पूर्णांक iCur;                                 /* A cursor number */
    पूर्णांक sz;                                   /* String literal length */
    SrcList *pSrcList;                        /* FROM clause */
    काष्ठा CCurHint *pCCurHint;               /* Used by codeCursorHint() */
    काष्ठा RefSrcList *pRefSrcList;           /* sqlite3ReferencesSrcList() */
    पूर्णांक *aiCol;                               /* array of column indexes */
    काष्ठा IdxCover *pIdxCover;               /* Check for index coverage */
    व्यंजक_List *pGroupBy;                       /* GROUP BY clause */
    Select *pSelect;                          /* HAVING to WHERE clause ctx */
    काष्ठा WindowRewrite *pRewrite;           /* Window rewrite context */
    काष्ठा WhereConst *pConst;                /* WHERE clause constants */
    काष्ठा RenameCtx *pRename;                /* RENAME COLUMN context */
    काष्ठा Table *pTab;                       /* Table of generated column */
    काष्ठा Covering_सूचकांक_Check *pCovIdxCk;     /* Check for covering index */
    SrcItem *pSrcItem;                        /* A single FROM clause item */
    DbFixer *pFix;                            /* See sqlite3FixSelect() */
    Mem *aMem;                                /* See sqlite3BtreeCursorHint() */
    काष्ठा CheckOnCtx *pCheckOnCtx;           /* See selectCheckOnClauses() */
  } u;
};

/*
** The following structure contains information used by the sqliteFix...
** routines as they walk the parse tree to make database references
** explicit.
*/
काष्ठा DbFixer {
  Parse *pParse;      /* The parsing context.  Error messages written here */
  Walker w;           /* Walker object */
  Schema *pSchema;    /* Fix items to this schema */
  u8 b_अस्थायी;           /* True for TEMP schema entries */
  स्थिर अक्षर *zDb;    /* Make sure all objects are contained in this database */
  स्थिर अक्षर *zType;  /* Type of the container - used for error messages */
  स्थिर Token *pName; /* Name of the container - used for error messages */
};

/* Forward declarations */
पूर्णांक sqlite3_Walk_व्यंजक(Walker*, व्यंजक*);
पूर्णांक sqlite3_Walk_व्यंजक_NN(Walker*, व्यंजक*);
पूर्णांक sqlite3_Walk_व्यंजक_List(Walker*, व्यंजक_List*);
पूर्णांक sqlite3WalkSelect(Walker*, Select*);
पूर्णांक sqlite3_Walk_Select_व्यंजक(Walker*, Select*);
पूर्णांक sqlite3WalkSelectFrom(Walker*, Select*);
पूर्णांक sqlite3_व्यंजक_Walk_Noop(Walker*, व्यंजक*);
पूर्णांक sqlite3SelectWalkNoop(Walker*, Select*);
पूर्णांक sqlite3SelectWalkFail(Walker*, Select*);
पूर्णांक sqlite3WalkerDepthIncrease(Walker*,Select*);
व्योम sqlite3WalkerDepthDecrease(Walker*,Select*);
व्योम sqlite3WalkWinDefnDummyCallback(Walker*,Select*);

#अगर_घोषित SQLITE_DEBUG
व्योम sqlite3SelectWalkAssert2(Walker*, Select*);
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_CTE
व्योम sqlite3_Select_निकालो_With(Walker*, Select*);
#अन्यथा
# define sqlite3_Select_निकालो_With 0
#पूर्ण_अगर

/*
** Return code from the parse-tree walking primitives and their
** callbacks.
*/
#घोषणा WRC_Continue    0   /* Continue down into children */
#घोषणा WRC_Prune       1   /* Omit children but continue walking siblings */
#घोषणा WRC_Abort       2   /* Abandon the tree walk */

/*
** A single common table expression
*/
काष्ठा Cte {
  अक्षर *zName;            /* Name of this CTE */
  व्यंजक_List *pCols;        /* List of explicit column names, or NULL */
  Select *pSelect;        /* The definition of this CTE */
  स्थिर अक्षर *zCteErr;    /* Error message for circular references */
  CteUse *pUse;           /* Usage information for this CTE */
  u8 eM10d;               /* The MATERIALIZED flag */
};

/*
** Allowed values for the materialized flag (eM10d):
*/
#घोषणा M10d_Yes       0  /* AS MATERIALIZED */
#घोषणा M10d_Any       1  /* Not specified.  Query planner's choice */
#घोषणा M10d_No        2  /* AS NOT MATERIALIZED */

/*
** An instance of the With object represents a WITH clause containing
** one or more CTEs (common table expressions).
*/
काष्ठा With {
  पूर्णांक nCte;               /* Number of CTEs in the WITH clause */
  पूर्णांक bView;              /* Belongs to the outermost Select of a view */
  With *pOuter;           /* Containing WITH clause, or NULL */
  Cte a[FLEXARRAY];       /* For each CTE in the WITH clause.... */
};

/* The size (in bytes) of a With object that can hold as many
** as N different CTEs. */
#घोषणा SZ_WITH(N)  (दुरत्व(With,a) + (N)*माप(Cte))

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
काष्ठा CteUse {
  पूर्णांक nUse;              /* Number of users of this CTE */
  पूर्णांक addrM9e;           /* Start of subroutine to compute materialization */
  पूर्णांक regRtn;            /* Return address register for addrM9e subroutine */
  पूर्णांक iCur;              /* Ephemeral table holding the materialization */
  LogEst nRowEst;        /* Estimated number of rows in the table */
  u8 eM10d;              /* The MATERIALIZED flag */
};


/* Client data associated with sqlite3_set_clientdata() and
** sqlite3_get_clientdata().
*/
काष्ठा Db_Client_तथ्य {
  Db_Client_तथ्य *p_अग्र;        /* Next in a linked list */
  व्योम *p_तथ्य;                /* The data */
  व्योम (*xDestructor)(व्योम*); /* Destructor.  Might be NULL */
  अक्षर zName[FLEXARRAY];      /* Name of this client data. MUST BE LAST */
};

/* The size (in bytes) of a DbClientData object that can has a name
** that is N bytes long, including the zero-terminator. */
#घोषणा SZ_DBCLIENTDATA(N) (दुरत्व(Db_Client_तथ्य,zName)+(N))

#अगर_घोषित SQLITE_DEBUG
/*
** An instance of the TreeView object is used for printing the content of
** data structures on sqlite3DebugPrintf() using a tree-like view.
*/
काष्ठा वृक्ष_View {
  पूर्णांक iLevel;             /* Which level of the tree we are on */
  u8  b_पंक्ति[100];         /* Draw vertical in column i if bLine[i] is true */
};
#पूर्ण_अगर /* SQLITE_DEBUG */

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
काष्ठा Window {
  अक्षर *zName;            /* Name of window (may be NULL) */
  अक्षर *zBase;            /* Name of base window for chaining (may be NULL) */
  व्यंजक_List *pPartition;   /* PARTITION BY clause */
  व्यंजक_List *pOrderBy;     /* ORDER BY clause */
  u8 eFrmType;            /* TK_RANGE, TK_GROUPS, TK_ROWS, or 0 */
  u8 eStart;              /* UNBOUNDED, CURRENT, PRECEDING or FOLLOWING */
  u8 eEnd;                /* UNBOUNDED, CURRENT, PRECEDING or FOLLOWING */
  u8 bImplicitFrame;      /* True if frame was implicitly specified */
  u8 eExclude;            /* TK_NO, TK_CURRENT, TK_TIES, TK_GROUP, or 0 */
  व्यंजक *pStart;           /* Expression for "<expr> PRECEDING" */
  व्यंजक *pEnd;             /* Expression for "<expr> FOLLOWING" */
  Window **ppThis;        /* Pointer to this object in Select.pWin list */
  Window *p_अग्र_Win;       /* Next window function belonging to this SELECT */
  व्यंजक *pFilter;          /* The FILTER expression */
  FuncDef *pWFunc;        /* The function */
  पूर्णांक iEphCsr;            /* Partition buffer or Peer buffer */
  पूर्णांक regAccum;           /* Accumulator */
  पूर्णांक reg_परिणाम;          /* Interim result */
  पूर्णांक csrApp;             /* Function cursor (used by min/max) */
  पूर्णांक regApp;             /* Function register (also used by min/max) */
  पूर्णांक regPart;            /* Array of registers for PARTITION BY values */
  व्यंजक *pOwner;           /* Expression object this window is attached to */
  पूर्णांक n_बफर_Col;         /* Number of columns in buffer table */
  पूर्णांक iArgCol;            /* Offset of first argument for this function */
  पूर्णांक regOne;             /* Register containing constant value 1 */
  पूर्णांक regStartRowid;
  पूर्णांक regEndRowid;
  u8 b_व्यंजक_Args;           /* Defer evaluation of window function arguments
                          ** due to the SQLITE_SUBTYPE flag */
};

Select *sqlite3_Multi_मूल्यावली(Parse *pParse, Select *p_बायाँ, व्यंजक_List *pRow);
व्योम sqlite3_Multi_मूल्यावली_End(Parse *pParse, Select *pVal);

#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
व्योम sqlite3WindowDelete(sqlite3*, Window*);
व्योम sqlite3WindowUnlinkFromSelect(Window*);
व्योम sqlite3WindowListDelete(sqlite3 *db, Window *p);
Window *sqlite3WindowAlloc(Parse*, पूर्णांक, पूर्णांक, व्यंजक*, पूर्णांक , व्यंजक*, u8);
व्योम sqlite3WindowAttach(Parse*, व्यंजक*, Window*);
व्योम sqlite3WindowLink(Select *pSel, Window *pWin);
पूर्णांक sqlite3WindowCompare(स्थिर Parse*, स्थिर Window*, स्थिर Window*, पूर्णांक);
व्योम sqlite3WindowCodeInit(Parse*, Select*);
व्योम sqlite3WindowCodeStep(Parse*, Select*, WhereInfo*, पूर्णांक, पूर्णांक);
पूर्णांक sqlite3WindowRewrite(Parse*, Select*);
व्योम sqlite3WindowUpdate(Parse*, Window*, Window*, FuncDef*);
Window *sqlite3WindowDup(sqlite3 *db, व्यंजक *pOwner, Window *p);
Window *sqlite3WindowListDup(sqlite3 *db, Window *p);
व्योम sqlite3WindowFunctions(व्योम);
व्योम sqlite3WindowChain(Parse*, Window*, Window*);
Window *sqlite3WindowAssemble(Parse*, Window*, व्यंजक_List*, व्यंजक_List*, Token*);
#अन्यथा
# define sqlite3WindowDelete(a,b)
# define sqlite3WindowFunctions()
# define sqlite3WindowAttach(a,b,c)
#पूर्ण_अगर

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
#घोषणा SQLITE_SKIP_UTF8(zIn) {                        \
  अगर( (*(zIn++))>=0xc0 ){                              \
    जबतक( (*zIn & 0xc0)==0x80 ){ zIn++; }             \
  }                                                    \
}

/*
** The SQLITE_*_BKPT macros are substitutes for the error codes with
** the same name but without the _BKPT suffix.  These macros invoke
** routines that report the line-number on which the error originated
** using sqlite3_log().  The routines also provide a convenient place
** to set a debugger breakpoint.
*/
पूर्णांक sqlite3ReportError(पूर्णांक iErr, पूर्णांक lineno, स्थिर अक्षर *zType);
पूर्णांक sqlite3CorruptError(पूर्णांक);
पूर्णांक sqlite3MisuseError(पूर्णांक);
पूर्णांक sqlite3CantopenError(पूर्णांक);
#घोषणा SQLITE_CORRUPT_BKPT sqlite3CorruptError(__LINE__)
#घोषणा SQLITE_MISUSE_BKPT sqlite3MisuseError(__LINE__)
#घोषणा SQLITE_CANTOPEN_BKPT sqlite3CantopenError(__LINE__)
#अगर_घोषित SQLITE_DEBUG
  पूर्णांक sqlite3NomemError(पूर्णांक);
  पूर्णांक sqlite3IoerrnomemError(पूर्णांक);
# define SQLITE_NOMEM_BKPT sqlite3NomemError(__LINE__)
# define SQLITE_IOERR_NOMEM_BKPT sqlite3IoerrnomemError(__LINE__)
#अन्यथा
# define SQLITE_NOMEM_BKPT SQLITE_NOMEM
# define SQLITE_IOERR_NOMEM_BKPT SQLITE_IOERR_NOMEM
#पूर्ण_अगर
#अगर defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_CORRUPT_PGNO)
  पूर्णांक sqlite3CorruptPgnoError(पूर्णांक,Pgno);
# define SQLITE_CORRUPT_PGNO(P) sqlite3CorruptPgnoError(__LINE__,(P))
#अन्यथा
# define SQLITE_CORRUPT_PGNO(P) sqlite3CorruptError(__LINE__)
#पूर्ण_अगर

/*
** FTS3 and FTS4 both require virtual table support
*/
#अगर defined(SQLITE_OMIT_VIRTUALTABLE)
# undef SQLITE_ENABLE_FTS3
# undef SQLITE_ENABLE_FTS4
#पूर्ण_अगर

/*
** FTS4 is really an extension for FTS3.  It is enabled using the
** SQLITE_ENABLE_FTS3 macro.  But to avoid confusion we also call
** the SQLITE_ENABLE_FTS4 macro to serve as an alias for SQLITE_ENABLE_FTS3.
*/
#अगर defined(SQLITE_ENABLE_FTS4) && !defined(SQLITE_ENABLE_FTS3)
# define SQLITE_ENABLE_FTS3 1
#पूर्ण_अगर

/*
** The following macros mimic the standard library functions toupper(),
** isspace(), isalnum(), isdigit() and isxdigit(), respectively. The
** sqlite versions only work for ASCII characters, regardless of locale.
*/
#अगर_घोषित SQLITE_ASCII
# define sqlite3Toupper(x)  ((x)&~(sqlite3CtypeMap[(अचिन्हित अक्षर)(x)]&0x20))
# define sqlite3Isspace(x)   (sqlite3CtypeMap[(अचिन्हित अक्षर)(x)]&0x01)
# define sqlite3Isalnum(x)   (sqlite3CtypeMap[(अचिन्हित अक्षर)(x)]&0x06)
# define sqlite3Isalpha(x)   (sqlite3CtypeMap[(अचिन्हित अक्षर)(x)]&0x02)
# define sqlite3Isdigit(x)   (sqlite3CtypeMap[(अचिन्हित अक्षर)(x)]&0x04)
# define sqlite3Isxdigit(x)  (sqlite3CtypeMap[(अचिन्हित अक्षर)(x)]&0x08)
# define sqlite3Tolower(x)   (sqlite3UpperToLower[(अचिन्हित अक्षर)(x)])
# define sqlite3Isquote(x)   (sqlite3CtypeMap[(अचिन्हित अक्षर)(x)]&0x80)
# define sqlite3JsonId1(x)   (sqlite3CtypeMap[(अचिन्हित अक्षर)(x)]&0x42)
# define sqlite3JsonId2(x)   (sqlite3CtypeMap[(अचिन्हित अक्षर)(x)]&0x46)
#अन्यथा
# define sqlite3Toupper(x)   बड़े((अचिन्हित अक्षर)(x))
# define sqlite3Isspace(x)   है_खाली((अचिन्हित अक्षर)(x))
# define sqlite3Isalnum(x)   है_अक्षर_अंक((अचिन्हित अक्षर)(x))
# define sqlite3Isalpha(x)   है_अक्षर((अचिन्हित अक्षर)(x))
# define sqlite3Isdigit(x)   है_अंक((अचिन्हित अक्षर)(x))
# define sqlite3Isxdigit(x)  है_षष्ठादशक((अचिन्हित अक्षर)(x))
# define sqlite3Tolower(x)   छोटे((अचिन्हित अक्षर)(x))
# define sqlite3Isquote(x)   ((x)=='"'||(x)=='\''||(x)=='['||(x)=='`')
# define sqlite3JsonId1(x)   (sqlite3IsIdChar(x)&&(x)<'0')
# define sqlite3JsonId2(x)   sqlite3IsIdChar(x)
#पूर्ण_अगर
पूर्णांक sqlite3IsIdChar(u8);

/*
** Internal function prototypes
*/
पूर्णांक sqlite3StrICmp(स्थिर अक्षर*,स्थिर अक्षर*);
पूर्णांक sqlite3Strlen30(स्थिर अक्षर*);
#घोषणा sqlite3Strlen30NN(C) (म_माप(C)&0x3fffffff)
अक्षर *sqlite3ColumnType(Column*,अक्षर*);
#घोषणा sqlite3StrNICmp sqlite3_strnicmp

पूर्णांक sqlite3MallocInit(व्योम);
व्योम sqlite3MallocEnd(व्योम);
व्योम *sqlite3Malloc(u64);
व्योम *sqlite3MallocZero(u64);
व्योम *sqlite3DbMallocZero(sqlite3*, u64);
व्योम *sqlite3DbMallocRaw(sqlite3*, u64);
व्योम *sqlite3DbMallocRawNN(sqlite3*, u64);
अक्षर *sqlite3DbStrDup(sqlite3*,स्थिर अक्षर*);
अक्षर *sqlite3DbStrNDup(sqlite3*,स्थिर अक्षर*, u64);
अक्षर *sqlite3DbSpanDup(sqlite3*,स्थिर अक्षर*,स्थिर अक्षर*);
व्योम *sqlite3Realloc(व्योम*, u64);
व्योम *sqlite3DbReallocOrFree(sqlite3 *, व्योम *, u64);
व्योम *sqlite3DbRealloc(sqlite3 *, व्योम *, u64);
व्योम sqlite3DbFree(sqlite3*, व्योम*);
व्योम sqlite3DbFreeNN(sqlite3*, व्योम*);
व्योम sqlite3DbNNFreeNN(sqlite3*, व्योम*);
पूर्णांक sqlite3_Malloc_आकार(स्थिर व्योम*);
पूर्णांक sqlite3_Db_Malloc_आकार(sqlite3*, स्थिर व्योम*);
व्योम *sqlite3PageMalloc(पूर्णांक);
व्योम sqlite3PageFree(व्योम*);
व्योम sqlite3MemSetDefault(व्योम);
#अगर_अघोषित SQLITE_UNTESTABLE
व्योम sqlite3BenignMallocHooks(व्योम (*)(व्योम), व्योम (*)(व्योम));
#पूर्ण_अगर
पूर्णांक sqlite3HeapNearlyFull(व्योम);

/*
** On systems with ample stack space and that support alloca(), make
** use of alloca() to obtain space for large automatic objects.  By default,
** obtain space from malloc().
**
** The alloca() routine never returns NULL.  This will cause code paths
** that deal with sqlite3StackAlloc() failures to be unreachable.
*/
#अगर_घोषित SQLITE_USE_ALLOCA
# define sqlite3_ढेर_Alloc_Raw(D,N)   alloca(N)
# define sqlite3_ढेर_Alloc_Raw_NN(D,N) alloca(N)
# define sqlite3_ढेर_Free(D,P)
# define sqlite3_ढेर_Free_NN(D,P)
#अन्यथा
# define sqlite3_ढेर_Alloc_Raw(D,N)   sqlite3DbMallocRaw(D,N)
# define sqlite3_ढेर_Alloc_Raw_NN(D,N) sqlite3DbMallocRawNN(D,N)
# define sqlite3_ढेर_Free(D,P)       sqlite3DbFree(D,P)
# define sqlite3_ढेर_Free_NN(D,P)     sqlite3DbFreeNN(D,P)
#पूर्ण_अगर

/* Do not allow both MEMSYS5 and MEMSYS3 to be defined together.  If they
** are, disable MEMSYS3
*/
#अगर_घोषित SQLITE_ENABLE_MEMSYS5
स्थिर sqlite3_mem_methods *sqlite3MemGetMemsys5(व्योम);
#अघोषित SQLITE_ENABLE_MEMSYS3
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_MEMSYS3
स्थिर sqlite3_mem_methods *sqlite3MemGetMemsys3(व्योम);
#पूर्ण_अगर


#अगर_अघोषित SQLITE_MUTEX_OMIT
  sqlite3_mutex_methods स्थिर *sqlite3DefaultMutex(व्योम);
  sqlite3_mutex_methods स्थिर *sqlite3NoopMutex(व्योम);
  sqlite3_mutex *sqlite3MutexAlloc(पूर्णांक);
  पूर्णांक sqlite3MutexInit(व्योम);
  पूर्णांक sqlite3MutexEnd(व्योम);
#पूर्ण_अगर
#अगर !defined(SQLITE_MUTEX_OMIT) && !defined(SQLITE_MUTEX_NOOP)
  व्योम sqlite3MemoryBarrier(व्योम);
#अन्यथा
# define sqlite3MemoryBarrier()
#पूर्ण_अगर

sqlite3_int64 sqlite3_Status_मूल्य(पूर्णांक);
व्योम sqlite3StatusUp(पूर्णांक, पूर्णांक);
व्योम sqlite3StatusDown(पूर्णांक, पूर्णांक);
व्योम sqlite3StatusHighwater(पूर्णांक, पूर्णांक);
पूर्णांक sqlite3LookasideUsed(sqlite3*,पूर्णांक*);

/* Access to mutexes used by sqlite3_status() */
sqlite3_mutex *sqlite3Pcache1Mutex(व्योम);
sqlite3_mutex *sqlite3MallocMutex(व्योम);


/* The SQLITE_THREAD_MISUSE_WARNINGS compile-time option used to be called
** SQLITE_ENABLE_MULTITHREADED_CHECKS.  Keep that older macro for backwards
** compatibility, at least for a while... */
#अगर_घोषित SQLITE_ENABLE_MULTITHREADED_CHECKS
# define SQLITE_THREAD_MISUSE_WARNINGS 1
#पूर्ण_अगर

/* SQLITE_THREAD_MISUSE_ABORT implies SQLITE_THREAD_MISUSE_WARNINGS */
#अगर_घोषित SQLITE_THREAD_MISUSE_ABORT
# define SQLITE_THREAD_MISUSE_WARNINGS 1
#पूर्ण_अगर

#अगर defined(SQLITE_THREAD_MISUSE_WARNINGS) && !defined(SQLITE_MUTEX_OMIT)
व्योम sqlite3MutexWarnOnContention(sqlite3_mutex*);
#अन्यथा
# define sqlite3MutexWarnOnContention(x)
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_FLOATING_POINT
# define EXP754 (((u64)0x7ff)<<52)
# define MAN754 ((((u64)1)<<52)-1)
# define IsNaN(X) (((X)&EXP754)==EXP754 && ((X)&MAN754)!=0)
# define IsOvfl(X) (((X)&EXP754)==EXP754)
  पूर्णांक sqlite3IsNaN(द्विगुन);
  पूर्णांक sqlite3IsOverflow(द्विगुन);
#अन्यथा
# define IsNaN(X)             0
# define sqlite3IsNaN(X)      0
# define sqlite3IsOVerflow(X) 0
#पूर्ण_अगर

/*
** An instance of the following structure holds information about SQL
** functions arguments that are the parameters to the printf() function.
*/
काष्ठा PrintfArguments {
  पूर्णांक nArg;                /* Total number of arguments */
  पूर्णांक nUsed;               /* Number of arguments used so far */
  sqlite3_मूल्य **apArg;   /* The argument values */
};

/*
** Maxium number of base-10 digits in an unsigned 64-bit integer
*/
#घोषणा SQLITE_U64_DIGITS 20

/*
** An instance of this object receives the decoding of a floating point
** value into an approximate decimal representation.
*/
काष्ठा FpDecode {
  पूर्णांक n;                           /* Significant digits in the decode */
  पूर्णांक iDP;                         /* Location of the decimal point */
  अक्षर *z;                         /* Start of significant digits */
  अक्षर zBuf[SQLITE_U64_DIGITS+1];  /* Storage for significant digits */
  अक्षर sign;                       /* '+' or '-' */
  अक्षर isSpecial;                  /* 1: Infinity  2: NaN */
};

व्योम sqlite3FpDecode(FpDecode*,द्विगुन,पूर्णांक,पूर्णांक);
अक्षर *sqlite3MPrintf(sqlite3*,स्थिर अक्षर*, ...);
अक्षर *sqlite3VMPrintf(sqlite3*,स्थिर अक्षर*, बहु_सूची);
#अगर defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  व्योम sqlite3DebugPrintf(स्थिर अक्षर*, ...);
#पूर्ण_अगर
#अगर defined(SQLITE_TEST)
  व्योम *sqlite3TestTextToPtr(स्थिर अक्षर*);
  स्थिर अक्षर *sqlite3TestPtrToText(व्योम*);
#पूर्ण_अगर

#अगर defined(SQLITE_DEBUG)
  व्योम sqlite3_वृक्ष_View_पंक्ति(वृक्ष_View*, स्थिर अक्षर *zFormat, ...);
  व्योम sqlite3_वृक्ष_View_व्यंजक(वृक्ष_View*, स्थिर व्यंजक*, u8);
  व्योम sqlite3_वृक्ष_View_Bare_व्यंजक_List(वृक्ष_View*, स्थिर व्यंजक_List*, स्थिर अक्षर*);
  व्योम sqlite3_वृक्ष_View_व्यंजक_List(वृक्ष_View*, स्थिर व्यंजक_List*, u8, स्थिर अक्षर*);
  व्योम sqlite3_वृक्ष_View_Bare_Id_List(वृक्ष_View*, स्थिर IdList*, स्थिर अक्षर*);
  व्योम sqlite3_वृक्ष_View_Id_List(वृक्ष_View*, स्थिर IdList*, u8, स्थिर अक्षर*);
  व्योम sqlite3_वृक्ष_View_Column_List(वृक्ष_View*, स्थिर Column*, पूर्णांक, u8);
  व्योम sqlite3_वृक्ष_View_Src_List(वृक्ष_View*, स्थिर SrcList*);
  व्योम sqlite3_वृक्ष_View_Select(वृक्ष_View*, स्थिर Select*, u8);
  व्योम sqlite3_वृक्ष_View_With(वृक्ष_View*, स्थिर With*, u8);
  व्योम sqlite3_वृक्ष_View_Upsert(वृक्ष_View*, स्थिर Upsert*, u8);
#अगर TREETRACE_ENABLED
  व्योम sqlite3_वृक्ष_View_Delete(स्थिर With*, स्थिर SrcList*, स्थिर व्यंजक*,
                             स्थिर व्यंजक_List*,स्थिर व्यंजक*, स्थिर Trigger*);
  व्योम sqlite3_वृक्ष_View_जोड़ो(स्थिर With*, स्थिर SrcList*,
                             स्थिर IdList*, स्थिर Select*, स्थिर व्यंजक_List*,
                             पूर्णांक, स्थिर Upsert*, स्थिर Trigger*);
  व्योम sqlite3_वृक्ष_View_Update(स्थिर With*, स्थिर SrcList*, स्थिर व्यंजक_List*,
                             स्थिर व्यंजक*, पूर्णांक, स्थिर व्यंजक_List*, स्थिर व्यंजक*,
                             स्थिर Upsert*, स्थिर Trigger*);
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_TRIGGER
  व्योम sqlite3_वृक्ष_View_Trigger_Step(वृक्ष_View*, स्थिर TriggerStep*, u8, u8);
  व्योम sqlite3_वृक्ष_View_Trigger(वृक्ष_View*, स्थिर Trigger*, u8, u8);
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
  व्योम sqlite3_वृक्ष_View_Window(वृक्ष_View*, स्थिर Window*, u8);
  व्योम sqlite3_वृक्ष_View_Win_Func(वृक्ष_View*, स्थिर Window*, u8);
#पूर्ण_अगर
  व्योम sqlite3_Show_व्यंजक(स्थिर व्यंजक*);
  व्योम sqlite3_Show_व्यंजक_List(स्थिर व्यंजक_List*);
  व्योम sqlite3ShowIdList(स्थिर IdList*);
  व्योम sqlite3ShowSrcList(स्थिर SrcList*);
  व्योम sqlite3ShowSelect(स्थिर Select*);
  व्योम sqlite3ShowWith(स्थिर With*);
  व्योम sqlite3ShowUpsert(स्थिर Upsert*);
#अगर_अघोषित SQLITE_OMIT_TRIGGER
  व्योम sqlite3ShowTriggerStep(स्थिर TriggerStep*);
  व्योम sqlite3ShowTriggerStepList(स्थिर TriggerStep*);
  व्योम sqlite3ShowTrigger(स्थिर Trigger*);
  व्योम sqlite3ShowTriggerList(स्थिर Trigger*);
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
  व्योम sqlite3ShowWindow(स्थिर Window*);
  व्योम sqlite3ShowWinFunc(स्थिर Window*);
#पूर्ण_अगर
  व्योम sqlite3ShowBitvec(Bitvec*);
#पूर्ण_अगर

व्योम sqlite3SetString(अक्षर **, sqlite3*, स्थिर अक्षर*);
व्योम sqlite3ProgressCheck(Parse*);
व्योम sqlite3_Error_संदेश(Parse*, स्थिर अक्षर*, ...);
पूर्णांक sqlite3ErrorToParser(sqlite3*,पूर्णांक);
व्योम sqlite3Dequote(अक्षर*);
व्योम sqlite3_Dequote_व्यंजक(व्यंजक*);
व्योम sqlite3DequoteToken(Token*);
व्योम sqlite3_Dequote_संख्या(Parse*, व्यंजक*);
व्योम sqlite3TokenInit(Token*,अक्षर*);
पूर्णांक sqlite3KeywordCode(स्थिर अचिन्हित अक्षर*, पूर्णांक);
पूर्णांक sqlite3RunParser(Parse*, स्थिर अक्षर*);
व्योम sqlite3FinishCoding(Parse*);
पूर्णांक sqlite3_Get_अस्थायी_Reg(Parse*);
व्योम sqlite3_Release_अस्थायी_Reg(Parse*,पूर्णांक);
पूर्णांक sqlite3_Get_अस्थायी_Range(Parse*,पूर्णांक);
व्योम sqlite3_Release_अस्थायी_Range(Parse*,पूर्णांक,पूर्णांक);
व्योम sqlite3_Clear_अस्थायी_Reg_Cache(Parse*);
व्योम sqlite3TouchRegister(Parse*,पूर्णांक);
#अगर defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
पूर्णांक sqlite3FirstAvailableRegister(Parse*,पूर्णांक);
#पूर्ण_अगर
#अगर_घोषित SQLITE_DEBUG
पूर्णांक sqlite3NoTempsInRange(Parse*,पूर्णांक,पूर्णांक);
#पूर्ण_अगर
व्यंजक *sqlite3_व्यंजक_Alloc(sqlite3*,पूर्णांक,स्थिर Token*,पूर्णांक);
व्यंजक *sqlite3_व्यंजक(sqlite3*,पूर्णांक,स्थिर अक्षर*);
व्यंजक *sqlite3_व्यंजक_Int32(sqlite3*,पूर्णांक);
व्योम sqlite3_व्यंजक_Attach_Subtrees(sqlite3*,व्यंजक*,व्यंजक*,व्यंजक*);
व्यंजक *sqlite3PExpr(Parse*, पूर्णांक, व्यंजक*, व्यंजक*);
व्योम sqlite3PExprAddSelect(Parse*, व्यंजक*, Select*);
व्यंजक *sqlite3_व्यंजक_And(Parse*,व्यंजक*, व्यंजक*);
व्यंजक *sqlite3_व्यंजक_Simplified_And_Or(व्यंजक*);
व्यंजक *sqlite3_व्यंजक_Function(Parse*,व्यंजक_List*, स्थिर Token*, पूर्णांक);
व्योम sqlite3_व्यंजक_Add_Function_Order_By(Parse*,व्यंजक*,व्यंजक_List*);
व्योम sqlite3_व्यंजक_Order_By_Aggregate_Error(Parse*,व्यंजक*);
व्योम sqlite3_व्यंजक_Function_Usable(Parse*,स्थिर व्यंजक*,स्थिर FuncDef*);
व्योम sqlite3_व्यंजक_Assign_Var_संख्या(Parse*, व्यंजक*, u32);
व्योम sqlite3_व्यंजक_Delete(sqlite3*, व्यंजक*);
व्योम sqlite3_व्यंजक_Delete_Generic(sqlite3*,व्योम*);
पूर्णांक sqlite3_व्यंजक_Deferred_Delete(Parse*, व्यंजक*);
व्योम sqlite3_व्यंजक_Unmap_And_Delete(Parse*, व्यंजक*);
व्यंजक_List *sqlite3_व्यंजक_List_Append(Parse*,व्यंजक_List*,व्यंजक*);
व्यंजक_List *sqlite3_व्यंजक_List_Append_Vector(Parse*,व्यंजक_List*,IdList*,व्यंजक*);
Select *sqlite3_व्यंजक_List_To_मूल्यावली(Parse*, पूर्णांक, व्यंजक_List*);
व्योम sqlite3_व्यंजक_List_Set_Sort_Order(व्यंजक_List*,पूर्णांक,पूर्णांक);
व्योम sqlite3_व्यंजक_List_Set_Name(Parse*,व्यंजक_List*,स्थिर Token*,पूर्णांक);
व्योम sqlite3_व्यंजक_List_Set_Span(Parse*,व्यंजक_List*,स्थिर अक्षर*,स्थिर अक्षर*);
व्योम sqlite3_व्यंजक_List_Delete(sqlite3*, व्यंजक_List*);
व्योम sqlite3_व्यंजक_List_Delete_Generic(sqlite3*,व्योम*);
पूर्णांक sqlite3_व्यंजक_Can_Return_Subtype(Parse*,व्यंजक*);
u32 sqlite3_व्यंजक_List_Flags(स्थिर व्यंजक_List*);
पूर्णांक sqlite3_सूचकांक_Has_Duplicate_मूल_Page(सूचकांक*);
पूर्णांक sqlite3Init(sqlite3*, अक्षर**);
पूर्णांक sqlite3InitCallback(व्योम*, पूर्णांक, अक्षर**, अक्षर**);
पूर्णांक sqlite3InitOne(sqlite3*, पूर्णांक, अक्षर**, u32);
व्योम sqlite3Pragma(Parse*,Token*,Token*,Token*,पूर्णांक);
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
Module *sqlite3PragmaVtabRegister(sqlite3*,स्थिर अक्षर *zName);
#पूर्ण_अगर
व्योम sqlite3ResetAllSchemasOfConnection(sqlite3*);
व्योम sqlite3ResetOneSchema(sqlite3*,पूर्णांक);
व्योम sqlite3CollapseDatabaseArray(sqlite3*);
व्योम sqlite3CommitInternalChanges(sqlite3*);
व्योम sqlite3_Column_Set_व्यंजक(Parse*,Table*,Column*,व्यंजक*);
व्यंजक *sqlite3_Column_व्यंजक(Table*,Column*);
व्यंजक *sqlite3_Column_व्यंजक_Auth(Table*,Column*,Parse*);
व्योम sqlite3ColumnSetColl(sqlite3*,Column*,स्थिर अक्षर*zColl);
स्थिर अक्षर *sqlite3ColumnColl(Column*);
व्योम sqlite3DeleteColumnNames(sqlite3*,Table*);
व्योम sqlite3GenerateColumnNames(Parse *pParse, Select *pSelect);
पूर्णांक sqlite3_Columns_From_व्यंजक_List(Parse*,व्यंजक_List*,i16*,Column**);
व्योम sqlite3SubqueryColumnTypes(Parse*,Table*,Select*,अक्षर);
Table *sqlite3_परिणाम_Set_Of_Select(Parse*,Select*,अक्षर);
व्योम sqlite3OpenSchemaTable(Parse *, पूर्णांक);
सूचकांक *sqlite3_Primary_कुंजी_सूचकांक(Table*);
पूर्णांक sqlite3_Table_Column_To_सूचकांक(सूचकांक*, पूर्णांक);
#अगर_घोषित SQLITE_OMIT_GENERATED_COLUMNS
# define sqlite3TableColumnToStorage(T,X) (X)  /* No-op pass-through */
# define sqlite3StorageColumnToTable(T,X) (X)  /* No-op pass-through */
#अन्यथा
  i16 sqlite3TableColumnToStorage(Table*, i16);
  i16 sqlite3StorageColumnToTable(Table*, i16);
#पूर्ण_अगर
व्योम sqlite3StartTable(Parse*,Token*,Token*,पूर्णांक,पूर्णांक,पूर्णांक,पूर्णांक);
#अगर SQLITE_ENABLE_HIDDEN_COLUMNS
  व्योम sqlite3ColumnPropertiesFromName(Table*, Column*);
#अन्यथा
# define sqlite3ColumnPropertiesFromName(T,C) /* no-op */
#पूर्ण_अगर
व्योम sqlite3AddColumn(Parse*,Token,Token);
व्योम sqlite3AddNotNull(Parse*, पूर्णांक);
व्योम sqlite3_Add_Primary_कुंजी(Parse*, व्यंजक_List*, पूर्णांक, पूर्णांक, पूर्णांक);
व्योम sqlite3AddCheckConstraint(Parse*, व्यंजक*, स्थिर अक्षर*, स्थिर अक्षर*);
व्योम sqlite3_Add_Default_मूल्य(Parse*,व्यंजक*,स्थिर अक्षर*,स्थिर अक्षर*);
व्योम sqlite3AddCollateType(Parse*, Token*);
व्योम sqlite3AddGenerated(Parse*,व्यंजक*,Token*);
व्योम sqlite3EndTable(Parse*,Token*,Token*,u32,Select*);
व्योम sqlite3AddReturning(Parse*,व्यंजक_List*);
पूर्णांक sqlite3ParseUri(स्थिर अक्षर*,स्थिर अक्षर*,अचिन्हित पूर्णांक*,
                    sqlite3_vfs**,अक्षर**,अक्षर **);
#घोषणा sqlite3CodecQueryParameters(A,B,C) 0
Btree *sqlite3DbNameToBtree(sqlite3*,स्थिर अक्षर*);

#अगर_घोषित SQLITE_UNTESTABLE
# define sqlite3FaultSim(X) SQLITE_OK
#अन्यथा
  पूर्णांक sqlite3FaultSim(पूर्णांक);
#पूर्ण_अगर

Bitvec *sqlite3BitvecCreate(u32);
पूर्णांक sqlite3BitvecTest(Bitvec*, u32);
पूर्णांक sqlite3BitvecTestNotNull(Bitvec*, u32);
पूर्णांक sqlite3BitvecSet(Bitvec*, u32);
व्योम sqlite3BitvecClear(Bitvec*, u32, व्योम*);
व्योम sqlite3BitvecDestroy(Bitvec*);
u32 sqlite3_Bitvec_आकार(Bitvec*);
#अगर_अघोषित SQLITE_UNTESTABLE
पूर्णांक sqlite3BitvecBuiltinTest(पूर्णांक,पूर्णांक*);
#पूर्ण_अगर

RowSet *sqlite3RowSetInit(sqlite3*);
व्योम sqlite3RowSetDelete(व्योम*);
व्योम sqlite3RowSetClear(व्योम*);
व्योम sqlite3_Row_Set_जोड़ो(RowSet*, i64);
पूर्णांक sqlite3RowSetTest(RowSet*, पूर्णांक iBatch, i64);
पूर्णांक sqlite3_Row_Set_अग्र(RowSet*, i64*);

व्योम sqlite3CreateView(Parse*,Token*,Token*,Token*,व्यंजक_List*,Select*,पूर्णांक,पूर्णांक);

#अगर !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
  पूर्णांक sqlite3ViewGetColumnNames(Parse*,Table*);
#अन्यथा
# define sqlite3ViewGetColumnNames(A,B) 0
#पूर्ण_अगर

#अगर SQLITE_MAX_ATTACHED>30
  पूर्णांक sqlite3DbMaskAllZero(yDbMask);
#पूर्ण_अगर
व्योम sqlite3DropTable(Parse*, SrcList*, पूर्णांक, पूर्णांक);
व्योम sqlite3CodeDropTable(Parse*, Table*, पूर्णांक, पूर्णांक);
व्योम sqlite3DeleteTable(sqlite3*, Table*);
व्योम sqlite3DeleteTableGeneric(sqlite3*, व्योम*);
व्योम sqlite3_Free_सूचकांक(sqlite3*, सूचकांक*);
#अगर_अघोषित SQLITE_OMIT_AUTOINCREMENT
  व्योम sqlite3AutoincrementBegin(Parse *pParse);
  व्योम sqlite3AutoincrementEnd(Parse *pParse);
#अन्यथा
# define sqlite3AutoincrementBegin(X)
# define sqlite3AutoincrementEnd(X)
#पूर्ण_अगर
व्योम sqlite3_जोड़ो(Parse*, SrcList*, Select*, IdList*, पूर्णांक, Upsert*);
#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
  व्योम sqlite3ComputeGeneratedColumns(Parse*, पूर्णांक, Table*);
#पूर्ण_अगर
व्योम *sqlite3ArrayAllocate(sqlite3*,व्योम*,पूर्णांक,पूर्णांक*,पूर्णांक*);
IdList *sqlite3IdListAppend(Parse*, IdList*, Token*);
पूर्णांक sqlite3_Id_List_सूचकांक(IdList*,स्थिर अक्षर*);
SrcList *sqlite3SrcListEnlarge(Parse*, SrcList*, पूर्णांक, पूर्णांक);
SrcList *sqlite3SrcListAppendList(Parse *pParse, SrcList *p1, SrcList *p2);
SrcList *sqlite3SrcListAppend(Parse*, SrcList*, Token*, Token*);
व्योम sqlite3SubqueryDelete(sqlite3*,Subquery*);
Select *sqlite3SubqueryDetach(sqlite3*,SrcItem*);
पूर्णांक sqlite3SrcItemAttachSubquery(Parse*, SrcItem*, Select*, पूर्णांक);
SrcList *sqlite3SrcListAppendFromTerm(Parse*, SrcList*, Token*, Token*,
                                      Token*, Select*, OnOrUsing*);
व्योम sqlite3SrcListIndexedBy(Parse *, SrcList *, Token *);
व्योम sqlite3SrcListFuncArgs(Parse*, SrcList*, व्यंजक_List*);
पूर्णांक sqlite3IndexedByLookup(Parse *, SrcItem *);
व्योम sqlite3SrcListShiftJoinType(Parse*,SrcList*);
व्योम sqlite3SrcListAssignCursors(Parse*, SrcList*);
व्योम sqlite3IdListDelete(sqlite3*, IdList*);
व्योम sqlite3ClearOnOrUsing(sqlite3*, OnOrUsing*);
व्योम sqlite3SrcListDelete(sqlite3*, SrcList*);
सूचकांक *sqlite3_Allocate_सूचकांक_Object(sqlite3*,पूर्णांक,पूर्णांक,अक्षर**);
पूर्णांक sqlite3_सूचकांक_Bloomable(स्थिर सूचकांक*,पूर्णांक);
व्योम sqlite3_Create_सूचकांक(Parse*,Token*,Token*,SrcList*,व्यंजक_List*,पूर्णांक,Token*,
                          व्यंजक*, पूर्णांक, पूर्णांक, u8);
व्योम sqlite3_Drop_सूचकांक(Parse*, SrcList*, पूर्णांक);
पूर्णांक sqlite3Select(Parse*, Select*, SelectDest*);
Select *sqlite3SelectNew(Parse*,व्यंजक_List*,SrcList*,व्यंजक*,व्यंजक_List*,
                         व्यंजक*,व्यंजक_List*,u32,व्यंजक*);
व्योम sqlite3SelectDelete(sqlite3*, Select*);
व्योम sqlite3SelectDeleteGeneric(sqlite3*,व्योम*);
व्योम sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect);
Table *sqlite3SrcListLookup(Parse*, SrcList*);
पूर्णांक sqlite3IsReadOnly(Parse*, Table*, Trigger*);
व्योम sqlite3OpenTable(Parse*, पूर्णांक iCur, पूर्णांक iDb, Table*, पूर्णांक);
#अगर defined(SQLITE_ENABLE_UPDATE_DELETE_LIMIT) && !defined(SQLITE_OMIT_SUBQUERY)
व्यंजक *sqlite3LimitWhere(Parse*,SrcList*,व्यंजक*,व्यंजक_List*,व्यंजक*,अक्षर*);
#पूर्ण_अगर
व्योम sqlite3_Code_Change_गिनती(Vdbe*,पूर्णांक,स्थिर अक्षर*);
व्योम sqlite3DeleteFrom(Parse*, SrcList*, व्यंजक*, व्यंजक_List*, व्यंजक*);
व्योम sqlite3Update(Parse*, SrcList*, व्यंजक_List*,व्यंजक*,पूर्णांक,व्यंजक_List*,व्यंजक*,
                   Upsert*);
WhereInfo *sqlite3WhereBegin(Parse*,SrcList*,व्यंजक*,व्यंजक_List*,
                             व्यंजक_List*,Select*,u16,पूर्णांक);
व्योम sqlite3WhereEnd(WhereInfo*);
LogEst sqlite3_Where_Output_Row_गिनती(WhereInfo*);
पूर्णांक sqlite3WhereIsDistinct(WhereInfo*);
पूर्णांक sqlite3WhereIsOrdered(WhereInfo*);
पूर्णांक sqlite3WhereOrderByLimitOptLabel(WhereInfo*);
व्योम sqlite3WhereMinMaxOptEarlyOut(Vdbe*,WhereInfo*);
पूर्णांक sqlite3WhereIsSorted(WhereInfo*);
पूर्णांक sqlite3WhereContinueLabel(WhereInfo*);
पूर्णांक sqlite3WhereBreakLabel(WhereInfo*);
पूर्णांक sqlite3WhereOkOnePass(WhereInfo*, पूर्णांक*);
#घोषणा ONEPASS_OFF      0        /* Use of ONEPASS not allowed */
#घोषणा ONEPASS_SINGLE   1        /* ONEPASS valid for a single row update */
#घोषणा ONEPASS_MULTI    2        /* ONEPASS is valid for multiple rows */
पूर्णांक sqlite3WhereUsesDeferredSeek(WhereInfo*);
व्योम sqlite3_व्यंजक_Code_Load_सूचकांक_Column(Parse*, सूचकांक*, पूर्णांक, पूर्णांक, पूर्णांक);
पूर्णांक sqlite3_व्यंजक_Code_Get_Column(Parse*, Table*, पूर्णांक, पूर्णांक, पूर्णांक, u8);
व्योम sqlite3_व्यंजक_Code_Get_Column_Of_Table(Vdbe*, Table*, पूर्णांक, पूर्णांक, पूर्णांक);
व्योम sqlite3_व्यंजक_Code_Move(Parse*, पूर्णांक, पूर्णांक, पूर्णांक);
व्योम sqlite3_व्यंजक_To_Register(व्यंजक *p_व्यंजक, पूर्णांक iReg);
व्योम sqlite3_व्यंजक_Code(Parse*, व्यंजक*, पूर्णांक);
#अगर_अघोषित SQLITE_OMIT_GENERATED_COLUMNS
व्योम sqlite3_व्यंजक_Code_Generated_Column(Parse*, Table*, Column*, पूर्णांक);
#पूर्ण_अगर
व्योम sqlite3_व्यंजक_Code_Copy(Parse*, व्यंजक*, पूर्णांक);
व्योम sqlite3_व्यंजक_Code_Factorable(Parse*, व्यंजक*, पूर्णांक);
पूर्णांक sqlite3_व्यंजक_Code_Run_Just_Once(Parse*, व्यंजक*, पूर्णांक);
व्योम sqlite3_व्यंजक_Null_Register_Range(Parse*, पूर्णांक, पूर्णांक);
पूर्णांक sqlite3_व्यंजक_Code_अस्थायी(Parse*, व्यंजक*, पूर्णांक*);
पूर्णांक sqlite3_व्यंजक_Code_Target(Parse*, व्यंजक*, पूर्णांक);
पूर्णांक sqlite3_व्यंजक_Code_व्यंजक_List(Parse*, व्यंजक_List*, पूर्णांक, पूर्णांक, u8);
#घोषणा SQLITE_ECEL_DUP      0x01  /* Deep, not shallow copies */
#घोषणा SQLITE_ECEL_FACTOR   0x02  /* Factor out constant terms */
#घोषणा SQLITE_ECEL_REF      0x04  /* Use ExprList.u.x.iOrderByCol */
#घोषणा SQLITE_ECEL_OMITREF  0x08  /* Omit if ExprList.u.x.iOrderByCol */
व्योम sqlite3_व्यंजक_If_True(Parse*, व्यंजक*, पूर्णांक, पूर्णांक);
व्योम sqlite3_व्यंजक_If_False(Parse*, व्यंजक*, पूर्णांक, पूर्णांक);
व्योम sqlite3_व्यंजक_If_False_Dup(Parse*, व्यंजक*, पूर्णांक, पूर्णांक);
Table *sqlite3FindTable(sqlite3*,स्थिर अक्षर*, स्थिर अक्षर*);
#घोषणा LOCATE_VIEW    0x01
#घोषणा LOCATE_NOERR   0x02
Table *sqlite3LocateTable(Parse*,u32 flags,स्थिर अक्षर*, स्थिर अक्षर*);
स्थिर अक्षर *sqlite3PreferredTableName(स्थिर अक्षर*);
Table *sqlite3LocateTableItem(Parse*,u32 flags,SrcItem *);
सूचकांक *sqlite3_Find_सूचकांक(sqlite3*,स्थिर अक्षर*, स्थिर अक्षर*);
व्योम sqlite3UnlinkAndDeleteTable(sqlite3*,पूर्णांक,स्थिर अक्षर*);
व्योम sqlite3_Unlink_And_Delete_सूचकांक(sqlite3*,पूर्णांक,स्थिर अक्षर*);
व्योम sqlite3Vacuum(Parse*,Token*,व्यंजक*);
पूर्णांक sqlite3RunVacuum(अक्षर**, sqlite3*, पूर्णांक, sqlite3_मूल्य*);
अक्षर *sqlite3NameFromToken(sqlite3*, स्थिर Token*);
पूर्णांक sqlite3_व्यंजक_Compare(स्थिर Parse*,स्थिर व्यंजक*,स्थिर व्यंजक*, पूर्णांक);
पूर्णांक sqlite3_व्यंजक_Compare_Skip(व्यंजक*,व्यंजक*,पूर्णांक);
पूर्णांक sqlite3_व्यंजक_List_Compare(स्थिर व्यंजक_List*,स्थिर व्यंजक_List*, पूर्णांक);
पूर्णांक sqlite3_व्यंजक_Implies_व्यंजक(स्थिर Parse*,स्थिर व्यंजक*,स्थिर व्यंजक*, पूर्णांक);
पूर्णांक sqlite3_व्यंजक_Implies_Non_Null_Row(व्यंजक*,पूर्णांक,पूर्णांक);
व्योम sqlite3AggInfoPersistWalkerInit(Walker*,Parse*);
व्योम sqlite3_व्यंजक_Analyze_Aggregates(NameContext*, व्यंजक*);
व्योम sqlite3_व्यंजक_Analyze_Agg_List(NameContext*,व्यंजक_List*);
पूर्णांक sqlite3_व्यंजक_Covered_By_सूचकांक(व्यंजक*, पूर्णांक iCur, सूचकांक *pIdx);
पूर्णांक sqlite3ReferencesSrcList(Parse*, व्यंजक*, SrcList*);
Vdbe *sqlite3GetVdbe(Parse*);
#अगर_अघोषित SQLITE_UNTESTABLE
व्योम sqlite3PrngSaveState(व्योम);
व्योम sqlite3PrngRestoreState(व्योम);
#पूर्ण_अगर
व्योम sqlite3RollbackAll(sqlite3*,पूर्णांक);
व्योम sqlite3CodeVerifySchema(Parse*, पूर्णांक);
व्योम sqlite3CodeVerifyNamedSchema(Parse*, स्थिर अक्षर *zDb);
व्योम sqlite3BeginTransaction(Parse*, पूर्णांक);
व्योम sqlite3EndTransaction(Parse*,पूर्णांक);
व्योम sqlite3Savepoint(Parse*, पूर्णांक, Token*);
व्योम sqlite3CloseSavepoints(sqlite3 *);
व्योम sqlite3LeaveMutexAndCloseZombie(sqlite3*);
u32 sqlite3IsTrueOrFalse(स्थिर अक्षर*);
पूर्णांक sqlite3_व्यंजक_Id_To_True_False(व्यंजक*);
पूर्णांक sqlite3_व्यंजक_Truth_मूल्य(स्थिर व्यंजक*);
पूर्णांक sqlite3_व्यंजक_Is_Constant(Parse*,व्यंजक*);
पूर्णांक sqlite3_व्यंजक_Is_Constant_Or_Function(व्यंजक*, u8);
पूर्णांक sqlite3_व्यंजक_Is_Constant_Or_Group_By(Parse*, व्यंजक*, व्यंजक_List*);
पूर्णांक sqlite3_व्यंजक_Is_Table_Constant(व्यंजक*,पूर्णांक,पूर्णांक);
पूर्णांक sqlite3_व्यंजक_Is_Single_Table_Constraint(व्यंजक*,स्थिर SrcList*,पूर्णांक,पूर्णांक);
पूर्णांक sqlite3_व्यंजक_List_Is_Constant(Parse *pParse, व्यंजक_List *pList, पूर्णांक bNoIs);
पूर्णांक sqlite3_व्यंजक_Contains_Subquery(व्यंजक*);
पूर्णांक sqlite3_व्यंजक_Is_Integer(स्थिर व्यंजक*, पूर्णांक*, Parse*, पूर्णांक);
पूर्णांक sqlite3_व्यंजक_Can_Be_Null(स्थिर व्यंजक*);
पूर्णांक sqlite3_व्यंजक_Needs_No_Affinity_Change(स्थिर व्यंजक*, अक्षर);
पूर्णांक sqlite3_व्यंजक_Is_Like_Operator(स्थिर व्यंजक*);
पूर्णांक sqlite3IsRowid(स्थिर अक्षर*);
स्थिर अक्षर *sqlite3RowidAlias(Table *pTab);
व्योम sqlite3GenerateRowDelete(
    Parse*,Table*,Trigger*,पूर्णांक,पूर्णांक,पूर्णांक,i16,u8,u8,u8,पूर्णांक);
व्योम sqlite3_Generate_Row_सूचकांक_Delete(Parse*, Table*, पूर्णांक, पूर्णांक, पूर्णांक*, पूर्णांक);
पूर्णांक sqlite3_Generate_सूचकांक_कुंजी(Parse*, सूचकांक*, पूर्णांक, पूर्णांक, पूर्णांक, पूर्णांक*,सूचकांक*,पूर्णांक);
व्योम sqlite3ResolvePartIdxLabel(Parse*,पूर्णांक);
पूर्णांक sqlite3_व्यंजक_References_Updated_Column(व्यंजक*,पूर्णांक*,पूर्णांक);
व्योम sqlite3GenerateConstraintChecks(Parse*,Table*,पूर्णांक*,पूर्णांक,पूर्णांक,पूर्णांक,पूर्णांक,
                                     u8,u8,पूर्णांक,पूर्णांक*,पूर्णांक*,Upsert*);
#अगर_घोषित SQLITE_ENABLE_NULL_TRIM
  व्योम sqlite3SetMakeRecordP5(Vdbe*,Table*);
#अन्यथा
# define sqlite3SetMakeRecordP5(A,B)
#पूर्ण_अगर
व्योम sqlite3CompleteInsertion(Parse*,Table*,पूर्णांक,पूर्णांक,पूर्णांक,पूर्णांक*,पूर्णांक,पूर्णांक,पूर्णांक);
पूर्णांक sqlite3OpenTableAndIndices(Parse*, Table*, पूर्णांक, u8, पूर्णांक, u8*, पूर्णांक*, पूर्णांक*);
व्योम sqlite3BeginWriteOperation(Parse*, पूर्णांक, पूर्णांक);
व्योम sqlite3MultiWrite(Parse*);
व्योम sqlite3MayAbort(Parse*);
व्योम sqlite3HaltConstraint(Parse*, पूर्णांक, पूर्णांक, अक्षर*, i8, u8);
व्योम sqlite3UniqueConstraint(Parse*, पूर्णांक, सूचकांक*);
व्योम sqlite3RowidConstraint(Parse*, पूर्णांक, Table*);
व्यंजक *sqlite3_व्यंजक_Dup(sqlite3*,स्थिर व्यंजक*,पूर्णांक);
व्यंजक_List *sqlite3_व्यंजक_List_Dup(sqlite3*,स्थिर व्यंजक_List*,पूर्णांक);
SrcList *sqlite3SrcListDup(sqlite3*,स्थिर SrcList*,पूर्णांक);
IdList *sqlite3IdListDup(sqlite3*,स्थिर IdList*);
Select *sqlite3SelectDup(sqlite3*,स्थिर Select*,पूर्णांक);
FuncDef *sqlite3_Function_खोज(पूर्णांक,स्थिर अक्षर*);
व्योम sqlite3_जोड़ो_Builtin_Funcs(FuncDef*,पूर्णांक);
FuncDef *sqlite3FindFunction(sqlite3*,स्थिर अक्षर*,पूर्णांक,u8,u8);
व्योम sqlite3_Quote_मूल्य(StrAccum*,sqlite3_मूल्य*,पूर्णांक);
पूर्णांक sqlite3AppendOneUtf8Character(अक्षर*, u32);
व्योम sqlite3RegisterBuiltinFunctions(व्योम);
व्योम sqlite3RegisterDateTimeFunctions(व्योम);
व्योम sqlite3RegisterJsonFunctions(व्योम);
व्योम sqlite3RegisterPerConnectionBuiltinFunctions(sqlite3*);
#अगर !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
  Module *sqlite3JsonVtabRegister(sqlite3*,स्थिर अक्षर*);
#पूर्ण_अगर
पूर्णांक sqlite3SafetyCheckOk(sqlite3*);
पूर्णांक sqlite3SafetyCheckSickOrOk(sqlite3*);
व्योम sqlite3ChangeCookie(Parse*, पूर्णांक);
With *sqlite3WithDup(sqlite3 *db, With *p);

#अगर !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
  Module *sqlite3CarrayRegister(sqlite3*);
#पूर्ण_अगर

#अगर !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
व्योम sqlite3MaterializeView(Parse*, Table*, व्यंजक*, व्यंजक_List*,व्यंजक*,पूर्णांक);
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_TRIGGER
  व्योम sqlite3BeginTrigger(Parse*, Token*,Token*,पूर्णांक,पूर्णांक,IdList*,SrcList*,
                           व्यंजक*,पूर्णांक, पूर्णांक);
  व्योम sqlite3FinishTrigger(Parse*, TriggerStep*, Token*);
  व्योम sqlite3DropTrigger(Parse*, SrcList*, पूर्णांक);
  व्योम sqlite3DropTriggerPtr(Parse*, Trigger*);
  Trigger *sqlite3TriggersExist(Parse *, Table*, पूर्णांक, व्यंजक_List*, पूर्णांक *pMask);
  Trigger *sqlite3TriggerList(Parse *, Table *);
  व्योम sqlite3CodeRowTrigger(Parse*, Trigger *, पूर्णांक, व्यंजक_List*, पूर्णांक, Table *,
                            पूर्णांक, पूर्णांक, पूर्णांक);
  व्योम sqlite3CodeRowTriggerDirect(Parse *, Trigger *, Table *, पूर्णांक, पूर्णांक, पूर्णांक);
  व्योम sqliteViewTriggers(Parse*, Table*, व्यंजक*, पूर्णांक, व्यंजक_List*);
  व्योम sqlite3DeleteTriggerStep(sqlite3*, TriggerStep*);
  TriggerStep *sqlite3TriggerSelectStep(sqlite3*,Select*,
                                        स्थिर अक्षर*,स्थिर अक्षर*);
  TriggerStep *sqlite3_Trigger_जोड़ो_Step(Parse*,SrcList*, IdList*,
                                        Select*,u8,Upsert*,
                                        स्थिर अक्षर*,स्थिर अक्षर*);
  TriggerStep *sqlite3TriggerUpdateStep(Parse*,SrcList*,SrcList*,व्यंजक_List*,
                                        व्यंजक*, u8, स्थिर अक्षर*,स्थिर अक्षर*);
  TriggerStep *sqlite3TriggerDeleteStep(Parse*,SrcList*, व्यंजक*,
                                        स्थिर अक्षर*,स्थिर अक्षर*);
  व्योम sqlite3DeleteTrigger(sqlite3*, Trigger*);
  व्योम sqlite3UnlinkAndDeleteTrigger(sqlite3*,पूर्णांक,स्थिर अक्षर*);
  u32 sqlite3TriggerColmask(Parse*,Trigger*,व्यंजक_List*,पूर्णांक,पूर्णांक,Table*,पूर्णांक);
# define sqlite3ParseToplevel(p) ((p)->pToplevel ? (p)->pToplevel : (p))
# define sqlite3IsToplevel(p) ((p)->pToplevel==0)
#अन्यथा
# define sqlite3TriggersExist(B,C,D,E,F) 0
# define sqlite3DeleteTrigger(A,B)
# define sqlite3DropTriggerPtr(A,B)
# define sqlite3UnlinkAndDeleteTrigger(A,B,C)
# define sqlite3CodeRowTrigger(A,B,C,D,E,F,G,H,I)
# define sqlite3CodeRowTriggerDirect(A,B,C,D,E,F)
# define sqlite3TriggerList(X, Y) 0
# define sqlite3ParseToplevel(p) p
# define sqlite3IsToplevel(p) 1
# define sqlite3TriggerColmask(A,B,C,D,E,F,G) 0
#पूर्ण_अगर

पूर्णांक sqlite3JoinType(Parse*, Token*, Token*, Token*);
पूर्णांक sqlite3_Column_सूचकांक(Table *pTab, स्थिर अक्षर *zCol);
व्योम sqlite3SrcItemColumnUsed(SrcItem*,पूर्णांक);
व्योम sqlite3_Set_Join_व्यंजक(व्यंजक*,पूर्णांक,u32);
व्योम sqlite3_Create_Foreign_कुंजी(Parse*, व्यंजक_List*, Token*, व्यंजक_List*, पूर्णांक);
व्योम sqlite3_Defer_Foreign_कुंजी(Parse*, पूर्णांक);
#अगर_अघोषित SQLITE_OMIT_AUTHORIZATION
  व्योम sqlite3AuthRead(Parse*,व्यंजक*,Schema*,SrcList*);
  पूर्णांक sqlite3AuthCheck(Parse*,पूर्णांक, स्थिर अक्षर*, स्थिर अक्षर*, स्थिर अक्षर*);
  व्योम sqlite3_Auth_Context_धकेलो(Parse*, AuthContext*, स्थिर अक्षर*);
  व्योम sqlite3_Auth_Context_निकालो(AuthContext*);
  पूर्णांक sqlite3AuthReadCol(Parse*, स्थिर अक्षर *, स्थिर अक्षर *, पूर्णांक);
#अन्यथा
# define sqlite3AuthRead(a,b,c,d)
# define sqlite3AuthCheck(a,b,c,d,e)    SQLITE_OK
# define sqlite3_Auth_Context_धकेलो(a,b,c)
# define sqlite3_Auth_Context_निकालो(a)  ((व्योम)(a))
#पूर्ण_अगर
पूर्णांक sqlite3DbIsNamed(sqlite3 *db, पूर्णांक iDb, स्थिर अक्षर *zName);
व्योम sqlite3Attach(Parse*, व्यंजक*, व्यंजक*, व्यंजक*);
व्योम sqlite3Detach(Parse*, व्यंजक*);
व्योम sqlite3FixInit(DbFixer*, Parse*, पूर्णांक, स्थिर अक्षर*, स्थिर Token*);
पूर्णांक sqlite3FixSrcList(DbFixer*, SrcList*);
पूर्णांक sqlite3FixSelect(DbFixer*, Select*);
पूर्णांक sqlite3_Fix_व्यंजक(DbFixer*, व्यंजक*);
पूर्णांक sqlite3FixTriggerStep(DbFixer*, TriggerStep*);

पूर्णांक sqlite3RealSameAsInt(द्विगुन,sqlite3_int64);
i64 sqlite3RealToI64(द्विगुन);
पूर्णांक sqlite3Int64ToText(i64,अक्षर*);
पूर्णांक sqlite3AtoF(स्थिर अक्षर *z, द्विगुन*);
पूर्णांक sqlite3GetInt32(स्थिर अक्षर *, पूर्णांक*);
पूर्णांक sqlite3GetUInt32(स्थिर अक्षर*, u32*);
पूर्णांक sqlite3Atoi(स्थिर अक्षर*);
#अगर_अघोषित SQLITE_OMIT_UTF16
पूर्णांक sqlite3Utf16ByteLen(स्थिर व्योम *p_तथ्य, पूर्णांक nByte, पूर्णांक nChar);
#पूर्ण_अगर
पूर्णांक sqlite3Utf8CharLen(स्थिर अक्षर *p_तथ्य, पूर्णांक nByte);
u32 sqlite3Utf8Read(स्थिर u8**);
पूर्णांक sqlite3Utf8ReadLimited(स्थिर u8*, पूर्णांक, u32*);
LogEst sqlite3LogEst(u64);
LogEst sqlite3LogEstAdd(LogEst,LogEst);
LogEst sqlite3LogEstFromDouble(द्विगुन);
u64 sqlite3LogEstToInt(LogEst);
VList *sqlite3VListAdd(sqlite3*,VList*,स्थिर अक्षर*,पूर्णांक,पूर्णांक);
स्थिर अक्षर *sqlite3_VList_अंक_To_Name(VList*,पूर्णांक);
पूर्णांक sqlite3_VList_Name_To_अंक(VList*,स्थिर अक्षर*,पूर्णांक);

/*
** Routines to read and write variable-length integers.  These used to
** be defined locally, but now we use the varint routines in the util.c
** file.
*/
पूर्णांक sqlite3PutVarint(अचिन्हित अक्षर*, u64);
u8 sqlite3GetVarint(स्थिर अचिन्हित अक्षर *, u64 *);
i64 sqlite3_Varint_मूल्य(स्थिर अचिन्हित अक्षर*);
u8 sqlite3GetVarint32(स्थिर अचिन्हित अक्षर *, u32 *);
पूर्णांक sqlite3VarintLen(u64 v);

/*
** The common case is for a varint to be a single byte.  They following
** macros handle the common case without a procedure call, but then call
** the procedure for larger varints.
*/
#घोषणा getVarint32(A,B)  \
  (u8)((*(A)<(u8)0x80)?((B)=(u32)*(A)),1:sqlite3GetVarint32((A),(u32 *)&(B)))
#घोषणा getVarint32NR(A,B) \
  B=(u32)*(A);अगर(B>=0x80)sqlite3GetVarint32((A),(u32*)&(B))
#घोषणा putVarint32(A,B)  \
  (u8)(((u32)(B)<(u32)0x80)?(*(A)=(अचिन्हित अक्षर)(B)),1:\
  sqlite3PutVarint((A),(B)))

स्थिर अक्षर *sqlite3_सूचकांक_Affinity_Str(sqlite3*, सूचकांक*);
अक्षर *sqlite3TableAffinityStr(sqlite3*,स्थिर Table*);
व्योम sqlite3TableAffinity(Vdbe*, Table*, पूर्णांक);
अक्षर sqlite3CompareAffinity(स्थिर व्यंजक *p_व्यंजक, अक्षर aff2);
पूर्णांक sqlite3_सूचकांक_Affinity_Ok(स्थिर व्यंजक *p_व्यंजक, अक्षर idx_affinity);
अक्षर sqlite3TableColumnAffinity(स्थिर Table*,पूर्णांक);
अक्षर sqlite3_व्यंजक_Affinity(स्थिर व्यंजक *p_व्यंजक);
पूर्णांक sqlite3_व्यंजक_तथ्य_Type(स्थिर व्यंजक *p_व्यंजक);
पूर्णांक sqlite3Atoi64(स्थिर अक्षर*, i64*, पूर्णांक, u8);
पूर्णांक sqlite3DecOrHexToI64(स्थिर अक्षर*, i64*);
व्योम sqlite3_Error_With_संदेश(sqlite3*, पूर्णांक, स्थिर अक्षर*,...);
व्योम sqlite3Error(sqlite3*,पूर्णांक);
व्योम sqlite3ErrorClear(sqlite3*);
व्योम sqlite3SystemError(sqlite3*,पूर्णांक);
#अगर !defined(SQLITE_OMIT_BLOB_LITERAL)
व्योम *sqlite3HexToBlob(sqlite3*, स्थिर अक्षर *z, पूर्णांक n);
#पूर्ण_अगर
u8 sqlite3HexToInt(पूर्णांक h);
पूर्णांक sqlite3TwoPartName(Parse *, Token *, Token *, Token **);

#अगर defined(SQLITE_NEED_ERR_NAME)
स्थिर अक्षर *sqlite3ErrName(पूर्णांक);
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_DESERIALIZE
पूर्णांक sqlite3MemdbInit(व्योम);
पूर्णांक sqlite3IsMemdb(स्थिर sqlite3_vfs*);
#अन्यथा
# define sqlite3IsMemdb(X) 0
#पूर्ण_अगर

स्थिर अक्षर *sqlite3ErrStr(पूर्णांक);
पूर्णांक sqlite3ReadSchema(Parse *pParse);
CollSeq *sqlite3FindCollSeq(sqlite3*,u8 enc, स्थिर अक्षर*,पूर्णांक);
पूर्णांक sqlite3_द्विआधारी_Compare(व्योम*,पूर्णांक,स्थिर व्योम*,पूर्णांक,स्थिर व्योम*);
#अगर SQLITE_USES_INLINE
अटल SQLITE_INLINE पूर्णांक sqlite3_Is_द्विआधारी(स्थिर CollSeq *p){
  निश्चित( p==0 || p->xCmp!=sqlite3_द्विआधारी_Compare
       || म_भेद(p->zName,"BINARY")==0 );
  वापस p==0 || p->xCmp==sqlite3_द्विआधारी_Compare;
}
#अन्यथा
पूर्णांक sqlite3_Is_द्विआधारी(स्थिर CollSeq*);
#पूर्ण_अगर
CollSeq *sqlite3LocateCollSeq(Parse *pParse, स्थिर अक्षर*zName);
व्योम sqlite3SetTextEncoding(sqlite3 *db, u8);
CollSeq *sqlite3_व्यंजक_Coll_Seq(Parse *pParse, स्थिर व्यंजक *p_व्यंजक);
CollSeq *sqlite3_व्यंजक_NNColl_Seq(Parse *pParse, स्थिर व्यंजक *p_व्यंजक);
पूर्णांक sqlite3_व्यंजक_Coll_Seq_Match(Parse*,स्थिर व्यंजक*,स्थिर व्यंजक*);
व्यंजक *sqlite3_व्यंजक_Add_Collate_Token(Parse *pParse, व्यंजक*, स्थिर Token*, पूर्णांक);
व्यंजक *sqlite3_व्यंजक_Add_Collate_String(Parse*,व्यंजक*,स्थिर अक्षर*);
व्यंजक *sqlite3_व्यंजक_Skip_Collate(व्यंजक*);
व्यंजक *sqlite3_व्यंजक_Skip_Collate_And_Likely(व्यंजक*);
पूर्णांक sqlite3CheckCollSeq(Parse *, CollSeq *);
पूर्णांक sqlite3WritableSchema(sqlite3*);
पूर्णांक sqlite3CheckObjectName(Parse*, स्थिर अक्षर*,स्थिर अक्षर*,स्थिर अक्षर*);
व्योम sqlite3VdbeSetChanges(sqlite3 *, i64);
पूर्णांक sqlite3AddInt64(i64*,i64);
पूर्णांक sqlite3SubInt64(i64*,i64);
पूर्णांक sqlite3MulInt64(i64*,i64);
पूर्णांक sqlite3AbsInt32(पूर्णांक);
#अगर_घोषित SQLITE_ENABLE_8_3_NAMES
व्योम sqlite3FileSuffix3(स्थिर अक्षर*, अक्षर*);
#अन्यथा
# define sqlite3FileSuffix3(X,Y)
#पूर्ण_अगर
u8 sqlite3GetBoolean(स्थिर अक्षर *z,u8);

स्थिर व्योम *sqlite3_मूल्य_Text(sqlite3_मूल्य*, u8);
पूर्णांक sqlite3_मूल्य_Is_Of_Class(स्थिर sqlite3_मूल्य*, व्योम(*)(व्योम*));
पूर्णांक sqlite3_मूल्य_Bytes(sqlite3_मूल्य*, u8);
व्योम sqlite3_मूल्य_Set_Str(sqlite3_मूल्य*, पूर्णांक, स्थिर व्योम *,u8,
                        व्योम(*)(व्योम*));
व्योम sqlite3_मूल्य_Set_Null(sqlite3_मूल्य*);
व्योम sqlite3_मूल्य_Free(sqlite3_मूल्य*);
#अगर_अघोषित SQLITE_UNTESTABLE
व्योम sqlite3_परिणाम_Int_Real(sqlite3_context*);
#पूर्ण_अगर
sqlite3_मूल्य *sqlite3_मूल्य_New(sqlite3 *);
#अगर_अघोषित SQLITE_OMIT_UTF16
अक्षर *sqlite3Utf16to8(sqlite3 *, स्थिर व्योम*, पूर्णांक, u8);
#पूर्ण_अगर
पूर्णांक sqlite3_मूल्य_From_व्यंजक(sqlite3 *, स्थिर व्यंजक *, u8, u8, sqlite3_मूल्य **);
व्योम sqlite3_मूल्य_Apply_Affinity(sqlite3_मूल्य *, u8, u8);
#अगर_अघोषित SQLITE_AMALGAMATION
बाह्य स्थिर अचिन्हित अक्षर sqlite3OpcodeProperty[];
बाह्य स्थिर अक्षर sqlite3_Str_द्विआधारी[];
बाह्य स्थिर अचिन्हित अक्षर sqlite3StdTypeLen[];
बाह्य स्थिर अक्षर sqlite3StdTypeAffinity[];
बाह्य स्थिर अक्षर *sqlite3StdType[];
बाह्य स्थिर अचिन्हित अक्षर sqlite3UpperToLower[];
बाह्य स्थिर अचिन्हित अक्षर *sqlite3aLTb;
बाह्य स्थिर अचिन्हित अक्षर *sqlite3aEQb;
बाह्य स्थिर अचिन्हित अक्षर *sqlite3aGTb;
बाह्य स्थिर अचिन्हित अक्षर sqlite3CtypeMap[];
बाह्य SQLITE_WSD काष्ठा Sqlite3Config sqlite3Config;
बाह्य FuncDefHash sqlite3BuiltinFunctions;
बाह्य स्थिर sqlite3_str sqlite3OomStr;
#अगर_अघोषित SQLITE_OMIT_WSD
बाह्य पूर्णांक sqlite3PendingByte;
#पूर्ण_अगर
#पूर्ण_अगर /* SQLITE_AMALGAMATION */
#अगर_घोषित VDBE_PROFILE
बाह्य sqlite3_uint64 sqlite3NProfileCnt;
#पूर्ण_अगर
व्योम sqlite3_मूल_Page_Moved(sqlite3*, पूर्णांक, Pgno, Pgno);
व्योम sqlite3Reindex(Parse*, Token*, Token*);
व्योम sqlite3AlterFunctions(व्योम);
व्योम sqlite3AlterRenameTable(Parse*, SrcList*, Token*);
व्योम sqlite3AlterRenameColumn(Parse*, SrcList*, Token*, Token*);
व्योम sqlite3AlterDropConstraint(Parse*,SrcList*,Token*,Token*);
व्योम sqlite3AlterAddConstraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *pFirst,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  स्थिर अक्षर *z_व्यंजक,       /* Text of CHECK expression */
  पूर्णांक n_व्यंजक,               /* Size of pExpr in bytes */
  व्यंजक *p_व्यंजक              /* The parsed CHECK expression */
);
व्योम sqlite3AlterSetNotNull(Parse*, SrcList*, Token*, Token*);
i64 sqlite3GetToken(स्थिर अचिन्हित अक्षर *, पूर्णांक *);
व्योम sqlite3NestedParse(Parse*, स्थिर अक्षर*, ...);
व्योम sqlite3ExpirePreparedStatements(sqlite3*, पूर्णांक);
व्योम sqlite3CodeRhsOfIN(Parse*, व्यंजक*, पूर्णांक, पूर्णांक);
पूर्णांक sqlite3CodeSubselect(Parse*, व्यंजक*);
व्योम sqlite3SelectPrep(Parse*, Select*, NameContext*);
पूर्णांक sqlite3ExpandSubquery(Parse*, SrcItem*);
व्योम sqlite3_Select_Wrong_अंक_Terms_Error(Parse *pParse, Select *p);
पूर्णांक sqlite3MatchEName(
  स्थिर काष्ठा व्यंजक_List_item*,
  स्थिर अक्षर*,
  स्थिर अक्षर*,
  स्थिर अक्षर*,
  पूर्णांक*
);
Bitmask sqlite3_व्यंजक_Col_Used(व्यंजक*);
u8 sqlite3StrIHash(स्थिर अक्षर*);
पूर्णांक sqlite3_Resolve_व्यंजक_Names(NameContext*, व्यंजक*);
पूर्णांक sqlite3_Resolve_व्यंजक_List_Names(NameContext*, व्यंजक_List*);
व्योम sqlite3ResolveSelectNames(Parse*, Select*, NameContext*);
पूर्णांक sqlite3ResolveSelfReference(Parse*,Table*,पूर्णांक,व्यंजक*,व्यंजक_List*);
पूर्णांक sqlite3ResolveOrderGroupBy(Parse*, Select*, व्यंजक_List*, स्थिर अक्षर*);
व्योम sqlite3ColumnDefault(Vdbe *, Table *, पूर्णांक, पूर्णांक);
व्योम sqlite3AlterFinishAddColumn(Parse *, Token *);
व्योम sqlite3AlterBeginAddColumn(Parse *, SrcList *);
व्योम sqlite3AlterDropColumn(Parse*, SrcList*, स्थिर Token*);
स्थिर व्योम *sqlite3RenameTokenMap(Parse*, स्थिर व्योम*, स्थिर Token*);
व्योम sqlite3RenameTokenRemap(Parse*, स्थिर व्योम *pTo, स्थिर व्योम *pFrom);
व्योम sqlite3_Rename_व्यंजक_Unmap(Parse*, व्यंजक*);
व्योम sqlite3RenameExprlistUnmap(Parse*, व्यंजक_List*);
CollSeq *sqlite3GetCollSeq(Parse*, u8, CollSeq *, स्थिर अक्षर*);
अक्षर sqlite3AffinityType(स्थिर अक्षर*, Column*);
व्योम sqlite3Analyze(Parse*, Token*, Token*);
पूर्णांक sqlite3InvokeBusyHandler(BusyHandler*);
पूर्णांक sqlite3FindDb(sqlite3*, Token*);
पूर्णांक sqlite3FindDbName(sqlite3 *, स्थिर अक्षर *);
पूर्णांक sqlite3AnalysisLoad(sqlite3*,पूर्णांक iDB);
व्योम sqlite3_Delete_सूचकांक_Samples(sqlite3*,सूचकांक*);
व्योम sqlite3DefaultRowEst(सूचकांक*);
व्योम sqlite3RegisterLikeFunctions(sqlite3*, पूर्णांक);
पूर्णांक sqlite3IsLikeFunction(sqlite3*,व्यंजक*,पूर्णांक*,अक्षर*);
व्योम sqlite3SchemaClear(व्योम *);
Schema *sqlite3SchemaGet(sqlite3 *, Btree *);
पूर्णांक sqlite3_Schema_To_सूचकांक(sqlite3 *db, Schema *);
कुंजी_Info *sqlite3_कुंजी_Info_Alloc(sqlite3*,पूर्णांक,पूर्णांक);
व्योम sqlite3_कुंजी_Info_Unref(कुंजी_Info*);
कुंजी_Info *sqlite3_कुंजी_Info_Ref(कुंजी_Info*);
कुंजी_Info *sqlite3_कुंजी_Info_Of_सूचकांक(Parse*, सूचकांक*);
कुंजी_Info *sqlite3_कुंजी_Info_From_व्यंजक_List(Parse*, व्यंजक_List*, पूर्णांक, पूर्णांक);
स्थिर अक्षर *sqlite3SelectOpName(पूर्णांक);
पूर्णांक sqlite3HasExplicitNulls(Parse*, व्यंजक_List*);

#अगर_घोषित SQLITE_DEBUG
पूर्णांक sqlite3_कुंजी_Info_Is_Writeable(कुंजी_Info*);
#पूर्ण_अगर
पूर्णांक sqlite3CreateFunc(sqlite3 *, स्थिर अक्षर *, पूर्णांक, पूर्णांक, व्योम *,
  व्योम (*)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य **),
  व्योम (*)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य **),
  व्योम (*)(sqlite3_context*),
  व्योम (*)(sqlite3_context*),
  व्योम (*)(sqlite3_context*,पूर्णांक,sqlite3_मूल्य **),
  FuncDestructor *pDestructor
);
व्योम sqlite3NoopDestructor(व्योम*);
व्योम *sqlite3OomFault(sqlite3*);
व्योम sqlite3OomClear(sqlite3*);
पूर्णांक sqlite3ApiExit(sqlite3 *db, पूर्णांक);
पूर्णांक sqlite3_Open_अस्थायी_Database(Parse *);

अक्षर *sqlite3RCStrRef(अक्षर*);
व्योम sqlite3RCStrUnref(व्योम*);
अक्षर *sqlite3RCStrNew(u64);
अक्षर *sqlite3RCStrResize(अक्षर*,u64);

व्योम sqlite3StrAccumInit(StrAccum*, sqlite3*, अक्षर*, पूर्णांक, पूर्णांक);
पूर्णांक sqlite3StrAccumEnlarge(StrAccum*, i64);
पूर्णांक sqlite3StrAccumEnlargeIfNeeded(StrAccum*, i64);
अक्षर *sqlite3StrAccumFinish(StrAccum*);
व्योम sqlite3StrAccumSetError(StrAccum*, u8);
व्योम sqlite3SelectDestInit(SelectDest*,पूर्णांक,पूर्णांक);
व्यंजक *sqlite3_Create_Column_व्यंजक(sqlite3 *, SrcList *, पूर्णांक, पूर्णांक);
व्योम sqlite3RecordErrorByteOffset(sqlite3*,स्थिर अक्षर*);
व्योम sqlite3_Record_Error_Offset_Of_व्यंजक(sqlite3*,स्थिर व्यंजक*);

व्योम sqlite3BackupRestart(sqlite3_backup *);
व्योम sqlite3BackupUpdate(sqlite3_backup *, Pgno, स्थिर u8 *);

#अगर_अघोषित SQLITE_OMIT_SUBQUERY
पूर्णांक sqlite3_व्यंजक_Check_IN(Parse*, व्यंजक*);
#अन्यथा
# define sqlite3_व्यंजक_Check_IN(x,y) SQLITE_OK
#पूर्ण_अगर

#अगर_घोषित SQLITE_ENABLE_STAT4
पूर्णांक sqlite3_Stat4_Probe_Set_मूल्य(
    Parse*,सूचकांक*,UnpackedRecord**,व्यंजक*,पूर्णांक,पूर्णांक,पूर्णांक*);
पूर्णांक sqlite3_Stat4_मूल्य_From_व्यंजक(Parse*, व्यंजक*, u8, sqlite3_मूल्य**);
व्योम sqlite3Stat4ProbeFree(UnpackedRecord*);
पूर्णांक sqlite3Stat4Column(sqlite3*, स्थिर व्योम*, पूर्णांक, पूर्णांक, sqlite3_मूल्य**);
अक्षर sqlite3_सूचकांक_Column_Affinity(sqlite3*, सूचकांक*, पूर्णांक);
#पूर्ण_अगर

/*
** The interface to the LEMON-generated parser
*/
#अगर_अघोषित SQLITE_AMALGAMATION
  व्योम *sqlite3ParserAlloc(व्योम*(*)(u64), Parse*);
  व्योम sqlite3ParserFree(व्योम*, व्योम(*)(व्योम*));
#पूर्ण_अगर
व्योम sqlite3Parser(व्योम*, पूर्णांक, Token);
पूर्णांक sqlite3ParserFallback(पूर्णांक);
#अगर_घोषित YYTRACKMAXSTACKDEPTH
  पूर्णांक sqlite3_Parser_ढेर_Peak(व्योम*);
#पूर्ण_अगर

व्योम sqlite3AutoLoadExtensions(sqlite3*);
#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
  व्योम sqlite3CloseExtensions(sqlite3*);
#अन्यथा
# define sqlite3CloseExtensions(X)
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_SHARED_CACHE
  व्योम sqlite3TableLock(Parse *, पूर्णांक, Pgno, u8, स्थिर अक्षर *);
#अन्यथा
  #घोषणा sqlite3TableLock(v,w,x,y,z)
#पूर्ण_अगर

#अगर_घोषित SQLITE_TEST
  पूर्णांक sqlite3Utf8To8(अचिन्हित अक्षर*);
#पूर्ण_अगर

#अगर_घोषित SQLITE_OMIT_VIRTUALTABLE
#  define sqlite3VtabClear(D,T)
#  define sqlite3VtabSync(X,Y) SQLITE_OK
#  define sqlite3VtabRollback(X)
#  define sqlite3VtabCommit(X)
#  define sqlite3VtabInSync(db) 0
#  define sqlite3VtabLock(X)
#  define sqlite3VtabUnlock(X)
#  define sqlite3VtabModuleUnref(D,X)
#  define sqlite3VtabUnlockList(X)
#  define sqlite3VtabSavepoint(X, Y, Z) SQLITE_OK
#  define sqlite3GetVTable(X,Y)  ((VTable*)0)
#अन्यथा
   व्योम sqlite3VtabClear(sqlite3 *db, Table*);
   व्योम sqlite3VtabDisconnect(sqlite3 *db, Table *p);
   पूर्णांक sqlite3VtabSync(sqlite3 *db, Vdbe*);
   पूर्णांक sqlite3VtabRollback(sqlite3 *db);
   पूर्णांक sqlite3VtabCommit(sqlite3 *db);
   व्योम sqlite3VtabLock(VTable *);
   व्योम sqlite3VtabUnlock(VTable *);
   व्योम sqlite3VtabModuleUnref(sqlite3*,Module*);
   व्योम sqlite3VtabUnlockList(sqlite3*);
   पूर्णांक sqlite3VtabSavepoint(sqlite3 *, पूर्णांक, पूर्णांक);
   व्योम sqlite3VtabImportErrmsg(Vdbe*, sqlite3_vtab*);
   VTable *sqlite3GetVTable(sqlite3*, Table*);
   Module *sqlite3VtabCreateModule(
     sqlite3*,
     स्थिर अक्षर*,
     स्थिर sqlite3_module*,
     व्योम*,
     व्योम(*)(व्योम*)
   );
#  define sqlite3VtabInSync(db) ((db)->nVTrans>0 && (db)->aVTrans==0)
#पूर्ण_अगर
पूर्णांक sqlite3ReadOnlyShadowTables(sqlite3 *db);
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  पूर्णांक sqlite3ShadowTableName(sqlite3 *db, स्थिर अक्षर *zName);
  पूर्णांक sqlite3IsShadowTableOf(sqlite3*,Table*,स्थिर अक्षर*);
  व्योम sqlite3MarkAllShadowTablesOf(sqlite3*, Table*);
  व्योम sqlite3VtabEponymousTableClearAll(sqlite3*);
#अन्यथा
# define sqlite3ShadowTableName(A,B) 0
# define sqlite3IsShadowTableOf(A,B,C) 0
# define sqlite3MarkAllShadowTablesOf(A,B)
# define sqlite3VtabEponymousTableClearAll(X)
#पूर्ण_अगर
पूर्णांक sqlite3VtabEponymousTableInit(Parse*,Module*);
व्योम sqlite3VtabEponymousTableClear(sqlite3*,Module*);
व्योम sqlite3VtabMakeWritable(Parse*,Table*);
व्योम sqlite3VtabBeginParse(Parse*, Token*, Token*, Token*, पूर्णांक);
व्योम sqlite3VtabFinishParse(Parse*, Token*);
व्योम sqlite3VtabArgInit(Parse*);
व्योम sqlite3VtabArgExtend(Parse*, Token*);
पूर्णांक sqlite3VtabCallCreate(sqlite3*, पूर्णांक, स्थिर अक्षर *, अक्षर **);
पूर्णांक sqlite3VtabCallConnect(Parse*, Table*);
पूर्णांक sqlite3VtabCallDestroy(sqlite3*, पूर्णांक, स्थिर अक्षर *);
पूर्णांक sqlite3VtabBegin(sqlite3 *, VTable *);

FuncDef *sqlite3VtabOverloadFunction(sqlite3 *,FuncDef*, पूर्णांक nArg, व्यंजक*);
व्योम sqlite3VtabUsesAllSchemas(Parse*);
sqlite3_int64 sqlite3StmtCurrentTime(sqlite3_context*);
पूर्णांक sqlite3_Vdbe_Parameter_सूचकांक(Vdbe*, स्थिर अक्षर*, पूर्णांक);
पूर्णांक sqlite3TransferBindings(sqlite3_stmt *, sqlite3_stmt *);
व्योम sqlite3ParseObjectInit(Parse*,sqlite3*);
व्योम sqlite3ParseObjectReset(Parse*);
व्योम *sqlite3ParserAddCleanup(Parse*,व्योम(*)(sqlite3*,व्योम*),व्योम*);
#अगर_घोषित SQLITE_ENABLE_NORMALIZE
अक्षर *sqlite3Normalize(Vdbe*, स्थिर अक्षर*);
#पूर्ण_अगर
पूर्णांक sqlite3Reprepare(Vdbe*);
व्योम sqlite3_व्यंजक_List_Check_लंबाई(Parse*, व्यंजक_List*, स्थिर अक्षर*);
CollSeq *sqlite3_व्यंजक_Compare_Coll_Seq(Parse*,स्थिर व्यंजक*);
CollSeq *sqlite3_द्विआधारी_Compare_Coll_Seq(Parse *, स्थिर व्यंजक*, स्थिर व्यंजक*);
पूर्णांक sqlite3_अस्थायी_In_Memory(स्थिर sqlite3*);
स्थिर अक्षर *sqlite3JournalModename(पूर्णांक);
#अगर_अघोषित SQLITE_OMIT_WAL
  पूर्णांक sqlite3Checkpoint(sqlite3*, पूर्णांक, पूर्णांक, पूर्णांक*, पूर्णांक*);
  पूर्णांक sqlite3WalDefaultHook(व्योम*,sqlite3*,स्थिर अक्षर*,पूर्णांक);
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_CTE
  Cte *sqlite3CteNew(Parse*,Token*,व्यंजक_List*,Select*,u8);
  व्योम sqlite3CteDelete(sqlite3*,Cte*);
  With *sqlite3WithAdd(Parse*,With*,Cte*);
  व्योम sqlite3WithDelete(sqlite3*,With*);
  व्योम sqlite3WithDeleteGeneric(sqlite3*,व्योम*);
  With *sqlite3_With_धकेलो(Parse*, With*, u8);
#अन्यथा
# define sqlite3CteNew(P,T,E,S)   ((व्योम*)0)
# define sqlite3CteDelete(D,C)
# define sqlite3CteWithAdd(P,W,C) ((व्योम*)0)
# define sqlite3WithDelete(x,y)
# define sqlite3_With_धकेलो(x,y,z) ((व्योम*)0)
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_UPSERT
  Upsert *sqlite3UpsertNew(sqlite3*,व्यंजक_List*,व्यंजक*,व्यंजक_List*,व्यंजक*,Upsert*);
  व्योम sqlite3UpsertDelete(sqlite3*,Upsert*);
  Upsert *sqlite3UpsertDup(sqlite3*,Upsert*);
  पूर्णांक sqlite3UpsertAnalyzeTarget(Parse*,SrcList*,Upsert*,Upsert*);
  व्योम sqlite3UpsertDoUpdate(Parse*,Upsert*,Table*,सूचकांक*,पूर्णांक);
  Upsert *sqlite3_Upsert_Of_सूचकांक(Upsert*,सूचकांक*);
  पूर्णांक sqlite3_Upsert_अग्र_Is_IPK(Upsert*);
#अन्यथा
#घोषणा sqlite3UpsertNew(u,v,w,x,y,z) ((Upsert*)0)
#घोषणा sqlite3UpsertDelete(x,y)
#घोषणा sqlite3UpsertDup(x,y)         ((Upsert*)0)
#घोषणा sqlite3_Upsert_Of_सूचकांक(x,y)     ((Upsert*)0)
#घोषणा sqlite3_Upsert_अग्र_Is_IPK(x)     0
#पूर्ण_अगर


/* Declarations for functions in fkey.c. All of these are replaced by
** no-op macros if OMIT_FOREIGN_KEY is defined. In this case no foreign
** key functionality is available. If OMIT_TRIGGER is defined but
** OMIT_FOREIGN_KEY is not, only some of the functions are no-oped. In
** this case foreign keys are parsed, but no other functionality is
** provided (enforcement of FK constraints requires the triggers sub-system).
*/
#अगर !defined(SQLITE_OMIT_FOREIGN_कुंजी) && !defined(SQLITE_OMIT_TRIGGER)
  व्योम sqlite3FkCheck(Parse*, Table*, पूर्णांक, पूर्णांक, पूर्णांक*, पूर्णांक);
  व्योम sqlite3FkDropTable(Parse*, SrcList *, Table*);
  व्योम sqlite3FkActions(Parse*, Table*, व्यंजक_List*, पूर्णांक, पूर्णांक*, पूर्णांक);
  पूर्णांक sqlite3FkRequired(Parse*, Table*, पूर्णांक*, पूर्णांक);
  u32 sqlite3FkOldmask(Parse*, Table*);
  FKey *sqlite3FkReferences(Table *);
  व्योम sqlite3FkClearTriggerCache(sqlite3*,पूर्णांक);
#अन्यथा
  #घोषणा sqlite3FkActions(a,b,c,d,e,f)
  #घोषणा sqlite3FkCheck(a,b,c,d,e,f)
  #घोषणा sqlite3FkDropTable(a,b,c)
  #घोषणा sqlite3FkOldmask(a,b)         0
  #घोषणा sqlite3FkRequired(a,b,c,d)    0
  #घोषणा sqlite3FkReferences(a)        0
  #घोषणा sqlite3FkClearTriggerCache(a,b)
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_FOREIGN_कुंजी
  व्योम sqlite3FkDelete(sqlite3 *, Table*);
  पूर्णांक sqlite3_Fk_Locate_सूचकांक(Parse*,Table*,FKey*,सूचकांक**,पूर्णांक**);
#अन्यथा
  #घोषणा sqlite3FkDelete(a,b)
  #घोषणा sqlite3_Fk_Locate_सूचकांक(a,b,c,d,e)
#पूर्ण_अगर


/*
** Available fault injectors.  Should be numbered beginning with 0.
*/
#घोषणा SQLITE_FAULTINJECTOR_MALLOC     0
#घोषणा SQLITE_FAULTINJECTOR_गिनती      1

/*
** The interface to the code in fault.c used for identifying "benign"
** malloc failures. This is only present if SQLITE_UNTESTABLE
** is not defined.
*/
#अगर_अघोषित SQLITE_UNTESTABLE
  व्योम sqlite3BeginBenignMalloc(व्योम);
  व्योम sqlite3EndBenignMalloc(व्योम);
#अन्यथा
  #घोषणा sqlite3BeginBenignMalloc()
  #घोषणा sqlite3EndBenignMalloc()
#पूर्ण_अगर

/*
** Allowed return values from sqlite3FindInIndex()
*/
#घोषणा IN_सूचकांक_ROWID        1   /* Search the rowid of the table */
#घोषणा IN_सूचकांक_EPH          2   /* Search an ephemeral b-tree */
#घोषणा IN_सूचकांक_सूचकांक_ASC    3   /* Existing index ASCENDING */
#घोषणा IN_सूचकांक_सूचकांक_DESC   4   /* Existing index DESCENDING */
#घोषणा IN_सूचकांक_NOOP         5   /* No table available. Use comparisons */
/*
** Allowed flags for the 3rd parameter to sqlite3FindInIndex().
*/
#घोषणा IN_सूचकांक_NOOP_OK     0x0001  /* OK to return IN_INDEX_NOOP */
#घोषणा IN_सूचकांक_MEMBERSHIP  0x0002  /* IN operator used for membership test */
#घोषणा IN_सूचकांक_LOOP        0x0004  /* IN operator used as a loop */
पूर्णांक sqlite3_Find_In_सूचकांक(Parse *, व्यंजक *, u32, पूर्णांक*, पूर्णांक*, पूर्णांक*);

पूर्णांक sqlite3JournalOpen(sqlite3_vfs *, स्थिर अक्षर *, sqlite3_file *, पूर्णांक, पूर्णांक);
पूर्णांक sqlite3_Journal_आकार(sqlite3_vfs *);
#अगर defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
  पूर्णांक sqlite3JournalCreate(sqlite3_file *);
#पूर्ण_अगर

पूर्णांक sqlite3JournalIsInMemory(sqlite3_file *p);
व्योम sqlite3MemJournalOpen(sqlite3_file *);

व्योम sqlite3_व्यंजक_Set_Height_And_Flags(Parse *pParse, व्यंजक *p);
#अगर SQLITE_MAX_व्यंजक_DEPTH>0
  पूर्णांक sqlite3_Select_व्यंजक_Height(स्थिर Select *);
  पूर्णांक sqlite3_व्यंजक_Check_Height(Parse*, पूर्णांक);
#अन्यथा
  #घोषणा sqlite3_Select_व्यंजक_Height(x) 0
  #घोषणा sqlite3_व्यंजक_Check_Height(x,y)
#पूर्ण_अगर
व्योम sqlite3_व्यंजक_Set_Error_Offset(व्यंजक*,पूर्णांक);

u32 sqlite3Get4byte(स्थिर u8*);
SQLITE_OPT_INLINE u64 sqlite3Get8byte(स्थिर u8*);
#अगर SQLITE_BYTEORDER!=4321
SQLITE_OPT_INLINE u64 sqlite3BSwap64(u64);
#पूर्ण_अगर
व्योम sqlite3Put4byte(u8*, u32);

#अगर_घोषित SQLITE_ENABLE_UNLOCK_NOTIFY
  व्योम sqlite3ConnectionBlocked(sqlite3 *, sqlite3 *);
  व्योम sqlite3ConnectionUnlocked(sqlite3 *db);
  व्योम sqlite3ConnectionClosed(sqlite3 *db);
#अन्यथा
  #घोषणा sqlite3ConnectionBlocked(x,y)
  #घोषणा sqlite3ConnectionUnlocked(x)
  #घोषणा sqlite3ConnectionClosed(x)
#पूर्ण_अगर

#अगर_घोषित SQLITE_DEBUG
  व्योम sqlite3ParserTrace(खाता*, अक्षर *);
#पूर्ण_अगर
#अगर defined(YYCOVERAGE)
  पूर्णांक sqlite3ParserCoverage(खाता*);
#पूर्ण_अगर

/*
** If the SQLITE_ENABLE IOTRACE exists then the global variable
** sqlite3IoTrace is a pointer to a printf-like routine used to
** print I/O tracing messages.
*/
#अगर_घोषित SQLITE_ENABLE_IOTRACE
# define IOTRACE(A)  अगर( sqlite3IoTrace ){ sqlite3IoTrace A; }
  व्योम sqlite3VdbeIOTraceSql(Vdbe*);
SQLITE_API SQLITE_EXTERN व्योम (SQLITE_CDECL *sqlite3IoTrace)(स्थिर अक्षर*,...);
#अन्यथा
# define IOTRACE(A)
# define sqlite3VdbeIOTraceSql(X)
#पूर्ण_अगर

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
#अगर_घोषित SQLITE_MEMDEBUG
  व्योम sqlite3MemdebugSetType(व्योम*,u8);
  पूर्णांक sqlite3MemdebugHasType(स्थिर व्योम*,u8);
  पूर्णांक sqlite3MemdebugNoType(स्थिर व्योम*,u8);
#अन्यथा
# define sqlite3MemdebugSetType(X,Y)  /* no-op */
# define sqlite3MemdebugHasType(X,Y)  1
# define sqlite3MemdebugNoType(X,Y)   1
#पूर्ण_अगर
#घोषणा MEMTYPE_HEAP       0x01  /* General heap allocations */
#घोषणा MEMTYPE_LOOKASIDE  0x02  /* Heap that might have been lookaside */
#घोषणा MEMTYPE_PCACHE     0x04  /* Page cache allocations */

/*
** Threading interface
*/
#अगर SQLITE_MAX_WORKER_THREADS>0
पूर्णांक sqlite3ThreadCreate(SQLiteThread**,व्योम*(*)(व्योम*),व्योम*);
पूर्णांक sqlite3ThreadJoin(SQLiteThread*, व्योम**);
#पूर्ण_अगर

#अगर defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)
पूर्णांक sqlite3DbpageRegister(sqlite3*);
#पूर्ण_अगर
#अगर defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)
पूर्णांक sqlite3DbstatRegister(sqlite3*);
#पूर्ण_अगर

पूर्णांक sqlite3_व्यंजक_Vector_आकार(स्थिर व्यंजक *p_व्यंजक);
पूर्णांक sqlite3_व्यंजक_Is_Vector(स्थिर व्यंजक *p_व्यंजक);
व्यंजक *sqlite3VectorFieldSubexpr(व्यंजक*, पूर्णांक);
व्यंजक *sqlite3_व्यंजक_For_Vector_Field(Parse*,व्यंजक*,पूर्णांक,पूर्णांक);
व्योम sqlite3_Vector_Error_संदेश(Parse*, व्यंजक*);

#अगर_अघोषित SQLITE_OMIT_COMPILEOPTION_DIAGS
स्थिर अक्षर **sqlite3CompileOptions(पूर्णांक *pnOpt);
#पूर्ण_अगर

#अगर SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
पूर्णांक sqlite3KvvfsInit(व्योम);
#पूर्ण_अगर

#अगर defined(VDBE_PROFILE) \
 || defined(SQLITE_PERFORMANCE_TRACE) \
 || defined(SQLITE_ENABLE_STMT_SCANSTATUS)
sqlite3_uint64 sqlite3Hwtime(व्योम);
#पूर्ण_अगर

#अगर_घोषित SQLITE_ENABLE_STMT_SCANSTATUS
# define IS_STMT_SCANSTATUS(db) (db->flags & SQLITE_StmtScanStatus)
#अन्यथा
# define IS_STMT_SCANSTATUS(db) 0
#पूर्ण_अगर

#पूर्ण_अगर /* SQLITEINT_H */
