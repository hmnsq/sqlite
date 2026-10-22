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
#अन्तर्भाव "sqliteInt.h"
#यदि SQLITE_OS_WIN               /* This file is used for Windows only */

/*
** Include code that is common to all os_*.c files
*/
#अन्तर्भाव "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#अन्तर्भाव "os_win.h"

/*
** This constant should already be defined (in the "WinDef.h" SDK file).
*/
#यदि_अपरिभाषितम् MAX_मार्गः
#  define MAX_मार्गः                      (260)
#यदि_अन्तः

/*
** Maximum pathname length (in chars) for Win32.  This should normally be
** MAX_PATH.
*/
#यदि_अपरिभाषितम् SQLITE_WIN32_MAX_मार्गः_CHARS
#  define SQLITE_WIN32_MAX_मार्गः_CHARS   (MAX_मार्गः)
#यदि_अन्तः

/*
** This constant should already be defined (in the "WinNT.h" SDK file).
*/
#यदि_अपरिभाषितम् UNICODE_STRING_MAX_CHARS
#  define UNICODE_STRING_MAX_CHARS      (32767)
#यदि_अन्तः

/*
** Maximum pathname length (in chars) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS.
*/
#यदि_अपरिभाषितम् SQLITE_WINNT_MAX_मार्गः_CHARS
#  define SQLITE_WINNT_MAX_मार्गः_CHARS   (UNICODE_STRING_MAX_CHARS)
#यदि_अन्तः

/*
** Maximum pathname length (in bytes) for Win32.  The MAX_PATH macro is in
** characters, so we allocate 4 bytes per character assuming worst-case of
** 4-bytes-per-character for UTF8.
*/
#यदि_अपरिभाषितम् SQLITE_WIN32_MAX_मार्गः_अष्टकानि
#  define SQLITE_WIN32_MAX_मार्गः_अष्टकानि   (SQLITE_WIN32_MAX_मार्गः_CHARS*4)
#यदि_अन्तः

/*
** Maximum pathname length (in bytes) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS * sizeof(WCHAR).
*/
#यदि_अपरिभाषितम् SQLITE_WINNT_MAX_मार्गः_अष्टकानि
#  define SQLITE_WINNT_MAX_मार्गः_अष्टकानि   \
                            (परिमाण(WCHAR) * SQLITE_WINNT_MAX_मार्गः_CHARS)
#यदि_अन्तः

/*
** Maximum error message length (in chars) for WinRT.
*/
#यदि_अपरिभाषितम् SQLITE_WIN32_MAX_ERRMSG_CHARS
#  define SQLITE_WIN32_MAX_ERRMSG_CHARS (1024)
#यदि_अन्तः

/*
** Returns non-zero if the character should be treated as a directory
** separator.
*/
#यदि_अपरिभाषितम् winIsDirSep
#  define winIsDirSep(a)                (((a) == '/') || ((a) == '\\'))
#यदि_अन्तः

/*
** This macro is used when a local variable is set to a value that is
** [sometimes] not used by the code (e.g. via conditional compilation).
*/
#यदि_अपरिभाषितम् अप्रयुक्तम्_VARIABLE_मानम्
#  define अप्रयुक्तम्_VARIABLE_मानम्(x)      (शून्य)(x)
#यदि_अन्तः

/*
** Returns the character that should be used as the directory separator.
*/
#यदि_अपरिभाषितम् winGetDirSep
#  define winGetDirSep()                '\\'
#यदि_अन्तः

/*
** Some older Microsoft compilers lack the following definition.
*/
#यदि_अपरिभाषितम् INVALID_FILE_गुणाः
# define INVALID_FILE_गुणाः ((DWORD)-1)
#यदि_अन्तः

#यदि_अपरिभाषितम् FILE_FLAG_आवरणम्
# define FILE_FLAG_आवरणम्          (0xFF3C0000)
#यदि_अन्तः

#यदि_अपरिभाषितम् FILE_ATTRIBUTE_आवरणम्
# define FILE_ATTRIBUTE_आवरणम्     (0x0003FFF7)
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
/* Forward references to structures used for WAL */
प्रकारनिर्देश संरचना winShm winShm;           /* A connection to shared-memory */
प्रकारनिर्देश संरचना win_Shm_ग्रन्थिः win_Shm_ग्रन्थिः;   /* A region of shared-memory */
#यदि_अन्तः

/*
** The winFile structure is a subclass of sqlite3_file* specific to the win32
** portability layer.
*/
प्रकारनिर्देश संरचना winFile winFile;
संरचना winFile {
  ध्रुव sqlite3_io_methods *p_विधानम्; /*** Must be first ***/
  sqlite3_vfs *pVfs;      /* The VFS used to open this file */
  HANDLE h;               /* Handle for accessing the file */
  u8 locktype;            /* Type of lock currently held on this file */
  ह्रस्व shared_तालकम्_Byte;   /* Randomly chosen byte used as a shared lock */
  u8 ctrlFlags;           /* Flags.  See WINFILE_* below */
  DWORD चरमः_Errno;        /* The Windows errno from the last I/O error */
#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
  winShm *pShm;           /* Instance of shared memory on this file */
#यदि_अन्तः
  ध्रुव वर्ण *z_मार्गः;      /* Full pathname of this file */
  पूर्णाङ्क szChunk;            /* Chunk size configured by FCNTL_CHUNK_SIZE */
#यदि SQLITE_MAX_MMAP_आकार>0
  पूर्णाङ्क n_Fetch_बहिर्गमनम्;                /* Number of outstanding xFetch references */
  HANDLE hMap;                  /* Handle for accessing memory mapping */
  शून्य *pMapRegion;             /* Area memory mapped */
  sqlite3_int64 mmap_आकार;       /* Size of mapped region */
  sqlite3_int64 mmap_आकार_Max;    /* Configured FCNTL_MMAP_SIZE value */
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_ENABLE_SETLK_TIMEOUT
  DWORD iBusyTimeout;        /* Wait this many millisec on locks */
  पूर्णाङ्क b_खण्डम्_On_Connect;
#यदि_अन्तः
};

#यदि_परिभाषितम् SQLITE_ENABLE_SETLK_TIMEOUT
# define winFileBusyTimeout(pDbFd) pDbFd->iBusyTimeout
#अन्यथा
# define winFileBusyTimeout(pDbFd) 0
#यदि_अन्तः

/*
** The winVfsAppData structure is used for the pAppData member for all of the
** Win32 VFS variants.
*/
प्रकारनिर्देश संरचना win_Vfs_App_दत्त win_Vfs_App_दत्त;
संरचना win_Vfs_App_दत्त {
  ध्रुव sqlite3_io_methods *p_विधानम्; /* The file I/O methods to use. */
  शून्य *p_App_दत्त;                    /* The extra pAppData, if any. */
  BOOL b_No_तालकम्;                      /* Non-zero if locking is disabled. */
};

/*
** Allowed values for winFile.ctrlFlags
*/
#परिभाषय WINFILE_RDONLY          0x02   /* Connection is read only */
#परिभाषय WINFILE_PERSIST_WAL     0x04   /* Persistent WAL mode */
#परिभाषय WINFILE_PSOW            0x10   /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */

/*
 * The size of the buffer used by sqlite3_win32_write_debug().
 */
#यदि_अपरिभाषितम् SQLITE_WIN32_DBG_BUF_आकार
#  define SQLITE_WIN32_DBG_BUF_आकार   ((पूर्णाङ्क)(4096-परिमाण(DWORD)))
#यदि_अन्तः

/*
 * If compiled with SQLITE_WIN32_MALLOC on Windows, we will use the
 * various Win32 API heap functions instead of our own.
 */
#यदि_परिभाषितम् SQLITE_WIN32_MALLOC

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
#यदि_अपरिभाषितम् SQLITE_WIN32_HEAP_CREATE
#  define SQLITE_WIN32_HEAP_CREATE        (TRUE)
#यदि_अन्तः

/*
 * This is the maximum possible initial size of the Win32-specific heap, in
 * bytes.
 */
#यदि_अपरिभाषितम् SQLITE_WIN32_HEAP_MAX_INIT_आकार
#  define SQLITE_WIN32_HEAP_MAX_INIT_आकार (4294967295U)
#यदि_अन्तः

/*
 * This is the extra space for the initial size of the Win32-specific heap,
 * in bytes.  This value may be zero.
 */
#यदि_अपरिभाषितम् SQLITE_WIN32_HEAP_INIT_EXTRA
#  define SQLITE_WIN32_HEAP_INIT_EXTRA  (4194304)
#यदि_अन्तः

/*
 * Calculate the maximum legal cache size, in pages, based on the maximum
 * possible initial heap size and the default page size, setting aside the
 * needed extra space.
 */
#यदि_अपरिभाषितम् SQLITE_WIN32_MAX_CACHE_आकार
#  define SQLITE_WIN32_MAX_CACHE_आकार   (((SQLITE_WIN32_HEAP_MAX_INIT_आकार) - \
                                          (SQLITE_WIN32_HEAP_INIT_EXTRA)) / \
                                         (SQLITE_DEFAULT_PAGE_आकार))
#यदि_अन्तः

/*
 * This is cache size used in the calculation of the initial size of the
 * Win32-specific heap.  It cannot be negative.
 */
#यदि_अपरिभाषितम् SQLITE_WIN32_CACHE_आकार
#  यदि SQLITE_DEFAULT_CACHE_आकार>=0
#    define SQLITE_WIN32_CACHE_आकार     (SQLITE_DEFAULT_CACHE_आकार)
#  अन्यथा
#    define SQLITE_WIN32_CACHE_आकार     (-(SQLITE_DEFAULT_CACHE_आकार))
#  endif
#यदि_अन्तः

/*
 * Make sure that the calculated cache size, in pages, cannot cause the
 * initial size of the Win32-specific heap to exceed the maximum amount
 * of memory that can be specified in the call to HeapCreate.
 */
#यदि SQLITE_WIN32_CACHE_आकार>SQLITE_WIN32_MAX_CACHE_आकार
#  undef SQLITE_WIN32_CACHE_आकार
#  define SQLITE_WIN32_CACHE_आकार       (2000)
#यदि_अन्तः

/*
 * The initial size of the Win32-specific heap.  This value may be zero.
 */
#यदि_अपरिभाषितम् SQLITE_WIN32_HEAP_INIT_आकार
#  define SQLITE_WIN32_HEAP_INIT_आकार   ((SQLITE_WIN32_CACHE_आकार) * \
                                         (SQLITE_DEFAULT_PAGE_आकार) + \
                                         (SQLITE_WIN32_HEAP_INIT_EXTRA))
#यदि_अन्तः

/*
 * The maximum size of the Win32-specific heap.  This value may be zero.
 */
#यदि_अपरिभाषितम् SQLITE_WIN32_HEAP_MAX_आकार
#  define SQLITE_WIN32_HEAP_MAX_आकार    (0)
#यदि_अन्तः

/*
 * The extra flags to use in calls to the Win32 heap APIs.  This value may be
 * zero for the default behavior.
 */
#यदि_अपरिभाषितम् SQLITE_WIN32_HEAP_FLAGS
#  define SQLITE_WIN32_HEAP_FLAGS       (0)
#यदि_अन्तः


/*
** The winMemData structure stores information required by the Win32-specific
** sqlite3_mem_methods implementation.
*/
प्रकारनिर्देश संरचना win_Mem_दत्त win_Mem_दत्त;
संरचना win_Mem_दत्त {
#यदि_अपरिभाषितम् NDEBUG
  u32 magic1;   /* Magic number to detect structure corruption. */
#यदि_अन्तः
  HANDLE hHeap; /* The handle to our heap. */
  BOOL bOwned;  /* Do we own the heap (i.e. destroy it on shutdown)? */
#यदि_अपरिभाषितम् NDEBUG
  u32 magic2;   /* Magic number to detect structure corruption. */
#यदि_अन्तः
};

#यदि_अपरिभाषितम् NDEBUG
#परिभाषय WINMEM_MAGIC1     0x42b2830b
#परिभाषय WINMEM_MAGIC2     0xbd4d7cf4
#यदि_अन्तः

स्थिर संरचना win_Mem_दत्त win_mem_दत्त = {
#यदि_अपरिभाषितम् NDEBUG
  WINMEM_MAGIC1,
#यदि_अन्तः
  रिक्त, FALSE
#यदि_अपरिभाषितम् NDEBUG
  ,WINMEM_MAGIC2
#यदि_अन्तः
};

#यदि_अपरिभाषितम् NDEBUG
#परिभाषय winMemAssertMagic1() प्रतिज्ञा( win_mem_दत्त.magic1==WINMEM_MAGIC1 )
#परिभाषय winMemAssertMagic2() प्रतिज्ञा( win_mem_दत्त.magic2==WINMEM_MAGIC2 )
#परिभाषय winMemAssertMagic()  winMemAssertMagic1(); winMemAssertMagic2();
#अन्यथा
#परिभाषय winMemAssertMagic()
#यदि_अन्तः

#परिभाषय win_Mem_Get_दत्त_Ptr()  &win_mem_दत्त
#परिभाषय winMemGetHeap()     win_mem_दत्त.hHeap
#परिभाषय winMemGetOwned()    win_mem_दत्त.bOwned

स्थिर शून्य *winMemMalloc(पूर्णाङ्क n_अष्टकानि);
स्थिर शून्य winMemFree(शून्य *p_प्राक्);
स्थिर शून्य *winMemRealloc(शून्य *p_प्राक्, पूर्णाङ्क n_अष्टकानि);
स्थिर पूर्णाङ्क win_Mem_आकार(शून्य *p);
स्थिर पूर्णाङ्क winMemRoundup(पूर्णाङ्क n);
स्थिर पूर्णाङ्क winMemInit(शून्य *p_App_दत्त);
स्थिर शून्य winMemShutdown(शून्य *p_App_दत्त);

ध्रुव sqlite3_mem_methods *sqlite3MemGetWin32(शून्य);
#यदि_अन्तः /* SQLITE_WIN32_MALLOC */

#यदि_अपरिभाषितम् SYSCALL
#  define SYSCALL sqlite3_syscall_ptr
#यदि_अन्तः

/*
** Many system calls are accessed through pointer-to-functions so that
** they may be overridden at runtime to facilitate fault injection during
** testing and sandboxing.  The following array holds the names and pointers
** to all overrideable system calls.
*/
स्थिर संरचना win_syscall {
  ध्रुव वर्ण *zName;            /* Name of the system call */
  sqlite3_syscall_ptr pCurrent; /* Current value of the system call */
  sqlite3_syscall_ptr pDefault; /* Default value */
} aSyscall[] = {
  { "AreFileApisANSI",         (SYSCALL)AreFileApisANSI,         0 },
#परिभाषय osAreFileApisANSI ((BOOL(WINAPI*)(VOID))aSyscall[0].pCurrent)

  { "CloseHandle",             (SYSCALL)CloseHandle,             0 },
#परिभाषय osCloseHandle ((BOOL(WINAPI*)(HANDLE))aSyscall[1].pCurrent)


  { "CreateFileW",             (SYSCALL)CreateFileW,             0 },
#परिभाषय osCreateFileW ((HANDLE(WINAPI*)(LPCWSTR,DWORD,DWORD, \
        LPSECURITY_गुणाः,DWORD,DWORD,HANDLE))aSyscall[2].pCurrent)

#यदि defined(SQLITE_UWP)
  { "CreateFileMappingFromApp",(SYSCALL)CreateFileMappingFromApp,0 },
#अन्यथा
  { "CreateFileMappingFromApp",(SYSCALL)0,                       0 },
#यदि_अन्तः
#परिभाषय osCreateFileMappingFromApp ((HANDLE(WINAPI*)(HANDLE,\
        PSECURITY_गुणाः, \
        ULONG,ULONG64,PCWSTR))aSyscall[3].pCurrent)

#यदि !defined(SQLITE_UWP)
  { "CreateFileMappingW",      (SYSCALL)CreateFileMappingW,      0 },
#अन्यथा
  { "CreateFileMappingW",      (SYSCALL)0,                       0 },
#यदि_अन्तः
#परिभाषय osCreateFileMappingW ((HANDLE(WINAPI*)(HANDLE,LPSECURITY_गुणाः, \
        DWORD,DWORD,DWORD,LPCWSTR))aSyscall[4].pCurrent)

  { "DeleteFileW",             (SYSCALL)DeleteFileW,             0 },
#परिभाषय osDeleteFileW ((BOOL(WINAPI*)(LPCWSTR))aSyscall[5].pCurrent)

  { "FlushFileBuffers",        (SYSCALL)FlushFileBuffers,        0 },
#परिभाषय osFlushFileBuffers ((BOOL(WINAPI*)(HANDLE))aSyscall[6].pCurrent)

  { "FormatMessageW",          (SYSCALL)Format_सन्देशः_W,          0 },
#परिभाषय os_Format_सन्देशः_W ((DWORD(WINAPI*)(DWORD,LPCVOID,DWORD,DWORD,LPWSTR, \
        DWORD,va_list*))aSyscall[7].pCurrent)

#यदि !defined(SQLITE_OMIT_आरोपय_EXTENSION) && !defined(SQLITE_UWP)
  { "FreeLibrary",             (SYSCALL)FreeLibrary,             0 },
#अन्यथा
  { "FreeLibrary",             (SYSCALL)0,                       0 },
#यदि_अन्तः

#परिभाषय osFreeLibrary ((BOOL(WINAPI*)(HMODULE))aSyscall[8].pCurrent)

  { "GetCurrentProcessId",     (SYSCALL)GetCurrentProcessId,     0 },
#परिभाषय osGetCurrentProcessId ((DWORD(WINAPI*)(VOID))aSyscall[9].pCurrent)

  { "GetFileAttributesW",      (SYSCALL)Get_File_गुणाः_W,      0 },
#परिभाषय os_Get_File_गुणाः_W ((DWORD(WINAPI*)(LPCWSTR))aSyscall[10].pCurrent)

  { "GetFileAttributesExW",    (SYSCALL)Get_File_गुणाः_Ex_W,    0 },
#परिभाषय os_Get_File_गुणाः_Ex_W ((BOOL(WINAPI*)(LPCWSTR,GET_FILEEX_INFO_स्तराः, \
        LPVOID))aSyscall[11].pCurrent)

  { "GetFileSizeEx",           (SYSCALL)Get_File_आकार_Ex,           0 },
#परिभाषय os_Get_File_आकार_Ex ((BOOL(WINAPI*)(HANDLE, \
                        PLARGE_INTEGER))aSyscall[12].pCurrent)

  { "GetFullPathNameW",        (SYSCALL)Get_पूर्णम्_मार्गः_Name_W,        0 },
#परिभाषय os_Get_पूर्णम्_मार्गः_Name_W ((DWORD(WINAPI*)(LPCWSTR,DWORD,LPWSTR, \
        LPWSTR*))aSyscall[13].pCurrent)

  { "GetLastError",            (SYSCALL)Get_चरमः_Error,            0 },
#परिभाषय os_Get_चरमः_Error ((DWORD(WINAPI*)(VOID))aSyscall[14].pCurrent)

#यदि !defined(SQLITE_OMIT_आरोपय_EXTENSION) && !defined(SQLITE_UWP)
  { "GetProcAddressA",         (SYSCALL)Get_Proc_स्थानाङ्कः,          0 },
#अन्यथा
  { "GetProcAddressA",         (SYSCALL)0,                       0 },
#यदि_अन्तः
#परिभाषय os_Get_Proc_स्थानाङ्कः_A ((FARPROC(WINAPI*)(HMODULE, \
        LPCSTR))aSyscall[15].pCurrent)

  { "GetSystemInfo",           (SYSCALL)GetSystemInfo,           0 },
#परिभाषय osGetSystemInfo ((VOID(WINAPI*)(LPSYSTEM_INFO))aSyscall[16].pCurrent)

  { "GetSystemTimeAsFileTime", (SYSCALL)GetSystemTimeAsFileTime, 0 },
#परिभाषय osGetSystemTimeAsFileTime ((VOID(WINAPI*)( \
        LPFILETIME))aSyscall[17].pCurrent)

#यदि_परिभाषितम् SQLITE_UWP
  { "GetTempPathW",            (SYSCALL)0,                       0 },
#अन्यथा
  { "GetTempPathW",            (SYSCALL)Get_अस्थायी_मार्गः_W,            0 },
#यदि_अन्तः
#परिभाषय os_Get_अस्थायी_मार्गः_W ((DWORD(WINAPI*)(DWORD,LPWSTR))aSyscall[18].pCurrent)

  { "GetTickCount64",          (SYSCALL)GetTickCount64,          0 },
#परिभाषय osGetTickCount64 ((ULONGLONG(WINAPI*)(VOID))aSyscall[19].pCurrent)

#यदि_परिभाषितम् SQLITE_UWP
  { "HeapAlloc",               (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapAlloc",               (SYSCALL)HeapAlloc,               0 },
#यदि_अन्तः
#परिभाषय osHeapAlloc ((LPVOID(WINAPI*)(HANDLE,DWORD, \
        आकार_T))aSyscall[20].pCurrent)

#यदि_परिभाषितम् SQLITE_UWP
  { "HeapCreate",              (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapCreate",              (SYSCALL)HeapCreate,              0 },
#यदि_अन्तः
#परिभाषय osHeapCreate ((HANDLE(WINAPI*)(DWORD,आकार_T, \
        आकार_T))aSyscall[21].pCurrent)

#यदि_परिभाषितम् SQLITE_UWP
  { "HeapDestroy",             (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapDestroy",             (SYSCALL)HeapDestroy,             0 },
#यदि_अन्तः
#परिभाषय osHeapDestroy ((BOOL(WINAPI*)(HANDLE))aSyscall[22].pCurrent)

#यदि_परिभाषितम् SQLITE_UWP
  { "HeapFree",                (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapFree",                (SYSCALL)HeapFree,                0 },
#यदि_अन्तः
#परिभाषय osHeapFree ((BOOL(WINAPI*)(HANDLE,DWORD,LPVOID))aSyscall[23].pCurrent)

#यदि_परिभाषितम् SQLITE_UWP
  { "HeapReAlloc",             (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapReAlloc",             (SYSCALL)HeapReAlloc,             0 },
#यदि_अन्तः
#परिभाषय osHeapReAlloc ((LPVOID(WINAPI*)(HANDLE,DWORD,LPVOID, \
        आकार_T))aSyscall[24].pCurrent)

#यदि_परिभाषितम् SQLITE_UWP
  { "HeapSize",                (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapSize",                (SYSCALL)Heap_आकार,                0 },
#यदि_अन्तः
#परिभाषय os_Heap_आकार ((आकार_T(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[25].pCurrent)

#यदि_परिभाषितम् SQLITE_UWP
  { "HeapValidate",            (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapValidate",            (SYSCALL)HeapValidate,            0 },
#यदि_अन्तः
#परिभाषय osHeapValidate ((BOOL(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[26].pCurrent)

#यदि_परिभाषितम् SQLITE_UWP
  { "HeapCompact",             (SYSCALL)0,                       0 },
#अन्यथा
  { "HeapCompact",             (SYSCALL)HeapCompact,             0 },
#यदि_अन्तः
#परिभाषय osHeapCompact ((UINT(WINAPI*)(HANDLE,DWORD))aSyscall[27].pCurrent)


#यदि !defined(SQLITE_OMIT_आरोपय_EXTENSION) && !defined(SQLITE_UWP)
  { "LoadLibraryW",            (SYSCALL)आरोपय_Library_W,            0 },
#अन्यथा
  { "LoadLibraryW",            (SYSCALL)0,                       0 },
#यदि_अन्तः
#परिभाषय os_आरोपय_Library_W ((HMODULE(WINAPI*)(LPCWSTR))aSyscall[28].pCurrent)

  { "LocalFree",               (SYSCALL)LocalFree,               0 },
#परिभाषय osLocalFree ((HLOCAL(WINAPI*)(HLOCAL))aSyscall[29].pCurrent)

  { "LockFileEx",              (SYSCALL)तालकम्_File_Ex,              0 },
#परिभाषय os_तालकम्_File_Ex ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[30].pCurrent)

#यदि (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_आकार>0) \
 && !defined(SQLITE_UWP)
  { "MapViewOfFile",           (SYSCALL)Map_View_सम्बन्धि_File,           0 },
#अन्यथा
  { "MapViewOfFile",           (SYSCALL)0,                       0 },
#यदि_अन्तः
#परिभाषय os_Map_View_सम्बन्धि_File ((LPVOID(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        आकार_T))aSyscall[31].pCurrent)

#यदि (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_आकार>0) \
 && defined(SQLITE_UWP)
  { "MapViewOfFileFromApp",    (SYSCALL)Map_View_सम्बन्धि_File_From_App,   0 },
#अन्यथा
  { "MapViewOfFileFromApp",    (SYSCALL)0,                      0 },
#यदि_अन्तः
#परिभाषय os_Map_View_सम्बन्धि_File_From_App ((LPVOID(WINAPI*)(HANDLE,ULONG,ULONG64, \
        आकार_T))aSyscall[32].pCurrent)

  { "MultiByteToWideChar",     (SYSCALL)Multi_Byte_गन्तव्यम्_Wide_Char,     0 },
#परिभाषय os_Multi_Byte_गन्तव्यम्_Wide_Char ((पूर्णाङ्क(WINAPI*)(UINT,DWORD,LPCSTR,पूर्णाङ्क,LPWSTR, \
        पूर्णाङ्क))aSyscall[33].pCurrent)

  { "QueryPerformanceCounter", (SYSCALL)Query_Performance_गणक, 0 },
#परिभाषय os_Query_Performance_गणक ((BOOL(WINAPI*)( \
        LARGE_INTEGER*))aSyscall[34].pCurrent)

  { "ReadFile",                (SYSCALL)ReadFile,                0 },
#परिभाषय osReadFile ((BOOL(WINAPI*)(HANDLE,LPVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[35].pCurrent)

  { "SetEndOfFile",            (SYSCALL)समुच्चयः_अन्तः_सम्बन्धि_File,            0 },
#परिभाषय os_समुच्चयः_अन्तः_सम्बन्धि_File ((BOOL(WINAPI*)(HANDLE))aSyscall[36].pCurrent)

  { "SetFilePointerEx",        (SYSCALL)समुच्चयः_File_Pointer_Ex,        0 },
#परिभाषय os_समुच्चयः_File_Pointer_Ex ((BOOL(WINAPI*)(HANDLE,LARGE_INTEGER,\
        PLARGE_INTEGER,DWORD))aSyscall[37].pCurrent)

  { "Sleep",                   (SYSCALL)Sleep,                   0 },
