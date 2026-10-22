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
#समावेश "sqliteInt.h"
#अगर SQLITE_OS_WIN               /* This file is used for Windows only */

/*
** Include code that is common to all os_*.c files
*/
#समावेश "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#समावेश "os_win.h"

/*
** This constant should already be defined (in the "WinDef.h" SDK file).
*/
#अगर_अघोषित MAX_PATH
#  define MAX_PATH                      (260)
#पूर्ण_अगर

/*
** Maximum pathname length (in chars) for Win32.  This should normally be
** MAX_PATH.
*/
#अगर_अघोषित SQLITE_WIN32_MAX_PATH_CHARS
#  define SQLITE_WIN32_MAX_PATH_CHARS   (MAX_PATH)
#पूर्ण_अगर

/*
** This constant should already be defined (in the "WinNT.h" SDK file).
*/
#अगर_अघोषित UNICODE_STRING_MAX_CHARS
#  define UNICODE_STRING_MAX_CHARS      (32767)
#पूर्ण_अगर

/*
** Maximum pathname length (in chars) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS.
*/
#अगर_अघोषित SQLITE_WINNT_MAX_PATH_CHARS
#  define SQLITE_WINNT_MAX_PATH_CHARS   (UNICODE_STRING_MAX_CHARS)
#पूर्ण_अगर

/*
** Maximum pathname length (in bytes) for Win32.  The MAX_PATH macro is in
** characters, so we allocate 4 bytes per character assuming worst-case of
** 4-bytes-per-character for UTF8.
*/
#अगर_अघोषित SQLITE_WIN32_MAX_PATH_BYTES
#  define SQLITE_WIN32_MAX_PATH_BYTES   (SQLITE_WIN32_MAX_PATH_CHARS*4)
#पूर्ण_अगर

/*
** Maximum pathname length (in bytes) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS * sizeof(WCHAR).
*/
#अगर_अघोषित SQLITE_WINNT_MAX_PATH_BYTES
#  define SQLITE_WINNT_MAX_PATH_BYTES   \
                            (माप(WCHAR) * SQLITE_WINNT_MAX_PATH_CHARS)
#पूर्ण_अगर

/*
** Maximum error message length (in chars) for WinRT.
*/
#अगर_अघोषित SQLITE_WIN32_MAX_ERRMSG_CHARS
#  define SQLITE_WIN32_MAX_ERRMSG_CHARS (1024)
#पूर्ण_अगर

/*
** Returns non-zero if the character should be treated as a directory
** separator.
*/
#अगर_अघोषित winIsDirSep
#  define winIsDirSep(a)                (((a) == '/') || ((a) == '\\'))
#पूर्ण_अगर

/*
** This macro is used when a local variable is set to a value that is
** [sometimes] not used by the code (e.g. via conditional compilation).
*/
#अगर_अघोषित UNUSED_VARIABLE_मूल्य
#  define UNUSED_VARIABLE_मूल्य(x)      (व्योम)(x)
#पूर्ण_अगर

/*
** Returns the character that should be used as the directory separator.
*/
#अगर_अघोषित winGetDirSep
#  define winGetDirSep()                '\\'
#पूर्ण_अगर

/*
** Some older Microsoft compilers lack the following definition.
*/
#अगर_अघोषित INVALID_FILE_ATTRIBUTES
# define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#पूर्ण_अगर

#अगर_अघोषित FILE_FLAG_MASK
# define FILE_FLAG_MASK          (0xFF3C0000)
#पूर्ण_अगर

#अगर_अघोषित FILE_ATTRIBUTE_MASK
# define FILE_ATTRIBUTE_MASK     (0x0003FFF7)
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_WAL
/* Forward references to structures used for WAL */
प्रकार काष्ठा winShm winShm;           /* A connection to shared-memory */
प्रकार काष्ठा win_Shm_गाँठ win_Shm_गाँठ;   /* A region of shared-memory */
#पूर्ण_अगर

/*
** The winFile structure is a subclass of sqlite3_file* specific to the win32
** portability layer.
*/
प्रकार काष्ठा winFile winFile;
काष्ठा winFile {
  स्थिर sqlite3_io_methods *pMethod; /*** Must be first ***/
  sqlite3_vfs *pVfs;      /* The VFS used to open this file */
  HANDLE h;               /* Handle for accessing the file */
  u8 locktype;            /* Type of lock currently held on this file */
  लघु sharedLockByte;   /* Randomly chosen byte used as a shared lock */
  u8 ctrlFlags;           /* Flags.  See WINFILE_* below */
  DWORD lastErrno;        /* The Windows errno from the last I/O error */
#अगर_अघोषित SQLITE_OMIT_WAL
  winShm *pShm;           /* Instance of shared memory on this file */
#पूर्ण_अगर
  स्थिर अक्षर *zPath;      /* Full pathname of this file */
  पूर्णांक szChunk;            /* Chunk size configured by FCNTL_CHUNK_SIZE */
#अगर SQLITE_MAX_MMAP_आकार>0
  पूर्णांक nFetchOut;                /* Number of outstanding xFetch references */
  HANDLE hMap;                  /* Handle for accessing memory mapping */
  व्योम *pMapRegion;             /* Area memory mapped */
  sqlite3_int64 mmap_आकार;       /* Size of mapped region */
  sqlite3_int64 mmap_आकार_Max;    /* Configured FCNTL_MMAP_SIZE value */
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_SETLK_TIMEOUT
  DWORD iBusyTimeout;        /* Wait this many millisec on locks */
  पूर्णांक bBlockOnConnect;
#पूर्ण_अगर
};

#अगर_घोषित SQLITE_ENABLE_SETLK_TIMEOUT
# define winFileBusyTimeout(pDbFd) pDbFd->iBusyTimeout
#अन्यथा
# define winFileBusyTimeout(pDbFd) 0
#पूर्ण_अगर

/*
** The winVfsAppData structure is used for the pAppData member for all of the
** Win32 VFS variants.
*/
प्रकार काष्ठा win_Vfs_App_तथ्य win_Vfs_App_तथ्य;
काष्ठा win_Vfs_App_तथ्य {
  स्थिर sqlite3_io_methods *pMethod; /* The file I/O methods to use. */
  व्योम *p_App_तथ्य;                    /* The extra pAppData, if any. */
  BOOL bNoLock;                      /* Non-zero if locking is disabled. */
};

/*
** Allowed values for winFile.ctrlFlags
*/
#घोषणा WINFILE_RDONLY          0x02   /* Connection is read only */
#घोषणा WINFILE_PERSIST_WAL     0x04   /* Persistent WAL mode */
#घोषणा WINFILE_PSOW            0x10   /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */

/*
 * The size of the buffer used by sqlite3_win32_write_debug().
 */
#अगर_अघोषित SQLITE_WIN32_DBG_BUF_आकार
#  define SQLITE_WIN32_DBG_BUF_आकार   ((पूर्णांक)(4096-माप(DWORD)))
#पूर्ण_अगर

/*
 * If compiled with SQLITE_WIN32_MALLOC on Windows, we will use the
 * various Win32 API heap functions instead of our own.
 */
#अगर_घोषित SQLITE_WIN32_MALLOC

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
#अगर_अघोषित SQLITE_WIN32_HEAP_CREATE
#  define SQLITE_WIN32_HEAP_CREATE        (TRUE)
#पूर्ण_अगर

/*
 * This is the maximum possible initial size of the Win32-specific heap, in
 * bytes.
 */
#अगर_अघोषित SQLITE_WIN32_HEAP_MAX_INIT_आकार
#  define SQLITE_WIN32_HEAP_MAX_INIT_आकार (4294967295U)
#पूर्ण_अगर

/*
 * This is the extra space for the initial size of the Win32-specific heap,
 * in bytes.  This value may be zero.
 */
#अगर_अघोषित SQLITE_WIN32_HEAP_INIT_EXTRA
#  define SQLITE_WIN32_HEAP_INIT_EXTRA  (4194304)
#पूर्ण_अगर

/*
 * Calculate the maximum legal cache size, in pages, based on the maximum
 * possible initial heap size and the default page size, setting aside the
 * needed extra space.
 */
#अगर_अघोषित SQLITE_WIN32_MAX_CACHE_आकार
#  define SQLITE_WIN32_MAX_CACHE_आकार   (((SQLITE_WIN32_HEAP_MAX_INIT_आकार) - \
                                          (SQLITE_WIN32_HEAP_INIT_EXTRA)) / \
                                         (SQLITE_DEFAULT_PAGE_आकार))
#पूर्ण_अगर

/*
 * This is cache size used in the calculation of the initial size of the
 * Win32-specific heap.  It cannot be negative.
 */
#अगर_अघोषित SQLITE_WIN32_CACHE_आकार
#  अगर SQLITE_DEFAULT_CACHE_आकार>=0
#    define SQLITE_WIN32_CACHE_आकार     (SQLITE_DEFAULT_CACHE_आकार)
#  अन्यथा
#    define SQLITE_WIN32_CACHE_आकार     (-(SQLITE_DEFAULT_CACHE_आकार))
#  endif
#पूर्ण_अगर

/*
 * Make sure that the calculated cache size, in pages, cannot cause the
 * initial size of the Win32-specific heap to exceed the maximum amount
 * of memory that can be specified in the call to HeapCreate.
 */
#अगर SQLITE_WIN32_CACHE_आकार>SQLITE_WIN32_MAX_CACHE_आकार
#  undef SQLITE_WIN32_CACHE_आकार
#  define SQLITE_WIN32_CACHE_आकार       (2000)
#पूर्ण_अगर

/*
 * The initial size of the Win32-specific heap.  This value may be zero.
 */
#अगर_अघोषित SQLITE_WIN32_HEAP_INIT_आकार
#  define SQLITE_WIN32_HEAP_INIT_आकार   ((SQLITE_WIN32_CACHE_आकार) * \
                                         (SQLITE_DEFAULT_PAGE_आकार) + \
                                         (SQLITE_WIN32_HEAP_INIT_EXTRA))
#पूर्ण_अगर

/*
 * The maximum size of the Win32-specific heap.  This value may be zero.
 */
#अगर_अघोषित SQLITE_WIN32_HEAP_MAX_आकार
#  define SQLITE_WIN32_HEAP_MAX_आकार    (0)
#पूर्ण_अगर

/*
 * The extra flags to use in calls to the Win32 heap APIs.  This value may be
 * zero for the default behavior.
 */
#अगर_अघोषित SQLITE_WIN32_HEAP_FLAGS
#  define SQLITE_WIN32_HEAP_FLAGS       (0)
#पूर्ण_अगर


/*
** The winMemData structure stores information required by the Win32-specific
** sqlite3_mem_methods implementation.
*/
प्रकार काष्ठा win_Mem_तथ्य win_Mem_तथ्य;
काष्ठा win_Mem_तथ्य {
#अगर_अघोषित न_संशोधन
  u32 magic1;   /* Magic number to detect structure corruption. */
#पूर्ण_अगर
  HANDLE hHeap; /* The handle to our heap. */
  BOOL bOwned;  /* Do we own the heap (i.e. destroy it on shutdown)? */
#अगर_अघोषित न_संशोधन
  u32 magic2;   /* Magic number to detect structure corruption. */
#पूर्ण_अगर
};

#अगर_अघोषित न_संशोधन
#घोषणा WINMEM_MAGIC1     0x42b2830b
#घोषणा WINMEM_MAGIC2     0xbd4d7cf4
#पूर्ण_अगर

अटल काष्ठा win_Mem_तथ्य win_mem_तथ्य = {
#अगर_अघोषित न_संशोधन
  WINMEM_MAGIC1,
#पूर्ण_अगर
  शून्य, FALSE
#अगर_अघोषित न_संशोधन
  ,WINMEM_MAGIC2
#पूर्ण_अगर
};

#अगर_अघोषित न_संशोधन
#घोषणा winMemAssertMagic1() निश्चित( win_mem_तथ्य.magic1==WINMEM_MAGIC1 )
#घोषणा winMemAssertMagic2() निश्चित( win_mem_तथ्य.magic2==WINMEM_MAGIC2 )
#घोषणा winMemAssertMagic()  winMemAssertMagic1(); winMemAssertMagic2();
#अन्यथा
#घोषणा winMemAssertMagic()
#पूर्ण_अगर

#घोषणा win_Mem_Get_तथ्य_Ptr()  &win_mem_तथ्य
#घोषणा winMemGetHeap()     win_mem_तथ्य.hHeap
#घोषणा winMemGetOwned()    win_mem_तथ्य.bOwned

अटल व्योम *winMemMalloc(पूर्णांक nBytes);
अटल व्योम winMemFree(व्योम *pPrior);
अटल व्योम *winMemRealloc(व्योम *pPrior, पूर्णांक nBytes);
अटल पूर्णांक win_Mem_आकार(व्योम *p);
अटल पूर्णांक winMemRoundup(पूर्णांक n);
अटल पूर्णांक winMemInit(व्योम *p_App_तथ्य);
अटल व्योम winMemShutdown(व्योम *p_App_तथ्य);

स्थिर sqlite3_mem_methods *sqlite3MemGetWin32(व्योम);
#पूर्ण_अगर /* SQLITE_WIN32_MALLOC */

#अगर_अघोषित SYSCALL
#  define SYSCALL sqlite3_syscall_ptr
#पूर्ण_अगर

/*
** Many system calls are accessed through pointer-to-functions so that
** they may be overridden at runtime to facilitate fault injection during
** testing and sandboxing.  The following array holds the names and pointers
** to all overrideable system calls.
*/
अटल काष्ठा win_syscall {
  स्थिर अक्षर *zName;            /* Name of the system call */
  sqlite3_syscall_ptr pCurrent; /* Current value of the system call */
  sqlite3_syscall_ptr pDefault; /* Default value */
} aSyscall[] = {
  { "AreFileApisANSI",         (SYSCALL)AreFileApisANSI,         0 },
#घोषणा osAreFileApisANSI ((BOOL(WINAPI*)(VOID))aSyscall[0].pCurrent)

  { "CloseHandle",             (SYSCALL)CloseHandle,             0 },
#घोषणा osCloseHandle ((BOOL(WINAPI*)(HANDLE))aSyscall[1].pCurrent)


  { "CreateFileW",             (SYSCALL)CreateFileW,             0 },
#घोषणा osCreateFileW ((HANDLE(WINAPI*)(LPCWSTR,DWORD,DWORD, \
        LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE))aSyscall[2].pCurrent)

#अगर defined(SQLITE_UWP)
  { "CreateFileMappingFromApp",(SYSCALL)CreateFileMappingFromApp,0 },
#अन्यथा
  { "CreateFileMappingFromApp",(SYSCALL)0,                       0 },
#पूर्ण_अगर
#घोषणा osCreateFileMappingFromApp ((HANDLE(WINAPI*)(HANDLE,\
        PSECURITY_ATTRIBUTES, \
        ULONG,ULONG64,PCWSTR))aSyscall[3].pCurrent)

#अगर !defined(SQLITE_UWP)
  { "CreateFileMappingW",      (SYSCALL)CreateFileMappingW,      0 },
#अन्यथा
  { "CreateFileMappingW",      (SYSCALL)0,                       0 },
#पूर्ण_अगर
#घोषणा osCreateFileMappingW ((HANDLE(WINAPI*)(HANDLE,LPSECURITY_ATTRIBUTES, \
        DWORD,DWORD,DWORD,LPCWSTR))aSyscall[4].pCurrent)

  { "DeleteFileW",             (SYSCALL)DeleteFileW,             0 },
#घोषणा osDeleteFileW ((BOOL(WINAPI*)(LPCWSTR))aSyscall[5].pCurrent)

  { "FlushFileBuffers",        (SYSCALL)FlushFileBuffers,        0 },
#घोषणा osFlushFileBuffers ((BOOL(WINAPI*)(HANDLE))aSyscall[6].pCurrent)

  { "FormatMessageW",          (SYSCALL)FormatMessageW,          0 },
#घोषणा osFormatMessageW ((DWORD(WINAPI*)(DWORD,LPCVOID,DWORD,DWORD,LPWSTR, \
        DWORD,बहु_सूची*))aSyscall[7].pCurrent)

#अगर !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "FreeLibrary",             (SYSCALL)FreeLibrary,             0 },
#अन्यथा
  { "FreeLibrary",             (SYSCALL)0,                       0 },
#पूर्ण_अगर

#घोषणा osFreeLibrary ((BOOL(WINAPI*)(HMODULE))aSyscall[8].pCurrent)

  { "GetCurrentProcessId",     (SYSCALL)GetCurrentProcessId,     0 },
#घोषणा osGetCurrentProcessId ((DWORD(WINAPI*)(VOID))aSyscall[9].pCurrent)

  { "GetFileAttributesW",      (SYSCALL)GetFileAttributesW,      0 },
#घोषणा osGetFileAttributesW ((DWORD(WINAPI*)(LPCWSTR))aSyscall[10].pCurrent)

  { "GetFileAttributesExW",    (SYSCALL)GetFileAttributesExW,    0 },
#घोषणा osGetFileAttributesExW ((BOOL(WINAPI*)(LPCWSTR,GET_FILEEX_INFO_LEVELS, \
        LPVOID))aSyscall[11].pCurrent)

  { "GetFileSizeEx",           (SYSCALL)Get_File_आकार_Ex,           0 },
#घोषणा os_Get_File_आकार_Ex ((BOOL(WINAPI*)(HANDLE, \
                        PLARGE_INTEGER))aSyscall[12].pCurrent)

  { "GetFullPathNameW",        (SYSCALL)GetFullPathNameW,        0 },
#घोषणा osGetFullPathNameW ((DWORD(WINAPI*)(LPCWSTR,DWORD,LPWSTR, \
        LPWSTR*))aSyscall[13].pCurrent)

  { "GetLastError",            (SYSCALL)GetLastError,            0 },
#घोषणा osGetLastError ((DWORD(WINAPI*)(VOID))aSyscall[14].pCurrent)

#अगर !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "GetProcAddressA",         (SYSCALL)GetProcAddress,          0 },
#अन्यथा
  { "GetProcAddressA",         (SYSCALL)0,                       0 },
#पूर्ण_अगर
#घोषणा osGetProcAddressA ((FARPROC(WINAPI*)(HMODULE, \
        LPCSTR))aSyscall[15].pCurrent)

  { "GetSystemInfo",           (SYSCALL)GetSystemInfo,           0 },
#घोषणा osGetSystemInfo ((VOID(WINAPI*)(LPSYSTEM_INFO))aSyscall[16].pCurrent)

  { "GetSystemTimeAsFileTime", (SYSCALL)GetSystemTimeAsFileTime, 0 },
#घोषणा osGetSystemTimeAsFileTime ((VOID(WINAPI*)( \
        LPFILETIME))aSyscall[17].pCurrent)

#अगर_घोषित SQLITE_UWP
  { "GetTempPathW",            (SYSCALL)0,                       0 },
#अन्यथा
  { "GetTempPathW",            (SYSCALL)Get_अस्थायी_Path_W,            0 },
#पूर्ण_अगर
#घोषणा os_Get_अस्थायी_Path_W ((DWORD(WINAPI*)(DWORD,LPWSTR))aSyscall[18].pCurrent)

  { "GetTickCount64",          (SYSCALL)GetTickCount64,          0 },
#घोषणा osGetTickCount64 ((ULONGLONG(WINAPI*)(VOID))aSyscall[19].pCurrent)

#अगर_घोषित SQLITE_UWP
  { "HeapAlloc",               (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapAlloc",               (SYSCALL)HeapAlloc,               0 },
#पूर्ण_अगर
#घोषणा osHeapAlloc ((LPVOID(WINAPI*)(HANDLE,DWORD, \
        आकार_T))aSyscall[20].pCurrent)

#अगर_घोषित SQLITE_UWP
  { "HeapCreate",              (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapCreate",              (SYSCALL)HeapCreate,              0 },
#पूर्ण_अगर
#घोषणा osHeapCreate ((HANDLE(WINAPI*)(DWORD,आकार_T, \
        आकार_T))aSyscall[21].pCurrent)

#अगर_घोषित SQLITE_UWP
  { "HeapDestroy",             (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapDestroy",             (SYSCALL)HeapDestroy,             0 },
#पूर्ण_अगर
#घोषणा osHeapDestroy ((BOOL(WINAPI*)(HANDLE))aSyscall[22].pCurrent)

#अगर_घोषित SQLITE_UWP
  { "HeapFree",                (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapFree",                (SYSCALL)HeapFree,                0 },
#पूर्ण_अगर
#घोषणा osHeapFree ((BOOL(WINAPI*)(HANDLE,DWORD,LPVOID))aSyscall[23].pCurrent)

#अगर_घोषित SQLITE_UWP
  { "HeapReAlloc",             (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapReAlloc",             (SYSCALL)HeapReAlloc,             0 },
#पूर्ण_अगर
#घोषणा osHeapReAlloc ((LPVOID(WINAPI*)(HANDLE,DWORD,LPVOID, \
        आकार_T))aSyscall[24].pCurrent)

#अगर_घोषित SQLITE_UWP
  { "HeapSize",                (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapSize",                (SYSCALL)Heap_आकार,                0 },
#पूर्ण_अगर
#घोषणा os_Heap_आकार ((आकार_T(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[25].pCurrent)

#अगर_घोषित SQLITE_UWP
  { "HeapValidate",            (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapValidate",            (SYSCALL)HeapValidate,            0 },
#पूर्ण_अगर
#घोषणा osHeapValidate ((BOOL(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[26].pCurrent)

#अगर_घोषित SQLITE_UWP
  { "HeapCompact",             (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapCompact",             (SYSCALL)HeapCompact,             0 },
#पूर्ण_अगर
#घोषणा osHeapCompact ((UINT(WINAPI*)(HANDLE,DWORD))aSyscall[27].pCurrent)


#अगर !defined(SQLITE_OMIT_LOAD_EXTENSION) && !defined(SQLITE_UWP)
  { "LoadLibraryW",            (SYSCALL)LoadLibraryW,            0 },
#अन्यथा
  { "LoadLibraryW",            (SYSCALL)0,                       0 },
#पूर्ण_अगर
#घोषणा osLoadLibraryW ((HMODULE(WINAPI*)(LPCWSTR))aSyscall[28].pCurrent)

  { "LocalFree",               (SYSCALL)LocalFree,               0 },
#घोषणा osLocalFree ((HLOCAL(WINAPI*)(HLOCAL))aSyscall[29].pCurrent)

  { "LockFileEx",              (SYSCALL)LockFileEx,              0 },
#घोषणा osLockFileEx ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[30].pCurrent)

#अगर (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_आकार>0) \
 && !defined(SQLITE_UWP)
  { "MapViewOfFile",           (SYSCALL)MapViewOfFile,           0 },
#अन्यथा
  { "MapViewOfFile",           (SYSCALL)0,                       0 },
#पूर्ण_अगर
#घोषणा osMapViewOfFile ((LPVOID(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        आकार_T))aSyscall[31].pCurrent)

#अगर (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_आकार>0) \
 && defined(SQLITE_UWP)
  { "MapViewOfFileFromApp",    (SYSCALL)MapViewOfFileFromApp,   0 },
#अन्यथा
  { "MapViewOfFileFromApp",    (SYSCALL)0,                      0 },
#पूर्ण_अगर
#घोषणा osMapViewOfFileFromApp ((LPVOID(WINAPI*)(HANDLE,ULONG,ULONG64, \
        आकार_T))aSyscall[32].pCurrent)

  { "MultiByteToWideChar",     (SYSCALL)MultiByteToWideChar,     0 },
#घोषणा osMultiByteToWideChar ((पूर्णांक(WINAPI*)(UINT,DWORD,LPCSTR,पूर्णांक,LPWSTR, \
        पूर्णांक))aSyscall[33].pCurrent)

  { "QueryPerformanceCounter", (SYSCALL)Query_Performance_गणक, 0 },
#घोषणा os_Query_Performance_गणक ((BOOL(WINAPI*)( \
        LARGE_INTEGER*))aSyscall[34].pCurrent)

  { "ReadFile",                (SYSCALL)ReadFile,                0 },
