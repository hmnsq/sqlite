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
#جيڪڏهن_وصف_نه_ٿيل MAX_رستو
#  define MAX_رستو                      (260)
#جيڪڏهن_پورو

/*
** Maximum pathname length (in chars) for Win32.  This should normally be
** MAX_PATH.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WIN32_MAX_رستو_CHARS
#  define SQLITE_WIN32_MAX_رستو_CHARS   (MAX_رستو)
#جيڪڏهن_پورو

/*
** This constant should already be defined (in the "WinNT.h" SDK file).
*/
#جيڪڏهن_وصف_نه_ٿيل UNICODE_STRING_MAX_CHARS
#  define UNICODE_STRING_MAX_CHARS      (32767)
#جيڪڏهن_پورو

/*
** Maximum pathname length (in chars) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WINNT_MAX_رستو_CHARS
#  define SQLITE_WINNT_MAX_رستو_CHARS   (UNICODE_STRING_MAX_CHARS)
#جيڪڏهن_پورو

/*
** Maximum pathname length (in bytes) for Win32.  The MAX_PATH macro is in
** characters, so we allocate 4 bytes per character assuming worst-case of
** 4-bytes-per-character for UTF8.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WIN32_MAX_رستو_بائيٽون
#  define SQLITE_WIN32_MAX_رستو_بائيٽون   (SQLITE_WIN32_MAX_رستو_CHARS*4)
#جيڪڏهن_پورو

/*
** Maximum pathname length (in bytes) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS * sizeof(WCHAR).
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WINNT_MAX_رستو_بائيٽون
#  define SQLITE_WINNT_MAX_رستو_بائيٽون   \
                            (حجم(WCHAR) * SQLITE_WINNT_MAX_رستو_CHARS)
#جيڪڏهن_پورو

/*
** Maximum error message length (in chars) for WinRT.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WIN32_MAX_ERRMSG_CHARS
#  define SQLITE_WIN32_MAX_ERRMSG_CHARS (1024)
#جيڪڏهن_پورو

/*
** Returns non-zero if the character should be treated as a directory
** separator.
*/
#جيڪڏهن_وصف_نه_ٿيل winIsDirSep
#  define winIsDirSep(a)                (((a) == '/') || ((a) == '\\'))
#جيڪڏهن_پورو

/*
** This macro is used when a local variable is set to a value that is
** [sometimes] not used by the code (e.g. via conditional compilation).
*/
#جيڪڏهن_وصف_نه_ٿيل اڻ_استعمال_ٿيل_VARIABLE_قدر
#  define اڻ_استعمال_ٿيل_VARIABLE_قدر(x)      (فراغ)(x)
#جيڪڏهن_پورو

/*
** Returns the character that should be used as the directory separator.
*/
#جيڪڏهن_وصف_نه_ٿيل winGetDirSep
#  define winGetDirSep()                '\\'
#جيڪڏهن_پورو

/*
** Some older Microsoft compilers lack the following definition.
*/
#جيڪڏهن_وصف_نه_ٿيل INVALID_FILE_خاصيتون
# define INVALID_FILE_خاصيتون ((DWORD)-1)
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل FILE_FLAG_نقاب
# define FILE_FLAG_نقاب          (0xFF3C0000)
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل FILE_ATTRIBUTE_نقاب
# define FILE_ATTRIBUTE_نقاب     (0x0003FFF7)
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WAL
/* Forward references to structures used for WAL */
تعريفنوع بنية winShm winShm;           /* A connection to shared-memory */
تعريفنوع بنية win_Shm_ڳنڍ win_Shm_ڳنڍ;   /* A region of shared-memory */
#جيڪڏهن_پورو

/*
** The winFile structure is a subclass of sqlite3_file* specific to the win32
** portability layer.
*/
تعريفنوع بنية winFile winFile;
بنية winFile {
  ثابت sqlite3_io_methods *p_طريقو; /*** Must be first ***/
  sqlite3_vfs *pVfs;      /* The VFS used to open this file */
  HANDLE h;               /* Handle for accessing the file */
  u8 locktype;            /* Type of lock currently held on this file */
  ننڍو shared_تالو_Byte;   /* Randomly chosen byte used as a shared lock */
  u8 ctrlFlags;           /* Flags.  See WINFILE_* below */
  DWORD آخري_Errno;        /* The Windows errno from the last I/O error */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WAL
  winShm *pShm;           /* Instance of shared memory on this file */
#جيڪڏهن_پورو
  ثابت حرف *z_رستو;      /* Full pathname of this file */
  صحيح szChunk;            /* Chunk size configured by FCNTL_CHUNK_SIZE */
#إذا SQLITE_MAX_MMAP_سائيز>0
  صحيح n_Fetch_ٻاهريون;                /* Number of outstanding xFetch references */
  HANDLE hMap;                  /* Handle for accessing memory mapping */
  فراغ *pMapRegion;             /* Area memory mapped */
  sqlite3_int64 mmap_سائيز;       /* Size of mapped region */
  sqlite3_int64 mmap_سائيز_Max;    /* Configured FCNTL_MMAP_SIZE value */
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SETLK_TIMEOUT
  DWORD iBusyTimeout;        /* Wait this many millisec on locks */
  صحيح b_بلاڪ_On_Connect;
#جيڪڏهن_پورو
};

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SETLK_TIMEOUT
# define winFileBusyTimeout(pDbFd) pDbFd->iBusyTimeout
#وإلا
# define winFileBusyTimeout(pDbFd) 0
#جيڪڏهن_پورو

/*
** The winVfsAppData structure is used for the pAppData member for all of the
** Win32 VFS variants.
*/
تعريفنوع بنية win_Vfs_App_معلومات win_Vfs_App_معلومات;
بنية win_Vfs_App_معلومات {
  ثابت sqlite3_io_methods *p_طريقو; /* The file I/O methods to use. */
  فراغ *p_App_معلومات;                    /* The extra pAppData, if any. */
  BOOL b_No_تالو;                      /* Non-zero if locking is disabled. */
};

/*
** Allowed values for winFile.ctrlFlags
*/
#وصف_ڪريو WINFILE_RDONLY          0x02   /* Connection is read only */
#وصف_ڪريو WINFILE_PERSIST_WAL     0x04   /* Persistent WAL mode */
#وصف_ڪريو WINFILE_PSOW            0x10   /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */

/*
 * The size of the buffer used by sqlite3_win32_write_debug().
 */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WIN32_DBG_BUF_سائيز
#  define SQLITE_WIN32_DBG_BUF_سائيز   ((صحيح)(4096-حجم(DWORD)))
#جيڪڏهن_پورو

/*
 * If compiled with SQLITE_WIN32_MALLOC on Windows, we will use the
 * various Win32 API heap functions instead of our own.
 */
#جيڪڏهن_وصف_ٿيل SQLITE_WIN32_MALLOC

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
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WIN32_ڍير_CREATE
#  define SQLITE_WIN32_ڍير_CREATE        (TRUE)
#جيڪڏهن_پورو

/*
 * This is the maximum possible initial size of the Win32-specific heap, in
 * bytes.
 */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WIN32_ڍير_MAX_INIT_سائيز
#  define SQLITE_WIN32_ڍير_MAX_INIT_سائيز (4294967295U)
#جيڪڏهن_پورو

/*
 * This is the extra space for the initial size of the Win32-specific heap,
 * in bytes.  This value may be zero.
 */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WIN32_ڍير_INIT_EXTRA
#  define SQLITE_WIN32_ڍير_INIT_EXTRA  (4194304)
#جيڪڏهن_پورو

/*
 * Calculate the maximum legal cache size, in pages, based on the maximum
 * possible initial heap size and the default page size, setting aside the
 * needed extra space.
 */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WIN32_MAX_CACHE_سائيز
#  define SQLITE_WIN32_MAX_CACHE_سائيز   (((SQLITE_WIN32_ڍير_MAX_INIT_سائيز) - \
                                          (SQLITE_WIN32_ڍير_INIT_EXTRA)) / \
                                         (SQLITE_DEFAULT_صفحو_سائيز))
#جيڪڏهن_پورو

/*
 * This is cache size used in the calculation of the initial size of the
 * Win32-specific heap.  It cannot be negative.
 */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WIN32_CACHE_سائيز
#  إذا SQLITE_DEFAULT_CACHE_سائيز>=0
#    define SQLITE_WIN32_CACHE_سائيز     (SQLITE_DEFAULT_CACHE_سائيز)
#  وإلا
#    define SQLITE_WIN32_CACHE_سائيز     (-(SQLITE_DEFAULT_CACHE_سائيز))
#  endif
#جيڪڏهن_پورو

/*
 * Make sure that the calculated cache size, in pages, cannot cause the
 * initial size of the Win32-specific heap to exceed the maximum amount
 * of memory that can be specified in the call to HeapCreate.
 */
#إذا SQLITE_WIN32_CACHE_سائيز>SQLITE_WIN32_MAX_CACHE_سائيز
#  undef SQLITE_WIN32_CACHE_سائيز
#  define SQLITE_WIN32_CACHE_سائيز       (2000)
#جيڪڏهن_پورو

/*
 * The initial size of the Win32-specific heap.  This value may be zero.
 */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WIN32_ڍير_INIT_سائيز
#  define SQLITE_WIN32_ڍير_INIT_سائيز   ((SQLITE_WIN32_CACHE_سائيز) * \
                                         (SQLITE_DEFAULT_صفحو_سائيز) + \
                                         (SQLITE_WIN32_ڍير_INIT_EXTRA))
#جيڪڏهن_پورو

/*
 * The maximum size of the Win32-specific heap.  This value may be zero.
 */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WIN32_ڍير_MAX_سائيز
#  define SQLITE_WIN32_ڍير_MAX_سائيز    (0)
#جيڪڏهن_پورو

/*
 * The extra flags to use in calls to the Win32 heap APIs.  This value may be
 * zero for the default behavior.
 */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WIN32_ڍير_FLAGS
#  define SQLITE_WIN32_ڍير_FLAGS       (0)
#جيڪڏهن_پورو


/*
** The winMemData structure stores information required by the Win32-specific
** sqlite3_mem_methods implementation.
*/
تعريفنوع بنية win_Mem_معلومات win_Mem_معلومات;
بنية win_Mem_معلومات {
#جيڪڏهن_وصف_نه_ٿيل NDEBUG
  u32 magic1;   /* Magic number to detect structure corruption. */
#جيڪڏهن_پورو
  HANDLE h_ڍير; /* The handle to our heap. */
  BOOL bOwned;  /* Do we own the heap (i.e. destroy it on shutdown)? */
#جيڪڏهن_وصف_نه_ٿيل NDEBUG
  u32 magic2;   /* Magic number to detect structure corruption. */
#جيڪڏهن_پورو
};

#جيڪڏهن_وصف_نه_ٿيل NDEBUG
#وصف_ڪريو WINMEM_MAGIC1     0x42b2830b
#وصف_ڪريو WINMEM_MAGIC2     0xbd4d7cf4
#جيڪڏهن_پورو

ساكن بنية win_Mem_معلومات win_mem_معلومات = {
#جيڪڏهن_وصف_نه_ٿيل NDEBUG
  WINMEM_MAGIC1,
#جيڪڏهن_پورو
  عدم, FALSE
#جيڪڏهن_وصف_نه_ٿيل NDEBUG
  ,WINMEM_MAGIC2
#جيڪڏهن_پورو
};

#جيڪڏهن_وصف_نه_ٿيل NDEBUG
#وصف_ڪريو winMemAssertMagic1() أكد( win_mem_معلومات.magic1==WINMEM_MAGIC1 )
#وصف_ڪريو winMemAssertMagic2() أكد( win_mem_معلومات.magic2==WINMEM_MAGIC2 )
#وصف_ڪريو winMemAssertMagic()  winMemAssertMagic1(); winMemAssertMagic2();
#وإلا
#وصف_ڪريو winMemAssertMagic()
#جيڪڏهن_پورو

#وصف_ڪريو win_Mem_Get_معلومات_Ptr()  &win_mem_معلومات
#وصف_ڪريو win_Mem_Get_ڍير()     win_mem_معلومات.h_ڍير
#وصف_ڪريو winMemGetOwned()    win_mem_معلومات.bOwned

ساكن فراغ *winMemMalloc(صحيح n_بائيٽون);
ساكن فراغ winMemFree(فراغ *p_اڳواٽ);
ساكن فراغ *winMemRealloc(فراغ *p_اڳواٽ, صحيح n_بائيٽون);
ساكن صحيح win_Mem_سائيز(فراغ *p);
ساكن صحيح winMemRoundup(صحيح n);
ساكن صحيح winMemInit(فراغ *p_App_معلومات);
ساكن فراغ winMemShutdown(فراغ *p_App_معلومات);

ثابت sqlite3_mem_methods *sqlite3MemGetWin32(فراغ);
#جيڪڏهن_پورو /* SQLITE_WIN32_MALLOC */

#جيڪڏهن_وصف_نه_ٿيل SYSCALL
#  define SYSCALL sqlite3_syscall_ptr
#جيڪڏهن_پورو

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
#وصف_ڪريو osAreFileApisANSI ((BOOL(WINAPI*)(VOID))aSyscall[0].pCurrent)

  { "CloseHandle",             (SYSCALL)CloseHandle,             0 },
#وصف_ڪريو osCloseHandle ((BOOL(WINAPI*)(HANDLE))aSyscall[1].pCurrent)


  { "CreateFileW",             (SYSCALL)CreateFileW,             0 },
#وصف_ڪريو osCreateFileW ((HANDLE(WINAPI*)(LPCWSTR,DWORD,DWORD, \
        LPSECURITY_خاصيتون,DWORD,DWORD,HANDLE))aSyscall[2].pCurrent)

#إذا defined(SQLITE_UWP)
  { "CreateFileMappingFromApp",(SYSCALL)CreateFileMappingFromApp,0 },
#وإلا
  { "CreateFileMappingFromApp",(SYSCALL)0,                       0 },
#جيڪڏهن_پورو
#وصف_ڪريو osCreateFileMappingFromApp ((HANDLE(WINAPI*)(HANDLE,\
        PSECURITY_خاصيتون, \
        ULONG,ULONG64,PCWSTR))aSyscall[3].pCurrent)

#إذا !defined(SQLITE_UWP)
  { "CreateFileMappingW",      (SYSCALL)CreateFileMappingW,      0 },
#وإلا
  { "CreateFileMappingW",      (SYSCALL)0,                       0 },
#جيڪڏهن_پورو
#وصف_ڪريو osCreateFileMappingW ((HANDLE(WINAPI*)(HANDLE,LPSECURITY_خاصيتون, \
        DWORD,DWORD,DWORD,LPCWSTR))aSyscall[4].pCurrent)

  { "DeleteFileW",             (SYSCALL)DeleteFileW,             0 },
#وصف_ڪريو osDeleteFileW ((BOOL(WINAPI*)(LPCWSTR))aSyscall[5].pCurrent)

  { "FlushFileBuffers",        (SYSCALL)FlushFileBuffers,        0 },
#وصف_ڪريو osFlushFileBuffers ((BOOL(WINAPI*)(HANDLE))aSyscall[6].pCurrent)

  { "FormatMessageW",          (SYSCALL)Format_پيغام_W,          0 },
#وصف_ڪريو os_Format_پيغام_W ((DWORD(WINAPI*)(DWORD,LPCVOID,DWORD,DWORD,LPWSTR, \
        DWORD,va_list*))aSyscall[7].pCurrent)

#إذا !defined(SQLITE_OMIT_ڀريو_EXTENSION) && !defined(SQLITE_UWP)
  { "FreeLibrary",             (SYSCALL)FreeLibrary,             0 },
#وإلا
  { "FreeLibrary",             (SYSCALL)0,                       0 },
#جيڪڏهن_پورو

#وصف_ڪريو osFreeLibrary ((BOOL(WINAPI*)(HMODULE))aSyscall[8].pCurrent)

  { "GetCurrentProcessId",     (SYSCALL)GetCurrentProcessId,     0 },
#وصف_ڪريو osGetCurrentProcessId ((DWORD(WINAPI*)(VOID))aSyscall[9].pCurrent)

  { "GetFileAttributesW",      (SYSCALL)Get_File_خاصيتون_W,      0 },
#وصف_ڪريو os_Get_File_خاصيتون_W ((DWORD(WINAPI*)(LPCWSTR))aSyscall[10].pCurrent)

  { "GetFileAttributesExW",    (SYSCALL)Get_File_خاصيتون_Ex_W,    0 },
#وصف_ڪريو os_Get_File_خاصيتون_Ex_W ((BOOL(WINAPI*)(LPCWSTR,GET_FILEEX_INFO_سطحون, \
        LPVOID))aSyscall[11].pCurrent)

  { "GetFileSizeEx",           (SYSCALL)Get_File_سائيز_Ex,           0 },
#وصف_ڪريو os_Get_File_سائيز_Ex ((BOOL(WINAPI*)(HANDLE, \
                        PLARGE_INTEGER))aSyscall[12].pCurrent)

  { "GetFullPathNameW",        (SYSCALL)Get_ڀريل_رستو_Name_W,        0 },
#وصف_ڪريو os_Get_ڀريل_رستو_Name_W ((DWORD(WINAPI*)(LPCWSTR,DWORD,LPWSTR, \
        LPWSTR*))aSyscall[13].pCurrent)

  { "GetLastError",            (SYSCALL)Get_آخري_Error,            0 },
#وصف_ڪريو os_Get_آخري_Error ((DWORD(WINAPI*)(VOID))aSyscall[14].pCurrent)

#إذا !defined(SQLITE_OMIT_ڀريو_EXTENSION) && !defined(SQLITE_UWP)
  { "GetProcAddressA",         (SYSCALL)Get_Proc_پتو,          0 },
#وإلا
  { "GetProcAddressA",         (SYSCALL)0,                       0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_Get_Proc_پتو_A ((FARPROC(WINAPI*)(HMODULE, \
        LPCSTR))aSyscall[15].pCurrent)

  { "GetSystemInfo",           (SYSCALL)GetSystemInfo,           0 },
#وصف_ڪريو osGetSystemInfo ((VOID(WINAPI*)(LPSYSTEM_INFO))aSyscall[16].pCurrent)

  { "GetSystemTimeAsFileTime", (SYSCALL)Get_System_وقت_As_File_وقت, 0 },
#وصف_ڪريو os_Get_System_وقت_As_File_وقت ((VOID(WINAPI*)( \
        LPFILETIME))aSyscall[17].pCurrent)

#جيڪڏهن_وصف_ٿيل SQLITE_UWP
  { "GetTempPathW",            (SYSCALL)0,                       0 },