#परिभाषय osSleep ((VOID(WINAPI*)(DWORD))aSyscall[38].pCurrent)

  { "UnlockFileEx",            (SYSCALL)UnlockFileEx,            0 },
#परिभाषय osUnlockFileEx ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[39].pCurrent)

#यदि !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_आकार>0
  { "UnmapViewOfFile",         (SYSCALL)Unmap_View_सम्बन्धि_File,         0 },
#अन्यथा
  { "UnmapViewOfFile",         (SYSCALL)0,                       0 },
#यदि_अन्तः
#परिभाषय os_Unmap_View_सम्बन्धि_File ((BOOL(WINAPI*)(LPCVOID))aSyscall[40].pCurrent)

  { "WideCharToMultiByte",     (SYSCALL)Wide_Char_गन्तव्यम्_Multi_Byte,     0 },
#परिभाषय os_Wide_Char_गन्तव्यम्_Multi_Byte ((पूर्णाङ्क(WINAPI*)(UINT,DWORD,LPCWSTR,पूर्णाङ्क,LPSTR,पूर्णाङ्क, \
        LPCSTR,LPBOOL))aSyscall[41].pCurrent)

  { "WriteFile",               (SYSCALL)WriteFile,               0 },
#परिभाषय osWriteFile ((BOOL(WINAPI*)(HANDLE,LPCVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[42].pCurrent)

  { "WaitForSingleObject",     (SYSCALL)WaitForSingleObject,     0 },
#परिभाषय osWaitForSingleObject ((DWORD(WINAPI*)(HANDLE, \
        DWORD))aSyscall[43].pCurrent)

  { "WaitForSingleObjectEx",   (SYSCALL)WaitForSingleObjectEx,   0 },
#परिभाषय osWaitForSingleObjectEx ((DWORD(WINAPI*)(HANDLE,DWORD, \
        BOOL))aSyscall[44].pCurrent)

  { "OutputDebugStringA",      (SYSCALL)OutputDebugStringA,      0 },
#परिभाषय osOutputDebugStringA ((VOID(WINAPI*)(LPCSTR))aSyscall[45].pCurrent)

  { "GetProcessHeap",          (SYSCALL)GetProcessHeap,          0 },
#परिभाषय osGetProcessHeap ((HANDLE(WINAPI*)(VOID))aSyscall[46].pCurrent)

/*
** NOTE: On some sub-platforms, the InterlockedCompareExchange "function"
**       is really just a macro that uses a compiler intrinsic (e.g. x64).
**       So do not try to make this is into a redefinable interface.
*/
#यदि defined(InterlockedCompareExchange)
  { "InterlockedCompareExchange", (SYSCALL)0,                    0 },
#परिभाषय osInterlockedCompareExchange InterlockedCompareExchange
#अन्यथा
  { "InterlockedCompareExchange", (SYSCALL)InterlockedCompareExchange, 0 },
#परिभाषय osInterlockedCompareExchange ((LONG(WINAPI*)(LONG चञ्चल*,\
        LONG,LONG))aSyscall[47].pCurrent)
#यदि_अन्तः /* defined(InterlockedCompareExchange) */

#यदि SQLITE_WIN32_USE_UUID
  { "UuidCreate",               (SYSCALL)UuidCreate,             0 },
#अन्यथा
  { "UuidCreate",               (SYSCALL)0,                      0 },
#यदि_अन्तः
#परिभाषय osUuidCreate ((RPC_स्थितिः(RPC_प्रविष्टिः*)(UUID*))aSyscall[48].pCurrent)

#यदि SQLITE_WIN32_USE_UUID
  { "UuidCreateSequential",     (SYSCALL)UuidCreateSequential,   0 },
#अन्यथा
  { "UuidCreateSequential",     (SYSCALL)0,                      0 },
#यदि_अन्तः
#परिभाषय osUuidCreateSequential \
        ((RPC_स्थितिः(RPC_प्रविष्टिः*)(UUID*))aSyscall[49].pCurrent)

#यदि !defined(SQLITE_NO_SYNC) && SQLITE_MAX_MMAP_आकार>0
  { "FlushViewOfFile",          (SYSCALL)Flush_View_सम्बन्धि_File,        0 },
#अन्यथा
  { "FlushViewOfFile",          (SYSCALL)0,                      0 },
#यदि_अन्तः
#परिभाषय os_Flush_View_सम्बन्धि_File \
        ((BOOL(WINAPI*)(LPCVOID,आकार_T))aSyscall[50].pCurrent)

#यदि_परिभाषितम् SQLITE_ENABLE_SETLK_TIMEOUT
  { "CreateEvent",              (SYSCALL)CreateEvent,            0 },
#अन्यथा
  { "CreateEvent",              (SYSCALL)0,                      0 },
#यदि_अन्तः
#परिभाषय osCreateEvent ((HANDLE(WINAPI*)(LPSECURITY_गुणाः,BOOL, \
                        BOOL,LPCSTR))aSyscall[51].pCurrent)

#यदि_परिभाषितम् SQLITE_ENABLE_SETLK_TIMEOUT
  { "CancelIo",                 (SYSCALL)CancelIo,               0 },
#अन्यथा
  { "CancelIo",                 (SYSCALL)0,                      0 },
#यदि_अन्तः
#परिभाषय osCancelIo ((BOOL(WINAPI*)(HANDLE))aSyscall[52].pCurrent)

#यदि_अपरिभाषितम् _WIN32
  { "getenv",                   (SYSCALL)getenv,                 0 },
#अन्यथा
  { "getenv",                   (SYSCALL)0,                      0 },
#यदि_अन्तः
#परिभाषय osGetenv ((ध्रुव वर्ण *(*)(ध्रुव वर्ण *))aSyscall[53].pCurrent)

#यदि_अपरिभाषितम् _WIN32
  { "getcwd",                   (SYSCALL)getcwd,                 0 },
#अन्यथा
  { "getcwd",                   (SYSCALL)0,                      0 },
#यदि_अन्तः
#परिभाषय osGetcwd ((वर्ण*(*)(वर्ण*,size_t))aSyscall[54].pCurrent)

#यदि_अपरिभाषितम् _WIN32
  { "readlink",                 (SYSCALL)readlink,               0 },
#अन्यथा
  { "readlink",                 (SYSCALL)0,                      0 },
#यदि_अन्तः
#परिभाषय osReadlink ((ssize_t(*)(ध्रुव वर्ण*,वर्ण*,size_t))aSyscall[55].pCurrent)

#यदि_अपरिभाषितम् _WIN32
  { "lstat",                    (SYSCALL)lstat,                  0 },
#अन्यथा
  { "lstat",                    (SYSCALL)0,                      0 },
#यदि_अन्तः
#परिभाषय osLstat ((पूर्णाङ्क(*)(ध्रुव वर्ण*,संरचना stat*))aSyscall[56].pCurrent)

#यदि_अपरिभाषितम् _WIN32
  { "__errno",                  (SYSCALL)__errno,                0 },
#अन्यथा
  { "__errno",                  (SYSCALL)0,                      0 },
#यदि_अन्तः
#परिभाषय osErrno (*((पूर्णाङ्क*(*)(शून्य))aSyscall[57].pCurrent)())

#यदि_अपरिभाषितम् _WIN32
  { "cygwin_conv_path",         (SYSCALL)cygwin_conv_मार्गः,       0 },
#अन्यथा
  { "cygwin_conv_path",         (SYSCALL)0,                      0 },
#यदि_अन्तः
#परिभाषय os_Cygwin_conv_मार्गः ((size_t(*)(अचिह्नित पूर्णाङ्क, \
    ध्रुव शून्य *, शून्य *, size_t))aSyscall[58].pCurrent)

  { "GetDriveTypeW",            (SYSCALL)GetDriveTypeW,          0 },
#परिभाषय osGetDriveTypeW ((UINT(WINAPI*)(LPCWSTR))aSyscall[59].pCurrent)

}; /* End of the overrideable system calls */

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "win32" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
स्थिर पूर्णाङ्क win_समुच्चयः_System_Call(
  sqlite3_vfs *p_Not_प्रयुक्तम्,        /* The VFS pointer.  Not used */
  ध्रुव वर्ण *zName,            /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
){
  अचिह्नित पूर्णाङ्क i;
  पूर्णाङ्क rc = SQLITE_NOTFOUND;

  अप्रयुक्तम्_PARAMETER(p_Not_प्रयुक्तम्);
  यदि( zName==0 ){
    /* If no zName is given, restore all system calls to their default
    ** settings and return NULL
    */
    rc = SQLITE_OK;
    प्रति(i=0; i<परिमाण(aSyscall)/परिमाण(aSyscall[0]); i++){
      यदि( aSyscall[i].pDefault ){
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  }अन्यथा{
    /* If zName is specified, operate on only the one system call
    ** specified.
    */
    प्रति(i=0; i<परिमाण(aSyscall)/परिमाण(aSyscall[0]); i++){
      यदि( वाक्यं_तुलय(zName, aSyscall[i].zName)==0 ){
        यदि( aSyscall[i].pDefault==0 ){
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = SQLITE_OK;
        यदि( pNewFunc==0 ) pNewFunc = aSyscall[i].pDefault;
        aSyscall[i].pCurrent = pNewFunc;
        भङ्ग;
      }
    }
  }
  प्रत्यागम rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
स्थिर sqlite3_syscall_ptr winGetSystemCall(
  sqlite3_vfs *p_Not_प्रयुक्तम्,
  ध्रुव वर्ण *zName
){
  अचिह्नित पूर्णाङ्क i;

  अप्रयुक्तम्_PARAMETER(p_Not_प्रयुक्तम्);
  प्रति(i=0; i<परिमाण(aSyscall)/परिमाण(aSyscall[0]); i++){
    यदि( वाक्यं_तुलय(zName, aSyscall[i].zName)==0 ) प्रत्यागम aSyscall[i].pCurrent;
  }
  प्रत्यागम 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
स्थिर ध्रुव वर्ण *win_अग्र_System_Call(sqlite3_vfs *p, ध्रुव वर्ण *zName){
  पूर्णाङ्क i = -1;

  अप्रयुक्तम्_PARAMETER(p);
  यदि( zName ){
    प्रति(i=0; i<Array_आकार(aSyscall)-1; i++){
      यदि( वाक्यं_तुलय(zName, aSyscall[i].zName)==0 ) भङ्ग;
    }
  }
  प्रति(i++; i<Array_आकार(aSyscall); i++){
    यदि( aSyscall[i].pCurrent!=0 ) प्रत्यागम aSyscall[i].zName;
  }
  प्रत्यागम 0;
}

#यदि_परिभाषितम् SQLITE_WIN32_MALLOC
#यदि_परिभाषितम् SQLITE_UWP
# error SQLITE_WIN32_MALLOC is incompatible with SQLITE_UWP
#यदि_अन्तः
/*
** If a Win32 native heap has been configured, this function will attempt to
** compact it.  Upon success, SQLITE_OK will be returned.  Upon failure, one
** of SQLITE_NOMEM, SQLITE_ERROR, or SQLITE_NOTFOUND will be returned.  The
** "pnLargest" argument, if non-zero, will be used to return the size of the
** largest committed free block in the heap, in bytes.
*/
पूर्णाङ्क sqlite3_win32_compact_heap(LPUINT pnLargest){
  पूर्णाङ्क rc = SQLITE_OK;
  UINT nLargest = 0;
  HANDLE hHeap;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  प्रतिज्ञा( hHeap!=0 );
  प्रतिज्ञा( hHeap!=INVALID_HANDLE_मानम् );
#यदि defined(SQLITE_WIN32_MALLOC_VALIDATE)
  प्रतिज्ञा( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, रिक्त) );
#यदि_अन्तः
  यदि( (nLargest=osHeapCompact(hHeap, SQLITE_WIN32_HEAP_FLAGS))==0 ){
    DWORD चरमः_Errno = os_Get_चरमः_Error();
    यदि( चरमः_Errno==NO_ERROR ){
      sqlite3_log(SQLITE_NOMEM, "failed to HeapCompact (no space), heap=%p",
                  (शून्य*)hHeap);
      rc = SQLITE_NOMEM_BKPT;
    }अन्यथा{
      sqlite3_log(SQLITE_ERROR, "failed to HeapCompact (%lu), heap=%p",
                  os_Get_चरमः_Error(), (शून्य*)hHeap);
      rc = SQLITE_ERROR;
    }
  }
  यदि( pnLargest ) *pnLargest = nLargest;
  प्रत्यागम rc;
}

/*
** If a Win32 native heap has been configured, this function will attempt to
** destroy and recreate it.  If the Win32 native heap is not isolated and/or
** the sqlite3_memory_used() function does not return zero, SQLITE_BUSY will
** be returned and no changes will be made to the Win32 native heap.
*/
पूर्णाङ्क sqlite3_win32_reset_heap(){
  पूर्णाङ्क rc;
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; ) /* The main static mutex */
  MUTEX_LOGIC( sqlite3_mutex *pMem; )    /* The memsys static mutex */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  MUTEX_LOGIC( pMem = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3_mutex_enter(pMem);
  winMemAssertMagic();
  यदि( winMemGetHeap()!=रिक्त && winMemGetOwned() && sqlite3_स्मृतिः_प्रयुक्तम्()==0 ){
    /*
    ** At this point, there should be no outstanding memory allocations on
    ** the heap.  Also, since both the main and memsys locks are currently
    ** being held by us, no other function (i.e. from another thread) should
    ** be able to even access the heap.  Attempt to destroy and recreate our
    ** isolated Win32 native heap now.
    */
    प्रतिज्ञा( winMemGetHeap()!=रिक्त );
    प्रतिज्ञा( winMemGetOwned() );
    प्रतिज्ञा( sqlite3_स्मृतिः_प्रयुक्तम्()==0 );
    winMemShutdown(win_Mem_Get_दत्त_Ptr());
    प्रतिज्ञा( winMemGetHeap()==रिक्त );
    प्रतिज्ञा( !winMemGetOwned() );
    प्रतिज्ञा( sqlite3_स्मृतिः_प्रयुक्तम्()==0 );
    rc = winMemInit(win_Mem_Get_दत्त_Ptr());
    प्रतिज्ञा( rc!=SQLITE_OK || winMemGetHeap()!=रिक्त );
    प्रतिज्ञा( rc!=SQLITE_OK || winMemGetOwned() );
    प्रतिज्ञा( rc!=SQLITE_OK || sqlite3_स्मृतिः_प्रयुक्तम्()==0 );
  }अन्यथा{
    /*
    ** The Win32 native heap cannot be modified because it may be in use.
    */
    rc = SQLITE_BUSY;
  }
  sqlite3_mutex_leave(pMem);
  sqlite3_mutex_leave(pMainMtx);
  प्रत्यागम rc;
}
#यदि_अन्तः /* SQLITE_WIN32_MALLOC */

#यदि_परिभाषितम् _WIN32
/*
** This function outputs the specified (ANSI) string to the Win32 debugger
** (if available).  Undocumented.  Might go away at any moment.
*/
शून्य sqlite3_win32_write_debug(ध्रुव वर्ण *zBuf, पूर्णाङ्क nBuf){
  वर्ण zDbgBuf[SQLITE_WIN32_DBG_BUF_आकार];
  पूर्णाङ्क nMin = MIN(nBuf, (SQLITE_WIN32_DBG_BUF_आकार - 1)); /* may be negative. */
  यदि( nMin<-1 ) nMin = -1; /* all negative values become -1. */
  प्रतिज्ञा( nMin==-1 || nMin==0 || nMin<SQLITE_WIN32_DBG_BUF_आकार );
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( !zBuf ){
    (शून्य)SQLITE_MISUSE_BKPT;
    प्रत्यागम;
  }
#यदि_अन्तः
  यदि( nMin>0 ){
    स्मृतिं_पूरय(zDbgBuf, 0, SQLITE_WIN32_DBG_BUF_आकार);
    स्मृतिं_प्रतिलिख(zDbgBuf, zBuf, nMin);
    osOutputDebugStringA(zDbgBuf);
  }अन्यथा{
    osOutputDebugStringA(zBuf);
  }
}

शून्य sqlite3_win32_sleep(DWORD milliseconds){
  osSleep(milliseconds);
}

#यदि SQLITE_MAX_कार्यकर्ता_तन्तवः>0 && SQLITE_THREADSAFE>0
DWORD sqlite3Win32Wait(HANDLE hObject){
  DWORD rc;
  यावत्( (rc = osWaitForSingleObjectEx(hObject, INFINITE,
                                       TRUE))==WAIT_IO_COMPLETION ){}
  प्रत्यागम rc;
}
#यदि_अन्तः
#यदि_अन्तः /* _WIN32 */

#यदि_परिभाषितम् SQLITE_WIN32_MALLOC
/*
** Allocate nBytes of memory.
*/
स्थिर शून्य *winMemMalloc(पूर्णाङ्क n_अष्टकानि){
  HANDLE hHeap;
  शून्य *p;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  प्रतिज्ञा( hHeap!=0 );
  प्रतिज्ञा( hHeap!=INVALID_HANDLE_मानम् );
#यदि defined(SQLITE_WIN32_MALLOC_VALIDATE)
  प्रतिज्ञा( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, रिक्त) );
#यदि_अन्तः
  प्रतिज्ञा( n_अष्टकानि>=0 );
  p = osHeapAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, (आकार_T)n_अष्टकानि);
  यदि( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapAlloc %u bytes (%lu), heap=%p",
                n_अष्टकानि, os_Get_चरमः_Error(), (शून्य*)hHeap);
  }
  प्रत्यागम p;
}

/*
** Free memory.
*/
स्थिर शून्य winMemFree(शून्य *p_प्राक्){
  HANDLE hHeap;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  प्रतिज्ञा( hHeap!=0 );
  प्रतिज्ञा( hHeap!=INVALID_HANDLE_मानम् );
#यदि defined(SQLITE_WIN32_MALLOC_VALIDATE)
  प्रतिज्ञा( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, p_प्राक्) );
#यदि_अन्तः
  यदि( !p_प्राक् ) प्रत्यागम; /* Passing NULL to HeapFree is undefined. */
  यदि( !osHeapFree(hHeap, SQLITE_WIN32_HEAP_FLAGS, p_प्राक्) ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapFree block %p (%lu), heap=%p",
                p_प्राक्, os_Get_चरमः_Error(), (शून्य*)hHeap);
  }
}

/*
** Change the size of an existing memory allocation
*/
स्थिर शून्य *winMemRealloc(शून्य *p_प्राक्, पूर्णाङ्क n_अष्टकानि){
  HANDLE hHeap;
  शून्य *p;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  प्रतिज्ञा( hHeap!=0 );
  प्रतिज्ञा( hHeap!=INVALID_HANDLE_मानम् );
#यदि defined(SQLITE_WIN32_MALLOC_VALIDATE)
  प्रतिज्ञा( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, p_प्राक्) );
#यदि_अन्तः
  प्रतिज्ञा( n_अष्टकानि>=0 );
  यदि( !p_प्राक् ){
    p = osHeapAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, (आकार_T)n_अष्टकानि);
  }अन्यथा{
    p = osHeapReAlloc(hHeap, SQLITE_WIN32_HEAP_FLAGS, p_प्राक्, (आकार_T)n_अष्टकानि);
  }
  यदि( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to %s %u bytes (%lu), heap=%p",
                p_प्राक् ? "HeapReAlloc" : "HeapAlloc", n_अष्टकानि, os_Get_चरमः_Error(),
                (शून्य*)hHeap);
  }
  प्रत्यागम p;
}

/*
** Return the size of an outstanding allocation, in bytes.
*/
स्थिर पूर्णाङ्क win_Mem_आकार(शून्य *p){
  HANDLE hHeap;
  आकार_T n;

  winMemAssertMagic();
  hHeap = winMemGetHeap();
  प्रतिज्ञा( hHeap!=0 );
  प्रतिज्ञा( hHeap!=INVALID_HANDLE_मानम् );
#यदि defined(SQLITE_WIN32_MALLOC_VALIDATE)
  प्रतिज्ञा( osHeapValidate(hHeap, SQLITE_WIN32_HEAP_FLAGS, p) );
#यदि_अन्तः
  यदि( !p ) प्रत्यागम 0;
  n = os_Heap_आकार(hHeap, SQLITE_WIN32_HEAP_FLAGS, p);
  यदि( n==(आकार_T)-1 ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapSize block %p (%lu), heap=%p",
                p, os_Get_चरमः_Error(), (शून्य*)hHeap);
    प्रत्यागम 0;
  }
  प्रत्यागम (पूर्णाङ्क)n;
}

/*
** Round up a request size to the next valid allocation size.
*/
स्थिर पूर्णाङ्क winMemRoundup(पूर्णाङ्क n){
  प्रत्यागम n;
}

/*
** Initialize this module.
*/
स्थिर पूर्णाङ्क winMemInit(शून्य *p_App_दत्त){
  win_Mem_दत्त *p_Win_Mem_दत्त = (win_Mem_दत्त *)p_App_दत्त;

  यदि( !p_Win_Mem_दत्त ) प्रत्यागम SQLITE_ERROR;
  प्रतिज्ञा( p_Win_Mem_दत्त->magic1==WINMEM_MAGIC1 );
  प्रतिज्ञा( p_Win_Mem_दत्त->magic2==WINMEM_MAGIC2 );

#यदि SQLITE_WIN32_HEAP_CREATE
  यदि( !p_Win_Mem_दत्त->hHeap ){
    DWORD dw_Initial_आकार = SQLITE_WIN32_HEAP_INIT_आकार;
    DWORD dw_अधिकतमम्_आकार = (DWORD)sqlite3GlobalConfig.nHeap;
    यदि( dw_अधिकतमम्_आकार==0 ){
      dw_अधिकतमम्_आकार = SQLITE_WIN32_HEAP_MAX_आकार;
    }अन्यथा यदि( dw_Initial_आकार>dw_अधिकतमम्_आकार ){
      dw_Initial_आकार = dw_अधिकतमम्_आकार;
    }
    p_Win_Mem_दत्त->hHeap = osHeapCreate(SQLITE_WIN32_HEAP_FLAGS,
                                      dw_Initial_आकार, dw_अधिकतमम्_आकार);
    यदि( !p_Win_Mem_दत्त->hHeap ){
      sqlite3_log(SQLITE_NOMEM,
          "failed to HeapCreate (%lu), flags=%u, initSize=%lu, maxSize=%lu",
          os_Get_चरमः_Error(), SQLITE_WIN32_HEAP_FLAGS, dw_Initial_आकार,
          dw_अधिकतमम्_आकार);
      प्रत्यागम SQLITE_NOMEM_BKPT;
    }
    p_Win_Mem_दत्त->bOwned = TRUE;
    प्रतिज्ञा( p_Win_Mem_दत्त->bOwned );
  }
#अन्यथा
  p_Win_Mem_दत्त->hHeap = osGetProcessHeap();
  यदि( !p_Win_Mem_दत्त->hHeap ){
    sqlite3_log(SQLITE_NOMEM,
        "failed to GetProcessHeap (%lu)", os_Get_चरमः_Error());
    प्रत्यागम SQLITE_NOMEM_BKPT;
  }
  p_Win_Mem_दत्त->bOwned = FALSE;
  प्रतिज्ञा( !p_Win_Mem_दत्त->bOwned );
#यदि_अन्तः
  प्रतिज्ञा( p_Win_Mem_दत्त->hHeap!=0 );
  प्रतिज्ञा( p_Win_Mem_दत्त->hHeap!=INVALID_HANDLE_मानम् );
#यदि defined(SQLITE_WIN32_MALLOC_VALIDATE)
  प्रतिज्ञा( osHeapValidate(p_Win_Mem_दत्त->hHeap, SQLITE_WIN32_HEAP_FLAGS, रिक्त) );
#यदि_अन्तः
  प्रत्यागम SQLITE_OK;
}