#घोषणा osReadFile ((BOOL(WINAPI*)(HANDLE,LPVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[35].pCurrent)

  { "SetEndOfFile",            (SYSCALL)SetEndOfFile,            0 },
#घोषणा osSetEndOfFile ((BOOL(WINAPI*)(HANDLE))aSyscall[36].pCurrent)

  { "SetFilePointerEx",        (SYSCALL)SetFilePointerEx,        0 },
#घोषणा osSetFilePointerEx ((BOOL(WINAPI*)(HANDLE,LARGE_INTEGER,\
        PLARGE_INTEGER,DWORD))aSyscall[37].pCurrent)

  { "Sleep",                   (SYSCALL)Sleep,                   0 },
#घोषणा osSleep ((VOID(WINAPI*)(DWORD))aSyscall[38].pCurrent)

  { "UnlockFileEx",            (SYSCALL)UnlockFileEx,            0 },
#घोषणा osUnlockFileEx ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[39].pCurrent)

#अगर !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_आकार>0
  { "UnmapViewOfFile",         (SYSCALL)UnmapViewOfFile,         0 },
#अन्यथा
  { "UnmapViewOfFile",         (SYSCALL)0,                       0 },
#पूर्ण_अगर
#घोषणा osUnmapViewOfFile ((BOOL(WINAPI*)(LPCVOID))aSyscall[40].pCurrent)

  { "WideCharToMultiByte",     (SYSCALL)WideCharToMultiByte,     0 },
#घोषणा osWideCharToMultiByte ((पूर्णांक(WINAPI*)(UINT,DWORD,LPCWSTR,पूर्णांक,LPSTR,पूर्णांक, \
        LPCSTR,LPBOOL))aSyscall[41].pCurrent)

  { "WriteFile",               (SYSCALL)WriteFile,               0 },
#घोषणा osWriteFile ((BOOL(WINAPI*)(HANDLE,LPCVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[42].pCurrent)

  { "WaitForSingleObject",     (SYSCALL)WaitForSingleObject,     0 },
#घोषणा osWaitForSingleObject ((DWORD(WINAPI*)(HANDLE, \
        DWORD))aSyscall[43].pCurrent)

  { "WaitForSingleObjectEx",   (SYSCALL)WaitForSingleObjectEx,   0 },
#घोषणा osWaitForSingleObjectEx ((DWORD(WINAPI*)(HANDLE,DWORD, \
        BOOL))aSyscall[44].pCurrent)

  { "OutputDebugStringA",      (SYSCALL)OutputDebugStringA,      0 },
#घोषणा osOutputDebugStringA ((VOID(WINAPI*)(LPCSTR))aSyscall[45].pCurrent)

  { "GetProcessHeap",          (SYSCALL)GetProcessHeap,          0 },
#घोषणा osGetProcessHeap ((HANDLE(WINAPI*)(VOID))aSyscall[46].pCurrent)

/*
** NOTE: On some sub-platforms, the InterlockedCompareExchange "function"
**       is really just a macro that uses a compiler intrinsic (e.g. x64).
**       So do not try to make this is into a redefinable interface.
*/
#अगर defined(InterlockedCompareExchange)
  { "InterlockedCompareExchange", (SYSCALL)0,                    0 },
#घोषणा osInterlockedCompareExchange InterlockedCompareExchange
#अन्यथा
  { "InterlockedCompareExchange", (SYSCALL)InterlockedCompareExchange, 0 },
#घोषणा osInterlockedCompareExchange ((LONG(WINAPI*)(LONG अस्थिर*,\
        LONG,LONG))aSyscall[47].pCurrent)
#पूर्ण_अगर /* defined(InterlockedCompareExchange) */

#अगर SQLITE_WIN32_USE_UUID
  { "UuidCreate",               (SYSCALL)UuidCreate,             0 },
#अन्यथा
  { "UuidCreate",               (SYSCALL)0,                      0 },
#पूर्ण_अगर
#घोषणा osUuidCreate ((RPC_STATUS(RPC_ENTRY*)(UUID*))aSyscall[48].pCurrent)

#अगर SQLITE_WIN32_USE_UUID
  { "UuidCreateSequential",     (SYSCALL)UuidCreateSequential,   0 },
#अन्यथा
  { "UuidCreateSequential",     (SYSCALL)0,                      0 },
#पूर्ण_अगर
#घोषणा osUuidCreateSequential \
        ((RPC_STATUS(RPC_ENTRY*)(UUID*))aSyscall[49].pCurrent)

#अगर !defined(SQLITE_NO_SYNC) && SQLITE_MAX_MMAP_आकार>0
  { "FlushViewOfFile",          (SYSCALL)FlushViewOfFile,        0 },
#अन्यथा
  { "FlushViewOfFile",          (SYSCALL)0,                      0 },
#पूर्ण_अगर
#घोषणा osFlushViewOfFile \
        ((BOOL(WINAPI*)(LPCVOID,आकार_T))aSyscall[50].pCurrent)

#अगर_घोषित SQLITE_ENABLE_SETLK_TIMEOUT
  { "CreateEvent",              (SYSCALL)CreateEvent,            0 },
#अन्यथा
  { "CreateEvent",              (SYSCALL)0,                      0 },
#पूर्ण_अगर
#घोषणा osCreateEvent ((HANDLE(WINAPI*)(LPSECURITY_ATTRIBUTES,BOOL, \
                        BOOL,LPCSTR))aSyscall[51].pCurrent)

#अगर_घोषित SQLITE_ENABLE_SETLK_TIMEOUT
  { "CancelIo",                 (SYSCALL)CancelIo,               0 },
#अन्यथा
  { "CancelIo",                 (SYSCALL)0,                      0 },
#पूर्ण_अगर
#घोषणा osCancelIo ((BOOL(WINAPI*)(HANDLE))aSyscall[52].pCurrent)

#अगर_अघोषित _WIN32
  { "getenv",                   (SYSCALL)दो_पर्या,                 0 },
#अन्यथा
  { "getenv",                   (SYSCALL)0,                      0 },
#पूर्ण_अगर
#घोषणा osGetenv ((स्थिर अक्षर *(*)(स्थिर अक्षर *))aSyscall[53].pCurrent)

#अगर_अघोषित _WIN32
  { "getcwd",                   (SYSCALL)getcwd,                 0 },
#अन्यथा
  { "getcwd",                   (SYSCALL)0,                      0 },
#पूर्ण_अगर
#घोषणा osGetcwd ((अक्षर*(*)(अक्षर*,माप_प्रकार))aSyscall[54].pCurrent)

#अगर_अघोषित _WIN32
  { "readlink",                 (SYSCALL)readlink,               0 },
#अन्यथा
  { "readlink",                 (SYSCALL)0,                      0 },
#पूर्ण_अगर
#घोषणा osReadlink ((ssize_t(*)(स्थिर अक्षर*,अक्षर*,माप_प्रकार))aSyscall[55].pCurrent)

#अगर_अघोषित _WIN32
  { "lstat",                    (SYSCALL)lstat,                  0 },
#अन्यथा
  { "lstat",                    (SYSCALL)0,                      0 },
#पूर्ण_अगर
#घोषणा osLstat ((पूर्णांक(*)(स्थिर अक्षर*,काष्ठा stat*))aSyscall[56].pCurrent)

#अगर_अघोषित _WIN32
  { "__errno",                  (SYSCALL)__errno,                0 },
#अन्यथा
  { "__errno",                  (SYSCALL)0,                      0 },
#पूर्ण_अगर
#घोषणा osErrno (*((पूर्णांक*(*)(व्योम))aSyscall[57].pCurrent)())

#अगर_अघोषित _WIN32
  { "cygwin_conv_path",         (SYSCALL)cygwin_conv_path,       0 },
#अन्यथा
  { "cygwin_conv_path",         (SYSCALL)0,                      0 },
#पूर्ण_अगर
#घोषणा osCygwin_conv_path ((माप_प्रकार(*)(अचिन्हित पूर्णांक, \
    स्थिर व्योम *, व्योम *, माप_प्रकार))aSyscall[58].pCurrent)

  { "GetDriveTypeW",            (SYSCALL)GetDriveTypeW,          0 },
#घोषणा osGetDriveTypeW ((UINT(WINAPI*)(LPCWSTR))aSyscall[59].pCurrent)

}; /* End of the overrideable system calls */

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "win32" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
अटल पूर्णांक winSetSystemCall(
  sqlite3_vfs *pNotUsed,        /* The VFS pointer.  Not used */
  स्थिर अक्षर *zName,            /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
){
  अचिन्हित पूर्णांक i;
  पूर्णांक rc = SQLITE_NOTFOUND;

  UNUSED_PARAMETER(pNotUsed);
  अगर( zName==0 ){
    /* If no zName is given, restore all system calls to their default
    ** settings and return NULL
    */
    rc = SQLITE_OK;
    क्रम(i=0; i<माप(aSyscall)/माप(aSyscall[0]); i++){
      अगर( aSyscall[i].pDefault ){
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  }अन्यथा{
    /* If zName is specified, operate on only the one system call
    ** specified.
    */
    क्रम(i=0; i<माप(aSyscall)/माप(aSyscall[0]); i++){
      अगर( म_भेद(zName, aSyscall[i].zName)==0 ){
        अगर( aSyscall[i].pDefault==0 ){
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = SQLITE_OK;
        अगर( pNewFunc==0 ) pNewFunc = aSyscall[i].pDefault;
        aSyscall[i].pCurrent = pNewFunc;
        अवरोध;
      }
    }
  }
  वापस rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
अटल sqlite3_syscall_ptr winGetSystemCall(
  sqlite3_vfs *pNotUsed,
  स्थिर अक्षर *zName
){
  अचिन्हित पूर्णांक i;

  UNUSED_PARAMETER(pNotUsed);
  क्रम(i=0; i<माप(aSyscall)/माप(aSyscall[0]); i++){
    अगर( म_भेद(zName, aSyscall[i].zName)==0 ) वापस aSyscall[i].pCurrent;
  }
  वापस 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
अटल स्थिर अक्षर *win_अग्र_System_Call(sqlite3_vfs *p, स्थिर अक्षर *zName){
  पूर्णांक i = -1;

  UNUSED_PARAMETER(p);
  अगर( zName ){
    क्रम(i=0; i<Array_आकार(aSyscall)-1; i++){
      अगर( म_भेद(zName, aSyscall[i].zName)==0 ) अवरोध;
    }
  }
  क्रम(i++; i<Array_आकार(aSyscall); i++){
    अगर( aSyscall[i].pCurrent!=0 ) वापस aSyscall[i].zName;
  }
  वापस 0;
}

#अगर_घोषित SQLITE_WIN32_MALLOC
#अगर_घोषित SQLITE_UWP
# error SQLITE_WIN32_MALLOC is incompatible with SQLITE_UWP
#पूर्ण_अगर
/*
** If a Win32 native heap has been configured, this function will attempt to
** compact it.  Upon success, SQLITE_OK will be returned.  Upon failure, one
** of SQLITE_NOMEM, SQLITE_ERROR, or SQLITE_NOTFOUND will be returned.  The
** "pnLargest" argument, if non-zero, will be used to return the size of the
** largest committed free block in the heap, in bytes.
*/
पूर्णांक sqlite3_win32_compact_heap(LPUINT pnLargest){
  पूर्णांक rc = SQLITE_OK;
  UINT nLargest = 0;
  HANDLE hHeap;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  निश्चित( hHeap!=0 );
  निश्चित( hHeap!=INVALID_HANDLE_मूल्य );
#अगर defined(SQLITE_WIN32_MALLOC_VALIDATE)
  निश्चित( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, शून्य) );
#पूर्ण_अगर
  अगर( (nLargest=osHeapCompact(hHeap, SQLITE_WIN32_HEAP_FLAGS))==0 ){
    DWORD lastErrno = osGetLastError();
    अगर( lastErrno==NO_ERROR ){
      sqlite3_log(SQLITE_NOMEM, "failed to HeapCompact (no space), heap=%p",
                  (व्योम*)hHeap);
      rc = SQLITE_NOMEM_BKPT;
    }अन्यथा{
      sqlite3_log(SQLITE_ERROR, "failed to HeapCompact (%lu), heap=%p",
                  osGetLastError(), (व्योम*)hHeap);
      rc = SQLITE_ERROR;
    }
  }
  अगर( pnLargest ) *pnLargest = nLargest;
  वापस rc;
}

/*
** If a Win32 native heap has been configured, this function will attempt to
** destroy and recreate it.  If the Win32 native heap is not isolated and/or
** the sqlite3_memory_used() function does not return zero, SQLITE_BUSY will
** be returned and no changes will be made to the Win32 native heap.
*/
पूर्णांक sqlite3_win32_reset_heap(){
  पूर्णांक rc;
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; ) /* The main static mutex */
  MUTEX_LOGIC( sqlite3_mutex *pMem; )    /* The memsys static mutex */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  MUTEX_LOGIC( pMem = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3_mutex_enter(pMem);
  winMemAssertMagic();
  अगर( winMemGetHeap()!=शून्य && winMemGetOwned() && sqlite3_memory_used()==0 ){
    /*
    ** At this point, there should be no outstanding memory allocations on
    ** the heap.  Also, since both the main and memsys locks are currently
    ** being held by us, no other function (i.e. from another thread) should
    ** be able to even access the heap.  Attempt to destroy and recreate our
    ** isolated Win32 native heap now.
    */
    निश्चित( winMemGetHeap()!=शून्य );
    निश्चित( winMemGetOwned() );
    निश्चित( sqlite3_memory_used()==0 );
    winMemShutdown(win_Mem_Get_तथ्य_Ptr());
    निश्चित( winMemGetHeap()==शून्य );
    निश्चित( !winMemGetOwned() );
    निश्चित( sqlite3_memory_used()==0 );
    rc = winMemInit(win_Mem_Get_तथ्य_Ptr());
    निश्चित( rc!=SQLITE_OK || winMemGetHeap()!=शून्य );
    निश्चित( rc!=SQLITE_OK || winMemGetOwned() );
    निश्चित( rc!=SQLITE_OK || sqlite3_memory_used()==0 );
  }अन्यथा{
    /*
    ** The Win32 native heap cannot be modified because it may be in use.
    */
    rc = SQLITE_BUSY;
  }
  sqlite3_mutex_leave(pMem);
  sqlite3_mutex_leave(pMainMtx);
  वापस rc;
}
#पूर्ण_अगर /* SQLITE_WIN32_MALLOC */

#अगर_घोषित _WIN32
/*
** This function outputs the specified (ANSI) string to the Win32 debugger
** (if available).  Undocumented.  Might go away at any moment.
*/
व्योम sqlite3_win32_write_debug(स्थिर अक्षर *zBuf, पूर्णांक nBuf){
  अक्षर zDbgBuf[SQLITE_WIN32_DBG_BUF_आकार];
  पूर्णांक nMin = MIN(nBuf, (SQLITE_WIN32_DBG_BUF_आकार - 1)); /* may be negative. */
  अगर( nMin<-1 ) nMin = -1; /* all negative values become -1. */
  निश्चित( nMin==-1 || nMin==0 || nMin<SQLITE_WIN32_DBG_BUF_आकार );
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !zBuf ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस;
  }
#पूर्ण_अगर
  अगर( nMin>0 ){
    स_रखो(zDbgBuf, 0, SQLITE_WIN32_DBG_BUF_आकार);
    स_नकल(zDbgBuf, zBuf, nMin);
    osOutputDebugStringA(zDbgBuf);
  }अन्यथा{
    osOutputDebugStringA(zBuf);
  }
}

व्योम sqlite3_win32_sleep(DWORD milliseconds){
  osSleep(milliseconds);
}

#अगर SQLITE_MAX_WORKER_THREADS>0 && SQLITE_THREADSAFE>0
DWORD sqlite3Win32Wait(HANDLE hObject){
  DWORD rc;
  जबतक( (rc = osWaitForSingleObjectEx(hObject, INFINITE,
                                       TRUE))==WAIT_IO_COMPLETION ){}
  वापस rc;
}
#पूर्ण_अगर
#पूर्ण_अगर /* _WIN32 */

#अगर_घोषित SQLITE_WIN32_MALLOC
/*
** Allocate nBytes of memory.
*/
अटल व्योम *winMemMalloc(पूर्णांक nBytes){
  HANDLE hHeap;
  व्योम *p;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  निश्चित( hHeap!=0 );
  निश्चित( hHeap!=INVALID_HANDLE_मूल्य );
#अगर defined(SQLITE_WIN32_MALLOC_VALIDATE)
  निश्चित( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, शून्य) );
#पूर्ण_अगर
  निश्चित( nBytes>=0 );
  p = osHeapAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, (आकार_T)nBytes);
  अगर( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapAlloc %u bytes (%lu), heap=%p",
                nBytes, osGetLastError(), (व्योम*)hHeap);
  }
  वापस p;
}

/*
** Free memory.
*/
अटल व्योम winMemFree(व्योम *pPrior){
  HANDLE hHeap;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  निश्चित( hHeap!=0 );
  निश्चित( hHeap!=INVALID_HANDLE_मूल्य );
#अगर defined(SQLITE_WIN32_MALLOC_VALIDATE)
  निश्चित( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) );
#पूर्ण_अगर
  अगर( !pPrior ) वापस; /* Passing NULL to HeapFree is undefined. */
  अगर( !osHeapFree(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapFree block %p (%lu), heap=%p",
                pPrior, osGetLastError(), (व्योम*)hHeap);
  }
}

/*
** Change the size of an existing memory allocation
*/
अटल व्योम *winMemRealloc(व्योम *pPrior, पूर्णांक nBytes){
  HANDLE hHeap;
  व्योम *p;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  निश्चित( hHeap!=0 );
  निश्चित( hHeap!=INVALID_HANDLE_मूल्य );
#अगर defined(SQLITE_WIN32_MALLOC_VALIDATE)
  निश्चित( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior) );
#पूर्ण_अगर
  निश्चित( nBytes>=0 );
  अगर( !pPrior ){
    p = osHeapAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, (आकार_T)nBytes);
  }अन्यथा{
    p = osHeapReAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, pPrior, (आकार_T)nBytes);
  }
  अगर( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to %s %u bytes (%lu), heap=%p",
                pPrior ? "HeapReAlloc" : "HeapAlloc", nBytes, osGetLastError(),
                (व्योम*)hHeap);
  }
  वापस p;
}

/*
** Return the size of an outstanding allocation, in bytes.
*/
अटल पूर्णांक win_Mem_आकार(व्योम *p){
  HANDLE hHeap;
  आकार_T n;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  निश्चित( hHeap!=0 );
  निश्चित( hHeap!=INVALID_HANDLE_मूल्य );
#अगर defined(SQLITE_WIN32_MALLOC_VALIDATE)
  निश्चित( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, p) );
#पूर्ण_अगर
  अगर( !p ) वापस 0;
  n = os_Heap_आकार(hHeap, SQLITE_WIN32_HEAP_FLAGS, p);
  अगर( n==(आकार_T)-1 ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapSize block %p (%lu), heap=%p",
                p, osGetLastError(), (व्योम*)hHeap);
    वापस 0;
  }
  वापस (पूर्णांक)n;
}

/*
** Round up a request size to the next valid allocation size.
*/
अटल पूर्णांक winMemRoundup(पूर्णांक n){
  वापस n;
}

/*
** Initialize this module.
*/
अटल पूर्णांक winMemInit(व्योम *p_App_तथ्य){
  win_Mem_तथ्य *p_Win_Mem_तथ्य = (win_Mem_तथ्य *)p_App_तथ्य;

  अगर( !p_Win_Mem_तथ्य ) वापस SQLITE_ERROR;
  निश्चित( p_Win_Mem_तथ्य->magic1==WINMEM_MAGIC1 );
  निश्चित( p_Win_Mem_तथ्य->magic2==WINMEM_MAGIC2 );

#अगर SQLITE_WIN32_HEAP_CREATE
  अगर( !p_Win_Mem_तथ्य->hHeap ){
    DWORD dw_Initial_आकार = SQLITE_WIN32_HEAP_INIT_आकार;
    DWORD dw_Maximum_आकार = (DWORD)sqlite3GlobalConfig.nHeap;
    अगर( dw_Maximum_आकार==0 ){
      dw_Maximum_आकार = SQLITE_WIN32_HEAP_MAX_आकार;
    }अन्यथा अगर( dw_Initial_आकार>dw_Maximum_आकार ){
      dw_Initial_आकार = dw_Maximum_आकार;
    }
    p_Win_Mem_तथ्य->hHeap = osHeapCreate(SQLITE_WIN32_HEAP_FLAGS,
                                      dw_Initial_आकार, dw_Maximum_आकार);
    अगर( !p_Win_Mem_तथ्य->hHeap ){
      sqlite3_log(SQLITE_NOMEM,
          "failed to HeapCreate (%lu), flags=%u, initSize=%lu, maxSize=%lu",
          osGetLastError(), SQLITE_WIN32_HEAP_FLAGS, dw_Initial_आकार,
          dw_Maximum_आकार);
      वापस SQLITE_NOMEM_BKPT;
    }
    p_Win_Mem_तथ्य->bOwned = TRUE;
    निश्चित( p_Win_Mem_तथ्य->bOwned );
  }
#अन्यथा
  p_Win_Mem_तथ्य->hHeap = osGetProcessHeap();
  अगर( !p_Win_Mem_तथ्य->hHeap ){
    sqlite3_log(SQLITE_NOMEM,
        "failed to GetProcessHeap (%lu)", osGetLastError());
    वापस SQLITE_NOMEM_BKPT;
  }
  p_Win_Mem_तथ्य->bOwned = FALSE;
  निश्चित( !p_Win_Mem_तथ्य->bOwned );
#पूर्ण_अगर
  निश्चित( p_Win_Mem_तथ्य->hHeap!=0 );
  निश्चित( p_Win_Mem_तथ्य->hHeap!=INVALID_HANDLE_मूल्य );
#अगर defined(SQLITE_WIN32_MALLOC_VALIDATE)
  निश्चित( osHeapValidate(p_Win_Mem_तथ्य->hHeap, SQLITE_WIN32_HEAP_FLAGS, शून्य) );
#पूर्ण_अगर
  वापस SQLITE_OK;
}

