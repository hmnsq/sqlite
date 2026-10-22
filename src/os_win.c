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
#అన్తర్భావ "sqliteInt.h"
#ఒకవేళ SQLITE_OS_WIN               /* This file is used for Windows only */

/*
** Include code that is common to all os_*.c files
*/
#అన్తర్భావ "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#అన్తర్భావ "os_win.h"

/*
** This constant should already be defined (in the "WinDef.h" SDK file).
*/
#నిర్వచితం_కాకపోతే MAX_మార్గం
#  define MAX_మార్గం                      (260)
#అయితే_ముగింపు

/*
** Maximum pathname length (in chars) for Win32.  This should normally be
** MAX_PATH.
*/
#నిర్వచితం_కాకపోతే SQLITE_WIN32_MAX_మార్గం_CHARS
#  define SQLITE_WIN32_MAX_మార్గం_CHARS   (MAX_మార్గం)
#అయితే_ముగింపు

/*
** This constant should already be defined (in the "WinNT.h" SDK file).
*/
#నిర్వచితం_కాకపోతే UNICODE_STRING_MAX_CHARS
#  define UNICODE_STRING_MAX_CHARS      (32767)
#అయితే_ముగింపు

/*
** Maximum pathname length (in chars) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS.
*/
#నిర్వచితం_కాకపోతే SQLITE_WINNT_MAX_మార్గం_CHARS
#  define SQLITE_WINNT_MAX_మార్గం_CHARS   (UNICODE_STRING_MAX_CHARS)
#అయితే_ముగింపు

/*
** Maximum pathname length (in bytes) for Win32.  The MAX_PATH macro is in
** characters, so we allocate 4 bytes per character assuming worst-case of
** 4-bytes-per-character for UTF8.
*/
#నిర్వచితం_కాకపోతే SQLITE_WIN32_MAX_మార్గం_బైట్లు
#  define SQLITE_WIN32_MAX_మార్గం_బైట్లు   (SQLITE_WIN32_MAX_మార్గం_CHARS*4)
#అయితే_ముగింపు

/*
** Maximum pathname length (in bytes) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS * sizeof(WCHAR).
*/
#నిర్వచితం_కాకపోతే SQLITE_WINNT_MAX_మార్గం_బైట్లు
#  define SQLITE_WINNT_MAX_మార్గం_బైట్లు   \
                            (పరిమాణం(WCHAR) * SQLITE_WINNT_MAX_మార్గం_CHARS)
#అయితే_ముగింపు

/*
** Maximum error message length (in chars) for WinRT.
*/
#నిర్వచితం_కాకపోతే SQLITE_WIN32_MAX_ERRMSG_CHARS
#  define SQLITE_WIN32_MAX_ERRMSG_CHARS (1024)
#అయితే_ముగింపు

/*
** Returns non-zero if the character should be treated as a directory
** separator.
*/
#నిర్వచితం_కాకపోతే winIsDirSep
#  define winIsDirSep(a)                (((a) == '/') || ((a) == '\\'))
#అయితే_ముగింపు

/*
** This macro is used when a local variable is set to a value that is
** [sometimes] not used by the code (e.g. via conditional compilation).
*/
#నిర్వచితం_కాకపోతే వాడని_VARIABLE_విలువ
#  define వాడని_VARIABLE_విలువ(x)      (శూన్య)(x)
#అయితే_ముగింపు

/*
** Returns the character that should be used as the directory separator.
*/
#నిర్వచితం_కాకపోతే winGetDirSep
#  define winGetDirSep()                '\\'
#అయితే_ముగింపు

/*
** Some older Microsoft compilers lack the following definition.
*/
#నిర్వచితం_కాకపోతే INVALID_FILE_లక్షణాలు
# define INVALID_FILE_లక్షణాలు ((DWORD)-1)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే FILE_FLAG_ముసుగు
# define FILE_FLAG_ముసుగు          (0xFF3C0000)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే FILE_ATTRIBUTE_ముసుగు
# define FILE_ATTRIBUTE_ముసుగు     (0x0003FFF7)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
/* Forward references to structures used for WAL */
రకనిర్వచనం నిర్మాణం winShm winShm;           /* A connection to shared-memory */
రకనిర్వచనం నిర్మాణం win_Shm_కణుపు win_Shm_కణుపు;   /* A region of shared-memory */
#అయితే_ముగింపు

/*
** The winFile structure is a subclass of sqlite3_file* specific to the win32
** portability layer.
*/
రకనిర్వచనం నిర్మాణం winFile winFile;
నిర్మాణం winFile {
  స్థిరరాశి sqlite3_io_methods *p_పద్ధతి; /*** Must be first ***/
  sqlite3_vfs *pVfs;      /* The VFS used to open this file */
  HANDLE h;               /* Handle for accessing the file */
  u8 locktype;            /* Type of lock currently held on this file */
  పొట్టి shared_తాళం_Byte;   /* Randomly chosen byte used as a shared lock */
  u8 ctrlFlags;           /* Flags.  See WINFILE_* below */
  DWORD చివరి_Errno;        /* The Windows errno from the last I/O error */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  winShm *pShm;           /* Instance of shared memory on this file */
#అయితే_ముగింపు
  స్థిరరాశి అక్షరం *z_మార్గం;      /* Full pathname of this file */
  పూర్ణాంకం szChunk;            /* Chunk size configured by FCNTL_CHUNK_SIZE */
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  పూర్ణాంకం n_Fetch_బహిర్గతం;                /* Number of outstanding xFetch references */
  HANDLE hMap;                  /* Handle for accessing memory mapping */
  శూన్య *pMapRegion;             /* Area memory mapped */
  sqlite3_int64 mmap_కొలత;       /* Size of mapped region */
  sqlite3_int64 mmap_కొలత_Max;    /* Configured FCNTL_MMAP_SIZE value */
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  DWORD iBusyTimeout;        /* Wait this many millisec on locks */
  పూర్ణాంకం b_ఖండం_On_Connect;
#అయితే_ముగింపు
};

#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
# define winFileBusyTimeout(pDbFd) pDbFd->iBusyTimeout
#లేకపోతే
# define winFileBusyTimeout(pDbFd) 0
#అయితే_ముగింపు

/*
** The winVfsAppData structure is used for the pAppData member for all of the
** Win32 VFS variants.
*/
రకనిర్వచనం నిర్మాణం win_Vfs_App_దత్తాంశం win_Vfs_App_దత్తాంశం;
నిర్మాణం win_Vfs_App_దత్తాంశం {
  స్థిరరాశి sqlite3_io_methods *p_పద్ధతి; /* The file I/O methods to use. */
  శూన్య *p_App_దత్తాంశం;                    /* The extra pAppData, if any. */
  BOOL b_No_తాళం;                      /* Non-zero if locking is disabled. */
};

/*
** Allowed values for winFile.ctrlFlags
*/
#నిర్వచించు WINFILE_RDONLY          0x02   /* Connection is read only */
#నిర్వచించు WINFILE_PERSIST_WAL     0x04   /* Persistent WAL mode */
#నిర్వచించు WINFILE_PSOW            0x10   /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */

/*
 * The size of the buffer used by sqlite3_win32_write_debug().
 */
#నిర్వచితం_కాకపోతే SQLITE_WIN32_DBG_BUF_కొలత
#  define SQLITE_WIN32_DBG_BUF_కొలత   ((పూర్ణాంకం)(4096-పరిమాణం(DWORD)))
#అయితే_ముగింపు

/*
 * If compiled with SQLITE_WIN32_MALLOC on Windows, we will use the
 * various Win32 API heap functions instead of our own.
 */
#నిర్వచితమైతే SQLITE_WIN32_MALLOC

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
#నిర్వచితం_కాకపోతే SQLITE_WIN32_కుప్ప_CREATE
#  define SQLITE_WIN32_కుప్ప_CREATE        (TRUE)
#అయితే_ముగింపు

/*
 * This is the maximum possible initial size of the Win32-specific heap, in
 * bytes.
 */
#నిర్వచితం_కాకపోతే SQLITE_WIN32_కుప్ప_MAX_INIT_కొలత
#  define SQLITE_WIN32_కుప్ప_MAX_INIT_కొలత (4294967295U)
#అయితే_ముగింపు

/*
 * This is the extra space for the initial size of the Win32-specific heap,
 * in bytes.  This value may be zero.
 */
#నిర్వచితం_కాకపోతే SQLITE_WIN32_కుప్ప_INIT_EXTRA
#  define SQLITE_WIN32_కుప్ప_INIT_EXTRA  (4194304)
#అయితే_ముగింపు

/*
 * Calculate the maximum legal cache size, in pages, based on the maximum
 * possible initial heap size and the default page size, setting aside the
 * needed extra space.
 */
#నిర్వచితం_కాకపోతే SQLITE_WIN32_MAX_CACHE_కొలత
#  define SQLITE_WIN32_MAX_CACHE_కొలత   (((SQLITE_WIN32_కుప్ప_MAX_INIT_కొలత) - \
                                          (SQLITE_WIN32_కుప్ప_INIT_EXTRA)) / \
                                         (SQLITE_DEFAULT_పుట_కొలత))
#అయితే_ముగింపు

/*
 * This is cache size used in the calculation of the initial size of the
 * Win32-specific heap.  It cannot be negative.
 */
#నిర్వచితం_కాకపోతే SQLITE_WIN32_CACHE_కొలత
#  ఒకవేళ SQLITE_DEFAULT_CACHE_కొలత>=0
#    define SQLITE_WIN32_CACHE_కొలత     (SQLITE_DEFAULT_CACHE_కొలత)
#  లేకపోతే
#    define SQLITE_WIN32_CACHE_కొలత     (-(SQLITE_DEFAULT_CACHE_కొలత))
#  endif
#అయితే_ముగింపు

/*
 * Make sure that the calculated cache size, in pages, cannot cause the
 * initial size of the Win32-specific heap to exceed the maximum amount
 * of memory that can be specified in the call to HeapCreate.
 */
#ఒకవేళ SQLITE_WIN32_CACHE_కొలత>SQLITE_WIN32_MAX_CACHE_కొలత
#  undef SQLITE_WIN32_CACHE_కొలత
#  define SQLITE_WIN32_CACHE_కొలత       (2000)
#అయితే_ముగింపు

/*
 * The initial size of the Win32-specific heap.  This value may be zero.
 */
#నిర్వచితం_కాకపోతే SQLITE_WIN32_కుప్ప_INIT_కొలత
#  define SQLITE_WIN32_కుప్ప_INIT_కొలత   ((SQLITE_WIN32_CACHE_కొలత) * \
                                         (SQLITE_DEFAULT_పుట_కొలత) + \
                                         (SQLITE_WIN32_కుప్ప_INIT_EXTRA))
#అయితే_ముగింపు

/*
 * The maximum size of the Win32-specific heap.  This value may be zero.
 */
#నిర్వచితం_కాకపోతే SQLITE_WIN32_కుప్ప_MAX_కొలత
#  define SQLITE_WIN32_కుప్ప_MAX_కొలత    (0)
#అయితే_ముగింపు

/*
 * The extra flags to use in calls to the Win32 heap APIs.  This value may be
 * zero for the default behavior.
 */
#నిర్వచితం_కాకపోతే SQLITE_WIN32_కుప్ప_FLAGS
#  define SQLITE_WIN32_కుప్ప_FLAGS       (0)
#అయితే_ముగింపు


/*
** The winMemData structure stores information required by the Win32-specific
** sqlite3_mem_methods implementation.
*/
రకనిర్వచనం నిర్మాణం win_Mem_దత్తాంశం win_Mem_దత్తాంశం;
నిర్మాణం win_Mem_దత్తాంశం {
#నిర్వచితం_కాకపోతే NDEBUG
  u32 magic1;   /* Magic number to detect structure corruption. */
#అయితే_ముగింపు
  HANDLE h_కుప్ప; /* The handle to our heap. */
  BOOL bOwned;  /* Do we own the heap (i.e. destroy it on shutdown)? */
#నిర్వచితం_కాకపోతే NDEBUG
  u32 magic2;   /* Magic number to detect structure corruption. */
#అయితే_ముగింపు
};

#నిర్వచితం_కాకపోతే NDEBUG
#నిర్వచించు WINMEM_MAGIC1     0x42b2830b
#నిర్వచించు WINMEM_MAGIC2     0xbd4d7cf4
#అయితే_ముగింపు

స్థిర నిర్మాణం win_Mem_దత్తాంశం win_mem_దత్తాంశం = {
#నిర్వచితం_కాకపోతే NDEBUG
  WINMEM_MAGIC1,
#అయితే_ముగింపు
  ఖాళీ, FALSE
#నిర్వచితం_కాకపోతే NDEBUG
  ,WINMEM_MAGIC2
#అయితే_ముగింపు
};

#నిర్వచితం_కాకపోతే NDEBUG
#నిర్వచించు winMemAssertMagic1() ప్రతిజ్ఞా( win_mem_దత్తాంశం.magic1==WINMEM_MAGIC1 )
#నిర్వచించు winMemAssertMagic2() ప్రతిజ్ఞా( win_mem_దత్తాంశం.magic2==WINMEM_MAGIC2 )
#నిర్వచించు winMemAssertMagic()  winMemAssertMagic1(); winMemAssertMagic2();
#లేకపోతే
#నిర్వచించు winMemAssertMagic()
#అయితే_ముగింపు

#నిర్వచించు win_Mem_Get_దత్తాంశం_Ptr()  &win_mem_దత్తాంశం
#నిర్వచించు win_Mem_Get_కుప్ప()     win_mem_దత్తాంశం.h_కుప్ప
#నిర్వచించు winMemGetOwned()    win_mem_దత్తాంశం.bOwned

స్థిర శూన్య *winMemMalloc(పూర్ణాంకం n_బైట్లు);
స్థిర శూన్య winMemFree(శూన్య *p_పూర్వ);
స్థిర శూన్య *winMemRealloc(శూన్య *p_పూర్వ, పూర్ణాంకం n_బైట్లు);
స్థిర పూర్ణాంకం win_Mem_కొలత(శూన్య *p);
స్థిర పూర్ణాంకం winMemRoundup(పూర్ణాంకం n);
స్థిర పూర్ణాంకం winMemInit(శూన్య *p_App_దత్తాంశం);
స్థిర శూన్య winMemShutdown(శూన్య *p_App_దత్తాంశం);

స్థిరరాశి sqlite3_mem_methods *sqlite3MemGetWin32(శూన్య);
#అయితే_ముగింపు /* SQLITE_WIN32_MALLOC */

#నిర్వచితం_కాకపోతే SYSCALL
#  define SYSCALL sqlite3_syscall_ptr
#అయితే_ముగింపు

/*
** Many system calls are accessed through pointer-to-functions so that
** they may be overridden at runtime to facilitate fault injection during
** testing and sandboxing.  The following array holds the names and pointers
** to all overrideable system calls.
*/
స్థిర నిర్మాణం win_syscall {
  స్థిరరాశి అక్షరం *zName;            /* Name of the system call */
  sqlite3_syscall_ptr pCurrent; /* Current value of the system call */
  sqlite3_syscall_ptr pDefault; /* Default value */
} aSyscall[] = {
  { "AreFileApisANSI",         (SYSCALL)AreFileApisANSI,         0 },
#నిర్వచించు osAreFileApisANSI ((BOOL(WINAPI*)(VOID))aSyscall[0].pCurrent)

  { "CloseHandle",             (SYSCALL)CloseHandle,             0 },
#నిర్వచించు osCloseHandle ((BOOL(WINAPI*)(HANDLE))aSyscall[1].pCurrent)


  { "CreateFileW",             (SYSCALL)CreateFileW,             0 },
#నిర్వచించు osCreateFileW ((HANDLE(WINAPI*)(LPCWSTR,DWORD,DWORD, \
        LPSECURITY_లక్షణాలు,DWORD,DWORD,HANDLE))aSyscall[2].pCurrent)

#ఒకవేళ defined(SQLITE_UWP)
  { "CreateFileMappingFromApp",(SYSCALL)CreateFileMappingFromApp,0 },
#లేకపోతే
  { "CreateFileMappingFromApp",(SYSCALL)0,                       0 },
#అయితే_ముగింపు
#నిర్వచించు osCreateFileMappingFromApp ((HANDLE(WINAPI*)(HANDLE,\
        PSECURITY_లక్షణాలు, \
        ULONG,ULONG64,PCWSTR))aSyscall[3].pCurrent)

#ఒకవేళ !defined(SQLITE_UWP)
  { "CreateFileMappingW",      (SYSCALL)CreateFileMappingW,      0 },
#లేకపోతే
  { "CreateFileMappingW",      (SYSCALL)0,                       0 },
#అయితే_ముగింపు
#నిర్వచించు osCreateFileMappingW ((HANDLE(WINAPI*)(HANDLE,LPSECURITY_లక్షణాలు, \
        DWORD,DWORD,DWORD,LPCWSTR))aSyscall[4].pCurrent)

  { "DeleteFileW",             (SYSCALL)DeleteFileW,             0 },
#నిర్వచించు osDeleteFileW ((BOOL(WINAPI*)(LPCWSTR))aSyscall[5].pCurrent)

  { "FlushFileBuffers",        (SYSCALL)FlushFileBuffers,        0 },
#నిర్వచించు osFlushFileBuffers ((BOOL(WINAPI*)(HANDLE))aSyscall[6].pCurrent)

  { "FormatMessageW",          (SYSCALL)Format_సందేశం_W,          0 },
#నిర్వచించు os_Format_సందేశం_W ((DWORD(WINAPI*)(DWORD,LPCVOID,DWORD,DWORD,LPWSTR, \
        DWORD,va_list*))aSyscall[7].pCurrent)

#ఒకవేళ !defined(SQLITE_OMIT_ఎక్కించు_EXTENSION) && !defined(SQLITE_UWP)
  { "FreeLibrary",             (SYSCALL)FreeLibrary,             0 },
#లేకపోతే
  { "FreeLibrary",             (SYSCALL)0,                       0 },
#అయితే_ముగింపు

#నిర్వచించు osFreeLibrary ((BOOL(WINAPI*)(HMODULE))aSyscall[8].pCurrent)

  { "GetCurrentProcessId",     (SYSCALL)GetCurrentProcessId,     0 },
#నిర్వచించు osGetCurrentProcessId ((DWORD(WINAPI*)(VOID))aSyscall[9].pCurrent)

  { "GetFileAttributesW",      (SYSCALL)Get_File_లక్షణాలు_W,      0 },
#నిర్వచించు os_Get_File_లక్షణాలు_W ((DWORD(WINAPI*)(LPCWSTR))aSyscall[10].pCurrent)

  { "GetFileAttributesExW",    (SYSCALL)Get_File_లక్షణాలు_Ex_W,    0 },
#నిర్వచించు os_Get_File_లక్షణాలు_Ex_W ((BOOL(WINAPI*)(LPCWSTR,GET_FILEEX_INFO_స్థాయిలు, \
        LPVOID))aSyscall[11].pCurrent)

  { "GetFileSizeEx",           (SYSCALL)Get_File_కొలత_Ex,           0 },
#నిర్వచించు os_Get_File_కొలత_Ex ((BOOL(WINAPI*)(HANDLE, \
                        PLARGE_INTEGER))aSyscall[12].pCurrent)

  { "GetFullPathNameW",        (SYSCALL)Get_నిండిన_మార్గం_Name_W,        0 },
#నిర్వచించు os_Get_నిండిన_మార్గం_Name_W ((DWORD(WINAPI*)(LPCWSTR,DWORD,LPWSTR, \
        LPWSTR*))aSyscall[13].pCurrent)

  { "GetLastError",            (SYSCALL)Get_చివరి_Error,            0 },
#నిర్వచించు os_Get_చివరి_Error ((DWORD(WINAPI*)(VOID))aSyscall[14].pCurrent)

#ఒకవేళ !defined(SQLITE_OMIT_ఎక్కించు_EXTENSION) && !defined(SQLITE_UWP)
  { "GetProcAddressA",         (SYSCALL)Get_Proc_చిరునామా,          0 },
#లేకపోతే
  { "GetProcAddressA",         (SYSCALL)0,                       0 },
#అయితే_ముగింపు
#నిర్వచించు os_Get_Proc_చిరునామా_A ((FARPROC(WINAPI*)(HMODULE, \
        LPCSTR))aSyscall[15].pCurrent)

  { "GetSystemInfo",           (SYSCALL)GetSystemInfo,           0 },
#నిర్వచించు osGetSystemInfo ((VOID(WINAPI*)(LPSYSTEM_INFO))aSyscall[16].pCurrent)

  { "GetSystemTimeAsFileTime", (SYSCALL)Get_System_సమయం_As_File_సమయం, 0 },
#నిర్వచించు os_Get_System_సమయం_As_File_సమయం ((VOID(WINAPI*)( \
        LPFILETIME))aSyscall[17].pCurrent)

#నిర్వచితమైతే SQLITE_UWP
  { "GetTempPathW",            (SYSCALL)0,                       0 },
#లేకపోతే
  { "GetTempPathW",            (SYSCALL)Get_తాత్కాలిక_మార్గం_W,            0 },
#అయితే_ముగింపు
#నిర్వచించు os_Get_తాత్కాలిక_మార్గం_W ((DWORD(WINAPI*)(DWORD,LPWSTR))aSyscall[18].pCurrent)

  { "GetTickCount64",          (SYSCALL)GetTickCount64,          0 },
#నిర్వచించు osGetTickCount64 ((ULONGLONG(WINAPI*)(VOID))aSyscall[19].pCurrent)

#నిర్వచితమైతే SQLITE_UWP
  { "HeapAlloc",               (SYSCALL)0,                       0 },
#లేకపోతే
  { "HeapAlloc",               (SYSCALL)కుప్ప_Alloc,               0 },
#అయితే_ముగింపు
#నిర్వచించు os_కుప్ప_Alloc ((LPVOID(WINAPI*)(HANDLE,DWORD, \
        కొలత_T))aSyscall[20].pCurrent)

#నిర్వచితమైతే SQLITE_UWP
  { "HeapCreate",              (SYSCALL)0,                       0 },
#లేకపోతే
  { "HeapCreate",              (SYSCALL)కుప్ప_Create,              0 },
#అయితే_ముగింపు
#నిర్వచించు os_కుప్ప_Create ((HANDLE(WINAPI*)(DWORD,కొలత_T, \
        కొలత_T))aSyscall[21].pCurrent)

#నిర్వచితమైతే SQLITE_UWP
  { "HeapDestroy",             (SYSCALL)0,                       0 },
#లేకపోతే
  { "HeapDestroy",             (SYSCALL)కుప్ప_Destroy,             0 },
#అయితే_ముగింపు
#నిర్వచించు os_కుప్ప_Destroy ((BOOL(WINAPI*)(HANDLE))aSyscall[22].pCurrent)

#నిర్వచితమైతే SQLITE_UWP
  { "HeapFree",                (SYSCALL)0,                       0 },
#లేకపోతే
  { "HeapFree",                (SYSCALL)కుప్ప_Free,                0 },
#అయితే_ముగింపు
#నిర్వచించు os_కుప్ప_Free ((BOOL(WINAPI*)(HANDLE,DWORD,LPVOID))aSyscall[23].pCurrent)

#నిర్వచితమైతే SQLITE_UWP
  { "HeapReAlloc",             (SYSCALL)0,                       0 },
#లేకపోతే
  { "HeapReAlloc",             (SYSCALL)కుప్ప_Re_Alloc,             0 },
#అయితే_ముగింపు
#నిర్వచించు os_కుప్ప_Re_Alloc ((LPVOID(WINAPI*)(HANDLE,DWORD,LPVOID, \
        కొలత_T))aSyscall[24].pCurrent)

#నిర్వచితమైతే SQLITE_UWP
  { "HeapSize",                (SYSCALL)0,                       0 },
#లేకపోతే
  { "HeapSize",                (SYSCALL)కుప్ప_కొలత,                0 },
#అయితే_ముగింపు
#నిర్వచించు os_కుప్ప_కొలత ((కొలత_T(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[25].pCurrent)

#నిర్వచితమైతే SQLITE_UWP
  { "HeapValidate",            (SYSCALL)0,                       0 },
#లేకపోతే
  { "HeapValidate",            (SYSCALL)కుప్ప_Validate,            0 },
#అయితే_ముగింపు
#నిర్వచించు os_కుప్ప_Validate ((BOOL(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[26].pCurrent)

#నిర్వచితమైతే SQLITE_UWP
  { "HeapCompact",             (SYSCALL)0,                       0 },
#లేకపోతే
  { "HeapCompact",             (SYSCALL)కుప్ప_Compact,             0 },
#అయితే_ముగింపు
#నిర్వచించు os_కుప్ప_Compact ((UINT(WINAPI*)(HANDLE,DWORD))aSyscall[27].pCurrent)


#ఒకవేళ !defined(SQLITE_OMIT_ఎక్కించు_EXTENSION) && !defined(SQLITE_UWP)
  { "LoadLibraryW",            (SYSCALL)ఎక్కించు_Library_W,            0 },
#లేకపోతే
  { "LoadLibraryW",            (SYSCALL)0,                       0 },
#అయితే_ముగింపు
#నిర్వచించు os_ఎక్కించు_Library_W ((HMODULE(WINAPI*)(LPCWSTR))aSyscall[28].pCurrent)

  { "LocalFree",               (SYSCALL)LocalFree,               0 },
#నిర్వచించు osLocalFree ((HLOCAL(WINAPI*)(HLOCAL))aSyscall[29].pCurrent)

  { "LockFileEx",              (SYSCALL)తాళం_File_Ex,              0 },
#నిర్వచించు os_తాళం_File_Ex ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[30].pCurrent)

#ఒకవేళ (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_కొలత>0) \
 && !defined(SQLITE_UWP)
  { "MapViewOfFile",           (SYSCALL)Map_View_యొక్క_File,           0 },
#లేకపోతే
  { "MapViewOfFile",           (SYSCALL)0,                       0 },
#అయితే_ముగింపు
#నిర్వచించు os_Map_View_యొక్క_File ((LPVOID(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        కొలత_T))aSyscall[31].pCurrent)

#ఒకవేళ (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_కొలత>0) \
 && defined(SQLITE_UWP)
  { "MapViewOfFileFromApp",    (SYSCALL)Map_View_యొక్క_File_From_App,   0 },
#లేకపోతే
  { "MapViewOfFileFromApp",    (SYSCALL)0,                      0 },
#అయితే_ముగింపు
#నిర్వచించు os_Map_View_యొక్క_File_From_App ((LPVOID(WINAPI*)(HANDLE,ULONG,ULONG64, \
        కొలత_T))aSyscall[32].pCurrent)

  { "MultiByteToWideChar",     (SYSCALL)Multi_Byte_గమ్యం_Wide_Char,     0 },
#నిర్వచించు os_Multi_Byte_గమ్యం_Wide_Char ((పూర్ణాంకం(WINAPI*)(UINT,DWORD,LPCSTR,పూర్ణాంకం,LPWSTR, \
        పూర్ణాంకం))aSyscall[33].pCurrent)

  { "QueryPerformanceCounter", (SYSCALL)QueryPerformanceCounter, 0 },