/*
** Deinitialize this module.
*/
स्थिर शून्य winMemShutdown(शून्य *p_App_दत्त){
  win_Mem_दत्त *p_Win_Mem_दत्त = (win_Mem_दत्त *)p_App_दत्त;

  यदि( !p_Win_Mem_दत्त ) प्रत्यागम;
  प्रतिज्ञा( p_Win_Mem_दत्त->magic1==WINMEM_MAGIC1 );
  प्रतिज्ञा( p_Win_Mem_दत्त->magic2==WINMEM_MAGIC2 );

  यदि( p_Win_Mem_दत्त->hHeap ){
    प्रतिज्ञा( p_Win_Mem_दत्त->hHeap!=INVALID_HANDLE_मानम् );
#यदि defined(SQLITE_WIN32_MALLOC_VALIDATE)
    प्रतिज्ञा( osHeapValidate(p_Win_Mem_दत्त->hHeap, SQLITE_WIN32_HEAP_FLAGS, रिक्त) );
#यदि_अन्तः
    यदि( p_Win_Mem_दत्त->bOwned ){
      यदि( !osHeapDestroy(p_Win_Mem_दत्त->hHeap) ){
        sqlite3_log(SQLITE_NOMEM, "failed to HeapDestroy (%lu), heap=%p",
                    os_Get_चरमः_Error(), (शून्य*)p_Win_Mem_दत्त->hHeap);
      }
      p_Win_Mem_दत्त->bOwned = FALSE;
    }
    p_Win_Mem_दत्त->hHeap = रिक्त;
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
ध्रुव sqlite3_mem_methods *sqlite3MemGetWin32(शून्य){
  स्थिर ध्रुव sqlite3_mem_methods winMemMethods = {
    winMemMalloc,
    winMemFree,
    winMemRealloc,
    win_Mem_आकार,
    winMemRoundup,
    winMemInit,
    winMemShutdown,
    &win_mem_दत्त
  };
  प्रत्यागम &winMemMethods;
}

शून्य sqlite3_Mem_समुच्चयः_Default(शून्य){
  sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetWin32());
}
#यदि_अन्तः /* SQLITE_WIN32_MALLOC */

#यदि_परिभाषितम् _WIN32
/*
** Convert a UTF-8 string to Microsoft Unicode.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
स्थिर LPWSTR win_Utf8_गन्तव्यम्_Unicode(ध्रुव वर्ण *zText){
  पूर्णाङ्क nChar;
  LPWSTR zWideText;

  nChar = os_Multi_Byte_गन्तव्यम्_Wide_Char(CP_UTF8, 0, zText, -1, रिक्त, 0);
  यदि( nChar==0 ){
    प्रत्यागम 0;
  }
  zWideText = sqlite3MallocZero( nChar*परिमाण(WCHAR) );
  यदि( zWideText==0 ){
    प्रत्यागम 0;
  }
  nChar = os_Multi_Byte_गन्तव्यम्_Wide_Char(CP_UTF8, 0, zText, -1, zWideText,
                                nChar);
  यदि( nChar==0 ){
    sqlite3_free(zWideText);
    zWideText = 0;
  }
  प्रत्यागम zWideText;
}
#यदि_अन्तः /* _WIN32 */

/*
** Convert a Microsoft Unicode string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
स्थिर वर्ण *win_Unicode_गन्तव्यम्_Utf8(LPCWSTR zWideText){
  पूर्णाङ्क nByte;
  वर्ण *zText;

  nByte = os_Wide_Char_गन्तव्यम्_Multi_Byte(CP_UTF8, 0, zWideText, -1, 0, 0, 0, 0);
  यदि( nByte == 0 ){
    प्रत्यागम 0;
  }
  zText = sqlite3MallocZero( nByte );
  यदि( zText==0 ){
    प्रत्यागम 0;
  }
  nByte = os_Wide_Char_गन्तव्यम्_Multi_Byte(CP_UTF8, 0, zWideText, -1, zText, nByte,
                                0, 0);
  यदि( nByte == 0 ){
    sqlite3_free(zText);
    zText = 0;
  }
  प्रत्यागम zText;
}

#यदि_परिभाषितम् _WIN32
/*
** Convert an ANSI string to Microsoft Unicode, using the ANSI or OEM
** code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
स्थिर LPWSTR win_Mbcs_गन्तव्यम्_Unicode(ध्रुव वर्ण *zText, पूर्णाङ्क useAnsi){
  पूर्णाङ्क nWideChar;
  LPWSTR zMbcsText;
  पूर्णाङ्क codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nWideChar = os_Multi_Byte_गन्तव्यम्_Wide_Char(codepage, 0, zText, -1, रिक्त,
                                0);
  यदि( nWideChar==0 ){
    प्रत्यागम 0;
  }
  zMbcsText = sqlite3MallocZero( nWideChar*परिमाण(WCHAR) );
  यदि( zMbcsText==0 ){
    प्रत्यागम 0;
  }
  nWideChar = os_Multi_Byte_गन्तव्यम्_Wide_Char(codepage, 0, zText, -1, zMbcsText,
                                nWideChar);
  यदि( nWideChar==0 ){
    sqlite3_free(zMbcsText);
    zMbcsText = 0;
  }
  प्रत्यागम zMbcsText;
}

/*
** Convert a Microsoft Unicode string to a multi-byte character string,
** using the ANSI or OEM code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
स्थिर वर्ण *win_Unicode_गन्तव्यम्_Mbcs(LPCWSTR zWideText, पूर्णाङ्क useAnsi){
  पूर्णाङ्क nByte;
  वर्ण *zText;
  पूर्णाङ्क codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nByte = os_Wide_Char_गन्तव्यम्_Multi_Byte(codepage, 0, zWideText, -1, 0, 0, 0, 0);
  यदि( nByte == 0 ){
    प्रत्यागम 0;
  }
  zText = sqlite3MallocZero( nByte );
  यदि( zText==0 ){
    प्रत्यागम 0;
  }
  nByte = os_Wide_Char_गन्तव्यम्_Multi_Byte(codepage, 0, zWideText, -1, zText,
                                nByte, 0, 0);
  यदि( nByte == 0 ){
    sqlite3_free(zText);
    zText = 0;
  }
  प्रत्यागम zText;
}

/*
** Convert a multi-byte character string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
स्थिर वर्ण *win_Mbcs_गन्तव्यम्_Utf8(ध्रुव वर्ण *zText, पूर्णाङ्क useAnsi){
  वर्ण *zTextUtf8;
  LPWSTR zTmpWide;

  zTmpWide = win_Mbcs_गन्तव्यम्_Unicode(zText, useAnsi);
  यदि( zTmpWide==0 ){
    प्रत्यागम 0;
  }
  zTextUtf8 = win_Unicode_गन्तव्यम्_Utf8(zTmpWide);
  sqlite3_free(zTmpWide);
  प्रत्यागम zTextUtf8;
}

/*
** Convert a UTF-8 string to a multi-byte character string.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
स्थिर वर्ण *win_Utf8_गन्तव्यम्_Mbcs(ध्रुव वर्ण *zText, पूर्णाङ्क useAnsi){
  वर्ण *zTextMbcs;
  LPWSTR zTmpWide;

  zTmpWide = win_Utf8_गन्तव्यम्_Unicode(zText);
  यदि( zTmpWide==0 ){
    प्रत्यागम 0;
  }
  zTextMbcs = win_Unicode_गन्तव्यम्_Mbcs(zTmpWide, useAnsi);
  sqlite3_free(zTmpWide);
  प्रत्यागम zTextMbcs;
}

/*
** This is a public wrapper for the winUtf8ToUnicode() function.
*/
LPWSTR sqlite3_win32_utf8_गन्तव्यम्_unicode(ध्रुव वर्ण *zText){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( !zText ){
    (शून्य)SQLITE_MISUSE_BKPT;
    प्रत्यागम 0;
  }
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  यदि( sqlite3_initialize() ) प्रत्यागम 0;
#यदि_अन्तः
  प्रत्यागम win_Utf8_गन्तव्यम्_Unicode(zText);
}

/*
** This is a public wrapper for the winUnicodeToUtf8() function.
*/
वर्ण *sqlite3_win32_unicode_गन्तव्यम्_utf8(LPCWSTR zWideText){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( !zWideText ){
    (शून्य)SQLITE_MISUSE_BKPT;
    प्रत्यागम 0;
  }
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  यदि( sqlite3_initialize() ) प्रत्यागम 0;
#यदि_अन्तः
  प्रत्यागम win_Unicode_गन्तव्यम्_Utf8(zWideText);
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
वर्ण *sqlite3_win32_mbcs_गन्तव्यम्_utf8(ध्रुव वर्ण *zText){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( !zText ){
    (शून्य)SQLITE_MISUSE_BKPT;
    प्रत्यागम 0;
  }
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  यदि( sqlite3_initialize() ) प्रत्यागम 0;
#यदि_अन्तः
  प्रत्यागम win_Mbcs_गन्तव्यम्_Utf8(zText, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
वर्ण *sqlite3_win32_mbcs_गन्तव्यम्_utf8_v2(ध्रुव वर्ण *zText, पूर्णाङ्क useAnsi){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( !zText ){
    (शून्य)SQLITE_MISUSE_BKPT;
    प्रत्यागम 0;
  }
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  यदि( sqlite3_initialize() ) प्रत्यागम 0;
#यदि_अन्तः
  प्रत्यागम win_Mbcs_गन्तव्यम्_Utf8(zText, useAnsi);
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
वर्ण *sqlite3_win32_utf8_गन्तव्यम्_mbcs(ध्रुव वर्ण *zText){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( !zText ){
    (शून्य)SQLITE_MISUSE_BKPT;
    प्रत्यागम 0;
  }
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  यदि( sqlite3_initialize() ) प्रत्यागम 0;
#यदि_अन्तः
  प्रत्यागम win_Utf8_गन्तव्यम्_Mbcs(zText, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
वर्ण *sqlite3_win32_utf8_गन्तव्यम्_mbcs_v2(ध्रुव वर्ण *zText, पूर्णाङ्क useAnsi){
#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( !zText ){
    (शून्य)SQLITE_MISUSE_BKPT;
    प्रत्यागम 0;
  }
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  यदि( sqlite3_initialize() ) प्रत्यागम 0;
#यदि_अन्तः
  प्रत्यागम win_Utf8_गन्तव्यम्_Mbcs(zText, useAnsi);
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-8 string.
*/
पूर्णाङ्क sqlite3_win32_समुच्चयः_directory8(
  अचिह्नित दीर्घ type, /* Identifier for directory being set or reset */
  ध्रुव वर्ण *z_मानम्  /* New value for directory being set or reset */
){
  वर्ण **ppDirectory = 0;
  पूर्णाङ्क rc;
#यदि_अपरिभाषितम् SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  यदि( rc ) प्रत्यागम rc;
#यदि_अन्तः
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  यदि( type==SQLITE_WIN32_दत्त_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_दत्त_directory;
  }अन्यथा यदि( type==SQLITE_WIN32_अस्थायी_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_अस्थायी_directory;
  }
  प्रतिज्ञा( !ppDirectory || type==SQLITE_WIN32_दत्त_DIRECTORY_TYPE
          || type==SQLITE_WIN32_अस्थायी_DIRECTORY_TYPE
  );
  प्रतिज्ञा( !ppDirectory || sqlite3MemdebugHasType(*ppDirectory, MEMTYPE_HEAP) );
  यदि( ppDirectory ){
    वर्ण *zCopy = 0;
    यदि( z_मानम् && z_मानम्[0] ){
      zCopy = sqlite3_mprintf("%s", z_मानम्);
      यदि ( zCopy==0 ){
        rc = SQLITE_NOMEM_BKPT;
        गच्छ समुच्चयः_directory8_समाप्तम्;
      }
    }
    sqlite3_free(*ppDirectory);
    *ppDirectory = zCopy;
    rc = SQLITE_OK;
  }अन्यथा{
    rc = SQLITE_ERROR;
  }
समुच्चयः_directory8_समाप्तम्:
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  प्रत्यागम rc;
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-16 string.
*/
पूर्णाङ्क sqlite3_win32_समुच्चयः_directory16(
  अचिह्नित दीर्घ type, /* Identifier for directory being set or reset */
  ध्रुव शून्य *z_मानम्  /* New value for directory being set or reset */
){
  पूर्णाङ्क rc;
  वर्ण *zUtf8 = 0;
  यदि( z_मानम् ){
    zUtf8 = sqlite3_win32_unicode_गन्तव्यम्_utf8(z_मानम्);
    यदि( zUtf8==0 ) प्रत्यागम SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_win32_समुच्चयः_directory8(type, zUtf8);
  यदि( zUtf8 ) sqlite3_free(zUtf8);
  प्रत्यागम rc;
}

/*
** This function sets the data directory or the temporary directory based on
** the provided arguments.  The type argument must be 1 in order to set the
** data directory or 2 in order to set the temporary directory.  The zValue
** argument is the name of the directory to use.  The return value will be
** SQLITE_OK if successful.
*/
पूर्णाङ्क sqlite3_win32_समुच्चयः_directory(
  अचिह्नित दीर्घ type, /* Identifier for directory being set or reset */
  शून्य *z_मानम्        /* New value for directory being set or reset */
){
  प्रत्यागम sqlite3_win32_समुच्चयः_directory16(type, z_मानम्);
}
#यदि_अन्तः /* _WIN32 */

/*
** The return value of winGetLastErrorMsg
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated).
*/
स्थिर पूर्णाङ्क win_Get_चरमः_Error_सन्देशः(DWORD चरमः_Errno, पूर्णाङ्क nBuf, वर्ण *zBuf){
  /* FormatMessage returns 0 on failure.  Otherwise it
  ** returns the number of TCHARs written to the output
  ** buffer, excluding the terminating null char.
  */
  DWORD dwLen = 0;
  वर्ण *z_बहिर्गमनम् = 0;
  LPWSTR z_अस्थायी_Wide = रिक्त;
  dwLen = os_Format_सन्देशः_W(FORMAT_सन्देशः_ALLOCATE_बफर |
                           FORMAT_सन्देशः_FROM_SYSTEM |
                           FORMAT_सन्देशः_IGNORE_INSERTS,
                           रिक्त,
                           चरमः_Errno,
                           0,
                           (LPWSTR) &z_अस्थायी_Wide,
                           0,
                           0);
  यदि( dwLen > 0 ){
    /* allocate a buffer and convert to UTF8 */
    sqlite3BeginBenignMalloc();
    z_बहिर्गमनम् = win_Unicode_गन्तव्यम्_Utf8(z_अस्थायी_Wide);
    sqlite3_अन्तः_Benign_Malloc();
    /* free the system buffer allocated by FormatMessage */
    osLocalFree(z_अस्थायी_Wide);
  }
  यदि( 0 == dwLen ){
    sqlite3_snprintf(nBuf, zBuf, "OsError 0x%lx (%lu)", चरमः_Errno, चरमः_Errno);
  }अन्यथा{
    /* copy a maximum of nBuf chars to output buffer */
    sqlite3_snprintf(nBuf, zBuf, "%s", z_बहिर्गमनम्);
    /* free the UTF8 buffer */
    sqlite3_free(z_बहिर्गमनम्);
  }
  प्रत्यागम 0;
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
#परिभाषय winLogError(a,b,c,d)   win_Log_Error_At_पङ्क्तिः(a,b,c,d,__LINE__)
स्थिर पूर्णाङ्क win_Log_Error_At_पङ्क्तिः(
  पूर्णाङ्क errcode,                    /* SQLite error code */
  DWORD चरमः_Errno,                /* Win32 last error */
  ध्रुव वर्ण *zFunc,              /* Name of OS function that failed */
  ध्रुव वर्ण *z_मार्गः,              /* File path associated with error */
  पूर्णाङ्क i_पङ्क्तिः                       /* Source line number where error occurred */
){
  वर्ण z_सन्देशः[500];                 /* Human readable error text */
  पूर्णाङ्क i;                          /* Loop counter */

  z_सन्देशः[0] = 0;
  win_Get_चरमः_Error_सन्देशः(चरमः_Errno, परिमाण(z_सन्देशः), z_सन्देशः);
  प्रतिज्ञा( errcode!=SQLITE_OK );
  यदि( z_मार्गः==0 ) z_मार्गः = "";
  प्रति(i=0; z_सन्देशः[i] && z_सन्देशः[i]!='\r' && z_सन्देशः[i]!='\n'; i++){}
  z_सन्देशः[i] = 0;
  sqlite3_log(errcode,
      "os_win.c:%d: (%lu) %s(%s) - %s",
      i_पङ्क्तिः, चरमः_Errno, zFunc, z_मार्गः, z_सन्देशः
  );

  प्रत्यागम errcode;
}

/*
** The number of times that a ReadFile(), WriteFile(), and DeleteFile()
** will be retried following a locking error - probably caused by
** antivirus software.  Also the initial delay before the first retry.
** The delay increases linearly with each retry.
*/
#यदि_अपरिभाषितम् SQLITE_WIN32_IOERR_RETRY
# define SQLITE_WIN32_IOERR_RETRY 10
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_WIN32_IOERR_RETRY_DELAY
# define SQLITE_WIN32_IOERR_RETRY_DELAY 25
#यदि_अन्तः
स्थिर पूर्णाङ्क winIoerrRetry = SQLITE_WIN32_IOERR_RETRY;
स्थिर पूर्णाङ्क winIoerrRetryDelay = SQLITE_WIN32_IOERR_RETRY_DELAY;

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
#यदि !defined(winIoerrCanRetry1)
#परिभाषय winIoerrCanRetry1(a) (((a)==ERROR_ACCESS_DENIED)        || \
                              ((a)==ERROR_SHARING_VIOLATION)    || \
                              ((a)==ERROR_तालकम्_VIOLATION)       || \
                              ((a)==ERROR_DEV_NOT_EXIST)        || \
                              ((a)==ERROR_NETNAME_DELETED)      || \
                              ((a)==ERROR_SEM_TIMEOUT)          || \
                              ((a)==ERROR_जालम्_UNREACHABLE))
#यदि_अन्तः

/*
** If a ReadFile() or WriteFile() error occurs, invoke this routine
** to see if it should be retried.  Return TRUE to retry.  Return FALSE
** to give up with an error.
*/
स्थिर पूर्णाङ्क winRetryIoerr(पूर्णाङ्क *pnRetry, DWORD *pError){
  DWORD e = os_Get_चरमः_Error();
  यदि( *pnRetry>=winIoerrRetry ){
    यदि( pError ){
      *pError = e;
    }
    प्रत्यागम 0;
  }
  यदि( winIoerrCanRetry1(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    प्रत्यागम 1;
  }
#यदि defined(winIoerrCanRetry2)
  अन्यथा यदि( winIoerrCanRetry2(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    प्रत्यागम 1;
  }
#यदि_अन्तः
  यदि( pError ){
    *pError = e;
  }
  प्रत्यागम 0;
}

/*
** Log a I/O error retry episode.
*/
स्थिर शून्य winLogIoerr(पूर्णाङ्क nRetry, पूर्णाङ्क lineno){
  यदि( nRetry ){
    sqlite3_log(SQLITE_NOTICE,
      "delayed %dms for lock/sharing conflict at line %d",
      winIoerrRetryDelay*nRetry*(nRetry+1)/2, lineno
    );
  }
}


/*
** Lock a file region.
*/
स्थिर BOOL win_तालकम्_File(
  LPHANDLE phFile,
  DWORD flags,
  DWORD विस्थापनम्_नीचः,
  DWORD विस्थापनम्_उच्चः,
  DWORD अङ्कः_अष्टकानि_नीचः,
  DWORD अङ्कः_अष्टकानि_उच्चः
){
  OVERLAPPED ovlp;
  स्मृतिं_पूरय(&ovlp, 0, परिमाण(OVERLAPPED));
  ovlp.विस्थापनम्_2 = विस्थापनम्_नीचः;
  ovlp.विस्थापनम्_उच्चः_2 = विस्थापनम्_उच्चः;
  प्रत्यागम os_तालकम्_File_Ex(*phFile, flags, 0, अङ्कः_अष्टकानि_नीचः, अङ्कः_अष्टकानि_उच्चः, &ovlp);
}

#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
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
स्थिर पूर्णाङ्क win_Handle_तालकम्_Timeout(
  HANDLE hFile,
  DWORD विस्थापनम्,
  DWORD nByte,
  पूर्णाङ्क bExcl,
  DWORD nMs
){
  DWORD flags = LOCKFILE_FAIL_IMMEDIATELY | (bExcl?LOCKFILE_EXCLUSIVE_तालकम्:0);
  पूर्णाङ्क rc = SQLITE_OK;
  BOOL ret;
  OVERLAPPED ovlp;
  स्मृतिं_पूरय(&ovlp, 0, परिमाण(OVERLAPPED));
  ovlp.विस्थापनम्_2 = विस्थापनम्;

#यदि defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  यदि( nMs!=0 ){
    flags &= ~LOCKFILE_FAIL_IMMEDIATELY;
  }
  ovlp.hEvent = osCreateEvent(रिक्त, TRUE, FALSE, रिक्त);
  यदि( ovlp.hEvent==रिक्त ){
    प्रत्यागम SQLITE_IOERR_तालकम्;
  }
#यदि_अन्तः
  ret = os_तालकम्_File_Ex(hFile, flags, 0, nByte, 0, &ovlp);
#यदि defined(SQLITE_ENABLE_SETLK_TIMEOUT)
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
  यदि( !ret && Get_चरमः_Error()==ERROR_IO_PENDING ){
    DWORD nDelay = (nMs==0 ? INFINITE : nMs);
    DWORD res = osWaitForSingleObject(ovlp.hEvent, nDelay);
    यदि( res==WAIT_OBJECT_0 ){
      ret = TRUE;
    }अन्यथा यदि( res==WAIT_TIMEOUT ){
#यदि SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = SQLITE_BUSY_TIMEOUT;
#अन्यथा
      rc = SQLITE_BUSY;
#यदि_अन्तः
    }अन्यथा{
      /* Some other error has occurred */
      rc = SQLITE_IOERR_तालकम्;
    }

    /* If it is still pending, cancel the LockFileEx() call. */
    osCancelIo(hFile);
  }

  osCloseHandle(ovlp.hEvent);
#यदि_अन्तः /* defined(SQLITE_ENABLE_SETLK_TIMEOUT) */

  यदि( rc==SQLITE_OK && !ret ){
    rc = SQLITE_BUSY;
  }
  प्रत्यागम rc;
}
#यदि_अन्तः /* #ifndef SQLITE_OMIT_WAL */

/*
** Unlock a file region.
 */
स्थिर BOOL winUnlockFile(
  LPHANDLE phFile,
  DWORD विस्थापनम्_नीचः,
  DWORD विस्थापनम्_उच्चः,
  DWORD अङ्कः_अष्टकानि_नीचः,
  DWORD अङ्कः_अष्टकानि_उच्चः
){
  OVERLAPPED ovlp;
  स्मृतिं_पूरय(&ovlp, 0, परिमाण(OVERLAPPED));
  ovlp.विस्थापनम्_2 = विस्थापनम्_नीचः;
  ovlp.विस्थापनम्_उच्चः_2 = विस्थापनम्_उच्चः;
  प्रत्यागम osUnlockFileEx(*phFile, 0, अङ्कः_अष्टकानि_नीचः, अङ्कः_अष्टकानि_उच्चः, &ovlp);
}

#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
/*
** Remove an nByte lock starting at offset iOff from HANDLE h.
*/
स्थिर पूर्णाङ्क winHandleUnlock(HANDLE h, पूर्णाङ्क iOff, पूर्णाङ्क nByte){
  BOOL ret = winUnlockFile(&h, iOff, 0, nByte, 0);
  प्रत्यागम (ret ? SQLITE_OK : SQLITE_IOERR_UNLOCK);
}
#यदि_अन्तः

/*****************************************************************************
** The next group of routines implement the I/O methods specified
** by the sqlite3_io_methods object.
******************************************************************************/

/*
** Some Microsoft compilers lack this definition.
*/
#यदि_अपरिभाषितम् INVALID_समुच्चयः_FILE_POINTER
# define INVALID_समुच्चयः_FILE_POINTER ((DWORD)-1)
#यदि_अन्तः

/*
** Seek the file handle h to offset nByte of the file.
**
** If successful, return SQLITE_OK. Or, if an error occurs, return an SQLite
** error code.
*/
स्थिर पूर्णाङ्क winHandleSeek(HANDLE h, sqlite3_int64 i_विस्थापनम्){
  पूर्णाङ्क rc = SQLITE_OK;             /* Return value */
  LARGE_INTEGER x;                /* The offset */

  x.QuadPart = i_विस्थापनम्;
  यदि( os_समुच्चयः_File_Pointer_Ex(h, x, 0, FILE_BEGIN)==0 ){
    rc = SQLITE_IOERR_SEEK;
  }
  OSTRACE(("SEEK file=%p, offset=%lld rc=%s\n", h, i_विस्थापनम्,sqlite3ErrName(rc)));
  प्रत्यागम rc;
}

/*
** Move the current position of the file handle passed as the first
** argument to offset iOffset within the file. If successful, return 0.
** Otherwise, set pFile->lastErrno and return non-zero.
*/
स्थिर पूर्णाङ्क winSeekFile(winFile *pFile, sqlite3_int64 i_विस्थापनम्){
  पूर्णाङ्क rc;

  rc = winHandleSeek(pFile->h, i_विस्थापनम्);
  यदि( rc!=SQLITE_OK ){
    pFile->चरमः_Errno = os_Get_चरमः_Error();
    winLogError(rc, pFile->चरमः_Errno, "winSeekFile", pFile->z_मार्गः);
  }
  प्रत्यागम rc;
}


#यदि SQLITE_MAX_MMAP_आकार>0
/* Forward references to VFS helper methods used for memory mapped files */
स्थिर पूर्णाङ्क winMapfile(winFile*, sqlite3_int64);
स्थिर पूर्णाङ्क winUnmapfile(winFile*);
#यदि_अन्तः

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
#परिभाषय MX_CLOSE_ATTEMPT 3
स्थिर पूर्णाङ्क winClose(sqlite3_file *id){
  पूर्णाङ्क rc, cnt = 0;
  winFile *pFile = (winFile*)id;

  प्रतिज्ञा( id!=0 );
#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
  प्रतिज्ञा( pFile->pShm==0 );
#यदि_अन्तः
  प्रतिज्ञा( pFile->h!=रिक्त && pFile->h!=INVALID_HANDLE_मानम् );
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p\n",
           osGetCurrentProcessId(), pFile, pFile->h));

#यदि SQLITE_MAX_MMAP_आकार>0
  winUnmapfile(pFile);
#यदि_अन्तः

  कुरु{
    rc = osCloseHandle(pFile->h);
    /* SimulateIOError( rc=0; cnt=MX_CLOSE_ATTEMPT; ); */
  }यावत्( rc==0 && ++cnt < MX_CLOSE_ATTEMPT && (osSleep(100), 1) );
  यदि( rc ){
    pFile->h = रिक्त;
  }
  Open_गणक(-1);
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, rc ? "ok" : "failed"));
  प्रत्यागम rc ? SQLITE_OK
            : winLogError(SQLITE_IOERR_CLOSE, os_Get_चरमः_Error(),
                          "winClose", pFile->z_मार्गः);
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
स्थिर पूर्णाङ्क winRead(
  sqlite3_file *id,          /* File to read from */
  शून्य *pBuf,                /* Write content into this buffer */
  पूर्णाङ्क amt,                   /* Number of bytes to read */
  sqlite3_int64 विस्थापनम्       /* Begin reading at this offset */
){
#यदि !defined(SQLITE_WIN32_NO_OVERLAPPED)
  OVERLAPPED overlapped;          /* The offset for ReadFile. */
#यदि_अन्तः
  winFile *pFile = (winFile*)id;  /* file handle */
  DWORD nRead;                    /* Number of bytes actually read from file */
  पूर्णाङ्क nRetry = 0;                 /* Number of retrys */

  प्रतिज्ञा( id!=0 );
  प्रतिज्ञा( amt>0 );
  प्रतिज्ञा( विस्थापनम्>=0 );
  SimulateIOError(प्रत्यागम SQLITE_IOERR_READ);
  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, विस्थापनम्, pFile->locktype));

#यदि SQLITE_MAX_MMAP_आकार>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  यदि( विस्थापनम्<pFile->mmap_आकार ){
    यदि( विस्थापनम्+amt <= pFile->mmap_आकार ){
      स्मृतिं_प्रतिलिख(pBuf, &((u8 *)(pFile->pMapRegion))[विस्थापनम्], amt);
      OSTRACE(("READ-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      प्रत्यागम SQLITE_OK;
    }अन्यथा{
      पूर्णाङ्क nCopy = (पूर्णाङ्क)(pFile->mmap_आकार - विस्थापनम्);
      स्मृतिं_प्रतिलिख(pBuf, &((u8 *)(pFile->pMapRegion))[विस्थापनम्], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      विस्थापनम् += nCopy;
    }
  }
#यदि_अन्तः

#यदि defined(SQLITE_WIN32_NO_OVERLAPPED)
  यदि( winSeekFile(pFile, विस्थापनम्) ){
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    प्रत्यागम SQLITE_पूर्णम्;
  }
  यावत्( !osReadFile(pFile->h, pBuf, amt, &nRead, 0) ){
#अन्यथा
  स्मृतिं_पूरय(&overlapped, 0, परिमाण(OVERLAPPED));
  overlapped.विस्थापनम्_2 = (LONG)(विस्थापनम् & 0xffffffff);
  overlapped.विस्थापनम्_उच्चः_2 = (LONG)((विस्थापनम्>>32) & 0x7fffffff);
  यावत्( !osReadFile(pFile->h, pBuf, amt, &nRead, &overlapped) &&
         os_Get_चरमः_Error()!=ERROR_HANDLE_EOF ){
#यदि_अन्तः
    DWORD चरमः_Errno;
    यदि( winRetryIoerr(&nRetry, &चरमः_Errno) ) अनुवर्त;
    pFile->चरमः_Errno = चरमः_Errno;
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    प्रत्यागम winLogError(SQLITE_IOERR_READ, pFile->चरमः_Errno,
                       "winRead", pFile->z_मार्गः);
  }
  winLogIoerr(nRetry, __LINE__);
  यदि( nRead<(DWORD)amt ){
    /* Unread parts of the buffer must be zero-filled */
    स्मृतिं_पूरय(&((वर्ण*)pBuf)[nRead], 0, amt-nRead);
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_SHORT_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    प्रत्यागम SQLITE_IOERR_SHORT_READ;
  }

  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  प्रत्यागम SQLITE_OK;
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
स्थिर पूर्णाङ्क winWrite(
  sqlite3_file *id,               /* File to write into */
  ध्रुव शून्य *pBuf,               /* The bytes to be written */
  पूर्णाङ्क amt,                        /* Number of bytes to write */
  sqlite3_int64 विस्थापनम्            /* Offset into the file to begin writing at */
){
  पूर्णाङ्क rc = 0;                     /* True if error has occurred, else false */
  winFile *pFile = (winFile*)id;  /* File handle */
  पूर्णाङ्क nRetry = 0;                 /* Number of retries */

  प्रतिज्ञा( amt>0 );
  प्रतिज्ञा( pFile );
  SimulateIOError(प्रत्यागम SQLITE_IOERR_WRITE);
  SimulateDiskfullError(प्रत्यागम SQLITE_पूर्णम्);

  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, विस्थापनम्, pFile->locktype));

#यदि defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_आकार>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  यदि( विस्थापनम्<pFile->mmap_आकार ){
    यदि( विस्थापनम्+amt <= pFile->mmap_आकार ){
      स्मृतिं_प्रतिलिख(&((u8 *)(pFile->pMapRegion))[विस्थापनम्], pBuf, amt);
      OSTRACE(("WRITE-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      प्रत्यागम SQLITE_OK;
    }अन्यथा{
      पूर्णाङ्क nCopy = (पूर्णाङ्क)(pFile->mmap_आकार - विस्थापनम्);
      स्मृतिं_प्रतिलिख(&((u8 *)(pFile->pMapRegion))[विस्थापनम्], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      विस्थापनम् += nCopy;
    }
  }
#यदि_अन्तः

#यदि defined(SQLITE_WIN32_NO_OVERLAPPED)
  rc = winSeekFile(pFile, विस्थापनम्);
  यदि( rc==0 ){
#अन्यथा
  {
#यदि_अन्तः
#यदि !defined(SQLITE_WIN32_NO_OVERLAPPED)
    OVERLAPPED overlapped;        /* The offset for WriteFile. */
#यदि_अन्तः
    u8 *aRem = (u8 *)pBuf;        /* Data yet to be written */
    पूर्णाङ्क nRem = amt;               /* Number of bytes yet to be written */
    DWORD nWrite;                 /* Bytes written by each WriteFile() call */
    DWORD चरमः_Errno = NO_ERROR;   /* Value returned by GetLastError() */

#यदि !defined(SQLITE_WIN32_NO_OVERLAPPED)
    स्मृतिं_पूरय(&overlapped, 0, परिमाण(OVERLAPPED));
    overlapped.विस्थापनम्_2 = (LONG)(विस्थापनम् & 0xffffffff);
    overlapped.विस्थापनम्_उच्चः_2 = (LONG)((विस्थापनम्>>32) & 0x7fffffff);
#यदि_अन्तः

    यावत्( nRem>0 ){
#यदि defined(SQLITE_WIN32_NO_OVERLAPPED)
      यदि( !osWriteFile(pFile->h, aRem, nRem, &nWrite, 0) ){
#अन्यथा
      यदि( !osWriteFile(pFile->h, aRem, nRem, &nWrite, &overlapped) ){
#यदि_अन्तः
        यदि( winRetryIoerr(&nRetry, &चरमः_Errno) ) अनुवर्त;
        भङ्ग;
      }
      प्रतिज्ञा( nWrite==0 || nWrite<=(DWORD)nRem );
      यदि( nWrite==0 || nWrite>(DWORD)nRem ){
        चरमः_Errno = os_Get_चरमः_Error();
        भङ्ग;
      }
#यदि !defined(SQLITE_WIN32_NO_OVERLAPPED)
      विस्थापनम् += nWrite;
      overlapped.विस्थापनम्_2 = (LONG)(विस्थापनम् & 0xffffffff);
      overlapped.विस्थापनम्_उच्चः_2 = (LONG)((विस्थापनम्>>32) & 0x7fffffff);
#यदि_अन्तः
      aRem += nWrite;
      nRem -= nWrite;
    }
    यदि( nRem>0 ){
      pFile->चरमः_Errno = चरमः_Errno;
      rc = 1;
    }
  }

  यदि( rc ){
    यदि(   ( pFile->चरमः_Errno==ERROR_HANDLE_DISK_पूर्णम् )
       || ( pFile->चरमः_Errno==ERROR_DISK_पूर्णम् )){
      OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      प्रत्यागम winLogError(SQLITE_पूर्णम्, pFile->चरमः_Errno,
                         "winWrite1", pFile->z_मार्गः);
    }
    OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_WRITE\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    प्रत्यागम winLogError(SQLITE_IOERR_WRITE, pFile->चरमः_Errno,
                       "winWrite2", pFile->z_मार्गः);
  }अन्यथा{
    winLogIoerr(nRetry, __LINE__);
  }
  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  प्रत्यागम SQLITE_OK;
}

#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
/*
** Truncate the file opened by handle h to nByte bytes in size.
*/
स्थिर पूर्णाङ्क winHandleTruncate(HANDLE h, sqlite3_int64 nByte){ 
  पूर्णाङ्क rc = SQLITE_OK;             /* Return code */
  rc = winHandleSeek(h, nByte);
  यदि( rc==SQLITE_OK ){
    यदि( 0==os_समुच्चयः_अन्तः_सम्बन्धि_File(h) ){
      rc = SQLITE_IOERR_TRUNCATE;
    }
  }
  प्रत्यागम rc;
}

/*
** Determine the size in bytes of the file opened by the handle passed as 
** the first argument.
*/
स्थिर पूर्णाङ्क win_Handle_आकार(HANDLE h, sqlite3_int64 *pnByte){ 
  पूर्णाङ्क rc = SQLITE_OK;
  LARGE_INTEGER x;
  प्रतिज्ञा( pnByte );
  यदि( os_Get_File_आकार_Ex(h, &x)==0 ){
    rc = SQLITE_IOERR_FSTAT;
    *pnByte = 0;
  }अन्यथा{
    *pnByte = x.QuadPart;
  }
  प्रत्यागम rc;
}

/*
** Close the handle passed as the only argument.
*/
स्थिर शून्य winHandleClose(HANDLE h){
  यदि( h!=INVALID_HANDLE_मानम् ){
    osCloseHandle(h);
  }
}
#यदि_अन्तः /* #ifndef SQLITE_OMIT_WAL */

/*
** Truncate an open file to a specified size
*/
स्थिर पूर्णाङ्क winTruncate(sqlite3_file *id, sqlite3_int64 nByte){
  winFile *pFile = (winFile*)id;  /* File handle object */
  पूर्णाङ्क rc = SQLITE_OK;             /* Return code for this function */
  DWORD चरमः_Errno;
#यदि SQLITE_MAX_MMAP_आकार>0
  sqlite3_int64 old_Mmap_आकार;
  यदि( pFile->n_Fetch_बहिर्गमनम्>0 ){
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
    प्रत्यागम SQLITE_OK;
  }
#यदि_अन्तः

  प्रतिज्ञा( pFile );
  SimulateIOError(प्रत्यागम SQLITE_IOERR_TRUNCATE);
  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, size=%lld, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, nByte, pFile->locktype));

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  यदि( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

#यदि SQLITE_MAX_MMAP_आकार>0
  यदि( pFile->pMapRegion ){
    old_Mmap_आकार = pFile->mmap_आकार;
  }अन्यथा{
    old_Mmap_आकार = 0;
  }
  winUnmapfile(pFile);
#यदि_अन्तः

  /* SetEndOfFile() returns non-zero when successful, or zero when it fails. */
  यदि( winSeekFile(pFile, nByte) ){
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->चरमः_Errno,
                     "winTruncate1", pFile->z_मार्गः);
  }अन्यथा यदि( 0==os_समुच्चयः_अन्तः_सम्बन्धि_File(pFile->h) &&
            ((चरमः_Errno = os_Get_चरमः_Error())!=ERROR_USER_MAPPED_FILE) ){
    pFile->चरमः_Errno = चरमः_Errno;
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->चरमः_Errno,
                     "winTruncate2", pFile->z_मार्गः);
  }

#यदि SQLITE_MAX_MMAP_आकार>0
  यदि( rc==SQLITE_OK && old_Mmap_आकार>0 ){
    यदि( old_Mmap_आकार>nByte ){
      winMapfile(pFile, -1);
    }अन्यथा{
      winMapfile(pFile, old_Mmap_आकार);
    }
  }
#यदि_अन्तः

  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, sqlite3ErrName(rc)));
  प्रत्यागम rc;
}

#यदि_परिभाषितम् SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occurring at the right times.
*/
पूर्णाङ्क sqlite3_sync_गणनम् = 0;
पूर्णाङ्क sqlite3_fullsync_गणनम् = 0;
#यदि_अन्तः

/*
** Make sure all writes to a particular file are committed to disk.
*/
स्थिर पूर्णाङ्क winSync(sqlite3_file *id, पूर्णाङ्क flags){
#यदि_अपरिभाषितम् SQLITE_NO_SYNC
  /*
  ** Used only when SQLITE_NO_SYNC is not defined.
   */
  BOOL rc;
#यदि_अन्तः
#यदि !defined(NDEBUG) || !defined(SQLITE_NO_SYNC) || \
    defined(SQLITE_HAVE_OS_TRACE)
  /*
  ** Used when SQLITE_NO_SYNC is not defined and by the assert() and/or
  ** OSTRACE() macros.
   */
  winFile *pFile = (winFile*)id;
#अन्यथा
  अप्रयुक्तम्_PARAMETER(id);
#यदि_अन्तः

  प्रतिज्ञा( pFile );
  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  प्रतिज्ञा((flags&0x0F)==SQLITE_SYNC_प्रसामान्यम्
      || (flags&0x0F)==SQLITE_SYNC_पूर्णम्
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( प्रत्यागम SQLITE_पूर्णम् );

  OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, flags=%x, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, flags,
           pFile->locktype));

#यदि_अपरिभाषितम् SQLITE_TEST
  अप्रयुक्तम्_PARAMETER(flags);
#अन्यथा
  यदि( (flags&0x0F)==SQLITE_SYNC_पूर्णम् ){
    sqlite3_fullsync_गणनम्++;
  }
  sqlite3_sync_गणनम्++;
#यदि_अन्तः

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op
  */
#यदि_परिभाषितम् SQLITE_NO_SYNC
  OSTRACE(("SYNC-NOP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  प्रत्यागम SQLITE_OK;
#अन्यथा
#यदि SQLITE_MAX_MMAP_आकार>0
  यदि( pFile->pMapRegion ){
    यदि( os_Flush_View_सम्बन्धि_File(pFile->pMapRegion, 0) ){
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_OK\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
    }अन्यथा{
      pFile->चरमः_Errno = os_Get_चरमः_Error();
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
      प्रत्यागम winLogError(SQLITE_IOERR_MMAP, pFile->चरमः_Errno,
                         "winSync1", pFile->z_मार्गः);
    }
  }
#यदि_अन्तः
  rc = osFlushFileBuffers(pFile->h);
  SimulateIOError( rc=FALSE );
  यदि( rc ){
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    प्रत्यागम SQLITE_OK;
  }अन्यथा{
    pFile->चरमः_Errno = os_Get_चरमः_Error();
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_FSYNC\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    प्रत्यागम winLogError(SQLITE_IOERR_FSYNC, pFile->चरमः_Errno,
                       "winSync2", pFile->z_मार्गः);
  }
#यदि_अन्तः
}

/*
** Determine the current size of a file in bytes
*/
स्थिर पूर्णाङ्क win_File_आकार(sqlite3_file *id, sqlite3_int64 *p_आकार){
  winFile *pFile = (winFile*)id;
  पूर्णाङ्क rc = SQLITE_OK;

  प्रतिज्ञा( id!=0 );
  प्रतिज्ञा( p_आकार!=0 );
  SimulateIOError(प्रत्यागम SQLITE_IOERR_FSTAT);
  OSTRACE(("SIZE file=%p, pSize=%p\n", pFile->h, p_आकार));
  {
    LARGE_INTEGER x;
    यदि( os_Get_File_आकार_Ex(pFile->h, &x)==0 ){
      *p_आकार = 0;
      pFile->चरमः_Errno = os_Get_चरमः_Error();
      rc = winLogError(SQLITE_IOERR_FSTAT, pFile->चरमः_Errno,
                       "winFileSize", pFile->z_मार्गः);
    }अन्यथा{
      *p_आकार = x.QuadPart;
    }
  }
  OSTRACE(("SIZE file=%p, pSize=%p, *pSize=%lld, rc=%s\n",
           pFile->h, p_आकार, *p_आकार, sqlite3ErrName(rc)));
  प्रत्यागम rc;
}

/*
** LOCKFILE_FAIL_IMMEDIATELY is undefined on some Windows systems.
*/
#यदि_अपरिभाषितम् LOCKFILE_FAIL_IMMEDIATELY
# define LOCKFILE_FAIL_IMMEDIATELY 1
#यदि_अन्तः

#यदि_अपरिभाषितम् LOCKFILE_EXCLUSIVE_तालकम्
# define LOCKFILE_EXCLUSIVE_तालकम् 2
#यदि_अन्तः

/*
** Historically, SQLite has used both the LockFile and LockFileEx functions.
** When the LockFile function was used, it was always expected to fail
** immediately if the lock could not be obtained.  Also, it always expected to
** obtain an exclusive lock.  These flags are used with the LockFileEx function
** and reflect those expectations; therefore, they should not be changed.
*/
#यदि_अपरिभाषितम् SQLITE_LOCKFILE_FLAGS
# define SQLITE_LOCKFILE_FLAGS   (LOCKFILE_FAIL_IMMEDIATELY | \
                                  LOCKFILE_EXCLUSIVE_तालकम्)
#यदि_अन्तः

/*
** Currently, SQLite never calls the LockFileEx function without wanting the
** call to fail immediately if the lock cannot be obtained.
*/
#यदि_अपरिभाषितम् SQLITE_LOCKFILEEX_FLAGS
# define SQLITE_LOCKFILEEX_FLAGS (LOCKFILE_FAIL_IMMEDIATELY)
#यदि_अन्तः

/*
** Acquire a reader lock.
** Different API routines are called depending on whether or not this
** is Win9x or WinNT.
*/
स्थिर पूर्णाङ्क win_Get_Read_तालकम्(winFile *pFile, पूर्णाङ्क b_खण्डम्){
  पूर्णाङ्क res;
  DWORD आवरणम् = ~(b_खण्डम् ? LOCKFILE_FAIL_IMMEDIATELY : 0);
  OSTRACE(("READ-LOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = win_तालकम्_File(&pFile->h, SQLITE_LOCKFILEEX_FLAGS&आवरणम्, SHARED_FIRST, 0,
                    SHARED_आकार, 0);
  यदि( res == 0 ){
    pFile->चरमः_Errno = os_Get_चरमः_Error();
    /* No need to log a failure to lock */
  }
  OSTRACE(("READ-LOCK file=%p, result=%d\n", pFile->h, res));
  प्रत्यागम res;
}

/*
** Undo a readlock
*/
स्थिर पूर्णाङ्क win_Unlock_Read_तालकम्(winFile *pFile){
  पूर्णाङ्क res;
  DWORD चरमः_Errno;
  OSTRACE(("READ-UNLOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winUnlockFile(&pFile->h, SHARED_FIRST, 0, SHARED_आकार, 0);
  यदि( res==0 && ((चरमः_Errno = os_Get_चरमः_Error())!=ERROR_NOT_LOCKED) ){
    pFile->चरमः_Errno = चरमः_Errno;
    winLogError(SQLITE_IOERR_UNLOCK, pFile->चरमः_Errno,
                "winUnlockReadLock", pFile->z_मार्गः);
  }
  OSTRACE(("READ-UNLOCK file=%p, result=%d\n", pFile->h, res));
  प्रत्यागम res;
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
स्थिर पूर्णाङ्क win_तालकम्(sqlite3_file *id, पूर्णाङ्क locktype){
  पूर्णाङ्क rc = SQLITE_OK;    /* Return code from subroutines */
  पूर्णाङ्क res = 1;           /* Result of a Windows lock call */
  पूर्णाङ्क newLocktype;       /* Set pFile->locktype to this value before exiting */
  पूर्णाङ्क got_Pending_तालकम् = 0;/* True if we acquired a PENDING lock this time */
  winFile *pFile = (winFile*)id;
  DWORD चरमः_Errno = NO_ERROR;

  प्रतिज्ञा( id!=0 );
  OSTRACE(("LOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->shared_तालकम्_Byte, locktype));

  /* If there is already a lock of this type or more restrictive on the
  ** OsFile, do nothing. Don't use the end_lock: exit path, as
  ** sqlite3OsEnterMutex() hasn't been called yet.
  */
  यदि( pFile->locktype>=locktype ){
    OSTRACE(("LOCK-HELD file=%p, rc=SQLITE_OK\n", pFile->h));
    प्रत्यागम SQLITE_OK;
  }

  /* Do not allow any kind of write-lock on a read-only database
  */
  यदि( (pFile->ctrlFlags & WINFILE_RDONLY)!=0 && locktype>=RESERVED_तालकम् ){
    प्रत्यागम SQLITE_IOERR_तालकम्;
  }

  /* Make sure the locking sequence is correct
  */
  प्रतिज्ञा( pFile->locktype!=NO_तालकम् || locktype==SHARED_तालकम् );
  प्रतिज्ञा( locktype!=PENDING_तालकम् );
  प्रतिज्ञा( locktype!=RESERVED_तालकम् || pFile->locktype==SHARED_तालकम् );

  /* Lock the PENDING_LOCK byte if we need to acquire an EXCLUSIVE lock or
  ** a SHARED lock.  If we are acquiring a SHARED lock, the acquisition of
  ** the PENDING_LOCK byte is temporary.
  */
  newLocktype = pFile->locktype;
  यदि( locktype==SHARED_तालकम्
   || (locktype==EXCLUSIVE_तालकम् && pFile->locktype==RESERVED_तालकम्)
  ){
    पूर्णाङ्क cnt = 3;

    /* Flags for the LockFileEx() call. This should be an exclusive lock if
    ** this call is to obtain EXCLUSIVE, or a shared lock if this call is to
    ** obtain SHARED.  */
    पूर्णाङ्क flags = LOCKFILE_FAIL_IMMEDIATELY;
    यदि( locktype==EXCLUSIVE_तालकम् ){
      flags |= LOCKFILE_EXCLUSIVE_तालकम्;
    }
    यावत्( cnt>0 ){
      /* Try 3 times to get the pending lock.  This is needed to work
      ** around problems caused by indexing and/or anti-virus software on
      ** Windows systems.
      **
      ** If you are using this code as a model for alternative VFSes, do not
      ** copy this retry logic.  It is a hack intended for Windows only.  */
      res = win_तालकम्_File(&pFile->h, flags, PENDING_BYTE, 0, 1, 0);
      यदि( res ) भङ्ग;

      चरमः_Errno = os_Get_चरमः_Error();
      OSTRACE(("LOCK-PENDING-FAIL file=%p, count=%d, result=%d\n", 
            pFile->h, cnt, res
      ));

      यदि( चरमः_Errno==ERROR_INVALID_HANDLE ){
        pFile->चरमः_Errno = चरमः_Errno;
        rc = SQLITE_IOERR_तालकम्;
        OSTRACE(("LOCK-FAIL file=%p, count=%d, rc=%s\n", 
              pFile->h, cnt, sqlite3ErrName(rc)
        ));
        प्रत्यागम rc;
      }

      cnt--;
      यदि( cnt>0 ) osSleep(1);
    }
    got_Pending_तालकम् = res;
  }

  /* Acquire a shared lock
  */
  यदि( locktype==SHARED_तालकम् && res ){
    प्रतिज्ञा( pFile->locktype==NO_तालकम् );
#यदि_परिभाषितम् SQLITE_ENABLE_SETLK_TIMEOUT
    res = win_Get_Read_तालकम्(pFile, pFile->b_खण्डम्_On_Connect);
#अन्यथा
    res = win_Get_Read_तालकम्(pFile, 0);
#यदि_अन्तः
    यदि( res ){
      newLocktype = SHARED_तालकम्;
    }अन्यथा{
      चरमः_Errno = os_Get_चरमः_Error();
    }
  }

  /* Acquire a RESERVED lock
  */
  यदि( locktype==RESERVED_तालकम् && res ){
    प्रतिज्ञा( pFile->locktype==SHARED_तालकम् );
    res = win_तालकम्_File(&pFile->h, SQLITE_LOCKFILE_FLAGS, RESERVED_BYTE, 0, 1, 0);
    यदि( res ){
      newLocktype = RESERVED_तालकम्;
    }अन्यथा{
      चरमः_Errno = os_Get_चरमः_Error();
    }
  }

  /* Acquire a PENDING lock
  */
  यदि( locktype==EXCLUSIVE_तालकम् && res ){
    newLocktype = PENDING_तालकम्;
    got_Pending_तालकम् = 0;
  }

  /* Acquire an EXCLUSIVE lock
  */
  यदि( locktype==EXCLUSIVE_तालकम् && res ){
    प्रतिज्ञा( pFile->locktype>=SHARED_तालकम् );
    (शून्य)win_Unlock_Read_तालकम्(pFile);
    res = win_तालकम्_File(&pFile->h, SQLITE_LOCKFILE_FLAGS, SHARED_FIRST, 0,
                      SHARED_आकार, 0);
    यदि( res ){
      newLocktype = EXCLUSIVE_तालकम्;
    }अन्यथा{
      चरमः_Errno = os_Get_चरमः_Error();
      win_Get_Read_तालकम्(pFile, 0);
    }
  }

  /* If we are holding a PENDING lock that ought to be released, then
  ** release it now.
  */
  यदि( got_Pending_तालकम् && locktype==SHARED_तालकम् ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }

  /* Update the state of the lock has held in the file descriptor then
  ** return the appropriate result code.
  */
  यदि( res ){
    rc = SQLITE_OK;
  }अन्यथा{
    pFile->चरमः_Errno = चरमः_Errno;
    rc = SQLITE_BUSY;
    OSTRACE(("LOCK-FAIL file=%p, wanted=%d, got=%d\n",
             pFile->h, locktype, newLocktype));
  }
  pFile->locktype = (u8)newLocktype;
  OSTRACE(("LOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  प्रत्यागम rc;
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, return
** non-zero, otherwise zero.
*/
स्थिर पूर्णाङ्क win_Check_Reserved_तालकम्(sqlite3_file *id, पूर्णाङ्क *p_Res_बहिर्गमनम्){
  पूर्णाङ्क res;
  winFile *pFile = (winFile*)id;

  SimulateIOError( प्रत्यागम SQLITE_IOERR_CHECKRESERVEDLOCK; );
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p\n", pFile->h, p_Res_बहिर्गमनम्));

  प्रतिज्ञा( id!=0 );
  यदि( pFile->locktype>=RESERVED_तालकम् ){
    res = 1;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (local)\n", pFile->h, res));
  }अन्यथा{
    res = win_तालकम्_File(&pFile->h, SQLITE_LOCKFILEEX_FLAGS,RESERVED_BYTE,0,1,0);
    यदि( res ){
      winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
    }
    res = !res;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (remote)\n", pFile->h, res));
  }
  *p_Res_बहिर्गमनम् = res;
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           pFile->h, p_Res_बहिर्गमनम्, *p_Res_बहिर्गमनम्));
  प्रत्यागम SQLITE_OK;
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
स्थिर पूर्णाङ्क winUnlock(sqlite3_file *id, पूर्णाङ्क locktype){
  पूर्णाङ्क type;
  winFile *pFile = (winFile*)id;
  पूर्णाङ्क rc = SQLITE_OK;
  प्रतिज्ञा( pFile!=0 );
  प्रतिज्ञा( locktype<=SHARED_तालकम् );
  OSTRACE(("UNLOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->shared_तालकम्_Byte, locktype));
  type = pFile->locktype;
  यदि( type>=EXCLUSIVE_तालकम् ){
    winUnlockFile(&pFile->h, SHARED_FIRST, 0, SHARED_आकार, 0);
    यदि( locktype==SHARED_तालकम् && !win_Get_Read_तालकम्(pFile, 0) ){
      /* This should never happen.  We should always be able to
      ** reacquire the read lock */
      rc = winLogError(SQLITE_IOERR_UNLOCK, os_Get_चरमः_Error(),
                       "winUnlock", pFile->z_मार्गः);
    }
  }
  यदि( type>=RESERVED_तालकम् ){
    winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
  }
  यदि( locktype==NO_तालकम् && type>=SHARED_तालकम् ){
    win_Unlock_Read_तालकम्(pFile);
  }
  यदि( type>=PENDING_तालकम् ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }
  pFile->locktype = (u8)locktype;
  OSTRACE(("UNLOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  प्रत्यागम rc;
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

स्थिर पूर्णाङ्क win_Nolock_तालकम्(sqlite3_file *id, पूर्णाङ्क locktype){
  अप्रयुक्तम्_PARAMETER(id);
  अप्रयुक्तम्_PARAMETER(locktype);
  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क win_Nolock_Check_Reserved_तालकम्(sqlite3_file *id, पूर्णाङ्क *p_Res_बहिर्गमनम्){
  अप्रयुक्तम्_PARAMETER(id);
  अप्रयुक्तम्_PARAMETER(p_Res_बहिर्गमनम्);
  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क winNolockUnlock(sqlite3_file *id, पूर्णाङ्क locktype){
  अप्रयुक्तम्_PARAMETER(id);
  अप्रयुक्तम्_PARAMETER(locktype);
  प्रत्यागम SQLITE_OK;
}

/******************* End of the no-op lock implementation *********************
******************************************************************************/

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
स्थिर शून्य win_बहुलकम्_Bit(winFile *pFile, अचिह्नित वर्ण आवरणम्, पूर्णाङ्क *pArg){
  यदि( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & आवरणम्)!=0;
  }अन्यथा यदि( (*pArg)==0 ){
    pFile->ctrlFlags &= ~आवरणम्;
  }अन्यथा{
    pFile->ctrlFlags |= आवरणम्;
  }
}

/* Forward references to VFS helper methods used for temporary files */
स्थिर पूर्णाङ्क winGetTempname(sqlite3_vfs *, वर्ण **);
स्थिर पूर्णाङ्क winIsDir(LPCWSTR);
स्थिर BOOL win_Is_Long_मार्गः_उपसर्गः(ध्रुव वर्ण *);
स्थिर BOOL winIsDriveLetterAndColon(ध्रुव वर्ण *);

/*
** Control and query of the open file handle.
*/
स्थिर पूर्णाङ्क winFileControl(sqlite3_file *id, पूर्णाङ्क op, शून्य *pArg){
  winFile *pFile = (winFile*)id;
  OSTRACE(("FCNTL file=%p, op=%d, pArg=%p\n", pFile->h, op, pArg));
  विकल्प( op ){
    पक्ष SQLITE_FCNTL_LOCKSTATE: {
      *(पूर्णाङ्क*)pArg = pFile->locktype;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      प्रत्यागम SQLITE_OK;
    }
    पक्ष SQLITE_FCNTL_चरमः_ERRNO: {
      *(पूर्णाङ्क*)pArg = (पूर्णाङ्क)pFile->चरमः_Errno;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      प्रत्यागम SQLITE_OK;
    }
    पक्ष SQLITE_FCNTL_CHUNK_आकार: {
      pFile->szChunk = *(पूर्णाङ्क *)pArg;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      प्रत्यागम SQLITE_OK;
    }
    पक्ष SQLITE_FCNTL_आकार_HINT: {
      यदि( pFile->szChunk>0 ){
        sqlite3_int64 oldSz;
        पूर्णाङ्क rc = win_File_आकार(id, &oldSz);
        यदि( rc==SQLITE_OK ){
          sqlite3_int64 newSz = *(sqlite3_int64*)pArg;
          यदि( newSz>oldSz ){
            SimulateIOErrorBenign(1);
            rc = winTruncate(id, newSz);
            SimulateIOErrorBenign(0);
          }
        }
        OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
        प्रत्यागम rc;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      प्रत्यागम SQLITE_OK;
    }
    पक्ष SQLITE_FCNTL_PERSIST_WAL: {
      win_बहुलकम्_Bit(pFile, WINFILE_PERSIST_WAL, (पूर्णाङ्क*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      प्रत्यागम SQLITE_OK;
    }
    पक्ष SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      win_बहुलकम्_Bit(pFile, WINFILE_PSOW, (पूर्णाङ्क*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      प्रत्यागम SQLITE_OK;
    }
    पक्ष SQLITE_FCNTL_VFSNAME: {
      *(वर्ण**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      प्रत्यागम SQLITE_OK;
    }
    पक्ष SQLITE_FCNTL_WIN32_AV_RETRY: {
      पूर्णाङ्क *a = (पूर्णाङ्क*)pArg;
      यदि( a[0]>0 ){
        winIoerrRetry = a[0];
      }अन्यथा{
        a[0] = winIoerrRetry;
      }
      यदि( a[1]>0 ){
        winIoerrRetryDelay = a[1];
      }अन्यथा{
        a[1] = winIoerrRetryDelay;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      प्रत्यागम SQLITE_OK;
    }
    पक्ष SQLITE_FCNTL_WIN32_GET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      *phFile = pFile->h;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      प्रत्यागम SQLITE_OK;
    }
#यदि_परिभाषितम् SQLITE_TEST
    पक्ष SQLITE_FCNTL_WIN32_समुच्चयः_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      HANDLE hOldFile = pFile->h;
      pFile->h = *phFile;
      *phFile = hOldFile;
      OSTRACE(("FCNTL oldFile=%p, newFile=%p, rc=SQLITE_OK\n",
               hOldFile, pFile->h));
      प्रत्यागम SQLITE_OK;
    }
#यदि_अन्तः
    पक्ष SQLITE_FCNTL_NULL_IO: {
      (शून्य)osCloseHandle(pFile->h);
      pFile->h = रिक्त;
      प्रत्यागम SQLITE_OK;
    }
    पक्ष SQLITE_FCNTL_TEMPFILENAME: {
      वर्ण *zTFile = 0;
      पूर्णाङ्क rc = winGetTempname(pFile->pVfs, &zTFile);
      यदि( rc==SQLITE_OK ){
        *(वर्ण**)pArg = zTFile;
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      प्रत्यागम rc;
    }
#यदि SQLITE_MAX_MMAP_आकार>0
    पक्ष SQLITE_FCNTL_MMAP_आकार: {
      i64 new_सीमा = *(i64*)pArg;
      पूर्णाङ्क rc = SQLITE_OK;
      यदि( new_सीमा>sqlite3GlobalConfig.mxMmap ){
        new_सीमा = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (SIZE_T) and passed
      ** to MapViewOfFile(). Restrict its value to 2GB if (SIZE_T) is not at
      ** least a 64-bit type. */
      यदि( new_सीमा>0 && परिमाण(आकार_T)<8 ){
        new_सीमा = (new_सीमा & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmap_आकार_Max;
      यदि( new_सीमा>=0 && new_सीमा!=pFile->mmap_आकार_Max && pFile->n_Fetch_बहिर्गमनम्==0 ){
        pFile->mmap_आकार_Max = new_सीमा;
        यदि( pFile->mmap_आकार>0 ){
          winUnmapfile(pFile);
          rc = winMapfile(pFile, -1);
        }
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      प्रत्यागम rc;
    }
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_ENABLE_SETLK_TIMEOUT
    पक्ष SQLITE_FCNTL_तालकम्_TIMEOUT: {
      पूर्णाङ्क iOld = pFile->iBusyTimeout;
      पूर्णाङ्क iNew = *(पूर्णाङ्क*)pArg;
#यदि SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = (iNew < 0) ? INFINITE : (DWORD)iNew;
#नोचेद्यदि SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = (DWORD)(!!iNew);
#अन्यथा
# error "SQLITE_ENABLE_SETLK_TIMEOUT must be set to 1 or 2"
#यदि_अन्तः
      *(पूर्णाङ्क*)pArg = iOld;
      प्रत्यागम SQLITE_OK;
    }
    पक्ष SQLITE_FCNTL_खण्डम्_ON_CONNECT: {
      पूर्णाङ्क iNew = *(पूर्णाङ्क*)pArg;
      pFile->b_खण्डम्_On_Connect = iNew;
      प्रत्यागम SQLITE_OK;
    }
#यदि_अन्तः /* SQLITE_ENABLE_SETLK_TIMEOUT */

#यदि defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    पक्ष SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      sqlite3_str_appendf(pStr, "{\"h\":%llu", (sqlite3_uint64)pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      यदि( pFile->locktype ){
        स्थिर ध्रुव वर्ण *az_तालकम्[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
        sqlite3_str_appendf(pStr, ",\"locktype\":\"%s\"", 
                                  az_तालकम्[pFile->locktype-1]);
      }
#यदि SQLITE_MAX_MMAP_आकार>0
      यदि( pFile->mmap_आकार ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmap_आकार);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->n_Fetch_बहिर्गमनम्);
      }
#यदि_अन्तः
      sqlite3_str_append(pStr, "}", 1);
      प्रत्यागम SQLITE_OK;
    }
#यदि_अन्तः /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

  }
  OSTRACE(("FCNTL file=%p, rc=SQLITE_NOTFOUND\n", pFile->h));
  प्रत्यागम SQLITE_NOTFOUND;
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
स्थिर पूर्णाङ्क win_Sector_आकार(sqlite3_file *id){
  (शून्य)id;
  प्रत्यागम SQLITE_DEFAULT_SECTOR_आकार;
}

/*
** Return a vector of device characteristics.
*/
स्थिर पूर्णाङ्क winDeviceCharacteristics(sqlite3_file *id){
  winFile *p = (winFile*)id;
  प्रत्यागम SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN | SQLITE_IOCAP_SUBPAGE_READ |
         ((p->ctrlFlags & WINFILE_PSOW)?SQLITE_IOCAP_POWERSAFE_OVERWRITE:0);
}

/*
** Windows will only let you create file view mappings
** on allocation size granularity boundaries.
** During sqlite3_os_init() we do a GetSystemInfo()
** to get the granularity size.
*/
स्थिर SYSTEM_INFO winSysInfo;

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
स्थिर LPWSTR winConvertFromUtf8Filename(ध्रुव वर्ण *zFilename){
  LPWSTR zConverted = 0;
#यदि_परिभाषितम् __CYGWIN__
  पूर्णाङ्क nChar;

  यदि( os_Cygwin_conv_मार्गः && !(winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2])) ){
    i64 nByte;
    पूर्णाङ्क convertflag = CCP_POSIX_गन्तव्यम्_WIN_W;
    यदि( !strchr(zFilename, '/') ) convertflag |= CCP_RELATIVE;
    nByte = (i64)os_Cygwin_conv_मार्गः(convertflag,
        zFilename, 0, 0);
    यदि( nByte>0 ){
      zConverted = sqlite3MallocZero(12+(u64)nByte);
      यदि ( zConverted==0 ){
        प्रत्यागम zConverted;
      }
      /* Filenames should be prefixed, except when converted
       * full path already starts with "\\?\". */
      यदि( os_Cygwin_conv_मार्गः(convertflag, zFilename,
                           zConverted+4, nByte)==0 ){
        यदि( (convertflag&CCP_RELATIVE) ){
          memmove(zConverted, zConverted+4, nByte);
        }अन्यथा यदि( memcmp(zConverted+4, L"\\\\", 4) ){
          स्मृतिं_प्रतिलिख(zConverted, L"\\\\?\\", 8);
        }अन्यथा यदि( zConverted[6]!='?' ){
          memmove(zConverted+6, zConverted+4, nByte);
          स्मृतिं_प्रतिलिख(zConverted, L"\\\\?\\UNC", 14);
        }अन्यथा{
          memmove(zConverted, zConverted+4, nByte);
        }
        प्रत्यागम zConverted;
      }
      sqlite3_free(zConverted);
    }
  }
  nChar = os_Multi_Byte_गन्तव्यम्_Wide_Char(CP_UTF8, 0, zFilename, -1, रिक्त, 0);
  यदि( nChar==0 ){
    प्रत्यागम 0;
  }
  zConverted = sqlite3MallocZero( ((u64)nChar+6)*परिमाण(WCHAR) );
  यदि( zConverted==0 ){
    प्रत्यागम 0;
  }
  nChar = os_Multi_Byte_गन्तव्यम्_Wide_Char(CP_UTF8, 0, zFilename, -1,
                                zConverted, nChar);
  यदि( nChar==0 ){
    sqlite3_free(zConverted);
    zConverted = 0;
  }अन्यथा यदि( nChar>MAX_मार्गः
      && winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2]) ){
    memmove(zConverted+4, zConverted, nChar*परिमाण(WCHAR));
    zConverted[2] = '\\';
    स्मृतिं_प्रतिलिख(zConverted, L"\\\\?\\", 8);
  }अन्यथा यदि( nChar>MAX_मार्गः
      && winIsDirSep(zFilename[0]) && winIsDirSep(zFilename[1])
      && zFilename[2] != '?' ){
    memmove(zConverted+6, zConverted, nChar*परिमाण(WCHAR));
    स्मृतिं_प्रतिलिख(zConverted, L"\\\\?\\UNC", 14);
  }
#अन्यथा /* if !defined(__CYGWIN__) */
  zConverted = win_Utf8_गन्तव्यम्_Unicode(zFilename);
#यदि_अन्तः /* __CYGWIN__ */
  /* caller will handle out of memory */
  प्रत्यागम zConverted;
}

#यदि_अपरिभाषितम् SQLITE_OMIT_WAL

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
स्थिर sqlite3_mutex *win_Big_तालकम् = 0;
स्थिर शून्य winShmEnterMutex(शून्य){
  sqlite3_mutex_enter(win_Big_तालकम्);
}
स्थिर शून्य winShmLeaveMutex(शून्य){
  sqlite3_mutex_leave(win_Big_तालकम्);
}
#यदि_अपरिभाषितम् NDEBUG
स्थिर पूर्णाङ्क winShmMutexHeld(शून्य) {
  प्रत्यागम sqlite3_mutex_held(win_Big_तालकम्);
}
#यदि_अन्तः

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
संरचना win_Shm_ग्रन्थिः {
  sqlite3_mutex *mutex;      /* Mutex to access this object */
  वर्ण *zFilename;           /* Name of the file */
  HANDLE hSharedShm;         /* File handle open on zFilename */
  पूर्णाङ्क b_Use_Shared_तालकम्_Handle;  /* True to use hSharedShm for everything */

  पूर्णाङ्क isUnlocked;            /* DMS lock has not yet been obtained */
  पूर्णाङ्क isReadonly;            /* True if read-only */
  पूर्णाङ्क szRegion;              /* Size of shared-memory regions */
  पूर्णाङ्क nRegion;               /* Size of array apRegion */

  संरचना ShmRegion {
    HANDLE hMap;             /* File handle from CreateFileMapping */
    शून्य *pMap;
  } *aRegion;
  DWORD चरमः_Errno;           /* The Windows errno from the last I/O error */

  winShm *pWinShmList;       /* List of winShm objects with ptrs to this */

  win_Shm_ग्रन्थिः *p_अग्र;         /* Next in list of all winShmNode objects */
#यदि defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 अग्र_Shm_Id;              /* Next available winShm.id value */
#यदि_अन्तः
};

/*
** A global array of all winShmNode objects.
**
** The winShmMutexHeld() must be true while reading or writing this list.
*/
स्थिर win_Shm_ग्रन्थिः *win_Shm_ग्रन्थिः_List = 0;

/*
** Structure used internally by this VFS to record the state of an
** open shared memory connection. There is one such structure for each
** winFile open on a wal mode database.
*/
संरचना winShm {
  win_Shm_ग्रन्थिः *p_Shm_ग्रन्थिः;      /* The underlying winShmNode object */
  u16 shared_आवरणम्;            /* Mask of shared locks held */
  u16 excl_आवरणम्;              /* Mask of exclusive locks held */
  HANDLE hShm;               /* File-handle on *-shm file. For locking. */
  पूर्णाङ्क bReadonly;             /* True if hShm is opened read-only */
#यदि defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 id;                     /* Id of this connection with its winShmNode */
#यदि_अन्तः
  winShm *p_Win_Shm_अग्र;       /* Next winShm object on same winShmNode */
};

/*
** Constants used for locking
*/
#परिभाषय WIN_SHM_आधारः   ((22+SQLITE_SHM_NLOCK)*4)        /* first lock byte */
#परिभाषय WIN_SHM_DMS    (WIN_SHM_आधारः+SQLITE_SHM_NLOCK)  /* deadman switch */

/* Forward references to VFS methods */
स्थिर पूर्णाङ्क winOpen(sqlite3_vfs*,ध्रुव वर्ण*,sqlite3_file*,पूर्णाङ्क,पूर्णाङ्क*);
स्थिर पूर्णाङ्क winDelete(sqlite3_vfs *,ध्रुव वर्ण*,पूर्णाङ्क);

/*
** Purge the winShmNodeList list of all entries with winShmNode.pWinShmList==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
स्थिर शून्य winShmPurge(sqlite3_vfs *pVfs, पूर्णाङ्क deleteFlag){
  win_Shm_ग्रन्थिः **pp;
  win_Shm_ग्रन्थिः *p;
  प्रतिज्ञा( winShmMutexHeld() );
  OSTRACE(("SHM-PURGE pid=%lu, deleteFlag=%d\n",
           osGetCurrentProcessId(), deleteFlag));
  pp = &win_Shm_ग्रन्थिः_List;
  यावत्( (p = *pp)!=0 ){
    यदि( p->pWinShmList==0 ){
      पूर्णाङ्क i;
      यदि( p->mutex ){ sqlite3_mutex_free(p->mutex); }
      प्रति(i=0; i<p->nRegion; i++){
        BOOL bRc = os_Unmap_View_सम्बन्धि_File(p->aRegion[i].pMap);
        OSTRACE(("SHM-PURGE-UNMAP pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        अप्रयुक्तम्_VARIABLE_मानम्(bRc);
        bRc = osCloseHandle(p->aRegion[i].hMap);
        OSTRACE(("SHM-PURGE-CLOSE pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        अप्रयुक्तम्_VARIABLE_मानम्(bRc);
      }
      winHandleClose(p->hSharedShm);
      यदि( deleteFlag ){
        SimulateIOErrorBenign(1);
        sqlite3BeginBenignMalloc();
        winDelete(pVfs, p->zFilename, 0);
        sqlite3_अन्तः_Benign_Malloc();
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
स्थिर पूर्णाङ्क win_Zero_Shared_स्मृतिः(HANDLE h){
  u8 aZero[8] = {0,0,0,0,0,0,0,0};
  DWORD nWrite = 0;
  OVERLAPPED ovlp;
  स्मृतिं_पूरय(&ovlp, 0, परिमाण(OVERLAPPED));
  यदि( !osWriteFile(h, aZero, परिमाण(aZero), &nWrite, &ovlp) ){
    /* If the file was opened with FILE_FLAG_OVERLAPPED, the write may still
    ** be pending. Wait for it with GetOverlappedResult().  */
    यदि( os_Get_चरमः_Error()!=ERROR_IO_PENDING
     || !Get_Overlapped_परिणाम(h, &ovlp, &nWrite, TRUE)
    ){
      nWrite = 0;
    }
  }
  प्रत्यागम (nWrite==परिमाण(aZero) ? SQLITE_OK : SQLITE_IOERR_WRITE);
}

/*
** The DMS lock has not yet been taken on the shm file associated with
** pShmNode. Take the lock. Truncate the *-shm file if required.
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
स्थिर पूर्णाङ्क win_तालकम्_Shared_स्मृतिः(win_Shm_ग्रन्थिः *p_Shm_ग्रन्थिः, DWORD nMs){
  HANDLE h = p_Shm_ग्रन्थिः->hSharedShm;
  पूर्णाङ्क rc = SQLITE_OK;

  प्रतिज्ञा( sqlite3_mutex_held(p_Shm_ग्रन्थिः->mutex) );
  rc = win_Handle_तालकम्_Timeout(h, WIN_SHM_DMS, 1, 1, 0);
  यदि( rc==SQLITE_OK ){
    /* We have an EXCLUSIVE lock on the DMS byte. This means that this
    ** is the first process to open the file. Truncate it to zero bytes
    ** in this case.  */
    यदि( p_Shm_ग्रन्थिः->isReadonly ){
      rc = SQLITE_READONLY_CANTINIT;
    }अन्यथा{
      यदि( SQLITE_OK!=winHandleTruncate(h, 0) ){
        /* Sometimes the truncate operation may fail because some other
        ** process is still holding an open mapping. So try to write
        ** zeroes into the start of the file instead. */
        rc = win_Zero_Shared_स्मृतिः(h);
      }
    }

    /* Release the EXCLUSIVE lock acquired above. */
    winUnlockFile(&h, WIN_SHM_DMS, 0, 1, 0);
  }अन्यथा यदि( (rc & 0xFF)==SQLITE_BUSY ){
    rc = SQLITE_OK;
  }

  यदि( rc==SQLITE_OK ){
    /* Take a SHARED lock on the DMS byte. */
    rc = win_Handle_तालकम्_Timeout(h, WIN_SHM_DMS, 1, 0, nMs);
    यदि( rc==SQLITE_OK ){
      p_Shm_ग्रन्थिः->isUnlocked = 0;
    }
  }

  प्रत्यागम rc;
}


/*
** This function is used to open a handle on a *-shm file.
**
** If SQLITE_ENABLE_SETLK_TIMEOUT is defined at build time, then the file
** is opened with FILE_FLAG_OVERLAPPED specified. If not, it is not.
*/
स्थिर पूर्णाङ्क winHandleOpen(
  ध्रुव वर्ण *zUtf8,              /* File to open */
  पूर्णाङ्क *pbReadonly,                /* IN/OUT: True for readonly handle */
  HANDLE *ph                      /* OUT: New HANDLE for file */
){
  पूर्णाङ्क rc = SQLITE_OK;
  LPWSTR zConverted = 0;
  पूर्णाङ्क bReadonly = *pbReadonly;
  HANDLE h = INVALID_HANDLE_मानम्;

#यदि_परिभाषितम् SQLITE_ENABLE_SETLK_TIMEOUT
  ध्रुव DWORD flag_overlapped = FILE_FLAG_OVERLAPPED;
#अन्यथा
  ध्रुव DWORD flag_overlapped = 0;
#यदि_अन्तः

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8);
  यदि( zConverted==0 ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8));
    rc = SQLITE_IOERR_NOMEM_BKPT;
    गच्छ winopenfile_बहिर्गमनम्;
  }

  /* Ensure the file we are trying to open is not actually a directory. */
  यदि( winIsDir(zConverted) ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8));
    rc = SQLITE_CANTOPEN_ISDIR;
    गच्छ winopenfile_बहिर्गमनम्;
  }

  /* TODO: platforms.
  ** TODO: retry-on-ioerr.
  */
  h = osCreateFileW(zConverted,         /* lpFileName */
      (GENERIC_READ | (bReadonly ? 0 : GENERIC_WRITE)),  /* dwDesiredAccess */
      FILE_SHARE_READ | FILE_SHARE_WRITE,        /* dwShareMode */
      रिक्त,                                      /* lpSecurityAttributes */
      OPEN_ALWAYS,                               /* dwCreationDisposition */
      FILE_ATTRIBUTE_प्रसामान्यम्|flag_overlapped,
      रिक्त
  );

  यदि( h==INVALID_HANDLE_मानम् ){
    यदि( bReadonly==0 ){
      bReadonly = 1;
      rc = winHandleOpen(zUtf8, &bReadonly, &h);
    }अन्यथा{
      rc = SQLITE_CANTOPEN_BKPT;
    }
  }

 winopenfile_बहिर्गमनम्:
  sqlite3_free(zConverted);
  *pbReadonly = bReadonly;
  *ph = h;
  प्रत्यागम rc;
}
 
/*
** Close pDbFd's connection to shared-memory.  Delete the underlying
** *-shm file if deleteFlag is true.
*/
स्थिर पूर्णाङ्क win_Close_Shared_स्मृतिः(winFile *pDbFd, पूर्णाङ्क deleteFlag){
  winShm *p;            /* The connection to be closed */
  winShm **pp;          /* Iterator for pShmNode->pWinShmList */
  win_Shm_ग्रन्थिः *p_Shm_ग्रन्थिः; /* The underlying shared-memory file */

  p = pDbFd->pShm;
  यदि( p==0 ) प्रत्यागम SQLITE_OK;
  यदि( p->hShm!=INVALID_HANDLE_मानम् ){
    osCloseHandle(p->hShm);
  }

  winShmEnterMutex();
  p_Shm_ग्रन्थिः = p->p_Shm_ग्रन्थिः;

  /* Remove this connection from the winShmNode.pWinShmList list */
  sqlite3_mutex_enter(p_Shm_ग्रन्थिः->mutex);
  प्रति(pp=&p_Shm_ग्रन्थिः->pWinShmList; *pp!=p; pp=&(*pp)->p_Win_Shm_अग्र){}
  *pp = p->p_Win_Shm_अग्र;
  sqlite3_mutex_leave(p_Shm_ग्रन्थिः->mutex);

  winShmPurge(pDbFd->pVfs, deleteFlag);
  winShmLeaveMutex();

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  प्रत्यागम SQLITE_OK;
}

/*
** testfixture builds may set this global variable to true via a
** Tcl interface. This forces the VFS to use the locking normally
** only used for UNC paths for all files.
*/
#यदि_परिभाषितम् SQLITE_TEST
पूर्णाङ्क sqlite3_win_test_unc_locking = 0;
#अन्यथा
# define sqlite3_win_test_unc_locking 0
#यदि_अन्तः

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
स्थिर पूर्णाङ्क winIsUNCPath(ध्रुव वर्ण *zFile){
  यदि( zFile[0]=='\\' && zFile[1]=='\\' ){
    यदि( zFile[2]=='?'
     && zFile[3]=='\\'
     && sqlite3Isalpha(zFile[4])
     && zFile[5]==':'
     && winIsDirSep(zFile[6])
    ){
      प्रत्यागम sqlite3_win_test_unc_locking;
    }अन्यथा{
      प्रत्यागम 1;
    }
  }
  प्रत्यागम sqlite3_win_test_unc_locking;
}

/*
** Return true if the string passed in is the name of a file on a
** remote (network-mounted) volume.
*/
स्थिर पूर्णाङ्क winIsRemoteVolume(ध्रुव वर्ण *zFile){
  WCHAR z_मूलम्[4];
  यदि( वाक्ये_तुलय(zFile, "\\\\?\\",4)==0 ) zFile += 4;
  यदि( !sqlite3Isalpha(zFile[0]) ) प्रत्यागम 0;
  यदि( zFile[1]!=':' ) प्रत्यागम 0;
  यदि( !winIsDirSep(zFile[2]) ) प्रत्यागम 0;
  z_मूलम्[0] = (WCHAR)zFile[0];
  z_मूलम्[1] = ':';
  z_मूलम्[2] = '\\';
  z_मूलम्[3] = 0;
  प्रत्यागम osGetDriveTypeW(z_मूलम्)==DRIVE_REMOTE;
}

/*
** Open the shared-memory area associated with database file pDbFd.
*/
स्थिर पूर्णाङ्क win_Open_Shared_स्मृतिः(winFile *pDbFd){
  संरचना winShm *p;                  /* The connection to be opened */
  win_Shm_ग्रन्थिः *p_Shm_ग्रन्थिः = 0;          /* The underlying mmapped file */
  पूर्णाङ्क rc = SQLITE_OK;                /* Result code */
  win_Shm_ग्रन्थिः *pNew;                  /* Newly allocated winShmNode */
  पूर्णाङ्क nName;                         /* Size of zName in bytes */

  प्रतिज्ञा( pDbFd->pShm==0 );    /* Not previously opened */

  /* Allocate space for the new sqlite3_shm object.  Also speculatively
  ** allocate space for a new winShmNode and filename.  */
  p = sqlite3MallocZero( परिमाण(*p) );
  यदि( p==0 ) प्रत्यागम SQLITE_IOERR_NOMEM_BKPT;
  nName = sqlite3Strlen30(pDbFd->z_मार्गः);
  pNew = sqlite3MallocZero( परिमाण(*p_Shm_ग्रन्थिः) + (i64)nName + 17 );
  यदि( pNew==0 ){
    sqlite3_free(p);
    प्रत्यागम SQLITE_IOERR_NOMEM_BKPT;
  }
  pNew->zFilename = (वर्ण*)&pNew[1];
  pNew->hSharedShm = INVALID_HANDLE_मानम्;
  pNew->isUnlocked = 1;
  यदि( winIsUNCPath(pDbFd->z_मार्गः) || winIsRemoteVolume(pDbFd->z_मार्गः) ){
    pNew->b_Use_Shared_तालकम्_Handle = 1;
  }अन्यथा{
    pNew->b_Use_Shared_तालकम्_Handle = 0;
  }
  sqlite3_snprintf(nName+15, pNew->zFilename, "%s-shm", pDbFd->z_मार्गः);
  sqlite3FileSuffix3(pDbFd->z_मार्गः, pNew->zFilename);

  /* Look to see if there is an existing winShmNode that can be used.
  ** If no matching winShmNode currently exists, then create a new one.  */
  winShmEnterMutex();
  प्रति(p_Shm_ग्रन्थिः = win_Shm_ग्रन्थिः_List; p_Shm_ग्रन्थिः; p_Shm_ग्रन्थिः=p_Shm_ग्रन्थिः->p_अग्र){
    /* TBD need to come up with better match here.  Perhaps
    ** use FILE_ID_BOTH_DIR_INFO Structure.  */
    यदि( sqlite3StrICmp(p_Shm_ग्रन्थिः->zFilename, pNew->zFilename)==0 ) भङ्ग;
  }
  यदि( p_Shm_ग्रन्थिः==0 ){
    p_Shm_ग्रन्थिः = pNew;

    /* Allocate a mutex for this winShmNode object, if one is required. */
    यदि( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_ग्रन्थिः->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      यदि( p_Shm_ग्रन्थिः->mutex==0 ) rc = SQLITE_IOERR_NOMEM_BKPT;
    }

    /* Open a file-handle to use for mappings, and for the DMS lock. */
    यदि( rc==SQLITE_OK ){
      HANDLE h = INVALID_HANDLE_मानम्;
      p_Shm_ग्रन्थिः->isReadonly = sqlite3_uri_boolean(pDbFd->z_मार्गः,"readonly_shm",0);
      rc = winHandleOpen(pNew->zFilename, &p_Shm_ग्रन्थिः->isReadonly, &h);
      p_Shm_ग्रन्थिः->hSharedShm = h;
    }

    /* If successful, link the new winShmNode into the global list. If an 
    ** error occurred, free the object. */
    यदि( rc==SQLITE_OK ){
      p_Shm_ग्रन्थिः->p_अग्र = win_Shm_ग्रन्थिः_List;
      win_Shm_ग्रन्थिः_List = p_Shm_ग्रन्थिः;
      pNew = 0;
    }अन्यथा{
      sqlite3_mutex_free(p_Shm_ग्रन्थिः->mutex);
      यदि( p_Shm_ग्रन्थिः->hSharedShm!=INVALID_HANDLE_मानम् ){
        osCloseHandle(p_Shm_ग्रन्थिः->hSharedShm);
      }
    }
  }

  /* If no error has occurred, link the winShm object to the winShmNode and
  ** the winShm to pDbFd.  */
  यदि( rc==SQLITE_OK ){
    sqlite3_mutex_enter(p_Shm_ग्रन्थिः->mutex);
    p->p_Shm_ग्रन्थिः = p_Shm_ग्रन्थिः;
    p->p_Win_Shm_अग्र = p_Shm_ग्रन्थिः->pWinShmList;
    p_Shm_ग्रन्थिः->pWinShmList = p;
#यदि defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
    p->id = p_Shm_ग्रन्थिः->अग्र_Shm_Id++;
#यदि_अन्तः
    pDbFd->pShm = p;
    sqlite3_mutex_leave(p_Shm_ग्रन्थिः->mutex);
  }अन्यथा यदि( p ){
    sqlite3_free(p);
  }

  प्रतिज्ञा( rc!=SQLITE_OK || p_Shm_ग्रन्थिः->isUnlocked==0 || p_Shm_ग्रन्थिः->nRegion==0 );
  winShmLeaveMutex();
  sqlite3_free(pNew);

  /* Open a file-handle on the *-shm file for this connection. This file-handle
  ** is only used for locking. The mapping of the *-shm file is created using
  ** the shared file handle in winShmNode.hSharedShm.  */
  यदि( rc==SQLITE_OK && p_Shm_ग्रन्थिः->b_Use_Shared_तालकम्_Handle==0 ){
    p->bReadonly = sqlite3_uri_boolean(pDbFd->z_मार्गः, "readonly_shm", 0);
    rc = winHandleOpen(p_Shm_ग्रन्थिः->zFilename, &p->bReadonly, &p->hShm);
    यदि( rc!=SQLITE_OK ){
      प्रतिज्ञा( p->hShm==INVALID_HANDLE_मानम् );
      win_Close_Shared_स्मृतिः(pDbFd, 0);
    }
  }

  प्रत्यागम rc;
}

/*
** Close a connection to shared-memory.  Delete the underlying
** storage if deleteFlag is true.
*/
स्थिर पूर्णाङ्क winShmUnmap(
  sqlite3_file *fd,          /* Database holding shared memory */
  पूर्णाङ्क deleteFlag             /* Delete after closing if true */
){
  प्रत्यागम win_Close_Shared_स्मृतिः((winFile*)fd, deleteFlag);
}

/*
** Change the lock state for a shared-memory segment.
*/
स्थिर पूर्णाङ्क win_Shm_तालकम्(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  पूर्णाङ्क ofst,                  /* First lock to acquire or release */
  पूर्णाङ्क n,                     /* Number of locks to acquire or release */
  पूर्णाङ्क flags                  /* What to do with the lock */
){
  winFile *pDbFd = (winFile*)fd;        /* Connection holding shared memory */
  winShm *p = pDbFd->pShm;              /* The shared memory being locked */
  win_Shm_ग्रन्थिः *p_Shm_ग्रन्थिः;
  पूर्णाङ्क rc = SQLITE_OK;                   /* Result code */
  u16 आवरणम् = (u16)((1U<<(ofst+n)) - (1U<<ofst)); /* Mask of locks to [un]take */

  यदि( p==0 ) प्रत्यागम SQLITE_IOERR_SHMLOCK;
  p_Shm_ग्रन्थिः = p->p_Shm_ग्रन्थिः;
  यदि( NEVER(p_Shm_ग्रन्थिः==0) ) प्रत्यागम SQLITE_IOERR_SHMLOCK;

  प्रतिज्ञा( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  प्रतिज्ञा( n>=1 );
  प्रतिज्ञा( flags==(SQLITE_SHM_तालकम् | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_तालकम् | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  प्रतिज्ञा( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );

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
#यदि defined(SQLITE_ENABLE_SETLK_TIMEOUT) && defined(SQLITE_DEBUG)
  {
    u16 तालकम्_आवरणम् = (p->excl_आवरणम्|p->shared_आवरणम्);
    प्रतिज्ञा( (flags & SQLITE_SHM_UNLOCK) || pDbFd->iBusyTimeout==0 || (
          (ofst!=2 || तालकम्_आवरणम्==0)
       && (ofst!=1 || तालकम्_आवरणम्==0 || तालकम्_आवरणम्==2)
       && (ofst!=0 || तालकम्_आवरणम्<3)
       && (ofst<3  || तालकम्_आवरणम्<(1<<ofst))
    ));
  }
#यदि_अन्तः

  /* Check if there is any work to do. There are three cases:
  **
  **    a) An unlock operation where there are locks to unlock,
  **    b) An shared lock where the requested lock is not already held
  **    c) An exclusive lock where the requested lock is not already held
  **
  ** The SQLite core never requests an exclusive lock that it already holds.
  ** This is assert()ed immediately below.  */
  प्रतिज्ञा( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_तालकम्) 
       || 0==(p->excl_आवरणम् & आवरणम्)
  );
  यदि( ((flags & SQLITE_SHM_UNLOCK) && ((p->excl_आवरणम्|p->shared_आवरणम्) & आवरणम्))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_तालकम्) && 0==(p->shared_आवरणम् & आवरणम्))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_तालकम्))
  ){
    HANDLE h = p->hShm;

    यदि( flags & SQLITE_SHM_UNLOCK ){
      /* Case (a) - unlock.  */

      प्रतिज्ञा( (p->excl_आवरणम् & p->shared_आवरणम्)==0 );
      प्रतिज्ञा( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->excl_आवरणम् & आवरणम्)==आवरणम् );
      प्रतिज्ञा( !(flags & SQLITE_SHM_SHARED) || (p->shared_आवरणम् & आवरणम्)==आवरणम् );

      प्रतिज्ञा( !(flags & SQLITE_SHM_SHARED) || n==1 );
      यदि( p_Shm_ग्रन्थिः->b_Use_Shared_तालकम्_Handle ){
        h = p_Shm_ग्रन्थिः->hSharedShm;
        यदि( flags & SQLITE_SHM_SHARED ){
          winShm *pShm;
          sqlite3_mutex_enter(p_Shm_ग्रन्थिः->mutex);
          प्रति(pShm=p_Shm_ग्रन्थिः->pWinShmList; pShm; pShm=pShm->p_Win_Shm_अग्र){
            यदि( pShm!=p && (pShm->shared_आवरणम् & आवरणम्) ){
              /* Another connection within this process is also holding this
              ** SHARED lock. So do not actually release the OS lock.  */
              h = INVALID_HANDLE_मानम्;
              भङ्ग;
            }
          }
          sqlite3_mutex_leave(p_Shm_ग्रन्थिः->mutex);
        }
      }

      यदि( h!=INVALID_HANDLE_मानम् ){
        rc = winHandleUnlock(h, ofst+WIN_SHM_आधारः, n);
      }

      /* If successful, also clear the bits in sharedMask/exclMask */
      यदि( rc==SQLITE_OK ){
        p->excl_आवरणम् = (p->excl_आवरणम् & ~आवरणम्);
        p->shared_आवरणम् = (p->shared_आवरणम् & ~आवरणम्);
      }
    }अन्यथा{
      पूर्णाङ्क bExcl = ((flags & SQLITE_SHM_EXCLUSIVE) ? 1 : 0);
      DWORD nMs = winFileBusyTimeout(pDbFd);

      यदि( p_Shm_ग्रन्थिः->b_Use_Shared_तालकम्_Handle ){
        winShm *pShm;
        h = p_Shm_ग्रन्थिः->hSharedShm;
        sqlite3_mutex_enter(p_Shm_ग्रन्थिः->mutex);
        प्रति(pShm=p_Shm_ग्रन्थिः->pWinShmList; pShm; pShm=pShm->p_Win_Shm_अग्र){
          यदि( bExcl ){
            यदि( (pShm->shared_आवरणम्|pShm->excl_आवरणम्) & आवरणम् ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_मानम्;
            }
          }अन्यथा{
            यदि( pShm->shared_आवरणम् & आवरणम् ){
              h = INVALID_HANDLE_मानम्;
            }अन्यथा यदि( pShm->excl_आवरणम् & आवरणम् ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_मानम्;
            }
          }
        }
        sqlite3_mutex_leave(p_Shm_ग्रन्थिः->mutex);
      }

      यदि( h!=INVALID_HANDLE_मानम् ){
        rc = win_Handle_तालकम्_Timeout(h, ofst+WIN_SHM_आधारः, n, bExcl, nMs);
      }
      यदि( rc==SQLITE_OK ){
        यदि( bExcl ){
          p->excl_आवरणम् = (p->excl_आवरणम् | आवरणम्);
        }अन्यथा{
          p->shared_आवरणम् = (p->shared_आवरणम् | आवरणम्);
        }
      }
    }
  }

  OSTRACE((
      "SHM-LOCK(%d,%d,%d) pid=%lu, id=%d, sharedMask=%03x, exclMask=%03x,"
      " rc=%s\n",
      ofst, n, flags, 
      osGetCurrentProcessId(), p->id, p->shared_आवरणम्, p->excl_आवरणम्,
      sqlite3ErrName(rc))
  );
  प्रत्यागम rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
स्थिर शून्य winShmBarrier(
  sqlite3_file *fd          /* Database holding the shared memory */
){
  अप्रयुक्तम्_PARAMETER(fd);
  sqlite3_स्मृतिः_Barrier();   /* compiler-defined memory barrier */
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
स्थिर पूर्णाङ्क winShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  पूर्णाङ्क iRegion,                    /* Region to retrieve */
  पूर्णाङ्क szRegion,                   /* Size of regions */
  पूर्णाङ्क isWrite,                    /* True to extend file if necessary */
  शून्य चञ्चल **pp              /* OUT: Mapped memory */
){
  winFile *pDbFd = (winFile*)fd;
  winShm *pShm = pDbFd->pShm;
  win_Shm_ग्रन्थिः *p_Shm_ग्रन्थिः;
  DWORD protect = PAGE_READWRITE;
  DWORD flags = FILE_MAP_WRITE | FILE_MAP_READ;
  पूर्णाङ्क rc = SQLITE_OK;

  यदि( !pShm ){
    rc = win_Open_Shared_स्मृतिः(pDbFd);
    यदि( rc!=SQLITE_OK ) प्रत्यागम rc;
    pShm = pDbFd->pShm;
    प्रतिज्ञा( pShm!=0 );
  }
  p_Shm_ग्रन्थिः = pShm->p_Shm_ग्रन्थिः;

  sqlite3_mutex_enter(p_Shm_ग्रन्थिः->mutex);
  यदि( p_Shm_ग्रन्थिः->isUnlocked ){
    /* Take the DMS lock. */
    प्रतिज्ञा( p_Shm_ग्रन्थिः->nRegion==0 );
    rc = win_तालकम्_Shared_स्मृतिः(p_Shm_ग्रन्थिः, winFileBusyTimeout(pDbFd));
    यदि( rc!=SQLITE_OK ) गच्छ shmpage_बहिर्गमनम्;
  }

  प्रतिज्ञा( szRegion==p_Shm_ग्रन्थिः->szRegion || p_Shm_ग्रन्थिः->nRegion==0 );
  यदि( p_Shm_ग्रन्थिः->nRegion<=iRegion ){
    HANDLE hShared = p_Shm_ग्रन्थिः->hSharedShm;
    संरचना ShmRegion *apNew;           /* New aRegion[] array */
    i64 nByte = ((i64)iRegion+1)*(i64)szRegion;  /* Minimum file size */
    sqlite3_int64 sz;                  /* Current size of wal-index file */

    p_Shm_ग्रन्थिः->szRegion = szRegion;

    /* The requested region is not mapped into this processes address space.
    ** Check to see if it has been allocated (i.e. if the wal-index file is
    ** large enough to contain the requested region).
    */
    rc = win_Handle_आकार(hShared, &sz);
    यदि( rc!=SQLITE_OK ){
      rc = winLogError(rc, os_Get_चरमः_Error(), "winShmMap1", pDbFd->z_मार्गः);
      गच्छ shmpage_बहिर्गमनम्;
    }

    यदि( sz<nByte ){
      /* The requested memory region does not exist. If isWrite is set to
      ** zero, exit early. *pp will be set to NULL and SQLITE_OK returned.
      **
      ** Alternatively, if isWrite is non-zero, use ftruncate() to allocate
      ** the requested memory region.  */
      यदि( !isWrite ) गच्छ shmpage_बहिर्गमनम्;
      rc = winHandleTruncate(hShared, nByte);
      यदि( rc!=SQLITE_OK ){
        rc = winLogError(rc, os_Get_चरमः_Error(), "winShmMap2", pDbFd->z_मार्गः);
        गच्छ shmpage_बहिर्गमनम्;
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (संरचना ShmRegion*)sqlite3_realloc64(
        p_Shm_ग्रन्थिः->aRegion, ((i64)iRegion+1)*परिमाण(apNew[0])
    );
    यदि( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      गच्छ shmpage_बहिर्गमनम्;
    }
    p_Shm_ग्रन्थिः->aRegion = apNew;

    यदि( p_Shm_ग्रन्थिः->isReadonly ){
      protect = PAGE_READONLY;
      flags = FILE_MAP_READ;
    }

    यावत्( p_Shm_ग्रन्थिः->nRegion<=iRegion ){
      HANDLE hMap = रिक्त;         /* file-mapping handle */
      शून्य *pMap = 0;             /* Mapped memory region */

#यदि_परिभाषितम् SQLITE_UWP
      hMap = osCreateFileMappingFromApp(hShared, रिक्त, protect, nByte, रिक्त);
#अन्यथा
      hMap = osCreateFileMappingW(hShared, रिक्त, protect, 0, nByte, रिक्त);
#यदि_अन्तः
      OSTRACE(("SHM-MAP-CREATE pid=%lu, region=%d, size=%lld, rc=%s\n",
               osGetCurrentProcessId(), p_Shm_ग्रन्थिः->nRegion, nByte,
               hMap ? "ok" : "failed"));
      यदि( hMap ){
        i64 i_विस्थापनम् = p_Shm_ग्रन्थिः->nRegion*szRegion;
        पूर्णाङ्क i_विस्थापनम्_सरणम् = i_विस्थापनम् % winSysInfo.dw_आवण्टनम्_Granularity;
#यदि_परिभाषितम् SQLITE_UWP
        pMap = os_Map_View_सम्बन्धि_File_From_App(hMap, flags,
            i_विस्थापनम् - i_विस्थापनम्_सरणम्, (i64)szRegion + i_विस्थापनम्_सरणम्
        );
#अन्यथा
        pMap = os_Map_View_सम्बन्धि_File(hMap, flags,
            0, i_विस्थापनम् - i_विस्थापनम्_सरणम्, (i64)szRegion + i_विस्थापनम्_सरणम्
        );
#यदि_अन्तः
        OSTRACE(("SHM-MAP-MAP pid=%lu, region=%d, offset=%d, size=%d, rc=%s\n",
                 osGetCurrentProcessId(), p_Shm_ग्रन्थिः->nRegion, i_विस्थापनम्,
                 szRegion, pMap ? "ok" : "failed"));
      }
      यदि( !pMap ){
        p_Shm_ग्रन्थिः->चरमः_Errno = os_Get_चरमः_Error();
        rc = winLogError(SQLITE_IOERR_SHMMAP, p_Shm_ग्रन्थिः->चरमः_Errno,
                         "winShmMap3", pDbFd->z_मार्गः);
        यदि( hMap ) osCloseHandle(hMap);
        गच्छ shmpage_बहिर्गमनम्;
      }

      p_Shm_ग्रन्थिः->aRegion[p_Shm_ग्रन्थिः->nRegion].pMap = pMap;
      p_Shm_ग्रन्थिः->aRegion[p_Shm_ग्रन्थिः->nRegion].hMap = hMap;
      p_Shm_ग्रन्थिः->nRegion++;
    }
  }

shmpage_बहिर्गमनम्:
  यदि( p_Shm_ग्रन्थिः->nRegion>iRegion ){
    i64 i_विस्थापनम् = (i64)iRegion*(i64)szRegion;
    पूर्णाङ्क i_विस्थापनम्_सरणम् = i_विस्थापनम् % winSysInfo.dw_आवण्टनम्_Granularity;
    वर्ण *p = (वर्ण *)p_Shm_ग्रन्थिः->aRegion[iRegion].pMap;
    *pp = (शून्य *)&p[i_विस्थापनम्_सरणम्];
  }अन्यथा{
    *pp = 0;
  }
  यदि( p_Shm_ग्रन्थिः->isReadonly && rc==SQLITE_OK ){
    rc = SQLITE_READONLY;
  }
  sqlite3_mutex_leave(p_Shm_ग्रन्थिः->mutex);
  प्रत्यागम rc;
}

#अन्यथा
# define winShmMap     0
# define win_Shm_तालकम्    0
# define winShmBarrier 0
# define winShmUnmap   0
#यदि_अन्तः /* #ifndef SQLITE_OMIT_WAL */

/*
** Cleans up the mapped region of the specified file, if any.
*/
#यदि SQLITE_MAX_MMAP_आकार>0
स्थिर पूर्णाङ्क winUnmapfile(winFile *pFile){
  प्रतिज्ञा( pFile!=0 );
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, pMapRegion=%p, "
           "mmapSize=%lld, mmapSizeMax=%lld\n",
           osGetCurrentProcessId(), pFile, pFile->hMap, pFile->pMapRegion,
           pFile->mmap_आकार, pFile->mmap_आकार_Max));
  यदि( pFile->pMapRegion ){
    यदि( !os_Unmap_View_सम्बन्धि_File(pFile->pMapRegion) ){
      pFile->चरमः_Errno = os_Get_चरमः_Error();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(), pFile,
               pFile->pMapRegion));
      प्रत्यागम winLogError(SQLITE_IOERR_MMAP, pFile->चरमः_Errno,
                         "winUnmapfile1", pFile->z_मार्गः);
    }
    pFile->pMapRegion = 0;
    pFile->mmap_आकार = 0;
  }
  यदि( pFile->hMap!=रिक्त ){
    यदि( !osCloseHandle(pFile->hMap) ){
      pFile->चरमः_Errno = os_Get_चरमः_Error();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, rc=SQLITE_IOERR_MMAP\n",
               osGetCurrentProcessId(), pFile, pFile->hMap));
      प्रत्यागम winLogError(SQLITE_IOERR_MMAP, pFile->चरमः_Errno,
                         "winUnmapfile2", pFile->z_मार्गः);
    }
    pFile->hMap = रिक्त;
  }
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile));
  प्रत्यागम SQLITE_OK;
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
स्थिर पूर्णाङ्क winMapfile(winFile *pFd, sqlite3_int64 nByte){
  sqlite3_int64 nMap = nByte;
  पूर्णाङ्क rc;

  प्रतिज्ञा( nMap>=0 || pFd->n_Fetch_बहिर्गमनम्==0 );
  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, size=%lld\n",
           osGetCurrentProcessId(), pFd, nByte));

  यदि( pFd->n_Fetch_बहिर्गमनम्>0 ) प्रत्यागम SQLITE_OK;

  यदि( nMap<0 ){
    rc = win_File_आकार((sqlite3_file*)pFd, &nMap);
    यदि( rc ){
      OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_IOERR_FSTAT\n",
               osGetCurrentProcessId(), pFd));
      प्रत्यागम SQLITE_IOERR_FSTAT;
    }
  }
  यदि( nMap>pFd->mmap_आकार_Max ){
    nMap = pFd->mmap_आकार_Max;
  }
  nMap &= ~(sqlite3_int64)(winSysInfo.dw_Page_आकार - 1);

  यदि( nMap==0 && pFd->mmap_आकार>0 ){
    winUnmapfile(pFd);
  }
  यदि( nMap!=pFd->mmap_आकार ){
    शून्य *pNew = 0;
    DWORD protect = PAGE_READONLY;
    DWORD flags = FILE_MAP_READ;

    winUnmapfile(pFd);
#यदि_परिभाषितम् SQLITE_MMAP_READWRITE
    यदि( (pFd->ctrlFlags & WINFILE_RDONLY)==0 ){
      protect = PAGE_READWRITE;
      flags |= FILE_MAP_WRITE;
    }
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_UWP
    pFd->hMap = osCreateFileMappingFromApp(pFd->h, रिक्त, protect, nMap, रिक्त);
#अन्यथा
    pFd->hMap = osCreateFileMappingW(pFd->h, रिक्त, protect,
                                (DWORD)((nMap>>32) & 0xffffffff),
                                (DWORD)(nMap & 0xffffffff), रिक्त);
#यदि_अन्तः
    यदि( pFd->hMap==रिक्त ){
      pFd->चरमः_Errno = os_Get_चरमः_Error();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->चरमः_Errno,
                       "winMapfile1", pFd->z_मार्गः);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-CREATE pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      प्रत्यागम SQLITE_OK;
    }
    प्रतिज्ञा( (nMap % winSysInfo.dw_Page_आकार)==0 );
    प्रतिज्ञा( परिमाण(आकार_T)==परिमाण(sqlite3_int64) || nMap<=0xffffffff );
#यदि_परिभाषितम् SQLITE_UWP
    pNew = os_Map_View_सम्बन्धि_File_From_App(pFd->hMap, flags, 0, (आकार_T)nMap);
#अन्यथा
    pNew = os_Map_View_सम्बन्धि_File(pFd->hMap, flags, 0, 0, (आकार_T)nMap);
#यदि_अन्तः
    यदि( pNew==रिक्त ){
      osCloseHandle(pFd->hMap);
      pFd->hMap = रिक्त;
      pFd->चरमः_Errno = os_Get_चरमः_Error();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->चरमः_Errno,
                       "winMapfile2", pFd->z_मार्गः);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-MAP pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      प्रत्यागम SQLITE_OK;
    }
    pFd->pMapRegion = pNew;
    pFd->mmap_आकार = nMap;
  }

  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFd));
  प्रत्यागम SQLITE_OK;
}
#यदि_अन्तः /* SQLITE_MAX_MMAP_SIZE>0 */

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
स्थिर पूर्णाङ्क winFetch(sqlite3_file *fd, i64 iOff, पूर्णाङ्क nAmt, शून्य **pp){
#यदि SQLITE_MAX_MMAP_आकार>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */
#यदि_अन्तः
  *pp = 0;

  OSTRACE(("FETCH pid=%lu, pFile=%p, offset=%lld, amount=%d, pp=%p\n",
           osGetCurrentProcessId(), fd, iOff, nAmt, pp));

#यदि SQLITE_MAX_MMAP_आकार>0
  यदि( pFd->mmap_आकार_Max>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    ध्रुव पूर्णाङ्क n_Eof_बफर = 256;
    यदि( pFd->pMapRegion==0 ){
      पूर्णाङ्क rc = winMapfile(pFd, -1);
      यदि( rc!=SQLITE_OK ){
        OSTRACE(("FETCH pid=%lu, pFile=%p, rc=%s\n",
                 osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
        प्रत्यागम rc;
      }
    }
    यदि( pFd->mmap_आकार >= (iOff+nAmt+n_Eof_बफर) ){
      प्रतिज्ञा( pFd->pMapRegion!=0 );
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->n_Fetch_बहिर्गमनम्++;
    }
  }
#यदि_अन्तः

  OSTRACE(("FETCH pid=%lu, pFile=%p, pp=%p, *pp=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd, pp, *pp));
  प्रत्यागम SQLITE_OK;
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
स्थिर पूर्णाङ्क winUnfetch(sqlite3_file *fd, i64 iOff, शून्य *p){
#यदि SQLITE_MAX_MMAP_आकार>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  प्रतिज्ञा( (p==0)==(pFd->n_Fetch_बहिर्गमनम्==0) );

  /* If p!=0, it must match the iOff value. */
  प्रतिज्ञा( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, offset=%lld, p=%p\n",
           osGetCurrentProcessId(), pFd, iOff, p));

  यदि( p ){
    pFd->n_Fetch_बहिर्गमनम्--;
  }अन्यथा{
    /* FIXME:  If Windows truly always prevents truncating or deleting a
    ** file while a mapping is held, then the following winUnmapfile() call
    ** is unnecessary can be omitted - potentially improving
    ** performance.  */
    winUnmapfile(pFd);
  }

  प्रतिज्ञा( pFd->n_Fetch_बहिर्गमनम्>=0 );
#यदि_अन्तः

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd));
  प्रत्यागम SQLITE_OK;
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
स्थिर ध्रुव sqlite3_io_methods win_Io_विधानम् = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_आकार,                    /* xFileSize */
  win_तालकम्,                        /* xLock */
  winUnlock,                      /* xUnlock */
  win_Check_Reserved_तालकम्,           /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_आकार,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  win_Shm_तालकम्,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

/*
** This vector defines all the methods that can operate on an
** sqlite3_file for win32 without performing any locking.
*/
स्थिर ध्रुव sqlite3_io_methods win_Io_Nolock_विधानम् = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_आकार,                    /* xFileSize */
  win_Nolock_तालकम्,                  /* xLock */
  winNolockUnlock,                /* xUnlock */
  win_Nolock_Check_Reserved_तालकम्,     /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_आकार,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  win_Shm_तालकम्,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

स्थिर win_Vfs_App_दत्त win_App_दत्त = {
  &win_Io_विधानम्,       /* pMethod */
  0,                  /* pAppData */
  0                   /* bNoLock */
};

स्थिर win_Vfs_App_दत्त win_Nolock_App_दत्त = {
  &win_Io_Nolock_विधानम्, /* pMethod */
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
स्थिर पूर्णाङ्क win_Make_अन्तः_अन्तर्_Dir_Sep(पूर्णाङ्क nBuf, वर्ण *zBuf){
  यदि( zBuf ){
    पूर्णाङ्क nLen = sqlite3Strlen30(zBuf);
    यदि( nLen>0 ){
      यदि( winIsDirSep(zBuf[nLen-1]) ){
        प्रत्यागम 1;
      }अन्यथा यदि( nLen+1<nBuf ){
        यदि( !osGetenv ){
          zBuf[nLen] = winGetDirSep();
        }अन्यथा यदि( winIsDriveLetterAndColon(zBuf) && winIsDirSep(zBuf[2]) ){
          zBuf[nLen] = '\\';
          zBuf[2]='\\';
        }अन्यथा{
          zBuf[nLen] = '/';
        }
        zBuf[nLen+1] = '\0';
        प्रत्यागम 1;
      }
    }
  }
  प्रत्यागम 0;
}

/*
** If sqlite3_temp_directory is defined, take the mutex and return true.
**
** If sqlite3_temp_directory is NULL (undefined), omit the mutex and
** return false.
*/
स्थिर पूर्णाङ्क win_अस्थायी_Dir_Defined(शून्य){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  यदि( sqlite3_अस्थायी_directory!=0 ) प्रत्यागम 1;
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  प्रत्यागम 0;
}

/*
** Create a temporary file name and store the resulting pointer into pzBuf.
** The pointer returned in pzBuf must be freed via sqlite3_free().
*/
स्थिर पूर्णाङ्क winGetTempname(sqlite3_vfs *pVfs, वर्ण **pzBuf){
  स्थिर ध्रुव वर्ण zChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  size_t i, j;
  DWORD pid;
  पूर्णाङ्क nPre = sqlite3Strlen30(SQLITE_अस्थायी_FILE_उपसर्गः);
  i64 nMax, nBuf, nDir, nLen;
  वर्ण *zBuf;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  SimulateIOError( प्रत्यागम SQLITE_IOERR );

  /* Allocate a temporary buffer to store the fully qualified file
  ** name for the temporary file.  If this fails, we cannot continue.
  */
  nMax = pVfs->mxPathname;
  nBuf = 2 + (i64)nMax;
  zBuf = sqlite3MallocZero( nBuf );
  यदि( !zBuf ){
    OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
    प्रत्यागम SQLITE_IOERR_NOMEM_BKPT;
  }

  /* Figure out the effective temporary directory.  First, check if one
  ** has been explicitly set by the application; otherwise, use the one
  ** configured by the operating system.
  */
  nDir = nMax - (nPre + 15);
  प्रतिज्ञा( nDir>0 );
  यदि( win_अस्थायी_Dir_Defined() ){
    पूर्णाङ्क nDirLen = sqlite3Strlen30(sqlite3_अस्थायी_directory);
    यदि( nDirLen>0 ){
      यदि( !winIsDirSep(sqlite3_अस्थायी_directory[nDirLen-1]) ){
        nDirLen++;
      }
      यदि( nDirLen>nDir ){
        sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
        sqlite3_free(zBuf);
        OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
        प्रत्यागम winLogError(SQLITE_ERROR, 0, "winGetTempname1", 0);
      }
      sqlite3_snprintf(nMax, zBuf, "%s", sqlite3_अस्थायी_directory);
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  }

#यदि defined(__CYGWIN__)
  अन्यथा यदि( osGetenv!=रिक्त ){
    स्थिर ध्रुव वर्ण *azDirs[] = {
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
    अचिह्नित पूर्णाङ्क i;
    ध्रुव वर्ण *zDir = 0;

    यदि( !azDirs[0] ) azDirs[0] = osGetenv("SQLITE_TMPDIR");
    यदि( !azDirs[1] ) azDirs[1] = osGetenv("TMPDIR");
    यदि( !azDirs[2] ) azDirs[2] = osGetenv("TMP");
    यदि( !azDirs[3] ) azDirs[3] = osGetenv("TEMP");
    यदि( !azDirs[4] ) azDirs[4] = osGetenv("USERPROFILE");
    प्रति(i=0; i<परिमाण(azDirs)/परिमाण(azDirs[0]); zDir=azDirs[i++]){
      LPWSTR zConverted;
      यदि( zDir==0 ) अनुवर्त;
      /* If the path starts with a drive letter followed by the colon
      ** character, assume it is already a native Win32 path; otherwise,
      ** it must be converted to a native Win32 path via the Cygwin API
      ** prior to using it.
      */
      {
        zConverted = winConvertFromUtf8Filename(zDir);
        यदि( !zConverted ){
          sqlite3_free(zBuf);
          OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
          प्रत्यागम SQLITE_IOERR_NOMEM_BKPT;
        }
        यदि( winIsDir(zConverted) ){
          sqlite3_snprintf(nMax, zBuf, "%s", zDir);
          sqlite3_free(zConverted);
          भङ्ग;
        }
        sqlite3_free(zConverted);
      }
    }
  }
#यदि_अन्तः

  अन्यथा{
    वर्ण *zMulti;
    LPWSTR z_Wide_मार्गः = sqlite3MallocZero( nMax*परिमाण(WCHAR) );
    यदि( !z_Wide_मार्गः ){
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      प्रत्यागम SQLITE_IOERR_NOMEM_BKPT;
    }
    यदि( os_Get_अस्थायी_मार्गः_W==0 || os_Get_अस्थायी_मार्गः_W(nMax, z_Wide_मार्गः)==0 ){
      sqlite3_free(z_Wide_मार्गः);
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_GETTEMPPATH\n"));
      प्रत्यागम winLogError(SQLITE_IOERR_GETTEMPPATH, os_Get_चरमः_Error(),
                         "winGetTempname2", 0);
    }
    zMulti = win_Unicode_गन्तव्यम्_Utf8(z_Wide_मार्गः);
    sqlite3_free(z_Wide_मार्गः);
    यदि( zMulti ){
      sqlite3_snprintf(nMax, zBuf, "%s", zMulti);
      sqlite3_free(zMulti);
    }अन्यथा{
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      प्रत्यागम SQLITE_IOERR_NOMEM_BKPT;
    }
  }

  /*
  ** Check to make sure the temporary directory ends with an appropriate
  ** separator.  If it does not and there is not enough space left to add
  ** one, fail.
  */
  यदि( !win_Make_अन्तः_अन्तर्_Dir_Sep(nDir+1, zBuf) ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    प्रत्यागम winLogError(SQLITE_ERROR, 0, "winGetTempname4", 0);
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
  यदि( (nLen + nPre + 17) > nBuf ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    प्रत्यागम winLogError(SQLITE_ERROR, 0, "winGetTempname5", 0);
  }

  sqlite3_snprintf(nBuf-16-nLen, zBuf+nLen, SQLITE_अस्थायी_FILE_उपसर्गः);

  j = sqlite3Strlen30(zBuf);
  sqlite3_randomness(15, &zBuf[j]);
  pid = osGetCurrentProcessId();
  प्रति(i=0; i<15; i++, j++){
    zBuf[j] += pid & 0xff;
    pid >>= 8;
    zBuf[j] = (वर्ण)zChars[ ((अचिह्नित वर्ण)zBuf[j])%(परिमाण(zChars)-1) ];
  }
  zBuf[j] = 0;
  zBuf[j+1] = 0;
  *pzBuf = zBuf;

  OSTRACE(("TEMP-FILENAME name=%s, rc=SQLITE_OK\n", zBuf));
  प्रत्यागम SQLITE_OK;
}

/*
** Return TRUE if the named file is really a directory.  Return false if
** it is something other than a directory, or if there is any kind of memory
** allocation failure.
*/
स्थिर पूर्णाङ्क winIsDir(LPCWSTR zConverted){
  DWORD attr;
  पूर्णाङ्क rc = 0;
  DWORD चरमः_Errno;
  पूर्णाङ्क cnt = 0;
  WIN32_FILE_ATTRIBUTE_दत्त s_Attr_दत्त;
  स्मृतिं_पूरय(&s_Attr_दत्त, 0, परिमाण(s_Attr_दत्त));
  यावत्( !(rc = os_Get_File_गुणाः_Ex_W(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_दत्त)) && winRetryIoerr(&cnt, &चरमः_Errno) ){}
  यदि( !rc ){
    प्रत्यागम 0; /* Invalid name? */
  }
  attr = s_Attr_दत्त.dw_File_गुणाः;
  प्रत्यागम (attr!=INVALID_FILE_गुणाः) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

/* forward reference */
स्थिर पूर्णाङ्क winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ध्रुव वर्ण *zFilename,     /* Name of file to check */
  पूर्णाङ्क flags,                 /* Type of test to make on this file */
  पूर्णाङ्क *p_Res_बहिर्गमनम्               /* OUT: Result */
);

/*
** The Windows version of xAccess() accepts an extra bit in the flags
** parameter that prevents an anti-virus retry loop.
*/
#परिभाषय NORETRY 0x4000

/*
** Open a file.
*/
स्थिर पूर्णाङ्क winOpen(
  sqlite3_vfs *pVfs,        /* Used to get maximum path length and AppData */
  ध्रुव वर्ण *zName,        /* Name of the file (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  पूर्णाङ्क flags,                /* Open mode flags */
  पूर्णाङ्क *p_बहिर्गमनम्_Flags            /* Status return flags */
){
  HANDLE h;
  DWORD चरमः_Errno = 0;
  DWORD dwDesiredAccess;
  DWORD dw_Share_बहुलकम्;
  DWORD dwCreationDisposition;
  DWORD dw_Flags_And_गुणाः = 0;
  win_Vfs_App_दत्त *p_App_दत्त;
  winFile *pFile = (winFile*)id;
  LPWSTR zConverted;              /* Filename in OS encoding */
  ध्रुव वर्ण *zUtf8Name = zName; /* Filename in UTF-8 encoding */
  पूर्णाङ्क cnt = 0;
  पूर्णाङ्क isRO = 0;              /* file is known to be accessible readonly */

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  वर्ण *zTmpname = 0; /* For temporary filename, if necessary. */

  पूर्णाङ्क rc = SQLITE_OK;            /* Function Return Code */
#यदि !defined(NDEBUG)
  पूर्णाङ्क eType = flags&0x0FFF00;  /* Type of file to open */
#यदि_अन्तः

  पूर्णाङ्क isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  पूर्णाङ्क isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  पूर्णाङ्क isCreate     = (flags & SQLITE_OPEN_CREATE);
  पूर्णाङ्क isReadonly   = (flags & SQLITE_OPEN_READONLY);
  पूर्णाङ्क isReadWrite  = (flags & SQLITE_OPEN_READWRITE);

#यदि_अपरिभाषितम् NDEBUG
  पूर्णाङ्क isOpenJournal = (isCreate && (
        eType==SQLITE_OPEN_SUPER_JOURNAL
     || eType==SQLITE_OPEN_MAIN_JOURNAL
     || eType==SQLITE_OPEN_WAL
  ));
#यदि_अन्तः

  OSTRACE(("OPEN name=%s, pFile=%p, flags=%x, pOutFlags=%p\n",
           zUtf8Name, id, flags, p_बहिर्गमनम्_Flags));

  /* Check the following statements are true:
  **
  **   (a) Exactly one of the READWRITE and READONLY flags must be set, and
  **   (b) if CREATE is set, then READWRITE must also be set, and
  **   (c) if EXCLUSIVE is set, then CREATE must also be set.
  **   (d) if DELETEONCLOSE is set, then CREATE must also be set.
  */
  प्रतिज्ञा((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
  प्रतिज्ञा(isCreate==0 || isReadWrite);
  प्रतिज्ञा(isExclusive==0 || isCreate);
  प्रतिज्ञा(isDelete==0 || isCreate);

  /* The main DB, main journal, WAL file and super-journal are never
  ** automatically deleted. Nor are they ever temporary files.  */
  प्रतिज्ञा( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_DB );
  प्रतिज्ञा( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_JOURNAL );
  प्रतिज्ञा( (!isDelete && zName) || eType!=SQLITE_OPEN_SUPER_JOURNAL );
  प्रतिज्ञा( (!isDelete && zName) || eType!=SQLITE_OPEN_WAL );

  /* Assert that the upper layer has set one of the "file-type" flags. */
  प्रतिज्ञा( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_अस्थायी_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_अस्थायी_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );

  प्रतिज्ञा( pFile!=0 );
  स्मृतिं_पूरय(pFile, 0, परिमाण(winFile));
  pFile->h = INVALID_HANDLE_मानम्;

  /* If the second argument to this function is NULL, generate a
  ** temporary file name to use
  */
  यदि( !zUtf8Name ){
    प्रतिज्ञा( isDelete && !isOpenJournal );
    rc = winGetTempname(pVfs, &zTmpname);
    यदि( rc!=SQLITE_OK ){
      OSTRACE(("OPEN name=%s, rc=%s", zUtf8Name, sqlite3ErrName(rc)));
      प्रत्यागम rc;
    }
    zUtf8Name = zTmpname;
  }

  /* Database filenames are double-zero terminated if they are not
  ** URIs with parameters.  Hence, they can always be passed into
  ** sqlite3_uri_parameter().
  */
  प्रतिज्ञा( (eType!=SQLITE_OPEN_MAIN_DB) || (flags & SQLITE_OPEN_URI) ||
       zUtf8Name[sqlite3Strlen30(zUtf8Name)+1]==0 );

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8Name);
  यदि( zConverted==0 ){
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8Name));
    प्रत्यागम SQLITE_IOERR_NOMEM_BKPT;
  }

  यदि( winIsDir(zConverted) ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8Name));
    प्रत्यागम SQLITE_CANTOPEN_ISDIR;
  }

  यदि( isReadWrite ){
    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
  }अन्यथा{
    dwDesiredAccess = GENERIC_READ;
  }

  /* SQLITE_OPEN_EXCLUSIVE is used to make sure that a new file is
  ** created. SQLite doesn't use it to indicate "exclusive access"
  ** as it is usually understood.
  */
  यदि( isExclusive ){
    /* Creates a new file, only if it does not already exist. */
    /* If the file exists, it fails. */
    dwCreationDisposition = CREATE_NEW;
  }अन्यथा यदि( isCreate ){
    /* Open existing file, or create if it doesn't exist */
    dwCreationDisposition = OPEN_ALWAYS;
  }अन्यथा{
    /* Opens a file, only if it exists. */
    dwCreationDisposition = OPEN_EXISTING;
  }

  यदि( 0==sqlite3_uri_boolean(zName, "exclusive", 0) ){
    dw_Share_बहुलकम् = FILE_SHARE_READ | FILE_SHARE_WRITE;
  }अन्यथा{
    dw_Share_बहुलकम् = 0;
  }

  यदि( isDelete ){
    dw_Flags_And_गुणाः = FILE_ATTRIBUTE_TEMPORARY
                               | FILE_ATTRIBUTE_HIDDEN
                               | FILE_FLAG_DELETE_ON_CLOSE;
  }अन्यथा{
    dw_Flags_And_गुणाः = FILE_ATTRIBUTE_प्रसामान्यम्;
  }
  कुरु{
    h = osCreateFileW(zConverted,
                      dwDesiredAccess,
                      dw_Share_बहुलकम्, रिक्त,
                      dwCreationDisposition,
                      dw_Flags_And_गुणाः,
                      रिक्त);
    यदि( h!=INVALID_HANDLE_मानम् ) भङ्ग;
    यदि( isReadWrite ){
      पूर्णाङ्क rc2;
      sqlite3BeginBenignMalloc();
      rc2 = winAccess(pVfs, zUtf8Name, SQLITE_ACCESS_READ|NORETRY, &isRO);
      sqlite3_अन्तः_Benign_Malloc();
      यदि( rc2==SQLITE_OK && isRO ) भङ्ग;
    }
  }यावत्( winRetryIoerr(&cnt, &चरमः_Errno) );
  winLogIoerr(cnt, __LINE__);

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, rc=%s\n", h, zUtf8Name,
           dwDesiredAccess, (h==INVALID_HANDLE_मानम्) ? "failed" : "ok"));

  यदि( h==INVALID_HANDLE_मानम् ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    यदि( isReadWrite && isRO && !isExclusive ){
      प्रत्यागम winOpen(pVfs, zName, id,
         ((flags|SQLITE_OPEN_READONLY) &
                     ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE)),
         p_बहिर्गमनम्_Flags);
    }अन्यथा{
      pFile->चरमः_Errno = चरमः_Errno;
      winLogError(SQLITE_CANTOPEN, pFile->चरमः_Errno, "winOpen", zUtf8Name);
      प्रत्यागम SQLITE_CANTOPEN_BKPT;
    }
  }

  यदि( p_बहिर्गमनम्_Flags ){
    यदि( isReadWrite ){
      *p_बहिर्गमनम्_Flags = SQLITE_OPEN_READWRITE;
    }अन्यथा{
      *p_बहिर्गमनम्_Flags = SQLITE_OPEN_READONLY;
    }
  }

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, pOutFlags=%p, *pOutFlags=%d, "
           "rc=%s\n", h, zUtf8Name, dwDesiredAccess, p_बहिर्गमनम्_Flags, p_बहिर्गमनम्_Flags ?
           *p_बहिर्गमनम्_Flags : 0, (h==INVALID_HANDLE_मानम्) ? "failed" : "ok"));

  p_App_दत्त = (win_Vfs_App_दत्त*)pVfs->p_App_दत्त;

  sqlite3_free(zConverted);
  sqlite3_free(zTmpname);
  id->pMethods = p_App_दत्त ? p_App_दत्त->p_विधानम् : &win_Io_विधानम्;
  pFile->pVfs = pVfs;
  pFile->h = h;
  यदि( isReadonly ){
    pFile->ctrlFlags |= WINFILE_RDONLY;
  }
  यदि( (flags & SQLITE_OPEN_MAIN_DB)
   && sqlite3_uri_boolean(zName, "psow", SQLITE_POWERSAFE_OVERWRITE) 
  ){
    pFile->ctrlFlags |= WINFILE_PSOW;
  }
  pFile->चरमः_Errno = NO_ERROR;
  pFile->z_मार्गः = zName;