#وإلا
  { "GetTempPathW",            (SYSCALL)Get_عارضي_رستو_W,            0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_Get_عارضي_رستو_W ((DWORD(WINAPI*)(DWORD,LPWSTR))aSyscall[18].pCurrent)

  { "GetTickCount64",          (SYSCALL)GetTickCount64,          0 },
#وصف_ڪريو osGetTickCount64 ((ULONGLONG(WINAPI*)(VOID))aSyscall[19].pCurrent)

#جيڪڏهن_وصف_ٿيل SQLITE_UWP
  { "HeapAlloc",               (SYSCALL)0,                       0 },
#وإلا
  { "HeapAlloc",               (SYSCALL)ڍير_Alloc,               0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_ڍير_Alloc ((LPVOID(WINAPI*)(HANDLE,DWORD, \
        سائيز_T))aSyscall[20].pCurrent)

#جيڪڏهن_وصف_ٿيل SQLITE_UWP
  { "HeapCreate",              (SYSCALL)0,                       0 },
#وإلا
  { "HeapCreate",              (SYSCALL)ڍير_Create,              0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_ڍير_Create ((HANDLE(WINAPI*)(DWORD,سائيز_T, \
        سائيز_T))aSyscall[21].pCurrent)

#جيڪڏهن_وصف_ٿيل SQLITE_UWP
  { "HeapDestroy",             (SYSCALL)0,                       0 },
#وإلا
  { "HeapDestroy",             (SYSCALL)ڍير_Destroy,             0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_ڍير_Destroy ((BOOL(WINAPI*)(HANDLE))aSyscall[22].pCurrent)

#جيڪڏهن_وصف_ٿيل SQLITE_UWP
  { "HeapFree",                (SYSCALL)0,                       0 },
#وإلا
  { "HeapFree",                (SYSCALL)ڍير_Free,                0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_ڍير_Free ((BOOL(WINAPI*)(HANDLE,DWORD,LPVOID))aSyscall[23].pCurrent)

#جيڪڏهن_وصف_ٿيل SQLITE_UWP
  { "HeapReAlloc",             (SYSCALL)0,                       0 },
#وإلا
  { "HeapReAlloc",             (SYSCALL)ڍير_Re_Alloc,             0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_ڍير_Re_Alloc ((LPVOID(WINAPI*)(HANDLE,DWORD,LPVOID, \
        سائيز_T))aSyscall[24].pCurrent)

#جيڪڏهن_وصف_ٿيل SQLITE_UWP
  { "HeapSize",                (SYSCALL)0,                       0 },
#وإلا
  { "HeapSize",                (SYSCALL)ڍير_سائيز,                0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_ڍير_سائيز ((سائيز_T(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[25].pCurrent)

#جيڪڏهن_وصف_ٿيل SQLITE_UWP
  { "HeapValidate",            (SYSCALL)0,                       0 },
#وإلا
  { "HeapValidate",            (SYSCALL)ڍير_Validate,            0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_ڍير_Validate ((BOOL(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[26].pCurrent)

#جيڪڏهن_وصف_ٿيل SQLITE_UWP
  { "HeapCompact",             (SYSCALL)0,                       0 },
#وإلا
  { "HeapCompact",             (SYSCALL)ڍير_Compact,             0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_ڍير_Compact ((UINT(WINAPI*)(HANDLE,DWORD))aSyscall[27].pCurrent)


#إذا !defined(SQLITE_OMIT_ڀريو_EXTENSION) && !defined(SQLITE_UWP)
  { "LoadLibraryW",            (SYSCALL)ڀريو_Library_W,            0 },
#وإلا
  { "LoadLibraryW",            (SYSCALL)0,                       0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_ڀريو_Library_W ((HMODULE(WINAPI*)(LPCWSTR))aSyscall[28].pCurrent)

  { "LocalFree",               (SYSCALL)LocalFree,               0 },
#وصف_ڪريو osLocalFree ((HLOCAL(WINAPI*)(HLOCAL))aSyscall[29].pCurrent)

  { "LockFileEx",              (SYSCALL)تالو_File_Ex,              0 },
#وصف_ڪريو os_تالو_File_Ex ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[30].pCurrent)

#إذا (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_سائيز>0) \
 && !defined(SQLITE_UWP)
  { "MapViewOfFile",           (SYSCALL)Map_View_جو_File,           0 },
#وإلا
  { "MapViewOfFile",           (SYSCALL)0,                       0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_Map_View_جو_File ((LPVOID(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        سائيز_T))aSyscall[31].pCurrent)

#إذا (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_سائيز>0) \
 && defined(SQLITE_UWP)
  { "MapViewOfFileFromApp",    (SYSCALL)Map_View_جو_File_From_App,   0 },
#وإلا
  { "MapViewOfFileFromApp",    (SYSCALL)0,                      0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_Map_View_جو_File_From_App ((LPVOID(WINAPI*)(HANDLE,ULONG,ULONG64, \
        سائيز_T))aSyscall[32].pCurrent)

  { "MultiByteToWideChar",     (SYSCALL)Multi_Byte_منزل_Wide_Char,     0 },
#وصف_ڪريو os_Multi_Byte_منزل_Wide_Char ((صحيح(WINAPI*)(UINT,DWORD,LPCSTR,صحيح,LPWSTR, \
        صحيح))aSyscall[33].pCurrent)

  { "QueryPerformanceCounter", (SYSCALL)QueryPerformanceCounter, 0 },
#وصف_ڪريو osQueryPerformanceCounter ((BOOL(WINAPI*)( \
        LARGE_INTEGER*))aSyscall[34].pCurrent)

  { "ReadFile",                (SYSCALL)ReadFile,                0 },
#وصف_ڪريو osReadFile ((BOOL(WINAPI*)(HANDLE,LPVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[35].pCurrent)

  { "SetEndOfFile",            (SYSCALL)سيٽ_پڇاڙي_جو_File,            0 },
#وصف_ڪريو os_سيٽ_پڇاڙي_جو_File ((BOOL(WINAPI*)(HANDLE))aSyscall[36].pCurrent)

  { "SetFilePointerEx",        (SYSCALL)سيٽ_File_اشارو_Ex,        0 },
#وصف_ڪريو os_سيٽ_File_اشارو_Ex ((BOOL(WINAPI*)(HANDLE,LARGE_INTEGER,\
        PLARGE_INTEGER,DWORD))aSyscall[37].pCurrent)

  { "Sleep",                   (SYSCALL)Sleep,                   0 },
#وصف_ڪريو osSleep ((VOID(WINAPI*)(DWORD))aSyscall[38].pCurrent)

  { "UnlockFileEx",            (SYSCALL)UnlockFileEx,            0 },
#وصف_ڪريو osUnlockFileEx ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[39].pCurrent)

#إذا !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_سائيز>0
  { "UnmapViewOfFile",         (SYSCALL)Unmap_View_جو_File,         0 },
#وإلا
  { "UnmapViewOfFile",         (SYSCALL)0,                       0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_Unmap_View_جو_File ((BOOL(WINAPI*)(LPCVOID))aSyscall[40].pCurrent)

  { "WideCharToMultiByte",     (SYSCALL)Wide_Char_منزل_Multi_Byte,     0 },
#وصف_ڪريو os_Wide_Char_منزل_Multi_Byte ((صحيح(WINAPI*)(UINT,DWORD,LPCWSTR,صحيح,LPSTR,صحيح, \
        LPCSTR,LPBOOL))aSyscall[41].pCurrent)

  { "WriteFile",               (SYSCALL)WriteFile,               0 },
#وصف_ڪريو osWriteFile ((BOOL(WINAPI*)(HANDLE,LPCVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[42].pCurrent)

  { "WaitForSingleObject",     (SYSCALL)WaitForSingleObject,     0 },
#وصف_ڪريو osWaitForSingleObject ((DWORD(WINAPI*)(HANDLE, \
        DWORD))aSyscall[43].pCurrent)

  { "WaitForSingleObjectEx",   (SYSCALL)WaitForSingleObjectEx,   0 },
#وصف_ڪريو osWaitForSingleObjectEx ((DWORD(WINAPI*)(HANDLE,DWORD, \
        BOOL))aSyscall[44].pCurrent)

  { "OutputDebugStringA",      (SYSCALL)OutputDebugStringA,      0 },
#وصف_ڪريو osOutputDebugStringA ((VOID(WINAPI*)(LPCSTR))aSyscall[45].pCurrent)

  { "GetProcessHeap",          (SYSCALL)Get_Process_ڍير,          0 },
#وصف_ڪريو os_Get_Process_ڍير ((HANDLE(WINAPI*)(VOID))aSyscall[46].pCurrent)

/*
** NOTE: On some sub-platforms, the InterlockedCompareExchange "function"
**       is really just a macro that uses a compiler intrinsic (e.g. x64).
**       So do not try to make this is into a redefinable interface.
*/
#إذا defined(InterlockedCompareExchange)
  { "InterlockedCompareExchange", (SYSCALL)0,                    0 },
#وصف_ڪريو osInterlockedCompareExchange InterlockedCompareExchange
#وإلا
  { "InterlockedCompareExchange", (SYSCALL)InterlockedCompareExchange, 0 },
#وصف_ڪريو osInterlockedCompareExchange ((LONG(WINAPI*)(LONG متطاير*,\
        LONG,LONG))aSyscall[47].pCurrent)
#جيڪڏهن_پورو /* defined(InterlockedCompareExchange) */

#إذا SQLITE_WIN32_USE_UUID
  { "UuidCreate",               (SYSCALL)UuidCreate,             0 },
#وإلا
  { "UuidCreate",               (SYSCALL)0,                      0 },
#جيڪڏهن_پورو
#وصف_ڪريو osUuidCreate ((RPC_حالت(RPC_داخلا*)(UUID*))aSyscall[48].pCurrent)

#إذا SQLITE_WIN32_USE_UUID
  { "UuidCreateSequential",     (SYSCALL)UuidCreateSequential,   0 },
#وإلا
  { "UuidCreateSequential",     (SYSCALL)0,                      0 },
#جيڪڏهن_پورو
#وصف_ڪريو osUuidCreateSequential \
        ((RPC_حالت(RPC_داخلا*)(UUID*))aSyscall[49].pCurrent)

#إذا !defined(SQLITE_NO_SYNC) && SQLITE_MAX_MMAP_سائيز>0
  { "FlushViewOfFile",          (SYSCALL)Flush_View_جو_File,        0 },
#وإلا
  { "FlushViewOfFile",          (SYSCALL)0,                      0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_Flush_View_جو_File \
        ((BOOL(WINAPI*)(LPCVOID,سائيز_T))aSyscall[50].pCurrent)

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SETLK_TIMEOUT
  { "CreateEvent",              (SYSCALL)CreateEvent,            0 },
#وإلا
  { "CreateEvent",              (SYSCALL)0,                      0 },
#جيڪڏهن_پورو
#وصف_ڪريو osCreateEvent ((HANDLE(WINAPI*)(LPSECURITY_خاصيتون,BOOL, \
                        BOOL,LPCSTR))aSyscall[51].pCurrent)

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SETLK_TIMEOUT
  { "CancelIo",                 (SYSCALL)CancelIo,               0 },
#وإلا
  { "CancelIo",                 (SYSCALL)0,                      0 },
#جيڪڏهن_پورو
#وصف_ڪريو osCancelIo ((BOOL(WINAPI*)(HANDLE))aSyscall[52].pCurrent)

#جيڪڏهن_وصف_نه_ٿيل _WIN32
  { "getenv",                   (SYSCALL)getenv,                 0 },
#وإلا
  { "getenv",                   (SYSCALL)0,                      0 },
#جيڪڏهن_پورو
#وصف_ڪريو osGetenv ((ثابت حرف *(*)(ثابت حرف *))aSyscall[53].pCurrent)

#جيڪڏهن_وصف_نه_ٿيل _WIN32
  { "getcwd",                   (SYSCALL)getcwd,                 0 },
#وإلا
  { "getcwd",                   (SYSCALL)0,                      0 },
#جيڪڏهن_پورو
#وصف_ڪريو osGetcwd ((حرف*(*)(حرف*,size_t))aSyscall[54].pCurrent)

#جيڪڏهن_وصف_نه_ٿيل _WIN32
  { "readlink",                 (SYSCALL)readlink,               0 },
#وإلا
  { "readlink",                 (SYSCALL)0,                      0 },
#جيڪڏهن_پورو
#وصف_ڪريو osReadlink ((ssize_t(*)(ثابت حرف*,حرف*,size_t))aSyscall[55].pCurrent)

#جيڪڏهن_وصف_نه_ٿيل _WIN32
  { "lstat",                    (SYSCALL)lstat,                  0 },
#وإلا
  { "lstat",                    (SYSCALL)0,                      0 },
#جيڪڏهن_پورو
#وصف_ڪريو osLstat ((صحيح(*)(ثابت حرف*,بنية stat*))aSyscall[56].pCurrent)

#جيڪڏهن_وصف_نه_ٿيل _WIN32
  { "__errno",                  (SYSCALL)__errno,                0 },
#وإلا
  { "__errno",                  (SYSCALL)0,                      0 },
#جيڪڏهن_پورو
#وصف_ڪريو osErrno (*((صحيح*(*)(فراغ))aSyscall[57].pCurrent)())

#جيڪڏهن_وصف_نه_ٿيل _WIN32
  { "cygwin_conv_path",         (SYSCALL)cygwin_conv_رستو,       0 },
#وإلا
  { "cygwin_conv_path",         (SYSCALL)0,                      0 },
#جيڪڏهن_پورو
#وصف_ڪريو os_Cygwin_conv_رستو ((size_t(*)(بنانشان صحيح, \
    ثابت فراغ *, فراغ *, size_t))aSyscall[58].pCurrent)

  { "GetDriveTypeW",            (SYSCALL)GetDriveTypeW,          0 },
#وصف_ڪريو osGetDriveTypeW ((UINT(WINAPI*)(LPCWSTR))aSyscall[59].pCurrent)

}; /* End of the overrideable system calls */

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "win32" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
ساكن صحيح win_سيٽ_System_Call(
  sqlite3_vfs *p_Not_استعمال_ٿيل,        /* The VFS pointer.  Not used */
  ثابت حرف *zName,            /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
){
  بنانشان صحيح i;
  صحيح rc = SQLITE_NOTFOUND;

  اڻ_استعمال_ٿيل_PARAMETER(p_Not_استعمال_ٿيل);
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
  sqlite3_vfs *p_Not_استعمال_ٿيل,
  ثابت حرف *zName
){
  بنانشان صحيح i;

  اڻ_استعمال_ٿيل_PARAMETER(p_Not_استعمال_ٿيل);
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
ساكن ثابت حرف *win_ايندڙ_System_Call(sqlite3_vfs *p, ثابت حرف *zName){
  صحيح i = -1;

  اڻ_استعمال_ٿيل_PARAMETER(p);
  إذا( zName ){
    لكل(i=0; i<Array_سائيز(aSyscall)-1; i++){
      إذا( strcmp(zName, aSyscall[i].zName)==0 ) اكسر;
    }
  }
  لكل(i++; i<Array_سائيز(aSyscall); i++){
    إذا( aSyscall[i].pCurrent!=0 ) أرجع aSyscall[i].zName;
  }
  أرجع 0;
}

#جيڪڏهن_وصف_ٿيل SQLITE_WIN32_MALLOC
#جيڪڏهن_وصف_ٿيل SQLITE_UWP
# error SQLITE_WIN32_MALLOC is incompatible with SQLITE_UWP
#جيڪڏهن_پورو
/*
** If a Win32 native heap has been configured, this function will attempt to
** compact it.  Upon success, SQLITE_OK will be returned.  Upon failure, one
** of SQLITE_NOMEM, SQLITE_ERROR, or SQLITE_NOTFOUND will be returned.  The
** "pnLargest" argument, if non-zero, will be used to return the size of the
** largest committed free block in the heap, in bytes.
*/
صحيح sqlite3_win32_compact_ڍير(LPUINT pn_سڀ_کان_وڏو){
  صحيح rc = SQLITE_OK;
  UINT n_سڀ_کان_وڏو = 0;
  HANDLE h_ڍير;

  winMemAssertMagic();
  h_ڍير = win_Mem_Get_ڍير();
  أكد( h_ڍير!=0 );
  أكد( h_ڍير!=INVALID_HANDLE_قدر );
#إذا defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( os_ڍير_Validate(h_ڍير, SQLITE_WIN32_ڍير_FLAGS, عدم) );
#جيڪڏهن_پورو
  إذا( (n_سڀ_کان_وڏو=os_ڍير_Compact(h_ڍير, SQLITE_WIN32_ڍير_FLAGS))==0 ){
    DWORD آخري_Errno = os_Get_آخري_Error();
    إذا( آخري_Errno==NO_ERROR ){
      sqlite3_log(SQLITE_NOMEM, "failed to HeapCompact (no space), heap=%p",
                  (فراغ*)h_ڍير);
      rc = SQLITE_NOMEM_BKPT;
    }وإلا{
      sqlite3_log(SQLITE_ERROR, "failed to HeapCompact (%lu), heap=%p",
                  os_Get_آخري_Error(), (فراغ*)h_ڍير);
      rc = SQLITE_ERROR;
    }
  }
  إذا( pn_سڀ_کان_وڏو ) *pn_سڀ_کان_وڏو = n_سڀ_کان_وڏو;
  أرجع rc;
}

/*
** If a Win32 native heap has been configured, this function will attempt to
** destroy and recreate it.  If the Win32 native heap is not isolated and/or
** the sqlite3_memory_used() function does not return zero, SQLITE_BUSY will
** be returned and no changes will be made to the Win32 native heap.
*/
صحيح sqlite3_win32_reset_ڍير(){
  صحيح rc;
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; ) /* The main static mutex */
  MUTEX_LOGIC( sqlite3_mutex *pMem; )    /* The memsys static mutex */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  MUTEX_LOGIC( pMem = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3_mutex_enter(pMem);
  winMemAssertMagic();
  إذا( win_Mem_Get_ڍير()!=عدم && winMemGetOwned() && sqlite3_يادگيري_استعمال_ٿيل()==0 ){
    /*
    ** At this point, there should be no outstanding memory allocations on
    ** the heap.  Also, since both the main and memsys locks are currently
    ** being held by us, no other function (i.e. from another thread) should
    ** be able to even access the heap.  Attempt to destroy and recreate our
    ** isolated Win32 native heap now.
    */
    أكد( win_Mem_Get_ڍير()!=عدم );
    أكد( winMemGetOwned() );
    أكد( sqlite3_يادگيري_استعمال_ٿيل()==0 );
    winMemShutdown(win_Mem_Get_معلومات_Ptr());
    أكد( win_Mem_Get_ڍير()==عدم );
    أكد( !winMemGetOwned() );
    أكد( sqlite3_يادگيري_استعمال_ٿيل()==0 );
    rc = winMemInit(win_Mem_Get_معلومات_Ptr());
    أكد( rc!=SQLITE_OK || win_Mem_Get_ڍير()!=عدم );
    أكد( rc!=SQLITE_OK || winMemGetOwned() );
    أكد( rc!=SQLITE_OK || sqlite3_يادگيري_استعمال_ٿيل()==0 );
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
#جيڪڏهن_پورو /* SQLITE_WIN32_MALLOC */

#جيڪڏهن_وصف_ٿيل _WIN32
/*
** This function outputs the specified (ANSI) string to the Win32 debugger
** (if available).  Undocumented.  Might go away at any moment.
*/
فراغ sqlite3_win32_write_debug(ثابت حرف *zBuf, صحيح nBuf){
  حرف zDbgBuf[SQLITE_WIN32_DBG_BUF_سائيز];
  صحيح nMin = MIN(nBuf, (SQLITE_WIN32_DBG_BUF_سائيز - 1)); /* may be negative. */
  إذا( nMin<-1 ) nMin = -1; /* all negative values become -1. */
  أكد( nMin==-1 || nMin==0 || nMin<SQLITE_WIN32_DBG_BUF_سائيز );
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( !zBuf ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع;
  }
#جيڪڏهن_پورو
  إذا( nMin>0 ){
    memset(zDbgBuf, 0, SQLITE_WIN32_DBG_BUF_سائيز);
    يادگيري_نقل(zDbgBuf, zBuf, nMin);
    osOutputDebugStringA(zDbgBuf);
  }وإلا{
    osOutputDebugStringA(zBuf);
  }
}

فراغ sqlite3_win32_sleep(DWORD milliseconds){
  osSleep(milliseconds);
}

#إذا SQLITE_MAX_ڪم_ڪندڙ_ڌاڳا>0 && SQLITE_THREADSAFE>0
DWORD sqlite3Win32Wait(HANDLE hObject){
  DWORD rc;
  طالما( (rc = osWaitForSingleObjectEx(hObject, INFINITE,
                                       TRUE))==WAIT_IO_COMPLETION ){}
  أرجع rc;
}
#جيڪڏهن_پورو
#جيڪڏهن_پورو /* _WIN32 */

#جيڪڏهن_وصف_ٿيل SQLITE_WIN32_MALLOC
/*
** Allocate nBytes of memory.
*/
ساكن فراغ *winMemMalloc(صحيح n_بائيٽون){
  HANDLE h_ڍير;
  فراغ *p;

  winMemAssertMagic();
  h_ڍير = win_Mem_Get_ڍير();
  أكد( h_ڍير!=0 );
  أكد( h_ڍير!=INVALID_HANDLE_قدر );
#إذا defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( os_ڍير_Validate(h_ڍير, SQLITE_WIN32_ڍير_FLAGS, عدم) );
#جيڪڏهن_پورو
  أكد( n_بائيٽون>=0 );
  p = os_ڍير_Alloc(h_ڍير, SQLITE_WIN32_ڍير_FLAGS, (سائيز_T)n_بائيٽون);
  إذا( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapAlloc %u bytes (%lu), heap=%p",
                n_بائيٽون, os_Get_آخري_Error(), (فراغ*)h_ڍير);
  }
  أرجع p;
}

/*
** Free memory.
*/
ساكن فراغ winMemFree(فراغ *p_اڳواٽ){
  HANDLE h_ڍير;

  winMemAssertMagic();
  h_ڍير = win_Mem_Get_ڍير();
  أكد( h_ڍير!=0 );
  أكد( h_ڍير!=INVALID_HANDLE_قدر );
#إذا defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( os_ڍير_Validate(h_ڍير, SQLITE_WIN32_ڍير_FLAGS, p_اڳواٽ) );
#جيڪڏهن_پورو
  إذا( !p_اڳواٽ ) أرجع; /* Passing NULL to HeapFree is undefined. */
  إذا( !os_ڍير_Free(h_ڍير, SQLITE_WIN32_ڍير_FLAGS, p_اڳواٽ) ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapFree block %p (%lu), heap=%p",
                p_اڳواٽ, os_Get_آخري_Error(), (فراغ*)h_ڍير);
  }
}

/*
** Change the size of an existing memory allocation
*/
ساكن فراغ *winMemRealloc(فراغ *p_اڳواٽ, صحيح n_بائيٽون){
  HANDLE h_ڍير;
  فراغ *p;

  winMemAssertMagic();
  h_ڍير = win_Mem_Get_ڍير();
  أكد( h_ڍير!=0 );
  أكد( h_ڍير!=INVALID_HANDLE_قدر );
#إذا defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( os_ڍير_Validate(h_ڍير, SQLITE_WIN32_ڍير_FLAGS, p_اڳواٽ) );
#جيڪڏهن_پورو
  أكد( n_بائيٽون>=0 );
  إذا( !p_اڳواٽ ){
    p = os_ڍير_Alloc(h_ڍير, SQLITE_WIN32_ڍير_FLAGS, (سائيز_T)n_بائيٽون);
  }وإلا{
    p = os_ڍير_Re_Alloc(h_ڍير, SQLITE_WIN32_ڍير_FLAGS, p_اڳواٽ, (سائيز_T)n_بائيٽون);
  }
  إذا( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to %s %u bytes (%lu), heap=%p",
                p_اڳواٽ ? "HeapReAlloc" : "HeapAlloc", n_بائيٽون, os_Get_آخري_Error(),
                (فراغ*)h_ڍير);
  }
  أرجع p;
}