#నిర్వచించు osQueryPerformanceCounter ((BOOL(WINAPI*)( \
        LARGE_INTEGER*))aSyscall[34].pCurrent)

  { "ReadFile",                (SYSCALL)ReadFile,                0 },
#నిర్వచించు osReadFile ((BOOL(WINAPI*)(HANDLE,LPVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[35].pCurrent)

  { "SetEndOfFile",            (SYSCALL)సమితి_చివర_యొక్క_File,            0 },
#నిర్వచించు os_సమితి_చివర_యొక్క_File ((BOOL(WINAPI*)(HANDLE))aSyscall[36].pCurrent)

  { "SetFilePointerEx",        (SYSCALL)సమితి_File_సూచిక_Ex,        0 },
#నిర్వచించు os_సమితి_File_సూచిక_Ex ((BOOL(WINAPI*)(HANDLE,LARGE_INTEGER,\
        PLARGE_INTEGER,DWORD))aSyscall[37].pCurrent)

  { "Sleep",                   (SYSCALL)Sleep,                   0 },
#నిర్వచించు osSleep ((VOID(WINAPI*)(DWORD))aSyscall[38].pCurrent)

  { "UnlockFileEx",            (SYSCALL)UnlockFileEx,            0 },
#నిర్వచించు osUnlockFileEx ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[39].pCurrent)

#ఒకవేళ !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_కొలత>0
  { "UnmapViewOfFile",         (SYSCALL)Unmap_View_యొక్క_File,         0 },
#లేకపోతే
  { "UnmapViewOfFile",         (SYSCALL)0,                       0 },
#అయితే_ముగింపు
#నిర్వచించు os_Unmap_View_యొక్క_File ((BOOL(WINAPI*)(LPCVOID))aSyscall[40].pCurrent)

  { "WideCharToMultiByte",     (SYSCALL)Wide_Char_గమ్యం_Multi_Byte,     0 },
#నిర్వచించు os_Wide_Char_గమ్యం_Multi_Byte ((పూర్ణాంకం(WINAPI*)(UINT,DWORD,LPCWSTR,పూర్ణాంకం,LPSTR,పూర్ణాంకం, \
        LPCSTR,LPBOOL))aSyscall[41].pCurrent)

  { "WriteFile",               (SYSCALL)WriteFile,               0 },
#నిర్వచించు osWriteFile ((BOOL(WINAPI*)(HANDLE,LPCVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[42].pCurrent)

  { "WaitForSingleObject",     (SYSCALL)WaitForSingleObject,     0 },
#నిర్వచించు osWaitForSingleObject ((DWORD(WINAPI*)(HANDLE, \
        DWORD))aSyscall[43].pCurrent)

  { "WaitForSingleObjectEx",   (SYSCALL)WaitForSingleObjectEx,   0 },
#నిర్వచించు osWaitForSingleObjectEx ((DWORD(WINAPI*)(HANDLE,DWORD, \
        BOOL))aSyscall[44].pCurrent)

  { "OutputDebugStringA",      (SYSCALL)OutputDebugStringA,      0 },
#నిర్వచించు osOutputDebugStringA ((VOID(WINAPI*)(LPCSTR))aSyscall[45].pCurrent)

  { "GetProcessHeap",          (SYSCALL)Get_Process_కుప్ప,          0 },
#నిర్వచించు os_Get_Process_కుప్ప ((HANDLE(WINAPI*)(VOID))aSyscall[46].pCurrent)

/*
** NOTE: On some sub-platforms, the InterlockedCompareExchange "function"
**       is really just a macro that uses a compiler intrinsic (e.g. x64).
**       So do not try to make this is into a redefinable interface.
*/
#ఒకవేళ defined(InterlockedCompareExchange)
  { "InterlockedCompareExchange", (SYSCALL)0,                    0 },
#నిర్వచించు osInterlockedCompareExchange InterlockedCompareExchange
#లేకపోతే
  { "InterlockedCompareExchange", (SYSCALL)InterlockedCompareExchange, 0 },
#నిర్వచించు osInterlockedCompareExchange ((LONG(WINAPI*)(LONG అస్థిర*,\
        LONG,LONG))aSyscall[47].pCurrent)
#అయితే_ముగింపు /* defined(InterlockedCompareExchange) */

#ఒకవేళ SQLITE_WIN32_USE_UUID
  { "UuidCreate",               (SYSCALL)UuidCreate,             0 },
#లేకపోతే
  { "UuidCreate",               (SYSCALL)0,                      0 },
#అయితే_ముగింపు
#నిర్వచించు osUuidCreate ((RPC_స్థితి(RPC_నమోదు*)(UUID*))aSyscall[48].pCurrent)

#ఒకవేళ SQLITE_WIN32_USE_UUID
  { "UuidCreateSequential",     (SYSCALL)UuidCreateSequential,   0 },
#లేకపోతే
  { "UuidCreateSequential",     (SYSCALL)0,                      0 },
#అయితే_ముగింపు
#నిర్వచించు osUuidCreateSequential \
        ((RPC_స్థితి(RPC_నమోదు*)(UUID*))aSyscall[49].pCurrent)

#ఒకవేళ !defined(SQLITE_NO_SYNC) && SQLITE_MAX_MMAP_కొలత>0
  { "FlushViewOfFile",          (SYSCALL)Flush_View_యొక్క_File,        0 },
#లేకపోతే
  { "FlushViewOfFile",          (SYSCALL)0,                      0 },
#అయితే_ముగింపు
#నిర్వచించు os_Flush_View_యొక్క_File \
        ((BOOL(WINAPI*)(LPCVOID,కొలత_T))aSyscall[50].pCurrent)

#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  { "CreateEvent",              (SYSCALL)CreateEvent,            0 },
#లేకపోతే
  { "CreateEvent",              (SYSCALL)0,                      0 },
#అయితే_ముగింపు
#నిర్వచించు osCreateEvent ((HANDLE(WINAPI*)(LPSECURITY_లక్షణాలు,BOOL, \
                        BOOL,LPCSTR))aSyscall[51].pCurrent)

#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  { "CancelIo",                 (SYSCALL)CancelIo,               0 },
#లేకపోతే
  { "CancelIo",                 (SYSCALL)0,                      0 },
#అయితే_ముగింపు
#నిర్వచించు osCancelIo ((BOOL(WINAPI*)(HANDLE))aSyscall[52].pCurrent)

#నిర్వచితం_కాకపోతే _WIN32
  { "getenv",                   (SYSCALL)getenv,                 0 },
#లేకపోతే
  { "getenv",                   (SYSCALL)0,                      0 },
#అయితే_ముగింపు
#నిర్వచించు osGetenv ((స్థిరరాశి అక్షరం *(*)(స్థిరరాశి అక్షరం *))aSyscall[53].pCurrent)

#నిర్వచితం_కాకపోతే _WIN32
  { "getcwd",                   (SYSCALL)getcwd,                 0 },
#లేకపోతే
  { "getcwd",                   (SYSCALL)0,                      0 },
#అయితే_ముగింపు
#నిర్వచించు osGetcwd ((అక్షరం*(*)(అక్షరం*,size_t))aSyscall[54].pCurrent)

#నిర్వచితం_కాకపోతే _WIN32
  { "readlink",                 (SYSCALL)readlink,               0 },
#లేకపోతే
  { "readlink",                 (SYSCALL)0,                      0 },
#అయితే_ముగింపు
#నిర్వచించు osReadlink ((ssize_t(*)(స్థిరరాశి అక్షరం*,అక్షరం*,size_t))aSyscall[55].pCurrent)

#నిర్వచితం_కాకపోతే _WIN32
  { "lstat",                    (SYSCALL)lstat,                  0 },
#లేకపోతే
  { "lstat",                    (SYSCALL)0,                      0 },
#అయితే_ముగింపు
#నిర్వచించు osLstat ((పూర్ణాంకం(*)(స్థిరరాశి అక్షరం*,నిర్మాణం stat*))aSyscall[56].pCurrent)

#నిర్వచితం_కాకపోతే _WIN32
  { "__errno",                  (SYSCALL)__errno,                0 },
#లేకపోతే
  { "__errno",                  (SYSCALL)0,                      0 },
#అయితే_ముగింపు
#నిర్వచించు osErrno (*((పూర్ణాంకం*(*)(శూన్య))aSyscall[57].pCurrent)())

#నిర్వచితం_కాకపోతే _WIN32
  { "cygwin_conv_path",         (SYSCALL)cygwin_conv_మార్గం,       0 },
#లేకపోతే
  { "cygwin_conv_path",         (SYSCALL)0,                      0 },
#అయితే_ముగింపు
#నిర్వచించు os_Cygwin_conv_మార్గం ((size_t(*)(గుర్తులేని పూర్ణాంకం, \
    స్థిరరాశి శూన్య *, శూన్య *, size_t))aSyscall[58].pCurrent)

  { "GetDriveTypeW",            (SYSCALL)GetDriveTypeW,          0 },
