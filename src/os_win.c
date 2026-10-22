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
#شامل "sqliteInt.h"
#اگر SQLITE_OS_WIN               /* This file is used for Windows only */

/*
** Include code that is common to all os_*.c files
*/
#شامل "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#شامل "os_win.h"

/*
** This constant should already be defined (in the "WinDef.h" SDK file).
*/
#اگر_غیر_تعریف_شدہ MAX_راستہ
#  define MAX_راستہ                      (260)
#اگر_ختم

/*
** Maximum pathname length (in chars) for Win32.  This should normally be
** MAX_PATH.
*/
#اگر_غیر_تعریف_شدہ SQLITE_WIN32_MAX_راستہ_CHARS
#  define SQLITE_WIN32_MAX_راستہ_CHARS   (MAX_راستہ)
#اگر_ختم

/*
** This constant should already be defined (in the "WinNT.h" SDK file).
*/
#اگر_غیر_تعریف_شدہ UNICODE_STRING_MAX_CHARS
#  define UNICODE_STRING_MAX_CHARS      (32767)
#اگر_ختم

/*
** Maximum pathname length (in chars) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS.
*/
#اگر_غیر_تعریف_شدہ SQLITE_WINNT_MAX_راستہ_CHARS
#  define SQLITE_WINNT_MAX_راستہ_CHARS   (UNICODE_STRING_MAX_CHARS)
#اگر_ختم

/*
** Maximum pathname length (in bytes) for Win32.  The MAX_PATH macro is in
** characters, so we allocate 4 bytes per character assuming worst-case of
** 4-bytes-per-character for UTF8.
*/
#اگر_غیر_تعریف_شدہ SQLITE_WIN32_MAX_راستہ_بائٹیں
#  define SQLITE_WIN32_MAX_راستہ_بائٹیں   (SQLITE_WIN32_MAX_راستہ_CHARS*4)
#اگر_ختم

/*
** Maximum pathname length (in bytes) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS * sizeof(WCHAR).
*/
#اگر_غیر_تعریف_شدہ SQLITE_WINNT_MAX_راستہ_بائٹیں
#  define SQLITE_WINNT_MAX_راستہ_بائٹیں   \
                            (حجم(WCHAR) * SQLITE_WINNT_MAX_راستہ_CHARS)
#اگر_ختم

/*
** Maximum error message length (in chars) for WinRT.
*/
#اگر_غیر_تعریف_شدہ SQLITE_WIN32_MAX_ERRMSG_CHARS
#  define SQLITE_WIN32_MAX_ERRMSG_CHARS (1024)
#اگر_ختم

/*
** Returns non-zero if the character should be treated as a directory
** separator.
*/
#اگر_غیر_تعریف_شدہ winIsDirSep
#  define winIsDirSep(a)                (((a) == '/') || ((a) == '\\'))
#اگر_ختم

/*
** This macro is used when a local variable is set to a value that is
** [sometimes] not used by the code (e.g. via conditional compilation).
*/
#اگر_غیر_تعریف_شدہ غیر_مستعمل_VARIABLE_قدر
#  define غیر_مستعمل_VARIABLE_قدر(x)      (خالی)(x)
#اگر_ختم

/*
** Returns the character that should be used as the directory separator.
*/
#اگر_غیر_تعریف_شدہ winGetDirSep
#  define winGetDirSep()                '\\'
#اگر_ختم

/*
** Some older Microsoft compilers lack the following definition.
*/
#اگر_غیر_تعریف_شدہ INVALID_FILE_اوصاف
# define INVALID_FILE_اوصاف ((DWORD)-1)
#اگر_ختم

#اگر_غیر_تعریف_شدہ FILE_FLAG_نقاب
# define FILE_FLAG_نقاب          (0xFF3C0000)
#اگر_ختم

#اگر_غیر_تعریف_شدہ FILE_ATTRIBUTE_نقاب
# define FILE_ATTRIBUTE_نقاب     (0x0003FFF7)
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
/* Forward references to structures used for WAL */
قسمتعریف ساخت winShm winShm;           /* A connection to shared-memory */
قسمتعریف ساخت win_Shm_گرہ win_Shm_گرہ;   /* A region of shared-memory */
#اگر_ختم

/*
** The winFile structure is a subclass of sqlite3_file* specific to the win32
** portability layer.
*/
قسمتعریف ساخت winFile winFile;
ساخت winFile {
  ثابت sqlite3_io_methods *p_طریقہ; /*** Must be first ***/
  sqlite3_vfs *pVfs;      /* The VFS used to open this file */
  HANDLE h;               /* Handle for accessing the file */
  u8 locktype;            /* Type of lock currently held on this file */
  مختصر shared_تالا_Byte;   /* Randomly chosen byte used as a shared lock */
  u8 ctrlFlags;           /* Flags.  See WINFILE_* below */
  DWORD آخری_Errno;        /* The Windows errno from the last I/O error */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
  winShm *pShm;           /* Instance of shared memory on this file */
#اگر_ختم
  ثابت حرف *z_راستہ;      /* Full pathname of this file */
  صحیح szChunk;            /* Chunk size configured by FCNTL_CHUNK_SIZE */
#اگر SQLITE_MAX_MMAP_سائز>0
  صحیح n_Fetch_برآمد;                /* Number of outstanding xFetch references */
  HANDLE hMap;                  /* Handle for accessing memory mapping */
  خالی *pMapRegion;             /* Area memory mapped */
  sqlite3_int64 mmap_سائز;       /* Size of mapped region */
  sqlite3_int64 mmap_سائز_Max;    /* Configured FCNTL_MMAP_SIZE value */
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
  DWORD iBusyTimeout;        /* Wait this many millisec on locks */
  صحیح b_بلاک_On_Connect;
#اگر_ختم
};

#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
# define winFileBusyTimeout(pDbFd) pDbFd->iBusyTimeout
#ورنہ
# define winFileBusyTimeout(pDbFd) 0
#اگر_ختم

/*
** The winVfsAppData structure is used for the pAppData member for all of the
** Win32 VFS variants.
*/
قسمتعریف ساخت win_Vfs_App_ڈیٹا win_Vfs_App_ڈیٹا;
ساخت win_Vfs_App_ڈیٹا {
  ثابت sqlite3_io_methods *p_طریقہ; /* The file I/O methods to use. */
  خالی *p_App_ڈیٹا;                    /* The extra pAppData, if any. */
  BOOL b_No_تالا;                      /* Non-zero if locking is disabled. */
};

/*
** Allowed values for winFile.ctrlFlags
*/
#تعریف_کرو WINFILE_RDONLY          0x02   /* Connection is read only */
#تعریف_کرو WINFILE_PERSIST_WAL     0x04   /* Persistent WAL mode */
#تعریف_کرو WINFILE_PSOW            0x10   /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */

/*
 * The size of the buffer used by sqlite3_win32_write_debug().
 */
#اگر_غیر_تعریف_شدہ SQLITE_WIN32_DBG_BUF_سائز
#  define SQLITE_WIN32_DBG_BUF_سائز   ((صحیح)(4096-حجم(DWORD)))
#اگر_ختم

/*
 * If compiled with SQLITE_WIN32_MALLOC on Windows, we will use the
 * various Win32 API heap functions instead of our own.
 */
#اگر_تعریف_شدہ SQLITE_WIN32_MALLOC

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
#اگر_غیر_تعریف_شدہ SQLITE_WIN32_ڈھیر_CREATE
#  define SQLITE_WIN32_ڈھیر_CREATE        (TRUE)
#اگر_ختم

/*
 * This is the maximum possible initial size of the Win32-specific heap, in
 * bytes.
 */
#اگر_غیر_تعریف_شدہ SQLITE_WIN32_ڈھیر_MAX_INIT_سائز
#  define SQLITE_WIN32_ڈھیر_MAX_INIT_سائز (4294967295U)
#اگر_ختم

/*
 * This is the extra space for the initial size of the Win32-specific heap,
 * in bytes.  This value may be zero.
 */
#اگر_غیر_تعریف_شدہ SQLITE_WIN32_ڈھیر_INIT_EXTRA
#  define SQLITE_WIN32_ڈھیر_INIT_EXTRA  (4194304)
#اگر_ختم

/*
 * Calculate the maximum legal cache size, in pages, based on the maximum
 * possible initial heap size and the default page size, setting aside the
 * needed extra space.
 */
#اگر_غیر_تعریف_شدہ SQLITE_WIN32_MAX_CACHE_سائز
#  define SQLITE_WIN32_MAX_CACHE_سائز   (((SQLITE_WIN32_ڈھیر_MAX_INIT_سائز) - \
                                          (SQLITE_WIN32_ڈھیر_INIT_EXTRA)) / \
                                         (SQLITE_DEFAULT_صفحہ_سائز))
#اگر_ختم

/*
 * This is cache size used in the calculation of the initial size of the
 * Win32-specific heap.  It cannot be negative.
 */
#اگر_غیر_تعریف_شدہ SQLITE_WIN32_CACHE_سائز
#  اگر SQLITE_DEFAULT_CACHE_سائز>=0
#    define SQLITE_WIN32_CACHE_سائز     (SQLITE_DEFAULT_CACHE_سائز)
#  ورنہ
#    define SQLITE_WIN32_CACHE_سائز     (-(SQLITE_DEFAULT_CACHE_سائز))
#  endif
#اگر_ختم

/*
 * Make sure that the calculated cache size, in pages, cannot cause the
 * initial size of the Win32-specific heap to exceed the maximum amount
 * of memory that can be specified in the call to HeapCreate.
 */
#اگر SQLITE_WIN32_CACHE_سائز>SQLITE_WIN32_MAX_CACHE_سائز
#  undef SQLITE_WIN32_CACHE_سائز
#  define SQLITE_WIN32_CACHE_سائز       (2000)
#اگر_ختم

/*
 * The initial size of the Win32-specific heap.  This value may be zero.
 */
#اگر_غیر_تعریف_شدہ SQLITE_WIN32_ڈھیر_INIT_سائز
#  define SQLITE_WIN32_ڈھیر_INIT_سائز   ((SQLITE_WIN32_CACHE_سائز) * \
                                         (SQLITE_DEFAULT_صفحہ_سائز) + \
                                         (SQLITE_WIN32_ڈھیر_INIT_EXTRA))
#اگر_ختم

/*
 * The maximum size of the Win32-specific heap.  This value may be zero.
 */
#اگر_غیر_تعریف_شدہ SQLITE_WIN32_ڈھیر_MAX_سائز
#  define SQLITE_WIN32_ڈھیر_MAX_سائز    (0)
#اگر_ختم

/*
 * The extra flags to use in calls to the Win32 heap APIs.  This value may be
 * zero for the default behavior.
 */
#اگر_غیر_تعریف_شدہ SQLITE_WIN32_ڈھیر_FLAGS
#  define SQLITE_WIN32_ڈھیر_FLAGS       (0)
#اگر_ختم


/*
** The winMemData structure stores information required by the Win32-specific
** sqlite3_mem_methods implementation.
*/
قسمتعریف ساخت win_Mem_ڈیٹا win_Mem_ڈیٹا;
ساخت win_Mem_ڈیٹا {
#اگر_غیر_تعریف_شدہ NDEBUG
  u32 magic1;   /* Magic number to detect structure corruption. */
#اگر_ختم
  HANDLE h_ڈھیر; /* The handle to our heap. */
  BOOL bOwned;  /* Do we own the heap (i.e. destroy it on shutdown)? */
#اگر_غیر_تعریف_شدہ NDEBUG
  u32 magic2;   /* Magic number to detect structure corruption. */
#اگر_ختم
};

#اگر_غیر_تعریف_شدہ NDEBUG
#تعریف_کرو WINMEM_MAGIC1     0x42b2830b
#تعریف_کرو WINMEM_MAGIC2     0xbd4d7cf4
#اگر_ختم

ساکن ساخت win_Mem_ڈیٹا win_mem_ڈیٹا = {
#اگر_غیر_تعریف_شدہ NDEBUG
  WINMEM_MAGIC1,
#اگر_ختم
  تہی, FALSE
#اگر_غیر_تعریف_شدہ NDEBUG
  ,WINMEM_MAGIC2
#اگر_ختم
};

#اگر_غیر_تعریف_شدہ NDEBUG
#تعریف_کرو winMemAssertMagic1() أكد( win_mem_ڈیٹا.magic1==WINMEM_MAGIC1 )
#تعریف_کرو winMemAssertMagic2() أكد( win_mem_ڈیٹا.magic2==WINMEM_MAGIC2 )
#تعریف_کرو winMemAssertMagic()  winMemAssertMagic1(); winMemAssertMagic2();
#ورنہ
#تعریف_کرو winMemAssertMagic()
#اگر_ختم

#تعریف_کرو win_Mem_Get_ڈیٹا_Ptr()  &win_mem_ڈیٹا
#تعریف_کرو win_Mem_Get_ڈھیر()     win_mem_ڈیٹا.h_ڈھیر
#تعریف_کرو winMemGetOwned()    win_mem_ڈیٹا.bOwned

ساکن خالی *winMemMalloc(صحیح n_بائٹیں);
ساکن خالی winMemFree(خالی *p_پیشگی);
ساکن خالی *winMemRealloc(خالی *p_پیشگی, صحیح n_بائٹیں);
ساکن صحیح win_Mem_سائز(خالی *p);
ساکن صحیح winMemRoundup(صحیح n);
ساکن صحیح winMemInit(خالی *p_App_ڈیٹا);
ساکن خالی winMemShutdown(خالی *p_App_ڈیٹا);

ثابت sqlite3_mem_methods *sqlite3MemGetWin32(خالی);
#اگر_ختم /* SQLITE_WIN32_MALLOC */

#اگر_غیر_تعریف_شدہ SYSCALL
#  define SYSCALL sqlite3_syscall_ptr
#اگر_ختم

/*
** Many system calls are accessed through pointer-to-functions so that
** they may be overridden at runtime to facilitate fault injection during
** testing and sandboxing.  The following array holds the names and pointers
** to all overrideable system calls.
*/
ساکن ساخت win_syscall {
  ثابت حرف *zName;            /* Name of the system call */
  sqlite3_syscall_ptr pCurrent; /* Current value of the system call */
  sqlite3_syscall_ptr pDefault; /* Default value */
} aSyscall[] = {
  { "AreFileApisANSI",         (SYSCALL)AreFileApisANSI,         0 },
#تعریف_کرو osAreFileApisANSI ((BOOL(WINAPI*)(VOID))aSyscall[0].pCurrent)

  { "CloseHandle",             (SYSCALL)CloseHandle,             0 },
#تعریف_کرو osCloseHandle ((BOOL(WINAPI*)(HANDLE))aSyscall[1].pCurrent)


  { "CreateFileW",             (SYSCALL)CreateFileW,             0 },
#تعریف_کرو osCreateFileW ((HANDLE(WINAPI*)(LPCWSTR,DWORD,DWORD, \
        LPSECURITY_اوصاف,DWORD,DWORD,HANDLE))aSyscall[2].pCurrent)

#اگر defined(SQLITE_UWP)
  { "CreateFileMappingFromApp",(SYSCALL)CreateFileMappingFromApp,0 },
#ورنہ
  { "CreateFileMappingFromApp",(SYSCALL)0,                       0 },
#اگر_ختم
#تعریف_کرو osCreateFileMappingFromApp ((HANDLE(WINAPI*)(HANDLE,\
        PSECURITY_اوصاف, \
        ULONG,ULONG64,PCWSTR))aSyscall[3].pCurrent)

#اگر !defined(SQLITE_UWP)
  { "CreateFileMappingW",      (SYSCALL)CreateFileMappingW,      0 },
#ورنہ
  { "CreateFileMappingW",      (SYSCALL)0,                       0 },
#اگر_ختم
#تعریف_کرو osCreateFileMappingW ((HANDLE(WINAPI*)(HANDLE,LPSECURITY_اوصاف, \
        DWORD,DWORD,DWORD,LPCWSTR))aSyscall[4].pCurrent)

  { "DeleteFileW",             (SYSCALL)DeleteFileW,             0 },
#تعریف_کرو osDeleteFileW ((BOOL(WINAPI*)(LPCWSTR))aSyscall[5].pCurrent)

  { "FlushFileBuffers",        (SYSCALL)FlushFileBuffers,        0 },
#تعریف_کرو osFlushFileBuffers ((BOOL(WINAPI*)(HANDLE))aSyscall[6].pCurrent)

  { "FormatMessageW",          (SYSCALL)Format_پیغام_W,          0 },
#تعریف_کرو os_Format_پیغام_W ((DWORD(WINAPI*)(DWORD,LPCVOID,DWORD,DWORD,LPWSTR, \
        DWORD,va_list*))aSyscall[7].pCurrent)

#اگر !defined(SQLITE_OMIT_بھرو_EXTENSION) && !defined(SQLITE_UWP)
  { "FreeLibrary",             (SYSCALL)FreeLibrary,             0 },
#ورنہ
  { "FreeLibrary",             (SYSCALL)0,                       0 },
#اگر_ختم

#تعریف_کرو osFreeLibrary ((BOOL(WINAPI*)(HMODULE))aSyscall[8].pCurrent)

  { "GetCurrentProcessId",     (SYSCALL)GetCurrentProcessId,     0 },
#تعریف_کرو osGetCurrentProcessId ((DWORD(WINAPI*)(VOID))aSyscall[9].pCurrent)

  { "GetFileAttributesW",      (SYSCALL)Get_File_اوصاف_W,      0 },
#تعریف_کرو os_Get_File_اوصاف_W ((DWORD(WINAPI*)(LPCWSTR))aSyscall[10].pCurrent)

  { "GetFileAttributesExW",    (SYSCALL)Get_File_اوصاف_Ex_W,    0 },
#تعریف_کرو os_Get_File_اوصاف_Ex_W ((BOOL(WINAPI*)(LPCWSTR,GET_FILEEX_INFO_سطحیں, \
        LPVOID))aSyscall[11].pCurrent)

  { "GetFileSizeEx",           (SYSCALL)Get_File_سائز_Ex,           0 },
#تعریف_کرو os_Get_File_سائز_Ex ((BOOL(WINAPI*)(HANDLE, \
                        PLARGE_INTEGER))aSyscall[12].pCurrent)

  { "GetFullPathNameW",        (SYSCALL)Get_بھرا_راستہ_Name_W,        0 },
#تعریف_کرو os_Get_بھرا_راستہ_Name_W ((DWORD(WINAPI*)(LPCWSTR,DWORD,LPWSTR, \
        LPWSTR*))aSyscall[13].pCurrent)

  { "GetLastError",            (SYSCALL)Get_آخری_Error,            0 },
#تعریف_کرو os_Get_آخری_Error ((DWORD(WINAPI*)(VOID))aSyscall[14].pCurrent)

#اگر !defined(SQLITE_OMIT_بھرو_EXTENSION) && !defined(SQLITE_UWP)
  { "GetProcAddressA",         (SYSCALL)Get_Proc_پتہ,          0 },
#ورنہ
  { "GetProcAddressA",         (SYSCALL)0,                       0 },
#اگر_ختم
#تعریف_کرو os_Get_Proc_پتہ_A ((FARPROC(WINAPI*)(HMODULE, \
        LPCSTR))aSyscall[15].pCurrent)

  { "GetSystemInfo",           (SYSCALL)GetSystemInfo,           0 },
#تعریف_کرو osGetSystemInfo ((VOID(WINAPI*)(LPSYSTEM_INFO))aSyscall[16].pCurrent)

  { "GetSystemTimeAsFileTime", (SYSCALL)Get_System_وقت_As_File_وقت, 0 },
#تعریف_کرو os_Get_System_وقت_As_File_وقت ((VOID(WINAPI*)( \
        LPFILETIME))aSyscall[17].pCurrent)

#اگر_تعریف_شدہ SQLITE_UWP
  { "GetTempPathW",            (SYSCALL)0,                       0 },
#ورنہ
  { "GetTempPathW",            (SYSCALL)Get_عارضی_راستہ_W,            0 },
#اگر_ختم
#تعریف_کرو os_Get_عارضی_راستہ_W ((DWORD(WINAPI*)(DWORD,LPWSTR))aSyscall[18].pCurrent)

  { "GetTickCount64",          (SYSCALL)GetTickCount64,          0 },
#تعریف_کرو osGetTickCount64 ((ULONGLONG(WINAPI*)(VOID))aSyscall[19].pCurrent)

#اگر_تعریف_شدہ SQLITE_UWP
  { "HeapAlloc",               (SYSCALL)0,                       0 },
#ورنہ
  { "HeapAlloc",               (SYSCALL)ڈھیر_Alloc,               0 },
#اگر_ختم
#تعریف_کرو os_ڈھیر_Alloc ((LPVOID(WINAPI*)(HANDLE,DWORD, \
        سائز_T))aSyscall[20].pCurrent)

#اگر_تعریف_شدہ SQLITE_UWP
  { "HeapCreate",              (SYSCALL)0,                       0 },
#ورنہ
  { "HeapCreate",              (SYSCALL)ڈھیر_Create,              0 },
#اگر_ختم
#تعریف_کرو os_ڈھیر_Create ((HANDLE(WINAPI*)(DWORD,سائز_T, \
        سائز_T))aSyscall[21].pCurrent)

#اگر_تعریف_شدہ SQLITE_UWP
  { "HeapDestroy",             (SYSCALL)0,                       0 },
#ورنہ
  { "HeapDestroy",             (SYSCALL)ڈھیر_Destroy,             0 },
#اگر_ختم
#تعریف_کرو os_ڈھیر_Destroy ((BOOL(WINAPI*)(HANDLE))aSyscall[22].pCurrent)

#اگر_تعریف_شدہ SQLITE_UWP
  { "HeapFree",                (SYSCALL)0,                       0 },
#ورنہ
  { "HeapFree",                (SYSCALL)ڈھیر_Free,                0 },
#اگر_ختم
#تعریف_کرو os_ڈھیر_Free ((BOOL(WINAPI*)(HANDLE,DWORD,LPVOID))aSyscall[23].pCurrent)

#اگر_تعریف_شدہ SQLITE_UWP
  { "HeapReAlloc",             (SYSCALL)0,                       0 },
#ورنہ
  { "HeapReAlloc",             (SYSCALL)ڈھیر_Re_Alloc,             0 },
#اگر_ختم
#تعریف_کرو os_ڈھیر_Re_Alloc ((LPVOID(WINAPI*)(HANDLE,DWORD,LPVOID, \
        سائز_T))aSyscall[24].pCurrent)

#اگر_تعریف_شدہ SQLITE_UWP
  { "HeapSize",                (SYSCALL)0,                       0 },
#ورنہ
  { "HeapSize",                (SYSCALL)ڈھیر_سائز,                0 },
#اگر_ختم
#تعریف_کرو os_ڈھیر_سائز ((سائز_T(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[25].pCurrent)

#اگر_تعریف_شدہ SQLITE_UWP
  { "HeapValidate",            (SYSCALL)0,                       0 },
#ورنہ
  { "HeapValidate",            (SYSCALL)ڈھیر_Validate,            0 },
#اگر_ختم
#تعریف_کرو os_ڈھیر_Validate ((BOOL(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[26].pCurrent)

#اگر_تعریف_شدہ SQLITE_UWP
  { "HeapCompact",             (SYSCALL)0,                       0 },
#ورنہ
  { "HeapCompact",             (SYSCALL)ڈھیر_Compact,             0 },
#اگر_ختم
#تعریف_کرو os_ڈھیر_Compact ((UINT(WINAPI*)(HANDLE,DWORD))aSyscall[27].pCurrent)


#اگر !defined(SQLITE_OMIT_بھرو_EXTENSION) && !defined(SQLITE_UWP)
  { "LoadLibraryW",            (SYSCALL)بھرو_Library_W,            0 },
#ورنہ
  { "LoadLibraryW",            (SYSCALL)0,                       0 },
#اگر_ختم
#تعریف_کرو os_بھرو_Library_W ((HMODULE(WINAPI*)(LPCWSTR))aSyscall[28].pCurrent)

  { "LocalFree",               (SYSCALL)LocalFree,               0 },
