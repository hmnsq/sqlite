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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଯଦି SQLITE_OS_WIN               /* This file is used for Windows only */

/*
** Include code that is common to all os_*.c files
*/
#ଅନ୍ତର୍ଭାଵ "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#ଅନ୍ତର୍ଭାଵ "os_win.h"

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
                            (ଆକାର(WCHAR) * SQLITE_WINNT_MAX_PATH_CHARS)
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
#ifndef UNUSED_VARIABLE_ମୂଲ୍ୟ
#  define UNUSED_VARIABLE_ମୂଲ୍ୟ(x)      (ଶୂନ୍ୟ)(x)
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା winShm winShm;           /* A connection to shared-memory */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା win_Shm_ଗଣ୍ଠି win_Shm_ଗଣ୍ଠି;   /* A region of shared-memory */
#endif

/*
** The winFile structure is a subclass of sqlite3_file* specific to the win32
** portability layer.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା winFile winFile;
ସଂରଚନା winFile {
  ଅଚଳ sqlite3_io_methods *pMethod; /*** Must be first ***/
  sqlite3_vfs *pVfs;      /* The VFS used to open this file */
  HANDLE h;               /* Handle for accessing the file */
  u8 locktype;            /* Type of lock currently held on this file */
  short sharedLockByte;   /* Randomly chosen byte used as a shared lock */
  u8 ctrlFlags;           /* Flags.  See WINFILE_* below */
  DWORD lastErrno;        /* The Windows errno from the last I/O error */
#ifndef SQLITE_OMIT_WAL
  winShm *pShm;           /* Instance of shared memory on this file */
#endif
  ଅଚଳ ଅକ୍ଷର *zPath;      /* Full pathname of this file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szChunk;            /* Chunk size configured by FCNTL_CHUNK_SIZE */
#ଯଦି SQLITE_MAX_MMAP_SIZE>0
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFetchOut;                /* Number of outstanding xFetch references */
  HANDLE hMap;                  /* Handle for accessing memory mapping */
  ଶୂନ୍ୟ *pMapRegion;             /* Area memory mapped */
  sqlite3_int64 mmapSize;       /* Size of mapped region */
  sqlite3_int64 mmapSizeMax;    /* Configured FCNTL_MMAP_SIZE value */
#endif
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  DWORD iBusyTimeout;        /* Wait this many millisec on locks */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bBlockOnConnect;
#endif
};

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
# define winFileBusyTimeout(pDbFd) pDbFd->iBusyTimeout
#ଅନ୍ୟଥା
# define winFileBusyTimeout(pDbFd) 0
#endif

/*
** The winVfsAppData structure is used for the pAppData member for all of the
** Win32 VFS variants.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା winVfsAppData winVfsAppData;
ସଂରଚନା winVfsAppData {
  ଅଚଳ sqlite3_io_methods *pMethod; /* The file I/O methods to use. */
  ଶୂନ୍ୟ *pAppData;                    /* The extra pAppData, if any. */
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
#  define SQLITE_WIN32_DBG_BUF_SIZE   ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(4096-ଆକାର(DWORD)))
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
#  ଯଦି SQLITE_DEFAULT_CACHE_SIZE>=0
#    define SQLITE_WIN32_CACHE_SIZE     (SQLITE_DEFAULT_CACHE_SIZE)
#  ଅନ୍ୟଥା
#    define SQLITE_WIN32_CACHE_SIZE     (-(SQLITE_DEFAULT_CACHE_SIZE))
#  endif
#endif

/*
 * Make sure that the calculated cache size, in pages, cannot cause the
 * initial size of the Win32-specific heap to exceed the maximum amount
 * of memory that can be specified in the call to HeapCreate.
 */
#ଯଦି SQLITE_WIN32_CACHE_SIZE>SQLITE_WIN32_MAX_CACHE_SIZE
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା winMemData winMemData;
ସଂରଚନା winMemData {
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

ସ୍ଥିର ସଂରଚନା winMemData win_mem_data = {
#ifndef NDEBUG
  WINMEM_MAGIC1,
#endif
  ଖାଲି, FALSE
#ifndef NDEBUG
  ,WINMEM_MAGIC2
#endif
};

#ifndef NDEBUG
#define winMemAssertMagic1() ପ୍ରତିଜ୍ଞା( win_mem_data.magic1==WINMEM_MAGIC1 )
#define winMemAssertMagic2() ପ୍ରତିଜ୍ଞା( win_mem_data.magic2==WINMEM_MAGIC2 )
#define winMemAssertMagic()  winMemAssertMagic1(); winMemAssertMagic2();
#ଅନ୍ୟଥା
#define winMemAssertMagic()
#endif

#define winMemGetDataPtr()  &win_mem_data
#define winMemGetHeap()     win_mem_data.hHeap
#define winMemGetOwned()    win_mem_data.bOwned

ସ୍ଥିର ଶୂନ୍ୟ *winMemMalloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBytes);
ସ୍ଥିର ଶୂନ୍ୟ winMemFree(ଶୂନ୍ୟ *pPrior);
ସ୍ଥିର ଶୂନ୍ୟ *winMemRealloc(ଶୂନ୍ୟ *pPrior, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBytes);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winMemSize(ଶୂନ୍ୟ *p);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winMemRoundup(ପୂର୍ଣ୍ଣସଂଖ୍ୟା n);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winMemInit(ଶୂନ୍ୟ *pAppData);
ସ୍ଥିର ଶୂନ୍ୟ winMemShutdown(ଶୂନ୍ୟ *pAppData);

ଅଚଳ sqlite3_mem_methods *sqlite3MemGetWin32(ଶୂନ୍ୟ);
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
ସ୍ଥିର ସଂରଚନା win_syscall {
  ଅଚଳ ଅକ୍ଷର *zName;            /* Name of the system call */
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

#ଯଦି defined(SQLITE_UWP)
  { "CreateFileMappingFromApp",(SYSCALL)CreateFileMappingFromApp,0 },
#ଅନ୍ୟଥା
  { "CreateFileMappingFromApp",(SYSCALL)0,                       0 },
#endif
#define osCreateFileMappingFromApp ((HANDLE(WINAPI*)(HANDLE,\
        PSECURITY_ATTRIBUTES, \
        ULONG,ULONG64,PCWSTR))aSyscall[3].pCurrent)

#ଯଦି !defined(SQLITE_UWP)
  { "CreateFileMappingW",      (SYSCALL)CreateFileMappingW,      0 },
#ଅନ୍ୟଥା
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

#ଯଦି !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "FreeLibrary",             (SYSCALL)FreeLibrary,             0 },
#ଅନ୍ୟଥା
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

#ଯଦି !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "GetProcAddressA",         (SYSCALL)GetProcAddress,          0 },
#ଅନ୍ୟଥା
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
#ଅନ୍ୟଥା
  { "GetTempPathW",            (SYSCALL)GetTempPathW,            0 },
#endif
#define osGetTempPathW ((DWORD(WINAPI*)(DWORD,LPWSTR))aSyscall[18].pCurrent)

  { "GetTickCount64",          (SYSCALL)GetTickCount64,          0 },
#define osGetTickCount64 ((ULONGLONG(WINAPI*)(VOID))aSyscall[19].pCurrent)

#ifdef SQLITE_UWP
  { "HeapAlloc",               (SYSCALL)0,                       0 },
#ଅନ୍ୟଥା
  { "HeapAlloc",               (SYSCALL)HeapAlloc,               0 },
#endif
#define osHeapAlloc ((LPVOID(WINAPI*)(HANDLE,DWORD, \
        SIZE_T))aSyscall[20].pCurrent)

#ifdef SQLITE_UWP
  { "HeapCreate",              (SYSCALL)0,                       0 },
#ଅନ୍ୟଥା
  { "HeapCreate",              (SYSCALL)HeapCreate,              0 },
#endif
#define osHeapCreate ((HANDLE(WINAPI*)(DWORD,SIZE_T, \
        SIZE_T))aSyscall[21].pCurrent)

#ifdef SQLITE_UWP
  { "HeapDestroy",             (SYSCALL)0,                       0 },
#ଅନ୍ୟଥା
  { "HeapDestroy",             (SYSCALL)HeapDestroy,             0 },
#endif
#define osHeapDestroy ((BOOL(WINAPI*)(HANDLE))aSyscall[22].pCurrent)

#ifdef SQLITE_UWP
  { "HeapFree",                (SYSCALL)0,                       0 },
#ଅନ୍ୟଥା
  { "HeapFree",                (SYSCALL)HeapFree,                0 },
#endif
#define osHeapFree ((BOOL(WINAPI*)(HANDLE,DWORD,LPVOID))aSyscall[23].pCurrent)

#ifdef SQLITE_UWP
  { "HeapReAlloc",             (SYSCALL)0,                       0 },
#ଅନ୍ୟଥା
  { "HeapReAlloc",             (SYSCALL)HeapReAlloc,             0 },
#endif
#define osHeapReAlloc ((LPVOID(WINAPI*)(HANDLE,DWORD,LPVOID, \
        SIZE_T))aSyscall[24].pCurrent)

#ifdef SQLITE_UWP
  { "HeapSize",                (SYSCALL)0,                       0 },
#ଅନ୍ୟଥା
  { "HeapSize",                (SYSCALL)HeapSize,                0 },
#endif
#define osHeapSize ((SIZE_T(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[25].pCurrent)

#ifdef SQLITE_UWP
  { "HeapValidate",            (SYSCALL)0,                       0 },
#ଅନ୍ୟଥା
  { "HeapValidate",            (SYSCALL)HeapValidate,            0 },
#endif
#define osHeapValidate ((BOOL(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[26].pCurrent)

#ifdef SQLITE_UWP
  { "HeapCompact",             (SYSCALL)0,                       0 },
#ଅନ୍ୟଥା
  { "HeapCompact",             (SYSCALL)HeapCompact,             0 },
#endif
#define osHeapCompact ((UINT(WINAPI*)(HANDLE,DWORD))aSyscall[27].pCurrent)


#ଯଦି !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "LoadLibraryW",            (SYSCALL)LoadLibraryW,            0 },
#ଅନ୍ୟଥା
  { "LoadLibraryW",            (SYSCALL)0,                       0 },
#endif
#define osLoadLibraryW ((HMODULE(WINAPI*)(LPCWSTR))aSyscall[28].pCurrent)

  { "LocalFree",               (SYSCALL)LocalFree,               0 },
#define osLocalFree ((HLOCAL(WINAPI*)(HLOCAL))aSyscall[29].pCurrent)

  { "LockFileEx",              (SYSCALL)LockFileEx,              0 },
#define osLockFileEx ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[30].pCurrent)

#ଯଦି (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0) \
 && !defined(SQLITE_UWP)
  { "MapViewOfFile",           (SYSCALL)MapViewOfFile,           0 },
#ଅନ୍ୟଥା
  { "MapViewOfFile",           (SYSCALL)0,                       0 },
#endif
#define osMapViewOfFile ((LPVOID(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        SIZE_T))aSyscall[31].pCurrent)

#ଯଦି (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0) \
 && defined(SQLITE_UWP)
  { "MapViewOfFileFromApp",    (SYSCALL)MapViewOfFileFromApp,   0 },
#ଅନ୍ୟଥା
  { "MapViewOfFileFromApp",    (SYSCALL)0,                      0 },
#endif
#define osMapViewOfFileFromApp ((LPVOID(WINAPI*)(HANDLE,ULONG,ULONG64, \
        SIZE_T))aSyscall[32].pCurrent)

  { "MultiByteToWideChar",     (SYSCALL)MultiByteToWideChar,     0 },
#define osMultiByteToWideChar ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା(WINAPI*)(UINT,DWORD,LPCSTR,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,LPWSTR, \
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା))aSyscall[33].pCurrent)

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

#ଯଦି !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0
  { "UnmapViewOfFile",         (SYSCALL)UnmapViewOfFile,         0 },
#ଅନ୍ୟଥା
  { "UnmapViewOfFile",         (SYSCALL)0,                       0 },
#endif
#define osUnmapViewOfFile ((BOOL(WINAPI*)(LPCVOID))aSyscall[40].pCurrent)

  { "WideCharToMultiByte",     (SYSCALL)WideCharToMultiByte,     0 },
#define osWideCharToMultiByte ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା(WINAPI*)(UINT,DWORD,LPCWSTR,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,LPSTR,ପୂର୍ଣ୍ଣସଂଖ୍ୟା, \
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
#ଯଦି defined(InterlockedCompareExchange)
  { "InterlockedCompareExchange", (SYSCALL)0,                    0 },
#define osInterlockedCompareExchange InterlockedCompareExchange
#ଅନ୍ୟଥା
  { "InterlockedCompareExchange", (SYSCALL)InterlockedCompareExchange, 0 },
#define osInterlockedCompareExchange ((LONG(WINAPI*)(LONG ଅସ୍ଥିର*,\
        LONG,LONG))aSyscall[47].pCurrent)
#endif /* defined(InterlockedCompareExchange) */

#ଯଦି SQLITE_WIN32_USE_UUID
  { "UuidCreate",               (SYSCALL)UuidCreate,             0 },
#ଅନ୍ୟଥା
  { "UuidCreate",               (SYSCALL)0,                      0 },
#endif
#define osUuidCreate ((RPC_STATUS(RPC_ENTRY*)(UUID*))aSyscall[48].pCurrent)

#ଯଦି SQLITE_WIN32_USE_UUID
  { "UuidCreateSequential",     (SYSCALL)UuidCreateSequential,   0 },
#ଅନ୍ୟଥା
  { "UuidCreateSequential",     (SYSCALL)0,                      0 },
#endif
#define osUuidCreateSequential \
        ((RPC_STATUS(RPC_ENTRY*)(UUID*))aSyscall[49].pCurrent)

#ଯଦି !defined(SQLITE_NO_SYNC) && SQLITE_MAX_MMAP_SIZE>0
  { "FlushViewOfFile",          (SYSCALL)FlushViewOfFile,        0 },
#ଅନ୍ୟଥା
  { "FlushViewOfFile",          (SYSCALL)0,                      0 },
#endif
#define osFlushViewOfFile \
        ((BOOL(WINAPI*)(LPCVOID,SIZE_T))aSyscall[50].pCurrent)

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  { "CreateEvent",              (SYSCALL)CreateEvent,            0 },
#ଅନ୍ୟଥା
  { "CreateEvent",              (SYSCALL)0,                      0 },
#endif
#define osCreateEvent ((HANDLE(WINAPI*)(LPSECURITY_ATTRIBUTES,BOOL, \
                        BOOL,LPCSTR))aSyscall[51].pCurrent)

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  { "CancelIo",                 (SYSCALL)CancelIo,               0 },
#ଅନ୍ୟଥା
  { "CancelIo",                 (SYSCALL)0,                      0 },
#endif
#define osCancelIo ((BOOL(WINAPI*)(HANDLE))aSyscall[52].pCurrent)

#ifndef _WIN32
  { "getenv",                   (SYSCALL)getenv,                 0 },
#ଅନ୍ୟଥା
  { "getenv",                   (SYSCALL)0,                      0 },
#endif
#define osGetenv ((ଅଚଳ ଅକ୍ଷର *(*)(ଅଚଳ ଅକ୍ଷର *))aSyscall[53].pCurrent)

#ifndef _WIN32
  { "getcwd",                   (SYSCALL)getcwd,                 0 },
#ଅନ୍ୟଥା
  { "getcwd",                   (SYSCALL)0,                      0 },
#endif
#define osGetcwd ((ଅକ୍ଷର*(*)(ଅକ୍ଷର*,size_t))aSyscall[54].pCurrent)

#ifndef _WIN32
  { "readlink",                 (SYSCALL)readlink,               0 },
#ଅନ୍ୟଥା
  { "readlink",                 (SYSCALL)0,                      0 },
#endif
#define osReadlink ((ssize_t(*)(ଅଚଳ ଅକ୍ଷର*,ଅକ୍ଷର*,size_t))aSyscall[55].pCurrent)

#ifndef _WIN32
  { "lstat",                    (SYSCALL)lstat,                  0 },
#ଅନ୍ୟଥା
  { "lstat",                    (SYSCALL)0,                      0 },
#endif
#define osLstat ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ଅଚଳ ଅକ୍ଷର*,ସଂରଚନା stat*))aSyscall[56].pCurrent)

#ifndef _WIN32
  { "__errno",                  (SYSCALL)__errno,                0 },
#ଅନ୍ୟଥା
  { "__errno",                  (SYSCALL)0,                      0 },
#endif
#define osErrno (*((ପୂର୍ଣ୍ଣସଂଖ୍ୟା*(*)(ଶୂନ୍ୟ))aSyscall[57].pCurrent)())

#ifndef _WIN32
  { "cygwin_conv_path",         (SYSCALL)cygwin_conv_path,       0 },
#ଅନ୍ୟଥା
  { "cygwin_conv_path",         (SYSCALL)0,                      0 },
#endif
#define osCygwin_conv_path ((size_t(*)(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା, \
    ଅଚଳ ଶୂନ୍ୟ *, ଶୂନ୍ୟ *, size_t))aSyscall[58].pCurrent)

  { "GetDriveTypeW",            (SYSCALL)GetDriveTypeW,          0 },