/*
** Deinitialize this module.
*/
अटल व्योम winMemShutdown(व्योम *p_App_तथ्य){
  win_Mem_तथ्य *p_Win_Mem_तथ्य = (win_Mem_तथ्य *)p_App_तथ्य;

  अगर( !p_Win_Mem_तथ्य ) वापस;
  निश्चित( p_Win_Mem_तथ्य->magic1==WINMEM_MAGIC1 );
  निश्चित( p_Win_Mem_तथ्य->magic2==WINMEM_MAGIC2 );

  अगर( p_Win_Mem_तथ्य->hHeap ){
    निश्चित( p_Win_Mem_तथ्य->hHeap!=INVALID_HANDLE_मूल्य );
#अगर defined(SQLITE_WIN32_MALLOC_VALIDATE)
    निश्चित( osHeapValidate(p_Win_Mem_तथ्य->hHeap, SQLITE_WIN32_HEAP_FLAGS, शून्य) );
#पूर्ण_अगर
    अगर( p_Win_Mem_तथ्य->bOwned ){
      अगर( !osHeapDestroy(p_Win_Mem_तथ्य->hHeap) ){
        sqlite3_log(SQLITE_NOMEM, "failed to HeapDestroy (%lu), heap=%p",
                    osGetLastError(), (व्योम*)p_Win_Mem_तथ्य->hHeap);
      }
      p_Win_Mem_तथ्य->bOwned = FALSE;
    }
    p_Win_Mem_तथ्य->hHeap = शून्य;
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
स्थिर sqlite3_mem_methods *sqlite3MemGetWin32(व्योम){
  अटल स्थिर sqlite3_mem_methods winMemMethods = {
    winMemMalloc,
    winMemFree,
    winMemRealloc,
    win_Mem_आकार,
    winMemRoundup,
    winMemInit,
    winMemShutdown,
    &win_mem_तथ्य
  };
  वापस &winMemMethods;
}

व्योम sqlite3MemSetDefault(व्योम){
  sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetWin32());
}
#पूर्ण_अगर /* SQLITE_WIN32_MALLOC */

#अगर_घोषित _WIN32
/*
** Convert a UTF-8 string to Microsoft Unicode.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
अटल LPWSTR winUtf8ToUnicode(स्थिर अक्षर *zText){
  पूर्णांक nChar;
  LPWSTR zWideText;

  nChar = osMultiByteToWideChar(CP_UTF8, 0, zText, -1, शून्य, 0);
  अगर( nChar==0 ){
    वापस 0;
  }
  zWideText = sqlite3MallocZero( nChar*माप(WCHAR) );
  अगर( zWideText==0 ){
    वापस 0;
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zText, -1, zWideText,
                                nChar);
  अगर( nChar==0 ){
    sqlite3_free(zWideText);
    zWideText = 0;
  }
  वापस zWideText;
}
#पूर्ण_अगर /* _WIN32 */

/*
** Convert a Microsoft Unicode string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
अटल अक्षर *winUnicodeToUtf8(LPCWSTR zWideText){
  पूर्णांक nByte;
  अक्षर *zText;

  nByte = osWideCharToMultiByte(CP_UTF8, 0, zWideText, -1, 0, 0, 0, 0);
  अगर( nByte == 0 ){
    वापस 0;
  }
  zText = sqlite3MallocZero( nByte );
  अगर( zText==0 ){
    वापस 0;
  }
  nByte = osWideCharToMultiByte(CP_UTF8, 0, zWideText, -1, zText, nByte,
                                0, 0);
  अगर( nByte == 0 ){
    sqlite3_free(zText);
    zText = 0;
  }
  वापस zText;
}

#अगर_घोषित _WIN32
/*
** Convert an ANSI string to Microsoft Unicode, using the ANSI or OEM
** code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
अटल LPWSTR winMbcsToUnicode(स्थिर अक्षर *zText, पूर्णांक useAnsi){
  पूर्णांक nWideChar;
  LPWSTR zMbcsText;
  पूर्णांक codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nWideChar = osMultiByteToWideChar(codepage, 0, zText, -1, शून्य,
                                0);
  अगर( nWideChar==0 ){
    वापस 0;
  }
  zMbcsText = sqlite3MallocZero( nWideChar*माप(WCHAR) );
  अगर( zMbcsText==0 ){
    वापस 0;
  }
  nWideChar = osMultiByteToWideChar(codepage, 0, zText, -1, zMbcsText,
                                nWideChar);
  अगर( nWideChar==0 ){
    sqlite3_free(zMbcsText);
    zMbcsText = 0;
  }
  वापस zMbcsText;
}

/*
** Convert a Microsoft Unicode string to a multi-byte character string,
** using the ANSI or OEM code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
अटल अक्षर *winUnicodeToMbcs(LPCWSTR zWideText, पूर्णांक useAnsi){
  पूर्णांक nByte;
  अक्षर *zText;
  पूर्णांक codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nByte = osWideCharToMultiByte(codepage, 0, zWideText, -1, 0, 0, 0, 0);
  अगर( nByte == 0 ){
    वापस 0;
  }
  zText = sqlite3MallocZero( nByte );
  अगर( zText==0 ){
    वापस 0;
  }
  nByte = osWideCharToMultiByte(codepage, 0, zWideText, -1, zText,
                                nByte, 0, 0);
  अगर( nByte == 0 ){
    sqlite3_free(zText);
    zText = 0;
  }
  वापस zText;
}

/*
** Convert a multi-byte character string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
अटल अक्षर *winMbcsToUtf8(स्थिर अक्षर *zText, पूर्णांक useAnsi){
  अक्षर *zTextUtf8;
  LPWSTR zTmpWide;

  zTmpWide = winMbcsToUnicode(zText, useAnsi);
  अगर( zTmpWide==0 ){
    वापस 0;
  }
  zTextUtf8 = winUnicodeToUtf8(zTmpWide);
  sqlite3_free(zTmpWide);
  वापस zTextUtf8;
}

/*
** Convert a UTF-8 string to a multi-byte character string.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
अटल अक्षर *winUtf8ToMbcs(स्थिर अक्षर *zText, पूर्णांक useAnsi){
  अक्षर *zTextMbcs;
  LPWSTR zTmpWide;

  zTmpWide = winUtf8ToUnicode(zText);
  अगर( zTmpWide==0 ){
    वापस 0;
  }
  zTextMbcs = winUnicodeToMbcs(zTmpWide, useAnsi);
  sqlite3_free(zTmpWide);
  वापस zTextMbcs;
}

/*
** This is a public wrapper for the winUtf8ToUnicode() function.
*/
LPWSTR sqlite3_win32_utf8_to_unicode(स्थिर अक्षर *zText){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !zText ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  अगर( sqlite3_initialize() ) वापस 0;
#पूर्ण_अगर
  वापस winUtf8ToUnicode(zText);
}

/*
** This is a public wrapper for the winUnicodeToUtf8() function.
*/
अक्षर *sqlite3_win32_unicode_to_utf8(LPCWSTR zWideText){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !zWideText ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  अगर( sqlite3_initialize() ) वापस 0;
#पूर्ण_अगर
  वापस winUnicodeToUtf8(zWideText);
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
अक्षर *sqlite3_win32_mbcs_to_utf8(स्थिर अक्षर *zText){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !zText ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  अगर( sqlite3_initialize() ) वापस 0;
#पूर्ण_अगर
  वापस winMbcsToUtf8(zText, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
अक्षर *sqlite3_win32_mbcs_to_utf8_v2(स्थिर अक्षर *zText, पूर्णांक useAnsi){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !zText ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  अगर( sqlite3_initialize() ) वापस 0;
#पूर्ण_अगर
  वापस winMbcsToUtf8(zText, useAnsi);
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
अक्षर *sqlite3_win32_utf8_to_mbcs(स्थिर अक्षर *zText){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !zText ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  अगर( sqlite3_initialize() ) वापस 0;
#पूर्ण_अगर
  वापस winUtf8ToMbcs(zText, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
अक्षर *sqlite3_win32_utf8_to_mbcs_v2(स्थिर अक्षर *zText, पूर्णांक useAnsi){
#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !zText ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  अगर( sqlite3_initialize() ) वापस 0;
#पूर्ण_अगर
  वापस winUtf8ToMbcs(zText, useAnsi);
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-8 string.
*/
पूर्णांक sqlite3_win32_set_directory8(
  अचिन्हित दीर्घ type, /* Identifier for directory being set or reset */
  स्थिर अक्षर *z_मूल्य  /* New value for directory being set or reset */
){
  अक्षर **ppDirectory = 0;
  पूर्णांक rc;
#अगर_अघोषित SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  अगर( rc ) वापस rc;
#पूर्ण_अगर
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  अगर( type==SQLITE_WIN32_तथ्य_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_तथ्य_directory;
  }अन्यथा अगर( type==SQLITE_WIN32_अस्थायी_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_अस्थायी_directory;
  }
  निश्चित( !ppDirectory || type==SQLITE_WIN32_तथ्य_DIRECTORY_TYPE
          || type==SQLITE_WIN32_अस्थायी_DIRECTORY_TYPE
  );
  निश्चित( !ppDirectory || sqlite3MemdebugHasType(*ppDirectory, MEMTYPE_HEAP) );
  अगर( ppDirectory ){
    अक्षर *zCopy = 0;
    अगर( z_मूल्य && z_मूल्य[0] ){
      zCopy = sqlite3_mprintf("%s", z_मूल्य);
      अगर ( zCopy==0 ){
        rc = SQLITE_NOMEM_BKPT;
        जाओ set_directory8_done;
      }
    }
    sqlite3_free(*ppDirectory);
    *ppDirectory = zCopy;
    rc = SQLITE_OK;
  }अन्यथा{
    rc = SQLITE_ERROR;
  }
set_directory8_done:
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  वापस rc;
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-16 string.
*/
पूर्णांक sqlite3_win32_set_directory16(
  अचिन्हित दीर्घ type, /* Identifier for directory being set or reset */
  स्थिर व्योम *z_मूल्य  /* New value for directory being set or reset */
){
  पूर्णांक rc;
  अक्षर *zUtf8 = 0;
  अगर( z_मूल्य ){
    zUtf8 = sqlite3_win32_unicode_to_utf8(z_मूल्य);
    अगर( zUtf8==0 ) वापस SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_win32_set_directory8(type, zUtf8);
  अगर( zUtf8 ) sqlite3_free(zUtf8);
  वापस rc;
}

/*
** This function sets the data directory or the temporary directory based on
** the provided arguments.  The type argument must be 1 in order to set the
** data directory or 2 in order to set the temporary directory.  The zValue
** argument is the name of the directory to use.  The return value will be
** SQLITE_OK if successful.
*/
पूर्णांक sqlite3_win32_set_directory(
  अचिन्हित दीर्घ type, /* Identifier for directory being set or reset */
  व्योम *z_मूल्य        /* New value for directory being set or reset */
){
  वापस sqlite3_win32_set_directory16(type, z_मूल्य);
}
#पूर्ण_अगर /* _WIN32 */

/*
** The return value of winGetLastErrorMsg
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated).
*/
अटल पूर्णांक win_Get_Last_Error_संदेश(DWORD lastErrno, पूर्णांक nBuf, अक्षर *zBuf){
  /* FormatMessage returns 0 on failure.  Otherwise it
  ** returns the number of TCHARs written to the output
  ** buffer, excluding the terminating null char.
  */
  DWORD dwLen = 0;
  अक्षर *zOut = 0;
  LPWSTR z_अस्थायी_Wide = शून्य;
  dwLen = osFormatMessageW(FORMAT_MESSAGE_ALLOCATE_बफर |
                           FORMAT_MESSAGE_FROM_SYSTEM |
                           FORMAT_MESSAGE_IGNORE_INSERTS,
                           शून्य,
                           lastErrno,
                           0,
                           (LPWSTR) &z_अस्थायी_Wide,
                           0,
                           0);
  अगर( dwLen > 0 ){
    /* allocate a buffer and convert to UTF8 */
    sqlite3BeginBenignMalloc();
    zOut = winUnicodeToUtf8(z_अस्थायी_Wide);
    sqlite3EndBenignMalloc();
    /* free the system buffer allocated by FormatMessage */
    osLocalFree(z_अस्थायी_Wide);
  }
  अगर( 0 == dwLen ){
    sqlite3_snprintf(nBuf, zBuf, "OsError 0x%lx (%lu)", lastErrno, lastErrno);
  }अन्यथा{
    /* copy a maximum of nBuf chars to output buffer */
    sqlite3_snprintf(nBuf, zBuf, "%s", zOut);
    /* free the UTF8 buffer */
    sqlite3_free(zOut);
  }
  वापस 0;
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
#घोषणा winLogError(a,b,c,d)   win_Log_Error_At_पंक्ति(a,b,c,d,__LINE__)
अटल पूर्णांक win_Log_Error_At_पंक्ति(
  पूर्णांक errcode,                    /* SQLite error code */
  DWORD lastErrno,                /* Win32 last error */
  स्थिर अक्षर *zFunc,              /* Name of OS function that failed */
  स्थिर अक्षर *zPath,              /* File path associated with error */
  पूर्णांक i_पंक्ति                       /* Source line number where error occurred */
){
  अक्षर z_संदेश[500];                 /* Human readable error text */
  पूर्णांक i;                          /* Loop counter */

  z_संदेश[0] = 0;
  win_Get_Last_Error_संदेश(lastErrno, माप(z_संदेश), z_संदेश);
  निश्चित( errcode!=SQLITE_OK );
  अगर( zPath==0 ) zPath = "";
  क्रम(i=0; z_संदेश[i] && z_संदेश[i]!='\r' && z_संदेश[i]!='\n'; i++){}
  z_संदेश[i] = 0;
  sqlite3_log(errcode,
      "os_win.c:%d: (%lu) %s(%s) - %s",
      i_पंक्ति, lastErrno, zFunc, zPath, z_संदेश
  );

  वापस errcode;
}

/*
** The number of times that a ReadFile(), WriteFile(), and DeleteFile()
** will be retried following a locking error - probably caused by
** antivirus software.  Also the initial delay before the first retry.
** The delay increases linearly with each retry.
*/
#अगर_अघोषित SQLITE_WIN32_IOERR_RETRY
# define SQLITE_WIN32_IOERR_RETRY 10
#पूर्ण_अगर
#अगर_अघोषित SQLITE_WIN32_IOERR_RETRY_DELAY
# define SQLITE_WIN32_IOERR_RETRY_DELAY 25
#पूर्ण_अगर
अटल पूर्णांक winIoerrRetry = SQLITE_WIN32_IOERR_RETRY;
अटल पूर्णांक winIoerrRetryDelay = SQLITE_WIN32_IOERR_RETRY_DELAY;

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
#अगर !defined(winIoerrCanRetry1)
#घोषणा winIoerrCanRetry1(a) (((a)==ERROR_ACCESS_DENIED)        || \
                              ((a)==ERROR_SHARING_VIOLATION)    || \
                              ((a)==ERROR_LOCK_VIOLATION)       || \
                              ((a)==ERROR_DEV_NOT_EXIST)        || \
                              ((a)==ERROR_NETNAME_DELETED)      || \
                              ((a)==ERROR_SEM_TIMEOUT)          || \
                              ((a)==ERROR_NETWORK_UNREACHABLE))
#पूर्ण_अगर

/*
** If a ReadFile() or WriteFile() error occurs, invoke this routine
** to see if it should be retried.  Return TRUE to retry.  Return FALSE
** to give up with an error.
*/
अटल पूर्णांक winRetryIoerr(पूर्णांक *pnRetry, DWORD *pError){
  DWORD e = osGetLastError();
  अगर( *pnRetry>=winIoerrRetry ){
    अगर( pError ){
      *pError = e;
    }
    वापस 0;
  }
  अगर( winIoerrCanRetry1(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    वापस 1;
  }
#अगर defined(winIoerrCanRetry2)
  अन्यथा अगर( winIoerrCanRetry2(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    वापस 1;
  }
#पूर्ण_अगर
  अगर( pError ){
    *pError = e;
  }
  वापस 0;
}

/*
** Log a I/O error retry episode.
*/
अटल व्योम winLogIoerr(पूर्णांक nRetry, पूर्णांक lineno){
  अगर( nRetry ){
    sqlite3_log(SQLITE_NOTICE,
      "delayed %dms for lock/sharing conflict at line %d",
      winIoerrRetryDelay*nRetry*(nRetry+1)/2, lineno
    );
  }
}


/*
** Lock a file region.
*/
अटल BOOL winLockFile(
  LPHANDLE phFile,
  DWORD flags,
  DWORD offset_निचला,
  DWORD offset_ऊपरी,
  DWORD अंक_Bytes_निचला,
  DWORD अंक_Bytes_ऊपरी
){
  OVERLAPPED ovlp;
  स_रखो(&ovlp, 0, माप(OVERLAPPED));
  ovlp.Offset = offset_निचला;
  ovlp.Offset_ऊपरी = offset_ऊपरी;
  वापस osLockFileEx(*phFile, flags, 0, अंक_Bytes_निचला, अंक_Bytes_ऊपरी, &ovlp);
}

#अगर_अघोषित SQLITE_OMIT_WAL
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
अटल पूर्णांक winHandleLockTimeout(
  HANDLE hFile,
  DWORD offset,
  DWORD nByte,
  पूर्णांक bExcl,
  DWORD nMs
){
  DWORD flags = LOCKFILE_FAIL_IMMEDIATELY | (bExcl?LOCKFILE_EXCLUSIVE_LOCK:0);
  पूर्णांक rc = SQLITE_OK;
  BOOL ret;
  OVERLAPPED ovlp;
  स_रखो(&ovlp, 0, माप(OVERLAPPED));
  ovlp.Offset = offset;

#अगर defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  अगर( nMs!=0 ){
    flags &= ~LOCKFILE_FAIL_IMMEDIATELY;
  }
  ovlp.hEvent = osCreateEvent(शून्य, TRUE, FALSE, शून्य);
  अगर( ovlp.hEvent==शून्य ){
    वापस SQLITE_IOERR_LOCK;
  }
#पूर्ण_अगर
  ret = osLockFileEx(hFile, flags, 0, nByte, 0, &ovlp);
#अगर defined(SQLITE_ENABLE_SETLK_TIMEOUT)
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
  अगर( !ret && GetLastError()==ERROR_IO_PENDING ){
    DWORD nDelay = (nMs==0 ? INFINITE : nMs);
    DWORD res = osWaitForSingleObject(ovlp.hEvent, nDelay);
    अगर( res==WAIT_OBJECT_0 ){
      ret = TRUE;
    }अन्यथा अगर( res==WAIT_TIMEOUT ){
#अगर SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = SQLITE_BUSY_TIMEOUT;
#अन्यथा
      rc = SQLITE_BUSY;
#पूर्ण_अगर
    }अन्यथा{
      /* Some other error has occurred */
      rc = SQLITE_IOERR_LOCK;
    }

    /* If it is still pending, cancel the LockFileEx() call. */
    osCancelIo(hFile);
  }

  osCloseHandle(ovlp.hEvent);
#पूर्ण_अगर /* defined(SQLITE_ENABLE_SETLK_TIMEOUT) */

  अगर( rc==SQLITE_OK && !ret ){
    rc = SQLITE_BUSY;
  }
  वापस rc;
}
#पूर्ण_अगर /* #ifndef SQLITE_OMIT_WAL */

/*
** Unlock a file region.
 */
अटल BOOL winUnlockFile(
  LPHANDLE phFile,
  DWORD offset_निचला,
  DWORD offset_ऊपरी,
  DWORD अंक_Bytes_निचला,
  DWORD अंक_Bytes_ऊपरी
){
  OVERLAPPED ovlp;
  स_रखो(&ovlp, 0, माप(OVERLAPPED));
  ovlp.Offset = offset_निचला;
  ovlp.Offset_ऊपरी = offset_ऊपरी;
  वापस osUnlockFileEx(*phFile, 0, अंक_Bytes_निचला, अंक_Bytes_ऊपरी, &ovlp);
}

#अगर_अघोषित SQLITE_OMIT_WAL
/*
** Remove an nByte lock starting at offset iOff from HANDLE h.
*/
अटल पूर्णांक winHandleUnlock(HANDLE h, पूर्णांक iOff, पूर्णांक nByte){
  BOOL ret = winUnlockFile(&h, iOff, 0, nByte, 0);
  वापस (ret ? SQLITE_OK : SQLITE_IOERR_UNLOCK);
}
#पूर्ण_अगर

/*****************************************************************************
** The next group of routines implement the I/O methods specified
** by the sqlite3_io_methods object.
******************************************************************************/

/*
** Some Microsoft compilers lack this definition.
*/
#अगर_अघोषित INVALID_SET_FILE_POINTER
# define INVALID_SET_FILE_POINTER ((DWORD)-1)
#पूर्ण_अगर

/*
** Seek the file handle h to offset nByte of the file.
**
** If successful, return SQLITE_OK. Or, if an error occurs, return an SQLite
** error code.
*/
अटल पूर्णांक winHandleSeek(HANDLE h, sqlite3_int64 iOffset){
  पूर्णांक rc = SQLITE_OK;             /* Return value */
  LARGE_INTEGER x;                /* The offset */

  x.QuadPart = iOffset;
  अगर( osSetFilePointerEx(h, x, 0, FILE_BEGIN)==0 ){
    rc = SQLITE_IOERR_SEEK;
  }
  OSTRACE(("SEEK file=%p, offset=%lld rc=%s\n", h, iOffset,sqlite3ErrName(rc)));
  वापस rc;
}

/*
** Move the current position of the file handle passed as the first
** argument to offset iOffset within the file. If successful, return 0.
** Otherwise, set pFile->lastErrno and return non-zero.
*/
अटल पूर्णांक winSeekFile(winFile *pFile, sqlite3_int64 iOffset){
  पूर्णांक rc;

  rc = winHandleSeek(pFile->h, iOffset);
  अगर( rc!=SQLITE_OK ){
    pFile->lastErrno = osGetLastError();
    winLogError(rc, pFile->lastErrno, "winSeekFile", pFile->zPath);
  }
  वापस rc;
}


#अगर SQLITE_MAX_MMAP_आकार>0
/* Forward references to VFS helper methods used for memory mapped files */
अटल पूर्णांक winMapfile(winFile*, sqlite3_int64);
अटल पूर्णांक winUnmapfile(winFile*);
#पूर्ण_अगर

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
#घोषणा MX_CLOSE_ATTEMPT 3
अटल पूर्णांक winClose(sqlite3_file *id){
  पूर्णांक rc, cnt = 0;
  winFile *pFile = (winFile*)id;

  निश्चित( id!=0 );
#अगर_अघोषित SQLITE_OMIT_WAL
  निश्चित( pFile->pShm==0 );
#पूर्ण_अगर
  निश्चित( pFile->h!=शून्य && pFile->h!=INVALID_HANDLE_मूल्य );
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p\n",
           osGetCurrentProcessId(), pFile, pFile->h));

#अगर SQLITE_MAX_MMAP_आकार>0
  winUnmapfile(pFile);
#पूर्ण_अगर

  करो{
    rc = osCloseHandle(pFile->h);
    /* SimulateIOError( rc=0; cnt=MX_CLOSE_ATTEMPT; ); */
  }जबतक( rc==0 && ++cnt < MX_CLOSE_ATTEMPT && (osSleep(100), 1) );
  अगर( rc ){
    pFile->h = शून्य;
  }
  Open_गणक(-1);
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, rc ? "ok" : "failed"));
  वापस rc ? SQLITE_OK
            : winLogError(SQLITE_IOERR_CLOSE, osGetLastError(),
                          "winClose", pFile->zPath);
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
अटल पूर्णांक winRead(
  sqlite3_file *id,          /* File to read from */
  व्योम *pBuf,                /* Write content into this buffer */
  पूर्णांक amt,                   /* Number of bytes to read */
  sqlite3_int64 offset       /* Begin reading at this offset */
){
#अगर !defined(SQLITE_WIN32_NO_OVERLAPPED)
  OVERLAPPED overlapped;          /* The offset for ReadFile. */
#पूर्ण_अगर
  winFile *pFile = (winFile*)id;  /* file handle */
  DWORD nRead;                    /* Number of bytes actually read from file */
  पूर्णांक nRetry = 0;                 /* Number of retrys */

  निश्चित( id!=0 );
  निश्चित( amt>0 );
  निश्चित( offset>=0 );
  SimulateIOError(वापस SQLITE_IOERR_READ);
  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, offset, pFile->locktype));

#अगर SQLITE_MAX_MMAP_आकार>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  अगर( offset<pFile->mmap_आकार ){
    अगर( offset+amt <= pFile->mmap_आकार ){
      स_नकल(pBuf, &((u8 *)(pFile->pMapRegion))[offset], amt);
      OSTRACE(("READ-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      वापस SQLITE_OK;
    }अन्यथा{
      पूर्णांक nCopy = (पूर्णांक)(pFile->mmap_आकार - offset);
      स_नकल(pBuf, &((u8 *)(pFile->pMapRegion))[offset], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#पूर्ण_अगर

#अगर defined(SQLITE_WIN32_NO_OVERLAPPED)
  अगर( winSeekFile(pFile, offset) ){
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    वापस SQLITE_FULL;
  }
  जबतक( !osReadFile(pFile->h, pBuf, amt, &nRead, 0) ){
#अन्यथा
  स_रखो(&overlapped, 0, माप(OVERLAPPED));
  overlapped.Offset = (LONG)(offset & 0xffffffff);
  overlapped.Offset_ऊपरी = (LONG)((offset>>32) & 0x7fffffff);
  जबतक( !osReadFile(pFile->h, pBuf, amt, &nRead, &overlapped) &&
         osGetLastError()!=ERROR_HANDLE_EOF ){
#पूर्ण_अगर
    DWORD lastErrno;
    अगर( winRetryIoerr(&nRetry, &lastErrno) ) जारी;
    pFile->lastErrno = lastErrno;
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    वापस winLogError(SQLITE_IOERR_READ, pFile->lastErrno,
                       "winRead", pFile->zPath);
  }
  winLogIoerr(nRetry, __LINE__);
  अगर( nRead<(DWORD)amt ){
    /* Unread parts of the buffer must be zero-filled */
    स_रखो(&((अक्षर*)pBuf)[nRead], 0, amt-nRead);
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_SHORT_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    वापस SQLITE_IOERR_SHORT_READ;
  }

  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  वापस SQLITE_OK;
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
अटल पूर्णांक winWrite(
  sqlite3_file *id,               /* File to write into */
  स्थिर व्योम *pBuf,               /* The bytes to be written */
  पूर्णांक amt,                        /* Number of bytes to write */
  sqlite3_int64 offset            /* Offset into the file to begin writing at */
){
  पूर्णांक rc = 0;                     /* True if error has occurred, else false */
  winFile *pFile = (winFile*)id;  /* File handle */
  पूर्णांक nRetry = 0;                 /* Number of retries */

  निश्चित( amt>0 );
  निश्चित( pFile );
  SimulateIOError(वापस SQLITE_IOERR_WRITE);
  SimulateDiskfullError(वापस SQLITE_FULL);

  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, offset, pFile->locktype));

#अगर defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_आकार>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  अगर( offset<pFile->mmap_आकार ){
    अगर( offset+amt <= pFile->mmap_आकार ){
      स_नकल(&((u8 *)(pFile->pMapRegion))[offset], pBuf, amt);
      OSTRACE(("WRITE-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      वापस SQLITE_OK;
    }अन्यथा{
      पूर्णांक nCopy = (पूर्णांक)(pFile->mmap_आकार - offset);
      स_नकल(&((u8 *)(pFile->pMapRegion))[offset], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#पूर्ण_अगर

#अगर defined(SQLITE_WIN32_NO_OVERLAPPED)
  rc = winSeekFile(pFile, offset);
  अगर( rc==0 ){
#अन्यथा
  {
#पूर्ण_अगर
#अगर !defined(SQLITE_WIN32_NO_OVERLAPPED)
    OVERLAPPED overlapped;        /* The offset for WriteFile. */
#पूर्ण_अगर
    u8 *aRem = (u8 *)pBuf;        /* Data yet to be written */
    पूर्णांक nRem = amt;               /* Number of bytes yet to be written */
    DWORD nWrite;                 /* Bytes written by each WriteFile() call */
    DWORD lastErrno = NO_ERROR;   /* Value returned by GetLastError() */

#अगर !defined(SQLITE_WIN32_NO_OVERLAPPED)
    स_रखो(&overlapped, 0, माप(OVERLAPPED));
    overlapped.Offset = (LONG)(offset & 0xffffffff);
    overlapped.Offset_ऊपरी = (LONG)((offset>>32) & 0x7fffffff);
#पूर्ण_अगर

    जबतक( nRem>0 ){
#अगर defined(SQLITE_WIN32_NO_OVERLAPPED)
      अगर( !osWriteFile(pFile->h, aRem, nRem, &nWrite, 0) ){
#अन्यथा
      अगर( !osWriteFile(pFile->h, aRem, nRem, &nWrite, &overlapped) ){
#पूर्ण_अगर
        अगर( winRetryIoerr(&nRetry, &lastErrno) ) जारी;
        अवरोध;
      }
      निश्चित( nWrite==0 || nWrite<=(DWORD)nRem );
      अगर( nWrite==0 || nWrite>(DWORD)nRem ){
        lastErrno = osGetLastError();
        अवरोध;
      }
#अगर !defined(SQLITE_WIN32_NO_OVERLAPPED)
      offset += nWrite;
      overlapped.Offset = (LONG)(offset & 0xffffffff);
      overlapped.Offset_ऊपरी = (LONG)((offset>>32) & 0x7fffffff);
#पूर्ण_अगर
      aRem += nWrite;
      nRem -= nWrite;
    }
    अगर( nRem>0 ){
      pFile->lastErrno = lastErrno;
      rc = 1;
    }
  }

  अगर( rc ){
    अगर(   ( pFile->lastErrno==ERROR_HANDLE_DISK_FULL )
       || ( pFile->lastErrno==ERROR_DISK_FULL )){
      OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      वापस winLogError(SQLITE_FULL, pFile->lastErrno,
                         "winWrite1", pFile->zPath);
    }
    OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_WRITE\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    वापस winLogError(SQLITE_IOERR_WRITE, pFile->lastErrno,
                       "winWrite2", pFile->zPath);
  }अन्यथा{
    winLogIoerr(nRetry, __LINE__);
  }
  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  वापस SQLITE_OK;
}

#अगर_अघोषित SQLITE_OMIT_WAL
/*
** Truncate the file opened by handle h to nByte bytes in size.
*/
अटल पूर्णांक winHandleTruncate(HANDLE h, sqlite3_int64 nByte){ 
  पूर्णांक rc = SQLITE_OK;             /* Return code */
  rc = winHandleSeek(h, nByte);
  अगर( rc==SQLITE_OK ){
    अगर( 0==osSetEndOfFile(h) ){
      rc = SQLITE_IOERR_TRUNCATE;
    }
  }
  वापस rc;
}

/*
** Determine the size in bytes of the file opened by the handle passed as 
** the first argument.
*/
अटल पूर्णांक win_Handle_आकार(HANDLE h, sqlite3_int64 *pnByte){ 
  पूर्णांक rc = SQLITE_OK;
  LARGE_INTEGER x;
  निश्चित( pnByte );
  अगर( os_Get_File_आकार_Ex(h, &x)==0 ){
    rc = SQLITE_IOERR_FSTAT;
    *pnByte = 0;
  }अन्यथा{
    *pnByte = x.QuadPart;
  }
  वापस rc;
}

/*
** Close the handle passed as the only argument.
*/
अटल व्योम winHandleClose(HANDLE h){
  अगर( h!=INVALID_HANDLE_मूल्य ){
    osCloseHandle(h);
  }
}
#पूर्ण_अगर /* #ifndef SQLITE_OMIT_WAL */

/*
** Truncate an open file to a specified size
*/
अटल पूर्णांक winTruncate(sqlite3_file *id, sqlite3_int64 nByte){
  winFile *pFile = (winFile*)id;  /* File handle object */
  पूर्णांक rc = SQLITE_OK;             /* Return code for this function */
  DWORD lastErrno;
#अगर SQLITE_MAX_MMAP_आकार>0
  sqlite3_int64 old_Mmap_आकार;
  अगर( pFile->nFetchOut>0 ){
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
    वापस SQLITE_OK;
  }
#पूर्ण_अगर

  निश्चित( pFile );
  SimulateIOError(वापस SQLITE_IOERR_TRUNCATE);
  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, size=%lld, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, nByte, pFile->locktype));

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  अगर( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

#अगर SQLITE_MAX_MMAP_आकार>0
  अगर( pFile->pMapRegion ){
    old_Mmap_आकार = pFile->mmap_आकार;
  }अन्यथा{
    old_Mmap_आकार = 0;
  }
  winUnmapfile(pFile);
#पूर्ण_अगर

  /* SetEndOfFile() returns non-zero when successful, or zero when it fails. */
  अगर( winSeekFile(pFile, nByte) ){
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->lastErrno,
                     "winTruncate1", pFile->zPath);
  }अन्यथा अगर( 0==osSetEndOfFile(pFile->h) &&
            ((lastErrno = osGetLastError())!=ERROR_USER_MAPPED_FILE) ){
    pFile->lastErrno = lastErrno;
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->lastErrno,
                     "winTruncate2", pFile->zPath);
  }

#अगर SQLITE_MAX_MMAP_आकार>0
  अगर( rc==SQLITE_OK && old_Mmap_आकार>0 ){
    अगर( old_Mmap_आकार>nByte ){
      winMapfile(pFile, -1);
    }अन्यथा{
      winMapfile(pFile, old_Mmap_आकार);
    }
  }
#पूर्ण_अगर

  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, sqlite3ErrName(rc)));
  वापस rc;
}

#अगर_घोषित SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occurring at the right times.
*/
पूर्णांक sqlite3_sync_गिनती = 0;
पूर्णांक sqlite3_fullsync_गिनती = 0;
#पूर्ण_अगर

/*
** Make sure all writes to a particular file are committed to disk.
*/
अटल पूर्णांक winSync(sqlite3_file *id, पूर्णांक flags){
#अगर_अघोषित SQLITE_NO_SYNC
  /*
  ** Used only when SQLITE_NO_SYNC is not defined.
   */
  BOOL rc;
#पूर्ण_अगर
#अगर !defined(न_संशोधन) || !defined(SQLITE_NO_SYNC) || \
    defined(SQLITE_HAVE_OS_TRACE)
  /*
  ** Used when SQLITE_NO_SYNC is not defined and by the assert() and/or
  ** OSTRACE() macros.
   */
  winFile *pFile = (winFile*)id;
#अन्यथा
  UNUSED_PARAMETER(id);
#पूर्ण_अगर

  निश्चित( pFile );
  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  निश्चित((flags&0x0F)==SQLITE_SYNC_NORMAL
      || (flags&0x0F)==SQLITE_SYNC_FULL
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( वापस SQLITE_FULL );

  OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, flags=%x, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, flags,
           pFile->locktype));

#अगर_अघोषित SQLITE_TEST
  UNUSED_PARAMETER(flags);
#अन्यथा
  अगर( (flags&0x0F)==SQLITE_SYNC_FULL ){
    sqlite3_fullsync_गिनती++;
  }
  sqlite3_sync_गिनती++;
#पूर्ण_अगर

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op
  */
#अगर_घोषित SQLITE_NO_SYNC
  OSTRACE(("SYNC-NOP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  वापस SQLITE_OK;
#अन्यथा
#अगर SQLITE_MAX_MMAP_आकार>0
  अगर( pFile->pMapRegion ){
    अगर( osFlushViewOfFile(pFile->pMapRegion, 0) ){
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_OK\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
    }अन्यथा{
      pFile->lastErrno = osGetLastError();
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
      वापस winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winSync1", pFile->zPath);
    }
  }
#पूर्ण_अगर
  rc = osFlushFileBuffers(pFile->h);
  SimulateIOError( rc=FALSE );
  अगर( rc ){
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    वापस SQLITE_OK;
  }अन्यथा{
    pFile->lastErrno = osGetLastError();
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_FSYNC\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    वापस winLogError(SQLITE_IOERR_FSYNC, pFile->lastErrno,
                       "winSync2", pFile->zPath);
  }
#पूर्ण_अगर
}

/*
** Determine the current size of a file in bytes
*/
अटल पूर्णांक win_File_आकार(sqlite3_file *id, sqlite3_int64 *p_आकार){
  winFile *pFile = (winFile*)id;
  पूर्णांक rc = SQLITE_OK;

  निश्चित( id!=0 );
  निश्चित( p_आकार!=0 );
  SimulateIOError(वापस SQLITE_IOERR_FSTAT);
  OSTRACE(("SIZE file=%p, pSize=%p\n", pFile->h, p_आकार));
  {
    LARGE_INTEGER x;
    अगर( os_Get_File_आकार_Ex(pFile->h, &x)==0 ){
      *p_आकार = 0;
      pFile->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_FSTAT, pFile->lastErrno,
                       "winFileSize", pFile->zPath);
    }अन्यथा{
      *p_आकार = x.QuadPart;
    }
  }
  OSTRACE(("SIZE file=%p, pSize=%p, *pSize=%lld, rc=%s\n",
           pFile->h, p_आकार, *p_आकार, sqlite3ErrName(rc)));
  वापस rc;
}

/*
** LOCKFILE_FAIL_IMMEDIATELY is undefined on some Windows systems.
*/
#अगर_अघोषित LOCKFILE_FAIL_IMMEDIATELY
# define LOCKFILE_FAIL_IMMEDIATELY 1
#पूर्ण_अगर

#अगर_अघोषित LOCKFILE_EXCLUSIVE_LOCK
# define LOCKFILE_EXCLUSIVE_LOCK 2
#पूर्ण_अगर

/*
** Historically, SQLite has used both the LockFile and LockFileEx functions.
** When the LockFile function was used, it was always expected to fail
** immediately if the lock could not be obtained.  Also, it always expected to
** obtain an exclusive lock.  These flags are used with the LockFileEx function
** and reflect those expectations; therefore, they should not be changed.
*/
#अगर_अघोषित SQLITE_LOCKFILE_FLAGS
# define SQLITE_LOCKFILE_FLAGS   (LOCKFILE_FAIL_IMMEDIATELY | \
                                  LOCKFILE_EXCLUSIVE_LOCK)
#पूर्ण_अगर

/*
** Currently, SQLite never calls the LockFileEx function without wanting the
** call to fail immediately if the lock cannot be obtained.
*/
#अगर_अघोषित SQLITE_LOCKFILEEX_FLAGS
# define SQLITE_LOCKFILEEX_FLAGS (LOCKFILE_FAIL_IMMEDIATELY)
#पूर्ण_अगर

/*
** Acquire a reader lock.
** Different API routines are called depending on whether or not this
** is Win9x or WinNT.
*/
अटल पूर्णांक winGetReadLock(winFile *pFile, पूर्णांक bBlock){
  पूर्णांक res;
  DWORD mask = ~(bBlock ? LOCKFILE_FAIL_IMMEDIATELY : 0);
  OSTRACE(("READ-LOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winLockFile(&pFile->h, SQLITE_LOCKFILEEX_FLAGS&mask, SHARED_FIRST, 0,
                    SHARED_आकार, 0);
  अगर( res == 0 ){
    pFile->lastErrno = osGetLastError();
    /* No need to log a failure to lock */
  }
  OSTRACE(("READ-LOCK file=%p, result=%d\n", pFile->h, res));
  वापस res;
}

/*
** Undo a readlock
*/
अटल पूर्णांक winUnlockReadLock(winFile *pFile){
  पूर्णांक res;
  DWORD lastErrno;
  OSTRACE(("READ-UNLOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winUnlockFile(&pFile->h, SHARED_FIRST, 0, SHARED_आकार, 0);
  अगर( res==0 && ((lastErrno = osGetLastError())!=ERROR_NOT_LOCKED) ){
    pFile->lastErrno = lastErrno;
    winLogError(SQLITE_IOERR_UNLOCK, pFile->lastErrno,
                "winUnlockReadLock", pFile->zPath);
  }
  OSTRACE(("READ-UNLOCK file=%p, result=%d\n", pFile->h, res));
  वापस res;
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
अटल पूर्णांक winLock(sqlite3_file *id, पूर्णांक locktype){
  पूर्णांक rc = SQLITE_OK;    /* Return code from subroutines */
  पूर्णांक res = 1;           /* Result of a Windows lock call */
  पूर्णांक newLocktype;       /* Set pFile->locktype to this value before exiting */
  पूर्णांक gotPendingLock = 0;/* True if we acquired a PENDING lock this time */
  winFile *pFile = (winFile*)id;
  DWORD lastErrno = NO_ERROR;

  निश्चित( id!=0 );
  OSTRACE(("LOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->sharedLockByte, locktype));

  /* If there is already a lock of this type or more restrictive on the
  ** OsFile, do nothing. Don't use the end_lock: exit path, as
  ** sqlite3OsEnterMutex() hasn't been called yet.
  */
  अगर( pFile->locktype>=locktype ){
    OSTRACE(("LOCK-HELD file=%p, rc=SQLITE_OK\n", pFile->h));
    वापस SQLITE_OK;
  }

  /* Do not allow any kind of write-lock on a read-only database
  */
  अगर( (pFile->ctrlFlags & WINFILE_RDONLY)!=0 && locktype>=RESERVED_LOCK ){
    वापस SQLITE_IOERR_LOCK;
  }

  /* Make sure the locking sequence is correct
  */
  निश्चित( pFile->locktype!=NO_LOCK || locktype==SHARED_LOCK );
  निश्चित( locktype!=PENDING_LOCK );
  निश्चित( locktype!=RESERVED_LOCK || pFile->locktype==SHARED_LOCK );

  /* Lock the PENDING_LOCK byte if we need to acquire an EXCLUSIVE lock or
  ** a SHARED lock.  If we are acquiring a SHARED lock, the acquisition of
  ** the PENDING_LOCK byte is temporary.
  */
  newLocktype = pFile->locktype;
  अगर( locktype==SHARED_LOCK
   || (locktype==EXCLUSIVE_LOCK && pFile->locktype==RESERVED_LOCK)
  ){
    पूर्णांक cnt = 3;

    /* Flags for the LockFileEx() call. This should be an exclusive lock if
    ** this call is to obtain EXCLUSIVE, or a shared lock if this call is to
    ** obtain SHARED.  */
    पूर्णांक flags = LOCKFILE_FAIL_IMMEDIATELY;
    अगर( locktype==EXCLUSIVE_LOCK ){
      flags |= LOCKFILE_EXCLUSIVE_LOCK;
    }
    जबतक( cnt>0 ){
      /* Try 3 times to get the pending lock.  This is needed to work
      ** around problems caused by indexing and/or anti-virus software on
      ** Windows systems.
      **
      ** If you are using this code as a model for alternative VFSes, do not
      ** copy this retry logic.  It is a hack intended for Windows only.  */
      res = winLockFile(&pFile->h, flags, PENDING_BYTE, 0, 1, 0);
      अगर( res ) अवरोध;

      lastErrno = osGetLastError();
      OSTRACE(("LOCK-PENDING-FAIL file=%p, count=%d, result=%d\n", 
            pFile->h, cnt, res
      ));

      अगर( lastErrno==ERROR_INVALID_HANDLE ){
        pFile->lastErrno = lastErrno;
        rc = SQLITE_IOERR_LOCK;
        OSTRACE(("LOCK-FAIL file=%p, count=%d, rc=%s\n", 
              pFile->h, cnt, sqlite3ErrName(rc)
        ));
        वापस rc;
      }

      cnt--;
      अगर( cnt>0 ) osSleep(1);
    }
    gotPendingLock = res;
  }

  /* Acquire a shared lock
  */
  अगर( locktype==SHARED_LOCK && res ){
    निश्चित( pFile->locktype==NO_LOCK );
#अगर_घोषित SQLITE_ENABLE_SETLK_TIMEOUT
    res = winGetReadLock(pFile, pFile->bBlockOnConnect);
#अन्यथा
    res = winGetReadLock(pFile, 0);
#पूर्ण_अगर
    अगर( res ){
      newLocktype = SHARED_LOCK;
    }अन्यथा{
      lastErrno = osGetLastError();
    }
  }

  /* Acquire a RESERVED lock
  */
  अगर( locktype==RESERVED_LOCK && res ){
    निश्चित( pFile->locktype==SHARED_LOCK );
    res = winLockFile(&pFile->h, SQLITE_LOCKFILE_FLAGS, RESERVED_BYTE, 0, 1, 0);
    अगर( res ){
      newLocktype = RESERVED_LOCK;
    }अन्यथा{
      lastErrno = osGetLastError();
    }
  }

  /* Acquire a PENDING lock
  */
  अगर( locktype==EXCLUSIVE_LOCK && res ){
    newLocktype = PENDING_LOCK;
    gotPendingLock = 0;
  }

  /* Acquire an EXCLUSIVE lock
  */
  अगर( locktype==EXCLUSIVE_LOCK && res ){
    निश्चित( pFile->locktype>=SHARED_LOCK );
    (व्योम)winUnlockReadLock(pFile);
    res = winLockFile(&pFile->h, SQLITE_LOCKFILE_FLAGS, SHARED_FIRST, 0,
                      SHARED_आकार, 0);
    अगर( res ){
      newLocktype = EXCLUSIVE_LOCK;
    }अन्यथा{
      lastErrno = osGetLastError();
      winGetReadLock(pFile, 0);
    }
  }

  /* If we are holding a PENDING lock that ought to be released, then
  ** release it now.
  */
  अगर( gotPendingLock && locktype==SHARED_LOCK ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }

  /* Update the state of the lock has held in the file descriptor then
  ** return the appropriate result code.
  */
  अगर( res ){
    rc = SQLITE_OK;
  }अन्यथा{
    pFile->lastErrno = lastErrno;
    rc = SQLITE_BUSY;
    OSTRACE(("LOCK-FAIL file=%p, wanted=%d, got=%d\n",
             pFile->h, locktype, newLocktype));
  }
  pFile->locktype = (u8)newLocktype;
  OSTRACE(("LOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  वापस rc;
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, return
** non-zero, otherwise zero.
*/
अटल पूर्णांक winCheckReservedLock(sqlite3_file *id, पूर्णांक *pResOut){
  पूर्णांक res;
  winFile *pFile = (winFile*)id;

  SimulateIOError( वापस SQLITE_IOERR_CHECKRESERVEDLOCK; );
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p\n", pFile->h, pResOut));

  निश्चित( id!=0 );
  अगर( pFile->locktype>=RESERVED_LOCK ){
    res = 1;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (local)\n", pFile->h, res));
  }अन्यथा{
    res = winLockFile(&pFile->h, SQLITE_LOCKFILEEX_FLAGS,RESERVED_BYTE,0,1,0);
    अगर( res ){
      winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
    }
    res = !res;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (remote)\n", pFile->h, res));
  }
  *pResOut = res;
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           pFile->h, pResOut, *pResOut));
  वापस SQLITE_OK;
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
अटल पूर्णांक winUnlock(sqlite3_file *id, पूर्णांक locktype){
  पूर्णांक type;
  winFile *pFile = (winFile*)id;
  पूर्णांक rc = SQLITE_OK;
  निश्चित( pFile!=0 );
  निश्चित( locktype<=SHARED_LOCK );
  OSTRACE(("UNLOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->sharedLockByte, locktype));
  type = pFile->locktype;
  अगर( type>=EXCLUSIVE_LOCK ){
    winUnlockFile(&pFile->h, SHARED_FIRST, 0, SHARED_आकार, 0);
    अगर( locktype==SHARED_LOCK && !winGetReadLock(pFile, 0) ){
      /* This should never happen.  We should always be able to
      ** reacquire the read lock */
      rc = winLogError(SQLITE_IOERR_UNLOCK, osGetLastError(),
                       "winUnlock", pFile->zPath);
    }
  }
  अगर( type>=RESERVED_LOCK ){
    winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
  }
  अगर( locktype==NO_LOCK && type>=SHARED_LOCK ){
    winUnlockReadLock(pFile);
  }
  अगर( type>=PENDING_LOCK ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }
  pFile->locktype = (u8)locktype;
  OSTRACE(("UNLOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  वापस rc;
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

अटल पूर्णांक winNolockLock(sqlite3_file *id, पूर्णांक locktype){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(locktype);
  वापस SQLITE_OK;
}

अटल पूर्णांक winNolockCheckReservedLock(sqlite3_file *id, पूर्णांक *pResOut){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(pResOut);
  वापस SQLITE_OK;
}

अटल पूर्णांक winNolockUnlock(sqlite3_file *id, पूर्णांक locktype){
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(locktype);
  वापस SQLITE_OK;
}

/******************* End of the no-op lock implementation *********************
******************************************************************************/

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
अटल व्योम winModeBit(winFile *pFile, अचिन्हित अक्षर mask, पूर्णांक *pArg){
  अगर( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & mask)!=0;
  }अन्यथा अगर( (*pArg)==0 ){
    pFile->ctrlFlags &= ~mask;
  }अन्यथा{
    pFile->ctrlFlags |= mask;
  }
}

/* Forward references to VFS helper methods used for temporary files */
अटल पूर्णांक winGetTempname(sqlite3_vfs *, अक्षर **);
अटल पूर्णांक winIsDir(LPCWSTR);
अटल BOOL winIsLongPathPrefix(स्थिर अक्षर *);
अटल BOOL winIsDriveLetterAndColon(स्थिर अक्षर *);

/*
** Control and query of the open file handle.
*/
अटल पूर्णांक winFileControl(sqlite3_file *id, पूर्णांक op, व्योम *pArg){
  winFile *pFile = (winFile*)id;
  OSTRACE(("FCNTL file=%p, op=%d, pArg=%p\n", pFile->h, op, pArg));
  चयन( op ){
    हाल SQLITE_FCNTL_LOCKSTATE: {
      *(पूर्णांक*)pArg = pFile->locktype;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      वापस SQLITE_OK;
    }
    हाल SQLITE_FCNTL_LAST_ERRNO: {
      *(पूर्णांक*)pArg = (पूर्णांक)pFile->lastErrno;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      वापस SQLITE_OK;
    }
    हाल SQLITE_FCNTL_CHUNK_आकार: {
      pFile->szChunk = *(पूर्णांक *)pArg;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      वापस SQLITE_OK;
    }
    हाल SQLITE_FCNTL_आकार_HINT: {
      अगर( pFile->szChunk>0 ){
        sqlite3_int64 oldSz;
        पूर्णांक rc = win_File_आकार(id, &oldSz);
        अगर( rc==SQLITE_OK ){
          sqlite3_int64 newSz = *(sqlite3_int64*)pArg;
          अगर( newSz>oldSz ){
            SimulateIOErrorBenign(1);
            rc = winTruncate(id, newSz);
            SimulateIOErrorBenign(0);
          }
        }
        OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
        वापस rc;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      वापस SQLITE_OK;
    }
    हाल SQLITE_FCNTL_PERSIST_WAL: {
      winModeBit(pFile, WINFILE_PERSIST_WAL, (पूर्णांक*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      वापस SQLITE_OK;
    }
    हाल SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      winModeBit(pFile, WINFILE_PSOW, (पूर्णांक*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      वापस SQLITE_OK;
    }
    हाल SQLITE_FCNTL_VFSNAME: {
      *(अक्षर**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      वापस SQLITE_OK;
    }
    हाल SQLITE_FCNTL_WIN32_AV_RETRY: {
      पूर्णांक *a = (पूर्णांक*)pArg;
      अगर( a[0]>0 ){
        winIoerrRetry = a[0];
      }अन्यथा{
        a[0] = winIoerrRetry;
      }
      अगर( a[1]>0 ){
        winIoerrRetryDelay = a[1];
      }अन्यथा{
        a[1] = winIoerrRetryDelay;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      वापस SQLITE_OK;
    }
    हाल SQLITE_FCNTL_WIN32_GET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      *phFile = pFile->h;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      वापस SQLITE_OK;
    }
#अगर_घोषित SQLITE_TEST
    हाल SQLITE_FCNTL_WIN32_SET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      HANDLE hOldFile = pFile->h;
      pFile->h = *phFile;
      *phFile = hOldFile;
      OSTRACE(("FCNTL oldFile=%p, newFile=%p, rc=SQLITE_OK\n",
               hOldFile, pFile->h));
      वापस SQLITE_OK;
    }
#पूर्ण_अगर
    हाल SQLITE_FCNTL_NULL_IO: {
      (व्योम)osCloseHandle(pFile->h);
      pFile->h = शून्य;
      वापस SQLITE_OK;
    }
    हाल SQLITE_FCNTL_TEMPFILENAME: {
      अक्षर *zTFile = 0;
      पूर्णांक rc = winGetTempname(pFile->pVfs, &zTFile);
      अगर( rc==SQLITE_OK ){
        *(अक्षर**)pArg = zTFile;
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      वापस rc;
    }
#अगर SQLITE_MAX_MMAP_आकार>0
    हाल SQLITE_FCNTL_MMAP_आकार: {
      i64 newLimit = *(i64*)pArg;
      पूर्णांक rc = SQLITE_OK;
      अगर( newLimit>sqlite3GlobalConfig.mxMmap ){
        newLimit = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (SIZE_T) and passed
      ** to MapViewOfFile(). Restrict its value to 2GB if (SIZE_T) is not at
      ** least a 64-bit type. */
      अगर( newLimit>0 && माप(आकार_T)<8 ){
        newLimit = (newLimit & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmap_आकार_Max;
      अगर( newLimit>=0 && newLimit!=pFile->mmap_आकार_Max && pFile->nFetchOut==0 ){
        pFile->mmap_आकार_Max = newLimit;
        अगर( pFile->mmap_आकार>0 ){
          winUnmapfile(pFile);
          rc = winMapfile(pFile, -1);
        }
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      वापस rc;
    }
#पूर्ण_अगर

#अगर_घोषित SQLITE_ENABLE_SETLK_TIMEOUT
    हाल SQLITE_FCNTL_LOCK_TIMEOUT: {
      पूर्णांक iOld = pFile->iBusyTimeout;
      पूर्णांक iNew = *(पूर्णांक*)pArg;
#अगर SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = (iNew < 0) ? INFINITE : (DWORD)iNew;
#या_अगर SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = (DWORD)(!!iNew);
#अन्यथा
# error "SQLITE_ENABLE_SETLK_TIMEOUT must be set to 1 or 2"
#पूर्ण_अगर
      *(पूर्णांक*)pArg = iOld;
      वापस SQLITE_OK;
    }
    हाल SQLITE_FCNTL_BLOCK_ON_CONNECT: {
      पूर्णांक iNew = *(पूर्णांक*)pArg;
      pFile->bBlockOnConnect = iNew;
      वापस SQLITE_OK;
    }
#पूर्ण_अगर /* SQLITE_ENABLE_SETLK_TIMEOUT */

#अगर defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    हाल SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      sqlite3_str_appendf(pStr, "{\"h\":%llu", (sqlite3_uint64)pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      अगर( pFile->locktype ){
        अटल स्थिर अक्षर *azLock[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
        sqlite3_str_appendf(pStr, ",\"locktype\":\"%s\"", 
                                  azLock[pFile->locktype-1]);
      }
#अगर SQLITE_MAX_MMAP_आकार>0
      अगर( pFile->mmap_आकार ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmap_आकार);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->nFetchOut);
      }
#पूर्ण_अगर
      sqlite3_str_append(pStr, "}", 1);
      वापस SQLITE_OK;
    }
#पूर्ण_अगर /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

  }
  OSTRACE(("FCNTL file=%p, rc=SQLITE_NOTFOUND\n", pFile->h));
  वापस SQLITE_NOTFOUND;
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
अटल पूर्णांक win_Sector_आकार(sqlite3_file *id){
  (व्योम)id;
  वापस SQLITE_DEFAULT_SECTOR_आकार;
}

/*
** Return a vector of device characteristics.
*/
अटल पूर्णांक winDeviceCharacteristics(sqlite3_file *id){
  winFile *p = (winFile*)id;
  वापस SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN | SQLITE_IOCAP_SUBPAGE_READ |
         ((p->ctrlFlags & WINFILE_PSOW)?SQLITE_IOCAP_POWERSAFE_OVERWRITE:0);
}

/*
** Windows will only let you create file view mappings
** on allocation size granularity boundaries.
** During sqlite3_os_init() we do a GetSystemInfo()
** to get the granularity size.
*/
अटल SYSTEM_INFO winSysInfo;

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
अटल LPWSTR winConvertFromUtf8Filename(स्थिर अक्षर *zFilename){
  LPWSTR zConverted = 0;
#अगर_घोषित __CYGWIN__
  पूर्णांक nChar;

  अगर( osCygwin_conv_path && !(winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2])) ){
    i64 nByte;
    पूर्णांक convertflag = CCP_POSIX_TO_WIN_W;
    अगर( !म_अक्षर(zFilename, '/') ) convertflag |= CCP_RELATIVE;
    nByte = (i64)osCygwin_conv_path(convertflag,
        zFilename, 0, 0);
    अगर( nByte>0 ){
      zConverted = sqlite3MallocZero(12+(u64)nByte);
      अगर ( zConverted==0 ){
        वापस zConverted;
      }
      /* Filenames should be prefixed, except when converted
       * full path already starts with "\\?\". */
      अगर( osCygwin_conv_path(convertflag, zFilename,
                           zConverted+4, nByte)==0 ){
        अगर( (convertflag&CCP_RELATIVE) ){
          स_हटाओ(zConverted, zConverted+4, nByte);
        }अन्यथा अगर( स_भेद(zConverted+4, L"\\\\", 4) ){
          स_नकल(zConverted, L"\\\\?\\", 8);
        }अन्यथा अगर( zConverted[6]!='?' ){
          स_हटाओ(zConverted+6, zConverted+4, nByte);
          स_नकल(zConverted, L"\\\\?\\UNC", 14);
        }अन्यथा{
          स_हटाओ(zConverted, zConverted+4, nByte);
        }
        वापस zConverted;
      }
      sqlite3_free(zConverted);
    }
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zFilename, -1, शून्य, 0);
  अगर( nChar==0 ){
    वापस 0;
  }
  zConverted = sqlite3MallocZero( ((u64)nChar+6)*माप(WCHAR) );
  अगर( zConverted==0 ){
    वापस 0;
  }
  nChar = osMultiByteToWideChar(CP_UTF8, 0, zFilename, -1,
                                zConverted, nChar);
  अगर( nChar==0 ){
    sqlite3_free(zConverted);
    zConverted = 0;
  }अन्यथा अगर( nChar>MAX_PATH
      && winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2]) ){
    स_हटाओ(zConverted+4, zConverted, nChar*माप(WCHAR));
    zConverted[2] = '\\';
    स_नकल(zConverted, L"\\\\?\\", 8);
  }अन्यथा अगर( nChar>MAX_PATH
      && winIsDirSep(zFilename[0]) && winIsDirSep(zFilename[1])
      && zFilename[2] != '?' ){
    स_हटाओ(zConverted+6, zConverted, nChar*माप(WCHAR));
    स_नकल(zConverted, L"\\\\?\\UNC", 14);
  }
#अन्यथा /* if !defined(__CYGWIN__) */
  zConverted = winUtf8ToUnicode(zFilename);
#पूर्ण_अगर /* __CYGWIN__ */
  /* caller will handle out of memory */
  वापस zConverted;
}

#अगर_अघोषित SQLITE_OMIT_WAL

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
अटल sqlite3_mutex *winBigLock = 0;
अटल व्योम winShmEnterMutex(व्योम){
  sqlite3_mutex_enter(winBigLock);
}
अटल व्योम winShmLeaveMutex(व्योम){
  sqlite3_mutex_leave(winBigLock);
}
#अगर_अघोषित न_संशोधन
अटल पूर्णांक winShmMutexHeld(व्योम) {
  वापस sqlite3_mutex_held(winBigLock);
}
#पूर्ण_अगर

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
काष्ठा win_Shm_गाँठ {
  sqlite3_mutex *mutex;      /* Mutex to access this object */
  अक्षर *zFilename;           /* Name of the file */
  HANDLE hSharedShm;         /* File handle open on zFilename */
  पूर्णांक bUseSharedLockHandle;  /* True to use hSharedShm for everything */

  पूर्णांक isUnlocked;            /* DMS lock has not yet been obtained */
  पूर्णांक isReadonly;            /* True if read-only */
  पूर्णांक szRegion;              /* Size of shared-memory regions */
  पूर्णांक nRegion;               /* Size of array apRegion */

  काष्ठा ShmRegion {
    HANDLE hMap;             /* File handle from CreateFileMapping */
    व्योम *pMap;
  } *aRegion;
  DWORD lastErrno;           /* The Windows errno from the last I/O error */

  winShm *pWinShmList;       /* List of winShm objects with ptrs to this */

  win_Shm_गाँठ *p_अग्र;         /* Next in list of all winShmNode objects */
#अगर defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 अग्र_Shm_Id;              /* Next available winShm.id value */
#पूर्ण_अगर
};

/*
** A global array of all winShmNode objects.
**
** The winShmMutexHeld() must be true while reading or writing this list.
*/
अटल win_Shm_गाँठ *win_Shm_गाँठ_List = 0;

/*
** Structure used internally by this VFS to record the state of an
** open shared memory connection. There is one such structure for each
** winFile open on a wal mode database.
*/
काष्ठा winShm {
  win_Shm_गाँठ *p_Shm_गाँठ;      /* The underlying winShmNode object */
  u16 sharedMask;            /* Mask of shared locks held */
  u16 exclMask;              /* Mask of exclusive locks held */
  HANDLE hShm;               /* File-handle on *-shm file. For locking. */
  पूर्णांक bReadonly;             /* True if hShm is opened read-only */
#अगर defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 id;                     /* Id of this connection with its winShmNode */
#पूर्ण_अगर
  winShm *p_Win_Shm_अग्र;       /* Next winShm object on same winShmNode */
};

/*
** Constants used for locking
*/
#घोषणा WIN_SHM_BASE   ((22+SQLITE_SHM_NLOCK)*4)        /* first lock byte */
#घोषणा WIN_SHM_DMS    (WIN_SHM_BASE+SQLITE_SHM_NLOCK)  /* deadman switch */

/* Forward references to VFS methods */
अटल पूर्णांक winOpen(sqlite3_vfs*,स्थिर अक्षर*,sqlite3_file*,पूर्णांक,पूर्णांक*);
अटल पूर्णांक winDelete(sqlite3_vfs *,स्थिर अक्षर*,पूर्णांक);

/*
** Purge the winShmNodeList list of all entries with winShmNode.pWinShmList==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
अटल व्योम winShmPurge(sqlite3_vfs *pVfs, पूर्णांक deleteFlag){
  win_Shm_गाँठ **pp;
  win_Shm_गाँठ *p;
  निश्चित( winShmMutexHeld() );
  OSTRACE(("SHM-PURGE pid=%lu, deleteFlag=%d\n",
           osGetCurrentProcessId(), deleteFlag));
  pp = &win_Shm_गाँठ_List;
  जबतक( (p = *pp)!=0 ){
    अगर( p->pWinShmList==0 ){
      पूर्णांक i;
      अगर( p->mutex ){ sqlite3_mutex_free(p->mutex); }
      क्रम(i=0; i<p->nRegion; i++){
        BOOL bRc = osUnmapViewOfFile(p->aRegion[i].pMap);
        OSTRACE(("SHM-PURGE-UNMAP pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        UNUSED_VARIABLE_मूल्य(bRc);
        bRc = osCloseHandle(p->aRegion[i].hMap);
        OSTRACE(("SHM-PURGE-CLOSE pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        UNUSED_VARIABLE_मूल्य(bRc);
      }
      winHandleClose(p->hSharedShm);
      अगर( deleteFlag ){
        SimulateIOErrorBenign(1);
        sqlite3BeginBenignMalloc();
        winDelete(pVfs, p->zFilename, 0);
        sqlite3EndBenignMalloc();
        SimulateIOErrorBenign(0);
      }
      *pp = p->p_अग्र;
      sqlite3_free(p->aRegion);
      sqlite3_free(p);
    }अन्यथा{
      pp = &p->p_अग्र;
    }
  }
}

/*
** Handle h is open on the *-shm file. This function zeroes the first
** 8 bytes of the file. If SQLITE_ENABLE_SETLK_TIMEOUT is defined, h may 
** have been opened with FILE_FLAG_OVERLAPPED.
*/
अटल पूर्णांक winZeroSharedMemory(HANDLE h){
  u8 aZero[8] = {0,0,0,0,0,0,0,0};
  DWORD nWrite = 0;
  OVERLAPPED ovlp;
  स_रखो(&ovlp, 0, माप(OVERLAPPED));
  अगर( !osWriteFile(h, aZero, माप(aZero), &nWrite, &ovlp) ){
    /* If the file was opened with FILE_FLAG_OVERLAPPED, the write may still
    ** be pending. Wait for it with GetOverlappedResult().  */
    अगर( osGetLastError()!=ERROR_IO_PENDING
     || !Get_Overlapped_परिणाम(h, &ovlp, &nWrite, TRUE)
    ){
      nWrite = 0;
    }
  }
  वापस (nWrite==माप(aZero) ? SQLITE_OK : SQLITE_IOERR_WRITE);
}

/*
** The DMS lock has not yet been taken on the shm file associated with
** pShmNode. Take the lock. Truncate the *-shm file if required.
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
अटल पूर्णांक winLockSharedMemory(win_Shm_गाँठ *p_Shm_गाँठ, DWORD nMs){
  HANDLE h = p_Shm_गाँठ->hSharedShm;
  पूर्णांक rc = SQLITE_OK;

  निश्चित( sqlite3_mutex_held(p_Shm_गाँठ->mutex) );
  rc = winHandleLockTimeout(h, WIN_SHM_DMS, 1, 1, 0);
  अगर( rc==SQLITE_OK ){
    /* We have an EXCLUSIVE lock on the DMS byte. This means that this
    ** is the first process to open the file. Truncate it to zero bytes
    ** in this case.  */
    अगर( p_Shm_गाँठ->isReadonly ){
      rc = SQLITE_READONLY_CANTINIT;
    }अन्यथा{
      अगर( SQLITE_OK!=winHandleTruncate(h, 0) ){
        /* Sometimes the truncate operation may fail because some other
        ** process is still holding an open mapping. So try to write
        ** zeroes into the start of the file instead. */
        rc = winZeroSharedMemory(h);
      }
    }

    /* Release the EXCLUSIVE lock acquired above. */
    winUnlockFile(&h, WIN_SHM_DMS, 0, 1, 0);
  }अन्यथा अगर( (rc & 0xFF)==SQLITE_BUSY ){
    rc = SQLITE_OK;
  }

  अगर( rc==SQLITE_OK ){
    /* Take a SHARED lock on the DMS byte. */
    rc = winHandleLockTimeout(h, WIN_SHM_DMS, 1, 0, nMs);
    अगर( rc==SQLITE_OK ){
      p_Shm_गाँठ->isUnlocked = 0;
    }
  }

  वापस rc;
}


/*
** This function is used to open a handle on a *-shm file.
**
** If SQLITE_ENABLE_SETLK_TIMEOUT is defined at build time, then the file
** is opened with FILE_FLAG_OVERLAPPED specified. If not, it is not.
*/
अटल पूर्णांक winHandleOpen(
  स्थिर अक्षर *zUtf8,              /* File to open */
  पूर्णांक *pbReadonly,                /* IN/OUT: True for readonly handle */
  HANDLE *ph                      /* OUT: New HANDLE for file */
){
  पूर्णांक rc = SQLITE_OK;
  LPWSTR zConverted = 0;
  पूर्णांक bReadonly = *pbReadonly;
  HANDLE h = INVALID_HANDLE_मूल्य;

#अगर_घोषित SQLITE_ENABLE_SETLK_TIMEOUT
  स्थिर DWORD flag_overlapped = FILE_FLAG_OVERLAPPED;
#अन्यथा
  स्थिर DWORD flag_overlapped = 0;
#पूर्ण_अगर

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8);
  अगर( zConverted==0 ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8));
    rc = SQLITE_IOERR_NOMEM_BKPT;
    जाओ winopenfile_out;
  }

  /* Ensure the file we are trying to open is not actually a directory. */
  अगर( winIsDir(zConverted) ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8));
    rc = SQLITE_CANTOPEN_ISDIR;
    जाओ winopenfile_out;
  }

  /* TODO: platforms.
  ** TODO: retry-on-ioerr.
  */
  h = osCreateFileW(zConverted,         /* lpFileName */
      (GENERIC_READ | (bReadonly ? 0 : GENERIC_WRITE)),  /* dwDesiredAccess */
      FILE_SHARE_READ | FILE_SHARE_WRITE,        /* dwShareMode */
      शून्य,                                      /* lpSecurityAttributes */
      OPEN_ALWAYS,                               /* dwCreationDisposition */
      FILE_ATTRIBUTE_NORMAL|flag_overlapped,
      शून्य
  );

  अगर( h==INVALID_HANDLE_मूल्य ){
    अगर( bReadonly==0 ){
      bReadonly = 1;
      rc = winHandleOpen(zUtf8, &bReadonly, &h);
    }अन्यथा{
      rc = SQLITE_CANTOPEN_BKPT;
    }
  }

 winopenfile_out:
  sqlite3_free(zConverted);
  *pbReadonly = bReadonly;
  *ph = h;
  वापस rc;
}
 
/*
** Close pDbFd's connection to shared-memory.  Delete the underlying
** *-shm file if deleteFlag is true.
*/
अटल पूर्णांक winCloseSharedMemory(winFile *pDbFd, पूर्णांक deleteFlag){
  winShm *p;            /* The connection to be closed */
  winShm **pp;          /* Iterator for pShmNode->pWinShmList */
  win_Shm_गाँठ *p_Shm_गाँठ; /* The underlying shared-memory file */

  p = pDbFd->pShm;
  अगर( p==0 ) वापस SQLITE_OK;
  अगर( p->hShm!=INVALID_HANDLE_मूल्य ){
    osCloseHandle(p->hShm);
  }

  winShmEnterMutex();
  p_Shm_गाँठ = p->p_Shm_गाँठ;

  /* Remove this connection from the winShmNode.pWinShmList list */
  sqlite3_mutex_enter(p_Shm_गाँठ->mutex);
  क्रम(pp=&p_Shm_गाँठ->pWinShmList; *pp!=p; pp=&(*pp)->p_Win_Shm_अग्र){}
  *pp = p->p_Win_Shm_अग्र;
  sqlite3_mutex_leave(p_Shm_गाँठ->mutex);

  winShmPurge(pDbFd->pVfs, deleteFlag);
  winShmLeaveMutex();

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  वापस SQLITE_OK;
}

/*
** testfixture builds may set this global variable to true via a
** Tcl interface. This forces the VFS to use the locking normally
** only used for UNC paths for all files.
*/
#अगर_घोषित SQLITE_TEST
पूर्णांक sqlite3_win_test_unc_locking = 0;
#अन्यथा
# define sqlite3_win_test_unc_locking 0
#पूर्ण_अगर

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
अटल पूर्णांक winIsUNCPath(स्थिर अक्षर *zFile){
  अगर( zFile[0]=='\\' && zFile[1]=='\\' ){
    अगर( zFile[2]=='?'
     && zFile[3]=='\\'
     && sqlite3Isalpha(zFile[4])
     && zFile[5]==':'
     && winIsDirSep(zFile[6])
    ){
      वापस sqlite3_win_test_unc_locking;
    }अन्यथा{
      वापस 1;
    }
  }
  वापस sqlite3_win_test_unc_locking;
}

/*
** Return true if the string passed in is the name of a file on a
** remote (network-mounted) volume.
*/
अटल पूर्णांक winIsRemoteVolume(स्थिर अक्षर *zFile){
  WCHAR z_मूल[4];
  अगर( म_भेदन(zFile, "\\\\?\\",4)==0 ) zFile += 4;
  अगर( !sqlite3Isalpha(zFile[0]) ) वापस 0;
  अगर( zFile[1]!=':' ) वापस 0;
  अगर( !winIsDirSep(zFile[2]) ) वापस 0;
  z_मूल[0] = (WCHAR)zFile[0];
  z_मूल[1] = ':';
  z_मूल[2] = '\\';
  z_मूल[3] = 0;
  वापस osGetDriveTypeW(z_मूल)==DRIVE_REMOTE;
}

/*
** Open the shared-memory area associated with database file pDbFd.
*/
अटल पूर्णांक winOpenSharedMemory(winFile *pDbFd){
  काष्ठा winShm *p;                  /* The connection to be opened */
  win_Shm_गाँठ *p_Shm_गाँठ = 0;          /* The underlying mmapped file */
  पूर्णांक rc = SQLITE_OK;                /* Result code */
  win_Shm_गाँठ *pNew;                  /* Newly allocated winShmNode */
  पूर्णांक nName;                         /* Size of zName in bytes */

  निश्चित( pDbFd->pShm==0 );    /* Not previously opened */

  /* Allocate space for the new sqlite3_shm object.  Also speculatively
  ** allocate space for a new winShmNode and filename.  */
  p = sqlite3MallocZero( माप(*p) );
  अगर( p==0 ) वापस SQLITE_IOERR_NOMEM_BKPT;
  nName = sqlite3Strlen30(pDbFd->zPath);
  pNew = sqlite3MallocZero( माप(*p_Shm_गाँठ) + (i64)nName + 17 );
  अगर( pNew==0 ){
    sqlite3_free(p);
    वापस SQLITE_IOERR_NOMEM_BKPT;
  }
  pNew->zFilename = (अक्षर*)&pNew[1];
  pNew->hSharedShm = INVALID_HANDLE_मूल्य;
  pNew->isUnlocked = 1;
  अगर( winIsUNCPath(pDbFd->zPath) || winIsRemoteVolume(pDbFd->zPath) ){
    pNew->bUseSharedLockHandle = 1;
  }अन्यथा{
    pNew->bUseSharedLockHandle = 0;
  }
  sqlite3_snprintf(nName+15, pNew->zFilename, "%s-shm", pDbFd->zPath);
  sqlite3FileSuffix3(pDbFd->zPath, pNew->zFilename);

  /* Look to see if there is an existing winShmNode that can be used.
  ** If no matching winShmNode currently exists, then create a new one.  */
  winShmEnterMutex();
  क्रम(p_Shm_गाँठ = win_Shm_गाँठ_List; p_Shm_गाँठ; p_Shm_गाँठ=p_Shm_गाँठ->p_अग्र){
    /* TBD need to come up with better match here.  Perhaps
    ** use FILE_ID_BOTH_DIR_INFO Structure.  */
    अगर( sqlite3StrICmp(p_Shm_गाँठ->zFilename, pNew->zFilename)==0 ) अवरोध;
  }
  अगर( p_Shm_गाँठ==0 ){
    p_Shm_गाँठ = pNew;

    /* Allocate a mutex for this winShmNode object, if one is required. */
    अगर( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_गाँठ->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      अगर( p_Shm_गाँठ->mutex==0 ) rc = SQLITE_IOERR_NOMEM_BKPT;
    }

    /* Open a file-handle to use for mappings, and for the DMS lock. */
    अगर( rc==SQLITE_OK ){
      HANDLE h = INVALID_HANDLE_मूल्य;
      p_Shm_गाँठ->isReadonly = sqlite3_uri_boolean(pDbFd->zPath,"readonly_shm",0);
      rc = winHandleOpen(pNew->zFilename, &p_Shm_गाँठ->isReadonly, &h);
      p_Shm_गाँठ->hSharedShm = h;
    }

    /* If successful, link the new winShmNode into the global list. If an 
    ** error occurred, free the object. */
    अगर( rc==SQLITE_OK ){
      p_Shm_गाँठ->p_अग्र = win_Shm_गाँठ_List;
      win_Shm_गाँठ_List = p_Shm_गाँठ;
      pNew = 0;
    }अन्यथा{
      sqlite3_mutex_free(p_Shm_गाँठ->mutex);
      अगर( p_Shm_गाँठ->hSharedShm!=INVALID_HANDLE_मूल्य ){
        osCloseHandle(p_Shm_गाँठ->hSharedShm);
      }
    }
  }

  /* If no error has occurred, link the winShm object to the winShmNode and
  ** the winShm to pDbFd.  */
  अगर( rc==SQLITE_OK ){
    sqlite3_mutex_enter(p_Shm_गाँठ->mutex);
    p->p_Shm_गाँठ = p_Shm_गाँठ;
    p->p_Win_Shm_अग्र = p_Shm_गाँठ->pWinShmList;
    p_Shm_गाँठ->pWinShmList = p;
#अगर defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
    p->id = p_Shm_गाँठ->अग्र_Shm_Id++;
#पूर्ण_अगर
    pDbFd->pShm = p;
    sqlite3_mutex_leave(p_Shm_गाँठ->mutex);
  }अन्यथा अगर( p ){
    sqlite3_free(p);
  }

  निश्चित( rc!=SQLITE_OK || p_Shm_गाँठ->isUnlocked==0 || p_Shm_गाँठ->nRegion==0 );
  winShmLeaveMutex();
  sqlite3_free(pNew);

  /* Open a file-handle on the *-shm file for this connection. This file-handle
  ** is only used for locking. The mapping of the *-shm file is created using
  ** the shared file handle in winShmNode.hSharedShm.  */
  अगर( rc==SQLITE_OK && p_Shm_गाँठ->bUseSharedLockHandle==0 ){
    p->bReadonly = sqlite3_uri_boolean(pDbFd->zPath, "readonly_shm", 0);
    rc = winHandleOpen(p_Shm_गाँठ->zFilename, &p->bReadonly, &p->hShm);
    अगर( rc!=SQLITE_OK ){
      निश्चित( p->hShm==INVALID_HANDLE_मूल्य );
      winCloseSharedMemory(pDbFd, 0);
    }
  }

  वापस rc;
}

/*
** Close a connection to shared-memory.  Delete the underlying
** storage if deleteFlag is true.
*/
अटल पूर्णांक winShmUnmap(
  sqlite3_file *fd,          /* Database holding shared memory */
  पूर्णांक deleteFlag             /* Delete after closing if true */
){
  वापस winCloseSharedMemory((winFile*)fd, deleteFlag);
}

/*
** Change the lock state for a shared-memory segment.
*/
अटल पूर्णांक winShmLock(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  पूर्णांक ofst,                  /* First lock to acquire or release */
  पूर्णांक n,                     /* Number of locks to acquire or release */
  पूर्णांक flags                  /* What to do with the lock */
){
  winFile *pDbFd = (winFile*)fd;        /* Connection holding shared memory */
  winShm *p = pDbFd->pShm;              /* The shared memory being locked */
  win_Shm_गाँठ *p_Shm_गाँठ;
  पूर्णांक rc = SQLITE_OK;                   /* Result code */
  u16 mask = (u16)((1U<<(ofst+n)) - (1U<<ofst)); /* Mask of locks to [un]take */

  अगर( p==0 ) वापस SQLITE_IOERR_SHMLOCK;
  p_Shm_गाँठ = p->p_Shm_गाँठ;
  अगर( NEVER(p_Shm_गाँठ==0) ) वापस SQLITE_IOERR_SHMLOCK;

  निश्चित( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  निश्चित( n>=1 );
  निश्चित( flags==(SQLITE_SHM_LOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_LOCK | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  निश्चित( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );

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
#अगर defined(SQLITE_ENABLE_SETLK_TIMEOUT) && defined(SQLITE_DEBUG)
  {
    u16 lockMask = (p->exclMask|p->sharedMask);
    निश्चित( (flags & SQLITE_SHM_UNLOCK) || pDbFd->iBusyTimeout==0 || (
          (ofst!=2 || lockMask==0)
       && (ofst!=1 || lockMask==0 || lockMask==2)
       && (ofst!=0 || lockMask<3)
       && (ofst<3  || lockMask<(1<<ofst))
    ));
  }
#पूर्ण_अगर

  /* Check if there is any work to do. There are three cases:
  **
  **    a) An unlock operation where there are locks to unlock,
  **    b) An shared lock where the requested lock is not already held
  **    c) An exclusive lock where the requested lock is not already held
  **
  ** The SQLite core never requests an exclusive lock that it already holds.
  ** This is assert()ed immediately below.  */
  निश्चित( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK) 
       || 0==(p->exclMask & mask)
  );
  अगर( ((flags & SQLITE_SHM_UNLOCK) && ((p->exclMask|p->sharedMask) & mask))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_LOCK) && 0==(p->sharedMask & mask))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK))
  ){
    HANDLE h = p->hShm;

    अगर( flags & SQLITE_SHM_UNLOCK ){
      /* Case (a) - unlock.  */

      निश्चित( (p->exclMask & p->sharedMask)==0 );
      निश्चित( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->exclMask & mask)==mask );
      निश्चित( !(flags & SQLITE_SHM_SHARED) || (p->sharedMask & mask)==mask );

      निश्चित( !(flags & SQLITE_SHM_SHARED) || n==1 );
      अगर( p_Shm_गाँठ->bUseSharedLockHandle ){
        h = p_Shm_गाँठ->hSharedShm;
        अगर( flags & SQLITE_SHM_SHARED ){
          winShm *pShm;
          sqlite3_mutex_enter(p_Shm_गाँठ->mutex);
          क्रम(pShm=p_Shm_गाँठ->pWinShmList; pShm; pShm=pShm->p_Win_Shm_अग्र){
            अगर( pShm!=p && (pShm->sharedMask & mask) ){
              /* Another connection within this process is also holding this
              ** SHARED lock. So do not actually release the OS lock.  */
              h = INVALID_HANDLE_मूल्य;
              अवरोध;
            }
          }
          sqlite3_mutex_leave(p_Shm_गाँठ->mutex);
        }
      }

      अगर( h!=INVALID_HANDLE_मूल्य ){
        rc = winHandleUnlock(h, ofst+WIN_SHM_BASE, n);
      }

      /* If successful, also clear the bits in sharedMask/exclMask */
      अगर( rc==SQLITE_OK ){
        p->exclMask = (p->exclMask & ~mask);
        p->sharedMask = (p->sharedMask & ~mask);
      }
    }अन्यथा{
      पूर्णांक bExcl = ((flags & SQLITE_SHM_EXCLUSIVE) ? 1 : 0);
      DWORD nMs = winFileBusyTimeout(pDbFd);

      अगर( p_Shm_गाँठ->bUseSharedLockHandle ){
        winShm *pShm;
        h = p_Shm_गाँठ->hSharedShm;
        sqlite3_mutex_enter(p_Shm_गाँठ->mutex);
        क्रम(pShm=p_Shm_गाँठ->pWinShmList; pShm; pShm=pShm->p_Win_Shm_अग्र){
          अगर( bExcl ){
            अगर( (pShm->sharedMask|pShm->exclMask) & mask ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_मूल्य;
            }
          }अन्यथा{
            अगर( pShm->sharedMask & mask ){
              h = INVALID_HANDLE_मूल्य;
            }अन्यथा अगर( pShm->exclMask & mask ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_मूल्य;
            }
          }
        }
        sqlite3_mutex_leave(p_Shm_गाँठ->mutex);
      }

      अगर( h!=INVALID_HANDLE_मूल्य ){
        rc = winHandleLockTimeout(h, ofst+WIN_SHM_BASE, n, bExcl, nMs);
      }
      अगर( rc==SQLITE_OK ){
        अगर( bExcl ){
          p->exclMask = (p->exclMask | mask);
        }अन्यथा{
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
  वापस rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
अटल व्योम winShmBarrier(
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
अटल पूर्णांक winShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  पूर्णांक iRegion,                    /* Region to retrieve */
  पूर्णांक szRegion,                   /* Size of regions */
  पूर्णांक isWrite,                    /* True to extend file if necessary */
  व्योम अस्थिर **pp              /* OUT: Mapped memory */
){
  winFile *pDbFd = (winFile*)fd;
  winShm *pShm = pDbFd->pShm;
  win_Shm_गाँठ *p_Shm_गाँठ;
  DWORD protect = PAGE_READWRITE;
  DWORD flags = FILE_MAP_WRITE | FILE_MAP_READ;
  पूर्णांक rc = SQLITE_OK;

  अगर( !pShm ){
    rc = winOpenSharedMemory(pDbFd);
    अगर( rc!=SQLITE_OK ) वापस rc;
    pShm = pDbFd->pShm;
    निश्चित( pShm!=0 );
  }
  p_Shm_गाँठ = pShm->p_Shm_गाँठ;

  sqlite3_mutex_enter(p_Shm_गाँठ->mutex);
  अगर( p_Shm_गाँठ->isUnlocked ){
    /* Take the DMS lock. */
    निश्चित( p_Shm_गाँठ->nRegion==0 );
    rc = winLockSharedMemory(p_Shm_गाँठ, winFileBusyTimeout(pDbFd));
    अगर( rc!=SQLITE_OK ) जाओ shmpage_out;
  }

  निश्चित( szRegion==p_Shm_गाँठ->szRegion || p_Shm_गाँठ->nRegion==0 );
  अगर( p_Shm_गाँठ->nRegion<=iRegion ){
    HANDLE hShared = p_Shm_गाँठ->hSharedShm;
    काष्ठा ShmRegion *apNew;           /* New aRegion[] array */
    i64 nByte = ((i64)iRegion+1)*(i64)szRegion;  /* Minimum file size */
    sqlite3_int64 sz;                  /* Current size of wal-index file */

    p_Shm_गाँठ->szRegion = szRegion;

    /* The requested region is not mapped into this processes address space.
    ** Check to see if it has been allocated (i.e. if the wal-index file is
    ** large enough to contain the requested region).
    */
    rc = win_Handle_आकार(hShared, &sz);
    अगर( rc!=SQLITE_OK ){
      rc = winLogError(rc, osGetLastError(), "winShmMap1", pDbFd->zPath);
      जाओ shmpage_out;
    }

    अगर( sz<nByte ){
      /* The requested memory region does not exist. If isWrite is set to
      ** zero, exit early. *pp will be set to NULL and SQLITE_OK returned.
      **
      ** Alternatively, if isWrite is non-zero, use ftruncate() to allocate
      ** the requested memory region.  */
      अगर( !isWrite ) जाओ shmpage_out;
      rc = winHandleTruncate(hShared, nByte);
      अगर( rc!=SQLITE_OK ){
        rc = winLogError(rc, osGetLastError(), "winShmMap2", pDbFd->zPath);
        जाओ shmpage_out;
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (काष्ठा ShmRegion*)sqlite3_realloc64(
        p_Shm_गाँठ->aRegion, ((i64)iRegion+1)*माप(apNew[0])
    );
    अगर( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      जाओ shmpage_out;
    }
    p_Shm_गाँठ->aRegion = apNew;

    अगर( p_Shm_गाँठ->isReadonly ){
      protect = PAGE_READONLY;
      flags = FILE_MAP_READ;
    }

    जबतक( p_Shm_गाँठ->nRegion<=iRegion ){
      HANDLE hMap = शून्य;         /* file-mapping handle */
      व्योम *pMap = 0;             /* Mapped memory region */

#अगर_घोषित SQLITE_UWP
      hMap = osCreateFileMappingFromApp(hShared, शून्य, protect, nByte, शून्य);
#अन्यथा
      hMap = osCreateFileMappingW(hShared, शून्य, protect, 0, nByte, शून्य);
#पूर्ण_अगर
      OSTRACE(("SHM-MAP-CREATE pid=%lu, region=%d, size=%lld, rc=%s\n",
               osGetCurrentProcessId(), p_Shm_गाँठ->nRegion, nByte,
               hMap ? "ok" : "failed"));
      अगर( hMap ){
        i64 iOffset = p_Shm_गाँठ->nRegion*szRegion;
        पूर्णांक iOffsetShift = iOffset % winSysInfo.dwAllocationGranularity;
#अगर_घोषित SQLITE_UWP
        pMap = osMapViewOfFileFromApp(hMap, flags,
            iOffset - iOffsetShift, (i64)szRegion + iOffsetShift
        );
#अन्यथा
        pMap = osMapViewOfFile(hMap, flags,
            0, iOffset - iOffsetShift, (i64)szRegion + iOffsetShift
        );
#पूर्ण_अगर
        OSTRACE(("SHM-MAP-MAP pid=%lu, region=%d, offset=%d, size=%d, rc=%s\n",
                 osGetCurrentProcessId(), p_Shm_गाँठ->nRegion, iOffset,
                 szRegion, pMap ? "ok" : "failed"));
      }
      अगर( !pMap ){
        p_Shm_गाँठ->lastErrno = osGetLastError();
        rc = winLogError(SQLITE_IOERR_SHMMAP, p_Shm_गाँठ->lastErrno,
                         "winShmMap3", pDbFd->zPath);
        अगर( hMap ) osCloseHandle(hMap);
        जाओ shmpage_out;
      }

      p_Shm_गाँठ->aRegion[p_Shm_गाँठ->nRegion].pMap = pMap;
      p_Shm_गाँठ->aRegion[p_Shm_गाँठ->nRegion].hMap = hMap;
      p_Shm_गाँठ->nRegion++;
    }
  }

shmpage_out:
  अगर( p_Shm_गाँठ->nRegion>iRegion ){
    i64 iOffset = (i64)iRegion*(i64)szRegion;
    पूर्णांक iOffsetShift = iOffset % winSysInfo.dwAllocationGranularity;
    अक्षर *p = (अक्षर *)p_Shm_गाँठ->aRegion[iRegion].pMap;
    *pp = (व्योम *)&p[iOffsetShift];
  }अन्यथा{
    *pp = 0;
  }
  अगर( p_Shm_गाँठ->isReadonly && rc==SQLITE_OK ){
    rc = SQLITE_READONLY;
  }
  sqlite3_mutex_leave(p_Shm_गाँठ->mutex);
  वापस rc;
}

#अन्यथा
# define winShmMap     0
# define winShmLock    0
# define winShmBarrier 0
# define winShmUnmap   0
#पूर्ण_अगर /* #ifndef SQLITE_OMIT_WAL */

/*
** Cleans up the mapped region of the specified file, if any.
*/
#अगर SQLITE_MAX_MMAP_आकार>0
अटल पूर्णांक winUnmapfile(winFile *pFile){
  निश्चित( pFile!=0 );
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, pMapRegion=%p, "
           "mmapSize=%lld, mmapSizeMax=%lld\n",
           osGetCurrentProcessId(), pFile, pFile->hMap, pFile->pMapRegion,
           pFile->mmap_आकार, pFile->mmap_आकार_Max));
  अगर( pFile->pMapRegion ){
    अगर( !osUnmapViewOfFile(pFile->pMapRegion) ){
      pFile->lastErrno = osGetLastError();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(), pFile,
               pFile->pMapRegion));
      वापस winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winUnmapfile1", pFile->zPath);
    }
    pFile->pMapRegion = 0;
    pFile->mmap_आकार = 0;
  }
  अगर( pFile->hMap!=शून्य ){
    अगर( !osCloseHandle(pFile->hMap) ){
      pFile->lastErrno = osGetLastError();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, rc=SQLITE_IOERR_MMAP\n",
               osGetCurrentProcessId(), pFile, pFile->hMap));
      वापस winLogError(SQLITE_IOERR_MMAP, pFile->lastErrno,
                         "winUnmapfile2", pFile->zPath);
    }
    pFile->hMap = शून्य;
  }
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile));
  वापस SQLITE_OK;
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
अटल पूर्णांक winMapfile(winFile *pFd, sqlite3_int64 nByte){
  sqlite3_int64 nMap = nByte;
  पूर्णांक rc;

  निश्चित( nMap>=0 || pFd->nFetchOut==0 );
  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, size=%lld\n",
           osGetCurrentProcessId(), pFd, nByte));

  अगर( pFd->nFetchOut>0 ) वापस SQLITE_OK;

  अगर( nMap<0 ){
    rc = win_File_आकार((sqlite3_file*)pFd, &nMap);
    अगर( rc ){
      OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_IOERR_FSTAT\n",
               osGetCurrentProcessId(), pFd));
      वापस SQLITE_IOERR_FSTAT;
    }
  }
  अगर( nMap>pFd->mmap_आकार_Max ){
    nMap = pFd->mmap_आकार_Max;
  }
  nMap &= ~(sqlite3_int64)(winSysInfo.dw_Page_आकार - 1);

  अगर( nMap==0 && pFd->mmap_आकार>0 ){
    winUnmapfile(pFd);
  }
  अगर( nMap!=pFd->mmap_आकार ){
    व्योम *pNew = 0;
    DWORD protect = PAGE_READONLY;
    DWORD flags = FILE_MAP_READ;

    winUnmapfile(pFd);
#अगर_घोषित SQLITE_MMAP_READWRITE
    अगर( (pFd->ctrlFlags & WINFILE_RDONLY)==0 ){
      protect = PAGE_READWRITE;
      flags |= FILE_MAP_WRITE;
    }
#पूर्ण_अगर
#अगर_घोषित SQLITE_UWP
    pFd->hMap = osCreateFileMappingFromApp(pFd->h, शून्य, protect, nMap, शून्य);
#अन्यथा
    pFd->hMap = osCreateFileMappingW(pFd->h, शून्य, protect,
                                (DWORD)((nMap>>32) & 0xffffffff),
                                (DWORD)(nMap & 0xffffffff), शून्य);
#पूर्ण_अगर
    अगर( pFd->hMap==शून्य ){
      pFd->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->lastErrno,
                       "winMapfile1", pFd->zPath);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-CREATE pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      वापस SQLITE_OK;
    }
    निश्चित( (nMap % winSysInfo.dw_Page_आकार)==0 );
    निश्चित( माप(आकार_T)==माप(sqlite3_int64) || nMap<=0xffffffff );
#अगर_घोषित SQLITE_UWP
    pNew = osMapViewOfFileFromApp(pFd->hMap, flags, 0, (आकार_T)nMap);
#अन्यथा
    pNew = osMapViewOfFile(pFd->hMap, flags, 0, 0, (आकार_T)nMap);
#पूर्ण_अगर
    अगर( pNew==शून्य ){
      osCloseHandle(pFd->hMap);
      pFd->hMap = शून्य;
      pFd->lastErrno = osGetLastError();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->lastErrno,
                       "winMapfile2", pFd->zPath);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-MAP pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      वापस SQLITE_OK;
    }
    pFd->pMapRegion = pNew;
    pFd->mmap_आकार = nMap;
  }

  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFd));
  वापस SQLITE_OK;
}
#पूर्ण_अगर /* SQLITE_MAX_MMAP_SIZE>0 */

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
अटल पूर्णांक winFetch(sqlite3_file *fd, i64 iOff, पूर्णांक nAmt, व्योम **pp){
#अगर SQLITE_MAX_MMAP_आकार>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */
#पूर्ण_अगर
  *pp = 0;

  OSTRACE(("FETCH pid=%lu, pFile=%p, offset=%lld, amount=%d, pp=%p\n",
           osGetCurrentProcessId(), fd, iOff, nAmt, pp));

#अगर SQLITE_MAX_MMAP_आकार>0
  अगर( pFd->mmap_आकार_Max>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    स्थिर पूर्णांक n_Eof_बफर = 256;
    अगर( pFd->pMapRegion==0 ){
      पूर्णांक rc = winMapfile(pFd, -1);
      अगर( rc!=SQLITE_OK ){
        OSTRACE(("FETCH pid=%lu, pFile=%p, rc=%s\n",
                 osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
        वापस rc;
      }
    }
    अगर( pFd->mmap_आकार >= (iOff+nAmt+n_Eof_बफर) ){
      निश्चित( pFd->pMapRegion!=0 );
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->nFetchOut++;
    }
  }
#पूर्ण_अगर

  OSTRACE(("FETCH pid=%lu, pFile=%p, pp=%p, *pp=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd, pp, *pp));
  वापस SQLITE_OK;
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
अटल पूर्णांक winUnfetch(sqlite3_file *fd, i64 iOff, व्योम *p){
#अगर SQLITE_MAX_MMAP_आकार>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  निश्चित( (p==0)==(pFd->nFetchOut==0) );

  /* If p!=0, it must match the iOff value. */
  निश्चित( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, offset=%lld, p=%p\n",
           osGetCurrentProcessId(), pFd, iOff, p));

  अगर( p ){
    pFd->nFetchOut--;
  }अन्यथा{
    /* FIXME:  If Windows truly always prevents truncating or deleting a
    ** file while a mapping is held, then the following winUnmapfile() call
    ** is unnecessary can be omitted - potentially improving
    ** performance.  */
    winUnmapfile(pFd);
  }

  निश्चित( pFd->nFetchOut>=0 );
#पूर्ण_अगर

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd));
  वापस SQLITE_OK;
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
अटल स्थिर sqlite3_io_methods winIoMethod = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_आकार,                    /* xFileSize */
  winLock,                        /* xLock */
  winUnlock,                      /* xUnlock */
  winCheckReservedLock,           /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_आकार,                  /* xSectorSize */
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
अटल स्थिर sqlite3_io_methods winIoNolockMethod = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_आकार,                    /* xFileSize */
  winNolockLock,                  /* xLock */
  winNolockUnlock,                /* xUnlock */
  winNolockCheckReservedLock,     /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_आकार,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  winShmLock,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

अटल win_Vfs_App_तथ्य win_App_तथ्य = {
  &winIoMethod,       /* pMethod */
  0,                  /* pAppData */
  0                   /* bNoLock */
};

अटल win_Vfs_App_तथ्य win_Nolock_App_तथ्य = {
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
अटल पूर्णांक winMakeEndInDirSep(पूर्णांक nBuf, अक्षर *zBuf){
  अगर( zBuf ){
    पूर्णांक nLen = sqlite3Strlen30(zBuf);
    अगर( nLen>0 ){
      अगर( winIsDirSep(zBuf[nLen-1]) ){
        वापस 1;
      }अन्यथा अगर( nLen+1<nBuf ){
        अगर( !osGetenv ){
          zBuf[nLen] = winGetDirSep();
        }अन्यथा अगर( winIsDriveLetterAndColon(zBuf) && winIsDirSep(zBuf[2]) ){
          zBuf[nLen] = '\\';
          zBuf[2]='\\';
        }अन्यथा{
          zBuf[nLen] = '/';
        }
        zBuf[nLen+1] = '\0';
        वापस 1;
      }
    }
  }
  वापस 0;
}

/*
** If sqlite3_temp_directory is defined, take the mutex and return true.
**
** If sqlite3_temp_directory is NULL (undefined), omit the mutex and
** return false.
*/
अटल पूर्णांक win_अस्थायी_Dir_Defined(व्योम){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  अगर( sqlite3_अस्थायी_directory!=0 ) वापस 1;
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  वापस 0;
}

/*
** Create a temporary file name and store the resulting pointer into pzBuf.
** The pointer returned in pzBuf must be freed via sqlite3_free().
*/
अटल पूर्णांक winGetTempname(sqlite3_vfs *pVfs, अक्षर **pzBuf){
  अटल स्थिर अक्षर zChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  माप_प्रकार i, j;
  DWORD pid;
  पूर्णांक nPre = sqlite3Strlen30(SQLITE_अस्थायी_FILE_PREFIX);
  i64 nMax, nBuf, nDir, nLen;
  अक्षर *zBuf;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  SimulateIOError( वापस SQLITE_IOERR );

  /* Allocate a temporary buffer to store the fully qualified file
  ** name for the temporary file.  If this fails, we cannot continue.
  */
  nMax = pVfs->mxPathname;
  nBuf = 2 + (i64)nMax;
  zBuf = sqlite3MallocZero( nBuf );
  अगर( !zBuf ){
    OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
    वापस SQLITE_IOERR_NOMEM_BKPT;
  }

  /* Figure out the effective temporary directory.  First, check if one
  ** has been explicitly set by the application; otherwise, use the one
  ** configured by the operating system.
  */
  nDir = nMax - (nPre + 15);
  निश्चित( nDir>0 );
  अगर( win_अस्थायी_Dir_Defined() ){
    पूर्णांक nDirLen = sqlite3Strlen30(sqlite3_अस्थायी_directory);
    अगर( nDirLen>0 ){
      अगर( !winIsDirSep(sqlite3_अस्थायी_directory[nDirLen-1]) ){
        nDirLen++;
      }
      अगर( nDirLen>nDir ){
        sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
        sqlite3_free(zBuf);
        OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
        वापस winLogError(SQLITE_ERROR, 0, "winGetTempname1", 0);
      }
      sqlite3_snprintf(nMax, zBuf, "%s", sqlite3_अस्थायी_directory);
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  }

#अगर defined(__CYGWIN__)
  अन्यथा अगर( osGetenv!=शून्य ){
    अटल स्थिर अक्षर *azDirs[] = {
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
    अचिन्हित पूर्णांक i;
    स्थिर अक्षर *zDir = 0;

    अगर( !azDirs[0] ) azDirs[0] = osGetenv("SQLITE_TMPDIR");
    अगर( !azDirs[1] ) azDirs[1] = osGetenv("TMPDIR");
    अगर( !azDirs[2] ) azDirs[2] = osGetenv("TMP");
    अगर( !azDirs[3] ) azDirs[3] = osGetenv("TEMP");
    अगर( !azDirs[4] ) azDirs[4] = osGetenv("USERPROFILE");
    क्रम(i=0; i<माप(azDirs)/माप(azDirs[0]); zDir=azDirs[i++]){
      LPWSTR zConverted;
      अगर( zDir==0 ) जारी;
      /* If the path starts with a drive letter followed by the colon
      ** character, assume it is already a native Win32 path; otherwise,
      ** it must be converted to a native Win32 path via the Cygwin API
      ** prior to using it.
      */
      {
        zConverted = winConvertFromUtf8Filename(zDir);
        अगर( !zConverted ){
          sqlite3_free(zBuf);
          OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
          वापस SQLITE_IOERR_NOMEM_BKPT;
        }
        अगर( winIsDir(zConverted) ){
          sqlite3_snprintf(nMax, zBuf, "%s", zDir);
          sqlite3_free(zConverted);
          अवरोध;
        }
        sqlite3_free(zConverted);
      }
    }
  }
#पूर्ण_अगर

  अन्यथा{
    अक्षर *zMulti;
    LPWSTR zWidePath = sqlite3MallocZero( nMax*माप(WCHAR) );
    अगर( !zWidePath ){
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      वापस SQLITE_IOERR_NOMEM_BKPT;
    }
    अगर( os_Get_अस्थायी_Path_W==0 || os_Get_अस्थायी_Path_W(nMax, zWidePath)==0 ){
      sqlite3_free(zWidePath);
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_GETTEMPPATH\n"));
      वापस winLogError(SQLITE_IOERR_GETTEMPPATH, osGetLastError(),
                         "winGetTempname2", 0);
    }
    zMulti = winUnicodeToUtf8(zWidePath);
    sqlite3_free(zWidePath);
    अगर( zMulti ){
      sqlite3_snprintf(nMax, zBuf, "%s", zMulti);
      sqlite3_free(zMulti);
    }अन्यथा{
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      वापस SQLITE_IOERR_NOMEM_BKPT;
    }
  }

  /*
  ** Check to make sure the temporary directory ends with an appropriate
  ** separator.  If it does not and there is not enough space left to add
  ** one, fail.
  */
  अगर( !winMakeEndInDirSep(nDir+1, zBuf) ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    वापस winLogError(SQLITE_ERROR, 0, "winGetTempname4", 0);
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
  अगर( (nLen + nPre + 17) > nBuf ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    वापस winLogError(SQLITE_ERROR, 0, "winGetTempname5", 0);
  }

  sqlite3_snprintf(nBuf-16-nLen, zBuf+nLen, SQLITE_अस्थायी_FILE_PREFIX);

  j = sqlite3Strlen30(zBuf);
  sqlite3_randomness(15, &zBuf[j]);
  pid = osGetCurrentProcessId();
  क्रम(i=0; i<15; i++, j++){
    zBuf[j] += pid & 0xff;
    pid >>= 8;
    zBuf[j] = (अक्षर)zChars[ ((अचिन्हित अक्षर)zBuf[j])%(माप(zChars)-1) ];
  }
  zBuf[j] = 0;
  zBuf[j+1] = 0;
  *pzBuf = zBuf;

  OSTRACE(("TEMP-FILENAME name=%s, rc=SQLITE_OK\n", zBuf));
  वापस SQLITE_OK;
}

/*
** Return TRUE if the named file is really a directory.  Return false if
** it is something other than a directory, or if there is any kind of memory
** allocation failure.
*/
अटल पूर्णांक winIsDir(LPCWSTR zConverted){
  DWORD attr;
  पूर्णांक rc = 0;
  DWORD lastErrno;
  पूर्णांक cnt = 0;
  WIN32_FILE_ATTRIBUTE_तथ्य s_Attr_तथ्य;
  स_रखो(&s_Attr_तथ्य, 0, माप(s_Attr_तथ्य));
  जबतक( !(rc = osGetFileAttributesExW(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_तथ्य)) && winRetryIoerr(&cnt, &lastErrno) ){}
  अगर( !rc ){
    वापस 0; /* Invalid name? */
  }
  attr = s_Attr_तथ्य.dwFileAttributes;
  वापस (attr!=INVALID_FILE_ATTRIBUTES) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

/* forward reference */
अटल पूर्णांक winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  स्थिर अक्षर *zFilename,     /* Name of file to check */
  पूर्णांक flags,                 /* Type of test to make on this file */
  पूर्णांक *pResOut               /* OUT: Result */
);

/*
** The Windows version of xAccess() accepts an extra bit in the flags
** parameter that prevents an anti-virus retry loop.
*/
#घोषणा NORETRY 0x4000

/*
** Open a file.
*/
अटल पूर्णांक winOpen(
  sqlite3_vfs *pVfs,        /* Used to get maximum path length and AppData */
  स्थिर अक्षर *zName,        /* Name of the file (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  पूर्णांक flags,                /* Open mode flags */
  पूर्णांक *pOutFlags            /* Status return flags */
){
  HANDLE h;
  DWORD lastErrno = 0;
  DWORD dwDesiredAccess;
  DWORD dwShareMode;
  DWORD dwCreationDisposition;
  DWORD dwFlagsAndAttributes = 0;
  win_Vfs_App_तथ्य *p_App_तथ्य;
  winFile *pFile = (winFile*)id;
  LPWSTR zConverted;              /* Filename in OS encoding */
  स्थिर अक्षर *zUtf8Name = zName; /* Filename in UTF-8 encoding */
  पूर्णांक cnt = 0;
  पूर्णांक isRO = 0;              /* file is known to be accessible readonly */

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  अक्षर *zTmpname = 0; /* For temporary filename, if necessary. */

  पूर्णांक rc = SQLITE_OK;            /* Function Return Code */
#अगर !defined(न_संशोधन)
  पूर्णांक eType = flags&0x0FFF00;  /* Type of file to open */
#पूर्ण_अगर

  पूर्णांक isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  पूर्णांक isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  पूर्णांक isCreate     = (flags & SQLITE_OPEN_CREATE);
  पूर्णांक isReadonly   = (flags & SQLITE_OPEN_READONLY);
  पूर्णांक isReadWrite  = (flags & SQLITE_OPEN_READWRITE);

#अगर_अघोषित न_संशोधन
  पूर्णांक isOpenJournal = (isCreate && (
        eType==SQLITE_OPEN_SUPER_JOURNAL
     || eType==SQLITE_OPEN_MAIN_JOURNAL
     || eType==SQLITE_OPEN_WAL
  ));
#पूर्ण_अगर

  OSTRACE(("OPEN name=%s, pFile=%p, flags=%x, pOutFlags=%p\n",
           zUtf8Name, id, flags, pOutFlags));

  /* Check the following statements are true:
  **
  **   (a) Exactly one of the READWRITE and READONLY flags must be set, and
  **   (b) if CREATE is set, then READWRITE must also be set, and
  **   (c) if EXCLUSIVE is set, then CREATE must also be set.
  **   (d) if DELETEONCLOSE is set, then CREATE must also be set.
  */
  निश्चित((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
  निश्चित(isCreate==0 || isReadWrite);
  निश्चित(isExclusive==0 || isCreate);
  निश्चित(isDelete==0 || isCreate);

  /* The main DB, main journal, WAL file and super-journal are never
  ** automatically deleted. Nor are they ever temporary files.  */
  निश्चित( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_DB );
  निश्चित( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_JOURNAL );
  निश्चित( (!isDelete && zName) || eType!=SQLITE_OPEN_SUPER_JOURNAL );
  निश्चित( (!isDelete && zName) || eType!=SQLITE_OPEN_WAL );

  /* Assert that the upper layer has set one of the "file-type" flags. */
  निश्चित( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_अस्थायी_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_अस्थायी_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );

  निश्चित( pFile!=0 );
  स_रखो(pFile, 0, माप(winFile));
  pFile->h = INVALID_HANDLE_मूल्य;

  /* If the second argument to this function is NULL, generate a
  ** temporary file name to use
  */
  अगर( !zUtf8Name ){
    निश्चित( isDelete && !isOpenJournal );
    rc = winGetTempname(pVfs, &zTmpname);
    अगर( rc!=SQLITE_OK ){
      OSTRACE(("OPEN name=%s, rc=%s", zUtf8Name, sqlite3ErrName(rc)));
      वापस rc;
    }
    zUtf8Name = zTmpname;
  }

  /* Database filenames are double-zero terminated if they are not
  ** URIs with parameters.  Hence, they can always be passed into
  ** sqlite3_uri_parameter().
  */
  निश्चित( (eType!=SQLITE_OPEN_MAIN_DB) || (flags & SQLITE_OPEN_URI) ||
       zUtf8Name[sqlite3Strlen30(zUtf8Name)+1]==0 );

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8Name);
  अगर( zConverted==0 ){
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8Name));
    वापस SQLITE_IOERR_NOMEM_BKPT;
  }

  अगर( winIsDir(zConverted) ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8Name));
    वापस SQLITE_CANTOPEN_ISDIR;
  }

  अगर( isReadWrite ){
    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
  }अन्यथा{
    dwDesiredAccess = GENERIC_READ;
  }

  /* SQLITE_OPEN_EXCLUSIVE is used to make sure that a new file is
  ** created. SQLite doesn't use it to indicate "exclusive access"
  ** as it is usually understood.
  */
  अगर( isExclusive ){
    /* Creates a new file, only if it does not already exist. */
    /* If the file exists, it fails. */
    dwCreationDisposition = CREATE_NEW;
  }अन्यथा अगर( isCreate ){
    /* Open existing file, or create if it doesn't exist */
    dwCreationDisposition = OPEN_ALWAYS;
  }अन्यथा{
    /* Opens a file, only if it exists. */
    dwCreationDisposition = OPEN_EXISTING;
  }

  अगर( 0==sqlite3_uri_boolean(zName, "exclusive", 0) ){
    dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
  }अन्यथा{
    dwShareMode = 0;
  }

  अगर( isDelete ){
    dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY
                               | FILE_ATTRIBUTE_HIDDEN
                               | FILE_FLAG_DELETE_ON_CLOSE;
  }अन्यथा{
    dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
  }
  करो{
    h = osCreateFileW(zConverted,
                      dwDesiredAccess,
                      dwShareMode, शून्य,
                      dwCreationDisposition,
                      dwFlagsAndAttributes,
                      शून्य);
    अगर( h!=INVALID_HANDLE_मूल्य ) अवरोध;
    अगर( isReadWrite ){
      पूर्णांक rc2;
      sqlite3BeginBenignMalloc();
      rc2 = winAccess(pVfs, zUtf8Name, SQLITE_ACCESS_READ|NORETRY, &isRO);
      sqlite3EndBenignMalloc();
      अगर( rc2==SQLITE_OK && isRO ) अवरोध;
    }
  }जबतक( winRetryIoerr(&cnt, &lastErrno) );
  winLogIoerr(cnt, __LINE__);

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, rc=%s\n", h, zUtf8Name,
           dwDesiredAccess, (h==INVALID_HANDLE_मूल्य) ? "failed" : "ok"));

  अगर( h==INVALID_HANDLE_मूल्य ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    अगर( isReadWrite && isRO && !isExclusive ){
      वापस winOpen(pVfs, zName, id,
         ((flags|SQLITE_OPEN_READONLY) &
                     ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE)),
         pOutFlags);
    }अन्यथा{
      pFile->lastErrno = lastErrno;
      winLogError(SQLITE_CANTOPEN, pFile->lastErrno, "winOpen", zUtf8Name);
      वापस SQLITE_CANTOPEN_BKPT;
    }
  }

  अगर( pOutFlags ){
    अगर( isReadWrite ){
      *pOutFlags = SQLITE_OPEN_READWRITE;
    }अन्यथा{
      *pOutFlags = SQLITE_OPEN_READONLY;
    }
  }

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, pOutFlags=%p, *pOutFlags=%d, "
           "rc=%s\n", h, zUtf8Name, dwDesiredAccess, pOutFlags, pOutFlags ?
           *pOutFlags : 0, (h==INVALID_HANDLE_मूल्य) ? "failed" : "ok"));

  p_App_तथ्य = (win_Vfs_App_तथ्य*)pVfs->p_App_तथ्य;

  sqlite3_free(zConverted);
  sqlite3_free(zTmpname);
  id->pMethods = p_App_तथ्य ? p_App_तथ्य->pMethod : &winIoMethod;
  pFile->pVfs = pVfs;
  pFile->h = h;
  अगर( isReadonly ){
    pFile->ctrlFlags |= WINFILE_RDONLY;
  }
  अगर( (flags & SQLITE_OPEN_MAIN_DB)
   && sqlite3_uri_boolean(zName, "psow", SQLITE_POWERSAFE_OVERWRITE) 
  ){
    pFile->ctrlFlags |= WINFILE_PSOW;
  }
  pFile->lastErrno = NO_ERROR;
  pFile->zPath = zName;
#अगर SQLITE_MAX_MMAP_आकार>0
  pFile->hMap = शून्य;
  pFile->pMapRegion = 0;
  pFile->mmap_आकार = 0;
  pFile->mmap_आकार_Max = sqlite3GlobalConfig.szMmap;
#पूर्ण_अगर

  Open_गणक(+1);
  वापस rc;
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
अटल पूर्णांक winDelete(
  sqlite3_vfs *pVfs,          /* Not used on win32 */
  स्थिर अक्षर *zFilename,      /* Name of file to delete */
  पूर्णांक syncDir                 /* Not used on win32 */
){
  पूर्णांक cnt = 0;
  पूर्णांक rc;
  DWORD attr;
  DWORD lastErrno = 0;
  LPWSTR zConverted;
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(syncDir);

  SimulateIOError(वापस SQLITE_IOERR_DELETE);
  OSTRACE(("DELETE name=%s, syncDir=%d\n", zFilename, syncDir));

  zConverted = winConvertFromUtf8Filename(zFilename);
  अगर( zConverted==0 ){
    OSTRACE(("DELETE name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    वापस SQLITE_IOERR_NOMEM_BKPT;
  }
  करो {
    attr = osGetFileAttributesW(zConverted);
    अगर ( attr==INVALID_FILE_ATTRIBUTES ){
      lastErrno = osGetLastError();
      अगर( lastErrno==ERROR_FILE_NOT_FOUND
       || lastErrno==ERROR_PATH_NOT_FOUND ){
        rc = SQLITE_IOERR_DELETE_NOENT; /* Already gone? */
      }अन्यथा{
        rc = SQLITE_ERROR;
      }
      अवरोध;
    }
    अगर ( attr&FILE_ATTRIBUTE_DIRECTORY ){
      rc = SQLITE_ERROR; /* Files only. */
      अवरोध;
    }
    अगर ( osDeleteFileW(zConverted) ){
      rc = SQLITE_OK; /* Deleted OK. */
      अवरोध;
    }
    अगर ( !winRetryIoerr(&cnt, &lastErrno) ){
      rc = SQLITE_ERROR; /* No more retries. */
      अवरोध;
    }
  } जबतक(1);
  अगर( rc && rc!=SQLITE_IOERR_DELETE_NOENT ){
    rc = winLogError(SQLITE_IOERR_DELETE, lastErrno, "winDelete", zFilename);
  }अन्यथा{
    winLogIoerr(cnt, __LINE__);
  }
  sqlite3_free(zConverted);
  OSTRACE(("DELETE name=%s, rc=%s\n", zFilename, sqlite3ErrName(rc)));
  वापस rc;
}

/*
** Check the existence and status of a file.
*/
अटल पूर्णांक winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  स्थिर अक्षर *zFilename,     /* Name of file to check */
  पूर्णांक flags,                 /* Type of test to make on this file */
  पूर्णांक *pResOut               /* OUT: Result */
){
  DWORD attr;
  पूर्णांक rc = 0;
  पूर्णांक cnt = 0;
  DWORD lastErrno = 0;
  LPWSTR zConverted;
  पूर्णांक noRetry = 0;           /* Do not use winRetryIoerr() */
  WIN32_FILE_ATTRIBUTE_तथ्य s_Attr_तथ्य;
  UNUSED_PARAMETER(pVfs);

  अगर( (flags & NORETRY)!=0 ){
    noRetry = 1;
    flags &= ~NORETRY;
  }  

  SimulateIOError( वापस SQLITE_IOERR_ACCESS; );
  OSTRACE(("ACCESS name=%s, flags=%x, pResOut=%p\n",
           zFilename, flags, pResOut));

  अगर( zFilename==0 ){
    *pResOut = 0;
    OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
             zFilename, pResOut, *pResOut));
    वापस SQLITE_OK;
  }

  zConverted = winConvertFromUtf8Filename(zFilename);
  अगर( zConverted==0 ){
    OSTRACE(("ACCESS name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    वापस SQLITE_IOERR_NOMEM_BKPT;
  }
  स_रखो(&s_Attr_तथ्य, 0, माप(s_Attr_तथ्य));
  जबतक( !(rc = osGetFileAttributesExW(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_तथ्य))
     && !noRetry
     && winRetryIoerr(&cnt, &lastErrno)
  ){ /* Loop until true */}
  अगर( rc ){
    /* For an SQLITE_ACCESS_EXISTS query, treat a zero-length file
    ** as if it does not exist.
    */
    अगर(    flags==SQLITE_ACCESS_EXISTS
        && s_Attr_तथ्य.n_File_आकार_ऊपरी==0
        && s_Attr_तथ्य.n_File_आकार_निचला==0 ){
      attr = INVALID_FILE_ATTRIBUTES;
    }अन्यथा{
      attr = s_Attr_तथ्य.dwFileAttributes;
    }
  }अन्यथा{
    अगर( noRetry ) lastErrno = osGetLastError();
    winLogIoerr(cnt, __LINE__);
    अगर( lastErrno!=ERROR_FILE_NOT_FOUND && lastErrno!=ERROR_PATH_NOT_FOUND ){
      sqlite3_free(zConverted);
      वापस winLogError(SQLITE_IOERR_ACCESS, lastErrno, "winAccess",
                         zFilename);
    }अन्यथा{
      attr = INVALID_FILE_ATTRIBUTES;
    }
  }
  sqlite3_free(zConverted);
  चयन( flags ){
    हाल SQLITE_ACCESS_READ:
    हाल SQLITE_ACCESS_EXISTS:
      rc = attr!=INVALID_FILE_ATTRIBUTES;
      अवरोध;
    हाल SQLITE_ACCESS_READWRITE:
      rc = attr!=INVALID_FILE_ATTRIBUTES &&
             (attr & FILE_ATTRIBUTE_READONLY)==0;
      अवरोध;
    शेष:
      निश्चित(!"Invalid flags argument");
  }
  *pResOut = rc;
  OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           zFilename, pResOut, *pResOut));
  वापस SQLITE_OK;
}

/*
** Returns non-zero if the specified path name starts with the "long path"
** prefix.
*/
अटल BOOL winIsLongPathPrefix(
  स्थिर अक्षर *zPathname
){
  वापस ( zPathname[0]=='\\' && zPathname[1]=='\\'
        && zPathname[2]=='?'  && zPathname[3]=='\\' );
}

/*
** Returns non-zero if the specified path name starts with a drive letter
** followed by a colon character.
*/
अटल BOOL winIsDriveLetterAndColon(
  स्थिर अक्षर *zPathname
){
  वापस ( sqlite3Isalpha(zPathname[0]) && zPathname[1]==':' );
}

#अगर_घोषित _WIN32
/*
** Returns non-zero if the specified path name should be used verbatim.  If
** non-zero is returned from this function, the calling function must simply
** use the provided path name verbatim -OR- resolve it into a full path name
** using the GetFullPathName Win32 API function (if available).
*/
अटल BOOL winIsVerbatimPathname(
  स्थिर अक्षर *zPathname
){
  /*
  ** If the path name starts with a forward slash or a backslash, it is either
  ** a legal UNC name, a volume relative path, or an absolute path name in the
  ** "Unix" format on Windows.  There is no easy way to differentiate between
  ** the final two cases; therefore, we return the safer return value of TRUE
  ** so that callers of this function will simply use it verbatim.
  */
  अगर ( winIsDirSep(zPathname[0]) ){
    वापस TRUE;
  }

  /*
  ** If the path name starts with a letter and a colon it is either a volume
  ** relative path or an absolute path.  Callers of this function must not
  ** attempt to treat it as a relative path name (i.e. they should simply use
  ** it verbatim).
  */
  अगर ( winIsDriveLetterAndColon(zPathname) ){
    वापस TRUE;
  }

  /*
  ** If we get to this point, the path name should almost certainly be a purely
  ** relative one (i.e. not a UNC name, not absolute, and not volume relative).
  */
  वापस FALSE;
}
#पूर्ण_अगर /* _WIN32 */

#अगर_घोषित __CYGWIN__
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
अटल व्योम winSimplifyName(अक्षर *z){
  पूर्णांक i, j;
  क्रम(i=j=0; z[i]; ++i){
    अगर( winIsDirSep(z[i]) ){
#अगर !defined(SQLITE_TEST)
      /* Some test-cases assume that "./foo" and "foo" are different */
      अगर( z[i+1]=='.' && winIsDirSep(z[i+2]) ){
        ++i;
        जारी;
      }
#पूर्ण_अगर
      अगर( !z[i+1] || (winIsDirSep(z[i+1]) && (i!=0)) ){
        जारी;
      }
      z[j++] = osGetenv?'/':'\\';
    }अन्यथा{
      z[j++] = z[i];
    }
  }
  जबतक(j<i) z[j++] = '\0';
}

#घोषणा SQLITE_MAX_SYMLINKS 100

अटल पूर्णांक mkFullPathname(
  स्थिर अक्षर *zPath,              /* Input path */
  अक्षर *zOut,                     /* Output buffer */
  पूर्णांक nOut                        /* Allocated size of buffer zOut */
){
  पूर्णांक nPath = sqlite3Strlen30(zPath);
  पूर्णांक iOff = 0;
  अगर( zPath[0]!='/' ){
    अगर( osGetcwd(zOut, nOut-2)==0 ){
      वापस winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno, "getcwd", zPath);
    }
    iOff = sqlite3Strlen30(zOut);
    zOut[iOff++] = '/';
  }
  अगर( (iOff+nPath+1)>nOut ){
    /* SQLite assumes that xFullPathname() nul-terminates the output buffer
    ** even if it returns an error.  */
    zOut[iOff] = '\0';
    वापस SQLITE_CANTOPEN_BKPT;
  }
  sqlite3_snprintf(nOut-iOff, &zOut[iOff], "%s", zPath);
  वापस SQLITE_OK;
}
#पूर्ण_अगर /* __CYGWIN__ */

/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
अटल पूर्णांक winFullPathnameNoMutex(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  स्थिर अक्षर *zRelative,        /* Possibly relative input path */
  पूर्णांक nFull,                    /* Size of output buffer in bytes */
  अक्षर *zFull                   /* Output buffer */
){
  पूर्णांक nByte;
  LPWSTR zConverted;
  अक्षर *zOut;

  /* If this path name begins with "/X:" or "\\?\", where "X" is any
  ** alphabetic character, discard the initial "/" from the pathname.
  */
  अगर( zRelative[0]=='/' && (winIsDriveLetterAndColon(zRelative+1)
       || winIsLongPathPrefix(zRelative+1)) ){
    zRelative++;
  }

  SimulateIOError( वापस SQLITE_ERROR );

#अगर_घोषित __CYGWIN__
  अगर( osGetcwd ){
    zFull[nFull-1] = '\0';
    अगर( !winIsDriveLetterAndColon(zRelative) || !winIsDirSep(zRelative[2]) ){
      पूर्णांक rc = SQLITE_OK;
      पूर्णांक nLink = 1;              /* Number of symbolic links followed so far */
      स्थिर अक्षर *zIn = zRelative; /* Input path for each iteration of loop */
      अक्षर *zDel = 0;
      काष्ठा stat buf;

      UNUSED_PARAMETER(pVfs);

      करो {
        /* Call lstat() on path zIn. Set bLink to true if the path
        ** is a symbolic link, or false otherwise.  */
        पूर्णांक bLink = 0;
        अगर( osLstat && osReadlink ) {
          अगर( osLstat(zIn, &buf)!=0 ){
            पूर्णांक myErrno = osErrno;
            अगर( myErrno!=ENOENT ){
              rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)myErrno,
                               "lstat", zIn);
            }
          }अन्यथा{
            bLink = ((buf.st_mode & 0170000) == 0120000);
          }

          अगर( bLink ){
            अगर( zDel==0 ){
              zDel = sqlite3MallocZero(nFull);
              अगर( zDel==0 ) rc = SQLITE_NOMEM;
            }अन्यथा अगर( ++nLink>SQLITE_MAX_SYMLINKS ){
              rc = SQLITE_CANTOPEN_BKPT;
            }

            अगर( rc==SQLITE_OK ){
              nByte = osReadlink(zIn, zDel, nFull-1);
              अगर( nByte ==(DWORD)-1 ){
                rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno,
                                 "readlink", zIn);
              }अन्यथा{
                अगर( zDel[0]!='/' ){
                  पूर्णांक n;
                  क्रम(n = sqlite3Strlen30(zIn); n>0 && zIn[n-1]!='/'; n--);
                  अगर( nByte+n+1>nFull ){
                    rc = SQLITE_CANTOPEN_BKPT;
                  }अन्यथा{
                    स_हटाओ(&zDel[n], zDel, nByte+1);
                    स_नकल(zDel, zIn, n);
                    nByte += n;
                  }
                }
                zDel[nByte] = '\0';
              }
            }

            zIn = zDel;
          }
        }

        निश्चित( rc!=SQLITE_OK || zIn!=zFull || zIn[0]=='/' );
        अगर( rc==SQLITE_OK && zIn!=zFull ){
          rc = mkFullPathname(zIn, zFull, nFull);
        }
        अगर( bLink==0 ) अवरोध;
        zIn = zFull;
      }जबतक( rc==SQLITE_OK );

      sqlite3_free(zDel);
      winSimplifyName(zFull);
      वापस rc;
    }
  }
#पूर्ण_अगर /* __CYGWIN__ */

#अगर defined(_WIN32)
  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing. This function could fail if, for example, the
  ** current working directory has been unlinked.
  */
  SimulateIOError( वापस SQLITE_ERROR );
  अगर ( sqlite3_तथ्य_directory && !winIsVerbatimPathname(zRelative) ){
    /*
    ** NOTE: We are dealing with a relative path name and the data
    **       directory has been set.  Therefore, use it as the basis
    **       for converting the relative path name to an absolute
    **       one by prepending the data directory and a backslash.
    */
    sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s%c%s",
                     sqlite3_तथ्य_directory, winGetDirSep(), zRelative);
    वापस SQLITE_OK;
  }
#पूर्ण_अगर
  zConverted = winConvertFromUtf8Filename(zRelative);
  अगर( zConverted==0 ){
    वापस SQLITE_IOERR_NOMEM_BKPT;
  }
  {
    LPWSTR z_अस्थायी;
    nByte = osGetFullPathNameW(zConverted, 0, 0, 0);
    अगर( nByte==0 ){
      sqlite3_free(zConverted);
      वापस winLogError(SQLITE_CANTOPEN_FULLPATH, osGetLastError(),
                         "winFullPathname1", zRelative);
    }
    nByte += 3;
    z_अस्थायी = sqlite3MallocZero( (u64)nByte*माप(WCHAR) );
    अगर( z_अस्थायी==0 ){
      sqlite3_free(zConverted);
      वापस SQLITE_IOERR_NOMEM_BKPT;
    }
    nByte = osGetFullPathNameW(zConverted, nByte, z_अस्थायी, 0);
    अगर( nByte==0 ){
      sqlite3_free(zConverted);
      sqlite3_free(z_अस्थायी);
      वापस winLogError(SQLITE_CANTOPEN_FULLPATH, osGetLastError(),
                         "winFullPathname2", zRelative);
    }
    sqlite3_free(zConverted);
    zOut = winUnicodeToUtf8(z_अस्थायी);
    sqlite3_free(z_अस्थायी);
  }
  अगर( zOut ){
#अगर_घोषित __CYGWIN__
    अगर( स_भेद(zOut, "\\\\?\\", 4) ){
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut);
    }अन्यथा अगर( स_भेद(zOut+4, "UNC\\", 4) ){
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut+4);
    }अन्यथा{
      अक्षर *p = zOut+6;
      *p = '\\';
      अगर( osGetcwd ){
        /* On Cygwin, UNC paths use forward slashes */
        जबतक( *p ){
          अगर( *p=='\\' ) *p = '/';
          ++p;
        }
      }
      sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut+6);
    }