#تعریف_کرو osLocalFree ((HLOCAL(WINAPI*)(HLOCAL))aSyscall[29].pCurrent)

  { "LockFileEx",              (SYSCALL)تالا_File_Ex,              0 },
#تعریف_کرو os_تالا_File_Ex ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[30].pCurrent)

#اگر (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_سائز>0) \
 && !defined(SQLITE_UWP)
  { "MapViewOfFile",           (SYSCALL)Map_View_کا_File,           0 },
#ورنہ
  { "MapViewOfFile",           (SYSCALL)0,                       0 },
#اگر_ختم
#تعریف_کرو os_Map_View_کا_File ((LPVOID(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        سائز_T))aSyscall[31].pCurrent)

#اگر (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_سائز>0) \
 && defined(SQLITE_UWP)
  { "MapViewOfFileFromApp",    (SYSCALL)Map_View_کا_File_From_App,   0 },
#ورنہ
  { "MapViewOfFileFromApp",    (SYSCALL)0,                      0 },
#اگر_ختم
#تعریف_کرو os_Map_View_کا_File_From_App ((LPVOID(WINAPI*)(HANDLE,ULONG,ULONG64, \
        سائز_T))aSyscall[32].pCurrent)

  { "MultiByteToWideChar",     (SYSCALL)Multi_Byte_مقصد_Wide_Char,     0 },
#تعریف_کرو os_Multi_Byte_مقصد_Wide_Char ((صحیح(WINAPI*)(UINT,DWORD,LPCSTR,صحیح,LPWSTR, \
        صحیح))aSyscall[33].pCurrent)

  { "QueryPerformanceCounter", (SYSCALL)Query_Performance_شمار, 0 },
#تعریف_کرو os_Query_Performance_شمار ((BOOL(WINAPI*)( \
        LARGE_INTEGER*))aSyscall[34].pCurrent)

  { "ReadFile",                (SYSCALL)ReadFile,                0 },
#تعریف_کرو osReadFile ((BOOL(WINAPI*)(HANDLE,LPVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[35].pCurrent)

  { "SetEndOfFile",            (SYSCALL)سیٹ_انتہا_کا_File,            0 },
#تعریف_کرو os_سیٹ_انتہا_کا_File ((BOOL(WINAPI*)(HANDLE))aSyscall[36].pCurrent)

  { "SetFilePointerEx",        (SYSCALL)سیٹ_File_اشارہ_گر_Ex,        0 },
#تعریف_کرو os_سیٹ_File_اشارہ_گر_Ex ((BOOL(WINAPI*)(HANDLE,LARGE_INTEGER,\
        PLARGE_INTEGER,DWORD))aSyscall[37].pCurrent)

  { "Sleep",                   (SYSCALL)Sleep,                   0 },
#تعریف_کرو osSleep ((VOID(WINAPI*)(DWORD))aSyscall[38].pCurrent)

  { "UnlockFileEx",            (SYSCALL)UnlockFileEx,            0 },
#تعریف_کرو osUnlockFileEx ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[39].pCurrent)

#اگر !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_سائز>0
  { "UnmapViewOfFile",         (SYSCALL)Unmap_View_کا_File,         0 },
#ورنہ
  { "UnmapViewOfFile",         (SYSCALL)0,                       0 },
#اگر_ختم
#تعریف_کرو os_Unmap_View_کا_File ((BOOL(WINAPI*)(LPCVOID))aSyscall[40].pCurrent)

  { "WideCharToMultiByte",     (SYSCALL)Wide_Char_مقصد_Multi_Byte,     0 },
#تعریف_کرو os_Wide_Char_مقصد_Multi_Byte ((صحیح(WINAPI*)(UINT,DWORD,LPCWSTR,صحیح,LPSTR,صحیح, \
        LPCSTR,LPBOOL))aSyscall[41].pCurrent)

  { "WriteFile",               (SYSCALL)WriteFile,               0 },
#تعریف_کرو osWriteFile ((BOOL(WINAPI*)(HANDLE,LPCVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[42].pCurrent)

  { "WaitForSingleObject",     (SYSCALL)WaitForSingleObject,     0 },
#تعریف_کرو osWaitForSingleObject ((DWORD(WINAPI*)(HANDLE, \
        DWORD))aSyscall[43].pCurrent)

  { "WaitForSingleObjectEx",   (SYSCALL)WaitForSingleObjectEx,   0 },
#تعریف_کرو osWaitForSingleObjectEx ((DWORD(WINAPI*)(HANDLE,DWORD, \
        BOOL))aSyscall[44].pCurrent)

  { "OutputDebugStringA",      (SYSCALL)OutputDebugStringA,      0 },
#تعریف_کرو osOutputDebugStringA ((VOID(WINAPI*)(LPCSTR))aSyscall[45].pCurrent)

  { "GetProcessHeap",          (SYSCALL)Get_Process_ڈھیر,          0 },
#تعریف_کرو os_Get_Process_ڈھیر ((HANDLE(WINAPI*)(VOID))aSyscall[46].pCurrent)

/*
** NOTE: On some sub-platforms, the InterlockedCompareExchange "function"
**       is really just a macro that uses a compiler intrinsic (e.g. x64).
**       So do not try to make this is into a redefinable interface.
*/
#اگر defined(InterlockedCompareExchange)
  { "InterlockedCompareExchange", (SYSCALL)0,                    0 },
#تعریف_کرو osInterlockedCompareExchange InterlockedCompareExchange
#ورنہ
  { "InterlockedCompareExchange", (SYSCALL)InterlockedCompareExchange, 0 },
#تعریف_کرو osInterlockedCompareExchange ((LONG(WINAPI*)(LONG متلون*,\
        LONG,LONG))aSyscall[47].pCurrent)
#اگر_ختم /* defined(InterlockedCompareExchange) */

#اگر SQLITE_WIN32_USE_UUID
  { "UuidCreate",               (SYSCALL)UuidCreate,             0 },
#ورنہ
  { "UuidCreate",               (SYSCALL)0,                      0 },
#اگر_ختم
#تعریف_کرو osUuidCreate ((RPC_حالت(RPC_اندراج*)(UUID*))aSyscall[48].pCurrent)

#اگر SQLITE_WIN32_USE_UUID
  { "UuidCreateSequential",     (SYSCALL)UuidCreateSequential,   0 },
#ورنہ
  { "UuidCreateSequential",     (SYSCALL)0,                      0 },
#اگر_ختم
#تعریف_کرو osUuidCreateSequential \
        ((RPC_حالت(RPC_اندراج*)(UUID*))aSyscall[49].pCurrent)

#اگر !defined(SQLITE_NO_SYNC) && SQLITE_MAX_MMAP_سائز>0
  { "FlushViewOfFile",          (SYSCALL)Flush_View_کا_File,        0 },
#ورنہ
  { "FlushViewOfFile",          (SYSCALL)0,                      0 },
#اگر_ختم
#تعریف_کرو os_Flush_View_کا_File \
        ((BOOL(WINAPI*)(LPCVOID,سائز_T))aSyscall[50].pCurrent)

#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
  { "CreateEvent",              (SYSCALL)CreateEvent,            0 },
#ورنہ
  { "CreateEvent",              (SYSCALL)0,                      0 },
#اگر_ختم
#تعریف_کرو osCreateEvent ((HANDLE(WINAPI*)(LPSECURITY_اوصاف,BOOL, \
                        BOOL,LPCSTR))aSyscall[51].pCurrent)

#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
  { "CancelIo",                 (SYSCALL)CancelIo,               0 },
#ورنہ
  { "CancelIo",                 (SYSCALL)0,                      0 },
#اگر_ختم
#تعریف_کرو osCancelIo ((BOOL(WINAPI*)(HANDLE))aSyscall[52].pCurrent)

#اگر_غیر_تعریف_شدہ _WIN32
  { "getenv",                   (SYSCALL)getenv,                 0 },
#ورنہ
  { "getenv",                   (SYSCALL)0,                      0 },
#اگر_ختم
#تعریف_کرو osGetenv ((ثابت حرف *(*)(ثابت حرف *))aSyscall[53].pCurrent)

#اگر_غیر_تعریف_شدہ _WIN32
  { "getcwd",                   (SYSCALL)getcwd,                 0 },
#ورنہ
  { "getcwd",                   (SYSCALL)0,                      0 },
#اگر_ختم
#تعریف_کرو osGetcwd ((حرف*(*)(حرف*,size_t))aSyscall[54].pCurrent)

#اگر_غیر_تعریف_شدہ _WIN32
  { "readlink",                 (SYSCALL)readlink,               0 },
#ورنہ
  { "readlink",                 (SYSCALL)0,                      0 },
#اگر_ختم
#تعریف_کرو osReadlink ((ssize_t(*)(ثابت حرف*,حرف*,size_t))aSyscall[55].pCurrent)

#اگر_غیر_تعریف_شدہ _WIN32
  { "lstat",                    (SYSCALL)lstat,                  0 },
#ورنہ
  { "lstat",                    (SYSCALL)0,                      0 },
#اگر_ختم
#تعریف_کرو osLstat ((صحیح(*)(ثابت حرف*,ساخت stat*))aSyscall[56].pCurrent)

#اگر_غیر_تعریف_شدہ _WIN32
  { "__errno",                  (SYSCALL)__errno,                0 },
#ورنہ
  { "__errno",                  (SYSCALL)0,                      0 },
#اگر_ختم
#تعریف_کرو osErrno (*((صحیح*(*)(خالی))aSyscall[57].pCurrent)())

#اگر_غیر_تعریف_شدہ _WIN32
  { "cygwin_conv_path",         (SYSCALL)cygwin_conv_راستہ,       0 },
#ورنہ
  { "cygwin_conv_path",         (SYSCALL)0,                      0 },
#اگر_ختم
#تعریف_کرو os_Cygwin_conv_راستہ ((size_t(*)(غیرعلامتی صحیح, \
    ثابت خالی *, خالی *, size_t))aSyscall[58].pCurrent)

  { "GetDriveTypeW",            (SYSCALL)GetDriveTypeW,          0 },
#تعریف_کرو osGetDriveTypeW ((UINT(WINAPI*)(LPCWSTR))aSyscall[59].pCurrent)

}; /* End of the overrideable system calls */

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "win32" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
ساکن صحیح win_سیٹ_System_Call(
  sqlite3_vfs *p_Not_مستعمل,        /* The VFS pointer.  Not used */
  ثابت حرف *zName,            /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
){
  غیرعلامتی صحیح i;
  صحیح rc = SQLITE_NOTFOUND;

  غیر_مستعمل_PARAMETER(p_Not_مستعمل);
  اگر( zName==0 ){
    /* If no zName is given, restore all system calls to their default
    ** settings and return NULL
    */
    rc = SQLITE_OK;
    برائے(i=0; i<حجم(aSyscall)/حجم(aSyscall[0]); i++){
      اگر( aSyscall[i].pDefault ){
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  }ورنہ{
    /* If zName is specified, operate on only the one system call
    ** specified.
    */
    برائے(i=0; i<حجم(aSyscall)/حجم(aSyscall[0]); i++){
      اگر( strcmp(zName, aSyscall[i].zName)==0 ){
        اگر( aSyscall[i].pDefault==0 ){
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = SQLITE_OK;
        اگر( pNewFunc==0 ) pNewFunc = aSyscall[i].pDefault;
        aSyscall[i].pCurrent = pNewFunc;
        توڑو;
      }
    }
  }
  واپس rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
ساکن sqlite3_syscall_ptr winGetSystemCall(
  sqlite3_vfs *p_Not_مستعمل,
  ثابت حرف *zName
){
  غیرعلامتی صحیح i;

  غیر_مستعمل_PARAMETER(p_Not_مستعمل);
  برائے(i=0; i<حجم(aSyscall)/حجم(aSyscall[0]); i++){
    اگر( strcmp(zName, aSyscall[i].zName)==0 ) واپس aSyscall[i].pCurrent;
  }
  واپس 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
ساکن ثابت حرف *win_اگلا_System_Call(sqlite3_vfs *p, ثابت حرف *zName){
  صحیح i = -1;

  غیر_مستعمل_PARAMETER(p);
  اگر( zName ){
    برائے(i=0; i<Array_سائز(aSyscall)-1; i++){
      اگر( strcmp(zName, aSyscall[i].zName)==0 ) توڑو;
    }
  }
  برائے(i++; i<Array_سائز(aSyscall); i++){
    اگر( aSyscall[i].pCurrent!=0 ) واپس aSyscall[i].zName;
  }
  واپس 0;
}

#اگر_تعریف_شدہ SQLITE_WIN32_MALLOC
#اگر_تعریف_شدہ SQLITE_UWP
# error SQLITE_WIN32_MALLOC is incompatible with SQLITE_UWP
#اگر_ختم
/*
** If a Win32 native heap has been configured, this function will attempt to
** compact it.  Upon success, SQLITE_OK will be returned.  Upon failure, one
** of SQLITE_NOMEM, SQLITE_ERROR, or SQLITE_NOTFOUND will be returned.  The
** "pnLargest" argument, if non-zero, will be used to return the size of the
** largest committed free block in the heap, in bytes.
*/
صحیح sqlite3_win32_compact_ڈھیر(LPUINT pn_سب_سے_بڑا){
  صحیح rc = SQLITE_OK;
  UINT n_سب_سے_بڑا = 0;
  HANDLE h_ڈھیر;

  winMemAssertMagic();
  h_ڈھیر = win_Mem_Get_ڈھیر();
  أكد( h_ڈھیر!=0 );
  أكد( h_ڈھیر!=INVALID_HANDLE_قدر );
#اگر defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( os_ڈھیر_Validate(h_ڈھیر, SQLITE_WIN32_ڈھیر_FLAGS, تہی) );
#اگر_ختم
  اگر( (n_سب_سے_بڑا=os_ڈھیر_Compact(h_ڈھیر, SQLITE_WIN32_ڈھیر_FLAGS))==0 ){
    DWORD آخری_Errno = os_Get_آخری_Error();
    اگر( آخری_Errno==NO_ERROR ){
      sqlite3_log(SQLITE_NOMEM, "failed to HeapCompact (no space), heap=%p",
                  (خالی*)h_ڈھیر);
      rc = SQLITE_NOMEM_BKPT;
    }ورنہ{
      sqlite3_log(SQLITE_ERROR, "failed to HeapCompact (%lu), heap=%p",
                  os_Get_آخری_Error(), (خالی*)h_ڈھیر);
      rc = SQLITE_ERROR;
    }
  }
  اگر( pn_سب_سے_بڑا ) *pn_سب_سے_بڑا = n_سب_سے_بڑا;
  واپس rc;
}

/*
** If a Win32 native heap has been configured, this function will attempt to
** destroy and recreate it.  If the Win32 native heap is not isolated and/or
** the sqlite3_memory_used() function does not return zero, SQLITE_BUSY will
** be returned and no changes will be made to the Win32 native heap.
*/
صحیح sqlite3_win32_reset_ڈھیر(){
  صحیح rc;
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; ) /* The main static mutex */
  MUTEX_LOGIC( sqlite3_mutex *pMem; )    /* The memsys static mutex */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  MUTEX_LOGIC( pMem = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3_mutex_enter(pMem);
  winMemAssertMagic();
  اگر( win_Mem_Get_ڈھیر()!=تہی && winMemGetOwned() && sqlite3_حافظہ_مستعمل()==0 ){
    /*
    ** At this point, there should be no outstanding memory allocations on
    ** the heap.  Also, since both the main and memsys locks are currently
    ** being held by us, no other function (i.e. from another thread) should
    ** be able to even access the heap.  Attempt to destroy and recreate our
    ** isolated Win32 native heap now.
    */
    أكد( win_Mem_Get_ڈھیر()!=تہی );
    أكد( winMemGetOwned() );
    أكد( sqlite3_حافظہ_مستعمل()==0 );
    winMemShutdown(win_Mem_Get_ڈیٹا_Ptr());
    أكد( win_Mem_Get_ڈھیر()==تہی );
    أكد( !winMemGetOwned() );
    أكد( sqlite3_حافظہ_مستعمل()==0 );
    rc = winMemInit(win_Mem_Get_ڈیٹا_Ptr());
    أكد( rc!=SQLITE_OK || win_Mem_Get_ڈھیر()!=تہی );
    أكد( rc!=SQLITE_OK || winMemGetOwned() );
    أكد( rc!=SQLITE_OK || sqlite3_حافظہ_مستعمل()==0 );
  }ورنہ{
    /*
    ** The Win32 native heap cannot be modified because it may be in use.
    */
    rc = SQLITE_BUSY;
  }
  sqlite3_mutex_leave(pMem);
  sqlite3_mutex_leave(pMainMtx);
  واپس rc;
}
#اگر_ختم /* SQLITE_WIN32_MALLOC */

#اگر_تعریف_شدہ _WIN32
/*
** This function outputs the specified (ANSI) string to the Win32 debugger
** (if available).  Undocumented.  Might go away at any moment.
*/
خالی sqlite3_win32_write_debug(ثابت حرف *zBuf, صحیح nBuf){
  حرف zDbgBuf[SQLITE_WIN32_DBG_BUF_سائز];
  صحیح nMin = MIN(nBuf, (SQLITE_WIN32_DBG_BUF_سائز - 1)); /* may be negative. */
  اگر( nMin<-1 ) nMin = -1; /* all negative values become -1. */
  أكد( nMin==-1 || nMin==0 || nMin<SQLITE_WIN32_DBG_BUF_سائز );
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !zBuf ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس;
  }
#اگر_ختم
  اگر( nMin>0 ){
    memset(zDbgBuf, 0, SQLITE_WIN32_DBG_BUF_سائز);
    حافظہ_نقل(zDbgBuf, zBuf, nMin);
    osOutputDebugStringA(zDbgBuf);
  }ورنہ{
    osOutputDebugStringA(zBuf);
  }
}

خالی sqlite3_win32_sleep(DWORD milliseconds){
  osSleep(milliseconds);
}

#اگر SQLITE_MAX_کارکن_دھاگے>0 && SQLITE_THREADSAFE>0
DWORD sqlite3Win32Wait(HANDLE hObject){
  DWORD rc;
  جب( (rc = osWaitForSingleObjectEx(hObject, INFINITE,
                                       TRUE))==WAIT_IO_COMPLETION ){}
  واپس rc;
}
#اگر_ختم
#اگر_ختم /* _WIN32 */

#اگر_تعریف_شدہ SQLITE_WIN32_MALLOC
/*
** Allocate nBytes of memory.
*/
ساکن خالی *winMemMalloc(صحیح n_بائٹیں){
  HANDLE h_ڈھیر;
  خالی *p;

  winMemAssertMagic();
  h_ڈھیر = win_Mem_Get_ڈھیر();
  أكد( h_ڈھیر!=0 );
  أكد( h_ڈھیر!=INVALID_HANDLE_قدر );
#اگر defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( os_ڈھیر_Validate(h_ڈھیر, SQLITE_WIN32_ڈھیر_FLAGS, تہی) );
#اگر_ختم
  أكد( n_بائٹیں>=0 );
  p = os_ڈھیر_Alloc(h_ڈھیر, SQLITE_WIN32_ڈھیر_FLAGS, (سائز_T)n_بائٹیں);
  اگر( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapAlloc %u bytes (%lu), heap=%p",
                n_بائٹیں, os_Get_آخری_Error(), (خالی*)h_ڈھیر);
  }
  واپس p;
}

/*
** Free memory.
*/
ساکن خالی winMemFree(خالی *p_پیشگی){
  HANDLE h_ڈھیر;

  winMemAssertMagic();
  h_ڈھیر = win_Mem_Get_ڈھیر();
  أكد( h_ڈھیر!=0 );
  أكد( h_ڈھیر!=INVALID_HANDLE_قدر );
#اگر defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( os_ڈھیر_Validate(h_ڈھیر, SQLITE_WIN32_ڈھیر_FLAGS, p_پیشگی) );
#اگر_ختم
  اگر( !p_پیشگی ) واپس; /* Passing NULL to HeapFree is undefined. */
  اگر( !os_ڈھیر_Free(h_ڈھیر, SQLITE_WIN32_ڈھیر_FLAGS, p_پیشگی) ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapFree block %p (%lu), heap=%p",
                p_پیشگی, os_Get_آخری_Error(), (خالی*)h_ڈھیر);
  }
}

/*
** Change the size of an existing memory allocation
*/
ساکن خالی *winMemRealloc(خالی *p_پیشگی, صحیح n_بائٹیں){
  HANDLE h_ڈھیر;
  خالی *p;

  winMemAssertMagic();
  h_ڈھیر = win_Mem_Get_ڈھیر();
  أكد( h_ڈھیر!=0 );
  أكد( h_ڈھیر!=INVALID_HANDLE_قدر );
#اگر defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( os_ڈھیر_Validate(h_ڈھیر, SQLITE_WIN32_ڈھیر_FLAGS, p_پیشگی) );
#اگر_ختم
  أكد( n_بائٹیں>=0 );
  اگر( !p_پیشگی ){
    p = os_ڈھیر_Alloc(h_ڈھیر, SQLITE_WIN32_ڈھیر_FLAGS, (سائز_T)n_بائٹیں);
  }ورنہ{
    p = os_ڈھیر_Re_Alloc(h_ڈھیر, SQLITE_WIN32_ڈھیر_FLAGS, p_پیشگی, (سائز_T)n_بائٹیں);
  }
  اگر( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to %s %u bytes (%lu), heap=%p",
                p_پیشگی ? "HeapReAlloc" : "HeapAlloc", n_بائٹیں, os_Get_آخری_Error(),
                (خالی*)h_ڈھیر);
  }
  واپس p;
}

/*
** Return the size of an outstanding allocation, in bytes.
*/
ساکن صحیح win_Mem_سائز(خالی *p){
  HANDLE h_ڈھیر;
  سائز_T n;

  winMemAssertMagic();
  h_ڈھیر = win_Mem_Get_ڈھیر();
  أكد( h_ڈھیر!=0 );
  أكد( h_ڈھیر!=INVALID_HANDLE_قدر );
#اگر defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( os_ڈھیر_Validate(h_ڈھیر, SQLITE_WIN32_ڈھیر_FLAGS, p) );
#اگر_ختم
  اگر( !p ) واپس 0;
  n = os_ڈھیر_سائز(h_ڈھیر, SQLITE_WIN32_ڈھیر_FLAGS, p);
  اگر( n==(سائز_T)-1 ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapSize block %p (%lu), heap=%p",
                p, os_Get_آخری_Error(), (خالی*)h_ڈھیر);
    واپس 0;
  }
  واپس (صحیح)n;
}

/*
** Round up a request size to the next valid allocation size.
*/
ساکن صحیح winMemRoundup(صحیح n){
  واپس n;
}

