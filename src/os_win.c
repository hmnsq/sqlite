/*
** 2004 May 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains code that is specific to Windows.
*/
#include "sqliteInt.h"
#якщо SQLITE_OS_WIN               /* This file is used for Windows only */

/*
** Include code that is common to all os_*.c files
*/
#include "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#include "os_win.h"

/*
** This constant should already be defined (in the "WinDef.h" SDK file).
*/
#ifndef MAX_PATH
#  define MAX_PATH                      (260)
#endif

/*
** Maximum pathname length (in chars) for Win32.  This should normally be
** MAX_PATH.
*/
#ifndef SQLITE_WIN32_MAX_PATH_CHARS
#  define SQLITE_WIN32_MAX_PATH_CHARS   (MAX_PATH)
#endif

/*
** This constant should already be defined (in the "WinNT.h" SDK file).
*/
#ifndef UNICODE_STRING_MAX_CHARS
#  define UNICODE_STRING_MAX_CHARS      (32767)
#endif

/*
** Maximum pathname length (in chars) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS.
*/
#ifndef SQLITE_WINNT_MAX_PATH_CHARS
#  define SQLITE_WINNT_MAX_PATH_CHARS   (UNICODE_STRING_MAX_CHARS)
#endif

/*
** Maximum pathname length (in bytes) for Win32.  The MAX_PATH macro is in
** characters, so we allocate 4 bytes per character assuming worst-case of
** 4-bytes-per-character for UTF8.
*/
#ifndef SQLITE_WIN32_MAX_PATH_BYTES
#  define SQLITE_WIN32_MAX_PATH_BYTES   (SQLITE_WIN32_MAX_PATH_CHARS*4)
#endif

/*
** Maximum pathname length (in bytes) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS * sizeof(WCHAR).
*/
#ifndef SQLITE_WINNT_MAX_PATH_BYTES
#  define SQLITE_WINNT_MAX_PATH_BYTES   \
                            (sizeof(WCHAR) * SQLITE_WINNT_MAX_PATH_CHARS)
#endif

/*
** Maximum error message length (in chars) for WinRT.
*/
#ifndef SQLITE_WIN32_MAX_ERRMSG_CHARS
#  define SQLITE_WIN32_MAX_ERRMSG_CHARS (1024)
#endif

/*
** Returns non-zero if the character should be treated as a directory
** separator.
*/
#ifndef winIsDirSep
#  define winIsDirSep(a)                (((a) == '/') || ((a) == '\\'))
#endif

/*
** This macro is used when a local variable is set to a value that is
** [sometimes] not used by the code (e.g. via conditional compilation).
*/
#ifndef UNUSED_VARIABLE_VALUE
#  define UNUSED_VARIABLE_VALUE(x)      (void)(x)
#endif

/*
** Returns the character that should be used as the directory separator.
*/
#ifndef winGetDirSep
#  define winGetDirSep()                '\\'
#endif

/*
** Some older Microsoft compilers lack the following definition.
*/
#ifndef INVALID_FILE_ATTRIBUTES
# define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif

#ifndef FILE_FLAG_MASK
# define FILE_FLAG_MASK          (0xFF3C0000)
#endif

#ifndef FILE_ATTRIBUTE_MASK
# define FILE_ATTRIBUTE_MASK     (0x0003FFF7)
#endif

#ifndef SQLITE_OMIT_WAL
/* Forward references to structures used for WAL */
typedef struct winShm winShm;           /* A connection to shared-memory */
typedef struct winShmNode winShmNode;   /* A region of shared-memory */
#endif

/*
** The winFile structure is a subclass of sqlite3_file* specific to the win32
** portability layer.
*/
typedef struct winFile winFile;
struct winFile {
  const sqlite3_io_methods *pMethod; /*** Must be first ***/
  sqlite3_vfs *pVfs;      /* The VFS used to open this file */
  HANDLE h;               /* Handle for accessing the file */
  u8 locktype;            /* Type of lock currently held on this file */
  short sharedLockByte;   /* Randomly chosen byte used as a shared lock */
  u8 ctrlFlags;           /* Flags.  See WINFILE_* below */
  DWORD lastErrno;        /* The Windows errno from the last I/O error */
#ifndef SQLITE_OMIT_WAL
  winShm *pShm;           /* Instance of shared memory on this file */
#endif
  const char *zPath;      /* Full pathname of this file */
  ціле szChunk;            /* Chunk size configured by FCNTL_CHUNK_SIZE */
#якщо SQLITE_MAX_MMAP_SIZE>0
  ціле nFetchOut;                /* Number of outstanding xFetch references */
  HANDLE hMap;                  /* Handle for accessing memory mapping */
  void *pMapRegion;             /* Area memory mapped */
  sqlite3_int64 mmapSize;       /* Size of mapped region */
  sqlite3_int64 mmapSizeMax;    /* Configured FCNTL_MMAP_SIZE value */
#endif
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  DWORD iBusyTimeout;        /* Wait this many millisec on locks */
  ціле bBlockOnConnect;
#endif
};

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
# define winFileBusyTimeout(pDbFd) pDbFd->iBusyTimeout
#інакше
# define winFileBusyTimeout(pDbFd) 0
#endif

/*
** The winVfsAppData structure is used for the pAppData member for all of the
** Win32 VFS variants.
*/
typedef struct winVfsAppData winVfsAppData;
struct winVfsAppData {
  const sqlite3_io_methods *pMethod; /* The file I/O methods to use. */
  void *pAppData;                    /* The extra pAppData, if any. */
  BOOL bNoLock;                      /* Non-zero if locking is disabled. */
};

/*
** Allowed values for winFile.ctrlFlags
*/
#define WINFILE_RDONLY          0x02   /* Connection is read only */
#define WINFILE_PERSIST_WAL     0x04   /* Persistent WAL mode */
#define WINFILE_PSOW            0x10   /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */

/*
 * The size of the buffer used by sqlite3_win32_write_debug().
 */
#ifndef SQLITE_WIN32_DBG_BUF_SIZE
#  define SQLITE_WIN32_DBG_BUF_SIZE   ((ціле)(4096-sizeof(DWORD)))
#endif

/*
 * If compiled with SQLITE_WIN32_MALLOC on Windows, we will use the
 * various Win32 API heap functions instead of our own.
 */
#ifdef SQLITE_WIN32_MALLOC

/*
 * If this is non-zero, an isolated heap will be created by the native Win32
 * allocator subsystem; otherwise, the default process heap will be used.  This
 * setting has no effect when compiling for WinRT.  By default, this is enabled
 * and an isolated heap will be created to store all allocated data.
 *
 ******************************************************************************
 * WARNING: It is important to note that when this setting is non-zero and the
 *          winMemShutdown function is called (e.g. by the sqlite3_shutdown
 *          function), all data that was allocated using the isolated heap will
 *          be freed immediately and any attempt to access any of that freed
 *          data will almost certainly result in an immediate access violation.
 ******************************************************************************
 */
#ifndef SQLITE_WIN32_HEAP_CREATE
#  define SQLITE_WIN32_HEAP_CREATE        (TRUE)
#endif

/*
 * This is the maximum possible initial size of the Win32-specific heap, in
 * bytes.
 */
#ifndef SQLITE_WIN32_HEAP_MAX_INIT_SIZE
#  define SQLITE_WIN32_HEAP_MAX_INIT_SIZE (4294967295U)
#endif

/*
 * This is the extra space for the initial size of the Win32-specific heap,
 * in bytes.  This value may be zero.
 */
#ifndef SQLITE_WIN32_HEAP_INIT_EXTRA
#  define SQLITE_WIN32_HEAP_INIT_EXTRA  (4194304)
#endif

/*
 * Calculate the maximum legal cache size, in pages, based on the maximum
 * possible initial heap size and the default page size, setting aside the
 * needed extra space.
 */
#ifndef SQLITE_WIN32_MAX_CACHE_SIZE
#  define SQLITE_WIN32_MAX_CACHE_SIZE   (((SQLITE_WIN32_HEAP_MAX_INIT_SIZE) - \
                                          (SQLITE_WIN32_HEAP_INIT_EXTRA)) / \
                                         (SQLITE_DEFAULT_PAGE_SIZE))
#endif

/*
 * This is cache size used in the calculation of the initial size of the
 * Win32-specific heap.  It cannot be negative.
 */
#ifndef SQLITE_WIN32_CACHE_SIZE
#  якщо SQLITE_DEFAULT_CACHE_SIZE>=0
#    define SQLITE_WIN32_CACHE_SIZE     (SQLITE_DEFAULT_CACHE_SIZE)
#  інакше
#    define SQLITE_WIN32_CACHE_SIZE     (-(SQLITE_DEFAULT_CACHE_SIZE))
#  endif
#endif

/*
 * Make sure that the calculated cache size, in pages, cannot cause the
 * initial size of the Win32-specific heap to exceed the maximum amount
 * of memory that can be specified in the call to HeapCreate.
 */
#якщо SQLITE_WIN32_CACHE_SIZE>SQLITE_WIN32_MAX_CACHE_SIZE
#  undef SQLITE_WIN32_CACHE_SIZE
#  define SQLITE_WIN32_CACHE_SIZE       (2000)
#endif

/*
 * The initial size of the Win32-specific heap.  This value may be zero.
 */
#ifndef SQLITE_WIN32_HEAP_INIT_SIZE
#  define SQLITE_WIN32_HEAP_INIT_SIZE   ((SQLITE_WIN32_CACHE_SIZE) * \
                                         (SQLITE_DEFAULT_PAGE_SIZE) + \
                                         (SQLITE_WIN32_HEAP_INIT_EXTRA))
#endif

/*
 * The maximum size of the Win32-specific heap.  This value may be zero.
 */
#ifndef SQLITE_WIN32_HEAP_MAX_SIZE
#  define SQLITE_WIN32_HEAP_MAX_SIZE    (0)
#endif

/*
 * The extra flags to use in calls to the Win32 heap APIs.  This value may be
 * zero for the default behavior.
 */
#ifndef SQLITE_WIN32_HEAP_FLAGS
#  define SQLITE_WIN32_HEAP_FLAGS       (0)
#endif


/*
** The winMemData structure stores information required by the Win32-specific
** sqlite3_mem_methods implementation.
*/
typedef struct winMemData winMemData;
struct winMemData {
#ifndef NDEBUG
  u32 magic1;   /* Magic number to detect structure corruption. */
#endif
  HANDLE hHeap; /* The handle to our heap. */
  BOOL bOwned;  /* Do we own the heap (i.e. destroy it on shutdown)? */
#ifndef NDEBUG
  u32 magic2;   /* Magic number to detect structure corruption. */
#endif
};

#ifndef NDEBUG
#define WINMEM_MAGIC1     0x42b2830b
#define WINMEM_MAGIC2     0xbd4d7cf4
#endif

static struct winMemData win_mem_data = {
#ifndef NDEBUG
  WINMEM_MAGIC1,
#endif
  NULL, FALSE
#ifndef NDEBUG
  ,WINMEM_MAGIC2
#endif
};

#ifndef NDEBUG
#define winMemAssertMagic1() assert( win_mem_data.magic1==WINMEM_MAGIC1 )
#define winMemAssertMagic2() assert( win_mem_data.magic2==WINMEM_MAGIC2 )
#define winMemAssertMagic()  winMemAssertMagic1(); winMemAssertMagic2();
#інакше
#define winMemAssertMagic()
#endif

#define winMemGetDataPtr()  &win_mem_data
#define winMemGetHeap()     win_mem_data.hHeap
#define winMemGetOwned()    win_mem_data.bOwned

static void *winMemMalloc(ціле nBytes);
static void winMemFree(void *pPrior);
static void *winMemRealloc(void *pPrior, ціле nBytes);
static ціле winMemSize(void *p);
static ціле winMemRoundup(ціле n);
static ціле winMemInit(void *pAppData);
static void winMemShutdown(void *pAppData);

const sqlite3_mem_methods *sqlite3MemGetWin32(void);
#endif /* SQLITE_WIN32_MALLOC */

#ifndef SYSCALL
#  define SYSCALL sqlite3_syscall_ptr
#endif

/*
** Many system calls are accessed through pointer-to-functions so that
** they may be overridden at runtime to facilitate fault injection during
** testing and sandboxing.  The following array holds the names and pointers
** to all overrideable system calls.
*/
static struct win_syscall {
  const char *zName;            /* Name of the system call */
  sqlite3_syscall_ptr pCurrent; /* Current value of the system call */
  sqlite3_syscall_ptr pDefault; /* Default value */
} aSyscall[] = {
  { "AreFileApisANSI",         (SYSCALL)AreFileApisANSI,         0 },
#define osAreFileApisANSI ((BOOL(WINAPI*)(VOID))aSyscall[0].pCurrent)

  { "CloseHandle",             (SYSCALL)CloseHandle,             0 },
#define osCloseHandle ((BOOL(WINAPI*)(HANDLE))aSyscall[1].pCurrent)


  { "CreateFileW",             (SYSCALL)CreateFileW,             0 },
#define osCreateFileW ((HANDLE(WINAPI*)(LPCWSTR,DWORD,DWORD, \
        LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE))aSyscall[2].pCurrent)

#якщо defined(SQLITE_UWP)
  { "CreateFileMappingFromApp",(SYSCALL)CreateFileMappingFromApp,0 },
#інакше
  { "CreateFileMappingFromApp",(SYSCALL)0,                       0 },
#endif
#define osCreateFileMappingFromApp ((HANDLE(WINAPI*)(HANDLE,\
        PSECURITY_ATTRIBUTES, \
        ULONG,ULONG64,PCWSTR))aSyscall[3].pCurrent)

#якщо !defined(SQLITE_UWP)
  { "CreateFileMappingW",      (SYSCALL)CreateFileMappingW,      0 },
#інакше
  { "CreateFileMappingW",      (SYSCALL)0,                       0 },
#endif
#define osCreateFileMappingW ((HANDLE(WINAPI*)(HANDLE,LPSECURITY_ATTRIBUTES, \
        DWORD,DWORD,DWORD,LPCWSTR))aSyscall[4].pCurrent)

  { "DeleteFileW",             (SYSCALL)DeleteFileW,             0 },
#define osDeleteFileW ((BOOL(WINAPI*)(LPCWSTR))aSyscall[5].pCurrent)

  { "FlushFileBuffers",        (SYSCALL)FlushFileBuffers,        0 },
#define osFlushFileBuffers ((BOOL(WINAPI*)(HANDLE))aSyscall[6].pCurrent)

  { "FormatMessageW",          (SYSCALL)FormatMessageW,          0 },
#define osFormatMessageW ((DWORD(WINAPI*)(DWORD,LPCVOID,DWORD,DWORD,LPWSTR, \
        DWORD,va_list*))aSyscall[7].pCurrent)

#якщо !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "FreeLibrary",             (SYSCALL)FreeLibrary,             0 },
#інакше
  { "FreeLibrary",             (SYSCALL)0,                       0 },
#endif

#define osFreeLibrary ((BOOL(WINAPI*)(HMODULE))aSyscall[8].pCurrent)

  { "GetCurrentProcessId",     (SYSCALL)GetCurrentProcessId,     0 },
#define osGetCurrentProcessId ((DWORD(WINAPI*)(VOID))aSyscall[9].pCurrent)

  { "GetFileAttributesW",      (SYSCALL)GetFileAttributesW,      0 },
#define osGetFileAttributesW ((DWORD(WINAPI*)(LPCWSTR))aSyscall[10].pCurrent)

  { "GetFileAttributesExW",    (SYSCALL)GetFileAttributesExW,    0 },
#define osGetFileAttributesExW ((BOOL(WINAPI*)(LPCWSTR,GET_FILEEX_INFO_LEVELS, \
        LPVOID))aSyscall[11].pCurrent)

  { "GetFileSizeEx",           (SYSCALL)GetFileSizeEx,           0 },
#define osGetFileSizeEx ((BOOL(WINAPI*)(HANDLE, \
                        PLARGE_INTEGER))aSyscall[12].pCurrent)

  { "GetFullPathNameW",        (SYSCALL)GetFullPathNameW,        0 },
#define osGetFullPathNameW ((DWORD(WINAPI*)(LPCWSTR,DWORD,LPWSTR, \
        LPWSTR*))aSyscall[13].pCurrent)

  { "GetLastError",            (SYSCALL)GetLastError,            0 },
#define osGetLastError ((DWORD(WINAPI*)(VOID))aSyscall[14].pCurrent)

#якщо !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "GetProcAddressA",         (SYSCALL)GetProcAddress,          0 },
#інакше
  { "GetProcAddressA",         (SYSCALL)0,                       0 },
#endif
#define osGetProcAddressA ((FARPROC(WINAPI*)(HMODULE, \
        LPCSTR))aSyscall[15].pCurrent)

  { "GetSystemInfo",           (SYSCALL)GetSystemInfo,           0 },
#define osGetSystemInfo ((VOID(WINAPI*)(LPSYSTEM_INFO))aSyscall[16].pCurrent)

  { "GetSystemTimeAsFileTime", (SYSCALL)GetSystemTimeAsFileTime, 0 },
#define osGetSystemTimeAsFileTime ((VOID(WINAPI*)( \
        LPFILETIME))aSyscall[17].pCurrent)

#ifdef SQLITE_UWP
  { "GetTempPathW",            (SYSCALL)0,                       0 },
#інакше
  { "GetTempPathW",            (SYSCALL)GetTempPathW,            0 },
#endif
#define osGetTempPathW ((DWORD(WINAPI*)(DWORD,LPWSTR))aSyscall[18].pCurrent)

  { "GetTickCount64",          (SYSCALL)GetTickCount64,          0 },
#define osGetTickCount64 ((ULONGLONG(WINAPI*)(VOID))aSyscall[19].pCurrent)

#ifdef SQLITE_UWP
  { "HeapAlloc",               (SYSCALL)0,                       0 },
#інакше
  { "HeapAlloc",               (SYSCALL)HeapAlloc,               0 },
#endif
#define osHeapAlloc ((LPVOID(WINAPI*)(HANDLE,DWORD, \
        SIZE_T))aSyscall[20].pCurrent)

#ifdef SQLITE_UWP
  { "HeapCreate",              (SYSCALL)0,                       0 },
#інакше
  { "HeapCreate",              (SYSCALL)HeapCreate,              0 },
#endif
#define osHeapCreate ((HANDLE(WINAPI*)(DWORD,SIZE_T, \
        SIZE_T))aSyscall[21].pCurrent)

#ifdef SQLITE_UWP
  { "HeapDestroy",             (SYSCALL)0,                       0 },
#інакше
  { "HeapDestroy",             (SYSCALL)HeapDestroy,             0 },
#endif
#define osHeapDestroy ((BOOL(WINAPI*)(HANDLE))aSyscall[22].pCurrent)

#ifdef SQLITE_UWP
  { "HeapFree",                (SYSCALL)0,                       0 },
#інакше
  { "HeapFree",                (SYSCALL)HeapFree,                0 },
#endif
#define osHeapFree ((BOOL(WINAPI*)(HANDLE,DWORD,LPVOID))aSyscall[23].pCurrent)

#ifdef SQLITE_UWP
  { "HeapReAlloc",             (SYSCALL)0,                       0 },
#інакше
  { "HeapReAlloc",             (SYSCALL)HeapReAlloc,             0 },
#endif
#define osHeapReAlloc ((LPVOID(WINAPI*)(HANDLE,DWORD,LPVOID, \
        SIZE_T))aSyscall[24].pCurrent)

#ifdef SQLITE_UWP
  { "HeapSize",                (SYSCALL)0,                       0 },
#інакше
  { "HeapSize",                (SYSCALL)HeapSize,                0 },
#endif
#define osHeapSize ((SIZE_T(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[25].pCurrent)

#ifdef SQLITE_UWP
  { "HeapValidate",            (SYSCALL)0,                       0 },
#інакше
  { "HeapValidate",            (SYSCALL)HeapValidate,            0 },
#endif
#define osHeapValidate ((BOOL(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[26].pCurrent)

#ifdef SQLITE_UWP
  { "HeapCompact",             (SYSCALL)0,                       0 },
#інакше
  { "HeapCompact",             (SYSCALL)HeapCompact,             0 },
#endif
#define osHeapCompact ((UINT(WINAPI*)(HANDLE,DWORD))aSyscall[27].pCurrent)


#якщо !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "LoadLibraryW",            (SYSCALL)LoadLibraryW,            0 },
#інакше
  { "LoadLibraryW",            (SYSCALL)0,                       0 },
#endif
#define osLoadLibraryW ((HMODULE(WINAPI*)(LPCWSTR))aSyscall[28].pCurrent)

  { "LocalFree",               (SYSCALL)LocalFree,               0 },
#define osLocalFree ((HLOCAL(WINAPI*)(HLOCAL))aSyscall[29].pCurrent)

  { "LockFileEx",              (SYSCALL)LockFileEx,              0 },
#define osLockFileEx ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[30].pCurrent)

#якщо (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0) \
 && !defined(SQLITE_UWP)
  { "MapViewOfFile",           (SYSCALL)MapViewOfFile,           0 },
#інакше
  { "MapViewOfFile",           (SYSCALL)0,                       0 },
#endif
#define osMapViewOfFile ((LPVOID(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        SIZE_T))aSyscall[31].pCurrent)

#якщо (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0) \
 && defined(SQLITE_UWP)
  { "MapViewOfFileFromApp",    (SYSCALL)MapViewOfFileFromApp,   0 },
#інакше
  { "MapViewOfFileFromApp",    (SYSCALL)0,                      0 },
#endif
#define osMapViewOfFileFromApp ((LPVOID(WINAPI*)(HANDLE,ULONG,ULONG64, \
        SIZE_T))aSyscall[32].pCurrent)

  { "MultiByteToWideChar",     (SYSCALL)MultiByteToWideChar,     0 },
#define osMultiByteToWideChar ((ціле(WINAPI*)(UINT,DWORD,LPCSTR,ціле,LPWSTR, \
        ціле))aSyscall[33].pCurrent)

  { "QueryPerformanceCounter", (SYSCALL)QueryPerformanceCounter, 0 },
#define osQueryPerformanceCounter ((BOOL(WINAPI*)( \
        LARGE_INTEGER*))aSyscall[34].pCurrent)

  { "ReadFile",                (SYSCALL)ReadFile,                0 },
#define osReadFile ((BOOL(WINAPI*)(HANDLE,LPVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[35].pCurrent)

  { "SetEndOfFile",            (SYSCALL)SetEndOfFile,            0 },
#define osSetEndOfFile ((BOOL(WINAPI*)(HANDLE))aSyscall[36].pCurrent)

  { "SetFilePointerEx",        (SYSCALL)SetFilePointerEx,        0 },
#define osSetFilePointerEx ((BOOL(WINAPI*)(HANDLE,LARGE_INTEGER,\
        PLARGE_INTEGER,DWORD))aSyscall[37].pCurrent)

  { "Sleep",                   (SYSCALL)Sleep,                   0 },
#define osSleep ((VOID(WINAPI*)(DWORD))aSyscall[38].pCurrent)

  { "UnlockFileEx",            (SYSCALL)UnlockFileEx,            0 },
#define osUnlockFileEx ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[39].pCurrent)

#якщо !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0
  { "UnmapViewOfFile",         (SYSCALL)UnmapViewOfFile,         0 },
#інакше
  { "UnmapViewOfFile",         (SYSCALL)0,                       0 },
