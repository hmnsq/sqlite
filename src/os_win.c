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
#ضمن "sqliteInt.h"
#إذا SQLITE_OS_WIN               /* This file is used for Windows only */

/*
** Include code that is common to all os_*.c files
*/
#ضمن "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#ضمن "os_win.h"

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
                            (حجم(WCHAR) * SQLITE_WINNT_MAX_PATH_CHARS)
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
#ifndef UNUSED_VARIABLE_درج
#  define UNUSED_VARIABLE_درج(x)      (فراغ)(x)
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
تعريفنوع بنية winShm winShm;           /* A connection to shared-memory */
تعريفنوع بنية win_Shm_كلي win_Shm_كلي;   /* A region of shared-memory */
#endif

/*
** The winFile structure is a subclass of sqlite3_file* specific to the win32
** portability layer.
*/
تعريفنوع بنية winFile winFile;
بنية winFile {
  ثابت sqlite3_io_methods *pMethod; /*** Must be first ***/
  sqlite3_vfs *pVfs;      /* The VFS used to open this file */
  HANDLE h;               /* Handle for accessing the file */
  u8 locktype;            /* Type of lock currently held on this file */
  short sharedLockByte;   /* Randomly chosen byte used as a shared lock */
  u8 ctrlFlags;           /* Flags.  See WINFILE_* below */
  DWORD lastErrno;        /* The Windows errno from the last I/O error */
#ifndef SQLITE_OMIT_WAL
  winShm *pShm;           /* Instance of shared memory on this file */
#endif
  ثابت حرف *zPath;      /* Full pathname of this file */
  صحيح szChunk;            /* Chunk size configured by FCNTL_CHUNK_SIZE */
#إذا SQLITE_MAX_MMAP_SIZE>0
  صحيح nFetchOut;                /* Number of outstanding xFetch references */
  HANDLE hMap;                  /* Handle for accessing memory mapping */
  فراغ *pMapRegion;             /* Area memory mapped */
  sqlite3_int64 mmapSize;       /* Size of mapped region */
  sqlite3_int64 mmapSizeMax;    /* Configured FCNTL_MMAP_SIZE value */
#endif
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  DWORD iBusyTimeout;        /* Wait this many millisec on locks */
  صحيح bBlockOnConnect;
#endif
};

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
# define winFileBusyTimeout(pDbFd) pDbFd->iBusyTimeout
#وإلا
# define winFileBusyTimeout(pDbFd) 0
#endif

/*
** The winVfsAppData structure is used for the pAppData member for all of the
** Win32 VFS variants.
*/
تعريفنوع بنية winVfsAppData winVfsAppData;
بنية winVfsAppData {
  ثابت sqlite3_io_methods *pMethod; /* The file I/O methods to use. */
  فراغ *pAppData;                    /* The extra pAppData, if any. */
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
#  define SQLITE_WIN32_DBG_BUF_SIZE   ((صحيح)(4096-حجم(DWORD)))
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
#  إذا SQLITE_DEFAULT_CACHE_SIZE>=0
#    define SQLITE_WIN32_CACHE_SIZE     (SQLITE_DEFAULT_CACHE_SIZE)
#  وإلا
#    define SQLITE_WIN32_CACHE_SIZE     (-(SQLITE_DEFAULT_CACHE_SIZE))
#  endif
#endif

/*
 * Make sure that the calculated cache size, in pages, cannot cause the
 * initial size of the Win32-specific heap to exceed the maximum amount
 * of memory that can be specified in the call to HeapCreate.
 */
#إذا SQLITE_WIN32_CACHE_SIZE>SQLITE_WIN32_MAX_CACHE_SIZE
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
تعريفنوع بنية winMemData winMemData;
بنية winMemData {
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

ساكن بنية winMemData win_mem_data = {
#ifndef NDEBUG
  WINMEM_MAGIC1,
#endif
  عدم, FALSE
#ifndef NDEBUG
  ,WINMEM_MAGIC2
#endif
};

#ifndef NDEBUG
#define winMemAssertMagic1() أكد( win_mem_data.magic1==WINMEM_MAGIC1 )
#define winMemAssertMagic2() أكد( win_mem_data.magic2==WINMEM_MAGIC2 )
#define winMemAssertMagic()  winMemAssertMagic1(); winMemAssertMagic2();
#وإلا
#define winMemAssertMagic()
#endif

#define winMemGetDataPtr()  &win_mem_data
#define winMemGetHeap()     win_mem_data.hHeap
#define winMemGetOwned()    win_mem_data.bOwned

ساكن فراغ *winMemMalloc(صحيح nBytes);
ساكن فراغ winMemFree(فراغ *pPrior);
ساكن فراغ *winMemRealloc(فراغ *pPrior, صحيح nBytes);
ساكن صحيح winMemSize(فراغ *p);
ساكن صحيح winMemRoundup(صحيح n);
ساكن صحيح winMemInit(فراغ *pAppData);
ساكن فراغ winMemShutdown(فراغ *pAppData);

ثابت sqlite3_mem_methods *sqlite3MemGetWin32(فراغ);
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
ساكن بنية win_syscall {
  ثابت حرف *zName;            /* Name of the system call */
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

#إذا defined(SQLITE_UWP)
  { "CreateFileMappingFromApp",(SYSCALL)CreateFileMappingFromApp,0 },
#وإلا
  { "CreateFileMappingFromApp",(SYSCALL)0,                       0 },
#endif
#define osCreateFileMappingFromApp ((HANDLE(WINAPI*)(HANDLE,\
        PSECURITY_ATTRIBUTES, \
        ULONG,ULONG64,PCWSTR))aSyscall[3].pCurrent)

#إذا !defined(SQLITE_UWP)
  { "CreateFileMappingW",      (SYSCALL)CreateFileMappingW,      0 },
#وإلا
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

#إذا !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "FreeLibrary",             (SYSCALL)FreeLibrary,             0 },
#وإلا
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

#إذا !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "GetProcAddressA",         (SYSCALL)GetProcAddress,          0 },
#وإلا
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
#وإلا
  { "GetTempPathW",            (SYSCALL)GetTempPathW,            0 },
#endif
#define osGetTempPathW ((DWORD(WINAPI*)(DWORD,LPWSTR))aSyscall[18].pCurrent)

  { "GetTickCount64",          (SYSCALL)GetTickCount64,          0 },
#define osGetTickCount64 ((ULONGLONG(WINAPI*)(VOID))aSyscall[19].pCurrent)

#ifdef SQLITE_UWP
  { "HeapAlloc",               (SYSCALL)0,                       0 },
#وإلا
  { "HeapAlloc",               (SYSCALL)HeapAlloc,               0 },
#endif
#define osHeapAlloc ((LPVOID(WINAPI*)(HANDLE,DWORD, \
        SIZE_T))aSyscall[20].pCurrent)

#ifdef SQLITE_UWP
  { "HeapCreate",              (SYSCALL)0,                       0 },
#وإلا
  { "HeapCreate",              (SYSCALL)HeapCreate,              0 },
#endif
#define osHeapCreate ((HANDLE(WINAPI*)(DWORD,SIZE_T, \
        SIZE_T))aSyscall[21].pCurrent)

#ifdef SQLITE_UWP
  { "HeapDestroy",             (SYSCALL)0,                       0 },
#وإلا
  { "HeapDestroy",             (SYSCALL)HeapDestroy,             0 },
#endif
#define osHeapDestroy ((BOOL(WINAPI*)(HANDLE))aSyscall[22].pCurrent)

#ifdef SQLITE_UWP
  { "HeapFree",                (SYSCALL)0,                       0 },
#وإلا
  { "HeapFree",                (SYSCALL)HeapFree,                0 },
#endif
#define osHeapFree ((BOOL(WINAPI*)(HANDLE,DWORD,LPVOID))aSyscall[23].pCurrent)

#ifdef SQLITE_UWP
  { "HeapReAlloc",             (SYSCALL)0,                       0 },
#وإلا
  { "HeapReAlloc",             (SYSCALL)HeapReAlloc,             0 },
#endif
#define osHeapReAlloc ((LPVOID(WINAPI*)(HANDLE,DWORD,LPVOID, \
        SIZE_T))aSyscall[24].pCurrent)

#ifdef SQLITE_UWP
  { "HeapSize",                (SYSCALL)0,                       0 },
#وإلا
  { "HeapSize",                (SYSCALL)HeapSize,                0 },
#endif
#define osHeapSize ((SIZE_T(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[25].pCurrent)

#ifdef SQLITE_UWP
  { "HeapValidate",            (SYSCALL)0,                       0 },
#وإلا
  { "HeapValidate",            (SYSCALL)HeapValidate,            0 },
#endif
#define osHeapValidate ((BOOL(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[26].pCurrent)

#ifdef SQLITE_UWP
  { "HeapCompact",             (SYSCALL)0,                       0 },
#وإلا
  { "HeapCompact",             (SYSCALL)HeapCompact,             0 },
#endif
#define osHeapCompact ((UINT(WINAPI*)(HANDLE,DWORD))aSyscall[27].pCurrent)


#إذا !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "LoadLibraryW",            (SYSCALL)LoadLibraryW,            0 },
#وإلا
  { "LoadLibraryW",            (SYSCALL)0,                       0 },
#endif
#define osLoadLibraryW ((HMODULE(WINAPI*)(LPCWSTR))aSyscall[28].pCurrent)

  { "LocalFree",               (SYSCALL)LocalFree,               0 },
#define osLocalFree ((HLOCAL(WINAPI*)(HLOCAL))aSyscall[29].pCurrent)

  { "LockFileEx",              (SYSCALL)LockFileEx,              0 },
#define osLockFileEx ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[30].pCurrent)

#إذا (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0) \
 && !defined(SQLITE_UWP)
  { "MapViewOfFile",           (SYSCALL)MapViewOfFile,           0 },
#وإلا
  { "MapViewOfFile",           (SYSCALL)0,                       0 },
#endif
#define osMapViewOfFile ((LPVOID(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        SIZE_T))aSyscall[31].pCurrent)

#إذا (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0) \
 && defined(SQLITE_UWP)
  { "MapViewOfFileFromApp",    (SYSCALL)MapViewOfFileFromApp,   0 },
#وإلا
  { "MapViewOfFileFromApp",    (SYSCALL)0,                      0 },
#endif
#define osMapViewOfFileFromApp ((LPVOID(WINAPI*)(HANDLE,ULONG,ULONG64, \
        SIZE_T))aSyscall[32].pCurrent)

  { "MultiByteToWideChar",     (SYSCALL)MultiByteToWideChar,     0 },
#define osMultiByteToWideChar ((صحيح(WINAPI*)(UINT,DWORD,LPCSTR,صحيح,LPWSTR, \
        صحيح))aSyscall[33].pCurrent)

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

#إذا !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0
  { "UnmapViewOfFile",         (SYSCALL)UnmapViewOfFile,         0 },
#وإلا
  { "UnmapViewOfFile",         (SYSCALL)0,                       0 },
#endif
#define osUnmapViewOfFile ((BOOL(WINAPI*)(LPCVOID))aSyscall[40].pCurrent)

  { "WideCharToMultiByte",     (SYSCALL)WideCharToMultiByte,     0 },
#define osWideCharToMultiByte ((صحيح(WINAPI*)(UINT,DWORD,LPCWSTR,صحيح,LPSTR,صحيح, \
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
#إذا defined(InterlockedCompareExchange)
  { "InterlockedCompareExchange", (SYSCALL)0,                    0 },
#define osInterlockedCompareExchange InterlockedCompareExchange
#وإلا
  { "InterlockedCompareExchange", (SYSCALL)InterlockedCompareExchange, 0 },
#define osInterlockedCompareExchange ((LONG(WINAPI*)(LONG متطاير*,\
        LONG,LONG))aSyscall[47].pCurrent)
#endif /* defined(InterlockedCompareExchange) */

#إذا SQLITE_WIN32_USE_UUID
  { "UuidCreate",               (SYSCALL)UuidCreate,             0 },
#وإلا
  { "UuidCreate",               (SYSCALL)0,                      0 },
#endif
#define osUuidCreate ((RPC_STATUS(RPC_ENTRY*)(UUID*))aSyscall[48].pCurrent)

#إذا SQLITE_WIN32_USE_UUID
  { "UuidCreateSequential",     (SYSCALL)UuidCreateSequential,   0 },
#وإلا
  { "UuidCreateSequential",     (SYSCALL)0,                      0 },
#endif
#define osUuidCreateSequential \
        ((RPC_STATUS(RPC_ENTRY*)(UUID*))aSyscall[49].pCurrent)

#إذا !defined(SQLITE_NO_SYNC) && SQLITE_MAX_MMAP_SIZE>0
  { "FlushViewOfFile",          (SYSCALL)FlushViewOfFile,        0 },
#وإلا
  { "FlushViewOfFile",          (SYSCALL)0,                      0 },
#endif
#define osFlushViewOfFile \
        ((BOOL(WINAPI*)(LPCVOID,SIZE_T))aSyscall[50].pCurrent)

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  { "CreateEvent",              (SYSCALL)CreateEvent,            0 },
#وإلا
  { "CreateEvent",              (SYSCALL)0,                      0 },
#endif
#define osCreateEvent ((HANDLE(WINAPI*)(LPSECURITY_ATTRIBUTES,BOOL, \
                        BOOL,LPCSTR))aSyscall[51].pCurrent)

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  { "CancelIo",                 (SYSCALL)CancelIo,               0 },
#وإلا
  { "CancelIo",                 (SYSCALL)0,                      0 },
#endif
#define osCancelIo ((BOOL(WINAPI*)(HANDLE))aSyscall[52].pCurrent)

#ifndef _WIN32
  { "getenv",                   (SYSCALL)getenv,                 0 },
#وإلا
  { "getenv",                   (SYSCALL)0,                      0 },
#endif
#define osGetenv ((ثابت حرف *(*)(ثابت حرف *))aSyscall[53].pCurrent)

#ifndef _WIN32
  { "getcwd",                   (SYSCALL)getcwd,                 0 },
#وإلا
  { "getcwd",                   (SYSCALL)0,                      0 },
#endif
#define osGetcwd ((حرف*(*)(حرف*,size_t))aSyscall[54].pCurrent)

#ifndef _WIN32
  { "readlink",                 (SYSCALL)readlink,               0 },
#وإلا
  { "readlink",                 (SYSCALL)0,                      0 },
#endif
#define osReadlink ((ssize_t(*)(ثابت حرف*,حرف*,size_t))aSyscall[55].pCurrent)

#ifndef _WIN32
  { "lstat",                    (SYSCALL)lstat,                  0 },
#وإلا
  { "lstat",                    (SYSCALL)0,                      0 },
#endif
#define osLstat ((صحيح(*)(ثابت حرف*,بنية stat*))aSyscall[56].pCurrent)

#ifndef _WIN32
  { "__errno",                  (SYSCALL)__errno,                0 },
#وإلا
  { "__errno",                  (SYSCALL)0,                      0 },
#endif
#define osErrno (*((صحيح*(*)(فراغ))aSyscall[57].pCurrent)())

#ifndef _WIN32
  { "cygwin_conv_path",         (SYSCALL)cygwin_conv_path,       0 },
#وإلا
  { "cygwin_conv_path",         (SYSCALL)0,                      0 },
#endif
#define osCygwin_conv_path ((size_t(*)(unsigned صحيح, \
    ثابت فراغ *, فراغ *, size_t))aSyscall[58].pCurrent)

  { "GetDriveTypeW",            (SYSCALL)GetDriveTypeW,          0 },