#నిర్వచించు osGetDriveTypeW ((UINT(WINAPI*)(LPCWSTR))aSyscall[59].pCurrent)

}; /* End of the overrideable system calls */

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "win32" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
స్థిర పూర్ణాంకం win_సమితి_System_Call(
  sqlite3_vfs *p_Not_వాడిన,        /* The VFS pointer.  Not used */
  స్థిరరాశి అక్షరం *zName,            /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
){
  గుర్తులేని పూర్ణాంకం i;
  పూర్ణాంకం rc = SQLITE_NOTFOUND;

  వాడని_PARAMETER(p_Not_వాడిన);
  ఒకవేళ( zName==0 ){
    /* If no zName is given, restore all system calls to their default
    ** settings and return NULL
    */
    rc = SQLITE_OK;
    కొరకు(i=0; i<పరిమాణం(aSyscall)/పరిమాణం(aSyscall[0]); i++){
      ఒకవేళ( aSyscall[i].pDefault ){
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  }లేకపోతే{
    /* If zName is specified, operate on only the one system call
    ** specified.
    */
    కొరకు(i=0; i<పరిమాణం(aSyscall)/పరిమాణం(aSyscall[0]); i++){
      ఒకవేళ( strcmp(zName, aSyscall[i].zName)==0 ){
        ఒకవేళ( aSyscall[i].pDefault==0 ){
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = SQLITE_OK;
        ఒకవేళ( pNewFunc==0 ) pNewFunc = aSyscall[i].pDefault;
        aSyscall[i].pCurrent = pNewFunc;
        ఆపు;
      }
    }
  }
  తిరిగి rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
స్థిర sqlite3_syscall_ptr winGetSystemCall(
  sqlite3_vfs *p_Not_వాడిన,
  స్థిరరాశి అక్షరం *zName
){
  గుర్తులేని పూర్ణాంకం i;

  వాడని_PARAMETER(p_Not_వాడిన);
  కొరకు(i=0; i<పరిమాణం(aSyscall)/పరిమాణం(aSyscall[0]); i++){
    ఒకవేళ( strcmp(zName, aSyscall[i].zName)==0 ) తిరిగి aSyscall[i].pCurrent;
  }
  తిరిగి 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
స్థిర స్థిరరాశి అక్షరం *win_తదుపరి_System_Call(sqlite3_vfs *p, స్థిరరాశి అక్షరం *zName){
  పూర్ణాంకం i = -1;

  వాడని_PARAMETER(p);
  ఒకవేళ( zName ){
    కొరకు(i=0; i<Array_కొలత(aSyscall)-1; i++){
      ఒకవేళ( strcmp(zName, aSyscall[i].zName)==0 ) ఆపు;
    }
  }
  కొరకు(i++; i<Array_కొలత(aSyscall); i++){
    ఒకవేళ( aSyscall[i].pCurrent!=0 ) తిరిగి aSyscall[i].zName;
  }
  తిరిగి 0;
}

#నిర్వచితమైతే SQLITE_WIN32_MALLOC
#నిర్వచితమైతే SQLITE_UWP
# error SQLITE_WIN32_MALLOC is incompatible with SQLITE_UWP
#అయితే_ముగింపు
/*
** If a Win32 native heap has been configured, this function will attempt to
** compact it.  Upon success, SQLITE_OK will be returned.  Upon failure, one
** of SQLITE_NOMEM, SQLITE_ERROR, or SQLITE_NOTFOUND will be returned.  The
** "pnLargest" argument, if non-zero, will be used to return the size of the
** largest committed free block in the heap, in bytes.
*/
పూర్ణాంకం sqlite3_win32_compact_కుప్ప(LPUINT pn_అతిపెద్ద){
  పూర్ణాంకం rc = SQLITE_OK;
  UINT n_అతిపెద్ద = 0;
  HANDLE h_కుప్ప;

  winMemAssertMagic();
  h_కుప్ప = win_Mem_Get_కుప్ప();
  ప్రతిజ్ఞా( h_కుప్ప!=0 );
  ప్రతిజ్ఞా( h_కుప్ప!=INVALID_HANDLE_విలువ );
#ఒకవేళ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ప్రతిజ్ఞా( os_కుప్ప_Validate(h_కుప్ప, SQLITE_WIN32_కుప్ప_FLAGS, ఖాళీ) );
#అయితే_ముగింపు
  ఒకవేళ( (n_అతిపెద్ద=os_కుప్ప_Compact(h_కుప్ప, SQLITE_WIN32_కుప్ప_FLAGS))==0 ){
    DWORD చివరి_Errno = os_Get_చివరి_Error();
    ఒకవేళ( చివరి_Errno==NO_ERROR ){
      sqlite3_log(SQLITE_NOMEM, "failed to HeapCompact (no space), heap=%p",
                  (శూన్య*)h_కుప్ప);
      rc = SQLITE_NOMEM_BKPT;
    }లేకపోతే{
      sqlite3_log(SQLITE_ERROR, "failed to HeapCompact (%lu), heap=%p",
                  os_Get_చివరి_Error(), (శూన్య*)h_కుప్ప);
      rc = SQLITE_ERROR;
    }
  }
  ఒకవేళ( pn_అతిపెద్ద ) *pn_అతిపెద్ద = n_అతిపెద్ద;
  తిరిగి rc;
}

/*
** If a Win32 native heap has been configured, this function will attempt to
** destroy and recreate it.  If the Win32 native heap is not isolated and/or
** the sqlite3_memory_used() function does not return zero, SQLITE_BUSY will
** be returned and no changes will be made to the Win32 native heap.
*/
పూర్ణాంకం sqlite3_win32_reset_కుప్ప(){
  పూర్ణాంకం rc;
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; ) /* The main static mutex */
  MUTEX_LOGIC( sqlite3_mutex *pMem; )    /* The memsys static mutex */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  MUTEX_LOGIC( pMem = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3_mutex_enter(pMem);
  winMemAssertMagic();
  ఒకవేళ( win_Mem_Get_కుప్ప()!=ఖాళీ && winMemGetOwned() && sqlite3_జ్ఞాపకం_వాడిన()==0 ){
    /*
    ** At this point, there should be no outstanding memory allocations on
    ** the heap.  Also, since both the main and memsys locks are currently
    ** being held by us, no other function (i.e. from another thread) should
    ** be able to even access the heap.  Attempt to destroy and recreate our
    ** isolated Win32 native heap now.
    */
    ప్రతిజ్ఞా( win_Mem_Get_కుప్ప()!=ఖాళీ );
    ప్రతిజ్ఞా( winMemGetOwned() );
    ప్రతిజ్ఞా( sqlite3_జ్ఞాపకం_వాడిన()==0 );
    winMemShutdown(win_Mem_Get_దత్తాంశం_Ptr());
    ప్రతిజ్ఞా( win_Mem_Get_కుప్ప()==ఖాళీ );
    ప్రతిజ్ఞా( !winMemGetOwned() );
    ప్రతిజ్ఞా( sqlite3_జ్ఞాపకం_వాడిన()==0 );
    rc = winMemInit(win_Mem_Get_దత్తాంశం_Ptr());
    ప్రతిజ్ఞా( rc!=SQLITE_OK || win_Mem_Get_కుప్ప()!=ఖాళీ );
    ప్రతిజ్ఞా( rc!=SQLITE_OK || winMemGetOwned() );
    ప్రతిజ్ఞా( rc!=SQLITE_OK || sqlite3_జ్ఞాపకం_వాడిన()==0 );
  }లేకపోతే{
    /*
    ** The Win32 native heap cannot be modified because it may be in use.
    */
    rc = SQLITE_BUSY;
  }
  sqlite3_mutex_leave(pMem);
  sqlite3_mutex_leave(pMainMtx);
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_WIN32_MALLOC */

#నిర్వచితమైతే _WIN32
/*
** This function outputs the specified (ANSI) string to the Win32 debugger
** (if available).  Undocumented.  Might go away at any moment.
*/
శూన్య sqlite3_win32_write_debug(స్థిరరాశి అక్షరం *zBuf, పూర్ణాంకం nBuf){
  అక్షరం zDbgBuf[SQLITE_WIN32_DBG_BUF_కొలత];
  పూర్ణాంకం nMin = MIN(nBuf, (SQLITE_WIN32_DBG_BUF_కొలత - 1)); /* may be negative. */
  ఒకవేళ( nMin<-1 ) nMin = -1; /* all negative values become -1. */
  ప్రతిజ్ఞా( nMin==-1 || nMin==0 || nMin<SQLITE_WIN32_DBG_BUF_కొలత );
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !zBuf ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి;
  }
#అయితే_ముగింపు
  ఒకవేళ( nMin>0 ){
    memset(zDbgBuf, 0, SQLITE_WIN32_DBG_BUF_కొలత);
    జ్ఞాపకం_నకలు(zDbgBuf, zBuf, nMin);
    osOutputDebugStringA(zDbgBuf);
  }లేకపోతే{
    osOutputDebugStringA(zBuf);
  }
}

శూన్య sqlite3_win32_sleep(DWORD milliseconds){
  osSleep(milliseconds);
}

#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0 && SQLITE_THREADSAFE>0
DWORD sqlite3Win32Wait(HANDLE hObject){
  DWORD rc;
  అయితే( (rc = osWaitForSingleObjectEx(hObject, INFINITE,
                                       TRUE))==WAIT_IO_COMPLETION ){}
  తిరిగి rc;
}
#అయితే_ముగింపు
#అయితే_ముగింపు /* _WIN32 */

#నిర్వచితమైతే SQLITE_WIN32_MALLOC
/*
** Allocate nBytes of memory.
*/
స్థిర శూన్య *winMemMalloc(పూర్ణాంకం n_బైట్లు){
  HANDLE h_కుప్ప;
  శూన్య *p;

  winMemAssertMagic();
  h_కుప్ప = win_Mem_Get_కుప్ప();
  ప్రతిజ్ఞా( h_కుప్ప!=0 );
  ప్రతిజ్ఞా( h_కుప్ప!=INVALID_HANDLE_విలువ );
#ఒకవేళ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ప్రతిజ్ఞా( os_కుప్ప_Validate(h_కుప్ప, SQLITE_WIN32_కుప్ప_FLAGS, ఖాళీ) );
#అయితే_ముగింపు
  ప్రతిజ్ఞా( n_బైట్లు>=0 );
  p = os_కుప్ప_Alloc(h_కుప్ప, SQLITE_WIN32_కుప్ప_FLAGS, (కొలత_T)n_బైట్లు);
  ఒకవేళ( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapAlloc %u bytes (%lu), heap=%p",
                n_బైట్లు, os_Get_చివరి_Error(), (శూన్య*)h_కుప్ప);
  }
  తిరిగి p;
}

/*
** Free memory.
*/
స్థిర శూన్య winMemFree(శూన్య *p_పూర్వ){
  HANDLE h_కుప్ప;

  winMemAssertMagic();
  h_కుప్ప = win_Mem_Get_కుప్ప();
  ప్రతిజ్ఞా( h_కుప్ప!=0 );
  ప్రతిజ్ఞా( h_కుప్ప!=INVALID_HANDLE_విలువ );
#ఒకవేళ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ప్రతిజ్ఞా( os_కుప్ప_Validate(h_కుప్ప, SQLITE_WIN32_కుప్ప_FLAGS, p_పూర్వ) );
#అయితే_ముగింపు
  ఒకవేళ( !p_పూర్వ ) తిరిగి; /* Passing NULL to HeapFree is undefined. */
  ఒకవేళ( !os_కుప్ప_Free(h_కుప్ప, SQLITE_WIN32_కుప్ప_FLAGS, p_పూర్వ) ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapFree block %p (%lu), heap=%p",
                p_పూర్వ, os_Get_చివరి_Error(), (శూన్య*)h_కుప్ప);
  }
}

/*
** Change the size of an existing memory allocation
*/
స్థిర శూన్య *winMemRealloc(శూన్య *p_పూర్వ, పూర్ణాంకం n_బైట్లు){
  HANDLE h_కుప్ప;
  శూన్య *p;

  winMemAssertMagic();
  h_కుప్ప = win_Mem_Get_కుప్ప();
  ప్రతిజ్ఞా( h_కుప్ప!=0 );
  ప్రతిజ్ఞా( h_కుప్ప!=INVALID_HANDLE_విలువ );
#ఒకవేళ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ప్రతిజ్ఞా( os_కుప్ప_Validate(h_కుప్ప, SQLITE_WIN32_కుప్ప_FLAGS, p_పూర్వ) );
#అయితే_ముగింపు
  ప్రతిజ్ఞా( n_బైట్లు>=0 );
  ఒకవేళ( !p_పూర్వ ){
    p = os_కుప్ప_Alloc(h_కుప్ప, SQLITE_WIN32_కుప్ప_FLAGS, (కొలత_T)n_బైట్లు);
  }లేకపోతే{
    p = os_కుప్ప_Re_Alloc(h_కుప్ప, SQLITE_WIN32_కుప్ప_FLAGS, p_పూర్వ, (కొలత_T)n_బైట్లు);
  }
  ఒకవేళ( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to %s %u bytes (%lu), heap=%p",
                p_పూర్వ ? "HeapReAlloc" : "HeapAlloc", n_బైట్లు, os_Get_చివరి_Error(),
                (శూన్య*)h_కుప్ప);
  }
  తిరిగి p;
}

/*
** Return the size of an outstanding allocation, in bytes.
*/
స్థిర పూర్ణాంకం win_Mem_కొలత(శూన్య *p){
  HANDLE h_కుప్ప;
  కొలత_T n;

  winMemAssertMagic();
  h_కుప్ప = win_Mem_Get_కుప్ప();
  ప్రతిజ్ఞా( h_కుప్ప!=0 );
  ప్రతిజ్ఞా( h_కుప్ప!=INVALID_HANDLE_విలువ );
#ఒకవేళ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ప్రతిజ్ఞా( os_కుప్ప_Validate(h_కుప్ప, SQLITE_WIN32_కుప్ప_FLAGS, p) );
#అయితే_ముగింపు
  ఒకవేళ( !p ) తిరిగి 0;
  n = os_కుప్ప_కొలత(h_కుప్ప, SQLITE_WIN32_కుప్ప_FLAGS, p);
  ఒకవేళ( n==(కొలత_T)-1 ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapSize block %p (%lu), heap=%p",
                p, os_Get_చివరి_Error(), (శూన్య*)h_కుప్ప);
    తిరిగి 0;
  }
  తిరిగి (పూర్ణాంకం)n;
}

/*
** Round up a request size to the next valid allocation size.
*/
స్థిర పూర్ణాంకం winMemRoundup(పూర్ణాంకం n){
  తిరిగి n;
}

/*
** Initialize this module.
*/
స్థిర పూర్ణాంకం winMemInit(శూన్య *p_App_దత్తాంశం){
  win_Mem_దత్తాంశం *p_Win_Mem_దత్తాంశం = (win_Mem_దత్తాంశం *)p_App_దత్తాంశం;

  ఒకవేళ( !p_Win_Mem_దత్తాంశం ) తిరిగి SQLITE_ERROR;
  ప్రతిజ్ఞా( p_Win_Mem_దత్తాంశం->magic1==WINMEM_MAGIC1 );
  ప్రతిజ్ఞా( p_Win_Mem_దత్తాంశం->magic2==WINMEM_MAGIC2 );

#ఒకవేళ SQLITE_WIN32_కుప్ప_CREATE
  ఒకవేళ( !p_Win_Mem_దత్తాంశం->h_కుప్ప ){
    DWORD dw_Initial_కొలత = SQLITE_WIN32_కుప్ప_INIT_కొలత;
    DWORD dw_గరిష్ఠ_కొలత = (DWORD)sqlite3GlobalConfig.n_కుప్ప;
    ఒకవేళ( dw_గరిష్ఠ_కొలత==0 ){
      dw_గరిష్ఠ_కొలత = SQLITE_WIN32_కుప్ప_MAX_కొలత;
    }లేకపోతే ఒకవేళ( dw_Initial_కొలత>dw_గరిష్ఠ_కొలత ){
      dw_Initial_కొలత = dw_గరిష్ఠ_కొలత;
    }
    p_Win_Mem_దత్తాంశం->h_కుప్ప = os_కుప్ప_Create(SQLITE_WIN32_కుప్ప_FLAGS,
                                      dw_Initial_కొలత, dw_గరిష్ఠ_కొలత);
    ఒకవేళ( !p_Win_Mem_దత్తాంశం->h_కుప్ప ){
      sqlite3_log(SQLITE_NOMEM,
          "failed to HeapCreate (%lu), flags=%u, initSize=%lu, maxSize=%lu",
          os_Get_చివరి_Error(), SQLITE_WIN32_కుప్ప_FLAGS, dw_Initial_కొలత,
          dw_గరిష్ఠ_కొలత);
      తిరిగి SQLITE_NOMEM_BKPT;
    }
    p_Win_Mem_దత్తాంశం->bOwned = TRUE;
    ప్రతిజ్ఞా( p_Win_Mem_దత్తాంశం->bOwned );
  }
#లేకపోతే
  p_Win_Mem_దత్తాంశం->h_కుప్ప = os_Get_Process_కుప్ప();
  ఒకవేళ( !p_Win_Mem_దత్తాంశం->h_కుప్ప ){
    sqlite3_log(SQLITE_NOMEM,
        "failed to GetProcessHeap (%lu)", os_Get_చివరి_Error());
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  p_Win_Mem_దత్తాంశం->bOwned = FALSE;
  ప్రతిజ్ఞా( !p_Win_Mem_దత్తాంశం->bOwned );
#అయితే_ముగింపు
  ప్రతిజ్ఞా( p_Win_Mem_దత్తాంశం->h_కుప్ప!=0 );
  ప్రతిజ్ఞా( p_Win_Mem_దత్తాంశం->h_కుప్ప!=INVALID_HANDLE_విలువ );
#ఒకవేళ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ప్రతిజ్ఞా( os_కుప్ప_Validate(p_Win_Mem_దత్తాంశం->h_కుప్ప, SQLITE_WIN32_కుప్ప_FLAGS, ఖాళీ) );
#అయితే_ముగింపు
  తిరిగి SQLITE_OK;
}

/*
** Deinitialize this module.
*/
స్థిర శూన్య winMemShutdown(శూన్య *p_App_దత్తాంశం){
  win_Mem_దత్తాంశం *p_Win_Mem_దత్తాంశం = (win_Mem_దత్తాంశం *)p_App_దత్తాంశం;

  ఒకవేళ( !p_Win_Mem_దత్తాంశం ) తిరిగి;
  ప్రతిజ్ఞా( p_Win_Mem_దత్తాంశం->magic1==WINMEM_MAGIC1 );
  ప్రతిజ్ఞా( p_Win_Mem_దత్తాంశం->magic2==WINMEM_MAGIC2 );

  ఒకవేళ( p_Win_Mem_దత్తాంశం->h_కుప్ప ){
    ప్రతిజ్ఞా( p_Win_Mem_దత్తాంశం->h_కుప్ప!=INVALID_HANDLE_విలువ );
#ఒకవేళ defined(SQLITE_WIN32_MALLOC_VALIDATE)
    ప్రతిజ్ఞా( os_కుప్ప_Validate(p_Win_Mem_దత్తాంశం->h_కుప్ప, SQLITE_WIN32_కుప్ప_FLAGS, ఖాళీ) );
#అయితే_ముగింపు
    ఒకవేళ( p_Win_Mem_దత్తాంశం->bOwned ){
      ఒకవేళ( !os_కుప్ప_Destroy(p_Win_Mem_దత్తాంశం->h_కుప్ప) ){
        sqlite3_log(SQLITE_NOMEM, "failed to HeapDestroy (%lu), heap=%p",
                    os_Get_చివరి_Error(), (శూన్య*)p_Win_Mem_దత్తాంశం->h_కుప్ప);
      }
      p_Win_Mem_దత్తాంశం->bOwned = FALSE;
    }
    p_Win_Mem_దత్తాంశం->h_కుప్ప = ఖాళీ;
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
స్థిరరాశి sqlite3_mem_methods *sqlite3MemGetWin32(శూన్య){
  స్థిర స్థిరరాశి sqlite3_mem_methods winMemMethods = {
    winMemMalloc,
    winMemFree,
    winMemRealloc,
    win_Mem_కొలత,
    winMemRoundup,
    winMemInit,
    winMemShutdown,
    &win_mem_దత్తాంశం
  };
  తిరిగి &winMemMethods;
}

శూన్య sqlite3_Mem_సమితి_Default(శూన్య){
  sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetWin32());
}
#అయితే_ముగింపు /* SQLITE_WIN32_MALLOC */

#నిర్వచితమైతే _WIN32
/*
** Convert a UTF-8 string to Microsoft Unicode.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
స్థిర LPWSTR win_Utf8_గమ్యం_Unicode(స్థిరరాశి అక్షరం *z_పాఠ్యం){
  పూర్ణాంకం nChar;
  LPWSTR z_Wide_పాఠ్యం;

  nChar = os_Multi_Byte_గమ్యం_Wide_Char(CP_UTF8, 0, z_పాఠ్యం, -1, ఖాళీ, 0);
  ఒకవేళ( nChar==0 ){
    తిరిగి 0;
  }
  z_Wide_పాఠ్యం = sqlite3MallocZero( nChar*పరిమాణం(WCHAR) );
  ఒకవేళ( z_Wide_పాఠ్యం==0 ){
    తిరిగి 0;
  }
  nChar = os_Multi_Byte_గమ్యం_Wide_Char(CP_UTF8, 0, z_పాఠ్యం, -1, z_Wide_పాఠ్యం,
                                nChar);
  ఒకవేళ( nChar==0 ){
    sqlite3_free(z_Wide_పాఠ్యం);
    z_Wide_పాఠ్యం = 0;
  }
  తిరిగి z_Wide_పాఠ్యం;
}
#అయితే_ముగింపు /* _WIN32 */

/*
** Convert a Microsoft Unicode string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
స్థిర అక్షరం *win_Unicode_గమ్యం_Utf8(LPCWSTR z_Wide_పాఠ్యం){
  పూర్ణాంకం nByte;
  అక్షరం *z_పాఠ్యం;

  nByte = os_Wide_Char_గమ్యం_Multi_Byte(CP_UTF8, 0, z_Wide_పాఠ్యం, -1, 0, 0, 0, 0);
  ఒకవేళ( nByte == 0 ){
    తిరిగి 0;
  }
  z_పాఠ్యం = sqlite3MallocZero( nByte );
  ఒకవేళ( z_పాఠ్యం==0 ){
    తిరిగి 0;
  }
  nByte = os_Wide_Char_గమ్యం_Multi_Byte(CP_UTF8, 0, z_Wide_పాఠ్యం, -1, z_పాఠ్యం, nByte,
                                0, 0);
  ఒకవేళ( nByte == 0 ){
    sqlite3_free(z_పాఠ్యం);
    z_పాఠ్యం = 0;
  }
  తిరిగి z_పాఠ్యం;
}

#నిర్వచితమైతే _WIN32
/*
** Convert an ANSI string to Microsoft Unicode, using the ANSI or OEM
** code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
స్థిర LPWSTR win_Mbcs_గమ్యం_Unicode(స్థిరరాశి అక్షరం *z_పాఠ్యం, పూర్ణాంకం useAnsi){
  పూర్ణాంకం nWideChar;
  LPWSTR z_Mbcs_పాఠ్యం;
  పూర్ణాంకం codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nWideChar = os_Multi_Byte_గమ్యం_Wide_Char(codepage, 0, z_పాఠ్యం, -1, ఖాళీ,
                                0);
  ఒకవేళ( nWideChar==0 ){
    తిరిగి 0;
  }
  z_Mbcs_పాఠ్యం = sqlite3MallocZero( nWideChar*పరిమాణం(WCHAR) );
  ఒకవేళ( z_Mbcs_పాఠ్యం==0 ){
    తిరిగి 0;
  }
  nWideChar = os_Multi_Byte_గమ్యం_Wide_Char(codepage, 0, z_పాఠ్యం, -1, z_Mbcs_పాఠ్యం,
                                nWideChar);
  ఒకవేళ( nWideChar==0 ){
    sqlite3_free(z_Mbcs_పాఠ్యం);
    z_Mbcs_పాఠ్యం = 0;
  }
  తిరిగి z_Mbcs_పాఠ్యం;
}

/*
** Convert a Microsoft Unicode string to a multi-byte character string,
** using the ANSI or OEM code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
స్థిర అక్షరం *win_Unicode_గమ్యం_Mbcs(LPCWSTR z_Wide_పాఠ్యం, పూర్ణాంకం useAnsi){
  పూర్ణాంకం nByte;
  అక్షరం *z_పాఠ్యం;
  పూర్ణాంకం codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nByte = os_Wide_Char_గమ్యం_Multi_Byte(codepage, 0, z_Wide_పాఠ్యం, -1, 0, 0, 0, 0);
  ఒకవేళ( nByte == 0 ){
    తిరిగి 0;
  }
  z_పాఠ్యం = sqlite3MallocZero( nByte );
  ఒకవేళ( z_పాఠ్యం==0 ){
    తిరిగి 0;
  }
  nByte = os_Wide_Char_గమ్యం_Multi_Byte(codepage, 0, z_Wide_పాఠ్యం, -1, z_పాఠ్యం,
                                nByte, 0, 0);
  ఒకవేళ( nByte == 0 ){
    sqlite3_free(z_పాఠ్యం);
    z_పాఠ్యం = 0;
  }
  తిరిగి z_పాఠ్యం;
}

/*
** Convert a multi-byte character string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
స్థిర అక్షరం *win_Mbcs_గమ్యం_Utf8(స్థిరరాశి అక్షరం *z_పాఠ్యం, పూర్ణాంకం useAnsi){
  అక్షరం *z_పాఠ్యం_Utf8;
  LPWSTR zTmpWide;

  zTmpWide = win_Mbcs_గమ్యం_Unicode(z_పాఠ్యం, useAnsi);
  ఒకవేళ( zTmpWide==0 ){
    తిరిగి 0;
  }
  z_పాఠ్యం_Utf8 = win_Unicode_గమ్యం_Utf8(zTmpWide);
  sqlite3_free(zTmpWide);
  తిరిగి z_పాఠ్యం_Utf8;
}

/*
** Convert a UTF-8 string to a multi-byte character string.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
స్థిర అక్షరం *win_Utf8_గమ్యం_Mbcs(స్థిరరాశి అక్షరం *z_పాఠ్యం, పూర్ణాంకం useAnsi){
  అక్షరం *z_పాఠ్యం_Mbcs;
  LPWSTR zTmpWide;

  zTmpWide = win_Utf8_గమ్యం_Unicode(z_పాఠ్యం);
  ఒకవేళ( zTmpWide==0 ){
    తిరిగి 0;
  }
  z_పాఠ్యం_Mbcs = win_Unicode_గమ్యం_Mbcs(zTmpWide, useAnsi);
  sqlite3_free(zTmpWide);
  తిరిగి z_పాఠ్యం_Mbcs;
}

/*
** This is a public wrapper for the winUtf8ToUnicode() function.
*/
LPWSTR sqlite3_win32_utf8_గమ్యం_unicode(స్థిరరాశి అక్షరం *z_పాఠ్యం){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !z_పాఠ్యం ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize() ) తిరిగి 0;
#అయితే_ముగింపు
  తిరిగి win_Utf8_గమ్యం_Unicode(z_పాఠ్యం);
}

/*
** This is a public wrapper for the winUnicodeToUtf8() function.
*/
అక్షరం *sqlite3_win32_unicode_గమ్యం_utf8(LPCWSTR z_Wide_పాఠ్యం){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !z_Wide_పాఠ్యం ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize() ) తిరిగి 0;
#అయితే_ముగింపు
  తిరిగి win_Unicode_గమ్యం_Utf8(z_Wide_పాఠ్యం);
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
అక్షరం *sqlite3_win32_mbcs_గమ్యం_utf8(స్థిరరాశి అక్షరం *z_పాఠ్యం){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !z_పాఠ్యం ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize() ) తిరిగి 0;
#అయితే_ముగింపు
  తిరిగి win_Mbcs_గమ్యం_Utf8(z_పాఠ్యం, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
అక్షరం *sqlite3_win32_mbcs_గమ్యం_utf8_v2(స్థిరరాశి అక్షరం *z_పాఠ్యం, పూర్ణాంకం useAnsi){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !z_పాఠ్యం ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize() ) తిరిగి 0;
#అయితే_ముగింపు
  తిరిగి win_Mbcs_గమ్యం_Utf8(z_పాఠ్యం, useAnsi);
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
అక్షరం *sqlite3_win32_utf8_గమ్యం_mbcs(స్థిరరాశి అక్షరం *z_పాఠ్యం){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !z_పాఠ్యం ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize() ) తిరిగి 0;
#అయితే_ముగింపు
  తిరిగి win_Utf8_గమ్యం_Mbcs(z_పాఠ్యం, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
అక్షరం *sqlite3_win32_utf8_గమ్యం_mbcs_v2(స్థిరరాశి అక్షరం *z_పాఠ్యం, పూర్ణాంకం useAnsi){
#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !z_పాఠ్యం ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  ఒకవేళ( sqlite3_initialize() ) తిరిగి 0;
#అయితే_ముగింపు
  తిరిగి win_Utf8_గమ్యం_Mbcs(z_పాఠ్యం, useAnsi);
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-8 string.
*/
పూర్ణాంకం sqlite3_win32_సమితి_directory8(
  గుర్తులేని దీర్ఘ type, /* Identifier for directory being set or reset */
  స్థిరరాశి అక్షరం *z_విలువ  /* New value for directory being set or reset */
){
  అక్షరం **ppDirectory = 0;
  పూర్ణాంకం rc;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ఒకవేళ( rc ) తిరిగి rc;
#అయితే_ముగింపు
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ఒకవేళ( type==SQLITE_WIN32_దత్తాంశం_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_దత్తాంశం_directory;
  }లేకపోతే ఒకవేళ( type==SQLITE_WIN32_తాత్కాలిక_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_తాత్కాలిక_directory;
  }
  ప్రతిజ్ఞా( !ppDirectory || type==SQLITE_WIN32_దత్తాంశం_DIRECTORY_TYPE
          || type==SQLITE_WIN32_తాత్కాలిక_DIRECTORY_TYPE
  );
  ప్రతిజ్ఞా( !ppDirectory || sqlite3MemdebugHasType(*ppDirectory, MEMTYPE_కుప్ప) );
  ఒకవేళ( ppDirectory ){
    అక్షరం *zCopy = 0;
    ఒకవేళ( z_విలువ && z_విలువ[0] ){
      zCopy = sqlite3_mprintf("%s", z_విలువ);
      ఒకవేళ ( zCopy==0 ){
        rc = SQLITE_NOMEM_BKPT;
        వెళ్ళు సమితి_directory8_పూర్తి;
      }
    }
    sqlite3_free(*ppDirectory);
    *ppDirectory = zCopy;
    rc = SQLITE_OK;
  }లేకపోతే{
    rc = SQLITE_ERROR;
  }
సమితి_directory8_పూర్తి:
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  తిరిగి rc;
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-16 string.
*/
పూర్ణాంకం sqlite3_win32_సమితి_directory16(
  గుర్తులేని దీర్ఘ type, /* Identifier for directory being set or reset */
  స్థిరరాశి శూన్య *z_విలువ  /* New value for directory being set or reset */
){
  పూర్ణాంకం rc;
  అక్షరం *zUtf8 = 0;
  ఒకవేళ( z_విలువ ){
    zUtf8 = sqlite3_win32_unicode_గమ్యం_utf8(z_విలువ);
    ఒకవేళ( zUtf8==0 ) తిరిగి SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_win32_సమితి_directory8(type, zUtf8);
  ఒకవేళ( zUtf8 ) sqlite3_free(zUtf8);
  తిరిగి rc;
}

/*
** This function sets the data directory or the temporary directory based on
** the provided arguments.  The type argument must be 1 in order to set the
** data directory or 2 in order to set the temporary directory.  The zValue
** argument is the name of the directory to use.  The return value will be
** SQLITE_OK if successful.
*/
పూర్ణాంకం sqlite3_win32_సమితి_directory(
  గుర్తులేని దీర్ఘ type, /* Identifier for directory being set or reset */
  శూన్య *z_విలువ        /* New value for directory being set or reset */
){
  తిరిగి sqlite3_win32_సమితి_directory16(type, z_విలువ);
}
#అయితే_ముగింపు /* _WIN32 */

/*
** The return value of winGetLastErrorMsg
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated).
*/
స్థిర పూర్ణాంకం win_Get_చివరి_Error_సందేశం(DWORD చివరి_Errno, పూర్ణాంకం nBuf, అక్షరం *zBuf){
  /* FormatMessage returns 0 on failure.  Otherwise it
  ** returns the number of TCHARs written to the output
  ** buffer, excluding the terminating null char.
  */
  DWORD dwLen = 0;
  అక్షరం *z_బహిర్గతం = 0;
  LPWSTR z_తాత్కాలిక_Wide = ఖాళీ;
  dwLen = os_Format_సందేశం_W(FORMAT_సందేశం_ALLOCATE_బఫరు |
                           FORMAT_సందేశం_FROM_SYSTEM |
                           FORMAT_సందేశం_IGNORE_INSERTS,
                           ఖాళీ,
                           చివరి_Errno,
                           0,
                           (LPWSTR) &z_తాత్కాలిక_Wide,
                           0,
                           0);
  ఒకవేళ( dwLen > 0 ){
    /* allocate a buffer and convert to UTF8 */
    sqlite3BeginBenignMalloc();
    z_బహిర్గతం = win_Unicode_గమ్యం_Utf8(z_తాత్కాలిక_Wide);
    sqlite3_చివర_Benign_Malloc();
    /* free the system buffer allocated by FormatMessage */
    osLocalFree(z_తాత్కాలిక_Wide);
  }
  ఒకవేళ( 0 == dwLen ){
    sqlite3_snprintf(nBuf, zBuf, "OsError 0x%lx (%lu)", చివరి_Errno, చివరి_Errno);
  }లేకపోతే{
    /* copy a maximum of nBuf chars to output buffer */
    sqlite3_snprintf(nBuf, zBuf, "%s", z_బహిర్గతం);
    /* free the UTF8 buffer */
    sqlite3_free(z_బహిర్గతం);
  }
  తిరిగి 0;
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
#నిర్వచించు winLogError(a,b,c,d)   win_Log_Error_At_పంక్తి(a,b,c,d,__LINE__)
స్థిర పూర్ణాంకం win_Log_Error_At_పంక్తి(
  పూర్ణాంకం errcode,                    /* SQLite error code */
  DWORD చివరి_Errno,                /* Win32 last error */
  స్థిరరాశి అక్షరం *zFunc,              /* Name of OS function that failed */
  స్థిరరాశి అక్షరం *z_మార్గం,              /* File path associated with error */
  పూర్ణాంకం i_పంక్తి                       /* Source line number where error occurred */
){
  అక్షరం z_సందేశం[500];                 /* Human readable error text */
  పూర్ణాంకం i;                          /* Loop counter */

  z_సందేశం[0] = 0;
  win_Get_చివరి_Error_సందేశం(చివరి_Errno, పరిమాణం(z_సందేశం), z_సందేశం);
  ప్రతిజ్ఞా( errcode!=SQLITE_OK );
  ఒకవేళ( z_మార్గం==0 ) z_మార్గం = "";
  కొరకు(i=0; z_సందేశం[i] && z_సందేశం[i]!='\r' && z_సందేశం[i]!='\n'; i++){}
  z_సందేశం[i] = 0;
  sqlite3_log(errcode,
      "os_win.c:%d: (%lu) %s(%s) - %s",
      i_పంక్తి, చివరి_Errno, zFunc, z_మార్గం, z_సందేశం
  );

  తిరిగి errcode;
}

/*
** The number of times that a ReadFile(), WriteFile(), and DeleteFile()
** will be retried following a locking error - probably caused by
** antivirus software.  Also the initial delay before the first retry.
** The delay increases linearly with each retry.
*/
#నిర్వచితం_కాకపోతే SQLITE_WIN32_IOERR_RETRY
# define SQLITE_WIN32_IOERR_RETRY 10
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_WIN32_IOERR_RETRY_DELAY
# define SQLITE_WIN32_IOERR_RETRY_DELAY 25
#అయితే_ముగింపు
స్థిర పూర్ణాంకం winIoerrRetry = SQLITE_WIN32_IOERR_RETRY;
స్థిర పూర్ణాంకం winIoerrRetryDelay = SQLITE_WIN32_IOERR_RETRY_DELAY;

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
#ఒకవేళ !defined(winIoerrCanRetry1)
#నిర్వచించు winIoerrCanRetry1(a) (((a)==ERROR_ACCESS_DENIED)        || \
                              ((a)==ERROR_SHARING_VIOLATION)    || \
                              ((a)==ERROR_తాళం_VIOLATION)       || \
                              ((a)==ERROR_DEV_NOT_EXIST)        || \
                              ((a)==ERROR_NETNAME_DELETED)      || \
                              ((a)==ERROR_SEM_TIMEOUT)          || \
                              ((a)==ERROR_జాలం_UNREACHABLE))
#అయితే_ముగింపు

/*
** If a ReadFile() or WriteFile() error occurs, invoke this routine
** to see if it should be retried.  Return TRUE to retry.  Return FALSE
** to give up with an error.
*/
స్థిర పూర్ణాంకం winRetryIoerr(పూర్ణాంకం *pnRetry, DWORD *pError){
  DWORD e = os_Get_చివరి_Error();
  ఒకవేళ( *pnRetry>=winIoerrRetry ){
    ఒకవేళ( pError ){
      *pError = e;
    }
    తిరిగి 0;
  }
  ఒకవేళ( winIoerrCanRetry1(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    తిరిగి 1;
  }
#ఒకవేళ defined(winIoerrCanRetry2)
  లేకపోతే ఒకవేళ( winIoerrCanRetry2(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    తిరిగి 1;
  }
#అయితే_ముగింపు
  ఒకవేళ( pError ){
    *pError = e;
  }
  తిరిగి 0;
}

/*
** Log a I/O error retry episode.
*/
స్థిర శూన్య winLogIoerr(పూర్ణాంకం nRetry, పూర్ణాంకం lineno){
  ఒకవేళ( nRetry ){
    sqlite3_log(SQLITE_NOTICE,
      "delayed %dms for lock/sharing conflict at line %d",
      winIoerrRetryDelay*nRetry*(nRetry+1)/2, lineno
    );
  }
}


/*
** Lock a file region.
*/
స్థిర BOOL win_తాళం_File(
  LPHANDLE phFile,
  DWORD flags,
  DWORD స్థానభ్రంశం_దిగువ,
  DWORD స్థానభ్రంశం_ఎగువ,
  DWORD అంకె_బైట్లు_దిగువ,
  DWORD అంకె_బైట్లు_ఎగువ
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, పరిమాణం(OVERLAPPED));
  ovlp.స్థానభ్రంశం_2 = స్థానభ్రంశం_దిగువ;
  ovlp.స్థానభ్రంశం_ఎగువ_2 = స్థానభ్రంశం_ఎగువ;
  తిరిగి os_తాళం_File_Ex(*phFile, flags, 0, అంకె_బైట్లు_దిగువ, అంకె_బైట్లు_ఎగువ, &ovlp);
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
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
స్థిర పూర్ణాంకం win_Handle_తాళం_Timeout(
  HANDLE hFile,
  DWORD స్థానభ్రంశం,
  DWORD nByte,
  పూర్ణాంకం bExcl,
  DWORD nMs
){
  DWORD flags = LOCKFILE_FAIL_IMMEDIATELY | (bExcl?LOCKFILE_EXCLUSIVE_తాళం:0);
  పూర్ణాంకం rc = SQLITE_OK;
  BOOL ret;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, పరిమాణం(OVERLAPPED));
  ovlp.స్థానభ్రంశం_2 = స్థానభ్రంశం;

#ఒకవేళ defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  ఒకవేళ( nMs!=0 ){
    flags &= ~LOCKFILE_FAIL_IMMEDIATELY;
  }
  ovlp.hEvent = osCreateEvent(ఖాళీ, TRUE, FALSE, ఖాళీ);
  ఒకవేళ( ovlp.hEvent==ఖాళీ ){
    తిరిగి SQLITE_IOERR_తాళం;
  }
#అయితే_ముగింపు
  ret = os_తాళం_File_Ex(hFile, flags, 0, nByte, 0, &ovlp);
#ఒకవేళ defined(SQLITE_ENABLE_SETLK_TIMEOUT)
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
  ఒకవేళ( !ret && Get_చివరి_Error()==ERROR_IO_PENDING ){
    DWORD nDelay = (nMs==0 ? INFINITE : nMs);
    DWORD res = osWaitForSingleObject(ovlp.hEvent, nDelay);
    ఒకవేళ( res==WAIT_OBJECT_0 ){
      ret = TRUE;
    }లేకపోతే ఒకవేళ( res==WAIT_TIMEOUT ){
#ఒకవేళ SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = SQLITE_BUSY_TIMEOUT;
#లేకపోతే
      rc = SQLITE_BUSY;
#అయితే_ముగింపు
    }లేకపోతే{
      /* Some other error has occurred */
      rc = SQLITE_IOERR_తాళం;
    }

    /* If it is still pending, cancel the LockFileEx() call. */
    osCancelIo(hFile);
  }

  osCloseHandle(ovlp.hEvent);
#అయితే_ముగింపు /* defined(SQLITE_ENABLE_SETLK_TIMEOUT) */

  ఒకవేళ( rc==SQLITE_OK && !ret ){
    rc = SQLITE_BUSY;
  }
  తిరిగి rc;
}
#అయితే_ముగింపు /* #ifndef SQLITE_OMIT_WAL */

/*
** Unlock a file region.
 */
స్థిర BOOL winUnlockFile(
  LPHANDLE phFile,
  DWORD స్థానభ్రంశం_దిగువ,
  DWORD స్థానభ్రంశం_ఎగువ,
  DWORD అంకె_బైట్లు_దిగువ,
  DWORD అంకె_బైట్లు_ఎగువ
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, పరిమాణం(OVERLAPPED));
  ovlp.స్థానభ్రంశం_2 = స్థానభ్రంశం_దిగువ;
  ovlp.స్థానభ్రంశం_ఎగువ_2 = స్థానభ్రంశం_ఎగువ;
  తిరిగి osUnlockFileEx(*phFile, 0, అంకె_బైట్లు_దిగువ, అంకె_బైట్లు_ఎగువ, &ovlp);
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
/*
** Remove an nByte lock starting at offset iOff from HANDLE h.
*/
స్థిర పూర్ణాంకం winHandleUnlock(HANDLE h, పూర్ణాంకం iOff, పూర్ణాంకం nByte){
  BOOL ret = winUnlockFile(&h, iOff, 0, nByte, 0);
  తిరిగి (ret ? SQLITE_OK : SQLITE_IOERR_UNLOCK);
}
#అయితే_ముగింపు

/*****************************************************************************
** The next group of routines implement the I/O methods specified
** by the sqlite3_io_methods object.
******************************************************************************/

/*
** Some Microsoft compilers lack this definition.
*/
#నిర్వచితం_కాకపోతే INVALID_సమితి_FILE_సూచిక
# define INVALID_సమితి_FILE_సూచిక ((DWORD)-1)
#అయితే_ముగింపు

/*
** Seek the file handle h to offset nByte of the file.
**
** If successful, return SQLITE_OK. Or, if an error occurs, return an SQLite
** error code.
*/
స్థిర పూర్ణాంకం winHandleSeek(HANDLE h, sqlite3_int64 i_స్థానభ్రంశం){
  పూర్ణాంకం rc = SQLITE_OK;             /* Return value */
  LARGE_INTEGER x;                /* The offset */

  x.QuadPart = i_స్థానభ్రంశం;
  ఒకవేళ( os_సమితి_File_సూచిక_Ex(h, x, 0, FILE_BEGIN)==0 ){
    rc = SQLITE_IOERR_SEEK;
  }
  OSTRACE(("SEEK file=%p, offset=%lld rc=%s\n", h, i_స్థానభ్రంశం,sqlite3ErrName(rc)));
  తిరిగి rc;
}

/*
** Move the current position of the file handle passed as the first
** argument to offset iOffset within the file. If successful, return 0.
** Otherwise, set pFile->lastErrno and return non-zero.
*/
స్థిర పూర్ణాంకం winSeekFile(winFile *pFile, sqlite3_int64 i_స్థానభ్రంశం){
  పూర్ణాంకం rc;

  rc = winHandleSeek(pFile->h, i_స్థానభ్రంశం);
  ఒకవేళ( rc!=SQLITE_OK ){
    pFile->చివరి_Errno = os_Get_చివరి_Error();
    winLogError(rc, pFile->చివరి_Errno, "winSeekFile", pFile->z_మార్గం);
  }
  తిరిగి rc;
}


#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
/* Forward references to VFS helper methods used for memory mapped files */
స్థిర పూర్ణాంకం winMapfile(winFile*, sqlite3_int64);
స్థిర పూర్ణాంకం winUnmapfile(winFile*);
#అయితే_ముగింపు

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
#నిర్వచించు MX_CLOSE_ATTEMPT 3
స్థిర పూర్ణాంకం winClose(sqlite3_file *id){
  పూర్ణాంకం rc, cnt = 0;
  winFile *pFile = (winFile*)id;

  ప్రతిజ్ఞా( id!=0 );
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  ప్రతిజ్ఞా( pFile->pShm==0 );
#అయితే_ముగింపు
  ప్రతిజ్ఞా( pFile->h!=ఖాళీ && pFile->h!=INVALID_HANDLE_విలువ );
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p\n",
           osGetCurrentProcessId(), pFile, pFile->h));

#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  winUnmapfile(pFile);
#అయితే_ముగింపు

  చేయి{
    rc = osCloseHandle(pFile->h);
    /* SimulateIOError( rc=0; cnt=MX_CLOSE_ATTEMPT; ); */
  }అయితే( rc==0 && ++cnt < MX_CLOSE_ATTEMPT && (osSleep(100), 1) );
  ఒకవేళ( rc ){
    pFile->h = ఖాళీ;
  }
  OpenCounter(-1);
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, rc ? "ok" : "failed"));
  తిరిగి rc ? SQLITE_OK
            : winLogError(SQLITE_IOERR_CLOSE, os_Get_చివరి_Error(),
                          "winClose", pFile->z_మార్గం);
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
స్థిర పూర్ణాంకం winRead(
  sqlite3_file *id,          /* File to read from */
  శూన్య *pBuf,                /* Write content into this buffer */
  పూర్ణాంకం amt,                   /* Number of bytes to read */
  sqlite3_int64 స్థానభ్రంశం       /* Begin reading at this offset */
){
#ఒకవేళ !defined(SQLITE_WIN32_NO_OVERLAPPED)
  OVERLAPPED overlapped;          /* The offset for ReadFile. */
#అయితే_ముగింపు
  winFile *pFile = (winFile*)id;  /* file handle */
  DWORD nRead;                    /* Number of bytes actually read from file */
  పూర్ణాంకం nRetry = 0;                 /* Number of retrys */

  ప్రతిజ్ఞా( id!=0 );
  ప్రతిజ్ఞా( amt>0 );
  ప్రతిజ్ఞా( స్థానభ్రంశం>=0 );
  SimulateIOError(తిరిగి SQLITE_IOERR_READ);
  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, స్థానభ్రంశం, pFile->locktype));

#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  ఒకవేళ( స్థానభ్రంశం<pFile->mmap_కొలత ){
    ఒకవేళ( స్థానభ్రంశం+amt <= pFile->mmap_కొలత ){
      జ్ఞాపకం_నకలు(pBuf, &((u8 *)(pFile->pMapRegion))[స్థానభ్రంశం], amt);
      OSTRACE(("READ-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      తిరిగి SQLITE_OK;
    }లేకపోతే{
      పూర్ణాంకం nCopy = (పూర్ణాంకం)(pFile->mmap_కొలత - స్థానభ్రంశం);
      జ్ఞాపకం_నకలు(pBuf, &((u8 *)(pFile->pMapRegion))[స్థానభ్రంశం], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      స్థానభ్రంశం += nCopy;
    }
  }
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_WIN32_NO_OVERLAPPED)
  ఒకవేళ( winSeekFile(pFile, స్థానభ్రంశం) ){
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    తిరిగి SQLITE_నిండిన;
  }
  అయితే( !osReadFile(pFile->h, pBuf, amt, &nRead, 0) ){
#లేకపోతే
  memset(&overlapped, 0, పరిమాణం(OVERLAPPED));
  overlapped.స్థానభ్రంశం_2 = (LONG)(స్థానభ్రంశం & 0xffffffff);
  overlapped.స్థానభ్రంశం_ఎగువ_2 = (LONG)((స్థానభ్రంశం>>32) & 0x7fffffff);
  అయితే( !osReadFile(pFile->h, pBuf, amt, &nRead, &overlapped) &&
         os_Get_చివరి_Error()!=ERROR_HANDLE_EOF ){
#అయితే_ముగింపు
    DWORD చివరి_Errno;
    ఒకవేళ( winRetryIoerr(&nRetry, &చివరి_Errno) ) కొనసాగు;
    pFile->చివరి_Errno = చివరి_Errno;
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    తిరిగి winLogError(SQLITE_IOERR_READ, pFile->చివరి_Errno,
                       "winRead", pFile->z_మార్గం);
  }
  winLogIoerr(nRetry, __LINE__);
  ఒకవేళ( nRead<(DWORD)amt ){
    /* Unread parts of the buffer must be zero-filled */
    memset(&((అక్షరం*)pBuf)[nRead], 0, amt-nRead);
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_SHORT_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    తిరిగి SQLITE_IOERR_SHORT_READ;
  }

  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  తిరిగి SQLITE_OK;
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
స్థిర పూర్ణాంకం winWrite(
  sqlite3_file *id,               /* File to write into */
  స్థిరరాశి శూన్య *pBuf,               /* The bytes to be written */
  పూర్ణాంకం amt,                        /* Number of bytes to write */
  sqlite3_int64 స్థానభ్రంశం            /* Offset into the file to begin writing at */
){
  పూర్ణాంకం rc = 0;                     /* True if error has occurred, else false */
  winFile *pFile = (winFile*)id;  /* File handle */
  పూర్ణాంకం nRetry = 0;                 /* Number of retries */

  ప్రతిజ్ఞా( amt>0 );
  ప్రతిజ్ఞా( pFile );
  SimulateIOError(తిరిగి SQLITE_IOERR_WRITE);
  SimulateDiskfullError(తిరిగి SQLITE_నిండిన);

  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, స్థానభ్రంశం, pFile->locktype));

#ఒకవేళ defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_కొలత>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  ఒకవేళ( స్థానభ్రంశం<pFile->mmap_కొలత ){
    ఒకవేళ( స్థానభ్రంశం+amt <= pFile->mmap_కొలత ){
      జ్ఞాపకం_నకలు(&((u8 *)(pFile->pMapRegion))[స్థానభ్రంశం], pBuf, amt);
      OSTRACE(("WRITE-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      తిరిగి SQLITE_OK;
    }లేకపోతే{
      పూర్ణాంకం nCopy = (పూర్ణాంకం)(pFile->mmap_కొలత - స్థానభ్రంశం);
      జ్ఞాపకం_నకలు(&((u8 *)(pFile->pMapRegion))[స్థానభ్రంశం], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      స్థానభ్రంశం += nCopy;
    }
  }
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_WIN32_NO_OVERLAPPED)
  rc = winSeekFile(pFile, స్థానభ్రంశం);
  ఒకవేళ( rc==0 ){
#లేకపోతే
  {
#అయితే_ముగింపు
#ఒకవేళ !defined(SQLITE_WIN32_NO_OVERLAPPED)
    OVERLAPPED overlapped;        /* The offset for WriteFile. */
#అయితే_ముగింపు
    u8 *aRem = (u8 *)pBuf;        /* Data yet to be written */
    పూర్ణాంకం nRem = amt;               /* Number of bytes yet to be written */
    DWORD nWrite;                 /* Bytes written by each WriteFile() call */
    DWORD చివరి_Errno = NO_ERROR;   /* Value returned by GetLastError() */

#ఒకవేళ !defined(SQLITE_WIN32_NO_OVERLAPPED)
    memset(&overlapped, 0, పరిమాణం(OVERLAPPED));
    overlapped.స్థానభ్రంశం_2 = (LONG)(స్థానభ్రంశం & 0xffffffff);
    overlapped.స్థానభ్రంశం_ఎగువ_2 = (LONG)((స్థానభ్రంశం>>32) & 0x7fffffff);
#అయితే_ముగింపు

    అయితే( nRem>0 ){
#ఒకవేళ defined(SQLITE_WIN32_NO_OVERLAPPED)
      ఒకవేళ( !osWriteFile(pFile->h, aRem, nRem, &nWrite, 0) ){
#లేకపోతే
      ఒకవేళ( !osWriteFile(pFile->h, aRem, nRem, &nWrite, &overlapped) ){
#అయితే_ముగింపు
        ఒకవేళ( winRetryIoerr(&nRetry, &చివరి_Errno) ) కొనసాగు;
        ఆపు;
      }
      ప్రతిజ్ఞా( nWrite==0 || nWrite<=(DWORD)nRem );
      ఒకవేళ( nWrite==0 || nWrite>(DWORD)nRem ){
        చివరి_Errno = os_Get_చివరి_Error();
        ఆపు;
      }
#ఒకవేళ !defined(SQLITE_WIN32_NO_OVERLAPPED)
      స్థానభ్రంశం += nWrite;
      overlapped.స్థానభ్రంశం_2 = (LONG)(స్థానభ్రంశం & 0xffffffff);
      overlapped.స్థానభ్రంశం_ఎగువ_2 = (LONG)((స్థానభ్రంశం>>32) & 0x7fffffff);
#అయితే_ముగింపు
      aRem += nWrite;
      nRem -= nWrite;
    }
    ఒకవేళ( nRem>0 ){
      pFile->చివరి_Errno = చివరి_Errno;
      rc = 1;
    }
  }

  ఒకవేళ( rc ){
    ఒకవేళ(   ( pFile->చివరి_Errno==ERROR_HANDLE_DISK_నిండిన )
       || ( pFile->చివరి_Errno==ERROR_DISK_నిండిన )){
      OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      తిరిగి winLogError(SQLITE_నిండిన, pFile->చివరి_Errno,
                         "winWrite1", pFile->z_మార్గం);
    }
    OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_WRITE\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    తిరిగి winLogError(SQLITE_IOERR_WRITE, pFile->చివరి_Errno,
                       "winWrite2", pFile->z_మార్గం);
  }లేకపోతే{
    winLogIoerr(nRetry, __LINE__);
  }
  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  తిరిగి SQLITE_OK;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
/*
** Truncate the file opened by handle h to nByte bytes in size.
*/
స్థిర పూర్ణాంకం winHandleTruncate(HANDLE h, sqlite3_int64 nByte){ 
  పూర్ణాంకం rc = SQLITE_OK;             /* Return code */
  rc = winHandleSeek(h, nByte);
  ఒకవేళ( rc==SQLITE_OK ){
    ఒకవేళ( 0==os_సమితి_చివర_యొక్క_File(h) ){
      rc = SQLITE_IOERR_TRUNCATE;
    }
  }
  తిరిగి rc;
}

/*
** Determine the size in bytes of the file opened by the handle passed as 
** the first argument.
*/
స్థిర పూర్ణాంకం win_Handle_కొలత(HANDLE h, sqlite3_int64 *pnByte){ 
  పూర్ణాంకం rc = SQLITE_OK;
  LARGE_INTEGER x;
  ప్రతిజ్ఞా( pnByte );
  ఒకవేళ( os_Get_File_కొలత_Ex(h, &x)==0 ){
    rc = SQLITE_IOERR_FSTAT;
    *pnByte = 0;
  }లేకపోతే{
    *pnByte = x.QuadPart;
  }
  తిరిగి rc;
}

/*
** Close the handle passed as the only argument.
*/
స్థిర శూన్య winHandleClose(HANDLE h){
  ఒకవేళ( h!=INVALID_HANDLE_విలువ ){
    osCloseHandle(h);
  }
}
#అయితే_ముగింపు /* #ifndef SQLITE_OMIT_WAL */

/*
** Truncate an open file to a specified size
*/
స్థిర పూర్ణాంకం winTruncate(sqlite3_file *id, sqlite3_int64 nByte){
  winFile *pFile = (winFile*)id;  /* File handle object */
  పూర్ణాంకం rc = SQLITE_OK;             /* Return code for this function */
  DWORD చివరి_Errno;
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  sqlite3_int64 old_Mmap_కొలత;
  ఒకవేళ( pFile->n_Fetch_బహిర్గతం>0 ){
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
    తిరిగి SQLITE_OK;
  }
#అయితే_ముగింపు

  ప్రతిజ్ఞా( pFile );
  SimulateIOError(తిరిగి SQLITE_IOERR_TRUNCATE);
  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, size=%lld, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, nByte, pFile->locktype));

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  ఒకవేళ( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  ఒకవేళ( pFile->pMapRegion ){
    old_Mmap_కొలత = pFile->mmap_కొలత;
  }లేకపోతే{
    old_Mmap_కొలత = 0;
  }
  winUnmapfile(pFile);
#అయితే_ముగింపు

  /* SetEndOfFile() returns non-zero when successful, or zero when it fails. */
  ఒకవేళ( winSeekFile(pFile, nByte) ){
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->చివరి_Errno,
                     "winTruncate1", pFile->z_మార్గం);
  }లేకపోతే ఒకవేళ( 0==os_సమితి_చివర_యొక్క_File(pFile->h) &&
            ((చివరి_Errno = os_Get_చివరి_Error())!=ERROR_USER_MAPPED_FILE) ){
    pFile->చివరి_Errno = చివరి_Errno;
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->చివరి_Errno,
                     "winTruncate2", pFile->z_మార్గం);
  }

#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  ఒకవేళ( rc==SQLITE_OK && old_Mmap_కొలత>0 ){
    ఒకవేళ( old_Mmap_కొలత>nByte ){
      winMapfile(pFile, -1);
    }లేకపోతే{
      winMapfile(pFile, old_Mmap_కొలత);
    }
  }
#అయితే_ముగింపు

  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, sqlite3ErrName(rc)));
  తిరిగి rc;
}

#నిర్వచితమైతే SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occurring at the right times.
*/
పూర్ణాంకం sqlite3_sync_లెక్క = 0;
పూర్ణాంకం sqlite3_fullsync_లెక్క = 0;
#అయితే_ముగింపు

/*
** Make sure all writes to a particular file are committed to disk.
*/
స్థిర పూర్ణాంకం winSync(sqlite3_file *id, పూర్ణాంకం flags){
#నిర్వచితం_కాకపోతే SQLITE_NO_SYNC
  /*
  ** Used only when SQLITE_NO_SYNC is not defined.
   */
  BOOL rc;
#అయితే_ముగింపు
#ఒకవేళ !defined(NDEBUG) || !defined(SQLITE_NO_SYNC) || \
    defined(SQLITE_HAVE_OS_TRACE)
  /*
  ** Used when SQLITE_NO_SYNC is not defined and by the assert() and/or
  ** OSTRACE() macros.
   */
  winFile *pFile = (winFile*)id;
#లేకపోతే
  వాడని_PARAMETER(id);
#అయితే_ముగింపు

  ప్రతిజ్ఞా( pFile );
  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  ప్రతిజ్ఞా((flags&0x0F)==SQLITE_SYNC_సాధారణ
      || (flags&0x0F)==SQLITE_SYNC_నిండిన
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( తిరిగి SQLITE_నిండిన );

  OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, flags=%x, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, flags,
           pFile->locktype));

#నిర్వచితం_కాకపోతే SQLITE_TEST
  వాడని_PARAMETER(flags);
#లేకపోతే
  ఒకవేళ( (flags&0x0F)==SQLITE_SYNC_నిండిన ){
    sqlite3_fullsync_లెక్క++;
  }
  sqlite3_sync_లెక్క++;
#అయితే_ముగింపు

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op
  */
#నిర్వచితమైతే SQLITE_NO_SYNC
  OSTRACE(("SYNC-NOP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  తిరిగి SQLITE_OK;
#లేకపోతే
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  ఒకవేళ( pFile->pMapRegion ){
    ఒకవేళ( os_Flush_View_యొక్క_File(pFile->pMapRegion, 0) ){
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_OK\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
    }లేకపోతే{
      pFile->చివరి_Errno = os_Get_చివరి_Error();
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
      తిరిగి winLogError(SQLITE_IOERR_MMAP, pFile->చివరి_Errno,
                         "winSync1", pFile->z_మార్గం);
    }
  }
#అయితే_ముగింపు
  rc = osFlushFileBuffers(pFile->h);
  SimulateIOError( rc=FALSE );
  ఒకవేళ( rc ){
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    తిరిగి SQLITE_OK;
  }లేకపోతే{
    pFile->చివరి_Errno = os_Get_చివరి_Error();
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_FSYNC\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    తిరిగి winLogError(SQLITE_IOERR_FSYNC, pFile->చివరి_Errno,
                       "winSync2", pFile->z_మార్గం);
  }
#అయితే_ముగింపు
}

/*
** Determine the current size of a file in bytes
*/
స్థిర పూర్ణాంకం win_File_కొలత(sqlite3_file *id, sqlite3_int64 *p_కొలత){
  winFile *pFile = (winFile*)id;
  పూర్ణాంకం rc = SQLITE_OK;

  ప్రతిజ్ఞా( id!=0 );
  ప్రతిజ్ఞా( p_కొలత!=0 );
  SimulateIOError(తిరిగి SQLITE_IOERR_FSTAT);
  OSTRACE(("SIZE file=%p, pSize=%p\n", pFile->h, p_కొలత));
  {
    LARGE_INTEGER x;
    ఒకవేళ( os_Get_File_కొలత_Ex(pFile->h, &x)==0 ){
      *p_కొలత = 0;
      pFile->చివరి_Errno = os_Get_చివరి_Error();
      rc = winLogError(SQLITE_IOERR_FSTAT, pFile->చివరి_Errno,
                       "winFileSize", pFile->z_మార్గం);
    }లేకపోతే{
      *p_కొలత = x.QuadPart;
    }
  }
  OSTRACE(("SIZE file=%p, pSize=%p, *pSize=%lld, rc=%s\n",
           pFile->h, p_కొలత, *p_కొలత, sqlite3ErrName(rc)));
  తిరిగి rc;
}

/*
** LOCKFILE_FAIL_IMMEDIATELY is undefined on some Windows systems.
*/
#నిర్వచితం_కాకపోతే LOCKFILE_FAIL_IMMEDIATELY
# define LOCKFILE_FAIL_IMMEDIATELY 1
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే LOCKFILE_EXCLUSIVE_తాళం
# define LOCKFILE_EXCLUSIVE_తాళం 2
#అయితే_ముగింపు

/*
** Historically, SQLite has used both the LockFile and LockFileEx functions.
** When the LockFile function was used, it was always expected to fail
** immediately if the lock could not be obtained.  Also, it always expected to
** obtain an exclusive lock.  These flags are used with the LockFileEx function
** and reflect those expectations; therefore, they should not be changed.
*/
#నిర్వచితం_కాకపోతే SQLITE_LOCKFILE_FLAGS
# define SQLITE_LOCKFILE_FLAGS   (LOCKFILE_FAIL_IMMEDIATELY | \
                                  LOCKFILE_EXCLUSIVE_తాళం)
#అయితే_ముగింపు

/*
** Currently, SQLite never calls the LockFileEx function without wanting the
** call to fail immediately if the lock cannot be obtained.
*/
#నిర్వచితం_కాకపోతే SQLITE_LOCKFILEEX_FLAGS
# define SQLITE_LOCKFILEEX_FLAGS (LOCKFILE_FAIL_IMMEDIATELY)
#అయితే_ముగింపు

/*
** Acquire a reader lock.
** Different API routines are called depending on whether or not this
** is Win9x or WinNT.
*/
స్థిర పూర్ణాంకం win_Get_Read_తాళం(winFile *pFile, పూర్ణాంకం b_ఖండం){
  పూర్ణాంకం res;
  DWORD ముసుగు = ~(b_ఖండం ? LOCKFILE_FAIL_IMMEDIATELY : 0);
  OSTRACE(("READ-LOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = win_తాళం_File(&pFile->h, SQLITE_LOCKFILEEX_FLAGS&ముసుగు, SHARED_మొదటి, 0,
                    SHARED_కొలత, 0);
  ఒకవేళ( res == 0 ){
    pFile->చివరి_Errno = os_Get_చివరి_Error();
    /* No need to log a failure to lock */
  }
  OSTRACE(("READ-LOCK file=%p, result=%d\n", pFile->h, res));
  తిరిగి res;
}

/*
** Undo a readlock
*/
స్థిర పూర్ణాంకం win_Unlock_Read_తాళం(winFile *pFile){
  పూర్ణాంకం res;
  DWORD చివరి_Errno;
  OSTRACE(("READ-UNLOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winUnlockFile(&pFile->h, SHARED_మొదటి, 0, SHARED_కొలత, 0);
  ఒకవేళ( res==0 && ((చివరి_Errno = os_Get_చివరి_Error())!=ERROR_NOT_LOCKED) ){
    pFile->చివరి_Errno = చివరి_Errno;
    winLogError(SQLITE_IOERR_UNLOCK, pFile->చివరి_Errno,
                "winUnlockReadLock", pFile->z_మార్గం);
  }
  OSTRACE(("READ-UNLOCK file=%p, result=%d\n", pFile->h, res));
  తిరిగి res;
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
స్థిర పూర్ణాంకం win_తాళం(sqlite3_file *id, పూర్ణాంకం locktype){
  పూర్ణాంకం rc = SQLITE_OK;    /* Return code from subroutines */
  పూర్ణాంకం res = 1;           /* Result of a Windows lock call */
  పూర్ణాంకం newLocktype;       /* Set pFile->locktype to this value before exiting */
  పూర్ణాంకం got_Pending_తాళం = 0;/* True if we acquired a PENDING lock this time */
  winFile *pFile = (winFile*)id;
  DWORD చివరి_Errno = NO_ERROR;

  ప్రతిజ్ఞా( id!=0 );
  OSTRACE(("LOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->shared_తాళం_Byte, locktype));

  /* If there is already a lock of this type or more restrictive on the
  ** OsFile, do nothing. Don't use the end_lock: exit path, as
  ** sqlite3OsEnterMutex() hasn't been called yet.
  */
  ఒకవేళ( pFile->locktype>=locktype ){
    OSTRACE(("LOCK-HELD file=%p, rc=SQLITE_OK\n", pFile->h));
    తిరిగి SQLITE_OK;
  }

  /* Do not allow any kind of write-lock on a read-only database
  */
  ఒకవేళ( (pFile->ctrlFlags & WINFILE_RDONLY)!=0 && locktype>=RESERVED_తాళం ){
    తిరిగి SQLITE_IOERR_తాళం;
  }

  /* Make sure the locking sequence is correct
  */
  ప్రతిజ్ఞా( pFile->locktype!=NO_తాళం || locktype==SHARED_తాళం );
  ప్రతిజ్ఞా( locktype!=PENDING_తాళం );
  ప్రతిజ్ఞా( locktype!=RESERVED_తాళం || pFile->locktype==SHARED_తాళం );

  /* Lock the PENDING_LOCK byte if we need to acquire an EXCLUSIVE lock or
  ** a SHARED lock.  If we are acquiring a SHARED lock, the acquisition of
  ** the PENDING_LOCK byte is temporary.
  */
  newLocktype = pFile->locktype;
  ఒకవేళ( locktype==SHARED_తాళం
   || (locktype==EXCLUSIVE_తాళం && pFile->locktype==RESERVED_తాళం)
  ){
    పూర్ణాంకం cnt = 3;

    /* Flags for the LockFileEx() call. This should be an exclusive lock if
    ** this call is to obtain EXCLUSIVE, or a shared lock if this call is to
    ** obtain SHARED.  */
    పూర్ణాంకం flags = LOCKFILE_FAIL_IMMEDIATELY;
    ఒకవేళ( locktype==EXCLUSIVE_తాళం ){
      flags |= LOCKFILE_EXCLUSIVE_తాళం;
    }
    అయితే( cnt>0 ){
      /* Try 3 times to get the pending lock.  This is needed to work
      ** around problems caused by indexing and/or anti-virus software on
      ** Windows systems.
      **
      ** If you are using this code as a model for alternative VFSes, do not
      ** copy this retry logic.  It is a hack intended for Windows only.  */
      res = win_తాళం_File(&pFile->h, flags, PENDING_BYTE, 0, 1, 0);
      ఒకవేళ( res ) ఆపు;

      చివరి_Errno = os_Get_చివరి_Error();
      OSTRACE(("LOCK-PENDING-FAIL file=%p, count=%d, result=%d\n", 
            pFile->h, cnt, res
      ));

      ఒకవేళ( చివరి_Errno==ERROR_INVALID_HANDLE ){
        pFile->చివరి_Errno = చివరి_Errno;
        rc = SQLITE_IOERR_తాళం;
        OSTRACE(("LOCK-FAIL file=%p, count=%d, rc=%s\n", 
              pFile->h, cnt, sqlite3ErrName(rc)
        ));
        తిరిగి rc;
      }

      cnt--;
      ఒకవేళ( cnt>0 ) osSleep(1);
    }
    got_Pending_తాళం = res;
  }

  /* Acquire a shared lock
  */
  ఒకవేళ( locktype==SHARED_తాళం && res ){
    ప్రతిజ్ఞా( pFile->locktype==NO_తాళం );
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
    res = win_Get_Read_తాళం(pFile, pFile->b_ఖండం_On_Connect);
#లేకపోతే
    res = win_Get_Read_తాళం(pFile, 0);
#అయితే_ముగింపు
    ఒకవేళ( res ){
      newLocktype = SHARED_తాళం;
    }లేకపోతే{
      చివరి_Errno = os_Get_చివరి_Error();
    }
  }

  /* Acquire a RESERVED lock
  */
  ఒకవేళ( locktype==RESERVED_తాళం && res ){
    ప్రతిజ్ఞా( pFile->locktype==SHARED_తాళం );
    res = win_తాళం_File(&pFile->h, SQLITE_LOCKFILE_FLAGS, RESERVED_BYTE, 0, 1, 0);
    ఒకవేళ( res ){
      newLocktype = RESERVED_తాళం;
    }లేకపోతే{
      చివరి_Errno = os_Get_చివరి_Error();
    }
  }

  /* Acquire a PENDING lock
  */
  ఒకవేళ( locktype==EXCLUSIVE_తాళం && res ){
    newLocktype = PENDING_తాళం;
    got_Pending_తాళం = 0;
  }

  /* Acquire an EXCLUSIVE lock
  */
  ఒకవేళ( locktype==EXCLUSIVE_తాళం && res ){
    ప్రతిజ్ఞా( pFile->locktype>=SHARED_తాళం );
    (శూన్య)win_Unlock_Read_తాళం(pFile);
    res = win_తాళం_File(&pFile->h, SQLITE_LOCKFILE_FLAGS, SHARED_మొదటి, 0,
                      SHARED_కొలత, 0);
    ఒకవేళ( res ){
      newLocktype = EXCLUSIVE_తాళం;
    }లేకపోతే{
      చివరి_Errno = os_Get_చివరి_Error();
      win_Get_Read_తాళం(pFile, 0);
    }
  }

  /* If we are holding a PENDING lock that ought to be released, then
  ** release it now.
  */
  ఒకవేళ( got_Pending_తాళం && locktype==SHARED_తాళం ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }

  /* Update the state of the lock has held in the file descriptor then
  ** return the appropriate result code.
  */
  ఒకవేళ( res ){
    rc = SQLITE_OK;
  }లేకపోతే{
    pFile->చివరి_Errno = చివరి_Errno;
    rc = SQLITE_BUSY;
    OSTRACE(("LOCK-FAIL file=%p, wanted=%d, got=%d\n",
             pFile->h, locktype, newLocktype));
  }
  pFile->locktype = (u8)newLocktype;
  OSTRACE(("LOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  తిరిగి rc;
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, return
** non-zero, otherwise zero.
*/
స్థిర పూర్ణాంకం win_Check_Reserved_తాళం(sqlite3_file *id, పూర్ణాంకం *p_Res_బహిర్గతం){
  పూర్ణాంకం res;
  winFile *pFile = (winFile*)id;

  SimulateIOError( తిరిగి SQLITE_IOERR_CHECKRESERVEDLOCK; );
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p\n", pFile->h, p_Res_బహిర్గతం));

  ప్రతిజ్ఞా( id!=0 );
  ఒకవేళ( pFile->locktype>=RESERVED_తాళం ){
    res = 1;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (local)\n", pFile->h, res));
  }లేకపోతే{
    res = win_తాళం_File(&pFile->h, SQLITE_LOCKFILEEX_FLAGS,RESERVED_BYTE,0,1,0);
    ఒకవేళ( res ){
      winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
    }
    res = !res;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (remote)\n", pFile->h, res));
  }
  *p_Res_బహిర్గతం = res;
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           pFile->h, p_Res_బహిర్గతం, *p_Res_బహిర్గతం));
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం winUnlock(sqlite3_file *id, పూర్ణాంకం locktype){
  పూర్ణాంకం type;
  winFile *pFile = (winFile*)id;
  పూర్ణాంకం rc = SQLITE_OK;
  ప్రతిజ్ఞా( pFile!=0 );
  ప్రతిజ్ఞా( locktype<=SHARED_తాళం );
  OSTRACE(("UNLOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->shared_తాళం_Byte, locktype));
  type = pFile->locktype;
  ఒకవేళ( type>=EXCLUSIVE_తాళం ){
    winUnlockFile(&pFile->h, SHARED_మొదటి, 0, SHARED_కొలత, 0);
    ఒకవేళ( locktype==SHARED_తాళం && !win_Get_Read_తాళం(pFile, 0) ){
      /* This should never happen.  We should always be able to
      ** reacquire the read lock */
      rc = winLogError(SQLITE_IOERR_UNLOCK, os_Get_చివరి_Error(),
                       "winUnlock", pFile->z_మార్గం);
    }
  }
  ఒకవేళ( type>=RESERVED_తాళం ){
    winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
  }
  ఒకవేళ( locktype==NO_తాళం && type>=SHARED_తాళం ){
    win_Unlock_Read_తాళం(pFile);
  }
  ఒకవేళ( type>=PENDING_తాళం ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }
  pFile->locktype = (u8)locktype;
  OSTRACE(("UNLOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  తిరిగి rc;
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

స్థిర పూర్ణాంకం win_Nolock_తాళం(sqlite3_file *id, పూర్ణాంకం locktype){
  వాడని_PARAMETER(id);
  వాడని_PARAMETER(locktype);
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం win_Nolock_Check_Reserved_తాళం(sqlite3_file *id, పూర్ణాంకం *p_Res_బహిర్గతం){
  వాడని_PARAMETER(id);
  వాడని_PARAMETER(p_Res_బహిర్గతం);
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం winNolockUnlock(sqlite3_file *id, పూర్ణాంకం locktype){
  వాడని_PARAMETER(id);
  వాడని_PARAMETER(locktype);
  తిరిగి SQLITE_OK;
}

/******************* End of the no-op lock implementation *********************
******************************************************************************/

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
స్థిర శూన్య win_బహుళకం_బిట్టు(winFile *pFile, గుర్తులేని అక్షరం ముసుగు, పూర్ణాంకం *pArg){
  ఒకవేళ( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & ముసుగు)!=0;
  }లేకపోతే ఒకవేళ( (*pArg)==0 ){
    pFile->ctrlFlags &= ~ముసుగు;
  }లేకపోతే{
    pFile->ctrlFlags |= ముసుగు;
  }
}

/* Forward references to VFS helper methods used for temporary files */
స్థిర పూర్ణాంకం winGetTempname(sqlite3_vfs *, అక్షరం **);
స్థిర పూర్ణాంకం winIsDir(LPCWSTR);
స్థిర BOOL win_Is_Long_మార్గం_ఉపసర్గ(స్థిరరాశి అక్షరం *);
స్థిర BOOL winIsDriveLetterAndColon(స్థిరరాశి అక్షరం *);

/*
** Control and query of the open file handle.
*/
స్థిర పూర్ణాంకం winFileControl(sqlite3_file *id, పూర్ణాంకం op, శూన్య *pArg){
  winFile *pFile = (winFile*)id;
  OSTRACE(("FCNTL file=%p, op=%d, pArg=%p\n", pFile->h, op, pArg));
  ఎంపిక( op ){
    స్థితి SQLITE_FCNTL_LOCKSTATE: {
      *(పూర్ణాంకం*)pArg = pFile->locktype;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_చివరి_ERRNO: {
      *(పూర్ణాంకం*)pArg = (పూర్ణాంకం)pFile->చివరి_Errno;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_CHUNK_కొలత: {
      pFile->szChunk = *(పూర్ణాంకం *)pArg;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_కొలత_HINT: {
      ఒకవేళ( pFile->szChunk>0 ){
        sqlite3_int64 oldSz;
        పూర్ణాంకం rc = win_File_కొలత(id, &oldSz);
        ఒకవేళ( rc==SQLITE_OK ){
          sqlite3_int64 newSz = *(sqlite3_int64*)pArg;
          ఒకవేళ( newSz>oldSz ){
            SimulateIOErrorBenign(1);
            rc = winTruncate(id, newSz);
            SimulateIOErrorBenign(0);
          }
        }
        OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
        తిరిగి rc;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_PERSIST_WAL: {
      win_బహుళకం_బిట్టు(pFile, WINFILE_PERSIST_WAL, (పూర్ణాంకం*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      win_బహుళకం_బిట్టు(pFile, WINFILE_PSOW, (పూర్ణాంకం*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_VFSNAME: {
      *(అక్షరం**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_WIN32_AV_RETRY: {
      పూర్ణాంకం *a = (పూర్ణాంకం*)pArg;
      ఒకవేళ( a[0]>0 ){
        winIoerrRetry = a[0];
      }లేకపోతే{
        a[0] = winIoerrRetry;
      }
      ఒకవేళ( a[1]>0 ){
        winIoerrRetryDelay = a[1];
      }లేకపోతే{
        a[1] = winIoerrRetryDelay;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_WIN32_GET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      *phFile = pFile->h;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      తిరిగి SQLITE_OK;
    }
#నిర్వచితమైతే SQLITE_TEST
    స్థితి SQLITE_FCNTL_WIN32_సమితి_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      HANDLE hOldFile = pFile->h;
      pFile->h = *phFile;
      *phFile = hOldFile;
      OSTRACE(("FCNTL oldFile=%p, newFile=%p, rc=SQLITE_OK\n",
               hOldFile, pFile->h));
      తిరిగి SQLITE_OK;
    }
#అయితే_ముగింపు
    స్థితి SQLITE_FCNTL_NULL_IO: {
      (శూన్య)osCloseHandle(pFile->h);
      pFile->h = ఖాళీ;
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_TEMPFILENAME: {
      అక్షరం *zTFile = 0;
      పూర్ణాంకం rc = winGetTempname(pFile->pVfs, &zTFile);
      ఒకవేళ( rc==SQLITE_OK ){
        *(అక్షరం**)pArg = zTFile;
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      తిరిగి rc;
    }
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
    స్థితి SQLITE_FCNTL_MMAP_కొలత: {
      i64 new_హద్దు = *(i64*)pArg;
      పూర్ణాంకం rc = SQLITE_OK;
      ఒకవేళ( new_హద్దు>sqlite3GlobalConfig.mxMmap ){
        new_హద్దు = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (SIZE_T) and passed
      ** to MapViewOfFile(). Restrict its value to 2GB if (SIZE_T) is not at
      ** least a 64-bit type. */
      ఒకవేళ( new_హద్దు>0 && పరిమాణం(కొలత_T)<8 ){
        new_హద్దు = (new_హద్దు & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmap_కొలత_Max;
      ఒకవేళ( new_హద్దు>=0 && new_హద్దు!=pFile->mmap_కొలత_Max && pFile->n_Fetch_బహిర్గతం==0 ){
        pFile->mmap_కొలత_Max = new_హద్దు;
        ఒకవేళ( pFile->mmap_కొలత>0 ){
          winUnmapfile(pFile);
          rc = winMapfile(pFile, -1);
        }
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      తిరిగి rc;
    }
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
    స్థితి SQLITE_FCNTL_తాళం_TIMEOUT: {
      పూర్ణాంకం iOld = pFile->iBusyTimeout;
      పూర్ణాంకం iNew = *(పూర్ణాంకం*)pArg;
#ఒకవేళ SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = (iNew < 0) ? INFINITE : (DWORD)iNew;
#లేదావేళ SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = (DWORD)(!!iNew);
#లేకపోతే
# error "SQLITE_ENABLE_SETLK_TIMEOUT must be set to 1 or 2"
#అయితే_ముగింపు
      *(పూర్ణాంకం*)pArg = iOld;
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_ఖండం_ON_CONNECT: {
      పూర్ణాంకం iNew = *(పూర్ణాంకం*)pArg;
      pFile->b_ఖండం_On_Connect = iNew;
      తిరిగి SQLITE_OK;
    }
#అయితే_ముగింపు /* SQLITE_ENABLE_SETLK_TIMEOUT */

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    స్థితి SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      sqlite3_str_appendf(pStr, "{\"h\":%llu", (sqlite3_uint64)pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      ఒకవేళ( pFile->locktype ){
        స్థిర స్థిరరాశి అక్షరం *az_తాళం[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
        sqlite3_str_appendf(pStr, ",\"locktype\":\"%s\"", 
                                  az_తాళం[pFile->locktype-1]);
      }
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
      ఒకవేళ( pFile->mmap_కొలత ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmap_కొలత);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->n_Fetch_బహిర్గతం);
      }
#అయితే_ముగింపు
      sqlite3_str_append(pStr, "}", 1);
      తిరిగి SQLITE_OK;
    }
#అయితే_ముగింపు /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

  }
  OSTRACE(("FCNTL file=%p, rc=SQLITE_NOTFOUND\n", pFile->h));
  తిరిగి SQLITE_NOTFOUND;
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
స్థిర పూర్ణాంకం win_Sector_కొలత(sqlite3_file *id){
  (శూన్య)id;
  తిరిగి SQLITE_DEFAULT_SECTOR_కొలత;
}

/*
** Return a vector of device characteristics.
*/
స్థిర పూర్ణాంకం winDeviceCharacteristics(sqlite3_file *id){
  winFile *p = (winFile*)id;
  తిరిగి SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN | SQLITE_IOCAP_SUBPAGE_READ |
         ((p->ctrlFlags & WINFILE_PSOW)?SQLITE_IOCAP_POWERSAFE_OVERWRITE:0);
}

/*
** Windows will only let you create file view mappings
** on allocation size granularity boundaries.
** During sqlite3_os_init() we do a GetSystemInfo()
** to get the granularity size.
*/
స్థిర SYSTEM_INFO winSysInfo;

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
స్థిర LPWSTR winConvertFromUtf8Filename(స్థిరరాశి అక్షరం *zFilename){
  LPWSTR zConverted = 0;
#నిర్వచితమైతే __CYGWIN__
  పూర్ణాంకం nChar;

  ఒకవేళ( os_Cygwin_conv_మార్గం && !(winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2])) ){
    i64 nByte;
    పూర్ణాంకం convertflag = CCP_POSIX_గమ్యం_WIN_W;
    ఒకవేళ( !strchr(zFilename, '/') ) convertflag |= CCP_RELATIVE;
    nByte = (i64)os_Cygwin_conv_మార్గం(convertflag,
        zFilename, 0, 0);
    ఒకవేళ( nByte>0 ){
      zConverted = sqlite3MallocZero(12+(u64)nByte);
      ఒకవేళ ( zConverted==0 ){
        తిరిగి zConverted;
      }
      /* Filenames should be prefixed, except when converted
       * full path already starts with "\\?\". */
      ఒకవేళ( os_Cygwin_conv_మార్గం(convertflag, zFilename,
                           zConverted+4, nByte)==0 ){
        ఒకవేళ( (convertflag&CCP_RELATIVE) ){
          memmove(zConverted, zConverted+4, nByte);
        }లేకపోతే ఒకవేళ( memcmp(zConverted+4, L"\\\\", 4) ){
          జ్ఞాపకం_నకలు(zConverted, L"\\\\?\\", 8);
        }లేకపోతే ఒకవేళ( zConverted[6]!='?' ){
          memmove(zConverted+6, zConverted+4, nByte);
          జ్ఞాపకం_నకలు(zConverted, L"\\\\?\\UNC", 14);
        }లేకపోతే{
          memmove(zConverted, zConverted+4, nByte);
        }
        తిరిగి zConverted;
      }
      sqlite3_free(zConverted);
    }
  }
  nChar = os_Multi_Byte_గమ్యం_Wide_Char(CP_UTF8, 0, zFilename, -1, ఖాళీ, 0);
  ఒకవేళ( nChar==0 ){
    తిరిగి 0;
  }
  zConverted = sqlite3MallocZero( ((u64)nChar+6)*పరిమాణం(WCHAR) );
  ఒకవేళ( zConverted==0 ){
    తిరిగి 0;
  }
  nChar = os_Multi_Byte_గమ్యం_Wide_Char(CP_UTF8, 0, zFilename, -1,
                                zConverted, nChar);
  ఒకవేళ( nChar==0 ){
    sqlite3_free(zConverted);
    zConverted = 0;
  }లేకపోతే ఒకవేళ( nChar>MAX_మార్గం
      && winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2]) ){
    memmove(zConverted+4, zConverted, nChar*పరిమాణం(WCHAR));
    zConverted[2] = '\\';
    జ్ఞాపకం_నకలు(zConverted, L"\\\\?\\", 8);
  }లేకపోతే ఒకవేళ( nChar>MAX_మార్గం
      && winIsDirSep(zFilename[0]) && winIsDirSep(zFilename[1])
      && zFilename[2] != '?' ){
    memmove(zConverted+6, zConverted, nChar*పరిమాణం(WCHAR));
    జ్ఞాపకం_నకలు(zConverted, L"\\\\?\\UNC", 14);
  }
#లేకపోతే /* if !defined(__CYGWIN__) */
  zConverted = win_Utf8_గమ్యం_Unicode(zFilename);
#అయితే_ముగింపు /* __CYGWIN__ */
  /* caller will handle out of memory */
  తిరిగి zConverted;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL

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
స్థిర sqlite3_mutex *win_Big_తాళం = 0;
స్థిర శూన్య winShmEnterMutex(శూన్య){
  sqlite3_mutex_enter(win_Big_తాళం);
}
స్థిర శూన్య winShmLeaveMutex(శూన్య){
  sqlite3_mutex_leave(win_Big_తాళం);
}
#నిర్వచితం_కాకపోతే NDEBUG
స్థిర పూర్ణాంకం winShmMutexHeld(శూన్య) {
  తిరిగి sqlite3_mutex_held(win_Big_తాళం);
}
#అయితే_ముగింపు

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
నిర్మాణం win_Shm_కణుపు {
  sqlite3_mutex *mutex;      /* Mutex to access this object */
  అక్షరం *zFilename;           /* Name of the file */
  HANDLE hSharedShm;         /* File handle open on zFilename */
  పూర్ణాంకం b_Use_Shared_తాళం_Handle;  /* True to use hSharedShm for everything */

  పూర్ణాంకం isUnlocked;            /* DMS lock has not yet been obtained */
  పూర్ణాంకం isReadonly;            /* True if read-only */
  పూర్ణాంకం szRegion;              /* Size of shared-memory regions */
  పూర్ణాంకం nRegion;               /* Size of array apRegion */

  నిర్మాణం ShmRegion {
    HANDLE hMap;             /* File handle from CreateFileMapping */
    శూన్య *pMap;
  } *aRegion;
  DWORD చివరి_Errno;           /* The Windows errno from the last I/O error */

  winShm *pWinShmList;       /* List of winShm objects with ptrs to this */

  win_Shm_కణుపు *p_తదుపరి;         /* Next in list of all winShmNode objects */
#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 తదుపరి_Shm_Id;              /* Next available winShm.id value */
#అయితే_ముగింపు
};

/*
** A global array of all winShmNode objects.
**
** The winShmMutexHeld() must be true while reading or writing this list.
*/
స్థిర win_Shm_కణుపు *win_Shm_కణుపు_List = 0;

/*
** Structure used internally by this VFS to record the state of an
** open shared memory connection. There is one such structure for each
** winFile open on a wal mode database.
*/
నిర్మాణం winShm {
  win_Shm_కణుపు *p_Shm_కణుపు;      /* The underlying winShmNode object */
  u16 shared_ముసుగు;            /* Mask of shared locks held */
  u16 excl_ముసుగు;              /* Mask of exclusive locks held */
  HANDLE hShm;               /* File-handle on *-shm file. For locking. */
  పూర్ణాంకం bReadonly;             /* True if hShm is opened read-only */
#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 id;                     /* Id of this connection with its winShmNode */
#అయితే_ముగింపు
  winShm *p_Win_Shm_తదుపరి;       /* Next winShm object on same winShmNode */
};

/*
** Constants used for locking
*/
#నిర్వచించు WIN_SHM_పునాది   ((22+SQLITE_SHM_NLOCK)*4)        /* first lock byte */
#నిర్వచించు WIN_SHM_DMS    (WIN_SHM_పునాది+SQLITE_SHM_NLOCK)  /* deadman switch */

/* Forward references to VFS methods */
స్థిర పూర్ణాంకం winOpen(sqlite3_vfs*,స్థిరరాశి అక్షరం*,sqlite3_file*,పూర్ణాంకం,పూర్ణాంకం*);
స్థిర పూర్ణాంకం winDelete(sqlite3_vfs *,స్థిరరాశి అక్షరం*,పూర్ణాంకం);

/*
** Purge the winShmNodeList list of all entries with winShmNode.pWinShmList==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
స్థిర శూన్య winShmPurge(sqlite3_vfs *pVfs, పూర్ణాంకం deleteFlag){
  win_Shm_కణుపు **pp;
  win_Shm_కణుపు *p;
  ప్రతిజ్ఞా( winShmMutexHeld() );
  OSTRACE(("SHM-PURGE pid=%lu, deleteFlag=%d\n",
           osGetCurrentProcessId(), deleteFlag));
  pp = &win_Shm_కణుపు_List;
  అయితే( (p = *pp)!=0 ){
    ఒకవేళ( p->pWinShmList==0 ){
      పూర్ణాంకం i;
      ఒకవేళ( p->mutex ){ sqlite3_mutex_free(p->mutex); }
      కొరకు(i=0; i<p->nRegion; i++){
        BOOL bRc = os_Unmap_View_యొక్క_File(p->aRegion[i].pMap);
        OSTRACE(("SHM-PURGE-UNMAP pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        వాడని_VARIABLE_విలువ(bRc);
        bRc = osCloseHandle(p->aRegion[i].hMap);
        OSTRACE(("SHM-PURGE-CLOSE pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        వాడని_VARIABLE_విలువ(bRc);
      }
      winHandleClose(p->hSharedShm);
      ఒకవేళ( deleteFlag ){
        SimulateIOErrorBenign(1);
        sqlite3BeginBenignMalloc();
        winDelete(pVfs, p->zFilename, 0);
        sqlite3_చివర_Benign_Malloc();
        SimulateIOErrorBenign(0);
      }
      *pp = p->p_తదుపరి;
      sqlite3_free(p->aRegion);
      sqlite3_free(p);
    }లేకపోతే{
      pp = &p->p_తదుపరి;
    }
  }
}

/*
** Handle h is open on the *-shm file. This function zeroes the first
** 8 bytes of the file. If SQLITE_ENABLE_SETLK_TIMEOUT is defined, h may 
** have been opened with FILE_FLAG_OVERLAPPED.
*/
స్థిర పూర్ణాంకం win_Zero_Shared_జ్ఞాపకం(HANDLE h){
  u8 aZero[8] = {0,0,0,0,0,0,0,0};
  DWORD nWrite = 0;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, పరిమాణం(OVERLAPPED));
  ఒకవేళ( !osWriteFile(h, aZero, పరిమాణం(aZero), &nWrite, &ovlp) ){
    /* If the file was opened with FILE_FLAG_OVERLAPPED, the write may still
    ** be pending. Wait for it with GetOverlappedResult().  */
    ఒకవేళ( os_Get_చివరి_Error()!=ERROR_IO_PENDING
     || !Get_Overlapped_ఫలితం(h, &ovlp, &nWrite, TRUE)
    ){
      nWrite = 0;
    }
  }
  తిరిగి (nWrite==పరిమాణం(aZero) ? SQLITE_OK : SQLITE_IOERR_WRITE);
}

/*
** The DMS lock has not yet been taken on the shm file associated with
** pShmNode. Take the lock. Truncate the *-shm file if required.
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
స్థిర పూర్ణాంకం win_తాళం_Shared_జ్ఞాపకం(win_Shm_కణుపు *p_Shm_కణుపు, DWORD nMs){
  HANDLE h = p_Shm_కణుపు->hSharedShm;
  పూర్ణాంకం rc = SQLITE_OK;

  ప్రతిజ్ఞా( sqlite3_mutex_held(p_Shm_కణుపు->mutex) );
  rc = win_Handle_తాళం_Timeout(h, WIN_SHM_DMS, 1, 1, 0);
  ఒకవేళ( rc==SQLITE_OK ){
    /* We have an EXCLUSIVE lock on the DMS byte. This means that this
    ** is the first process to open the file. Truncate it to zero bytes
    ** in this case.  */
    ఒకవేళ( p_Shm_కణుపు->isReadonly ){
      rc = SQLITE_READONLY_CANTINIT;
    }లేకపోతే{
      ఒకవేళ( SQLITE_OK!=winHandleTruncate(h, 0) ){
        /* Sometimes the truncate operation may fail because some other
        ** process is still holding an open mapping. So try to write
        ** zeroes into the start of the file instead. */
        rc = win_Zero_Shared_జ్ఞాపకం(h);
      }
    }

    /* Release the EXCLUSIVE lock acquired above. */
    winUnlockFile(&h, WIN_SHM_DMS, 0, 1, 0);
  }లేకపోతే ఒకవేళ( (rc & 0xFF)==SQLITE_BUSY ){
    rc = SQLITE_OK;
  }

  ఒకవేళ( rc==SQLITE_OK ){
    /* Take a SHARED lock on the DMS byte. */
    rc = win_Handle_తాళం_Timeout(h, WIN_SHM_DMS, 1, 0, nMs);
    ఒకవేళ( rc==SQLITE_OK ){
      p_Shm_కణుపు->isUnlocked = 0;
    }
  }

  తిరిగి rc;
}


/*
** This function is used to open a handle on a *-shm file.
**
** If SQLITE_ENABLE_SETLK_TIMEOUT is defined at build time, then the file
** is opened with FILE_FLAG_OVERLAPPED specified. If not, it is not.
*/
స్థిర పూర్ణాంకం winHandleOpen(
  స్థిరరాశి అక్షరం *zUtf8,              /* File to open */
  పూర్ణాంకం *pbReadonly,                /* IN/OUT: True for readonly handle */
  HANDLE *ph                      /* OUT: New HANDLE for file */
){
  పూర్ణాంకం rc = SQLITE_OK;
  LPWSTR zConverted = 0;
  పూర్ణాంకం bReadonly = *pbReadonly;
  HANDLE h = INVALID_HANDLE_విలువ;

#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  స్థిరరాశి DWORD flag_overlapped = FILE_FLAG_OVERLAPPED;
#లేకపోతే
  స్థిరరాశి DWORD flag_overlapped = 0;
#అయితే_ముగింపు

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8);
  ఒకవేళ( zConverted==0 ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8));
    rc = SQLITE_IOERR_NOMEM_BKPT;
    వెళ్ళు winopenfile_బహిర్గతం;
  }

  /* Ensure the file we are trying to open is not actually a directory. */
  ఒకవేళ( winIsDir(zConverted) ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8));
    rc = SQLITE_CANTOPEN_ISDIR;
    వెళ్ళు winopenfile_బహిర్గతం;
  }

  /* TODO: platforms.
  ** TODO: retry-on-ioerr.
  */
  h = osCreateFileW(zConverted,         /* lpFileName */
      (GENERIC_READ | (bReadonly ? 0 : GENERIC_WRITE)),  /* dwDesiredAccess */
      FILE_SHARE_READ | FILE_SHARE_WRITE,        /* dwShareMode */
      ఖాళీ,                                      /* lpSecurityAttributes */
      OPEN_ALWAYS,                               /* dwCreationDisposition */
      FILE_ATTRIBUTE_సాధారణ|flag_overlapped,
      ఖాళీ
  );

  ఒకవేళ( h==INVALID_HANDLE_విలువ ){
    ఒకవేళ( bReadonly==0 ){
      bReadonly = 1;
      rc = winHandleOpen(zUtf8, &bReadonly, &h);
    }లేకపోతే{
      rc = SQLITE_CANTOPEN_BKPT;
    }
  }

 winopenfile_బహిర్గతం:
  sqlite3_free(zConverted);
  *pbReadonly = bReadonly;
  *ph = h;
  తిరిగి rc;
}
 
/*
** Close pDbFd's connection to shared-memory.  Delete the underlying
** *-shm file if deleteFlag is true.
*/
స్థిర పూర్ణాంకం win_Close_Shared_జ్ఞాపకం(winFile *pDbFd, పూర్ణాంకం deleteFlag){
  winShm *p;            /* The connection to be closed */
  winShm **pp;          /* Iterator for pShmNode->pWinShmList */
  win_Shm_కణుపు *p_Shm_కణుపు; /* The underlying shared-memory file */

  p = pDbFd->pShm;
  ఒకవేళ( p==0 ) తిరిగి SQLITE_OK;
  ఒకవేళ( p->hShm!=INVALID_HANDLE_విలువ ){
    osCloseHandle(p->hShm);
  }

  winShmEnterMutex();
  p_Shm_కణుపు = p->p_Shm_కణుపు;

  /* Remove this connection from the winShmNode.pWinShmList list */
  sqlite3_mutex_enter(p_Shm_కణుపు->mutex);
  కొరకు(pp=&p_Shm_కణుపు->pWinShmList; *pp!=p; pp=&(*pp)->p_Win_Shm_తదుపరి){}
  *pp = p->p_Win_Shm_తదుపరి;
  sqlite3_mutex_leave(p_Shm_కణుపు->mutex);

  winShmPurge(pDbFd->pVfs, deleteFlag);
  winShmLeaveMutex();

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  తిరిగి SQLITE_OK;
}

/*
** testfixture builds may set this global variable to true via a
** Tcl interface. This forces the VFS to use the locking normally
** only used for UNC paths for all files.
*/
#నిర్వచితమైతే SQLITE_TEST
పూర్ణాంకం sqlite3_win_test_unc_locking = 0;
#లేకపోతే
# define sqlite3_win_test_unc_locking 0
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం winIsUNCPath(స్థిరరాశి అక్షరం *zFile){
  ఒకవేళ( zFile[0]=='\\' && zFile[1]=='\\' ){
    ఒకవేళ( zFile[2]=='?'
     && zFile[3]=='\\'
     && sqlite3Isalpha(zFile[4])
     && zFile[5]==':'
     && winIsDirSep(zFile[6])
    ){
      తిరిగి sqlite3_win_test_unc_locking;
    }లేకపోతే{
      తిరిగి 1;
    }
  }
  తిరిగి sqlite3_win_test_unc_locking;
}

/*
** Return true if the string passed in is the name of a file on a
** remote (network-mounted) volume.
*/
స్థిర పూర్ణాంకం winIsRemoteVolume(స్థిరరాశి అక్షరం *zFile){
  WCHAR z_మూలం[4];
  ఒకవేళ( అక్షరశ్రేణి_పోలిక(zFile, "\\\\?\\",4)==0 ) zFile += 4;
  ఒకవేళ( !sqlite3Isalpha(zFile[0]) ) తిరిగి 0;
  ఒకవేళ( zFile[1]!=':' ) తిరిగి 0;
  ఒకవేళ( !winIsDirSep(zFile[2]) ) తిరిగి 0;
  z_మూలం[0] = (WCHAR)zFile[0];
  z_మూలం[1] = ':';
  z_మూలం[2] = '\\';
  z_మూలం[3] = 0;
  తిరిగి osGetDriveTypeW(z_మూలం)==DRIVE_REMOTE;
}

/*
** Open the shared-memory area associated with database file pDbFd.
*/
స్థిర పూర్ణాంకం win_Open_Shared_జ్ఞాపకం(winFile *pDbFd){
  నిర్మాణం winShm *p;                  /* The connection to be opened */
  win_Shm_కణుపు *p_Shm_కణుపు = 0;          /* The underlying mmapped file */
  పూర్ణాంకం rc = SQLITE_OK;                /* Result code */
  win_Shm_కణుపు *pNew;                  /* Newly allocated winShmNode */
  పూర్ణాంకం nName;                         /* Size of zName in bytes */

  ప్రతిజ్ఞా( pDbFd->pShm==0 );    /* Not previously opened */

  /* Allocate space for the new sqlite3_shm object.  Also speculatively
  ** allocate space for a new winShmNode and filename.  */
  p = sqlite3MallocZero( పరిమాణం(*p) );
  ఒకవేళ( p==0 ) తిరిగి SQLITE_IOERR_NOMEM_BKPT;
  nName = sqlite3Strlen30(pDbFd->z_మార్గం);
  pNew = sqlite3MallocZero( పరిమాణం(*p_Shm_కణుపు) + (i64)nName + 17 );
  ఒకవేళ( pNew==0 ){
    sqlite3_free(p);
    తిరిగి SQLITE_IOERR_NOMEM_BKPT;
  }
  pNew->zFilename = (అక్షరం*)&pNew[1];
  pNew->hSharedShm = INVALID_HANDLE_విలువ;
  pNew->isUnlocked = 1;
  ఒకవేళ( winIsUNCPath(pDbFd->z_మార్గం) || winIsRemoteVolume(pDbFd->z_మార్గం) ){
    pNew->b_Use_Shared_తాళం_Handle = 1;
  }లేకపోతే{
    pNew->b_Use_Shared_తాళం_Handle = 0;
  }
  sqlite3_snprintf(nName+15, pNew->zFilename, "%s-shm", pDbFd->z_మార్గం);
  sqlite3FileSuffix3(pDbFd->z_మార్గం, pNew->zFilename);

  /* Look to see if there is an existing winShmNode that can be used.
  ** If no matching winShmNode currently exists, then create a new one.  */
  winShmEnterMutex();
  కొరకు(p_Shm_కణుపు = win_Shm_కణుపు_List; p_Shm_కణుపు; p_Shm_కణుపు=p_Shm_కణుపు->p_తదుపరి){
    /* TBD need to come up with better match here.  Perhaps
    ** use FILE_ID_BOTH_DIR_INFO Structure.  */
    ఒకవేళ( sqlite3StrICmp(p_Shm_కణుపు->zFilename, pNew->zFilename)==0 ) ఆపు;
  }
  ఒకవేళ( p_Shm_కణుపు==0 ){
    p_Shm_కణుపు = pNew;

    /* Allocate a mutex for this winShmNode object, if one is required. */
    ఒకవేళ( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_కణుపు->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      ఒకవేళ( p_Shm_కణుపు->mutex==0 ) rc = SQLITE_IOERR_NOMEM_BKPT;
    }

    /* Open a file-handle to use for mappings, and for the DMS lock. */
    ఒకవేళ( rc==SQLITE_OK ){
      HANDLE h = INVALID_HANDLE_విలువ;
      p_Shm_కణుపు->isReadonly = sqlite3_uri_boolean(pDbFd->z_మార్గం,"readonly_shm",0);
      rc = winHandleOpen(pNew->zFilename, &p_Shm_కణుపు->isReadonly, &h);
      p_Shm_కణుపు->hSharedShm = h;
    }

    /* If successful, link the new winShmNode into the global list. If an 
    ** error occurred, free the object. */
    ఒకవేళ( rc==SQLITE_OK ){
      p_Shm_కణుపు->p_తదుపరి = win_Shm_కణుపు_List;
      win_Shm_కణుపు_List = p_Shm_కణుపు;
      pNew = 0;
    }లేకపోతే{
      sqlite3_mutex_free(p_Shm_కణుపు->mutex);
      ఒకవేళ( p_Shm_కణుపు->hSharedShm!=INVALID_HANDLE_విలువ ){
        osCloseHandle(p_Shm_కణుపు->hSharedShm);
      }
    }
  }

  /* If no error has occurred, link the winShm object to the winShmNode and
  ** the winShm to pDbFd.  */
  ఒకవేళ( rc==SQLITE_OK ){
    sqlite3_mutex_enter(p_Shm_కణుపు->mutex);
    p->p_Shm_కణుపు = p_Shm_కణుపు;
    p->p_Win_Shm_తదుపరి = p_Shm_కణుపు->pWinShmList;
    p_Shm_కణుపు->pWinShmList = p;
#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
    p->id = p_Shm_కణుపు->తదుపరి_Shm_Id++;
#అయితే_ముగింపు
    pDbFd->pShm = p;
    sqlite3_mutex_leave(p_Shm_కణుపు->mutex);
  }లేకపోతే ఒకవేళ( p ){
    sqlite3_free(p);
  }

  ప్రతిజ్ఞా( rc!=SQLITE_OK || p_Shm_కణుపు->isUnlocked==0 || p_Shm_కణుపు->nRegion==0 );
  winShmLeaveMutex();
  sqlite3_free(pNew);

  /* Open a file-handle on the *-shm file for this connection. This file-handle
  ** is only used for locking. The mapping of the *-shm file is created using
  ** the shared file handle in winShmNode.hSharedShm.  */
  ఒకవేళ( rc==SQLITE_OK && p_Shm_కణుపు->b_Use_Shared_తాళం_Handle==0 ){
    p->bReadonly = sqlite3_uri_boolean(pDbFd->z_మార్గం, "readonly_shm", 0);
    rc = winHandleOpen(p_Shm_కణుపు->zFilename, &p->bReadonly, &p->hShm);
    ఒకవేళ( rc!=SQLITE_OK ){
      ప్రతిజ్ఞా( p->hShm==INVALID_HANDLE_విలువ );
      win_Close_Shared_జ్ఞాపకం(pDbFd, 0);
    }
  }

  తిరిగి rc;
}

/*
** Close a connection to shared-memory.  Delete the underlying
** storage if deleteFlag is true.
*/
స్థిర పూర్ణాంకం winShmUnmap(
  sqlite3_file *fd,          /* Database holding shared memory */
  పూర్ణాంకం deleteFlag             /* Delete after closing if true */
){
  తిరిగి win_Close_Shared_జ్ఞాపకం((winFile*)fd, deleteFlag);
}

/*
** Change the lock state for a shared-memory segment.
*/
స్థిర పూర్ణాంకం win_Shm_తాళం(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  పూర్ణాంకం ofst,                  /* First lock to acquire or release */
  పూర్ణాంకం n,                     /* Number of locks to acquire or release */
  పూర్ణాంకం flags                  /* What to do with the lock */
){
  winFile *pDbFd = (winFile*)fd;        /* Connection holding shared memory */
  winShm *p = pDbFd->pShm;              /* The shared memory being locked */
  win_Shm_కణుపు *p_Shm_కణుపు;
  పూర్ణాంకం rc = SQLITE_OK;                   /* Result code */
  u16 ముసుగు = (u16)((1U<<(ofst+n)) - (1U<<ofst)); /* Mask of locks to [un]take */

  ఒకవేళ( p==0 ) తిరిగి SQLITE_IOERR_SHMLOCK;
  p_Shm_కణుపు = p->p_Shm_కణుపు;
  ఒకవేళ( NEVER(p_Shm_కణుపు==0) ) తిరిగి SQLITE_IOERR_SHMLOCK;

  ప్రతిజ్ఞా( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  ప్రతిజ్ఞా( n>=1 );
  ప్రతిజ్ఞా( flags==(SQLITE_SHM_తాళం | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_తాళం | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  ప్రతిజ్ఞా( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );

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
#ఒకవేళ defined(SQLITE_ENABLE_SETLK_TIMEOUT) && defined(SQLITE_DEBUG)
  {
    u16 తాళం_ముసుగు = (p->excl_ముసుగు|p->shared_ముసుగు);
    ప్రతిజ్ఞా( (flags & SQLITE_SHM_UNLOCK) || pDbFd->iBusyTimeout==0 || (
          (ofst!=2 || తాళం_ముసుగు==0)
       && (ofst!=1 || తాళం_ముసుగు==0 || తాళం_ముసుగు==2)
       && (ofst!=0 || తాళం_ముసుగు<3)
       && (ofst<3  || తాళం_ముసుగు<(1<<ofst))
    ));
  }
#అయితే_ముగింపు

  /* Check if there is any work to do. There are three cases:
  **
  **    a) An unlock operation where there are locks to unlock,
  **    b) An shared lock where the requested lock is not already held
  **    c) An exclusive lock where the requested lock is not already held
  **
  ** The SQLite core never requests an exclusive lock that it already holds.
  ** This is assert()ed immediately below.  */
  ప్రతిజ్ఞా( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_తాళం) 
       || 0==(p->excl_ముసుగు & ముసుగు)
  );
  ఒకవేళ( ((flags & SQLITE_SHM_UNLOCK) && ((p->excl_ముసుగు|p->shared_ముసుగు) & ముసుగు))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_తాళం) && 0==(p->shared_ముసుగు & ముసుగు))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_తాళం))
  ){
    HANDLE h = p->hShm;

    ఒకవేళ( flags & SQLITE_SHM_UNLOCK ){
      /* Case (a) - unlock.  */

      ప్రతిజ్ఞా( (p->excl_ముసుగు & p->shared_ముసుగు)==0 );
      ప్రతిజ్ఞా( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->excl_ముసుగు & ముసుగు)==ముసుగు );
      ప్రతిజ్ఞా( !(flags & SQLITE_SHM_SHARED) || (p->shared_ముసుగు & ముసుగు)==ముసుగు );

      ప్రతిజ్ఞా( !(flags & SQLITE_SHM_SHARED) || n==1 );
      ఒకవేళ( p_Shm_కణుపు->b_Use_Shared_తాళం_Handle ){
        h = p_Shm_కణుపు->hSharedShm;
        ఒకవేళ( flags & SQLITE_SHM_SHARED ){
          winShm *pShm;
          sqlite3_mutex_enter(p_Shm_కణుపు->mutex);
          కొరకు(pShm=p_Shm_కణుపు->pWinShmList; pShm; pShm=pShm->p_Win_Shm_తదుపరి){
            ఒకవేళ( pShm!=p && (pShm->shared_ముసుగు & ముసుగు) ){
              /* Another connection within this process is also holding this
              ** SHARED lock. So do not actually release the OS lock.  */
              h = INVALID_HANDLE_విలువ;
              ఆపు;
            }
          }
          sqlite3_mutex_leave(p_Shm_కణుపు->mutex);
        }
      }

      ఒకవేళ( h!=INVALID_HANDLE_విలువ ){
        rc = winHandleUnlock(h, ofst+WIN_SHM_పునాది, n);
      }

      /* If successful, also clear the bits in sharedMask/exclMask */
      ఒకవేళ( rc==SQLITE_OK ){
        p->excl_ముసుగు = (p->excl_ముసుగు & ~ముసుగు);
        p->shared_ముసుగు = (p->shared_ముసుగు & ~ముసుగు);
      }
    }లేకపోతే{
      పూర్ణాంకం bExcl = ((flags & SQLITE_SHM_EXCLUSIVE) ? 1 : 0);
      DWORD nMs = winFileBusyTimeout(pDbFd);

      ఒకవేళ( p_Shm_కణుపు->b_Use_Shared_తాళం_Handle ){
        winShm *pShm;
        h = p_Shm_కణుపు->hSharedShm;
        sqlite3_mutex_enter(p_Shm_కణుపు->mutex);
        కొరకు(pShm=p_Shm_కణుపు->pWinShmList; pShm; pShm=pShm->p_Win_Shm_తదుపరి){
          ఒకవేళ( bExcl ){
            ఒకవేళ( (pShm->shared_ముసుగు|pShm->excl_ముసుగు) & ముసుగు ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_విలువ;
            }
          }లేకపోతే{
            ఒకవేళ( pShm->shared_ముసుగు & ముసుగు ){
              h = INVALID_HANDLE_విలువ;
            }లేకపోతే ఒకవేళ( pShm->excl_ముసుగు & ముసుగు ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_విలువ;
            }
          }
        }
        sqlite3_mutex_leave(p_Shm_కణుపు->mutex);
      }

      ఒకవేళ( h!=INVALID_HANDLE_విలువ ){
        rc = win_Handle_తాళం_Timeout(h, ofst+WIN_SHM_పునాది, n, bExcl, nMs);
      }
      ఒకవేళ( rc==SQLITE_OK ){
        ఒకవేళ( bExcl ){
          p->excl_ముసుగు = (p->excl_ముసుగు | ముసుగు);
        }లేకపోతే{
          p->shared_ముసుగు = (p->shared_ముసుగు | ముసుగు);
        }
      }
    }
  }

  OSTRACE((
      "SHM-LOCK(%d,%d,%d) pid=%lu, id=%d, sharedMask=%03x, exclMask=%03x,"
      " rc=%s\n",
      ofst, n, flags, 
      osGetCurrentProcessId(), p->id, p->shared_ముసుగు, p->excl_ముసుగు,
      sqlite3ErrName(rc))
  );
  తిరిగి rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
స్థిర శూన్య winShmBarrier(
  sqlite3_file *fd          /* Database holding the shared memory */
){
  వాడని_PARAMETER(fd);
  sqlite3_జ్ఞాపకం_Barrier();   /* compiler-defined memory barrier */
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
స్థిర పూర్ణాంకం winShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  పూర్ణాంకం iRegion,                    /* Region to retrieve */
  పూర్ణాంకం szRegion,                   /* Size of regions */
  పూర్ణాంకం isWrite,                    /* True to extend file if necessary */
  శూన్య అస్థిర **pp              /* OUT: Mapped memory */
){
  winFile *pDbFd = (winFile*)fd;
  winShm *pShm = pDbFd->pShm;
  win_Shm_కణుపు *p_Shm_కణుపు;
  DWORD protect = పుట_READWRITE;
  DWORD flags = FILE_MAP_WRITE | FILE_MAP_READ;
  పూర్ణాంకం rc = SQLITE_OK;

  ఒకవేళ( !pShm ){
    rc = win_Open_Shared_జ్ఞాపకం(pDbFd);
    ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
    pShm = pDbFd->pShm;
    ప్రతిజ్ఞా( pShm!=0 );
  }
  p_Shm_కణుపు = pShm->p_Shm_కణుపు;

  sqlite3_mutex_enter(p_Shm_కణుపు->mutex);
  ఒకవేళ( p_Shm_కణుపు->isUnlocked ){
    /* Take the DMS lock. */
    ప్రతిజ్ఞా( p_Shm_కణుపు->nRegion==0 );
    rc = win_తాళం_Shared_జ్ఞాపకం(p_Shm_కణుపు, winFileBusyTimeout(pDbFd));
    ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు shmpage_బహిర్గతం;
  }

  ప్రతిజ్ఞా( szRegion==p_Shm_కణుపు->szRegion || p_Shm_కణుపు->nRegion==0 );
  ఒకవేళ( p_Shm_కణుపు->nRegion<=iRegion ){
    HANDLE hShared = p_Shm_కణుపు->hSharedShm;
    నిర్మాణం ShmRegion *apNew;           /* New aRegion[] array */
    i64 nByte = ((i64)iRegion+1)*(i64)szRegion;  /* Minimum file size */
    sqlite3_int64 sz;                  /* Current size of wal-index file */

    p_Shm_కణుపు->szRegion = szRegion;

    /* The requested region is not mapped into this processes address space.
    ** Check to see if it has been allocated (i.e. if the wal-index file is
    ** large enough to contain the requested region).
    */
    rc = win_Handle_కొలత(hShared, &sz);
    ఒకవేళ( rc!=SQLITE_OK ){
      rc = winLogError(rc, os_Get_చివరి_Error(), "winShmMap1", pDbFd->z_మార్గం);
      వెళ్ళు shmpage_బహిర్గతం;
    }

    ఒకవేళ( sz<nByte ){
      /* The requested memory region does not exist. If isWrite is set to
      ** zero, exit early. *pp will be set to NULL and SQLITE_OK returned.
      **
      ** Alternatively, if isWrite is non-zero, use ftruncate() to allocate
      ** the requested memory region.  */
      ఒకవేళ( !isWrite ) వెళ్ళు shmpage_బహిర్గతం;
      rc = winHandleTruncate(hShared, nByte);
      ఒకవేళ( rc!=SQLITE_OK ){
        rc = winLogError(rc, os_Get_చివరి_Error(), "winShmMap2", pDbFd->z_మార్గం);
        వెళ్ళు shmpage_బహిర్గతం;
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (నిర్మాణం ShmRegion*)sqlite3_realloc64(
        p_Shm_కణుపు->aRegion, ((i64)iRegion+1)*పరిమాణం(apNew[0])
    );
    ఒకవేళ( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      వెళ్ళు shmpage_బహిర్గతం;
    }
    p_Shm_కణుపు->aRegion = apNew;

    ఒకవేళ( p_Shm_కణుపు->isReadonly ){
      protect = పుట_READONLY;
      flags = FILE_MAP_READ;
    }

    అయితే( p_Shm_కణుపు->nRegion<=iRegion ){
      HANDLE hMap = ఖాళీ;         /* file-mapping handle */
      శూన్య *pMap = 0;             /* Mapped memory region */

#నిర్వచితమైతే SQLITE_UWP
      hMap = osCreateFileMappingFromApp(hShared, ఖాళీ, protect, nByte, ఖాళీ);
#లేకపోతే
      hMap = osCreateFileMappingW(hShared, ఖాళీ, protect, 0, nByte, ఖాళీ);
#అయితే_ముగింపు
      OSTRACE(("SHM-MAP-CREATE pid=%lu, region=%d, size=%lld, rc=%s\n",
               osGetCurrentProcessId(), p_Shm_కణుపు->nRegion, nByte,
               hMap ? "ok" : "failed"));
      ఒకవేళ( hMap ){
        i64 i_స్థానభ్రంశం = p_Shm_కణుపు->nRegion*szRegion;
        పూర్ణాంకం i_స్థానభ్రంశం_జరుపు = i_స్థానభ్రంశం % winSysInfo.dw_కేటాయింపు_Granularity;
#నిర్వచితమైతే SQLITE_UWP
        pMap = os_Map_View_యొక్క_File_From_App(hMap, flags,
            i_స్థానభ్రంశం - i_స్థానభ్రంశం_జరుపు, (i64)szRegion + i_స్థానభ్రంశం_జరుపు
        );
#లేకపోతే
        pMap = os_Map_View_యొక్క_File(hMap, flags,
            0, i_స్థానభ్రంశం - i_స్థానభ్రంశం_జరుపు, (i64)szRegion + i_స్థానభ్రంశం_జరుపు
        );
#అయితే_ముగింపు
        OSTRACE(("SHM-MAP-MAP pid=%lu, region=%d, offset=%d, size=%d, rc=%s\n",
                 osGetCurrentProcessId(), p_Shm_కణుపు->nRegion, i_స్థానభ్రంశం,
                 szRegion, pMap ? "ok" : "failed"));
      }
      ఒకవేళ( !pMap ){
        p_Shm_కణుపు->చివరి_Errno = os_Get_చివరి_Error();
        rc = winLogError(SQLITE_IOERR_SHMMAP, p_Shm_కణుపు->చివరి_Errno,
                         "winShmMap3", pDbFd->z_మార్గం);
        ఒకవేళ( hMap ) osCloseHandle(hMap);
        వెళ్ళు shmpage_బహిర్గతం;
      }

      p_Shm_కణుపు->aRegion[p_Shm_కణుపు->nRegion].pMap = pMap;
      p_Shm_కణుపు->aRegion[p_Shm_కణుపు->nRegion].hMap = hMap;
      p_Shm_కణుపు->nRegion++;
    }
  }

shmpage_బహిర్గతం:
  ఒకవేళ( p_Shm_కణుపు->nRegion>iRegion ){
    i64 i_స్థానభ్రంశం = (i64)iRegion*(i64)szRegion;
    పూర్ణాంకం i_స్థానభ్రంశం_జరుపు = i_స్థానభ్రంశం % winSysInfo.dw_కేటాయింపు_Granularity;
    అక్షరం *p = (అక్షరం *)p_Shm_కణుపు->aRegion[iRegion].pMap;
    *pp = (శూన్య *)&p[i_స్థానభ్రంశం_జరుపు];
  }లేకపోతే{
    *pp = 0;
  }
  ఒకవేళ( p_Shm_కణుపు->isReadonly && rc==SQLITE_OK ){
    rc = SQLITE_READONLY;
  }
  sqlite3_mutex_leave(p_Shm_కణుపు->mutex);
  తిరిగి rc;
}

#లేకపోతే
# define winShmMap     0
# define win_Shm_తాళం    0
# define winShmBarrier 0
# define winShmUnmap   0
#అయితే_ముగింపు /* #ifndef SQLITE_OMIT_WAL */

/*
** Cleans up the mapped region of the specified file, if any.
*/
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
స్థిర పూర్ణాంకం winUnmapfile(winFile *pFile){
  ప్రతిజ్ఞా( pFile!=0 );
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, pMapRegion=%p, "
           "mmapSize=%lld, mmapSizeMax=%lld\n",
           osGetCurrentProcessId(), pFile, pFile->hMap, pFile->pMapRegion,
           pFile->mmap_కొలత, pFile->mmap_కొలత_Max));
  ఒకవేళ( pFile->pMapRegion ){
    ఒకవేళ( !os_Unmap_View_యొక్క_File(pFile->pMapRegion) ){
      pFile->చివరి_Errno = os_Get_చివరి_Error();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(), pFile,
               pFile->pMapRegion));
      తిరిగి winLogError(SQLITE_IOERR_MMAP, pFile->చివరి_Errno,
                         "winUnmapfile1", pFile->z_మార్గం);
    }
    pFile->pMapRegion = 0;
    pFile->mmap_కొలత = 0;
  }
  ఒకవేళ( pFile->hMap!=ఖాళీ ){
    ఒకవేళ( !osCloseHandle(pFile->hMap) ){
      pFile->చివరి_Errno = os_Get_చివరి_Error();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, rc=SQLITE_IOERR_MMAP\n",
               osGetCurrentProcessId(), pFile, pFile->hMap));
      తిరిగి winLogError(SQLITE_IOERR_MMAP, pFile->చివరి_Errno,
                         "winUnmapfile2", pFile->z_మార్గం);
    }
    pFile->hMap = ఖాళీ;
  }
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile));
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం winMapfile(winFile *pFd, sqlite3_int64 nByte){
  sqlite3_int64 nMap = nByte;
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( nMap>=0 || pFd->n_Fetch_బహిర్గతం==0 );
  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, size=%lld\n",
           osGetCurrentProcessId(), pFd, nByte));

  ఒకవేళ( pFd->n_Fetch_బహిర్గతం>0 ) తిరిగి SQLITE_OK;

  ఒకవేళ( nMap<0 ){
    rc = win_File_కొలత((sqlite3_file*)pFd, &nMap);
    ఒకవేళ( rc ){
      OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_IOERR_FSTAT\n",
               osGetCurrentProcessId(), pFd));
      తిరిగి SQLITE_IOERR_FSTAT;
    }
  }
  ఒకవేళ( nMap>pFd->mmap_కొలత_Max ){
    nMap = pFd->mmap_కొలత_Max;
  }
  nMap &= ~(sqlite3_int64)(winSysInfo.dw_పుట_కొలత - 1);

  ఒకవేళ( nMap==0 && pFd->mmap_కొలత>0 ){
    winUnmapfile(pFd);
  }
  ఒకవేళ( nMap!=pFd->mmap_కొలత ){
    శూన్య *pNew = 0;
    DWORD protect = పుట_READONLY;
    DWORD flags = FILE_MAP_READ;

    winUnmapfile(pFd);
#నిర్వచితమైతే SQLITE_MMAP_READWRITE
    ఒకవేళ( (pFd->ctrlFlags & WINFILE_RDONLY)==0 ){
      protect = పుట_READWRITE;
      flags |= FILE_MAP_WRITE;
    }
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_UWP
    pFd->hMap = osCreateFileMappingFromApp(pFd->h, ఖాళీ, protect, nMap, ఖాళీ);
#లేకపోతే
    pFd->hMap = osCreateFileMappingW(pFd->h, ఖాళీ, protect,
                                (DWORD)((nMap>>32) & 0xffffffff),
                                (DWORD)(nMap & 0xffffffff), ఖాళీ);
#అయితే_ముగింపు
    ఒకవేళ( pFd->hMap==ఖాళీ ){
      pFd->చివరి_Errno = os_Get_చివరి_Error();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->చివరి_Errno,
                       "winMapfile1", pFd->z_మార్గం);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-CREATE pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      తిరిగి SQLITE_OK;
    }
    ప్రతిజ్ఞా( (nMap % winSysInfo.dw_పుట_కొలత)==0 );
    ప్రతిజ్ఞా( పరిమాణం(కొలత_T)==పరిమాణం(sqlite3_int64) || nMap<=0xffffffff );
#నిర్వచితమైతే SQLITE_UWP
    pNew = os_Map_View_యొక్క_File_From_App(pFd->hMap, flags, 0, (కొలత_T)nMap);
#లేకపోతే
    pNew = os_Map_View_యొక్క_File(pFd->hMap, flags, 0, 0, (కొలత_T)nMap);
#అయితే_ముగింపు
    ఒకవేళ( pNew==ఖాళీ ){
      osCloseHandle(pFd->hMap);
      pFd->hMap = ఖాళీ;
      pFd->చివరి_Errno = os_Get_చివరి_Error();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->చివరి_Errno,
                       "winMapfile2", pFd->z_మార్గం);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-MAP pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      తిరిగి SQLITE_OK;
    }
    pFd->pMapRegion = pNew;
    pFd->mmap_కొలత = nMap;
  }

  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFd));
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* SQLITE_MAX_MMAP_SIZE>0 */

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
స్థిర పూర్ణాంకం winFetch(sqlite3_file *fd, i64 iOff, పూర్ణాంకం nAmt, శూన్య **pp){
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */
#అయితే_ముగింపు
  *pp = 0;

  OSTRACE(("FETCH pid=%lu, pFile=%p, offset=%lld, amount=%d, pp=%p\n",
           osGetCurrentProcessId(), fd, iOff, nAmt, pp));

#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  ఒకవేళ( pFd->mmap_కొలత_Max>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    స్థిరరాశి పూర్ణాంకం n_Eof_బఫరు = 256;
    ఒకవేళ( pFd->pMapRegion==0 ){
      పూర్ణాంకం rc = winMapfile(pFd, -1);
      ఒకవేళ( rc!=SQLITE_OK ){
        OSTRACE(("FETCH pid=%lu, pFile=%p, rc=%s\n",
                 osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
        తిరిగి rc;
      }
    }
    ఒకవేళ( pFd->mmap_కొలత >= (iOff+nAmt+n_Eof_బఫరు) ){
      ప్రతిజ్ఞా( pFd->pMapRegion!=0 );
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->n_Fetch_బహిర్గతం++;
    }
  }
#అయితే_ముగింపు

  OSTRACE(("FETCH pid=%lu, pFile=%p, pp=%p, *pp=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd, pp, *pp));
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం winUnfetch(sqlite3_file *fd, i64 iOff, శూన్య *p){
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  ప్రతిజ్ఞా( (p==0)==(pFd->n_Fetch_బహిర్గతం==0) );

  /* If p!=0, it must match the iOff value. */
  ప్రతిజ్ఞా( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, offset=%lld, p=%p\n",
           osGetCurrentProcessId(), pFd, iOff, p));

  ఒకవేళ( p ){
    pFd->n_Fetch_బహిర్గతం--;
  }లేకపోతే{
    /* FIXME:  If Windows truly always prevents truncating or deleting a
    ** file while a mapping is held, then the following winUnmapfile() call
    ** is unnecessary can be omitted - potentially improving
    ** performance.  */
    winUnmapfile(pFd);
  }

  ప్రతిజ్ఞా( pFd->n_Fetch_బహిర్గతం>=0 );
#అయితే_ముగింపు

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd));
  తిరిగి SQLITE_OK;
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
స్థిర స్థిరరాశి sqlite3_io_methods win_Io_పద్ధతి = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_కొలత,                    /* xFileSize */
  win_తాళం,                        /* xLock */
  winUnlock,                      /* xUnlock */
  win_Check_Reserved_తాళం,           /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_కొలత,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  win_Shm_తాళం,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

/*
** This vector defines all the methods that can operate on an
** sqlite3_file for win32 without performing any locking.
*/
స్థిర స్థిరరాశి sqlite3_io_methods win_Io_Nolock_పద్ధతి = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_కొలత,                    /* xFileSize */
  win_Nolock_తాళం,                  /* xLock */
  winNolockUnlock,                /* xUnlock */
  win_Nolock_Check_Reserved_తాళం,     /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_కొలత,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  win_Shm_తాళం,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

స్థిర win_Vfs_App_దత్తాంశం win_App_దత్తాంశం = {
  &win_Io_పద్ధతి,       /* pMethod */
  0,                  /* pAppData */
  0                   /* bNoLock */
};

స్థిర win_Vfs_App_దత్తాంశం win_Nolock_App_దత్తాంశం = {
  &win_Io_Nolock_పద్ధతి, /* pMethod */
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
స్థిర పూర్ణాంకం win_Make_చివర_లోపల_Dir_Sep(పూర్ణాంకం nBuf, అక్షరం *zBuf){
  ఒకవేళ( zBuf ){
    పూర్ణాంకం nLen = sqlite3Strlen30(zBuf);
    ఒకవేళ( nLen>0 ){
      ఒకవేళ( winIsDirSep(zBuf[nLen-1]) ){
        తిరిగి 1;
      }లేకపోతే ఒకవేళ( nLen+1<nBuf ){
        ఒకవేళ( !osGetenv ){
          zBuf[nLen] = winGetDirSep();
        }లేకపోతే ఒకవేళ( winIsDriveLetterAndColon(zBuf) && winIsDirSep(zBuf[2]) ){
          zBuf[nLen] = '\\';
          zBuf[2]='\\';
        }లేకపోతే{
          zBuf[nLen] = '/';
        }
        zBuf[nLen+1] = '\0';
        తిరిగి 1;
      }
    }
  }
  తిరిగి 0;
}

/*
** If sqlite3_temp_directory is defined, take the mutex and return true.
**
** If sqlite3_temp_directory is NULL (undefined), omit the mutex and
** return false.
*/
స్థిర పూర్ణాంకం win_తాత్కాలిక_Dir_Defined(శూన్య){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ఒకవేళ( sqlite3_తాత్కాలిక_directory!=0 ) తిరిగి 1;
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  తిరిగి 0;
}

/*
** Create a temporary file name and store the resulting pointer into pzBuf.
** The pointer returned in pzBuf must be freed via sqlite3_free().
*/
స్థిర పూర్ణాంకం winGetTempname(sqlite3_vfs *pVfs, అక్షరం **pzBuf){
  స్థిర స్థిరరాశి అక్షరం zChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  size_t i, j;
  DWORD pid;
  పూర్ణాంకం nPre = sqlite3Strlen30(SQLITE_తాత్కాలిక_FILE_ఉపసర్గ);
  i64 nMax, nBuf, nDir, nLen;
  అక్షరం *zBuf;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  SimulateIOError( తిరిగి SQLITE_IOERR );

  /* Allocate a temporary buffer to store the fully qualified file
  ** name for the temporary file.  If this fails, we cannot continue.
  */
  nMax = pVfs->mxPathname;
  nBuf = 2 + (i64)nMax;
  zBuf = sqlite3MallocZero( nBuf );
  ఒకవేళ( !zBuf ){
    OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
    తిరిగి SQLITE_IOERR_NOMEM_BKPT;
  }

  /* Figure out the effective temporary directory.  First, check if one
  ** has been explicitly set by the application; otherwise, use the one
  ** configured by the operating system.
  */
  nDir = nMax - (nPre + 15);
  ప్రతిజ్ఞా( nDir>0 );
  ఒకవేళ( win_తాత్కాలిక_Dir_Defined() ){
    పూర్ణాంకం nDirLen = sqlite3Strlen30(sqlite3_తాత్కాలిక_directory);
    ఒకవేళ( nDirLen>0 ){
      ఒకవేళ( !winIsDirSep(sqlite3_తాత్కాలిక_directory[nDirLen-1]) ){
        nDirLen++;
      }
      ఒకవేళ( nDirLen>nDir ){
        sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
        sqlite3_free(zBuf);
        OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
        తిరిగి winLogError(SQLITE_ERROR, 0, "winGetTempname1", 0);
      }
      sqlite3_snprintf(nMax, zBuf, "%s", sqlite3_తాత్కాలిక_directory);
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  }

#ఒకవేళ defined(__CYGWIN__)
  లేకపోతే ఒకవేళ( osGetenv!=ఖాళీ ){
    స్థిర స్థిరరాశి అక్షరం *azDirs[] = {
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
    గుర్తులేని పూర్ణాంకం i;
    స్థిరరాశి అక్షరం *zDir = 0;

    ఒకవేళ( !azDirs[0] ) azDirs[0] = osGetenv("SQLITE_TMPDIR");
    ఒకవేళ( !azDirs[1] ) azDirs[1] = osGetenv("TMPDIR");
    ఒకవేళ( !azDirs[2] ) azDirs[2] = osGetenv("TMP");
    ఒకవేళ( !azDirs[3] ) azDirs[3] = osGetenv("TEMP");
    ఒకవేళ( !azDirs[4] ) azDirs[4] = osGetenv("USERPROFILE");
    కొరకు(i=0; i<పరిమాణం(azDirs)/పరిమాణం(azDirs[0]); zDir=azDirs[i++]){
      LPWSTR zConverted;
      ఒకవేళ( zDir==0 ) కొనసాగు;
      /* If the path starts with a drive letter followed by the colon
      ** character, assume it is already a native Win32 path; otherwise,
      ** it must be converted to a native Win32 path via the Cygwin API
      ** prior to using it.
      */
      {
        zConverted = winConvertFromUtf8Filename(zDir);
        ఒకవేళ( !zConverted ){
          sqlite3_free(zBuf);
          OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
          తిరిగి SQLITE_IOERR_NOMEM_BKPT;
        }
        ఒకవేళ( winIsDir(zConverted) ){
          sqlite3_snprintf(nMax, zBuf, "%s", zDir);
          sqlite3_free(zConverted);
          ఆపు;
        }
        sqlite3_free(zConverted);
      }
    }
  }
#అయితే_ముగింపు

  లేకపోతే{
    అక్షరం *zMulti;
    LPWSTR z_Wide_మార్గం = sqlite3MallocZero( nMax*పరిమాణం(WCHAR) );
    ఒకవేళ( !z_Wide_మార్గం ){
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      తిరిగి SQLITE_IOERR_NOMEM_BKPT;
    }
    ఒకవేళ( os_Get_తాత్కాలిక_మార్గం_W==0 || os_Get_తాత్కాలిక_మార్గం_W(nMax, z_Wide_మార్గం)==0 ){
      sqlite3_free(z_Wide_మార్గం);
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_GETTEMPPATH\n"));
      తిరిగి winLogError(SQLITE_IOERR_GETTEMPPATH, os_Get_చివరి_Error(),
                         "winGetTempname2", 0);
    }
    zMulti = win_Unicode_గమ్యం_Utf8(z_Wide_మార్గం);
    sqlite3_free(z_Wide_మార్గం);
    ఒకవేళ( zMulti ){
      sqlite3_snprintf(nMax, zBuf, "%s", zMulti);
      sqlite3_free(zMulti);
    }లేకపోతే{
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      తిరిగి SQLITE_IOERR_NOMEM_BKPT;
    }
  }

  /*
  ** Check to make sure the temporary directory ends with an appropriate
  ** separator.  If it does not and there is not enough space left to add
  ** one, fail.
  */
  ఒకవేళ( !win_Make_చివర_లోపల_Dir_Sep(nDir+1, zBuf) ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    తిరిగి winLogError(SQLITE_ERROR, 0, "winGetTempname4", 0);
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
  ఒకవేళ( (nLen + nPre + 17) > nBuf ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    తిరిగి winLogError(SQLITE_ERROR, 0, "winGetTempname5", 0);
  }

  sqlite3_snprintf(nBuf-16-nLen, zBuf+nLen, SQLITE_తాత్కాలిక_FILE_ఉపసర్గ);

  j = sqlite3Strlen30(zBuf);
  sqlite3_randomness(15, &zBuf[j]);
  pid = osGetCurrentProcessId();
  కొరకు(i=0; i<15; i++, j++){
    zBuf[j] += pid & 0xff;
    pid >>= 8;
    zBuf[j] = (అక్షరం)zChars[ ((గుర్తులేని అక్షరం)zBuf[j])%(పరిమాణం(zChars)-1) ];
  }
  zBuf[j] = 0;
  zBuf[j+1] = 0;
  *pzBuf = zBuf;

  OSTRACE(("TEMP-FILENAME name=%s, rc=SQLITE_OK\n", zBuf));
  తిరిగి SQLITE_OK;
}

/*
** Return TRUE if the named file is really a directory.  Return false if
** it is something other than a directory, or if there is any kind of memory
** allocation failure.
*/
స్థిర పూర్ణాంకం winIsDir(LPCWSTR zConverted){
  DWORD attr;
  పూర్ణాంకం rc = 0;
  DWORD చివరి_Errno;
  పూర్ణాంకం cnt = 0;
  WIN32_FILE_ATTRIBUTE_దత్తాంశం s_Attr_దత్తాంశం;
  memset(&s_Attr_దత్తాంశం, 0, పరిమాణం(s_Attr_దత్తాంశం));
  అయితే( !(rc = os_Get_File_లక్షణాలు_Ex_W(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_దత్తాంశం)) && winRetryIoerr(&cnt, &చివరి_Errno) ){}
  ఒకవేళ( !rc ){
    తిరిగి 0; /* Invalid name? */
  }
  attr = s_Attr_దత్తాంశం.dw_File_లక్షణాలు;
  తిరిగి (attr!=INVALID_FILE_లక్షణాలు) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

/* forward reference */
స్థిర పూర్ణాంకం winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  స్థిరరాశి అక్షరం *zFilename,     /* Name of file to check */
  పూర్ణాంకం flags,                 /* Type of test to make on this file */
  పూర్ణాంకం *p_Res_బహిర్గతం               /* OUT: Result */
);

/*
** The Windows version of xAccess() accepts an extra bit in the flags
** parameter that prevents an anti-virus retry loop.
*/
#నిర్వచించు NORETRY 0x4000

/*
** Open a file.
*/
స్థిర పూర్ణాంకం winOpen(
  sqlite3_vfs *pVfs,        /* Used to get maximum path length and AppData */
  స్థిరరాశి అక్షరం *zName,        /* Name of the file (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  పూర్ణాంకం flags,                /* Open mode flags */
  పూర్ణాంకం *p_బహిర్గతం_Flags            /* Status return flags */
){
  HANDLE h;
  DWORD చివరి_Errno = 0;
  DWORD dwDesiredAccess;
  DWORD dw_Share_బహుళకం;
  DWORD dwCreationDisposition;
  DWORD dw_Flags_And_లక్షణాలు = 0;
  win_Vfs_App_దత్తాంశం *p_App_దత్తాంశం;
  winFile *pFile = (winFile*)id;
  LPWSTR zConverted;              /* Filename in OS encoding */
  స్థిరరాశి అక్షరం *zUtf8Name = zName; /* Filename in UTF-8 encoding */
  పూర్ణాంకం cnt = 0;
  పూర్ణాంకం isRO = 0;              /* file is known to be accessible readonly */

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  అక్షరం *zTmpname = 0; /* For temporary filename, if necessary. */

  పూర్ణాంకం rc = SQLITE_OK;            /* Function Return Code */
#ఒకవేళ !defined(NDEBUG)
  పూర్ణాంకం eType = flags&0x0FFF00;  /* Type of file to open */
#అయితే_ముగింపు

  పూర్ణాంకం isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  పూర్ణాంకం isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  పూర్ణాంకం isCreate     = (flags & SQLITE_OPEN_CREATE);
  పూర్ణాంకం isReadonly   = (flags & SQLITE_OPEN_READONLY);
  పూర్ణాంకం isReadWrite  = (flags & SQLITE_OPEN_READWRITE);

#నిర్వచితం_కాకపోతే NDEBUG
  పూర్ణాంకం isOpenJournal = (isCreate && (
        eType==SQLITE_OPEN_SUPER_JOURNAL
     || eType==SQLITE_OPEN_MAIN_JOURNAL
     || eType==SQLITE_OPEN_WAL
  ));
#అయితే_ముగింపు

  OSTRACE(("OPEN name=%s, pFile=%p, flags=%x, pOutFlags=%p\n",
           zUtf8Name, id, flags, p_బహిర్గతం_Flags));

  /* Check the following statements are true:
  **
  **   (a) Exactly one of the READWRITE and READONLY flags must be set, and
  **   (b) if CREATE is set, then READWRITE must also be set, and
  **   (c) if EXCLUSIVE is set, then CREATE must also be set.
  **   (d) if DELETEONCLOSE is set, then CREATE must also be set.
  */
  ప్రతిజ్ఞా((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
  ప్రతిజ్ఞా(isCreate==0 || isReadWrite);
  ప్రతిజ్ఞా(isExclusive==0 || isCreate);
  ప్రతిజ్ఞా(isDelete==0 || isCreate);

  /* The main DB, main journal, WAL file and super-journal are never
  ** automatically deleted. Nor are they ever temporary files.  */
  ప్రతిజ్ఞా( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_DB );
  ప్రతిజ్ఞా( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_JOURNAL );
  ప్రతిజ్ఞా( (!isDelete && zName) || eType!=SQLITE_OPEN_SUPER_JOURNAL );
  ప్రతిజ్ఞా( (!isDelete && zName) || eType!=SQLITE_OPEN_WAL );

  /* Assert that the upper layer has set one of the "file-type" flags. */
  ప్రతిజ్ఞా( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_తాత్కాలిక_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_తాత్కాలిక_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );

  ప్రతిజ్ఞా( pFile!=0 );
  memset(pFile, 0, పరిమాణం(winFile));
  pFile->h = INVALID_HANDLE_విలువ;

  /* If the second argument to this function is NULL, generate a
  ** temporary file name to use
  */
  ఒకవేళ( !zUtf8Name ){
    ప్రతిజ్ఞా( isDelete && !isOpenJournal );
    rc = winGetTempname(pVfs, &zTmpname);
    ఒకవేళ( rc!=SQLITE_OK ){
      OSTRACE(("OPEN name=%s, rc=%s", zUtf8Name, sqlite3ErrName(rc)));
      తిరిగి rc;
    }
    zUtf8Name = zTmpname;
  }

  /* Database filenames are double-zero terminated if they are not
  ** URIs with parameters.  Hence, they can always be passed into
  ** sqlite3_uri_parameter().
  */
  ప్రతిజ్ఞా( (eType!=SQLITE_OPEN_MAIN_DB) || (flags & SQLITE_OPEN_URI) ||
       zUtf8Name[sqlite3Strlen30(zUtf8Name)+1]==0 );

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8Name);
  ఒకవేళ( zConverted==0 ){
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8Name));
    తిరిగి SQLITE_IOERR_NOMEM_BKPT;
  }

  ఒకవేళ( winIsDir(zConverted) ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8Name));
    తిరిగి SQLITE_CANTOPEN_ISDIR;
  }

  ఒకవేళ( isReadWrite ){
    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
  }లేకపోతే{
    dwDesiredAccess = GENERIC_READ;
  }

  /* SQLITE_OPEN_EXCLUSIVE is used to make sure that a new file is
  ** created. SQLite doesn't use it to indicate "exclusive access"
  ** as it is usually understood.
  */
  ఒకవేళ( isExclusive ){
    /* Creates a new file, only if it does not already exist. */
    /* If the file exists, it fails. */
    dwCreationDisposition = CREATE_NEW;
  }లేకపోతే ఒకవేళ( isCreate ){
    /* Open existing file, or create if it doesn't exist */
    dwCreationDisposition = OPEN_ALWAYS;
  }లేకపోతే{
    /* Opens a file, only if it exists. */
    dwCreationDisposition = OPEN_EXISTING;
  }

  ఒకవేళ( 0==sqlite3_uri_boolean(zName, "exclusive", 0) ){
    dw_Share_బహుళకం = FILE_SHARE_READ | FILE_SHARE_WRITE;
  }లేకపోతే{
    dw_Share_బహుళకం = 0;
  }

  ఒకవేళ( isDelete ){
    dw_Flags_And_లక్షణాలు = FILE_ATTRIBUTE_TEMPORARY
                               | FILE_ATTRIBUTE_HIDDEN
                               | FILE_FLAG_DELETE_ON_CLOSE;
  }లేకపోతే{
    dw_Flags_And_లక్షణాలు = FILE_ATTRIBUTE_సాధారణ;
  }
  చేయి{
    h = osCreateFileW(zConverted,
                      dwDesiredAccess,
                      dw_Share_బహుళకం, ఖాళీ,
                      dwCreationDisposition,
                      dw_Flags_And_లక్షణాలు,
                      ఖాళీ);
    ఒకవేళ( h!=INVALID_HANDLE_విలువ ) ఆపు;
    ఒకవేళ( isReadWrite ){
      పూర్ణాంకం rc2;
      sqlite3BeginBenignMalloc();
      rc2 = winAccess(pVfs, zUtf8Name, SQLITE_ACCESS_READ|NORETRY, &isRO);
      sqlite3_చివర_Benign_Malloc();
      ఒకవేళ( rc2==SQLITE_OK && isRO ) ఆపు;
    }
  }అయితే( winRetryIoerr(&cnt, &చివరి_Errno) );
  winLogIoerr(cnt, __LINE__);

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, rc=%s\n", h, zUtf8Name,
           dwDesiredAccess, (h==INVALID_HANDLE_విలువ) ? "failed" : "ok"));

  ఒకవేళ( h==INVALID_HANDLE_విలువ ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    ఒకవేళ( isReadWrite && isRO && !isExclusive ){
      తిరిగి winOpen(pVfs, zName, id,
         ((flags|SQLITE_OPEN_READONLY) &
                     ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE)),
         p_బహిర్గతం_Flags);
    }లేకపోతే{
      pFile->చివరి_Errno = చివరి_Errno;
      winLogError(SQLITE_CANTOPEN, pFile->చివరి_Errno, "winOpen", zUtf8Name);
      తిరిగి SQLITE_CANTOPEN_BKPT;
    }
  }

  ఒకవేళ( p_బహిర్గతం_Flags ){
    ఒకవేళ( isReadWrite ){
      *p_బహిర్గతం_Flags = SQLITE_OPEN_READWRITE;
    }లేకపోతే{
      *p_బహిర్గతం_Flags = SQLITE_OPEN_READONLY;
    }
  }

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, pOutFlags=%p, *pOutFlags=%d, "
           "rc=%s\n", h, zUtf8Name, dwDesiredAccess, p_బహిర్గతం_Flags, p_బహిర్గతం_Flags ?
           *p_బహిర్గతం_Flags : 0, (h==INVALID_HANDLE_విలువ) ? "failed" : "ok"));

  p_App_దత్తాంశం = (win_Vfs_App_దత్తాంశం*)pVfs->p_App_దత్తాంశం;

  sqlite3_free(zConverted);
  sqlite3_free(zTmpname);
  id->pMethods = p_App_దత్తాంశం ? p_App_దత్తాంశం->p_పద్ధతి : &win_Io_పద్ధతి;
  pFile->pVfs = pVfs;
  pFile->h = h;
  ఒకవేళ( isReadonly ){
    pFile->ctrlFlags |= WINFILE_RDONLY;
  }
  ఒకవేళ( (flags & SQLITE_OPEN_MAIN_DB)
   && sqlite3_uri_boolean(zName, "psow", SQLITE_POWERSAFE_OVERWRITE) 
  ){
    pFile->ctrlFlags |= WINFILE_PSOW;
  }
  pFile->చివరి_Errno = NO_ERROR;
  pFile->z_మార్గం = zName;
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  pFile->hMap = ఖాళీ;
  pFile->pMapRegion = 0;
  pFile->mmap_కొలత = 0;
  pFile->mmap_కొలత_Max = sqlite3GlobalConfig.szMmap;
#అయితే_ముగింపు

  OpenCounter(+1);
  తిరిగి rc;
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
స్థిర పూర్ణాంకం winDelete(
  sqlite3_vfs *pVfs,          /* Not used on win32 */
  స్థిరరాశి అక్షరం *zFilename,      /* Name of file to delete */
  పూర్ణాంకం syncDir                 /* Not used on win32 */
){
  పూర్ణాంకం cnt = 0;
  పూర్ణాంకం rc;
  DWORD attr;
  DWORD చివరి_Errno = 0;
  LPWSTR zConverted;
  వాడని_PARAMETER(pVfs);
  వాడని_PARAMETER(syncDir);

  SimulateIOError(తిరిగి SQLITE_IOERR_DELETE);
  OSTRACE(("DELETE name=%s, syncDir=%d\n", zFilename, syncDir));

  zConverted = winConvertFromUtf8Filename(zFilename);
  ఒకవేళ( zConverted==0 ){
    OSTRACE(("DELETE name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    తిరిగి SQLITE_IOERR_NOMEM_BKPT;
  }
  చేయి {
    attr = os_Get_File_లక్షణాలు_W(zConverted);
    ఒకవేళ ( attr==INVALID_FILE_లక్షణాలు ){
      చివరి_Errno = os_Get_చివరి_Error();
      ఒకవేళ( చివరి_Errno==ERROR_FILE_NOT_దొరికింది
       || చివరి_Errno==ERROR_మార్గం_NOT_దొరికింది ){
        rc = SQLITE_IOERR_DELETE_NOENT; /* Already gone? */
      }లేకపోతే{
        rc = SQLITE_ERROR;
      }
      ఆపు;
    }
    ఒకవేళ ( attr&FILE_ATTRIBUTE_DIRECTORY ){
      rc = SQLITE_ERROR; /* Files only. */
      ఆపు;
    }
    ఒకవేళ ( osDeleteFileW(zConverted) ){
      rc = SQLITE_OK; /* Deleted OK. */
      ఆపు;
    }
    ఒకవేళ ( !winRetryIoerr(&cnt, &చివరి_Errno) ){
      rc = SQLITE_ERROR; /* No more retries. */
      ఆపు;
    }
  } అయితే(1);
  ఒకవేళ( rc && rc!=SQLITE_IOERR_DELETE_NOENT ){
    rc = winLogError(SQLITE_IOERR_DELETE, చివరి_Errno, "winDelete", zFilename);
  }లేకపోతే{
    winLogIoerr(cnt, __LINE__);
  }
  sqlite3_free(zConverted);
  OSTRACE(("DELETE name=%s, rc=%s\n", zFilename, sqlite3ErrName(rc)));
  తిరిగి rc;
}

/*
** Check the existence and status of a file.
*/
స్థిర పూర్ణాంకం winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  స్థిరరాశి అక్షరం *zFilename,     /* Name of file to check */
  పూర్ణాంకం flags,                 /* Type of test to make on this file */
  పూర్ణాంకం *p_Res_బహిర్గతం               /* OUT: Result */
){
  DWORD attr;
  పూర్ణాంకం rc = 0;
  పూర్ణాంకం cnt = 0;
  DWORD చివరి_Errno = 0;
  LPWSTR zConverted;
  పూర్ణాంకం noRetry = 0;           /* Do not use winRetryIoerr() */
  WIN32_FILE_ATTRIBUTE_దత్తాంశం s_Attr_దత్తాంశం;
  వాడని_PARAMETER(pVfs);

  ఒకవేళ( (flags & NORETRY)!=0 ){
    noRetry = 1;
    flags &= ~NORETRY;
  }  

  SimulateIOError( తిరిగి SQLITE_IOERR_ACCESS; );
  OSTRACE(("ACCESS name=%s, flags=%x, pResOut=%p\n",
           zFilename, flags, p_Res_బహిర్గతం));

  ఒకవేళ( zFilename==0 ){
    *p_Res_బహిర్గతం = 0;
    OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
             zFilename, p_Res_బహిర్గతం, *p_Res_బహిర్గతం));
    తిరిగి SQLITE_OK;
  }

  zConverted = winConvertFromUtf8Filename(zFilename);
  ఒకవేళ( zConverted==0 ){
    OSTRACE(("ACCESS name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    తిరిగి SQLITE_IOERR_NOMEM_BKPT;
  }
  memset(&s_Attr_దత్తాంశం, 0, పరిమాణం(s_Attr_దత్తాంశం));
  అయితే( !(rc = os_Get_File_లక్షణాలు_Ex_W(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_దత్తాంశం))
     && !noRetry
     && winRetryIoerr(&cnt, &చివరి_Errno)
  ){ /* Loop until true */}
  ఒకవేళ( rc ){
    /* For an SQLITE_ACCESS_EXISTS query, treat a zero-length file
    ** as if it does not exist.
    */
    ఒకవేళ(    flags==SQLITE_ACCESS_EXISTS
        && s_Attr_దత్తాంశం.n_File_కొలత_ఎగువ==0
        && s_Attr_దత్తాంశం.n_File_కొలత_దిగువ==0 ){
      attr = INVALID_FILE_లక్షణాలు;
    }లేకపోతే{
      attr = s_Attr_దత్తాంశం.dw_File_లక్షణాలు;
    }
  }లేకపోతే{
    ఒకవేళ( noRetry ) చివరి_Errno = os_Get_చివరి_Error();
    winLogIoerr(cnt, __LINE__);
    ఒకవేళ( చివరి_Errno!=ERROR_FILE_NOT_దొరికింది && చివరి_Errno!=ERROR_మార్గం_NOT_దొరికింది ){
      sqlite3_free(zConverted);
      తిరిగి winLogError(SQLITE_IOERR_ACCESS, చివరి_Errno, "winAccess",
                         zFilename);
    }లేకపోతే{
      attr = INVALID_FILE_లక్షణాలు;
    }
  }
  sqlite3_free(zConverted);
  ఎంపిక( flags ){
    స్థితి SQLITE_ACCESS_READ:
    స్థితి SQLITE_ACCESS_EXISTS:
      rc = attr!=INVALID_FILE_లక్షణాలు;
      ఆపు;
    స్థితి SQLITE_ACCESS_READWRITE:
      rc = attr!=INVALID_FILE_లక్షణాలు &&
             (attr & FILE_ATTRIBUTE_READONLY)==0;
      ఆపు;
    అప్రమేయ:
      ప్రతిజ్ఞా(!"Invalid flags argument");
  }
  *p_Res_బహిర్గతం = rc;
  OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           zFilename, p_Res_బహిర్గతం, *p_Res_బహిర్గతం));
  తిరిగి SQLITE_OK;
}

/*
** Returns non-zero if the specified path name starts with the "long path"
** prefix.
*/
స్థిర BOOL win_Is_Long_మార్గం_ఉపసర్గ(
  స్థిరరాశి అక్షరం *zPathname
){
  తిరిగి ( zPathname[0]=='\\' && zPathname[1]=='\\'
        && zPathname[2]=='?'  && zPathname[3]=='\\' );
}

/*
** Returns non-zero if the specified path name starts with a drive letter
** followed by a colon character.
*/
స్థిర BOOL winIsDriveLetterAndColon(
  స్థిరరాశి అక్షరం *zPathname
){
  తిరిగి ( sqlite3Isalpha(zPathname[0]) && zPathname[1]==':' );
}

#నిర్వచితమైతే _WIN32
/*
** Returns non-zero if the specified path name should be used verbatim.  If
** non-zero is returned from this function, the calling function must simply
** use the provided path name verbatim -OR- resolve it into a full path name
** using the GetFullPathName Win32 API function (if available).
*/
స్థిర BOOL winIsVerbatimPathname(
  స్థిరరాశి అక్షరం *zPathname
){
  /*
  ** If the path name starts with a forward slash or a backslash, it is either
  ** a legal UNC name, a volume relative path, or an absolute path name in the
  ** "Unix" format on Windows.  There is no easy way to differentiate between
  ** the final two cases; therefore, we return the safer return value of TRUE
  ** so that callers of this function will simply use it verbatim.
  */
  ఒకవేళ ( winIsDirSep(zPathname[0]) ){
    తిరిగి TRUE;
  }

  /*
  ** If the path name starts with a letter and a colon it is either a volume
  ** relative path or an absolute path.  Callers of this function must not
  ** attempt to treat it as a relative path name (i.e. they should simply use
  ** it verbatim).
  */
  ఒకవేళ ( winIsDriveLetterAndColon(zPathname) ){
    తిరిగి TRUE;
  }

  /*
  ** If we get to this point, the path name should almost certainly be a purely
  ** relative one (i.e. not a UNC name, not absolute, and not volume relative).
  */
  తిరిగి FALSE;
}
#అయితే_ముగింపు /* _WIN32 */

#నిర్వచితమైతే __CYGWIN__
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
స్థిర శూన్య winSimplifyName(అక్షరం *z){
  పూర్ణాంకం i, j;
  కొరకు(i=j=0; z[i]; ++i){
    ఒకవేళ( winIsDirSep(z[i]) ){
#ఒకవేళ !defined(SQLITE_TEST)
      /* Some test-cases assume that "./foo" and "foo" are different */
      ఒకవేళ( z[i+1]=='.' && winIsDirSep(z[i+2]) ){
        ++i;
        కొనసాగు;
      }
#అయితే_ముగింపు
      ఒకవేళ( !z[i+1] || (winIsDirSep(z[i+1]) && (i!=0)) ){
        కొనసాగు;
      }
      z[j++] = osGetenv?'/':'\\';
    }లేకపోతే{
      z[j++] = z[i];
    }
  }
  అయితే(j<i) z[j++] = '\0';
}

#నిర్వచించు SQLITE_MAX_SYMLINKS 100

స్థిర పూర్ణాంకం mk_నిండిన_Pathname(
  స్థిరరాశి అక్షరం *z_మార్గం,              /* Input path */
  అక్షరం *z_బహిర్గతం,                     /* Output buffer */
  పూర్ణాంకం n_బహిర్గతం                        /* Allocated size of buffer zOut */
){
  పూర్ణాంకం n_మార్గం = sqlite3Strlen30(z_మార్గం);
  పూర్ణాంకం iOff = 0;
  ఒకవేళ( z_మార్గం[0]!='/' ){
    ఒకవేళ( osGetcwd(z_బహిర్గతం, n_బహిర్గతం-2)==0 ){
      తిరిగి winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno, "getcwd", z_మార్గం);
    }
    iOff = sqlite3Strlen30(z_బహిర్గతం);
    z_బహిర్గతం[iOff++] = '/';
  }
  ఒకవేళ( (iOff+n_మార్గం+1)>n_బహిర్గతం ){
    /* SQLite assumes that xFullPathname() nul-terminates the output buffer
    ** even if it returns an error.  */
    z_బహిర్గతం[iOff] = '\0';
    తిరిగి SQLITE_CANTOPEN_BKPT;
  }
  sqlite3_snprintf(n_బహిర్గతం-iOff, &z_బహిర్గతం[iOff], "%s", z_మార్గం);
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* __CYGWIN__ */

/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
స్థిర పూర్ణాంకం win_నిండిన_Pathname_No_Mutex(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  స్థిరరాశి అక్షరం *zRelative,        /* Possibly relative input path */
  పూర్ణాంకం n_నిండిన,                    /* Size of output buffer in bytes */
  అక్షరం *z_నిండిన                   /* Output buffer */
){
  పూర్ణాంకం nByte;
  LPWSTR zConverted;
  అక్షరం *z_బహిర్గతం;

  /* If this path name begins with "/X:" or "\\?\", where "X" is any
  ** alphabetic character, discard the initial "/" from the pathname.
  */
  ఒకవేళ( zRelative[0]=='/' && (winIsDriveLetterAndColon(zRelative+1)
       || win_Is_Long_మార్గం_ఉపసర్గ(zRelative+1)) ){
    zRelative++;
  }

  SimulateIOError( తిరిగి SQLITE_ERROR );

#నిర్వచితమైతే __CYGWIN__
  ఒకవేళ( osGetcwd ){
    z_నిండిన[n_నిండిన-1] = '\0';
    ఒకవేళ( !winIsDriveLetterAndColon(zRelative) || !winIsDirSep(zRelative[2]) ){
      పూర్ణాంకం rc = SQLITE_OK;
      పూర్ణాంకం nLink = 1;              /* Number of symbolic links followed so far */
      స్థిరరాశి అక్షరం *z_లోపల = zRelative; /* Input path for each iteration of loop */
      అక్షరం *zDel = 0;
      నిర్మాణం stat buf;

      వాడని_PARAMETER(pVfs);

      చేయి {
        /* Call lstat() on path zIn. Set bLink to true if the path
        ** is a symbolic link, or false otherwise.  */
        పూర్ణాంకం bLink = 0;
        ఒకవేళ( osLstat && osReadlink ) {
          ఒకవేళ( osLstat(z_లోపల, &buf)!=0 ){
            పూర్ణాంకం myErrno = osErrno;
            ఒకవేళ( myErrno!=ENOENT ){
              rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)myErrno,
                               "lstat", z_లోపల);
            }
          }లేకపోతే{
            bLink = ((buf.st_బహుళకం & 0170000) == 0120000);
          }

          ఒకవేళ( bLink ){
            ఒకవేళ( zDel==0 ){
              zDel = sqlite3MallocZero(n_నిండిన);
              ఒకవేళ( zDel==0 ) rc = SQLITE_NOMEM;
            }లేకపోతే ఒకవేళ( ++nLink>SQLITE_MAX_SYMLINKS ){
              rc = SQLITE_CANTOPEN_BKPT;
            }

            ఒకవేళ( rc==SQLITE_OK ){
              nByte = osReadlink(z_లోపల, zDel, n_నిండిన-1);
              ఒకవేళ( nByte ==(DWORD)-1 ){
                rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno,
                                 "readlink", z_లోపల);
              }లేకపోతే{
                ఒకవేళ( zDel[0]!='/' ){
                  పూర్ణాంకం n;
                  కొరకు(n = sqlite3Strlen30(z_లోపల); n>0 && z_లోపల[n-1]!='/'; n--);
                  ఒకవేళ( nByte+n+1>n_నిండిన ){
                    rc = SQLITE_CANTOPEN_BKPT;
                  }లేకపోతే{
                    memmove(&zDel[n], zDel, nByte+1);
                    జ్ఞాపకం_నకలు(zDel, z_లోపల, n);
                    nByte += n;
                  }
                }
                zDel[nByte] = '\0';
              }
            }

            z_లోపల = zDel;
          }
        }

        ప్రతిజ్ఞా( rc!=SQLITE_OK || z_లోపల!=z_నిండిన || z_లోపల[0]=='/' );
        ఒకవేళ( rc==SQLITE_OK && z_లోపల!=z_నిండిన ){
          rc = mk_నిండిన_Pathname(z_లోపల, z_నిండిన, n_నిండిన);
        }
        ఒకవేళ( bLink==0 ) ఆపు;
        z_లోపల = z_నిండిన;
      }అయితే( rc==SQLITE_OK );

      sqlite3_free(zDel);
      winSimplifyName(z_నిండిన);
      తిరిగి rc;
    }
  }
#అయితే_ముగింపు /* __CYGWIN__ */

#ఒకవేళ defined(_WIN32)
  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing. This function could fail if, for example, the
  ** current working directory has been unlinked.
  */
  SimulateIOError( తిరిగి SQLITE_ERROR );
  ఒకవేళ ( sqlite3_దత్తాంశం_directory && !winIsVerbatimPathname(zRelative) ){
    /*
    ** NOTE: We are dealing with a relative path name and the data
    **       directory has been set.  Therefore, use it as the basis
    **       for converting the relative path name to an absolute
    **       one by prepending the data directory and a backslash.
    */
    sqlite3_snprintf(MIN(n_నిండిన, pVfs->mxPathname), z_నిండిన, "%s%c%s",
                     sqlite3_దత్తాంశం_directory, winGetDirSep(), zRelative);
    తిరిగి SQLITE_OK;
  }
#అయితే_ముగింపు
  zConverted = winConvertFromUtf8Filename(zRelative);
  ఒకవేళ( zConverted==0 ){
    తిరిగి SQLITE_IOERR_NOMEM_BKPT;
  }
  {
    LPWSTR z_తాత్కాలిక;
    nByte = os_Get_నిండిన_మార్గం_Name_W(zConverted, 0, 0, 0);
    ఒకవేళ( nByte==0 ){
      sqlite3_free(zConverted);
      తిరిగి winLogError(SQLITE_CANTOPEN_FULLPATH, os_Get_చివరి_Error(),
                         "winFullPathname1", zRelative);
    }
    nByte += 3;
    z_తాత్కాలిక = sqlite3MallocZero( (u64)nByte*పరిమాణం(WCHAR) );
    ఒకవేళ( z_తాత్కాలిక==0 ){
      sqlite3_free(zConverted);
      తిరిగి SQLITE_IOERR_NOMEM_BKPT;
    }
    nByte = os_Get_నిండిన_మార్గం_Name_W(zConverted, nByte, z_తాత్కాలిక, 0);
    ఒకవేళ( nByte==0 ){
      sqlite3_free(zConverted);
      sqlite3_free(z_తాత్కాలిక);
      తిరిగి winLogError(SQLITE_CANTOPEN_FULLPATH, os_Get_చివరి_Error(),
                         "winFullPathname2", zRelative);
    }
    sqlite3_free(zConverted);
    z_బహిర్గతం = win_Unicode_గమ్యం_Utf8(z_తాత్కాలిక);
    sqlite3_free(z_తాత్కాలిక);
  }
  ఒకవేళ( z_బహిర్గతం ){
#నిర్వచితమైతే __CYGWIN__
    ఒకవేళ( memcmp(z_బహిర్గతం, "\\\\?\\", 4) ){
      sqlite3_snprintf(MIN(n_నిండిన, pVfs->mxPathname), z_నిండిన, "%s", z_బహిర్గతం);
    }లేకపోతే ఒకవేళ( memcmp(z_బహిర్గతం+4, "UNC\\", 4) ){
      sqlite3_snprintf(MIN(n_నిండిన, pVfs->mxPathname), z_నిండిన, "%s", z_బహిర్గతం+4);
    }లేకపోతే{
      అక్షరం *p = z_బహిర్గతం+6;
      *p = '\\';
      ఒకవేళ( osGetcwd ){
        /* On Cygwin, UNC paths use forward slashes */
        అయితే( *p ){
          ఒకవేళ( *p=='\\' ) *p = '/';
          ++p;
        }
      }
      sqlite3_snprintf(MIN(n_నిండిన, pVfs->mxPathname), z_నిండిన, "%s", z_బహిర్గతం+6);
    }
#లేకపోతే
    sqlite3_snprintf(MIN(n_నిండిన, pVfs->mxPathname), z_నిండిన, "%s", z_బహిర్గతం);
#అయితే_ముగింపు /* __CYGWIN__ */
    sqlite3_free(z_బహిర్గతం);
    తిరిగి SQLITE_OK;
  }లేకపోతే{
    తిరిగి SQLITE_IOERR_NOMEM_BKPT;
  }
}
స్థిర పూర్ణాంకం win_నిండిన_Pathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  స్థిరరాశి అక్షరం *zRelative,        /* Possibly relative input path */
  పూర్ణాంకం n_నిండిన,                    /* Size of output buffer in bytes */
  అక్షరం *z_నిండిన                   /* Output buffer */
){
  పూర్ణాంకం rc;
  MUTEX_LOGIC( sqlite3_mutex *pMutex; )
  MUTEX_LOGIC( pMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR); )
  sqlite3_mutex_enter(pMutex);
  rc = win_నిండిన_Pathname_No_Mutex(pVfs, zRelative, n_నిండిన, z_నిండిన);
  sqlite3_mutex_leave(pMutex);
  తిరిగి rc;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
స్థిర శూన్య *winDlOpen(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *zFilename){
  HANDLE h = 0;
  LPWSTR zConverted = winConvertFromUtf8Filename(zFilename);
  వాడని_PARAMETER(pVfs);
  ఒకవేళ( zConverted==0 ){
    OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (శూన్య*)0));
    తిరిగి 0;
  }
  h = os_ఎక్కించు_Library_W ? os_ఎక్కించు_Library_W(zConverted) : 0;
  OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (శూన్య*)h));
  sqlite3_free(zConverted);
  తిరిగి (శూన్య*)h;
}
స్థిర శూన్య winDlError(sqlite3_vfs *pVfs, పూర్ణాంకం nBuf, అక్షరం *z_Buf_బహిర్గతం){
  వాడని_PARAMETER(pVfs);
  win_Get_చివరి_Error_సందేశం(os_Get_చివరి_Error(), nBuf, z_Buf_బహిర్గతం);
}
స్థిర శూన్య (*winDlSym(sqlite3_vfs *pVfs,శూన్య *pH,స్థిరరాశి అక్షరం *zSym))(శూన్య){
  FARPROC proc = 0;
  వాడని_PARAMETER(pVfs);
  proc = os_Get_Proc_చిరునామా_A ? os_Get_Proc_చిరునామా_A((HANDLE)pH, zSym) : 0;
  OSTRACE(("DLSYM handle=%p, symbol=%s, address=%p\n",
           (శూన్య*)pH, zSym, (శూన్య*)proc));
  తిరిగి (శూన్య(*)(శూన్య))proc;
}
స్థిర శూన్య winDlClose(sqlite3_vfs *pVfs, శూన్య *pHandle){
  వాడని_PARAMETER(pVfs);
  ఒకవేళ( osFreeLibrary!=0 ) osFreeLibrary((HANDLE)pHandle);
  OSTRACE(("DLCLOSE handle=%p\n", (శూన్య*)pHandle));
}
#లేకపోతే /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #నిర్వచించు winDlOpen  0
  #నిర్వచించు winDlError 0
  #నిర్వచించు winDlSym   0
  #నిర్వచించు winDlClose 0