#endif
#define osUnmapViewOfFile ((BOOL(WINAPI*)(LPCVOID))aSyscall[40].pCurrent)

  { "WideCharToMultiByte",     (SYSCALL)WideCharToMultiByte,     0 },
#define osWideCharToMultiByte ((ціле(WINAPI*)(UINT,DWORD,LPCWSTR,ціле,LPSTR,ціле, \
        LPCSTR,LPBOOL))aSyscall[41].pCurrent)

  { "WriteFile",               (SYSCALL)WriteFile,               0 },
#define osWriteFile ((BOOL(WINAPI*)(HANDLE,LPCVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[42].pCurrent)

  { "WaitForSingleObject",     (SYSCALL)WaitForSingleObject,     0 },
#define osWaitForSingleObject ((DWORD(WINAPI*)(HANDLE, \
        DWORD))aSyscall[43].pCurrent)

  { "WaitForSingleObjectEx",   (SYSCALL)WaitForSingleObjectEx,   0 },
#define osWaitForSingleObjectEx ((DWORD(WINAPI*)(HANDLE,DWORD, \
        BOOL))aSyscall[44].pCurrent)

  { "OutputDebugStringA",      (SYSCALL)OutputDebugStringA,      0 },
#define osOutputDebugStringA ((VOID(WINAPI*)(LPCSTR))aSyscall[45].pCurrent)

  { "GetProcessHeap",          (SYSCALL)GetProcessHeap,          0 },
#define osGetProcessHeap ((HANDLE(WINAPI*)(VOID))aSyscall[46].pCurrent)

/*
** NOTE: On some sub-platforms, the InterlockedCompareExchange "function"
**       is really just a macro that uses a compiler intrinsic (e.g. x64).
**       So do not try to make this is into a redefinable interface.
*/
#якщо defined(InterlockedCompareExchange)
  { "InterlockedCompareExchange", (SYSCALL)0,                    0 },
#define osInterlockedCompareExchange InterlockedCompareExchange
#інакше
  { "InterlockedCompareExchange", (SYSCALL)InterlockedCompareExchange, 0 },
#define osInterlockedCompareExchange ((LONG(WINAPI*)(LONG volatile*,\
        LONG,LONG))aSyscall[47].pCurrent)
#endif /* defined(InterlockedCompareExchange) */

#якщо SQLITE_WIN32_USE_UUID
  { "UuidCreate",               (SYSCALL)UuidCreate,             0 },
#інакше
  { "UuidCreate",               (SYSCALL)0,                      0 },
#endif
#define osUuidCreate ((RPC_STATUS(RPC_ENTRY*)(UUID*))aSyscall[48].pCurrent)

#якщо SQLITE_WIN32_USE_UUID
  { "UuidCreateSequential",     (SYSCALL)UuidCreateSequential,   0 },
#інакше
  { "UuidCreateSequential",     (SYSCALL)0,                      0 },
#endif
#define osUuidCreateSequential \
        ((RPC_STATUS(RPC_ENTRY*)(UUID*))aSyscall[49].pCurrent)

#якщо !defined(SQLITE_NO_SYNC) && SQLITE_MAX_MMAP_SIZE>0
  { "FlushViewOfFile",          (SYSCALL)FlushViewOfFile,        0 },
#інакше
  { "FlushViewOfFile",          (SYSCALL)0,                      0 },
#endif
#define osFlushViewOfFile \
        ((BOOL(WINAPI*)(LPCVOID,SIZE_T))aSyscall[50].pCurrent)

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  { "CreateEvent",              (SYSCALL)CreateEvent,            0 },
#інакше
  { "CreateEvent",              (SYSCALL)0,                      0 },
#endif
#define osCreateEvent ((HANDLE(WINAPI*)(LPSECURITY_ATTRIBUTES,BOOL, \
                        BOOL,LPCSTR))aSyscall[51].pCurrent)

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  { "CancelIo",                 (SYSCALL)CancelIo,               0 },
#інакше
  { "CancelIo",                 (SYSCALL)0,                      0 },
#endif
#define osCancelIo ((BOOL(WINAPI*)(HANDLE))aSyscall[52].pCurrent)

#ifndef _WIN32
  { "getenv",                   (SYSCALL)getenv,                 0 },
#інакше
  { "getenv",                   (SYSCALL)0,                      0 },
#endif
#define osGetenv ((const char *(*)(const char *))aSyscall[53].pCurrent)

#ifndef _WIN32
  { "getcwd",                   (SYSCALL)getcwd,                 0 },
#інакше
  { "getcwd",                   (SYSCALL)0,                      0 },
#endif
#define osGetcwd ((char*(*)(char*,size_t))aSyscall[54].pCurrent)

#ifndef _WIN32
  { "readlink",                 (SYSCALL)readlink,               0 },
#інакше
  { "readlink",                 (SYSCALL)0,                      0 },
#endif
#define osReadlink ((ssize_t(*)(const char*,char*,size_t))aSyscall[55].pCurrent)

#ifndef _WIN32
  { "lstat",                    (SYSCALL)lstat,                  0 },
#інакше
  { "lstat",                    (SYSCALL)0,                      0 },
#endif
#define osLstat ((ціле(*)(const char*,struct stat*))aSyscall[56].pCurrent)

#ifndef _WIN32
  { "__errno",                  (SYSCALL)__errno,                0 },
#інакше
  { "__errno",                  (SYSCALL)0,                      0 },
#endif
#define osErrno (*((ціле*(*)(void))aSyscall[57].pCurrent)())

#ifndef _WIN32
  { "cygwin_conv_path",         (SYSCALL)cygwin_conv_path,       0 },
#інакше
  { "cygwin_conv_path",         (SYSCALL)0,                      0 },
#endif
#define osCygwin_conv_path ((size_t(*)(unsigned ціле, \
    const void *, void *, size_t))aSyscall[58].pCurrent)

  { "GetDriveTypeW",            (SYSCALL)GetDriveTypeW,          0 },
#define osGetDriveTypeW ((UINT(WINAPI*)(LPCWSTR))aSyscall[59].pCurrent)

}; /* End of the overrideable system calls */

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "win32" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
static ціле winSetSystemCall(
  sqlite3_vfs *pNotUsed,        /* The VFS pointer.  Not used */
  const char *zName,            /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
){
  unsigned ціле i;
  ціле rc = SQLITE_NOTFOUND;

  UNUSED_PARAMETER(pNotUsed);
  якщо( zName==0 ){
    /* If no zName is given, restore all system calls to their default
    ** settings and return NULL
    */
    rc = SQLITE_OK;
    для(i=0; i<sizeof(aSyscall)/sizeof(aSyscall[0]); i++){
      якщо( aSyscall[i].pDefault ){
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  }інакше{
    /* If zName is specified, operate on only the one system call
    ** specified.
    */
    для(i=0; i<sizeof(aSyscall)/sizeof(aSyscall[0]); i++){
      якщо( strcmp(zName, aSyscall[i].zName)==0 ){
        якщо( aSyscall[i].pDefault==0 ){
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = SQLITE_OK;
        якщо( pNewFunc==0 ) pNewFunc = aSyscall[i].pDefault;
        aSyscall[i].pCurrent = pNewFunc;
        break;
      }
    }
  }
  повернути rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
static sqlite3_syscall_ptr winGetSystemCall(
  sqlite3_vfs *pNotUsed,
  const char *zName
){
  unsigned ціле i;

  UNUSED_PARAMETER(pNotUsed);
  для(i=0; i<sizeof(aSyscall)/sizeof(aSyscall[0]); i++){
    якщо( strcmp(zName, aSyscall[i].zName)==0 ) повернути aSyscall[i].pCurrent;
  }
  повернути 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
static const char *winNextSystemCall(sqlite3_vfs *p, const char *zName){
  ціле i = -1;

  UNUSED_PARAMETER(p);
  якщо( zName ){
    для(i=0; i<ArraySize(aSyscall)-1; i++){
      якщо( strcmp(zName, aSyscall[i].zName)==0 ) break;
    }
  }
  для(i++; i<ArraySize(aSyscall); i++){
    якщо( aSyscall[i].pCurrent!=0 ) повернути aSyscall[i].zName;
  }
  повернути 0;
}

#ifdef SQLITE_WIN32_MALLOC
#ifdef SQLITE_UWP
# error SQLITE_WIN32_MALLOC is incompatible with SQLITE_UWP
#endif
/*
** If a Win32 native heap has been configured, this function will attempt to
** compact it.  Upon success, SQLITE_OK will be returned.  Upon failure, one
** of SQLITE_NOMEM, SQLITE_ERROR, or SQLITE_NOTFOUND will be returned.  The
** "pnLargest" argument, if non-zero, will be used to return the size of the
** largest committed free block in the heap, in bytes.
*/
ціле sqlite3_win32_compact_heap(LPUINT pnLargest){
  ціле rc = SQLITE_OK;
  UINT nLargest = 0;
  HANDLE hHeap;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  assert( hHeap!=0 );
  assert( hHeap!=INVALID_HANDLE_VALUE );
#якщо defined(SQLITE_WIN32_MALLOC_VALIDATE)
  assert( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, NULL) );
#endif
  якщо( (nLargest=osHeapCompact(hHeap, SQLITE_WIN32_HEAP_FLAGS))==0 ){
    DWORD lastErrno = osGetLastError();
    якщо( lastErrno==NO_ERROR ){
      sqlite3_log(SQLITE_NOMEM, "failed to HeapCompact (no space), heap=%p",
                  (void*)hHeap);
      rc = SQLITE_NOMEM_BKPT;
    }інакше{
      sqlite3_log(SQLITE_ERROR, "failed to HeapCompact (%lu), heap=%p",
                  osGetLastError(), (void*)hHeap);
      rc = SQLITE_ERROR;
    }
  }
  якщо( pnLargest ) *pnLargest = nLargest;
  повернути rc;
}

/*
** If a Win32 native heap has been configured, this function will attempt to
** destroy and recreate it.  If the Win32 native heap is not isolated and/or
** the sqlite3_memory_used() function does not return zero, SQLITE_BUSY will
** be returned and no changes will be made to the Win32 native heap.
*/
ціле sqlite3_win32_reset_heap(){
  ціле rc;
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; ) /* The main static mutex */
  MUTEX_LOGIC( sqlite3_mutex *pMem; )    /* The memsys static mutex */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  MUTEX_LOGIC( pMem = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3_mutex_enter(pMem);
  winMemAssertMagic();
  якщо( winMemGetHeap()!=NULL && winMemGetOwned() && sqlite3_memory_used()==0 ){
    /*
    ** At this point, there should be no outstanding memory allocations on
    ** the heap.  Also, since both the main and memsys locks are currently
    ** being held by us, no other function (i.e. from another thread) should
    ** be able to even access the heap.  Attempt to destroy and recreate our
    ** isolated Win32 native heap now.
    */
    assert( winMemGetHeap()!=NULL );
    assert( winMemGetOwned() );
    assert( sqlite3_memory_used()==0 );
    winMemShutdown(winMemGetDataPtr());
    assert( winMemGetHeap()==NULL );
    assert( !winMemGetOwned() );
    assert( sqlite3_memory_used()==0 );
    rc = winMemInit(winMemGetDataPtr());
    assert( rc!=SQLITE_OK || winMemGetHeap()!=NULL );
    assert( rc!=SQLITE_OK || winMemGetOwned() );
    assert( rc!=SQLITE_OK || sqlite3_memory_used()==0 );
  }інакше{
    /*
    ** The Win32 native heap cannot be modified because it may be in use.
    */
    rc = SQLITE_BUSY;
  }
  sqlite3_mutex_leave(pMem);
  sqlite3_mutex_leave(pMainMtx);
  повернути rc;
}
#endif /* SQLITE_WIN32_MALLOC */

#ifdef _WIN32
/*
** This function outputs the specified (ANSI) string to the Win32 debugger
** (if available).  Undocumented.  Might go away at any moment.
*/
void sqlite3_win32_write_debug(const char *zBuf, ціле nBuf){
  char zDbgBuf[SQLITE_WIN32_DBG_BUF_SIZE];
  ціле nMin = MIN(nBuf, (SQLITE_WIN32_DBG_BUF_SIZE - 1)); /* may be negative. */
  якщо( nMin<-1 ) nMin = -1; /* all negative values become -1. */
  assert( nMin==-1 || nMin==0 || nMin<SQLITE_WIN32_DBG_BUF_SIZE );
#ifdef SQLITE_ENABLE_API_ARMOR
  якщо( !zBuf ){
    (void)SQLITE_MISUSE_BKPT;
    повернути;
  }
#endif
  якщо( nMin>0 ){
    memset(zDbgBuf, 0, SQLITE_WIN32_DBG_BUF_SIZE);
    memcpy(zDbgBuf, zBuf, nMin);
    osOutputDebugStringA(zDbgBuf);
  }інакше{
    osOutputDebugStringA(zBuf);
  }
}

void sqlite3_win32_sleep(DWORD milliseconds){
  osSleep(milliseconds);
}

#якщо SQLITE_MAX_WORKER_THREADS>0 && SQLITE_THREADSAFE>0
DWORD sqlite3Win32Wait(HANDLE hObject){
  DWORD rc;
  поки( (rc = osWaitForSingleObjectEx(hObject, INFINITE,
                                       TRUE))==WAIT_IO_COMPLETION ){}
  повернути rc;
}
#endif
#endif /* _WIN32 */

#ifdef SQLITE_WIN32_MALLOC
/*
** Allocate nBytes of memory.
*/
static void *winMemMalloc(ціле nBytes){
  HANDLE hHeap;
  void *p;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  assert( hHeap!=0 );
  assert( hHeap!=INVALID_HANDLE_VALUE );
#якщо defined(SQLITE_WIN32_MALLOC_VALIDATE)
  assert( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, NULL) );
#endif
  assert( nBytes>=0 );
  p = osHeapAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, (SIZE_T)nBytes);
  якщо( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapAlloc %u bytes (%lu), heap=%p",
                nBytes, osGetLastError(), (void*)hHeap);
  }
  повернути p;
}

/*
** Free memory.
*/
static void winMemFree(void *pPrior){
  HANDLE hHeap;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  assert( hHeap!=0 );
  assert( hHeap!=INVALID_HANDLE_VALUE );
#якщо defined(SQLITE_WIN32_MALLOC_VALIDATE)
  assert( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) );
#endif
  якщо( !pPrior ) повернути; /* Passing NULL to HeapFree is undefined. */
  якщо( !osHeapFree(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapFree block %p (%lu), heap=%p",
                pPrior, osGetLastError(), (void*)hHeap);
  }
}

/*
** Change the size of an existing memory allocation
*/
static void *winMemRealloc(void *pPrior, ціле nBytes){
  HANDLE hHeap;
  void *p;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  assert( hHeap!=0 );
  assert( hHeap!=INVALID_HANDLE_VALUE );
#якщо defined(SQLITE_WIN32_MALLOC_VALIDATE)
  assert( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) );
#endif
  assert( nBytes>=0 );
  якщо( !pPrior ){
    p = osHeapAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, (SIZE_T)nBytes);
  }інакше{
    p = osHeapReAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior, (SIZE_T)nBytes);
  }
  якщо( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to %s %u bytes (%lu), heap=%p",
                pPrior ? "HeapReAlloc" : "HeapAlloc", nBytes, osGetLastError(),
                (void*)hHeap);
  }
  повернути p;
}

/*
** Return the size of an outstanding allocation, in bytes.
*/
static ціле winMemSize(void *p){
  HANDLE hHeap;
  SIZE_T n;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  assert( hHeap!=0 );
  assert( hHeap!=INVALID_HANDLE_VALUE );
#якщо defined(SQLITE_WIN32_MALLOC_VALIDATE)
  assert( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, p) );
#endif
  якщо( !p ) повернути 0;
  n = osHeapSize(hHeap, SQLITE_WIN32_HEAP_FLAGS, p);
  якщо( n==(SIZE_T)-1 ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapSize block %p (%lu), heap=%p",
                p, osGetLastError(), (void*)hHeap);
    повернути 0;
  }
  повернути (ціле)n;
}

/*
** Round up a request size to the next valid allocation size.
*/
static ціле winMemRoundup(ціле n){
  повернути n;
}

/*
** Initialize this module.
*/
static ціле winMemInit(void *pAppData){
  winMemData *pWinMemData = (winMemData *)pAppData;

  якщо( !pWinMemData ) повернути SQLITE_ERROR;
  assert( pWinMemData->magic1==WINMEM_MAGIC1 );
  assert( pWinMemData->magic2==WINMEM_MAGIC2 );

#якщо SQLITE_WIN32_HEAP_CREATE
  якщо( !pWinMemData->hHeap ){
    DWORD dwInitialSize = SQLITE_WIN32_HEAP_INIT_SIZE;
    DWORD dwMaximumSize = (DWORD)sqlite3GlobalConfig.nHeap;
    якщо( dwMaximumSize==0 ){
      dwMaximumSize = SQLITE_WIN32_HEAP_MAX_SIZE;
    }інакше якщо( dwInitialSize>dwMaximumSize ){
      dwInitialSize = dwMaximumSize;
    }
    pWinMemData->hHeap = osHeapCreate(SQLITE_WIN32_HEAP_FLAGS,
                                      dwInitialSize, dwMaximumSize);
    якщо( !pWinMemData->hHeap ){
      sqlite3_log(SQLITE_NOMEM,
          "failed to HeapCreate (%lu), flags=%u, initSize=%lu, maxSize=%lu",
          osGetLastError(), SQLITE_WIN32_HEAP_FLAGS, dwInitialSize,
          dwMaximumSize);
      повернути SQLITE_NOMEM_BKPT;
    }
    pWinMemData->bOwned = TRUE;
    assert( pWinMemData->bOwned );
  }
#інакше
  pWinMemData->hHeap = osGetProcessHeap();
  якщо( !pWinMemData->hHeap ){
    sqlite3_log(SQLITE_NOMEM,
        "failed to GetProcessHeap (%lu)", osGetLastError());
    повернути SQLITE_NOMEM_BKPT;
  }
  pWinMemData->bOwned = FALSE;
  assert( !pWinMemData->bOwned );
#endif
  assert( pWinMemData->hHeap!=0 );
  assert( pWinMemData->hHeap!=INVALID_HANDLE_VALUE );
#якщо defined(SQLITE_WIN32_MALLOC_VALIDATE)
  assert( osHeapValidate(pWinMemData->hHeap, SQLITE_WIN32_HEAP_FLAGS, NULL) );
#endif
  повернути SQLITE_OK;
}

/*
** Deinitialize this module.
*/
static void winMemShutdown(void *pAppData){
  winMemData *pWinMemData = (winMemData *)pAppData;

  якщо( !pWinMemData ) повернути;
  assert( pWinMemData->magic1==WINMEM_MAGIC1 );
  assert( pWinMemData->magic2==WINMEM_MAGIC2 );

  якщо( pWinMemData->hHeap ){
    assert( pWinMemData->hHeap!=INVALID_HANDLE_VALUE );
#якщо defined(SQLITE_WIN32_MALLOC_VALIDATE)
    assert( osHeapValidate(pWinMemData->hHeap, SQLITE_WIN32_HEAP_FLAGS, NULL) );
#endif
    якщо( pWinMemData->bOwned ){
      якщо( !osHeapDestroy(pWinMemData->hHeap) ){
        sqlite3_log(SQLITE_NOMEM, "failed to HeapDestroy (%lu), heap=%p",
                    osGetLastError(), (void*)pWinMemData->hHeap);
      }
      pWinMemData->bOwned = FALSE;
    }
    pWinMemData->hHeap = NULL;
  }
}

/*
** Populate the low-level memory allocation function pointers in
** sqlite3GlobalConfig.m with pointers to the routines in this file. The
** arguments specify the block of memory to manage.
**
** This routine is only called by sqlite3_config(), and therefore
** is not required to be threadsafe (it is not).
*/
const sqlite3_mem_methods *sqlite3MemGetWin32(void){
  static const sqlite3_mem_methods winMemMethods = {
    winMemMalloc,
    winMemFree,
    winMemRealloc,
    winMemSize,
    winMemRoundup,
    winMemInit,
    winMemShutdown,
    &win_mem_data
  };
  повернути &winMemMethods;
}

void sqlite3MemSetDefault(void){
  sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetWin32());
}
#endif /* SQLITE_WIN32_MALLOC */

#ifdef _WIN32
/*
** Convert a UTF-8 string to Microsoft Unicode.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
static LPWSTR winUtf8ToUnicode(const char *zText){
  ціле nChar;
  LPWSTR zWideText;

  nChar = osMultiByteToWideChar(CP_UTF8, 0, zText, -1, NULL, 0);
  якщо( nChar==0 ){
    повернути 0;
  }
  zWideText = sqlite3MallocZero( nChar*sizeof(WCHAR) );
  якщо( zWideText==0 ){
    повернути 0;
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zText, -1, zWideText,
                                nChar);
  якщо( nChar==0 ){
    sqlite3_free(zWideText);
    zWideText = 0;
  }
  повернути zWideText;
}
#endif /* _WIN32 */