#define osGetDriveTypeW ((UINT(WINAPI*)(LPCWSTR))aSyscall[59].pCurrent)

}; /* End of the overrideable system calls */

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "win32" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
ساكن صحيح winSetSystemCall(
  sqlite3_vfs *pNotUsed,        /* The VFS pointer.  Not used */
  ثابت حرف *zName,            /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
){
  unsigned صحيح i;
  صحيح rc = SQLITE_NOTFOUND;

  UNUSED_PARAMETER(pNotUsed);
  إذا( zName==0 ){
    /* If no zName is given, restore all system calls to their default
    ** settings and return NULL
    */
    rc = SQLITE_OK;
    لكل(i=0; i<حجم(aSyscall)/حجم(aSyscall[0]); i++){
      إذا( aSyscall[i].pDefault ){
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  }وإلا{
    /* If zName is specified, operate on only the one system call
    ** specified.
    */
    لكل(i=0; i<حجم(aSyscall)/حجم(aSyscall[0]); i++){
      إذا( strcmp(zName, aSyscall[i].zName)==0 ){
        إذا( aSyscall[i].pDefault==0 ){
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = SQLITE_OK;
        إذا( pNewFunc==0 ) pNewFunc = aSyscall[i].pDefault;
        aSyscall[i].pCurrent = pNewFunc;
        اكسر;
      }
    }
  }
  أرجع rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
ساكن sqlite3_syscall_ptr winGetSystemCall(
  sqlite3_vfs *pNotUsed,
  ثابت حرف *zName
){
  unsigned صحيح i;

  UNUSED_PARAMETER(pNotUsed);
  لكل(i=0; i<حجم(aSyscall)/حجم(aSyscall[0]); i++){
    إذا( strcmp(zName, aSyscall[i].zName)==0 ) أرجع aSyscall[i].pCurrent;
  }
  أرجع 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
ساكن ثابت حرف *winNextSystemCall(sqlite3_vfs *p, ثابت حرف *zName){
  صحيح i = -1;

  UNUSED_PARAMETER(p);
  إذا( zName ){
    لكل(i=0; i<ArraySize(aSyscall)-1; i++){
      إذا( strcmp(zName, aSyscall[i].zName)==0 ) اكسر;
    }
  }
  لكل(i++; i<ArraySize(aSyscall); i++){
    إذا( aSyscall[i].pCurrent!=0 ) أرجع aSyscall[i].zName;
  }
  أرجع 0;
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
صحيح sqlite3_win32_compact_heap(LPUINT pnLargest){
  صحيح rc = SQLITE_OK;
  UINT nLargest = 0;
  HANDLE hHeap;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  أكد( hHeap!=0 );
  أكد( hHeap!=INVALID_HANDLE_درج );
#إذا defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, عدم) );
#endif
  إذا( (nLargest=osHeapCompact(hHeap, SQLITE_WIN32_HEAP_FLAGS))==0 ){
    DWORD lastErrno = osGetLastError();
    إذا( lastErrno==NO_ERROR ){
      sqlite3_log(SQLITE_NOMEM, "failed to HeapCompact (no space), heap=%p",
                  (فراغ*)hHeap);
      rc = SQLITE_NOMEM_BKPT;
    }وإلا{
      sqlite3_log(SQLITE_ERROR, "failed to HeapCompact (%lu), heap=%p",
                  osGetLastError(), (فراغ*)hHeap);
      rc = SQLITE_ERROR;
    }
  }
  إذا( pnLargest ) *pnLargest = nLargest;
  أرجع rc;
}

/*
** If a Win32 native heap has been configured, this function will attempt to
** destroy and recreate it.  If the Win32 native heap is not isolated and/or
** the sqlite3_memory_used() function does not return zero, SQLITE_BUSY will
** be returned and no changes will be made to the Win32 native heap.
*/
صحيح sqlite3_win32_reset_heap(){
  صحيح rc;
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; ) /* The main static mutex */
  MUTEX_LOGIC( sqlite3_mutex *pMem; )    /* The memsys static mutex */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  MUTEX_LOGIC( pMem = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3_mutex_enter(pMem);
  winMemAssertMagic();
  إذا( winMemGetHeap()!=عدم && winMemGetOwned() && sqlite3_memory_used()==0 ){
    /*
    ** At this point, there should be no outstanding memory allocations on
    ** the heap.  Also, since both the main and memsys locks are currently
    ** being held by us, no other function (i.e. from another thread) should
    ** be able to even access the heap.  Attempt to destroy and recreate our
    ** isolated Win32 native heap now.
    */
    أكد( winMemGetHeap()!=عدم );
    أكد( winMemGetOwned() );
    أكد( sqlite3_memory_used()==0 );
    winMemShutdown(winMemGetDataPtr());
    أكد( winMemGetHeap()==عدم );
    أكد( !winMemGetOwned() );
    أكد( sqlite3_memory_used()==0 );
    rc = winMemInit(winMemGetDataPtr());
    أكد( rc!=SQLITE_OK || winMemGetHeap()!=عدم );
    أكد( rc!=SQLITE_OK || winMemGetOwned() );
    أكد( rc!=SQLITE_OK || sqlite3_memory_used()==0 );
  }وإلا{
    /*
    ** The Win32 native heap cannot be modified because it may be in use.
    */
    rc = SQLITE_BUSY;
  }
  sqlite3_mutex_leave(pMem);
  sqlite3_mutex_leave(pMainMtx);
  أرجع rc;
}
#endif /* SQLITE_WIN32_MALLOC */

#ifdef _WIN32
/*
** This function outputs the specified (ANSI) string to the Win32 debugger
** (if available).  Undocumented.  Might go away at any moment.
*/
فراغ sqlite3_win32_write_debug(ثابت حرف *zBuf, صحيح nBuf){
  حرف zDbgBuf[SQLITE_WIN32_DBG_BUF_SIZE];
  صحيح nMin = MIN(nBuf, (SQLITE_WIN32_DBG_BUF_SIZE - 1)); /* may be negative. */
  إذا( nMin<-1 ) nMin = -1; /* all negative values become -1. */
  أكد( nMin==-1 || nMin==0 || nMin<SQLITE_WIN32_DBG_BUF_SIZE );
#ifdef SQLITE_ENABLE_API_ARMOR
  إذا( !zBuf ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع;
  }
#endif
  إذا( nMin>0 ){
    memset(zDbgBuf, 0, SQLITE_WIN32_DBG_BUF_SIZE);
    memcpy(zDbgBuf, zBuf, nMin);
    osOutputDebugStringA(zDbgBuf);
  }وإلا{
    osOutputDebugStringA(zBuf);
  }
}

فراغ sqlite3_win32_sleep(DWORD milliseconds){
  osSleep(milliseconds);
}

#إذا SQLITE_MAX_WORKER_THREADS>0 && SQLITE_THREADSAFE>0
DWORD sqlite3Win32Wait(HANDLE hObject){
  DWORD rc;
  طالما( (rc = osWaitForSingleObjectEx(hObject, INFINITE,
                                       TRUE))==WAIT_IO_COMPLETION ){}
  أرجع rc;
}
#endif
#endif /* _WIN32 */

#ifdef SQLITE_WIN32_MALLOC
/*
** Allocate nBytes of memory.
*/
ساكن فراغ *winMemMalloc(صحيح nBytes){
  HANDLE hHeap;
  فراغ *p;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  أكد( hHeap!=0 );
  أكد( hHeap!=INVALID_HANDLE_درج );
#إذا defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, عدم) );
#endif
  أكد( nBytes>=0 );
  p = osHeapAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, (SIZE_T)nBytes);
  إذا( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapAlloc %u bytes (%lu), heap=%p",
                nBytes, osGetLastError(), (فراغ*)hHeap);
  }
  أرجع p;
}

/*
** Free memory.
*/
ساكن فراغ winMemFree(فراغ *pPrior){
  HANDLE hHeap;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  أكد( hHeap!=0 );
  أكد( hHeap!=INVALID_HANDLE_درج );
#إذا defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) );
#endif
  إذا( !pPrior ) أرجع; /* Passing NULL to HeapFree is undefined. */
  إذا( !osHeapFree(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapFree block %p (%lu), heap=%p",
                pPrior, osGetLastError(), (فراغ*)hHeap);
  }
}

/*
** Change the size of an existing memory allocation
*/
ساكن فراغ *winMemRealloc(فراغ *pPrior, صحيح nBytes){
  HANDLE hHeap;
  فراغ *p;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  أكد( hHeap!=0 );
  أكد( hHeap!=INVALID_HANDLE_درج );
#إذا defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) );
#endif
  أكد( nBytes>=0 );
  إذا( !pPrior ){
    p = osHeapAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, (SIZE_T)nBytes);
  }وإلا{
    p = osHeapReAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior, (SIZE_T)nBytes);
  }
  إذا( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to %s %u bytes (%lu), heap=%p",
                pPrior ? "HeapReAlloc" : "HeapAlloc", nBytes, osGetLastError(),
                (فراغ*)hHeap);
  }
  أرجع p;
}

/*
** Return the size of an outstanding allocation, in bytes.
*/
ساكن صحيح winMemSize(فراغ *p){
  HANDLE hHeap;
  SIZE_T n;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  أكد( hHeap!=0 );
  أكد( hHeap!=INVALID_HANDLE_درج );
#إذا defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, p) );
#endif
  إذا( !p ) أرجع 0;
  n = osHeapSize(hHeap, SQLITE_WIN32_HEAP_FLAGS, p);
  إذا( n==(SIZE_T)-1 ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapSize block %p (%lu), heap=%p",
                p, osGetLastError(), (فراغ*)hHeap);
    أرجع 0;
  }
  أرجع (صحيح)n;
}

/*
** Round up a request size to the next valid allocation size.
*/
ساكن صحيح winMemRoundup(صحيح n){
  أرجع n;
}

