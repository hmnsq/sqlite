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
#که_نه_تعریف_شوی SQLITEINT_H
#تعریف_کړه SQLITEINT_H

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
#که_نه_تعریف_شوی SQLITE_TCLAPI
#  define SQLITE_TCLAPI
#د_که_پای

/*
** Include the header file used to customize the compiler options for MSVC.
** This should be done first so that it can successfully prevent spurious
** compiler warnings due to subsequent content in this file and other files
** that are included by this file.
*/
#ضمن "msvc.h"

/*
** Special setup for VxWorks
*/
#ضمن "vxworks.h"

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
#که_نه_تعریف_شوی SQLITE_DISABLE_LFS
# define _LARGE_FILE       1
# ifndef FILE_دننه_واټن_بټونه
#   define FILE_دننه_واټن_بټونه 64
# endif
# define LARGEFILE_سرچینه 1
#د_که_پای

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
#إذا defined(__GNUC__) && !defined(SQLITE_DISABLE_INTRINSIC)
# define GCC_نسخه (__GNUC__*1000000+__GNUC_MINOR__*1000+__GNUC_PATCHLEVEL__)
#وإلا
# define GCC_نسخه 0
#د_که_پای
#إذا defined(_MSC_VER) && !defined(SQLITE_DISABLE_INTRINSIC)
# define MSVC_نسخه _MSC_VER
#وإلا
# define MSVC_نسخه 0
#د_که_پای

/*
** Some C99 functions in "math.h" are only present for MSVC when its version
** is associated with Visual Studio 2013 or higher.
*/
#که_نه_تعریف_شوی SQLITE_HAVE_C99_MATH_FUNCS
# إذا MSVC_نسخه==0 || MSVC_نسخه>=1800
#  define SQLITE_HAVE_C99_MATH_FUNCS (1)
# وإلا
#  define SQLITE_HAVE_C99_MATH_FUNCS (0)
# endif
#د_که_پای

/* Needed for various definitions... */
#إذا defined(__GNUC__) && !defined(GNU_سرچینه)
# define GNU_سرچینه
#د_که_پای

#إذا defined(__OpenBSD__) && !defined(BSD_سرچینه)
# define BSD_سرچینه
#د_که_پای

/*
** Macro to disable warnings about missing "break" at the end of a "case".
*/
#إذا defined(__has_attribute)
#  إذا __has_attribute(fallthrough)
#    define deliberate_fall_through __attribute__((fallthrough));
#  endif
#د_که_پای
#إذا !defined(deliberate_fall_through)
#  define deliberate_fall_through
#د_که_پای

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
#إذا defined(_HAVE_MINGW_H)
# include "mingw.h"
#وإلاإذا defined(_HAVE__MINGW_H)
# include "_mingw.h"
#د_که_پای

/*
** For MinGW version 4.x (and higher), check to see if the _USE_32BIT_TIME_T
** define is required to maintain binary compatibility with the MSVC runtime
** library in use (e.g. for Windows XP).
*/
#إذا !defined(USE_32_بټ_وخت_T) && !defined(USE_64_بټ_وخت_T) && \
    defined(_WIN32) && !defined(_WIN64) && \
    defined(__MINGW_MAJOR_VERSION) && __MINGW_MAJOR_VERSION >= 4 && \
    defined(__MSVCRT__)
# define USE_32_بټ_وخت_T
#د_که_پای

/* Optionally #include a user-defined header, whereby compilation options
** may be set prior to where they take effect, but after platform setup.
** If SQLITE_CUSTOM_INCLUDE=? is defined, its value names the #include
** file.
*/
#که_تعریف_شوی SQLITE_CUSTOM_INCLUDE
# define INC_STRINGIFY_(f) #f
# define INC_STRINGIFY(f) INC_STRINGIFY_(f)
# include INC_STRINGIFY(SQLITE_CUSTOM_INCLUDE)
#د_که_پای

/* The public SQLite interface.  The _FILE_OFFSET_BITS macro must appear
** first in QNX.  Also, the _USE_32BIT_TIME_T macro must appear first for
** MinGW.
*/
#ضمن "sqlite3.h"

/*
** Reuse the STATIC_VFS1 for mutex access to sqlite3_temp_directory.
*/
#تعریف_کړه SQLITE_MUTEX_STATIC_TEMPDIR SQLITE_MUTEX_STATIC_VFS1

/*
** Include the configuration header output by 'configure' if we're using the
** autoconf-based build
*/
#إذا defined(_HAVE_SQLITE_CONFIG_H) && !defined(SQLITECONFIG_H)
#ضمن "sqlite_cfg.h"
#تعریف_کړه SQLITECONFIG_H 1
#د_که_پای

#ضمن "sqliteLimit.h"

/* Disable nuisance warnings on Borland compilers */
#إذا defined(__BORLANDC__)
#لارښوونه warn -rch /* unreachable code */
#لارښوونه warn -ccc /* Condition is always true or false */
#لارښوونه warn -aus /* Assigned value is never used */
#لارښوونه warn -csu /* Comparing signed and unsigned */
#لارښوونه warn -spa /* Suspicious pointer arithmetic */
#د_که_پای

/*
** A few places in the code require atomic load/store of aligned
** integer values.
*/
#که_نه_تعریف_شوی __has_extension
# define __has_extension(x) 0     /* compatibility with non-clang compilers */
#د_که_پای
#إذا GCC_نسخه>=4007000 || __has_extension(c_atomic)
# define SQLITE_ATOMIC_INTRINSICS 1
# define Atomic_بارول(PTR)       __atomic_load_n((PTR),__ATOMIC_RELAXED)
# define AtomicStore(PTR,VAL)  __atomic_store_n((PTR),(VAL),__ATOMIC_RELAXED)
#وإلا
# define SQLITE_ATOMIC_INTRINSICS 0
# define Atomic_بارول(PTR)       (*(PTR))
# define AtomicStore(PTR,VAL)  (*(PTR) = (VAL))
#د_که_پای

/*
** Include standard header files as necessary
*/
#ضمن <stdint.h>
#که_تعریف_شوی HAVE_INTTYPES_H
#ضمن <inttypes.h>
#د_که_پای

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
#إذا defined(HAVE_STDINT_H)   /* Use this case if we have ANSI headers */
# define SQLITE_INT_منزل_PTR(X)  ((فراغ*)(intptr_t)(X))
# define SQLITE_PTR_منزل_INT(X)  ((صحيح)(intptr_t)(X))
#وإلاإذا defined(__PTRDIFF_TYPE__)  /* This case should work for GCC */
# define SQLITE_INT_منزل_PTR(X)  ((فراغ*)(__PTRDIFF_TYPE__)(X))
# define SQLITE_PTR_منزل_INT(X)  ((صحيح)(__PTRDIFF_TYPE__)(X))
#وإلاإذا !defined(__GNUC__)       /* Works for compilers other than LLVM */
# define SQLITE_INT_منزل_PTR(X)  ((فراغ*)&((حرف*)0)[X])
# define SQLITE_PTR_منزل_INT(X)  ((صحيح)(((حرف*)X)-(حرف*)0))
#وإلا                          /* Generates a warning - but it always works */
# define SQLITE_INT_منزل_PTR(X)  ((فراغ*)(X))
# define SQLITE_PTR_منزل_INT(X)  ((صحيح)(X))
#د_که_پای

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
#إذا defined(SQLITE_DISABLE_INLINE)
#  define SQLITE_NOINLINE
#  define SQLITE_INLINE
#  define SQLITE_OPT_INLINE
#  define SQLITE_کاروي_INLINE 0
#وإلاإذا defined(__GNUC__)
#  define SQLITE_NOINLINE    __attribute__((noinline))
#  define SQLITE_INLINE      __attribute__((always_inline)) مضمن
#  define SQLITE_OPT_INLINE  __attribute__((always_inline)) مضمن
#  define SQLITE_کاروي_INLINE 1
#وإلاإذا defined(_MSC_VER) && _MSC_VER>=1310
#  define SQLITE_NOINLINE    __declspec(noinline)
#  define SQLITE_INLINE      __forceinline
#  define SQLITE_OPT_INLINE  __forceinline
#  define SQLITE_کاروي_INLINE 1
#وإلا
#  define SQLITE_NOINLINE
#  define SQLITE_INLINE
#  define SQLITE_OPT_INLINE
#  define SQLITE_کاروي_INLINE 0
#د_که_پای
#إذا defined(SQLITE_COVERAGE_TEST) || defined(__STRICT_ANSI__)
# undef SQLITE_INLINE
# define SQLITE_INLINE
# undef SQLITE_OPT_INLINE
# define SQLITE_OPT_INLINE
# undef SQLITE_کاروي_INLINE
# define SQLITE_کاروي_INLINE 0
#د_که_پای
#إذا !defined(SQLITE_AMALGAMATION)
# undef SQLITE_OPT_INLINE
# define SQLITE_OPT_INLINE
#د_که_پای

/*
** Make sure that the compiler intrinsics we desire are enabled when
** compiling with an appropriate version of MSVC unless prevented by
** the SQLITE_DISABLE_INTRINSIC define.
*/
#إذا !defined(SQLITE_DISABLE_INTRINSIC)
#  إذا defined(_MSC_VER) && _MSC_VER>=1400
#    إذا !defined(_WIN32_WCE)
#      include <intrin.h>
#      pragma intrinsic(_byteswap_ushort)
#      pragma intrinsic(_byteswap_ulong)
#      pragma intrinsic(_byteswap_uint64)
#      pragma intrinsic(_ReadWriteBarrier)
#    وإلا
#      include <cmnintrin.h>
#    endif
#  endif
#د_که_پای

/*
** Enable SQLITE_USE_SEH by default on MSVC builds.  Only omit
** SEH support if the -DSQLITE_OMIT_SEH option is given.
*/
#إذا defined(_MSC_VER) && !defined(SQLITE_OMIT_SEH)
# define SQLITE_USE_SEH 1
#وإلا
# undef SQLITE_USE_SEH
#د_که_پای

/*
** Enable SQLITE_DIRECT_OVERFLOW_READ, unless the build explicitly
** disables it using -DSQLITE_DIRECT_OVERFLOW_READ=0
*/
#إذا defined(SQLITE_مستقیم_ډکېدنه_READ) && SQLITE_مستقیم_ډکېدنه_READ+1==1
  /* Disable if -DSQLITE_DIRECT_OVERFLOW_READ=0 */
# undef SQLITE_مستقیم_ډکېدنه_READ
#وإلا
  /* In all other cases, enable */
# define SQLITE_مستقیم_ډکېدنه_READ 1
#د_که_پای


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
#إذا !defined(SQLITE_THREADSAFE)
# إذا defined(THREADSAFE)
#   define SQLITE_THREADSAFE THREADSAFE
# وإلا
#   define SQLITE_THREADSAFE 1 /* IMP: R-07272-22309 */
# endif
#د_که_پای

/*
** Powersafe overwrite is on by default.  But can be turned off using
** the -DSQLITE_POWERSAFE_OVERWRITE=0 command-line option.
*/
#که_نه_تعریف_شوی SQLITE_POWERSAFE_OVERWRITE
# define SQLITE_POWERSAFE_OVERWRITE 1
#د_که_پای

/*
** EVIDENCE-OF: R-25715-37072 Memory allocation statistics are enabled by
** default unless SQLite is compiled with SQLITE_DEFAULT_MEMSTATUS=0 in
** which case memory allocation statistics are disabled by default.
*/
#إذا !defined(SQLITE_DEFAULT_MEMSTATUS)
# define SQLITE_DEFAULT_MEMSTATUS 1
#د_که_پای

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
#إذا defined(SQLITE_SYSTEM_MALLOC) \
  + defined(SQLITE_WIN32_MALLOC) \
  + defined(SQLITE_ZERO_MALLOC) \
  + defined(SQLITE_MEMDEBUG)>1
# error "Two or نور د the following compile-وخت configuration options\
 are defined but at most one is allowed:\
 SQLITE_SYSTEM_MALLOC, SQLITE_WIN32_MALLOC, SQLITE_MEMDEBUG,\
 SQLITE_ZERO_MALLOC"
#د_که_پای
#إذا defined(SQLITE_SYSTEM_MALLOC) \
  + defined(SQLITE_WIN32_MALLOC) \
  + defined(SQLITE_ZERO_MALLOC) \
  + defined(SQLITE_MEMDEBUG)==0
# define SQLITE_SYSTEM_MALLOC 1
#د_که_پای

/*
** If SQLITE_MALLOC_SOFT_LIMIT is not zero, then try to keep the
** sizes of memory allocations below this value where possible.
*/
#إذا !defined(SQLITE_MALLOC_SOFT_حد)
# define SQLITE_MALLOC_SOFT_حد 1024
#د_که_پای

/*
** We need to define _XOPEN_SOURCE as follows in order to enable
** recursive mutexes on most Unix systems and fchmod() on OpenBSD.
** But _XOPEN_SOURCE define causes problems for Mac OS X, so omit
** it.
*/
#إذا !defined(XOPEN_سرچینه) && !defined(__DARWIN__) && !defined(__APPLE__)
#  define XOPEN_سرچینه 600
#د_که_پای

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
#إذا !defined(NDEBUG) && !defined(SQLITE_DEBUG)
# define NDEBUG 1
#د_که_پای
#إذا defined(NDEBUG) && defined(SQLITE_DEBUG)
# undef NDEBUG
#د_که_پای

/*
** Enable SQLITE_ENABLE_EXPLAIN_COMMENTS if SQLITE_DEBUG is turned on.
*/
#إذا !defined(SQLITE_ENABLE_EXPLAIN_COMMENTS) && defined(SQLITE_DEBUG)
# define SQLITE_ENABLE_EXPLAIN_COMMENTS 1
#د_که_پای

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
#إذا defined(SQLITE_MUTATION_TEST)
# define testcase(X)
#وإلاإذا defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
# ifndef SQLITE_AMALGAMATION
    خارجي بینښه صحيح sqlite3CoverageCounter;
# endif
# define testcase(X)  إذا( X ){ sqlite3CoverageCounter += (بینښه)__LINE__; }
#وإلا
# define testcase(X)
#د_که_پای

/*
** The TESTONLY macro is used to enclose variable declarations or
** other bits of code that are needed to support the arguments
** within testcase() and assert() macros.
*/
#إذا !defined(NDEBUG) || defined(SQLITE_COVERAGE_TEST)
# define TESTONLY(X)  X
#وإلا
# define TESTONLY(X)
#د_که_پای

/*
** Sometimes we need a small amount of code such as a variable initialization
** to setup for a later assert() statement.  We do not want this code to
** appear when assert() is disabled.  The following macro is therefore
** used to contain that setup code.  The "VVA" acronym stands for
** "Verification, Validation, and Accreditation".  In other words, the
** code within VVA_ONLY() will only run during verification processes.
*/
#که_نه_تعریف_شوی NDEBUG
# define VVA_ONLY(X)  X
#وإلا
# define VVA_ONLY(X)
#د_که_پای

/*
** Disable ALWAYS() and NEVER() (make them pass-throughs) for coverage
** and mutation testing
*/
#إذا defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_MUTATION_TEST)
# define SQLITE_OMIT_AUXILIARY_SAFETY_CHECKS  1
#د_که_پای

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
#إذا defined(SQLITE_OMIT_AUXILIARY_SAFETY_CHECKS)
# define ALWAYS(X)      (1)
# define NEVER(X)       (0)
#وإلاإذا !defined(NDEBUG)
# define ALWAYS(X)      ((X)?1:(أكد(0),0))
# define NEVER(X)       ((X)?(أكد(0),1):0)
#وإلا
# define ALWAYS(X)      (X)
# define NEVER(X)       (X)
#د_که_پای

/*
** Some conditionals are optimizations only.  In other words, if the
** conditionals are replaced with a constant 1 (true) or 0 (false) then
** the correct answer is still obtained, though perhaps not as quickly.
**
** The following macros mark these optimizations conditionals.
*/
#إذا defined(SQLITE_MUTATION_TEST)
# define OK_IF_ALWAYS_TRUE(X)  (1)
# define OK_IF_ALWAYS_FALSE(X) (0)
#وإلا
# define OK_IF_ALWAYS_TRUE(X)  (X)
# define OK_IF_ALWAYS_FALSE(X) (X)
#د_که_پای

/*
** Some malloc failures are only possible if SQLITE_TEST_REALLOC_STRESS is
** defined.  We need to defend against those failures when testing with
** SQLITE_TEST_REALLOC_STRESS, but we don't want the unreachable branches
** during a normal build.  The following macro can be used to disable tests
** that are always false except when SQLITE_TEST_REALLOC_STRESS is set.
*/
#إذا defined(SQLITE_TEST_REALLOC_STRESS)
# define ONLY_IF_REALLOC_STRESS(X)  (X)
#وإلاإذا !defined(NDEBUG)
# define ONLY_IF_REALLOC_STRESS(X)  ((X)?(أكد(0),1):0)
#وإلا
# define ONLY_IF_REALLOC_STRESS(X)  (0)
#د_که_پای

/*
** Declarations used for tracing the operating system interfaces.
*/
#إذا defined(SQLITE_FORCE_OS_TRACE) || defined(SQLITE_TEST) || \
    (defined(SQLITE_DEBUG) && SQLITE_OS_WIN)
  خارجي صحيح sqlite3OSTrace;
# define OSTRACE(X)          إذا( sqlite3OSTrace ) sqlite3DebugPrintf X
# define SQLITE_HAVE_OS_TRACE
#وإلا
# define OSTRACE(X)
# undef  SQLITE_HAVE_OS_TRACE
#د_که_پای

/*
** Is the sqlite3ErrName() function needed in the build?  Currently,
** it is needed by "mutex_w32.c" (when debugging), "os_win.c" (when
** OSTRACE is enabled), and by several "test*.c" files (which are
** compiled using SQLITE_TEST).
*/
#إذا defined(SQLITE_HAVE_OS_TRACE) || defined(SQLITE_TEST) || \
    (defined(SQLITE_DEBUG) && SQLITE_OS_WIN)
# define SQLITE_NEED_ERR_NAME
#وإلا
# undef  SQLITE_NEED_ERR_NAME
#د_که_پای

/*
** SQLITE_ENABLE_EXPLAIN_COMMENTS is incompatible with SQLITE_OMIT_EXPLAIN
*/
#که_تعریف_شوی SQLITE_OMIT_EXPLAIN
# undef SQLITE_ENABLE_EXPLAIN_COMMENTS
#د_که_پای

/*
** SQLITE_OMIT_VIRTUALTABLE implies SQLITE_OMIT_ALTERTABLE
*/
#إذا defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_ALTERTABLE)
# define SQLITE_OMIT_ALTERTABLE
#د_که_پای

#تعریف_کړه SQLITE_DIGIT_SEPARATOR '_'

/*
** Return true (non-zero) if the input is an integer that is too large
** to fit in 32-bits.  This macro is used inside of various testcase()
** macros to verify that we have tested SQLite for large-file support.
*/
#تعریف_کړه IS_BIG_INT(X)  (((X)&~(i64)0xffffffff)!=0)

/*
** The macro unlikely() is a hint that surrounds a boolean
** expression that is usually false.  Macro likely() surrounds
** a boolean expression that is usually true.  These hints could,
** in theory, be used by the compiler to generate better code, but
** currently they are just comments for human readers.
*/
#تعریف_کړه likely(X)    (X)
#تعریف_کړه unlikely(X)  (X)

#ضمن "hash.h"
#ضمن "parse.h"
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <assert.h>
#ضمن <stddef.h>
#ضمن <ctype.h>

/*
** Use a macro to replace memcpy() if compiled with SQLITE_INLINE_MEMCPY.
** This allows better measurements of where memcpy() is used when running
** cachegrind.  But this macro version of memcpy() is very slow so it
** should not be used in production.  This is a performance measurement
** hack only.
*/
#که_تعریف_شوی SQLITE_INLINE_MEMCPY
# define حافظه_نقل(D,S,N) {حرف*xxd=(حرف*)(D);ثابت حرف*xxs=(ثابت حرف*)(S);\
                        صحيح xxn=(N);طالما(xxn-->0)*(xxd++)=*(xxs++);}
#د_که_پای

/*
** If compiling for a processor that lacks floating point support,
** substitute integer for floating-point
*/
#که_تعریف_شوی SQLITE_OMIT_FLOATING_ټکی
# define مزدوج sqlite_int64
# define عائم sqlite_int64
# define fabs(X) ((X)<0?-(X):(X))
# define sqlite3_Is_ډکېدنه(X) 0
# define INFINITY (9223372036854775807LL)
# ifndef SQLITE_BIG_DBL
#   define SQLITE_BIG_DBL (((sqlite3_int64)1)<<50)
# endif
# define SQLITE_OMIT_DATETIME_FUNCS 1
# define SQLITE_OMIT_TRACE 1
# undef SQLITE_MIXED_ENDIAN_64_بټ_FLOAT
# undef SQLITE_HAVE_ISNAN
#د_که_پای
#که_نه_تعریف_شوی SQLITE_BIG_DBL
# define SQLITE_BIG_DBL (1e99)
#د_که_پای

/*
** OMIT_TEMPDB is set to 1 if SQLITE_OMIT_TEMPDB is defined, or 0
** afterward. Having this macro allows us to cause the C compiler
** to omit code used by TEMP tables without messy #ifndef statements.
*/
#که_تعریف_شوی SQLITE_OMIT_TEMPDB
#تعریف_کړه OMIT_TEMPDB 1
#وإلا
#تعریف_کړه OMIT_TEMPDB 0
#د_که_پای

/*
** The "file format" number is an integer that is incremented whenever
** the VDBE-level file format changes.  The following macros define the
** the default file format for new databases and the maximum file format
** that the library can read.
*/
#تعریف_کړه SQLITE_MAX_FILE_FORMAT 4
#که_نه_تعریف_شوی SQLITE_DEFAULT_FILE_FORMAT
# define SQLITE_DEFAULT_FILE_FORMAT 4
#د_که_پای

/*
** Determine whether triggers are recursive by default.  This can be
** changed at run-time using a pragma.
*/
#که_نه_تعریف_شوی SQLITE_DEFAULT_RECURSIVE_TRIGGERS
# define SQLITE_DEFAULT_RECURSIVE_TRIGGERS 0
#د_که_پای

/*
** Provide a default value for SQLITE_TEMP_STORE in case it is not specified
** on the command-line
*/
#که_نه_تعریف_شوی SQLITE_لنډمهاله_STORE
# define SQLITE_لنډمهاله_STORE 1
#د_که_پای

/*
** If no value has been provided for SQLITE_MAX_WORKER_THREADS, or if
** SQLITE_TEMP_STORE is set to 3 (never use temporary files), set it
** to zero.
*/
#إذا SQLITE_لنډمهاله_STORE==3 || SQLITE_THREADSAFE==0
# undef SQLITE_MAX_کارګر_تارونه
# define SQLITE_MAX_کارګر_تارونه 0
#د_که_پای
#که_نه_تعریف_شوی SQLITE_MAX_کارګر_تارونه
# define SQLITE_MAX_کارګر_تارونه 8
#د_که_پای
#که_نه_تعریف_شوی SQLITE_DEFAULT_کارګر_تارونه
# define SQLITE_DEFAULT_کارګر_تارونه 0
#د_که_پای
#إذا SQLITE_DEFAULT_کارګر_تارونه>SQLITE_MAX_کارګر_تارونه
# undef SQLITE_MAX_کارګر_تارونه
# define SQLITE_MAX_کارګر_تارونه SQLITE_DEFAULT_کارګر_تارونه
#د_که_پای

/*
** The default initial allocation for the pagecache when using separate
** pagecaches for each database connection.  A positive number is the
** number of pages.  A negative number N translations means that a buffer
** of -1024*N bytes is allocated and used for as many pages as it will hold.
**
** The default value of "20" was chosen to minimize the run-time of the
** speedtest1 test program with options: --shrink-memory --reprepare
*/
#که_نه_تعریف_شوی SQLITE_DEFAULT_PCACHE_INITSZ
# define SQLITE_DEFAULT_PCACHE_INITSZ 20
#د_که_پای

/*
** Default value for the SQLITE_CONFIG_SORTERREF_SIZE option.
*/
#که_نه_تعریف_شوی SQLITE_DEFAULT_SORTERREF_اندازه
# define SQLITE_DEFAULT_SORTERREF_اندازه 0x7fffffff
#د_که_پای

/*
** The compile-time options SQLITE_MMAP_READWRITE and
** SQLITE_ENABLE_BATCH_ATOMIC_WRITE are not compatible with one another.
** You must choose one or the other (or neither) but not both.
*/
#إذا defined(SQLITE_MMAP_READWRITE) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
#تېروتنه Cannot use both SQLITE_MMAP_READWRITE and SQLITE_ENABLE_BATCH_ATOMIC_WRITE
#د_که_پای

/*
** GCC does not define the offsetof() macro so we'll have to do it
** ourselves.
*/
#که_نه_تعریف_شوی offsetof
# define offsetof(ST,M) ((size_t)((حرف*)&((ST*)0)->M - (حرف*)0))
#د_که_پای

/*
** sizeof64() is like sizeof(), but always returns a 64-bit value, even
** on 32-bit builds. This can help to avoid overflow by ensuring 64-bit
** arithmetic is used consistently in both 32-bit and 64-bit builds.
*/
#تعریف_کړه sizeof64(X) ((sqlite3_int64)حجم(X))

/*
** Work around C99 "flex-array" syntax for pre-C99 compilers, so as
** to avoid complaints from -fsanitize=strict-bounds.
*/
#إذا defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define FLEXARRAY
#وإلا
# define FLEXARRAY 1
#د_که_پای

/*
** Macros to compute minimum and maximum of two numbers.
*/
#که_نه_تعریف_شوی MIN
# define MIN(A,B) ((A)<(B)?(A):(B))
#د_که_پای
#که_نه_تعریف_شوی MAX
# define MAX(A,B) ((A)>(B)?(A):(B))
#د_که_پای

/*
** Swap two objects of type TYPE.
*/
#تعریف_کړه بدلون(TYPE,A,B) {TYPE t=A; A=B; B=t;}

/*
** Check to see if this machine uses EBCDIC.  (Yes, believe it or
** not, there are still machines out there that use EBCDIC.)
*/
#إذا 'A' == '\301'
# define SQLITE_EBCDIC 1
#وإلا
# define SQLITE_ASCII 1
#د_که_پای

/*
** Integers of known sizes.  These typedefs might change for architectures
** where the sizes very.  Preprocessor macros are available so that the
** types can be conveniently redefined at compile-type.  Like this:
**
**         cc '-DUINTPTR_TYPE=long long int' ...
*/
#که_نه_تعریف_شوی UINT32_TYPE
# ifdef HAVE_UINT32_T
#  define UINT32_TYPE uint32_t
# وإلا
#  define UINT32_TYPE بینښه صحيح
# endif
#د_که_پای
#که_نه_تعریف_شوی UINT16_TYPE
# ifdef HAVE_UINT16_T
#  define UINT16_TYPE uint16_t
# وإلا
#  define UINT16_TYPE بینښه لنډ صحيح
# endif
#د_که_پای
#که_نه_تعریف_شوی INT16_TYPE
# ifdef HAVE_INT16_T
#  define INT16_TYPE int16_t
# وإلا
#  define INT16_TYPE لنډ صحيح
# endif
#د_که_پای
#که_نه_تعریف_شوی UINT8_TYPE
# ifdef HAVE_UINT8_T
#  define UINT8_TYPE uint8_t
# وإلا
#  define UINT8_TYPE بینښه حرف
# endif
#د_که_پای
#که_نه_تعریف_شوی INT8_TYPE
# ifdef HAVE_INT8_T
#  define INT8_TYPE int8_t
# وإلا
#  define INT8_TYPE نښهدار حرف
# endif
#د_که_پای
تعريفنوع sqlite_int64 i64;          /* 8-byte signed integer */
تعريفنوع sqlite_uint64 u64;         /* 8-byte unsigned integer */
تعريفنوع UINT32_TYPE u32;           /* 4-byte unsigned integer */
تعريفنوع UINT16_TYPE u16;           /* 2-byte unsigned integer */
تعريفنوع INT16_TYPE i16;            /* 2-byte signed integer */
تعريفنوع UINT8_TYPE u8;             /* 1-byte unsigned integer */
تعريفنوع INT8_TYPE i8;              /* 1-byte signed integer */

/* A bitfield type for use inside of structures.  Always follow with :N where
** N is the number of bits.
*/
تعريفنوع بینښه bft;  /* Bit Field Type */

/*
** SQLITE_MAX_U32 is a u64 constant that is the maximum u64 value
** that can be stored in a u32 without loss of data.  The value
** is 0x00000000ffffffff.  But because of quirks of some compilers, we
** have to specify the value in the less intuitive manner shown:
*/
#تعریف_کړه SQLITE_MAX_U32  ((((u64)1)<<32)-1)

/*
** The datatype used to store estimates of the number of rows in a
** table or index.
*/
تعريفنوع u64 tRowcnt;

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
تعريفنوع INT16_TYPE LogEst;
#تعریف_کړه LOGEST_MIN (-32768)
#تعریف_کړه LOGEST_MAX (32767)

/*
** Set the SQLITE_PTRSIZE macro to the number of bytes in a pointer
*/
#که_نه_تعریف_شوی SQLITE_PTRSIZE
# إذا defined(__SIZEOF_POINTER__)
#   define SQLITE_PTRSIZE __SIZEOF_POINTER__
# elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
       defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
      (defined(__APPLE__) && defined(__ppc__)) ||                         \
      (defined(__TOS_AIX__) && !defined(__64BIT__))
#   define SQLITE_PTRSIZE 4
# وإلا
#   define SQLITE_PTRSIZE 8
# endif
#د_که_پای

/* The uptr type is an unsigned integer large enough to hold a pointer
*/
#إذا defined(HAVE_STDINT_H)
  تعريفنوع uintptr_t uptr;
#وإلاإذا SQLITE_PTRSIZE==4
  تعريفنوع u32 uptr;
#وإلا
  تعريفنوع u64 uptr;
#د_که_پای

/*
** The SQLITE_WITHIN(P,S,E) macro checks to see if pointer P points to
** something between S (inclusive) and E (exclusive).
**
** In other words, S is a buffer and E is a pointer to the first byte after
** the end of buffer S.  This macro returns true if P points to something
** contained within the buffer S.
*/
#تعریف_کړه SQLITE_WITHIN(P,S,E)   (((uptr)(P)>=(uptr)(S))&&((uptr)(P)<(uptr)(E)))

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
#تعریف_کړه SQLITE_ډکېدنه(P,S,E) (((uptr)(S)<(uptr)(P))&&((uptr)(E)>(uptr)(P)))

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
#که_نه_تعریف_شوی SQLITE_BYTEORDER  /* Replicate changes at tag-20230904a */
# إذا defined(__BYTE_ORDER__) && __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
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
# وإلا
#   define SQLITE_BYTEORDER 0
# endif
#د_که_پای
#إذا SQLITE_BYTEORDER==4321
# define SQLITE_BIGENDIAN    1
# define SQLITE_LITTLEENDIAN 0
# define SQLITE_UTF16NATIVE  SQLITE_UTF16BE
#وإلاإذا SQLITE_BYTEORDER==1234
# define SQLITE_BIGENDIAN    0
# define SQLITE_LITTLEENDIAN 1
# define SQLITE_UTF16NATIVE  SQLITE_UTF16LE
#وإلا
# ifdef SQLITE_AMALGAMATION
  ثابت صحيح sqlite3one = 1;
# وإلا
  خارجي ثابت صحيح sqlite3one;
# endif
# define SQLITE_BIGENDIAN    (*(حرف *)(&sqlite3one)==0)
# define SQLITE_LITTLEENDIAN (*(حرف *)(&sqlite3one)==1)
# define SQLITE_UTF16NATIVE  (SQLITE_BIGENDIAN?SQLITE_UTF16BE:SQLITE_UTF16LE)
#د_که_پای

/*
** Constants for the largest and smallest possible 64-bit signed integers.
** These macros are designed to work correctly on both 32-bit and 64-bit
** compilers.
*/
#تعریف_کړه تر_ټولو_لوی_INT64  (0xffffffff|(((i64)0x7fffffff)<<32))
#تعریف_کړه تر_ټولو_لوی_UINT64 (0xffffffff|(((u64)0xffffffff)<<32))
#تعریف_کړه تر_ټولو_کوچنی_INT64 (((i64)-1) - تر_ټولو_لوی_INT64)

/*
** Macro SMXV(n) return the maximum value that can be held in variable n,
** assuming n is a signed integer type.  UMXV(n) is similar for unsigned
** integer types.
*/
#تعریف_کړه SMXV(n) ((((i64)1)<<(حجم(n)*8-1))-1)
#تعریف_کړه UMXV(n) ((((i64)1)<<(حجم(n)*8))-1)

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
#تعریف_کړه ROUND8(x)     (((x)+7)&~7)
#إذا SQLITE_PTRSIZE==8
# define ROUND8P(x)   (x)
#وإلا
# define ROUND8P(x)   (((x)+7)&~7)
#د_که_پای

/*
** Round down to the nearest multiple of 8
*/
#تعریف_کړه ROUNDDOWN8(x) ((x)&~7)

