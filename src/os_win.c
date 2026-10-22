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
#অন্তর্ভাব "sqliteInt.h"
#যদি SQLITE_OS_WIN               /* This file is used for Windows only */

/*
** Include code that is common to all os_*.c files
*/
#অন্তর্ভাব "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#অন্তর্ভাব "os_win.h"

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
                            (আকার(WCHAR) * SQLITE_WINNT_MAX_PATH_CHARS)
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
#ifndef UNUSED_VARIABLE_মান
#  define UNUSED_VARIABLE_মান(x)      (শূন্য)(x)
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
প্রকারলেখ কাঠামো winShm winShm;           /* A connection to shared-memory */
প্রকারলেখ কাঠামো win_Shm_গ্রন্থি win_Shm_গ্রন্থি;   /* A region of shared-memory */
#endif

/*
** The winFile structure is a subclass of sqlite3_file* specific to the win32
** portability layer.
*/
প্রকারলেখ কাঠামো winFile winFile;
কাঠামো winFile {
  ধ্রুবক sqlite3_io_methods *pMethod; /*** Must be first ***/
  sqlite3_vfs *pVfs;      /* The VFS used to open this file */
  HANDLE h;               /* Handle for accessing the file */
  u8 locktype;            /* Type of lock currently held on this file */
  short sharedLockByte;   /* Randomly chosen byte used as a shared lock */
  u8 ctrlFlags;           /* Flags.  See WINFILE_* below */
  DWORD lastErrno;        /* The Windows errno from the last I/O error */
#ifndef SQLITE_OMIT_WAL
  winShm *pShm;           /* Instance of shared memory on this file */
#endif
  ধ্রুবক অক্ষর *zPath;      /* Full pathname of this file */
  পূর্ণসংখ্যা szChunk;            /* Chunk size configured by FCNTL_CHUNK_SIZE */
#যদি SQLITE_MAX_MMAP_আকার>0
  পূর্ণসংখ্যা nFetchOut;                /* Number of outstanding xFetch references */
  HANDLE hMap;                  /* Handle for accessing memory mapping */
  শূন্য *pMapRegion;             /* Area memory mapped */
  sqlite3_int64 mmap_আকার;       /* Size of mapped region */
  sqlite3_int64 mmap_আকার_Max;    /* Configured FCNTL_MMAP_SIZE value */
#endif
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  DWORD iBusyTimeout;        /* Wait this many millisec on locks */
  পূর্ণসংখ্যা bBlockOnConnect;
#endif
};

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
# define winFileBusyTimeout(pDbFd) pDbFd->iBusyTimeout
#অন্যথা
# define winFileBusyTimeout(pDbFd) 0
#endif

/*
** The winVfsAppData structure is used for the pAppData member for all of the
** Win32 VFS variants.
*/
প্রকারলেখ কাঠামো win_Vfs_App_তথ্য win_Vfs_App_তথ্য;
কাঠামো win_Vfs_App_তথ্য {
  ধ্রুবক sqlite3_io_methods *pMethod; /* The file I/O methods to use. */
  শূন্য *p_App_তথ্য;                    /* The extra pAppData, if any. */
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
#ifndef SQLITE_WIN32_DBG_BUF_আকার
#  define SQLITE_WIN32_DBG_BUF_আকার   ((পূর্ণসংখ্যা)(4096-আকার(DWORD)))
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
#ifndef SQLITE_WIN32_HEAP_MAX_INIT_আকার
#  define SQLITE_WIN32_HEAP_MAX_INIT_আকার (4294967295U)
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
#ifndef SQLITE_WIN32_MAX_CACHE_আকার
#  define SQLITE_WIN32_MAX_CACHE_আকার   (((SQLITE_WIN32_HEAP_MAX_INIT_আকার) - \
                                          (SQLITE_WIN32_HEAP_INIT_EXTRA)) / \
                                         (SQLITE_DEFAULT_PAGE_আকার))
#endif

/*
 * This is cache size used in the calculation of the initial size of the
 * Win32-specific heap.  It cannot be negative.
 */
#ifndef SQLITE_WIN32_CACHE_আকার
#  যদি SQLITE_DEFAULT_CACHE_আকার>=0
#    define SQLITE_WIN32_CACHE_আকার     (SQLITE_DEFAULT_CACHE_আকার)
#  অন্যথা
#    define SQLITE_WIN32_CACHE_আকার     (-(SQLITE_DEFAULT_CACHE_আকার))
#  endif
#endif

/*
 * Make sure that the calculated cache size, in pages, cannot cause the
 * initial size of the Win32-specific heap to exceed the maximum amount
 * of memory that can be specified in the call to HeapCreate.
 */
#যদি SQLITE_WIN32_CACHE_আকার>SQLITE_WIN32_MAX_CACHE_আকার
#  undef SQLITE_WIN32_CACHE_আকার
#  define SQLITE_WIN32_CACHE_আকার       (2000)
#endif

/*
 * The initial size of the Win32-specific heap.  This value may be zero.
 */
#ifndef SQLITE_WIN32_HEAP_INIT_আকার
#  define SQLITE_WIN32_HEAP_INIT_আকার   ((SQLITE_WIN32_CACHE_আকার) * \
                                         (SQLITE_DEFAULT_PAGE_আকার) + \
                                         (SQLITE_WIN32_HEAP_INIT_EXTRA))
#endif

/*
 * The maximum size of the Win32-specific heap.  This value may be zero.
 */
#ifndef SQLITE_WIN32_HEAP_MAX_আকার
#  define SQLITE_WIN32_HEAP_MAX_আকার    (0)
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
প্রকারলেখ কাঠামো win_Mem_তথ্য win_Mem_তথ্য;
কাঠামো win_Mem_তথ্য {
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

স্থির কাঠামো win_Mem_তথ্য win_mem_তথ্য = {
#ifndef NDEBUG
  WINMEM_MAGIC1,
#endif
  ফাঁকা, FALSE
#ifndef NDEBUG
  ,WINMEM_MAGIC2
#endif
};

#ifndef NDEBUG
#define winMemAssertMagic1() প্রতিজ্ঞা( win_mem_তথ্য.magic1==WINMEM_MAGIC1 )
#define winMemAssertMagic2() প্রতিজ্ঞা( win_mem_তথ্য.magic2==WINMEM_MAGIC2 )
#define winMemAssertMagic()  winMemAssertMagic1(); winMemAssertMagic2();
#অন্যথা
#define winMemAssertMagic()
#endif

#define win_Mem_Get_তথ্য_Ptr()  &win_mem_তথ্য
#define winMemGetHeap()     win_mem_তথ্য.hHeap
#define winMemGetOwned()    win_mem_তথ্য.bOwned

স্থির শূন্য *winMemMalloc(পূর্ণসংখ্যা nBytes);
স্থির শূন্য winMemFree(শূন্য *pPrior);
স্থির শূন্য *winMemRealloc(শূন্য *pPrior, পূর্ণসংখ্যা nBytes);
স্থির পূর্ণসংখ্যা win_Mem_আকার(শূন্য *p);
স্থির পূর্ণসংখ্যা winMemRoundup(পূর্ণসংখ্যা n);
স্থির পূর্ণসংখ্যা winMemInit(শূন্য *p_App_তথ্য);
স্থির শূন্য winMemShutdown(শূন্য *p_App_তথ্য);

ধ্রুবক sqlite3_mem_methods *sqlite3MemGetWin32(শূন্য);
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
স্থির কাঠামো win_syscall {
  ধ্রুবক অক্ষর *zName;            /* Name of the system call */
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

#যদি defined(SQLITE_UWP)
  { "CreateFileMappingFromApp",(SYSCALL)CreateFileMappingFromApp,0 },
#অন্যথা
  { "CreateFileMappingFromApp",(SYSCALL)0,                       0 },
#endif
#define osCreateFileMappingFromApp ((HANDLE(WINAPI*)(HANDLE,\
        PSECURITY_ATTRIBUTES, \
        ULONG,ULONG64,PCWSTR))aSyscall[3].pCurrent)

#যদি !defined(SQLITE_UWP)
  { "CreateFileMappingW",      (SYSCALL)CreateFileMappingW,      0 },
#অন্যথা
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

#যদি !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "FreeLibrary",             (SYSCALL)FreeLibrary,             0 },
#অন্যথা
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

  { "GetFileSizeEx",           (SYSCALL)Get_File_আকার_Ex,           0 },
#define os_Get_File_আকার_Ex ((BOOL(WINAPI*)(HANDLE, \
                        PLARGE_INTEGER))aSyscall[12].pCurrent)

  { "GetFullPathNameW",        (SYSCALL)GetFullPathNameW,        0 },
#define osGetFullPathNameW ((DWORD(WINAPI*)(LPCWSTR,DWORD,LPWSTR, \
        LPWSTR*))aSyscall[13].pCurrent)

  { "GetLastError",            (SYSCALL)GetLastError,            0 },
#define osGetLastError ((DWORD(WINAPI*)(VOID))aSyscall[14].pCurrent)

#যদি !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "GetProcAddressA",         (SYSCALL)GetProcAddress,          0 },
#অন্যথা
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
#অন্যথা
  { "GetTempPathW",            (SYSCALL)Get_অস্থায়ী_Path_W,            0 },
#endif
#define os_Get_অস্থায়ী_Path_W ((DWORD(WINAPI*)(DWORD,LPWSTR))aSyscall[18].pCurrent)

  { "GetTickCount64",          (SYSCALL)GetTickCount64,          0 },
#define osGetTickCount64 ((ULONGLONG(WINAPI*)(VOID))aSyscall[19].pCurrent)

#ifdef SQLITE_UWP
  { "HeapAlloc",               (SYSCALL)0,                       0 },
#অন্যথা
  { "HeapAlloc",               (SYSCALL)HeapAlloc,               0 },
#endif
#define osHeapAlloc ((LPVOID(WINAPI*)(HANDLE,DWORD, \
        আকার_T))aSyscall[20].pCurrent)

#ifdef SQLITE_UWP
  { "HeapCreate",              (SYSCALL)0,                       0 },
#অন্যথা
  { "HeapCreate",              (SYSCALL)HeapCreate,              0 },
#endif
#define osHeapCreate ((HANDLE(WINAPI*)(DWORD,আকার_T, \
        আকার_T))aSyscall[21].pCurrent)

#ifdef SQLITE_UWP
  { "HeapDestroy",             (SYSCALL)0,                       0 },
#অন্যথা
  { "HeapDestroy",             (SYSCALL)HeapDestroy,             0 },
#endif
#define osHeapDestroy ((BOOL(WINAPI*)(HANDLE))aSyscall[22].pCurrent)

#ifdef SQLITE_UWP
  { "HeapFree",                (SYSCALL)0,                       0 },
#অন্যথা
  { "HeapFree",                (SYSCALL)HeapFree,                0 },
#endif
#define osHeapFree ((BOOL(WINAPI*)(HANDLE,DWORD,LPVOID))aSyscall[23].pCurrent)

#ifdef SQLITE_UWP
  { "HeapReAlloc",             (SYSCALL)0,                       0 },
#অন্যথা
  { "HeapReAlloc",             (SYSCALL)HeapReAlloc,             0 },
#endif
#define osHeapReAlloc ((LPVOID(WINAPI*)(HANDLE,DWORD,LPVOID, \
        আকার_T))aSyscall[24].pCurrent)

#ifdef SQLITE_UWP
  { "HeapSize",                (SYSCALL)0,                       0 },
#অন্যথা
  { "HeapSize",                (SYSCALL)Heap_আকার,                0 },
#endif
#define os_Heap_আকার ((আকার_T(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[25].pCurrent)

#ifdef SQLITE_UWP
  { "HeapValidate",            (SYSCALL)0,                       0 },
#অন্যথা
  { "HeapValidate",            (SYSCALL)HeapValidate,            0 },
#endif
#define osHeapValidate ((BOOL(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[26].pCurrent)

#ifdef SQLITE_UWP
  { "HeapCompact",             (SYSCALL)0,                       0 },
#অন্যথা
  { "HeapCompact",             (SYSCALL)HeapCompact,             0 },
#endif
#define osHeapCompact ((UINT(WINAPI*)(HANDLE,DWORD))aSyscall[27].pCurrent)


#যদি !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "LoadLibraryW",            (SYSCALL)LoadLibraryW,            0 },
#অন্যথা
  { "LoadLibraryW",            (SYSCALL)0,                       0 },
#endif
#define osLoadLibraryW ((HMODULE(WINAPI*)(LPCWSTR))aSyscall[28].pCurrent)

  { "LocalFree",               (SYSCALL)LocalFree,               0 },
#define osLocalFree ((HLOCAL(WINAPI*)(HLOCAL))aSyscall[29].pCurrent)

  { "LockFileEx",              (SYSCALL)LockFileEx,              0 },
#define osLockFileEx ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[30].pCurrent)

#যদি (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_আকার>0) \
 && !defined(SQLITE_UWP)
  { "MapViewOfFile",           (SYSCALL)MapViewOfFile,           0 },
#অন্যথা
  { "MapViewOfFile",           (SYSCALL)0,                       0 },
#endif
#define osMapViewOfFile ((LPVOID(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        আকার_T))aSyscall[31].pCurrent)

#যদি (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_আকার>0) \
 && defined(SQLITE_UWP)
  { "MapViewOfFileFromApp",    (SYSCALL)MapViewOfFileFromApp,   0 },
#অন্যথা
  { "MapViewOfFileFromApp",    (SYSCALL)0,                      0 },
#endif
#define osMapViewOfFileFromApp ((LPVOID(WINAPI*)(HANDLE,ULONG,ULONG64, \
        আকার_T))aSyscall[32].pCurrent)

  { "MultiByteToWideChar",     (SYSCALL)MultiByteToWideChar,     0 },
#define osMultiByteToWideChar ((পূর্ণসংখ্যা(WINAPI*)(UINT,DWORD,LPCSTR,পূর্ণসংখ্যা,LPWSTR, \
        পূর্ণসংখ্যা))aSyscall[33].pCurrent)

  { "QueryPerformanceCounter", (SYSCALL)Query_Performance_গণক, 0 },
#define os_Query_Performance_গণক ((BOOL(WINAPI*)( \
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

#যদি !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_আকার>0
  { "UnmapViewOfFile",         (SYSCALL)UnmapViewOfFile,         0 },
#অন্যথা
  { "UnmapViewOfFile",         (SYSCALL)0,                       0 },
#endif
#define osUnmapViewOfFile ((BOOL(WINAPI*)(LPCVOID))aSyscall[40].pCurrent)

  { "WideCharToMultiByte",     (SYSCALL)WideCharToMultiByte,     0 },
#define osWideCharToMultiByte ((পূর্ণসংখ্যা(WINAPI*)(UINT,DWORD,LPCWSTR,পূর্ণসংখ্যা,LPSTR,পূর্ণসংখ্যা, \
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
#যদি defined(InterlockedCompareExchange)
  { "InterlockedCompareExchange", (SYSCALL)0,                    0 },
#define osInterlockedCompareExchange InterlockedCompareExchange
#অন্যথা
  { "InterlockedCompareExchange", (SYSCALL)InterlockedCompareExchange, 0 },
#define osInterlockedCompareExchange ((LONG(WINAPI*)(LONG অস্থির*,\
        LONG,LONG))aSyscall[47].pCurrent)
#endif /* defined(InterlockedCompareExchange) */

#যদি SQLITE_WIN32_USE_UUID
  { "UuidCreate",               (SYSCALL)UuidCreate,             0 },
#অন্যথা
  { "UuidCreate",               (SYSCALL)0,                      0 },
#endif
#define osUuidCreate ((RPC_STATUS(RPC_ENTRY*)(UUID*))aSyscall[48].pCurrent)

#যদি SQLITE_WIN32_USE_UUID
  { "UuidCreateSequential",     (SYSCALL)UuidCreateSequential,   0 },
#অন্যথা
  { "UuidCreateSequential",     (SYSCALL)0,                      0 },
#endif
#define osUuidCreateSequential \
        ((RPC_STATUS(RPC_ENTRY*)(UUID*))aSyscall[49].pCurrent)

#যদি !defined(SQLITE_NO_SYNC) && SQLITE_MAX_MMAP_আকার>0
  { "FlushViewOfFile",          (SYSCALL)FlushViewOfFile,        0 },
#অন্যথা
  { "FlushViewOfFile",          (SYSCALL)0,                      0 },
#endif
#define osFlushViewOfFile \
        ((BOOL(WINAPI*)(LPCVOID,আকার_T))aSyscall[50].pCurrent)

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  { "CreateEvent",              (SYSCALL)CreateEvent,            0 },
#অন্যথা
  { "CreateEvent",              (SYSCALL)0,                      0 },
#endif
#define osCreateEvent ((HANDLE(WINAPI*)(LPSECURITY_ATTRIBUTES,BOOL, \
                        BOOL,LPCSTR))aSyscall[51].pCurrent)

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  { "CancelIo",                 (SYSCALL)CancelIo,               0 },
#অন্যথা
  { "CancelIo",                 (SYSCALL)0,                      0 },
#endif
#define osCancelIo ((BOOL(WINAPI*)(HANDLE))aSyscall[52].pCurrent)

#ifndef _WIN32
  { "getenv",                   (SYSCALL)getenv,                 0 },
#অন্যথা
  { "getenv",                   (SYSCALL)0,                      0 },
#endif
#define osGetenv ((ধ্রুবক অক্ষর *(*)(ধ্রুবক অক্ষর *))aSyscall[53].pCurrent)

#ifndef _WIN32
  { "getcwd",                   (SYSCALL)getcwd,                 0 },
#অন্যথা
  { "getcwd",                   (SYSCALL)0,                      0 },
#endif
#define osGetcwd ((অক্ষর*(*)(অক্ষর*,size_t))aSyscall[54].pCurrent)

#ifndef _WIN32
  { "readlink",                 (SYSCALL)readlink,               0 },
#অন্যথা
  { "readlink",                 (SYSCALL)0,                      0 },
#endif
#define osReadlink ((ssize_t(*)(ধ্রুবক অক্ষর*,অক্ষর*,size_t))aSyscall[55].pCurrent)

#ifndef _WIN32
  { "lstat",                    (SYSCALL)lstat,                  0 },
#অন্যথা
  { "lstat",                    (SYSCALL)0,                      0 },
#endif
#define osLstat ((পূর্ণসংখ্যা(*)(ধ্রুবক অক্ষর*,কাঠামো stat*))aSyscall[56].pCurrent)

#ifndef _WIN32
  { "__errno",                  (SYSCALL)__errno,                0 },
#অন্যথা
  { "__errno",                  (SYSCALL)0,                      0 },
#endif
#define osErrno (*((পূর্ণসংখ্যা*(*)(শূন্য))aSyscall[57].pCurrent)())

#ifndef _WIN32
  { "cygwin_conv_path",         (SYSCALL)cygwin_conv_path,       0 },
#অন্যথা
  { "cygwin_conv_path",         (SYSCALL)0,                      0 },
#endif
#define osCygwin_conv_path ((size_t(*)(unsigned পূর্ণসংখ্যা, \
    ধ্রুবক শূন্য *, শূন্য *, size_t))aSyscall[58].pCurrent)

  { "GetDriveTypeW",            (SYSCALL)GetDriveTypeW,          0 },