#यदि SQLITE_MAX_MMAP_आकार>0
  pFile->hMap = रिक्त;
  pFile->pMapRegion = 0;
  pFile->mmap_आकार = 0;
  pFile->mmap_आकार_Max = sqlite3GlobalConfig.szMmap;
#यदि_अन्तः

  Open_गणक(+1);
  प्रत्यागम rc;
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
स्थिर पूर्णाङ्क winDelete(
  sqlite3_vfs *pVfs,          /* Not used on win32 */
  ध्रुव वर्ण *zFilename,      /* Name of file to delete */
  पूर्णाङ्क syncDir                 /* Not used on win32 */
){
  पूर्णाङ्क cnt = 0;
  पूर्णाङ्क rc;
  DWORD attr;
  DWORD चरमः_Errno = 0;
  LPWSTR zConverted;
  अप्रयुक्तम्_PARAMETER(pVfs);
  अप्रयुक्तम्_PARAMETER(syncDir);

  SimulateIOError(प्रत्यागम SQLITE_IOERR_DELETE);
  OSTRACE(("DELETE name=%s, syncDir=%d\n", zFilename, syncDir));

  zConverted = winConvertFromUtf8Filename(zFilename);
  यदि( zConverted==0 ){
    OSTRACE(("DELETE name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    प्रत्यागम SQLITE_IOERR_NOMEM_BKPT;
  }
  कुरु {
    attr = os_Get_File_गुणाः_W(zConverted);
    यदि ( attr==INVALID_FILE_गुणाः ){
      चरमः_Errno = os_Get_चरमः_Error();
      यदि( चरमः_Errno==ERROR_FILE_NOT_प्राप्तम्
       || चरमः_Errno==ERROR_मार्गः_NOT_प्राप्तम् ){
        rc = SQLITE_IOERR_DELETE_NOENT; /* Already gone? */
      }अन्यथा{
        rc = SQLITE_ERROR;
      }
      भङ्ग;
    }
    यदि ( attr&FILE_ATTRIBUTE_DIRECTORY ){
      rc = SQLITE_ERROR; /* Files only. */
      भङ्ग;
    }
    यदि ( osDeleteFileW(zConverted) ){
      rc = SQLITE_OK; /* Deleted OK. */
      भङ्ग;
    }
    यदि ( !winRetryIoerr(&cnt, &चरमः_Errno) ){
      rc = SQLITE_ERROR; /* No more retries. */
      भङ्ग;
    }
  } यावत्(1);
  यदि( rc && rc!=SQLITE_IOERR_DELETE_NOENT ){
    rc = winLogError(SQLITE_IOERR_DELETE, चरमः_Errno, "winDelete", zFilename);
  }अन्यथा{
    winLogIoerr(cnt, __LINE__);
  }
  sqlite3_free(zConverted);
  OSTRACE(("DELETE name=%s, rc=%s\n", zFilename, sqlite3ErrName(rc)));
  प्रत्यागम rc;
}

/*
** Check the existence and status of a file.
*/
स्थिर पूर्णाङ्क winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ध्रुव वर्ण *zFilename,     /* Name of file to check */
  पूर्णाङ्क flags,                 /* Type of test to make on this file */
  पूर्णाङ्क *p_Res_बहिर्गमनम्               /* OUT: Result */
){
  DWORD attr;
  पूर्णाङ्क rc = 0;
  पूर्णाङ्क cnt = 0;
  DWORD चरमः_Errno = 0;
  LPWSTR zConverted;
  पूर्णाङ्क noRetry = 0;           /* Do not use winRetryIoerr() */
  WIN32_FILE_ATTRIBUTE_दत्त s_Attr_दत्त;
  अप्रयुक्तम्_PARAMETER(pVfs);

  यदि( (flags & NORETRY)!=0 ){
    noRetry = 1;
    flags &= ~NORETRY;
  }  

  SimulateIOError( प्रत्यागम SQLITE_IOERR_ACCESS; );
  OSTRACE(("ACCESS name=%s, flags=%x, pResOut=%p\n",
           zFilename, flags, p_Res_बहिर्गमनम्));

  यदि( zFilename==0 ){
    *p_Res_बहिर्गमनम् = 0;
    OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
             zFilename, p_Res_बहिर्गमनम्, *p_Res_बहिर्गमनम्));
    प्रत्यागम SQLITE_OK;
  }

  zConverted = winConvertFromUtf8Filename(zFilename);
  यदि( zConverted==0 ){
    OSTRACE(("ACCESS name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    प्रत्यागम SQLITE_IOERR_NOMEM_BKPT;
  }
  स्मृतिं_पूरय(&s_Attr_दत्त, 0, परिमाण(s_Attr_दत्त));
  यावत्( !(rc = os_Get_File_गुणाः_Ex_W(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_दत्त))
     && !noRetry
     && winRetryIoerr(&cnt, &चरमः_Errno)
  ){ /* Loop until true */}
  यदि( rc ){
    /* For an SQLITE_ACCESS_EXISTS query, treat a zero-length file
    ** as if it does not exist.
    */
    यदि(    flags==SQLITE_ACCESS_EXISTS
        && s_Attr_दत्त.n_File_आकार_उच्चः==0
        && s_Attr_दत्त.n_File_आकार_नीचः==0 ){
      attr = INVALID_FILE_गुणाः;
    }अन्यथा{
      attr = s_Attr_दत्त.dw_File_गुणाः;
    }
  }अन्यथा{
    यदि( noRetry ) चरमः_Errno = os_Get_चरमः_Error();
    winLogIoerr(cnt, __LINE__);
    यदि( चरमः_Errno!=ERROR_FILE_NOT_प्राप्तम् && चरमः_Errno!=ERROR_मार्गः_NOT_प्राप्तम् ){
      sqlite3_free(zConverted);
      प्रत्यागम winLogError(SQLITE_IOERR_ACCESS, चरमः_Errno, "winAccess",
                         zFilename);
    }अन्यथा{
      attr = INVALID_FILE_गुणाः;
    }
  }
  sqlite3_free(zConverted);
  विकल्प( flags ){
    पक्ष SQLITE_ACCESS_READ:
    पक्ष SQLITE_ACCESS_EXISTS:
      rc = attr!=INVALID_FILE_गुणाः;
      भङ्ग;
    पक्ष SQLITE_ACCESS_READWRITE:
      rc = attr!=INVALID_FILE_गुणाः &&
             (attr & FILE_ATTRIBUTE_READONLY)==0;
      भङ्ग;
    स्वभाव:
      प्रतिज्ञा(!"Invalid flags argument");
  }
  *p_Res_बहिर्गमनम् = rc;
  OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           zFilename, p_Res_बहिर्गमनम्, *p_Res_बहिर्गमनम्));
  प्रत्यागम SQLITE_OK;
}