#अन्यथा
    sqlite3_snprintf(MIN(nFull, pVfs->mxPathname), zFull, "%s", zOut);
#पूर्ण_अगर /* __CYGWIN__ */
    sqlite3_free(zOut);
    वापस SQLITE_OK;
  }अन्यथा{
    वापस SQLITE_IOERR_NOMEM_BKPT;
  }
}
अटल पूर्णांक winFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  स्थिर अक्षर *zRelative,        /* Possibly relative input path */
  पूर्णांक nFull,                    /* Size of output buffer in bytes */
  अक्षर *zFull                   /* Output buffer */
){
  पूर्णांक rc;
  MUTEX_LOGIC( sqlite3_mutex *pMutex; )
  MUTEX_LOGIC( pMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR); )
  sqlite3_mutex_enter(pMutex);
  rc = winFullPathnameNoMutex(pVfs, zRelative, nFull, zFull);
  sqlite3_mutex_leave(pMutex);
  वापस rc;
}

#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
अटल व्योम *winDlOpen(sqlite3_vfs *pVfs, स्थिर अक्षर *zFilename){
  HANDLE h = 0;
  LPWSTR zConverted = winConvertFromUtf8Filename(zFilename);
  UNUSED_PARAMETER(pVfs);
  अगर( zConverted==0 ){
    OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (व्योम*)0));
    वापस 0;
  }
  h = osLoadLibraryW ? osLoadLibraryW(zConverted) : 0;
  OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (व्योम*)h));
  sqlite3_free(zConverted);
  वापस (व्योम*)h;
}
अटल व्योम winDlError(sqlite3_vfs *pVfs, पूर्णांक nBuf, अक्षर *zBufOut){
  UNUSED_PARAMETER(pVfs);
  win_Get_Last_Error_संदेश(osGetLastError(), nBuf, zBufOut);
}
अटल व्योम (*winDlSym(sqlite3_vfs *pVfs,व्योम *pH,स्थिर अक्षर *zSym))(व्योम){
  FARPROC proc = 0;
  UNUSED_PARAMETER(pVfs);
  proc = osGetProcAddressA ? osGetProcAddressA((HANDLE)pH, zSym) : 0;
  OSTRACE(("DLSYM handle=%p, symbol=%s, address=%p\n",
           (व्योम*)pH, zSym, (व्योम*)proc));
  वापस (व्योम(*)(व्योम))proc;
}
अटल व्योम winDlClose(sqlite3_vfs *pVfs, व्योम *pHandle){
  UNUSED_PARAMETER(pVfs);
  अगर( osFreeLibrary!=0 ) osFreeLibrary((HANDLE)pHandle);
  OSTRACE(("DLCLOSE handle=%p\n", (व्योम*)pHandle));
}
#अन्यथा /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #घोषणा winDlOpen  0
  #घोषणा winDlError 0
  #घोषणा winDlSym   0
  #घोषणा winDlClose 0