/*
** Return the size of an outstanding allocation, in bytes.
*/
ساكن صحيح win_Mem_سائيز(فراغ *p){
  HANDLE h_ڍير;
  سائيز_T n;

  winMemAssertMagic();
  h_ڍير = win_Mem_Get_ڍير();
  أكد( h_ڍير!=0 );
  أكد( h_ڍير!=INVALID_HANDLE_قدر );
#إذا defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( os_ڍير_Validate(h_ڍير, SQLITE_WIN32_ڍير_FLAGS, p) );
#جيڪڏهن_پورو
  إذا( !p ) أرجع 0;
  n = os_ڍير_سائيز(h_ڍير, SQLITE_WIN32_ڍير_FLAGS, p);
  إذا( n==(سائيز_T)-1 ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapSize block %p (%lu), heap=%p",
                p, os_Get_آخري_Error(), (فراغ*)h_ڍير);
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
ساكن صحيح winMemInit(فراغ *p_App_معلومات){
  win_Mem_معلومات *p_Win_Mem_معلومات = (win_Mem_معلومات *)p_App_معلومات;

  إذا( !p_Win_Mem_معلومات ) أرجع SQLITE_ERROR;
  أكد( p_Win_Mem_معلومات->magic1==WINMEM_MAGIC1 );
  أكد( p_Win_Mem_معلومات->magic2==WINMEM_MAGIC2 );

#إذا SQLITE_WIN32_ڍير_CREATE
  إذا( !p_Win_Mem_معلومات->h_ڍير ){
    DWORD dw_Initial_سائيز = SQLITE_WIN32_ڍير_INIT_سائيز;
    DWORD dw_وڌ_کان_وڌ_سائيز = (DWORD)sqlite3GlobalConfig.n_ڍير;
    إذا( dw_وڌ_کان_وڌ_سائيز==0 ){
      dw_وڌ_کان_وڌ_سائيز = SQLITE_WIN32_ڍير_MAX_سائيز;
    }وإلا إذا( dw_Initial_سائيز>dw_وڌ_کان_وڌ_سائيز ){
      dw_Initial_سائيز = dw_وڌ_کان_وڌ_سائيز;
    }
    p_Win_Mem_معلومات->h_ڍير = os_ڍير_Create(SQLITE_WIN32_ڍير_FLAGS,
                                      dw_Initial_سائيز, dw_وڌ_کان_وڌ_سائيز);
    إذا( !p_Win_Mem_معلومات->h_ڍير ){
      sqlite3_log(SQLITE_NOMEM,
          "failed to HeapCreate (%lu), flags=%u, initSize=%lu, maxSize=%lu",
          os_Get_آخري_Error(), SQLITE_WIN32_ڍير_FLAGS, dw_Initial_سائيز,
          dw_وڌ_کان_وڌ_سائيز);
      أرجع SQLITE_NOMEM_BKPT;
    }
    p_Win_Mem_معلومات->bOwned = TRUE;
    أكد( p_Win_Mem_معلومات->bOwned );
  }
#وإلا
  p_Win_Mem_معلومات->h_ڍير = os_Get_Process_ڍير();
  إذا( !p_Win_Mem_معلومات->h_ڍير ){
    sqlite3_log(SQLITE_NOMEM,
        "failed to GetProcessHeap (%lu)", os_Get_آخري_Error());
    أرجع SQLITE_NOMEM_BKPT;
  }
  p_Win_Mem_معلومات->bOwned = FALSE;
  أكد( !p_Win_Mem_معلومات->bOwned );
#جيڪڏهن_پورو
  أكد( p_Win_Mem_معلومات->h_ڍير!=0 );
  أكد( p_Win_Mem_معلومات->h_ڍير!=INVALID_HANDLE_قدر );
#إذا defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( os_ڍير_Validate(p_Win_Mem_معلومات->h_ڍير, SQLITE_WIN32_ڍير_FLAGS, عدم) );
#جيڪڏهن_پورو
  أرجع SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ساكن فراغ winMemShutdown(فراغ *p_App_معلومات){
  win_Mem_معلومات *p_Win_Mem_معلومات = (win_Mem_معلومات *)p_App_معلومات;

  إذا( !p_Win_Mem_معلومات ) أرجع;
  أكد( p_Win_Mem_معلومات->magic1==WINMEM_MAGIC1 );
  أكد( p_Win_Mem_معلومات->magic2==WINMEM_MAGIC2 );

  إذا( p_Win_Mem_معلومات->h_ڍير ){
    أكد( p_Win_Mem_معلومات->h_ڍير!=INVALID_HANDLE_قدر );
#إذا defined(SQLITE_WIN32_MALLOC_VALIDATE)
    أكد( os_ڍير_Validate(p_Win_Mem_معلومات->h_ڍير, SQLITE_WIN32_ڍير_FLAGS, عدم) );
#جيڪڏهن_پورو
    إذا( p_Win_Mem_معلومات->bOwned ){
      إذا( !os_ڍير_Destroy(p_Win_Mem_معلومات->h_ڍير) ){
        sqlite3_log(SQLITE_NOMEM, "failed to HeapDestroy (%lu), heap=%p",
                    os_Get_آخري_Error(), (فراغ*)p_Win_Mem_معلومات->h_ڍير);
      }
      p_Win_Mem_معلومات->bOwned = FALSE;
    }
    p_Win_Mem_معلومات->h_ڍير = عدم;
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
    win_Mem_سائيز,
    winMemRoundup,
    winMemInit,
    winMemShutdown,
    &win_mem_معلومات
  };
  أرجع &winMemMethods;
}

فراغ sqlite3_Mem_سيٽ_Default(فراغ){
  sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetWin32());
}
#جيڪڏهن_پورو /* SQLITE_WIN32_MALLOC */

#جيڪڏهن_وصف_ٿيل _WIN32
/*
** Convert a UTF-8 string to Microsoft Unicode.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساكن LPWSTR win_Utf8_منزل_Unicode(ثابت حرف *z_متن){
  صحيح nChar;
  LPWSTR z_Wide_متن;

  nChar = os_Multi_Byte_منزل_Wide_Char(CP_UTF8, 0, z_متن, -1, عدم, 0);
  إذا( nChar==0 ){
    أرجع 0;
  }
  z_Wide_متن = sqlite3MallocZero( nChar*حجم(WCHAR) );
  إذا( z_Wide_متن==0 ){
    أرجع 0;
  }
  nChar = os_Multi_Byte_منزل_Wide_Char(CP_UTF8, 0, z_متن, -1, z_Wide_متن,
                                nChar);
  إذا( nChar==0 ){
    sqlite3_free(z_Wide_متن);
    z_Wide_متن = 0;
  }
  أرجع z_Wide_متن;
}
#جيڪڏهن_پورو /* _WIN32 */

/*
** Convert a Microsoft Unicode string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساكن حرف *win_Unicode_منزل_Utf8(LPCWSTR z_Wide_متن){
  صحيح nByte;
  حرف *z_متن;

  nByte = os_Wide_Char_منزل_Multi_Byte(CP_UTF8, 0, z_Wide_متن, -1, 0, 0, 0, 0);
  إذا( nByte == 0 ){
    أرجع 0;
  }
  z_متن = sqlite3MallocZero( nByte );
  إذا( z_متن==0 ){
    أرجع 0;
  }
  nByte = os_Wide_Char_منزل_Multi_Byte(CP_UTF8, 0, z_Wide_متن, -1, z_متن, nByte,
                                0, 0);
  إذا( nByte == 0 ){
    sqlite3_free(z_متن);
    z_متن = 0;
  }
  أرجع z_متن;
}

#جيڪڏهن_وصف_ٿيل _WIN32
/*
** Convert an ANSI string to Microsoft Unicode, using the ANSI or OEM
** code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساكن LPWSTR win_Mbcs_منزل_Unicode(ثابت حرف *z_متن, صحيح useAnsi){
  صحيح nWideChar;
  LPWSTR z_Mbcs_متن;
  صحيح codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nWideChar = os_Multi_Byte_منزل_Wide_Char(codepage, 0, z_متن, -1, عدم,
                                0);
  إذا( nWideChar==0 ){
    أرجع 0;
  }
  z_Mbcs_متن = sqlite3MallocZero( nWideChar*حجم(WCHAR) );
  إذا( z_Mbcs_متن==0 ){
    أرجع 0;
  }
  nWideChar = os_Multi_Byte_منزل_Wide_Char(codepage, 0, z_متن, -1, z_Mbcs_متن,
                                nWideChar);
  إذا( nWideChar==0 ){
    sqlite3_free(z_Mbcs_متن);
    z_Mbcs_متن = 0;
  }
  أرجع z_Mbcs_متن;
}

/*
** Convert a Microsoft Unicode string to a multi-byte character string,
** using the ANSI or OEM code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساكن حرف *win_Unicode_منزل_Mbcs(LPCWSTR z_Wide_متن, صحيح useAnsi){
  صحيح nByte;
  حرف *z_متن;
  صحيح codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nByte = os_Wide_Char_منزل_Multi_Byte(codepage, 0, z_Wide_متن, -1, 0, 0, 0, 0);
  إذا( nByte == 0 ){
    أرجع 0;
  }
  z_متن = sqlite3MallocZero( nByte );
  إذا( z_متن==0 ){
    أرجع 0;
  }
  nByte = os_Wide_Char_منزل_Multi_Byte(codepage, 0, z_Wide_متن, -1, z_متن,
                                nByte, 0, 0);
  إذا( nByte == 0 ){
    sqlite3_free(z_متن);
    z_متن = 0;
  }
  أرجع z_متن;
}

/*
** Convert a multi-byte character string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساكن حرف *win_Mbcs_منزل_Utf8(ثابت حرف *z_متن, صحيح useAnsi){
  حرف *z_متن_Utf8;
  LPWSTR zTmpWide;

  zTmpWide = win_Mbcs_منزل_Unicode(z_متن, useAnsi);
  إذا( zTmpWide==0 ){
    أرجع 0;
  }
  z_متن_Utf8 = win_Unicode_منزل_Utf8(zTmpWide);
  sqlite3_free(zTmpWide);
  أرجع z_متن_Utf8;
}

/*
** Convert a UTF-8 string to a multi-byte character string.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساكن حرف *win_Utf8_منزل_Mbcs(ثابت حرف *z_متن, صحيح useAnsi){
  حرف *z_متن_Mbcs;
  LPWSTR zTmpWide;

  zTmpWide = win_Utf8_منزل_Unicode(z_متن);
  إذا( zTmpWide==0 ){
    أرجع 0;
  }
  z_متن_Mbcs = win_Unicode_منزل_Mbcs(zTmpWide, useAnsi);
  sqlite3_free(zTmpWide);
  أرجع z_متن_Mbcs;
}

/*
** This is a public wrapper for the winUtf8ToUnicode() function.
*/
LPWSTR sqlite3_win32_utf8_منزل_unicode(ثابت حرف *z_متن){
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( !z_متن ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#جيڪڏهن_پورو
  أرجع win_Utf8_منزل_Unicode(z_متن);
}

/*
** This is a public wrapper for the winUnicodeToUtf8() function.
*/
حرف *sqlite3_win32_unicode_منزل_utf8(LPCWSTR z_Wide_متن){
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( !z_Wide_متن ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#جيڪڏهن_پورو
  أرجع win_Unicode_منزل_Utf8(z_Wide_متن);
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
حرف *sqlite3_win32_mbcs_منزل_utf8(ثابت حرف *z_متن){
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( !z_متن ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#جيڪڏهن_پورو
  أرجع win_Mbcs_منزل_Utf8(z_متن, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
حرف *sqlite3_win32_mbcs_منزل_utf8_v2(ثابت حرف *z_متن, صحيح useAnsi){
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( !z_متن ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#جيڪڏهن_پورو
  أرجع win_Mbcs_منزل_Utf8(z_متن, useAnsi);
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
حرف *sqlite3_win32_utf8_منزل_mbcs(ثابت حرف *z_متن){
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( !z_متن ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#جيڪڏهن_پورو
  أرجع win_Utf8_منزل_Mbcs(z_متن, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
حرف *sqlite3_win32_utf8_منزل_mbcs_v2(ثابت حرف *z_متن, صحيح useAnsi){
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( !z_متن ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  إذا( sqlite3_initialize() ) أرجع 0;
#جيڪڏهن_پورو
  أرجع win_Utf8_منزل_Mbcs(z_متن, useAnsi);
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-8 string.
*/
صحيح sqlite3_win32_سيٽ_directory8(
  بنانشان ڊگهو type, /* Identifier for directory being set or reset */
  ثابت حرف *z_قدر  /* New value for directory being set or reset */
){
  حرف **ppDirectory = 0;
  صحيح rc;
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  إذا( rc ) أرجع rc;
#جيڪڏهن_پورو
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  إذا( type==SQLITE_WIN32_معلومات_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_معلومات_directory;
  }وإلا إذا( type==SQLITE_WIN32_عارضي_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_عارضي_directory;
  }
  أكد( !ppDirectory || type==SQLITE_WIN32_معلومات_DIRECTORY_TYPE
          || type==SQLITE_WIN32_عارضي_DIRECTORY_TYPE
  );
  أكد( !ppDirectory || sqlite3MemdebugHasType(*ppDirectory, MEMTYPE_ڍير) );
  إذا( ppDirectory ){
    حرف *zCopy = 0;
    إذا( z_قدر && z_قدر[0] ){
      zCopy = sqlite3_mprintf("%s", z_قدر);
      إذا ( zCopy==0 ){
        rc = SQLITE_NOMEM_BKPT;
        اذهب سيٽ_directory8_پورو;
      }
    }
    sqlite3_free(*ppDirectory);
    *ppDirectory = zCopy;
    rc = SQLITE_OK;
  }وإلا{
    rc = SQLITE_ERROR;
  }
سيٽ_directory8_پورو:
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  أرجع rc;
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-16 string.
*/
صحيح sqlite3_win32_سيٽ_directory16(
  بنانشان ڊگهو type, /* Identifier for directory being set or reset */
  ثابت فراغ *z_قدر  /* New value for directory being set or reset */
){
  صحيح rc;
  حرف *zUtf8 = 0;
  إذا( z_قدر ){
    zUtf8 = sqlite3_win32_unicode_منزل_utf8(z_قدر);
    إذا( zUtf8==0 ) أرجع SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_win32_سيٽ_directory8(type, zUtf8);
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
صحيح sqlite3_win32_سيٽ_directory(
  بنانشان ڊگهو type, /* Identifier for directory being set or reset */
  فراغ *z_قدر        /* New value for directory being set or reset */
){
  أرجع sqlite3_win32_سيٽ_directory16(type, z_قدر);
}
#جيڪڏهن_پورو /* _WIN32 */

/*
** The return value of winGetLastErrorMsg
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated).
*/
ساكن صحيح win_Get_آخري_Error_نياپو(DWORD آخري_Errno, صحيح nBuf, حرف *zBuf){
  /* FormatMessage returns 0 on failure.  Otherwise it
  ** returns the number of TCHARs written to the output
  ** buffer, excluding the terminating null char.
  */
  DWORD dwLen = 0;
  حرف *z_ٻاهريون = 0;
  LPWSTR z_عارضي_Wide = عدم;
  dwLen = os_Format_پيغام_W(FORMAT_پيغام_ALLOCATE_بفر |
                           FORMAT_پيغام_FROM_SYSTEM |
                           FORMAT_پيغام_IGNORE_INSERTS,
                           عدم,
                           آخري_Errno,
                           0,
                           (LPWSTR) &z_عارضي_Wide,
                           0,
                           0);
  إذا( dwLen > 0 ){
    /* allocate a buffer and convert to UTF8 */
    sqlite3BeginBenignMalloc();
    z_ٻاهريون = win_Unicode_منزل_Utf8(z_عارضي_Wide);
    sqlite3_پڇاڙي_Benign_Malloc();
    /* free the system buffer allocated by FormatMessage */
    osLocalFree(z_عارضي_Wide);
  }
  إذا( 0 == dwLen ){
    sqlite3_snprintf(nBuf, zBuf, "OsError 0x%lx (%lu)", آخري_Errno, آخري_Errno);
  }وإلا{
    /* copy a maximum of nBuf chars to output buffer */
    sqlite3_snprintf(nBuf, zBuf, "%s", z_ٻاهريون);
    /* free the UTF8 buffer */
    sqlite3_free(z_ٻاهريون);
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
#وصف_ڪريو winLogError(a,b,c,d)   win_Log_Error_At_سٽ(a,b,c,d,__LINE__)
ساكن صحيح win_Log_Error_At_سٽ(
  صحيح errcode,                    /* SQLite error code */
  DWORD آخري_Errno,                /* Win32 last error */
  ثابت حرف *zFunc,              /* Name of OS function that failed */
  ثابت حرف *z_رستو,              /* File path associated with error */
  صحيح i_سٽ                       /* Source line number where error occurred */
){
  حرف z_نياپو[500];                 /* Human readable error text */
  صحيح i;                          /* Loop counter */

  z_نياپو[0] = 0;
  win_Get_آخري_Error_نياپو(آخري_Errno, حجم(z_نياپو), z_نياپو);
  أكد( errcode!=SQLITE_OK );
  إذا( z_رستو==0 ) z_رستو = "";
  لكل(i=0; z_نياپو[i] && z_نياپو[i]!='\r' && z_نياپو[i]!='\n'; i++){}
  z_نياپو[i] = 0;
  sqlite3_log(errcode,
      "os_win.c:%d: (%lu) %s(%s) - %s",
      i_سٽ, آخري_Errno, zFunc, z_رستو, z_نياپو
  );

  أرجع errcode;
}

/*
** The number of times that a ReadFile(), WriteFile(), and DeleteFile()
** will be retried following a locking error - probably caused by
** antivirus software.  Also the initial delay before the first retry.
** The delay increases linearly with each retry.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WIN32_IOERR_RETRY
# define SQLITE_WIN32_IOERR_RETRY 10
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل SQLITE_WIN32_IOERR_RETRY_DELAY
# define SQLITE_WIN32_IOERR_RETRY_DELAY 25
#جيڪڏهن_پورو
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
#وصف_ڪريو winIoerrCanRetry1(a) (((a)==ERROR_ACCESS_DENIED)        || \
                              ((a)==ERROR_SHARING_VIOLATION)    || \
                              ((a)==ERROR_تالو_VIOLATION)       || \
                              ((a)==ERROR_DEV_NOT_EXIST)        || \
                              ((a)==ERROR_NETNAME_DELETED)      || \
                              ((a)==ERROR_SEM_TIMEOUT)          || \
                              ((a)==ERROR_ڄار_UNREACHABLE))
#جيڪڏهن_پورو

/*
** If a ReadFile() or WriteFile() error occurs, invoke this routine
** to see if it should be retried.  Return TRUE to retry.  Return FALSE
** to give up with an error.
*/
ساكن صحيح winRetryIoerr(صحيح *pnRetry, DWORD *pError){
  DWORD e = os_Get_آخري_Error();
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
#جيڪڏهن_پورو
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
ساكن BOOL win_تالو_File(
  LPHANDLE phFile,
  DWORD flags,
  DWORD وٿي_هيٺيون,
  DWORD وٿي_مٿاهون,
  DWORD انگ_بائيٽون_هيٺيون,
  DWORD انگ_بائيٽون_مٿاهون
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, حجم(OVERLAPPED));
  ovlp.وٿي_2 = وٿي_هيٺيون;
  ovlp.وٿي_مٿاهون_2 = وٿي_مٿاهون;
  أرجع os_تالو_File_Ex(*phFile, flags, 0, انگ_بائيٽون_هيٺيون, انگ_بائيٽون_مٿاهون, &ovlp);
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WAL
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
ساكن صحيح win_Handle_تالو_Timeout(
  HANDLE hFile,
  DWORD وٿي,
  DWORD nByte,
  صحيح bExcl,
  DWORD nMs
){
  DWORD flags = LOCKFILE_FAIL_IMMEDIATELY | (bExcl?LOCKFILE_EXCLUSIVE_تالو:0);
  صحيح rc = SQLITE_OK;
  BOOL ret;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, حجم(OVERLAPPED));
  ovlp.وٿي_2 = وٿي;

#إذا defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  إذا( nMs!=0 ){
    flags &= ~LOCKFILE_FAIL_IMMEDIATELY;
  }
  ovlp.hEvent = osCreateEvent(عدم, TRUE, FALSE, عدم);
  إذا( ovlp.hEvent==عدم ){
    أرجع SQLITE_IOERR_تالو;
  }
#جيڪڏهن_پورو
  ret = os_تالو_File_Ex(hFile, flags, 0, nByte, 0, &ovlp);
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
  إذا( !ret && Get_آخري_Error()==ERROR_IO_PENDING ){
    DWORD nDelay = (nMs==0 ? INFINITE : nMs);
    DWORD res = osWaitForSingleObject(ovlp.hEvent, nDelay);
    إذا( res==WAIT_OBJECT_0 ){
      ret = TRUE;
    }وإلا إذا( res==WAIT_TIMEOUT ){
#إذا SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = SQLITE_BUSY_TIMEOUT;
#وإلا
      rc = SQLITE_BUSY;
#جيڪڏهن_پورو
    }وإلا{
      /* Some other error has occurred */
      rc = SQLITE_IOERR_تالو;
    }

    /* If it is still pending, cancel the LockFileEx() call. */
    osCancelIo(hFile);
  }

  osCloseHandle(ovlp.hEvent);
#جيڪڏهن_پورو /* defined(SQLITE_ENABLE_SETLK_TIMEOUT) */

  إذا( rc==SQLITE_OK && !ret ){
    rc = SQLITE_BUSY;
  }
  أرجع rc;
}
#جيڪڏهن_پورو /* #ifndef SQLITE_OMIT_WAL */

/*
** Unlock a file region.
 */
ساكن BOOL winUnlockFile(
  LPHANDLE phFile,
  DWORD وٿي_هيٺيون,
  DWORD وٿي_مٿاهون,
  DWORD انگ_بائيٽون_هيٺيون,
  DWORD انگ_بائيٽون_مٿاهون
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, حجم(OVERLAPPED));
  ovlp.وٿي_2 = وٿي_هيٺيون;
  ovlp.وٿي_مٿاهون_2 = وٿي_مٿاهون;
  أرجع osUnlockFileEx(*phFile, 0, انگ_بائيٽون_هيٺيون, انگ_بائيٽون_مٿاهون, &ovlp);
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WAL
/*
** Remove an nByte lock starting at offset iOff from HANDLE h.
*/
ساكن صحيح winHandleUnlock(HANDLE h, صحيح iOff, صحيح nByte){
  BOOL ret = winUnlockFile(&h, iOff, 0, nByte, 0);
  أرجع (ret ? SQLITE_OK : SQLITE_IOERR_UNLOCK);
}
#جيڪڏهن_پورو

/*****************************************************************************
** The next group of routines implement the I/O methods specified
** by the sqlite3_io_methods object.
******************************************************************************/

/*
** Some Microsoft compilers lack this definition.
*/
#جيڪڏهن_وصف_نه_ٿيل INVALID_سيٽ_FILE_اشارو
# define INVALID_سيٽ_FILE_اشارو ((DWORD)-1)
#جيڪڏهن_پورو

/*
** Seek the file handle h to offset nByte of the file.
**
** If successful, return SQLITE_OK. Or, if an error occurs, return an SQLite
** error code.
*/
ساكن صحيح winHandleSeek(HANDLE h, sqlite3_int64 i_وٿي){
  صحيح rc = SQLITE_OK;             /* Return value */
  LARGE_INTEGER x;                /* The offset */

  x.QuadPart = i_وٿي;
  إذا( os_سيٽ_File_اشارو_Ex(h, x, 0, FILE_BEGIN)==0 ){
    rc = SQLITE_IOERR_SEEK;
  }
  OSTRACE(("SEEK file=%p, offset=%lld rc=%s\n", h, i_وٿي,sqlite3ErrName(rc)));
  أرجع rc;
}

/*
** Move the current position of the file handle passed as the first
** argument to offset iOffset within the file. If successful, return 0.
** Otherwise, set pFile->lastErrno and return non-zero.
*/
ساكن صحيح winSeekFile(winFile *pFile, sqlite3_int64 i_وٿي){
  صحيح rc;

  rc = winHandleSeek(pFile->h, i_وٿي);
  إذا( rc!=SQLITE_OK ){
    pFile->آخري_Errno = os_Get_آخري_Error();
    winLogError(rc, pFile->آخري_Errno, "winSeekFile", pFile->z_رستو);
  }
  أرجع rc;
}


#إذا SQLITE_MAX_MMAP_سائيز>0
/* Forward references to VFS helper methods used for memory mapped files */
ساكن صحيح winMapfile(winFile*, sqlite3_int64);
ساكن صحيح winUnmapfile(winFile*);
#جيڪڏهن_پورو

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
#وصف_ڪريو MX_CLOSE_ATTEMPT 3
ساكن صحيح winClose(sqlite3_file *id){
  صحيح rc, cnt = 0;
  winFile *pFile = (winFile*)id;

  أكد( id!=0 );
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WAL
  أكد( pFile->pShm==0 );
#جيڪڏهن_پورو
  أكد( pFile->h!=عدم && pFile->h!=INVALID_HANDLE_قدر );
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p\n",
           osGetCurrentProcessId(), pFile, pFile->h));

#إذا SQLITE_MAX_MMAP_سائيز>0
  winUnmapfile(pFile);
#جيڪڏهن_پورو

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
            : winLogError(SQLITE_IOERR_CLOSE, os_Get_آخري_Error(),
                          "winClose", pFile->z_رستو);
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
  sqlite3_int64 وٿي       /* Begin reading at this offset */
){
#إذا !defined(SQLITE_WIN32_NO_OVERLAPPED)
  OVERLAPPED overlapped;          /* The offset for ReadFile. */
#جيڪڏهن_پورو
  winFile *pFile = (winFile*)id;  /* file handle */
  DWORD nRead;                    /* Number of bytes actually read from file */
  صحيح nRetry = 0;                 /* Number of retrys */

  أكد( id!=0 );
  أكد( amt>0 );
  أكد( وٿي>=0 );
  SimulateIOError(أرجع SQLITE_IOERR_READ);
  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, وٿي, pFile->locktype));

#إذا SQLITE_MAX_MMAP_سائيز>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  إذا( وٿي<pFile->mmap_سائيز ){
    إذا( وٿي+amt <= pFile->mmap_سائيز ){
      يادگيري_نقل(pBuf, &((u8 *)(pFile->pMapRegion))[وٿي], amt);
      OSTRACE(("READ-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      أرجع SQLITE_OK;
    }وإلا{
      صحيح nCopy = (صحيح)(pFile->mmap_سائيز - وٿي);
      يادگيري_نقل(pBuf, &((u8 *)(pFile->pMapRegion))[وٿي], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      وٿي += nCopy;
    }
  }
#جيڪڏهن_پورو

#إذا defined(SQLITE_WIN32_NO_OVERLAPPED)
  إذا( winSeekFile(pFile, وٿي) ){
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    أرجع SQLITE_ڀريل;
  }
  طالما( !osReadFile(pFile->h, pBuf, amt, &nRead, 0) ){
#وإلا
  memset(&overlapped, 0, حجم(OVERLAPPED));
  overlapped.وٿي_2 = (LONG)(وٿي & 0xffffffff);
  overlapped.وٿي_مٿاهون_2 = (LONG)((وٿي>>32) & 0x7fffffff);
  طالما( !osReadFile(pFile->h, pBuf, amt, &nRead, &overlapped) &&
         os_Get_آخري_Error()!=ERROR_HANDLE_EOF ){
#جيڪڏهن_پورو
    DWORD آخري_Errno;
    إذا( winRetryIoerr(&nRetry, &آخري_Errno) ) استمر;
    pFile->آخري_Errno = آخري_Errno;
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    أرجع winLogError(SQLITE_IOERR_READ, pFile->آخري_Errno,
                       "winRead", pFile->z_رستو);
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
  sqlite3_int64 وٿي            /* Offset into the file to begin writing at */
){
  صحيح rc = 0;                     /* True if error has occurred, else false */
  winFile *pFile = (winFile*)id;  /* File handle */
  صحيح nRetry = 0;                 /* Number of retries */

  أكد( amt>0 );
  أكد( pFile );
  SimulateIOError(أرجع SQLITE_IOERR_WRITE);
  SimulateDiskfullError(أرجع SQLITE_ڀريل);

  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, وٿي, pFile->locktype));

#إذا defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_سائيز>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  إذا( وٿي<pFile->mmap_سائيز ){
    إذا( وٿي+amt <= pFile->mmap_سائيز ){
      يادگيري_نقل(&((u8 *)(pFile->pMapRegion))[وٿي], pBuf, amt);
      OSTRACE(("WRITE-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      أرجع SQLITE_OK;
    }وإلا{
      صحيح nCopy = (صحيح)(pFile->mmap_سائيز - وٿي);
      يادگيري_نقل(&((u8 *)(pFile->pMapRegion))[وٿي], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      وٿي += nCopy;
    }
  }
#جيڪڏهن_پورو

#إذا defined(SQLITE_WIN32_NO_OVERLAPPED)
  rc = winSeekFile(pFile, وٿي);
  إذا( rc==0 ){
#وإلا
  {
#جيڪڏهن_پورو
#إذا !defined(SQLITE_WIN32_NO_OVERLAPPED)
    OVERLAPPED overlapped;        /* The offset for WriteFile. */
#جيڪڏهن_پورو
    u8 *aRem = (u8 *)pBuf;        /* Data yet to be written */
    صحيح nRem = amt;               /* Number of bytes yet to be written */
    DWORD nWrite;                 /* Bytes written by each WriteFile() call */
    DWORD آخري_Errno = NO_ERROR;   /* Value returned by GetLastError() */

#إذا !defined(SQLITE_WIN32_NO_OVERLAPPED)
    memset(&overlapped, 0, حجم(OVERLAPPED));
    overlapped.وٿي_2 = (LONG)(وٿي & 0xffffffff);
    overlapped.وٿي_مٿاهون_2 = (LONG)((وٿي>>32) & 0x7fffffff);
#جيڪڏهن_پورو

    طالما( nRem>0 ){
#إذا defined(SQLITE_WIN32_NO_OVERLAPPED)
      إذا( !osWriteFile(pFile->h, aRem, nRem, &nWrite, 0) ){
#وإلا
      إذا( !osWriteFile(pFile->h, aRem, nRem, &nWrite, &overlapped) ){
#جيڪڏهن_پورو
        إذا( winRetryIoerr(&nRetry, &آخري_Errno) ) استمر;
        اكسر;
      }
      أكد( nWrite==0 || nWrite<=(DWORD)nRem );
      إذا( nWrite==0 || nWrite>(DWORD)nRem ){
        آخري_Errno = os_Get_آخري_Error();
        اكسر;
      }
#إذا !defined(SQLITE_WIN32_NO_OVERLAPPED)
      وٿي += nWrite;
      overlapped.وٿي_2 = (LONG)(وٿي & 0xffffffff);
      overlapped.وٿي_مٿاهون_2 = (LONG)((وٿي>>32) & 0x7fffffff);
#جيڪڏهن_پورو
      aRem += nWrite;
      nRem -= nWrite;
    }
    إذا( nRem>0 ){
      pFile->آخري_Errno = آخري_Errno;
      rc = 1;
    }
  }

  إذا( rc ){
    إذا(   ( pFile->آخري_Errno==ERROR_HANDLE_DISK_ڀريل )
       || ( pFile->آخري_Errno==ERROR_DISK_ڀريل )){
      OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      أرجع winLogError(SQLITE_ڀريل, pFile->آخري_Errno,
                         "winWrite1", pFile->z_رستو);
    }
    OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_WRITE\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    أرجع winLogError(SQLITE_IOERR_WRITE, pFile->آخري_Errno,
                       "winWrite2", pFile->z_رستو);
  }وإلا{
    winLogIoerr(nRetry, __LINE__);
  }
  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  أرجع SQLITE_OK;
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WAL
/*
** Truncate the file opened by handle h to nByte bytes in size.
*/
ساكن صحيح winHandleTruncate(HANDLE h, sqlite3_int64 nByte){ 
  صحيح rc = SQLITE_OK;             /* Return code */
  rc = winHandleSeek(h, nByte);
  إذا( rc==SQLITE_OK ){
    إذا( 0==os_سيٽ_پڇاڙي_جو_File(h) ){
      rc = SQLITE_IOERR_TRUNCATE;
    }
  }
  أرجع rc;
}

/*
** Determine the size in bytes of the file opened by the handle passed as 
** the first argument.
*/
ساكن صحيح win_Handle_سائيز(HANDLE h, sqlite3_int64 *pnByte){ 
  صحيح rc = SQLITE_OK;
  LARGE_INTEGER x;
  أكد( pnByte );
  إذا( os_Get_File_سائيز_Ex(h, &x)==0 ){
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
  إذا( h!=INVALID_HANDLE_قدر ){
    osCloseHandle(h);
  }
}
#جيڪڏهن_پورو /* #ifndef SQLITE_OMIT_WAL */

/*
** Truncate an open file to a specified size
*/
ساكن صحيح winTruncate(sqlite3_file *id, sqlite3_int64 nByte){
  winFile *pFile = (winFile*)id;  /* File handle object */
  صحيح rc = SQLITE_OK;             /* Return code for this function */
  DWORD آخري_Errno;
#إذا SQLITE_MAX_MMAP_سائيز>0
  sqlite3_int64 old_Mmap_سائيز;
  إذا( pFile->n_Fetch_ٻاهريون>0 ){
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
#جيڪڏهن_پورو

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

#إذا SQLITE_MAX_MMAP_سائيز>0
  إذا( pFile->pMapRegion ){
    old_Mmap_سائيز = pFile->mmap_سائيز;
  }وإلا{
    old_Mmap_سائيز = 0;
  }
  winUnmapfile(pFile);
#جيڪڏهن_پورو

  /* SetEndOfFile() returns non-zero when successful, or zero when it fails. */
  إذا( winSeekFile(pFile, nByte) ){
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->آخري_Errno,
                     "winTruncate1", pFile->z_رستو);
  }وإلا إذا( 0==os_سيٽ_پڇاڙي_جو_File(pFile->h) &&
            ((آخري_Errno = os_Get_آخري_Error())!=ERROR_USER_MAPPED_FILE) ){
    pFile->آخري_Errno = آخري_Errno;
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->آخري_Errno,
                     "winTruncate2", pFile->z_رستو);
  }

#إذا SQLITE_MAX_MMAP_سائيز>0
  إذا( rc==SQLITE_OK && old_Mmap_سائيز>0 ){
    إذا( old_Mmap_سائيز>nByte ){
      winMapfile(pFile, -1);
    }وإلا{
      winMapfile(pFile, old_Mmap_سائيز);
    }
  }
#جيڪڏهن_پورو

  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, sqlite3ErrName(rc)));
  أرجع rc;
}

#جيڪڏهن_وصف_ٿيل SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occurring at the right times.
*/
صحيح sqlite3_sync_ڳڻپ = 0;
صحيح sqlite3_fullsync_ڳڻپ = 0;
#جيڪڏهن_پورو

/*
** Make sure all writes to a particular file are committed to disk.
*/
ساكن صحيح winSync(sqlite3_file *id, صحيح flags){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_NO_SYNC
  /*
  ** Used only when SQLITE_NO_SYNC is not defined.
   */
  BOOL rc;
#جيڪڏهن_پورو
#إذا !defined(NDEBUG) || !defined(SQLITE_NO_SYNC) || \
    defined(SQLITE_HAVE_OS_TRACE)
  /*
  ** Used when SQLITE_NO_SYNC is not defined and by the assert() and/or
  ** OSTRACE() macros.
   */
  winFile *pFile = (winFile*)id;
#وإلا
  اڻ_استعمال_ٿيل_PARAMETER(id);
#جيڪڏهن_پورو

  أكد( pFile );
  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  أكد((flags&0x0F)==SQLITE_SYNC_عام
      || (flags&0x0F)==SQLITE_SYNC_ڀريل
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( أرجع SQLITE_ڀريل );

  OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, flags=%x, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, flags,
           pFile->locktype));

#جيڪڏهن_وصف_نه_ٿيل SQLITE_TEST
  اڻ_استعمال_ٿيل_PARAMETER(flags);
#وإلا
  إذا( (flags&0x0F)==SQLITE_SYNC_ڀريل ){
    sqlite3_fullsync_ڳڻپ++;
  }
  sqlite3_sync_ڳڻپ++;
#جيڪڏهن_پورو

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op
  */
#جيڪڏهن_وصف_ٿيل SQLITE_NO_SYNC
  OSTRACE(("SYNC-NOP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  أرجع SQLITE_OK;
#وإلا
#إذا SQLITE_MAX_MMAP_سائيز>0
  إذا( pFile->pMapRegion ){
    إذا( os_Flush_View_جو_File(pFile->pMapRegion, 0) ){
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_OK\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
    }وإلا{
      pFile->آخري_Errno = os_Get_آخري_Error();
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
      أرجع winLogError(SQLITE_IOERR_MMAP, pFile->آخري_Errno,
                         "winSync1", pFile->z_رستو);
    }
  }
#جيڪڏهن_پورو
  rc = osFlushFileBuffers(pFile->h);
  SimulateIOError( rc=FALSE );
  إذا( rc ){
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    أرجع SQLITE_OK;
  }وإلا{
    pFile->آخري_Errno = os_Get_آخري_Error();
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_FSYNC\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    أرجع winLogError(SQLITE_IOERR_FSYNC, pFile->آخري_Errno,
                       "winSync2", pFile->z_رستو);
  }
#جيڪڏهن_پورو
}

/*
** Determine the current size of a file in bytes
*/
ساكن صحيح win_File_سائيز(sqlite3_file *id, sqlite3_int64 *p_سائيز){
  winFile *pFile = (winFile*)id;
  صحيح rc = SQLITE_OK;

  أكد( id!=0 );
  أكد( p_سائيز!=0 );
  SimulateIOError(أرجع SQLITE_IOERR_FSTAT);
  OSTRACE(("SIZE file=%p, pSize=%p\n", pFile->h, p_سائيز));
  {
    LARGE_INTEGER x;
    إذا( os_Get_File_سائيز_Ex(pFile->h, &x)==0 ){
      *p_سائيز = 0;
      pFile->آخري_Errno = os_Get_آخري_Error();
      rc = winLogError(SQLITE_IOERR_FSTAT, pFile->آخري_Errno,
                       "winFileSize", pFile->z_رستو);
    }وإلا{
      *p_سائيز = x.QuadPart;
    }
  }
  OSTRACE(("SIZE file=%p, pSize=%p, *pSize=%lld, rc=%s\n",
           pFile->h, p_سائيز, *p_سائيز, sqlite3ErrName(rc)));
  أرجع rc;
}

/*
** LOCKFILE_FAIL_IMMEDIATELY is undefined on some Windows systems.
*/
#جيڪڏهن_وصف_نه_ٿيل LOCKFILE_FAIL_IMMEDIATELY
# define LOCKFILE_FAIL_IMMEDIATELY 1
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل LOCKFILE_EXCLUSIVE_تالو
# define LOCKFILE_EXCLUSIVE_تالو 2
#جيڪڏهن_پورو

/*
** Historically, SQLite has used both the LockFile and LockFileEx functions.
** When the LockFile function was used, it was always expected to fail
** immediately if the lock could not be obtained.  Also, it always expected to
** obtain an exclusive lock.  These flags are used with the LockFileEx function
** and reflect those expectations; therefore, they should not be changed.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_LOCKFILE_FLAGS
# define SQLITE_LOCKFILE_FLAGS   (LOCKFILE_FAIL_IMMEDIATELY | \
                                  LOCKFILE_EXCLUSIVE_تالو)
#جيڪڏهن_پورو

/*
** Currently, SQLite never calls the LockFileEx function without wanting the
** call to fail immediately if the lock cannot be obtained.
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_LOCKFILEEX_FLAGS
# define SQLITE_LOCKFILEEX_FLAGS (LOCKFILE_FAIL_IMMEDIATELY)
#جيڪڏهن_پورو

/*
** Acquire a reader lock.
** Different API routines are called depending on whether or not this
** is Win9x or WinNT.
*/
ساكن صحيح win_Get_Read_تالو(winFile *pFile, صحيح b_بلاڪ){
  صحيح res;
  DWORD نقاب = ~(b_بلاڪ ? LOCKFILE_FAIL_IMMEDIATELY : 0);
  OSTRACE(("READ-LOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = win_تالو_File(&pFile->h, SQLITE_LOCKFILEEX_FLAGS&نقاب, SHARED_پهريون, 0,
                    SHARED_سائيز, 0);
  إذا( res == 0 ){
    pFile->آخري_Errno = os_Get_آخري_Error();
    /* No need to log a failure to lock */
  }
  OSTRACE(("READ-LOCK file=%p, result=%d\n", pFile->h, res));
  أرجع res;
}

/*
** Undo a readlock
*/
ساكن صحيح win_Unlock_Read_تالو(winFile *pFile){
  صحيح res;
  DWORD آخري_Errno;
  OSTRACE(("READ-UNLOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winUnlockFile(&pFile->h, SHARED_پهريون, 0, SHARED_سائيز, 0);
  إذا( res==0 && ((آخري_Errno = os_Get_آخري_Error())!=ERROR_NOT_LOCKED) ){
    pFile->آخري_Errno = آخري_Errno;
    winLogError(SQLITE_IOERR_UNLOCK, pFile->آخري_Errno,
                "winUnlockReadLock", pFile->z_رستو);
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
ساكن صحيح win_تالو(sqlite3_file *id, صحيح locktype){
  صحيح rc = SQLITE_OK;    /* Return code from subroutines */
  صحيح res = 1;           /* Result of a Windows lock call */
  صحيح newLocktype;       /* Set pFile->locktype to this value before exiting */
  صحيح got_Pending_تالو = 0;/* True if we acquired a PENDING lock this time */
  winFile *pFile = (winFile*)id;
  DWORD آخري_Errno = NO_ERROR;

  أكد( id!=0 );
  OSTRACE(("LOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->shared_تالو_Byte, locktype));

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
  إذا( (pFile->ctrlFlags & WINFILE_RDONLY)!=0 && locktype>=RESERVED_تالو ){
    أرجع SQLITE_IOERR_تالو;
  }

  /* Make sure the locking sequence is correct
  */
  أكد( pFile->locktype!=NO_تالو || locktype==SHARED_تالو );
  أكد( locktype!=PENDING_تالو );
  أكد( locktype!=RESERVED_تالو || pFile->locktype==SHARED_تالو );

  /* Lock the PENDING_LOCK byte if we need to acquire an EXCLUSIVE lock or
  ** a SHARED lock.  If we are acquiring a SHARED lock, the acquisition of
  ** the PENDING_LOCK byte is temporary.
  */
  newLocktype = pFile->locktype;
  إذا( locktype==SHARED_تالو
   || (locktype==EXCLUSIVE_تالو && pFile->locktype==RESERVED_تالو)
  ){
    صحيح cnt = 3;

    /* Flags for the LockFileEx() call. This should be an exclusive lock if
    ** this call is to obtain EXCLUSIVE, or a shared lock if this call is to
    ** obtain SHARED.  */
    صحيح flags = LOCKFILE_FAIL_IMMEDIATELY;
    إذا( locktype==EXCLUSIVE_تالو ){
      flags |= LOCKFILE_EXCLUSIVE_تالو;
    }
    طالما( cnt>0 ){
      /* Try 3 times to get the pending lock.  This is needed to work
      ** around problems caused by indexing and/or anti-virus software on
      ** Windows systems.
      **
      ** If you are using this code as a model for alternative VFSes, do not
      ** copy this retry logic.  It is a hack intended for Windows only.  */
      res = win_تالو_File(&pFile->h, flags, PENDING_BYTE, 0, 1, 0);
      إذا( res ) اكسر;

      آخري_Errno = os_Get_آخري_Error();
      OSTRACE(("LOCK-PENDING-FAIL file=%p, count=%d, result=%d\n", 
            pFile->h, cnt, res
      ));

      إذا( آخري_Errno==ERROR_INVALID_HANDLE ){
        pFile->آخري_Errno = آخري_Errno;
        rc = SQLITE_IOERR_تالو;
        OSTRACE(("LOCK-FAIL file=%p, count=%d, rc=%s\n", 
              pFile->h, cnt, sqlite3ErrName(rc)
        ));
        أرجع rc;
      }

      cnt--;
      إذا( cnt>0 ) osSleep(1);
    }
    got_Pending_تالو = res;
  }

  /* Acquire a shared lock
  */
  إذا( locktype==SHARED_تالو && res ){
    أكد( pFile->locktype==NO_تالو );
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SETLK_TIMEOUT
    res = win_Get_Read_تالو(pFile, pFile->b_بلاڪ_On_Connect);
#وإلا
    res = win_Get_Read_تالو(pFile, 0);
#جيڪڏهن_پورو
    إذا( res ){
      newLocktype = SHARED_تالو;
    }وإلا{
      آخري_Errno = os_Get_آخري_Error();
    }
  }

  /* Acquire a RESERVED lock
  */
  إذا( locktype==RESERVED_تالو && res ){
    أكد( pFile->locktype==SHARED_تالو );
    res = win_تالو_File(&pFile->h, SQLITE_LOCKFILE_FLAGS, RESERVED_BYTE, 0, 1, 0);
    إذا( res ){
      newLocktype = RESERVED_تالو;
    }وإلا{
      آخري_Errno = os_Get_آخري_Error();
    }
  }

  /* Acquire a PENDING lock
  */
  إذا( locktype==EXCLUSIVE_تالو && res ){
    newLocktype = PENDING_تالو;
    got_Pending_تالو = 0;
  }

  /* Acquire an EXCLUSIVE lock
  */
  إذا( locktype==EXCLUSIVE_تالو && res ){
    أكد( pFile->locktype>=SHARED_تالو );
    (فراغ)win_Unlock_Read_تالو(pFile);
    res = win_تالو_File(&pFile->h, SQLITE_LOCKFILE_FLAGS, SHARED_پهريون, 0,
                      SHARED_سائيز, 0);
    إذا( res ){
      newLocktype = EXCLUSIVE_تالو;
    }وإلا{
      آخري_Errno = os_Get_آخري_Error();
      win_Get_Read_تالو(pFile, 0);
    }
  }

  /* If we are holding a PENDING lock that ought to be released, then
  ** release it now.
  */
  إذا( got_Pending_تالو && locktype==SHARED_تالو ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }

  /* Update the state of the lock has held in the file descriptor then
  ** return the appropriate result code.
  */
  إذا( res ){
    rc = SQLITE_OK;
  }وإلا{
    pFile->آخري_Errno = آخري_Errno;
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
ساكن صحيح win_Check_Reserved_تالو(sqlite3_file *id, صحيح *p_Res_ٻاهريون){
  صحيح res;
  winFile *pFile = (winFile*)id;

  SimulateIOError( أرجع SQLITE_IOERR_CHECKRESERVEDLOCK; );
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p\n", pFile->h, p_Res_ٻاهريون));

  أكد( id!=0 );
  إذا( pFile->locktype>=RESERVED_تالو ){
    res = 1;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (local)\n", pFile->h, res));
  }وإلا{
    res = win_تالو_File(&pFile->h, SQLITE_LOCKFILEEX_FLAGS,RESERVED_BYTE,0,1,0);
    إذا( res ){
      winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
    }
    res = !res;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (remote)\n", pFile->h, res));
  }
  *p_Res_ٻاهريون = res;
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           pFile->h, p_Res_ٻاهريون, *p_Res_ٻاهريون));
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
  أكد( locktype<=SHARED_تالو );
  OSTRACE(("UNLOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->shared_تالو_Byte, locktype));
  type = pFile->locktype;
  إذا( type>=EXCLUSIVE_تالو ){
    winUnlockFile(&pFile->h, SHARED_پهريون, 0, SHARED_سائيز, 0);
    إذا( locktype==SHARED_تالو && !win_Get_Read_تالو(pFile, 0) ){
      /* This should never happen.  We should always be able to
      ** reacquire the read lock */
      rc = winLogError(SQLITE_IOERR_UNLOCK, os_Get_آخري_Error(),
                       "winUnlock", pFile->z_رستو);
    }
  }
  إذا( type>=RESERVED_تالو ){
    winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
  }
  إذا( locktype==NO_تالو && type>=SHARED_تالو ){
    win_Unlock_Read_تالو(pFile);
  }
  إذا( type>=PENDING_تالو ){
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

ساكن صحيح win_Nolock_تالو(sqlite3_file *id, صحيح locktype){
  اڻ_استعمال_ٿيل_PARAMETER(id);
  اڻ_استعمال_ٿيل_PARAMETER(locktype);
  أرجع SQLITE_OK;
}

ساكن صحيح win_Nolock_Check_Reserved_تالو(sqlite3_file *id, صحيح *p_Res_ٻاهريون){
  اڻ_استعمال_ٿيل_PARAMETER(id);
  اڻ_استعمال_ٿيل_PARAMETER(p_Res_ٻاهريون);
  أرجع SQLITE_OK;
}

ساكن صحيح winNolockUnlock(sqlite3_file *id, صحيح locktype){
  اڻ_استعمال_ٿيل_PARAMETER(id);
  اڻ_استعمال_ٿيل_PARAMETER(locktype);
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
ساكن فراغ win_گهڻائي_ٻٽو_انگ(winFile *pFile, بنانشان حرف نقاب, صحيح *pArg){
  إذا( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & نقاب)!=0;
  }وإلا إذا( (*pArg)==0 ){
    pFile->ctrlFlags &= ~نقاب;
  }وإلا{
    pFile->ctrlFlags |= نقاب;
  }
}

/* Forward references to VFS helper methods used for temporary files */
ساكن صحيح winGetTempname(sqlite3_vfs *, حرف **);
ساكن صحيح winIsDir(LPCWSTR);
ساكن BOOL win_Is_Long_رستو_اڳياڙي(ثابت حرف *);
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
    حالة SQLITE_FCNTL_آخري_ERRNO: {
      *(صحيح*)pArg = (صحيح)pFile->آخري_Errno;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      أرجع SQLITE_OK;
    }
    حالة SQLITE_FCNTL_CHUNK_سائيز: {
      pFile->szChunk = *(صحيح *)pArg;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      أرجع SQLITE_OK;
    }
    حالة SQLITE_FCNTL_سائيز_HINT: {
      إذا( pFile->szChunk>0 ){
        sqlite3_int64 oldSz;
        صحيح rc = win_File_سائيز(id, &oldSz);
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
      win_گهڻائي_ٻٽو_انگ(pFile, WINFILE_PERSIST_WAL, (صحيح*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      أرجع SQLITE_OK;
    }
    حالة SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      win_گهڻائي_ٻٽو_انگ(pFile, WINFILE_PSOW, (صحيح*)pArg);
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
#جيڪڏهن_وصف_ٿيل SQLITE_TEST
    حالة SQLITE_FCNTL_WIN32_سيٽ_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      HANDLE hOldFile = pFile->h;
      pFile->h = *phFile;
      *phFile = hOldFile;
      OSTRACE(("FCNTL oldFile=%p, newFile=%p, rc=SQLITE_OK\n",
               hOldFile, pFile->h));
      أرجع SQLITE_OK;
    }
#جيڪڏهن_پورو
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
#إذا SQLITE_MAX_MMAP_سائيز>0
    حالة SQLITE_FCNTL_MMAP_سائيز: {
      i64 new_حد_بندي = *(i64*)pArg;
      صحيح rc = SQLITE_OK;
      إذا( new_حد_بندي>sqlite3GlobalConfig.mxMmap ){
        new_حد_بندي = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (SIZE_T) and passed
      ** to MapViewOfFile(). Restrict its value to 2GB if (SIZE_T) is not at
      ** least a 64-bit type. */
      إذا( new_حد_بندي>0 && حجم(سائيز_T)<8 ){
        new_حد_بندي = (new_حد_بندي & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmap_سائيز_Max;
      إذا( new_حد_بندي>=0 && new_حد_بندي!=pFile->mmap_سائيز_Max && pFile->n_Fetch_ٻاهريون==0 ){
        pFile->mmap_سائيز_Max = new_حد_بندي;
        إذا( pFile->mmap_سائيز>0 ){
          winUnmapfile(pFile);
          rc = winMapfile(pFile, -1);
        }
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      أرجع rc;
    }
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SETLK_TIMEOUT
    حالة SQLITE_FCNTL_تالو_TIMEOUT: {
      صحيح iOld = pFile->iBusyTimeout;
      صحيح iNew = *(صحيح*)pArg;
#إذا SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = (iNew < 0) ? INFINITE : (DWORD)iNew;
#وإلاإذا SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = (DWORD)(!!iNew);
#وإلا
# error "SQLITE_ENABLE_SETLK_TIMEOUT must be set to 1 or 2"
#جيڪڏهن_پورو
      *(صحيح*)pArg = iOld;
      أرجع SQLITE_OK;
    }
    حالة SQLITE_FCNTL_بلاڪ_ON_CONNECT: {
      صحيح iNew = *(صحيح*)pArg;
      pFile->b_بلاڪ_On_Connect = iNew;
      أرجع SQLITE_OK;
    }
#جيڪڏهن_پورو /* SQLITE_ENABLE_SETLK_TIMEOUT */

#إذا defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    حالة SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      sqlite3_str_appendf(pStr, "{\"h\":%llu", (sqlite3_uint64)pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      إذا( pFile->locktype ){
        ساكن ثابت حرف *az_تالو[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
        sqlite3_str_appendf(pStr, ",\"locktype\":\"%s\"", 
                                  az_تالو[pFile->locktype-1]);
      }
#إذا SQLITE_MAX_MMAP_سائيز>0
      إذا( pFile->mmap_سائيز ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmap_سائيز);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->n_Fetch_ٻاهريون);
      }
#جيڪڏهن_پورو
      sqlite3_str_append(pStr, "}", 1);
      أرجع SQLITE_OK;
    }
#جيڪڏهن_پورو /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

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
ساكن صحيح win_Sector_سائيز(sqlite3_file *id){
  (فراغ)id;
  أرجع SQLITE_DEFAULT_SECTOR_سائيز;
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
#جيڪڏهن_وصف_ٿيل __CYGWIN__
  صحيح nChar;

  إذا( os_Cygwin_conv_رستو && !(winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2])) ){
    i64 nByte;
    صحيح convertflag = CCP_POSIX_منزل_WIN_W;
    إذا( !strchr(zFilename, '/') ) convertflag |= CCP_RELATIVE;
    nByte = (i64)os_Cygwin_conv_رستو(convertflag,
        zFilename, 0, 0);
    إذا( nByte>0 ){
      zConverted = sqlite3MallocZero(12+(u64)nByte);
      إذا ( zConverted==0 ){
        أرجع zConverted;
      }
      /* Filenames should be prefixed, except when converted
       * full path already starts with "\\?\". */
      إذا( os_Cygwin_conv_رستو(convertflag, zFilename,
                           zConverted+4, nByte)==0 ){
        إذا( (convertflag&CCP_RELATIVE) ){
          memmove(zConverted, zConverted+4, nByte);
        }وإلا إذا( memcmp(zConverted+4, L"\\\\", 4) ){
          يادگيري_نقل(zConverted, L"\\\\?\\", 8);
        }وإلا إذا( zConverted[6]!='?' ){
          memmove(zConverted+6, zConverted+4, nByte);
          يادگيري_نقل(zConverted, L"\\\\?\\UNC", 14);
        }وإلا{
          memmove(zConverted, zConverted+4, nByte);
        }
        أرجع zConverted;
      }
      sqlite3_free(zConverted);
    }
  }
  nChar = os_Multi_Byte_منزل_Wide_Char(CP_UTF8, 0, zFilename, -1, عدم, 0);
  إذا( nChar==0 ){
    أرجع 0;
  }
  zConverted = sqlite3MallocZero( ((u64)nChar+6)*حجم(WCHAR) );
  إذا( zConverted==0 ){
    أرجع 0;
  }
  nChar = os_Multi_Byte_منزل_Wide_Char(CP_UTF8, 0, zFilename, -1,
                                zConverted, nChar);
  إذا( nChar==0 ){
    sqlite3_free(zConverted);
    zConverted = 0;
  }وإلا إذا( nChar>MAX_رستو
      && winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2]) ){
    memmove(zConverted+4, zConverted, nChar*حجم(WCHAR));
    zConverted[2] = '\\';
    يادگيري_نقل(zConverted, L"\\\\?\\", 8);
  }وإلا إذا( nChar>MAX_رستو
      && winIsDirSep(zFilename[0]) && winIsDirSep(zFilename[1])
      && zFilename[2] != '?' ){
    memmove(zConverted+6, zConverted, nChar*حجم(WCHAR));
    يادگيري_نقل(zConverted, L"\\\\?\\UNC", 14);
  }
#وإلا /* if !defined(__CYGWIN__) */
  zConverted = win_Utf8_منزل_Unicode(zFilename);
#جيڪڏهن_پورو /* __CYGWIN__ */
  /* caller will handle out of memory */
  أرجع zConverted;
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WAL

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
ساكن sqlite3_mutex *win_Big_تالو = 0;
ساكن فراغ winShmEnterMutex(فراغ){
  sqlite3_mutex_enter(win_Big_تالو);
}
ساكن فراغ winShmLeaveMutex(فراغ){
  sqlite3_mutex_leave(win_Big_تالو);
}
#جيڪڏهن_وصف_نه_ٿيل NDEBUG
ساكن صحيح winShmMutexHeld(فراغ) {
  أرجع sqlite3_mutex_held(win_Big_تالو);
}
#جيڪڏهن_پورو

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
بنية win_Shm_ڳنڍ {
  sqlite3_mutex *mutex;      /* Mutex to access this object */
  حرف *zFilename;           /* Name of the file */
  HANDLE hSharedShm;         /* File handle open on zFilename */
  صحيح b_Use_Shared_تالو_Handle;  /* True to use hSharedShm for everything */

  صحيح isUnlocked;            /* DMS lock has not yet been obtained */
  صحيح isReadonly;            /* True if read-only */
  صحيح szRegion;              /* Size of shared-memory regions */
  صحيح nRegion;               /* Size of array apRegion */

  بنية ShmRegion {
    HANDLE hMap;             /* File handle from CreateFileMapping */
    فراغ *pMap;
  } *aRegion;
  DWORD آخري_Errno;           /* The Windows errno from the last I/O error */

  winShm *pWinShmList;       /* List of winShm objects with ptrs to this */

  win_Shm_ڳنڍ *p_ايندڙ;         /* Next in list of all winShmNode objects */
#إذا defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 ايندڙ_Shm_Id;              /* Next available winShm.id value */
#جيڪڏهن_پورو
};

/*
** A global array of all winShmNode objects.
**
** The winShmMutexHeld() must be true while reading or writing this list.
*/
ساكن win_Shm_ڳنڍ *win_Shm_ڳنڍ_List = 0;

/*
** Structure used internally by this VFS to record the state of an
** open shared memory connection. There is one such structure for each
** winFile open on a wal mode database.
*/
بنية winShm {
  win_Shm_ڳنڍ *p_Shm_ڳنڍ;      /* The underlying winShmNode object */
  u16 shared_نقاب;            /* Mask of shared locks held */
  u16 excl_نقاب;              /* Mask of exclusive locks held */
  HANDLE hShm;               /* File-handle on *-shm file. For locking. */
  صحيح bReadonly;             /* True if hShm is opened read-only */
#إذا defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 id;                     /* Id of this connection with its winShmNode */
#جيڪڏهن_پورو
  winShm *p_Win_Shm_ايندڙ;       /* Next winShm object on same winShmNode */
};

/*
** Constants used for locking
*/
#وصف_ڪريو WIN_SHM_بنياد   ((22+SQLITE_SHM_NLOCK)*4)        /* first lock byte */
#وصف_ڪريو WIN_SHM_DMS    (WIN_SHM_بنياد+SQLITE_SHM_NLOCK)  /* deadman switch */

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
  win_Shm_ڳنڍ **pp;
  win_Shm_ڳنڍ *p;
  أكد( winShmMutexHeld() );
  OSTRACE(("SHM-PURGE pid=%lu, deleteFlag=%d\n",
           osGetCurrentProcessId(), deleteFlag));
  pp = &win_Shm_ڳنڍ_List;
  طالما( (p = *pp)!=0 ){
    إذا( p->pWinShmList==0 ){
      صحيح i;
      إذا( p->mutex ){ sqlite3_mutex_free(p->mutex); }
      لكل(i=0; i<p->nRegion; i++){
        BOOL bRc = os_Unmap_View_جو_File(p->aRegion[i].pMap);
        OSTRACE(("SHM-PURGE-UNMAP pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        اڻ_استعمال_ٿيل_VARIABLE_قدر(bRc);
        bRc = osCloseHandle(p->aRegion[i].hMap);
        OSTRACE(("SHM-PURGE-CLOSE pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        اڻ_استعمال_ٿيل_VARIABLE_قدر(bRc);
      }
      winHandleClose(p->hSharedShm);
      إذا( deleteFlag ){
        SimulateIOErrorBenign(1);
        sqlite3BeginBenignMalloc();
        winDelete(pVfs, p->zFilename, 0);
        sqlite3_پڇاڙي_Benign_Malloc();
        SimulateIOErrorBenign(0);
      }
      *pp = p->p_ايندڙ;
      sqlite3_free(p->aRegion);
      sqlite3_free(p);
    }وإلا{
      pp = &p->p_ايندڙ;
    }
  }
}

/*
** Handle h is open on the *-shm file. This function zeroes the first
** 8 bytes of the file. If SQLITE_ENABLE_SETLK_TIMEOUT is defined, h may 
** have been opened with FILE_FLAG_OVERLAPPED.
*/
ساكن صحيح win_Zero_Shared_يادگيري(HANDLE h){
  u8 aZero[8] = {0,0,0,0,0,0,0,0};
  DWORD nWrite = 0;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, حجم(OVERLAPPED));
  إذا( !osWriteFile(h, aZero, حجم(aZero), &nWrite, &ovlp) ){
    /* If the file was opened with FILE_FLAG_OVERLAPPED, the write may still
    ** be pending. Wait for it with GetOverlappedResult().  */
    إذا( os_Get_آخري_Error()!=ERROR_IO_PENDING
     || !Get_Overlapped_نتيجو(h, &ovlp, &nWrite, TRUE)
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
ساكن صحيح win_تالو_Shared_يادگيري(win_Shm_ڳنڍ *p_Shm_ڳنڍ, DWORD nMs){
  HANDLE h = p_Shm_ڳنڍ->hSharedShm;
  صحيح rc = SQLITE_OK;

  أكد( sqlite3_mutex_held(p_Shm_ڳنڍ->mutex) );
  rc = win_Handle_تالو_Timeout(h, WIN_SHM_DMS, 1, 1, 0);
  إذا( rc==SQLITE_OK ){
    /* We have an EXCLUSIVE lock on the DMS byte. This means that this
    ** is the first process to open the file. Truncate it to zero bytes
    ** in this case.  */
    إذا( p_Shm_ڳنڍ->isReadonly ){
      rc = SQLITE_READONLY_CANTINIT;
    }وإلا{
      إذا( SQLITE_OK!=winHandleTruncate(h, 0) ){
        /* Sometimes the truncate operation may fail because some other
        ** process is still holding an open mapping. So try to write
        ** zeroes into the start of the file instead. */
        rc = win_Zero_Shared_يادگيري(h);
      }
    }

    /* Release the EXCLUSIVE lock acquired above. */
    winUnlockFile(&h, WIN_SHM_DMS, 0, 1, 0);
  }وإلا إذا( (rc & 0xFF)==SQLITE_BUSY ){
    rc = SQLITE_OK;
  }

  إذا( rc==SQLITE_OK ){
    /* Take a SHARED lock on the DMS byte. */
    rc = win_Handle_تالو_Timeout(h, WIN_SHM_DMS, 1, 0, nMs);
    إذا( rc==SQLITE_OK ){
      p_Shm_ڳنڍ->isUnlocked = 0;
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
  HANDLE h = INVALID_HANDLE_قدر;

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SETLK_TIMEOUT
  ثابت DWORD flag_overlapped = FILE_FLAG_OVERLAPPED;
#وإلا
  ثابت DWORD flag_overlapped = 0;
#جيڪڏهن_پورو

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8);
  إذا( zConverted==0 ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8));
    rc = SQLITE_IOERR_NOMEM_BKPT;
    اذهب winopenfile_ٻاهريون;
  }

  /* Ensure the file we are trying to open is not actually a directory. */
  إذا( winIsDir(zConverted) ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8));
    rc = SQLITE_CANTOPEN_ISDIR;
    اذهب winopenfile_ٻاهريون;
  }

  /* TODO: platforms.
  ** TODO: retry-on-ioerr.
  */
  h = osCreateFileW(zConverted,         /* lpFileName */
      (GENERIC_READ | (bReadonly ? 0 : GENERIC_WRITE)),  /* dwDesiredAccess */
      FILE_SHARE_READ | FILE_SHARE_WRITE,        /* dwShareMode */
      عدم,                                      /* lpSecurityAttributes */
      OPEN_ALWAYS,                               /* dwCreationDisposition */
      FILE_ATTRIBUTE_عام|flag_overlapped,
      عدم
  );

  إذا( h==INVALID_HANDLE_قدر ){
    إذا( bReadonly==0 ){
      bReadonly = 1;
      rc = winHandleOpen(zUtf8, &bReadonly, &h);
    }وإلا{
      rc = SQLITE_CANTOPEN_BKPT;
    }
  }

 winopenfile_ٻاهريون:
  sqlite3_free(zConverted);
  *pbReadonly = bReadonly;
  *ph = h;
  أرجع rc;
}
 
/*
** Close pDbFd's connection to shared-memory.  Delete the underlying
** *-shm file if deleteFlag is true.
*/
ساكن صحيح win_Close_Shared_يادگيري(winFile *pDbFd, صحيح deleteFlag){
  winShm *p;            /* The connection to be closed */
  winShm **pp;          /* Iterator for pShmNode->pWinShmList */
  win_Shm_ڳنڍ *p_Shm_ڳنڍ; /* The underlying shared-memory file */

  p = pDbFd->pShm;
  إذا( p==0 ) أرجع SQLITE_OK;
  إذا( p->hShm!=INVALID_HANDLE_قدر ){
    osCloseHandle(p->hShm);
  }

  winShmEnterMutex();
  p_Shm_ڳنڍ = p->p_Shm_ڳنڍ;

  /* Remove this connection from the winShmNode.pWinShmList list */
  sqlite3_mutex_enter(p_Shm_ڳنڍ->mutex);
  لكل(pp=&p_Shm_ڳنڍ->pWinShmList; *pp!=p; pp=&(*pp)->p_Win_Shm_ايندڙ){}
  *pp = p->p_Win_Shm_ايندڙ;
  sqlite3_mutex_leave(p_Shm_ڳنڍ->mutex);

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
#جيڪڏهن_وصف_ٿيل SQLITE_TEST
صحيح sqlite3_win_test_unc_locking = 0;
#وإلا
# define sqlite3_win_test_unc_locking 0
#جيڪڏهن_پورو

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
  WCHAR z_پاڙ[4];
  إذا( لڙي_ڀيٽ(zFile, "\\\\?\\",4)==0 ) zFile += 4;
  إذا( !sqlite3Isalpha(zFile[0]) ) أرجع 0;
  إذا( zFile[1]!=':' ) أرجع 0;
  إذا( !winIsDirSep(zFile[2]) ) أرجع 0;
  z_پاڙ[0] = (WCHAR)zFile[0];
  z_پاڙ[1] = ':';
  z_پاڙ[2] = '\\';
  z_پاڙ[3] = 0;
  أرجع osGetDriveTypeW(z_پاڙ)==DRIVE_REMOTE;
}

/*
** Open the shared-memory area associated with database file pDbFd.
*/
ساكن صحيح win_Open_Shared_يادگيري(winFile *pDbFd){
  بنية winShm *p;                  /* The connection to be opened */
  win_Shm_ڳنڍ *p_Shm_ڳنڍ = 0;          /* The underlying mmapped file */
  صحيح rc = SQLITE_OK;                /* Result code */
  win_Shm_ڳنڍ *pNew;                  /* Newly allocated winShmNode */
  صحيح nName;                         /* Size of zName in bytes */

  أكد( pDbFd->pShm==0 );    /* Not previously opened */

  /* Allocate space for the new sqlite3_shm object.  Also speculatively
  ** allocate space for a new winShmNode and filename.  */
  p = sqlite3MallocZero( حجم(*p) );
  إذا( p==0 ) أرجع SQLITE_IOERR_NOMEM_BKPT;
  nName = sqlite3Strlen30(pDbFd->z_رستو);
  pNew = sqlite3MallocZero( حجم(*p_Shm_ڳنڍ) + (i64)nName + 17 );
  إذا( pNew==0 ){
    sqlite3_free(p);
    أرجع SQLITE_IOERR_NOMEM_BKPT;
  }
  pNew->zFilename = (حرف*)&pNew[1];
  pNew->hSharedShm = INVALID_HANDLE_قدر;
  pNew->isUnlocked = 1;
  إذا( winIsUNCPath(pDbFd->z_رستو) || winIsRemoteVolume(pDbFd->z_رستو) ){
    pNew->b_Use_Shared_تالو_Handle = 1;
  }وإلا{
    pNew->b_Use_Shared_تالو_Handle = 0;
  }
  sqlite3_snprintf(nName+15, pNew->zFilename, "%s-shm", pDbFd->z_رستو);
  sqlite3FileSuffix3(pDbFd->z_رستو, pNew->zFilename);

  /* Look to see if there is an existing winShmNode that can be used.
  ** If no matching winShmNode currently exists, then create a new one.  */
  winShmEnterMutex();
  لكل(p_Shm_ڳنڍ = win_Shm_ڳنڍ_List; p_Shm_ڳنڍ; p_Shm_ڳنڍ=p_Shm_ڳنڍ->p_ايندڙ){
    /* TBD need to come up with better match here.  Perhaps
    ** use FILE_ID_BOTH_DIR_INFO Structure.  */
    إذا( sqlite3StrICmp(p_Shm_ڳنڍ->zFilename, pNew->zFilename)==0 ) اكسر;
  }
  إذا( p_Shm_ڳنڍ==0 ){
    p_Shm_ڳنڍ = pNew;

    /* Allocate a mutex for this winShmNode object, if one is required. */
    إذا( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_ڳنڍ->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      إذا( p_Shm_ڳنڍ->mutex==0 ) rc = SQLITE_IOERR_NOMEM_BKPT;
    }

    /* Open a file-handle to use for mappings, and for the DMS lock. */
    إذا( rc==SQLITE_OK ){
      HANDLE h = INVALID_HANDLE_قدر;
      p_Shm_ڳنڍ->isReadonly = sqlite3_uri_boolean(pDbFd->z_رستو,"readonly_shm",0);
      rc = winHandleOpen(pNew->zFilename, &p_Shm_ڳنڍ->isReadonly, &h);
      p_Shm_ڳنڍ->hSharedShm = h;
    }

    /* If successful, link the new winShmNode into the global list. If an 
    ** error occurred, free the object. */
    إذا( rc==SQLITE_OK ){
      p_Shm_ڳنڍ->p_ايندڙ = win_Shm_ڳنڍ_List;
      win_Shm_ڳنڍ_List = p_Shm_ڳنڍ;
      pNew = 0;
    }وإلا{
      sqlite3_mutex_free(p_Shm_ڳنڍ->mutex);
      إذا( p_Shm_ڳنڍ->hSharedShm!=INVALID_HANDLE_قدر ){
        osCloseHandle(p_Shm_ڳنڍ->hSharedShm);
      }
    }
  }

  /* If no error has occurred, link the winShm object to the winShmNode and
  ** the winShm to pDbFd.  */
  إذا( rc==SQLITE_OK ){
    sqlite3_mutex_enter(p_Shm_ڳنڍ->mutex);
    p->p_Shm_ڳنڍ = p_Shm_ڳنڍ;
    p->p_Win_Shm_ايندڙ = p_Shm_ڳنڍ->pWinShmList;
    p_Shm_ڳنڍ->pWinShmList = p;
#إذا defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
    p->id = p_Shm_ڳنڍ->ايندڙ_Shm_Id++;
#جيڪڏهن_پورو
    pDbFd->pShm = p;
    sqlite3_mutex_leave(p_Shm_ڳنڍ->mutex);
  }وإلا إذا( p ){
    sqlite3_free(p);
  }

  أكد( rc!=SQLITE_OK || p_Shm_ڳنڍ->isUnlocked==0 || p_Shm_ڳنڍ->nRegion==0 );
  winShmLeaveMutex();
  sqlite3_free(pNew);

  /* Open a file-handle on the *-shm file for this connection. This file-handle
  ** is only used for locking. The mapping of the *-shm file is created using
  ** the shared file handle in winShmNode.hSharedShm.  */
  إذا( rc==SQLITE_OK && p_Shm_ڳنڍ->b_Use_Shared_تالو_Handle==0 ){
    p->bReadonly = sqlite3_uri_boolean(pDbFd->z_رستو, "readonly_shm", 0);
    rc = winHandleOpen(p_Shm_ڳنڍ->zFilename, &p->bReadonly, &p->hShm);
    إذا( rc!=SQLITE_OK ){
      أكد( p->hShm==INVALID_HANDLE_قدر );
      win_Close_Shared_يادگيري(pDbFd, 0);
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
  أرجع win_Close_Shared_يادگيري((winFile*)fd, deleteFlag);
}

/*
** Change the lock state for a shared-memory segment.
*/
ساكن صحيح win_Shm_تالو(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  صحيح ofst,                  /* First lock to acquire or release */
  صحيح n,                     /* Number of locks to acquire or release */
  صحيح flags                  /* What to do with the lock */
){
  winFile *pDbFd = (winFile*)fd;        /* Connection holding shared memory */
  winShm *p = pDbFd->pShm;              /* The shared memory being locked */
  win_Shm_ڳنڍ *p_Shm_ڳنڍ;
  صحيح rc = SQLITE_OK;                   /* Result code */
  u16 نقاب = (u16)((1U<<(ofst+n)) - (1U<<ofst)); /* Mask of locks to [un]take */

  إذا( p==0 ) أرجع SQLITE_IOERR_SHMLOCK;
  p_Shm_ڳنڍ = p->p_Shm_ڳنڍ;
  إذا( NEVER(p_Shm_ڳنڍ==0) ) أرجع SQLITE_IOERR_SHMLOCK;

  أكد( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  أكد( n>=1 );
  أكد( flags==(SQLITE_SHM_تالو | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_تالو | SQLITE_SHM_EXCLUSIVE)
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
    u16 تالو_نقاب = (p->excl_نقاب|p->shared_نقاب);
    أكد( (flags & SQLITE_SHM_UNLOCK) || pDbFd->iBusyTimeout==0 || (
          (ofst!=2 || تالو_نقاب==0)
       && (ofst!=1 || تالو_نقاب==0 || تالو_نقاب==2)
       && (ofst!=0 || تالو_نقاب<3)
       && (ofst<3  || تالو_نقاب<(1<<ofst))
    ));
  }
#جيڪڏهن_پورو

  /* Check if there is any work to do. There are three cases:
  **
  **    a) An unlock operation where there are locks to unlock,
  **    b) An shared lock where the requested lock is not already held
  **    c) An exclusive lock where the requested lock is not already held
  **
  ** The SQLite core never requests an exclusive lock that it already holds.
  ** This is assert()ed immediately below.  */
  أكد( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_تالو) 
       || 0==(p->excl_نقاب & نقاب)
  );
  إذا( ((flags & SQLITE_SHM_UNLOCK) && ((p->excl_نقاب|p->shared_نقاب) & نقاب))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_تالو) && 0==(p->shared_نقاب & نقاب))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_تالو))
  ){
    HANDLE h = p->hShm;

    إذا( flags & SQLITE_SHM_UNLOCK ){
      /* Case (a) - unlock.  */

      أكد( (p->excl_نقاب & p->shared_نقاب)==0 );
      أكد( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->excl_نقاب & نقاب)==نقاب );
      أكد( !(flags & SQLITE_SHM_SHARED) || (p->shared_نقاب & نقاب)==نقاب );

      أكد( !(flags & SQLITE_SHM_SHARED) || n==1 );
      إذا( p_Shm_ڳنڍ->b_Use_Shared_تالو_Handle ){
        h = p_Shm_ڳنڍ->hSharedShm;
        إذا( flags & SQLITE_SHM_SHARED ){
          winShm *pShm;
          sqlite3_mutex_enter(p_Shm_ڳنڍ->mutex);
          لكل(pShm=p_Shm_ڳنڍ->pWinShmList; pShm; pShm=pShm->p_Win_Shm_ايندڙ){
            إذا( pShm!=p && (pShm->shared_نقاب & نقاب) ){
              /* Another connection within this process is also holding this
              ** SHARED lock. So do not actually release the OS lock.  */
              h = INVALID_HANDLE_قدر;
              اكسر;
            }
          }
          sqlite3_mutex_leave(p_Shm_ڳنڍ->mutex);
        }
      }

      إذا( h!=INVALID_HANDLE_قدر ){
        rc = winHandleUnlock(h, ofst+WIN_SHM_بنياد, n);
      }

      /* If successful, also clear the bits in sharedMask/exclMask */
      إذا( rc==SQLITE_OK ){
        p->excl_نقاب = (p->excl_نقاب & ~نقاب);
        p->shared_نقاب = (p->shared_نقاب & ~نقاب);
      }
    }وإلا{
      صحيح bExcl = ((flags & SQLITE_SHM_EXCLUSIVE) ? 1 : 0);
      DWORD nMs = winFileBusyTimeout(pDbFd);

      إذا( p_Shm_ڳنڍ->b_Use_Shared_تالو_Handle ){
        winShm *pShm;
        h = p_Shm_ڳنڍ->hSharedShm;
        sqlite3_mutex_enter(p_Shm_ڳنڍ->mutex);
        لكل(pShm=p_Shm_ڳنڍ->pWinShmList; pShm; pShm=pShm->p_Win_Shm_ايندڙ){
          إذا( bExcl ){
            إذا( (pShm->shared_نقاب|pShm->excl_نقاب) & نقاب ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_قدر;
            }
          }وإلا{
            إذا( pShm->shared_نقاب & نقاب ){
              h = INVALID_HANDLE_قدر;
            }وإلا إذا( pShm->excl_نقاب & نقاب ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_قدر;
            }
          }
        }
        sqlite3_mutex_leave(p_Shm_ڳنڍ->mutex);
      }

      إذا( h!=INVALID_HANDLE_قدر ){
        rc = win_Handle_تالو_Timeout(h, ofst+WIN_SHM_بنياد, n, bExcl, nMs);
      }
      إذا( rc==SQLITE_OK ){
        إذا( bExcl ){
          p->excl_نقاب = (p->excl_نقاب | نقاب);
        }وإلا{
          p->shared_نقاب = (p->shared_نقاب | نقاب);
        }
      }
    }
  }

  OSTRACE((
      "SHM-LOCK(%d,%d,%d) pid=%lu, id=%d, sharedMask=%03x, exclMask=%03x,"
      " rc=%s\n",
      ofst, n, flags, 
      osGetCurrentProcessId(), p->id, p->shared_نقاب, p->excl_نقاب,
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
  اڻ_استعمال_ٿيل_PARAMETER(fd);
  sqlite3_يادگيري_Barrier();   /* compiler-defined memory barrier */
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
  win_Shm_ڳنڍ *p_Shm_ڳنڍ;
  DWORD protect = صفحو_READWRITE;
  DWORD flags = FILE_MAP_WRITE | FILE_MAP_READ;
  صحيح rc = SQLITE_OK;

  إذا( !pShm ){
    rc = win_Open_Shared_يادگيري(pDbFd);
    إذا( rc!=SQLITE_OK ) أرجع rc;
    pShm = pDbFd->pShm;
    أكد( pShm!=0 );
  }
  p_Shm_ڳنڍ = pShm->p_Shm_ڳنڍ;

  sqlite3_mutex_enter(p_Shm_ڳنڍ->mutex);
  إذا( p_Shm_ڳنڍ->isUnlocked ){
    /* Take the DMS lock. */
    أكد( p_Shm_ڳنڍ->nRegion==0 );
    rc = win_تالو_Shared_يادگيري(p_Shm_ڳنڍ, winFileBusyTimeout(pDbFd));
    إذا( rc!=SQLITE_OK ) اذهب shmpage_ٻاهريون;
  }

  أكد( szRegion==p_Shm_ڳنڍ->szRegion || p_Shm_ڳنڍ->nRegion==0 );
  إذا( p_Shm_ڳنڍ->nRegion<=iRegion ){
    HANDLE hShared = p_Shm_ڳنڍ->hSharedShm;
    بنية ShmRegion *apNew;           /* New aRegion[] array */
    i64 nByte = ((i64)iRegion+1)*(i64)szRegion;  /* Minimum file size */
    sqlite3_int64 sz;                  /* Current size of wal-index file */

    p_Shm_ڳنڍ->szRegion = szRegion;

    /* The requested region is not mapped into this processes address space.
    ** Check to see if it has been allocated (i.e. if the wal-index file is
    ** large enough to contain the requested region).
    */
    rc = win_Handle_سائيز(hShared, &sz);
    إذا( rc!=SQLITE_OK ){
      rc = winLogError(rc, os_Get_آخري_Error(), "winShmMap1", pDbFd->z_رستو);
      اذهب shmpage_ٻاهريون;
    }

    إذا( sz<nByte ){
      /* The requested memory region does not exist. If isWrite is set to
      ** zero, exit early. *pp will be set to NULL and SQLITE_OK returned.
      **
      ** Alternatively, if isWrite is non-zero, use ftruncate() to allocate
      ** the requested memory region.  */
      إذا( !isWrite ) اذهب shmpage_ٻاهريون;
      rc = winHandleTruncate(hShared, nByte);
      إذا( rc!=SQLITE_OK ){
        rc = winLogError(rc, os_Get_آخري_Error(), "winShmMap2", pDbFd->z_رستو);
        اذهب shmpage_ٻاهريون;
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (بنية ShmRegion*)sqlite3_realloc64(
        p_Shm_ڳنڍ->aRegion, ((i64)iRegion+1)*حجم(apNew[0])
    );
    إذا( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      اذهب shmpage_ٻاهريون;
    }
    p_Shm_ڳنڍ->aRegion = apNew;

    إذا( p_Shm_ڳنڍ->isReadonly ){
      protect = صفحو_READONLY;
      flags = FILE_MAP_READ;
    }

    طالما( p_Shm_ڳنڍ->nRegion<=iRegion ){
      HANDLE hMap = عدم;         /* file-mapping handle */
      فراغ *pMap = 0;             /* Mapped memory region */

#جيڪڏهن_وصف_ٿيل SQLITE_UWP
      hMap = osCreateFileMappingFromApp(hShared, عدم, protect, nByte, عدم);
#وإلا
      hMap = osCreateFileMappingW(hShared, عدم, protect, 0, nByte, عدم);
#جيڪڏهن_پورو
      OSTRACE(("SHM-MAP-CREATE pid=%lu, region=%d, size=%lld, rc=%s\n",
               osGetCurrentProcessId(), p_Shm_ڳنڍ->nRegion, nByte,
               hMap ? "ok" : "failed"));
      إذا( hMap ){
        i64 i_وٿي = p_Shm_ڳنڍ->nRegion*szRegion;
        صحيح i_وٿي_سرڪايو = i_وٿي % winSysInfo.dw_ورهاست_Granularity;
#جيڪڏهن_وصف_ٿيل SQLITE_UWP
        pMap = os_Map_View_جو_File_From_App(hMap, flags,
            i_وٿي - i_وٿي_سرڪايو, (i64)szRegion + i_وٿي_سرڪايو
        );
#وإلا
        pMap = os_Map_View_جو_File(hMap, flags,
            0, i_وٿي - i_وٿي_سرڪايو, (i64)szRegion + i_وٿي_سرڪايو
        );
#جيڪڏهن_پورو
        OSTRACE(("SHM-MAP-MAP pid=%lu, region=%d, offset=%d, size=%d, rc=%s\n",
                 osGetCurrentProcessId(), p_Shm_ڳنڍ->nRegion, i_وٿي,
                 szRegion, pMap ? "ok" : "failed"));
      }
      إذا( !pMap ){
        p_Shm_ڳنڍ->آخري_Errno = os_Get_آخري_Error();
        rc = winLogError(SQLITE_IOERR_SHMMAP, p_Shm_ڳنڍ->آخري_Errno,
                         "winShmMap3", pDbFd->z_رستو);
        إذا( hMap ) osCloseHandle(hMap);
        اذهب shmpage_ٻاهريون;
      }

      p_Shm_ڳنڍ->aRegion[p_Shm_ڳنڍ->nRegion].pMap = pMap;
      p_Shm_ڳنڍ->aRegion[p_Shm_ڳنڍ->nRegion].hMap = hMap;
      p_Shm_ڳنڍ->nRegion++;
    }
  }

shmpage_ٻاهريون:
  إذا( p_Shm_ڳنڍ->nRegion>iRegion ){
    i64 i_وٿي = (i64)iRegion*(i64)szRegion;
    صحيح i_وٿي_سرڪايو = i_وٿي % winSysInfo.dw_ورهاست_Granularity;
    حرف *p = (حرف *)p_Shm_ڳنڍ->aRegion[iRegion].pMap;
    *pp = (فراغ *)&p[i_وٿي_سرڪايو];
  }وإلا{
    *pp = 0;
  }
  إذا( p_Shm_ڳنڍ->isReadonly && rc==SQLITE_OK ){
    rc = SQLITE_READONLY;
  }
  sqlite3_mutex_leave(p_Shm_ڳنڍ->mutex);
  أرجع rc;
}

#وإلا
# define winShmMap     0
# define win_Shm_تالو    0
# define winShmBarrier 0
# define winShmUnmap   0
#جيڪڏهن_پورو /* #ifndef SQLITE_OMIT_WAL */

/*
** Cleans up the mapped region of the specified file, if any.
*/
#إذا SQLITE_MAX_MMAP_سائيز>0
ساكن صحيح winUnmapfile(winFile *pFile){
  أكد( pFile!=0 );
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, pMapRegion=%p, "
           "mmapSize=%lld, mmapSizeMax=%lld\n",
           osGetCurrentProcessId(), pFile, pFile->hMap, pFile->pMapRegion,
           pFile->mmap_سائيز, pFile->mmap_سائيز_Max));
  إذا( pFile->pMapRegion ){
    إذا( !os_Unmap_View_جو_File(pFile->pMapRegion) ){
      pFile->آخري_Errno = os_Get_آخري_Error();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(), pFile,
               pFile->pMapRegion));
      أرجع winLogError(SQLITE_IOERR_MMAP, pFile->آخري_Errno,
                         "winUnmapfile1", pFile->z_رستو);
    }
    pFile->pMapRegion = 0;
    pFile->mmap_سائيز = 0;
  }
  إذا( pFile->hMap!=عدم ){
    إذا( !osCloseHandle(pFile->hMap) ){
      pFile->آخري_Errno = os_Get_آخري_Error();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, rc=SQLITE_IOERR_MMAP\n",
               osGetCurrentProcessId(), pFile, pFile->hMap));
      أرجع winLogError(SQLITE_IOERR_MMAP, pFile->آخري_Errno,
                         "winUnmapfile2", pFile->z_رستو);
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

  أكد( nMap>=0 || pFd->n_Fetch_ٻاهريون==0 );
  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, size=%lld\n",
           osGetCurrentProcessId(), pFd, nByte));

  إذا( pFd->n_Fetch_ٻاهريون>0 ) أرجع SQLITE_OK;

  إذا( nMap<0 ){
    rc = win_File_سائيز((sqlite3_file*)pFd, &nMap);
    إذا( rc ){
      OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_IOERR_FSTAT\n",
               osGetCurrentProcessId(), pFd));
      أرجع SQLITE_IOERR_FSTAT;
    }
  }
  إذا( nMap>pFd->mmap_سائيز_Max ){
    nMap = pFd->mmap_سائيز_Max;
  }
  nMap &= ~(sqlite3_int64)(winSysInfo.dw_صفحو_سائيز - 1);

  إذا( nMap==0 && pFd->mmap_سائيز>0 ){
    winUnmapfile(pFd);
  }
  إذا( nMap!=pFd->mmap_سائيز ){
    فراغ *pNew = 0;
    DWORD protect = صفحو_READONLY;
    DWORD flags = FILE_MAP_READ;

    winUnmapfile(pFd);
#جيڪڏهن_وصف_ٿيل SQLITE_MMAP_READWRITE
    إذا( (pFd->ctrlFlags & WINFILE_RDONLY)==0 ){
      protect = صفحو_READWRITE;
      flags |= FILE_MAP_WRITE;
    }
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل SQLITE_UWP
    pFd->hMap = osCreateFileMappingFromApp(pFd->h, عدم, protect, nMap, عدم);
#وإلا
    pFd->hMap = osCreateFileMappingW(pFd->h, عدم, protect,
                                (DWORD)((nMap>>32) & 0xffffffff),
                                (DWORD)(nMap & 0xffffffff), عدم);
#جيڪڏهن_پورو
    إذا( pFd->hMap==عدم ){
      pFd->آخري_Errno = os_Get_آخري_Error();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->آخري_Errno,
                       "winMapfile1", pFd->z_رستو);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-CREATE pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      أرجع SQLITE_OK;
    }
    أكد( (nMap % winSysInfo.dw_صفحو_سائيز)==0 );
    أكد( حجم(سائيز_T)==حجم(sqlite3_int64) || nMap<=0xffffffff );
#جيڪڏهن_وصف_ٿيل SQLITE_UWP
    pNew = os_Map_View_جو_File_From_App(pFd->hMap, flags, 0, (سائيز_T)nMap);
#وإلا
    pNew = os_Map_View_جو_File(pFd->hMap, flags, 0, 0, (سائيز_T)nMap);
#جيڪڏهن_پورو
    إذا( pNew==عدم ){
      osCloseHandle(pFd->hMap);
      pFd->hMap = عدم;
      pFd->آخري_Errno = os_Get_آخري_Error();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->آخري_Errno,
                       "winMapfile2", pFd->z_رستو);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-MAP pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      أرجع SQLITE_OK;
    }
    pFd->pMapRegion = pNew;
    pFd->mmap_سائيز = nMap;
  }

  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFd));
  أرجع SQLITE_OK;
}
#جيڪڏهن_پورو /* SQLITE_MAX_MMAP_SIZE>0 */

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
#إذا SQLITE_MAX_MMAP_سائيز>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */
#جيڪڏهن_پورو
  *pp = 0;

  OSTRACE(("FETCH pid=%lu, pFile=%p, offset=%lld, amount=%d, pp=%p\n",
           osGetCurrentProcessId(), fd, iOff, nAmt, pp));

#إذا SQLITE_MAX_MMAP_سائيز>0
  إذا( pFd->mmap_سائيز_Max>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    ثابت صحيح n_Eof_بفر = 256;
    إذا( pFd->pMapRegion==0 ){
      صحيح rc = winMapfile(pFd, -1);
      إذا( rc!=SQLITE_OK ){
        OSTRACE(("FETCH pid=%lu, pFile=%p, rc=%s\n",
                 osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
        أرجع rc;
      }
    }
    إذا( pFd->mmap_سائيز >= (iOff+nAmt+n_Eof_بفر) ){
      أكد( pFd->pMapRegion!=0 );
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->n_Fetch_ٻاهريون++;
    }
  }
#جيڪڏهن_پورو

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
#إذا SQLITE_MAX_MMAP_سائيز>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  أكد( (p==0)==(pFd->n_Fetch_ٻاهريون==0) );

  /* If p!=0, it must match the iOff value. */
  أكد( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, offset=%lld, p=%p\n",
           osGetCurrentProcessId(), pFd, iOff, p));

  إذا( p ){
    pFd->n_Fetch_ٻاهريون--;
  }وإلا{
    /* FIXME:  If Windows truly always prevents truncating or deleting a
    ** file while a mapping is held, then the following winUnmapfile() call
    ** is unnecessary can be omitted - potentially improving
    ** performance.  */
    winUnmapfile(pFd);
  }

  أكد( pFd->n_Fetch_ٻاهريون>=0 );
#جيڪڏهن_پورو

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
ساكن ثابت sqlite3_io_methods win_Io_طريقو = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_سائيز,                    /* xFileSize */
  win_تالو,                        /* xLock */
  winUnlock,                      /* xUnlock */
  win_Check_Reserved_تالو,           /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_سائيز,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  win_Shm_تالو,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

/*
** This vector defines all the methods that can operate on an
** sqlite3_file for win32 without performing any locking.
*/
ساكن ثابت sqlite3_io_methods win_Io_Nolock_طريقو = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_سائيز,                    /* xFileSize */
  win_Nolock_تالو,                  /* xLock */
  winNolockUnlock,                /* xUnlock */
  win_Nolock_Check_Reserved_تالو,     /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_سائيز,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  win_Shm_تالو,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

ساكن win_Vfs_App_معلومات win_App_معلومات = {
  &win_Io_طريقو,       /* pMethod */
  0,                  /* pAppData */
  0                   /* bNoLock */
};

ساكن win_Vfs_App_معلومات win_Nolock_App_معلومات = {
  &win_Io_Nolock_طريقو, /* pMethod */
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
ساكن صحيح win_Make_پڇاڙي_اندريون_Dir_Sep(صحيح nBuf, حرف *zBuf){
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
ساكن صحيح win_عارضي_Dir_Defined(فراغ){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  إذا( sqlite3_عارضي_directory!=0 ) أرجع 1;
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
  صحيح nPre = sqlite3Strlen30(SQLITE_عارضي_FILE_اڳياڙي);
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
  إذا( win_عارضي_Dir_Defined() ){
    صحيح nDirLen = sqlite3Strlen30(sqlite3_عارضي_directory);
    إذا( nDirLen>0 ){
      إذا( !winIsDirSep(sqlite3_عارضي_directory[nDirLen-1]) ){
        nDirLen++;
      }
      إذا( nDirLen>nDir ){
        sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
        sqlite3_free(zBuf);
        OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
        أرجع winLogError(SQLITE_ERROR, 0, "winGetTempname1", 0);
      }
      sqlite3_snprintf(nMax, zBuf, "%s", sqlite3_عارضي_directory);
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
    بنانشان صحيح i;
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
#جيڪڏهن_پورو

  وإلا{
    حرف *zMulti;
    LPWSTR z_Wide_رستو = sqlite3MallocZero( nMax*حجم(WCHAR) );
    إذا( !z_Wide_رستو ){
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      أرجع SQLITE_IOERR_NOMEM_BKPT;
    }
    إذا( os_Get_عارضي_رستو_W==0 || os_Get_عارضي_رستو_W(nMax, z_Wide_رستو)==0 ){
      sqlite3_free(z_Wide_رستو);
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_GETTEMPPATH\n"));
      أرجع winLogError(SQLITE_IOERR_GETTEMPPATH, os_Get_آخري_Error(),
                         "winGetTempname2", 0);
    }
    zMulti = win_Unicode_منزل_Utf8(z_Wide_رستو);
    sqlite3_free(z_Wide_رستو);
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
  إذا( !win_Make_پڇاڙي_اندريون_Dir_Sep(nDir+1, zBuf) ){
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

  sqlite3_snprintf(nBuf-16-nLen, zBuf+nLen, SQLITE_عارضي_FILE_اڳياڙي);

  j = sqlite3Strlen30(zBuf);
  sqlite3_randomness(15, &zBuf[j]);
  pid = osGetCurrentProcessId();
  لكل(i=0; i<15; i++, j++){
    zBuf[j] += pid & 0xff;
    pid >>= 8;
    zBuf[j] = (حرف)zChars[ ((بنانشان حرف)zBuf[j])%(حجم(zChars)-1) ];
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
  DWORD آخري_Errno;
  صحيح cnt = 0;
  WIN32_FILE_ATTRIBUTE_معلومات s_Attr_معلومات;
  memset(&s_Attr_معلومات, 0, حجم(s_Attr_معلومات));
  طالما( !(rc = os_Get_File_خاصيتون_Ex_W(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_معلومات)) && winRetryIoerr(&cnt, &آخري_Errno) ){}
  إذا( !rc ){
    أرجع 0; /* Invalid name? */
  }
  attr = s_Attr_معلومات.dw_File_خاصيتون;
  أرجع (attr!=INVALID_FILE_خاصيتون) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

/* forward reference */
ساكن صحيح winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ثابت حرف *zFilename,     /* Name of file to check */
  صحيح flags,                 /* Type of test to make on this file */
  صحيح *p_Res_ٻاهريون               /* OUT: Result */
);

/*
** The Windows version of xAccess() accepts an extra bit in the flags
** parameter that prevents an anti-virus retry loop.
*/
#وصف_ڪريو NORETRY 0x4000

/*
** Open a file.
*/
ساكن صحيح winOpen(
  sqlite3_vfs *pVfs,        /* Used to get maximum path length and AppData */
  ثابت حرف *zName,        /* Name of the file (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  صحيح flags,                /* Open mode flags */
  صحيح *p_ٻاهريون_Flags            /* Status return flags */
){
  HANDLE h;
  DWORD آخري_Errno = 0;
  DWORD dwDesiredAccess;
  DWORD dw_Share_گهڻائي;
  DWORD dwCreationDisposition;
  DWORD dw_Flags_And_خاصيتون = 0;
  win_Vfs_App_معلومات *p_App_معلومات;
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
#جيڪڏهن_پورو

  صحيح isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  صحيح isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  صحيح isCreate     = (flags & SQLITE_OPEN_CREATE);
  صحيح isReadonly   = (flags & SQLITE_OPEN_READONLY);
  صحيح isReadWrite  = (flags & SQLITE_OPEN_READWRITE);

#جيڪڏهن_وصف_نه_ٿيل NDEBUG
  صحيح isOpenJournal = (isCreate && (
        eType==SQLITE_OPEN_SUPER_JOURNAL
     || eType==SQLITE_OPEN_MAIN_JOURNAL
     || eType==SQLITE_OPEN_WAL
  ));
#جيڪڏهن_پورو

  OSTRACE(("OPEN name=%s, pFile=%p, flags=%x, pOutFlags=%p\n",
           zUtf8Name, id, flags, p_ٻاهريون_Flags));

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
  أكد( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_عارضي_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_عارضي_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );

  أكد( pFile!=0 );
  memset(pFile, 0, حجم(winFile));
  pFile->h = INVALID_HANDLE_قدر;

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
    dw_Share_گهڻائي = FILE_SHARE_READ | FILE_SHARE_WRITE;
  }وإلا{
    dw_Share_گهڻائي = 0;
  }

  إذا( isDelete ){
    dw_Flags_And_خاصيتون = FILE_ATTRIBUTE_TEMPORARY
                               | FILE_ATTRIBUTE_HIDDEN
                               | FILE_FLAG_DELETE_ON_CLOSE;
  }وإلا{
    dw_Flags_And_خاصيتون = FILE_ATTRIBUTE_عام;
  }
  افعل{
    h = osCreateFileW(zConverted,
                      dwDesiredAccess,
                      dw_Share_گهڻائي, عدم,
                      dwCreationDisposition,
                      dw_Flags_And_خاصيتون,
                      عدم);
    إذا( h!=INVALID_HANDLE_قدر ) اكسر;
    إذا( isReadWrite ){
      صحيح rc2;
      sqlite3BeginBenignMalloc();
      rc2 = winAccess(pVfs, zUtf8Name, SQLITE_ACCESS_READ|NORETRY, &isRO);
      sqlite3_پڇاڙي_Benign_Malloc();
      إذا( rc2==SQLITE_OK && isRO ) اكسر;
    }
  }طالما( winRetryIoerr(&cnt, &آخري_Errno) );
  winLogIoerr(cnt, __LINE__);

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, rc=%s\n", h, zUtf8Name,
           dwDesiredAccess, (h==INVALID_HANDLE_قدر) ? "failed" : "ok"));

  إذا( h==INVALID_HANDLE_قدر ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    إذا( isReadWrite && isRO && !isExclusive ){
      أرجع winOpen(pVfs, zName, id,
         ((flags|SQLITE_OPEN_READONLY) &
                     ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE)),
         p_ٻاهريون_Flags);
    }وإلا{
      pFile->آخري_Errno = آخري_Errno;
      winLogError(SQLITE_CANTOPEN, pFile->آخري_Errno, "winOpen", zUtf8Name);
      أرجع SQLITE_CANTOPEN_BKPT;
    }
  }

  إذا( p_ٻاهريون_Flags ){
    إذا( isReadWrite ){
      *p_ٻاهريون_Flags = SQLITE_OPEN_READWRITE;
    }وإلا{
      *p_ٻاهريون_Flags = SQLITE_OPEN_READONLY;
    }
  }

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, pOutFlags=%p, *pOutFlags=%d, "
           "rc=%s\n", h, zUtf8Name, dwDesiredAccess, p_ٻاهريون_Flags, p_ٻاهريون_Flags ?
           *p_ٻاهريون_Flags : 0, (h==INVALID_HANDLE_قدر) ? "failed" : "ok"));

  p_App_معلومات = (win_Vfs_App_معلومات*)pVfs->p_App_معلومات;

  sqlite3_free(zConverted);
  sqlite3_free(zTmpname);
  id->pMethods = p_App_معلومات ? p_App_معلومات->p_طريقو : &win_Io_طريقو;
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
  pFile->آخري_Errno = NO_ERROR;
  pFile->z_رستو = zName;
#إذا SQLITE_MAX_MMAP_سائيز>0
  pFile->hMap = عدم;
  pFile->pMapRegion = 0;
  pFile->mmap_سائيز = 0;
  pFile->mmap_سائيز_Max = sqlite3GlobalConfig.szMmap;
#جيڪڏهن_پورو

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
  DWORD آخري_Errno = 0;
  LPWSTR zConverted;
  اڻ_استعمال_ٿيل_PARAMETER(pVfs);
  اڻ_استعمال_ٿيل_PARAMETER(syncDir);

  SimulateIOError(أرجع SQLITE_IOERR_DELETE);
  OSTRACE(("DELETE name=%s, syncDir=%d\n", zFilename, syncDir));

  zConverted = winConvertFromUtf8Filename(zFilename);
  إذا( zConverted==0 ){
    OSTRACE(("DELETE name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    أرجع SQLITE_IOERR_NOMEM_BKPT;
  }
  افعل {
    attr = os_Get_File_خاصيتون_W(zConverted);
    إذا ( attr==INVALID_FILE_خاصيتون ){
      آخري_Errno = os_Get_آخري_Error();
      إذا( آخري_Errno==ERROR_FILE_NOT_مليو
       || آخري_Errno==ERROR_رستو_NOT_مليو ){
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
    إذا ( !winRetryIoerr(&cnt, &آخري_Errno) ){
      rc = SQLITE_ERROR; /* No more retries. */
      اكسر;
    }
  } طالما(1);
  إذا( rc && rc!=SQLITE_IOERR_DELETE_NOENT ){
    rc = winLogError(SQLITE_IOERR_DELETE, آخري_Errno, "winDelete", zFilename);
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
  صحيح *p_Res_ٻاهريون               /* OUT: Result */
){
  DWORD attr;
  صحيح rc = 0;
  صحيح cnt = 0;
  DWORD آخري_Errno = 0;
  LPWSTR zConverted;
  صحيح noRetry = 0;           /* Do not use winRetryIoerr() */
  WIN32_FILE_ATTRIBUTE_معلومات s_Attr_معلومات;
  اڻ_استعمال_ٿيل_PARAMETER(pVfs);

  إذا( (flags & NORETRY)!=0 ){
    noRetry = 1;
    flags &= ~NORETRY;
  }  

  SimulateIOError( أرجع SQLITE_IOERR_ACCESS; );
  OSTRACE(("ACCESS name=%s, flags=%x, pResOut=%p\n",
           zFilename, flags, p_Res_ٻاهريون));

  إذا( zFilename==0 ){
    *p_Res_ٻاهريون = 0;
    OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
             zFilename, p_Res_ٻاهريون, *p_Res_ٻاهريون));
    أرجع SQLITE_OK;
  }

  zConverted = winConvertFromUtf8Filename(zFilename);
  إذا( zConverted==0 ){
    OSTRACE(("ACCESS name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    أرجع SQLITE_IOERR_NOMEM_BKPT;
  }
  memset(&s_Attr_معلومات, 0, حجم(s_Attr_معلومات));
  طالما( !(rc = os_Get_File_خاصيتون_Ex_W(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_معلومات))
     && !noRetry
     && winRetryIoerr(&cnt, &آخري_Errno)
  ){ /* Loop until true */}
  إذا( rc ){
    /* For an SQLITE_ACCESS_EXISTS query, treat a zero-length file
    ** as if it does not exist.
    */
    إذا(    flags==SQLITE_ACCESS_EXISTS
        && s_Attr_معلومات.n_File_سائيز_مٿاهون==0
        && s_Attr_معلومات.n_File_سائيز_هيٺيون==0 ){
      attr = INVALID_FILE_خاصيتون;
    }وإلا{
      attr = s_Attr_معلومات.dw_File_خاصيتون;
    }
  }وإلا{
    إذا( noRetry ) آخري_Errno = os_Get_آخري_Error();
    winLogIoerr(cnt, __LINE__);
    إذا( آخري_Errno!=ERROR_FILE_NOT_مليو && آخري_Errno!=ERROR_رستو_NOT_مليو ){
      sqlite3_free(zConverted);
      أرجع winLogError(SQLITE_IOERR_ACCESS, آخري_Errno, "winAccess",
                         zFilename);
    }وإلا{
      attr = INVALID_FILE_خاصيتون;
    }
  }
  sqlite3_free(zConverted);
  بدل( flags ){
    حالة SQLITE_ACCESS_READ:
    حالة SQLITE_ACCESS_EXISTS:
      rc = attr!=INVALID_FILE_خاصيتون;
      اكسر;
    حالة SQLITE_ACCESS_READWRITE:
      rc = attr!=INVALID_FILE_خاصيتون &&
             (attr & FILE_ATTRIBUTE_READONLY)==0;
      اكسر;
    افتراضي:
      أكد(!"Invalid flags argument");
  }
  *p_Res_ٻاهريون = rc;
  OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           zFilename, p_Res_ٻاهريون, *p_Res_ٻاهريون));
  أرجع SQLITE_OK;
}

/*
** Returns non-zero if the specified path name starts with the "long path"
** prefix.
*/
ساكن BOOL win_Is_Long_رستو_اڳياڙي(
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

#جيڪڏهن_وصف_ٿيل _WIN32
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
#جيڪڏهن_پورو /* _WIN32 */

#جيڪڏهن_وصف_ٿيل __CYGWIN__
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
#جيڪڏهن_پورو
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

#وصف_ڪريو SQLITE_MAX_SYMLINKS 100

ساكن صحيح mk_ڀريل_Pathname(
  ثابت حرف *z_رستو,              /* Input path */
  حرف *z_ٻاهريون,                     /* Output buffer */
  صحيح n_ٻاهريون                        /* Allocated size of buffer zOut */
){
  صحيح n_رستو = sqlite3Strlen30(z_رستو);
  صحيح iOff = 0;
  إذا( z_رستو[0]!='/' ){
    إذا( osGetcwd(z_ٻاهريون, n_ٻاهريون-2)==0 ){
      أرجع winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno, "getcwd", z_رستو);
    }
    iOff = sqlite3Strlen30(z_ٻاهريون);
    z_ٻاهريون[iOff++] = '/';
  }
  إذا( (iOff+n_رستو+1)>n_ٻاهريون ){
    /* SQLite assumes that xFullPathname() nul-terminates the output buffer
    ** even if it returns an error.  */
    z_ٻاهريون[iOff] = '\0';
    أرجع SQLITE_CANTOPEN_BKPT;
  }
  sqlite3_snprintf(n_ٻاهريون-iOff, &z_ٻاهريون[iOff], "%s", z_رستو);
  أرجع SQLITE_OK;
}
#جيڪڏهن_پورو /* __CYGWIN__ */

/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
ساكن صحيح win_ڀريل_Pathname_No_Mutex(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ثابت حرف *zRelative,        /* Possibly relative input path */
  صحيح n_ڀريل,                    /* Size of output buffer in bytes */
  حرف *z_ڀريل                   /* Output buffer */
){
  صحيح nByte;
  LPWSTR zConverted;
  حرف *z_ٻاهريون;

  /* If this path name begins with "/X:" or "\\?\", where "X" is any
  ** alphabetic character, discard the initial "/" from the pathname.
  */
  إذا( zRelative[0]=='/' && (winIsDriveLetterAndColon(zRelative+1)
       || win_Is_Long_رستو_اڳياڙي(zRelative+1)) ){
    zRelative++;
  }

  SimulateIOError( أرجع SQLITE_ERROR );

#جيڪڏهن_وصف_ٿيل __CYGWIN__
  إذا( osGetcwd ){
    z_ڀريل[n_ڀريل-1] = '\0';
    إذا( !winIsDriveLetterAndColon(zRelative) || !winIsDirSep(zRelative[2]) ){
      صحيح rc = SQLITE_OK;
      صحيح nLink = 1;              /* Number of symbolic links followed so far */
      ثابت حرف *z_اندريون = zRelative; /* Input path for each iteration of loop */
      حرف *zDel = 0;
      بنية stat buf;

      اڻ_استعمال_ٿيل_PARAMETER(pVfs);

      افعل {
        /* Call lstat() on path zIn. Set bLink to true if the path
        ** is a symbolic link, or false otherwise.  */
        صحيح bLink = 0;
        إذا( osLstat && osReadlink ) {
          إذا( osLstat(z_اندريون, &buf)!=0 ){
            صحيح myErrno = osErrno;
            إذا( myErrno!=ENOENT ){
              rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)myErrno,
                               "lstat", z_اندريون);
            }
          }وإلا{
            bLink = ((buf.st_گهڻائي & 0170000) == 0120000);
          }

          إذا( bLink ){
            إذا( zDel==0 ){
              zDel = sqlite3MallocZero(n_ڀريل);
              إذا( zDel==0 ) rc = SQLITE_NOMEM;
            }وإلا إذا( ++nLink>SQLITE_MAX_SYMLINKS ){
              rc = SQLITE_CANTOPEN_BKPT;
            }

            إذا( rc==SQLITE_OK ){
              nByte = osReadlink(z_اندريون, zDel, n_ڀريل-1);
              إذا( nByte ==(DWORD)-1 ){
                rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno,
                                 "readlink", z_اندريون);
              }وإلا{
                إذا( zDel[0]!='/' ){
                  صحيح n;
                  لكل(n = sqlite3Strlen30(z_اندريون); n>0 && z_اندريون[n-1]!='/'; n--);
                  إذا( nByte+n+1>n_ڀريل ){
                    rc = SQLITE_CANTOPEN_BKPT;
                  }وإلا{
                    memmove(&zDel[n], zDel, nByte+1);
                    يادگيري_نقل(zDel, z_اندريون, n);
                    nByte += n;
                  }
                }
                zDel[nByte] = '\0';
              }
            }

            z_اندريون = zDel;
          }
        }

        أكد( rc!=SQLITE_OK || z_اندريون!=z_ڀريل || z_اندريون[0]=='/' );
        إذا( rc==SQLITE_OK && z_اندريون!=z_ڀريل ){
          rc = mk_ڀريل_Pathname(z_اندريون, z_ڀريل, n_ڀريل);
        }
        إذا( bLink==0 ) اكسر;
        z_اندريون = z_ڀريل;
      }طالما( rc==SQLITE_OK );

      sqlite3_free(zDel);
      winSimplifyName(z_ڀريل);
      أرجع rc;
    }
  }
#جيڪڏهن_پورو /* __CYGWIN__ */

#إذا defined(_WIN32)
  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing. This function could fail if, for example, the
  ** current working directory has been unlinked.
  */
  SimulateIOError( أرجع SQLITE_ERROR );
  إذا ( sqlite3_معلومات_directory && !winIsVerbatimPathname(zRelative) ){
    /*
    ** NOTE: We are dealing with a relative path name and the data
    **       directory has been set.  Therefore, use it as the basis
    **       for converting the relative path name to an absolute
    **       one by prepending the data directory and a backslash.
    */
    sqlite3_snprintf(MIN(n_ڀريل, pVfs->mxPathname), z_ڀريل, "%s%c%s",
                     sqlite3_معلومات_directory, winGetDirSep(), zRelative);
    أرجع SQLITE_OK;
  }
#جيڪڏهن_پورو
  zConverted = winConvertFromUtf8Filename(zRelative);
  إذا( zConverted==0 ){
    أرجع SQLITE_IOERR_NOMEM_BKPT;
  }
  {
    LPWSTR z_عارضي;
    nByte = os_Get_ڀريل_رستو_Name_W(zConverted, 0, 0, 0);
    إذا( nByte==0 ){
      sqlite3_free(zConverted);
      أرجع winLogError(SQLITE_CANTOPEN_FULLPATH, os_Get_آخري_Error(),
                         "winFullPathname1", zRelative);
    }
    nByte += 3;
    z_عارضي = sqlite3MallocZero( (u64)nByte*حجم(WCHAR) );
    إذا( z_عارضي==0 ){
      sqlite3_free(zConverted);
      أرجع SQLITE_IOERR_NOMEM_BKPT;
    }
    nByte = os_Get_ڀريل_رستو_Name_W(zConverted, nByte, z_عارضي, 0);
    إذا( nByte==0 ){
      sqlite3_free(zConverted);
      sqlite3_free(z_عارضي);
      أرجع winLogError(SQLITE_CANTOPEN_FULLPATH, os_Get_آخري_Error(),
                         "winFullPathname2", zRelative);
    }
    sqlite3_free(zConverted);
    z_ٻاهريون = win_Unicode_منزل_Utf8(z_عارضي);
    sqlite3_free(z_عارضي);
  }
  إذا( z_ٻاهريون ){
#جيڪڏهن_وصف_ٿيل __CYGWIN__
    إذا( memcmp(z_ٻاهريون, "\\\\?\\", 4) ){
      sqlite3_snprintf(MIN(n_ڀريل, pVfs->mxPathname), z_ڀريل, "%s", z_ٻاهريون);
    }وإلا إذا( memcmp(z_ٻاهريون+4, "UNC\\", 4) ){
      sqlite3_snprintf(MIN(n_ڀريل, pVfs->mxPathname), z_ڀريل, "%s", z_ٻاهريون+4);
    }وإلا{
      حرف *p = z_ٻاهريون+6;
      *p = '\\';
      إذا( osGetcwd ){
        /* On Cygwin, UNC paths use forward slashes */
        طالما( *p ){
          إذا( *p=='\\' ) *p = '/';
          ++p;
        }
      }
      sqlite3_snprintf(MIN(n_ڀريل, pVfs->mxPathname), z_ڀريل, "%s", z_ٻاهريون+6);
    }
#وإلا
    sqlite3_snprintf(MIN(n_ڀريل, pVfs->mxPathname), z_ڀريل, "%s", z_ٻاهريون);
#جيڪڏهن_پورو /* __CYGWIN__ */
    sqlite3_free(z_ٻاهريون);
    أرجع SQLITE_OK;
  }وإلا{
    أرجع SQLITE_IOERR_NOMEM_BKPT;
  }
}
ساكن صحيح win_ڀريل_Pathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ثابت حرف *zRelative,        /* Possibly relative input path */
  صحيح n_ڀريل,                    /* Size of output buffer in bytes */
  حرف *z_ڀريل                   /* Output buffer */
){
  صحيح rc;
  MUTEX_LOGIC( sqlite3_mutex *pMutex; )
  MUTEX_LOGIC( pMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR); )
  sqlite3_mutex_enter(pMutex);
  rc = win_ڀريل_Pathname_No_Mutex(pVfs, zRelative, n_ڀريل, z_ڀريل);
  sqlite3_mutex_leave(pMutex);
  أرجع rc;
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_ڀريو_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
ساكن فراغ *winDlOpen(sqlite3_vfs *pVfs, ثابت حرف *zFilename){
  HANDLE h = 0;
  LPWSTR zConverted = winConvertFromUtf8Filename(zFilename);
  اڻ_استعمال_ٿيل_PARAMETER(pVfs);
  إذا( zConverted==0 ){
    OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (فراغ*)0));
    أرجع 0;
  }
  h = os_ڀريو_Library_W ? os_ڀريو_Library_W(zConverted) : 0;
  OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (فراغ*)h));
  sqlite3_free(zConverted);
  أرجع (فراغ*)h;
}
ساكن فراغ winDlError(sqlite3_vfs *pVfs, صحيح nBuf, حرف *z_Buf_ٻاهريون){
  اڻ_استعمال_ٿيل_PARAMETER(pVfs);
  win_Get_آخري_Error_نياپو(os_Get_آخري_Error(), nBuf, z_Buf_ٻاهريون);
}
ساكن فراغ (*winDlSym(sqlite3_vfs *pVfs,فراغ *pH,ثابت حرف *zSym))(فراغ){
  FARPROC proc = 0;
  اڻ_استعمال_ٿيل_PARAMETER(pVfs);
  proc = os_Get_Proc_پتو_A ? os_Get_Proc_پتو_A((HANDLE)pH, zSym) : 0;
  OSTRACE(("DLSYM handle=%p, symbol=%s, address=%p\n",
           (فراغ*)pH, zSym, (فراغ*)proc));
  أرجع (فراغ(*)(فراغ))proc;
}
ساكن فراغ winDlClose(sqlite3_vfs *pVfs, فراغ *pHandle){
  اڻ_استعمال_ٿيل_PARAMETER(pVfs);
  إذا( osFreeLibrary!=0 ) osFreeLibrary((HANDLE)pHandle);
  OSTRACE(("DLCLOSE handle=%p\n", (فراغ*)pHandle));
}
#وإلا /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #وصف_ڪريو winDlOpen  0
  #وصف_ڪريو winDlError 0
  #وصف_ڪريو winDlSym   0
  #وصف_ڪريو winDlClose 0
#جيڪڏهن_پورو

/* State information for the randomness gatherer. */
تعريفنوع بنية EntropyGatherer EntropyGatherer;
بنية EntropyGatherer {
  بنانشان حرف *a;   /* Gather entropy into this buffer */
  صحيح na;             /* Size of a[] in bytes */
  صحيح i;              /* XOR next input into a[i] */
  صحيح nXor;           /* Number of XOR operations done */
};

#إذا !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
/* Mix sz bytes of entropy into p. */
ساكن فراغ xor_يادگيري(EntropyGatherer *p, بنانشان حرف *x, صحيح sz){
  صحيح j, k;
  لكل(j=0, k=p->i; j<sz; j++){
    p->a[k++] ^= x[j];
    إذا( k>=p->na ) k = 0;
  }
  p->i = k;
  p->nXor += sz;
}
#جيڪڏهن_پورو /* !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS) */

/*
** Write up to nBuf bytes of randomness into zBuf.
*/
ساكن صحيح winRandomness(sqlite3_vfs *pVfs, صحيح nBuf, حرف *zBuf){
#إذا defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS)
  اڻ_استعمال_ٿيل_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  أرجع nBuf;
#وإلا
  EntropyGatherer e;
  اڻ_استعمال_ٿيل_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  e.a = (بنانشان حرف*)zBuf;
  e.na = nBuf;
  e.nXor = 0;
  e.i = 0;
  {
    FILETIME x;
    os_Get_System_وقت_As_File_وقت(&x);
    xor_يادگيري(&e, (بنانشان حرف*)&x, حجم(x));
  }
  {
    DWORD pid = osGetCurrentProcessId();
    xor_يادگيري(&e, (بنانشان حرف*)&pid, حجم(pid));
  }
  {
    ULONGLONG cnt = osGetTickCount64();
    xor_يادگيري(&e, (بنانشان حرف*)&cnt, حجم(cnt));
  }
  {
    LARGE_INTEGER i;
    osQueryPerformanceCounter(&i);
    xor_يادگيري(&e, (بنانشان حرف*)&i, حجم(i));
  }
#إذا SQLITE_WIN32_USE_UUID
#جيڪڏهن_وصف_ٿيل SQLITE_UWP
# error SQLITE_WIN32_USE_UUID is incompatible with SQLITE_UWP
#جيڪڏهن_پورو
  {
    UUID id;
    memset(&id, 0, حجم(id));
    osUuidCreate(&id);
    xor_يادگيري(&e, (بنانشان حرف*)&id, حجم(id));
    memset(&id, 0, حجم(UUID));
    osUuidCreateSequential(&id);
    xor_يادگيري(&e, (بنانشان حرف*)&id, حجم(id));
  }
#جيڪڏهن_پورو /* SQLITE_WIN32_USE_UUID */
  أرجع e.nXor>nBuf ? nBuf : e.nXor;
#جيڪڏهن_پورو /* defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS) */
}


/*
** Sleep for a little while.  Return the amount of time slept.
*/
ساكن صحيح winSleep(sqlite3_vfs *pVfs, صحيح microsec){
  osSleep((microsec+999)/1000);
  اڻ_استعمال_ٿيل_PARAMETER(pVfs);
  أرجع ((microsec+999)/1000)*1000;
}

/*
** The following variable, if set to a non-zero value, is interpreted as
** the number of seconds since 1970 and is used to set the result of
** sqlite3OsCurrentTime() during testing.
*/
#جيڪڏهن_وصف_ٿيل SQLITE_TEST
صحيح sqlite3_current_وقت = 0;  /* Fake system time in seconds since 1970. */
#جيڪڏهن_پورو

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
ساكن صحيح win_Current_وقت_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *piNow){
  /* FILETIME structure is a 64-bit value representing the number of
     100-nanosecond intervals since January 1, 1601 (= JD 2305813.5).
  */
  FILETIME ft;
  ساكن ثابت sqlite3_int64 winFiletimeEpoch = 23058135*(sqlite3_int64)8640000;
#جيڪڏهن_وصف_ٿيل SQLITE_TEST
  ساكن ثابت sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
#جيڪڏهن_پورو
  /* 2^32 - to avoid use of LL and warnings in gcc */
  ساكن ثابت sqlite3_int64 max32_ٻٽو_انگ_قدر =
      (sqlite3_int64)2000000000 + (sqlite3_int64)2000000000 +
      (sqlite3_int64)294967296;

  os_Get_System_وقت_As_File_وقت( &ft );
  *piNow = winFiletimeEpoch +
            ((((sqlite3_int64)ft.dw_مٿاهون_Date_وقت)*max32_ٻٽو_انگ_قدر) +
               (sqlite3_int64)ft.dw_هيٺيون_Date_وقت)/(sqlite3_int64)10000;

#جيڪڏهن_وصف_ٿيل SQLITE_TEST
  إذا( sqlite3_current_وقت ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_وقت + unixEpoch;
  }
#جيڪڏهن_پورو
  اڻ_استعمال_ٿيل_PARAMETER(pVfs);
  أرجع SQLITE_OK;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
ساكن صحيح win_Current_وقت(sqlite3_vfs *pVfs, مزدوج *prNow){
  صحيح rc;
  sqlite3_int64 i;
  rc = win_Current_وقت_Int64(pVfs, &i);
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
ساكن صحيح win_Get_آخري_Error(sqlite3_vfs *pVfs, صحيح nBuf, حرف *zBuf){
  DWORD e = os_Get_آخري_Error();
  اڻ_استعمال_ٿيل_PARAMETER(pVfs);
  إذا( nBuf>0 ) win_Get_آخري_Error_نياپو(e, nBuf, zBuf);
  أرجع e;
}

/*
** Initialize and deinitialize the operating system interface.
*/
صحيح sqlite3_os_init(فراغ){
  ساكن sqlite3_vfs winVfs = {
    3,                     /* iVersion */
    حجم(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_رستو_بائيٽون, /* mxPathname */
    0,                     /* pNext */
    "win32",               /* zName */
    &win_App_معلومات,           /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_ڀريل_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_وقت,        /* xCurrentTime */
    win_Get_آخري_Error,       /* xGetLastError */
    win_Current_وقت_Int64,   /* xCurrentTimeInt64 */
    win_سيٽ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_ايندڙ_System_Call,     /* xNextSystemCall */
  };
  ساكن sqlite3_vfs win_Long_رستو_Vfs = {
    3,                     /* iVersion */
    حجم(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_رستو_بائيٽون, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath",      /* zName */
    &win_App_معلومات,           /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_ڀريل_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_وقت,        /* xCurrentTime */
    win_Get_آخري_Error,       /* xGetLastError */
    win_Current_وقت_Int64,   /* xCurrentTimeInt64 */
    win_سيٽ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_ايندڙ_System_Call,     /* xNextSystemCall */
  };
  ساكن sqlite3_vfs winNolockVfs = {
    3,                     /* iVersion */
    حجم(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_رستو_بائيٽون, /* mxPathname */
    0,                     /* pNext */
    "win32-none",          /* zName */
    &win_Nolock_App_معلومات,     /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_ڀريل_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_وقت,        /* xCurrentTime */
    win_Get_آخري_Error,       /* xGetLastError */
    win_Current_وقت_Int64,   /* xCurrentTimeInt64 */
    win_سيٽ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_ايندڙ_System_Call,     /* xNextSystemCall */
  };
  ساكن sqlite3_vfs win_Long_رستو_Nolock_Vfs = {
    3,                     /* iVersion */
    حجم(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_رستو_بائيٽون, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath-none", /* zName */
    &win_Nolock_App_معلومات,     /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_ڀريل_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_وقت,        /* xCurrentTime */
    win_Get_آخري_Error,       /* xGetLastError */
    win_Current_وقت_Int64,   /* xCurrentTimeInt64 */
    win_سيٽ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_ايندڙ_System_Call,     /* xNextSystemCall */
  };

  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  أكد( Array_سائيز(aSyscall)==60 );
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
  أكد( winSysInfo.dw_ورهاست_Granularity>0 );
  أكد( winSysInfo.dw_صفحو_سائيز>0 );

  sqlite3_vfs_رجسٽر(&winVfs, 1);
  sqlite3_vfs_رجسٽر(&win_Long_رستو_Vfs, 0);
  sqlite3_vfs_رجسٽر(&winNolockVfs, 0);
  sqlite3_vfs_رجسٽر(&win_Long_رستو_Nolock_Vfs, 0);
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WAL
  win_Big_تالو = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#جيڪڏهن_پورو

  أرجع SQLITE_OK;
}

صحيح sqlite3_os_پڇاڙي(فراغ){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WAL
  win_Big_تالو = 0;
#جيڪڏهن_پورو
  أرجع SQLITE_OK;
}

#جيڪڏهن_پورو /* SQLITE_OS_WIN */