#అయితే_ముగింపు

/* State information for the randomness gatherer. */
రకనిర్వచనం నిర్మాణం EntropyGatherer EntropyGatherer;
నిర్మాణం EntropyGatherer {
  గుర్తులేని అక్షరం *a;   /* Gather entropy into this buffer */
  పూర్ణాంకం na;             /* Size of a[] in bytes */
  పూర్ణాంకం i;              /* XOR next input into a[i] */
  పూర్ణాంకం nXor;           /* Number of XOR operations done */
};

#ఒకవేళ !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
/* Mix sz bytes of entropy into p. */
స్థిర శూన్య xor_జ్ఞాపకం(EntropyGatherer *p, గుర్తులేని అక్షరం *x, పూర్ణాంకం sz){
  పూర్ణాంకం j, k;
  కొరకు(j=0, k=p->i; j<sz; j++){
    p->a[k++] ^= x[j];
    ఒకవేళ( k>=p->na ) k = 0;
  }
  p->i = k;
  p->nXor += sz;
}
#అయితే_ముగింపు /* !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS) */

/*
** Write up to nBuf bytes of randomness into zBuf.
*/
స్థిర పూర్ణాంకం winRandomness(sqlite3_vfs *pVfs, పూర్ణాంకం nBuf, అక్షరం *zBuf){
#ఒకవేళ defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS)
  వాడని_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  తిరిగి nBuf;
#లేకపోతే
  EntropyGatherer e;
  వాడని_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  e.a = (గుర్తులేని అక్షరం*)zBuf;
  e.na = nBuf;
  e.nXor = 0;
  e.i = 0;
  {
    FILETIME x;
    os_Get_System_సమయం_As_File_సమయం(&x);
    xor_జ్ఞాపకం(&e, (గుర్తులేని అక్షరం*)&x, పరిమాణం(x));
  }
  {
    DWORD pid = osGetCurrentProcessId();
    xor_జ్ఞాపకం(&e, (గుర్తులేని అక్షరం*)&pid, పరిమాణం(pid));
  }
  {
    ULONGLONG cnt = osGetTickCount64();
    xor_జ్ఞాపకం(&e, (గుర్తులేని అక్షరం*)&cnt, పరిమాణం(cnt));
  }
  {
    LARGE_INTEGER i;
    osQueryPerformanceCounter(&i);
    xor_జ్ఞాపకం(&e, (గుర్తులేని అక్షరం*)&i, పరిమాణం(i));
  }
#ఒకవేళ SQLITE_WIN32_USE_UUID
#నిర్వచితమైతే SQLITE_UWP
# error SQLITE_WIN32_USE_UUID is incompatible with SQLITE_UWP
#అయితే_ముగింపు
  {
    UUID id;
    memset(&id, 0, పరిమాణం(id));
    osUuidCreate(&id);
    xor_జ్ఞాపకం(&e, (గుర్తులేని అక్షరం*)&id, పరిమాణం(id));
    memset(&id, 0, పరిమాణం(UUID));
    osUuidCreateSequential(&id);
    xor_జ్ఞాపకం(&e, (గుర్తులేని అక్షరం*)&id, పరిమాణం(id));
  }
#అయితే_ముగింపు /* SQLITE_WIN32_USE_UUID */
  తిరిగి e.nXor>nBuf ? nBuf : e.nXor;
#అయితే_ముగింపు /* defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS) */
}