/*
** Initialize this module.
*/
ساکن صحیح winMemInit(خالی *p_App_ڈیٹا){
  win_Mem_ڈیٹا *p_Win_Mem_ڈیٹا = (win_Mem_ڈیٹا *)p_App_ڈیٹا;

  اگر( !p_Win_Mem_ڈیٹا ) واپس SQLITE_ERROR;
  أكد( p_Win_Mem_ڈیٹا->magic1==WINMEM_MAGIC1 );
  أكد( p_Win_Mem_ڈیٹا->magic2==WINMEM_MAGIC2 );

#اگر SQLITE_WIN32_ڈھیر_CREATE
  اگر( !p_Win_Mem_ڈیٹا->h_ڈھیر ){
    DWORD dw_Initial_سائز = SQLITE_WIN32_ڈھیر_INIT_سائز;
    DWORD dw_زیادہ_سے_زیادہ_سائز = (DWORD)sqlite3GlobalConfig.n_ڈھیر;
    اگر( dw_زیادہ_سے_زیادہ_سائز==0 ){
      dw_زیادہ_سے_زیادہ_سائز = SQLITE_WIN32_ڈھیر_MAX_سائز;
    }ورنہ اگر( dw_Initial_سائز>dw_زیادہ_سے_زیادہ_سائز ){
      dw_Initial_سائز = dw_زیادہ_سے_زیادہ_سائز;
    }
    p_Win_Mem_ڈیٹا->h_ڈھیر = os_ڈھیر_Create(SQLITE_WIN32_ڈھیر_FLAGS,
                                      dw_Initial_سائز, dw_زیادہ_سے_زیادہ_سائز);
    اگر( !p_Win_Mem_ڈیٹا->h_ڈھیر ){
      sqlite3_log(SQLITE_NOMEM,
          "failed to HeapCreate (%lu), flags=%u, initSize=%lu, maxSize=%lu",
          os_Get_آخری_Error(), SQLITE_WIN32_ڈھیر_FLAGS, dw_Initial_سائز,
          dw_زیادہ_سے_زیادہ_سائز);
      واپس SQLITE_NOMEM_BKPT;
    }
    p_Win_Mem_ڈیٹا->bOwned = TRUE;
    أكد( p_Win_Mem_ڈیٹا->bOwned );
  }
#ورنہ
  p_Win_Mem_ڈیٹا->h_ڈھیر = os_Get_Process_ڈھیر();
  اگر( !p_Win_Mem_ڈیٹا->h_ڈھیر ){
    sqlite3_log(SQLITE_NOMEM,
        "failed to GetProcessHeap (%lu)", os_Get_آخری_Error());
    واپس SQLITE_NOMEM_BKPT;
  }
  p_Win_Mem_ڈیٹا->bOwned = FALSE;
  أكد( !p_Win_Mem_ڈیٹا->bOwned );
#اگر_ختم
  أكد( p_Win_Mem_ڈیٹا->h_ڈھیر!=0 );
  أكد( p_Win_Mem_ڈیٹا->h_ڈھیر!=INVALID_HANDLE_قدر );
#اگر defined(SQLITE_WIN32_MALLOC_VALIDATE)
  أكد( os_ڈھیر_Validate(p_Win_Mem_ڈیٹا->h_ڈھیر, SQLITE_WIN32_ڈھیر_FLAGS, تہی) );
#اگر_ختم
  واپس SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ساکن خالی winMemShutdown(خالی *p_App_ڈیٹا){
  win_Mem_ڈیٹا *p_Win_Mem_ڈیٹا = (win_Mem_ڈیٹا *)p_App_ڈیٹا;

  اگر( !p_Win_Mem_ڈیٹا ) واپس;
  أكد( p_Win_Mem_ڈیٹا->magic1==WINMEM_MAGIC1 );
  أكد( p_Win_Mem_ڈیٹا->magic2==WINMEM_MAGIC2 );

  اگر( p_Win_Mem_ڈیٹا->h_ڈھیر ){
    أكد( p_Win_Mem_ڈیٹا->h_ڈھیر!=INVALID_HANDLE_قدر );
#اگر defined(SQLITE_WIN32_MALLOC_VALIDATE)
    أكد( os_ڈھیر_Validate(p_Win_Mem_ڈیٹا->h_ڈھیر, SQLITE_WIN32_ڈھیر_FLAGS, تہی) );
#اگر_ختم
    اگر( p_Win_Mem_ڈیٹا->bOwned ){
      اگر( !os_ڈھیر_Destroy(p_Win_Mem_ڈیٹا->h_ڈھیر) ){
        sqlite3_log(SQLITE_NOMEM, "failed to HeapDestroy (%lu), heap=%p",
                    os_Get_آخری_Error(), (خالی*)p_Win_Mem_ڈیٹا->h_ڈھیر);
      }
      p_Win_Mem_ڈیٹا->bOwned = FALSE;
    }
    p_Win_Mem_ڈیٹا->h_ڈھیر = تہی;
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
ثابت sqlite3_mem_methods *sqlite3MemGetWin32(خالی){
  ساکن ثابت sqlite3_mem_methods winMemMethods = {
    winMemMalloc,
    winMemFree,
    winMemRealloc,
    win_Mem_سائز,
    winMemRoundup,
    winMemInit,
    winMemShutdown,
    &win_mem_ڈیٹا
  };
  واپس &winMemMethods;
}

خالی sqlite3_Mem_سیٹ_Default(خالی){
  sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetWin32());
}
#اگر_ختم /* SQLITE_WIN32_MALLOC */

#اگر_تعریف_شدہ _WIN32
/*
** Convert a UTF-8 string to Microsoft Unicode.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساکن LPWSTR win_Utf8_مقصد_Unicode(ثابت حرف *z_متن){
  صحیح nChar;
  LPWSTR z_Wide_متن;

  nChar = os_Multi_Byte_مقصد_Wide_Char(CP_UTF8, 0, z_متن, -1, تہی, 0);
  اگر( nChar==0 ){
    واپس 0;
  }
  z_Wide_متن = sqlite3MallocZero( nChar*حجم(WCHAR) );
  اگر( z_Wide_متن==0 ){
    واپس 0;
  }
  nChar = os_Multi_Byte_مقصد_Wide_Char(CP_UTF8, 0, z_متن, -1, z_Wide_متن,
                                nChar);
  اگر( nChar==0 ){
    sqlite3_free(z_Wide_متن);
    z_Wide_متن = 0;
  }
  واپس z_Wide_متن;
}
#اگر_ختم /* _WIN32 */

/*
** Convert a Microsoft Unicode string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساکن حرف *win_Unicode_مقصد_Utf8(LPCWSTR z_Wide_متن){
  صحیح nByte;
  حرف *z_متن;

  nByte = os_Wide_Char_مقصد_Multi_Byte(CP_UTF8, 0, z_Wide_متن, -1, 0, 0, 0, 0);
  اگر( nByte == 0 ){
    واپس 0;
  }
  z_متن = sqlite3MallocZero( nByte );
  اگر( z_متن==0 ){
    واپس 0;
  }
  nByte = os_Wide_Char_مقصد_Multi_Byte(CP_UTF8, 0, z_Wide_متن, -1, z_متن, nByte,
                                0, 0);
  اگر( nByte == 0 ){
    sqlite3_free(z_متن);
    z_متن = 0;
  }
  واپس z_متن;
}

#اگر_تعریف_شدہ _WIN32
/*
** Convert an ANSI string to Microsoft Unicode, using the ANSI or OEM
** code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساکن LPWSTR win_Mbcs_مقصد_Unicode(ثابت حرف *z_متن, صحیح useAnsi){
  صحیح nWideChar;
  LPWSTR z_Mbcs_متن;
  صحیح codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nWideChar = os_Multi_Byte_مقصد_Wide_Char(codepage, 0, z_متن, -1, تہی,
                                0);
  اگر( nWideChar==0 ){
    واپس 0;
  }
  z_Mbcs_متن = sqlite3MallocZero( nWideChar*حجم(WCHAR) );
  اگر( z_Mbcs_متن==0 ){
    واپس 0;
  }
  nWideChar = os_Multi_Byte_مقصد_Wide_Char(codepage, 0, z_متن, -1, z_Mbcs_متن,
                                nWideChar);
  اگر( nWideChar==0 ){
    sqlite3_free(z_Mbcs_متن);
    z_Mbcs_متن = 0;
  }
  واپس z_Mbcs_متن;
}

/*
** Convert a Microsoft Unicode string to a multi-byte character string,
** using the ANSI or OEM code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساکن حرف *win_Unicode_مقصد_Mbcs(LPCWSTR z_Wide_متن, صحیح useAnsi){
  صحیح nByte;
  حرف *z_متن;
  صحیح codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nByte = os_Wide_Char_مقصد_Multi_Byte(codepage, 0, z_Wide_متن, -1, 0, 0, 0, 0);
  اگر( nByte == 0 ){
    واپس 0;
  }
  z_متن = sqlite3MallocZero( nByte );
  اگر( z_متن==0 ){
    واپس 0;
  }
  nByte = os_Wide_Char_مقصد_Multi_Byte(codepage, 0, z_Wide_متن, -1, z_متن,
                                nByte, 0, 0);
  اگر( nByte == 0 ){
    sqlite3_free(z_متن);
    z_متن = 0;
  }
  واپس z_متن;
}

/*
** Convert a multi-byte character string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساکن حرف *win_Mbcs_مقصد_Utf8(ثابت حرف *z_متن, صحیح useAnsi){
  حرف *z_متن_Utf8;
  LPWSTR zTmpWide;

  zTmpWide = win_Mbcs_مقصد_Unicode(z_متن, useAnsi);
  اگر( zTmpWide==0 ){
    واپس 0;
  }
  z_متن_Utf8 = win_Unicode_مقصد_Utf8(zTmpWide);
  sqlite3_free(zTmpWide);
  واپس z_متن_Utf8;
}

/*
** Convert a UTF-8 string to a multi-byte character string.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ساکن حرف *win_Utf8_مقصد_Mbcs(ثابت حرف *z_متن, صحیح useAnsi){
  حرف *z_متن_Mbcs;
  LPWSTR zTmpWide;

  zTmpWide = win_Utf8_مقصد_Unicode(z_متن);
  اگر( zTmpWide==0 ){
    واپس 0;
  }
  z_متن_Mbcs = win_Unicode_مقصد_Mbcs(zTmpWide, useAnsi);
  sqlite3_free(zTmpWide);
  واپس z_متن_Mbcs;
}

/*
** This is a public wrapper for the winUtf8ToUnicode() function.
*/
LPWSTR sqlite3_win32_utf8_مقصد_unicode(ثابت حرف *z_متن){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !z_متن ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) واپس 0;
#اگر_ختم
  واپس win_Utf8_مقصد_Unicode(z_متن);
}

/*
** This is a public wrapper for the winUnicodeToUtf8() function.
*/
حرف *sqlite3_win32_unicode_مقصد_utf8(LPCWSTR z_Wide_متن){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !z_Wide_متن ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) واپس 0;
#اگر_ختم
  واپس win_Unicode_مقصد_Utf8(z_Wide_متن);
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
حرف *sqlite3_win32_mbcs_مقصد_utf8(ثابت حرف *z_متن){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !z_متن ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) واپس 0;
#اگر_ختم
  واپس win_Mbcs_مقصد_Utf8(z_متن, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
حرف *sqlite3_win32_mbcs_مقصد_utf8_v2(ثابت حرف *z_متن, صحیح useAnsi){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !z_متن ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) واپس 0;
#اگر_ختم
  واپس win_Mbcs_مقصد_Utf8(z_متن, useAnsi);
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
حرف *sqlite3_win32_utf8_مقصد_mbcs(ثابت حرف *z_متن){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !z_متن ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) واپس 0;
#اگر_ختم
  واپس win_Utf8_مقصد_Mbcs(z_متن, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
حرف *sqlite3_win32_utf8_مقصد_mbcs_v2(ثابت حرف *z_متن, صحیح useAnsi){
#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !z_متن ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  اگر( sqlite3_initialize() ) واپس 0;
#اگر_ختم
  واپس win_Utf8_مقصد_Mbcs(z_متن, useAnsi);
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-8 string.
*/
صحیح sqlite3_win32_سیٹ_directory8(
  غیرعلامتی طویل type, /* Identifier for directory being set or reset */
  ثابت حرف *z_قدر  /* New value for directory being set or reset */
){
  حرف **ppDirectory = 0;
  صحیح rc;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  اگر( rc ) واپس rc;
#اگر_ختم
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  اگر( type==SQLITE_WIN32_ڈیٹا_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_ڈیٹا_directory;
  }ورنہ اگر( type==SQLITE_WIN32_عارضی_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_عارضی_directory;
  }
  أكد( !ppDirectory || type==SQLITE_WIN32_ڈیٹا_DIRECTORY_TYPE
          || type==SQLITE_WIN32_عارضی_DIRECTORY_TYPE
  );
  أكد( !ppDirectory || sqlite3MemdebugHasType(*ppDirectory, MEMTYPE_ڈھیر) );
  اگر( ppDirectory ){
    حرف *zCopy = 0;
    اگر( z_قدر && z_قدر[0] ){
      zCopy = sqlite3_mprintf("%s", z_قدر);
      اگر ( zCopy==0 ){
        rc = SQLITE_NOMEM_BKPT;
        جاؤ سیٹ_directory8_مکمل;
      }
    }
    sqlite3_free(*ppDirectory);
    *ppDirectory = zCopy;
    rc = SQLITE_OK;
  }ورنہ{
    rc = SQLITE_ERROR;
  }
سیٹ_directory8_مکمل:
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  واپس rc;
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-16 string.
*/
صحیح sqlite3_win32_سیٹ_directory16(
  غیرعلامتی طویل type, /* Identifier for directory being set or reset */
  ثابت خالی *z_قدر  /* New value for directory being set or reset */
){
  صحیح rc;
  حرف *zUtf8 = 0;
  اگر( z_قدر ){
    zUtf8 = sqlite3_win32_unicode_مقصد_utf8(z_قدر);
    اگر( zUtf8==0 ) واپس SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_win32_سیٹ_directory8(type, zUtf8);
  اگر( zUtf8 ) sqlite3_free(zUtf8);
  واپس rc;
}

/*
** This function sets the data directory or the temporary directory based on
** the provided arguments.  The type argument must be 1 in order to set the
** data directory or 2 in order to set the temporary directory.  The zValue
** argument is the name of the directory to use.  The return value will be
** SQLITE_OK if successful.
*/
صحیح sqlite3_win32_سیٹ_directory(
  غیرعلامتی طویل type, /* Identifier for directory being set or reset */
  خالی *z_قدر        /* New value for directory being set or reset */
){
  واپس sqlite3_win32_سیٹ_directory16(type, z_قدر);
}
#اگر_ختم /* _WIN32 */

/*
** The return value of winGetLastErrorMsg
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated).
*/
ساکن صحیح win_Get_آخری_Error_پیغام(DWORD آخری_Errno, صحیح nBuf, حرف *zBuf){
  /* FormatMessage returns 0 on failure.  Otherwise it
  ** returns the number of TCHARs written to the output
  ** buffer, excluding the terminating null char.
  */
  DWORD dwLen = 0;
  حرف *z_برآمد = 0;
  LPWSTR z_عارضی_Wide = تہی;
  dwLen = os_Format_پیغام_W(FORMAT_پیغام_ALLOCATE_بفر |
                           FORMAT_پیغام_FROM_SYSTEM |
                           FORMAT_پیغام_IGNORE_INSERTS,
                           تہی,
                           آخری_Errno,
                           0,
                           (LPWSTR) &z_عارضی_Wide,
                           0,
                           0);
  اگر( dwLen > 0 ){
    /* allocate a buffer and convert to UTF8 */
    sqlite3BeginBenignMalloc();
    z_برآمد = win_Unicode_مقصد_Utf8(z_عارضی_Wide);
    sqlite3_انتہا_Benign_Malloc();
    /* free the system buffer allocated by FormatMessage */
    osLocalFree(z_عارضی_Wide);
  }
  اگر( 0 == dwLen ){
    sqlite3_snprintf(nBuf, zBuf, "OsError 0x%lx (%lu)", آخری_Errno, آخری_Errno);
  }ورنہ{
    /* copy a maximum of nBuf chars to output buffer */
    sqlite3_snprintf(nBuf, zBuf, "%s", z_برآمد);
    /* free the UTF8 buffer */
    sqlite3_free(z_برآمد);
  }
  واپس 0;
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
#تعریف_کرو winLogError(a,b,c,d)   win_Log_Error_At_سطر(a,b,c,d,__LINE__)
ساکن صحیح win_Log_Error_At_سطر(
  صحیح errcode,                    /* SQLite error code */
  DWORD آخری_Errno,                /* Win32 last error */
  ثابت حرف *zFunc,              /* Name of OS function that failed */
  ثابت حرف *z_راستہ,              /* File path associated with error */
  صحیح i_سطر                       /* Source line number where error occurred */
){
  حرف z_پیغام[500];                 /* Human readable error text */
  صحیح i;                          /* Loop counter */

  z_پیغام[0] = 0;
  win_Get_آخری_Error_پیغام(آخری_Errno, حجم(z_پیغام), z_پیغام);
  أكد( errcode!=SQLITE_OK );
  اگر( z_راستہ==0 ) z_راستہ = "";
  برائے(i=0; z_پیغام[i] && z_پیغام[i]!='\r' && z_پیغام[i]!='\n'; i++){}
  z_پیغام[i] = 0;
  sqlite3_log(errcode,
      "os_win.c:%d: (%lu) %s(%s) - %s",
      i_سطر, آخری_Errno, zFunc, z_راستہ, z_پیغام
  );

  واپس errcode;
}

/*
** The number of times that a ReadFile(), WriteFile(), and DeleteFile()
** will be retried following a locking error - probably caused by
** antivirus software.  Also the initial delay before the first retry.
** The delay increases linearly with each retry.
*/
#اگر_غیر_تعریف_شدہ SQLITE_WIN32_IOERR_RETRY
# define SQLITE_WIN32_IOERR_RETRY 10
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_WIN32_IOERR_RETRY_DELAY
# define SQLITE_WIN32_IOERR_RETRY_DELAY 25
#اگر_ختم
ساکن صحیح winIoerrRetry = SQLITE_WIN32_IOERR_RETRY;
ساکن صحیح winIoerrRetryDelay = SQLITE_WIN32_IOERR_RETRY_DELAY;

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
#اگر !defined(winIoerrCanRetry1)
#تعریف_کرو winIoerrCanRetry1(a) (((a)==ERROR_ACCESS_DENIED)        || \
                              ((a)==ERROR_SHARING_VIOLATION)    || \
                              ((a)==ERROR_تالا_VIOLATION)       || \
                              ((a)==ERROR_DEV_NOT_EXIST)        || \
                              ((a)==ERROR_NETNAME_DELETED)      || \
                              ((a)==ERROR_SEM_TIMEOUT)          || \
                              ((a)==ERROR_جال_UNREACHABLE))
#اگر_ختم

/*
** If a ReadFile() or WriteFile() error occurs, invoke this routine
** to see if it should be retried.  Return TRUE to retry.  Return FALSE
** to give up with an error.
*/
ساکن صحیح winRetryIoerr(صحیح *pnRetry, DWORD *pError){
  DWORD e = os_Get_آخری_Error();
  اگر( *pnRetry>=winIoerrRetry ){
    اگر( pError ){
      *pError = e;
    }
    واپس 0;
  }
  اگر( winIoerrCanRetry1(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    واپس 1;
  }
#اگر defined(winIoerrCanRetry2)
  ورنہ اگر( winIoerrCanRetry2(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    واپس 1;
  }
#اگر_ختم
  اگر( pError ){
    *pError = e;
  }
  واپس 0;
}

/*
** Log a I/O error retry episode.
*/
ساکن خالی winLogIoerr(صحیح nRetry, صحیح lineno){
  اگر( nRetry ){
    sqlite3_log(SQLITE_NOTICE,
      "delayed %dms for lock/sharing conflict at line %d",
      winIoerrRetryDelay*nRetry*(nRetry+1)/2, lineno
    );
  }
}


/*
** Lock a file region.
*/
ساکن BOOL win_تالا_File(
  LPHANDLE phFile,
  DWORD flags,
  DWORD انحراف_کم,
  DWORD انحراف_بلند,
  DWORD عدد_بائٹیں_کم,
  DWORD عدد_بائٹیں_بلند
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, حجم(OVERLAPPED));
  ovlp.انحراف_2 = انحراف_کم;
  ovlp.انحراف_بلند_2 = انحراف_بلند;
  واپس os_تالا_File_Ex(*phFile, flags, 0, عدد_بائٹیں_کم, عدد_بائٹیں_بلند, &ovlp);
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
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
ساکن صحیح win_Handle_تالا_Timeout(
  HANDLE hFile,
  DWORD انحراف,
  DWORD nByte,
  صحیح bExcl,
  DWORD nMs
){
  DWORD flags = LOCKFILE_FAIL_IMMEDIATELY | (bExcl?LOCKFILE_EXCLUSIVE_تالا:0);
  صحیح rc = SQLITE_OK;
  BOOL ret;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, حجم(OVERLAPPED));
  ovlp.انحراف_2 = انحراف;

#اگر defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  اگر( nMs!=0 ){
    flags &= ~LOCKFILE_FAIL_IMMEDIATELY;
  }
  ovlp.hEvent = osCreateEvent(تہی, TRUE, FALSE, تہی);
  اگر( ovlp.hEvent==تہی ){
    واپس SQLITE_IOERR_تالا;
  }
#اگر_ختم
  ret = os_تالا_File_Ex(hFile, flags, 0, nByte, 0, &ovlp);
#اگر defined(SQLITE_ENABLE_SETLK_TIMEOUT)
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
  اگر( !ret && Get_آخری_Error()==ERROR_IO_PENDING ){
    DWORD nDelay = (nMs==0 ? INFINITE : nMs);
    DWORD res = osWaitForSingleObject(ovlp.hEvent, nDelay);
    اگر( res==WAIT_OBJECT_0 ){
      ret = TRUE;
    }ورنہ اگر( res==WAIT_TIMEOUT ){
#اگر SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = SQLITE_BUSY_TIMEOUT;
#ورنہ
      rc = SQLITE_BUSY;
#اگر_ختم
    }ورنہ{
      /* Some other error has occurred */
      rc = SQLITE_IOERR_تالا;
    }

    /* If it is still pending, cancel the LockFileEx() call. */
    osCancelIo(hFile);
  }

  osCloseHandle(ovlp.hEvent);
#اگر_ختم /* defined(SQLITE_ENABLE_SETLK_TIMEOUT) */

  اگر( rc==SQLITE_OK && !ret ){
    rc = SQLITE_BUSY;
  }
  واپس rc;
}
#اگر_ختم /* #ifndef SQLITE_OMIT_WAL */

/*
** Unlock a file region.
 */
ساکن BOOL winUnlockFile(
  LPHANDLE phFile,
  DWORD انحراف_کم,
  DWORD انحراف_بلند,
  DWORD عدد_بائٹیں_کم,
  DWORD عدد_بائٹیں_بلند
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, حجم(OVERLAPPED));
  ovlp.انحراف_2 = انحراف_کم;
  ovlp.انحراف_بلند_2 = انحراف_بلند;
  واپس osUnlockFileEx(*phFile, 0, عدد_بائٹیں_کم, عدد_بائٹیں_بلند, &ovlp);
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
/*
** Remove an nByte lock starting at offset iOff from HANDLE h.
*/
ساکن صحیح winHandleUnlock(HANDLE h, صحیح iOff, صحیح nByte){
  BOOL ret = winUnlockFile(&h, iOff, 0, nByte, 0);
  واپس (ret ? SQLITE_OK : SQLITE_IOERR_UNLOCK);
}
#اگر_ختم

/*****************************************************************************
** The next group of routines implement the I/O methods specified
** by the sqlite3_io_methods object.
******************************************************************************/

/*
** Some Microsoft compilers lack this definition.
*/
#اگر_غیر_تعریف_شدہ INVALID_سیٹ_FILE_اشارہ_گر
# define INVALID_سیٹ_FILE_اشارہ_گر ((DWORD)-1)
#اگر_ختم

/*
** Seek the file handle h to offset nByte of the file.
**
** If successful, return SQLITE_OK. Or, if an error occurs, return an SQLite
** error code.
*/
ساکن صحیح winHandleSeek(HANDLE h, sqlite3_int64 i_انحراف){
  صحیح rc = SQLITE_OK;             /* Return value */
  LARGE_INTEGER x;                /* The offset */

  x.QuadPart = i_انحراف;
  اگر( os_سیٹ_File_اشارہ_گر_Ex(h, x, 0, FILE_BEGIN)==0 ){
    rc = SQLITE_IOERR_SEEK;
  }
  OSTRACE(("SEEK file=%p, offset=%lld rc=%s\n", h, i_انحراف,sqlite3ErrName(rc)));
  واپس rc;
}

/*
** Move the current position of the file handle passed as the first
** argument to offset iOffset within the file. If successful, return 0.
** Otherwise, set pFile->lastErrno and return non-zero.
*/
ساکن صحیح winSeekFile(winFile *pFile, sqlite3_int64 i_انحراف){
  صحیح rc;

  rc = winHandleSeek(pFile->h, i_انحراف);
  اگر( rc!=SQLITE_OK ){
    pFile->آخری_Errno = os_Get_آخری_Error();
    winLogError(rc, pFile->آخری_Errno, "winSeekFile", pFile->z_راستہ);
  }
  واپس rc;
}


#اگر SQLITE_MAX_MMAP_سائز>0
/* Forward references to VFS helper methods used for memory mapped files */
ساکن صحیح winMapfile(winFile*, sqlite3_int64);
ساکن صحیح winUnmapfile(winFile*);
#اگر_ختم

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
#تعریف_کرو MX_CLOSE_ATTEMPT 3
ساکن صحیح winClose(sqlite3_file *id){
  صحیح rc, cnt = 0;
  winFile *pFile = (winFile*)id;

  أكد( id!=0 );
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
  أكد( pFile->pShm==0 );
#اگر_ختم
  أكد( pFile->h!=تہی && pFile->h!=INVALID_HANDLE_قدر );
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p\n",
           osGetCurrentProcessId(), pFile, pFile->h));

#اگر SQLITE_MAX_MMAP_سائز>0
  winUnmapfile(pFile);
#اگر_ختم

  کرو{
    rc = osCloseHandle(pFile->h);
    /* SimulateIOError( rc=0; cnt=MX_CLOSE_ATTEMPT; ); */
  }جب( rc==0 && ++cnt < MX_CLOSE_ATTEMPT && (osSleep(100), 1) );
  اگر( rc ){
    pFile->h = تہی;
  }
  Open_شمار(-1);
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, rc ? "ok" : "failed"));
  واپس rc ? SQLITE_OK
            : winLogError(SQLITE_IOERR_CLOSE, os_Get_آخری_Error(),
                          "winClose", pFile->z_راستہ);
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
ساکن صحیح winRead(
  sqlite3_file *id,          /* File to read from */
  خالی *pBuf,                /* Write content into this buffer */
  صحیح amt,                   /* Number of bytes to read */
  sqlite3_int64 انحراف       /* Begin reading at this offset */
){
#اگر !defined(SQLITE_WIN32_NO_OVERLAPPED)
  OVERLAPPED overlapped;          /* The offset for ReadFile. */
#اگر_ختم
  winFile *pFile = (winFile*)id;  /* file handle */
  DWORD nRead;                    /* Number of bytes actually read from file */
  صحیح nRetry = 0;                 /* Number of retrys */

  أكد( id!=0 );
  أكد( amt>0 );
  أكد( انحراف>=0 );
  SimulateIOError(واپس SQLITE_IOERR_READ);
  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, انحراف, pFile->locktype));

#اگر SQLITE_MAX_MMAP_سائز>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  اگر( انحراف<pFile->mmap_سائز ){
    اگر( انحراف+amt <= pFile->mmap_سائز ){
      حافظہ_نقل(pBuf, &((u8 *)(pFile->pMapRegion))[انحراف], amt);
      OSTRACE(("READ-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      واپس SQLITE_OK;
    }ورنہ{
      صحیح nCopy = (صحیح)(pFile->mmap_سائز - انحراف);
      حافظہ_نقل(pBuf, &((u8 *)(pFile->pMapRegion))[انحراف], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      انحراف += nCopy;
    }
  }
#اگر_ختم

#اگر defined(SQLITE_WIN32_NO_OVERLAPPED)
  اگر( winSeekFile(pFile, انحراف) ){
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    واپس SQLITE_بھرا;
  }
  جب( !osReadFile(pFile->h, pBuf, amt, &nRead, 0) ){
#ورنہ
  memset(&overlapped, 0, حجم(OVERLAPPED));
  overlapped.انحراف_2 = (LONG)(انحراف & 0xffffffff);
  overlapped.انحراف_بلند_2 = (LONG)((انحراف>>32) & 0x7fffffff);
  جب( !osReadFile(pFile->h, pBuf, amt, &nRead, &overlapped) &&
         os_Get_آخری_Error()!=ERROR_HANDLE_EOF ){
#اگر_ختم
    DWORD آخری_Errno;
    اگر( winRetryIoerr(&nRetry, &آخری_Errno) ) جاری;
    pFile->آخری_Errno = آخری_Errno;
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    واپس winLogError(SQLITE_IOERR_READ, pFile->آخری_Errno,
                       "winRead", pFile->z_راستہ);
  }
  winLogIoerr(nRetry, __LINE__);
  اگر( nRead<(DWORD)amt ){
    /* Unread parts of the buffer must be zero-filled */
    memset(&((حرف*)pBuf)[nRead], 0, amt-nRead);
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_SHORT_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    واپس SQLITE_IOERR_SHORT_READ;
  }

  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  واپس SQLITE_OK;
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
ساکن صحیح winWrite(
  sqlite3_file *id,               /* File to write into */
  ثابت خالی *pBuf,               /* The bytes to be written */
  صحیح amt,                        /* Number of bytes to write */
  sqlite3_int64 انحراف            /* Offset into the file to begin writing at */
){
  صحیح rc = 0;                     /* True if error has occurred, else false */
  winFile *pFile = (winFile*)id;  /* File handle */
  صحیح nRetry = 0;                 /* Number of retries */

  أكد( amt>0 );
  أكد( pFile );
  SimulateIOError(واپس SQLITE_IOERR_WRITE);
  SimulateDiskfullError(واپس SQLITE_بھرا);

  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, انحراف, pFile->locktype));

#اگر defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_سائز>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  اگر( انحراف<pFile->mmap_سائز ){
    اگر( انحراف+amt <= pFile->mmap_سائز ){
      حافظہ_نقل(&((u8 *)(pFile->pMapRegion))[انحراف], pBuf, amt);
      OSTRACE(("WRITE-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      واپس SQLITE_OK;
    }ورنہ{
      صحیح nCopy = (صحیح)(pFile->mmap_سائز - انحراف);
      حافظہ_نقل(&((u8 *)(pFile->pMapRegion))[انحراف], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      انحراف += nCopy;
    }
  }
#اگر_ختم

#اگر defined(SQLITE_WIN32_NO_OVERLAPPED)
  rc = winSeekFile(pFile, انحراف);
  اگر( rc==0 ){
#ورنہ
  {
#اگر_ختم
#اگر !defined(SQLITE_WIN32_NO_OVERLAPPED)
    OVERLAPPED overlapped;        /* The offset for WriteFile. */
#اگر_ختم
    u8 *aRem = (u8 *)pBuf;        /* Data yet to be written */
    صحیح nRem = amt;               /* Number of bytes yet to be written */
    DWORD nWrite;                 /* Bytes written by each WriteFile() call */
    DWORD آخری_Errno = NO_ERROR;   /* Value returned by GetLastError() */

#اگر !defined(SQLITE_WIN32_NO_OVERLAPPED)
    memset(&overlapped, 0, حجم(OVERLAPPED));
    overlapped.انحراف_2 = (LONG)(انحراف & 0xffffffff);
    overlapped.انحراف_بلند_2 = (LONG)((انحراف>>32) & 0x7fffffff);
#اگر_ختم

    جب( nRem>0 ){
#اگر defined(SQLITE_WIN32_NO_OVERLAPPED)
      اگر( !osWriteFile(pFile->h, aRem, nRem, &nWrite, 0) ){
#ورنہ
      اگر( !osWriteFile(pFile->h, aRem, nRem, &nWrite, &overlapped) ){
#اگر_ختم
        اگر( winRetryIoerr(&nRetry, &آخری_Errno) ) جاری;
        توڑو;
      }
      أكد( nWrite==0 || nWrite<=(DWORD)nRem );
      اگر( nWrite==0 || nWrite>(DWORD)nRem ){
        آخری_Errno = os_Get_آخری_Error();
        توڑو;
      }
#اگر !defined(SQLITE_WIN32_NO_OVERLAPPED)
      انحراف += nWrite;
      overlapped.انحراف_2 = (LONG)(انحراف & 0xffffffff);
      overlapped.انحراف_بلند_2 = (LONG)((انحراف>>32) & 0x7fffffff);
#اگر_ختم
      aRem += nWrite;
      nRem -= nWrite;
    }
    اگر( nRem>0 ){
      pFile->آخری_Errno = آخری_Errno;
      rc = 1;
    }
  }

  اگر( rc ){
    اگر(   ( pFile->آخری_Errno==ERROR_HANDLE_DISK_بھرا )
       || ( pFile->آخری_Errno==ERROR_DISK_بھرا )){
      OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      واپس winLogError(SQLITE_بھرا, pFile->آخری_Errno,
                         "winWrite1", pFile->z_راستہ);
    }
    OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_WRITE\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    واپس winLogError(SQLITE_IOERR_WRITE, pFile->آخری_Errno,
                       "winWrite2", pFile->z_راستہ);
  }ورنہ{
    winLogIoerr(nRetry, __LINE__);
  }
  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  واپس SQLITE_OK;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
/*
** Truncate the file opened by handle h to nByte bytes in size.
*/
ساکن صحیح winHandleTruncate(HANDLE h, sqlite3_int64 nByte){ 
  صحیح rc = SQLITE_OK;             /* Return code */
  rc = winHandleSeek(h, nByte);
  اگر( rc==SQLITE_OK ){
    اگر( 0==os_سیٹ_انتہا_کا_File(h) ){
      rc = SQLITE_IOERR_TRUNCATE;
    }
  }
  واپس rc;
}

/*
** Determine the size in bytes of the file opened by the handle passed as 
** the first argument.
*/
ساکن صحیح win_Handle_سائز(HANDLE h, sqlite3_int64 *pnByte){ 
  صحیح rc = SQLITE_OK;
  LARGE_INTEGER x;
  أكد( pnByte );
  اگر( os_Get_File_سائز_Ex(h, &x)==0 ){
    rc = SQLITE_IOERR_FSTAT;
    *pnByte = 0;
  }ورنہ{
    *pnByte = x.QuadPart;
  }
  واپس rc;
}

/*
** Close the handle passed as the only argument.
*/
ساکن خالی winHandleClose(HANDLE h){
  اگر( h!=INVALID_HANDLE_قدر ){
    osCloseHandle(h);
  }
}
#اگر_ختم /* #ifndef SQLITE_OMIT_WAL */

/*
** Truncate an open file to a specified size
*/
ساکن صحیح winTruncate(sqlite3_file *id, sqlite3_int64 nByte){
  winFile *pFile = (winFile*)id;  /* File handle object */
  صحیح rc = SQLITE_OK;             /* Return code for this function */
  DWORD آخری_Errno;
#اگر SQLITE_MAX_MMAP_سائز>0
  sqlite3_int64 old_Mmap_سائز;
  اگر( pFile->n_Fetch_برآمد>0 ){
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
    واپس SQLITE_OK;
  }
#اگر_ختم

  أكد( pFile );
  SimulateIOError(واپس SQLITE_IOERR_TRUNCATE);
  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, size=%lld, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, nByte, pFile->locktype));

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  اگر( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

#اگر SQLITE_MAX_MMAP_سائز>0
  اگر( pFile->pMapRegion ){
    old_Mmap_سائز = pFile->mmap_سائز;
  }ورنہ{
    old_Mmap_سائز = 0;
  }
  winUnmapfile(pFile);
#اگر_ختم

  /* SetEndOfFile() returns non-zero when successful, or zero when it fails. */
  اگر( winSeekFile(pFile, nByte) ){
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->آخری_Errno,
                     "winTruncate1", pFile->z_راستہ);
  }ورنہ اگر( 0==os_سیٹ_انتہا_کا_File(pFile->h) &&
            ((آخری_Errno = os_Get_آخری_Error())!=ERROR_USER_MAPPED_FILE) ){
    pFile->آخری_Errno = آخری_Errno;
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->آخری_Errno,
                     "winTruncate2", pFile->z_راستہ);
  }

#اگر SQLITE_MAX_MMAP_سائز>0
  اگر( rc==SQLITE_OK && old_Mmap_سائز>0 ){
    اگر( old_Mmap_سائز>nByte ){
      winMapfile(pFile, -1);
    }ورنہ{
      winMapfile(pFile, old_Mmap_سائز);
    }
  }
#اگر_ختم

  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, sqlite3ErrName(rc)));
  واپس rc;
}

#اگر_تعریف_شدہ SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occurring at the right times.
*/
صحیح sqlite3_sync_گنتی = 0;
صحیح sqlite3_fullsync_گنتی = 0;
#اگر_ختم

/*
** Make sure all writes to a particular file are committed to disk.
*/
ساکن صحیح winSync(sqlite3_file *id, صحیح flags){
#اگر_غیر_تعریف_شدہ SQLITE_NO_SYNC
  /*
  ** Used only when SQLITE_NO_SYNC is not defined.
   */
  BOOL rc;
#اگر_ختم
#اگر !defined(NDEBUG) || !defined(SQLITE_NO_SYNC) || \
    defined(SQLITE_HAVE_OS_TRACE)
  /*
  ** Used when SQLITE_NO_SYNC is not defined and by the assert() and/or
  ** OSTRACE() macros.
   */
  winFile *pFile = (winFile*)id;
#ورنہ
  غیر_مستعمل_PARAMETER(id);
#اگر_ختم

  أكد( pFile );
  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  أكد((flags&0x0F)==SQLITE_SYNC_معمول
      || (flags&0x0F)==SQLITE_SYNC_بھرا
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( واپس SQLITE_بھرا );

  OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, flags=%x, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, flags,
           pFile->locktype));

#اگر_غیر_تعریف_شدہ SQLITE_TEST
  غیر_مستعمل_PARAMETER(flags);
#ورنہ
  اگر( (flags&0x0F)==SQLITE_SYNC_بھرا ){
    sqlite3_fullsync_گنتی++;
  }
  sqlite3_sync_گنتی++;
#اگر_ختم

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op
  */
#اگر_تعریف_شدہ SQLITE_NO_SYNC
  OSTRACE(("SYNC-NOP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  واپس SQLITE_OK;
#ورنہ
#اگر SQLITE_MAX_MMAP_سائز>0
  اگر( pFile->pMapRegion ){
    اگر( os_Flush_View_کا_File(pFile->pMapRegion, 0) ){
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_OK\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
    }ورنہ{
      pFile->آخری_Errno = os_Get_آخری_Error();
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
      واپس winLogError(SQLITE_IOERR_MMAP, pFile->آخری_Errno,
                         "winSync1", pFile->z_راستہ);
    }
  }
#اگر_ختم
  rc = osFlushFileBuffers(pFile->h);
  SimulateIOError( rc=FALSE );
  اگر( rc ){
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    واپس SQLITE_OK;
  }ورنہ{
    pFile->آخری_Errno = os_Get_آخری_Error();
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_FSYNC\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    واپس winLogError(SQLITE_IOERR_FSYNC, pFile->آخری_Errno,
                       "winSync2", pFile->z_راستہ);
  }
#اگر_ختم
}

/*
** Determine the current size of a file in bytes
*/
ساکن صحیح win_File_سائز(sqlite3_file *id, sqlite3_int64 *p_سائز){
  winFile *pFile = (winFile*)id;
  صحیح rc = SQLITE_OK;

  أكد( id!=0 );
  أكد( p_سائز!=0 );
  SimulateIOError(واپس SQLITE_IOERR_FSTAT);
  OSTRACE(("SIZE file=%p, pSize=%p\n", pFile->h, p_سائز));
  {
    LARGE_INTEGER x;
    اگر( os_Get_File_سائز_Ex(pFile->h, &x)==0 ){
      *p_سائز = 0;
      pFile->آخری_Errno = os_Get_آخری_Error();
      rc = winLogError(SQLITE_IOERR_FSTAT, pFile->آخری_Errno,
                       "winFileSize", pFile->z_راستہ);
    }ورنہ{
      *p_سائز = x.QuadPart;
    }
  }
  OSTRACE(("SIZE file=%p, pSize=%p, *pSize=%lld, rc=%s\n",
           pFile->h, p_سائز, *p_سائز, sqlite3ErrName(rc)));
  واپس rc;
}

/*
** LOCKFILE_FAIL_IMMEDIATELY is undefined on some Windows systems.
*/
#اگر_غیر_تعریف_شدہ LOCKFILE_FAIL_IMMEDIATELY
# define LOCKFILE_FAIL_IMMEDIATELY 1
#اگر_ختم

#اگر_غیر_تعریف_شدہ LOCKFILE_EXCLUSIVE_تالا
# define LOCKFILE_EXCLUSIVE_تالا 2
#اگر_ختم

/*
** Historically, SQLite has used both the LockFile and LockFileEx functions.
** When the LockFile function was used, it was always expected to fail
** immediately if the lock could not be obtained.  Also, it always expected to
** obtain an exclusive lock.  These flags are used with the LockFileEx function
** and reflect those expectations; therefore, they should not be changed.
*/
#اگر_غیر_تعریف_شدہ SQLITE_LOCKFILE_FLAGS
# define SQLITE_LOCKFILE_FLAGS   (LOCKFILE_FAIL_IMMEDIATELY | \
                                  LOCKFILE_EXCLUSIVE_تالا)
#اگر_ختم

/*
** Currently, SQLite never calls the LockFileEx function without wanting the
** call to fail immediately if the lock cannot be obtained.
*/
#اگر_غیر_تعریف_شدہ SQLITE_LOCKFILEEX_FLAGS
# define SQLITE_LOCKFILEEX_FLAGS (LOCKFILE_FAIL_IMMEDIATELY)
#اگر_ختم

/*
** Acquire a reader lock.
** Different API routines are called depending on whether or not this
** is Win9x or WinNT.
*/
ساکن صحیح win_Get_Read_تالا(winFile *pFile, صحیح b_بلاک){
  صحیح res;
  DWORD نقاب = ~(b_بلاک ? LOCKFILE_FAIL_IMMEDIATELY : 0);
  OSTRACE(("READ-LOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = win_تالا_File(&pFile->h, SQLITE_LOCKFILEEX_FLAGS&نقاب, SHARED_پہلا, 0,
                    SHARED_سائز, 0);
  اگر( res == 0 ){
    pFile->آخری_Errno = os_Get_آخری_Error();
    /* No need to log a failure to lock */
  }
  OSTRACE(("READ-LOCK file=%p, result=%d\n", pFile->h, res));
  واپس res;
}

/*
** Undo a readlock
*/
ساکن صحیح win_Unlock_Read_تالا(winFile *pFile){
  صحیح res;
  DWORD آخری_Errno;
  OSTRACE(("READ-UNLOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winUnlockFile(&pFile->h, SHARED_پہلا, 0, SHARED_سائز, 0);
  اگر( res==0 && ((آخری_Errno = os_Get_آخری_Error())!=ERROR_NOT_LOCKED) ){
    pFile->آخری_Errno = آخری_Errno;
    winLogError(SQLITE_IOERR_UNLOCK, pFile->آخری_Errno,
                "winUnlockReadLock", pFile->z_راستہ);
  }
  OSTRACE(("READ-UNLOCK file=%p, result=%d\n", pFile->h, res));
  واپس res;
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
ساکن صحیح win_تالا(sqlite3_file *id, صحیح locktype){
  صحیح rc = SQLITE_OK;    /* Return code from subroutines */
  صحیح res = 1;           /* Result of a Windows lock call */
  صحیح newLocktype;       /* Set pFile->locktype to this value before exiting */
  صحیح got_Pending_تالا = 0;/* True if we acquired a PENDING lock this time */
  winFile *pFile = (winFile*)id;
  DWORD آخری_Errno = NO_ERROR;

  أكد( id!=0 );
  OSTRACE(("LOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->shared_تالا_Byte, locktype));

  /* If there is already a lock of this type or more restrictive on the
  ** OsFile, do nothing. Don't use the end_lock: exit path, as
  ** sqlite3OsEnterMutex() hasn't been called yet.
  */
  اگر( pFile->locktype>=locktype ){
    OSTRACE(("LOCK-HELD file=%p, rc=SQLITE_OK\n", pFile->h));
    واپس SQLITE_OK;
  }

  /* Do not allow any kind of write-lock on a read-only database
  */
  اگر( (pFile->ctrlFlags & WINFILE_RDONLY)!=0 && locktype>=RESERVED_تالا ){
    واپس SQLITE_IOERR_تالا;
  }

  /* Make sure the locking sequence is correct
  */
  أكد( pFile->locktype!=NO_تالا || locktype==SHARED_تالا );
  أكد( locktype!=PENDING_تالا );
  أكد( locktype!=RESERVED_تالا || pFile->locktype==SHARED_تالا );

  /* Lock the PENDING_LOCK byte if we need to acquire an EXCLUSIVE lock or
  ** a SHARED lock.  If we are acquiring a SHARED lock, the acquisition of
  ** the PENDING_LOCK byte is temporary.
  */
  newLocktype = pFile->locktype;
  اگر( locktype==SHARED_تالا
   || (locktype==EXCLUSIVE_تالا && pFile->locktype==RESERVED_تالا)
  ){
    صحیح cnt = 3;

    /* Flags for the LockFileEx() call. This should be an exclusive lock if
    ** this call is to obtain EXCLUSIVE, or a shared lock if this call is to
    ** obtain SHARED.  */
    صحیح flags = LOCKFILE_FAIL_IMMEDIATELY;
    اگر( locktype==EXCLUSIVE_تالا ){
      flags |= LOCKFILE_EXCLUSIVE_تالا;
    }
    جب( cnt>0 ){
      /* Try 3 times to get the pending lock.  This is needed to work
      ** around problems caused by indexing and/or anti-virus software on
      ** Windows systems.
      **
      ** If you are using this code as a model for alternative VFSes, do not
      ** copy this retry logic.  It is a hack intended for Windows only.  */
      res = win_تالا_File(&pFile->h, flags, PENDING_BYTE, 0, 1, 0);
      اگر( res ) توڑو;

      آخری_Errno = os_Get_آخری_Error();
      OSTRACE(("LOCK-PENDING-FAIL file=%p, count=%d, result=%d\n", 
            pFile->h, cnt, res
      ));

      اگر( آخری_Errno==ERROR_INVALID_HANDLE ){
        pFile->آخری_Errno = آخری_Errno;
        rc = SQLITE_IOERR_تالا;
        OSTRACE(("LOCK-FAIL file=%p, count=%d, rc=%s\n", 
              pFile->h, cnt, sqlite3ErrName(rc)
        ));
        واپس rc;
      }

      cnt--;
      اگر( cnt>0 ) osSleep(1);
    }
    got_Pending_تالا = res;
  }

  /* Acquire a shared lock
  */
  اگر( locktype==SHARED_تالا && res ){
    أكد( pFile->locktype==NO_تالا );
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
    res = win_Get_Read_تالا(pFile, pFile->b_بلاک_On_Connect);
#ورنہ
    res = win_Get_Read_تالا(pFile, 0);
#اگر_ختم
    اگر( res ){
      newLocktype = SHARED_تالا;
    }ورنہ{
      آخری_Errno = os_Get_آخری_Error();
    }
  }

  /* Acquire a RESERVED lock
  */
  اگر( locktype==RESERVED_تالا && res ){
    أكد( pFile->locktype==SHARED_تالا );
    res = win_تالا_File(&pFile->h, SQLITE_LOCKFILE_FLAGS, RESERVED_BYTE, 0, 1, 0);
    اگر( res ){
      newLocktype = RESERVED_تالا;
    }ورنہ{
      آخری_Errno = os_Get_آخری_Error();
    }
  }

  /* Acquire a PENDING lock
  */
  اگر( locktype==EXCLUSIVE_تالا && res ){
    newLocktype = PENDING_تالا;
    got_Pending_تالا = 0;
  }

  /* Acquire an EXCLUSIVE lock
  */
  اگر( locktype==EXCLUSIVE_تالا && res ){
    أكد( pFile->locktype>=SHARED_تالا );
    (خالی)win_Unlock_Read_تالا(pFile);
    res = win_تالا_File(&pFile->h, SQLITE_LOCKFILE_FLAGS, SHARED_پہلا, 0,
                      SHARED_سائز, 0);
    اگر( res ){
      newLocktype = EXCLUSIVE_تالا;
    }ورنہ{
      آخری_Errno = os_Get_آخری_Error();
      win_Get_Read_تالا(pFile, 0);
    }
  }

  /* If we are holding a PENDING lock that ought to be released, then
  ** release it now.
  */
  اگر( got_Pending_تالا && locktype==SHARED_تالا ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }

  /* Update the state of the lock has held in the file descriptor then
  ** return the appropriate result code.
  */
  اگر( res ){
    rc = SQLITE_OK;
  }ورنہ{
    pFile->آخری_Errno = آخری_Errno;
    rc = SQLITE_BUSY;
    OSTRACE(("LOCK-FAIL file=%p, wanted=%d, got=%d\n",
             pFile->h, locktype, newLocktype));
  }
  pFile->locktype = (u8)newLocktype;
  OSTRACE(("LOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  واپس rc;
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, return
** non-zero, otherwise zero.
*/
ساکن صحیح win_Check_Reserved_تالا(sqlite3_file *id, صحیح *p_Res_برآمد){
  صحیح res;
  winFile *pFile = (winFile*)id;

  SimulateIOError( واپس SQLITE_IOERR_CHECKRESERVEDLOCK; );
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p\n", pFile->h, p_Res_برآمد));

  أكد( id!=0 );
  اگر( pFile->locktype>=RESERVED_تالا ){
    res = 1;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (local)\n", pFile->h, res));
  }ورنہ{
    res = win_تالا_File(&pFile->h, SQLITE_LOCKFILEEX_FLAGS,RESERVED_BYTE,0,1,0);
    اگر( res ){
      winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
    }
    res = !res;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (remote)\n", pFile->h, res));
  }
  *p_Res_برآمد = res;
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           pFile->h, p_Res_برآمد, *p_Res_برآمد));
  واپس SQLITE_OK;
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
ساکن صحیح winUnlock(sqlite3_file *id, صحیح locktype){
  صحیح type;
  winFile *pFile = (winFile*)id;
  صحیح rc = SQLITE_OK;
  أكد( pFile!=0 );
  أكد( locktype<=SHARED_تالا );
  OSTRACE(("UNLOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->shared_تالا_Byte, locktype));
  type = pFile->locktype;
  اگر( type>=EXCLUSIVE_تالا ){
    winUnlockFile(&pFile->h, SHARED_پہلا, 0, SHARED_سائز, 0);
    اگر( locktype==SHARED_تالا && !win_Get_Read_تالا(pFile, 0) ){
      /* This should never happen.  We should always be able to
      ** reacquire the read lock */
      rc = winLogError(SQLITE_IOERR_UNLOCK, os_Get_آخری_Error(),
                       "winUnlock", pFile->z_راستہ);
    }
  }
  اگر( type>=RESERVED_تالا ){
    winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
  }
  اگر( locktype==NO_تالا && type>=SHARED_تالا ){
    win_Unlock_Read_تالا(pFile);
  }
  اگر( type>=PENDING_تالا ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }
  pFile->locktype = (u8)locktype;
  OSTRACE(("UNLOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  واپس rc;
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

ساکن صحیح win_Nolock_تالا(sqlite3_file *id, صحیح locktype){
  غیر_مستعمل_PARAMETER(id);
  غیر_مستعمل_PARAMETER(locktype);
  واپس SQLITE_OK;
}

ساکن صحیح win_Nolock_Check_Reserved_تالا(sqlite3_file *id, صحیح *p_Res_برآمد){
  غیر_مستعمل_PARAMETER(id);
  غیر_مستعمل_PARAMETER(p_Res_برآمد);
  واپس SQLITE_OK;
}

ساکن صحیح winNolockUnlock(sqlite3_file *id, صحیح locktype){
  غیر_مستعمل_PARAMETER(id);
  غیر_مستعمل_PARAMETER(locktype);
  واپس SQLITE_OK;
}

/******************* End of the no-op lock implementation *********************
******************************************************************************/

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
ساکن خالی win_منوال_ثنائی_ہندسہ(winFile *pFile, غیرعلامتی حرف نقاب, صحیح *pArg){
  اگر( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & نقاب)!=0;
  }ورنہ اگر( (*pArg)==0 ){
    pFile->ctrlFlags &= ~نقاب;
  }ورنہ{
    pFile->ctrlFlags |= نقاب;
  }
}

/* Forward references to VFS helper methods used for temporary files */
ساکن صحیح winGetTempname(sqlite3_vfs *, حرف **);
ساکن صحیح winIsDir(LPCWSTR);
ساکن BOOL win_Is_Long_راستہ_سابقہ(ثابت حرف *);
ساکن BOOL winIsDriveLetterAndColon(ثابت حرف *);

/*
** Control and query of the open file handle.
*/
ساکن صحیح winFileControl(sqlite3_file *id, صحیح op, خالی *pArg){
  winFile *pFile = (winFile*)id;
  OSTRACE(("FCNTL file=%p, op=%d, pArg=%p\n", pFile->h, op, pArg));
  انتخاب( op ){
    حالت SQLITE_FCNTL_LOCKSTATE: {
      *(صحیح*)pArg = pFile->locktype;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_آخری_ERRNO: {
      *(صحیح*)pArg = (صحیح)pFile->آخری_Errno;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_CHUNK_سائز: {
      pFile->szChunk = *(صحیح *)pArg;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_سائز_HINT: {
      اگر( pFile->szChunk>0 ){
        sqlite3_int64 oldSz;
        صحیح rc = win_File_سائز(id, &oldSz);
        اگر( rc==SQLITE_OK ){
          sqlite3_int64 newSz = *(sqlite3_int64*)pArg;
          اگر( newSz>oldSz ){
            SimulateIOErrorBenign(1);
            rc = winTruncate(id, newSz);
            SimulateIOErrorBenign(0);
          }
        }
        OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
        واپس rc;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_PERSIST_WAL: {
      win_منوال_ثنائی_ہندسہ(pFile, WINFILE_PERSIST_WAL, (صحیح*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      win_منوال_ثنائی_ہندسہ(pFile, WINFILE_PSOW, (صحیح*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_VFSNAME: {
      *(حرف**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_WIN32_AV_RETRY: {
      صحیح *a = (صحیح*)pArg;
      اگر( a[0]>0 ){
        winIoerrRetry = a[0];
      }ورنہ{
        a[0] = winIoerrRetry;
      }
      اگر( a[1]>0 ){
        winIoerrRetryDelay = a[1];
      }ورنہ{
        a[1] = winIoerrRetryDelay;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_WIN32_GET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      *phFile = pFile->h;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      واپس SQLITE_OK;
    }
#اگر_تعریف_شدہ SQLITE_TEST
    حالت SQLITE_FCNTL_WIN32_سیٹ_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      HANDLE hOldFile = pFile->h;
      pFile->h = *phFile;
      *phFile = hOldFile;
      OSTRACE(("FCNTL oldFile=%p, newFile=%p, rc=SQLITE_OK\n",
               hOldFile, pFile->h));
      واپس SQLITE_OK;
    }
#اگر_ختم
    حالت SQLITE_FCNTL_NULL_IO: {
      (خالی)osCloseHandle(pFile->h);
      pFile->h = تہی;
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_TEMPFILENAME: {
      حرف *zTFile = 0;
      صحیح rc = winGetTempname(pFile->pVfs, &zTFile);
      اگر( rc==SQLITE_OK ){
        *(حرف**)pArg = zTFile;
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      واپس rc;
    }
#اگر SQLITE_MAX_MMAP_سائز>0
    حالت SQLITE_FCNTL_MMAP_سائز: {
      i64 new_حد_بندی = *(i64*)pArg;
      صحیح rc = SQLITE_OK;
      اگر( new_حد_بندی>sqlite3GlobalConfig.mxMmap ){
        new_حد_بندی = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (SIZE_T) and passed
      ** to MapViewOfFile(). Restrict its value to 2GB if (SIZE_T) is not at
      ** least a 64-bit type. */
      اگر( new_حد_بندی>0 && حجم(سائز_T)<8 ){
        new_حد_بندی = (new_حد_بندی & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmap_سائز_Max;
      اگر( new_حد_بندی>=0 && new_حد_بندی!=pFile->mmap_سائز_Max && pFile->n_Fetch_برآمد==0 ){
        pFile->mmap_سائز_Max = new_حد_بندی;
        اگر( pFile->mmap_سائز>0 ){
          winUnmapfile(pFile);
          rc = winMapfile(pFile, -1);
        }
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      واپس rc;
    }
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
    حالت SQLITE_FCNTL_تالا_TIMEOUT: {
      صحیح iOld = pFile->iBusyTimeout;
      صحیح iNew = *(صحیح*)pArg;
#اگر SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = (iNew < 0) ? INFINITE : (DWORD)iNew;
#وگرنہ_اگر SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = (DWORD)(!!iNew);
#ورنہ
# error "SQLITE_ENABLE_SETLK_TIMEOUT must be set to 1 or 2"
#اگر_ختم
      *(صحیح*)pArg = iOld;
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_بلاک_ON_CONNECT: {
      صحیح iNew = *(صحیح*)pArg;
      pFile->b_بلاک_On_Connect = iNew;
      واپس SQLITE_OK;
    }
#اگر_ختم /* SQLITE_ENABLE_SETLK_TIMEOUT */

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    حالت SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      sqlite3_str_appendf(pStr, "{\"h\":%llu", (sqlite3_uint64)pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      اگر( pFile->locktype ){
        ساکن ثابت حرف *az_تالا[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
        sqlite3_str_appendf(pStr, ",\"locktype\":\"%s\"", 
                                  az_تالا[pFile->locktype-1]);
      }
#اگر SQLITE_MAX_MMAP_سائز>0
      اگر( pFile->mmap_سائز ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmap_سائز);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->n_Fetch_برآمد);
      }
#اگر_ختم
      sqlite3_str_append(pStr, "}", 1);
      واپس SQLITE_OK;
    }
#اگر_ختم /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

  }
  OSTRACE(("FCNTL file=%p, rc=SQLITE_NOTFOUND\n", pFile->h));
  واپس SQLITE_NOTFOUND;
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
ساکن صحیح win_Sector_سائز(sqlite3_file *id){
  (خالی)id;
  واپس SQLITE_DEFAULT_SECTOR_سائز;
}

/*
** Return a vector of device characteristics.
*/
ساکن صحیح winDeviceCharacteristics(sqlite3_file *id){
  winFile *p = (winFile*)id;
  واپس SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN | SQLITE_IOCAP_SUBPAGE_READ |
         ((p->ctrlFlags & WINFILE_PSOW)?SQLITE_IOCAP_POWERSAFE_OVERWRITE:0);
}

/*
** Windows will only let you create file view mappings
** on allocation size granularity boundaries.
** During sqlite3_os_init() we do a GetSystemInfo()
** to get the granularity size.
*/
ساکن SYSTEM_INFO winSysInfo;

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
ساکن LPWSTR winConvertFromUtf8Filename(ثابت حرف *zFilename){
  LPWSTR zConverted = 0;
#اگر_تعریف_شدہ __CYGWIN__
  صحیح nChar;

  اگر( os_Cygwin_conv_راستہ && !(winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2])) ){
    i64 nByte;
    صحیح convertflag = CCP_POSIX_مقصد_WIN_W;
    اگر( !strchr(zFilename, '/') ) convertflag |= CCP_RELATIVE;
    nByte = (i64)os_Cygwin_conv_راستہ(convertflag,
        zFilename, 0, 0);
    اگر( nByte>0 ){
      zConverted = sqlite3MallocZero(12+(u64)nByte);
      اگر ( zConverted==0 ){
        واپس zConverted;
      }
      /* Filenames should be prefixed, except when converted
       * full path already starts with "\\?\". */
      اگر( os_Cygwin_conv_راستہ(convertflag, zFilename,
                           zConverted+4, nByte)==0 ){
        اگر( (convertflag&CCP_RELATIVE) ){
          memmove(zConverted, zConverted+4, nByte);
        }ورنہ اگر( memcmp(zConverted+4, L"\\\\", 4) ){
          حافظہ_نقل(zConverted, L"\\\\?\\", 8);
        }ورنہ اگر( zConverted[6]!='?' ){
          memmove(zConverted+6, zConverted+4, nByte);
          حافظہ_نقل(zConverted, L"\\\\?\\UNC", 14);
        }ورنہ{
          memmove(zConverted, zConverted+4, nByte);
        }
        واپس zConverted;
      }
      sqlite3_free(zConverted);
    }
  }
  nChar = os_Multi_Byte_مقصد_Wide_Char(CP_UTF8, 0, zFilename, -1, تہی, 0);
  اگر( nChar==0 ){
    واپس 0;
  }
  zConverted = sqlite3MallocZero( ((u64)nChar+6)*حجم(WCHAR) );
  اگر( zConverted==0 ){
    واپس 0;
  }
  nChar = os_Multi_Byte_مقصد_Wide_Char(CP_UTF8, 0, zFilename, -1,
                                zConverted, nChar);
  اگر( nChar==0 ){
    sqlite3_free(zConverted);
    zConverted = 0;
  }ورنہ اگر( nChar>MAX_راستہ
      && winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2]) ){
    memmove(zConverted+4, zConverted, nChar*حجم(WCHAR));
    zConverted[2] = '\\';
    حافظہ_نقل(zConverted, L"\\\\?\\", 8);
  }ورنہ اگر( nChar>MAX_راستہ
      && winIsDirSep(zFilename[0]) && winIsDirSep(zFilename[1])
      && zFilename[2] != '?' ){
    memmove(zConverted+6, zConverted, nChar*حجم(WCHAR));
    حافظہ_نقل(zConverted, L"\\\\?\\UNC", 14);
  }
#ورنہ /* if !defined(__CYGWIN__) */
  zConverted = win_Utf8_مقصد_Unicode(zFilename);
#اگر_ختم /* __CYGWIN__ */
  /* caller will handle out of memory */
  واپس zConverted;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL

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
ساکن sqlite3_mutex *win_Big_تالا = 0;
ساکن خالی winShmEnterMutex(خالی){
  sqlite3_mutex_enter(win_Big_تالا);
}
ساکن خالی winShmLeaveMutex(خالی){
  sqlite3_mutex_leave(win_Big_تالا);
}
#اگر_غیر_تعریف_شدہ NDEBUG
ساکن صحیح winShmMutexHeld(خالی) {
  واپس sqlite3_mutex_held(win_Big_تالا);
}
#اگر_ختم

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
ساخت win_Shm_گرہ {
  sqlite3_mutex *mutex;      /* Mutex to access this object */
  حرف *zFilename;           /* Name of the file */
  HANDLE hSharedShm;         /* File handle open on zFilename */
  صحیح b_Use_Shared_تالا_Handle;  /* True to use hSharedShm for everything */

  صحیح isUnlocked;            /* DMS lock has not yet been obtained */
  صحیح isReadonly;            /* True if read-only */
  صحیح szRegion;              /* Size of shared-memory regions */
  صحیح nRegion;               /* Size of array apRegion */

  ساخت ShmRegion {
    HANDLE hMap;             /* File handle from CreateFileMapping */
    خالی *pMap;
  } *aRegion;
  DWORD آخری_Errno;           /* The Windows errno from the last I/O error */

  winShm *pWinShmList;       /* List of winShm objects with ptrs to this */

  win_Shm_گرہ *p_اگلا;         /* Next in list of all winShmNode objects */
#اگر defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 اگلا_Shm_Id;              /* Next available winShm.id value */
#اگر_ختم
};

/*
** A global array of all winShmNode objects.
**
** The winShmMutexHeld() must be true while reading or writing this list.
*/
ساکن win_Shm_گرہ *win_Shm_گرہ_List = 0;

/*
** Structure used internally by this VFS to record the state of an
** open shared memory connection. There is one such structure for each
** winFile open on a wal mode database.
*/
ساخت winShm {
  win_Shm_گرہ *p_Shm_گرہ;      /* The underlying winShmNode object */
  u16 shared_نقاب;            /* Mask of shared locks held */
  u16 excl_نقاب;              /* Mask of exclusive locks held */
  HANDLE hShm;               /* File-handle on *-shm file. For locking. */
  صحیح bReadonly;             /* True if hShm is opened read-only */
#اگر defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 id;                     /* Id of this connection with its winShmNode */
#اگر_ختم
  winShm *p_Win_Shm_اگلا;       /* Next winShm object on same winShmNode */
};

/*
** Constants used for locking
*/
#تعریف_کرو WIN_SHM_بنیاد   ((22+SQLITE_SHM_NLOCK)*4)        /* first lock byte */
#تعریف_کرو WIN_SHM_DMS    (WIN_SHM_بنیاد+SQLITE_SHM_NLOCK)  /* deadman switch */

/* Forward references to VFS methods */
ساکن صحیح winOpen(sqlite3_vfs*,ثابت حرف*,sqlite3_file*,صحیح,صحیح*);
ساکن صحیح winDelete(sqlite3_vfs *,ثابت حرف*,صحیح);

/*
** Purge the winShmNodeList list of all entries with winShmNode.pWinShmList==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
ساکن خالی winShmPurge(sqlite3_vfs *pVfs, صحیح deleteFlag){
  win_Shm_گرہ **pp;
  win_Shm_گرہ *p;
  أكد( winShmMutexHeld() );
  OSTRACE(("SHM-PURGE pid=%lu, deleteFlag=%d\n",
           osGetCurrentProcessId(), deleteFlag));
  pp = &win_Shm_گرہ_List;
  جب( (p = *pp)!=0 ){
    اگر( p->pWinShmList==0 ){
      صحیح i;
      اگر( p->mutex ){ sqlite3_mutex_free(p->mutex); }
      برائے(i=0; i<p->nRegion; i++){
        BOOL bRc = os_Unmap_View_کا_File(p->aRegion[i].pMap);
        OSTRACE(("SHM-PURGE-UNMAP pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        غیر_مستعمل_VARIABLE_قدر(bRc);
        bRc = osCloseHandle(p->aRegion[i].hMap);
        OSTRACE(("SHM-PURGE-CLOSE pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        غیر_مستعمل_VARIABLE_قدر(bRc);
      }
      winHandleClose(p->hSharedShm);
      اگر( deleteFlag ){
        SimulateIOErrorBenign(1);
        sqlite3BeginBenignMalloc();
        winDelete(pVfs, p->zFilename, 0);
        sqlite3_انتہا_Benign_Malloc();
        SimulateIOErrorBenign(0);
      }
      *pp = p->p_اگلا;
      sqlite3_free(p->aRegion);
      sqlite3_free(p);
    }ورنہ{
      pp = &p->p_اگلا;
    }
  }
}

/*
** Handle h is open on the *-shm file. This function zeroes the first
** 8 bytes of the file. If SQLITE_ENABLE_SETLK_TIMEOUT is defined, h may 
** have been opened with FILE_FLAG_OVERLAPPED.
*/
ساکن صحیح win_Zero_Shared_حافظہ(HANDLE h){
  u8 aZero[8] = {0,0,0,0,0,0,0,0};
  DWORD nWrite = 0;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, حجم(OVERLAPPED));
  اگر( !osWriteFile(h, aZero, حجم(aZero), &nWrite, &ovlp) ){
    /* If the file was opened with FILE_FLAG_OVERLAPPED, the write may still
    ** be pending. Wait for it with GetOverlappedResult().  */
    اگر( os_Get_آخری_Error()!=ERROR_IO_PENDING
     || !Get_Overlapped_نتیجہ(h, &ovlp, &nWrite, TRUE)
    ){
      nWrite = 0;
    }
  }
  واپس (nWrite==حجم(aZero) ? SQLITE_OK : SQLITE_IOERR_WRITE);
}

/*
** The DMS lock has not yet been taken on the shm file associated with
** pShmNode. Take the lock. Truncate the *-shm file if required.
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
ساکن صحیح win_تالا_Shared_حافظہ(win_Shm_گرہ *p_Shm_گرہ, DWORD nMs){
  HANDLE h = p_Shm_گرہ->hSharedShm;
  صحیح rc = SQLITE_OK;

  أكد( sqlite3_mutex_held(p_Shm_گرہ->mutex) );
  rc = win_Handle_تالا_Timeout(h, WIN_SHM_DMS, 1, 1, 0);
  اگر( rc==SQLITE_OK ){
    /* We have an EXCLUSIVE lock on the DMS byte. This means that this
    ** is the first process to open the file. Truncate it to zero bytes
    ** in this case.  */
    اگر( p_Shm_گرہ->isReadonly ){
      rc = SQLITE_READONLY_CANTINIT;
    }ورنہ{
      اگر( SQLITE_OK!=winHandleTruncate(h, 0) ){
        /* Sometimes the truncate operation may fail because some other
        ** process is still holding an open mapping. So try to write
        ** zeroes into the start of the file instead. */
        rc = win_Zero_Shared_حافظہ(h);
      }
    }

    /* Release the EXCLUSIVE lock acquired above. */
    winUnlockFile(&h, WIN_SHM_DMS, 0, 1, 0);
  }ورنہ اگر( (rc & 0xFF)==SQLITE_BUSY ){
    rc = SQLITE_OK;
  }

  اگر( rc==SQLITE_OK ){
    /* Take a SHARED lock on the DMS byte. */
    rc = win_Handle_تالا_Timeout(h, WIN_SHM_DMS, 1, 0, nMs);
    اگر( rc==SQLITE_OK ){
      p_Shm_گرہ->isUnlocked = 0;
    }
  }

  واپس rc;
}


/*
** This function is used to open a handle on a *-shm file.
**
** If SQLITE_ENABLE_SETLK_TIMEOUT is defined at build time, then the file
** is opened with FILE_FLAG_OVERLAPPED specified. If not, it is not.
*/
ساکن صحیح winHandleOpen(
  ثابت حرف *zUtf8,              /* File to open */
  صحیح *pbReadonly,                /* IN/OUT: True for readonly handle */
  HANDLE *ph                      /* OUT: New HANDLE for file */
){
  صحیح rc = SQLITE_OK;
  LPWSTR zConverted = 0;
  صحیح bReadonly = *pbReadonly;
  HANDLE h = INVALID_HANDLE_قدر;

#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
  ثابت DWORD flag_overlapped = FILE_FLAG_OVERLAPPED;
#ورنہ
  ثابت DWORD flag_overlapped = 0;
#اگر_ختم

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8);
  اگر( zConverted==0 ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8));
    rc = SQLITE_IOERR_NOMEM_BKPT;
    جاؤ winopenfile_برآمد;
  }

  /* Ensure the file we are trying to open is not actually a directory. */
  اگر( winIsDir(zConverted) ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8));
    rc = SQLITE_CANTOPEN_ISDIR;
    جاؤ winopenfile_برآمد;
  }

  /* TODO: platforms.
  ** TODO: retry-on-ioerr.
  */
  h = osCreateFileW(zConverted,         /* lpFileName */
      (GENERIC_READ | (bReadonly ? 0 : GENERIC_WRITE)),  /* dwDesiredAccess */
      FILE_SHARE_READ | FILE_SHARE_WRITE,        /* dwShareMode */
      تہی,                                      /* lpSecurityAttributes */
      OPEN_ALWAYS,                               /* dwCreationDisposition */
      FILE_ATTRIBUTE_معمول|flag_overlapped,
      تہی
  );

  اگر( h==INVALID_HANDLE_قدر ){
    اگر( bReadonly==0 ){
      bReadonly = 1;
      rc = winHandleOpen(zUtf8, &bReadonly, &h);
    }ورنہ{
      rc = SQLITE_CANTOPEN_BKPT;
    }
  }

 winopenfile_برآمد:
  sqlite3_free(zConverted);
  *pbReadonly = bReadonly;
  *ph = h;
  واپس rc;
}
 
/*
** Close pDbFd's connection to shared-memory.  Delete the underlying
** *-shm file if deleteFlag is true.
*/
ساکن صحیح win_Close_Shared_حافظہ(winFile *pDbFd, صحیح deleteFlag){
  winShm *p;            /* The connection to be closed */
  winShm **pp;          /* Iterator for pShmNode->pWinShmList */
  win_Shm_گرہ *p_Shm_گرہ; /* The underlying shared-memory file */

  p = pDbFd->pShm;
  اگر( p==0 ) واپس SQLITE_OK;
  اگر( p->hShm!=INVALID_HANDLE_قدر ){
    osCloseHandle(p->hShm);
  }

  winShmEnterMutex();
  p_Shm_گرہ = p->p_Shm_گرہ;

  /* Remove this connection from the winShmNode.pWinShmList list */
  sqlite3_mutex_enter(p_Shm_گرہ->mutex);
  برائے(pp=&p_Shm_گرہ->pWinShmList; *pp!=p; pp=&(*pp)->p_Win_Shm_اگلا){}
  *pp = p->p_Win_Shm_اگلا;
  sqlite3_mutex_leave(p_Shm_گرہ->mutex);

  winShmPurge(pDbFd->pVfs, deleteFlag);
  winShmLeaveMutex();

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  واپس SQLITE_OK;
}

/*
** testfixture builds may set this global variable to true via a
** Tcl interface. This forces the VFS to use the locking normally
** only used for UNC paths for all files.
*/
#اگر_تعریف_شدہ SQLITE_TEST
صحیح sqlite3_win_test_unc_locking = 0;
#ورنہ
# define sqlite3_win_test_unc_locking 0
#اگر_ختم

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
ساکن صحیح winIsUNCPath(ثابت حرف *zFile){
  اگر( zFile[0]=='\\' && zFile[1]=='\\' ){
    اگر( zFile[2]=='?'
     && zFile[3]=='\\'
     && sqlite3Isalpha(zFile[4])
     && zFile[5]==':'
     && winIsDirSep(zFile[6])
    ){
      واپس sqlite3_win_test_unc_locking;
    }ورنہ{
      واپس 1;
    }
  }
  واپس sqlite3_win_test_unc_locking;
}

/*
** Return true if the string passed in is the name of a file on a
** remote (network-mounted) volume.
*/
ساکن صحیح winIsRemoteVolume(ثابت حرف *zFile){
  WCHAR z_جڑ[4];
  اگر( لڑی_موازنہ(zFile, "\\\\?\\",4)==0 ) zFile += 4;
  اگر( !sqlite3Isalpha(zFile[0]) ) واپس 0;
  اگر( zFile[1]!=':' ) واپس 0;
  اگر( !winIsDirSep(zFile[2]) ) واپس 0;
  z_جڑ[0] = (WCHAR)zFile[0];
  z_جڑ[1] = ':';
  z_جڑ[2] = '\\';
  z_جڑ[3] = 0;
  واپس osGetDriveTypeW(z_جڑ)==DRIVE_REMOTE;
}

/*
** Open the shared-memory area associated with database file pDbFd.
*/
ساکن صحیح win_Open_Shared_حافظہ(winFile *pDbFd){
  ساخت winShm *p;                  /* The connection to be opened */
  win_Shm_گرہ *p_Shm_گرہ = 0;          /* The underlying mmapped file */
  صحیح rc = SQLITE_OK;                /* Result code */
  win_Shm_گرہ *pNew;                  /* Newly allocated winShmNode */
  صحیح nName;                         /* Size of zName in bytes */

  أكد( pDbFd->pShm==0 );    /* Not previously opened */

  /* Allocate space for the new sqlite3_shm object.  Also speculatively
  ** allocate space for a new winShmNode and filename.  */
  p = sqlite3MallocZero( حجم(*p) );
  اگر( p==0 ) واپس SQLITE_IOERR_NOMEM_BKPT;
  nName = sqlite3Strlen30(pDbFd->z_راستہ);
  pNew = sqlite3MallocZero( حجم(*p_Shm_گرہ) + (i64)nName + 17 );
  اگر( pNew==0 ){
    sqlite3_free(p);
    واپس SQLITE_IOERR_NOMEM_BKPT;
  }
  pNew->zFilename = (حرف*)&pNew[1];
  pNew->hSharedShm = INVALID_HANDLE_قدر;
  pNew->isUnlocked = 1;
  اگر( winIsUNCPath(pDbFd->z_راستہ) || winIsRemoteVolume(pDbFd->z_راستہ) ){
    pNew->b_Use_Shared_تالا_Handle = 1;
  }ورنہ{
    pNew->b_Use_Shared_تالا_Handle = 0;
  }
  sqlite3_snprintf(nName+15, pNew->zFilename, "%s-shm", pDbFd->z_راستہ);
  sqlite3FileSuffix3(pDbFd->z_راستہ, pNew->zFilename);

  /* Look to see if there is an existing winShmNode that can be used.
  ** If no matching winShmNode currently exists, then create a new one.  */
  winShmEnterMutex();
  برائے(p_Shm_گرہ = win_Shm_گرہ_List; p_Shm_گرہ; p_Shm_گرہ=p_Shm_گرہ->p_اگلا){
    /* TBD need to come up with better match here.  Perhaps
    ** use FILE_ID_BOTH_DIR_INFO Structure.  */
    اگر( sqlite3StrICmp(p_Shm_گرہ->zFilename, pNew->zFilename)==0 ) توڑو;
  }
  اگر( p_Shm_گرہ==0 ){
    p_Shm_گرہ = pNew;

    /* Allocate a mutex for this winShmNode object, if one is required. */
    اگر( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_گرہ->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      اگر( p_Shm_گرہ->mutex==0 ) rc = SQLITE_IOERR_NOMEM_BKPT;
    }

    /* Open a file-handle to use for mappings, and for the DMS lock. */
    اگر( rc==SQLITE_OK ){
      HANDLE h = INVALID_HANDLE_قدر;
      p_Shm_گرہ->isReadonly = sqlite3_uri_boolean(pDbFd->z_راستہ,"readonly_shm",0);
      rc = winHandleOpen(pNew->zFilename, &p_Shm_گرہ->isReadonly, &h);
      p_Shm_گرہ->hSharedShm = h;
    }

    /* If successful, link the new winShmNode into the global list. If an 
    ** error occurred, free the object. */
    اگر( rc==SQLITE_OK ){
      p_Shm_گرہ->p_اگلا = win_Shm_گرہ_List;
      win_Shm_گرہ_List = p_Shm_گرہ;
      pNew = 0;
    }ورنہ{
      sqlite3_mutex_free(p_Shm_گرہ->mutex);
      اگر( p_Shm_گرہ->hSharedShm!=INVALID_HANDLE_قدر ){
        osCloseHandle(p_Shm_گرہ->hSharedShm);
      }
    }
  }

  /* If no error has occurred, link the winShm object to the winShmNode and
  ** the winShm to pDbFd.  */
  اگر( rc==SQLITE_OK ){
    sqlite3_mutex_enter(p_Shm_گرہ->mutex);
    p->p_Shm_گرہ = p_Shm_گرہ;
    p->p_Win_Shm_اگلا = p_Shm_گرہ->pWinShmList;
    p_Shm_گرہ->pWinShmList = p;
#اگر defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
    p->id = p_Shm_گرہ->اگلا_Shm_Id++;
#اگر_ختم
    pDbFd->pShm = p;
    sqlite3_mutex_leave(p_Shm_گرہ->mutex);
  }ورنہ اگر( p ){
    sqlite3_free(p);
  }

  أكد( rc!=SQLITE_OK || p_Shm_گرہ->isUnlocked==0 || p_Shm_گرہ->nRegion==0 );
  winShmLeaveMutex();
  sqlite3_free(pNew);

  /* Open a file-handle on the *-shm file for this connection. This file-handle
  ** is only used for locking. The mapping of the *-shm file is created using
  ** the shared file handle in winShmNode.hSharedShm.  */
  اگر( rc==SQLITE_OK && p_Shm_گرہ->b_Use_Shared_تالا_Handle==0 ){
    p->bReadonly = sqlite3_uri_boolean(pDbFd->z_راستہ, "readonly_shm", 0);
    rc = winHandleOpen(p_Shm_گرہ->zFilename, &p->bReadonly, &p->hShm);
    اگر( rc!=SQLITE_OK ){
      أكد( p->hShm==INVALID_HANDLE_قدر );
      win_Close_Shared_حافظہ(pDbFd, 0);
    }
  }

  واپس rc;
}

/*
** Close a connection to shared-memory.  Delete the underlying
** storage if deleteFlag is true.
*/
ساکن صحیح winShmUnmap(
  sqlite3_file *fd,          /* Database holding shared memory */
  صحیح deleteFlag             /* Delete after closing if true */
){
  واپس win_Close_Shared_حافظہ((winFile*)fd, deleteFlag);
}

/*
** Change the lock state for a shared-memory segment.
*/
ساکن صحیح win_Shm_تالا(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  صحیح ofst,                  /* First lock to acquire or release */
  صحیح n,                     /* Number of locks to acquire or release */
  صحیح flags                  /* What to do with the lock */
){
  winFile *pDbFd = (winFile*)fd;        /* Connection holding shared memory */
  winShm *p = pDbFd->pShm;              /* The shared memory being locked */
  win_Shm_گرہ *p_Shm_گرہ;
  صحیح rc = SQLITE_OK;                   /* Result code */
  u16 نقاب = (u16)((1U<<(ofst+n)) - (1U<<ofst)); /* Mask of locks to [un]take */

  اگر( p==0 ) واپس SQLITE_IOERR_SHMLOCK;
  p_Shm_گرہ = p->p_Shm_گرہ;
  اگر( NEVER(p_Shm_گرہ==0) ) واپس SQLITE_IOERR_SHMLOCK;

  أكد( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  أكد( n>=1 );
  أكد( flags==(SQLITE_SHM_تالا | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_تالا | SQLITE_SHM_EXCLUSIVE)
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
#اگر defined(SQLITE_ENABLE_SETLK_TIMEOUT) && defined(SQLITE_DEBUG)
  {
    u16 تالا_نقاب = (p->excl_نقاب|p->shared_نقاب);
    أكد( (flags & SQLITE_SHM_UNLOCK) || pDbFd->iBusyTimeout==0 || (
          (ofst!=2 || تالا_نقاب==0)
       && (ofst!=1 || تالا_نقاب==0 || تالا_نقاب==2)
       && (ofst!=0 || تالا_نقاب<3)
       && (ofst<3  || تالا_نقاب<(1<<ofst))
    ));
  }
#اگر_ختم

  /* Check if there is any work to do. There are three cases:
  **
  **    a) An unlock operation where there are locks to unlock,
  **    b) An shared lock where the requested lock is not already held
  **    c) An exclusive lock where the requested lock is not already held
  **
  ** The SQLite core never requests an exclusive lock that it already holds.
  ** This is assert()ed immediately below.  */
  أكد( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_تالا) 
       || 0==(p->excl_نقاب & نقاب)
  );
  اگر( ((flags & SQLITE_SHM_UNLOCK) && ((p->excl_نقاب|p->shared_نقاب) & نقاب))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_تالا) && 0==(p->shared_نقاب & نقاب))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_تالا))
  ){
    HANDLE h = p->hShm;

    اگر( flags & SQLITE_SHM_UNLOCK ){
      /* Case (a) - unlock.  */

      أكد( (p->excl_نقاب & p->shared_نقاب)==0 );
      أكد( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->excl_نقاب & نقاب)==نقاب );
      أكد( !(flags & SQLITE_SHM_SHARED) || (p->shared_نقاب & نقاب)==نقاب );

      أكد( !(flags & SQLITE_SHM_SHARED) || n==1 );
      اگر( p_Shm_گرہ->b_Use_Shared_تالا_Handle ){
        h = p_Shm_گرہ->hSharedShm;
        اگر( flags & SQLITE_SHM_SHARED ){
          winShm *pShm;
          sqlite3_mutex_enter(p_Shm_گرہ->mutex);
          برائے(pShm=p_Shm_گرہ->pWinShmList; pShm; pShm=pShm->p_Win_Shm_اگلا){
            اگر( pShm!=p && (pShm->shared_نقاب & نقاب) ){
              /* Another connection within this process is also holding this
              ** SHARED lock. So do not actually release the OS lock.  */
              h = INVALID_HANDLE_قدر;
              توڑو;
            }
          }
          sqlite3_mutex_leave(p_Shm_گرہ->mutex);
        }
      }

      اگر( h!=INVALID_HANDLE_قدر ){
        rc = winHandleUnlock(h, ofst+WIN_SHM_بنیاد, n);
      }

      /* If successful, also clear the bits in sharedMask/exclMask */
      اگر( rc==SQLITE_OK ){
        p->excl_نقاب = (p->excl_نقاب & ~نقاب);
        p->shared_نقاب = (p->shared_نقاب & ~نقاب);
      }
    }ورنہ{
      صحیح bExcl = ((flags & SQLITE_SHM_EXCLUSIVE) ? 1 : 0);
      DWORD nMs = winFileBusyTimeout(pDbFd);

      اگر( p_Shm_گرہ->b_Use_Shared_تالا_Handle ){
        winShm *pShm;
        h = p_Shm_گرہ->hSharedShm;
        sqlite3_mutex_enter(p_Shm_گرہ->mutex);
        برائے(pShm=p_Shm_گرہ->pWinShmList; pShm; pShm=pShm->p_Win_Shm_اگلا){
          اگر( bExcl ){
            اگر( (pShm->shared_نقاب|pShm->excl_نقاب) & نقاب ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_قدر;
            }
          }ورنہ{
            اگر( pShm->shared_نقاب & نقاب ){
              h = INVALID_HANDLE_قدر;
            }ورنہ اگر( pShm->excl_نقاب & نقاب ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_قدر;
            }
          }
        }
        sqlite3_mutex_leave(p_Shm_گرہ->mutex);
      }

      اگر( h!=INVALID_HANDLE_قدر ){
        rc = win_Handle_تالا_Timeout(h, ofst+WIN_SHM_بنیاد, n, bExcl, nMs);
      }
      اگر( rc==SQLITE_OK ){
        اگر( bExcl ){
          p->excl_نقاب = (p->excl_نقاب | نقاب);
        }ورنہ{
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
  واپس rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
ساکن خالی winShmBarrier(
  sqlite3_file *fd          /* Database holding the shared memory */
){
  غیر_مستعمل_PARAMETER(fd);
  sqlite3_حافظہ_Barrier();   /* compiler-defined memory barrier */
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
ساکن صحیح winShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  صحیح iRegion,                    /* Region to retrieve */
  صحیح szRegion,                   /* Size of regions */
  صحیح isWrite,                    /* True to extend file if necessary */
  خالی متلون **pp              /* OUT: Mapped memory */
){
  winFile *pDbFd = (winFile*)fd;
  winShm *pShm = pDbFd->pShm;
  win_Shm_گرہ *p_Shm_گرہ;
  DWORD protect = صفحہ_READWRITE;
  DWORD flags = FILE_MAP_WRITE | FILE_MAP_READ;
  صحیح rc = SQLITE_OK;

  اگر( !pShm ){
    rc = win_Open_Shared_حافظہ(pDbFd);
    اگر( rc!=SQLITE_OK ) واپس rc;
    pShm = pDbFd->pShm;
    أكد( pShm!=0 );
  }
  p_Shm_گرہ = pShm->p_Shm_گرہ;

  sqlite3_mutex_enter(p_Shm_گرہ->mutex);
  اگر( p_Shm_گرہ->isUnlocked ){
    /* Take the DMS lock. */
    أكد( p_Shm_گرہ->nRegion==0 );
    rc = win_تالا_Shared_حافظہ(p_Shm_گرہ, winFileBusyTimeout(pDbFd));
    اگر( rc!=SQLITE_OK ) جاؤ shmpage_برآمد;
  }

  أكد( szRegion==p_Shm_گرہ->szRegion || p_Shm_گرہ->nRegion==0 );
  اگر( p_Shm_گرہ->nRegion<=iRegion ){
    HANDLE hShared = p_Shm_گرہ->hSharedShm;
    ساخت ShmRegion *apNew;           /* New aRegion[] array */
    i64 nByte = ((i64)iRegion+1)*(i64)szRegion;  /* Minimum file size */
    sqlite3_int64 sz;                  /* Current size of wal-index file */

    p_Shm_گرہ->szRegion = szRegion;

    /* The requested region is not mapped into this processes address space.
    ** Check to see if it has been allocated (i.e. if the wal-index file is
    ** large enough to contain the requested region).
    */
    rc = win_Handle_سائز(hShared, &sz);
    اگر( rc!=SQLITE_OK ){
      rc = winLogError(rc, os_Get_آخری_Error(), "winShmMap1", pDbFd->z_راستہ);
      جاؤ shmpage_برآمد;
    }

    اگر( sz<nByte ){
      /* The requested memory region does not exist. If isWrite is set to
      ** zero, exit early. *pp will be set to NULL and SQLITE_OK returned.
      **
      ** Alternatively, if isWrite is non-zero, use ftruncate() to allocate
      ** the requested memory region.  */
      اگر( !isWrite ) جاؤ shmpage_برآمد;
      rc = winHandleTruncate(hShared, nByte);
      اگر( rc!=SQLITE_OK ){
        rc = winLogError(rc, os_Get_آخری_Error(), "winShmMap2", pDbFd->z_راستہ);
        جاؤ shmpage_برآمد;
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (ساخت ShmRegion*)sqlite3_realloc64(
        p_Shm_گرہ->aRegion, ((i64)iRegion+1)*حجم(apNew[0])
    );
    اگر( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      جاؤ shmpage_برآمد;
    }
    p_Shm_گرہ->aRegion = apNew;

    اگر( p_Shm_گرہ->isReadonly ){
      protect = صفحہ_READONLY;
      flags = FILE_MAP_READ;
    }

    جب( p_Shm_گرہ->nRegion<=iRegion ){
      HANDLE hMap = تہی;         /* file-mapping handle */
      خالی *pMap = 0;             /* Mapped memory region */

#اگر_تعریف_شدہ SQLITE_UWP
      hMap = osCreateFileMappingFromApp(hShared, تہی, protect, nByte, تہی);
#ورنہ
      hMap = osCreateFileMappingW(hShared, تہی, protect, 0, nByte, تہی);
#اگر_ختم
      OSTRACE(("SHM-MAP-CREATE pid=%lu, region=%d, size=%lld, rc=%s\n",
               osGetCurrentProcessId(), p_Shm_گرہ->nRegion, nByte,
               hMap ? "ok" : "failed"));
      اگر( hMap ){
        i64 i_انحراف = p_Shm_گرہ->nRegion*szRegion;
        صحیح i_انحراف_سرکاؤ = i_انحراف % winSysInfo.dw_تخصیص_Granularity;
#اگر_تعریف_شدہ SQLITE_UWP
        pMap = os_Map_View_کا_File_From_App(hMap, flags,
            i_انحراف - i_انحراف_سرکاؤ, (i64)szRegion + i_انحراف_سرکاؤ
        );
#ورنہ
        pMap = os_Map_View_کا_File(hMap, flags,
            0, i_انحراف - i_انحراف_سرکاؤ, (i64)szRegion + i_انحراف_سرکاؤ
        );
#اگر_ختم
        OSTRACE(("SHM-MAP-MAP pid=%lu, region=%d, offset=%d, size=%d, rc=%s\n",
                 osGetCurrentProcessId(), p_Shm_گرہ->nRegion, i_انحراف,
                 szRegion, pMap ? "ok" : "failed"));
      }
      اگر( !pMap ){
        p_Shm_گرہ->آخری_Errno = os_Get_آخری_Error();
        rc = winLogError(SQLITE_IOERR_SHMMAP, p_Shm_گرہ->آخری_Errno,
                         "winShmMap3", pDbFd->z_راستہ);
        اگر( hMap ) osCloseHandle(hMap);
        جاؤ shmpage_برآمد;
      }

      p_Shm_گرہ->aRegion[p_Shm_گرہ->nRegion].pMap = pMap;
      p_Shm_گرہ->aRegion[p_Shm_گرہ->nRegion].hMap = hMap;
      p_Shm_گرہ->nRegion++;
    }
  }

shmpage_برآمد:
  اگر( p_Shm_گرہ->nRegion>iRegion ){
    i64 i_انحراف = (i64)iRegion*(i64)szRegion;
    صحیح i_انحراف_سرکاؤ = i_انحراف % winSysInfo.dw_تخصیص_Granularity;
    حرف *p = (حرف *)p_Shm_گرہ->aRegion[iRegion].pMap;
    *pp = (خالی *)&p[i_انحراف_سرکاؤ];
  }ورنہ{
    *pp = 0;
  }
  اگر( p_Shm_گرہ->isReadonly && rc==SQLITE_OK ){
    rc = SQLITE_READONLY;
  }
  sqlite3_mutex_leave(p_Shm_گرہ->mutex);
  واپس rc;
}

#ورنہ
# define winShmMap     0
# define win_Shm_تالا    0
# define winShmBarrier 0
# define winShmUnmap   0
#اگر_ختم /* #ifndef SQLITE_OMIT_WAL */

/*
** Cleans up the mapped region of the specified file, if any.
*/
#اگر SQLITE_MAX_MMAP_سائز>0
ساکن صحیح winUnmapfile(winFile *pFile){
  أكد( pFile!=0 );
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, pMapRegion=%p, "
           "mmapSize=%lld, mmapSizeMax=%lld\n",
           osGetCurrentProcessId(), pFile, pFile->hMap, pFile->pMapRegion,
           pFile->mmap_سائز, pFile->mmap_سائز_Max));
  اگر( pFile->pMapRegion ){
    اگر( !os_Unmap_View_کا_File(pFile->pMapRegion) ){
      pFile->آخری_Errno = os_Get_آخری_Error();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(), pFile,
               pFile->pMapRegion));
      واپس winLogError(SQLITE_IOERR_MMAP, pFile->آخری_Errno,
                         "winUnmapfile1", pFile->z_راستہ);
    }
    pFile->pMapRegion = 0;
    pFile->mmap_سائز = 0;
  }
  اگر( pFile->hMap!=تہی ){
    اگر( !osCloseHandle(pFile->hMap) ){
      pFile->آخری_Errno = os_Get_آخری_Error();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, rc=SQLITE_IOERR_MMAP\n",
               osGetCurrentProcessId(), pFile, pFile->hMap));
      واپس winLogError(SQLITE_IOERR_MMAP, pFile->آخری_Errno,
                         "winUnmapfile2", pFile->z_راستہ);
    }
    pFile->hMap = تہی;
  }
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile));
  واپس SQLITE_OK;
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
ساکن صحیح winMapfile(winFile *pFd, sqlite3_int64 nByte){
  sqlite3_int64 nMap = nByte;
  صحیح rc;

  أكد( nMap>=0 || pFd->n_Fetch_برآمد==0 );
  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, size=%lld\n",
           osGetCurrentProcessId(), pFd, nByte));

  اگر( pFd->n_Fetch_برآمد>0 ) واپس SQLITE_OK;

  اگر( nMap<0 ){
    rc = win_File_سائز((sqlite3_file*)pFd, &nMap);
    اگر( rc ){
      OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_IOERR_FSTAT\n",
               osGetCurrentProcessId(), pFd));
      واپس SQLITE_IOERR_FSTAT;
    }
  }
  اگر( nMap>pFd->mmap_سائز_Max ){
    nMap = pFd->mmap_سائز_Max;
  }
  nMap &= ~(sqlite3_int64)(winSysInfo.dw_صفحہ_سائز - 1);

  اگر( nMap==0 && pFd->mmap_سائز>0 ){
    winUnmapfile(pFd);
  }
  اگر( nMap!=pFd->mmap_سائز ){
    خالی *pNew = 0;
    DWORD protect = صفحہ_READONLY;
    DWORD flags = FILE_MAP_READ;

    winUnmapfile(pFd);
#اگر_تعریف_شدہ SQLITE_MMAP_READWRITE
    اگر( (pFd->ctrlFlags & WINFILE_RDONLY)==0 ){
      protect = صفحہ_READWRITE;
      flags |= FILE_MAP_WRITE;
    }
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_UWP
    pFd->hMap = osCreateFileMappingFromApp(pFd->h, تہی, protect, nMap, تہی);
#ورنہ
    pFd->hMap = osCreateFileMappingW(pFd->h, تہی, protect,
                                (DWORD)((nMap>>32) & 0xffffffff),
                                (DWORD)(nMap & 0xffffffff), تہی);
#اگر_ختم
    اگر( pFd->hMap==تہی ){
      pFd->آخری_Errno = os_Get_آخری_Error();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->آخری_Errno,
                       "winMapfile1", pFd->z_راستہ);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-CREATE pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      واپس SQLITE_OK;
    }
    أكد( (nMap % winSysInfo.dw_صفحہ_سائز)==0 );
    أكد( حجم(سائز_T)==حجم(sqlite3_int64) || nMap<=0xffffffff );
#اگر_تعریف_شدہ SQLITE_UWP
    pNew = os_Map_View_کا_File_From_App(pFd->hMap, flags, 0, (سائز_T)nMap);
#ورنہ
    pNew = os_Map_View_کا_File(pFd->hMap, flags, 0, 0, (سائز_T)nMap);
#اگر_ختم
    اگر( pNew==تہی ){
      osCloseHandle(pFd->hMap);
      pFd->hMap = تہی;
      pFd->آخری_Errno = os_Get_آخری_Error();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->آخری_Errno,
                       "winMapfile2", pFd->z_راستہ);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-MAP pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      واپس SQLITE_OK;
    }
    pFd->pMapRegion = pNew;
    pFd->mmap_سائز = nMap;
  }

  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFd));
  واپس SQLITE_OK;
}
#اگر_ختم /* SQLITE_MAX_MMAP_SIZE>0 */

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
ساکن صحیح winFetch(sqlite3_file *fd, i64 iOff, صحیح nAmt, خالی **pp){
#اگر SQLITE_MAX_MMAP_سائز>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */
#اگر_ختم
  *pp = 0;

  OSTRACE(("FETCH pid=%lu, pFile=%p, offset=%lld, amount=%d, pp=%p\n",
           osGetCurrentProcessId(), fd, iOff, nAmt, pp));

#اگر SQLITE_MAX_MMAP_سائز>0
  اگر( pFd->mmap_سائز_Max>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    ثابت صحیح n_Eof_بفر = 256;
    اگر( pFd->pMapRegion==0 ){
      صحیح rc = winMapfile(pFd, -1);
      اگر( rc!=SQLITE_OK ){
        OSTRACE(("FETCH pid=%lu, pFile=%p, rc=%s\n",
                 osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
        واپس rc;
      }
    }
    اگر( pFd->mmap_سائز >= (iOff+nAmt+n_Eof_بفر) ){
      أكد( pFd->pMapRegion!=0 );
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->n_Fetch_برآمد++;
    }
  }
#اگر_ختم

  OSTRACE(("FETCH pid=%lu, pFile=%p, pp=%p, *pp=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd, pp, *pp));
  واپس SQLITE_OK;
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
ساکن صحیح winUnfetch(sqlite3_file *fd, i64 iOff, خالی *p){
#اگر SQLITE_MAX_MMAP_سائز>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  أكد( (p==0)==(pFd->n_Fetch_برآمد==0) );

  /* If p!=0, it must match the iOff value. */
  أكد( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, offset=%lld, p=%p\n",
           osGetCurrentProcessId(), pFd, iOff, p));

  اگر( p ){
    pFd->n_Fetch_برآمد--;
  }ورنہ{
    /* FIXME:  If Windows truly always prevents truncating or deleting a
    ** file while a mapping is held, then the following winUnmapfile() call
    ** is unnecessary can be omitted - potentially improving
    ** performance.  */
    winUnmapfile(pFd);
  }

  أكد( pFd->n_Fetch_برآمد>=0 );
#اگر_ختم

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd));
  واپس SQLITE_OK;
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
ساکن ثابت sqlite3_io_methods win_Io_طریقہ = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_سائز,                    /* xFileSize */
  win_تالا,                        /* xLock */
  winUnlock,                      /* xUnlock */
  win_Check_Reserved_تالا,           /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_سائز,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  win_Shm_تالا,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

/*
** This vector defines all the methods that can operate on an
** sqlite3_file for win32 without performing any locking.
*/
ساکن ثابت sqlite3_io_methods win_Io_Nolock_طریقہ = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_سائز,                    /* xFileSize */
  win_Nolock_تالا,                  /* xLock */
  winNolockUnlock,                /* xUnlock */
  win_Nolock_Check_Reserved_تالا,     /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_سائز,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  win_Shm_تالا,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

ساکن win_Vfs_App_ڈیٹا win_App_ڈیٹا = {
  &win_Io_طریقہ,       /* pMethod */
  0,                  /* pAppData */
  0                   /* bNoLock */
};

ساکن win_Vfs_App_ڈیٹا win_Nolock_App_ڈیٹا = {
  &win_Io_Nolock_طریقہ, /* pMethod */
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
ساکن صحیح win_Make_انتہا_اندر_Dir_Sep(صحیح nBuf, حرف *zBuf){
  اگر( zBuf ){
    صحیح nLen = sqlite3Strlen30(zBuf);
    اگر( nLen>0 ){
      اگر( winIsDirSep(zBuf[nLen-1]) ){
        واپس 1;
      }ورنہ اگر( nLen+1<nBuf ){
        اگر( !osGetenv ){
          zBuf[nLen] = winGetDirSep();
        }ورنہ اگر( winIsDriveLetterAndColon(zBuf) && winIsDirSep(zBuf[2]) ){
          zBuf[nLen] = '\\';
          zBuf[2]='\\';
        }ورنہ{
          zBuf[nLen] = '/';
        }
        zBuf[nLen+1] = '\0';
        واپس 1;
      }
    }
  }
  واپس 0;
}

/*
** If sqlite3_temp_directory is defined, take the mutex and return true.
**
** If sqlite3_temp_directory is NULL (undefined), omit the mutex and
** return false.
*/
ساکن صحیح win_عارضی_Dir_Defined(خالی){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  اگر( sqlite3_عارضی_directory!=0 ) واپس 1;
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  واپس 0;
}

/*
** Create a temporary file name and store the resulting pointer into pzBuf.
** The pointer returned in pzBuf must be freed via sqlite3_free().
*/
ساکن صحیح winGetTempname(sqlite3_vfs *pVfs, حرف **pzBuf){
  ساکن ثابت حرف zChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  size_t i, j;
  DWORD pid;
  صحیح nPre = sqlite3Strlen30(SQLITE_عارضی_FILE_سابقہ);
  i64 nMax, nBuf, nDir, nLen;
  حرف *zBuf;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  SimulateIOError( واپس SQLITE_IOERR );

  /* Allocate a temporary buffer to store the fully qualified file
  ** name for the temporary file.  If this fails, we cannot continue.
  */
  nMax = pVfs->mxPathname;
  nBuf = 2 + (i64)nMax;
  zBuf = sqlite3MallocZero( nBuf );
  اگر( !zBuf ){
    OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
    واپس SQLITE_IOERR_NOMEM_BKPT;
  }

  /* Figure out the effective temporary directory.  First, check if one
  ** has been explicitly set by the application; otherwise, use the one
  ** configured by the operating system.
  */
  nDir = nMax - (nPre + 15);
  أكد( nDir>0 );
  اگر( win_عارضی_Dir_Defined() ){
    صحیح nDirLen = sqlite3Strlen30(sqlite3_عارضی_directory);
    اگر( nDirLen>0 ){
      اگر( !winIsDirSep(sqlite3_عارضی_directory[nDirLen-1]) ){
        nDirLen++;
      }
      اگر( nDirLen>nDir ){
        sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
        sqlite3_free(zBuf);
        OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
        واپس winLogError(SQLITE_ERROR, 0, "winGetTempname1", 0);
      }
      sqlite3_snprintf(nMax, zBuf, "%s", sqlite3_عارضی_directory);
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  }

#اگر defined(__CYGWIN__)
  ورنہ اگر( osGetenv!=تہی ){
    ساکن ثابت حرف *azDirs[] = {
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
    غیرعلامتی صحیح i;
    ثابت حرف *zDir = 0;

    اگر( !azDirs[0] ) azDirs[0] = osGetenv("SQLITE_TMPDIR");
    اگر( !azDirs[1] ) azDirs[1] = osGetenv("TMPDIR");
    اگر( !azDirs[2] ) azDirs[2] = osGetenv("TMP");
    اگر( !azDirs[3] ) azDirs[3] = osGetenv("TEMP");
    اگر( !azDirs[4] ) azDirs[4] = osGetenv("USERPROFILE");
    برائے(i=0; i<حجم(azDirs)/حجم(azDirs[0]); zDir=azDirs[i++]){
      LPWSTR zConverted;
      اگر( zDir==0 ) جاری;
      /* If the path starts with a drive letter followed by the colon
      ** character, assume it is already a native Win32 path; otherwise,
      ** it must be converted to a native Win32 path via the Cygwin API
      ** prior to using it.
      */
      {
        zConverted = winConvertFromUtf8Filename(zDir);
        اگر( !zConverted ){
          sqlite3_free(zBuf);
          OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
          واپس SQLITE_IOERR_NOMEM_BKPT;
        }
        اگر( winIsDir(zConverted) ){
          sqlite3_snprintf(nMax, zBuf, "%s", zDir);
          sqlite3_free(zConverted);
          توڑو;
        }
        sqlite3_free(zConverted);
      }
    }
  }
#اگر_ختم

  ورنہ{
    حرف *zMulti;
    LPWSTR z_Wide_راستہ = sqlite3MallocZero( nMax*حجم(WCHAR) );
    اگر( !z_Wide_راستہ ){
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      واپس SQLITE_IOERR_NOMEM_BKPT;
    }
    اگر( os_Get_عارضی_راستہ_W==0 || os_Get_عارضی_راستہ_W(nMax, z_Wide_راستہ)==0 ){
      sqlite3_free(z_Wide_راستہ);
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_GETTEMPPATH\n"));
      واپس winLogError(SQLITE_IOERR_GETTEMPPATH, os_Get_آخری_Error(),
                         "winGetTempname2", 0);
    }
    zMulti = win_Unicode_مقصد_Utf8(z_Wide_راستہ);
    sqlite3_free(z_Wide_راستہ);
    اگر( zMulti ){
      sqlite3_snprintf(nMax, zBuf, "%s", zMulti);
      sqlite3_free(zMulti);
    }ورنہ{
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      واپس SQLITE_IOERR_NOMEM_BKPT;
    }
  }

  /*
  ** Check to make sure the temporary directory ends with an appropriate
  ** separator.  If it does not and there is not enough space left to add
  ** one, fail.
  */
  اگر( !win_Make_انتہا_اندر_Dir_Sep(nDir+1, zBuf) ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    واپس winLogError(SQLITE_ERROR, 0, "winGetTempname4", 0);
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
  اگر( (nLen + nPre + 17) > nBuf ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    واپس winLogError(SQLITE_ERROR, 0, "winGetTempname5", 0);
  }

  sqlite3_snprintf(nBuf-16-nLen, zBuf+nLen, SQLITE_عارضی_FILE_سابقہ);

  j = sqlite3Strlen30(zBuf);
  sqlite3_randomness(15, &zBuf[j]);
  pid = osGetCurrentProcessId();
  برائے(i=0; i<15; i++, j++){
    zBuf[j] += pid & 0xff;
    pid >>= 8;
    zBuf[j] = (حرف)zChars[ ((غیرعلامتی حرف)zBuf[j])%(حجم(zChars)-1) ];
  }
  zBuf[j] = 0;
  zBuf[j+1] = 0;
  *pzBuf = zBuf;

  OSTRACE(("TEMP-FILENAME name=%s, rc=SQLITE_OK\n", zBuf));
  واپس SQLITE_OK;
}

/*
** Return TRUE if the named file is really a directory.  Return false if
** it is something other than a directory, or if there is any kind of memory
** allocation failure.
*/
ساکن صحیح winIsDir(LPCWSTR zConverted){
  DWORD attr;
  صحیح rc = 0;
  DWORD آخری_Errno;
  صحیح cnt = 0;
  WIN32_FILE_ATTRIBUTE_ڈیٹا s_Attr_ڈیٹا;
  memset(&s_Attr_ڈیٹا, 0, حجم(s_Attr_ڈیٹا));
  جب( !(rc = os_Get_File_اوصاف_Ex_W(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_ڈیٹا)) && winRetryIoerr(&cnt, &آخری_Errno) ){}
  اگر( !rc ){
    واپس 0; /* Invalid name? */
  }
  attr = s_Attr_ڈیٹا.dw_File_اوصاف;
  واپس (attr!=INVALID_FILE_اوصاف) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

/* forward reference */
ساکن صحیح winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ثابت حرف *zFilename,     /* Name of file to check */
  صحیح flags,                 /* Type of test to make on this file */
  صحیح *p_Res_برآمد               /* OUT: Result */
);

/*
** The Windows version of xAccess() accepts an extra bit in the flags
** parameter that prevents an anti-virus retry loop.
*/
#تعریف_کرو NORETRY 0x4000

/*
** Open a file.
*/
ساکن صحیح winOpen(
  sqlite3_vfs *pVfs,        /* Used to get maximum path length and AppData */
  ثابت حرف *zName,        /* Name of the file (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  صحیح flags,                /* Open mode flags */
  صحیح *p_برآمد_Flags            /* Status return flags */
){
  HANDLE h;
  DWORD آخری_Errno = 0;
  DWORD dwDesiredAccess;
  DWORD dw_Share_منوال;
  DWORD dwCreationDisposition;
  DWORD dw_Flags_And_اوصاف = 0;
  win_Vfs_App_ڈیٹا *p_App_ڈیٹا;
  winFile *pFile = (winFile*)id;
  LPWSTR zConverted;              /* Filename in OS encoding */
  ثابت حرف *zUtf8Name = zName; /* Filename in UTF-8 encoding */
  صحیح cnt = 0;
  صحیح isRO = 0;              /* file is known to be accessible readonly */

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  حرف *zTmpname = 0; /* For temporary filename, if necessary. */

  صحیح rc = SQLITE_OK;            /* Function Return Code */
#اگر !defined(NDEBUG)
  صحیح eType = flags&0x0FFF00;  /* Type of file to open */
#اگر_ختم

  صحیح isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  صحیح isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  صحیح isCreate     = (flags & SQLITE_OPEN_CREATE);
  صحیح isReadonly   = (flags & SQLITE_OPEN_READONLY);
  صحیح isReadWrite  = (flags & SQLITE_OPEN_READWRITE);

#اگر_غیر_تعریف_شدہ NDEBUG
  صحیح isOpenJournal = (isCreate && (
        eType==SQLITE_OPEN_SUPER_JOURNAL
     || eType==SQLITE_OPEN_MAIN_JOURNAL
     || eType==SQLITE_OPEN_WAL
  ));
#اگر_ختم

  OSTRACE(("OPEN name=%s, pFile=%p, flags=%x, pOutFlags=%p\n",
           zUtf8Name, id, flags, p_برآمد_Flags));

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
  أكد( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_عارضی_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_عارضی_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );

  أكد( pFile!=0 );
  memset(pFile, 0, حجم(winFile));
  pFile->h = INVALID_HANDLE_قدر;

  /* If the second argument to this function is NULL, generate a
  ** temporary file name to use
  */
  اگر( !zUtf8Name ){
    أكد( isDelete && !isOpenJournal );
    rc = winGetTempname(pVfs, &zTmpname);
    اگر( rc!=SQLITE_OK ){
      OSTRACE(("OPEN name=%s, rc=%s", zUtf8Name, sqlite3ErrName(rc)));
      واپس rc;
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
  اگر( zConverted==0 ){
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8Name));
    واپس SQLITE_IOERR_NOMEM_BKPT;
  }

  اگر( winIsDir(zConverted) ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8Name));
    واپس SQLITE_CANTOPEN_ISDIR;
  }

  اگر( isReadWrite ){
    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
  }ورنہ{
    dwDesiredAccess = GENERIC_READ;
  }

  /* SQLITE_OPEN_EXCLUSIVE is used to make sure that a new file is
  ** created. SQLite doesn't use it to indicate "exclusive access"
  ** as it is usually understood.
  */
  اگر( isExclusive ){
    /* Creates a new file, only if it does not already exist. */
    /* If the file exists, it fails. */
    dwCreationDisposition = CREATE_NEW;
  }ورنہ اگر( isCreate ){
    /* Open existing file, or create if it doesn't exist */
    dwCreationDisposition = OPEN_ALWAYS;
  }ورنہ{
    /* Opens a file, only if it exists. */
    dwCreationDisposition = OPEN_EXISTING;
  }

  اگر( 0==sqlite3_uri_boolean(zName, "exclusive", 0) ){
    dw_Share_منوال = FILE_SHARE_READ | FILE_SHARE_WRITE;
  }ورنہ{
    dw_Share_منوال = 0;
  }

  اگر( isDelete ){
    dw_Flags_And_اوصاف = FILE_ATTRIBUTE_TEMPORARY
                               | FILE_ATTRIBUTE_HIDDEN
                               | FILE_FLAG_DELETE_ON_CLOSE;
  }ورنہ{
    dw_Flags_And_اوصاف = FILE_ATTRIBUTE_معمول;
  }
  کرو{
    h = osCreateFileW(zConverted,
                      dwDesiredAccess,
                      dw_Share_منوال, تہی,
                      dwCreationDisposition,
                      dw_Flags_And_اوصاف,
                      تہی);
    اگر( h!=INVALID_HANDLE_قدر ) توڑو;
    اگر( isReadWrite ){
      صحیح rc2;
      sqlite3BeginBenignMalloc();
      rc2 = winAccess(pVfs, zUtf8Name, SQLITE_ACCESS_READ|NORETRY, &isRO);
      sqlite3_انتہا_Benign_Malloc();
      اگر( rc2==SQLITE_OK && isRO ) توڑو;
    }
  }جب( winRetryIoerr(&cnt, &آخری_Errno) );
  winLogIoerr(cnt, __LINE__);

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, rc=%s\n", h, zUtf8Name,
           dwDesiredAccess, (h==INVALID_HANDLE_قدر) ? "failed" : "ok"));

  اگر( h==INVALID_HANDLE_قدر ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    اگر( isReadWrite && isRO && !isExclusive ){
      واپس winOpen(pVfs, zName, id,
         ((flags|SQLITE_OPEN_READONLY) &
                     ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE)),
         p_برآمد_Flags);
    }ورنہ{
      pFile->آخری_Errno = آخری_Errno;
      winLogError(SQLITE_CANTOPEN, pFile->آخری_Errno, "winOpen", zUtf8Name);
      واپس SQLITE_CANTOPEN_BKPT;
    }
  }

  اگر( p_برآمد_Flags ){
    اگر( isReadWrite ){
      *p_برآمد_Flags = SQLITE_OPEN_READWRITE;
    }ورنہ{
      *p_برآمد_Flags = SQLITE_OPEN_READONLY;
    }
  }

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, pOutFlags=%p, *pOutFlags=%d, "
           "rc=%s\n", h, zUtf8Name, dwDesiredAccess, p_برآمد_Flags, p_برآمد_Flags ?
           *p_برآمد_Flags : 0, (h==INVALID_HANDLE_قدر) ? "failed" : "ok"));

  p_App_ڈیٹا = (win_Vfs_App_ڈیٹا*)pVfs->p_App_ڈیٹا;

  sqlite3_free(zConverted);
  sqlite3_free(zTmpname);
  id->pMethods = p_App_ڈیٹا ? p_App_ڈیٹا->p_طریقہ : &win_Io_طریقہ;
  pFile->pVfs = pVfs;
  pFile->h = h;
  اگر( isReadonly ){
    pFile->ctrlFlags |= WINFILE_RDONLY;
  }
  اگر( (flags & SQLITE_OPEN_MAIN_DB)
   && sqlite3_uri_boolean(zName, "psow", SQLITE_POWERSAFE_OVERWRITE) 
  ){
    pFile->ctrlFlags |= WINFILE_PSOW;
  }
  pFile->آخری_Errno = NO_ERROR;
  pFile->z_راستہ = zName;
#اگر SQLITE_MAX_MMAP_سائز>0
  pFile->hMap = تہی;
  pFile->pMapRegion = 0;
  pFile->mmap_سائز = 0;
  pFile->mmap_سائز_Max = sqlite3GlobalConfig.szMmap;
#اگر_ختم

  Open_شمار(+1);
  واپس rc;
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
ساکن صحیح winDelete(
  sqlite3_vfs *pVfs,          /* Not used on win32 */
  ثابت حرف *zFilename,      /* Name of file to delete */
  صحیح syncDir                 /* Not used on win32 */
){
  صحیح cnt = 0;
  صحیح rc;
  DWORD attr;
  DWORD آخری_Errno = 0;
  LPWSTR zConverted;
  غیر_مستعمل_PARAMETER(pVfs);
  غیر_مستعمل_PARAMETER(syncDir);

  SimulateIOError(واپس SQLITE_IOERR_DELETE);
  OSTRACE(("DELETE name=%s, syncDir=%d\n", zFilename, syncDir));

  zConverted = winConvertFromUtf8Filename(zFilename);
  اگر( zConverted==0 ){
    OSTRACE(("DELETE name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    واپس SQLITE_IOERR_NOMEM_BKPT;
  }
  کرو {
    attr = os_Get_File_اوصاف_W(zConverted);
    اگر ( attr==INVALID_FILE_اوصاف ){
      آخری_Errno = os_Get_آخری_Error();
      اگر( آخری_Errno==ERROR_FILE_NOT_ملا
       || آخری_Errno==ERROR_راستہ_NOT_ملا ){
        rc = SQLITE_IOERR_DELETE_NOENT; /* Already gone? */
      }ورنہ{
        rc = SQLITE_ERROR;
      }
      توڑو;
    }
    اگر ( attr&FILE_ATTRIBUTE_DIRECTORY ){
      rc = SQLITE_ERROR; /* Files only. */
      توڑو;
    }
    اگر ( osDeleteFileW(zConverted) ){
      rc = SQLITE_OK; /* Deleted OK. */
      توڑو;
    }
    اگر ( !winRetryIoerr(&cnt, &آخری_Errno) ){
      rc = SQLITE_ERROR; /* No more retries. */
      توڑو;
    }
  } جب(1);
  اگر( rc && rc!=SQLITE_IOERR_DELETE_NOENT ){
    rc = winLogError(SQLITE_IOERR_DELETE, آخری_Errno, "winDelete", zFilename);
  }ورنہ{
    winLogIoerr(cnt, __LINE__);
  }
  sqlite3_free(zConverted);
  OSTRACE(("DELETE name=%s, rc=%s\n", zFilename, sqlite3ErrName(rc)));
  واپس rc;
}

/*
** Check the existence and status of a file.
*/
ساکن صحیح winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ثابت حرف *zFilename,     /* Name of file to check */
  صحیح flags,                 /* Type of test to make on this file */
  صحیح *p_Res_برآمد               /* OUT: Result */
){
  DWORD attr;
  صحیح rc = 0;
  صحیح cnt = 0;
  DWORD آخری_Errno = 0;
  LPWSTR zConverted;
  صحیح noRetry = 0;           /* Do not use winRetryIoerr() */
  WIN32_FILE_ATTRIBUTE_ڈیٹا s_Attr_ڈیٹا;
  غیر_مستعمل_PARAMETER(pVfs);

  اگر( (flags & NORETRY)!=0 ){
    noRetry = 1;
    flags &= ~NORETRY;
  }  

  SimulateIOError( واپس SQLITE_IOERR_ACCESS; );
  OSTRACE(("ACCESS name=%s, flags=%x, pResOut=%p\n",
           zFilename, flags, p_Res_برآمد));

  اگر( zFilename==0 ){
    *p_Res_برآمد = 0;
    OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
             zFilename, p_Res_برآمد, *p_Res_برآمد));
    واپس SQLITE_OK;
  }

  zConverted = winConvertFromUtf8Filename(zFilename);
  اگر( zConverted==0 ){
    OSTRACE(("ACCESS name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    واپس SQLITE_IOERR_NOMEM_BKPT;
  }
  memset(&s_Attr_ڈیٹا, 0, حجم(s_Attr_ڈیٹا));
  جب( !(rc = os_Get_File_اوصاف_Ex_W(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_ڈیٹا))
     && !noRetry
     && winRetryIoerr(&cnt, &آخری_Errno)
  ){ /* Loop until true */}
  اگر( rc ){
    /* For an SQLITE_ACCESS_EXISTS query, treat a zero-length file
    ** as if it does not exist.
    */
    اگر(    flags==SQLITE_ACCESS_EXISTS
        && s_Attr_ڈیٹا.n_File_سائز_بلند==0
        && s_Attr_ڈیٹا.n_File_سائز_کم==0 ){
      attr = INVALID_FILE_اوصاف;
    }ورنہ{
      attr = s_Attr_ڈیٹا.dw_File_اوصاف;
    }
  }ورنہ{
    اگر( noRetry ) آخری_Errno = os_Get_آخری_Error();
    winLogIoerr(cnt, __LINE__);
    اگر( آخری_Errno!=ERROR_FILE_NOT_ملا && آخری_Errno!=ERROR_راستہ_NOT_ملا ){
      sqlite3_free(zConverted);
      واپس winLogError(SQLITE_IOERR_ACCESS, آخری_Errno, "winAccess",
                         zFilename);
    }ورنہ{
      attr = INVALID_FILE_اوصاف;
    }
  }
  sqlite3_free(zConverted);
  انتخاب( flags ){
    حالت SQLITE_ACCESS_READ:
    حالت SQLITE_ACCESS_EXISTS:
      rc = attr!=INVALID_FILE_اوصاف;
      توڑو;
    حالت SQLITE_ACCESS_READWRITE:
      rc = attr!=INVALID_FILE_اوصاف &&
             (attr & FILE_ATTRIBUTE_READONLY)==0;
      توڑو;
    طےشدہ:
      أكد(!"Invalid flags argument");
  }
  *p_Res_برآمد = rc;
  OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           zFilename, p_Res_برآمد, *p_Res_برآمد));
  واپس SQLITE_OK;
}

/*
** Returns non-zero if the specified path name starts with the "long path"
** prefix.
*/
ساکن BOOL win_Is_Long_راستہ_سابقہ(
  ثابت حرف *zPathname
){
  واپس ( zPathname[0]=='\\' && zPathname[1]=='\\'
        && zPathname[2]=='?'  && zPathname[3]=='\\' );
}

/*
** Returns non-zero if the specified path name starts with a drive letter
** followed by a colon character.
*/
ساکن BOOL winIsDriveLetterAndColon(
  ثابت حرف *zPathname
){
  واپس ( sqlite3Isalpha(zPathname[0]) && zPathname[1]==':' );
}

#اگر_تعریف_شدہ _WIN32
/*
** Returns non-zero if the specified path name should be used verbatim.  If
** non-zero is returned from this function, the calling function must simply
** use the provided path name verbatim -OR- resolve it into a full path name
** using the GetFullPathName Win32 API function (if available).
*/
ساکن BOOL winIsVerbatimPathname(
  ثابت حرف *zPathname
){
  /*
  ** If the path name starts with a forward slash or a backslash, it is either
  ** a legal UNC name, a volume relative path, or an absolute path name in the
  ** "Unix" format on Windows.  There is no easy way to differentiate between
  ** the final two cases; therefore, we return the safer return value of TRUE
  ** so that callers of this function will simply use it verbatim.
  */
  اگر ( winIsDirSep(zPathname[0]) ){
    واپس TRUE;
  }

  /*
  ** If the path name starts with a letter and a colon it is either a volume
  ** relative path or an absolute path.  Callers of this function must not
  ** attempt to treat it as a relative path name (i.e. they should simply use
  ** it verbatim).
  */
  اگر ( winIsDriveLetterAndColon(zPathname) ){
    واپس TRUE;
  }

  /*
  ** If we get to this point, the path name should almost certainly be a purely
  ** relative one (i.e. not a UNC name, not absolute, and not volume relative).
  */
  واپس FALSE;
}
#اگر_ختم /* _WIN32 */

#اگر_تعریف_شدہ __CYGWIN__
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
ساکن خالی winSimplifyName(حرف *z){
  صحیح i, j;
  برائے(i=j=0; z[i]; ++i){
    اگر( winIsDirSep(z[i]) ){
#اگر !defined(SQLITE_TEST)
      /* Some test-cases assume that "./foo" and "foo" are different */
      اگر( z[i+1]=='.' && winIsDirSep(z[i+2]) ){
        ++i;
        جاری;
      }
#اگر_ختم
      اگر( !z[i+1] || (winIsDirSep(z[i+1]) && (i!=0)) ){
        جاری;
      }
      z[j++] = osGetenv?'/':'\\';
    }ورنہ{
      z[j++] = z[i];
    }
  }
  جب(j<i) z[j++] = '\0';
}

#تعریف_کرو SQLITE_MAX_SYMLINKS 100

ساکن صحیح mk_بھرا_Pathname(
  ثابت حرف *z_راستہ,              /* Input path */
  حرف *z_برآمد,                     /* Output buffer */
  صحیح n_برآمد                        /* Allocated size of buffer zOut */
){
  صحیح n_راستہ = sqlite3Strlen30(z_راستہ);
  صحیح iOff = 0;
  اگر( z_راستہ[0]!='/' ){
    اگر( osGetcwd(z_برآمد, n_برآمد-2)==0 ){
      واپس winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno, "getcwd", z_راستہ);
    }
    iOff = sqlite3Strlen30(z_برآمد);
    z_برآمد[iOff++] = '/';
  }
  اگر( (iOff+n_راستہ+1)>n_برآمد ){
    /* SQLite assumes that xFullPathname() nul-terminates the output buffer
    ** even if it returns an error.  */
    z_برآمد[iOff] = '\0';
    واپس SQLITE_CANTOPEN_BKPT;
  }
  sqlite3_snprintf(n_برآمد-iOff, &z_برآمد[iOff], "%s", z_راستہ);
  واپس SQLITE_OK;
}
#اگر_ختم /* __CYGWIN__ */

/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
ساکن صحیح win_بھرا_Pathname_No_Mutex(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ثابت حرف *zRelative,        /* Possibly relative input path */
  صحیح n_بھرا,                    /* Size of output buffer in bytes */
  حرف *z_بھرا                   /* Output buffer */
){
  صحیح nByte;
  LPWSTR zConverted;
  حرف *z_برآمد;

  /* If this path name begins with "/X:" or "\\?\", where "X" is any
  ** alphabetic character, discard the initial "/" from the pathname.
  */
  اگر( zRelative[0]=='/' && (winIsDriveLetterAndColon(zRelative+1)
       || win_Is_Long_راستہ_سابقہ(zRelative+1)) ){
    zRelative++;
  }

  SimulateIOError( واپس SQLITE_ERROR );

#اگر_تعریف_شدہ __CYGWIN__
  اگر( osGetcwd ){
    z_بھرا[n_بھرا-1] = '\0';
    اگر( !winIsDriveLetterAndColon(zRelative) || !winIsDirSep(zRelative[2]) ){
      صحیح rc = SQLITE_OK;
      صحیح nLink = 1;              /* Number of symbolic links followed so far */
      ثابت حرف *z_اندر = zRelative; /* Input path for each iteration of loop */
      حرف *zDel = 0;
      ساخت stat buf;

      غیر_مستعمل_PARAMETER(pVfs);

      کرو {
        /* Call lstat() on path zIn. Set bLink to true if the path
        ** is a symbolic link, or false otherwise.  */
        صحیح bLink = 0;
        اگر( osLstat && osReadlink ) {
          اگر( osLstat(z_اندر, &buf)!=0 ){
            صحیح myErrno = osErrno;
            اگر( myErrno!=ENOENT ){
              rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)myErrno,
                               "lstat", z_اندر);
            }
          }ورنہ{
            bLink = ((buf.st_منوال & 0170000) == 0120000);
          }

          اگر( bLink ){
            اگر( zDel==0 ){
              zDel = sqlite3MallocZero(n_بھرا);
              اگر( zDel==0 ) rc = SQLITE_NOMEM;
            }ورنہ اگر( ++nLink>SQLITE_MAX_SYMLINKS ){
              rc = SQLITE_CANTOPEN_BKPT;
            }

            اگر( rc==SQLITE_OK ){
              nByte = osReadlink(z_اندر, zDel, n_بھرا-1);
              اگر( nByte ==(DWORD)-1 ){
                rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno,
                                 "readlink", z_اندر);
              }ورنہ{
                اگر( zDel[0]!='/' ){
                  صحیح n;
                  برائے(n = sqlite3Strlen30(z_اندر); n>0 && z_اندر[n-1]!='/'; n--);
                  اگر( nByte+n+1>n_بھرا ){
                    rc = SQLITE_CANTOPEN_BKPT;
                  }ورنہ{
                    memmove(&zDel[n], zDel, nByte+1);
                    حافظہ_نقل(zDel, z_اندر, n);
                    nByte += n;
                  }
                }
                zDel[nByte] = '\0';
              }
            }

            z_اندر = zDel;
          }
        }

        أكد( rc!=SQLITE_OK || z_اندر!=z_بھرا || z_اندر[0]=='/' );
        اگر( rc==SQLITE_OK && z_اندر!=z_بھرا ){
          rc = mk_بھرا_Pathname(z_اندر, z_بھرا, n_بھرا);
        }
        اگر( bLink==0 ) توڑو;
        z_اندر = z_بھرا;
      }جب( rc==SQLITE_OK );

      sqlite3_free(zDel);
      winSimplifyName(z_بھرا);
      واپس rc;
    }
  }
#اگر_ختم /* __CYGWIN__ */

#اگر defined(_WIN32)
  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing. This function could fail if, for example, the
  ** current working directory has been unlinked.
  */
  SimulateIOError( واپس SQLITE_ERROR );
  اگر ( sqlite3_ڈیٹا_directory && !winIsVerbatimPathname(zRelative) ){
    /*
    ** NOTE: We are dealing with a relative path name and the data
    **       directory has been set.  Therefore, use it as the basis
    **       for converting the relative path name to an absolute
    **       one by prepending the data directory and a backslash.
    */
    sqlite3_snprintf(MIN(n_بھرا, pVfs->mxPathname), z_بھرا, "%s%c%s",
                     sqlite3_ڈیٹا_directory, winGetDirSep(), zRelative);
    واپس SQLITE_OK;
  }
#اگر_ختم
  zConverted = winConvertFromUtf8Filename(zRelative);
  اگر( zConverted==0 ){
    واپس SQLITE_IOERR_NOMEM_BKPT;
  }
  {
    LPWSTR z_عارضی;
    nByte = os_Get_بھرا_راستہ_Name_W(zConverted, 0, 0, 0);
    اگر( nByte==0 ){
      sqlite3_free(zConverted);
      واپس winLogError(SQLITE_CANTOPEN_FULLPATH, os_Get_آخری_Error(),
                         "winFullPathname1", zRelative);
    }
    nByte += 3;
    z_عارضی = sqlite3MallocZero( (u64)nByte*حجم(WCHAR) );
    اگر( z_عارضی==0 ){
      sqlite3_free(zConverted);
      واپس SQLITE_IOERR_NOMEM_BKPT;
    }
    nByte = os_Get_بھرا_راستہ_Name_W(zConverted, nByte, z_عارضی, 0);
    اگر( nByte==0 ){
      sqlite3_free(zConverted);
      sqlite3_free(z_عارضی);
      واپس winLogError(SQLITE_CANTOPEN_FULLPATH, os_Get_آخری_Error(),
                         "winFullPathname2", zRelative);
    }
    sqlite3_free(zConverted);
    z_برآمد = win_Unicode_مقصد_Utf8(z_عارضی);
    sqlite3_free(z_عارضی);
  }
  اگر( z_برآمد ){
#اگر_تعریف_شدہ __CYGWIN__
    اگر( memcmp(z_برآمد, "\\\\?\\", 4) ){
      sqlite3_snprintf(MIN(n_بھرا, pVfs->mxPathname), z_بھرا, "%s", z_برآمد);
    }ورنہ اگر( memcmp(z_برآمد+4, "UNC\\", 4) ){
      sqlite3_snprintf(MIN(n_بھرا, pVfs->mxPathname), z_بھرا, "%s", z_برآمد+4);
    }ورنہ{
      حرف *p = z_برآمد+6;
      *p = '\\';
      اگر( osGetcwd ){
        /* On Cygwin, UNC paths use forward slashes */
        جب( *p ){
          اگر( *p=='\\' ) *p = '/';
          ++p;
        }
      }
      sqlite3_snprintf(MIN(n_بھرا, pVfs->mxPathname), z_بھرا, "%s", z_برآمد+6);
    }
#ورنہ
    sqlite3_snprintf(MIN(n_بھرا, pVfs->mxPathname), z_بھرا, "%s", z_برآمد);
#اگر_ختم /* __CYGWIN__ */
    sqlite3_free(z_برآمد);
    واپس SQLITE_OK;
  }ورنہ{
    واپس SQLITE_IOERR_NOMEM_BKPT;
  }
}
ساکن صحیح win_بھرا_Pathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ثابت حرف *zRelative,        /* Possibly relative input path */
  صحیح n_بھرا,                    /* Size of output buffer in bytes */
  حرف *z_بھرا                   /* Output buffer */
){
  صحیح rc;
  MUTEX_LOGIC( sqlite3_mutex *pMutex; )
  MUTEX_LOGIC( pMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR); )
  sqlite3_mutex_enter(pMutex);
  rc = win_بھرا_Pathname_No_Mutex(pVfs, zRelative, n_بھرا, z_بھرا);
  sqlite3_mutex_leave(pMutex);
  واپس rc;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
ساکن خالی *winDlOpen(sqlite3_vfs *pVfs, ثابت حرف *zFilename){
  HANDLE h = 0;
  LPWSTR zConverted = winConvertFromUtf8Filename(zFilename);
  غیر_مستعمل_PARAMETER(pVfs);
  اگر( zConverted==0 ){
    OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (خالی*)0));
    واپس 0;
  }
  h = os_بھرو_Library_W ? os_بھرو_Library_W(zConverted) : 0;
  OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (خالی*)h));
  sqlite3_free(zConverted);
  واپس (خالی*)h;
}
ساکن خالی winDlError(sqlite3_vfs *pVfs, صحیح nBuf, حرف *z_Buf_برآمد){
  غیر_مستعمل_PARAMETER(pVfs);
  win_Get_آخری_Error_پیغام(os_Get_آخری_Error(), nBuf, z_Buf_برآمد);
}
ساکن خالی (*winDlSym(sqlite3_vfs *pVfs,خالی *pH,ثابت حرف *zSym))(خالی){
  FARPROC proc = 0;
  غیر_مستعمل_PARAMETER(pVfs);
  proc = os_Get_Proc_پتہ_A ? os_Get_Proc_پتہ_A((HANDLE)pH, zSym) : 0;
  OSTRACE(("DLSYM handle=%p, symbol=%s, address=%p\n",
           (خالی*)pH, zSym, (خالی*)proc));
  واپس (خالی(*)(خالی))proc;
}
ساکن خالی winDlClose(sqlite3_vfs *pVfs, خالی *pHandle){
  غیر_مستعمل_PARAMETER(pVfs);
  اگر( osFreeLibrary!=0 ) osFreeLibrary((HANDLE)pHandle);
  OSTRACE(("DLCLOSE handle=%p\n", (خالی*)pHandle));
}
#ورنہ /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #تعریف_کرو winDlOpen  0
  #تعریف_کرو winDlError 0
  #تعریف_کرو winDlSym   0
  #تعریف_کرو winDlClose 0
#اگر_ختم

/* State information for the randomness gatherer. */
قسمتعریف ساخت EntropyGatherer EntropyGatherer;
ساخت EntropyGatherer {
  غیرعلامتی حرف *a;   /* Gather entropy into this buffer */
  صحیح na;             /* Size of a[] in bytes */
  صحیح i;              /* XOR next input into a[i] */
  صحیح nXor;           /* Number of XOR operations done */
};

#اگر !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
/* Mix sz bytes of entropy into p. */
ساکن خالی xor_حافظہ(EntropyGatherer *p, غیرعلامتی حرف *x, صحیح sz){
  صحیح j, k;
  برائے(j=0, k=p->i; j<sz; j++){
    p->a[k++] ^= x[j];
    اگر( k>=p->na ) k = 0;
  }
  p->i = k;
  p->nXor += sz;
}
#اگر_ختم /* !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS) */

/*
** Write up to nBuf bytes of randomness into zBuf.
*/
ساکن صحیح winRandomness(sqlite3_vfs *pVfs, صحیح nBuf, حرف *zBuf){
#اگر defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS)
  غیر_مستعمل_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  واپس nBuf;
#ورنہ
  EntropyGatherer e;
  غیر_مستعمل_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  e.a = (غیرعلامتی حرف*)zBuf;
  e.na = nBuf;
  e.nXor = 0;
  e.i = 0;
  {
    FILETIME x;
    os_Get_System_وقت_As_File_وقت(&x);
    xor_حافظہ(&e, (غیرعلامتی حرف*)&x, حجم(x));
  }
  {
    DWORD pid = osGetCurrentProcessId();
    xor_حافظہ(&e, (غیرعلامتی حرف*)&pid, حجم(pid));
  }
  {
    ULONGLONG cnt = osGetTickCount64();
    xor_حافظہ(&e, (غیرعلامتی حرف*)&cnt, حجم(cnt));
  }
  {
    LARGE_INTEGER i;
    os_Query_Performance_شمار(&i);
    xor_حافظہ(&e, (غیرعلامتی حرف*)&i, حجم(i));
  }
#اگر SQLITE_WIN32_USE_UUID
#اگر_تعریف_شدہ SQLITE_UWP
# error SQLITE_WIN32_USE_UUID is incompatible with SQLITE_UWP
#اگر_ختم
  {
    UUID id;
    memset(&id, 0, حجم(id));
    osUuidCreate(&id);
    xor_حافظہ(&e, (غیرعلامتی حرف*)&id, حجم(id));
    memset(&id, 0, حجم(UUID));
    osUuidCreateSequential(&id);
    xor_حافظہ(&e, (غیرعلامتی حرف*)&id, حجم(id));
  }
#اگر_ختم /* SQLITE_WIN32_USE_UUID */
  واپس e.nXor>nBuf ? nBuf : e.nXor;
#اگر_ختم /* defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS) */
}


/*
** Sleep for a little while.  Return the amount of time slept.
*/
ساکن صحیح winSleep(sqlite3_vfs *pVfs, صحیح microsec){
  osSleep((microsec+999)/1000);
  غیر_مستعمل_PARAMETER(pVfs);
  واپس ((microsec+999)/1000)*1000;
}

/*
** The following variable, if set to a non-zero value, is interpreted as
** the number of seconds since 1970 and is used to set the result of
** sqlite3OsCurrentTime() during testing.
*/
#اگر_تعریف_شدہ SQLITE_TEST
صحیح sqlite3_current_وقت = 0;  /* Fake system time in seconds since 1970. */
#اگر_ختم

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
ساکن صحیح win_Current_وقت_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *piNow){
  /* FILETIME structure is a 64-bit value representing the number of
     100-nanosecond intervals since January 1, 1601 (= JD 2305813.5).
  */
  FILETIME ft;
  ساکن ثابت sqlite3_int64 winFiletimeEpoch = 23058135*(sqlite3_int64)8640000;
#اگر_تعریف_شدہ SQLITE_TEST
  ساکن ثابت sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
#اگر_ختم
  /* 2^32 - to avoid use of LL and warnings in gcc */
  ساکن ثابت sqlite3_int64 max32_ثنائی_ہندسہ_قدر =
      (sqlite3_int64)2000000000 + (sqlite3_int64)2000000000 +
      (sqlite3_int64)294967296;

  os_Get_System_وقت_As_File_وقت( &ft );
  *piNow = winFiletimeEpoch +
            ((((sqlite3_int64)ft.dw_بلند_Date_وقت)*max32_ثنائی_ہندسہ_قدر) +
               (sqlite3_int64)ft.dw_کم_Date_وقت)/(sqlite3_int64)10000;

#اگر_تعریف_شدہ SQLITE_TEST
  اگر( sqlite3_current_وقت ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_وقت + unixEpoch;
  }
#اگر_ختم
  غیر_مستعمل_PARAMETER(pVfs);
  واپس SQLITE_OK;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
ساکن صحیح win_Current_وقت(sqlite3_vfs *pVfs, دوہرا *prNow){
  صحیح rc;
  sqlite3_int64 i;
  rc = win_Current_وقت_Int64(pVfs, &i);
  اگر( !rc ){
    *prNow = i/86400000.0;
  }
  واپس rc;
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
ساکن صحیح win_Get_آخری_Error(sqlite3_vfs *pVfs, صحیح nBuf, حرف *zBuf){
  DWORD e = os_Get_آخری_Error();
  غیر_مستعمل_PARAMETER(pVfs);
  اگر( nBuf>0 ) win_Get_آخری_Error_پیغام(e, nBuf, zBuf);
  واپس e;
}

/*
** Initialize and deinitialize the operating system interface.
*/
صحیح sqlite3_os_init(خالی){
  ساکن sqlite3_vfs winVfs = {
    3,                     /* iVersion */
    حجم(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_راستہ_بائٹیں, /* mxPathname */
    0,                     /* pNext */
    "win32",               /* zName */
    &win_App_ڈیٹا,           /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_بھرا_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_وقت,        /* xCurrentTime */
    win_Get_آخری_Error,       /* xGetLastError */
    win_Current_وقت_Int64,   /* xCurrentTimeInt64 */
    win_سیٹ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_اگلا_System_Call,     /* xNextSystemCall */
  };
  ساکن sqlite3_vfs win_Long_راستہ_Vfs = {
    3,                     /* iVersion */
    حجم(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_راستہ_بائٹیں, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath",      /* zName */
    &win_App_ڈیٹا,           /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_بھرا_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_وقت,        /* xCurrentTime */
    win_Get_آخری_Error,       /* xGetLastError */
    win_Current_وقت_Int64,   /* xCurrentTimeInt64 */
    win_سیٹ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_اگلا_System_Call,     /* xNextSystemCall */
  };
  ساکن sqlite3_vfs winNolockVfs = {
    3,                     /* iVersion */
    حجم(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_راستہ_بائٹیں, /* mxPathname */
    0,                     /* pNext */
    "win32-none",          /* zName */
    &win_Nolock_App_ڈیٹا,     /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_بھرا_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_وقت,        /* xCurrentTime */
    win_Get_آخری_Error,       /* xGetLastError */
    win_Current_وقت_Int64,   /* xCurrentTimeInt64 */
    win_سیٹ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_اگلا_System_Call,     /* xNextSystemCall */
  };
  ساکن sqlite3_vfs win_Long_راستہ_Nolock_Vfs = {
    3,                     /* iVersion */
    حجم(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_راستہ_بائٹیں, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath-none", /* zName */
    &win_Nolock_App_ڈیٹا,     /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_بھرا_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_وقت,        /* xCurrentTime */
    win_Get_آخری_Error,       /* xGetLastError */
    win_Current_وقت_Int64,   /* xCurrentTimeInt64 */
    win_سیٹ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_اگلا_System_Call,     /* xNextSystemCall */
  };

  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  أكد( Array_سائز(aSyscall)==60 );
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
  أكد( winSysInfo.dw_تخصیص_Granularity>0 );
  أكد( winSysInfo.dw_صفحہ_سائز>0 );

  sqlite3_vfs_رجسٹر(&winVfs, 1);
  sqlite3_vfs_رجسٹر(&win_Long_راستہ_Vfs, 0);
  sqlite3_vfs_رجسٹر(&winNolockVfs, 0);
  sqlite3_vfs_رجسٹر(&win_Long_راستہ_Nolock_Vfs, 0);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
  win_Big_تالا = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#اگر_ختم

  واپس SQLITE_OK;
}

صحیح sqlite3_os_انتہا(خالی){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
  win_Big_تالا = 0;
#اگر_ختم
  واپس SQLITE_OK;
}

#اگر_ختم /* SQLITE_OS_WIN */