/*
** Convert a Microsoft Unicode string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
static char *winUnicodeToUtf8(LPCWSTR zWideText){
  ціле nByte;
  char *zText;

  nByte = osWideCharToMultiByte(CP_UTF8, 0, zWideText, -1, 0, 0, 0, 0);
  якщо( nByte == 0 ){
    повернути 0;
  }
  zText = sqlite3MallocZero( nByte );
  якщо( zText==0 ){
    повернути 0;
  }
  nByte = osWideCharToMultiByte(CP_UTF8, 0, zWideText, -1, zText, nByte,
                                0, 0);
  якщо( nByte == 0 ){
    sqlite3_free(zText);
    zText = 0;
  }
  повернути zText;
}

#ifdef _WIN32
/*
** Convert an ANSI string to Microsoft Unicode, using the ANSI or OEM
** code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
static LPWSTR winMbcsToUnicode(const char *zText, ціле useAnsi){
  ціле nWideChar;
  LPWSTR zMbcsText;
  ціле codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nWideChar = osMultiByteToWideChar(codepage, 0, zText, -1, NULL,
                                0);
  якщо( nWideChar==0 ){
    повернути 0;
  }
  zMbcsText = sqlite3MallocZero( nWideChar*sizeof(WCHAR) );
  якщо( zMbcsText==0 ){
    повернути 0;
  }
  nWideChar = osMultiByteToWideChar(codepage, 0, zText, -1, zMbcsText,
                                nWideChar);
  якщо( nWideChar==0 ){
    sqlite3_free(zMbcsText);
    zMbcsText = 0;
  }
  повернути zMbcsText;
}

/*
** Convert a Microsoft Unicode string to a multi-byte character string,
** using the ANSI or OEM code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
static char *winUnicodeToMbcs(LPCWSTR zWideText, ціле useAnsi){
  ціле nByte;
  char *zText;
  ціле codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nByte = osWideCharToMultiByte(codepage, 0, zWideText, -1, 0, 0, 0, 0);
  якщо( nByte == 0 ){
    повернути 0;
  }
  zText = sqlite3MallocZero( nByte );
  якщо( zText==0 ){
    повернути 0;
  }
  nByte = osWideCharToMultiByte(codepage, 0, zWideText, -1, zText,
                                nByte, 0, 0);
  якщо( nByte == 0 ){
    sqlite3_free(zText);
    zText = 0;
  }
  повернути zText;
}

/*
** Convert a multi-byte character string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
static char *winMbcsToUtf8(const char *zText, ціле useAnsi){
  char *zTextUtf8;
  LPWSTR zTmpWide;

  zTmpWide = winMbcsToUnicode(zText, useAnsi);
  якщо( zTmpWide==0 ){
    повернути 0;
  }
  zTextUtf8 = winUnicodeToUtf8(zTmpWide);
  sqlite3_free(zTmpWide);
  повернути zTextUtf8;
}

/*
** Convert a UTF-8 string to a multi-byte character string.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
static char *winUtf8ToMbcs(const char *zText, ціле useAnsi){
  char *zTextMbcs;
  LPWSTR zTmpWide;

  zTmpWide = winUtf8ToUnicode(zText);
  якщо( zTmpWide==0 ){
    повернути 0;
  }
  zTextMbcs = winUnicodeToMbcs(zTmpWide, useAnsi);
  sqlite3_free(zTmpWide);
  повернути zTextMbcs;
}

/*
** This is a public wrapper for the winUtf8ToUnicode() function.
*/
LPWSTR sqlite3_win32_utf8_to_unicode(const char *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  якщо( !zText ){
    (void)SQLITE_MISUSE_BKPT;
    повернути 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  якщо( sqlite3_initialize() ) повернути 0;
#endif
  повернути winUtf8ToUnicode(zText);
}

/*
** This is a public wrapper for the winUnicodeToUtf8() function.
*/
char *sqlite3_win32_unicode_to_utf8(LPCWSTR zWideText){
#ifdef SQLITE_ENABLE_API_ARMOR
  якщо( !zWideText ){
    (void)SQLITE_MISUSE_BKPT;
    повернути 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  якщо( sqlite3_initialize() ) повернути 0;
#endif
  повернути winUnicodeToUtf8(zWideText);
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
char *sqlite3_win32_mbcs_to_utf8(const char *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  якщо( !zText ){
    (void)SQLITE_MISUSE_BKPT;
    повернути 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  якщо( sqlite3_initialize() ) повернути 0;
#endif
  повернути winMbcsToUtf8(zText, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
char *sqlite3_win32_mbcs_to_utf8_v2(const char *zText, ціле useAnsi){
#ifdef SQLITE_ENABLE_API_ARMOR
  якщо( !zText ){
    (void)SQLITE_MISUSE_BKPT;
    повернути 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  якщо( sqlite3_initialize() ) повернути 0;
#endif
  повернути winMbcsToUtf8(zText, useAnsi);
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
char *sqlite3_win32_utf8_to_mbcs(const char *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  якщо( !zText ){
    (void)SQLITE_MISUSE_BKPT;
    повернути 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  якщо( sqlite3_initialize() ) повернути 0;
#endif
  повернути winUtf8ToMbcs(zText, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
char *sqlite3_win32_utf8_to_mbcs_v2(const char *zText, ціле useAnsi){
#ifdef SQLITE_ENABLE_API_ARMOR
  якщо( !zText ){
    (void)SQLITE_MISUSE_BKPT;
    повернути 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  якщо( sqlite3_initialize() ) повернути 0;
#endif
  повернути winUtf8ToMbcs(zText, useAnsi);
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-8 string.
*/
ціле sqlite3_win32_set_directory8(
  unsigned long type, /* Identifier for directory being set or reset */
  const char *zValue  /* New value for directory being set or reset */
){
  char **ppDirectory = 0;
  ціле rc;
#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  якщо( rc ) повернути rc;
#endif
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  якщо( type==SQLITE_WIN32_DATA_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_data_directory;
  }інакше якщо( type==SQLITE_WIN32_TEMP_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_temp_directory;
  }
  assert( !ppDirectory || type==SQLITE_WIN32_DATA_DIRECTORY_TYPE
          || type==SQLITE_WIN32_TEMP_DIRECTORY_TYPE
  );
  assert( !ppDirectory || sqlite3MemdebugHasType(*ppDirectory, MEMTYPE_HEAP) );
  якщо( ppDirectory ){
    char *zCopy = 0;
    якщо( zValue && zValue[0] ){
      zCopy = sqlite3_mprintf("%s", zValue);
      якщо ( zCopy==0 ){
        rc = SQLITE_NOMEM_BKPT;
        goto set_directory8_done;
      }
    }
    sqlite3_free(*ppDirectory);
    *ppDirectory = zCopy;
    rc = SQLITE_OK;
  }інакше{
    rc = SQLITE_ERROR;
  }
set_directory8_done:
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  повернути rc;
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-16 string.
*/
ціле sqlite3_win32_set_directory16(
  unsigned long type, /* Identifier for directory being set or reset */
  const void *zValue  /* New value for directory being set or reset */
){
  ціле rc;
  char *zUtf8 = 0;
  якщо( zValue ){
    zUtf8 = sqlite3_win32_unicode_to_utf8(zValue);
    якщо( zUtf8==0 ) повернути SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_win32_set_directory8(type, zUtf8);
  якщо( zUtf8 ) sqlite3_free(zUtf8);
  повернути rc;
}

/*
** This function sets the data directory or the temporary directory based on
** the provided arguments.  The type argument must be 1 in order to set the
** data directory or 2 in order to set the temporary directory.  The zValue
** argument is the name of the directory to use.  The return value will be
** SQLITE_OK if successful.
*/
ціле sqlite3_win32_set_directory(
  unsigned long type, /* Identifier for directory being set or reset */
  void *zValue        /* New value for directory being set or reset */
){
  повернути sqlite3_win32_set_directory16(type, zValue);
}
#endif /* _WIN32 */

/*
** The return value of winGetLastErrorMsg
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated).
*/
static ціле winGetLastErrorMsg(DWORD lastErrno, ціле nBuf, char *zBuf){
  /* FormatMessage returns 0 on failure.  Otherwise it
  ** returns the number of TCHARs written to the output
  ** buffer, excluding the terminating null char.
  */
  DWORD dwLen = 0;
  char *zOut = 0;
  LPWSTR zTempWide = NULL;
  dwLen = osFormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_FROM_SYSTEM |
                           FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL,
                           lastErrno,
                           0,
                           (LPWSTR) &zTempWide,
                           0,
                           0);
  якщо( dwLen > 0 ){
    /* allocate a buffer and convert to UTF8 */
    sqlite3BeginBenignMalloc();
    zOut = winUnicodeToUtf8(zTempWide);
    sqlite3EndBenignMalloc();
    /* free the system buffer allocated by FormatMessage */
    osLocalFree(zTempWide);
  }
  якщо( 0 == dwLen ){
    sqlite3_snprintf(nBuf, zBuf, "OsError 0x%lx (%lu)", lastErrno, lastErrno);
  }інакше{
    /* copy a maximum of nBuf chars to output buffer */
    sqlite3_snprintf(nBuf, zBuf, "%s", zOut);
    /* free the UTF8 buffer */
    sqlite3_free(zOut);
  }
  повернути 0;
}

/*
**
** This function - winLogErrorAtLine() - is only ever called via the macro
** winLogError().
**
** This routine is invoked after an error occurs in an OS function.
** It logs a message using sqlite3_log() containing the current value of
** error code and, if possible, the human-readable equivalent from
** FormatMessage.
**
** The first argument passed to the macro should be the error code that
** will be returned to SQLite (e.g. SQLITE_IOERR_DELETE, SQLITE_CANTOPEN).
** The two subsequent arguments should be the name of the OS function that
** failed and the associated file-system path, if any.
*/
#define winLogError(a,b,c,d)   winLogErrorAtLine(a,b,c,d,__LINE__)
static ціле winLogErrorAtLine(
  ціле errcode,                    /* SQLite error code */
  DWORD lastErrno,                /* Win32 last error */
  const char *zFunc,              /* Name of OS function that failed */
  const char *zPath,              /* File path associated with error */
  ціле iLine                       /* Source line number where error occurred */
){
  char zMsg[500];                 /* Human readable error text */
  ціле i;                          /* Loop counter */

  zMsg[0] = 0;
  winGetLastErrorMsg(lastErrno, sizeof(zMsg), zMsg);
  assert( errcode!=SQLITE_OK );
  якщо( zPath==0 ) zPath = "";
  для(i=0; zMsg[i] && zMsg[i]!='\r' && zMsg[i]!='\n'; i++){}
  zMsg[i] = 0;
  sqlite3_log(errcode,
      "os_win.c:%d: (%lu) %s(%s) - %s",
      iLine, lastErrno, zFunc, zPath, zMsg
  );

  повернути errcode;
}

/*
** The number of times that a ReadFile(), WriteFile(), and DeleteFile()
** will be retried following a locking error - probably caused by
** antivirus software.  Also the initial delay before the first retry.
** The delay increases linearly with each retry.
*/
#ifndef SQLITE_WIN32_IOERR_RETRY
# define SQLITE_WIN32_IOERR_RETRY 10
#endif
#ifndef SQLITE_WIN32_IOERR_RETRY_DELAY
# define SQLITE_WIN32_IOERR_RETRY_DELAY 25
#endif
static ціле winIoerrRetry = SQLITE_WIN32_IOERR_RETRY;
static ціле winIoerrRetryDelay = SQLITE_WIN32_IOERR_RETRY_DELAY;

/*
** The "winIoerrCanRetry1" macro is used to determine if a particular I/O
** error code obtained via GetLastError() is eligible to be retried.  It
** must accept the error code DWORD as its only argument and should return
** non-zero if the error code is transient in nature and the operation
** responsible for generating the original error might succeed upon being
** retried.  The argument to this macro should be a variable.
**
** Additionally, a macro named "winIoerrCanRetry2" may be defined.  If it
** is defined, it will be consulted only when the macro "winIoerrCanRetry1"
** returns zero.  The "winIoerrCanRetry2" macro is completely optional and
** may be used to include additional error codes in the set that should
** result in the failing I/O operation being retried by the caller.  If
** defined, the "winIoerrCanRetry2" macro must exhibit external semantics
** identical to those of the "winIoerrCanRetry1" macro.
*/
#якщо !defined(winIoerrCanRetry1)
#define winIoerrCanRetry1(a) (((a)==ERROR_ACCESS_DENIED)        || \
                              ((a)==ERROR_SHARING_VIOLATION)    || \
                              ((a)==ERROR_LOCK_VIOLATION)       || \
                              ((a)==ERROR_DEV_NOT_EXIST)        || \
                              ((a)==ERROR_NETNAME_DELETED)      || \
                              ((a)==ERROR_SEM_TIMEOUT)          || \
                              ((a)==ERROR_NETWORK_UNREACHABLE))
#endif

/*
** If a ReadFile() or WriteFile() error occurs, invoke this routine
** to see if it should be retried.  Return TRUE to retry.  Return FALSE
** to give up with an error.
*/
static ціле winRetryIoerr(ціле *pnRetry, DWORD *pError){
  DWORD e = osGetLastError();
  якщо( *pnRetry>=winIoerrRetry ){
    якщо( pError ){
      *pError = e;
    }
    повернути 0;
  }
  якщо( winIoerrCanRetry1(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    повернути 1;
  }
#якщо defined(winIoerrCanRetry2)
  інакше якщо( winIoerrCanRetry2(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    повернути 1;
  }
#endif
  якщо( pError ){
    *pError = e;
  }
  повернути 0;
}

/*
** Log a I/O error retry episode.
*/
static void winLogIoerr(ціле nRetry, ціле lineno){
  якщо( nRetry ){
    sqlite3_log(SQLITE_NOTICE,
      "delayed %dms for lock/sharing conflict at line %d",
      winIoerrRetryDelay*nRetry*(nRetry+1)/2, lineno
    );
  }
}


/*
** Lock a file region.
*/
static BOOL winLockFile(
  LPHANDLE phFile,
  DWORD flags,
  DWORD offsetLow,
  DWORD offsetHigh,
  DWORD numBytesLow,
  DWORD numBytesHigh
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, sizeof(OVERLAPPED));
  ovlp.Offset = offsetLow;
  ovlp.OffsetHigh = offsetHigh;
  повернути osLockFileEx(*phFile, flags, 0, numBytesLow, numBytesHigh, &ovlp);
}

#ifndef SQLITE_OMIT_WAL
/*
** Lock a region of nByte bytes starting at offset offset of file hFile.
** Take an EXCLUSIVE lock if parameter bExclusive is true, or a SHARED lock
** otherwise. If nMs is greater than zero and the lock cannot be obtained
** immediately, block for that many ms before giving up.
**
** This function returns SQLITE_OK if the lock is obtained successfully. If
** some other process holds the lock, SQLITE_BUSY is returned if nMs==0, or
** SQLITE_BUSY_TIMEOUT otherwise. Or, if an error occurs, SQLITE_IOERR.
*/
static ціле winHandleLockTimeout(
  HANDLE hFile,
  DWORD offset,
  DWORD nByte,
  ціле bExcl,
  DWORD nMs
){
  DWORD flags = LOCKFILE_FAIL_IMMEDIATELY | (bExcl?LOCKFILE_EXCLUSIVE_LOCK:0);
  ціле rc = SQLITE_OK;
  BOOL ret;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, sizeof(OVERLAPPED));
  ovlp.Offset = offset;

#якщо defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  якщо( nMs!=0 ){
    flags &= ~LOCKFILE_FAIL_IMMEDIATELY;
  }
  ovlp.hEvent = osCreateEvent(NULL, TRUE, FALSE, NULL);
  якщо( ovlp.hEvent==NULL ){
    повернути SQLITE_IOERR_LOCK;
  }
#endif
  ret = osLockFileEx(hFile, flags, 0, nByte, 0, &ovlp);
#якщо defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  /* If SQLITE_ENABLE_SETLK_TIMEOUT is defined, then the file-handle was
  ** opened with FILE_FLAG_OVERHEAD specified. In this case, the call to
  ** LockFileEx() may fail because the request is still pending. This can
  ** happen even if LOCKFILE_FAIL_IMMEDIATELY was specified.  
  **
  ** If nMs is 0, then LOCKFILE_FAIL_IMMEDIATELY was set in the flags 
  ** passed to LockFileEx(). In this case, if the operation is pending,
  ** block indefinitely until it is finished.
  **
  ** Otherwise, wait for up to nMs ms for the operation to finish. nMs
  ** may be set to INFINITE.
  */
  якщо( !ret && GetLastError()==ERROR_IO_PENDING ){
    DWORD nDelay = (nMs==0 ? INFINITE : nMs);
    DWORD res = osWaitForSingleObject(ovlp.hEvent, nDelay);
    якщо( res==WAIT_OBJECT_0 ){
      ret = TRUE;
    }інакше якщо( res==WAIT_TIMEOUT ){
#якщо SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = SQLITE_BUSY_TIMEOUT;
#інакше
      rc = SQLITE_BUSY;
#endif
    }інакше{
      /* Some other error has occurred */
      rc = SQLITE_IOERR_LOCK;
    }

    /* If it is still pending, cancel the LockFileEx() call. */
    osCancelIo(hFile);
  }

  osCloseHandle(ovlp.hEvent);
#endif /* defined(SQLITE_ENABLE_SETLK_TIMEOUT) */

  якщо( rc==SQLITE_OK && !ret ){
    rc = SQLITE_BUSY;
  }
  повернути rc;
}
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Unlock a file region.
 */
static BOOL winUnlockFile(
  LPHANDLE phFile,
  DWORD offsetLow,
  DWORD offsetHigh,
  DWORD numBytesLow,
  DWORD numBytesHigh
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, sizeof(OVERLAPPED));
  ovlp.Offset = offsetLow;
  ovlp.OffsetHigh = offsetHigh;
  повернути osUnlockFileEx(*phFile, 0, numBytesLow, numBytesHigh, &ovlp);
}

#ifndef SQLITE_OMIT_WAL
/*
** Remove an nByte lock starting at offset iOff from HANDLE h.
*/
static ціле winHandleUnlock(HANDLE h, ціле iOff, ціле nByte){
  BOOL ret = winUnlockFile(&h, iOff, 0, nByte, 0);
  повернути (ret ? SQLITE_OK : SQLITE_IOERR_UNLOCK);
}
#endif

/*****************************************************************************
** The next group of routines implement the I/O methods specified
** by the sqlite3_io_methods object.
******************************************************************************/

/*
** Some Microsoft compilers lack this definition.
*/
#ifndef INVALID_SET_FILE_POINTER
# define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

/*
** Seek the file handle h to offset nByte of the file.
**
** If successful, return SQLITE_OK. Or, if an error occurs, return an SQLite
** error code.
*/
static ціле winHandleSeek(HANDLE h, sqlite3_int64 iOffset){
  ціле rc = SQLITE_OK;             /* Return value */
  LARGE_INTEGER x;                /* The offset */

  x.QuadPart = iOffset;
  якщо( osSetFilePointerEx(h, x, 0, FILE_BEGIN)==0 ){
    rc = SQLITE_IOERR_SEEK;
  }
  OSTRACE(("SEEK file=%p, offset=%lld rc=%s\n", h, iOffset,sqlite3ErrName(rc)));
  повернути rc;
}

/*
** Move the current position of the file handle passed as the first
** argument to offset iOffset within the file. If successful, return 0.
** Otherwise, set pFile->lastErrno and return non-zero.
*/
static ціле winSeekFile(winFile *pFile, sqlite3_int64 iOffset){
  ціле rc;

  rc = winHandleSeek(pFile->h, iOffset);
  якщо( rc!=SQLITE_OK ){
    pFile->lastErrno = osGetLastError();
    winLogError(rc, pFile->lastErrno, "winSeekFile", pFile->zPath);
  }
  повернути rc;
}


#якщо SQLITE_MAX_MMAP_SIZE>0
/* Forward references to VFS helper methods used for memory mapped files */
static ціле winMapfile(winFile*, sqlite3_int64);
static ціле winUnmapfile(winFile*);
#endif

/*
** Close a file.
**
** It is reported that an attempt to close a handle might sometimes
** fail.  This is a very unreasonable result, but Windows is notorious
** for being unreasonable so I do not doubt that it might happen.  If
** the close fails, we pause for 100 milliseconds and try again.  As
** many as MX_CLOSE_ATTEMPT attempts to close the handle are made before
** giving up and returning an error.
*/
#define MX_CLOSE_ATTEMPT 3
static ціле winClose(sqlite3_file *id){
  ціле rc, cnt = 0;
  winFile *pFile = (winFile*)id;

  assert( id!=0 );
#ifndef SQLITE_OMIT_WAL
  assert( pFile->pShm==0 );
#endif
  assert( pFile->h!=NULL && pFile->h!=INVALID_HANDLE_VALUE );
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p\n",
           osGetCurrentProcessId(), pFile, pFile->h));

#якщо SQLITE_MAX_MMAP_SIZE>0
  winUnmapfile(pFile);
#endif

  do{
    rc = osCloseHandle(pFile->h);
    /* SimulateIOError( rc=0; cnt=MX_CLOSE_ATTEMPT; ); */
  }поки( rc==0 && ++cnt < MX_CLOSE_ATTEMPT && (osSleep(100), 1) );
  якщо( rc ){
    pFile->h = NULL;
  }
  OpenCounter(-1);
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, rc ? "ok" : "failed"));
  повернути rc ? SQLITE_OK
            : winLogError(SQLITE_IOERR_CLOSE, osGetLastError(),
                          "winClose", pFile->zPath);
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
static ціле winRead(
  sqlite3_file *id,          /* File to read from */
  void *pBuf,                /* Write content into this buffer */
  ціле amt,                   /* Number of bytes to read */
  sqlite3_int64 offset       /* Begin reading at this offset */
){
#якщо !defined(SQLITE_WIN32_NO_OVERLAPPED)
  OVERLAPPED overlapped;          /* The offset for ReadFile. */
#endif
  winFile *pFile = (winFile*)id;  /* file handle */
  DWORD nRead;                    /* Number of bytes actually read from file */
  ціле nRetry = 0;                 /* Number of retrys */

  assert( id!=0 );
  assert( amt>0 );
  assert( offset>=0 );
  SimulateIOError(повернути SQLITE_IOERR_READ);
  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, offset, pFile->locktype));

#якщо SQLITE_MAX_MMAP_SIZE>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  якщо( offset<pFile->mmapSize ){
    якщо( offset+amt <= pFile->mmapSize ){
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], amt);
      OSTRACE(("READ-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      повернути SQLITE_OK;
    }інакше{
      ціле nCopy = (ціле)(pFile->mmapSize - offset);
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#endif

#якщо defined(SQLITE_WIN32_NO_OVERLAPPED)
  якщо( winSeekFile(pFile, offset) ){
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    повернути SQLITE_FULL;
  }
  поки( !osReadFile(pFile->h, pBuf, amt, &nRead, 0) ){
#інакше
  memset(&overlapped, 0, sizeof(OVERLAPPED));
  overlapped.Offset = (LONG)(offset & 0xffffffff);
  overlapped.OffsetHigh = (LONG)((offset>>32) & 0x7fffffff);
  поки( !osReadFile(pFile->h, pBuf, amt, &nRead, &overlapped) &&
         osGetLastError()!=ERROR_HANDLE_EOF ){
#endif
    DWORD lastErrno;
    якщо( winRetryIoerr(&nRetry, &lastErrno) ) continue;
    pFile->lastErrno = lastErrno;
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    повернути winLogError(SQLITE_IOERR_READ, pFile->lastErrno,
                       "winRead", pFile->zPath);
  }
  winLogIoerr(nRetry, __LINE__);
  якщо( nRead<(DWORD)amt ){
    /* Unread parts of the buffer must be zero-filled */
    memset(&((char*)pBuf)[nRead], 0, amt-nRead);
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_SHORT_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    повернути SQLITE_IOERR_SHORT_READ;
  }

  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  повернути SQLITE_OK;
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
static ціле winWrite(
  sqlite3_file *id,               /* File to write into */
  const void *pBuf,               /* The bytes to be written */
  ціле amt,                        /* Number of bytes to write */
  sqlite3_int64 offset            /* Offset into the file to begin writing at */
){
  ціле rc = 0;                     /* True if error has occurred, else false */
  winFile *pFile = (winFile*)id;  /* File handle */
  ціле nRetry = 0;                 /* Number of retries */

  assert( amt>0 );
  assert( pFile );
  SimulateIOError(повернути SQLITE_IOERR_WRITE);
  SimulateDiskfullError(повернути SQLITE_FULL);

  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, offset, pFile->locktype));

#якщо defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_SIZE>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  якщо( offset<pFile->mmapSize ){
    якщо( offset+amt <= pFile->mmapSize ){
      memcpy(&((u8 *)(pFile->pMapRegion))[offset], pBuf, amt);
      OSTRACE(("WRITE-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      повернути SQLITE_OK;
    }інакше{
      ціле nCopy = (ціле)(pFile->mmapSize - offset);
      memcpy(&((u8 *)(pFile->pMapRegion))[offset], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#endif

#якщо defined(SQLITE_WIN32_NO_OVERLAPPED)
  rc = winSeekFile(pFile, offset);
  якщо( rc==0 ){
#інакше
  {
#endif
#якщо !defined(SQLITE_WIN32_NO_OVERLAPPED)
    OVERLAPPED overlapped;        /* The offset for WriteFile. */
#endif
    u8 *aRem = (u8 *)pBuf;        /* Data yet to be written */
    ціле nRem = amt;               /* Number of bytes yet to be written */
    DWORD nWrite;                 /* Bytes written by each WriteFile() call */
    DWORD lastErrno = NO_ERROR;   /* Value returned by GetLastError() */

#якщо !defined(SQLITE_WIN32_NO_OVERLAPPED)
    memset(&overlapped, 0, sizeof(OVERLAPPED));
    overlapped.Offset = (LONG)(offset & 0xffffffff);
    overlapped.OffsetHigh = (LONG)((offset>>32) & 0x7fffffff);
#endif

    поки( nRem>0 ){
#якщо defined(SQLITE_WIN32_NO_OVERLAPPED)
      якщо( !osWriteFile(pFile->h, aRem, nRem, &nWrite, 0) ){
#інакше
      якщо( !osWriteFile(pFile->h, aRem, nRem, &nWrite, &overlapped) ){
#endif
        якщо( winRetryIoerr(&nRetry, &lastErrno) ) continue;
        break;
      }
      assert( nWrite==0 || nWrite<=(DWORD)nRem );
      якщо( nWrite==0 || nWrite>(DWORD)nRem ){
        lastErrno = osGetLastError();
        break;
      }
#якщо !defined(SQLITE_WIN32_NO_OVERLAPPED)
      offset += nWrite;
      overlapped.Offset = (LONG)(offset & 0xffffffff);
      overlapped.OffsetHigh = (LONG)((offset>>32) & 0x7fffffff);
#endif
      aRem += nWrite;
      nRem -= nWrite;
    }
    якщо( nRem>0 ){
      pFile->lastErrno = lastErrno;
      rc = 1;
    }
  }

  якщо( rc ){
    якщо(   ( pFile->lastErrno==ERROR_HANDLE_DISK_FULL )
       || ( pFile->lastErrno==ERROR_DISK_FULL )){
      OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      повернути winLogError(SQLITE_FULL, pFile->lastErrno,
                         "winWrite1", pFile->zPath);
    }
    OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_WRITE\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    повернути winLogError(SQLITE_IOERR_WRITE, pFile->lastErrno,
                       "winWrite2", pFile->zPath);
  }інакше{
    winLogIoerr(nRetry, __LINE__);
  }
  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  повернути SQLITE_OK;
}

#ifndef SQLITE_OMIT_WAL
/*
** Truncate the file opened by handle h to nByte bytes in size.
*/
static ціле winHandleTruncate(HANDLE h, sqlite3_int64 nByte){ 
  ціле rc = SQLITE_OK;             /* Return code */
  rc = winHandleSeek(h, nByte);
  якщо( rc==SQLITE_OK ){
    якщо( 0==osSetEndOfFile(h) ){
      rc = SQLITE_IOERR_TRUNCATE;
    }
  }
  повернути rc;
}

/*
** Determine the size in bytes of the file opened by the handle passed as 
** the first argument.
*/
static ціле winHandleSize(HANDLE h, sqlite3_int64 *pnByte){ 
  ціле rc = SQLITE_OK;
  LARGE_INTEGER x;
  assert( pnByte );
  якщо( osGetFileSizeEx(h, &x)==0 ){
    rc = SQLITE_IOERR_FSTAT;
    *pnByte = 0;
  }інакше{
    *pnByte = x.QuadPart;
  }
  повернути rc;
}

/*
** Close the handle passed as the only argument.
*/
static void winHandleClose(HANDLE h){
  якщо( h!=INVALID_HANDLE_VALUE ){
    osCloseHandle(h);
  }
}
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Truncate an open file to a specified size
*/
static ціле winTruncate(sqlite3_file *id, sqlite3_int64 nByte){
  winFile *pFile = (winFile*)id;  /* File handle object */
  ціле rc = SQLITE_OK;             /* Return code for this function */
  DWORD lastErrno;
#якщо SQLITE_MAX_MMAP_SIZE>0
  sqlite3_int64 oldMmapSize;
  якщо( pFile->nFetchOut>0 ){
    /* File truncation is a no-op if there are outstanding memory mapped
    ** pages.  This is because truncating the file means temporarily unmapping
    ** the file, and that might delete memory out from under existing cursors.
    **
    ** This can result in incremental vacuum not truncating the file,
    ** if there is an active read cursor when the incremental vacuum occurs.
    ** No real harm comes of this - the database file is not corrupted,
    ** though some folks might complain that the file is bigger than it
    ** needs to be.
    **
    ** The only feasible work-around is to defer the truncation until after
    ** all references to memory-mapped content are closed.  That is doable,
    ** but involves adding a few branches in the common write code path which
    ** could slow down normal operations slightly.  Hence, we have decided for
    ** now to simply make transactions a no-op if there are pending reads.  We
    ** can maybe revisit this decision in the future.
    */
    повернути SQLITE_OK;
  }
#endif

  assert( pFile );
  SimulateIOError(повернути SQLITE_IOERR_TRUNCATE);
  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, size=%lld, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, nByte, pFile->locktype));

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  якщо( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

#якщо SQLITE_MAX_MMAP_SIZE>0
  якщо( pFile->pMapRegion ){
    oldMmapSize = pFile->mmapSize;
  }інакше{
    oldMmapSize = 0;
  }
  winUnmapfile(pFile);
#endif

  /* SetEndOfFile() returns non-zero when successful, or zero when it fails. */
  якщо( winSeekFile(pFile, nByte) ){
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->lastErrno,
                     "winTruncate1", pFile->zPath);
  }інакше якщо( 0==osSetEndOfFile(pFile->h) &&
            ((lastErrno = osGetLastError())!=ERROR_USER_MAPPED_FILE) ){
    pFile->lastErrno = lastErrno;
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->lastErrno,
                     "winTruncate2", pFile->zPath);
  }

#якщо SQLITE_MAX_MMAP_SIZE>0
  якщо( rc==SQLITE_OK && oldMmapSize>0 ){
    якщо( oldMmapSize>nByte ){
      winMapfile(pFile, -1);
    }інакше{
      winMapfile(pFile, oldMmapSize);
    }
  }
#endif

  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, sqlite3ErrName(rc)));
  повернути rc;
}

#ifdef SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occurring at the right times.
*/
ціле sqlite3_sync_count = 0;
ціле sqlite3_fullsync_count = 0;
#endif

/*
** Make sure all writes to a particular file are committed to disk.
*/
static ціле winSync(sqlite3_file *id, ціле flags){
#ifndef SQLITE_NO_SYNC
  /*
  ** Used only when SQLITE_NO_SYNC is not defined.
   */
  BOOL rc;
#endif
#якщо !defined(NDEBUG) || !defined(SQLITE_NO_SYNC) || \
    defined(SQLITE_HAVE_OS_TRACE)
  /*
  ** Used when SQLITE_NO_SYNC is not defined and by the assert() and/or
  ** OSTRACE() macros.
   */
  winFile *pFile = (winFile*)id;
#інакше
  UNUSED_PARAMETER(id);
#endif

  assert( pFile );
  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  assert((flags&0x0F)==SQLITE_SYNC_NORMAL
      || (flags&0x0F)==SQLITE_SYNC_FULL
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( повернути SQLITE_FULL );

  OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, flags=%x, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, flags,
           pFile->locktype));

#ifndef SQLITE_TEST
  UNUSED_PARAMETER(flags);
#інакше
  якщо( (flags&0x0F)==SQLITE_SYNC_FULL ){
    sqlite3_fullsync_count++;
  }
  sqlite3_sync_count++;
#endif

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op
  */
#ifdef SQLITE_NO_SYNC
  OSTRACE(("SYNC-NOP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  повернути SQLITE_OK;
#інакше
#якщо SQLITE_MAX_MMAP_SIZE>0
  якщо( pFile->pMapRegion ){
    якщо( osFlushViewOfFile(pFile->pMapRegion, 0) ){
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_OK\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
    }інакше{
      pFile->lastErrno = osGetLastError();
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
      повернути winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winSync1", pFile->zPath);
    }
  }
#endif
  rc = osFlushFileBuffers(pFile->h);
  SimulateIOError( rc=FALSE );
  якщо( rc ){
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    повернути SQLITE_OK;
  }інакше{
    pFile->lastErrno = osGetLastError();
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_FSYNC\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    повернути winLogError(SQLITE_IOERR_FSYNC, pFile->lastErrno,
                       "winSync2", pFile->zPath);
  }
#endif
}

/*
** Determine the current size of a file in bytes
*/
static ціле winFileSize(sqlite3_file *id, sqlite3_int64 *pSize){
  winFile *pFile = (winFile*)id;
  ціле rc = SQLITE_OK;

  assert( id!=0 );
  assert( pSize!=0 );
  SimulateIOError(повернути SQLITE_IOERR_FSTAT);
  OSTRACE(("SIZE file=%p, pSize=%p\n", pFile->h, pSize));
  {
    LARGE_INTEGER x;
    якщо( osGetFileSizeEx(pFile->h, &x)==0 ){
      *pSize = 0;
      pFile->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_FSTAT, pFile->lastErrno,
                       "winFileSize", pFile->zPath);
    }інакше{
      *pSize = x.QuadPart;
    }
  }
  OSTRACE(("SIZE file=%p, pSize=%p, *pSize=%lld, rc=%s\n",
           pFile->h, pSize, *pSize, sqlite3ErrName(rc)));
  повернути rc;
}

/*
** LOCKFILE_FAIL_IMMEDIATELY is undefined on some Windows systems.
*/
#ifndef LOCKFILE_FAIL_IMMEDIATELY
# define LOCKFILE_FAIL_IMMEDIATELY 1
#endif

#ifndef LOCKFILE_EXCLUSIVE_LOCK
# define LOCKFILE_EXCLUSIVE_LOCK 2
#endif

/*
** Historically, SQLite has used both the LockFile and LockFileEx functions.
** When the LockFile function was used, it was always expected to fail
** immediately if the lock could not be obtained.  Also, it always expected to
** obtain an exclusive lock.  These flags are used with the LockFileEx function
** and reflect those expectations; therefore, they should not be changed.
*/
#ifndef SQLITE_LOCKFILE_FLAGS
# define SQLITE_LOCKFILE_FLAGS   (LOCKFILE_FAIL_IMMEDIATELY | \
                                  LOCKFILE_EXCLUSIVE_LOCK)
#endif

/*
** Currently, SQLite never calls the LockFileEx function without wanting the
** call to fail immediately if the lock cannot be obtained.
*/
#ifndef SQLITE_LOCKFILEEX_FLAGS
# define SQLITE_LOCKFILEEX_FLAGS (LOCKFILE_FAIL_IMMEDIATELY)
#endif

/*
** Acquire a reader lock.
** Different API routines are called depending on whether or not this
** is Win9x or WinNT.
*/
static ціле winGetReadLock(winFile *pFile, ціле bBlock){
  ціле res;
  DWORD mask = ~(bBlock ? LOCKFILE_FAIL_IMMEDIATELY : 0);
  OSTRACE(("READ-LOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winLockFile(&pFile->h, SQLITE_LOCKFILEEX_FLAGS&mask, SHARED_FIRST, 0,
                    SHARED_SIZE, 0);
  якщо( res == 0 ){
    pFile->lastErrno = osGetLastError();
    /* No need to log a failure to lock */
  }
  OSTRACE(("READ-LOCK file=%p, result=%d\n", pFile->h, res));
  повернути res;
}

/*
** Undo a readlock
*/
static ціле winUnlockReadLock(winFile *pFile){
  ціле res;
  DWORD lastErrno;
  OSTRACE(("READ-UNLOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winUnlockFile(&pFile->h, SHARED_FIRST, 0, SHARED_SIZE, 0);
  якщо( res==0 && ((lastErrno = osGetLastError())!=ERROR_NOT_LOCKED) ){
    pFile->lastErrno = lastErrno;
    winLogError(SQLITE_IOERR_UNLOCK, pFile->lastErrno,
                "winUnlockReadLock", pFile->zPath);
  }
  OSTRACE(("READ-UNLOCK file=%p, result=%d\n", pFile->h, res));
  повернути res;
}

/*
** Lock the file with the lock specified by parameter locktype - one
** of the following:
**
**     (1) SHARED_LOCK
**     (2) RESERVED_LOCK
**     (3) PENDING_LOCK
**     (4) EXCLUSIVE_LOCK
**
** Sometimes when requesting one lock state, additional lock states
** are inserted in between.  The locking might fail on one of the later
** transitions leaving the lock state different from what it started but
** still short of its goal.  The following chart shows the allowed
** transitions and the inserted intermediate states:
**
**    UNLOCKED -> SHARED
**    SHARED -> RESERVED
**    SHARED -> (PENDING) -> EXCLUSIVE
**    RESERVED -> (PENDING) -> EXCLUSIVE
**    PENDING -> EXCLUSIVE
**
** This routine will only increase a lock.  The winUnlock() routine
** erases all locks at once and returns us immediately to locking level 0.
** It is not possible to lower the locking level one step at a time.  You
** must go straight to locking level 0.
*/
static ціле winLock(sqlite3_file *id, ціле locktype){
  ціле rc = SQLITE_OK;    /* Return code from subroutines */
  ціле res = 1;           /* Result of a Windows lock call */
  ціле newLocktype;       /* Set pFile->locktype to this value before exiting */
  ціле gotPendingLock = 0;/* True if we acquired a PENDING lock this time */
  winFile *pFile = (winFile*)id;
  DWORD lastErrno = NO_ERROR;

  assert( id!=0 );
  OSTRACE(("LOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->sharedLockByte, locktype));

  /* If there is already a lock of this type or more restrictive on the
  ** OsFile, do nothing. Don't use the end_lock: exit path, as
  ** sqlite3OsEnterMutex() hasn't been called yet.
  */
  якщо( pFile->locktype>=locktype ){
    OSTRACE(("LOCK-HELD file=%p, rc=SQLITE_OK\n", pFile->h));
    повернути SQLITE_OK;
  }

  /* Do not allow any kind of write-lock on a read-only database
  */
  якщо( (pFile->ctrlFlags & WINFILE_RDONLY)!=0 && locktype>=RESERVED_LOCK ){
    повернути SQLITE_IOERR_LOCK;
  }

  /* Make sure the locking sequence is correct
  */
  assert( pFile->locktype!=NO_LOCK || locktype==SHARED_LOCK );
  assert( locktype!=PENDING_LOCK );
  assert( locktype!=RESERVED_LOCK || pFile->locktype==SHARED_LOCK );

  /* Lock the PENDING_LOCK byte if we need to acquire an EXCLUSIVE lock or
  ** a SHARED lock.  If we are acquiring a SHARED lock, the acquisition of
  ** the PENDING_LOCK byte is temporary.
  */
  newLocktype = pFile->locktype;
  якщо( locktype==SHARED_LOCK
   || (locktype==EXCLUSIVE_LOCK && pFile->locktype==RESERVED_LOCK)
  ){
    ціле cnt = 3;

    /* Flags for the LockFileEx() call. This should be an exclusive lock if
    ** this call is to obtain EXCLUSIVE, or a shared lock if this call is to
    ** obtain SHARED.  */
    ціле flags = LOCKFILE_FAIL_IMMEDIATELY;
    якщо( locktype==EXCLUSIVE_LOCK ){
      flags |= LOCKFILE_EXCLUSIVE_LOCK;
    }
    поки( cnt>0 ){
      /* Try 3 times to get the pending lock.  This is needed to work
      ** around problems caused by indexing and/or anti-virus software on
      ** Windows systems.
      **
      ** If you are using this code as a model for alternative VFSes, do not
      ** copy this retry logic.  It is a hack intended for Windows only.  */
      res = winLockFile(&pFile->h, flags, PENDING_BYTE, 0, 1, 0);
      якщо( res ) break;

      lastErrno = osGetLastError();
      OSTRACE(("LOCK-PENDING-FAIL file=%p, count=%d, result=%d\n", 
            pFile->h, cnt, res
      ));

      якщо( lastErrno==ERROR_INVALID_HANDLE ){
        pFile->lastErrno = lastErrno;
        rc = SQLITE_IOERR_LOCK;
        OSTRACE(("LOCK-FAIL file=%p, count=%d, rc=%s\n", 
              pFile->h, cnt, sqlite3ErrName(rc)
        ));
        повернути rc;
      }

      cnt--;
      якщо( cnt>0 ) osSleep(1);
    }
    gotPendingLock = res;
  }

  /* Acquire a shared lock
  */
  якщо( locktype==SHARED_LOCK && res ){
    assert( pFile->locktype==NO_LOCK );
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    res = winGetReadLock(pFile, pFile->bBlockOnConnect);
#інакше
    res = winGetReadLock(pFile, 0);
#endif
    якщо( res ){
      newLocktype = SHARED_LOCK;
    }інакше{
      lastErrno = osGetLastError();
    }
  }

  /* Acquire a RESERVED lock
  */
  якщо( locktype==RESERVED_LOCK && res ){
    assert( pFile->locktype==SHARED_LOCK );
    res = winLockFile(&pFile->h, SQLITE_LOCKFILE_FLAGS, RESERVED_BYTE, 0, 1, 0);
    якщо( res ){
      newLocktype = RESERVED_LOCK;
    }інакше{
      lastErrno = osGetLastError();
    }
  }

  /* Acquire a PENDING lock
  */
  якщо( locktype==EXCLUSIVE_LOCK && res ){
    newLocktype = PENDING_LOCK;
    gotPendingLock = 0;
  }

  /* Acquire an EXCLUSIVE lock
  */
  якщо( locktype==EXCLUSIVE_LOCK && res ){
    assert( pFile->locktype>=SHARED_LOCK );
    (void)winUnlockReadLock(pFile);
    res = winLockFile(&pFile->h, SQLITE_LOCKFILE_FLAGS, SHARED_FIRST, 0,
                      SHARED_SIZE, 0);
    якщо( res ){
      newLocktype = EXCLUSIVE_LOCK;
    }інакше{
      lastErrno = osGetLastError();
      winGetReadLock(pFile, 0);
    }
  }

  /* If we are holding a PENDING lock that ought to be released, then
  ** release it now.
  */
  якщо( gotPendingLock && locktype==SHARED_LOCK ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }

  /* Update the state of the lock has held in the file descriptor then
  ** return the appropriate result code.
  */
  якщо( res ){
    rc = SQLITE_OK;
  }інакше{
    pFile->lastErrno = lastErrno;
    rc = SQLITE_BUSY;
    OSTRACE(("LOCK-FAIL file=%p, wanted=%d, got=%d\n",
             pFile->h, locktype, newLocktype));
  }
  pFile->locktype = (u8)newLocktype;
  OSTRACE(("LOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  повернути rc;
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, return
** non-zero, otherwise zero.
*/
static ціле winCheckReservedLock(sqlite3_file *id, ціле *pResOut){
  ціле res;
  winFile *pFile = (winFile*)id;

  SimulateIOError( повернути SQLITE_IOERR_CHECKRESERVEDLOCK; );
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p\n", pFile->h, pResOut));

  assert( id!=0 );
  якщо( pFile->locktype>=RESERVED_LOCK ){
    res = 1;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (local)\n", pFile->h, res));
  }інакше{
    res = winLockFile(&pFile->h, SQLITE_LOCKFILEEX_FLAGS,RESERVED_BYTE,0,1,0);
    якщо( res ){
      winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
    }
    res = !res;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (remote)\n", pFile->h, res));
  }
  *pResOut = res;
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           pFile->h, pResOut, *pResOut));
  повернути SQLITE_OK;
}

/*
** Lower the locking level on file descriptor id to locktype.  locktype
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
**
** It is not possible for this routine to fail if the second argument
** is NO_LOCK.  If the second argument is SHARED_LOCK then this routine
** might return SQLITE_IOERR;
*/
static ціле winUnlock(sqlite3_file *id, ціле locktype){
  ціле type;
  winFile *pFile = (winFile*)id;
  ціле rc = SQLITE_OK;
  assert( pFile!=0 );
  assert( locktype<=SHARED_LOCK );
  OSTRACE(("UNLOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->sharedLockByte, locktype));
  type = pFile->locktype;
  якщо( type>=EXCLUSIVE_LOCK ){
    winUnlockFile(&pFile->h, SHARED_FIRST, 0, SHARED_SIZE, 0);
    якщо( locktype==SHARED_LOCK && !winGetReadLock(pFile, 0) ){
      /* This should never happen.  We should always be able to
      ** reacquire the read lock */
      rc = winLogError(SQLITE_IOERR_UNLOCK, osGetLastError(),
                       "winUnlock", pFile->zPath);
    }
  }
  якщо( type>=RESERVED_LOCK ){
    winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
  }
  якщо( locktype==NO_LOCK && type>=SHARED_LOCK ){
    winUnlockReadLock(pFile);
  }
  якщо( type>=PENDING_LOCK ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }
  pFile->locktype = (u8)locktype;
  OSTRACE(("UNLOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  повернути rc;
}

/******************************************************************************
****************************** No-op Locking **********************************
**
** Of the various locking implementations available, this is by far the
** simplest:  locking is ignored.  No attempt is made to lock the database
** file for reading or writing.
**
** This locking mode is appropriate for use on read-only databases
** (ex: databases that are burned into CD-ROM, for example.)  It can
** also be used if the application employs some external mechanism to
** prevent simultaneous access of the same database by two or more
** database connections.  But there is a serious risk of database
** corruption if this locking mode is used in situations where multiple
** database connections are accessing the same database file at the same
** time and one or more of those connections are writing.
*/

static ціле winNolockLock(sqlite3_file *id, ціле locktype){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(locktype);
  повернути SQLITE_OK;
}

static ціле winNolockCheckReservedLock(sqlite3_file *id, ціле *pResOut){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(pResOut);
  повернути SQLITE_OK;
}

static ціле winNolockUnlock(sqlite3_file *id, ціле locktype){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(locktype);
  повернути SQLITE_OK;
}

/******************* End of the no-op lock implementation *********************
******************************************************************************/

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
static void winModeBit(winFile *pFile, unsigned char mask, ціле *pArg){
  якщо( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & mask)!=0;
  }інакше якщо( (*pArg)==0 ){
    pFile->ctrlFlags &= ~mask;
  }інакше{
    pFile->ctrlFlags |= mask;
  }
}

/* Forward references to VFS helper methods used for temporary files */
static ціле winGetTempname(sqlite3_vfs *, char **);
static ціле winIsDir(LPCWSTR);
static BOOL winIsLongPathPrefix(const char *);
static BOOL winIsDriveLetterAndColon(const char *);

/*
** Control and query of the open file handle.
*/
static ціле winFileControl(sqlite3_file *id, ціле op, void *pArg){
  winFile *pFile = (winFile*)id;
  OSTRACE(("FCNTL file=%p, op=%d, pArg=%p\n", pFile->h, op, pArg));
  switch( op ){
    case SQLITE_FCNTL_LOCKSTATE: {
      *(ціле*)pArg = pFile->locktype;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      повернути SQLITE_OK;
    }
    case SQLITE_FCNTL_LAST_ERRNO: {
      *(ціле*)pArg = (ціле)pFile->lastErrno;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      повернути SQLITE_OK;
    }
    case SQLITE_FCNTL_CHUNK_SIZE: {
      pFile->szChunk = *(ціле *)pArg;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      повернути SQLITE_OK;
    }
    case SQLITE_FCNTL_SIZE_HINT: {
      якщо( pFile->szChunk>0 ){
        sqlite3_int64 oldSz;
        ціле rc = winFileSize(id, &oldSz);
        якщо( rc==SQLITE_OK ){
          sqlite3_int64 newSz = *(sqlite3_int64*)pArg;
          якщо( newSz>oldSz ){
            SimulateIOErrorBenign(1);
            rc = winTruncate(id, newSz);
            SimulateIOErrorBenign(0);
          }
        }
        OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
        повернути rc;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      повернути SQLITE_OK;
    }
    case SQLITE_FCNTL_PERSIST_WAL: {
      winModeBit(pFile, WINFILE_PERSIST_WAL, (ціле*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      повернути SQLITE_OK;
    }
    case SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      winModeBit(pFile, WINFILE_PSOW, (ціле*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      повернути SQLITE_OK;
    }
    case SQLITE_FCNTL_VFSNAME: {
      *(char**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      повернути SQLITE_OK;
    }
    case SQLITE_FCNTL_WIN32_AV_RETRY: {
      ціле *a = (ціле*)pArg;
      якщо( a[0]>0 ){
        winIoerrRetry = a[0];
      }інакше{
        a[0] = winIoerrRetry;
      }
      якщо( a[1]>0 ){
        winIoerrRetryDelay = a[1];
      }інакше{
        a[1] = winIoerrRetryDelay;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      повернути SQLITE_OK;
    }
    case SQLITE_FCNTL_WIN32_GET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      *phFile = pFile->h;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      повернути SQLITE_OK;
    }
#ifdef SQLITE_TEST
    case SQLITE_FCNTL_WIN32_SET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      HANDLE hOldFile = pFile->h;
      pFile->h = *phFile;
      *phFile = hOldFile;
      OSTRACE(("FCNTL oldFile=%p, newFile=%p, rc=SQLITE_OK\n",
               hOldFile, pFile->h));
      повернути SQLITE_OK;
    }
#endif
    case SQLITE_FCNTL_NULL_IO: {
      (void)osCloseHandle(pFile->h);
      pFile->h = NULL;
      повернути SQLITE_OK;
    }
    case SQLITE_FCNTL_TEMPFILENAME: {
      char *zTFile = 0;
      ціле rc = winGetTempname(pFile->pVfs, &zTFile);
      якщо( rc==SQLITE_OK ){
        *(char**)pArg = zTFile;
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      повернути rc;
    }
#якщо SQLITE_MAX_MMAP_SIZE>0
    case SQLITE_FCNTL_MMAP_SIZE: {
      i64 newLimit = *(i64*)pArg;
      ціле rc = SQLITE_OK;
      якщо( newLimit>sqlite3GlobalConfig.mxMmap ){
        newLimit = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (SIZE_T) and passed
      ** to MapViewOfFile(). Restrict its value to 2GB if (SIZE_T) is not at
      ** least a 64-bit type. */
      якщо( newLimit>0 && sizeof(SIZE_T)<8 ){
        newLimit = (newLimit & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmapSizeMax;
      якщо( newLimit>=0 && newLimit!=pFile->mmapSizeMax && pFile->nFetchOut==0 ){
        pFile->mmapSizeMax = newLimit;
        якщо( pFile->mmapSize>0 ){
          winUnmapfile(pFile);
          rc = winMapfile(pFile, -1);
        }
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      повернути rc;
    }
#endif

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    case SQLITE_FCNTL_LOCK_TIMEOUT: {
      ціле iOld = pFile->iBusyTimeout;
      ціле iNew = *(ціле*)pArg;
#якщо SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = (iNew < 0) ? INFINITE : (DWORD)iNew;
#elif SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = (DWORD)(!!iNew);
#інакше
# error "SQLITE_ENABLE_SETLK_TIMEOUT must be set to 1 or 2"
#endif
      *(ціле*)pArg = iOld;
      повернути SQLITE_OK;
    }
    case SQLITE_FCNTL_BLOCK_ON_CONNECT: {
      ціле iNew = *(ціле*)pArg;
      pFile->bBlockOnConnect = iNew;
      повернути SQLITE_OK;
    }
#endif /* SQLITE_ENABLE_SETLK_TIMEOUT */

#якщо defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    case SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      sqlite3_str_appendf(pStr, "{\"h\":%llu", (sqlite3_uint64)pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      якщо( pFile->locktype ){
        static const char *azLock[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
        sqlite3_str_appendf(pStr, ",\"locktype\":\"%s\"", 
                                  azLock[pFile->locktype-1]);
      }
#якщо SQLITE_MAX_MMAP_SIZE>0
      якщо( pFile->mmapSize ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmapSize);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->nFetchOut);
      }
#endif
      sqlite3_str_append(pStr, "}", 1);
      повернути SQLITE_OK;
    }
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

  }
  OSTRACE(("FCNTL file=%p, rc=SQLITE_NOTFOUND\n", pFile->h));
  повернути SQLITE_NOTFOUND;
}

/*
** Return the sector size in bytes of the underlying block device for
** the specified file. This is almost always 512 bytes, but may be
** larger for some devices.
**
** SQLite code assumes this function cannot fail. It also assumes that
** if two files are created in the same file-system directory (i.e.
** a database and its journal file) that the sector size will be the
** same for both.
*/
static ціле winSectorSize(sqlite3_file *id){
  (void)id;
  повернути SQLITE_DEFAULT_SECTOR_SIZE;
}

/*
** Return a vector of device characteristics.
*/
static ціле winDeviceCharacteristics(sqlite3_file *id){
  winFile *p = (winFile*)id;
  повернути SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN | SQLITE_IOCAP_SUBPAGE_READ |
         ((p->ctrlFlags & WINFILE_PSOW)?SQLITE_IOCAP_POWERSAFE_OVERWRITE:0);
}

/*
** Windows will only let you create file view mappings
** on allocation size granularity boundaries.
** During sqlite3_os_init() we do a GetSystemInfo()
** to get the granularity size.
*/
static SYSTEM_INFO winSysInfo;

/*
** Convert a UTF-8 filename into whatever form the underlying
** operating system wants filenames in.  Space to hold the result
** is obtained from malloc and must be freed by the calling
** function
**
** On Cygwin, 3 possible input forms are accepted:
** - If the filename starts with "<drive>:/" or "<drive>:\",
**   it is converted to UTF-16 as-is.
** - If the filename contains '/', it is assumed to be a
**   Cygwin absolute path, it is converted to a win32
**   absolute path in UTF-16.
** - Otherwise it must be a filename only, the win32 filename
**   is returned in UTF-16.
** Note: If the function cygwin_conv_path() fails, only
**   UTF-8 -> UTF-16 conversion will be done. This can only
**   happen when the file path >32k, in which case winUtf8ToUnicode()
**   will fail too.
*/
static LPWSTR winConvertFromUtf8Filename(const char *zFilename){
  LPWSTR zConverted = 0;
#ifdef __CYGWIN__
  ціле nChar;

  якщо( osCygwin_conv_path && !(winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2])) ){
    i64 nByte;
    ціле convertflag = CCP_POSIX_TO_WIN_W;
    якщо( !strchr(zFilename, '/') ) convertflag |= CCP_RELATIVE;
    nByte = (i64)osCygwin_conv_path(convertflag,
        zFilename, 0, 0);
    якщо( nByte>0 ){
      zConverted = sqlite3MallocZero(12+(u64)nByte);
      якщо ( zConverted==0 ){
        повернути zConverted;
      }
      /* Filenames should be prefixed, except when converted
       * full path already starts with "\\?\". */
      якщо( osCygwin_conv_path(convertflag, zFilename,
                           zConverted+4, nByte)==0 ){
        якщо( (convertflag&CCP_RELATIVE) ){
          memmove(zConverted, zConverted+4, nByte);
        }інакше якщо( memcmp(zConverted+4, L"\\\\", 4) ){
          memcpy(zConverted, L"\\\\?\\", 8);
        }інакше якщо( zConverted[6]!='?' ){
          memmove(zConverted+6, zConverted+4, nByte);
          memcpy(zConverted, L"\\\\?\\UNC", 14);
        }інакше{
          memmove(zConverted, zConverted+4, nByte);
        }
        повернути zConverted;
      }
      sqlite3_free(zConverted);
    }
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zFilename, -1, NULL, 0);
  якщо( nChar==0 ){
    повернути 0;
  }
  zConverted = sqlite3MallocZero( ((u64)nChar+6)*sizeof(WCHAR) );
  якщо( zConverted==0 ){
    повернути 0;
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zFilename, -1,
                                zConverted, nChar);
  якщо( nChar==0 ){
    sqlite3_free(zConverted);
    zConverted = 0;
  }інакше якщо( nChar>MAX_PATH
      && winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2]) ){
    memmove(zConverted+4, zConverted, nChar*sizeof(WCHAR));
    zConverted[2] = '\\';
    memcpy(zConverted, L"\\\\?\\", 8);
  }інакше якщо( nChar>MAX_PATH
      && winIsDirSep(zFilename[0]) && winIsDirSep(zFilename[1])
      && zFilename[2] != '?' ){
    memmove(zConverted+6, zConverted, nChar*sizeof(WCHAR));
    memcpy(zConverted, L"\\\\?\\UNC", 14);
  }
#інакше /* if !defined(__CYGWIN__) */
  zConverted = winUtf8ToUnicode(zFilename);
#endif /* __CYGWIN__ */
  /* caller will handle out of memory */
  повернути zConverted;
}

#ifndef SQLITE_OMIT_WAL

/*
** Helper functions to obtain and relinquish the global mutex. The
** global mutex is used to protect the winLockInfo objects used by
** this file, all of which may be shared by multiple threads.
**
** Function winShmMutexHeld() is used to assert() that the global mutex
** is held when required. This function is only used as part of assert()
** statements. e.g.
**
**   winShmEnterMutex()
**     assert( winShmMutexHeld() );
**   winShmLeaveMutex()
*/
static sqlite3_mutex *winBigLock = 0;
static void winShmEnterMutex(void){
  sqlite3_mutex_enter(winBigLock);
}
static void winShmLeaveMutex(void){
  sqlite3_mutex_leave(winBigLock);
}
#ifndef NDEBUG
static ціле winShmMutexHeld(void) {
  повернути sqlite3_mutex_held(winBigLock);
}
#endif

/*
** Object used to represent a single file opened and mmapped to provide
** shared memory.  When multiple threads all reference the same
** log-summary, each thread has its own winFile object, but they all
** point to a single instance of this object.  In other words, each
** log-summary is opened only once per process.
**
** winShmMutexHeld() must be true when creating or destroying
** this object, or while editing the global linked list that starts
** at winShmNodeList.
**
** When reading or writing the linked list starting at winShmNode.pWinShmList,
** pShmNode->mutex must be held.
**
** The following fields are constant after the object is created:
**
**      zFilename
**      hSharedShm
**      mutex
**      bUseSharedLockHandle
**
** Either winShmNode.mutex must be held or winShmNode.pWinShmList==0 and
** winShmMutexHeld() is true when reading or writing any other field
** in this structure.
**
** File-handle hSharedShm is always used to (a) take the DMS lock, (b) 
** truncate the *-shm file if the DMS-locking protocol demands it, and 
** (c) map regions of the *-shm file into memory using MapViewOfFile() 
** or similar. If bUseSharedLockHandle is true, then other locks are also 
** taken on hSharedShm. Or, if bUseSharedLockHandle is false, then other 
** locks are taken using each connection's winShm.hShm handles.
*/
struct winShmNode {
  sqlite3_mutex *mutex;      /* Mutex to access this object */
  char *zFilename;           /* Name of the file */
  HANDLE hSharedShm;         /* File handle open on zFilename */
  ціле bUseSharedLockHandle;  /* True to use hSharedShm for everything */

  ціле isUnlocked;            /* DMS lock has not yet been obtained */
  ціле isReadonly;            /* True if read-only */
  ціле szRegion;              /* Size of shared-memory regions */
  ціле nRegion;               /* Size of array apRegion */

  struct ShmRegion {
    HANDLE hMap;             /* File handle from CreateFileMapping */
    void *pMap;
  } *aRegion;
  DWORD lastErrno;           /* The Windows errno from the last I/O error */

  winShm *pWinShmList;       /* List of winShm objects with ptrs to this */

  winShmNode *pNext;         /* Next in list of all winShmNode objects */
#якщо defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 nextShmId;              /* Next available winShm.id value */
#endif
};

/*
** A global array of all winShmNode objects.
**
** The winShmMutexHeld() must be true while reading or writing this list.
*/
static winShmNode *winShmNodeList = 0;

/*
** Structure used internally by this VFS to record the state of an
** open shared memory connection. There is one such structure for each
** winFile open on a wal mode database.
*/
struct winShm {
  winShmNode *pShmNode;      /* The underlying winShmNode object */
  u16 sharedMask;            /* Mask of shared locks held */
  u16 exclMask;              /* Mask of exclusive locks held */
  HANDLE hShm;               /* File-handle on *-shm file. For locking. */
  ціле bReadonly;             /* True if hShm is opened read-only */
#якщо defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 id;                     /* Id of this connection with its winShmNode */
#endif
  winShm *pWinShmNext;       /* Next winShm object on same winShmNode */
};

/*
** Constants used for locking
*/
#define WIN_SHM_BASE   ((22+SQLITE_SHM_NLOCK)*4)        /* first lock byte */
#define WIN_SHM_DMS    (WIN_SHM_BASE+SQLITE_SHM_NLOCK)  /* deadman switch */

/* Forward references to VFS methods */
static ціле winOpen(sqlite3_vfs*,const char*,sqlite3_file*,ціле,ціле*);
static ціле winDelete(sqlite3_vfs *,const char*,ціле);

/*
** Purge the winShmNodeList list of all entries with winShmNode.pWinShmList==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
static void winShmPurge(sqlite3_vfs *pVfs, ціле deleteFlag){
  winShmNode **pp;
  winShmNode *p;
  assert( winShmMutexHeld() );
  OSTRACE(("SHM-PURGE pid=%lu, deleteFlag=%d\n",
           osGetCurrentProcessId(), deleteFlag));
  pp = &winShmNodeList;
  поки( (p = *pp)!=0 ){
    якщо( p->pWinShmList==0 ){
      ціле i;
      якщо( p->mutex ){ sqlite3_mutex_free(p->mutex); }
      для(i=0; i<p->nRegion; i++){
        BOOL bRc = osUnmapViewOfFile(p->aRegion[i].pMap);
        OSTRACE(("SHM-PURGE-UNMAP pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        UNUSED_VARIABLE_VALUE(bRc);
        bRc = osCloseHandle(p->aRegion[i].hMap);
        OSTRACE(("SHM-PURGE-CLOSE pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        UNUSED_VARIABLE_VALUE(bRc);
      }
      winHandleClose(p->hSharedShm);
      якщо( deleteFlag ){
        SimulateIOErrorBenign(1);
        sqlite3BeginBenignMalloc();
        winDelete(pVfs, p->zFilename, 0);
        sqlite3EndBenignMalloc();
        SimulateIOErrorBenign(0);
      }
      *pp = p->pNext;
      sqlite3_free(p->aRegion);
      sqlite3_free(p);
    }інакше{
      pp = &p->pNext;
    }
  }
}

/*
** Handle h is open on the *-shm file. This function zeroes the first
** 8 bytes of the file. If SQLITE_ENABLE_SETLK_TIMEOUT is defined, h may 
** have been opened with FILE_FLAG_OVERLAPPED.
*/
static ціле winZeroSharedMemory(HANDLE h){
  u8 aZero[8] = {0,0,0,0,0,0,0,0};
  DWORD nWrite = 0;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, sizeof(OVERLAPPED));
  якщо( !osWriteFile(h, aZero, sizeof(aZero), &nWrite, &ovlp) ){
    /* If the file was opened with FILE_FLAG_OVERLAPPED, the write may still
    ** be pending. Wait for it with GetOverlappedResult().  */
    якщо( osGetLastError()!=ERROR_IO_PENDING
     || !GetOverlappedResult(h, &ovlp, &nWrite, TRUE)
    ){
      nWrite = 0;
    }
  }
  повернути (nWrite==sizeof(aZero) ? SQLITE_OK : SQLITE_IOERR_WRITE);
}

/*
** The DMS lock has not yet been taken on the shm file associated with
** pShmNode. Take the lock. Truncate the *-shm file if required.
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
static ціле winLockSharedMemory(winShmNode *pShmNode, DWORD nMs){
  HANDLE h = pShmNode->hSharedShm;
  ціле rc = SQLITE_OK;

  assert( sqlite3_mutex_held(pShmNode->mutex) );
  rc = winHandleLockTimeout(h, WIN_SHM_DMS, 1, 1, 0);
  якщо( rc==SQLITE_OK ){
    /* We have an EXCLUSIVE lock on the DMS byte. This means that this
    ** is the first process to open the file. Truncate it to zero bytes
    ** in this case.  */
    якщо( pShmNode->isReadonly ){
      rc = SQLITE_READONLY_CANTINIT;
    }інакше{
      якщо( SQLITE_OK!=winHandleTruncate(h, 0) ){
        /* Sometimes the truncate operation may fail because some other
        ** process is still holding an open mapping. So try to write
        ** zeroes into the start of the file instead. */
        rc = winZeroSharedMemory(h);
      }
    }

    /* Release the EXCLUSIVE lock acquired above. */
    winUnlockFile(&h, WIN_SHM_DMS, 0, 1, 0);
  }інакше якщо( (rc & 0xFF)==SQLITE_BUSY ){
    rc = SQLITE_OK;
  }

  якщо( rc==SQLITE_OK ){
    /* Take a SHARED lock on the DMS byte. */
    rc = winHandleLockTimeout(h, WIN_SHM_DMS, 1, 0, nMs);
    якщо( rc==SQLITE_OK ){
      pShmNode->isUnlocked = 0;
    }
  }

  повернути rc;
}


/*
** This function is used to open a handle on a *-shm file.
**
** If SQLITE_ENABLE_SETLK_TIMEOUT is defined at build time, then the file
** is opened with FILE_FLAG_OVERLAPPED specified. If not, it is not.
*/
static ціле winHandleOpen(
  const char *zUtf8,              /* File to open */
  ціле *pbReadonly,                /* IN/OUT: True for readonly handle */
  HANDLE *ph                      /* OUT: New HANDLE for file */
){
  ціле rc = SQLITE_OK;
  LPWSTR zConverted = 0;
  ціле bReadonly = *pbReadonly;
  HANDLE h = INVALID_HANDLE_VALUE;

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  const DWORD flag_overlapped = FILE_FLAG_OVERLAPPED;
#інакше
  const DWORD flag_overlapped = 0;
#endif

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8);
  якщо( zConverted==0 ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8));
    rc = SQLITE_IOERR_NOMEM_BKPT;
    goto winopenfile_out;
  }

  /* Ensure the file we are trying to open is not actually a directory. */
  якщо( winIsDir(zConverted) ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8));
    rc = SQLITE_CANTOPEN_ISDIR;
    goto winopenfile_out;
  }

  /* TODO: platforms.
  ** TODO: retry-on-ioerr.
  */
  h = osCreateFileW(zConverted,         /* lpFileName */
      (GENERIC_READ | (bReadonly ? 0 : GENERIC_WRITE)),  /* dwDesiredAccess */
      FILE_SHARE_READ | FILE_SHARE_WRITE,        /* dwShareMode */
      NULL,                                      /* lpSecurityAttributes */
      OPEN_ALWAYS,                               /* dwCreationDisposition */
      FILE_ATTRIBUTE_NORMAL|flag_overlapped,
      NULL
  );

  якщо( h==INVALID_HANDLE_VALUE ){
    якщо( bReadonly==0 ){
      bReadonly = 1;
      rc = winHandleOpen(zUtf8, &bReadonly, &h);
    }інакше{
      rc = SQLITE_CANTOPEN_BKPT;
    }
  }

 winopenfile_out:
  sqlite3_free(zConverted);
  *pbReadonly = bReadonly;
  *ph = h;
  повернути rc;
}
 
/*
** Close pDbFd's connection to shared-memory.  Delete the underlying
** *-shm file if deleteFlag is true.
*/
static ціле winCloseSharedMemory(winFile *pDbFd, ціле deleteFlag){
  winShm *p;            /* The connection to be closed */
  winShm **pp;          /* Iterator for pShmNode->pWinShmList */
  winShmNode *pShmNode; /* The underlying shared-memory file */

  p = pDbFd->pShm;
  якщо( p==0 ) повернути SQLITE_OK;
  якщо( p->hShm!=INVALID_HANDLE_VALUE ){
    osCloseHandle(p->hShm);
  }

  winShmEnterMutex();
  pShmNode = p->pShmNode;

  /* Remove this connection from the winShmNode.pWinShmList list */
  sqlite3_mutex_enter(pShmNode->mutex);
  для(pp=&pShmNode->pWinShmList; *pp!=p; pp=&(*pp)->pWinShmNext){}
  *pp = p->pWinShmNext;
  sqlite3_mutex_leave(pShmNode->mutex);

  winShmPurge(pDbFd->pVfs, deleteFlag);
  winShmLeaveMutex();

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  повернути SQLITE_OK;
}

/*
** testfixture builds may set this global variable to true via a
** Tcl interface. This forces the VFS to use the locking normally
** only used for UNC paths for all files.
*/
#ifdef SQLITE_TEST
ціле sqlite3_win_test_unc_locking = 0;
#інакше
# define sqlite3_win_test_unc_locking 0
#endif

/*
** Return true if the string passed as the only argument is likely
** to be a UNC path.  Return false if not.
**
** Return true if:
**
**   (1) The name begins with "\\"
**   (2) But does not begin with "\\?\C:\" where C can be any alphabetic
**       character.
**
** For testing, also return true in all cases if the global variable
** sqlite3_win_test_unc_locking is true.
*/
static ціле winIsUNCPath(const char *zFile){
  якщо( zFile[0]=='\\' && zFile[1]=='\\' ){
    якщо( zFile[2]=='?'
     && zFile[3]=='\\'
     && sqlite3Isalpha(zFile[4])
     && zFile[5]==':'
     && winIsDirSep(zFile[6])
    ){
      повернути sqlite3_win_test_unc_locking;
    }інакше{
      повернути 1;
    }
  }
  повернути sqlite3_win_test_unc_locking;
}

/*
** Return true if the string passed in is the name of a file on a
** remote (network-mounted) volume.
*/
static ціле winIsRemoteVolume(const char *zFile){
  WCHAR zRoot[4];
  якщо( strncmp(zFile, "\\\\?\\",4)==0 ) zFile += 4;
  якщо( !sqlite3Isalpha(zFile[0]) ) повернути 0;
  якщо( zFile[1]!=':' ) повернути 0;
  якщо( !winIsDirSep(zFile[2]) ) повернути 0;
  zRoot[0] = (WCHAR)zFile[0];
  zRoot[1] = ':';
  zRoot[2] = '\\';
  zRoot[3] = 0;
  повернути osGetDriveTypeW(zRoot)==DRIVE_REMOTE;
}

/*
** Open the shared-memory area associated with database file pDbFd.
*/
static ціле winOpenSharedMemory(winFile *pDbFd){
  struct winShm *p;                  /* The connection to be opened */
  winShmNode *pShmNode = 0;          /* The underlying mmapped file */
  ціле rc = SQLITE_OK;                /* Result code */
  winShmNode *pNew;                  /* Newly allocated winShmNode */
  ціле nName;                         /* Size of zName in bytes */

  assert( pDbFd->pShm==0 );    /* Not previously opened */

  /* Allocate space for the new sqlite3_shm object.  Also speculatively
  ** allocate space for a new winShmNode and filename.  */
  p = sqlite3MallocZero( sizeof(*p) );
  якщо( p==0 ) повернути SQLITE_IOERR_NOMEM_BKPT;
  nName = sqlite3Strlen30(pDbFd->zPath);
  pNew = sqlite3MallocZero( sizeof(*pShmNode) + (i64)nName + 17 );
  якщо( pNew==0 ){
    sqlite3_free(p);
    повернути SQLITE_IOERR_NOMEM_BKPT;
  }
  pNew->zFilename = (char*)&pNew[1];
  pNew->hSharedShm = INVALID_HANDLE_VALUE;
  pNew->isUnlocked = 1;
  якщо( winIsUNCPath(pDbFd->zPath) || winIsRemoteVolume(pDbFd->zPath) ){
    pNew->bUseSharedLockHandle = 1;
  }інакше{
    pNew->bUseSharedLockHandle = 0;
  }
  sqlite3_snprintf(nName+15, pNew->zFilename, "%s-shm", pDbFd->zPath);
  sqlite3FileSuffix3(pDbFd->zPath, pNew->zFilename);

  /* Look to see if there is an existing winShmNode that can be used.
  ** If no matching winShmNode currently exists, then create a new one.  */
  winShmEnterMutex();
  для(pShmNode = winShmNodeList; pShmNode; pShmNode=pShmNode->pNext){
    /* TBD need to come up with better match here.  Perhaps
    ** use FILE_ID_BOTH_DIR_INFO Structure.  */
    якщо( sqlite3StrICmp(pShmNode->zFilename, pNew->zFilename)==0 ) break;
  }
  якщо( pShmNode==0 ){
    pShmNode = pNew;

    /* Allocate a mutex for this winShmNode object, if one is required. */
    якщо( sqlite3GlobalConfig.bCoreMutex ){
      pShmNode->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      якщо( pShmNode->mutex==0 ) rc = SQLITE_IOERR_NOMEM_BKPT;
    }

    /* Open a file-handle to use for mappings, and for the DMS lock. */
    якщо( rc==SQLITE_OK ){
      HANDLE h = INVALID_HANDLE_VALUE;
      pShmNode->isReadonly = sqlite3_uri_boolean(pDbFd->zPath,"readonly_shm",0);
      rc = winHandleOpen(pNew->zFilename, &pShmNode->isReadonly, &h);
      pShmNode->hSharedShm = h;
    }

    /* If successful, link the new winShmNode into the global list. If an 
    ** error occurred, free the object. */
    якщо( rc==SQLITE_OK ){
      pShmNode->pNext = winShmNodeList;
      winShmNodeList = pShmNode;
      pNew = 0;
    }інакше{
      sqlite3_mutex_free(pShmNode->mutex);
      якщо( pShmNode->hSharedShm!=INVALID_HANDLE_VALUE ){
        osCloseHandle(pShmNode->hSharedShm);
      }
    }
  }

  /* If no error has occurred, link the winShm object to the winShmNode and
  ** the winShm to pDbFd.  */
  якщо( rc==SQLITE_OK ){
    sqlite3_mutex_enter(pShmNode->mutex);
    p->pShmNode = pShmNode;
    p->pWinShmNext = pShmNode->pWinShmList;
    pShmNode->pWinShmList = p;
#якщо defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
    p->id = pShmNode->nextShmId++;
#endif
    pDbFd->pShm = p;
    sqlite3_mutex_leave(pShmNode->mutex);
  }інакше якщо( p ){
    sqlite3_free(p);
  }

  assert( rc!=SQLITE_OK || pShmNode->isUnlocked==0 || pShmNode->nRegion==0 );
  winShmLeaveMutex();
  sqlite3_free(pNew);

  /* Open a file-handle on the *-shm file for this connection. This file-handle
  ** is only used for locking. The mapping of the *-shm file is created using
  ** the shared file handle in winShmNode.hSharedShm.  */
  якщо( rc==SQLITE_OK && pShmNode->bUseSharedLockHandle==0 ){
    p->bReadonly = sqlite3_uri_boolean(pDbFd->zPath, "readonly_shm", 0);
    rc = winHandleOpen(pShmNode->zFilename, &p->bReadonly, &p->hShm);
    якщо( rc!=SQLITE_OK ){
      assert( p->hShm==INVALID_HANDLE_VALUE );
      winCloseSharedMemory(pDbFd, 0);
    }
  }

  повернути rc;
}

/*
** Close a connection to shared-memory.  Delete the underlying
** storage if deleteFlag is true.
*/
static ціле winShmUnmap(
  sqlite3_file *fd,          /* Database holding shared memory */
  ціле deleteFlag             /* Delete after closing if true */
){
  повернути winCloseSharedMemory((winFile*)fd, deleteFlag);
}

/*
** Change the lock state for a shared-memory segment.
*/
static ціле winShmLock(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  ціле ofst,                  /* First lock to acquire or release */
  ціле n,                     /* Number of locks to acquire or release */
  ціле flags                  /* What to do with the lock */
){
  winFile *pDbFd = (winFile*)fd;        /* Connection holding shared memory */
  winShm *p = pDbFd->pShm;              /* The shared memory being locked */
  winShmNode *pShmNode;
  ціле rc = SQLITE_OK;                   /* Result code */
  u16 mask = (u16)((1U<<(ofst+n)) - (1U<<ofst)); /* Mask of locks to [un]take */

  якщо( p==0 ) повернути SQLITE_IOERR_SHMLOCK;
  pShmNode = p->pShmNode;
  якщо( NEVER(pShmNode==0) ) повернути SQLITE_IOERR_SHMLOCK;

  assert( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  assert( n>=1 );
  assert( flags==(SQLITE_SHM_LOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_LOCK | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  assert( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );

  /* Check that, if this to be a blocking lock, no locks that occur later
  ** in the following list than the lock being obtained are already held:
  **
  **   1. Recovery lock (ofst==2).
  **   2. Checkpointer lock (ofst==1).
  **   3. Write lock (ofst==0).
  **   4. Read locks (ofst>=3 && ofst<SQLITE_SHM_NLOCK).
  **
  ** In other words, if this is a blocking lock, none of the locks that
  ** occur later in the above list than the lock being obtained may be
  ** held.
  */
#якщо defined(SQLITE_ENABLE_SETLK_TIMEOUT) && defined(SQLITE_DEBUG)
  {
    u16 lockMask = (p->exclMask|p->sharedMask);
    assert( (flags & SQLITE_SHM_UNLOCK) || pDbFd->iBusyTimeout==0 || (
          (ofst!=2 || lockMask==0)
       && (ofst!=1 || lockMask==0 || lockMask==2)
       && (ofst!=0 || lockMask<3)
       && (ofst<3  || lockMask<(1<<ofst))
    ));
  }
#endif

  /* Check if there is any work to do. There are three cases:
  **
  **    a) An unlock operation where there are locks to unlock,
  **    b) An shared lock where the requested lock is not already held
  **    c) An exclusive lock where the requested lock is not already held
  **
  ** The SQLite core never requests an exclusive lock that it already holds.
  ** This is assert()ed immediately below.  */
  assert( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK) 
       || 0==(p->exclMask & mask)
  );
  якщо( ((flags & SQLITE_SHM_UNLOCK) && ((p->exclMask|p->sharedMask) & mask))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_LOCK) && 0==(p->sharedMask & mask))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK))
  ){
    HANDLE h = p->hShm;

    якщо( flags & SQLITE_SHM_UNLOCK ){
      /* Case (a) - unlock.  */

      assert( (p->exclMask & p->sharedMask)==0 );
      assert( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->exclMask & mask)==mask );
      assert( !(flags & SQLITE_SHM_SHARED) || (p->sharedMask & mask)==mask );

      assert( !(flags & SQLITE_SHM_SHARED) || n==1 );
      якщо( pShmNode->bUseSharedLockHandle ){
        h = pShmNode->hSharedShm;
        якщо( flags & SQLITE_SHM_SHARED ){
          winShm *pShm;
          sqlite3_mutex_enter(pShmNode->mutex);
          для(pShm=pShmNode->pWinShmList; pShm; pShm=pShm->pWinShmNext){
            якщо( pShm!=p && (pShm->sharedMask & mask) ){
              /* Another connection within this process is also holding this
              ** SHARED lock. So do not actually release the OS lock.  */
              h = INVALID_HANDLE_VALUE;
              break;
            }
          }
          sqlite3_mutex_leave(pShmNode->mutex);
        }
      }

      якщо( h!=INVALID_HANDLE_VALUE ){
        rc = winHandleUnlock(h, ofst+WIN_SHM_BASE, n);
      }

      /* If successful, also clear the bits in sharedMask/exclMask */
      якщо( rc==SQLITE_OK ){
        p->exclMask = (p->exclMask & ~mask);
        p->sharedMask = (p->sharedMask & ~mask);
      }
    }інакше{
      ціле bExcl = ((flags & SQLITE_SHM_EXCLUSIVE) ? 1 : 0);
      DWORD nMs = winFileBusyTimeout(pDbFd);

      якщо( pShmNode->bUseSharedLockHandle ){
        winShm *pShm;
        h = pShmNode->hSharedShm;
        sqlite3_mutex_enter(pShmNode->mutex);
        для(pShm=pShmNode->pWinShmList; pShm; pShm=pShm->pWinShmNext){
          якщо( bExcl ){
            якщо( (pShm->sharedMask|pShm->exclMask) & mask ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_VALUE;
            }
          }інакше{
            якщо( pShm->sharedMask & mask ){
              h = INVALID_HANDLE_VALUE;
            }інакше якщо( pShm->exclMask & mask ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_VALUE;
            }
          }
        }
        sqlite3_mutex_leave(pShmNode->mutex);
      }

      якщо( h!=INVALID_HANDLE_VALUE ){
        rc = winHandleLockTimeout(h, ofst+WIN_SHM_BASE, n, bExcl, nMs);
      }
      якщо( rc==SQLITE_OK ){
        якщо( bExcl ){
          p->exclMask = (p->exclMask | mask);
        }інакше{
          p->sharedMask = (p->sharedMask | mask);
        }
      }
    }
  }

  OSTRACE((
      "SHM-LOCK(%d,%d,%d) pid=%lu, id=%d, sharedMask=%03x, exclMask=%03x,"
      " rc=%s\n",
      ofst, n, flags, 
      osGetCurrentProcessId(), p->id, p->sharedMask, p->exclMask,
      sqlite3ErrName(rc))
  );
  повернути rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
static void winShmBarrier(
  sqlite3_file *fd          /* Database holding the shared memory */
){
  UNUSED_PARAMETER(fd);
  sqlite3MemoryBarrier();   /* compiler-defined memory barrier */
  winShmEnterMutex();       /* Also mutex, for redundancy */
  winShmLeaveMutex();
}

/*
** This function is called to obtain a pointer to region iRegion of the
** shared-memory associated with the database file fd. Shared-memory regions
** are numbered starting from zero. Each shared-memory region is szRegion
** bytes in size.
**
** If an error occurs, an error code is returned and *pp is set to NULL.
**
** Otherwise, if the isWrite parameter is 0 and the requested shared-memory
** region has not been allocated (by any client, including one running in a
** separate process), then *pp is set to NULL and SQLITE_OK returned. If
** isWrite is non-zero and the requested shared-memory region has not yet
** been allocated, it is allocated by this function.
**
** If the shared-memory region has already been allocated or is allocated by
** this call as described above, then it is mapped into this processes
** address space (if it is not already), *pp is set to point to the mapped
** memory and SQLITE_OK returned.
*/
static ціле winShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  ціле iRegion,                    /* Region to retrieve */
  ціле szRegion,                   /* Size of regions */
  ціле isWrite,                    /* True to extend file if necessary */
  void volatile **pp              /* OUT: Mapped memory */
){
  winFile *pDbFd = (winFile*)fd;
  winShm *pShm = pDbFd->pShm;
  winShmNode *pShmNode;
  DWORD protect = PAGE_READWRITE;
  DWORD flags = FILE_MAP_WRITE | FILE_MAP_READ;
  ціле rc = SQLITE_OK;

  якщо( !pShm ){
    rc = winOpenSharedMemory(pDbFd);
    якщо( rc!=SQLITE_OK ) повернути rc;
    pShm = pDbFd->pShm;
    assert( pShm!=0 );
  }
  pShmNode = pShm->pShmNode;

  sqlite3_mutex_enter(pShmNode->mutex);
  якщо( pShmNode->isUnlocked ){
    /* Take the DMS lock. */
    assert( pShmNode->nRegion==0 );
    rc = winLockSharedMemory(pShmNode, winFileBusyTimeout(pDbFd));
    якщо( rc!=SQLITE_OK ) goto shmpage_out;
  }

  assert( szRegion==pShmNode->szRegion || pShmNode->nRegion==0 );
  якщо( pShmNode->nRegion<=iRegion ){
    HANDLE hShared = pShmNode->hSharedShm;
    struct ShmRegion *apNew;           /* New aRegion[] array */
    i64 nByte = ((i64)iRegion+1)*(i64)szRegion;  /* Minimum file size */
    sqlite3_int64 sz;                  /* Current size of wal-index file */

    pShmNode->szRegion = szRegion;

    /* The requested region is not mapped into this processes address space.
    ** Check to see if it has been allocated (i.e. if the wal-index file is
    ** large enough to contain the requested region).
    */
    rc = winHandleSize(hShared, &sz);
    якщо( rc!=SQLITE_OK ){
      rc = winLogError(rc, osGetLastError(), "winShmMap1", pDbFd->zPath);
      goto shmpage_out;
    }

    якщо( sz<nByte ){
      /* The requested memory region does not exist. If isWrite is set to
      ** zero, exit early. *pp will be set to NULL and SQLITE_OK returned.
      **
      ** Alternatively, if isWrite is non-zero, use ftruncate() to allocate
      ** the requested memory region.  */
      якщо( !isWrite ) goto shmpage_out;
      rc = winHandleTruncate(hShared, nByte);
      якщо( rc!=SQLITE_OK ){
        rc = winLogError(rc, osGetLastError(), "winShmMap2", pDbFd->zPath);
        goto shmpage_out;
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (struct ShmRegion*)sqlite3_realloc64(
        pShmNode->aRegion, ((i64)iRegion+1)*sizeof(apNew[0])
    );
    якщо( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      goto shmpage_out;
    }
    pShmNode->aRegion = apNew;

    якщо( pShmNode->isReadonly ){
      protect = PAGE_READONLY;
      flags = FILE_MAP_READ;
    }

    поки( pShmNode->nRegion<=iRegion ){
      HANDLE hMap = NULL;         /* file-mapping handle */
      void *pMap = 0;             /* Mapped memory region */

#ifdef SQLITE_UWP
      hMap = osCreateFileMappingFromApp(hShared, NULL, protect, nByte, NULL);
#інакше
      hMap = osCreateFileMappingW(hShared, NULL, protect, 0, nByte, NULL);
#endif
      OSTRACE(("SHM-MAP-CREATE pid=%lu, region=%d, size=%lld, rc=%s\n",
               osGetCurrentProcessId(), pShmNode->nRegion, nByte,
               hMap ? "ok" : "failed"));
      якщо( hMap ){
        i64 iOffset = pShmNode->nRegion*szRegion;
        ціле iOffsetShift = iOffset % winSysInfo.dwAllocationGranularity;
#ifdef SQLITE_UWP
        pMap = osMapViewOfFileFromApp(hMap, flags,
            iOffset - iOffsetShift, (i64)szRegion + iOffsetShift
        );
#інакше
        pMap = osMapViewOfFile(hMap, flags,
            0, iOffset - iOffsetShift, (i64)szRegion + iOffsetShift
        );
#endif
        OSTRACE(("SHM-MAP-MAP pid=%lu, region=%d, offset=%d, size=%d, rc=%s\n",
                 osGetCurrentProcessId(), pShmNode->nRegion, iOffset,
                 szRegion, pMap ? "ok" : "failed"));
      }
      якщо( !pMap ){
        pShmNode->lastErrno = osGetLastError();
        rc = winLogError(SQLITE_IOERR_SHMMAP, pShmNode->lastErrno,
                         "winShmMap3", pDbFd->zPath);
        якщо( hMap ) osCloseHandle(hMap);
        goto shmpage_out;
      }

      pShmNode->aRegion[pShmNode->nRegion].pMap = pMap;
      pShmNode->aRegion[pShmNode->nRegion].hMap = hMap;
      pShmNode->nRegion++;
    }
  }

shmpage_out:
  якщо( pShmNode->nRegion>iRegion ){
    i64 iOffset = (i64)iRegion*(i64)szRegion;
    ціле iOffsetShift = iOffset % winSysInfo.dwAllocationGranularity;
    char *p = (char *)pShmNode->aRegion[iRegion].pMap;
    *pp = (void *)&p[iOffsetShift];
  }інакше{
    *pp = 0;
  }
  якщо( pShmNode->isReadonly && rc==SQLITE_OK ){
    rc = SQLITE_READONLY;
  }
  sqlite3_mutex_leave(pShmNode->mutex);
  повернути rc;
}

#інакше
# define winShmMap     0
# define winShmLock    0
# define winShmBarrier 0
# define winShmUnmap   0
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Cleans up the mapped region of the specified file, if any.
*/
#якщо SQLITE_MAX_MMAP_SIZE>0
static ціле winUnmapfile(winFile *pFile){
  assert( pFile!=0 );
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, pMapRegion=%p, "
           "mmapSize=%lld, mmapSizeMax=%lld\n",
           osGetCurrentProcessId(), pFile, pFile->hMap, pFile->pMapRegion,
           pFile->mmapSize, pFile->mmapSizeMax));
  якщо( pFile->pMapRegion ){
    якщо( !osUnmapViewOfFile(pFile->pMapRegion) ){
      pFile->lastErrno = osGetLastError();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(), pFile,
               pFile->pMapRegion));
      повернути winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winUnmapfile1", pFile->zPath);
    }
    pFile->pMapRegion = 0;
    pFile->mmapSize = 0;
  }
  якщо( pFile->hMap!=NULL ){
    якщо( !osCloseHandle(pFile->hMap) ){
      pFile->lastErrno = osGetLastError();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, rc=SQLITE_IOERR_MMAP\n",
               osGetCurrentProcessId(), pFile, pFile->hMap));
      повернути winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winUnmapfile2", pFile->zPath);
    }
    pFile->hMap = NULL;
  }
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile));
  повернути SQLITE_OK;
}

/*
** Memory map or remap the file opened by file-descriptor pFd (if the file
** is already mapped, the existing mapping is replaced by the new). Or, if
** there already exists a mapping for this file, and there are still
** outstanding xFetch() references to it, this function is a no-op.
**
** If parameter nByte is non-negative, then it is the requested size of
** the mapping to create. Otherwise, if nByte is less than zero, then the
** requested size is the size of the file on disk. The actual size of the
** created mapping is either the requested size or the value configured
** using SQLITE_FCNTL_MMAP_SIZE, whichever is smaller.
**
** SQLITE_OK is returned if no error occurs (even if the mapping is not
** recreated as a result of outstanding references) or an SQLite error
** code otherwise.
*/
static ціле winMapfile(winFile *pFd, sqlite3_int64 nByte){
  sqlite3_int64 nMap = nByte;
  ціле rc;

  assert( nMap>=0 || pFd->nFetchOut==0 );
  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, size=%lld\n",
           osGetCurrentProcessId(), pFd, nByte));

  якщо( pFd->nFetchOut>0 ) повернути SQLITE_OK;

  якщо( nMap<0 ){
    rc = winFileSize((sqlite3_file*)pFd, &nMap);
    якщо( rc ){
      OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_IOERR_FSTAT\n",
               osGetCurrentProcessId(), pFd));
      повернути SQLITE_IOERR_FSTAT;
    }
  }
  якщо( nMap>pFd->mmapSizeMax ){
    nMap = pFd->mmapSizeMax;
  }
  nMap &= ~(sqlite3_int64)(winSysInfo.dwPageSize - 1);

  якщо( nMap==0 && pFd->mmapSize>0 ){
    winUnmapfile(pFd);
  }
  якщо( nMap!=pFd->mmapSize ){
    void *pNew = 0;
    DWORD protect = PAGE_READONLY;
    DWORD flags = FILE_MAP_READ;

    winUnmapfile(pFd);
#ifdef SQLITE_MMAP_READWRITE
    якщо( (pFd->ctrlFlags & WINFILE_RDONLY)==0 ){
      protect = PAGE_READWRITE;
      flags |= FILE_MAP_WRITE;
    }
#endif
#ifdef SQLITE_UWP
    pFd->hMap = osCreateFileMappingFromApp(pFd->h, NULL, protect, nMap, NULL);
#інакше
    pFd->hMap = osCreateFileMappingW(pFd->h, NULL, protect,
                                (DWORD)((nMap>>32) & 0xffffffff),
                                (DWORD)(nMap & 0xffffffff), NULL);
#endif
    якщо( pFd->hMap==NULL ){
      pFd->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->lastErrno,
                       "winMapfile1", pFd->zPath);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-CREATE pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      повернути SQLITE_OK;
    }
    assert( (nMap % winSysInfo.dwPageSize)==0 );
    assert( sizeof(SIZE_T)==sizeof(sqlite3_int64) || nMap<=0xffffffff );