#define osGetDriveTypeW ((UINT(WINAPI*)(LPCWSTR))aSyscall[59].pCurrent)

}; /* End of the overrideable system calls */

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "win32" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
স্থির পূর্ণসংখ্যা winSetSystemCall(
  sqlite3_vfs *pNotUsed,        /* The VFS pointer.  Not used */
  ধ্রুবক অক্ষর *zName,            /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
){
  unsigned পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা rc = SQLITE_NOTFOUND;

  UNUSED_PARAMETER(pNotUsed);
  যদি( zName==0 ){
    /* If no zName is given, restore all system calls to their default
    ** settings and return NULL
    */
    rc = SQLITE_OK;
    জন্য(i=0; i<আকার(aSyscall)/আকার(aSyscall[0]); i++){
      যদি( aSyscall[i].pDefault ){
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  }অন্যথা{
    /* If zName is specified, operate on only the one system call
    ** specified.
    */
    জন্য(i=0; i<আকার(aSyscall)/আকার(aSyscall[0]); i++){
      যদি( strcmp(zName, aSyscall[i].zName)==0 ){
        যদি( aSyscall[i].pDefault==0 ){
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = SQLITE_OK;
        যদি( pNewFunc==0 ) pNewFunc = aSyscall[i].pDefault;
        aSyscall[i].pCurrent = pNewFunc;
        বিরতি;
      }
    }
  }
  ফেরত rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
স্থির sqlite3_syscall_ptr winGetSystemCall(
  sqlite3_vfs *pNotUsed,
  ধ্রুবক অক্ষর *zName
){
  unsigned পূর্ণসংখ্যা i;

  UNUSED_PARAMETER(pNotUsed);
  জন্য(i=0; i<আকার(aSyscall)/আকার(aSyscall[0]); i++){
    যদি( strcmp(zName, aSyscall[i].zName)==0 ) ফেরত aSyscall[i].pCurrent;
  }
  ফেরত 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
স্থির ধ্রুবক অক্ষর *win_পরবর্তী_System_Call(sqlite3_vfs *p, ধ্রুবক অক্ষর *zName){
  পূর্ণসংখ্যা i = -1;

  UNUSED_PARAMETER(p);
  যদি( zName ){
    জন্য(i=0; i<Array_আকার(aSyscall)-1; i++){
      যদি( strcmp(zName, aSyscall[i].zName)==0 ) বিরতি;
    }
  }
  জন্য(i++; i<Array_আকার(aSyscall); i++){
    যদি( aSyscall[i].pCurrent!=0 ) ফেরত aSyscall[i].zName;
  }
  ফেরত 0;
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
পূর্ণসংখ্যা sqlite3_win32_compact_heap(LPUINT pnLargest){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  UINT nLargest = 0;
  HANDLE hHeap;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  প্রতিজ্ঞা( hHeap!=0 );
  প্রতিজ্ঞা( hHeap!=INVALID_HANDLE_মান );
#যদি defined(SQLITE_WIN32_MALLOC_VALIDATE)
  প্রতিজ্ঞা( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, ফাঁকা) );
#endif
  যদি( (nLargest=osHeapCompact(hHeap, SQLITE_WIN32_HEAP_FLAGS))==0 ){
    DWORD lastErrno = osGetLastError();
    যদি( lastErrno==NO_ERROR ){
      sqlite3_log(SQLITE_NOMEM, "failed to HeapCompact (no space), heap=%p",
                  (শূন্য*)hHeap);
      rc = SQLITE_NOMEM_BKPT;
    }অন্যথা{
      sqlite3_log(SQLITE_ERROR, "failed to HeapCompact (%lu), heap=%p",
                  osGetLastError(), (শূন্য*)hHeap);
      rc = SQLITE_ERROR;
    }
  }
  যদি( pnLargest ) *pnLargest = nLargest;
  ফেরত rc;
}

/*
** If a Win32 native heap has been configured, this function will attempt to
** destroy and recreate it.  If the Win32 native heap is not isolated and/or
** the sqlite3_memory_used() function does not return zero, SQLITE_BUSY will
** be returned and no changes will be made to the Win32 native heap.
*/
পূর্ণসংখ্যা sqlite3_win32_reset_heap(){
  পূর্ণসংখ্যা rc;
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; ) /* The main static mutex */
  MUTEX_LOGIC( sqlite3_mutex *pMem; )    /* The memsys static mutex */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  MUTEX_LOGIC( pMem = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3_mutex_enter(pMem);
  winMemAssertMagic();
  যদি( winMemGetHeap()!=ফাঁকা && winMemGetOwned() && sqlite3_memory_used()==0 ){
    /*
    ** At this point, there should be no outstanding memory allocations on
    ** the heap.  Also, since both the main and memsys locks are currently
    ** being held by us, no other function (i.e. from another thread) should
    ** be able to even access the heap.  Attempt to destroy and recreate our
    ** isolated Win32 native heap now.
    */
    প্রতিজ্ঞা( winMemGetHeap()!=ফাঁকা );
    প্রতিজ্ঞা( winMemGetOwned() );
    প্রতিজ্ঞা( sqlite3_memory_used()==0 );
    winMemShutdown(win_Mem_Get_তথ্য_Ptr());
    প্রতিজ্ঞা( winMemGetHeap()==ফাঁকা );
    প্রতিজ্ঞা( !winMemGetOwned() );
    প্রতিজ্ঞা( sqlite3_memory_used()==0 );
    rc = winMemInit(win_Mem_Get_তথ্য_Ptr());
    প্রতিজ্ঞা( rc!=SQLITE_OK || winMemGetHeap()!=ফাঁকা );
    প্রতিজ্ঞা( rc!=SQLITE_OK || winMemGetOwned() );
    প্রতিজ্ঞা( rc!=SQLITE_OK || sqlite3_memory_used()==0 );
  }অন্যথা{
    /*
    ** The Win32 native heap cannot be modified because it may be in use.
    */
    rc = SQLITE_BUSY;
  }
  sqlite3_mutex_leave(pMem);
  sqlite3_mutex_leave(pMainMtx);
  ফেরত rc;
}
#endif /* SQLITE_WIN32_MALLOC */

#ifdef _WIN32
/*
** This function outputs the specified (ANSI) string to the Win32 debugger
** (if available).  Undocumented.  Might go away at any moment.
*/
শূন্য sqlite3_win32_write_debug(ধ্রুবক অক্ষর *zBuf, পূর্ণসংখ্যা nBuf){
  অক্ষর zDbgBuf[SQLITE_WIN32_DBG_BUF_আকার];
  পূর্ণসংখ্যা nMin = MIN(nBuf, (SQLITE_WIN32_DBG_BUF_আকার - 1)); /* may be negative. */
  যদি( nMin<-1 ) nMin = -1; /* all negative values become -1. */
  প্রতিজ্ঞা( nMin==-1 || nMin==0 || nMin<SQLITE_WIN32_DBG_BUF_আকার );
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !zBuf ){
    (শূন্য)SQLITE_MISUSE_BKPT;
    ফেরত;
  }
#endif
  যদি( nMin>0 ){
    memset(zDbgBuf, 0, SQLITE_WIN32_DBG_BUF_আকার);
    memcpy(zDbgBuf, zBuf, nMin);
    osOutputDebugStringA(zDbgBuf);
  }অন্যথা{
    osOutputDebugStringA(zBuf);
  }
}

শূন্য sqlite3_win32_sleep(DWORD milliseconds){
  osSleep(milliseconds);
}

#যদি SQLITE_MAX_WORKER_THREADS>0 && SQLITE_THREADSAFE>0
DWORD sqlite3Win32Wait(HANDLE hObject){
  DWORD rc;
  যখন( (rc = osWaitForSingleObjectEx(hObject, INFINITE,
                                       TRUE))==WAIT_IO_COMPLETION ){}
  ফেরত rc;
}
#endif
#endif /* _WIN32 */

#ifdef SQLITE_WIN32_MALLOC
/*
** Allocate nBytes of memory.
*/
স্থির শূন্য *winMemMalloc(পূর্ণসংখ্যা nBytes){
  HANDLE hHeap;
  শূন্য *p;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  প্রতিজ্ঞা( hHeap!=0 );
  প্রতিজ্ঞা( hHeap!=INVALID_HANDLE_মান );
#যদি defined(SQLITE_WIN32_MALLOC_VALIDATE)
  প্রতিজ্ঞা( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, ফাঁকা) );
#endif
  প্রতিজ্ঞা( nBytes>=0 );
  p = osHeapAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, (আকার_T)nBytes);
  যদি( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapAlloc %u bytes (%lu), heap=%p",
                nBytes, osGetLastError(), (শূন্য*)hHeap);
  }
  ফেরত p;
}

/*
** Free memory.
*/
স্থির শূন্য winMemFree(শূন্য *pPrior){
  HANDLE hHeap;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  প্রতিজ্ঞা( hHeap!=0 );
  প্রতিজ্ঞা( hHeap!=INVALID_HANDLE_মান );
#যদি defined(SQLITE_WIN32_MALLOC_VALIDATE)
  প্রতিজ্ঞা( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) );
#endif
  যদি( !pPrior ) ফেরত; /* Passing NULL to HeapFree is undefined. */
  যদি( !osHeapFree(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapFree block %p (%lu), heap=%p",
                pPrior, osGetLastError(), (শূন্য*)hHeap);
  }
}

/*
** Change the size of an existing memory allocation
*/
স্থির শূন্য *winMemRealloc(শূন্য *pPrior, পূর্ণসংখ্যা nBytes){
  HANDLE hHeap;
  শূন্য *p;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  প্রতিজ্ঞা( hHeap!=0 );
  প্রতিজ্ঞা( hHeap!=INVALID_HANDLE_মান );
#যদি defined(SQLITE_WIN32_MALLOC_VALIDATE)
  প্রতিজ্ঞা( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) );
#endif
  প্রতিজ্ঞা( nBytes>=0 );
  যদি( !pPrior ){
    p = osHeapAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, (আকার_T)nBytes);
  }অন্যথা{
    p = osHeapReAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior, (আকার_T)nBytes);
  }
  যদি( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to %s %u bytes (%lu), heap=%p",
                pPrior ? "HeapReAlloc" : "HeapAlloc", nBytes, osGetLastError(),
                (শূন্য*)hHeap);
  }
  ফেরত p;
}

/*
** Return the size of an outstanding allocation, in bytes.
*/
স্থির পূর্ণসংখ্যা win_Mem_আকার(শূন্য *p){
  HANDLE hHeap;
  আকার_T n;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  প্রতিজ্ঞা( hHeap!=0 );
  প্রতিজ্ঞা( hHeap!=INVALID_HANDLE_মান );
#যদি defined(SQLITE_WIN32_MALLOC_VALIDATE)
  প্রতিজ্ঞা( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, p) );
#endif
  যদি( !p ) ফেরত 0;
  n = os_Heap_আকার(hHeap, SQLITE_WIN32_HEAP_FLAGS, p);
  যদি( n==(আকার_T)-1 ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapSize block %p (%lu), heap=%p",
                p, osGetLastError(), (শূন্য*)hHeap);
    ফেরত 0;
  }
  ফেরত (পূর্ণসংখ্যা)n;
}

/*
** Round up a request size to the next valid allocation size.
*/
স্থির পূর্ণসংখ্যা winMemRoundup(পূর্ণসংখ্যা n){
  ফেরত n;
}

/*
** Initialize this module.
*/
স্থির পূর্ণসংখ্যা winMemInit(শূন্য *p_App_তথ্য){
  win_Mem_তথ্য *p_Win_Mem_তথ্য = (win_Mem_তথ্য *)p_App_তথ্য;

  যদি( !p_Win_Mem_তথ্য ) ফেরত SQLITE_ERROR;
  প্রতিজ্ঞা( p_Win_Mem_তথ্য->magic1==WINMEM_MAGIC1 );
  প্রতিজ্ঞা( p_Win_Mem_তথ্য->magic2==WINMEM_MAGIC2 );

#যদি SQLITE_WIN32_HEAP_CREATE
  যদি( !p_Win_Mem_তথ্য->hHeap ){
    DWORD dw_Initial_আকার = SQLITE_WIN32_HEAP_INIT_আকার;
    DWORD dw_Maximum_আকার = (DWORD)sqlite3GlobalConfig.nHeap;
    যদি( dw_Maximum_আকার==0 ){
      dw_Maximum_আকার = SQLITE_WIN32_HEAP_MAX_আকার;
    }অন্যথা যদি( dw_Initial_আকার>dw_Maximum_আকার ){
      dw_Initial_আকার = dw_Maximum_আকার;
    }
    p_Win_Mem_তথ্য->hHeap = osHeapCreate(SQLITE_WIN32_HEAP_FLAGS,
                                      dw_Initial_আকার, dw_Maximum_আকার);
    যদি( !p_Win_Mem_তথ্য->hHeap ){
      sqlite3_log(SQLITE_NOMEM,
          "failed to HeapCreate (%lu), flags=%u, initSize=%lu, maxSize=%lu",
          osGetLastError(), SQLITE_WIN32_HEAP_FLAGS, dw_Initial_আকার,
          dw_Maximum_আকার);
      ফেরত SQLITE_NOMEM_BKPT;
    }
    p_Win_Mem_তথ্য->bOwned = TRUE;
    প্রতিজ্ঞা( p_Win_Mem_তথ্য->bOwned );
  }
#অন্যথা
  p_Win_Mem_তথ্য->hHeap = osGetProcessHeap();
  যদি( !p_Win_Mem_তথ্য->hHeap ){
    sqlite3_log(SQLITE_NOMEM,
        "failed to GetProcessHeap (%lu)", osGetLastError());
    ফেরত SQLITE_NOMEM_BKPT;
  }
  p_Win_Mem_তথ্য->bOwned = FALSE;
  প্রতিজ্ঞা( !p_Win_Mem_তথ্য->bOwned );
#endif
  প্রতিজ্ঞা( p_Win_Mem_তথ্য->hHeap!=0 );
  প্রতিজ্ঞা( p_Win_Mem_তথ্য->hHeap!=INVALID_HANDLE_মান );
#যদি defined(SQLITE_WIN32_MALLOC_VALIDATE)
  প্রতিজ্ঞা( osHeapValidate(p_Win_Mem_তথ্য->hHeap, SQLITE_WIN32_HEAP_FLAGS, ফাঁকা) );
#endif
  ফেরত SQLITE_OK;
}

