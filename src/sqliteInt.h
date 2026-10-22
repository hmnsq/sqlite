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
#यदि_अपरिभाषितम् SQLITEINT_H
#परिभाषय SQLITEINT_H

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
#यदि_अपरिभाषितम् SQLITE_TCLAPI
#  define SQLITE_TCLAPI
#यदि_अन्तः

/*
** Include the header file used to customize the compiler options for MSVC.
** This should be done first so that it can successfully prevent spurious
** compiler warnings due to subsequent content in this file and other files
** that are included by this file.
*/
#अन्तर्भाव "msvc.h"

/*
** Special setup for VxWorks
*/
#अन्तर्भाव "vxworks.h"

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
#यदि_अपरिभाषितम् SQLITE_DISABLE_LFS
# define _LARGE_FILE       1
# ifndef FILE_विस्थापनम्_द्वयङ्काः
#   define FILE_विस्थापनम्_द्वयङ्काः 64
# endif
# define _LARGEFILE_SOURCE 1
#यदि_अन्तः

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
#यदि defined(__GNUC__) && !defined(SQLITE_DISABLE_INTRINSIC)
# define GCC_संस्करणम् (__GNUC__*1000000+__GNUC_MINOR__*1000+__GNUC_PATCHLEVEL__)
#अन्यथा
# define GCC_संस्करणम् 0
#यदि_अन्तः
#यदि defined(_MSC_VER) && !defined(SQLITE_DISABLE_INTRINSIC)
# define MSVC_संस्करणम् _MSC_VER
#अन्यथा
# define MSVC_संस्करणम् 0
#यदि_अन्तः

/*
** Some C99 functions in "math.h" are only present for MSVC when its version
** is associated with Visual Studio 2013 or higher.
*/
#यदि_अपरिभाषितम् SQLITE_HAVE_C99_MATH_FUNCS
# यदि MSVC_संस्करणम्==0 || MSVC_संस्करणम्>=1800
#  define SQLITE_HAVE_C99_MATH_FUNCS (1)
# अन्यथा
#  define SQLITE_HAVE_C99_MATH_FUNCS (0)
# endif
#यदि_अन्तः

/* Needed for various definitions... */
#यदि defined(__GNUC__) && !defined(_GNU_SOURCE)
# define _GNU_SOURCE
#यदि_अन्तः

#यदि defined(__OpenBSD__) && !defined(_BSD_SOURCE)
# define _BSD_SOURCE
#यदि_अन्तः

/*
** Macro to disable warnings about missing "break" at the end of a "case".
*/
#यदि defined(__has_attribute)
#  यदि __has_attribute(fallthrough)
#    define deliberate_fall_through __attribute__((fallthrough));
#  endif
#यदि_अन्तः
#यदि !defined(deliberate_fall_through)
#  define deliberate_fall_through
#यदि_अन्तः

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
#यदि defined(_HAVE_MINGW_H)
# include "mingw.h"
#नोचेद्यदि defined(_HAVE__MINGW_H)
# include "_mingw.h"
#यदि_अन्तः

/*
** For MinGW version 4.x (and higher), check to see if the _USE_32BIT_TIME_T
** define is required to maintain binary compatibility with the MSVC runtime
** library in use (e.g. for Windows XP).
*/
#यदि !defined(_USE_32BIT_TIME_T) && !defined(_USE_64BIT_TIME_T) && \
    defined(_WIN32) && !defined(_WIN64) && \
    defined(__MINGW_MAJOR_VERSION) && __MINGW_MAJOR_VERSION >= 4 && \
    defined(__MSVCRT__)
# define _USE_32BIT_TIME_T
#यदि_अन्तः

/* Optionally #include a user-defined header, whereby compilation options
** may be set prior to where they take effect, but after platform setup.
** If SQLITE_CUSTOM_INCLUDE=? is defined, its value names the #include
** file.
*/
#यदि_परिभाषितम् SQLITE_CUSTOM_INCLUDE
# define INC_STRINGIFY_(f) #f
# define INC_STRINGIFY(f) INC_STRINGIFY_(f)
# include INC_STRINGIFY(SQLITE_CUSTOM_INCLUDE)
#यदि_अन्तः

/* The public SQLite interface.  The _FILE_OFFSET_BITS macro must appear
** first in QNX.  Also, the _USE_32BIT_TIME_T macro must appear first for
** MinGW.
*/
#अन्तर्भाव "sqlite3.h"

/*
** Reuse the STATIC_VFS1 for mutex access to sqlite3_temp_directory.
*/
#परिभाषय SQLITE_MUTEX_STATIC_TEMPDIR SQLITE_MUTEX_STATIC_VFS1

/*
** Include the configuration header output by 'configure' if we're using the
** autoconf-based build
*/
#यदि defined(_HAVE_SQLITE_CONFIG_H) && !defined(SQLITECONFIG_H)
#अन्तर्भाव "sqlite_cfg.h"
#परिभाषय SQLITECONFIG_H 1
#यदि_अन्तः

#अन्तर्भाव "sqliteLimit.h"

/* Disable nuisance warnings on Borland compilers */
#यदि defined(__BORLANDC__)
#निर्देशः warn -rch /* unreachable code */
#निर्देशः warn -ccc /* Condition is always true or false */
#निर्देशः warn -aus /* Assigned value is never used */
#निर्देशः warn -csu /* Comparing signed and unsigned */
#निर्देशः warn -spa /* Suspicious pointer arithmetic */
#यदि_अन्तः

/*
** A few places in the code require atomic load/store of aligned
** integer values.
*/
#यदि_अपरिभाषितम् __has_extension
# define __has_extension(x) 0     /* compatibility with non-clang compilers */
#यदि_अन्तः
#यदि GCC_संस्करणम्>=4007000 || __has_extension(c_atomic)
# define SQLITE_ATOMIC_INTRINSICS 1
# define Atomic_आरोपय(PTR)       __atomic_load_n((PTR),__ATOMIC_RELAXED)
# define AtomicStore(PTR,VAL)  __atomic_store_n((PTR),(VAL),__ATOMIC_RELAXED)
#अन्यथा
# define SQLITE_ATOMIC_INTRINSICS 0
# define Atomic_आरोपय(PTR)       (*(PTR))
# define AtomicStore(PTR,VAL)  (*(PTR) = (VAL))
#यदि_अन्तः

/*
** Include standard header files as necessary
*/
#अन्तर्भाव <stdint.h>
#यदि_परिभाषितम् HAVE_INTTYPES_H
#अन्तर्भाव <inttypes.h>
#यदि_अन्तः

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
#यदि defined(HAVE_STDINT_H)   /* Use this case if we have ANSI headers */
# define SQLITE_INT_गन्तव्यम्_PTR(X)  ((शून्य*)(intptr_t)(X))
# define SQLITE_PTR_गन्तव्यम्_INT(X)  ((पूर्णाङ्क)(intptr_t)(X))
#नोचेद्यदि defined(__PTRDIFF_TYPE__)  /* This case should work for GCC */
# define SQLITE_INT_गन्तव्यम्_PTR(X)  ((शून्य*)(__PTRDIFF_TYPE__)(X))
# define SQLITE_PTR_गन्तव्यम्_INT(X)  ((पूर्णाङ्क)(__PTRDIFF_TYPE__)(X))
#नोचेद्यदि !defined(__GNUC__)       /* Works for compilers other than LLVM */
# define SQLITE_INT_गन्तव्यम्_PTR(X)  ((शून्य*)&((वर्ण*)0)[X])
# define SQLITE_PTR_गन्तव्यम्_INT(X)  ((पूर्णाङ्क)(((वर्ण*)X)-(वर्ण*)0))
#अन्यथा                          /* Generates a warning - but it always works */
# define SQLITE_INT_गन्तव्यम्_PTR(X)  ((शून्य*)(X))
# define SQLITE_PTR_गन्तव्यम्_INT(X)  ((पूर्णाङ्क)(X))
#यदि_अन्तः

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
#यदि defined(SQLITE_DISABLE_INLINE)
#  define SQLITE_NOINLINE
#  define SQLITE_INLINE
#  define SQLITE_OPT_INLINE
#  define SQLITE_उपयुङ्क्ते_INLINE 0
#नोचेद्यदि defined(__GNUC__)
#  define SQLITE_NOINLINE    __attribute__((noinline))
#  define SQLITE_INLINE      __attribute__((always_inline)) अन्तःस्थ
#  define SQLITE_OPT_INLINE  __attribute__((always_inline)) अन्तःस्थ
#  define SQLITE_उपयुङ्क्ते_INLINE 1
#नोचेद्यदि defined(_MSC_VER) && _MSC_VER>=1310
#  define SQLITE_NOINLINE    __declspec(noinline)
#  define SQLITE_INLINE      __forceinline
#  define SQLITE_OPT_INLINE  __forceinline
#  define SQLITE_उपयुङ्क्ते_INLINE 1
#अन्यथा
#  define SQLITE_NOINLINE
#  define SQLITE_INLINE
#  define SQLITE_OPT_INLINE
#  define SQLITE_उपयुङ्क्ते_INLINE 0
#यदि_अन्तः
#यदि defined(SQLITE_COVERAGE_TEST) || defined(__STRICT_ANSI__)
# undef SQLITE_INLINE
# define SQLITE_INLINE
# undef SQLITE_OPT_INLINE
# define SQLITE_OPT_INLINE
# undef SQLITE_उपयुङ्क्ते_INLINE
# define SQLITE_उपयुङ्क्ते_INLINE 0
#यदि_अन्तः
#यदि !defined(SQLITE_AMALGAMATION)
# undef SQLITE_OPT_INLINE
# define SQLITE_OPT_INLINE
#यदि_अन्तः

/*
** Make sure that the compiler intrinsics we desire are enabled when
** compiling with an appropriate version of MSVC unless prevented by
** the SQLITE_DISABLE_INTRINSIC define.
*/
#यदि !defined(SQLITE_DISABLE_INTRINSIC)
#  यदि defined(_MSC_VER) && _MSC_VER>=1400
#    यदि !defined(_WIN32_WCE)
#      include <intrin.h>
#      pragma intrinsic(_byteswap_ushort)
#      pragma intrinsic(_byteswap_ulong)
#      pragma intrinsic(_byteswap_uint64)
#      pragma intrinsic(_ReadWriteBarrier)
#    अन्यथा
#      include <cmnintrin.h>
#    endif
#  endif
#यदि_अन्तः

/*
** Enable SQLITE_USE_SEH by default on MSVC builds.  Only omit
** SEH support if the -DSQLITE_OMIT_SEH option is given.
*/
#यदि defined(_MSC_VER) && !defined(SQLITE_OMIT_SEH)
# define SQLITE_USE_SEH 1
#अन्यथा
# undef SQLITE_USE_SEH
#यदि_अन्तः

/*
** Enable SQLITE_DIRECT_OVERFLOW_READ, unless the build explicitly
** disables it using -DSQLITE_DIRECT_OVERFLOW_READ=0
*/
#यदि defined(SQLITE_प्रत्यक्षम्_अतिप्रवाहः_READ) && SQLITE_प्रत्यक्षम्_अतिप्रवाहः_READ+1==1
  /* Disable if -DSQLITE_DIRECT_OVERFLOW_READ=0 */
# undef SQLITE_प्रत्यक्षम्_अतिप्रवाहः_READ
#अन्यथा
  /* In all other cases, enable */
# define SQLITE_प्रत्यक्षम्_अतिप्रवाहः_READ 1
#यदि_अन्तः


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
#यदि !defined(SQLITE_THREADSAFE)
# यदि defined(THREADSAFE)
#   define SQLITE_THREADSAFE THREADSAFE
# अन्यथा
#   define SQLITE_THREADSAFE 1 /* IMP: R-07272-22309 */
# endif
#यदि_अन्तः

/*
** Powersafe overwrite is on by default.  But can be turned off using
** the -DSQLITE_POWERSAFE_OVERWRITE=0 command-line option.
*/
#यदि_अपरिभाषितम् SQLITE_POWERSAFE_OVERWRITE
# define SQLITE_POWERSAFE_OVERWRITE 1
#यदि_अन्तः

/*
** EVIDENCE-OF: R-25715-37072 Memory allocation statistics are enabled by
** default unless SQLite is compiled with SQLITE_DEFAULT_MEMSTATUS=0 in
** which case memory allocation statistics are disabled by default.
*/
#यदि !defined(SQLITE_DEFAULT_MEMSTATUS)
# define SQLITE_DEFAULT_MEMSTATUS 1
#यदि_अन्तः

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
#यदि defined(SQLITE_SYSTEM_MALLOC) \
  + defined(SQLITE_WIN32_MALLOC) \
  + defined(SQLITE_ZERO_MALLOC) \
  + defined(SQLITE_MEMDEBUG)>1
# error "Two or अधिकम् सम्बन्धि the following compile-time configuration options\
 are defined but at most one is allowed:\
 SQLITE_SYSTEM_MALLOC, SQLITE_WIN32_MALLOC, SQLITE_MEMDEBUG,\
 SQLITE_ZERO_MALLOC"
#यदि_अन्तः
#यदि defined(SQLITE_SYSTEM_MALLOC) \
  + defined(SQLITE_WIN32_MALLOC) \
  + defined(SQLITE_ZERO_MALLOC) \
  + defined(SQLITE_MEMDEBUG)==0
# define SQLITE_SYSTEM_MALLOC 1
#यदि_अन्तः

/*
** If SQLITE_MALLOC_SOFT_LIMIT is not zero, then try to keep the
** sizes of memory allocations below this value where possible.
*/
#यदि !defined(SQLITE_MALLOC_SOFT_सीमा)
# define SQLITE_MALLOC_SOFT_सीमा 1024
#यदि_अन्तः

/*
** We need to define _XOPEN_SOURCE as follows in order to enable
** recursive mutexes on most Unix systems and fchmod() on OpenBSD.
** But _XOPEN_SOURCE define causes problems for Mac OS X, so omit
** it.
*/
#यदि !defined(_XOPEN_SOURCE) && !defined(__DARWIN__) && !defined(__APPLE__)
#  define _XOPEN_SOURCE 600
#यदि_अन्तः

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
#यदि !defined(NDEBUG) && !defined(SQLITE_DEBUG)
# define NDEBUG 1
#यदि_अन्तः
#यदि defined(NDEBUG) && defined(SQLITE_DEBUG)
# undef NDEBUG
#यदि_अन्तः

/*
** Enable SQLITE_ENABLE_EXPLAIN_COMMENTS if SQLITE_DEBUG is turned on.
*/
#यदि !defined(SQLITE_ENABLE_EXPLAIN_COMMENTS) && defined(SQLITE_DEBUG)
# define SQLITE_ENABLE_EXPLAIN_COMMENTS 1
#यदि_अन्तः

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
#यदि defined(SQLITE_MUTATION_TEST)
# define testcase(X)
#नोचेद्यदि defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_DEBUG)
# ifndef SQLITE_AMALGAMATION
    बाह्य अचिह्नित पूर्णाङ्क sqlite3_Coverage_गणक;
# endif
# define testcase(X)  यदि( X ){ sqlite3_Coverage_गणक += (अचिह्नित)__LINE__; }
#अन्यथा
# define testcase(X)
#यदि_अन्तः

/*
** The TESTONLY macro is used to enclose variable declarations or
** other bits of code that are needed to support the arguments
** within testcase() and assert() macros.
*/
#यदि !defined(NDEBUG) || defined(SQLITE_COVERAGE_TEST)
# define TESTONLY(X)  X
#अन्यथा
# define TESTONLY(X)
#यदि_अन्तः

/*
** Sometimes we need a small amount of code such as a variable initialization
** to setup for a later assert() statement.  We do not want this code to
** appear when assert() is disabled.  The following macro is therefore
** used to contain that setup code.  The "VVA" acronym stands for
** "Verification, Validation, and Accreditation".  In other words, the
** code within VVA_ONLY() will only run during verification processes.
*/
#यदि_अपरिभाषितम् NDEBUG
# define VVA_ONLY(X)  X
#अन्यथा
# define VVA_ONLY(X)
#यदि_अन्तः

/*
** Disable ALWAYS() and NEVER() (make them pass-throughs) for coverage
** and mutation testing
*/
#यदि defined(SQLITE_COVERAGE_TEST) || defined(SQLITE_MUTATION_TEST)
# define SQLITE_OMIT_AUXILIARY_SAFETY_CHECKS  1
#यदि_अन्तः

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
#यदि defined(SQLITE_OMIT_AUXILIARY_SAFETY_CHECKS)
# define ALWAYS(X)      (1)
# define NEVER(X)       (0)
#नोचेद्यदि !defined(NDEBUG)
# define ALWAYS(X)      ((X)?1:(प्रतिज्ञा(0),0))
# define NEVER(X)       ((X)?(प्रतिज्ञा(0),1):0)
#अन्यथा
# define ALWAYS(X)      (X)
# define NEVER(X)       (X)
#यदि_अन्तः

/*
** Some conditionals are optimizations only.  In other words, if the
** conditionals are replaced with a constant 1 (true) or 0 (false) then
** the correct answer is still obtained, though perhaps not as quickly.
**
** The following macros mark these optimizations conditionals.
*/
#यदि defined(SQLITE_MUTATION_TEST)
# define OK_IF_ALWAYS_TRUE(X)  (1)
# define OK_IF_ALWAYS_FALSE(X) (0)
#अन्यथा
# define OK_IF_ALWAYS_TRUE(X)  (X)
# define OK_IF_ALWAYS_FALSE(X) (X)
#यदि_अन्तः

/*
** Some malloc failures are only possible if SQLITE_TEST_REALLOC_STRESS is
** defined.  We need to defend against those failures when testing with
** SQLITE_TEST_REALLOC_STRESS, but we don't want the unreachable branches
** during a normal build.  The following macro can be used to disable tests
** that are always false except when SQLITE_TEST_REALLOC_STRESS is set.
*/
#यदि defined(SQLITE_TEST_REALLOC_STRESS)
# define ONLY_IF_REALLOC_STRESS(X)  (X)
#नोचेद्यदि !defined(NDEBUG)
# define ONLY_IF_REALLOC_STRESS(X)  ((X)?(प्रतिज्ञा(0),1):0)
#अन्यथा
# define ONLY_IF_REALLOC_STRESS(X)  (0)
#यदि_अन्तः

/*
** Declarations used for tracing the operating system interfaces.
*/
#यदि defined(SQLITE_FORCE_OS_TRACE) || defined(SQLITE_TEST) || \
    (defined(SQLITE_DEBUG) && SQLITE_OS_WIN)
  बाह्य पूर्णाङ्क sqlite3OSTrace;
# define OSTRACE(X)          यदि( sqlite3OSTrace ) sqlite3DebugPrintf X
# define SQLITE_HAVE_OS_TRACE
#अन्यथा
# define OSTRACE(X)
# undef  SQLITE_HAVE_OS_TRACE
#यदि_अन्तः

/*
** Is the sqlite3ErrName() function needed in the build?  Currently,
** it is needed by "mutex_w32.c" (when debugging), "os_win.c" (when
** OSTRACE is enabled), and by several "test*.c" files (which are
** compiled using SQLITE_TEST).
*/
#यदि defined(SQLITE_HAVE_OS_TRACE) || defined(SQLITE_TEST) || \
    (defined(SQLITE_DEBUG) && SQLITE_OS_WIN)
# define SQLITE_NEED_ERR_NAME
#अन्यथा
# undef  SQLITE_NEED_ERR_NAME
#यदि_अन्तः

/*
** SQLITE_ENABLE_EXPLAIN_COMMENTS is incompatible with SQLITE_OMIT_EXPLAIN
*/
#यदि_परिभाषितम् SQLITE_OMIT_EXPLAIN
# undef SQLITE_ENABLE_EXPLAIN_COMMENTS
#यदि_अन्तः

/*
** SQLITE_OMIT_VIRTUALTABLE implies SQLITE_OMIT_ALTERTABLE
*/
#यदि defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_ALTERTABLE)
# define SQLITE_OMIT_ALTERTABLE
#यदि_अन्तः

#परिभाषय SQLITE_DIGIT_SEPARATOR '_'

/*
** Return true (non-zero) if the input is an integer that is too large
** to fit in 32-bits.  This macro is used inside of various testcase()
** macros to verify that we have tested SQLite for large-file support.
*/
#परिभाषय IS_BIG_INT(X)  (((X)&~(i64)0xffffffff)!=0)

/*
** The macro unlikely() is a hint that surrounds a boolean
** expression that is usually false.  Macro likely() surrounds
** a boolean expression that is usually true.  These hints could,
** in theory, be used by the compiler to generate better code, but
** currently they are just comments for human readers.
*/
#परिभाषय likely(X)    (X)
#परिभाषय unlikely(X)  (X)

#अन्तर्भाव "hash.h"
#अन्तर्भाव "parse.h"
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <ctype.h>

/*
** Use a macro to replace memcpy() if compiled with SQLITE_INLINE_MEMCPY.
** This allows better measurements of where memcpy() is used when running
** cachegrind.  But this macro version of memcpy() is very slow so it
** should not be used in production.  This is a performance measurement
** hack only.
*/
#यदि_परिभाषितम् SQLITE_INLINE_MEMCPY
# define स्मृतिं_प्रतिलिख(D,S,N) {वर्ण*xxd=(वर्ण*)(D);ध्रुव वर्ण*xxs=(ध्रुव वर्ण*)(S);\
                        पूर्णाङ्क xxn=(N);यावत्(xxn-->0)*(xxd++)=*(xxs++);}
#यदि_अन्तः

/*
** If compiling for a processor that lacks floating point support,
** substitute integer for floating-point
*/
#यदि_परिभाषितम् SQLITE_OMIT_FLOATING_POINT
# define द्विगुण sqlite_int64
# define भिन्न sqlite_int64
# define fabs(X) ((X)<0?-(X):(X))
# define sqlite3_Is_अतिप्रवाहः(X) 0
# define INFINITY (9223372036854775807LL)
# ifndef SQLITE_BIG_DBL
#   define SQLITE_BIG_DBL (((sqlite3_int64)1)<<50)
# endif
# define SQLITE_OMIT_DATETIME_FUNCS 1
# define SQLITE_OMIT_TRACE 1
# undef SQLITE_MIXED_ENDIAN_64BIT_FLOAT
# undef SQLITE_HAVE_ISNAN
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_BIG_DBL
# define SQLITE_BIG_DBL (1e99)
#यदि_अन्तः

/*
** OMIT_TEMPDB is set to 1 if SQLITE_OMIT_TEMPDB is defined, or 0
** afterward. Having this macro allows us to cause the C compiler
** to omit code used by TEMP tables without messy #ifndef statements.
*/
#यदि_परिभाषितम् SQLITE_OMIT_TEMPDB
#परिभाषय OMIT_TEMPDB 1
#अन्यथा
#परिभाषय OMIT_TEMPDB 0
#यदि_अन्तः

/*
** The "file format" number is an integer that is incremented whenever
** the VDBE-level file format changes.  The following macros define the
** the default file format for new databases and the maximum file format
** that the library can read.
*/
#परिभाषय SQLITE_MAX_FILE_FORMAT 4
#यदि_अपरिभाषितम् SQLITE_DEFAULT_FILE_FORMAT
# define SQLITE_DEFAULT_FILE_FORMAT 4
#यदि_अन्तः

/*
** Determine whether triggers are recursive by default.  This can be
** changed at run-time using a pragma.
*/
#यदि_अपरिभाषितम् SQLITE_DEFAULT_RECURSIVE_TRIGGERS
# define SQLITE_DEFAULT_RECURSIVE_TRIGGERS 0
#यदि_अन्तः

/*
** Provide a default value for SQLITE_TEMP_STORE in case it is not specified
** on the command-line
*/
#यदि_अपरिभाषितम् SQLITE_अस्थायी_STORE
# define SQLITE_अस्थायी_STORE 1
#यदि_अन्तः

/*
** If no value has been provided for SQLITE_MAX_WORKER_THREADS, or if
** SQLITE_TEMP_STORE is set to 3 (never use temporary files), set it
** to zero.
*/
#यदि SQLITE_अस्थायी_STORE==3 || SQLITE_THREADSAFE==0
# undef SQLITE_MAX_कार्यकर्ता_तन्तवः
# define SQLITE_MAX_कार्यकर्ता_तन्तवः 0
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_MAX_कार्यकर्ता_तन्तवः
# define SQLITE_MAX_कार्यकर्ता_तन्तवः 8
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_DEFAULT_कार्यकर्ता_तन्तवः
# define SQLITE_DEFAULT_कार्यकर्ता_तन्तवः 0
#यदि_अन्तः
#यदि SQLITE_DEFAULT_कार्यकर्ता_तन्तवः>SQLITE_MAX_कार्यकर्ता_तन्तवः
# undef SQLITE_MAX_कार्यकर्ता_तन्तवः
# define SQLITE_MAX_कार्यकर्ता_तन्तवः SQLITE_DEFAULT_कार्यकर्ता_तन्तवः
#यदि_अन्तः

/*
** The default initial allocation for the pagecache when using separate
** pagecaches for each database connection.  A positive number is the
** number of pages.  A negative number N translations means that a buffer
** of -1024*N bytes is allocated and used for as many pages as it will hold.
**
** The default value of "20" was chosen to minimize the run-time of the
** speedtest1 test program with options: --shrink-memory --reprepare
*/
#यदि_अपरिभाषितम् SQLITE_DEFAULT_PCACHE_INITSZ
# define SQLITE_DEFAULT_PCACHE_INITSZ 20
#यदि_अन्तः

/*
** Default value for the SQLITE_CONFIG_SORTERREF_SIZE option.
*/
#यदि_अपरिभाषितम् SQLITE_DEFAULT_SORTERREF_आकार
# define SQLITE_DEFAULT_SORTERREF_आकार 0x7fffffff
#यदि_अन्तः

/*
** The compile-time options SQLITE_MMAP_READWRITE and
** SQLITE_ENABLE_BATCH_ATOMIC_WRITE are not compatible with one another.
** You must choose one or the other (or neither) but not both.
*/
#यदि defined(SQLITE_MMAP_READWRITE) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
#दोषः Cannot use both SQLITE_MMAP_READWRITE and SQLITE_ENABLE_BATCH_ATOMIC_WRITE
#यदि_अन्तः

/*
** GCC does not define the offsetof() macro so we'll have to do it
** ourselves.
*/
#यदि_अपरिभाषितम् offsetof
# define offsetof(ST,M) ((size_t)((वर्ण*)&((ST*)0)->M - (वर्ण*)0))
#यदि_अन्तः

/*
** sizeof64() is like sizeof(), but always returns a 64-bit value, even
** on 32-bit builds. This can help to avoid overflow by ensuring 64-bit
** arithmetic is used consistently in both 32-bit and 64-bit builds.
*/
#परिभाषय sizeof64(X) ((sqlite3_int64)परिमाण(X))

/*
** Work around C99 "flex-array" syntax for pre-C99 compilers, so as
** to avoid complaints from -fsanitize=strict-bounds.
*/
#यदि defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
# define FLEXARRAY
#अन्यथा
# define FLEXARRAY 1
#यदि_अन्तः

/*
** Macros to compute minimum and maximum of two numbers.
*/
#यदि_अपरिभाषितम् MIN
# define MIN(A,B) ((A)<(B)?(A):(B))
#यदि_अन्तः
#यदि_अपरिभाषितम् MAX
# define MAX(A,B) ((A)>(B)?(A):(B))
#यदि_अन्तः

/*
** Swap two objects of type TYPE.
*/
#परिभाषय SWAP(TYPE,A,B) {TYPE t=A; A=B; B=t;}

/*
** Check to see if this machine uses EBCDIC.  (Yes, believe it or
** not, there are still machines out there that use EBCDIC.)
*/
#यदि 'A' == '\301'
# define SQLITE_EBCDIC 1
#अन्यथा
# define SQLITE_ASCII 1
#यदि_अन्तः

/*
** Integers of known sizes.  These typedefs might change for architectures
** where the sizes very.  Preprocessor macros are available so that the
** types can be conveniently redefined at compile-type.  Like this:
**
**         cc '-DUINTPTR_TYPE=long long int' ...
*/
#यदि_अपरिभाषितम् UINT32_TYPE
# ifdef HAVE_UINT32_T
#  define UINT32_TYPE uint32_t
# अन्यथा
#  define UINT32_TYPE अचिह्नित पूर्णाङ्क
# endif
#यदि_अन्तः
#यदि_अपरिभाषितम् UINT16_TYPE
# ifdef HAVE_UINT16_T
#  define UINT16_TYPE uint16_t
# अन्यथा
#  define UINT16_TYPE अचिह्नित ह्रस्व पूर्णाङ्क
# endif
#यदि_अन्तः
#यदि_अपरिभाषितम् INT16_TYPE
# ifdef HAVE_INT16_T
#  define INT16_TYPE int16_t
# अन्यथा
#  define INT16_TYPE ह्रस्व पूर्णाङ्क
# endif
#यदि_अन्तः
#यदि_अपरिभाषितम् UINT8_TYPE
# ifdef HAVE_UINT8_T
#  define UINT8_TYPE uint8_t
# अन्यथा
#  define UINT8_TYPE अचिह्नित वर्ण
# endif
#यदि_अन्तः
#यदि_अपरिभाषितम् INT8_TYPE
# ifdef HAVE_INT8_T
#  define INT8_TYPE int8_t
# अन्यथा
#  define INT8_TYPE चिह्नित वर्ण
# endif
#यदि_अन्तः
प्रकारनिर्देश sqlite_int64 i64;          /* 8-byte signed integer */
प्रकारनिर्देश sqlite_uint64 u64;         /* 8-byte unsigned integer */
प्रकारनिर्देश UINT32_TYPE u32;           /* 4-byte unsigned integer */
प्रकारनिर्देश UINT16_TYPE u16;           /* 2-byte unsigned integer */
प्रकारनिर्देश INT16_TYPE i16;            /* 2-byte signed integer */
प्रकारनिर्देश UINT8_TYPE u8;             /* 1-byte unsigned integer */
प्रकारनिर्देश INT8_TYPE i8;              /* 1-byte signed integer */

/* A bitfield type for use inside of structures.  Always follow with :N where
** N is the number of bits.
*/
प्रकारनिर्देश अचिह्नित bft;  /* Bit Field Type */

/*
** SQLITE_MAX_U32 is a u64 constant that is the maximum u64 value
** that can be stored in a u32 without loss of data.  The value
** is 0x00000000ffffffff.  But because of quirks of some compilers, we
** have to specify the value in the less intuitive manner shown:
*/
#परिभाषय SQLITE_MAX_U32  ((((u64)1)<<32)-1)

/*
** The datatype used to store estimates of the number of rows in a
** table or index.
*/
प्रकारनिर्देश u64 tRowcnt;

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
प्रकारनिर्देश INT16_TYPE LogEst;
#परिभाषय LOGEST_MIN (-32768)
#परिभाषय LOGEST_MAX (32767)

/*
** Set the SQLITE_PTRSIZE macro to the number of bytes in a pointer
*/
#यदि_अपरिभाषितम् SQLITE_PTRSIZE
# यदि defined(__SIZEOF_POINTER__)
#   define SQLITE_PTRSIZE __SIZEOF_POINTER__
# elif defined(i386)     || defined(__i386__)   || defined(_M_IX86) ||    \
       defined(_M_ARM)   || defined(__arm__)    || defined(__x86)   ||    \
      (defined(__APPLE__) && defined(__ppc__)) ||                         \
      (defined(__TOS_AIX__) && !defined(__64BIT__))
#   define SQLITE_PTRSIZE 4
# अन्यथा
#   define SQLITE_PTRSIZE 8
# endif
#यदि_अन्तः

/* The uptr type is an unsigned integer large enough to hold a pointer
*/
#यदि defined(HAVE_STDINT_H)
  प्रकारनिर्देश uintptr_t uptr;
#नोचेद्यदि SQLITE_PTRSIZE==4
  प्रकारनिर्देश u32 uptr;
#अन्यथा
  प्रकारनिर्देश u64 uptr;
#यदि_अन्तः

/*
** The SQLITE_WITHIN(P,S,E) macro checks to see if pointer P points to
** something between S (inclusive) and E (exclusive).
**
** In other words, S is a buffer and E is a pointer to the first byte after
** the end of buffer S.  This macro returns true if P points to something
** contained within the buffer S.
*/
#परिभाषय SQLITE_WITHIN(P,S,E)   (((uptr)(P)>=(uptr)(S))&&((uptr)(P)<(uptr)(E)))

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
#परिभाषय SQLITE_अतिप्रवाहः(P,S,E) (((uptr)(S)<(uptr)(P))&&((uptr)(E)>(uptr)(P)))

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
#यदि_अपरिभाषितम् SQLITE_BYTEORDER  /* Replicate changes at tag-20230904a */
# यदि defined(__BYTE_ORDER__) && __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
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
#यदि_अन्तः
#यदि SQLITE_BYTEORDER==4321
# define SQLITE_BIGENDIAN    1
# define SQLITE_LITTLEENDIAN 0
# define SQLITE_UTF16NATIVE  SQLITE_UTF16BE
#नोचेद्यदि SQLITE_BYTEORDER==1234
# define SQLITE_BIGENDIAN    0
# define SQLITE_LITTLEENDIAN 1
# define SQLITE_UTF16NATIVE  SQLITE_UTF16LE
#अन्यथा
# ifdef SQLITE_AMALGAMATION
  ध्रुव पूर्णाङ्क sqlite3one = 1;
# अन्यथा
  बाह्य ध्रुव पूर्णाङ्क sqlite3one;
# endif
# define SQLITE_BIGENDIAN    (*(वर्ण *)(&sqlite3one)==0)
# define SQLITE_LITTLEENDIAN (*(वर्ण *)(&sqlite3one)==1)
# define SQLITE_UTF16NATIVE  (SQLITE_BIGENDIAN?SQLITE_UTF16BE:SQLITE_UTF16LE)
#यदि_अन्तः

/*
** Constants for the largest and smallest possible 64-bit signed integers.
** These macros are designed to work correctly on both 32-bit and 64-bit
** compilers.
*/
#परिभाषय LARGEST_INT64  (0xffffffff|(((i64)0x7fffffff)<<32))
#परिभाषय LARGEST_UINT64 (0xffffffff|(((u64)0xffffffff)<<32))
#परिभाषय SMALLEST_INT64 (((i64)-1) - LARGEST_INT64)

/*
** Macro SMXV(n) return the maximum value that can be held in variable n,
** assuming n is a signed integer type.  UMXV(n) is similar for unsigned
** integer types.
*/
#परिभाषय SMXV(n) ((((i64)1)<<(परिमाण(n)*8-1))-1)
#परिभाषय UMXV(n) ((((i64)1)<<(परिमाण(n)*8))-1)

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
#परिभाषय ROUND8(x)     (((x)+7)&~7)
#यदि SQLITE_PTRSIZE==8
# define ROUND8P(x)   (x)
#अन्यथा
# define ROUND8P(x)   (((x)+7)&~7)
#यदि_अन्तः

/*
** Round down to the nearest multiple of 8
*/
#परिभाषय ROUNDDOWN8(x) ((x)&~7)

/*
** Assert that the pointer X is aligned to an 8-byte boundary.  This
** macro is used only within assert() to verify that the code gets
** all alignment restrictions correct.
**
** Except, if SQLITE_4_BYTE_ALIGNED_MALLOC is defined, then the
** underlying malloc() implementation might return us 4-byte aligned
** pointers.  In that case, only verify 4-byte alignment.
*/
#यदि_परिभाषितम् SQLITE_4_BYTE_ALIGNED_MALLOC
# define EIGHT_BYTE_ALIGNMENT(X)   ((((uptr)(X) - (uptr)0)&3)==0)
#अन्यथा
# define EIGHT_BYTE_ALIGNMENT(X)   ((((uptr)(X) - (uptr)0)&7)==0)
#यदि_अन्तः
#परिभाषय TWO_BYTE_ALIGNMENT(X)      ((((uptr)(X) - (uptr)0)&1)==0)

/*
** Disable MMAP on platforms where it is known to not work
*/
#यदि defined(__OpenBSD__) || defined(__QNXNTO__)
# undef SQLITE_MAX_MMAP_आकार
# define SQLITE_MAX_MMAP_आकार 0
#यदि_अन्तः