#ifdef SQLITE_UWP
    pNew = osMapViewOfFileFromApp(pFd->hMap, flags, 0, (SIZE_T)nMap);
#інакше
    pNew = osMapViewOfFile(pFd->hMap, flags, 0, 0, (SIZE_T)nMap);
#endif
    якщо( pNew==NULL ){
      osCloseHandle(pFd->hMap);
      pFd->hMap = NULL;
      pFd->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->lastErrno,
                       "winMapfile2", pFd->zPath);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-MAP pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      повернути SQLITE_OK;
    }
    pFd->pMapRegion = pNew;
    pFd->mmapSize = nMap;
  }

  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFd));
  повернути SQLITE_OK;
}
#endif /* SQLITE_MAX_MMAP_SIZE>0 */

/*
** If possible, return a pointer to a mapping of file fd starting at offset
** iOff. The mapping must be valid for at least nAmt bytes.
**
** If such a pointer can be obtained, store it in *pp and return SQLITE_OK.
** Or, if one cannot but no error occurs, set *pp to 0 and return SQLITE_OK.
** Finally, if an error does occur, return an SQLite error code. The final
** value of *pp is undefined in this case.
**
** If this function does return a pointer, the caller must eventually
** release the reference by calling winUnfetch().
*/
static ціле winFetch(sqlite3_file *fd, i64 iOff, ціле nAmt, void **pp){
#якщо SQLITE_MAX_MMAP_SIZE>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */
#endif
  *pp = 0;

  OSTRACE(("FETCH pid=%lu, pFile=%p, offset=%lld, amount=%d, pp=%p\n",
           osGetCurrentProcessId(), fd, iOff, nAmt, pp));

#якщо SQLITE_MAX_MMAP_SIZE>0
  якщо( pFd->mmapSizeMax>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    const ціле nEofBuffer = 256;
    якщо( pFd->pMapRegion==0 ){
      ціле rc = winMapfile(pFd, -1);
      якщо( rc!=SQLITE_OK ){
        OSTRACE(("FETCH pid=%lu, pFile=%p, rc=%s\n",
                 osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
        повернути rc;
      }
    }
    якщо( pFd->mmapSize >= (iOff+nAmt+nEofBuffer) ){
      assert( pFd->pMapRegion!=0 );
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->nFetchOut++;
    }
  }
#endif

  OSTRACE(("FETCH pid=%lu, pFile=%p, pp=%p, *pp=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd, pp, *pp));
  повернути SQLITE_OK;
}

/*
** If the third argument is non-NULL, then this function releases a
** reference obtained by an earlier call to winFetch(). The second
** argument passed to this function must be the same as the corresponding
** argument that was passed to the winFetch() invocation.
**
** Or, if the third argument is NULL, then this function is being called
** to inform the VFS layer that, according to POSIX, any existing mapping
** may now be invalid and should be unmapped.
*/
static ціле winUnfetch(sqlite3_file *fd, i64 iOff, void *p){
#якщо SQLITE_MAX_MMAP_SIZE>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  assert( (p==0)==(pFd->nFetchOut==0) );

  /* If p!=0, it must match the iOff value. */
  assert( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, offset=%lld, p=%p\n",
           osGetCurrentProcessId(), pFd, iOff, p));

  якщо( p ){
    pFd->nFetchOut--;
  }інакше{
    /* FIXME:  If Windows truly always prevents truncating or deleting a
    ** file while a mapping is held, then the following winUnmapfile() call
    ** is unnecessary can be omitted - potentially improving
    ** performance.  */
    winUnmapfile(pFd);
  }

  assert( pFd->nFetchOut>=0 );
#endif

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd));
  повернути SQLITE_OK;
}

/*
** Here ends the implementation of all sqlite3_file methods.
**
********************** End sqlite3_file Methods *******************************
******************************************************************************/

/*
** This vector defines all the methods that can operate on an
** sqlite3_file for win32.
*/
static const sqlite3_io_methods winIoMethod = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  winFileSize,                    /* xFileSize */
  winLock,                        /* xLock */
  winUnlock,                      /* xUnlock */
  winCheckReservedLock,           /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  winSectorSize,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  winShmLock,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

/*
** This vector defines all the methods that can operate on an
** sqlite3_file for win32 without performing any locking.
*/
static const sqlite3_io_methods winIoNolockMethod = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  winFileSize,                    /* xFileSize */
  winNolockLock,                  /* xLock */
  winNolockUnlock,                /* xUnlock */
  winNolockCheckReservedLock,     /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  winSectorSize,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  winShmLock,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

static winVfsAppData winAppData = {
  &winIoMethod,       /* pMethod */
  0,                  /* pAppData */
  0                   /* bNoLock */
};

static winVfsAppData winNolockAppData = {
  &winIoNolockMethod, /* pMethod */
  0,                  /* pAppData */
  1                   /* bNoLock */
};

/****************************************************************************
**************************** sqlite3_vfs methods ****************************
**
** This division contains the implementation of methods on the
** sqlite3_vfs object.
*/

/*
** This function returns non-zero if the specified UTF-8 string buffer
** ends with a directory separator character or one was successfully
** added to it.
*/
static ціле winMakeEndInDirSep(ціле nBuf, char *zBuf){
  якщо( zBuf ){
    ціле nLen = sqlite3Strlen30(zBuf);
    якщо( nLen>0 ){
      якщо( winIsDirSep(zBuf[nLen-1]) ){
        повернути 1;
      }інакше якщо( nLen+1<nBuf ){
        якщо( !osGetenv ){
          zBuf[nLen] = winGetDirSep();
        }інакше якщо( winIsDriveLetterAndColon(zBuf) && winIsDirSep(zBuf[2]) ){
          zBuf[nLen] = '\\';
          zBuf[2]='\\';
        }інакше{
          zBuf[nLen] = '/';
        }
        zBuf[nLen+1] = '\0';
        повернути 1;
      }
    }
  }
  повернути 0;
}

/*
** If sqlite3_temp_directory is defined, take the mutex and return true.
**
** If sqlite3_temp_directory is NULL (undefined), omit the mutex and
** return false.
*/
static ціле winTempDirDefined(void){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  якщо( sqlite3_temp_directory!=0 ) повернути 1;
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  повернути 0;
}

/*
** Create a temporary file name and store the resulting pointer into pzBuf.
** The pointer returned in pzBuf must be freed via sqlite3_free().
*/
static ціле winGetTempname(sqlite3_vfs *pVfs, char **pzBuf){
  static const char zChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  size_t i, j;
  DWORD pid;
  ціле nPre = sqlite3Strlen30(SQLITE_TEMP_FILE_PREFIX);
  i64 nMax, nBuf, nDir, nLen;
  char *zBuf;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  SimulateIOError( повернути SQLITE_IOERR );

  /* Allocate a temporary buffer to store the fully qualified file
  ** name for the temporary file.  If this fails, we cannot continue.
  */
  nMax = pVfs->mxPathname;
  nBuf = 2 + (i64)nMax;
  zBuf = sqlite3MallocZero( nBuf );
  якщо( !zBuf ){
    OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
    повернути SQLITE_IOERR_NOMEM_BKPT;
  }

  /* Figure out the effective temporary directory.  First, check if one
  ** has been explicitly set by the application; otherwise, use the one
  ** configured by the operating system.
  */
  nDir = nMax - (nPre + 15);
  assert( nDir>0 );
  якщо( winTempDirDefined() ){
    ціле nDirLen = sqlite3Strlen30(sqlite3_temp_directory);
    якщо( nDirLen>0 ){
      якщо( !winIsDirSep(sqlite3_temp_directory[nDirLen-1]) ){
        nDirLen++;
      }
      якщо( nDirLen>nDir ){
        sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
        sqlite3_free(zBuf);
        OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
        повернути winLogError(SQLITE_ERROR, 0, "winGetTempname1", 0);
      }
      sqlite3_snprintf(nMax, zBuf, "%s", sqlite3_temp_directory);
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  }

#якщо defined(__CYGWIN__)
  інакше якщо( osGetenv!=NULL ){
    static const char *azDirs[] = {
       0, /* getenv("SQLITE_TMPDIR") */
       0, /* getenv("TMPDIR") */
       0, /* getenv("TMP") */
       0, /* getenv("TEMP") */
       0, /* getenv("USERPROFILE") */
       "/var/tmp",
       "/usr/tmp",
       "/tmp",
       ".",
       0        /* List terminator */
    };
    unsigned ціле i;
    const char *zDir = 0;

    якщо( !azDirs[0] ) azDirs[0] = osGetenv("SQLITE_TMPDIR");
    якщо( !azDirs[1] ) azDirs[1] = osGetenv("TMPDIR");
    якщо( !azDirs[2] ) azDirs[2] = osGetenv("TMP");
    якщо( !azDirs[3] ) azDirs[3] = osGetenv("TEMP");
    якщо( !azDirs[4] ) azDirs[4] = osGetenv("USERPROFILE");
    для(i=0; i<sizeof(azDirs)/sizeof(azDirs[0]); zDir=azDirs[i++]){
      LPWSTR zConverted;
      якщо( zDir==0 ) continue;
      /* If the path starts with a drive letter followed by the colon
      ** character, assume it is already a native Win32 path; otherwise,
      ** it must be converted to a native Win32 path via the Cygwin API
      ** prior to using it.
      */
      {
        zConverted = winConvertFromUtf8Filename(zDir);
        якщо( !zConverted ){
          sqlite3_free(zBuf);
          OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
          повернути SQLITE_IOERR_NOMEM_BKPT;
        }
        якщо( winIsDir(zConverted) ){
          sqlite3_snprintf(nMax, zBuf, "%s", zDir);
          sqlite3_free(zConverted);
          break;
        }
        sqlite3_free(zConverted);
      }
    }
  }
#endif

  інакше{
    char *zMulti;
    LPWSTR zWidePath = sqlite3MallocZero( nMax*sizeof(WCHAR) );
    якщо( !zWidePath ){
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      повернути SQLITE_IOERR_NOMEM_BKPT;
    }
    якщо( osGetTempPathW==0 || osGetTempPathW(nMax, zWidePath)==0 ){
      sqlite3_free(zWidePath);
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_GETTEMPPATH\n"));
      повернути winLogError(SQLITE_IOERR_GETTEMPPATH, osGetLastError(),
                         "winGetTempname2", 0);
    }
    zMulti = winUnicodeToUtf8(zWidePath);
    sqlite3_free(zWidePath);
    якщо( zMulti ){
      sqlite3_snprintf(nMax, zBuf, "%s", zMulti);
      sqlite3_free(zMulti);
    }інакше{
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      повернути SQLITE_IOERR_NOMEM_BKPT;
    }
  }

  /*
  ** Check to make sure the temporary directory ends with an appropriate
  ** separator.  If it does not and there is not enough space left to add
  ** one, fail.
  */
  якщо( !winMakeEndInDirSep(nDir+1, zBuf) ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    повернути winLogError(SQLITE_ERROR, 0, "winGetTempname4", 0);
  }

  /*
  ** Check that the output buffer is large enough for the temporary file
  ** name in the following format:
  **
  **   "<temporary_directory>/etilqs_XXXXXXXXXXXXXXX\0\0"
  **
  ** If not, return SQLITE_ERROR.  The number 17 is used here in order to
  ** account for the space used by the 15 character random suffix and the
  ** two trailing NUL characters.  The final directory separator character
  ** has already added if it was not already present.
  */
  nLen = sqlite3Strlen30(zBuf);
  якщо( (nLen + nPre + 17) > nBuf ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    повернути winLogError(SQLITE_ERROR, 0, "winGetTempname5", 0);
  }

  sqlite3_snprintf(nBuf-16-nLen, zBuf+nLen, SQLITE_TEMP_FILE_PREFIX);

  j = sqlite3Strlen30(zBuf);
  sqlite3_randomness(15, &zBuf[j]);
  pid = osGetCurrentProcessId();
  для(i=0; i<15; i++, j++){
    zBuf[j] += pid & 0xff;
    pid >>= 8;
    zBuf[j] = (char)zChars[ ((unsigned char)zBuf[j])%(sizeof(zChars)-1) ];
  }
  zBuf[j] = 0;
  zBuf[j+1] = 0;
  *pzBuf = zBuf;

  OSTRACE(("TEMP-FILENAME name=%s, rc=SQLITE_OK\n", zBuf));
  повернути SQLITE_OK;
}

/*
** Return TRUE if the named file is really a directory.  Return false if
** it is something other than a directory, or if there is any kind of memory
** allocation failure.
*/
static ціле winIsDir(LPCWSTR zConverted){
  DWORD attr;
  ціле rc = 0;
  DWORD lastErrno;
  ціле cnt = 0;
  WIN32_FILE_ATTRIBUTE_DATA sAttrData;
  memset(&sAttrData, 0, sizeof(sAttrData));
  поки( !(rc = osGetFileAttributesExW(zConverted,
                           GetFileExInfoStandard,
                           &sAttrData)) && winRetryIoerr(&cnt, &lastErrno) ){}
  якщо( !rc ){
    повернути 0; /* Invalid name? */
  }
  attr = sAttrData.dwFileAttributes;
  повернути (attr!=INVALID_FILE_ATTRIBUTES) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

/* forward reference */
static ціле winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  const char *zFilename,     /* Name of file to check */
  ціле flags,                 /* Type of test to make on this file */
  ціле *pResOut               /* OUT: Result */
);

/*
** The Windows version of xAccess() accepts an extra bit in the flags
** parameter that prevents an anti-virus retry loop.
*/
#define NORETRY 0x4000

/*
** Open a file.
*/
static ціле winOpen(
  sqlite3_vfs *pVfs,        /* Used to get maximum path length and AppData */
  const char *zName,        /* Name of the file (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  ціле flags,                /* Open mode flags */
  ціле *pOutFlags            /* Status return flags */
){
  HANDLE h;
  DWORD lastErrno = 0;
  DWORD dwDesiredAccess;
  DWORD dwShareMode;
  DWORD dwCreationDisposition;
  DWORD dwFlagsAndAttributes = 0;
  winVfsAppData *pAppData;
  winFile *pFile = (winFile*)id;
  LPWSTR zConverted;              /* Filename in OS encoding */
  const char *zUtf8Name = zName; /* Filename in UTF-8 encoding */
  ціле cnt = 0;
  ціле isRO = 0;              /* file is known to be accessible readonly */

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  char *zTmpname = 0; /* For temporary filename, if necessary. */

  ціле rc = SQLITE_OK;            /* Function Return Code */
#якщо !defined(NDEBUG)
  ціле eType = flags&0x0FFF00;  /* Type of file to open */
#endif

  ціле isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  ціле isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  ціле isCreate     = (flags & SQLITE_OPEN_CREATE);
  ціле isReadonly   = (flags & SQLITE_OPEN_READONLY);
  ціле isReadWrite  = (flags & SQLITE_OPEN_READWRITE);

#ifndef NDEBUG
  ціле isOpenJournal = (isCreate && (
        eType==SQLITE_OPEN_SUPER_JOURNAL
     || eType==SQLITE_OPEN_MAIN_JOURNAL
     || eType==SQLITE_OPEN_WAL
  ));
#endif

  OSTRACE(("OPEN name=%s, pFile=%p, flags=%x, pOutFlags=%p\n",
           zUtf8Name, id, flags, pOutFlags));

  /* Check the following statements are true:
  **
  **   (a) Exactly one of the READWRITE and READONLY flags must be set, and
  **   (b) if CREATE is set, then READWRITE must also be set, and
  **   (c) if EXCLUSIVE is set, then CREATE must also be set.
  **   (d) if DELETEONCLOSE is set, then CREATE must also be set.
  */
  assert((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
  assert(isCreate==0 || isReadWrite);
  assert(isExclusive==0 || isCreate);
  assert(isDelete==0 || isCreate);

  /* The main DB, main journal, WAL file and super-journal are never
  ** automatically deleted. Nor are they ever temporary files.  */
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_DB );
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_JOURNAL );
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_SUPER_JOURNAL );
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_WAL );

  /* Assert that the upper layer has set one of the "file-type" flags. */
  assert( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_TEMP_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_TEMP_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );

  assert( pFile!=0 );
  memset(pFile, 0, sizeof(winFile));
  pFile->h = INVALID_HANDLE_VALUE;

  /* If the second argument to this function is NULL, generate a
  ** temporary file name to use
  */
  якщо( !zUtf8Name ){
    assert( isDelete && !isOpenJournal );
    rc = winGetTempname(pVfs, &zTmpname);
    якщо( rc!=SQLITE_OK ){
      OSTRACE(("OPEN name=%s, rc=%s", zUtf8Name, sqlite3ErrName(rc)));
      повернути rc;
    }
    zUtf8Name = zTmpname;
  }

  /* Database filenames are double-zero terminated if they are not
  ** URIs with parameters.  Hence, they can always be passed into
  ** sqlite3_uri_parameter().
  */
  assert( (eType!=SQLITE_OPEN_MAIN_DB) || (flags & SQLITE_OPEN_URI) ||
       zUtf8Name[sqlite3Strlen30(zUtf8Name)+1]==0 );

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8Name);
  якщо( zConverted==0 ){
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8Name));
    повернути SQLITE_IOERR_NOMEM_BKPT;
  }

  якщо( winIsDir(zConverted) ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8Name));
    повернути SQLITE_CANTOPEN_ISDIR;
  }

  якщо( isReadWrite ){
    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
  }інакше{
    dwDesiredAccess = GENERIC_READ;
  }

  /* SQLITE_OPEN_EXCLUSIVE is used to make sure that a new file is
  ** created. SQLite doesn't use it to indicate "exclusive access"
  ** as it is usually understood.
  */
  якщо( isExclusive ){
    /* Creates a new file, only if it does not already exist. */
    /* If the file exists, it fails. */
    dwCreationDisposition = CREATE_NEW;
  }інакше якщо( isCreate ){
    /* Open existing file, or create if it doesn't exist */
    dwCreationDisposition = OPEN_ALWAYS;
  }інакше{
    /* Opens a file, only if it exists. */
    dwCreationDisposition = OPEN_EXISTING;
  }

  якщо( 0==sqlite3_uri_boolean(zName, "exclusive", 0) ){
    dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  }інакше{
    dwShareMode = 0;
  }

  якщо( isDelete ){
    dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY
                               | FILE_ATTRIBUTE_HIDDEN
                               | FILE_FLAG_DELETE_ON_CLOSE;
  }інакше{
    dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
  }
  do{
    h = osCreateFileW(zConverted,
                      dwDesiredAccess,
                      dwShareMode, NULL,
                      dwCreationDisposition,
                      dwFlagsAndAttributes,
                      NULL);
    якщо( h!=INVALID_HANDLE_VALUE ) break;
    якщо( isReadWrite ){
      ціле rc2;
      sqlite3BeginBenignMalloc();
      rc2 = winAccess(pVfs, zUtf8Name, SQLITE_ACCESS_READ|NORETRY, &isRO);
      sqlite3EndBenignMalloc();
      якщо( rc2==SQLITE_OK && isRO ) break;
    }
  }поки( winRetryIoerr(&cnt, &lastErrno) );
  winLogIoerr(cnt, __LINE__);

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, rc=%s\n", h, zUtf8Name,
           dwDesiredAccess, (h==INVALID_HANDLE_VALUE) ? "failed" : "ok"));

  якщо( h==INVALID_HANDLE_VALUE ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    якщо( isReadWrite && isRO && !isExclusive ){
      повернути winOpen(pVfs, zName, id,
         ((flags|SQLITE_OPEN_READONLY) &
                     ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE)),
         pOutFlags);
    }інакше{
      pFile->lastErrno = lastErrno;
      winLogError(SQLITE_CANTOPEN, pFile->lastErrno, "winOpen", zUtf8Name);
      повернути SQLITE_CANTOPEN_BKPT;
    }
  }

  якщо( pOutFlags ){
    якщо( isReadWrite ){
      *pOutFlags = SQLITE_OPEN_READWRITE;
    }інакше{
      *pOutFlags = SQLITE_OPEN_READONLY;
    }
  }

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, pOutFlags=%p, *pOutFlags=%d, "
           "rc=%s\n", h, zUtf8Name, dwDesiredAccess, pOutFlags, pOutFlags ?
           *pOutFlags : 0, (h==INVALID_HANDLE_VALUE) ? "failed" : "ok"));

  pAppData = (winVfsAppData*)pVfs->pAppData;

  sqlite3_free(zConverted);
  sqlite3_free(zTmpname);
  id->pMethods = pAppData ? pAppData->pMethod : &winIoMethod;
  pFile->pVfs = pVfs;
  pFile->h = h;
  якщо( isReadonly ){
    pFile->ctrlFlags |= WINFILE_RDONLY;
  }
  якщо( (flags & SQLITE_OPEN_MAIN_DB)
   && sqlite3_uri_boolean(zName, "psow", SQLITE_POWERSAFE_OVERWRITE) 
  ){
    pFile->ctrlFlags |= WINFILE_PSOW;
  }
  pFile->lastErrno = NO_ERROR;
  pFile->zPath = zName;
