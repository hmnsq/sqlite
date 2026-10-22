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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਜੇ SQLITE_OS_WIN               /* This file is used for Windows only */

/*
** Include code that is common to all os_*.c files
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "os_win.h"

/*
** This constant should already be defined (in the "WinDef.h" SDK file).
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MAX_ਰਸਤਾ
#  define MAX_ਰਸਤਾ                      (260)
#ਜੇ_ਅੰਤ

/*
** Maximum pathname length (in chars) for Win32.  This should normally be
** MAX_PATH.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_MAX_ਰਸਤਾ_CHARS
#  define SQLITE_WIN32_MAX_ਰਸਤਾ_CHARS   (MAX_ਰਸਤਾ)
#ਜੇ_ਅੰਤ

/*
** This constant should already be defined (in the "WinNT.h" SDK file).
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ UNICODE_STRING_MAX_CHARS
#  define UNICODE_STRING_MAX_CHARS      (32767)
#ਜੇ_ਅੰਤ

/*
** Maximum pathname length (in chars) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WINNT_MAX_ਰਸਤਾ_CHARS
#  define SQLITE_WINNT_MAX_ਰਸਤਾ_CHARS   (UNICODE_STRING_MAX_CHARS)
#ਜੇ_ਅੰਤ

/*
** Maximum pathname length (in bytes) for Win32.  The MAX_PATH macro is in
** characters, so we allocate 4 bytes per character assuming worst-case of
** 4-bytes-per-character for UTF8.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_MAX_ਰਸਤਾ_ਬਾਈਟਾਂ
#  define SQLITE_WIN32_MAX_ਰਸਤਾ_ਬਾਈਟਾਂ   (SQLITE_WIN32_MAX_ਰਸਤਾ_CHARS*4)
#ਜੇ_ਅੰਤ

/*
** Maximum pathname length (in bytes) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS * sizeof(WCHAR).
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WINNT_MAX_ਰਸਤਾ_ਬਾਈਟਾਂ
#  define SQLITE_WINNT_MAX_ਰਸਤਾ_ਬਾਈਟਾਂ   \
                            (ਆਕਾਰ(WCHAR) * SQLITE_WINNT_MAX_ਰਸਤਾ_CHARS)
#ਜੇ_ਅੰਤ

/*
** Maximum error message length (in chars) for WinRT.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_MAX_ERRMSG_CHARS
#  define SQLITE_WIN32_MAX_ERRMSG_CHARS (1024)
#ਜੇ_ਅੰਤ

/*
** Returns non-zero if the character should be treated as a directory
** separator.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ winIsDirSep
#  define winIsDirSep(a)                (((a) == '/') || ((a) == '\\'))
#ਜੇ_ਅੰਤ

/*
** This macro is used when a local variable is set to a value that is
** [sometimes] not used by the code (e.g. via conditional compilation).
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ਅਣਵਰਤਿਆ_VARIABLE_ਮੁੱਲ
#  define ਅਣਵਰਤਿਆ_VARIABLE_ਮੁੱਲ(x)      (ਖਾਲੀ)(x)
#ਜੇ_ਅੰਤ

/*
** Returns the character that should be used as the directory separator.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ winGetDirSep
#  define winGetDirSep()                '\\'
#ਜੇ_ਅੰਤ

/*
** Some older Microsoft compilers lack the following definition.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ INVALID_FILE_ਗੁਣ
# define INVALID_FILE_ਗੁਣ ((DWORD)-1)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FILE_FLAG_ਨਕਾਬ
# define FILE_FLAG_ਨਕਾਬ          (0xFF3C0000)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FILE_ATTRIBUTE_ਨਕਾਬ
# define FILE_ATTRIBUTE_ਨਕਾਬ     (0x0003FFF7)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
/* Forward references to structures used for WAL */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ winShm winShm;           /* A connection to shared-memory */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ win_Shm_ਗੰਢ win_Shm_ਗੰਢ;   /* A region of shared-memory */
#ਜੇ_ਅੰਤ

/*
** The winFile structure is a subclass of sqlite3_file* specific to the win32
** portability layer.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ winFile winFile;
ਬਣਤਰ winFile {
  ਅਚਲ sqlite3_io_methods *p_ਵਿਧੀ; /*** Must be first ***/
  sqlite3_vfs *pVfs;      /* The VFS used to open this file */
  HANDLE h;               /* Handle for accessing the file */
  u8 locktype;            /* Type of lock currently held on this file */
  ਛੋਟਾ shared_ਤਾਲਾ_Byte;   /* Randomly chosen byte used as a shared lock */
  u8 ctrlFlags;           /* Flags.  See WINFILE_* below */
  DWORD ਆਖਰੀ_Errno;        /* The Windows errno from the last I/O error */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  winShm *pShm;           /* Instance of shared memory on this file */
#ਜੇ_ਅੰਤ
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ;      /* Full pathname of this file */
  ਪੂਰਨ szChunk;            /* Chunk size configured by FCNTL_CHUNK_SIZE */
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  ਪੂਰਨ n_Fetch_ਬਾਹਰਲਾ;                /* Number of outstanding xFetch references */
  HANDLE hMap;                  /* Handle for accessing memory mapping */
  ਖਾਲੀ *pMapRegion;             /* Area memory mapped */
  sqlite3_int64 mmap_ਮਾਪ;       /* Size of mapped region */
  sqlite3_int64 mmap_ਮਾਪ_Max;    /* Configured FCNTL_MMAP_SIZE value */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
  DWORD iBusyTimeout;        /* Wait this many millisec on locks */
  ਪੂਰਨ b_ਖੰਡ_On_Connect;
#ਜੇ_ਅੰਤ
};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
# define winFileBusyTimeout(pDbFd) pDbFd->iBusyTimeout
#ਨਹੀਂਤਾਂ
# define winFileBusyTimeout(pDbFd) 0
#ਜੇ_ਅੰਤ

/*
** The winVfsAppData structure is used for the pAppData member for all of the
** Win32 VFS variants.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ win_Vfs_App_ਅੰਕੜੇ win_Vfs_App_ਅੰਕੜੇ;
ਬਣਤਰ win_Vfs_App_ਅੰਕੜੇ {
  ਅਚਲ sqlite3_io_methods *p_ਵਿਧੀ; /* The file I/O methods to use. */
  ਖਾਲੀ *p_App_ਅੰਕੜੇ;                    /* The extra pAppData, if any. */
  BOOL b_No_ਤਾਲਾ;                      /* Non-zero if locking is disabled. */
};

/*
** Allowed values for winFile.ctrlFlags
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WINFILE_RDONLY          0x02   /* Connection is read only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WINFILE_PERSIST_WAL     0x04   /* Persistent WAL mode */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WINFILE_PSOW            0x10   /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */

/*
 * The size of the buffer used by sqlite3_win32_write_debug().
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_DBG_BUF_ਮਾਪ
#  define SQLITE_WIN32_DBG_BUF_ਮਾਪ   ((ਪੂਰਨ)(4096-ਆਕਾਰ(DWORD)))
#ਜੇ_ਅੰਤ

/*
 * If compiled with SQLITE_WIN32_MALLOC on Windows, we will use the
 * various Win32 API heap functions instead of our own.
 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_MALLOC

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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_ਢੇਰ_CREATE
#  define SQLITE_WIN32_ਢੇਰ_CREATE        (TRUE)
#ਜੇ_ਅੰਤ

/*
 * This is the maximum possible initial size of the Win32-specific heap, in
 * bytes.
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_ਢੇਰ_MAX_INIT_ਮਾਪ
#  define SQLITE_WIN32_ਢੇਰ_MAX_INIT_ਮਾਪ (4294967295U)
#ਜੇ_ਅੰਤ

/*
 * This is the extra space for the initial size of the Win32-specific heap,
 * in bytes.  This value may be zero.
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_ਢੇਰ_INIT_EXTRA
#  define SQLITE_WIN32_ਢੇਰ_INIT_EXTRA  (4194304)
#ਜੇ_ਅੰਤ

/*
 * Calculate the maximum legal cache size, in pages, based on the maximum
 * possible initial heap size and the default page size, setting aside the
 * needed extra space.
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_MAX_CACHE_ਮਾਪ
#  define SQLITE_WIN32_MAX_CACHE_ਮਾਪ   (((SQLITE_WIN32_ਢੇਰ_MAX_INIT_ਮਾਪ) - \
                                          (SQLITE_WIN32_ਢੇਰ_INIT_EXTRA)) / \
                                         (SQLITE_DEFAULT_ਪੰਨਾ_ਮਾਪ))
#ਜੇ_ਅੰਤ

/*
 * This is cache size used in the calculation of the initial size of the
 * Win32-specific heap.  It cannot be negative.
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_CACHE_ਮਾਪ
#  ਜੇ SQLITE_DEFAULT_CACHE_ਮਾਪ>=0
#    define SQLITE_WIN32_CACHE_ਮਾਪ     (SQLITE_DEFAULT_CACHE_ਮਾਪ)
#  ਨਹੀਂਤਾਂ
#    define SQLITE_WIN32_CACHE_ਮਾਪ     (-(SQLITE_DEFAULT_CACHE_ਮਾਪ))
#  endif
#ਜੇ_ਅੰਤ

/*
 * Make sure that the calculated cache size, in pages, cannot cause the
 * initial size of the Win32-specific heap to exceed the maximum amount
 * of memory that can be specified in the call to HeapCreate.
 */
#ਜੇ SQLITE_WIN32_CACHE_ਮਾਪ>SQLITE_WIN32_MAX_CACHE_ਮਾਪ
#  undef SQLITE_WIN32_CACHE_ਮਾਪ
#  define SQLITE_WIN32_CACHE_ਮਾਪ       (2000)
#ਜੇ_ਅੰਤ

/*
 * The initial size of the Win32-specific heap.  This value may be zero.
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_ਢੇਰ_INIT_ਮਾਪ
#  define SQLITE_WIN32_ਢੇਰ_INIT_ਮਾਪ   ((SQLITE_WIN32_CACHE_ਮਾਪ) * \
                                         (SQLITE_DEFAULT_ਪੰਨਾ_ਮਾਪ) + \
                                         (SQLITE_WIN32_ਢੇਰ_INIT_EXTRA))
#ਜੇ_ਅੰਤ

/*
 * The maximum size of the Win32-specific heap.  This value may be zero.
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_ਢੇਰ_MAX_ਮਾਪ
#  define SQLITE_WIN32_ਢੇਰ_MAX_ਮਾਪ    (0)
#ਜੇ_ਅੰਤ

/*
 * The extra flags to use in calls to the Win32 heap APIs.  This value may be
 * zero for the default behavior.
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_ਢੇਰ_FLAGS
#  define SQLITE_WIN32_ਢੇਰ_FLAGS       (0)
#ਜੇ_ਅੰਤ


/*
** The winMemData structure stores information required by the Win32-specific
** sqlite3_mem_methods implementation.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ win_Mem_ਅੰਕੜੇ win_Mem_ਅੰਕੜੇ;
ਬਣਤਰ win_Mem_ਅੰਕੜੇ {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  u32 magic1;   /* Magic number to detect structure corruption. */
#ਜੇ_ਅੰਤ
  HANDLE h_ਢੇਰ; /* The handle to our heap. */
  BOOL bOwned;  /* Do we own the heap (i.e. destroy it on shutdown)? */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  u32 magic2;   /* Magic number to detect structure corruption. */
#ਜੇ_ਅੰਤ
};

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WINMEM_MAGIC1     0x42b2830b
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WINMEM_MAGIC2     0xbd4d7cf4
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਬਣਤਰ win_Mem_ਅੰਕੜੇ win_mem_ਅੰਕੜੇ = {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  WINMEM_MAGIC1,
#ਜੇ_ਅੰਤ
  ਨਿਰਾ, FALSE
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  ,WINMEM_MAGIC2
#ਜੇ_ਅੰਤ
};

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ winMemAssertMagic1() ਪ੍ਰਤਿਜ੍ਞਾ( win_mem_ਅੰਕੜੇ.magic1==WINMEM_MAGIC1 )
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ winMemAssertMagic2() ਪ੍ਰਤਿਜ੍ਞਾ( win_mem_ਅੰਕੜੇ.magic2==WINMEM_MAGIC2 )
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ winMemAssertMagic()  winMemAssertMagic1(); winMemAssertMagic2();
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ winMemAssertMagic()
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ win_Mem_Get_ਅੰਕੜੇ_Ptr()  &win_mem_ਅੰਕੜੇ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ win_Mem_Get_ਢੇਰ()     win_mem_ਅੰਕੜੇ.h_ਢੇਰ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ winMemGetOwned()    win_mem_ਅੰਕੜੇ.bOwned

ਸਥਿਰ ਖਾਲੀ *winMemMalloc(ਪੂਰਨ n_ਬਾਈਟਾਂ);
ਸਥਿਰ ਖਾਲੀ winMemFree(ਖਾਲੀ *p_ਪੂਰਵ);
ਸਥਿਰ ਖਾਲੀ *winMemRealloc(ਖਾਲੀ *p_ਪੂਰਵ, ਪੂਰਨ n_ਬਾਈਟਾਂ);
ਸਥਿਰ ਪੂਰਨ win_Mem_ਮਾਪ(ਖਾਲੀ *p);
ਸਥਿਰ ਪੂਰਨ winMemRoundup(ਪੂਰਨ n);
ਸਥਿਰ ਪੂਰਨ winMemInit(ਖਾਲੀ *p_App_ਅੰਕੜੇ);
ਸਥਿਰ ਖਾਲੀ winMemShutdown(ਖਾਲੀ *p_App_ਅੰਕੜੇ);

ਅਚਲ sqlite3_mem_methods *sqlite3MemGetWin32(ਖਾਲੀ);
#ਜੇ_ਅੰਤ /* SQLITE_WIN32_MALLOC */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SYSCALL
#  define SYSCALL sqlite3_syscall_ptr
#ਜੇ_ਅੰਤ