#define osGetDriveTypeW ((UINT(WINAPI*)(LPCWSTR))aSyscall[59].pCurrent)

}; /* End of the overrideable system calls */

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "win32" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winSetSystemCall(
  sqlite3_vfs *pNotUsed,        /* The VFS pointer.  Not used */
  ଅଚଳ ଅକ୍ଷର *zName,            /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
){
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_NOTFOUND;

  UNUSED_PARAMETER(pNotUsed);
  ଯଦି( zName==0 ){
    /* If no zName is given, restore all system calls to their default
    ** settings and return NULL
    */
    rc = SQLITE_OK;
    ପାଇଁ(i=0; i<ଆକାର(aSyscall)/ଆକାର(aSyscall[0]); i++){
      ଯଦି( aSyscall[i].pDefault ){
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  }ଅନ୍ୟଥା{
    /* If zName is specified, operate on only the one system call
    ** specified.
    */
    ପାଇଁ(i=0; i<ଆକାର(aSyscall)/ଆକାର(aSyscall[0]); i++){
      ଯଦି( strcmp(zName, aSyscall[i].zName)==0 ){
        ଯଦି( aSyscall[i].pDefault==0 ){
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = SQLITE_OK;
        ଯଦି( pNewFunc==0 ) pNewFunc = aSyscall[i].pDefault;
        aSyscall[i].pCurrent = pNewFunc;
        ଭାଙ୍ଗ;
      }
    }
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
ସ୍ଥିର sqlite3_syscall_ptr winGetSystemCall(
  sqlite3_vfs *pNotUsed,
  ଅଚଳ ଅକ୍ଷର *zName
){
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  UNUSED_PARAMETER(pNotUsed);
  ପାଇଁ(i=0; i<ଆକାର(aSyscall)/ଆକାର(aSyscall[0]); i++){
    ଯଦି( strcmp(zName, aSyscall[i].zName)==0 ) ଫେରନ୍ତୁ aSyscall[i].pCurrent;
  }
  ଫେରନ୍ତୁ 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *winNextSystemCall(sqlite3_vfs *p, ଅଚଳ ଅକ୍ଷର *zName){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = -1;

  UNUSED_PARAMETER(p);
  ଯଦି( zName ){
    ପାଇଁ(i=0; i<ArraySize(aSyscall)-1; i++){
      ଯଦି( strcmp(zName, aSyscall[i].zName)==0 ) ଭାଙ୍ଗ;
    }
  }
  ପାଇଁ(i++; i<ArraySize(aSyscall); i++){
    ଯଦି( aSyscall[i].pCurrent!=0 ) ଫେରନ୍ତୁ aSyscall[i].zName;
  }
  ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_win32_compact_heap(LPUINT pnLargest){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  UINT nLargest = 0;
  HANDLE hHeap;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  ପ୍ରତିଜ୍ଞା( hHeap!=0 );
  ପ୍ରତିଜ୍ଞା( hHeap!=INVALID_HANDLE_ମୂଲ୍ୟ );
#ଯଦି defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ପ୍ରତିଜ୍ଞା( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, ଖାଲି) );
#endif
  ଯଦି( (nLargest=osHeapCompact(hHeap, SQLITE_WIN32_HEAP_FLAGS))==0 ){
    DWORD lastErrno = osGetLastError();
    ଯଦି( lastErrno==NO_ERROR ){
      sqlite3_log(SQLITE_NOMEM, "failed to HeapCompact (no space), heap=%p",
                  (ଶୂନ୍ୟ*)hHeap);
      rc = SQLITE_NOMEM_BKPT;
    }ଅନ୍ୟଥା{
      sqlite3_log(SQLITE_ERROR, "failed to HeapCompact (%lu), heap=%p",
                  osGetLastError(), (ଶୂନ୍ୟ*)hHeap);
      rc = SQLITE_ERROR;
    }
  }
  ଯଦି( pnLargest ) *pnLargest = nLargest;
  ଫେରନ୍ତୁ rc;
}

/*
** If a Win32 native heap has been configured, this function will attempt to
** destroy and recreate it.  If the Win32 native heap is not isolated and/or
** the sqlite3_memory_used() function does not return zero, SQLITE_BUSY will
** be returned and no changes will be made to the Win32 native heap.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_win32_reset_heap(){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; ) /* The main static mutex */
  MUTEX_LOGIC( sqlite3_mutex *pMem; )    /* The memsys static mutex */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  MUTEX_LOGIC( pMem = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3_mutex_enter(pMem);
  winMemAssertMagic();
  ଯଦି( winMemGetHeap()!=ଖାଲି && winMemGetOwned() && sqlite3_memory_used()==0 ){
    /*
    ** At this point, there should be no outstanding memory allocations on
    ** the heap.  Also, since both the main and memsys locks are currently
    ** being held by us, no other function (i.e. from another thread) should
    ** be able to even access the heap.  Attempt to destroy and recreate our
    ** isolated Win32 native heap now.
    */
    ପ୍ରତିଜ୍ଞା( winMemGetHeap()!=ଖାଲି );
    ପ୍ରତିଜ୍ଞା( winMemGetOwned() );
    ପ୍ରତିଜ୍ଞା( sqlite3_memory_used()==0 );
    winMemShutdown(winMemGetDataPtr());
    ପ୍ରତିଜ୍ଞା( winMemGetHeap()==ଖାଲି );
    ପ୍ରତିଜ୍ଞା( !winMemGetOwned() );
    ପ୍ରତିଜ୍ଞା( sqlite3_memory_used()==0 );
    rc = winMemInit(winMemGetDataPtr());
    ପ୍ରତିଜ୍ଞା( rc!=SQLITE_OK || winMemGetHeap()!=ଖାଲି );
    ପ୍ରତିଜ୍ଞା( rc!=SQLITE_OK || winMemGetOwned() );
    ପ୍ରତିଜ୍ଞା( rc!=SQLITE_OK || sqlite3_memory_used()==0 );
  }ଅନ୍ୟଥା{
    /*
    ** The Win32 native heap cannot be modified because it may be in use.
    */
    rc = SQLITE_BUSY;
  }
  sqlite3_mutex_leave(pMem);
  sqlite3_mutex_leave(pMainMtx);
  ଫେରନ୍ତୁ rc;
}
#endif /* SQLITE_WIN32_MALLOC */

#ifdef _WIN32
/*
** This function outputs the specified (ANSI) string to the Win32 debugger
** (if available).  Undocumented.  Might go away at any moment.
*/
ଶୂନ୍ୟ sqlite3_win32_write_debug(ଅଚଳ ଅକ୍ଷର *zBuf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf){
  ଅକ୍ଷର zDbgBuf[SQLITE_WIN32_DBG_BUF_SIZE];
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMin = MIN(nBuf, (SQLITE_WIN32_DBG_BUF_SIZE - 1)); /* may be negative. */
  ଯଦି( nMin<-1 ) nMin = -1; /* all negative values become -1. */
  ପ୍ରତିଜ୍ଞା( nMin==-1 || nMin==0 || nMin<SQLITE_WIN32_DBG_BUF_SIZE );
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !zBuf ){
    (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
    ଫେରନ୍ତୁ;
  }
#endif
  ଯଦି( nMin>0 ){
    memset(zDbgBuf, 0, SQLITE_WIN32_DBG_BUF_SIZE);
    memcpy(zDbgBuf, zBuf, nMin);
    osOutputDebugStringA(zDbgBuf);
  }ଅନ୍ୟଥା{
    osOutputDebugStringA(zBuf);
  }
}

ଶୂନ୍ୟ sqlite3_win32_sleep(DWORD milliseconds){
  osSleep(milliseconds);
}

#ଯଦି SQLITE_MAX_WORKER_THREADS>0 && SQLITE_THREADSAFE>0
DWORD sqlite3Win32Wait(HANDLE hObject){
  DWORD rc;
  ଯେତେବେଳେ( (rc = osWaitForSingleObjectEx(hObject, INFINITE,
                                       TRUE))==WAIT_IO_COMPLETION ){}
  ଫେରନ୍ତୁ rc;
}
#endif
#endif /* _WIN32 */

#ifdef SQLITE_WIN32_MALLOC
/*
** Allocate nBytes of memory.
*/
ସ୍ଥିର ଶୂନ୍ୟ *winMemMalloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBytes){
  HANDLE hHeap;
  ଶୂନ୍ୟ *p;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  ପ୍ରତିଜ୍ଞା( hHeap!=0 );
  ପ୍ରତିଜ୍ଞା( hHeap!=INVALID_HANDLE_ମୂଲ୍ୟ );
#ଯଦି defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ପ୍ରତିଜ୍ଞା( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, ଖାଲି) );
#endif
  ପ୍ରତିଜ୍ଞା( nBytes>=0 );
  p = osHeapAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, (SIZE_T)nBytes);
  ଯଦି( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapAlloc %u bytes (%lu), heap=%p",
                nBytes, osGetLastError(), (ଶୂନ୍ୟ*)hHeap);
  }
  ଫେରନ୍ତୁ p;
}

/*
** Free memory.
*/
ସ୍ଥିର ଶୂନ୍ୟ winMemFree(ଶୂନ୍ୟ *pPrior){
  HANDLE hHeap;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  ପ୍ରତିଜ୍ଞା( hHeap!=0 );
  ପ୍ରତିଜ୍ଞା( hHeap!=INVALID_HANDLE_ମୂଲ୍ୟ );
#ଯଦି defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ପ୍ରତିଜ୍ଞା( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) );
#endif
  ଯଦି( !pPrior ) ଫେରନ୍ତୁ; /* Passing NULL to HeapFree is undefined. */
  ଯଦି( !osHeapFree(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapFree block %p (%lu), heap=%p",
                pPrior, osGetLastError(), (ଶୂନ୍ୟ*)hHeap);
  }
}

/*
** Change the size of an existing memory allocation
*/
ସ୍ଥିର ଶୂନ୍ୟ *winMemRealloc(ଶୂନ୍ୟ *pPrior, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBytes){
  HANDLE hHeap;
  ଶୂନ୍ୟ *p;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  ପ୍ରତିଜ୍ଞା( hHeap!=0 );
  ପ୍ରତିଜ୍ଞା( hHeap!=INVALID_HANDLE_ମୂଲ୍ୟ );
#ଯଦି defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ପ୍ରତିଜ୍ଞା( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) );
#endif
  ପ୍ରତିଜ୍ଞା( nBytes>=0 );
  ଯଦି( !pPrior ){
    p = osHeapAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, (SIZE_T)nBytes);
  }ଅନ୍ୟଥା{
    p = osHeapReAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior, (SIZE_T)nBytes);
  }
  ଯଦି( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to %s %u bytes (%lu), heap=%p",
                pPrior ? "HeapReAlloc" : "HeapAlloc", nBytes, osGetLastError(),
                (ଶୂନ୍ୟ*)hHeap);
  }
  ଫେରନ୍ତୁ p;
}

/*
** Return the size of an outstanding allocation, in bytes.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winMemSize(ଶୂନ୍ୟ *p){
  HANDLE hHeap;
  SIZE_T n;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  ପ୍ରତିଜ୍ଞା( hHeap!=0 );
  ପ୍ରତିଜ୍ଞା( hHeap!=INVALID_HANDLE_ମୂଲ୍ୟ );
#ଯଦି defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ପ୍ରତିଜ୍ଞା( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, p) );
#endif
  ଯଦି( !p ) ଫେରନ୍ତୁ 0;
  n = osHeapSize(hHeap, SQLITE_WIN32_HEAP_FLAGS, p);
  ଯଦି( n==(SIZE_T)-1 ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapSize block %p (%lu), heap=%p",
                p, osGetLastError(), (ଶୂନ୍ୟ*)hHeap);
    ଫେରନ୍ତୁ 0;
  }
  ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n;
}

/*
** Round up a request size to the next valid allocation size.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winMemRoundup(ପୂର୍ଣ୍ଣସଂଖ୍ୟା n){
  ଫେରନ୍ତୁ n;
}

/*
** Initialize this module.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winMemInit(ଶୂନ୍ୟ *pAppData){
  winMemData *pWinMemData = (winMemData *)pAppData;

  ଯଦି( !pWinMemData ) ଫେରନ୍ତୁ SQLITE_ERROR;
  ପ୍ରତିଜ୍ଞା( pWinMemData->magic1==WINMEM_MAGIC1 );
  ପ୍ରତିଜ୍ଞା( pWinMemData->magic2==WINMEM_MAGIC2 );

#ଯଦି SQLITE_WIN32_HEAP_CREATE
  ଯଦି( !pWinMemData->hHeap ){
    DWORD dwInitialSize = SQLITE_WIN32_HEAP_INIT_SIZE;
    DWORD dwMaximumSize = (DWORD)sqlite3GlobalConfig.nHeap;
    ଯଦି( dwMaximumSize==0 ){
      dwMaximumSize = SQLITE_WIN32_HEAP_MAX_SIZE;
    }ଅନ୍ୟଥା ଯଦି( dwInitialSize>dwMaximumSize ){
      dwInitialSize = dwMaximumSize;
    }
    pWinMemData->hHeap = osHeapCreate(SQLITE_WIN32_HEAP_FLAGS,
                                      dwInitialSize, dwMaximumSize);
    ଯଦି( !pWinMemData->hHeap ){
      sqlite3_log(SQLITE_NOMEM,
          "failed to HeapCreate (%lu), flags=%u, initSize=%lu, maxSize=%lu",
          osGetLastError(), SQLITE_WIN32_HEAP_FLAGS, dwInitialSize,
          dwMaximumSize);
      ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
    }
    pWinMemData->bOwned = TRUE;
    ପ୍ରତିଜ୍ଞା( pWinMemData->bOwned );
  }
#ଅନ୍ୟଥା
  pWinMemData->hHeap = osGetProcessHeap();
  ଯଦି( !pWinMemData->hHeap ){
    sqlite3_log(SQLITE_NOMEM,
        "failed to GetProcessHeap (%lu)", osGetLastError());
    ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  }
  pWinMemData->bOwned = FALSE;
  ପ୍ରତିଜ୍ଞା( !pWinMemData->bOwned );
#endif
  ପ୍ରତିଜ୍ଞା( pWinMemData->hHeap!=0 );
  ପ୍ରତିଜ୍ଞା( pWinMemData->hHeap!=INVALID_HANDLE_ମୂଲ୍ୟ );
#ଯଦି defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ପ୍ରତିଜ୍ଞା( osHeapValidate(pWinMemData->hHeap, SQLITE_WIN32_HEAP_FLAGS, ଖାଲି) );
#endif
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ସ୍ଥିର ଶୂନ୍ୟ winMemShutdown(ଶୂନ୍ୟ *pAppData){
  winMemData *pWinMemData = (winMemData *)pAppData;

  ଯଦି( !pWinMemData ) ଫେରନ୍ତୁ;
  ପ୍ରତିଜ୍ଞା( pWinMemData->magic1==WINMEM_MAGIC1 );
  ପ୍ରତିଜ୍ଞା( pWinMemData->magic2==WINMEM_MAGIC2 );

  ଯଦି( pWinMemData->hHeap ){
    ପ୍ରତିଜ୍ଞା( pWinMemData->hHeap!=INVALID_HANDLE_ମୂଲ୍ୟ );
#ଯଦି defined(SQLITE_WIN32_MALLOC_VALIDATE)
    ପ୍ରତିଜ୍ଞା( osHeapValidate(pWinMemData->hHeap, SQLITE_WIN32_HEAP_FLAGS, ଖାଲି) );
#endif
    ଯଦି( pWinMemData->bOwned ){
      ଯଦି( !osHeapDestroy(pWinMemData->hHeap) ){
        sqlite3_log(SQLITE_NOMEM, "failed to HeapDestroy (%lu), heap=%p",
                    osGetLastError(), (ଶୂନ୍ୟ*)pWinMemData->hHeap);
      }
      pWinMemData->bOwned = FALSE;
    }
    pWinMemData->hHeap = ଖାଲି;
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
ଅଚଳ sqlite3_mem_methods *sqlite3MemGetWin32(ଶୂନ୍ୟ){
  ସ୍ଥିର ଅଚଳ sqlite3_mem_methods winMemMethods = {
    winMemMalloc,
    winMemFree,
    winMemRealloc,
    winMemSize,
    winMemRoundup,
    winMemInit,
    winMemShutdown,
    &win_mem_data
  };
  ଫେରନ୍ତୁ &winMemMethods;
}

ଶୂନ୍ୟ sqlite3MemSetDefault(ଶୂନ୍ୟ){
  sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetWin32());
}
#endif /* SQLITE_WIN32_MALLOC */

#ifdef _WIN32
/*
** Convert a UTF-8 string to Microsoft Unicode.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ସ୍ଥିର LPWSTR winUtf8ToUnicode(ଅଚଳ ଅକ୍ଷର *zText){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nChar;
  LPWSTR zWideText;

  nChar = osMultiByteToWideChar(CP_UTF8, 0, zText, -1, ଖାଲି, 0);
  ଯଦି( nChar==0 ){
    ଫେରନ୍ତୁ 0;
  }
  zWideText = sqlite3MallocZero( nChar*ଆକାର(WCHAR) );
  ଯଦି( zWideText==0 ){
    ଫେରନ୍ତୁ 0;
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zText, -1, zWideText,
                                nChar);
  ଯଦି( nChar==0 ){
    sqlite3_free(zWideText);
    zWideText = 0;
  }
  ଫେରନ୍ତୁ zWideText;
}
#endif /* _WIN32 */

/*
** Convert a Microsoft Unicode string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ସ୍ଥିର ଅକ୍ଷର *winUnicodeToUtf8(LPCWSTR zWideText){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;
  ଅକ୍ଷର *zText;

  nByte = osWideCharToMultiByte(CP_UTF8, 0, zWideText, -1, 0, 0, 0, 0);
  ଯଦି( nByte == 0 ){
    ଫେରନ୍ତୁ 0;
  }
  zText = sqlite3MallocZero( nByte );
  ଯଦି( zText==0 ){
    ଫେରନ୍ତୁ 0;
  }
  nByte = osWideCharToMultiByte(CP_UTF8, 0, zWideText, -1, zText, nByte,
                                0, 0);
  ଯଦି( nByte == 0 ){
    sqlite3_free(zText);
    zText = 0;
  }
  ଫେରନ୍ତୁ zText;
}

#ifdef _WIN32
/*
** Convert an ANSI string to Microsoft Unicode, using the ANSI or OEM
** code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ସ୍ଥିର LPWSTR winMbcsToUnicode(ଅଚଳ ଅକ୍ଷର *zText, ପୂର୍ଣ୍ଣସଂଖ୍ୟା useAnsi){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nWideChar;
  LPWSTR zMbcsText;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nWideChar = osMultiByteToWideChar(codepage, 0, zText, -1, ଖାଲି,
                                0);
  ଯଦି( nWideChar==0 ){
    ଫେରନ୍ତୁ 0;
  }
  zMbcsText = sqlite3MallocZero( nWideChar*ଆକାର(WCHAR) );
  ଯଦି( zMbcsText==0 ){
    ଫେରନ୍ତୁ 0;
  }
  nWideChar = osMultiByteToWideChar(codepage, 0, zText, -1, zMbcsText,
                                nWideChar);
  ଯଦି( nWideChar==0 ){
    sqlite3_free(zMbcsText);
    zMbcsText = 0;
  }
  ଫେରନ୍ତୁ zMbcsText;
}

/*
** Convert a Microsoft Unicode string to a multi-byte character string,
** using the ANSI or OEM code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ସ୍ଥିର ଅକ୍ଷର *winUnicodeToMbcs(LPCWSTR zWideText, ପୂର୍ଣ୍ଣସଂଖ୍ୟା useAnsi){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;
  ଅକ୍ଷର *zText;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nByte = osWideCharToMultiByte(codepage, 0, zWideText, -1, 0, 0, 0, 0);
  ଯଦି( nByte == 0 ){
    ଫେରନ୍ତୁ 0;
  }
  zText = sqlite3MallocZero( nByte );
  ଯଦି( zText==0 ){
    ଫେରନ୍ତୁ 0;
  }
  nByte = osWideCharToMultiByte(codepage, 0, zWideText, -1, zText,
                                nByte, 0, 0);
  ଯଦି( nByte == 0 ){
    sqlite3_free(zText);
    zText = 0;
  }
  ଫେରନ୍ତୁ zText;
}

/*
** Convert a multi-byte character string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ସ୍ଥିର ଅକ୍ଷର *winMbcsToUtf8(ଅଚଳ ଅକ୍ଷର *zText, ପୂର୍ଣ୍ଣସଂଖ୍ୟା useAnsi){
  ଅକ୍ଷର *zTextUtf8;
  LPWSTR zTmpWide;

  zTmpWide = winMbcsToUnicode(zText, useAnsi);
  ଯଦି( zTmpWide==0 ){
    ଫେରନ୍ତୁ 0;
  }
  zTextUtf8 = winUnicodeToUtf8(zTmpWide);
  sqlite3_free(zTmpWide);
  ଫେରନ୍ତୁ zTextUtf8;
}

/*
** Convert a UTF-8 string to a multi-byte character string.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ସ୍ଥିର ଅକ୍ଷର *winUtf8ToMbcs(ଅଚଳ ଅକ୍ଷର *zText, ପୂର୍ଣ୍ଣସଂଖ୍ୟା useAnsi){
  ଅକ୍ଷର *zTextMbcs;
  LPWSTR zTmpWide;

  zTmpWide = winUtf8ToUnicode(zText);
  ଯଦି( zTmpWide==0 ){
    ଫେରନ୍ତୁ 0;
  }
  zTextMbcs = winUnicodeToMbcs(zTmpWide, useAnsi);
  sqlite3_free(zTmpWide);
  ଫେରନ୍ତୁ zTextMbcs;
}

/*
** This is a public wrapper for the winUtf8ToUnicode() function.
*/
LPWSTR sqlite3_win32_utf8_to_unicode(ଅଚଳ ଅକ୍ଷର *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !zText ){
    (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
    ଫେରନ୍ତୁ 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ଯଦି( sqlite3_initialize() ) ଫେରନ୍ତୁ 0;
#endif
  ଫେରନ୍ତୁ winUtf8ToUnicode(zText);
}

/*
** This is a public wrapper for the winUnicodeToUtf8() function.
*/
ଅକ୍ଷର *sqlite3_win32_unicode_to_utf8(LPCWSTR zWideText){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !zWideText ){
    (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
    ଫେରନ୍ତୁ 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ଯଦି( sqlite3_initialize() ) ଫେରନ୍ତୁ 0;
#endif
  ଫେରନ୍ତୁ winUnicodeToUtf8(zWideText);
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
ଅକ୍ଷର *sqlite3_win32_mbcs_to_utf8(ଅଚଳ ଅକ୍ଷର *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !zText ){
    (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
    ଫେରନ୍ତୁ 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ଯଦି( sqlite3_initialize() ) ଫେରନ୍ତୁ 0;
#endif
  ଫେରନ୍ତୁ winMbcsToUtf8(zText, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
ଅକ୍ଷର *sqlite3_win32_mbcs_to_utf8_v2(ଅଚଳ ଅକ୍ଷର *zText, ପୂର୍ଣ୍ଣସଂଖ୍ୟା useAnsi){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !zText ){
    (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
    ଫେରନ୍ତୁ 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ଯଦି( sqlite3_initialize() ) ଫେରନ୍ତୁ 0;
#endif
  ଫେରନ୍ତୁ winMbcsToUtf8(zText, useAnsi);
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
ଅକ୍ଷର *sqlite3_win32_utf8_to_mbcs(ଅଚଳ ଅକ୍ଷର *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !zText ){
    (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
    ଫେରନ୍ତୁ 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ଯଦି( sqlite3_initialize() ) ଫେରନ୍ତୁ 0;
#endif
  ଫେରନ୍ତୁ winUtf8ToMbcs(zText, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
ଅକ୍ଷର *sqlite3_win32_utf8_to_mbcs_v2(ଅଚଳ ଅକ୍ଷର *zText, ପୂର୍ଣ୍ଣସଂଖ୍ୟା useAnsi){
#ifdef SQLITE_ENABLE_API_ARMOR
  ଯଦି( !zText ){
    (ଶୂନ୍ୟ)SQLITE_MISUSE_BKPT;
    ଫେରନ୍ତୁ 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  ଯଦି( sqlite3_initialize() ) ଫେରନ୍ତୁ 0;
#endif
  ଫେରନ୍ତୁ winUtf8ToMbcs(zText, useAnsi);
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-8 string.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_win32_set_directory8(
  unsigned long type, /* Identifier for directory being set or reset */
  ଅଚଳ ଅକ୍ଷର *z_ମୂଲ୍ୟ  /* New value for directory being set or reset */
){
  ଅକ୍ଷର **ppDirectory = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ଯଦି( rc ) ଫେରନ୍ତୁ rc;
#endif
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ଯଦି( type==SQLITE_WIN32_DATA_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_data_directory;
  }ଅନ୍ୟଥା ଯଦି( type==SQLITE_WIN32_TEMP_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_temp_directory;
  }
  ପ୍ରତିଜ୍ଞା( !ppDirectory || type==SQLITE_WIN32_DATA_DIRECTORY_TYPE
          || type==SQLITE_WIN32_TEMP_DIRECTORY_TYPE
  );
  ପ୍ରତିଜ୍ଞା( !ppDirectory || sqlite3MemdebugHasType(*ppDirectory, MEMTYPE_HEAP) );
  ଯଦି( ppDirectory ){
    ଅକ୍ଷର *zCopy = 0;
    ଯଦି( z_ମୂଲ୍ୟ && z_ମୂଲ୍ୟ[0] ){
      zCopy = sqlite3_mprintf("%s", z_ମୂଲ୍ୟ);
      ଯଦି ( zCopy==0 ){
        rc = SQLITE_NOMEM_BKPT;
        ଯାଅ set_directory8_done;
      }
    }
    sqlite3_free(*ppDirectory);
    *ppDirectory = zCopy;
    rc = SQLITE_OK;
  }ଅନ୍ୟଥା{
    rc = SQLITE_ERROR;
  }
set_directory8_done:
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ଫେରନ୍ତୁ rc;
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-16 string.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_win32_set_directory16(
  unsigned long type, /* Identifier for directory being set or reset */
  ଅଚଳ ଶୂନ୍ୟ *z_ମୂଲ୍ୟ  /* New value for directory being set or reset */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଅକ୍ଷର *zUtf8 = 0;
  ଯଦି( z_ମୂଲ୍ୟ ){
    zUtf8 = sqlite3_win32_unicode_to_utf8(z_ମୂଲ୍ୟ);
    ଯଦି( zUtf8==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_win32_set_directory8(type, zUtf8);
  ଯଦି( zUtf8 ) sqlite3_free(zUtf8);
  ଫେରନ୍ତୁ rc;
}

/*
** This function sets the data directory or the temporary directory based on
** the provided arguments.  The type argument must be 1 in order to set the
** data directory or 2 in order to set the temporary directory.  The zValue
** argument is the name of the directory to use.  The return value will be
** SQLITE_OK if successful.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_win32_set_directory(
  unsigned long type, /* Identifier for directory being set or reset */
  ଶୂନ୍ୟ *z_ମୂଲ୍ୟ        /* New value for directory being set or reset */
){
  ଫେରନ୍ତୁ sqlite3_win32_set_directory16(type, z_ମୂଲ୍ୟ);
}
#endif /* _WIN32 */

/*
** The return value of winGetLastErrorMsg
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated).
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା win_Get_Last_Error_ସନ୍ଦେଶ(DWORD lastErrno, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf, ଅକ୍ଷର *zBuf){
  /* FormatMessage returns 0 on failure.  Otherwise it
  ** returns the number of TCHARs written to the output
  ** buffer, excluding the terminating null char.
  */
  DWORD dwLen = 0;
  ଅକ୍ଷର *zOut = 0;
  LPWSTR zTempWide = ଖାଲି;
  dwLen = osFormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_FROM_SYSTEM |
                           FORMAT_MESSAGE_IGNORE_INSERTS,
                           ଖାଲି,
                           lastErrno,
                           0,
                           (LPWSTR) &zTempWide,
                           0,
                           0);
  ଯଦି( dwLen > 0 ){
    /* allocate a buffer and convert to UTF8 */
    sqlite3BeginBenignMalloc();
    zOut = winUnicodeToUtf8(zTempWide);
    sqlite3EndBenignMalloc();
    /* free the system buffer allocated by FormatMessage */
    osLocalFree(zTempWide);
  }
  ଯଦି( 0 == dwLen ){
    sqlite3_snprintf(nBuf, zBuf, "OsError 0x%lx (%lu)", lastErrno, lastErrno);
  }ଅନ୍ୟଥା{
    /* copy a maximum of nBuf chars to output buffer */
    sqlite3_snprintf(nBuf, zBuf, "%s", zOut);
    /* free the UTF8 buffer */
    sqlite3_free(zOut);
  }
  ଫେରନ୍ତୁ 0;
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
#define winLogError(a,b,c,d)   win_Log_Error_At_ଧାଡ଼ି(a,b,c,d,__LINE__)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା win_Log_Error_At_ଧାଡ଼ି(
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା errcode,                    /* SQLite error code */
  DWORD lastErrno,                /* Win32 last error */
  ଅଚଳ ଅକ୍ଷର *zFunc,              /* Name of OS function that failed */
  ଅଚଳ ଅକ୍ଷର *zPath,              /* File path associated with error */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_ଧାଡ଼ି                       /* Source line number where error occurred */
){
  ଅକ୍ଷର z_ସନ୍ଦେଶ[500];                 /* Human readable error text */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                          /* Loop counter */

  z_ସନ୍ଦେଶ[0] = 0;
  win_Get_Last_Error_ସନ୍ଦେଶ(lastErrno, ଆକାର(z_ସନ୍ଦେଶ), z_ସନ୍ଦେଶ);
  ପ୍ରତିଜ୍ଞା( errcode!=SQLITE_OK );
  ଯଦି( zPath==0 ) zPath = "";
  ପାଇଁ(i=0; z_ସନ୍ଦେଶ[i] && z_ସନ୍ଦେଶ[i]!='\r' && z_ସନ୍ଦେଶ[i]!='\n'; i++){}
  z_ସନ୍ଦେଶ[i] = 0;
  sqlite3_log(errcode,
      "os_win.c:%d: (%lu) %s(%s) - %s",
      i_ଧାଡ଼ି, lastErrno, zFunc, zPath, z_ସନ୍ଦେଶ
  );

  ଫେରନ୍ତୁ errcode;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winIoerrRetry = SQLITE_WIN32_IOERR_RETRY;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winIoerrRetryDelay = SQLITE_WIN32_IOERR_RETRY_DELAY;

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
#ଯଦି !defined(winIoerrCanRetry1)
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winRetryIoerr(ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pnRetry, DWORD *pError){
  DWORD e = osGetLastError();
  ଯଦି( *pnRetry>=winIoerrRetry ){
    ଯଦି( pError ){
      *pError = e;
    }
    ଫେରନ୍ତୁ 0;
  }
  ଯଦି( winIoerrCanRetry1(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    ଫେରନ୍ତୁ 1;
  }
#ଯଦି defined(winIoerrCanRetry2)
  ଅନ୍ୟଥା ଯଦି( winIoerrCanRetry2(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    ଫେରନ୍ତୁ 1;
  }
#endif
  ଯଦି( pError ){
    *pError = e;
  }
  ଫେରନ୍ତୁ 0;
}

/*
** Log a I/O error retry episode.
*/
ସ୍ଥିର ଶୂନ୍ୟ winLogIoerr(ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRetry, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno){
  ଯଦି( nRetry ){
    sqlite3_log(SQLITE_NOTICE,
      "delayed %dms for lock/sharing conflict at line %d",
      winIoerrRetryDelay*nRetry*(nRetry+1)/2, lineno
    );
  }
}


/*
** Lock a file region.
*/
ସ୍ଥିର BOOL winLockFile(
  LPHANDLE phFile,
  DWORD flags,
  DWORD offset_ତଳ,
  DWORD offset_ଉପର,
  DWORD ଅଙ୍କ_Bytes_ତଳ,
  DWORD ଅଙ୍କ_Bytes_ଉପର
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, ଆକାର(OVERLAPPED));
  ovlp.Offset = offset_ତଳ;
  ovlp.Offset_ଉପର = offset_ଉପର;
  ଫେରନ୍ତୁ osLockFileEx(*phFile, flags, 0, ଅଙ୍କ_Bytes_ତଳ, ଅଙ୍କ_Bytes_ଉପର, &ovlp);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winHandleLockTimeout(
  HANDLE hFile,
  DWORD offset,
  DWORD nByte,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bExcl,
  DWORD nMs
){
  DWORD flags = LOCKFILE_FAIL_IMMEDIATELY | (bExcl?LOCKFILE_EXCLUSIVE_LOCK:0);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  BOOL ret;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, ଆକାର(OVERLAPPED));
  ovlp.Offset = offset;

#ଯଦି defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  ଯଦି( nMs!=0 ){
    flags &= ~LOCKFILE_FAIL_IMMEDIATELY;
  }
  ovlp.hEvent = osCreateEvent(ଖାଲି, TRUE, FALSE, ଖାଲି);
  ଯଦି( ovlp.hEvent==ଖାଲି ){
    ଫେରନ୍ତୁ SQLITE_IOERR_LOCK;
  }
#endif
  ret = osLockFileEx(hFile, flags, 0, nByte, 0, &ovlp);
#ଯଦି defined(SQLITE_ENABLE_SETLK_TIMEOUT)
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
  ଯଦି( !ret && GetLastError()==ERROR_IO_PENDING ){
    DWORD nDelay = (nMs==0 ? INFINITE : nMs);
    DWORD res = osWaitForSingleObject(ovlp.hEvent, nDelay);
    ଯଦି( res==WAIT_OBJECT_0 ){
      ret = TRUE;
    }ଅନ୍ୟଥା ଯଦି( res==WAIT_TIMEOUT ){
#ଯଦି SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = SQLITE_BUSY_TIMEOUT;
#ଅନ୍ୟଥା
      rc = SQLITE_BUSY;
#endif
    }ଅନ୍ୟଥା{
      /* Some other error has occurred */
      rc = SQLITE_IOERR_LOCK;
    }

    /* If it is still pending, cancel the LockFileEx() call. */
    osCancelIo(hFile);
  }

  osCloseHandle(ovlp.hEvent);
#endif /* defined(SQLITE_ENABLE_SETLK_TIMEOUT) */

  ଯଦି( rc==SQLITE_OK && !ret ){
    rc = SQLITE_BUSY;
  }
  ଫେରନ୍ତୁ rc;
}
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Unlock a file region.
 */
ସ୍ଥିର BOOL winUnlockFile(
  LPHANDLE phFile,
  DWORD offset_ତଳ,
  DWORD offset_ଉପର,
  DWORD ଅଙ୍କ_Bytes_ତଳ,
  DWORD ଅଙ୍କ_Bytes_ଉପର
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, ଆକାର(OVERLAPPED));
  ovlp.Offset = offset_ତଳ;
  ovlp.Offset_ଉପର = offset_ଉପର;
  ଫେରନ୍ତୁ osUnlockFileEx(*phFile, 0, ଅଙ୍କ_Bytes_ତଳ, ଅଙ୍କ_Bytes_ଉପର, &ovlp);
}

#ifndef SQLITE_OMIT_WAL
/*
** Remove an nByte lock starting at offset iOff from HANDLE h.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winHandleUnlock(HANDLE h, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOff, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte){
  BOOL ret = winUnlockFile(&h, iOff, 0, nByte, 0);
  ଫେରନ୍ତୁ (ret ? SQLITE_OK : SQLITE_IOERR_UNLOCK);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winHandleSeek(HANDLE h, sqlite3_int64 iOffset){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;             /* Return value */
  LARGE_INTEGER x;                /* The offset */

  x.QuadPart = iOffset;
  ଯଦି( osSetFilePointerEx(h, x, 0, FILE_BEGIN)==0 ){
    rc = SQLITE_IOERR_SEEK;
  }
  OSTRACE(("SEEK file=%p, offset=%lld rc=%s\n", h, iOffset,sqlite3ErrName(rc)));
  ଫେରନ୍ତୁ rc;
}

/*
** Move the current position of the file handle passed as the first
** argument to offset iOffset within the file. If successful, return 0.
** Otherwise, set pFile->lastErrno and return non-zero.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winSeekFile(winFile *pFile, sqlite3_int64 iOffset){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  rc = winHandleSeek(pFile->h, iOffset);
  ଯଦି( rc!=SQLITE_OK ){
    pFile->lastErrno = osGetLastError();
    winLogError(rc, pFile->lastErrno, "winSeekFile", pFile->zPath);
  }
  ଫେରନ୍ତୁ rc;
}


#ଯଦି SQLITE_MAX_MMAP_SIZE>0
/* Forward references to VFS helper methods used for memory mapped files */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winMapfile(winFile*, sqlite3_int64);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winUnmapfile(winFile*);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winClose(sqlite3_file *id){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc, cnt = 0;
  winFile *pFile = (winFile*)id;

  ପ୍ରତିଜ୍ଞା( id!=0 );
#ifndef SQLITE_OMIT_WAL
  ପ୍ରତିଜ୍ଞା( pFile->pShm==0 );
#endif
  ପ୍ରତିଜ୍ଞା( pFile->h!=ଖାଲି && pFile->h!=INVALID_HANDLE_ମୂଲ୍ୟ );
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p\n",
           osGetCurrentProcessId(), pFile, pFile->h));

#ଯଦି SQLITE_MAX_MMAP_SIZE>0
  winUnmapfile(pFile);
#endif

  କର{
    rc = osCloseHandle(pFile->h);
    /* SimulateIOError( rc=0; cnt=MX_CLOSE_ATTEMPT; ); */
  }ଯେତେବେଳେ( rc==0 && ++cnt < MX_CLOSE_ATTEMPT && (osSleep(100), 1) );
  ଯଦି( rc ){
    pFile->h = ଖାଲି;
  }
  OpenCounter(-1);
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, rc ? "ok" : "failed"));
  ଫେରନ୍ତୁ rc ? SQLITE_OK
            : winLogError(SQLITE_IOERR_CLOSE, osGetLastError(),
                          "winClose", pFile->zPath);
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winRead(
  sqlite3_file *id,          /* File to read from */
  ଶୂନ୍ୟ *pBuf,                /* Write content into this buffer */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା amt,                   /* Number of bytes to read */
  sqlite3_int64 offset       /* Begin reading at this offset */
){
#ଯଦି !defined(SQLITE_WIN32_NO_OVERLAPPED)
  OVERLAPPED overlapped;          /* The offset for ReadFile. */
#endif
  winFile *pFile = (winFile*)id;  /* file handle */
  DWORD nRead;                    /* Number of bytes actually read from file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRetry = 0;                 /* Number of retrys */

  ପ୍ରତିଜ୍ଞା( id!=0 );
  ପ୍ରତିଜ୍ଞା( amt>0 );
  ପ୍ରତିଜ୍ଞା( offset>=0 );
  SimulateIOError(ଫେରନ୍ତୁ SQLITE_IOERR_READ);
  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, offset, pFile->locktype));

#ଯଦି SQLITE_MAX_MMAP_SIZE>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  ଯଦି( offset<pFile->mmapSize ){
    ଯଦି( offset+amt <= pFile->mmapSize ){
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], amt);
      OSTRACE(("READ-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      ଫେରନ୍ତୁ SQLITE_OK;
    }ଅନ୍ୟଥା{
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCopy = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(pFile->mmapSize - offset);
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#endif

#ଯଦି defined(SQLITE_WIN32_NO_OVERLAPPED)
  ଯଦି( winSeekFile(pFile, offset) ){
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ଫେରନ୍ତୁ SQLITE_FULL;
  }
  ଯେତେବେଳେ( !osReadFile(pFile->h, pBuf, amt, &nRead, 0) ){
#ଅନ୍ୟଥା
  memset(&overlapped, 0, ଆକାର(OVERLAPPED));
  overlapped.Offset = (LONG)(offset & 0xffffffff);
  overlapped.Offset_ଉପର = (LONG)((offset>>32) & 0x7fffffff);
  ଯେତେବେଳେ( !osReadFile(pFile->h, pBuf, amt, &nRead, &overlapped) &&
         osGetLastError()!=ERROR_HANDLE_EOF ){
#endif
    DWORD lastErrno;
    ଯଦି( winRetryIoerr(&nRetry, &lastErrno) ) ଜାରି;
    pFile->lastErrno = lastErrno;
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ଫେରନ୍ତୁ winLogError(SQLITE_IOERR_READ, pFile->lastErrno,
                       "winRead", pFile->zPath);
  }
  winLogIoerr(nRetry, __LINE__);
  ଯଦି( nRead<(DWORD)amt ){
    /* Unread parts of the buffer must be zero-filled */
    memset(&((ଅକ୍ଷର*)pBuf)[nRead], 0, amt-nRead);
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_SHORT_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ଫେରନ୍ତୁ SQLITE_IOERR_SHORT_READ;
  }

  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winWrite(
  sqlite3_file *id,               /* File to write into */
  ଅଚଳ ଶୂନ୍ୟ *pBuf,               /* The bytes to be written */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା amt,                        /* Number of bytes to write */
  sqlite3_int64 offset            /* Offset into the file to begin writing at */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;                     /* True if error has occurred, else false */
  winFile *pFile = (winFile*)id;  /* File handle */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRetry = 0;                 /* Number of retries */

  ପ୍ରତିଜ୍ଞା( amt>0 );
  ପ୍ରତିଜ୍ଞା( pFile );
  SimulateIOError(ଫେରନ୍ତୁ SQLITE_IOERR_WRITE);
  SimulateDiskfullError(ଫେରନ୍ତୁ SQLITE_FULL);

  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, offset, pFile->locktype));

#ଯଦି defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_SIZE>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  ଯଦି( offset<pFile->mmapSize ){
    ଯଦି( offset+amt <= pFile->mmapSize ){
      memcpy(&((u8 *)(pFile->pMapRegion))[offset], pBuf, amt);
      OSTRACE(("WRITE-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      ଫେରନ୍ତୁ SQLITE_OK;
    }ଅନ୍ୟଥା{
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCopy = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(pFile->mmapSize - offset);
      memcpy(&((u8 *)(pFile->pMapRegion))[offset], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#endif

#ଯଦି defined(SQLITE_WIN32_NO_OVERLAPPED)
  rc = winSeekFile(pFile, offset);
  ଯଦି( rc==0 ){
#ଅନ୍ୟଥା
  {
#endif
#ଯଦି !defined(SQLITE_WIN32_NO_OVERLAPPED)
    OVERLAPPED overlapped;        /* The offset for WriteFile. */
#endif
    u8 *aRem = (u8 *)pBuf;        /* Data yet to be written */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRem = amt;               /* Number of bytes yet to be written */
    DWORD nWrite;                 /* Bytes written by each WriteFile() call */
    DWORD lastErrno = NO_ERROR;   /* Value returned by GetLastError() */

#ଯଦି !defined(SQLITE_WIN32_NO_OVERLAPPED)
    memset(&overlapped, 0, ଆକାର(OVERLAPPED));
    overlapped.Offset = (LONG)(offset & 0xffffffff);
    overlapped.Offset_ଉପର = (LONG)((offset>>32) & 0x7fffffff);
#endif

    ଯେତେବେଳେ( nRem>0 ){
#ଯଦି defined(SQLITE_WIN32_NO_OVERLAPPED)
      ଯଦି( !osWriteFile(pFile->h, aRem, nRem, &nWrite, 0) ){
#ଅନ୍ୟଥା
      ଯଦି( !osWriteFile(pFile->h, aRem, nRem, &nWrite, &overlapped) ){
#endif
        ଯଦି( winRetryIoerr(&nRetry, &lastErrno) ) ଜାରି;
        ଭାଙ୍ଗ;
      }
      ପ୍ରତିଜ୍ଞା( nWrite==0 || nWrite<=(DWORD)nRem );
      ଯଦି( nWrite==0 || nWrite>(DWORD)nRem ){
        lastErrno = osGetLastError();
        ଭାଙ୍ଗ;
      }
#ଯଦି !defined(SQLITE_WIN32_NO_OVERLAPPED)
      offset += nWrite;
      overlapped.Offset = (LONG)(offset & 0xffffffff);
      overlapped.Offset_ଉପର = (LONG)((offset>>32) & 0x7fffffff);
#endif
      aRem += nWrite;
      nRem -= nWrite;
    }
    ଯଦି( nRem>0 ){
      pFile->lastErrno = lastErrno;
      rc = 1;
    }
  }

  ଯଦି( rc ){
    ଯଦି(   ( pFile->lastErrno==ERROR_HANDLE_DISK_FULL )
       || ( pFile->lastErrno==ERROR_DISK_FULL )){
      OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      ଫେରନ୍ତୁ winLogError(SQLITE_FULL, pFile->lastErrno,
                         "winWrite1", pFile->zPath);
    }
    OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_WRITE\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ଫେରନ୍ତୁ winLogError(SQLITE_IOERR_WRITE, pFile->lastErrno,
                       "winWrite2", pFile->zPath);
  }ଅନ୍ୟଥା{
    winLogIoerr(nRetry, __LINE__);
  }
  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  ଫେରନ୍ତୁ SQLITE_OK;
}

#ifndef SQLITE_OMIT_WAL
/*
** Truncate the file opened by handle h to nByte bytes in size.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winHandleTruncate(HANDLE h, sqlite3_int64 nByte){ 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;             /* Return code */
  rc = winHandleSeek(h, nByte);
  ଯଦି( rc==SQLITE_OK ){
    ଯଦି( 0==osSetEndOfFile(h) ){
      rc = SQLITE_IOERR_TRUNCATE;
    }
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Determine the size in bytes of the file opened by the handle passed as 
** the first argument.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winHandleSize(HANDLE h, sqlite3_int64 *pnByte){ 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  LARGE_INTEGER x;
  ପ୍ରତିଜ୍ଞା( pnByte );
  ଯଦି( osGetFileSizeEx(h, &x)==0 ){
    rc = SQLITE_IOERR_FSTAT;
    *pnByte = 0;
  }ଅନ୍ୟଥା{
    *pnByte = x.QuadPart;
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Close the handle passed as the only argument.
*/
ସ୍ଥିର ଶୂନ୍ୟ winHandleClose(HANDLE h){
  ଯଦି( h!=INVALID_HANDLE_ମୂଲ୍ୟ ){
    osCloseHandle(h);
  }
}
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Truncate an open file to a specified size
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winTruncate(sqlite3_file *id, sqlite3_int64 nByte){
  winFile *pFile = (winFile*)id;  /* File handle object */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;             /* Return code for this function */
  DWORD lastErrno;
#ଯଦି SQLITE_MAX_MMAP_SIZE>0
  sqlite3_int64 oldMmapSize;
  ଯଦି( pFile->nFetchOut>0 ){
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
    ଫେରନ୍ତୁ SQLITE_OK;
  }
#endif

  ପ୍ରତିଜ୍ଞା( pFile );
  SimulateIOError(ଫେରନ୍ତୁ SQLITE_IOERR_TRUNCATE);
  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, size=%lld, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, nByte, pFile->locktype));

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  ଯଦି( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

#ଯଦି SQLITE_MAX_MMAP_SIZE>0
  ଯଦି( pFile->pMapRegion ){
    oldMmapSize = pFile->mmapSize;
  }ଅନ୍ୟଥା{
    oldMmapSize = 0;
  }
  winUnmapfile(pFile);
#endif

  /* SetEndOfFile() returns non-zero when successful, or zero when it fails. */
  ଯଦି( winSeekFile(pFile, nByte) ){
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->lastErrno,
                     "winTruncate1", pFile->zPath);
  }ଅନ୍ୟଥା ଯଦି( 0==osSetEndOfFile(pFile->h) &&
            ((lastErrno = osGetLastError())!=ERROR_USER_MAPPED_FILE) ){
    pFile->lastErrno = lastErrno;
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->lastErrno,
                     "winTruncate2", pFile->zPath);
  }

#ଯଦି SQLITE_MAX_MMAP_SIZE>0
  ଯଦି( rc==SQLITE_OK && oldMmapSize>0 ){
    ଯଦି( oldMmapSize>nByte ){
      winMapfile(pFile, -1);
    }ଅନ୍ୟଥା{
      winMapfile(pFile, oldMmapSize);
    }
  }
#endif

  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, sqlite3ErrName(rc)));
  ଫେରନ୍ତୁ rc;
}

#ifdef SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occurring at the right times.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_sync_ଗଣନା = 0;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_fullsync_ଗଣନା = 0;
#endif

/*
** Make sure all writes to a particular file are committed to disk.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winSync(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
#ifndef SQLITE_NO_SYNC
  /*
  ** Used only when SQLITE_NO_SYNC is not defined.
   */
  BOOL rc;
#endif
#ଯଦି !defined(NDEBUG) || !defined(SQLITE_NO_SYNC) || \
    defined(SQLITE_HAVE_OS_TRACE)
  /*
  ** Used when SQLITE_NO_SYNC is not defined and by the assert() and/or
  ** OSTRACE() macros.
   */
  winFile *pFile = (winFile*)id;
#ଅନ୍ୟଥା
  UNUSED_PARAMETER(id);
#endif

  ପ୍ରତିଜ୍ଞା( pFile );
  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  ପ୍ରତିଜ୍ଞା((flags&0x0F)==SQLITE_SYNC_NORMAL
      || (flags&0x0F)==SQLITE_SYNC_FULL
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( ଫେରନ୍ତୁ SQLITE_FULL );

  OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, flags=%x, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, flags,
           pFile->locktype));

#ifndef SQLITE_TEST
  UNUSED_PARAMETER(flags);
#ଅନ୍ୟଥା
  ଯଦି( (flags&0x0F)==SQLITE_SYNC_FULL ){
    sqlite3_fullsync_ଗଣନା++;
  }
  sqlite3_sync_ଗଣନା++;
#endif

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op
  */
#ifdef SQLITE_NO_SYNC
  OSTRACE(("SYNC-NOP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  ଫେରନ୍ତୁ SQLITE_OK;
#ଅନ୍ୟଥା
#ଯଦି SQLITE_MAX_MMAP_SIZE>0
  ଯଦି( pFile->pMapRegion ){
    ଯଦି( osFlushViewOfFile(pFile->pMapRegion, 0) ){
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_OK\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
    }ଅନ୍ୟଥା{
      pFile->lastErrno = osGetLastError();
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
      ଫେରନ୍ତୁ winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winSync1", pFile->zPath);
    }
  }
#endif
  rc = osFlushFileBuffers(pFile->h);
  SimulateIOError( rc=FALSE );
  ଯଦି( rc ){
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ଫେରନ୍ତୁ SQLITE_OK;
  }ଅନ୍ୟଥା{
    pFile->lastErrno = osGetLastError();
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_FSYNC\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ଫେରନ୍ତୁ winLogError(SQLITE_IOERR_FSYNC, pFile->lastErrno,
                       "winSync2", pFile->zPath);
  }
#endif
}

/*
** Determine the current size of a file in bytes
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winFileSize(sqlite3_file *id, sqlite3_int64 *pSize){
  winFile *pFile = (winFile*)id;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;

  ପ୍ରତିଜ୍ଞା( id!=0 );
  ପ୍ରତିଜ୍ଞା( pSize!=0 );
  SimulateIOError(ଫେରନ୍ତୁ SQLITE_IOERR_FSTAT);
  OSTRACE(("SIZE file=%p, pSize=%p\n", pFile->h, pSize));
  {
    LARGE_INTEGER x;
    ଯଦି( osGetFileSizeEx(pFile->h, &x)==0 ){
      *pSize = 0;
      pFile->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_FSTAT, pFile->lastErrno,
                       "winFileSize", pFile->zPath);
    }ଅନ୍ୟଥା{
      *pSize = x.QuadPart;
    }
  }
  OSTRACE(("SIZE file=%p, pSize=%p, *pSize=%lld, rc=%s\n",
           pFile->h, pSize, *pSize, sqlite3ErrName(rc)));
  ଫେରନ୍ତୁ rc;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winGetReadLock(winFile *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bBlock){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
  DWORD mask = ~(bBlock ? LOCKFILE_FAIL_IMMEDIATELY : 0);
  OSTRACE(("READ-LOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winLockFile(&pFile->h, SQLITE_LOCKFILEEX_FLAGS&mask, SHARED_FIRST, 0,
                    SHARED_SIZE, 0);
  ଯଦି( res == 0 ){
    pFile->lastErrno = osGetLastError();
    /* No need to log a failure to lock */
  }
  OSTRACE(("READ-LOCK file=%p, result=%d\n", pFile->h, res));
  ଫେରନ୍ତୁ res;
}

/*
** Undo a readlock
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winUnlockReadLock(winFile *pFile){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
  DWORD lastErrno;
  OSTRACE(("READ-UNLOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winUnlockFile(&pFile->h, SHARED_FIRST, 0, SHARED_SIZE, 0);
  ଯଦି( res==0 && ((lastErrno = osGetLastError())!=ERROR_NOT_LOCKED) ){
    pFile->lastErrno = lastErrno;
    winLogError(SQLITE_IOERR_UNLOCK, pFile->lastErrno,
                "winUnlockReadLock", pFile->zPath);
  }
  OSTRACE(("READ-UNLOCK file=%p, result=%d\n", pFile->h, res));
  ଫେରନ୍ତୁ res;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winLock(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା locktype){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;    /* Return code from subroutines */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 1;           /* Result of a Windows lock call */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା newLocktype;       /* Set pFile->locktype to this value before exiting */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା gotPendingLock = 0;/* True if we acquired a PENDING lock this time */
  winFile *pFile = (winFile*)id;
  DWORD lastErrno = NO_ERROR;

  ପ୍ରତିଜ୍ଞା( id!=0 );
  OSTRACE(("LOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->sharedLockByte, locktype));

  /* If there is already a lock of this type or more restrictive on the
  ** OsFile, do nothing. Don't use the end_lock: exit path, as
  ** sqlite3OsEnterMutex() hasn't been called yet.
  */
  ଯଦି( pFile->locktype>=locktype ){
    OSTRACE(("LOCK-HELD file=%p, rc=SQLITE_OK\n", pFile->h));
    ଫେରନ୍ତୁ SQLITE_OK;
  }

  /* Do not allow any kind of write-lock on a read-only database
  */
  ଯଦି( (pFile->ctrlFlags & WINFILE_RDONLY)!=0 && locktype>=RESERVED_LOCK ){
    ଫେରନ୍ତୁ SQLITE_IOERR_LOCK;
  }

  /* Make sure the locking sequence is correct
  */
  ପ୍ରତିଜ୍ଞା( pFile->locktype!=NO_LOCK || locktype==SHARED_LOCK );
  ପ୍ରତିଜ୍ଞା( locktype!=PENDING_LOCK );
  ପ୍ରତିଜ୍ଞା( locktype!=RESERVED_LOCK || pFile->locktype==SHARED_LOCK );

  /* Lock the PENDING_LOCK byte if we need to acquire an EXCLUSIVE lock or
  ** a SHARED lock.  If we are acquiring a SHARED lock, the acquisition of
  ** the PENDING_LOCK byte is temporary.
  */
  newLocktype = pFile->locktype;
  ଯଦି( locktype==SHARED_LOCK
   || (locktype==EXCLUSIVE_LOCK && pFile->locktype==RESERVED_LOCK)
  ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt = 3;

    /* Flags for the LockFileEx() call. This should be an exclusive lock if
    ** this call is to obtain EXCLUSIVE, or a shared lock if this call is to
    ** obtain SHARED.  */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = LOCKFILE_FAIL_IMMEDIATELY;
    ଯଦି( locktype==EXCLUSIVE_LOCK ){
      flags |= LOCKFILE_EXCLUSIVE_LOCK;
    }
    ଯେତେବେଳେ( cnt>0 ){
      /* Try 3 times to get the pending lock.  This is needed to work
      ** around problems caused by indexing and/or anti-virus software on
      ** Windows systems.
      **
      ** If you are using this code as a model for alternative VFSes, do not
      ** copy this retry logic.  It is a hack intended for Windows only.  */
      res = winLockFile(&pFile->h, flags, PENDING_BYTE, 0, 1, 0);
      ଯଦି( res ) ଭାଙ୍ଗ;

      lastErrno = osGetLastError();
      OSTRACE(("LOCK-PENDING-FAIL file=%p, count=%d, result=%d\n", 
            pFile->h, cnt, res
      ));

      ଯଦି( lastErrno==ERROR_INVALID_HANDLE ){
        pFile->lastErrno = lastErrno;
        rc = SQLITE_IOERR_LOCK;
        OSTRACE(("LOCK-FAIL file=%p, count=%d, rc=%s\n", 
              pFile->h, cnt, sqlite3ErrName(rc)
        ));
        ଫେରନ୍ତୁ rc;
      }

      cnt--;
      ଯଦି( cnt>0 ) osSleep(1);
    }
    gotPendingLock = res;
  }

  /* Acquire a shared lock
  */
  ଯଦି( locktype==SHARED_LOCK && res ){
    ପ୍ରତିଜ୍ଞା( pFile->locktype==NO_LOCK );
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    res = winGetReadLock(pFile, pFile->bBlockOnConnect);
#ଅନ୍ୟଥା
    res = winGetReadLock(pFile, 0);
#endif
    ଯଦି( res ){
      newLocktype = SHARED_LOCK;
    }ଅନ୍ୟଥା{
      lastErrno = osGetLastError();
    }
  }

  /* Acquire a RESERVED lock
  */
  ଯଦି( locktype==RESERVED_LOCK && res ){
    ପ୍ରତିଜ୍ଞା( pFile->locktype==SHARED_LOCK );
    res = winLockFile(&pFile->h, SQLITE_LOCKFILE_FLAGS, RESERVED_BYTE, 0, 1, 0);
    ଯଦି( res ){
      newLocktype = RESERVED_LOCK;
    }ଅନ୍ୟଥା{
      lastErrno = osGetLastError();
    }
  }

  /* Acquire a PENDING lock
  */
  ଯଦି( locktype==EXCLUSIVE_LOCK && res ){
    newLocktype = PENDING_LOCK;
    gotPendingLock = 0;
  }

  /* Acquire an EXCLUSIVE lock
  */
  ଯଦି( locktype==EXCLUSIVE_LOCK && res ){
    ପ୍ରତିଜ୍ଞା( pFile->locktype>=SHARED_LOCK );
    (ଶୂନ୍ୟ)winUnlockReadLock(pFile);
    res = winLockFile(&pFile->h, SQLITE_LOCKFILE_FLAGS, SHARED_FIRST, 0,
                      SHARED_SIZE, 0);
    ଯଦି( res ){
      newLocktype = EXCLUSIVE_LOCK;
    }ଅନ୍ୟଥା{
      lastErrno = osGetLastError();
      winGetReadLock(pFile, 0);
    }
  }

  /* If we are holding a PENDING lock that ought to be released, then
  ** release it now.
  */
  ଯଦି( gotPendingLock && locktype==SHARED_LOCK ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }

  /* Update the state of the lock has held in the file descriptor then
  ** return the appropriate result code.
  */
  ଯଦି( res ){
    rc = SQLITE_OK;
  }ଅନ୍ୟଥା{
    pFile->lastErrno = lastErrno;
    rc = SQLITE_BUSY;
    OSTRACE(("LOCK-FAIL file=%p, wanted=%d, got=%d\n",
             pFile->h, locktype, newLocktype));
  }
  pFile->locktype = (u8)newLocktype;
  OSTRACE(("LOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  ଫେରନ୍ତୁ rc;
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, return
** non-zero, otherwise zero.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winCheckReservedLock(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
  winFile *pFile = (winFile*)id;

  SimulateIOError( ଫେରନ୍ତୁ SQLITE_IOERR_CHECKRESERVEDLOCK; );
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p\n", pFile->h, pResOut));

  ପ୍ରତିଜ୍ଞା( id!=0 );
  ଯଦି( pFile->locktype>=RESERVED_LOCK ){
    res = 1;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (local)\n", pFile->h, res));
  }ଅନ୍ୟଥା{
    res = winLockFile(&pFile->h, SQLITE_LOCKFILEEX_FLAGS,RESERVED_BYTE,0,1,0);
    ଯଦି( res ){
      winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
    }
    res = !res;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (remote)\n", pFile->h, res));
  }
  *pResOut = res;
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           pFile->h, pResOut, *pResOut));
  ଫେରନ୍ତୁ SQLITE_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winUnlock(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା locktype){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା type;
  winFile *pFile = (winFile*)id;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ପ୍ରତିଜ୍ଞା( pFile!=0 );
  ପ୍ରତିଜ୍ଞା( locktype<=SHARED_LOCK );
  OSTRACE(("UNLOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->sharedLockByte, locktype));
  type = pFile->locktype;
  ଯଦି( type>=EXCLUSIVE_LOCK ){
    winUnlockFile(&pFile->h, SHARED_FIRST, 0, SHARED_SIZE, 0);
    ଯଦି( locktype==SHARED_LOCK && !winGetReadLock(pFile, 0) ){
      /* This should never happen.  We should always be able to
      ** reacquire the read lock */
      rc = winLogError(SQLITE_IOERR_UNLOCK, osGetLastError(),
                       "winUnlock", pFile->zPath);
    }
  }
  ଯଦି( type>=RESERVED_LOCK ){
    winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
  }
  ଯଦି( locktype==NO_LOCK && type>=SHARED_LOCK ){
    winUnlockReadLock(pFile);
  }
  ଯଦି( type>=PENDING_LOCK ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }
  pFile->locktype = (u8)locktype;
  OSTRACE(("UNLOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  ଫେରନ୍ତୁ rc;
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

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winNolockLock(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା locktype){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(locktype);
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winNolockCheckReservedLock(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(pResOut);
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winNolockUnlock(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା locktype){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(locktype);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/******************* End of the no-op lock implementation *********************
******************************************************************************/

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
ସ୍ଥିର ଶୂନ୍ୟ winModeBit(winFile *pFile, unsigned ଅକ୍ଷର mask, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pArg){
  ଯଦି( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & mask)!=0;
  }ଅନ୍ୟଥା ଯଦି( (*pArg)==0 ){
    pFile->ctrlFlags &= ~mask;
  }ଅନ୍ୟଥା{
    pFile->ctrlFlags |= mask;
  }
}

/* Forward references to VFS helper methods used for temporary files */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winGetTempname(sqlite3_vfs *, ଅକ୍ଷର **);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winIsDir(LPCWSTR);
ସ୍ଥିର BOOL winIsLongPathPrefix(ଅଚଳ ଅକ୍ଷର *);
ସ୍ଥିର BOOL winIsDriveLetterAndColon(ଅଚଳ ଅକ୍ଷର *);

/*
** Control and query of the open file handle.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winFileControl(sqlite3_file *id, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg){
  winFile *pFile = (winFile*)id;
  OSTRACE(("FCNTL file=%p, op=%d, pArg=%p\n", pFile->h, op, pArg));
  ବାଛ( op ){
    ସ୍ଥିତି SQLITE_FCNTL_LOCKSTATE: {
      *(ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)pArg = pFile->locktype;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ଫେରନ୍ତୁ SQLITE_OK;
    }
    ସ୍ଥିତି SQLITE_FCNTL_LAST_ERRNO: {
      *(ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)pArg = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pFile->lastErrno;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ଫେରନ୍ତୁ SQLITE_OK;
    }
    ସ୍ଥିତି SQLITE_FCNTL_CHUNK_SIZE: {
      pFile->szChunk = *(ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)pArg;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ଫେରନ୍ତୁ SQLITE_OK;
    }
    ସ୍ଥିତି SQLITE_FCNTL_SIZE_HINT: {
      ଯଦି( pFile->szChunk>0 ){
        sqlite3_int64 oldSz;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = winFileSize(id, &oldSz);
        ଯଦି( rc==SQLITE_OK ){
          sqlite3_int64 newSz = *(sqlite3_int64*)pArg;
          ଯଦି( newSz>oldSz ){
            SimulateIOErrorBenign(1);
            rc = winTruncate(id, newSz);
            SimulateIOErrorBenign(0);
          }
        }
        OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
        ଫେରନ୍ତୁ rc;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ଫେରନ୍ତୁ SQLITE_OK;
    }
    ସ୍ଥିତି SQLITE_FCNTL_PERSIST_WAL: {
      winModeBit(pFile, WINFILE_PERSIST_WAL, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ଫେରନ୍ତୁ SQLITE_OK;
    }
    ସ୍ଥିତି SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      winModeBit(pFile, WINFILE_PSOW, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ଫେରନ୍ତୁ SQLITE_OK;
    }
    ସ୍ଥିତି SQLITE_FCNTL_VFSNAME: {
      *(ଅକ୍ଷର**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ଫେରନ୍ତୁ SQLITE_OK;
    }
    ସ୍ଥିତି SQLITE_FCNTL_WIN32_AV_RETRY: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା *a = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)pArg;
      ଯଦି( a[0]>0 ){
        winIoerrRetry = a[0];
      }ଅନ୍ୟଥା{
        a[0] = winIoerrRetry;
      }
      ଯଦି( a[1]>0 ){
        winIoerrRetryDelay = a[1];
      }ଅନ୍ୟଥା{
        a[1] = winIoerrRetryDelay;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ଫେରନ୍ତୁ SQLITE_OK;
    }
    ସ୍ଥିତି SQLITE_FCNTL_WIN32_GET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      *phFile = pFile->h;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ଫେରନ୍ତୁ SQLITE_OK;
    }
#ifdef SQLITE_TEST
    ସ୍ଥିତି SQLITE_FCNTL_WIN32_SET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      HANDLE hOldFile = pFile->h;
      pFile->h = *phFile;
      *phFile = hOldFile;
      OSTRACE(("FCNTL oldFile=%p, newFile=%p, rc=SQLITE_OK\n",
               hOldFile, pFile->h));
      ଫେରନ୍ତୁ SQLITE_OK;
    }
#endif
    ସ୍ଥିତି SQLITE_FCNTL_NULL_IO: {
      (ଶୂନ୍ୟ)osCloseHandle(pFile->h);
      pFile->h = ଖାଲି;
      ଫେରନ୍ତୁ SQLITE_OK;
    }
    ସ୍ଥିତି SQLITE_FCNTL_TEMPFILENAME: {
      ଅକ୍ଷର *zTFile = 0;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = winGetTempname(pFile->pVfs, &zTFile);
      ଯଦି( rc==SQLITE_OK ){
        *(ଅକ୍ଷର**)pArg = zTFile;
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      ଫେରନ୍ତୁ rc;
    }
#ଯଦି SQLITE_MAX_MMAP_SIZE>0
    ସ୍ଥିତି SQLITE_FCNTL_MMAP_SIZE: {
      i64 newLimit = *(i64*)pArg;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
      ଯଦି( newLimit>sqlite3GlobalConfig.mxMmap ){
        newLimit = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (SIZE_T) and passed
      ** to MapViewOfFile(). Restrict its value to 2GB if (SIZE_T) is not at
      ** least a 64-bit type. */
      ଯଦି( newLimit>0 && ଆକାର(SIZE_T)<8 ){
        newLimit = (newLimit & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmapSizeMax;
      ଯଦି( newLimit>=0 && newLimit!=pFile->mmapSizeMax && pFile->nFetchOut==0 ){
        pFile->mmapSizeMax = newLimit;
        ଯଦି( pFile->mmapSize>0 ){
          winUnmapfile(pFile);
          rc = winMapfile(pFile, -1);
        }
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      ଫେରନ୍ତୁ rc;
    }
#endif

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    ସ୍ଥିତି SQLITE_FCNTL_LOCK_TIMEOUT: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOld = pFile->iBusyTimeout;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iNew = *(ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)pArg;
#ଯଦି SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = (iNew < 0) ? INFINITE : (DWORD)iNew;
#ନଚେତଯଦି SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = (DWORD)(!!iNew);
#ଅନ୍ୟଥା
# error "SQLITE_ENABLE_SETLK_TIMEOUT must be set to 1 or 2"
#endif
      *(ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)pArg = iOld;
      ଫେରନ୍ତୁ SQLITE_OK;
    }
    ସ୍ଥିତି SQLITE_FCNTL_BLOCK_ON_CONNECT: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iNew = *(ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)pArg;
      pFile->bBlockOnConnect = iNew;
      ଫେରନ୍ତୁ SQLITE_OK;
    }
#endif /* SQLITE_ENABLE_SETLK_TIMEOUT */

#ଯଦି defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    ସ୍ଥିତି SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      sqlite3_str_appendf(pStr, "{\"h\":%llu", (sqlite3_uint64)pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      ଯଦି( pFile->locktype ){
        ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *azLock[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
        sqlite3_str_appendf(pStr, ",\"locktype\":\"%s\"", 
                                  azLock[pFile->locktype-1]);
      }
#ଯଦି SQLITE_MAX_MMAP_SIZE>0
      ଯଦି( pFile->mmapSize ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmapSize);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->nFetchOut);
      }
#endif
      sqlite3_str_append(pStr, "}", 1);
      ଫେରନ୍ତୁ SQLITE_OK;
    }
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

  }
  OSTRACE(("FCNTL file=%p, rc=SQLITE_NOTFOUND\n", pFile->h));
  ଫେରନ୍ତୁ SQLITE_NOTFOUND;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winSectorSize(sqlite3_file *id){
  (ଶୂନ୍ୟ)id;
  ଫେରନ୍ତୁ SQLITE_DEFAULT_SECTOR_SIZE;
}

/*
** Return a vector of device characteristics.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winDeviceCharacteristics(sqlite3_file *id){
  winFile *p = (winFile*)id;
  ଫେରନ୍ତୁ SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN | SQLITE_IOCAP_SUBPAGE_READ |
         ((p->ctrlFlags & WINFILE_PSOW)?SQLITE_IOCAP_POWERSAFE_OVERWRITE:0);
}

/*
** Windows will only let you create file view mappings
** on allocation size granularity boundaries.
** During sqlite3_os_init() we do a GetSystemInfo()
** to get the granularity size.
*/
ସ୍ଥିର SYSTEM_INFO winSysInfo;

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
ସ୍ଥିର LPWSTR winConvertFromUtf8Filename(ଅଚଳ ଅକ୍ଷର *zFilename){
  LPWSTR zConverted = 0;
#ifdef __CYGWIN__
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nChar;

  ଯଦି( osCygwin_conv_path && !(winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2])) ){
    i64 nByte;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା convertflag = CCP_POSIX_TO_WIN_W;
    ଯଦି( !strchr(zFilename, '/') ) convertflag |= CCP_RELATIVE;
    nByte = (i64)osCygwin_conv_path(convertflag,
        zFilename, 0, 0);
    ଯଦି( nByte>0 ){
      zConverted = sqlite3MallocZero(12+(u64)nByte);
      ଯଦି ( zConverted==0 ){
        ଫେରନ୍ତୁ zConverted;
      }
      /* Filenames should be prefixed, except when converted
       * full path already starts with "\\?\". */
      ଯଦି( osCygwin_conv_path(convertflag, zFilename,
                           zConverted+4, nByte)==0 ){
        ଯଦି( (convertflag&CCP_RELATIVE) ){
          memmove(zConverted, zConverted+4, nByte);
        }ଅନ୍ୟଥା ଯଦି( memcmp(zConverted+4, L"\\\\", 4) ){
          memcpy(zConverted, L"\\\\?\\", 8);
        }ଅନ୍ୟଥା ଯଦି( zConverted[6]!='?' ){
          memmove(zConverted+6, zConverted+4, nByte);
          memcpy(zConverted, L"\\\\?\\UNC", 14);
        }ଅନ୍ୟଥା{
          memmove(zConverted, zConverted+4, nByte);
        }
        ଫେରନ୍ତୁ zConverted;
      }
      sqlite3_free(zConverted);
    }
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zFilename, -1, ଖାଲି, 0);
  ଯଦି( nChar==0 ){
    ଫେରନ୍ତୁ 0;
  }
  zConverted = sqlite3MallocZero( ((u64)nChar+6)*ଆକାର(WCHAR) );
  ଯଦି( zConverted==0 ){
    ଫେରନ୍ତୁ 0;
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zFilename, -1,
                                zConverted, nChar);
  ଯଦି( nChar==0 ){
    sqlite3_free(zConverted);
    zConverted = 0;
  }ଅନ୍ୟଥା ଯଦି( nChar>MAX_PATH
      && winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2]) ){
    memmove(zConverted+4, zConverted, nChar*ଆକାର(WCHAR));
    zConverted[2] = '\\';
    memcpy(zConverted, L"\\\\?\\", 8);
  }ଅନ୍ୟଥା ଯଦି( nChar>MAX_PATH
      && winIsDirSep(zFilename[0]) && winIsDirSep(zFilename[1])
      && zFilename[2] != '?' ){
    memmove(zConverted+6, zConverted, nChar*ଆକାର(WCHAR));
    memcpy(zConverted, L"\\\\?\\UNC", 14);
  }
#ଅନ୍ୟଥା /* if !defined(__CYGWIN__) */
  zConverted = winUtf8ToUnicode(zFilename);
#endif /* __CYGWIN__ */
  /* caller will handle out of memory */
  ଫେରନ୍ତୁ zConverted;
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
ସ୍ଥିର sqlite3_mutex *winBigLock = 0;
ସ୍ଥିର ଶୂନ୍ୟ winShmEnterMutex(ଶୂନ୍ୟ){
  sqlite3_mutex_enter(winBigLock);
}
ସ୍ଥିର ଶୂନ୍ୟ winShmLeaveMutex(ଶୂନ୍ୟ){
  sqlite3_mutex_leave(winBigLock);
}
#ifndef NDEBUG
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winShmMutexHeld(ଶୂନ୍ୟ) {
  ଫେରନ୍ତୁ sqlite3_mutex_held(winBigLock);
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
ସଂରଚନା win_Shm_ଗଣ୍ଠି {
  sqlite3_mutex *mutex;      /* Mutex to access this object */
  ଅକ୍ଷର *zFilename;           /* Name of the file */
  HANDLE hSharedShm;         /* File handle open on zFilename */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bUseSharedLockHandle;  /* True to use hSharedShm for everything */

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isUnlocked;            /* DMS lock has not yet been obtained */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isReadonly;            /* True if read-only */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szRegion;              /* Size of shared-memory regions */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRegion;               /* Size of array apRegion */

  ସଂରଚନା ShmRegion {
    HANDLE hMap;             /* File handle from CreateFileMapping */
    ଶୂନ୍ୟ *pMap;
  } *aRegion;
  DWORD lastErrno;           /* The Windows errno from the last I/O error */

  winShm *pWinShmList;       /* List of winShm objects with ptrs to this */

  win_Shm_ଗଣ୍ଠି *pNext;         /* Next in list of all winShmNode objects */
#ଯଦି defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 nextShmId;              /* Next available winShm.id value */
#endif
};

/*
** A global array of all winShmNode objects.
**
** The winShmMutexHeld() must be true while reading or writing this list.
*/
ସ୍ଥିର win_Shm_ଗଣ୍ଠି *win_Shm_ଗଣ୍ଠି_List = 0;

/*
** Structure used internally by this VFS to record the state of an
** open shared memory connection. There is one such structure for each
** winFile open on a wal mode database.
*/
ସଂରଚନା winShm {
  win_Shm_ଗଣ୍ଠି *p_Shm_ଗଣ୍ଠି;      /* The underlying winShmNode object */
  u16 sharedMask;            /* Mask of shared locks held */
  u16 exclMask;              /* Mask of exclusive locks held */
  HANDLE hShm;               /* File-handle on *-shm file. For locking. */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bReadonly;             /* True if hShm is opened read-only */
#ଯଦି defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winOpen(sqlite3_vfs*,ଅଚଳ ଅକ୍ଷର*,sqlite3_file*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winDelete(sqlite3_vfs *,ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/*
** Purge the winShmNodeList list of all entries with winShmNode.pWinShmList==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
ସ୍ଥିର ଶୂନ୍ୟ winShmPurge(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା deleteFlag){
  win_Shm_ଗଣ୍ଠି **pp;
  win_Shm_ଗଣ୍ଠି *p;
  ପ୍ରତିଜ୍ଞା( winShmMutexHeld() );
  OSTRACE(("SHM-PURGE pid=%lu, deleteFlag=%d\n",
           osGetCurrentProcessId(), deleteFlag));
  pp = &win_Shm_ଗଣ୍ଠି_List;
  ଯେତେବେଳେ( (p = *pp)!=0 ){
    ଯଦି( p->pWinShmList==0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      ଯଦି( p->mutex ){ sqlite3_mutex_free(p->mutex); }
      ପାଇଁ(i=0; i<p->nRegion; i++){
        BOOL bRc = osUnmapViewOfFile(p->aRegion[i].pMap);
        OSTRACE(("SHM-PURGE-UNMAP pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        UNUSED_VARIABLE_ମୂଲ୍ୟ(bRc);
        bRc = osCloseHandle(p->aRegion[i].hMap);
        OSTRACE(("SHM-PURGE-CLOSE pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        UNUSED_VARIABLE_ମୂଲ୍ୟ(bRc);
      }
      winHandleClose(p->hSharedShm);
      ଯଦି( deleteFlag ){
        SimulateIOErrorBenign(1);
        sqlite3BeginBenignMalloc();
        winDelete(pVfs, p->zFilename, 0);
        sqlite3EndBenignMalloc();
        SimulateIOErrorBenign(0);
      }
      *pp = p->pNext;
      sqlite3_free(p->aRegion);
      sqlite3_free(p);
    }ଅନ୍ୟଥା{
      pp = &p->pNext;
    }
  }
}

/*
** Handle h is open on the *-shm file. This function zeroes the first
** 8 bytes of the file. If SQLITE_ENABLE_SETLK_TIMEOUT is defined, h may 
** have been opened with FILE_FLAG_OVERLAPPED.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winZeroSharedMemory(HANDLE h){
  u8 aZero[8] = {0,0,0,0,0,0,0,0};
  DWORD nWrite = 0;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, ଆକାର(OVERLAPPED));
  ଯଦି( !osWriteFile(h, aZero, ଆକାର(aZero), &nWrite, &ovlp) ){
    /* If the file was opened with FILE_FLAG_OVERLAPPED, the write may still
    ** be pending. Wait for it with GetOverlappedResult().  */
    ଯଦି( osGetLastError()!=ERROR_IO_PENDING
     || !GetOverlappedResult(h, &ovlp, &nWrite, TRUE)
    ){
      nWrite = 0;
    }
  }
  ଫେରନ୍ତୁ (nWrite==ଆକାର(aZero) ? SQLITE_OK : SQLITE_IOERR_WRITE);
}

/*
** The DMS lock has not yet been taken on the shm file associated with
** pShmNode. Take the lock. Truncate the *-shm file if required.
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winLockSharedMemory(win_Shm_ଗଣ୍ଠି *p_Shm_ଗଣ୍ଠି, DWORD nMs){
  HANDLE h = p_Shm_ଗଣ୍ଠି->hSharedShm;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;

  ପ୍ରତିଜ୍ଞା( sqlite3_mutex_held(p_Shm_ଗଣ୍ଠି->mutex) );
  rc = winHandleLockTimeout(h, WIN_SHM_DMS, 1, 1, 0);
  ଯଦି( rc==SQLITE_OK ){
    /* We have an EXCLUSIVE lock on the DMS byte. This means that this
    ** is the first process to open the file. Truncate it to zero bytes
    ** in this case.  */
    ଯଦି( p_Shm_ଗଣ୍ଠି->isReadonly ){
      rc = SQLITE_READONLY_CANTINIT;
    }ଅନ୍ୟଥା{
      ଯଦି( SQLITE_OK!=winHandleTruncate(h, 0) ){
        /* Sometimes the truncate operation may fail because some other
        ** process is still holding an open mapping. So try to write
        ** zeroes into the start of the file instead. */
        rc = winZeroSharedMemory(h);
      }
    }

    /* Release the EXCLUSIVE lock acquired above. */
    winUnlockFile(&h, WIN_SHM_DMS, 0, 1, 0);
  }ଅନ୍ୟଥା ଯଦି( (rc & 0xFF)==SQLITE_BUSY ){
    rc = SQLITE_OK;
  }

  ଯଦି( rc==SQLITE_OK ){
    /* Take a SHARED lock on the DMS byte. */
    rc = winHandleLockTimeout(h, WIN_SHM_DMS, 1, 0, nMs);
    ଯଦି( rc==SQLITE_OK ){
      p_Shm_ଗଣ୍ଠି->isUnlocked = 0;
    }
  }

  ଫେରନ୍ତୁ rc;
}


/*
** This function is used to open a handle on a *-shm file.
**
** If SQLITE_ENABLE_SETLK_TIMEOUT is defined at build time, then the file
** is opened with FILE_FLAG_OVERLAPPED specified. If not, it is not.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winHandleOpen(
  ଅଚଳ ଅକ୍ଷର *zUtf8,              /* File to open */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pbReadonly,                /* IN/OUT: True for readonly handle */
  HANDLE *ph                      /* OUT: New HANDLE for file */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  LPWSTR zConverted = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bReadonly = *pbReadonly;
  HANDLE h = INVALID_HANDLE_ମୂଲ୍ୟ;

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  ଅଚଳ DWORD flag_overlapped = FILE_FLAG_OVERLAPPED;
#ଅନ୍ୟଥା
  ଅଚଳ DWORD flag_overlapped = 0;
#endif

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8);
  ଯଦି( zConverted==0 ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8));
    rc = SQLITE_IOERR_NOMEM_BKPT;
    ଯାଅ winopenfile_out;
  }

  /* Ensure the file we are trying to open is not actually a directory. */
  ଯଦି( winIsDir(zConverted) ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8));
    rc = SQLITE_CANTOPEN_ISDIR;
    ଯାଅ winopenfile_out;
  }

  /* TODO: platforms.
  ** TODO: retry-on-ioerr.
  */
  h = osCreateFileW(zConverted,         /* lpFileName */
      (GENERIC_READ | (bReadonly ? 0 : GENERIC_WRITE)),  /* dwDesiredAccess */
      FILE_SHARE_READ | FILE_SHARE_WRITE,        /* dwShareMode */
      ଖାଲି,                                      /* lpSecurityAttributes */
      OPEN_ALWAYS,                               /* dwCreationDisposition */
      FILE_ATTRIBUTE_NORMAL|flag_overlapped,
      ଖାଲି
  );

  ଯଦି( h==INVALID_HANDLE_ମୂଲ୍ୟ ){
    ଯଦି( bReadonly==0 ){
      bReadonly = 1;
      rc = winHandleOpen(zUtf8, &bReadonly, &h);
    }ଅନ୍ୟଥା{
      rc = SQLITE_CANTOPEN_BKPT;
    }
  }

 winopenfile_out:
  sqlite3_free(zConverted);
  *pbReadonly = bReadonly;
  *ph = h;
  ଫେରନ୍ତୁ rc;
}
 
/*
** Close pDbFd's connection to shared-memory.  Delete the underlying
** *-shm file if deleteFlag is true.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winCloseSharedMemory(winFile *pDbFd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା deleteFlag){
  winShm *p;            /* The connection to be closed */
  winShm **pp;          /* Iterator for pShmNode->pWinShmList */
  win_Shm_ଗଣ୍ଠି *p_Shm_ଗଣ୍ଠି; /* The underlying shared-memory file */

  p = pDbFd->pShm;
  ଯଦି( p==0 ) ଫେରନ୍ତୁ SQLITE_OK;
  ଯଦି( p->hShm!=INVALID_HANDLE_ମୂଲ୍ୟ ){
    osCloseHandle(p->hShm);
  }

  winShmEnterMutex();
  p_Shm_ଗଣ୍ଠି = p->p_Shm_ଗଣ୍ଠି;

  /* Remove this connection from the winShmNode.pWinShmList list */
  sqlite3_mutex_enter(p_Shm_ଗଣ୍ଠି->mutex);
  ପାଇଁ(pp=&p_Shm_ଗଣ୍ଠି->pWinShmList; *pp!=p; pp=&(*pp)->pWinShmNext){}
  *pp = p->pWinShmNext;
  sqlite3_mutex_leave(p_Shm_ଗଣ୍ଠି->mutex);

  winShmPurge(pDbFd->pVfs, deleteFlag);
  winShmLeaveMutex();

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** testfixture builds may set this global variable to true via a
** Tcl interface. This forces the VFS to use the locking normally
** only used for UNC paths for all files.
*/
#ifdef SQLITE_TEST
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_win_test_unc_locking = 0;
#ଅନ୍ୟଥା
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winIsUNCPath(ଅଚଳ ଅକ୍ଷର *zFile){
  ଯଦି( zFile[0]=='\\' && zFile[1]=='\\' ){
    ଯଦି( zFile[2]=='?'
     && zFile[3]=='\\'
     && sqlite3Isalpha(zFile[4])
     && zFile[5]==':'
     && winIsDirSep(zFile[6])
    ){
      ଫେରନ୍ତୁ sqlite3_win_test_unc_locking;
    }ଅନ୍ୟଥା{
      ଫେରନ୍ତୁ 1;
    }
  }
  ଫେରନ୍ତୁ sqlite3_win_test_unc_locking;
}

/*
** Return true if the string passed in is the name of a file on a
** remote (network-mounted) volume.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winIsRemoteVolume(ଅଚଳ ଅକ୍ଷର *zFile){
  WCHAR z_ମୂଳ[4];
  ଯଦି( strncmp(zFile, "\\\\?\\",4)==0 ) zFile += 4;
  ଯଦି( !sqlite3Isalpha(zFile[0]) ) ଫେରନ୍ତୁ 0;
  ଯଦି( zFile[1]!=':' ) ଫେରନ୍ତୁ 0;
  ଯଦି( !winIsDirSep(zFile[2]) ) ଫେରନ୍ତୁ 0;
  z_ମୂଳ[0] = (WCHAR)zFile[0];
  z_ମୂଳ[1] = ':';
  z_ମୂଳ[2] = '\\';
  z_ମୂଳ[3] = 0;
  ଫେରନ୍ତୁ osGetDriveTypeW(z_ମୂଳ)==DRIVE_REMOTE;
}

/*
** Open the shared-memory area associated with database file pDbFd.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winOpenSharedMemory(winFile *pDbFd){
  ସଂରଚନା winShm *p;                  /* The connection to be opened */
  win_Shm_ଗଣ୍ଠି *p_Shm_ଗଣ୍ଠି = 0;          /* The underlying mmapped file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;                /* Result code */
  win_Shm_ଗଣ୍ଠି *pNew;                  /* Newly allocated winShmNode */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nName;                         /* Size of zName in bytes */

  ପ୍ରତିଜ୍ଞା( pDbFd->pShm==0 );    /* Not previously opened */

  /* Allocate space for the new sqlite3_shm object.  Also speculatively
  ** allocate space for a new winShmNode and filename.  */
  p = sqlite3MallocZero( ଆକାର(*p) );
  ଯଦି( p==0 ) ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM_BKPT;
  nName = sqlite3Strlen30(pDbFd->zPath);
  pNew = sqlite3MallocZero( ଆକାର(*p_Shm_ଗଣ୍ଠି) + (i64)nName + 17 );
  ଯଦି( pNew==0 ){
    sqlite3_free(p);
    ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM_BKPT;
  }
  pNew->zFilename = (ଅକ୍ଷର*)&pNew[1];
  pNew->hSharedShm = INVALID_HANDLE_ମୂଲ୍ୟ;
  pNew->isUnlocked = 1;
  ଯଦି( winIsUNCPath(pDbFd->zPath) || winIsRemoteVolume(pDbFd->zPath) ){
    pNew->bUseSharedLockHandle = 1;
  }ଅନ୍ୟଥା{
    pNew->bUseSharedLockHandle = 0;
  }
  sqlite3_snprintf(nName+15, pNew->zFilename, "%s-shm", pDbFd->zPath);
  sqlite3FileSuffix3(pDbFd->zPath, pNew->zFilename);

  /* Look to see if there is an existing winShmNode that can be used.
  ** If no matching winShmNode currently exists, then create a new one.  */
  winShmEnterMutex();
  ପାଇଁ(p_Shm_ଗଣ୍ଠି = win_Shm_ଗଣ୍ଠି_List; p_Shm_ଗଣ୍ଠି; p_Shm_ଗଣ୍ଠି=p_Shm_ଗଣ୍ଠି->pNext){
    /* TBD need to come up with better match here.  Perhaps
    ** use FILE_ID_BOTH_DIR_INFO Structure.  */
    ଯଦି( sqlite3StrICmp(p_Shm_ଗଣ୍ଠି->zFilename, pNew->zFilename)==0 ) ଭାଙ୍ଗ;
  }
  ଯଦି( p_Shm_ଗଣ୍ଠି==0 ){
    p_Shm_ଗଣ୍ଠି = pNew;

    /* Allocate a mutex for this winShmNode object, if one is required. */
    ଯଦି( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_ଗଣ୍ଠି->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      ଯଦି( p_Shm_ଗଣ୍ଠି->mutex==0 ) rc = SQLITE_IOERR_NOMEM_BKPT;
    }

    /* Open a file-handle to use for mappings, and for the DMS lock. */
    ଯଦି( rc==SQLITE_OK ){
      HANDLE h = INVALID_HANDLE_ମୂଲ୍ୟ;
      p_Shm_ଗଣ୍ଠି->isReadonly = sqlite3_uri_boolean(pDbFd->zPath,"readonly_shm",0);
      rc = winHandleOpen(pNew->zFilename, &p_Shm_ଗଣ୍ଠି->isReadonly, &h);
      p_Shm_ଗଣ୍ଠି->hSharedShm = h;
    }

    /* If successful, link the new winShmNode into the global list. If an 
    ** error occurred, free the object. */
    ଯଦି( rc==SQLITE_OK ){
      p_Shm_ଗଣ୍ଠି->pNext = win_Shm_ଗଣ୍ଠି_List;
      win_Shm_ଗଣ୍ଠି_List = p_Shm_ଗଣ୍ଠି;
      pNew = 0;
    }ଅନ୍ୟଥା{
      sqlite3_mutex_free(p_Shm_ଗଣ୍ଠି->mutex);
      ଯଦି( p_Shm_ଗଣ୍ଠି->hSharedShm!=INVALID_HANDLE_ମୂଲ୍ୟ ){
        osCloseHandle(p_Shm_ଗଣ୍ଠି->hSharedShm);
      }
    }
  }

  /* If no error has occurred, link the winShm object to the winShmNode and
  ** the winShm to pDbFd.  */
  ଯଦି( rc==SQLITE_OK ){
    sqlite3_mutex_enter(p_Shm_ଗଣ୍ଠି->mutex);
    p->p_Shm_ଗଣ୍ଠି = p_Shm_ଗଣ୍ଠି;
    p->pWinShmNext = p_Shm_ଗଣ୍ଠି->pWinShmList;
    p_Shm_ଗଣ୍ଠି->pWinShmList = p;
#ଯଦି defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
    p->id = p_Shm_ଗଣ୍ଠି->nextShmId++;
#endif
    pDbFd->pShm = p;
    sqlite3_mutex_leave(p_Shm_ଗଣ୍ଠି->mutex);
  }ଅନ୍ୟଥା ଯଦି( p ){
    sqlite3_free(p);
  }

  ପ୍ରତିଜ୍ଞା( rc!=SQLITE_OK || p_Shm_ଗଣ୍ଠି->isUnlocked==0 || p_Shm_ଗଣ୍ଠି->nRegion==0 );
  winShmLeaveMutex();
  sqlite3_free(pNew);

  /* Open a file-handle on the *-shm file for this connection. This file-handle
  ** is only used for locking. The mapping of the *-shm file is created using
  ** the shared file handle in winShmNode.hSharedShm.  */
  ଯଦି( rc==SQLITE_OK && p_Shm_ଗଣ୍ଠି->bUseSharedLockHandle==0 ){
    p->bReadonly = sqlite3_uri_boolean(pDbFd->zPath, "readonly_shm", 0);
    rc = winHandleOpen(p_Shm_ଗଣ୍ଠି->zFilename, &p->bReadonly, &p->hShm);
    ଯଦି( rc!=SQLITE_OK ){
      ପ୍ରତିଜ୍ଞା( p->hShm==INVALID_HANDLE_ମୂଲ୍ୟ );
      winCloseSharedMemory(pDbFd, 0);
    }
  }

  ଫେରନ୍ତୁ rc;
}

/*
** Close a connection to shared-memory.  Delete the underlying
** storage if deleteFlag is true.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winShmUnmap(
  sqlite3_file *fd,          /* Database holding shared memory */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା deleteFlag             /* Delete after closing if true */
){
  ଫେରନ୍ତୁ winCloseSharedMemory((winFile*)fd, deleteFlag);
}

/*
** Change the lock state for a shared-memory segment.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winShmLock(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ofst,                  /* First lock to acquire or release */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,                     /* Number of locks to acquire or release */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags                  /* What to do with the lock */
){
  winFile *pDbFd = (winFile*)fd;        /* Connection holding shared memory */
  winShm *p = pDbFd->pShm;              /* The shared memory being locked */
  win_Shm_ଗଣ୍ଠି *p_Shm_ଗଣ୍ଠି;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;                   /* Result code */
  u16 mask = (u16)((1U<<(ofst+n)) - (1U<<ofst)); /* Mask of locks to [un]take */

  ଯଦି( p==0 ) ଫେରନ୍ତୁ SQLITE_IOERR_SHMLOCK;
  p_Shm_ଗଣ୍ଠି = p->p_Shm_ଗଣ୍ଠି;
  ଯଦି( NEVER(p_Shm_ଗଣ୍ଠି==0) ) ଫେରନ୍ତୁ SQLITE_IOERR_SHMLOCK;

  ପ୍ରତିଜ୍ଞା( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  ପ୍ରତିଜ୍ଞା( n>=1 );
  ପ୍ରତିଜ୍ଞା( flags==(SQLITE_SHM_LOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_LOCK | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  ପ୍ରତିଜ୍ଞା( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );

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
#ଯଦି defined(SQLITE_ENABLE_SETLK_TIMEOUT) && defined(SQLITE_DEBUG)
  {
    u16 lockMask = (p->exclMask|p->sharedMask);
    ପ୍ରତିଜ୍ଞା( (flags & SQLITE_SHM_UNLOCK) || pDbFd->iBusyTimeout==0 || (
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
  ପ୍ରତିଜ୍ଞା( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK) 
       || 0==(p->exclMask & mask)
  );
  ଯଦି( ((flags & SQLITE_SHM_UNLOCK) && ((p->exclMask|p->sharedMask) & mask))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_LOCK) && 0==(p->sharedMask & mask))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK))
  ){
    HANDLE h = p->hShm;

    ଯଦି( flags & SQLITE_SHM_UNLOCK ){
      /* Case (a) - unlock.  */

      ପ୍ରତିଜ୍ଞା( (p->exclMask & p->sharedMask)==0 );
      ପ୍ରତିଜ୍ଞା( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->exclMask & mask)==mask );
      ପ୍ରତିଜ୍ଞା( !(flags & SQLITE_SHM_SHARED) || (p->sharedMask & mask)==mask );

      ପ୍ରତିଜ୍ଞା( !(flags & SQLITE_SHM_SHARED) || n==1 );
      ଯଦି( p_Shm_ଗଣ୍ଠି->bUseSharedLockHandle ){
        h = p_Shm_ଗଣ୍ଠି->hSharedShm;
        ଯଦି( flags & SQLITE_SHM_SHARED ){
          winShm *pShm;
          sqlite3_mutex_enter(p_Shm_ଗଣ୍ଠି->mutex);
          ପାଇଁ(pShm=p_Shm_ଗଣ୍ଠି->pWinShmList; pShm; pShm=pShm->pWinShmNext){
            ଯଦି( pShm!=p && (pShm->sharedMask & mask) ){
              /* Another connection within this process is also holding this
              ** SHARED lock. So do not actually release the OS lock.  */
              h = INVALID_HANDLE_ମୂଲ୍ୟ;
              ଭାଙ୍ଗ;
            }
          }
          sqlite3_mutex_leave(p_Shm_ଗଣ୍ଠି->mutex);
        }
      }

      ଯଦି( h!=INVALID_HANDLE_ମୂଲ୍ୟ ){
        rc = winHandleUnlock(h, ofst+WIN_SHM_BASE, n);
      }

      /* If successful, also clear the bits in sharedMask/exclMask */
      ଯଦି( rc==SQLITE_OK ){
        p->exclMask = (p->exclMask & ~mask);
        p->sharedMask = (p->sharedMask & ~mask);
      }
    }ଅନ୍ୟଥା{
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା bExcl = ((flags & SQLITE_SHM_EXCLUSIVE) ? 1 : 0);
      DWORD nMs = winFileBusyTimeout(pDbFd);

      ଯଦି( p_Shm_ଗଣ୍ଠି->bUseSharedLockHandle ){
        winShm *pShm;
        h = p_Shm_ଗଣ୍ଠି->hSharedShm;
        sqlite3_mutex_enter(p_Shm_ଗଣ୍ଠି->mutex);
        ପାଇଁ(pShm=p_Shm_ଗଣ୍ଠି->pWinShmList; pShm; pShm=pShm->pWinShmNext){
          ଯଦି( bExcl ){
            ଯଦି( (pShm->sharedMask|pShm->exclMask) & mask ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_ମୂଲ୍ୟ;
            }
          }ଅନ୍ୟଥା{
            ଯଦି( pShm->sharedMask & mask ){
              h = INVALID_HANDLE_ମୂଲ୍ୟ;
            }ଅନ୍ୟଥା ଯଦି( pShm->exclMask & mask ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_ମୂଲ୍ୟ;
            }
          }
        }
        sqlite3_mutex_leave(p_Shm_ଗଣ୍ଠି->mutex);
      }

      ଯଦି( h!=INVALID_HANDLE_ମୂଲ୍ୟ ){
        rc = winHandleLockTimeout(h, ofst+WIN_SHM_BASE, n, bExcl, nMs);
      }
      ଯଦି( rc==SQLITE_OK ){
        ଯଦି( bExcl ){
          p->exclMask = (p->exclMask | mask);
        }ଅନ୍ୟଥା{
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
  ଫେରନ୍ତୁ rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
ସ୍ଥିର ଶୂନ୍ୟ winShmBarrier(
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRegion,                    /* Region to retrieve */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szRegion,                   /* Size of regions */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isWrite,                    /* True to extend file if necessary */
  ଶୂନ୍ୟ ଅସ୍ଥିର **pp              /* OUT: Mapped memory */
){
  winFile *pDbFd = (winFile*)fd;
  winShm *pShm = pDbFd->pShm;
  win_Shm_ଗଣ୍ଠି *p_Shm_ଗଣ୍ଠି;
  DWORD protect = PAGE_READWRITE;
  DWORD flags = FILE_MAP_WRITE | FILE_MAP_READ;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;

  ଯଦି( !pShm ){
    rc = winOpenSharedMemory(pDbFd);
    ଯଦି( rc!=SQLITE_OK ) ଫେରନ୍ତୁ rc;
    pShm = pDbFd->pShm;
    ପ୍ରତିଜ୍ଞା( pShm!=0 );
  }
  p_Shm_ଗଣ୍ଠି = pShm->p_Shm_ଗଣ୍ଠି;

  sqlite3_mutex_enter(p_Shm_ଗଣ୍ଠି->mutex);
  ଯଦି( p_Shm_ଗଣ୍ଠି->isUnlocked ){
    /* Take the DMS lock. */
    ପ୍ରତିଜ୍ଞା( p_Shm_ଗଣ୍ଠି->nRegion==0 );
    rc = winLockSharedMemory(p_Shm_ଗଣ୍ଠି, winFileBusyTimeout(pDbFd));
    ଯଦି( rc!=SQLITE_OK ) ଯାଅ shmpage_out;
  }

  ପ୍ରତିଜ୍ଞା( szRegion==p_Shm_ଗଣ୍ଠି->szRegion || p_Shm_ଗଣ୍ଠି->nRegion==0 );
  ଯଦି( p_Shm_ଗଣ୍ଠି->nRegion<=iRegion ){
    HANDLE hShared = p_Shm_ଗଣ୍ଠି->hSharedShm;
    ସଂରଚନା ShmRegion *apNew;           /* New aRegion[] array */
    i64 nByte = ((i64)iRegion+1)*(i64)szRegion;  /* Minimum file size */
    sqlite3_int64 sz;                  /* Current size of wal-index file */

    p_Shm_ଗଣ୍ଠି->szRegion = szRegion;

    /* The requested region is not mapped into this processes address space.
    ** Check to see if it has been allocated (i.e. if the wal-index file is
    ** large enough to contain the requested region).
    */
    rc = winHandleSize(hShared, &sz);
    ଯଦି( rc!=SQLITE_OK ){
      rc = winLogError(rc, osGetLastError(), "winShmMap1", pDbFd->zPath);
      ଯାଅ shmpage_out;
    }

    ଯଦି( sz<nByte ){
      /* The requested memory region does not exist. If isWrite is set to
      ** zero, exit early. *pp will be set to NULL and SQLITE_OK returned.
      **
      ** Alternatively, if isWrite is non-zero, use ftruncate() to allocate
      ** the requested memory region.  */
      ଯଦି( !isWrite ) ଯାଅ shmpage_out;
      rc = winHandleTruncate(hShared, nByte);
      ଯଦି( rc!=SQLITE_OK ){
        rc = winLogError(rc, osGetLastError(), "winShmMap2", pDbFd->zPath);
        ଯାଅ shmpage_out;
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (ସଂରଚନା ShmRegion*)sqlite3_realloc64(
        p_Shm_ଗଣ୍ଠି->aRegion, ((i64)iRegion+1)*ଆକାର(apNew[0])
    );
    ଯଦି( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      ଯାଅ shmpage_out;
    }
    p_Shm_ଗଣ୍ଠି->aRegion = apNew;

    ଯଦି( p_Shm_ଗଣ୍ଠି->isReadonly ){
      protect = PAGE_READONLY;
      flags = FILE_MAP_READ;
    }

    ଯେତେବେଳେ( p_Shm_ଗଣ୍ଠି->nRegion<=iRegion ){
      HANDLE hMap = ଖାଲି;         /* file-mapping handle */
      ଶୂନ୍ୟ *pMap = 0;             /* Mapped memory region */

#ifdef SQLITE_UWP
      hMap = osCreateFileMappingFromApp(hShared, ଖାଲି, protect, nByte, ଖାଲି);
#ଅନ୍ୟଥା
      hMap = osCreateFileMappingW(hShared, ଖାଲି, protect, 0, nByte, ଖାଲି);
#endif
      OSTRACE(("SHM-MAP-CREATE pid=%lu, region=%d, size=%lld, rc=%s\n",
               osGetCurrentProcessId(), p_Shm_ଗଣ୍ଠି->nRegion, nByte,
               hMap ? "ok" : "failed"));
      ଯଦି( hMap ){
        i64 iOffset = p_Shm_ଗଣ୍ଠି->nRegion*szRegion;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOffsetShift = iOffset % winSysInfo.dwAllocationGranularity;
#ifdef SQLITE_UWP
        pMap = osMapViewOfFileFromApp(hMap, flags,
            iOffset - iOffsetShift, (i64)szRegion + iOffsetShift
        );
#ଅନ୍ୟଥା
        pMap = osMapViewOfFile(hMap, flags,
            0, iOffset - iOffsetShift, (i64)szRegion + iOffsetShift
        );
#endif
        OSTRACE(("SHM-MAP-MAP pid=%lu, region=%d, offset=%d, size=%d, rc=%s\n",
                 osGetCurrentProcessId(), p_Shm_ଗଣ୍ଠି->nRegion, iOffset,
                 szRegion, pMap ? "ok" : "failed"));
      }
      ଯଦି( !pMap ){
        p_Shm_ଗଣ୍ଠି->lastErrno = osGetLastError();
        rc = winLogError(SQLITE_IOERR_SHMMAP, p_Shm_ଗଣ୍ଠି->lastErrno,
                         "winShmMap3", pDbFd->zPath);
        ଯଦି( hMap ) osCloseHandle(hMap);
        ଯାଅ shmpage_out;
      }

      p_Shm_ଗଣ୍ଠି->aRegion[p_Shm_ଗଣ୍ଠି->nRegion].pMap = pMap;
      p_Shm_ଗଣ୍ଠି->aRegion[p_Shm_ଗଣ୍ଠି->nRegion].hMap = hMap;
      p_Shm_ଗଣ୍ଠି->nRegion++;
    }
  }

shmpage_out:
  ଯଦି( p_Shm_ଗଣ୍ଠି->nRegion>iRegion ){
    i64 iOffset = (i64)iRegion*(i64)szRegion;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOffsetShift = iOffset % winSysInfo.dwAllocationGranularity;
    ଅକ୍ଷର *p = (ଅକ୍ଷର *)p_Shm_ଗଣ୍ଠି->aRegion[iRegion].pMap;
    *pp = (ଶୂନ୍ୟ *)&p[iOffsetShift];
  }ଅନ୍ୟଥା{
    *pp = 0;
  }
  ଯଦି( p_Shm_ଗଣ୍ଠି->isReadonly && rc==SQLITE_OK ){
    rc = SQLITE_READONLY;
  }
  sqlite3_mutex_leave(p_Shm_ଗଣ୍ଠି->mutex);
  ଫେରନ୍ତୁ rc;
}

#ଅନ୍ୟଥା
# define winShmMap     0
# define winShmLock    0
# define winShmBarrier 0
# define winShmUnmap   0
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Cleans up the mapped region of the specified file, if any.
*/
#ଯଦି SQLITE_MAX_MMAP_SIZE>0
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winUnmapfile(winFile *pFile){
  ପ୍ରତିଜ୍ଞା( pFile!=0 );
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, pMapRegion=%p, "
           "mmapSize=%lld, mmapSizeMax=%lld\n",
           osGetCurrentProcessId(), pFile, pFile->hMap, pFile->pMapRegion,
           pFile->mmapSize, pFile->mmapSizeMax));
  ଯଦି( pFile->pMapRegion ){
    ଯଦି( !osUnmapViewOfFile(pFile->pMapRegion) ){
      pFile->lastErrno = osGetLastError();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(), pFile,
               pFile->pMapRegion));
      ଫେରନ୍ତୁ winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winUnmapfile1", pFile->zPath);
    }
    pFile->pMapRegion = 0;
    pFile->mmapSize = 0;
  }
  ଯଦି( pFile->hMap!=ଖାଲି ){
    ଯଦି( !osCloseHandle(pFile->hMap) ){
      pFile->lastErrno = osGetLastError();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, rc=SQLITE_IOERR_MMAP\n",
               osGetCurrentProcessId(), pFile, pFile->hMap));
      ଫେରନ୍ତୁ winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winUnmapfile2", pFile->zPath);
    }
    pFile->hMap = ଖାଲି;
  }
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile));
  ଫେରନ୍ତୁ SQLITE_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winMapfile(winFile *pFd, sqlite3_int64 nByte){
  sqlite3_int64 nMap = nByte;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ପ୍ରତିଜ୍ଞା( nMap>=0 || pFd->nFetchOut==0 );
  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, size=%lld\n",
           osGetCurrentProcessId(), pFd, nByte));

  ଯଦି( pFd->nFetchOut>0 ) ଫେରନ୍ତୁ SQLITE_OK;

  ଯଦି( nMap<0 ){
    rc = winFileSize((sqlite3_file*)pFd, &nMap);
    ଯଦି( rc ){
      OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_IOERR_FSTAT\n",
               osGetCurrentProcessId(), pFd));
      ଫେରନ୍ତୁ SQLITE_IOERR_FSTAT;
    }
  }
  ଯଦି( nMap>pFd->mmapSizeMax ){
    nMap = pFd->mmapSizeMax;
  }
  nMap &= ~(sqlite3_int64)(winSysInfo.dwPageSize - 1);

  ଯଦି( nMap==0 && pFd->mmapSize>0 ){
    winUnmapfile(pFd);
  }
  ଯଦି( nMap!=pFd->mmapSize ){
    ଶୂନ୍ୟ *pNew = 0;
    DWORD protect = PAGE_READONLY;
    DWORD flags = FILE_MAP_READ;

    winUnmapfile(pFd);
#ifdef SQLITE_MMAP_READWRITE
    ଯଦି( (pFd->ctrlFlags & WINFILE_RDONLY)==0 ){
      protect = PAGE_READWRITE;
      flags |= FILE_MAP_WRITE;
    }
#endif
#ifdef SQLITE_UWP
    pFd->hMap = osCreateFileMappingFromApp(pFd->h, ଖାଲି, protect, nMap, ଖାଲି);
#ଅନ୍ୟଥା
    pFd->hMap = osCreateFileMappingW(pFd->h, ଖାଲି, protect,
                                (DWORD)((nMap>>32) & 0xffffffff),
                                (DWORD)(nMap & 0xffffffff), ଖାଲି);
#endif
    ଯଦି( pFd->hMap==ଖାଲି ){
      pFd->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->lastErrno,
                       "winMapfile1", pFd->zPath);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-CREATE pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      ଫେରନ୍ତୁ SQLITE_OK;
    }
    ପ୍ରତିଜ୍ଞା( (nMap % winSysInfo.dwPageSize)==0 );
    ପ୍ରତିଜ୍ଞା( ଆକାର(SIZE_T)==ଆକାର(sqlite3_int64) || nMap<=0xffffffff );
#ifdef SQLITE_UWP
    pNew = osMapViewOfFileFromApp(pFd->hMap, flags, 0, (SIZE_T)nMap);
#ଅନ୍ୟଥା
    pNew = osMapViewOfFile(pFd->hMap, flags, 0, 0, (SIZE_T)nMap);
#endif
    ଯଦି( pNew==ଖାଲି ){
      osCloseHandle(pFd->hMap);
      pFd->hMap = ଖାଲି;
      pFd->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->lastErrno,
                       "winMapfile2", pFd->zPath);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-MAP pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      ଫେରନ୍ତୁ SQLITE_OK;
    }
    pFd->pMapRegion = pNew;
    pFd->mmapSize = nMap;
  }

  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFd));
  ଫେରନ୍ତୁ SQLITE_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winFetch(sqlite3_file *fd, i64 iOff, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nAmt, ଶୂନ୍ୟ **pp){
#ଯଦି SQLITE_MAX_MMAP_SIZE>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */
#endif
  *pp = 0;

  OSTRACE(("FETCH pid=%lu, pFile=%p, offset=%lld, amount=%d, pp=%p\n",
           osGetCurrentProcessId(), fd, iOff, nAmt, pp));

#ଯଦି SQLITE_MAX_MMAP_SIZE>0
  ଯଦି( pFd->mmapSizeMax>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା nEofBuffer = 256;
    ଯଦି( pFd->pMapRegion==0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = winMapfile(pFd, -1);
      ଯଦି( rc!=SQLITE_OK ){
        OSTRACE(("FETCH pid=%lu, pFile=%p, rc=%s\n",
                 osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
        ଫେରନ୍ତୁ rc;
      }
    }
    ଯଦି( pFd->mmapSize >= (iOff+nAmt+nEofBuffer) ){
      ପ୍ରତିଜ୍ଞା( pFd->pMapRegion!=0 );
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->nFetchOut++;
    }
  }
#endif

  OSTRACE(("FETCH pid=%lu, pFile=%p, pp=%p, *pp=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd, pp, *pp));
  ଫେରନ୍ତୁ SQLITE_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winUnfetch(sqlite3_file *fd, i64 iOff, ଶୂନ୍ୟ *p){
#ଯଦି SQLITE_MAX_MMAP_SIZE>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  ପ୍ରତିଜ୍ଞା( (p==0)==(pFd->nFetchOut==0) );

  /* If p!=0, it must match the iOff value. */
  ପ୍ରତିଜ୍ଞା( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, offset=%lld, p=%p\n",
           osGetCurrentProcessId(), pFd, iOff, p));

  ଯଦି( p ){
    pFd->nFetchOut--;
  }ଅନ୍ୟଥା{
    /* FIXME:  If Windows truly always prevents truncating or deleting a
    ** file while a mapping is held, then the following winUnmapfile() call
    ** is unnecessary can be omitted - potentially improving
    ** performance.  */
    winUnmapfile(pFd);
  }

  ପ୍ରତିଜ୍ଞା( pFd->nFetchOut>=0 );
#endif

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd));
  ଫେରନ୍ତୁ SQLITE_OK;
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
ସ୍ଥିର ଅଚଳ sqlite3_io_methods winIoMethod = {
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
ସ୍ଥିର ଅଚଳ sqlite3_io_methods winIoNolockMethod = {
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

ସ୍ଥିର winVfsAppData winAppData = {
  &winIoMethod,       /* pMethod */
  0,                  /* pAppData */
  0                   /* bNoLock */
};

ସ୍ଥିର winVfsAppData winNolockAppData = {
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winMakeEndInDirSep(ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf, ଅକ୍ଷର *zBuf){
  ଯଦି( zBuf ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nLen = sqlite3Strlen30(zBuf);
    ଯଦି( nLen>0 ){
      ଯଦି( winIsDirSep(zBuf[nLen-1]) ){
        ଫେରନ୍ତୁ 1;
      }ଅନ୍ୟଥା ଯଦି( nLen+1<nBuf ){
        ଯଦି( !osGetenv ){
          zBuf[nLen] = winGetDirSep();
        }ଅନ୍ୟଥା ଯଦି( winIsDriveLetterAndColon(zBuf) && winIsDirSep(zBuf[2]) ){
          zBuf[nLen] = '\\';
          zBuf[2]='\\';
        }ଅନ୍ୟଥା{
          zBuf[nLen] = '/';
        }
        zBuf[nLen+1] = '\0';
        ଫେରନ୍ତୁ 1;
      }
    }
  }
  ଫେରନ୍ତୁ 0;
}

/*
** If sqlite3_temp_directory is defined, take the mutex and return true.
**
** If sqlite3_temp_directory is NULL (undefined), omit the mutex and
** return false.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winTempDirDefined(ଶୂନ୍ୟ){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ଯଦି( sqlite3_temp_directory!=0 ) ଫେରନ୍ତୁ 1;
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ଫେରନ୍ତୁ 0;
}

/*
** Create a temporary file name and store the resulting pointer into pzBuf.
** The pointer returned in pzBuf must be freed via sqlite3_free().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winGetTempname(sqlite3_vfs *pVfs, ଅକ୍ଷର **pzBuf){
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର zChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  size_t i, j;
  DWORD pid;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPre = sqlite3Strlen30(SQLITE_TEMP_FILE_PREFIX);
  i64 nMax, nBuf, nDir, nLen;
  ଅକ୍ଷର *zBuf;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  SimulateIOError( ଫେରନ୍ତୁ SQLITE_IOERR );

  /* Allocate a temporary buffer to store the fully qualified file
  ** name for the temporary file.  If this fails, we cannot continue.
  */
  nMax = pVfs->mxPathname;
  nBuf = 2 + (i64)nMax;
  zBuf = sqlite3MallocZero( nBuf );
  ଯଦି( !zBuf ){
    OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
    ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM_BKPT;
  }

  /* Figure out the effective temporary directory.  First, check if one
  ** has been explicitly set by the application; otherwise, use the one
  ** configured by the operating system.
  */
  nDir = nMax - (nPre + 15);
  ପ୍ରତିଜ୍ଞା( nDir>0 );
  ଯଦି( winTempDirDefined() ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nDirLen = sqlite3Strlen30(sqlite3_temp_directory);
    ଯଦି( nDirLen>0 ){
      ଯଦି( !winIsDirSep(sqlite3_temp_directory[nDirLen-1]) ){
        nDirLen++;
      }
      ଯଦି( nDirLen>nDir ){
        sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
        sqlite3_free(zBuf);
        OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
        ଫେରନ୍ତୁ winLogError(SQLITE_ERROR, 0, "winGetTempname1", 0);
      }
      sqlite3_snprintf(nMax, zBuf, "%s", sqlite3_temp_directory);
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  }

#ଯଦି defined(__CYGWIN__)
  ଅନ୍ୟଥା ଯଦି( osGetenv!=ଖାଲି ){
    ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *azDirs[] = {
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
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ଅଚଳ ଅକ୍ଷର *zDir = 0;

    ଯଦି( !azDirs[0] ) azDirs[0] = osGetenv("SQLITE_TMPDIR");
    ଯଦି( !azDirs[1] ) azDirs[1] = osGetenv("TMPDIR");
    ଯଦି( !azDirs[2] ) azDirs[2] = osGetenv("TMP");
    ଯଦି( !azDirs[3] ) azDirs[3] = osGetenv("TEMP");
    ଯଦି( !azDirs[4] ) azDirs[4] = osGetenv("USERPROFILE");
    ପାଇଁ(i=0; i<ଆକାର(azDirs)/ଆକାର(azDirs[0]); zDir=azDirs[i++]){
      LPWSTR zConverted;
      ଯଦି( zDir==0 ) ଜାରି;
      /* If the path starts with a drive letter followed by the colon
      ** character, assume it is already a native Win32 path; otherwise,
      ** it must be converted to a native Win32 path via the Cygwin API
      ** prior to using it.
      */
      {
        zConverted = winConvertFromUtf8Filename(zDir);
        ଯଦି( !zConverted ){
          sqlite3_free(zBuf);
          OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
          ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM_BKPT;
        }
        ଯଦି( winIsDir(zConverted) ){
          sqlite3_snprintf(nMax, zBuf, "%s", zDir);
          sqlite3_free(zConverted);
          ଭାଙ୍ଗ;
        }
        sqlite3_free(zConverted);
      }
    }
  }
#endif

  ଅନ୍ୟଥା{
    ଅକ୍ଷର *zMulti;
    LPWSTR zWidePath = sqlite3MallocZero( nMax*ଆକାର(WCHAR) );
    ଯଦି( !zWidePath ){
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM_BKPT;
    }
    ଯଦି( osGetTempPathW==0 || osGetTempPathW(nMax, zWidePath)==0 ){
      sqlite3_free(zWidePath);
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_GETTEMPPATH\n"));
      ଫେରନ୍ତୁ winLogError(SQLITE_IOERR_GETTEMPPATH, osGetLastError(),
                         "winGetTempname2", 0);
    }
    zMulti = winUnicodeToUtf8(zWidePath);
    sqlite3_free(zWidePath);
    ଯଦି( zMulti ){
      sqlite3_snprintf(nMax, zBuf, "%s", zMulti);
      sqlite3_free(zMulti);
    }ଅନ୍ୟଥା{
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM_BKPT;
    }
  }

  /*
  ** Check to make sure the temporary directory ends with an appropriate
  ** separator.  If it does not and there is not enough space left to add
  ** one, fail.
  */
  ଯଦି( !winMakeEndInDirSep(nDir+1, zBuf) ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    ଫେରନ୍ତୁ winLogError(SQLITE_ERROR, 0, "winGetTempname4", 0);
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
  ଯଦି( (nLen + nPre + 17) > nBuf ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    ଫେରନ୍ତୁ winLogError(SQLITE_ERROR, 0, "winGetTempname5", 0);
  }

  sqlite3_snprintf(nBuf-16-nLen, zBuf+nLen, SQLITE_TEMP_FILE_PREFIX);

  j = sqlite3Strlen30(zBuf);
  sqlite3_randomness(15, &zBuf[j]);
  pid = osGetCurrentProcessId();
  ପାଇଁ(i=0; i<15; i++, j++){
    zBuf[j] += pid & 0xff;
    pid >>= 8;
    zBuf[j] = (ଅକ୍ଷର)zChars[ ((unsigned ଅକ୍ଷର)zBuf[j])%(ଆକାର(zChars)-1) ];
  }
  zBuf[j] = 0;
  zBuf[j+1] = 0;
  *pzBuf = zBuf;

  OSTRACE(("TEMP-FILENAME name=%s, rc=SQLITE_OK\n", zBuf));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Return TRUE if the named file is really a directory.  Return false if
** it is something other than a directory, or if there is any kind of memory
** allocation failure.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winIsDir(LPCWSTR zConverted){
  DWORD attr;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;
  DWORD lastErrno;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt = 0;
  WIN32_FILE_ATTRIBUTE_DATA sAttrData;
  memset(&sAttrData, 0, ଆକାର(sAttrData));
  ଯେତେବେଳେ( !(rc = osGetFileAttributesExW(zConverted,
                           GetFileExInfoStandard,
                           &sAttrData)) && winRetryIoerr(&cnt, &lastErrno) ){}
  ଯଦି( !rc ){
    ଫେରନ୍ତୁ 0; /* Invalid name? */
  }
  attr = sAttrData.dwFileAttributes;
  ଫେରନ୍ତୁ (attr!=INVALID_FILE_ATTRIBUTES) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

/* forward reference */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ଅଚଳ ଅକ୍ଷର *zFilename,     /* Name of file to check */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,                 /* Type of test to make on this file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut               /* OUT: Result */
);

/*
** The Windows version of xAccess() accepts an extra bit in the flags
** parameter that prevents an anti-virus retry loop.
*/
#define NORETRY 0x4000

/*
** Open a file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winOpen(
  sqlite3_vfs *pVfs,        /* Used to get maximum path length and AppData */
  ଅଚଳ ଅକ୍ଷର *zName,        /* Name of the file (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,                /* Open mode flags */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pOutFlags            /* Status return flags */
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
  ଅଚଳ ଅକ୍ଷର *zUtf8Name = zName; /* Filename in UTF-8 encoding */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isRO = 0;              /* file is known to be accessible readonly */

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  ଅକ୍ଷର *zTmpname = 0; /* For temporary filename, if necessary. */

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;            /* Function Return Code */
#ଯଦି !defined(NDEBUG)
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType = flags&0x0FFF00;  /* Type of file to open */
#endif

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isCreate     = (flags & SQLITE_OPEN_CREATE);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isReadonly   = (flags & SQLITE_OPEN_READONLY);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isReadWrite  = (flags & SQLITE_OPEN_READWRITE);

#ifndef NDEBUG
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isOpenJournal = (isCreate && (
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
  ପ୍ରତିଜ୍ଞା((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
  ପ୍ରତିଜ୍ଞା(isCreate==0 || isReadWrite);
  ପ୍ରତିଜ୍ଞା(isExclusive==0 || isCreate);
  ପ୍ରତିଜ୍ଞା(isDelete==0 || isCreate);

  /* The main DB, main journal, WAL file and super-journal are never
  ** automatically deleted. Nor are they ever temporary files.  */
  ପ୍ରତିଜ୍ଞା( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_DB );
  ପ୍ରତିଜ୍ଞା( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_JOURNAL );
  ପ୍ରତିଜ୍ଞା( (!isDelete && zName) || eType!=SQLITE_OPEN_SUPER_JOURNAL );
  ପ୍ରତିଜ୍ଞା( (!isDelete && zName) || eType!=SQLITE_OPEN_WAL );

  /* Assert that the upper layer has set one of the "file-type" flags. */
  ପ୍ରତିଜ୍ଞା( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_TEMP_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_TEMP_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );

  ପ୍ରତିଜ୍ଞା( pFile!=0 );
  memset(pFile, 0, ଆକାର(winFile));
  pFile->h = INVALID_HANDLE_ମୂଲ୍ୟ;

  /* If the second argument to this function is NULL, generate a
  ** temporary file name to use
  */
  ଯଦି( !zUtf8Name ){
    ପ୍ରତିଜ୍ଞା( isDelete && !isOpenJournal );
    rc = winGetTempname(pVfs, &zTmpname);
    ଯଦି( rc!=SQLITE_OK ){
      OSTRACE(("OPEN name=%s, rc=%s", zUtf8Name, sqlite3ErrName(rc)));
      ଫେରନ୍ତୁ rc;
    }
    zUtf8Name = zTmpname;
  }

  /* Database filenames are double-zero terminated if they are not
  ** URIs with parameters.  Hence, they can always be passed into
  ** sqlite3_uri_parameter().
  */
  ପ୍ରତିଜ୍ଞା( (eType!=SQLITE_OPEN_MAIN_DB) || (flags & SQLITE_OPEN_URI) ||
       zUtf8Name[sqlite3Strlen30(zUtf8Name)+1]==0 );

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8Name);
  ଯଦି( zConverted==0 ){
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8Name));
    ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM_BKPT;
  }

  ଯଦି( winIsDir(zConverted) ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8Name));
    ଫେରନ୍ତୁ SQLITE_CANTOPEN_ISDIR;
  }

  ଯଦି( isReadWrite ){
    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
  }ଅନ୍ୟଥା{
    dwDesiredAccess = GENERIC_READ;
  }

  /* SQLITE_OPEN_EXCLUSIVE is used to make sure that a new file is
  ** created. SQLite doesn't use it to indicate "exclusive access"
  ** as it is usually understood.
  */
  ଯଦି( isExclusive ){
    /* Creates a new file, only if it does not already exist. */
    /* If the file exists, it fails. */
    dwCreationDisposition = CREATE_NEW;
  }ଅନ୍ୟଥା ଯଦି( isCreate ){
    /* Open existing file, or create if it doesn't exist */
    dwCreationDisposition = OPEN_ALWAYS;
  }ଅନ୍ୟଥା{
    /* Opens a file, only if it exists. */
    dwCreationDisposition = OPEN_EXISTING;
  }

  ଯଦି( 0==sqlite3_uri_boolean(zName, "exclusive", 0) ){
    dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  }ଅନ୍ୟଥା{
    dwShareMode = 0;
  }

  ଯଦି( isDelete ){
    dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY
                               | FILE_ATTRIBUTE_HIDDEN
                               | FILE_FLAG_DELETE_ON_CLOSE;
  }ଅନ୍ୟଥା{
    dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
  }
  କର{
    h = osCreateFileW(zConverted,
                      dwDesiredAccess,
                      dwShareMode, ଖାଲି,
                      dwCreationDisposition,
                      dwFlagsAndAttributes,
                      ଖାଲି);
    ଯଦି( h!=INVALID_HANDLE_ମୂଲ୍ୟ ) ଭାଙ୍ଗ;
    ଯଦି( isReadWrite ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc2;
      sqlite3BeginBenignMalloc();
      rc2 = winAccess(pVfs, zUtf8Name, SQLITE_ACCESS_READ|NORETRY, &isRO);
      sqlite3EndBenignMalloc();
      ଯଦି( rc2==SQLITE_OK && isRO ) ଭାଙ୍ଗ;
    }
  }ଯେତେବେଳେ( winRetryIoerr(&cnt, &lastErrno) );
  winLogIoerr(cnt, __LINE__);

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, rc=%s\n", h, zUtf8Name,
           dwDesiredAccess, (h==INVALID_HANDLE_ମୂଲ୍ୟ) ? "failed" : "ok"));

  ଯଦି( h==INVALID_HANDLE_ମୂଲ୍ୟ ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    ଯଦି( isReadWrite && isRO && !isExclusive ){
      ଫେରନ୍ତୁ winOpen(pVfs, zName, id,
         ((flags|SQLITE_OPEN_READONLY) &
                     ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE)),
         pOutFlags);
    }ଅନ୍ୟଥା{
      pFile->lastErrno = lastErrno;
      winLogError(SQLITE_CANTOPEN, pFile->lastErrno, "winOpen", zUtf8Name);
      ଫେରନ୍ତୁ SQLITE_CANTOPEN_BKPT;
    }
  }

  ଯଦି( pOutFlags ){
    ଯଦି( isReadWrite ){
      *pOutFlags = SQLITE_OPEN_READWRITE;
    }ଅନ୍ୟଥା{
      *pOutFlags = SQLITE_OPEN_READONLY;
    }
  }

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, pOutFlags=%p, *pOutFlags=%d, "
           "rc=%s\n", h, zUtf8Name, dwDesiredAccess, pOutFlags, pOutFlags ?
           *pOutFlags : 0, (h==INVALID_HANDLE_ମୂଲ୍ୟ) ? "failed" : "ok"));

  pAppData = (winVfsAppData*)pVfs->pAppData;

  sqlite3_free(zConverted);
  sqlite3_free(zTmpname);
  id->pMethods = pAppData ? pAppData->pMethod : &winIoMethod;
  pFile->pVfs = pVfs;
  pFile->h = h;
  ଯଦି( isReadonly ){
    pFile->ctrlFlags |= WINFILE_RDONLY;
  }
  ଯଦି( (flags & SQLITE_OPEN_MAIN_DB)
   && sqlite3_uri_boolean(zName, "psow", SQLITE_POWERSAFE_OVERWRITE) 
  ){
    pFile->ctrlFlags |= WINFILE_PSOW;
  }
  pFile->lastErrno = NO_ERROR;
  pFile->zPath = zName;
#ଯଦି SQLITE_MAX_MMAP_SIZE>0
  pFile->hMap = ଖାଲି;
  pFile->pMapRegion = 0;
  pFile->mmapSize = 0;
  pFile->mmapSizeMax = sqlite3GlobalConfig.szMmap;
#endif

  OpenCounter(+1);
  ଫେରନ୍ତୁ rc;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winDelete(
  sqlite3_vfs *pVfs,          /* Not used on win32 */
  ଅଚଳ ଅକ୍ଷର *zFilename,      /* Name of file to delete */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା syncDir                 /* Not used on win32 */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  DWORD attr;
  DWORD lastErrno = 0;
  LPWSTR zConverted;
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(syncDir);

  SimulateIOError(ଫେରନ୍ତୁ SQLITE_IOERR_DELETE);
  OSTRACE(("DELETE name=%s, syncDir=%d\n", zFilename, syncDir));

  zConverted = winConvertFromUtf8Filename(zFilename);
  ଯଦି( zConverted==0 ){
    OSTRACE(("DELETE name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM_BKPT;
  }
  କର {
    attr = osGetFileAttributesW(zConverted);
    ଯଦି ( attr==INVALID_FILE_ATTRIBUTES ){
      lastErrno = osGetLastError();
      ଯଦି( lastErrno==ERROR_FILE_NOT_FOUND
       || lastErrno==ERROR_PATH_NOT_FOUND ){
        rc = SQLITE_IOERR_DELETE_NOENT; /* Already gone? */
      }ଅନ୍ୟଥା{
        rc = SQLITE_ERROR;
      }
      ଭାଙ୍ଗ;
    }
    ଯଦି ( attr&FILE_ATTRIBUTE_DIRECTORY ){
      rc = SQLITE_ERROR; /* Files only. */
      ଭାଙ୍ଗ;
    }
    ଯଦି ( osDeleteFileW(zConverted) ){
      rc = SQLITE_OK; /* Deleted OK. */
      ଭାଙ୍ଗ;
    }
    ଯଦି ( !winRetryIoerr(&cnt, &lastErrno) ){
      rc = SQLITE_ERROR; /* No more retries. */
      ଭାଙ୍ଗ;
    }
  } ଯେତେବେଳେ(1);
  ଯଦି( rc && rc!=SQLITE_IOERR_DELETE_NOENT ){
    rc = winLogError(SQLITE_IOERR_DELETE, lastErrno, "winDelete", zFilename);
  }ଅନ୍ୟଥା{
    winLogIoerr(cnt, __LINE__);
  }
  sqlite3_free(zConverted);
  OSTRACE(("DELETE name=%s, rc=%s\n", zFilename, sqlite3ErrName(rc)));
  ଫେରନ୍ତୁ rc;
}

/*
** Check the existence and status of a file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ଅଚଳ ଅକ୍ଷର *zFilename,     /* Name of file to check */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,                 /* Type of test to make on this file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut               /* OUT: Result */
){
  DWORD attr;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt = 0;
  DWORD lastErrno = 0;
  LPWSTR zConverted;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା noRetry = 0;           /* Do not use winRetryIoerr() */
  WIN32_FILE_ATTRIBUTE_DATA sAttrData;
  UNUSED_PARAMETER(pVfs);

  ଯଦି( (flags & NORETRY)!=0 ){
    noRetry = 1;
    flags &= ~NORETRY;
  }  

  SimulateIOError( ଫେରନ୍ତୁ SQLITE_IOERR_ACCESS; );
  OSTRACE(("ACCESS name=%s, flags=%x, pResOut=%p\n",
           zFilename, flags, pResOut));

  ଯଦି( zFilename==0 ){
    *pResOut = 0;
    OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
             zFilename, pResOut, *pResOut));
    ଫେରନ୍ତୁ SQLITE_OK;
  }

  zConverted = winConvertFromUtf8Filename(zFilename);
  ଯଦି( zConverted==0 ){
    OSTRACE(("ACCESS name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM_BKPT;
  }
  memset(&sAttrData, 0, ଆକାର(sAttrData));
  ଯେତେବେଳେ( !(rc = osGetFileAttributesExW(zConverted,
                           GetFileExInfoStandard,
                           &sAttrData))
     && !noRetry
     && winRetryIoerr(&cnt, &lastErrno)
  ){ /* Loop until true */}
  ଯଦି( rc ){
    /* For an SQLITE_ACCESS_EXISTS query, treat a zero-length file
    ** as if it does not exist.
    */
    ଯଦି(    flags==SQLITE_ACCESS_EXISTS
        && sAttrData.n_File_Size_ଉପର==0
        && sAttrData.n_File_Size_ତଳ==0 ){
      attr = INVALID_FILE_ATTRIBUTES;
    }ଅନ୍ୟଥା{
      attr = sAttrData.dwFileAttributes;
    }
  }ଅନ୍ୟଥା{
    ଯଦି( noRetry ) lastErrno = osGetLastError();
    winLogIoerr(cnt, __LINE__);
    ଯଦି( lastErrno!=ERROR_FILE_NOT_FOUND && lastErrno!=ERROR_PATH_NOT_FOUND ){
      sqlite3_free(zConverted);
      ଫେରନ୍ତୁ winLogError(SQLITE_IOERR_ACCESS, lastErrno, "winAccess",
                         zFilename);
    }ଅନ୍ୟଥା{
      attr = INVALID_FILE_ATTRIBUTES;
    }
  }
  sqlite3_free(zConverted);
  ବାଛ( flags ){
    ସ୍ଥିତି SQLITE_ACCESS_READ:
    ସ୍ଥିତି SQLITE_ACCESS_EXISTS:
      rc = attr!=INVALID_FILE_ATTRIBUTES;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି SQLITE_ACCESS_READWRITE:
      rc = attr!=INVALID_FILE_ATTRIBUTES &&
             (attr & FILE_ATTRIBUTE_READONLY)==0;
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ପ୍ରତିଜ୍ଞା(!"Invalid flags argument");
  }
  *pResOut = rc;
  OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           zFilename, pResOut, *pResOut));
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Returns non-zero if the specified path name starts with the "long path"
** prefix.
*/
ସ୍ଥିର BOOL winIsLongPathPrefix(
  ଅଚଳ ଅକ୍ଷର *zPathname
){
  ଫେରନ୍ତୁ ( zPathname[0]=='\\' && zPathname[1]=='\\'
        && zPathname[2]=='?'  && zPathname[3]=='\\' );
}

/*
** Returns non-zero if the specified path name starts with a drive letter
** followed by a colon character.
*/
ସ୍ଥିର BOOL winIsDriveLetterAndColon(
  ଅଚଳ ଅକ୍ଷର *zPathname
){
  ଫେରନ୍ତୁ ( sqlite3Isalpha(zPathname[0]) && zPathname[1]==':' );
}

#ifdef _WIN32
/*
** Returns non-zero if the specified path name should be used verbatim.  If
** non-zero is returned from this function, the calling function must simply
** use the provided path name verbatim -OR- resolve it into a full path name
** using the GetFullPathName Win32 API function (if available).
*/
ସ୍ଥିର BOOL winIsVerbatimPathname(
  ଅଚଳ ଅକ୍ଷର *zPathname
){
  /*
  ** If the path name starts with a forward slash or a backslash, it is either
  ** a legal UNC name, a volume relative path, or an absolute path name in the
  ** "Unix" format on Windows.  There is no easy way to differentiate between
  ** the final two cases; therefore, we return the safer return value of TRUE
  ** so that callers of this function will simply use it verbatim.
  */
  ଯଦି ( winIsDirSep(zPathname[0]) ){
    ଫେରନ୍ତୁ TRUE;
  }

  /*
  ** If the path name starts with a letter and a colon it is either a volume
  ** relative path or an absolute path.  Callers of this function must not
  ** attempt to treat it as a relative path name (i.e. they should simply use
  ** it verbatim).
  */
  ଯଦି ( winIsDriveLetterAndColon(zPathname) ){
    ଫେରନ୍ତୁ TRUE;
  }

  /*
  ** If we get to this point, the path name should almost certainly be a purely
  ** relative one (i.e. not a UNC name, not absolute, and not volume relative).
  */
  ଫେରନ୍ତୁ FALSE;
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
ସ୍ଥିର ଶୂନ୍ୟ winSimplifyName(ଅକ୍ଷର *z){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
  ପାଇଁ(i=j=0; z[i]; ++i){
    ଯଦି( winIsDirSep(z[i]) ){
#ଯଦି !defined(SQLITE_TEST)
      /* Some test-cases assume that "./foo" and "foo" are different */
      ଯଦି( z[i+1]=='.' && winIsDirSep(z[i+2]) ){
        ++i;
        ଜାରି;
      }
#endif
      ଯଦି( !z[i+1] || (winIsDirSep(z[i+1]) && (i!=0)) ){
        ଜାରି;
      }
      z[j++] = osGetenv?'/':'\\';
    }ଅନ୍ୟଥା{
      z[j++] = z[i];
    }
  }
  ଯେତେବେଳେ(j<i) z[j++] = '\0';
}

#define SQLITE_MAX_SYMLINKS 100

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା mkFullPathname(
  ଅଚଳ ଅକ୍ଷର *zPath,              /* Input path */
  ଅକ୍ଷର *zOut,                     /* Output buffer */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOut                        /* Allocated size of buffer zOut */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPath = sqlite3Strlen30(zPath);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOff = 0;
  ଯଦି( zPath[0]!='/' ){
    ଯଦି( osGetcwd(zOut, nOut-2)==0 ){
      ଫେରନ୍ତୁ winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno, "getcwd", zPath);
    }
    iOff = sqlite3Strlen30(zOut);
    zOut[iOff++] = '/';
  }
  ଯଦି( (iOff+nPath+1)>nOut ){
    /* SQLite assumes that xFullPathname() nul-terminates the output buffer
    ** even if it returns an error.  */
    zOut[iOff] = '\0';
    ଫେରନ୍ତୁ SQLITE_CANTOPEN_BKPT;
  }
  sqlite3_snprintf(nOut-iOff, &zOut[iOff], "%s", zPath);
  ଫେରନ୍ତୁ SQLITE_OK;
}
#endif /* __CYGWIN__ */

/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winFullPathnameNoMutex(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ଅଚଳ ଅକ୍ଷର *zRelative,        /* Possibly relative input path */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFull,                    /* Size of output buffer in bytes */
  ଅକ୍ଷର *zFull                   /* Output buffer */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;
  LPWSTR zConverted;
  ଅକ୍ଷର *zOut;

  /* If this path name begins with "/X:" or "\\?\", where "X" is any
  ** alphabetic character, discard the initial "/" from the pathname.
  */
  ଯଦି( zRelative[0]=='/' && (winIsDriveLetterAndColon(zRelative+1)
       || winIsLongPathPrefix(zRelative+1)) ){
    zRelative++;
  }

  SimulateIOError( ଫେରନ୍ତୁ SQLITE_ERROR );

#ifdef __CYGWIN__
  ଯଦି( osGetcwd ){
    zFull[nFull-1] = '\0';
    ଯଦି( !winIsDriveLetterAndColon(zRelative) || !winIsDirSep(zRelative[2]) ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nLink = 1;              /* Number of symbolic links followed so far */
      ଅଚଳ ଅକ୍ଷର *zIn = zRelative; /* Input path for each iteration of loop */
      ଅକ୍ଷର *zDel = 0;
      ସଂରଚନା stat buf;

      UNUSED_PARAMETER(pVfs);

      କର {
        /* Call lstat() on path zIn. Set bLink to true if the path
        ** is a symbolic link, or false otherwise.  */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା bLink = 0;
        ଯଦି( osLstat && osReadlink ) {
          ଯଦି( osLstat(zIn, &buf)!=0 ){
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା myErrno = osErrno;
            ଯଦି( myErrno!=ENOENT ){
              rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)myErrno,
                               "lstat", zIn);
            }
          }ଅନ୍ୟଥା{
            bLink = ((buf.st_mode & 0170000) == 0120000);
          }

          ଯଦି( bLink ){
            ଯଦି( zDel==0 ){
              zDel = sqlite3MallocZero(nFull);
              ଯଦି( zDel==0 ) rc = SQLITE_NOMEM;
            }ଅନ୍ୟଥା ଯଦି( ++nLink>SQLITE_MAX_SYMLINKS ){
              rc = SQLITE_CANTOPEN_BKPT;
            }

            ଯଦି( rc==SQLITE_OK ){
              nByte = osReadlink(zIn, zDel, nFull-1);
              ଯଦି( nByte ==(DWORD)-1 ){
                rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno,
                                 "readlink", zIn);
              }ଅନ୍ୟଥା{
                ଯଦି( zDel[0]!='/' ){
                  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
                  ପାଇଁ(n = sqlite3Strlen30(zIn); n>0 && zIn[n-1]!='/'; n--);
                  ଯଦି( nByte+n+1>nFull ){
                    rc = SQLITE_CANTOPEN_BKPT;
                  }ଅନ୍ୟଥା{
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

        ପ୍ରତିଜ୍ଞା( rc!=SQLITE_OK || zIn!=zFull || zIn[0]=='/' );
        ଯଦି( rc==SQLITE_OK && zIn!=zFull ){
          rc = mkFullPathname(zIn, zFull, nFull);
        }
        ଯଦି( bLink==0 ) ଭାଙ୍ଗ;
        zIn = zFull;
      }ଯେତେବେଳେ( rc==SQLITE_OK );

      sqlite3_free(zDel);
      winSimplifyName(zFull);
      ଫେରନ୍ତୁ rc;
    }
  }
#endif /* __CYGWIN__ */

#ଯଦି defined(_WIN32)
  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing. This function could fail if, for example, the
  ** current working directory has been unlinked.
  */
  SimulateIOError( ଫେରନ୍ତୁ SQLITE_ERROR );
  ଯଦି ( sqlite3_data_directory && !winIsVerbatimPathname(zRelative) ){
    /*
    ** NOTE: We are dealing with a relative path name and the data
    **       directory has been set.  Therefore, use it as the basis
    **       for converting the relative path name to an absolute
    **       one by prepending the data directory and a backslash.
    */
    sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s%c%s",
                     sqlite3_data_directory, winGetDirSep(), zRelative);
    ଫେରନ୍ତୁ SQLITE_OK;
  }
#endif
  zConverted = winConvertFromUtf8Filename(zRelative);
  ଯଦି( zConverted==0 ){
    ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM_BKPT;
  }
  {
    LPWSTR zTemp;
    nByte = osGetFullPathNameW(zConverted, 0, 0, 0);
    ଯଦି( nByte==0 ){
      sqlite3_free(zConverted);
      ଫେରନ୍ତୁ winLogError(SQLITE_CANTOPEN_FULLPATH, osGetLastError(),
                         "winFullPathname1", zRelative);
    }
    nByte += 3;
    zTemp = sqlite3MallocZero( (u64)nByte*ଆକାର(WCHAR) );
    ଯଦି( zTemp==0 ){
      sqlite3_free(zConverted);
      ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM_BKPT;
    }
    nByte = osGetFullPathNameW(zConverted, nByte, zTemp, 0);
    ଯଦି( nByte==0 ){
      sqlite3_free(zConverted);
      sqlite3_free(zTemp);
      ଫେରନ୍ତୁ winLogError(SQLITE_CANTOPEN_FULLPATH, osGetLastError(),
                         "winFullPathname2", zRelative);
    }
    sqlite3_free(zConverted);
    zOut = winUnicodeToUtf8(zTemp);
    sqlite3_free(zTemp);
  }
  ଯଦି( zOut ){
#ifdef __CYGWIN__
    ଯଦି( memcmp(zOut, "\\\\?\\", 4) ){
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut);
    }ଅନ୍ୟଥା ଯଦି( memcmp(zOut+4, "UNC\\", 4) ){
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut+4);
    }ଅନ୍ୟଥା{
      ଅକ୍ଷର *p = zOut+6;
      *p = '\\';
      ଯଦି( osGetcwd ){
        /* On Cygwin, UNC paths use forward slashes */
        ଯେତେବେଳେ( *p ){
          ଯଦି( *p=='\\' ) *p = '/';
          ++p;
        }
      }
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut+6);
    }
#ଅନ୍ୟଥା
    sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut);
#endif /* __CYGWIN__ */
    sqlite3_free(zOut);
    ଫେରନ୍ତୁ SQLITE_OK;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ SQLITE_IOERR_NOMEM_BKPT;
  }
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ଅଚଳ ଅକ୍ଷର *zRelative,        /* Possibly relative input path */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFull,                    /* Size of output buffer in bytes */
  ଅକ୍ଷର *zFull                   /* Output buffer */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  MUTEX_LOGIC( sqlite3_mutex *pMutex; )
  MUTEX_LOGIC( pMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR); )
  sqlite3_mutex_enter(pMutex);
  rc = winFullPathnameNoMutex(pVfs, zRelative, nFull, zFull);
  sqlite3_mutex_leave(pMutex);
  ଫେରନ୍ତୁ rc;
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
ସ୍ଥିର ଶୂନ୍ୟ *winDlOpen(sqlite3_vfs *pVfs, ଅଚଳ ଅକ୍ଷର *zFilename){
  HANDLE h = 0;
  LPWSTR zConverted = winConvertFromUtf8Filename(zFilename);
  UNUSED_PARAMETER(pVfs);
  ଯଦି( zConverted==0 ){
    OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (ଶୂନ୍ୟ*)0));
    ଫେରନ୍ତୁ 0;
  }
  h = osLoadLibraryW ? osLoadLibraryW(zConverted) : 0;
  OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (ଶୂନ୍ୟ*)h));
  sqlite3_free(zConverted);
  ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)h;
}
ସ୍ଥିର ଶୂନ୍ୟ winDlError(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf, ଅକ୍ଷର *zBufOut){
  UNUSED_PARAMETER(pVfs);
  win_Get_Last_Error_ସନ୍ଦେଶ(osGetLastError(), nBuf, zBufOut);
}
ସ୍ଥିର ଶୂନ୍ୟ (*winDlSym(sqlite3_vfs *pVfs,ଶୂନ୍ୟ *pH,ଅଚଳ ଅକ୍ଷର *zSym))(ଶୂନ୍ୟ){
  FARPROC proc = 0;
  UNUSED_PARAMETER(pVfs);
  proc = osGetProcAddressA ? osGetProcAddressA((HANDLE)pH, zSym) : 0;
  OSTRACE(("DLSYM handle=%p, symbol=%s, address=%p\n",
           (ଶୂନ୍ୟ*)pH, zSym, (ଶୂନ୍ୟ*)proc));
  ଫେରନ୍ତୁ (ଶୂନ୍ୟ(*)(ଶୂନ୍ୟ))proc;
}
ସ୍ଥିର ଶୂନ୍ୟ winDlClose(sqlite3_vfs *pVfs, ଶୂନ୍ୟ *pHandle){
  UNUSED_PARAMETER(pVfs);
  ଯଦି( osFreeLibrary!=0 ) osFreeLibrary((HANDLE)pHandle);
  OSTRACE(("DLCLOSE handle=%p\n", (ଶୂନ୍ୟ*)pHandle));
}
#ଅନ୍ୟଥା /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #define winDlOpen  0
  #define winDlError 0
  #define winDlSym   0
  #define winDlClose 0