#पूर्ण_अगर

/* State information for the randomness gatherer. */
प्रकार काष्ठा EntropyGatherer EntropyGatherer;
काष्ठा EntropyGatherer {
  अचिन्हित अक्षर *a;   /* Gather entropy into this buffer */
  पूर्णांक na;             /* Size of a[] in bytes */
  पूर्णांक i;              /* XOR next input into a[i] */
  पूर्णांक nXor;           /* Number of XOR operations done */
};

#अगर !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
/* Mix sz bytes of entropy into p. */
अटल व्योम xorMemory(EntropyGatherer *p, अचिन्हित अक्षर *x, पूर्णांक sz){
  पूर्णांक j, k;
  क्रम(j=0, k=p->i; j<sz; j++){
    p->a[k++] ^= x[j];
    अगर( k>=p->na ) k = 0;
  }
  p->i = k;
  p->nXor += sz;
}
#पूर्ण_अगर /* !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS) */

/*
** Write up to nBuf bytes of randomness into zBuf.
*/
अटल पूर्णांक winRandomness(sqlite3_vfs *pVfs, पूर्णांक nBuf, अक्षर *zBuf){
#अगर defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS)
  UNUSED_PARAMETER(pVfs);
  स_रखो(zBuf, 0, nBuf);
  वापस nBuf;
#अन्यथा
  EntropyGatherer e;
  UNUSED_PARAMETER(pVfs);
  स_रखो(zBuf, 0, nBuf);
  e.a = (अचिन्हित अक्षर*)zBuf;
  e.na = nBuf;
  e.nXor = 0;
  e.i = 0;
  {
    FILETIME x;
    osGetSystemTimeAsFileTime(&x);
    xorMemory(&e, (अचिन्हित अक्षर*)&x, माप(x));
  }
  {
    DWORD pid = osGetCurrentProcessId();
    xorMemory(&e, (अचिन्हित अक्षर*)&pid, माप(pid));
  }
  {
    ULONGLONG cnt = osGetTickCount64();
    xorMemory(&e, (अचिन्हित अक्षर*)&cnt, माप(cnt));
  }
  {
    LARGE_INTEGER i;
    os_Query_Performance_गणक(&i);
    xorMemory(&e, (अचिन्हित अक्षर*)&i, माप(i));
  }
#अगर SQLITE_WIN32_USE_UUID
#अगर_घोषित SQLITE_UWP
# error SQLITE_WIN32_USE_UUID is incompatible with SQLITE_UWP
#पूर्ण_अगर
  {
    UUID id;
    स_रखो(&id, 0, माप(id));
    osUuidCreate(&id);
    xorMemory(&e, (अचिन्हित अक्षर*)&id, माप(id));
    स_रखो(&id, 0, माप(UUID));
    osUuidCreateSequential(&id);
    xorMemory(&e, (अचिन्हित अक्षर*)&id, माप(id));
  }
#पूर्ण_अगर /* SQLITE_WIN32_USE_UUID */
  वापस e.nXor>nBuf ? nBuf : e.nXor;
#पूर्ण_अगर /* defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS) */
}