/*
** Initialize this module.
*/
ساكن صحيح winMemInit(فراغ *pAppData){
  winMemData *pWinMemData = (winMemData *)pAppData;

  إذا( !pWinMemData ) أرجع SQLITE_ERROR;
  أكد( pWinMemData->magic1==WINMEM_MAGIC1 );
  أكد( pWinMemData->magic2==WINMEM_MAGIC2 );

#إذا SQLITE_WIN32_HEAP_CREATE
  إذا( !pWinMemData->hHeap ){
    DWORD dwInitialSize = SQLITE_WIN32_HEAP_INIT_SIZE;
    DWORD dwMaximumSize = (DWORD)sqlite3GlobalConfig.nHeap;
    إذا( dwMaximumSize==0 ){
      dwMaximumSize = SQLITE_WIN32_HEAP_MAX_SIZE;
    }وإلا إذا( dwInitialSize>dwMaximumSize ){
      dwInitialSize = dwMaximumSize;
    }
    pWinMemData->hHeap = osHeapCreate(SQLITE_WIN32_HEAP_FLAGS,
                                      dwInitialSize, dwMaximumSize);
    إذا( !pWinMemData->hHeap ){
      sqlite3_log(SQLITE_NOMEM,
          "failed to HeapCreate (%lu), flags=%u, initSize=%lu, maxSize=%lu",
          osGetLastError(), SQLITE_WIN32_HEAP_FLAGS, dwInitialSize,
          dwMaximumSize);
      أرجع SQLITE_NOMEM_BKPT;
    }
    pWinMemData->bOwned = TRUE;
    أكد( pWinMemData->bOwned );
  }
#وإلا
  pWinMemData->hHeap = osGetProcessHeap();
  إذا( !pWinMemData->hHeap ){
    sqlite3_log(SQLITE_NOMEM,
        "failed to GetProcessHeap (%lu)", osGetLastError());
    أرجع SQLITE_NOMEM_BKPT;
  }
  pWinMemData->bOwned = FALSE;
  أكد( !pWinMemData->bOwned );
#endif
  أكد( pWinMemData->hHeap!=0 );
  أكد( pWinMemData->hHeap!=INVALID_HANDLE_درج );
#إذا defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( osHeapValidate(pWinMemData->hHeap, SQLITE_WIN32_HEAP_FLAGS, عدم) );
#endif
  أرجع SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ساكن فراغ winMemShutdown(فراغ *pAppData){
  winMemData *pWinMemData = (winMemData *)pAppData;

  إذا( !pWinMemData ) أرجع;
  أكد( pWinMemData->magic1==WINMEM_MAGIC1 );
  أكد( pWinMemData->magic2==WINMEM_MAGIC2 );

  إذا( pWinMemData->hHeap ){
    أكد( pWinMemData->hHeap!=INVALID_HANDLE_درج );
#إذا defined(SQLITE_WIN32_MALLOC_VALIDATE)
    أكد( osHeapValidate(pWinMemData->hHeap, SQLITE_WIN32_HEAP_FLAGS, عدم) );
#endif
    إذا( pWinMemData->bOwned ){
      إذا( !osHeapDestroy(pWinMemData->hHeap) ){
        sqlite3_log(SQLITE_NOMEM, "failed to HeapDestroy (%lu), heap=%p",
                    osGetLastError(), (فراغ*)pWinMemData->hHeap);
      }
      pWinMemData->bOwned = FALSE;
    }
    pWinMemData->hHeap = عدم;
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
ثابت sqlite3_mem_methods *sqlite3MemGetWin32(فراغ){
  ساكن ثابت sqlite3_mem_methods winMemMethods = {
    winMemMalloc,
    winMemFree,
    winMemRealloc,
    winMemSize,
    winMemRoundup,
    winMemInit,
    winMemShutdown,
    &win_mem_data
  };
  أرجع &winMemMethods;
}

فراغ sqlite3MemSetDefault(فراغ){
  sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetWin32());
}
#endif /* SQLITE_WIN32_MALLOC */

#ifdef _WIN32
/*
** Convert a UTF-8 string to Microsoft Unicode.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساكن LPWSTR winUtf8ToUnicode(ثابت حرف *zText){
  صحيح nChar;
  LPWSTR zWideText;

  nChar = osMultiByteToWideChar(CP_UTF8, 0, zText, -1, عدم, 0);
  إذا( nChar==0 ){
    أرجع 0;
  }
  zWideText = sqlite3MallocZero( nChar*حجم(WCHAR) );
  إذا( zWideText==0 ){
    أرجع 0;
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zText, -1, zWideText,
                                nChar);
  إذا( nChar==0 ){
    sqlite3_free(zWideText);
    zWideText = 0;
  }
  أرجع zWideText;
}
#endif /* _WIN32 */

/*
** Convert a Microsoft Unicode string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساكن حرف *winUnicodeToUtf8(LPCWSTR zWideText){
  صحيح nByte;
  حرف *zText;

  nByte = osWideCharToMultiByte(CP_UTF8, 0, zWideText, -1, 0, 0, 0, 0);
  إذا( nByte == 0 ){
    أرجع 0;
  }
  zText = sqlite3MallocZero( nByte );
  إذا( zText==0 ){
    أرجع 0;
  }
  nByte = osWideCharToMultiByte(CP_UTF8, 0, zWideText, -1, zText, nByte,
                                0, 0);
  إذا( nByte == 0 ){
    sqlite3_free(zText);
    zText = 0;
  }
  أرجع zText;
}

#ifdef _WIN32
/*
** Convert an ANSI string to Microsoft Unicode, using the ANSI or OEM
** code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساكن LPWSTR winMbcsToUnicode(ثابت حرف *zText, صحيح useAnsi){
  صحيح nWideChar;
  LPWSTR zMbcsText;
  صحيح codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nWideChar = osMultiByteToWideChar(codepage, 0, zText, -1, عدم,
                                0);
  إذا( nWideChar==0 ){
    أرجع 0;
  }
  zMbcsText = sqlite3MallocZero( nWideChar*حجم(WCHAR) );
  إذا( zMbcsText==0 ){
    أرجع 0;
  }
  nWideChar = osMultiByteToWideChar(codepage, 0, zText, -1, zMbcsText,
                                nWideChar);
  إذا( nWideChar==0 ){
    sqlite3_free(zMbcsText);
    zMbcsText = 0;
  }
  أرجع zMbcsText;
}

/*
** Convert a Microsoft Unicode string to a multi-byte character string,
** using the ANSI or OEM code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساكن حرف *winUnicodeToMbcs(LPCWSTR zWideText, صحيح useAnsi){
  صحيح nByte;
  حرف *zText;
  صحيح codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nByte = osWideCharToMultiByte(codepage, 0, zWideText, -1, 0, 0, 0, 0);
  إذا( nByte == 0 ){
    أرجع 0;
  }
  zText = sqlite3MallocZero( nByte );
  إذا( zText==0 ){
    أرجع 0;
  }
  nByte = osWideCharToMultiByte(codepage, 0, zWideText, -1, zText,
                                nByte, 0, 0);
  إذا( nByte == 0 ){
    sqlite3_free(zText);
    zText = 0;
  }
  أرجع zText;
}

/*
** Convert a multi-byte character string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساكن حرف *winMbcsToUtf8(ثابت حرف *zText, صحيح useAnsi){
  حرف *zTextUtf8;
  LPWSTR zTmpWide;

  zTmpWide = winMbcsToUnicode(zText, useAnsi);
  إذا( zTmpWide==0 ){
    أرجع 0;
  }
  zTextUtf8 = winUnicodeToUtf8(zTmpWide);
  sqlite3_free(zTmpWide);
  أرجع zTextUtf8;
}

/*
** Convert a UTF-8 string to a multi-byte character string.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساكن حرف *winUtf8ToMbcs(ثابت حرف *zText, صحيح useAnsi){
  حرف *zTextMbcs;
  LPWSTR zTmpWide;

  zTmpWide = winUtf8ToUnicode(zText);
  إذا( zTmpWide==0 ){
    أرجع 0;
  }
  zTextMbcs = winUnicodeToMbcs(zTmpWide, useAnsi);
  sqlite3_free(zTmpWide);
  أرجع zTextMbcs;
}

/*
** This is a public wrapper for the winUtf8ToUnicode() function.
*/
LPWSTR sqlite3_win32_utf8_to_unicode(ثابت حرف *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  إذا( !zText ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#endif
  أرجع winUtf8ToUnicode(zText);
}

/*
** This is a public wrapper for the winUnicodeToUtf8() function.
*/
حرف *sqlite3_win32_unicode_to_utf8(LPCWSTR zWideText){
#ifdef SQLITE_ENABLE_API_ARMOR
  إذا( !zWideText ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#endif
  أرجع winUnicodeToUtf8(zWideText);
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
حرف *sqlite3_win32_mbcs_to_utf8(ثابت حرف *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  إذا( !zText ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#endif
  أرجع winMbcsToUtf8(zText, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
حرف *sqlite3_win32_mbcs_to_utf8_v2(ثابت حرف *zText, صحيح useAnsi){
#ifdef SQLITE_ENABLE_API_ARMOR
  إذا( !zText ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#endif
  أرجع winMbcsToUtf8(zText, useAnsi);
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
حرف *sqlite3_win32_utf8_to_mbcs(ثابت حرف *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  إذا( !zText ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#endif
  أرجع winUtf8ToMbcs(zText, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
حرف *sqlite3_win32_utf8_to_mbcs_v2(ثابت حرف *zText, صحيح useAnsi){
#ifdef SQLITE_ENABLE_API_ARMOR
  إذا( !zText ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#endif
  أرجع winUtf8ToMbcs(zText, useAnsi);
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-8 string.
*/
صحيح sqlite3_win32_set_directory8(
  unsigned long type, /* Identifier for directory being set or reset */
  ثابت حرف *z_درج  /* New value for directory being set or reset */
){
  حرف **ppDirectory = 0;
  صحيح rc;
#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  إذا( rc ) أرجع rc;
#endif
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  إذا( type==SQLITE_WIN32_DATA_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_data_directory;
  }وإلا إذا( type==SQLITE_WIN32_TEMP_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_temp_directory;
  }
  أكد( !ppDirectory || type==SQLITE_WIN32_DATA_DIRECTORY_TYPE
          || type==SQLITE_WIN32_TEMP_DIRECTORY_TYPE
  );
  أكد( !ppDirectory || sqlite3MemdebugHasType(*ppDirectory, MEMTYPE_HEAP) );
  إذا( ppDirectory ){
    حرف *zCopy = 0;
    إذا( z_درج && z_درج[0] ){
      zCopy = sqlite3_mprintf("%s", z_درج);
      إذا ( zCopy==0 ){
        rc = SQLITE_NOMEM_BKPT;
        اذهب set_directory8_done;
      }
    }
    sqlite3_free(*ppDirectory);
    *ppDirectory = zCopy;
    rc = SQLITE_OK;
  }وإلا{
    rc = SQLITE_ERROR;
  }
set_directory8_done:
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  أرجع rc;
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-16 string.
*/
صحيح sqlite3_win32_set_directory16(
  unsigned long type, /* Identifier for directory being set or reset */
  ثابت فراغ *z_درج  /* New value for directory being set or reset */
){
  صحيح rc;
  حرف *zUtf8 = 0;
  إذا( z_درج ){
    zUtf8 = sqlite3_win32_unicode_to_utf8(z_درج);
    إذا( zUtf8==0 ) أرجع SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_win32_set_directory8(type, zUtf8);
  إذا( zUtf8 ) sqlite3_free(zUtf8);
  أرجع rc;
}

/*
** This function sets the data directory or the temporary directory based on
** the provided arguments.  The type argument must be 1 in order to set the
** data directory or 2 in order to set the temporary directory.  The zValue
** argument is the name of the directory to use.  The return value will be
** SQLITE_OK if successful.
*/
صحيح sqlite3_win32_set_directory(
  unsigned long type, /* Identifier for directory being set or reset */
  فراغ *z_درج        /* New value for directory being set or reset */
){
  أرجع sqlite3_win32_set_directory16(type, z_درج);
}
#endif /* _WIN32 */

/*
** The return value of winGetLastErrorMsg
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated).
*/
ساكن صحيح win_Get_Last_Error_سكو(DWORD lastErrno, صحيح nBuf, حرف *zBuf){
  /* FormatMessage returns 0 on failure.  Otherwise it
  ** returns the number of TCHARs written to the output
  ** buffer, excluding the terminating null char.
  */
  DWORD dwLen = 0;
  حرف *zOut = 0;
  LPWSTR zTempWide = عدم;
  dwLen = osFormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_FROM_SYSTEM |
                           FORMAT_MESSAGE_IGNORE_INSERTS,
                           عدم,
                           lastErrno,
                           0,
                           (LPWSTR) &zTempWide,
                           0,
                           0);
  إذا( dwLen > 0 ){
    /* allocate a buffer and convert to UTF8 */
    sqlite3BeginBenignMalloc();
    zOut = winUnicodeToUtf8(zTempWide);
    sqlite3EndBenignMalloc();
    /* free the system buffer allocated by FormatMessage */
    osLocalFree(zTempWide);
  }
  إذا( 0 == dwLen ){
    sqlite3_snprintf(nBuf, zBuf, "OsError 0x%lx (%lu)", lastErrno, lastErrno);
  }وإلا{
    /* copy a maximum of nBuf chars to output buffer */
    sqlite3_snprintf(nBuf, zBuf, "%s", zOut);
    /* free the UTF8 buffer */
    sqlite3_free(zOut);
  }
  أرجع 0;
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
#define winLogError(a,b,c,d)   win_Log_Error_At_لي(a,b,c,d,__LINE__)
ساكن صحيح win_Log_Error_At_لي(
  صحيح errcode,                    /* SQLite error code */
  DWORD lastErrno,                /* Win32 last error */
  ثابت حرف *zFunc,              /* Name of OS function that failed */
  ثابت حرف *zPath,              /* File path associated with error */
  صحيح i_لي                       /* Source line number where error occurred */
){
  حرف z_سكو[500];                 /* Human readable error text */
  صحيح i;                          /* Loop counter */

  z_سكو[0] = 0;
  win_Get_Last_Error_سكو(lastErrno, حجم(z_سكو), z_سكو);
  أكد( errcode!=SQLITE_OK );
  إذا( zPath==0 ) zPath = "";
  لكل(i=0; z_سكو[i] && z_سكو[i]!='\r' && z_سكو[i]!='\n'; i++){}
  z_سكو[i] = 0;
  sqlite3_log(errcode,
      "os_win.c:%d: (%lu) %s(%s) - %s",
      i_لي, lastErrno, zFunc, zPath, z_سكو
  );

  أرجع errcode;
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
ساكن صحيح winIoerrRetry = SQLITE_WIN32_IOERR_RETRY;
ساكن صحيح winIoerrRetryDelay = SQLITE_WIN32_IOERR_RETRY_DELAY;

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
#إذا !defined(winIoerrCanRetry1)
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
ساكن صحيح winRetryIoerr(صحيح *pnRetry, DWORD *pError){
  DWORD e = osGetLastError();
  إذا( *pnRetry>=winIoerrRetry ){
    إذا( pError ){
      *pError = e;
    }
    أرجع 0;
  }
  إذا( winIoerrCanRetry1(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    أرجع 1;
  }
#إذا defined(winIoerrCanRetry2)
  وإلا إذا( winIoerrCanRetry2(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    أرجع 1;
  }
#endif
  إذا( pError ){
    *pError = e;
  }
  أرجع 0;
}

/*
** Log a I/O error retry episode.
*/
ساكن فراغ winLogIoerr(صحيح nRetry, صحيح lineno){
  إذا( nRetry ){
    sqlite3_log(SQLITE_NOTICE,
      "delayed %dms for lock/sharing conflict at line %d",
      winIoerrRetryDelay*nRetry*(nRetry+1)/2, lineno
    );
  }
}


/*
** Lock a file region.
*/
ساكن BOOL winLockFile(
  LPHANDLE phFile,
  DWORD flags,
  DWORD offset_كس,
  DWORD offset_سم,
  DWORD لمب_Bytes_كس,
  DWORD لمب_Bytes_سم
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, حجم(OVERLAPPED));
  ovlp.Offset = offset_كس;
  ovlp.Offset_سم = offset_سم;
  أرجع osLockFileEx(*phFile, flags, 0, لمب_Bytes_كس, لمب_Bytes_سم, &ovlp);
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
ساكن صحيح winHandleLockTimeout(
  HANDLE hFile,
  DWORD offset,
  DWORD nByte,
  صحيح bExcl,
  DWORD nMs
){
  DWORD flags = LOCKFILE_FAIL_IMMEDIATELY | (bExcl?LOCKFILE_EXCLUSIVE_LOCK:0);
  صحيح rc = SQLITE_OK;
  BOOL ret;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, حجم(OVERLAPPED));
  ovlp.Offset = offset;

#إذا defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  إذا( nMs!=0 ){
    flags &= ~LOCKFILE_FAIL_IMMEDIATELY;
  }
  ovlp.hEvent = osCreateEvent(عدم, TRUE, FALSE, عدم);
  إذا( ovlp.hEvent==عدم ){
    أرجع SQLITE_IOERR_LOCK;
  }
#endif
  ret = osLockFileEx(hFile, flags, 0, nByte, 0, &ovlp);
#إذا defined(SQLITE_ENABLE_SETLK_TIMEOUT)
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
  إذا( !ret && GetLastError()==ERROR_IO_PENDING ){
    DWORD nDelay = (nMs==0 ? INFINITE : nMs);
    DWORD res = osWaitForSingleObject(ovlp.hEvent, nDelay);
    إذا( res==WAIT_OBJECT_0 ){
      ret = TRUE;
    }وإلا إذا( res==WAIT_TIMEOUT ){
#إذا SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = SQLITE_BUSY_TIMEOUT;
#وإلا
      rc = SQLITE_BUSY;
#endif
    }وإلا{
      /* Some other error has occurred */
      rc = SQLITE_IOERR_LOCK;
    }

    /* If it is still pending, cancel the LockFileEx() call. */
    osCancelIo(hFile);
  }

  osCloseHandle(ovlp.hEvent);
#endif /* defined(SQLITE_ENABLE_SETLK_TIMEOUT) */

  إذا( rc==SQLITE_OK && !ret ){
    rc = SQLITE_BUSY;
  }
  أرجع rc;
}
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Unlock a file region.
 */
ساكن BOOL winUnlockFile(
  LPHANDLE phFile,
  DWORD offset_كس,
  DWORD offset_سم,
  DWORD لمب_Bytes_كس,
  DWORD لمب_Bytes_سم
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, حجم(OVERLAPPED));
  ovlp.Offset = offset_كس;
  ovlp.Offset_سم = offset_سم;
  أرجع osUnlockFileEx(*phFile, 0, لمب_Bytes_كس, لمب_Bytes_سم, &ovlp);
}

#ifndef SQLITE_OMIT_WAL
/*
** Remove an nByte lock starting at offset iOff from HANDLE h.
*/
ساكن صحيح winHandleUnlock(HANDLE h, صحيح iOff, صحيح nByte){
  BOOL ret = winUnlockFile(&h, iOff, 0, nByte, 0);
  أرجع (ret ? SQLITE_OK : SQLITE_IOERR_UNLOCK);
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
ساكن صحيح winHandleSeek(HANDLE h, sqlite3_int64 iOffset){
  صحيح rc = SQLITE_OK;             /* Return value */
  LARGE_INTEGER x;                /* The offset */

  x.QuadPart = iOffset;
  إذا( osSetFilePointerEx(h, x, 0, FILE_BEGIN)==0 ){
    rc = SQLITE_IOERR_SEEK;
  }
  OSTRACE(("SEEK file=%p, offset=%lld rc=%s\n", h, iOffset,sqlite3ErrName(rc)));
  أرجع rc;
}

/*
** Move the current position of the file handle passed as the first
** argument to offset iOffset within the file. If successful, return 0.
** Otherwise, set pFile->lastErrno and return non-zero.
*/
ساكن صحيح winSeekFile(winFile *pFile, sqlite3_int64 iOffset){
  صحيح rc;

  rc = winHandleSeek(pFile->h, iOffset);
  إذا( rc!=SQLITE_OK ){
    pFile->lastErrno = osGetLastError();
    winLogError(rc, pFile->lastErrno, "winSeekFile", pFile->zPath);
  }
  أرجع rc;
}


#إذا SQLITE_MAX_MMAP_SIZE>0
/* Forward references to VFS helper methods used for memory mapped files */
ساكن صحيح winMapfile(winFile*, sqlite3_int64);
ساكن صحيح winUnmapfile(winFile*);
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
ساكن صحيح winClose(sqlite3_file *id){
  صحيح rc, cnt = 0;
  winFile *pFile = (winFile*)id;

  أكد( id!=0 );
#ifndef SQLITE_OMIT_WAL
  أكد( pFile->pShm==0 );
#endif
  أكد( pFile->h!=عدم && pFile->h!=INVALID_HANDLE_درج );
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p\n",
           osGetCurrentProcessId(), pFile, pFile->h));

#إذا SQLITE_MAX_MMAP_SIZE>0
  winUnmapfile(pFile);
#endif

  افعل{
    rc = osCloseHandle(pFile->h);
    /* SimulateIOError( rc=0; cnt=MX_CLOSE_ATTEMPT; ); */
  }طالما( rc==0 && ++cnt < MX_CLOSE_ATTEMPT && (osSleep(100), 1) );
  إذا( rc ){
    pFile->h = عدم;
  }
  OpenCounter(-1);
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, rc ? "ok" : "failed"));
  أرجع rc ? SQLITE_OK
            : winLogError(SQLITE_IOERR_CLOSE, osGetLastError(),
                          "winClose", pFile->zPath);
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
ساكن صحيح winRead(
  sqlite3_file *id,          /* File to read from */
  فراغ *pBuf,                /* Write content into this buffer */
  صحيح amt,                   /* Number of bytes to read */
  sqlite3_int64 offset       /* Begin reading at this offset */
){
#إذا !defined(SQLITE_WIN32_NO_OVERLAPPED)
  OVERLAPPED overlapped;          /* The offset for ReadFile. */
#endif
  winFile *pFile = (winFile*)id;  /* file handle */
  DWORD nRead;                    /* Number of bytes actually read from file */
  صحيح nRetry = 0;                 /* Number of retrys */

  أكد( id!=0 );
  أكد( amt>0 );
  أكد( offset>=0 );
  SimulateIOError(أرجع SQLITE_IOERR_READ);
  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, offset, pFile->locktype));

#إذا SQLITE_MAX_MMAP_SIZE>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  إذا( offset<pFile->mmapSize ){
    إذا( offset+amt <= pFile->mmapSize ){
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], amt);
      OSTRACE(("READ-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      أرجع SQLITE_OK;
    }وإلا{
      صحيح nCopy = (صحيح)(pFile->mmapSize - offset);
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#endif

#إذا defined(SQLITE_WIN32_NO_OVERLAPPED)
  إذا( winSeekFile(pFile, offset) ){
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    أرجع SQLITE_FULL;
  }
  طالما( !osReadFile(pFile->h, pBuf, amt, &nRead, 0) ){
#وإلا
  memset(&overlapped, 0, حجم(OVERLAPPED));
  overlapped.Offset = (LONG)(offset & 0xffffffff);
  overlapped.Offset_سم = (LONG)((offset>>32) & 0x7fffffff);
  طالما( !osReadFile(pFile->h, pBuf, amt, &nRead, &overlapped) &&
         osGetLastError()!=ERROR_HANDLE_EOF ){
#endif
    DWORD lastErrno;
    إذا( winRetryIoerr(&nRetry, &lastErrno) ) استمر;
    pFile->lastErrno = lastErrno;
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    أرجع winLogError(SQLITE_IOERR_READ, pFile->lastErrno,
                       "winRead", pFile->zPath);
  }
  winLogIoerr(nRetry, __LINE__);
  إذا( nRead<(DWORD)amt ){
    /* Unread parts of the buffer must be zero-filled */
    memset(&((حرف*)pBuf)[nRead], 0, amt-nRead);
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_SHORT_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    أرجع SQLITE_IOERR_SHORT_READ;
  }

  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  أرجع SQLITE_OK;
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
ساكن صحيح winWrite(
  sqlite3_file *id,               /* File to write into */
  ثابت فراغ *pBuf,               /* The bytes to be written */
  صحيح amt,                        /* Number of bytes to write */
  sqlite3_int64 offset            /* Offset into the file to begin writing at */
){
  صحيح rc = 0;                     /* True if error has occurred, else false */
  winFile *pFile = (winFile*)id;  /* File handle */
  صحيح nRetry = 0;                 /* Number of retries */

  أكد( amt>0 );
  أكد( pFile );
  SimulateIOError(أرجع SQLITE_IOERR_WRITE);
  SimulateDiskfullError(أرجع SQLITE_FULL);

  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, offset, pFile->locktype));

#إذا defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_SIZE>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  إذا( offset<pFile->mmapSize ){
    إذا( offset+amt <= pFile->mmapSize ){
      memcpy(&((u8 *)(pFile->pMapRegion))[offset], pBuf, amt);
      OSTRACE(("WRITE-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      أرجع SQLITE_OK;
    }وإلا{
      صحيح nCopy = (صحيح)(pFile->mmapSize - offset);
      memcpy(&((u8 *)(pFile->pMapRegion))[offset], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#endif

#إذا defined(SQLITE_WIN32_NO_OVERLAPPED)
  rc = winSeekFile(pFile, offset);
  إذا( rc==0 ){
#وإلا
  {
#endif
#إذا !defined(SQLITE_WIN32_NO_OVERLAPPED)
    OVERLAPPED overlapped;        /* The offset for WriteFile. */
#endif
    u8 *aRem = (u8 *)pBuf;        /* Data yet to be written */
    صحيح nRem = amt;               /* Number of bytes yet to be written */
    DWORD nWrite;                 /* Bytes written by each WriteFile() call */
    DWORD lastErrno = NO_ERROR;   /* Value returned by GetLastError() */

#إذا !defined(SQLITE_WIN32_NO_OVERLAPPED)
    memset(&overlapped, 0, حجم(OVERLAPPED));
    overlapped.Offset = (LONG)(offset & 0xffffffff);
    overlapped.Offset_سم = (LONG)((offset>>32) & 0x7fffffff);
#endif

    طالما( nRem>0 ){
#إذا defined(SQLITE_WIN32_NO_OVERLAPPED)
      إذا( !osWriteFile(pFile->h, aRem, nRem, &nWrite, 0) ){
#وإلا
      إذا( !osWriteFile(pFile->h, aRem, nRem, &nWrite, &overlapped) ){
#endif
        إذا( winRetryIoerr(&nRetry, &lastErrno) ) استمر;
        اكسر;
      }
      أكد( nWrite==0 || nWrite<=(DWORD)nRem );
      إذا( nWrite==0 || nWrite>(DWORD)nRem ){
        lastErrno = osGetLastError();
        اكسر;
      }
#إذا !defined(SQLITE_WIN32_NO_OVERLAPPED)
      offset += nWrite;
      overlapped.Offset = (LONG)(offset & 0xffffffff);
      overlapped.Offset_سم = (LONG)((offset>>32) & 0x7fffffff);
#endif
      aRem += nWrite;
      nRem -= nWrite;
    }
    إذا( nRem>0 ){
      pFile->lastErrno = lastErrno;
      rc = 1;
    }
  }

  إذا( rc ){
    إذا(   ( pFile->lastErrno==ERROR_HANDLE_DISK_FULL )
       || ( pFile->lastErrno==ERROR_DISK_FULL )){
      OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      أرجع winLogError(SQLITE_FULL, pFile->lastErrno,
                         "winWrite1", pFile->zPath);
    }
    OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_WRITE\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    أرجع winLogError(SQLITE_IOERR_WRITE, pFile->lastErrno,
                       "winWrite2", pFile->zPath);
  }وإلا{
    winLogIoerr(nRetry, __LINE__);
  }
  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  أرجع SQLITE_OK;
}

#ifndef SQLITE_OMIT_WAL
/*
** Truncate the file opened by handle h to nByte bytes in size.
*/
ساكن صحيح winHandleTruncate(HANDLE h, sqlite3_int64 nByte){ 
  صحيح rc = SQLITE_OK;             /* Return code */
  rc = winHandleSeek(h, nByte);
  إذا( rc==SQLITE_OK ){
    إذا( 0==osSetEndOfFile(h) ){
      rc = SQLITE_IOERR_TRUNCATE;
    }
  }
  أرجع rc;
}

/*
** Determine the size in bytes of the file opened by the handle passed as 
** the first argument.
*/
ساكن صحيح winHandleSize(HANDLE h, sqlite3_int64 *pnByte){ 
  صحيح rc = SQLITE_OK;
  LARGE_INTEGER x;
  أكد( pnByte );
  إذا( osGetFileSizeEx(h, &x)==0 ){
    rc = SQLITE_IOERR_FSTAT;
    *pnByte = 0;
  }وإلا{
    *pnByte = x.QuadPart;
  }
  أرجع rc;
}

/*
** Close the handle passed as the only argument.
*/
ساكن فراغ winHandleClose(HANDLE h){
  إذا( h!=INVALID_HANDLE_درج ){
    osCloseHandle(h);
  }
}
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Truncate an open file to a specified size
*/
ساكن صحيح winTruncate(sqlite3_file *id, sqlite3_int64 nByte){
  winFile *pFile = (winFile*)id;  /* File handle object */
  صحيح rc = SQLITE_OK;             /* Return code for this function */
  DWORD lastErrno;
#إذا SQLITE_MAX_MMAP_SIZE>0
  sqlite3_int64 oldMmapSize;
  إذا( pFile->nFetchOut>0 ){
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
    أرجع SQLITE_OK;
  }
#endif

  أكد( pFile );
  SimulateIOError(أرجع SQLITE_IOERR_TRUNCATE);
  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, size=%lld, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, nByte, pFile->locktype));

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  إذا( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

#إذا SQLITE_MAX_MMAP_SIZE>0
  إذا( pFile->pMapRegion ){
    oldMmapSize = pFile->mmapSize;
  }وإلا{
    oldMmapSize = 0;
  }
  winUnmapfile(pFile);
#endif

  /* SetEndOfFile() returns non-zero when successful, or zero when it fails. */
  إذا( winSeekFile(pFile, nByte) ){
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->lastErrno,
                     "winTruncate1", pFile->zPath);
  }وإلا إذا( 0==osSetEndOfFile(pFile->h) &&
            ((lastErrno = osGetLastError())!=ERROR_USER_MAPPED_FILE) ){
    pFile->lastErrno = lastErrno;
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->lastErrno,
                     "winTruncate2", pFile->zPath);
  }

#إذا SQLITE_MAX_MMAP_SIZE>0
  إذا( rc==SQLITE_OK && oldMmapSize>0 ){
    إذا( oldMmapSize>nByte ){
      winMapfile(pFile, -1);
    }وإلا{
      winMapfile(pFile, oldMmapSize);
    }
  }
#endif

  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, sqlite3ErrName(rc)));
  أرجع rc;
}

#ifdef SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occurring at the right times.
*/
صحيح sqlite3_sync_كدي = 0;
صحيح sqlite3_fullsync_كدي = 0;
#endif

/*
** Make sure all writes to a particular file are committed to disk.
*/
ساكن صحيح winSync(sqlite3_file *id, صحيح flags){
#ifndef SQLITE_NO_SYNC
  /*
  ** Used only when SQLITE_NO_SYNC is not defined.
   */
  BOOL rc;
#endif
#إذا !defined(NDEBUG) || !defined(SQLITE_NO_SYNC) || \
    defined(SQLITE_HAVE_OS_TRACE)
  /*
  ** Used when SQLITE_NO_SYNC is not defined and by the assert() and/or
  ** OSTRACE() macros.
   */
  winFile *pFile = (winFile*)id;
#وإلا
  UNUSED_PARAMETER(id);
#endif

  أكد( pFile );
  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  أكد((flags&0x0F)==SQLITE_SYNC_NORMAL
      || (flags&0x0F)==SQLITE_SYNC_FULL
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( أرجع SQLITE_FULL );

  OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, flags=%x, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, flags,
           pFile->locktype));

#ifndef SQLITE_TEST
  UNUSED_PARAMETER(flags);
#وإلا
  إذا( (flags&0x0F)==SQLITE_SYNC_FULL ){
    sqlite3_fullsync_كدي++;
  }
  sqlite3_sync_كدي++;
#endif

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op
  */
#ifdef SQLITE_NO_SYNC
  OSTRACE(("SYNC-NOP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  أرجع SQLITE_OK;
#وإلا
#إذا SQLITE_MAX_MMAP_SIZE>0
  إذا( pFile->pMapRegion ){
    إذا( osFlushViewOfFile(pFile->pMapRegion, 0) ){
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_OK\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
    }وإلا{
      pFile->lastErrno = osGetLastError();
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
      أرجع winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winSync1", pFile->zPath);
    }
  }
#endif
  rc = osFlushFileBuffers(pFile->h);
  SimulateIOError( rc=FALSE );
  إذا( rc ){
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    أرجع SQLITE_OK;
  }وإلا{
    pFile->lastErrno = osGetLastError();
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_FSYNC\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    أرجع winLogError(SQLITE_IOERR_FSYNC, pFile->lastErrno,
                       "winSync2", pFile->zPath);
  }
#endif
}

/*
** Determine the current size of a file in bytes
*/
ساكن صحيح winFileSize(sqlite3_file *id, sqlite3_int64 *pSize){
  winFile *pFile = (winFile*)id;
  صحيح rc = SQLITE_OK;

  أكد( id!=0 );
  أكد( pSize!=0 );
  SimulateIOError(أرجع SQLITE_IOERR_FSTAT);
  OSTRACE(("SIZE file=%p, pSize=%p\n", pFile->h, pSize));
  {
    LARGE_INTEGER x;
    إذا( osGetFileSizeEx(pFile->h, &x)==0 ){
      *pSize = 0;
      pFile->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_FSTAT, pFile->lastErrno,
                       "winFileSize", pFile->zPath);
    }وإلا{
      *pSize = x.QuadPart;
    }
  }
  OSTRACE(("SIZE file=%p, pSize=%p, *pSize=%lld, rc=%s\n",
           pFile->h, pSize, *pSize, sqlite3ErrName(rc)));
  أرجع rc;
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
ساكن صحيح winGetReadLock(winFile *pFile, صحيح bBlock){
  صحيح res;
  DWORD mask = ~(bBlock ? LOCKFILE_FAIL_IMMEDIATELY : 0);
  OSTRACE(("READ-LOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winLockFile(&pFile->h, SQLITE_LOCKFILEEX_FLAGS&mask, SHARED_FIRST, 0,
                    SHARED_SIZE, 0);
  إذا( res == 0 ){
    pFile->lastErrno = osGetLastError();
    /* No need to log a failure to lock */
  }
  OSTRACE(("READ-LOCK file=%p, result=%d\n", pFile->h, res));
  أرجع res;
}

/*
** Undo a readlock
*/
ساكن صحيح winUnlockReadLock(winFile *pFile){
  صحيح res;
  DWORD lastErrno;
  OSTRACE(("READ-UNLOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winUnlockFile(&pFile->h, SHARED_FIRST, 0, SHARED_SIZE, 0);
  إذا( res==0 && ((lastErrno = osGetLastError())!=ERROR_NOT_LOCKED) ){
    pFile->lastErrno = lastErrno;
    winLogError(SQLITE_IOERR_UNLOCK, pFile->lastErrno,
                "winUnlockReadLock", pFile->zPath);
  }
  OSTRACE(("READ-UNLOCK file=%p, result=%d\n", pFile->h, res));
  أرجع res;
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
ساكن صحيح winLock(sqlite3_file *id, صحيح locktype){
  صحيح rc = SQLITE_OK;    /* Return code from subroutines */
  صحيح res = 1;           /* Result of a Windows lock call */
  صحيح newLocktype;       /* Set pFile->locktype to this value before exiting */
  صحيح gotPendingLock = 0;/* True if we acquired a PENDING lock this time */
  winFile *pFile = (winFile*)id;
  DWORD lastErrno = NO_ERROR;

  أكد( id!=0 );
  OSTRACE(("LOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->sharedLockByte, locktype));

  /* If there is already a lock of this type or more restrictive on the
  ** OsFile, do nothing. Don't use the end_lock: exit path, as
  ** sqlite3OsEnterMutex() hasn't been called yet.
  */
  إذا( pFile->locktype>=locktype ){
    OSTRACE(("LOCK-HELD file=%p, rc=SQLITE_OK\n", pFile->h));
    أرجع SQLITE_OK;
  }

  /* Do not allow any kind of write-lock on a read-only database
  */
  إذا( (pFile->ctrlFlags & WINFILE_RDONLY)!=0 && locktype>=RESERVED_LOCK ){
    أرجع SQLITE_IOERR_LOCK;
  }

  /* Make sure the locking sequence is correct
  */
  أكد( pFile->locktype!=NO_LOCK || locktype==SHARED_LOCK );
  أكد( locktype!=PENDING_LOCK );
  أكد( locktype!=RESERVED_LOCK || pFile->locktype==SHARED_LOCK );

  /* Lock the PENDING_LOCK byte if we need to acquire an EXCLUSIVE lock or
  ** a SHARED lock.  If we are acquiring a SHARED lock, the acquisition of
  ** the PENDING_LOCK byte is temporary.
  */
  newLocktype = pFile->locktype;
  إذا( locktype==SHARED_LOCK
   || (locktype==EXCLUSIVE_LOCK && pFile->locktype==RESERVED_LOCK)
  ){
    صحيح cnt = 3;

    /* Flags for the LockFileEx() call. This should be an exclusive lock if
    ** this call is to obtain EXCLUSIVE, or a shared lock if this call is to
    ** obtain SHARED.  */
    صحيح flags = LOCKFILE_FAIL_IMMEDIATELY;
    إذا( locktype==EXCLUSIVE_LOCK ){
      flags |= LOCKFILE_EXCLUSIVE_LOCK;
    }
    طالما( cnt>0 ){
      /* Try 3 times to get the pending lock.  This is needed to work
      ** around problems caused by indexing and/or anti-virus software on
      ** Windows systems.
      **
      ** If you are using this code as a model for alternative VFSes, do not
      ** copy this retry logic.  It is a hack intended for Windows only.  */
      res = winLockFile(&pFile->h, flags, PENDING_BYTE, 0, 1, 0);
      إذا( res ) اكسر;

      lastErrno = osGetLastError();
      OSTRACE(("LOCK-PENDING-FAIL file=%p, count=%d, result=%d\n", 
            pFile->h, cnt, res
      ));

      إذا( lastErrno==ERROR_INVALID_HANDLE ){
        pFile->lastErrno = lastErrno;
        rc = SQLITE_IOERR_LOCK;
        OSTRACE(("LOCK-FAIL file=%p, count=%d, rc=%s\n", 
              pFile->h, cnt, sqlite3ErrName(rc)
        ));
        أرجع rc;
      }

      cnt--;
      إذا( cnt>0 ) osSleep(1);
    }
    gotPendingLock = res;
  }

  /* Acquire a shared lock
  */
  إذا( locktype==SHARED_LOCK && res ){
    أكد( pFile->locktype==NO_LOCK );
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    res = winGetReadLock(pFile, pFile->bBlockOnConnect);
#وإلا
    res = winGetReadLock(pFile, 0);
#endif
    إذا( res ){
      newLocktype = SHARED_LOCK;
    }وإلا{
      lastErrno = osGetLastError();
    }
  }

  /* Acquire a RESERVED lock
  */
  إذا( locktype==RESERVED_LOCK && res ){
    أكد( pFile->locktype==SHARED_LOCK );
    res = winLockFile(&pFile->h, SQLITE_LOCKFILE_FLAGS, RESERVED_BYTE, 0, 1, 0);
    إذا( res ){
      newLocktype = RESERVED_LOCK;
    }وإلا{
      lastErrno = osGetLastError();
    }
  }

  /* Acquire a PENDING lock
  */
  إذا( locktype==EXCLUSIVE_LOCK && res ){
    newLocktype = PENDING_LOCK;
    gotPendingLock = 0;
  }

  /* Acquire an EXCLUSIVE lock
  */
  إذا( locktype==EXCLUSIVE_LOCK && res ){
    أكد( pFile->locktype>=SHARED_LOCK );
    (فراغ)winUnlockReadLock(pFile);
    res = winLockFile(&pFile->h, SQLITE_LOCKFILE_FLAGS, SHARED_FIRST, 0,
                      SHARED_SIZE, 0);
    إذا( res ){
      newLocktype = EXCLUSIVE_LOCK;
    }وإلا{
      lastErrno = osGetLastError();
      winGetReadLock(pFile, 0);
    }
  }

  /* If we are holding a PENDING lock that ought to be released, then
  ** release it now.
  */
  إذا( gotPendingLock && locktype==SHARED_LOCK ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }

  /* Update the state of the lock has held in the file descriptor then
  ** return the appropriate result code.
  */
  إذا( res ){
    rc = SQLITE_OK;
  }وإلا{
    pFile->lastErrno = lastErrno;
    rc = SQLITE_BUSY;
    OSTRACE(("LOCK-FAIL file=%p, wanted=%d, got=%d\n",
             pFile->h, locktype, newLocktype));
  }
  pFile->locktype = (u8)newLocktype;
  OSTRACE(("LOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  أرجع rc;
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, return
** non-zero, otherwise zero.
*/
ساكن صحيح winCheckReservedLock(sqlite3_file *id, صحيح *pResOut){
  صحيح res;
  winFile *pFile = (winFile*)id;

  SimulateIOError( أرجع SQLITE_IOERR_CHECKRESERVEDLOCK; );
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p\n", pFile->h, pResOut));

  أكد( id!=0 );
  إذا( pFile->locktype>=RESERVED_LOCK ){
    res = 1;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (local)\n", pFile->h, res));
  }وإلا{
    res = winLockFile(&pFile->h, SQLITE_LOCKFILEEX_FLAGS,RESERVED_BYTE,0,1,0);
    إذا( res ){
      winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
    }
    res = !res;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (remote)\n", pFile->h, res));
  }
  *pResOut = res;
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           pFile->h, pResOut, *pResOut));
  أرجع SQLITE_OK;
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
ساكن صحيح winUnlock(sqlite3_file *id, صحيح locktype){
  صحيح type;
  winFile *pFile = (winFile*)id;
  صحيح rc = SQLITE_OK;
  أكد( pFile!=0 );
  أكد( locktype<=SHARED_LOCK );
  OSTRACE(("UNLOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->sharedLockByte, locktype));
  type = pFile->locktype;
  إذا( type>=EXCLUSIVE_LOCK ){
    winUnlockFile(&pFile->h, SHARED_FIRST, 0, SHARED_SIZE, 0);
    إذا( locktype==SHARED_LOCK && !winGetReadLock(pFile, 0) ){
      /* This should never happen.  We should always be able to
      ** reacquire the read lock */
      rc = winLogError(SQLITE_IOERR_UNLOCK, osGetLastError(),
                       "winUnlock", pFile->zPath);
    }
  }
  إذا( type>=RESERVED_LOCK ){
    winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
  }
  إذا( locktype==NO_LOCK && type>=SHARED_LOCK ){
    winUnlockReadLock(pFile);
  }
  إذا( type>=PENDING_LOCK ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }
  pFile->locktype = (u8)locktype;
  OSTRACE(("UNLOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  أرجع rc;
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

ساكن صحيح winNolockLock(sqlite3_file *id, صحيح locktype){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(locktype);
  أرجع SQLITE_OK;
}

ساكن صحيح winNolockCheckReservedLock(sqlite3_file *id, صحيح *pResOut){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(pResOut);
  أرجع SQLITE_OK;
}

ساكن صحيح winNolockUnlock(sqlite3_file *id, صحيح locktype){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(locktype);
  أرجع SQLITE_OK;
}

/******************* End of the no-op lock implementation *********************
******************************************************************************/

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
ساكن فراغ winModeBit(winFile *pFile, unsigned حرف mask, صحيح *pArg){
  إذا( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & mask)!=0;
  }وإلا إذا( (*pArg)==0 ){
    pFile->ctrlFlags &= ~mask;
  }وإلا{
    pFile->ctrlFlags |= mask;
  }
}

/* Forward references to VFS helper methods used for temporary files */
ساكن صحيح winGetTempname(sqlite3_vfs *, حرف **);
ساكن صحيح winIsDir(LPCWSTR);
ساكن BOOL winIsLongPathPrefix(ثابت حرف *);
ساكن BOOL winIsDriveLetterAndColon(ثابت حرف *);

/*
** Control and query of the open file handle.
*/
ساكن صحيح winFileControl(sqlite3_file *id, صحيح op, فراغ *pArg){
  winFile *pFile = (winFile*)id;
  OSTRACE(("FCNTL file=%p, op=%d, pArg=%p\n", pFile->h, op, pArg));
  بدل( op ){
    حالة SQLITE_FCNTL_LOCKSTATE: {
      *(صحيح*)pArg = pFile->locktype;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      أرجع SQLITE_OK;
    }
    حالة SQLITE_FCNTL_LAST_ERRNO: {
      *(صحيح*)pArg = (صحيح)pFile->lastErrno;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      أرجع SQLITE_OK;
    }
    حالة SQLITE_FCNTL_CHUNK_SIZE: {
      pFile->szChunk = *(صحيح *)pArg;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      أرجع SQLITE_OK;
    }
    حالة SQLITE_FCNTL_SIZE_HINT: {
      إذا( pFile->szChunk>0 ){
        sqlite3_int64 oldSz;
        صحيح rc = winFileSize(id, &oldSz);
        إذا( rc==SQLITE_OK ){
          sqlite3_int64 newSz = *(sqlite3_int64*)pArg;
          إذا( newSz>oldSz ){
            SimulateIOErrorBenign(1);
            rc = winTruncate(id, newSz);
            SimulateIOErrorBenign(0);
          }
        }
        OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
        أرجع rc;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      أرجع SQLITE_OK;
    }
    حالة SQLITE_FCNTL_PERSIST_WAL: {
      winModeBit(pFile, WINFILE_PERSIST_WAL, (صحيح*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      أرجع SQLITE_OK;
    }
    حالة SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      winModeBit(pFile, WINFILE_PSOW, (صحيح*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      أرجع SQLITE_OK;
    }
    حالة SQLITE_FCNTL_VFSNAME: {
      *(حرف**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      أرجع SQLITE_OK;
    }
    حالة SQLITE_FCNTL_WIN32_AV_RETRY: {
      صحيح *a = (صحيح*)pArg;
      إذا( a[0]>0 ){
        winIoerrRetry = a[0];
      }وإلا{
        a[0] = winIoerrRetry;
      }
      إذا( a[1]>0 ){
        winIoerrRetryDelay = a[1];
      }وإلا{
        a[1] = winIoerrRetryDelay;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      أرجع SQLITE_OK;
    }
    حالة SQLITE_FCNTL_WIN32_GET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      *phFile = pFile->h;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      أرجع SQLITE_OK;
    }
#ifdef SQLITE_TEST
    حالة SQLITE_FCNTL_WIN32_SET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      HANDLE hOldFile = pFile->h;
      pFile->h = *phFile;
      *phFile = hOldFile;
      OSTRACE(("FCNTL oldFile=%p, newFile=%p, rc=SQLITE_OK\n",
               hOldFile, pFile->h));
      أرجع SQLITE_OK;
    }
#endif
    حالة SQLITE_FCNTL_NULL_IO: {
      (فراغ)osCloseHandle(pFile->h);
      pFile->h = عدم;
      أرجع SQLITE_OK;
    }
    حالة SQLITE_FCNTL_TEMPFILENAME: {
      حرف *zTFile = 0;
      صحيح rc = winGetTempname(pFile->pVfs, &zTFile);
      إذا( rc==SQLITE_OK ){
        *(حرف**)pArg = zTFile;
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      أرجع rc;
    }
#إذا SQLITE_MAX_MMAP_SIZE>0
    حالة SQLITE_FCNTL_MMAP_SIZE: {
      i64 newLimit = *(i64*)pArg;
      صحيح rc = SQLITE_OK;
      إذا( newLimit>sqlite3GlobalConfig.mxMmap ){
        newLimit = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (SIZE_T) and passed
      ** to MapViewOfFile(). Restrict its value to 2GB if (SIZE_T) is not at
      ** least a 64-bit type. */
      إذا( newLimit>0 && حجم(SIZE_T)<8 ){
        newLimit = (newLimit & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmapSizeMax;
      إذا( newLimit>=0 && newLimit!=pFile->mmapSizeMax && pFile->nFetchOut==0 ){
        pFile->mmapSizeMax = newLimit;
        إذا( pFile->mmapSize>0 ){
          winUnmapfile(pFile);
          rc = winMapfile(pFile, -1);
        }
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      أرجع rc;
    }
#endif

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    حالة SQLITE_FCNTL_LOCK_TIMEOUT: {
      صحيح iOld = pFile->iBusyTimeout;
      صحيح iNew = *(صحيح*)pArg;
#إذا SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = (iNew < 0) ? INFINITE : (DWORD)iNew;
#وإلاإذا SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = (DWORD)(!!iNew);
#وإلا
# error "SQLITE_ENABLE_SETLK_TIMEOUT must be set to 1 or 2"
#endif
      *(صحيح*)pArg = iOld;
      أرجع SQLITE_OK;
    }
    حالة SQLITE_FCNTL_BLOCK_ON_CONNECT: {
      صحيح iNew = *(صحيح*)pArg;
      pFile->bBlockOnConnect = iNew;
      أرجع SQLITE_OK;
    }
#endif /* SQLITE_ENABLE_SETLK_TIMEOUT */

#إذا defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    حالة SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      sqlite3_str_appendf(pStr, "{\"h\":%llu", (sqlite3_uint64)pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      إذا( pFile->locktype ){
        ساكن ثابت حرف *azLock[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
        sqlite3_str_appendf(pStr, ",\"locktype\":\"%s\"", 
                                  azLock[pFile->locktype-1]);
      }
#إذا SQLITE_MAX_MMAP_SIZE>0
      إذا( pFile->mmapSize ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmapSize);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->nFetchOut);
      }
#endif
      sqlite3_str_append(pStr, "}", 1);
      أرجع SQLITE_OK;
    }
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

  }
  OSTRACE(("FCNTL file=%p, rc=SQLITE_NOTFOUND\n", pFile->h));
  أرجع SQLITE_NOTFOUND;
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
ساكن صحيح winSectorSize(sqlite3_file *id){
  (فراغ)id;
  أرجع SQLITE_DEFAULT_SECTOR_SIZE;
}

/*
** Return a vector of device characteristics.
*/
ساكن صحيح winDeviceCharacteristics(sqlite3_file *id){
  winFile *p = (winFile*)id;
  أرجع SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN | SQLITE_IOCAP_SUBPAGE_READ |
         ((p->ctrlFlags & WINFILE_PSOW)?SQLITE_IOCAP_POWERSAFE_OVERWRITE:0);
}

/*
** Windows will only let you create file view mappings
** on allocation size granularity boundaries.
** During sqlite3_os_init() we do a GetSystemInfo()
** to get the granularity size.
*/
ساكن SYSTEM_INFO winSysInfo;

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
ساكن LPWSTR winConvertFromUtf8Filename(ثابت حرف *zFilename){
  LPWSTR zConverted = 0;
#ifdef __CYGWIN__
  صحيح nChar;

  إذا( osCygwin_conv_path && !(winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2])) ){
    i64 nByte;
    صحيح convertflag = CCP_POSIX_TO_WIN_W;
    إذا( !strchr(zFilename, '/') ) convertflag |= CCP_RELATIVE;
    nByte = (i64)osCygwin_conv_path(convertflag,
        zFilename, 0, 0);
    إذا( nByte>0 ){
      zConverted = sqlite3MallocZero(12+(u64)nByte);
      إذا ( zConverted==0 ){
        أرجع zConverted;
      }
      /* Filenames should be prefixed, except when converted
       * full path already starts with "\\?\". */
      إذا( osCygwin_conv_path(convertflag, zFilename,
                           zConverted+4, nByte)==0 ){
        إذا( (convertflag&CCP_RELATIVE) ){
          memmove(zConverted, zConverted+4, nByte);
        }وإلا إذا( memcmp(zConverted+4, L"\\\\", 4) ){
          memcpy(zConverted, L"\\\\?\\", 8);
        }وإلا إذا( zConverted[6]!='?' ){
          memmove(zConverted+6, zConverted+4, nByte);
          memcpy(zConverted, L"\\\\?\\UNC", 14);
        }وإلا{
          memmove(zConverted, zConverted+4, nByte);
        }
        أرجع zConverted;
      }
      sqlite3_free(zConverted);
    }
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zFilename, -1, عدم, 0);
  إذا( nChar==0 ){
    أرجع 0;
  }
  zConverted = sqlite3MallocZero( ((u64)nChar+6)*حجم(WCHAR) );
  إذا( zConverted==0 ){
    أرجع 0;
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zFilename, -1,
                                zConverted, nChar);
  إذا( nChar==0 ){
    sqlite3_free(zConverted);
    zConverted = 0;
  }وإلا إذا( nChar>MAX_PATH
      && winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2]) ){
    memmove(zConverted+4, zConverted, nChar*حجم(WCHAR));
    zConverted[2] = '\\';
    memcpy(zConverted, L"\\\\?\\", 8);
  }وإلا إذا( nChar>MAX_PATH
      && winIsDirSep(zFilename[0]) && winIsDirSep(zFilename[1])
      && zFilename[2] != '?' ){
    memmove(zConverted+6, zConverted, nChar*حجم(WCHAR));
    memcpy(zConverted, L"\\\\?\\UNC", 14);
  }
#وإلا /* if !defined(__CYGWIN__) */
  zConverted = winUtf8ToUnicode(zFilename);
#endif /* __CYGWIN__ */
  /* caller will handle out of memory */
  أرجع zConverted;
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
ساكن sqlite3_mutex *winBigLock = 0;
ساكن فراغ winShmEnterMutex(فراغ){
  sqlite3_mutex_enter(winBigLock);
}
ساكن فراغ winShmLeaveMutex(فراغ){
  sqlite3_mutex_leave(winBigLock);
}
#ifndef NDEBUG
ساكن صحيح winShmMutexHeld(فراغ) {
  أرجع sqlite3_mutex_held(winBigLock);
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
بنية win_Shm_كلي {
  sqlite3_mutex *mutex;      /* Mutex to access this object */
  حرف *zFilename;           /* Name of the file */
  HANDLE hSharedShm;         /* File handle open on zFilename */
  صحيح bUseSharedLockHandle;  /* True to use hSharedShm for everything */

  صحيح isUnlocked;            /* DMS lock has not yet been obtained */
  صحيح isReadonly;            /* True if read-only */
  صحيح szRegion;              /* Size of shared-memory regions */
  صحيح nRegion;               /* Size of array apRegion */

  بنية ShmRegion {
    HANDLE hMap;             /* File handle from CreateFileMapping */
    فراغ *pMap;
  } *aRegion;
  DWORD lastErrno;           /* The Windows errno from the last I/O error */

  winShm *pWinShmList;       /* List of winShm objects with ptrs to this */

  win_Shm_كلي *pNext;         /* Next in list of all winShmNode objects */
#إذا defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 nextShmId;              /* Next available winShm.id value */
#endif
};

/*
** A global array of all winShmNode objects.
**
** The winShmMutexHeld() must be true while reading or writing this list.
*/
ساكن win_Shm_كلي *win_Shm_كلي_List = 0;

/*
** Structure used internally by this VFS to record the state of an
** open shared memory connection. There is one such structure for each
** winFile open on a wal mode database.
*/
بنية winShm {
  win_Shm_كلي *p_Shm_كلي;      /* The underlying winShmNode object */
  u16 sharedMask;            /* Mask of shared locks held */
  u16 exclMask;              /* Mask of exclusive locks held */
  HANDLE hShm;               /* File-handle on *-shm file. For locking. */
  صحيح bReadonly;             /* True if hShm is opened read-only */
#إذا defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
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
ساكن صحيح winOpen(sqlite3_vfs*,ثابت حرف*,sqlite3_file*,صحيح,صحيح*);
ساكن صحيح winDelete(sqlite3_vfs *,ثابت حرف*,صحيح);

/*
** Purge the winShmNodeList list of all entries with winShmNode.pWinShmList==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
ساكن فراغ winShmPurge(sqlite3_vfs *pVfs, صحيح deleteFlag){
  win_Shm_كلي **pp;
  win_Shm_كلي *p;
  أكد( winShmMutexHeld() );
  OSTRACE(("SHM-PURGE pid=%lu, deleteFlag=%d\n",
           osGetCurrentProcessId(), deleteFlag));
  pp = &win_Shm_كلي_List;
  طالما( (p = *pp)!=0 ){
    إذا( p->pWinShmList==0 ){
      صحيح i;
      إذا( p->mutex ){ sqlite3_mutex_free(p->mutex); }
      لكل(i=0; i<p->nRegion; i++){
        BOOL bRc = osUnmapViewOfFile(p->aRegion[i].pMap);
        OSTRACE(("SHM-PURGE-UNMAP pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        UNUSED_VARIABLE_درج(bRc);
        bRc = osCloseHandle(p->aRegion[i].hMap);
        OSTRACE(("SHM-PURGE-CLOSE pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        UNUSED_VARIABLE_درج(bRc);
      }
      winHandleClose(p->hSharedShm);
      إذا( deleteFlag ){
        SimulateIOErrorBenign(1);
        sqlite3BeginBenignMalloc();
        winDelete(pVfs, p->zFilename, 0);
        sqlite3EndBenignMalloc();
        SimulateIOErrorBenign(0);
      }
      *pp = p->pNext;
      sqlite3_free(p->aRegion);
      sqlite3_free(p);
    }وإلا{
      pp = &p->pNext;
    }
  }
}

/*
** Handle h is open on the *-shm file. This function zeroes the first
** 8 bytes of the file. If SQLITE_ENABLE_SETLK_TIMEOUT is defined, h may 
** have been opened with FILE_FLAG_OVERLAPPED.
*/
ساكن صحيح winZeroSharedMemory(HANDLE h){
  u8 aZero[8] = {0,0,0,0,0,0,0,0};
  DWORD nWrite = 0;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, حجم(OVERLAPPED));
  إذا( !osWriteFile(h, aZero, حجم(aZero), &nWrite, &ovlp) ){
    /* If the file was opened with FILE_FLAG_OVERLAPPED, the write may still
    ** be pending. Wait for it with GetOverlappedResult().  */
    إذا( osGetLastError()!=ERROR_IO_PENDING
     || !GetOverlappedResult(h, &ovlp, &nWrite, TRUE)
    ){
      nWrite = 0;
    }
  }
  أرجع (nWrite==حجم(aZero) ? SQLITE_OK : SQLITE_IOERR_WRITE);
}

/*
** The DMS lock has not yet been taken on the shm file associated with
** pShmNode. Take the lock. Truncate the *-shm file if required.
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
ساكن صحيح winLockSharedMemory(win_Shm_كلي *p_Shm_كلي, DWORD nMs){
  HANDLE h = p_Shm_كلي->hSharedShm;
  صحيح rc = SQLITE_OK;

  أكد( sqlite3_mutex_held(p_Shm_كلي->mutex) );
  rc = winHandleLockTimeout(h, WIN_SHM_DMS, 1, 1, 0);
  إذا( rc==SQLITE_OK ){
    /* We have an EXCLUSIVE lock on the DMS byte. This means that this
    ** is the first process to open the file. Truncate it to zero bytes
    ** in this case.  */
    إذا( p_Shm_كلي->isReadonly ){
      rc = SQLITE_READONLY_CANTINIT;
    }وإلا{
      إذا( SQLITE_OK!=winHandleTruncate(h, 0) ){
        /* Sometimes the truncate operation may fail because some other
        ** process is still holding an open mapping. So try to write
        ** zeroes into the start of the file instead. */
        rc = winZeroSharedMemory(h);
      }
    }

    /* Release the EXCLUSIVE lock acquired above. */
    winUnlockFile(&h, WIN_SHM_DMS, 0, 1, 0);
  }وإلا إذا( (rc & 0xFF)==SQLITE_BUSY ){
    rc = SQLITE_OK;
  }

  إذا( rc==SQLITE_OK ){
    /* Take a SHARED lock on the DMS byte. */
    rc = winHandleLockTimeout(h, WIN_SHM_DMS, 1, 0, nMs);
    إذا( rc==SQLITE_OK ){
      p_Shm_كلي->isUnlocked = 0;
    }
  }

  أرجع rc;
}


/*
** This function is used to open a handle on a *-shm file.
**
** If SQLITE_ENABLE_SETLK_TIMEOUT is defined at build time, then the file
** is opened with FILE_FLAG_OVERLAPPED specified. If not, it is not.
*/
ساكن صحيح winHandleOpen(
  ثابت حرف *zUtf8,              /* File to open */
  صحيح *pbReadonly,                /* IN/OUT: True for readonly handle */
  HANDLE *ph                      /* OUT: New HANDLE for file */
){
  صحيح rc = SQLITE_OK;
  LPWSTR zConverted = 0;
  صحيح bReadonly = *pbReadonly;
  HANDLE h = INVALID_HANDLE_درج;

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  ثابت DWORD flag_overlapped = FILE_FLAG_OVERLAPPED;
#وإلا
  ثابت DWORD flag_overlapped = 0;
#endif

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8);
  إذا( zConverted==0 ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8));
    rc = SQLITE_IOERR_NOMEM_BKPT;
    اذهب winopenfile_out;
  }

  /* Ensure the file we are trying to open is not actually a directory. */
  إذا( winIsDir(zConverted) ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8));
    rc = SQLITE_CANTOPEN_ISDIR;
    اذهب winopenfile_out;
  }

  /* TODO: platforms.
  ** TODO: retry-on-ioerr.
  */
  h = osCreateFileW(zConverted,         /* lpFileName */
      (GENERIC_READ | (bReadonly ? 0 : GENERIC_WRITE)),  /* dwDesiredAccess */
      FILE_SHARE_READ | FILE_SHARE_WRITE,        /* dwShareMode */
      عدم,                                      /* lpSecurityAttributes */
      OPEN_ALWAYS,                               /* dwCreationDisposition */
      FILE_ATTRIBUTE_NORMAL|flag_overlapped,
      عدم
  );

  إذا( h==INVALID_HANDLE_درج ){
    إذا( bReadonly==0 ){
      bReadonly = 1;
      rc = winHandleOpen(zUtf8, &bReadonly, &h);
    }وإلا{
      rc = SQLITE_CANTOPEN_BKPT;
    }
  }

 winopenfile_out:
  sqlite3_free(zConverted);
  *pbReadonly = bReadonly;
  *ph = h;
  أرجع rc;
}
 
/*
** Close pDbFd's connection to shared-memory.  Delete the underlying
** *-shm file if deleteFlag is true.
*/
ساكن صحيح winCloseSharedMemory(winFile *pDbFd, صحيح deleteFlag){
  winShm *p;            /* The connection to be closed */
  winShm **pp;          /* Iterator for pShmNode->pWinShmList */
  win_Shm_كلي *p_Shm_كلي; /* The underlying shared-memory file */

  p = pDbFd->pShm;
  إذا( p==0 ) أرجع SQLITE_OK;
  إذا( p->hShm!=INVALID_HANDLE_درج ){
    osCloseHandle(p->hShm);
  }

  winShmEnterMutex();
  p_Shm_كلي = p->p_Shm_كلي;

  /* Remove this connection from the winShmNode.pWinShmList list */
  sqlite3_mutex_enter(p_Shm_كلي->mutex);
  لكل(pp=&p_Shm_كلي->pWinShmList; *pp!=p; pp=&(*pp)->pWinShmNext){}
  *pp = p->pWinShmNext;
  sqlite3_mutex_leave(p_Shm_كلي->mutex);

  winShmPurge(pDbFd->pVfs, deleteFlag);
  winShmLeaveMutex();

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  أرجع SQLITE_OK;
}

/*
** testfixture builds may set this global variable to true via a
** Tcl interface. This forces the VFS to use the locking normally
** only used for UNC paths for all files.
*/
#ifdef SQLITE_TEST
صحيح sqlite3_win_test_unc_locking = 0;
#وإلا
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
ساكن صحيح winIsUNCPath(ثابت حرف *zFile){
  إذا( zFile[0]=='\\' && zFile[1]=='\\' ){
    إذا( zFile[2]=='?'
     && zFile[3]=='\\'
     && sqlite3Isalpha(zFile[4])
     && zFile[5]==':'
     && winIsDirSep(zFile[6])
    ){
      أرجع sqlite3_win_test_unc_locking;
    }وإلا{
      أرجع 1;
    }
  }
  أرجع sqlite3_win_test_unc_locking;
}

/*
** Return true if the string passed in is the name of a file on a
** remote (network-mounted) volume.
*/
ساكن صحيح winIsRemoteVolume(ثابت حرف *zFile){
  WCHAR z_سيو[4];
  إذا( strncmp(zFile, "\\\\?\\",4)==0 ) zFile += 4;
  إذا( !sqlite3Isalpha(zFile[0]) ) أرجع 0;
  إذا( zFile[1]!=':' ) أرجع 0;
  إذا( !winIsDirSep(zFile[2]) ) أرجع 0;
  z_سيو[0] = (WCHAR)zFile[0];
  z_سيو[1] = ':';
  z_سيو[2] = '\\';
  z_سيو[3] = 0;
  أرجع osGetDriveTypeW(z_سيو)==DRIVE_REMOTE;
}

/*
** Open the shared-memory area associated with database file pDbFd.
*/
ساكن صحيح winOpenSharedMemory(winFile *pDbFd){
  بنية winShm *p;                  /* The connection to be opened */
  win_Shm_كلي *p_Shm_كلي = 0;          /* The underlying mmapped file */
  صحيح rc = SQLITE_OK;                /* Result code */
  win_Shm_كلي *pNew;                  /* Newly allocated winShmNode */
  صحيح nName;                         /* Size of zName in bytes */

  أكد( pDbFd->pShm==0 );    /* Not previously opened */

  /* Allocate space for the new sqlite3_shm object.  Also speculatively
  ** allocate space for a new winShmNode and filename.  */
  p = sqlite3MallocZero( حجم(*p) );
  إذا( p==0 ) أرجع SQLITE_IOERR_NOMEM_BKPT;
  nName = sqlite3Strlen30(pDbFd->zPath);
  pNew = sqlite3MallocZero( حجم(*p_Shm_كلي) + (i64)nName + 17 );
  إذا( pNew==0 ){
    sqlite3_free(p);
    أرجع SQLITE_IOERR_NOMEM_BKPT;
  }
  pNew->zFilename = (حرف*)&pNew[1];
  pNew->hSharedShm = INVALID_HANDLE_درج;
  pNew->isUnlocked = 1;
  إذا( winIsUNCPath(pDbFd->zPath) || winIsRemoteVolume(pDbFd->zPath) ){
    pNew->bUseSharedLockHandle = 1;
  }وإلا{
    pNew->bUseSharedLockHandle = 0;
  }
  sqlite3_snprintf(nName+15, pNew->zFilename, "%s-shm", pDbFd->zPath);
  sqlite3FileSuffix3(pDbFd->zPath, pNew->zFilename);

  /* Look to see if there is an existing winShmNode that can be used.
  ** If no matching winShmNode currently exists, then create a new one.  */
  winShmEnterMutex();
  لكل(p_Shm_كلي = win_Shm_كلي_List; p_Shm_كلي; p_Shm_كلي=p_Shm_كلي->pNext){
    /* TBD need to come up with better match here.  Perhaps
    ** use FILE_ID_BOTH_DIR_INFO Structure.  */
    إذا( sqlite3StrICmp(p_Shm_كلي->zFilename, pNew->zFilename)==0 ) اكسر;
  }
  إذا( p_Shm_كلي==0 ){
    p_Shm_كلي = pNew;

    /* Allocate a mutex for this winShmNode object, if one is required. */
    إذا( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_كلي->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      إذا( p_Shm_كلي->mutex==0 ) rc = SQLITE_IOERR_NOMEM_BKPT;
    }

    /* Open a file-handle to use for mappings, and for the DMS lock. */
    إذا( rc==SQLITE_OK ){
      HANDLE h = INVALID_HANDLE_درج;
      p_Shm_كلي->isReadonly = sqlite3_uri_boolean(pDbFd->zPath,"readonly_shm",0);
      rc = winHandleOpen(pNew->zFilename, &p_Shm_كلي->isReadonly, &h);
      p_Shm_كلي->hSharedShm = h;
    }

    /* If successful, link the new winShmNode into the global list. If an 
    ** error occurred, free the object. */
    إذا( rc==SQLITE_OK ){
      p_Shm_كلي->pNext = win_Shm_كلي_List;
      win_Shm_كلي_List = p_Shm_كلي;
      pNew = 0;
    }وإلا{
      sqlite3_mutex_free(p_Shm_كلي->mutex);
      إذا( p_Shm_كلي->hSharedShm!=INVALID_HANDLE_درج ){
        osCloseHandle(p_Shm_كلي->hSharedShm);
      }
    }
  }

  /* If no error has occurred, link the winShm object to the winShmNode and
  ** the winShm to pDbFd.  */
  إذا( rc==SQLITE_OK ){
    sqlite3_mutex_enter(p_Shm_كلي->mutex);
    p->p_Shm_كلي = p_Shm_كلي;
    p->pWinShmNext = p_Shm_كلي->pWinShmList;
    p_Shm_كلي->pWinShmList = p;
#إذا defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
    p->id = p_Shm_كلي->nextShmId++;
#endif
    pDbFd->pShm = p;
    sqlite3_mutex_leave(p_Shm_كلي->mutex);
  }وإلا إذا( p ){
    sqlite3_free(p);
  }

  أكد( rc!=SQLITE_OK || p_Shm_كلي->isUnlocked==0 || p_Shm_كلي->nRegion==0 );
  winShmLeaveMutex();
  sqlite3_free(pNew);

  /* Open a file-handle on the *-shm file for this connection. This file-handle
  ** is only used for locking. The mapping of the *-shm file is created using
  ** the shared file handle in winShmNode.hSharedShm.  */
  إذا( rc==SQLITE_OK && p_Shm_كلي->bUseSharedLockHandle==0 ){
    p->bReadonly = sqlite3_uri_boolean(pDbFd->zPath, "readonly_shm", 0);
    rc = winHandleOpen(p_Shm_كلي->zFilename, &p->bReadonly, &p->hShm);
    إذا( rc!=SQLITE_OK ){
      أكد( p->hShm==INVALID_HANDLE_درج );
      winCloseSharedMemory(pDbFd, 0);
    }
  }

  أرجع rc;
}

/*
** Close a connection to shared-memory.  Delete the underlying
** storage if deleteFlag is true.
*/
ساكن صحيح winShmUnmap(
  sqlite3_file *fd,          /* Database holding shared memory */
  صحيح deleteFlag             /* Delete after closing if true */
){
  أرجع winCloseSharedMemory((winFile*)fd, deleteFlag);
}

/*
** Change the lock state for a shared-memory segment.
*/
ساكن صحيح winShmLock(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  صحيح ofst,                  /* First lock to acquire or release */
  صحيح n,                     /* Number of locks to acquire or release */
  صحيح flags                  /* What to do with the lock */
){
  winFile *pDbFd = (winFile*)fd;        /* Connection holding shared memory */
  winShm *p = pDbFd->pShm;              /* The shared memory being locked */
  win_Shm_كلي *p_Shm_كلي;
  صحيح rc = SQLITE_OK;                   /* Result code */
  u16 mask = (u16)((1U<<(ofst+n)) - (1U<<ofst)); /* Mask of locks to [un]take */

  إذا( p==0 ) أرجع SQLITE_IOERR_SHMLOCK;
  p_Shm_كلي = p->p_Shm_كلي;
  إذا( NEVER(p_Shm_كلي==0) ) أرجع SQLITE_IOERR_SHMLOCK;

  أكد( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  أكد( n>=1 );
  أكد( flags==(SQLITE_SHM_LOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_LOCK | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  أكد( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );

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
#إذا defined(SQLITE_ENABLE_SETLK_TIMEOUT) && defined(SQLITE_DEBUG)
  {
    u16 lockMask = (p->exclMask|p->sharedMask);
    أكد( (flags & SQLITE_SHM_UNLOCK) || pDbFd->iBusyTimeout==0 || (
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
  أكد( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK) 
       || 0==(p->exclMask & mask)
  );
  إذا( ((flags & SQLITE_SHM_UNLOCK) && ((p->exclMask|p->sharedMask) & mask))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_LOCK) && 0==(p->sharedMask & mask))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK))
  ){
    HANDLE h = p->hShm;

    إذا( flags & SQLITE_SHM_UNLOCK ){
      /* Case (a) - unlock.  */

      أكد( (p->exclMask & p->sharedMask)==0 );
      أكد( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->exclMask & mask)==mask );
      أكد( !(flags & SQLITE_SHM_SHARED) || (p->sharedMask & mask)==mask );

      أكد( !(flags & SQLITE_SHM_SHARED) || n==1 );
      إذا( p_Shm_كلي->bUseSharedLockHandle ){
        h = p_Shm_كلي->hSharedShm;
        إذا( flags & SQLITE_SHM_SHARED ){
          winShm *pShm;
          sqlite3_mutex_enter(p_Shm_كلي->mutex);
          لكل(pShm=p_Shm_كلي->pWinShmList; pShm; pShm=pShm->pWinShmNext){
            إذا( pShm!=p && (pShm->sharedMask & mask) ){
              /* Another connection within this process is also holding this
              ** SHARED lock. So do not actually release the OS lock.  */
              h = INVALID_HANDLE_درج;
              اكسر;
            }
          }
          sqlite3_mutex_leave(p_Shm_كلي->mutex);
        }
      }

      إذا( h!=INVALID_HANDLE_درج ){
        rc = winHandleUnlock(h, ofst+WIN_SHM_BASE, n);
      }

      /* If successful, also clear the bits in sharedMask/exclMask */
      إذا( rc==SQLITE_OK ){
        p->exclMask = (p->exclMask & ~mask);
        p->sharedMask = (p->sharedMask & ~mask);
      }
    }وإلا{
      صحيح bExcl = ((flags & SQLITE_SHM_EXCLUSIVE) ? 1 : 0);
      DWORD nMs = winFileBusyTimeout(pDbFd);

      إذا( p_Shm_كلي->bUseSharedLockHandle ){
        winShm *pShm;
        h = p_Shm_كلي->hSharedShm;
        sqlite3_mutex_enter(p_Shm_كلي->mutex);
        لكل(pShm=p_Shm_كلي->pWinShmList; pShm; pShm=pShm->pWinShmNext){
          إذا( bExcl ){
            إذا( (pShm->sharedMask|pShm->exclMask) & mask ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_درج;
            }
          }وإلا{
            إذا( pShm->sharedMask & mask ){
              h = INVALID_HANDLE_درج;
            }وإلا إذا( pShm->exclMask & mask ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_درج;
            }
          }
        }
        sqlite3_mutex_leave(p_Shm_كلي->mutex);
      }

      إذا( h!=INVALID_HANDLE_درج ){
        rc = winHandleLockTimeout(h, ofst+WIN_SHM_BASE, n, bExcl, nMs);
      }
      إذا( rc==SQLITE_OK ){
        إذا( bExcl ){
          p->exclMask = (p->exclMask | mask);
        }وإلا{
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
  أرجع rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
ساكن فراغ winShmBarrier(
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
ساكن صحيح winShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  صحيح iRegion,                    /* Region to retrieve */
  صحيح szRegion,                   /* Size of regions */
  صحيح isWrite,                    /* True to extend file if necessary */
  فراغ متطاير **pp              /* OUT: Mapped memory */
){
  winFile *pDbFd = (winFile*)fd;
  winShm *pShm = pDbFd->pShm;
  win_Shm_كلي *p_Shm_كلي;
  DWORD protect = PAGE_READWRITE;
  DWORD flags = FILE_MAP_WRITE | FILE_MAP_READ;
  صحيح rc = SQLITE_OK;

  إذا( !pShm ){
    rc = winOpenSharedMemory(pDbFd);
    إذا( rc!=SQLITE_OK ) أرجع rc;
    pShm = pDbFd->pShm;
    أكد( pShm!=0 );
  }
  p_Shm_كلي = pShm->p_Shm_كلي;

  sqlite3_mutex_enter(p_Shm_كلي->mutex);
  إذا( p_Shm_كلي->isUnlocked ){
    /* Take the DMS lock. */
    أكد( p_Shm_كلي->nRegion==0 );
    rc = winLockSharedMemory(p_Shm_كلي, winFileBusyTimeout(pDbFd));
    إذا( rc!=SQLITE_OK ) اذهب shmpage_out;
  }

  أكد( szRegion==p_Shm_كلي->szRegion || p_Shm_كلي->nRegion==0 );
  إذا( p_Shm_كلي->nRegion<=iRegion ){
    HANDLE hShared = p_Shm_كلي->hSharedShm;
    بنية ShmRegion *apNew;           /* New aRegion[] array */
    i64 nByte = ((i64)iRegion+1)*(i64)szRegion;  /* Minimum file size */
    sqlite3_int64 sz;                  /* Current size of wal-index file */

    p_Shm_كلي->szRegion = szRegion;

    /* The requested region is not mapped into this processes address space.
    ** Check to see if it has been allocated (i.e. if the wal-index file is
    ** large enough to contain the requested region).
    */
    rc = winHandleSize(hShared, &sz);
    إذا( rc!=SQLITE_OK ){
      rc = winLogError(rc, osGetLastError(), "winShmMap1", pDbFd->zPath);
      اذهب shmpage_out;
    }

    إذا( sz<nByte ){
      /* The requested memory region does not exist. If isWrite is set to
      ** zero, exit early. *pp will be set to NULL and SQLITE_OK returned.
      **
      ** Alternatively, if isWrite is non-zero, use ftruncate() to allocate
      ** the requested memory region.  */
      إذا( !isWrite ) اذهب shmpage_out;
      rc = winHandleTruncate(hShared, nByte);
      إذا( rc!=SQLITE_OK ){
        rc = winLogError(rc, osGetLastError(), "winShmMap2", pDbFd->zPath);
        اذهب shmpage_out;
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (بنية ShmRegion*)sqlite3_realloc64(
        p_Shm_كلي->aRegion, ((i64)iRegion+1)*حجم(apNew[0])
    );
    إذا( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      اذهب shmpage_out;
    }
    p_Shm_كلي->aRegion = apNew;

    إذا( p_Shm_كلي->isReadonly ){
      protect = PAGE_READONLY;
      flags = FILE_MAP_READ;
    }

    طالما( p_Shm_كلي->nRegion<=iRegion ){
      HANDLE hMap = عدم;         /* file-mapping handle */
      فراغ *pMap = 0;             /* Mapped memory region */

#ifdef SQLITE_UWP
      hMap = osCreateFileMappingFromApp(hShared, عدم, protect, nByte, عدم);
#وإلا
      hMap = osCreateFileMappingW(hShared, عدم, protect, 0, nByte, عدم);
#endif
      OSTRACE(("SHM-MAP-CREATE pid=%lu, region=%d, size=%lld, rc=%s\n",
               osGetCurrentProcessId(), p_Shm_كلي->nRegion, nByte,
               hMap ? "ok" : "failed"));
      إذا( hMap ){
        i64 iOffset = p_Shm_كلي->nRegion*szRegion;
        صحيح iOffsetShift = iOffset % winSysInfo.dwAllocationGranularity;
#ifdef SQLITE_UWP
        pMap = osMapViewOfFileFromApp(hMap, flags,
            iOffset - iOffsetShift, (i64)szRegion + iOffsetShift
        );
#وإلا
        pMap = osMapViewOfFile(hMap, flags,
            0, iOffset - iOffsetShift, (i64)szRegion + iOffsetShift
        );
#endif
        OSTRACE(("SHM-MAP-MAP pid=%lu, region=%d, offset=%d, size=%d, rc=%s\n",
                 osGetCurrentProcessId(), p_Shm_كلي->nRegion, iOffset,
                 szRegion, pMap ? "ok" : "failed"));
      }
      إذا( !pMap ){
        p_Shm_كلي->lastErrno = osGetLastError();
        rc = winLogError(SQLITE_IOERR_SHMMAP, p_Shm_كلي->lastErrno,
                         "winShmMap3", pDbFd->zPath);
        إذا( hMap ) osCloseHandle(hMap);
        اذهب shmpage_out;
      }

      p_Shm_كلي->aRegion[p_Shm_كلي->nRegion].pMap = pMap;
      p_Shm_كلي->aRegion[p_Shm_كلي->nRegion].hMap = hMap;
      p_Shm_كلي->nRegion++;
    }
  }

shmpage_out:
  إذا( p_Shm_كلي->nRegion>iRegion ){
    i64 iOffset = (i64)iRegion*(i64)szRegion;
    صحيح iOffsetShift = iOffset % winSysInfo.dwAllocationGranularity;
    حرف *p = (حرف *)p_Shm_كلي->aRegion[iRegion].pMap;
    *pp = (فراغ *)&p[iOffsetShift];
  }وإلا{
    *pp = 0;
  }
  إذا( p_Shm_كلي->isReadonly && rc==SQLITE_OK ){
    rc = SQLITE_READONLY;
  }
  sqlite3_mutex_leave(p_Shm_كلي->mutex);
  أرجع rc;
}

#وإلا
# define winShmMap     0
# define winShmLock    0
# define winShmBarrier 0
# define winShmUnmap   0
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Cleans up the mapped region of the specified file, if any.
*/
#إذا SQLITE_MAX_MMAP_SIZE>0
ساكن صحيح winUnmapfile(winFile *pFile){
  أكد( pFile!=0 );
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, pMapRegion=%p, "
           "mmapSize=%lld, mmapSizeMax=%lld\n",
           osGetCurrentProcessId(), pFile, pFile->hMap, pFile->pMapRegion,
           pFile->mmapSize, pFile->mmapSizeMax));
  إذا( pFile->pMapRegion ){
    إذا( !osUnmapViewOfFile(pFile->pMapRegion) ){
      pFile->lastErrno = osGetLastError();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(), pFile,
               pFile->pMapRegion));
      أرجع winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winUnmapfile1", pFile->zPath);
    }
    pFile->pMapRegion = 0;
    pFile->mmapSize = 0;
  }
  إذا( pFile->hMap!=عدم ){
    إذا( !osCloseHandle(pFile->hMap) ){
      pFile->lastErrno = osGetLastError();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, rc=SQLITE_IOERR_MMAP\n",
               osGetCurrentProcessId(), pFile, pFile->hMap));
      أرجع winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winUnmapfile2", pFile->zPath);
    }
    pFile->hMap = عدم;
  }
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile));
  أرجع SQLITE_OK;
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
ساكن صحيح winMapfile(winFile *pFd, sqlite3_int64 nByte){
  sqlite3_int64 nMap = nByte;
  صحيح rc;

  أكد( nMap>=0 || pFd->nFetchOut==0 );
  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, size=%lld\n",
           osGetCurrentProcessId(), pFd, nByte));

  إذا( pFd->nFetchOut>0 ) أرجع SQLITE_OK;

  إذا( nMap<0 ){
    rc = winFileSize((sqlite3_file*)pFd, &nMap);
    إذا( rc ){
      OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_IOERR_FSTAT\n",
               osGetCurrentProcessId(), pFd));
      أرجع SQLITE_IOERR_FSTAT;
    }
  }
  إذا( nMap>pFd->mmapSizeMax ){
    nMap = pFd->mmapSizeMax;
  }
  nMap &= ~(sqlite3_int64)(winSysInfo.dwPageSize - 1);

  إذا( nMap==0 && pFd->mmapSize>0 ){
    winUnmapfile(pFd);
  }
  إذا( nMap!=pFd->mmapSize ){
    فراغ *pNew = 0;
    DWORD protect = PAGE_READONLY;
    DWORD flags = FILE_MAP_READ;

    winUnmapfile(pFd);
#ifdef SQLITE_MMAP_READWRITE
    إذا( (pFd->ctrlFlags & WINFILE_RDONLY)==0 ){
      protect = PAGE_READWRITE;
      flags |= FILE_MAP_WRITE;
    }
#endif
#ifdef SQLITE_UWP
    pFd->hMap = osCreateFileMappingFromApp(pFd->h, عدم, protect, nMap, عدم);
#وإلا
    pFd->hMap = osCreateFileMappingW(pFd->h, عدم, protect,
                                (DWORD)((nMap>>32) & 0xffffffff),
                                (DWORD)(nMap & 0xffffffff), عدم);
#endif
    إذا( pFd->hMap==عدم ){
      pFd->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->lastErrno,
                       "winMapfile1", pFd->zPath);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-CREATE pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      أرجع SQLITE_OK;
    }
    أكد( (nMap % winSysInfo.dwPageSize)==0 );
    أكد( حجم(SIZE_T)==حجم(sqlite3_int64) || nMap<=0xffffffff );
#ifdef SQLITE_UWP
    pNew = osMapViewOfFileFromApp(pFd->hMap, flags, 0, (SIZE_T)nMap);
#وإلا
    pNew = osMapViewOfFile(pFd->hMap, flags, 0, 0, (SIZE_T)nMap);
#endif
    إذا( pNew==عدم ){
      osCloseHandle(pFd->hMap);
      pFd->hMap = عدم;
      pFd->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->lastErrno,
                       "winMapfile2", pFd->zPath);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-MAP pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      أرجع SQLITE_OK;
    }
    pFd->pMapRegion = pNew;
    pFd->mmapSize = nMap;
  }

  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFd));
  أرجع SQLITE_OK;
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
ساكن صحيح winFetch(sqlite3_file *fd, i64 iOff, صحيح nAmt, فراغ **pp){
#إذا SQLITE_MAX_MMAP_SIZE>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */
#endif
  *pp = 0;

  OSTRACE(("FETCH pid=%lu, pFile=%p, offset=%lld, amount=%d, pp=%p\n",
           osGetCurrentProcessId(), fd, iOff, nAmt, pp));

#إذا SQLITE_MAX_MMAP_SIZE>0
  إذا( pFd->mmapSizeMax>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    ثابت صحيح nEofBuffer = 256;
    إذا( pFd->pMapRegion==0 ){
      صحيح rc = winMapfile(pFd, -1);
      إذا( rc!=SQLITE_OK ){
        OSTRACE(("FETCH pid=%lu, pFile=%p, rc=%s\n",
                 osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
        أرجع rc;
      }
    }
    إذا( pFd->mmapSize >= (iOff+nAmt+nEofBuffer) ){
      أكد( pFd->pMapRegion!=0 );
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->nFetchOut++;
    }
  }
#endif

  OSTRACE(("FETCH pid=%lu, pFile=%p, pp=%p, *pp=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd, pp, *pp));
  أرجع SQLITE_OK;
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
ساكن صحيح winUnfetch(sqlite3_file *fd, i64 iOff, فراغ *p){
#إذا SQLITE_MAX_MMAP_SIZE>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  أكد( (p==0)==(pFd->nFetchOut==0) );

  /* If p!=0, it must match the iOff value. */
  أكد( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, offset=%lld, p=%p\n",
           osGetCurrentProcessId(), pFd, iOff, p));

  إذا( p ){
    pFd->nFetchOut--;
  }وإلا{
    /* FIXME:  If Windows truly always prevents truncating or deleting a
    ** file while a mapping is held, then the following winUnmapfile() call
    ** is unnecessary can be omitted - potentially improving
    ** performance.  */
    winUnmapfile(pFd);
  }

  أكد( pFd->nFetchOut>=0 );
#endif

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd));
  أرجع SQLITE_OK;
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
ساكن ثابت sqlite3_io_methods winIoMethod = {
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
ساكن ثابت sqlite3_io_methods winIoNolockMethod = {
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

ساكن winVfsAppData winAppData = {
  &winIoMethod,       /* pMethod */
  0,                  /* pAppData */
  0                   /* bNoLock */
};

ساكن winVfsAppData winNolockAppData = {
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
ساكن صحيح winMakeEndInDirSep(صحيح nBuf, حرف *zBuf){
  إذا( zBuf ){
    صحيح nLen = sqlite3Strlen30(zBuf);
    إذا( nLen>0 ){
      إذا( winIsDirSep(zBuf[nLen-1]) ){
        أرجع 1;
      }وإلا إذا( nLen+1<nBuf ){
        إذا( !osGetenv ){
          zBuf[nLen] = winGetDirSep();
        }وإلا إذا( winIsDriveLetterAndColon(zBuf) && winIsDirSep(zBuf[2]) ){
          zBuf[nLen] = '\\';
          zBuf[2]='\\';
        }وإلا{
          zBuf[nLen] = '/';
        }
        zBuf[nLen+1] = '\0';
        أرجع 1;
      }
    }
  }
  أرجع 0;
}

/*
** If sqlite3_temp_directory is defined, take the mutex and return true.
**
** If sqlite3_temp_directory is NULL (undefined), omit the mutex and
** return false.
*/
ساكن صحيح winTempDirDefined(فراغ){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  إذا( sqlite3_temp_directory!=0 ) أرجع 1;
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  أرجع 0;
}

/*
** Create a temporary file name and store the resulting pointer into pzBuf.
** The pointer returned in pzBuf must be freed via sqlite3_free().
*/
ساكن صحيح winGetTempname(sqlite3_vfs *pVfs, حرف **pzBuf){
  ساكن ثابت حرف zChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  size_t i, j;
  DWORD pid;
  صحيح nPre = sqlite3Strlen30(SQLITE_TEMP_FILE_PREFIX);
  i64 nMax, nBuf, nDir, nLen;
  حرف *zBuf;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  SimulateIOError( أرجع SQLITE_IOERR );

  /* Allocate a temporary buffer to store the fully qualified file
  ** name for the temporary file.  If this fails, we cannot continue.
  */
  nMax = pVfs->mxPathname;
  nBuf = 2 + (i64)nMax;
  zBuf = sqlite3MallocZero( nBuf );
  إذا( !zBuf ){
    OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
    أرجع SQLITE_IOERR_NOMEM_BKPT;
  }

  /* Figure out the effective temporary directory.  First, check if one
  ** has been explicitly set by the application; otherwise, use the one
  ** configured by the operating system.
  */
  nDir = nMax - (nPre + 15);
  أكد( nDir>0 );
  إذا( winTempDirDefined() ){
    صحيح nDirLen = sqlite3Strlen30(sqlite3_temp_directory);
    إذا( nDirLen>0 ){
      إذا( !winIsDirSep(sqlite3_temp_directory[nDirLen-1]) ){
        nDirLen++;
      }
      إذا( nDirLen>nDir ){
        sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
        sqlite3_free(zBuf);
        OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
        أرجع winLogError(SQLITE_ERROR, 0, "winGetTempname1", 0);
      }
      sqlite3_snprintf(nMax, zBuf, "%s", sqlite3_temp_directory);
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  }

#إذا defined(__CYGWIN__)
  وإلا إذا( osGetenv!=عدم ){
    ساكن ثابت حرف *azDirs[] = {
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
    unsigned صحيح i;
    ثابت حرف *zDir = 0;

    إذا( !azDirs[0] ) azDirs[0] = osGetenv("SQLITE_TMPDIR");
    إذا( !azDirs[1] ) azDirs[1] = osGetenv("TMPDIR");
    إذا( !azDirs[2] ) azDirs[2] = osGetenv("TMP");
    إذا( !azDirs[3] ) azDirs[3] = osGetenv("TEMP");
    إذا( !azDirs[4] ) azDirs[4] = osGetenv("USERPROFILE");
    لكل(i=0; i<حجم(azDirs)/حجم(azDirs[0]); zDir=azDirs[i++]){
      LPWSTR zConverted;
      إذا( zDir==0 ) استمر;
      /* If the path starts with a drive letter followed by the colon
      ** character, assume it is already a native Win32 path; otherwise,
      ** it must be converted to a native Win32 path via the Cygwin API
      ** prior to using it.
      */
      {
        zConverted = winConvertFromUtf8Filename(zDir);
        إذا( !zConverted ){
          sqlite3_free(zBuf);
          OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
          أرجع SQLITE_IOERR_NOMEM_BKPT;
        }
        إذا( winIsDir(zConverted) ){
          sqlite3_snprintf(nMax, zBuf, "%s", zDir);
          sqlite3_free(zConverted);
          اكسر;
        }
        sqlite3_free(zConverted);
      }
    }
  }
#endif

  وإلا{
    حرف *zMulti;
    LPWSTR zWidePath = sqlite3MallocZero( nMax*حجم(WCHAR) );
    إذا( !zWidePath ){
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      أرجع SQLITE_IOERR_NOMEM_BKPT;
    }
    إذا( osGetTempPathW==0 || osGetTempPathW(nMax, zWidePath)==0 ){
      sqlite3_free(zWidePath);
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_GETTEMPPATH\n"));
      أرجع winLogError(SQLITE_IOERR_GETTEMPPATH, osGetLastError(),
                         "winGetTempname2", 0);
    }
    zMulti = winUnicodeToUtf8(zWidePath);
    sqlite3_free(zWidePath);
    إذا( zMulti ){
      sqlite3_snprintf(nMax, zBuf, "%s", zMulti);
      sqlite3_free(zMulti);
    }وإلا{
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      أرجع SQLITE_IOERR_NOMEM_BKPT;
    }
  }

  /*
  ** Check to make sure the temporary directory ends with an appropriate
  ** separator.  If it does not and there is not enough space left to add
  ** one, fail.
  */
  إذا( !winMakeEndInDirSep(nDir+1, zBuf) ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    أرجع winLogError(SQLITE_ERROR, 0, "winGetTempname4", 0);
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
  إذا( (nLen + nPre + 17) > nBuf ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    أرجع winLogError(SQLITE_ERROR, 0, "winGetTempname5", 0);
  }

  sqlite3_snprintf(nBuf-16-nLen, zBuf+nLen, SQLITE_TEMP_FILE_PREFIX);

  j = sqlite3Strlen30(zBuf);
  sqlite3_randomness(15, &zBuf[j]);
  pid = osGetCurrentProcessId();
  لكل(i=0; i<15; i++, j++){
    zBuf[j] += pid & 0xff;
    pid >>= 8;
    zBuf[j] = (حرف)zChars[ ((unsigned حرف)zBuf[j])%(حجم(zChars)-1) ];
  }
  zBuf[j] = 0;
  zBuf[j+1] = 0;
  *pzBuf = zBuf;

  OSTRACE(("TEMP-FILENAME name=%s, rc=SQLITE_OK\n", zBuf));
  أرجع SQLITE_OK;
}

/*
** Return TRUE if the named file is really a directory.  Return false if
** it is something other than a directory, or if there is any kind of memory
** allocation failure.
*/
ساكن صحيح winIsDir(LPCWSTR zConverted){
  DWORD attr;
  صحيح rc = 0;
  DWORD lastErrno;
  صحيح cnt = 0;
  WIN32_FILE_ATTRIBUTE_DATA sAttrData;
  memset(&sAttrData, 0, حجم(sAttrData));
  طالما( !(rc = osGetFileAttributesExW(zConverted,
                           GetFileExInfoStandard,
                           &sAttrData)) && winRetryIoerr(&cnt, &lastErrno) ){}
  إذا( !rc ){
    أرجع 0; /* Invalid name? */
  }
  attr = sAttrData.dwFileAttributes;
  أرجع (attr!=INVALID_FILE_ATTRIBUTES) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

/* forward reference */
ساكن صحيح winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ثابت حرف *zFilename,     /* Name of file to check */
  صحيح flags,                 /* Type of test to make on this file */
  صحيح *pResOut               /* OUT: Result */
);

/*
** The Windows version of xAccess() accepts an extra bit in the flags
** parameter that prevents an anti-virus retry loop.
*/
#define NORETRY 0x4000

/*
** Open a file.
*/
ساكن صحيح winOpen(
  sqlite3_vfs *pVfs,        /* Used to get maximum path length and AppData */
  ثابت حرف *zName,        /* Name of the file (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  صحيح flags,                /* Open mode flags */
  صحيح *pOutFlags            /* Status return flags */
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
  ثابت حرف *zUtf8Name = zName; /* Filename in UTF-8 encoding */
  صحيح cnt = 0;
  صحيح isRO = 0;              /* file is known to be accessible readonly */

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  حرف *zTmpname = 0; /* For temporary filename, if necessary. */

  صحيح rc = SQLITE_OK;            /* Function Return Code */
#إذا !defined(NDEBUG)
  صحيح eType = flags&0x0FFF00;  /* Type of file to open */
#endif

  صحيح isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  صحيح isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  صحيح isCreate     = (flags & SQLITE_OPEN_CREATE);
  صحيح isReadonly   = (flags & SQLITE_OPEN_READONLY);
  صحيح isReadWrite  = (flags & SQLITE_OPEN_READWRITE);

#ifndef NDEBUG
  صحيح isOpenJournal = (isCreate && (
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
  أكد((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
  أكد(isCreate==0 || isReadWrite);
  أكد(isExclusive==0 || isCreate);
  أكد(isDelete==0 || isCreate);

  /* The main DB, main journal, WAL file and super-journal are never
  ** automatically deleted. Nor are they ever temporary files.  */
  أكد( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_DB );
  أكد( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_JOURNAL );
  أكد( (!isDelete && zName) || eType!=SQLITE_OPEN_SUPER_JOURNAL );
  أكد( (!isDelete && zName) || eType!=SQLITE_OPEN_WAL );

  /* Assert that the upper layer has set one of the "file-type" flags. */
  أكد( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_TEMP_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_TEMP_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );

  أكد( pFile!=0 );
  memset(pFile, 0, حجم(winFile));
  pFile->h = INVALID_HANDLE_درج;

  /* If the second argument to this function is NULL, generate a
  ** temporary file name to use
  */
  إذا( !zUtf8Name ){
    أكد( isDelete && !isOpenJournal );
    rc = winGetTempname(pVfs, &zTmpname);
    إذا( rc!=SQLITE_OK ){
      OSTRACE(("OPEN name=%s, rc=%s", zUtf8Name, sqlite3ErrName(rc)));
      أرجع rc;
    }
    zUtf8Name = zTmpname;
  }

  /* Database filenames are double-zero terminated if they are not
  ** URIs with parameters.  Hence, they can always be passed into
  ** sqlite3_uri_parameter().
  */
  أكد( (eType!=SQLITE_OPEN_MAIN_DB) || (flags & SQLITE_OPEN_URI) ||
       zUtf8Name[sqlite3Strlen30(zUtf8Name)+1]==0 );

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8Name);
  إذا( zConverted==0 ){
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8Name));
    أرجع SQLITE_IOERR_NOMEM_BKPT;
  }

  إذا( winIsDir(zConverted) ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8Name));
    أرجع SQLITE_CANTOPEN_ISDIR;
  }

  إذا( isReadWrite ){
    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
  }وإلا{
    dwDesiredAccess = GENERIC_READ;
  }

  /* SQLITE_OPEN_EXCLUSIVE is used to make sure that a new file is
  ** created. SQLite doesn't use it to indicate "exclusive access"
  ** as it is usually understood.
  */
  إذا( isExclusive ){
    /* Creates a new file, only if it does not already exist. */
    /* If the file exists, it fails. */
    dwCreationDisposition = CREATE_NEW;
  }وإلا إذا( isCreate ){
    /* Open existing file, or create if it doesn't exist */
    dwCreationDisposition = OPEN_ALWAYS;
  }وإلا{
    /* Opens a file, only if it exists. */
    dwCreationDisposition = OPEN_EXISTING;
  }

  إذا( 0==sqlite3_uri_boolean(zName, "exclusive", 0) ){
    dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  }وإلا{
    dwShareMode = 0;
  }

  إذا( isDelete ){
    dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY
                               | FILE_ATTRIBUTE_HIDDEN
                               | FILE_FLAG_DELETE_ON_CLOSE;
  }وإلا{
    dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
  }
  افعل{
    h = osCreateFileW(zConverted,
                      dwDesiredAccess,
                      dwShareMode, عدم,
                      dwCreationDisposition,
                      dwFlagsAndAttributes,
                      عدم);
    إذا( h!=INVALID_HANDLE_درج ) اكسر;
    إذا( isReadWrite ){
      صحيح rc2;
      sqlite3BeginBenignMalloc();
      rc2 = winAccess(pVfs, zUtf8Name, SQLITE_ACCESS_READ|NORETRY, &isRO);
      sqlite3EndBenignMalloc();
      إذا( rc2==SQLITE_OK && isRO ) اكسر;
    }
  }طالما( winRetryIoerr(&cnt, &lastErrno) );
  winLogIoerr(cnt, __LINE__);

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, rc=%s\n", h, zUtf8Name,
           dwDesiredAccess, (h==INVALID_HANDLE_درج) ? "failed" : "ok"));

  إذا( h==INVALID_HANDLE_درج ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    إذا( isReadWrite && isRO && !isExclusive ){
      أرجع winOpen(pVfs, zName, id,
         ((flags|SQLITE_OPEN_READONLY) &
                     ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE)),
         pOutFlags);
    }وإلا{
      pFile->lastErrno = lastErrno;
      winLogError(SQLITE_CANTOPEN, pFile->lastErrno, "winOpen", zUtf8Name);
      أرجع SQLITE_CANTOPEN_BKPT;
    }
  }

  إذا( pOutFlags ){
    إذا( isReadWrite ){
      *pOutFlags = SQLITE_OPEN_READWRITE;
    }وإلا{
      *pOutFlags = SQLITE_OPEN_READONLY;
    }
  }

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, pOutFlags=%p, *pOutFlags=%d, "
           "rc=%s\n", h, zUtf8Name, dwDesiredAccess, pOutFlags, pOutFlags ?
           *pOutFlags : 0, (h==INVALID_HANDLE_درج) ? "failed" : "ok"));

  pAppData = (winVfsAppData*)pVfs->pAppData;

  sqlite3_free(zConverted);
  sqlite3_free(zTmpname);
  id->pMethods = pAppData ? pAppData->pMethod : &winIoMethod;
  pFile->pVfs = pVfs;
  pFile->h = h;
  إذا( isReadonly ){
    pFile->ctrlFlags |= WINFILE_RDONLY;
  }
  إذا( (flags & SQLITE_OPEN_MAIN_DB)
   && sqlite3_uri_boolean(zName, "psow", SQLITE_POWERSAFE_OVERWRITE) 
  ){
    pFile->ctrlFlags |= WINFILE_PSOW;
  }
  pFile->lastErrno = NO_ERROR;
  pFile->zPath = zName;
#إذا SQLITE_MAX_MMAP_SIZE>0
  pFile->hMap = عدم;
  pFile->pMapRegion = 0;
  pFile->mmapSize = 0;
  pFile->mmapSizeMax = sqlite3GlobalConfig.szMmap;
#endif

  OpenCounter(+1);
  أرجع rc;
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
ساكن صحيح winDelete(
  sqlite3_vfs *pVfs,          /* Not used on win32 */
  ثابت حرف *zFilename,      /* Name of file to delete */
  صحيح syncDir                 /* Not used on win32 */
){
  صحيح cnt = 0;
  صحيح rc;
  DWORD attr;
  DWORD lastErrno = 0;
  LPWSTR zConverted;
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(syncDir);

  SimulateIOError(أرجع SQLITE_IOERR_DELETE);
  OSTRACE(("DELETE name=%s, syncDir=%d\n", zFilename, syncDir));

  zConverted = winConvertFromUtf8Filename(zFilename);
  إذا( zConverted==0 ){
    OSTRACE(("DELETE name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    أرجع SQLITE_IOERR_NOMEM_BKPT;
  }
  افعل {
    attr = osGetFileAttributesW(zConverted);
    إذا ( attr==INVALID_FILE_ATTRIBUTES ){
      lastErrno = osGetLastError();
      إذا( lastErrno==ERROR_FILE_NOT_FOUND
       || lastErrno==ERROR_PATH_NOT_FOUND ){
        rc = SQLITE_IOERR_DELETE_NOENT; /* Already gone? */
      }وإلا{
        rc = SQLITE_ERROR;
      }
      اكسر;
    }
    إذا ( attr&FILE_ATTRIBUTE_DIRECTORY ){
      rc = SQLITE_ERROR; /* Files only. */
      اكسر;
    }
    إذا ( osDeleteFileW(zConverted) ){
      rc = SQLITE_OK; /* Deleted OK. */
      اكسر;
    }
    إذا ( !winRetryIoerr(&cnt, &lastErrno) ){
      rc = SQLITE_ERROR; /* No more retries. */
      اكسر;
    }
  } طالما(1);
  إذا( rc && rc!=SQLITE_IOERR_DELETE_NOENT ){
    rc = winLogError(SQLITE_IOERR_DELETE, lastErrno, "winDelete", zFilename);
  }وإلا{
    winLogIoerr(cnt, __LINE__);
  }
  sqlite3_free(zConverted);
  OSTRACE(("DELETE name=%s, rc=%s\n", zFilename, sqlite3ErrName(rc)));
  أرجع rc;
}

/*
** Check the existence and status of a file.
*/
ساكن صحيح winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ثابت حرف *zFilename,     /* Name of file to check */
  صحيح flags,                 /* Type of test to make on this file */
  صحيح *pResOut               /* OUT: Result */
){
  DWORD attr;
  صحيح rc = 0;
  صحيح cnt = 0;
  DWORD lastErrno = 0;
  LPWSTR zConverted;
  صحيح noRetry = 0;           /* Do not use winRetryIoerr() */
  WIN32_FILE_ATTRIBUTE_DATA sAttrData;
  UNUSED_PARAMETER(pVfs);

  إذا( (flags & NORETRY)!=0 ){
    noRetry = 1;
    flags &= ~NORETRY;
  }  

  SimulateIOError( أرجع SQLITE_IOERR_ACCESS; );
  OSTRACE(("ACCESS name=%s, flags=%x, pResOut=%p\n",
           zFilename, flags, pResOut));

  إذا( zFilename==0 ){
    *pResOut = 0;
    OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
             zFilename, pResOut, *pResOut));
    أرجع SQLITE_OK;
  }

  zConverted = winConvertFromUtf8Filename(zFilename);
  إذا( zConverted==0 ){
    OSTRACE(("ACCESS name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    أرجع SQLITE_IOERR_NOMEM_BKPT;
  }
  memset(&sAttrData, 0, حجم(sAttrData));
  طالما( !(rc = osGetFileAttributesExW(zConverted,
                           GetFileExInfoStandard,
                           &sAttrData))
     && !noRetry
     && winRetryIoerr(&cnt, &lastErrno)
  ){ /* Loop until true */}
  إذا( rc ){
    /* For an SQLITE_ACCESS_EXISTS query, treat a zero-length file
    ** as if it does not exist.
    */
    إذا(    flags==SQLITE_ACCESS_EXISTS
        && sAttrData.n_File_Size_سم==0
        && sAttrData.n_File_Size_كس==0 ){
      attr = INVALID_FILE_ATTRIBUTES;
    }وإلا{
      attr = sAttrData.dwFileAttributes;
    }
  }وإلا{
    إذا( noRetry ) lastErrno = osGetLastError();
    winLogIoerr(cnt, __LINE__);
    إذا( lastErrno!=ERROR_FILE_NOT_FOUND && lastErrno!=ERROR_PATH_NOT_FOUND ){
      sqlite3_free(zConverted);
      أرجع winLogError(SQLITE_IOERR_ACCESS, lastErrno, "winAccess",
                         zFilename);
    }وإلا{
      attr = INVALID_FILE_ATTRIBUTES;
    }
  }
  sqlite3_free(zConverted);
  بدل( flags ){
    حالة SQLITE_ACCESS_READ:
    حالة SQLITE_ACCESS_EXISTS:
      rc = attr!=INVALID_FILE_ATTRIBUTES;
      اكسر;
    حالة SQLITE_ACCESS_READWRITE:
      rc = attr!=INVALID_FILE_ATTRIBUTES &&
             (attr & FILE_ATTRIBUTE_READONLY)==0;
      اكسر;
    افتراضي:
      أكد(!"Invalid flags argument");
  }
  *pResOut = rc;
  OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           zFilename, pResOut, *pResOut));
  أرجع SQLITE_OK;
}

/*
** Returns non-zero if the specified path name starts with the "long path"
** prefix.
*/
ساكن BOOL winIsLongPathPrefix(
  ثابت حرف *zPathname
){
  أرجع ( zPathname[0]=='\\' && zPathname[1]=='\\'
        && zPathname[2]=='?'  && zPathname[3]=='\\' );
}

/*
** Returns non-zero if the specified path name starts with a drive letter
** followed by a colon character.
*/
ساكن BOOL winIsDriveLetterAndColon(
  ثابت حرف *zPathname
){
  أرجع ( sqlite3Isalpha(zPathname[0]) && zPathname[1]==':' );
}

#ifdef _WIN32
/*
** Returns non-zero if the specified path name should be used verbatim.  If
** non-zero is returned from this function, the calling function must simply
** use the provided path name verbatim -OR- resolve it into a full path name
** using the GetFullPathName Win32 API function (if available).
*/
ساكن BOOL winIsVerbatimPathname(
  ثابت حرف *zPathname
){
  /*
  ** If the path name starts with a forward slash or a backslash, it is either
  ** a legal UNC name, a volume relative path, or an absolute path name in the
  ** "Unix" format on Windows.  There is no easy way to differentiate between
  ** the final two cases; therefore, we return the safer return value of TRUE
  ** so that callers of this function will simply use it verbatim.
  */
  إذا ( winIsDirSep(zPathname[0]) ){
    أرجع TRUE;
  }

  /*
  ** If the path name starts with a letter and a colon it is either a volume
  ** relative path or an absolute path.  Callers of this function must not
  ** attempt to treat it as a relative path name (i.e. they should simply use
  ** it verbatim).
  */
  إذا ( winIsDriveLetterAndColon(zPathname) ){
    أرجع TRUE;
  }

  /*
  ** If we get to this point, the path name should almost certainly be a purely
  ** relative one (i.e. not a UNC name, not absolute, and not volume relative).
  */
  أرجع FALSE;
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
ساكن فراغ winSimplifyName(حرف *z){
  صحيح i, j;
  لكل(i=j=0; z[i]; ++i){
    إذا( winIsDirSep(z[i]) ){
#إذا !defined(SQLITE_TEST)
      /* Some test-cases assume that "./foo" and "foo" are different */
      إذا( z[i+1]=='.' && winIsDirSep(z[i+2]) ){
        ++i;
        استمر;
      }
#endif
      إذا( !z[i+1] || (winIsDirSep(z[i+1]) && (i!=0)) ){
        استمر;
      }
      z[j++] = osGetenv?'/':'\\';
    }وإلا{
      z[j++] = z[i];
    }
  }
  طالما(j<i) z[j++] = '\0';
}

#define SQLITE_MAX_SYMLINKS 100

ساكن صحيح mkFullPathname(
  ثابت حرف *zPath,              /* Input path */
  حرف *zOut,                     /* Output buffer */
  صحيح nOut                        /* Allocated size of buffer zOut */
){
  صحيح nPath = sqlite3Strlen30(zPath);
  صحيح iOff = 0;
  إذا( zPath[0]!='/' ){
    إذا( osGetcwd(zOut, nOut-2)==0 ){
      أرجع winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno, "getcwd", zPath);
    }
    iOff = sqlite3Strlen30(zOut);
    zOut[iOff++] = '/';
  }
  إذا( (iOff+nPath+1)>nOut ){
    /* SQLite assumes that xFullPathname() nul-terminates the output buffer
    ** even if it returns an error.  */
    zOut[iOff] = '\0';
    أرجع SQLITE_CANTOPEN_BKPT;
  }
  sqlite3_snprintf(nOut-iOff, &zOut[iOff], "%s", zPath);
  أرجع SQLITE_OK;
}
#endif /* __CYGWIN__ */

/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
ساكن صحيح winFullPathnameNoMutex(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ثابت حرف *zRelative,        /* Possibly relative input path */
  صحيح nFull,                    /* Size of output buffer in bytes */
  حرف *zFull                   /* Output buffer */
){
  صحيح nByte;
  LPWSTR zConverted;
  حرف *zOut;

  /* If this path name begins with "/X:" or "\\?\", where "X" is any
  ** alphabetic character, discard the initial "/" from the pathname.
  */
  إذا( zRelative[0]=='/' && (winIsDriveLetterAndColon(zRelative+1)
       || winIsLongPathPrefix(zRelative+1)) ){
    zRelative++;
  }

  SimulateIOError( أرجع SQLITE_ERROR );

#ifdef __CYGWIN__
  إذا( osGetcwd ){
    zFull[nFull-1] = '\0';
    إذا( !winIsDriveLetterAndColon(zRelative) || !winIsDirSep(zRelative[2]) ){
      صحيح rc = SQLITE_OK;
      صحيح nLink = 1;              /* Number of symbolic links followed so far */
      ثابت حرف *zIn = zRelative; /* Input path for each iteration of loop */
      حرف *zDel = 0;
      بنية stat buf;

      UNUSED_PARAMETER(pVfs);

      افعل {
        /* Call lstat() on path zIn. Set bLink to true if the path
        ** is a symbolic link, or false otherwise.  */
        صحيح bLink = 0;
        إذا( osLstat && osReadlink ) {
          إذا( osLstat(zIn, &buf)!=0 ){
            صحيح myErrno = osErrno;
            إذا( myErrno!=ENOENT ){
              rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)myErrno,
                               "lstat", zIn);
            }
          }وإلا{
            bLink = ((buf.st_mode & 0170000) == 0120000);
          }

          إذا( bLink ){
            إذا( zDel==0 ){
              zDel = sqlite3MallocZero(nFull);
              إذا( zDel==0 ) rc = SQLITE_NOMEM;
            }وإلا إذا( ++nLink>SQLITE_MAX_SYMLINKS ){
              rc = SQLITE_CANTOPEN_BKPT;
            }

            إذا( rc==SQLITE_OK ){
              nByte = osReadlink(zIn, zDel, nFull-1);
              إذا( nByte ==(DWORD)-1 ){
                rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno,
                                 "readlink", zIn);
              }وإلا{
                إذا( zDel[0]!='/' ){
                  صحيح n;
                  لكل(n = sqlite3Strlen30(zIn); n>0 && zIn[n-1]!='/'; n--);
                  إذا( nByte+n+1>nFull ){
                    rc = SQLITE_CANTOPEN_BKPT;
                  }وإلا{
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

        أكد( rc!=SQLITE_OK || zIn!=zFull || zIn[0]=='/' );
        إذا( rc==SQLITE_OK && zIn!=zFull ){
          rc = mkFullPathname(zIn, zFull, nFull);
        }
        إذا( bLink==0 ) اكسر;
        zIn = zFull;
      }طالما( rc==SQLITE_OK );

      sqlite3_free(zDel);
      winSimplifyName(zFull);
      أرجع rc;
    }
  }
#endif /* __CYGWIN__ */

#إذا defined(_WIN32)
  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing. This function could fail if, for example, the
  ** current working directory has been unlinked.
  */
  SimulateIOError( أرجع SQLITE_ERROR );
  إذا ( sqlite3_data_directory && !winIsVerbatimPathname(zRelative) ){
    /*
    ** NOTE: We are dealing with a relative path name and the data
    **       directory has been set.  Therefore, use it as the basis
    **       for converting the relative path name to an absolute
    **       one by prepending the data directory and a backslash.
    */
    sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s%c%s",
                     sqlite3_data_directory, winGetDirSep(), zRelative);
    أرجع SQLITE_OK;
  }
#endif
  zConverted = winConvertFromUtf8Filename(zRelative);
  إذا( zConverted==0 ){
    أرجع SQLITE_IOERR_NOMEM_BKPT;
  }
  {
    LPWSTR zTemp;
    nByte = osGetFullPathNameW(zConverted, 0, 0, 0);
    إذا( nByte==0 ){
      sqlite3_free(zConverted);
      أرجع winLogError(SQLITE_CANTOPEN_FULLPATH, osGetLastError(),
                         "winFullPathname1", zRelative);
    }
    nByte += 3;
    zTemp = sqlite3MallocZero( (u64)nByte*حجم(WCHAR) );
    إذا( zTemp==0 ){
      sqlite3_free(zConverted);
      أرجع SQLITE_IOERR_NOMEM_BKPT;
    }
    nByte = osGetFullPathNameW(zConverted, nByte, zTemp, 0);
    إذا( nByte==0 ){
      sqlite3_free(zConverted);
      sqlite3_free(zTemp);
      أرجع winLogError(SQLITE_CANTOPEN_FULLPATH, osGetLastError(),
                         "winFullPathname2", zRelative);
    }
    sqlite3_free(zConverted);
    zOut = winUnicodeToUtf8(zTemp);
    sqlite3_free(zTemp);
  }
  إذا( zOut ){
#ifdef __CYGWIN__
    إذا( memcmp(zOut, "\\\\?\\", 4) ){
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut);
    }وإلا إذا( memcmp(zOut+4, "UNC\\", 4) ){
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut+4);
    }وإلا{
      حرف *p = zOut+6;
      *p = '\\';
      إذا( osGetcwd ){
        /* On Cygwin, UNC paths use forward slashes */
        طالما( *p ){
          إذا( *p=='\\' ) *p = '/';
          ++p;
        }
      }
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut+6);
    }
#وإلا
    sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut);
#endif /* __CYGWIN__ */
    sqlite3_free(zOut);
    أرجع SQLITE_OK;
  }وإلا{
    أرجع SQLITE_IOERR_NOMEM_BKPT;
  }
}
ساكن صحيح winFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ثابت حرف *zRelative,        /* Possibly relative input path */
  صحيح nFull,                    /* Size of output buffer in bytes */
  حرف *zFull                   /* Output buffer */
){
  صحيح rc;
  MUTEX_LOGIC( sqlite3_mutex *pMutex; )
  MUTEX_LOGIC( pMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR); )
  sqlite3_mutex_enter(pMutex);
  rc = winFullPathnameNoMutex(pVfs, zRelative, nFull, zFull);
  sqlite3_mutex_leave(pMutex);
  أرجع rc;
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
ساكن فراغ *winDlOpen(sqlite3_vfs *pVfs, ثابت حرف *zFilename){
  HANDLE h = 0;
  LPWSTR zConverted = winConvertFromUtf8Filename(zFilename);
  UNUSED_PARAMETER(pVfs);
  إذا( zConverted==0 ){
    OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (فراغ*)0));
    أرجع 0;
  }
  h = osLoadLibraryW ? osLoadLibraryW(zConverted) : 0;
  OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (فراغ*)h));
  sqlite3_free(zConverted);
  أرجع (فراغ*)h;
}
ساكن فراغ winDlError(sqlite3_vfs *pVfs, صحيح nBuf, حرف *zBufOut){
  UNUSED_PARAMETER(pVfs);
  win_Get_Last_Error_سكو(osGetLastError(), nBuf, zBufOut);
}
ساكن فراغ (*winDlSym(sqlite3_vfs *pVfs,فراغ *pH,ثابت حرف *zSym))(فراغ){
  FARPROC proc = 0;
  UNUSED_PARAMETER(pVfs);
  proc = osGetProcAddressA ? osGetProcAddressA((HANDLE)pH, zSym) : 0;
  OSTRACE(("DLSYM handle=%p, symbol=%s, address=%p\n",
           (فراغ*)pH, zSym, (فراغ*)proc));
  أرجع (فراغ(*)(فراغ))proc;
}
ساكن فراغ winDlClose(sqlite3_vfs *pVfs, فراغ *pHandle){
  UNUSED_PARAMETER(pVfs);
  إذا( osFreeLibrary!=0 ) osFreeLibrary((HANDLE)pHandle);
  OSTRACE(("DLCLOSE handle=%p\n", (فراغ*)pHandle));
}
#وإلا /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #define winDlOpen  0
  #define winDlError 0
  #define winDlSym   0
  #define winDlClose 0
#endif

/* State information for the randomness gatherer. */
تعريفنوع بنية EntropyGatherer EntropyGatherer;
بنية EntropyGatherer {
  unsigned حرف *a;   /* Gather entropy into this buffer */
  صحيح na;             /* Size of a[] in bytes */
  صحيح i;              /* XOR next input into a[i] */
  صحيح nXor;           /* Number of XOR operations done */
};

#إذا !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
/* Mix sz bytes of entropy into p. */
ساكن فراغ xorMemory(EntropyGatherer *p, unsigned حرف *x, صحيح sz){
  صحيح j, k;
  لكل(j=0, k=p->i; j<sz; j++){
    p->a[k++] ^= x[j];
    إذا( k>=p->na ) k = 0;
  }
  p->i = k;
  p->nXor += sz;
}
#endif /* !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS) */

/*
** Write up to nBuf bytes of randomness into zBuf.
*/
ساكن صحيح winRandomness(sqlite3_vfs *pVfs, صحيح nBuf, حرف *zBuf){
#إذا defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS)
  UNUSED_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  أرجع nBuf;
#وإلا
  EntropyGatherer e;
  UNUSED_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  e.a = (unsigned حرف*)zBuf;
  e.na = nBuf;
  e.nXor = 0;
  e.i = 0;
  {
    FILETIME x;
    osGetSystemTimeAsFileTime(&x);
    xorMemory(&e, (unsigned حرف*)&x, حجم(x));
  }
  {
    DWORD pid = osGetCurrentProcessId();
    xorMemory(&e, (unsigned حرف*)&pid, حجم(pid));
  }
  {
    ULONGLONG cnt = osGetTickCount64();
    xorMemory(&e, (unsigned حرف*)&cnt, حجم(cnt));
  }
  {
    LARGE_INTEGER i;
    osQueryPerformanceCounter(&i);
    xorMemory(&e, (unsigned حرف*)&i, حجم(i));
  }
#إذا SQLITE_WIN32_USE_UUID
#ifdef SQLITE_UWP
# error SQLITE_WIN32_USE_UUID is incompatible with SQLITE_UWP
#endif
  {
    UUID id;
    memset(&id, 0, حجم(id));
    osUuidCreate(&id);
    xorMemory(&e, (unsigned حرف*)&id, حجم(id));
    memset(&id, 0, حجم(UUID));
    osUuidCreateSequential(&id);
    xorMemory(&e, (unsigned حرف*)&id, حجم(id));
  }
#endif /* SQLITE_WIN32_USE_UUID */
  أرجع e.nXor>nBuf ? nBuf : e.nXor;
#endif /* defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS) */
}


/*
** Sleep for a little while.  Return the amount of time slept.
*/
ساكن صحيح winSleep(sqlite3_vfs *pVfs, صحيح microsec){
  osSleep((microsec+999)/1000);
  UNUSED_PARAMETER(pVfs);
  أرجع ((microsec+999)/1000)*1000;
}

/*
** The following variable, if set to a non-zero value, is interpreted as
** the number of seconds since 1970 and is used to set the result of
** sqlite3OsCurrentTime() during testing.
*/
#ifdef SQLITE_TEST
صحيح sqlite3_current_time = 0;  /* Fake system time in seconds since 1970. */
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
ساكن صحيح winCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *piNow){
  /* FILETIME structure is a 64-bit value representing the number of
     100-nanosecond intervals since January 1, 1601 (= JD 2305813.5).
  */
  FILETIME ft;
  ساكن ثابت sqlite3_int64 winFiletimeEpoch = 23058135*(sqlite3_int64)8640000;
#ifdef SQLITE_TEST
  ساكن ثابت sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
#endif
  /* 2^32 - to avoid use of LL and warnings in gcc */
  ساكن ثابت sqlite3_int64 max32_Bit_درج =
      (sqlite3_int64)2000000000 + (sqlite3_int64)2000000000 +
      (sqlite3_int64)294967296;

  osGetSystemTimeAsFileTime( &ft );
  *piNow = winFiletimeEpoch +
            ((((sqlite3_int64)ft.dw_سم_Date_Time)*max32_Bit_درج) +
               (sqlite3_int64)ft.dw_كس_Date_Time)/(sqlite3_int64)10000;

#ifdef SQLITE_TEST
  إذا( sqlite3_current_time ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_time + unixEpoch;
  }
#endif
  UNUSED_PARAMETER(pVfs);
  أرجع SQLITE_OK;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
ساكن صحيح winCurrentTime(sqlite3_vfs *pVfs, مزدوج *prNow){
  صحيح rc;
  sqlite3_int64 i;
  rc = winCurrentTimeInt64(pVfs, &i);
  إذا( !rc ){
    *prNow = i/86400000.0;
  }
  أرجع rc;
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
ساكن صحيح winGetLastError(sqlite3_vfs *pVfs, صحيح nBuf, حرف *zBuf){
  DWORD e = osGetLastError();
  UNUSED_PARAMETER(pVfs);
  إذا( nBuf>0 ) win_Get_Last_Error_سكو(e, nBuf, zBuf);
  أرجع e;
}

/*
** Initialize and deinitialize the operating system interface.
*/
صحيح sqlite3_os_init(فراغ){
  ساكن sqlite3_vfs winVfs = {
    3,                     /* iVersion */
    حجم(winFile),       /* szOsFile */
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
  ساكن sqlite3_vfs winLongPathVfs = {
    3,                     /* iVersion */
    حجم(winFile),       /* szOsFile */
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
  ساكن sqlite3_vfs winNolockVfs = {
    3,                     /* iVersion */
    حجم(winFile),       /* szOsFile */
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
  ساكن sqlite3_vfs winLongPathNolockVfs = {
    3,                     /* iVersion */
    حجم(winFile),       /* szOsFile */
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
  أكد( ArraySize(aSyscall)==60 );
  أكد( strcmp(aSyscall[0].zName,"AreFileApisANSI")==0 );
  أكد( strcmp(aSyscall[8].zName,"FreeLibrary")==0 );
  أكد( strcmp(aSyscall[16].zName,"GetSystemInfo")==0 );
  أكد( strcmp(aSyscall[24].zName,"HeapReAlloc")==0 );
  أكد( strcmp(aSyscall[32].zName,"MapViewOfFileFromApp")==0 );
  أكد( strcmp(aSyscall[40].zName,"UnmapViewOfFile")==0 );
  أكد( strcmp(aSyscall[48].zName,"UuidCreate")==0 );
  أكد( strcmp(aSyscall[56].zName,"lstat")==0 );

  /* get memory map allocation granularity */
  memset(&winSysInfo, 0, حجم(SYSTEM_INFO));
  osGetSystemInfo(&winSysInfo);
  أكد( winSysInfo.dwAllocationGranularity>0 );
  أكد( winSysInfo.dwPageSize>0 );

  sqlite3_vfs_register(&winVfs, 1);
  sqlite3_vfs_register(&winLongPathVfs, 0);
  sqlite3_vfs_register(&winNolockVfs, 0);
  sqlite3_vfs_register(&winLongPathNolockVfs, 0);
#ifndef SQLITE_OMIT_WAL
  winBigLock = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif

  أرجع SQLITE_OK;
}

صحيح sqlite3_os_end(فراغ){
#ifndef SQLITE_OMIT_WAL
  winBigLock = 0;
#endif
  أرجع SQLITE_OK;
}

#endif /* SQLITE_OS_WIN */