#endif

/* State information for the randomness gatherer. */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା EntropyGatherer EntropyGatherer;
ସଂରଚନା EntropyGatherer {
  unsigned ଅକ୍ଷର *a;   /* Gather entropy into this buffer */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା na;             /* Size of a[] in bytes */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;              /* XOR next input into a[i] */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nXor;           /* Number of XOR operations done */
};

#ଯଦି !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
/* Mix sz bytes of entropy into p. */
ସ୍ଥିର ଶୂନ୍ୟ xorMemory(EntropyGatherer *p, unsigned ଅକ୍ଷର *x, ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା j, k;
  ପାଇଁ(j=0, k=p->i; j<sz; j++){
    p->a[k++] ^= x[j];
    ଯଦି( k>=p->na ) k = 0;
  }
  p->i = k;
  p->nXor += sz;
}
#endif /* !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS) */

/*
** Write up to nBuf bytes of randomness into zBuf.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winRandomness(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf, ଅକ୍ଷର *zBuf){
#ଯଦି defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS)
  UNUSED_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  ଫେରନ୍ତୁ nBuf;
#ଅନ୍ୟଥା
  EntropyGatherer e;
  UNUSED_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  e.a = (unsigned ଅକ୍ଷର*)zBuf;
  e.na = nBuf;
  e.nXor = 0;
  e.i = 0;
  {
    FILETIME x;
    osGetSystemTimeAsFileTime(&x);
    xorMemory(&e, (unsigned ଅକ୍ଷର*)&x, ଆକାର(x));
  }
  {
    DWORD pid = osGetCurrentProcessId();
    xorMemory(&e, (unsigned ଅକ୍ଷର*)&pid, ଆକାର(pid));
  }
  {
    ULONGLONG cnt = osGetTickCount64();
    xorMemory(&e, (unsigned ଅକ୍ଷର*)&cnt, ଆକାର(cnt));
  }
  {
    LARGE_INTEGER i;
    osQueryPerformanceCounter(&i);
    xorMemory(&e, (unsigned ଅକ୍ଷର*)&i, ଆକାର(i));
  }
#ଯଦି SQLITE_WIN32_USE_UUID
#ifdef SQLITE_UWP
# error SQLITE_WIN32_USE_UUID is incompatible with SQLITE_UWP
#endif
  {
    UUID id;
    memset(&id, 0, ଆକାର(id));
    osUuidCreate(&id);
    xorMemory(&e, (unsigned ଅକ୍ଷର*)&id, ଆକାର(id));
    memset(&id, 0, ଆକାର(UUID));
    osUuidCreateSequential(&id);
    xorMemory(&e, (unsigned ଅକ୍ଷର*)&id, ଆକାର(id));
  }
#endif /* SQLITE_WIN32_USE_UUID */
  ଫେରନ୍ତୁ e.nXor>nBuf ? nBuf : e.nXor;