/*
** Returns non-zero if the specified path name starts with the "long path"
** prefix.
*/
स्थिर BOOL win_Is_Long_मार्गः_उपसर्गः(
  ध्रुव वर्ण *zPathname
){
  प्रत्यागम ( zPathname[0]=='\\' && zPathname[1]=='\\'
        && zPathname[2]=='?'  && zPathname[3]=='\\' );
}

/*
** Returns non-zero if the specified path name starts with a drive letter
** followed by a colon character.
*/
स्थिर BOOL winIsDriveLetterAndColon(
  ध्रुव वर्ण *zPathname
){
  प्रत्यागम ( sqlite3Isalpha(zPathname[0]) && zPathname[1]==':' );
}

#यदि_परिभाषितम् _WIN32
/*
** Returns non-zero if the specified path name should be used verbatim.  If
** non-zero is returned from this function, the calling function must simply
** use the provided path name verbatim -OR- resolve it into a full path name
** using the GetFullPathName Win32 API function (if available).
*/
स्थिर BOOL winIsVerbatimPathname(
  ध्रुव वर्ण *zPathname
){
  /*
  ** If the path name starts with a forward slash or a backslash, it is either
  ** a legal UNC name, a volume relative path, or an absolute path name in the
  ** "Unix" format on Windows.  There is no easy way to differentiate between
  ** the final two cases; therefore, we return the safer return value of TRUE
  ** so that callers of this function will simply use it verbatim.
  */
  यदि ( winIsDirSep(zPathname[0]) ){
    प्रत्यागम TRUE;
  }

  /*
  ** If the path name starts with a letter and a colon it is either a volume
  ** relative path or an absolute path.  Callers of this function must not
  ** attempt to treat it as a relative path name (i.e. they should simply use
  ** it verbatim).
  */
  यदि ( winIsDriveLetterAndColon(zPathname) ){
    प्रत्यागम TRUE;
  }

  /*
  ** If we get to this point, the path name should almost certainly be a purely
  ** relative one (i.e. not a UNC name, not absolute, and not volume relative).
  */
  प्रत्यागम FALSE;
}
#यदि_अन्तः /* _WIN32 */

