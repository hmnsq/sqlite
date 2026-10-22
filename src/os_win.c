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
#અન્તર્ભાવ "sqliteInt.h"
#જો SQLITE_OS_WIN               /* This file is used for Windows only */

/*
** Include code that is common to all os_*.c files
*/
#અન્તર્ભાવ "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#અન્તર્ભાવ "os_win.h"

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
                            (કદ(WCHAR) * SQLITE_WINNT_MAX_PATH_CHARS)
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
#ifndef UNUSED_VARIABLE_કિંમત
#  define UNUSED_VARIABLE_કિંમત(x)      (શૂન્ય)(x)
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
પ્રકારવ્યાખ્યા સંરચના winShm winShm;           /* A connection to shared-memory */
પ્રકારવ્યાખ્યા સંરચના win_Shm_ગાંઠ win_Shm_ગાંઠ;   /* A region of shared-memory */
#endif

/*
** The winFile structure is a subclass of sqlite3_file* specific to the win32
** portability layer.
*/
પ્રકારવ્યાખ્યા સંરચના winFile winFile;
સંરચના winFile {
  અચલ sqlite3_io_methods *pMethod; /*** Must be first ***/
  sqlite3_vfs *pVfs;      /* The VFS used to open this file */
  HANDLE h;               /* Handle for accessing the file */
  u8 locktype;            /* Type of lock currently held on this file */
  short sharedLockByte;   /* Randomly chosen byte used as a shared lock */
  u8 ctrlFlags;           /* Flags.  See WINFILE_* below */
  DWORD lastErrno;        /* The Windows errno from the last I/O error */
#ifndef SQLITE_OMIT_WAL
  winShm *pShm;           /* Instance of shared memory on this file */
#endif
  અચલ અક્ષર *zPath;      /* Full pathname of this file */
  પૂર્ણાંક szChunk;            /* Chunk size configured by FCNTL_CHUNK_SIZE */
#જો SQLITE_MAX_MMAP_SIZE>0
  પૂર્ણાંક nFetchOut;                /* Number of outstanding xFetch references */
  HANDLE hMap;                  /* Handle for accessing memory mapping */
  શૂન્ય *pMapRegion;             /* Area memory mapped */
  sqlite3_int64 mmapSize;       /* Size of mapped region */
  sqlite3_int64 mmapSizeMax;    /* Configured FCNTL_MMAP_SIZE value */
#endif
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  DWORD iBusyTimeout;        /* Wait this many millisec on locks */
  પૂર્ણાંક bBlockOnConnect;
#endif
};

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
# define winFileBusyTimeout(pDbFd) pDbFd->iBusyTimeout
#નહીંતર
# define winFileBusyTimeout(pDbFd) 0
#endif

/*
** The winVfsAppData structure is used for the pAppData member for all of the
** Win32 VFS variants.
*/
પ્રકારવ્યાખ્યા સંરચના winVfsAppData winVfsAppData;
સંરચના winVfsAppData {
  અચલ sqlite3_io_methods *pMethod; /* The file I/O methods to use. */
  શૂન્ય *pAppData;                    /* The extra pAppData, if any. */
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
#  define SQLITE_WIN32_DBG_BUF_SIZE   ((પૂર્ણાંક)(4096-કદ(DWORD)))
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
#  જો SQLITE_DEFAULT_CACHE_SIZE>=0
#    define SQLITE_WIN32_CACHE_SIZE     (SQLITE_DEFAULT_CACHE_SIZE)
#  નહીંતર
#    define SQLITE_WIN32_CACHE_SIZE     (-(SQLITE_DEFAULT_CACHE_SIZE))
#  endif
#endif

/*
 * Make sure that the calculated cache size, in pages, cannot cause the
 * initial size of the Win32-specific heap to exceed the maximum amount
 * of memory that can be specified in the call to HeapCreate.
 */
#જો SQLITE_WIN32_CACHE_SIZE>SQLITE_WIN32_MAX_CACHE_SIZE
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
પ્રકારવ્યાખ્યા સંરચના winMemData winMemData;
સંરચના winMemData {
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

સ્થિર સંરચના winMemData win_mem_data = {
#ifndef NDEBUG
  WINMEM_MAGIC1,
#endif
  ખાલી, FALSE
#ifndef NDEBUG
  ,WINMEM_MAGIC2
#endif
};

#ifndef NDEBUG
#define winMemAssertMagic1() પ્રતિજ્ઞા( win_mem_data.magic1==WINMEM_MAGIC1 )
#define winMemAssertMagic2() પ્રતિજ્ઞા( win_mem_data.magic2==WINMEM_MAGIC2 )
#define winMemAssertMagic()  winMemAssertMagic1(); winMemAssertMagic2();
#નહીંતર
#define winMemAssertMagic()
#endif

#define winMemGetDataPtr()  &win_mem_data
#define winMemGetHeap()     win_mem_data.hHeap
#define winMemGetOwned()    win_mem_data.bOwned

સ્થિર શૂન્ય *winMemMalloc(પૂર્ણાંક nBytes);
સ્થિર શૂન્ય winMemFree(શૂન્ય *pPrior);
સ્થિર શૂન્ય *winMemRealloc(શૂન્ય *pPrior, પૂર્ણાંક nBytes);
સ્થિર પૂર્ણાંક winMemSize(શૂન્ય *p);
સ્થિર પૂર્ણાંક winMemRoundup(પૂર્ણાંક n);
સ્થિર પૂર્ણાંક winMemInit(શૂન્ય *pAppData);
સ્થિર શૂન્ય winMemShutdown(શૂન્ય *pAppData);

અચલ sqlite3_mem_methods *sqlite3MemGetWin32(શૂન્ય);
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
સ્થિર સંરચના win_syscall {
  અચલ અક્ષર *zName;            /* Name of the system call */
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

#જો defined(SQLITE_UWP)
  { "CreateFileMappingFromApp",(SYSCALL)CreateFileMappingFromApp,0 },
#નહીંતર
  { "CreateFileMappingFromApp",(SYSCALL)0,                       0 },
#endif
#define osCreateFileMappingFromApp ((HANDLE(WINAPI*)(HANDLE,\
        PSECURITY_ATTRIBUTES, \
        ULONG,ULONG64,PCWSTR))aSyscall[3].pCurrent)

#જો !defined(SQLITE_UWP)
  { "CreateFileMappingW",      (SYSCALL)CreateFileMappingW,      0 },
#નહીંતર
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

#જો !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "FreeLibrary",             (SYSCALL)FreeLibrary,             0 },
#નહીંતર
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

#જો !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "GetProcAddressA",         (SYSCALL)GetProcAddress,          0 },
#નહીંતર
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
#નહીંતર
  { "GetTempPathW",            (SYSCALL)GetTempPathW,            0 },
#endif
#define osGetTempPathW ((DWORD(WINAPI*)(DWORD,LPWSTR))aSyscall[18].pCurrent)

  { "GetTickCount64",          (SYSCALL)GetTickCount64,          0 },
#define osGetTickCount64 ((ULONGLONG(WINAPI*)(VOID))aSyscall[19].pCurrent)

#ifdef SQLITE_UWP
  { "HeapAlloc",               (SYSCALL)0,                       0 },
#નહીંતર
  { "HeapAlloc",               (SYSCALL)HeapAlloc,               0 },
#endif
#define osHeapAlloc ((LPVOID(WINAPI*)(HANDLE,DWORD, \
        SIZE_T))aSyscall[20].pCurrent)

#ifdef SQLITE_UWP
  { "HeapCreate",              (SYSCALL)0,                       0 },
#નહીંતર
  { "HeapCreate",              (SYSCALL)HeapCreate,              0 },
#endif
#define osHeapCreate ((HANDLE(WINAPI*)(DWORD,SIZE_T, \
        SIZE_T))aSyscall[21].pCurrent)

#ifdef SQLITE_UWP
  { "HeapDestroy",             (SYSCALL)0,                       0 },
#નહીંતર
  { "HeapDestroy",             (SYSCALL)HeapDestroy,             0 },
#endif
#define osHeapDestroy ((BOOL(WINAPI*)(HANDLE))aSyscall[22].pCurrent)

#ifdef SQLITE_UWP
  { "HeapFree",                (SYSCALL)0,                       0 },
#નહીંતર
  { "HeapFree",                (SYSCALL)HeapFree,                0 },
#endif
#define osHeapFree ((BOOL(WINAPI*)(HANDLE,DWORD,LPVOID))aSyscall[23].pCurrent)

#ifdef SQLITE_UWP
  { "HeapReAlloc",             (SYSCALL)0,                       0 },
#નહીંતર
  { "HeapReAlloc",             (SYSCALL)HeapReAlloc,             0 },
#endif
#define osHeapReAlloc ((LPVOID(WINAPI*)(HANDLE,DWORD,LPVOID, \
        SIZE_T))aSyscall[24].pCurrent)

#ifdef SQLITE_UWP
  { "HeapSize",                (SYSCALL)0,                       0 },
#નહીંતર
  { "HeapSize",                (SYSCALL)HeapSize,                0 },
#endif
#define osHeapSize ((SIZE_T(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[25].pCurrent)

#ifdef SQLITE_UWP
  { "HeapValidate",            (SYSCALL)0,                       0 },
#નહીંતર
  { "HeapValidate",            (SYSCALL)HeapValidate,            0 },
#endif
#define osHeapValidate ((BOOL(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[26].pCurrent)

#ifdef SQLITE_UWP
  { "HeapCompact",             (SYSCALL)0,                       0 },
#નહીંતર
  { "HeapCompact",             (SYSCALL)HeapCompact,             0 },
#endif
#define osHeapCompact ((UINT(WINAPI*)(HANDLE,DWORD))aSyscall[27].pCurrent)


#જો !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "LoadLibraryW",            (SYSCALL)LoadLibraryW,            0 },
#નહીંતર
  { "LoadLibraryW",            (SYSCALL)0,                       0 },
#endif
#define osLoadLibraryW ((HMODULE(WINAPI*)(LPCWSTR))aSyscall[28].pCurrent)

  { "LocalFree",               (SYSCALL)LocalFree,               0 },
#define osLocalFree ((HLOCAL(WINAPI*)(HLOCAL))aSyscall[29].pCurrent)

  { "LockFileEx",              (SYSCALL)LockFileEx,              0 },
#define osLockFileEx ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[30].pCurrent)

#જો (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0) \
 && !defined(SQLITE_UWP)
  { "MapViewOfFile",           (SYSCALL)MapViewOfFile,           0 },
#નહીંતર
  { "MapViewOfFile",           (SYSCALL)0,                       0 },
#endif
#define osMapViewOfFile ((LPVOID(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        SIZE_T))aSyscall[31].pCurrent)

#જો (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0) \
 && defined(SQLITE_UWP)
  { "MapViewOfFileFromApp",    (SYSCALL)MapViewOfFileFromApp,   0 },
#નહીંતર
  { "MapViewOfFileFromApp",    (SYSCALL)0,                      0 },
#endif
#define osMapViewOfFileFromApp ((LPVOID(WINAPI*)(HANDLE,ULONG,ULONG64, \
        SIZE_T))aSyscall[32].pCurrent)

  { "MultiByteToWideChar",     (SYSCALL)MultiByteToWideChar,     0 },
#define osMultiByteToWideChar ((પૂર્ણાંક(WINAPI*)(UINT,DWORD,LPCSTR,પૂર્ણાંક,LPWSTR, \
        પૂર્ણાંક))aSyscall[33].pCurrent)

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

#જો !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0
  { "UnmapViewOfFile",         (SYSCALL)UnmapViewOfFile,         0 },
#નહીંતર
  { "UnmapViewOfFile",         (SYSCALL)0,                       0 },
#endif
#define osUnmapViewOfFile ((BOOL(WINAPI*)(LPCVOID))aSyscall[40].pCurrent)

  { "WideCharToMultiByte",     (SYSCALL)WideCharToMultiByte,     0 },
#define osWideCharToMultiByte ((પૂર્ણાંક(WINAPI*)(UINT,DWORD,LPCWSTR,પૂર્ણાંક,LPSTR,પૂર્ણાંક, \
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
#જો defined(InterlockedCompareExchange)
  { "InterlockedCompareExchange", (SYSCALL)0,                    0 },
#define osInterlockedCompareExchange InterlockedCompareExchange
#નહીંતર
  { "InterlockedCompareExchange", (SYSCALL)InterlockedCompareExchange, 0 },
#define osInterlockedCompareExchange ((LONG(WINAPI*)(LONG અસ્થિર*,\
        LONG,LONG))aSyscall[47].pCurrent)
#endif /* defined(InterlockedCompareExchange) */

#જો SQLITE_WIN32_USE_UUID
  { "UuidCreate",               (SYSCALL)UuidCreate,             0 },
#નહીંતર
  { "UuidCreate",               (SYSCALL)0,                      0 },
#endif
#define osUuidCreate ((RPC_STATUS(RPC_ENTRY*)(UUID*))aSyscall[48].pCurrent)

#જો SQLITE_WIN32_USE_UUID
  { "UuidCreateSequential",     (SYSCALL)UuidCreateSequential,   0 },
#નહીંતર
  { "UuidCreateSequential",     (SYSCALL)0,                      0 },
#endif
#define osUuidCreateSequential \
        ((RPC_STATUS(RPC_ENTRY*)(UUID*))aSyscall[49].pCurrent)

#જો !defined(SQLITE_NO_SYNC) && SQLITE_MAX_MMAP_SIZE>0
  { "FlushViewOfFile",          (SYSCALL)FlushViewOfFile,        0 },
#નહીંતર
  { "FlushViewOfFile",          (SYSCALL)0,                      0 },
#endif
#define osFlushViewOfFile \
        ((BOOL(WINAPI*)(LPCVOID,SIZE_T))aSyscall[50].pCurrent)

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  { "CreateEvent",              (SYSCALL)CreateEvent,            0 },
#નહીંતર
  { "CreateEvent",              (SYSCALL)0,                      0 },
#endif
#define osCreateEvent ((HANDLE(WINAPI*)(LPSECURITY_ATTRIBUTES,BOOL, \
                        BOOL,LPCSTR))aSyscall[51].pCurrent)

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  { "CancelIo",                 (SYSCALL)CancelIo,               0 },
#નહીંતર
  { "CancelIo",                 (SYSCALL)0,                      0 },
#endif
#define osCancelIo ((BOOL(WINAPI*)(HANDLE))aSyscall[52].pCurrent)

#ifndef _WIN32
  { "getenv",                   (SYSCALL)getenv,                 0 },
#નહીંતર
  { "getenv",                   (SYSCALL)0,                      0 },
#endif
#define osGetenv ((અચલ અક્ષર *(*)(અચલ અક્ષર *))aSyscall[53].pCurrent)

#ifndef _WIN32
  { "getcwd",                   (SYSCALL)getcwd,                 0 },
#નહીંતર
  { "getcwd",                   (SYSCALL)0,                      0 },
#endif
#define osGetcwd ((અક્ષર*(*)(અક્ષર*,size_t))aSyscall[54].pCurrent)

#ifndef _WIN32
  { "readlink",                 (SYSCALL)readlink,               0 },
#નહીંતર
  { "readlink",                 (SYSCALL)0,                      0 },
#endif
#define osReadlink ((ssize_t(*)(અચલ અક્ષર*,અક્ષર*,size_t))aSyscall[55].pCurrent)

#ifndef _WIN32
  { "lstat",                    (SYSCALL)lstat,                  0 },
#નહીંતર
  { "lstat",                    (SYSCALL)0,                      0 },
#endif
#define osLstat ((પૂર્ણાંક(*)(અચલ અક્ષર*,સંરચના stat*))aSyscall[56].pCurrent)

#ifndef _WIN32
  { "__errno",                  (SYSCALL)__errno,                0 },
#નહીંતર
  { "__errno",                  (SYSCALL)0,                      0 },
#endif
#define osErrno (*((પૂર્ણાંક*(*)(શૂન્ય))aSyscall[57].pCurrent)())

#ifndef _WIN32
  { "cygwin_conv_path",         (SYSCALL)cygwin_conv_path,       0 },
#નહીંતર
  { "cygwin_conv_path",         (SYSCALL)0,                      0 },
#endif
#define osCygwin_conv_path ((size_t(*)(unsigned પૂર્ણાંક, \
    અચલ શૂન્ય *, શૂન્ય *, size_t))aSyscall[58].pCurrent)

  { "GetDriveTypeW",            (SYSCALL)GetDriveTypeW,          0 },