#якщо SQLITE_MAX_MMAP_SIZE>0
  pFile->hMap = NULL;
  pFile->pMapRegion = 0;
  pFile->mmapSize = 0;
  pFile->mmapSizeMax = sqlite3GlobalConfig.szMmap;
#endif

  OpenCounter(+1);
  повернути rc;
}

/*
** Delete the named file.
**
** Note that Windows does not allow a file to be deleted if some other
** process has it open.  Sometimes a virus scanner or indexing program
** will open a journal file shortly after it is created in order to do
** whatever it does.  While this other process is holding the
** file open, we will be unable to delete it.  To work around this
** problem, we delay 100 milliseconds and try to delete again.  Up
** to MX_DELETION_ATTEMPTs deletion attempts are run before giving
** up and returning an error.
*/
static ціле winDelete(
  sqlite3_vfs *pVfs,          /* Not used on win32 */
  const char *zFilename,      /* Name of file to delete */
  ціле syncDir                 /* Not used on win32 */
){
  ціле cnt = 0;
  ціле rc;
  DWORD attr;
  DWORD lastErrno = 0;
  LPWSTR zConverted;
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(syncDir);

  SimulateIOError(повернути SQLITE_IOERR_DELETE);
  OSTRACE(("DELETE name=%s, syncDir=%d\n", zFilename, syncDir));

  zConverted = winConvertFromUtf8Filename(zFilename);
  якщо( zConverted==0 ){
    OSTRACE(("DELETE name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    повернути SQLITE_IOERR_NOMEM_BKPT;
  }
  do {
    attr = osGetFileAttributesW(zConverted);
    якщо ( attr==INVALID_FILE_ATTRIBUTES ){
      lastErrno = osGetLastError();
      якщо( lastErrno==ERROR_FILE_NOT_FOUND
       || lastErrno==ERROR_PATH_NOT_FOUND ){
        rc = SQLITE_IOERR_DELETE_NOENT; /* Already gone? */
      }інакше{
        rc = SQLITE_ERROR;
      }
      break;
    }
    якщо ( attr&FILE_ATTRIBUTE_DIRECTORY ){
      rc = SQLITE_ERROR; /* Files only. */
      break;
    }
    якщо ( osDeleteFileW(zConverted) ){
      rc = SQLITE_OK; /* Deleted OK. */
      break;
    }
    якщо ( !winRetryIoerr(&cnt, &lastErrno) ){
      rc = SQLITE_ERROR; /* No more retries. */
      break;
    }
  } поки(1);
  якщо( rc && rc!=SQLITE_IOERR_DELETE_NOENT ){
    rc = winLogError(SQLITE_IOERR_DELETE, lastErrno, "winDelete", zFilename);
  }інакше{
    winLogIoerr(cnt, __LINE__);
  }
  sqlite3_free(zConverted);
  OSTRACE(("DELETE name=%s, rc=%s\n", zFilename, sqlite3ErrName(rc)));
  повернути rc;
}

/*
** Check the existence and status of a file.
*/
static ціле winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  const char *zFilename,     /* Name of file to check */
  ціле flags,                 /* Type of test to make on this file */
  ціле *pResOut               /* OUT: Result */
){
  DWORD attr;
  ціле rc = 0;
  ціле cnt = 0;
  DWORD lastErrno = 0;
  LPWSTR zConverted;
  ціле noRetry = 0;           /* Do not use winRetryIoerr() */
  WIN32_FILE_ATTRIBUTE_DATA sAttrData;
  UNUSED_PARAMETER(pVfs);

  якщо( (flags & NORETRY)!=0 ){
    noRetry = 1;
    flags &= ~NORETRY;
  }  

  SimulateIOError( повернути SQLITE_IOERR_ACCESS; );
  OSTRACE(("ACCESS name=%s, flags=%x, pResOut=%p\n",
           zFilename, flags, pResOut));

  якщо( zFilename==0 ){
    *pResOut = 0;
    OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
             zFilename, pResOut, *pResOut));
    повернути SQLITE_OK;
  }

  zConverted = winConvertFromUtf8Filename(zFilename);
  якщо( zConverted==0 ){
    OSTRACE(("ACCESS name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    повернути SQLITE_IOERR_NOMEM_BKPT;
  }
  memset(&sAttrData, 0, sizeof(sAttrData));
  поки( !(rc = osGetFileAttributesExW(zConverted,
                           GetFileExInfoStandard,
                           &sAttrData))
     && !noRetry
     && winRetryIoerr(&cnt, &lastErrno)
  ){ /* Loop until true */}
  якщо( rc ){
    /* For an SQLITE_ACCESS_EXISTS query, treat a zero-length file
    ** as if it does not exist.
    */
    якщо(    flags==SQLITE_ACCESS_EXISTS
        && sAttrData.nFileSizeHigh==0
        && sAttrData.nFileSizeLow==0 ){
      attr = INVALID_FILE_ATTRIBUTES;
    }інакше{
      attr = sAttrData.dwFileAttributes;
    }
  }інакше{
    якщо( noRetry ) lastErrno = osGetLastError();
    winLogIoerr(cnt, __LINE__);
    якщо( lastErrno!=ERROR_FILE_NOT_FOUND && lastErrno!=ERROR_PATH_NOT_FOUND ){
      sqlite3_free(zConverted);
      повернути winLogError(SQLITE_IOERR_ACCESS, lastErrno, "winAccess",
                         zFilename);
    }інакше{
      attr = INVALID_FILE_ATTRIBUTES;
    }
  }
  sqlite3_free(zConverted);
  switch( flags ){
    case SQLITE_ACCESS_READ:
    case SQLITE_ACCESS_EXISTS:
      rc = attr!=INVALID_FILE_ATTRIBUTES;
      break;
    case SQLITE_ACCESS_READWRITE:
      rc = attr!=INVALID_FILE_ATTRIBUTES &&
             (attr & FILE_ATTRIBUTE_READONLY)==0;
      break;
    default:
      assert(!"Invalid flags argument");
  }
  *pResOut = rc;
  OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           zFilename, pResOut, *pResOut));
  повернути SQLITE_OK;
}