#यदि_परिभाषितम् __CYGWIN__
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
स्थिर शून्य winSimplifyName(वर्ण *z){
  पूर्णाङ्क i, j;
  प्रति(i=j=0; z[i]; ++i){
    यदि( winIsDirSep(z[i]) ){
#यदि !defined(SQLITE_TEST)
      /* Some test-cases assume that "./foo" and "foo" are different */
      यदि( z[i+1]=='.' && winIsDirSep(z[i+2]) ){
        ++i;
        अनुवर्त;
      }
#यदि_अन्तः
      यदि( !z[i+1] || (winIsDirSep(z[i+1]) && (i!=0)) ){
        अनुवर्त;
      }
      z[j++] = osGetenv?'/':'\\';
    }अन्यथा{
      z[j++] = z[i];
    }
  }
  यावत्(j<i) z[j++] = '\0';
}

#परिभाषय SQLITE_MAX_SYMLINKS 100

स्थिर पूर्णाङ्क mk_पूर्णम्_Pathname(
  ध्रुव वर्ण *z_मार्गः,              /* Input path */
  वर्ण *z_बहिर्गमनम्,                     /* Output buffer */
  पूर्णाङ्क n_बहिर्गमनम्                        /* Allocated size of buffer zOut */
){
  पूर्णाङ्क n_मार्गः = sqlite3Strlen30(z_मार्गः);
  पूर्णाङ्क iOff = 0;
  यदि( z_मार्गः[0]!='/' ){
    यदि( osGetcwd(z_बहिर्गमनम्, n_बहिर्गमनम्-2)==0 ){
      प्रत्यागम winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno, "getcwd", z_मार्गः);
    }
    iOff = sqlite3Strlen30(z_बहिर्गमनम्);
    z_बहिर्गमनम्[iOff++] = '/';
  }
  यदि( (iOff+n_मार्गः+1)>n_बहिर्गमनम् ){
    /* SQLite assumes that xFullPathname() nul-terminates the output buffer
    ** even if it returns an error.  */
    z_बहिर्गमनम्[iOff] = '\0';
    प्रत्यागम SQLITE_CANTOPEN_BKPT;
  }
  sqlite3_snprintf(n_बहिर्गमनम्-iOff, &z_बहिर्गमनम्[iOff], "%s", z_मार्गः);
  प्रत्यागम SQLITE_OK;
}
#यदि_अन्तः /* __CYGWIN__ */