#define osGetDriveTypeW ((UINT(WINAPI*)(LPCWSTR))aSyscall[59].pCurrent)

}; /* End of the overrideable system calls */

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "win32" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
સ્થિર પૂર્ણાંક winSetSystemCall(
  sqlite3_vfs *pNotUsed,        /* The VFS pointer.  Not used */
  અચલ અક્ષર *zName,            /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
){
  unsigned પૂર્ણાંક i;
  પૂર્ણાંક rc = SQLITE_NOTFOUND;

  UNUSED_PARAMETER(pNotUsed);
  જો( zName==0 ){
    /* If no zName is given, restore all system calls to their default
    ** settings and return NULL
    */
    rc = SQLITE_OK;
    માટે(i=0; i<કદ(aSyscall)/કદ(aSyscall[0]); i++){
      જો( aSyscall[i].pDefault ){
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  }નહીંતર{
    /* If zName is specified, operate on only the one system call
    ** specified.
    */
    માટે(i=0; i<કદ(aSyscall)/કદ(aSyscall[0]); i++){
      જો( strcmp(zName, aSyscall[i].zName)==0 ){
        જો( aSyscall[i].pDefault==0 ){
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = SQLITE_OK;
        જો( pNewFunc==0 ) pNewFunc = aSyscall[i].pDefault;
        aSyscall[i].pCurrent = pNewFunc;
        તોડો;
      }
    }
  }
  પાછા rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
સ્થિર sqlite3_syscall_ptr winGetSystemCall(
  sqlite3_vfs *pNotUsed,
  અચલ અક્ષર *zName
){
  unsigned પૂર્ણાંક i;

  UNUSED_PARAMETER(pNotUsed);
  માટે(i=0; i<કદ(aSyscall)/કદ(aSyscall[0]); i++){
    જો( strcmp(zName, aSyscall[i].zName)==0 ) પાછા aSyscall[i].pCurrent;
  }
  પાછા 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
સ્થિર અચલ અક્ષર *winNextSystemCall(sqlite3_vfs *p, અચલ અક્ષર *zName){
  પૂર્ણાંક i = -1;

  UNUSED_PARAMETER(p);
  જો( zName ){
    માટે(i=0; i<ArraySize(aSyscall)-1; i++){
      જો( strcmp(zName, aSyscall[i].zName)==0 ) તોડો;
    }
  }
  માટે(i++; i<ArraySize(aSyscall); i++){
    જો( aSyscall[i].pCurrent!=0 ) પાછા aSyscall[i].zName;
  }
  પાછા 0;
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
પૂર્ણાંક sqlite3_win32_compact_heap(LPUINT pnLargest){
  પૂર્ણાંક rc = SQLITE_OK;
  UINT nLargest = 0;
  HANDLE hHeap;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  પ્રતિજ્ઞા( hHeap!=0 );
  પ્રતિજ્ઞા( hHeap!=INVALID_HANDLE_કિંમત );
#જો defined(SQLITE_WIN32_MALLOC_VALIDATE)
  પ્રતિજ્ઞા( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, ખાલી) );
#endif
  જો( (nLargest=osHeapCompact(hHeap, SQLITE_WIN32_HEAP_FLAGS))==0 ){
    DWORD lastErrno = osGetLastError();
    જો( lastErrno==NO_ERROR ){
      sqlite3_log(SQLITE_NOMEM, "failed to HeapCompact (no space), heap=%p",
                  (શૂન્ય*)hHeap);
      rc = SQLITE_NOMEM_BKPT;
    }નહીંતર{
      sqlite3_log(SQLITE_ERROR, "failed to HeapCompact (%lu), heap=%p",
                  osGetLastError(), (શૂન્ય*)hHeap);
      rc = SQLITE_ERROR;
    }
  }
  જો( pnLargest ) *pnLargest = nLargest;
  પાછા rc;
}

/*
** If a Win32 native heap has been configured, this function will attempt to
** destroy and recreate it.  If the Win32 native heap is not isolated and/or
** the sqlite3_memory_used() function does not return zero, SQLITE_BUSY will
** be returned and no changes will be made to the Win32 native heap.
*/
પૂર્ણાંક sqlite3_win32_reset_heap(){
  પૂર્ણાંક rc;
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; ) /* The main static mutex */
  MUTEX_LOGIC( sqlite3_mutex *pMem; )    /* The memsys static mutex */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  MUTEX_LOGIC( pMem = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3_mutex_enter(pMem);
  winMemAssertMagic();
  જો( winMemGetHeap()!=ખાલી && winMemGetOwned() && sqlite3_memory_used()==0 ){
    /*
    ** At this point, there should be no outstanding memory allocations on
    ** the heap.  Also, since both the main and memsys locks are currently
    ** being held by us, no other function (i.e. from another thread) should
    ** be able to even access the heap.  Attempt to destroy and recreate our
    ** isolated Win32 native heap now.
    */
    પ્રતિજ્ઞા( winMemGetHeap()!=ખાલી );
    પ્રતિજ્ઞા( winMemGetOwned() );
    પ્રતિજ્ઞા( sqlite3_memory_used()==0 );
    winMemShutdown(winMemGetDataPtr());
    પ્રતિજ્ઞા( winMemGetHeap()==ખાલી );
    પ્રતિજ્ઞા( !winMemGetOwned() );
    પ્રતિજ્ઞા( sqlite3_memory_used()==0 );
    rc = winMemInit(winMemGetDataPtr());
    પ્રતિજ્ઞા( rc!=SQLITE_OK || winMemGetHeap()!=ખાલી );
    પ્રતિજ્ઞા( rc!=SQLITE_OK || winMemGetOwned() );
    પ્રતિજ્ઞા( rc!=SQLITE_OK || sqlite3_memory_used()==0 );
  }નહીંતર{
    /*
    ** The Win32 native heap cannot be modified because it may be in use.
    */
    rc = SQLITE_BUSY;
  }
  sqlite3_mutex_leave(pMem);
  sqlite3_mutex_leave(pMainMtx);
  પાછા rc;
}
#endif /* SQLITE_WIN32_MALLOC */

#ifdef _WIN32
/*
** This function outputs the specified (ANSI) string to the Win32 debugger
** (if available).  Undocumented.  Might go away at any moment.
*/
શૂન્ય sqlite3_win32_write_debug(અચલ અક્ષર *zBuf, પૂર્ણાંક nBuf){
  અક્ષર zDbgBuf[SQLITE_WIN32_DBG_BUF_SIZE];
  પૂર્ણાંક nMin = MIN(nBuf, (SQLITE_WIN32_DBG_BUF_SIZE - 1)); /* may be negative. */
  જો( nMin<-1 ) nMin = -1; /* all negative values become -1. */
  પ્રતિજ્ઞા( nMin==-1 || nMin==0 || nMin<SQLITE_WIN32_DBG_BUF_SIZE );
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !zBuf ){
    (શૂન્ય)SQLITE_MISUSE_BKPT;
    પાછા;
  }
#endif
  જો( nMin>0 ){
    memset(zDbgBuf, 0, SQLITE_WIN32_DBG_BUF_SIZE);
    memcpy(zDbgBuf, zBuf, nMin);
    osOutputDebugStringA(zDbgBuf);
  }નહીંતર{
    osOutputDebugStringA(zBuf);
  }
}

શૂન્ય sqlite3_win32_sleep(DWORD milliseconds){
  osSleep(milliseconds);
}

#જો SQLITE_MAX_WORKER_THREADS>0 && SQLITE_THREADSAFE>0
DWORD sqlite3Win32Wait(HANDLE hObject){
  DWORD rc;
  જ્યારે( (rc = osWaitForSingleObjectEx(hObject, INFINITE,
                                       TRUE))==WAIT_IO_COMPLETION ){}
  પાછા rc;
}
#endif
#endif /* _WIN32 */

#ifdef SQLITE_WIN32_MALLOC
/*
** Allocate nBytes of memory.
*/
સ્થિર શૂન્ય *winMemMalloc(પૂર્ણાંક nBytes){
  HANDLE hHeap;
  શૂન્ય *p;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  પ્રતિજ્ઞા( hHeap!=0 );
  પ્રતિજ્ઞા( hHeap!=INVALID_HANDLE_કિંમત );
#જો defined(SQLITE_WIN32_MALLOC_VALIDATE)
  પ્રતિજ્ઞા( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, ખાલી) );
#endif
  પ્રતિજ્ઞા( nBytes>=0 );
  p = osHeapAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, (SIZE_T)nBytes);
  જો( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapAlloc %u bytes (%lu), heap=%p",
                nBytes, osGetLastError(), (શૂન્ય*)hHeap);
  }
  પાછા p;
}

/*
** Free memory.
*/
સ્થિર શૂન્ય winMemFree(શૂન્ય *pPrior){
  HANDLE hHeap;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  પ્રતિજ્ઞા( hHeap!=0 );
  પ્રતિજ્ઞા( hHeap!=INVALID_HANDLE_કિંમત );
#જો defined(SQLITE_WIN32_MALLOC_VALIDATE)
  પ્રતિજ્ઞા( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) );
#endif
  જો( !pPrior ) પાછા; /* Passing NULL to HeapFree is undefined. */
  જો( !osHeapFree(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapFree block %p (%lu), heap=%p",
                pPrior, osGetLastError(), (શૂન્ય*)hHeap);
  }
}

/*
** Change the size of an existing memory allocation
*/
સ્થિર શૂન્ય *winMemRealloc(શૂન્ય *pPrior, પૂર્ણાંક nBytes){
  HANDLE hHeap;
  શૂન્ય *p;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  પ્રતિજ્ઞા( hHeap!=0 );
  પ્રતિજ્ઞા( hHeap!=INVALID_HANDLE_કિંમત );
#જો defined(SQLITE_WIN32_MALLOC_VALIDATE)
  પ્રતિજ્ઞા( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) );
#endif
  પ્રતિજ્ઞા( nBytes>=0 );
  જો( !pPrior ){
    p = osHeapAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, (SIZE_T)nBytes);
  }નહીંતર{
    p = osHeapReAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior, (SIZE_T)nBytes);
  }
  જો( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to %s %u bytes (%lu), heap=%p",
                pPrior ? "HeapReAlloc" : "HeapAlloc", nBytes, osGetLastError(),
                (શૂન્ય*)hHeap);
  }
  પાછા p;
}

/*
** Return the size of an outstanding allocation, in bytes.
*/
સ્થિર પૂર્ણાંક winMemSize(શૂન્ય *p){
  HANDLE hHeap;
  SIZE_T n;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  પ્રતિજ્ઞા( hHeap!=0 );
  પ્રતિજ્ઞા( hHeap!=INVALID_HANDLE_કિંમત );
#જો defined(SQLITE_WIN32_MALLOC_VALIDATE)
  પ્રતિજ્ઞા( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, p) );
#endif
  જો( !p ) પાછા 0;
  n = osHeapSize(hHeap, SQLITE_WIN32_HEAP_FLAGS, p);
  જો( n==(SIZE_T)-1 ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapSize block %p (%lu), heap=%p",
                p, osGetLastError(), (શૂન્ય*)hHeap);
    પાછા 0;
  }
  પાછા (પૂર્ણાંક)n;
}

/*
** Round up a request size to the next valid allocation size.
*/
સ્થિર પૂર્ણાંક winMemRoundup(પૂર્ણાંક n){
  પાછા n;
}

/*
** Initialize this module.
*/
સ્થિર પૂર્ણાંક winMemInit(શૂન્ય *pAppData){
  winMemData *pWinMemData = (winMemData *)pAppData;

  જો( !pWinMemData ) પાછા SQLITE_ERROR;
  પ્રતિજ્ઞા( pWinMemData->magic1==WINMEM_MAGIC1 );
  પ્રતિજ્ઞા( pWinMemData->magic2==WINMEM_MAGIC2 );

#જો SQLITE_WIN32_HEAP_CREATE
  જો( !pWinMemData->hHeap ){
    DWORD dwInitialSize = SQLITE_WIN32_HEAP_INIT_SIZE;
    DWORD dwMaximumSize = (DWORD)sqlite3GlobalConfig.nHeap;
    જો( dwMaximumSize==0 ){
      dwMaximumSize = SQLITE_WIN32_HEAP_MAX_SIZE;
    }નહીંતર જો( dwInitialSize>dwMaximumSize ){
      dwInitialSize = dwMaximumSize;
    }
    pWinMemData->hHeap = osHeapCreate(SQLITE_WIN32_HEAP_FLAGS,
                                      dwInitialSize, dwMaximumSize);
    જો( !pWinMemData->hHeap ){
      sqlite3_log(SQLITE_NOMEM,
          "failed to HeapCreate (%lu), flags=%u, initSize=%lu, maxSize=%lu",
          osGetLastError(), SQLITE_WIN32_HEAP_FLAGS, dwInitialSize,
          dwMaximumSize);
      પાછા SQLITE_NOMEM_BKPT;
    }
    pWinMemData->bOwned = TRUE;
    પ્રતિજ્ઞા( pWinMemData->bOwned );
  }
#નહીંતર
  pWinMemData->hHeap = osGetProcessHeap();
  જો( !pWinMemData->hHeap ){
    sqlite3_log(SQLITE_NOMEM,
        "failed to GetProcessHeap (%lu)", osGetLastError());
    પાછા SQLITE_NOMEM_BKPT;
  }
  pWinMemData->bOwned = FALSE;
  પ્રતિજ્ઞા( !pWinMemData->bOwned );
#endif
  પ્રતિજ્ઞા( pWinMemData->hHeap!=0 );
  પ્રતિજ્ઞા( pWinMemData->hHeap!=INVALID_HANDLE_કિંમત );
#જો defined(SQLITE_WIN32_MALLOC_VALIDATE)
  પ્રતિજ્ઞા( osHeapValidate(pWinMemData->hHeap, SQLITE_WIN32_HEAP_FLAGS, ખાલી) );
#endif
  પાછા SQLITE_OK;
}