/*
** Returns non-zero if the specified path name starts with the "long path"
** prefix.
*/
static BOOL winIsLongPathPrefix(
  const char *zPathname
){
  повернути ( zPathname[0]=='\\' && zPathname[1]=='\\'
        && zPathname[2]=='?'  && zPathname[3]=='\\' );
}

/*
** Returns non-zero if the specified path name starts with a drive letter
** followed by a colon character.
*/
static BOOL winIsDriveLetterAndColon(
  const char *zPathname
){
  повернути ( sqlite3Isalpha(zPathname[0]) && zPathname[1]==':' );
}

#ifdef _WIN32
/*
** Returns non-zero if the specified path name should be used verbatim.  If
** non-zero is returned from this function, the calling function must simply
** use the provided path name verbatim -OR- resolve it into a full path name
** using the GetFullPathName Win32 API function (if available).
*/
static BOOL winIsVerbatimPathname(
  const char *zPathname
){
  /*
  ** If the path name starts with a forward slash or a backslash, it is either
  ** a legal UNC name, a volume relative path, or an absolute path name in the
  ** "Unix" format on Windows.  There is no easy way to differentiate between
  ** the final two cases; therefore, we return the safer return value of TRUE
  ** so that callers of this function will simply use it verbatim.
  */
  якщо ( winIsDirSep(zPathname[0]) ){
    повернути TRUE;
  }

  /*
  ** If the path name starts with a letter and a colon it is either a volume
  ** relative path or an absolute path.  Callers of this function must not
  ** attempt to treat it as a relative path name (i.e. they should simply use
  ** it verbatim).
  */
  якщо ( winIsDriveLetterAndColon(zPathname) ){
    повернути TRUE;
  }

  /*
  ** If we get to this point, the path name should almost certainly be a purely
  ** relative one (i.e. not a UNC name, not absolute, and not volume relative).
  */
  повернути FALSE;
}
#endif /* _WIN32 */