/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
स्थिर पूर्णाङ्क win_पूर्णम्_Pathname_No_Mutex(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ध्रुव वर्ण *zRelative,        /* Possibly relative input path */
  पूर्णाङ्क n_पूर्णम्,                    /* Size of output buffer in bytes */
  वर्ण *z_पूर्णम्                   /* Output buffer */
){
  पूर्णाङ्क nByte;
  LPWSTR zConverted;
  वर्ण *z_बहिर्गमनम्;

  /* If this path name begins with "/X:" or "\\?\", where "X" is any
  ** alphabetic character, discard the initial "/" from the pathname.
  */
  यदि( zRelative[0]=='/' && (winIsDriveLetterAndColon(zRelative+1)
       || win_Is_Long_मार्गः_उपसर्गः(zRelative+1)) ){
    zRelative++;
  }

  SimulateIOError( प्रत्यागम SQLITE_ERROR );

#यदि_परिभाषितम् __CYGWIN__
  यदि( osGetcwd ){
    z_पूर्णम्[n_पूर्णम्-1] = '\0';
    यदि( !winIsDriveLetterAndColon(zRelative) || !winIsDirSep(zRelative[2]) ){
      पूर्णाङ्क rc = SQLITE_OK;
      पूर्णाङ्क nLink = 1;              /* Number of symbolic links followed so far */
      ध्रुव वर्ण *z_अन्तर् = zRelative; /* Input path for each iteration of loop */
      वर्ण *zDel = 0;
      संरचना stat buf;

      अप्रयुक्तम्_PARAMETER(pVfs);

      कुरु {
        /* Call lstat() on path zIn. Set bLink to true if the path
        ** is a symbolic link, or false otherwise.  */
        पूर्णाङ्क bLink = 0;
        यदि( osLstat && osReadlink ) {
          यदि( osLstat(z_अन्तर्, &buf)!=0 ){
            पूर्णाङ्क myErrno = osErrno;
            यदि( myErrno!=ENOENT ){
              rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)myErrno,
                               "lstat", z_अन्तर्);
            }
          }अन्यथा{
            bLink = ((buf.st_बहुलकम् & 0170000) == 0120000);
          }

          यदि( bLink ){
            यदि( zDel==0 ){
              zDel = sqlite3MallocZero(n_पूर्णम्);
              यदि( zDel==0 ) rc = SQLITE_NOMEM;
            }अन्यथा यदि( ++nLink>SQLITE_MAX_SYMLINKS ){
              rc = SQLITE_CANTOPEN_BKPT;
            }

            यदि( rc==SQLITE_OK ){
              nByte = osReadlink(z_अन्तर्, zDel, n_पूर्णम्-1);
              यदि( nByte ==(DWORD)-1 ){
                rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno,
                                 "readlink", z_अन्तर्);
              }अन्यथा{
                यदि( zDel[0]!='/' ){
                  पूर्णाङ्क n;
                  प्रति(n = sqlite3Strlen30(z_अन्तर्); n>0 && z_अन्तर्[n-1]!='/'; n--);
                  यदि( nByte+n+1>n_पूर्णम् ){
                    rc = SQLITE_CANTOPEN_BKPT;
                  }अन्यथा{
                    memmove(&zDel[n], zDel, nByte+1);
                    स्मृतिं_प्रतिलिख(zDel, z_अन्तर्, n);
                    nByte += n;
                  }
                }
                zDel[nByte] = '\0';
              }
            }

            z_अन्तर् = zDel;
          }
        }

        प्रतिज्ञा( rc!=SQLITE_OK || z_अन्तर्!=z_पूर्णम् || z_अन्तर्[0]=='/' );
        यदि( rc==SQLITE_OK && z_अन्तर्!=z_पूर्णम् ){
          rc = mk_पूर्णम्_Pathname(z_अन्तर्, z_पूर्णम्, n_पूर्णम्);
        }
        यदि( bLink==0 ) भङ्ग;
        z_अन्तर् = z_पूर्णम्;
      }यावत्( rc==SQLITE_OK );

      sqlite3_free(zDel);
      winSimplifyName(z_पूर्णम्);
      प्रत्यागम rc;
    }
  }