/*
** Deinitialize this module.
*/
સ્થિર શૂન્ય winMemShutdown(શૂન્ય *pAppData){
  winMemData *pWinMemData = (winMemData *)pAppData;

  જો( !pWinMemData ) પાછા;
  પ્રતિજ્ઞા( pWinMemData->magic1==WINMEM_MAGIC1 );
  પ્રતિજ્ઞા( pWinMemData->magic2==WINMEM_MAGIC2 );

  જો( pWinMemData->hHeap ){
    પ્રતિજ્ઞા( pWinMemData->hHeap!=INVALID_HANDLE_કિંમત );
#જો defined(SQLITE_WIN32_MALLOC_VALIDATE)
    પ્રતિજ્ઞા( osHeapValidate(pWinMemData->hHeap, SQLITE_WIN32_HEAP_FLAGS, ખાલી) );
#endif
    જો( pWinMemData->bOwned ){
      જો( !osHeapDestroy(pWinMemData->hHeap) ){
        sqlite3_log(SQLITE_NOMEM, "failed to HeapDestroy (%lu), heap=%p",
                    osGetLastError(), (શૂન્ય*)pWinMemData->hHeap);
      }
      pWinMemData->bOwned = FALSE;
    }
    pWinMemData->hHeap = ખાલી;
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
અચલ sqlite3_mem_methods *sqlite3MemGetWin32(શૂન્ય){
  સ્થિર અચલ sqlite3_mem_methods winMemMethods = {
    winMemMalloc,
    winMemFree,
    winMemRealloc,
    winMemSize,
    winMemRoundup,
    winMemInit,
    winMemShutdown,
    &win_mem_data
  };
  પાછા &winMemMethods;
}

શૂન્ય sqlite3MemSetDefault(શૂન્ય){
  sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetWin32());
}
#endif /* SQLITE_WIN32_MALLOC */

#ifdef _WIN32
/*
** Convert a UTF-8 string to Microsoft Unicode.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
સ્થિર LPWSTR winUtf8ToUnicode(અચલ અક્ષર *zText){
  પૂર્ણાંક nChar;
  LPWSTR zWideText;

  nChar = osMultiByteToWideChar(CP_UTF8, 0, zText, -1, ખાલી, 0);
  જો( nChar==0 ){
    પાછા 0;
  }
  zWideText = sqlite3MallocZero( nChar*કદ(WCHAR) );
  જો( zWideText==0 ){
    પાછા 0;
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zText, -1, zWideText,
                                nChar);
  જો( nChar==0 ){
    sqlite3_free(zWideText);
    zWideText = 0;
  }
  પાછા zWideText;
}
#endif /* _WIN32 */

/*
** Convert a Microsoft Unicode string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
સ્થિર અક્ષર *winUnicodeToUtf8(LPCWSTR zWideText){
  પૂર્ણાંક nByte;
  અક્ષર *zText;

  nByte = osWideCharToMultiByte(CP_UTF8, 0, zWideText, -1, 0, 0, 0, 0);
  જો( nByte == 0 ){
    પાછા 0;
  }
  zText = sqlite3MallocZero( nByte );
  જો( zText==0 ){
    પાછા 0;
  }
  nByte = osWideCharToMultiByte(CP_UTF8, 0, zWideText, -1, zText, nByte,
                                0, 0);
  જો( nByte == 0 ){
    sqlite3_free(zText);
    zText = 0;
  }
  પાછા zText;
}

#ifdef _WIN32
/*
** Convert an ANSI string to Microsoft Unicode, using the ANSI or OEM
** code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
સ્થિર LPWSTR winMbcsToUnicode(અચલ અક્ષર *zText, પૂર્ણાંક useAnsi){
  પૂર્ણાંક nWideChar;
  LPWSTR zMbcsText;
  પૂર્ણાંક codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nWideChar = osMultiByteToWideChar(codepage, 0, zText, -1, ખાલી,
                                0);
  જો( nWideChar==0 ){
    પાછા 0;
  }
  zMbcsText = sqlite3MallocZero( nWideChar*કદ(WCHAR) );
  જો( zMbcsText==0 ){
    પાછા 0;
  }
  nWideChar = osMultiByteToWideChar(codepage, 0, zText, -1, zMbcsText,
                                nWideChar);
  જો( nWideChar==0 ){
    sqlite3_free(zMbcsText);
    zMbcsText = 0;
  }
  પાછા zMbcsText;
}

/*
** Convert a Microsoft Unicode string to a multi-byte character string,
** using the ANSI or OEM code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
સ્થિર અક્ષર *winUnicodeToMbcs(LPCWSTR zWideText, પૂર્ણાંક useAnsi){
  પૂર્ણાંક nByte;
  અક્ષર *zText;
  પૂર્ણાંક codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nByte = osWideCharToMultiByte(codepage, 0, zWideText, -1, 0, 0, 0, 0);
  જો( nByte == 0 ){
    પાછા 0;
  }
  zText = sqlite3MallocZero( nByte );
  જો( zText==0 ){
    પાછા 0;
  }
  nByte = osWideCharToMultiByte(codepage, 0, zWideText, -1, zText,
                                nByte, 0, 0);
  જો( nByte == 0 ){
    sqlite3_free(zText);
    zText = 0;
  }
  પાછા zText;
}

/*
** Convert a multi-byte character string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
સ્થિર અક્ષર *winMbcsToUtf8(અચલ અક્ષર *zText, પૂર્ણાંક useAnsi){
  અક્ષર *zTextUtf8;
  LPWSTR zTmpWide;

  zTmpWide = winMbcsToUnicode(zText, useAnsi);
  જો( zTmpWide==0 ){
    પાછા 0;
  }
  zTextUtf8 = winUnicodeToUtf8(zTmpWide);
  sqlite3_free(zTmpWide);
  પાછા zTextUtf8;
}

/*
** Convert a UTF-8 string to a multi-byte character string.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
સ્થિર અક્ષર *winUtf8ToMbcs(અચલ અક્ષર *zText, પૂર્ણાંક useAnsi){
  અક્ષર *zTextMbcs;
  LPWSTR zTmpWide;

  zTmpWide = winUtf8ToUnicode(zText);
  જો( zTmpWide==0 ){
    પાછા 0;
  }
  zTextMbcs = winUnicodeToMbcs(zTmpWide, useAnsi);
  sqlite3_free(zTmpWide);
  પાછા zTextMbcs;
}

/*
** This is a public wrapper for the winUtf8ToUnicode() function.
*/
LPWSTR sqlite3_win32_utf8_to_unicode(અચલ અક્ષર *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !zText ){
    (શૂન્ય)SQLITE_MISUSE_BKPT;
    પાછા 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  જો( sqlite3_initialize() ) પાછા 0;
#endif
  પાછા winUtf8ToUnicode(zText);
}

/*
** This is a public wrapper for the winUnicodeToUtf8() function.
*/
અક્ષર *sqlite3_win32_unicode_to_utf8(LPCWSTR zWideText){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !zWideText ){
    (શૂન્ય)SQLITE_MISUSE_BKPT;
    પાછા 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  જો( sqlite3_initialize() ) પાછા 0;
#endif
  પાછા winUnicodeToUtf8(zWideText);
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
અક્ષર *sqlite3_win32_mbcs_to_utf8(અચલ અક્ષર *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !zText ){
    (શૂન્ય)SQLITE_MISUSE_BKPT;
    પાછા 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  જો( sqlite3_initialize() ) પાછા 0;
#endif
  પાછા winMbcsToUtf8(zText, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
અક્ષર *sqlite3_win32_mbcs_to_utf8_v2(અચલ અક્ષર *zText, પૂર્ણાંક useAnsi){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !zText ){
    (શૂન્ય)SQLITE_MISUSE_BKPT;
    પાછા 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  જો( sqlite3_initialize() ) પાછા 0;
#endif
  પાછા winMbcsToUtf8(zText, useAnsi);
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
અક્ષર *sqlite3_win32_utf8_to_mbcs(અચલ અક્ષર *zText){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !zText ){
    (શૂન્ય)SQLITE_MISUSE_BKPT;
    પાછા 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  જો( sqlite3_initialize() ) પાછા 0;
#endif
  પાછા winUtf8ToMbcs(zText, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
અક્ષર *sqlite3_win32_utf8_to_mbcs_v2(અચલ અક્ષર *zText, પૂર્ણાંક useAnsi){
#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !zText ){
    (શૂન્ય)SQLITE_MISUSE_BKPT;
    પાછા 0;
  }
#endif
#ifndef SQLITE_OMIT_AUTOINIT
  જો( sqlite3_initialize() ) પાછા 0;
#endif
  પાછા winUtf8ToMbcs(zText, useAnsi);
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-8 string.
*/
પૂર્ણાંક sqlite3_win32_set_directory8(
  unsigned long type, /* Identifier for directory being set or reset */
  અચલ અક્ષર *z_કિંમત  /* New value for directory being set or reset */
){
  અક્ષર **ppDirectory = 0;
  પૂર્ણાંક rc;
#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  જો( rc ) પાછા rc;
#endif
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  જો( type==SQLITE_WIN32_DATA_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_data_directory;
  }નહીંતર જો( type==SQLITE_WIN32_TEMP_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_temp_directory;
  }
  પ્રતિજ્ઞા( !ppDirectory || type==SQLITE_WIN32_DATA_DIRECTORY_TYPE
          || type==SQLITE_WIN32_TEMP_DIRECTORY_TYPE
  );
  પ્રતિજ્ઞા( !ppDirectory || sqlite3MemdebugHasType(*ppDirectory, MEMTYPE_HEAP) );
  જો( ppDirectory ){
    અક્ષર *zCopy = 0;
    જો( z_કિંમત && z_કિંમત[0] ){
      zCopy = sqlite3_mprintf("%s", z_કિંમત);
      જો ( zCopy==0 ){
        rc = SQLITE_NOMEM_BKPT;
        જાઓ set_directory8_done;
      }
    }
    sqlite3_free(*ppDirectory);
    *ppDirectory = zCopy;
    rc = SQLITE_OK;
  }નહીંતર{
    rc = SQLITE_ERROR;
  }
set_directory8_done:
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  પાછા rc;
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-16 string.
*/
પૂર્ણાંક sqlite3_win32_set_directory16(
  unsigned long type, /* Identifier for directory being set or reset */
  અચલ શૂન્ય *z_કિંમત  /* New value for directory being set or reset */
){
  પૂર્ણાંક rc;
  અક્ષર *zUtf8 = 0;
  જો( z_કિંમત ){
    zUtf8 = sqlite3_win32_unicode_to_utf8(z_કિંમત);
    જો( zUtf8==0 ) પાછા SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_win32_set_directory8(type, zUtf8);
  જો( zUtf8 ) sqlite3_free(zUtf8);
  પાછા rc;
}

/*
** This function sets the data directory or the temporary directory based on
** the provided arguments.  The type argument must be 1 in order to set the
** data directory or 2 in order to set the temporary directory.  The zValue
** argument is the name of the directory to use.  The return value will be
** SQLITE_OK if successful.
*/
પૂર્ણાંક sqlite3_win32_set_directory(
  unsigned long type, /* Identifier for directory being set or reset */
  શૂન્ય *z_કિંમત        /* New value for directory being set or reset */
){
  પાછા sqlite3_win32_set_directory16(type, z_કિંમત);
}
#endif /* _WIN32 */

/*
** The return value of winGetLastErrorMsg
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated).
*/
સ્થિર પૂર્ણાંક win_Get_Last_Error_સંદેશ(DWORD lastErrno, પૂર્ણાંક nBuf, અક્ષર *zBuf){
  /* FormatMessage returns 0 on failure.  Otherwise it
  ** returns the number of TCHARs written to the output
  ** buffer, excluding the terminating null char.
  */
  DWORD dwLen = 0;
  અક્ષર *zOut = 0;
  LPWSTR zTempWide = ખાલી;
  dwLen = osFormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_FROM_SYSTEM |
                           FORMAT_MESSAGE_IGNORE_INSERTS,
                           ખાલી,
                           lastErrno,
                           0,
                           (LPWSTR) &zTempWide,
                           0,
                           0);
  જો( dwLen > 0 ){
    /* allocate a buffer and convert to UTF8 */
    sqlite3BeginBenignMalloc();
    zOut = winUnicodeToUtf8(zTempWide);
    sqlite3EndBenignMalloc();
    /* free the system buffer allocated by FormatMessage */
    osLocalFree(zTempWide);
  }
  જો( 0 == dwLen ){
    sqlite3_snprintf(nBuf, zBuf, "OsError 0x%lx (%lu)", lastErrno, lastErrno);
  }નહીંતર{
    /* copy a maximum of nBuf chars to output buffer */
    sqlite3_snprintf(nBuf, zBuf, "%s", zOut);
    /* free the UTF8 buffer */
    sqlite3_free(zOut);
  }
  પાછા 0;
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
#define winLogError(a,b,c,d)   win_Log_Error_At_લીટી(a,b,c,d,__LINE__)
સ્થિર પૂર્ણાંક win_Log_Error_At_લીટી(
  પૂર્ણાંક errcode,                    /* SQLite error code */
  DWORD lastErrno,                /* Win32 last error */
  અચલ અક્ષર *zFunc,              /* Name of OS function that failed */
  અચલ અક્ષર *zPath,              /* File path associated with error */
  પૂર્ણાંક i_લીટી                       /* Source line number where error occurred */
){
  અક્ષર z_સંદેશ[500];                 /* Human readable error text */
  પૂર્ણાંક i;                          /* Loop counter */

  z_સંદેશ[0] = 0;
  win_Get_Last_Error_સંદેશ(lastErrno, કદ(z_સંદેશ), z_સંદેશ);
  પ્રતિજ્ઞા( errcode!=SQLITE_OK );
  જો( zPath==0 ) zPath = "";
  માટે(i=0; z_સંદેશ[i] && z_સંદેશ[i]!='\r' && z_સંદેશ[i]!='\n'; i++){}
  z_સંદેશ[i] = 0;
  sqlite3_log(errcode,
      "os_win.c:%d: (%lu) %s(%s) - %s",
      i_લીટી, lastErrno, zFunc, zPath, z_સંદેશ
  );

  પાછા errcode;
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
સ્થિર પૂર્ણાંક winIoerrRetry = SQLITE_WIN32_IOERR_RETRY;
સ્થિર પૂર્ણાંક winIoerrRetryDelay = SQLITE_WIN32_IOERR_RETRY_DELAY;

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
#જો !defined(winIoerrCanRetry1)
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
સ્થિર પૂર્ણાંક winRetryIoerr(પૂર્ણાંક *pnRetry, DWORD *pError){
  DWORD e = osGetLastError();
  જો( *pnRetry>=winIoerrRetry ){
    જો( pError ){
      *pError = e;
    }
    પાછા 0;
  }
  જો( winIoerrCanRetry1(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    પાછા 1;
  }
#જો defined(winIoerrCanRetry2)
  નહીંતર જો( winIoerrCanRetry2(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    પાછા 1;
  }
#endif
  જો( pError ){
    *pError = e;
  }
  પાછા 0;
}

/*
** Log a I/O error retry episode.
*/
સ્થિર શૂન્ય winLogIoerr(પૂર્ણાંક nRetry, પૂર્ણાંક lineno){
  જો( nRetry ){
    sqlite3_log(SQLITE_NOTICE,
      "delayed %dms for lock/sharing conflict at line %d",
      winIoerrRetryDelay*nRetry*(nRetry+1)/2, lineno
    );
  }
}


/*
** Lock a file region.
*/
સ્થિર BOOL winLockFile(
  LPHANDLE phFile,
  DWORD flags,
  DWORD offset_નીચલું,
  DWORD offset_ઉપલું,
  DWORD અંક_Bytes_નીચલું,
  DWORD અંક_Bytes_ઉપલું
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, કદ(OVERLAPPED));
  ovlp.Offset = offset_નીચલું;
  ovlp.Offset_ઉપલું = offset_ઉપલું;
  પાછા osLockFileEx(*phFile, flags, 0, અંક_Bytes_નીચલું, અંક_Bytes_ઉપલું, &ovlp);
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
સ્થિર પૂર્ણાંક winHandleLockTimeout(
  HANDLE hFile,
  DWORD offset,
  DWORD nByte,
  પૂર્ણાંક bExcl,
  DWORD nMs
){
  DWORD flags = LOCKFILE_FAIL_IMMEDIATELY | (bExcl?LOCKFILE_EXCLUSIVE_LOCK:0);
  પૂર્ણાંક rc = SQLITE_OK;
  BOOL ret;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, કદ(OVERLAPPED));
  ovlp.Offset = offset;

#જો defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  જો( nMs!=0 ){
    flags &= ~LOCKFILE_FAIL_IMMEDIATELY;
  }
  ovlp.hEvent = osCreateEvent(ખાલી, TRUE, FALSE, ખાલી);
  જો( ovlp.hEvent==ખાલી ){
    પાછા SQLITE_IOERR_LOCK;
  }
#endif
  ret = osLockFileEx(hFile, flags, 0, nByte, 0, &ovlp);
#જો defined(SQLITE_ENABLE_SETLK_TIMEOUT)
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
  જો( !ret && GetLastError()==ERROR_IO_PENDING ){
    DWORD nDelay = (nMs==0 ? INFINITE : nMs);
    DWORD res = osWaitForSingleObject(ovlp.hEvent, nDelay);
    જો( res==WAIT_OBJECT_0 ){
      ret = TRUE;
    }નહીંતર જો( res==WAIT_TIMEOUT ){
#જો SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = SQLITE_BUSY_TIMEOUT;
#નહીંતર
      rc = SQLITE_BUSY;
#endif
    }નહીંતર{
      /* Some other error has occurred */
      rc = SQLITE_IOERR_LOCK;
    }

    /* If it is still pending, cancel the LockFileEx() call. */
    osCancelIo(hFile);
  }

  osCloseHandle(ovlp.hEvent);
#endif /* defined(SQLITE_ENABLE_SETLK_TIMEOUT) */

  જો( rc==SQLITE_OK && !ret ){
    rc = SQLITE_BUSY;
  }
  પાછા rc;
}
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Unlock a file region.
 */
સ્થિર BOOL winUnlockFile(
  LPHANDLE phFile,
  DWORD offset_નીચલું,
  DWORD offset_ઉપલું,
  DWORD અંક_Bytes_નીચલું,
  DWORD અંક_Bytes_ઉપલું
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, કદ(OVERLAPPED));
  ovlp.Offset = offset_નીચલું;
  ovlp.Offset_ઉપલું = offset_ઉપલું;
  પાછા osUnlockFileEx(*phFile, 0, અંક_Bytes_નીચલું, અંક_Bytes_ઉપલું, &ovlp);
}

#ifndef SQLITE_OMIT_WAL
/*
** Remove an nByte lock starting at offset iOff from HANDLE h.
*/
સ્થિર પૂર્ણાંક winHandleUnlock(HANDLE h, પૂર્ણાંક iOff, પૂર્ણાંક nByte){
  BOOL ret = winUnlockFile(&h, iOff, 0, nByte, 0);
  પાછા (ret ? SQLITE_OK : SQLITE_IOERR_UNLOCK);
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
સ્થિર પૂર્ણાંક winHandleSeek(HANDLE h, sqlite3_int64 iOffset){
  પૂર્ણાંક rc = SQLITE_OK;             /* Return value */
  LARGE_INTEGER x;                /* The offset */

  x.QuadPart = iOffset;
  જો( osSetFilePointerEx(h, x, 0, FILE_BEGIN)==0 ){
    rc = SQLITE_IOERR_SEEK;
  }
  OSTRACE(("SEEK file=%p, offset=%lld rc=%s\n", h, iOffset,sqlite3ErrName(rc)));
  પાછા rc;
}

/*
** Move the current position of the file handle passed as the first
** argument to offset iOffset within the file. If successful, return 0.
** Otherwise, set pFile->lastErrno and return non-zero.
*/
સ્થિર પૂર્ણાંક winSeekFile(winFile *pFile, sqlite3_int64 iOffset){
  પૂર્ણાંક rc;

  rc = winHandleSeek(pFile->h, iOffset);
  જો( rc!=SQLITE_OK ){
    pFile->lastErrno = osGetLastError();
    winLogError(rc, pFile->lastErrno, "winSeekFile", pFile->zPath);
  }
  પાછા rc;
}


#જો SQLITE_MAX_MMAP_SIZE>0
/* Forward references to VFS helper methods used for memory mapped files */
સ્થિર પૂર્ણાંક winMapfile(winFile*, sqlite3_int64);
સ્થિર પૂર્ણાંક winUnmapfile(winFile*);
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
સ્થિર પૂર્ણાંક winClose(sqlite3_file *id){
  પૂર્ણાંક rc, cnt = 0;
  winFile *pFile = (winFile*)id;

  પ્રતિજ્ઞા( id!=0 );
#ifndef SQLITE_OMIT_WAL
  પ્રતિજ્ઞા( pFile->pShm==0 );
#endif
  પ્રતિજ્ઞા( pFile->h!=ખાલી && pFile->h!=INVALID_HANDLE_કિંમત );
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p\n",
           osGetCurrentProcessId(), pFile, pFile->h));

#જો SQLITE_MAX_MMAP_SIZE>0
  winUnmapfile(pFile);
#endif

  કરો{
    rc = osCloseHandle(pFile->h);
    /* SimulateIOError( rc=0; cnt=MX_CLOSE_ATTEMPT; ); */
  }જ્યારે( rc==0 && ++cnt < MX_CLOSE_ATTEMPT && (osSleep(100), 1) );
  જો( rc ){
    pFile->h = ખાલી;
  }
  OpenCounter(-1);
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, rc ? "ok" : "failed"));
  પાછા rc ? SQLITE_OK
            : winLogError(SQLITE_IOERR_CLOSE, osGetLastError(),
                          "winClose", pFile->zPath);
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
સ્થિર પૂર્ણાંક winRead(
  sqlite3_file *id,          /* File to read from */
  શૂન્ય *pBuf,                /* Write content into this buffer */
  પૂર્ણાંક amt,                   /* Number of bytes to read */
  sqlite3_int64 offset       /* Begin reading at this offset */
){
#જો !defined(SQLITE_WIN32_NO_OVERLAPPED)
  OVERLAPPED overlapped;          /* The offset for ReadFile. */
#endif
  winFile *pFile = (winFile*)id;  /* file handle */
  DWORD nRead;                    /* Number of bytes actually read from file */
  પૂર્ણાંક nRetry = 0;                 /* Number of retrys */

  પ્રતિજ્ઞા( id!=0 );
  પ્રતિજ્ઞા( amt>0 );
  પ્રતિજ્ઞા( offset>=0 );
  SimulateIOError(પાછા SQLITE_IOERR_READ);
  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, offset, pFile->locktype));

#જો SQLITE_MAX_MMAP_SIZE>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  જો( offset<pFile->mmapSize ){
    જો( offset+amt <= pFile->mmapSize ){
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], amt);
      OSTRACE(("READ-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      પાછા SQLITE_OK;
    }નહીંતર{
      પૂર્ણાંક nCopy = (પૂર્ણાંક)(pFile->mmapSize - offset);
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#endif

#જો defined(SQLITE_WIN32_NO_OVERLAPPED)
  જો( winSeekFile(pFile, offset) ){
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    પાછા SQLITE_FULL;
  }
  જ્યારે( !osReadFile(pFile->h, pBuf, amt, &nRead, 0) ){
#નહીંતર
  memset(&overlapped, 0, કદ(OVERLAPPED));
  overlapped.Offset = (LONG)(offset & 0xffffffff);
  overlapped.Offset_ઉપલું = (LONG)((offset>>32) & 0x7fffffff);
  જ્યારે( !osReadFile(pFile->h, pBuf, amt, &nRead, &overlapped) &&
         osGetLastError()!=ERROR_HANDLE_EOF ){
#endif
    DWORD lastErrno;
    જો( winRetryIoerr(&nRetry, &lastErrno) ) ચાલુ;
    pFile->lastErrno = lastErrno;
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    પાછા winLogError(SQLITE_IOERR_READ, pFile->lastErrno,
                       "winRead", pFile->zPath);
  }
  winLogIoerr(nRetry, __LINE__);
  જો( nRead<(DWORD)amt ){
    /* Unread parts of the buffer must be zero-filled */
    memset(&((અક્ષર*)pBuf)[nRead], 0, amt-nRead);
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_SHORT_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    પાછા SQLITE_IOERR_SHORT_READ;
  }

  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  પાછા SQLITE_OK;
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
સ્થિર પૂર્ણાંક winWrite(
  sqlite3_file *id,               /* File to write into */
  અચલ શૂન્ય *pBuf,               /* The bytes to be written */
  પૂર્ણાંક amt,                        /* Number of bytes to write */
  sqlite3_int64 offset            /* Offset into the file to begin writing at */
){
  પૂર્ણાંક rc = 0;                     /* True if error has occurred, else false */
  winFile *pFile = (winFile*)id;  /* File handle */
  પૂર્ણાંક nRetry = 0;                 /* Number of retries */

  પ્રતિજ્ઞા( amt>0 );
  પ્રતિજ્ઞા( pFile );
  SimulateIOError(પાછા SQLITE_IOERR_WRITE);
  SimulateDiskfullError(પાછા SQLITE_FULL);

  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, offset, pFile->locktype));

#જો defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_SIZE>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  જો( offset<pFile->mmapSize ){
    જો( offset+amt <= pFile->mmapSize ){
      memcpy(&((u8 *)(pFile->pMapRegion))[offset], pBuf, amt);
      OSTRACE(("WRITE-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      પાછા SQLITE_OK;
    }નહીંતર{
      પૂર્ણાંક nCopy = (પૂર્ણાંક)(pFile->mmapSize - offset);
      memcpy(&((u8 *)(pFile->pMapRegion))[offset], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#endif

#જો defined(SQLITE_WIN32_NO_OVERLAPPED)
  rc = winSeekFile(pFile, offset);
  જો( rc==0 ){
#નહીંતર
  {
#endif
#જો !defined(SQLITE_WIN32_NO_OVERLAPPED)
    OVERLAPPED overlapped;        /* The offset for WriteFile. */
#endif
    u8 *aRem = (u8 *)pBuf;        /* Data yet to be written */
    પૂર્ણાંક nRem = amt;               /* Number of bytes yet to be written */
    DWORD nWrite;                 /* Bytes written by each WriteFile() call */
    DWORD lastErrno = NO_ERROR;   /* Value returned by GetLastError() */

#જો !defined(SQLITE_WIN32_NO_OVERLAPPED)
    memset(&overlapped, 0, કદ(OVERLAPPED));
    overlapped.Offset = (LONG)(offset & 0xffffffff);
    overlapped.Offset_ઉપલું = (LONG)((offset>>32) & 0x7fffffff);
#endif

    જ્યારે( nRem>0 ){
#જો defined(SQLITE_WIN32_NO_OVERLAPPED)
      જો( !osWriteFile(pFile->h, aRem, nRem, &nWrite, 0) ){
#નહીંતર
      જો( !osWriteFile(pFile->h, aRem, nRem, &nWrite, &overlapped) ){
#endif
        જો( winRetryIoerr(&nRetry, &lastErrno) ) ચાલુ;
        તોડો;
      }
      પ્રતિજ્ઞા( nWrite==0 || nWrite<=(DWORD)nRem );
      જો( nWrite==0 || nWrite>(DWORD)nRem ){
        lastErrno = osGetLastError();
        તોડો;
      }
#જો !defined(SQLITE_WIN32_NO_OVERLAPPED)
      offset += nWrite;
      overlapped.Offset = (LONG)(offset & 0xffffffff);
      overlapped.Offset_ઉપલું = (LONG)((offset>>32) & 0x7fffffff);
#endif
      aRem += nWrite;
      nRem -= nWrite;
    }
    જો( nRem>0 ){
      pFile->lastErrno = lastErrno;
      rc = 1;
    }
  }

  જો( rc ){
    જો(   ( pFile->lastErrno==ERROR_HANDLE_DISK_FULL )
       || ( pFile->lastErrno==ERROR_DISK_FULL )){
      OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      પાછા winLogError(SQLITE_FULL, pFile->lastErrno,
                         "winWrite1", pFile->zPath);
    }
    OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_WRITE\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    પાછા winLogError(SQLITE_IOERR_WRITE, pFile->lastErrno,
                       "winWrite2", pFile->zPath);
  }નહીંતર{
    winLogIoerr(nRetry, __LINE__);
  }
  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  પાછા SQLITE_OK;
}

#ifndef SQLITE_OMIT_WAL
/*
** Truncate the file opened by handle h to nByte bytes in size.
*/
સ્થિર પૂર્ણાંક winHandleTruncate(HANDLE h, sqlite3_int64 nByte){ 
  પૂર્ણાંક rc = SQLITE_OK;             /* Return code */
  rc = winHandleSeek(h, nByte);
  જો( rc==SQLITE_OK ){
    જો( 0==osSetEndOfFile(h) ){
      rc = SQLITE_IOERR_TRUNCATE;
    }
  }
  પાછા rc;
}

/*
** Determine the size in bytes of the file opened by the handle passed as 
** the first argument.
*/
સ્થિર પૂર્ણાંક winHandleSize(HANDLE h, sqlite3_int64 *pnByte){ 
  પૂર્ણાંક rc = SQLITE_OK;
  LARGE_INTEGER x;
  પ્રતિજ્ઞા( pnByte );
  જો( osGetFileSizeEx(h, &x)==0 ){
    rc = SQLITE_IOERR_FSTAT;
    *pnByte = 0;
  }નહીંતર{
    *pnByte = x.QuadPart;
  }
  પાછા rc;
}

/*
** Close the handle passed as the only argument.
*/
સ્થિર શૂન્ય winHandleClose(HANDLE h){
  જો( h!=INVALID_HANDLE_કિંમત ){
    osCloseHandle(h);
  }
}
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Truncate an open file to a specified size
*/
સ્થિર પૂર્ણાંક winTruncate(sqlite3_file *id, sqlite3_int64 nByte){
  winFile *pFile = (winFile*)id;  /* File handle object */
  પૂર્ણાંક rc = SQLITE_OK;             /* Return code for this function */
  DWORD lastErrno;
#જો SQLITE_MAX_MMAP_SIZE>0
  sqlite3_int64 oldMmapSize;
  જો( pFile->nFetchOut>0 ){
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
    પાછા SQLITE_OK;
  }
#endif

  પ્રતિજ્ઞા( pFile );
  SimulateIOError(પાછા SQLITE_IOERR_TRUNCATE);
  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, size=%lld, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, nByte, pFile->locktype));

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  જો( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

#જો SQLITE_MAX_MMAP_SIZE>0
  જો( pFile->pMapRegion ){
    oldMmapSize = pFile->mmapSize;
  }નહીંતર{
    oldMmapSize = 0;
  }
  winUnmapfile(pFile);
#endif

  /* SetEndOfFile() returns non-zero when successful, or zero when it fails. */
  જો( winSeekFile(pFile, nByte) ){
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->lastErrno,
                     "winTruncate1", pFile->zPath);
  }નહીંતર જો( 0==osSetEndOfFile(pFile->h) &&
            ((lastErrno = osGetLastError())!=ERROR_USER_MAPPED_FILE) ){
    pFile->lastErrno = lastErrno;
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->lastErrno,
                     "winTruncate2", pFile->zPath);
  }

#જો SQLITE_MAX_MMAP_SIZE>0
  જો( rc==SQLITE_OK && oldMmapSize>0 ){
    જો( oldMmapSize>nByte ){
      winMapfile(pFile, -1);
    }નહીંતર{
      winMapfile(pFile, oldMmapSize);
    }
  }
#endif

  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, sqlite3ErrName(rc)));
  પાછા rc;
}

#ifdef SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occurring at the right times.
*/
પૂર્ણાંક sqlite3_sync_ગણતરી = 0;
પૂર્ણાંક sqlite3_fullsync_ગણતરી = 0;
#endif

/*
** Make sure all writes to a particular file are committed to disk.
*/
સ્થિર પૂર્ણાંક winSync(sqlite3_file *id, પૂર્ણાંક flags){
#ifndef SQLITE_NO_SYNC
  /*
  ** Used only when SQLITE_NO_SYNC is not defined.
   */
  BOOL rc;
#endif
#જો !defined(NDEBUG) || !defined(SQLITE_NO_SYNC) || \
    defined(SQLITE_HAVE_OS_TRACE)
  /*
  ** Used when SQLITE_NO_SYNC is not defined and by the assert() and/or
  ** OSTRACE() macros.
   */
  winFile *pFile = (winFile*)id;
#નહીંતર
  UNUSED_PARAMETER(id);
#endif

  પ્રતિજ્ઞા( pFile );
  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  પ્રતિજ્ઞા((flags&0x0F)==SQLITE_SYNC_NORMAL
      || (flags&0x0F)==SQLITE_SYNC_FULL
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( પાછા SQLITE_FULL );

  OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, flags=%x, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, flags,
           pFile->locktype));

#ifndef SQLITE_TEST
  UNUSED_PARAMETER(flags);
#નહીંતર
  જો( (flags&0x0F)==SQLITE_SYNC_FULL ){
    sqlite3_fullsync_ગણતરી++;
  }
  sqlite3_sync_ગણતરી++;
#endif

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op
  */
#ifdef SQLITE_NO_SYNC
  OSTRACE(("SYNC-NOP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  પાછા SQLITE_OK;
#નહીંતર
#જો SQLITE_MAX_MMAP_SIZE>0
  જો( pFile->pMapRegion ){
    જો( osFlushViewOfFile(pFile->pMapRegion, 0) ){
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_OK\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
    }નહીંતર{
      pFile->lastErrno = osGetLastError();
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
      પાછા winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winSync1", pFile->zPath);
    }
  }
#endif
  rc = osFlushFileBuffers(pFile->h);
  SimulateIOError( rc=FALSE );
  જો( rc ){
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    પાછા SQLITE_OK;
  }નહીંતર{
    pFile->lastErrno = osGetLastError();
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_FSYNC\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    પાછા winLogError(SQLITE_IOERR_FSYNC, pFile->lastErrno,
                       "winSync2", pFile->zPath);
  }
#endif
}

/*
** Determine the current size of a file in bytes
*/
સ્થિર પૂર્ણાંક winFileSize(sqlite3_file *id, sqlite3_int64 *pSize){
  winFile *pFile = (winFile*)id;
  પૂર્ણાંક rc = SQLITE_OK;

  પ્રતિજ્ઞા( id!=0 );
  પ્રતિજ્ઞા( pSize!=0 );
  SimulateIOError(પાછા SQLITE_IOERR_FSTAT);
  OSTRACE(("SIZE file=%p, pSize=%p\n", pFile->h, pSize));
  {
    LARGE_INTEGER x;
    જો( osGetFileSizeEx(pFile->h, &x)==0 ){
      *pSize = 0;
      pFile->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_FSTAT, pFile->lastErrno,
                       "winFileSize", pFile->zPath);
    }નહીંતર{
      *pSize = x.QuadPart;
    }
  }
  OSTRACE(("SIZE file=%p, pSize=%p, *pSize=%lld, rc=%s\n",
           pFile->h, pSize, *pSize, sqlite3ErrName(rc)));
  પાછા rc;
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
સ્થિર પૂર્ણાંક winGetReadLock(winFile *pFile, પૂર્ણાંક bBlock){
  પૂર્ણાંક res;
  DWORD mask = ~(bBlock ? LOCKFILE_FAIL_IMMEDIATELY : 0);
  OSTRACE(("READ-LOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winLockFile(&pFile->h, SQLITE_LOCKFILEEX_FLAGS&mask, SHARED_FIRST, 0,
                    SHARED_SIZE, 0);
  જો( res == 0 ){
    pFile->lastErrno = osGetLastError();
    /* No need to log a failure to lock */
  }
  OSTRACE(("READ-LOCK file=%p, result=%d\n", pFile->h, res));
  પાછા res;
}

/*
** Undo a readlock
*/
સ્થિર પૂર્ણાંક winUnlockReadLock(winFile *pFile){
  પૂર્ણાંક res;
  DWORD lastErrno;
  OSTRACE(("READ-UNLOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winUnlockFile(&pFile->h, SHARED_FIRST, 0, SHARED_SIZE, 0);
  જો( res==0 && ((lastErrno = osGetLastError())!=ERROR_NOT_LOCKED) ){
    pFile->lastErrno = lastErrno;
    winLogError(SQLITE_IOERR_UNLOCK, pFile->lastErrno,
                "winUnlockReadLock", pFile->zPath);
  }
  OSTRACE(("READ-UNLOCK file=%p, result=%d\n", pFile->h, res));
  પાછા res;
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
સ્થિર પૂર્ણાંક winLock(sqlite3_file *id, પૂર્ણાંક locktype){
  પૂર્ણાંક rc = SQLITE_OK;    /* Return code from subroutines */
  પૂર્ણાંક res = 1;           /* Result of a Windows lock call */
  પૂર્ણાંક newLocktype;       /* Set pFile->locktype to this value before exiting */
  પૂર્ણાંક gotPendingLock = 0;/* True if we acquired a PENDING lock this time */
  winFile *pFile = (winFile*)id;
  DWORD lastErrno = NO_ERROR;

  પ્રતિજ્ઞા( id!=0 );
  OSTRACE(("LOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->sharedLockByte, locktype));

  /* If there is already a lock of this type or more restrictive on the
  ** OsFile, do nothing. Don't use the end_lock: exit path, as
  ** sqlite3OsEnterMutex() hasn't been called yet.
  */
  જો( pFile->locktype>=locktype ){
    OSTRACE(("LOCK-HELD file=%p, rc=SQLITE_OK\n", pFile->h));
    પાછા SQLITE_OK;
  }

  /* Do not allow any kind of write-lock on a read-only database
  */
  જો( (pFile->ctrlFlags & WINFILE_RDONLY)!=0 && locktype>=RESERVED_LOCK ){
    પાછા SQLITE_IOERR_LOCK;
  }

  /* Make sure the locking sequence is correct
  */
  પ્રતિજ્ઞા( pFile->locktype!=NO_LOCK || locktype==SHARED_LOCK );
  પ્રતિજ્ઞા( locktype!=PENDING_LOCK );
  પ્રતિજ્ઞા( locktype!=RESERVED_LOCK || pFile->locktype==SHARED_LOCK );

  /* Lock the PENDING_LOCK byte if we need to acquire an EXCLUSIVE lock or
  ** a SHARED lock.  If we are acquiring a SHARED lock, the acquisition of
  ** the PENDING_LOCK byte is temporary.
  */
  newLocktype = pFile->locktype;
  જો( locktype==SHARED_LOCK
   || (locktype==EXCLUSIVE_LOCK && pFile->locktype==RESERVED_LOCK)
  ){
    પૂર્ણાંક cnt = 3;

    /* Flags for the LockFileEx() call. This should be an exclusive lock if
    ** this call is to obtain EXCLUSIVE, or a shared lock if this call is to
    ** obtain SHARED.  */
    પૂર્ણાંક flags = LOCKFILE_FAIL_IMMEDIATELY;
    જો( locktype==EXCLUSIVE_LOCK ){
      flags |= LOCKFILE_EXCLUSIVE_LOCK;
    }
    જ્યારે( cnt>0 ){
      /* Try 3 times to get the pending lock.  This is needed to work
      ** around problems caused by indexing and/or anti-virus software on
      ** Windows systems.
      **
      ** If you are using this code as a model for alternative VFSes, do not
      ** copy this retry logic.  It is a hack intended for Windows only.  */
      res = winLockFile(&pFile->h, flags, PENDING_BYTE, 0, 1, 0);
      જો( res ) તોડો;

      lastErrno = osGetLastError();
      OSTRACE(("LOCK-PENDING-FAIL file=%p, count=%d, result=%d\n", 
            pFile->h, cnt, res
      ));

      જો( lastErrno==ERROR_INVALID_HANDLE ){
        pFile->lastErrno = lastErrno;
        rc = SQLITE_IOERR_LOCK;
        OSTRACE(("LOCK-FAIL file=%p, count=%d, rc=%s\n", 
              pFile->h, cnt, sqlite3ErrName(rc)
        ));
        પાછા rc;
      }

      cnt--;
      જો( cnt>0 ) osSleep(1);
    }
    gotPendingLock = res;
  }

  /* Acquire a shared lock
  */
  જો( locktype==SHARED_LOCK && res ){
    પ્રતિજ્ઞા( pFile->locktype==NO_LOCK );
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    res = winGetReadLock(pFile, pFile->bBlockOnConnect);
#નહીંતર
    res = winGetReadLock(pFile, 0);
#endif
    જો( res ){
      newLocktype = SHARED_LOCK;
    }નહીંતર{
      lastErrno = osGetLastError();
    }
  }

  /* Acquire a RESERVED lock
  */
  જો( locktype==RESERVED_LOCK && res ){
    પ્રતિજ્ઞા( pFile->locktype==SHARED_LOCK );
    res = winLockFile(&pFile->h, SQLITE_LOCKFILE_FLAGS, RESERVED_BYTE, 0, 1, 0);
    જો( res ){
      newLocktype = RESERVED_LOCK;
    }નહીંતર{
      lastErrno = osGetLastError();
    }
  }

  /* Acquire a PENDING lock
  */
  જો( locktype==EXCLUSIVE_LOCK && res ){
    newLocktype = PENDING_LOCK;
    gotPendingLock = 0;
  }

  /* Acquire an EXCLUSIVE lock
  */
  જો( locktype==EXCLUSIVE_LOCK && res ){
    પ્રતિજ્ઞા( pFile->locktype>=SHARED_LOCK );
    (શૂન્ય)winUnlockReadLock(pFile);
    res = winLockFile(&pFile->h, SQLITE_LOCKFILE_FLAGS, SHARED_FIRST, 0,
                      SHARED_SIZE, 0);
    જો( res ){
      newLocktype = EXCLUSIVE_LOCK;
    }નહીંતર{
      lastErrno = osGetLastError();
      winGetReadLock(pFile, 0);
    }
  }

  /* If we are holding a PENDING lock that ought to be released, then
  ** release it now.
  */
  જો( gotPendingLock && locktype==SHARED_LOCK ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }

  /* Update the state of the lock has held in the file descriptor then
  ** return the appropriate result code.
  */
  જો( res ){
    rc = SQLITE_OK;
  }નહીંતર{
    pFile->lastErrno = lastErrno;
    rc = SQLITE_BUSY;
    OSTRACE(("LOCK-FAIL file=%p, wanted=%d, got=%d\n",
             pFile->h, locktype, newLocktype));
  }
  pFile->locktype = (u8)newLocktype;
  OSTRACE(("LOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  પાછા rc;
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, return
** non-zero, otherwise zero.
*/
સ્થિર પૂર્ણાંક winCheckReservedLock(sqlite3_file *id, પૂર્ણાંક *pResOut){
  પૂર્ણાંક res;
  winFile *pFile = (winFile*)id;

  SimulateIOError( પાછા SQLITE_IOERR_CHECKRESERVEDLOCK; );
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p\n", pFile->h, pResOut));

  પ્રતિજ્ઞા( id!=0 );
  જો( pFile->locktype>=RESERVED_LOCK ){
    res = 1;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (local)\n", pFile->h, res));
  }નહીંતર{
    res = winLockFile(&pFile->h, SQLITE_LOCKFILEEX_FLAGS,RESERVED_BYTE,0,1,0);
    જો( res ){
      winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
    }
    res = !res;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (remote)\n", pFile->h, res));
  }
  *pResOut = res;
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           pFile->h, pResOut, *pResOut));
  પાછા SQLITE_OK;
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
સ્થિર પૂર્ણાંક winUnlock(sqlite3_file *id, પૂર્ણાંક locktype){
  પૂર્ણાંક type;
  winFile *pFile = (winFile*)id;
  પૂર્ણાંક rc = SQLITE_OK;
  પ્રતિજ્ઞા( pFile!=0 );
  પ્રતિજ્ઞા( locktype<=SHARED_LOCK );
  OSTRACE(("UNLOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->sharedLockByte, locktype));
  type = pFile->locktype;
  જો( type>=EXCLUSIVE_LOCK ){
    winUnlockFile(&pFile->h, SHARED_FIRST, 0, SHARED_SIZE, 0);
    જો( locktype==SHARED_LOCK && !winGetReadLock(pFile, 0) ){
      /* This should never happen.  We should always be able to
      ** reacquire the read lock */
      rc = winLogError(SQLITE_IOERR_UNLOCK, osGetLastError(),
                       "winUnlock", pFile->zPath);
    }
  }
  જો( type>=RESERVED_LOCK ){
    winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
  }
  જો( locktype==NO_LOCK && type>=SHARED_LOCK ){
    winUnlockReadLock(pFile);
  }
  જો( type>=PENDING_LOCK ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }
  pFile->locktype = (u8)locktype;
  OSTRACE(("UNLOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  પાછા rc;
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

સ્થિર પૂર્ણાંક winNolockLock(sqlite3_file *id, પૂર્ણાંક locktype){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(locktype);
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક winNolockCheckReservedLock(sqlite3_file *id, પૂર્ણાંક *pResOut){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(pResOut);
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક winNolockUnlock(sqlite3_file *id, પૂર્ણાંક locktype){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(locktype);
  પાછા SQLITE_OK;
}

/******************* End of the no-op lock implementation *********************
******************************************************************************/

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
સ્થિર શૂન્ય winModeBit(winFile *pFile, unsigned અક્ષર mask, પૂર્ણાંક *pArg){
  જો( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & mask)!=0;
  }નહીંતર જો( (*pArg)==0 ){
    pFile->ctrlFlags &= ~mask;
  }નહીંતર{
    pFile->ctrlFlags |= mask;
  }
}

/* Forward references to VFS helper methods used for temporary files */
સ્થિર પૂર્ણાંક winGetTempname(sqlite3_vfs *, અક્ષર **);
સ્થિર પૂર્ણાંક winIsDir(LPCWSTR);
સ્થિર BOOL winIsLongPathPrefix(અચલ અક્ષર *);
સ્થિર BOOL winIsDriveLetterAndColon(અચલ અક્ષર *);

/*
** Control and query of the open file handle.
*/
સ્થિર પૂર્ણાંક winFileControl(sqlite3_file *id, પૂર્ણાંક op, શૂન્ય *pArg){
  winFile *pFile = (winFile*)id;
  OSTRACE(("FCNTL file=%p, op=%d, pArg=%p\n", pFile->h, op, pArg));
  પસંદગી( op ){
    સ્થિતિ SQLITE_FCNTL_LOCKSTATE: {
      *(પૂર્ણાંક*)pArg = pFile->locktype;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      પાછા SQLITE_OK;
    }
    સ્થિતિ SQLITE_FCNTL_LAST_ERRNO: {
      *(પૂર્ણાંક*)pArg = (પૂર્ણાંક)pFile->lastErrno;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      પાછા SQLITE_OK;
    }
    સ્થિતિ SQLITE_FCNTL_CHUNK_SIZE: {
      pFile->szChunk = *(પૂર્ણાંક *)pArg;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      પાછા SQLITE_OK;
    }
    સ્થિતિ SQLITE_FCNTL_SIZE_HINT: {
      જો( pFile->szChunk>0 ){
        sqlite3_int64 oldSz;
        પૂર્ણાંક rc = winFileSize(id, &oldSz);
        જો( rc==SQLITE_OK ){
          sqlite3_int64 newSz = *(sqlite3_int64*)pArg;
          જો( newSz>oldSz ){
            SimulateIOErrorBenign(1);
            rc = winTruncate(id, newSz);
            SimulateIOErrorBenign(0);
          }
        }
        OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
        પાછા rc;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      પાછા SQLITE_OK;
    }
    સ્થિતિ SQLITE_FCNTL_PERSIST_WAL: {
      winModeBit(pFile, WINFILE_PERSIST_WAL, (પૂર્ણાંક*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      પાછા SQLITE_OK;
    }
    સ્થિતિ SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      winModeBit(pFile, WINFILE_PSOW, (પૂર્ણાંક*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      પાછા SQLITE_OK;
    }
    સ્થિતિ SQLITE_FCNTL_VFSNAME: {
      *(અક્ષર**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      પાછા SQLITE_OK;
    }
    સ્થિતિ SQLITE_FCNTL_WIN32_AV_RETRY: {
      પૂર્ણાંક *a = (પૂર્ણાંક*)pArg;
      જો( a[0]>0 ){
        winIoerrRetry = a[0];
      }નહીંતર{
        a[0] = winIoerrRetry;
      }
      જો( a[1]>0 ){
        winIoerrRetryDelay = a[1];
      }નહીંતર{
        a[1] = winIoerrRetryDelay;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      પાછા SQLITE_OK;
    }
    સ્થિતિ SQLITE_FCNTL_WIN32_GET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      *phFile = pFile->h;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      પાછા SQLITE_OK;
    }
#ifdef SQLITE_TEST
    સ્થિતિ SQLITE_FCNTL_WIN32_SET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      HANDLE hOldFile = pFile->h;
      pFile->h = *phFile;
      *phFile = hOldFile;
      OSTRACE(("FCNTL oldFile=%p, newFile=%p, rc=SQLITE_OK\n",
               hOldFile, pFile->h));
      પાછા SQLITE_OK;
    }
#endif
    સ્થિતિ SQLITE_FCNTL_NULL_IO: {
      (શૂન્ય)osCloseHandle(pFile->h);
      pFile->h = ખાલી;
      પાછા SQLITE_OK;
    }
    સ્થિતિ SQLITE_FCNTL_TEMPFILENAME: {
      અક્ષર *zTFile = 0;
      પૂર્ણાંક rc = winGetTempname(pFile->pVfs, &zTFile);
      જો( rc==SQLITE_OK ){
        *(અક્ષર**)pArg = zTFile;
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      પાછા rc;
    }
#જો SQLITE_MAX_MMAP_SIZE>0
    સ્થિતિ SQLITE_FCNTL_MMAP_SIZE: {
      i64 newLimit = *(i64*)pArg;
      પૂર્ણાંક rc = SQLITE_OK;
      જો( newLimit>sqlite3GlobalConfig.mxMmap ){
        newLimit = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (SIZE_T) and passed
      ** to MapViewOfFile(). Restrict its value to 2GB if (SIZE_T) is not at
      ** least a 64-bit type. */
      જો( newLimit>0 && કદ(SIZE_T)<8 ){
        newLimit = (newLimit & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmapSizeMax;
      જો( newLimit>=0 && newLimit!=pFile->mmapSizeMax && pFile->nFetchOut==0 ){
        pFile->mmapSizeMax = newLimit;
        જો( pFile->mmapSize>0 ){
          winUnmapfile(pFile);
          rc = winMapfile(pFile, -1);
        }
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      પાછા rc;
    }
#endif

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    સ્થિતિ SQLITE_FCNTL_LOCK_TIMEOUT: {
      પૂર્ણાંક iOld = pFile->iBusyTimeout;
      પૂર્ણાંક iNew = *(પૂર્ણાંક*)pArg;
#જો SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = (iNew < 0) ? INFINITE : (DWORD)iNew;
#અથવાજો SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = (DWORD)(!!iNew);
#નહીંતર
# error "SQLITE_ENABLE_SETLK_TIMEOUT must be set to 1 or 2"
#endif
      *(પૂર્ણાંક*)pArg = iOld;
      પાછા SQLITE_OK;
    }
    સ્થિતિ SQLITE_FCNTL_BLOCK_ON_CONNECT: {
      પૂર્ણાંક iNew = *(પૂર્ણાંક*)pArg;
      pFile->bBlockOnConnect = iNew;
      પાછા SQLITE_OK;
    }
#endif /* SQLITE_ENABLE_SETLK_TIMEOUT */

#જો defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    સ્થિતિ SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      sqlite3_str_appendf(pStr, "{\"h\":%llu", (sqlite3_uint64)pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      જો( pFile->locktype ){
        સ્થિર અચલ અક્ષર *azLock[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
        sqlite3_str_appendf(pStr, ",\"locktype\":\"%s\"", 
                                  azLock[pFile->locktype-1]);
      }
#જો SQLITE_MAX_MMAP_SIZE>0
      જો( pFile->mmapSize ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmapSize);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->nFetchOut);
      }
#endif
      sqlite3_str_append(pStr, "}", 1);
      પાછા SQLITE_OK;
    }
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

  }
  OSTRACE(("FCNTL file=%p, rc=SQLITE_NOTFOUND\n", pFile->h));
  પાછા SQLITE_NOTFOUND;
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
સ્થિર પૂર્ણાંક winSectorSize(sqlite3_file *id){
  (શૂન્ય)id;
  પાછા SQLITE_DEFAULT_SECTOR_SIZE;
}

/*
** Return a vector of device characteristics.
*/
સ્થિર પૂર્ણાંક winDeviceCharacteristics(sqlite3_file *id){
  winFile *p = (winFile*)id;
  પાછા SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN | SQLITE_IOCAP_SUBPAGE_READ |
         ((p->ctrlFlags & WINFILE_PSOW)?SQLITE_IOCAP_POWERSAFE_OVERWRITE:0);
}

/*
** Windows will only let you create file view mappings
** on allocation size granularity boundaries.
** During sqlite3_os_init() we do a GetSystemInfo()
** to get the granularity size.
*/
સ્થિર SYSTEM_INFO winSysInfo;

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
સ્થિર LPWSTR winConvertFromUtf8Filename(અચલ અક્ષર *zFilename){
  LPWSTR zConverted = 0;
#ifdef __CYGWIN__
  પૂર્ણાંક nChar;

  જો( osCygwin_conv_path && !(winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2])) ){
    i64 nByte;
    પૂર્ણાંક convertflag = CCP_POSIX_TO_WIN_W;
    જો( !strchr(zFilename, '/') ) convertflag |= CCP_RELATIVE;
    nByte = (i64)osCygwin_conv_path(convertflag,
        zFilename, 0, 0);
    જો( nByte>0 ){
      zConverted = sqlite3MallocZero(12+(u64)nByte);
      જો ( zConverted==0 ){
        પાછા zConverted;
      }
      /* Filenames should be prefixed, except when converted
       * full path already starts with "\\?\". */
      જો( osCygwin_conv_path(convertflag, zFilename,
                           zConverted+4, nByte)==0 ){
        જો( (convertflag&CCP_RELATIVE) ){
          memmove(zConverted, zConverted+4, nByte);
        }નહીંતર જો( memcmp(zConverted+4, L"\\\\", 4) ){
          memcpy(zConverted, L"\\\\?\\", 8);
        }નહીંતર જો( zConverted[6]!='?' ){
          memmove(zConverted+6, zConverted+4, nByte);
          memcpy(zConverted, L"\\\\?\\UNC", 14);
        }નહીંતર{
          memmove(zConverted, zConverted+4, nByte);
        }
        પાછા zConverted;
      }
      sqlite3_free(zConverted);
    }
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zFilename, -1, ખાલી, 0);
  જો( nChar==0 ){
    પાછા 0;
  }
  zConverted = sqlite3MallocZero( ((u64)nChar+6)*કદ(WCHAR) );
  જો( zConverted==0 ){
    પાછા 0;
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zFilename, -1,
                                zConverted, nChar);
  જો( nChar==0 ){
    sqlite3_free(zConverted);
    zConverted = 0;
  }નહીંતર જો( nChar>MAX_PATH
      && winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2]) ){
    memmove(zConverted+4, zConverted, nChar*કદ(WCHAR));
    zConverted[2] = '\\';
    memcpy(zConverted, L"\\\\?\\", 8);
  }નહીંતર જો( nChar>MAX_PATH
      && winIsDirSep(zFilename[0]) && winIsDirSep(zFilename[1])
      && zFilename[2] != '?' ){
    memmove(zConverted+6, zConverted, nChar*કદ(WCHAR));
    memcpy(zConverted, L"\\\\?\\UNC", 14);
  }
#નહીંતર /* if !defined(__CYGWIN__) */
  zConverted = winUtf8ToUnicode(zFilename);
#endif /* __CYGWIN__ */
  /* caller will handle out of memory */
  પાછા zConverted;
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
સ્થિર sqlite3_mutex *winBigLock = 0;
સ્થિર શૂન્ય winShmEnterMutex(શૂન્ય){
  sqlite3_mutex_enter(winBigLock);
}
સ્થિર શૂન્ય winShmLeaveMutex(શૂન્ય){
  sqlite3_mutex_leave(winBigLock);
}
#ifndef NDEBUG
સ્થિર પૂર્ણાંક winShmMutexHeld(શૂન્ય) {
  પાછા sqlite3_mutex_held(winBigLock);
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
સંરચના win_Shm_ગાંઠ {
  sqlite3_mutex *mutex;      /* Mutex to access this object */
  અક્ષર *zFilename;           /* Name of the file */
  HANDLE hSharedShm;         /* File handle open on zFilename */
  પૂર્ણાંક bUseSharedLockHandle;  /* True to use hSharedShm for everything */

  પૂર્ણાંક isUnlocked;            /* DMS lock has not yet been obtained */
  પૂર્ણાંક isReadonly;            /* True if read-only */
  પૂર્ણાંક szRegion;              /* Size of shared-memory regions */
  પૂર્ણાંક nRegion;               /* Size of array apRegion */

  સંરચના ShmRegion {
    HANDLE hMap;             /* File handle from CreateFileMapping */
    શૂન્ય *pMap;
  } *aRegion;
  DWORD lastErrno;           /* The Windows errno from the last I/O error */

  winShm *pWinShmList;       /* List of winShm objects with ptrs to this */

  win_Shm_ગાંઠ *pNext;         /* Next in list of all winShmNode objects */
#જો defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 nextShmId;              /* Next available winShm.id value */
#endif
};

/*
** A global array of all winShmNode objects.
**
** The winShmMutexHeld() must be true while reading or writing this list.
*/
સ્થિર win_Shm_ગાંઠ *win_Shm_ગાંઠ_List = 0;

/*
** Structure used internally by this VFS to record the state of an
** open shared memory connection. There is one such structure for each
** winFile open on a wal mode database.
*/
સંરચના winShm {
  win_Shm_ગાંઠ *p_Shm_ગાંઠ;      /* The underlying winShmNode object */
  u16 sharedMask;            /* Mask of shared locks held */
  u16 exclMask;              /* Mask of exclusive locks held */
  HANDLE hShm;               /* File-handle on *-shm file. For locking. */
  પૂર્ણાંક bReadonly;             /* True if hShm is opened read-only */
#જો defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
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
સ્થિર પૂર્ણાંક winOpen(sqlite3_vfs*,અચલ અક્ષર*,sqlite3_file*,પૂર્ણાંક,પૂર્ણાંક*);
સ્થિર પૂર્ણાંક winDelete(sqlite3_vfs *,અચલ અક્ષર*,પૂર્ણાંક);

/*
** Purge the winShmNodeList list of all entries with winShmNode.pWinShmList==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
સ્થિર શૂન્ય winShmPurge(sqlite3_vfs *pVfs, પૂર્ણાંક deleteFlag){
  win_Shm_ગાંઠ **pp;
  win_Shm_ગાંઠ *p;
  પ્રતિજ્ઞા( winShmMutexHeld() );
  OSTRACE(("SHM-PURGE pid=%lu, deleteFlag=%d\n",
           osGetCurrentProcessId(), deleteFlag));
  pp = &win_Shm_ગાંઠ_List;
  જ્યારે( (p = *pp)!=0 ){
    જો( p->pWinShmList==0 ){
      પૂર્ણાંક i;
      જો( p->mutex ){ sqlite3_mutex_free(p->mutex); }
      માટે(i=0; i<p->nRegion; i++){
        BOOL bRc = osUnmapViewOfFile(p->aRegion[i].pMap);
        OSTRACE(("SHM-PURGE-UNMAP pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        UNUSED_VARIABLE_કિંમત(bRc);
        bRc = osCloseHandle(p->aRegion[i].hMap);
        OSTRACE(("SHM-PURGE-CLOSE pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        UNUSED_VARIABLE_કિંમત(bRc);
      }
      winHandleClose(p->hSharedShm);
      જો( deleteFlag ){
        SimulateIOErrorBenign(1);
        sqlite3BeginBenignMalloc();
        winDelete(pVfs, p->zFilename, 0);
        sqlite3EndBenignMalloc();
        SimulateIOErrorBenign(0);
      }
      *pp = p->pNext;
      sqlite3_free(p->aRegion);
      sqlite3_free(p);
    }નહીંતર{
      pp = &p->pNext;
    }
  }
}

/*
** Handle h is open on the *-shm file. This function zeroes the first
** 8 bytes of the file. If SQLITE_ENABLE_SETLK_TIMEOUT is defined, h may 
** have been opened with FILE_FLAG_OVERLAPPED.
*/
સ્થિર પૂર્ણાંક winZeroSharedMemory(HANDLE h){
  u8 aZero[8] = {0,0,0,0,0,0,0,0};
  DWORD nWrite = 0;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, કદ(OVERLAPPED));
  જો( !osWriteFile(h, aZero, કદ(aZero), &nWrite, &ovlp) ){
    /* If the file was opened with FILE_FLAG_OVERLAPPED, the write may still
    ** be pending. Wait for it with GetOverlappedResult().  */
    જો( osGetLastError()!=ERROR_IO_PENDING
     || !GetOverlappedResult(h, &ovlp, &nWrite, TRUE)
    ){
      nWrite = 0;
    }
  }
  પાછા (nWrite==કદ(aZero) ? SQLITE_OK : SQLITE_IOERR_WRITE);
}

/*
** The DMS lock has not yet been taken on the shm file associated with
** pShmNode. Take the lock. Truncate the *-shm file if required.
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
સ્થિર પૂર્ણાંક winLockSharedMemory(win_Shm_ગાંઠ *p_Shm_ગાંઠ, DWORD nMs){
  HANDLE h = p_Shm_ગાંઠ->hSharedShm;
  પૂર્ણાંક rc = SQLITE_OK;

  પ્રતિજ્ઞા( sqlite3_mutex_held(p_Shm_ગાંઠ->mutex) );
  rc = winHandleLockTimeout(h, WIN_SHM_DMS, 1, 1, 0);
  જો( rc==SQLITE_OK ){
    /* We have an EXCLUSIVE lock on the DMS byte. This means that this
    ** is the first process to open the file. Truncate it to zero bytes
    ** in this case.  */
    જો( p_Shm_ગાંઠ->isReadonly ){
      rc = SQLITE_READONLY_CANTINIT;
    }નહીંતર{
      જો( SQLITE_OK!=winHandleTruncate(h, 0) ){
        /* Sometimes the truncate operation may fail because some other
        ** process is still holding an open mapping. So try to write
        ** zeroes into the start of the file instead. */
        rc = winZeroSharedMemory(h);
      }
    }

    /* Release the EXCLUSIVE lock acquired above. */
    winUnlockFile(&h, WIN_SHM_DMS, 0, 1, 0);
  }નહીંતર જો( (rc & 0xFF)==SQLITE_BUSY ){
    rc = SQLITE_OK;
  }

  જો( rc==SQLITE_OK ){
    /* Take a SHARED lock on the DMS byte. */
    rc = winHandleLockTimeout(h, WIN_SHM_DMS, 1, 0, nMs);
    જો( rc==SQLITE_OK ){
      p_Shm_ગાંઠ->isUnlocked = 0;
    }
  }

  પાછા rc;
}


/*
** This function is used to open a handle on a *-shm file.
**
** If SQLITE_ENABLE_SETLK_TIMEOUT is defined at build time, then the file
** is opened with FILE_FLAG_OVERLAPPED specified. If not, it is not.
*/
સ્થિર પૂર્ણાંક winHandleOpen(
  અચલ અક્ષર *zUtf8,              /* File to open */
  પૂર્ણાંક *pbReadonly,                /* IN/OUT: True for readonly handle */
  HANDLE *ph                      /* OUT: New HANDLE for file */
){
  પૂર્ણાંક rc = SQLITE_OK;
  LPWSTR zConverted = 0;
  પૂર્ણાંક bReadonly = *pbReadonly;
  HANDLE h = INVALID_HANDLE_કિંમત;

#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  અચલ DWORD flag_overlapped = FILE_FLAG_OVERLAPPED;
#નહીંતર
  અચલ DWORD flag_overlapped = 0;
#endif

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8);
  જો( zConverted==0 ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8));
    rc = SQLITE_IOERR_NOMEM_BKPT;
    જાઓ winopenfile_out;
  }

  /* Ensure the file we are trying to open is not actually a directory. */
  જો( winIsDir(zConverted) ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8));
    rc = SQLITE_CANTOPEN_ISDIR;
    જાઓ winopenfile_out;
  }

  /* TODO: platforms.
  ** TODO: retry-on-ioerr.
  */
  h = osCreateFileW(zConverted,         /* lpFileName */
      (GENERIC_READ | (bReadonly ? 0 : GENERIC_WRITE)),  /* dwDesiredAccess */
      FILE_SHARE_READ | FILE_SHARE_WRITE,        /* dwShareMode */
      ખાલી,                                      /* lpSecurityAttributes */
      OPEN_ALWAYS,                               /* dwCreationDisposition */
      FILE_ATTRIBUTE_NORMAL|flag_overlapped,
      ખાલી
  );

  જો( h==INVALID_HANDLE_કિંમત ){
    જો( bReadonly==0 ){
      bReadonly = 1;
      rc = winHandleOpen(zUtf8, &bReadonly, &h);
    }નહીંતર{
      rc = SQLITE_CANTOPEN_BKPT;
    }
  }

 winopenfile_out:
  sqlite3_free(zConverted);
  *pbReadonly = bReadonly;
  *ph = h;
  પાછા rc;
}
 
/*
** Close pDbFd's connection to shared-memory.  Delete the underlying
** *-shm file if deleteFlag is true.
*/
સ્થિર પૂર્ણાંક winCloseSharedMemory(winFile *pDbFd, પૂર્ણાંક deleteFlag){
  winShm *p;            /* The connection to be closed */
  winShm **pp;          /* Iterator for pShmNode->pWinShmList */
  win_Shm_ગાંઠ *p_Shm_ગાંઠ; /* The underlying shared-memory file */

  p = pDbFd->pShm;
  જો( p==0 ) પાછા SQLITE_OK;
  જો( p->hShm!=INVALID_HANDLE_કિંમત ){
    osCloseHandle(p->hShm);
  }

  winShmEnterMutex();
  p_Shm_ગાંઠ = p->p_Shm_ગાંઠ;

  /* Remove this connection from the winShmNode.pWinShmList list */
  sqlite3_mutex_enter(p_Shm_ગાંઠ->mutex);
  માટે(pp=&p_Shm_ગાંઠ->pWinShmList; *pp!=p; pp=&(*pp)->pWinShmNext){}
  *pp = p->pWinShmNext;
  sqlite3_mutex_leave(p_Shm_ગાંઠ->mutex);

  winShmPurge(pDbFd->pVfs, deleteFlag);
  winShmLeaveMutex();

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  પાછા SQLITE_OK;
}

/*
** testfixture builds may set this global variable to true via a
** Tcl interface. This forces the VFS to use the locking normally
** only used for UNC paths for all files.
*/
#ifdef SQLITE_TEST
પૂર્ણાંક sqlite3_win_test_unc_locking = 0;
#નહીંતર
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
સ્થિર પૂર્ણાંક winIsUNCPath(અચલ અક્ષર *zFile){
  જો( zFile[0]=='\\' && zFile[1]=='\\' ){
    જો( zFile[2]=='?'
     && zFile[3]=='\\'
     && sqlite3Isalpha(zFile[4])
     && zFile[5]==':'
     && winIsDirSep(zFile[6])
    ){
      પાછા sqlite3_win_test_unc_locking;
    }નહીંતર{
      પાછા 1;
    }
  }
  પાછા sqlite3_win_test_unc_locking;
}

/*
** Return true if the string passed in is the name of a file on a
** remote (network-mounted) volume.
*/
સ્થિર પૂર્ણાંક winIsRemoteVolume(અચલ અક્ષર *zFile){
  WCHAR z_મૂળ[4];
  જો( strncmp(zFile, "\\\\?\\",4)==0 ) zFile += 4;
  જો( !sqlite3Isalpha(zFile[0]) ) પાછા 0;
  જો( zFile[1]!=':' ) પાછા 0;
  જો( !winIsDirSep(zFile[2]) ) પાછા 0;
  z_મૂળ[0] = (WCHAR)zFile[0];
  z_મૂળ[1] = ':';
  z_મૂળ[2] = '\\';
  z_મૂળ[3] = 0;
  પાછા osGetDriveTypeW(z_મૂળ)==DRIVE_REMOTE;
}

/*
** Open the shared-memory area associated with database file pDbFd.
*/
સ્થિર પૂર્ણાંક winOpenSharedMemory(winFile *pDbFd){
  સંરચના winShm *p;                  /* The connection to be opened */
  win_Shm_ગાંઠ *p_Shm_ગાંઠ = 0;          /* The underlying mmapped file */
  પૂર્ણાંક rc = SQLITE_OK;                /* Result code */
  win_Shm_ગાંઠ *pNew;                  /* Newly allocated winShmNode */
  પૂર્ણાંક nName;                         /* Size of zName in bytes */

  પ્રતિજ્ઞા( pDbFd->pShm==0 );    /* Not previously opened */

  /* Allocate space for the new sqlite3_shm object.  Also speculatively
  ** allocate space for a new winShmNode and filename.  */
  p = sqlite3MallocZero( કદ(*p) );
  જો( p==0 ) પાછા SQLITE_IOERR_NOMEM_BKPT;
  nName = sqlite3Strlen30(pDbFd->zPath);
  pNew = sqlite3MallocZero( કદ(*p_Shm_ગાંઠ) + (i64)nName + 17 );
  જો( pNew==0 ){
    sqlite3_free(p);
    પાછા SQLITE_IOERR_NOMEM_BKPT;
  }
  pNew->zFilename = (અક્ષર*)&pNew[1];
  pNew->hSharedShm = INVALID_HANDLE_કિંમત;
  pNew->isUnlocked = 1;
  જો( winIsUNCPath(pDbFd->zPath) || winIsRemoteVolume(pDbFd->zPath) ){
    pNew->bUseSharedLockHandle = 1;
  }નહીંતર{
    pNew->bUseSharedLockHandle = 0;
  }
  sqlite3_snprintf(nName+15, pNew->zFilename, "%s-shm", pDbFd->zPath);
  sqlite3FileSuffix3(pDbFd->zPath, pNew->zFilename);

  /* Look to see if there is an existing winShmNode that can be used.
  ** If no matching winShmNode currently exists, then create a new one.  */
  winShmEnterMutex();
  માટે(p_Shm_ગાંઠ = win_Shm_ગાંઠ_List; p_Shm_ગાંઠ; p_Shm_ગાંઠ=p_Shm_ગાંઠ->pNext){
    /* TBD need to come up with better match here.  Perhaps
    ** use FILE_ID_BOTH_DIR_INFO Structure.  */
    જો( sqlite3StrICmp(p_Shm_ગાંઠ->zFilename, pNew->zFilename)==0 ) તોડો;
  }
  જો( p_Shm_ગાંઠ==0 ){
    p_Shm_ગાંઠ = pNew;

    /* Allocate a mutex for this winShmNode object, if one is required. */
    જો( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_ગાંઠ->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      જો( p_Shm_ગાંઠ->mutex==0 ) rc = SQLITE_IOERR_NOMEM_BKPT;
    }

    /* Open a file-handle to use for mappings, and for the DMS lock. */
    જો( rc==SQLITE_OK ){
      HANDLE h = INVALID_HANDLE_કિંમત;
      p_Shm_ગાંઠ->isReadonly = sqlite3_uri_boolean(pDbFd->zPath,"readonly_shm",0);
      rc = winHandleOpen(pNew->zFilename, &p_Shm_ગાંઠ->isReadonly, &h);
      p_Shm_ગાંઠ->hSharedShm = h;
    }

    /* If successful, link the new winShmNode into the global list. If an 
    ** error occurred, free the object. */
    જો( rc==SQLITE_OK ){
      p_Shm_ગાંઠ->pNext = win_Shm_ગાંઠ_List;
      win_Shm_ગાંઠ_List = p_Shm_ગાંઠ;
      pNew = 0;
    }નહીંતર{
      sqlite3_mutex_free(p_Shm_ગાંઠ->mutex);
      જો( p_Shm_ગાંઠ->hSharedShm!=INVALID_HANDLE_કિંમત ){
        osCloseHandle(p_Shm_ગાંઠ->hSharedShm);
      }
    }
  }

  /* If no error has occurred, link the winShm object to the winShmNode and
  ** the winShm to pDbFd.  */
  જો( rc==SQLITE_OK ){
    sqlite3_mutex_enter(p_Shm_ગાંઠ->mutex);
    p->p_Shm_ગાંઠ = p_Shm_ગાંઠ;
    p->pWinShmNext = p_Shm_ગાંઠ->pWinShmList;
    p_Shm_ગાંઠ->pWinShmList = p;
#જો defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
    p->id = p_Shm_ગાંઠ->nextShmId++;
#endif
    pDbFd->pShm = p;
    sqlite3_mutex_leave(p_Shm_ગાંઠ->mutex);
  }નહીંતર જો( p ){
    sqlite3_free(p);
  }

  પ્રતિજ્ઞા( rc!=SQLITE_OK || p_Shm_ગાંઠ->isUnlocked==0 || p_Shm_ગાંઠ->nRegion==0 );
  winShmLeaveMutex();
  sqlite3_free(pNew);

  /* Open a file-handle on the *-shm file for this connection. This file-handle
  ** is only used for locking. The mapping of the *-shm file is created using
  ** the shared file handle in winShmNode.hSharedShm.  */
  જો( rc==SQLITE_OK && p_Shm_ગાંઠ->bUseSharedLockHandle==0 ){
    p->bReadonly = sqlite3_uri_boolean(pDbFd->zPath, "readonly_shm", 0);
    rc = winHandleOpen(p_Shm_ગાંઠ->zFilename, &p->bReadonly, &p->hShm);
    જો( rc!=SQLITE_OK ){
      પ્રતિજ્ઞા( p->hShm==INVALID_HANDLE_કિંમત );
      winCloseSharedMemory(pDbFd, 0);
    }
  }

  પાછા rc;
}

/*
** Close a connection to shared-memory.  Delete the underlying
** storage if deleteFlag is true.
*/
સ્થિર પૂર્ણાંક winShmUnmap(
  sqlite3_file *fd,          /* Database holding shared memory */
  પૂર્ણાંક deleteFlag             /* Delete after closing if true */
){
  પાછા winCloseSharedMemory((winFile*)fd, deleteFlag);
}

/*
** Change the lock state for a shared-memory segment.
*/
સ્થિર પૂર્ણાંક winShmLock(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  પૂર્ણાંક ofst,                  /* First lock to acquire or release */
  પૂર્ણાંક n,                     /* Number of locks to acquire or release */
  પૂર્ણાંક flags                  /* What to do with the lock */
){
  winFile *pDbFd = (winFile*)fd;        /* Connection holding shared memory */
  winShm *p = pDbFd->pShm;              /* The shared memory being locked */
  win_Shm_ગાંઠ *p_Shm_ગાંઠ;
  પૂર્ણાંક rc = SQLITE_OK;                   /* Result code */
  u16 mask = (u16)((1U<<(ofst+n)) - (1U<<ofst)); /* Mask of locks to [un]take */

  જો( p==0 ) પાછા SQLITE_IOERR_SHMLOCK;
  p_Shm_ગાંઠ = p->p_Shm_ગાંઠ;
  જો( NEVER(p_Shm_ગાંઠ==0) ) પાછા SQLITE_IOERR_SHMLOCK;

  પ્રતિજ્ઞા( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  પ્રતિજ્ઞા( n>=1 );
  પ્રતિજ્ઞા( flags==(SQLITE_SHM_LOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_LOCK | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  પ્રતિજ્ઞા( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );

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
#જો defined(SQLITE_ENABLE_SETLK_TIMEOUT) && defined(SQLITE_DEBUG)
  {
    u16 lockMask = (p->exclMask|p->sharedMask);
    પ્રતિજ્ઞા( (flags & SQLITE_SHM_UNLOCK) || pDbFd->iBusyTimeout==0 || (
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
  પ્રતિજ્ઞા( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK) 
       || 0==(p->exclMask & mask)
  );
  જો( ((flags & SQLITE_SHM_UNLOCK) && ((p->exclMask|p->sharedMask) & mask))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_LOCK) && 0==(p->sharedMask & mask))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK))
  ){
    HANDLE h = p->hShm;

    જો( flags & SQLITE_SHM_UNLOCK ){
      /* Case (a) - unlock.  */

      પ્રતિજ્ઞા( (p->exclMask & p->sharedMask)==0 );
      પ્રતિજ્ઞા( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->exclMask & mask)==mask );
      પ્રતિજ્ઞા( !(flags & SQLITE_SHM_SHARED) || (p->sharedMask & mask)==mask );

      પ્રતિજ્ઞા( !(flags & SQLITE_SHM_SHARED) || n==1 );
      જો( p_Shm_ગાંઠ->bUseSharedLockHandle ){
        h = p_Shm_ગાંઠ->hSharedShm;
        જો( flags & SQLITE_SHM_SHARED ){
          winShm *pShm;
          sqlite3_mutex_enter(p_Shm_ગાંઠ->mutex);
          માટે(pShm=p_Shm_ગાંઠ->pWinShmList; pShm; pShm=pShm->pWinShmNext){
            જો( pShm!=p && (pShm->sharedMask & mask) ){
              /* Another connection within this process is also holding this
              ** SHARED lock. So do not actually release the OS lock.  */
              h = INVALID_HANDLE_કિંમત;
              તોડો;
            }
          }
          sqlite3_mutex_leave(p_Shm_ગાંઠ->mutex);
        }
      }

      જો( h!=INVALID_HANDLE_કિંમત ){
        rc = winHandleUnlock(h, ofst+WIN_SHM_BASE, n);
      }

      /* If successful, also clear the bits in sharedMask/exclMask */
      જો( rc==SQLITE_OK ){
        p->exclMask = (p->exclMask & ~mask);
        p->sharedMask = (p->sharedMask & ~mask);
      }
    }નહીંતર{
      પૂર્ણાંક bExcl = ((flags & SQLITE_SHM_EXCLUSIVE) ? 1 : 0);
      DWORD nMs = winFileBusyTimeout(pDbFd);

      જો( p_Shm_ગાંઠ->bUseSharedLockHandle ){
        winShm *pShm;
        h = p_Shm_ગાંઠ->hSharedShm;
        sqlite3_mutex_enter(p_Shm_ગાંઠ->mutex);
        માટે(pShm=p_Shm_ગાંઠ->pWinShmList; pShm; pShm=pShm->pWinShmNext){
          જો( bExcl ){
            જો( (pShm->sharedMask|pShm->exclMask) & mask ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_કિંમત;
            }
          }નહીંતર{
            જો( pShm->sharedMask & mask ){
              h = INVALID_HANDLE_કિંમત;
            }નહીંતર જો( pShm->exclMask & mask ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_કિંમત;
            }
          }
        }
        sqlite3_mutex_leave(p_Shm_ગાંઠ->mutex);
      }

      જો( h!=INVALID_HANDLE_કિંમત ){
        rc = winHandleLockTimeout(h, ofst+WIN_SHM_BASE, n, bExcl, nMs);
      }
      જો( rc==SQLITE_OK ){
        જો( bExcl ){
          p->exclMask = (p->exclMask | mask);
        }નહીંતર{
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
  પાછા rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
સ્થિર શૂન્ય winShmBarrier(
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
સ્થિર પૂર્ણાંક winShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  પૂર્ણાંક iRegion,                    /* Region to retrieve */
  પૂર્ણાંક szRegion,                   /* Size of regions */
  પૂર્ણાંક isWrite,                    /* True to extend file if necessary */
  શૂન્ય અસ્થિર **pp              /* OUT: Mapped memory */
){
  winFile *pDbFd = (winFile*)fd;
  winShm *pShm = pDbFd->pShm;
  win_Shm_ગાંઠ *p_Shm_ગાંઠ;
  DWORD protect = PAGE_READWRITE;
  DWORD flags = FILE_MAP_WRITE | FILE_MAP_READ;
  પૂર્ણાંક rc = SQLITE_OK;

  જો( !pShm ){
    rc = winOpenSharedMemory(pDbFd);
    જો( rc!=SQLITE_OK ) પાછા rc;
    pShm = pDbFd->pShm;
    પ્રતિજ્ઞા( pShm!=0 );
  }
  p_Shm_ગાંઠ = pShm->p_Shm_ગાંઠ;

  sqlite3_mutex_enter(p_Shm_ગાંઠ->mutex);
  જો( p_Shm_ગાંઠ->isUnlocked ){
    /* Take the DMS lock. */
    પ્રતિજ્ઞા( p_Shm_ગાંઠ->nRegion==0 );
    rc = winLockSharedMemory(p_Shm_ગાંઠ, winFileBusyTimeout(pDbFd));
    જો( rc!=SQLITE_OK ) જાઓ shmpage_out;
  }

  પ્રતિજ્ઞા( szRegion==p_Shm_ગાંઠ->szRegion || p_Shm_ગાંઠ->nRegion==0 );
  જો( p_Shm_ગાંઠ->nRegion<=iRegion ){
    HANDLE hShared = p_Shm_ગાંઠ->hSharedShm;
    સંરચના ShmRegion *apNew;           /* New aRegion[] array */
    i64 nByte = ((i64)iRegion+1)*(i64)szRegion;  /* Minimum file size */
    sqlite3_int64 sz;                  /* Current size of wal-index file */

    p_Shm_ગાંઠ->szRegion = szRegion;

    /* The requested region is not mapped into this processes address space.
    ** Check to see if it has been allocated (i.e. if the wal-index file is
    ** large enough to contain the requested region).
    */
    rc = winHandleSize(hShared, &sz);
    જો( rc!=SQLITE_OK ){
      rc = winLogError(rc, osGetLastError(), "winShmMap1", pDbFd->zPath);
      જાઓ shmpage_out;
    }

    જો( sz<nByte ){
      /* The requested memory region does not exist. If isWrite is set to
      ** zero, exit early. *pp will be set to NULL and SQLITE_OK returned.
      **
      ** Alternatively, if isWrite is non-zero, use ftruncate() to allocate
      ** the requested memory region.  */
      જો( !isWrite ) જાઓ shmpage_out;
      rc = winHandleTruncate(hShared, nByte);
      જો( rc!=SQLITE_OK ){
        rc = winLogError(rc, osGetLastError(), "winShmMap2", pDbFd->zPath);
        જાઓ shmpage_out;
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (સંરચના ShmRegion*)sqlite3_realloc64(
        p_Shm_ગાંઠ->aRegion, ((i64)iRegion+1)*કદ(apNew[0])
    );
    જો( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      જાઓ shmpage_out;
    }
    p_Shm_ગાંઠ->aRegion = apNew;

    જો( p_Shm_ગાંઠ->isReadonly ){
      protect = PAGE_READONLY;
      flags = FILE_MAP_READ;
    }

    જ્યારે( p_Shm_ગાંઠ->nRegion<=iRegion ){
      HANDLE hMap = ખાલી;         /* file-mapping handle */
      શૂન્ય *pMap = 0;             /* Mapped memory region */

#ifdef SQLITE_UWP
      hMap = osCreateFileMappingFromApp(hShared, ખાલી, protect, nByte, ખાલી);
#નહીંતર
      hMap = osCreateFileMappingW(hShared, ખાલી, protect, 0, nByte, ખાલી);
#endif
      OSTRACE(("SHM-MAP-CREATE pid=%lu, region=%d, size=%lld, rc=%s\n",
               osGetCurrentProcessId(), p_Shm_ગાંઠ->nRegion, nByte,
               hMap ? "ok" : "failed"));
      જો( hMap ){
        i64 iOffset = p_Shm_ગાંઠ->nRegion*szRegion;
        પૂર્ણાંક iOffsetShift = iOffset % winSysInfo.dwAllocationGranularity;
#ifdef SQLITE_UWP
        pMap = osMapViewOfFileFromApp(hMap, flags,
            iOffset - iOffsetShift, (i64)szRegion + iOffsetShift
        );
#નહીંતર
        pMap = osMapViewOfFile(hMap, flags,
            0, iOffset - iOffsetShift, (i64)szRegion + iOffsetShift
        );
#endif
        OSTRACE(("SHM-MAP-MAP pid=%lu, region=%d, offset=%d, size=%d, rc=%s\n",
                 osGetCurrentProcessId(), p_Shm_ગાંઠ->nRegion, iOffset,
                 szRegion, pMap ? "ok" : "failed"));
      }
      જો( !pMap ){
        p_Shm_ગાંઠ->lastErrno = osGetLastError();
        rc = winLogError(SQLITE_IOERR_SHMMAP, p_Shm_ગાંઠ->lastErrno,
                         "winShmMap3", pDbFd->zPath);
        જો( hMap ) osCloseHandle(hMap);
        જાઓ shmpage_out;
      }

      p_Shm_ગાંઠ->aRegion[p_Shm_ગાંઠ->nRegion].pMap = pMap;
      p_Shm_ગાંઠ->aRegion[p_Shm_ગાંઠ->nRegion].hMap = hMap;
      p_Shm_ગાંઠ->nRegion++;
    }
  }

shmpage_out:
  જો( p_Shm_ગાંઠ->nRegion>iRegion ){
    i64 iOffset = (i64)iRegion*(i64)szRegion;
    પૂર્ણાંક iOffsetShift = iOffset % winSysInfo.dwAllocationGranularity;
    અક્ષર *p = (અક્ષર *)p_Shm_ગાંઠ->aRegion[iRegion].pMap;
    *pp = (શૂન્ય *)&p[iOffsetShift];
  }નહીંતર{
    *pp = 0;
  }
  જો( p_Shm_ગાંઠ->isReadonly && rc==SQLITE_OK ){
    rc = SQLITE_READONLY;
  }
  sqlite3_mutex_leave(p_Shm_ગાંઠ->mutex);
  પાછા rc;
}

#નહીંતર
# define winShmMap     0
# define winShmLock    0
# define winShmBarrier 0
# define winShmUnmap   0
#endif /* #ifndef SQLITE_OMIT_WAL */

/*
** Cleans up the mapped region of the specified file, if any.
*/
#જો SQLITE_MAX_MMAP_SIZE>0
સ્થિર પૂર્ણાંક winUnmapfile(winFile *pFile){
  પ્રતિજ્ઞા( pFile!=0 );
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, pMapRegion=%p, "
           "mmapSize=%lld, mmapSizeMax=%lld\n",
           osGetCurrentProcessId(), pFile, pFile->hMap, pFile->pMapRegion,
           pFile->mmapSize, pFile->mmapSizeMax));
  જો( pFile->pMapRegion ){
    જો( !osUnmapViewOfFile(pFile->pMapRegion) ){
      pFile->lastErrno = osGetLastError();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(), pFile,
               pFile->pMapRegion));
      પાછા winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winUnmapfile1", pFile->zPath);
    }
    pFile->pMapRegion = 0;
    pFile->mmapSize = 0;
  }
  જો( pFile->hMap!=ખાલી ){
    જો( !osCloseHandle(pFile->hMap) ){
      pFile->lastErrno = osGetLastError();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, rc=SQLITE_IOERR_MMAP\n",
               osGetCurrentProcessId(), pFile, pFile->hMap));
      પાછા winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winUnmapfile2", pFile->zPath);
    }
    pFile->hMap = ખાલી;
  }
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile));
  પાછા SQLITE_OK;
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
સ્થિર પૂર્ણાંક winMapfile(winFile *pFd, sqlite3_int64 nByte){
  sqlite3_int64 nMap = nByte;
  પૂર્ણાંક rc;

  પ્રતિજ્ઞા( nMap>=0 || pFd->nFetchOut==0 );
  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, size=%lld\n",
           osGetCurrentProcessId(), pFd, nByte));

  જો( pFd->nFetchOut>0 ) પાછા SQLITE_OK;

  જો( nMap<0 ){
    rc = winFileSize((sqlite3_file*)pFd, &nMap);
    જો( rc ){
      OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_IOERR_FSTAT\n",
               osGetCurrentProcessId(), pFd));
      પાછા SQLITE_IOERR_FSTAT;
    }
  }
  જો( nMap>pFd->mmapSizeMax ){
    nMap = pFd->mmapSizeMax;
  }
  nMap &= ~(sqlite3_int64)(winSysInfo.dwPageSize - 1);

  જો( nMap==0 && pFd->mmapSize>0 ){
    winUnmapfile(pFd);
  }
  જો( nMap!=pFd->mmapSize ){
    શૂન્ય *pNew = 0;
    DWORD protect = PAGE_READONLY;
    DWORD flags = FILE_MAP_READ;

    winUnmapfile(pFd);
#ifdef SQLITE_MMAP_READWRITE
    જો( (pFd->ctrlFlags & WINFILE_RDONLY)==0 ){
      protect = PAGE_READWRITE;
      flags |= FILE_MAP_WRITE;
    }
#endif
#ifdef SQLITE_UWP
    pFd->hMap = osCreateFileMappingFromApp(pFd->h, ખાલી, protect, nMap, ખાલી);
#નહીંતર
    pFd->hMap = osCreateFileMappingW(pFd->h, ખાલી, protect,
                                (DWORD)((nMap>>32) & 0xffffffff),
                                (DWORD)(nMap & 0xffffffff), ખાલી);
#endif
    જો( pFd->hMap==ખાલી ){
      pFd->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->lastErrno,
                       "winMapfile1", pFd->zPath);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-CREATE pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      પાછા SQLITE_OK;
    }
    પ્રતિજ્ઞા( (nMap % winSysInfo.dwPageSize)==0 );
    પ્રતિજ્ઞા( કદ(SIZE_T)==કદ(sqlite3_int64) || nMap<=0xffffffff );
#ifdef SQLITE_UWP
    pNew = osMapViewOfFileFromApp(pFd->hMap, flags, 0, (SIZE_T)nMap);
#નહીંતર
    pNew = osMapViewOfFile(pFd->hMap, flags, 0, 0, (SIZE_T)nMap);
#endif
    જો( pNew==ખાલી ){
      osCloseHandle(pFd->hMap);
      pFd->hMap = ખાલી;
      pFd->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->lastErrno,
                       "winMapfile2", pFd->zPath);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-MAP pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      પાછા SQLITE_OK;
    }
    pFd->pMapRegion = pNew;
    pFd->mmapSize = nMap;
  }

  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFd));
  પાછા SQLITE_OK;
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
સ્થિર પૂર્ણાંક winFetch(sqlite3_file *fd, i64 iOff, પૂર્ણાંક nAmt, શૂન્ય **pp){
#જો SQLITE_MAX_MMAP_SIZE>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */
#endif
  *pp = 0;

  OSTRACE(("FETCH pid=%lu, pFile=%p, offset=%lld, amount=%d, pp=%p\n",
           osGetCurrentProcessId(), fd, iOff, nAmt, pp));

#જો SQLITE_MAX_MMAP_SIZE>0
  જો( pFd->mmapSizeMax>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    અચલ પૂર્ણાંક nEofBuffer = 256;
    જો( pFd->pMapRegion==0 ){
      પૂર્ણાંક rc = winMapfile(pFd, -1);
      જો( rc!=SQLITE_OK ){
        OSTRACE(("FETCH pid=%lu, pFile=%p, rc=%s\n",
                 osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
        પાછા rc;
      }
    }
    જો( pFd->mmapSize >= (iOff+nAmt+nEofBuffer) ){
      પ્રતિજ્ઞા( pFd->pMapRegion!=0 );
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->nFetchOut++;
    }
  }
#endif

  OSTRACE(("FETCH pid=%lu, pFile=%p, pp=%p, *pp=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd, pp, *pp));
  પાછા SQLITE_OK;
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
સ્થિર પૂર્ણાંક winUnfetch(sqlite3_file *fd, i64 iOff, શૂન્ય *p){
#જો SQLITE_MAX_MMAP_SIZE>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  પ્રતિજ્ઞા( (p==0)==(pFd->nFetchOut==0) );

  /* If p!=0, it must match the iOff value. */
  પ્રતિજ્ઞા( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, offset=%lld, p=%p\n",
           osGetCurrentProcessId(), pFd, iOff, p));

  જો( p ){
    pFd->nFetchOut--;
  }નહીંતર{
    /* FIXME:  If Windows truly always prevents truncating or deleting a
    ** file while a mapping is held, then the following winUnmapfile() call
    ** is unnecessary can be omitted - potentially improving
    ** performance.  */
    winUnmapfile(pFd);
  }

  પ્રતિજ્ઞા( pFd->nFetchOut>=0 );
#endif

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd));
  પાછા SQLITE_OK;
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
સ્થિર અચલ sqlite3_io_methods winIoMethod = {
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
સ્થિર અચલ sqlite3_io_methods winIoNolockMethod = {
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

સ્થિર winVfsAppData winAppData = {
  &winIoMethod,       /* pMethod */
  0,                  /* pAppData */
  0                   /* bNoLock */
};

સ્થિર winVfsAppData winNolockAppData = {
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
સ્થિર પૂર્ણાંક winMakeEndInDirSep(પૂર્ણાંક nBuf, અક્ષર *zBuf){
  જો( zBuf ){
    પૂર્ણાંક nLen = sqlite3Strlen30(zBuf);
    જો( nLen>0 ){
      જો( winIsDirSep(zBuf[nLen-1]) ){
        પાછા 1;
      }નહીંતર જો( nLen+1<nBuf ){
        જો( !osGetenv ){
          zBuf[nLen] = winGetDirSep();
        }નહીંતર જો( winIsDriveLetterAndColon(zBuf) && winIsDirSep(zBuf[2]) ){
          zBuf[nLen] = '\\';
          zBuf[2]='\\';
        }નહીંતર{
          zBuf[nLen] = '/';
        }
        zBuf[nLen+1] = '\0';
        પાછા 1;
      }
    }
  }
  પાછા 0;
}

/*
** If sqlite3_temp_directory is defined, take the mutex and return true.
**
** If sqlite3_temp_directory is NULL (undefined), omit the mutex and
** return false.
*/
સ્થિર પૂર્ણાંક winTempDirDefined(શૂન્ય){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  જો( sqlite3_temp_directory!=0 ) પાછા 1;
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  પાછા 0;
}

/*
** Create a temporary file name and store the resulting pointer into pzBuf.
** The pointer returned in pzBuf must be freed via sqlite3_free().
*/
સ્થિર પૂર્ણાંક winGetTempname(sqlite3_vfs *pVfs, અક્ષર **pzBuf){
  સ્થિર અચલ અક્ષર zChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  size_t i, j;
  DWORD pid;
  પૂર્ણાંક nPre = sqlite3Strlen30(SQLITE_TEMP_FILE_PREFIX);
  i64 nMax, nBuf, nDir, nLen;
  અક્ષર *zBuf;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  SimulateIOError( પાછા SQLITE_IOERR );

  /* Allocate a temporary buffer to store the fully qualified file
  ** name for the temporary file.  If this fails, we cannot continue.
  */
  nMax = pVfs->mxPathname;
  nBuf = 2 + (i64)nMax;
  zBuf = sqlite3MallocZero( nBuf );
  જો( !zBuf ){
    OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
    પાછા SQLITE_IOERR_NOMEM_BKPT;
  }

  /* Figure out the effective temporary directory.  First, check if one
  ** has been explicitly set by the application; otherwise, use the one
  ** configured by the operating system.
  */
  nDir = nMax - (nPre + 15);
  પ્રતિજ્ઞા( nDir>0 );
  જો( winTempDirDefined() ){
    પૂર્ણાંક nDirLen = sqlite3Strlen30(sqlite3_temp_directory);
    જો( nDirLen>0 ){
      જો( !winIsDirSep(sqlite3_temp_directory[nDirLen-1]) ){
        nDirLen++;
      }
      જો( nDirLen>nDir ){
        sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
        sqlite3_free(zBuf);
        OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
        પાછા winLogError(SQLITE_ERROR, 0, "winGetTempname1", 0);
      }
      sqlite3_snprintf(nMax, zBuf, "%s", sqlite3_temp_directory);
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  }

#જો defined(__CYGWIN__)
  નહીંતર જો( osGetenv!=ખાલી ){
    સ્થિર અચલ અક્ષર *azDirs[] = {
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
    unsigned પૂર્ણાંક i;
    અચલ અક્ષર *zDir = 0;

    જો( !azDirs[0] ) azDirs[0] = osGetenv("SQLITE_TMPDIR");
    જો( !azDirs[1] ) azDirs[1] = osGetenv("TMPDIR");
    જો( !azDirs[2] ) azDirs[2] = osGetenv("TMP");
    જો( !azDirs[3] ) azDirs[3] = osGetenv("TEMP");
    જો( !azDirs[4] ) azDirs[4] = osGetenv("USERPROFILE");
    માટે(i=0; i<કદ(azDirs)/કદ(azDirs[0]); zDir=azDirs[i++]){
      LPWSTR zConverted;
      જો( zDir==0 ) ચાલુ;
      /* If the path starts with a drive letter followed by the colon
      ** character, assume it is already a native Win32 path; otherwise,
      ** it must be converted to a native Win32 path via the Cygwin API
      ** prior to using it.
      */
      {
        zConverted = winConvertFromUtf8Filename(zDir);
        જો( !zConverted ){
          sqlite3_free(zBuf);
          OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
          પાછા SQLITE_IOERR_NOMEM_BKPT;
        }
        જો( winIsDir(zConverted) ){
          sqlite3_snprintf(nMax, zBuf, "%s", zDir);
          sqlite3_free(zConverted);
          તોડો;
        }
        sqlite3_free(zConverted);
      }
    }
  }
#endif

  નહીંતર{
    અક્ષર *zMulti;
    LPWSTR zWidePath = sqlite3MallocZero( nMax*કદ(WCHAR) );
    જો( !zWidePath ){
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      પાછા SQLITE_IOERR_NOMEM_BKPT;
    }
    જો( osGetTempPathW==0 || osGetTempPathW(nMax, zWidePath)==0 ){
      sqlite3_free(zWidePath);
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_GETTEMPPATH\n"));
      પાછા winLogError(SQLITE_IOERR_GETTEMPPATH, osGetLastError(),
                         "winGetTempname2", 0);
    }
    zMulti = winUnicodeToUtf8(zWidePath);
    sqlite3_free(zWidePath);
    જો( zMulti ){
      sqlite3_snprintf(nMax, zBuf, "%s", zMulti);
      sqlite3_free(zMulti);
    }નહીંતર{
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      પાછા SQLITE_IOERR_NOMEM_BKPT;
    }
  }

  /*
  ** Check to make sure the temporary directory ends with an appropriate
  ** separator.  If it does not and there is not enough space left to add
  ** one, fail.
  */
  જો( !winMakeEndInDirSep(nDir+1, zBuf) ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    પાછા winLogError(SQLITE_ERROR, 0, "winGetTempname4", 0);
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
  જો( (nLen + nPre + 17) > nBuf ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    પાછા winLogError(SQLITE_ERROR, 0, "winGetTempname5", 0);
  }

  sqlite3_snprintf(nBuf-16-nLen, zBuf+nLen, SQLITE_TEMP_FILE_PREFIX);

  j = sqlite3Strlen30(zBuf);
  sqlite3_randomness(15, &zBuf[j]);
  pid = osGetCurrentProcessId();
  માટે(i=0; i<15; i++, j++){
    zBuf[j] += pid & 0xff;
    pid >>= 8;
    zBuf[j] = (અક્ષર)zChars[ ((unsigned અક્ષર)zBuf[j])%(કદ(zChars)-1) ];
  }
  zBuf[j] = 0;
  zBuf[j+1] = 0;
  *pzBuf = zBuf;

  OSTRACE(("TEMP-FILENAME name=%s, rc=SQLITE_OK\n", zBuf));
  પાછા SQLITE_OK;
}

/*
** Return TRUE if the named file is really a directory.  Return false if
** it is something other than a directory, or if there is any kind of memory
** allocation failure.
*/
સ્થિર પૂર્ણાંક winIsDir(LPCWSTR zConverted){
  DWORD attr;
  પૂર્ણાંક rc = 0;
  DWORD lastErrno;
  પૂર્ણાંક cnt = 0;
  WIN32_FILE_ATTRIBUTE_DATA sAttrData;
  memset(&sAttrData, 0, કદ(sAttrData));
  જ્યારે( !(rc = osGetFileAttributesExW(zConverted,
                           GetFileExInfoStandard,
                           &sAttrData)) && winRetryIoerr(&cnt, &lastErrno) ){}
  જો( !rc ){
    પાછા 0; /* Invalid name? */
  }
  attr = sAttrData.dwFileAttributes;
  પાછા (attr!=INVALID_FILE_ATTRIBUTES) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

/* forward reference */
સ્થિર પૂર્ણાંક winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  અચલ અક્ષર *zFilename,     /* Name of file to check */
  પૂર્ણાંક flags,                 /* Type of test to make on this file */
  પૂર્ણાંક *pResOut               /* OUT: Result */
);

/*
** The Windows version of xAccess() accepts an extra bit in the flags
** parameter that prevents an anti-virus retry loop.
*/
#define NORETRY 0x4000

/*
** Open a file.
*/
સ્થિર પૂર્ણાંક winOpen(
  sqlite3_vfs *pVfs,        /* Used to get maximum path length and AppData */
  અચલ અક્ષર *zName,        /* Name of the file (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  પૂર્ણાંક flags,                /* Open mode flags */
  પૂર્ણાંક *pOutFlags            /* Status return flags */
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
  અચલ અક્ષર *zUtf8Name = zName; /* Filename in UTF-8 encoding */
  પૂર્ણાંક cnt = 0;
  પૂર્ણાંક isRO = 0;              /* file is known to be accessible readonly */

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  અક્ષર *zTmpname = 0; /* For temporary filename, if necessary. */

  પૂર્ણાંક rc = SQLITE_OK;            /* Function Return Code */
#જો !defined(NDEBUG)
  પૂર્ણાંક eType = flags&0x0FFF00;  /* Type of file to open */
#endif

  પૂર્ણાંક isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  પૂર્ણાંક isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  પૂર્ણાંક isCreate     = (flags & SQLITE_OPEN_CREATE);
  પૂર્ણાંક isReadonly   = (flags & SQLITE_OPEN_READONLY);
  પૂર્ણાંક isReadWrite  = (flags & SQLITE_OPEN_READWRITE);

#ifndef NDEBUG
  પૂર્ણાંક isOpenJournal = (isCreate && (
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
  પ્રતિજ્ઞા((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
  પ્રતિજ્ઞા(isCreate==0 || isReadWrite);
  પ્રતિજ્ઞા(isExclusive==0 || isCreate);
  પ્રતિજ્ઞા(isDelete==0 || isCreate);

  /* The main DB, main journal, WAL file and super-journal are never
  ** automatically deleted. Nor are they ever temporary files.  */
  પ્રતિજ્ઞા( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_DB );
  પ્રતિજ્ઞા( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_JOURNAL );
  પ્રતિજ્ઞા( (!isDelete && zName) || eType!=SQLITE_OPEN_SUPER_JOURNAL );
  પ્રતિજ્ઞા( (!isDelete && zName) || eType!=SQLITE_OPEN_WAL );

  /* Assert that the upper layer has set one of the "file-type" flags. */
  પ્રતિજ્ઞા( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_TEMP_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_TEMP_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );

  પ્રતિજ્ઞા( pFile!=0 );
  memset(pFile, 0, કદ(winFile));
  pFile->h = INVALID_HANDLE_કિંમત;

  /* If the second argument to this function is NULL, generate a
  ** temporary file name to use
  */
  જો( !zUtf8Name ){
    પ્રતિજ્ઞા( isDelete && !isOpenJournal );
    rc = winGetTempname(pVfs, &zTmpname);
    જો( rc!=SQLITE_OK ){
      OSTRACE(("OPEN name=%s, rc=%s", zUtf8Name, sqlite3ErrName(rc)));
      પાછા rc;
    }
    zUtf8Name = zTmpname;
  }

  /* Database filenames are double-zero terminated if they are not
  ** URIs with parameters.  Hence, they can always be passed into
  ** sqlite3_uri_parameter().
  */
  પ્રતિજ્ઞા( (eType!=SQLITE_OPEN_MAIN_DB) || (flags & SQLITE_OPEN_URI) ||
       zUtf8Name[sqlite3Strlen30(zUtf8Name)+1]==0 );

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8Name);
  જો( zConverted==0 ){
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8Name));
    પાછા SQLITE_IOERR_NOMEM_BKPT;
  }

  જો( winIsDir(zConverted) ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8Name));
    પાછા SQLITE_CANTOPEN_ISDIR;
  }

  જો( isReadWrite ){
    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
  }નહીંતર{
    dwDesiredAccess = GENERIC_READ;
  }

  /* SQLITE_OPEN_EXCLUSIVE is used to make sure that a new file is
  ** created. SQLite doesn't use it to indicate "exclusive access"
  ** as it is usually understood.
  */
  જો( isExclusive ){
    /* Creates a new file, only if it does not already exist. */
    /* If the file exists, it fails. */
    dwCreationDisposition = CREATE_NEW;
  }નહીંતર જો( isCreate ){
    /* Open existing file, or create if it doesn't exist */
    dwCreationDisposition = OPEN_ALWAYS;
  }નહીંતર{
    /* Opens a file, only if it exists. */
    dwCreationDisposition = OPEN_EXISTING;
  }

  જો( 0==sqlite3_uri_boolean(zName, "exclusive", 0) ){
    dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  }નહીંતર{
    dwShareMode = 0;
  }

  જો( isDelete ){
    dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY
                               | FILE_ATTRIBUTE_HIDDEN
                               | FILE_FLAG_DELETE_ON_CLOSE;
  }નહીંતર{
    dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
  }
  કરો{
    h = osCreateFileW(zConverted,
                      dwDesiredAccess,
                      dwShareMode, ખાલી,
                      dwCreationDisposition,
                      dwFlagsAndAttributes,
                      ખાલી);
    જો( h!=INVALID_HANDLE_કિંમત ) તોડો;
    જો( isReadWrite ){
      પૂર્ણાંક rc2;
      sqlite3BeginBenignMalloc();
      rc2 = winAccess(pVfs, zUtf8Name, SQLITE_ACCESS_READ|NORETRY, &isRO);
      sqlite3EndBenignMalloc();
      જો( rc2==SQLITE_OK && isRO ) તોડો;
    }
  }જ્યારે( winRetryIoerr(&cnt, &lastErrno) );
  winLogIoerr(cnt, __LINE__);

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, rc=%s\n", h, zUtf8Name,
           dwDesiredAccess, (h==INVALID_HANDLE_કિંમત) ? "failed" : "ok"));

  જો( h==INVALID_HANDLE_કિંમત ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    જો( isReadWrite && isRO && !isExclusive ){
      પાછા winOpen(pVfs, zName, id,
         ((flags|SQLITE_OPEN_READONLY) &
                     ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE)),
         pOutFlags);
    }નહીંતર{
      pFile->lastErrno = lastErrno;
      winLogError(SQLITE_CANTOPEN, pFile->lastErrno, "winOpen", zUtf8Name);
      પાછા SQLITE_CANTOPEN_BKPT;
    }
  }

  જો( pOutFlags ){
    જો( isReadWrite ){
      *pOutFlags = SQLITE_OPEN_READWRITE;
    }નહીંતર{
      *pOutFlags = SQLITE_OPEN_READONLY;
    }
  }

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, pOutFlags=%p, *pOutFlags=%d, "
           "rc=%s\n", h, zUtf8Name, dwDesiredAccess, pOutFlags, pOutFlags ?
           *pOutFlags : 0, (h==INVALID_HANDLE_કિંમત) ? "failed" : "ok"));

  pAppData = (winVfsAppData*)pVfs->pAppData;

  sqlite3_free(zConverted);
  sqlite3_free(zTmpname);
  id->pMethods = pAppData ? pAppData->pMethod : &winIoMethod;
  pFile->pVfs = pVfs;
  pFile->h = h;
  જો( isReadonly ){
    pFile->ctrlFlags |= WINFILE_RDONLY;
  }
  જો( (flags & SQLITE_OPEN_MAIN_DB)
   && sqlite3_uri_boolean(zName, "psow", SQLITE_POWERSAFE_OVERWRITE) 
  ){
    pFile->ctrlFlags |= WINFILE_PSOW;
  }
  pFile->lastErrno = NO_ERROR;
  pFile->zPath = zName;
#જો SQLITE_MAX_MMAP_SIZE>0
  pFile->hMap = ખાલી;
  pFile->pMapRegion = 0;
  pFile->mmapSize = 0;
  pFile->mmapSizeMax = sqlite3GlobalConfig.szMmap;
#endif

  OpenCounter(+1);
  પાછા rc;
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
સ્થિર પૂર્ણાંક winDelete(
  sqlite3_vfs *pVfs,          /* Not used on win32 */
  અચલ અક્ષર *zFilename,      /* Name of file to delete */
  પૂર્ણાંક syncDir                 /* Not used on win32 */
){
  પૂર્ણાંક cnt = 0;
  પૂર્ણાંક rc;
  DWORD attr;
  DWORD lastErrno = 0;
  LPWSTR zConverted;
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(syncDir);

  SimulateIOError(પાછા SQLITE_IOERR_DELETE);
  OSTRACE(("DELETE name=%s, syncDir=%d\n", zFilename, syncDir));

  zConverted = winConvertFromUtf8Filename(zFilename);
  જો( zConverted==0 ){
    OSTRACE(("DELETE name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    પાછા SQLITE_IOERR_NOMEM_BKPT;
  }
  કરો {
    attr = osGetFileAttributesW(zConverted);
    જો ( attr==INVALID_FILE_ATTRIBUTES ){
      lastErrno = osGetLastError();
      જો( lastErrno==ERROR_FILE_NOT_FOUND
       || lastErrno==ERROR_PATH_NOT_FOUND ){
        rc = SQLITE_IOERR_DELETE_NOENT; /* Already gone? */
      }નહીંતર{
        rc = SQLITE_ERROR;
      }
      તોડો;
    }
    જો ( attr&FILE_ATTRIBUTE_DIRECTORY ){
      rc = SQLITE_ERROR; /* Files only. */
      તોડો;
    }
    જો ( osDeleteFileW(zConverted) ){
      rc = SQLITE_OK; /* Deleted OK. */
      તોડો;
    }
    જો ( !winRetryIoerr(&cnt, &lastErrno) ){
      rc = SQLITE_ERROR; /* No more retries. */
      તોડો;
    }
  } જ્યારે(1);
  જો( rc && rc!=SQLITE_IOERR_DELETE_NOENT ){
    rc = winLogError(SQLITE_IOERR_DELETE, lastErrno, "winDelete", zFilename);
  }નહીંતર{
    winLogIoerr(cnt, __LINE__);
  }
  sqlite3_free(zConverted);
  OSTRACE(("DELETE name=%s, rc=%s\n", zFilename, sqlite3ErrName(rc)));
  પાછા rc;
}

/*
** Check the existence and status of a file.
*/
સ્થિર પૂર્ણાંક winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  અચલ અક્ષર *zFilename,     /* Name of file to check */
  પૂર્ણાંક flags,                 /* Type of test to make on this file */
  પૂર્ણાંક *pResOut               /* OUT: Result */
){
  DWORD attr;
  પૂર્ણાંક rc = 0;
  પૂર્ણાંક cnt = 0;
  DWORD lastErrno = 0;
  LPWSTR zConverted;
  પૂર્ણાંક noRetry = 0;           /* Do not use winRetryIoerr() */
  WIN32_FILE_ATTRIBUTE_DATA sAttrData;
  UNUSED_PARAMETER(pVfs);

  જો( (flags & NORETRY)!=0 ){
    noRetry = 1;
    flags &= ~NORETRY;
  }  

  SimulateIOError( પાછા SQLITE_IOERR_ACCESS; );
  OSTRACE(("ACCESS name=%s, flags=%x, pResOut=%p\n",
           zFilename, flags, pResOut));

  જો( zFilename==0 ){
    *pResOut = 0;
    OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
             zFilename, pResOut, *pResOut));
    પાછા SQLITE_OK;
  }

  zConverted = winConvertFromUtf8Filename(zFilename);
  જો( zConverted==0 ){
    OSTRACE(("ACCESS name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    પાછા SQLITE_IOERR_NOMEM_BKPT;
  }
  memset(&sAttrData, 0, કદ(sAttrData));
  જ્યારે( !(rc = osGetFileAttributesExW(zConverted,
                           GetFileExInfoStandard,
                           &sAttrData))
     && !noRetry
     && winRetryIoerr(&cnt, &lastErrno)
  ){ /* Loop until true */}
  જો( rc ){
    /* For an SQLITE_ACCESS_EXISTS query, treat a zero-length file
    ** as if it does not exist.
    */
    જો(    flags==SQLITE_ACCESS_EXISTS
        && sAttrData.n_File_Size_ઉપલું==0
        && sAttrData.n_File_Size_નીચલું==0 ){
      attr = INVALID_FILE_ATTRIBUTES;
    }નહીંતર{
      attr = sAttrData.dwFileAttributes;
    }
  }નહીંતર{
    જો( noRetry ) lastErrno = osGetLastError();
    winLogIoerr(cnt, __LINE__);
    જો( lastErrno!=ERROR_FILE_NOT_FOUND && lastErrno!=ERROR_PATH_NOT_FOUND ){
      sqlite3_free(zConverted);
      પાછા winLogError(SQLITE_IOERR_ACCESS, lastErrno, "winAccess",
                         zFilename);
    }નહીંતર{
      attr = INVALID_FILE_ATTRIBUTES;
    }
  }
  sqlite3_free(zConverted);
  પસંદગી( flags ){
    સ્થિતિ SQLITE_ACCESS_READ:
    સ્થિતિ SQLITE_ACCESS_EXISTS:
      rc = attr!=INVALID_FILE_ATTRIBUTES;
      તોડો;
    સ્થિતિ SQLITE_ACCESS_READWRITE:
      rc = attr!=INVALID_FILE_ATTRIBUTES &&
             (attr & FILE_ATTRIBUTE_READONLY)==0;
      તોડો;
    મૂળભૂત:
      પ્રતિજ્ઞા(!"Invalid flags argument");
  }
  *pResOut = rc;
  OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           zFilename, pResOut, *pResOut));
  પાછા SQLITE_OK;
}

/*
** Returns non-zero if the specified path name starts with the "long path"
** prefix.
*/
સ્થિર BOOL winIsLongPathPrefix(
  અચલ અક્ષર *zPathname
){
  પાછા ( zPathname[0]=='\\' && zPathname[1]=='\\'
        && zPathname[2]=='?'  && zPathname[3]=='\\' );
}

/*
** Returns non-zero if the specified path name starts with a drive letter
** followed by a colon character.
*/
સ્થિર BOOL winIsDriveLetterAndColon(
  અચલ અક્ષર *zPathname
){
  પાછા ( sqlite3Isalpha(zPathname[0]) && zPathname[1]==':' );
}

#ifdef _WIN32
/*
** Returns non-zero if the specified path name should be used verbatim.  If
** non-zero is returned from this function, the calling function must simply
** use the provided path name verbatim -OR- resolve it into a full path name
** using the GetFullPathName Win32 API function (if available).
*/
સ્થિર BOOL winIsVerbatimPathname(
  અચલ અક્ષર *zPathname
){
  /*
  ** If the path name starts with a forward slash or a backslash, it is either
  ** a legal UNC name, a volume relative path, or an absolute path name in the
  ** "Unix" format on Windows.  There is no easy way to differentiate between
  ** the final two cases; therefore, we return the safer return value of TRUE
  ** so that callers of this function will simply use it verbatim.
  */
  જો ( winIsDirSep(zPathname[0]) ){
    પાછા TRUE;
  }

  /*
  ** If the path name starts with a letter and a colon it is either a volume
  ** relative path or an absolute path.  Callers of this function must not
  ** attempt to treat it as a relative path name (i.e. they should simply use
  ** it verbatim).
  */
  જો ( winIsDriveLetterAndColon(zPathname) ){
    પાછા TRUE;
  }

  /*
  ** If we get to this point, the path name should almost certainly be a purely
  ** relative one (i.e. not a UNC name, not absolute, and not volume relative).
  */
  પાછા FALSE;
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
સ્થિર શૂન્ય winSimplifyName(અક્ષર *z){
  પૂર્ણાંક i, j;
  માટે(i=j=0; z[i]; ++i){
    જો( winIsDirSep(z[i]) ){
#જો !defined(SQLITE_TEST)
      /* Some test-cases assume that "./foo" and "foo" are different */
      જો( z[i+1]=='.' && winIsDirSep(z[i+2]) ){
        ++i;
        ચાલુ;
      }
#endif
      જો( !z[i+1] || (winIsDirSep(z[i+1]) && (i!=0)) ){
        ચાલુ;
      }
      z[j++] = osGetenv?'/':'\\';
    }નહીંતર{
      z[j++] = z[i];
    }
  }
  જ્યારે(j<i) z[j++] = '\0';
}

#define SQLITE_MAX_SYMLINKS 100

સ્થિર પૂર્ણાંક mkFullPathname(
  અચલ અક્ષર *zPath,              /* Input path */
  અક્ષર *zOut,                     /* Output buffer */
  પૂર્ણાંક nOut                        /* Allocated size of buffer zOut */
){
  પૂર્ણાંક nPath = sqlite3Strlen30(zPath);
  પૂર્ણાંક iOff = 0;
  જો( zPath[0]!='/' ){
    જો( osGetcwd(zOut, nOut-2)==0 ){
      પાછા winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno, "getcwd", zPath);
    }
    iOff = sqlite3Strlen30(zOut);
    zOut[iOff++] = '/';
  }
  જો( (iOff+nPath+1)>nOut ){
    /* SQLite assumes that xFullPathname() nul-terminates the output buffer
    ** even if it returns an error.  */
    zOut[iOff] = '\0';
    પાછા SQLITE_CANTOPEN_BKPT;
  }
  sqlite3_snprintf(nOut-iOff, &zOut[iOff], "%s", zPath);
  પાછા SQLITE_OK;
}
#endif /* __CYGWIN__ */

/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
સ્થિર પૂર્ણાંક winFullPathnameNoMutex(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  અચલ અક્ષર *zRelative,        /* Possibly relative input path */
  પૂર્ણાંક nFull,                    /* Size of output buffer in bytes */
  અક્ષર *zFull                   /* Output buffer */
){
  પૂર્ણાંક nByte;
  LPWSTR zConverted;
  અક્ષર *zOut;

  /* If this path name begins with "/X:" or "\\?\", where "X" is any
  ** alphabetic character, discard the initial "/" from the pathname.
  */
  જો( zRelative[0]=='/' && (winIsDriveLetterAndColon(zRelative+1)
       || winIsLongPathPrefix(zRelative+1)) ){
    zRelative++;
  }

  SimulateIOError( પાછા SQLITE_ERROR );

#ifdef __CYGWIN__
  જો( osGetcwd ){
    zFull[nFull-1] = '\0';
    જો( !winIsDriveLetterAndColon(zRelative) || !winIsDirSep(zRelative[2]) ){
      પૂર્ણાંક rc = SQLITE_OK;
      પૂર્ણાંક nLink = 1;              /* Number of symbolic links followed so far */
      અચલ અક્ષર *zIn = zRelative; /* Input path for each iteration of loop */
      અક્ષર *zDel = 0;
      સંરચના stat buf;

      UNUSED_PARAMETER(pVfs);

      કરો {
        /* Call lstat() on path zIn. Set bLink to true if the path
        ** is a symbolic link, or false otherwise.  */
        પૂર્ણાંક bLink = 0;
        જો( osLstat && osReadlink ) {
          જો( osLstat(zIn, &buf)!=0 ){
            પૂર્ણાંક myErrno = osErrno;
            જો( myErrno!=ENOENT ){
              rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)myErrno,
                               "lstat", zIn);
            }
          }નહીંતર{
            bLink = ((buf.st_mode & 0170000) == 0120000);
          }

          જો( bLink ){
            જો( zDel==0 ){
              zDel = sqlite3MallocZero(nFull);
              જો( zDel==0 ) rc = SQLITE_NOMEM;
            }નહીંતર જો( ++nLink>SQLITE_MAX_SYMLINKS ){
              rc = SQLITE_CANTOPEN_BKPT;
            }

            જો( rc==SQLITE_OK ){
              nByte = osReadlink(zIn, zDel, nFull-1);
              જો( nByte ==(DWORD)-1 ){
                rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno,
                                 "readlink", zIn);
              }નહીંતર{
                જો( zDel[0]!='/' ){
                  પૂર્ણાંક n;
                  માટે(n = sqlite3Strlen30(zIn); n>0 && zIn[n-1]!='/'; n--);
                  જો( nByte+n+1>nFull ){
                    rc = SQLITE_CANTOPEN_BKPT;
                  }નહીંતર{
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

        પ્રતિજ્ઞા( rc!=SQLITE_OK || zIn!=zFull || zIn[0]=='/' );
        જો( rc==SQLITE_OK && zIn!=zFull ){
          rc = mkFullPathname(zIn, zFull, nFull);
        }
        જો( bLink==0 ) તોડો;
        zIn = zFull;
      }જ્યારે( rc==SQLITE_OK );

      sqlite3_free(zDel);
      winSimplifyName(zFull);
      પાછા rc;
    }
  }
#endif /* __CYGWIN__ */

#જો defined(_WIN32)
  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing. This function could fail if, for example, the
  ** current working directory has been unlinked.
  */
  SimulateIOError( પાછા SQLITE_ERROR );
  જો ( sqlite3_data_directory && !winIsVerbatimPathname(zRelative) ){
    /*
    ** NOTE: We are dealing with a relative path name and the data
    **       directory has been set.  Therefore, use it as the basis
    **       for converting the relative path name to an absolute
    **       one by prepending the data directory and a backslash.
    */
    sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s%c%s",
                     sqlite3_data_directory, winGetDirSep(), zRelative);
    પાછા SQLITE_OK;
  }
#endif
  zConverted = winConvertFromUtf8Filename(zRelative);
  જો( zConverted==0 ){
    પાછા SQLITE_IOERR_NOMEM_BKPT;
  }
  {
    LPWSTR zTemp;
    nByte = osGetFullPathNameW(zConverted, 0, 0, 0);
    જો( nByte==0 ){
      sqlite3_free(zConverted);
      પાછા winLogError(SQLITE_CANTOPEN_FULLPATH, osGetLastError(),
                         "winFullPathname1", zRelative);
    }
    nByte += 3;
    zTemp = sqlite3MallocZero( (u64)nByte*કદ(WCHAR) );
    જો( zTemp==0 ){
      sqlite3_free(zConverted);
      પાછા SQLITE_IOERR_NOMEM_BKPT;
    }
    nByte = osGetFullPathNameW(zConverted, nByte, zTemp, 0);
    જો( nByte==0 ){
      sqlite3_free(zConverted);
      sqlite3_free(zTemp);
      પાછા winLogError(SQLITE_CANTOPEN_FULLPATH, osGetLastError(),
                         "winFullPathname2", zRelative);
    }
    sqlite3_free(zConverted);
    zOut = winUnicodeToUtf8(zTemp);
    sqlite3_free(zTemp);
  }
  જો( zOut ){
#ifdef __CYGWIN__
    જો( memcmp(zOut, "\\\\?\\", 4) ){
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut);
    }નહીંતર જો( memcmp(zOut+4, "UNC\\", 4) ){
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut+4);
    }નહીંતર{
      અક્ષર *p = zOut+6;
      *p = '\\';
      જો( osGetcwd ){
        /* On Cygwin, UNC paths use forward slashes */
        જ્યારે( *p ){
          જો( *p=='\\' ) *p = '/';
          ++p;
        }
      }
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut+6);
    }
#નહીંતર
    sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut);
#endif /* __CYGWIN__ */
    sqlite3_free(zOut);
    પાછા SQLITE_OK;
  }નહીંતર{
    પાછા SQLITE_IOERR_NOMEM_BKPT;
  }
}
સ્થિર પૂર્ણાંક winFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  અચલ અક્ષર *zRelative,        /* Possibly relative input path */
  પૂર્ણાંક nFull,                    /* Size of output buffer in bytes */
  અક્ષર *zFull                   /* Output buffer */
){
  પૂર્ણાંક rc;
  MUTEX_LOGIC( sqlite3_mutex *pMutex; )
  MUTEX_LOGIC( pMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR); )
  sqlite3_mutex_enter(pMutex);
  rc = winFullPathnameNoMutex(pVfs, zRelative, nFull, zFull);
  sqlite3_mutex_leave(pMutex);
  પાછા rc;
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
સ્થિર શૂન્ય *winDlOpen(sqlite3_vfs *pVfs, અચલ અક્ષર *zFilename){
  HANDLE h = 0;
  LPWSTR zConverted = winConvertFromUtf8Filename(zFilename);
  UNUSED_PARAMETER(pVfs);
  જો( zConverted==0 ){
    OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (શૂન્ય*)0));
    પાછા 0;
  }
  h = osLoadLibraryW ? osLoadLibraryW(zConverted) : 0;
  OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (શૂન્ય*)h));
  sqlite3_free(zConverted);
  પાછા (શૂન્ય*)h;
}
સ્થિર શૂન્ય winDlError(sqlite3_vfs *pVfs, પૂર્ણાંક nBuf, અક્ષર *zBufOut){
  UNUSED_PARAMETER(pVfs);
  win_Get_Last_Error_સંદેશ(osGetLastError(), nBuf, zBufOut);
}
સ્થિર શૂન્ય (*winDlSym(sqlite3_vfs *pVfs,શૂન્ય *pH,અચલ અક્ષર *zSym))(શૂન્ય){
  FARPROC proc = 0;
  UNUSED_PARAMETER(pVfs);
  proc = osGetProcAddressA ? osGetProcAddressA((HANDLE)pH, zSym) : 0;
  OSTRACE(("DLSYM handle=%p, symbol=%s, address=%p\n",
           (શૂન્ય*)pH, zSym, (શૂન્ય*)proc));
  પાછા (શૂન્ય(*)(શૂન્ય))proc;
}
સ્થિર શૂન્ય winDlClose(sqlite3_vfs *pVfs, શૂન્ય *pHandle){
  UNUSED_PARAMETER(pVfs);
  જો( osFreeLibrary!=0 ) osFreeLibrary((HANDLE)pHandle);
  OSTRACE(("DLCLOSE handle=%p\n", (શૂન્ય*)pHandle));
}
#નહીંતર /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #define winDlOpen  0
  #define winDlError 0
  #define winDlSym   0
  #define winDlClose 0
#endif

/* State information for the randomness gatherer. */
પ્રકારવ્યાખ્યા સંરચના EntropyGatherer EntropyGatherer;
સંરચના EntropyGatherer {
  unsigned અક્ષર *a;   /* Gather entropy into this buffer */
  પૂર્ણાંક na;             /* Size of a[] in bytes */
  પૂર્ણાંક i;              /* XOR next input into a[i] */
  પૂર્ણાંક nXor;           /* Number of XOR operations done */
};

#જો !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
/* Mix sz bytes of entropy into p. */
સ્થિર શૂન્ય xorMemory(EntropyGatherer *p, unsigned અક્ષર *x, પૂર્ણાંક sz){
  પૂર્ણાંક j, k;
  માટે(j=0, k=p->i; j<sz; j++){
    p->a[k++] ^= x[j];
    જો( k>=p->na ) k = 0;
  }
  p->i = k;
  p->nXor += sz;
}
#endif /* !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS) */

/*
** Write up to nBuf bytes of randomness into zBuf.
*/
સ્થિર પૂર્ણાંક winRandomness(sqlite3_vfs *pVfs, પૂર્ણાંક nBuf, અક્ષર *zBuf){
#જો defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS)
  UNUSED_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  પાછા nBuf;
#નહીંતર
  EntropyGatherer e;
  UNUSED_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  e.a = (unsigned અક્ષર*)zBuf;
  e.na = nBuf;
  e.nXor = 0;
  e.i = 0;
  {
    FILETIME x;
    osGetSystemTimeAsFileTime(&x);
    xorMemory(&e, (unsigned અક્ષર*)&x, કદ(x));
  }
  {
    DWORD pid = osGetCurrentProcessId();
    xorMemory(&e, (unsigned અક્ષર*)&pid, કદ(pid));
  }
  {
    ULONGLONG cnt = osGetTickCount64();
    xorMemory(&e, (unsigned અક્ષર*)&cnt, કદ(cnt));
  }
  {
    LARGE_INTEGER i;
    osQueryPerformanceCounter(&i);
    xorMemory(&e, (unsigned અક્ષર*)&i, કદ(i));
  }
#જો SQLITE_WIN32_USE_UUID
#ifdef SQLITE_UWP
# error SQLITE_WIN32_USE_UUID is incompatible with SQLITE_UWP
#endif
  {
    UUID id;
    memset(&id, 0, કદ(id));
    osUuidCreate(&id);
    xorMemory(&e, (unsigned અક્ષર*)&id, કદ(id));
    memset(&id, 0, કદ(UUID));
    osUuidCreateSequential(&id);
    xorMemory(&e, (unsigned અક્ષર*)&id, કદ(id));
  }
#endif /* SQLITE_WIN32_USE_UUID */
  પાછા e.nXor>nBuf ? nBuf : e.nXor;
#endif /* defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS) */
}


/*
** Sleep for a little while.  Return the amount of time slept.
*/
સ્થિર પૂર્ણાંક winSleep(sqlite3_vfs *pVfs, પૂર્ણાંક microsec){
  osSleep((microsec+999)/1000);
  UNUSED_PARAMETER(pVfs);
  પાછા ((microsec+999)/1000)*1000;
}

/*
** The following variable, if set to a non-zero value, is interpreted as
** the number of seconds since 1970 and is used to set the result of
** sqlite3OsCurrentTime() during testing.
*/
#ifdef SQLITE_TEST
પૂર્ણાંક sqlite3_current_time = 0;  /* Fake system time in seconds since 1970. */
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
સ્થિર પૂર્ણાંક winCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *piNow){
  /* FILETIME structure is a 64-bit value representing the number of
     100-nanosecond intervals since January 1, 1601 (= JD 2305813.5).
  */
  FILETIME ft;
  સ્થિર અચલ sqlite3_int64 winFiletimeEpoch = 23058135*(sqlite3_int64)8640000;
#ifdef SQLITE_TEST
  સ્થિર અચલ sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
#endif
  /* 2^32 - to avoid use of LL and warnings in gcc */
  સ્થિર અચલ sqlite3_int64 max32_Bit_કિંમત =
      (sqlite3_int64)2000000000 + (sqlite3_int64)2000000000 +
      (sqlite3_int64)294967296;

  osGetSystemTimeAsFileTime( &ft );
  *piNow = winFiletimeEpoch +
            ((((sqlite3_int64)ft.dw_ઉપલું_Date_Time)*max32_Bit_કિંમત) +
               (sqlite3_int64)ft.dw_નીચલું_Date_Time)/(sqlite3_int64)10000;

#ifdef SQLITE_TEST
  જો( sqlite3_current_time ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_time + unixEpoch;
  }
#endif
  UNUSED_PARAMETER(pVfs);
  પાછા SQLITE_OK;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
સ્થિર પૂર્ણાંક winCurrentTime(sqlite3_vfs *pVfs, દ્વિગુણ *prNow){
  પૂર્ણાંક rc;
  sqlite3_int64 i;
  rc = winCurrentTimeInt64(pVfs, &i);
  જો( !rc ){
    *prNow = i/86400000.0;
  }
  પાછા rc;
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
સ્થિર પૂર્ણાંક winGetLastError(sqlite3_vfs *pVfs, પૂર્ણાંક nBuf, અક્ષર *zBuf){
  DWORD e = osGetLastError();
  UNUSED_PARAMETER(pVfs);
  જો( nBuf>0 ) win_Get_Last_Error_સંદેશ(e, nBuf, zBuf);
  પાછા e;
}

/*
** Initialize and deinitialize the operating system interface.
*/
પૂર્ણાંક sqlite3_os_init(શૂન્ય){
  સ્થિર sqlite3_vfs winVfs = {
    3,                     /* iVersion */
    કદ(winFile),       /* szOsFile */
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
  સ્થિર sqlite3_vfs winLongPathVfs = {
    3,                     /* iVersion */
    કદ(winFile),       /* szOsFile */
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
  સ્થિર sqlite3_vfs winNolockVfs = {
    3,                     /* iVersion */
    કદ(winFile),       /* szOsFile */
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
  સ્થિર sqlite3_vfs winLongPathNolockVfs = {
    3,                     /* iVersion */
    કદ(winFile),       /* szOsFile */
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
  પ્રતિજ્ઞા( ArraySize(aSyscall)==60 );
  પ્રતિજ્ઞા( strcmp(aSyscall[0].zName,"AreFileApisANSI")==0 );
  પ્રતિજ્ઞા( strcmp(aSyscall[8].zName,"FreeLibrary")==0 );
  પ્રતિજ્ઞા( strcmp(aSyscall[16].zName,"GetSystemInfo")==0 );
  પ્રતિજ્ઞા( strcmp(aSyscall[24].zName,"HeapReAlloc")==0 );
  પ્રતિજ્ઞા( strcmp(aSyscall[32].zName,"MapViewOfFileFromApp")==0 );
  પ્રતિજ્ઞા( strcmp(aSyscall[40].zName,"UnmapViewOfFile")==0 );
  પ્રતિજ્ઞા( strcmp(aSyscall[48].zName,"UuidCreate")==0 );
  પ્રતિજ્ઞા( strcmp(aSyscall[56].zName,"lstat")==0 );

  /* get memory map allocation granularity */
  memset(&winSysInfo, 0, કદ(SYSTEM_INFO));
  osGetSystemInfo(&winSysInfo);
  પ્રતિજ્ઞા( winSysInfo.dwAllocationGranularity>0 );
  પ્રતિજ્ઞા( winSysInfo.dwPageSize>0 );

  sqlite3_vfs_register(&winVfs, 1);
  sqlite3_vfs_register(&winLongPathVfs, 0);
  sqlite3_vfs_register(&winNolockVfs, 0);
  sqlite3_vfs_register(&winLongPathNolockVfs, 0);
#ifndef SQLITE_OMIT_WAL
  winBigLock = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif

  પાછા SQLITE_OK;
}

પૂર્ણાંક sqlite3_os_end(શૂન્ય){
#ifndef SQLITE_OMIT_WAL
  winBigLock = 0;
#endif
  પાછા SQLITE_OK;
}

#endif /* SQLITE_OS_WIN */