/*
** Sleep for a little while.  Return the amount of time slept.
*/
స్థిర పూర్ణాంకం winSleep(sqlite3_vfs *pVfs, పూర్ణాంకం microsec){
  osSleep((microsec+999)/1000);
  వాడని_PARAMETER(pVfs);
  తిరిగి ((microsec+999)/1000)*1000;
}

/*
** The following variable, if set to a non-zero value, is interpreted as
** the number of seconds since 1970 and is used to set the result of
** sqlite3OsCurrentTime() during testing.
*/
#నిర్వచితమైతే SQLITE_TEST
పూర్ణాంకం sqlite3_current_సమయం = 0;  /* Fake system time in seconds since 1970. */
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం win_Current_సమయం_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *piNow){
  /* FILETIME structure is a 64-bit value representing the number of
     100-nanosecond intervals since January 1, 1601 (= JD 2305813.5).
  */
  FILETIME ft;
  స్థిర స్థిరరాశి sqlite3_int64 winFiletimeEpoch = 23058135*(sqlite3_int64)8640000;
#నిర్వచితమైతే SQLITE_TEST
  స్థిర స్థిరరాశి sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
#అయితే_ముగింపు
  /* 2^32 - to avoid use of LL and warnings in gcc */
  స్థిర స్థిరరాశి sqlite3_int64 max32_బిట్టు_విలువ =
      (sqlite3_int64)2000000000 + (sqlite3_int64)2000000000 +
      (sqlite3_int64)294967296;

  os_Get_System_సమయం_As_File_సమయం( &ft );
  *piNow = winFiletimeEpoch +
            ((((sqlite3_int64)ft.dw_ఎగువ_Date_సమయం)*max32_బిట్టు_విలువ) +
               (sqlite3_int64)ft.dw_దిగువ_Date_సమయం)/(sqlite3_int64)10000;

#నిర్వచితమైతే SQLITE_TEST
  ఒకవేళ( sqlite3_current_సమయం ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_సమయం + unixEpoch;
  }
#అయితే_ముగింపు
  వాడని_PARAMETER(pVfs);
  తిరిగి SQLITE_OK;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
స్థిర పూర్ణాంకం win_Current_సమయం(sqlite3_vfs *pVfs, ద్విగుణ *prNow){
  పూర్ణాంకం rc;
  sqlite3_int64 i;
  rc = win_Current_సమయం_Int64(pVfs, &i);
  ఒకవేళ( !rc ){
    *prNow = i/86400000.0;
  }
  తిరిగి rc;
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
స్థిర పూర్ణాంకం win_Get_చివరి_Error(sqlite3_vfs *pVfs, పూర్ణాంకం nBuf, అక్షరం *zBuf){
  DWORD e = os_Get_చివరి_Error();
  వాడని_PARAMETER(pVfs);
  ఒకవేళ( nBuf>0 ) win_Get_చివరి_Error_సందేశం(e, nBuf, zBuf);
  తిరిగి e;
}

/*
** Initialize and deinitialize the operating system interface.
*/
పూర్ణాంకం sqlite3_os_init(శూన్య){
  స్థిర sqlite3_vfs winVfs = {
    3,                     /* iVersion */
    పరిమాణం(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_మార్గం_బైట్లు, /* mxPathname */
    0,                     /* pNext */
    "win32",               /* zName */
    &win_App_దత్తాంశం,           /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_నిండిన_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_సమయం,        /* xCurrentTime */
    win_Get_చివరి_Error,       /* xGetLastError */
    win_Current_సమయం_Int64,   /* xCurrentTimeInt64 */
    win_సమితి_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_తదుపరి_System_Call,     /* xNextSystemCall */
  };
  స్థిర sqlite3_vfs win_Long_మార్గం_Vfs = {
    3,                     /* iVersion */
    పరిమాణం(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_మార్గం_బైట్లు, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath",      /* zName */
    &win_App_దత్తాంశం,           /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_నిండిన_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_సమయం,        /* xCurrentTime */
    win_Get_చివరి_Error,       /* xGetLastError */
    win_Current_సమయం_Int64,   /* xCurrentTimeInt64 */
    win_సమితి_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_తదుపరి_System_Call,     /* xNextSystemCall */
  };
  స్థిర sqlite3_vfs winNolockVfs = {
    3,                     /* iVersion */
    పరిమాణం(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_మార్గం_బైట్లు, /* mxPathname */
    0,                     /* pNext */
    "win32-none",          /* zName */
    &win_Nolock_App_దత్తాంశం,     /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_నిండిన_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_సమయం,        /* xCurrentTime */
    win_Get_చివరి_Error,       /* xGetLastError */
    win_Current_సమయం_Int64,   /* xCurrentTimeInt64 */
    win_సమితి_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_తదుపరి_System_Call,     /* xNextSystemCall */
  };
  స్థిర sqlite3_vfs win_Long_మార్గం_Nolock_Vfs = {
    3,                     /* iVersion */
    పరిమాణం(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_మార్గం_బైట్లు, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath-none", /* zName */
    &win_Nolock_App_దత్తాంశం,     /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_నిండిన_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_సమయం,        /* xCurrentTime */
    win_Get_చివరి_Error,       /* xGetLastError */
    win_Current_సమయం_Int64,   /* xCurrentTimeInt64 */
    win_సమితి_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_తదుపరి_System_Call,     /* xNextSystemCall */
  };

  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  ప్రతిజ్ఞా( Array_కొలత(aSyscall)==60 );
  ప్రతిజ్ఞా( strcmp(aSyscall[0].zName,"AreFileApisANSI")==0 );
  ప్రతిజ్ఞా( strcmp(aSyscall[8].zName,"FreeLibrary")==0 );
  ప్రతిజ్ఞా( strcmp(aSyscall[16].zName,"GetSystemInfo")==0 );
  ప్రతిజ్ఞా( strcmp(aSyscall[24].zName,"HeapReAlloc")==0 );
  ప్రతిజ్ఞా( strcmp(aSyscall[32].zName,"MapViewOfFileFromApp")==0 );
  ప్రతిజ్ఞా( strcmp(aSyscall[40].zName,"UnmapViewOfFile")==0 );
  ప్రతిజ్ఞా( strcmp(aSyscall[48].zName,"UuidCreate")==0 );
  ప్రతిజ్ఞా( strcmp(aSyscall[56].zName,"lstat")==0 );

  /* get memory map allocation granularity */
  memset(&winSysInfo, 0, పరిమాణం(SYSTEM_INFO));
  osGetSystemInfo(&winSysInfo);
  ప్రతిజ్ఞా( winSysInfo.dw_కేటాయింపు_Granularity>0 );
  ప్రతిజ్ఞా( winSysInfo.dw_పుట_కొలత>0 );

  sqlite3_vfs_రిజిస్టరు(&winVfs, 1);
  sqlite3_vfs_రిజిస్టరు(&win_Long_మార్గం_Vfs, 0);
  sqlite3_vfs_రిజిస్టరు(&winNolockVfs, 0);
  sqlite3_vfs_రిజిస్టరు(&win_Long_మార్గం_Nolock_Vfs, 0);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  win_Big_తాళం = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#అయితే_ముగింపు

  తిరిగి SQLITE_OK;
}

పూర్ణాంకం sqlite3_os_చివర(శూన్య){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  win_Big_తాళం = 0;
#అయితే_ముగింపు
  తిరిగి SQLITE_OK;
}

#అయితే_ముగింపు /* SQLITE_OS_WIN */