/*
** Sleep for a little while.  Return the amount of time slept.
*/
अटल पूर्णांक winSleep(sqlite3_vfs *pVfs, पूर्णांक microsec){
  osSleep((microsec+999)/1000);
  UNUSED_PARAMETER(pVfs);
  वापस ((microsec+999)/1000)*1000;
}

/*
** The following variable, if set to a non-zero value, is interpreted as
** the number of seconds since 1970 and is used to set the result of
** sqlite3OsCurrentTime() during testing.
*/
#अगर_घोषित SQLITE_TEST
पूर्णांक sqlite3_current_time = 0;  /* Fake system time in seconds since 1970. */
#पूर्ण_अगर

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
अटल पूर्णांक winCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *piNow){
  /* FILETIME structure is a 64-bit value representing the number of
     100-nanosecond intervals since January 1, 1601 (= JD 2305813.5).
  */
  FILETIME ft;
  अटल स्थिर sqlite3_int64 winFiletimeEpoch = 23058135*(sqlite3_int64)8640000;
#अगर_घोषित SQLITE_TEST
  अटल स्थिर sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
#पूर्ण_अगर
  /* 2^32 - to avoid use of LL and warnings in gcc */
  अटल स्थिर sqlite3_int64 max32_Bit_मूल्य =
      (sqlite3_int64)2000000000 + (sqlite3_int64)2000000000 +
      (sqlite3_int64)294967296;

  osGetSystemTimeAsFileTime( &ft );
  *piNow = winFiletimeEpoch +
            ((((sqlite3_int64)ft.dw_ऊपरी_Date_Time)*max32_Bit_मूल्य) +
               (sqlite3_int64)ft.dw_निचला_Date_Time)/(sqlite3_int64)10000;

#अगर_घोषित SQLITE_TEST
  अगर( sqlite3_current_time ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_time + unixEpoch;
  }
#पूर्ण_अगर
  UNUSED_PARAMETER(pVfs);
  वापस SQLITE_OK;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
अटल पूर्णांक winCurrentTime(sqlite3_vfs *pVfs, द्विगुन *prNow){
  पूर्णांक rc;
  sqlite3_int64 i;
  rc = winCurrentTimeInt64(pVfs, &i);
  अगर( !rc ){
    *prNow = i/86400000.0;
  }
  वापस rc;
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
अटल पूर्णांक winGetLastError(sqlite3_vfs *pVfs, पूर्णांक nBuf, अक्षर *zBuf){
  DWORD e = osGetLastError();
  UNUSED_PARAMETER(pVfs);
  अगर( nBuf>0 ) win_Get_Last_Error_संदेश(e, nBuf, zBuf);
  वापस e;
}

/*
** Initialize and deinitialize the operating system interface.
*/
पूर्णांक sqlite3_os_init(व्योम){
  अटल sqlite3_vfs winVfs = {
    3,                     /* iVersion */
    माप(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_PATH_BYTES, /* mxPathname */
    0,                     /* pNext */
    "win32",               /* zName */
    &win_App_तथ्य,           /* pAppData */
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
    win_अग्र_System_Call,     /* xNextSystemCall */
  };
  अटल sqlite3_vfs winLongPathVfs = {
    3,                     /* iVersion */
    माप(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_PATH_BYTES, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath",      /* zName */
    &win_App_तथ्य,           /* pAppData */
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
    win_अग्र_System_Call,     /* xNextSystemCall */
  };
  अटल sqlite3_vfs winNolockVfs = {
    3,                     /* iVersion */
    माप(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_PATH_BYTES, /* mxPathname */
    0,                     /* pNext */
    "win32-none",          /* zName */
    &win_Nolock_App_तथ्य,     /* pAppData */
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
    win_अग्र_System_Call,     /* xNextSystemCall */
  };
  अटल sqlite3_vfs winLongPathNolockVfs = {
    3,                     /* iVersion */
    माप(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_PATH_BYTES, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath-none", /* zName */
    &win_Nolock_App_तथ्य,     /* pAppData */
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
    win_अग्र_System_Call,     /* xNextSystemCall */
  };

  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  निश्चित( Array_आकार(aSyscall)==60 );
  निश्चित( म_भेद(aSyscall[0].zName,"AreFileApisANSI")==0 );
  निश्चित( म_भेद(aSyscall[8].zName,"FreeLibrary")==0 );
  निश्चित( म_भेद(aSyscall[16].zName,"GetSystemInfo")==0 );
  निश्चित( म_भेद(aSyscall[24].zName,"HeapReAlloc")==0 );
  निश्चित( म_भेद(aSyscall[32].zName,"MapViewOfFileFromApp")==0 );
  निश्चित( म_भेद(aSyscall[40].zName,"UnmapViewOfFile")==0 );
  निश्चित( म_भेद(aSyscall[48].zName,"UuidCreate")==0 );
  निश्चित( म_भेद(aSyscall[56].zName,"lstat")==0 );

  /* get memory map allocation granularity */
  स_रखो(&winSysInfo, 0, माप(SYSTEM_INFO));
  osGetSystemInfo(&winSysInfo);
  निश्चित( winSysInfo.dwAllocationGranularity>0 );
  निश्चित( winSysInfo.dw_Page_आकार>0 );

  sqlite3_vfs_register(&winVfs, 1);
  sqlite3_vfs_register(&winLongPathVfs, 0);
  sqlite3_vfs_register(&winNolockVfs, 0);
  sqlite3_vfs_register(&winLongPathNolockVfs, 0);
#अगर_अघोषित SQLITE_OMIT_WAL
  winBigLock = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#पूर्ण_अगर

  वापस SQLITE_OK;
}

पूर्णांक sqlite3_os_end(व्योम){
#अगर_अघोषित SQLITE_OMIT_WAL
  winBigLock = 0;
#पूर्ण_अगर
  वापस SQLITE_OK;
}

#पूर्ण_अगर /* SQLITE_OS_WIN */