/*
** Assert that the pointer X is aligned to an 8-byte boundary.  This
** macro is used only within assert() to verify that the code gets
** all alignment restrictions correct.
**
** Except, if SQLITE_4_BYTE_ALIGNED_MALLOC is defined, then the
** underlying malloc() implementation might return us 4-byte aligned
** pointers.  In that case, only verify 4-byte alignment.
*/
#که_تعریف_شوی SQLITE_4_BYTE_ALIGNED_MALLOC
# define EIGHT_BYTE_ALIGNMENT(X)   ((((uptr)(X) - (uptr)0)&3)==0)
#وإلا
# define EIGHT_BYTE_ALIGNMENT(X)   ((((uptr)(X) - (uptr)0)&7)==0)
#د_که_پای
#تعریف_کړه TWO_BYTE_ALIGNMENT(X)      ((((uptr)(X) - (uptr)0)&1)==0)

/*
** Disable MMAP on platforms where it is known to not work
*/
#إذا defined(__OpenBSD__) || defined(__QNXNTO__)
# undef SQLITE_MAX_MMAP_اندازه
# define SQLITE_MAX_MMAP_اندازه 0
#د_که_پای

/*
** Default maximum size of memory used by memory-mapped I/O in the VFS
*/
#که_تعریف_شوی __APPLE__
# include <TargetConditionals.h>
#د_که_پای
#که_نه_تعریف_شوی SQLITE_MAX_MMAP_اندازه
# إذا defined(__linux__) \
  || defined(_WIN32) \
  || (defined(__APPLE__) && defined(__MACH__)) \
  || defined(__sun) \
  || defined(__FreeBSD__) \
  || defined(__DragonFly__)
#   define SQLITE_MAX_MMAP_اندازه 0x7fff0000  /* 2147418112 */
# وإلا
#   define SQLITE_MAX_MMAP_اندازه 0
# endif
#د_که_پای

/*
** The default MMAP_SIZE is zero on all platforms.  Or, even if a larger
** default MMAP_SIZE is specified at compile-time, make sure that it does
** not exceed the maximum mmap size.
*/
#که_نه_تعریف_شوی SQLITE_DEFAULT_MMAP_اندازه
# define SQLITE_DEFAULT_MMAP_اندازه 0
#د_که_پای
#إذا SQLITE_DEFAULT_MMAP_اندازه>SQLITE_MAX_MMAP_اندازه
# undef SQLITE_DEFAULT_MMAP_اندازه
# define SQLITE_DEFAULT_MMAP_اندازه SQLITE_MAX_MMAP_اندازه
#د_که_پای

/*
** TREETRACE_ENABLED will be either 1 or 0 depending on whether or not
** the Abstract Syntax Tree tracing logic is turned on.
*/
#إذا !defined(SQLITE_AMALGAMATION)
خارجي u32 sqlite3_ونه_Trace;
#د_که_پای
#إذا defined(SQLITE_DEBUG) \
    && (defined(SQLITE_TEST) || defined(SQLITE_ENABLE_SELECTTRACE) \
                             || defined(SQLITE_ENABLE_TREETRACE))
# define TREETRACE_ENABLED 1
# define TREETRACE(K,P,S,X)  \
  إذا(sqlite3_ونه_Trace&(K))   \
    sqlite3DebugPrintf("%u/%d/%p: ",(S)->selId,(P)->addrExplain,(S)),\
    sqlite3DebugPrintf X
#وإلا
# define TREETRACE(K,P,S,X)
# define TREETRACE_ENABLED 0
#د_که_پای

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
خارجي u32 sqlite3WhereTrace;
#إذا defined(SQLITE_DEBUG) \
    && (defined(SQLITE_TEST) || defined(SQLITE_ENABLE_WHERETRACE))