#ifdef __CYGWIN__
/*
** Simplify a filename into its canonical form
** by making the following changes:
**
**  * convert any '/' to '\' (win32) or reverse (Cygwin)
**  * removing any trailing and duplicate / (except for UNC paths)
**  * convert /./ into just /
**
** Changes are made in-place.  Return the new name length.
**
** The original filename is in z[0..]. If the path is shortened,
** no-longer used bytes will be written by '\0'.
*/
static void winSimplifyName(char *z){
  ціле i, j;
  для(i=j=0; z[i]; ++i){
    якщо( winIsDirSep(z[i]) ){
#якщо !defined(SQLITE_TEST)
      /* Some test-cases assume that "./foo" and "foo" are different */
      якщо( z[i+1]=='.' && winIsDirSep(z[i+2]) ){
        ++i;
        continue;
      }
#endif
      якщо( !z[i+1] || (winIsDirSep(z[i+1]) && (i!=0)) ){
        continue;
      }
      z[j++] = osGetenv?'/':'\\';
    }інакше{
      z[j++] = z[i];
    }
  }
  поки(j<i) z[j++] = '\0';
}

#define SQLITE_MAX_SYMLINKS 100

static ціле mkFullPathname(
  const char *zPath,              /* Input path */
  char *zOut,                     /* Output buffer */
  ціле nOut                        /* Allocated size of buffer zOut */
){
  ціле nPath = sqlite3Strlen30(zPath);
  ціле iOff = 0;
  якщо( zPath[0]!='/' ){
    якщо( osGetcwd(zOut, nOut-2)==0 ){
      повернути winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno, "getcwd", zPath);
    }
    iOff = sqlite3Strlen30(zOut);
    zOut[iOff++] = '/';
  }
  якщо( (iOff+nPath+1)>nOut ){
    /* SQLite assumes that xFullPathname() nul-terminates the output buffer
    ** even if it returns an error.  */
    zOut[iOff] = '\0';
    повернути SQLITE_CANTOPEN_BKPT;
  }
  sqlite3_snprintf(nOut-iOff, &zOut[iOff], "%s", zPath);
  повернути SQLITE_OK;
}
#endif /* __CYGWIN__ */