/*
** Many system calls are accessed through pointer-to-functions so that
** they may be overridden at runtime to facilitate fault injection during
** testing and sandboxing.  The following array holds the names and pointers
** to all overrideable system calls.
*/
ਸਥਿਰ ਬਣਤਰ win_syscall {
  ਅਚਲ ਅੱਖਰ *zName;            /* Name of the system call */
  sqlite3_syscall_ptr pCurrent; /* Current value of the system call */
  sqlite3_syscall_ptr pDefault; /* Default value */
} aSyscall[] = {
  { "AreFileApisANSI",         (SYSCALL)AreFileApisANSI,         0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osAreFileApisANSI ((BOOL(WINAPI*)(VOID))aSyscall[0].pCurrent)

  { "CloseHandle",             (SYSCALL)CloseHandle,             0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osCloseHandle ((BOOL(WINAPI*)(HANDLE))aSyscall[1].pCurrent)


  { "CreateFileW",             (SYSCALL)CreateFileW,             0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osCreateFileW ((HANDLE(WINAPI*)(LPCWSTR,DWORD,DWORD, \
        LPSECURITY_ਗੁਣ,DWORD,DWORD,HANDLE))aSyscall[2].pCurrent)

#ਜੇ defined(SQLITE_UWP)
  { "CreateFileMappingFromApp",(SYSCALL)CreateFileMappingFromApp,0 },
#ਨਹੀਂਤਾਂ
  { "CreateFileMappingFromApp",(SYSCALL)0,                       0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osCreateFileMappingFromApp ((HANDLE(WINAPI*)(HANDLE,\
        PSECURITY_ਗੁਣ, \
        ULONG,ULONG64,PCWSTR))aSyscall[3].pCurrent)

#ਜੇ !defined(SQLITE_UWP)
  { "CreateFileMappingW",      (SYSCALL)CreateFileMappingW,      0 },
#ਨਹੀਂਤਾਂ
  { "CreateFileMappingW",      (SYSCALL)0,                       0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osCreateFileMappingW ((HANDLE(WINAPI*)(HANDLE,LPSECURITY_ਗੁਣ, \
        DWORD,DWORD,DWORD,LPCWSTR))aSyscall[4].pCurrent)

  { "DeleteFileW",             (SYSCALL)DeleteFileW,             0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osDeleteFileW ((BOOL(WINAPI*)(LPCWSTR))aSyscall[5].pCurrent)

  { "FlushFileBuffers",        (SYSCALL)FlushFileBuffers,        0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osFlushFileBuffers ((BOOL(WINAPI*)(HANDLE))aSyscall[6].pCurrent)

  { "FormatMessageW",          (SYSCALL)Format_ਸੁਨੇਹਾ_W,          0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Format_ਸੁਨੇਹਾ_W ((DWORD(WINAPI*)(DWORD,LPCVOID,DWORD,DWORD,LPWSTR, \
        DWORD,va_list*))aSyscall[7].pCurrent)

#ਜੇ !defined(SQLITE_OMIT_ਲੱਦੋ_EXTENSION) && !defined(SQLITE_UWP)
  { "FreeLibrary",             (SYSCALL)FreeLibrary,             0 },
#ਨਹੀਂਤਾਂ
  { "FreeLibrary",             (SYSCALL)0,                       0 },
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osFreeLibrary ((BOOL(WINAPI*)(HMODULE))aSyscall[8].pCurrent)

  { "GetCurrentProcessId",     (SYSCALL)GetCurrentProcessId,     0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osGetCurrentProcessId ((DWORD(WINAPI*)(VOID))aSyscall[9].pCurrent)

  { "GetFileAttributesW",      (SYSCALL)Get_File_ਗੁਣ_W,      0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Get_File_ਗੁਣ_W ((DWORD(WINAPI*)(LPCWSTR))aSyscall[10].pCurrent)

  { "GetFileAttributesExW",    (SYSCALL)Get_File_ਗੁਣ_Ex_W,    0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Get_File_ਗੁਣ_Ex_W ((BOOL(WINAPI*)(LPCWSTR,GET_FILEEX_INFO_ਪੱਧਰ, \
        LPVOID))aSyscall[11].pCurrent)

  { "GetFileSizeEx",           (SYSCALL)Get_File_ਮਾਪ_Ex,           0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Get_File_ਮਾਪ_Ex ((BOOL(WINAPI*)(HANDLE, \
                        PLARGE_INTEGER))aSyscall[12].pCurrent)

  { "GetFullPathNameW",        (SYSCALL)Get_ਭਰਿਆ_ਰਸਤਾ_Name_W,        0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Get_ਭਰਿਆ_ਰਸਤਾ_Name_W ((DWORD(WINAPI*)(LPCWSTR,DWORD,LPWSTR, \
        LPWSTR*))aSyscall[13].pCurrent)

  { "GetLastError",            (SYSCALL)Get_ਆਖਰੀ_Error,            0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Get_ਆਖਰੀ_Error ((DWORD(WINAPI*)(VOID))aSyscall[14].pCurrent)

#ਜੇ !defined(SQLITE_OMIT_ਲੱਦੋ_EXTENSION) && !defined(SQLITE_UWP)
  { "GetProcAddressA",         (SYSCALL)Get_Proc_ਪਤਾ,          0 },
#ਨਹੀਂਤਾਂ
  { "GetProcAddressA",         (SYSCALL)0,                       0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Get_Proc_ਪਤਾ_A ((FARPROC(WINAPI*)(HMODULE, \
        LPCSTR))aSyscall[15].pCurrent)

  { "GetSystemInfo",           (SYSCALL)GetSystemInfo,           0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osGetSystemInfo ((VOID(WINAPI*)(LPSYSTEM_INFO))aSyscall[16].pCurrent)

  { "GetSystemTimeAsFileTime", (SYSCALL)Get_System_ਸਮਾਂ_As_File_ਸਮਾਂ, 0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Get_System_ਸਮਾਂ_As_File_ਸਮਾਂ ((VOID(WINAPI*)( \
        LPFILETIME))aSyscall[17].pCurrent)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
  { "GetTempPathW",            (SYSCALL)0,                       0 },
#ਨਹੀਂਤਾਂ
  { "GetTempPathW",            (SYSCALL)Get_ਆਰਜ਼ੀ_ਰਸਤਾ_W,            0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Get_ਆਰਜ਼ੀ_ਰਸਤਾ_W ((DWORD(WINAPI*)(DWORD,LPWSTR))aSyscall[18].pCurrent)

  { "GetTickCount64",          (SYSCALL)GetTickCount64,          0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osGetTickCount64 ((ULONGLONG(WINAPI*)(VOID))aSyscall[19].pCurrent)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
  { "HeapAlloc",               (SYSCALL)0,                       0 },
#ਨਹੀਂਤਾਂ
  { "HeapAlloc",               (SYSCALL)ਢੇਰ_Alloc,               0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_ਢੇਰ_Alloc ((LPVOID(WINAPI*)(HANDLE,DWORD, \
        ਮਾਪ_T))aSyscall[20].pCurrent)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
  { "HeapCreate",              (SYSCALL)0,                       0 },
#ਨਹੀਂਤਾਂ
  { "HeapCreate",              (SYSCALL)ਢੇਰ_Create,              0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_ਢੇਰ_Create ((HANDLE(WINAPI*)(DWORD,ਮਾਪ_T, \
        ਮਾਪ_T))aSyscall[21].pCurrent)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
  { "HeapDestroy",             (SYSCALL)0,                       0 },
#ਨਹੀਂਤਾਂ
  { "HeapDestroy",             (SYSCALL)ਢੇਰ_Destroy,             0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_ਢੇਰ_Destroy ((BOOL(WINAPI*)(HANDLE))aSyscall[22].pCurrent)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
  { "HeapFree",                (SYSCALL)0,                       0 },
#ਨਹੀਂਤਾਂ
  { "HeapFree",                (SYSCALL)ਢੇਰ_Free,                0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_ਢੇਰ_Free ((BOOL(WINAPI*)(HANDLE,DWORD,LPVOID))aSyscall[23].pCurrent)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
  { "HeapReAlloc",             (SYSCALL)0,                       0 },
#ਨਹੀਂਤਾਂ
  { "HeapReAlloc",             (SYSCALL)ਢੇਰ_Re_Alloc,             0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_ਢੇਰ_Re_Alloc ((LPVOID(WINAPI*)(HANDLE,DWORD,LPVOID, \
        ਮਾਪ_T))aSyscall[24].pCurrent)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
  { "HeapSize",                (SYSCALL)0,                       0 },
#ਨਹੀਂਤਾਂ
  { "HeapSize",                (SYSCALL)ਢੇਰ_ਮਾਪ,                0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_ਢੇਰ_ਮਾਪ ((ਮਾਪ_T(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[25].pCurrent)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
  { "HeapValidate",            (SYSCALL)0,                       0 },
#ਨਹੀਂਤਾਂ
  { "HeapValidate",            (SYSCALL)ਢੇਰ_Validate,            0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_ਢੇਰ_Validate ((BOOL(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[26].pCurrent)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
  { "HeapCompact",             (SYSCALL)0,                       0 },
#ਨਹੀਂਤਾਂ
  { "HeapCompact",             (SYSCALL)ਢੇਰ_Compact,             0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_ਢੇਰ_Compact ((UINT(WINAPI*)(HANDLE,DWORD))aSyscall[27].pCurrent)


#ਜੇ !defined(SQLITE_OMIT_ਲੱਦੋ_EXTENSION) && !defined(SQLITE_UWP)
  { "LoadLibraryW",            (SYSCALL)ਲੱਦੋ_Library_W,            0 },
#ਨਹੀਂਤਾਂ
  { "LoadLibraryW",            (SYSCALL)0,                       0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_ਲੱਦੋ_Library_W ((HMODULE(WINAPI*)(LPCWSTR))aSyscall[28].pCurrent)

  { "LocalFree",               (SYSCALL)LocalFree,               0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osLocalFree ((HLOCAL(WINAPI*)(HLOCAL))aSyscall[29].pCurrent)

  { "LockFileEx",              (SYSCALL)ਤਾਲਾ_File_Ex,              0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_ਤਾਲਾ_File_Ex ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[30].pCurrent)

#ਜੇ (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_ਮਾਪ>0) \
 && !defined(SQLITE_UWP)
  { "MapViewOfFile",           (SYSCALL)Map_View_ਦਾ_File,           0 },
#ਨਹੀਂਤਾਂ
  { "MapViewOfFile",           (SYSCALL)0,                       0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Map_View_ਦਾ_File ((LPVOID(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        ਮਾਪ_T))aSyscall[31].pCurrent)

#ਜੇ (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_ਮਾਪ>0) \
 && defined(SQLITE_UWP)
  { "MapViewOfFileFromApp",    (SYSCALL)Map_View_ਦਾ_File_From_App,   0 },
#ਨਹੀਂਤਾਂ
  { "MapViewOfFileFromApp",    (SYSCALL)0,                      0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Map_View_ਦਾ_File_From_App ((LPVOID(WINAPI*)(HANDLE,ULONG,ULONG64, \
        ਮਾਪ_T))aSyscall[32].pCurrent)

  { "MultiByteToWideChar",     (SYSCALL)Multi_Byte_ਮੰਜ਼ਿਲ_Wide_Char,     0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Multi_Byte_ਮੰਜ਼ਿਲ_Wide_Char ((ਪੂਰਨ(WINAPI*)(UINT,DWORD,LPCSTR,ਪੂਰਨ,LPWSTR, \
        ਪੂਰਨ))aSyscall[33].pCurrent)

  { "QueryPerformanceCounter", (SYSCALL)QueryPerformanceCounter, 0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osQueryPerformanceCounter ((BOOL(WINAPI*)( \
        LARGE_INTEGER*))aSyscall[34].pCurrent)

  { "ReadFile",                (SYSCALL)ReadFile,                0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osReadFile ((BOOL(WINAPI*)(HANDLE,LPVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[35].pCurrent)

  { "SetEndOfFile",            (SYSCALL)ਸੈੱਟ_ਸਿਰਾ_ਦਾ_File,            0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_ਸੈੱਟ_ਸਿਰਾ_ਦਾ_File ((BOOL(WINAPI*)(HANDLE))aSyscall[36].pCurrent)

  { "SetFilePointerEx",        (SYSCALL)ਸੈੱਟ_File_ਸੂਚਕ_Ex,        0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_ਸੈੱਟ_File_ਸੂਚਕ_Ex ((BOOL(WINAPI*)(HANDLE,LARGE_INTEGER,\
        PLARGE_INTEGER,DWORD))aSyscall[37].pCurrent)

  { "Sleep",                   (SYSCALL)Sleep,                   0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osSleep ((VOID(WINAPI*)(DWORD))aSyscall[38].pCurrent)

  { "UnlockFileEx",            (SYSCALL)UnlockFileEx,            0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osUnlockFileEx ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[39].pCurrent)

#ਜੇ !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_ਮਾਪ>0
  { "UnmapViewOfFile",         (SYSCALL)Unmap_View_ਦਾ_File,         0 },
#ਨਹੀਂਤਾਂ
  { "UnmapViewOfFile",         (SYSCALL)0,                       0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Unmap_View_ਦਾ_File ((BOOL(WINAPI*)(LPCVOID))aSyscall[40].pCurrent)

  { "WideCharToMultiByte",     (SYSCALL)Wide_Char_ਮੰਜ਼ਿਲ_Multi_Byte,     0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Wide_Char_ਮੰਜ਼ਿਲ_Multi_Byte ((ਪੂਰਨ(WINAPI*)(UINT,DWORD,LPCWSTR,ਪੂਰਨ,LPSTR,ਪੂਰਨ, \
        LPCSTR,LPBOOL))aSyscall[41].pCurrent)

  { "WriteFile",               (SYSCALL)WriteFile,               0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osWriteFile ((BOOL(WINAPI*)(HANDLE,LPCVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[42].pCurrent)

  { "WaitForSingleObject",     (SYSCALL)WaitForSingleObject,     0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osWaitForSingleObject ((DWORD(WINAPI*)(HANDLE, \
        DWORD))aSyscall[43].pCurrent)

  { "WaitForSingleObjectEx",   (SYSCALL)WaitForSingleObjectEx,   0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osWaitForSingleObjectEx ((DWORD(WINAPI*)(HANDLE,DWORD, \
        BOOL))aSyscall[44].pCurrent)

  { "OutputDebugStringA",      (SYSCALL)OutputDebugStringA,      0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osOutputDebugStringA ((VOID(WINAPI*)(LPCSTR))aSyscall[45].pCurrent)

  { "GetProcessHeap",          (SYSCALL)Get_Process_ਢੇਰ,          0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Get_Process_ਢੇਰ ((HANDLE(WINAPI*)(VOID))aSyscall[46].pCurrent)

/*
** NOTE: On some sub-platforms, the InterlockedCompareExchange "function"
**       is really just a macro that uses a compiler intrinsic (e.g. x64).
**       So do not try to make this is into a redefinable interface.
*/
#ਜੇ defined(InterlockedCompareExchange)
  { "InterlockedCompareExchange", (SYSCALL)0,                    0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osInterlockedCompareExchange InterlockedCompareExchange
#ਨਹੀਂਤਾਂ
  { "InterlockedCompareExchange", (SYSCALL)InterlockedCompareExchange, 0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osInterlockedCompareExchange ((LONG(WINAPI*)(LONG ਅਸਥਿਰ*,\
        LONG,LONG))aSyscall[47].pCurrent)
#ਜੇ_ਅੰਤ /* defined(InterlockedCompareExchange) */

#ਜੇ SQLITE_WIN32_USE_UUID
  { "UuidCreate",               (SYSCALL)UuidCreate,             0 },
#ਨਹੀਂਤਾਂ
  { "UuidCreate",               (SYSCALL)0,                      0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osUuidCreate ((RPC_ਸਥਿਤੀ(RPC_ਇੰਦਰਾਜ*)(UUID*))aSyscall[48].pCurrent)

#ਜੇ SQLITE_WIN32_USE_UUID
  { "UuidCreateSequential",     (SYSCALL)UuidCreateSequential,   0 },
#ਨਹੀਂਤਾਂ
  { "UuidCreateSequential",     (SYSCALL)0,                      0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osUuidCreateSequential \
        ((RPC_ਸਥਿਤੀ(RPC_ਇੰਦਰਾਜ*)(UUID*))aSyscall[49].pCurrent)

#ਜੇ !defined(SQLITE_NO_SYNC) && SQLITE_MAX_MMAP_ਮਾਪ>0
  { "FlushViewOfFile",          (SYSCALL)Flush_View_ਦਾ_File,        0 },
#ਨਹੀਂਤਾਂ
  { "FlushViewOfFile",          (SYSCALL)0,                      0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Flush_View_ਦਾ_File \
        ((BOOL(WINAPI*)(LPCVOID,ਮਾਪ_T))aSyscall[50].pCurrent)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
  { "CreateEvent",              (SYSCALL)CreateEvent,            0 },
#ਨਹੀਂਤਾਂ
  { "CreateEvent",              (SYSCALL)0,                      0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osCreateEvent ((HANDLE(WINAPI*)(LPSECURITY_ਗੁਣ,BOOL, \
                        BOOL,LPCSTR))aSyscall[51].pCurrent)

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
  { "CancelIo",                 (SYSCALL)CancelIo,               0 },
#ਨਹੀਂਤਾਂ
  { "CancelIo",                 (SYSCALL)0,                      0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osCancelIo ((BOOL(WINAPI*)(HANDLE))aSyscall[52].pCurrent)

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _WIN32
  { "getenv",                   (SYSCALL)getenv,                 0 },
#ਨਹੀਂਤਾਂ
  { "getenv",                   (SYSCALL)0,                      0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osGetenv ((ਅਚਲ ਅੱਖਰ *(*)(ਅਚਲ ਅੱਖਰ *))aSyscall[53].pCurrent)

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _WIN32
  { "getcwd",                   (SYSCALL)getcwd,                 0 },
#ਨਹੀਂਤਾਂ
  { "getcwd",                   (SYSCALL)0,                      0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osGetcwd ((ਅੱਖਰ*(*)(ਅੱਖਰ*,size_t))aSyscall[54].pCurrent)

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _WIN32
  { "readlink",                 (SYSCALL)readlink,               0 },
#ਨਹੀਂਤਾਂ
  { "readlink",                 (SYSCALL)0,                      0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osReadlink ((ssize_t(*)(ਅਚਲ ਅੱਖਰ*,ਅੱਖਰ*,size_t))aSyscall[55].pCurrent)

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _WIN32
  { "lstat",                    (SYSCALL)lstat,                  0 },
#ਨਹੀਂਤਾਂ
  { "lstat",                    (SYSCALL)0,                      0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osLstat ((ਪੂਰਨ(*)(ਅਚਲ ਅੱਖਰ*,ਬਣਤਰ stat*))aSyscall[56].pCurrent)

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _WIN32
  { "__errno",                  (SYSCALL)__errno,                0 },
#ਨਹੀਂਤਾਂ
  { "__errno",                  (SYSCALL)0,                      0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osErrno (*((ਪੂਰਨ*(*)(ਖਾਲੀ))aSyscall[57].pCurrent)())

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _WIN32
  { "cygwin_conv_path",         (SYSCALL)cygwin_conv_ਰਸਤਾ,       0 },
#ਨਹੀਂਤਾਂ
  { "cygwin_conv_path",         (SYSCALL)0,                      0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ os_Cygwin_conv_ਰਸਤਾ ((size_t(*)(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ, \
    ਅਚਲ ਖਾਲੀ *, ਖਾਲੀ *, size_t))aSyscall[58].pCurrent)

  { "GetDriveTypeW",            (SYSCALL)GetDriveTypeW,          0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osGetDriveTypeW ((UINT(WINAPI*)(LPCWSTR))aSyscall[59].pCurrent)

}; /* End of the overrideable system calls */

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "win32" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
ਸਥਿਰ ਪੂਰਨ win_ਸੈੱਟ_System_Call(
  sqlite3_vfs *p_Not_ਵਰਤਿਆ,        /* The VFS pointer.  Not used */
  ਅਚਲ ਅੱਖਰ *zName,            /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਪੂਰਨ rc = SQLITE_NOTFOUND;

  ਅਣਵਰਤਿਆ_PARAMETER(p_Not_ਵਰਤਿਆ);
  ਜੇ( zName==0 ){
    /* If no zName is given, restore all system calls to their default
    ** settings and return NULL
    */
    rc = SQLITE_OK;
    ਲਈ(i=0; i<ਆਕਾਰ(aSyscall)/ਆਕਾਰ(aSyscall[0]); i++){
      ਜੇ( aSyscall[i].pDefault ){
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  }ਨਹੀਂਤਾਂ{
    /* If zName is specified, operate on only the one system call
    ** specified.
    */
    ਲਈ(i=0; i<ਆਕਾਰ(aSyscall)/ਆਕਾਰ(aSyscall[0]); i++){
      ਜੇ( strcmp(zName, aSyscall[i].zName)==0 ){
        ਜੇ( aSyscall[i].pDefault==0 ){
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = SQLITE_OK;
        ਜੇ( pNewFunc==0 ) pNewFunc = aSyscall[i].pDefault;
        aSyscall[i].pCurrent = pNewFunc;
        ਰੋਕੋ;
      }
    }
  }
  ਵਾਪਸ rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
ਸਥਿਰ sqlite3_syscall_ptr winGetSystemCall(
  sqlite3_vfs *p_Not_ਵਰਤਿਆ,
  ਅਚਲ ਅੱਖਰ *zName
){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

  ਅਣਵਰਤਿਆ_PARAMETER(p_Not_ਵਰਤਿਆ);
  ਲਈ(i=0; i<ਆਕਾਰ(aSyscall)/ਆਕਾਰ(aSyscall[0]); i++){
    ਜੇ( strcmp(zName, aSyscall[i].zName)==0 ) ਵਾਪਸ aSyscall[i].pCurrent;
  }
  ਵਾਪਸ 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *win_ਅਗਲਾ_System_Call(sqlite3_vfs *p, ਅਚਲ ਅੱਖਰ *zName){
  ਪੂਰਨ i = -1;

  ਅਣਵਰਤਿਆ_PARAMETER(p);
  ਜੇ( zName ){
    ਲਈ(i=0; i<Array_ਮਾਪ(aSyscall)-1; i++){
      ਜੇ( strcmp(zName, aSyscall[i].zName)==0 ) ਰੋਕੋ;
    }
  }
  ਲਈ(i++; i<Array_ਮਾਪ(aSyscall); i++){
    ਜੇ( aSyscall[i].pCurrent!=0 ) ਵਾਪਸ aSyscall[i].zName;
  }
  ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_MALLOC
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
# error SQLITE_WIN32_MALLOC is incompatible with SQLITE_UWP
#ਜੇ_ਅੰਤ
/*
** If a Win32 native heap has been configured, this function will attempt to
** compact it.  Upon success, SQLITE_OK will be returned.  Upon failure, one
** of SQLITE_NOMEM, SQLITE_ERROR, or SQLITE_NOTFOUND will be returned.  The
** "pnLargest" argument, if non-zero, will be used to return the size of the
** largest committed free block in the heap, in bytes.
*/
ਪੂਰਨ sqlite3_win32_compact_ਢੇਰ(LPUINT pn_ਸਭ_ਤੋਂ_ਵੱਡਾ){
  ਪੂਰਨ rc = SQLITE_OK;
  UINT n_ਸਭ_ਤੋਂ_ਵੱਡਾ = 0;
  HANDLE h_ਢੇਰ;

  winMemAssertMagic();
  h_ਢੇਰ = win_Mem_Get_ਢੇਰ();
  ਪ੍ਰਤਿਜ੍ਞਾ( h_ਢੇਰ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( h_ਢੇਰ!=INVALID_HANDLE_ਮੁੱਲ );
#ਜੇ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ਪ੍ਰਤਿਜ੍ਞਾ( os_ਢੇਰ_Validate(h_ਢੇਰ, SQLITE_WIN32_ਢੇਰ_FLAGS, ਨਿਰਾ) );
#ਜੇ_ਅੰਤ
  ਜੇ( (n_ਸਭ_ਤੋਂ_ਵੱਡਾ=os_ਢੇਰ_Compact(h_ਢੇਰ, SQLITE_WIN32_ਢੇਰ_FLAGS))==0 ){
    DWORD ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
    ਜੇ( ਆਖਰੀ_Errno==NO_ERROR ){
      sqlite3_log(SQLITE_NOMEM, "failed to HeapCompact (no space), heap=%p",
                  (ਖਾਲੀ*)h_ਢੇਰ);
      rc = SQLITE_NOMEM_BKPT;
    }ਨਹੀਂਤਾਂ{
      sqlite3_log(SQLITE_ERROR, "failed to HeapCompact (%lu), heap=%p",
                  os_Get_ਆਖਰੀ_Error(), (ਖਾਲੀ*)h_ਢੇਰ);
      rc = SQLITE_ERROR;
    }
  }
  ਜੇ( pn_ਸਭ_ਤੋਂ_ਵੱਡਾ ) *pn_ਸਭ_ਤੋਂ_ਵੱਡਾ = n_ਸਭ_ਤੋਂ_ਵੱਡਾ;
  ਵਾਪਸ rc;
}

/*
** If a Win32 native heap has been configured, this function will attempt to
** destroy and recreate it.  If the Win32 native heap is not isolated and/or
** the sqlite3_memory_used() function does not return zero, SQLITE_BUSY will
** be returned and no changes will be made to the Win32 native heap.
*/
ਪੂਰਨ sqlite3_win32_reset_ਢੇਰ(){
  ਪੂਰਨ rc;
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; ) /* The main static mutex */
  MUTEX_LOGIC( sqlite3_mutex *pMem; )    /* The memsys static mutex */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  MUTEX_LOGIC( pMem = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3_mutex_enter(pMem);
  winMemAssertMagic();
  ਜੇ( win_Mem_Get_ਢੇਰ()!=ਨਿਰਾ && winMemGetOwned() && sqlite3_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ()==0 ){
    /*
    ** At this point, there should be no outstanding memory allocations on
    ** the heap.  Also, since both the main and memsys locks are currently
    ** being held by us, no other function (i.e. from another thread) should
    ** be able to even access the heap.  Attempt to destroy and recreate our
    ** isolated Win32 native heap now.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( win_Mem_Get_ਢੇਰ()!=ਨਿਰਾ );
    ਪ੍ਰਤਿਜ੍ਞਾ( winMemGetOwned() );
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ()==0 );
    winMemShutdown(win_Mem_Get_ਅੰਕੜੇ_Ptr());
    ਪ੍ਰਤਿਜ੍ਞਾ( win_Mem_Get_ਢੇਰ()==ਨਿਰਾ );
    ਪ੍ਰਤਿਜ੍ਞਾ( !winMemGetOwned() );
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ()==0 );
    rc = winMemInit(win_Mem_Get_ਅੰਕੜੇ_Ptr());
    ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || win_Mem_Get_ਢੇਰ()!=ਨਿਰਾ );
    ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || winMemGetOwned() );
    ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || sqlite3_ਸਮ੍ਰਿਤੀ_ਵਰਤਿਆ()==0 );
  }ਨਹੀਂਤਾਂ{
    /*
    ** The Win32 native heap cannot be modified because it may be in use.
    */
    rc = SQLITE_BUSY;
  }
  sqlite3_mutex_leave(pMem);
  sqlite3_mutex_leave(pMainMtx);
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_WIN32_MALLOC */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
/*
** This function outputs the specified (ANSI) string to the Win32 debugger
** (if available).  Undocumented.  Might go away at any moment.
*/
ਖਾਲੀ sqlite3_win32_write_debug(ਅਚਲ ਅੱਖਰ *zBuf, ਪੂਰਨ nBuf){
  ਅੱਖਰ zDbgBuf[SQLITE_WIN32_DBG_BUF_ਮਾਪ];
  ਪੂਰਨ nMin = MIN(nBuf, (SQLITE_WIN32_DBG_BUF_ਮਾਪ - 1)); /* may be negative. */
  ਜੇ( nMin<-1 ) nMin = -1; /* all negative values become -1. */
  ਪ੍ਰਤਿਜ੍ਞਾ( nMin==-1 || nMin==0 || nMin<SQLITE_WIN32_DBG_BUF_ਮਾਪ );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !zBuf ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
  ਜੇ( nMin>0 ){
    memset(zDbgBuf, 0, SQLITE_WIN32_DBG_BUF_ਮਾਪ);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(zDbgBuf, zBuf, nMin);
    osOutputDebugStringA(zDbgBuf);
  }ਨਹੀਂਤਾਂ{
    osOutputDebugStringA(zBuf);
  }
}

ਖਾਲੀ sqlite3_win32_sleep(DWORD milliseconds){
  osSleep(milliseconds);
}

#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0 && SQLITE_THREADSAFE>0
DWORD sqlite3Win32Wait(HANDLE hObject){
  DWORD rc;
  ਜਦੋਂ( (rc = osWaitForSingleObjectEx(hObject, INFINITE,
                                       TRUE))==WAIT_IO_COMPLETION ){}
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* _WIN32 */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_MALLOC
/*
** Allocate nBytes of memory.
*/
ਸਥਿਰ ਖਾਲੀ *winMemMalloc(ਪੂਰਨ n_ਬਾਈਟਾਂ){
  HANDLE h_ਢੇਰ;
  ਖਾਲੀ *p;

  winMemAssertMagic();
  h_ਢੇਰ = win_Mem_Get_ਢੇਰ();
  ਪ੍ਰਤਿਜ੍ਞਾ( h_ਢੇਰ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( h_ਢੇਰ!=INVALID_HANDLE_ਮੁੱਲ );
#ਜੇ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ਪ੍ਰਤਿਜ੍ਞਾ( os_ਢੇਰ_Validate(h_ਢੇਰ, SQLITE_WIN32_ਢੇਰ_FLAGS, ਨਿਰਾ) );
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਬਾਈਟਾਂ>=0 );
  p = os_ਢੇਰ_Alloc(h_ਢੇਰ, SQLITE_WIN32_ਢੇਰ_FLAGS, (ਮਾਪ_T)n_ਬਾਈਟਾਂ);
  ਜੇ( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapAlloc %u bytes (%lu), heap=%p",
                n_ਬਾਈਟਾਂ, os_Get_ਆਖਰੀ_Error(), (ਖਾਲੀ*)h_ਢੇਰ);
  }
  ਵਾਪਸ p;
}

/*
** Free memory.
*/
ਸਥਿਰ ਖਾਲੀ winMemFree(ਖਾਲੀ *p_ਪੂਰਵ){
  HANDLE h_ਢੇਰ;

  winMemAssertMagic();
  h_ਢੇਰ = win_Mem_Get_ਢੇਰ();
  ਪ੍ਰਤਿਜ੍ਞਾ( h_ਢੇਰ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( h_ਢੇਰ!=INVALID_HANDLE_ਮੁੱਲ );
#ਜੇ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ਪ੍ਰਤਿਜ੍ਞਾ( os_ਢੇਰ_Validate(h_ਢੇਰ, SQLITE_WIN32_ਢੇਰ_FLAGS, p_ਪੂਰਵ) );
#ਜੇ_ਅੰਤ
  ਜੇ( !p_ਪੂਰਵ ) ਵਾਪਸ; /* Passing NULL to HeapFree is undefined. */
  ਜੇ( !os_ਢੇਰ_Free(h_ਢੇਰ, SQLITE_WIN32_ਢੇਰ_FLAGS, p_ਪੂਰਵ) ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapFree block %p (%lu), heap=%p",
                p_ਪੂਰਵ, os_Get_ਆਖਰੀ_Error(), (ਖਾਲੀ*)h_ਢੇਰ);
  }
}

/*
** Change the size of an existing memory allocation
*/
ਸਥਿਰ ਖਾਲੀ *winMemRealloc(ਖਾਲੀ *p_ਪੂਰਵ, ਪੂਰਨ n_ਬਾਈਟਾਂ){
  HANDLE h_ਢੇਰ;
  ਖਾਲੀ *p;

  winMemAssertMagic();
  h_ਢੇਰ = win_Mem_Get_ਢੇਰ();
  ਪ੍ਰਤਿਜ੍ਞਾ( h_ਢੇਰ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( h_ਢੇਰ!=INVALID_HANDLE_ਮੁੱਲ );
#ਜੇ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ਪ੍ਰਤਿਜ੍ਞਾ( os_ਢੇਰ_Validate(h_ਢੇਰ, SQLITE_WIN32_ਢੇਰ_FLAGS, p_ਪੂਰਵ) );
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਬਾਈਟਾਂ>=0 );
  ਜੇ( !p_ਪੂਰਵ ){
    p = os_ਢੇਰ_Alloc(h_ਢੇਰ, SQLITE_WIN32_ਢੇਰ_FLAGS, (ਮਾਪ_T)n_ਬਾਈਟਾਂ);
  }ਨਹੀਂਤਾਂ{
    p = os_ਢੇਰ_Re_Alloc(h_ਢੇਰ, SQLITE_WIN32_ਢੇਰ_FLAGS, p_ਪੂਰਵ, (ਮਾਪ_T)n_ਬਾਈਟਾਂ);
  }
  ਜੇ( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to %s %u bytes (%lu), heap=%p",
                p_ਪੂਰਵ ? "HeapReAlloc" : "HeapAlloc", n_ਬਾਈਟਾਂ, os_Get_ਆਖਰੀ_Error(),
                (ਖਾਲੀ*)h_ਢੇਰ);
  }
  ਵਾਪਸ p;
}

/*
** Return the size of an outstanding allocation, in bytes.
*/
ਸਥਿਰ ਪੂਰਨ win_Mem_ਮਾਪ(ਖਾਲੀ *p){
  HANDLE h_ਢੇਰ;
  ਮਾਪ_T n;

  winMemAssertMagic();
  h_ਢੇਰ = win_Mem_Get_ਢੇਰ();
  ਪ੍ਰਤਿਜ੍ਞਾ( h_ਢੇਰ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( h_ਢੇਰ!=INVALID_HANDLE_ਮੁੱਲ );
#ਜੇ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ਪ੍ਰਤਿਜ੍ਞਾ( os_ਢੇਰ_Validate(h_ਢੇਰ, SQLITE_WIN32_ਢੇਰ_FLAGS, p) );
#ਜੇ_ਅੰਤ
  ਜੇ( !p ) ਵਾਪਸ 0;
  n = os_ਢੇਰ_ਮਾਪ(h_ਢੇਰ, SQLITE_WIN32_ਢੇਰ_FLAGS, p);
  ਜੇ( n==(ਮਾਪ_T)-1 ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapSize block %p (%lu), heap=%p",
                p, os_Get_ਆਖਰੀ_Error(), (ਖਾਲੀ*)h_ਢੇਰ);
    ਵਾਪਸ 0;
  }
  ਵਾਪਸ (ਪੂਰਨ)n;
}

/*
** Round up a request size to the next valid allocation size.
*/
ਸਥਿਰ ਪੂਰਨ winMemRoundup(ਪੂਰਨ n){
  ਵਾਪਸ n;
}

/*
** Initialize this module.
*/
ਸਥਿਰ ਪੂਰਨ winMemInit(ਖਾਲੀ *p_App_ਅੰਕੜੇ){
  win_Mem_ਅੰਕੜੇ *p_Win_Mem_ਅੰਕੜੇ = (win_Mem_ਅੰਕੜੇ *)p_App_ਅੰਕੜੇ;

  ਜੇ( !p_Win_Mem_ਅੰਕੜੇ ) ਵਾਪਸ SQLITE_ERROR;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Win_Mem_ਅੰਕੜੇ->magic1==WINMEM_MAGIC1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Win_Mem_ਅੰਕੜੇ->magic2==WINMEM_MAGIC2 );

#ਜੇ SQLITE_WIN32_ਢੇਰ_CREATE
  ਜੇ( !p_Win_Mem_ਅੰਕੜੇ->h_ਢੇਰ ){
    DWORD dw_Initial_ਮਾਪ = SQLITE_WIN32_ਢੇਰ_INIT_ਮਾਪ;
    DWORD dw_ਵੱਧ_ਤੋਂ_ਵੱਧ_ਮਾਪ = (DWORD)sqlite3GlobalConfig.n_ਢੇਰ;
    ਜੇ( dw_ਵੱਧ_ਤੋਂ_ਵੱਧ_ਮਾਪ==0 ){
      dw_ਵੱਧ_ਤੋਂ_ਵੱਧ_ਮਾਪ = SQLITE_WIN32_ਢੇਰ_MAX_ਮਾਪ;
    }ਨਹੀਂਤਾਂ ਜੇ( dw_Initial_ਮਾਪ>dw_ਵੱਧ_ਤੋਂ_ਵੱਧ_ਮਾਪ ){
      dw_Initial_ਮਾਪ = dw_ਵੱਧ_ਤੋਂ_ਵੱਧ_ਮਾਪ;
    }
    p_Win_Mem_ਅੰਕੜੇ->h_ਢੇਰ = os_ਢੇਰ_Create(SQLITE_WIN32_ਢੇਰ_FLAGS,
                                      dw_Initial_ਮਾਪ, dw_ਵੱਧ_ਤੋਂ_ਵੱਧ_ਮਾਪ);
    ਜੇ( !p_Win_Mem_ਅੰਕੜੇ->h_ਢੇਰ ){
      sqlite3_log(SQLITE_NOMEM,
          "failed to HeapCreate (%lu), flags=%u, initSize=%lu, maxSize=%lu",
          os_Get_ਆਖਰੀ_Error(), SQLITE_WIN32_ਢੇਰ_FLAGS, dw_Initial_ਮਾਪ,
          dw_ਵੱਧ_ਤੋਂ_ਵੱਧ_ਮਾਪ);
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
    p_Win_Mem_ਅੰਕੜੇ->bOwned = TRUE;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_Win_Mem_ਅੰਕੜੇ->bOwned );
  }
#ਨਹੀਂਤਾਂ
  p_Win_Mem_ਅੰਕੜੇ->h_ਢੇਰ = os_Get_Process_ਢੇਰ();
  ਜੇ( !p_Win_Mem_ਅੰਕੜੇ->h_ਢੇਰ ){
    sqlite3_log(SQLITE_NOMEM,
        "failed to GetProcessHeap (%lu)", os_Get_ਆਖਰੀ_Error());
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  p_Win_Mem_ਅੰਕੜੇ->bOwned = FALSE;
  ਪ੍ਰਤਿਜ੍ਞਾ( !p_Win_Mem_ਅੰਕੜੇ->bOwned );
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Win_Mem_ਅੰਕੜੇ->h_ਢੇਰ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Win_Mem_ਅੰਕੜੇ->h_ਢੇਰ!=INVALID_HANDLE_ਮੁੱਲ );
#ਜੇ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ਪ੍ਰਤਿਜ੍ਞਾ( os_ਢੇਰ_Validate(p_Win_Mem_ਅੰਕੜੇ->h_ਢੇਰ, SQLITE_WIN32_ਢੇਰ_FLAGS, ਨਿਰਾ) );
#ਜੇ_ਅੰਤ
  ਵਾਪਸ SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ਸਥਿਰ ਖਾਲੀ winMemShutdown(ਖਾਲੀ *p_App_ਅੰਕੜੇ){
  win_Mem_ਅੰਕੜੇ *p_Win_Mem_ਅੰਕੜੇ = (win_Mem_ਅੰਕੜੇ *)p_App_ਅੰਕੜੇ;

  ਜੇ( !p_Win_Mem_ਅੰਕੜੇ ) ਵਾਪਸ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Win_Mem_ਅੰਕੜੇ->magic1==WINMEM_MAGIC1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Win_Mem_ਅੰਕੜੇ->magic2==WINMEM_MAGIC2 );

  ਜੇ( p_Win_Mem_ਅੰਕੜੇ->h_ਢੇਰ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_Win_Mem_ਅੰਕੜੇ->h_ਢੇਰ!=INVALID_HANDLE_ਮੁੱਲ );
#ਜੇ defined(SQLITE_WIN32_MALLOC_VALIDATE)
    ਪ੍ਰਤਿਜ੍ਞਾ( os_ਢੇਰ_Validate(p_Win_Mem_ਅੰਕੜੇ->h_ਢੇਰ, SQLITE_WIN32_ਢੇਰ_FLAGS, ਨਿਰਾ) );
#ਜੇ_ਅੰਤ
    ਜੇ( p_Win_Mem_ਅੰਕੜੇ->bOwned ){
      ਜੇ( !os_ਢੇਰ_Destroy(p_Win_Mem_ਅੰਕੜੇ->h_ਢੇਰ) ){
        sqlite3_log(SQLITE_NOMEM, "failed to HeapDestroy (%lu), heap=%p",
                    os_Get_ਆਖਰੀ_Error(), (ਖਾਲੀ*)p_Win_Mem_ਅੰਕੜੇ->h_ਢੇਰ);
      }
      p_Win_Mem_ਅੰਕੜੇ->bOwned = FALSE;
    }
    p_Win_Mem_ਅੰਕੜੇ->h_ਢੇਰ = ਨਿਰਾ;
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
ਅਚਲ sqlite3_mem_methods *sqlite3MemGetWin32(ਖਾਲੀ){
  ਸਥਿਰ ਅਚਲ sqlite3_mem_methods winMemMethods = {
    winMemMalloc,
    winMemFree,
    winMemRealloc,
    win_Mem_ਮਾਪ,
    winMemRoundup,
    winMemInit,
    winMemShutdown,
    &win_mem_ਅੰਕੜੇ
  };
  ਵਾਪਸ &winMemMethods;
}

ਖਾਲੀ sqlite3_Mem_ਸੈੱਟ_Default(ਖਾਲੀ){
  sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetWin32());
}
#ਜੇ_ਅੰਤ /* SQLITE_WIN32_MALLOC */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
/*
** Convert a UTF-8 string to Microsoft Unicode.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ਸਥਿਰ LPWSTR win_Utf8_ਮੰਜ਼ਿਲ_Unicode(ਅਚਲ ਅੱਖਰ *z_ਪਾਠ){
  ਪੂਰਨ nChar;
  LPWSTR z_Wide_ਪਾਠ;

  nChar = os_Multi_Byte_ਮੰਜ਼ਿਲ_Wide_Char(CP_UTF8, 0, z_ਪਾਠ, -1, ਨਿਰਾ, 0);
  ਜੇ( nChar==0 ){
    ਵਾਪਸ 0;
  }
  z_Wide_ਪਾਠ = sqlite3MallocZero( nChar*ਆਕਾਰ(WCHAR) );
  ਜੇ( z_Wide_ਪਾਠ==0 ){
    ਵਾਪਸ 0;
  }
  nChar = os_Multi_Byte_ਮੰਜ਼ਿਲ_Wide_Char(CP_UTF8, 0, z_ਪਾਠ, -1, z_Wide_ਪਾਠ,
                                nChar);
  ਜੇ( nChar==0 ){
    sqlite3_free(z_Wide_ਪਾਠ);
    z_Wide_ਪਾਠ = 0;
  }
  ਵਾਪਸ z_Wide_ਪਾਠ;
}
#ਜੇ_ਅੰਤ /* _WIN32 */

/*
** Convert a Microsoft Unicode string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ਸਥਿਰ ਅੱਖਰ *win_Unicode_ਮੰਜ਼ਿਲ_Utf8(LPCWSTR z_Wide_ਪਾਠ){
  ਪੂਰਨ nByte;
  ਅੱਖਰ *z_ਪਾਠ;

  nByte = os_Wide_Char_ਮੰਜ਼ਿਲ_Multi_Byte(CP_UTF8, 0, z_Wide_ਪਾਠ, -1, 0, 0, 0, 0);
  ਜੇ( nByte == 0 ){
    ਵਾਪਸ 0;
  }
  z_ਪਾਠ = sqlite3MallocZero( nByte );
  ਜੇ( z_ਪਾਠ==0 ){
    ਵਾਪਸ 0;
  }
  nByte = os_Wide_Char_ਮੰਜ਼ਿਲ_Multi_Byte(CP_UTF8, 0, z_Wide_ਪਾਠ, -1, z_ਪਾਠ, nByte,
                                0, 0);
  ਜੇ( nByte == 0 ){
    sqlite3_free(z_ਪਾਠ);
    z_ਪਾਠ = 0;
  }
  ਵਾਪਸ z_ਪਾਠ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
/*
** Convert an ANSI string to Microsoft Unicode, using the ANSI or OEM
** code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ਸਥਿਰ LPWSTR win_Mbcs_ਮੰਜ਼ਿਲ_Unicode(ਅਚਲ ਅੱਖਰ *z_ਪਾਠ, ਪੂਰਨ useAnsi){
  ਪੂਰਨ nWideChar;
  LPWSTR z_Mbcs_ਪਾਠ;
  ਪੂਰਨ codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nWideChar = os_Multi_Byte_ਮੰਜ਼ਿਲ_Wide_Char(codepage, 0, z_ਪਾਠ, -1, ਨਿਰਾ,
                                0);
  ਜੇ( nWideChar==0 ){
    ਵਾਪਸ 0;
  }
  z_Mbcs_ਪਾਠ = sqlite3MallocZero( nWideChar*ਆਕਾਰ(WCHAR) );
  ਜੇ( z_Mbcs_ਪਾਠ==0 ){
    ਵਾਪਸ 0;
  }
  nWideChar = os_Multi_Byte_ਮੰਜ਼ਿਲ_Wide_Char(codepage, 0, z_ਪਾਠ, -1, z_Mbcs_ਪਾਠ,
                                nWideChar);
  ਜੇ( nWideChar==0 ){
    sqlite3_free(z_Mbcs_ਪਾਠ);
    z_Mbcs_ਪਾਠ = 0;
  }
  ਵਾਪਸ z_Mbcs_ਪਾਠ;
}

/*
** Convert a Microsoft Unicode string to a multi-byte character string,
** using the ANSI or OEM code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ਸਥਿਰ ਅੱਖਰ *win_Unicode_ਮੰਜ਼ਿਲ_Mbcs(LPCWSTR z_Wide_ਪਾਠ, ਪੂਰਨ useAnsi){
  ਪੂਰਨ nByte;
  ਅੱਖਰ *z_ਪਾਠ;
  ਪੂਰਨ codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nByte = os_Wide_Char_ਮੰਜ਼ਿਲ_Multi_Byte(codepage, 0, z_Wide_ਪਾਠ, -1, 0, 0, 0, 0);
  ਜੇ( nByte == 0 ){
    ਵਾਪਸ 0;
  }
  z_ਪਾਠ = sqlite3MallocZero( nByte );
  ਜੇ( z_ਪਾਠ==0 ){
    ਵਾਪਸ 0;
  }
  nByte = os_Wide_Char_ਮੰਜ਼ਿਲ_Multi_Byte(codepage, 0, z_Wide_ਪਾਠ, -1, z_ਪਾਠ,
                                nByte, 0, 0);
  ਜੇ( nByte == 0 ){
    sqlite3_free(z_ਪਾਠ);
    z_ਪਾਠ = 0;
  }
  ਵਾਪਸ z_ਪਾਠ;
}

/*
** Convert a multi-byte character string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ਸਥਿਰ ਅੱਖਰ *win_Mbcs_ਮੰਜ਼ਿਲ_Utf8(ਅਚਲ ਅੱਖਰ *z_ਪਾਠ, ਪੂਰਨ useAnsi){
  ਅੱਖਰ *z_ਪਾਠ_Utf8;
  LPWSTR zTmpWide;

  zTmpWide = win_Mbcs_ਮੰਜ਼ਿਲ_Unicode(z_ਪਾਠ, useAnsi);
  ਜੇ( zTmpWide==0 ){
    ਵਾਪਸ 0;
  }
  z_ਪਾਠ_Utf8 = win_Unicode_ਮੰਜ਼ਿਲ_Utf8(zTmpWide);
  sqlite3_free(zTmpWide);
  ਵਾਪਸ z_ਪਾਠ_Utf8;
}

/*
** Convert a UTF-8 string to a multi-byte character string.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ਸਥਿਰ ਅੱਖਰ *win_Utf8_ਮੰਜ਼ਿਲ_Mbcs(ਅਚਲ ਅੱਖਰ *z_ਪਾਠ, ਪੂਰਨ useAnsi){
  ਅੱਖਰ *z_ਪਾਠ_Mbcs;
  LPWSTR zTmpWide;

  zTmpWide = win_Utf8_ਮੰਜ਼ਿਲ_Unicode(z_ਪਾਠ);
  ਜੇ( zTmpWide==0 ){
    ਵਾਪਸ 0;
  }
  z_ਪਾਠ_Mbcs = win_Unicode_ਮੰਜ਼ਿਲ_Mbcs(zTmpWide, useAnsi);
  sqlite3_free(zTmpWide);
  ਵਾਪਸ z_ਪਾਠ_Mbcs;
}

/*
** This is a public wrapper for the winUtf8ToUnicode() function.
*/
LPWSTR sqlite3_win32_utf8_ਮੰਜ਼ਿਲ_unicode(ਅਚਲ ਅੱਖਰ *z_ਪਾਠ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !z_ਪਾਠ ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize() ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ win_Utf8_ਮੰਜ਼ਿਲ_Unicode(z_ਪਾਠ);
}

/*
** This is a public wrapper for the winUnicodeToUtf8() function.
*/
ਅੱਖਰ *sqlite3_win32_unicode_ਮੰਜ਼ਿਲ_utf8(LPCWSTR z_Wide_ਪਾਠ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !z_Wide_ਪਾਠ ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize() ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ win_Unicode_ਮੰਜ਼ਿਲ_Utf8(z_Wide_ਪਾਠ);
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
ਅੱਖਰ *sqlite3_win32_mbcs_ਮੰਜ਼ਿਲ_utf8(ਅਚਲ ਅੱਖਰ *z_ਪਾਠ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !z_ਪਾਠ ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize() ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ win_Mbcs_ਮੰਜ਼ਿਲ_Utf8(z_ਪਾਠ, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
ਅੱਖਰ *sqlite3_win32_mbcs_ਮੰਜ਼ਿਲ_utf8_v2(ਅਚਲ ਅੱਖਰ *z_ਪਾਠ, ਪੂਰਨ useAnsi){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !z_ਪਾਠ ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize() ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ win_Mbcs_ਮੰਜ਼ਿਲ_Utf8(z_ਪਾਠ, useAnsi);
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
ਅੱਖਰ *sqlite3_win32_utf8_ਮੰਜ਼ਿਲ_mbcs(ਅਚਲ ਅੱਖਰ *z_ਪਾਠ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !z_ਪਾਠ ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize() ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ win_Utf8_ਮੰਜ਼ਿਲ_Mbcs(z_ਪਾਠ, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
ਅੱਖਰ *sqlite3_win32_utf8_ਮੰਜ਼ਿਲ_mbcs_v2(ਅਚਲ ਅੱਖਰ *z_ਪਾਠ, ਪੂਰਨ useAnsi){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !z_ਪਾਠ ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  ਜੇ( sqlite3_initialize() ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ win_Utf8_ਮੰਜ਼ਿਲ_Mbcs(z_ਪਾਠ, useAnsi);
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-8 string.
*/
ਪੂਰਨ sqlite3_win32_ਸੈੱਟ_directory8(
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ type, /* Identifier for directory being set or reset */
  ਅਚਲ ਅੱਖਰ *z_ਮੁੱਲ  /* New value for directory being set or reset */
){
  ਅੱਖਰ **ppDirectory = 0;
  ਪੂਰਨ rc;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ਜੇ( rc ) ਵਾਪਸ rc;
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ਜੇ( type==SQLITE_WIN32_ਅੰਕੜੇ_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_ਅੰਕੜੇ_directory;
  }ਨਹੀਂਤਾਂ ਜੇ( type==SQLITE_WIN32_ਆਰਜ਼ੀ_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_ਆਰਜ਼ੀ_directory;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( !ppDirectory || type==SQLITE_WIN32_ਅੰਕੜੇ_DIRECTORY_TYPE
          || type==SQLITE_WIN32_ਆਰਜ਼ੀ_DIRECTORY_TYPE
  );
  ਪ੍ਰਤਿਜ੍ਞਾ( !ppDirectory || sqlite3MemdebugHasType(*ppDirectory, MEMTYPE_ਢੇਰ) );
  ਜੇ( ppDirectory ){
    ਅੱਖਰ *zCopy = 0;
    ਜੇ( z_ਮੁੱਲ && z_ਮੁੱਲ[0] ){
      zCopy = sqlite3_mprintf("%s", z_ਮੁੱਲ);
      ਜੇ ( zCopy==0 ){
        rc = SQLITE_NOMEM_BKPT;
        ਜਾਓ ਸੈੱਟ_directory8_ਪੂਰਾ;
      }
    }
    sqlite3_free(*ppDirectory);
    *ppDirectory = zCopy;
    rc = SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    rc = SQLITE_ERROR;
  }
ਸੈੱਟ_directory8_ਪੂਰਾ:
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ਵਾਪਸ rc;
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-16 string.
*/
ਪੂਰਨ sqlite3_win32_ਸੈੱਟ_directory16(
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ type, /* Identifier for directory being set or reset */
  ਅਚਲ ਖਾਲੀ *z_ਮੁੱਲ  /* New value for directory being set or reset */
){
  ਪੂਰਨ rc;
  ਅੱਖਰ *zUtf8 = 0;
  ਜੇ( z_ਮੁੱਲ ){
    zUtf8 = sqlite3_win32_unicode_ਮੰਜ਼ਿਲ_utf8(z_ਮੁੱਲ);
    ਜੇ( zUtf8==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_win32_ਸੈੱਟ_directory8(type, zUtf8);
  ਜੇ( zUtf8 ) sqlite3_free(zUtf8);
  ਵਾਪਸ rc;
}

/*
** This function sets the data directory or the temporary directory based on
** the provided arguments.  The type argument must be 1 in order to set the
** data directory or 2 in order to set the temporary directory.  The zValue
** argument is the name of the directory to use.  The return value will be
** SQLITE_OK if successful.
*/
ਪੂਰਨ sqlite3_win32_ਸੈੱਟ_directory(
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ type, /* Identifier for directory being set or reset */
  ਖਾਲੀ *z_ਮੁੱਲ        /* New value for directory being set or reset */
){
  ਵਾਪਸ sqlite3_win32_ਸੈੱਟ_directory16(type, z_ਮੁੱਲ);
}
#ਜੇ_ਅੰਤ /* _WIN32 */

/*
** The return value of winGetLastErrorMsg
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated).
*/
ਸਥਿਰ ਪੂਰਨ win_Get_ਆਖਰੀ_Error_ਸੁਨੇਹਾ(DWORD ਆਖਰੀ_Errno, ਪੂਰਨ nBuf, ਅੱਖਰ *zBuf){
  /* FormatMessage returns 0 on failure.  Otherwise it
  ** returns the number of TCHARs written to the output
  ** buffer, excluding the terminating null char.
  */
  DWORD dwLen = 0;
  ਅੱਖਰ *z_ਬਾਹਰਲਾ = 0;
  LPWSTR z_ਆਰਜ਼ੀ_Wide = ਨਿਰਾ;
  dwLen = os_Format_ਸੁਨੇਹਾ_W(FORMAT_ਸੁਨੇਹਾ_ALLOCATE_ਬਫਰ |
                           FORMAT_ਸੁਨੇਹਾ_FROM_SYSTEM |
                           FORMAT_ਸੁਨੇਹਾ_IGNORE_INSERTS,
                           ਨਿਰਾ,
                           ਆਖਰੀ_Errno,
                           0,
                           (LPWSTR) &z_ਆਰਜ਼ੀ_Wide,
                           0,
                           0);
  ਜੇ( dwLen > 0 ){
    /* allocate a buffer and convert to UTF8 */
    sqlite3BeginBenignMalloc();
    z_ਬਾਹਰਲਾ = win_Unicode_ਮੰਜ਼ਿਲ_Utf8(z_ਆਰਜ਼ੀ_Wide);
    sqlite3_ਸਿਰਾ_Benign_Malloc();
    /* free the system buffer allocated by FormatMessage */
    osLocalFree(z_ਆਰਜ਼ੀ_Wide);
  }
  ਜੇ( 0 == dwLen ){
    sqlite3_snprintf(nBuf, zBuf, "OsError 0x%lx (%lu)", ਆਖਰੀ_Errno, ਆਖਰੀ_Errno);
  }ਨਹੀਂਤਾਂ{
    /* copy a maximum of nBuf chars to output buffer */
    sqlite3_snprintf(nBuf, zBuf, "%s", z_ਬਾਹਰਲਾ);
    /* free the UTF8 buffer */
    sqlite3_free(z_ਬਾਹਰਲਾ);
  }
  ਵਾਪਸ 0;
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ winLogError(a,b,c,d)   win_Log_Error_At_ਸਤਰ(a,b,c,d,__LINE__)
ਸਥਿਰ ਪੂਰਨ win_Log_Error_At_ਸਤਰ(
  ਪੂਰਨ errcode,                    /* SQLite error code */
  DWORD ਆਖਰੀ_Errno,                /* Win32 last error */
  ਅਚਲ ਅੱਖਰ *zFunc,              /* Name of OS function that failed */
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,              /* File path associated with error */
  ਪੂਰਨ i_ਸਤਰ                       /* Source line number where error occurred */
){
  ਅੱਖਰ z_ਸੁਨੇਹਾ[500];                 /* Human readable error text */
  ਪੂਰਨ i;                          /* Loop counter */

  z_ਸੁਨੇਹਾ[0] = 0;
  win_Get_ਆਖਰੀ_Error_ਸੁਨੇਹਾ(ਆਖਰੀ_Errno, ਆਕਾਰ(z_ਸੁਨੇਹਾ), z_ਸੁਨੇਹਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ( errcode!=SQLITE_OK );
  ਜੇ( z_ਰਸਤਾ==0 ) z_ਰਸਤਾ = "";
  ਲਈ(i=0; z_ਸੁਨੇਹਾ[i] && z_ਸੁਨੇਹਾ[i]!='\r' && z_ਸੁਨੇਹਾ[i]!='\n'; i++){}
  z_ਸੁਨੇਹਾ[i] = 0;
  sqlite3_log(errcode,
      "os_win.c:%d: (%lu) %s(%s) - %s",
      i_ਸਤਰ, ਆਖਰੀ_Errno, zFunc, z_ਰਸਤਾ, z_ਸੁਨੇਹਾ
  );

  ਵਾਪਸ errcode;
}

/*
** The number of times that a ReadFile(), WriteFile(), and DeleteFile()
** will be retried following a locking error - probably caused by
** antivirus software.  Also the initial delay before the first retry.
** The delay increases linearly with each retry.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_IOERR_RETRY
# define SQLITE_WIN32_IOERR_RETRY 10
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_WIN32_IOERR_RETRY_DELAY
# define SQLITE_WIN32_IOERR_RETRY_DELAY 25
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ winIoerrRetry = SQLITE_WIN32_IOERR_RETRY;
ਸਥਿਰ ਪੂਰਨ winIoerrRetryDelay = SQLITE_WIN32_IOERR_RETRY_DELAY;

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
#ਜੇ !defined(winIoerrCanRetry1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ winIoerrCanRetry1(a) (((a)==ERROR_ACCESS_DENIED)        || \
                              ((a)==ERROR_SHARING_VIOLATION)    || \
                              ((a)==ERROR_ਤਾਲਾ_VIOLATION)       || \
                              ((a)==ERROR_DEV_NOT_EXIST)        || \
                              ((a)==ERROR_NETNAME_DELETED)      || \
                              ((a)==ERROR_SEM_TIMEOUT)          || \
                              ((a)==ERROR_ਜਾਲ_UNREACHABLE))
#ਜੇ_ਅੰਤ

/*
** If a ReadFile() or WriteFile() error occurs, invoke this routine
** to see if it should be retried.  Return TRUE to retry.  Return FALSE
** to give up with an error.
*/
ਸਥਿਰ ਪੂਰਨ winRetryIoerr(ਪੂਰਨ *pnRetry, DWORD *pError){
  DWORD e = os_Get_ਆਖਰੀ_Error();
  ਜੇ( *pnRetry>=winIoerrRetry ){
    ਜੇ( pError ){
      *pError = e;
    }
    ਵਾਪਸ 0;
  }
  ਜੇ( winIoerrCanRetry1(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    ਵਾਪਸ 1;
  }
#ਜੇ defined(winIoerrCanRetry2)
  ਨਹੀਂਤਾਂ ਜੇ( winIoerrCanRetry2(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    ਵਾਪਸ 1;
  }
#ਜੇ_ਅੰਤ
  ਜੇ( pError ){
    *pError = e;
  }
  ਵਾਪਸ 0;
}

/*
** Log a I/O error retry episode.
*/
ਸਥਿਰ ਖਾਲੀ winLogIoerr(ਪੂਰਨ nRetry, ਪੂਰਨ lineno){
  ਜੇ( nRetry ){
    sqlite3_log(SQLITE_NOTICE,
      "delayed %dms for lock/sharing conflict at line %d",
      winIoerrRetryDelay*nRetry*(nRetry+1)/2, lineno
    );
  }
}


/*
** Lock a file region.
*/
ਸਥਿਰ BOOL win_ਤਾਲਾ_File(
  LPHANDLE phFile,
  DWORD flags,
  DWORD ਵਿੱਥ_ਨੀਵਾਂ,
  DWORD ਵਿੱਥ_ਉੱਚਾ,
  DWORD ਅੰਕ_ਬਾਈਟਾਂ_ਨੀਵਾਂ,
  DWORD ਅੰਕ_ਬਾਈਟਾਂ_ਉੱਚਾ
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, ਆਕਾਰ(OVERLAPPED));
  ovlp.ਵਿੱਥ_2 = ਵਿੱਥ_ਨੀਵਾਂ;
  ovlp.ਵਿੱਥ_ਉੱਚਾ_2 = ਵਿੱਥ_ਉੱਚਾ;
  ਵਾਪਸ os_ਤਾਲਾ_File_Ex(*phFile, flags, 0, ਅੰਕ_ਬਾਈਟਾਂ_ਨੀਵਾਂ, ਅੰਕ_ਬਾਈਟਾਂ_ਉੱਚਾ, &ovlp);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
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
ਸਥਿਰ ਪੂਰਨ win_Handle_ਤਾਲਾ_Timeout(
  HANDLE hFile,
  DWORD ਵਿੱਥ,
  DWORD nByte,
  ਪੂਰਨ bExcl,
  DWORD nMs
){
  DWORD flags = LOCKFILE_FAIL_IMMEDIATELY | (bExcl?LOCKFILE_EXCLUSIVE_ਤਾਲਾ:0);
  ਪੂਰਨ rc = SQLITE_OK;
  BOOL ret;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, ਆਕਾਰ(OVERLAPPED));
  ovlp.ਵਿੱਥ_2 = ਵਿੱਥ;

#ਜੇ defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  ਜੇ( nMs!=0 ){
    flags &= ~LOCKFILE_FAIL_IMMEDIATELY;
  }
  ovlp.hEvent = osCreateEvent(ਨਿਰਾ, TRUE, FALSE, ਨਿਰਾ);
  ਜੇ( ovlp.hEvent==ਨਿਰਾ ){
    ਵਾਪਸ SQLITE_IOERR_ਤਾਲਾ;
  }
#ਜੇ_ਅੰਤ
  ret = os_ਤਾਲਾ_File_Ex(hFile, flags, 0, nByte, 0, &ovlp);
#ਜੇ defined(SQLITE_ENABLE_SETLK_TIMEOUT)
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
  ਜੇ( !ret && Get_ਆਖਰੀ_Error()==ERROR_IO_PENDING ){
    DWORD nDelay = (nMs==0 ? INFINITE : nMs);
    DWORD res = osWaitForSingleObject(ovlp.hEvent, nDelay);
    ਜੇ( res==WAIT_OBJECT_0 ){
      ret = TRUE;
    }ਨਹੀਂਤਾਂ ਜੇ( res==WAIT_TIMEOUT ){
#ਜੇ SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = SQLITE_BUSY_TIMEOUT;
#ਨਹੀਂਤਾਂ
      rc = SQLITE_BUSY;
#ਜੇ_ਅੰਤ
    }ਨਹੀਂਤਾਂ{
      /* Some other error has occurred */
      rc = SQLITE_IOERR_ਤਾਲਾ;
    }

    /* If it is still pending, cancel the LockFileEx() call. */
    osCancelIo(hFile);
  }

  osCloseHandle(ovlp.hEvent);
#ਜੇ_ਅੰਤ /* defined(SQLITE_ENABLE_SETLK_TIMEOUT) */

  ਜੇ( rc==SQLITE_OK && !ret ){
    rc = SQLITE_BUSY;
  }
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* #ifndef SQLITE_OMIT_WAL */

/*
** Unlock a file region.
 */
ਸਥਿਰ BOOL winUnlockFile(
  LPHANDLE phFile,
  DWORD ਵਿੱਥ_ਨੀਵਾਂ,
  DWORD ਵਿੱਥ_ਉੱਚਾ,
  DWORD ਅੰਕ_ਬਾਈਟਾਂ_ਨੀਵਾਂ,
  DWORD ਅੰਕ_ਬਾਈਟਾਂ_ਉੱਚਾ
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, ਆਕਾਰ(OVERLAPPED));
  ovlp.ਵਿੱਥ_2 = ਵਿੱਥ_ਨੀਵਾਂ;
  ovlp.ਵਿੱਥ_ਉੱਚਾ_2 = ਵਿੱਥ_ਉੱਚਾ;
  ਵਾਪਸ osUnlockFileEx(*phFile, 0, ਅੰਕ_ਬਾਈਟਾਂ_ਨੀਵਾਂ, ਅੰਕ_ਬਾਈਟਾਂ_ਉੱਚਾ, &ovlp);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
/*
** Remove an nByte lock starting at offset iOff from HANDLE h.
*/
ਸਥਿਰ ਪੂਰਨ winHandleUnlock(HANDLE h, ਪੂਰਨ iOff, ਪੂਰਨ nByte){
  BOOL ret = winUnlockFile(&h, iOff, 0, nByte, 0);
  ਵਾਪਸ (ret ? SQLITE_OK : SQLITE_IOERR_UNLOCK);
}
#ਜੇ_ਅੰਤ

/*****************************************************************************
** The next group of routines implement the I/O methods specified
** by the sqlite3_io_methods object.
******************************************************************************/

/*
** Some Microsoft compilers lack this definition.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ INVALID_ਸੈੱਟ_FILE_ਸੂਚਕ
# define INVALID_ਸੈੱਟ_FILE_ਸੂਚਕ ((DWORD)-1)
#ਜੇ_ਅੰਤ

/*
** Seek the file handle h to offset nByte of the file.
**
** If successful, return SQLITE_OK. Or, if an error occurs, return an SQLite
** error code.
*/
ਸਥਿਰ ਪੂਰਨ winHandleSeek(HANDLE h, sqlite3_int64 i_ਵਿੱਥ){
  ਪੂਰਨ rc = SQLITE_OK;             /* Return value */
  LARGE_INTEGER x;                /* The offset */

  x.QuadPart = i_ਵਿੱਥ;
  ਜੇ( os_ਸੈੱਟ_File_ਸੂਚਕ_Ex(h, x, 0, FILE_BEGIN)==0 ){
    rc = SQLITE_IOERR_SEEK;
  }
  OSTRACE(("SEEK file=%p, offset=%lld rc=%s\n", h, i_ਵਿੱਥ,sqlite3ErrName(rc)));
  ਵਾਪਸ rc;
}

/*
** Move the current position of the file handle passed as the first
** argument to offset iOffset within the file. If successful, return 0.
** Otherwise, set pFile->lastErrno and return non-zero.
*/
ਸਥਿਰ ਪੂਰਨ winSeekFile(winFile *pFile, sqlite3_int64 i_ਵਿੱਥ){
  ਪੂਰਨ rc;

  rc = winHandleSeek(pFile->h, i_ਵਿੱਥ);
  ਜੇ( rc!=SQLITE_OK ){
    pFile->ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
    winLogError(rc, pFile->ਆਖਰੀ_Errno, "winSeekFile", pFile->z_ਰਸਤਾ);
  }
  ਵਾਪਸ rc;
}


#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
/* Forward references to VFS helper methods used for memory mapped files */
ਸਥਿਰ ਪੂਰਨ winMapfile(winFile*, sqlite3_int64);
ਸਥਿਰ ਪੂਰਨ winUnmapfile(winFile*);
#ਜੇ_ਅੰਤ

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MX_CLOSE_ATTEMPT 3
ਸਥਿਰ ਪੂਰਨ winClose(sqlite3_file *id){
  ਪੂਰਨ rc, cnt = 0;
  winFile *pFile = (winFile*)id;

  ਪ੍ਰਤਿਜ੍ਞਾ( id!=0 );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->pShm==0 );
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->h!=ਨਿਰਾ && pFile->h!=INVALID_HANDLE_ਮੁੱਲ );
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p\n",
           osGetCurrentProcessId(), pFile, pFile->h));

#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  winUnmapfile(pFile);
#ਜੇ_ਅੰਤ

  ਕਰੋ{
    rc = osCloseHandle(pFile->h);
    /* SimulateIOError( rc=0; cnt=MX_CLOSE_ATTEMPT; ); */
  }ਜਦੋਂ( rc==0 && ++cnt < MX_CLOSE_ATTEMPT && (osSleep(100), 1) );
  ਜੇ( rc ){
    pFile->h = ਨਿਰਾ;
  }
  OpenCounter(-1);
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, rc ? "ok" : "failed"));
  ਵਾਪਸ rc ? SQLITE_OK
            : winLogError(SQLITE_IOERR_CLOSE, os_Get_ਆਖਰੀ_Error(),
                          "winClose", pFile->z_ਰਸਤਾ);
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
ਸਥਿਰ ਪੂਰਨ winRead(
  sqlite3_file *id,          /* File to read from */
  ਖਾਲੀ *pBuf,                /* Write content into this buffer */
  ਪੂਰਨ amt,                   /* Number of bytes to read */
  sqlite3_int64 ਵਿੱਥ       /* Begin reading at this offset */
){
#ਜੇ !defined(SQLITE_WIN32_NO_OVERLAPPED)
  OVERLAPPED overlapped;          /* The offset for ReadFile. */
#ਜੇ_ਅੰਤ
  winFile *pFile = (winFile*)id;  /* file handle */
  DWORD nRead;                    /* Number of bytes actually read from file */
  ਪੂਰਨ nRetry = 0;                 /* Number of retrys */

  ਪ੍ਰਤਿਜ੍ਞਾ( id!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( amt>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿੱਥ>=0 );
  SimulateIOError(ਵਾਪਸ SQLITE_IOERR_READ);
  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, ਵਿੱਥ, pFile->locktype));

#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  ਜੇ( ਵਿੱਥ<pFile->mmap_ਮਾਪ ){
    ਜੇ( ਵਿੱਥ+amt <= pFile->mmap_ਮਾਪ ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(pBuf, &((u8 *)(pFile->pMapRegion))[ਵਿੱਥ], amt);
      OSTRACE(("READ-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      ਵਾਪਸ SQLITE_OK;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ nCopy = (ਪੂਰਨ)(pFile->mmap_ਮਾਪ - ਵਿੱਥ);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(pBuf, &((u8 *)(pFile->pMapRegion))[ਵਿੱਥ], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      ਵਿੱਥ += nCopy;
    }
  }
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_WIN32_NO_OVERLAPPED)
  ਜੇ( winSeekFile(pFile, ਵਿੱਥ) ){
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ਵਾਪਸ SQLITE_ਭਰਿਆ;
  }
  ਜਦੋਂ( !osReadFile(pFile->h, pBuf, amt, &nRead, 0) ){
#ਨਹੀਂਤਾਂ
  memset(&overlapped, 0, ਆਕਾਰ(OVERLAPPED));
  overlapped.ਵਿੱਥ_2 = (LONG)(ਵਿੱਥ & 0xffffffff);
  overlapped.ਵਿੱਥ_ਉੱਚਾ_2 = (LONG)((ਵਿੱਥ>>32) & 0x7fffffff);
  ਜਦੋਂ( !osReadFile(pFile->h, pBuf, amt, &nRead, &overlapped) &&
         os_Get_ਆਖਰੀ_Error()!=ERROR_HANDLE_EOF ){
#ਜੇ_ਅੰਤ
    DWORD ਆਖਰੀ_Errno;
    ਜੇ( winRetryIoerr(&nRetry, &ਆਖਰੀ_Errno) ) ਜਾਰੀ;
    pFile->ਆਖਰੀ_Errno = ਆਖਰੀ_Errno;
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ਵਾਪਸ winLogError(SQLITE_IOERR_READ, pFile->ਆਖਰੀ_Errno,
                       "winRead", pFile->z_ਰਸਤਾ);
  }
  winLogIoerr(nRetry, __LINE__);
  ਜੇ( nRead<(DWORD)amt ){
    /* Unread parts of the buffer must be zero-filled */
    memset(&((ਅੱਖਰ*)pBuf)[nRead], 0, amt-nRead);
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_SHORT_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ਵਾਪਸ SQLITE_IOERR_SHORT_READ;
  }

  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  ਵਾਪਸ SQLITE_OK;
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
ਸਥਿਰ ਪੂਰਨ winWrite(
  sqlite3_file *id,               /* File to write into */
  ਅਚਲ ਖਾਲੀ *pBuf,               /* The bytes to be written */
  ਪੂਰਨ amt,                        /* Number of bytes to write */
  sqlite3_int64 ਵਿੱਥ            /* Offset into the file to begin writing at */
){
  ਪੂਰਨ rc = 0;                     /* True if error has occurred, else false */
  winFile *pFile = (winFile*)id;  /* File handle */
  ਪੂਰਨ nRetry = 0;                 /* Number of retries */

  ਪ੍ਰਤਿਜ੍ਞਾ( amt>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  SimulateIOError(ਵਾਪਸ SQLITE_IOERR_WRITE);
  SimulateDiskfullError(ਵਾਪਸ SQLITE_ਭਰਿਆ);

  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, ਵਿੱਥ, pFile->locktype));

#ਜੇ defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_ਮਾਪ>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  ਜੇ( ਵਿੱਥ<pFile->mmap_ਮਾਪ ){
    ਜੇ( ਵਿੱਥ+amt <= pFile->mmap_ਮਾਪ ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&((u8 *)(pFile->pMapRegion))[ਵਿੱਥ], pBuf, amt);
      OSTRACE(("WRITE-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      ਵਾਪਸ SQLITE_OK;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ nCopy = (ਪੂਰਨ)(pFile->mmap_ਮਾਪ - ਵਿੱਥ);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&((u8 *)(pFile->pMapRegion))[ਵਿੱਥ], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      ਵਿੱਥ += nCopy;
    }
  }
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_WIN32_NO_OVERLAPPED)
  rc = winSeekFile(pFile, ਵਿੱਥ);
  ਜੇ( rc==0 ){
#ਨਹੀਂਤਾਂ
  {
#ਜੇ_ਅੰਤ
#ਜੇ !defined(SQLITE_WIN32_NO_OVERLAPPED)
    OVERLAPPED overlapped;        /* The offset for WriteFile. */
#ਜੇ_ਅੰਤ
    u8 *aRem = (u8 *)pBuf;        /* Data yet to be written */
    ਪੂਰਨ nRem = amt;               /* Number of bytes yet to be written */
    DWORD nWrite;                 /* Bytes written by each WriteFile() call */
    DWORD ਆਖਰੀ_Errno = NO_ERROR;   /* Value returned by GetLastError() */

#ਜੇ !defined(SQLITE_WIN32_NO_OVERLAPPED)
    memset(&overlapped, 0, ਆਕਾਰ(OVERLAPPED));
    overlapped.ਵਿੱਥ_2 = (LONG)(ਵਿੱਥ & 0xffffffff);
    overlapped.ਵਿੱਥ_ਉੱਚਾ_2 = (LONG)((ਵਿੱਥ>>32) & 0x7fffffff);
#ਜੇ_ਅੰਤ

    ਜਦੋਂ( nRem>0 ){
#ਜੇ defined(SQLITE_WIN32_NO_OVERLAPPED)
      ਜੇ( !osWriteFile(pFile->h, aRem, nRem, &nWrite, 0) ){
#ਨਹੀਂਤਾਂ
      ਜੇ( !osWriteFile(pFile->h, aRem, nRem, &nWrite, &overlapped) ){
#ਜੇ_ਅੰਤ
        ਜੇ( winRetryIoerr(&nRetry, &ਆਖਰੀ_Errno) ) ਜਾਰੀ;
        ਰੋਕੋ;
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( nWrite==0 || nWrite<=(DWORD)nRem );
      ਜੇ( nWrite==0 || nWrite>(DWORD)nRem ){
        ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
        ਰੋਕੋ;
      }
#ਜੇ !defined(SQLITE_WIN32_NO_OVERLAPPED)
      ਵਿੱਥ += nWrite;
      overlapped.ਵਿੱਥ_2 = (LONG)(ਵਿੱਥ & 0xffffffff);
      overlapped.ਵਿੱਥ_ਉੱਚਾ_2 = (LONG)((ਵਿੱਥ>>32) & 0x7fffffff);
#ਜੇ_ਅੰਤ
      aRem += nWrite;
      nRem -= nWrite;
    }
    ਜੇ( nRem>0 ){
      pFile->ਆਖਰੀ_Errno = ਆਖਰੀ_Errno;
      rc = 1;
    }
  }

  ਜੇ( rc ){
    ਜੇ(   ( pFile->ਆਖਰੀ_Errno==ERROR_HANDLE_DISK_ਭਰਿਆ )
       || ( pFile->ਆਖਰੀ_Errno==ERROR_DISK_ਭਰਿਆ )){
      OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      ਵਾਪਸ winLogError(SQLITE_ਭਰਿਆ, pFile->ਆਖਰੀ_Errno,
                         "winWrite1", pFile->z_ਰਸਤਾ);
    }
    OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_WRITE\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ਵਾਪਸ winLogError(SQLITE_IOERR_WRITE, pFile->ਆਖਰੀ_Errno,
                       "winWrite2", pFile->z_ਰਸਤਾ);
  }ਨਹੀਂਤਾਂ{
    winLogIoerr(nRetry, __LINE__);
  }
  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
/*
** Truncate the file opened by handle h to nByte bytes in size.
*/
ਸਥਿਰ ਪੂਰਨ winHandleTruncate(HANDLE h, sqlite3_int64 nByte){ 
  ਪੂਰਨ rc = SQLITE_OK;             /* Return code */
  rc = winHandleSeek(h, nByte);
  ਜੇ( rc==SQLITE_OK ){
    ਜੇ( 0==os_ਸੈੱਟ_ਸਿਰਾ_ਦਾ_File(h) ){
      rc = SQLITE_IOERR_TRUNCATE;
    }
  }
  ਵਾਪਸ rc;
}

/*
** Determine the size in bytes of the file opened by the handle passed as 
** the first argument.
*/
ਸਥਿਰ ਪੂਰਨ win_Handle_ਮਾਪ(HANDLE h, sqlite3_int64 *pnByte){ 
  ਪੂਰਨ rc = SQLITE_OK;
  LARGE_INTEGER x;
  ਪ੍ਰਤਿਜ੍ਞਾ( pnByte );
  ਜੇ( os_Get_File_ਮਾਪ_Ex(h, &x)==0 ){
    rc = SQLITE_IOERR_FSTAT;
    *pnByte = 0;
  }ਨਹੀਂਤਾਂ{
    *pnByte = x.QuadPart;
  }
  ਵਾਪਸ rc;
}

/*
** Close the handle passed as the only argument.
*/
ਸਥਿਰ ਖਾਲੀ winHandleClose(HANDLE h){
  ਜੇ( h!=INVALID_HANDLE_ਮੁੱਲ ){
    osCloseHandle(h);
  }
}
#ਜੇ_ਅੰਤ /* #ifndef SQLITE_OMIT_WAL */

/*
** Truncate an open file to a specified size
*/
ਸਥਿਰ ਪੂਰਨ winTruncate(sqlite3_file *id, sqlite3_int64 nByte){
  winFile *pFile = (winFile*)id;  /* File handle object */
  ਪੂਰਨ rc = SQLITE_OK;             /* Return code for this function */
  DWORD ਆਖਰੀ_Errno;
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  sqlite3_int64 old_Mmap_ਮਾਪ;
  ਜੇ( pFile->n_Fetch_ਬਾਹਰਲਾ>0 ){
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
    ਵਾਪਸ SQLITE_OK;
  }
#ਜੇ_ਅੰਤ

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  SimulateIOError(ਵਾਪਸ SQLITE_IOERR_TRUNCATE);
  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, size=%lld, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, nByte, pFile->locktype));

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  ਜੇ( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  ਜੇ( pFile->pMapRegion ){
    old_Mmap_ਮਾਪ = pFile->mmap_ਮਾਪ;
  }ਨਹੀਂਤਾਂ{
    old_Mmap_ਮਾਪ = 0;
  }
  winUnmapfile(pFile);
#ਜੇ_ਅੰਤ

  /* SetEndOfFile() returns non-zero when successful, or zero when it fails. */
  ਜੇ( winSeekFile(pFile, nByte) ){
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->ਆਖਰੀ_Errno,
                     "winTruncate1", pFile->z_ਰਸਤਾ);
  }ਨਹੀਂਤਾਂ ਜੇ( 0==os_ਸੈੱਟ_ਸਿਰਾ_ਦਾ_File(pFile->h) &&
            ((ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error())!=ERROR_USER_MAPPED_FILE) ){
    pFile->ਆਖਰੀ_Errno = ਆਖਰੀ_Errno;
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->ਆਖਰੀ_Errno,
                     "winTruncate2", pFile->z_ਰਸਤਾ);
  }

#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  ਜੇ( rc==SQLITE_OK && old_Mmap_ਮਾਪ>0 ){
    ਜੇ( old_Mmap_ਮਾਪ>nByte ){
      winMapfile(pFile, -1);
    }ਨਹੀਂਤਾਂ{
      winMapfile(pFile, old_Mmap_ਮਾਪ);
    }
  }
#ਜੇ_ਅੰਤ

  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, sqlite3ErrName(rc)));
  ਵਾਪਸ rc;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occurring at the right times.
*/
ਪੂਰਨ sqlite3_sync_ਗਿਣਤੀ = 0;
ਪੂਰਨ sqlite3_fullsync_ਗਿਣਤੀ = 0;
#ਜੇ_ਅੰਤ

/*
** Make sure all writes to a particular file are committed to disk.
*/
ਸਥਿਰ ਪੂਰਨ winSync(sqlite3_file *id, ਪੂਰਨ flags){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_NO_SYNC
  /*
  ** Used only when SQLITE_NO_SYNC is not defined.
   */
  BOOL rc;
#ਜੇ_ਅੰਤ
#ਜੇ !defined(NDEBUG) || !defined(SQLITE_NO_SYNC) || \
    defined(SQLITE_HAVE_OS_TRACE)
  /*
  ** Used when SQLITE_NO_SYNC is not defined and by the assert() and/or
  ** OSTRACE() macros.
   */
  winFile *pFile = (winFile*)id;
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(id);
#ਜੇ_ਅੰਤ

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  ਪ੍ਰਤਿਜ੍ਞਾ((flags&0x0F)==SQLITE_SYNC_ਸਾਧਾਰਨ
      || (flags&0x0F)==SQLITE_SYNC_ਭਰਿਆ
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( ਵਾਪਸ SQLITE_ਭਰਿਆ );

  OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, flags=%x, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, flags,
           pFile->locktype));

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  ਅਣਵਰਤਿਆ_PARAMETER(flags);
#ਨਹੀਂਤਾਂ
  ਜੇ( (flags&0x0F)==SQLITE_SYNC_ਭਰਿਆ ){
    sqlite3_fullsync_ਗਿਣਤੀ++;
  }
  sqlite3_sync_ਗਿਣਤੀ++;
#ਜੇ_ਅੰਤ

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op
  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_NO_SYNC
  OSTRACE(("SYNC-NOP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  ਵਾਪਸ SQLITE_OK;
#ਨਹੀਂਤਾਂ
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  ਜੇ( pFile->pMapRegion ){
    ਜੇ( os_Flush_View_ਦਾ_File(pFile->pMapRegion, 0) ){
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_OK\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
    }ਨਹੀਂਤਾਂ{
      pFile->ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
      ਵਾਪਸ winLogError(SQLITE_IOERR_MMAP, pFile->ਆਖਰੀ_Errno,
                         "winSync1", pFile->z_ਰਸਤਾ);
    }
  }
#ਜੇ_ਅੰਤ
  rc = osFlushFileBuffers(pFile->h);
  SimulateIOError( rc=FALSE );
  ਜੇ( rc ){
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ਵਾਪਸ SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    pFile->ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_FSYNC\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ਵਾਪਸ winLogError(SQLITE_IOERR_FSYNC, pFile->ਆਖਰੀ_Errno,
                       "winSync2", pFile->z_ਰਸਤਾ);
  }
#ਜੇ_ਅੰਤ
}

/*
** Determine the current size of a file in bytes
*/
ਸਥਿਰ ਪੂਰਨ win_File_ਮਾਪ(sqlite3_file *id, sqlite3_int64 *p_ਮਾਪ){
  winFile *pFile = (winFile*)id;
  ਪੂਰਨ rc = SQLITE_OK;

  ਪ੍ਰਤਿਜ੍ਞਾ( id!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮਾਪ!=0 );
  SimulateIOError(ਵਾਪਸ SQLITE_IOERR_FSTAT);
  OSTRACE(("SIZE file=%p, pSize=%p\n", pFile->h, p_ਮਾਪ));
  {
    LARGE_INTEGER x;
    ਜੇ( os_Get_File_ਮਾਪ_Ex(pFile->h, &x)==0 ){
      *p_ਮਾਪ = 0;
      pFile->ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
      rc = winLogError(SQLITE_IOERR_FSTAT, pFile->ਆਖਰੀ_Errno,
                       "winFileSize", pFile->z_ਰਸਤਾ);
    }ਨਹੀਂਤਾਂ{
      *p_ਮਾਪ = x.QuadPart;
    }
  }
  OSTRACE(("SIZE file=%p, pSize=%p, *pSize=%lld, rc=%s\n",
           pFile->h, p_ਮਾਪ, *p_ਮਾਪ, sqlite3ErrName(rc)));
  ਵਾਪਸ rc;
}

/*
** LOCKFILE_FAIL_IMMEDIATELY is undefined on some Windows systems.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LOCKFILE_FAIL_IMMEDIATELY
# define LOCKFILE_FAIL_IMMEDIATELY 1
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LOCKFILE_EXCLUSIVE_ਤਾਲਾ
# define LOCKFILE_EXCLUSIVE_ਤਾਲਾ 2
#ਜੇ_ਅੰਤ

/*
** Historically, SQLite has used both the LockFile and LockFileEx functions.
** When the LockFile function was used, it was always expected to fail
** immediately if the lock could not be obtained.  Also, it always expected to
** obtain an exclusive lock.  These flags are used with the LockFileEx function
** and reflect those expectations; therefore, they should not be changed.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_LOCKFILE_FLAGS
# define SQLITE_LOCKFILE_FLAGS   (LOCKFILE_FAIL_IMMEDIATELY | \
                                  LOCKFILE_EXCLUSIVE_ਤਾਲਾ)
#ਜੇ_ਅੰਤ

/*
** Currently, SQLite never calls the LockFileEx function without wanting the
** call to fail immediately if the lock cannot be obtained.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_LOCKFILEEX_FLAGS
# define SQLITE_LOCKFILEEX_FLAGS (LOCKFILE_FAIL_IMMEDIATELY)
#ਜੇ_ਅੰਤ

/*
** Acquire a reader lock.
** Different API routines are called depending on whether or not this
** is Win9x or WinNT.
*/
ਸਥਿਰ ਪੂਰਨ win_Get_Read_ਤਾਲਾ(winFile *pFile, ਪੂਰਨ b_ਖੰਡ){
  ਪੂਰਨ res;
  DWORD ਨਕਾਬ = ~(b_ਖੰਡ ? LOCKFILE_FAIL_IMMEDIATELY : 0);
  OSTRACE(("READ-LOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = win_ਤਾਲਾ_File(&pFile->h, SQLITE_LOCKFILEEX_FLAGS&ਨਕਾਬ, SHARED_ਪਹਿਲਾ, 0,
                    SHARED_ਮਾਪ, 0);
  ਜੇ( res == 0 ){
    pFile->ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
    /* No need to log a failure to lock */
  }
  OSTRACE(("READ-LOCK file=%p, result=%d\n", pFile->h, res));
  ਵਾਪਸ res;
}

/*
** Undo a readlock
*/
ਸਥਿਰ ਪੂਰਨ win_Unlock_Read_ਤਾਲਾ(winFile *pFile){
  ਪੂਰਨ res;
  DWORD ਆਖਰੀ_Errno;
  OSTRACE(("READ-UNLOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winUnlockFile(&pFile->h, SHARED_ਪਹਿਲਾ, 0, SHARED_ਮਾਪ, 0);
  ਜੇ( res==0 && ((ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error())!=ERROR_NOT_LOCKED) ){
    pFile->ਆਖਰੀ_Errno = ਆਖਰੀ_Errno;
    winLogError(SQLITE_IOERR_UNLOCK, pFile->ਆਖਰੀ_Errno,
                "winUnlockReadLock", pFile->z_ਰਸਤਾ);
  }
  OSTRACE(("READ-UNLOCK file=%p, result=%d\n", pFile->h, res));
  ਵਾਪਸ res;
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
ਸਥਿਰ ਪੂਰਨ win_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ locktype){
  ਪੂਰਨ rc = SQLITE_OK;    /* Return code from subroutines */
  ਪੂਰਨ res = 1;           /* Result of a Windows lock call */
  ਪੂਰਨ newLocktype;       /* Set pFile->locktype to this value before exiting */
  ਪੂਰਨ got_Pending_ਤਾਲਾ = 0;/* True if we acquired a PENDING lock this time */
  winFile *pFile = (winFile*)id;
  DWORD ਆਖਰੀ_Errno = NO_ERROR;

  ਪ੍ਰਤਿਜ੍ਞਾ( id!=0 );
  OSTRACE(("LOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->shared_ਤਾਲਾ_Byte, locktype));

  /* If there is already a lock of this type or more restrictive on the
  ** OsFile, do nothing. Don't use the end_lock: exit path, as
  ** sqlite3OsEnterMutex() hasn't been called yet.
  */
  ਜੇ( pFile->locktype>=locktype ){
    OSTRACE(("LOCK-HELD file=%p, rc=SQLITE_OK\n", pFile->h));
    ਵਾਪਸ SQLITE_OK;
  }

  /* Do not allow any kind of write-lock on a read-only database
  */
  ਜੇ( (pFile->ctrlFlags & WINFILE_RDONLY)!=0 && locktype>=RESERVED_ਤਾਲਾ ){
    ਵਾਪਸ SQLITE_IOERR_ਤਾਲਾ;
  }

  /* Make sure the locking sequence is correct
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->locktype!=NO_ਤਾਲਾ || locktype==SHARED_ਤਾਲਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( locktype!=PENDING_ਤਾਲਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( locktype!=RESERVED_ਤਾਲਾ || pFile->locktype==SHARED_ਤਾਲਾ );

  /* Lock the PENDING_LOCK byte if we need to acquire an EXCLUSIVE lock or
  ** a SHARED lock.  If we are acquiring a SHARED lock, the acquisition of
  ** the PENDING_LOCK byte is temporary.
  */
  newLocktype = pFile->locktype;
  ਜੇ( locktype==SHARED_ਤਾਲਾ
   || (locktype==EXCLUSIVE_ਤਾਲਾ && pFile->locktype==RESERVED_ਤਾਲਾ)
  ){
    ਪੂਰਨ cnt = 3;

    /* Flags for the LockFileEx() call. This should be an exclusive lock if
    ** this call is to obtain EXCLUSIVE, or a shared lock if this call is to
    ** obtain SHARED.  */
    ਪੂਰਨ flags = LOCKFILE_FAIL_IMMEDIATELY;
    ਜੇ( locktype==EXCLUSIVE_ਤਾਲਾ ){
      flags |= LOCKFILE_EXCLUSIVE_ਤਾਲਾ;
    }
    ਜਦੋਂ( cnt>0 ){
      /* Try 3 times to get the pending lock.  This is needed to work
      ** around problems caused by indexing and/or anti-virus software on
      ** Windows systems.
      **
      ** If you are using this code as a model for alternative VFSes, do not
      ** copy this retry logic.  It is a hack intended for Windows only.  */
      res = win_ਤਾਲਾ_File(&pFile->h, flags, PENDING_BYTE, 0, 1, 0);
      ਜੇ( res ) ਰੋਕੋ;

      ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
      OSTRACE(("LOCK-PENDING-FAIL file=%p, count=%d, result=%d\n", 
            pFile->h, cnt, res
      ));

      ਜੇ( ਆਖਰੀ_Errno==ERROR_INVALID_HANDLE ){
        pFile->ਆਖਰੀ_Errno = ਆਖਰੀ_Errno;
        rc = SQLITE_IOERR_ਤਾਲਾ;
        OSTRACE(("LOCK-FAIL file=%p, count=%d, rc=%s\n", 
              pFile->h, cnt, sqlite3ErrName(rc)
        ));
        ਵਾਪਸ rc;
      }

      cnt--;
      ਜੇ( cnt>0 ) osSleep(1);
    }
    got_Pending_ਤਾਲਾ = res;
  }

  /* Acquire a shared lock
  */
  ਜੇ( locktype==SHARED_ਤਾਲਾ && res ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pFile->locktype==NO_ਤਾਲਾ );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
    res = win_Get_Read_ਤਾਲਾ(pFile, pFile->b_ਖੰਡ_On_Connect);
#ਨਹੀਂਤਾਂ
    res = win_Get_Read_ਤਾਲਾ(pFile, 0);
#ਜੇ_ਅੰਤ
    ਜੇ( res ){
      newLocktype = SHARED_ਤਾਲਾ;
    }ਨਹੀਂਤਾਂ{
      ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
    }
  }

  /* Acquire a RESERVED lock
  */
  ਜੇ( locktype==RESERVED_ਤਾਲਾ && res ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pFile->locktype==SHARED_ਤਾਲਾ );
    res = win_ਤਾਲਾ_File(&pFile->h, SQLITE_LOCKFILE_FLAGS, RESERVED_BYTE, 0, 1, 0);
    ਜੇ( res ){
      newLocktype = RESERVED_ਤਾਲਾ;
    }ਨਹੀਂਤਾਂ{
      ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
    }
  }

  /* Acquire a PENDING lock
  */
  ਜੇ( locktype==EXCLUSIVE_ਤਾਲਾ && res ){
    newLocktype = PENDING_ਤਾਲਾ;
    got_Pending_ਤਾਲਾ = 0;
  }

  /* Acquire an EXCLUSIVE lock
  */
  ਜੇ( locktype==EXCLUSIVE_ਤਾਲਾ && res ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pFile->locktype>=SHARED_ਤਾਲਾ );
    (ਖਾਲੀ)win_Unlock_Read_ਤਾਲਾ(pFile);
    res = win_ਤਾਲਾ_File(&pFile->h, SQLITE_LOCKFILE_FLAGS, SHARED_ਪਹਿਲਾ, 0,
                      SHARED_ਮਾਪ, 0);
    ਜੇ( res ){
      newLocktype = EXCLUSIVE_ਤਾਲਾ;
    }ਨਹੀਂਤਾਂ{
      ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
      win_Get_Read_ਤਾਲਾ(pFile, 0);
    }
  }

  /* If we are holding a PENDING lock that ought to be released, then
  ** release it now.
  */
  ਜੇ( got_Pending_ਤਾਲਾ && locktype==SHARED_ਤਾਲਾ ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }

  /* Update the state of the lock has held in the file descriptor then
  ** return the appropriate result code.
  */
  ਜੇ( res ){
    rc = SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    pFile->ਆਖਰੀ_Errno = ਆਖਰੀ_Errno;
    rc = SQLITE_BUSY;
    OSTRACE(("LOCK-FAIL file=%p, wanted=%d, got=%d\n",
             pFile->h, locktype, newLocktype));
  }
  pFile->locktype = (u8)newLocktype;
  OSTRACE(("LOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  ਵਾਪਸ rc;
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, return
** non-zero, otherwise zero.
*/
ਸਥਿਰ ਪੂਰਨ win_Check_Reserved_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  ਪੂਰਨ res;
  winFile *pFile = (winFile*)id;

  SimulateIOError( ਵਾਪਸ SQLITE_IOERR_CHECKRESERVEDLOCK; );
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p\n", pFile->h, p_Res_ਬਾਹਰਲਾ));

  ਪ੍ਰਤਿਜ੍ਞਾ( id!=0 );
  ਜੇ( pFile->locktype>=RESERVED_ਤਾਲਾ ){
    res = 1;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (local)\n", pFile->h, res));
  }ਨਹੀਂਤਾਂ{
    res = win_ਤਾਲਾ_File(&pFile->h, SQLITE_LOCKFILEEX_FLAGS,RESERVED_BYTE,0,1,0);
    ਜੇ( res ){
      winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
    }
    res = !res;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (remote)\n", pFile->h, res));
  }
  *p_Res_ਬਾਹਰਲਾ = res;
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           pFile->h, p_Res_ਬਾਹਰਲਾ, *p_Res_ਬਾਹਰਲਾ));
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ winUnlock(sqlite3_file *id, ਪੂਰਨ locktype){
  ਪੂਰਨ type;
  winFile *pFile = (winFile*)id;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( locktype<=SHARED_ਤਾਲਾ );
  OSTRACE(("UNLOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->shared_ਤਾਲਾ_Byte, locktype));
  type = pFile->locktype;
  ਜੇ( type>=EXCLUSIVE_ਤਾਲਾ ){
    winUnlockFile(&pFile->h, SHARED_ਪਹਿਲਾ, 0, SHARED_ਮਾਪ, 0);
    ਜੇ( locktype==SHARED_ਤਾਲਾ && !win_Get_Read_ਤਾਲਾ(pFile, 0) ){
      /* This should never happen.  We should always be able to
      ** reacquire the read lock */
      rc = winLogError(SQLITE_IOERR_UNLOCK, os_Get_ਆਖਰੀ_Error(),
                       "winUnlock", pFile->z_ਰਸਤਾ);
    }
  }
  ਜੇ( type>=RESERVED_ਤਾਲਾ ){
    winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
  }
  ਜੇ( locktype==NO_ਤਾਲਾ && type>=SHARED_ਤਾਲਾ ){
    win_Unlock_Read_ਤਾਲਾ(pFile);
  }
  ਜੇ( type>=PENDING_ਤਾਲਾ ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }
  pFile->locktype = (u8)locktype;
  OSTRACE(("UNLOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  ਵਾਪਸ rc;
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

ਸਥਿਰ ਪੂਰਨ win_Nolock_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ locktype){
  ਅਣਵਰਤਿਆ_PARAMETER(id);
  ਅਣਵਰਤਿਆ_PARAMETER(locktype);
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ win_Nolock_Check_Reserved_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  ਅਣਵਰਤਿਆ_PARAMETER(id);
  ਅਣਵਰਤਿਆ_PARAMETER(p_Res_ਬਾਹਰਲਾ);
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ winNolockUnlock(sqlite3_file *id, ਪੂਰਨ locktype){
  ਅਣਵਰਤਿਆ_PARAMETER(id);
  ਅਣਵਰਤਿਆ_PARAMETER(locktype);
  ਵਾਪਸ SQLITE_OK;
}

/******************* End of the no-op lock implementation *********************
******************************************************************************/

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
ਸਥਿਰ ਖਾਲੀ win_ਬਹੁਲਕ_ਦੋਆਧਾਰੀ_ਅੰਕ(winFile *pFile, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਨਕਾਬ, ਪੂਰਨ *pArg){
  ਜੇ( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & ਨਕਾਬ)!=0;
  }ਨਹੀਂਤਾਂ ਜੇ( (*pArg)==0 ){
    pFile->ctrlFlags &= ~ਨਕਾਬ;
  }ਨਹੀਂਤਾਂ{
    pFile->ctrlFlags |= ਨਕਾਬ;
  }
}

/* Forward references to VFS helper methods used for temporary files */
ਸਥਿਰ ਪੂਰਨ winGetTempname(sqlite3_vfs *, ਅੱਖਰ **);
ਸਥਿਰ ਪੂਰਨ winIsDir(LPCWSTR);
ਸਥਿਰ BOOL win_Is_Long_ਰਸਤਾ_ਅਗੇਤਰ(ਅਚਲ ਅੱਖਰ *);
ਸਥਿਰ BOOL winIsDriveLetterAndColon(ਅਚਲ ਅੱਖਰ *);

/*
** Control and query of the open file handle.
*/
ਸਥਿਰ ਪੂਰਨ winFileControl(sqlite3_file *id, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  winFile *pFile = (winFile*)id;
  OSTRACE(("FCNTL file=%p, op=%d, pArg=%p\n", pFile->h, op, pArg));
  ਚੋਣ( op ){
    ਹਾਲਤ SQLITE_FCNTL_LOCKSTATE: {
      *(ਪੂਰਨ*)pArg = pFile->locktype;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_ਆਖਰੀ_ERRNO: {
      *(ਪੂਰਨ*)pArg = (ਪੂਰਨ)pFile->ਆਖਰੀ_Errno;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_CHUNK_ਮਾਪ: {
      pFile->szChunk = *(ਪੂਰਨ *)pArg;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_ਮਾਪ_HINT: {
      ਜੇ( pFile->szChunk>0 ){
        sqlite3_int64 oldSz;
        ਪੂਰਨ rc = win_File_ਮਾਪ(id, &oldSz);
        ਜੇ( rc==SQLITE_OK ){
          sqlite3_int64 newSz = *(sqlite3_int64*)pArg;
          ਜੇ( newSz>oldSz ){
            SimulateIOErrorBenign(1);
            rc = winTruncate(id, newSz);
            SimulateIOErrorBenign(0);
          }
        }
        OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
        ਵਾਪਸ rc;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_PERSIST_WAL: {
      win_ਬਹੁਲਕ_ਦੋਆਧਾਰੀ_ਅੰਕ(pFile, WINFILE_PERSIST_WAL, (ਪੂਰਨ*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      win_ਬਹੁਲਕ_ਦੋਆਧਾਰੀ_ਅੰਕ(pFile, WINFILE_PSOW, (ਪੂਰਨ*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_VFSNAME: {
      *(ਅੱਖਰ**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_WIN32_AV_RETRY: {
      ਪੂਰਨ *a = (ਪੂਰਨ*)pArg;
      ਜੇ( a[0]>0 ){
        winIoerrRetry = a[0];
      }ਨਹੀਂਤਾਂ{
        a[0] = winIoerrRetry;
      }
      ਜੇ( a[1]>0 ){
        winIoerrRetryDelay = a[1];
      }ਨਹੀਂਤਾਂ{
        a[1] = winIoerrRetryDelay;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_WIN32_GET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      *phFile = pFile->h;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ਵਾਪਸ SQLITE_OK;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
    ਹਾਲਤ SQLITE_FCNTL_WIN32_ਸੈੱਟ_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      HANDLE hOldFile = pFile->h;
      pFile->h = *phFile;
      *phFile = hOldFile;
      OSTRACE(("FCNTL oldFile=%p, newFile=%p, rc=SQLITE_OK\n",
               hOldFile, pFile->h));
      ਵਾਪਸ SQLITE_OK;
    }
#ਜੇ_ਅੰਤ
    ਹਾਲਤ SQLITE_FCNTL_NULL_IO: {
      (ਖਾਲੀ)osCloseHandle(pFile->h);
      pFile->h = ਨਿਰਾ;
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_TEMPFILENAME: {
      ਅੱਖਰ *zTFile = 0;
      ਪੂਰਨ rc = winGetTempname(pFile->pVfs, &zTFile);
      ਜੇ( rc==SQLITE_OK ){
        *(ਅੱਖਰ**)pArg = zTFile;
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      ਵਾਪਸ rc;
    }
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
    ਹਾਲਤ SQLITE_FCNTL_MMAP_ਮਾਪ: {
      i64 new_ਹੱਦਬੰਦੀ = *(i64*)pArg;
      ਪੂਰਨ rc = SQLITE_OK;
      ਜੇ( new_ਹੱਦਬੰਦੀ>sqlite3GlobalConfig.mxMmap ){
        new_ਹੱਦਬੰਦੀ = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (SIZE_T) and passed
      ** to MapViewOfFile(). Restrict its value to 2GB if (SIZE_T) is not at
      ** least a 64-bit type. */
      ਜੇ( new_ਹੱਦਬੰਦੀ>0 && ਆਕਾਰ(ਮਾਪ_T)<8 ){
        new_ਹੱਦਬੰਦੀ = (new_ਹੱਦਬੰਦੀ & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmap_ਮਾਪ_Max;
      ਜੇ( new_ਹੱਦਬੰਦੀ>=0 && new_ਹੱਦਬੰਦੀ!=pFile->mmap_ਮਾਪ_Max && pFile->n_Fetch_ਬਾਹਰਲਾ==0 ){
        pFile->mmap_ਮਾਪ_Max = new_ਹੱਦਬੰਦੀ;
        ਜੇ( pFile->mmap_ਮਾਪ>0 ){
          winUnmapfile(pFile);
          rc = winMapfile(pFile, -1);
        }
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      ਵਾਪਸ rc;
    }
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
    ਹਾਲਤ SQLITE_FCNTL_ਤਾਲਾ_TIMEOUT: {
      ਪੂਰਨ iOld = pFile->iBusyTimeout;
      ਪੂਰਨ iNew = *(ਪੂਰਨ*)pArg;
#ਜੇ SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = (iNew < 0) ? INFINITE : (DWORD)iNew;
#ਜਾਂਜੇ SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = (DWORD)(!!iNew);
#ਨਹੀਂਤਾਂ
# error "SQLITE_ENABLE_SETLK_TIMEOUT must be set to 1 or 2"
#ਜੇ_ਅੰਤ
      *(ਪੂਰਨ*)pArg = iOld;
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_ਖੰਡ_ON_CONNECT: {
      ਪੂਰਨ iNew = *(ਪੂਰਨ*)pArg;
      pFile->b_ਖੰਡ_On_Connect = iNew;
      ਵਾਪਸ SQLITE_OK;
    }
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_SETLK_TIMEOUT */

#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    ਹਾਲਤ SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      sqlite3_str_appendf(pStr, "{\"h\":%llu", (sqlite3_uint64)pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      ਜੇ( pFile->locktype ){
        ਸਥਿਰ ਅਚਲ ਅੱਖਰ *az_ਤਾਲਾ[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
        sqlite3_str_appendf(pStr, ",\"locktype\":\"%s\"", 
                                  az_ਤਾਲਾ[pFile->locktype-1]);
      }
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
      ਜੇ( pFile->mmap_ਮਾਪ ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmap_ਮਾਪ);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->n_Fetch_ਬਾਹਰਲਾ);
      }
#ਜੇ_ਅੰਤ
      sqlite3_str_append(pStr, "}", 1);
      ਵਾਪਸ SQLITE_OK;
    }
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

  }
  OSTRACE(("FCNTL file=%p, rc=SQLITE_NOTFOUND\n", pFile->h));
  ਵਾਪਸ SQLITE_NOTFOUND;
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
ਸਥਿਰ ਪੂਰਨ win_Sector_ਮਾਪ(sqlite3_file *id){
  (ਖਾਲੀ)id;
  ਵਾਪਸ SQLITE_DEFAULT_SECTOR_ਮਾਪ;
}

/*
** Return a vector of device characteristics.
*/
ਸਥਿਰ ਪੂਰਨ winDeviceCharacteristics(sqlite3_file *id){
  winFile *p = (winFile*)id;
  ਵਾਪਸ SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN | SQLITE_IOCAP_SUBPAGE_READ |
         ((p->ctrlFlags & WINFILE_PSOW)?SQLITE_IOCAP_POWERSAFE_OVERWRITE:0);
}

/*
** Windows will only let you create file view mappings
** on allocation size granularity boundaries.
** During sqlite3_os_init() we do a GetSystemInfo()
** to get the granularity size.
*/
ਸਥਿਰ SYSTEM_INFO winSysInfo;

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
ਸਥਿਰ LPWSTR winConvertFromUtf8Filename(ਅਚਲ ਅੱਖਰ *zFilename){
  LPWSTR zConverted = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __CYGWIN__
  ਪੂਰਨ nChar;

  ਜੇ( os_Cygwin_conv_ਰਸਤਾ && !(winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2])) ){
    i64 nByte;
    ਪੂਰਨ convertflag = CCP_POSIX_ਮੰਜ਼ਿਲ_WIN_W;
    ਜੇ( !strchr(zFilename, '/') ) convertflag |= CCP_RELATIVE;
    nByte = (i64)os_Cygwin_conv_ਰਸਤਾ(convertflag,
        zFilename, 0, 0);
    ਜੇ( nByte>0 ){
      zConverted = sqlite3MallocZero(12+(u64)nByte);
      ਜੇ ( zConverted==0 ){
        ਵਾਪਸ zConverted;
      }
      /* Filenames should be prefixed, except when converted
       * full path already starts with "\\?\". */
      ਜੇ( os_Cygwin_conv_ਰਸਤਾ(convertflag, zFilename,
                           zConverted+4, nByte)==0 ){
        ਜੇ( (convertflag&CCP_RELATIVE) ){
          memmove(zConverted, zConverted+4, nByte);
        }ਨਹੀਂਤਾਂ ਜੇ( memcmp(zConverted+4, L"\\\\", 4) ){
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(zConverted, L"\\\\?\\", 8);
        }ਨਹੀਂਤਾਂ ਜੇ( zConverted[6]!='?' ){
          memmove(zConverted+6, zConverted+4, nByte);
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(zConverted, L"\\\\?\\UNC", 14);
        }ਨਹੀਂਤਾਂ{
          memmove(zConverted, zConverted+4, nByte);
        }
        ਵਾਪਸ zConverted;
      }
      sqlite3_free(zConverted);
    }
  }
  nChar = os_Multi_Byte_ਮੰਜ਼ਿਲ_Wide_Char(CP_UTF8, 0, zFilename, -1, ਨਿਰਾ, 0);
  ਜੇ( nChar==0 ){
    ਵਾਪਸ 0;
  }
  zConverted = sqlite3MallocZero( ((u64)nChar+6)*ਆਕਾਰ(WCHAR) );
  ਜੇ( zConverted==0 ){
    ਵਾਪਸ 0;
  }
  nChar = os_Multi_Byte_ਮੰਜ਼ਿਲ_Wide_Char(CP_UTF8, 0, zFilename, -1,
                                zConverted, nChar);
  ਜੇ( nChar==0 ){
    sqlite3_free(zConverted);
    zConverted = 0;
  }ਨਹੀਂਤਾਂ ਜੇ( nChar>MAX_ਰਸਤਾ
      && winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2]) ){
    memmove(zConverted+4, zConverted, nChar*ਆਕਾਰ(WCHAR));
    zConverted[2] = '\\';
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(zConverted, L"\\\\?\\", 8);
  }ਨਹੀਂਤਾਂ ਜੇ( nChar>MAX_ਰਸਤਾ
      && winIsDirSep(zFilename[0]) && winIsDirSep(zFilename[1])
      && zFilename[2] != '?' ){
    memmove(zConverted+6, zConverted, nChar*ਆਕਾਰ(WCHAR));
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(zConverted, L"\\\\?\\UNC", 14);
  }
#ਨਹੀਂਤਾਂ /* if !defined(__CYGWIN__) */
  zConverted = win_Utf8_ਮੰਜ਼ਿਲ_Unicode(zFilename);
#ਜੇ_ਅੰਤ /* __CYGWIN__ */
  /* caller will handle out of memory */
  ਵਾਪਸ zConverted;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL

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
ਸਥਿਰ sqlite3_mutex *win_Big_ਤਾਲਾ = 0;
ਸਥਿਰ ਖਾਲੀ winShmEnterMutex(ਖਾਲੀ){
  sqlite3_mutex_enter(win_Big_ਤਾਲਾ);
}
ਸਥਿਰ ਖਾਲੀ winShmLeaveMutex(ਖਾਲੀ){
  sqlite3_mutex_leave(win_Big_ਤਾਲਾ);
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
ਸਥਿਰ ਪੂਰਨ winShmMutexHeld(ਖਾਲੀ) {
  ਵਾਪਸ sqlite3_mutex_held(win_Big_ਤਾਲਾ);
}
#ਜੇ_ਅੰਤ

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
ਬਣਤਰ win_Shm_ਗੰਢ {
  sqlite3_mutex *mutex;      /* Mutex to access this object */
  ਅੱਖਰ *zFilename;           /* Name of the file */
  HANDLE hSharedShm;         /* File handle open on zFilename */
  ਪੂਰਨ b_Use_Shared_ਤਾਲਾ_Handle;  /* True to use hSharedShm for everything */

  ਪੂਰਨ isUnlocked;            /* DMS lock has not yet been obtained */
  ਪੂਰਨ isReadonly;            /* True if read-only */
  ਪੂਰਨ szRegion;              /* Size of shared-memory regions */
  ਪੂਰਨ nRegion;               /* Size of array apRegion */

  ਬਣਤਰ ShmRegion {
    HANDLE hMap;             /* File handle from CreateFileMapping */
    ਖਾਲੀ *pMap;
  } *aRegion;
  DWORD ਆਖਰੀ_Errno;           /* The Windows errno from the last I/O error */

  winShm *pWinShmList;       /* List of winShm objects with ptrs to this */

  win_Shm_ਗੰਢ *p_ਅਗਲਾ;         /* Next in list of all winShmNode objects */
#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 ਅਗਲਾ_Shm_Id;              /* Next available winShm.id value */
#ਜੇ_ਅੰਤ
};

/*
** A global array of all winShmNode objects.
**
** The winShmMutexHeld() must be true while reading or writing this list.
*/
ਸਥਿਰ win_Shm_ਗੰਢ *win_Shm_ਗੰਢ_List = 0;

/*
** Structure used internally by this VFS to record the state of an
** open shared memory connection. There is one such structure for each
** winFile open on a wal mode database.
*/
ਬਣਤਰ winShm {
  win_Shm_ਗੰਢ *p_Shm_ਗੰਢ;      /* The underlying winShmNode object */
  u16 shared_ਨਕਾਬ;            /* Mask of shared locks held */
  u16 excl_ਨਕਾਬ;              /* Mask of exclusive locks held */
  HANDLE hShm;               /* File-handle on *-shm file. For locking. */
  ਪੂਰਨ bReadonly;             /* True if hShm is opened read-only */
#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 id;                     /* Id of this connection with its winShmNode */
#ਜੇ_ਅੰਤ
  winShm *p_Win_Shm_ਅਗਲਾ;       /* Next winShm object on same winShmNode */
};

/*
** Constants used for locking
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WIN_SHM_ਅਧਾਰ   ((22+SQLITE_SHM_NLOCK)*4)        /* first lock byte */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WIN_SHM_DMS    (WIN_SHM_ਅਧਾਰ+SQLITE_SHM_NLOCK)  /* deadman switch */

/* Forward references to VFS methods */
ਸਥਿਰ ਪੂਰਨ winOpen(sqlite3_vfs*,ਅਚਲ ਅੱਖਰ*,sqlite3_file*,ਪੂਰਨ,ਪੂਰਨ*);
ਸਥਿਰ ਪੂਰਨ winDelete(sqlite3_vfs *,ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ);

/*
** Purge the winShmNodeList list of all entries with winShmNode.pWinShmList==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
ਸਥਿਰ ਖਾਲੀ winShmPurge(sqlite3_vfs *pVfs, ਪੂਰਨ deleteFlag){
  win_Shm_ਗੰਢ **pp;
  win_Shm_ਗੰਢ *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( winShmMutexHeld() );
  OSTRACE(("SHM-PURGE pid=%lu, deleteFlag=%d\n",
           osGetCurrentProcessId(), deleteFlag));
  pp = &win_Shm_ਗੰਢ_List;
  ਜਦੋਂ( (p = *pp)!=0 ){
    ਜੇ( p->pWinShmList==0 ){
      ਪੂਰਨ i;
      ਜੇ( p->mutex ){ sqlite3_mutex_free(p->mutex); }
      ਲਈ(i=0; i<p->nRegion; i++){
        BOOL bRc = os_Unmap_View_ਦਾ_File(p->aRegion[i].pMap);
        OSTRACE(("SHM-PURGE-UNMAP pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        ਅਣਵਰਤਿਆ_VARIABLE_ਮੁੱਲ(bRc);
        bRc = osCloseHandle(p->aRegion[i].hMap);
        OSTRACE(("SHM-PURGE-CLOSE pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        ਅਣਵਰਤਿਆ_VARIABLE_ਮੁੱਲ(bRc);
      }
      winHandleClose(p->hSharedShm);
      ਜੇ( deleteFlag ){
        SimulateIOErrorBenign(1);
        sqlite3BeginBenignMalloc();
        winDelete(pVfs, p->zFilename, 0);
        sqlite3_ਸਿਰਾ_Benign_Malloc();
        SimulateIOErrorBenign(0);
      }
      *pp = p->p_ਅਗਲਾ;
      sqlite3_free(p->aRegion);
      sqlite3_free(p);
    }ਨਹੀਂਤਾਂ{
      pp = &p->p_ਅਗਲਾ;
    }
  }
}

/*
** Handle h is open on the *-shm file. This function zeroes the first
** 8 bytes of the file. If SQLITE_ENABLE_SETLK_TIMEOUT is defined, h may 
** have been opened with FILE_FLAG_OVERLAPPED.
*/
ਸਥਿਰ ਪੂਰਨ win_Zero_Shared_ਸਮ੍ਰਿਤੀ(HANDLE h){
  u8 aZero[8] = {0,0,0,0,0,0,0,0};
  DWORD nWrite = 0;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, ਆਕਾਰ(OVERLAPPED));
  ਜੇ( !osWriteFile(h, aZero, ਆਕਾਰ(aZero), &nWrite, &ovlp) ){
    /* If the file was opened with FILE_FLAG_OVERLAPPED, the write may still
    ** be pending. Wait for it with GetOverlappedResult().  */
    ਜੇ( os_Get_ਆਖਰੀ_Error()!=ERROR_IO_PENDING
     || !Get_Overlapped_ਨਤੀਜਾ(h, &ovlp, &nWrite, TRUE)
    ){
      nWrite = 0;
    }
  }
  ਵਾਪਸ (nWrite==ਆਕਾਰ(aZero) ? SQLITE_OK : SQLITE_IOERR_WRITE);
}

/*
** The DMS lock has not yet been taken on the shm file associated with
** pShmNode. Take the lock. Truncate the *-shm file if required.
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
ਸਥਿਰ ਪੂਰਨ win_ਤਾਲਾ_Shared_ਸਮ੍ਰਿਤੀ(win_Shm_ਗੰਢ *p_Shm_ਗੰਢ, DWORD nMs){
  HANDLE h = p_Shm_ਗੰਢ->hSharedShm;
  ਪੂਰਨ rc = SQLITE_OK;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_Shm_ਗੰਢ->mutex) );
  rc = win_Handle_ਤਾਲਾ_Timeout(h, WIN_SHM_DMS, 1, 1, 0);
  ਜੇ( rc==SQLITE_OK ){
    /* We have an EXCLUSIVE lock on the DMS byte. This means that this
    ** is the first process to open the file. Truncate it to zero bytes
    ** in this case.  */
    ਜੇ( p_Shm_ਗੰਢ->isReadonly ){
      rc = SQLITE_READONLY_CANTINIT;
    }ਨਹੀਂਤਾਂ{
      ਜੇ( SQLITE_OK!=winHandleTruncate(h, 0) ){
        /* Sometimes the truncate operation may fail because some other
        ** process is still holding an open mapping. So try to write
        ** zeroes into the start of the file instead. */
        rc = win_Zero_Shared_ਸਮ੍ਰਿਤੀ(h);
      }
    }

    /* Release the EXCLUSIVE lock acquired above. */
    winUnlockFile(&h, WIN_SHM_DMS, 0, 1, 0);
  }ਨਹੀਂਤਾਂ ਜੇ( (rc & 0xFF)==SQLITE_BUSY ){
    rc = SQLITE_OK;
  }

  ਜੇ( rc==SQLITE_OK ){
    /* Take a SHARED lock on the DMS byte. */
    rc = win_Handle_ਤਾਲਾ_Timeout(h, WIN_SHM_DMS, 1, 0, nMs);
    ਜੇ( rc==SQLITE_OK ){
      p_Shm_ਗੰਢ->isUnlocked = 0;
    }
  }

  ਵਾਪਸ rc;
}


/*
** This function is used to open a handle on a *-shm file.
**
** If SQLITE_ENABLE_SETLK_TIMEOUT is defined at build time, then the file
** is opened with FILE_FLAG_OVERLAPPED specified. If not, it is not.
*/
ਸਥਿਰ ਪੂਰਨ winHandleOpen(
  ਅਚਲ ਅੱਖਰ *zUtf8,              /* File to open */
  ਪੂਰਨ *pbReadonly,                /* IN/OUT: True for readonly handle */
  HANDLE *ph                      /* OUT: New HANDLE for file */
){
  ਪੂਰਨ rc = SQLITE_OK;
  LPWSTR zConverted = 0;
  ਪੂਰਨ bReadonly = *pbReadonly;
  HANDLE h = INVALID_HANDLE_ਮੁੱਲ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
  ਅਚਲ DWORD flag_overlapped = FILE_FLAG_OVERLAPPED;
#ਨਹੀਂਤਾਂ
  ਅਚਲ DWORD flag_overlapped = 0;
#ਜੇ_ਅੰਤ

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8);
  ਜੇ( zConverted==0 ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8));
    rc = SQLITE_IOERR_NOMEM_BKPT;
    ਜਾਓ winopenfile_ਬਾਹਰਲਾ;
  }

  /* Ensure the file we are trying to open is not actually a directory. */
  ਜੇ( winIsDir(zConverted) ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8));
    rc = SQLITE_CANTOPEN_ISDIR;
    ਜਾਓ winopenfile_ਬਾਹਰਲਾ;
  }

  /* TODO: platforms.
  ** TODO: retry-on-ioerr.
  */
  h = osCreateFileW(zConverted,         /* lpFileName */
      (GENERIC_READ | (bReadonly ? 0 : GENERIC_WRITE)),  /* dwDesiredAccess */
      FILE_SHARE_READ | FILE_SHARE_WRITE,        /* dwShareMode */
      ਨਿਰਾ,                                      /* lpSecurityAttributes */
      OPEN_ALWAYS,                               /* dwCreationDisposition */
      FILE_ATTRIBUTE_ਸਾਧਾਰਨ|flag_overlapped,
      ਨਿਰਾ
  );

  ਜੇ( h==INVALID_HANDLE_ਮੁੱਲ ){
    ਜੇ( bReadonly==0 ){
      bReadonly = 1;
      rc = winHandleOpen(zUtf8, &bReadonly, &h);
    }ਨਹੀਂਤਾਂ{
      rc = SQLITE_CANTOPEN_BKPT;
    }
  }

 winopenfile_ਬਾਹਰਲਾ:
  sqlite3_free(zConverted);
  *pbReadonly = bReadonly;
  *ph = h;
  ਵਾਪਸ rc;
}
 
/*
** Close pDbFd's connection to shared-memory.  Delete the underlying
** *-shm file if deleteFlag is true.
*/
ਸਥਿਰ ਪੂਰਨ win_Close_Shared_ਸਮ੍ਰਿਤੀ(winFile *pDbFd, ਪੂਰਨ deleteFlag){
  winShm *p;            /* The connection to be closed */
  winShm **pp;          /* Iterator for pShmNode->pWinShmList */
  win_Shm_ਗੰਢ *p_Shm_ਗੰਢ; /* The underlying shared-memory file */

  p = pDbFd->pShm;
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_OK;
  ਜੇ( p->hShm!=INVALID_HANDLE_ਮੁੱਲ ){
    osCloseHandle(p->hShm);
  }

  winShmEnterMutex();
  p_Shm_ਗੰਢ = p->p_Shm_ਗੰਢ;

  /* Remove this connection from the winShmNode.pWinShmList list */
  sqlite3_mutex_enter(p_Shm_ਗੰਢ->mutex);
  ਲਈ(pp=&p_Shm_ਗੰਢ->pWinShmList; *pp!=p; pp=&(*pp)->p_Win_Shm_ਅਗਲਾ){}
  *pp = p->p_Win_Shm_ਅਗਲਾ;
  sqlite3_mutex_leave(p_Shm_ਗੰਢ->mutex);

  winShmPurge(pDbFd->pVfs, deleteFlag);
  winShmLeaveMutex();

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  ਵਾਪਸ SQLITE_OK;
}

/*
** testfixture builds may set this global variable to true via a
** Tcl interface. This forces the VFS to use the locking normally
** only used for UNC paths for all files.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
ਪੂਰਨ sqlite3_win_test_unc_locking = 0;
#ਨਹੀਂਤਾਂ
# define sqlite3_win_test_unc_locking 0
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਪੂਰਨ winIsUNCPath(ਅਚਲ ਅੱਖਰ *zFile){
  ਜੇ( zFile[0]=='\\' && zFile[1]=='\\' ){
    ਜੇ( zFile[2]=='?'
     && zFile[3]=='\\'
     && sqlite3Isalpha(zFile[4])
     && zFile[5]==':'
     && winIsDirSep(zFile[6])
    ){
      ਵਾਪਸ sqlite3_win_test_unc_locking;
    }ਨਹੀਂਤਾਂ{
      ਵਾਪਸ 1;
    }
  }
  ਵਾਪਸ sqlite3_win_test_unc_locking;
}

/*
** Return true if the string passed in is the name of a file on a
** remote (network-mounted) volume.
*/
ਸਥਿਰ ਪੂਰਨ winIsRemoteVolume(ਅਚਲ ਅੱਖਰ *zFile){
  WCHAR z_ਜੜ੍ਹ[4];
  ਜੇ( ਲੜੀ_ਤੁਲਨਾ(zFile, "\\\\?\\",4)==0 ) zFile += 4;
  ਜੇ( !sqlite3Isalpha(zFile[0]) ) ਵਾਪਸ 0;
  ਜੇ( zFile[1]!=':' ) ਵਾਪਸ 0;
  ਜੇ( !winIsDirSep(zFile[2]) ) ਵਾਪਸ 0;
  z_ਜੜ੍ਹ[0] = (WCHAR)zFile[0];
  z_ਜੜ੍ਹ[1] = ':';
  z_ਜੜ੍ਹ[2] = '\\';
  z_ਜੜ੍ਹ[3] = 0;
  ਵਾਪਸ osGetDriveTypeW(z_ਜੜ੍ਹ)==DRIVE_REMOTE;
}

/*
** Open the shared-memory area associated with database file pDbFd.
*/
ਸਥਿਰ ਪੂਰਨ win_Open_Shared_ਸਮ੍ਰਿਤੀ(winFile *pDbFd){
  ਬਣਤਰ winShm *p;                  /* The connection to be opened */
  win_Shm_ਗੰਢ *p_Shm_ਗੰਢ = 0;          /* The underlying mmapped file */
  ਪੂਰਨ rc = SQLITE_OK;                /* Result code */
  win_Shm_ਗੰਢ *pNew;                  /* Newly allocated winShmNode */
  ਪੂਰਨ nName;                         /* Size of zName in bytes */

  ਪ੍ਰਤਿਜ੍ਞਾ( pDbFd->pShm==0 );    /* Not previously opened */

  /* Allocate space for the new sqlite3_shm object.  Also speculatively
  ** allocate space for a new winShmNode and filename.  */
  p = sqlite3MallocZero( ਆਕਾਰ(*p) );
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_IOERR_NOMEM_BKPT;
  nName = sqlite3Strlen30(pDbFd->z_ਰਸਤਾ);
  pNew = sqlite3MallocZero( ਆਕਾਰ(*p_Shm_ਗੰਢ) + (i64)nName + 17 );
  ਜੇ( pNew==0 ){
    sqlite3_free(p);
    ਵਾਪਸ SQLITE_IOERR_NOMEM_BKPT;
  }
  pNew->zFilename = (ਅੱਖਰ*)&pNew[1];
  pNew->hSharedShm = INVALID_HANDLE_ਮੁੱਲ;
  pNew->isUnlocked = 1;
  ਜੇ( winIsUNCPath(pDbFd->z_ਰਸਤਾ) || winIsRemoteVolume(pDbFd->z_ਰਸਤਾ) ){
    pNew->b_Use_Shared_ਤਾਲਾ_Handle = 1;
  }ਨਹੀਂਤਾਂ{
    pNew->b_Use_Shared_ਤਾਲਾ_Handle = 0;
  }
  sqlite3_snprintf(nName+15, pNew->zFilename, "%s-shm", pDbFd->z_ਰਸਤਾ);
  sqlite3FileSuffix3(pDbFd->z_ਰਸਤਾ, pNew->zFilename);

  /* Look to see if there is an existing winShmNode that can be used.
  ** If no matching winShmNode currently exists, then create a new one.  */
  winShmEnterMutex();
  ਲਈ(p_Shm_ਗੰਢ = win_Shm_ਗੰਢ_List; p_Shm_ਗੰਢ; p_Shm_ਗੰਢ=p_Shm_ਗੰਢ->p_ਅਗਲਾ){
    /* TBD need to come up with better match here.  Perhaps
    ** use FILE_ID_BOTH_DIR_INFO Structure.  */
    ਜੇ( sqlite3StrICmp(p_Shm_ਗੰਢ->zFilename, pNew->zFilename)==0 ) ਰੋਕੋ;
  }
  ਜੇ( p_Shm_ਗੰਢ==0 ){
    p_Shm_ਗੰਢ = pNew;

    /* Allocate a mutex for this winShmNode object, if one is required. */
    ਜੇ( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_ਗੰਢ->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      ਜੇ( p_Shm_ਗੰਢ->mutex==0 ) rc = SQLITE_IOERR_NOMEM_BKPT;
    }

    /* Open a file-handle to use for mappings, and for the DMS lock. */
    ਜੇ( rc==SQLITE_OK ){
      HANDLE h = INVALID_HANDLE_ਮੁੱਲ;
      p_Shm_ਗੰਢ->isReadonly = sqlite3_uri_boolean(pDbFd->z_ਰਸਤਾ,"readonly_shm",0);
      rc = winHandleOpen(pNew->zFilename, &p_Shm_ਗੰਢ->isReadonly, &h);
      p_Shm_ਗੰਢ->hSharedShm = h;
    }

    /* If successful, link the new winShmNode into the global list. If an 
    ** error occurred, free the object. */
    ਜੇ( rc==SQLITE_OK ){
      p_Shm_ਗੰਢ->p_ਅਗਲਾ = win_Shm_ਗੰਢ_List;
      win_Shm_ਗੰਢ_List = p_Shm_ਗੰਢ;
      pNew = 0;
    }ਨਹੀਂਤਾਂ{
      sqlite3_mutex_free(p_Shm_ਗੰਢ->mutex);
      ਜੇ( p_Shm_ਗੰਢ->hSharedShm!=INVALID_HANDLE_ਮੁੱਲ ){
        osCloseHandle(p_Shm_ਗੰਢ->hSharedShm);
      }
    }
  }

  /* If no error has occurred, link the winShm object to the winShmNode and
  ** the winShm to pDbFd.  */
  ਜੇ( rc==SQLITE_OK ){
    sqlite3_mutex_enter(p_Shm_ਗੰਢ->mutex);
    p->p_Shm_ਗੰਢ = p_Shm_ਗੰਢ;
    p->p_Win_Shm_ਅਗਲਾ = p_Shm_ਗੰਢ->pWinShmList;
    p_Shm_ਗੰਢ->pWinShmList = p;
#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
    p->id = p_Shm_ਗੰਢ->ਅਗਲਾ_Shm_Id++;
#ਜੇ_ਅੰਤ
    pDbFd->pShm = p;
    sqlite3_mutex_leave(p_Shm_ਗੰਢ->mutex);
  }ਨਹੀਂਤਾਂ ਜੇ( p ){
    sqlite3_free(p);
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || p_Shm_ਗੰਢ->isUnlocked==0 || p_Shm_ਗੰਢ->nRegion==0 );
  winShmLeaveMutex();
  sqlite3_free(pNew);

  /* Open a file-handle on the *-shm file for this connection. This file-handle
  ** is only used for locking. The mapping of the *-shm file is created using
  ** the shared file handle in winShmNode.hSharedShm.  */
  ਜੇ( rc==SQLITE_OK && p_Shm_ਗੰਢ->b_Use_Shared_ਤਾਲਾ_Handle==0 ){
    p->bReadonly = sqlite3_uri_boolean(pDbFd->z_ਰਸਤਾ, "readonly_shm", 0);
    rc = winHandleOpen(p_Shm_ਗੰਢ->zFilename, &p->bReadonly, &p->hShm);
    ਜੇ( rc!=SQLITE_OK ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p->hShm==INVALID_HANDLE_ਮੁੱਲ );
      win_Close_Shared_ਸਮ੍ਰਿਤੀ(pDbFd, 0);
    }
  }

  ਵਾਪਸ rc;
}

/*
** Close a connection to shared-memory.  Delete the underlying
** storage if deleteFlag is true.
*/
ਸਥਿਰ ਪੂਰਨ winShmUnmap(
  sqlite3_file *fd,          /* Database holding shared memory */
  ਪੂਰਨ deleteFlag             /* Delete after closing if true */
){
  ਵਾਪਸ win_Close_Shared_ਸਮ੍ਰਿਤੀ((winFile*)fd, deleteFlag);
}

/*
** Change the lock state for a shared-memory segment.
*/
ਸਥਿਰ ਪੂਰਨ win_Shm_ਤਾਲਾ(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  ਪੂਰਨ ofst,                  /* First lock to acquire or release */
  ਪੂਰਨ n,                     /* Number of locks to acquire or release */
  ਪੂਰਨ flags                  /* What to do with the lock */
){
  winFile *pDbFd = (winFile*)fd;        /* Connection holding shared memory */
  winShm *p = pDbFd->pShm;              /* The shared memory being locked */
  win_Shm_ਗੰਢ *p_Shm_ਗੰਢ;
  ਪੂਰਨ rc = SQLITE_OK;                   /* Result code */
  u16 ਨਕਾਬ = (u16)((1U<<(ofst+n)) - (1U<<ofst)); /* Mask of locks to [un]take */

  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_IOERR_SHMLOCK;
  p_Shm_ਗੰਢ = p->p_Shm_ਗੰਢ;
  ਜੇ( NEVER(p_Shm_ਗੰਢ==0) ) ਵਾਪਸ SQLITE_IOERR_SHMLOCK;

  ਪ੍ਰਤਿਜ੍ਞਾ( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  ਪ੍ਰਤਿਜ੍ਞਾ( n>=1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( flags==(SQLITE_SHM_ਤਾਲਾ | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_ਤਾਲਾ | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  ਪ੍ਰਤਿਜ੍ਞਾ( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );

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
#ਜੇ defined(SQLITE_ENABLE_SETLK_TIMEOUT) && defined(SQLITE_DEBUG)
  {
    u16 ਤਾਲਾ_ਨਕਾਬ = (p->excl_ਨਕਾਬ|p->shared_ਨਕਾਬ);
    ਪ੍ਰਤਿਜ੍ਞਾ( (flags & SQLITE_SHM_UNLOCK) || pDbFd->iBusyTimeout==0 || (
          (ofst!=2 || ਤਾਲਾ_ਨਕਾਬ==0)
       && (ofst!=1 || ਤਾਲਾ_ਨਕਾਬ==0 || ਤਾਲਾ_ਨਕਾਬ==2)
       && (ofst!=0 || ਤਾਲਾ_ਨਕਾਬ<3)
       && (ofst<3  || ਤਾਲਾ_ਨਕਾਬ<(1<<ofst))
    ));
  }
#ਜੇ_ਅੰਤ

  /* Check if there is any work to do. There are three cases:
  **
  **    a) An unlock operation where there are locks to unlock,
  **    b) An shared lock where the requested lock is not already held
  **    c) An exclusive lock where the requested lock is not already held
  **
  ** The SQLite core never requests an exclusive lock that it already holds.
  ** This is assert()ed immediately below.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_ਤਾਲਾ) 
       || 0==(p->excl_ਨਕਾਬ & ਨਕਾਬ)
  );
  ਜੇ( ((flags & SQLITE_SHM_UNLOCK) && ((p->excl_ਨਕਾਬ|p->shared_ਨਕਾਬ) & ਨਕਾਬ))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_ਤਾਲਾ) && 0==(p->shared_ਨਕਾਬ & ਨਕਾਬ))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_ਤਾਲਾ))
  ){
    HANDLE h = p->hShm;

    ਜੇ( flags & SQLITE_SHM_UNLOCK ){
      /* Case (a) - unlock.  */

      ਪ੍ਰਤਿਜ੍ਞਾ( (p->excl_ਨਕਾਬ & p->shared_ਨਕਾਬ)==0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->excl_ਨਕਾਬ & ਨਕਾਬ)==ਨਕਾਬ );
      ਪ੍ਰਤਿਜ੍ਞਾ( !(flags & SQLITE_SHM_SHARED) || (p->shared_ਨਕਾਬ & ਨਕਾਬ)==ਨਕਾਬ );

      ਪ੍ਰਤਿਜ੍ਞਾ( !(flags & SQLITE_SHM_SHARED) || n==1 );
      ਜੇ( p_Shm_ਗੰਢ->b_Use_Shared_ਤਾਲਾ_Handle ){
        h = p_Shm_ਗੰਢ->hSharedShm;
        ਜੇ( flags & SQLITE_SHM_SHARED ){
          winShm *pShm;
          sqlite3_mutex_enter(p_Shm_ਗੰਢ->mutex);
          ਲਈ(pShm=p_Shm_ਗੰਢ->pWinShmList; pShm; pShm=pShm->p_Win_Shm_ਅਗਲਾ){
            ਜੇ( pShm!=p && (pShm->shared_ਨਕਾਬ & ਨਕਾਬ) ){
              /* Another connection within this process is also holding this
              ** SHARED lock. So do not actually release the OS lock.  */
              h = INVALID_HANDLE_ਮੁੱਲ;
              ਰੋਕੋ;
            }
          }
          sqlite3_mutex_leave(p_Shm_ਗੰਢ->mutex);
        }
      }

      ਜੇ( h!=INVALID_HANDLE_ਮੁੱਲ ){
        rc = winHandleUnlock(h, ofst+WIN_SHM_ਅਧਾਰ, n);
      }

      /* If successful, also clear the bits in sharedMask/exclMask */
      ਜੇ( rc==SQLITE_OK ){
        p->excl_ਨਕਾਬ = (p->excl_ਨਕਾਬ & ~ਨਕਾਬ);
        p->shared_ਨਕਾਬ = (p->shared_ਨਕਾਬ & ~ਨਕਾਬ);
      }
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ bExcl = ((flags & SQLITE_SHM_EXCLUSIVE) ? 1 : 0);
      DWORD nMs = winFileBusyTimeout(pDbFd);

      ਜੇ( p_Shm_ਗੰਢ->b_Use_Shared_ਤਾਲਾ_Handle ){
        winShm *pShm;
        h = p_Shm_ਗੰਢ->hSharedShm;
        sqlite3_mutex_enter(p_Shm_ਗੰਢ->mutex);
        ਲਈ(pShm=p_Shm_ਗੰਢ->pWinShmList; pShm; pShm=pShm->p_Win_Shm_ਅਗਲਾ){
          ਜੇ( bExcl ){
            ਜੇ( (pShm->shared_ਨਕਾਬ|pShm->excl_ਨਕਾਬ) & ਨਕਾਬ ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_ਮੁੱਲ;
            }
          }ਨਹੀਂਤਾਂ{
            ਜੇ( pShm->shared_ਨਕਾਬ & ਨਕਾਬ ){
              h = INVALID_HANDLE_ਮੁੱਲ;
            }ਨਹੀਂਤਾਂ ਜੇ( pShm->excl_ਨਕਾਬ & ਨਕਾਬ ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_ਮੁੱਲ;
            }
          }
        }
        sqlite3_mutex_leave(p_Shm_ਗੰਢ->mutex);
      }

      ਜੇ( h!=INVALID_HANDLE_ਮੁੱਲ ){
        rc = win_Handle_ਤਾਲਾ_Timeout(h, ofst+WIN_SHM_ਅਧਾਰ, n, bExcl, nMs);
      }
      ਜੇ( rc==SQLITE_OK ){
        ਜੇ( bExcl ){
          p->excl_ਨਕਾਬ = (p->excl_ਨਕਾਬ | ਨਕਾਬ);
        }ਨਹੀਂਤਾਂ{
          p->shared_ਨਕਾਬ = (p->shared_ਨਕਾਬ | ਨਕਾਬ);
        }
      }
    }
  }

  OSTRACE((
      "SHM-LOCK(%d,%d,%d) pid=%lu, id=%d, sharedMask=%03x, exclMask=%03x,"
      " rc=%s\n",
      ofst, n, flags, 
      osGetCurrentProcessId(), p->id, p->shared_ਨਕਾਬ, p->excl_ਨਕਾਬ,
      sqlite3ErrName(rc))
  );
  ਵਾਪਸ rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
ਸਥਿਰ ਖਾਲੀ winShmBarrier(
  sqlite3_file *fd          /* Database holding the shared memory */
){
  ਅਣਵਰਤਿਆ_PARAMETER(fd);
  sqlite3_ਸਮ੍ਰਿਤੀ_Barrier();   /* compiler-defined memory barrier */
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
ਸਥਿਰ ਪੂਰਨ winShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  ਪੂਰਨ iRegion,                    /* Region to retrieve */
  ਪੂਰਨ szRegion,                   /* Size of regions */
  ਪੂਰਨ isWrite,                    /* True to extend file if necessary */
  ਖਾਲੀ ਅਸਥਿਰ **pp              /* OUT: Mapped memory */
){
  winFile *pDbFd = (winFile*)fd;
  winShm *pShm = pDbFd->pShm;
  win_Shm_ਗੰਢ *p_Shm_ਗੰਢ;
  DWORD protect = ਪੰਨਾ_READWRITE;
  DWORD flags = FILE_MAP_WRITE | FILE_MAP_READ;
  ਪੂਰਨ rc = SQLITE_OK;

  ਜੇ( !pShm ){
    rc = win_Open_Shared_ਸਮ੍ਰਿਤੀ(pDbFd);
    ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
    pShm = pDbFd->pShm;
    ਪ੍ਰਤਿਜ੍ਞਾ( pShm!=0 );
  }
  p_Shm_ਗੰਢ = pShm->p_Shm_ਗੰਢ;

  sqlite3_mutex_enter(p_Shm_ਗੰਢ->mutex);
  ਜੇ( p_Shm_ਗੰਢ->isUnlocked ){
    /* Take the DMS lock. */
    ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ->nRegion==0 );
    rc = win_ਤਾਲਾ_Shared_ਸਮ੍ਰਿਤੀ(p_Shm_ਗੰਢ, winFileBusyTimeout(pDbFd));
    ਜੇ( rc!=SQLITE_OK ) ਜਾਓ shmpage_ਬਾਹਰਲਾ;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( szRegion==p_Shm_ਗੰਢ->szRegion || p_Shm_ਗੰਢ->nRegion==0 );
  ਜੇ( p_Shm_ਗੰਢ->nRegion<=iRegion ){
    HANDLE hShared = p_Shm_ਗੰਢ->hSharedShm;
    ਬਣਤਰ ShmRegion *apNew;           /* New aRegion[] array */
    i64 nByte = ((i64)iRegion+1)*(i64)szRegion;  /* Minimum file size */
    sqlite3_int64 sz;                  /* Current size of wal-index file */

    p_Shm_ਗੰਢ->szRegion = szRegion;

    /* The requested region is not mapped into this processes address space.
    ** Check to see if it has been allocated (i.e. if the wal-index file is
    ** large enough to contain the requested region).
    */
    rc = win_Handle_ਮਾਪ(hShared, &sz);
    ਜੇ( rc!=SQLITE_OK ){
      rc = winLogError(rc, os_Get_ਆਖਰੀ_Error(), "winShmMap1", pDbFd->z_ਰਸਤਾ);
      ਜਾਓ shmpage_ਬਾਹਰਲਾ;
    }

    ਜੇ( sz<nByte ){
      /* The requested memory region does not exist. If isWrite is set to
      ** zero, exit early. *pp will be set to NULL and SQLITE_OK returned.
      **
      ** Alternatively, if isWrite is non-zero, use ftruncate() to allocate
      ** the requested memory region.  */
      ਜੇ( !isWrite ) ਜਾਓ shmpage_ਬਾਹਰਲਾ;
      rc = winHandleTruncate(hShared, nByte);
      ਜੇ( rc!=SQLITE_OK ){
        rc = winLogError(rc, os_Get_ਆਖਰੀ_Error(), "winShmMap2", pDbFd->z_ਰਸਤਾ);
        ਜਾਓ shmpage_ਬਾਹਰਲਾ;
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (ਬਣਤਰ ShmRegion*)sqlite3_realloc64(
        p_Shm_ਗੰਢ->aRegion, ((i64)iRegion+1)*ਆਕਾਰ(apNew[0])
    );
    ਜੇ( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      ਜਾਓ shmpage_ਬਾਹਰਲਾ;
    }
    p_Shm_ਗੰਢ->aRegion = apNew;

    ਜੇ( p_Shm_ਗੰਢ->isReadonly ){
      protect = ਪੰਨਾ_READONLY;
      flags = FILE_MAP_READ;
    }

    ਜਦੋਂ( p_Shm_ਗੰਢ->nRegion<=iRegion ){
      HANDLE hMap = ਨਿਰਾ;         /* file-mapping handle */
      ਖਾਲੀ *pMap = 0;             /* Mapped memory region */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
      hMap = osCreateFileMappingFromApp(hShared, ਨਿਰਾ, protect, nByte, ਨਿਰਾ);
#ਨਹੀਂਤਾਂ
      hMap = osCreateFileMappingW(hShared, ਨਿਰਾ, protect, 0, nByte, ਨਿਰਾ);
#ਜੇ_ਅੰਤ
      OSTRACE(("SHM-MAP-CREATE pid=%lu, region=%d, size=%lld, rc=%s\n",
               osGetCurrentProcessId(), p_Shm_ਗੰਢ->nRegion, nByte,
               hMap ? "ok" : "failed"));
      ਜੇ( hMap ){
        i64 i_ਵਿੱਥ = p_Shm_ਗੰਢ->nRegion*szRegion;
        ਪੂਰਨ i_ਵਿੱਥ_ਖਿਸਕਾਓ = i_ਵਿੱਥ % winSysInfo.dw_ਵੰਡ_Granularity;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
        pMap = os_Map_View_ਦਾ_File_From_App(hMap, flags,
            i_ਵਿੱਥ - i_ਵਿੱਥ_ਖਿਸਕਾਓ, (i64)szRegion + i_ਵਿੱਥ_ਖਿਸਕਾਓ
        );
#ਨਹੀਂਤਾਂ
        pMap = os_Map_View_ਦਾ_File(hMap, flags,
            0, i_ਵਿੱਥ - i_ਵਿੱਥ_ਖਿਸਕਾਓ, (i64)szRegion + i_ਵਿੱਥ_ਖਿਸਕਾਓ
        );
#ਜੇ_ਅੰਤ
        OSTRACE(("SHM-MAP-MAP pid=%lu, region=%d, offset=%d, size=%d, rc=%s\n",
                 osGetCurrentProcessId(), p_Shm_ਗੰਢ->nRegion, i_ਵਿੱਥ,
                 szRegion, pMap ? "ok" : "failed"));
      }
      ਜੇ( !pMap ){
        p_Shm_ਗੰਢ->ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
        rc = winLogError(SQLITE_IOERR_SHMMAP, p_Shm_ਗੰਢ->ਆਖਰੀ_Errno,
                         "winShmMap3", pDbFd->z_ਰਸਤਾ);
        ਜੇ( hMap ) osCloseHandle(hMap);
        ਜਾਓ shmpage_ਬਾਹਰਲਾ;
      }

      p_Shm_ਗੰਢ->aRegion[p_Shm_ਗੰਢ->nRegion].pMap = pMap;
      p_Shm_ਗੰਢ->aRegion[p_Shm_ਗੰਢ->nRegion].hMap = hMap;
      p_Shm_ਗੰਢ->nRegion++;
    }
  }

shmpage_ਬਾਹਰਲਾ:
  ਜੇ( p_Shm_ਗੰਢ->nRegion>iRegion ){
    i64 i_ਵਿੱਥ = (i64)iRegion*(i64)szRegion;
    ਪੂਰਨ i_ਵਿੱਥ_ਖਿਸਕਾਓ = i_ਵਿੱਥ % winSysInfo.dw_ਵੰਡ_Granularity;
    ਅੱਖਰ *p = (ਅੱਖਰ *)p_Shm_ਗੰਢ->aRegion[iRegion].pMap;
    *pp = (ਖਾਲੀ *)&p[i_ਵਿੱਥ_ਖਿਸਕਾਓ];
  }ਨਹੀਂਤਾਂ{
    *pp = 0;
  }
  ਜੇ( p_Shm_ਗੰਢ->isReadonly && rc==SQLITE_OK ){
    rc = SQLITE_READONLY;
  }
  sqlite3_mutex_leave(p_Shm_ਗੰਢ->mutex);
  ਵਾਪਸ rc;
}

#ਨਹੀਂਤਾਂ
# define winShmMap     0
# define win_Shm_ਤਾਲਾ    0
# define winShmBarrier 0
# define winShmUnmap   0
#ਜੇ_ਅੰਤ /* #ifndef SQLITE_OMIT_WAL */

/*
** Cleans up the mapped region of the specified file, if any.
*/
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
ਸਥਿਰ ਪੂਰਨ winUnmapfile(winFile *pFile){
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile!=0 );
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, pMapRegion=%p, "
           "mmapSize=%lld, mmapSizeMax=%lld\n",
           osGetCurrentProcessId(), pFile, pFile->hMap, pFile->pMapRegion,
           pFile->mmap_ਮਾਪ, pFile->mmap_ਮਾਪ_Max));
  ਜੇ( pFile->pMapRegion ){
    ਜੇ( !os_Unmap_View_ਦਾ_File(pFile->pMapRegion) ){
      pFile->ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(), pFile,
               pFile->pMapRegion));
      ਵਾਪਸ winLogError(SQLITE_IOERR_MMAP, pFile->ਆਖਰੀ_Errno,
                         "winUnmapfile1", pFile->z_ਰਸਤਾ);
    }
    pFile->pMapRegion = 0;
    pFile->mmap_ਮਾਪ = 0;
  }
  ਜੇ( pFile->hMap!=ਨਿਰਾ ){
    ਜੇ( !osCloseHandle(pFile->hMap) ){
      pFile->ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, rc=SQLITE_IOERR_MMAP\n",
               osGetCurrentProcessId(), pFile, pFile->hMap));
      ਵਾਪਸ winLogError(SQLITE_IOERR_MMAP, pFile->ਆਖਰੀ_Errno,
                         "winUnmapfile2", pFile->z_ਰਸਤਾ);
    }
    pFile->hMap = ਨਿਰਾ;
  }
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile));
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ winMapfile(winFile *pFd, sqlite3_int64 nByte){
  sqlite3_int64 nMap = nByte;
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( nMap>=0 || pFd->n_Fetch_ਬਾਹਰਲਾ==0 );
  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, size=%lld\n",
           osGetCurrentProcessId(), pFd, nByte));

  ਜੇ( pFd->n_Fetch_ਬਾਹਰਲਾ>0 ) ਵਾਪਸ SQLITE_OK;

  ਜੇ( nMap<0 ){
    rc = win_File_ਮਾਪ((sqlite3_file*)pFd, &nMap);
    ਜੇ( rc ){
      OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_IOERR_FSTAT\n",
               osGetCurrentProcessId(), pFd));
      ਵਾਪਸ SQLITE_IOERR_FSTAT;
    }
  }
  ਜੇ( nMap>pFd->mmap_ਮਾਪ_Max ){
    nMap = pFd->mmap_ਮਾਪ_Max;
  }
  nMap &= ~(sqlite3_int64)(winSysInfo.dw_ਪੰਨਾ_ਮਾਪ - 1);

  ਜੇ( nMap==0 && pFd->mmap_ਮਾਪ>0 ){
    winUnmapfile(pFd);
  }
  ਜੇ( nMap!=pFd->mmap_ਮਾਪ ){
    ਖਾਲੀ *pNew = 0;
    DWORD protect = ਪੰਨਾ_READONLY;
    DWORD flags = FILE_MAP_READ;

    winUnmapfile(pFd);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MMAP_READWRITE
    ਜੇ( (pFd->ctrlFlags & WINFILE_RDONLY)==0 ){
      protect = ਪੰਨਾ_READWRITE;
      flags |= FILE_MAP_WRITE;
    }
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
    pFd->hMap = osCreateFileMappingFromApp(pFd->h, ਨਿਰਾ, protect, nMap, ਨਿਰਾ);
#ਨਹੀਂਤਾਂ
    pFd->hMap = osCreateFileMappingW(pFd->h, ਨਿਰਾ, protect,
                                (DWORD)((nMap>>32) & 0xffffffff),
                                (DWORD)(nMap & 0xffffffff), ਨਿਰਾ);
#ਜੇ_ਅੰਤ
    ਜੇ( pFd->hMap==ਨਿਰਾ ){
      pFd->ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->ਆਖਰੀ_Errno,
                       "winMapfile1", pFd->z_ਰਸਤਾ);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-CREATE pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      ਵਾਪਸ SQLITE_OK;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( (nMap % winSysInfo.dw_ਪੰਨਾ_ਮਾਪ)==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(ਮਾਪ_T)==ਆਕਾਰ(sqlite3_int64) || nMap<=0xffffffff );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
    pNew = os_Map_View_ਦਾ_File_From_App(pFd->hMap, flags, 0, (ਮਾਪ_T)nMap);
#ਨਹੀਂਤਾਂ
    pNew = os_Map_View_ਦਾ_File(pFd->hMap, flags, 0, 0, (ਮਾਪ_T)nMap);
#ਜੇ_ਅੰਤ
    ਜੇ( pNew==ਨਿਰਾ ){
      osCloseHandle(pFd->hMap);
      pFd->hMap = ਨਿਰਾ;
      pFd->ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->ਆਖਰੀ_Errno,
                       "winMapfile2", pFd->z_ਰਸਤਾ);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-MAP pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      ਵਾਪਸ SQLITE_OK;
    }
    pFd->pMapRegion = pNew;
    pFd->mmap_ਮਾਪ = nMap;
  }

  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFd));
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ /* SQLITE_MAX_MMAP_SIZE>0 */

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
ਸਥਿਰ ਪੂਰਨ winFetch(sqlite3_file *fd, i64 iOff, ਪੂਰਨ nAmt, ਖਾਲੀ **pp){
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */
#ਜੇ_ਅੰਤ
  *pp = 0;

  OSTRACE(("FETCH pid=%lu, pFile=%p, offset=%lld, amount=%d, pp=%p\n",
           osGetCurrentProcessId(), fd, iOff, nAmt, pp));

#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  ਜੇ( pFd->mmap_ਮਾਪ_Max>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    ਅਚਲ ਪੂਰਨ n_Eof_ਬਫਰ = 256;
    ਜੇ( pFd->pMapRegion==0 ){
      ਪੂਰਨ rc = winMapfile(pFd, -1);
      ਜੇ( rc!=SQLITE_OK ){
        OSTRACE(("FETCH pid=%lu, pFile=%p, rc=%s\n",
                 osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
        ਵਾਪਸ rc;
      }
    }
    ਜੇ( pFd->mmap_ਮਾਪ >= (iOff+nAmt+n_Eof_ਬਫਰ) ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pFd->pMapRegion!=0 );
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->n_Fetch_ਬਾਹਰਲਾ++;
    }
  }
#ਜੇ_ਅੰਤ

  OSTRACE(("FETCH pid=%lu, pFile=%p, pp=%p, *pp=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd, pp, *pp));
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ winUnfetch(sqlite3_file *fd, i64 iOff, ਖਾਲੀ *p){
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( (p==0)==(pFd->n_Fetch_ਬਾਹਰਲਾ==0) );

  /* If p!=0, it must match the iOff value. */
  ਪ੍ਰਤਿਜ੍ਞਾ( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, offset=%lld, p=%p\n",
           osGetCurrentProcessId(), pFd, iOff, p));

  ਜੇ( p ){
    pFd->n_Fetch_ਬਾਹਰਲਾ--;
  }ਨਹੀਂਤਾਂ{
    /* FIXME:  If Windows truly always prevents truncating or deleting a
    ** file while a mapping is held, then the following winUnmapfile() call
    ** is unnecessary can be omitted - potentially improving
    ** performance.  */
    winUnmapfile(pFd);
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( pFd->n_Fetch_ਬਾਹਰਲਾ>=0 );
#ਜੇ_ਅੰਤ

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd));
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਅਚਲ sqlite3_io_methods win_Io_ਵਿਧੀ = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_ਮਾਪ,                    /* xFileSize */
  win_ਤਾਲਾ,                        /* xLock */
  winUnlock,                      /* xUnlock */
  win_Check_Reserved_ਤਾਲਾ,           /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_ਮਾਪ,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  win_Shm_ਤਾਲਾ,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

/*
** This vector defines all the methods that can operate on an
** sqlite3_file for win32 without performing any locking.
*/
ਸਥਿਰ ਅਚਲ sqlite3_io_methods win_Io_Nolock_ਵਿਧੀ = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_ਮਾਪ,                    /* xFileSize */
  win_Nolock_ਤਾਲਾ,                  /* xLock */
  winNolockUnlock,                /* xUnlock */
  win_Nolock_Check_Reserved_ਤਾਲਾ,     /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_ਮਾਪ,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  win_Shm_ਤਾਲਾ,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

ਸਥਿਰ win_Vfs_App_ਅੰਕੜੇ win_App_ਅੰਕੜੇ = {
  &win_Io_ਵਿਧੀ,       /* pMethod */
  0,                  /* pAppData */
  0                   /* bNoLock */
};

ਸਥਿਰ win_Vfs_App_ਅੰਕੜੇ win_Nolock_App_ਅੰਕੜੇ = {
  &win_Io_Nolock_ਵਿਧੀ, /* pMethod */
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
ਸਥਿਰ ਪੂਰਨ win_Make_ਸਿਰਾ_ਵਿੱਚ_Dir_Sep(ਪੂਰਨ nBuf, ਅੱਖਰ *zBuf){
  ਜੇ( zBuf ){
    ਪੂਰਨ nLen = sqlite3Strlen30(zBuf);
    ਜੇ( nLen>0 ){
      ਜੇ( winIsDirSep(zBuf[nLen-1]) ){
        ਵਾਪਸ 1;
      }ਨਹੀਂਤਾਂ ਜੇ( nLen+1<nBuf ){
        ਜੇ( !osGetenv ){
          zBuf[nLen] = winGetDirSep();
        }ਨਹੀਂਤਾਂ ਜੇ( winIsDriveLetterAndColon(zBuf) && winIsDirSep(zBuf[2]) ){
          zBuf[nLen] = '\\';
          zBuf[2]='\\';
        }ਨਹੀਂਤਾਂ{
          zBuf[nLen] = '/';
        }
        zBuf[nLen+1] = '\0';
        ਵਾਪਸ 1;
      }
    }
  }
  ਵਾਪਸ 0;
}

/*
** If sqlite3_temp_directory is defined, take the mutex and return true.
**
** If sqlite3_temp_directory is NULL (undefined), omit the mutex and
** return false.
*/
ਸਥਿਰ ਪੂਰਨ win_ਆਰਜ਼ੀ_Dir_Defined(ਖਾਲੀ){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ਜੇ( sqlite3_ਆਰਜ਼ੀ_directory!=0 ) ਵਾਪਸ 1;
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ਵਾਪਸ 0;
}

/*
** Create a temporary file name and store the resulting pointer into pzBuf.
** The pointer returned in pzBuf must be freed via sqlite3_free().
*/
ਸਥਿਰ ਪੂਰਨ winGetTempname(sqlite3_vfs *pVfs, ਅੱਖਰ **pzBuf){
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ zChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  size_t i, j;
  DWORD pid;
  ਪੂਰਨ nPre = sqlite3Strlen30(SQLITE_ਆਰਜ਼ੀ_FILE_ਅਗੇਤਰ);
  i64 nMax, nBuf, nDir, nLen;
  ਅੱਖਰ *zBuf;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  SimulateIOError( ਵਾਪਸ SQLITE_IOERR );

  /* Allocate a temporary buffer to store the fully qualified file
  ** name for the temporary file.  If this fails, we cannot continue.
  */
  nMax = pVfs->mxPathname;
  nBuf = 2 + (i64)nMax;
  zBuf = sqlite3MallocZero( nBuf );
  ਜੇ( !zBuf ){
    OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
    ਵਾਪਸ SQLITE_IOERR_NOMEM_BKPT;
  }

  /* Figure out the effective temporary directory.  First, check if one
  ** has been explicitly set by the application; otherwise, use the one
  ** configured by the operating system.
  */
  nDir = nMax - (nPre + 15);
  ਪ੍ਰਤਿਜ੍ਞਾ( nDir>0 );
  ਜੇ( win_ਆਰਜ਼ੀ_Dir_Defined() ){
    ਪੂਰਨ nDirLen = sqlite3Strlen30(sqlite3_ਆਰਜ਼ੀ_directory);
    ਜੇ( nDirLen>0 ){
      ਜੇ( !winIsDirSep(sqlite3_ਆਰਜ਼ੀ_directory[nDirLen-1]) ){
        nDirLen++;
      }
      ਜੇ( nDirLen>nDir ){
        sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
        sqlite3_free(zBuf);
        OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
        ਵਾਪਸ winLogError(SQLITE_ERROR, 0, "winGetTempname1", 0);
      }
      sqlite3_snprintf(nMax, zBuf, "%s", sqlite3_ਆਰਜ਼ੀ_directory);
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  }

#ਜੇ defined(__CYGWIN__)
  ਨਹੀਂਤਾਂ ਜੇ( osGetenv!=ਨਿਰਾ ){
    ਸਥਿਰ ਅਚਲ ਅੱਖਰ *azDirs[] = {
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
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
    ਅਚਲ ਅੱਖਰ *zDir = 0;

    ਜੇ( !azDirs[0] ) azDirs[0] = osGetenv("SQLITE_TMPDIR");
    ਜੇ( !azDirs[1] ) azDirs[1] = osGetenv("TMPDIR");
    ਜੇ( !azDirs[2] ) azDirs[2] = osGetenv("TMP");
    ਜੇ( !azDirs[3] ) azDirs[3] = osGetenv("TEMP");
    ਜੇ( !azDirs[4] ) azDirs[4] = osGetenv("USERPROFILE");
    ਲਈ(i=0; i<ਆਕਾਰ(azDirs)/ਆਕਾਰ(azDirs[0]); zDir=azDirs[i++]){
      LPWSTR zConverted;
      ਜੇ( zDir==0 ) ਜਾਰੀ;
      /* If the path starts with a drive letter followed by the colon
      ** character, assume it is already a native Win32 path; otherwise,
      ** it must be converted to a native Win32 path via the Cygwin API
      ** prior to using it.
      */
      {
        zConverted = winConvertFromUtf8Filename(zDir);
        ਜੇ( !zConverted ){
          sqlite3_free(zBuf);
          OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
          ਵਾਪਸ SQLITE_IOERR_NOMEM_BKPT;
        }
        ਜੇ( winIsDir(zConverted) ){
          sqlite3_snprintf(nMax, zBuf, "%s", zDir);
          sqlite3_free(zConverted);
          ਰੋਕੋ;
        }
        sqlite3_free(zConverted);
      }
    }
  }
#ਜੇ_ਅੰਤ

  ਨਹੀਂਤਾਂ{
    ਅੱਖਰ *zMulti;
    LPWSTR z_Wide_ਰਸਤਾ = sqlite3MallocZero( nMax*ਆਕਾਰ(WCHAR) );
    ਜੇ( !z_Wide_ਰਸਤਾ ){
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      ਵਾਪਸ SQLITE_IOERR_NOMEM_BKPT;
    }
    ਜੇ( os_Get_ਆਰਜ਼ੀ_ਰਸਤਾ_W==0 || os_Get_ਆਰਜ਼ੀ_ਰਸਤਾ_W(nMax, z_Wide_ਰਸਤਾ)==0 ){
      sqlite3_free(z_Wide_ਰਸਤਾ);
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_GETTEMPPATH\n"));
      ਵਾਪਸ winLogError(SQLITE_IOERR_GETTEMPPATH, os_Get_ਆਖਰੀ_Error(),
                         "winGetTempname2", 0);
    }
    zMulti = win_Unicode_ਮੰਜ਼ਿਲ_Utf8(z_Wide_ਰਸਤਾ);
    sqlite3_free(z_Wide_ਰਸਤਾ);
    ਜੇ( zMulti ){
      sqlite3_snprintf(nMax, zBuf, "%s", zMulti);
      sqlite3_free(zMulti);
    }ਨਹੀਂਤਾਂ{
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      ਵਾਪਸ SQLITE_IOERR_NOMEM_BKPT;
    }
  }

  /*
  ** Check to make sure the temporary directory ends with an appropriate
  ** separator.  If it does not and there is not enough space left to add
  ** one, fail.
  */
  ਜੇ( !win_Make_ਸਿਰਾ_ਵਿੱਚ_Dir_Sep(nDir+1, zBuf) ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    ਵਾਪਸ winLogError(SQLITE_ERROR, 0, "winGetTempname4", 0);
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
  ਜੇ( (nLen + nPre + 17) > nBuf ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    ਵਾਪਸ winLogError(SQLITE_ERROR, 0, "winGetTempname5", 0);
  }

  sqlite3_snprintf(nBuf-16-nLen, zBuf+nLen, SQLITE_ਆਰਜ਼ੀ_FILE_ਅਗੇਤਰ);

  j = sqlite3Strlen30(zBuf);
  sqlite3_randomness(15, &zBuf[j]);
  pid = osGetCurrentProcessId();
  ਲਈ(i=0; i<15; i++, j++){
    zBuf[j] += pid & 0xff;
    pid >>= 8;
    zBuf[j] = (ਅੱਖਰ)zChars[ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)zBuf[j])%(ਆਕਾਰ(zChars)-1) ];
  }
  zBuf[j] = 0;
  zBuf[j+1] = 0;
  *pzBuf = zBuf;

  OSTRACE(("TEMP-FILENAME name=%s, rc=SQLITE_OK\n", zBuf));
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return TRUE if the named file is really a directory.  Return false if
** it is something other than a directory, or if there is any kind of memory
** allocation failure.
*/
ਸਥਿਰ ਪੂਰਨ winIsDir(LPCWSTR zConverted){
  DWORD attr;
  ਪੂਰਨ rc = 0;
  DWORD ਆਖਰੀ_Errno;
  ਪੂਰਨ cnt = 0;
  WIN32_FILE_ATTRIBUTE_ਅੰਕੜੇ s_Attr_ਅੰਕੜੇ;
  memset(&s_Attr_ਅੰਕੜੇ, 0, ਆਕਾਰ(s_Attr_ਅੰਕੜੇ));
  ਜਦੋਂ( !(rc = os_Get_File_ਗੁਣ_Ex_W(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_ਅੰਕੜੇ)) && winRetryIoerr(&cnt, &ਆਖਰੀ_Errno) ){}
  ਜੇ( !rc ){
    ਵਾਪਸ 0; /* Invalid name? */
  }
  attr = s_Attr_ਅੰਕੜੇ.dw_File_ਗੁਣ;
  ਵਾਪਸ (attr!=INVALID_FILE_ਗੁਣ) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

/* forward reference */
ਸਥਿਰ ਪੂਰਨ winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ਅਚਲ ਅੱਖਰ *zFilename,     /* Name of file to check */
  ਪੂਰਨ flags,                 /* Type of test to make on this file */
  ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ               /* OUT: Result */
);

/*
** The Windows version of xAccess() accepts an extra bit in the flags
** parameter that prevents an anti-virus retry loop.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NORETRY 0x4000

/*
** Open a file.
*/
ਸਥਿਰ ਪੂਰਨ winOpen(
  sqlite3_vfs *pVfs,        /* Used to get maximum path length and AppData */
  ਅਚਲ ਅੱਖਰ *zName,        /* Name of the file (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  ਪੂਰਨ flags,                /* Open mode flags */
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags            /* Status return flags */
){
  HANDLE h;
  DWORD ਆਖਰੀ_Errno = 0;
  DWORD dwDesiredAccess;
  DWORD dw_Share_ਬਹੁਲਕ;
  DWORD dwCreationDisposition;
  DWORD dw_Flags_And_ਗੁਣ = 0;
  win_Vfs_App_ਅੰਕੜੇ *p_App_ਅੰਕੜੇ;
  winFile *pFile = (winFile*)id;
  LPWSTR zConverted;              /* Filename in OS encoding */
  ਅਚਲ ਅੱਖਰ *zUtf8Name = zName; /* Filename in UTF-8 encoding */
  ਪੂਰਨ cnt = 0;
  ਪੂਰਨ isRO = 0;              /* file is known to be accessible readonly */

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  ਅੱਖਰ *zTmpname = 0; /* For temporary filename, if necessary. */

  ਪੂਰਨ rc = SQLITE_OK;            /* Function Return Code */
#ਜੇ !defined(NDEBUG)
  ਪੂਰਨ eType = flags&0x0FFF00;  /* Type of file to open */
#ਜੇ_ਅੰਤ

  ਪੂਰਨ isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  ਪੂਰਨ isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  ਪੂਰਨ isCreate     = (flags & SQLITE_OPEN_CREATE);
  ਪੂਰਨ isReadonly   = (flags & SQLITE_OPEN_READONLY);
  ਪੂਰਨ isReadWrite  = (flags & SQLITE_OPEN_READWRITE);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  ਪੂਰਨ isOpenJournal = (isCreate && (
        eType==SQLITE_OPEN_SUPER_JOURNAL
     || eType==SQLITE_OPEN_MAIN_JOURNAL
     || eType==SQLITE_OPEN_WAL
  ));
#ਜੇ_ਅੰਤ

  OSTRACE(("OPEN name=%s, pFile=%p, flags=%x, pOutFlags=%p\n",
           zUtf8Name, id, flags, p_ਬਾਹਰਲਾ_Flags));

  /* Check the following statements are true:
  **
  **   (a) Exactly one of the READWRITE and READONLY flags must be set, and
  **   (b) if CREATE is set, then READWRITE must also be set, and
  **   (c) if EXCLUSIVE is set, then CREATE must also be set.
  **   (d) if DELETEONCLOSE is set, then CREATE must also be set.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
  ਪ੍ਰਤਿਜ੍ਞਾ(isCreate==0 || isReadWrite);
  ਪ੍ਰਤਿਜ੍ਞਾ(isExclusive==0 || isCreate);
  ਪ੍ਰਤਿਜ੍ਞਾ(isDelete==0 || isCreate);

  /* The main DB, main journal, WAL file and super-journal are never
  ** automatically deleted. Nor are they ever temporary files.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_DB );
  ਪ੍ਰਤਿਜ੍ਞਾ( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_JOURNAL );
  ਪ੍ਰਤਿਜ੍ਞਾ( (!isDelete && zName) || eType!=SQLITE_OPEN_SUPER_JOURNAL );
  ਪ੍ਰਤਿਜ੍ਞਾ( (!isDelete && zName) || eType!=SQLITE_OPEN_WAL );

  /* Assert that the upper layer has set one of the "file-type" flags. */
  ਪ੍ਰਤਿਜ੍ਞਾ( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_ਆਰਜ਼ੀ_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_ਆਰਜ਼ੀ_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile!=0 );
  memset(pFile, 0, ਆਕਾਰ(winFile));
  pFile->h = INVALID_HANDLE_ਮੁੱਲ;

  /* If the second argument to this function is NULL, generate a
  ** temporary file name to use
  */
  ਜੇ( !zUtf8Name ){
    ਪ੍ਰਤਿਜ੍ਞਾ( isDelete && !isOpenJournal );
    rc = winGetTempname(pVfs, &zTmpname);
    ਜੇ( rc!=SQLITE_OK ){
      OSTRACE(("OPEN name=%s, rc=%s", zUtf8Name, sqlite3ErrName(rc)));
      ਵਾਪਸ rc;
    }
    zUtf8Name = zTmpname;
  }

  /* Database filenames are double-zero terminated if they are not
  ** URIs with parameters.  Hence, they can always be passed into
  ** sqlite3_uri_parameter().
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( (eType!=SQLITE_OPEN_MAIN_DB) || (flags & SQLITE_OPEN_URI) ||
       zUtf8Name[sqlite3Strlen30(zUtf8Name)+1]==0 );

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8Name);
  ਜੇ( zConverted==0 ){
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8Name));
    ਵਾਪਸ SQLITE_IOERR_NOMEM_BKPT;
  }

  ਜੇ( winIsDir(zConverted) ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8Name));
    ਵਾਪਸ SQLITE_CANTOPEN_ISDIR;
  }

  ਜੇ( isReadWrite ){
    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
  }ਨਹੀਂਤਾਂ{
    dwDesiredAccess = GENERIC_READ;
  }

  /* SQLITE_OPEN_EXCLUSIVE is used to make sure that a new file is
  ** created. SQLite doesn't use it to indicate "exclusive access"
  ** as it is usually understood.
  */
  ਜੇ( isExclusive ){
    /* Creates a new file, only if it does not already exist. */
    /* If the file exists, it fails. */
    dwCreationDisposition = CREATE_NEW;
  }ਨਹੀਂਤਾਂ ਜੇ( isCreate ){
    /* Open existing file, or create if it doesn't exist */
    dwCreationDisposition = OPEN_ALWAYS;
  }ਨਹੀਂਤਾਂ{
    /* Opens a file, only if it exists. */
    dwCreationDisposition = OPEN_EXISTING;
  }

  ਜੇ( 0==sqlite3_uri_boolean(zName, "exclusive", 0) ){
    dw_Share_ਬਹੁਲਕ = FILE_SHARE_READ | FILE_SHARE_WRITE;
  }ਨਹੀਂਤਾਂ{
    dw_Share_ਬਹੁਲਕ = 0;
  }

  ਜੇ( isDelete ){
    dw_Flags_And_ਗੁਣ = FILE_ATTRIBUTE_TEMPORARY
                               | FILE_ATTRIBUTE_HIDDEN
                               | FILE_FLAG_DELETE_ON_CLOSE;
  }ਨਹੀਂਤਾਂ{
    dw_Flags_And_ਗੁਣ = FILE_ATTRIBUTE_ਸਾਧਾਰਨ;
  }
  ਕਰੋ{
    h = osCreateFileW(zConverted,
                      dwDesiredAccess,
                      dw_Share_ਬਹੁਲਕ, ਨਿਰਾ,
                      dwCreationDisposition,
                      dw_Flags_And_ਗੁਣ,
                      ਨਿਰਾ);
    ਜੇ( h!=INVALID_HANDLE_ਮੁੱਲ ) ਰੋਕੋ;
    ਜੇ( isReadWrite ){
      ਪੂਰਨ rc2;
      sqlite3BeginBenignMalloc();
      rc2 = winAccess(pVfs, zUtf8Name, SQLITE_ACCESS_READ|NORETRY, &isRO);
      sqlite3_ਸਿਰਾ_Benign_Malloc();
      ਜੇ( rc2==SQLITE_OK && isRO ) ਰੋਕੋ;
    }
  }ਜਦੋਂ( winRetryIoerr(&cnt, &ਆਖਰੀ_Errno) );
  winLogIoerr(cnt, __LINE__);

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, rc=%s\n", h, zUtf8Name,
           dwDesiredAccess, (h==INVALID_HANDLE_ਮੁੱਲ) ? "failed" : "ok"));

  ਜੇ( h==INVALID_HANDLE_ਮੁੱਲ ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    ਜੇ( isReadWrite && isRO && !isExclusive ){
      ਵਾਪਸ winOpen(pVfs, zName, id,
         ((flags|SQLITE_OPEN_READONLY) &
                     ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE)),
         p_ਬਾਹਰਲਾ_Flags);
    }ਨਹੀਂਤਾਂ{
      pFile->ਆਖਰੀ_Errno = ਆਖਰੀ_Errno;
      winLogError(SQLITE_CANTOPEN, pFile->ਆਖਰੀ_Errno, "winOpen", zUtf8Name);
      ਵਾਪਸ SQLITE_CANTOPEN_BKPT;
    }
  }

  ਜੇ( p_ਬਾਹਰਲਾ_Flags ){
    ਜੇ( isReadWrite ){
      *p_ਬਾਹਰਲਾ_Flags = SQLITE_OPEN_READWRITE;
    }ਨਹੀਂਤਾਂ{
      *p_ਬਾਹਰਲਾ_Flags = SQLITE_OPEN_READONLY;
    }
  }

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, pOutFlags=%p, *pOutFlags=%d, "
           "rc=%s\n", h, zUtf8Name, dwDesiredAccess, p_ਬਾਹਰਲਾ_Flags, p_ਬਾਹਰਲਾ_Flags ?
           *p_ਬਾਹਰਲਾ_Flags : 0, (h==INVALID_HANDLE_ਮੁੱਲ) ? "failed" : "ok"));

  p_App_ਅੰਕੜੇ = (win_Vfs_App_ਅੰਕੜੇ*)pVfs->p_App_ਅੰਕੜੇ;

  sqlite3_free(zConverted);
  sqlite3_free(zTmpname);
  id->pMethods = p_App_ਅੰਕੜੇ ? p_App_ਅੰਕੜੇ->p_ਵਿਧੀ : &win_Io_ਵਿਧੀ;
  pFile->pVfs = pVfs;
  pFile->h = h;
  ਜੇ( isReadonly ){
    pFile->ctrlFlags |= WINFILE_RDONLY;
  }
  ਜੇ( (flags & SQLITE_OPEN_MAIN_DB)
   && sqlite3_uri_boolean(zName, "psow", SQLITE_POWERSAFE_OVERWRITE) 
  ){
    pFile->ctrlFlags |= WINFILE_PSOW;
  }
  pFile->ਆਖਰੀ_Errno = NO_ERROR;
  pFile->z_ਰਸਤਾ = zName;
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  pFile->hMap = ਨਿਰਾ;
  pFile->pMapRegion = 0;
  pFile->mmap_ਮਾਪ = 0;
  pFile->mmap_ਮਾਪ_Max = sqlite3GlobalConfig.szMmap;
#ਜੇ_ਅੰਤ

  OpenCounter(+1);
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ winDelete(
  sqlite3_vfs *pVfs,          /* Not used on win32 */
  ਅਚਲ ਅੱਖਰ *zFilename,      /* Name of file to delete */
  ਪੂਰਨ syncDir                 /* Not used on win32 */
){
  ਪੂਰਨ cnt = 0;
  ਪੂਰਨ rc;
  DWORD attr;
  DWORD ਆਖਰੀ_Errno = 0;
  LPWSTR zConverted;
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);
  ਅਣਵਰਤਿਆ_PARAMETER(syncDir);

  SimulateIOError(ਵਾਪਸ SQLITE_IOERR_DELETE);
  OSTRACE(("DELETE name=%s, syncDir=%d\n", zFilename, syncDir));

  zConverted = winConvertFromUtf8Filename(zFilename);
  ਜੇ( zConverted==0 ){
    OSTRACE(("DELETE name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    ਵਾਪਸ SQLITE_IOERR_NOMEM_BKPT;
  }
  ਕਰੋ {
    attr = os_Get_File_ਗੁਣ_W(zConverted);
    ਜੇ ( attr==INVALID_FILE_ਗੁਣ ){
      ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
      ਜੇ( ਆਖਰੀ_Errno==ERROR_FILE_NOT_ਮਿਲਿਆ
       || ਆਖਰੀ_Errno==ERROR_ਰਸਤਾ_NOT_ਮਿਲਿਆ ){
        rc = SQLITE_IOERR_DELETE_NOENT; /* Already gone? */
      }ਨਹੀਂਤਾਂ{
        rc = SQLITE_ERROR;
      }
      ਰੋਕੋ;
    }
    ਜੇ ( attr&FILE_ATTRIBUTE_DIRECTORY ){
      rc = SQLITE_ERROR; /* Files only. */
      ਰੋਕੋ;
    }
    ਜੇ ( osDeleteFileW(zConverted) ){
      rc = SQLITE_OK; /* Deleted OK. */
      ਰੋਕੋ;
    }
    ਜੇ ( !winRetryIoerr(&cnt, &ਆਖਰੀ_Errno) ){
      rc = SQLITE_ERROR; /* No more retries. */
      ਰੋਕੋ;
    }
  } ਜਦੋਂ(1);
  ਜੇ( rc && rc!=SQLITE_IOERR_DELETE_NOENT ){
    rc = winLogError(SQLITE_IOERR_DELETE, ਆਖਰੀ_Errno, "winDelete", zFilename);
  }ਨਹੀਂਤਾਂ{
    winLogIoerr(cnt, __LINE__);
  }
  sqlite3_free(zConverted);
  OSTRACE(("DELETE name=%s, rc=%s\n", zFilename, sqlite3ErrName(rc)));
  ਵਾਪਸ rc;
}

/*
** Check the existence and status of a file.
*/
ਸਥਿਰ ਪੂਰਨ winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ਅਚਲ ਅੱਖਰ *zFilename,     /* Name of file to check */
  ਪੂਰਨ flags,                 /* Type of test to make on this file */
  ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ               /* OUT: Result */
){
  DWORD attr;
  ਪੂਰਨ rc = 0;
  ਪੂਰਨ cnt = 0;
  DWORD ਆਖਰੀ_Errno = 0;
  LPWSTR zConverted;
  ਪੂਰਨ noRetry = 0;           /* Do not use winRetryIoerr() */
  WIN32_FILE_ATTRIBUTE_ਅੰਕੜੇ s_Attr_ਅੰਕੜੇ;
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);

  ਜੇ( (flags & NORETRY)!=0 ){
    noRetry = 1;
    flags &= ~NORETRY;
  }  

  SimulateIOError( ਵਾਪਸ SQLITE_IOERR_ACCESS; );
  OSTRACE(("ACCESS name=%s, flags=%x, pResOut=%p\n",
           zFilename, flags, p_Res_ਬਾਹਰਲਾ));

  ਜੇ( zFilename==0 ){
    *p_Res_ਬਾਹਰਲਾ = 0;
    OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
             zFilename, p_Res_ਬਾਹਰਲਾ, *p_Res_ਬਾਹਰਲਾ));
    ਵਾਪਸ SQLITE_OK;
  }

  zConverted = winConvertFromUtf8Filename(zFilename);
  ਜੇ( zConverted==0 ){
    OSTRACE(("ACCESS name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    ਵਾਪਸ SQLITE_IOERR_NOMEM_BKPT;
  }
  memset(&s_Attr_ਅੰਕੜੇ, 0, ਆਕਾਰ(s_Attr_ਅੰਕੜੇ));
  ਜਦੋਂ( !(rc = os_Get_File_ਗੁਣ_Ex_W(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_ਅੰਕੜੇ))
     && !noRetry
     && winRetryIoerr(&cnt, &ਆਖਰੀ_Errno)
  ){ /* Loop until true */}
  ਜੇ( rc ){
    /* For an SQLITE_ACCESS_EXISTS query, treat a zero-length file
    ** as if it does not exist.
    */
    ਜੇ(    flags==SQLITE_ACCESS_EXISTS
        && s_Attr_ਅੰਕੜੇ.n_File_ਮਾਪ_ਉੱਚਾ==0
        && s_Attr_ਅੰਕੜੇ.n_File_ਮਾਪ_ਨੀਵਾਂ==0 ){
      attr = INVALID_FILE_ਗੁਣ;
    }ਨਹੀਂਤਾਂ{
      attr = s_Attr_ਅੰਕੜੇ.dw_File_ਗੁਣ;
    }
  }ਨਹੀਂਤਾਂ{
    ਜੇ( noRetry ) ਆਖਰੀ_Errno = os_Get_ਆਖਰੀ_Error();
    winLogIoerr(cnt, __LINE__);
    ਜੇ( ਆਖਰੀ_Errno!=ERROR_FILE_NOT_ਮਿਲਿਆ && ਆਖਰੀ_Errno!=ERROR_ਰਸਤਾ_NOT_ਮਿਲਿਆ ){
      sqlite3_free(zConverted);
      ਵਾਪਸ winLogError(SQLITE_IOERR_ACCESS, ਆਖਰੀ_Errno, "winAccess",
                         zFilename);
    }ਨਹੀਂਤਾਂ{
      attr = INVALID_FILE_ਗੁਣ;
    }
  }
  sqlite3_free(zConverted);
  ਚੋਣ( flags ){
    ਹਾਲਤ SQLITE_ACCESS_READ:
    ਹਾਲਤ SQLITE_ACCESS_EXISTS:
      rc = attr!=INVALID_FILE_ਗੁਣ;
      ਰੋਕੋ;
    ਹਾਲਤ SQLITE_ACCESS_READWRITE:
      rc = attr!=INVALID_FILE_ਗੁਣ &&
             (attr & FILE_ATTRIBUTE_READONLY)==0;
      ਰੋਕੋ;
    ਮੂਲ:
      ਪ੍ਰਤਿਜ੍ਞਾ(!"Invalid flags argument");
  }
  *p_Res_ਬਾਹਰਲਾ = rc;
  OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           zFilename, p_Res_ਬਾਹਰਲਾ, *p_Res_ਬਾਹਰਲਾ));
  ਵਾਪਸ SQLITE_OK;
}

/*
** Returns non-zero if the specified path name starts with the "long path"
** prefix.
*/
ਸਥਿਰ BOOL win_Is_Long_ਰਸਤਾ_ਅਗੇਤਰ(
  ਅਚਲ ਅੱਖਰ *zPathname
){
  ਵਾਪਸ ( zPathname[0]=='\\' && zPathname[1]=='\\'
        && zPathname[2]=='?'  && zPathname[3]=='\\' );
}

/*
** Returns non-zero if the specified path name starts with a drive letter
** followed by a colon character.
*/
ਸਥਿਰ BOOL winIsDriveLetterAndColon(
  ਅਚਲ ਅੱਖਰ *zPathname
){
  ਵਾਪਸ ( sqlite3Isalpha(zPathname[0]) && zPathname[1]==':' );
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
/*
** Returns non-zero if the specified path name should be used verbatim.  If
** non-zero is returned from this function, the calling function must simply
** use the provided path name verbatim -OR- resolve it into a full path name
** using the GetFullPathName Win32 API function (if available).
*/
ਸਥਿਰ BOOL winIsVerbatimPathname(
  ਅਚਲ ਅੱਖਰ *zPathname
){
  /*
  ** If the path name starts with a forward slash or a backslash, it is either
  ** a legal UNC name, a volume relative path, or an absolute path name in the
  ** "Unix" format on Windows.  There is no easy way to differentiate between
  ** the final two cases; therefore, we return the safer return value of TRUE
  ** so that callers of this function will simply use it verbatim.
  */
  ਜੇ ( winIsDirSep(zPathname[0]) ){
    ਵਾਪਸ TRUE;
  }

  /*
  ** If the path name starts with a letter and a colon it is either a volume
  ** relative path or an absolute path.  Callers of this function must not
  ** attempt to treat it as a relative path name (i.e. they should simply use
  ** it verbatim).
  */
  ਜੇ ( winIsDriveLetterAndColon(zPathname) ){
    ਵਾਪਸ TRUE;
  }

  /*
  ** If we get to this point, the path name should almost certainly be a purely
  ** relative one (i.e. not a UNC name, not absolute, and not volume relative).
  */
  ਵਾਪਸ FALSE;
}
#ਜੇ_ਅੰਤ /* _WIN32 */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __CYGWIN__
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
ਸਥਿਰ ਖਾਲੀ winSimplifyName(ਅੱਖਰ *z){
  ਪੂਰਨ i, j;
  ਲਈ(i=j=0; z[i]; ++i){
    ਜੇ( winIsDirSep(z[i]) ){
#ਜੇ !defined(SQLITE_TEST)
      /* Some test-cases assume that "./foo" and "foo" are different */
      ਜੇ( z[i+1]=='.' && winIsDirSep(z[i+2]) ){
        ++i;
        ਜਾਰੀ;
      }
#ਜੇ_ਅੰਤ
      ਜੇ( !z[i+1] || (winIsDirSep(z[i+1]) && (i!=0)) ){
        ਜਾਰੀ;
      }
      z[j++] = osGetenv?'/':'\\';
    }ਨਹੀਂਤਾਂ{
      z[j++] = z[i];
    }
  }
  ਜਦੋਂ(j<i) z[j++] = '\0';
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MAX_SYMLINKS 100

ਸਥਿਰ ਪੂਰਨ mk_ਭਰਿਆ_Pathname(
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,              /* Input path */
  ਅੱਖਰ *z_ਬਾਹਰਲਾ,                     /* Output buffer */
  ਪੂਰਨ n_ਬਾਹਰਲਾ                        /* Allocated size of buffer zOut */
){
  ਪੂਰਨ n_ਰਸਤਾ = sqlite3Strlen30(z_ਰਸਤਾ);
  ਪੂਰਨ iOff = 0;
  ਜੇ( z_ਰਸਤਾ[0]!='/' ){
    ਜੇ( osGetcwd(z_ਬਾਹਰਲਾ, n_ਬਾਹਰਲਾ-2)==0 ){
      ਵਾਪਸ winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno, "getcwd", z_ਰਸਤਾ);
    }
    iOff = sqlite3Strlen30(z_ਬਾਹਰਲਾ);
    z_ਬਾਹਰਲਾ[iOff++] = '/';
  }
  ਜੇ( (iOff+n_ਰਸਤਾ+1)>n_ਬਾਹਰਲਾ ){
    /* SQLite assumes that xFullPathname() nul-terminates the output buffer
    ** even if it returns an error.  */
    z_ਬਾਹਰਲਾ[iOff] = '\0';
    ਵਾਪਸ SQLITE_CANTOPEN_BKPT;
  }
  sqlite3_snprintf(n_ਬਾਹਰਲਾ-iOff, &z_ਬਾਹਰਲਾ[iOff], "%s", z_ਰਸਤਾ);
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ /* __CYGWIN__ */

/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
ਸਥਿਰ ਪੂਰਨ win_ਭਰਿਆ_Pathname_No_Mutex(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ਅਚਲ ਅੱਖਰ *zRelative,        /* Possibly relative input path */
  ਪੂਰਨ n_ਭਰਿਆ,                    /* Size of output buffer in bytes */
  ਅੱਖਰ *z_ਭਰਿਆ                   /* Output buffer */
){
  ਪੂਰਨ nByte;
  LPWSTR zConverted;
  ਅੱਖਰ *z_ਬਾਹਰਲਾ;

  /* If this path name begins with "/X:" or "\\?\", where "X" is any
  ** alphabetic character, discard the initial "/" from the pathname.
  */
  ਜੇ( zRelative[0]=='/' && (winIsDriveLetterAndColon(zRelative+1)
       || win_Is_Long_ਰਸਤਾ_ਅਗੇਤਰ(zRelative+1)) ){
    zRelative++;
  }

  SimulateIOError( ਵਾਪਸ SQLITE_ERROR );

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __CYGWIN__
  ਜੇ( osGetcwd ){
    z_ਭਰਿਆ[n_ਭਰਿਆ-1] = '\0';
    ਜੇ( !winIsDriveLetterAndColon(zRelative) || !winIsDirSep(zRelative[2]) ){
      ਪੂਰਨ rc = SQLITE_OK;
      ਪੂਰਨ nLink = 1;              /* Number of symbolic links followed so far */
      ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ = zRelative; /* Input path for each iteration of loop */
      ਅੱਖਰ *zDel = 0;
      ਬਣਤਰ stat buf;

      ਅਣਵਰਤਿਆ_PARAMETER(pVfs);

      ਕਰੋ {
        /* Call lstat() on path zIn. Set bLink to true if the path
        ** is a symbolic link, or false otherwise.  */
        ਪੂਰਨ bLink = 0;
        ਜੇ( osLstat && osReadlink ) {
          ਜੇ( osLstat(z_ਵਿੱਚ, &buf)!=0 ){
            ਪੂਰਨ myErrno = osErrno;
            ਜੇ( myErrno!=ENOENT ){
              rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)myErrno,
                               "lstat", z_ਵਿੱਚ);
            }
          }ਨਹੀਂਤਾਂ{
            bLink = ((buf.st_ਬਹੁਲਕ & 0170000) == 0120000);
          }

          ਜੇ( bLink ){
            ਜੇ( zDel==0 ){
              zDel = sqlite3MallocZero(n_ਭਰਿਆ);
              ਜੇ( zDel==0 ) rc = SQLITE_NOMEM;
            }ਨਹੀਂਤਾਂ ਜੇ( ++nLink>SQLITE_MAX_SYMLINKS ){
              rc = SQLITE_CANTOPEN_BKPT;
            }

            ਜੇ( rc==SQLITE_OK ){
              nByte = osReadlink(z_ਵਿੱਚ, zDel, n_ਭਰਿਆ-1);
              ਜੇ( nByte ==(DWORD)-1 ){
                rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno,
                                 "readlink", z_ਵਿੱਚ);
              }ਨਹੀਂਤਾਂ{
                ਜੇ( zDel[0]!='/' ){
                  ਪੂਰਨ n;
                  ਲਈ(n = sqlite3Strlen30(z_ਵਿੱਚ); n>0 && z_ਵਿੱਚ[n-1]!='/'; n--);
                  ਜੇ( nByte+n+1>n_ਭਰਿਆ ){
                    rc = SQLITE_CANTOPEN_BKPT;
                  }ਨਹੀਂਤਾਂ{
                    memmove(&zDel[n], zDel, nByte+1);
                    ਸਮ੍ਰਿਤੀ_ਨਕਲ(zDel, z_ਵਿੱਚ, n);
                    nByte += n;
                  }
                }
                zDel[nByte] = '\0';
              }
            }

            z_ਵਿੱਚ = zDel;
          }
        }

        ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || z_ਵਿੱਚ!=z_ਭਰਿਆ || z_ਵਿੱਚ[0]=='/' );
        ਜੇ( rc==SQLITE_OK && z_ਵਿੱਚ!=z_ਭਰਿਆ ){
          rc = mk_ਭਰਿਆ_Pathname(z_ਵਿੱਚ, z_ਭਰਿਆ, n_ਭਰਿਆ);
        }
        ਜੇ( bLink==0 ) ਰੋਕੋ;
        z_ਵਿੱਚ = z_ਭਰਿਆ;
      }ਜਦੋਂ( rc==SQLITE_OK );

      sqlite3_free(zDel);
      winSimplifyName(z_ਭਰਿਆ);
      ਵਾਪਸ rc;
    }
  }
#ਜੇ_ਅੰਤ /* __CYGWIN__ */

#ਜੇ defined(_WIN32)
  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing. This function could fail if, for example, the
  ** current working directory has been unlinked.
  */
  SimulateIOError( ਵਾਪਸ SQLITE_ERROR );
  ਜੇ ( sqlite3_ਅੰਕੜੇ_directory && !winIsVerbatimPathname(zRelative) ){
    /*
    ** NOTE: We are dealing with a relative path name and the data
    **       directory has been set.  Therefore, use it as the basis
    **       for converting the relative path name to an absolute
    **       one by prepending the data directory and a backslash.
    */
    sqlite3_snprintf(MIN(n_ਭਰਿਆ, pVfs->mxPathname), z_ਭਰਿਆ, "%s%c%s",
                     sqlite3_ਅੰਕੜੇ_directory, winGetDirSep(), zRelative);
    ਵਾਪਸ SQLITE_OK;
  }
#ਜੇ_ਅੰਤ
  zConverted = winConvertFromUtf8Filename(zRelative);
  ਜੇ( zConverted==0 ){
    ਵਾਪਸ SQLITE_IOERR_NOMEM_BKPT;
  }
  {
    LPWSTR z_ਆਰਜ਼ੀ;
    nByte = os_Get_ਭਰਿਆ_ਰਸਤਾ_Name_W(zConverted, 0, 0, 0);
    ਜੇ( nByte==0 ){
      sqlite3_free(zConverted);
      ਵਾਪਸ winLogError(SQLITE_CANTOPEN_FULLPATH, os_Get_ਆਖਰੀ_Error(),
                         "winFullPathname1", zRelative);
    }
    nByte += 3;
    z_ਆਰਜ਼ੀ = sqlite3MallocZero( (u64)nByte*ਆਕਾਰ(WCHAR) );
    ਜੇ( z_ਆਰਜ਼ੀ==0 ){
      sqlite3_free(zConverted);
      ਵਾਪਸ SQLITE_IOERR_NOMEM_BKPT;
    }
    nByte = os_Get_ਭਰਿਆ_ਰਸਤਾ_Name_W(zConverted, nByte, z_ਆਰਜ਼ੀ, 0);
    ਜੇ( nByte==0 ){
      sqlite3_free(zConverted);
      sqlite3_free(z_ਆਰਜ਼ੀ);
      ਵਾਪਸ winLogError(SQLITE_CANTOPEN_FULLPATH, os_Get_ਆਖਰੀ_Error(),
                         "winFullPathname2", zRelative);
    }
    sqlite3_free(zConverted);
    z_ਬਾਹਰਲਾ = win_Unicode_ਮੰਜ਼ਿਲ_Utf8(z_ਆਰਜ਼ੀ);
    sqlite3_free(z_ਆਰਜ਼ੀ);
  }
  ਜੇ( z_ਬਾਹਰਲਾ ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __CYGWIN__
    ਜੇ( memcmp(z_ਬਾਹਰਲਾ, "\\\\?\\", 4) ){
      sqlite3_snprintf(MIN(n_ਭਰਿਆ, pVfs->mxPathname), z_ਭਰਿਆ, "%s", z_ਬਾਹਰਲਾ);
    }ਨਹੀਂਤਾਂ ਜੇ( memcmp(z_ਬਾਹਰਲਾ+4, "UNC\\", 4) ){
      sqlite3_snprintf(MIN(n_ਭਰਿਆ, pVfs->mxPathname), z_ਭਰਿਆ, "%s", z_ਬਾਹਰਲਾ+4);
    }ਨਹੀਂਤਾਂ{
      ਅੱਖਰ *p = z_ਬਾਹਰਲਾ+6;
      *p = '\\';
      ਜੇ( osGetcwd ){
        /* On Cygwin, UNC paths use forward slashes */
        ਜਦੋਂ( *p ){
          ਜੇ( *p=='\\' ) *p = '/';
          ++p;
        }
      }
      sqlite3_snprintf(MIN(n_ਭਰਿਆ, pVfs->mxPathname), z_ਭਰਿਆ, "%s", z_ਬਾਹਰਲਾ+6);
    }
#ਨਹੀਂਤਾਂ
    sqlite3_snprintf(MIN(n_ਭਰਿਆ, pVfs->mxPathname), z_ਭਰਿਆ, "%s", z_ਬਾਹਰਲਾ);
#ਜੇ_ਅੰਤ /* __CYGWIN__ */
    sqlite3_free(z_ਬਾਹਰਲਾ);
    ਵਾਪਸ SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ SQLITE_IOERR_NOMEM_BKPT;
  }
}
ਸਥਿਰ ਪੂਰਨ win_ਭਰਿਆ_Pathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ਅਚਲ ਅੱਖਰ *zRelative,        /* Possibly relative input path */
  ਪੂਰਨ n_ਭਰਿਆ,                    /* Size of output buffer in bytes */
  ਅੱਖਰ *z_ਭਰਿਆ                   /* Output buffer */
){
  ਪੂਰਨ rc;
  MUTEX_LOGIC( sqlite3_mutex *pMutex; )
  MUTEX_LOGIC( pMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR); )
  sqlite3_mutex_enter(pMutex);
  rc = win_ਭਰਿਆ_Pathname_No_Mutex(pVfs, zRelative, n_ਭਰਿਆ, z_ਭਰਿਆ);
  sqlite3_mutex_leave(pMutex);
  ਵਾਪਸ rc;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
ਸਥਿਰ ਖਾਲੀ *winDlOpen(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *zFilename){
  HANDLE h = 0;
  LPWSTR zConverted = winConvertFromUtf8Filename(zFilename);
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);
  ਜੇ( zConverted==0 ){
    OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (ਖਾਲੀ*)0));
    ਵਾਪਸ 0;
  }
  h = os_ਲੱਦੋ_Library_W ? os_ਲੱਦੋ_Library_W(zConverted) : 0;
  OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (ਖਾਲੀ*)h));
  sqlite3_free(zConverted);
  ਵਾਪਸ (ਖਾਲੀ*)h;
}
ਸਥਿਰ ਖਾਲੀ winDlError(sqlite3_vfs *pVfs, ਪੂਰਨ nBuf, ਅੱਖਰ *z_Buf_ਬਾਹਰਲਾ){
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);
  win_Get_ਆਖਰੀ_Error_ਸੁਨੇਹਾ(os_Get_ਆਖਰੀ_Error(), nBuf, z_Buf_ਬਾਹਰਲਾ);
}
ਸਥਿਰ ਖਾਲੀ (*winDlSym(sqlite3_vfs *pVfs,ਖਾਲੀ *pH,ਅਚਲ ਅੱਖਰ *zSym))(ਖਾਲੀ){
  FARPROC proc = 0;
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);
  proc = os_Get_Proc_ਪਤਾ_A ? os_Get_Proc_ਪਤਾ_A((HANDLE)pH, zSym) : 0;
  OSTRACE(("DLSYM handle=%p, symbol=%s, address=%p\n",
           (ਖਾਲੀ*)pH, zSym, (ਖਾਲੀ*)proc));
  ਵਾਪਸ (ਖਾਲੀ(*)(ਖਾਲੀ))proc;
}
ਸਥਿਰ ਖਾਲੀ winDlClose(sqlite3_vfs *pVfs, ਖਾਲੀ *pHandle){
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);
  ਜੇ( osFreeLibrary!=0 ) osFreeLibrary((HANDLE)pHandle);
  OSTRACE(("DLCLOSE handle=%p\n", (ਖਾਲੀ*)pHandle));
}
#ਨਹੀਂਤਾਂ /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ winDlOpen  0
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ winDlError 0
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ winDlSym   0
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ winDlClose 0
#ਜੇ_ਅੰਤ

/* State information for the randomness gatherer. */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ EntropyGatherer EntropyGatherer;
ਬਣਤਰ EntropyGatherer {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a;   /* Gather entropy into this buffer */
  ਪੂਰਨ na;             /* Size of a[] in bytes */
  ਪੂਰਨ i;              /* XOR next input into a[i] */
  ਪੂਰਨ nXor;           /* Number of XOR operations done */
};

#ਜੇ !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
/* Mix sz bytes of entropy into p. */
ਸਥਿਰ ਖਾਲੀ xor_ਸਮ੍ਰਿਤੀ(EntropyGatherer *p, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *x, ਪੂਰਨ sz){
  ਪੂਰਨ j, k;
  ਲਈ(j=0, k=p->i; j<sz; j++){
    p->a[k++] ^= x[j];
    ਜੇ( k>=p->na ) k = 0;
  }
  p->i = k;
  p->nXor += sz;
}
#ਜੇ_ਅੰਤ /* !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS) */

/*
** Write up to nBuf bytes of randomness into zBuf.
*/
ਸਥਿਰ ਪੂਰਨ winRandomness(sqlite3_vfs *pVfs, ਪੂਰਨ nBuf, ਅੱਖਰ *zBuf){
#ਜੇ defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS)
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  ਵਾਪਸ nBuf;
#ਨਹੀਂਤਾਂ
  EntropyGatherer e;
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  e.a = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)zBuf;
  e.na = nBuf;
  e.nXor = 0;
  e.i = 0;
  {
    FILETIME x;
    os_Get_System_ਸਮਾਂ_As_File_ਸਮਾਂ(&x);
    xor_ਸਮ੍ਰਿਤੀ(&e, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)&x, ਆਕਾਰ(x));
  }
  {
    DWORD pid = osGetCurrentProcessId();
    xor_ਸਮ੍ਰਿਤੀ(&e, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)&pid, ਆਕਾਰ(pid));
  }
  {
    ULONGLONG cnt = osGetTickCount64();
    xor_ਸਮ੍ਰਿਤੀ(&e, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)&cnt, ਆਕਾਰ(cnt));
  }
  {
    LARGE_INTEGER i;
    osQueryPerformanceCounter(&i);
    xor_ਸਮ੍ਰਿਤੀ(&e, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)&i, ਆਕਾਰ(i));
  }
#ਜੇ SQLITE_WIN32_USE_UUID
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UWP
# error SQLITE_WIN32_USE_UUID is incompatible with SQLITE_UWP
#ਜੇ_ਅੰਤ
  {
    UUID id;
    memset(&id, 0, ਆਕਾਰ(id));
    osUuidCreate(&id);
    xor_ਸਮ੍ਰਿਤੀ(&e, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)&id, ਆਕਾਰ(id));
    memset(&id, 0, ਆਕਾਰ(UUID));
    osUuidCreateSequential(&id);
    xor_ਸਮ੍ਰਿਤੀ(&e, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)&id, ਆਕਾਰ(id));
  }
#ਜੇ_ਅੰਤ /* SQLITE_WIN32_USE_UUID */
  ਵਾਪਸ e.nXor>nBuf ? nBuf : e.nXor;
#ਜੇ_ਅੰਤ /* defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS) */
}


/*
** Sleep for a little while.  Return the amount of time slept.
*/
ਸਥਿਰ ਪੂਰਨ winSleep(sqlite3_vfs *pVfs, ਪੂਰਨ microsec){
  osSleep((microsec+999)/1000);
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);
  ਵਾਪਸ ((microsec+999)/1000)*1000;
}

/*
** The following variable, if set to a non-zero value, is interpreted as
** the number of seconds since 1970 and is used to set the result of
** sqlite3OsCurrentTime() during testing.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
ਪੂਰਨ sqlite3_current_ਸਮਾਂ = 0;  /* Fake system time in seconds since 1970. */
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਪੂਰਨ win_Current_ਸਮਾਂ_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *piNow){
  /* FILETIME structure is a 64-bit value representing the number of
     100-nanosecond intervals since January 1, 1601 (= JD 2305813.5).
  */
  FILETIME ft;
  ਸਥਿਰ ਅਚਲ sqlite3_int64 winFiletimeEpoch = 23058135*(sqlite3_int64)8640000;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  ਸਥਿਰ ਅਚਲ sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
#ਜੇ_ਅੰਤ
  /* 2^32 - to avoid use of LL and warnings in gcc */
  ਸਥਿਰ ਅਚਲ sqlite3_int64 max32_ਦੋਆਧਾਰੀ_ਅੰਕ_ਮੁੱਲ =
      (sqlite3_int64)2000000000 + (sqlite3_int64)2000000000 +
      (sqlite3_int64)294967296;

  os_Get_System_ਸਮਾਂ_As_File_ਸਮਾਂ( &ft );
  *piNow = winFiletimeEpoch +
            ((((sqlite3_int64)ft.dw_ਉੱਚਾ_Date_ਸਮਾਂ)*max32_ਦੋਆਧਾਰੀ_ਅੰਕ_ਮੁੱਲ) +
               (sqlite3_int64)ft.dw_ਨੀਵਾਂ_Date_ਸਮਾਂ)/(sqlite3_int64)10000;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  ਜੇ( sqlite3_current_ਸਮਾਂ ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_ਸਮਾਂ + unixEpoch;
  }
#ਜੇ_ਅੰਤ
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
ਸਥਿਰ ਪੂਰਨ win_Current_ਸਮਾਂ(sqlite3_vfs *pVfs, ਦੁੱਗਣਾ *prNow){
  ਪੂਰਨ rc;
  sqlite3_int64 i;
  rc = win_Current_ਸਮਾਂ_Int64(pVfs, &i);
  ਜੇ( !rc ){
    *prNow = i/86400000.0;
  }
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ win_Get_ਆਖਰੀ_Error(sqlite3_vfs *pVfs, ਪੂਰਨ nBuf, ਅੱਖਰ *zBuf){
  DWORD e = os_Get_ਆਖਰੀ_Error();
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);
  ਜੇ( nBuf>0 ) win_Get_ਆਖਰੀ_Error_ਸੁਨੇਹਾ(e, nBuf, zBuf);
  ਵਾਪਸ e;
}

/*
** Initialize and deinitialize the operating system interface.
*/
ਪੂਰਨ sqlite3_os_init(ਖਾਲੀ){
  ਸਥਿਰ sqlite3_vfs winVfs = {
    3,                     /* iVersion */
    ਆਕਾਰ(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_ਰਸਤਾ_ਬਾਈਟਾਂ, /* mxPathname */
    0,                     /* pNext */
    "win32",               /* zName */
    &win_App_ਅੰਕੜੇ,           /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_ਭਰਿਆ_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_ਸਮਾਂ,        /* xCurrentTime */
    win_Get_ਆਖਰੀ_Error,       /* xGetLastError */
    win_Current_ਸਮਾਂ_Int64,   /* xCurrentTimeInt64 */
    win_ਸੈੱਟ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_ਅਗਲਾ_System_Call,     /* xNextSystemCall */
  };
  ਸਥਿਰ sqlite3_vfs win_Long_ਰਸਤਾ_Vfs = {
    3,                     /* iVersion */
    ਆਕਾਰ(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_ਰਸਤਾ_ਬਾਈਟਾਂ, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath",      /* zName */
    &win_App_ਅੰਕੜੇ,           /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_ਭਰਿਆ_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_ਸਮਾਂ,        /* xCurrentTime */
    win_Get_ਆਖਰੀ_Error,       /* xGetLastError */
    win_Current_ਸਮਾਂ_Int64,   /* xCurrentTimeInt64 */
    win_ਸੈੱਟ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_ਅਗਲਾ_System_Call,     /* xNextSystemCall */
  };
  ਸਥਿਰ sqlite3_vfs winNolockVfs = {
    3,                     /* iVersion */
    ਆਕਾਰ(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_ਰਸਤਾ_ਬਾਈਟਾਂ, /* mxPathname */
    0,                     /* pNext */
    "win32-none",          /* zName */
    &win_Nolock_App_ਅੰਕੜੇ,     /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_ਭਰਿਆ_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_ਸਮਾਂ,        /* xCurrentTime */
    win_Get_ਆਖਰੀ_Error,       /* xGetLastError */
    win_Current_ਸਮਾਂ_Int64,   /* xCurrentTimeInt64 */
    win_ਸੈੱਟ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_ਅਗਲਾ_System_Call,     /* xNextSystemCall */
  };
  ਸਥਿਰ sqlite3_vfs win_Long_ਰਸਤਾ_Nolock_Vfs = {
    3,                     /* iVersion */
    ਆਕਾਰ(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_ਰਸਤਾ_ਬਾਈਟਾਂ, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath-none", /* zName */
    &win_Nolock_App_ਅੰਕੜੇ,     /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_ਭਰਿਆ_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_ਸਮਾਂ,        /* xCurrentTime */
    win_Get_ਆਖਰੀ_Error,       /* xGetLastError */
    win_Current_ਸਮਾਂ_Int64,   /* xCurrentTimeInt64 */
    win_ਸੈੱਟ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_ਅਗਲਾ_System_Call,     /* xNextSystemCall */
  };

  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  ਪ੍ਰਤਿਜ੍ਞਾ( Array_ਮਾਪ(aSyscall)==60 );
  ਪ੍ਰਤਿਜ੍ਞਾ( strcmp(aSyscall[0].zName,"AreFileApisANSI")==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( strcmp(aSyscall[8].zName,"FreeLibrary")==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( strcmp(aSyscall[16].zName,"GetSystemInfo")==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( strcmp(aSyscall[24].zName,"HeapReAlloc")==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( strcmp(aSyscall[32].zName,"MapViewOfFileFromApp")==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( strcmp(aSyscall[40].zName,"UnmapViewOfFile")==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( strcmp(aSyscall[48].zName,"UuidCreate")==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( strcmp(aSyscall[56].zName,"lstat")==0 );

  /* get memory map allocation granularity */
  memset(&winSysInfo, 0, ਆਕਾਰ(SYSTEM_INFO));
  osGetSystemInfo(&winSysInfo);
  ਪ੍ਰਤਿਜ੍ਞਾ( winSysInfo.dw_ਵੰਡ_Granularity>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( winSysInfo.dw_ਪੰਨਾ_ਮਾਪ>0 );

  sqlite3_vfs_ਰਜਿਸਟਰ(&winVfs, 1);
  sqlite3_vfs_ਰਜਿਸਟਰ(&win_Long_ਰਸਤਾ_Vfs, 0);
  sqlite3_vfs_ਰਜਿਸਟਰ(&winNolockVfs, 0);
  sqlite3_vfs_ਰਜਿਸਟਰ(&win_Long_ਰਸਤਾ_Nolock_Vfs, 0);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  win_Big_ਤਾਲਾ = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#ਜੇ_ਅੰਤ

  ਵਾਪਸ SQLITE_OK;
}

ਪੂਰਨ sqlite3_os_ਸਿਰਾ(ਖਾਲੀ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  win_Big_ਤਾਲਾ = 0;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ_ਅੰਤ /* SQLITE_OS_WIN */