# define WHERETRACE(K,X)  إذا(sqlite3WhereTrace&(K)) sqlite3DebugPrintf X
# define WHERETRACE_ENABLED 1
#وإلا
# define WHERETRACE(K,X)
#د_که_پای

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
تعريفنوع بنية BusyHandler BusyHandler;
بنية BusyHandler {
  صحيح (*xBusyHandler)(فراغ *,صحيح);  /* The busy callback */
  فراغ *pBusyArg;                   /* First arg to busy callback */
  صحيح nBusy;                        /* Incremented with each busy call */
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
#تعریف_کړه LEGACY_SCHEMA_جدول          "sqlite_master"
#تعریف_کړه LEGACY_لنډمهاله_SCHEMA_جدول     "sqlite_temp_master"
#تعریف_کړه PREFERRED_SCHEMA_جدول       "sqlite_schema"
#تعریف_کړه PREFERRED_لنډمهاله_SCHEMA_جدول  "sqlite_temp_schema"


/*
** The root-page of the schema table.
*/
#تعریف_کړه SCHEMA_ریښه    1

/*
** The name of the schema table.  The name is different for TEMP.
*/
#تعریف_کړه SCHEMA_جدول(x) \
    ((!OMIT_TEMPDB)&&(x==1)?LEGACY_لنډمهاله_SCHEMA_جدول:LEGACY_SCHEMA_جدول)

/*
** A convenience macro that returns the number of elements in
** an array.
*/
#تعریف_کړه Array_اندازه(X)    ((صحيح)(حجم(X)/حجم(X[0])))

/*
** Determine if the argument is a power of two
*/
#تعریف_کړه Is_Power_د_Two(X) (((X)&((X)-1))==0)

/*
** The following value as a destructor means to use sqlite3DbFree().
** The sqlite3DbFree() routine requires two parameters instead of the
** one parameter that destructors normally want.  So we have to introduce
** this magic value that the code knows to handle differently.  Any
** pointer will work here as long as it is distinct from SQLITE_STATIC
** and SQLITE_TRANSIENT.
*/
#تعریف_کړه SQLITE_DYNAMIC   ((sqlite3_destructor_type)sqlite3_لیکه_سېټ_Clear)

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
#که_تعریف_شوی SQLITE_OMIT_WSD
  #تعریف_کړه SQLITE_WSD ثابت
  #تعریف_کړه GLOBAL(t,v) (*(t*)sqlite3_wsd_پیدا_کړه((فراغ*)&(v), حجم(v)))
  #تعریف_کړه sqlite3GlobalConfig GLOBAL(بنية Sqlite3Config, sqlite3Config)
  صحيح sqlite3_wsd_init(صحيح N, صحيح J);
  فراغ *sqlite3_wsd_پیدا_کړه(فراغ *K, صحيح L);
#وإلا
  #تعریف_کړه SQLITE_WSD
  #تعریف_کړه GLOBAL(t,v) v
  #تعریف_کړه sqlite3GlobalConfig sqlite3Config
#د_که_پای

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
#تعریف_کړه ناکارول_شوی_PARAMETER(x) (فراغ)(x)
#تعریف_کړه ناکارول_شوی_PARAMETER2(x,y) ناکارول_شوی_PARAMETER(x),ناکارول_شوی_PARAMETER(y)

/*
** Forward references to structures
*/
تعريفنوع بنية AggInfo AggInfo;
تعريفنوع بنية AuthContext AuthContext;
تعريفنوع بنية AutoincInfo AutoincInfo;
تعريفنوع بنية Bitvec Bitvec;
تعريفنوع بنية CollSeq CollSeq;
تعريفنوع بنية ستنه ستنه;
تعريفنوع بنية Cte Cte;
تعريفنوع بنية CteUse CteUse;
تعريفنوع بنية Db Db;
تعريفنوع بنية Db_Client_معلومات Db_Client_معلومات;
تعريفنوع بنية DbFixer DbFixer;
تعريفنوع بنية Schema Schema;
تعريفنوع بنية بیان بیان;
تعريفنوع بنية بیان_List بیان_List;
تعريفنوع بنية FKey FKey;
تعريفنوع بنية FpDecode FpDecode;
تعريفنوع بنية FuncDestructor FuncDestructor;
تعريفنوع بنية FuncDef FuncDef;
تعريفنوع بنية FuncDefHash FuncDefHash;
تعريفنوع بنية IdList IdList;
تعريفنوع بنية شاخص شاخص;
تعريفنوع بنية شاخصي_بیان شاخصي_بیان;
تعريفنوع بنية شاخص_Sample شاخص_Sample;
تعريفنوع بنية کلی_Class کلی_Class;
تعريفنوع بنية کلی_Info کلی_Info;
تعريفنوع بنية Lookaside Lookaside;
تعريفنوع بنية Lookaside_خونه Lookaside_خونه;
تعريفنوع بنية Module Module;
تعريفنوع بنية NameContext NameContext;
تعريفنوع بنية OnOrUsing OnOrUsing;
تعريفنوع بنية Parse Parse;
تعريفنوع بنية ParseCleanup ParseCleanup;
تعريفنوع بنية PreUpdate PreUpdate;
تعريفنوع بنية PrintfArguments PrintfArguments;
تعريفنوع بنية RCStr RCStr;
تعريفنوع بنية RenameToken RenameToken;
تعريفنوع بنية Returning Returning;
تعريفنوع بنية لیکه_سېټ لیکه_سېټ;
تعريفنوع بنية Savepoint Savepoint;
تعريفنوع بنية Select Select;
تعريفنوع بنية SQLiteThread SQLiteThread;
تعريفنوع بنية SelectDest SelectDest;
تعريفنوع بنية Subquery Subquery;
تعريفنوع بنية Src_توکی Src_توکی;
تعريفنوع بنية SrcList SrcList;
تعريفنوع بنية sqlite3_str StrAccum; /* Internal alias for sqlite3_str */
تعريفنوع بنية جدول جدول;
تعريفنوع بنية جدول_قلف جدول_قلف;
تعريفنوع بنية Token Token;
تعريفنوع بنية ونه_View ونه_View;
تعريفنوع بنية Trigger Trigger;
تعريفنوع بنية TriggerPrg TriggerPrg;
تعريفنوع بنية Trigger_ګام Trigger_ګام;
تعريفنوع بنية Unpacked_ریکارډ Unpacked_ریکارډ;
تعريفنوع بنية Upsert Upsert;
تعريفنوع بنية VTable VTable;
تعريفنوع بنية VtabCtx VtabCtx;
تعريفنوع بنية Walker Walker;
تعريفنوع بنية WhereInfo WhereInfo;
تعريفنوع بنية کړکۍ کړکۍ;
تعريفنوع بنية With With;


/*
** The bitmask datatype defined below is used for various optimizations.
**
** Changing this from a 64-bit to a 32-bit type limits the number of
** tables in a join to 32 instead of 64.  But it also reduces the size
** of the library by 738 bytes on ix86.
*/
#که_تعریف_شوی SQLITE_BITMASK_TYPE
  تعريفنوع SQLITE_BITMASK_TYPE Bitmask;
#وإلا
  تعريفنوع u64 Bitmask;
#د_که_پای

/*
** The number of bits in a Bitmask.  "BMS" means "BitMask Size".
*/
#تعریف_کړه BMS  ((صحيح)(حجم(Bitmask)*8))

/*
** A bit in a Bitmask
*/
#تعریف_کړه MASKBIT(n)    (((Bitmask)1)<<(n))
#تعریف_کړه MASKBIT64(n)  (((u64)1)<<(n))
#تعریف_کړه MASKBIT32(n)  (((بینښه صحيح)1)<<(n))
#تعریف_کړه SMASKBIT32(n) ((n)<=31?((بینښه صحيح)1)<<(n):0)
#تعریف_کړه ALLBITS       ((Bitmask)-1)
#تعریف_کړه TOPBIT        (((Bitmask)1)<<(BMS-1))

/* A VList object records a mapping between parameters/variables/wildcards
** in the SQL statement (such as $abc, @pqr, or :xyz) and the integer
** variable number associated with that parameter.  See the format description
** on the sqlite3VListAdd() routine for more information.  A VList is really
** just an array of integers.
*/
تعريفنوع صحيح VList;

/*
** Defer sourcing vdbe.h and btree.h until after the "u8" and
** "BusyHandler" typedefs. vdbe.h also requires a few of the opaque
** pointer types (i.e. FuncDef) defined above.
*/
#ضمن "os.h"
#ضمن "pager.h"
#ضمن "btree.h"
#ضمن "vdbe.h"
#ضمن "pcache.h"
#ضمن "mutex.h"

/* The SQLITE_EXTRA_DURABLE compile-time option used to set the default
** synchronous setting to EXTRA.  It is no longer supported.
*/
#که_تعریف_شوی SQLITE_EXTRA_DURABLE
# warning Use SQLITE_DEFAULT_SYNCHRONOUS=3 instead د SQLITE_EXTRA_DURABLE
# define SQLITE_DEFAULT_SYNCHRONOUS 3
#د_که_پای

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
#که_نه_تعریف_شوی SQLITE_DEFAULT_SYNCHRONOUS
# define SQLITE_DEFAULT_SYNCHRONOUS 2
#د_که_پای
#که_نه_تعریف_شوی SQLITE_DEFAULT_WAL_SYNCHRONOUS
# define SQLITE_DEFAULT_WAL_SYNCHRONOUS SQLITE_DEFAULT_SYNCHRONOUS
#د_که_پای

/*
** Each database file to be accessed by the system is an instance
** of the following structure.  There are normally two of these structures
** in the sqlite.aDb[] array.  aDb[0] is the main database file and
** aDb[1] is the database file used to hold temporary tables.  Additional
** databases may be attached.
*/
بنية Db {
  حرف *zDbSName;      /* Name of this database. (schema name, not filename) */
  Btree *pBt;          /* The B*Tree structure for this database file */
  u8 safety_level;     /* How aggressive at syncing data to disk */
  u8 b_Sync_سېټ;         /* True if "PRAGMA synchronous=N" has been run */
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
بنية Schema {
  صحيح schema_cookie;   /* Database schema version number for this file */
  صحيح iGeneration;     /* Generation counter.  Incremented with each change */
  Hash tblHash;        /* All tables indexed by name */
  Hash idxHash;        /* All (named) indices indexed by name */
  Hash trigHash;       /* All triggers indexed by name */
  Hash fkeyHash;       /* All foreign keys by referenced table name */
  جدول *pSeqTab;      /* The sqlite_sequence table used by AUTOINCREMENT */
  u8 file_format;      /* Schema format version for this file */
  u8 enc;              /* Text encoding used by this database */
  u16 schemaFlags;     /* Flags associated with this schema */
  صحيح cache_اندازه;      /* Number of pages to use in the cache */
};

/*
** These macros can be used to test, set, or clear bits in the
** Db.pSchema->flags field.
*/
#تعریف_کړه DbHasProperty(D,I,P)     (((D)->aDb[I].pSchema->schemaFlags&(P))==(P))
#تعریف_کړه DbHasAnyProperty(D,I,P)  (((D)->aDb[I].pSchema->schemaFlags&(P))!=0)
#تعریف_کړه Db_سېټ_Property(D,I,P)     (D)->aDb[I].pSchema->schemaFlags|=(P)
#تعریف_کړه DbClearProperty(D,I,P)   (D)->aDb[I].pSchema->schemaFlags&=~(P)

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
#تعریف_کړه DB_SchemaLoaded    0x0001  /* The schema has been loaded */
#تعریف_کړه DB_UnresetViews    0x0002  /* Some views have defined column names */
#تعریف_کړه DB_ResetWanted     0x0008  /* Reset the schema when nSchemaLock==0 */

/*
** The number of different kinds of things that can be limited
** using the sqlite3_limit() interface.
*/
#تعریف_کړه SQLITE_N_حد (SQLITE_حد_CTES+1)

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
بنية Lookaside {
  u32 bDisable;           /* Only operate the lookaside when zero */
  u16 sz;                 /* Size of each buffer in bytes */
  u16 szTrue;             /* True value of sz, even if disabled */
  u8 bMalloced;           /* True if pStart obtained from sqlite3_malloc() */
  u32 n_خونه;              /* Number of lookaside slots allocated */
  u32 anStat[3];          /* 0: hits.  1: size misses.  2: full misses */
  Lookaside_خونه *pInit;   /* List of buffers not previously used */
  Lookaside_خونه *pFree;   /* List of available buffers */
#که_نه_تعریف_شوی SQLITE_OMIT_TWOSIZE_LOOKASIDE
  Lookaside_خونه *pSmallInit; /* List of small buffers not previously used */
  Lookaside_خونه *pSmallFree; /* List of available small buffers */
  فراغ *p_منځ;          /* First byte past end of full-size buffers and
                          ** the first byte of LOOKASIDE_SMALL buffers */
#د_که_پای /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  فراغ *p_پیل;           /* First byte of available memory space */
  فراغ *p_پای_ټکی;             /* First byte past end of available space */
  فراغ *p_True_پای_ټکی;         /* True value of pEnd, when db->pnBytesFreed!=0 */
};
بنية Lookaside_خونه {
  Lookaside_خونه *p_بل;    /* Next buffer in the list of free buffers */
};

#تعریف_کړه DisableLookaside  db->lookaside.bDisable++;db->lookaside.sz=0
#تعریف_کړه EnableLookaside   db->lookaside.bDisable--;\
   db->lookaside.sz=db->lookaside.bDisable?0:db->lookaside.szTrue

/* Size of the smaller allocations in two-size lookaside */
#که_تعریف_شوی SQLITE_OMIT_TWOSIZE_LOOKASIDE
#  define LOOKASIDE_SMALL           0
#وإلا
#  define LOOKASIDE_SMALL         128
#د_که_پای

/*
** A hash table for built-in function definitions.  (Application-defined
** functions use a regular table table from hash.h.)
**
** Hash each FuncDef structure into one of the FuncDefHash.a[] slots.
** Collisions are on the FuncDef.u.pHash chain.  Use the SQLITE_FUNC_HASH()
** macro to compute a hash on the function name.
*/
#تعریف_کړه SQLITE_FUNC_HASH_SZ 23
بنية FuncDefHash {
  FuncDef *a[SQLITE_FUNC_HASH_SZ];       /* Hash table for functions */
};
#تعریف_کړه SQLITE_FUNC_HASH(C,L) (((C)+(L))%SQLITE_FUNC_HASH_SZ)

/*
** typedef for the authorization callback function.
*/
تعريفنوع صحيح (*sqlite3_xauth)(فراغ*,صحيح,ثابت حرف*,ثابت حرف*,ثابت حرف*,
                             ثابت حرف*);

#که_نه_تعریف_شوی SQLITE_OMIT_DEPRECATED
/* This is an extra SQLITE_TRACE macro that indicates "legacy" tracing
** in the style of sqlite3_trace()
*/
#تعریف_کړه SQLITE_TRACE_LEGACY          0x40     /* Use the legacy xTrace */
#تعریف_کړه SQLITE_TRACE_XPROFILE        0x80     /* Use the legacy xProfile */
#وإلا
#تعریف_کړه SQLITE_TRACE_LEGACY          0
#تعریف_کړه SQLITE_TRACE_XPROFILE        0
#د_که_پای /* SQLITE_OMIT_DEPRECATED */
#تعریف_کړه SQLITE_TRACE_NONLEGACY_ماسک  0x0f     /* Normal flags */

/*
** Maximum number of sqlite3.aDb[] entries.  This is the number of attached
** databases plus 2 for "main" and "temp".
*/
#تعریف_کړه SQLITE_MAX_DB (SQLITE_MAX_ATTACHED+2)

/*
** Each database connection is an instance of the following structure.
*/
بنية sqlite3 {
  sqlite3_vfs *pVfs;            /* OS Interface */
  بنية Vdbe *pVdbe;           /* List of active virtual machines */
  CollSeq *pDfltColl;           /* BINARY collseq for the database encoding */
  sqlite3_mutex *mutex;         /* Connection mutex */
  Db *aDb;                      /* All backends */
  صحيح nDb;                      /* Number of backends currently in use */
  u32 mDbFlags;                 /* flags recording internal state */
  u64 flags;                    /* flags settable by pragmas. See below */
  i64 وروستی_Rowid;                /* ROWID of most recent insert (see above) */
  i64 szMmap;                   /* Default mmap_size setting */
  u32 n_Schema_قلف;              /* Do not reset the schema when non-zero */
  بینښه صحيح openFlags;       /* Flags passed to sqlite3_vfs.xOpen() */
  صحيح err_کوډ;                  /* Most recent error code (SQLITE_*) */
  صحيح err_Byte_دننه_واټن;            /* Byte offset of error in SQL statement */
  صحيح err_ماسک;                  /* & result codes with this before returning */
  صحيح iSysErrno;                /* Errno value from last system error */
  u8 enc;                       /* Text encoding */
  u8 autoCommit;                /* The auto-commit flag. */
  u8 لنډمهاله_store;                /* 1: file 2: memory 0: default */
  u8 mallocFailed;              /* True if we have seen a malloc failure */
  u8 bBenignMalloc;             /* Do not require OOMs if true */
  u8 dflt_قلف_ډېرپېښ;              /* Default locking-mode for attached dbs */
  نښهدار حرف بل_Autovac;      /* Autovac setting after VACUUM if >=0 */
  u8 suppressErr;               /* Do not issue error messages if true */
  u8 vtabOnConflict;            /* Value to return for s3_vtab_on_conflict() */
  u8 isTransactionSavepoint;    /* True if the outermost savepoint is a TS */
  u8 mTrace;                    /* zero or more SQLITE_TRACE flags */
  u8 noSharedCache;             /* True if no shared-cache backends */
  u8 nSqlExec;                  /* Number of pending OP_SqlExec opcodes */
  u8 eOpenState;                /* Current condition of the connection */
  u8 nFpDigit;                  /* Significant digits to keep on double->text */
  صحيح بل_Pagesize;             /* Pagesize after VACUUM if >0 */
  i64 nChange;                  /* Value returned by sqlite3_changes() */
  i64 n_ټول_Change;             /* Value returned by sqlite3_total_changes() */
  u32 dbOptFlags;               /* Flags to enable/disable optimizations */
  صحيح a_حد[SQLITE_N_حد];   /* Limits */
  صحيح nMaxSorterMmap;           /* Maximum size of regions mapped by sorter */
  بنية sqlite3InitInfo {      /* Information used during initialization */
    Pgno newTnum;               /* Rootpage of table being initialized */
    u8 iDb;                     /* Which db file is being initialized */
    u8 busy;                    /* TRUE if initing. 2 if due to ADD COLUMN */
    بینښه orphanTrigger : 1; /* Last statement is orphaned TEMP trigger */
    بینښه imposter_جدول : 2; /* Building an imposter table */
    بینښه reopenMemdb : 1;   /* ATTACH is really a reopen using MemDB */
    ثابت حرف **azInit;        /* "type", "name", and "tbl_name" columns */
  } init;
  صحيح n_Vdbe_فعال;              /* Number of VDBEs currently running */
  صحيح nVdbeRead;                /* Number of active VDBEs that read or write */
  صحيح nVdbeWrite;               /* Number of active VDBEs that read and write */
  صحيح nVdbeExec;                /* Number of nested calls to VdbeExec() */
  صحيح nVDestroy;                /* Number of active OP_VDestroy operations */
  صحيح nExtension;               /* Number of loaded extensions */
  فراغ **aExtension;            /* Array of shared library handles */
  اتحاد {
    فراغ (*xLegacy)(فراغ*,ثابت حرف*);   /* mTrace==SQLITE_TRACE_LEGACY */
    صحيح (*xV2)(u32,فراغ*,فراغ*,فراغ*);    /* All other mTrace values */
  } trace;
  فراغ *pTraceArg;                        /* Argument to the trace function */
#که_نه_تعریف_شوی SQLITE_OMIT_DEPRECATED
  فراغ (*xProfile)(فراغ*,ثابت حرف*,u64);  /* Profiling function */
  فراغ *pProfileArg;                        /* Argument to profile function */
#د_که_پای
  فراغ *pCommitArg;                 /* Argument to xCommitCallback() */
  صحيح (*xCommitCallback)(فراغ*);    /* Invoked at every commit. */
  فراغ *pRollbackArg;               /* Argument to xRollbackCallback() */
  فراغ (*xRollbackCallback)(فراغ*); /* Invoked at every commit. */
  فراغ *pUpdateArg;
  فراغ (*xUpdateCallback)(فراغ*,صحيح, ثابت حرف*,ثابت حرف*,sqlite_int64);
  فراغ *p_Autovac_پاڼې_Arg;           /* Client argument to autovac_pages */
  فراغ (*xAutovacDestr)(فراغ*);     /* Destructor for pAutovacPAgesArg */
  بینښه صحيح (*x_Autovac_پاڼې)(فراغ*,ثابت حرف*,u32,u32,u32);
  Parse *pParse;                /* Current parse */
#که_تعریف_شوی SQLITE_ENABLE_PREUPDATE_HOOK
  فراغ *pPreUpdateArg;          /* First argument to xPreUpdateCallback */
  فراغ (*xPreUpdateCallback)(   /* Registered using sqlite3_preupdate_hook() */
    فراغ*,sqlite3*,صحيح,حرف ثابت*,حرف ثابت*,sqlite3_int64,sqlite3_int64
  );
  PreUpdate *pPreUpdate;        /* Context for active pre-update callback */
#د_که_پای /* SQLITE_ENABLE_PREUPDATE_HOOK */
#که_نه_تعریف_شوی SQLITE_OMIT_WAL
  صحيح (*xWalCallback)(فراغ *, sqlite3 *, ثابت حرف *, صحيح);
  فراغ *pWalArg;
#د_که_پای
  فراغ(*xCollNeeded)(فراغ*,sqlite3*,صحيح e_متن_Rep,ثابت حرف*);
  فراغ(*xCollNeeded16)(فراغ*,sqlite3*,صحيح e_متن_Rep,ثابت فراغ*);
  فراغ *pCollNeededArg;
  sqlite3_ارزښت *pErr;          /* Most recent error message */
  اتحاد {
    متطاير صحيح isInterrupted; /* True if sqlite3_interrupt has been called */
    مزدوج notUsed1;            /* Spacer */
  } u1;
  Lookaside lookaside;          /* Lookaside malloc configuration */
#که_نه_تعریف_شوی SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth;          /* Access authorization function */
  فراغ *pAuthArg;               /* 1st argument to the access auth function */
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_پرمختګ_CALLBACK
  صحيح (*x_پرمختګ)(فراغ *);     /* The progress callback */
  فراغ *p_پرمختګ_Arg;           /* Argument to the progress callback */
  بینښه n_پرمختګ_Ops;        /* Number of opcodes for progress callback */
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
  صحيح nVTrans;                  /* Allocated size of aVTrans */
  Hash aModule;                 /* populated by sqlite3_create_module() */
  VtabCtx *pVtabCtx;            /* Context for active vtab connect/create */
  VTable **aVTrans;             /* Virtual tables with open transactions */
  VTable *pDisconnect;          /* Disconnect these in next sqlite3_prepare() */
#د_که_پای
  Hash aFunc;                   /* Hash table of connection functions */
  Hash aCollSeq;                /* All collating sequences */
  BusyHandler busyHandler;      /* Busy callback */
  Db aDbStatic[2];              /* Static space for the 2 default backends */
  Savepoint *pSavepoint;        /* List of active savepoints */
  صحيح n_Analysis_حد;           /* Number of index rows to ANALYZE */
  صحيح busyTimeout;              /* Busy handler timeout, in msec */
#که_تعریف_شوی SQLITE_ENABLE_SETLK_TIMEOUT
  صحيح setlkTimeout;             /* Blocking lock timeout, in msec. -1 -> inf. */
  صحيح setlkFlags;               /* Flags passed to setlk_timeout() */
#د_که_پای
  صحيح nSavepoint;               /* Number of non-transaction savepoints */
  صحيح nStatement;               /* Number of nested statement-transactions  */
  i64 nDeferredCons;            /* Net deferred constraints this transaction. */
  i64 nDeferredImmCons;         /* Net deferred immediate constraints */
  صحيح *pn_بایټونه_Freed;            /* If not NULL, increment this in DbFree() */
  Db_Client_معلومات *p_Db_معلومات;        /* sqlite3_set_clientdata() content */
  u64 nSpill;                   /* TEMP content spilled to disk */
#که_تعریف_شوی SQLITE_ENABLE_UNLOCK_NOTIFY
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
  فراغ *pUnlockArg;                     /* Argument to xUnlockNotify */
  فراغ (*xUnlockNotify)(فراغ **, صحيح);  /* Unlock notify callback */
  sqlite3 *p_بل_Blocked;        /* Next in list of all blocked connections */
#د_که_پای
};

/*
** A macro to discover the encoding of a database.
*/
#تعریف_کړه SCHEMA_ENC(db) ((db)->aDb[0].pSchema->enc)
#تعریف_کړه ENC(db)        ((db)->enc)

/*
** A u64 constant where the lower 32 bits are all zeros.  Only the
** upper 32 bits are included in the argument.  Necessary because some
** C-compilers still do not accept LL integer literals.
*/
#تعریف_کړه HI(X)  ((u64)(X)<<32)

/*
** Possible values for the sqlite3.flags.
**
** Value constraints (enforced via assert()):
**      SQLITE_FullFSync     == PAGER_FULLFSYNC
**      SQLITE_CkptFullFSync == PAGER_CKPT_FULLFSYNC
**      SQLITE_CacheSpill    == PAGER_CACHE_SPILL
*/
#تعریف_کړه SQLITE_WriteSchema    0x00000001  /* OK to update SQLITE_SCHEMA */
#تعریف_کړه SQLITE_LegacyFileFmt  0x00000002  /* Create new databases in format 1 */
#تعریف_کړه SQLITE_ډک_ستون_نومونه   0x00000004  /* Show full column names on SELECT */
#تعریف_کړه SQLITE_ډک_FSync      0x00000008  /* Use full fsync on the backend */
#تعریف_کړه SQLITE_Ckpt_ډک_FSync  0x00000010  /* Use full fsync for checkpoint */
#تعریف_کړه SQLITE_CacheSpill     0x00000020  /* OK to spill pager cache */
#تعریف_کړه SQLITE_Short_ستون_نومونه  0x00000040  /* Show short columns names */
#تعریف_کړه SQLITE_TrustedSchema  0x00000080  /* Allow unsafe functions and
                                          ** vtabs in the schema definition */
#تعریف_کړه SQLITE_NullCallback   0x00000100  /* Invoke the callback once if the */
                                          /*   result set is empty */
#تعریف_کړه SQLITE_IgnoreChecks   0x00000200  /* Do not enforce check constraints */
#تعریف_کړه SQLITE_Stmt_لټون_حالت 0x00000400  /* Enable stmt_scanstats() counters */
#تعریف_کړه SQLITE_NoCkptOnClose  0x00000800  /* No checkpoint on close()/DETACH */
#تعریف_کړه SQLITE_سرچپه_ترتیب   0x00001000  /* Reverse unordered SELECTs */
#تعریف_کړه SQLITE_RecTriggers    0x00002000  /* Enable recursive triggers */
#تعریف_کړه SQLITE_Foreign_کلیګانې    0x00004000  /* Enforce foreign key constraints  */
#تعریف_کړه SQLITE_Auto_شاخص      0x00008000  /* Enable automatic indexes */
#تعریف_کړه SQLITE_بارول_Extension  0x00010000  /* Enable load_extension */
#تعریف_کړه SQLITE_بارول_Ext_Func    0x00020000  /* Enable load_extension() SQL func */
#تعریف_کړه SQLITE_EnableTrigger  0x00040000  /* True to enable triggers */
#تعریف_کړه SQLITE_DeferFKs       0x00080000  /* Defer all FK constraints */
#تعریف_کړه SQLITE_QueryOnly      0x00100000  /* Disable database changes */
#تعریف_کړه SQLITE_Cell_اندازه_Ck     0x00200000  /* Check btree cell sizes on load */
#تعریف_کړه SQLITE_Fts3Tokenizer  0x00400000  /* Enable fts3_tokenizer(2) */
#تعریف_کړه SQLITE_EnableQPSG     0x00800000  /* Query Planner Stability Guarantee*/
#تعریف_کړه SQLITE_TriggerEQP     0x01000000  /* Show trigger EXPLAIN QUERY PLAN */
#تعریف_کړه SQLITE_ResetDatabase  0x02000000  /* Reset the database */
#تعریف_کړه SQLITE_LegacyAlter    0x04000000  /* Legacy ALTER TABLE behaviour */
#تعریف_کړه SQLITE_NoSchemaError  0x08000000  /* Do not report schema parse errors*/
#تعریف_کړه SQLITE_Defensive      0x10000000  /* Input SQL is likely hostile */
#تعریف_کړه SQLITE_DqsDDL         0x20000000  /* dbl-quoted strings allowed in DDL*/
#تعریف_کړه SQLITE_DqsDML         0x40000000  /* dbl-quoted strings allowed in DML*/
#تعریف_کړه SQLITE_EnableView     0x80000000  /* Enable the use of views */
#تعریف_کړه SQLITE_شمېر_لیکې      HI(0x00001) /* Count rows changed by INSERT, */
                                          /*   DELETE, or UPDATE and return */
                                          /*   the count using a callback. */
#تعریف_کړه SQLITE_CorruptRdOnly  HI(0x00002) /* Prohibit writes due to error */
#تعریف_کړه SQLITE_ReadUncommit   HI(0x00004) /* READ UNCOMMITTED in shared-cache */
#تعریف_کړه SQLITE_FkNoAction     HI(0x00008) /* Treat all FK as NO ACTION */
#تعریف_کړه SQLITE_AttachCreate   HI(0x00010) /* ATTACH allowed to create new dbs */
#تعریف_کړه SQLITE_AttachWrite    HI(0x00020) /* ATTACH allowed to open for write */
#تعریف_کړه SQLITE_Comments       HI(0x00040) /* Enable SQL comments */

/* Flags used only if debugging */
#که_تعریف_شوی SQLITE_DEBUG
#تعریف_کړه SQLITE_SqlTrace       HI(0x0100000) /* Debug print SQL as it executes */
#تعریف_کړه SQLITE_VdbeListing    HI(0x0200000) /* Debug listings of VDBE progs */
#تعریف_کړه SQLITE_VdbeTrace      HI(0x0400000) /* True to trace VDBE execution */
#تعریف_کړه SQLITE_VdbeAddopTrace HI(0x0800000) /* Trace sqlite3VdbeAddOp() calls */
#تعریف_کړه SQLITE_VdbeEQP        HI(0x1000000) /* Debug EXPLAIN QUERY PLAN */
#تعریف_کړه SQLITE_ParserTrace    HI(0x2000000) /* PRAGMA parser_trace=ON */
#د_که_پای

/*
** Allowed values for sqlite3.mDbFlags
*/
#تعریف_کړه DBFLAG_SchemaChange   0x0001  /* Uncommitted Hash table changes */
#تعریف_کړه DBFLAG_PreferBuiltin  0x0002  /* Preference to built-in funcs */
#تعریف_کړه DBFLAG_Vacuum         0x0004  /* Currently in a VACUUM */
#تعریف_کړه DBFLAG_VacuumInto     0x0008  /* Currently running VACUUM INTO */
#تعریف_کړه DBFLAG_Schema_معلوم_Ok  0x0010  /* Schema is known to be valid */
#تعریف_کړه DBFLAG_دننی_Func   0x0020  /* Allow use of internal functions */
#تعریف_کړه DBFLAG_EncodingFixed  0x0040  /* No longer possible to change enc. */

/*
** Bits of the sqlite3.dbOptFlags field that are used by the
** sqlite3_test_control(SQLITE_TESTCTRL_OPTIMIZATIONS,...) interface to
** selectively disable various optimizations. The numbers in each macro
** are the one-based bit position (for improved readability).
**
** Keep these values in sync with the ext/misc/optname.c extension.
*/
#تعریف_کړه SQLITE_QueryFlattener  1 /* Query flattening */
#تعریف_کړه SQLITE_کړکۍ_Func      2 /* Use xInverse for window functions */
#تعریف_کړه SQLITE_ډله_By_ترتیب    3 /* GROUPBY cover of ORDERBY */
#تعریف_کړه SQLITE_Join_نه_موندنه        4 /* Unwind join loops on a lookup miss*/
#تعریف_کړه SQLITE_DistinctOpt     5 /* DISTINCT using indexes */
#تعریف_کړه SQLITE_Cover_Idx_لټون    6 /* Covering index scans */
#تعریف_کړه SQLITE_ترتیب_By_Idx_Join  7 /* ORDER BY of joins via index */
#تعریف_کړه SQLITE_متعدي      8 /* Transitive constraints */
#تعریف_کړه SQLITE_OmitNoopJoin    9 /* Omit unused tables in joins */
#تعریف_کړه SQLITE_شمېر_د_View    10 /* The count-of-view optimization */
#تعریف_کړه SQLITE_CursorHints    11 /* Add OP_CursorHint opcodes */
#تعریف_کړه SQLITE_Stat4          12 /* Use STAT4 data */
#تعریف_کړه SQLITE_ټېل_ښکته       13 /* WHERE-clause push-down opt */
#تعریف_کړه SQLITE_SimplifyJoin   14 /* Convert LEFT JOIN to JOIN */
#تعریف_کړه SQLITE_Skip_لټون       15 /* Skip-scans */
#تعریف_کړه SQLITE_PropagateConst 16 /* The constant propagation opt */
#تعریف_کړه SQLITE_MinMaxOpt      17 /* The min/max optimization */
#تعریف_کړه SQLITE_Seek_لټون       18 /* The OP_SeekScan optimization */
#تعریف_کړه SQLITE_Omit_ترتیب_By    19 /* Omit pointless ORDER BY */
#تعریف_کړه SQLITE_BloomFilter    20 /* Use a Bloom filters */
                          /*  21 -- Available for reuse */
#تعریف_کړه SQLITE_متوازن_یوځای  22 /* Balance multi-way merges */
#تعریف_کړه SQLITE_ReleaseReg     23 /* Use OP_ReleaseReg for testing */
#تعریف_کړه SQLITE_FlttnUnionAll  24 /* Disable the UNION ALL flattener */
#تعریف_کړه SQLITE_شاخصي_بیان    25 /* Pull exprs from index when able */
#تعریف_کړه SQLITE_Coroutines     26 /* Co-routines for subqueries */
#تعریف_کړه SQLITE_Null_ناکارول_شوی_ستون_شمېر 27 /* NULL unused columns in subqueries */
#تعریف_کړه SQLITE_OnePass        28 /* Single-pass DELETE and UPDATE */
#تعریف_کړه SQLITE_ترتیب_By_Subq    29 /* ORDER BY in subquery helps outer */
#تعریف_کړه SQLITE_ستوری_Query      30 /* Heurists for star queries */
#تعریف_کړه SQLITE_Exists_منزل_Join   31 /* The EXISTS-to-JOIN optimization */
#تعریف_کړه SQLITE_Union_حد     32 /* Optimizations for UNION + LIMIT */

/*
** Macros for testing whether or not optimizations are enabled or disabled.
*/
#تعریف_کړه OptimizationDisabled(D,M)  (((D)->dbOptFlags&(1ul<<((M)-1)))!=0)
#تعریف_کړه OptimizationEnabled(D,M)   (((D)->dbOptFlags&(1ul<<((M)-1)))==0)

/*
** Return true if it OK to factor constant expressions into the initialization
** code. The argument is a Parse object for the code generator.
*/
#تعریف_کړه Const_ضریب_Ok(P) ((P)->ok_Const_ضریب)

/* Possible values for the sqlite3.eOpenState field.
** The numbers are randomly selected such that a minimum of three bits must
** change to convert any number to another or to zero
*/
#تعریف_کړه SQLITE_STATE_OPEN     0x76  /* Database is open */
#تعریف_کړه SQLITE_STATE_CLOSED   0xce  /* Database is closed */
#تعریف_کړه SQLITE_STATE_SICK     0xba  /* Error and awaiting close */
#تعریف_کړه SQLITE_STATE_BUSY     0x6d  /* Database currently in use */
#تعریف_کړه SQLITE_STATE_ERROR    0xd5  /* An SQLITE_MISUSE error occurred */
#تعریف_کړه SQLITE_STATE_ZOMBIE   0xa7  /* Close with last statement close */

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
بنية FuncDef {
  i16 nArg;            /* Number of arguments.  -1 means unlimited */
  u32 funcFlags;       /* Some combination of SQLITE_FUNC_* */
  فراغ *p_User_معلومات;     /* User data parameter */
  FuncDef *p_بل;      /* Next function with same name */
  فراغ (*xSFunc)(sqlite3_context*,صحيح,sqlite3_ارزښت**); /* func or agg-step */
  فراغ (*xFinalize)(sqlite3_context*);                  /* Agg finalizer */
  فراغ (*x_ارزښت)(sqlite3_context*);                     /* Current agg value */
  فراغ (*xInverse)(sqlite3_context*,صحيح,sqlite3_ارزښت**); /* inverse agg-step */
  ثابت حرف *zName;   /* SQL name of the function. */
  اتحاد {
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
بنية FuncDestructor {
  صحيح nRef;
  فراغ (*xDestroy)(فراغ *);
  فراغ *p_User_معلومات;
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
#تعریف_کړه SQLITE_FUNC_ENCMASK  0x0003 /* SQLITE_UTF8, SQLITE_UTF16BE or UTF16LE */
#تعریف_کړه SQLITE_FUNC_LIKE     0x0004 /* Candidate for the LIKE optimization */
#تعریف_کړه SQLITE_FUNC_CASE     0x0008 /* Case-sensitive LIKE-type function */
#تعریف_کړه SQLITE_FUNC_EPHEM    0x0010 /* Ephemeral.  Delete with VDBE */
#تعریف_کړه SQLITE_FUNC_NEEDCOLL 0x0020 /* sqlite3GetFuncCollSeq() might be called*/
#تعریف_کړه SQLITE_FUNC_اوږدتیا   0x0040 /* Built-in length() function */
#تعریف_کړه SQLITE_FUNC_TYPEOF   0x0080 /* Built-in typeof() function */
#تعریف_کړه SQLITE_FUNC_BYTELEN  0x00c0 /* Built-in octet_length() function */
#تعریف_کړه SQLITE_FUNC_شمېر    0x0100 /* Built-in count(*) aggregate */
/*                           0x0200 -- available for reuse */
#تعریف_کړه SQLITE_FUNC_UNLIKELY 0x0400 /* Built-in unlikely() function */
#تعریف_کړه SQLITE_FUNC_CONSTANT 0x0800 /* Constant inputs give a constant output */
#تعریف_کړه SQLITE_FUNC_MINMAX   0x1000 /* True for min() and max() aggregates */
#تعریف_کړه SQLITE_FUNC_SLOCHNG  0x2000 /* "Slow Change". Value constant during a
                                    ** single query - might change over time */
#تعریف_کړه SQLITE_FUNC_TEST     0x4000 /* Built-in testing functions */
#تعریف_کړه SQLITE_FUNC_RUNONLY  0x8000 /* Cannot be used by valueFromFunction */
#تعریف_کړه SQLITE_FUNC_کړکۍ   0x00010000 /* Built-in window-only function */
#تعریف_کړه SQLITE_FUNC_دننی 0x00040000 /* For use by NestedParse() only */
#تعریف_کړه SQLITE_FUNC_مستقیم   0x00080000 /* Not for use in TRIGGERs or VIEWs */
/* SQLITE_SUBTYPE            0x00100000 // Consumer of subtypes */
#تعریف_کړه SQLITE_FUNC_UNSAFE   0x00200000 /* Function has side effects */
#تعریف_کړه SQLITE_FUNC_INLINE   0x00400000 /* Functions implemented in-line */
#تعریف_کړه SQLITE_FUNC_BUILTIN  0x00800000 /* This is a built-in function */
/*  SQLITE_RESULT_SUBTYPE    0x01000000 // Generator of subtypes */
#تعریف_کړه SQLITE_FUNC_ANYORDER 0x08000000 /* count/min/max aggregate */

/* Identifier numbers for each in-line function */
#تعریف_کړه INLINEFUNC_coalesce             0
#تعریف_کړه INLINEFUNC_پایله_ورکوي_nonnull_لیکه  1
#تعریف_کړه INLINEFUNC_بیان_پایله_ورکوي_بیان    2
#تعریف_کړه INLINEFUNC_بیان_compare         3
#تعریف_کړه INLINEFUNC_affinity             4
#تعریف_کړه INLINEFUNC_iif                  5
#تعریف_کړه INLINEFUNC_sqlite_دننه_واټن        6
#تعریف_کړه INLINEFUNC_unlikely            99  /* Default case */

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
#تعریف_کړه FUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_CONSTANT|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_منزل_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#تعریف_کړه VFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_منزل_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#تعریف_کړه SFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|SQLITE_DIRECTONLY|SQLITE_FUNC_UNSAFE, \
   SQLITE_INT_منزل_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#تعریف_کړه MFUNCTION(zName, nArg, xPtr, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_CONSTANT|SQLITE_UTF8, \
   xPtr, 0, xFunc, 0, 0, 0, #zName, {0} }
#تعریف_کړه JFUNCTION(zName, nArg, bUseCache, bWS, bRS, bJsonB, iArg, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_DETERMINISTIC|SQLITE_FUNC_CONSTANT|\
   SQLITE_UTF8|((bUseCache)*SQLITE_FUNC_RUNONLY)|\
   ((bRS)*SQLITE_SUBTYPE)|((bWS)*SQLITE_پایله_SUBTYPE), \
   SQLITE_INT_منزل_PTR(iArg|((bJsonB)*JSON_BLOB)),0,xFunc,0, 0, 0, #zName, {0} }
#تعریف_کړه INLINE_FUNC(zName, nArg, iArg, mFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_UTF8|SQLITE_FUNC_INLINE|SQLITE_FUNC_CONSTANT|(mFlags), \
   SQLITE_INT_منزل_PTR(iArg), 0, noopFunc, 0, 0, 0, #zName, {0} }
#تعریف_کړه TEST_FUNC(zName, nArg, iArg, mFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
         SQLITE_UTF8|SQLITE_FUNC_دننی|SQLITE_FUNC_TEST| \
         SQLITE_FUNC_INLINE|SQLITE_FUNC_CONSTANT|(mFlags), \
   SQLITE_INT_منزل_PTR(iArg), 0, noopFunc, 0, 0, 0, #zName, {0} }
#تعریف_کړه DFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_SLOCHNG|SQLITE_UTF8, \
   0, 0, xFunc, 0, 0, 0, #zName, {0} }
#تعریف_کړه خالص_DATE(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
         SQLITE_FUNC_SLOCHNG|SQLITE_UTF8|SQLITE_FUNC_CONSTANT, \
   (فراغ*)&sqlite3Config, 0, xFunc, 0, 0, 0, #zName, {0} }
#تعریف_کړه FUNCTION2(zName, nArg, iArg, bNC, xFunc, extraFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_CONSTANT|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL)|extraFlags,\
   SQLITE_INT_منزل_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#تعریف_کړه STR_FUNCTION(zName, nArg, pArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_SLOCHNG|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   pArg, 0, xFunc, 0, 0, 0, #zName, {0} }
#تعریف_کړه LIKEFUNC(zName, nArg, arg, flags) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_CONSTANT|SQLITE_UTF8|flags, \
   (فراغ *)arg, 0, likeFunc, 0, 0, 0, #zName, {0} }
#تعریف_کړه WAGGREGATE(zName, nArg, arg, nc, x_ګام, xFinal, x_ارزښت, xInverse, f) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|(nc*SQLITE_FUNC_NEEDCOLL)|f, \
   SQLITE_INT_منزل_PTR(arg), 0, x_ګام,xFinal,x_ارزښت,xInverse,#zName, {0}}
#تعریف_کړه دننی_FUNCTION(zName, nArg, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_دننی|SQLITE_UTF8|SQLITE_FUNC_CONSTANT, \
   0, 0, xFunc, 0, 0, 0, #zName, {0} }


/*
** All current savepoints are stored in a linked list starting at
** sqlite3.pSavepoint. The first element in the list is the most recently
** opened savepoint. Savepoints are added to the list by the vdbe
** OP_Savepoint instruction.
*/
بنية Savepoint {
  حرف *zName;                        /* Savepoint name (nul-terminated) */
  i64 nDeferredCons;                  /* Number of deferred fk violations */
  i64 nDeferredImmCons;               /* Number of deferred imm fk. */
  Savepoint *p_بل;                   /* Parent savepoint (if any) */
};

/*
** The following are used as the second parameter to sqlite3Savepoint(),
** and as the P1 argument to the OP_Savepoint instruction.
*/
#تعریف_کړه SAVEPOINT_BEGIN      0
#تعریف_کړه SAVEPOINT_RELEASE    1
#تعریف_کړه SAVEPOINT_ROLLBACK   2


/*
** Each SQLite module (virtual table definition) is defined by an
** instance of the following structure, stored in the sqlite3.aModule
** hash table.
*/
بنية Module {
  ثابت sqlite3_module *pModule;       /* Callback pointers */
  ثابت حرف *zName;                   /* Name passed to create_module() */
  صحيح nRefModule;                      /* Number of pointers to this object */
  فراغ *pAux;                          /* pAux passed to create_module() */
  فراغ (*xDestroy)(فراغ *);            /* Module destructor function */
  جدول *pEpoTab;                      /* Eponymous table for this module */
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
بنية ستنه {
  حرف *zCnName;        /* Name of this column */
  بینښه notNull :4;  /* An OE_ code for handling a NOT NULL constraint */
  بینښه eCType :4;   /* One of the standard types */
  حرف affinity;        /* One of the SQLITE_AFF_... values */
  u8 szEst;             /* Est size of value in this column. sizeof(INT)==1 */
  u8 hName;             /* Column name hash for faster lookup */
  u16 iDflt;            /* 1-based index of DEFAULT.  0 means "none" */
  u16 ستون_Flags;         /* Boolean properties.  See COLFLAG_ defines below */
};

/* Allowed values for Column.eCType.
**
** Values must match entries in the global constant arrays
** sqlite3StdTypeLen[] and sqlite3StdType[].  Each value is one more
** than the offset into these arrays for the corresponding name.
** Adjust the SQLITE_N_STDTYPE value if adding or removing entries.
*/
#تعریف_کړه COLTYPE_CUSTOM      0   /* Type appended to zName */
#تعریف_کړه COLTYPE_ANY         1
#تعریف_کړه COLTYPE_BLOB        2
#تعریف_کړه COLTYPE_INT         3
#تعریف_کړه COLTYPE_INTEGER     4
#تعریف_کړه COLTYPE_REAL        5
#تعریف_کړه COLTYPE_متن        6
#تعریف_کړه SQLITE_N_STDTYPE    6  /* Number of standard types */

/* Allowed values for Column.colFlags.
**
** Constraints:
**         TF_HasVirtual == COLFLAG_VIRTUAL
**         TF_HasStored  == COLFLAG_STORED
**         TF_HasHidden  == COLFLAG_HIDDEN
*/
#تعریف_کړه COLFLAG_PRIMKEY   0x0001   /* Column is part of the primary key */
#تعریف_کړه COLFLAG_HIDDEN    0x0002   /* A hidden column in a virtual table */
#تعریف_کړه COLFLAG_HASTYPE   0x0004   /* Type name follows column name */
#تعریف_کړه COLFLAG_UNIQUE    0x0008   /* Column def contains "UNIQUE" or "PK" */
#تعریف_کړه COLFLAG_SORTERREF 0x0010   /* Use sorter-refs with this column */
#تعریف_کړه COLFLAG_VIRTUAL   0x0020   /* GENERATED ALWAYS AS ... VIRTUAL */
#تعریف_کړه COLFLAG_STORED    0x0040   /* GENERATED ALWAYS AS ... STORED */
#تعریف_کړه COLFLAG_NOTAVAIL  0x0080   /* STORED column not yet calculated */
#تعریف_کړه COLFLAG_BUSY      0x0100   /* Blocks recursion on GENERATED columns */
#تعریف_کړه COLFLAG_HASCOLL   0x0200   /* Has collating sequence name in zCnName */
#تعریف_کړه COLFLAG_NOEXPAND  0x0400   /* Omit this column when expanding "*" */
#تعریف_کړه COLFLAG_GENERATED 0x0060   /* Combo: _STORED, _VIRTUAL */
#تعریف_کړه COLFLAG_NOINSERT  0x0062   /* Combo: _HIDDEN, _STORED, _VIRTUAL */

/*
** A "Collating Sequence" is defined by an instance of the following
** structure. Conceptually, a collating sequence consists of a name and
** a comparison routine that defines the order of that sequence.
**
** If CollSeq.xCmp is NULL, it means that the
** collating sequence is undefined.  Indices built on an undefined
** collating sequence may not be read or written.
*/
بنية CollSeq {
  حرف *zName;          /* Name of the collating sequence, UTF-8 encoded */
  u8 enc;               /* Text encoding handled by xCmp() */
  فراغ *pUser;          /* First argument to xCmp() */
  صحيح (*xCmp)(فراغ*,صحيح, ثابت فراغ*, صحيح, ثابت فراغ*);
  فراغ (*xDel)(فراغ*);  /* Destructor for pUser */
};

/*
** A sort order can be either ASC or DESC.
*/
#تعریف_کړه SQLITE_SO_ASC       0  /* Sort in ascending order */
#تعریف_کړه SQLITE_SO_DESC      1  /* Sort in ascending order */
#تعریف_کړه SQLITE_SO_UNDEFINED -1 /* No sort order specified */

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
#تعریف_کړه SQLITE_AFF_NONE     0x40  /* '@' */
#تعریف_کړه SQLITE_AFF_BLOB     0x41  /* 'A' */
#تعریف_کړه SQLITE_AFF_متن     0x42  /* 'B' */
#تعریف_کړه SQLITE_AFF_NUMERIC  0x43  /* 'C' */
#تعریف_کړه SQLITE_AFF_INTEGER  0x44  /* 'D' */
#تعریف_کړه SQLITE_AFF_REAL     0x45  /* 'E' */
#تعریف_کړه SQLITE_AFF_FLEXNUM  0x46  /* 'F' */
#تعریف_کړه SQLITE_AFF_DEFER    0x58  /* 'X'  - defer computation until later */

#تعریف_کړه sqlite3IsNumericAffinity(X)  ((X)>=SQLITE_AFF_NUMERIC)

/*
** The SQLITE_AFF_MASK values masks off the significant bits of an
** affinity value.
*/
#تعریف_کړه SQLITE_AFF_ماسک     0x47

/*
** Additional bit values that can be ORed with an affinity without
** changing the affinity.
**
** The SQLITE_NOTNULL flag is a combination of NULLEQ and JUMPIFNULL.
** It causes an assert() to fire if either operand to a comparison
** operator is NULL.  It is added to certain comparison operators to
** prove that the operands are always NOT NULL.
*/
#تعریف_کړه SQLITE_JUMPIFNULL   0x10  /* jumps if either operand is NULL */
#تعریف_کړه SQLITE_NULLEQ       0x80  /* NULL=NULL */
#تعریف_کړه SQLITE_NOTNULL      0x90  /* Assert that operands are never NULL */

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
بنية VTable {
  sqlite3 *db;              /* Database connection associated with this table */
  Module *pMod;             /* Pointer to module implementation */
  sqlite3_vtab *pVtab;      /* Pointer to vtab instance */
  صحيح nRef;                 /* Number of pointers to this structure */
  u8 bConstraint;           /* True if constraints are supported */
  u8 bAllSchemas;           /* True if might use any attached schema */
  u8 eVtabRisk;             /* Riskiness of allowing hacker access */
  صحيح iSavepoint;           /* Depth of the SAVEPOINT stack */
  VTable *p_بل;            /* Next in linked list (see above) */
};

/* Allowed values for VTable.eVtabRisk
*/
#تعریف_کړه SQLITE_VTABRISK_ټیټ          0
#تعریف_کړه SQLITE_VTABRISK_نورمال       1
#تعریف_کړه SQLITE_VTABRISK_لوړ         2

/*
** The schema for each SQL table, virtual table, and view is represented
** in memory by an instance of the following structure.
*/
بنية جدول {
  حرف *zName;         /* Name of the table or view */
  ستنه *a_ستون;        /* Information about each column */
  شاخص *p_شاخص;       /* List of SQL indexes on this table. */
  حرف *z_ستون_Aff;       /* String defining the affinity of each column */
  بیان_List *pCheck;    /* All CHECK constraints */
                       /*   ... also used as column name list in a VIEW */
  Pgno tnum;           /* Root BTree page for this table */
  u32 nTabRef;         /* Number of pointers to this Table */
  u32 tabFlags;        /* Mask of TF_* values */
  i16 iPKey;           /* If not negative, use aCol[iPKey] as the rowid */
  i16 n_ستون;            /* Number of columns in this table */
  i16 nNVCol;          /* Number of columns that are not VIRTUAL */
  LogEst n_لیکه_Log_Est;   /* Estimated rows in table - from sqlite_stat1 table */
  LogEst sz_Tab_لیکه;     /* Estimated size of each table row in bytes */
#که_تعریف_شوی SQLITE_ENABLE_COSTMULT
  LogEst لګښت_Mult;     /* Cost multiplier for using this table */
#د_که_پای
  u8 کلی_Conf;          /* What to do in case of uniqueness conflict on iPKey */
  u8 eTabType;         /* 0: normal, 1: virtual, 2: view */
  اتحاد {
    بنية {             /* Used by ordinary tables: */
      صحيح جمع_کړه_ستون_دننه_واټن;    /* Offset in CREATE TABLE stmt to add a new column */
      FKey *pFKey;         /* Linked list of all foreign keys in this table */
      بیان_List *pDfltList; /* DEFAULT clauses on various columns.
                           ** Or the AS clause for generated columns. */
    } tab;
    بنية {             /* Used by views: */
      Select *pSelect;     /* View definition */
    } view;
    بنية {             /* Used by virtual tables only: */
      صحيح nArg;            /* Number of arguments to the module */
      حرف **azArg;        /* 0: module 1: schema 2: vtab name 3...: args */
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
#تعریف_کړه TF_Readonly       0x00000001 /* Read-only system table */
#تعریف_کړه TF_HasHidden      0x00000002 /* Has one or more hidden columns */
#تعریف_کړه TF_Has_Primary_کلی  0x00000004 /* Table has a primary key */
#تعریف_کړه TF_Autoincrement  0x00000008 /* Integer primary key is autoincrement */
#تعریف_کړه TF_HasStat1       0x00000010 /* nRowLogEst set from sqlite_stat1 */
#تعریف_کړه TF_HasVirtual     0x00000020 /* Has one or more VIRTUAL columns */
#تعریف_کړه TF_HasStored      0x00000040 /* Has one or more STORED columns */
#تعریف_کړه TF_HasGenerated   0x00000060 /* Combo: HasVirtual + HasStored */
#تعریف_کړه TF_WithoutRowid   0x00000080 /* No rowid.  PRIMARY KEY is the key */
#تعریف_کړه TF_MaybeReanalyze 0x00000100 /* Maybe run ANALYZE on this table */
#تعریف_کړه TF_NoVisibleRowid 0x00000200 /* No user-visible "rowid" column */
#تعریف_کړه TF_OOOHidden      0x00000400 /* Out-of-Order hidden columns */
#تعریف_کړه TF_HasNotNull     0x00000800 /* Contains NOT NULL constraints */
#تعریف_کړه TF_Shadow         0x00001000 /* True for a shadow table */
#تعریف_کړه TF_HasStat4       0x00002000 /* STAT4 info available for this table */
#تعریف_کړه TF_Ephemeral      0x00004000 /* An ephemeral table */
#تعریف_کړه TF_Eponymous      0x00008000 /* An eponymous virtual table */
#تعریف_کړه TF_Strict         0x00010000 /* STRICT mode */
#تعریف_کړه TF_Imposter       0x00020000 /* An imposter table */

/*
** Allowed values for Table.eTabType
*/
#تعریف_کړه TABTYP_NORM      0     /* Ordinary table */
#تعریف_کړه TABTYP_VTAB      1     /* Virtual table */
#تعریف_کړه TABTYP_VIEW      2     /* A view */

#تعریف_کړه IsView(X)           ((X)->eTabType==TABTYP_VIEW)
#تعریف_کړه Is_Ordinary_جدول(X)  ((X)->eTabType==TABTYP_NORM)

/*
** Test to see whether or not a table is a virtual table.  This is
** done as a macro so that it will be optimized out when virtual
** table support is omitted from the build.
*/
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
#  define IsVirtual(X)      ((X)->eTabType==TABTYP_VTAB)
#  define بیان_Is_Vtab(X)  \
   ((X)->op==TK_ستنه && (X)->y.pTab->eTabType==TABTYP_VTAB)
#وإلا
#  define IsVirtual(X)      0
#  define بیان_Is_Vtab(X)     0
#د_که_پای

/*
** Macros to determine if a column is hidden.  IsOrdinaryHiddenColumn()
** only works for non-virtual tables (ordinary tables and views) and is
** always false unless SQLITE_ENABLE_HIDDEN_COLUMNS is defined.  The
** IsHiddenColumn() macro is general purpose.
*/
#إذا defined(SQLITE_ENABLE_HIDDEN_COLUMNS)
#  define Is_Hidden_ستنه(X)         (((X)->ستون_Flags & COLFLAG_HIDDEN)!=0)
#  define Is_Ordinary_Hidden_ستنه(X) (((X)->ستون_Flags & COLFLAG_HIDDEN)!=0)
#وإلاإذا !defined(SQLITE_OMIT_VIRTUALTABLE)
#  define Is_Hidden_ستنه(X)         (((X)->ستون_Flags & COLFLAG_HIDDEN)!=0)
#  define Is_Ordinary_Hidden_ستنه(X) 0
#وإلا
#  define Is_Hidden_ستنه(X)         0
#  define Is_Ordinary_Hidden_ستنه(X) 0
#د_که_پای


/* Does the table have a rowid */
#تعریف_کړه HasRowid(X)     (((X)->tabFlags & TF_WithoutRowid)==0)
#تعریف_کړه VisibleRowid(X) (((X)->tabFlags & TF_NoVisibleRowid)==0)

/* Macro is true if the SQLITE_ALLOW_ROWID_IN_VIEW (mis-)feature is
** available.  By default, this macro is false
*/
#که_نه_تعریف_شوی SQLITE_ALLOW_ROWID_دننه_VIEW
# define ViewCanHaveRowid     0
#وإلا
# define ViewCanHaveRowid     (sqlite3Config.mNoVisibleRowid==0)
#د_که_پای

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
بنية FKey {
  جدول *pFrom;     /* Table containing the REFERENCES clause (aka: Child) */
  FKey *p_بل_From;  /* Next FKey with the same in pFrom. Next parent of pFrom */
  حرف *z_منزل;        /* Name of table that the key points to (aka: Parent) */
  FKey *p_بل_منزل;    /* Next with the same zTo. Next child of zTo. */
  FKey *p_Prev_منزل;    /* Previous with the same zTo */
  صحيح n_ستون;         /* Number of columns in this key */
  /* EV: R-30323-21917 */
  u8 isDeferred;       /* True if constraint checking is deferred till COMMIT */
  u8 aAction[2];        /* ON DELETE and ON UPDATE actions, respectively */
  Trigger *apTrigger[2];/* Triggers for aAction[] actions */
  بنية s_ستون_Map {      /* Mapping of columns in pFrom to columns in zTo */
    صحيح iFrom;            /* Index of column in pFrom */
    حرف *z_ستون;           /* Name of column in zTo.  If NULL use PRIMARY KEY */
  } a_ستون[FLEXARRAY];      /* One entry for each of nCol columns */
};

/* The size (in bytes) of an FKey object holding N columns.  The answer
** does NOT include space to hold the zTo name. */
#تعریف_کړه SZ_FKEY(N)  (offsetof(FKey,a_ستون)+(N)*حجم(بنية s_ستون_Map))

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
#تعریف_کړه OE_None     0   /* There is no constraint to check */
#تعریف_کړه OE_Rollback 1   /* Fail the operation and rollback the transaction */
#تعریف_کړه OE_Abort    2   /* Back out changes but do no rollback transaction */
#تعریف_کړه OE_Fail     3   /* Stop the operation but leave all prior changes */
#تعریف_کړه OE_Ignore   4   /* Ignore the error. Do not do the INSERT or UPDATE */
#تعریف_کړه OE_Replace  5   /* Delete existing record, then do INSERT or UPDATE */
#تعریف_کړه OE_Update   6   /* Process as a DO UPDATE in an upsert */
#تعریف_کړه OE_Restrict 7   /* OE_Abort for IMMEDIATE, OE_Rollback for DEFERRED */
#تعریف_کړه OE_سېټ_Null  8   /* Set the foreign key value to NULL */
#تعریف_کړه OE_سېټ_Dflt  9   /* Set the foreign key value to its default */
#تعریف_کړه OE_Cascade  10  /* Cascade the changes */
#تعریف_کړه OE_Default  11  /* Do whatever the default action is */


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
بنية کلی_Info {
  u32 nRef;           /* Number of references to this KeyInfo object */
  u8 enc;             /* Text encoding - one of the SQLITE_UTF* values */
  u16 n_کلی_ډګر;      /* Number of key columns in the index */
  u16 n_All_ډګر;      /* Total columns, including key plus others */
  sqlite3 *db;        /* The database connection */
  u8 *a_ترتیبول_Flags;     /* Sort order for each column. */
  CollSeq *aColl[FLEXARRAY]; /* Collating sequence for each term of the key */
};

/* The size (in bytes) of a KeyInfo object with up to N fields.  This includes
** the main body of the KeyInfo object and the aColl[] array of N elements,
** but does not count the memory used to hold aSortFlags[]. */
#تعریف_کړه SZ_KEYINFO(N)  (offsetof(کلی_Info,aColl) + (N)*حجم(CollSeq*))

/* The size of a bare KeyInfo with no aColl[] entries */
#إذا FLEXARRAY+1 > 1
# define SZ_KEYINFO_0   offsetof(کلی_Info,aColl)
#وإلا
# define SZ_KEYINFO_0   حجم(کلی_Info)
#د_که_پای

/*
** Allowed bit values for entries in the KeyInfo.aSortFlags[] array.
*/
#تعریف_کړه KEYINFO_ترتیب_DESC    0x01    /* DESC sort order */
#تعریف_کړه KEYINFO_ترتیب_BIGNULL 0x02    /* NULL is larger than any other value */

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
بنية Unpacked_ریکارډ {
  کلی_Info *p_کلی_Info;  /* Comparison info for the index that is unpacked */
  Mem *aMem;          /* Values for columns of the index */
  اتحاد {
    حرف *z;            /* Cache of aMem[0].z for vdbeRecordCompareString() */
    i64 i;              /* Cache of aMem[0].u.i for vdbeRecordCompareInt() */
  } u;
  صحيح n;              /* Cache of aMem[0].n used by vdbeRecordCompareString() */
  u16 n_ډګر;         /* Number of entries in apMem[] */
  i8 default_rc;      /* Comparison result if keys are equal */
  u8 err_کوډ;         /* Error detected by xRecordCompare (CORRUPT or NOMEM) */
  i8 r1;              /* Value to return if (lhs < rhs) */
  i8 r2;              /* Value to return if (lhs > rhs) */
  u8 eq_کتل_شوی;          /* True if an equality comparison has been seen */
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
بنية شاخص {
  حرف *zName;             /* Name of this index */
  i16 *ai_ستنه;           /* Which columns are used by this index.  1st is 0 */
  LogEst *ai_لیکه_Log_Est;     /* From ANALYZE: Est. rows selected by each column */
  جدول *p_جدول;           /* The SQL table being indexed */
  حرف *z_ستون_Aff;           /* String defining the affinity of each column */
  شاخص *p_بل;            /* The next index associated with the same table */
  Schema *pSchema;         /* Schema containing this index */
  u8 *a_ترتیبول_ترتیب;          /* for each column: True==DESC, False==ASC */
  ثابت حرف **azColl;     /* Array of collation sequence names for index */
  بیان *pPartIdxWhere;     /* WHERE clause for partial indices */
  بیان_List *a_ستون_بیان;      /* Column expressions */
  Pgno tnum;               /* DB Page containing root of this index */
  LogEst sz_Idx_لیکه;         /* Estimated average row size in bytes */
  u16 n_کلی_ستون;             /* Number of columns forming the key */
  u16 n_ستنه;             /* Nr columns in btree. Can be 2*Table.nCol */
  u8 onError;              /* OE_Abort, OE_Ignore, OE_Replace, or OE_None */
  بینښه idxType:2;      /* 0:Normal 1:UNIQUE, 2:PRIMARY KEY, 3:IPK */
  بینښه bUnordered:1;   /* Use this index for == or IN queries only */
  بینښه uniqNotNull:1;  /* True if UNIQUE and NOT NULL for all columns */
  بینښه isResized:1;    /* True if resizeIndexObject() has been called */
  بینښه isCovering:1;   /* True if this is a covering index */
  بینښه no_Skip_لټون:1;   /* Do not try to use skip-scan if true */
  بینښه hasStat1:1;     /* aiRowLogEst values come from sqlite_stat1 */
  بینښه bNoQuery:1;     /* Do not use this index to optimize queries */
  بینښه b_Asc_کلی_Bug:1;   /* True if the bba7b69f9849b5bf bug applies */
  بینښه bHasVCol:1;     /* Index references one or more VIRTUAL columns */
  بینښه b_Has_بیان:1;     /* Index contains an expression, either a literal
                           ** expression, or a reference to a VIRTUAL column */
#که_تعریف_شوی SQLITE_ENABLE_STAT4
  صحيح nSample;             /* Number of elements in aSample[] */
  صحيح mxSample;            /* Number of slots allocated to aSample[] */
  صحيح n_Sample_ستون;          /* Size of IndexSample.anEq[] and so on */
  tRowcnt *aAvgEq;         /* Average nEq values for keys not in aSample */
  شاخص_Sample *aSample;    /* Samples of the left-most key */
  tRowcnt *ai_لیکه_Est;       /* Non-logarithmic stat1 data for this index */
  tRowcnt n_لیکه_Est0;        /* Non-logarithmic number of rows in the index */
#د_که_پای
  Bitmask ستون_Not_Idxed;     /* Unindexed columns in pTab */
};

/*
** Allowed values for Index.idxType
*/
#تعریف_کړه SQLITE_IDXTYPE_APPDEF      0   /* Created using CREATE INDEX */
#تعریف_کړه SQLITE_IDXTYPE_UNIQUE      1   /* Implements a UNIQUE constraint */
#تعریف_کړه SQLITE_IDXTYPE_PRIMARYKEY  2   /* Is the PRIMARY KEY for the table */
#تعریف_کړه SQLITE_IDXTYPE_IPK         3   /* INTEGER PRIMARY KEY index */

/* Return true if index X is a PRIMARY KEY index */
#تعریف_کړه Is_Primary_کلی_شاخص(X)  ((X)->idxType==SQLITE_IDXTYPE_PRIMARYKEY)

/* Return true if index X is a UNIQUE index */
#تعریف_کړه Is_Unique_شاخص(X)      ((X)->onError!=OE_None)

/* The Index.aiColumn[] values are normally positive integer.  But
** there are some negative values that have special meaning:
*/
#تعریف_کړه XN_ROWID     (-1)     /* Indexed column is the rowid */
#تعریف_کړه XN_بیان      (-2)     /* Indexed column is an expression */

/*
** Each sample stored in the sqlite_stat4 table is represented in memory
** using a structure of this type.  See documentation at the top of the
** analyze.c source file for additional information.
*/
بنية شاخص_Sample {
  فراغ *p;          /* Pointer to sampled record */
  صحيح n;            /* Size of record in bytes */
  tRowcnt *anEq;    /* Est. number of rows where the key equals this sample */
  tRowcnt *anLt;    /* Est. number of rows where key is less than this sample */
  tRowcnt *anDLt;   /* Est. number of distinct keys less than this sample */
};

/*
** Possible values to use within the flags argument to sqlite3GetToken().
*/
#تعریف_کړه SQLITE_TOKEN_QUOTED    0x1 /* Token is a quoted identifier. */
#تعریف_کړه SQLITE_TOKEN_KEYWORD   0x2 /* Token is a keyword. */

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
بنية Token {
  ثابت حرف *z;     /* Text of the token.  Not NULL-terminated! */
  بینښه صحيح n;    /* Number of characters in this token */
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
بنية AggInfo {
  u8 مستقیم_ډېرپېښ;          /* Direct rendering mode means take data directly
                          ** from source tables rather than from accumulators */
  u8 use_ترتیبونه_Idx;       /* In direct mode, reference the sorting index rather
                          ** than the source table */
  u32 n_ترتیبونه_ستنه;     /* Number of columns in the sorting index */
  صحيح ترتیبونه_Idx;         /* Cursor number of the sorting index */
  صحيح ترتیبونه_Idx_PTab;     /* Cursor number of pseudo-table */
  صحيح i_لومړی_Reg;          /* First register in range for aCol[] and aFunc[] */
  بیان_List *p_ډله_By;     /* The group by clause */
  بنية Agg_Info_ستون {    /* For each column used in source tables */
    جدول *pTab;             /* Source table */
    بیان *pCExpr;            /* The original expression */
    صحيح i_جدول;              /* Cursor number of the source table */
    صحيح i_ستنه;             /* Column number within the source table */
    صحيح i_Sorter_ستنه;       /* Column number in the sorting index */
  } *a_ستون;
  صحيح n_ستنه;            /* Number of used entries in aCol[] */
  صحيح n_راټولونکی;       /* Number of columns that show through to the output.
                          ** Additional columns are used only as parameters to
                          ** aggregate functions */
  بنية AggInfo_func {   /* For each aggregate function */
    بیان *pFExpr;            /* Expression encoding the function */
    FuncDef *pFunc;          /* The aggregate function implementation */
    صحيح iDistinct;           /* Ephemeral table used to enforce DISTINCT */
    صحيح iDistAddr;           /* Address of OP_OpenEphemeral */
    صحيح iOBTab;              /* Ephemeral table to implement ORDER BY */
    u8 bOBPayload;           /* iOBTab has payload columns separate from key */
    u8 bOBUnique;            /* Enforce uniqueness on iOBTab keys */
    u8 bUseSubtype;          /* Transfer subtype info through sorter */
  } *aFunc;
  صحيح nFunc;              /* Number of entries in aFunc[] */
  u32 selId;              /* Select to which this AggInfo belongs */
#که_تعریف_شوی SQLITE_DEBUG
  Select *pSelect;        /* SELECT statement that this AggInfo supports */
#د_که_پای
};

/*
** Macros to compute aCol[] and aFunc[] register numbers.
**
** These macros should not be used prior to the call to
** assignAggregateRegisters() that computes the value of pAggInfo->iFirstReg.
** The assert()s that are part of this macro verify that constraint.
*/
#که_نه_تعریف_شوی NDEBUG
#تعریف_کړه Agg_Info_ستنه_Reg(A,I)  (أكد((A)->i_لومړی_Reg),(A)->i_لومړی_Reg+(I))
#تعریف_کړه AggInfoFuncReg(A,I)    \
                      (أكد((A)->i_لومړی_Reg),(A)->i_لومړی_Reg+(A)->n_ستنه+(I))
#وإلا
#تعریف_کړه Agg_Info_ستنه_Reg(A,I)  ((A)->i_لومړی_Reg+(I))
#تعریف_کړه AggInfoFuncReg(A,I)    \
                      ((A)->i_لومړی_Reg+(A)->n_ستنه+(I))
#د_که_پای

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
#إذا SQLITE_MAX_VARIABLE_شمېره<32767
تعريفنوع i16 ynVar;
#وإلا
تعريفنوع صحيح ynVar;
#د_که_پای

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
بنية بیان {
  u8 op;                 /* Operation performed by this node */
  حرف aff_بیان;          /* affinity, or RAISE type */
  u8 op2;                /* TK_REGISTER/TK_TRUTH: original value of Expr.op
                         ** TK_COLUMN: the value of p5 for OP_Column
                         ** TK_AGG_FUNCTION: nesting depth
                         ** TK_FUNCTION: NC_SelfRef flag if needs OP_PureFunc */
#که_تعریف_شوی SQLITE_DEBUG
  u8 vvaFlags;           /* Verification flags. */
#د_که_پای
  u32 flags;             /* Various flags.  EP_* See below */
  اتحاد {
    حرف *zToken;          /* Token value. Zero terminated and dequoted */
    صحيح i_ارزښت;            /* Non-negative integer value if EP_IntValue */
  } u;

  /* If the EP_TokenOnly flag is set in the Expr.flags mask, then no
  ** space is allocated for the fields below this point. An attempt to
  ** access them will result in a segfault or malfunction.
  *********************************************************************/

  بیان *p_کیڼ;           /* Left subnode */
  بیان *p_ښی;          /* Right subnode */
  اتحاد {
    بیان_List *pList;     /* op = IN, EXISTS, SELECT, CASE, FUNCTION, BETWEEN */
    Select *pSelect;     /* EP_xIsSelect and op = IN, EXISTS, SELECT */
  } x;

  /* If the EP_Reduced flag is set in the Expr.flags mask, then no
  ** space is allocated for the fields below this point. An attempt to
  ** access them will result in a segfault or malfunction.
  *********************************************************************/

#إذا SQLITE_MAX_بیان_ژوروالی>0
  صحيح n_لوړوالی;           /* Height of the tree headed by this node */
#د_که_پای
  صحيح i_جدول;            /* TK_COLUMN: cursor number of table holding column
                         ** TK_REGISTER: register number
                         ** TK_TRIGGER: 1 -> new, 0 -> old
                         ** EP_Unlikely:  134217728 times likelihood
                         ** TK_IN: ephemeral table holding RHS
                         ** TK_SELECT_COLUMN: Number of columns on the LHS
                         ** TK_SELECT: 1st register of result vector */
  ynVar i_ستنه;         /* TK_COLUMN: column index.  -1 for rowid.
                         ** TK_VARIABLE: variable number (always >= 1).
                         ** TK_SELECT_COLUMN: column of the result vector */
  i16 iAgg;              /* Which entry in pAggInfo->aCol[] or ->aFunc[] */
  اتحاد {
    صحيح iJoin;             /* If EP_OuterON or EP_InnerON, the right table */
    صحيح iOfst;             /* else: start of token from start of statement */
  } w;
  AggInfo *pAggInfo;     /* Used by TK_AGG_COLUMN and TK_AGG_FUNCTION */
  اتحاد {
    جدول *pTab;           /* TK_COLUMN: Table containing column. Can be NULL
                           ** for a column of an index on an expression */
    کړکۍ *pWin;          /* EP_WinFunc: Window/Filter defn for a function */
    صحيح nReg;              /* TK_NULLS: Number of registers to NULL out */
    بنية {               /* TK_IN, TK_SELECT, and TK_EXISTS */
      صحيح iAddr;             /* Subroutine entry address */
      صحيح regReturn;         /* Register used to hold return address */
    } sub;
  } y;
};

/* The following are the meanings of bits in the Expr.flags field.
** Value restrictions:
**
**          EP_Agg == NC_HasAgg == SF_HasAgg
**          EP_Win == NC_HasWin
*/
#تعریف_کړه EP_OuterON    0x000001 /* Originates in ON/USING clause of outer join */
#تعریف_کړه EP_InnerON    0x000002 /* Originates in ON/USING of an inner join */
#تعریف_کړه EP_Distinct   0x000004 /* Aggregate function with DISTINCT keyword */
#تعریف_کړه EP_HasFunc    0x000008 /* Contains one or more functions of any kind */
#تعریف_کړه EP_Agg        0x000010 /* Contains one or more aggregate functions */
#تعریف_کړه EP_Fixed_ستون   0x000020 /* TK_Column with a known fixed value */
#تعریف_کړه EP_VarSelect  0x000040 /* pSelect is correlated, not constant */
#تعریف_کړه EP_DblQuoted  0x000080 /* token.z was originally in "..." */
#تعریف_کړه EP_InfixFunc  0x000100 /* True for an infix function: LIKE, GLOB, etc */
#تعریف_کړه EP_Collate    0x000200 /* Tree contains a TK_COLLATE operator */
#تعریف_کړه EP_Commuted   0x000400 /* Comparison operator has been commuted */
#تعریف_کړه EP_Int_ارزښت   0x000800 /* Integer value contained in u.iValue */
#تعریف_کړه EP_xIsSelect  0x001000 /* x.pSelect is valid (otherwise x.pList is) */
#تعریف_کړه EP_Skip       0x002000 /* Operator does not contribute to affinity */
#تعریف_کړه EP_Reduced    0x004000 /* Expr struct EXPR_REDUCEDSIZE bytes only */
#تعریف_کړه EP_Win        0x008000 /* Contains window functions */
#تعریف_کړه EP_TokenOnly  0x010000 /* Expr struct EXPR_TOKENONLYSIZE bytes only */
#تعریف_کړه EP_ډک_اندازه   0x020000 /* Expr structure must remain full sized */
#تعریف_کړه EP_If_Null_لیکه  0x040000 /* The TK_IF_NULL_ROW opcode */
#تعریف_کړه EP_Unlikely   0x080000 /* unlikely() or likelihood() function */
#تعریف_کړه EP_ConstFunc  0x100000 /* A SQLITE_FUNC_CONSTANT or _SLOCHNG function */
#تعریف_کړه EP_CanBeNull  0x200000 /* Can be null despite NOT NULL constraint */
#تعریف_کړه EP_Subquery   0x400000 /* Tree contains a TK_SELECT operator */
#تعریف_کړه EP_ونې_پاڼه       0x800000 /* Expr.pLeft, .pRight, .u.pSelect all NULL */
#تعریف_کړه EP_WinFunc   0x1000000 /* TK_FUNCTION with Expr.y.pWin set */
#تعریف_کړه EP_Subrtn    0x2000000 /* Uses Expr.y.sub. TK_IN, _SELECT, or _EXISTS */
#تعریف_کړه EP_Quoted    0x4000000 /* TK_ID was originally quoted */
#تعریف_کړه EP_Static    0x8000000 /* Held in memory not obtained from malloc() */
#تعریف_کړه EP_IsTrue   0x10000000 /* Always has boolean value of TRUE */
#تعریف_کړه EP_IsFalse  0x20000000 /* Always has boolean value of FALSE */
#تعریف_کړه EP_FromDDL  0x40000000 /* Originates from sqlite_schema */
#تعریف_کړه EP_SubtArg  0x80000000 /* Is argument to SQLITE_SUBTYPE function */

/* The EP_Propagate mask is a set of properties that automatically propagate
** upwards into parent nodes.
*/
#تعریف_کړه EP_Propagate (EP_Collate|EP_Subquery|EP_HasFunc)

/* Macros can be used to test, set, or clear bits in the
** Expr.flags field.
*/
#تعریف_کړه بیان_Has_Property(E,P)     (((E)->flags&(u32)(P))!=0)
#تعریف_کړه بیان_Has_All_Property(E,P)  (((E)->flags&(u32)(P))==(u32)(P))
#تعریف_کړه بیان_سېټ_Property(E,P)     (E)->flags|=(u32)(P)
#تعریف_کړه بیان_Clear_Property(E,P)   (E)->flags&=~(u32)(P)
#تعریف_کړه بیان_Always_True(E)   (((E)->flags&(EP_OuterON|EP_IsTrue))==EP_IsTrue)
#تعریف_کړه بیان_Always_False(E)  (((E)->flags&(EP_OuterON|EP_IsFalse))==EP_IsFalse)
#تعریف_کړه بیان_Is_ډک_اندازه(E)   (((E)->flags&(EP_Reduced|EP_TokenOnly))==0)

/* Macros used to ensure that the correct members of unions are accessed
** in Expr.
*/
#تعریف_کړه بیان_Use_UToken(E)    (((E)->flags&EP_Int_ارزښت)==0)
#تعریف_کړه بیان_Use_UValue(E)    (((E)->flags&EP_Int_ارزښت)!=0)
#تعریف_کړه بیان_Use_WOfst(E)     (((E)->flags&(EP_InnerON|EP_OuterON))==0)
#تعریف_کړه بیان_Use_WJoin(E)     (((E)->flags&(EP_InnerON|EP_OuterON))!=0)
#تعریف_کړه بیان_Use_XList(E)     (((E)->flags&EP_xIsSelect)==0)
#تعریف_کړه بیان_Use_XSelect(E)   (((E)->flags&EP_xIsSelect)!=0)
#تعریف_کړه بیان_Use_YTab(E)      (((E)->flags&(EP_WinFunc|EP_Subrtn))==0)
#تعریف_کړه بیان_Use_YWin(E)      (((E)->flags&EP_WinFunc)!=0)
#تعریف_کړه بیان_Use_YSub(E)      (((E)->flags&EP_Subrtn)!=0)

/* Flags for use with Expr.vvaFlags
*/
#تعریف_کړه EP_NoReduce   0x01  /* Cannot EXPRDUP_REDUCE this Expr */
#تعریف_کړه EP_Immutable  0x02  /* Do not change this Expr node */

/* The ExprSetVVAProperty() macro is used for Verification, Validation,
** and Accreditation only.  It works like ExprSetProperty() during VVA
** processes but is a no-op for delivery.
*/
#که_تعریف_شوی SQLITE_DEBUG
# define بیان_سېټ_VVAProperty(E,P)   (E)->vvaFlags|=(P)
# define بیان_Has_VVAProperty(E,P)   (((E)->vvaFlags&(P))!=0)
# define بیان_Clear_VVAProperties(E) (E)->vvaFlags = 0
#وإلا
# define بیان_سېټ_VVAProperty(E,P)
# define بیان_Has_VVAProperty(E,P)   0
# define بیان_Clear_VVAProperties(E)
#د_که_پای

/*
** Macros to determine the number of bytes required by a normal Expr
** struct, an Expr struct with the EP_Reduced flag set in Expr.flags
** and an Expr struct with the EP_TokenOnly flag set.
*/
#تعریف_کړه بیان_FULLSIZE           حجم(بیان)           /* Full size */
#تعریف_کړه بیان_REDUCEDSIZE        offsetof(بیان,i_جدول)  /* Common features */
#تعریف_کړه بیان_TOKENONLYSIZE      offsetof(بیان,p_کیڼ)   /* Fewer features */

/*
** Flags passed to the sqlite3ExprDup() function. See the header comment
** above sqlite3ExprDup() for details.
*/
#تعریف_کړه EXPRDUP_REDUCE         0x0001  /* Used reduced-size Expr nodes */

/*
** True if the expression passed as an argument was a function with
** an OVER() clause (a window function).
*/
#که_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
# define Is_کړکۍ_Func(p) 0
#وإلا
# define Is_کړکۍ_Func(p) ( \
    بیان_Has_Property((p), EP_WinFunc) && p->y.pWin->eFrmType!=TK_FILTER \
 )
#د_که_پای

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
بنية بیان_List {
  صحيح n_بیان;             /* Number of expressions on the list */
  صحيح nAlloc;            /* Number of a[] slots allocated */
  بنية بیان_List_توکی { /* For each expression in the list */
    بیان *p_بیان;            /* The parse tree for this expression */
    حرف *zEName;           /* Token associated with this expression */
    بنية {
      u8 ترتیبول_Flags;           /* Mask of KEYINFO_ORDER_* flags */
      بینښه eEName :2;     /* Meaning of zEName */
      بینښه بشپړ :1;       /* Indicates when processing is finished */
      بینښه reusable :1;   /* Constant expression is reusable */
      بینښه bSorterRef :1; /* Defer evaluation until after sorting */
      بینښه bNulls :1;     /* True if explicit "NULLS FIRST/LAST" */
      بینښه b_کارول_شوی :1;      /* This column used in a SF_NestedFrom subquery */
      بینښه b_Using_غړی:1;  /* Term from the USING clause of a NestedFrom */
      بینښه bNoExpand: 1;  /* Term is an auxiliary in NestedFrom and should
                              ** not be expanded by "*" in parent queries */
    } fg;
    اتحاد {
      بنية {             /* Used by any ExprList other than Parse.pConsExpr */
        u16 i_ترتیب_By_ستون;      /* For ORDER BY, column number in result set */
        u16 iAlias;           /* Index into Parse.aAlias[] for zName */
      } x;
      صحيح i_Const_بیان_Reg;   /* Register in which Expr value is cached. Used only
                           ** by Parse.pConstExpr */
    } u;
  } a[FLEXARRAY];          /* One slot for each expression in the list */
};

/* The size (in bytes) of an ExprList object that is big enough to hold
** as many as N expressions. */
#تعریف_کړه SZ_EXPRLIST(N)  \
             (offsetof(بیان_List,a) + (N)*حجم(بنية بیان_List_توکی))

/*
** Allowed values for Expr.a.eEName
*/
#تعریف_کړه ENAME_NAME  0       /* The AS clause of a result set */
#تعریف_کړه ENAME_پراختیا  1       /* Complete text of the result set expression */
#تعریف_کړه ENAME_TAB   2       /* "DB.TABLE.NAME" for the result set */
#تعریف_کړه ENAME_ROWID 3       /* "DB.TABLE._rowid_" for * expansion of rowid */

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
بنية IdList {
  صحيح nId;         /* Number of identifiers on the list */
  بنية Id_List_توکی {
    حرف *zName;      /* Name of the identifier */
  } a[FLEXARRAY];
};

/* The size (in bytes) of an IdList object that can hold up to N IDs. */
#تعریف_کړه SZ_IDLIST(N)  (offsetof(IdList,a)+(N)*حجم(بنية Id_List_توکی))

/*
** Allowed values for IdList.eType, which determines which value of the a.u4
** is valid.
*/
#تعریف_کړه EU4_NONE   0   /* Does not use IdList.a.u4 */
#تعریف_کړه EU4_IDX    1   /* Uses IdList.a.u4.idx */
#تعریف_کړه EU4_بیان   2   /* Uses IdList.a.u4.pExpr -- NOT CURRENTLY USED */

/*
** Details of the implementation of a subquery.
*/
بنية Subquery {
  Select *pSelect;  /* A SELECT statement used in place of a table name */
  صحيح addrFillSub;  /* Address of subroutine to initialize a subquery */
  صحيح regReturn;    /* Register holding return address of addrFillSub */
  صحيح reg_پایله;    /* Registers holding results of a co-routine */
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
بنية Src_توکی {
  حرف *zName;      /* Name of the table */
  حرف *zAlias;     /* The "B" part of a "A AS B" phrase.  zName is the "A" */
  جدول *pSTab;     /* Table object for zName. Mnemonic: Srcitem-TABle */
  بنية {
    u8 jointype;      /* Type of join between this table and the previous */
    بینښه not_شاخصي :1;    /* True if there is a NOT INDEXED clause */
    بینښه is_شاخصي_By :1;   /* True if there is an INDEXED BY clause */
    بینښه isSubquery :1;    /* True if this term is a subquery */
    بینښه isTabFunc :1;     /* True if table-valued-function syntax */
    بینښه isCorrelated :1;  /* True if sub-query is correlated */
    بینښه isMaterialized:1; /* This is a materialized view */
    بینښه له_لارې_Coroutine :1;  /* Implemented as a co-routine */
    بینښه isRecursive :1;   /* True for recursive reference in WITH */
    بینښه fromDDL :1;       /* Comes from sqlite_schema */
    بینښه isCte :1;         /* This is a CTE */
    بینښه notCte :1;        /* This item may not match a CTE */
    بینښه isUsing :1;       /* u3.pUsing is valid */
    بینښه isOn :1;          /* u3.pOn was once valid and non-NULL */
    بینښه isSynthUsing :1;  /* u3.pUsing is synthesized from NATURAL */
    بینښه isNestedFrom :1;  /* pSelect is a SF_NestedFrom subquery */
    بینښه rowid_کارول_شوی :1;     /* The ROWID of this table is referenced */
    بینښه fixedSchema :1;   /* Uses u4.pSchema, not u4.zDatabase */
    بینښه hadSchema :1;     /* Had u4.zDatabase before u4.pSchema */
    بینښه fromExists :1;    /* Comes from WHERE EXISTS(...) */
  } fg;
  صحيح iCursor;      /* The VDBE cursor number used to access this table */
  Bitmask ستون_کارول_شوی;  /* Bit N set if column N used. Details above for N>62 */
  اتحاد {
    حرف *z_شاخصي_By;    /* Identifier from "INDEXED BY <zIndex>" clause */
    بیان_List *pFuncArg;  /* Arguments to table-valued-function */
    u32 n_لیکه;            /* Number of rows in a VALUES clause */
  } u1;
  اتحاد {
    شاخص *pIBIndex;  /* Index structure corresponding to u1.zIndexedBy */
    CteUse *pCteUse;  /* CTE Usage info when fg.isCte is true */
  } u2;
  اتحاد {
    بیان *pOn;        /* fg.isUsing==0 =>  The ON clause of a join */
    IdList *pUsing;   /* fg.isUsing==1 =>  The USING clause of a join */
  } u3;
  اتحاد {
    Schema *pSchema;  /* Schema to which this item is fixed */
    حرف *zDatabase;  /* Name of database holding this table */
    Subquery *pSubq;  /* Description of a subquery */
  } u4;
};

/*
** The OnOrUsing object represents either an ON clause or a USING clause.
** It can never be both at the same time, but it can be neither.
*/
بنية OnOrUsing {
  بیان *pOn;         /* The ON clause of a join */
  IdList *pUsing;    /* The USING clause of a join */
};

/*
** This object represents one or more tables that are the source of
** content for an SQL statement.  For example, a single SrcList object
** is used to hold the FROM clause of a SELECT statement.  SrcList also
** represents the target tables for DELETE, INSERT, and UPDATE statements.
**
*/
بنية SrcList {
  صحيح nSrc;             /* Number of tables or subqueries in the FROM clause */
  u32 nAlloc;           /* Number of entries allocated in a[] below */
  Src_توکی a[FLEXARRAY]; /* One entry for each identifier on the list */
};

/* Size (in bytes) of a SrcList object that can hold as many as N
** SrcItem objects. */
#تعریف_کړه SZ_SRCLIST(N) (offsetof(SrcList,a)+(N)*حجم(Src_توکی))

/* Size (in bytes( of a SrcList object that holds 1 SrcItem.  This is a
** special case of SZ_SRCITEM(1) that comes up often. */
#تعریف_کړه SZ_SRCLIST_1  (offsetof(SrcList,a)+حجم(Src_توکی))

/*
** Permitted values of the SrcList.a.jointype field
*/
#تعریف_کړه JT_INNER     0x01    /* Any kind of inner or cross join */
#تعریف_کړه JT_CROSS     0x02    /* Explicit use of the CROSS keyword */
#تعریف_کړه JT_NATURAL   0x04    /* True for a "natural" join */
#تعریف_کړه JT_کیڼ      0x08    /* Left outer join */
#تعریف_کړه JT_ښی     0x10    /* Right outer join */
#تعریف_کړه JT_OUTER     0x20    /* The "OUTER" keyword is present */
#تعریف_کړه JT_LTORJ     0x40    /* One of the LEFT operands of a RIGHT JOIN
                             ** Mnemonic: Left Table Of Right Join */
#تعریف_کړه JT_ERROR     0x80    /* unknown or unsupported join type */

/*
** Flags appropriate for the wctrlFlags parameter of sqlite3WhereBegin()
** and the WhereInfo.wctrlFlags member.
**
** Value constraints (enforced via assert()):
**     WHERE_USE_LIMIT  == SF_FixedLimit
*/
#تعریف_کړه WHERE_ORDERBY_نورمال   0x0000 /* No-op */
#تعریف_کړه WHERE_ORDERBY_MIN      0x0001 /* ORDER BY processing for min() func */
#تعریف_کړه WHERE_ORDERBY_MAX      0x0002 /* ORDER BY processing for max() func */
#تعریف_کړه WHERE_ONEPASS_DESIRED  0x0004 /* Want to do one-pass UPDATE/DELETE */
#تعریف_کړه WHERE_ONEPASS_MULTIROW 0x0008 /* ONEPASS is ok with multiple rows */
#تعریف_کړه WHERE_DUPLICATES_OK    0x0010 /* Ok to return a row more than once */
#تعریف_کړه WHERE_OR_SUBCLAUSE     0x0020 /* Processing a sub-WHERE as part of
                                      ** the OR optimization  */
#تعریف_کړه WHERE_GROUPBY          0x0040 /* pOrderBy is really a GROUP BY */
#تعریف_کړه WHERE_DISTINCTBY       0x0080 /* pOrderby is really a DISTINCT clause */
#تعریف_کړه WHERE_WANT_DISTINCT    0x0100 /* All output needs to be distinct */
#تعریف_کړه WHERE_SORTBYGROUP      0x0200 /* Support sqlite3WhereIsSorted() */
#تعریف_کړه WHERE_AGG_DISTINCT     0x0400 /* Query is "SELECT agg(DISTINCT ...)" */
#تعریف_کړه WHERE_ORDERBY_حد    0x0800 /* ORDERBY+LIMIT on the inner loop */
#تعریف_کړه WHERE_ښی_JOIN       0x1000 /* Processing a RIGHT JOIN */
#تعریف_کړه WHERE_KEEP_ALL_JOINS   0x2000 /* Do not do the omit-noop-join opt */
#تعریف_کړه WHERE_USE_حد        0x4000 /* Use the LIMIT in cost estimates */
                        /*     0x8000    not currently used */

/* Allowed return values from sqlite3WhereIsDistinct()
*/
#تعریف_کړه WHERE_DISTINCT_NOOP      0  /* DISTINCT keyword not used */
#تعریف_کړه WHERE_DISTINCT_UNIQUE    1  /* No duplicates */
#تعریف_کړه WHERE_DISTINCT_ORDERED   2  /* All duplicates are adjacent */
#تعریف_کړه WHERE_DISTINCT_UNORDERED 3  /* Duplicates are scattered */

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
بنية NameContext {
  Parse *pParse;       /* The parser */
  SrcList *pSrcList;   /* One or more tables used to resolve names */
  اتحاد {
    بیان_List *pEList;    /* Optional list of result-set columns */
    AggInfo *pAggInfo;   /* Information about aggregates at this level */
    Upsert *pUpsert;     /* ON CONFLICT clause information from an upsert */
    صحيح i_بنسټ_Reg;        /* For TK_REGISTER when parsing RETURNING */
  } uNC;
  NameContext *p_بل;  /* Next outer name context.  NULL for outermost */
  صحيح nRef;            /* Number of names resolved by this context */
  صحيح nNcErr;          /* Number of errors encountered while resolving names */
  صحيح ncFlags;         /* Zero or more NC_* flags defined below */
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
#تعریف_کړه NC_AllowAgg  0x000001 /* Aggregate functions are allowed here */
#تعریف_کړه NC_PartIdx   0x000002 /* True if resolving a partial index WHERE */
#تعریف_کړه NC_IsCheck   0x000004 /* True if resolving a CHECK constraint */
#تعریف_کړه NC_Gen_ستون    0x000008 /* True for a GENERATED ALWAYS AS clause */
#تعریف_کړه NC_HasAgg    0x000010 /* One or more aggregate functions seen */
#تعریف_کړه NC_Idx_بیان   0x000020 /* True if resolving columns of CREATE INDEX */
#تعریف_کړه NC_SelfRef   0x00002e /* Combo: PartIdx, isCheck, GenCol, and IdxExpr */
#تعریف_کړه NC_Subquery  0x000040 /* A subquery has been seen */
#تعریف_کړه NC_UEList    0x000080 /* True if uNC.pEList is used */
#تعریف_کړه NC_UAggInfo  0x000100 /* True if uNC.pAggInfo is used */
#تعریف_کړه NC_UUpsert   0x000200 /* True if uNC.pUpsert is used */
#تعریف_کړه NC_UBaseReg  0x000400 /* True if uNC.iBaseReg is used */
#تعریف_کړه NC_MinMaxAgg 0x001000 /* min/max aggregates seen.  See note above */
/*                   0x002000 // available for reuse */
#تعریف_کړه NC_AllowWin  0x004000 /* Window functions are allowed here */
#تعریف_کړه NC_HasWin    0x008000 /* One or more window functions seen */
#تعریف_کړه NC_IsDDL     0x010000 /* Resolving names in a CREATE statement */
#تعریف_کړه NC_دننه_Agg_Func 0x020000 /* True if analyzing arguments to an agg func */
#تعریف_کړه NC_FromDDL   0x040000 /* SQL text comes from sqlite_schema */
#تعریف_کړه NC_NoSelect  0x080000 /* Do not descend into sub-selects */
#تعریف_کړه NC_Where     0x100000 /* Processing WHERE clause of a SELECT */
#تعریف_کړه NC_ترتیب_Agg 0x8000000 /* Has an aggregate other than count/min/max */

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
بنية Upsert {
  بیان_List *p_Upsert_موخه;  /* Optional description of conflict target */
  بیان *p_Upsert_موخه_Where; /* WHERE clause for partial index targets */
  بیان_List *p_Upsert_سېټ;     /* The SET clause from an ON CONFLICT UPDATE */
  بیان *pUpsertWhere;       /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_بل_Upsert;      /* Next ON CONFLICT clause in the list */
  u8 isDoUpdate;            /* True for DO UPDATE.  False for DO NOTHING */
  u8 isDup;                 /* True if 2nd or later with same pUpsertIdx */
  /* Above this point is the parse tree for the ON CONFLICT clauses.
  ** The next group of fields stores intermediate data. */
  فراغ *p_منزل_Free;            /* Free memory when deleting the Upsert object */
  /* All fields above are owned by the Upsert object and must be freed
  ** when the Upsert is destroyed.  The fields below are used to transfer
  ** information from the INSERT processing down into the UPDATE processing
  ** while generating code.  The fields below are owned by the INSERT
  ** statement and will be freed by INSERT processing. */
  شاخص *pUpsertIdx;        /* UNIQUE constraint specified by pUpsertTarget */
  SrcList *pUpsertSrc;      /* Table to be updated */
  صحيح reg_معلومات;              /* First register holding array of VALUES */
  صحيح i_معلومات_Cur;             /* Index of the data cursor */
  صحيح iIdxCur;              /* Index of the first index cursor */
};

/*
** An instance of the following structure contains all information
** needed to generate code for a single SELECT statement.
*/
بنية Select {
  u8 op;                 /* One of: TK_UNION TK_ALL TK_INTERSECT TK_EXCEPT */
  LogEst n_Select_لیکه;     /* Estimated number of result rows */
  u32 selFlags;          /* Various SF_* values */
  صحيح i_حد, i_دننه_واټن;   /* Memory registers holding LIMIT & OFFSET counters */
  u32 selId;             /* Unique identifier number for this SELECT */
  بیان_List *pEList;      /* The fields of the result */
  SrcList *pSrc;         /* The FROM clause */
  بیان *pWhere;          /* The WHERE clause */
  بیان_List *p_ډله_By;    /* The GROUP BY clause */
  بیان *pHaving;         /* The HAVING clause */
  بیان_List *p_ترتیب_By;    /* The ORDER BY clause */
  Select *p_مخکینی;        /* Prior select in a compound select statement */
  Select *p_بل;         /* Next select to the left in a compound */
  بیان *p_حد;          /* LIMIT expression. NULL means not used. */
  With *pWith;           /* WITH clause attached to this select. Or NULL. */
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
  کړکۍ *pWin;          /* List of window functions */
  کړکۍ *pWinDefn;      /* List of named window definitions */
#د_که_پای
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
#تعریف_کړه SF_Distinct      0x0000001 /* Output should be DISTINCT */
#تعریف_کړه SF_All           0x0000002 /* Includes the ALL keyword */
#تعریف_کړه SF_Resolved      0x0000004 /* Identifiers have been resolved */
#تعریف_کړه SF_Aggregate     0x0000008 /* Contains agg functions or a GROUP BY */
#تعریف_کړه SF_HasAgg        0x0000010 /* Contains aggregate functions */
#تعریف_کړه SF_Cloned_Rhs_دننه   0x0000020 /* Cloned RHS of an IN operator */
#تعریف_کړه SF_Expanded      0x0000040 /* sqlite3SelectExpand() called on this */
#تعریف_کړه SF_HasTypeInfo   0x0000080 /* FROM subqueries have Table metadata */
#تعریف_کړه SF_Compound      0x0000100 /* Part of a compound query */
#تعریف_کړه SF_ارزښتونه        0x0000200 /* Synthesized from VALUES clause */
#تعریف_کړه SF_Multi_ارزښت    0x0000400 /* Single VALUES term with multiple rows */
#تعریف_کړه SF_NestedFrom    0x0000800 /* Part of a parenthesized FROM clause */
#تعریف_کړه SF_MinMaxAgg     0x0001000 /* Aggregate containing min() or max() */
#تعریف_کړه SF_Recursive     0x0002000 /* The recursive part of a recursive CTE */
#تعریف_کړه SF_Fixed_حد    0x0004000 /* nSelectRow set by a constant LIMIT */
/*                       0x0008000 // available for reuse */
#تعریف_کړه SF_Converted     0x0010000 /* By convertCompoundSelectToSubquery() */
#تعریف_کړه SF_IncludeHidden 0x0020000 /* Include hidden columns in output */
#تعریف_کړه SF_Complex_پایله 0x0040000 /* Result contains subquery or function */
#تعریف_کړه SF_WhereBegin    0x0080000 /* Really a WhereBegin() call.  Debug Only */
#تعریف_کړه SF_WinRewrite    0x0100000 /* Window function rewrite accomplished */
#تعریف_کړه SF_View          0x0200000 /* SELECT statement is a view */
/*                       0x0400000 // available for reuse */
#تعریف_کړه SF_UFSrcCheck    0x0800000 /* Check pSrc as required by UPDATE...FROM */
#تعریف_کړه SF_ټېل_ښکته      0x1000000 /* Modified by WHERE-clause push-down opt */
#تعریف_کړه SF_MultiPart     0x2000000 /* Has multiple incompatible PARTITIONs */
#تعریف_کړه SF_CopyCte       0x4000000 /* SELECT statement is a copy of a CTE */
#تعریف_کړه SF_ترتیب_By_Reqd   0x8000000 /* The ORDER BY clause may not be omitted */
#تعریف_کړه SF_UpdateFrom   0x10000000 /* Query originates with UPDATE FROM */
#تعریف_کړه SF_Correlated   0x20000000 /* True if references the outer context */
#تعریف_کړه SF_On_منزل_Where    0x40000000 /* One or more ON clauses moved to WHERE */

/* True if SrcItem X is a subquery that has SF_NestedFrom */
#تعریف_کړه IsNestedFrom(X) \
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
#تعریف_کړه SRT_Exists       1  /* Store 1 if the result is not empty */
#تعریف_کړه SRT_Discard      2  /* Do not save the results anywhere */
#تعریف_کړه SRT_DistFifo     3  /* Like SRT_Fifo, but unique results only */
#تعریف_کړه SRT_Dist_کتار    4  /* Like SRT_Queue, but unique results only */

/* The DISTINCT clause is ignored for all of the above.  Not that
** IgnorableDistinct() implies IgnorableOrderby() */
#تعریف_کړه IgnorableDistinct(X) ((X->eDest)<=SRT_Dist_کتار)

#تعریف_کړه SRT_کتار        5  /* Store result in an queue */
#تعریف_کړه SRT_Fifo         6  /* Store result as data with an automatic rowid */

/* The ORDER BY clause is ignored for all of the above */
#تعریف_کړه IgnorableOrderby(X) ((X->eDest)<=SRT_Fifo)

#تعریف_کړه SRT_Output       7  /* Output each row of result */
#تعریف_کړه SRT_Mem          8  /* Store result in a memory cell */
#تعریف_کړه SRT_سېټ          9  /* Store results as keys in an index */
#تعریف_کړه SRT_EphemTab    10  /* Create transient tab and store like SRT_Table */
#تعریف_کړه SRT_Coroutine   11  /* Generate a single row of result */
#تعریف_کړه SRT_جدول       12  /* Store result as data with an automatic rowid */
#تعریف_کړه SRT_Upfrom      13  /* Store result as data with rowid */

/*
** An instance of this object describes where to put of the results of
** a SELECT statement.
*/
بنية SelectDest {
  u8 eDest;            /* How to dispose of the results.  One of SRT_* above. */
  صحيح iSDParm;         /* A parameter used by the eDest disposal method */
  صحيح iSDParm2;        /* A second parameter for the eDest disposal method */
  صحيح iSdst;           /* Base register where results are written */
  صحيح nSdst;           /* Number of registers allocated */
  حرف *zAffSdst;      /* Affinity used for SRT_Set */
  بیان_List *p_ترتیب_By;  /* Key columns for SRT_Queue and SRT_DistQueue */
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
بنية AutoincInfo {
  AutoincInfo *p_بل;   /* Next info block in a list of them all */
  جدول *pTab;          /* Table this info block refers to */
  صحيح iDb;              /* Index in sqlite3.aDb[] of database holding pTab */
  صحيح regCtr;           /* Memory register holding the rowid counter */
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
بنية TriggerPrg {
  Trigger *pTrigger;      /* Trigger this program was coded from */
  TriggerPrg *p_بل;      /* Next entry in Parse.pTriggerPrg list */
  Sub_پروګرام *p_پروګرام;   /* Program implementing pTrigger/orconf */
  صحيح orconf;             /* Default ON CONFLICT policy */
  u32 aColmask[2];        /* Masks of old.*, new.* columns accessed */
};

/*
** The yDbMask datatype for the bitmask of all attached databases.
*/
#إذا SQLITE_MAX_ATTACHED>30
  تعريفنوع بینښه حرف y_Db_ماسک[(SQLITE_MAX_ATTACHED+9)/8];
# define Db_ماسک_Test(M,I)    (((M)[(I)/8]&(1<<((I)&7)))!=0)
# define Db_ماسک_Zero(M)      memset((M),0,حجم(M))
# define Db_ماسک_سېټ(M,I)     (M)[(I)/8]|=(1<<((I)&7))
# define Db_ماسک_All_Zero(M)   sqlite3_Db_ماسک_All_Zero(M)
# define Db_ماسک_Non_Zero(M)   (sqlite3_Db_ماسک_All_Zero(M)==0)
#وإلا
  تعريفنوع بینښه صحيح y_Db_ماسک;
# define Db_ماسک_Test(M,I)    (((M)&(((y_Db_ماسک)1)<<(I)))!=0)
# define Db_ماسک_Zero(M)      ((M)=0)
# define Db_ماسک_سېټ(M,I)     ((M)|=(((y_Db_ماسک)1)<<(I)))
# define Db_ماسک_All_Zero(M)   ((M)==0)
# define Db_ماسک_Non_Zero(M)   ((M)!=0)
#د_که_پای

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
بنية شاخصي_بیان {
  بیان *p_بیان;            /* The expression contained in the index */
  صحيح i_معلومات_Cur;           /* The data cursor associated with the index */
  صحيح iIdxCur;            /* The index cursor */
  صحيح i_Idx_ستون;            /* The index column that contains value of pExpr */
  u8 b_Maybe_Null_لیکه;       /* True if we need an OP_IfNullRow check */
  u8 aff;                 /* Affinity of the pExpr expression */
  شاخصي_بیان *pIENext;   /* Next in a list of all indexed expressions */
#که_تعریف_شوی SQLITE_ENABLE_EXPLAIN_COMMENTS
  ثابت حرف *zIdxName;   /* Name of index, used only for bytecode comments */
#د_که_پای
};

/*
** An instance of the ParseCleanup object specifies an operation that
** should be performed after parsing to deallocation resources obtained
** during the parse and which are no longer needed.
*/
بنية ParseCleanup {
  ParseCleanup *p_بل;               /* Next cleanup task */
  فراغ *pPtr;                        /* Pointer to object to deallocate */
  فراغ (*xCleanup)(sqlite3*,فراغ*);  /* Deallocation routine */
};

/*
** Number of 64-bit entried in the variable number bitmap cache (VNBMC).
*/
#که_نه_تعریف_شوی SQLITE_VNBMC
# define SQLITE_VNBMC 2
#د_که_پای

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
بنية Parse {
  sqlite3 *db;         /* The main database structure */
  حرف *z_Err_پیغام;       /* An error message */
  Vdbe *pVdbe;         /* An engine for executing database bytecode */
  صحيح rc;              /* Return code from execution */
  LogEst nQueryLoop;   /* Est number of iterations of a query (10*log2(N)) */
  u8 nested;           /* Number of nested calls to the parser/code generator */
  u8 n_لنډمهاله_Reg;         /* Number of temporary registers in aTempReg[] */
  u8 isMultiWrite;     /* True if statement may modify/insert multiple rows */
  u8 disableLookaside; /* Number of times lookaside has been disabled */
  u8 prepFlags;        /* SQLITE_PREPARE_* flags */
  u8 withinRJSubrtn;   /* Nesting level for RIGHT JOIN body subroutines */
  u8 mSubrtnSig;       /* mini Bloom filter on available SubrtnSig.selId */
  u8 eTriggerOp;       /* TK_UPDATE, TK_INSERT or TK_DELETE */
  u8 eOrconf;          /* Default ON CONFLICT policy for trigger steps */
#إذا defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
  u8 earlyCleanup;     /* OOM inside sqlite3ParserAddCleanup() */
#د_که_پای
#که_تعریف_شوی SQLITE_DEBUG
  u8 ifNotExists;      /* Might be true if IF NOT EXISTS.  Assert()s only */
  u8 isCreate;         /* CREATE TABLE, INDEX, or VIEW (but not TRIGGER)
                       ** and ALTER TABLE ADD COLUMN. */
#د_که_پای
  bft disableTriggers:1; /* True to disable triggers */
  bft mayAbort :1;     /* True if statement may throw an ABORT exception */
  bft hasCompound :1;  /* Need to invoke convertCompoundSelectToSubquery() */
  bft bReturning :1;   /* Coding a RETURNING trigger */
  bft bHasExists :1;   /* Has a correlated "EXISTS (SELECT ....)" expression */
  bft ستون_نومونه_سېټ :1;  /* TRUE after OP_ColumnName has been issued to pVdbe */
  bft bHasWith :1;     /* True if statement contains WITH */
  bft ok_Const_ضریب:1; /* OK to factor out constants */
  bft checkSchema :1;  /* Causes schema cookie check after an error */
  bft کاروي_Ainc :1;     /* True if pAinc is valid */
  صحيح nRangeReg;       /* Size of the temporary register block */
  صحيح iRangeReg;       /* First register in temporary register block */
  صحيح nErr;            /* Number of errors seen */
  صحيح nTab;            /* Number of previously allocated VDBE cursors */
  صحيح nMem;            /* Number of memory cells used so far */
  صحيح szOpAlloc;       /* Bytes of memory space allocated for Vdbe.aOp[] */
  صحيح iSelfTab;        /* Table associated with an index on expr, or negative
                       ** of the base register during check-constraint eval */
  صحيح nNestSel;        /* Number of nested SELECT statements and/or VIEWs */
  صحيح nLabel;          /* The *negative* of the number of labels used */
  صحيح nLabelAlloc;     /* Number of slots in aLabel */
  صحيح *aLabel;         /* Space to hold the labels */
  بیان_List *p_Const_بیان;/* Constant expressions */
  شاخصي_بیان *pIdxEpr;/* List of expressions used by active indexes */
  شاخصي_بیان *p_Idx_Part_بیان; /* Exprs constrained by index WHERE clauses */
  y_Db_ماسک write_ماسک;   /* Start a write transaction on these databases */
  y_Db_ماسک cookie_ماسک;  /* Bitmask of schema verified databases */
  صحيح nMaxArg;         /* Max args to xUpdate and xFilter vtab methods */
  صحيح nSelect;         /* Number of SELECT stmts. Counter for Select.selId */
#که_نه_تعریف_شوی SQLITE_OMIT_پرمختګ_CALLBACK
  u32 n_پرمختګ_ګامونه;  /* xProgress steps taken during sqlite3_prepare() */
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_SHARED_CACHE
  صحيح n_جدول_قلف;        /* Number of locks in aTableLock */
#د_که_پای
  Parse *pToplevel;    /* Parse structure for main program (or NULL) */
  جدول *pTriggerTab;  /* Table triggers are being coded for */
  TriggerPrg *pTriggerPrg;  /* Linked list of coded triggers */
  ParseCleanup *pCleanup;   /* List of cleanup operations to run after parse */
  Trigger *pTrigCoded;      /* Trigger whose sub-program is being coded */

  /**************************************************************************
  ** Fields above must be initialized to zero.  The fields that follow,
  ** down to the beginning of the recursive section, do not need to be
  ** initialized as they will be set before being used.  The boundary is
  ** determined by offsetof(Parse,aTempReg).
  **************************************************************************/

  صحيح a_لنډمهاله_Reg[8];        /* Holding area for temporary registers */
  Parse *pOuterParse;     /* Outer Parse object when nested */
  Token sNameToken;       /* Token with unqualified schema object name */
  u32 oldmask;            /* Mask of old.* columns referenced */
  u32 newmask;            /* Mask of new.* columns referenced */
  اتحاد {
    بنية {  /* These fields available when isCreate is true */
      صحيح addrCrTab;        /* Address of OP_CreateBtree on CREATE TABLE */
      صحيح regRowid;         /* Register holding rowid of CREATE TABLE entry */
      صحيح reg_ریښه;          /* Register holding root page for new objects */
      Token constraintName; /* Name of the constraint currently being parsed */
    } cr;
    بنية {  /* These fields available to all other statements */
      Returning *pReturning; /* The RETURNING clause */
    } d;
  } u1;
  AutoincInfo *pAinc;     /* Information about AUTOINCREMENT counters. Only
                          ** valid if Parse.usesAinc is true */
#که_نه_تعریف_شوی SQLITE_OMIT_SHARED_CACHE
  جدول_قلف *a_جدول_قلف;  /* Required table locks for shared-cache mode. Only
                          ** valid if Parse.nTableLock>0 */
#د_که_پای


  /************************************************************************
  ** Above is constant between recursions.  Below is reset before and after
  ** each recursion.  The boundary between these two regions is determined
  ** using offsetof(Parse,sLastToken) so the sLastToken field must be the
  ** first field in the recursive region.
  ************************************************************************/

  Token s_وروستی_Token;       /* The last token parsed */
  ynVar nVar;               /* Number of '?' variables seen in the SQL so far */
  u64 aVnbmc[SQLITE_VNBMC]; /* Varible Number Bitmap Cache */
  u8 i_Pk_ترتیبول_ترتیب;          /* ASC or DESC for INTEGER PRIMARY KEY */
  u8 explain;               /* True if the EXPLAIN flag is found on the query */
  u8 e_Parse_ډېرپېښ;            /* PARSE_MODE_XXX constant */
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
  صحيح n_Vtab_قلف;            /* Number of virtual tables to lock */
#د_که_پای
  صحيح n_لوړوالی;              /* Expression tree height of current sub-select */
  صحيح addrExplain;          /* Address of current OP_Explain opcode */
  VList *pVList;            /* Mapping between variable names and numbers */
  Vdbe *pReprepare;         /* VM being reprepared (sqlite3Reprepare()) */
  ثابت حرف *z_لکۍ;        /* All SQL text past the last semicolon parsed */
  جدول *p_New_جدول;         /* A table being constructed by CREATE TABLE */
  شاخص *p_New_شاخص;         /* An index being constructed by CREATE INDEX.
                            ** Also used to hold redundant UNIQUE constraints
                            ** during a RENAME COLUMN */
  Trigger *pNewTrigger;     /* Trigger under construct by a CREATE TRIGGER */
  ثابت حرف *zAuthContext; /* The 6th parameter to db->xAuth callbacks */
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
  Token sArg;               /* Complete text of a module argument */
  جدول **ap_Vtab_قلف;       /* Pointer to virtual tables needing locking */
#د_که_پای
  With *pWith;              /* Current WITH clause, or NULL */
#که_نه_تعریف_شوی SQLITE_OMIT_ALTERTABLE
  RenameToken *pRename;     /* Tokens subject to renaming by ALTER TABLE */
#د_که_پای
};

/* Allowed values for Parse.eParseMode
*/
#تعریف_کړه PARSE_ډېرپېښ_نورمال        0
#تعریف_کړه PARSE_ډېرپېښ_DECLARE_VTAB  1
#تعریف_کړه PARSE_ډېرپېښ_RENAME        2
#تعریف_کړه PARSE_ډېرپېښ_UNMAP         3

/*
** Sizes and pointers of various parts of the Parse object.
*/
#تعریف_کړه PARSE_HDR(X)  (((حرف*)(X))+offsetof(Parse,z_Err_پیغام))
#تعریف_کړه PARSE_HDR_SZ (offsetof(Parse,a_لنډمهاله_Reg)-offsetof(Parse,z_Err_پیغام)) /* Recursive part w/o aColCache*/
#تعریف_کړه PARSE_RECURSE_SZ offsetof(Parse,s_وروستی_Token)    /* Recursive part */
#تعریف_کړه PARSE_لکۍ_SZ (حجم(Parse)-PARSE_RECURSE_SZ) /* Non-recursive part */
#تعریف_کړه PARSE_لکۍ(X) (((حرف*)(X))+PARSE_RECURSE_SZ)  /* Pointer to tail */

/*
** Return true if currently inside an sqlite3_declare_vtab() call.
*/
#که_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
  #تعریف_کړه دننه_DECLARE_VTAB 0
#وإلا
  #تعریف_کړه دننه_DECLARE_VTAB (pParse->e_Parse_ډېرپېښ==PARSE_ډېرپېښ_DECLARE_VTAB)
#د_که_پای

#إذا defined(SQLITE_OMIT_ALTERTABLE)
  #تعریف_کړه دننه_RENAME_OBJECT 0
#وإلا
  #تعریف_کړه دننه_RENAME_OBJECT (pParse->e_Parse_ډېرپېښ>=PARSE_ډېرپېښ_RENAME)
#د_که_پای

#إذا defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_OMIT_ALTERTABLE)
  #تعریف_کړه دننه_SPECIAL_PARSE 0
#وإلا
  #تعریف_کړه دننه_SPECIAL_PARSE (pParse->e_Parse_ډېرپېښ!=PARSE_ډېرپېښ_نورمال)
#د_که_پای

/*
** An instance of the following structure can be declared on a stack and used
** to save the Parse.zAuthContext value so that it can be restored later.
*/
بنية AuthContext {
  ثابت حرف *zAuthContext;   /* Put saved Parse.zAuthContext here */
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
#تعریف_کړه OPFLAG_NCHANGE       0x01    /* OP_Insert: Set to update db->nChange */
                                     /* Also used in P2 (not P5) of OP_Delete */
#تعریف_کړه OPFLAG_NOCHNG        0x01    /* OP_VColumn nochange for UPDATE */
#تعریف_کړه OPFLAG_EPHEM         0x01    /* OP_Column: Ephemeral output is ok */
#تعریف_کړه OPFLAG_LASTROWID     0x20    /* Set to update db->lastRowid */
#تعریف_کړه OPFLAG_ISUPDATE      0x04    /* This OP_Insert is an sql UPDATE */
#تعریف_کړه OPFLAG_APPEND        0x08    /* This is likely to be an append */
#تعریف_کړه OPFLAG_USESEEKRESULT 0x10    /* Try to avoid a seek in BtreeInsert() */
#تعریف_کړه OPFLAG_ISNOOP        0x40    /* OP_Delete does pre-update-hook only */
#تعریف_کړه OPFLAG_LENGTHARG     0x40    /* OP_Column only used for length() */
#تعریف_کړه OPFLAG_TYPEOFARG     0x80    /* OP_Column only used for typeof() */
#تعریف_کړه OPFLAG_BYTELENARG    0xc0    /* OP_Column only for octet_length() */
#تعریف_کړه OPFLAG_BULKCSR       0x01    /* OP_Open** used to open bulk cursor */
#تعریف_کړه OPFLAG_SEEKEQ        0x02    /* OP_Open** cursor uses EQ seek only */
#تعریف_کړه OPFLAG_FORDELETE     0x08    /* OP_Open should use BTREE_FORDELETE */
#تعریف_کړه OPFLAG_P2ISREG       0x10    /* P2 to OP_Open** is a register number */
#تعریف_کړه OPFLAG_ترتیب_بدلول       0x01    /* OP_Compare: use the permutation */
#تعریف_کړه OPFLAG_SAVEPOSITION  0x02    /* OP_Delete/Insert: save cursor pos */
#تعریف_کړه OPFLAG_AUXDELETE     0x04    /* OP_Delete: index in a DELETE op */
#تعریف_کړه OPFLAG_NOCHNG_MAGIC  0x6d    /* OP_MakeRecord: serialtype 10 is ok */
#تعریف_کړه OPFLAG_PREFORMAT     0x80    /* OP_Insert uses preformatted cell */

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
بنية Trigger {
  حرف *zName;            /* The name of the trigger                        */
  حرف *جدول_2;            /* The table or view to which the trigger applies */
  u8 op;                  /* One of TK_DELETE, TK_UPDATE, TK_INSERT         */
  u8 tr_tm;               /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  u8 bReturning;          /* This trigger implements a RETURNING clause */
  بیان *pWhen;            /* The WHEN clause of the expression (may be NULL) */
  IdList *pColumns;       /* If this is an UPDATE OF <column-list> trigger,
                             the <column-list> is stored here */
  Schema *pSchema;        /* Schema containing the trigger */
  Schema *pTabSchema;     /* Schema containing the table */
  Trigger_ګام *ګام_list; /* Link list of trigger program steps             */
  Trigger *p_بل;         /* Next trigger associated with the table */
};

/*
** A trigger is either a BEFORE or an AFTER trigger.  The following constants
** determine which.
**
** If there are multiple triggers, you might of some BEFORE and some AFTER.
** In that cases, the constants below can be ORed together.
*/
#تعریف_کړه TRIGGER_BEFORE  1
#تعریف_کړه TRIGGER_AFTER   2

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
بنية Trigger_ګام {
  u8 op;               /* One of TK_DELETE, TK_UPDATE, TK_INSERT, TK_SELECT,
                       ** or TK_RETURNING */
  u8 orconf;           /* OE_Rollback etc. */
  Trigger *pTrig;      /* The trigger that this step is a part of */
  Select *pSelect;     /* SELECT statement or RHS of INSERT INTO SELECT ... */
  SrcList *pSrc;       /* Table to insert/update/delete */
  بیان *pWhere;        /* The WHERE clause for DELETE or UPDATE steps */
  بیان_List *p_بیان_List; /* SET clause for UPDATE, or RETURNING clause */
  IdList *pIdList;     /* Column names for INSERT */
  Upsert *pUpsert;     /* Upsert clauses on an INSERT */
  حرف *z_پراختیا;         /* Original SQL text of this command */
  Trigger_ګام *p_بل;  /* Next in the link-list */
  Trigger_ګام *p_وروستی;  /* Last element in link-list. Valid for 1st elem only */
};

/*
** Information about a RETURNING clause
*/
بنية Returning {
  Parse *pParse;        /* The parse that includes the RETURNING clause */
  بیان_List *pReturnEL;  /* List of expressions to return */
  Trigger retTrig;      /* The transient trigger that implements RETURNING */
  Trigger_ګام retTStep; /* The trigger step */
  صحيح iRetCur;          /* Transient table holding RETURNING results */
  صحيح n_Ret_ستون;          /* Number of in pReturnEL after expansion */
  صحيح iRetReg;          /* Register array for holding a row of RETURNING */
  حرف zName[40];       /* Name of trigger: "sqlite_returning_%p" */
};

/*
** An object used to accumulate the text of a string where we
** do not necessarily know how big the string will be in the end.
*/
بنية sqlite3_str {
  sqlite3 *db;         /* Optional database for lookaside.  Can be NULL */
  حرف *z_متن;         /* The string collected so far */
  u32  nAlloc;         /* Amount of space allocated in zText */
  u32  mxAlloc;        /* Maximum allowed allocation.  0 for no malloc usage */
  u32  nChar;          /* Length of the string so far */
  u8   accError;       /* SQLITE_NOMEM or SQLITE_TOOBIG */
  u8   printfFlags;    /* SQLITE_PRINTF flags below */
};
#تعریف_کړه SQLITE_PRINTF_دننی 0x01  /* Internal-use-only converters allowed */
#تعریف_کړه SQLITE_PRINTF_SQLFUNC  0x02  /* SQL function arguments to VXPrintf */
#تعریف_کړه SQLITE_PRINTF_MALLOCED 0x04  /* True if zText is allocated space */

#تعریف_کړه isMalloced(X)  (((X)->printfFlags & SQLITE_PRINTF_MALLOCED)!=0)

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
بنية RCStr {
  u64 nRCRef;            /* Number of references */
  /* Total structure size should be a multiple of 8 bytes for alignment */
};

/*
** A pointer to this structure is used to communicate information
** from sqlite3Init and OP_ParseSchema into the sqlite3InitCallback.
*/
تعريفنوع بنية {
  sqlite3 *db;        /* The database being initialized */
  حرف **pz_Err_پیغام;    /* Error message stored here */
  صحيح iDb;            /* 0 for main database.  1 for TEMP, 2.. for ATTACHed */
  صحيح rc;             /* Result code stored here */
  u32 mInitFlags;     /* Flags controlling error messages */
  u32 n_Init_لیکه;       /* Number of rows processed */
  Pgno mx_پاڼه;        /* Maximum page number.  0 for no limit. */
} Init_معلومات;

/*
** Allowed values for mInitFlags
*/
#تعریف_کړه INITFLAG_Alter_ماسک     0x0007  /* Types of ALTER */
#تعریف_کړه INITFLAG_AlterRename   0x0001  /* Reparse after a RENAME */
#تعریف_کړه INITFLAG_AlterDrop     0x0002  /* Reparse after a DROP COLUMN */
#تعریف_کړه INITFLAG_Alter_جمع_کړه      0x0003  /* Reparse after an ADD COLUMN */
#تعریف_کړه INITFLAG_AlterDropCons 0x0004  /* Reparse after a DROP CONSTRAINT */

/* Tuning parameters are set using SQLITE_TESTCTRL_TUNE and are controlled
** on debug-builds of the CLI using ".testctrl tune ID VALUE".  Tuning
** parameters are for temporary use during development, to help find
** optimal values for parameters in the query planner.  The should not
** be used on trunk check-ins.  They are a temporary mechanism available
** for transient development builds only.
**
** Tuning parameters are numbered starting with 1.
*/
#تعریف_کړه SQLITE_NTUNE  6             /* Should be zero for all trunk check-ins */
#که_تعریف_شوی SQLITE_DEBUG
# define Tuning(X)  (sqlite3Config.aTune[(X)-1])
#وإلا
# define Tuning(X)  0
#د_که_پای

/*
** Structure containing global configuration data for the SQLite library.
**
** This structure also contains some state information.
*/
بنية Sqlite3Config {
  صحيح bMemstat;                     /* True to enable memory status */
  u8 bCoreMutex;                    /* True to enable core mutexing */
  u8 b_ډک_Mutex;                    /* True to enable full mutexing */
  u8 bOpenUri;                      /* True to interpret filenames as URIs */
  u8 bUseCis;                       /* Use covering indices for full-scans */
  u8 bSmallMalloc;                  /* Avoid large memory allocations if true */
  u8 bExtraSchemaChecks;            /* Verify type,name,tbl_name in schema */
#که_تعریف_شوی SQLITE_DEBUG
  u8 bJsonSelfcheck;                /* Double-check JSON parsing */
#د_که_پای
  صحيح mxStrlen;                     /* Maximum string length */
  صحيح neverCorrupt;                 /* Database is always well-formed */
  صحيح szLookaside;                  /* Default lookaside buffer size */
  صحيح nLookaside;                   /* Default lookaside buffer count */
  صحيح nStmtSpill;                   /* Stmt-journal spill-to-disk threshold */
  sqlite3_mem_methods m;            /* Low-level memory allocation interface */
  sqlite3_mutex_methods mutex;      /* Low-level mutex interface */
  sqlite3_pcache_methods2 pcache2;  /* Low-level page-cache interface */
  فراغ *p_ډېرۍ;                      /* Heap storage space */
  صحيح n_ډېرۍ;                        /* Size of pHeap[] */
  صحيح mnReq, mxReq;                 /* Min and max heap requests sizes */
  sqlite3_int64 szMmap;             /* mmap() space per open file */
  sqlite3_int64 mxMmap;             /* Maximum value for szMmap */
  فراغ *p_پاڼه;                      /* Page cache memory */
  صحيح sz_پاڼه;                       /* Size of each page in pPage[] */
  صحيح n_پاڼه;                        /* Number of pages in pPage[] */
  صحيح mx_Parser_ډېری;                /* maximum depth of the parser stack */
  صحيح sharedCacheEnabled;           /* true if shared-cache mode enabled */
  u32 szPma;                        /* Maximum Sorter PMA size */
  /* The above might be initialized to non-zero.  The following need to always
  ** initially be zero, however. */
  صحيح isInit;                       /* True after initialization has finished */
  صحيح دننه_پرمختګ;                   /* True while initialization in progress */
  صحيح isMutexInit;                  /* True after mutexes are initialized */
  صحيح isMallocInit;                 /* True after malloc is initialized */
  صحيح isPCacheInit;                 /* True after malloc is initialized */
  صحيح nRefInitMutex;                /* Number of users of pInitMutex */
  sqlite3_mutex *pInitMutex;        /* Mutex used by sqlite3_initialize() */
  فراغ (*xLog)(فراغ*,صحيح,ثابت حرف*); /* Function for logging */
  فراغ *pLogArg;                       /* First argument to xLog() */
#که_تعریف_شوی SQLITE_ENABLE_SQLLOG
  فراغ(*xSqllog)(فراغ*,sqlite3*,ثابت حرف*, صحيح);
  فراغ *pSqllogArg;
#د_که_پای
#که_تعریف_شوی SQLITE_VDBE_COVERAGE
  /* The following callback (if not NULL) is invoked on every VDBE branch
  ** operation.  Set the callback using SQLITE_TESTCTRL_VDBE_COVERAGE.
  */
  فراغ (*xVdbeBranch)(فراغ*,بینښه i_Src_کرښه,u8 eThis,u8 eMx);  /* Callback */
  فراغ *pVdbeBranchArg;                                     /* 1st argument */
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_DESERIALIZE
  sqlite3_int64 mx_Memdb_اندازه;        /* Default max memdb size */
#د_که_پای
#که_نه_تعریف_شوی SQLITE_UNTESTABLE
  صحيح (*xTestCallback)(صحيح);        /* Invoked by sqlite3FaultSim() */
#د_که_پای
#که_تعریف_شوی SQLITE_ALLOW_ROWID_دننه_VIEW
  u32 mNoVisibleRowid;              /* TF_NoVisibleRowid if the ROWID_IN_VIEW
                                    ** feature is disabled.  0 if rowids can
                                    ** occur in views. */
#د_که_پای
  صحيح bLocaltimeFault;              /* True to fail localtime() calls */
  صحيح (*xAltLocaltime)(ثابت فراغ*,فراغ*); /* Alternative localtime() routine */
  صحيح i_Once_Reset_پوله;          /* When to reset OP_Once counters */
  u32 szSorterRef;                  /* Min size in bytes to use sorter-refs */
  بینښه صحيح i_Prng_تخم;           /* Alternative fixed seed for the PRNG */
  /* vvvv--- must be last ---vvv */
#که_تعریف_شوی SQLITE_DEBUG
  sqlite3_int64 aTune[SQLITE_NTUNE]; /* Tuning parameters */
#د_که_پای
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
#تعریف_کړه CORRUPT_DB  (sqlite3Config.neverCorrupt==0)

/*
** Context pointer passed down through the tree-walk.
*/
بنية Walker {
  Parse *pParse;                            /* Parser context.  */
  صحيح (*x_بیان_Callback)(Walker*, بیان*);     /* Callback for expressions */
  صحيح (*xSelectCallback)(Walker*,Select*);  /* Callback for SELECTs */
  فراغ (*xSelectCallback2)(Walker*,Select*);/* Second callback for SELECTs */
  صحيح walker_ژوروالی;                          /* Number of subqueries */
  u16 e_کوډ;                                /* A small processing code */
  u16 mWFlags;                              /* Use-dependent flags */
  اتحاد {                                   /* Extra data for callback */
    NameContext *pNC;                         /* Naming context */
    صحيح n;                                    /* A counter */
    صحيح iCur;                                 /* A cursor number */
    صحيح sz;                                   /* String literal length */
    SrcList *pSrcList;                        /* FROM clause */
    بنية CCurHint *pCCurHint;               /* Used by codeCursorHint() */
    بنية RefSrcList *pRefSrcList;           /* sqlite3ReferencesSrcList() */
    صحيح *ai_ستون;                               /* array of column indexes */
    بنية IdxCover *pIdxCover;               /* Check for index coverage */
    بیان_List *p_ډله_By;                       /* GROUP BY clause */
    Select *pSelect;                          /* HAVING to WHERE clause ctx */
    بنية کړکۍ_Rewrite *pRewrite;           /* Window rewrite context */
    بنية WhereConst *pConst;                /* WHERE clause constants */
    بنية RenameCtx *pRename;                /* RENAME COLUMN context */
    بنية جدول *pTab;                       /* Table of generated column */
    بنية Covering_شاخص_Check *pCovIdxCk;     /* Check for covering index */
    Src_توکی *p_Src_توکی;                        /* A single FROM clause item */
    DbFixer *pFix;                            /* See sqlite3FixSelect() */
    Mem *aMem;                                /* See sqlite3BtreeCursorHint() */
    بنية CheckOnCtx *pCheckOnCtx;           /* See selectCheckOnClauses() */
  } u;
};

/*
** The following structure contains information used by the sqliteFix...
** routines as they walk the parse tree to make database references
** explicit.
*/
بنية DbFixer {
  Parse *pParse;      /* The parsing context.  Error messages written here */
  Walker w;           /* Walker object */
  Schema *pSchema;    /* Fix items to this schema */
  u8 b_لنډمهاله;           /* True for TEMP schema entries */
  ثابت حرف *zDb;    /* Make sure all objects are contained in this database */
  ثابت حرف *zType;  /* Type of the container - used for error messages */
  ثابت Token *pName; /* Name of the container - used for error messages */
};

/* Forward declarations */
صحيح sqlite3_Walk_بیان(Walker*, بیان*);
صحيح sqlite3_Walk_بیان_NN(Walker*, بیان*);
صحيح sqlite3_Walk_بیان_List(Walker*, بیان_List*);
صحيح sqlite3WalkSelect(Walker*, Select*);
صحيح sqlite3_Walk_Select_بیان(Walker*, Select*);
صحيح sqlite3WalkSelectFrom(Walker*, Select*);
صحيح sqlite3_بیان_Walk_Noop(Walker*, بیان*);
صحيح sqlite3SelectWalkNoop(Walker*, Select*);
صحيح sqlite3SelectWalkFail(Walker*, Select*);
صحيح sqlite3_Walker_ژوروالی_Increase(Walker*,Select*);
فراغ sqlite3_Walker_ژوروالی_Decrease(Walker*,Select*);
فراغ sqlite3WalkWinDefnDummyCallback(Walker*,Select*);

#که_تعریف_شوی SQLITE_DEBUG
فراغ sqlite3SelectWalkAssert2(Walker*, Select*);
#د_که_پای

#که_نه_تعریف_شوی SQLITE_OMIT_CTE
فراغ sqlite3_Select_راوباسه_With(Walker*, Select*);
#وإلا
# define sqlite3_Select_راوباسه_With 0
#د_که_پای

/*
** Return code from the parse-tree walking primitives and their
** callbacks.
*/
#تعریف_کړه WRC_Continue    0   /* Continue down into children */
#تعریف_کړه WRC_Prune       1   /* Omit children but continue walking siblings */
#تعریف_کړه WRC_Abort       2   /* Abandon the tree walk */

/*
** A single common table expression
*/
بنية Cte {
  حرف *zName;            /* Name of this CTE */
  بیان_List *p_ستون_شمېر;        /* List of explicit column names, or NULL */
  Select *pSelect;        /* The definition of this CTE */
  ثابت حرف *zCteErr;    /* Error message for circular references */
  CteUse *pUse;           /* Usage information for this CTE */
  u8 eM10d;               /* The MATERIALIZED flag */
};

/*
** Allowed values for the materialized flag (eM10d):
*/
#تعریف_کړه M10d_Yes       0  /* AS MATERIALIZED */
#تعریف_کړه M10d_Any       1  /* Not specified.  Query planner's choice */
#تعریف_کړه M10d_No        2  /* AS NOT MATERIALIZED */

/*
** An instance of the With object represents a WITH clause containing
** one or more CTEs (common table expressions).
*/
بنية With {
  صحيح nCte;               /* Number of CTEs in the WITH clause */
  صحيح bView;              /* Belongs to the outermost Select of a view */
  With *pOuter;           /* Containing WITH clause, or NULL */
  Cte a[FLEXARRAY];       /* For each CTE in the WITH clause.... */
};

/* The size (in bytes) of a With object that can hold as many
** as N different CTEs. */
#تعریف_کړه SZ_WITH(N)  (offsetof(With,a) + (N)*حجم(Cte))

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
بنية CteUse {
  صحيح nUse;              /* Number of users of this CTE */
  صحيح addrM9e;           /* Start of subroutine to compute materialization */
  صحيح regRtn;            /* Return address register for addrM9e subroutine */
  صحيح iCur;              /* Ephemeral table holding the materialization */
  LogEst n_لیکه_Est;        /* Estimated number of rows in the table */
  u8 eM10d;              /* The MATERIALIZED flag */
};


/* Client data associated with sqlite3_set_clientdata() and
** sqlite3_get_clientdata().
*/
بنية Db_Client_معلومات {
  Db_Client_معلومات *p_بل;        /* Next in a linked list */
  فراغ *p_معلومات;                /* The data */
  فراغ (*xDestructor)(فراغ*); /* Destructor.  Might be NULL */
  حرف zName[FLEXARRAY];      /* Name of this client data. MUST BE LAST */
};

/* The size (in bytes) of a DbClientData object that can has a name
** that is N bytes long, including the zero-terminator. */
#تعریف_کړه SZ_DBCLIENTDATA(N) (offsetof(Db_Client_معلومات,zName)+(N))

#که_تعریف_شوی SQLITE_DEBUG
/*
** An instance of the TreeView object is used for printing the content of
** data structures on sqlite3DebugPrintf() using a tree-like view.
*/
بنية ونه_View {
  صحيح iLevel;             /* Which level of the tree we are on */
  u8  b_کرښه[100];         /* Draw vertical in column i if bLine[i] is true */
};
#د_که_پای /* SQLITE_DEBUG */

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
بنية کړکۍ {
  حرف *zName;            /* Name of window (may be NULL) */
  حرف *z_بنسټ;            /* Name of base window for chaining (may be NULL) */
  بیان_List *p_برخه_کول;   /* PARTITION BY clause */
  بیان_List *p_ترتیب_By;     /* ORDER BY clause */
  u8 eFrmType;            /* TK_RANGE, TK_GROUPS, TK_ROWS, or 0 */
  u8 e_پیل;              /* UNBOUNDED, CURRENT, PRECEDING or FOLLOWING */
  u8 e_پای_ټکی;                /* UNBOUNDED, CURRENT, PRECEDING or FOLLOWING */
  u8 b_Implicit_چوکاټ;      /* True if frame was implicitly specified */
  u8 eExclude;            /* TK_NO, TK_CURRENT, TK_TIES, TK_GROUP, or 0 */
  بیان *p_پیل;           /* Expression for "<expr> PRECEDING" */
  بیان *p_پای_ټکی;             /* Expression for "<expr> FOLLOWING" */
  کړکۍ **ppThis;        /* Pointer to this object in Select.pWin list */
  کړکۍ *p_بل_Win;       /* Next window function belonging to this SELECT */
  بیان *pFilter;          /* The FILTER expression */
  FuncDef *pWFunc;        /* The function */
  صحيح iEphCsr;            /* Partition buffer or Peer buffer */
  صحيح regAccum;           /*راټولونکی*/
  صحيح reg_پایله;          /* Interim result */
  صحيح csrApp;             /* Function cursor (used by min/max) */
  صحيح regApp;             /* Function register (also used by min/max) */
  صحيح regPart;            /* Array of registers for PARTITION BY values */
  بیان *pOwner;           /* Expression object this window is attached to */
  صحيح n_بفر_ستون;         /* Number of columns in buffer table */
  صحيح i_Arg_ستون;            /* Offset of first argument for this function */
  صحيح regOne;             /* Register containing constant value 1 */
  صحيح reg_پیل_Rowid;
  صحيح reg_پای_ټکی_Rowid;
  u8 b_بیان_دلیلونه;           /* Defer evaluation of window function arguments
                          ** due to the SQLITE_SUBTYPE flag */
};

Select *sqlite3_Multi_ارزښتونه(Parse *pParse, Select *p_کیڼ, بیان_List *p_لیکه);
فراغ sqlite3_Multi_ارزښتونه_پای_ټکی(Parse *pParse, Select *pVal);

#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
فراغ sqlite3_کړکۍ_Delete(sqlite3*, کړکۍ*);
فراغ sqlite3_کړکۍ_Unlink_From_Select(کړکۍ*);
فراغ sqlite3_کړکۍ_List_Delete(sqlite3 *db, کړکۍ *p);
کړکۍ *sqlite3_کړکۍ_Alloc(Parse*, صحيح, صحيح, بیان*, صحيح , بیان*, u8);
فراغ sqlite3_کړکۍ_Attach(Parse*, بیان*, کړکۍ*);
فراغ sqlite3_کړکۍ_Link(Select *pSel, کړکۍ *pWin);
صحيح sqlite3_کړکۍ_Compare(ثابت Parse*, ثابت کړکۍ*, ثابت کړکۍ*, صحيح);
فراغ sqlite3_کړکۍ_کوډ_Init(Parse*, Select*);
فراغ sqlite3_کړکۍ_کوډ_ګام(Parse*, Select*, WhereInfo*, صحيح, صحيح);
صحيح sqlite3_کړکۍ_Rewrite(Parse*, Select*);
فراغ sqlite3_کړکۍ_Update(Parse*, کړکۍ*, کړکۍ*, FuncDef*);
کړکۍ *sqlite3_کړکۍ_Dup(sqlite3 *db, بیان *pOwner, کړکۍ *p);
کړکۍ *sqlite3_کړکۍ_List_Dup(sqlite3 *db, کړکۍ *p);
فراغ sqlite3_کړکۍ_Functions(فراغ);
فراغ sqlite3_کړکۍ_Chain(Parse*, کړکۍ*, کړکۍ*);
کړکۍ *sqlite3_کړکۍ_Assemble(Parse*, کړکۍ*, بیان_List*, بیان_List*, Token*);
#وإلا
# define sqlite3_کړکۍ_Delete(a,b)
# define sqlite3_کړکۍ_Functions()
# define sqlite3_کړکۍ_Attach(a,b,c)
#د_که_پای

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
#تعریف_کړه SQLITE_SKIP_UTF8(z_دننه) {                        \
  إذا( (*(z_دننه++))>=0xc0 ){                              \
    طالما( (*z_دننه & 0xc0)==0x80 ){ z_دننه++; }             \
  }                                                    \
}

/*
** The SQLITE_*_BKPT macros are substitutes for the error codes with
** the same name but without the _BKPT suffix.  These macros invoke
** routines that report the line-number on which the error originated
** using sqlite3_log().  The routines also provide a convenient place
** to set a debugger breakpoint.
*/
صحيح sqlite3ReportError(صحيح iErr, صحيح lineno, ثابت حرف *zType);
صحيح sqlite3CorruptError(صحيح);
صحيح sqlite3MisuseError(صحيح);
صحيح sqlite3CantopenError(صحيح);
#تعریف_کړه SQLITE_CORRUPT_BKPT sqlite3CorruptError(__LINE__)
#تعریف_کړه SQLITE_MISUSE_BKPT sqlite3MisuseError(__LINE__)
#تعریف_کړه SQLITE_CANTOPEN_BKPT sqlite3CantopenError(__LINE__)
#که_تعریف_شوی SQLITE_DEBUG
  صحيح sqlite3NomemError(صحيح);
  صحيح sqlite3IoerrnomemError(صحيح);
# define SQLITE_NOMEM_BKPT sqlite3NomemError(__LINE__)
# define SQLITE_IOERR_NOMEM_BKPT sqlite3IoerrnomemError(__LINE__)
#وإلا
# define SQLITE_NOMEM_BKPT SQLITE_NOMEM
# define SQLITE_IOERR_NOMEM_BKPT SQLITE_IOERR_NOMEM
#د_که_پای
#إذا defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_CORRUPT_PGNO)
  صحيح sqlite3CorruptPgnoError(صحيح,Pgno);
# define SQLITE_CORRUPT_PGNO(P) sqlite3CorruptPgnoError(__LINE__,(P))
#وإلا
# define SQLITE_CORRUPT_PGNO(P) sqlite3CorruptError(__LINE__)
#د_که_پای

/*
** FTS3 and FTS4 both require virtual table support
*/
#إذا defined(SQLITE_OMIT_VIRTUALTABLE)
# undef SQLITE_ENABLE_FTS3
# undef SQLITE_ENABLE_FTS4
#د_که_پای

/*
** FTS4 is really an extension for FTS3.  It is enabled using the
** SQLITE_ENABLE_FTS3 macro.  But to avoid confusion we also call
** the SQLITE_ENABLE_FTS4 macro to serve as an alias for SQLITE_ENABLE_FTS3.
*/
#إذا defined(SQLITE_ENABLE_FTS4) && !defined(SQLITE_ENABLE_FTS3)
# define SQLITE_ENABLE_FTS3 1
#د_که_پای

/*
** The following macros mimic the standard library functions toupper(),
** isspace(), isalnum(), isdigit() and isxdigit(), respectively. The
** sqlite versions only work for ASCII characters, regardless of locale.
*/
#که_تعریف_شوی SQLITE_ASCII
# define sqlite3Toupper(x)  ((x)&~(sqlite3CtypeMap[(بینښه حرف)(x)]&0x20))
# define sqlite3Isspace(x)   (sqlite3CtypeMap[(بینښه حرف)(x)]&0x01)
# define sqlite3Isalnum(x)   (sqlite3CtypeMap[(بینښه حرف)(x)]&0x06)
# define sqlite3Isalpha(x)   (sqlite3CtypeMap[(بینښه حرف)(x)]&0x02)
# define sqlite3Isdigit(x)   (sqlite3CtypeMap[(بینښه حرف)(x)]&0x04)
# define sqlite3Isxdigit(x)  (sqlite3CtypeMap[(بینښه حرف)(x)]&0x08)
# define sqlite3Tolower(x)   (sqlite3_پاسنی_منزل_ښکتنی[(بینښه حرف)(x)])
# define sqlite3Isquote(x)   (sqlite3CtypeMap[(بینښه حرف)(x)]&0x80)
# define sqlite3JsonId1(x)   (sqlite3CtypeMap[(بینښه حرف)(x)]&0x42)
# define sqlite3JsonId2(x)   (sqlite3CtypeMap[(بینښه حرف)(x)]&0x46)
#وإلا
# define sqlite3Toupper(x)   لوی_توری((بینښه حرف)(x))
# define sqlite3Isspace(x)   که_تشه((بینښه حرف)(x))
# define sqlite3Isalnum(x)   که_توری_یا_عدد((بینښه حرف)(x))
# define sqlite3Isalpha(x)   که_توری((بینښه حرف)(x))
# define sqlite3Isdigit(x)   که_عدد((بینښه حرف)(x))
# define sqlite3Isxdigit(x)  isxdigit((بینښه حرف)(x))
# define sqlite3Tolower(x)   tolower((بینښه حرف)(x))
# define sqlite3Isquote(x)   ((x)=='"'||(x)=='\''||(x)=='['||(x)=='`')
# define sqlite3JsonId1(x)   (sqlite3IsIdChar(x)&&(x)<'0')
# define sqlite3JsonId2(x)   sqlite3IsIdChar(x)
#د_که_پای
صحيح sqlite3IsIdChar(u8);

/*
** Internal function prototypes
*/
صحيح sqlite3StrICmp(ثابت حرف*,ثابت حرف*);
صحيح sqlite3Strlen30(ثابت حرف*);
#تعریف_کړه sqlite3Strlen30NN(C) (مزي_اوږدوالی(C)&0x3fffffff)
حرف *sqlite3_ستنه_Type(ستنه*,حرف*);
#تعریف_کړه sqlite3StrNICmp sqlite3_strnicmp

صحيح sqlite3MallocInit(فراغ);
فراغ sqlite3_Malloc_پای_ټکی(فراغ);
فراغ *sqlite3Malloc(u64);
فراغ *sqlite3MallocZero(u64);
فراغ *sqlite3DbMallocZero(sqlite3*, u64);
فراغ *sqlite3DbMallocRaw(sqlite3*, u64);
فراغ *sqlite3DbMallocRawNN(sqlite3*, u64);
حرف *sqlite3DbStrDup(sqlite3*,ثابت حرف*);
حرف *sqlite3DbStrNDup(sqlite3*,ثابت حرف*, u64);
حرف *sqlite3_Db_پراختیا_Dup(sqlite3*,ثابت حرف*,ثابت حرف*);
فراغ *sqlite3Realloc(فراغ*, u64);
فراغ *sqlite3DbReallocOrFree(sqlite3 *, فراغ *, u64);
فراغ *sqlite3DbRealloc(sqlite3 *, فراغ *, u64);
فراغ sqlite3DbFree(sqlite3*, فراغ*);
فراغ sqlite3DbFreeNN(sqlite3*, فراغ*);
فراغ sqlite3DbNNFreeNN(sqlite3*, فراغ*);
صحيح sqlite3_Malloc_اندازه(ثابت فراغ*);
صحيح sqlite3_Db_Malloc_اندازه(sqlite3*, ثابت فراغ*);
فراغ *sqlite3_پاڼه_Malloc(صحيح);
فراغ sqlite3_پاڼه_Free(فراغ*);
فراغ sqlite3_Mem_سېټ_Default(فراغ);
#که_نه_تعریف_شوی SQLITE_UNTESTABLE
فراغ sqlite3BenignMallocHooks(فراغ (*)(فراغ), فراغ (*)(فراغ));
#د_که_پای
صحيح sqlite3_ډېرۍ_Nearly_ډک(فراغ);

/*
** On systems with ample stack space and that support alloca(), make
** use of alloca() to obtain space for large automatic objects.  By default,
** obtain space from malloc().
**
** The alloca() routine never returns NULL.  This will cause code paths
** that deal with sqlite3StackAlloc() failures to be unreachable.
*/
#که_تعریف_شوی SQLITE_USE_ALLOCA
# define sqlite3_ډېری_Alloc_Raw(D,N)   alloca(N)
# define sqlite3_ډېری_Alloc_Raw_NN(D,N) alloca(N)
# define sqlite3_ډېری_Free(D,P)
# define sqlite3_ډېری_Free_NN(D,P)
#وإلا
# define sqlite3_ډېری_Alloc_Raw(D,N)   sqlite3DbMallocRaw(D,N)
# define sqlite3_ډېری_Alloc_Raw_NN(D,N) sqlite3DbMallocRawNN(D,N)
# define sqlite3_ډېری_Free(D,P)       sqlite3DbFree(D,P)
# define sqlite3_ډېری_Free_NN(D,P)     sqlite3DbFreeNN(D,P)
#د_که_پای

/* Do not allow both MEMSYS5 and MEMSYS3 to be defined together.  If they
** are, disable MEMSYS3
*/
#که_تعریف_شوی SQLITE_ENABLE_MEMSYS5
ثابت sqlite3_mem_methods *sqlite3MemGetMemsys5(فراغ);
#تعریف_لغوه SQLITE_ENABLE_MEMSYS3
#د_که_پای
#که_تعریف_شوی SQLITE_ENABLE_MEMSYS3
ثابت sqlite3_mem_methods *sqlite3MemGetMemsys3(فراغ);
#د_که_پای


#که_نه_تعریف_شوی SQLITE_MUTEX_OMIT
  sqlite3_mutex_methods ثابت *sqlite3DefaultMutex(فراغ);
  sqlite3_mutex_methods ثابت *sqlite3NoopMutex(فراغ);
  sqlite3_mutex *sqlite3MutexAlloc(صحيح);
  صحيح sqlite3MutexInit(فراغ);
  صحيح sqlite3_Mutex_پای_ټکی(فراغ);
#د_که_پای
#إذا !defined(SQLITE_MUTEX_OMIT) && !defined(SQLITE_MUTEX_NOOP)
  فراغ sqlite3_حافظه_Barrier(فراغ);
#وإلا
# define sqlite3_حافظه_Barrier()
#د_که_پای

sqlite3_int64 sqlite3_حالت_ارزښت(صحيح);
فراغ sqlite3_حالت_پورته(صحيح, صحيح);
فراغ sqlite3_حالت_ښکته(صحيح, صحيح);
فراغ sqlite3_حالت_Highwater(صحيح, صحيح);
صحيح sqlite3_Lookaside_کارول_شوی(sqlite3*,صحيح*);

/* Access to mutexes used by sqlite3_status() */
sqlite3_mutex *sqlite3Pcache1Mutex(فراغ);
sqlite3_mutex *sqlite3MallocMutex(فراغ);


/* The SQLITE_THREAD_MISUSE_WARNINGS compile-time option used to be called
** SQLITE_ENABLE_MULTITHREADED_CHECKS.  Keep that older macro for backwards
** compatibility, at least for a while... */
#که_تعریف_شوی SQLITE_ENABLE_MULTITHREADED_CHECKS
# define SQLITE_THREAD_MISUSE_WARNINGS 1
#د_که_پای

/* SQLITE_THREAD_MISUSE_ABORT implies SQLITE_THREAD_MISUSE_WARNINGS */
#که_تعریف_شوی SQLITE_THREAD_MISUSE_ABORT
# define SQLITE_THREAD_MISUSE_WARNINGS 1
#د_که_پای

#إذا defined(SQLITE_THREAD_MISUSE_WARNINGS) && !defined(SQLITE_MUTEX_OMIT)
فراغ sqlite3MutexWarnOnContention(sqlite3_mutex*);
#وإلا
# define sqlite3MutexWarnOnContention(x)
#د_که_پای

#که_نه_تعریف_شوی SQLITE_OMIT_FLOATING_ټکی
# define EXP754 (((u64)0x7ff)<<52)
# define MAN754 ((((u64)1)<<52)-1)
# define IsNaN(X) (((X)&EXP754)==EXP754 && ((X)&MAN754)!=0)
# define IsOvfl(X) (((X)&EXP754)==EXP754)
  صحيح sqlite3IsNaN(مزدوج);
  صحيح sqlite3_Is_ډکېدنه(مزدوج);
#وإلا
# define IsNaN(X)             0
# define sqlite3IsNaN(X)      0
# define sqlite3_Is_ډکېدنه_2(X) 0
#د_که_پای

/*
** An instance of the following structure holds information about SQL
** functions arguments that are the parameters to the printf() function.
*/
بنية PrintfArguments {
  صحيح nArg;                /* Total number of arguments */
  صحيح n_کارول_شوی;               /* Number of arguments used so far */
  sqlite3_ارزښت **apArg;   /* The argument values */
};

/*
** Maxium number of base-10 digits in an unsigned 64-bit integer
*/
#تعریف_کړه SQLITE_U64_DIGITS 20

/*
** An instance of this object receives the decoding of a floating point
** value into an approximate decimal representation.
*/
بنية FpDecode {
  صحيح n;                           /* Significant digits in the decode */
  صحيح iDP;                         /* Location of the decimal point */
  حرف *z;                         /* Start of significant digits */
  حرف zBuf[SQLITE_U64_DIGITS+1];  /* Storage for significant digits */
  حرف sign;                       /* '+' or '-' */
  حرف isSpecial;                  /* 1: Infinity  2: NaN */
};

فراغ sqlite3FpDecode(FpDecode*,مزدوج,صحيح,صحيح);
حرف *sqlite3MPrintf(sqlite3*,ثابت حرف*, ...);
حرف *sqlite3VMPrintf(sqlite3*,ثابت حرف*, va_list);
#إذا defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  فراغ sqlite3DebugPrintf(ثابت حرف*, ...);
#د_که_پای
#إذا defined(SQLITE_TEST)
  فراغ *sqlite3_Test_متن_منزل_Ptr(ثابت حرف*);
  ثابت حرف *sqlite3_Test_Ptr_منزل_متن(فراغ*);
#د_که_پای

#إذا defined(SQLITE_DEBUG)
  فراغ sqlite3_ونه_View_کرښه(ونه_View*, ثابت حرف *zFormat, ...);
  فراغ sqlite3_ونه_View_بیان(ونه_View*, ثابت بیان*, u8);
  فراغ sqlite3_ونه_View_Bare_بیان_List(ونه_View*, ثابت بیان_List*, ثابت حرف*);
  فراغ sqlite3_ونه_View_بیان_List(ونه_View*, ثابت بیان_List*, u8, ثابت حرف*);
  فراغ sqlite3_ونه_View_Bare_Id_List(ونه_View*, ثابت IdList*, ثابت حرف*);
  فراغ sqlite3_ونه_View_Id_List(ونه_View*, ثابت IdList*, u8, ثابت حرف*);
  فراغ sqlite3_ونه_View_ستنه_List(ونه_View*, ثابت ستنه*, صحيح, u8);
  فراغ sqlite3_ونه_View_Src_List(ونه_View*, ثابت SrcList*);
  فراغ sqlite3_ونه_View_Select(ونه_View*, ثابت Select*, u8);
  فراغ sqlite3_ونه_View_With(ونه_View*, ثابت With*, u8);
  فراغ sqlite3_ونه_View_Upsert(ونه_View*, ثابت Upsert*, u8);
#إذا TREETRACE_ENABLED
  فراغ sqlite3_ونه_View_Delete(ثابت With*, ثابت SrcList*, ثابت بیان*,
                             ثابت بیان_List*,ثابت بیان*, ثابت Trigger*);
  فراغ sqlite3_ونه_View_ورننباسه(ثابت With*, ثابت SrcList*,
                             ثابت IdList*, ثابت Select*, ثابت بیان_List*,
                             صحيح, ثابت Upsert*, ثابت Trigger*);
  فراغ sqlite3_ونه_View_Update(ثابت With*, ثابت SrcList*, ثابت بیان_List*,
                             ثابت بیان*, صحيح, ثابت بیان_List*, ثابت بیان*,
                             ثابت Upsert*, ثابت Trigger*);
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_TRIGGER
  فراغ sqlite3_ونه_View_Trigger_ګام(ونه_View*, ثابت Trigger_ګام*, u8, u8);
  فراغ sqlite3_ونه_View_Trigger(ونه_View*, ثابت Trigger*, u8, u8);
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
  فراغ sqlite3_ونه_View_کړکۍ(ونه_View*, ثابت کړکۍ*, u8);
  فراغ sqlite3_ونه_View_Win_Func(ونه_View*, ثابت کړکۍ*, u8);
#د_که_پای
  فراغ sqlite3_Show_بیان(ثابت بیان*);
  فراغ sqlite3_Show_بیان_List(ثابت بیان_List*);
  فراغ sqlite3ShowIdList(ثابت IdList*);
  فراغ sqlite3ShowSrcList(ثابت SrcList*);
  فراغ sqlite3ShowSelect(ثابت Select*);
  فراغ sqlite3ShowWith(ثابت With*);
  فراغ sqlite3ShowUpsert(ثابت Upsert*);
#که_نه_تعریف_شوی SQLITE_OMIT_TRIGGER
  فراغ sqlite3_Show_Trigger_ګام(ثابت Trigger_ګام*);
  فراغ sqlite3_Show_Trigger_ګام_List(ثابت Trigger_ګام*);
  فراغ sqlite3ShowTrigger(ثابت Trigger*);
  فراغ sqlite3ShowTriggerList(ثابت Trigger*);
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
  فراغ sqlite3_Show_کړکۍ(ثابت کړکۍ*);
  فراغ sqlite3ShowWinFunc(ثابت کړکۍ*);
#د_که_پای
  فراغ sqlite3ShowBitvec(Bitvec*);
#د_که_پای

فراغ sqlite3_سېټ_String(حرف **, sqlite3*, ثابت حرف*);
فراغ sqlite3_پرمختګ_Check(Parse*);
فراغ sqlite3_Error_پیغام(Parse*, ثابت حرف*, ...);
صحيح sqlite3_Error_منزل_Parser(sqlite3*,صحيح);
فراغ sqlite3Dequote(حرف*);
فراغ sqlite3_Dequote_بیان(بیان*);
فراغ sqlite3DequoteToken(Token*);
فراغ sqlite3_Dequote_شمېره(Parse*, بیان*);
فراغ sqlite3TokenInit(Token*,حرف*);
صحيح sqlite3_Keyword_کوډ(ثابت بینښه حرف*, صحيح);
صحيح sqlite3_پرلپسې_Parser(Parse*, ثابت حرف*);
فراغ sqlite3_پای_Coding(Parse*);
صحيح sqlite3_Get_لنډمهاله_Reg(Parse*);
فراغ sqlite3_Release_لنډمهاله_Reg(Parse*,صحيح);
صحيح sqlite3_Get_لنډمهاله_Range(Parse*,صحيح);
فراغ sqlite3_Release_لنډمهاله_Range(Parse*,صحيح,صحيح);
فراغ sqlite3_Clear_لنډمهاله_Reg_Cache(Parse*);
فراغ sqlite3_Touch_ثبتوونکی(Parse*,صحيح);
#إذا defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
صحيح sqlite3_لومړی_شته_ثبتوونکی(Parse*,صحيح);
#د_که_پای
#که_تعریف_شوی SQLITE_DEBUG
صحيح sqlite3_No_Temps_دننه_Range(Parse*,صحيح,صحيح);
#د_که_پای
بیان *sqlite3_بیان_Alloc(sqlite3*,صحيح,ثابت Token*,صحيح);
بیان *sqlite3_بیان(sqlite3*,صحيح,ثابت حرف*);
بیان *sqlite3_بیان_Int32(sqlite3*,صحيح);
فراغ sqlite3_بیان_Attach_Subtrees(sqlite3*,بیان*,بیان*,بیان*);
بیان *sqlite3PExpr(Parse*, صحيح, بیان*, بیان*);
فراغ sqlite3_PExpr_جمع_کړه_Select(Parse*, بیان*, Select*);
بیان *sqlite3_بیان_And(Parse*,بیان*, بیان*);
بیان *sqlite3_بیان_Simplified_And_Or(بیان*);
بیان *sqlite3_بیان_Function(Parse*,بیان_List*, ثابت Token*, صحيح);
فراغ sqlite3_بیان_جمع_کړه_Function_ترتیب_By(Parse*,بیان*,بیان_List*);
فراغ sqlite3_بیان_ترتیب_By_Aggregate_Error(Parse*,بیان*);
فراغ sqlite3_بیان_Function_Usable(Parse*,ثابت بیان*,ثابت FuncDef*);
فراغ sqlite3_بیان_ټاکل_Var_شمېره(Parse*, بیان*, u32);
فراغ sqlite3_بیان_Delete(sqlite3*, بیان*);
فراغ sqlite3_بیان_Delete_Generic(sqlite3*,فراغ*);
صحيح sqlite3_بیان_Deferred_Delete(Parse*, بیان*);
فراغ sqlite3_بیان_Unmap_And_Delete(Parse*, بیان*);
بیان_List *sqlite3_بیان_List_Append(Parse*,بیان_List*,بیان*);
بیان_List *sqlite3_بیان_List_Append_ویکټور(Parse*,بیان_List*,IdList*,بیان*);
Select *sqlite3_بیان_List_منزل_ارزښتونه(Parse*, صحيح, بیان_List*);
فراغ sqlite3_بیان_List_سېټ_ترتیبول_ترتیب(بیان_List*,صحيح,صحيح);
فراغ sqlite3_بیان_List_سېټ_Name(Parse*,بیان_List*,ثابت Token*,صحيح);
فراغ sqlite3_بیان_List_سېټ_پراختیا(Parse*,بیان_List*,ثابت حرف*,ثابت حرف*);
فراغ sqlite3_بیان_List_Delete(sqlite3*, بیان_List*);
فراغ sqlite3_بیان_List_Delete_Generic(sqlite3*,فراغ*);
صحيح sqlite3_بیان_Can_Return_Subtype(Parse*,بیان*);
u32 sqlite3_بیان_List_Flags(ثابت بیان_List*);
صحيح sqlite3_شاخص_Has_Duplicate_ریښه_پاڼه(شاخص*);
صحيح sqlite3Init(sqlite3*, حرف**);
صحيح sqlite3InitCallback(فراغ*, صحيح, حرف**, حرف**);
صحيح sqlite3InitOne(sqlite3*, صحيح, حرف**, u32);
فراغ sqlite3Pragma(Parse*,Token*,Token*,Token*,صحيح);
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
Module *sqlite3_Pragma_Vtab_ثبتوونکی(sqlite3*,ثابت حرف *zName);
#د_که_پای
فراغ sqlite3_Reset_All_Schemas_د_Connection(sqlite3*);
فراغ sqlite3ResetOneSchema(sqlite3*,صحيح);
فراغ sqlite3CollapseDatabaseArray(sqlite3*);
فراغ sqlite3_Commit_دننی_Changes(sqlite3*);
فراغ sqlite3_ستنه_سېټ_بیان(Parse*,جدول*,ستنه*,بیان*);
بیان *sqlite3_ستنه_بیان(جدول*,ستنه*);
بیان *sqlite3_ستنه_بیان_Auth(جدول*,ستنه*,Parse*);
فراغ sqlite3_ستنه_سېټ_Coll(sqlite3*,ستنه*,ثابت حرف*zColl);
ثابت حرف *sqlite3_ستنه_Coll(ستنه*);
فراغ sqlite3_Delete_ستنه_نومونه(sqlite3*,جدول*);
فراغ sqlite3_Generate_ستنه_نومونه(Parse *pParse, Select *pSelect);
صحيح sqlite3_Columns_From_بیان_List(Parse*,بیان_List*,i16*,ستنه**);
فراغ sqlite3_Subquery_ستنه_Types(Parse*,جدول*,Select*,حرف);
جدول *sqlite3_پایله_سېټ_د_Select(Parse*,Select*,حرف);
فراغ sqlite3_Open_Schema_جدول(Parse *, صحيح);
شاخص *sqlite3_Primary_کلی_شاخص(جدول*);
صحيح sqlite3_جدول_ستنه_منزل_شاخص(شاخص*, صحيح);
#که_تعریف_شوی SQLITE_OMIT_GENERATED_COLUMNS
# define sqlite3_جدول_ستنه_منزل_Storage(T,X) (X)  /* No-op pass-through */
# define sqlite3_Storage_ستنه_منزل_جدول(T,X) (X)  /* No-op pass-through */
#وإلا
  i16 sqlite3_جدول_ستنه_منزل_Storage(جدول*, i16);
  i16 sqlite3_Storage_ستنه_منزل_جدول(جدول*, i16);
#د_که_پای
فراغ sqlite3_پیل_جدول(Parse*,Token*,Token*,صحيح,صحيح,صحيح,صحيح);
#إذا SQLITE_ENABLE_HIDDEN_COLUMNS
  فراغ sqlite3_ستنه_Properties_From_Name(جدول*, ستنه*);
#وإلا
# define sqlite3_ستنه_Properties_From_Name(T,C) /* no-op */
#د_که_پای
فراغ sqlite3_جمع_کړه_ستنه(Parse*,Token,Token);
فراغ sqlite3_جمع_کړه_Not_Null(Parse*, صحيح);
فراغ sqlite3_جمع_کړه_Primary_کلی(Parse*, بیان_List*, صحيح, صحيح, صحيح);
فراغ sqlite3_جمع_کړه_Check_Constraint(Parse*, بیان*, ثابت حرف*, ثابت حرف*);
فراغ sqlite3_جمع_کړه_Default_ارزښت(Parse*,بیان*,ثابت حرف*,ثابت حرف*);
فراغ sqlite3_جمع_کړه_Collate_Type(Parse*, Token*);
فراغ sqlite3_جمع_کړه_Generated(Parse*,بیان*,Token*);
فراغ sqlite3_پای_ټکی_جدول(Parse*,Token*,Token*,u32,Select*);
فراغ sqlite3_جمع_کړه_Returning(Parse*,بیان_List*);
صحيح sqlite3ParseUri(ثابت حرف*,ثابت حرف*,بینښه صحيح*,
                    sqlite3_vfs**,حرف**,حرف **);
#تعریف_کړه sqlite3CodecQueryParameters(A,B,C) 0
Btree *sqlite3_Db_Name_منزل_Btree(sqlite3*,ثابت حرف*);

#که_تعریف_شوی SQLITE_UNTESTABLE
# define sqlite3FaultSim(X) SQLITE_OK
#وإلا
  صحيح sqlite3FaultSim(صحيح);
#د_که_پای

Bitvec *sqlite3BitvecCreate(u32);
صحيح sqlite3BitvecTest(Bitvec*, u32);
صحيح sqlite3BitvecTestNotNull(Bitvec*, u32);
صحيح sqlite3_Bitvec_سېټ(Bitvec*, u32);
فراغ sqlite3BitvecClear(Bitvec*, u32, فراغ*);
فراغ sqlite3BitvecDestroy(Bitvec*);
u32 sqlite3_Bitvec_اندازه(Bitvec*);
#که_نه_تعریف_شوی SQLITE_UNTESTABLE
صحيح sqlite3BitvecBuiltinTest(صحيح,صحيح*);
#د_که_پای

لیکه_سېټ *sqlite3_لیکه_سېټ_Init(sqlite3*);
فراغ sqlite3_لیکه_سېټ_Delete(فراغ*);
فراغ sqlite3_لیکه_سېټ_Clear(فراغ*);
فراغ sqlite3_لیکه_سېټ_ورننباسه(لیکه_سېټ*, i64);
صحيح sqlite3_لیکه_سېټ_Test(لیکه_سېټ*, صحيح iBatch, i64);
صحيح sqlite3_لیکه_سېټ_بل(لیکه_سېټ*, i64*);

فراغ sqlite3CreateView(Parse*,Token*,Token*,Token*,بیان_List*,Select*,صحيح,صحيح);

#إذا !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
  صحيح sqlite3_View_Get_ستنه_نومونه(Parse*,جدول*);
#وإلا
# define sqlite3_View_Get_ستنه_نومونه(A,B) 0
#د_که_پای

#إذا SQLITE_MAX_ATTACHED>30
  صحيح sqlite3_Db_ماسک_All_Zero(y_Db_ماسک);
#د_که_پای
فراغ sqlite3_Drop_جدول(Parse*, SrcList*, صحيح, صحيح);
فراغ sqlite3_کوډ_Drop_جدول(Parse*, جدول*, صحيح, صحيح);
فراغ sqlite3_Delete_جدول(sqlite3*, جدول*);
فراغ sqlite3_Delete_جدول_Generic(sqlite3*, فراغ*);
فراغ sqlite3_Free_شاخص(sqlite3*, شاخص*);
#که_نه_تعریف_شوی SQLITE_OMIT_AUTOINCREMENT
  فراغ sqlite3AutoincrementBegin(Parse *pParse);
  فراغ sqlite3_Autoincrement_پای_ټکی(Parse *pParse);
#وإلا
# define sqlite3AutoincrementBegin(X)
# define sqlite3_Autoincrement_پای_ټکی(X)
#د_که_پای
فراغ sqlite3_ورننباسه(Parse*, SrcList*, Select*, IdList*, صحيح, Upsert*);
#که_نه_تعریف_شوی SQLITE_OMIT_GENERATED_COLUMNS
  فراغ sqlite3ComputeGeneratedColumns(Parse*, صحيح, جدول*);
#د_که_پای
فراغ *sqlite3ArrayAllocate(sqlite3*,فراغ*,صحيح,صحيح*,صحيح*);
IdList *sqlite3IdListAppend(Parse*, IdList*, Token*);
صحيح sqlite3_Id_List_شاخص(IdList*,ثابت حرف*);
SrcList *sqlite3SrcListEnlarge(Parse*, SrcList*, صحيح, صحيح);
SrcList *sqlite3SrcListAppendList(Parse *pParse, SrcList *p1, SrcList *p2);
SrcList *sqlite3SrcListAppend(Parse*, SrcList*, Token*, Token*);
فراغ sqlite3SubqueryDelete(sqlite3*,Subquery*);
Select *sqlite3SubqueryDetach(sqlite3*,Src_توکی*);
صحيح sqlite3_Src_توکی_Attach_Subquery(Parse*, Src_توکی*, Select*, صحيح);
SrcList *sqlite3_Src_List_Append_From_غړی(Parse*, SrcList*, Token*, Token*,
                                      Token*, Select*, OnOrUsing*);
فراغ sqlite3_Src_List_شاخصي_By(Parse *, SrcList *, Token *);
فراغ sqlite3_Src_List_Func_دلیلونه(Parse*, SrcList*, بیان_List*);
صحيح sqlite3_شاخصي_By_Lookup(Parse *, Src_توکی *);
فراغ sqlite3_Src_List_ښويدنه_Join_Type(Parse*,SrcList*);
فراغ sqlite3_Src_List_ټاکل_Cursors(Parse*, SrcList*);
فراغ sqlite3IdListDelete(sqlite3*, IdList*);
فراغ sqlite3ClearOnOrUsing(sqlite3*, OnOrUsing*);
فراغ sqlite3SrcListDelete(sqlite3*, SrcList*);
شاخص *sqlite3_Allocate_شاخص_Object(sqlite3*,صحيح,صحيح,حرف**);
صحيح sqlite3_شاخص_Bloomable(ثابت شاخص*,صحيح);
فراغ sqlite3_Create_شاخص(Parse*,Token*,Token*,SrcList*,بیان_List*,صحيح,Token*,
                          بیان*, صحيح, صحيح, u8);
فراغ sqlite3_Drop_شاخص(Parse*, SrcList*, صحيح);
صحيح sqlite3Select(Parse*, Select*, SelectDest*);
Select *sqlite3SelectNew(Parse*,بیان_List*,SrcList*,بیان*,بیان_List*,
                         بیان*,بیان_List*,u32,بیان*);
فراغ sqlite3SelectDelete(sqlite3*, Select*);
فراغ sqlite3SelectDeleteGeneric(sqlite3*,فراغ*);
فراغ sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect);
جدول *sqlite3SrcListLookup(Parse*, SrcList*);
صحيح sqlite3IsReadOnly(Parse*, جدول*, Trigger*);
فراغ sqlite3_Open_جدول(Parse*, صحيح iCur, صحيح iDb, جدول*, صحيح);
#إذا defined(SQLITE_ENABLE_UPDATE_DELETE_حد) && !defined(SQLITE_OMIT_SUBQUERY)
بیان *sqlite3_حد_Where(Parse*,SrcList*,بیان*,بیان_List*,بیان*,حرف*);
#د_که_پای
فراغ sqlite3_کوډ_Change_شمېر(Vdbe*,صحيح,ثابت حرف*);
فراغ sqlite3DeleteFrom(Parse*, SrcList*, بیان*, بیان_List*, بیان*);
فراغ sqlite3Update(Parse*, SrcList*, بیان_List*,بیان*,صحيح,بیان_List*,بیان*,
                   Upsert*);
WhereInfo *sqlite3WhereBegin(Parse*,SrcList*,بیان*,بیان_List*,
                             بیان_List*,Select*,u16,صحيح);
فراغ sqlite3_Where_پای_ټکی(WhereInfo*);
LogEst sqlite3_Where_Output_لیکه_شمېر(WhereInfo*);
صحيح sqlite3WhereIsDistinct(WhereInfo*);
صحيح sqlite3WhereIsOrdered(WhereInfo*);
صحيح sqlite3_Where_ترتیب_By_حد_Opt_Label(WhereInfo*);
فراغ sqlite3_Where_Min_Max_Opt_Early_بهر(Vdbe*,WhereInfo*);
صحيح sqlite3WhereIsSorted(WhereInfo*);
صحيح sqlite3WhereContinueLabel(WhereInfo*);
صحيح sqlite3WhereBreakLabel(WhereInfo*);
صحيح sqlite3WhereOkOnePass(WhereInfo*, صحيح*);
#تعریف_کړه ONEPASS_OFF      0        /* Use of ONEPASS not allowed */
#تعریف_کړه ONEPASS_SINGLE   1        /* ONEPASS valid for a single row update */
#تعریف_کړه ONEPASS_MULTI    2        /* ONEPASS is valid for multiple rows */
صحيح sqlite3_Where_کاروي_Deferred_Seek(WhereInfo*);
فراغ sqlite3_بیان_کوډ_بارول_شاخص_ستنه(Parse*, شاخص*, صحيح, صحيح, صحيح);
صحيح sqlite3_بیان_کوډ_Get_ستنه(Parse*, جدول*, صحيح, صحيح, صحيح, u8);
فراغ sqlite3_بیان_کوډ_Get_ستنه_د_جدول(Vdbe*, جدول*, صحيح, صحيح, صحيح);
فراغ sqlite3_بیان_کوډ_Move(Parse*, صحيح, صحيح, صحيح);
فراغ sqlite3_بیان_منزل_ثبتوونکی(بیان *p_بیان, صحيح iReg);
فراغ sqlite3_بیان_کوډ(Parse*, بیان*, صحيح);
#که_نه_تعریف_شوی SQLITE_OMIT_GENERATED_COLUMNS
فراغ sqlite3_بیان_کوډ_Generated_ستنه(Parse*, جدول*, ستنه*, صحيح);
#د_که_پای
فراغ sqlite3_بیان_کوډ_Copy(Parse*, بیان*, صحيح);
فراغ sqlite3_بیان_کوډ_Factorable(Parse*, بیان*, صحيح);
صحيح sqlite3_بیان_کوډ_پرلپسې_Just_Once(Parse*, بیان*, صحيح);
فراغ sqlite3_بیان_Null_ثبتوونکی_Range(Parse*, صحيح, صحيح);
صحيح sqlite3_بیان_کوډ_لنډمهاله(Parse*, بیان*, صحيح*);
صحيح sqlite3_بیان_کوډ_موخه(Parse*, بیان*, صحيح);
صحيح sqlite3_بیان_کوډ_بیان_List(Parse*, بیان_List*, صحيح, صحيح, u8);
#تعریف_کړه SQLITE_ECEL_DUP      0x01  /* Deep, not shallow copies */
#تعریف_کړه SQLITE_ECEL_ضریب   0x02  /* Factor out constant terms */
#تعریف_کړه SQLITE_ECEL_REF      0x04  /* Use ExprList.u.x.iOrderByCol */
#تعریف_کړه SQLITE_ECEL_OMITREF  0x08  /* Omit if ExprList.u.x.iOrderByCol */
فراغ sqlite3_بیان_If_True(Parse*, بیان*, صحيح, صحيح);
فراغ sqlite3_بیان_If_False(Parse*, بیان*, صحيح, صحيح);
فراغ sqlite3_بیان_If_False_Dup(Parse*, بیان*, صحيح, صحيح);
جدول *sqlite3_پیدا_کړه_جدول(sqlite3*,ثابت حرف*, ثابت حرف*);
#تعریف_کړه LOCATE_VIEW    0x01
#تعریف_کړه LOCATE_NOERR   0x02
جدول *sqlite3_Locate_جدول(Parse*,u32 flags,ثابت حرف*, ثابت حرف*);
ثابت حرف *sqlite3_Preferred_جدول_Name(ثابت حرف*);
جدول *sqlite3_Locate_جدول_توکی(Parse*,u32 flags,Src_توکی *);
شاخص *sqlite3_پیدا_کړه_شاخص(sqlite3*,ثابت حرف*, ثابت حرف*);
فراغ sqlite3_Unlink_And_Delete_جدول(sqlite3*,صحيح,ثابت حرف*);
فراغ sqlite3_Unlink_And_Delete_شاخص(sqlite3*,صحيح,ثابت حرف*);
فراغ sqlite3Vacuum(Parse*,Token*,بیان*);
صحيح sqlite3_پرلپسې_Vacuum(حرف**, sqlite3*, صحيح, sqlite3_ارزښت*);
حرف *sqlite3NameFromToken(sqlite3*, ثابت Token*);
صحيح sqlite3_بیان_Compare(ثابت Parse*,ثابت بیان*,ثابت بیان*, صحيح);
صحيح sqlite3_بیان_Compare_Skip(بیان*,بیان*,صحيح);
صحيح sqlite3_بیان_List_Compare(ثابت بیان_List*,ثابت بیان_List*, صحيح);
صحيح sqlite3_بیان_پایله_ورکوي_بیان(ثابت Parse*,ثابت بیان*,ثابت بیان*, صحيح);
صحيح sqlite3_بیان_پایله_ورکوي_Non_Null_لیکه(بیان*,صحيح,صحيح);
فراغ sqlite3AggInfoPersistWalkerInit(Walker*,Parse*);
فراغ sqlite3_بیان_Analyze_Aggregates(NameContext*, بیان*);
فراغ sqlite3_بیان_Analyze_Agg_List(NameContext*,بیان_List*);
صحيح sqlite3_بیان_Covered_By_شاخص(بیان*, صحيح iCur, شاخص *pIdx);
صحيح sqlite3ReferencesSrcList(Parse*, بیان*, SrcList*);
Vdbe *sqlite3GetVdbe(Parse*);
#که_نه_تعریف_شوی SQLITE_UNTESTABLE
فراغ sqlite3PrngSaveState(فراغ);
فراغ sqlite3PrngRestoreState(فراغ);
#د_که_پای
فراغ sqlite3RollbackAll(sqlite3*,صحيح);
فراغ sqlite3_کوډ_Verify_Schema(Parse*, صحيح);
فراغ sqlite3_کوډ_Verify_Named_Schema(Parse*, ثابت حرف *zDb);
فراغ sqlite3BeginTransaction(Parse*, صحيح);
فراغ sqlite3_پای_ټکی_Transaction(Parse*,صحيح);
فراغ sqlite3Savepoint(Parse*, صحيح, Token*);
فراغ sqlite3CloseSavepoints(sqlite3 *);
فراغ sqlite3LeaveMutexAndCloseZombie(sqlite3*);
u32 sqlite3IsTrueOrFalse(ثابت حرف*);
صحيح sqlite3_بیان_Id_منزل_True_False(بیان*);
صحيح sqlite3_بیان_Truth_ارزښت(ثابت بیان*);
صحيح sqlite3_بیان_Is_Constant(Parse*,بیان*);
صحيح sqlite3_بیان_Is_Constant_Or_Function(بیان*, u8);
صحيح sqlite3_بیان_Is_Constant_Or_ډله_By(Parse*, بیان*, بیان_List*);
صحيح sqlite3_بیان_Is_جدول_Constant(بیان*,صحيح,صحيح);
صحيح sqlite3_بیان_Is_Single_جدول_Constraint(بیان*,ثابت SrcList*,صحيح,صحيح);
صحيح sqlite3_بیان_List_Is_Constant(Parse *pParse, بیان_List *pList, صحيح bNoIs);
صحيح sqlite3_بیان_لري_Subquery(بیان*);
صحيح sqlite3_بیان_Is_Integer(ثابت بیان*, صحيح*, Parse*, صحيح);
صحيح sqlite3_بیان_Can_Be_Null(ثابت بیان*);
صحيح sqlite3_بیان_Needs_No_Affinity_Change(ثابت بیان*, حرف);
صحيح sqlite3_بیان_Is_Like_Operator(ثابت بیان*);
صحيح sqlite3IsRowid(ثابت حرف*);
ثابت حرف *sqlite3RowidAlias(جدول *pTab);
فراغ sqlite3_Generate_لیکه_Delete(
    Parse*,جدول*,Trigger*,صحيح,صحيح,صحيح,i16,u8,u8,u8,صحيح);
فراغ sqlite3_Generate_لیکه_شاخص_Delete(Parse*, جدول*, صحيح, صحيح, صحيح*, صحيح);
صحيح sqlite3_Generate_شاخص_کلی(Parse*, شاخص*, صحيح, صحيح, صحيح, صحيح*,شاخص*,صحيح);
فراغ sqlite3ResolvePartIdxLabel(Parse*,صحيح);
صحيح sqlite3_بیان_References_Updated_ستنه(بیان*,صحيح*,صحيح);
فراغ sqlite3GenerateConstraintChecks(Parse*,جدول*,صحيح*,صحيح,صحيح,صحيح,صحيح,
                                     u8,u8,صحيح,صحيح*,صحيح*,Upsert*);
#که_تعریف_شوی SQLITE_ENABLE_NULL_TRIM
  فراغ sqlite3_سېټ_Make_ریکارډ_P5(Vdbe*,جدول*);
#وإلا
# define sqlite3_سېټ_Make_ریکارډ_P5(A,B)
#د_که_پای
فراغ sqlite3CompleteInsertion(Parse*,جدول*,صحيح,صحيح,صحيح,صحيح*,صحيح,صحيح,صحيح);
صحيح sqlite3_Open_جدول_And_Indices(Parse*, جدول*, صحيح, u8, صحيح, u8*, صحيح*, صحيح*);
فراغ sqlite3BeginWriteOperation(Parse*, صحيح, صحيح);
فراغ sqlite3MultiWrite(Parse*);
فراغ sqlite3MayAbort(Parse*);
فراغ sqlite3HaltConstraint(Parse*, صحيح, صحيح, حرف*, i8, u8);
فراغ sqlite3UniqueConstraint(Parse*, صحيح, شاخص*);
فراغ sqlite3RowidConstraint(Parse*, صحيح, جدول*);
بیان *sqlite3_بیان_Dup(sqlite3*,ثابت بیان*,صحيح);
بیان_List *sqlite3_بیان_List_Dup(sqlite3*,ثابت بیان_List*,صحيح);
SrcList *sqlite3SrcListDup(sqlite3*,ثابت SrcList*,صحيح);
IdList *sqlite3IdListDup(sqlite3*,ثابت IdList*);
Select *sqlite3SelectDup(sqlite3*,ثابت Select*,صحيح);
FuncDef *sqlite3_Function_پلټنه(صحيح,ثابت حرف*);
فراغ sqlite3_ورننباسه_Builtin_Funcs(FuncDef*,صحيح);
FuncDef *sqlite3_پیدا_کړه_Function(sqlite3*,ثابت حرف*,صحيح,u8,u8);
فراغ sqlite3_Quote_ارزښت(StrAccum*,sqlite3_ارزښت*,صحيح);
صحيح sqlite3AppendOneUtf8Character(حرف*, u32);
فراغ sqlite3_ثبتوونکی_Builtin_Functions(فراغ);
فراغ sqlite3_ثبتوونکی_Date_وخت_Functions(فراغ);
فراغ sqlite3_ثبتوونکی_Json_Functions(فراغ);
فراغ sqlite3_ثبتوونکی_فی_Connection_Builtin_Functions(sqlite3*);
#إذا !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
  Module *sqlite3_Json_Vtab_ثبتوونکی(sqlite3*,ثابت حرف*);
#د_که_پای
صحيح sqlite3SafetyCheckOk(sqlite3*);
صحيح sqlite3SafetyCheckSickOrOk(sqlite3*);
فراغ sqlite3ChangeCookie(Parse*, صحيح);
With *sqlite3WithDup(sqlite3 *db, With *p);

#إذا !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
  Module *sqlite3_Carray_ثبتوونکی(sqlite3*);
#د_که_پای

#إذا !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
فراغ sqlite3MaterializeView(Parse*, جدول*, بیان*, بیان_List*,بیان*,صحيح);
#د_که_پای

#که_نه_تعریف_شوی SQLITE_OMIT_TRIGGER
  فراغ sqlite3BeginTrigger(Parse*, Token*,Token*,صحيح,صحيح,IdList*,SrcList*,
                           بیان*,صحيح, صحيح);
  فراغ sqlite3_پای_Trigger(Parse*, Trigger_ګام*, Token*);
  فراغ sqlite3DropTrigger(Parse*, SrcList*, صحيح);
  فراغ sqlite3DropTriggerPtr(Parse*, Trigger*);
  Trigger *sqlite3TriggersExist(Parse *, جدول*, صحيح, بیان_List*, صحيح *p_ماسک);
  Trigger *sqlite3TriggerList(Parse *, جدول *);
  فراغ sqlite3_کوډ_لیکه_Trigger(Parse*, Trigger *, صحيح, بیان_List*, صحيح, جدول *,
                            صحيح, صحيح, صحيح);
  فراغ sqlite3_کوډ_لیکه_Trigger_مستقیم(Parse *, Trigger *, جدول *, صحيح, صحيح, صحيح);
  فراغ sqliteViewTriggers(Parse*, جدول*, بیان*, صحيح, بیان_List*);
  فراغ sqlite3_Delete_Trigger_ګام(sqlite3*, Trigger_ګام*);
  Trigger_ګام *sqlite3_Trigger_Select_ګام(sqlite3*,Select*,
                                        ثابت حرف*,ثابت حرف*);
  Trigger_ګام *sqlite3_Trigger_ورننباسه_ګام(Parse*,SrcList*, IdList*,
                                        Select*,u8,Upsert*,
                                        ثابت حرف*,ثابت حرف*);
  Trigger_ګام *sqlite3_Trigger_Update_ګام(Parse*,SrcList*,SrcList*,بیان_List*,
                                        بیان*, u8, ثابت حرف*,ثابت حرف*);
  Trigger_ګام *sqlite3_Trigger_Delete_ګام(Parse*,SrcList*, بیان*,
                                        ثابت حرف*,ثابت حرف*);
  فراغ sqlite3DeleteTrigger(sqlite3*, Trigger*);
  فراغ sqlite3UnlinkAndDeleteTrigger(sqlite3*,صحيح,ثابت حرف*);
  u32 sqlite3TriggerColmask(Parse*,Trigger*,بیان_List*,صحيح,صحيح,جدول*,صحيح);
# define sqlite3ParseToplevel(p) ((p)->pToplevel ? (p)->pToplevel : (p))
# define sqlite3IsToplevel(p) ((p)->pToplevel==0)
#وإلا
# define sqlite3TriggersExist(B,C,D,E,F) 0
# define sqlite3DeleteTrigger(A,B)
# define sqlite3DropTriggerPtr(A,B)
# define sqlite3UnlinkAndDeleteTrigger(A,B,C)
# define sqlite3_کوډ_لیکه_Trigger(A,B,C,D,E,F,G,H,I)
# define sqlite3_کوډ_لیکه_Trigger_مستقیم(A,B,C,D,E,F)
# define sqlite3TriggerList(X, Y) 0
# define sqlite3ParseToplevel(p) p
# define sqlite3IsToplevel(p) 1
# define sqlite3TriggerColmask(A,B,C,D,E,F,G) 0
#د_که_پای

صحيح sqlite3JoinType(Parse*, Token*, Token*, Token*);
صحيح sqlite3_ستنه_شاخص(جدول *pTab, ثابت حرف *z_ستون);
فراغ sqlite3_Src_توکی_ستنه_کارول_شوی(Src_توکی*,صحيح);
فراغ sqlite3_سېټ_Join_بیان(بیان*,صحيح,u32);
فراغ sqlite3_Create_Foreign_کلی(Parse*, بیان_List*, Token*, بیان_List*, صحيح);
فراغ sqlite3_Defer_Foreign_کلی(Parse*, صحيح);
#که_نه_تعریف_شوی SQLITE_OMIT_AUTHORIZATION
  فراغ sqlite3AuthRead(Parse*,بیان*,Schema*,SrcList*);
  صحيح sqlite3AuthCheck(Parse*,صحيح, ثابت حرف*, ثابت حرف*, ثابت حرف*);
  فراغ sqlite3_Auth_Context_ټېل(Parse*, AuthContext*, ثابت حرف*);
  فراغ sqlite3_Auth_Context_راوباسه(AuthContext*);
  صحيح sqlite3_Auth_Read_ستون(Parse*, ثابت حرف *, ثابت حرف *, صحيح);
#وإلا
# define sqlite3AuthRead(a,b,c,d)
# define sqlite3AuthCheck(a,b,c,d,e)    SQLITE_OK
# define sqlite3_Auth_Context_ټېل(a,b,c)
# define sqlite3_Auth_Context_راوباسه(a)  ((فراغ)(a))
#د_که_پای
صحيح sqlite3DbIsNamed(sqlite3 *db, صحيح iDb, ثابت حرف *zName);
فراغ sqlite3Attach(Parse*, بیان*, بیان*, بیان*);
فراغ sqlite3Detach(Parse*, بیان*);
فراغ sqlite3FixInit(DbFixer*, Parse*, صحيح, ثابت حرف*, ثابت Token*);
صحيح sqlite3FixSrcList(DbFixer*, SrcList*);
صحيح sqlite3FixSelect(DbFixer*, Select*);
صحيح sqlite3_Fix_بیان(DbFixer*, بیان*);
صحيح sqlite3_Fix_Trigger_ګام(DbFixer*, Trigger_ګام*);

صحيح sqlite3RealSameAsInt(مزدوج,sqlite3_int64);
i64 sqlite3_Real_منزل_I64(مزدوج);
صحيح sqlite3_Int64_منزل_متن(i64,حرف*);
صحيح sqlite3AtoF(ثابت حرف *z, مزدوج*);
صحيح sqlite3GetInt32(ثابت حرف *, صحيح*);
صحيح sqlite3GetUInt32(ثابت حرف*, u32*);
صحيح sqlite3Atoi(ثابت حرف*);
#که_نه_تعریف_شوی SQLITE_OMIT_UTF16
صحيح sqlite3Utf16ByteLen(ثابت فراغ *p_معلومات, صحيح nByte, صحيح nChar);
#د_که_پای
صحيح sqlite3Utf8CharLen(ثابت حرف *p_معلومات, صحيح nByte);
u32 sqlite3Utf8Read(ثابت u8**);
صحيح sqlite3Utf8ReadLimited(ثابت u8*, صحيح, u32*);
LogEst sqlite3LogEst(u64);
LogEst sqlite3_Log_Est_جمع_کړه(LogEst,LogEst);
LogEst sqlite3LogEstFromDouble(مزدوج);
u64 sqlite3_Log_Est_منزل_Int(LogEst);
VList *sqlite3_VList_جمع_کړه(sqlite3*,VList*,ثابت حرف*,صحيح,صحيح);
ثابت حرف *sqlite3_VList_عدد_منزل_Name(VList*,صحيح);
صحيح sqlite3_VList_Name_منزل_عدد(VList*,ثابت حرف*,صحيح);

/*
** Routines to read and write variable-length integers.  These used to
** be defined locally, but now we use the varint routines in the util.c
** file.
*/
صحيح sqlite3PutVarint(بینښه حرف*, u64);
u8 sqlite3GetVarint(ثابت بینښه حرف *, u64 *);
i64 sqlite3_Varint_ارزښت(ثابت بینښه حرف*);
u8 sqlite3GetVarint32(ثابت بینښه حرف *, u32 *);
صحيح sqlite3VarintLen(u64 v);

/*
** The common case is for a varint to be a single byte.  They following
** macros handle the common case without a procedure call, but then call
** the procedure for larger varints.
*/
#تعریف_کړه getVarint32(A,B)  \
  (u8)((*(A)<(u8)0x80)?((B)=(u32)*(A)),1:sqlite3GetVarint32((A),(u32 *)&(B)))
#تعریف_کړه getVarint32NR(A,B) \
  B=(u32)*(A);إذا(B>=0x80)sqlite3GetVarint32((A),(u32*)&(B))
#تعریف_کړه putVarint32(A,B)  \
  (u8)(((u32)(B)<(u32)0x80)?(*(A)=(بینښه حرف)(B)),1:\
  sqlite3PutVarint((A),(B)))

ثابت حرف *sqlite3_شاخص_Affinity_Str(sqlite3*, شاخص*);
حرف *sqlite3_جدول_Affinity_Str(sqlite3*,ثابت جدول*);
فراغ sqlite3_جدول_Affinity(Vdbe*, جدول*, صحيح);
حرف sqlite3CompareAffinity(ثابت بیان *p_بیان, حرف aff2);
صحيح sqlite3_شاخص_Affinity_Ok(ثابت بیان *p_بیان, حرف idx_affinity);
حرف sqlite3_جدول_ستنه_Affinity(ثابت جدول*,صحيح);
حرف sqlite3_بیان_Affinity(ثابت بیان *p_بیان);
صحيح sqlite3_بیان_معلومات_Type(ثابت بیان *p_بیان);
صحيح sqlite3Atoi64(ثابت حرف*, i64*, صحيح, u8);
صحيح sqlite3_Dec_Or_Hex_منزل_I64(ثابت حرف*, i64*);
فراغ sqlite3_Error_With_پیغام(sqlite3*, صحيح, ثابت حرف*,...);
فراغ sqlite3Error(sqlite3*,صحيح);
فراغ sqlite3ErrorClear(sqlite3*);
فراغ sqlite3SystemError(sqlite3*,صحيح);
#إذا !defined(SQLITE_OMIT_BLOB_LITERAL)
فراغ *sqlite3_Hex_منزل_Blob(sqlite3*, ثابت حرف *z, صحيح n);
#د_که_پای
u8 sqlite3_Hex_منزل_Int(صحيح h);
صحيح sqlite3TwoPartName(Parse *, Token *, Token *, Token **);

#إذا defined(SQLITE_NEED_ERR_NAME)
ثابت حرف *sqlite3ErrName(صحيح);
#د_که_پای

#که_نه_تعریف_شوی SQLITE_OMIT_DESERIALIZE
صحيح sqlite3MemdbInit(فراغ);
صحيح sqlite3IsMemdb(ثابت sqlite3_vfs*);
#وإلا
# define sqlite3IsMemdb(X) 0
#د_که_پای

ثابت حرف *sqlite3ErrStr(صحيح);
صحيح sqlite3ReadSchema(Parse *pParse);
CollSeq *sqlite3_پیدا_کړه_Coll_Seq(sqlite3*,u8 enc, ثابت حرف*,صحيح);
صحيح sqlite3_دوګونی_Compare(فراغ*,صحيح,ثابت فراغ*,صحيح,ثابت فراغ*);
#إذا SQLITE_کاروي_INLINE
ساكن SQLITE_INLINE صحيح sqlite3_Is_دوګونی(ثابت CollSeq *p){
  أكد( p==0 || p->xCmp!=sqlite3_دوګونی_Compare
       || strcmp(p->zName,"BINARY")==0 );
  أرجع p==0 || p->xCmp==sqlite3_دوګونی_Compare;
}
#وإلا
صحيح sqlite3_Is_دوګونی(ثابت CollSeq*);
#د_که_پای
CollSeq *sqlite3LocateCollSeq(Parse *pParse, ثابت حرف*zName);
فراغ sqlite3_سېټ_متن_Encoding(sqlite3 *db, u8);
CollSeq *sqlite3_بیان_Coll_Seq(Parse *pParse, ثابت بیان *p_بیان);
CollSeq *sqlite3_بیان_NNColl_Seq(Parse *pParse, ثابت بیان *p_بیان);
صحيح sqlite3_بیان_Coll_Seq_سمون(Parse*,ثابت بیان*,ثابت بیان*);
بیان *sqlite3_بیان_جمع_کړه_Collate_Token(Parse *pParse, بیان*, ثابت Token*, صحيح);
بیان *sqlite3_بیان_جمع_کړه_Collate_String(Parse*,بیان*,ثابت حرف*);
بیان *sqlite3_بیان_Skip_Collate(بیان*);
بیان *sqlite3_بیان_Skip_Collate_And_Likely(بیان*);
صحيح sqlite3CheckCollSeq(Parse *, CollSeq *);
صحيح sqlite3WritableSchema(sqlite3*);
صحيح sqlite3CheckObjectName(Parse*, ثابت حرف*,ثابت حرف*,ثابت حرف*);
فراغ sqlite3_Vdbe_سېټ_Changes(sqlite3 *, i64);
صحيح sqlite3_جمع_کړه_Int64(i64*,i64);
صحيح sqlite3SubInt64(i64*,i64);
صحيح sqlite3MulInt64(i64*,i64);
صحيح sqlite3AbsInt32(صحيح);
#که_تعریف_شوی SQLITE_ENABLE_8_3_نومونه
فراغ sqlite3FileSuffix3(ثابت حرف*, حرف*);
#وإلا
# define sqlite3FileSuffix3(X,Y)
#د_که_پای
u8 sqlite3GetBoolean(ثابت حرف *z,u8);

ثابت فراغ *sqlite3_ارزښت_متن_2(sqlite3_ارزښت*, u8);
صحيح sqlite3_ارزښت_Is_د_Class(ثابت sqlite3_ارزښت*, فراغ(*)(فراغ*));
صحيح sqlite3_ارزښت_بایټونه_2(sqlite3_ارزښت*, u8);
فراغ sqlite3_ارزښت_سېټ_Str(sqlite3_ارزښت*, صحيح, ثابت فراغ *,u8,
                        فراغ(*)(فراغ*));
فراغ sqlite3_ارزښت_سېټ_Null(sqlite3_ارزښت*);
فراغ sqlite3_ارزښت_Free(sqlite3_ارزښت*);
#که_نه_تعریف_شوی SQLITE_UNTESTABLE
فراغ sqlite3_پایله_Int_Real(sqlite3_context*);
#د_که_پای
sqlite3_ارزښت *sqlite3_ارزښت_New(sqlite3 *);
#که_نه_تعریف_شوی SQLITE_OMIT_UTF16
حرف *sqlite3Utf16to8(sqlite3 *, ثابت فراغ*, صحيح, u8);
#د_که_پای
صحيح sqlite3_ارزښت_From_بیان(sqlite3 *, ثابت بیان *, u8, u8, sqlite3_ارزښت **);
فراغ sqlite3_ارزښت_Apply_Affinity(sqlite3_ارزښت *, u8, u8);
#که_نه_تعریف_شوی SQLITE_AMALGAMATION
خارجي ثابت بینښه حرف sqlite3OpcodeProperty[];
خارجي ثابت حرف sqlite3_Str_دوګونی[];
خارجي ثابت بینښه حرف sqlite3StdTypeLen[];
خارجي ثابت حرف sqlite3StdTypeAffinity[];
خارجي ثابت حرف *sqlite3StdType[];
خارجي ثابت بینښه حرف sqlite3_پاسنی_منزل_ښکتنی[];
خارجي ثابت بینښه حرف *sqlite3aLTb;
خارجي ثابت بینښه حرف *sqlite3aEQb;
خارجي ثابت بینښه حرف *sqlite3aGTb;
خارجي ثابت بینښه حرف sqlite3CtypeMap[];
خارجي SQLITE_WSD بنية Sqlite3Config sqlite3Config;
خارجي FuncDefHash sqlite3BuiltinFunctions;
خارجي ثابت sqlite3_str sqlite3OomStr;
#که_نه_تعریف_شوی SQLITE_OMIT_WSD
خارجي صحيح sqlite3PendingByte;
#د_که_پای
#د_که_پای /* SQLITE_AMALGAMATION */
#که_تعریف_شوی VDBE_PROFILE
خارجي sqlite3_uint64 sqlite3NProfileCnt;
#د_که_پای
فراغ sqlite3_ریښه_پاڼه_Moved(sqlite3*, صحيح, Pgno, Pgno);
فراغ sqlite3Reindex(Parse*, Token*, Token*);
فراغ sqlite3AlterFunctions(فراغ);
فراغ sqlite3_Alter_Rename_جدول(Parse*, SrcList*, Token*);
فراغ sqlite3_Alter_Rename_ستنه(Parse*, SrcList*, Token*, Token*);
فراغ sqlite3AlterDropConstraint(Parse*,SrcList*,Token*,Token*);
فراغ sqlite3_Alter_جمع_کړه_Constraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *p_لومړی,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  ثابت حرف *z_بیان,       /* Text of CHECK expression */
  صحيح n_بیان,               /* Size of pExpr in bytes */
  بیان *p_بیان              /* The parsed CHECK expression */
);
فراغ sqlite3_Alter_سېټ_Not_Null(Parse*, SrcList*, Token*, Token*);
i64 sqlite3GetToken(ثابت بینښه حرف *, صحيح *);
فراغ sqlite3NestedParse(Parse*, ثابت حرف*, ...);
فراغ sqlite3ExpirePreparedStatements(sqlite3*, صحيح);
فراغ sqlite3_کوډ_Rhs_د_دننه(Parse*, بیان*, صحيح, صحيح);
صحيح sqlite3_کوډ_Subselect(Parse*, بیان*);
فراغ sqlite3SelectPrep(Parse*, Select*, NameContext*);
صحيح sqlite3ExpandSubquery(Parse*, Src_توکی*);
فراغ sqlite3_Select_Wrong_عدد_Terms_Error(Parse *pParse, Select *p);
صحيح sqlite3_سمون_EName(
  ثابت بنية بیان_List_توکی*,
  ثابت حرف*,
  ثابت حرف*,
  ثابت حرف*,
  صحيح*
);
Bitmask sqlite3_بیان_ستون_کارول_شوی(بیان*);
u8 sqlite3StrIHash(ثابت حرف*);
صحيح sqlite3_Resolve_بیان_نومونه(NameContext*, بیان*);
صحيح sqlite3_Resolve_بیان_List_نومونه(NameContext*, بیان_List*);
فراغ sqlite3_Resolve_Select_نومونه(Parse*, Select*, NameContext*);
صحيح sqlite3ResolveSelfReference(Parse*,جدول*,صحيح,بیان*,بیان_List*);
صحيح sqlite3_Resolve_ترتیب_ډله_By(Parse*, Select*, بیان_List*, ثابت حرف*);
فراغ sqlite3_ستنه_Default(Vdbe *, جدول *, صحيح, صحيح);
فراغ sqlite3_Alter_پای_جمع_کړه_ستنه(Parse *, Token *);
فراغ sqlite3_Alter_Begin_جمع_کړه_ستنه(Parse *, SrcList *);
فراغ sqlite3_Alter_Drop_ستنه(Parse*, SrcList*, ثابت Token*);
ثابت فراغ *sqlite3RenameTokenMap(Parse*, ثابت فراغ*, ثابت Token*);
فراغ sqlite3RenameTokenRemap(Parse*, ثابت فراغ *p_منزل, ثابت فراغ *pFrom);
فراغ sqlite3_Rename_بیان_Unmap(Parse*, بیان*);
فراغ sqlite3RenameExprlistUnmap(Parse*, بیان_List*);
CollSeq *sqlite3GetCollSeq(Parse*, u8, CollSeq *, ثابت حرف*);
حرف sqlite3AffinityType(ثابت حرف*, ستنه*);
فراغ sqlite3Analyze(Parse*, Token*, Token*);
صحيح sqlite3InvokeBusyHandler(BusyHandler*);
صحيح sqlite3_پیدا_کړه_Db(sqlite3*, Token*);
صحيح sqlite3_پیدا_کړه_Db_Name(sqlite3 *, ثابت حرف *);
صحيح sqlite3_Analysis_بارول(sqlite3*,صحيح iDB);
فراغ sqlite3_Delete_شاخص_Samples(sqlite3*,شاخص*);
فراغ sqlite3_Default_لیکه_Est(شاخص*);
فراغ sqlite3_ثبتوونکی_Like_Functions(sqlite3*, صحيح);
صحيح sqlite3IsLikeFunction(sqlite3*,بیان*,صحيح*,حرف*);
فراغ sqlite3SchemaClear(فراغ *);
Schema *sqlite3SchemaGet(sqlite3 *, Btree *);
صحيح sqlite3_Schema_منزل_شاخص(sqlite3 *db, Schema *);
کلی_Info *sqlite3_کلی_Info_Alloc(sqlite3*,صحيح,صحيح);
فراغ sqlite3_کلی_Info_Unref(کلی_Info*);
کلی_Info *sqlite3_کلی_Info_Ref(کلی_Info*);
کلی_Info *sqlite3_کلی_Info_د_شاخص(Parse*, شاخص*);
کلی_Info *sqlite3_کلی_Info_From_بیان_List(Parse*, بیان_List*, صحيح, صحيح);
ثابت حرف *sqlite3SelectOpName(صحيح);
صحيح sqlite3HasExplicitNulls(Parse*, بیان_List*);

#که_تعریف_شوی SQLITE_DEBUG
صحيح sqlite3_کلی_Info_Is_Writeable(کلی_Info*);
#د_که_پای
صحيح sqlite3CreateFunc(sqlite3 *, ثابت حرف *, صحيح, صحيح, فراغ *,
  فراغ (*)(sqlite3_context*,صحيح,sqlite3_ارزښت **),
  فراغ (*)(sqlite3_context*,صحيح,sqlite3_ارزښت **),
  فراغ (*)(sqlite3_context*),
  فراغ (*)(sqlite3_context*),
  فراغ (*)(sqlite3_context*,صحيح,sqlite3_ارزښت **),
  FuncDestructor *pDestructor
);
فراغ sqlite3NoopDestructor(فراغ*);
فراغ *sqlite3OomFault(sqlite3*);
فراغ sqlite3OomClear(sqlite3*);
صحيح sqlite3ApiExit(sqlite3 *db, صحيح);
صحيح sqlite3_Open_لنډمهاله_Database(Parse *);

حرف *sqlite3RCStrRef(حرف*);
فراغ sqlite3RCStrUnref(فراغ*);
حرف *sqlite3RCStrNew(u64);
حرف *sqlite3RCStrResize(حرف*,u64);

فراغ sqlite3StrAccumInit(StrAccum*, sqlite3*, حرف*, صحيح, صحيح);
صحيح sqlite3StrAccumEnlarge(StrAccum*, i64);
صحيح sqlite3StrAccumEnlargeIfNeeded(StrAccum*, i64);
حرف *sqlite3_Str_Accum_پای(StrAccum*);
فراغ sqlite3_Str_Accum_سېټ_Error(StrAccum*, u8);
فراغ sqlite3SelectDestInit(SelectDest*,صحيح,صحيح);
بیان *sqlite3_Create_ستنه_بیان(sqlite3 *, SrcList *, صحيح, صحيح);
فراغ sqlite3_ریکارډ_Error_Byte_دننه_واټن(sqlite3*,ثابت حرف*);
فراغ sqlite3_ریکارډ_Error_دننه_واټن_د_بیان(sqlite3*,ثابت بیان*);

فراغ sqlite3BackupRestart(sqlite3_backup *);
فراغ sqlite3BackupUpdate(sqlite3_backup *, Pgno, ثابت u8 *);

#که_نه_تعریف_شوی SQLITE_OMIT_SUBQUERY
صحيح sqlite3_بیان_Check_دننه(Parse*, بیان*);
#وإلا
# define sqlite3_بیان_Check_دننه(x,y) SQLITE_OK
#د_که_پای

#که_تعریف_شوی SQLITE_ENABLE_STAT4
صحيح sqlite3_Stat4_Probe_سېټ_ارزښت(
    Parse*,شاخص*,Unpacked_ریکارډ**,بیان*,صحيح,صحيح,صحيح*);
صحيح sqlite3_Stat4_ارزښت_From_بیان(Parse*, بیان*, u8, sqlite3_ارزښت**);
فراغ sqlite3Stat4ProbeFree(Unpacked_ریکارډ*);
صحيح sqlite3_Stat4_ستنه(sqlite3*, ثابت فراغ*, صحيح, صحيح, sqlite3_ارزښت**);
حرف sqlite3_شاخص_ستنه_Affinity(sqlite3*, شاخص*, صحيح);
#د_که_پای

/*
** The interface to the LEMON-generated parser
*/
#که_نه_تعریف_شوی SQLITE_AMALGAMATION
  فراغ *sqlite3ParserAlloc(فراغ*(*)(u64), Parse*);
  فراغ sqlite3ParserFree(فراغ*, فراغ(*)(فراغ*));
#د_که_پای
فراغ sqlite3Parser(فراغ*, صحيح, Token);
صحيح sqlite3ParserFallback(صحيح);
#که_تعریف_شوی YYTRACKMAXSTACKDEPTH
  صحيح sqlite3_Parser_ډېری_Peak(فراغ*);
#د_که_پای

فراغ sqlite3_Auto_بارول_Extensions(sqlite3*);
#که_نه_تعریف_شوی SQLITE_OMIT_بارول_EXTENSION
  فراغ sqlite3CloseExtensions(sqlite3*);
#وإلا
# define sqlite3CloseExtensions(X)
#د_که_پای

#که_نه_تعریف_شوی SQLITE_OMIT_SHARED_CACHE
  فراغ sqlite3_جدول_قلف(Parse *, صحيح, Pgno, u8, ثابت حرف *);
#وإلا
  #تعریف_کړه sqlite3_جدول_قلف(v,w,x,y,z)
#د_که_پای

#که_تعریف_شوی SQLITE_TEST
  صحيح sqlite3Utf8To8(بینښه حرف*);
#د_که_پای

#که_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
#  define sqlite3VtabClear(D,T)
#  define sqlite3VtabSync(X,Y) SQLITE_OK
#  define sqlite3VtabRollback(X)
#  define sqlite3VtabCommit(X)
#  define sqlite3_Vtab_دننه_Sync(db) 0
#  define sqlite3_Vtab_قلف(X)
#  define sqlite3VtabUnlock(X)
#  define sqlite3VtabModuleUnref(D,X)
#  define sqlite3VtabUnlockList(X)
#  define sqlite3VtabSavepoint(X, Y, Z) SQLITE_OK
#  define sqlite3GetVTable(X,Y)  ((VTable*)0)
#وإلا
   فراغ sqlite3VtabClear(sqlite3 *db, جدول*);
   فراغ sqlite3VtabDisconnect(sqlite3 *db, جدول *p);
   صحيح sqlite3VtabSync(sqlite3 *db, Vdbe*);
   صحيح sqlite3VtabRollback(sqlite3 *db);
   صحيح sqlite3VtabCommit(sqlite3 *db);
   فراغ sqlite3_Vtab_قلف(VTable *);
   فراغ sqlite3VtabUnlock(VTable *);
   فراغ sqlite3VtabModuleUnref(sqlite3*,Module*);
   فراغ sqlite3VtabUnlockList(sqlite3*);
   صحيح sqlite3VtabSavepoint(sqlite3 *, صحيح, صحيح);
   فراغ sqlite3VtabImportErrmsg(Vdbe*, sqlite3_vtab*);
   VTable *sqlite3GetVTable(sqlite3*, جدول*);
   Module *sqlite3VtabCreateModule(
     sqlite3*,
     ثابت حرف*,
     ثابت sqlite3_module*,
     فراغ*,
     فراغ(*)(فراغ*)
   );
#  define sqlite3_Vtab_دننه_Sync(db) ((db)->nVTrans>0 && (db)->aVTrans==0)
#د_که_پای
صحيح sqlite3ReadOnlyShadowTables(sqlite3 *db);
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
  صحيح sqlite3_Shadow_جدول_Name(sqlite3 *db, ثابت حرف *zName);
  صحيح sqlite3_Is_Shadow_جدول_د(sqlite3*,جدول*,ثابت حرف*);
  فراغ sqlite3_Mark_All_Shadow_Tables_د(sqlite3*, جدول*);
  فراغ sqlite3_Vtab_Eponymous_جدول_Clear_All(sqlite3*);
#وإلا
# define sqlite3_Shadow_جدول_Name(A,B) 0
# define sqlite3_Is_Shadow_جدول_د(A,B,C) 0
# define sqlite3_Mark_All_Shadow_Tables_د(A,B)
# define sqlite3_Vtab_Eponymous_جدول_Clear_All(X)
#د_که_پای
صحيح sqlite3_Vtab_Eponymous_جدول_Init(Parse*,Module*);
فراغ sqlite3_Vtab_Eponymous_جدول_Clear(sqlite3*,Module*);
فراغ sqlite3VtabMakeWritable(Parse*,جدول*);
فراغ sqlite3VtabBeginParse(Parse*, Token*, Token*, Token*, صحيح);
فراغ sqlite3_Vtab_پای_Parse(Parse*, Token*);
فراغ sqlite3VtabArgInit(Parse*);
فراغ sqlite3VtabArgExtend(Parse*, Token*);
صحيح sqlite3VtabCallCreate(sqlite3*, صحيح, ثابت حرف *, حرف **);
صحيح sqlite3VtabCallConnect(Parse*, جدول*);
صحيح sqlite3VtabCallDestroy(sqlite3*, صحيح, ثابت حرف *);
صحيح sqlite3VtabBegin(sqlite3 *, VTable *);

FuncDef *sqlite3VtabOverloadFunction(sqlite3 *,FuncDef*, صحيح nArg, بیان*);
فراغ sqlite3_Vtab_کاروي_All_Schemas(Parse*);
sqlite3_int64 sqlite3_Stmt_Current_وخت(sqlite3_context*);
صحيح sqlite3_Vdbe_Parameter_شاخص(Vdbe*, ثابت حرف*, صحيح);
صحيح sqlite3TransferBindings(sqlite3_stmt *, sqlite3_stmt *);
فراغ sqlite3ParseObjectInit(Parse*,sqlite3*);
فراغ sqlite3ParseObjectReset(Parse*);
فراغ *sqlite3_Parser_جمع_کړه_Cleanup(Parse*,فراغ(*)(sqlite3*,فراغ*),فراغ*);
#که_تعریف_شوی SQLITE_ENABLE_NORMALIZE
حرف *sqlite3Normalize(Vdbe*, ثابت حرف*);
#د_که_پای
صحيح sqlite3Reprepare(Vdbe*);
فراغ sqlite3_بیان_List_Check_اوږدتیا(Parse*, بیان_List*, ثابت حرف*);
CollSeq *sqlite3_بیان_Compare_Coll_Seq(Parse*,ثابت بیان*);
CollSeq *sqlite3_دوګونی_Compare_Coll_Seq(Parse *, ثابت بیان*, ثابت بیان*);
صحيح sqlite3_لنډمهاله_دننه_حافظه(ثابت sqlite3*);
ثابت حرف *sqlite3JournalModename(صحيح);
#که_نه_تعریف_شوی SQLITE_OMIT_WAL
  صحيح sqlite3Checkpoint(sqlite3*, صحيح, صحيح, صحيح*, صحيح*);
  صحيح sqlite3WalDefaultHook(فراغ*,sqlite3*,ثابت حرف*,صحيح);
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_CTE
  Cte *sqlite3CteNew(Parse*,Token*,بیان_List*,Select*,u8);
  فراغ sqlite3CteDelete(sqlite3*,Cte*);
  With *sqlite3_With_جمع_کړه(Parse*,With*,Cte*);
  فراغ sqlite3WithDelete(sqlite3*,With*);
  فراغ sqlite3WithDeleteGeneric(sqlite3*,فراغ*);
  With *sqlite3_With_ټېل(Parse*, With*, u8);
#وإلا
# define sqlite3CteNew(P,T,E,S)   ((فراغ*)0)
# define sqlite3CteDelete(D,C)
# define sqlite3_Cte_With_جمع_کړه(P,W,C) ((فراغ*)0)
# define sqlite3WithDelete(x,y)
# define sqlite3_With_ټېل(x,y,z) ((فراغ*)0)
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_UPSERT
  Upsert *sqlite3UpsertNew(sqlite3*,بیان_List*,بیان*,بیان_List*,بیان*,Upsert*);
  فراغ sqlite3UpsertDelete(sqlite3*,Upsert*);
  Upsert *sqlite3UpsertDup(sqlite3*,Upsert*);
  صحيح sqlite3_Upsert_Analyze_موخه(Parse*,SrcList*,Upsert*,Upsert*);
  فراغ sqlite3UpsertDoUpdate(Parse*,Upsert*,جدول*,شاخص*,صحيح);
  Upsert *sqlite3_Upsert_د_شاخص(Upsert*,شاخص*);
  صحيح sqlite3_Upsert_بل_Is_IPK(Upsert*);
#وإلا
#تعریف_کړه sqlite3UpsertNew(u,v,w,x,y,z) ((Upsert*)0)
#تعریف_کړه sqlite3UpsertDelete(x,y)
#تعریف_کړه sqlite3UpsertDup(x,y)         ((Upsert*)0)
#تعریف_کړه sqlite3_Upsert_د_شاخص(x,y)     ((Upsert*)0)
#تعریف_کړه sqlite3_Upsert_بل_Is_IPK(x)     0
#د_که_پای


/* Declarations for functions in fkey.c. All of these are replaced by
** no-op macros if OMIT_FOREIGN_KEY is defined. In this case no foreign
** key functionality is available. If OMIT_TRIGGER is defined but
** OMIT_FOREIGN_KEY is not, only some of the functions are no-oped. In
** this case foreign keys are parsed, but no other functionality is
** provided (enforcement of FK constraints requires the triggers sub-system).
*/
#إذا !defined(SQLITE_OMIT_FOREIGN_کلی) && !defined(SQLITE_OMIT_TRIGGER)
  فراغ sqlite3FkCheck(Parse*, جدول*, صحيح, صحيح, صحيح*, صحيح);
  فراغ sqlite3_Fk_Drop_جدول(Parse*, SrcList *, جدول*);
  فراغ sqlite3FkActions(Parse*, جدول*, بیان_List*, صحيح, صحيح*, صحيح);
  صحيح sqlite3FkRequired(Parse*, جدول*, صحيح*, صحيح);
  u32 sqlite3FkOldmask(Parse*, جدول*);
  FKey *sqlite3FkReferences(جدول *);
  فراغ sqlite3FkClearTriggerCache(sqlite3*,صحيح);
#وإلا
  #تعریف_کړه sqlite3FkActions(a,b,c,d,e,f)
  #تعریف_کړه sqlite3FkCheck(a,b,c,d,e,f)
  #تعریف_کړه sqlite3_Fk_Drop_جدول(a,b,c)
  #تعریف_کړه sqlite3FkOldmask(a,b)         0
  #تعریف_کړه sqlite3FkRequired(a,b,c,d)    0
  #تعریف_کړه sqlite3FkReferences(a)        0
  #تعریف_کړه sqlite3FkClearTriggerCache(a,b)
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_FOREIGN_کلی
  فراغ sqlite3FkDelete(sqlite3 *, جدول*);
  صحيح sqlite3_Fk_Locate_شاخص(Parse*,جدول*,FKey*,شاخص**,صحيح**);
#وإلا
  #تعریف_کړه sqlite3FkDelete(a,b)
  #تعریف_کړه sqlite3_Fk_Locate_شاخص(a,b,c,d,e)
#د_که_پای


/*
** Available fault injectors.  Should be numbered beginning with 0.
*/
#تعریف_کړه SQLITE_FAULTINJECTOR_MALLOC     0
#تعریف_کړه SQLITE_FAULTINJECTOR_شمېر      1

/*
** The interface to the code in fault.c used for identifying "benign"
** malloc failures. This is only present if SQLITE_UNTESTABLE
** is not defined.
*/
#که_نه_تعریف_شوی SQLITE_UNTESTABLE
  فراغ sqlite3BeginBenignMalloc(فراغ);
  فراغ sqlite3_پای_ټکی_Benign_Malloc(فراغ);
#وإلا
  #تعریف_کړه sqlite3BeginBenignMalloc()
  #تعریف_کړه sqlite3_پای_ټکی_Benign_Malloc()
#د_که_پای

/*
** Allowed return values from sqlite3FindInIndex()
*/
#تعریف_کړه دننه_شاخص_ROWID        1   /* Search the rowid of the table */
#تعریف_کړه دننه_شاخص_EPH          2   /* Search an ephemeral b-tree */
#تعریف_کړه دننه_شاخص_شاخص_ASC    3   /* Existing index ASCENDING */
#تعریف_کړه دننه_شاخص_شاخص_DESC   4   /* Existing index DESCENDING */
#تعریف_کړه دننه_شاخص_NOOP         5   /* No table available. Use comparisons */
/*
** Allowed flags for the 3rd parameter to sqlite3FindInIndex().
*/
#تعریف_کړه دننه_شاخص_NOOP_OK     0x0001  /* OK to return IN_INDEX_NOOP */
#تعریف_کړه دننه_شاخص_MEMBERSHIP  0x0002  /* IN operator used for membership test */
#تعریف_کړه دننه_شاخص_LOOP        0x0004  /* IN operator used as a loop */
صحيح sqlite3_پیدا_کړه_دننه_شاخص(Parse *, بیان *, u32, صحيح*, صحيح*, صحيح*);

صحيح sqlite3JournalOpen(sqlite3_vfs *, ثابت حرف *, sqlite3_file *, صحيح, صحيح);
صحيح sqlite3_Journal_اندازه(sqlite3_vfs *);
#إذا defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
  صحيح sqlite3JournalCreate(sqlite3_file *);
#د_که_پای

صحيح sqlite3_Journal_Is_دننه_حافظه(sqlite3_file *p);
فراغ sqlite3MemJournalOpen(sqlite3_file *);

فراغ sqlite3_بیان_سېټ_لوړوالی_And_Flags(Parse *pParse, بیان *p);
#إذا SQLITE_MAX_بیان_ژوروالی>0
  صحيح sqlite3_Select_بیان_لوړوالی(ثابت Select *);
  صحيح sqlite3_بیان_Check_لوړوالی(Parse*, صحيح);
#وإلا
  #تعریف_کړه sqlite3_Select_بیان_لوړوالی(x) 0
  #تعریف_کړه sqlite3_بیان_Check_لوړوالی(x,y)
#د_که_پای
فراغ sqlite3_بیان_سېټ_Error_دننه_واټن(بیان*,صحيح);

u32 sqlite3Get4byte(ثابت u8*);
SQLITE_OPT_INLINE u64 sqlite3Get8byte(ثابت u8*);
#إذا SQLITE_BYTEORDER!=4321
SQLITE_OPT_INLINE u64 sqlite3BSwap64(u64);
#د_که_پای
فراغ sqlite3Put4byte(u8*, u32);

#که_تعریف_شوی SQLITE_ENABLE_UNLOCK_NOTIFY
  فراغ sqlite3ConnectionBlocked(sqlite3 *, sqlite3 *);
  فراغ sqlite3ConnectionUnlocked(sqlite3 *db);
  فراغ sqlite3ConnectionClosed(sqlite3 *db);
#وإلا
  #تعریف_کړه sqlite3ConnectionBlocked(x,y)
  #تعریف_کړه sqlite3ConnectionUnlocked(x)
  #تعریف_کړه sqlite3ConnectionClosed(x)
#د_که_پای

#که_تعریف_شوی SQLITE_DEBUG
  فراغ sqlite3ParserTrace(FILE*, حرف *);
#د_که_پای
#إذا defined(YYCOVERAGE)
  صحيح sqlite3ParserCoverage(FILE*);
#د_که_پای

/*
** If the SQLITE_ENABLE IOTRACE exists then the global variable
** sqlite3IoTrace is a pointer to a printf-like routine used to
** print I/O tracing messages.
*/
#که_تعریف_شوی SQLITE_ENABLE_IOTRACE
# define IOTRACE(A)  إذا( sqlite3IoTrace ){ sqlite3IoTrace A; }
  فراغ sqlite3VdbeIOTraceSql(Vdbe*);
SQLITE_API SQLITE_EXTERN فراغ (SQLITE_CDECL *sqlite3IoTrace)(ثابت حرف*,...);
#وإلا
# define IOTRACE(A)
# define sqlite3VdbeIOTraceSql(X)
#د_که_پای

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
#که_تعریف_شوی SQLITE_MEMDEBUG
  فراغ sqlite3_Memdebug_سېټ_Type(فراغ*,u8);
  صحيح sqlite3MemdebugHasType(ثابت فراغ*,u8);
  صحيح sqlite3MemdebugNoType(ثابت فراغ*,u8);
#وإلا
# define sqlite3_Memdebug_سېټ_Type(X,Y)  /* no-op */
# define sqlite3MemdebugHasType(X,Y)  1
# define sqlite3MemdebugNoType(X,Y)   1
#د_که_پای
#تعریف_کړه MEMTYPE_ډېرۍ       0x01  /* General heap allocations */
#تعریف_کړه MEMTYPE_LOOKASIDE  0x02  /* Heap that might have been lookaside */
#تعریف_کړه MEMTYPE_PCACHE     0x04  /* Page cache allocations */

/*
** Threading interface
*/
#إذا SQLITE_MAX_کارګر_تارونه>0
صحيح sqlite3ThreadCreate(SQLiteThread**,فراغ*(*)(فراغ*),فراغ*);
صحيح sqlite3ThreadJoin(SQLiteThread*, فراغ**);
#د_که_پای

#إذا defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)
صحيح sqlite3_Dbpage_ثبتوونکی(sqlite3*);
#د_که_پای
#إذا defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)
صحيح sqlite3_Dbstat_ثبتوونکی(sqlite3*);
#د_که_پای

صحيح sqlite3_بیان_ویکټور_اندازه(ثابت بیان *p_بیان);
صحيح sqlite3_بیان_Is_ویکټور(ثابت بیان *p_بیان);
بیان *sqlite3_ویکټور_ډګر_Subexpr(بیان*, صحيح);
بیان *sqlite3_بیان_For_ویکټور_ډګر(Parse*,بیان*,صحيح,صحيح);
فراغ sqlite3_ویکټور_Error_پیغام(Parse*, بیان*);

#که_نه_تعریف_شوی SQLITE_OMIT_COMPILEOPTION_DIAGS
ثابت حرف **sqlite3CompileOptions(صحيح *pnOpt);
#د_که_پای

#إذا SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
صحيح sqlite3KvvfsInit(فراغ);
#د_که_پای

#إذا defined(VDBE_PROFILE) \
 || defined(SQLITE_PERFORMANCE_TRACE) \
 || defined(SQLITE_ENABLE_STMT_SCANSTATUS)
sqlite3_uint64 sqlite3Hwtime(فراغ);
#د_که_پای

#که_تعریف_شوی SQLITE_ENABLE_STMT_SCANSTATUS
# define IS_STMT_SCANSTATUS(db) (db->flags & SQLITE_Stmt_لټون_حالت)
#وإلا
# define IS_STMT_SCANSTATUS(db) 0
#د_که_پای

#د_که_پای /* SQLITEINT_H */