/*
** Default maximum size of memory used by memory-mapped I/O in the VFS
*/
#यदि_परिभाषितम् __APPLE__
# include <TargetConditionals.h>
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_MAX_MMAP_आकार
# यदि defined(__linux__) \
  || defined(_WIN32) \
  || (defined(__APPLE__) && defined(__MACH__)) \
  || defined(__sun) \
  || defined(__FreeBSD__) \
  || defined(__DragonFly__)
#   define SQLITE_MAX_MMAP_आकार 0x7fff0000  /* 2147418112 */
# अन्यथा
#   define SQLITE_MAX_MMAP_आकार 0
# endif
#यदि_अन्तः

/*
** The default MMAP_SIZE is zero on all platforms.  Or, even if a larger
** default MMAP_SIZE is specified at compile-time, make sure that it does
** not exceed the maximum mmap size.
*/
#यदि_अपरिभाषितम् SQLITE_DEFAULT_MMAP_आकार
# define SQLITE_DEFAULT_MMAP_आकार 0
#यदि_अन्तः
#यदि SQLITE_DEFAULT_MMAP_आकार>SQLITE_MAX_MMAP_आकार
# undef SQLITE_DEFAULT_MMAP_आकार
# define SQLITE_DEFAULT_MMAP_आकार SQLITE_MAX_MMAP_आकार
#यदि_अन्तः

/*
** TREETRACE_ENABLED will be either 1 or 0 depending on whether or not
** the Abstract Syntax Tree tracing logic is turned on.
*/
#यदि !defined(SQLITE_AMALGAMATION)
बाह्य u32 sqlite3_वृक्षः_Trace;
#यदि_अन्तः
#यदि defined(SQLITE_DEBUG) \
    && (defined(SQLITE_TEST) || defined(SQLITE_ENABLE_SELECTTRACE) \
                             || defined(SQLITE_ENABLE_TREETRACE))
# define TREETRACE_ENABLED 1
# define TREETRACE(K,P,S,X)  \
  यदि(sqlite3_वृक्षः_Trace&(K))   \
    sqlite3DebugPrintf("%u/%d/%p: ",(S)->selId,(P)->addrExplain,(S)),\
    sqlite3DebugPrintf X
#अन्यथा
# define TREETRACE(K,P,S,X)
# define TREETRACE_ENABLED 0
#यदि_अन्तः

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
#यदि defined(SQLITE_DEBUG) \
    && (defined(SQLITE_TEST) || defined(SQLITE_ENABLE_WHERETRACE))