/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
static ціле winFullPathnameNoMutex(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  const char *zRelative,        /* Possibly relative input path */
  ціле nFull,                    /* Size of output buffer in bytes */
  char *zFull                   /* Output buffer */
){
  ціле nByte;
  LPWSTR zConverted;
  char *zOut;

  /* If this path name begins with "/X:" or "\\?\", where "X" is any
  ** alphabetic character, discard the initial "/" from the pathname.
  */
  якщо( zRelative[0]=='/' && (winIsDriveLetterAndColon(zRelative+1)
       || winIsLongPathPrefix(zRelative+1)) ){
    zRelative++;
  }

  SimulateIOError( повернути SQLITE_ERROR );

#ifdef __CYGWIN__
  якщо( osGetcwd ){
    zFull[nFull-1] = '\0';
    якщо( !winIsDriveLetterAndColon(zRelative) || !winIsDirSep(zRelative[2]) ){
      ціле rc = SQLITE_OK;
      ціле nLink = 1;              /* Number of symbolic links followed so far */
      const char *zIn = zRelative; /* Input path for each iteration of loop */
      char *zDel = 0;
      struct stat buf;

      UNUSED_PARAMETER(pVfs);

      do {
        /* Call lstat() on path zIn. Set bLink to true if the path
        ** is a symbolic link, or false otherwise.  */
        ціле bLink = 0;
        якщо( osLstat && osReadlink ) {
          якщо( osLstat(zIn, &buf)!=0 ){
            ціле myErrno = osErrno;
            якщо( myErrno!=ENOENT ){
              rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)myErrno,
                               "lstat", zIn);
            }
          }інакше{
            bLink = ((buf.st_mode & 0170000) == 0120000);
          }

          якщо( bLink ){
            якщо( zDel==0 ){
              zDel = sqlite3MallocZero(nFull);
              якщо( zDel==0 ) rc = SQLITE_NOMEM;
            }інакше якщо( ++nLink>SQLITE_MAX_SYMLINKS ){
              rc = SQLITE_CANTOPEN_BKPT;
            }

            якщо( rc==SQLITE_OK ){
              nByte = osReadlink(zIn, zDel, nFull-1);
              якщо( nByte ==(DWORD)-1 ){
                rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno,
                                 "readlink", zIn);
              }інакше{
                якщо( zDel[0]!='/' ){
                  ціле n;
                  для(n = sqlite3Strlen30(zIn); n>0 && zIn[n-1]!='/'; n--);
                  якщо( nByte+n+1>nFull ){
                    rc = SQLITE_CANTOPEN_BKPT;
                  }інакше{
                    memmove(&zDel[n], zDel, nByte+1);
                    memcpy(zDel, zIn, n);
                    nByte += n;
                  }
                }
                zDel[nByte] = '\0';
              }
            }

            zIn = zDel;
          }
        }

        assert( rc!=SQLITE_OK || zIn!=zFull || zIn[0]=='/' );
        якщо( rc==SQLITE_OK && zIn!=zFull ){
          rc = mkFullPathname(zIn, zFull, nFull);
        }
        якщо( bLink==0 ) break;
        zIn = zFull;
      }поки( rc==SQLITE_OK );

      sqlite3_free(zDel);
      winSimplifyName(zFull);
      повернути rc;
    }
  }
#endif /* __CYGWIN__ */

#якщо defined(_WIN32)
  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing. This function could fail if, for example, the
  ** current working directory has been unlinked.
  */
  SimulateIOError( повернути SQLITE_ERROR );
  якщо ( sqlite3_data_directory && !winIsVerbatimPathname(zRelative) ){
    /*
    ** NOTE: We are dealing with a relative path name and the data
    **       directory has been set.  Therefore, use it as the basis
    **       for converting the relative path name to an absolute
    **       one by prepending the data directory and a backslash.
    */
    sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s%c%s",
                     sqlite3_data_directory, winGetDirSep(), zRelative);
    повернути SQLITE_OK;
  }
#endif
  zConverted = winConvertFromUtf8Filename(zRelative);
  якщо( zConverted==0 ){
    повернути SQLITE_IOERR_NOMEM_BKPT;
  }
  {
    LPWSTR zTemp;
    nByte = osGetFullPathNameW(zConverted, 0, 0, 0);
    якщо( nByte==0 ){
      sqlite3_free(zConverted);
      повернути winLogError(SQLITE_CANTOPEN_FULLPATH, osGetLastError(),
                         "winFullPathname1", zRelative);
    }
    nByte += 3;
    zTemp = sqlite3MallocZero( (u64)nByte*sizeof(WCHAR) );
    якщо( zTemp==0 ){
      sqlite3_free(zConverted);
      повернути SQLITE_IOERR_NOMEM_BKPT;
    }
    nByte = osGetFullPathNameW(zConverted, nByte, zTemp, 0);
    якщо( nByte==0 ){
      sqlite3_free(zConverted);
      sqlite3_free(zTemp);
      повернути winLogError(SQLITE_CANTOPEN_FULLPATH, osGetLastError(),
                         "winFullPathname2", zRelative);
    }
    sqlite3_free(zConverted);
    zOut = winUnicodeToUtf8(zTemp);
    sqlite3_free(zTemp);
  }
  якщо( zOut ){
#ifdef __CYGWIN__
    якщо( memcmp(zOut, "\\\\?\\", 4) ){
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut);
    }інакше якщо( memcmp(zOut+4, "UNC\\", 4) ){
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut+4);
    }інакше{
      char *p = zOut+6;
      *p = '\\';
      якщо( osGetcwd ){
        /* On Cygwin, UNC paths use forward slashes */
        поки( *p ){
          якщо( *p=='\\' ) *p = '/';
          ++p;
        }
      }
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut+6);
    }
#інакше
    sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut);
#endif /* __CYGWIN__ */
    sqlite3_free(zOut);
    повернути SQLITE_OK;
  }інакше{
    повернути SQLITE_IOERR_NOMEM_BKPT;
  }
}
static ціле winFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  const char *zRelative,        /* Possibly relative input path */
  ціле nFull,                    /* Size of output buffer in bytes */
  char *zFull                   /* Output buffer */
){
  ціле rc;
  MUTEX_LOGIC( sqlite3_mutex *pMutex; )
  MUTEX_LOGIC( pMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR); )
  sqlite3_mutex_enter(pMutex);
  rc = winFullPathnameNoMutex(pVfs, zRelative, nFull, zFull);
  sqlite3_mutex_leave(pMutex);
  повернути rc;
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
static void *winDlOpen(sqlite3_vfs *pVfs, const char *zFilename){
  HANDLE h = 0;
  LPWSTR zConverted = winConvertFromUtf8Filename(zFilename);
  UNUSED_PARAMETER(pVfs);
  якщо( zConverted==0 ){
    OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (void*)0));
    повернути 0;
  }
  h = osLoadLibraryW ? osLoadLibraryW(zConverted) : 0;
  OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (void*)h));
  sqlite3_free(zConverted);
  повернути (void*)h;
}
static void winDlError(sqlite3_vfs *pVfs, ціле nBuf, char *zBufOut){
  UNUSED_PARAMETER(pVfs);
  winGetLastErrorMsg(osGetLastError(), nBuf, zBufOut);
}
static void (*winDlSym(sqlite3_vfs *pVfs,void *pH,const char *zSym))(void){
  FARPROC proc = 0;
  UNUSED_PARAMETER(pVfs);
  proc = osGetProcAddressA ? osGetProcAddressA((HANDLE)pH, zSym) : 0;
  OSTRACE(("DLSYM handle=%p, symbol=%s, address=%p\n",
           (void*)pH, zSym, (void*)proc));
  повернути (void(*)(void))proc;
}
static void winDlClose(sqlite3_vfs *pVfs, void *pHandle){
  UNUSED_PARAMETER(pVfs);
  якщо( osFreeLibrary!=0 ) osFreeLibrary((HANDLE)pHandle);
  OSTRACE(("DLCLOSE handle=%p\n", (void*)pHandle));
}
#інакше /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #define winDlOpen  0
  #define winDlError 0
  #define winDlSym   0
  #define winDlClose 0
#endif

/* State information for the randomness gatherer. */
typedef struct EntropyGatherer EntropyGatherer;
struct EntropyGatherer {
  unsigned char *a;   /* Gather entropy into this buffer */
  ціле na;             /* Size of a[] in bytes */
  ціле i;              /* XOR next input into a[i] */
  ціле nXor;           /* Number of XOR operations done */
};

#якщо !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
/* Mix sz bytes of entropy into p. */
static void xorMemory(EntropyGatherer *p, unsigned char *x, ціле sz){
  ціле j, k;
  для(j=0, k=p->i; j<sz; j++){
    p->a[k++] ^= x[j];
    якщо( k>=p->na ) k = 0;
  }
  p->i = k;
  p->nXor += sz;
}
#endif /* !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS) */

/*
** Write up to nBuf bytes of randomness into zBuf.
*/
static ціле winRandomness(sqlite3_vfs *pVfs, ціле nBuf, char *zBuf){
#якщо defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS)
  UNUSED_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  повернути nBuf;
#інакше
  EntropyGatherer e;
  UNUSED_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  e.a = (unsigned char*)zBuf;
  e.na = nBuf;
  e.nXor = 0;
  e.i = 0;
  {
    FILETIME x;
    osGetSystemTimeAsFileTime(&x);
    xorMemory(&e, (unsigned char*)&x, sizeof(x));
  }
  {
    DWORD pid = osGetCurrentProcessId();
    xorMemory(&e, (unsigned char*)&pid, sizeof(pid));
  }
  {
    ULONGLONG cnt = osGetTickCount64();
    xorMemory(&e, (unsigned char*)&cnt, sizeof(cnt));
  }
  {
    LARGE_INTEGER i;
    osQueryPerformanceCounter(&i);
    xorMemory(&e, (unsigned char*)&i, sizeof(i));
  }
#якщо SQLITE_WIN32_USE_UUID
#ifdef SQLITE_UWP
# error SQLITE_WIN32_USE_UUID is incompatible with SQLITE_UWP
#endif
  {
    UUID id;
    memset(&id, 0, sizeof(id));
    osUuidCreate(&id);
    xorMemory(&e, (unsigned char*)&id, sizeof(id));
    memset(&id, 0, sizeof(UUID));
    osUuidCreateSequential(&id);
    xorMemory(&e, (unsigned char*)&id, sizeof(id));
  }
#endif /* SQLITE_WIN32_USE_UUID */
  повернути e.nXor>nBuf ? nBuf : e.nXor;
#endif /* defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS) */
}


/*
** Sleep for a little while.  Return the amount of time slept.
*/
static ціле winSleep(sqlite3_vfs *pVfs, ціле microsec){
  osSleep((microsec+999)/1000);
  UNUSED_PARAMETER(pVfs);
  повернути ((microsec+999)/1000)*1000;
}

/*
** The following variable, if set to a non-zero value, is interpreted as
** the number of seconds since 1970 and is used to set the result of
** sqlite3OsCurrentTime() during testing.
*/
#ifdef SQLITE_TEST
ціле sqlite3_current_time = 0;  /* Fake system time in seconds since 1970. */
#endif

/*
** Find the current time (in Universal Coordinated Time).  Write into *piNow
** the current time and date as a Julian Day number times 86_400_000.  In
** other words, write into *piNow the number of milliseconds since the Julian
** epoch of noon in Greenwich on November 24, 4714 B.C according to the
** proleptic Gregorian calendar.
**
** On success, return SQLITE_OK.  Return SQLITE_ERROR if the time and date
** cannot be found.
*/
static ціле winCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *piNow){
  /* FILETIME structure is a 64-bit value representing the number of
     100-nanosecond intervals since January 1, 1601 (= JD 2305813.5).
  */
  FILETIME ft;
  static const sqlite3_int64 winFiletimeEpoch = 23058135*(sqlite3_int64)8640000;
#ifdef SQLITE_TEST
  static const sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
#endif
  /* 2^32 - to avoid use of LL and warnings in gcc */
  static const sqlite3_int64 max32BitValue =
      (sqlite3_int64)2000000000 + (sqlite3_int64)2000000000 +
      (sqlite3_int64)294967296;

  osGetSystemTimeAsFileTime( &ft );
  *piNow = winFiletimeEpoch +
            ((((sqlite3_int64)ft.dwHighDateTime)*max32BitValue) +
               (sqlite3_int64)ft.dwLowDateTime)/(sqlite3_int64)10000;

#ifdef SQLITE_TEST
  якщо( sqlite3_current_time ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_time + unixEpoch;
  }
#endif
  UNUSED_PARAMETER(pVfs);
  повернути SQLITE_OK;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
static ціле winCurrentTime(sqlite3_vfs *pVfs, double *prNow){
  ціле rc;
  sqlite3_int64 i;
  rc = winCurrentTimeInt64(pVfs, &i);
  якщо( !rc ){
    *prNow = i/86400000.0;
  }
  повернути rc;
}

/*
** The idea is that this function works like a combination of
** GetLastError() and FormatMessage() on Windows (or errno and
** strerror_r() on Unix). After an error is returned by an OS
** function, SQLite calls this function with zBuf pointing to
** a buffer of nBuf bytes. The OS layer should populate the
** buffer with a nul-terminated UTF-8 encoded error message
** describing the last IO error to have occurred within the calling
** thread.
**
** If the error message is too large for the supplied buffer,
** it should be truncated. The return value of xGetLastError
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated). If non-zero is returned,
** then it is not necessary to include the nul-terminator character
** in the output buffer.
**
** Not supplying an error message will have no adverse effect
** on SQLite. It is fine to have an implementation that never
** returns an error message:
**
**   int xGetLastError(sqlite3_vfs *pVfs, int nBuf, char *zBuf){
**     assert(zBuf[0]=='\0');
**     return 0;
**   }
**
** However if an error message is supplied, it will be incorporated
** by sqlite into the error message available to the user using
** sqlite3_errmsg(), possibly making IO errors easier to debug.
*/
static ціле winGetLastError(sqlite3_vfs *pVfs, ціле nBuf, char *zBuf){
  DWORD e = osGetLastError();
  UNUSED_PARAMETER(pVfs);
  якщо( nBuf>0 ) winGetLastErrorMsg(e, nBuf, zBuf);
  повернути e;
}

/*
** Initialize and deinitialize the operating system interface.
*/
ціле sqlite3_os_init(void){
  static sqlite3_vfs winVfs = {
    3,                     /* iVersion */
    sizeof(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_PATH_BYTES, /* mxPathname */
    0,                     /* pNext */
    "win32",               /* zName */
    &winAppData,           /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    winFullPathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    winCurrentTime,        /* xCurrentTime */
    winGetLastError,       /* xGetLastError */
    winCurrentTimeInt64,   /* xCurrentTimeInt64 */
    winSetSystemCall,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    winNextSystemCall,     /* xNextSystemCall */
  };
  static sqlite3_vfs winLongPathVfs = {
    3,                     /* iVersion */
    sizeof(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_PATH_BYTES, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath",      /* zName */
    &winAppData,           /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    winFullPathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    winCurrentTime,        /* xCurrentTime */
    winGetLastError,       /* xGetLastError */
    winCurrentTimeInt64,   /* xCurrentTimeInt64 */
    winSetSystemCall,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    winNextSystemCall,     /* xNextSystemCall */
  };
  static sqlite3_vfs winNolockVfs = {
    3,                     /* iVersion */
    sizeof(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_PATH_BYTES, /* mxPathname */
    0,                     /* pNext */
    "win32-none",          /* zName */
    &winNolockAppData,     /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    winFullPathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    winCurrentTime,        /* xCurrentTime */
    winGetLastError,       /* xGetLastError */
    winCurrentTimeInt64,   /* xCurrentTimeInt64 */
    winSetSystemCall,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    winNextSystemCall,     /* xNextSystemCall */
  };
  static sqlite3_vfs winLongPathNolockVfs = {
    3,                     /* iVersion */
    sizeof(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_PATH_BYTES, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath-none", /* zName */
    &winNolockAppData,     /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    winFullPathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    winCurrentTime,        /* xCurrentTime */
    winGetLastError,       /* xGetLastError */
    winCurrentTimeInt64,   /* xCurrentTimeInt64 */
    winSetSystemCall,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    winNextSystemCall,     /* xNextSystemCall */
  };

  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  assert( ArraySize(aSyscall)==60 );
  assert( strcmp(aSyscall[0].zName,"AreFileApisANSI")==0 );
  assert( strcmp(aSyscall[8].zName,"FreeLibrary")==0 );
  assert( strcmp(aSyscall[16].zName,"GetSystemInfo")==0 );
  assert( strcmp(aSyscall[24].zName,"HeapReAlloc")==0 );
  assert( strcmp(aSyscall[32].zName,"MapViewOfFileFromApp")==0 );
  assert( strcmp(aSyscall[40].zName,"UnmapViewOfFile")==0 );
  assert( strcmp(aSyscall[48].zName,"UuidCreate")==0 );
  assert( strcmp(aSyscall[56].zName,"lstat")==0 );

  /* get memory map allocation granularity */
  memset(&winSysInfo, 0, sizeof(SYSTEM_INFO));
  osGetSystemInfo(&winSysInfo);
  assert( winSysInfo.dwAllocationGranularity>0 );
  assert( winSysInfo.dwPageSize>0 );

  sqlite3_vfs_register(&winVfs, 1);
  sqlite3_vfs_register(&winLongPathVfs, 0);
  sqlite3_vfs_register(&winNolockVfs, 0);
  sqlite3_vfs_register(&winLongPathNolockVfs, 0);
#ifndef SQLITE_OMIT_WAL
  winBigLock = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif

  повернути SQLITE_OK;
}

ціле sqlite3_os_end(void){
#ifndef SQLITE_OMIT_WAL
  winBigLock = 0;
#endif
  повернути SQLITE_OK;
}

#endif /* SQLITE_OS_WIN */