#यदि_अन्तः /* __CYGWIN__ */

#यदि defined(_WIN32)
  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing. This function could fail if, for example, the
  ** current working directory has been unlinked.
  */
  SimulateIOError( प्रत्यागम SQLITE_ERROR );
  यदि ( sqlite3_दत्त_directory && !winIsVerbatimPathname(zRelative) ){
    /*
    ** NOTE: We are dealing with a relative path name and the data
    **       directory has been set.  Therefore, use it as the basis
    **       for converting the relative path name to an absolute
    **       one by prepending the data directory and a backslash.
    */
    sqlite3_snprintf(MIN(n_पूर्णम्, pVfs->mxPathname), z_पूर्णम्, "%s%c%s",
                     sqlite3_दत्त_directory, winGetDirSep(), zRelative);
    प्रत्यागम SQLITE_OK;
  }
#यदि_अन्तः
  zConverted = winConvertFromUtf8Filename(zRelative);
  यदि( zConverted==0 ){
    प्रत्यागम SQLITE_IOERR_NOMEM_BKPT;
  }
  {
    LPWSTR z_अस्थायी;
    nByte = os_Get_पूर्णम्_मार्गः_Name_W(zConverted, 0, 0, 0);
    यदि( nByte==0 ){
      sqlite3_free(zConverted);
      प्रत्यागम winLogError(SQLITE_CANTOPEN_FULLPATH, os_Get_चरमः_Error(),
                         "winFullPathname1", zRelative);
    }
    nByte += 3;
    z_अस्थायी = sqlite3MallocZero( (u64)nByte*परिमाण(WCHAR) );
    यदि( z_अस्थायी==0 ){
      sqlite3_free(zConverted);
      प्रत्यागम SQLITE_IOERR_NOMEM_BKPT;
    }
    nByte = os_Get_पूर्णम्_मार्गः_Name_W(zConverted, nByte, z_अस्थायी, 0);
    यदि( nByte==0 ){
      sqlite3_free(zConverted);
      sqlite3_free(z_अस्थायी);
      प्रत्यागम winLogError(SQLITE_CANTOPEN_FULLPATH, os_Get_चरमः_Error(),
                         "winFullPathname2", zRelative);
    }
    sqlite3_free(zConverted);
    z_बहिर्गमनम् = win_Unicode_गन्तव्यम्_Utf8(z_अस्थायी);
    sqlite3_free(z_अस्थायी);
  }
  यदि( z_बहिर्गमनम् ){
#यदि_परिभाषितम् __CYGWIN__
    यदि( memcmp(z_बहिर्गमनम्, "\\\\?\\", 4) ){
      sqlite3_snprintf(MIN(n_पूर्णम्, pVfs->mxPathname), z_पूर्णम्, "%s", z_बहिर्गमनम्);
    }अन्यथा यदि( memcmp(z_बहिर्गमनम्+4, "UNC\\", 4) ){
      sqlite3_snprintf(MIN(n_पूर्णम्, pVfs->mxPathname), z_पूर्णम्, "%s", z_बहिर्गमनम्+4);
    }अन्यथा{
      वर्ण *p = z_बहिर्गमनम्+6;
      *p = '\\';
      यदि( osGetcwd ){
        /* On Cygwin, UNC paths use forward slashes */
        यावत्( *p ){
          यदि( *p=='\\' ) *p = '/';
          ++p;
        }
      }
      sqlite3_snprintf(MIN(n_पूर्णम्, pVfs->mxPathname), z_पूर्णम्, "%s", z_बहिर्गमनम्+6);
    }
#अन्यथा
    sqlite3_snprintf(MIN(n_पूर्णम्, pVfs->mxPathname), z_पूर्णम्, "%s", z_बहिर्गमनम्);
#यदि_अन्तः /* __CYGWIN__ */
    sqlite3_free(z_बहिर्गमनम्);
    प्रत्यागम SQLITE_OK;
  }अन्यथा{
    प्रत्यागम SQLITE_IOERR_NOMEM_BKPT;
  }
}
स्थिर पूर्णाङ्क win_पूर्णम्_Pathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ध्रुव वर्ण *zRelative,        /* Possibly relative input path */
  पूर्णाङ्क n_पूर्णम्,                    /* Size of output buffer in bytes */
  वर्ण *z_पूर्णम्                   /* Output buffer */
){
  पूर्णाङ्क rc;
  MUTEX_LOGIC( sqlite3_mutex *pMutex; )
  MUTEX_LOGIC( pMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR); )
  sqlite3_mutex_enter(pMutex);
  rc = win_पूर्णम्_Pathname_No_Mutex(pVfs, zRelative, n_पूर्णम्, z_पूर्णम्);
  sqlite3_mutex_leave(pMutex);
  प्रत्यागम rc;
}

#यदि_अपरिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
स्थिर शून्य *winDlOpen(sqlite3_vfs *pVfs, ध्रुव वर्ण *zFilename){
  HANDLE h = 0;
  LPWSTR zConverted = winConvertFromUtf8Filename(zFilename);
  अप्रयुक्तम्_PARAMETER(pVfs);
  यदि( zConverted==0 ){
    OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (शून्य*)0));
    प्रत्यागम 0;
  }
  h = os_आरोपय_Library_W ? os_आरोपय_Library_W(zConverted) : 0;
  OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (शून्य*)h));
  sqlite3_free(zConverted);
  प्रत्यागम (शून्य*)h;
}
स्थिर शून्य winDlError(sqlite3_vfs *pVfs, पूर्णाङ्क nBuf, वर्ण *z_Buf_बहिर्गमनम्){
  अप्रयुक्तम्_PARAMETER(pVfs);
  win_Get_चरमः_Error_सन्देशः(os_Get_चरमः_Error(), nBuf, z_Buf_बहिर्गमनम्);
}
स्थिर शून्य (*winDlSym(sqlite3_vfs *pVfs,शून्य *pH,ध्रुव वर्ण *zSym))(शून्य){
  FARPROC proc = 0;
  अप्रयुक्तम्_PARAMETER(pVfs);
  proc = os_Get_Proc_स्थानाङ्कः_A ? os_Get_Proc_स्थानाङ्कः_A((HANDLE)pH, zSym) : 0;
  OSTRACE(("DLSYM handle=%p, symbol=%s, address=%p\n",
           (शून्य*)pH, zSym, (शून्य*)proc));
  प्रत्यागम (शून्य(*)(शून्य))proc;
}
स्थिर शून्य winDlClose(sqlite3_vfs *pVfs, शून्य *pHandle){
  अप्रयुक्तम्_PARAMETER(pVfs);
  यदि( osFreeLibrary!=0 ) osFreeLibrary((HANDLE)pHandle);
  OSTRACE(("DLCLOSE handle=%p\n", (शून्य*)pHandle));
}
#अन्यथा /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #परिभाषय winDlOpen  0
  #परिभाषय winDlError 0
  #परिभाषय winDlSym   0
  #परिभाषय winDlClose 0
#यदि_अन्तः

/* State information for the randomness gatherer. */
प्रकारनिर्देश संरचना EntropyGatherer EntropyGatherer;
संरचना EntropyGatherer {
  अचिह्नित वर्ण *a;   /* Gather entropy into this buffer */
  पूर्णाङ्क na;             /* Size of a[] in bytes */
  पूर्णाङ्क i;              /* XOR next input into a[i] */
  पूर्णाङ्क nXor;           /* Number of XOR operations done */
};

#यदि !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
/* Mix sz bytes of entropy into p. */
स्थिर शून्य xor_स्मृतिः(EntropyGatherer *p, अचिह्नित वर्ण *x, पूर्णाङ्क sz){
  पूर्णाङ्क j, k;
  प्रति(j=0, k=p->i; j<sz; j++){
    p->a[k++] ^= x[j];
    यदि( k>=p->na ) k = 0;
  }
  p->i = k;
  p->nXor += sz;
}
#यदि_अन्तः /* !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS) */

/*
** Write up to nBuf bytes of randomness into zBuf.
*/
स्थिर पूर्णाङ्क winRandomness(sqlite3_vfs *pVfs, पूर्णाङ्क nBuf, वर्ण *zBuf){
#यदि defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS)
  अप्रयुक्तम्_PARAMETER(pVfs);
  स्मृतिं_पूरय(zBuf, 0, nBuf);
  प्रत्यागम nBuf;
#अन्यथा
  EntropyGatherer e;
  अप्रयुक्तम्_PARAMETER(pVfs);
  स्मृतिं_पूरय(zBuf, 0, nBuf);
  e.a = (अचिह्नित वर्ण*)zBuf;
  e.na = nBuf;
  e.nXor = 0;
  e.i = 0;
  {
    FILETIME x;
    osGetSystemTimeAsFileTime(&x);
    xor_स्मृतिः(&e, (अचिह्नित वर्ण*)&x, परिमाण(x));
  }
  {
    DWORD pid = osGetCurrentProcessId();
    xor_स्मृतिः(&e, (अचिह्नित वर्ण*)&pid, परिमाण(pid));
  }
  {
    ULONGLONG cnt = osGetTickCount64();
    xor_स्मृतिः(&e, (अचिह्नित वर्ण*)&cnt, परिमाण(cnt));
  }
  {
    LARGE_INTEGER i;
    os_Query_Performance_गणक(&i);
    xor_स्मृतिः(&e, (अचिह्नित वर्ण*)&i, परिमाण(i));
  }
#यदि SQLITE_WIN32_USE_UUID
#यदि_परिभाषितम् SQLITE_UWP
# error SQLITE_WIN32_USE_UUID is incompatible with SQLITE_UWP
#यदि_अन्तः
  {
    UUID id;
    स्मृतिं_पूरय(&id, 0, परिमाण(id));
    osUuidCreate(&id);
    xor_स्मृतिः(&e, (अचिह्नित वर्ण*)&id, परिमाण(id));
    स्मृतिं_पूरय(&id, 0, परिमाण(UUID));
    osUuidCreateSequential(&id);
    xor_स्मृतिः(&e, (अचिह्नित वर्ण*)&id, परिमाण(id));
  }
#यदि_अन्तः /* SQLITE_WIN32_USE_UUID */
  प्रत्यागम e.nXor>nBuf ? nBuf : e.nXor;
#यदि_अन्तः /* defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS) */
}


/*
** Sleep for a little while.  Return the amount of time slept.
*/
स्थिर पूर्णाङ्क winSleep(sqlite3_vfs *pVfs, पूर्णाङ्क microsec){
  osSleep((microsec+999)/1000);
  अप्रयुक्तम्_PARAMETER(pVfs);
  प्रत्यागम ((microsec+999)/1000)*1000;
}

/*
** The following variable, if set to a non-zero value, is interpreted as
** the number of seconds since 1970 and is used to set the result of
** sqlite3OsCurrentTime() during testing.
*/
#यदि_परिभाषितम् SQLITE_TEST
पूर्णाङ्क sqlite3_current_time = 0;  /* Fake system time in seconds since 1970. */
#यदि_अन्तः

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
स्थिर पूर्णाङ्क winCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *piNow){
  /* FILETIME structure is a 64-bit value representing the number of
     100-nanosecond intervals since January 1, 1601 (= JD 2305813.5).
  */
  FILETIME ft;
  स्थिर ध्रुव sqlite3_int64 winFiletimeEpoch = 23058135*(sqlite3_int64)8640000;
#यदि_परिभाषितम् SQLITE_TEST
  स्थिर ध्रुव sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
#यदि_अन्तः
  /* 2^32 - to avoid use of LL and warnings in gcc */
  स्थिर ध्रुव sqlite3_int64 max32_Bit_मानम् =
      (sqlite3_int64)2000000000 + (sqlite3_int64)2000000000 +
      (sqlite3_int64)294967296;

  osGetSystemTimeAsFileTime( &ft );
  *piNow = winFiletimeEpoch +
            ((((sqlite3_int64)ft.dw_उच्चः_Date_Time)*max32_Bit_मानम्) +
               (sqlite3_int64)ft.dw_नीचः_Date_Time)/(sqlite3_int64)10000;

#यदि_परिभाषितम् SQLITE_TEST
  यदि( sqlite3_current_time ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_time + unixEpoch;
  }
#यदि_अन्तः
  अप्रयुक्तम्_PARAMETER(pVfs);
  प्रत्यागम SQLITE_OK;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
स्थिर पूर्णाङ्क winCurrentTime(sqlite3_vfs *pVfs, द्विगुण *prNow){
  पूर्णाङ्क rc;
  sqlite3_int64 i;
  rc = winCurrentTimeInt64(pVfs, &i);
  यदि( !rc ){
    *prNow = i/86400000.0;
  }
  प्रत्यागम rc;
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
स्थिर पूर्णाङ्क win_Get_चरमः_Error(sqlite3_vfs *pVfs, पूर्णाङ्क nBuf, वर्ण *zBuf){
  DWORD e = os_Get_चरमः_Error();
  अप्रयुक्तम्_PARAMETER(pVfs);
  यदि( nBuf>0 ) win_Get_चरमः_Error_सन्देशः(e, nBuf, zBuf);
  प्रत्यागम e;
}

/*
** Initialize and deinitialize the operating system interface.
*/
पूर्णाङ्क sqlite3_os_init(शून्य){
  स्थिर sqlite3_vfs winVfs = {
    3,                     /* iVersion */
    परिमाण(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_मार्गः_अष्टकानि, /* mxPathname */
    0,                     /* pNext */
    "win32",               /* zName */
    &win_App_दत्त,           /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_पूर्णम्_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    winCurrentTime,        /* xCurrentTime */
    win_Get_चरमः_Error,       /* xGetLastError */
    winCurrentTimeInt64,   /* xCurrentTimeInt64 */
    win_समुच्चयः_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_अग्र_System_Call,     /* xNextSystemCall */
  };
  स्थिर sqlite3_vfs win_Long_मार्गः_Vfs = {
    3,                     /* iVersion */
    परिमाण(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_मार्गः_अष्टकानि, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath",      /* zName */
    &win_App_दत्त,           /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_पूर्णम्_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    winCurrentTime,        /* xCurrentTime */
    win_Get_चरमः_Error,       /* xGetLastError */
    winCurrentTimeInt64,   /* xCurrentTimeInt64 */
    win_समुच्चयः_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_अग्र_System_Call,     /* xNextSystemCall */
  };
  स्थिर sqlite3_vfs winNolockVfs = {
    3,                     /* iVersion */
    परिमाण(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_मार्गः_अष्टकानि, /* mxPathname */
    0,                     /* pNext */
    "win32-none",          /* zName */
    &win_Nolock_App_दत्त,     /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_पूर्णम्_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    winCurrentTime,        /* xCurrentTime */
    win_Get_चरमः_Error,       /* xGetLastError */
    winCurrentTimeInt64,   /* xCurrentTimeInt64 */
    win_समुच्चयः_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_अग्र_System_Call,     /* xNextSystemCall */
  };
  स्थिर sqlite3_vfs win_Long_मार्गः_Nolock_Vfs = {
    3,                     /* iVersion */
    परिमाण(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_मार्गः_अष्टकानि, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath-none", /* zName */
    &win_Nolock_App_दत्त,     /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_पूर्णम्_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    winCurrentTime,        /* xCurrentTime */
    win_Get_चरमः_Error,       /* xGetLastError */
    winCurrentTimeInt64,   /* xCurrentTimeInt64 */
    win_समुच्चयः_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_अग्र_System_Call,     /* xNextSystemCall */
  };

  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  प्रतिज्ञा( Array_आकार(aSyscall)==60 );
  प्रतिज्ञा( वाक्यं_तुलय(aSyscall[0].zName,"AreFileApisANSI")==0 );
  प्रतिज्ञा( वाक्यं_तुलय(aSyscall[8].zName,"FreeLibrary")==0 );
  प्रतिज्ञा( वाक्यं_तुलय(aSyscall[16].zName,"GetSystemInfo")==0 );
  प्रतिज्ञा( वाक्यं_तुलय(aSyscall[24].zName,"HeapReAlloc")==0 );
  प्रतिज्ञा( वाक्यं_तुलय(aSyscall[32].zName,"MapViewOfFileFromApp")==0 );
  प्रतिज्ञा( वाक्यं_तुलय(aSyscall[40].zName,"UnmapViewOfFile")==0 );
  प्रतिज्ञा( वाक्यं_तुलय(aSyscall[48].zName,"UuidCreate")==0 );
  प्रतिज्ञा( वाक्यं_तुलय(aSyscall[56].zName,"lstat")==0 );

  /* get memory map allocation granularity */
  स्मृतिं_पूरय(&winSysInfo, 0, परिमाण(SYSTEM_INFO));
  osGetSystemInfo(&winSysInfo);
  प्रतिज्ञा( winSysInfo.dw_आवण्टनम्_Granularity>0 );
  प्रतिज्ञा( winSysInfo.dw_Page_आकार>0 );

  sqlite3_vfs_पञ्जिकाकोशः(&winVfs, 1);
  sqlite3_vfs_पञ्जिकाकोशः(&win_Long_मार्गः_Vfs, 0);
  sqlite3_vfs_पञ्जिकाकोशः(&winNolockVfs, 0);
  sqlite3_vfs_पञ्जिकाकोशः(&win_Long_मार्गः_Nolock_Vfs, 0);
#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
  win_Big_तालकम् = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#यदि_अन्तः

  प्रत्यागम SQLITE_OK;
}

पूर्णाङ्क sqlite3_os_अन्तः(शून्य){
#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
  win_Big_तालकम् = 0;
#यदि_अन्तः
  प्रत्यागम SQLITE_OK;
}

#यदि_अन्तः /* SQLITE_OS_WIN */