# define WHERETRACE(K,X)  यदि(sqlite3WhereTrace&(K)) sqlite3DebugPrintf X
# define WHERETRACE_ENABLED 1
#अन्यथा
# define WHERETRACE(K,X)
#यदि_अन्तः

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
प्रकारनिर्देश संरचना BusyHandler BusyHandler;
संरचना BusyHandler {
  पूर्णाङ्क (*xBusyHandler)(शून्य *,पूर्णाङ्क);  /* The busy callback */
  शून्य *pBusyArg;                   /* First arg to busy callback */
  पूर्णाङ्क nBusy;                        /* Incremented with each busy call */
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
#परिभाषय LEGACY_SCHEMA_सारणी          "sqlite_master"
#परिभाषय LEGACY_अस्थायी_SCHEMA_सारणी     "sqlite_temp_master"
#परिभाषय PREFERRED_SCHEMA_सारणी       "sqlite_schema"
#परिभाषय PREFERRED_अस्थायी_SCHEMA_सारणी  "sqlite_temp_schema"


/*
** The root-page of the schema table.
*/
#परिभाषय SCHEMA_मूलम्    1

/*
** The name of the schema table.  The name is different for TEMP.
*/
#परिभाषय SCHEMA_सारणी(x) \
    ((!OMIT_TEMPDB)&&(x==1)?LEGACY_अस्थायी_SCHEMA_सारणी:LEGACY_SCHEMA_सारणी)

/*
** A convenience macro that returns the number of elements in
** an array.
*/
#परिभाषय Array_आकार(X)    ((पूर्णाङ्क)(परिमाण(X)/परिमाण(X[0])))

/*
** Determine if the argument is a power of two
*/
#परिभाषय Is_Power_सम्बन्धि_Two(X) (((X)&((X)-1))==0)

/*
** The following value as a destructor means to use sqlite3DbFree().
** The sqlite3DbFree() routine requires two parameters instead of the
** one parameter that destructors normally want.  So we have to introduce
** this magic value that the code knows to handle differently.  Any
** pointer will work here as long as it is distinct from SQLITE_STATIC
** and SQLITE_TRANSIENT.
*/
#परिभाषय SQLITE_DYNAMIC   ((sqlite3_destructor_type)sqlite3_Row_समुच्चयः_Clear)

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
#यदि_परिभाषितम् SQLITE_OMIT_WSD
  #परिभाषय SQLITE_WSD ध्रुव
  #परिभाषय GLOBAL(t,v) (*(t*)sqlite3_wsd_find((शून्य*)&(v), परिमाण(v)))
  #परिभाषय sqlite3GlobalConfig GLOBAL(संरचना Sqlite3Config, sqlite3Config)
  पूर्णाङ्क sqlite3_wsd_init(पूर्णाङ्क N, पूर्णाङ्क J);
  शून्य *sqlite3_wsd_find(शून्य *K, पूर्णाङ्क L);
#अन्यथा
  #परिभाषय SQLITE_WSD
  #परिभाषय GLOBAL(t,v) v
  #परिभाषय sqlite3GlobalConfig sqlite3Config
#यदि_अन्तः

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
#परिभाषय अप्रयुक्तम्_PARAMETER(x) (शून्य)(x)
#परिभाषय अप्रयुक्तम्_PARAMETER2(x,y) अप्रयुक्तम्_PARAMETER(x),अप्रयुक्तम्_PARAMETER(y)

/*
** Forward references to structures
*/
प्रकारनिर्देश संरचना AggInfo AggInfo;
प्रकारनिर्देश संरचना AuthContext AuthContext;
प्रकारनिर्देश संरचना AutoincInfo AutoincInfo;
प्रकारनिर्देश संरचना Bitvec Bitvec;
प्रकारनिर्देश संरचना CollSeq CollSeq;
प्रकारनिर्देश संरचना Column Column;
प्रकारनिर्देश संरचना Cte Cte;
प्रकारनिर्देश संरचना CteUse CteUse;
प्रकारनिर्देश संरचना Db Db;
प्रकारनिर्देश संरचना Db_Client_दत्त Db_Client_दत्त;
प्रकारनिर्देश संरचना DbFixer DbFixer;
प्रकारनिर्देश संरचना Schema Schema;
प्रकारनिर्देश संरचना व्यञ्जकम् व्यञ्जकम्;
प्रकारनिर्देश संरचना व्यञ्जकम्_List व्यञ्जकम्_List;
प्रकारनिर्देश संरचना FKey FKey;
प्रकारनिर्देश संरचना FpDecode FpDecode;
प्रकारनिर्देश संरचना FuncDestructor FuncDestructor;
प्रकारनिर्देश संरचना FuncDef FuncDef;
प्रकारनिर्देश संरचना FuncDefHash FuncDefHash;
प्रकारनिर्देश संरचना IdList IdList;
प्रकारनिर्देश संरचना सूचकांक सूचकांक;
प्रकारनिर्देश संरचना अनुक्रमितम्_व्यञ्जकम् अनुक्रमितम्_व्यञ्जकम्;
प्रकारनिर्देश संरचना सूचकांक_Sample सूचकांक_Sample;
प्रकारनिर्देश संरचना कुञ्चिका_Class कुञ्चिका_Class;
प्रकारनिर्देश संरचना कुञ्चिका_Info कुञ्चिका_Info;
प्रकारनिर्देश संरचना Lookaside Lookaside;
प्रकारनिर्देश संरचना LookasideSlot LookasideSlot;
प्रकारनिर्देश संरचना Module Module;
प्रकारनिर्देश संरचना NameContext NameContext;
प्रकारनिर्देश संरचना OnOrUsing OnOrUsing;
प्रकारनिर्देश संरचना Parse Parse;
प्रकारनिर्देश संरचना ParseCleanup ParseCleanup;
प्रकारनिर्देश संरचना PreUpdate PreUpdate;
प्रकारनिर्देश संरचना PrintfArguments PrintfArguments;
प्रकारनिर्देश संरचना RCStr RCStr;
प्रकारनिर्देश संरचना RenameToken RenameToken;
प्रकारनिर्देश संरचना Returning Returning;
प्रकारनिर्देश संरचना Row_समुच्चयः Row_समुच्चयः;
प्रकारनिर्देश संरचना Savepoint Savepoint;
प्रकारनिर्देश संरचना Select Select;
प्रकारनिर्देश संरचना SQLiteThread SQLiteThread;
प्रकारनिर्देश संरचना SelectDest SelectDest;
प्रकारनिर्देश संरचना Subquery Subquery;
प्रकारनिर्देश संरचना SrcItem SrcItem;
प्रकारनिर्देश संरचना SrcList SrcList;
प्रकारनिर्देश संरचना sqlite3_str StrAccum; /* Internal alias for sqlite3_str */
प्रकारनिर्देश संरचना सारणी सारणी;
प्रकारनिर्देश संरचना सारणी_तालकम् सारणी_तालकम्;
प्रकारनिर्देश संरचना Token Token;
प्रकारनिर्देश संरचना वृक्षः_View वृक्षः_View;
प्रकारनिर्देश संरचना Trigger Trigger;
प्रकारनिर्देश संरचना TriggerPrg TriggerPrg;
प्रकारनिर्देश संरचना Trigger_चरणः Trigger_चरणः;
प्रकारनिर्देश संरचना Unpacked_लेखः Unpacked_लेखः;
प्रकारनिर्देश संरचना Upsert Upsert;
प्रकारनिर्देश संरचना VTable VTable;
प्रकारनिर्देश संरचना VtabCtx VtabCtx;
प्रकारनिर्देश संरचना Walker Walker;
प्रकारनिर्देश संरचना WhereInfo WhereInfo;
प्रकारनिर्देश संरचना गवाक्षः गवाक्षः;
प्रकारनिर्देश संरचना With With;


/*
** The bitmask datatype defined below is used for various optimizations.
**
** Changing this from a 64-bit to a 32-bit type limits the number of
** tables in a join to 32 instead of 64.  But it also reduces the size
** of the library by 738 bytes on ix86.
*/
#यदि_परिभाषितम् SQLITE_BITMASK_TYPE
  प्रकारनिर्देश SQLITE_BITMASK_TYPE Bitmask;
#अन्यथा
  प्रकारनिर्देश u64 Bitmask;
#यदि_अन्तः

/*
** The number of bits in a Bitmask.  "BMS" means "BitMask Size".
*/
#परिभाषय BMS  ((पूर्णाङ्क)(परिमाण(Bitmask)*8))

/*
** A bit in a Bitmask
*/
#परिभाषय MASKBIT(n)    (((Bitmask)1)<<(n))
#परिभाषय MASKBIT64(n)  (((u64)1)<<(n))
#परिभाषय MASKBIT32(n)  (((अचिह्नित पूर्णाङ्क)1)<<(n))
#परिभाषय SMASKBIT32(n) ((n)<=31?((अचिह्नित पूर्णाङ्क)1)<<(n):0)
#परिभाषय ALLBITS       ((Bitmask)-1)
#परिभाषय TOPBIT        (((Bitmask)1)<<(BMS-1))

/* A VList object records a mapping between parameters/variables/wildcards
** in the SQL statement (such as $abc, @pqr, or :xyz) and the integer
** variable number associated with that parameter.  See the format description
** on the sqlite3VListAdd() routine for more information.  A VList is really
** just an array of integers.
*/
प्रकारनिर्देश पूर्णाङ्क VList;

/*
** Defer sourcing vdbe.h and btree.h until after the "u8" and
** "BusyHandler" typedefs. vdbe.h also requires a few of the opaque
** pointer types (i.e. FuncDef) defined above.
*/
#अन्तर्भाव "os.h"
#अन्तर्भाव "pager.h"
#अन्तर्भाव "btree.h"
#अन्तर्भाव "vdbe.h"
#अन्तर्भाव "pcache.h"
#अन्तर्भाव "mutex.h"

/* The SQLITE_EXTRA_DURABLE compile-time option used to set the default
** synchronous setting to EXTRA.  It is no longer supported.
*/
#यदि_परिभाषितम् SQLITE_EXTRA_DURABLE
# warning Use SQLITE_DEFAULT_SYNCHRONOUS=3 instead सम्बन्धि SQLITE_EXTRA_DURABLE
# define SQLITE_DEFAULT_SYNCHRONOUS 3
#यदि_अन्तः

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
#यदि_अपरिभाषितम् SQLITE_DEFAULT_SYNCHRONOUS
# define SQLITE_DEFAULT_SYNCHRONOUS 2
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_DEFAULT_WAL_SYNCHRONOUS
# define SQLITE_DEFAULT_WAL_SYNCHRONOUS SQLITE_DEFAULT_SYNCHRONOUS
#यदि_अन्तः

/*
** Each database file to be accessed by the system is an instance
** of the following structure.  There are normally two of these structures
** in the sqlite.aDb[] array.  aDb[0] is the main database file and
** aDb[1] is the database file used to hold temporary tables.  Additional
** databases may be attached.
*/
संरचना Db {
  वर्ण *zDbSName;      /* Name of this database. (schema name, not filename) */
  Btree *pBt;          /* The B*Tree structure for this database file */
  u8 safety_level;     /* How aggressive at syncing data to disk */
  u8 b_Sync_समुच्चयः;         /* True if "PRAGMA synchronous=N" has been run */
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
संरचना Schema {
  पूर्णाङ्क schema_cookie;   /* Database schema version number for this file */
  पूर्णाङ्क iGeneration;     /* Generation counter.  Incremented with each change */
  Hash tblHash;        /* All tables indexed by name */
  Hash idxHash;        /* All (named) indices indexed by name */
  Hash trigHash;       /* All triggers indexed by name */
  Hash fkeyHash;       /* All foreign keys by referenced table name */
  सारणी *pSeqTab;      /* The sqlite_sequence table used by AUTOINCREMENT */
  u8 file_format;      /* Schema format version for this file */
  u8 enc;              /* Text encoding used by this database */
  u16 schemaFlags;     /* Flags associated with this schema */
  पूर्णाङ्क cache_आकार;      /* Number of pages to use in the cache */
};

/*
** These macros can be used to test, set, or clear bits in the
** Db.pSchema->flags field.
*/
#परिभाषय DbHasProperty(D,I,P)     (((D)->aDb[I].pSchema->schemaFlags&(P))==(P))
#परिभाषय DbHasAnyProperty(D,I,P)  (((D)->aDb[I].pSchema->schemaFlags&(P))!=0)
#परिभाषय Db_समुच्चयः_Property(D,I,P)     (D)->aDb[I].pSchema->schemaFlags|=(P)
#परिभाषय DbClearProperty(D,I,P)   (D)->aDb[I].pSchema->schemaFlags&=~(P)

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
#परिभाषय DB_SchemaLoaded    0x0001  /* The schema has been loaded */
#परिभाषय DB_UnresetViews    0x0002  /* Some views have defined column names */
#परिभाषय DB_ResetWanted     0x0008  /* Reset the schema when nSchemaLock==0 */

/*
** The number of different kinds of things that can be limited
** using the sqlite3_limit() interface.
*/
#परिभाषय SQLITE_N_सीमा (SQLITE_सीमा_CTES+1)

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
संरचना Lookaside {
  u32 bDisable;           /* Only operate the lookaside when zero */
  u16 sz;                 /* Size of each buffer in bytes */
  u16 szTrue;             /* True value of sz, even if disabled */
  u8 bMalloced;           /* True if pStart obtained from sqlite3_malloc() */
  u32 nSlot;              /* Number of lookaside slots allocated */
  u32 anStat[3];          /* 0: hits.  1: size misses.  2: full misses */
  LookasideSlot *pInit;   /* List of buffers not previously used */
  LookasideSlot *pFree;   /* List of available buffers */
#यदि_अपरिभाषितम् SQLITE_OMIT_TWOSIZE_LOOKASIDE
  LookasideSlot *pSmallInit; /* List of small buffers not previously used */
  LookasideSlot *pSmallFree; /* List of available small buffers */
  शून्य *p_मध्यः;          /* First byte past end of full-size buffers and
                          ** the first byte of LOOKASIDE_SMALL buffers */
#यदि_अन्तः /* SQLITE_OMIT_TWOSIZE_LOOKASIDE */
  शून्य *pStart;           /* First byte of available memory space */
  शून्य *p_अन्तः;             /* First byte past end of available space */
  शून्य *p_True_अन्तः;         /* True value of pEnd, when db->pnBytesFreed!=0 */
};
संरचना LookasideSlot {
  LookasideSlot *p_अग्र;    /* Next buffer in the list of free buffers */
};

#परिभाषय DisableLookaside  db->lookaside.bDisable++;db->lookaside.sz=0
#परिभाषय EnableLookaside   db->lookaside.bDisable--;\
   db->lookaside.sz=db->lookaside.bDisable?0:db->lookaside.szTrue

/* Size of the smaller allocations in two-size lookaside */
#यदि_परिभाषितम् SQLITE_OMIT_TWOSIZE_LOOKASIDE
#  define LOOKASIDE_SMALL           0
#अन्यथा
#  define LOOKASIDE_SMALL         128
#यदि_अन्तः

/*
** A hash table for built-in function definitions.  (Application-defined
** functions use a regular table table from hash.h.)
**
** Hash each FuncDef structure into one of the FuncDefHash.a[] slots.
** Collisions are on the FuncDef.u.pHash chain.  Use the SQLITE_FUNC_HASH()
** macro to compute a hash on the function name.
*/
#परिभाषय SQLITE_FUNC_HASH_SZ 23
संरचना FuncDefHash {
  FuncDef *a[SQLITE_FUNC_HASH_SZ];       /* Hash table for functions */
};
#परिभाषय SQLITE_FUNC_HASH(C,L) (((C)+(L))%SQLITE_FUNC_HASH_SZ)

/*
** typedef for the authorization callback function.
*/
प्रकारनिर्देश पूर्णाङ्क (*sqlite3_xauth)(शून्य*,पूर्णाङ्क,ध्रुव वर्ण*,ध्रुव वर्ण*,ध्रुव वर्ण*,
                             ध्रुव वर्ण*);

#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
/* This is an extra SQLITE_TRACE macro that indicates "legacy" tracing
** in the style of sqlite3_trace()
*/
#परिभाषय SQLITE_TRACE_LEGACY          0x40     /* Use the legacy xTrace */
#परिभाषय SQLITE_TRACE_XPROFILE        0x80     /* Use the legacy xProfile */
#अन्यथा
#परिभाषय SQLITE_TRACE_LEGACY          0
#परिभाषय SQLITE_TRACE_XPROFILE        0
#यदि_अन्तः /* SQLITE_OMIT_DEPRECATED */
#परिभाषय SQLITE_TRACE_NONLEGACY_आवरणम्  0x0f     /* Normal flags */

/*
** Maximum number of sqlite3.aDb[] entries.  This is the number of attached
** databases plus 2 for "main" and "temp".
*/
#परिभाषय SQLITE_MAX_DB (SQLITE_MAX_ATTACHED+2)

/*
** Each database connection is an instance of the following structure.
*/
संरचना sqlite3 {
  sqlite3_vfs *pVfs;            /* OS Interface */
  संरचना Vdbe *pVdbe;           /* List of active virtual machines */
  CollSeq *pDfltColl;           /* BINARY collseq for the database encoding */
  sqlite3_mutex *mutex;         /* Connection mutex */
  Db *aDb;                      /* All backends */
  पूर्णाङ्क nDb;                      /* Number of backends currently in use */
  u32 mDbFlags;                 /* flags recording internal state */
  u64 flags;                    /* flags settable by pragmas. See below */
  i64 चरमः_Rowid;                /* ROWID of most recent insert (see above) */
  i64 szMmap;                   /* Default mmap_size setting */
  u32 n_Schema_तालकम्;              /* Do not reset the schema when non-zero */
  अचिह्नित पूर्णाङ्क openFlags;       /* Flags passed to sqlite3_vfs.xOpen() */
  पूर्णाङ्क err_संहिता;                  /* Most recent error code (SQLITE_*) */
  पूर्णाङ्क err_Byte_विस्थापनम्;            /* Byte offset of error in SQL statement */
  पूर्णाङ्क err_आवरणम्;                  /* & result codes with this before returning */
  पूर्णाङ्क iSysErrno;                /* Errno value from last system error */
  u8 enc;                       /* Text encoding */
  u8 autoCommit;                /* The auto-commit flag. */
  u8 अस्थायी_store;                /* 1: file 2: memory 0: default */
  u8 mallocFailed;              /* True if we have seen a malloc failure */
  u8 bBenignMalloc;             /* Do not require OOMs if true */
  u8 dflt_तालकम्_बहुलकम्;              /* Default locking-mode for attached dbs */
  चिह्नित वर्ण अग्र_Autovac;      /* Autovac setting after VACUUM if >=0 */
  u8 suppressErr;               /* Do not issue error messages if true */
  u8 vtabOnConflict;            /* Value to return for s3_vtab_on_conflict() */
  u8 isTransactionSavepoint;    /* True if the outermost savepoint is a TS */
  u8 mTrace;                    /* zero or more SQLITE_TRACE flags */
  u8 noSharedCache;             /* True if no shared-cache backends */
  u8 nSqlExec;                  /* Number of pending OP_SqlExec opcodes */
  u8 eOpenState;                /* Current condition of the connection */
  u8 nFpDigit;                  /* Significant digits to keep on double->text */
  पूर्णाङ्क अग्र_Pagesize;             /* Pagesize after VACUUM if >0 */
  i64 nChange;                  /* Value returned by sqlite3_changes() */
  i64 n_योग_Change;             /* Value returned by sqlite3_total_changes() */
  u32 dbOptFlags;               /* Flags to enable/disable optimizations */
  पूर्णाङ्क a_सीमा[SQLITE_N_सीमा];   /* Limits */
  पूर्णाङ्क nMaxSorterMmap;           /* Maximum size of regions mapped by sorter */
  संरचना sqlite3InitInfo {      /* Information used during initialization */
    Pgno newTnum;               /* Rootpage of table being initialized */
    u8 iDb;                     /* Which db file is being initialized */
    u8 busy;                    /* TRUE if initing. 2 if due to ADD COLUMN */
    अचिह्नित orphanTrigger : 1; /* Last statement is orphaned TEMP trigger */
    अचिह्नित imposter_सारणी : 2; /* Building an imposter table */
    अचिह्नित reopenMemdb : 1;   /* ATTACH is really a reopen using MemDB */
    ध्रुव वर्ण **azInit;        /* "type", "name", and "tbl_name" columns */
  } init;
  पूर्णाङ्क n_Vdbe_सक्रिय;              /* Number of VDBEs currently running */
  पूर्णाङ्क nVdbeRead;                /* Number of active VDBEs that read or write */
  पूर्णाङ्क nVdbeWrite;               /* Number of active VDBEs that read and write */
  पूर्णाङ्क nVdbeExec;                /* Number of nested calls to VdbeExec() */
  पूर्णाङ्क nVDestroy;                /* Number of active OP_VDestroy operations */
  पूर्णाङ्क nExtension;               /* Number of loaded extensions */
  शून्य **aExtension;            /* Array of shared library handles */
  सङ्घ {
    शून्य (*xLegacy)(शून्य*,ध्रुव वर्ण*);   /* mTrace==SQLITE_TRACE_LEGACY */
    पूर्णाङ्क (*xV2)(u32,शून्य*,शून्य*,शून्य*);    /* All other mTrace values */
  } trace;
  शून्य *pTraceArg;                        /* Argument to the trace function */
#यदि_अपरिभाषितम् SQLITE_OMIT_DEPRECATED
  शून्य (*xProfile)(शून्य*,ध्रुव वर्ण*,u64);  /* Profiling function */
  शून्य *pProfileArg;                        /* Argument to profile function */
#यदि_अन्तः
  शून्य *pCommitArg;                 /* Argument to xCommitCallback() */
  पूर्णाङ्क (*xCommitCallback)(शून्य*);    /* Invoked at every commit. */
  शून्य *pRollbackArg;               /* Argument to xRollbackCallback() */
  शून्य (*xRollbackCallback)(शून्य*); /* Invoked at every commit. */
  शून्य *pUpdateArg;
  शून्य (*xUpdateCallback)(शून्य*,पूर्णाङ्क, ध्रुव वर्ण*,ध्रुव वर्ण*,sqlite_int64);
  शून्य *p_Autovac_पृष्ठानि_Arg;           /* Client argument to autovac_pages */
  शून्य (*xAutovacDestr)(शून्य*);     /* Destructor for pAutovacPAgesArg */
  अचिह्नित पूर्णाङ्क (*x_Autovac_पृष्ठानि)(शून्य*,ध्रुव वर्ण*,u32,u32,u32);
  Parse *pParse;                /* Current parse */
#यदि_परिभाषितम् SQLITE_ENABLE_PREUPDATE_HOOK
  शून्य *pPreUpdateArg;          /* First argument to xPreUpdateCallback */
  शून्य (*xPreUpdateCallback)(   /* Registered using sqlite3_preupdate_hook() */
    शून्य*,sqlite3*,पूर्णाङ्क,वर्ण ध्रुव*,वर्ण ध्रुव*,sqlite3_int64,sqlite3_int64
  );
  PreUpdate *pPreUpdate;        /* Context for active pre-update callback */
#यदि_अन्तः /* SQLITE_ENABLE_PREUPDATE_HOOK */
#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
  पूर्णाङ्क (*xWalCallback)(शून्य *, sqlite3 *, ध्रुव वर्ण *, पूर्णाङ्क);
  शून्य *pWalArg;
#यदि_अन्तः
  शून्य(*xCollNeeded)(शून्य*,sqlite3*,पूर्णाङ्क eTextRep,ध्रुव वर्ण*);
  शून्य(*xCollNeeded16)(शून्य*,sqlite3*,पूर्णाङ्क eTextRep,ध्रुव शून्य*);
  शून्य *pCollNeededArg;
  sqlite3_मानम् *pErr;          /* Most recent error message */
  सङ्घ {
    चञ्चल पूर्णाङ्क isInterrupted; /* True if sqlite3_interrupt has been called */
    द्विगुण notUsed1;            /* Spacer */
  } u1;
  Lookaside lookaside;          /* Lookaside malloc configuration */
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTHORIZATION
  sqlite3_xauth xAuth;          /* Access authorization function */
  शून्य *pAuthArg;               /* 1st argument to the access auth function */
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_प्रगतिः_CALLBACK
  पूर्णाङ्क (*x_प्रगतिः)(शून्य *);     /* The progress callback */
  शून्य *p_प्रगतिः_Arg;           /* Argument to the progress callback */
  अचिह्नित n_प्रगतिः_Ops;        /* Number of opcodes for progress callback */
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
  पूर्णाङ्क nVTrans;                  /* Allocated size of aVTrans */
  Hash aModule;                 /* populated by sqlite3_create_module() */
  VtabCtx *pVtabCtx;            /* Context for active vtab connect/create */
  VTable **aVTrans;             /* Virtual tables with open transactions */
  VTable *pDisconnect;          /* Disconnect these in next sqlite3_prepare() */
#यदि_अन्तः
  Hash aFunc;                   /* Hash table of connection functions */
  Hash aCollSeq;                /* All collating sequences */
  BusyHandler busyHandler;      /* Busy callback */
  Db aDbStatic[2];              /* Static space for the 2 default backends */
  Savepoint *pSavepoint;        /* List of active savepoints */
  पूर्णाङ्क n_Analysis_सीमा;           /* Number of index rows to ANALYZE */
  पूर्णाङ्क busyTimeout;              /* Busy handler timeout, in msec */
#यदि_परिभाषितम् SQLITE_ENABLE_SETLK_TIMEOUT
  पूर्णाङ्क setlkTimeout;             /* Blocking lock timeout, in msec. -1 -> inf. */
  पूर्णाङ्क setlkFlags;               /* Flags passed to setlk_timeout() */
#यदि_अन्तः
  पूर्णाङ्क nSavepoint;               /* Number of non-transaction savepoints */
  पूर्णाङ्क nStatement;               /* Number of nested statement-transactions  */
  i64 nDeferredCons;            /* Net deferred constraints this transaction. */
  i64 nDeferredImmCons;         /* Net deferred immediate constraints */
  पूर्णाङ्क *pn_अष्टकानि_Freed;            /* If not NULL, increment this in DbFree() */
  Db_Client_दत्त *p_Db_दत्त;        /* sqlite3_set_clientdata() content */
  u64 nSpill;                   /* TEMP content spilled to disk */
#यदि_परिभाषितम् SQLITE_ENABLE_UNLOCK_NOTIFY
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
  शून्य *pUnlockArg;                     /* Argument to xUnlockNotify */
  शून्य (*xUnlockNotify)(शून्य **, पूर्णाङ्क);  /* Unlock notify callback */
  sqlite3 *p_अग्र_Blocked;        /* Next in list of all blocked connections */
#यदि_अन्तः
};

/*
** A macro to discover the encoding of a database.
*/
#परिभाषय SCHEMA_ENC(db) ((db)->aDb[0].pSchema->enc)
#परिभाषय ENC(db)        ((db)->enc)

/*
** A u64 constant where the lower 32 bits are all zeros.  Only the
** upper 32 bits are included in the argument.  Necessary because some
** C-compilers still do not accept LL integer literals.
*/
#परिभाषय HI(X)  ((u64)(X)<<32)

/*
** Possible values for the sqlite3.flags.
**
** Value constraints (enforced via assert()):
**      SQLITE_FullFSync     == PAGER_FULLFSYNC
**      SQLITE_CkptFullFSync == PAGER_CKPT_FULLFSYNC
**      SQLITE_CacheSpill    == PAGER_CACHE_SPILL
*/
#परिभाषय SQLITE_WriteSchema    0x00000001  /* OK to update SQLITE_SCHEMA */
#परिभाषय SQLITE_LegacyFileFmt  0x00000002  /* Create new databases in format 1 */
#परिभाषय SQLITE_पूर्णम्_Col_नामानि   0x00000004  /* Show full column names on SELECT */
#परिभाषय SQLITE_पूर्णम्_FSync      0x00000008  /* Use full fsync on the backend */
#परिभाषय SQLITE_Ckpt_पूर्णम्_FSync  0x00000010  /* Use full fsync for checkpoint */
#परिभाषय SQLITE_CacheSpill     0x00000020  /* OK to spill pager cache */
#परिभाषय SQLITE_Short_Col_नामानि  0x00000040  /* Show short columns names */
#परिभाषय SQLITE_TrustedSchema  0x00000080  /* Allow unsafe functions and
                                          ** vtabs in the schema definition */
#परिभाषय SQLITE_NullCallback   0x00000100  /* Invoke the callback once if the */
                                          /*   result set is empty */
#परिभाषय SQLITE_IgnoreChecks   0x00000200  /* Do not enforce check constraints */
#परिभाषय SQLITE_Stmt_अवलोकनम्_स्थितिः 0x00000400  /* Enable stmt_scanstats() counters */
#परिभाषय SQLITE_NoCkptOnClose  0x00000800  /* No checkpoint on close()/DETACH */
#परिभाषय SQLITE_Reverse_क्रमः   0x00001000  /* Reverse unordered SELECTs */
#परिभाषय SQLITE_RecTriggers    0x00002000  /* Enable recursive triggers */
#परिभाषय SQLITE_Foreign_कुञ्चिकाः    0x00004000  /* Enforce foreign key constraints  */
#परिभाषय SQLITE_Auto_सूचकांक      0x00008000  /* Enable automatic indexes */
#परिभाषय SQLITE_आरोपय_Extension  0x00010000  /* Enable load_extension */
#परिभाषय SQLITE_आरोपय_Ext_Func    0x00020000  /* Enable load_extension() SQL func */
#परिभाषय SQLITE_EnableTrigger  0x00040000  /* True to enable triggers */
#परिभाषय SQLITE_DeferFKs       0x00080000  /* Defer all FK constraints */
#परिभाषय SQLITE_QueryOnly      0x00100000  /* Disable database changes */
#परिभाषय SQLITE_Cell_आकार_Ck     0x00200000  /* Check btree cell sizes on load */
#परिभाषय SQLITE_Fts3Tokenizer  0x00400000  /* Enable fts3_tokenizer(2) */
#परिभाषय SQLITE_EnableQPSG     0x00800000  /* Query Planner Stability Guarantee*/
#परिभाषय SQLITE_TriggerEQP     0x01000000  /* Show trigger EXPLAIN QUERY PLAN */
#परिभाषय SQLITE_ResetDatabase  0x02000000  /* Reset the database */
#परिभाषय SQLITE_LegacyAlter    0x04000000  /* Legacy ALTER TABLE behaviour */
#परिभाषय SQLITE_NoSchemaError  0x08000000  /* Do not report schema parse errors*/
#परिभाषय SQLITE_Defensive      0x10000000  /* Input SQL is likely hostile */
#परिभाषय SQLITE_DqsDDL         0x20000000  /* dbl-quoted strings allowed in DDL*/
#परिभाषय SQLITE_DqsDML         0x40000000  /* dbl-quoted strings allowed in DML*/
#परिभाषय SQLITE_EnableView     0x80000000  /* Enable the use of views */
#परिभाषय SQLITE_गणनम्_पङ्क्तिकाः      HI(0x00001) /* Count rows changed by INSERT, */
                                          /*   DELETE, or UPDATE and return */
                                          /*   the count using a callback. */
#परिभाषय SQLITE_CorruptRdOnly  HI(0x00002) /* Prohibit writes due to error */
#परिभाषय SQLITE_ReadUncommit   HI(0x00004) /* READ UNCOMMITTED in shared-cache */
#परिभाषय SQLITE_FkNoAction     HI(0x00008) /* Treat all FK as NO ACTION */
#परिभाषय SQLITE_AttachCreate   HI(0x00010) /* ATTACH allowed to create new dbs */
#परिभाषय SQLITE_AttachWrite    HI(0x00020) /* ATTACH allowed to open for write */
#परिभाषय SQLITE_Comments       HI(0x00040) /* Enable SQL comments */

/* Flags used only if debugging */
#यदि_परिभाषितम् SQLITE_DEBUG
#परिभाषय SQLITE_SqlTrace       HI(0x0100000) /* Debug print SQL as it executes */
#परिभाषय SQLITE_VdbeListing    HI(0x0200000) /* Debug listings of VDBE progs */
#परिभाषय SQLITE_VdbeTrace      HI(0x0400000) /* True to trace VDBE execution */
#परिभाषय SQLITE_VdbeAddopTrace HI(0x0800000) /* Trace sqlite3VdbeAddOp() calls */
#परिभाषय SQLITE_VdbeEQP        HI(0x1000000) /* Debug EXPLAIN QUERY PLAN */
#परिभाषय SQLITE_ParserTrace    HI(0x2000000) /* PRAGMA parser_trace=ON */
#यदि_अन्तः

/*
** Allowed values for sqlite3.mDbFlags
*/
#परिभाषय DBFLAG_SchemaChange   0x0001  /* Uncommitted Hash table changes */
#परिभाषय DBFLAG_PreferBuiltin  0x0002  /* Preference to built-in funcs */
#परिभाषय DBFLAG_Vacuum         0x0004  /* Currently in a VACUUM */
#परिभाषय DBFLAG_VacuumInto     0x0008  /* Currently running VACUUM INTO */
#परिभाषय DBFLAG_Schema_ज्ञातम्_Ok  0x0010  /* Schema is known to be valid */
#परिभाषय DBFLAG_आन्तरिकम्_Func   0x0020  /* Allow use of internal functions */
#परिभाषय DBFLAG_EncodingFixed  0x0040  /* No longer possible to change enc. */

/*
** Bits of the sqlite3.dbOptFlags field that are used by the
** sqlite3_test_control(SQLITE_TESTCTRL_OPTIMIZATIONS,...) interface to
** selectively disable various optimizations. The numbers in each macro
** are the one-based bit position (for improved readability).
**
** Keep these values in sync with the ext/misc/optname.c extension.
*/
#परिभाषय SQLITE_QueryFlattener  1 /* Query flattening */
#परिभाषय SQLITE_गवाक्षः_Func      2 /* Use xInverse for window functions */
#परिभाषय SQLITE_गणः_By_क्रमः    3 /* GROUPBY cover of ORDERBY */
#परिभाषय SQLITE_Join_अलब्धिः        4 /* Unwind join loops on a lookup miss*/
#परिभाषय SQLITE_DistinctOpt     5 /* DISTINCT using indexes */
#परिभाषय SQLITE_Cover_Idx_अवलोकनम्    6 /* Covering index scans */
#परिभाषय SQLITE_क्रमः_By_Idx_Join  7 /* ORDER BY of joins via index */
#परिभाषय SQLITE_संक्रामकम्      8 /* Transitive constraints */
#परिभाषय SQLITE_OmitNoopJoin    9 /* Omit unused tables in joins */
#परिभाषय SQLITE_गणनम्_सम्बन्धि_View    10 /* The count-of-view optimization */
#परिभाषय SQLITE_CursorHints    11 /* Add OP_CursorHint opcodes */
#परिभाषय SQLITE_Stat4          12 /* Use STAT4 data */
#परिभाषय SQLITE_प्रक्षिप_अधस्तात्       13 /* WHERE-clause push-down opt */
#परिभाषय SQLITE_SimplifyJoin   14 /* Convert LEFT JOIN to JOIN */
#परिभाषय SQLITE_Skip_अवलोकनम्       15 /* Skip-scans */
#परिभाषय SQLITE_PropagateConst 16 /* The constant propagation opt */
#परिभाषय SQLITE_MinMaxOpt      17 /* The min/max optimization */
#परिभाषय SQLITE_Seek_अवलोकनम्       18 /* The OP_SeekScan optimization */
#परिभाषय SQLITE_Omit_क्रमः_By    19 /* Omit pointless ORDER BY */
#परिभाषय SQLITE_BloomFilter    20 /* Use a Bloom filters */
                          /*  21 -- Available for reuse */
#परिभाषय SQLITE_BalancedMerge  22 /* Balance multi-way merges */
#परिभाषय SQLITE_ReleaseReg     23 /* Use OP_ReleaseReg for testing */
#परिभाषय SQLITE_FlttnUnionAll  24 /* Disable the UNION ALL flattener */
#परिभाषय SQLITE_अनुक्रमितम्_व्यञ्जकम्    25 /* Pull exprs from index when able */
#परिभाषय SQLITE_Coroutines     26 /* Co-routines for subqueries */
#परिभाषय SQLITE_Null_अप्रयुक्तम्_स्तम्भसङ्ख्या 27 /* NULL unused columns in subqueries */
#परिभाषय SQLITE_OnePass        28 /* Single-pass DELETE and UPDATE */
#परिभाषय SQLITE_क्रमः_By_Subq    29 /* ORDER BY in subquery helps outer */
#परिभाषय SQLITE_तारकम्_Query      30 /* Heurists for star queries */
#परिभाषय SQLITE_Exists_गन्तव्यम्_Join   31 /* The EXISTS-to-JOIN optimization */
#परिभाषय SQLITE_Union_सीमा     32 /* Optimizations for UNION + LIMIT */

/*
** Macros for testing whether or not optimizations are enabled or disabled.
*/
#परिभाषय OptimizationDisabled(D,M)  (((D)->dbOptFlags&(1ul<<((M)-1)))!=0)
#परिभाषय OptimizationEnabled(D,M)   (((D)->dbOptFlags&(1ul<<((M)-1)))==0)

/*
** Return true if it OK to factor constant expressions into the initialization
** code. The argument is a Parse object for the code generator.
*/
#परिभाषय Const_गुणकः_Ok(P) ((P)->ok_Const_गुणकः)

/* Possible values for the sqlite3.eOpenState field.
** The numbers are randomly selected such that a minimum of three bits must
** change to convert any number to another or to zero
*/
#परिभाषय SQLITE_STATE_OPEN     0x76  /* Database is open */
#परिभाषय SQLITE_STATE_CLOSED   0xce  /* Database is closed */
#परिभाषय SQLITE_STATE_SICK     0xba  /* Error and awaiting close */
#परिभाषय SQLITE_STATE_BUSY     0x6d  /* Database currently in use */
#परिभाषय SQLITE_STATE_ERROR    0xd5  /* An SQLITE_MISUSE error occurred */
#परिभाषय SQLITE_STATE_ZOMBIE   0xa7  /* Close with last statement close */

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
संरचना FuncDef {
  i16 nArg;            /* Number of arguments.  -1 means unlimited */
  u32 funcFlags;       /* Some combination of SQLITE_FUNC_* */
  शून्य *p_User_दत्त;     /* User data parameter */
  FuncDef *p_अग्र;      /* Next function with same name */
  शून्य (*xSFunc)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम्**); /* func or agg-step */
  शून्य (*xFinalize)(sqlite3_context*);                  /* Agg finalizer */
  शून्य (*x_मानम्)(sqlite3_context*);                     /* Current agg value */
  शून्य (*xInverse)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम्**); /* inverse agg-step */
  ध्रुव वर्ण *zName;   /* SQL name of the function. */
  सङ्घ {
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
संरचना FuncDestructor {
  पूर्णाङ्क nRef;
  शून्य (*xDestroy)(शून्य *);
  शून्य *p_User_दत्त;
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
#परिभाषय SQLITE_FUNC_ENCMASK  0x0003 /* SQLITE_UTF8, SQLITE_UTF16BE or UTF16LE */
#परिभाषय SQLITE_FUNC_LIKE     0x0004 /* Candidate for the LIKE optimization */
#परिभाषय SQLITE_FUNC_CASE     0x0008 /* Case-sensitive LIKE-type function */
#परिभाषय SQLITE_FUNC_EPHEM    0x0010 /* Ephemeral.  Delete with VDBE */
#परिभाषय SQLITE_FUNC_NEEDCOLL 0x0020 /* sqlite3GetFuncCollSeq() might be called*/
#परिभाषय SQLITE_FUNC_दीर्घता   0x0040 /* Built-in length() function */
#परिभाषय SQLITE_FUNC_TYPEOF   0x0080 /* Built-in typeof() function */
#परिभाषय SQLITE_FUNC_BYTELEN  0x00c0 /* Built-in octet_length() function */
#परिभाषय SQLITE_FUNC_गणनम्    0x0100 /* Built-in count(*) aggregate */
/*                           0x0200 -- available for reuse */
#परिभाषय SQLITE_FUNC_UNLIKELY 0x0400 /* Built-in unlikely() function */
#परिभाषय SQLITE_FUNC_CONSTANT 0x0800 /* Constant inputs give a constant output */
#परिभाषय SQLITE_FUNC_MINMAX   0x1000 /* True for min() and max() aggregates */
#परिभाषय SQLITE_FUNC_SLOCHNG  0x2000 /* "Slow Change". Value constant during a
                                    ** single query - might change over time */
#परिभाषय SQLITE_FUNC_TEST     0x4000 /* Built-in testing functions */
#परिभाषय SQLITE_FUNC_RUNONLY  0x8000 /* Cannot be used by valueFromFunction */
#परिभाषय SQLITE_FUNC_गवाक्षः   0x00010000 /* Built-in window-only function */
#परिभाषय SQLITE_FUNC_आन्तरिकम् 0x00040000 /* For use by NestedParse() only */
#परिभाषय SQLITE_FUNC_प्रत्यक्षम्   0x00080000 /* Not for use in TRIGGERs or VIEWs */
/* SQLITE_SUBTYPE            0x00100000 // Consumer of subtypes */
#परिभाषय SQLITE_FUNC_UNSAFE   0x00200000 /* Function has side effects */
#परिभाषय SQLITE_FUNC_INLINE   0x00400000 /* Functions implemented in-line */
#परिभाषय SQLITE_FUNC_BUILTIN  0x00800000 /* This is a built-in function */
/*  SQLITE_RESULT_SUBTYPE    0x01000000 // Generator of subtypes */
#परिभाषय SQLITE_FUNC_ANYORDER 0x08000000 /* count/min/max aggregate */

/* Identifier numbers for each in-line function */
#परिभाषय INLINEFUNC_coalesce             0
#परिभाषय INLINEFUNC_उपपादयति_nonnull_row  1
#परिभाषय INLINEFUNC_व्यञ्जकम्_उपपादयति_व्यञ्जकम्    2
#परिभाषय INLINEFUNC_व्यञ्जकम्_compare         3
#परिभाषय INLINEFUNC_affinity             4
#परिभाषय INLINEFUNC_iif                  5
#परिभाषय INLINEFUNC_sqlite_विस्थापनम्        6
#परिभाषय INLINEFUNC_unlikely            99  /* Default case */

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
#परिभाषय FUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_CONSTANT|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_गन्तव्यम्_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#परिभाषय VFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   SQLITE_INT_गन्तव्यम्_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#परिभाषय SFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|SQLITE_DIRECTONLY|SQLITE_FUNC_UNSAFE, \
   SQLITE_INT_गन्तव्यम्_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#परिभाषय MFUNCTION(zName, nArg, xPtr, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_CONSTANT|SQLITE_UTF8, \
   xPtr, 0, xFunc, 0, 0, 0, #zName, {0} }
#परिभाषय JFUNCTION(zName, nArg, bUseCache, bWS, bRS, bJsonB, iArg, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_DETERMINISTIC|SQLITE_FUNC_CONSTANT|\
   SQLITE_UTF8|((bUseCache)*SQLITE_FUNC_RUNONLY)|\
   ((bRS)*SQLITE_SUBTYPE)|((bWS)*SQLITE_परिणाम_SUBTYPE), \
   SQLITE_INT_गन्तव्यम्_PTR(iArg|((bJsonB)*JSON_BLOB)),0,xFunc,0, 0, 0, #zName, {0} }
#परिभाषय INLINE_FUNC(zName, nArg, iArg, mFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_UTF8|SQLITE_FUNC_INLINE|SQLITE_FUNC_CONSTANT|(mFlags), \
   SQLITE_INT_गन्तव्यम्_PTR(iArg), 0, noopFunc, 0, 0, 0, #zName, {0} }
#परिभाषय TEST_FUNC(zName, nArg, iArg, mFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
         SQLITE_UTF8|SQLITE_FUNC_आन्तरिकम्|SQLITE_FUNC_TEST| \
         SQLITE_FUNC_INLINE|SQLITE_FUNC_CONSTANT|(mFlags), \
   SQLITE_INT_गन्तव्यम्_PTR(iArg), 0, noopFunc, 0, 0, 0, #zName, {0} }
#परिभाषय DFUNCTION(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_SLOCHNG|SQLITE_UTF8, \
   0, 0, xFunc, 0, 0, 0, #zName, {0} }
#परिभाषय विशुद्धम्_DATE(zName, nArg, iArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
         SQLITE_FUNC_SLOCHNG|SQLITE_UTF8|SQLITE_FUNC_CONSTANT, \
   (शून्य*)&sqlite3Config, 0, xFunc, 0, 0, 0, #zName, {0} }
#परिभाषय FUNCTION2(zName, nArg, iArg, bNC, xFunc, extraFlags) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_CONSTANT|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL)|extraFlags,\
   SQLITE_INT_गन्तव्यम्_PTR(iArg), 0, xFunc, 0, 0, 0, #zName, {0} }
#परिभाषय STR_FUNCTION(zName, nArg, pArg, bNC, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_SLOCHNG|SQLITE_UTF8|(bNC*SQLITE_FUNC_NEEDCOLL), \
   pArg, 0, xFunc, 0, 0, 0, #zName, {0} }
#परिभाषय LIKEFUNC(zName, nArg, arg, flags) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_FUNC_CONSTANT|SQLITE_UTF8|flags, \
   (शून्य *)arg, 0, likeFunc, 0, 0, 0, #zName, {0} }
#परिभाषय WAGGREGATE(zName, nArg, arg, nc, x_चरणः, xFinal, x_मानम्, xInverse, f) \
  {nArg, SQLITE_FUNC_BUILTIN|SQLITE_UTF8|(nc*SQLITE_FUNC_NEEDCOLL)|f, \
   SQLITE_INT_गन्तव्यम्_PTR(arg), 0, x_चरणः,xFinal,x_मानम्,xInverse,#zName, {0}}
#परिभाषय आन्तरिकम्_FUNCTION(zName, nArg, xFunc) \
  {nArg, SQLITE_FUNC_BUILTIN|\
   SQLITE_FUNC_आन्तरिकम्|SQLITE_UTF8|SQLITE_FUNC_CONSTANT, \
   0, 0, xFunc, 0, 0, 0, #zName, {0} }


/*
** All current savepoints are stored in a linked list starting at
** sqlite3.pSavepoint. The first element in the list is the most recently
** opened savepoint. Savepoints are added to the list by the vdbe
** OP_Savepoint instruction.
*/
संरचना Savepoint {
  वर्ण *zName;                        /* Savepoint name (nul-terminated) */
  i64 nDeferredCons;                  /* Number of deferred fk violations */
  i64 nDeferredImmCons;               /* Number of deferred imm fk. */
  Savepoint *p_अग्र;                   /* Parent savepoint (if any) */
};

/*
** The following are used as the second parameter to sqlite3Savepoint(),
** and as the P1 argument to the OP_Savepoint instruction.
*/
#परिभाषय SAVEPOINT_BEGIN      0
#परिभाषय SAVEPOINT_RELEASE    1
#परिभाषय SAVEPOINT_ROLLBACK   2


/*
** Each SQLite module (virtual table definition) is defined by an
** instance of the following structure, stored in the sqlite3.aModule
** hash table.
*/
संरचना Module {
  ध्रुव sqlite3_module *pModule;       /* Callback pointers */
  ध्रुव वर्ण *zName;                   /* Name passed to create_module() */
  पूर्णाङ्क nRefModule;                      /* Number of pointers to this object */
  शून्य *pAux;                          /* pAux passed to create_module() */
  शून्य (*xDestroy)(शून्य *);            /* Module destructor function */
  सारणी *pEpoTab;                      /* Eponymous table for this module */
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
संरचना Column {
  वर्ण *zCnName;        /* Name of this column */
  अचिह्नित notNull :4;  /* An OE_ code for handling a NOT NULL constraint */
  अचिह्नित eCType :4;   /* One of the standard types */
  वर्ण affinity;        /* One of the SQLITE_AFF_... values */
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
#परिभाषय COLTYPE_CUSTOM      0   /* Type appended to zName */
#परिभाषय COLTYPE_ANY         1
#परिभाषय COLTYPE_BLOB        2
#परिभाषय COLTYPE_INT         3
#परिभाषय COLTYPE_INTEGER     4
#परिभाषय COLTYPE_REAL        5
#परिभाषय COLTYPE_TEXT        6
#परिभाषय SQLITE_N_STDTYPE    6  /* Number of standard types */

/* Allowed values for Column.colFlags.
**
** Constraints:
**         TF_HasVirtual == COLFLAG_VIRTUAL
**         TF_HasStored  == COLFLAG_STORED
**         TF_HasHidden  == COLFLAG_HIDDEN
*/
#परिभाषय COLFLAG_PRIMKEY   0x0001   /* Column is part of the primary key */
#परिभाषय COLFLAG_HIDDEN    0x0002   /* A hidden column in a virtual table */
#परिभाषय COLFLAG_HASTYPE   0x0004   /* Type name follows column name */
#परिभाषय COLFLAG_UNIQUE    0x0008   /* Column def contains "UNIQUE" or "PK" */
#परिभाषय COLFLAG_SORTERREF 0x0010   /* Use sorter-refs with this column */
#परिभाषय COLFLAG_VIRTUAL   0x0020   /* GENERATED ALWAYS AS ... VIRTUAL */
#परिभाषय COLFLAG_STORED    0x0040   /* GENERATED ALWAYS AS ... STORED */
#परिभाषय COLFLAG_NOTAVAIL  0x0080   /* STORED column not yet calculated */
#परिभाषय COLFLAG_BUSY      0x0100   /* Blocks recursion on GENERATED columns */
#परिभाषय COLFLAG_HASCOLL   0x0200   /* Has collating sequence name in zCnName */
#परिभाषय COLFLAG_NOEXPAND  0x0400   /* Omit this column when expanding "*" */
#परिभाषय COLFLAG_GENERATED 0x0060   /* Combo: _STORED, _VIRTUAL */
#परिभाषय COLFLAG_NOINSERT  0x0062   /* Combo: _HIDDEN, _STORED, _VIRTUAL */

/*
** A "Collating Sequence" is defined by an instance of the following
** structure. Conceptually, a collating sequence consists of a name and
** a comparison routine that defines the order of that sequence.
**
** If CollSeq.xCmp is NULL, it means that the
** collating sequence is undefined.  Indices built on an undefined
** collating sequence may not be read or written.
*/
संरचना CollSeq {
  वर्ण *zName;          /* Name of the collating sequence, UTF-8 encoded */
  u8 enc;               /* Text encoding handled by xCmp() */
  शून्य *pUser;          /* First argument to xCmp() */
  पूर्णाङ्क (*xCmp)(शून्य*,पूर्णाङ्क, ध्रुव शून्य*, पूर्णाङ्क, ध्रुव शून्य*);
  शून्य (*xDel)(शून्य*);  /* Destructor for pUser */
};

/*
** A sort order can be either ASC or DESC.
*/
#परिभाषय SQLITE_SO_ASC       0  /* Sort in ascending order */
#परिभाषय SQLITE_SO_DESC      1  /* Sort in ascending order */
#परिभाषय SQLITE_SO_UNDEFINED -1 /* No sort order specified */

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
#परिभाषय SQLITE_AFF_NONE     0x40  /* '@' */
#परिभाषय SQLITE_AFF_BLOB     0x41  /* 'A' */
#परिभाषय SQLITE_AFF_TEXT     0x42  /* 'B' */
#परिभाषय SQLITE_AFF_NUMERIC  0x43  /* 'C' */
#परिभाषय SQLITE_AFF_INTEGER  0x44  /* 'D' */
#परिभाषय SQLITE_AFF_REAL     0x45  /* 'E' */
#परिभाषय SQLITE_AFF_FLEXNUM  0x46  /* 'F' */
#परिभाषय SQLITE_AFF_DEFER    0x58  /* 'X'  - defer computation until later */

#परिभाषय sqlite3IsNumericAffinity(X)  ((X)>=SQLITE_AFF_NUMERIC)

/*
** The SQLITE_AFF_MASK values masks off the significant bits of an
** affinity value.
*/
#परिभाषय SQLITE_AFF_आवरणम्     0x47

/*
** Additional bit values that can be ORed with an affinity without
** changing the affinity.
**
** The SQLITE_NOTNULL flag is a combination of NULLEQ and JUMPIFNULL.
** It causes an assert() to fire if either operand to a comparison
** operator is NULL.  It is added to certain comparison operators to
** prove that the operands are always NOT NULL.
*/
#परिभाषय SQLITE_JUMPIFNULL   0x10  /* jumps if either operand is NULL */
#परिभाषय SQLITE_NULLEQ       0x80  /* NULL=NULL */
#परिभाषय SQLITE_NOTNULL      0x90  /* Assert that operands are never NULL */

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
संरचना VTable {
  sqlite3 *db;              /* Database connection associated with this table */
  Module *pMod;             /* Pointer to module implementation */
  sqlite3_vtab *pVtab;      /* Pointer to vtab instance */
  पूर्णाङ्क nRef;                 /* Number of pointers to this structure */
  u8 bConstraint;           /* True if constraints are supported */
  u8 bAllSchemas;           /* True if might use any attached schema */
  u8 eVtabRisk;             /* Riskiness of allowing hacker access */
  पूर्णाङ्क iSavepoint;           /* Depth of the SAVEPOINT stack */
  VTable *p_अग्र;            /* Next in linked list (see above) */
};

/* Allowed values for VTable.eVtabRisk
*/
#परिभाषय SQLITE_VTABRISK_नीचः          0
#परिभाषय SQLITE_VTABRISK_प्रसामान्यम्       1
#परिभाषय SQLITE_VTABRISK_उच्चः         2

/*
** The schema for each SQL table, virtual table, and view is represented
** in memory by an instance of the following structure.
*/
संरचना सारणी {
  वर्ण *zName;         /* Name of the table or view */
  Column *aCol;        /* Information about each column */
  सूचकांक *p_सूचकांक;       /* List of SQL indexes on this table. */
  वर्ण *zColAff;       /* String defining the affinity of each column */
  व्यञ्जकम्_List *pCheck;    /* All CHECK constraints */
                       /*   ... also used as column name list in a VIEW */
  Pgno tnum;           /* Root BTree page for this table */
  u32 nTabRef;         /* Number of pointers to this Table */
  u32 tabFlags;        /* Mask of TF_* values */
  i16 iPKey;           /* If not negative, use aCol[iPKey] as the rowid */
  i16 nCol;            /* Number of columns in this table */
  i16 nNVCol;          /* Number of columns that are not VIRTUAL */
  LogEst nRowLogEst;   /* Estimated rows in table - from sqlite_stat1 table */
  LogEst szTabRow;     /* Estimated size of each table row in bytes */
#यदि_परिभाषितम् SQLITE_ENABLE_COSTMULT
  LogEst costMult;     /* Cost multiplier for using this table */
#यदि_अन्तः
  u8 कुञ्चिका_Conf;          /* What to do in case of uniqueness conflict on iPKey */
  u8 eTabType;         /* 0: normal, 1: virtual, 2: view */
  सङ्घ {
    संरचना {             /* Used by ordinary tables: */
      पूर्णाङ्क सङ्कलय_Col_विस्थापनम्;    /* Offset in CREATE TABLE stmt to add a new column */
      FKey *pFKey;         /* Linked list of all foreign keys in this table */
      व्यञ्जकम्_List *pDfltList; /* DEFAULT clauses on various columns.
                           ** Or the AS clause for generated columns. */
    } tab;
    संरचना {             /* Used by views: */
      Select *pSelect;     /* View definition */
    } view;
    संरचना {             /* Used by virtual tables only: */
      पूर्णाङ्क nArg;            /* Number of arguments to the module */
      वर्ण **azArg;        /* 0: module 1: schema 2: vtab name 3...: args */
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
#परिभाषय TF_Readonly       0x00000001 /* Read-only system table */
#परिभाषय TF_HasHidden      0x00000002 /* Has one or more hidden columns */
#परिभाषय TF_Has_Primary_कुञ्चिका  0x00000004 /* Table has a primary key */
#परिभाषय TF_Autoincrement  0x00000008 /* Integer primary key is autoincrement */
#परिभाषय TF_HasStat1       0x00000010 /* nRowLogEst set from sqlite_stat1 */
#परिभाषय TF_HasVirtual     0x00000020 /* Has one or more VIRTUAL columns */
#परिभाषय TF_HasStored      0x00000040 /* Has one or more STORED columns */
#परिभाषय TF_HasGenerated   0x00000060 /* Combo: HasVirtual + HasStored */
#परिभाषय TF_WithoutRowid   0x00000080 /* No rowid.  PRIMARY KEY is the key */
#परिभाषय TF_MaybeReanalyze 0x00000100 /* Maybe run ANALYZE on this table */
#परिभाषय TF_NoVisibleRowid 0x00000200 /* No user-visible "rowid" column */
#परिभाषय TF_OOOHidden      0x00000400 /* Out-of-Order hidden columns */
#परिभाषय TF_HasNotNull     0x00000800 /* Contains NOT NULL constraints */
#परिभाषय TF_Shadow         0x00001000 /* True for a shadow table */
#परिभाषय TF_HasStat4       0x00002000 /* STAT4 info available for this table */
#परिभाषय TF_Ephemeral      0x00004000 /* An ephemeral table */
#परिभाषय TF_Eponymous      0x00008000 /* An eponymous virtual table */
#परिभाषय TF_Strict         0x00010000 /* STRICT mode */
#परिभाषय TF_Imposter       0x00020000 /* An imposter table */

/*
** Allowed values for Table.eTabType
*/
#परिभाषय TABTYP_NORM      0     /* Ordinary table */
#परिभाषय TABTYP_VTAB      1     /* Virtual table */
#परिभाषय TABTYP_VIEW      2     /* A view */

#परिभाषय IsView(X)           ((X)->eTabType==TABTYP_VIEW)
#परिभाषय Is_Ordinary_सारणी(X)  ((X)->eTabType==TABTYP_NORM)

/*
** Test to see whether or not a table is a virtual table.  This is
** done as a macro so that it will be optimized out when virtual
** table support is omitted from the build.
*/
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
#  define IsVirtual(X)      ((X)->eTabType==TABTYP_VTAB)
#  define व्यञ्जकम्_Is_Vtab(X)  \
   ((X)->op==TK_COLUMN && (X)->y.pTab->eTabType==TABTYP_VTAB)
#अन्यथा
#  define IsVirtual(X)      0
#  define व्यञ्जकम्_Is_Vtab(X)     0
#यदि_अन्तः

/*
** Macros to determine if a column is hidden.  IsOrdinaryHiddenColumn()
** only works for non-virtual tables (ordinary tables and views) and is
** always false unless SQLITE_ENABLE_HIDDEN_COLUMNS is defined.  The
** IsHiddenColumn() macro is general purpose.
*/
#यदि defined(SQLITE_ENABLE_HIDDEN_COLUMNS)
#  define IsHiddenColumn(X)         (((X)->colFlags & COLFLAG_HIDDEN)!=0)
#  define IsOrdinaryHiddenColumn(X) (((X)->colFlags & COLFLAG_HIDDEN)!=0)
#नोचेद्यदि !defined(SQLITE_OMIT_VIRTUALTABLE)
#  define IsHiddenColumn(X)         (((X)->colFlags & COLFLAG_HIDDEN)!=0)
#  define IsOrdinaryHiddenColumn(X) 0
#अन्यथा
#  define IsHiddenColumn(X)         0
#  define IsOrdinaryHiddenColumn(X) 0
#यदि_अन्तः


/* Does the table have a rowid */
#परिभाषय HasRowid(X)     (((X)->tabFlags & TF_WithoutRowid)==0)
#परिभाषय VisibleRowid(X) (((X)->tabFlags & TF_NoVisibleRowid)==0)

/* Macro is true if the SQLITE_ALLOW_ROWID_IN_VIEW (mis-)feature is
** available.  By default, this macro is false
*/
#यदि_अपरिभाषितम् SQLITE_ALLOW_ROWID_अन्तर्_VIEW
# define ViewCanHaveRowid     0
#अन्यथा
# define ViewCanHaveRowid     (sqlite3Config.mNoVisibleRowid==0)
#यदि_अन्तः

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
संरचना FKey {
  सारणी *pFrom;     /* Table containing the REFERENCES clause (aka: Child) */
  FKey *p_अग्र_From;  /* Next FKey with the same in pFrom. Next parent of pFrom */
  वर्ण *z_गन्तव्यम्;        /* Name of table that the key points to (aka: Parent) */
  FKey *p_अग्र_गन्तव्यम्;    /* Next with the same zTo. Next child of zTo. */
  FKey *p_पूर्व_गन्तव्यम्;    /* Previous with the same zTo */
  पूर्णाङ्क nCol;         /* Number of columns in this key */
  /* EV: R-30323-21917 */
  u8 isDeferred;       /* True if constraint checking is deferred till COMMIT */
  u8 aAction[2];        /* ON DELETE and ON UPDATE actions, respectively */
  Trigger *apTrigger[2];/* Triggers for aAction[] actions */
  संरचना sColMap {      /* Mapping of columns in pFrom to columns in zTo */
    पूर्णाङ्क iFrom;            /* Index of column in pFrom */
    वर्ण *zCol;           /* Name of column in zTo.  If NULL use PRIMARY KEY */
  } aCol[FLEXARRAY];      /* One entry for each of nCol columns */
};

/* The size (in bytes) of an FKey object holding N columns.  The answer
** does NOT include space to hold the zTo name. */
#परिभाषय SZ_FKEY(N)  (offsetof(FKey,aCol)+(N)*परिमाण(संरचना sColMap))

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
#परिभाषय OE_None     0   /* There is no constraint to check */
#परिभाषय OE_Rollback 1   /* Fail the operation and rollback the transaction */
#परिभाषय OE_Abort    2   /* Back out changes but do no rollback transaction */
#परिभाषय OE_Fail     3   /* Stop the operation but leave all prior changes */
#परिभाषय OE_Ignore   4   /* Ignore the error. Do not do the INSERT or UPDATE */
#परिभाषय OE_Replace  5   /* Delete existing record, then do INSERT or UPDATE */
#परिभाषय OE_Update   6   /* Process as a DO UPDATE in an upsert */
#परिभाषय OE_Restrict 7   /* OE_Abort for IMMEDIATE, OE_Rollback for DEFERRED */
#परिभाषय OE_समुच्चयः_Null  8   /* Set the foreign key value to NULL */
#परिभाषय OE_समुच्चयः_Dflt  9   /* Set the foreign key value to its default */
#परिभाषय OE_Cascade  10  /* Cascade the changes */
#परिभाषय OE_Default  11  /* Do whatever the default action is */


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
संरचना कुञ्चिका_Info {
  u32 nRef;           /* Number of references to this KeyInfo object */
  u8 enc;             /* Text encoding - one of the SQLITE_UTF* values */
  u16 n_कुञ्चिका_क्षेत्रम्;      /* Number of key columns in the index */
  u16 n_All_क्षेत्रम्;      /* Total columns, including key plus others */
  sqlite3 *db;        /* The database connection */
  u8 *a_क्रमय_Flags;     /* Sort order for each column. */
  CollSeq *aColl[FLEXARRAY]; /* Collating sequence for each term of the key */
};

/* The size (in bytes) of a KeyInfo object with up to N fields.  This includes
** the main body of the KeyInfo object and the aColl[] array of N elements,
** but does not count the memory used to hold aSortFlags[]. */
#परिभाषय SZ_KEYINFO(N)  (offsetof(कुञ्चिका_Info,aColl) + (N)*परिमाण(CollSeq*))

/* The size of a bare KeyInfo with no aColl[] entries */
#यदि FLEXARRAY+1 > 1
# define SZ_KEYINFO_0   offsetof(कुञ्चिका_Info,aColl)
#अन्यथा
# define SZ_KEYINFO_0   परिमाण(कुञ्चिका_Info)
#यदि_अन्तः

/*
** Allowed bit values for entries in the KeyInfo.aSortFlags[] array.
*/
#परिभाषय KEYINFO_क्रमः_DESC    0x01    /* DESC sort order */
#परिभाषय KEYINFO_क्रमः_BIGNULL 0x02    /* NULL is larger than any other value */

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
संरचना Unpacked_लेखः {
  कुञ्चिका_Info *p_कुञ्चिका_Info;  /* Comparison info for the index that is unpacked */
  Mem *aMem;          /* Values for columns of the index */
  सङ्घ {
    वर्ण *z;            /* Cache of aMem[0].z for vdbeRecordCompareString() */
    i64 i;              /* Cache of aMem[0].u.i for vdbeRecordCompareInt() */
  } u;
  पूर्णाङ्क n;              /* Cache of aMem[0].n used by vdbeRecordCompareString() */
  u16 n_क्षेत्रम्;         /* Number of entries in apMem[] */
  i8 default_rc;      /* Comparison result if keys are equal */
  u8 err_संहिता;         /* Error detected by xRecordCompare (CORRUPT or NOMEM) */
  i8 r1;              /* Value to return if (lhs < rhs) */
  i8 r2;              /* Value to return if (lhs > rhs) */
  u8 eq_आलोकितम्;          /* True if an equality comparison has been seen */
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
संरचना सूचकांक {
  वर्ण *zName;             /* Name of this index */
  i16 *aiColumn;           /* Which columns are used by this index.  1st is 0 */
  LogEst *aiRowLogEst;     /* From ANALYZE: Est. rows selected by each column */
  सारणी *p_सारणी;           /* The SQL table being indexed */
  वर्ण *zColAff;           /* String defining the affinity of each column */
  सूचकांक *p_अग्र;            /* The next index associated with the same table */
  Schema *pSchema;         /* Schema containing this index */
  u8 *a_क्रमय_क्रमः;          /* for each column: True==DESC, False==ASC */
  ध्रुव वर्ण **azColl;     /* Array of collation sequence names for index */
  व्यञ्जकम् *pPartIdxWhere;     /* WHERE clause for partial indices */
  व्यञ्जकम्_List *a_Col_व्यञ्जकम्;      /* Column expressions */
  Pgno tnum;               /* DB Page containing root of this index */
  LogEst szIdxRow;         /* Estimated average row size in bytes */
  u16 n_कुञ्चिका_Col;             /* Number of columns forming the key */
  u16 nColumn;             /* Nr columns in btree. Can be 2*Table.nCol */
  u8 onError;              /* OE_Abort, OE_Ignore, OE_Replace, or OE_None */
  अचिह्नित idxType:2;      /* 0:Normal 1:UNIQUE, 2:PRIMARY KEY, 3:IPK */
  अचिह्नित bUnordered:1;   /* Use this index for == or IN queries only */
  अचिह्नित uniqNotNull:1;  /* True if UNIQUE and NOT NULL for all columns */
  अचिह्नित isResized:1;    /* True if resizeIndexObject() has been called */
  अचिह्नित isCovering:1;   /* True if this is a covering index */
  अचिह्नित no_Skip_अवलोकनम्:1;   /* Do not try to use skip-scan if true */
  अचिह्नित hasStat1:1;     /* aiRowLogEst values come from sqlite_stat1 */
  अचिह्नित bNoQuery:1;     /* Do not use this index to optimize queries */
  अचिह्नित b_Asc_कुञ्चिका_Bug:1;   /* True if the bba7b69f9849b5bf bug applies */
  अचिह्नित bHasVCol:1;     /* Index references one or more VIRTUAL columns */
  अचिह्नित b_Has_व्यञ्जकम्:1;     /* Index contains an expression, either a literal
                           ** expression, or a reference to a VIRTUAL column */
#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
  पूर्णाङ्क nSample;             /* Number of elements in aSample[] */
  पूर्णाङ्क mxSample;            /* Number of slots allocated to aSample[] */
  पूर्णाङ्क nSampleCol;          /* Size of IndexSample.anEq[] and so on */
  tRowcnt *aAvgEq;         /* Average nEq values for keys not in aSample */
  सूचकांक_Sample *aSample;    /* Samples of the left-most key */
  tRowcnt *aiRowEst;       /* Non-logarithmic stat1 data for this index */
  tRowcnt nRowEst0;        /* Non-logarithmic number of rows in the index */
#यदि_अन्तः
  Bitmask colNotIdxed;     /* Unindexed columns in pTab */
};

/*
** Allowed values for Index.idxType
*/
#परिभाषय SQLITE_IDXTYPE_APPDEF      0   /* Created using CREATE INDEX */
#परिभाषय SQLITE_IDXTYPE_UNIQUE      1   /* Implements a UNIQUE constraint */
#परिभाषय SQLITE_IDXTYPE_PRIMARYKEY  2   /* Is the PRIMARY KEY for the table */
#परिभाषय SQLITE_IDXTYPE_IPK         3   /* INTEGER PRIMARY KEY index */

/* Return true if index X is a PRIMARY KEY index */
#परिभाषय Is_Primary_कुञ्चिका_सूचकांक(X)  ((X)->idxType==SQLITE_IDXTYPE_PRIMARYKEY)

/* Return true if index X is a UNIQUE index */
#परिभाषय Is_Unique_सूचकांक(X)      ((X)->onError!=OE_None)

/* The Index.aiColumn[] values are normally positive integer.  But
** there are some negative values that have special meaning:
*/
#परिभाषय XN_ROWID     (-1)     /* Indexed column is the rowid */
#परिभाषय XN_व्यञ्जकम्      (-2)     /* Indexed column is an expression */

/*
** Each sample stored in the sqlite_stat4 table is represented in memory
** using a structure of this type.  See documentation at the top of the
** analyze.c source file for additional information.
*/
संरचना सूचकांक_Sample {
  शून्य *p;          /* Pointer to sampled record */
  पूर्णाङ्क n;            /* Size of record in bytes */
  tRowcnt *anEq;    /* Est. number of rows where the key equals this sample */
  tRowcnt *anLt;    /* Est. number of rows where key is less than this sample */
  tRowcnt *anDLt;   /* Est. number of distinct keys less than this sample */
};

/*
** Possible values to use within the flags argument to sqlite3GetToken().
*/
#परिभाषय SQLITE_TOKEN_QUOTED    0x1 /* Token is a quoted identifier. */
#परिभाषय SQLITE_TOKEN_KEYWORD   0x2 /* Token is a keyword. */

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
संरचना Token {
  ध्रुव वर्ण *z;     /* Text of the token.  Not NULL-terminated! */
  अचिह्नित पूर्णाङ्क n;    /* Number of characters in this token */
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
संरचना AggInfo {
  u8 प्रत्यक्षम्_बहुलकम्;          /* Direct rendering mode means take data directly
                          ** from source tables rather than from accumulators */
  u8 use_क्रमणम्_Idx;       /* In direct mode, reference the sorting index rather
                          ** than the source table */
  u32 n_क्रमणम्_Column;     /* Number of columns in the sorting index */
  पूर्णाङ्क क्रमणम्_Idx;         /* Cursor number of the sorting index */
  पूर्णाङ्क क्रमणम्_Idx_PTab;     /* Cursor number of pseudo-table */
  पूर्णाङ्क iFirstReg;          /* First register in range for aCol[] and aFunc[] */
  व्यञ्जकम्_List *p_गणः_By;     /* The group by clause */
  संरचना AggInfo_col {    /* For each column used in source tables */
    सारणी *pTab;             /* Source table */
    व्यञ्जकम् *pCExpr;            /* The original expression */
    पूर्णाङ्क i_सारणी;              /* Cursor number of the source table */
    पूर्णाङ्क iColumn;             /* Column number within the source table */
    पूर्णाङ्क iSorterColumn;       /* Column number in the sorting index */
  } *aCol;
  पूर्णाङ्क nColumn;            /* Number of used entries in aCol[] */
  पूर्णाङ्क n_सञ्चायकः;       /* Number of columns that show through to the output.
                          ** Additional columns are used only as parameters to
                          ** aggregate functions */
  संरचना AggInfo_func {   /* For each aggregate function */
    व्यञ्जकम् *pFExpr;            /* Expression encoding the function */
    FuncDef *pFunc;          /* The aggregate function implementation */
    पूर्णाङ्क iDistinct;           /* Ephemeral table used to enforce DISTINCT */
    पूर्णाङ्क iDistAddr;           /* Address of OP_OpenEphemeral */
    पूर्णाङ्क iOBTab;              /* Ephemeral table to implement ORDER BY */
    u8 bOBPayload;           /* iOBTab has payload columns separate from key */
    u8 bOBUnique;            /* Enforce uniqueness on iOBTab keys */
    u8 bUseSubtype;          /* Transfer subtype info through sorter */
  } *aFunc;
  पूर्णाङ्क nFunc;              /* Number of entries in aFunc[] */
  u32 selId;              /* Select to which this AggInfo belongs */
#यदि_परिभाषितम् SQLITE_DEBUG
  Select *pSelect;        /* SELECT statement that this AggInfo supports */
#यदि_अन्तः
};

/*
** Macros to compute aCol[] and aFunc[] register numbers.
**
** These macros should not be used prior to the call to
** assignAggregateRegisters() that computes the value of pAggInfo->iFirstReg.
** The assert()s that are part of this macro verify that constraint.
*/
#यदि_अपरिभाषितम् NDEBUG
#परिभाषय AggInfoColumnReg(A,I)  (प्रतिज्ञा((A)->iFirstReg),(A)->iFirstReg+(I))
#परिभाषय AggInfoFuncReg(A,I)    \
                      (प्रतिज्ञा((A)->iFirstReg),(A)->iFirstReg+(A)->nColumn+(I))
#अन्यथा
#परिभाषय AggInfoColumnReg(A,I)  ((A)->iFirstReg+(I))
#परिभाषय AggInfoFuncReg(A,I)    \
                      ((A)->iFirstReg+(A)->nColumn+(I))
#यदि_अन्तः

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
#यदि SQLITE_MAX_VARIABLE_सङ्ख्या<32767
प्रकारनिर्देश i16 ynVar;
#अन्यथा
प्रकारनिर्देश पूर्णाङ्क ynVar;
#यदि_अन्तः

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
संरचना व्यञ्जकम् {
  u8 op;                 /* Operation performed by this node */
  वर्ण aff_व्यञ्जकम्;          /* affinity, or RAISE type */
  u8 op2;                /* TK_REGISTER/TK_TRUTH: original value of Expr.op
                         ** TK_COLUMN: the value of p5 for OP_Column
                         ** TK_AGG_FUNCTION: nesting depth
                         ** TK_FUNCTION: NC_SelfRef flag if needs OP_PureFunc */
#यदि_परिभाषितम् SQLITE_DEBUG
  u8 vvaFlags;           /* Verification flags. */
#यदि_अन्तः
  u32 flags;             /* Various flags.  EP_* See below */
  सङ्घ {
    वर्ण *zToken;          /* Token value. Zero terminated and dequoted */
    पूर्णाङ्क i_मानम्;            /* Non-negative integer value if EP_IntValue */
  } u;

  /* If the EP_TokenOnly flag is set in the Expr.flags mask, then no
  ** space is allocated for the fields below this point. An attempt to
  ** access them will result in a segfault or malfunction.
  *********************************************************************/

  व्यञ्जकम् *p_वामः;           /* Left subnode */
  व्यञ्जकम् *p_दक्षिणः;          /* Right subnode */
  सङ्घ {
    व्यञ्जकम्_List *pList;     /* op = IN, EXISTS, SELECT, CASE, FUNCTION, BETWEEN */
    Select *pSelect;     /* EP_xIsSelect and op = IN, EXISTS, SELECT */
  } x;

  /* If the EP_Reduced flag is set in the Expr.flags mask, then no
  ** space is allocated for the fields below this point. An attempt to
  ** access them will result in a segfault or malfunction.
  *********************************************************************/

#यदि SQLITE_MAX_व्यञ्जकम्_गभीरता>0
  पूर्णाङ्क nHeight;           /* Height of the tree headed by this node */
#यदि_अन्तः
  पूर्णाङ्क i_सारणी;            /* TK_COLUMN: cursor number of table holding column
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
  सङ्घ {
    पूर्णाङ्क iJoin;             /* If EP_OuterON or EP_InnerON, the right table */
    पूर्णाङ्क iOfst;             /* else: start of token from start of statement */
  } w;
  AggInfo *pAggInfo;     /* Used by TK_AGG_COLUMN and TK_AGG_FUNCTION */
  सङ्घ {
    सारणी *pTab;           /* TK_COLUMN: Table containing column. Can be NULL
                           ** for a column of an index on an expression */
    गवाक्षः *pWin;          /* EP_WinFunc: Window/Filter defn for a function */
    पूर्णाङ्क nReg;              /* TK_NULLS: Number of registers to NULL out */
    संरचना {               /* TK_IN, TK_SELECT, and TK_EXISTS */
      पूर्णाङ्क iAddr;             /* Subroutine entry address */
      पूर्णाङ्क regReturn;         /* Register used to hold return address */
    } sub;
  } y;
};

/* The following are the meanings of bits in the Expr.flags field.
** Value restrictions:
**
**          EP_Agg == NC_HasAgg == SF_HasAgg
**          EP_Win == NC_HasWin
*/
#परिभाषय EP_OuterON    0x000001 /* Originates in ON/USING clause of outer join */
#परिभाषय EP_InnerON    0x000002 /* Originates in ON/USING of an inner join */
#परिभाषय EP_Distinct   0x000004 /* Aggregate function with DISTINCT keyword */
#परिभाषय EP_HasFunc    0x000008 /* Contains one or more functions of any kind */
#परिभाषय EP_Agg        0x000010 /* Contains one or more aggregate functions */
#परिभाषय EP_FixedCol   0x000020 /* TK_Column with a known fixed value */
#परिभाषय EP_VarSelect  0x000040 /* pSelect is correlated, not constant */
#परिभाषय EP_DblQuoted  0x000080 /* token.z was originally in "..." */
#परिभाषय EP_InfixFunc  0x000100 /* True for an infix function: LIKE, GLOB, etc */
#परिभाषय EP_Collate    0x000200 /* Tree contains a TK_COLLATE operator */
#परिभाषय EP_Commuted   0x000400 /* Comparison operator has been commuted */
#परिभाषय EP_Int_मानम्   0x000800 /* Integer value contained in u.iValue */
#परिभाषय EP_xIsSelect  0x001000 /* x.pSelect is valid (otherwise x.pList is) */
#परिभाषय EP_Skip       0x002000 /* Operator does not contribute to affinity */
#परिभाषय EP_Reduced    0x004000 /* Expr struct EXPR_REDUCEDSIZE bytes only */
#परिभाषय EP_Win        0x008000 /* Contains window functions */
#परिभाषय EP_TokenOnly  0x010000 /* Expr struct EXPR_TOKENONLYSIZE bytes only */
#परिभाषय EP_पूर्णम्_आकार   0x020000 /* Expr structure must remain full sized */
#परिभाषय EP_IfNullRow  0x040000 /* The TK_IF_NULL_ROW opcode */
#परिभाषय EP_Unlikely   0x080000 /* unlikely() or likelihood() function */
#परिभाषय EP_ConstFunc  0x100000 /* A SQLITE_FUNC_CONSTANT or _SLOCHNG function */
#परिभाषय EP_CanBeNull  0x200000 /* Can be null despite NOT NULL constraint */
#परिभाषय EP_Subquery   0x400000 /* Tree contains a TK_SELECT operator */
#परिभाषय EP_पर्णम्       0x800000 /* Expr.pLeft, .pRight, .u.pSelect all NULL */
#परिभाषय EP_WinFunc   0x1000000 /* TK_FUNCTION with Expr.y.pWin set */
#परिभाषय EP_Subrtn    0x2000000 /* Uses Expr.y.sub. TK_IN, _SELECT, or _EXISTS */
#परिभाषय EP_Quoted    0x4000000 /* TK_ID was originally quoted */
#परिभाषय EP_Static    0x8000000 /* Held in memory not obtained from malloc() */
#परिभाषय EP_IsTrue   0x10000000 /* Always has boolean value of TRUE */
#परिभाषय EP_IsFalse  0x20000000 /* Always has boolean value of FALSE */
#परिभाषय EP_FromDDL  0x40000000 /* Originates from sqlite_schema */
#परिभाषय EP_SubtArg  0x80000000 /* Is argument to SQLITE_SUBTYPE function */

/* The EP_Propagate mask is a set of properties that automatically propagate
** upwards into parent nodes.
*/
#परिभाषय EP_Propagate (EP_Collate|EP_Subquery|EP_HasFunc)

/* Macros can be used to test, set, or clear bits in the
** Expr.flags field.
*/
#परिभाषय व्यञ्जकम्_Has_Property(E,P)     (((E)->flags&(u32)(P))!=0)
#परिभाषय व्यञ्जकम्_Has_All_Property(E,P)  (((E)->flags&(u32)(P))==(u32)(P))
#परिभाषय व्यञ्जकम्_समुच्चयः_Property(E,P)     (E)->flags|=(u32)(P)
#परिभाषय व्यञ्जकम्_Clear_Property(E,P)   (E)->flags&=~(u32)(P)
#परिभाषय व्यञ्जकम्_Always_True(E)   (((E)->flags&(EP_OuterON|EP_IsTrue))==EP_IsTrue)
#परिभाषय व्यञ्जकम्_Always_False(E)  (((E)->flags&(EP_OuterON|EP_IsFalse))==EP_IsFalse)
#परिभाषय व्यञ्जकम्_Is_पूर्णम्_आकार(E)   (((E)->flags&(EP_Reduced|EP_TokenOnly))==0)

/* Macros used to ensure that the correct members of unions are accessed
** in Expr.
*/
#परिभाषय व्यञ्जकम्_Use_UToken(E)    (((E)->flags&EP_Int_मानम्)==0)
#परिभाषय व्यञ्जकम्_Use_UValue(E)    (((E)->flags&EP_Int_मानम्)!=0)
#परिभाषय व्यञ्जकम्_Use_WOfst(E)     (((E)->flags&(EP_InnerON|EP_OuterON))==0)
#परिभाषय व्यञ्जकम्_Use_WJoin(E)     (((E)->flags&(EP_InnerON|EP_OuterON))!=0)
#परिभाषय व्यञ्जकम्_Use_XList(E)     (((E)->flags&EP_xIsSelect)==0)
#परिभाषय व्यञ्जकम्_Use_XSelect(E)   (((E)->flags&EP_xIsSelect)!=0)
#परिभाषय व्यञ्जकम्_Use_YTab(E)      (((E)->flags&(EP_WinFunc|EP_Subrtn))==0)
#परिभाषय व्यञ्जकम्_Use_YWin(E)      (((E)->flags&EP_WinFunc)!=0)
#परिभाषय व्यञ्जकम्_Use_YSub(E)      (((E)->flags&EP_Subrtn)!=0)

/* Flags for use with Expr.vvaFlags
*/
#परिभाषय EP_NoReduce   0x01  /* Cannot EXPRDUP_REDUCE this Expr */
#परिभाषय EP_Immutable  0x02  /* Do not change this Expr node */

/* The ExprSetVVAProperty() macro is used for Verification, Validation,
** and Accreditation only.  It works like ExprSetProperty() during VVA
** processes but is a no-op for delivery.
*/
#यदि_परिभाषितम् SQLITE_DEBUG
# define व्यञ्जकम्_समुच्चयः_VVAProperty(E,P)   (E)->vvaFlags|=(P)
# define व्यञ्जकम्_Has_VVAProperty(E,P)   (((E)->vvaFlags&(P))!=0)
# define व्यञ्जकम्_Clear_VVAProperties(E) (E)->vvaFlags = 0
#अन्यथा
# define व्यञ्जकम्_समुच्चयः_VVAProperty(E,P)
# define व्यञ्जकम्_Has_VVAProperty(E,P)   0
# define व्यञ्जकम्_Clear_VVAProperties(E)
#यदि_अन्तः

/*
** Macros to determine the number of bytes required by a normal Expr
** struct, an Expr struct with the EP_Reduced flag set in Expr.flags
** and an Expr struct with the EP_TokenOnly flag set.
*/
#परिभाषय व्यञ्जकम्_FULLSIZE           परिमाण(व्यञ्जकम्)           /* Full size */
#परिभाषय व्यञ्जकम्_REDUCEDSIZE        offsetof(व्यञ्जकम्,i_सारणी)  /* Common features */
#परिभाषय व्यञ्जकम्_TOKENONLYSIZE      offsetof(व्यञ्जकम्,p_वामः)   /* Fewer features */

/*
** Flags passed to the sqlite3ExprDup() function. See the header comment
** above sqlite3ExprDup() for details.
*/
#परिभाषय EXPRDUP_REDUCE         0x0001  /* Used reduced-size Expr nodes */

/*
** True if the expression passed as an argument was a function with
** an OVER() clause (a window function).
*/
#यदि_परिभाषितम् SQLITE_OMIT_WINDOWFUNC
# define Is_गवाक्षः_Func(p) 0
#अन्यथा
# define Is_गवाक्षः_Func(p) ( \
    व्यञ्जकम्_Has_Property((p), EP_WinFunc) && p->y.pWin->eFrmType!=TK_FILTER \
 )
#यदि_अन्तः

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
संरचना व्यञ्जकम्_List {
  पूर्णाङ्क n_व्यञ्जकम्;             /* Number of expressions on the list */
  पूर्णाङ्क nAlloc;            /* Number of a[] slots allocated */
  संरचना व्यञ्जकम्_List_item { /* For each expression in the list */
    व्यञ्जकम् *p_व्यञ्जकम्;            /* The parse tree for this expression */
    वर्ण *zEName;           /* Token associated with this expression */
    संरचना {
      u8 क्रमय_Flags;           /* Mask of KEYINFO_ORDER_* flags */
      अचिह्नित eEName :2;     /* Meaning of zEName */
      अचिह्नित समाप्तम् :1;       /* Indicates when processing is finished */
      अचिह्नित reusable :1;   /* Constant expression is reusable */
      अचिह्नित bSorterRef :1; /* Defer evaluation until after sorting */
      अचिह्नित bNulls :1;     /* True if explicit "NULLS FIRST/LAST" */
      अचिह्नित b_प्रयुक्तम् :1;      /* This column used in a SF_NestedFrom subquery */
      अचिह्नित b_Using_पदांशः:1;  /* Term from the USING clause of a NestedFrom */
      अचिह्नित bNoExpand: 1;  /* Term is an auxiliary in NestedFrom and should
                              ** not be expanded by "*" in parent queries */
    } fg;
    सङ्घ {
      संरचना {             /* Used by any ExprList other than Parse.pConsExpr */
        u16 i_क्रमः_By_Col;      /* For ORDER BY, column number in result set */
        u16 iAlias;           /* Index into Parse.aAlias[] for zName */
      } x;
      पूर्णाङ्क i_Const_व्यञ्जकम्_Reg;   /* Register in which Expr value is cached. Used only
                           ** by Parse.pConstExpr */
    } u;
  } a[FLEXARRAY];          /* One slot for each expression in the list */
};

/* The size (in bytes) of an ExprList object that is big enough to hold
** as many as N expressions. */
#परिभाषय SZ_EXPRLIST(N)  \
             (offsetof(व्यञ्जकम्_List,a) + (N)*परिमाण(संरचना व्यञ्जकम्_List_item))

/*
** Allowed values for Expr.a.eEName
*/
#परिभाषय ENAME_NAME  0       /* The AS clause of a result set */
#परिभाषय ENAME_विस्तृतिः  1       /* Complete text of the result set expression */
#परिभाषय ENAME_TAB   2       /* "DB.TABLE.NAME" for the result set */
#परिभाषय ENAME_ROWID 3       /* "DB.TABLE._rowid_" for * expansion of rowid */

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
संरचना IdList {
  पूर्णाङ्क nId;         /* Number of identifiers on the list */
  संरचना IdList_item {
    वर्ण *zName;      /* Name of the identifier */
  } a[FLEXARRAY];
};

/* The size (in bytes) of an IdList object that can hold up to N IDs. */
#परिभाषय SZ_IDLIST(N)  (offsetof(IdList,a)+(N)*परिमाण(संरचना IdList_item))

/*
** Allowed values for IdList.eType, which determines which value of the a.u4
** is valid.
*/
#परिभाषय EU4_NONE   0   /* Does not use IdList.a.u4 */
#परिभाषय EU4_IDX    1   /* Uses IdList.a.u4.idx */
#परिभाषय EU4_व्यञ्जकम्   2   /* Uses IdList.a.u4.pExpr -- NOT CURRENTLY USED */

/*
** Details of the implementation of a subquery.
*/
संरचना Subquery {
  Select *pSelect;  /* A SELECT statement used in place of a table name */
  पूर्णाङ्क addrFillSub;  /* Address of subroutine to initialize a subquery */
  पूर्णाङ्क regReturn;    /* Register holding return address of addrFillSub */
  पूर्णाङ्क reg_परिणाम;    /* Registers holding results of a co-routine */
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
संरचना SrcItem {
  वर्ण *zName;      /* Name of the table */
  वर्ण *zAlias;     /* The "B" part of a "A AS B" phrase.  zName is the "A" */
  सारणी *pSTab;     /* Table object for zName. Mnemonic: Srcitem-TABle */
  संरचना {
    u8 jointype;      /* Type of join between this table and the previous */
    अचिह्नित not_अनुक्रमितम् :1;    /* True if there is a NOT INDEXED clause */
    अचिह्नित is_अनुक्रमितम्_By :1;   /* True if there is an INDEXED BY clause */
    अचिह्नित isSubquery :1;    /* True if this term is a subquery */
    अचिह्नित isTabFunc :1;     /* True if table-valued-function syntax */
    अचिह्नित isCorrelated :1;  /* True if sub-query is correlated */
    अचिह्नित isMaterialized:1; /* This is a materialized view */
    अचिह्नित द्वारा_Coroutine :1;  /* Implemented as a co-routine */
    अचिह्नित isRecursive :1;   /* True for recursive reference in WITH */
    अचिह्नित fromDDL :1;       /* Comes from sqlite_schema */
    अचिह्नित isCte :1;         /* This is a CTE */
    अचिह्नित notCte :1;        /* This item may not match a CTE */
    अचिह्नित isUsing :1;       /* u3.pUsing is valid */
    अचिह्नित isOn :1;          /* u3.pOn was once valid and non-NULL */
    अचिह्नित isSynthUsing :1;  /* u3.pUsing is synthesized from NATURAL */
    अचिह्नित isNestedFrom :1;  /* pSelect is a SF_NestedFrom subquery */
    अचिह्नित rowid_प्रयुक्तम् :1;     /* The ROWID of this table is referenced */
    अचिह्नित fixedSchema :1;   /* Uses u4.pSchema, not u4.zDatabase */
    अचिह्नित hadSchema :1;     /* Had u4.zDatabase before u4.pSchema */
    अचिह्नित fromExists :1;    /* Comes from WHERE EXISTS(...) */
  } fg;
  पूर्णाङ्क iCursor;      /* The VDBE cursor number used to access this table */
  Bitmask col_प्रयुक्तम्;  /* Bit N set if column N used. Details above for N>62 */
  सङ्घ {
    वर्ण *z_अनुक्रमितम्_By;    /* Identifier from "INDEXED BY <zIndex>" clause */
    व्यञ्जकम्_List *pFuncArg;  /* Arguments to table-valued-function */
    u32 nRow;            /* Number of rows in a VALUES clause */
  } u1;
  सङ्घ {
    सूचकांक *pIBIndex;  /* Index structure corresponding to u1.zIndexedBy */
    CteUse *pCteUse;  /* CTE Usage info when fg.isCte is true */
  } u2;
  सङ्घ {
    व्यञ्जकम् *pOn;        /* fg.isUsing==0 =>  The ON clause of a join */
    IdList *pUsing;   /* fg.isUsing==1 =>  The USING clause of a join */
  } u3;
  सङ्घ {
    Schema *pSchema;  /* Schema to which this item is fixed */
    वर्ण *zDatabase;  /* Name of database holding this table */
    Subquery *pSubq;  /* Description of a subquery */
  } u4;
};

/*
** The OnOrUsing object represents either an ON clause or a USING clause.
** It can never be both at the same time, but it can be neither.
*/
संरचना OnOrUsing {
  व्यञ्जकम् *pOn;         /* The ON clause of a join */
  IdList *pUsing;    /* The USING clause of a join */
};

/*
** This object represents one or more tables that are the source of
** content for an SQL statement.  For example, a single SrcList object
** is used to hold the FROM clause of a SELECT statement.  SrcList also
** represents the target tables for DELETE, INSERT, and UPDATE statements.
**
*/
संरचना SrcList {
  पूर्णाङ्क nSrc;             /* Number of tables or subqueries in the FROM clause */
  u32 nAlloc;           /* Number of entries allocated in a[] below */
  SrcItem a[FLEXARRAY]; /* One entry for each identifier on the list */
};

/* Size (in bytes) of a SrcList object that can hold as many as N
** SrcItem objects. */
#परिभाषय SZ_SRCLIST(N) (offsetof(SrcList,a)+(N)*परिमाण(SrcItem))

/* Size (in bytes( of a SrcList object that holds 1 SrcItem.  This is a
** special case of SZ_SRCITEM(1) that comes up often. */
#परिभाषय SZ_SRCLIST_1  (offsetof(SrcList,a)+परिमाण(SrcItem))

/*
** Permitted values of the SrcList.a.jointype field
*/
#परिभाषय JT_INNER     0x01    /* Any kind of inner or cross join */
#परिभाषय JT_CROSS     0x02    /* Explicit use of the CROSS keyword */
#परिभाषय JT_NATURAL   0x04    /* True for a "natural" join */
#परिभाषय JT_वामः      0x08    /* Left outer join */
#परिभाषय JT_दक्षिणः     0x10    /* Right outer join */
#परिभाषय JT_OUTER     0x20    /* The "OUTER" keyword is present */
#परिभाषय JT_LTORJ     0x40    /* One of the LEFT operands of a RIGHT JOIN
                             ** Mnemonic: Left Table Of Right Join */
#परिभाषय JT_ERROR     0x80    /* unknown or unsupported join type */

/*
** Flags appropriate for the wctrlFlags parameter of sqlite3WhereBegin()
** and the WhereInfo.wctrlFlags member.
**
** Value constraints (enforced via assert()):
**     WHERE_USE_LIMIT  == SF_FixedLimit
*/
#परिभाषय WHERE_ORDERBY_प्रसामान्यम्   0x0000 /* No-op */
#परिभाषय WHERE_ORDERBY_MIN      0x0001 /* ORDER BY processing for min() func */
#परिभाषय WHERE_ORDERBY_MAX      0x0002 /* ORDER BY processing for max() func */
#परिभाषय WHERE_ONEPASS_DESIRED  0x0004 /* Want to do one-pass UPDATE/DELETE */
#परिभाषय WHERE_ONEPASS_MULTIROW 0x0008 /* ONEPASS is ok with multiple rows */
#परिभाषय WHERE_DUPLICATES_OK    0x0010 /* Ok to return a row more than once */
#परिभाषय WHERE_OR_SUBCLAUSE     0x0020 /* Processing a sub-WHERE as part of
                                      ** the OR optimization  */
#परिभाषय WHERE_GROUPBY          0x0040 /* pOrderBy is really a GROUP BY */
#परिभाषय WHERE_DISTINCTBY       0x0080 /* pOrderby is really a DISTINCT clause */
#परिभाषय WHERE_WANT_DISTINCT    0x0100 /* All output needs to be distinct */
#परिभाषय WHERE_SORTBYGROUP      0x0200 /* Support sqlite3WhereIsSorted() */
#परिभाषय WHERE_AGG_DISTINCT     0x0400 /* Query is "SELECT agg(DISTINCT ...)" */
#परिभाषय WHERE_ORDERBY_सीमा    0x0800 /* ORDERBY+LIMIT on the inner loop */
#परिभाषय WHERE_दक्षिणः_JOIN       0x1000 /* Processing a RIGHT JOIN */
#परिभाषय WHERE_KEEP_ALL_JOINS   0x2000 /* Do not do the omit-noop-join opt */
#परिभाषय WHERE_USE_सीमा        0x4000 /* Use the LIMIT in cost estimates */
                        /*     0x8000    not currently used */

/* Allowed return values from sqlite3WhereIsDistinct()
*/
#परिभाषय WHERE_DISTINCT_NOOP      0  /* DISTINCT keyword not used */
#परिभाषय WHERE_DISTINCT_UNIQUE    1  /* No duplicates */
#परिभाषय WHERE_DISTINCT_ORDERED   2  /* All duplicates are adjacent */
#परिभाषय WHERE_DISTINCT_UNORDERED 3  /* Duplicates are scattered */

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
संरचना NameContext {
  Parse *pParse;       /* The parser */
  SrcList *pSrcList;   /* One or more tables used to resolve names */
  सङ्घ {
    व्यञ्जकम्_List *pEList;    /* Optional list of result-set columns */
    AggInfo *pAggInfo;   /* Information about aggregates at this level */
    Upsert *pUpsert;     /* ON CONFLICT clause information from an upsert */
    पूर्णाङ्क i_आधारः_Reg;        /* For TK_REGISTER when parsing RETURNING */
  } uNC;
  NameContext *p_अग्र;  /* Next outer name context.  NULL for outermost */
  पूर्णाङ्क nRef;            /* Number of names resolved by this context */
  पूर्णाङ्क nNcErr;          /* Number of errors encountered while resolving names */
  पूर्णाङ्क ncFlags;         /* Zero or more NC_* flags defined below */
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
#परिभाषय NC_AllowAgg  0x000001 /* Aggregate functions are allowed here */
#परिभाषय NC_PartIdx   0x000002 /* True if resolving a partial index WHERE */
#परिभाषय NC_IsCheck   0x000004 /* True if resolving a CHECK constraint */
#परिभाषय NC_GenCol    0x000008 /* True for a GENERATED ALWAYS AS clause */
#परिभाषय NC_HasAgg    0x000010 /* One or more aggregate functions seen */
#परिभाषय NC_Idx_व्यञ्जकम्   0x000020 /* True if resolving columns of CREATE INDEX */
#परिभाषय NC_SelfRef   0x00002e /* Combo: PartIdx, isCheck, GenCol, and IdxExpr */
#परिभाषय NC_Subquery  0x000040 /* A subquery has been seen */
#परिभाषय NC_UEList    0x000080 /* True if uNC.pEList is used */
#परिभाषय NC_UAggInfo  0x000100 /* True if uNC.pAggInfo is used */
#परिभाषय NC_UUpsert   0x000200 /* True if uNC.pUpsert is used */
#परिभाषय NC_UBaseReg  0x000400 /* True if uNC.iBaseReg is used */
#परिभाषय NC_MinMaxAgg 0x001000 /* min/max aggregates seen.  See note above */
/*                   0x002000 // available for reuse */
#परिभाषय NC_AllowWin  0x004000 /* Window functions are allowed here */
#परिभाषय NC_HasWin    0x008000 /* One or more window functions seen */
#परिभाषय NC_IsDDL     0x010000 /* Resolving names in a CREATE statement */
#परिभाषय NC_अन्तर्_Agg_Func 0x020000 /* True if analyzing arguments to an agg func */
#परिभाषय NC_FromDDL   0x040000 /* SQL text comes from sqlite_schema */
#परिभाषय NC_NoSelect  0x080000 /* Do not descend into sub-selects */
#परिभाषय NC_Where     0x100000 /* Processing WHERE clause of a SELECT */
#परिभाषय NC_क्रमः_Agg 0x8000000 /* Has an aggregate other than count/min/max */

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
संरचना Upsert {
  व्यञ्जकम्_List *pUpsertTarget;  /* Optional description of conflict target */
  व्यञ्जकम् *pUpsertTargetWhere; /* WHERE clause for partial index targets */
  व्यञ्जकम्_List *p_Upsert_समुच्चयः;     /* The SET clause from an ON CONFLICT UPDATE */
  व्यञ्जकम् *pUpsertWhere;       /* WHERE clause for the ON CONFLICT UPDATE */
  Upsert *p_अग्र_Upsert;      /* Next ON CONFLICT clause in the list */
  u8 isDoUpdate;            /* True for DO UPDATE.  False for DO NOTHING */
  u8 isDup;                 /* True if 2nd or later with same pUpsertIdx */
  /* Above this point is the parse tree for the ON CONFLICT clauses.
  ** The next group of fields stores intermediate data. */
  शून्य *p_गन्तव्यम्_Free;            /* Free memory when deleting the Upsert object */
  /* All fields above are owned by the Upsert object and must be freed
  ** when the Upsert is destroyed.  The fields below are used to transfer
  ** information from the INSERT processing down into the UPDATE processing
  ** while generating code.  The fields below are owned by the INSERT
  ** statement and will be freed by INSERT processing. */
  सूचकांक *pUpsertIdx;        /* UNIQUE constraint specified by pUpsertTarget */
  SrcList *pUpsertSrc;      /* Table to be updated */
  पूर्णाङ्क reg_दत्त;              /* First register holding array of VALUES */
  पूर्णाङ्क i_दत्त_Cur;             /* Index of the data cursor */
  पूर्णाङ्क iIdxCur;              /* Index of the first index cursor */
};

/*
** An instance of the following structure contains all information
** needed to generate code for a single SELECT statement.
*/
संरचना Select {
  u8 op;                 /* One of: TK_UNION TK_ALL TK_INTERSECT TK_EXCEPT */
  LogEst nSelectRow;     /* Estimated number of result rows */
  u32 selFlags;          /* Various SF_* values */
  पूर्णाङ्क i_सीमा, i_विस्थापनम्;   /* Memory registers holding LIMIT & OFFSET counters */
  u32 selId;             /* Unique identifier number for this SELECT */
  व्यञ्जकम्_List *pEList;      /* The fields of the result */
  SrcList *pSrc;         /* The FROM clause */
  व्यञ्जकम् *pWhere;          /* The WHERE clause */
  व्यञ्जकम्_List *p_गणः_By;    /* The GROUP BY clause */
  व्यञ्जकम् *pHaving;         /* The HAVING clause */
  व्यञ्जकम्_List *p_क्रमः_By;    /* The ORDER BY clause */
  Select *p_प्राक्;        /* Prior select in a compound select statement */
  Select *p_अग्र;         /* Next select to the left in a compound */
  व्यञ्जकम् *p_सीमा;          /* LIMIT expression. NULL means not used. */
  With *pWith;           /* WITH clause attached to this select. Or NULL. */
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
  गवाक्षः *pWin;          /* List of window functions */
  गवाक्षः *pWinDefn;      /* List of named window definitions */
#यदि_अन्तः
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
#परिभाषय SF_Distinct      0x0000001 /* Output should be DISTINCT */
#परिभाषय SF_All           0x0000002 /* Includes the ALL keyword */
#परिभाषय SF_Resolved      0x0000004 /* Identifiers have been resolved */
#परिभाषय SF_Aggregate     0x0000008 /* Contains agg functions or a GROUP BY */
#परिभाषय SF_HasAgg        0x0000010 /* Contains aggregate functions */
#परिभाषय SF_Cloned_Rhs_अन्तर्   0x0000020 /* Cloned RHS of an IN operator */
#परिभाषय SF_Expanded      0x0000040 /* sqlite3SelectExpand() called on this */
#परिभाषय SF_HasTypeInfo   0x0000080 /* FROM subqueries have Table metadata */
#परिभाषय SF_Compound      0x0000100 /* Part of a compound query */
#परिभाषय SF_मानानि        0x0000200 /* Synthesized from VALUES clause */
#परिभाषय SF_Multi_मानम्    0x0000400 /* Single VALUES term with multiple rows */
#परिभाषय SF_NestedFrom    0x0000800 /* Part of a parenthesized FROM clause */
#परिभाषय SF_MinMaxAgg     0x0001000 /* Aggregate containing min() or max() */
#परिभाषय SF_Recursive     0x0002000 /* The recursive part of a recursive CTE */
#परिभाषय SF_Fixed_सीमा    0x0004000 /* nSelectRow set by a constant LIMIT */
/*                       0x0008000 // available for reuse */
#परिभाषय SF_Converted     0x0010000 /* By convertCompoundSelectToSubquery() */
#परिभाषय SF_IncludeHidden 0x0020000 /* Include hidden columns in output */
#परिभाषय SF_Complex_परिणाम 0x0040000 /* Result contains subquery or function */
#परिभाषय SF_WhereBegin    0x0080000 /* Really a WhereBegin() call.  Debug Only */
#परिभाषय SF_WinRewrite    0x0100000 /* Window function rewrite accomplished */
#परिभाषय SF_View          0x0200000 /* SELECT statement is a view */
/*                       0x0400000 // available for reuse */
#परिभाषय SF_UFSrcCheck    0x0800000 /* Check pSrc as required by UPDATE...FROM */
#परिभाषय SF_प्रक्षिप_अधस्तात्      0x1000000 /* Modified by WHERE-clause push-down opt */
#परिभाषय SF_MultiPart     0x2000000 /* Has multiple incompatible PARTITIONs */
#परिभाषय SF_CopyCte       0x4000000 /* SELECT statement is a copy of a CTE */
#परिभाषय SF_क्रमः_By_Reqd   0x8000000 /* The ORDER BY clause may not be omitted */
#परिभाषय SF_UpdateFrom   0x10000000 /* Query originates with UPDATE FROM */
#परिभाषय SF_Correlated   0x20000000 /* True if references the outer context */
#परिभाषय SF_On_गन्तव्यम्_Where    0x40000000 /* One or more ON clauses moved to WHERE */

/* True if SrcItem X is a subquery that has SF_NestedFrom */
#परिभाषय IsNestedFrom(X) \
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
#परिभाषय SRT_Exists       1  /* Store 1 if the result is not empty */
#परिभाषय SRT_Discard      2  /* Do not save the results anywhere */
#परिभाषय SRT_DistFifo     3  /* Like SRT_Fifo, but unique results only */
#परिभाषय SRT_DistQueue    4  /* Like SRT_Queue, but unique results only */

/* The DISTINCT clause is ignored for all of the above.  Not that
** IgnorableDistinct() implies IgnorableOrderby() */
#परिभाषय IgnorableDistinct(X) ((X->eDest)<=SRT_DistQueue)

#परिभाषय SRT_Queue        5  /* Store result in an queue */
#परिभाषय SRT_Fifo         6  /* Store result as data with an automatic rowid */

/* The ORDER BY clause is ignored for all of the above */
#परिभाषय IgnorableOrderby(X) ((X->eDest)<=SRT_Fifo)

#परिभाषय SRT_Output       7  /* Output each row of result */
#परिभाषय SRT_Mem          8  /* Store result in a memory cell */
#परिभाषय SRT_समुच्चयः          9  /* Store results as keys in an index */
#परिभाषय SRT_EphemTab    10  /* Create transient tab and store like SRT_Table */
#परिभाषय SRT_Coroutine   11  /* Generate a single row of result */
#परिभाषय SRT_सारणी       12  /* Store result as data with an automatic rowid */
#परिभाषय SRT_Upfrom      13  /* Store result as data with rowid */

/*
** An instance of this object describes where to put of the results of
** a SELECT statement.
*/
संरचना SelectDest {
  u8 eDest;            /* How to dispose of the results.  One of SRT_* above. */
  पूर्णाङ्क iSDParm;         /* A parameter used by the eDest disposal method */
  पूर्णाङ्क iSDParm2;        /* A second parameter for the eDest disposal method */
  पूर्णाङ्क iSdst;           /* Base register where results are written */
  पूर्णाङ्क nSdst;           /* Number of registers allocated */
  वर्ण *zAffSdst;      /* Affinity used for SRT_Set */
  व्यञ्जकम्_List *p_क्रमः_By;  /* Key columns for SRT_Queue and SRT_DistQueue */
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
संरचना AutoincInfo {
  AutoincInfo *p_अग्र;   /* Next info block in a list of them all */
  सारणी *pTab;          /* Table this info block refers to */
  पूर्णाङ्क iDb;              /* Index in sqlite3.aDb[] of database holding pTab */
  पूर्णाङ्क regCtr;           /* Memory register holding the rowid counter */
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
संरचना TriggerPrg {
  Trigger *pTrigger;      /* Trigger this program was coded from */
  TriggerPrg *p_अग्र;      /* Next entry in Parse.pTriggerPrg list */
  Sub_क्रमादेशः *p_क्रमादेशः;   /* Program implementing pTrigger/orconf */
  पूर्णाङ्क orconf;             /* Default ON CONFLICT policy */
  u32 aColmask[2];        /* Masks of old.*, new.* columns accessed */
};

/*
** The yDbMask datatype for the bitmask of all attached databases.
*/
#यदि SQLITE_MAX_ATTACHED>30
  प्रकारनिर्देश अचिह्नित वर्ण y_Db_आवरणम्[(SQLITE_MAX_ATTACHED+9)/8];
# define Db_आवरणम्_Test(M,I)    (((M)[(I)/8]&(1<<((I)&7)))!=0)
# define Db_आवरणम्_Zero(M)      स्मृतिं_पूरय((M),0,परिमाण(M))
# define Db_आवरणम्_समुच्चयः(M,I)     (M)[(I)/8]|=(1<<((I)&7))
# define Db_आवरणम्_All_Zero(M)   sqlite3_Db_आवरणम्_All_Zero(M)
# define Db_आवरणम्_Non_Zero(M)   (sqlite3_Db_आवरणम्_All_Zero(M)==0)
#अन्यथा
  प्रकारनिर्देश अचिह्नित पूर्णाङ्क y_Db_आवरणम्;
# define Db_आवरणम्_Test(M,I)    (((M)&(((y_Db_आवरणम्)1)<<(I)))!=0)
# define Db_आवरणम्_Zero(M)      ((M)=0)
# define Db_आवरणम्_समुच्चयः(M,I)     ((M)|=(((y_Db_आवरणम्)1)<<(I)))
# define Db_आवरणम्_All_Zero(M)   ((M)==0)
# define Db_आवरणम्_Non_Zero(M)   ((M)!=0)
#यदि_अन्तः

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
संरचना अनुक्रमितम्_व्यञ्जकम् {
  व्यञ्जकम् *p_व्यञ्जकम्;            /* The expression contained in the index */
  पूर्णाङ्क i_दत्त_Cur;           /* The data cursor associated with the index */
  पूर्णाङ्क iIdxCur;            /* The index cursor */
  पूर्णाङ्क iIdxCol;            /* The index column that contains value of pExpr */
  u8 bMaybeNullRow;       /* True if we need an OP_IfNullRow check */
  u8 aff;                 /* Affinity of the pExpr expression */
  अनुक्रमितम्_व्यञ्जकम् *pIENext;   /* Next in a list of all indexed expressions */
#यदि_परिभाषितम् SQLITE_ENABLE_EXPLAIN_COMMENTS
  ध्रुव वर्ण *zIdxName;   /* Name of index, used only for bytecode comments */
#यदि_अन्तः
};

/*
** An instance of the ParseCleanup object specifies an operation that
** should be performed after parsing to deallocation resources obtained
** during the parse and which are no longer needed.
*/
संरचना ParseCleanup {
  ParseCleanup *p_अग्र;               /* Next cleanup task */
  शून्य *pPtr;                        /* Pointer to object to deallocate */
  शून्य (*xCleanup)(sqlite3*,शून्य*);  /* Deallocation routine */
};

/*
** Number of 64-bit entried in the variable number bitmap cache (VNBMC).
*/
#यदि_अपरिभाषितम् SQLITE_VNBMC
# define SQLITE_VNBMC 2
#यदि_अन्तः

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
संरचना Parse {
  sqlite3 *db;         /* The main database structure */
  वर्ण *z_Err_सन्देशः;       /* An error message */
  Vdbe *pVdbe;         /* An engine for executing database bytecode */
  पूर्णाङ्क rc;              /* Return code from execution */
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
#यदि defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
  u8 earlyCleanup;     /* OOM inside sqlite3ParserAddCleanup() */
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_DEBUG
  u8 ifNotExists;      /* Might be true if IF NOT EXISTS.  Assert()s only */
  u8 isCreate;         /* CREATE TABLE, INDEX, or VIEW (but not TRIGGER)
                       ** and ALTER TABLE ADD COLUMN. */
#यदि_अन्तः
  bft disableTriggers:1; /* True to disable triggers */
  bft mayAbort :1;     /* True if statement may throw an ABORT exception */
  bft hasCompound :1;  /* Need to invoke convertCompoundSelectToSubquery() */
  bft bReturning :1;   /* Coding a RETURNING trigger */
  bft bHasExists :1;   /* Has a correlated "EXISTS (SELECT ....)" expression */
  bft col_नामानि_समुच्चयः :1;  /* TRUE after OP_ColumnName has been issued to pVdbe */
  bft bHasWith :1;     /* True if statement contains WITH */
  bft ok_Const_गुणकः:1; /* OK to factor out constants */
  bft checkSchema :1;  /* Causes schema cookie check after an error */
  bft उपयुङ्क्ते_Ainc :1;     /* True if pAinc is valid */
  पूर्णाङ्क nRangeReg;       /* Size of the temporary register block */
  पूर्णाङ्क iRangeReg;       /* First register in temporary register block */
  पूर्णाङ्क nErr;            /* Number of errors seen */
  पूर्णाङ्क nTab;            /* Number of previously allocated VDBE cursors */
  पूर्णाङ्क nMem;            /* Number of memory cells used so far */
  पूर्णाङ्क szOpAlloc;       /* Bytes of memory space allocated for Vdbe.aOp[] */
  पूर्णाङ्क iSelfTab;        /* Table associated with an index on expr, or negative
                       ** of the base register during check-constraint eval */
  पूर्णाङ्क nNestSel;        /* Number of nested SELECT statements and/or VIEWs */
  पूर्णाङ्क nLabel;          /* The *negative* of the number of labels used */
  पूर्णाङ्क nLabelAlloc;     /* Number of slots in aLabel */
  पूर्णाङ्क *aLabel;         /* Space to hold the labels */
  व्यञ्जकम्_List *p_Const_व्यञ्जकम्;/* Constant expressions */
  अनुक्रमितम्_व्यञ्जकम् *pIdxEpr;/* List of expressions used by active indexes */
  अनुक्रमितम्_व्यञ्जकम् *p_Idx_Part_व्यञ्जकम्; /* Exprs constrained by index WHERE clauses */
  y_Db_आवरणम् write_आवरणम्;   /* Start a write transaction on these databases */
  y_Db_आवरणम् cookie_आवरणम्;  /* Bitmask of schema verified databases */
  पूर्णाङ्क nMaxArg;         /* Max args to xUpdate and xFilter vtab methods */
  पूर्णाङ्क nSelect;         /* Number of SELECT stmts. Counter for Select.selId */
#यदि_अपरिभाषितम् SQLITE_OMIT_प्रगतिः_CALLBACK
  u32 n_प्रगतिः_चरणाः;  /* xProgress steps taken during sqlite3_prepare() */
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_SHARED_CACHE
  पूर्णाङ्क n_सारणी_तालकम्;        /* Number of locks in aTableLock */
#यदि_अन्तः
  Parse *pToplevel;    /* Parse structure for main program (or NULL) */
  सारणी *pTriggerTab;  /* Table triggers are being coded for */
  TriggerPrg *pTriggerPrg;  /* Linked list of coded triggers */
  ParseCleanup *pCleanup;   /* List of cleanup operations to run after parse */
  Trigger *pTrigCoded;      /* Trigger whose sub-program is being coded */

  /**************************************************************************
  ** Fields above must be initialized to zero.  The fields that follow,
  ** down to the beginning of the recursive section, do not need to be
  ** initialized as they will be set before being used.  The boundary is
  ** determined by offsetof(Parse,aTempReg).
  **************************************************************************/

  पूर्णाङ्क a_अस्थायी_Reg[8];        /* Holding area for temporary registers */
  Parse *pOuterParse;     /* Outer Parse object when nested */
  Token sNameToken;       /* Token with unqualified schema object name */
  u32 oldmask;            /* Mask of old.* columns referenced */
  u32 newmask;            /* Mask of new.* columns referenced */
  सङ्घ {
    संरचना {  /* These fields available when isCreate is true */
      पूर्णाङ्क addrCrTab;        /* Address of OP_CreateBtree on CREATE TABLE */
      पूर्णाङ्क regRowid;         /* Register holding rowid of CREATE TABLE entry */
      पूर्णाङ्क reg_मूलम्;          /* Register holding root page for new objects */
      Token constraintName; /* Name of the constraint currently being parsed */
    } cr;
    संरचना {  /* These fields available to all other statements */
      Returning *pReturning; /* The RETURNING clause */
    } d;
  } u1;
  AutoincInfo *pAinc;     /* Information about AUTOINCREMENT counters. Only
                          ** valid if Parse.usesAinc is true */
#यदि_अपरिभाषितम् SQLITE_OMIT_SHARED_CACHE
  सारणी_तालकम् *a_सारणी_तालकम्;  /* Required table locks for shared-cache mode. Only
                          ** valid if Parse.nTableLock>0 */
#यदि_अन्तः


  /************************************************************************
  ** Above is constant between recursions.  Below is reset before and after
  ** each recursion.  The boundary between these two regions is determined
  ** using offsetof(Parse,sLastToken) so the sLastToken field must be the
  ** first field in the recursive region.
  ************************************************************************/

  Token s_चरमः_Token;       /* The last token parsed */
  ynVar nVar;               /* Number of '?' variables seen in the SQL so far */
  u64 aVnbmc[SQLITE_VNBMC]; /* Varible Number Bitmap Cache */
  u8 i_Pk_क्रमय_क्रमः;          /* ASC or DESC for INTEGER PRIMARY KEY */
  u8 explain;               /* True if the EXPLAIN flag is found on the query */
  u8 e_Parse_बहुलकम्;            /* PARSE_MODE_XXX constant */
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
  पूर्णाङ्क n_Vtab_तालकम्;            /* Number of virtual tables to lock */
#यदि_अन्तः
  पूर्णाङ्क nHeight;              /* Expression tree height of current sub-select */
  पूर्णाङ्क addrExplain;          /* Address of current OP_Explain opcode */
  VList *pVList;            /* Mapping between variable names and numbers */
  Vdbe *pReprepare;         /* VM being reprepared (sqlite3Reprepare()) */
  ध्रुव वर्ण *z_पुच्छ;        /* All SQL text past the last semicolon parsed */
  सारणी *p_New_सारणी;         /* A table being constructed by CREATE TABLE */
  सूचकांक *p_New_सूचकांक;         /* An index being constructed by CREATE INDEX.
                            ** Also used to hold redundant UNIQUE constraints
                            ** during a RENAME COLUMN */
  Trigger *pNewTrigger;     /* Trigger under construct by a CREATE TRIGGER */
  ध्रुव वर्ण *zAuthContext; /* The 6th parameter to db->xAuth callbacks */
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
  Token sArg;               /* Complete text of a module argument */
  सारणी **ap_Vtab_तालकम्;       /* Pointer to virtual tables needing locking */
#यदि_अन्तः
  With *pWith;              /* Current WITH clause, or NULL */
#यदि_अपरिभाषितम् SQLITE_OMIT_ALTERTABLE
  RenameToken *pRename;     /* Tokens subject to renaming by ALTER TABLE */
#यदि_अन्तः
};

/* Allowed values for Parse.eParseMode
*/
#परिभाषय PARSE_बहुलकम्_प्रसामान्यम्        0
#परिभाषय PARSE_बहुलकम्_DECLARE_VTAB  1
#परिभाषय PARSE_बहुलकम्_RENAME        2
#परिभाषय PARSE_बहुलकम्_UNMAP         3

/*
** Sizes and pointers of various parts of the Parse object.
*/
#परिभाषय PARSE_HDR(X)  (((वर्ण*)(X))+offsetof(Parse,z_Err_सन्देशः))
#परिभाषय PARSE_HDR_SZ (offsetof(Parse,a_अस्थायी_Reg)-offsetof(Parse,z_Err_सन्देशः)) /* Recursive part w/o aColCache*/
#परिभाषय PARSE_RECURSE_SZ offsetof(Parse,s_चरमः_Token)    /* Recursive part */
#परिभाषय PARSE_पुच्छ_SZ (परिमाण(Parse)-PARSE_RECURSE_SZ) /* Non-recursive part */
#परिभाषय PARSE_पुच्छ(X) (((वर्ण*)(X))+PARSE_RECURSE_SZ)  /* Pointer to tail */

/*
** Return true if currently inside an sqlite3_declare_vtab() call.
*/
#यदि_परिभाषितम् SQLITE_OMIT_VIRTUALTABLE
  #परिभाषय अन्तर्_DECLARE_VTAB 0
#अन्यथा
  #परिभाषय अन्तर्_DECLARE_VTAB (pParse->e_Parse_बहुलकम्==PARSE_बहुलकम्_DECLARE_VTAB)
#यदि_अन्तः

#यदि defined(SQLITE_OMIT_ALTERTABLE)
  #परिभाषय अन्तर्_RENAME_OBJECT 0
#अन्यथा
  #परिभाषय अन्तर्_RENAME_OBJECT (pParse->e_Parse_बहुलकम्>=PARSE_बहुलकम्_RENAME)
#यदि_अन्तः

#यदि defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_OMIT_ALTERTABLE)
  #परिभाषय अन्तर्_SPECIAL_PARSE 0
#अन्यथा
  #परिभाषय अन्तर्_SPECIAL_PARSE (pParse->e_Parse_बहुलकम्!=PARSE_बहुलकम्_प्रसामान्यम्)
#यदि_अन्तः

/*
** An instance of the following structure can be declared on a stack and used
** to save the Parse.zAuthContext value so that it can be restored later.
*/
संरचना AuthContext {
  ध्रुव वर्ण *zAuthContext;   /* Put saved Parse.zAuthContext here */
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
#परिभाषय OPFLAG_NCHANGE       0x01    /* OP_Insert: Set to update db->nChange */
                                     /* Also used in P2 (not P5) of OP_Delete */
#परिभाषय OPFLAG_NOCHNG        0x01    /* OP_VColumn nochange for UPDATE */
#परिभाषय OPFLAG_EPHEM         0x01    /* OP_Column: Ephemeral output is ok */
#परिभाषय OPFLAG_LASTROWID     0x20    /* Set to update db->lastRowid */
#परिभाषय OPFLAG_ISUPDATE      0x04    /* This OP_Insert is an sql UPDATE */
#परिभाषय OPFLAG_APPEND        0x08    /* This is likely to be an append */
#परिभाषय OPFLAG_USESEEKRESULT 0x10    /* Try to avoid a seek in BtreeInsert() */
#परिभाषय OPFLAG_ISNOOP        0x40    /* OP_Delete does pre-update-hook only */
#परिभाषय OPFLAG_LENGTHARG     0x40    /* OP_Column only used for length() */
#परिभाषय OPFLAG_TYPEOFARG     0x80    /* OP_Column only used for typeof() */
#परिभाषय OPFLAG_BYTELENARG    0xc0    /* OP_Column only for octet_length() */
#परिभाषय OPFLAG_BULKCSR       0x01    /* OP_Open** used to open bulk cursor */
#परिभाषय OPFLAG_SEEKEQ        0x02    /* OP_Open** cursor uses EQ seek only */
#परिभाषय OPFLAG_FORDELETE     0x08    /* OP_Open should use BTREE_FORDELETE */
#परिभाषय OPFLAG_P2ISREG       0x10    /* P2 to OP_Open** is a register number */
#परिभाषय OPFLAG_PERMUTE       0x01    /* OP_Compare: use the permutation */
#परिभाषय OPFLAG_SAVEPOSITION  0x02    /* OP_Delete/Insert: save cursor pos */
#परिभाषय OPFLAG_AUXDELETE     0x04    /* OP_Delete: index in a DELETE op */
#परिभाषय OPFLAG_NOCHNG_MAGIC  0x6d    /* OP_MakeRecord: serialtype 10 is ok */
#परिभाषय OPFLAG_PREFORMAT     0x80    /* OP_Insert uses preformatted cell */

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
संरचना Trigger {
  वर्ण *zName;            /* The name of the trigger                        */
  वर्ण *सारणी_2;            /* The table or view to which the trigger applies */
  u8 op;                  /* One of TK_DELETE, TK_UPDATE, TK_INSERT         */
  u8 tr_tm;               /* One of TRIGGER_BEFORE, TRIGGER_AFTER */
  u8 bReturning;          /* This trigger implements a RETURNING clause */
  व्यञ्जकम् *pWhen;            /* The WHEN clause of the expression (may be NULL) */
  IdList *pColumns;       /* If this is an UPDATE OF <column-list> trigger,
                             the <column-list> is stored here */
  Schema *pSchema;        /* Schema containing the trigger */
  Schema *pTabSchema;     /* Schema containing the table */
  Trigger_चरणः *चरणः_list; /* Link list of trigger program steps             */
  Trigger *p_अग्र;         /* Next trigger associated with the table */
};

/*
** A trigger is either a BEFORE or an AFTER trigger.  The following constants
** determine which.
**
** If there are multiple triggers, you might of some BEFORE and some AFTER.
** In that cases, the constants below can be ORed together.
*/
#परिभाषय TRIGGER_BEFORE  1
#परिभाषय TRIGGER_AFTER   2

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
संरचना Trigger_चरणः {
  u8 op;               /* One of TK_DELETE, TK_UPDATE, TK_INSERT, TK_SELECT,
                       ** or TK_RETURNING */
  u8 orconf;           /* OE_Rollback etc. */
  Trigger *pTrig;      /* The trigger that this step is a part of */
  Select *pSelect;     /* SELECT statement or RHS of INSERT INTO SELECT ... */
  SrcList *pSrc;       /* Table to insert/update/delete */
  व्यञ्जकम् *pWhere;        /* The WHERE clause for DELETE or UPDATE steps */
  व्यञ्जकम्_List *p_व्यञ्जकम्_List; /* SET clause for UPDATE, or RETURNING clause */
  IdList *pIdList;     /* Column names for INSERT */
  Upsert *pUpsert;     /* Upsert clauses on an INSERT */
  वर्ण *z_विस्तृतिः;         /* Original SQL text of this command */
  Trigger_चरणः *p_अग्र;  /* Next in the link-list */
  Trigger_चरणः *p_चरमः;  /* Last element in link-list. Valid for 1st elem only */
};

/*
** Information about a RETURNING clause
*/
संरचना Returning {
  Parse *pParse;        /* The parse that includes the RETURNING clause */
  व्यञ्जकम्_List *pReturnEL;  /* List of expressions to return */
  Trigger retTrig;      /* The transient trigger that implements RETURNING */
  Trigger_चरणः retTStep; /* The trigger step */
  पूर्णाङ्क iRetCur;          /* Transient table holding RETURNING results */
  पूर्णाङ्क nRetCol;          /* Number of in pReturnEL after expansion */
  पूर्णाङ्क iRetReg;          /* Register array for holding a row of RETURNING */
  वर्ण zName[40];       /* Name of trigger: "sqlite_returning_%p" */
};

/*
** An object used to accumulate the text of a string where we
** do not necessarily know how big the string will be in the end.
*/
संरचना sqlite3_str {
  sqlite3 *db;         /* Optional database for lookaside.  Can be NULL */
  वर्ण *zText;         /* The string collected so far */
  u32  nAlloc;         /* Amount of space allocated in zText */
  u32  mxAlloc;        /* Maximum allowed allocation.  0 for no malloc usage */
  u32  nChar;          /* Length of the string so far */
  u8   accError;       /* SQLITE_NOMEM or SQLITE_TOOBIG */
  u8   printfFlags;    /* SQLITE_PRINTF flags below */
};
#परिभाषय SQLITE_PRINTF_आन्तरिकम् 0x01  /* Internal-use-only converters allowed */
#परिभाषय SQLITE_PRINTF_SQLFUNC  0x02  /* SQL function arguments to VXPrintf */
#परिभाषय SQLITE_PRINTF_MALLOCED 0x04  /* True if zText is allocated space */

#परिभाषय isMalloced(X)  (((X)->printfFlags & SQLITE_PRINTF_MALLOCED)!=0)

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
संरचना RCStr {
  u64 nRCRef;            /* Number of references */
  /* Total structure size should be a multiple of 8 bytes for alignment */
};

/*
** A pointer to this structure is used to communicate information
** from sqlite3Init and OP_ParseSchema into the sqlite3InitCallback.
*/
प्रकारनिर्देश संरचना {
  sqlite3 *db;        /* The database being initialized */
  वर्ण **pz_Err_सन्देशः;    /* Error message stored here */
  पूर्णाङ्क iDb;            /* 0 for main database.  1 for TEMP, 2.. for ATTACHed */
  पूर्णाङ्क rc;             /* Result code stored here */
  u32 mInitFlags;     /* Flags controlling error messages */
  u32 nInitRow;       /* Number of rows processed */
  Pgno mxPage;        /* Maximum page number.  0 for no limit. */
} Init_दत्त;

/*
** Allowed values for mInitFlags
*/
#परिभाषय INITFLAG_Alter_आवरणम्     0x0007  /* Types of ALTER */
#परिभाषय INITFLAG_AlterRename   0x0001  /* Reparse after a RENAME */
#परिभाषय INITFLAG_AlterDrop     0x0002  /* Reparse after a DROP COLUMN */
#परिभाषय INITFLAG_Alter_सङ्कलय      0x0003  /* Reparse after an ADD COLUMN */
#परिभाषय INITFLAG_AlterDropCons 0x0004  /* Reparse after a DROP CONSTRAINT */

/* Tuning parameters are set using SQLITE_TESTCTRL_TUNE and are controlled
** on debug-builds of the CLI using ".testctrl tune ID VALUE".  Tuning
** parameters are for temporary use during development, to help find
** optimal values for parameters in the query planner.  The should not
** be used on trunk check-ins.  They are a temporary mechanism available
** for transient development builds only.
**
** Tuning parameters are numbered starting with 1.
*/
#परिभाषय SQLITE_NTUNE  6             /* Should be zero for all trunk check-ins */
#यदि_परिभाषितम् SQLITE_DEBUG
# define Tuning(X)  (sqlite3Config.aTune[(X)-1])
#अन्यथा
# define Tuning(X)  0
#यदि_अन्तः

/*
** Structure containing global configuration data for the SQLite library.
**
** This structure also contains some state information.
*/
संरचना Sqlite3Config {
  पूर्णाङ्क bMemstat;                     /* True to enable memory status */
  u8 bCoreMutex;                    /* True to enable core mutexing */
  u8 b_पूर्णम्_Mutex;                    /* True to enable full mutexing */
  u8 bOpenUri;                      /* True to interpret filenames as URIs */
  u8 bUseCis;                       /* Use covering indices for full-scans */
  u8 bSmallMalloc;                  /* Avoid large memory allocations if true */
  u8 bExtraSchemaChecks;            /* Verify type,name,tbl_name in schema */
#यदि_परिभाषितम् SQLITE_DEBUG
  u8 bJsonSelfcheck;                /* Double-check JSON parsing */
#यदि_अन्तः
  पूर्णाङ्क mxStrlen;                     /* Maximum string length */
  पूर्णाङ्क neverCorrupt;                 /* Database is always well-formed */
  पूर्णाङ्क szLookaside;                  /* Default lookaside buffer size */
  पूर्णाङ्क nLookaside;                   /* Default lookaside buffer count */
  पूर्णाङ्क nStmtSpill;                   /* Stmt-journal spill-to-disk threshold */
  sqlite3_mem_methods m;            /* Low-level memory allocation interface */
  sqlite3_mutex_methods mutex;      /* Low-level mutex interface */
  sqlite3_pcache_methods2 pcache2;  /* Low-level page-cache interface */
  शून्य *pHeap;                      /* Heap storage space */
  पूर्णाङ्क nHeap;                        /* Size of pHeap[] */
  पूर्णाङ्क mnReq, mxReq;                 /* Min and max heap requests sizes */
  sqlite3_int64 szMmap;             /* mmap() space per open file */
  sqlite3_int64 mxMmap;             /* Maximum value for szMmap */
  शून्य *pPage;                      /* Page cache memory */
  पूर्णाङ्क szPage;                       /* Size of each page in pPage[] */
  पूर्णाङ्क nPage;                        /* Number of pages in pPage[] */
  पूर्णाङ्क mx_Parser_राशिः;                /* maximum depth of the parser stack */
  पूर्णाङ्क sharedCacheEnabled;           /* true if shared-cache mode enabled */
  u32 szPma;                        /* Maximum Sorter PMA size */
  /* The above might be initialized to non-zero.  The following need to always
  ** initially be zero, however. */
  पूर्णाङ्क isInit;                       /* True after initialization has finished */
  पूर्णाङ्क अन्तर्_प्रगतिः;                   /* True while initialization in progress */
  पूर्णाङ्क isMutexInit;                  /* True after mutexes are initialized */
  पूर्णाङ्क isMallocInit;                 /* True after malloc is initialized */
  पूर्णाङ्क isPCacheInit;                 /* True after malloc is initialized */
  पूर्णाङ्क nRefInitMutex;                /* Number of users of pInitMutex */
  sqlite3_mutex *pInitMutex;        /* Mutex used by sqlite3_initialize() */
  शून्य (*xLog)(शून्य*,पूर्णाङ्क,ध्रुव वर्ण*); /* Function for logging */
  शून्य *pLogArg;                       /* First argument to xLog() */
#यदि_परिभाषितम् SQLITE_ENABLE_SQLLOG
  शून्य(*xSqllog)(शून्य*,sqlite3*,ध्रुव वर्ण*, पूर्णाङ्क);
  शून्य *pSqllogArg;
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_VDBE_COVERAGE
  /* The following callback (if not NULL) is invoked on every VDBE branch
  ** operation.  Set the callback using SQLITE_TESTCTRL_VDBE_COVERAGE.
  */
  शून्य (*xVdbeBranch)(शून्य*,अचिह्नित i_Src_पङ्क्तिः,u8 eThis,u8 eMx);  /* Callback */
  शून्य *pVdbeBranchArg;                                     /* 1st argument */
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_DESERIALIZE
  sqlite3_int64 mx_Memdb_आकार;        /* Default max memdb size */
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_UNTESTABLE
  पूर्णाङ्क (*xTestCallback)(पूर्णाङ्क);        /* Invoked by sqlite3FaultSim() */
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_ALLOW_ROWID_अन्तर्_VIEW
  u32 mNoVisibleRowid;              /* TF_NoVisibleRowid if the ROWID_IN_VIEW
                                    ** feature is disabled.  0 if rowids can
                                    ** occur in views. */
#यदि_अन्तः
  पूर्णाङ्क bLocaltimeFault;              /* True to fail localtime() calls */
  पूर्णाङ्क (*xAltLocaltime)(ध्रुव शून्य*,शून्य*); /* Alternative localtime() routine */
  पूर्णाङ्क i_Once_Reset_देहली;          /* When to reset OP_Once counters */
  u32 szSorterRef;                  /* Min size in bytes to use sorter-refs */
  अचिह्नित पूर्णाङ्क i_Prng_बीजम्;           /* Alternative fixed seed for the PRNG */
  /* vvvv--- must be last ---vvv */
#यदि_परिभाषितम् SQLITE_DEBUG
  sqlite3_int64 aTune[SQLITE_NTUNE]; /* Tuning parameters */
#यदि_अन्तः
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
#परिभाषय CORRUPT_DB  (sqlite3Config.neverCorrupt==0)

/*
** Context pointer passed down through the tree-walk.
*/
संरचना Walker {
  Parse *pParse;                            /* Parser context.  */
  पूर्णाङ्क (*x_व्यञ्जकम्_Callback)(Walker*, व्यञ्जकम्*);     /* Callback for expressions */
  पूर्णाङ्क (*xSelectCallback)(Walker*,Select*);  /* Callback for SELECTs */
  शून्य (*xSelectCallback2)(Walker*,Select*);/* Second callback for SELECTs */
  पूर्णाङ्क walker_गभीरता;                          /* Number of subqueries */
  u16 e_संहिता;                                /* A small processing code */
  u16 mWFlags;                              /* Use-dependent flags */
  सङ्घ {                                   /* Extra data for callback */
    NameContext *pNC;                         /* Naming context */
    पूर्णाङ्क n;                                    /* A counter */
    पूर्णाङ्क iCur;                                 /* A cursor number */
    पूर्णाङ्क sz;                                   /* String literal length */
    SrcList *pSrcList;                        /* FROM clause */
    संरचना CCurHint *pCCurHint;               /* Used by codeCursorHint() */
    संरचना RefSrcList *pRefSrcList;           /* sqlite3ReferencesSrcList() */
    पूर्णाङ्क *aiCol;                               /* array of column indexes */
    संरचना IdxCover *pIdxCover;               /* Check for index coverage */
    व्यञ्जकम्_List *p_गणः_By;                       /* GROUP BY clause */
    Select *pSelect;                          /* HAVING to WHERE clause ctx */
    संरचना गवाक्षः_Rewrite *pRewrite;           /* Window rewrite context */
    संरचना WhereConst *pConst;                /* WHERE clause constants */
    संरचना RenameCtx *pRename;                /* RENAME COLUMN context */
    संरचना सारणी *pTab;                       /* Table of generated column */
    संरचना Covering_सूचकांक_Check *pCovIdxCk;     /* Check for covering index */
    SrcItem *pSrcItem;                        /* A single FROM clause item */
    DbFixer *pFix;                            /* See sqlite3FixSelect() */
    Mem *aMem;                                /* See sqlite3BtreeCursorHint() */
    संरचना CheckOnCtx *pCheckOnCtx;           /* See selectCheckOnClauses() */
  } u;
};

/*
** The following structure contains information used by the sqliteFix...
** routines as they walk the parse tree to make database references
** explicit.
*/
संरचना DbFixer {
  Parse *pParse;      /* The parsing context.  Error messages written here */
  Walker w;           /* Walker object */
  Schema *pSchema;    /* Fix items to this schema */
  u8 b_अस्थायी;           /* True for TEMP schema entries */
  ध्रुव वर्ण *zDb;    /* Make sure all objects are contained in this database */
  ध्रुव वर्ण *zType;  /* Type of the container - used for error messages */
  ध्रुव Token *pName; /* Name of the container - used for error messages */
};

/* Forward declarations */
पूर्णाङ्क sqlite3_Walk_व्यञ्जकम्(Walker*, व्यञ्जकम्*);
पूर्णाङ्क sqlite3_Walk_व्यञ्जकम्_NN(Walker*, व्यञ्जकम्*);
पूर्णाङ्क sqlite3_Walk_व्यञ्जकम्_List(Walker*, व्यञ्जकम्_List*);
पूर्णाङ्क sqlite3WalkSelect(Walker*, Select*);
पूर्णाङ्क sqlite3_Walk_Select_व्यञ्जकम्(Walker*, Select*);
पूर्णाङ्क sqlite3WalkSelectFrom(Walker*, Select*);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Walk_Noop(Walker*, व्यञ्जकम्*);
पूर्णाङ्क sqlite3SelectWalkNoop(Walker*, Select*);
पूर्णाङ्क sqlite3SelectWalkFail(Walker*, Select*);
पूर्णाङ्क sqlite3_Walker_गभीरता_Increase(Walker*,Select*);
शून्य sqlite3_Walker_गभीरता_Decrease(Walker*,Select*);
शून्य sqlite3WalkWinDefnDummyCallback(Walker*,Select*);

#यदि_परिभाषितम् SQLITE_DEBUG
शून्य sqlite3SelectWalkAssert2(Walker*, Select*);
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_CTE
शून्य sqlite3_Select_निष्कासय_With(Walker*, Select*);
#अन्यथा
# define sqlite3_Select_निष्कासय_With 0
#यदि_अन्तः

/*
** Return code from the parse-tree walking primitives and their
** callbacks.
*/
#परिभाषय WRC_Continue    0   /* Continue down into children */
#परिभाषय WRC_Prune       1   /* Omit children but continue walking siblings */
#परिभाषय WRC_Abort       2   /* Abandon the tree walk */

/*
** A single common table expression
*/
संरचना Cte {
  वर्ण *zName;            /* Name of this CTE */
  व्यञ्जकम्_List *p_स्तम्भसङ्ख्या;        /* List of explicit column names, or NULL */
  Select *pSelect;        /* The definition of this CTE */
  ध्रुव वर्ण *zCteErr;    /* Error message for circular references */
  CteUse *pUse;           /* Usage information for this CTE */
  u8 eM10d;               /* The MATERIALIZED flag */
};

/*
** Allowed values for the materialized flag (eM10d):
*/
#परिभाषय M10d_Yes       0  /* AS MATERIALIZED */
#परिभाषय M10d_Any       1  /* Not specified.  Query planner's choice */
#परिभाषय M10d_No        2  /* AS NOT MATERIALIZED */

/*
** An instance of the With object represents a WITH clause containing
** one or more CTEs (common table expressions).
*/
संरचना With {
  पूर्णाङ्क nCte;               /* Number of CTEs in the WITH clause */
  पूर्णाङ्क bView;              /* Belongs to the outermost Select of a view */
  With *pOuter;           /* Containing WITH clause, or NULL */
  Cte a[FLEXARRAY];       /* For each CTE in the WITH clause.... */
};

/* The size (in bytes) of a With object that can hold as many
** as N different CTEs. */
#परिभाषय SZ_WITH(N)  (offsetof(With,a) + (N)*परिमाण(Cte))

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
संरचना CteUse {
  पूर्णाङ्क nUse;              /* Number of users of this CTE */
  पूर्णाङ्क addrM9e;           /* Start of subroutine to compute materialization */
  पूर्णाङ्क regRtn;            /* Return address register for addrM9e subroutine */
  पूर्णाङ्क iCur;              /* Ephemeral table holding the materialization */
  LogEst nRowEst;        /* Estimated number of rows in the table */
  u8 eM10d;              /* The MATERIALIZED flag */
};


/* Client data associated with sqlite3_set_clientdata() and
** sqlite3_get_clientdata().
*/
संरचना Db_Client_दत्त {
  Db_Client_दत्त *p_अग्र;        /* Next in a linked list */
  शून्य *p_दत्त;                /* The data */
  शून्य (*xDestructor)(शून्य*); /* Destructor.  Might be NULL */
  वर्ण zName[FLEXARRAY];      /* Name of this client data. MUST BE LAST */
};

/* The size (in bytes) of a DbClientData object that can has a name
** that is N bytes long, including the zero-terminator. */
#परिभाषय SZ_DBCLIENTDATA(N) (offsetof(Db_Client_दत्त,zName)+(N))

#यदि_परिभाषितम् SQLITE_DEBUG
/*
** An instance of the TreeView object is used for printing the content of
** data structures on sqlite3DebugPrintf() using a tree-like view.
*/
संरचना वृक्षः_View {
  पूर्णाङ्क iLevel;             /* Which level of the tree we are on */
  u8  b_पङ्क्तिः[100];         /* Draw vertical in column i if bLine[i] is true */
};
#यदि_अन्तः /* SQLITE_DEBUG */

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
संरचना गवाक्षः {
  वर्ण *zName;            /* Name of window (may be NULL) */
  वर्ण *z_आधारः;            /* Name of base window for chaining (may be NULL) */
  व्यञ्जकम्_List *pPartition;   /* PARTITION BY clause */
  व्यञ्जकम्_List *p_क्रमः_By;     /* ORDER BY clause */
  u8 eFrmType;            /* TK_RANGE, TK_GROUPS, TK_ROWS, or 0 */
  u8 eStart;              /* UNBOUNDED, CURRENT, PRECEDING or FOLLOWING */
  u8 e_अन्तः;                /* UNBOUNDED, CURRENT, PRECEDING or FOLLOWING */
  u8 bImplicitFrame;      /* True if frame was implicitly specified */
  u8 eExclude;            /* TK_NO, TK_CURRENT, TK_TIES, TK_GROUP, or 0 */
  व्यञ्जकम् *pStart;           /* Expression for "<expr> PRECEDING" */
  व्यञ्जकम् *p_अन्तः;             /* Expression for "<expr> FOLLOWING" */
  गवाक्षः **ppThis;        /* Pointer to this object in Select.pWin list */
  गवाक्षः *p_अग्र_Win;       /* Next window function belonging to this SELECT */
  व्यञ्जकम् *pFilter;          /* The FILTER expression */
  FuncDef *pWFunc;        /* The function */
  पूर्णाङ्क iEphCsr;            /* Partition buffer or Peer buffer */
  पूर्णाङ्क regAccum;           /*सञ्चायकः*/
  पूर्णाङ्क reg_परिणाम;          /* Interim result */
  पूर्णाङ्क csrApp;             /* Function cursor (used by min/max) */
  पूर्णाङ्क regApp;             /* Function register (also used by min/max) */
  पूर्णाङ्क regPart;            /* Array of registers for PARTITION BY values */
  व्यञ्जकम् *pOwner;           /* Expression object this window is attached to */
  पूर्णाङ्क n_बफर_Col;         /* Number of columns in buffer table */
  पूर्णाङ्क iArgCol;            /* Offset of first argument for this function */
  पूर्णाङ्क regOne;             /* Register containing constant value 1 */
  पूर्णाङ्क regStartRowid;
  पूर्णाङ्क reg_अन्तः_Rowid;
  u8 b_व्यञ्जकम्_तर्काः;           /* Defer evaluation of window function arguments
                          ** due to the SQLITE_SUBTYPE flag */
};

Select *sqlite3_Multi_मानानि(Parse *pParse, Select *p_वामः, व्यञ्जकम्_List *pRow);
शून्य sqlite3_Multi_मानानि_अन्तः(Parse *pParse, Select *pVal);

#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
शून्य sqlite3_गवाक्षः_Delete(sqlite3*, गवाक्षः*);
शून्य sqlite3_गवाक्षः_Unlink_From_Select(गवाक्षः*);
शून्य sqlite3_गवाक्षः_List_Delete(sqlite3 *db, गवाक्षः *p);
गवाक्षः *sqlite3_गवाक्षः_Alloc(Parse*, पूर्णाङ्क, पूर्णाङ्क, व्यञ्जकम्*, पूर्णाङ्क , व्यञ्जकम्*, u8);
शून्य sqlite3_गवाक्षः_Attach(Parse*, व्यञ्जकम्*, गवाक्षः*);
शून्य sqlite3_गवाक्षः_Link(Select *pSel, गवाक्षः *pWin);
पूर्णाङ्क sqlite3_गवाक्षः_Compare(ध्रुव Parse*, ध्रुव गवाक्षः*, ध्रुव गवाक्षः*, पूर्णाङ्क);
शून्य sqlite3_गवाक्षः_संहिता_Init(Parse*, Select*);
शून्य sqlite3_गवाक्षः_संहिता_चरणः(Parse*, Select*, WhereInfo*, पूर्णाङ्क, पूर्णाङ्क);
पूर्णाङ्क sqlite3_गवाक्षः_Rewrite(Parse*, Select*);
शून्य sqlite3_गवाक्षः_Update(Parse*, गवाक्षः*, गवाक्षः*, FuncDef*);
गवाक्षः *sqlite3_गवाक्षः_Dup(sqlite3 *db, व्यञ्जकम् *pOwner, गवाक्षः *p);
गवाक्षः *sqlite3_गवाक्षः_List_Dup(sqlite3 *db, गवाक्षः *p);
शून्य sqlite3_गवाक्षः_Functions(शून्य);
शून्य sqlite3_गवाक्षः_Chain(Parse*, गवाक्षः*, गवाक्षः*);
गवाक्षः *sqlite3_गवाक्षः_Assemble(Parse*, गवाक्षः*, व्यञ्जकम्_List*, व्यञ्जकम्_List*, Token*);
#अन्यथा
# define sqlite3_गवाक्षः_Delete(a,b)
# define sqlite3_गवाक्षः_Functions()
# define sqlite3_गवाक्षः_Attach(a,b,c)
#यदि_अन्तः

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
#परिभाषय SQLITE_SKIP_UTF8(z_अन्तर्) {                        \
  यदि( (*(z_अन्तर्++))>=0xc0 ){                              \
    यावत्( (*z_अन्तर् & 0xc0)==0x80 ){ z_अन्तर्++; }             \
  }                                                    \
}

/*
** The SQLITE_*_BKPT macros are substitutes for the error codes with
** the same name but without the _BKPT suffix.  These macros invoke
** routines that report the line-number on which the error originated
** using sqlite3_log().  The routines also provide a convenient place
** to set a debugger breakpoint.
*/
पूर्णाङ्क sqlite3ReportError(पूर्णाङ्क iErr, पूर्णाङ्क lineno, ध्रुव वर्ण *zType);
पूर्णाङ्क sqlite3CorruptError(पूर्णाङ्क);
पूर्णाङ्क sqlite3MisuseError(पूर्णाङ्क);
पूर्णाङ्क sqlite3CantopenError(पूर्णाङ्क);
#परिभाषय SQLITE_CORRUPT_BKPT sqlite3CorruptError(__LINE__)
#परिभाषय SQLITE_MISUSE_BKPT sqlite3MisuseError(__LINE__)
#परिभाषय SQLITE_CANTOPEN_BKPT sqlite3CantopenError(__LINE__)
#यदि_परिभाषितम् SQLITE_DEBUG
  पूर्णाङ्क sqlite3NomemError(पूर्णाङ्क);
  पूर्णाङ्क sqlite3IoerrnomemError(पूर्णाङ्क);
# define SQLITE_NOMEM_BKPT sqlite3NomemError(__LINE__)
# define SQLITE_IOERR_NOMEM_BKPT sqlite3IoerrnomemError(__LINE__)
#अन्यथा
# define SQLITE_NOMEM_BKPT SQLITE_NOMEM
# define SQLITE_IOERR_NOMEM_BKPT SQLITE_IOERR_NOMEM
#यदि_अन्तः
#यदि defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_CORRUPT_PGNO)
  पूर्णाङ्क sqlite3CorruptPgnoError(पूर्णाङ्क,Pgno);
# define SQLITE_CORRUPT_PGNO(P) sqlite3CorruptPgnoError(__LINE__,(P))
#अन्यथा
# define SQLITE_CORRUPT_PGNO(P) sqlite3CorruptError(__LINE__)
#यदि_अन्तः

/*
** FTS3 and FTS4 both require virtual table support
*/
#यदि defined(SQLITE_OMIT_VIRTUALTABLE)
# undef SQLITE_ENABLE_FTS3
# undef SQLITE_ENABLE_FTS4
#यदि_अन्तः

/*
** FTS4 is really an extension for FTS3.  It is enabled using the
** SQLITE_ENABLE_FTS3 macro.  But to avoid confusion we also call
** the SQLITE_ENABLE_FTS4 macro to serve as an alias for SQLITE_ENABLE_FTS3.
*/
#यदि defined(SQLITE_ENABLE_FTS4) && !defined(SQLITE_ENABLE_FTS3)
# define SQLITE_ENABLE_FTS3 1
#यदि_अन्तः

/*
** The following macros mimic the standard library functions toupper(),
** isspace(), isalnum(), isdigit() and isxdigit(), respectively. The
** sqlite versions only work for ASCII characters, regardless of locale.
*/
#यदि_परिभाषितम् SQLITE_ASCII
# define sqlite3Toupper(x)  ((x)&~(sqlite3CtypeMap[(अचिह्नित वर्ण)(x)]&0x20))
# define sqlite3Isspace(x)   (sqlite3CtypeMap[(अचिह्नित वर्ण)(x)]&0x01)
# define sqlite3Isalnum(x)   (sqlite3CtypeMap[(अचिह्नित वर्ण)(x)]&0x06)
# define sqlite3Isalpha(x)   (sqlite3CtypeMap[(अचिह्नित वर्ण)(x)]&0x02)
# define sqlite3Isdigit(x)   (sqlite3CtypeMap[(अचिह्नित वर्ण)(x)]&0x04)
# define sqlite3Isxdigit(x)  (sqlite3CtypeMap[(अचिह्नित वर्ण)(x)]&0x08)
# define sqlite3Tolower(x)   (sqlite3_ऊर्ध्वस्थम्_गन्तव्यम्_अधस्तनम्[(अचिह्नित वर्ण)(x)])
# define sqlite3Isquote(x)   (sqlite3CtypeMap[(अचिह्नित वर्ण)(x)]&0x80)
# define sqlite3JsonId1(x)   (sqlite3CtypeMap[(अचिह्नित वर्ण)(x)]&0x42)
# define sqlite3JsonId2(x)   (sqlite3CtypeMap[(अचिह्नित वर्ण)(x)]&0x46)
#अन्यथा
# define sqlite3Toupper(x)   महाक्षरम्((अचिह्नित वर्ण)(x))
# define sqlite3Isspace(x)   रिक्तम्_वा((अचिह्नित वर्ण)(x))
# define sqlite3Isalnum(x)   अक्षरम्_अङ्कः_वा((अचिह्नित वर्ण)(x))
# define sqlite3Isalpha(x)   अक्षरम्_वा((अचिह्नित वर्ण)(x))
# define sqlite3Isdigit(x)   अङ्कः_वा((अचिह्नित वर्ण)(x))
# define sqlite3Isxdigit(x)  isxdigit((अचिह्नित वर्ण)(x))
# define sqlite3Tolower(x)   tolower((अचिह्नित वर्ण)(x))
# define sqlite3Isquote(x)   ((x)=='"'||(x)=='\''||(x)=='['||(x)=='`')
# define sqlite3JsonId1(x)   (sqlite3IsIdChar(x)&&(x)<'0')
# define sqlite3JsonId2(x)   sqlite3IsIdChar(x)
#यदि_अन्तः
पूर्णाङ्क sqlite3IsIdChar(u8);

/*
** Internal function prototypes
*/
पूर्णाङ्क sqlite3StrICmp(ध्रुव वर्ण*,ध्रुव वर्ण*);
पूर्णाङ्क sqlite3Strlen30(ध्रुव वर्ण*);
#परिभाषय sqlite3Strlen30NN(C) (वाक्यदैर्घ्यम्(C)&0x3fffffff)
वर्ण *sqlite3ColumnType(Column*,वर्ण*);
#परिभाषय sqlite3StrNICmp sqlite3_strnicmp

पूर्णाङ्क sqlite3MallocInit(शून्य);
शून्य sqlite3_Malloc_अन्तः(शून्य);
शून्य *sqlite3Malloc(u64);
शून्य *sqlite3MallocZero(u64);
शून्य *sqlite3DbMallocZero(sqlite3*, u64);
शून्य *sqlite3DbMallocRaw(sqlite3*, u64);
शून्य *sqlite3DbMallocRawNN(sqlite3*, u64);
वर्ण *sqlite3DbStrDup(sqlite3*,ध्रुव वर्ण*);
वर्ण *sqlite3DbStrNDup(sqlite3*,ध्रुव वर्ण*, u64);
वर्ण *sqlite3_Db_विस्तृतिः_Dup(sqlite3*,ध्रुव वर्ण*,ध्रुव वर्ण*);
शून्य *sqlite3Realloc(शून्य*, u64);
शून्य *sqlite3DbReallocOrFree(sqlite3 *, शून्य *, u64);
शून्य *sqlite3DbRealloc(sqlite3 *, शून्य *, u64);
शून्य sqlite3DbFree(sqlite3*, शून्य*);
शून्य sqlite3DbFreeNN(sqlite3*, शून्य*);
शून्य sqlite3DbNNFreeNN(sqlite3*, शून्य*);
पूर्णाङ्क sqlite3_Malloc_आकार(ध्रुव शून्य*);
पूर्णाङ्क sqlite3_Db_Malloc_आकार(sqlite3*, ध्रुव शून्य*);
शून्य *sqlite3PageMalloc(पूर्णाङ्क);
शून्य sqlite3PageFree(शून्य*);
शून्य sqlite3_Mem_समुच्चयः_Default(शून्य);
#यदि_अपरिभाषितम् SQLITE_UNTESTABLE
शून्य sqlite3BenignMallocHooks(शून्य (*)(शून्य), शून्य (*)(शून्य));
#यदि_अन्तः
पूर्णाङ्क sqlite3_Heap_Nearly_पूर्णम्(शून्य);

/*
** On systems with ample stack space and that support alloca(), make
** use of alloca() to obtain space for large automatic objects.  By default,
** obtain space from malloc().
**
** The alloca() routine never returns NULL.  This will cause code paths
** that deal with sqlite3StackAlloc() failures to be unreachable.
*/
#यदि_परिभाषितम् SQLITE_USE_ALLOCA
# define sqlite3_राशिः_Alloc_Raw(D,N)   alloca(N)
# define sqlite3_राशिः_Alloc_Raw_NN(D,N) alloca(N)
# define sqlite3_राशिः_Free(D,P)
# define sqlite3_राशिः_Free_NN(D,P)
#अन्यथा
# define sqlite3_राशिः_Alloc_Raw(D,N)   sqlite3DbMallocRaw(D,N)
# define sqlite3_राशिः_Alloc_Raw_NN(D,N) sqlite3DbMallocRawNN(D,N)
# define sqlite3_राशिः_Free(D,P)       sqlite3DbFree(D,P)
# define sqlite3_राशिः_Free_NN(D,P)     sqlite3DbFreeNN(D,P)
#यदि_अन्तः

/* Do not allow both MEMSYS5 and MEMSYS3 to be defined together.  If they
** are, disable MEMSYS3
*/
#यदि_परिभाषितम् SQLITE_ENABLE_MEMSYS5
ध्रुव sqlite3_mem_methods *sqlite3MemGetMemsys5(शून्य);
#अपरिभाषय SQLITE_ENABLE_MEMSYS3
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_ENABLE_MEMSYS3
ध्रुव sqlite3_mem_methods *sqlite3MemGetMemsys3(शून्य);
#यदि_अन्तः


#यदि_अपरिभाषितम् SQLITE_MUTEX_OMIT
  sqlite3_mutex_methods ध्रुव *sqlite3DefaultMutex(शून्य);
  sqlite3_mutex_methods ध्रुव *sqlite3NoopMutex(शून्य);
  sqlite3_mutex *sqlite3MutexAlloc(पूर्णाङ्क);
  पूर्णाङ्क sqlite3MutexInit(शून्य);
  पूर्णाङ्क sqlite3_Mutex_अन्तः(शून्य);
#यदि_अन्तः
#यदि !defined(SQLITE_MUTEX_OMIT) && !defined(SQLITE_MUTEX_NOOP)
  शून्य sqlite3_स्मृतिः_Barrier(शून्य);
#अन्यथा
# define sqlite3_स्मृतिः_Barrier()
#यदि_अन्तः

sqlite3_int64 sqlite3_स्थितिः_मानम्(पूर्णाङ्क);
शून्य sqlite3_स्थितिः_ऊर्ध्वम्(पूर्णाङ्क, पूर्णाङ्क);
शून्य sqlite3_स्थितिः_अधस्तात्(पूर्णाङ्क, पूर्णाङ्क);
शून्य sqlite3_स्थितिः_Highwater(पूर्णाङ्क, पूर्णाङ्क);
पूर्णाङ्क sqlite3_Lookaside_प्रयुक्तम्(sqlite3*,पूर्णाङ्क*);

/* Access to mutexes used by sqlite3_status() */
sqlite3_mutex *sqlite3Pcache1Mutex(शून्य);
sqlite3_mutex *sqlite3MallocMutex(शून्य);


/* The SQLITE_THREAD_MISUSE_WARNINGS compile-time option used to be called
** SQLITE_ENABLE_MULTITHREADED_CHECKS.  Keep that older macro for backwards
** compatibility, at least for a while... */
#यदि_परिभाषितम् SQLITE_ENABLE_MULTITHREADED_CHECKS
# define SQLITE_THREAD_MISUSE_WARNINGS 1
#यदि_अन्तः

/* SQLITE_THREAD_MISUSE_ABORT implies SQLITE_THREAD_MISUSE_WARNINGS */
#यदि_परिभाषितम् SQLITE_THREAD_MISUSE_ABORT
# define SQLITE_THREAD_MISUSE_WARNINGS 1
#यदि_अन्तः

#यदि defined(SQLITE_THREAD_MISUSE_WARNINGS) && !defined(SQLITE_MUTEX_OMIT)
शून्य sqlite3MutexWarnOnContention(sqlite3_mutex*);
#अन्यथा
# define sqlite3MutexWarnOnContention(x)
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_FLOATING_POINT
# define EXP754 (((u64)0x7ff)<<52)
# define MAN754 ((((u64)1)<<52)-1)
# define IsNaN(X) (((X)&EXP754)==EXP754 && ((X)&MAN754)!=0)
# define IsOvfl(X) (((X)&EXP754)==EXP754)
  पूर्णाङ्क sqlite3IsNaN(द्विगुण);
  पूर्णाङ्क sqlite3_Is_अतिप्रवाहः(द्विगुण);
#अन्यथा
# define IsNaN(X)             0
# define sqlite3IsNaN(X)      0
# define sqlite3_Is_अतिप्रवाहः_2(X) 0
#यदि_अन्तः

/*
** An instance of the following structure holds information about SQL
** functions arguments that are the parameters to the printf() function.
*/
संरचना PrintfArguments {
  पूर्णाङ्क nArg;                /* Total number of arguments */
  पूर्णाङ्क n_प्रयुक्तम्;               /* Number of arguments used so far */
  sqlite3_मानम् **apArg;   /* The argument values */
};

/*
** Maxium number of base-10 digits in an unsigned 64-bit integer
*/
#परिभाषय SQLITE_U64_DIGITS 20

/*
** An instance of this object receives the decoding of a floating point
** value into an approximate decimal representation.
*/
संरचना FpDecode {
  पूर्णाङ्क n;                           /* Significant digits in the decode */
  पूर्णाङ्क iDP;                         /* Location of the decimal point */
  वर्ण *z;                         /* Start of significant digits */
  वर्ण zBuf[SQLITE_U64_DIGITS+1];  /* Storage for significant digits */
  वर्ण sign;                       /* '+' or '-' */
  वर्ण isSpecial;                  /* 1: Infinity  2: NaN */
};

शून्य sqlite3FpDecode(FpDecode*,द्विगुण,पूर्णाङ्क,पूर्णाङ्क);
वर्ण *sqlite3MPrintf(sqlite3*,ध्रुव वर्ण*, ...);
वर्ण *sqlite3VMPrintf(sqlite3*,ध्रुव वर्ण*, va_list);
#यदि defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  शून्य sqlite3DebugPrintf(ध्रुव वर्ण*, ...);
#यदि_अन्तः
#यदि defined(SQLITE_TEST)
  शून्य *sqlite3_Test_Text_गन्तव्यम्_Ptr(ध्रुव वर्ण*);
  ध्रुव वर्ण *sqlite3_Test_Ptr_गन्तव्यम्_Text(शून्य*);
#यदि_अन्तः

#यदि defined(SQLITE_DEBUG)
  शून्य sqlite3_वृक्षः_View_पङ्क्तिः(वृक्षः_View*, ध्रुव वर्ण *zFormat, ...);
  शून्य sqlite3_वृक्षः_View_व्यञ्जकम्(वृक्षः_View*, ध्रुव व्यञ्जकम्*, u8);
  शून्य sqlite3_वृक्षः_View_Bare_व्यञ्जकम्_List(वृक्षः_View*, ध्रुव व्यञ्जकम्_List*, ध्रुव वर्ण*);
  शून्य sqlite3_वृक्षः_View_व्यञ्जकम्_List(वृक्षः_View*, ध्रुव व्यञ्जकम्_List*, u8, ध्रुव वर्ण*);
  शून्य sqlite3_वृक्षः_View_Bare_Id_List(वृक्षः_View*, ध्रुव IdList*, ध्रुव वर्ण*);
  शून्य sqlite3_वृक्षः_View_Id_List(वृक्षः_View*, ध्रुव IdList*, u8, ध्रुव वर्ण*);
  शून्य sqlite3_वृक्षः_View_Column_List(वृक्षः_View*, ध्रुव Column*, पूर्णाङ्क, u8);
  शून्य sqlite3_वृक्षः_View_Src_List(वृक्षः_View*, ध्रुव SrcList*);
  शून्य sqlite3_वृक्षः_View_Select(वृक्षः_View*, ध्रुव Select*, u8);
  शून्य sqlite3_वृक्षः_View_With(वृक्षः_View*, ध्रुव With*, u8);
  शून्य sqlite3_वृक्षः_View_Upsert(वृक्षः_View*, ध्रुव Upsert*, u8);
#यदि TREETRACE_ENABLED
  शून्य sqlite3_वृक्षः_View_Delete(ध्रुव With*, ध्रुव SrcList*, ध्रुव व्यञ्जकम्*,
                             ध्रुव व्यञ्जकम्_List*,ध्रुव व्यञ्जकम्*, ध्रुव Trigger*);
  शून्य sqlite3_वृक्षः_View_निवेशय(ध्रुव With*, ध्रुव SrcList*,
                             ध्रुव IdList*, ध्रुव Select*, ध्रुव व्यञ्जकम्_List*,
                             पूर्णाङ्क, ध्रुव Upsert*, ध्रुव Trigger*);
  शून्य sqlite3_वृक्षः_View_Update(ध्रुव With*, ध्रुव SrcList*, ध्रुव व्यञ्जकम्_List*,
                             ध्रुव व्यञ्जकम्*, पूर्णाङ्क, ध्रुव व्यञ्जकम्_List*, ध्रुव व्यञ्जकम्*,
                             ध्रुव Upsert*, ध्रुव Trigger*);
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_TRIGGER
  शून्य sqlite3_वृक्षः_View_Trigger_चरणः(वृक्षः_View*, ध्रुव Trigger_चरणः*, u8, u8);
  शून्य sqlite3_वृक्षः_View_Trigger(वृक्षः_View*, ध्रुव Trigger*, u8, u8);
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
  शून्य sqlite3_वृक्षः_View_गवाक्षः(वृक्षः_View*, ध्रुव गवाक्षः*, u8);
  शून्य sqlite3_वृक्षः_View_Win_Func(वृक्षः_View*, ध्रुव गवाक्षः*, u8);
#यदि_अन्तः
  शून्य sqlite3_Show_व्यञ्जकम्(ध्रुव व्यञ्जकम्*);
  शून्य sqlite3_Show_व्यञ्जकम्_List(ध्रुव व्यञ्जकम्_List*);
  शून्य sqlite3ShowIdList(ध्रुव IdList*);
  शून्य sqlite3ShowSrcList(ध्रुव SrcList*);
  शून्य sqlite3ShowSelect(ध्रुव Select*);
  शून्य sqlite3ShowWith(ध्रुव With*);
  शून्य sqlite3ShowUpsert(ध्रुव Upsert*);
#यदि_अपरिभाषितम् SQLITE_OMIT_TRIGGER
  शून्य sqlite3_Show_Trigger_चरणः(ध्रुव Trigger_चरणः*);
  शून्य sqlite3_Show_Trigger_चरणः_List(ध्रुव Trigger_चरणः*);
  शून्य sqlite3ShowTrigger(ध्रुव Trigger*);
  शून्य sqlite3ShowTriggerList(ध्रुव Trigger*);
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
  शून्य sqlite3_Show_गवाक्षः(ध्रुव गवाक्षः*);
  शून्य sqlite3ShowWinFunc(ध्रुव गवाक्षः*);
#यदि_अन्तः
  शून्य sqlite3ShowBitvec(Bitvec*);
#यदि_अन्तः

शून्य sqlite3_समुच्चयः_String(वर्ण **, sqlite3*, ध्रुव वर्ण*);
शून्य sqlite3_प्रगतिः_Check(Parse*);
शून्य sqlite3_Error_सन्देशः(Parse*, ध्रुव वर्ण*, ...);
पूर्णाङ्क sqlite3_Error_गन्तव्यम्_Parser(sqlite3*,पूर्णाङ्क);
शून्य sqlite3Dequote(वर्ण*);
शून्य sqlite3_Dequote_व्यञ्जकम्(व्यञ्जकम्*);
शून्य sqlite3DequoteToken(Token*);
शून्य sqlite3_Dequote_सङ्ख्या(Parse*, व्यञ्जकम्*);
शून्य sqlite3TokenInit(Token*,वर्ण*);
पूर्णाङ्क sqlite3_Keyword_संहिता(ध्रुव अचिह्नित वर्ण*, पूर्णाङ्क);
पूर्णाङ्क sqlite3_सततम्_Parser(Parse*, ध्रुव वर्ण*);
शून्य sqlite3FinishCoding(Parse*);
पूर्णाङ्क sqlite3_Get_अस्थायी_Reg(Parse*);
शून्य sqlite3_Release_अस्थायी_Reg(Parse*,पूर्णाङ्क);
पूर्णाङ्क sqlite3_Get_अस्थायी_Range(Parse*,पूर्णाङ्क);
शून्य sqlite3_Release_अस्थायी_Range(Parse*,पूर्णाङ्क,पूर्णाङ्क);
शून्य sqlite3_Clear_अस्थायी_Reg_Cache(Parse*);
शून्य sqlite3_Touch_पञ्जिकाकोशः(Parse*,पूर्णाङ्क);
#यदि defined(SQLITE_ENABLE_STAT4) || defined(SQLITE_DEBUG)
पूर्णाङ्क sqlite3_First_उपलब्धम्_पञ्जिकाकोशः(Parse*,पूर्णाङ्क);
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_DEBUG
पूर्णाङ्क sqlite3_No_Temps_अन्तर्_Range(Parse*,पूर्णाङ्क,पूर्णाङ्क);
#यदि_अन्तः
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Alloc(sqlite3*,पूर्णाङ्क,ध्रुव Token*,पूर्णाङ्क);
व्यञ्जकम् *sqlite3_व्यञ्जकम्(sqlite3*,पूर्णाङ्क,ध्रुव वर्ण*);
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Int32(sqlite3*,पूर्णाङ्क);
शून्य sqlite3_व्यञ्जकम्_Attach_Subtrees(sqlite3*,व्यञ्जकम्*,व्यञ्जकम्*,व्यञ्जकम्*);
व्यञ्जकम् *sqlite3PExpr(Parse*, पूर्णाङ्क, व्यञ्जकम्*, व्यञ्जकम्*);
शून्य sqlite3_PExpr_सङ्कलय_Select(Parse*, व्यञ्जकम्*, Select*);
व्यञ्जकम् *sqlite3_व्यञ्जकम्_And(Parse*,व्यञ्जकम्*, व्यञ्जकम्*);
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Simplified_And_Or(व्यञ्जकम्*);
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Function(Parse*,व्यञ्जकम्_List*, ध्रुव Token*, पूर्णाङ्क);
शून्य sqlite3_व्यञ्जकम्_सङ्कलय_Function_क्रमः_By(Parse*,व्यञ्जकम्*,व्यञ्जकम्_List*);
शून्य sqlite3_व्यञ्जकम्_क्रमः_By_Aggregate_Error(Parse*,व्यञ्जकम्*);
शून्य sqlite3_व्यञ्जकम्_Function_Usable(Parse*,ध्रुव व्यञ्जकम्*,ध्रुव FuncDef*);
शून्य sqlite3_व्यञ्जकम्_नियोजय_Var_सङ्ख्या(Parse*, व्यञ्जकम्*, u32);
शून्य sqlite3_व्यञ्जकम्_Delete(sqlite3*, व्यञ्जकम्*);
शून्य sqlite3_व्यञ्जकम्_Delete_Generic(sqlite3*,शून्य*);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Deferred_Delete(Parse*, व्यञ्जकम्*);
शून्य sqlite3_व्यञ्जकम्_Unmap_And_Delete(Parse*, व्यञ्जकम्*);
व्यञ्जकम्_List *sqlite3_व्यञ्जकम्_List_Append(Parse*,व्यञ्जकम्_List*,व्यञ्जकम्*);
व्यञ्जकम्_List *sqlite3_व्यञ्जकम्_List_Append_सदिशम्(Parse*,व्यञ्जकम्_List*,IdList*,व्यञ्जकम्*);
Select *sqlite3_व्यञ्जकम्_List_गन्तव्यम्_मानानि(Parse*, पूर्णाङ्क, व्यञ्जकम्_List*);
शून्य sqlite3_व्यञ्जकम्_List_समुच्चयः_क्रमय_क्रमः(व्यञ्जकम्_List*,पूर्णाङ्क,पूर्णाङ्क);
शून्य sqlite3_व्यञ्जकम्_List_समुच्चयः_Name(Parse*,व्यञ्जकम्_List*,ध्रुव Token*,पूर्णाङ्क);
शून्य sqlite3_व्यञ्जकम्_List_समुच्चयः_विस्तृतिः(Parse*,व्यञ्जकम्_List*,ध्रुव वर्ण*,ध्रुव वर्ण*);
शून्य sqlite3_व्यञ्जकम्_List_Delete(sqlite3*, व्यञ्जकम्_List*);
शून्य sqlite3_व्यञ्जकम्_List_Delete_Generic(sqlite3*,शून्य*);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Can_Return_Subtype(Parse*,व्यञ्जकम्*);
u32 sqlite3_व्यञ्जकम्_List_Flags(ध्रुव व्यञ्जकम्_List*);
पूर्णाङ्क sqlite3_सूचकांक_Has_Duplicate_मूलम्_Page(सूचकांक*);
पूर्णाङ्क sqlite3Init(sqlite3*, वर्ण**);
पूर्णाङ्क sqlite3InitCallback(शून्य*, पूर्णाङ्क, वर्ण**, वर्ण**);
पूर्णाङ्क sqlite3InitOne(sqlite3*, पूर्णाङ्क, वर्ण**, u32);
शून्य sqlite3Pragma(Parse*,Token*,Token*,Token*,पूर्णाङ्क);
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
Module *sqlite3_Pragma_Vtab_पञ्जिकाकोशः(sqlite3*,ध्रुव वर्ण *zName);
#यदि_अन्तः
शून्य sqlite3_Reset_All_Schemas_सम्बन्धि_Connection(sqlite3*);
शून्य sqlite3ResetOneSchema(sqlite3*,पूर्णाङ्क);
शून्य sqlite3CollapseDatabaseArray(sqlite3*);
शून्य sqlite3_Commit_आन्तरिकम्_Changes(sqlite3*);
शून्य sqlite3_Column_समुच्चयः_व्यञ्जकम्(Parse*,सारणी*,Column*,व्यञ्जकम्*);
व्यञ्जकम् *sqlite3_Column_व्यञ्जकम्(सारणी*,Column*);
व्यञ्जकम् *sqlite3_Column_व्यञ्जकम्_Auth(सारणी*,Column*,Parse*);
शून्य sqlite3_Column_समुच्चयः_Coll(sqlite3*,Column*,ध्रुव वर्ण*zColl);
ध्रुव वर्ण *sqlite3ColumnColl(Column*);
शून्य sqlite3_Delete_Column_नामानि(sqlite3*,सारणी*);
शून्य sqlite3_Generate_Column_नामानि(Parse *pParse, Select *pSelect);
पूर्णाङ्क sqlite3_Columns_From_व्यञ्जकम्_List(Parse*,व्यञ्जकम्_List*,i16*,Column**);
शून्य sqlite3SubqueryColumnTypes(Parse*,सारणी*,Select*,वर्ण);
सारणी *sqlite3_परिणाम_समुच्चयः_सम्बन्धि_Select(Parse*,Select*,वर्ण);
शून्य sqlite3_Open_Schema_सारणी(Parse *, पूर्णाङ्क);
सूचकांक *sqlite3_Primary_कुञ्चिका_सूचकांक(सारणी*);
पूर्णाङ्क sqlite3_सारणी_Column_गन्तव्यम्_सूचकांक(सूचकांक*, पूर्णाङ्क);
#यदि_परिभाषितम् SQLITE_OMIT_GENERATED_COLUMNS
# define sqlite3_सारणी_Column_गन्तव्यम्_Storage(T,X) (X)  /* No-op pass-through */
# define sqlite3_Storage_Column_गन्तव्यम्_सारणी(T,X) (X)  /* No-op pass-through */
#अन्यथा
  i16 sqlite3_सारणी_Column_गन्तव्यम्_Storage(सारणी*, i16);
  i16 sqlite3_Storage_Column_गन्तव्यम्_सारणी(सारणी*, i16);
#यदि_अन्तः
शून्य sqlite3_Start_सारणी(Parse*,Token*,Token*,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क);
#यदि SQLITE_ENABLE_HIDDEN_COLUMNS
  शून्य sqlite3ColumnPropertiesFromName(सारणी*, Column*);
#अन्यथा
# define sqlite3ColumnPropertiesFromName(T,C) /* no-op */
#यदि_अन्तः
शून्य sqlite3_सङ्कलय_Column(Parse*,Token,Token);
शून्य sqlite3_सङ्कलय_Not_Null(Parse*, पूर्णाङ्क);
शून्य sqlite3_सङ्कलय_Primary_कुञ्चिका(Parse*, व्यञ्जकम्_List*, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क);
शून्य sqlite3_सङ्कलय_Check_Constraint(Parse*, व्यञ्जकम्*, ध्रुव वर्ण*, ध्रुव वर्ण*);
शून्य sqlite3_सङ्कलय_Default_मानम्(Parse*,व्यञ्जकम्*,ध्रुव वर्ण*,ध्रुव वर्ण*);
शून्य sqlite3_सङ्कलय_Collate_Type(Parse*, Token*);
शून्य sqlite3_सङ्कलय_Generated(Parse*,व्यञ्जकम्*,Token*);
शून्य sqlite3_अन्तः_सारणी(Parse*,Token*,Token*,u32,Select*);
शून्य sqlite3_सङ्कलय_Returning(Parse*,व्यञ्जकम्_List*);
पूर्णाङ्क sqlite3ParseUri(ध्रुव वर्ण*,ध्रुव वर्ण*,अचिह्नित पूर्णाङ्क*,
                    sqlite3_vfs**,वर्ण**,वर्ण **);
#परिभाषय sqlite3CodecQueryParameters(A,B,C) 0
Btree *sqlite3_Db_Name_गन्तव्यम्_Btree(sqlite3*,ध्रुव वर्ण*);

#यदि_परिभाषितम् SQLITE_UNTESTABLE
# define sqlite3FaultSim(X) SQLITE_OK
#अन्यथा
  पूर्णाङ्क sqlite3FaultSim(पूर्णाङ्क);
#यदि_अन्तः

Bitvec *sqlite3BitvecCreate(u32);
पूर्णाङ्क sqlite3BitvecTest(Bitvec*, u32);
पूर्णाङ्क sqlite3BitvecTestNotNull(Bitvec*, u32);
पूर्णाङ्क sqlite3_Bitvec_समुच्चयः(Bitvec*, u32);
शून्य sqlite3BitvecClear(Bitvec*, u32, शून्य*);
शून्य sqlite3BitvecDestroy(Bitvec*);
u32 sqlite3_Bitvec_आकार(Bitvec*);
#यदि_अपरिभाषितम् SQLITE_UNTESTABLE
पूर्णाङ्क sqlite3BitvecBuiltinTest(पूर्णाङ्क,पूर्णाङ्क*);
#यदि_अन्तः

Row_समुच्चयः *sqlite3_Row_समुच्चयः_Init(sqlite3*);
शून्य sqlite3_Row_समुच्चयः_Delete(शून्य*);
शून्य sqlite3_Row_समुच्चयः_Clear(शून्य*);
शून्य sqlite3_Row_समुच्चयः_निवेशय(Row_समुच्चयः*, i64);
पूर्णाङ्क sqlite3_Row_समुच्चयः_Test(Row_समुच्चयः*, पूर्णाङ्क iBatch, i64);
पूर्णाङ्क sqlite3_Row_समुच्चयः_अग्र(Row_समुच्चयः*, i64*);

शून्य sqlite3CreateView(Parse*,Token*,Token*,Token*,व्यञ्जकम्_List*,Select*,पूर्णाङ्क,पूर्णाङ्क);

#यदि !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
  पूर्णाङ्क sqlite3_View_Get_Column_नामानि(Parse*,सारणी*);
#अन्यथा
# define sqlite3_View_Get_Column_नामानि(A,B) 0
#यदि_अन्तः

#यदि SQLITE_MAX_ATTACHED>30
  पूर्णाङ्क sqlite3_Db_आवरणम्_All_Zero(y_Db_आवरणम्);
#यदि_अन्तः
शून्य sqlite3_Drop_सारणी(Parse*, SrcList*, पूर्णाङ्क, पूर्णाङ्क);
शून्य sqlite3_संहिता_Drop_सारणी(Parse*, सारणी*, पूर्णाङ्क, पूर्णाङ्क);
शून्य sqlite3_Delete_सारणी(sqlite3*, सारणी*);
शून्य sqlite3_Delete_सारणी_Generic(sqlite3*, शून्य*);
शून्य sqlite3_Free_सूचकांक(sqlite3*, सूचकांक*);
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINCREMENT
  शून्य sqlite3AutoincrementBegin(Parse *pParse);
  शून्य sqlite3_Autoincrement_अन्तः(Parse *pParse);
#अन्यथा
# define sqlite3AutoincrementBegin(X)
# define sqlite3_Autoincrement_अन्तः(X)
#यदि_अन्तः
शून्य sqlite3_निवेशय(Parse*, SrcList*, Select*, IdList*, पूर्णाङ्क, Upsert*);
#यदि_अपरिभाषितम् SQLITE_OMIT_GENERATED_COLUMNS
  शून्य sqlite3ComputeGeneratedColumns(Parse*, पूर्णाङ्क, सारणी*);
#यदि_अन्तः
शून्य *sqlite3ArrayAllocate(sqlite3*,शून्य*,पूर्णाङ्क,पूर्णाङ्क*,पूर्णाङ्क*);
IdList *sqlite3IdListAppend(Parse*, IdList*, Token*);
पूर्णाङ्क sqlite3_Id_List_सूचकांक(IdList*,ध्रुव वर्ण*);
SrcList *sqlite3SrcListEnlarge(Parse*, SrcList*, पूर्णाङ्क, पूर्णाङ्क);
SrcList *sqlite3SrcListAppendList(Parse *pParse, SrcList *p1, SrcList *p2);
SrcList *sqlite3SrcListAppend(Parse*, SrcList*, Token*, Token*);
शून्य sqlite3SubqueryDelete(sqlite3*,Subquery*);
Select *sqlite3SubqueryDetach(sqlite3*,SrcItem*);
पूर्णाङ्क sqlite3SrcItemAttachSubquery(Parse*, SrcItem*, Select*, पूर्णाङ्क);
SrcList *sqlite3_Src_List_Append_From_पदांशः(Parse*, SrcList*, Token*, Token*,
                                      Token*, Select*, OnOrUsing*);
शून्य sqlite3_Src_List_अनुक्रमितम्_By(Parse *, SrcList *, Token *);
शून्य sqlite3_Src_List_Func_तर्काः(Parse*, SrcList*, व्यञ्जकम्_List*);
पूर्णाङ्क sqlite3_अनुक्रमितम्_By_Lookup(Parse *, SrcItem *);
शून्य sqlite3_Src_List_सरणम्_Join_Type(Parse*,SrcList*);
शून्य sqlite3_Src_List_नियोजय_Cursors(Parse*, SrcList*);
शून्य sqlite3IdListDelete(sqlite3*, IdList*);
शून्य sqlite3ClearOnOrUsing(sqlite3*, OnOrUsing*);
शून्य sqlite3SrcListDelete(sqlite3*, SrcList*);
सूचकांक *sqlite3_Allocate_सूचकांक_Object(sqlite3*,पूर्णाङ्क,पूर्णाङ्क,वर्ण**);
पूर्णाङ्क sqlite3_सूचकांक_Bloomable(ध्रुव सूचकांक*,पूर्णाङ्क);
शून्य sqlite3_Create_सूचकांक(Parse*,Token*,Token*,SrcList*,व्यञ्जकम्_List*,पूर्णाङ्क,Token*,
                          व्यञ्जकम्*, पूर्णाङ्क, पूर्णाङ्क, u8);
शून्य sqlite3_Drop_सूचकांक(Parse*, SrcList*, पूर्णाङ्क);
पूर्णाङ्क sqlite3Select(Parse*, Select*, SelectDest*);
Select *sqlite3SelectNew(Parse*,व्यञ्जकम्_List*,SrcList*,व्यञ्जकम्*,व्यञ्जकम्_List*,
                         व्यञ्जकम्*,व्यञ्जकम्_List*,u32,व्यञ्जकम्*);
शून्य sqlite3SelectDelete(sqlite3*, Select*);
शून्य sqlite3SelectDeleteGeneric(sqlite3*,शून्य*);
शून्य sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect);
सारणी *sqlite3SrcListLookup(Parse*, SrcList*);
पूर्णाङ्क sqlite3IsReadOnly(Parse*, सारणी*, Trigger*);
शून्य sqlite3_Open_सारणी(Parse*, पूर्णाङ्क iCur, पूर्णाङ्क iDb, सारणी*, पूर्णाङ्क);
#यदि defined(SQLITE_ENABLE_UPDATE_DELETE_सीमा) && !defined(SQLITE_OMIT_SUBQUERY)
व्यञ्जकम् *sqlite3_सीमा_Where(Parse*,SrcList*,व्यञ्जकम्*,व्यञ्जकम्_List*,व्यञ्जकम्*,वर्ण*);
#यदि_अन्तः
शून्य sqlite3_संहिता_Change_गणनम्(Vdbe*,पूर्णाङ्क,ध्रुव वर्ण*);
शून्य sqlite3DeleteFrom(Parse*, SrcList*, व्यञ्जकम्*, व्यञ्जकम्_List*, व्यञ्जकम्*);
शून्य sqlite3Update(Parse*, SrcList*, व्यञ्जकम्_List*,व्यञ्जकम्*,पूर्णाङ्क,व्यञ्जकम्_List*,व्यञ्जकम्*,
                   Upsert*);
WhereInfo *sqlite3WhereBegin(Parse*,SrcList*,व्यञ्जकम्*,व्यञ्जकम्_List*,
                             व्यञ्जकम्_List*,Select*,u16,पूर्णाङ्क);
शून्य sqlite3_Where_अन्तः(WhereInfo*);
LogEst sqlite3_Where_Output_Row_गणनम्(WhereInfo*);
पूर्णाङ्क sqlite3WhereIsDistinct(WhereInfo*);
पूर्णाङ्क sqlite3WhereIsOrdered(WhereInfo*);
पूर्णाङ्क sqlite3_Where_क्रमः_By_सीमा_Opt_Label(WhereInfo*);
शून्य sqlite3_Where_Min_Max_Opt_Early_बहिर्गमनम्(Vdbe*,WhereInfo*);
पूर्णाङ्क sqlite3WhereIsSorted(WhereInfo*);
पूर्णाङ्क sqlite3WhereContinueLabel(WhereInfo*);
पूर्णाङ्क sqlite3WhereBreakLabel(WhereInfo*);
पूर्णाङ्क sqlite3WhereOkOnePass(WhereInfo*, पूर्णाङ्क*);
#परिभाषय ONEPASS_OFF      0        /* Use of ONEPASS not allowed */
#परिभाषय ONEPASS_SINGLE   1        /* ONEPASS valid for a single row update */
#परिभाषय ONEPASS_MULTI    2        /* ONEPASS is valid for multiple rows */
पूर्णाङ्क sqlite3_Where_उपयुङ्क्ते_Deferred_Seek(WhereInfo*);
शून्य sqlite3_व्यञ्जकम्_संहिता_आरोपय_सूचकांक_Column(Parse*, सूचकांक*, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क);
पूर्णाङ्क sqlite3_व्यञ्जकम्_संहिता_Get_Column(Parse*, सारणी*, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क, u8);
शून्य sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(Vdbe*, सारणी*, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क);
शून्य sqlite3_व्यञ्जकम्_संहिता_Move(Parse*, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क);
शून्य sqlite3_व्यञ्जकम्_गन्तव्यम्_पञ्जिकाकोशः(व्यञ्जकम् *p_व्यञ्जकम्, पूर्णाङ्क iReg);
शून्य sqlite3_व्यञ्जकम्_संहिता(Parse*, व्यञ्जकम्*, पूर्णाङ्क);
#यदि_अपरिभाषितम् SQLITE_OMIT_GENERATED_COLUMNS
शून्य sqlite3_व्यञ्जकम्_संहिता_Generated_Column(Parse*, सारणी*, Column*, पूर्णाङ्क);
#यदि_अन्तः
शून्य sqlite3_व्यञ्जकम्_संहिता_Copy(Parse*, व्यञ्जकम्*, पूर्णाङ्क);
शून्य sqlite3_व्यञ्जकम्_संहिता_Factorable(Parse*, व्यञ्जकम्*, पूर्णाङ्क);
पूर्णाङ्क sqlite3_व्यञ्जकम्_संहिता_सततम्_Just_Once(Parse*, व्यञ्जकम्*, पूर्णाङ्क);
शून्य sqlite3_व्यञ्जकम्_Null_पञ्जिकाकोशः_Range(Parse*, पूर्णाङ्क, पूर्णाङ्क);
पूर्णाङ्क sqlite3_व्यञ्जकम्_संहिता_अस्थायी(Parse*, व्यञ्जकम्*, पूर्णाङ्क*);
पूर्णाङ्क sqlite3_व्यञ्जकम्_संहिता_Target(Parse*, व्यञ्जकम्*, पूर्णाङ्क);
पूर्णाङ्क sqlite3_व्यञ्जकम्_संहिता_व्यञ्जकम्_List(Parse*, व्यञ्जकम्_List*, पूर्णाङ्क, पूर्णाङ्क, u8);
#परिभाषय SQLITE_ECEL_DUP      0x01  /* Deep, not shallow copies */
#परिभाषय SQLITE_ECEL_गुणकः   0x02  /* Factor out constant terms */
#परिभाषय SQLITE_ECEL_REF      0x04  /* Use ExprList.u.x.iOrderByCol */
#परिभाषय SQLITE_ECEL_OMITREF  0x08  /* Omit if ExprList.u.x.iOrderByCol */
शून्य sqlite3_व्यञ्जकम्_If_True(Parse*, व्यञ्जकम्*, पूर्णाङ्क, पूर्णाङ्क);
शून्य sqlite3_व्यञ्जकम्_If_False(Parse*, व्यञ्जकम्*, पूर्णाङ्क, पूर्णाङ्क);
शून्य sqlite3_व्यञ्जकम्_If_False_Dup(Parse*, व्यञ्जकम्*, पूर्णाङ्क, पूर्णाङ्क);
सारणी *sqlite3_Find_सारणी(sqlite3*,ध्रुव वर्ण*, ध्रुव वर्ण*);
#परिभाषय LOCATE_VIEW    0x01
#परिभाषय LOCATE_NOERR   0x02
सारणी *sqlite3_Locate_सारणी(Parse*,u32 flags,ध्रुव वर्ण*, ध्रुव वर्ण*);
ध्रुव वर्ण *sqlite3_Preferred_सारणी_Name(ध्रुव वर्ण*);
सारणी *sqlite3_Locate_सारणी_Item(Parse*,u32 flags,SrcItem *);
सूचकांक *sqlite3_Find_सूचकांक(sqlite3*,ध्रुव वर्ण*, ध्रुव वर्ण*);
शून्य sqlite3_Unlink_And_Delete_सारणी(sqlite3*,पूर्णाङ्क,ध्रुव वर्ण*);
शून्य sqlite3_Unlink_And_Delete_सूचकांक(sqlite3*,पूर्णाङ्क,ध्रुव वर्ण*);
शून्य sqlite3Vacuum(Parse*,Token*,व्यञ्जकम्*);
पूर्णाङ्क sqlite3_सततम्_Vacuum(वर्ण**, sqlite3*, पूर्णाङ्क, sqlite3_मानम्*);
वर्ण *sqlite3NameFromToken(sqlite3*, ध्रुव Token*);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Compare(ध्रुव Parse*,ध्रुव व्यञ्जकम्*,ध्रुव व्यञ्जकम्*, पूर्णाङ्क);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Compare_Skip(व्यञ्जकम्*,व्यञ्जकम्*,पूर्णाङ्क);
पूर्णाङ्क sqlite3_व्यञ्जकम्_List_Compare(ध्रुव व्यञ्जकम्_List*,ध्रुव व्यञ्जकम्_List*, पूर्णाङ्क);
पूर्णाङ्क sqlite3_व्यञ्जकम्_उपपादयति_व्यञ्जकम्(ध्रुव Parse*,ध्रुव व्यञ्जकम्*,ध्रुव व्यञ्जकम्*, पूर्णाङ्क);
पूर्णाङ्क sqlite3_व्यञ्जकम्_उपपादयति_Non_Null_Row(व्यञ्जकम्*,पूर्णाङ्क,पूर्णाङ्क);
शून्य sqlite3AggInfoPersistWalkerInit(Walker*,Parse*);
शून्य sqlite3_व्यञ्जकम्_Analyze_Aggregates(NameContext*, व्यञ्जकम्*);
शून्य sqlite3_व्यञ्जकम्_Analyze_Agg_List(NameContext*,व्यञ्जकम्_List*);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Covered_By_सूचकांक(व्यञ्जकम्*, पूर्णाङ्क iCur, सूचकांक *pIdx);
पूर्णाङ्क sqlite3ReferencesSrcList(Parse*, व्यञ्जकम्*, SrcList*);
Vdbe *sqlite3GetVdbe(Parse*);
#यदि_अपरिभाषितम् SQLITE_UNTESTABLE
शून्य sqlite3PrngSaveState(शून्य);
शून्य sqlite3PrngRestoreState(शून्य);
#यदि_अन्तः
शून्य sqlite3RollbackAll(sqlite3*,पूर्णाङ्क);
शून्य sqlite3_संहिता_Verify_Schema(Parse*, पूर्णाङ्क);
शून्य sqlite3_संहिता_Verify_Named_Schema(Parse*, ध्रुव वर्ण *zDb);
शून्य sqlite3BeginTransaction(Parse*, पूर्णाङ्क);
शून्य sqlite3_अन्तः_Transaction(Parse*,पूर्णाङ्क);
शून्य sqlite3Savepoint(Parse*, पूर्णाङ्क, Token*);
शून्य sqlite3CloseSavepoints(sqlite3 *);
शून्य sqlite3LeaveMutexAndCloseZombie(sqlite3*);
u32 sqlite3IsTrueOrFalse(ध्रुव वर्ण*);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Id_गन्तव्यम्_True_False(व्यञ्जकम्*);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Truth_मानम्(ध्रुव व्यञ्जकम्*);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Constant(Parse*,व्यञ्जकम्*);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Constant_Or_Function(व्यञ्जकम्*, u8);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Constant_Or_गणः_By(Parse*, व्यञ्जकम्*, व्यञ्जकम्_List*);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_सारणी_Constant(व्यञ्जकम्*,पूर्णाङ्क,पूर्णाङ्क);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Single_सारणी_Constraint(व्यञ्जकम्*,ध्रुव SrcList*,पूर्णाङ्क,पूर्णाङ्क);
पूर्णाङ्क sqlite3_व्यञ्जकम्_List_Is_Constant(Parse *pParse, व्यञ्जकम्_List *pList, पूर्णाङ्क bNoIs);
पूर्णाङ्क sqlite3_व्यञ्जकम्_अन्तर्भवति_Subquery(व्यञ्जकम्*);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Integer(ध्रुव व्यञ्जकम्*, पूर्णाङ्क*, Parse*, पूर्णाङ्क);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Can_Be_Null(ध्रुव व्यञ्जकम्*);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Needs_No_Affinity_Change(ध्रुव व्यञ्जकम्*, वर्ण);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_Like_Operator(ध्रुव व्यञ्जकम्*);
पूर्णाङ्क sqlite3IsRowid(ध्रुव वर्ण*);
ध्रुव वर्ण *sqlite3RowidAlias(सारणी *pTab);
शून्य sqlite3GenerateRowDelete(
    Parse*,सारणी*,Trigger*,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क,i16,u8,u8,u8,पूर्णाङ्क);
शून्य sqlite3_Generate_Row_सूचकांक_Delete(Parse*, सारणी*, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क*, पूर्णाङ्क);
पूर्णाङ्क sqlite3_Generate_सूचकांक_कुञ्चिका(Parse*, सूचकांक*, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क*,सूचकांक*,पूर्णाङ्क);
शून्य sqlite3ResolvePartIdxLabel(Parse*,पूर्णाङ्क);
पूर्णाङ्क sqlite3_व्यञ्जकम्_References_Updated_Column(व्यञ्जकम्*,पूर्णाङ्क*,पूर्णाङ्क);
शून्य sqlite3GenerateConstraintChecks(Parse*,सारणी*,पूर्णाङ्क*,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क,
                                     u8,u8,पूर्णाङ्क,पूर्णाङ्क*,पूर्णाङ्क*,Upsert*);
#यदि_परिभाषितम् SQLITE_ENABLE_NULL_TRIM
  शून्य sqlite3_समुच्चयः_Make_लेखः_P5(Vdbe*,सारणी*);
#अन्यथा
# define sqlite3_समुच्चयः_Make_लेखः_P5(A,B)
#यदि_अन्तः
शून्य sqlite3CompleteInsertion(Parse*,सारणी*,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क*,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क);
पूर्णाङ्क sqlite3_Open_सारणी_And_Indices(Parse*, सारणी*, पूर्णाङ्क, u8, पूर्णाङ्क, u8*, पूर्णाङ्क*, पूर्णाङ्क*);
शून्य sqlite3BeginWriteOperation(Parse*, पूर्णाङ्क, पूर्णाङ्क);
शून्य sqlite3MultiWrite(Parse*);
शून्य sqlite3MayAbort(Parse*);
शून्य sqlite3HaltConstraint(Parse*, पूर्णाङ्क, पूर्णाङ्क, वर्ण*, i8, u8);
शून्य sqlite3UniqueConstraint(Parse*, पूर्णाङ्क, सूचकांक*);
शून्य sqlite3RowidConstraint(Parse*, पूर्णाङ्क, सारणी*);
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Dup(sqlite3*,ध्रुव व्यञ्जकम्*,पूर्णाङ्क);
व्यञ्जकम्_List *sqlite3_व्यञ्जकम्_List_Dup(sqlite3*,ध्रुव व्यञ्जकम्_List*,पूर्णाङ्क);
SrcList *sqlite3SrcListDup(sqlite3*,ध्रुव SrcList*,पूर्णाङ्क);
IdList *sqlite3IdListDup(sqlite3*,ध्रुव IdList*);
Select *sqlite3SelectDup(sqlite3*,ध्रुव Select*,पूर्णाङ्क);
FuncDef *sqlite3_Function_अन्वेषणम्(पूर्णाङ्क,ध्रुव वर्ण*);
शून्य sqlite3_निवेशय_Builtin_Funcs(FuncDef*,पूर्णाङ्क);
FuncDef *sqlite3FindFunction(sqlite3*,ध्रुव वर्ण*,पूर्णाङ्क,u8,u8);
शून्य sqlite3_Quote_मानम्(StrAccum*,sqlite3_मानम्*,पूर्णाङ्क);
पूर्णाङ्क sqlite3AppendOneUtf8Character(वर्ण*, u32);
शून्य sqlite3_पञ्जिकाकोशः_Builtin_Functions(शून्य);
शून्य sqlite3_पञ्जिकाकोशः_Date_Time_Functions(शून्य);
शून्य sqlite3_पञ्जिकाकोशः_Json_Functions(शून्य);
शून्य sqlite3_पञ्जिकाकोशः_प्रत्येकम्_Connection_Builtin_Functions(sqlite3*);
#यदि !defined(SQLITE_OMIT_VIRTUALTABLE) && !defined(SQLITE_OMIT_JSON)
  Module *sqlite3_Json_Vtab_पञ्जिकाकोशः(sqlite3*,ध्रुव वर्ण*);
#यदि_अन्तः
पूर्णाङ्क sqlite3SafetyCheckOk(sqlite3*);
पूर्णाङ्क sqlite3SafetyCheckSickOrOk(sqlite3*);
शून्य sqlite3ChangeCookie(Parse*, पूर्णाङ्क);
With *sqlite3WithDup(sqlite3 *db, With *p);

#यदि !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(SQLITE_ENABLE_CARRAY)
  Module *sqlite3_Carray_पञ्जिकाकोशः(sqlite3*);
#यदि_अन्तः

#यदि !defined(SQLITE_OMIT_VIEW) && !defined(SQLITE_OMIT_TRIGGER)
शून्य sqlite3MaterializeView(Parse*, सारणी*, व्यञ्जकम्*, व्यञ्जकम्_List*,व्यञ्जकम्*,पूर्णाङ्क);
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_TRIGGER
  शून्य sqlite3BeginTrigger(Parse*, Token*,Token*,पूर्णाङ्क,पूर्णाङ्क,IdList*,SrcList*,
                           व्यञ्जकम्*,पूर्णाङ्क, पूर्णाङ्क);
  शून्य sqlite3FinishTrigger(Parse*, Trigger_चरणः*, Token*);
  शून्य sqlite3DropTrigger(Parse*, SrcList*, पूर्णाङ्क);
  शून्य sqlite3DropTriggerPtr(Parse*, Trigger*);
  Trigger *sqlite3TriggersExist(Parse *, सारणी*, पूर्णाङ्क, व्यञ्जकम्_List*, पूर्णाङ्क *p_आवरणम्);
  Trigger *sqlite3TriggerList(Parse *, सारणी *);
  शून्य sqlite3_संहिता_Row_Trigger(Parse*, Trigger *, पूर्णाङ्क, व्यञ्जकम्_List*, पूर्णाङ्क, सारणी *,
                            पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क);
  शून्य sqlite3_संहिता_Row_Trigger_प्रत्यक्षम्(Parse *, Trigger *, सारणी *, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क);
  शून्य sqliteViewTriggers(Parse*, सारणी*, व्यञ्जकम्*, पूर्णाङ्क, व्यञ्जकम्_List*);
  शून्य sqlite3_Delete_Trigger_चरणः(sqlite3*, Trigger_चरणः*);
  Trigger_चरणः *sqlite3_Trigger_Select_चरणः(sqlite3*,Select*,
                                        ध्रुव वर्ण*,ध्रुव वर्ण*);
  Trigger_चरणः *sqlite3_Trigger_निवेशय_चरणः(Parse*,SrcList*, IdList*,
                                        Select*,u8,Upsert*,
                                        ध्रुव वर्ण*,ध्रुव वर्ण*);
  Trigger_चरणः *sqlite3_Trigger_Update_चरणः(Parse*,SrcList*,SrcList*,व्यञ्जकम्_List*,
                                        व्यञ्जकम्*, u8, ध्रुव वर्ण*,ध्रुव वर्ण*);
  Trigger_चरणः *sqlite3_Trigger_Delete_चरणः(Parse*,SrcList*, व्यञ्जकम्*,
                                        ध्रुव वर्ण*,ध्रुव वर्ण*);
  शून्य sqlite3DeleteTrigger(sqlite3*, Trigger*);
  शून्य sqlite3UnlinkAndDeleteTrigger(sqlite3*,पूर्णाङ्क,ध्रुव वर्ण*);
  u32 sqlite3TriggerColmask(Parse*,Trigger*,व्यञ्जकम्_List*,पूर्णाङ्क,पूर्णाङ्क,सारणी*,पूर्णाङ्क);
# define sqlite3ParseToplevel(p) ((p)->pToplevel ? (p)->pToplevel : (p))
# define sqlite3IsToplevel(p) ((p)->pToplevel==0)
#अन्यथा
# define sqlite3TriggersExist(B,C,D,E,F) 0
# define sqlite3DeleteTrigger(A,B)
# define sqlite3DropTriggerPtr(A,B)
# define sqlite3UnlinkAndDeleteTrigger(A,B,C)
# define sqlite3_संहिता_Row_Trigger(A,B,C,D,E,F,G,H,I)
# define sqlite3_संहिता_Row_Trigger_प्रत्यक्षम्(A,B,C,D,E,F)
# define sqlite3TriggerList(X, Y) 0
# define sqlite3ParseToplevel(p) p
# define sqlite3IsToplevel(p) 1
# define sqlite3TriggerColmask(A,B,C,D,E,F,G) 0
#यदि_अन्तः

पूर्णाङ्क sqlite3JoinType(Parse*, Token*, Token*, Token*);
पूर्णाङ्क sqlite3_Column_सूचकांक(सारणी *pTab, ध्रुव वर्ण *zCol);
शून्य sqlite3_Src_Item_Column_प्रयुक्तम्(SrcItem*,पूर्णाङ्क);
शून्य sqlite3_समुच्चयः_Join_व्यञ्जकम्(व्यञ्जकम्*,पूर्णाङ्क,u32);
शून्य sqlite3_Create_Foreign_कुञ्चिका(Parse*, व्यञ्जकम्_List*, Token*, व्यञ्जकम्_List*, पूर्णाङ्क);
शून्य sqlite3_Defer_Foreign_कुञ्चिका(Parse*, पूर्णाङ्क);
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTHORIZATION
  शून्य sqlite3AuthRead(Parse*,व्यञ्जकम्*,Schema*,SrcList*);
  पूर्णाङ्क sqlite3AuthCheck(Parse*,पूर्णाङ्क, ध्रुव वर्ण*, ध्रुव वर्ण*, ध्रुव वर्ण*);
  शून्य sqlite3_Auth_Context_प्रक्षिप(Parse*, AuthContext*, ध्रुव वर्ण*);
  शून्य sqlite3_Auth_Context_निष्कासय(AuthContext*);
  पूर्णाङ्क sqlite3AuthReadCol(Parse*, ध्रुव वर्ण *, ध्रुव वर्ण *, पूर्णाङ्क);
#अन्यथा
# define sqlite3AuthRead(a,b,c,d)
# define sqlite3AuthCheck(a,b,c,d,e)    SQLITE_OK
# define sqlite3_Auth_Context_प्रक्षिप(a,b,c)
# define sqlite3_Auth_Context_निष्कासय(a)  ((शून्य)(a))
#यदि_अन्तः
पूर्णाङ्क sqlite3DbIsNamed(sqlite3 *db, पूर्णाङ्क iDb, ध्रुव वर्ण *zName);
शून्य sqlite3Attach(Parse*, व्यञ्जकम्*, व्यञ्जकम्*, व्यञ्जकम्*);
शून्य sqlite3Detach(Parse*, व्यञ्जकम्*);
शून्य sqlite3FixInit(DbFixer*, Parse*, पूर्णाङ्क, ध्रुव वर्ण*, ध्रुव Token*);
पूर्णाङ्क sqlite3FixSrcList(DbFixer*, SrcList*);
पूर्णाङ्क sqlite3FixSelect(DbFixer*, Select*);
पूर्णाङ्क sqlite3_Fix_व्यञ्जकम्(DbFixer*, व्यञ्जकम्*);
पूर्णाङ्क sqlite3_Fix_Trigger_चरणः(DbFixer*, Trigger_चरणः*);

पूर्णाङ्क sqlite3RealSameAsInt(द्विगुण,sqlite3_int64);
i64 sqlite3_Real_गन्तव्यम्_I64(द्विगुण);
पूर्णाङ्क sqlite3_Int64_गन्तव्यम्_Text(i64,वर्ण*);
पूर्णाङ्क sqlite3AtoF(ध्रुव वर्ण *z, द्विगुण*);
पूर्णाङ्क sqlite3GetInt32(ध्रुव वर्ण *, पूर्णाङ्क*);
पूर्णाङ्क sqlite3GetUInt32(ध्रुव वर्ण*, u32*);
पूर्णाङ्क sqlite3Atoi(ध्रुव वर्ण*);
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
पूर्णाङ्क sqlite3Utf16ByteLen(ध्रुव शून्य *p_दत्त, पूर्णाङ्क nByte, पूर्णाङ्क nChar);
#यदि_अन्तः
पूर्णाङ्क sqlite3Utf8CharLen(ध्रुव वर्ण *p_दत्त, पूर्णाङ्क nByte);
u32 sqlite3Utf8Read(ध्रुव u8**);
पूर्णाङ्क sqlite3Utf8ReadLimited(ध्रुव u8*, पूर्णाङ्क, u32*);
LogEst sqlite3LogEst(u64);
LogEst sqlite3_Log_Est_सङ्कलय(LogEst,LogEst);
LogEst sqlite3LogEstFromDouble(द्विगुण);
u64 sqlite3_Log_Est_गन्तव्यम्_Int(LogEst);
VList *sqlite3_VList_सङ्कलय(sqlite3*,VList*,ध्रुव वर्ण*,पूर्णाङ्क,पूर्णाङ्क);
ध्रुव वर्ण *sqlite3_VList_अङ्कः_गन्तव्यम्_Name(VList*,पूर्णाङ्क);
पूर्णाङ्क sqlite3_VList_Name_गन्तव्यम्_अङ्कः(VList*,ध्रुव वर्ण*,पूर्णाङ्क);

/*
** Routines to read and write variable-length integers.  These used to
** be defined locally, but now we use the varint routines in the util.c
** file.
*/
पूर्णाङ्क sqlite3PutVarint(अचिह्नित वर्ण*, u64);
u8 sqlite3GetVarint(ध्रुव अचिह्नित वर्ण *, u64 *);
i64 sqlite3_Varint_मानम्(ध्रुव अचिह्नित वर्ण*);
u8 sqlite3GetVarint32(ध्रुव अचिह्नित वर्ण *, u32 *);
पूर्णाङ्क sqlite3VarintLen(u64 v);

/*
** The common case is for a varint to be a single byte.  They following
** macros handle the common case without a procedure call, but then call
** the procedure for larger varints.
*/
#परिभाषय getVarint32(A,B)  \
  (u8)((*(A)<(u8)0x80)?((B)=(u32)*(A)),1:sqlite3GetVarint32((A),(u32 *)&(B)))
#परिभाषय getVarint32NR(A,B) \
  B=(u32)*(A);यदि(B>=0x80)sqlite3GetVarint32((A),(u32*)&(B))
#परिभाषय putVarint32(A,B)  \
  (u8)(((u32)(B)<(u32)0x80)?(*(A)=(अचिह्नित वर्ण)(B)),1:\
  sqlite3PutVarint((A),(B)))

ध्रुव वर्ण *sqlite3_सूचकांक_Affinity_Str(sqlite3*, सूचकांक*);
वर्ण *sqlite3_सारणी_Affinity_Str(sqlite3*,ध्रुव सारणी*);
शून्य sqlite3_सारणी_Affinity(Vdbe*, सारणी*, पूर्णाङ्क);
वर्ण sqlite3CompareAffinity(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्, वर्ण aff2);
पूर्णाङ्क sqlite3_सूचकांक_Affinity_Ok(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्, वर्ण idx_affinity);
वर्ण sqlite3_सारणी_Column_Affinity(ध्रुव सारणी*,पूर्णाङ्क);
वर्ण sqlite3_व्यञ्जकम्_Affinity(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्);
पूर्णाङ्क sqlite3_व्यञ्जकम्_दत्त_Type(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्);
पूर्णाङ्क sqlite3Atoi64(ध्रुव वर्ण*, i64*, पूर्णाङ्क, u8);
पूर्णाङ्क sqlite3_Dec_Or_Hex_गन्तव्यम्_I64(ध्रुव वर्ण*, i64*);
शून्य sqlite3_Error_With_सन्देशः(sqlite3*, पूर्णाङ्क, ध्रुव वर्ण*,...);
शून्य sqlite3Error(sqlite3*,पूर्णाङ्क);
शून्य sqlite3ErrorClear(sqlite3*);
शून्य sqlite3SystemError(sqlite3*,पूर्णाङ्क);
#यदि !defined(SQLITE_OMIT_BLOB_LITERAL)
शून्य *sqlite3_Hex_गन्तव्यम्_Blob(sqlite3*, ध्रुव वर्ण *z, पूर्णाङ्क n);
#यदि_अन्तः
u8 sqlite3_Hex_गन्तव्यम्_Int(पूर्णाङ्क h);
पूर्णाङ्क sqlite3TwoPartName(Parse *, Token *, Token *, Token **);

#यदि defined(SQLITE_NEED_ERR_NAME)
ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_DESERIALIZE
पूर्णाङ्क sqlite3MemdbInit(शून्य);
पूर्णाङ्क sqlite3IsMemdb(ध्रुव sqlite3_vfs*);
#अन्यथा
# define sqlite3IsMemdb(X) 0
#यदि_अन्तः

ध्रुव वर्ण *sqlite3ErrStr(पूर्णाङ्क);
पूर्णाङ्क sqlite3ReadSchema(Parse *pParse);
CollSeq *sqlite3FindCollSeq(sqlite3*,u8 enc, ध्रुव वर्ण*,पूर्णाङ्क);
पूर्णाङ्क sqlite3_द्विमान_Compare(शून्य*,पूर्णाङ्क,ध्रुव शून्य*,पूर्णाङ्क,ध्रुव शून्य*);
#यदि SQLITE_उपयुङ्क्ते_INLINE
स्थिर SQLITE_INLINE पूर्णाङ्क sqlite3_Is_द्विमान(ध्रुव CollSeq *p){
  प्रतिज्ञा( p==0 || p->xCmp!=sqlite3_द्विमान_Compare
       || वाक्यं_तुलय(p->zName,"BINARY")==0 );
  प्रत्यागम p==0 || p->xCmp==sqlite3_द्विमान_Compare;
}
#अन्यथा
पूर्णाङ्क sqlite3_Is_द्विमान(ध्रुव CollSeq*);
#यदि_अन्तः
CollSeq *sqlite3LocateCollSeq(Parse *pParse, ध्रुव वर्ण*zName);
शून्य sqlite3_समुच्चयः_Text_Encoding(sqlite3 *db, u8);
CollSeq *sqlite3_व्यञ्जकम्_Coll_Seq(Parse *pParse, ध्रुव व्यञ्जकम् *p_व्यञ्जकम्);
CollSeq *sqlite3_व्यञ्जकम्_NNColl_Seq(Parse *pParse, ध्रुव व्यञ्जकम् *p_व्यञ्जकम्);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Coll_Seq_मेलनम्(Parse*,ध्रुव व्यञ्जकम्*,ध्रुव व्यञ्जकम्*);
व्यञ्जकम् *sqlite3_व्यञ्जकम्_सङ्कलय_Collate_Token(Parse *pParse, व्यञ्जकम्*, ध्रुव Token*, पूर्णाङ्क);
व्यञ्जकम् *sqlite3_व्यञ्जकम्_सङ्कलय_Collate_String(Parse*,व्यञ्जकम्*,ध्रुव वर्ण*);
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Skip_Collate(व्यञ्जकम्*);
व्यञ्जकम् *sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(व्यञ्जकम्*);
पूर्णाङ्क sqlite3CheckCollSeq(Parse *, CollSeq *);
पूर्णाङ्क sqlite3WritableSchema(sqlite3*);
पूर्णाङ्क sqlite3CheckObjectName(Parse*, ध्रुव वर्ण*,ध्रुव वर्ण*,ध्रुव वर्ण*);
शून्य sqlite3_Vdbe_समुच्चयः_Changes(sqlite3 *, i64);
पूर्णाङ्क sqlite3_सङ्कलय_Int64(i64*,i64);
पूर्णाङ्क sqlite3SubInt64(i64*,i64);
पूर्णाङ्क sqlite3MulInt64(i64*,i64);
पूर्णाङ्क sqlite3AbsInt32(पूर्णाङ्क);
#यदि_परिभाषितम् SQLITE_ENABLE_8_3_नामानि
शून्य sqlite3FileSuffix3(ध्रुव वर्ण*, वर्ण*);
#अन्यथा
# define sqlite3FileSuffix3(X,Y)
#यदि_अन्तः
u8 sqlite3GetBoolean(ध्रुव वर्ण *z,u8);

ध्रुव शून्य *sqlite3_मानम्_Text(sqlite3_मानम्*, u8);
पूर्णाङ्क sqlite3_मानम्_Is_सम्बन्धि_Class(ध्रुव sqlite3_मानम्*, शून्य(*)(शून्य*));
पूर्णाङ्क sqlite3_मानम्_अष्टकानि_2(sqlite3_मानम्*, u8);
शून्य sqlite3_मानम्_समुच्चयः_Str(sqlite3_मानम्*, पूर्णाङ्क, ध्रुव शून्य *,u8,
                        शून्य(*)(शून्य*));
शून्य sqlite3_मानम्_समुच्चयः_Null(sqlite3_मानम्*);
शून्य sqlite3_मानम्_Free(sqlite3_मानम्*);
#यदि_अपरिभाषितम् SQLITE_UNTESTABLE
शून्य sqlite3_परिणाम_Int_Real(sqlite3_context*);
#यदि_अन्तः
sqlite3_मानम् *sqlite3_मानम्_New(sqlite3 *);
#यदि_अपरिभाषितम् SQLITE_OMIT_UTF16
वर्ण *sqlite3Utf16to8(sqlite3 *, ध्रुव शून्य*, पूर्णाङ्क, u8);
#यदि_अन्तः
पूर्णाङ्क sqlite3_मानम्_From_व्यञ्जकम्(sqlite3 *, ध्रुव व्यञ्जकम् *, u8, u8, sqlite3_मानम् **);
शून्य sqlite3_मानम्_Apply_Affinity(sqlite3_मानम् *, u8, u8);
#यदि_अपरिभाषितम् SQLITE_AMALGAMATION
बाह्य ध्रुव अचिह्नित वर्ण sqlite3OpcodeProperty[];
बाह्य ध्रुव वर्ण sqlite3_Str_द्विमान[];
बाह्य ध्रुव अचिह्नित वर्ण sqlite3StdTypeLen[];
बाह्य ध्रुव वर्ण sqlite3StdTypeAffinity[];
बाह्य ध्रुव वर्ण *sqlite3StdType[];
बाह्य ध्रुव अचिह्नित वर्ण sqlite3_ऊर्ध्वस्थम्_गन्तव्यम्_अधस्तनम्[];
बाह्य ध्रुव अचिह्नित वर्ण *sqlite3aLTb;
बाह्य ध्रुव अचिह्नित वर्ण *sqlite3aEQb;
बाह्य ध्रुव अचिह्नित वर्ण *sqlite3aGTb;
बाह्य ध्रुव अचिह्नित वर्ण sqlite3CtypeMap[];
बाह्य SQLITE_WSD संरचना Sqlite3Config sqlite3Config;
बाह्य FuncDefHash sqlite3BuiltinFunctions;
बाह्य ध्रुव sqlite3_str sqlite3OomStr;
#यदि_अपरिभाषितम् SQLITE_OMIT_WSD
बाह्य पूर्णाङ्क sqlite3PendingByte;
#यदि_अन्तः
#यदि_अन्तः /* SQLITE_AMALGAMATION */
#यदि_परिभाषितम् VDBE_PROFILE
बाह्य sqlite3_uint64 sqlite3NProfileCnt;
#यदि_अन्तः
शून्य sqlite3_मूलम्_Page_Moved(sqlite3*, पूर्णाङ्क, Pgno, Pgno);
शून्य sqlite3Reindex(Parse*, Token*, Token*);
शून्य sqlite3AlterFunctions(शून्य);
शून्य sqlite3_Alter_Rename_सारणी(Parse*, SrcList*, Token*);
शून्य sqlite3AlterRenameColumn(Parse*, SrcList*, Token*, Token*);
शून्य sqlite3AlterDropConstraint(Parse*,SrcList*,Token*,Token*);
शून्य sqlite3_Alter_सङ्कलय_Constraint(
  Parse *pParse,           /* Parse context */
  SrcList *pSrc,           /* Table to add constraint to */
  Token *pFirst,           /* First token of new constraint */
  Token *pName,            /* Name of new constraint. NULL if name omitted. */
  ध्रुव वर्ण *z_व्यञ्जकम्,       /* Text of CHECK expression */
  पूर्णाङ्क n_व्यञ्जकम्,               /* Size of pExpr in bytes */
  व्यञ्जकम् *p_व्यञ्जकम्              /* The parsed CHECK expression */
);
शून्य sqlite3_Alter_समुच्चयः_Not_Null(Parse*, SrcList*, Token*, Token*);
i64 sqlite3GetToken(ध्रुव अचिह्नित वर्ण *, पूर्णाङ्क *);
शून्य sqlite3NestedParse(Parse*, ध्रुव वर्ण*, ...);
शून्य sqlite3ExpirePreparedStatements(sqlite3*, पूर्णाङ्क);
शून्य sqlite3_संहिता_Rhs_सम्बन्धि_अन्तर्(Parse*, व्यञ्जकम्*, पूर्णाङ्क, पूर्णाङ्क);
पूर्णाङ्क sqlite3_संहिता_Subselect(Parse*, व्यञ्जकम्*);
शून्य sqlite3SelectPrep(Parse*, Select*, NameContext*);
पूर्णाङ्क sqlite3ExpandSubquery(Parse*, SrcItem*);
शून्य sqlite3_Select_Wrong_अङ्कः_Terms_Error(Parse *pParse, Select *p);
पूर्णाङ्क sqlite3_मेलनम्_EName(
  ध्रुव संरचना व्यञ्जकम्_List_item*,
  ध्रुव वर्ण*,
  ध्रुव वर्ण*,
  ध्रुव वर्ण*,
  पूर्णाङ्क*
);
Bitmask sqlite3_व्यञ्जकम्_Col_प्रयुक्तम्(व्यञ्जकम्*);
u8 sqlite3StrIHash(ध्रुव वर्ण*);
पूर्णाङ्क sqlite3_Resolve_व्यञ्जकम्_नामानि(NameContext*, व्यञ्जकम्*);
पूर्णाङ्क sqlite3_Resolve_व्यञ्जकम्_List_नामानि(NameContext*, व्यञ्जकम्_List*);
शून्य sqlite3_Resolve_Select_नामानि(Parse*, Select*, NameContext*);
पूर्णाङ्क sqlite3ResolveSelfReference(Parse*,सारणी*,पूर्णाङ्क,व्यञ्जकम्*,व्यञ्जकम्_List*);
पूर्णाङ्क sqlite3_Resolve_क्रमः_गणः_By(Parse*, Select*, व्यञ्जकम्_List*, ध्रुव वर्ण*);
शून्य sqlite3ColumnDefault(Vdbe *, सारणी *, पूर्णाङ्क, पूर्णाङ्क);
शून्य sqlite3_Alter_Finish_सङ्कलय_Column(Parse *, Token *);
शून्य sqlite3_Alter_Begin_सङ्कलय_Column(Parse *, SrcList *);
शून्य sqlite3AlterDropColumn(Parse*, SrcList*, ध्रुव Token*);
ध्रुव शून्य *sqlite3RenameTokenMap(Parse*, ध्रुव शून्य*, ध्रुव Token*);
शून्य sqlite3RenameTokenRemap(Parse*, ध्रुव शून्य *p_गन्तव्यम्, ध्रुव शून्य *pFrom);
शून्य sqlite3_Rename_व्यञ्जकम्_Unmap(Parse*, व्यञ्जकम्*);
शून्य sqlite3RenameExprlistUnmap(Parse*, व्यञ्जकम्_List*);
CollSeq *sqlite3GetCollSeq(Parse*, u8, CollSeq *, ध्रुव वर्ण*);
वर्ण sqlite3AffinityType(ध्रुव वर्ण*, Column*);
शून्य sqlite3Analyze(Parse*, Token*, Token*);
पूर्णाङ्क sqlite3InvokeBusyHandler(BusyHandler*);
पूर्णाङ्क sqlite3FindDb(sqlite3*, Token*);
पूर्णाङ्क sqlite3FindDbName(sqlite3 *, ध्रुव वर्ण *);
पूर्णाङ्क sqlite3_Analysis_आरोपय(sqlite3*,पूर्णाङ्क iDB);
शून्य sqlite3_Delete_सूचकांक_Samples(sqlite3*,सूचकांक*);
शून्य sqlite3DefaultRowEst(सूचकांक*);
शून्य sqlite3_पञ्जिकाकोशः_Like_Functions(sqlite3*, पूर्णाङ्क);
पूर्णाङ्क sqlite3IsLikeFunction(sqlite3*,व्यञ्जकम्*,पूर्णाङ्क*,वर्ण*);
शून्य sqlite3SchemaClear(शून्य *);
Schema *sqlite3SchemaGet(sqlite3 *, Btree *);
पूर्णाङ्क sqlite3_Schema_गन्तव्यम्_सूचकांक(sqlite3 *db, Schema *);
कुञ्चिका_Info *sqlite3_कुञ्चिका_Info_Alloc(sqlite3*,पूर्णाङ्क,पूर्णाङ्क);
शून्य sqlite3_कुञ्चिका_Info_Unref(कुञ्चिका_Info*);
कुञ्चिका_Info *sqlite3_कुञ्चिका_Info_Ref(कुञ्चिका_Info*);
कुञ्चिका_Info *sqlite3_कुञ्चिका_Info_सम्बन्धि_सूचकांक(Parse*, सूचकांक*);
कुञ्चिका_Info *sqlite3_कुञ्चिका_Info_From_व्यञ्जकम्_List(Parse*, व्यञ्जकम्_List*, पूर्णाङ्क, पूर्णाङ्क);
ध्रुव वर्ण *sqlite3SelectOpName(पूर्णाङ्क);
पूर्णाङ्क sqlite3HasExplicitNulls(Parse*, व्यञ्जकम्_List*);

#यदि_परिभाषितम् SQLITE_DEBUG
पूर्णाङ्क sqlite3_कुञ्चिका_Info_Is_Writeable(कुञ्चिका_Info*);
#यदि_अन्तः
पूर्णाङ्क sqlite3CreateFunc(sqlite3 *, ध्रुव वर्ण *, पूर्णाङ्क, पूर्णाङ्क, शून्य *,
  शून्य (*)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम् **),
  शून्य (*)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम् **),
  शून्य (*)(sqlite3_context*),
  शून्य (*)(sqlite3_context*),
  शून्य (*)(sqlite3_context*,पूर्णाङ्क,sqlite3_मानम् **),
  FuncDestructor *pDestructor
);
शून्य sqlite3NoopDestructor(शून्य*);
शून्य *sqlite3OomFault(sqlite3*);
शून्य sqlite3OomClear(sqlite3*);
पूर्णाङ्क sqlite3ApiExit(sqlite3 *db, पूर्णाङ्क);
पूर्णाङ्क sqlite3_Open_अस्थायी_Database(Parse *);

वर्ण *sqlite3RCStrRef(वर्ण*);
शून्य sqlite3RCStrUnref(शून्य*);
वर्ण *sqlite3RCStrNew(u64);
वर्ण *sqlite3RCStrResize(वर्ण*,u64);

शून्य sqlite3StrAccumInit(StrAccum*, sqlite3*, वर्ण*, पूर्णाङ्क, पूर्णाङ्क);
पूर्णाङ्क sqlite3StrAccumEnlarge(StrAccum*, i64);
पूर्णाङ्क sqlite3StrAccumEnlargeIfNeeded(StrAccum*, i64);
वर्ण *sqlite3StrAccumFinish(StrAccum*);
शून्य sqlite3_Str_Accum_समुच्चयः_Error(StrAccum*, u8);
शून्य sqlite3SelectDestInit(SelectDest*,पूर्णाङ्क,पूर्णाङ्क);
व्यञ्जकम् *sqlite3_Create_Column_व्यञ्जकम्(sqlite3 *, SrcList *, पूर्णाङ्क, पूर्णाङ्क);
शून्य sqlite3_लेखः_Error_Byte_विस्थापनम्(sqlite3*,ध्रुव वर्ण*);
शून्य sqlite3_लेखः_Error_विस्थापनम्_सम्बन्धि_व्यञ्जकम्(sqlite3*,ध्रुव व्यञ्जकम्*);

शून्य sqlite3BackupRestart(sqlite3_backup *);
शून्य sqlite3BackupUpdate(sqlite3_backup *, Pgno, ध्रुव u8 *);

#यदि_अपरिभाषितम् SQLITE_OMIT_SUBQUERY
पूर्णाङ्क sqlite3_व्यञ्जकम्_Check_अन्तर्(Parse*, व्यञ्जकम्*);
#अन्यथा
# define sqlite3_व्यञ्जकम्_Check_अन्तर्(x,y) SQLITE_OK
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_ENABLE_STAT4
पूर्णाङ्क sqlite3_Stat4_Probe_समुच्चयः_मानम्(
    Parse*,सूचकांक*,Unpacked_लेखः**,व्यञ्जकम्*,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क*);
पूर्णाङ्क sqlite3_Stat4_मानम्_From_व्यञ्जकम्(Parse*, व्यञ्जकम्*, u8, sqlite3_मानम्**);
शून्य sqlite3Stat4ProbeFree(Unpacked_लेखः*);
पूर्णाङ्क sqlite3Stat4Column(sqlite3*, ध्रुव शून्य*, पूर्णाङ्क, पूर्णाङ्क, sqlite3_मानम्**);
वर्ण sqlite3_सूचकांक_Column_Affinity(sqlite3*, सूचकांक*, पूर्णाङ्क);
#यदि_अन्तः

/*
** The interface to the LEMON-generated parser
*/
#यदि_अपरिभाषितम् SQLITE_AMALGAMATION
  शून्य *sqlite3ParserAlloc(शून्य*(*)(u64), Parse*);
  शून्य sqlite3ParserFree(शून्य*, शून्य(*)(शून्य*));
#यदि_अन्तः
शून्य sqlite3Parser(शून्य*, पूर्णाङ्क, Token);
पूर्णाङ्क sqlite3ParserFallback(पूर्णाङ्क);
#यदि_परिभाषितम् YYTRACKMAXSTACKDEPTH
  पूर्णाङ्क sqlite3_Parser_राशिः_Peak(शून्य*);
#यदि_अन्तः

शून्य sqlite3_Auto_आरोपय_Extensions(sqlite3*);
#यदि_अपरिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
  शून्य sqlite3CloseExtensions(sqlite3*);
#अन्यथा
# define sqlite3CloseExtensions(X)
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_SHARED_CACHE
  शून्य sqlite3_सारणी_तालकम्(Parse *, पूर्णाङ्क, Pgno, u8, ध्रुव वर्ण *);
#अन्यथा
  #परिभाषय sqlite3_सारणी_तालकम्(v,w,x,y,z)
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_TEST
  पूर्णाङ्क sqlite3Utf8To8(अचिह्नित वर्ण*);
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_OMIT_VIRTUALTABLE
#  define sqlite3VtabClear(D,T)
#  define sqlite3VtabSync(X,Y) SQLITE_OK
#  define sqlite3VtabRollback(X)
#  define sqlite3VtabCommit(X)
#  define sqlite3_Vtab_अन्तर्_Sync(db) 0
#  define sqlite3_Vtab_तालकम्(X)
#  define sqlite3VtabUnlock(X)
#  define sqlite3VtabModuleUnref(D,X)
#  define sqlite3VtabUnlockList(X)
#  define sqlite3VtabSavepoint(X, Y, Z) SQLITE_OK
#  define sqlite3GetVTable(X,Y)  ((VTable*)0)
#अन्यथा
   शून्य sqlite3VtabClear(sqlite3 *db, सारणी*);
   शून्य sqlite3VtabDisconnect(sqlite3 *db, सारणी *p);
   पूर्णाङ्क sqlite3VtabSync(sqlite3 *db, Vdbe*);
   पूर्णाङ्क sqlite3VtabRollback(sqlite3 *db);
   पूर्णाङ्क sqlite3VtabCommit(sqlite3 *db);
   शून्य sqlite3_Vtab_तालकम्(VTable *);
   शून्य sqlite3VtabUnlock(VTable *);
   शून्य sqlite3VtabModuleUnref(sqlite3*,Module*);
   शून्य sqlite3VtabUnlockList(sqlite3*);
   पूर्णाङ्क sqlite3VtabSavepoint(sqlite3 *, पूर्णाङ्क, पूर्णाङ्क);
   शून्य sqlite3VtabImportErrmsg(Vdbe*, sqlite3_vtab*);
   VTable *sqlite3GetVTable(sqlite3*, सारणी*);
   Module *sqlite3VtabCreateModule(
     sqlite3*,
     ध्रुव वर्ण*,
     ध्रुव sqlite3_module*,
     शून्य*,
     शून्य(*)(शून्य*)
   );
#  define sqlite3_Vtab_अन्तर्_Sync(db) ((db)->nVTrans>0 && (db)->aVTrans==0)
#यदि_अन्तः
पूर्णाङ्क sqlite3ReadOnlyShadowTables(sqlite3 *db);
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
  पूर्णाङ्क sqlite3_Shadow_सारणी_Name(sqlite3 *db, ध्रुव वर्ण *zName);
  पूर्णाङ्क sqlite3_Is_Shadow_सारणी_सम्बन्धि(sqlite3*,सारणी*,ध्रुव वर्ण*);
  शून्य sqlite3_Mark_All_Shadow_Tables_सम्बन्धि(sqlite3*, सारणी*);
  शून्य sqlite3_Vtab_Eponymous_सारणी_Clear_All(sqlite3*);
#अन्यथा
# define sqlite3_Shadow_सारणी_Name(A,B) 0
# define sqlite3_Is_Shadow_सारणी_सम्बन्धि(A,B,C) 0
# define sqlite3_Mark_All_Shadow_Tables_सम्बन्धि(A,B)
# define sqlite3_Vtab_Eponymous_सारणी_Clear_All(X)
#यदि_अन्तः
पूर्णाङ्क sqlite3_Vtab_Eponymous_सारणी_Init(Parse*,Module*);
शून्य sqlite3_Vtab_Eponymous_सारणी_Clear(sqlite3*,Module*);
शून्य sqlite3VtabMakeWritable(Parse*,सारणी*);
शून्य sqlite3VtabBeginParse(Parse*, Token*, Token*, Token*, पूर्णाङ्क);
शून्य sqlite3VtabFinishParse(Parse*, Token*);
शून्य sqlite3VtabArgInit(Parse*);
शून्य sqlite3VtabArgExtend(Parse*, Token*);
पूर्णाङ्क sqlite3VtabCallCreate(sqlite3*, पूर्णाङ्क, ध्रुव वर्ण *, वर्ण **);
पूर्णाङ्क sqlite3VtabCallConnect(Parse*, सारणी*);
पूर्णाङ्क sqlite3VtabCallDestroy(sqlite3*, पूर्णाङ्क, ध्रुव वर्ण *);
पूर्णाङ्क sqlite3VtabBegin(sqlite3 *, VTable *);

FuncDef *sqlite3VtabOverloadFunction(sqlite3 *,FuncDef*, पूर्णाङ्क nArg, व्यञ्जकम्*);
शून्य sqlite3_Vtab_उपयुङ्क्ते_All_Schemas(Parse*);
sqlite3_int64 sqlite3StmtCurrentTime(sqlite3_context*);
पूर्णाङ्क sqlite3_Vdbe_Parameter_सूचकांक(Vdbe*, ध्रुव वर्ण*, पूर्णाङ्क);
पूर्णाङ्क sqlite3TransferBindings(sqlite3_stmt *, sqlite3_stmt *);
शून्य sqlite3ParseObjectInit(Parse*,sqlite3*);
शून्य sqlite3ParseObjectReset(Parse*);
शून्य *sqlite3_Parser_सङ्कलय_Cleanup(Parse*,शून्य(*)(sqlite3*,शून्य*),शून्य*);
#यदि_परिभाषितम् SQLITE_ENABLE_NORMALIZE
वर्ण *sqlite3Normalize(Vdbe*, ध्रुव वर्ण*);
#यदि_अन्तः
पूर्णाङ्क sqlite3Reprepare(Vdbe*);
शून्य sqlite3_व्यञ्जकम्_List_Check_दीर्घता(Parse*, व्यञ्जकम्_List*, ध्रुव वर्ण*);
CollSeq *sqlite3_व्यञ्जकम्_Compare_Coll_Seq(Parse*,ध्रुव व्यञ्जकम्*);
CollSeq *sqlite3_द्विमान_Compare_Coll_Seq(Parse *, ध्रुव व्यञ्जकम्*, ध्रुव व्यञ्जकम्*);
पूर्णाङ्क sqlite3_अस्थायी_अन्तर्_स्मृतिः(ध्रुव sqlite3*);
ध्रुव वर्ण *sqlite3JournalModename(पूर्णाङ्क);
#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
  पूर्णाङ्क sqlite3Checkpoint(sqlite3*, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क*, पूर्णाङ्क*);
  पूर्णाङ्क sqlite3WalDefaultHook(शून्य*,sqlite3*,ध्रुव वर्ण*,पूर्णाङ्क);
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_CTE
  Cte *sqlite3CteNew(Parse*,Token*,व्यञ्जकम्_List*,Select*,u8);
  शून्य sqlite3CteDelete(sqlite3*,Cte*);
  With *sqlite3_With_सङ्कलय(Parse*,With*,Cte*);
  शून्य sqlite3WithDelete(sqlite3*,With*);
  शून्य sqlite3WithDeleteGeneric(sqlite3*,शून्य*);
  With *sqlite3_With_प्रक्षिप(Parse*, With*, u8);
#अन्यथा
# define sqlite3CteNew(P,T,E,S)   ((शून्य*)0)
# define sqlite3CteDelete(D,C)
# define sqlite3_Cte_With_सङ्कलय(P,W,C) ((शून्य*)0)
# define sqlite3WithDelete(x,y)
# define sqlite3_With_प्रक्षिप(x,y,z) ((शून्य*)0)
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_UPSERT
  Upsert *sqlite3UpsertNew(sqlite3*,व्यञ्जकम्_List*,व्यञ्जकम्*,व्यञ्जकम्_List*,व्यञ्जकम्*,Upsert*);
  शून्य sqlite3UpsertDelete(sqlite3*,Upsert*);
  Upsert *sqlite3UpsertDup(sqlite3*,Upsert*);
  पूर्णाङ्क sqlite3UpsertAnalyzeTarget(Parse*,SrcList*,Upsert*,Upsert*);
  शून्य sqlite3UpsertDoUpdate(Parse*,Upsert*,सारणी*,सूचकांक*,पूर्णाङ्क);
  Upsert *sqlite3_Upsert_सम्बन्धि_सूचकांक(Upsert*,सूचकांक*);
  पूर्णाङ्क sqlite3_Upsert_अग्र_Is_IPK(Upsert*);
#अन्यथा
#परिभाषय sqlite3UpsertNew(u,v,w,x,y,z) ((Upsert*)0)
#परिभाषय sqlite3UpsertDelete(x,y)
#परिभाषय sqlite3UpsertDup(x,y)         ((Upsert*)0)
#परिभाषय sqlite3_Upsert_सम्बन्धि_सूचकांक(x,y)     ((Upsert*)0)
#परिभाषय sqlite3_Upsert_अग्र_Is_IPK(x)     0
#यदि_अन्तः


/* Declarations for functions in fkey.c. All of these are replaced by
** no-op macros if OMIT_FOREIGN_KEY is defined. In this case no foreign
** key functionality is available. If OMIT_TRIGGER is defined but
** OMIT_FOREIGN_KEY is not, only some of the functions are no-oped. In
** this case foreign keys are parsed, but no other functionality is
** provided (enforcement of FK constraints requires the triggers sub-system).
*/
#यदि !defined(SQLITE_OMIT_FOREIGN_कुञ्चिका) && !defined(SQLITE_OMIT_TRIGGER)
  शून्य sqlite3FkCheck(Parse*, सारणी*, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क*, पूर्णाङ्क);
  शून्य sqlite3_Fk_Drop_सारणी(Parse*, SrcList *, सारणी*);
  शून्य sqlite3FkActions(Parse*, सारणी*, व्यञ्जकम्_List*, पूर्णाङ्क, पूर्णाङ्क*, पूर्णाङ्क);
  पूर्णाङ्क sqlite3FkRequired(Parse*, सारणी*, पूर्णाङ्क*, पूर्णाङ्क);
  u32 sqlite3FkOldmask(Parse*, सारणी*);
  FKey *sqlite3FkReferences(सारणी *);
  शून्य sqlite3FkClearTriggerCache(sqlite3*,पूर्णाङ्क);
#अन्यथा
  #परिभाषय sqlite3FkActions(a,b,c,d,e,f)
  #परिभाषय sqlite3FkCheck(a,b,c,d,e,f)
  #परिभाषय sqlite3_Fk_Drop_सारणी(a,b,c)
  #परिभाषय sqlite3FkOldmask(a,b)         0
  #परिभाषय sqlite3FkRequired(a,b,c,d)    0
  #परिभाषय sqlite3FkReferences(a)        0
  #परिभाषय sqlite3FkClearTriggerCache(a,b)
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_FOREIGN_कुञ्चिका
  शून्य sqlite3FkDelete(sqlite3 *, सारणी*);
  पूर्णाङ्क sqlite3_Fk_Locate_सूचकांक(Parse*,सारणी*,FKey*,सूचकांक**,पूर्णाङ्क**);
#अन्यथा
  #परिभाषय sqlite3FkDelete(a,b)
  #परिभाषय sqlite3_Fk_Locate_सूचकांक(a,b,c,d,e)
#यदि_अन्तः


/*
** Available fault injectors.  Should be numbered beginning with 0.
*/
#परिभाषय SQLITE_FAULTINJECTOR_MALLOC     0
#परिभाषय SQLITE_FAULTINJECTOR_गणनम्      1

/*
** The interface to the code in fault.c used for identifying "benign"
** malloc failures. This is only present if SQLITE_UNTESTABLE
** is not defined.
*/
#यदि_अपरिभाषितम् SQLITE_UNTESTABLE
  शून्य sqlite3BeginBenignMalloc(शून्य);
  शून्य sqlite3_अन्तः_Benign_Malloc(शून्य);
#अन्यथा
  #परिभाषय sqlite3BeginBenignMalloc()
  #परिभाषय sqlite3_अन्तः_Benign_Malloc()
#यदि_अन्तः

/*
** Allowed return values from sqlite3FindInIndex()
*/
#परिभाषय अन्तर्_सूचकांक_ROWID        1   /* Search the rowid of the table */
#परिभाषय अन्तर्_सूचकांक_EPH          2   /* Search an ephemeral b-tree */
#परिभाषय अन्तर्_सूचकांक_सूचकांक_ASC    3   /* Existing index ASCENDING */
#परिभाषय अन्तर्_सूचकांक_सूचकांक_DESC   4   /* Existing index DESCENDING */
#परिभाषय अन्तर्_सूचकांक_NOOP         5   /* No table available. Use comparisons */
/*
** Allowed flags for the 3rd parameter to sqlite3FindInIndex().
*/
#परिभाषय अन्तर्_सूचकांक_NOOP_OK     0x0001  /* OK to return IN_INDEX_NOOP */
#परिभाषय अन्तर्_सूचकांक_MEMBERSHIP  0x0002  /* IN operator used for membership test */
#परिभाषय अन्तर्_सूचकांक_LOOP        0x0004  /* IN operator used as a loop */
पूर्णाङ्क sqlite3_Find_अन्तर्_सूचकांक(Parse *, व्यञ्जकम् *, u32, पूर्णाङ्क*, पूर्णाङ्क*, पूर्णाङ्क*);

पूर्णाङ्क sqlite3JournalOpen(sqlite3_vfs *, ध्रुव वर्ण *, sqlite3_file *, पूर्णाङ्क, पूर्णाङ्क);
पूर्णाङ्क sqlite3_Journal_आकार(sqlite3_vfs *);
#यदि defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
  पूर्णाङ्क sqlite3JournalCreate(sqlite3_file *);
#यदि_अन्तः

पूर्णाङ्क sqlite3_Journal_Is_अन्तर्_स्मृतिः(sqlite3_file *p);
शून्य sqlite3MemJournalOpen(sqlite3_file *);

शून्य sqlite3_व्यञ्जकम्_समुच्चयः_Height_And_Flags(Parse *pParse, व्यञ्जकम् *p);
#यदि SQLITE_MAX_व्यञ्जकम्_गभीरता>0
  पूर्णाङ्क sqlite3_Select_व्यञ्जकम्_Height(ध्रुव Select *);
  पूर्णाङ्क sqlite3_व्यञ्जकम्_Check_Height(Parse*, पूर्णाङ्क);
#अन्यथा
  #परिभाषय sqlite3_Select_व्यञ्जकम्_Height(x) 0
  #परिभाषय sqlite3_व्यञ्जकम्_Check_Height(x,y)
#यदि_अन्तः
शून्य sqlite3_व्यञ्जकम्_समुच्चयः_Error_विस्थापनम्(व्यञ्जकम्*,पूर्णाङ्क);

u32 sqlite3Get4byte(ध्रुव u8*);
SQLITE_OPT_INLINE u64 sqlite3Get8byte(ध्रुव u8*);
#यदि SQLITE_BYTEORDER!=4321
SQLITE_OPT_INLINE u64 sqlite3BSwap64(u64);
#यदि_अन्तः
शून्य sqlite3Put4byte(u8*, u32);

#यदि_परिभाषितम् SQLITE_ENABLE_UNLOCK_NOTIFY
  शून्य sqlite3ConnectionBlocked(sqlite3 *, sqlite3 *);
  शून्य sqlite3ConnectionUnlocked(sqlite3 *db);
  शून्य sqlite3ConnectionClosed(sqlite3 *db);
#अन्यथा
  #परिभाषय sqlite3ConnectionBlocked(x,y)
  #परिभाषय sqlite3ConnectionUnlocked(x)
  #परिभाषय sqlite3ConnectionClosed(x)
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_DEBUG
  शून्य sqlite3ParserTrace(FILE*, वर्ण *);
#यदि_अन्तः
#यदि defined(YYCOVERAGE)
  पूर्णाङ्क sqlite3ParserCoverage(FILE*);
#यदि_अन्तः

/*
** If the SQLITE_ENABLE IOTRACE exists then the global variable
** sqlite3IoTrace is a pointer to a printf-like routine used to
** print I/O tracing messages.
*/
#यदि_परिभाषितम् SQLITE_ENABLE_IOTRACE
# define IOTRACE(A)  यदि( sqlite3IoTrace ){ sqlite3IoTrace A; }
  शून्य sqlite3VdbeIOTraceSql(Vdbe*);
SQLITE_API SQLITE_EXTERN शून्य (SQLITE_CDECL *sqlite3IoTrace)(ध्रुव वर्ण*,...);
#अन्यथा
# define IOTRACE(A)
# define sqlite3VdbeIOTraceSql(X)
#यदि_अन्तः

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
#यदि_परिभाषितम् SQLITE_MEMDEBUG
  शून्य sqlite3_Memdebug_समुच्चयः_Type(शून्य*,u8);
  पूर्णाङ्क sqlite3MemdebugHasType(ध्रुव शून्य*,u8);
  पूर्णाङ्क sqlite3MemdebugNoType(ध्रुव शून्य*,u8);
#अन्यथा
# define sqlite3_Memdebug_समुच्चयः_Type(X,Y)  /* no-op */
# define sqlite3MemdebugHasType(X,Y)  1
# define sqlite3MemdebugNoType(X,Y)   1
#यदि_अन्तः
#परिभाषय MEMTYPE_HEAP       0x01  /* General heap allocations */
#परिभाषय MEMTYPE_LOOKASIDE  0x02  /* Heap that might have been lookaside */
#परिभाषय MEMTYPE_PCACHE     0x04  /* Page cache allocations */

/*
** Threading interface
*/
#यदि SQLITE_MAX_कार्यकर्ता_तन्तवः>0
पूर्णाङ्क sqlite3ThreadCreate(SQLiteThread**,शून्य*(*)(शून्य*),शून्य*);
पूर्णाङ्क sqlite3ThreadJoin(SQLiteThread*, शून्य**);
#यदि_अन्तः

#यदि defined(SQLITE_ENABLE_DBPAGE_VTAB) || defined(SQLITE_TEST)
पूर्णाङ्क sqlite3_Dbpage_पञ्जिकाकोशः(sqlite3*);
#यदि_अन्तः
#यदि defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)
पूर्णाङ्क sqlite3_Dbstat_पञ्जिकाकोशः(sqlite3*);
#यदि_अन्तः

पूर्णाङ्क sqlite3_व्यञ्जकम्_सदिशम्_आकार(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्);
पूर्णाङ्क sqlite3_व्यञ्जकम्_Is_सदिशम्(ध्रुव व्यञ्जकम् *p_व्यञ्जकम्);
व्यञ्जकम् *sqlite3_सदिशम्_क्षेत्रम्_Subexpr(व्यञ्जकम्*, पूर्णाङ्क);
व्यञ्जकम् *sqlite3_व्यञ्जकम्_For_सदिशम्_क्षेत्रम्(Parse*,व्यञ्जकम्*,पूर्णाङ्क,पूर्णाङ्क);
शून्य sqlite3_सदिशम्_Error_सन्देशः(Parse*, व्यञ्जकम्*);

#यदि_अपरिभाषितम् SQLITE_OMIT_COMPILEOPTION_DIAGS
ध्रुव वर्ण **sqlite3CompileOptions(पूर्णाङ्क *pnOpt);
#यदि_अन्तः

#यदि SQLITE_OS_UNIX && defined(SQLITE_OS_KV_OPTIONAL)
पूर्णाङ्क sqlite3KvvfsInit(शून्य);
#यदि_अन्तः

#यदि defined(VDBE_PROFILE) \
 || defined(SQLITE_PERFORMANCE_TRACE) \
 || defined(SQLITE_ENABLE_STMT_SCANSTATUS)
sqlite3_uint64 sqlite3Hwtime(शून्य);
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
# define IS_STMT_SCANSTATUS(db) (db->flags & SQLITE_Stmt_अवलोकनम्_स्थितिः)
#अन्यथा
# define IS_STMT_SCANSTATUS(db) 0
#यदि_अन्तः

#यदि_अन्तः /* SQLITEINT_H */