/*
** Deinitialize this module.
*/
স্থির শূন্য winMemShutdown(শূন্য *p_App_তথ্য){
  win_Mem_তথ্য *p_Win_Mem_তথ্য = (win_Mem_তথ্য *)p_App_তথ্য;

  যদি( !p_Win_Mem_তথ্য ) ফেরত;
  প্রতিজ্ঞা( p_Win_Mem_তথ্য->magic1==WINMEM_MAGIC1 );
  প্রতিজ্ঞা( p_Win_Mem_তথ্য->magic2==WINMEM_MAGIC2 );

  যদি( p_Win_Mem_তথ্য->hHeap ){
    প্রতিজ্ঞা( p_Win_Mem_তথ্য->hHeap!=INVALID_HANDLE_মান );
#যদি defined(SQLITE_WIN32_MALLOC_VALIDATE)
    প্রতিজ্ঞা( osHeapValidate(p_Win_Mem_তথ্য->hHeap, SQLITE_WIN32_HEAP_FLAGS, ফাঁকা) );
#endif
    যদি( p_Win_Mem_তথ্য->bOwned ){
      যদি( !osHeapDestroy(p_Win_Mem_তথ্য->hHeap) ){
        sqlite3_log(SQLITE_NOMEM, "failed to HeapDestroy (%lu), heap=%p",
                    osGetLastError(), (শূন্য*)p_Win_Mem_তথ্য->hHeap);
      }
      p_Win_Mem_তথ্য->bOwned = FALSE;
    }
    p_Win_Mem_তথ্য->hHeap = ফাঁকা;
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
ধ্রুবক sqlite3_mem_methods *sqlite3MemGetWin32(শূন্য){
  স্থির ধ্রুবক sqlite3_mem_methods winMemMethods = {
    winMemMalloc,
    winMemFree,
    winMemRealloc,
    win_Mem_আকার,
    winMemRoundup,
    winMemInit,
    winMemShutdown,
    &win_mem_তথ্য
  };
  ফেরত &winMemMethods;
}

শূন্য sqlite3MemSetDefault(শূন্য){
  sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetWin32());
}
#endif /* SQLITE_WIN32_MALLOC */

#ifdef _WIN32
/*
** Convert a UTF-8 string to Microsoft Unicode.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
স্থির LPWSTR winUtf8ToUnicode(ধ্রুবক অক্ষর *zText){
  পূর্ণসংখ্যা nChar;
  LPWSTR zWideText;

  nChar = osMultiByteToWideChar(CP_UTF8, 0, zText, -1, ফাঁকা, 0);
  যদি( nChar==0 ){
    ফেরত 0;
  }
  zWideText = sqlite3MallocZero( nChar*আকার(WCHAR) );
  যদি( zWideText==0 ){
    ফেরত 0;
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zText, -1, zWideText,
                                nChar);
  যদি( nChar==0 ){
    sqlite3_free(zWideText);
    zWideText = 0;
  }
  ফেরত zWideText;
}
#endif /* _WIN32 */

/*
** Convert a Microsoft Unicode string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
স্থির অক্ষর *winUnicodeToUtf8(LPCWSTR zWideText){
  পূর্ণসংখ্যা nByte;
  অক্ষর *zText;

  nByte = osWideCharToMultiByte(CP_UTF8, 0, zWideText, -1, 0, 0, 0, 0);
  যদি( nByte == 0 ){
    ফেরত 0;
  }
  zText = sqlite3MallocZero( nByte );
  যদি( zText==0 ){
    ফেরত 0;
  }
  nByte = osWideCharToMultiByte(CP_UTF8, 0, zWideText, -1, zText, nByte,
                                0, 0);
  যদি( nByte == 0 ){
    sqlite3_free(zText);
    zText = 0;
  }
  ফেরত zText;
}

#ifdef _WIN32
/*
** Convert an ANSI string to Microsoft Unicode, using the ANSI or OEM
** code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
স্থির LPWSTR winMbcsToUnicode(ধ্রুবক অক্ষর *zText, পূর্ণসংখ্যা useAnsi){
  পূর্ণসংখ্যা nWideChar;
  LPWSTR zMbcsText;
  পূর্ণসংখ্যা codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nWideChar = osMultiByteToWideChar(codepage, 0, zText, -1, ফাঁকা,
                                0);
  যদি( nWideChar==0 ){
    ফেরত 0;
  }
  zMbcsText = sqlite3MallocZero( nWideChar*আকার(WCHAR) );
  যদি( zMbcsText==0 ){
    ফেরত 0;
  }
  nWideChar = osMultiByteToWideChar(codepage, 0, zText, -1, zMbcsText,
                                nWideChar);
  যদি( nWideChar==0 ){
    sqlite3_free(zMbcsText);
    zMbcsText = 0;
  }
  ফেরত zMbcsText;
}

/*
** Convert a Microsoft Unicode string to a multi-byte character string,
** using the ANSI or OEM code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
স্থির অক্ষর *winUnicodeToMbcs(LPCWSTR zWideText, পূর্ণসংখ্যা useAnsi){
  পূর্ণসংখ্যা nByte;
  অক্ষর *zText;
  পূর্ণসংখ্যা codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nByte = osWideCharToMultiByte(codepage, 0, zWideText, -1, 0, 0, 0, 0);
  যদি( nByte == 0 ){
    ফেরত 0;
  }
  zText = sqlite3MallocZero( nByte );
  যদি( zText==0 ){
    ফেরত 0;
  }
  nByte = osWideCharToMultiByte(codepage, 0, zWideText, -1, zText,
                                nByte, 0, 0);
  যদি( nByte == 0 ){
    sqlite3_free(zText);
    zText = 0;
  }
  ফেরত zText;
}

/*
** Convert a multi-byte character string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
স্থির অক্ষর *winMbcsToUtf8(ধ্রুবক অক্ষর *zText, পূর্ণসংখ্যা useAnsi){
  অক্ষর *zTextUtf8;
  LPWSTR zTmpWide;

  zTmpWide = winMbcsToUnicode(zText, useAnsi);
  যদি( zTmpWide==0 ){
    ফেরত 0;
  }
  zTextUtf8 = winUnicodeToUtf8(zTmpWide);
  sqlite3_free(zTmpWide);
  ফেরত zTextUtf8;
}

/*
** Convert a UTF-8 string to a multi-byte character string.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
স্থির অক্ষর *winUtf8ToMbcs(ধ্রুবক অক্ষর *zText, পূর্ণসংখ্যা useAnsi){
  অক্ষর *zTextMbcs;
  LPWSTR zTmpWide;

  zTmpWide = winUtf8ToUnicode(zText);
  যদি( zTmpWide==0 ){
    ফেরত 0;
  }
  zTextMbcs = winUnicodeToMbcs(zTmpWide, useAnsi);
  sqlite3_free(zTmpWide);
  ফেরত zTextMbcs;
}

/*
** This is a public wrapper for the winUtf8ToUnicode() function.
*/
LPWSTR sqlite3_win32_utf8_to_unicode(ধ্রুবক অক্ষর *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !zText ){
    (শূন্য)SQLITE_MISUSE_BKPT;
    ফেরত 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  যদি( sqlite3_initialize() ) ফেরত 0;
#endif
  ফেরত winUtf8ToUnicode(zText);
}

/*
** This is a public wrapper for the winUnicodeToUtf8() function.
*/
অক্ষর *sqlite3_win32_unicode_to_utf8(LPCWSTR zWideText){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !zWideText ){
    (শূন্য)SQLITE_MISUSE_BKPT;
    ফেরত 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  যদি( sqlite3_initialize() ) ফেরত 0;
#endif
  ফেরত winUnicodeToUtf8(zWideText);
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
অক্ষর *sqlite3_win32_mbcs_to_utf8(ধ্রুবক অক্ষর *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !zText ){
    (শূন্য)SQLITE_MISUSE_BKPT;
    ফেরত 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  যদি( sqlite3_initialize() ) ফেরত 0;
#endif
  ফেরত winMbcsToUtf8(zText, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
অক্ষর *sqlite3_win32_mbcs_to_utf8_v2(ধ্রুবক অক্ষর *zText, পূর্ণসংখ্যা useAnsi){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !zText ){
    (শূন্য)SQLITE_MISUSE_BKPT;
    ফেরত 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  যদি( sqlite3_initialize() ) ফেরত 0;
#endif
  ফেরত winMbcsToUtf8(zText, useAnsi);
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
অক্ষর *sqlite3_win32_utf8_to_mbcs(ধ্রুবক অক্ষর *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !zText ){
    (শূন্য)SQLITE_MISUSE_BKPT;
    ফেরত 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  যদি( sqlite3_initialize() ) ফেরত 0;
#endif
  ফেরত winUtf8ToMbcs(zText, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
অক্ষর *sqlite3_win32_utf8_to_mbcs_v2(ধ্রুবক অক্ষর *zText, পূর্ণসংখ্যা useAnsi){
#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !zText ){
    (শূন্য)SQLITE_MISUSE_BKPT;
    ফেরত 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  যদি( sqlite3_initialize() ) ফেরত 0;
#endif
  ফেরত winUtf8ToMbcs(zText, useAnsi);
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-8 string.
*/
পূর্ণসংখ্যা sqlite3_win32_set_directory8(
  unsigned long type, /* Identifier for directory being set or reset */
  ধ্রুবক অক্ষর *z_মান  /* New value for directory being set or reset */
){
  অক্ষর **ppDirectory = 0;
  পূর্ণসংখ্যা rc;
#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  যদি( rc ) ফেরত rc;
#endif
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  যদি( type==SQLITE_WIN32_তথ্য_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_তথ্য_directory;
  }অন্যথা যদি( type==SQLITE_WIN32_অস্থায়ী_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_অস্থায়ী_directory;
  }
  প্রতিজ্ঞা( !ppDirectory || type==SQLITE_WIN32_তথ্য_DIRECTORY_TYPE
          || type==SQLITE_WIN32_অস্থায়ী_DIRECTORY_TYPE
  );
  প্রতিজ্ঞা( !ppDirectory || sqlite3MemdebugHasType(*ppDirectory, MEMTYPE_HEAP) );
  যদি( ppDirectory ){
    অক্ষর *zCopy = 0;
    যদি( z_মান && z_মান[0] ){
      zCopy = sqlite3_mprintf("%s", z_মান);
      যদি ( zCopy==0 ){
        rc = SQLITE_NOMEM_BKPT;
        যাও set_directory8_done;
      }
    }
    sqlite3_free(*ppDirectory);
    *ppDirectory = zCopy;
    rc = SQLITE_OK;
  }অন্যথা{
    rc = SQLITE_ERROR;
  }
set_directory8_done:
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ফেরত rc;
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-16 string.
*/
পূর্ণসংখ্যা sqlite3_win32_set_directory16(
  unsigned long type, /* Identifier for directory being set or reset */
  ধ্রুবক শূন্য *z_মান  /* New value for directory being set or reset */
){
  পূর্ণসংখ্যা rc;
  অক্ষর *zUtf8 = 0;
  যদি( z_মান ){
    zUtf8 = sqlite3_win32_unicode_to_utf8(z_মান);
    যদি( zUtf8==0 ) ফেরত SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_win32_set_directory8(type, zUtf8);
  যদি( zUtf8 ) sqlite3_free(zUtf8);
  ফেরত rc;
}

/*
** This function sets the data directory or the temporary directory based on
** the provided arguments.  The type argument must be 1 in order to set the
** data directory or 2 in order to set the temporary directory.  The zValue
** argument is the name of the directory to use.  The return value will be
** SQLITE_OK if successful.
*/
পূর্ণসংখ্যা sqlite3_win32_set_directory(
  unsigned long type, /* Identifier for directory being set or reset */
  শূন্য *z_মান        /* New value for directory being set or reset */
){
  ফেরত sqlite3_win32_set_directory16(type, z_মান);
}
#endif /* _WIN32 */

/*
** The return value of winGetLastErrorMsg
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated).
*/
স্থির পূর্ণসংখ্যা win_Get_Last_Error_বার্তা(DWORD lastErrno, পূর্ণসংখ্যা nBuf, অক্ষর *zBuf){
  /* FormatMessage returns 0 on failure.  Otherwise it
  ** returns the number of TCHARs written to the output
  ** buffer, excluding the terminating null char.
  */
  DWORD dwLen = 0;
  অক্ষর *zOut = 0;
  LPWSTR z_অস্থায়ী_Wide = ফাঁকা;
  dwLen = osFormatMessageW(FORMAT_MESSAGE_ALLOCATE_বাফার |
                           FORMAT_MESSAGE_FROM_SYSTEM |
                           FORMAT_MESSAGE_IGNORE_INSERTS,
                           ফাঁকা,
                           lastErrno,
                           0,
                           (LPWSTR) &z_অস্থায়ী_Wide,
                           0,
                           0);
  যদি( dwLen > 0 ){
    /* allocate a buffer and convert to UTF8 */
    sqlite3BeginBenignMalloc();
    zOut = winUnicodeToUtf8(z_অস্থায়ী_Wide);
    sqlite3EndBenignMalloc();
    /* free the system buffer allocated by FormatMessage */
    osLocalFree(z_অস্থায়ী_Wide);
  }
  যদি( 0 == dwLen ){
    sqlite3_snprintf(nBuf, zBuf, "OsError 0x%lx (%lu)", lastErrno, lastErrno);
  }অন্যথা{
    /* copy a maximum of nBuf chars to output buffer */
    sqlite3_snprintf(nBuf, zBuf, "%s", zOut);
    /* free the UTF8 buffer */
    sqlite3_free(zOut);
  }
  ফেরত 0;
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
#define winLogError(a,b,c,d)   win_Log_Error_At_সারি(a,b,c,d,__LINE__)
স্থির পূর্ণসংখ্যা win_Log_Error_At_সারি(
  পূর্ণসংখ্যা errcode,                    /* SQLite error code */
  DWORD lastErrno,                /* Win32 last error */
  ধ্রুবক অক্ষর *zFunc,              /* Name of OS function that failed */
  ধ্রুবক অক্ষর *zPath,              /* File path associated with error */
  পূর্ণসংখ্যা i_সারি                       /* Source line number where error occurred */
){
  অক্ষর z_বার্তা[500];                 /* Human readable error text */
  পূর্ণসংখ্যা i;                          /* Loop counter */

  z_বার্তা[0] = 0;
  win_Get_Last_Error_বার্তা(lastErrno, আকার(z_বার্তা), z_বার্তা);
  প্রতিজ্ঞা( errcode!=SQLITE_OK );
  যদি( zPath==0 ) zPath = "";
  জন্য(i=0; z_বার্তা[i] && z_বার্তা[i]!='\r' && z_বার্তা[i]!='\n'; i++){}
  z_বার্তা[i] = 0;
  sqlite3_log(errcode,
      "os_win.c:%d: (%lu) %s(%s) - %s",
      i_সারি, lastErrno, zFunc, zPath, z_বার্তা
  );

  ফেরত errcode;
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
স্থির পূর্ণসংখ্যা winIoerrRetry = SQLITE_WIN32_IOERR_RETRY;
স্থির পূর্ণসংখ্যা winIoerrRetryDelay = SQLITE_WIN32_IOERR_RETRY_DELAY;

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
#যদি !defined(winIoerrCanRetry1)
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
স্থির পূর্ণসংখ্যা winRetryIoerr(পূর্ণসংখ্যা *pnRetry, DWORD *pError){
  DWORD e = osGetLastError();
  যদি( *pnRetry>=winIoerrRetry ){
    যদি( pError ){
      *pError = e;
    }
    ফেরত 0;
  }
  যদি( winIoerrCanRetry1(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    ফেরত 1;
  }
#যদি defined(winIoerrCanRetry2)
  অন্যথা যদি( winIoerrCanRetry2(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    ফেরত 1;
  }
#endif
  যদি( pError ){
    *pError = e;
  }
  ফেরত 0;
}

/*
** Log a I/O error retry episode.
*/
স্থির শূন্য winLogIoerr(পূর্ণসংখ্যা nRetry, পূর্ণসংখ্যা lineno){
  যদি( nRetry ){
    sqlite3_log(SQLITE_NOTICE,
      "delayed %dms for lock/sharing conflict at line %d",
      winIoerrRetryDelay*nRetry*(nRetry+1)/2, lineno
    );
  }
}


/*
** Lock a file region.
*/
স্থির BOOL winLockFile(
  LPHANDLE phFile,
  DWORD flags,
  DWORD offset_নিচের,
  DWORD offset_উপরের,
  DWORD অঙ্ক_Bytes_নিচের,
  DWORD অঙ্ক_Bytes_উপরের
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, আকার(OVERLAPPED));
  ovlp.Offset = offset_নিচের;
  ovlp.Offset_উপরের = offset_উপরের;
  ফেরত osLockFileEx(*phFile, flags, 0, অঙ্ক_Bytes_নিচের, অঙ্ক_Bytes_উপরের, &ovlp);
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
স্থির পূর্ণসংখ্যা winHandleLockTimeout(
  HANDLE hFile,
  DWORD offset,
  DWORD nByte,
  পূর্ণসংখ্যা bExcl,
  DWORD nMs
){
  DWORD flags = LOCKFILE_FAIL_IMMEDIATELY | (bExcl?LOCKFILE_EXCLUSIVE_LOCK:0);
  পূর্ণসংখ্যা rc = SQLITE_OK;
  BOOL ret;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, আকার(OVERLAPPED));
  ovlp.Offset = offset;

#যদি defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  যদি( nMs!=0 ){
    flags &= ~LOCKFILE_FAIL_IMMEDIATELY;
  }
  ovlp.hEvent = osCreateEvent(ফাঁকা, TRUE, FALSE, ফাঁকা);
  যদি( ovlp.hEvent==ফাঁকা ){
    ফেরত SQLITE_IOERR_LOCK;
  }
#endif
  ret = osLockFileEx(hFile, flags, 0, nByte, 0, &ovlp);
#যদি defined(SQLITE_ENABLE_SETLK_TIMEOUT)
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
  যদি( !ret && GetLastError()==ERROR_IO_PENDING ){
    DWORD nDelay = (nMs==0 ? INFINITE : nMs);
    DWORD res = osWaitForSingleObject(ovlp.hEvent, nDelay);
    যদি( res==WAIT_OBJECT_0 ){
      ret = TRUE;
    }অন্যথা যদি( res==WAIT_TIMEOUT ){
#যদি SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = SQLITE_BUSY_TIMEOUT;
#অন্যথা
      rc = SQLITE_BUSY;
#endif
    }অন্যথা{
      /* Some other error has occurred */
      rc = SQLITE_IOERR_LOCK;
    }

    /* If it is still pending, cancel the LockFileEx() call. */
    osCancelIo(hFile);
  }

  osCloseHandle(ovlp.hEvent);
#endif /* defined(SQLITE_ENABLE_SETLK_TIMEOUT) */

  যদি( rc==SQLITE_OK && !ret ){
    rc = SQLITE_BUSY;
  }
  ফেরত rc;
}
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Unlock a file region.
 */
স্থির BOOL winUnlockFile(
  LPHANDLE phFile,
  DWORD offset_নিচের,
  DWORD offset_উপরের,
  DWORD অঙ্ক_Bytes_নিচের,
  DWORD অঙ্ক_Bytes_উপরের
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, আকার(OVERLAPPED));
  ovlp.Offset = offset_নিচের;
  ovlp.Offset_উপরের = offset_উপরের;
  ফেরত osUnlockFileEx(*phFile, 0, অঙ্ক_Bytes_নিচের, অঙ্ক_Bytes_উপরের, &ovlp);
}

#ifndef SQLITE_OMIT_WAL
/*
** Remove an nByte lock starting at offset iOff from HANDLE h.
*/
স্থির পূর্ণসংখ্যা winHandleUnlock(HANDLE h, পূর্ণসংখ্যা iOff, পূর্ণসংখ্যা nByte){
  BOOL ret = winUnlockFile(&h, iOff, 0, nByte, 0);
  ফেরত (ret ? SQLITE_OK : SQLITE_IOERR_UNLOCK);
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
স্থির পূর্ণসংখ্যা winHandleSeek(HANDLE h, sqlite3_int64 iOffset){
  পূর্ণসংখ্যা rc = SQLITE_OK;             /* Return value */
  LARGE_INTEGER x;                /* The offset */

  x.QuadPart = iOffset;
  যদি( osSetFilePointerEx(h, x, 0, FILE_BEGIN)==0 ){
    rc = SQLITE_IOERR_SEEK;
  }
  OSTRACE(("SEEK file=%p, offset=%lld rc=%s\n", h, iOffset,sqlite3ErrName(rc)));
  ফেরত rc;
}

/*
** Move the current position of the file handle passed as the first
** argument to offset iOffset within the file. If successful, return 0.
** Otherwise, set pFile->lastErrno and return non-zero.
*/
স্থির পূর্ণসংখ্যা winSeekFile(winFile *pFile, sqlite3_int64 iOffset){
  পূর্ণসংখ্যা rc;

  rc = winHandleSeek(pFile->h, iOffset);
  যদি( rc!=SQLITE_OK ){
    pFile->lastErrno = osGetLastError();
    winLogError(rc, pFile->lastErrno, "winSeekFile", pFile->zPath);
  }
  ফেরত rc;
}


#যদি SQLITE_MAX_MMAP_আকার>0
/* Forward references to VFS helper methods used for memory mapped files */
স্থির পূর্ণসংখ্যা winMapfile(winFile*, sqlite3_int64);
স্থির পূর্ণসংখ্যা winUnmapfile(winFile*);
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
স্থির পূর্ণসংখ্যা winClose(sqlite3_file *id){
  পূর্ণসংখ্যা rc, cnt = 0;
  winFile *pFile = (winFile*)id;

  প্রতিজ্ঞা( id!=0 );
#ifndef SQLITE_OMIT_WAL
  প্রতিজ্ঞা( pFile->pShm==0 );
#endif
  প্রতিজ্ঞা( pFile->h!=ফাঁকা && pFile->h!=INVALID_HANDLE_মান );
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p\n",
           osGetCurrentProcessId(), pFile, pFile->h));

#যদি SQLITE_MAX_MMAP_আকার>0
  winUnmapfile(pFile);
#endif

  করো{
    rc = osCloseHandle(pFile->h);
    /* SimulateIOError( rc=0; cnt=MX_CLOSE_ATTEMPT; ); */
  }যখন( rc==0 && ++cnt < MX_CLOSE_ATTEMPT && (osSleep(100), 1) );
  যদি( rc ){
    pFile->h = ফাঁকা;
  }
  Open_গণক(-1);
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, rc ? "ok" : "failed"));
  ফেরত rc ? SQLITE_OK
            : winLogError(SQLITE_IOERR_CLOSE, osGetLastError(),
                          "winClose", pFile->zPath);
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
স্থির পূর্ণসংখ্যা winRead(
  sqlite3_file *id,          /* File to read from */
  শূন্য *pBuf,                /* Write content into this buffer */
  পূর্ণসংখ্যা amt,                   /* Number of bytes to read */
  sqlite3_int64 offset       /* Begin reading at this offset */
){
#যদি !defined(SQLITE_WIN32_NO_OVERLAPPED)
  OVERLAPPED overlapped;          /* The offset for ReadFile. */
#endif
  winFile *pFile = (winFile*)id;  /* file handle */
  DWORD nRead;                    /* Number of bytes actually read from file */
  পূর্ণসংখ্যা nRetry = 0;                 /* Number of retrys */

  প্রতিজ্ঞা( id!=0 );
  প্রতিজ্ঞা( amt>0 );
  প্রতিজ্ঞা( offset>=0 );
  SimulateIOError(ফেরত SQLITE_IOERR_READ);
  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, offset, pFile->locktype));

#যদি SQLITE_MAX_MMAP_আকার>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  যদি( offset<pFile->mmap_আকার ){
    যদি( offset+amt <= pFile->mmap_আকার ){
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], amt);
      OSTRACE(("READ-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      ফেরত SQLITE_OK;
    }অন্যথা{
      পূর্ণসংখ্যা nCopy = (পূর্ণসংখ্যা)(pFile->mmap_আকার - offset);
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#endif

#যদি defined(SQLITE_WIN32_NO_OVERLAPPED)
  যদি( winSeekFile(pFile, offset) ){
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ফেরত SQLITE_FULL;
  }
  যখন( !osReadFile(pFile->h, pBuf, amt, &nRead, 0) ){
#অন্যথা
  memset(&overlapped, 0, আকার(OVERLAPPED));
  overlapped.Offset = (LONG)(offset & 0xffffffff);
  overlapped.Offset_উপরের = (LONG)((offset>>32) & 0x7fffffff);
  যখন( !osReadFile(pFile->h, pBuf, amt, &nRead, &overlapped) &&
         osGetLastError()!=ERROR_HANDLE_EOF ){
#endif
    DWORD lastErrno;
    যদি( winRetryIoerr(&nRetry, &lastErrno) ) চালু;
    pFile->lastErrno = lastErrno;
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ফেরত winLogError(SQLITE_IOERR_READ, pFile->lastErrno,
                       "winRead", pFile->zPath);
  }
  winLogIoerr(nRetry, __LINE__);
  যদি( nRead<(DWORD)amt ){
    /* Unread parts of the buffer must be zero-filled */
    memset(&((অক্ষর*)pBuf)[nRead], 0, amt-nRead);
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_SHORT_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ফেরত SQLITE_IOERR_SHORT_READ;
  }

  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  ফেরত SQLITE_OK;
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
স্থির পূর্ণসংখ্যা winWrite(
  sqlite3_file *id,               /* File to write into */
  ধ্রুবক শূন্য *pBuf,               /* The bytes to be written */
  পূর্ণসংখ্যা amt,                        /* Number of bytes to write */
  sqlite3_int64 offset            /* Offset into the file to begin writing at */
){
  পূর্ণসংখ্যা rc = 0;                     /* True if error has occurred, else false */
  winFile *pFile = (winFile*)id;  /* File handle */
  পূর্ণসংখ্যা nRetry = 0;                 /* Number of retries */

  প্রতিজ্ঞা( amt>0 );
  প্রতিজ্ঞা( pFile );
  SimulateIOError(ফেরত SQLITE_IOERR_WRITE);
  SimulateDiskfullError(ফেরত SQLITE_FULL);

  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, offset, pFile->locktype));

#যদি defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_আকার>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  যদি( offset<pFile->mmap_আকার ){
    যদি( offset+amt <= pFile->mmap_আকার ){
      memcpy(&((u8 *)(pFile->pMapRegion))[offset], pBuf, amt);
      OSTRACE(("WRITE-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      ফেরত SQLITE_OK;
    }অন্যথা{
      পূর্ণসংখ্যা nCopy = (পূর্ণসংখ্যা)(pFile->mmap_আকার - offset);
      memcpy(&((u8 *)(pFile->pMapRegion))[offset], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#endif

#যদি defined(SQLITE_WIN32_NO_OVERLAPPED)
  rc = winSeekFile(pFile, offset);
  যদি( rc==0 ){
#অন্যথা
  {
#endif
#যদি !defined(SQLITE_WIN32_NO_OVERLAPPED)
    OVERLAPPED overlapped;        /* The offset for WriteFile. */
#endif
    u8 *aRem = (u8 *)pBuf;        /* Data yet to be written */
    পূর্ণসংখ্যা nRem = amt;               /* Number of bytes yet to be written */
    DWORD nWrite;                 /* Bytes written by each WriteFile() call */
    DWORD lastErrno = NO_ERROR;   /* Value returned by GetLastError() */

#যদি !defined(SQLITE_WIN32_NO_OVERLAPPED)
    memset(&overlapped, 0, আকার(OVERLAPPED));
    overlapped.Offset = (LONG)(offset & 0xffffffff);
    overlapped.Offset_উপরের = (LONG)((offset>>32) & 0x7fffffff);
#endif

    যখন( nRem>0 ){
#যদি defined(SQLITE_WIN32_NO_OVERLAPPED)
      যদি( !osWriteFile(pFile->h, aRem, nRem, &nWrite, 0) ){
#অন্যথা
      যদি( !osWriteFile(pFile->h, aRem, nRem, &nWrite, &overlapped) ){
#endif
        যদি( winRetryIoerr(&nRetry, &lastErrno) ) চালু;
        বিরতি;
      }
      প্রতিজ্ঞা( nWrite==0 || nWrite<=(DWORD)nRem );
      যদি( nWrite==0 || nWrite>(DWORD)nRem ){
        lastErrno = osGetLastError();
        বিরতি;
      }
#যদি !defined(SQLITE_WIN32_NO_OVERLAPPED)
      offset += nWrite;
      overlapped.Offset = (LONG)(offset & 0xffffffff);
      overlapped.Offset_উপরের = (LONG)((offset>>32) & 0x7fffffff);
#endif
      aRem += nWrite;
      nRem -= nWrite;
    }
    যদি( nRem>0 ){
      pFile->lastErrno = lastErrno;
      rc = 1;
    }
  }

  যদি( rc ){
    যদি(   ( pFile->lastErrno==ERROR_HANDLE_DISK_FULL )
       || ( pFile->lastErrno==ERROR_DISK_FULL )){
      OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      ফেরত winLogError(SQLITE_FULL, pFile->lastErrno,
                         "winWrite1", pFile->zPath);
    }
    OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_WRITE\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ফেরত winLogError(SQLITE_IOERR_WRITE, pFile->lastErrno,
                       "winWrite2", pFile->zPath);
  }অন্যথা{
    winLogIoerr(nRetry, __LINE__);
  }
  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  ফেরত SQLITE_OK;
}

#ifndef SQLITE_OMIT_WAL
/*
** Truncate the file opened by handle h to nByte bytes in size.
*/
স্থির পূর্ণসংখ্যা winHandleTruncate(HANDLE h, sqlite3_int64 nByte){ 
  পূর্ণসংখ্যা rc = SQLITE_OK;             /* Return code */
  rc = winHandleSeek(h, nByte);
  যদি( rc==SQLITE_OK ){
    যদি( 0==osSetEndOfFile(h) ){
      rc = SQLITE_IOERR_TRUNCATE;
    }
  }
  ফেরত rc;
}

/*
** Determine the size in bytes of the file opened by the handle passed as 
** the first argument.
*/
স্থির পূর্ণসংখ্যা win_Handle_আকার(HANDLE h, sqlite3_int64 *pnByte){ 
  পূর্ণসংখ্যা rc = SQLITE_OK;
  LARGE_INTEGER x;
  প্রতিজ্ঞা( pnByte );
  যদি( os_Get_File_আকার_Ex(h, &x)==0 ){
    rc = SQLITE_IOERR_FSTAT;
    *pnByte = 0;
  }অন্যথা{
    *pnByte = x.QuadPart;
  }
  ফেরত rc;
}

/*
** Close the handle passed as the only argument.
*/
স্থির শূন্য winHandleClose(HANDLE h){
  যদি( h!=INVALID_HANDLE_মান ){
    osCloseHandle(h);
  }
}
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Truncate an open file to a specified size
*/
স্থির পূর্ণসংখ্যা winTruncate(sqlite3_file *id, sqlite3_int64 nByte){
  winFile *pFile = (winFile*)id;  /* File handle object */
  পূর্ণসংখ্যা rc = SQLITE_OK;             /* Return code for this function */
  DWORD lastErrno;
#যদি SQLITE_MAX_MMAP_আকার>0
  sqlite3_int64 old_Mmap_আকার;
  যদি( pFile->nFetchOut>0 ){
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
    ফেরত SQLITE_OK;
  }
#endif

  প্রতিজ্ঞা( pFile );
  SimulateIOError(ফেরত SQLITE_IOERR_TRUNCATE);
  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, size=%lld, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, nByte, pFile->locktype));

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  যদি( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

#যদি SQLITE_MAX_MMAP_আকার>0
  যদি( pFile->pMapRegion ){
    old_Mmap_আকার = pFile->mmap_আকার;
  }অন্যথা{
    old_Mmap_আকার = 0;
  }
  winUnmapfile(pFile);
#endif

  /* SetEndOfFile() returns non-zero when successful, or zero when it fails. */
  যদি( winSeekFile(pFile, nByte) ){
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->lastErrno,
                     "winTruncate1", pFile->zPath);
  }অন্যথা যদি( 0==osSetEndOfFile(pFile->h) &&
            ((lastErrno = osGetLastError())!=ERROR_USER_MAPPED_FILE) ){
    pFile->lastErrno = lastErrno;
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->lastErrno,
                     "winTruncate2", pFile->zPath);
  }

#যদি SQLITE_MAX_MMAP_আকার>0
  যদি( rc==SQLITE_OK && old_Mmap_আকার>0 ){
    যদি( old_Mmap_আকার>nByte ){
      winMapfile(pFile, -1);
    }অন্যথা{
      winMapfile(pFile, old_Mmap_আকার);
    }
  }
#endif

  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, sqlite3ErrName(rc)));
  ফেরত rc;
}

#ifdef SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occurring at the right times.
*/
পূর্ণসংখ্যা sqlite3_sync_গুনতি = 0;
পূর্ণসংখ্যা sqlite3_fullsync_গুনতি = 0;
#endif

/*
** Make sure all writes to a particular file are committed to disk.
*/
স্থির পূর্ণসংখ্যা winSync(sqlite3_file *id, পূর্ণসংখ্যা flags){
#ifndef SQLITE_NO_SYNC
  /*
  ** Used only when SQLITE_NO_SYNC is not defined.
   */
  BOOL rc;
#endif
#যদি !defined(NDEBUG) || !defined(SQLITE_NO_SYNC) || \
    defined(SQLITE_HAVE_OS_TRACE)
  /*
  ** Used when SQLITE_NO_SYNC is not defined and by the assert() and/or
  ** OSTRACE() macros.
   */
  winFile *pFile = (winFile*)id;
#অন্যথা
  UNUSED_PARAMETER(id);
#endif

  প্রতিজ্ঞা( pFile );
  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  প্রতিজ্ঞা((flags&0x0F)==SQLITE_SYNC_NORMAL
      || (flags&0x0F)==SQLITE_SYNC_FULL
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( ফেরত SQLITE_FULL );

  OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, flags=%x, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, flags,
           pFile->locktype));

#ifndef SQLITE_TEST
  UNUSED_PARAMETER(flags);
#অন্যথা
  যদি( (flags&0x0F)==SQLITE_SYNC_FULL ){
    sqlite3_fullsync_গুনতি++;
  }
  sqlite3_sync_গুনতি++;
#endif

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op
  */
#ifdef SQLITE_NO_SYNC
  OSTRACE(("SYNC-NOP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  ফেরত SQLITE_OK;
#অন্যথা
#যদি SQLITE_MAX_MMAP_আকার>0
  যদি( pFile->pMapRegion ){
    যদি( osFlushViewOfFile(pFile->pMapRegion, 0) ){
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_OK\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
    }অন্যথা{
      pFile->lastErrno = osGetLastError();
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
      ফেরত winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winSync1", pFile->zPath);
    }
  }
#endif
  rc = osFlushFileBuffers(pFile->h);
  SimulateIOError( rc=FALSE );
  যদি( rc ){
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ফেরত SQLITE_OK;
  }অন্যথা{
    pFile->lastErrno = osGetLastError();
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_FSYNC\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ফেরত winLogError(SQLITE_IOERR_FSYNC, pFile->lastErrno,
                       "winSync2", pFile->zPath);
  }
#endif
}

/*
** Determine the current size of a file in bytes
*/
স্থির পূর্ণসংখ্যা win_File_আকার(sqlite3_file *id, sqlite3_int64 *p_আকার){
  winFile *pFile = (winFile*)id;
  পূর্ণসংখ্যা rc = SQLITE_OK;

  প্রতিজ্ঞা( id!=0 );
  প্রতিজ্ঞা( p_আকার!=0 );
  SimulateIOError(ফেরত SQLITE_IOERR_FSTAT);
  OSTRACE(("SIZE file=%p, pSize=%p\n", pFile->h, p_আকার));
  {
    LARGE_INTEGER x;
    যদি( os_Get_File_আকার_Ex(pFile->h, &x)==0 ){
      *p_আকার = 0;
      pFile->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_FSTAT, pFile->lastErrno,
                       "winFileSize", pFile->zPath);
    }অন্যথা{
      *p_আকার = x.QuadPart;
    }
  }
  OSTRACE(("SIZE file=%p, pSize=%p, *pSize=%lld, rc=%s\n",
           pFile->h, p_আকার, *p_আকার, sqlite3ErrName(rc)));
  ফেরত rc;
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
স্থির পূর্ণসংখ্যা winGetReadLock(winFile *pFile, পূর্ণসংখ্যা bBlock){
  পূর্ণসংখ্যা res;
  DWORD mask = ~(bBlock ? LOCKFILE_FAIL_IMMEDIATELY : 0);
  OSTRACE(("READ-LOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winLockFile(&pFile->h, SQLITE_LOCKFILEEX_FLAGS&mask, SHARED_FIRST, 0,
                    SHARED_আকার, 0);
  যদি( res == 0 ){
    pFile->lastErrno = osGetLastError();
    /* No need to log a failure to lock */
  }
  OSTRACE(("READ-LOCK file=%p, result=%d\n", pFile->h, res));
  ফেরত res;
}

/*
** Undo a readlock
*/
স্থির পূর্ণসংখ্যা winUnlockReadLock(winFile *pFile){
  পূর্ণসংখ্যা res;
  DWORD lastErrno;
  OSTRACE(("READ-UNLOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winUnlockFile(&pFile->h, SHARED_FIRST, 0, SHARED_আকার, 0);
  যদি( res==0 && ((lastErrno = osGetLastError())!=ERROR_NOT_LOCKED) ){
    pFile->lastErrno = lastErrno;
    winLogError(SQLITE_IOERR_UNLOCK, pFile->lastErrno,
                "winUnlockReadLock", pFile->zPath);
  }
  OSTRACE(("READ-UNLOCK file=%p, result=%d\n", pFile->h, res));
  ফেরত res;
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
স্থির পূর্ণসংখ্যা winLock(sqlite3_file *id, পূর্ণসংখ্যা locktype){
  পূর্ণসংখ্যা rc = SQLITE_OK;    /* Return code from subroutines */
  পূর্ণসংখ্যা res = 1;           /* Result of a Windows lock call */
  পূর্ণসংখ্যা newLocktype;       /* Set pFile->locktype to this value before exiting */
  পূর্ণসংখ্যা gotPendingLock = 0;/* True if we acquired a PENDING lock this time */
  winFile *pFile = (winFile*)id;
  DWORD lastErrno = NO_ERROR;

  প্রতিজ্ঞা( id!=0 );
  OSTRACE(("LOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->sharedLockByte, locktype));

  /* If there is already a lock of this type or more restrictive on the
  ** OsFile, do nothing. Don't use the end_lock: exit path, as
  ** sqlite3OsEnterMutex() hasn't been called yet.
  */
  যদি( pFile->locktype>=locktype ){
    OSTRACE(("LOCK-HELD file=%p, rc=SQLITE_OK\n", pFile->h));
    ফেরত SQLITE_OK;
  }

  /* Do not allow any kind of write-lock on a read-only database
  */
  যদি( (pFile->ctrlFlags & WINFILE_RDONLY)!=0 && locktype>=RESERVED_LOCK ){
    ফেরত SQLITE_IOERR_LOCK;
  }

  /* Make sure the locking sequence is correct
  */
  প্রতিজ্ঞা( pFile->locktype!=NO_LOCK || locktype==SHARED_LOCK );
  প্রতিজ্ঞা( locktype!=PENDING_LOCK );
  প্রতিজ্ঞা( locktype!=RESERVED_LOCK || pFile->locktype==SHARED_LOCK );

  /* Lock the PENDING_LOCK byte if we need to acquire an EXCLUSIVE lock or
  ** a SHARED lock.  If we are acquiring a SHARED lock, the acquisition of
  ** the PENDING_LOCK byte is temporary.
  */
  newLocktype = pFile->locktype;
  যদি( locktype==SHARED_LOCK
   || (locktype==EXCLUSIVE_LOCK && pFile->locktype==RESERVED_LOCK)
  ){
    পূর্ণসংখ্যা cnt = 3;

    /* Flags for the LockFileEx() call. This should be an exclusive lock if
    ** this call is to obtain EXCLUSIVE, or a shared lock if this call is to
    ** obtain SHARED.  */
    পূর্ণসংখ্যা flags = LOCKFILE_FAIL_IMMEDIATELY;
    যদি( locktype==EXCLUSIVE_LOCK ){
      flags |= LOCKFILE_EXCLUSIVE_LOCK;
    }
    যখন( cnt>0 ){
      /* Try 3 times to get the pending lock.  This is needed to work
      ** around problems caused by indexing and/or anti-virus software on
      ** Windows systems.
      **
      ** If you are using this code as a model for alternative VFSes, do not
      ** copy this retry logic.  It is a hack intended for Windows only.  */
      res = winLockFile(&pFile->h, flags, PENDING_BYTE, 0, 1, 0);
      যদি( res ) বিরতি;

      lastErrno = osGetLastError();
      OSTRACE(("LOCK-PENDING-FAIL file=%p, count=%d, result=%d\n", 
            pFile->h, cnt, res
      ));

      যদি( lastErrno==ERROR_INVALID_HANDLE ){
        pFile->lastErrno = lastErrno;
        rc = SQLITE_IOERR_LOCK;
        OSTRACE(("LOCK-FAIL file=%p, count=%d, rc=%s\n", 
              pFile->h, cnt, sqlite3ErrName(rc)
        ));
        ফেরত rc;
      }

      cnt--;
      যদি( cnt>0 ) osSleep(1);
    }
    gotPendingLock = res;
  }

  /* Acquire a shared lock
  */
  যদি( locktype==SHARED_LOCK && res ){
    প্রতিজ্ঞা( pFile->locktype==NO_LOCK );
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    res = winGetReadLock(pFile, pFile->bBlockOnConnect);
#অন্যথা
    res = winGetReadLock(pFile, 0);
#endif
    যদি( res ){
      newLocktype = SHARED_LOCK;
    }অন্যথা{
      lastErrno = osGetLastError();
    }
  }

  /* Acquire a RESERVED lock
  */
  যদি( locktype==RESERVED_LOCK && res ){
    প্রতিজ্ঞা( pFile->locktype==SHARED_LOCK );
    res = winLockFile(&pFile->h, SQLITE_LOCKFILE_FLAGS, RESERVED_BYTE, 0, 1, 0);
    যদি( res ){
      newLocktype = RESERVED_LOCK;
    }অন্যথা{
      lastErrno = osGetLastError();
    }
  }

  /* Acquire a PENDING lock
  */
  যদি( locktype==EXCLUSIVE_LOCK && res ){
    newLocktype = PENDING_LOCK;
    gotPendingLock = 0;
  }

  /* Acquire an EXCLUSIVE lock
  */
  যদি( locktype==EXCLUSIVE_LOCK && res ){
    প্রতিজ্ঞা( pFile->locktype>=SHARED_LOCK );
    (শূন্য)winUnlockReadLock(pFile);
    res = winLockFile(&pFile->h, SQLITE_LOCKFILE_FLAGS, SHARED_FIRST, 0,
                      SHARED_আকার, 0);
    যদি( res ){
      newLocktype = EXCLUSIVE_LOCK;
    }অন্যথা{
      lastErrno = osGetLastError();
      winGetReadLock(pFile, 0);
    }
  }

  /* If we are holding a PENDING lock that ought to be released, then
  ** release it now.
  */
  যদি( gotPendingLock && locktype==SHARED_LOCK ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }

  /* Update the state of the lock has held in the file descriptor then
  ** return the appropriate result code.
  */
  যদি( res ){
    rc = SQLITE_OK;
  }অন্যথা{
    pFile->lastErrno = lastErrno;
    rc = SQLITE_BUSY;
    OSTRACE(("LOCK-FAIL file=%p, wanted=%d, got=%d\n",
             pFile->h, locktype, newLocktype));
  }
  pFile->locktype = (u8)newLocktype;
  OSTRACE(("LOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  ফেরত rc;
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, return
** non-zero, otherwise zero.
*/
স্থির পূর্ণসংখ্যা winCheckReservedLock(sqlite3_file *id, পূর্ণসংখ্যা *pResOut){
  পূর্ণসংখ্যা res;
  winFile *pFile = (winFile*)id;

  SimulateIOError( ফেরত SQLITE_IOERR_CHECKRESERVEDLOCK; );
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p\n", pFile->h, pResOut));

  প্রতিজ্ঞা( id!=0 );
  যদি( pFile->locktype>=RESERVED_LOCK ){
    res = 1;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (local)\n", pFile->h, res));
  }অন্যথা{
    res = winLockFile(&pFile->h, SQLITE_LOCKFILEEX_FLAGS,RESERVED_BYTE,0,1,0);
    যদি( res ){
      winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
    }
    res = !res;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (remote)\n", pFile->h, res));
  }
  *pResOut = res;
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           pFile->h, pResOut, *pResOut));
  ফেরত SQLITE_OK;
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
স্থির পূর্ণসংখ্যা winUnlock(sqlite3_file *id, পূর্ণসংখ্যা locktype){
  পূর্ণসংখ্যা type;
  winFile *pFile = (winFile*)id;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  প্রতিজ্ঞা( pFile!=0 );
  প্রতিজ্ঞা( locktype<=SHARED_LOCK );
  OSTRACE(("UNLOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->sharedLockByte, locktype));
  type = pFile->locktype;
  যদি( type>=EXCLUSIVE_LOCK ){
    winUnlockFile(&pFile->h, SHARED_FIRST, 0, SHARED_আকার, 0);
    যদি( locktype==SHARED_LOCK && !winGetReadLock(pFile, 0) ){
      /* This should never happen.  We should always be able to
      ** reacquire the read lock */
      rc = winLogError(SQLITE_IOERR_UNLOCK, osGetLastError(),
                       "winUnlock", pFile->zPath);
    }
  }
  যদি( type>=RESERVED_LOCK ){
    winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
  }
  যদি( locktype==NO_LOCK && type>=SHARED_LOCK ){
    winUnlockReadLock(pFile);
  }
  যদি( type>=PENDING_LOCK ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }
  pFile->locktype = (u8)locktype;
  OSTRACE(("UNLOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  ফেরত rc;
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

স্থির পূর্ণসংখ্যা winNolockLock(sqlite3_file *id, পূর্ণসংখ্যা locktype){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(locktype);
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা winNolockCheckReservedLock(sqlite3_file *id, পূর্ণসংখ্যা *pResOut){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(pResOut);
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা winNolockUnlock(sqlite3_file *id, পূর্ণসংখ্যা locktype){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(locktype);
  ফেরত SQLITE_OK;
}

/******************* End of the no-op lock implementation *********************
******************************************************************************/

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
স্থির শূন্য winModeBit(winFile *pFile, unsigned অক্ষর mask, পূর্ণসংখ্যা *pArg){
  যদি( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & mask)!=0;
  }অন্যথা যদি( (*pArg)==0 ){
    pFile->ctrlFlags &= ~mask;
  }অন্যথা{
    pFile->ctrlFlags |= mask;
  }
}

/* Forward references to VFS helper methods used for temporary files */
স্থির পূর্ণসংখ্যা winGetTempname(sqlite3_vfs *, অক্ষর **);
স্থির পূর্ণসংখ্যা winIsDir(LPCWSTR);
স্থির BOOL winIsLongPathPrefix(ধ্রুবক অক্ষর *);
স্থির BOOL winIsDriveLetterAndColon(ধ্রুবক অক্ষর *);

/*
** Control and query of the open file handle.
*/
স্থির পূর্ণসংখ্যা winFileControl(sqlite3_file *id, পূর্ণসংখ্যা op, শূন্য *pArg){
  winFile *pFile = (winFile*)id;
  OSTRACE(("FCNTL file=%p, op=%d, pArg=%p\n", pFile->h, op, pArg));
  নির্বাচন( op ){
    ক্ষেত্র SQLITE_FCNTL_LOCKSTATE: {
      *(পূর্ণসংখ্যা*)pArg = pFile->locktype;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ফেরত SQLITE_OK;
    }
    ক্ষেত্র SQLITE_FCNTL_LAST_ERRNO: {
      *(পূর্ণসংখ্যা*)pArg = (পূর্ণসংখ্যা)pFile->lastErrno;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ফেরত SQLITE_OK;
    }
    ক্ষেত্র SQLITE_FCNTL_CHUNK_আকার: {
      pFile->szChunk = *(পূর্ণসংখ্যা *)pArg;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ফেরত SQLITE_OK;
    }
    ক্ষেত্র SQLITE_FCNTL_আকার_HINT: {
      যদি( pFile->szChunk>0 ){
        sqlite3_int64 oldSz;
        পূর্ণসংখ্যা rc = win_File_আকার(id, &oldSz);
        যদি( rc==SQLITE_OK ){
          sqlite3_int64 newSz = *(sqlite3_int64*)pArg;
          যদি( newSz>oldSz ){
            SimulateIOErrorBenign(1);
            rc = winTruncate(id, newSz);
            SimulateIOErrorBenign(0);
          }
        }
        OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
        ফেরত rc;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ফেরত SQLITE_OK;
    }
    ক্ষেত্র SQLITE_FCNTL_PERSIST_WAL: {
      winModeBit(pFile, WINFILE_PERSIST_WAL, (পূর্ণসংখ্যা*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ফেরত SQLITE_OK;
    }
    ক্ষেত্র SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      winModeBit(pFile, WINFILE_PSOW, (পূর্ণসংখ্যা*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ফেরত SQLITE_OK;
    }
    ক্ষেত্র SQLITE_FCNTL_VFSNAME: {
      *(অক্ষর**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ফেরত SQLITE_OK;
    }
    ক্ষেত্র SQLITE_FCNTL_WIN32_AV_RETRY: {
      পূর্ণসংখ্যা *a = (পূর্ণসংখ্যা*)pArg;
      যদি( a[0]>0 ){
        winIoerrRetry = a[0];
      }অন্যথা{
        a[0] = winIoerrRetry;
      }
      যদি( a[1]>0 ){
        winIoerrRetryDelay = a[1];
      }অন্যথা{
        a[1] = winIoerrRetryDelay;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ফেরত SQLITE_OK;
    }
    ক্ষেত্র SQLITE_FCNTL_WIN32_GET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      *phFile = pFile->h;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ফেরত SQLITE_OK;
    }
#ifdef SQLITE_TEST
    ক্ষেত্র SQLITE_FCNTL_WIN32_SET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      HANDLE hOldFile = pFile->h;
      pFile->h = *phFile;
      *phFile = hOldFile;
      OSTRACE(("FCNTL oldFile=%p, newFile=%p, rc=SQLITE_OK\n",
               hOldFile, pFile->h));
      ফেরত SQLITE_OK;
    }
#endif
    ক্ষেত্র SQLITE_FCNTL_NULL_IO: {
      (শূন্য)osCloseHandle(pFile->h);
      pFile->h = ফাঁকা;
      ফেরত SQLITE_OK;
    }
    ক্ষেত্র SQLITE_FCNTL_TEMPFILENAME: {
      অক্ষর *zTFile = 0;
      পূর্ণসংখ্যা rc = winGetTempname(pFile->pVfs, &zTFile);
      যদি( rc==SQLITE_OK ){
        *(অক্ষর**)pArg = zTFile;
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      ফেরত rc;
    }
#যদি SQLITE_MAX_MMAP_আকার>0
    ক্ষেত্র SQLITE_FCNTL_MMAP_আকার: {
      i64 newLimit = *(i64*)pArg;
      পূর্ণসংখ্যা rc = SQLITE_OK;
      যদি( newLimit>sqlite3GlobalConfig.mxMmap ){
        newLimit = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (SIZE_T) and passed
      ** to MapViewOfFile(). Restrict its value to 2GB if (SIZE_T) is not at
      ** least a 64-bit type. */
      যদি( newLimit>0 && আকার(আকার_T)<8 ){
        newLimit = (newLimit & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmap_আকার_Max;
      যদি( newLimit>=0 && newLimit!=pFile->mmap_আকার_Max && pFile->nFetchOut==0 ){
        pFile->mmap_আকার_Max = newLimit;
        যদি( pFile->mmap_আকার>0 ){
          winUnmapfile(pFile);
          rc = winMapfile(pFile, -1);
        }
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      ফেরত rc;
    }
#endif

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    ক্ষেত্র SQLITE_FCNTL_LOCK_TIMEOUT: {
      পূর্ণসংখ্যা iOld = pFile->iBusyTimeout;
      পূর্ণসংখ্যা iNew = *(পূর্ণসংখ্যা*)pArg;
#যদি SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = (iNew < 0) ? INFINITE : (DWORD)iNew;
#নতুবা_যদি SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = (DWORD)(!!iNew);
#অন্যথা
# error "SQLITE_ENABLE_SETLK_TIMEOUT must be set to 1 or 2"
#endif
      *(পূর্ণসংখ্যা*)pArg = iOld;
      ফেরত SQLITE_OK;
    }
    ক্ষেত্র SQLITE_FCNTL_BLOCK_ON_CONNECT: {
      পূর্ণসংখ্যা iNew = *(পূর্ণসংখ্যা*)pArg;
      pFile->bBlockOnConnect = iNew;
      ফেরত SQLITE_OK;
    }
#endif /* SQLITE_ENABLE_SETLK_TIMEOUT */

#যদি defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    ক্ষেত্র SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      sqlite3_str_appendf(pStr, "{\"h\":%llu", (sqlite3_uint64)pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      যদি( pFile->locktype ){
        স্থির ধ্রুবক অক্ষর *azLock[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
        sqlite3_str_appendf(pStr, ",\"locktype\":\"%s\"", 
                                  azLock[pFile->locktype-1]);
      }
#যদি SQLITE_MAX_MMAP_আকার>0
      যদি( pFile->mmap_আকার ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmap_আকার);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->nFetchOut);
      }
#endif
      sqlite3_str_append(pStr, "}", 1);
      ফেরত SQLITE_OK;
    }
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

  }
  OSTRACE(("FCNTL file=%p, rc=SQLITE_NOTFOUND\n", pFile->h));
  ফেরত SQLITE_NOTFOUND;
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
স্থির পূর্ণসংখ্যা win_Sector_আকার(sqlite3_file *id){
  (শূন্য)id;
  ফেরত SQLITE_DEFAULT_SECTOR_আকার;
}

/*
** Return a vector of device characteristics.
*/
স্থির পূর্ণসংখ্যা winDeviceCharacteristics(sqlite3_file *id){
  winFile *p = (winFile*)id;
  ফেরত SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN | SQLITE_IOCAP_SUBPAGE_READ |
         ((p->ctrlFlags & WINFILE_PSOW)?SQLITE_IOCAP_POWERSAFE_OVERWRITE:0);
}

/*
** Windows will only let you create file view mappings
** on allocation size granularity boundaries.
** During sqlite3_os_init() we do a GetSystemInfo()
** to get the granularity size.
*/
স্থির SYSTEM_INFO winSysInfo;

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
স্থির LPWSTR winConvertFromUtf8Filename(ধ্রুবক অক্ষর *zFilename){
  LPWSTR zConverted = 0;
#ifdef __CYGWIN__
  পূর্ণসংখ্যা nChar;

  যদি( osCygwin_conv_path && !(winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2])) ){
    i64 nByte;
    পূর্ণসংখ্যা convertflag = CCP_POSIX_TO_WIN_W;
    যদি( !strchr(zFilename, '/') ) convertflag |= CCP_RELATIVE;
    nByte = (i64)osCygwin_conv_path(convertflag,
        zFilename, 0, 0);
    যদি( nByte>0 ){
      zConverted = sqlite3MallocZero(12+(u64)nByte);
      যদি ( zConverted==0 ){
        ফেরত zConverted;
      }
      /* Filenames should be prefixed, except when converted
       * full path already starts with "\\?\". */
      যদি( osCygwin_conv_path(convertflag, zFilename,
                           zConverted+4, nByte)==0 ){
        যদি( (convertflag&CCP_RELATIVE) ){
          memmove(zConverted, zConverted+4, nByte);
        }অন্যথা যদি( memcmp(zConverted+4, L"\\\\", 4) ){
          memcpy(zConverted, L"\\\\?\\", 8);
        }অন্যথা যদি( zConverted[6]!='?' ){
          memmove(zConverted+6, zConverted+4, nByte);
          memcpy(zConverted, L"\\\\?\\UNC", 14);
        }অন্যথা{
          memmove(zConverted, zConverted+4, nByte);
        }
        ফেরত zConverted;
      }
      sqlite3_free(zConverted);
    }
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zFilename, -1, ফাঁকা, 0);
  যদি( nChar==0 ){
    ফেরত 0;
  }
  zConverted = sqlite3MallocZero( ((u64)nChar+6)*আকার(WCHAR) );
  যদি( zConverted==0 ){
    ফেরত 0;
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zFilename, -1,
                                zConverted, nChar);
  যদি( nChar==0 ){
    sqlite3_free(zConverted);
    zConverted = 0;
  }অন্যথা যদি( nChar>MAX_PATH
      && winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2]) ){
    memmove(zConverted+4, zConverted, nChar*আকার(WCHAR));
    zConverted[2] = '\\';
    memcpy(zConverted, L"\\\\?\\", 8);
  }অন্যথা যদি( nChar>MAX_PATH
      && winIsDirSep(zFilename[0]) && winIsDirSep(zFilename[1])
      && zFilename[2] != '?' ){
    memmove(zConverted+6, zConverted, nChar*আকার(WCHAR));
    memcpy(zConverted, L"\\\\?\\UNC", 14);
  }
#অন্যথা /* if !defined(__CYGWIN__) */
  zConverted = winUtf8ToUnicode(zFilename);
#endif /* __CYGWIN__ */
  /* caller will handle out of memory */
  ফেরত zConverted;
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
স্থির sqlite3_mutex *winBigLock = 0;
স্থির শূন্য winShmEnterMutex(শূন্য){
  sqlite3_mutex_enter(winBigLock);
}
স্থির শূন্য winShmLeaveMutex(শূন্য){
  sqlite3_mutex_leave(winBigLock);
}
#ifndef NDEBUG
স্থির পূর্ণসংখ্যা winShmMutexHeld(শূন্য) {
  ফেরত sqlite3_mutex_held(winBigLock);
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
কাঠামো win_Shm_গ্রন্থি {
  sqlite3_mutex *mutex;      /* Mutex to access this object */
  অক্ষর *zFilename;           /* Name of the file */
  HANDLE hSharedShm;         /* File handle open on zFilename */
  পূর্ণসংখ্যা bUseSharedLockHandle;  /* True to use hSharedShm for everything */

  পূর্ণসংখ্যা isUnlocked;            /* DMS lock has not yet been obtained */
  পূর্ণসংখ্যা isReadonly;            /* True if read-only */
  পূর্ণসংখ্যা szRegion;              /* Size of shared-memory regions */
  পূর্ণসংখ্যা nRegion;               /* Size of array apRegion */

  কাঠামো ShmRegion {
    HANDLE hMap;             /* File handle from CreateFileMapping */
    শূন্য *pMap;
  } *aRegion;
  DWORD lastErrno;           /* The Windows errno from the last I/O error */

  winShm *pWinShmList;       /* List of winShm objects with ptrs to this */

  win_Shm_গ্রন্থি *p_পরবর্তী;         /* Next in list of all winShmNode objects */
#যদি defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 পরবর্তী_Shm_Id;              /* Next available winShm.id value */
#endif
};

/*
** A global array of all winShmNode objects.
**
** The winShmMutexHeld() must be true while reading or writing this list.
*/
স্থির win_Shm_গ্রন্থি *win_Shm_গ্রন্থি_List = 0;

/*
** Structure used internally by this VFS to record the state of an
** open shared memory connection. There is one such structure for each
** winFile open on a wal mode database.
*/
কাঠামো winShm {
  win_Shm_গ্রন্থি *p_Shm_গ্রন্থি;      /* The underlying winShmNode object */
  u16 sharedMask;            /* Mask of shared locks held */
  u16 exclMask;              /* Mask of exclusive locks held */
  HANDLE hShm;               /* File-handle on *-shm file. For locking. */
  পূর্ণসংখ্যা bReadonly;             /* True if hShm is opened read-only */
#যদি defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 id;                     /* Id of this connection with its winShmNode */
#endif
  winShm *p_Win_Shm_পরবর্তী;       /* Next winShm object on same winShmNode */
};

/*
** Constants used for locking
*/
#define WIN_SHM_BASE   ((22+SQLITE_SHM_NLOCK)*4)        /* first lock byte */
#define WIN_SHM_DMS    (WIN_SHM_BASE+SQLITE_SHM_NLOCK)  /* deadman switch */

/* Forward references to VFS methods */
স্থির পূর্ণসংখ্যা winOpen(sqlite3_vfs*,ধ্রুবক অক্ষর*,sqlite3_file*,পূর্ণসংখ্যা,পূর্ণসংখ্যা*);
স্থির পূর্ণসংখ্যা winDelete(sqlite3_vfs *,ধ্রুবক অক্ষর*,পূর্ণসংখ্যা);

/*
** Purge the winShmNodeList list of all entries with winShmNode.pWinShmList==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
স্থির শূন্য winShmPurge(sqlite3_vfs *pVfs, পূর্ণসংখ্যা deleteFlag){
  win_Shm_গ্রন্থি **pp;
  win_Shm_গ্রন্থি *p;
  প্রতিজ্ঞা( winShmMutexHeld() );
  OSTRACE(("SHM-PURGE pid=%lu, deleteFlag=%d\n",
           osGetCurrentProcessId(), deleteFlag));
  pp = &win_Shm_গ্রন্থি_List;
  যখন( (p = *pp)!=0 ){
    যদি( p->pWinShmList==0 ){
      পূর্ণসংখ্যা i;
      যদি( p->mutex ){ sqlite3_mutex_free(p->mutex); }
      জন্য(i=0; i<p->nRegion; i++){
        BOOL bRc = osUnmapViewOfFile(p->aRegion[i].pMap);
        OSTRACE(("SHM-PURGE-UNMAP pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        UNUSED_VARIABLE_মান(bRc);
        bRc = osCloseHandle(p->aRegion[i].hMap);
        OSTRACE(("SHM-PURGE-CLOSE pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        UNUSED_VARIABLE_মান(bRc);
      }
      winHandleClose(p->hSharedShm);
      যদি( deleteFlag ){
        SimulateIOErrorBenign(1);
        sqlite3BeginBenignMalloc();
        winDelete(pVfs, p->zFilename, 0);
        sqlite3EndBenignMalloc();
        SimulateIOErrorBenign(0);
      }
      *pp = p->p_পরবর্তী;
      sqlite3_free(p->aRegion);
      sqlite3_free(p);
    }অন্যথা{
      pp = &p->p_পরবর্তী;
    }
  }
}

/*
** Handle h is open on the *-shm file. This function zeroes the first
** 8 bytes of the file. If SQLITE_ENABLE_SETLK_TIMEOUT is defined, h may 
** have been opened with FILE_FLAG_OVERLAPPED.
*/
স্থির পূর্ণসংখ্যা winZeroSharedMemory(HANDLE h){
  u8 aZero[8] = {0,0,0,0,0,0,0,0};
  DWORD nWrite = 0;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, আকার(OVERLAPPED));
  যদি( !osWriteFile(h, aZero, আকার(aZero), &nWrite, &ovlp) ){
    /* If the file was opened with FILE_FLAG_OVERLAPPED, the write may still
    ** be pending. Wait for it with GetOverlappedResult().  */
    যদি( osGetLastError()!=ERROR_IO_PENDING
     || !Get_Overlapped_ফলাফল(h, &ovlp, &nWrite, TRUE)
    ){
      nWrite = 0;
    }
  }
  ফেরত (nWrite==আকার(aZero) ? SQLITE_OK : SQLITE_IOERR_WRITE);
}

/*
** The DMS lock has not yet been taken on the shm file associated with
** pShmNode. Take the lock. Truncate the *-shm file if required.
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
স্থির পূর্ণসংখ্যা winLockSharedMemory(win_Shm_গ্রন্থি *p_Shm_গ্রন্থি, DWORD nMs){
  HANDLE h = p_Shm_গ্রন্থি->hSharedShm;
  পূর্ণসংখ্যা rc = SQLITE_OK;

  প্রতিজ্ঞা( sqlite3_mutex_held(p_Shm_গ্রন্থি->mutex) );
  rc = winHandleLockTimeout(h, WIN_SHM_DMS, 1, 1, 0);
  যদি( rc==SQLITE_OK ){
    /* We have an EXCLUSIVE lock on the DMS byte. This means that this
    ** is the first process to open the file. Truncate it to zero bytes
    ** in this case.  */
    যদি( p_Shm_গ্রন্থি->isReadonly ){
      rc = SQLITE_READONLY_CANTINIT;
    }অন্যথা{
      যদি( SQLITE_OK!=winHandleTruncate(h, 0) ){
        /* Sometimes the truncate operation may fail because some other
        ** process is still holding an open mapping. So try to write
        ** zeroes into the start of the file instead. */
        rc = winZeroSharedMemory(h);
      }
    }

    /* Release the EXCLUSIVE lock acquired above. */
    winUnlockFile(&h, WIN_SHM_DMS, 0, 1, 0);
  }অন্যথা যদি( (rc & 0xFF)==SQLITE_BUSY ){
    rc = SQLITE_OK;
  }

  যদি( rc==SQLITE_OK ){
    /* Take a SHARED lock on the DMS byte. */
    rc = winHandleLockTimeout(h, WIN_SHM_DMS, 1, 0, nMs);
    যদি( rc==SQLITE_OK ){
      p_Shm_গ্রন্থি->isUnlocked = 0;
    }
  }

  ফেরত rc;
}


/*
** This function is used to open a handle on a *-shm file.
**
** If SQLITE_ENABLE_SETLK_TIMEOUT is defined at build time, then the file
** is opened with FILE_FLAG_OVERLAPPED specified. If not, it is not.
*/
স্থির পূর্ণসংখ্যা winHandleOpen(
  ধ্রুবক অক্ষর *zUtf8,              /* File to open */
  পূর্ণসংখ্যা *pbReadonly,                /* IN/OUT: True for readonly handle */
  HANDLE *ph                      /* OUT: New HANDLE for file */
){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  LPWSTR zConverted = 0;
  পূর্ণসংখ্যা bReadonly = *pbReadonly;
  HANDLE h = INVALID_HANDLE_মান;

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  ধ্রুবক DWORD flag_overlapped = FILE_FLAG_OVERLAPPED;
#অন্যথা
  ধ্রুবক DWORD flag_overlapped = 0;
#endif

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8);
  যদি( zConverted==0 ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8));
    rc = SQLITE_IOERR_NOMEM_BKPT;
    যাও winopenfile_out;
  }

  /* Ensure the file we are trying to open is not actually a directory. */
  যদি( winIsDir(zConverted) ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8));
    rc = SQLITE_CANTOPEN_ISDIR;
    যাও winopenfile_out;
  }

  /* TODO: platforms.
  ** TODO: retry-on-ioerr.
  */
  h = osCreateFileW(zConverted,         /* lpFileName */
      (GENERIC_READ | (bReadonly ? 0 : GENERIC_WRITE)),  /* dwDesiredAccess */
      FILE_SHARE_READ | FILE_SHARE_WRITE,        /* dwShareMode */
      ফাঁকা,                                      /* lpSecurityAttributes */
      OPEN_ALWAYS,                               /* dwCreationDisposition */
      FILE_ATTRIBUTE_NORMAL|flag_overlapped,
      ফাঁকা
  );

  যদি( h==INVALID_HANDLE_মান ){
    যদি( bReadonly==0 ){
      bReadonly = 1;
      rc = winHandleOpen(zUtf8, &bReadonly, &h);
    }অন্যথা{
      rc = SQLITE_CANTOPEN_BKPT;
    }
  }

 winopenfile_out:
  sqlite3_free(zConverted);
  *pbReadonly = bReadonly;
  *ph = h;
  ফেরত rc;
}
 
/*
** Close pDbFd's connection to shared-memory.  Delete the underlying
** *-shm file if deleteFlag is true.
*/
স্থির পূর্ণসংখ্যা winCloseSharedMemory(winFile *pDbFd, পূর্ণসংখ্যা deleteFlag){
  winShm *p;            /* The connection to be closed */
  winShm **pp;          /* Iterator for pShmNode->pWinShmList */
  win_Shm_গ্রন্থি *p_Shm_গ্রন্থি; /* The underlying shared-memory file */

  p = pDbFd->pShm;
  যদি( p==0 ) ফেরত SQLITE_OK;
  যদি( p->hShm!=INVALID_HANDLE_মান ){
    osCloseHandle(p->hShm);
  }

  winShmEnterMutex();
  p_Shm_গ্রন্থি = p->p_Shm_গ্রন্থি;

  /* Remove this connection from the winShmNode.pWinShmList list */
  sqlite3_mutex_enter(p_Shm_গ্রন্থি->mutex);
  জন্য(pp=&p_Shm_গ্রন্থি->pWinShmList; *pp!=p; pp=&(*pp)->p_Win_Shm_পরবর্তী){}
  *pp = p->p_Win_Shm_পরবর্তী;
  sqlite3_mutex_leave(p_Shm_গ্রন্থি->mutex);

  winShmPurge(pDbFd->pVfs, deleteFlag);
  winShmLeaveMutex();

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  ফেরত SQLITE_OK;
}

/*
** testfixture builds may set this global variable to true via a
** Tcl interface. This forces the VFS to use the locking normally
** only used for UNC paths for all files.
*/
#ifdef SQLITE_TEST
পূর্ণসংখ্যা sqlite3_win_test_unc_locking = 0;
#অন্যথা
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
স্থির পূর্ণসংখ্যা winIsUNCPath(ধ্রুবক অক্ষর *zFile){
  যদি( zFile[0]=='\\' && zFile[1]=='\\' ){
    যদি( zFile[2]=='?'
     && zFile[3]=='\\'
     && sqlite3Isalpha(zFile[4])
     && zFile[5]==':'
     && winIsDirSep(zFile[6])
    ){
      ফেরত sqlite3_win_test_unc_locking;
    }অন্যথা{
      ফেরত 1;
    }
  }
  ফেরত sqlite3_win_test_unc_locking;
}

/*
** Return true if the string passed in is the name of a file on a
** remote (network-mounted) volume.
*/
স্থির পূর্ণসংখ্যা winIsRemoteVolume(ধ্রুবক অক্ষর *zFile){
  WCHAR z_মূল[4];
  যদি( strncmp(zFile, "\\\\?\\",4)==0 ) zFile += 4;
  যদি( !sqlite3Isalpha(zFile[0]) ) ফেরত 0;
  যদি( zFile[1]!=':' ) ফেরত 0;
  যদি( !winIsDirSep(zFile[2]) ) ফেরত 0;
  z_মূল[0] = (WCHAR)zFile[0];
  z_মূল[1] = ':';
  z_মূল[2] = '\\';
  z_মূল[3] = 0;
  ফেরত osGetDriveTypeW(z_মূল)==DRIVE_REMOTE;
}

/*
** Open the shared-memory area associated with database file pDbFd.
*/
স্থির পূর্ণসংখ্যা winOpenSharedMemory(winFile *pDbFd){
  কাঠামো winShm *p;                  /* The connection to be opened */
  win_Shm_গ্রন্থি *p_Shm_গ্রন্থি = 0;          /* The underlying mmapped file */
  পূর্ণসংখ্যা rc = SQLITE_OK;                /* Result code */
  win_Shm_গ্রন্থি *pNew;                  /* Newly allocated winShmNode */
  পূর্ণসংখ্যা nName;                         /* Size of zName in bytes */

  প্রতিজ্ঞা( pDbFd->pShm==0 );    /* Not previously opened */

  /* Allocate space for the new sqlite3_shm object.  Also speculatively
  ** allocate space for a new winShmNode and filename.  */
  p = sqlite3MallocZero( আকার(*p) );
  যদি( p==0 ) ফেরত SQLITE_IOERR_NOMEM_BKPT;
  nName = sqlite3Strlen30(pDbFd->zPath);
  pNew = sqlite3MallocZero( আকার(*p_Shm_গ্রন্থি) + (i64)nName + 17 );
  যদি( pNew==0 ){
    sqlite3_free(p);
    ফেরত SQLITE_IOERR_NOMEM_BKPT;
  }
  pNew->zFilename = (অক্ষর*)&pNew[1];
  pNew->hSharedShm = INVALID_HANDLE_মান;
  pNew->isUnlocked = 1;
  যদি( winIsUNCPath(pDbFd->zPath) || winIsRemoteVolume(pDbFd->zPath) ){
    pNew->bUseSharedLockHandle = 1;
  }অন্যথা{
    pNew->bUseSharedLockHandle = 0;
  }
  sqlite3_snprintf(nName+15, pNew->zFilename, "%s-shm", pDbFd->zPath);
  sqlite3FileSuffix3(pDbFd->zPath, pNew->zFilename);

  /* Look to see if there is an existing winShmNode that can be used.
  ** If no matching winShmNode currently exists, then create a new one.  */
  winShmEnterMutex();
  জন্য(p_Shm_গ্রন্থি = win_Shm_গ্রন্থি_List; p_Shm_গ্রন্থি; p_Shm_গ্রন্থি=p_Shm_গ্রন্থি->p_পরবর্তী){
    /* TBD need to come up with better match here.  Perhaps
    ** use FILE_ID_BOTH_DIR_INFO Structure.  */
    যদি( sqlite3StrICmp(p_Shm_গ্রন্থি->zFilename, pNew->zFilename)==0 ) বিরতি;
  }
  যদি( p_Shm_গ্রন্থি==0 ){
    p_Shm_গ্রন্থি = pNew;

    /* Allocate a mutex for this winShmNode object, if one is required. */
    যদি( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_গ্রন্থি->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      যদি( p_Shm_গ্রন্থি->mutex==0 ) rc = SQLITE_IOERR_NOMEM_BKPT;
    }

    /* Open a file-handle to use for mappings, and for the DMS lock. */
    যদি( rc==SQLITE_OK ){
      HANDLE h = INVALID_HANDLE_মান;
      p_Shm_গ্রন্থি->isReadonly = sqlite3_uri_boolean(pDbFd->zPath,"readonly_shm",0);
      rc = winHandleOpen(pNew->zFilename, &p_Shm_গ্রন্থি->isReadonly, &h);
      p_Shm_গ্রন্থি->hSharedShm = h;
    }

    /* If successful, link the new winShmNode into the global list. If an 
    ** error occurred, free the object. */
    যদি( rc==SQLITE_OK ){
      p_Shm_গ্রন্থি->p_পরবর্তী = win_Shm_গ্রন্থি_List;
      win_Shm_গ্রন্থি_List = p_Shm_গ্রন্থি;
      pNew = 0;
    }অন্যথা{
      sqlite3_mutex_free(p_Shm_গ্রন্থি->mutex);
      যদি( p_Shm_গ্রন্থি->hSharedShm!=INVALID_HANDLE_মান ){
        osCloseHandle(p_Shm_গ্রন্থি->hSharedShm);
      }
    }
  }

  /* If no error has occurred, link the winShm object to the winShmNode and
  ** the winShm to pDbFd.  */
  যদি( rc==SQLITE_OK ){
    sqlite3_mutex_enter(p_Shm_গ্রন্থি->mutex);
    p->p_Shm_গ্রন্থি = p_Shm_গ্রন্থি;
    p->p_Win_Shm_পরবর্তী = p_Shm_গ্রন্থি->pWinShmList;
    p_Shm_গ্রন্থি->pWinShmList = p;
#যদি defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
    p->id = p_Shm_গ্রন্থি->পরবর্তী_Shm_Id++;
#endif
    pDbFd->pShm = p;
    sqlite3_mutex_leave(p_Shm_গ্রন্থি->mutex);
  }অন্যথা যদি( p ){
    sqlite3_free(p);
  }

  প্রতিজ্ঞা( rc!=SQLITE_OK || p_Shm_গ্রন্থি->isUnlocked==0 || p_Shm_গ্রন্থি->nRegion==0 );
  winShmLeaveMutex();
  sqlite3_free(pNew);

  /* Open a file-handle on the *-shm file for this connection. This file-handle
  ** is only used for locking. The mapping of the *-shm file is created using
  ** the shared file handle in winShmNode.hSharedShm.  */
  যদি( rc==SQLITE_OK && p_Shm_গ্রন্থি->bUseSharedLockHandle==0 ){
    p->bReadonly = sqlite3_uri_boolean(pDbFd->zPath, "readonly_shm", 0);
    rc = winHandleOpen(p_Shm_গ্রন্থি->zFilename, &p->bReadonly, &p->hShm);
    যদি( rc!=SQLITE_OK ){
      প্রতিজ্ঞা( p->hShm==INVALID_HANDLE_মান );
      winCloseSharedMemory(pDbFd, 0);
    }
  }

  ফেরত rc;
}

/*
** Close a connection to shared-memory.  Delete the underlying
** storage if deleteFlag is true.
*/
স্থির পূর্ণসংখ্যা winShmUnmap(
  sqlite3_file *fd,          /* Database holding shared memory */
  পূর্ণসংখ্যা deleteFlag             /* Delete after closing if true */
){
  ফেরত winCloseSharedMemory((winFile*)fd, deleteFlag);
}

/*
** Change the lock state for a shared-memory segment.
*/
স্থির পূর্ণসংখ্যা winShmLock(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  পূর্ণসংখ্যা ofst,                  /* First lock to acquire or release */
  পূর্ণসংখ্যা n,                     /* Number of locks to acquire or release */
  পূর্ণসংখ্যা flags                  /* What to do with the lock */
){
  winFile *pDbFd = (winFile*)fd;        /* Connection holding shared memory */
  winShm *p = pDbFd->pShm;              /* The shared memory being locked */
  win_Shm_গ্রন্থি *p_Shm_গ্রন্থি;
  পূর্ণসংখ্যা rc = SQLITE_OK;                   /* Result code */
  u16 mask = (u16)((1U<<(ofst+n)) - (1U<<ofst)); /* Mask of locks to [un]take */

  যদি( p==0 ) ফেরত SQLITE_IOERR_SHMLOCK;
  p_Shm_গ্রন্থি = p->p_Shm_গ্রন্থি;
  যদি( NEVER(p_Shm_গ্রন্থি==0) ) ফেরত SQLITE_IOERR_SHMLOCK;

  প্রতিজ্ঞা( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  প্রতিজ্ঞা( n>=1 );
  প্রতিজ্ঞা( flags==(SQLITE_SHM_LOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_LOCK | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  প্রতিজ্ঞা( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );

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
#যদি defined(SQLITE_ENABLE_SETLK_TIMEOUT) && defined(SQLITE_DEBUG)
  {
    u16 lockMask = (p->exclMask|p->sharedMask);
    প্রতিজ্ঞা( (flags & SQLITE_SHM_UNLOCK) || pDbFd->iBusyTimeout==0 || (
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
  প্রতিজ্ঞা( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK) 
       || 0==(p->exclMask & mask)
  );
  যদি( ((flags & SQLITE_SHM_UNLOCK) && ((p->exclMask|p->sharedMask) & mask))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_LOCK) && 0==(p->sharedMask & mask))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK))
  ){
    HANDLE h = p->hShm;

    যদি( flags & SQLITE_SHM_UNLOCK ){
      /* Case (a) - unlock.  */

      প্রতিজ্ঞা( (p->exclMask & p->sharedMask)==0 );
      প্রতিজ্ঞা( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->exclMask & mask)==mask );
      প্রতিজ্ঞা( !(flags & SQLITE_SHM_SHARED) || (p->sharedMask & mask)==mask );

      প্রতিজ্ঞা( !(flags & SQLITE_SHM_SHARED) || n==1 );
      যদি( p_Shm_গ্রন্থি->bUseSharedLockHandle ){
        h = p_Shm_গ্রন্থি->hSharedShm;
        যদি( flags & SQLITE_SHM_SHARED ){
          winShm *pShm;
          sqlite3_mutex_enter(p_Shm_গ্রন্থি->mutex);
          জন্য(pShm=p_Shm_গ্রন্থি->pWinShmList; pShm; pShm=pShm->p_Win_Shm_পরবর্তী){
            যদি( pShm!=p && (pShm->sharedMask & mask) ){
              /* Another connection within this process is also holding this
              ** SHARED lock. So do not actually release the OS lock.  */
              h = INVALID_HANDLE_মান;
              বিরতি;
            }
          }
          sqlite3_mutex_leave(p_Shm_গ্রন্থি->mutex);
        }
      }

      যদি( h!=INVALID_HANDLE_মান ){
        rc = winHandleUnlock(h, ofst+WIN_SHM_BASE, n);
      }

      /* If successful, also clear the bits in sharedMask/exclMask */
      যদি( rc==SQLITE_OK ){
        p->exclMask = (p->exclMask & ~mask);
        p->sharedMask = (p->sharedMask & ~mask);
      }
    }অন্যথা{
      পূর্ণসংখ্যা bExcl = ((flags & SQLITE_SHM_EXCLUSIVE) ? 1 : 0);
      DWORD nMs = winFileBusyTimeout(pDbFd);

      যদি( p_Shm_গ্রন্থি->bUseSharedLockHandle ){
        winShm *pShm;
        h = p_Shm_গ্রন্থি->hSharedShm;
        sqlite3_mutex_enter(p_Shm_গ্রন্থি->mutex);
        জন্য(pShm=p_Shm_গ্রন্থি->pWinShmList; pShm; pShm=pShm->p_Win_Shm_পরবর্তী){
          যদি( bExcl ){
            যদি( (pShm->sharedMask|pShm->exclMask) & mask ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_মান;
            }
          }অন্যথা{
            যদি( pShm->sharedMask & mask ){
              h = INVALID_HANDLE_মান;
            }অন্যথা যদি( pShm->exclMask & mask ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_মান;
            }
          }
        }
        sqlite3_mutex_leave(p_Shm_গ্রন্থি->mutex);
      }

      যদি( h!=INVALID_HANDLE_মান ){
        rc = winHandleLockTimeout(h, ofst+WIN_SHM_BASE, n, bExcl, nMs);
      }
      যদি( rc==SQLITE_OK ){
        যদি( bExcl ){
          p->exclMask = (p->exclMask | mask);
        }অন্যথা{
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
  ফেরত rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
স্থির শূন্য winShmBarrier(
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
স্থির পূর্ণসংখ্যা winShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  পূর্ণসংখ্যা iRegion,                    /* Region to retrieve */
  পূর্ণসংখ্যা szRegion,                   /* Size of regions */
  পূর্ণসংখ্যা isWrite,                    /* True to extend file if necessary */
  শূন্য অস্থির **pp              /* OUT: Mapped memory */
){
  winFile *pDbFd = (winFile*)fd;
  winShm *pShm = pDbFd->pShm;
  win_Shm_গ্রন্থি *p_Shm_গ্রন্থি;
  DWORD protect = PAGE_READWRITE;
  DWORD flags = FILE_MAP_WRITE | FILE_MAP_READ;
  পূর্ণসংখ্যা rc = SQLITE_OK;

  যদি( !pShm ){
    rc = winOpenSharedMemory(pDbFd);
    যদি( rc!=SQLITE_OK ) ফেরত rc;
    pShm = pDbFd->pShm;
    প্রতিজ্ঞা( pShm!=0 );
  }
  p_Shm_গ্রন্থি = pShm->p_Shm_গ্রন্থি;

  sqlite3_mutex_enter(p_Shm_গ্রন্থি->mutex);
  যদি( p_Shm_গ্রন্থি->isUnlocked ){
    /* Take the DMS lock. */
    প্রতিজ্ঞা( p_Shm_গ্রন্থি->nRegion==0 );
    rc = winLockSharedMemory(p_Shm_গ্রন্থি, winFileBusyTimeout(pDbFd));
    যদি( rc!=SQLITE_OK ) যাও shmpage_out;
  }

  প্রতিজ্ঞা( szRegion==p_Shm_গ্রন্থি->szRegion || p_Shm_গ্রন্থি->nRegion==0 );
  যদি( p_Shm_গ্রন্থি->nRegion<=iRegion ){
    HANDLE hShared = p_Shm_গ্রন্থি->hSharedShm;
    কাঠামো ShmRegion *apNew;           /* New aRegion[] array */
    i64 nByte = ((i64)iRegion+1)*(i64)szRegion;  /* Minimum file size */
    sqlite3_int64 sz;                  /* Current size of wal-index file */

    p_Shm_গ্রন্থি->szRegion = szRegion;

    /* The requested region is not mapped into this processes address space.
    ** Check to see if it has been allocated (i.e. if the wal-index file is
    ** large enough to contain the requested region).
    */
    rc = win_Handle_আকার(hShared, &sz);
    যদি( rc!=SQLITE_OK ){
      rc = winLogError(rc, osGetLastError(), "winShmMap1", pDbFd->zPath);
      যাও shmpage_out;
    }

    যদি( sz<nByte ){
      /* The requested memory region does not exist. If isWrite is set to
      ** zero, exit early. *pp will be set to NULL and SQLITE_OK returned.
      **
      ** Alternatively, if isWrite is non-zero, use ftruncate() to allocate
      ** the requested memory region.  */
      যদি( !isWrite ) যাও shmpage_out;
      rc = winHandleTruncate(hShared, nByte);
      যদি( rc!=SQLITE_OK ){
        rc = winLogError(rc, osGetLastError(), "winShmMap2", pDbFd->zPath);
        যাও shmpage_out;
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (কাঠামো ShmRegion*)sqlite3_realloc64(
        p_Shm_গ্রন্থি->aRegion, ((i64)iRegion+1)*আকার(apNew[0])
    );
    যদি( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      যাও shmpage_out;
    }
    p_Shm_গ্রন্থি->aRegion = apNew;

    যদি( p_Shm_গ্রন্থি->isReadonly ){
      protect = PAGE_READONLY;
      flags = FILE_MAP_READ;
    }

    যখন( p_Shm_গ্রন্থি->nRegion<=iRegion ){
      HANDLE hMap = ফাঁকা;         /* file-mapping handle */
      শূন্য *pMap = 0;             /* Mapped memory region */

#ifdef SQLITE_UWP
      hMap = osCreateFileMappingFromApp(hShared, ফাঁকা, protect, nByte, ফাঁকা);
#অন্যথা
      hMap = osCreateFileMappingW(hShared, ফাঁকা, protect, 0, nByte, ফাঁকা);
#endif
      OSTRACE(("SHM-MAP-CREATE pid=%lu, region=%d, size=%lld, rc=%s\n",
               osGetCurrentProcessId(), p_Shm_গ্রন্থি->nRegion, nByte,
               hMap ? "ok" : "failed"));
      যদি( hMap ){
        i64 iOffset = p_Shm_গ্রন্থি->nRegion*szRegion;
        পূর্ণসংখ্যা iOffsetShift = iOffset % winSysInfo.dwAllocationGranularity;
#ifdef SQLITE_UWP
        pMap = osMapViewOfFileFromApp(hMap, flags,
            iOffset - iOffsetShift, (i64)szRegion + iOffsetShift
        );
#অন্যথা
        pMap = osMapViewOfFile(hMap, flags,
            0, iOffset - iOffsetShift, (i64)szRegion + iOffsetShift
        );
#endif
        OSTRACE(("SHM-MAP-MAP pid=%lu, region=%d, offset=%d, size=%d, rc=%s\n",
                 osGetCurrentProcessId(), p_Shm_গ্রন্থি->nRegion, iOffset,
                 szRegion, pMap ? "ok" : "failed"));
      }
      যদি( !pMap ){
        p_Shm_গ্রন্থি->lastErrno = osGetLastError();
        rc = winLogError(SQLITE_IOERR_SHMMAP, p_Shm_গ্রন্থি->lastErrno,
                         "winShmMap3", pDbFd->zPath);
        যদি( hMap ) osCloseHandle(hMap);
        যাও shmpage_out;
      }

      p_Shm_গ্রন্থি->aRegion[p_Shm_গ্রন্থি->nRegion].pMap = pMap;
      p_Shm_গ্রন্থি->aRegion[p_Shm_গ্রন্থি->nRegion].hMap = hMap;
      p_Shm_গ্রন্থি->nRegion++;
    }
  }

shmpage_out:
  যদি( p_Shm_গ্রন্থি->nRegion>iRegion ){
    i64 iOffset = (i64)iRegion*(i64)szRegion;
    পূর্ণসংখ্যা iOffsetShift = iOffset % winSysInfo.dwAllocationGranularity;
    অক্ষর *p = (অক্ষর *)p_Shm_গ্রন্থি->aRegion[iRegion].pMap;
    *pp = (শূন্য *)&p[iOffsetShift];
  }অন্যথা{
    *pp = 0;
  }
  যদি( p_Shm_গ্রন্থি->isReadonly && rc==SQLITE_OK ){
    rc = SQLITE_READONLY;
  }
  sqlite3_mutex_leave(p_Shm_গ্রন্থি->mutex);
  ফেরত rc;
}

#অন্যথা
# define winShmMap     0
# define winShmLock    0
# define winShmBarrier 0
# define winShmUnmap   0
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Cleans up the mapped region of the specified file, if any.
*/
#যদি SQLITE_MAX_MMAP_আকার>0
স্থির পূর্ণসংখ্যা winUnmapfile(winFile *pFile){
  প্রতিজ্ঞা( pFile!=0 );
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, pMapRegion=%p, "
           "mmapSize=%lld, mmapSizeMax=%lld\n",
           osGetCurrentProcessId(), pFile, pFile->hMap, pFile->pMapRegion,
           pFile->mmap_আকার, pFile->mmap_আকার_Max));
  যদি( pFile->pMapRegion ){
    যদি( !osUnmapViewOfFile(pFile->pMapRegion) ){
      pFile->lastErrno = osGetLastError();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(), pFile,
               pFile->pMapRegion));
      ফেরত winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winUnmapfile1", pFile->zPath);
    }
    pFile->pMapRegion = 0;
    pFile->mmap_আকার = 0;
  }
  যদি( pFile->hMap!=ফাঁকা ){
    যদি( !osCloseHandle(pFile->hMap) ){
      pFile->lastErrno = osGetLastError();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, rc=SQLITE_IOERR_MMAP\n",
               osGetCurrentProcessId(), pFile, pFile->hMap));
      ফেরত winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winUnmapfile2", pFile->zPath);
    }
    pFile->hMap = ফাঁকা;
  }
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile));
  ফেরত SQLITE_OK;
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
স্থির পূর্ণসংখ্যা winMapfile(winFile *pFd, sqlite3_int64 nByte){
  sqlite3_int64 nMap = nByte;
  পূর্ণসংখ্যা rc;

  প্রতিজ্ঞা( nMap>=0 || pFd->nFetchOut==0 );
  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, size=%lld\n",
           osGetCurrentProcessId(), pFd, nByte));

  যদি( pFd->nFetchOut>0 ) ফেরত SQLITE_OK;

  যদি( nMap<0 ){
    rc = win_File_আকার((sqlite3_file*)pFd, &nMap);
    যদি( rc ){
      OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_IOERR_FSTAT\n",
               osGetCurrentProcessId(), pFd));
      ফেরত SQLITE_IOERR_FSTAT;
    }
  }
  যদি( nMap>pFd->mmap_আকার_Max ){
    nMap = pFd->mmap_আকার_Max;
  }
  nMap &= ~(sqlite3_int64)(winSysInfo.dw_Page_আকার - 1);

  যদি( nMap==0 && pFd->mmap_আকার>0 ){
    winUnmapfile(pFd);
  }
  যদি( nMap!=pFd->mmap_আকার ){
    শূন্য *pNew = 0;
    DWORD protect = PAGE_READONLY;
    DWORD flags = FILE_MAP_READ;

    winUnmapfile(pFd);
#ifdef SQLITE_MMAP_READWRITE
    যদি( (pFd->ctrlFlags & WINFILE_RDONLY)==0 ){
      protect = PAGE_READWRITE;
      flags |= FILE_MAP_WRITE;
    }
#endif
#ifdef SQLITE_UWP
    pFd->hMap = osCreateFileMappingFromApp(pFd->h, ফাঁকা, protect, nMap, ফাঁকা);
#অন্যথা
    pFd->hMap = osCreateFileMappingW(pFd->h, ফাঁকা, protect,
                                (DWORD)((nMap>>32) & 0xffffffff),
                                (DWORD)(nMap & 0xffffffff), ফাঁকা);
#endif
    যদি( pFd->hMap==ফাঁকা ){
      pFd->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->lastErrno,
                       "winMapfile1", pFd->zPath);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-CREATE pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      ফেরত SQLITE_OK;
    }
    প্রতিজ্ঞা( (nMap % winSysInfo.dw_Page_আকার)==0 );
    প্রতিজ্ঞা( আকার(আকার_T)==আকার(sqlite3_int64) || nMap<=0xffffffff );
#ifdef SQLITE_UWP
    pNew = osMapViewOfFileFromApp(pFd->hMap, flags, 0, (আকার_T)nMap);
#অন্যথা
    pNew = osMapViewOfFile(pFd->hMap, flags, 0, 0, (আকার_T)nMap);
#endif
    যদি( pNew==ফাঁকা ){
      osCloseHandle(pFd->hMap);
      pFd->hMap = ফাঁকা;
      pFd->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->lastErrno,
                       "winMapfile2", pFd->zPath);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-MAP pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      ফেরত SQLITE_OK;
    }
    pFd->pMapRegion = pNew;
    pFd->mmap_আকার = nMap;
  }

  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFd));
  ফেরত SQLITE_OK;
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
স্থির পূর্ণসংখ্যা winFetch(sqlite3_file *fd, i64 iOff, পূর্ণসংখ্যা nAmt, শূন্য **pp){
#যদি SQLITE_MAX_MMAP_আকার>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */
#endif
  *pp = 0;

  OSTRACE(("FETCH pid=%lu, pFile=%p, offset=%lld, amount=%d, pp=%p\n",
           osGetCurrentProcessId(), fd, iOff, nAmt, pp));

#যদি SQLITE_MAX_MMAP_আকার>0
  যদি( pFd->mmap_আকার_Max>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    ধ্রুবক পূর্ণসংখ্যা n_Eof_বাফার = 256;
    যদি( pFd->pMapRegion==0 ){
      পূর্ণসংখ্যা rc = winMapfile(pFd, -1);
      যদি( rc!=SQLITE_OK ){
        OSTRACE(("FETCH pid=%lu, pFile=%p, rc=%s\n",
                 osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
        ফেরত rc;
      }
    }
    যদি( pFd->mmap_আকার >= (iOff+nAmt+n_Eof_বাফার) ){
      প্রতিজ্ঞা( pFd->pMapRegion!=0 );
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->nFetchOut++;
    }
  }
#endif

  OSTRACE(("FETCH pid=%lu, pFile=%p, pp=%p, *pp=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd, pp, *pp));
  ফেরত SQLITE_OK;
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
স্থির পূর্ণসংখ্যা winUnfetch(sqlite3_file *fd, i64 iOff, শূন্য *p){
#যদি SQLITE_MAX_MMAP_আকার>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  প্রতিজ্ঞা( (p==0)==(pFd->nFetchOut==0) );

  /* If p!=0, it must match the iOff value. */
  প্রতিজ্ঞা( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, offset=%lld, p=%p\n",
           osGetCurrentProcessId(), pFd, iOff, p));

  যদি( p ){
    pFd->nFetchOut--;
  }অন্যথা{
    /* FIXME:  If Windows truly always prevents truncating or deleting a
    ** file while a mapping is held, then the following winUnmapfile() call
    ** is unnecessary can be omitted - potentially improving
    ** performance.  */
    winUnmapfile(pFd);
  }

  প্রতিজ্ঞা( pFd->nFetchOut>=0 );
#endif

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd));
  ফেরত SQLITE_OK;
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
স্থির ধ্রুবক sqlite3_io_methods winIoMethod = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_আকার,                    /* xFileSize */
  winLock,                        /* xLock */
  winUnlock,                      /* xUnlock */
  winCheckReservedLock,           /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_আকার,                  /* xSectorSize */
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
স্থির ধ্রুবক sqlite3_io_methods winIoNolockMethod = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_আকার,                    /* xFileSize */
  winNolockLock,                  /* xLock */
  winNolockUnlock,                /* xUnlock */
  winNolockCheckReservedLock,     /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_আকার,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  winShmLock,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

স্থির win_Vfs_App_তথ্য win_App_তথ্য = {
  &winIoMethod,       /* pMethod */
  0,                  /* pAppData */
  0                   /* bNoLock */
};

স্থির win_Vfs_App_তথ্য win_Nolock_App_তথ্য = {
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
স্থির পূর্ণসংখ্যা winMakeEndInDirSep(পূর্ণসংখ্যা nBuf, অক্ষর *zBuf){
  যদি( zBuf ){
    পূর্ণসংখ্যা nLen = sqlite3Strlen30(zBuf);
    যদি( nLen>0 ){
      যদি( winIsDirSep(zBuf[nLen-1]) ){
        ফেরত 1;
      }অন্যথা যদি( nLen+1<nBuf ){
        যদি( !osGetenv ){
          zBuf[nLen] = winGetDirSep();
        }অন্যথা যদি( winIsDriveLetterAndColon(zBuf) && winIsDirSep(zBuf[2]) ){
          zBuf[nLen] = '\\';
          zBuf[2]='\\';
        }অন্যথা{
          zBuf[nLen] = '/';
        }
        zBuf[nLen+1] = '\0';
        ফেরত 1;
      }
    }
  }
  ফেরত 0;
}

/*
** If sqlite3_temp_directory is defined, take the mutex and return true.
**
** If sqlite3_temp_directory is NULL (undefined), omit the mutex and
** return false.
*/
স্থির পূর্ণসংখ্যা win_অস্থায়ী_Dir_Defined(শূন্য){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  যদি( sqlite3_অস্থায়ী_directory!=0 ) ফেরত 1;
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ফেরত 0;
}

/*
** Create a temporary file name and store the resulting pointer into pzBuf.
** The pointer returned in pzBuf must be freed via sqlite3_free().
*/
স্থির পূর্ণসংখ্যা winGetTempname(sqlite3_vfs *pVfs, অক্ষর **pzBuf){
  স্থির ধ্রুবক অক্ষর zChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  size_t i, j;
  DWORD pid;
  পূর্ণসংখ্যা nPre = sqlite3Strlen30(SQLITE_অস্থায়ী_FILE_PREFIX);
  i64 nMax, nBuf, nDir, nLen;
  অক্ষর *zBuf;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  SimulateIOError( ফেরত SQLITE_IOERR );

  /* Allocate a temporary buffer to store the fully qualified file
  ** name for the temporary file.  If this fails, we cannot continue.
  */
  nMax = pVfs->mxPathname;
  nBuf = 2 + (i64)nMax;
  zBuf = sqlite3MallocZero( nBuf );
  যদি( !zBuf ){
    OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
    ফেরত SQLITE_IOERR_NOMEM_BKPT;
  }

  /* Figure out the effective temporary directory.  First, check if one
  ** has been explicitly set by the application; otherwise, use the one
  ** configured by the operating system.
  */
  nDir = nMax - (nPre + 15);
  প্রতিজ্ঞা( nDir>0 );
  যদি( win_অস্থায়ী_Dir_Defined() ){
    পূর্ণসংখ্যা nDirLen = sqlite3Strlen30(sqlite3_অস্থায়ী_directory);
    যদি( nDirLen>0 ){
      যদি( !winIsDirSep(sqlite3_অস্থায়ী_directory[nDirLen-1]) ){
        nDirLen++;
      }
      যদি( nDirLen>nDir ){
        sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
        sqlite3_free(zBuf);
        OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
        ফেরত winLogError(SQLITE_ERROR, 0, "winGetTempname1", 0);
      }
      sqlite3_snprintf(nMax, zBuf, "%s", sqlite3_অস্থায়ী_directory);
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  }

#যদি defined(__CYGWIN__)
  অন্যথা যদি( osGetenv!=ফাঁকা ){
    স্থির ধ্রুবক অক্ষর *azDirs[] = {
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
    unsigned পূর্ণসংখ্যা i;
    ধ্রুবক অক্ষর *zDir = 0;

    যদি( !azDirs[0] ) azDirs[0] = osGetenv("SQLITE_TMPDIR");
    যদি( !azDirs[1] ) azDirs[1] = osGetenv("TMPDIR");
    যদি( !azDirs[2] ) azDirs[2] = osGetenv("TMP");
    যদি( !azDirs[3] ) azDirs[3] = osGetenv("TEMP");
    যদি( !azDirs[4] ) azDirs[4] = osGetenv("USERPROFILE");
    জন্য(i=0; i<আকার(azDirs)/আকার(azDirs[0]); zDir=azDirs[i++]){
      LPWSTR zConverted;
      যদি( zDir==0 ) চালু;
      /* If the path starts with a drive letter followed by the colon
      ** character, assume it is already a native Win32 path; otherwise,
      ** it must be converted to a native Win32 path via the Cygwin API
      ** prior to using it.
      */
      {
        zConverted = winConvertFromUtf8Filename(zDir);
        যদি( !zConverted ){
          sqlite3_free(zBuf);
          OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
          ফেরত SQLITE_IOERR_NOMEM_BKPT;
        }
        যদি( winIsDir(zConverted) ){
          sqlite3_snprintf(nMax, zBuf, "%s", zDir);
          sqlite3_free(zConverted);
          বিরতি;
        }
        sqlite3_free(zConverted);
      }
    }
  }
#endif

  অন্যথা{
    অক্ষর *zMulti;
    LPWSTR zWidePath = sqlite3MallocZero( nMax*আকার(WCHAR) );
    যদি( !zWidePath ){
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      ফেরত SQLITE_IOERR_NOMEM_BKPT;
    }
    যদি( os_Get_অস্থায়ী_Path_W==0 || os_Get_অস্থায়ী_Path_W(nMax, zWidePath)==0 ){
      sqlite3_free(zWidePath);
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_GETTEMPPATH\n"));
      ফেরত winLogError(SQLITE_IOERR_GETTEMPPATH, osGetLastError(),
                         "winGetTempname2", 0);
    }
    zMulti = winUnicodeToUtf8(zWidePath);
    sqlite3_free(zWidePath);
    যদি( zMulti ){
      sqlite3_snprintf(nMax, zBuf, "%s", zMulti);
      sqlite3_free(zMulti);
    }অন্যথা{
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      ফেরত SQLITE_IOERR_NOMEM_BKPT;
    }
  }

  /*
  ** Check to make sure the temporary directory ends with an appropriate
  ** separator.  If it does not and there is not enough space left to add
  ** one, fail.
  */
  যদি( !winMakeEndInDirSep(nDir+1, zBuf) ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    ফেরত winLogError(SQLITE_ERROR, 0, "winGetTempname4", 0);
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
  যদি( (nLen + nPre + 17) > nBuf ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    ফেরত winLogError(SQLITE_ERROR, 0, "winGetTempname5", 0);
  }

  sqlite3_snprintf(nBuf-16-nLen, zBuf+nLen, SQLITE_অস্থায়ী_FILE_PREFIX);

  j = sqlite3Strlen30(zBuf);
  sqlite3_randomness(15, &zBuf[j]);
  pid = osGetCurrentProcessId();
  জন্য(i=0; i<15; i++, j++){
    zBuf[j] += pid & 0xff;
    pid >>= 8;
    zBuf[j] = (অক্ষর)zChars[ ((unsigned অক্ষর)zBuf[j])%(আকার(zChars)-1) ];
  }
  zBuf[j] = 0;
  zBuf[j+1] = 0;
  *pzBuf = zBuf;

  OSTRACE(("TEMP-FILENAME name=%s, rc=SQLITE_OK\n", zBuf));
  ফেরত SQLITE_OK;
}

/*
** Return TRUE if the named file is really a directory.  Return false if
** it is something other than a directory, or if there is any kind of memory
** allocation failure.
*/
স্থির পূর্ণসংখ্যা winIsDir(LPCWSTR zConverted){
  DWORD attr;
  পূর্ণসংখ্যা rc = 0;
  DWORD lastErrno;
  পূর্ণসংখ্যা cnt = 0;
  WIN32_FILE_ATTRIBUTE_তথ্য s_Attr_তথ্য;
  memset(&s_Attr_তথ্য, 0, আকার(s_Attr_তথ্য));
  যখন( !(rc = osGetFileAttributesExW(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_তথ্য)) && winRetryIoerr(&cnt, &lastErrno) ){}
  যদি( !rc ){
    ফেরত 0; /* Invalid name? */
  }
  attr = s_Attr_তথ্য.dwFileAttributes;
  ফেরত (attr!=INVALID_FILE_ATTRIBUTES) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

/* forward reference */
স্থির পূর্ণসংখ্যা winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ধ্রুবক অক্ষর *zFilename,     /* Name of file to check */
  পূর্ণসংখ্যা flags,                 /* Type of test to make on this file */
  পূর্ণসংখ্যা *pResOut               /* OUT: Result */
);

/*
** The Windows version of xAccess() accepts an extra bit in the flags
** parameter that prevents an anti-virus retry loop.
*/
#define NORETRY 0x4000

/*
** Open a file.
*/
স্থির পূর্ণসংখ্যা winOpen(
  sqlite3_vfs *pVfs,        /* Used to get maximum path length and AppData */
  ধ্রুবক অক্ষর *zName,        /* Name of the file (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  পূর্ণসংখ্যা flags,                /* Open mode flags */
  পূর্ণসংখ্যা *pOutFlags            /* Status return flags */
){
  HANDLE h;
  DWORD lastErrno = 0;
  DWORD dwDesiredAccess;
  DWORD dwShareMode;
  DWORD dwCreationDisposition;
  DWORD dwFlagsAndAttributes = 0;
  win_Vfs_App_তথ্য *p_App_তথ্য;
  winFile *pFile = (winFile*)id;
  LPWSTR zConverted;              /* Filename in OS encoding */
  ধ্রুবক অক্ষর *zUtf8Name = zName; /* Filename in UTF-8 encoding */
  পূর্ণসংখ্যা cnt = 0;
  পূর্ণসংখ্যা isRO = 0;              /* file is known to be accessible readonly */

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  অক্ষর *zTmpname = 0; /* For temporary filename, if necessary. */

  পূর্ণসংখ্যা rc = SQLITE_OK;            /* Function Return Code */
#যদি !defined(NDEBUG)
  পূর্ণসংখ্যা eType = flags&0x0FFF00;  /* Type of file to open */
#endif

  পূর্ণসংখ্যা isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  পূর্ণসংখ্যা isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  পূর্ণসংখ্যা isCreate     = (flags & SQLITE_OPEN_CREATE);
  পূর্ণসংখ্যা isReadonly   = (flags & SQLITE_OPEN_READONLY);
  পূর্ণসংখ্যা isReadWrite  = (flags & SQLITE_OPEN_READWRITE);

#ifndef NDEBUG
  পূর্ণসংখ্যা isOpenJournal = (isCreate && (
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
  প্রতিজ্ঞা((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
  প্রতিজ্ঞা(isCreate==0 || isReadWrite);
  প্রতিজ্ঞা(isExclusive==0 || isCreate);
  প্রতিজ্ঞা(isDelete==0 || isCreate);

  /* The main DB, main journal, WAL file and super-journal are never
  ** automatically deleted. Nor are they ever temporary files.  */
  প্রতিজ্ঞা( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_DB );
  প্রতিজ্ঞা( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_JOURNAL );
  প্রতিজ্ঞা( (!isDelete && zName) || eType!=SQLITE_OPEN_SUPER_JOURNAL );
  প্রতিজ্ঞা( (!isDelete && zName) || eType!=SQLITE_OPEN_WAL );

  /* Assert that the upper layer has set one of the "file-type" flags. */
  প্রতিজ্ঞা( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_অস্থায়ী_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_অস্থায়ী_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );

  প্রতিজ্ঞা( pFile!=0 );
  memset(pFile, 0, আকার(winFile));
  pFile->h = INVALID_HANDLE_মান;

  /* If the second argument to this function is NULL, generate a
  ** temporary file name to use
  */
  যদি( !zUtf8Name ){
    প্রতিজ্ঞা( isDelete && !isOpenJournal );
    rc = winGetTempname(pVfs, &zTmpname);
    যদি( rc!=SQLITE_OK ){
      OSTRACE(("OPEN name=%s, rc=%s", zUtf8Name, sqlite3ErrName(rc)));
      ফেরত rc;
    }
    zUtf8Name = zTmpname;
  }

  /* Database filenames are double-zero terminated if they are not
  ** URIs with parameters.  Hence, they can always be passed into
  ** sqlite3_uri_parameter().
  */
  প্রতিজ্ঞা( (eType!=SQLITE_OPEN_MAIN_DB) || (flags & SQLITE_OPEN_URI) ||
       zUtf8Name[sqlite3Strlen30(zUtf8Name)+1]==0 );

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8Name);
  যদি( zConverted==0 ){
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8Name));
    ফেরত SQLITE_IOERR_NOMEM_BKPT;
  }

  যদি( winIsDir(zConverted) ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8Name));
    ফেরত SQLITE_CANTOPEN_ISDIR;
  }

  যদি( isReadWrite ){
    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
  }অন্যথা{
    dwDesiredAccess = GENERIC_READ;
  }

  /* SQLITE_OPEN_EXCLUSIVE is used to make sure that a new file is
  ** created. SQLite doesn't use it to indicate "exclusive access"
  ** as it is usually understood.
  */
  যদি( isExclusive ){
    /* Creates a new file, only if it does not already exist. */
    /* If the file exists, it fails. */
    dwCreationDisposition = CREATE_NEW;
  }অন্যথা যদি( isCreate ){
    /* Open existing file, or create if it doesn't exist */
    dwCreationDisposition = OPEN_ALWAYS;
  }অন্যথা{
    /* Opens a file, only if it exists. */
    dwCreationDisposition = OPEN_EXISTING;
  }

  যদি( 0==sqlite3_uri_boolean(zName, "exclusive", 0) ){
    dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  }অন্যথা{
    dwShareMode = 0;
  }

  যদি( isDelete ){
    dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY
                               | FILE_ATTRIBUTE_HIDDEN
                               | FILE_FLAG_DELETE_ON_CLOSE;
  }অন্যথা{
    dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
  }
  করো{
    h = osCreateFileW(zConverted,
                      dwDesiredAccess,
                      dwShareMode, ফাঁকা,
                      dwCreationDisposition,
                      dwFlagsAndAttributes,
                      ফাঁকা);
    যদি( h!=INVALID_HANDLE_মান ) বিরতি;
    যদি( isReadWrite ){
      পূর্ণসংখ্যা rc2;
      sqlite3BeginBenignMalloc();
      rc2 = winAccess(pVfs, zUtf8Name, SQLITE_ACCESS_READ|NORETRY, &isRO);
      sqlite3EndBenignMalloc();
      যদি( rc2==SQLITE_OK && isRO ) বিরতি;
    }
  }যখন( winRetryIoerr(&cnt, &lastErrno) );
  winLogIoerr(cnt, __LINE__);

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, rc=%s\n", h, zUtf8Name,
           dwDesiredAccess, (h==INVALID_HANDLE_মান) ? "failed" : "ok"));

  যদি( h==INVALID_HANDLE_মান ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    যদি( isReadWrite && isRO && !isExclusive ){
      ফেরত winOpen(pVfs, zName, id,
         ((flags|SQLITE_OPEN_READONLY) &
                     ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE)),
         pOutFlags);
    }অন্যথা{
      pFile->lastErrno = lastErrno;
      winLogError(SQLITE_CANTOPEN, pFile->lastErrno, "winOpen", zUtf8Name);
      ফেরত SQLITE_CANTOPEN_BKPT;
    }
  }

  যদি( pOutFlags ){
    যদি( isReadWrite ){
      *pOutFlags = SQLITE_OPEN_READWRITE;
    }অন্যথা{
      *pOutFlags = SQLITE_OPEN_READONLY;
    }
  }

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, pOutFlags=%p, *pOutFlags=%d, "
           "rc=%s\n", h, zUtf8Name, dwDesiredAccess, pOutFlags, pOutFlags ?
           *pOutFlags : 0, (h==INVALID_HANDLE_মান) ? "failed" : "ok"));

  p_App_তথ্য = (win_Vfs_App_তথ্য*)pVfs->p_App_তথ্য;

  sqlite3_free(zConverted);
  sqlite3_free(zTmpname);
  id->pMethods = p_App_তথ্য ? p_App_তথ্য->pMethod : &winIoMethod;
  pFile->pVfs = pVfs;
  pFile->h = h;
  যদি( isReadonly ){
    pFile->ctrlFlags |= WINFILE_RDONLY;
  }
  যদি( (flags & SQLITE_OPEN_MAIN_DB)
   && sqlite3_uri_boolean(zName, "psow", SQLITE_POWERSAFE_OVERWRITE) 
  ){
    pFile->ctrlFlags |= WINFILE_PSOW;
  }
  pFile->lastErrno = NO_ERROR;
  pFile->zPath = zName;
#যদি SQLITE_MAX_MMAP_আকার>0
  pFile->hMap = ফাঁকা;
  pFile->pMapRegion = 0;
  pFile->mmap_আকার = 0;
  pFile->mmap_আকার_Max = sqlite3GlobalConfig.szMmap;
#endif

  Open_গণক(+1);
  ফেরত rc;
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
স্থির পূর্ণসংখ্যা winDelete(
  sqlite3_vfs *pVfs,          /* Not used on win32 */
  ধ্রুবক অক্ষর *zFilename,      /* Name of file to delete */
  পূর্ণসংখ্যা syncDir                 /* Not used on win32 */
){
  পূর্ণসংখ্যা cnt = 0;
  পূর্ণসংখ্যা rc;
  DWORD attr;
  DWORD lastErrno = 0;
  LPWSTR zConverted;
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(syncDir);

  SimulateIOError(ফেরত SQLITE_IOERR_DELETE);
  OSTRACE(("DELETE name=%s, syncDir=%d\n", zFilename, syncDir));

  zConverted = winConvertFromUtf8Filename(zFilename);
  যদি( zConverted==0 ){
    OSTRACE(("DELETE name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    ফেরত SQLITE_IOERR_NOMEM_BKPT;
  }
  করো {
    attr = osGetFileAttributesW(zConverted);
    যদি ( attr==INVALID_FILE_ATTRIBUTES ){
      lastErrno = osGetLastError();
      যদি( lastErrno==ERROR_FILE_NOT_FOUND
       || lastErrno==ERROR_PATH_NOT_FOUND ){
        rc = SQLITE_IOERR_DELETE_NOENT; /* Already gone? */
      }অন্যথা{
        rc = SQLITE_ERROR;
      }
      বিরতি;
    }
    যদি ( attr&FILE_ATTRIBUTE_DIRECTORY ){
      rc = SQLITE_ERROR; /* Files only. */
      বিরতি;
    }
    যদি ( osDeleteFileW(zConverted) ){
      rc = SQLITE_OK; /* Deleted OK. */
      বিরতি;
    }
    যদি ( !winRetryIoerr(&cnt, &lastErrno) ){
      rc = SQLITE_ERROR; /* No more retries. */
      বিরতি;
    }
  } যখন(1);
  যদি( rc && rc!=SQLITE_IOERR_DELETE_NOENT ){
    rc = winLogError(SQLITE_IOERR_DELETE, lastErrno, "winDelete", zFilename);
  }অন্যথা{
    winLogIoerr(cnt, __LINE__);
  }
  sqlite3_free(zConverted);
  OSTRACE(("DELETE name=%s, rc=%s\n", zFilename, sqlite3ErrName(rc)));
  ফেরত rc;
}

/*
** Check the existence and status of a file.
*/
স্থির পূর্ণসংখ্যা winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ধ্রুবক অক্ষর *zFilename,     /* Name of file to check */
  পূর্ণসংখ্যা flags,                 /* Type of test to make on this file */
  পূর্ণসংখ্যা *pResOut               /* OUT: Result */
){
  DWORD attr;
  পূর্ণসংখ্যা rc = 0;
  পূর্ণসংখ্যা cnt = 0;
  DWORD lastErrno = 0;
  LPWSTR zConverted;
  পূর্ণসংখ্যা noRetry = 0;           /* Do not use winRetryIoerr() */
  WIN32_FILE_ATTRIBUTE_তথ্য s_Attr_তথ্য;
  UNUSED_PARAMETER(pVfs);

  যদি( (flags & NORETRY)!=0 ){
    noRetry = 1;
    flags &= ~NORETRY;
  }  

  SimulateIOError( ফেরত SQLITE_IOERR_ACCESS; );
  OSTRACE(("ACCESS name=%s, flags=%x, pResOut=%p\n",
           zFilename, flags, pResOut));

  যদি( zFilename==0 ){
    *pResOut = 0;
    OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
             zFilename, pResOut, *pResOut));
    ফেরত SQLITE_OK;
  }

  zConverted = winConvertFromUtf8Filename(zFilename);
  যদি( zConverted==0 ){
    OSTRACE(("ACCESS name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    ফেরত SQLITE_IOERR_NOMEM_BKPT;
  }
  memset(&s_Attr_তথ্য, 0, আকার(s_Attr_তথ্য));
  যখন( !(rc = osGetFileAttributesExW(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_তথ্য))
     && !noRetry
     && winRetryIoerr(&cnt, &lastErrno)
  ){ /* Loop until true */}
  যদি( rc ){
    /* For an SQLITE_ACCESS_EXISTS query, treat a zero-length file
    ** as if it does not exist.
    */
    যদি(    flags==SQLITE_ACCESS_EXISTS
        && s_Attr_তথ্য.n_File_আকার_উপরের==0
        && s_Attr_তথ্য.n_File_আকার_নিচের==0 ){
      attr = INVALID_FILE_ATTRIBUTES;
    }অন্যথা{
      attr = s_Attr_তথ্য.dwFileAttributes;
    }
  }অন্যথা{
    যদি( noRetry ) lastErrno = osGetLastError();
    winLogIoerr(cnt, __LINE__);
    যদি( lastErrno!=ERROR_FILE_NOT_FOUND && lastErrno!=ERROR_PATH_NOT_FOUND ){
      sqlite3_free(zConverted);
      ফেরত winLogError(SQLITE_IOERR_ACCESS, lastErrno, "winAccess",
                         zFilename);
    }অন্যথা{
      attr = INVALID_FILE_ATTRIBUTES;
    }
  }
  sqlite3_free(zConverted);
  নির্বাচন( flags ){
    ক্ষেত্র SQLITE_ACCESS_READ:
    ক্ষেত্র SQLITE_ACCESS_EXISTS:
      rc = attr!=INVALID_FILE_ATTRIBUTES;
      বিরতি;
    ক্ষেত্র SQLITE_ACCESS_READWRITE:
      rc = attr!=INVALID_FILE_ATTRIBUTES &&
             (attr & FILE_ATTRIBUTE_READONLY)==0;
      বিরতি;
    পূর্বনির্ধারিত:
      প্রতিজ্ঞা(!"Invalid flags argument");
  }
  *pResOut = rc;
  OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           zFilename, pResOut, *pResOut));
  ফেরত SQLITE_OK;
}

/*
** Returns non-zero if the specified path name starts with the "long path"
** prefix.
*/
স্থির BOOL winIsLongPathPrefix(
  ধ্রুবক অক্ষর *zPathname
){
  ফেরত ( zPathname[0]=='\\' && zPathname[1]=='\\'
        && zPathname[2]=='?'  && zPathname[3]=='\\' );
}

/*
** Returns non-zero if the specified path name starts with a drive letter
** followed by a colon character.
*/
স্থির BOOL winIsDriveLetterAndColon(
  ধ্রুবক অক্ষর *zPathname
){
  ফেরত ( sqlite3Isalpha(zPathname[0]) && zPathname[1]==':' );
}

#ifdef _WIN32
/*
** Returns non-zero if the specified path name should be used verbatim.  If
** non-zero is returned from this function, the calling function must simply
** use the provided path name verbatim -OR- resolve it into a full path name
** using the GetFullPathName Win32 API function (if available).
*/
স্থির BOOL winIsVerbatimPathname(
  ধ্রুবক অক্ষর *zPathname
){
  /*
  ** If the path name starts with a forward slash or a backslash, it is either
  ** a legal UNC name, a volume relative path, or an absolute path name in the
  ** "Unix" format on Windows.  There is no easy way to differentiate between
  ** the final two cases; therefore, we return the safer return value of TRUE
  ** so that callers of this function will simply use it verbatim.
  */
  যদি ( winIsDirSep(zPathname[0]) ){
    ফেরত TRUE;
  }

  /*
  ** If the path name starts with a letter and a colon it is either a volume
  ** relative path or an absolute path.  Callers of this function must not
  ** attempt to treat it as a relative path name (i.e. they should simply use
  ** it verbatim).
  */
  যদি ( winIsDriveLetterAndColon(zPathname) ){
    ফেরত TRUE;
  }

  /*
  ** If we get to this point, the path name should almost certainly be a purely
  ** relative one (i.e. not a UNC name, not absolute, and not volume relative).
  */
  ফেরত FALSE;
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
স্থির শূন্য winSimplifyName(অক্ষর *z){
  পূর্ণসংখ্যা i, j;
  জন্য(i=j=0; z[i]; ++i){
    যদি( winIsDirSep(z[i]) ){
#যদি !defined(SQLITE_TEST)
      /* Some test-cases assume that "./foo" and "foo" are different */
      যদি( z[i+1]=='.' && winIsDirSep(z[i+2]) ){
        ++i;
        চালু;
      }
#endif
      যদি( !z[i+1] || (winIsDirSep(z[i+1]) && (i!=0)) ){
        চালু;
      }
      z[j++] = osGetenv?'/':'\\';
    }অন্যথা{
      z[j++] = z[i];
    }
  }
  যখন(j<i) z[j++] = '\0';
}

#define SQLITE_MAX_SYMLINKS 100

স্থির পূর্ণসংখ্যা mkFullPathname(
  ধ্রুবক অক্ষর *zPath,              /* Input path */
  অক্ষর *zOut,                     /* Output buffer */
  পূর্ণসংখ্যা nOut                        /* Allocated size of buffer zOut */
){
  পূর্ণসংখ্যা nPath = sqlite3Strlen30(zPath);
  পূর্ণসংখ্যা iOff = 0;
  যদি( zPath[0]!='/' ){
    যদি( osGetcwd(zOut, nOut-2)==0 ){
      ফেরত winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno, "getcwd", zPath);
    }
    iOff = sqlite3Strlen30(zOut);
    zOut[iOff++] = '/';
  }
  যদি( (iOff+nPath+1)>nOut ){
    /* SQLite assumes that xFullPathname() nul-terminates the output buffer
    ** even if it returns an error.  */
    zOut[iOff] = '\0';
    ফেরত SQLITE_CANTOPEN_BKPT;
  }
  sqlite3_snprintf(nOut-iOff, &zOut[iOff], "%s", zPath);
  ফেরত SQLITE_OK;
}
#endif /* __CYGWIN__ */

/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
স্থির পূর্ণসংখ্যা winFullPathnameNoMutex(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ধ্রুবক অক্ষর *zRelative,        /* Possibly relative input path */
  পূর্ণসংখ্যা nFull,                    /* Size of output buffer in bytes */
  অক্ষর *zFull                   /* Output buffer */
){
  পূর্ণসংখ্যা nByte;
  LPWSTR zConverted;
  অক্ষর *zOut;

  /* If this path name begins with "/X:" or "\\?\", where "X" is any
  ** alphabetic character, discard the initial "/" from the pathname.
  */
  যদি( zRelative[0]=='/' && (winIsDriveLetterAndColon(zRelative+1)
       || winIsLongPathPrefix(zRelative+1)) ){
    zRelative++;
  }

  SimulateIOError( ফেরত SQLITE_ERROR );

#ifdef __CYGWIN__
  যদি( osGetcwd ){
    zFull[nFull-1] = '\0';
    যদি( !winIsDriveLetterAndColon(zRelative) || !winIsDirSep(zRelative[2]) ){
      পূর্ণসংখ্যা rc = SQLITE_OK;
      পূর্ণসংখ্যা nLink = 1;              /* Number of symbolic links followed so far */
      ধ্রুবক অক্ষর *zIn = zRelative; /* Input path for each iteration of loop */
      অক্ষর *zDel = 0;
      কাঠামো stat buf;

      UNUSED_PARAMETER(pVfs);

      করো {
        /* Call lstat() on path zIn. Set bLink to true if the path
        ** is a symbolic link, or false otherwise.  */
        পূর্ণসংখ্যা bLink = 0;
        যদি( osLstat && osReadlink ) {
          যদি( osLstat(zIn, &buf)!=0 ){
            পূর্ণসংখ্যা myErrno = osErrno;
            যদি( myErrno!=ENOENT ){
              rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)myErrno,
                               "lstat", zIn);
            }
          }অন্যথা{
            bLink = ((buf.st_mode & 0170000) == 0120000);
          }

          যদি( bLink ){
            যদি( zDel==0 ){
              zDel = sqlite3MallocZero(nFull);
              যদি( zDel==0 ) rc = SQLITE_NOMEM;
            }অন্যথা যদি( ++nLink>SQLITE_MAX_SYMLINKS ){
              rc = SQLITE_CANTOPEN_BKPT;
            }

            যদি( rc==SQLITE_OK ){
              nByte = osReadlink(zIn, zDel, nFull-1);
              যদি( nByte ==(DWORD)-1 ){
                rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno,
                                 "readlink", zIn);
              }অন্যথা{
                যদি( zDel[0]!='/' ){
                  পূর্ণসংখ্যা n;
                  জন্য(n = sqlite3Strlen30(zIn); n>0 && zIn[n-1]!='/'; n--);
                  যদি( nByte+n+1>nFull ){
                    rc = SQLITE_CANTOPEN_BKPT;
                  }অন্যথা{
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

        প্রতিজ্ঞা( rc!=SQLITE_OK || zIn!=zFull || zIn[0]=='/' );
        যদি( rc==SQLITE_OK && zIn!=zFull ){
          rc = mkFullPathname(zIn, zFull, nFull);
        }
        যদি( bLink==0 ) বিরতি;
        zIn = zFull;
      }যখন( rc==SQLITE_OK );

      sqlite3_free(zDel);
      winSimplifyName(zFull);
      ফেরত rc;
    }
  }
#endif /* __CYGWIN__ */

#যদি defined(_WIN32)
  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing. This function could fail if, for example, the
  ** current working directory has been unlinked.
  */
  SimulateIOError( ফেরত SQLITE_ERROR );
  যদি ( sqlite3_তথ্য_directory && !winIsVerbatimPathname(zRelative) ){
    /*
    ** NOTE: We are dealing with a relative path name and the data
    **       directory has been set.  Therefore, use it as the basis
    **       for converting the relative path name to an absolute
    **       one by prepending the data directory and a backslash.
    */
    sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s%c%s",
                     sqlite3_তথ্য_directory, winGetDirSep(), zRelative);
    ফেরত SQLITE_OK;
  }
#endif
  zConverted = winConvertFromUtf8Filename(zRelative);
  যদি( zConverted==0 ){
    ফেরত SQLITE_IOERR_NOMEM_BKPT;
  }
  {
    LPWSTR z_অস্থায়ী;
    nByte = osGetFullPathNameW(zConverted, 0, 0, 0);
    যদি( nByte==0 ){
      sqlite3_free(zConverted);
      ফেরত winLogError(SQLITE_CANTOPEN_FULLPATH, osGetLastError(),
                         "winFullPathname1", zRelative);
    }
    nByte += 3;
    z_অস্থায়ী = sqlite3MallocZero( (u64)nByte*আকার(WCHAR) );
    যদি( z_অস্থায়ী==0 ){
      sqlite3_free(zConverted);
      ফেরত SQLITE_IOERR_NOMEM_BKPT;
    }
    nByte = osGetFullPathNameW(zConverted, nByte, z_অস্থায়ী, 0);
    যদি( nByte==0 ){
      sqlite3_free(zConverted);
      sqlite3_free(z_অস্থায়ী);
      ফেরত winLogError(SQLITE_CANTOPEN_FULLPATH, osGetLastError(),
                         "winFullPathname2", zRelative);
    }
    sqlite3_free(zConverted);
    zOut = winUnicodeToUtf8(z_অস্থায়ী);
    sqlite3_free(z_অস্থায়ী);
  }
  যদি( zOut ){
#ifdef __CYGWIN__
    যদি( memcmp(zOut, "\\\\?\\", 4) ){
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut);
    }অন্যথা যদি( memcmp(zOut+4, "UNC\\", 4) ){
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut+4);
    }অন্যথা{
      অক্ষর *p = zOut+6;
      *p = '\\';
      যদি( osGetcwd ){
        /* On Cygwin, UNC paths use forward slashes */
        যখন( *p ){
          যদি( *p=='\\' ) *p = '/';
          ++p;
        }
      }
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut+6);
    }
#অন্যথা
    sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut);
#endif /* __CYGWIN__ */
    sqlite3_free(zOut);
    ফেরত SQLITE_OK;
  }অন্যথা{
    ফেরত SQLITE_IOERR_NOMEM_BKPT;
  }
}
স্থির পূর্ণসংখ্যা winFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ধ্রুবক অক্ষর *zRelative,        /* Possibly relative input path */
  পূর্ণসংখ্যা nFull,                    /* Size of output buffer in bytes */
  অক্ষর *zFull                   /* Output buffer */
){
  পূর্ণসংখ্যা rc;
  MUTEX_LOGIC( sqlite3_mutex *pMutex; )
  MUTEX_LOGIC( pMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR); )
  sqlite3_mutex_enter(pMutex);
  rc = winFullPathnameNoMutex(pVfs, zRelative, nFull, zFull);
  sqlite3_mutex_leave(pMutex);
  ফেরত rc;
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
স্থির শূন্য *winDlOpen(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zFilename){
  HANDLE h = 0;
  LPWSTR zConverted = winConvertFromUtf8Filename(zFilename);
  UNUSED_PARAMETER(pVfs);
  যদি( zConverted==0 ){
    OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (শূন্য*)0));
    ফেরত 0;
  }
  h = osLoadLibraryW ? osLoadLibraryW(zConverted) : 0;
  OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (শূন্য*)h));
  sqlite3_free(zConverted);
  ফেরত (শূন্য*)h;
}
স্থির শূন্য winDlError(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nBuf, অক্ষর *zBufOut){
  UNUSED_PARAMETER(pVfs);
  win_Get_Last_Error_বার্তা(osGetLastError(), nBuf, zBufOut);
}
স্থির শূন্য (*winDlSym(sqlite3_vfs *pVfs,শূন্য *pH,ধ্রুবক অক্ষর *zSym))(শূন্য){
  FARPROC proc = 0;
  UNUSED_PARAMETER(pVfs);
  proc = osGetProcAddressA ? osGetProcAddressA((HANDLE)pH, zSym) : 0;
  OSTRACE(("DLSYM handle=%p, symbol=%s, address=%p\n",
           (শূন্য*)pH, zSym, (শূন্য*)proc));
  ফেরত (শূন্য(*)(শূন্য))proc;
}
স্থির শূন্য winDlClose(sqlite3_vfs *pVfs, শূন্য *pHandle){
  UNUSED_PARAMETER(pVfs);
  যদি( osFreeLibrary!=0 ) osFreeLibrary((HANDLE)pHandle);
  OSTRACE(("DLCLOSE handle=%p\n", (শূন্য*)pHandle));
}
#অন্যথা /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #define winDlOpen  0
  #define winDlError 0
  #define winDlSym   0
  #define winDlClose 0
#endif

/* State information for the randomness gatherer. */
প্রকারলেখ কাঠামো EntropyGatherer EntropyGatherer;
কাঠামো EntropyGatherer {
  unsigned অক্ষর *a;   /* Gather entropy into this buffer */
  পূর্ণসংখ্যা na;             /* Size of a[] in bytes */
  পূর্ণসংখ্যা i;              /* XOR next input into a[i] */
  পূর্ণসংখ্যা nXor;           /* Number of XOR operations done */
};

#যদি !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
/* Mix sz bytes of entropy into p. */
স্থির শূন্য xorMemory(EntropyGatherer *p, unsigned অক্ষর *x, পূর্ণসংখ্যা sz){
  পূর্ণসংখ্যা j, k;
  জন্য(j=0, k=p->i; j<sz; j++){
    p->a[k++] ^= x[j];
    যদি( k>=p->na ) k = 0;
  }
  p->i = k;
  p->nXor += sz;
}
#endif /* !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS) */

/*
** Write up to nBuf bytes of randomness into zBuf.
*/
স্থির পূর্ণসংখ্যা winRandomness(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nBuf, অক্ষর *zBuf){
#যদি defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS)
  UNUSED_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  ফেরত nBuf;
#অন্যথা
  EntropyGatherer e;
  UNUSED_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  e.a = (unsigned অক্ষর*)zBuf;
  e.na = nBuf;
  e.nXor = 0;
  e.i = 0;
  {
    FILETIME x;
    osGetSystemTimeAsFileTime(&x);
    xorMemory(&e, (unsigned অক্ষর*)&x, আকার(x));
  }
  {
    DWORD pid = osGetCurrentProcessId();
    xorMemory(&e, (unsigned অক্ষর*)&pid, আকার(pid));
  }
  {
    ULONGLONG cnt = osGetTickCount64();
    xorMemory(&e, (unsigned অক্ষর*)&cnt, আকার(cnt));
  }
  {
    LARGE_INTEGER i;
    os_Query_Performance_গণক(&i);
    xorMemory(&e, (unsigned অক্ষর*)&i, আকার(i));
  }
#যদি SQLITE_WIN32_USE_UUID
#ifdef SQLITE_UWP
# error SQLITE_WIN32_USE_UUID is incompatible with SQLITE_UWP
#endif
  {
    UUID id;
    memset(&id, 0, আকার(id));
    osUuidCreate(&id);
    xorMemory(&e, (unsigned অক্ষর*)&id, আকার(id));
    memset(&id, 0, আকার(UUID));
    osUuidCreateSequential(&id);
    xorMemory(&e, (unsigned অক্ষর*)&id, আকার(id));
  }
#endif /* SQLITE_WIN32_USE_UUID */
  ফেরত e.nXor>nBuf ? nBuf : e.nXor;
#endif /* defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS) */
}


/*
** Sleep for a little while.  Return the amount of time slept.
*/
স্থির পূর্ণসংখ্যা winSleep(sqlite3_vfs *pVfs, পূর্ণসংখ্যা microsec){
  osSleep((microsec+999)/1000);
  UNUSED_PARAMETER(pVfs);
  ফেরত ((microsec+999)/1000)*1000;
}

/*
** The following variable, if set to a non-zero value, is interpreted as
** the number of seconds since 1970 and is used to set the result of
** sqlite3OsCurrentTime() during testing.
*/
#ifdef SQLITE_TEST
পূর্ণসংখ্যা sqlite3_current_time = 0;  /* Fake system time in seconds since 1970. */
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
স্থির পূর্ণসংখ্যা winCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *piNow){
  /* FILETIME structure is a 64-bit value representing the number of
     100-nanosecond intervals since January 1, 1601 (= JD 2305813.5).
  */
  FILETIME ft;
  স্থির ধ্রুবক sqlite3_int64 winFiletimeEpoch = 23058135*(sqlite3_int64)8640000;
#ifdef SQLITE_TEST
  স্থির ধ্রুবক sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
#endif
  /* 2^32 - to avoid use of LL and warnings in gcc */
  স্থির ধ্রুবক sqlite3_int64 max32_Bit_মান =
      (sqlite3_int64)2000000000 + (sqlite3_int64)2000000000 +
      (sqlite3_int64)294967296;

  osGetSystemTimeAsFileTime( &ft );
  *piNow = winFiletimeEpoch +
            ((((sqlite3_int64)ft.dw_উপরের_Date_Time)*max32_Bit_মান) +
               (sqlite3_int64)ft.dw_নিচের_Date_Time)/(sqlite3_int64)10000;

#ifdef SQLITE_TEST
  যদি( sqlite3_current_time ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_time + unixEpoch;
  }
#endif
  UNUSED_PARAMETER(pVfs);
  ফেরত SQLITE_OK;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
স্থির পূর্ণসংখ্যা winCurrentTime(sqlite3_vfs *pVfs, দ্বিগুণ *prNow){
  পূর্ণসংখ্যা rc;
  sqlite3_int64 i;
  rc = winCurrentTimeInt64(pVfs, &i);
  যদি( !rc ){
    *prNow = i/86400000.0;
  }
  ফেরত rc;
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
স্থির পূর্ণসংখ্যা winGetLastError(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nBuf, অক্ষর *zBuf){
  DWORD e = osGetLastError();
  UNUSED_PARAMETER(pVfs);
  যদি( nBuf>0 ) win_Get_Last_Error_বার্তা(e, nBuf, zBuf);
  ফেরত e;
}

/*
** Initialize and deinitialize the operating system interface.
*/
পূর্ণসংখ্যা sqlite3_os_init(শূন্য){
  স্থির sqlite3_vfs winVfs = {
    3,                     /* iVersion */
    আকার(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_PATH_BYTES, /* mxPathname */
    0,                     /* pNext */
    "win32",               /* zName */
    &win_App_তথ্য,           /* pAppData */
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
    win_পরবর্তী_System_Call,     /* xNextSystemCall */
  };
  স্থির sqlite3_vfs winLongPathVfs = {
    3,                     /* iVersion */
    আকার(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_PATH_BYTES, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath",      /* zName */
    &win_App_তথ্য,           /* pAppData */
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
    win_পরবর্তী_System_Call,     /* xNextSystemCall */
  };
  স্থির sqlite3_vfs winNolockVfs = {
    3,                     /* iVersion */
    আকার(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_PATH_BYTES, /* mxPathname */
    0,                     /* pNext */
    "win32-none",          /* zName */
    &win_Nolock_App_তথ্য,     /* pAppData */
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
    win_পরবর্তী_System_Call,     /* xNextSystemCall */
  };
  স্থির sqlite3_vfs winLongPathNolockVfs = {
    3,                     /* iVersion */
    আকার(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_PATH_BYTES, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath-none", /* zName */
    &win_Nolock_App_তথ্য,     /* pAppData */
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
    win_পরবর্তী_System_Call,     /* xNextSystemCall */
  };

  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  প্রতিজ্ঞা( Array_আকার(aSyscall)==60 );
  প্রতিজ্ঞা( strcmp(aSyscall[0].zName,"AreFileApisANSI")==0 );
  প্রতিজ্ঞা( strcmp(aSyscall[8].zName,"FreeLibrary")==0 );
  প্রতিজ্ঞা( strcmp(aSyscall[16].zName,"GetSystemInfo")==0 );
  প্রতিজ্ঞা( strcmp(aSyscall[24].zName,"HeapReAlloc")==0 );
  প্রতিজ্ঞা( strcmp(aSyscall[32].zName,"MapViewOfFileFromApp")==0 );
  প্রতিজ্ঞা( strcmp(aSyscall[40].zName,"UnmapViewOfFile")==0 );
  প্রতিজ্ঞা( strcmp(aSyscall[48].zName,"UuidCreate")==0 );
  প্রতিজ্ঞা( strcmp(aSyscall[56].zName,"lstat")==0 );

  /* get memory map allocation granularity */
  memset(&winSysInfo, 0, আকার(SYSTEM_INFO));
  osGetSystemInfo(&winSysInfo);
  প্রতিজ্ঞা( winSysInfo.dwAllocationGranularity>0 );
  প্রতিজ্ঞা( winSysInfo.dw_Page_আকার>0 );

  sqlite3_vfs_register(&winVfs, 1);
  sqlite3_vfs_register(&winLongPathVfs, 0);
  sqlite3_vfs_register(&winNolockVfs, 0);
  sqlite3_vfs_register(&winLongPathNolockVfs, 0);
#ifndef SQLITE_OMIT_WAL
  winBigLock = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif

  ফেরত SQLITE_OK;
}

পূর্ণসংখ্যা sqlite3_os_end(শূন্য){
#ifndef SQLITE_OMIT_WAL
  winBigLock = 0;
#endif
  ফেরত SQLITE_OK;
}

#endif /* SQLITE_OS_WIN */