#endif /* defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS) */
}


/*
** Sleep for a little while.  Return the amount of time slept.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winSleep(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା microsec){
  osSleep((microsec+999)/1000);
  UNUSED_PARAMETER(pVfs);
  ଫେରନ୍ତୁ ((microsec+999)/1000)*1000;
}

/*
** The following variable, if set to a non-zero value, is interpreted as
** the number of seconds since 1970 and is used to set the result of
** sqlite3OsCurrentTime() during testing.
*/
#ifdef SQLITE_TEST
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_current_time = 0;  /* Fake system time in seconds since 1970. */
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *piNow){
  /* FILETIME structure is a 64-bit value representing the number of
     100-nanosecond intervals since January 1, 1601 (= JD 2305813.5).
  */
  FILETIME ft;
  ସ୍ଥିର ଅଚଳ sqlite3_int64 winFiletimeEpoch = 23058135*(sqlite3_int64)8640000;
#ifdef SQLITE_TEST
  ସ୍ଥିର ଅଚଳ sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
#endif
  /* 2^32 - to avoid use of LL and warnings in gcc */
  ସ୍ଥିର ଅଚଳ sqlite3_int64 max32_Bit_ମୂଲ୍ୟ =
      (sqlite3_int64)2000000000 + (sqlite3_int64)2000000000 +
      (sqlite3_int64)294967296;

  osGetSystemTimeAsFileTime( &ft );
  *piNow = winFiletimeEpoch +
            ((((sqlite3_int64)ft.dw_ଉପର_Date_Time)*max32_Bit_ମୂଲ୍ୟ) +
               (sqlite3_int64)ft.dw_ତଳ_Date_Time)/(sqlite3_int64)10000;

#ifdef SQLITE_TEST
  ଯଦି( sqlite3_current_time ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_time + unixEpoch;
  }
#endif
  UNUSED_PARAMETER(pVfs);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winCurrentTime(sqlite3_vfs *pVfs, ଦ୍ୱିଗୁଣ *prNow){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_int64 i;
  rc = winCurrentTimeInt64(pVfs, &i);
  ଯଦି( !rc ){
    *prNow = i/86400000.0;
  }
  ଫେରନ୍ତୁ rc;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା winGetLastError(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf, ଅକ୍ଷର *zBuf){
  DWORD e = osGetLastError();
  UNUSED_PARAMETER(pVfs);
  ଯଦି( nBuf>0 ) win_Get_Last_Error_ସନ୍ଦେଶ(e, nBuf, zBuf);
  ଫେରନ୍ତୁ e;
}

/*
** Initialize and deinitialize the operating system interface.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_os_init(ଶୂନ୍ୟ){
  ସ୍ଥିର sqlite3_vfs winVfs = {
    3,                     /* iVersion */
    ଆକାର(winFile),       /* szOsFile */
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
  ସ୍ଥିର sqlite3_vfs winLongPathVfs = {
    3,                     /* iVersion */
    ଆକାର(winFile),       /* szOsFile */
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
  ସ୍ଥିର sqlite3_vfs winNolockVfs = {
    3,                     /* iVersion */
    ଆକାର(winFile),       /* szOsFile */
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
  ସ୍ଥିର sqlite3_vfs winLongPathNolockVfs = {
    3,                     /* iVersion */
    ଆକାର(winFile),       /* szOsFile */
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
  ପ୍ରତିଜ୍ଞା( ArraySize(aSyscall)==60 );
  ପ୍ରତିଜ୍ଞା( strcmp(aSyscall[0].zName,"AreFileApisANSI")==0 );
  ପ୍ରତିଜ୍ଞା( strcmp(aSyscall[8].zName,"FreeLibrary")==0 );
  ପ୍ରତିଜ୍ଞା( strcmp(aSyscall[16].zName,"GetSystemInfo")==0 );
  ପ୍ରତିଜ୍ଞା( strcmp(aSyscall[24].zName,"HeapReAlloc")==0 );
  ପ୍ରତିଜ୍ଞା( strcmp(aSyscall[32].zName,"MapViewOfFileFromApp")==0 );
  ପ୍ରତିଜ୍ଞା( strcmp(aSyscall[40].zName,"UnmapViewOfFile")==0 );
  ପ୍ରତିଜ୍ଞା( strcmp(aSyscall[48].zName,"UuidCreate")==0 );
  ପ୍ରତିଜ୍ଞା( strcmp(aSyscall[56].zName,"lstat")==0 );

  /* get memory map allocation granularity */
  memset(&winSysInfo, 0, ଆକାର(SYSTEM_INFO));
  osGetSystemInfo(&winSysInfo);
  ପ୍ରତିଜ୍ଞା( winSysInfo.dwAllocationGranularity>0 );
  ପ୍ରତିଜ୍ଞା( winSysInfo.dwPageSize>0 );

  sqlite3_vfs_register(&winVfs, 1);
  sqlite3_vfs_register(&winLongPathVfs, 0);
  sqlite3_vfs_register(&winNolockVfs, 0);
  sqlite3_vfs_register(&winLongPathNolockVfs, 0);
#ifndef SQLITE_OMIT_WAL
  winBigLock = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif

  ଫେରନ୍ତୁ SQLITE_OK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_os_end(ଶୂନ୍ୟ){
#ifndef SQLITE_OMIT_WAL
  winBigLock = 0;
#endif
  ଫେରନ୍ତୁ SQLITE_OK;
}

#endif /* SQLITE_OS_WIN */
