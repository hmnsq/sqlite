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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಆದರೆ SQLITE_OS_WIN               /* This file is used for Windows only */

/*
** Include code that is common to all os_*.c files
*/
#ಅನ್ತರ್ಭಾವ "os_common.h"

/*
** Include the header file for the Windows VFS.
*/
#ಅನ್ತರ್ಭಾವ "os_win.h"

/*
** This constant should already be defined (in the "WinDef.h" SDK file).
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ MAX_ಮಾರ್ಗ
#  define MAX_ಮಾರ್ಗ                      (260)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Maximum pathname length (in chars) for Win32.  This should normally be
** MAX_PATH.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WIN32_MAX_ಮಾರ್ಗ_CHARS
#  define SQLITE_WIN32_MAX_ಮಾರ್ಗ_CHARS   (MAX_ಮಾರ್ಗ)
#ಆದರೆ_ಅಂತ್ಯ

/*
** This constant should already be defined (in the "WinNT.h" SDK file).
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ UNICODE_STRING_MAX_CHARS
#  define UNICODE_STRING_MAX_CHARS      (32767)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Maximum pathname length (in chars) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WINNT_MAX_ಮಾರ್ಗ_CHARS
#  define SQLITE_WINNT_MAX_ಮಾರ್ಗ_CHARS   (UNICODE_STRING_MAX_CHARS)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Maximum pathname length (in bytes) for Win32.  The MAX_PATH macro is in
** characters, so we allocate 4 bytes per character assuming worst-case of
** 4-bytes-per-character for UTF8.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WIN32_MAX_ಮಾರ್ಗ_ಬೈಟುಗಳು
#  define SQLITE_WIN32_MAX_ಮಾರ್ಗ_ಬೈಟುಗಳು   (SQLITE_WIN32_MAX_ಮಾರ್ಗ_CHARS*4)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Maximum pathname length (in bytes) for WinNT.  This should normally be
** UNICODE_STRING_MAX_CHARS * sizeof(WCHAR).
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WINNT_MAX_ಮಾರ್ಗ_ಬೈಟುಗಳು
#  define SQLITE_WINNT_MAX_ಮಾರ್ಗ_ಬೈಟುಗಳು   \
                            (ಗಾತ್ರ(WCHAR) * SQLITE_WINNT_MAX_ಮಾರ್ಗ_CHARS)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Maximum error message length (in chars) for WinRT.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WIN32_MAX_ERRMSG_CHARS
#  define SQLITE_WIN32_MAX_ERRMSG_CHARS (1024)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Returns non-zero if the character should be treated as a directory
** separator.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ winIsDirSep
#  define winIsDirSep(a)                (((a) == '/') || ((a) == '\\'))
#ಆದರೆ_ಅಂತ್ಯ

/*
** This macro is used when a local variable is set to a value that is
** [sometimes] not used by the code (e.g. via conditional compilation).
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ಬಳಸದ_VARIABLE_ಮೌಲ್ಯ
#  define ಬಳಸದ_VARIABLE_ಮೌಲ್ಯ(x)      (ಶೂನ್ಯ)(x)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Returns the character that should be used as the directory separator.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ winGetDirSep
#  define winGetDirSep()                '\\'
#ಆದರೆ_ಅಂತ್ಯ

/*
** Some older Microsoft compilers lack the following definition.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ INVALID_FILE_ಗುಣಲಕ್ಷಣಗಳು
# define INVALID_FILE_ಗುಣಲಕ್ಷಣಗಳು ((DWORD)-1)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FILE_FLAG_ಮುಖವಾಡ
# define FILE_FLAG_ಮುಖವಾಡ          (0xFF3C0000)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FILE_ATTRIBUTE_ಮುಖವಾಡ
# define FILE_ATTRIBUTE_ಮುಖವಾಡ     (0x0003FFF7)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
/* Forward references to structures used for WAL */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ winShm winShm;           /* A connection to shared-memory */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ win_Shm_ಗಂಟು win_Shm_ಗಂಟು;   /* A region of shared-memory */
#ಆದರೆ_ಅಂತ್ಯ

/*
** The winFile structure is a subclass of sqlite3_file* specific to the win32
** portability layer.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ winFile winFile;
ರಚನೆ winFile {
  ಸ್ಥಿರಾಂಕ sqlite3_io_methods *p_ವಿಧಾನ; /*** Must be first ***/
  sqlite3_vfs *pVfs;      /* The VFS used to open this file */
  HANDLE h;               /* Handle for accessing the file */
  u8 locktype;            /* Type of lock currently held on this file */
  ಗಿಡ್ಡ shared_ಬೀಗ_Byte;   /* Randomly chosen byte used as a shared lock */
  u8 ctrlFlags;           /* Flags.  See WINFILE_* below */
  DWORD ಕೊನೆಯ_Errno;        /* The Windows errno from the last I/O error */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  winShm *pShm;           /* Instance of shared memory on this file */
#ಆದರೆ_ಅಂತ್ಯ
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ;      /* Full pathname of this file */
  ಪೂರ್ಣಾಂಕ szChunk;            /* Chunk size configured by FCNTL_CHUNK_SIZE */
#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  ಪೂರ್ಣಾಂಕ n_Fetch_ಹೊರಹೋಗು;                /* Number of outstanding xFetch references */
  HANDLE hMap;                  /* Handle for accessing memory mapping */
  ಶೂನ್ಯ *pMapRegion;             /* Area memory mapped */
  sqlite3_int64 mmap_ಗಾತ್ರ;       /* Size of mapped region */
  sqlite3_int64 mmap_ಗಾತ್ರ_Max;    /* Configured FCNTL_MMAP_SIZE value */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
  DWORD iBusyTimeout;        /* Wait this many millisec on locks */
  ಪೂರ್ಣಾಂಕ b_ಖಂಡ_On_Connect;
#ಆದರೆ_ಅಂತ್ಯ
};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
# define winFileBusyTimeout(pDbFd) pDbFd->iBusyTimeout
#ಇಲ್ಲದಿದ್ದರೆ
# define winFileBusyTimeout(pDbFd) 0
#ಆದರೆ_ಅಂತ್ಯ

/*
** The winVfsAppData structure is used for the pAppData member for all of the
** Win32 VFS variants.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ win_Vfs_App_ದತ್ತಾಂಶ win_Vfs_App_ದತ್ತಾಂಶ;
ರಚನೆ win_Vfs_App_ದತ್ತಾಂಶ {
  ಸ್ಥಿರಾಂಕ sqlite3_io_methods *p_ವಿಧಾನ; /* The file I/O methods to use. */
  ಶೂನ್ಯ *p_App_ದತ್ತಾಂಶ;                    /* The extra pAppData, if any. */
  BOOL b_No_ಬೀಗ;                      /* Non-zero if locking is disabled. */
};

/*
** Allowed values for winFile.ctrlFlags
*/
#ವ್ಯಾಖ್ಯಾನಿಸು WINFILE_RDONLY          0x02   /* Connection is read only */
#ವ್ಯಾಖ್ಯಾನಿಸು WINFILE_PERSIST_WAL     0x04   /* Persistent WAL mode */
#ವ್ಯಾಖ್ಯಾನಿಸು WINFILE_PSOW            0x10   /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */

/*
 * The size of the buffer used by sqlite3_win32_write_debug().
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WIN32_DBG_BUF_ಗಾತ್ರ
#  define SQLITE_WIN32_DBG_BUF_ಗಾತ್ರ   ((ಪೂರ್ಣಾಂಕ)(4096-ಗಾತ್ರ(DWORD)))
#ಆದರೆ_ಅಂತ್ಯ

/*
 * If compiled with SQLITE_WIN32_MALLOC on Windows, we will use the
 * various Win32 API heap functions instead of our own.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_WIN32_MALLOC

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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WIN32_ರಾಶಿ_CREATE
#  define SQLITE_WIN32_ರಾಶಿ_CREATE        (TRUE)
#ಆದರೆ_ಅಂತ್ಯ

/*
 * This is the maximum possible initial size of the Win32-specific heap, in
 * bytes.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WIN32_ರಾಶಿ_MAX_INIT_ಗಾತ್ರ
#  define SQLITE_WIN32_ರಾಶಿ_MAX_INIT_ಗಾತ್ರ (4294967295U)
#ಆದರೆ_ಅಂತ್ಯ

/*
 * This is the extra space for the initial size of the Win32-specific heap,
 * in bytes.  This value may be zero.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WIN32_ರಾಶಿ_INIT_EXTRA
#  define SQLITE_WIN32_ರಾಶಿ_INIT_EXTRA  (4194304)
#ಆದರೆ_ಅಂತ್ಯ

/*
 * Calculate the maximum legal cache size, in pages, based on the maximum
 * possible initial heap size and the default page size, setting aside the
 * needed extra space.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WIN32_MAX_CACHE_ಗಾತ್ರ
#  define SQLITE_WIN32_MAX_CACHE_ಗಾತ್ರ   (((SQLITE_WIN32_ರಾಶಿ_MAX_INIT_ಗಾತ್ರ) - \
                                          (SQLITE_WIN32_ರಾಶಿ_INIT_EXTRA)) / \
                                         (SQLITE_DEFAULT_ಪುಟ_ಗಾತ್ರ))
#ಆದರೆ_ಅಂತ್ಯ

/*
 * This is cache size used in the calculation of the initial size of the
 * Win32-specific heap.  It cannot be negative.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WIN32_CACHE_ಗಾತ್ರ
#  ಆದರೆ SQLITE_DEFAULT_CACHE_ಗಾತ್ರ>=0
#    define SQLITE_WIN32_CACHE_ಗಾತ್ರ     (SQLITE_DEFAULT_CACHE_ಗಾತ್ರ)
#  ಇಲ್ಲದಿದ್ದರೆ
#    define SQLITE_WIN32_CACHE_ಗಾತ್ರ     (-(SQLITE_DEFAULT_CACHE_ಗಾತ್ರ))
#  endif
#ಆದರೆ_ಅಂತ್ಯ

/*
 * Make sure that the calculated cache size, in pages, cannot cause the
 * initial size of the Win32-specific heap to exceed the maximum amount
 * of memory that can be specified in the call to HeapCreate.
 */
#ಆದರೆ SQLITE_WIN32_CACHE_ಗಾತ್ರ>SQLITE_WIN32_MAX_CACHE_ಗಾತ್ರ
#  undef SQLITE_WIN32_CACHE_ಗಾತ್ರ
#  define SQLITE_WIN32_CACHE_ಗಾತ್ರ       (2000)
#ಆದರೆ_ಅಂತ್ಯ

/*
 * The initial size of the Win32-specific heap.  This value may be zero.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WIN32_ರಾಶಿ_INIT_ಗಾತ್ರ
#  define SQLITE_WIN32_ರಾಶಿ_INIT_ಗಾತ್ರ   ((SQLITE_WIN32_CACHE_ಗಾತ್ರ) * \
                                         (SQLITE_DEFAULT_ಪುಟ_ಗಾತ್ರ) + \
                                         (SQLITE_WIN32_ರಾಶಿ_INIT_EXTRA))
#ಆದರೆ_ಅಂತ್ಯ

/*
 * The maximum size of the Win32-specific heap.  This value may be zero.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WIN32_ರಾಶಿ_MAX_ಗಾತ್ರ
#  define SQLITE_WIN32_ರಾಶಿ_MAX_ಗಾತ್ರ    (0)
#ಆದರೆ_ಅಂತ್ಯ

/*
 * The extra flags to use in calls to the Win32 heap APIs.  This value may be
 * zero for the default behavior.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WIN32_ರಾಶಿ_FLAGS
#  define SQLITE_WIN32_ರಾಶಿ_FLAGS       (0)
#ಆದರೆ_ಅಂತ್ಯ


/*
** The winMemData structure stores information required by the Win32-specific
** sqlite3_mem_methods implementation.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ win_Mem_ದತ್ತಾಂಶ win_Mem_ದತ್ತಾಂಶ;
ರಚನೆ win_Mem_ದತ್ತಾಂಶ {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  u32 magic1;   /* Magic number to detect structure corruption. */
#ಆದರೆ_ಅಂತ್ಯ
  HANDLE h_ರಾಶಿ; /* The handle to our heap. */
  BOOL bOwned;  /* Do we own the heap (i.e. destroy it on shutdown)? */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  u32 magic2;   /* Magic number to detect structure corruption. */
#ಆದರೆ_ಅಂತ್ಯ
};

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
#ವ್ಯಾಖ್ಯಾನಿಸು WINMEM_MAGIC1     0x42b2830b
#ವ್ಯಾಖ್ಯಾನಿಸು WINMEM_MAGIC2     0xbd4d7cf4
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ರಚನೆ win_Mem_ದತ್ತಾಂಶ win_mem_ದತ್ತಾಂಶ = {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  WINMEM_MAGIC1,
#ಆದರೆ_ಅಂತ್ಯ
  ಖಾಲಿ, FALSE
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  ,WINMEM_MAGIC2
#ಆದರೆ_ಅಂತ್ಯ
};

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
#ವ್ಯಾಖ್ಯಾನಿಸು winMemAssertMagic1() ಪ್ರತಿಜ್ಞಾ( win_mem_ದತ್ತಾಂಶ.magic1==WINMEM_MAGIC1 )
#ವ್ಯಾಖ್ಯಾನಿಸು winMemAssertMagic2() ಪ್ರತಿಜ್ಞಾ( win_mem_ದತ್ತಾಂಶ.magic2==WINMEM_MAGIC2 )
#ವ್ಯಾಖ್ಯಾನಿಸು winMemAssertMagic()  winMemAssertMagic1(); winMemAssertMagic2();
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು winMemAssertMagic()
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು win_Mem_Get_ದತ್ತಾಂಶ_Ptr()  &win_mem_ದತ್ತಾಂಶ
#ವ್ಯಾಖ್ಯಾನಿಸು win_Mem_Get_ರಾಶಿ()     win_mem_ದತ್ತಾಂಶ.h_ರಾಶಿ
#ವ್ಯಾಖ್ಯಾನಿಸು winMemGetOwned()    win_mem_ದತ್ತಾಂಶ.bOwned

ಸ್ಥಿರ ಶೂನ್ಯ *winMemMalloc(ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು);
ಸ್ಥಿರ ಶೂನ್ಯ winMemFree(ಶೂನ್ಯ *p_ಪೂರ್ವ);
ಸ್ಥಿರ ಶೂನ್ಯ *winMemRealloc(ಶೂನ್ಯ *p_ಪೂರ್ವ, ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Mem_ಗಾತ್ರ(ಶೂನ್ಯ *p);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winMemRoundup(ಪೂರ್ಣಾಂಕ n);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winMemInit(ಶೂನ್ಯ *p_App_ದತ್ತಾಂಶ);
ಸ್ಥಿರ ಶೂನ್ಯ winMemShutdown(ಶೂನ್ಯ *p_App_ದತ್ತಾಂಶ);

ಸ್ಥಿರಾಂಕ sqlite3_mem_methods *sqlite3MemGetWin32(ಶೂನ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_WIN32_MALLOC */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SYSCALL
#  define SYSCALL sqlite3_syscall_ptr
#ಆದರೆ_ಅಂತ್ಯ

/*
** Many system calls are accessed through pointer-to-functions so that
** they may be overridden at runtime to facilitate fault injection during
** testing and sandboxing.  The following array holds the names and pointers
** to all overrideable system calls.
*/
ಸ್ಥಿರ ರಚನೆ win_syscall {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;            /* Name of the system call */
  sqlite3_syscall_ptr pCurrent; /* Current value of the system call */
  sqlite3_syscall_ptr pDefault; /* Default value */
} aSyscall[] = {
  { "AreFileApisANSI",         (SYSCALL)AreFileApisANSI,         0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osAreFileApisANSI ((BOOL(WINAPI*)(VOID))aSyscall[0].pCurrent)

  { "CloseHandle",             (SYSCALL)CloseHandle,             0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osCloseHandle ((BOOL(WINAPI*)(HANDLE))aSyscall[1].pCurrent)


  { "CreateFileW",             (SYSCALL)CreateFileW,             0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osCreateFileW ((HANDLE(WINAPI*)(LPCWSTR,DWORD,DWORD, \
        LPSECURITY_ಗುಣಲಕ್ಷಣಗಳು,DWORD,DWORD,HANDLE))aSyscall[2].pCurrent)

#ಆದರೆ defined(SQLITE_UWP)
  { "CreateFileMappingFromApp",(SYSCALL)CreateFileMappingFromApp,0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "CreateFileMappingFromApp",(SYSCALL)0,                       0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು osCreateFileMappingFromApp ((HANDLE(WINAPI*)(HANDLE,\
        PSECURITY_ಗುಣಲಕ್ಷಣಗಳು, \
        ULONG,ULONG64,PCWSTR))aSyscall[3].pCurrent)

#ಆದರೆ !defined(SQLITE_UWP)
  { "CreateFileMappingW",      (SYSCALL)CreateFileMappingW,      0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "CreateFileMappingW",      (SYSCALL)0,                       0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು osCreateFileMappingW ((HANDLE(WINAPI*)(HANDLE,LPSECURITY_ಗುಣಲಕ್ಷಣಗಳು, \
        DWORD,DWORD,DWORD,LPCWSTR))aSyscall[4].pCurrent)

  { "DeleteFileW",             (SYSCALL)DeleteFileW,             0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osDeleteFileW ((BOOL(WINAPI*)(LPCWSTR))aSyscall[5].pCurrent)

  { "FlushFileBuffers",        (SYSCALL)FlushFileBuffers,        0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osFlushFileBuffers ((BOOL(WINAPI*)(HANDLE))aSyscall[6].pCurrent)

  { "FormatMessageW",          (SYSCALL)Format_ಸಂದೇಶ_W,          0 },
#ವ್ಯಾಖ್ಯಾನಿಸು os_Format_ಸಂದೇಶ_W ((DWORD(WINAPI*)(DWORD,LPCVOID,DWORD,DWORD,LPWSTR, \
        DWORD,va_list*))aSyscall[7].pCurrent)

#ಆದರೆ !defined(SQLITE_OMIT_ತುಂಬು_EXTENSION) && !defined(SQLITE_UWP)
  { "FreeLibrary",             (SYSCALL)FreeLibrary,             0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "FreeLibrary",             (SYSCALL)0,                       0 },
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು osFreeLibrary ((BOOL(WINAPI*)(HMODULE))aSyscall[8].pCurrent)

  { "GetCurrentProcessId",     (SYSCALL)GetCurrentProcessId,     0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osGetCurrentProcessId ((DWORD(WINAPI*)(VOID))aSyscall[9].pCurrent)

  { "GetFileAttributesW",      (SYSCALL)Get_File_ಗುಣಲಕ್ಷಣಗಳು_W,      0 },
#ವ್ಯಾಖ್ಯಾನಿಸು os_Get_File_ಗುಣಲಕ್ಷಣಗಳು_W ((DWORD(WINAPI*)(LPCWSTR))aSyscall[10].pCurrent)

  { "GetFileAttributesExW",    (SYSCALL)Get_File_ಗುಣಲಕ್ಷಣಗಳು_Ex_W,    0 },
#ವ್ಯಾಖ್ಯಾನಿಸು os_Get_File_ಗುಣಲಕ್ಷಣಗಳು_Ex_W ((BOOL(WINAPI*)(LPCWSTR,GET_FILEEX_INFO_ಮಟ್ಟಗಳು, \
        LPVOID))aSyscall[11].pCurrent)

  { "GetFileSizeEx",           (SYSCALL)Get_File_ಗಾತ್ರ_Ex,           0 },
#ವ್ಯಾಖ್ಯಾನಿಸು os_Get_File_ಗಾತ್ರ_Ex ((BOOL(WINAPI*)(HANDLE, \
                        PLARGE_INTEGER))aSyscall[12].pCurrent)

  { "GetFullPathNameW",        (SYSCALL)Get_ತುಂಬಿದ_ಮಾರ್ಗ_Name_W,        0 },
#ವ್ಯಾಖ್ಯಾನಿಸು os_Get_ತುಂಬಿದ_ಮಾರ್ಗ_Name_W ((DWORD(WINAPI*)(LPCWSTR,DWORD,LPWSTR, \
        LPWSTR*))aSyscall[13].pCurrent)

  { "GetLastError",            (SYSCALL)Get_ಕೊನೆಯ_Error,            0 },
#ವ್ಯಾಖ್ಯಾನಿಸು os_Get_ಕೊನೆಯ_Error ((DWORD(WINAPI*)(VOID))aSyscall[14].pCurrent)

#ಆದರೆ !defined(SQLITE_OMIT_ತುಂಬು_EXTENSION) && !defined(SQLITE_UWP)
  { "GetProcAddressA",         (SYSCALL)Get_Proc_ವಿಳಾಸ,          0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "GetProcAddressA",         (SYSCALL)0,                       0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_Get_Proc_ವಿಳಾಸ_A ((FARPROC(WINAPI*)(HMODULE, \
        LPCSTR))aSyscall[15].pCurrent)

  { "GetSystemInfo",           (SYSCALL)GetSystemInfo,           0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osGetSystemInfo ((VOID(WINAPI*)(LPSYSTEM_INFO))aSyscall[16].pCurrent)

  { "GetSystemTimeAsFileTime", (SYSCALL)Get_System_ಸಮಯ_As_File_ಸಮಯ, 0 },
#ವ್ಯಾಖ್ಯಾನಿಸು os_Get_System_ಸಮಯ_As_File_ಸಮಯ ((VOID(WINAPI*)( \
        LPFILETIME))aSyscall[17].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
  { "GetTempPathW",            (SYSCALL)0,                       0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "GetTempPathW",            (SYSCALL)Get_ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ_W,            0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_Get_ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ_W ((DWORD(WINAPI*)(DWORD,LPWSTR))aSyscall[18].pCurrent)

  { "GetTickCount64",          (SYSCALL)GetTickCount64,          0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osGetTickCount64 ((ULONGLONG(WINAPI*)(VOID))aSyscall[19].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
  { "HeapAlloc",               (SYSCALL)0,                       0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "HeapAlloc",               (SYSCALL)ರಾಶಿ_Alloc,               0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_ರಾಶಿ_Alloc ((LPVOID(WINAPI*)(HANDLE,DWORD, \
        ಗಾತ್ರ_T))aSyscall[20].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
  { "HeapCreate",              (SYSCALL)0,                       0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "HeapCreate",              (SYSCALL)ರಾಶಿ_Create,              0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_ರಾಶಿ_Create ((HANDLE(WINAPI*)(DWORD,ಗಾತ್ರ_T, \
        ಗಾತ್ರ_T))aSyscall[21].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
  { "HeapDestroy",             (SYSCALL)0,                       0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "HeapDestroy",             (SYSCALL)ರಾಶಿ_Destroy,             0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_ರಾಶಿ_Destroy ((BOOL(WINAPI*)(HANDLE))aSyscall[22].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
  { "HeapFree",                (SYSCALL)0,                       0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "HeapFree",                (SYSCALL)ರಾಶಿ_Free,                0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_ರಾಶಿ_Free ((BOOL(WINAPI*)(HANDLE,DWORD,LPVOID))aSyscall[23].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
  { "HeapReAlloc",             (SYSCALL)0,                       0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "HeapReAlloc",             (SYSCALL)ರಾಶಿ_Re_Alloc,             0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_ರಾಶಿ_Re_Alloc ((LPVOID(WINAPI*)(HANDLE,DWORD,LPVOID, \
        ಗಾತ್ರ_T))aSyscall[24].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
  { "HeapSize",                (SYSCALL)0,                       0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "HeapSize",                (SYSCALL)ರಾಶಿ_ಗಾತ್ರ,                0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_ರಾಶಿ_ಗಾತ್ರ ((ಗಾತ್ರ_T(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[25].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
  { "HeapValidate",            (SYSCALL)0,                       0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "HeapValidate",            (SYSCALL)ರಾಶಿ_Validate,            0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_ರಾಶಿ_Validate ((BOOL(WINAPI*)(HANDLE,DWORD, \
        LPCVOID))aSyscall[26].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
  { "HeapCompact",             (SYSCALL)0,                       0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "HeapCompact",             (SYSCALL)ರಾಶಿ_Compact,             0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_ರಾಶಿ_Compact ((UINT(WINAPI*)(HANDLE,DWORD))aSyscall[27].pCurrent)


#ಆದರೆ !defined(SQLITE_OMIT_ತುಂಬು_EXTENSION) && !defined(SQLITE_UWP)
  { "LoadLibraryW",            (SYSCALL)ತುಂಬು_Library_W,            0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "LoadLibraryW",            (SYSCALL)0,                       0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_ತುಂಬು_Library_W ((HMODULE(WINAPI*)(LPCWSTR))aSyscall[28].pCurrent)

  { "LocalFree",               (SYSCALL)LocalFree,               0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osLocalFree ((HLOCAL(WINAPI*)(HLOCAL))aSyscall[29].pCurrent)

  { "LockFileEx",              (SYSCALL)ಬೀಗ_File_Ex,              0 },
#ವ್ಯಾಖ್ಯಾನಿಸು os_ಬೀಗ_File_Ex ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[30].pCurrent)

#ಆದರೆ (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_ಗಾತ್ರ>0) \
 && !defined(SQLITE_UWP)
  { "MapViewOfFile",           (SYSCALL)Map_View_ದ_File,           0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "MapViewOfFile",           (SYSCALL)0,                       0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_Map_View_ದ_File ((LPVOID(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        ಗಾತ್ರ_T))aSyscall[31].pCurrent)

#ಆದರೆ (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_ಗಾತ್ರ>0) \
 && defined(SQLITE_UWP)
  { "MapViewOfFileFromApp",    (SYSCALL)Map_View_ದ_File_From_App,   0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "MapViewOfFileFromApp",    (SYSCALL)0,                      0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_Map_View_ದ_File_From_App ((LPVOID(WINAPI*)(HANDLE,ULONG,ULONG64, \
        ಗಾತ್ರ_T))aSyscall[32].pCurrent)

  { "MultiByteToWideChar",     (SYSCALL)Multi_Byte_ಗಮ್ಯ_Wide_Char,     0 },
#ವ್ಯಾಖ್ಯಾನಿಸು os_Multi_Byte_ಗಮ್ಯ_Wide_Char ((ಪೂರ್ಣಾಂಕ(WINAPI*)(UINT,DWORD,LPCSTR,ಪೂರ್ಣಾಂಕ,LPWSTR, \
        ಪೂರ್ಣಾಂಕ))aSyscall[33].pCurrent)

  { "QueryPerformanceCounter", (SYSCALL)QueryPerformanceCounter, 0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osQueryPerformanceCounter ((BOOL(WINAPI*)( \
        LARGE_INTEGER*))aSyscall[34].pCurrent)

  { "ReadFile",                (SYSCALL)ReadFile,                0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osReadFile ((BOOL(WINAPI*)(HANDLE,LPVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[35].pCurrent)

  { "SetEndOfFile",            (SYSCALL)ಗಣ_ಕೊನೆ_ದ_File,            0 },
#ವ್ಯಾಖ್ಯಾನಿಸು os_ಗಣ_ಕೊನೆ_ದ_File ((BOOL(WINAPI*)(HANDLE))aSyscall[36].pCurrent)

  { "SetFilePointerEx",        (SYSCALL)ಗಣ_File_ಸೂಚಕ_Ex,        0 },
#ವ್ಯಾಖ್ಯಾನಿಸು os_ಗಣ_File_ಸೂಚಕ_Ex ((BOOL(WINAPI*)(HANDLE,LARGE_INTEGER,\
        PLARGE_INTEGER,DWORD))aSyscall[37].pCurrent)

  { "Sleep",                   (SYSCALL)Sleep,                   0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osSleep ((VOID(WINAPI*)(DWORD))aSyscall[38].pCurrent)

  { "UnlockFileEx",            (SYSCALL)UnlockFileEx,            0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osUnlockFileEx ((BOOL(WINAPI*)(HANDLE,DWORD,DWORD,DWORD, \
        LPOVERLAPPED))aSyscall[39].pCurrent)

#ಆದರೆ !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  { "UnmapViewOfFile",         (SYSCALL)Unmap_View_ದ_File,         0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "UnmapViewOfFile",         (SYSCALL)0,                       0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_Unmap_View_ದ_File ((BOOL(WINAPI*)(LPCVOID))aSyscall[40].pCurrent)

  { "WideCharToMultiByte",     (SYSCALL)Wide_Char_ಗಮ್ಯ_Multi_Byte,     0 },
#ವ್ಯಾಖ್ಯಾನಿಸು os_Wide_Char_ಗಮ್ಯ_Multi_Byte ((ಪೂರ್ಣಾಂಕ(WINAPI*)(UINT,DWORD,LPCWSTR,ಪೂರ್ಣಾಂಕ,LPSTR,ಪೂರ್ಣಾಂಕ, \
        LPCSTR,LPBOOL))aSyscall[41].pCurrent)

  { "WriteFile",               (SYSCALL)WriteFile,               0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osWriteFile ((BOOL(WINAPI*)(HANDLE,LPCVOID,DWORD,LPDWORD, \
        LPOVERLAPPED))aSyscall[42].pCurrent)

  { "WaitForSingleObject",     (SYSCALL)WaitForSingleObject,     0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osWaitForSingleObject ((DWORD(WINAPI*)(HANDLE, \
        DWORD))aSyscall[43].pCurrent)

  { "WaitForSingleObjectEx",   (SYSCALL)WaitForSingleObjectEx,   0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osWaitForSingleObjectEx ((DWORD(WINAPI*)(HANDLE,DWORD, \
        BOOL))aSyscall[44].pCurrent)

  { "OutputDebugStringA",      (SYSCALL)OutputDebugStringA,      0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osOutputDebugStringA ((VOID(WINAPI*)(LPCSTR))aSyscall[45].pCurrent)

  { "GetProcessHeap",          (SYSCALL)Get_Process_ರಾಶಿ,          0 },
#ವ್ಯಾಖ್ಯಾನಿಸು os_Get_Process_ರಾಶಿ ((HANDLE(WINAPI*)(VOID))aSyscall[46].pCurrent)

/*
** NOTE: On some sub-platforms, the InterlockedCompareExchange "function"
**       is really just a macro that uses a compiler intrinsic (e.g. x64).
**       So do not try to make this is into a redefinable interface.
*/
#ಆದರೆ defined(InterlockedCompareExchange)
  { "InterlockedCompareExchange", (SYSCALL)0,                    0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osInterlockedCompareExchange InterlockedCompareExchange
#ಇಲ್ಲದಿದ್ದರೆ
  { "InterlockedCompareExchange", (SYSCALL)InterlockedCompareExchange, 0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osInterlockedCompareExchange ((LONG(WINAPI*)(LONG ಅಸ್ಥಿರ*,\
        LONG,LONG))aSyscall[47].pCurrent)
#ಆದರೆ_ಅಂತ್ಯ /* defined(InterlockedCompareExchange) */

#ಆದರೆ SQLITE_WIN32_USE_UUID
  { "UuidCreate",               (SYSCALL)UuidCreate,             0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "UuidCreate",               (SYSCALL)0,                      0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು osUuidCreate ((RPC_ಸ್ಥಿತಿ(RPC_ನಮೂದು*)(UUID*))aSyscall[48].pCurrent)

#ಆದರೆ SQLITE_WIN32_USE_UUID
  { "UuidCreateSequential",     (SYSCALL)UuidCreateSequential,   0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "UuidCreateSequential",     (SYSCALL)0,                      0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು osUuidCreateSequential \
        ((RPC_ಸ್ಥಿತಿ(RPC_ನಮೂದು*)(UUID*))aSyscall[49].pCurrent)

#ಆದರೆ !defined(SQLITE_NO_SYNC) && SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  { "FlushViewOfFile",          (SYSCALL)Flush_View_ದ_File,        0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "FlushViewOfFile",          (SYSCALL)0,                      0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_Flush_View_ದ_File \
        ((BOOL(WINAPI*)(LPCVOID,ಗಾತ್ರ_T))aSyscall[50].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
  { "CreateEvent",              (SYSCALL)CreateEvent,            0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "CreateEvent",              (SYSCALL)0,                      0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು osCreateEvent ((HANDLE(WINAPI*)(LPSECURITY_ಗುಣಲಕ್ಷಣಗಳು,BOOL, \
                        BOOL,LPCSTR))aSyscall[51].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
  { "CancelIo",                 (SYSCALL)CancelIo,               0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "CancelIo",                 (SYSCALL)0,                      0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು osCancelIo ((BOOL(WINAPI*)(HANDLE))aSyscall[52].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _WIN32
  { "getenv",                   (SYSCALL)getenv,                 0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "getenv",                   (SYSCALL)0,                      0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು osGetenv ((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *(*)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *))aSyscall[53].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _WIN32
  { "getcwd",                   (SYSCALL)getcwd,                 0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "getcwd",                   (SYSCALL)0,                      0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು osGetcwd ((ಅಕ್ಷರ*(*)(ಅಕ್ಷರ*,size_t))aSyscall[54].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _WIN32
  { "readlink",                 (SYSCALL)readlink,               0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "readlink",                 (SYSCALL)0,                      0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು osReadlink ((ssize_t(*)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಅಕ್ಷರ*,size_t))aSyscall[55].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _WIN32
  { "lstat",                    (SYSCALL)lstat,                  0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "lstat",                    (SYSCALL)0,                      0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು osLstat ((ಪೂರ್ಣಾಂಕ(*)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ರಚನೆ stat*))aSyscall[56].pCurrent)

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _WIN32
  { "__errno",                  (SYSCALL)__errno,                0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "__errno",                  (SYSCALL)0,                      0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು osErrno (*((ಪೂರ್ಣಾಂಕ*(*)(ಶೂನ್ಯ))aSyscall[57].pCurrent)())

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _WIN32
  { "cygwin_conv_path",         (SYSCALL)cygwin_conv_ಮಾರ್ಗ,       0 },
#ಇಲ್ಲದಿದ್ದರೆ
  { "cygwin_conv_path",         (SYSCALL)0,                      0 },
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು os_Cygwin_conv_ಮಾರ್ಗ ((size_t(*)(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ, \
    ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *, ಶೂನ್ಯ *, size_t))aSyscall[58].pCurrent)

  { "GetDriveTypeW",            (SYSCALL)GetDriveTypeW,          0 },
#ವ್ಯಾಖ್ಯಾನಿಸು osGetDriveTypeW ((UINT(WINAPI*)(LPCWSTR))aSyscall[59].pCurrent)

}; /* End of the overrideable system calls */

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "win32" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_ಗಣ_System_Call(
  sqlite3_vfs *p_Not_ಬಳಸಿದ,        /* The VFS pointer.  Not used */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,            /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_NOTFOUND;

  ಬಳಸದ_PARAMETER(p_Not_ಬಳಸಿದ);
  ಆದರೆ( zName==0 ){
    /* If no zName is given, restore all system calls to their default
    ** settings and return NULL
    */
    rc = SQLITE_OK;
    ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aSyscall)/ಗಾತ್ರ(aSyscall[0]); i++){
      ಆದರೆ( aSyscall[i].pDefault ){
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* If zName is specified, operate on only the one system call
    ** specified.
    */
    ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aSyscall)/ಗಾತ್ರ(aSyscall[0]); i++){
      ಆದರೆ( strcmp(zName, aSyscall[i].zName)==0 ){
        ಆದರೆ( aSyscall[i].pDefault==0 ){
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = SQLITE_OK;
        ಆದರೆ( pNewFunc==0 ) pNewFunc = aSyscall[i].pDefault;
        aSyscall[i].pCurrent = pNewFunc;
        ನಿಲ್ಲಿಸು;
      }
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
ಸ್ಥಿರ sqlite3_syscall_ptr winGetSystemCall(
  sqlite3_vfs *p_Not_ಬಳಸಿದ,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName
){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

  ಬಳಸದ_PARAMETER(p_Not_ಬಳಸಿದ);
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aSyscall)/ಗಾತ್ರ(aSyscall[0]); i++){
    ಆದರೆ( strcmp(zName, aSyscall[i].zName)==0 ) ಹಿಂತಿರುಗಿ aSyscall[i].pCurrent;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *win_ಮುಂದಿನ_System_Call(sqlite3_vfs *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName){
  ಪೂರ್ಣಾಂಕ i = -1;

  ಬಳಸದ_PARAMETER(p);
  ಆದರೆ( zName ){
    ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(aSyscall)-1; i++){
      ಆದರೆ( strcmp(zName, aSyscall[i].zName)==0 ) ನಿಲ್ಲಿಸು;
    }
  }
  ಗಾಗಿ(i++; i<Array_ಗಾತ್ರ(aSyscall); i++){
    ಆದರೆ( aSyscall[i].pCurrent!=0 ) ಹಿಂತಿರುಗಿ aSyscall[i].zName;
  }
  ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_WIN32_MALLOC
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
# error SQLITE_WIN32_MALLOC is incompatible with SQLITE_UWP
#ಆದರೆ_ಅಂತ್ಯ
/*
** If a Win32 native heap has been configured, this function will attempt to
** compact it.  Upon success, SQLITE_OK will be returned.  Upon failure, one
** of SQLITE_NOMEM, SQLITE_ERROR, or SQLITE_NOTFOUND will be returned.  The
** "pnLargest" argument, if non-zero, will be used to return the size of the
** largest committed free block in the heap, in bytes.
*/
ಪೂರ್ಣಾಂಕ sqlite3_win32_compact_ರಾಶಿ(LPUINT pn_ಅತಿದೊಡ್ಡ){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  UINT n_ಅತಿದೊಡ್ಡ = 0;
  HANDLE h_ರಾಶಿ;

  winMemAssertMagic();
  h_ರಾಶಿ = win_Mem_Get_ರಾಶಿ();
  ಪ್ರತಿಜ್ಞಾ( h_ರಾಶಿ!=0 );
  ಪ್ರತಿಜ್ಞಾ( h_ರಾಶಿ!=INVALID_HANDLE_ಮೌಲ್ಯ );
#ಆದರೆ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ಪ್ರತಿಜ್ಞಾ( os_ರಾಶಿ_Validate(h_ರಾಶಿ, SQLITE_WIN32_ರಾಶಿ_FLAGS, ಖಾಲಿ) );
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( (n_ಅತಿದೊಡ್ಡ=os_ರಾಶಿ_Compact(h_ರಾಶಿ, SQLITE_WIN32_ರಾಶಿ_FLAGS))==0 ){
    DWORD ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
    ಆದರೆ( ಕೊನೆಯ_Errno==NO_ERROR ){
      sqlite3_log(SQLITE_NOMEM, "failed to HeapCompact (no space), heap=%p",
                  (ಶೂನ್ಯ*)h_ರಾಶಿ);
      rc = SQLITE_NOMEM_BKPT;
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_log(SQLITE_ERROR, "failed to HeapCompact (%lu), heap=%p",
                  os_Get_ಕೊನೆಯ_Error(), (ಶೂನ್ಯ*)h_ರಾಶಿ);
      rc = SQLITE_ERROR;
    }
  }
  ಆದರೆ( pn_ಅತಿದೊಡ್ಡ ) *pn_ಅತಿದೊಡ್ಡ = n_ಅತಿದೊಡ್ಡ;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** If a Win32 native heap has been configured, this function will attempt to
** destroy and recreate it.  If the Win32 native heap is not isolated and/or
** the sqlite3_memory_used() function does not return zero, SQLITE_BUSY will
** be returned and no changes will be made to the Win32 native heap.
*/
ಪೂರ್ಣಾಂಕ sqlite3_win32_reset_ರಾಶಿ(){
  ಪೂರ್ಣಾಂಕ rc;
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; ) /* The main static mutex */
  MUTEX_LOGIC( sqlite3_mutex *pMem; )    /* The memsys static mutex */
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  MUTEX_LOGIC( pMem = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MEM); )
  sqlite3_mutex_enter(pMainMtx);
  sqlite3_mutex_enter(pMem);
  winMemAssertMagic();
  ಆದರೆ( win_Mem_Get_ರಾಶಿ()!=ಖಾಲಿ && winMemGetOwned() && sqlite3_ಸ್ಮರಣೆ_ಬಳಸಿದ()==0 ){
    /*
    ** At this point, there should be no outstanding memory allocations on
    ** the heap.  Also, since both the main and memsys locks are currently
    ** being held by us, no other function (i.e. from another thread) should
    ** be able to even access the heap.  Attempt to destroy and recreate our
    ** isolated Win32 native heap now.
    */
    ಪ್ರತಿಜ್ಞಾ( win_Mem_Get_ರಾಶಿ()!=ಖಾಲಿ );
    ಪ್ರತಿಜ್ಞಾ( winMemGetOwned() );
    ಪ್ರತಿಜ್ಞಾ( sqlite3_ಸ್ಮರಣೆ_ಬಳಸಿದ()==0 );
    winMemShutdown(win_Mem_Get_ದತ್ತಾಂಶ_Ptr());
    ಪ್ರತಿಜ್ಞಾ( win_Mem_Get_ರಾಶಿ()==ಖಾಲಿ );
    ಪ್ರತಿಜ್ಞಾ( !winMemGetOwned() );
    ಪ್ರತಿಜ್ಞಾ( sqlite3_ಸ್ಮರಣೆ_ಬಳಸಿದ()==0 );
    rc = winMemInit(win_Mem_Get_ದತ್ತಾಂಶ_Ptr());
    ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || win_Mem_Get_ರಾಶಿ()!=ಖಾಲಿ );
    ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || winMemGetOwned() );
    ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || sqlite3_ಸ್ಮರಣೆ_ಬಳಸಿದ()==0 );
  }ಇಲ್ಲದಿದ್ದರೆ{
    /*
    ** The Win32 native heap cannot be modified because it may be in use.
    */
    rc = SQLITE_BUSY;
  }
  sqlite3_mutex_leave(pMem);
  sqlite3_mutex_leave(pMainMtx);
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_WIN32_MALLOC */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
/*
** This function outputs the specified (ANSI) string to the Win32 debugger
** (if available).  Undocumented.  Might go away at any moment.
*/
ಶೂನ್ಯ sqlite3_win32_write_debug(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zBuf, ಪೂರ್ಣಾಂಕ nBuf){
  ಅಕ್ಷರ zDbgBuf[SQLITE_WIN32_DBG_BUF_ಗಾತ್ರ];
  ಪೂರ್ಣಾಂಕ nMin = MIN(nBuf, (SQLITE_WIN32_DBG_BUF_ಗಾತ್ರ - 1)); /* may be negative. */
  ಆದರೆ( nMin<-1 ) nMin = -1; /* all negative values become -1. */
  ಪ್ರತಿಜ್ಞಾ( nMin==-1 || nMin==0 || nMin<SQLITE_WIN32_DBG_BUF_ಗಾತ್ರ );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !zBuf ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( nMin>0 ){
    memset(zDbgBuf, 0, SQLITE_WIN32_DBG_BUF_ಗಾತ್ರ);
    ಸ್ಮರಣೆ_ನಕಲು(zDbgBuf, zBuf, nMin);
    osOutputDebugStringA(zDbgBuf);
  }ಇಲ್ಲದಿದ್ದರೆ{
    osOutputDebugStringA(zBuf);
  }
}

ಶೂನ್ಯ sqlite3_win32_sleep(DWORD milliseconds){
  osSleep(milliseconds);
}

#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0 && SQLITE_THREADSAFE>0
DWORD sqlite3Win32Wait(HANDLE hObject){
  DWORD rc;
  ಆಗ( (rc = osWaitForSingleObjectEx(hObject, INFINITE,
                                       TRUE))==WAIT_IO_COMPLETION ){}
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* _WIN32 */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_WIN32_MALLOC
/*
** Allocate nBytes of memory.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *winMemMalloc(ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು){
  HANDLE h_ರಾಶಿ;
  ಶೂನ್ಯ *p;

  winMemAssertMagic();
  h_ರಾಶಿ = win_Mem_Get_ರಾಶಿ();
  ಪ್ರತಿಜ್ಞಾ( h_ರಾಶಿ!=0 );
  ಪ್ರತಿಜ್ಞಾ( h_ರಾಶಿ!=INVALID_HANDLE_ಮೌಲ್ಯ );
#ಆದರೆ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ಪ್ರತಿಜ್ಞಾ( os_ರಾಶಿ_Validate(h_ರಾಶಿ, SQLITE_WIN32_ರಾಶಿ_FLAGS, ಖಾಲಿ) );
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( n_ಬೈಟುಗಳು>=0 );
  p = os_ರಾಶಿ_Alloc(h_ರಾಶಿ, SQLITE_WIN32_ರಾಶಿ_FLAGS, (ಗಾತ್ರ_T)n_ಬೈಟುಗಳು);
  ಆದರೆ( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapAlloc %u bytes (%lu), heap=%p",
                n_ಬೈಟುಗಳು, os_Get_ಕೊನೆಯ_Error(), (ಶೂನ್ಯ*)h_ರಾಶಿ);
  }
  ಹಿಂತಿರುಗಿ p;
}

/*
** Free memory.
*/
ಸ್ಥಿರ ಶೂನ್ಯ winMemFree(ಶೂನ್ಯ *p_ಪೂರ್ವ){
  HANDLE h_ರಾಶಿ;

  winMemAssertMagic();
  h_ರಾಶಿ = win_Mem_Get_ರಾಶಿ();
  ಪ್ರತಿಜ್ಞಾ( h_ರಾಶಿ!=0 );
  ಪ್ರತಿಜ್ಞಾ( h_ರಾಶಿ!=INVALID_HANDLE_ಮೌಲ್ಯ );
#ಆದರೆ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ಪ್ರತಿಜ್ಞಾ( os_ರಾಶಿ_Validate(h_ರಾಶಿ, SQLITE_WIN32_ರಾಶಿ_FLAGS, p_ಪೂರ್ವ) );
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( !p_ಪೂರ್ವ ) ಹಿಂತಿರುಗಿ; /* Passing NULL to HeapFree is undefined. */
  ಆದರೆ( !os_ರಾಶಿ_Free(h_ರಾಶಿ, SQLITE_WIN32_ರಾಶಿ_FLAGS, p_ಪೂರ್ವ) ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapFree block %p (%lu), heap=%p",
                p_ಪೂರ್ವ, os_Get_ಕೊನೆಯ_Error(), (ಶೂನ್ಯ*)h_ರಾಶಿ);
  }
}

/*
** Change the size of an existing memory allocation
*/
ಸ್ಥಿರ ಶೂನ್ಯ *winMemRealloc(ಶೂನ್ಯ *p_ಪೂರ್ವ, ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು){
  HANDLE h_ರಾಶಿ;
  ಶೂನ್ಯ *p;

  winMemAssertMagic();
  h_ರಾಶಿ = win_Mem_Get_ರಾಶಿ();
  ಪ್ರತಿಜ್ಞಾ( h_ರಾಶಿ!=0 );
  ಪ್ರತಿಜ್ಞಾ( h_ರಾಶಿ!=INVALID_HANDLE_ಮೌಲ್ಯ );
#ಆದರೆ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ಪ್ರತಿಜ್ಞಾ( os_ರಾಶಿ_Validate(h_ರಾಶಿ, SQLITE_WIN32_ರಾಶಿ_FLAGS, p_ಪೂರ್ವ) );
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( n_ಬೈಟುಗಳು>=0 );
  ಆದರೆ( !p_ಪೂರ್ವ ){
    p = os_ರಾಶಿ_Alloc(h_ರಾಶಿ, SQLITE_WIN32_ರಾಶಿ_FLAGS, (ಗಾತ್ರ_T)n_ಬೈಟುಗಳು);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p = os_ರಾಶಿ_Re_Alloc(h_ರಾಶಿ, SQLITE_WIN32_ರಾಶಿ_FLAGS, p_ಪೂರ್ವ, (ಗಾತ್ರ_T)n_ಬೈಟುಗಳು);
  }
  ಆದರೆ( !p ){
    sqlite3_log(SQLITE_NOMEM, "failed to %s %u bytes (%lu), heap=%p",
                p_ಪೂರ್ವ ? "HeapReAlloc" : "HeapAlloc", n_ಬೈಟುಗಳು, os_Get_ಕೊನೆಯ_Error(),
                (ಶೂನ್ಯ*)h_ರಾಶಿ);
  }
  ಹಿಂತಿರುಗಿ p;
}

/*
** Return the size of an outstanding allocation, in bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Mem_ಗಾತ್ರ(ಶೂನ್ಯ *p){
  HANDLE h_ರಾಶಿ;
  ಗಾತ್ರ_T n;

  winMemAssertMagic();
  h_ರಾಶಿ = win_Mem_Get_ರಾಶಿ();
  ಪ್ರತಿಜ್ಞಾ( h_ರಾಶಿ!=0 );
  ಪ್ರತಿಜ್ಞಾ( h_ರಾಶಿ!=INVALID_HANDLE_ಮೌಲ್ಯ );
#ಆದರೆ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ಪ್ರತಿಜ್ಞಾ( os_ರಾಶಿ_Validate(h_ರಾಶಿ, SQLITE_WIN32_ರಾಶಿ_FLAGS, p) );
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( !p ) ಹಿಂತಿರುಗಿ 0;
  n = os_ರಾಶಿ_ಗಾತ್ರ(h_ರಾಶಿ, SQLITE_WIN32_ರಾಶಿ_FLAGS, p);
  ಆದರೆ( n==(ಗಾತ್ರ_T)-1 ){
    sqlite3_log(SQLITE_NOMEM, "failed to HeapSize block %p (%lu), heap=%p",
                p, os_Get_ಕೊನೆಯ_Error(), (ಶೂನ್ಯ*)h_ರಾಶಿ);
    ಹಿಂತಿರುಗಿ 0;
  }
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)n;
}

/*
** Round up a request size to the next valid allocation size.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winMemRoundup(ಪೂರ್ಣಾಂಕ n){
  ಹಿಂತಿರುಗಿ n;
}

/*
** Initialize this module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winMemInit(ಶೂನ್ಯ *p_App_ದತ್ತಾಂಶ){
  win_Mem_ದತ್ತಾಂಶ *p_Win_Mem_ದತ್ತಾಂಶ = (win_Mem_ದತ್ತಾಂಶ *)p_App_ದತ್ತಾಂಶ;

  ಆದರೆ( !p_Win_Mem_ದತ್ತಾಂಶ ) ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  ಪ್ರತಿಜ್ಞಾ( p_Win_Mem_ದತ್ತಾಂಶ->magic1==WINMEM_MAGIC1 );
  ಪ್ರತಿಜ್ಞಾ( p_Win_Mem_ದತ್ತಾಂಶ->magic2==WINMEM_MAGIC2 );

#ಆದರೆ SQLITE_WIN32_ರಾಶಿ_CREATE
  ಆದರೆ( !p_Win_Mem_ದತ್ತಾಂಶ->h_ರಾಶಿ ){
    DWORD dw_Initial_ಗಾತ್ರ = SQLITE_WIN32_ರಾಶಿ_INIT_ಗಾತ್ರ;
    DWORD dw_ಗರಿಷ್ಠ_ಗಾತ್ರ = (DWORD)sqlite3GlobalConfig.n_ರಾಶಿ;
    ಆದರೆ( dw_ಗರಿಷ್ಠ_ಗಾತ್ರ==0 ){
      dw_ಗರಿಷ್ಠ_ಗಾತ್ರ = SQLITE_WIN32_ರಾಶಿ_MAX_ಗಾತ್ರ;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( dw_Initial_ಗಾತ್ರ>dw_ಗರಿಷ್ಠ_ಗಾತ್ರ ){
      dw_Initial_ಗಾತ್ರ = dw_ಗರಿಷ್ಠ_ಗಾತ್ರ;
    }
    p_Win_Mem_ದತ್ತಾಂಶ->h_ರಾಶಿ = os_ರಾಶಿ_Create(SQLITE_WIN32_ರಾಶಿ_FLAGS,
                                      dw_Initial_ಗಾತ್ರ, dw_ಗರಿಷ್ಠ_ಗಾತ್ರ);
    ಆದರೆ( !p_Win_Mem_ದತ್ತಾಂಶ->h_ರಾಶಿ ){
      sqlite3_log(SQLITE_NOMEM,
          "failed to HeapCreate (%lu), flags=%u, initSize=%lu, maxSize=%lu",
          os_Get_ಕೊನೆಯ_Error(), SQLITE_WIN32_ರಾಶಿ_FLAGS, dw_Initial_ಗಾತ್ರ,
          dw_ಗರಿಷ್ಠ_ಗಾತ್ರ);
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    }
    p_Win_Mem_ದತ್ತಾಂಶ->bOwned = TRUE;
    ಪ್ರತಿಜ್ಞಾ( p_Win_Mem_ದತ್ತಾಂಶ->bOwned );
  }
#ಇಲ್ಲದಿದ್ದರೆ
  p_Win_Mem_ದತ್ತಾಂಶ->h_ರಾಶಿ = os_Get_Process_ರಾಶಿ();
  ಆದರೆ( !p_Win_Mem_ದತ್ತಾಂಶ->h_ರಾಶಿ ){
    sqlite3_log(SQLITE_NOMEM,
        "failed to GetProcessHeap (%lu)", os_Get_ಕೊನೆಯ_Error());
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  p_Win_Mem_ದತ್ತಾಂಶ->bOwned = FALSE;
  ಪ್ರತಿಜ್ಞಾ( !p_Win_Mem_ದತ್ತಾಂಶ->bOwned );
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( p_Win_Mem_ದತ್ತಾಂಶ->h_ರಾಶಿ!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_Win_Mem_ದತ್ತಾಂಶ->h_ರಾಶಿ!=INVALID_HANDLE_ಮೌಲ್ಯ );
#ಆದರೆ defined(SQLITE_WIN32_MALLOC_VALIDATE)
  ಪ್ರತಿಜ್ಞಾ( os_ರಾಶಿ_Validate(p_Win_Mem_ದತ್ತಾಂಶ->h_ರಾಶಿ, SQLITE_WIN32_ರಾಶಿ_FLAGS, ಖಾಲಿ) );
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Deinitialize this module.
*/
ಸ್ಥಿರ ಶೂನ್ಯ winMemShutdown(ಶೂನ್ಯ *p_App_ದತ್ತಾಂಶ){
  win_Mem_ದತ್ತಾಂಶ *p_Win_Mem_ದತ್ತಾಂಶ = (win_Mem_ದತ್ತಾಂಶ *)p_App_ದತ್ತಾಂಶ;

  ಆದರೆ( !p_Win_Mem_ದತ್ತಾಂಶ ) ಹಿಂತಿರುಗಿ;
  ಪ್ರತಿಜ್ಞಾ( p_Win_Mem_ದತ್ತಾಂಶ->magic1==WINMEM_MAGIC1 );
  ಪ್ರತಿಜ್ಞಾ( p_Win_Mem_ದತ್ತಾಂಶ->magic2==WINMEM_MAGIC2 );

  ಆದರೆ( p_Win_Mem_ದತ್ತಾಂಶ->h_ರಾಶಿ ){
    ಪ್ರತಿಜ್ಞಾ( p_Win_Mem_ದತ್ತಾಂಶ->h_ರಾಶಿ!=INVALID_HANDLE_ಮೌಲ್ಯ );
#ಆದರೆ defined(SQLITE_WIN32_MALLOC_VALIDATE)
    ಪ್ರತಿಜ್ಞಾ( os_ರಾಶಿ_Validate(p_Win_Mem_ದತ್ತಾಂಶ->h_ರಾಶಿ, SQLITE_WIN32_ರಾಶಿ_FLAGS, ಖಾಲಿ) );
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( p_Win_Mem_ದತ್ತಾಂಶ->bOwned ){
      ಆದರೆ( !os_ರಾಶಿ_Destroy(p_Win_Mem_ದತ್ತಾಂಶ->h_ರಾಶಿ) ){
        sqlite3_log(SQLITE_NOMEM, "failed to HeapDestroy (%lu), heap=%p",
                    os_Get_ಕೊನೆಯ_Error(), (ಶೂನ್ಯ*)p_Win_Mem_ದತ್ತಾಂಶ->h_ರಾಶಿ);
      }
      p_Win_Mem_ದತ್ತಾಂಶ->bOwned = FALSE;
    }
    p_Win_Mem_ದತ್ತಾಂಶ->h_ರಾಶಿ = ಖಾಲಿ;
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
ಸ್ಥಿರಾಂಕ sqlite3_mem_methods *sqlite3MemGetWin32(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_mem_methods winMemMethods = {
    winMemMalloc,
    winMemFree,
    winMemRealloc,
    win_Mem_ಗಾತ್ರ,
    winMemRoundup,
    winMemInit,
    winMemShutdown,
    &win_mem_ದತ್ತಾಂಶ
  };
  ಹಿಂತಿರುಗಿ &winMemMethods;
}

ಶೂನ್ಯ sqlite3_Mem_ಗಣ_Default(ಶೂನ್ಯ){
  sqlite3_config(SQLITE_CONFIG_MALLOC, sqlite3MemGetWin32());
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_WIN32_MALLOC */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
/*
** Convert a UTF-8 string to Microsoft Unicode.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ಸ್ಥಿರ LPWSTR win_Utf8_ಗಮ್ಯ_Unicode(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪಠ್ಯ){
  ಪೂರ್ಣಾಂಕ nChar;
  LPWSTR z_Wide_ಪಠ್ಯ;

  nChar = os_Multi_Byte_ಗಮ್ಯ_Wide_Char(CP_UTF8, 0, z_ಪಠ್ಯ, -1, ಖಾಲಿ, 0);
  ಆದರೆ( nChar==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  z_Wide_ಪಠ್ಯ = sqlite3MallocZero( nChar*ಗಾತ್ರ(WCHAR) );
  ಆದರೆ( z_Wide_ಪಠ್ಯ==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  nChar = os_Multi_Byte_ಗಮ್ಯ_Wide_Char(CP_UTF8, 0, z_ಪಠ್ಯ, -1, z_Wide_ಪಠ್ಯ,
                                nChar);
  ಆದರೆ( nChar==0 ){
    sqlite3_free(z_Wide_ಪಠ್ಯ);
    z_Wide_ಪಠ್ಯ = 0;
  }
  ಹಿಂತಿರುಗಿ z_Wide_ಪಠ್ಯ;
}
#ಆದರೆ_ಅಂತ್ಯ /* _WIN32 */

/*
** Convert a Microsoft Unicode string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ಸ್ಥಿರ ಅಕ್ಷರ *win_Unicode_ಗಮ್ಯ_Utf8(LPCWSTR z_Wide_ಪಠ್ಯ){
  ಪೂರ್ಣಾಂಕ nByte;
  ಅಕ್ಷರ *z_ಪಠ್ಯ;

  nByte = os_Wide_Char_ಗಮ್ಯ_Multi_Byte(CP_UTF8, 0, z_Wide_ಪಠ್ಯ, -1, 0, 0, 0, 0);
  ಆದರೆ( nByte == 0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  z_ಪಠ್ಯ = sqlite3MallocZero( nByte );
  ಆದರೆ( z_ಪಠ್ಯ==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  nByte = os_Wide_Char_ಗಮ್ಯ_Multi_Byte(CP_UTF8, 0, z_Wide_ಪಠ್ಯ, -1, z_ಪಠ್ಯ, nByte,
                                0, 0);
  ಆದರೆ( nByte == 0 ){
    sqlite3_free(z_ಪಠ್ಯ);
    z_ಪಠ್ಯ = 0;
  }
  ಹಿಂತಿರುಗಿ z_ಪಠ್ಯ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
/*
** Convert an ANSI string to Microsoft Unicode, using the ANSI or OEM
** code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ಸ್ಥಿರ LPWSTR win_Mbcs_ಗಮ್ಯ_Unicode(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪಠ್ಯ, ಪೂರ್ಣಾಂಕ useAnsi){
  ಪೂರ್ಣಾಂಕ nWideChar;
  LPWSTR z_Mbcs_ಪಠ್ಯ;
  ಪೂರ್ಣಾಂಕ codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nWideChar = os_Multi_Byte_ಗಮ್ಯ_Wide_Char(codepage, 0, z_ಪಠ್ಯ, -1, ಖಾಲಿ,
                                0);
  ಆದರೆ( nWideChar==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  z_Mbcs_ಪಠ್ಯ = sqlite3MallocZero( nWideChar*ಗಾತ್ರ(WCHAR) );
  ಆದರೆ( z_Mbcs_ಪಠ್ಯ==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  nWideChar = os_Multi_Byte_ಗಮ್ಯ_Wide_Char(codepage, 0, z_ಪಠ್ಯ, -1, z_Mbcs_ಪಠ್ಯ,
                                nWideChar);
  ಆದರೆ( nWideChar==0 ){
    sqlite3_free(z_Mbcs_ಪಠ್ಯ);
    z_Mbcs_ಪಠ್ಯ = 0;
  }
  ಹಿಂತಿರುಗಿ z_Mbcs_ಪಠ್ಯ;
}

/*
** Convert a Microsoft Unicode string to a multi-byte character string,
** using the ANSI or OEM code page.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ಸ್ಥಿರ ಅಕ್ಷರ *win_Unicode_ಗಮ್ಯ_Mbcs(LPCWSTR z_Wide_ಪಠ್ಯ, ಪೂರ್ಣಾಂಕ useAnsi){
  ಪೂರ್ಣಾಂಕ nByte;
  ಅಕ್ಷರ *z_ಪಠ್ಯ;
  ಪೂರ್ಣಾಂಕ codepage = useAnsi ? CP_ACP : CP_OEMCP;

  nByte = os_Wide_Char_ಗಮ್ಯ_Multi_Byte(codepage, 0, z_Wide_ಪಠ್ಯ, -1, 0, 0, 0, 0);
  ಆದರೆ( nByte == 0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  z_ಪಠ್ಯ = sqlite3MallocZero( nByte );
  ಆದರೆ( z_ಪಠ್ಯ==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  nByte = os_Wide_Char_ಗಮ್ಯ_Multi_Byte(codepage, 0, z_Wide_ಪಠ್ಯ, -1, z_ಪಠ್ಯ,
                                nByte, 0, 0);
  ಆದರೆ( nByte == 0 ){
    sqlite3_free(z_ಪಠ್ಯ);
    z_ಪಠ್ಯ = 0;
  }
  ಹಿಂತಿರುಗಿ z_ಪಠ್ಯ;
}

/*
** Convert a multi-byte character string to UTF-8.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ಸ್ಥಿರ ಅಕ್ಷರ *win_Mbcs_ಗಮ್ಯ_Utf8(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪಠ್ಯ, ಪೂರ್ಣಾಂಕ useAnsi){
  ಅಕ್ಷರ *z_ಪಠ್ಯ_Utf8;
  LPWSTR zTmpWide;

  zTmpWide = win_Mbcs_ಗಮ್ಯ_Unicode(z_ಪಠ್ಯ, useAnsi);
  ಆದರೆ( zTmpWide==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  z_ಪಠ್ಯ_Utf8 = win_Unicode_ಗಮ್ಯ_Utf8(zTmpWide);
  sqlite3_free(zTmpWide);
  ಹಿಂತಿರುಗಿ z_ಪಠ್ಯ_Utf8;
}

/*
** Convert a UTF-8 string to a multi-byte character string.
**
** Space to hold the returned string is obtained from sqlite3_malloc().
*/
ಸ್ಥಿರ ಅಕ್ಷರ *win_Utf8_ಗಮ್ಯ_Mbcs(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪಠ್ಯ, ಪೂರ್ಣಾಂಕ useAnsi){
  ಅಕ್ಷರ *z_ಪಠ್ಯ_Mbcs;
  LPWSTR zTmpWide;

  zTmpWide = win_Utf8_ಗಮ್ಯ_Unicode(z_ಪಠ್ಯ);
  ಆದರೆ( zTmpWide==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  z_ಪಠ್ಯ_Mbcs = win_Unicode_ಗಮ್ಯ_Mbcs(zTmpWide, useAnsi);
  sqlite3_free(zTmpWide);
  ಹಿಂತಿರುಗಿ z_ಪಠ್ಯ_Mbcs;
}

/*
** This is a public wrapper for the winUtf8ToUnicode() function.
*/
LPWSTR sqlite3_win32_utf8_ಗಮ್ಯ_unicode(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪಠ್ಯ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !z_ಪಠ್ಯ ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize() ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ win_Utf8_ಗಮ್ಯ_Unicode(z_ಪಠ್ಯ);
}

/*
** This is a public wrapper for the winUnicodeToUtf8() function.
*/
ಅಕ್ಷರ *sqlite3_win32_unicode_ಗಮ್ಯ_utf8(LPCWSTR z_Wide_ಪಠ್ಯ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !z_Wide_ಪಠ್ಯ ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize() ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ win_Unicode_ಗಮ್ಯ_Utf8(z_Wide_ಪಠ್ಯ);
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
ಅಕ್ಷರ *sqlite3_win32_mbcs_ಗಮ್ಯ_utf8(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪಠ್ಯ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !z_ಪಠ್ಯ ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize() ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ win_Mbcs_ಗಮ್ಯ_Utf8(z_ಪಠ್ಯ, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winMbcsToUtf8() function.
*/
ಅಕ್ಷರ *sqlite3_win32_mbcs_ಗಮ್ಯ_utf8_v2(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪಠ್ಯ, ಪೂರ್ಣಾಂಕ useAnsi){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !z_ಪಠ್ಯ ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize() ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ win_Mbcs_ಗಮ್ಯ_Utf8(z_ಪಠ್ಯ, useAnsi);
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
ಅಕ್ಷರ *sqlite3_win32_utf8_ಗಮ್ಯ_mbcs(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪಠ್ಯ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !z_ಪಠ್ಯ ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize() ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ win_Utf8_ಗಮ್ಯ_Mbcs(z_ಪಠ್ಯ, osAreFileApisANSI());
}

/*
** This is a public wrapper for the winUtf8ToMbcs() function.
*/
ಅಕ್ಷರ *sqlite3_win32_utf8_ಗಮ್ಯ_mbcs_v2(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಪಠ್ಯ, ಪೂರ್ಣಾಂಕ useAnsi){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !z_ಪಠ್ಯ ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  ಆದರೆ( sqlite3_initialize() ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ win_Utf8_ಗಮ್ಯ_Mbcs(z_ಪಠ್ಯ, useAnsi);
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-8 string.
*/
ಪೂರ್ಣಾಂಕ sqlite3_win32_ಗಣ_directory8(
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ type, /* Identifier for directory being set or reset */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮೌಲ್ಯ  /* New value for directory being set or reset */
){
  ಅಕ್ಷರ **ppDirectory = 0;
  ಪೂರ್ಣಾಂಕ rc;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ಆದರೆ( type==SQLITE_WIN32_ದತ್ತಾಂಶ_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_ದತ್ತಾಂಶ_directory;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( type==SQLITE_WIN32_ತಾತ್ಕಾಲಿಕ_DIRECTORY_TYPE ){
    ppDirectory = &sqlite3_ತಾತ್ಕಾಲಿಕ_directory;
  }
  ಪ್ರತಿಜ್ಞಾ( !ppDirectory || type==SQLITE_WIN32_ದತ್ತಾಂಶ_DIRECTORY_TYPE
          || type==SQLITE_WIN32_ತಾತ್ಕಾಲಿಕ_DIRECTORY_TYPE
  );
  ಪ್ರತಿಜ್ಞಾ( !ppDirectory || sqlite3MemdebugHasType(*ppDirectory, MEMTYPE_ರಾಶಿ) );
  ಆದರೆ( ppDirectory ){
    ಅಕ್ಷರ *zCopy = 0;
    ಆದರೆ( z_ಮೌಲ್ಯ && z_ಮೌಲ್ಯ[0] ){
      zCopy = sqlite3_mprintf("%s", z_ಮೌಲ್ಯ);
      ಆದರೆ ( zCopy==0 ){
        rc = SQLITE_NOMEM_BKPT;
        ಹೋಗು ಗಣ_directory8_ಮುಗಿದ;
      }
    }
    sqlite3_free(*ppDirectory);
    *ppDirectory = zCopy;
    rc = SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = SQLITE_ERROR;
  }
ಗಣ_directory8_ಮುಗಿದ:
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This function is the same as sqlite3_win32_set_directory (below); however,
** it accepts a UTF-16 string.
*/
ಪೂರ್ಣಾಂಕ sqlite3_win32_ಗಣ_directory16(
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ type, /* Identifier for directory being set or reset */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z_ಮೌಲ್ಯ  /* New value for directory being set or reset */
){
  ಪೂರ್ಣಾಂಕ rc;
  ಅಕ್ಷರ *zUtf8 = 0;
  ಆದರೆ( z_ಮೌಲ್ಯ ){
    zUtf8 = sqlite3_win32_unicode_ಗಮ್ಯ_utf8(z_ಮೌಲ್ಯ);
    ಆದರೆ( zUtf8==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  rc = sqlite3_win32_ಗಣ_directory8(type, zUtf8);
  ಆದರೆ( zUtf8 ) sqlite3_free(zUtf8);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This function sets the data directory or the temporary directory based on
** the provided arguments.  The type argument must be 1 in order to set the
** data directory or 2 in order to set the temporary directory.  The zValue
** argument is the name of the directory to use.  The return value will be
** SQLITE_OK if successful.
*/
ಪೂರ್ಣಾಂಕ sqlite3_win32_ಗಣ_directory(
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ type, /* Identifier for directory being set or reset */
  ಶೂನ್ಯ *z_ಮೌಲ್ಯ        /* New value for directory being set or reset */
){
  ಹಿಂತಿರುಗಿ sqlite3_win32_ಗಣ_directory16(type, z_ಮೌಲ್ಯ);
}
#ಆದರೆ_ಅಂತ್ಯ /* _WIN32 */

/*
** The return value of winGetLastErrorMsg
** is zero if the error message fits in the buffer, or non-zero
** otherwise (if the message was truncated).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Get_ಕೊನೆಯ_Error_ಸಂದೇಶ(DWORD ಕೊನೆಯ_Errno, ಪೂರ್ಣಾಂಕ nBuf, ಅಕ್ಷರ *zBuf){
  /* FormatMessage returns 0 on failure.  Otherwise it
  ** returns the number of TCHARs written to the output
  ** buffer, excluding the terminating null char.
  */
  DWORD dwLen = 0;
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು = 0;
  LPWSTR z_ತಾತ್ಕಾಲಿಕ_Wide = ಖಾಲಿ;
  dwLen = os_Format_ಸಂದೇಶ_W(FORMAT_ಸಂದೇಶ_ALLOCATE_ಬಫರ್ |
                           FORMAT_ಸಂದೇಶ_FROM_SYSTEM |
                           FORMAT_ಸಂದೇಶ_IGNORE_INSERTS,
                           ಖಾಲಿ,
                           ಕೊನೆಯ_Errno,
                           0,
                           (LPWSTR) &z_ತಾತ್ಕಾಲಿಕ_Wide,
                           0,
                           0);
  ಆದರೆ( dwLen > 0 ){
    /* allocate a buffer and convert to UTF8 */
    sqlite3BeginBenignMalloc();
    z_ಹೊರಹೋಗು = win_Unicode_ಗಮ್ಯ_Utf8(z_ತಾತ್ಕಾಲಿಕ_Wide);
    sqlite3_ಕೊನೆ_Benign_Malloc();
    /* free the system buffer allocated by FormatMessage */
    osLocalFree(z_ತಾತ್ಕಾಲಿಕ_Wide);
  }
  ಆದರೆ( 0 == dwLen ){
    sqlite3_snprintf(nBuf, zBuf, "OsError 0x%lx (%lu)", ಕೊನೆಯ_Errno, ಕೊನೆಯ_Errno);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* copy a maximum of nBuf chars to output buffer */
    sqlite3_snprintf(nBuf, zBuf, "%s", z_ಹೊರಹೋಗು);
    /* free the UTF8 buffer */
    sqlite3_free(z_ಹೊರಹೋಗು);
  }
  ಹಿಂತಿರುಗಿ 0;
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
#ವ್ಯಾಖ್ಯಾನಿಸು winLogError(a,b,c,d)   win_Log_Error_At_ಸಾಲು(a,b,c,d,__LINE__)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Log_Error_At_ಸಾಲು(
  ಪೂರ್ಣಾಂಕ errcode,                    /* SQLite error code */
  DWORD ಕೊನೆಯ_Errno,                /* Win32 last error */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFunc,              /* Name of OS function that failed */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ,              /* File path associated with error */
  ಪೂರ್ಣಾಂಕ i_ಸಾಲು                       /* Source line number where error occurred */
){
  ಅಕ್ಷರ z_ಸಂದೇಶ[500];                 /* Human readable error text */
  ಪೂರ್ಣಾಂಕ i;                          /* Loop counter */

  z_ಸಂದೇಶ[0] = 0;
  win_Get_ಕೊನೆಯ_Error_ಸಂದೇಶ(ಕೊನೆಯ_Errno, ಗಾತ್ರ(z_ಸಂದೇಶ), z_ಸಂದೇಶ);
  ಪ್ರತಿಜ್ಞಾ( errcode!=SQLITE_OK );
  ಆದರೆ( z_ಮಾರ್ಗ==0 ) z_ಮಾರ್ಗ = "";
  ಗಾಗಿ(i=0; z_ಸಂದೇಶ[i] && z_ಸಂದೇಶ[i]!='\r' && z_ಸಂದೇಶ[i]!='\n'; i++){}
  z_ಸಂದೇಶ[i] = 0;
  sqlite3_log(errcode,
      "os_win.c:%d: (%lu) %s(%s) - %s",
      i_ಸಾಲು, ಕೊನೆಯ_Errno, zFunc, z_ಮಾರ್ಗ, z_ಸಂದೇಶ
  );

  ಹಿಂತಿರುಗಿ errcode;
}

/*
** The number of times that a ReadFile(), WriteFile(), and DeleteFile()
** will be retried following a locking error - probably caused by
** antivirus software.  Also the initial delay before the first retry.
** The delay increases linearly with each retry.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WIN32_IOERR_RETRY
# define SQLITE_WIN32_IOERR_RETRY 10
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_WIN32_IOERR_RETRY_DELAY
# define SQLITE_WIN32_IOERR_RETRY_DELAY 25
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winIoerrRetry = SQLITE_WIN32_IOERR_RETRY;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winIoerrRetryDelay = SQLITE_WIN32_IOERR_RETRY_DELAY;

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
#ಆದರೆ !defined(winIoerrCanRetry1)
#ವ್ಯಾಖ್ಯಾನಿಸು winIoerrCanRetry1(a) (((a)==ERROR_ACCESS_DENIED)        || \
                              ((a)==ERROR_SHARING_VIOLATION)    || \
                              ((a)==ERROR_ಬೀಗ_VIOLATION)       || \
                              ((a)==ERROR_DEV_NOT_EXIST)        || \
                              ((a)==ERROR_NETNAME_DELETED)      || \
                              ((a)==ERROR_SEM_TIMEOUT)          || \
                              ((a)==ERROR_ಜಾಲ_UNREACHABLE))
#ಆದರೆ_ಅಂತ್ಯ

/*
** If a ReadFile() or WriteFile() error occurs, invoke this routine
** to see if it should be retried.  Return TRUE to retry.  Return FALSE
** to give up with an error.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winRetryIoerr(ಪೂರ್ಣಾಂಕ *pnRetry, DWORD *pError){
  DWORD e = os_Get_ಕೊನೆಯ_Error();
  ಆದರೆ( *pnRetry>=winIoerrRetry ){
    ಆದರೆ( pError ){
      *pError = e;
    }
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( winIoerrCanRetry1(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    ಹಿಂತಿರುಗಿ 1;
  }
#ಆದರೆ defined(winIoerrCanRetry2)
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( winIoerrCanRetry2(e) ){
    osSleep(winIoerrRetryDelay*(1+*pnRetry));
    ++*pnRetry;
    ಹಿಂತಿರುಗಿ 1;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( pError ){
    *pError = e;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Log a I/O error retry episode.
*/
ಸ್ಥಿರ ಶೂನ್ಯ winLogIoerr(ಪೂರ್ಣಾಂಕ nRetry, ಪೂರ್ಣಾಂಕ lineno){
  ಆದರೆ( nRetry ){
    sqlite3_log(SQLITE_NOTICE,
      "delayed %dms for lock/sharing conflict at line %d",
      winIoerrRetryDelay*nRetry*(nRetry+1)/2, lineno
    );
  }
}


/*
** Lock a file region.
*/
ಸ್ಥಿರ BOOL win_ಬೀಗ_File(
  LPHANDLE phFile,
  DWORD flags,
  DWORD ಸ್ಥಾನಾಂತರ_ಕೆಳಗಿನ,
  DWORD ಸ್ಥಾನಾಂತರ_ಮೇಲಿನ,
  DWORD ಅಂಕೆ_ಬೈಟುಗಳು_ಕೆಳಗಿನ,
  DWORD ಅಂಕೆ_ಬೈಟುಗಳು_ಮೇಲಿನ
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, ಗಾತ್ರ(OVERLAPPED));
  ovlp.ಸ್ಥಾನಾಂತರ_2 = ಸ್ಥಾನಾಂತರ_ಕೆಳಗಿನ;
  ovlp.ಸ್ಥಾನಾಂತರ_ಮೇಲಿನ_2 = ಸ್ಥಾನಾಂತರ_ಮೇಲಿನ;
  ಹಿಂತಿರುಗಿ os_ಬೀಗ_File_Ex(*phFile, flags, 0, ಅಂಕೆ_ಬೈಟುಗಳು_ಕೆಳಗಿನ, ಅಂಕೆ_ಬೈಟುಗಳು_ಮೇಲಿನ, &ovlp);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Handle_ಬೀಗ_Timeout(
  HANDLE hFile,
  DWORD ಸ್ಥಾನಾಂತರ,
  DWORD nByte,
  ಪೂರ್ಣಾಂಕ bExcl,
  DWORD nMs
){
  DWORD flags = LOCKFILE_FAIL_IMMEDIATELY | (bExcl?LOCKFILE_EXCLUSIVE_ಬೀಗ:0);
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  BOOL ret;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, ಗಾತ್ರ(OVERLAPPED));
  ovlp.ಸ್ಥಾನಾಂತರ_2 = ಸ್ಥಾನಾಂತರ;

#ಆದರೆ defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  ಆದರೆ( nMs!=0 ){
    flags &= ~LOCKFILE_FAIL_IMMEDIATELY;
  }
  ovlp.hEvent = osCreateEvent(ಖಾಲಿ, TRUE, FALSE, ಖಾಲಿ);
  ಆದರೆ( ovlp.hEvent==ಖಾಲಿ ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_ಬೀಗ;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ret = os_ಬೀಗ_File_Ex(hFile, flags, 0, nByte, 0, &ovlp);
#ಆದರೆ defined(SQLITE_ENABLE_SETLK_TIMEOUT)
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
  ಆದರೆ( !ret && Get_ಕೊನೆಯ_Error()==ERROR_IO_PENDING ){
    DWORD nDelay = (nMs==0 ? INFINITE : nMs);
    DWORD res = osWaitForSingleObject(ovlp.hEvent, nDelay);
    ಆದರೆ( res==WAIT_OBJECT_0 ){
      ret = TRUE;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( res==WAIT_TIMEOUT ){
#ಆದರೆ SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = SQLITE_BUSY_TIMEOUT;
#ಇಲ್ಲದಿದ್ದರೆ
      rc = SQLITE_BUSY;
#ಆದರೆ_ಅಂತ್ಯ
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Some other error has occurred */
      rc = SQLITE_IOERR_ಬೀಗ;
    }

    /* If it is still pending, cancel the LockFileEx() call. */
    osCancelIo(hFile);
  }

  osCloseHandle(ovlp.hEvent);
#ಆದರೆ_ಅಂತ್ಯ /* defined(SQLITE_ENABLE_SETLK_TIMEOUT) */

  ಆದರೆ( rc==SQLITE_OK && !ret ){
    rc = SQLITE_BUSY;
  }
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* #ifndef SQLITE_OMIT_WAL */

/*
** Unlock a file region.
 */
ಸ್ಥಿರ BOOL winUnlockFile(
  LPHANDLE phFile,
  DWORD ಸ್ಥಾನಾಂತರ_ಕೆಳಗಿನ,
  DWORD ಸ್ಥಾನಾಂತರ_ಮೇಲಿನ,
  DWORD ಅಂಕೆ_ಬೈಟುಗಳು_ಕೆಳಗಿನ,
  DWORD ಅಂಕೆ_ಬೈಟುಗಳು_ಮೇಲಿನ
){
  OVERLAPPED ovlp;
  memset(&ovlp, 0, ಗಾತ್ರ(OVERLAPPED));
  ovlp.ಸ್ಥಾನಾಂತರ_2 = ಸ್ಥಾನಾಂತರ_ಕೆಳಗಿನ;
  ovlp.ಸ್ಥಾನಾಂತರ_ಮೇಲಿನ_2 = ಸ್ಥಾನಾಂತರ_ಮೇಲಿನ;
  ಹಿಂತಿರುಗಿ osUnlockFileEx(*phFile, 0, ಅಂಕೆ_ಬೈಟುಗಳು_ಕೆಳಗಿನ, ಅಂಕೆ_ಬೈಟುಗಳು_ಮೇಲಿನ, &ovlp);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
/*
** Remove an nByte lock starting at offset iOff from HANDLE h.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winHandleUnlock(HANDLE h, ಪೂರ್ಣಾಂಕ iOff, ಪೂರ್ಣಾಂಕ nByte){
  BOOL ret = winUnlockFile(&h, iOff, 0, nByte, 0);
  ಹಿಂತಿರುಗಿ (ret ? SQLITE_OK : SQLITE_IOERR_UNLOCK);
}
#ಆದರೆ_ಅಂತ್ಯ

/*****************************************************************************
** The next group of routines implement the I/O methods specified
** by the sqlite3_io_methods object.
******************************************************************************/

/*
** Some Microsoft compilers lack this definition.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ INVALID_ಗಣ_FILE_ಸೂಚಕ
# define INVALID_ಗಣ_FILE_ಸೂಚಕ ((DWORD)-1)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Seek the file handle h to offset nByte of the file.
**
** If successful, return SQLITE_OK. Or, if an error occurs, return an SQLite
** error code.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winHandleSeek(HANDLE h, sqlite3_int64 i_ಸ್ಥಾನಾಂತರ){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;             /* Return value */
  LARGE_INTEGER x;                /* The offset */

  x.QuadPart = i_ಸ್ಥಾನಾಂತರ;
  ಆದರೆ( os_ಗಣ_File_ಸೂಚಕ_Ex(h, x, 0, FILE_BEGIN)==0 ){
    rc = SQLITE_IOERR_SEEK;
  }
  OSTRACE(("SEEK file=%p, offset=%lld rc=%s\n", h, i_ಸ್ಥಾನಾಂತರ,sqlite3ErrName(rc)));
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Move the current position of the file handle passed as the first
** argument to offset iOffset within the file. If successful, return 0.
** Otherwise, set pFile->lastErrno and return non-zero.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winSeekFile(winFile *pFile, sqlite3_int64 i_ಸ್ಥಾನಾಂತರ){
  ಪೂರ್ಣಾಂಕ rc;

  rc = winHandleSeek(pFile->h, i_ಸ್ಥಾನಾಂತರ);
  ಆದರೆ( rc!=SQLITE_OK ){
    pFile->ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
    winLogError(rc, pFile->ಕೊನೆಯ_Errno, "winSeekFile", pFile->z_ಮಾರ್ಗ);
  }
  ಹಿಂತಿರುಗಿ rc;
}


#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
/* Forward references to VFS helper methods used for memory mapped files */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winMapfile(winFile*, sqlite3_int64);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winUnmapfile(winFile*);
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿಸು MX_CLOSE_ATTEMPT 3
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winClose(sqlite3_file *id){
  ಪೂರ್ಣಾಂಕ rc, cnt = 0;
  winFile *pFile = (winFile*)id;

  ಪ್ರತಿಜ್ಞಾ( id!=0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  ಪ್ರತಿಜ್ಞಾ( pFile->pShm==0 );
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( pFile->h!=ಖಾಲಿ && pFile->h!=INVALID_HANDLE_ಮೌಲ್ಯ );
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p\n",
           osGetCurrentProcessId(), pFile, pFile->h));

#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  winUnmapfile(pFile);
#ಆದರೆ_ಅಂತ್ಯ

  ಮಾಡು{
    rc = osCloseHandle(pFile->h);
    /* SimulateIOError( rc=0; cnt=MX_CLOSE_ATTEMPT; ); */
  }ಆಗ( rc==0 && ++cnt < MX_CLOSE_ATTEMPT && (osSleep(100), 1) );
  ಆದರೆ( rc ){
    pFile->h = ಖಾಲಿ;
  }
  OpenCounter(-1);
  OSTRACE(("CLOSE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, rc ? "ok" : "failed"));
  ಹಿಂತಿರುಗಿ rc ? SQLITE_OK
            : winLogError(SQLITE_IOERR_CLOSE, os_Get_ಕೊನೆಯ_Error(),
                          "winClose", pFile->z_ಮಾರ್ಗ);
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winRead(
  sqlite3_file *id,          /* File to read from */
  ಶೂನ್ಯ *pBuf,                /* Write content into this buffer */
  ಪೂರ್ಣಾಂಕ amt,                   /* Number of bytes to read */
  sqlite3_int64 ಸ್ಥಾನಾಂತರ       /* Begin reading at this offset */
){
#ಆದರೆ !defined(SQLITE_WIN32_NO_OVERLAPPED)
  OVERLAPPED overlapped;          /* The offset for ReadFile. */
#ಆದರೆ_ಅಂತ್ಯ
  winFile *pFile = (winFile*)id;  /* file handle */
  DWORD nRead;                    /* Number of bytes actually read from file */
  ಪೂರ್ಣಾಂಕ nRetry = 0;                 /* Number of retrys */

  ಪ್ರತಿಜ್ಞಾ( id!=0 );
  ಪ್ರತಿಜ್ಞಾ( amt>0 );
  ಪ್ರತಿಜ್ಞಾ( ಸ್ಥಾನಾಂತರ>=0 );
  SimulateIOError(ಹಿಂತಿರುಗಿ SQLITE_IOERR_READ);
  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, ಸ್ಥಾನಾಂತರ, pFile->locktype));

#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  ಆದರೆ( ಸ್ಥಾನಾಂತರ<pFile->mmap_ಗಾತ್ರ ){
    ಆದರೆ( ಸ್ಥಾನಾಂತರ+amt <= pFile->mmap_ಗಾತ್ರ ){
      ಸ್ಮರಣೆ_ನಕಲು(pBuf, &((u8 *)(pFile->pMapRegion))[ಸ್ಥಾನಾಂತರ], amt);
      OSTRACE(("READ-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ nCopy = (ಪೂರ್ಣಾಂಕ)(pFile->mmap_ಗಾತ್ರ - ಸ್ಥಾನಾಂತರ);
      ಸ್ಮರಣೆ_ನಕಲು(pBuf, &((u8 *)(pFile->pMapRegion))[ಸ್ಥಾನಾಂತರ], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      ಸ್ಥಾನಾಂತರ += nCopy;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(SQLITE_WIN32_NO_OVERLAPPED)
  ಆದರೆ( winSeekFile(pFile, ಸ್ಥಾನಾಂತರ) ){
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ಹಿಂತಿರುಗಿ SQLITE_ತುಂಬಿದ;
  }
  ಆಗ( !osReadFile(pFile->h, pBuf, amt, &nRead, 0) ){
#ಇಲ್ಲದಿದ್ದರೆ
  memset(&overlapped, 0, ಗಾತ್ರ(OVERLAPPED));
  overlapped.ಸ್ಥಾನಾಂತರ_2 = (LONG)(ಸ್ಥಾನಾಂತರ & 0xffffffff);
  overlapped.ಸ್ಥಾನಾಂತರ_ಮೇಲಿನ_2 = (LONG)((ಸ್ಥಾನಾಂತರ>>32) & 0x7fffffff);
  ಆಗ( !osReadFile(pFile->h, pBuf, amt, &nRead, &overlapped) &&
         os_Get_ಕೊನೆಯ_Error()!=ERROR_HANDLE_EOF ){
#ಆದರೆ_ಅಂತ್ಯ
    DWORD ಕೊನೆಯ_Errno;
    ಆದರೆ( winRetryIoerr(&nRetry, &ಕೊನೆಯ_Errno) ) ಮುಂದುವರಿ;
    pFile->ಕೊನೆಯ_Errno = ಕೊನೆಯ_Errno;
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ಹಿಂತಿರುಗಿ winLogError(SQLITE_IOERR_READ, pFile->ಕೊನೆಯ_Errno,
                       "winRead", pFile->z_ಮಾರ್ಗ);
  }
  winLogIoerr(nRetry, __LINE__);
  ಆದರೆ( nRead<(DWORD)amt ){
    /* Unread parts of the buffer must be zero-filled */
    memset(&((ಅಕ್ಷರ*)pBuf)[nRead], 0, amt-nRead);
    OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_SHORT_READ\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_SHORT_READ;
  }

  OSTRACE(("READ pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winWrite(
  sqlite3_file *id,               /* File to write into */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pBuf,               /* The bytes to be written */
  ಪೂರ್ಣಾಂಕ amt,                        /* Number of bytes to write */
  sqlite3_int64 ಸ್ಥಾನಾಂತರ            /* Offset into the file to begin writing at */
){
  ಪೂರ್ಣಾಂಕ rc = 0;                     /* True if error has occurred, else false */
  winFile *pFile = (winFile*)id;  /* File handle */
  ಪೂರ್ಣಾಂಕ nRetry = 0;                 /* Number of retries */

  ಪ್ರತಿಜ್ಞಾ( amt>0 );
  ಪ್ರತಿಜ್ಞಾ( pFile );
  SimulateIOError(ಹಿಂತಿರುಗಿ SQLITE_IOERR_WRITE);
  SimulateDiskfullError(ಹಿಂತಿರುಗಿ SQLITE_ತುಂಬಿದ);

  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, buffer=%p, amount=%d, "
           "offset=%lld, lock=%d\n", osGetCurrentProcessId(), pFile,
           pFile->h, pBuf, amt, ಸ್ಥಾನಾಂತರ, pFile->locktype));

#ಆದರೆ defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  ಆದರೆ( ಸ್ಥಾನಾಂತರ<pFile->mmap_ಗಾತ್ರ ){
    ಆದರೆ( ಸ್ಥಾನಾಂತರ+amt <= pFile->mmap_ಗಾತ್ರ ){
      ಸ್ಮರಣೆ_ನಕಲು(&((u8 *)(pFile->pMapRegion))[ಸ್ಥಾನಾಂತರ], pBuf, amt);
      OSTRACE(("WRITE-MMAP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ nCopy = (ಪೂರ್ಣಾಂಕ)(pFile->mmap_ಗಾತ್ರ - ಸ್ಥಾನಾಂತರ);
      ಸ್ಮರಣೆ_ನಕಲು(&((u8 *)(pFile->pMapRegion))[ಸ್ಥಾನಾಂತರ], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      ಸ್ಥಾನಾಂತರ += nCopy;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(SQLITE_WIN32_NO_OVERLAPPED)
  rc = winSeekFile(pFile, ಸ್ಥಾನಾಂತರ);
  ಆದರೆ( rc==0 ){
#ಇಲ್ಲದಿದ್ದರೆ
  {
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(SQLITE_WIN32_NO_OVERLAPPED)
    OVERLAPPED overlapped;        /* The offset for WriteFile. */
#ಆದರೆ_ಅಂತ್ಯ
    u8 *aRem = (u8 *)pBuf;        /* Data yet to be written */
    ಪೂರ್ಣಾಂಕ nRem = amt;               /* Number of bytes yet to be written */
    DWORD nWrite;                 /* Bytes written by each WriteFile() call */
    DWORD ಕೊನೆಯ_Errno = NO_ERROR;   /* Value returned by GetLastError() */

#ಆದರೆ !defined(SQLITE_WIN32_NO_OVERLAPPED)
    memset(&overlapped, 0, ಗಾತ್ರ(OVERLAPPED));
    overlapped.ಸ್ಥಾನಾಂತರ_2 = (LONG)(ಸ್ಥಾನಾಂತರ & 0xffffffff);
    overlapped.ಸ್ಥಾನಾಂತರ_ಮೇಲಿನ_2 = (LONG)((ಸ್ಥಾನಾಂತರ>>32) & 0x7fffffff);
#ಆದರೆ_ಅಂತ್ಯ

    ಆಗ( nRem>0 ){
#ಆದರೆ defined(SQLITE_WIN32_NO_OVERLAPPED)
      ಆದರೆ( !osWriteFile(pFile->h, aRem, nRem, &nWrite, 0) ){
#ಇಲ್ಲದಿದ್ದರೆ
      ಆದರೆ( !osWriteFile(pFile->h, aRem, nRem, &nWrite, &overlapped) ){
#ಆದರೆ_ಅಂತ್ಯ
        ಆದರೆ( winRetryIoerr(&nRetry, &ಕೊನೆಯ_Errno) ) ಮುಂದುವರಿ;
        ನಿಲ್ಲಿಸು;
      }
      ಪ್ರತಿಜ್ಞಾ( nWrite==0 || nWrite<=(DWORD)nRem );
      ಆದರೆ( nWrite==0 || nWrite>(DWORD)nRem ){
        ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
        ನಿಲ್ಲಿಸು;
      }
#ಆದರೆ !defined(SQLITE_WIN32_NO_OVERLAPPED)
      ಸ್ಥಾನಾಂತರ += nWrite;
      overlapped.ಸ್ಥಾನಾಂತರ_2 = (LONG)(ಸ್ಥಾನಾಂತರ & 0xffffffff);
      overlapped.ಸ್ಥಾನಾಂತರ_ಮೇಲಿನ_2 = (LONG)((ಸ್ಥಾನಾಂತರ>>32) & 0x7fffffff);
#ಆದರೆ_ಅಂತ್ಯ
      aRem += nWrite;
      nRem -= nWrite;
    }
    ಆದರೆ( nRem>0 ){
      pFile->ಕೊನೆಯ_Errno = ಕೊನೆಯ_Errno;
      rc = 1;
    }
  }

  ಆದರೆ( rc ){
    ಆದರೆ(   ( pFile->ಕೊನೆಯ_Errno==ERROR_HANDLE_DISK_ತುಂಬಿದ )
       || ( pFile->ಕೊನೆಯ_Errno==ERROR_DISK_ತುಂಬಿದ )){
      OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_FULL\n",
               osGetCurrentProcessId(), pFile, pFile->h));
      ಹಿಂತಿರುಗಿ winLogError(SQLITE_ತುಂಬಿದ, pFile->ಕೊನೆಯ_Errno,
                         "winWrite1", pFile->z_ಮಾರ್ಗ);
    }
    OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_WRITE\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ಹಿಂತಿರುಗಿ winLogError(SQLITE_IOERR_WRITE, pFile->ಕೊನೆಯ_Errno,
                       "winWrite2", pFile->z_ಮಾರ್ಗ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    winLogIoerr(nRetry, __LINE__);
  }
  OSTRACE(("WRITE pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
/*
** Truncate the file opened by handle h to nByte bytes in size.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winHandleTruncate(HANDLE h, sqlite3_int64 nByte){ 
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;             /* Return code */
  rc = winHandleSeek(h, nByte);
  ಆದರೆ( rc==SQLITE_OK ){
    ಆದರೆ( 0==os_ಗಣ_ಕೊನೆ_ದ_File(h) ){
      rc = SQLITE_IOERR_TRUNCATE;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Determine the size in bytes of the file opened by the handle passed as 
** the first argument.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Handle_ಗಾತ್ರ(HANDLE h, sqlite3_int64 *pnByte){ 
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  LARGE_INTEGER x;
  ಪ್ರತಿಜ್ಞಾ( pnByte );
  ಆದರೆ( os_Get_File_ಗಾತ್ರ_Ex(h, &x)==0 ){
    rc = SQLITE_IOERR_FSTAT;
    *pnByte = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    *pnByte = x.QuadPart;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Close the handle passed as the only argument.
*/
ಸ್ಥಿರ ಶೂನ್ಯ winHandleClose(HANDLE h){
  ಆದರೆ( h!=INVALID_HANDLE_ಮೌಲ್ಯ ){
    osCloseHandle(h);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* #ifndef SQLITE_OMIT_WAL */

/*
** Truncate an open file to a specified size
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winTruncate(sqlite3_file *id, sqlite3_int64 nByte){
  winFile *pFile = (winFile*)id;  /* File handle object */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;             /* Return code for this function */
  DWORD ಕೊನೆಯ_Errno;
#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  sqlite3_int64 old_Mmap_ಗಾತ್ರ;
  ಆದರೆ( pFile->n_Fetch_ಹೊರಹೋಗು>0 ){
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
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( pFile );
  SimulateIOError(ಹಿಂತಿರುಗಿ SQLITE_IOERR_TRUNCATE);
  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, size=%lld, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, nByte, pFile->locktype));

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  ಆದರೆ( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  ಆದರೆ( pFile->pMapRegion ){
    old_Mmap_ಗಾತ್ರ = pFile->mmap_ಗಾತ್ರ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    old_Mmap_ಗಾತ್ರ = 0;
  }
  winUnmapfile(pFile);
#ಆದರೆ_ಅಂತ್ಯ

  /* SetEndOfFile() returns non-zero when successful, or zero when it fails. */
  ಆದರೆ( winSeekFile(pFile, nByte) ){
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->ಕೊನೆಯ_Errno,
                     "winTruncate1", pFile->z_ಮಾರ್ಗ);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( 0==os_ಗಣ_ಕೊನೆ_ದ_File(pFile->h) &&
            ((ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error())!=ERROR_USER_MAPPED_FILE) ){
    pFile->ಕೊನೆಯ_Errno = ಕೊನೆಯ_Errno;
    rc = winLogError(SQLITE_IOERR_TRUNCATE, pFile->ಕೊನೆಯ_Errno,
                     "winTruncate2", pFile->z_ಮಾರ್ಗ);
  }

#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  ಆದರೆ( rc==SQLITE_OK && old_Mmap_ಗಾತ್ರ>0 ){
    ಆದರೆ( old_Mmap_ಗಾತ್ರ>nByte ){
      winMapfile(pFile, -1);
    }ಇಲ್ಲದಿದ್ದರೆ{
      winMapfile(pFile, old_Mmap_ಗಾತ್ರ);
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  OSTRACE(("TRUNCATE pid=%lu, pFile=%p, file=%p, rc=%s\n",
           osGetCurrentProcessId(), pFile, pFile->h, sqlite3ErrName(rc)));
  ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occurring at the right times.
*/
ಪೂರ್ಣಾಂಕ sqlite3_sync_ಎಣಿಕೆ = 0;
ಪೂರ್ಣಾಂಕ sqlite3_fullsync_ಎಣಿಕೆ = 0;
#ಆದರೆ_ಅಂತ್ಯ

/*
** Make sure all writes to a particular file are committed to disk.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winSync(sqlite3_file *id, ಪೂರ್ಣಾಂಕ flags){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_NO_SYNC
  /*
  ** Used only when SQLITE_NO_SYNC is not defined.
   */
  BOOL rc;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(NDEBUG) || !defined(SQLITE_NO_SYNC) || \
    defined(SQLITE_HAVE_OS_TRACE)
  /*
  ** Used when SQLITE_NO_SYNC is not defined and by the assert() and/or
  ** OSTRACE() macros.
   */
  winFile *pFile = (winFile*)id;
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಳಸದ_PARAMETER(id);
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( pFile );
  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  ಪ್ರತಿಜ್ಞಾ((flags&0x0F)==SQLITE_SYNC_ಸಾಮಾನ್ಯ
      || (flags&0x0F)==SQLITE_SYNC_ತುಂಬಿದ
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( ಹಿಂತಿರುಗಿ SQLITE_ತುಂಬಿದ );

  OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, flags=%x, lock=%d\n",
           osGetCurrentProcessId(), pFile, pFile->h, flags,
           pFile->locktype));

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_TEST
  ಬಳಸದ_PARAMETER(flags);
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ( (flags&0x0F)==SQLITE_SYNC_ತುಂಬಿದ ){
    sqlite3_fullsync_ಎಣಿಕೆ++;
  }
  sqlite3_sync_ಎಣಿಕೆ++;
#ಆದರೆ_ಅಂತ್ಯ

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_NO_SYNC
  OSTRACE(("SYNC-NOP pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile, pFile->h));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
#ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  ಆದರೆ( pFile->pMapRegion ){
    ಆದರೆ( os_Flush_View_ದ_File(pFile->pMapRegion, 0) ){
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_OK\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
    }ಇಲ್ಲದಿದ್ದರೆ{
      pFile->ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
      OSTRACE(("SYNC-MMAP pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(),
               pFile, pFile->pMapRegion));
      ಹಿಂತಿರುಗಿ winLogError(SQLITE_IOERR_MMAP, pFile->ಕೊನೆಯ_Errno,
                         "winSync1", pFile->z_ಮಾರ್ಗ);
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
  rc = osFlushFileBuffers(pFile->h);
  SimulateIOError( rc=FALSE );
  ಆದರೆ( rc ){
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_OK\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pFile->ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
    OSTRACE(("SYNC pid=%lu, pFile=%p, file=%p, rc=SQLITE_IOERR_FSYNC\n",
             osGetCurrentProcessId(), pFile, pFile->h));
    ಹಿಂತಿರುಗಿ winLogError(SQLITE_IOERR_FSYNC, pFile->ಕೊನೆಯ_Errno,
                       "winSync2", pFile->z_ಮಾರ್ಗ);
  }
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Determine the current size of a file in bytes
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_File_ಗಾತ್ರ(sqlite3_file *id, sqlite3_int64 *p_ಗಾತ್ರ){
  winFile *pFile = (winFile*)id;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  ಪ್ರತಿಜ್ಞಾ( id!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಗಾತ್ರ!=0 );
  SimulateIOError(ಹಿಂತಿರುಗಿ SQLITE_IOERR_FSTAT);
  OSTRACE(("SIZE file=%p, pSize=%p\n", pFile->h, p_ಗಾತ್ರ));
  {
    LARGE_INTEGER x;
    ಆದರೆ( os_Get_File_ಗಾತ್ರ_Ex(pFile->h, &x)==0 ){
      *p_ಗಾತ್ರ = 0;
      pFile->ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
      rc = winLogError(SQLITE_IOERR_FSTAT, pFile->ಕೊನೆಯ_Errno,
                       "winFileSize", pFile->z_ಮಾರ್ಗ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      *p_ಗಾತ್ರ = x.QuadPart;
    }
  }
  OSTRACE(("SIZE file=%p, pSize=%p, *pSize=%lld, rc=%s\n",
           pFile->h, p_ಗಾತ್ರ, *p_ಗಾತ್ರ, sqlite3ErrName(rc)));
  ಹಿಂತಿರುಗಿ rc;
}

/*
** LOCKFILE_FAIL_IMMEDIATELY is undefined on some Windows systems.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LOCKFILE_FAIL_IMMEDIATELY
# define LOCKFILE_FAIL_IMMEDIATELY 1
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LOCKFILE_EXCLUSIVE_ಬೀಗ
# define LOCKFILE_EXCLUSIVE_ಬೀಗ 2
#ಆದರೆ_ಅಂತ್ಯ

/*
** Historically, SQLite has used both the LockFile and LockFileEx functions.
** When the LockFile function was used, it was always expected to fail
** immediately if the lock could not be obtained.  Also, it always expected to
** obtain an exclusive lock.  These flags are used with the LockFileEx function
** and reflect those expectations; therefore, they should not be changed.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_LOCKFILE_FLAGS
# define SQLITE_LOCKFILE_FLAGS   (LOCKFILE_FAIL_IMMEDIATELY | \
                                  LOCKFILE_EXCLUSIVE_ಬೀಗ)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Currently, SQLite never calls the LockFileEx function without wanting the
** call to fail immediately if the lock cannot be obtained.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_LOCKFILEEX_FLAGS
# define SQLITE_LOCKFILEEX_FLAGS (LOCKFILE_FAIL_IMMEDIATELY)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Acquire a reader lock.
** Different API routines are called depending on whether or not this
** is Win9x or WinNT.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Get_Read_ಬೀಗ(winFile *pFile, ಪೂರ್ಣಾಂಕ b_ಖಂಡ){
  ಪೂರ್ಣಾಂಕ res;
  DWORD ಮುಖವಾಡ = ~(b_ಖಂಡ ? LOCKFILE_FAIL_IMMEDIATELY : 0);
  OSTRACE(("READ-LOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = win_ಬೀಗ_File(&pFile->h, SQLITE_LOCKFILEEX_FLAGS&ಮುಖವಾಡ, SHARED_ಮೊದಲ, 0,
                    SHARED_ಗಾತ್ರ, 0);
  ಆದರೆ( res == 0 ){
    pFile->ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
    /* No need to log a failure to lock */
  }
  OSTRACE(("READ-LOCK file=%p, result=%d\n", pFile->h, res));
  ಹಿಂತಿರುಗಿ res;
}

/*
** Undo a readlock
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Unlock_Read_ಬೀಗ(winFile *pFile){
  ಪೂರ್ಣಾಂಕ res;
  DWORD ಕೊನೆಯ_Errno;
  OSTRACE(("READ-UNLOCK file=%p, lock=%d\n", pFile->h, pFile->locktype));
  res = winUnlockFile(&pFile->h, SHARED_ಮೊದಲ, 0, SHARED_ಗಾತ್ರ, 0);
  ಆದರೆ( res==0 && ((ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error())!=ERROR_NOT_LOCKED) ){
    pFile->ಕೊನೆಯ_Errno = ಕೊನೆಯ_Errno;
    winLogError(SQLITE_IOERR_UNLOCK, pFile->ಕೊನೆಯ_Errno,
                "winUnlockReadLock", pFile->z_ಮಾರ್ಗ);
  }
  OSTRACE(("READ-UNLOCK file=%p, result=%d\n", pFile->h, res));
  ಹಿಂತಿರುಗಿ res;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_ಬೀಗ(sqlite3_file *id, ಪೂರ್ಣಾಂಕ locktype){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;    /* Return code from subroutines */
  ಪೂರ್ಣಾಂಕ res = 1;           /* Result of a Windows lock call */
  ಪೂರ್ಣಾಂಕ newLocktype;       /* Set pFile->locktype to this value before exiting */
  ಪೂರ್ಣಾಂಕ got_Pending_ಬೀಗ = 0;/* True if we acquired a PENDING lock this time */
  winFile *pFile = (winFile*)id;
  DWORD ಕೊನೆಯ_Errno = NO_ERROR;

  ಪ್ರತಿಜ್ಞಾ( id!=0 );
  OSTRACE(("LOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->shared_ಬೀಗ_Byte, locktype));

  /* If there is already a lock of this type or more restrictive on the
  ** OsFile, do nothing. Don't use the end_lock: exit path, as
  ** sqlite3OsEnterMutex() hasn't been called yet.
  */
  ಆದರೆ( pFile->locktype>=locktype ){
    OSTRACE(("LOCK-HELD file=%p, rc=SQLITE_OK\n", pFile->h));
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }

  /* Do not allow any kind of write-lock on a read-only database
  */
  ಆದರೆ( (pFile->ctrlFlags & WINFILE_RDONLY)!=0 && locktype>=RESERVED_ಬೀಗ ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_ಬೀಗ;
  }

  /* Make sure the locking sequence is correct
  */
  ಪ್ರತಿಜ್ಞಾ( pFile->locktype!=NO_ಬೀಗ || locktype==SHARED_ಬೀಗ );
  ಪ್ರತಿಜ್ಞಾ( locktype!=PENDING_ಬೀಗ );
  ಪ್ರತಿಜ್ಞಾ( locktype!=RESERVED_ಬೀಗ || pFile->locktype==SHARED_ಬೀಗ );

  /* Lock the PENDING_LOCK byte if we need to acquire an EXCLUSIVE lock or
  ** a SHARED lock.  If we are acquiring a SHARED lock, the acquisition of
  ** the PENDING_LOCK byte is temporary.
  */
  newLocktype = pFile->locktype;
  ಆದರೆ( locktype==SHARED_ಬೀಗ
   || (locktype==EXCLUSIVE_ಬೀಗ && pFile->locktype==RESERVED_ಬೀಗ)
  ){
    ಪೂರ್ಣಾಂಕ cnt = 3;

    /* Flags for the LockFileEx() call. This should be an exclusive lock if
    ** this call is to obtain EXCLUSIVE, or a shared lock if this call is to
    ** obtain SHARED.  */
    ಪೂರ್ಣಾಂಕ flags = LOCKFILE_FAIL_IMMEDIATELY;
    ಆದರೆ( locktype==EXCLUSIVE_ಬೀಗ ){
      flags |= LOCKFILE_EXCLUSIVE_ಬೀಗ;
    }
    ಆಗ( cnt>0 ){
      /* Try 3 times to get the pending lock.  This is needed to work
      ** around problems caused by indexing and/or anti-virus software on
      ** Windows systems.
      **
      ** If you are using this code as a model for alternative VFSes, do not
      ** copy this retry logic.  It is a hack intended for Windows only.  */
      res = win_ಬೀಗ_File(&pFile->h, flags, PENDING_BYTE, 0, 1, 0);
      ಆದರೆ( res ) ನಿಲ್ಲಿಸು;

      ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
      OSTRACE(("LOCK-PENDING-FAIL file=%p, count=%d, result=%d\n", 
            pFile->h, cnt, res
      ));

      ಆದರೆ( ಕೊನೆಯ_Errno==ERROR_INVALID_HANDLE ){
        pFile->ಕೊನೆಯ_Errno = ಕೊನೆಯ_Errno;
        rc = SQLITE_IOERR_ಬೀಗ;
        OSTRACE(("LOCK-FAIL file=%p, count=%d, rc=%s\n", 
              pFile->h, cnt, sqlite3ErrName(rc)
        ));
        ಹಿಂತಿರುಗಿ rc;
      }

      cnt--;
      ಆದರೆ( cnt>0 ) osSleep(1);
    }
    got_Pending_ಬೀಗ = res;
  }

  /* Acquire a shared lock
  */
  ಆದರೆ( locktype==SHARED_ಬೀಗ && res ){
    ಪ್ರತಿಜ್ಞಾ( pFile->locktype==NO_ಬೀಗ );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
    res = win_Get_Read_ಬೀಗ(pFile, pFile->b_ಖಂಡ_On_Connect);
#ಇಲ್ಲದಿದ್ದರೆ
    res = win_Get_Read_ಬೀಗ(pFile, 0);
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( res ){
      newLocktype = SHARED_ಬೀಗ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
    }
  }

  /* Acquire a RESERVED lock
  */
  ಆದರೆ( locktype==RESERVED_ಬೀಗ && res ){
    ಪ್ರತಿಜ್ಞಾ( pFile->locktype==SHARED_ಬೀಗ );
    res = win_ಬೀಗ_File(&pFile->h, SQLITE_LOCKFILE_FLAGS, RESERVED_BYTE, 0, 1, 0);
    ಆದರೆ( res ){
      newLocktype = RESERVED_ಬೀಗ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
    }
  }

  /* Acquire a PENDING lock
  */
  ಆದರೆ( locktype==EXCLUSIVE_ಬೀಗ && res ){
    newLocktype = PENDING_ಬೀಗ;
    got_Pending_ಬೀಗ = 0;
  }

  /* Acquire an EXCLUSIVE lock
  */
  ಆದರೆ( locktype==EXCLUSIVE_ಬೀಗ && res ){
    ಪ್ರತಿಜ್ಞಾ( pFile->locktype>=SHARED_ಬೀಗ );
    (ಶೂನ್ಯ)win_Unlock_Read_ಬೀಗ(pFile);
    res = win_ಬೀಗ_File(&pFile->h, SQLITE_LOCKFILE_FLAGS, SHARED_ಮೊದಲ, 0,
                      SHARED_ಗಾತ್ರ, 0);
    ಆದರೆ( res ){
      newLocktype = EXCLUSIVE_ಬೀಗ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
      win_Get_Read_ಬೀಗ(pFile, 0);
    }
  }

  /* If we are holding a PENDING lock that ought to be released, then
  ** release it now.
  */
  ಆದರೆ( got_Pending_ಬೀಗ && locktype==SHARED_ಬೀಗ ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }

  /* Update the state of the lock has held in the file descriptor then
  ** return the appropriate result code.
  */
  ಆದರೆ( res ){
    rc = SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pFile->ಕೊನೆಯ_Errno = ಕೊನೆಯ_Errno;
    rc = SQLITE_BUSY;
    OSTRACE(("LOCK-FAIL file=%p, wanted=%d, got=%d\n",
             pFile->h, locktype, newLocktype));
  }
  pFile->locktype = (u8)newLocktype;
  OSTRACE(("LOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, return
** non-zero, otherwise zero.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Check_Reserved_ಬೀಗ(sqlite3_file *id, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  ಪೂರ್ಣಾಂಕ res;
  winFile *pFile = (winFile*)id;

  SimulateIOError( ಹಿಂತಿರುಗಿ SQLITE_IOERR_CHECKRESERVEDLOCK; );
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p\n", pFile->h, p_Res_ಹೊರಹೋಗು));

  ಪ್ರತಿಜ್ಞಾ( id!=0 );
  ಆದರೆ( pFile->locktype>=RESERVED_ಬೀಗ ){
    res = 1;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (local)\n", pFile->h, res));
  }ಇಲ್ಲದಿದ್ದರೆ{
    res = win_ಬೀಗ_File(&pFile->h, SQLITE_LOCKFILEEX_FLAGS,RESERVED_BYTE,0,1,0);
    ಆದರೆ( res ){
      winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
    }
    res = !res;
    OSTRACE(("TEST-WR-LOCK file=%p, result=%d (remote)\n", pFile->h, res));
  }
  *p_Res_ಹೊರಹೋಗು = res;
  OSTRACE(("TEST-WR-LOCK file=%p, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           pFile->h, p_Res_ಹೊರಹೋಗು, *p_Res_ಹೊರಹೋಗು));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winUnlock(sqlite3_file *id, ಪೂರ್ಣಾಂಕ locktype){
  ಪೂರ್ಣಾಂಕ type;
  winFile *pFile = (winFile*)id;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪ್ರತಿಜ್ಞಾ( pFile!=0 );
  ಪ್ರತಿಜ್ಞಾ( locktype<=SHARED_ಬೀಗ );
  OSTRACE(("UNLOCK file=%p, oldLock=%d(%d), newLock=%d\n",
           pFile->h, pFile->locktype, pFile->shared_ಬೀಗ_Byte, locktype));
  type = pFile->locktype;
  ಆದರೆ( type>=EXCLUSIVE_ಬೀಗ ){
    winUnlockFile(&pFile->h, SHARED_ಮೊದಲ, 0, SHARED_ಗಾತ್ರ, 0);
    ಆದರೆ( locktype==SHARED_ಬೀಗ && !win_Get_Read_ಬೀಗ(pFile, 0) ){
      /* This should never happen.  We should always be able to
      ** reacquire the read lock */
      rc = winLogError(SQLITE_IOERR_UNLOCK, os_Get_ಕೊನೆಯ_Error(),
                       "winUnlock", pFile->z_ಮಾರ್ಗ);
    }
  }
  ಆದರೆ( type>=RESERVED_ಬೀಗ ){
    winUnlockFile(&pFile->h, RESERVED_BYTE, 0, 1, 0);
  }
  ಆದರೆ( locktype==NO_ಬೀಗ && type>=SHARED_ಬೀಗ ){
    win_Unlock_Read_ಬೀಗ(pFile);
  }
  ಆದರೆ( type>=PENDING_ಬೀಗ ){
    winUnlockFile(&pFile->h, PENDING_BYTE, 0, 1, 0);
  }
  pFile->locktype = (u8)locktype;
  OSTRACE(("UNLOCK file=%p, lock=%d, rc=%s\n",
           pFile->h, pFile->locktype, sqlite3ErrName(rc)));
  ಹಿಂತಿರುಗಿ rc;
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

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Nolock_ಬೀಗ(sqlite3_file *id, ಪೂರ್ಣಾಂಕ locktype){
  ಬಳಸದ_PARAMETER(id);
  ಬಳಸದ_PARAMETER(locktype);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Nolock_Check_Reserved_ಬೀಗ(sqlite3_file *id, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  ಬಳಸದ_PARAMETER(id);
  ಬಳಸದ_PARAMETER(p_Res_ಹೊರಹೋಗು);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winNolockUnlock(sqlite3_file *id, ಪೂರ್ಣಾಂಕ locktype){
  ಬಳಸದ_PARAMETER(id);
  ಬಳಸದ_PARAMETER(locktype);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/******************* End of the no-op lock implementation *********************
******************************************************************************/

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
ಸ್ಥಿರ ಶೂನ್ಯ win_ಬಹುಲಕ_ಬಿಟ್(winFile *pFile, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಮುಖವಾಡ, ಪೂರ್ಣಾಂಕ *pArg){
  ಆದರೆ( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & ಮುಖವಾಡ)!=0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (*pArg)==0 ){
    pFile->ctrlFlags &= ~ಮುಖವಾಡ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pFile->ctrlFlags |= ಮುಖವಾಡ;
  }
}

/* Forward references to VFS helper methods used for temporary files */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winGetTempname(sqlite3_vfs *, ಅಕ್ಷರ **);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winIsDir(LPCWSTR);
ಸ್ಥಿರ BOOL win_Is_Long_ಮಾರ್ಗ_ಪೂರ್ವಪ್ರತ್ಯಯ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
ಸ್ಥಿರ BOOL winIsDriveLetterAndColon(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);

/*
** Control and query of the open file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winFileControl(sqlite3_file *id, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  winFile *pFile = (winFile*)id;
  OSTRACE(("FCNTL file=%p, op=%d, pArg=%p\n", pFile->h, op, pArg));
  ಆಯ್ಕೆ( op ){
    ಸ್ಥಿತಿ SQLITE_FCNTL_LOCKSTATE: {
      *(ಪೂರ್ಣಾಂಕ*)pArg = pFile->locktype;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಸ್ಥಿತಿ SQLITE_FCNTL_ಕೊನೆಯ_ERRNO: {
      *(ಪೂರ್ಣಾಂಕ*)pArg = (ಪೂರ್ಣಾಂಕ)pFile->ಕೊನೆಯ_Errno;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಸ್ಥಿತಿ SQLITE_FCNTL_CHUNK_ಗಾತ್ರ: {
      pFile->szChunk = *(ಪೂರ್ಣಾಂಕ *)pArg;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಸ್ಥಿತಿ SQLITE_FCNTL_ಗಾತ್ರ_HINT: {
      ಆದರೆ( pFile->szChunk>0 ){
        sqlite3_int64 oldSz;
        ಪೂರ್ಣಾಂಕ rc = win_File_ಗಾತ್ರ(id, &oldSz);
        ಆದರೆ( rc==SQLITE_OK ){
          sqlite3_int64 newSz = *(sqlite3_int64*)pArg;
          ಆದರೆ( newSz>oldSz ){
            SimulateIOErrorBenign(1);
            rc = winTruncate(id, newSz);
            SimulateIOErrorBenign(0);
          }
        }
        OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
        ಹಿಂತಿರುಗಿ rc;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಸ್ಥಿತಿ SQLITE_FCNTL_PERSIST_WAL: {
      win_ಬಹುಲಕ_ಬಿಟ್(pFile, WINFILE_PERSIST_WAL, (ಪೂರ್ಣಾಂಕ*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಸ್ಥಿತಿ SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      win_ಬಹುಲಕ_ಬಿಟ್(pFile, WINFILE_PSOW, (ಪೂರ್ಣಾಂಕ*)pArg);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಸ್ಥಿತಿ SQLITE_FCNTL_VFSNAME: {
      *(ಅಕ್ಷರ**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಸ್ಥಿತಿ SQLITE_FCNTL_WIN32_AV_RETRY: {
      ಪೂರ್ಣಾಂಕ *a = (ಪೂರ್ಣಾಂಕ*)pArg;
      ಆದರೆ( a[0]>0 ){
        winIoerrRetry = a[0];
      }ಇಲ್ಲದಿದ್ದರೆ{
        a[0] = winIoerrRetry;
      }
      ಆದರೆ( a[1]>0 ){
        winIoerrRetryDelay = a[1];
      }ಇಲ್ಲದಿದ್ದರೆ{
        a[1] = winIoerrRetryDelay;
      }
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಸ್ಥಿತಿ SQLITE_FCNTL_WIN32_GET_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      *phFile = pFile->h;
      OSTRACE(("FCNTL file=%p, rc=SQLITE_OK\n", pFile->h));
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
    ಸ್ಥಿತಿ SQLITE_FCNTL_WIN32_ಗಣ_HANDLE: {
      LPHANDLE phFile = (LPHANDLE)pArg;
      HANDLE hOldFile = pFile->h;
      pFile->h = *phFile;
      *phFile = hOldFile;
      OSTRACE(("FCNTL oldFile=%p, newFile=%p, rc=SQLITE_OK\n",
               hOldFile, pFile->h));
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ SQLITE_FCNTL_NULL_IO: {
      (ಶೂನ್ಯ)osCloseHandle(pFile->h);
      pFile->h = ಖಾಲಿ;
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಸ್ಥಿತಿ SQLITE_FCNTL_TEMPFILENAME: {
      ಅಕ್ಷರ *zTFile = 0;
      ಪೂರ್ಣಾಂಕ rc = winGetTempname(pFile->pVfs, &zTFile);
      ಆದರೆ( rc==SQLITE_OK ){
        *(ಅಕ್ಷರ**)pArg = zTFile;
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      ಹಿಂತಿರುಗಿ rc;
    }
#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
    ಸ್ಥಿತಿ SQLITE_FCNTL_MMAP_ಗಾತ್ರ: {
      i64 new_ಎಲ್ಲೆ = *(i64*)pArg;
      ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
      ಆದರೆ( new_ಎಲ್ಲೆ>sqlite3GlobalConfig.mxMmap ){
        new_ಎಲ್ಲೆ = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (SIZE_T) and passed
      ** to MapViewOfFile(). Restrict its value to 2GB if (SIZE_T) is not at
      ** least a 64-bit type. */
      ಆದರೆ( new_ಎಲ್ಲೆ>0 && ಗಾತ್ರ(ಗಾತ್ರ_T)<8 ){
        new_ಎಲ್ಲೆ = (new_ಎಲ್ಲೆ & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmap_ಗಾತ್ರ_Max;
      ಆದರೆ( new_ಎಲ್ಲೆ>=0 && new_ಎಲ್ಲೆ!=pFile->mmap_ಗಾತ್ರ_Max && pFile->n_Fetch_ಹೊರಹೋಗು==0 ){
        pFile->mmap_ಗಾತ್ರ_Max = new_ಎಲ್ಲೆ;
        ಆದರೆ( pFile->mmap_ಗಾತ್ರ>0 ){
          winUnmapfile(pFile);
          rc = winMapfile(pFile, -1);
        }
      }
      OSTRACE(("FCNTL file=%p, rc=%s\n", pFile->h, sqlite3ErrName(rc)));
      ಹಿಂತಿರುಗಿ rc;
    }
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
    ಸ್ಥಿತಿ SQLITE_FCNTL_ಬೀಗ_TIMEOUT: {
      ಪೂರ್ಣಾಂಕ iOld = pFile->iBusyTimeout;
      ಪೂರ್ಣಾಂಕ iNew = *(ಪೂರ್ಣಾಂಕ*)pArg;
#ಆದರೆ SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = (iNew < 0) ? INFINITE : (DWORD)iNew;
#ಅಥವಾಆದರೆ SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = (DWORD)(!!iNew);
#ಇಲ್ಲದಿದ್ದರೆ
# error "SQLITE_ENABLE_SETLK_TIMEOUT must be set to 1 or 2"
#ಆದರೆ_ಅಂತ್ಯ
      *(ಪೂರ್ಣಾಂಕ*)pArg = iOld;
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಸ್ಥಿತಿ SQLITE_FCNTL_ಖಂಡ_ON_CONNECT: {
      ಪೂರ್ಣಾಂಕ iNew = *(ಪೂರ್ಣಾಂಕ*)pArg;
      pFile->b_ಖಂಡ_On_Connect = iNew;
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_SETLK_TIMEOUT */

#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    ಸ್ಥಿತಿ SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      sqlite3_str_appendf(pStr, "{\"h\":%llu", (sqlite3_uint64)pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      ಆದರೆ( pFile->locktype ){
        ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *az_ಬೀಗ[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
        sqlite3_str_appendf(pStr, ",\"locktype\":\"%s\"", 
                                  az_ಬೀಗ[pFile->locktype-1]);
      }
#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
      ಆದರೆ( pFile->mmap_ಗಾತ್ರ ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmap_ಗಾತ್ರ);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->n_Fetch_ಹೊರಹೋಗು);
      }
#ಆದರೆ_ಅಂತ್ಯ
      sqlite3_str_append(pStr, "}", 1);
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

  }
  OSTRACE(("FCNTL file=%p, rc=SQLITE_NOTFOUND\n", pFile->h));
  ಹಿಂತಿರುಗಿ SQLITE_NOTFOUND;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Sector_ಗಾತ್ರ(sqlite3_file *id){
  (ಶೂನ್ಯ)id;
  ಹಿಂತಿರುಗಿ SQLITE_DEFAULT_SECTOR_ಗಾತ್ರ;
}

/*
** Return a vector of device characteristics.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winDeviceCharacteristics(sqlite3_file *id){
  winFile *p = (winFile*)id;
  ಹಿಂತಿರುಗಿ SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN | SQLITE_IOCAP_SUBPAGE_READ |
         ((p->ctrlFlags & WINFILE_PSOW)?SQLITE_IOCAP_POWERSAFE_OVERWRITE:0);
}

/*
** Windows will only let you create file view mappings
** on allocation size granularity boundaries.
** During sqlite3_os_init() we do a GetSystemInfo()
** to get the granularity size.
*/
ಸ್ಥಿರ SYSTEM_INFO winSysInfo;

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
ಸ್ಥಿರ LPWSTR winConvertFromUtf8Filename(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename){
  LPWSTR zConverted = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __CYGWIN__
  ಪೂರ್ಣಾಂಕ nChar;

  ಆದರೆ( os_Cygwin_conv_ಮಾರ್ಗ && !(winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2])) ){
    i64 nByte;
    ಪೂರ್ಣಾಂಕ convertflag = CCP_POSIX_ಗಮ್ಯ_WIN_W;
    ಆದರೆ( !strchr(zFilename, '/') ) convertflag |= CCP_RELATIVE;
    nByte = (i64)os_Cygwin_conv_ಮಾರ್ಗ(convertflag,
        zFilename, 0, 0);
    ಆದರೆ( nByte>0 ){
      zConverted = sqlite3MallocZero(12+(u64)nByte);
      ಆದರೆ ( zConverted==0 ){
        ಹಿಂತಿರುಗಿ zConverted;
      }
      /* Filenames should be prefixed, except when converted
       * full path already starts with "\\?\". */
      ಆದರೆ( os_Cygwin_conv_ಮಾರ್ಗ(convertflag, zFilename,
                           zConverted+4, nByte)==0 ){
        ಆದರೆ( (convertflag&CCP_RELATIVE) ){
          memmove(zConverted, zConverted+4, nByte);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( memcmp(zConverted+4, L"\\\\", 4) ){
          ಸ್ಮರಣೆ_ನಕಲು(zConverted, L"\\\\?\\", 8);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( zConverted[6]!='?' ){
          memmove(zConverted+6, zConverted+4, nByte);
          ಸ್ಮರಣೆ_ನಕಲು(zConverted, L"\\\\?\\UNC", 14);
        }ಇಲ್ಲದಿದ್ದರೆ{
          memmove(zConverted, zConverted+4, nByte);
        }
        ಹಿಂತಿರುಗಿ zConverted;
      }
      sqlite3_free(zConverted);
    }
  }
  nChar = os_Multi_Byte_ಗಮ್ಯ_Wide_Char(CP_UTF8, 0, zFilename, -1, ಖಾಲಿ, 0);
  ಆದರೆ( nChar==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  zConverted = sqlite3MallocZero( ((u64)nChar+6)*ಗಾತ್ರ(WCHAR) );
  ಆದರೆ( zConverted==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  nChar = os_Multi_Byte_ಗಮ್ಯ_Wide_Char(CP_UTF8, 0, zFilename, -1,
                                zConverted, nChar);
  ಆದರೆ( nChar==0 ){
    sqlite3_free(zConverted);
    zConverted = 0;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nChar>MAX_ಮಾರ್ಗ
      && winIsDriveLetterAndColon(zFilename)
      && winIsDirSep(zFilename[2]) ){
    memmove(zConverted+4, zConverted, nChar*ಗಾತ್ರ(WCHAR));
    zConverted[2] = '\\';
    ಸ್ಮರಣೆ_ನಕಲು(zConverted, L"\\\\?\\", 8);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nChar>MAX_ಮಾರ್ಗ
      && winIsDirSep(zFilename[0]) && winIsDirSep(zFilename[1])
      && zFilename[2] != '?' ){
    memmove(zConverted+6, zConverted, nChar*ಗಾತ್ರ(WCHAR));
    ಸ್ಮರಣೆ_ನಕಲು(zConverted, L"\\\\?\\UNC", 14);
  }
#ಇಲ್ಲದಿದ್ದರೆ /* if !defined(__CYGWIN__) */
  zConverted = win_Utf8_ಗಮ್ಯ_Unicode(zFilename);
#ಆದರೆ_ಅಂತ್ಯ /* __CYGWIN__ */
  /* caller will handle out of memory */
  ಹಿಂತಿರುಗಿ zConverted;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL

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
ಸ್ಥಿರ sqlite3_mutex *win_Big_ಬೀಗ = 0;
ಸ್ಥಿರ ಶೂನ್ಯ winShmEnterMutex(ಶೂನ್ಯ){
  sqlite3_mutex_enter(win_Big_ಬೀಗ);
}
ಸ್ಥಿರ ಶೂನ್ಯ winShmLeaveMutex(ಶೂನ್ಯ){
  sqlite3_mutex_leave(win_Big_ಬೀಗ);
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winShmMutexHeld(ಶೂನ್ಯ) {
  ಹಿಂತಿರುಗಿ sqlite3_mutex_held(win_Big_ಬೀಗ);
}
#ಆದರೆ_ಅಂತ್ಯ

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
ರಚನೆ win_Shm_ಗಂಟು {
  sqlite3_mutex *mutex;      /* Mutex to access this object */
  ಅಕ್ಷರ *zFilename;           /* Name of the file */
  HANDLE hSharedShm;         /* File handle open on zFilename */
  ಪೂರ್ಣಾಂಕ b_Use_Shared_ಬೀಗ_Handle;  /* True to use hSharedShm for everything */

  ಪೂರ್ಣಾಂಕ isUnlocked;            /* DMS lock has not yet been obtained */
  ಪೂರ್ಣಾಂಕ isReadonly;            /* True if read-only */
  ಪೂರ್ಣಾಂಕ szRegion;              /* Size of shared-memory regions */
  ಪೂರ್ಣಾಂಕ nRegion;               /* Size of array apRegion */

  ರಚನೆ ShmRegion {
    HANDLE hMap;             /* File handle from CreateFileMapping */
    ಶೂನ್ಯ *pMap;
  } *aRegion;
  DWORD ಕೊನೆಯ_Errno;           /* The Windows errno from the last I/O error */

  winShm *pWinShmList;       /* List of winShm objects with ptrs to this */

  win_Shm_ಗಂಟು *p_ಮುಂದಿನ;         /* Next in list of all winShmNode objects */
#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 ಮುಂದಿನ_Shm_Id;              /* Next available winShm.id value */
#ಆದರೆ_ಅಂತ್ಯ
};

/*
** A global array of all winShmNode objects.
**
** The winShmMutexHeld() must be true while reading or writing this list.
*/
ಸ್ಥಿರ win_Shm_ಗಂಟು *win_Shm_ಗಂಟು_List = 0;

/*
** Structure used internally by this VFS to record the state of an
** open shared memory connection. There is one such structure for each
** winFile open on a wal mode database.
*/
ರಚನೆ winShm {
  win_Shm_ಗಂಟು *p_Shm_ಗಂಟು;      /* The underlying winShmNode object */
  u16 shared_ಮುಖವಾಡ;            /* Mask of shared locks held */
  u16 excl_ಮುಖವಾಡ;              /* Mask of exclusive locks held */
  HANDLE hShm;               /* File-handle on *-shm file. For locking. */
  ಪೂರ್ಣಾಂಕ bReadonly;             /* True if hShm is opened read-only */
#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
  u8 id;                     /* Id of this connection with its winShmNode */
#ಆದರೆ_ಅಂತ್ಯ
  winShm *p_Win_Shm_ಮುಂದಿನ;       /* Next winShm object on same winShmNode */
};

/*
** Constants used for locking
*/
#ವ್ಯಾಖ್ಯಾನಿಸು WIN_SHM_ತಳ   ((22+SQLITE_SHM_NLOCK)*4)        /* first lock byte */
#ವ್ಯಾಖ್ಯಾನಿಸು WIN_SHM_DMS    (WIN_SHM_ತಳ+SQLITE_SHM_NLOCK)  /* deadman switch */

/* Forward references to VFS methods */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winOpen(sqlite3_vfs*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,sqlite3_file*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winDelete(sqlite3_vfs *,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ);

/*
** Purge the winShmNodeList list of all entries with winShmNode.pWinShmList==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
ಸ್ಥಿರ ಶೂನ್ಯ winShmPurge(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ deleteFlag){
  win_Shm_ಗಂಟು **pp;
  win_Shm_ಗಂಟು *p;
  ಪ್ರತಿಜ್ಞಾ( winShmMutexHeld() );
  OSTRACE(("SHM-PURGE pid=%lu, deleteFlag=%d\n",
           osGetCurrentProcessId(), deleteFlag));
  pp = &win_Shm_ಗಂಟು_List;
  ಆಗ( (p = *pp)!=0 ){
    ಆದರೆ( p->pWinShmList==0 ){
      ಪೂರ್ಣಾಂಕ i;
      ಆದರೆ( p->mutex ){ sqlite3_mutex_free(p->mutex); }
      ಗಾಗಿ(i=0; i<p->nRegion; i++){
        BOOL bRc = os_Unmap_View_ದ_File(p->aRegion[i].pMap);
        OSTRACE(("SHM-PURGE-UNMAP pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        ಬಳಸದ_VARIABLE_ಮೌಲ್ಯ(bRc);
        bRc = osCloseHandle(p->aRegion[i].hMap);
        OSTRACE(("SHM-PURGE-CLOSE pid=%lu, region=%d, rc=%s\n",
                 osGetCurrentProcessId(), i, bRc ? "ok" : "failed"));
        ಬಳಸದ_VARIABLE_ಮೌಲ್ಯ(bRc);
      }
      winHandleClose(p->hSharedShm);
      ಆದರೆ( deleteFlag ){
        SimulateIOErrorBenign(1);
        sqlite3BeginBenignMalloc();
        winDelete(pVfs, p->zFilename, 0);
        sqlite3_ಕೊನೆ_Benign_Malloc();
        SimulateIOErrorBenign(0);
      }
      *pp = p->p_ಮುಂದಿನ;
      sqlite3_free(p->aRegion);
      sqlite3_free(p);
    }ಇಲ್ಲದಿದ್ದರೆ{
      pp = &p->p_ಮುಂದಿನ;
    }
  }
}

/*
** Handle h is open on the *-shm file. This function zeroes the first
** 8 bytes of the file. If SQLITE_ENABLE_SETLK_TIMEOUT is defined, h may 
** have been opened with FILE_FLAG_OVERLAPPED.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Zero_Shared_ಸ್ಮರಣೆ(HANDLE h){
  u8 aZero[8] = {0,0,0,0,0,0,0,0};
  DWORD nWrite = 0;
  OVERLAPPED ovlp;
  memset(&ovlp, 0, ಗಾತ್ರ(OVERLAPPED));
  ಆದರೆ( !osWriteFile(h, aZero, ಗಾತ್ರ(aZero), &nWrite, &ovlp) ){
    /* If the file was opened with FILE_FLAG_OVERLAPPED, the write may still
    ** be pending. Wait for it with GetOverlappedResult().  */
    ಆದರೆ( os_Get_ಕೊನೆಯ_Error()!=ERROR_IO_PENDING
     || !Get_Overlapped_ಫಲಿತಾಂಶ(h, &ovlp, &nWrite, TRUE)
    ){
      nWrite = 0;
    }
  }
  ಹಿಂತಿರುಗಿ (nWrite==ಗಾತ್ರ(aZero) ? SQLITE_OK : SQLITE_IOERR_WRITE);
}

/*
** The DMS lock has not yet been taken on the shm file associated with
** pShmNode. Take the lock. Truncate the *-shm file if required.
** Return SQLITE_OK if successful, or an SQLite error code otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_ಬೀಗ_Shared_ಸ್ಮರಣೆ(win_Shm_ಗಂಟು *p_Shm_ಗಂಟು, DWORD nMs){
  HANDLE h = p_Shm_ಗಂಟು->hSharedShm;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_Shm_ಗಂಟು->mutex) );
  rc = win_Handle_ಬೀಗ_Timeout(h, WIN_SHM_DMS, 1, 1, 0);
  ಆದರೆ( rc==SQLITE_OK ){
    /* We have an EXCLUSIVE lock on the DMS byte. This means that this
    ** is the first process to open the file. Truncate it to zero bytes
    ** in this case.  */
    ಆದರೆ( p_Shm_ಗಂಟು->isReadonly ){
      rc = SQLITE_READONLY_CANTINIT;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( SQLITE_OK!=winHandleTruncate(h, 0) ){
        /* Sometimes the truncate operation may fail because some other
        ** process is still holding an open mapping. So try to write
        ** zeroes into the start of the file instead. */
        rc = win_Zero_Shared_ಸ್ಮರಣೆ(h);
      }
    }

    /* Release the EXCLUSIVE lock acquired above. */
    winUnlockFile(&h, WIN_SHM_DMS, 0, 1, 0);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (rc & 0xFF)==SQLITE_BUSY ){
    rc = SQLITE_OK;
  }

  ಆದರೆ( rc==SQLITE_OK ){
    /* Take a SHARED lock on the DMS byte. */
    rc = win_Handle_ಬೀಗ_Timeout(h, WIN_SHM_DMS, 1, 0, nMs);
    ಆದರೆ( rc==SQLITE_OK ){
      p_Shm_ಗಂಟು->isUnlocked = 0;
    }
  }

  ಹಿಂತಿರುಗಿ rc;
}


/*
** This function is used to open a handle on a *-shm file.
**
** If SQLITE_ENABLE_SETLK_TIMEOUT is defined at build time, then the file
** is opened with FILE_FLAG_OVERLAPPED specified. If not, it is not.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winHandleOpen(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zUtf8,              /* File to open */
  ಪೂರ್ಣಾಂಕ *pbReadonly,                /* IN/OUT: True for readonly handle */
  HANDLE *ph                      /* OUT: New HANDLE for file */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  LPWSTR zConverted = 0;
  ಪೂರ್ಣಾಂಕ bReadonly = *pbReadonly;
  HANDLE h = INVALID_HANDLE_ಮೌಲ್ಯ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
  ಸ್ಥಿರಾಂಕ DWORD flag_overlapped = FILE_FLAG_OVERLAPPED;
#ಇಲ್ಲದಿದ್ದರೆ
  ಸ್ಥಿರಾಂಕ DWORD flag_overlapped = 0;
#ಆದರೆ_ಅಂತ್ಯ

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8);
  ಆದರೆ( zConverted==0 ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8));
    rc = SQLITE_IOERR_NOMEM_BKPT;
    ಹೋಗು winopenfile_ಹೊರಹೋಗು;
  }

  /* Ensure the file we are trying to open is not actually a directory. */
  ಆದರೆ( winIsDir(zConverted) ){
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8));
    rc = SQLITE_CANTOPEN_ISDIR;
    ಹೋಗು winopenfile_ಹೊರಹೋಗು;
  }

  /* TODO: platforms.
  ** TODO: retry-on-ioerr.
  */
  h = osCreateFileW(zConverted,         /* lpFileName */
      (GENERIC_READ | (bReadonly ? 0 : GENERIC_WRITE)),  /* dwDesiredAccess */
      FILE_SHARE_READ | FILE_SHARE_WRITE,        /* dwShareMode */
      ಖಾಲಿ,                                      /* lpSecurityAttributes */
      OPEN_ALWAYS,                               /* dwCreationDisposition */
      FILE_ATTRIBUTE_ಸಾಮಾನ್ಯ|flag_overlapped,
      ಖಾಲಿ
  );

  ಆದರೆ( h==INVALID_HANDLE_ಮೌಲ್ಯ ){
    ಆದರೆ( bReadonly==0 ){
      bReadonly = 1;
      rc = winHandleOpen(zUtf8, &bReadonly, &h);
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = SQLITE_CANTOPEN_BKPT;
    }
  }

 winopenfile_ಹೊರಹೋಗು:
  sqlite3_free(zConverted);
  *pbReadonly = bReadonly;
  *ph = h;
  ಹಿಂತಿರುಗಿ rc;
}
 
/*
** Close pDbFd's connection to shared-memory.  Delete the underlying
** *-shm file if deleteFlag is true.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Close_Shared_ಸ್ಮರಣೆ(winFile *pDbFd, ಪೂರ್ಣಾಂಕ deleteFlag){
  winShm *p;            /* The connection to be closed */
  winShm **pp;          /* Iterator for pShmNode->pWinShmList */
  win_Shm_ಗಂಟು *p_Shm_ಗಂಟು; /* The underlying shared-memory file */

  p = pDbFd->pShm;
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  ಆದರೆ( p->hShm!=INVALID_HANDLE_ಮೌಲ್ಯ ){
    osCloseHandle(p->hShm);
  }

  winShmEnterMutex();
  p_Shm_ಗಂಟು = p->p_Shm_ಗಂಟು;

  /* Remove this connection from the winShmNode.pWinShmList list */
  sqlite3_mutex_enter(p_Shm_ಗಂಟು->mutex);
  ಗಾಗಿ(pp=&p_Shm_ಗಂಟು->pWinShmList; *pp!=p; pp=&(*pp)->p_Win_Shm_ಮುಂದಿನ){}
  *pp = p->p_Win_Shm_ಮುಂದಿನ;
  sqlite3_mutex_leave(p_Shm_ಗಂಟು->mutex);

  winShmPurge(pDbFd->pVfs, deleteFlag);
  winShmLeaveMutex();

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** testfixture builds may set this global variable to true via a
** Tcl interface. This forces the VFS to use the locking normally
** only used for UNC paths for all files.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
ಪೂರ್ಣಾಂಕ sqlite3_win_test_unc_locking = 0;
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_win_test_unc_locking 0
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winIsUNCPath(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile){
  ಆದರೆ( zFile[0]=='\\' && zFile[1]=='\\' ){
    ಆದರೆ( zFile[2]=='?'
     && zFile[3]=='\\'
     && sqlite3Isalpha(zFile[4])
     && zFile[5]==':'
     && winIsDirSep(zFile[6])
    ){
      ಹಿಂತಿರುಗಿ sqlite3_win_test_unc_locking;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಹಿಂತಿರುಗಿ sqlite3_win_test_unc_locking;
}

/*
** Return true if the string passed in is the name of a file on a
** remote (network-mounted) volume.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winIsRemoteVolume(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile){
  WCHAR z_ಬೇರು[4];
  ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(zFile, "\\\\?\\",4)==0 ) zFile += 4;
  ಆದರೆ( !sqlite3Isalpha(zFile[0]) ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( zFile[1]!=':' ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( !winIsDirSep(zFile[2]) ) ಹಿಂತಿರುಗಿ 0;
  z_ಬೇರು[0] = (WCHAR)zFile[0];
  z_ಬೇರು[1] = ':';
  z_ಬೇರು[2] = '\\';
  z_ಬೇರು[3] = 0;
  ಹಿಂತಿರುಗಿ osGetDriveTypeW(z_ಬೇರು)==DRIVE_REMOTE;
}

/*
** Open the shared-memory area associated with database file pDbFd.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Open_Shared_ಸ್ಮರಣೆ(winFile *pDbFd){
  ರಚನೆ winShm *p;                  /* The connection to be opened */
  win_Shm_ಗಂಟು *p_Shm_ಗಂಟು = 0;          /* The underlying mmapped file */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;                /* Result code */
  win_Shm_ಗಂಟು *pNew;                  /* Newly allocated winShmNode */
  ಪೂರ್ಣಾಂಕ nName;                         /* Size of zName in bytes */

  ಪ್ರತಿಜ್ಞಾ( pDbFd->pShm==0 );    /* Not previously opened */

  /* Allocate space for the new sqlite3_shm object.  Also speculatively
  ** allocate space for a new winShmNode and filename.  */
  p = sqlite3MallocZero( ಗಾತ್ರ(*p) );
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM_BKPT;
  nName = sqlite3Strlen30(pDbFd->z_ಮಾರ್ಗ);
  pNew = sqlite3MallocZero( ಗಾತ್ರ(*p_Shm_ಗಂಟು) + (i64)nName + 17 );
  ಆದರೆ( pNew==0 ){
    sqlite3_free(p);
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM_BKPT;
  }
  pNew->zFilename = (ಅಕ್ಷರ*)&pNew[1];
  pNew->hSharedShm = INVALID_HANDLE_ಮೌಲ್ಯ;
  pNew->isUnlocked = 1;
  ಆದರೆ( winIsUNCPath(pDbFd->z_ಮಾರ್ಗ) || winIsRemoteVolume(pDbFd->z_ಮಾರ್ಗ) ){
    pNew->b_Use_Shared_ಬೀಗ_Handle = 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pNew->b_Use_Shared_ಬೀಗ_Handle = 0;
  }
  sqlite3_snprintf(nName+15, pNew->zFilename, "%s-shm", pDbFd->z_ಮಾರ್ಗ);
  sqlite3FileSuffix3(pDbFd->z_ಮಾರ್ಗ, pNew->zFilename);

  /* Look to see if there is an existing winShmNode that can be used.
  ** If no matching winShmNode currently exists, then create a new one.  */
  winShmEnterMutex();
  ಗಾಗಿ(p_Shm_ಗಂಟು = win_Shm_ಗಂಟು_List; p_Shm_ಗಂಟು; p_Shm_ಗಂಟು=p_Shm_ಗಂಟು->p_ಮುಂದಿನ){
    /* TBD need to come up with better match here.  Perhaps
    ** use FILE_ID_BOTH_DIR_INFO Structure.  */
    ಆದರೆ( sqlite3StrICmp(p_Shm_ಗಂಟು->zFilename, pNew->zFilename)==0 ) ನಿಲ್ಲಿಸು;
  }
  ಆದರೆ( p_Shm_ಗಂಟು==0 ){
    p_Shm_ಗಂಟು = pNew;

    /* Allocate a mutex for this winShmNode object, if one is required. */
    ಆದರೆ( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_ಗಂಟು->mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      ಆದರೆ( p_Shm_ಗಂಟು->mutex==0 ) rc = SQLITE_IOERR_NOMEM_BKPT;
    }

    /* Open a file-handle to use for mappings, and for the DMS lock. */
    ಆದರೆ( rc==SQLITE_OK ){
      HANDLE h = INVALID_HANDLE_ಮೌಲ್ಯ;
      p_Shm_ಗಂಟು->isReadonly = sqlite3_uri_boolean(pDbFd->z_ಮಾರ್ಗ,"readonly_shm",0);
      rc = winHandleOpen(pNew->zFilename, &p_Shm_ಗಂಟು->isReadonly, &h);
      p_Shm_ಗಂಟು->hSharedShm = h;
    }

    /* If successful, link the new winShmNode into the global list. If an 
    ** error occurred, free the object. */
    ಆದರೆ( rc==SQLITE_OK ){
      p_Shm_ಗಂಟು->p_ಮುಂದಿನ = win_Shm_ಗಂಟು_List;
      win_Shm_ಗಂಟು_List = p_Shm_ಗಂಟು;
      pNew = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_mutex_free(p_Shm_ಗಂಟು->mutex);
      ಆದರೆ( p_Shm_ಗಂಟು->hSharedShm!=INVALID_HANDLE_ಮೌಲ್ಯ ){
        osCloseHandle(p_Shm_ಗಂಟು->hSharedShm);
      }
    }
  }

  /* If no error has occurred, link the winShm object to the winShmNode and
  ** the winShm to pDbFd.  */
  ಆದರೆ( rc==SQLITE_OK ){
    sqlite3_mutex_enter(p_Shm_ಗಂಟು->mutex);
    p->p_Shm_ಗಂಟು = p_Shm_ಗಂಟು;
    p->p_Win_Shm_ಮುಂದಿನ = p_Shm_ಗಂಟು->pWinShmList;
    p_Shm_ಗಂಟು->pWinShmList = p;
#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_HAVE_OS_TRACE)
    p->id = p_Shm_ಗಂಟು->ಮುಂದಿನ_Shm_Id++;
#ಆದರೆ_ಅಂತ್ಯ
    pDbFd->pShm = p;
    sqlite3_mutex_leave(p_Shm_ಗಂಟು->mutex);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p ){
    sqlite3_free(p);
  }

  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || p_Shm_ಗಂಟು->isUnlocked==0 || p_Shm_ಗಂಟು->nRegion==0 );
  winShmLeaveMutex();
  sqlite3_free(pNew);

  /* Open a file-handle on the *-shm file for this connection. This file-handle
  ** is only used for locking. The mapping of the *-shm file is created using
  ** the shared file handle in winShmNode.hSharedShm.  */
  ಆದರೆ( rc==SQLITE_OK && p_Shm_ಗಂಟು->b_Use_Shared_ಬೀಗ_Handle==0 ){
    p->bReadonly = sqlite3_uri_boolean(pDbFd->z_ಮಾರ್ಗ, "readonly_shm", 0);
    rc = winHandleOpen(p_Shm_ಗಂಟು->zFilename, &p->bReadonly, &p->hShm);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಪ್ರತಿಜ್ಞಾ( p->hShm==INVALID_HANDLE_ಮೌಲ್ಯ );
      win_Close_Shared_ಸ್ಮರಣೆ(pDbFd, 0);
    }
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** Close a connection to shared-memory.  Delete the underlying
** storage if deleteFlag is true.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winShmUnmap(
  sqlite3_file *fd,          /* Database holding shared memory */
  ಪೂರ್ಣಾಂಕ deleteFlag             /* Delete after closing if true */
){
  ಹಿಂತಿರುಗಿ win_Close_Shared_ಸ್ಮರಣೆ((winFile*)fd, deleteFlag);
}

/*
** Change the lock state for a shared-memory segment.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Shm_ಬೀಗ(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  ಪೂರ್ಣಾಂಕ ofst,                  /* First lock to acquire or release */
  ಪೂರ್ಣಾಂಕ n,                     /* Number of locks to acquire or release */
  ಪೂರ್ಣಾಂಕ flags                  /* What to do with the lock */
){
  winFile *pDbFd = (winFile*)fd;        /* Connection holding shared memory */
  winShm *p = pDbFd->pShm;              /* The shared memory being locked */
  win_Shm_ಗಂಟು *p_Shm_ಗಂಟು;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;                   /* Result code */
  u16 ಮುಖವಾಡ = (u16)((1U<<(ofst+n)) - (1U<<ofst)); /* Mask of locks to [un]take */

  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ SQLITE_IOERR_SHMLOCK;
  p_Shm_ಗಂಟು = p->p_Shm_ಗಂಟು;
  ಆದರೆ( NEVER(p_Shm_ಗಂಟು==0) ) ಹಿಂತಿರುಗಿ SQLITE_IOERR_SHMLOCK;

  ಪ್ರತಿಜ್ಞಾ( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  ಪ್ರತಿಜ್ಞಾ( n>=1 );
  ಪ್ರತಿಜ್ಞಾ( flags==(SQLITE_SHM_ಬೀಗ | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_ಬೀಗ | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  ಪ್ರತಿಜ್ಞಾ( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );

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
#ಆದರೆ defined(SQLITE_ENABLE_SETLK_TIMEOUT) && defined(SQLITE_DEBUG)
  {
    u16 ಬೀಗ_ಮುಖವಾಡ = (p->excl_ಮುಖವಾಡ|p->shared_ಮುಖವಾಡ);
    ಪ್ರತಿಜ್ಞಾ( (flags & SQLITE_SHM_UNLOCK) || pDbFd->iBusyTimeout==0 || (
          (ofst!=2 || ಬೀಗ_ಮುಖವಾಡ==0)
       && (ofst!=1 || ಬೀಗ_ಮುಖವಾಡ==0 || ಬೀಗ_ಮುಖವಾಡ==2)
       && (ofst!=0 || ಬೀಗ_ಮುಖವಾಡ<3)
       && (ofst<3  || ಬೀಗ_ಮುಖವಾಡ<(1<<ofst))
    ));
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Check if there is any work to do. There are three cases:
  **
  **    a) An unlock operation where there are locks to unlock,
  **    b) An shared lock where the requested lock is not already held
  **    c) An exclusive lock where the requested lock is not already held
  **
  ** The SQLite core never requests an exclusive lock that it already holds.
  ** This is assert()ed immediately below.  */
  ಪ್ರತಿಜ್ಞಾ( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_ಬೀಗ) 
       || 0==(p->excl_ಮುಖವಾಡ & ಮುಖವಾಡ)
  );
  ಆದರೆ( ((flags & SQLITE_SHM_UNLOCK) && ((p->excl_ಮುಖವಾಡ|p->shared_ಮುಖವಾಡ) & ಮುಖವಾಡ))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_ಬೀಗ) && 0==(p->shared_ಮುಖವಾಡ & ಮುಖವಾಡ))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_ಬೀಗ))
  ){
    HANDLE h = p->hShm;

    ಆದರೆ( flags & SQLITE_SHM_UNLOCK ){
      /* Case (a) - unlock.  */

      ಪ್ರತಿಜ್ಞಾ( (p->excl_ಮುಖವಾಡ & p->shared_ಮುಖವಾಡ)==0 );
      ಪ್ರತಿಜ್ಞಾ( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->excl_ಮುಖವಾಡ & ಮುಖವಾಡ)==ಮುಖವಾಡ );
      ಪ್ರತಿಜ್ಞಾ( !(flags & SQLITE_SHM_SHARED) || (p->shared_ಮುಖವಾಡ & ಮುಖವಾಡ)==ಮುಖವಾಡ );

      ಪ್ರತಿಜ್ಞಾ( !(flags & SQLITE_SHM_SHARED) || n==1 );
      ಆದರೆ( p_Shm_ಗಂಟು->b_Use_Shared_ಬೀಗ_Handle ){
        h = p_Shm_ಗಂಟು->hSharedShm;
        ಆದರೆ( flags & SQLITE_SHM_SHARED ){
          winShm *pShm;
          sqlite3_mutex_enter(p_Shm_ಗಂಟು->mutex);
          ಗಾಗಿ(pShm=p_Shm_ಗಂಟು->pWinShmList; pShm; pShm=pShm->p_Win_Shm_ಮುಂದಿನ){
            ಆದರೆ( pShm!=p && (pShm->shared_ಮುಖವಾಡ & ಮುಖವಾಡ) ){
              /* Another connection within this process is also holding this
              ** SHARED lock. So do not actually release the OS lock.  */
              h = INVALID_HANDLE_ಮೌಲ್ಯ;
              ನಿಲ್ಲಿಸು;
            }
          }
          sqlite3_mutex_leave(p_Shm_ಗಂಟು->mutex);
        }
      }

      ಆದರೆ( h!=INVALID_HANDLE_ಮೌಲ್ಯ ){
        rc = winHandleUnlock(h, ofst+WIN_SHM_ತಳ, n);
      }

      /* If successful, also clear the bits in sharedMask/exclMask */
      ಆದರೆ( rc==SQLITE_OK ){
        p->excl_ಮುಖವಾಡ = (p->excl_ಮುಖವಾಡ & ~ಮುಖವಾಡ);
        p->shared_ಮುಖವಾಡ = (p->shared_ಮುಖವಾಡ & ~ಮುಖವಾಡ);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ bExcl = ((flags & SQLITE_SHM_EXCLUSIVE) ? 1 : 0);
      DWORD nMs = winFileBusyTimeout(pDbFd);

      ಆದರೆ( p_Shm_ಗಂಟು->b_Use_Shared_ಬೀಗ_Handle ){
        winShm *pShm;
        h = p_Shm_ಗಂಟು->hSharedShm;
        sqlite3_mutex_enter(p_Shm_ಗಂಟು->mutex);
        ಗಾಗಿ(pShm=p_Shm_ಗಂಟು->pWinShmList; pShm; pShm=pShm->p_Win_Shm_ಮುಂದಿನ){
          ಆದರೆ( bExcl ){
            ಆದರೆ( (pShm->shared_ಮುಖವಾಡ|pShm->excl_ಮುಖವಾಡ) & ಮುಖವಾಡ ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_ಮೌಲ್ಯ;
            }
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಆದರೆ( pShm->shared_ಮುಖವಾಡ & ಮುಖವಾಡ ){
              h = INVALID_HANDLE_ಮೌಲ್ಯ;
            }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pShm->excl_ಮುಖವಾಡ & ಮುಖವಾಡ ){
              rc = SQLITE_BUSY;
              h = INVALID_HANDLE_ಮೌಲ್ಯ;
            }
          }
        }
        sqlite3_mutex_leave(p_Shm_ಗಂಟು->mutex);
      }

      ಆದರೆ( h!=INVALID_HANDLE_ಮೌಲ್ಯ ){
        rc = win_Handle_ಬೀಗ_Timeout(h, ofst+WIN_SHM_ತಳ, n, bExcl, nMs);
      }
      ಆದರೆ( rc==SQLITE_OK ){
        ಆದರೆ( bExcl ){
          p->excl_ಮುಖವಾಡ = (p->excl_ಮುಖವಾಡ | ಮುಖವಾಡ);
        }ಇಲ್ಲದಿದ್ದರೆ{
          p->shared_ಮುಖವಾಡ = (p->shared_ಮುಖವಾಡ | ಮುಖವಾಡ);
        }
      }
    }
  }

  OSTRACE((
      "SHM-LOCK(%d,%d,%d) pid=%lu, id=%d, sharedMask=%03x, exclMask=%03x,"
      " rc=%s\n",
      ofst, n, flags, 
      osGetCurrentProcessId(), p->id, p->shared_ಮುಖವಾಡ, p->excl_ಮುಖವಾಡ,
      sqlite3ErrName(rc))
  );
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
ಸ್ಥಿರ ಶೂನ್ಯ winShmBarrier(
  sqlite3_file *fd          /* Database holding the shared memory */
){
  ಬಳಸದ_PARAMETER(fd);
  sqlite3_ಸ್ಮರಣೆ_Barrier();   /* compiler-defined memory barrier */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  ಪೂರ್ಣಾಂಕ iRegion,                    /* Region to retrieve */
  ಪೂರ್ಣಾಂಕ szRegion,                   /* Size of regions */
  ಪೂರ್ಣಾಂಕ isWrite,                    /* True to extend file if necessary */
  ಶೂನ್ಯ ಅಸ್ಥಿರ **pp              /* OUT: Mapped memory */
){
  winFile *pDbFd = (winFile*)fd;
  winShm *pShm = pDbFd->pShm;
  win_Shm_ಗಂಟು *p_Shm_ಗಂಟು;
  DWORD protect = ಪುಟ_READWRITE;
  DWORD flags = FILE_MAP_WRITE | FILE_MAP_READ;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  ಆದರೆ( !pShm ){
    rc = win_Open_Shared_ಸ್ಮರಣೆ(pDbFd);
    ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;
    pShm = pDbFd->pShm;
    ಪ್ರತಿಜ್ಞಾ( pShm!=0 );
  }
  p_Shm_ಗಂಟು = pShm->p_Shm_ಗಂಟು;

  sqlite3_mutex_enter(p_Shm_ಗಂಟು->mutex);
  ಆದರೆ( p_Shm_ಗಂಟು->isUnlocked ){
    /* Take the DMS lock. */
    ಪ್ರತಿಜ್ಞಾ( p_Shm_ಗಂಟು->nRegion==0 );
    rc = win_ಬೀಗ_Shared_ಸ್ಮರಣೆ(p_Shm_ಗಂಟು, winFileBusyTimeout(pDbFd));
    ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು shmpage_ಹೊರಹೋಗು;
  }

  ಪ್ರತಿಜ್ಞಾ( szRegion==p_Shm_ಗಂಟು->szRegion || p_Shm_ಗಂಟು->nRegion==0 );
  ಆದರೆ( p_Shm_ಗಂಟು->nRegion<=iRegion ){
    HANDLE hShared = p_Shm_ಗಂಟು->hSharedShm;
    ರಚನೆ ShmRegion *apNew;           /* New aRegion[] array */
    i64 nByte = ((i64)iRegion+1)*(i64)szRegion;  /* Minimum file size */
    sqlite3_int64 sz;                  /* Current size of wal-index file */

    p_Shm_ಗಂಟು->szRegion = szRegion;

    /* The requested region is not mapped into this processes address space.
    ** Check to see if it has been allocated (i.e. if the wal-index file is
    ** large enough to contain the requested region).
    */
    rc = win_Handle_ಗಾತ್ರ(hShared, &sz);
    ಆದರೆ( rc!=SQLITE_OK ){
      rc = winLogError(rc, os_Get_ಕೊನೆಯ_Error(), "winShmMap1", pDbFd->z_ಮಾರ್ಗ);
      ಹೋಗು shmpage_ಹೊರಹೋಗು;
    }

    ಆದರೆ( sz<nByte ){
      /* The requested memory region does not exist. If isWrite is set to
      ** zero, exit early. *pp will be set to NULL and SQLITE_OK returned.
      **
      ** Alternatively, if isWrite is non-zero, use ftruncate() to allocate
      ** the requested memory region.  */
      ಆದರೆ( !isWrite ) ಹೋಗು shmpage_ಹೊರಹೋಗು;
      rc = winHandleTruncate(hShared, nByte);
      ಆದರೆ( rc!=SQLITE_OK ){
        rc = winLogError(rc, os_Get_ಕೊನೆಯ_Error(), "winShmMap2", pDbFd->z_ಮಾರ್ಗ);
        ಹೋಗು shmpage_ಹೊರಹೋಗು;
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (ರಚನೆ ShmRegion*)sqlite3_realloc64(
        p_Shm_ಗಂಟು->aRegion, ((i64)iRegion+1)*ಗಾತ್ರ(apNew[0])
    );
    ಆದರೆ( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      ಹೋಗು shmpage_ಹೊರಹೋಗು;
    }
    p_Shm_ಗಂಟು->aRegion = apNew;

    ಆದರೆ( p_Shm_ಗಂಟು->isReadonly ){
      protect = ಪುಟ_READONLY;
      flags = FILE_MAP_READ;
    }

    ಆಗ( p_Shm_ಗಂಟು->nRegion<=iRegion ){
      HANDLE hMap = ಖಾಲಿ;         /* file-mapping handle */
      ಶೂನ್ಯ *pMap = 0;             /* Mapped memory region */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
      hMap = osCreateFileMappingFromApp(hShared, ಖಾಲಿ, protect, nByte, ಖಾಲಿ);
#ಇಲ್ಲದಿದ್ದರೆ
      hMap = osCreateFileMappingW(hShared, ಖಾಲಿ, protect, 0, nByte, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ
      OSTRACE(("SHM-MAP-CREATE pid=%lu, region=%d, size=%lld, rc=%s\n",
               osGetCurrentProcessId(), p_Shm_ಗಂಟು->nRegion, nByte,
               hMap ? "ok" : "failed"));
      ಆದರೆ( hMap ){
        i64 i_ಸ್ಥಾನಾಂತರ = p_Shm_ಗಂಟು->nRegion*szRegion;
        ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ_ಸರಿಸು = i_ಸ್ಥಾನಾಂತರ % winSysInfo.dw_ಹಂಚಿಕೆ_Granularity;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
        pMap = os_Map_View_ದ_File_From_App(hMap, flags,
            i_ಸ್ಥಾನಾಂತರ - i_ಸ್ಥಾನಾಂತರ_ಸರಿಸು, (i64)szRegion + i_ಸ್ಥಾನಾಂತರ_ಸರಿಸು
        );
#ಇಲ್ಲದಿದ್ದರೆ
        pMap = os_Map_View_ದ_File(hMap, flags,
            0, i_ಸ್ಥಾನಾಂತರ - i_ಸ್ಥಾನಾಂತರ_ಸರಿಸು, (i64)szRegion + i_ಸ್ಥಾನಾಂತರ_ಸರಿಸು
        );
#ಆದರೆ_ಅಂತ್ಯ
        OSTRACE(("SHM-MAP-MAP pid=%lu, region=%d, offset=%d, size=%d, rc=%s\n",
                 osGetCurrentProcessId(), p_Shm_ಗಂಟು->nRegion, i_ಸ್ಥಾನಾಂತರ,
                 szRegion, pMap ? "ok" : "failed"));
      }
      ಆದರೆ( !pMap ){
        p_Shm_ಗಂಟು->ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
        rc = winLogError(SQLITE_IOERR_SHMMAP, p_Shm_ಗಂಟು->ಕೊನೆಯ_Errno,
                         "winShmMap3", pDbFd->z_ಮಾರ್ಗ);
        ಆದರೆ( hMap ) osCloseHandle(hMap);
        ಹೋಗು shmpage_ಹೊರಹೋಗು;
      }

      p_Shm_ಗಂಟು->aRegion[p_Shm_ಗಂಟು->nRegion].pMap = pMap;
      p_Shm_ಗಂಟು->aRegion[p_Shm_ಗಂಟು->nRegion].hMap = hMap;
      p_Shm_ಗಂಟು->nRegion++;
    }
  }

shmpage_ಹೊರಹೋಗು:
  ಆದರೆ( p_Shm_ಗಂಟು->nRegion>iRegion ){
    i64 i_ಸ್ಥಾನಾಂತರ = (i64)iRegion*(i64)szRegion;
    ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ_ಸರಿಸು = i_ಸ್ಥಾನಾಂತರ % winSysInfo.dw_ಹಂಚಿಕೆ_Granularity;
    ಅಕ್ಷರ *p = (ಅಕ್ಷರ *)p_Shm_ಗಂಟು->aRegion[iRegion].pMap;
    *pp = (ಶೂನ್ಯ *)&p[i_ಸ್ಥಾನಾಂತರ_ಸರಿಸು];
  }ಇಲ್ಲದಿದ್ದರೆ{
    *pp = 0;
  }
  ಆದರೆ( p_Shm_ಗಂಟು->isReadonly && rc==SQLITE_OK ){
    rc = SQLITE_READONLY;
  }
  sqlite3_mutex_leave(p_Shm_ಗಂಟು->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

#ಇಲ್ಲದಿದ್ದರೆ
# define winShmMap     0
# define win_Shm_ಬೀಗ    0
# define winShmBarrier 0
# define winShmUnmap   0
#ಆದರೆ_ಅಂತ್ಯ /* #ifndef SQLITE_OMIT_WAL */

/*
** Cleans up the mapped region of the specified file, if any.
*/
#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winUnmapfile(winFile *pFile){
  ಪ್ರತಿಜ್ಞಾ( pFile!=0 );
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, pMapRegion=%p, "
           "mmapSize=%lld, mmapSizeMax=%lld\n",
           osGetCurrentProcessId(), pFile, pFile->hMap, pFile->pMapRegion,
           pFile->mmap_ಗಾತ್ರ, pFile->mmap_ಗಾತ್ರ_Max));
  ಆದರೆ( pFile->pMapRegion ){
    ಆದರೆ( !os_Unmap_View_ದ_File(pFile->pMapRegion) ){
      pFile->ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, pMapRegion=%p, "
               "rc=SQLITE_IOERR_MMAP\n", osGetCurrentProcessId(), pFile,
               pFile->pMapRegion));
      ಹಿಂತಿರುಗಿ winLogError(SQLITE_IOERR_MMAP, pFile->ಕೊನೆಯ_Errno,
                         "winUnmapfile1", pFile->z_ಮಾರ್ಗ);
    }
    pFile->pMapRegion = 0;
    pFile->mmap_ಗಾತ್ರ = 0;
  }
  ಆದರೆ( pFile->hMap!=ಖಾಲಿ ){
    ಆದರೆ( !osCloseHandle(pFile->hMap) ){
      pFile->ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
      OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, hMap=%p, rc=SQLITE_IOERR_MMAP\n",
               osGetCurrentProcessId(), pFile, pFile->hMap));
      ಹಿಂತಿರುಗಿ winLogError(SQLITE_IOERR_MMAP, pFile->ಕೊನೆಯ_Errno,
                         "winUnmapfile2", pFile->z_ಮಾರ್ಗ);
    }
    pFile->hMap = ಖಾಲಿ;
  }
  OSTRACE(("UNMAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFile));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winMapfile(winFile *pFd, sqlite3_int64 nByte){
  sqlite3_int64 nMap = nByte;
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ( nMap>=0 || pFd->n_Fetch_ಹೊರಹೋಗು==0 );
  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, size=%lld\n",
           osGetCurrentProcessId(), pFd, nByte));

  ಆದರೆ( pFd->n_Fetch_ಹೊರಹೋಗು>0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;

  ಆದರೆ( nMap<0 ){
    rc = win_File_ಗಾತ್ರ((sqlite3_file*)pFd, &nMap);
    ಆದರೆ( rc ){
      OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_IOERR_FSTAT\n",
               osGetCurrentProcessId(), pFd));
      ಹಿಂತಿರುಗಿ SQLITE_IOERR_FSTAT;
    }
  }
  ಆದರೆ( nMap>pFd->mmap_ಗಾತ್ರ_Max ){
    nMap = pFd->mmap_ಗಾತ್ರ_Max;
  }
  nMap &= ~(sqlite3_int64)(winSysInfo.dw_ಪುಟ_ಗಾತ್ರ - 1);

  ಆದರೆ( nMap==0 && pFd->mmap_ಗಾತ್ರ>0 ){
    winUnmapfile(pFd);
  }
  ಆದರೆ( nMap!=pFd->mmap_ಗಾತ್ರ ){
    ಶೂನ್ಯ *pNew = 0;
    DWORD protect = ಪುಟ_READONLY;
    DWORD flags = FILE_MAP_READ;

    winUnmapfile(pFd);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_MMAP_READWRITE
    ಆದರೆ( (pFd->ctrlFlags & WINFILE_RDONLY)==0 ){
      protect = ಪುಟ_READWRITE;
      flags |= FILE_MAP_WRITE;
    }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
    pFd->hMap = osCreateFileMappingFromApp(pFd->h, ಖಾಲಿ, protect, nMap, ಖಾಲಿ);
#ಇಲ್ಲದಿದ್ದರೆ
    pFd->hMap = osCreateFileMappingW(pFd->h, ಖಾಲಿ, protect,
                                (DWORD)((nMap>>32) & 0xffffffff),
                                (DWORD)(nMap & 0xffffffff), ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( pFd->hMap==ಖಾಲಿ ){
      pFd->ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->ಕೊನೆಯ_Errno,
                       "winMapfile1", pFd->z_ಮಾರ್ಗ);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-CREATE pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಪ್ರತಿಜ್ಞಾ( (nMap % winSysInfo.dw_ಪುಟ_ಗಾತ್ರ)==0 );
    ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(ಗಾತ್ರ_T)==ಗಾತ್ರ(sqlite3_int64) || nMap<=0xffffffff );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
    pNew = os_Map_View_ದ_File_From_App(pFd->hMap, flags, 0, (ಗಾತ್ರ_T)nMap);
#ಇಲ್ಲದಿದ್ದರೆ
    pNew = os_Map_View_ದ_File(pFd->hMap, flags, 0, 0, (ಗಾತ್ರ_T)nMap);
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( pNew==ಖಾಲಿ ){
      osCloseHandle(pFd->hMap);
      pFd->hMap = ಖಾಲಿ;
      pFd->ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
      rc = winLogError(SQLITE_IOERR_MMAP, pFd->ಕೊನೆಯ_Errno,
                       "winMapfile2", pFd->z_ಮಾರ್ಗ);
      /* Log the error, but continue normal operation using xRead/xWrite */
      OSTRACE(("MAP-FILE-MAP pid=%lu, pFile=%p, rc=%s\n",
               osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    pFd->pMapRegion = pNew;
    pFd->mmap_ಗಾತ್ರ = nMap;
  }

  OSTRACE(("MAP-FILE pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), pFd));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_MAX_MMAP_SIZE>0 */

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winFetch(sqlite3_file *fd, i64 iOff, ಪೂರ್ಣಾಂಕ nAmt, ಶೂನ್ಯ **pp){
#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */
#ಆದರೆ_ಅಂತ್ಯ
  *pp = 0;

  OSTRACE(("FETCH pid=%lu, pFile=%p, offset=%lld, amount=%d, pp=%p\n",
           osGetCurrentProcessId(), fd, iOff, nAmt, pp));

#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  ಆದರೆ( pFd->mmap_ಗಾತ್ರ_Max>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ n_Eof_ಬಫರ್ = 256;
    ಆದರೆ( pFd->pMapRegion==0 ){
      ಪೂರ್ಣಾಂಕ rc = winMapfile(pFd, -1);
      ಆದರೆ( rc!=SQLITE_OK ){
        OSTRACE(("FETCH pid=%lu, pFile=%p, rc=%s\n",
                 osGetCurrentProcessId(), pFd, sqlite3ErrName(rc)));
        ಹಿಂತಿರುಗಿ rc;
      }
    }
    ಆದರೆ( pFd->mmap_ಗಾತ್ರ >= (iOff+nAmt+n_Eof_ಬಫರ್) ){
      ಪ್ರತಿಜ್ಞಾ( pFd->pMapRegion!=0 );
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->n_Fetch_ಹೊರಹೋಗು++;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  OSTRACE(("FETCH pid=%lu, pFile=%p, pp=%p, *pp=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd, pp, *pp));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winUnfetch(sqlite3_file *fd, i64 iOff, ಶೂನ್ಯ *p){
#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  winFile *pFd = (winFile*)fd;   /* The underlying database file */

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  ಪ್ರತಿಜ್ಞಾ( (p==0)==(pFd->n_Fetch_ಹೊರಹೋಗು==0) );

  /* If p!=0, it must match the iOff value. */
  ಪ್ರತಿಜ್ಞಾ( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, offset=%lld, p=%p\n",
           osGetCurrentProcessId(), pFd, iOff, p));

  ಆದರೆ( p ){
    pFd->n_Fetch_ಹೊರಹೋಗು--;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* FIXME:  If Windows truly always prevents truncating or deleting a
    ** file while a mapping is held, then the following winUnmapfile() call
    ** is unnecessary can be omitted - potentially improving
    ** performance.  */
    winUnmapfile(pFd);
  }

  ಪ್ರತಿಜ್ಞಾ( pFd->n_Fetch_ಹೊರಹೋಗು>=0 );
#ಆದರೆ_ಅಂತ್ಯ

  OSTRACE(("UNFETCH pid=%lu, pFile=%p, rc=SQLITE_OK\n",
           osGetCurrentProcessId(), fd));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_io_methods win_Io_ವಿಧಾನ = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_ಗಾತ್ರ,                    /* xFileSize */
  win_ಬೀಗ,                        /* xLock */
  winUnlock,                      /* xUnlock */
  win_Check_Reserved_ಬೀಗ,           /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_ಗಾತ್ರ,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  win_Shm_ಬೀಗ,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

/*
** This vector defines all the methods that can operate on an
** sqlite3_file for win32 without performing any locking.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_io_methods win_Io_Nolock_ವಿಧಾನ = {
  3,                              /* iVersion */
  winClose,                       /* xClose */
  winRead,                        /* xRead */
  winWrite,                       /* xWrite */
  winTruncate,                    /* xTruncate */
  winSync,                        /* xSync */
  win_File_ಗಾತ್ರ,                    /* xFileSize */
  win_Nolock_ಬೀಗ,                  /* xLock */
  winNolockUnlock,                /* xUnlock */
  win_Nolock_Check_Reserved_ಬೀಗ,     /* xCheckReservedLock */
  winFileControl,                 /* xFileControl */
  win_Sector_ಗಾತ್ರ,                  /* xSectorSize */
  winDeviceCharacteristics,       /* xDeviceCharacteristics */
  winShmMap,                      /* xShmMap */
  win_Shm_ಬೀಗ,                     /* xShmLock */
  winShmBarrier,                  /* xShmBarrier */
  winShmUnmap,                    /* xShmUnmap */
  winFetch,                       /* xFetch */
  winUnfetch                      /* xUnfetch */
};

ಸ್ಥಿರ win_Vfs_App_ದತ್ತಾಂಶ win_App_ದತ್ತಾಂಶ = {
  &win_Io_ವಿಧಾನ,       /* pMethod */
  0,                  /* pAppData */
  0                   /* bNoLock */
};

ಸ್ಥಿರ win_Vfs_App_ದತ್ತಾಂಶ win_Nolock_App_ದತ್ತಾಂಶ = {
  &win_Io_Nolock_ವಿಧಾನ, /* pMethod */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Make_ಕೊನೆ_ಒಳ_Dir_Sep(ಪೂರ್ಣಾಂಕ nBuf, ಅಕ್ಷರ *zBuf){
  ಆದರೆ( zBuf ){
    ಪೂರ್ಣಾಂಕ nLen = sqlite3Strlen30(zBuf);
    ಆದರೆ( nLen>0 ){
      ಆದರೆ( winIsDirSep(zBuf[nLen-1]) ){
        ಹಿಂತಿರುಗಿ 1;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nLen+1<nBuf ){
        ಆದರೆ( !osGetenv ){
          zBuf[nLen] = winGetDirSep();
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( winIsDriveLetterAndColon(zBuf) && winIsDirSep(zBuf[2]) ){
          zBuf[nLen] = '\\';
          zBuf[2]='\\';
        }ಇಲ್ಲದಿದ್ದರೆ{
          zBuf[nLen] = '/';
        }
        zBuf[nLen+1] = '\0';
        ಹಿಂತಿರುಗಿ 1;
      }
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** If sqlite3_temp_directory is defined, take the mutex and return true.
**
** If sqlite3_temp_directory is NULL (undefined), omit the mutex and
** return false.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_ತಾತ್ಕಾಲಿಕ_Dir_Defined(ಶೂನ್ಯ){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ಆದರೆ( sqlite3_ತಾತ್ಕಾಲಿಕ_directory!=0 ) ಹಿಂತಿರುಗಿ 1;
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Create a temporary file name and store the resulting pointer into pzBuf.
** The pointer returned in pzBuf must be freed via sqlite3_free().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winGetTempname(sqlite3_vfs *pVfs, ಅಕ್ಷರ **pzBuf){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ zChars[] =
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789";
  size_t i, j;
  DWORD pid;
  ಪೂರ್ಣಾಂಕ nPre = sqlite3Strlen30(SQLITE_ತಾತ್ಕಾಲಿಕ_FILE_ಪೂರ್ವಪ್ರತ್ಯಯ);
  i64 nMax, nBuf, nDir, nLen;
  ಅಕ್ಷರ *zBuf;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  SimulateIOError( ಹಿಂತಿರುಗಿ SQLITE_IOERR );

  /* Allocate a temporary buffer to store the fully qualified file
  ** name for the temporary file.  If this fails, we cannot continue.
  */
  nMax = pVfs->mxPathname;
  nBuf = 2 + (i64)nMax;
  zBuf = sqlite3MallocZero( nBuf );
  ಆದರೆ( !zBuf ){
    OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM_BKPT;
  }

  /* Figure out the effective temporary directory.  First, check if one
  ** has been explicitly set by the application; otherwise, use the one
  ** configured by the operating system.
  */
  nDir = nMax - (nPre + 15);
  ಪ್ರತಿಜ್ಞಾ( nDir>0 );
  ಆದರೆ( win_ತಾತ್ಕಾಲಿಕ_Dir_Defined() ){
    ಪೂರ್ಣಾಂಕ nDirLen = sqlite3Strlen30(sqlite3_ತಾತ್ಕಾಲಿಕ_directory);
    ಆದರೆ( nDirLen>0 ){
      ಆದರೆ( !winIsDirSep(sqlite3_ತಾತ್ಕಾಲಿಕ_directory[nDirLen-1]) ){
        nDirLen++;
      }
      ಆದರೆ( nDirLen>nDir ){
        sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
        sqlite3_free(zBuf);
        OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
        ಹಿಂತಿರುಗಿ winLogError(SQLITE_ERROR, 0, "winGetTempname1", 0);
      }
      sqlite3_snprintf(nMax, zBuf, "%s", sqlite3_ತಾತ್ಕಾಲಿಕ_directory);
    }
    sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  }

#ಆದರೆ defined(__CYGWIN__)
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( osGetenv!=ಖಾಲಿ ){
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azDirs[] = {
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
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDir = 0;

    ಆದರೆ( !azDirs[0] ) azDirs[0] = osGetenv("SQLITE_TMPDIR");
    ಆದರೆ( !azDirs[1] ) azDirs[1] = osGetenv("TMPDIR");
    ಆದರೆ( !azDirs[2] ) azDirs[2] = osGetenv("TMP");
    ಆದರೆ( !azDirs[3] ) azDirs[3] = osGetenv("TEMP");
    ಆದರೆ( !azDirs[4] ) azDirs[4] = osGetenv("USERPROFILE");
    ಗಾಗಿ(i=0; i<ಗಾತ್ರ(azDirs)/ಗಾತ್ರ(azDirs[0]); zDir=azDirs[i++]){
      LPWSTR zConverted;
      ಆದರೆ( zDir==0 ) ಮುಂದುವರಿ;
      /* If the path starts with a drive letter followed by the colon
      ** character, assume it is already a native Win32 path; otherwise,
      ** it must be converted to a native Win32 path via the Cygwin API
      ** prior to using it.
      */
      {
        zConverted = winConvertFromUtf8Filename(zDir);
        ಆದರೆ( !zConverted ){
          sqlite3_free(zBuf);
          OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
          ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM_BKPT;
        }
        ಆದರೆ( winIsDir(zConverted) ){
          sqlite3_snprintf(nMax, zBuf, "%s", zDir);
          sqlite3_free(zConverted);
          ನಿಲ್ಲಿಸು;
        }
        sqlite3_free(zConverted);
      }
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಇಲ್ಲದಿದ್ದರೆ{
    ಅಕ್ಷರ *zMulti;
    LPWSTR z_Wide_ಮಾರ್ಗ = sqlite3MallocZero( nMax*ಗಾತ್ರ(WCHAR) );
    ಆದರೆ( !z_Wide_ಮಾರ್ಗ ){
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM_BKPT;
    }
    ಆದರೆ( os_Get_ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ_W==0 || os_Get_ತಾತ್ಕಾಲಿಕ_ಮಾರ್ಗ_W(nMax, z_Wide_ಮಾರ್ಗ)==0 ){
      sqlite3_free(z_Wide_ಮಾರ್ಗ);
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_GETTEMPPATH\n"));
      ಹಿಂತಿರುಗಿ winLogError(SQLITE_IOERR_GETTEMPPATH, os_Get_ಕೊನೆಯ_Error(),
                         "winGetTempname2", 0);
    }
    zMulti = win_Unicode_ಗಮ್ಯ_Utf8(z_Wide_ಮಾರ್ಗ);
    sqlite3_free(z_Wide_ಮಾರ್ಗ);
    ಆದರೆ( zMulti ){
      sqlite3_snprintf(nMax, zBuf, "%s", zMulti);
      sqlite3_free(zMulti);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_free(zBuf);
      OSTRACE(("TEMP-FILENAME rc=SQLITE_IOERR_NOMEM\n"));
      ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM_BKPT;
    }
  }

  /*
  ** Check to make sure the temporary directory ends with an appropriate
  ** separator.  If it does not and there is not enough space left to add
  ** one, fail.
  */
  ಆದರೆ( !win_Make_ಕೊನೆ_ಒಳ_Dir_Sep(nDir+1, zBuf) ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    ಹಿಂತಿರುಗಿ winLogError(SQLITE_ERROR, 0, "winGetTempname4", 0);
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
  ಆದರೆ( (nLen + nPre + 17) > nBuf ){
    sqlite3_free(zBuf);
    OSTRACE(("TEMP-FILENAME rc=SQLITE_ERROR\n"));
    ಹಿಂತಿರುಗಿ winLogError(SQLITE_ERROR, 0, "winGetTempname5", 0);
  }

  sqlite3_snprintf(nBuf-16-nLen, zBuf+nLen, SQLITE_ತಾತ್ಕಾಲಿಕ_FILE_ಪೂರ್ವಪ್ರತ್ಯಯ);

  j = sqlite3Strlen30(zBuf);
  sqlite3_randomness(15, &zBuf[j]);
  pid = osGetCurrentProcessId();
  ಗಾಗಿ(i=0; i<15; i++, j++){
    zBuf[j] += pid & 0xff;
    pid >>= 8;
    zBuf[j] = (ಅಕ್ಷರ)zChars[ ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)zBuf[j])%(ಗಾತ್ರ(zChars)-1) ];
  }
  zBuf[j] = 0;
  zBuf[j+1] = 0;
  *pzBuf = zBuf;

  OSTRACE(("TEMP-FILENAME name=%s, rc=SQLITE_OK\n", zBuf));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return TRUE if the named file is really a directory.  Return false if
** it is something other than a directory, or if there is any kind of memory
** allocation failure.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winIsDir(LPCWSTR zConverted){
  DWORD attr;
  ಪೂರ್ಣಾಂಕ rc = 0;
  DWORD ಕೊನೆಯ_Errno;
  ಪೂರ್ಣಾಂಕ cnt = 0;
  WIN32_FILE_ATTRIBUTE_ದತ್ತಾಂಶ s_Attr_ದತ್ತಾಂಶ;
  memset(&s_Attr_ದತ್ತಾಂಶ, 0, ಗಾತ್ರ(s_Attr_ದತ್ತಾಂಶ));
  ಆಗ( !(rc = os_Get_File_ಗುಣಲಕ್ಷಣಗಳು_Ex_W(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_ದತ್ತಾಂಶ)) && winRetryIoerr(&cnt, &ಕೊನೆಯ_Errno) ){}
  ಆದರೆ( !rc ){
    ಹಿಂತಿರುಗಿ 0; /* Invalid name? */
  }
  attr = s_Attr_ದತ್ತಾಂಶ.dw_File_ಗುಣಲಕ್ಷಣಗಳು;
  ಹಿಂತಿರುಗಿ (attr!=INVALID_FILE_ಗುಣಲಕ್ಷಣಗಳು) && (attr&FILE_ATTRIBUTE_DIRECTORY);
}

/* forward reference */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename,     /* Name of file to check */
  ಪೂರ್ಣಾಂಕ flags,                 /* Type of test to make on this file */
  ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು               /* OUT: Result */
);

/*
** The Windows version of xAccess() accepts an extra bit in the flags
** parameter that prevents an anti-virus retry loop.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು NORETRY 0x4000

/*
** Open a file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winOpen(
  sqlite3_vfs *pVfs,        /* Used to get maximum path length and AppData */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,        /* Name of the file (UTF-8) */
  sqlite3_file *id,         /* Write the SQLite file handle here */
  ಪೂರ್ಣಾಂಕ flags,                /* Open mode flags */
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags            /* Status return flags */
){
  HANDLE h;
  DWORD ಕೊನೆಯ_Errno = 0;
  DWORD dwDesiredAccess;
  DWORD dw_Share_ಬಹುಲಕ;
  DWORD dwCreationDisposition;
  DWORD dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು = 0;
  win_Vfs_App_ದತ್ತಾಂಶ *p_App_ದತ್ತಾಂಶ;
  winFile *pFile = (winFile*)id;
  LPWSTR zConverted;              /* Filename in OS encoding */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zUtf8Name = zName; /* Filename in UTF-8 encoding */
  ಪೂರ್ಣಾಂಕ cnt = 0;
  ಪೂರ್ಣಾಂಕ isRO = 0;              /* file is known to be accessible readonly */

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  ಅಕ್ಷರ *zTmpname = 0; /* For temporary filename, if necessary. */

  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;            /* Function Return Code */
#ಆದರೆ !defined(NDEBUG)
  ಪೂರ್ಣಾಂಕ eType = flags&0x0FFF00;  /* Type of file to open */
#ಆದರೆ_ಅಂತ್ಯ

  ಪೂರ್ಣಾಂಕ isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  ಪೂರ್ಣಾಂಕ isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  ಪೂರ್ಣಾಂಕ isCreate     = (flags & SQLITE_OPEN_CREATE);
  ಪೂರ್ಣಾಂಕ isReadonly   = (flags & SQLITE_OPEN_READONLY);
  ಪೂರ್ಣಾಂಕ isReadWrite  = (flags & SQLITE_OPEN_READWRITE);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  ಪೂರ್ಣಾಂಕ isOpenJournal = (isCreate && (
        eType==SQLITE_OPEN_SUPER_JOURNAL
     || eType==SQLITE_OPEN_MAIN_JOURNAL
     || eType==SQLITE_OPEN_WAL
  ));
#ಆದರೆ_ಅಂತ್ಯ

  OSTRACE(("OPEN name=%s, pFile=%p, flags=%x, pOutFlags=%p\n",
           zUtf8Name, id, flags, p_ಹೊರಹೋಗು_Flags));

  /* Check the following statements are true:
  **
  **   (a) Exactly one of the READWRITE and READONLY flags must be set, and
  **   (b) if CREATE is set, then READWRITE must also be set, and
  **   (c) if EXCLUSIVE is set, then CREATE must also be set.
  **   (d) if DELETEONCLOSE is set, then CREATE must also be set.
  */
  ಪ್ರತಿಜ್ಞಾ((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
  ಪ್ರತಿಜ್ಞಾ(isCreate==0 || isReadWrite);
  ಪ್ರತಿಜ್ಞಾ(isExclusive==0 || isCreate);
  ಪ್ರತಿಜ್ಞಾ(isDelete==0 || isCreate);

  /* The main DB, main journal, WAL file and super-journal are never
  ** automatically deleted. Nor are they ever temporary files.  */
  ಪ್ರತಿಜ್ಞಾ( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_DB );
  ಪ್ರತಿಜ್ಞಾ( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_JOURNAL );
  ಪ್ರತಿಜ್ಞಾ( (!isDelete && zName) || eType!=SQLITE_OPEN_SUPER_JOURNAL );
  ಪ್ರತಿಜ್ಞಾ( (!isDelete && zName) || eType!=SQLITE_OPEN_WAL );

  /* Assert that the upper layer has set one of the "file-type" flags. */
  ಪ್ರತಿಜ್ಞಾ( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_ತಾತ್ಕಾಲಿಕ_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_ತಾತ್ಕಾಲಿಕ_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );

  ಪ್ರತಿಜ್ಞಾ( pFile!=0 );
  memset(pFile, 0, ಗಾತ್ರ(winFile));
  pFile->h = INVALID_HANDLE_ಮೌಲ್ಯ;

  /* If the second argument to this function is NULL, generate a
  ** temporary file name to use
  */
  ಆದರೆ( !zUtf8Name ){
    ಪ್ರತಿಜ್ಞಾ( isDelete && !isOpenJournal );
    rc = winGetTempname(pVfs, &zTmpname);
    ಆದರೆ( rc!=SQLITE_OK ){
      OSTRACE(("OPEN name=%s, rc=%s", zUtf8Name, sqlite3ErrName(rc)));
      ಹಿಂತಿರುಗಿ rc;
    }
    zUtf8Name = zTmpname;
  }

  /* Database filenames are double-zero terminated if they are not
  ** URIs with parameters.  Hence, they can always be passed into
  ** sqlite3_uri_parameter().
  */
  ಪ್ರತಿಜ್ಞಾ( (eType!=SQLITE_OPEN_MAIN_DB) || (flags & SQLITE_OPEN_URI) ||
       zUtf8Name[sqlite3Strlen30(zUtf8Name)+1]==0 );

  /* Convert the filename to the system encoding. */
  zConverted = winConvertFromUtf8Filename(zUtf8Name);
  ಆದರೆ( zConverted==0 ){
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_IOERR_NOMEM", zUtf8Name));
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM_BKPT;
  }

  ಆದರೆ( winIsDir(zConverted) ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    OSTRACE(("OPEN name=%s, rc=SQLITE_CANTOPEN_ISDIR", zUtf8Name));
    ಹಿಂತಿರುಗಿ SQLITE_CANTOPEN_ISDIR;
  }

  ಆದರೆ( isReadWrite ){
    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
  }ಇಲ್ಲದಿದ್ದರೆ{
    dwDesiredAccess = GENERIC_READ;
  }

  /* SQLITE_OPEN_EXCLUSIVE is used to make sure that a new file is
  ** created. SQLite doesn't use it to indicate "exclusive access"
  ** as it is usually understood.
  */
  ಆದರೆ( isExclusive ){
    /* Creates a new file, only if it does not already exist. */
    /* If the file exists, it fails. */
    dwCreationDisposition = CREATE_NEW;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( isCreate ){
    /* Open existing file, or create if it doesn't exist */
    dwCreationDisposition = OPEN_ALWAYS;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Opens a file, only if it exists. */
    dwCreationDisposition = OPEN_EXISTING;
  }

  ಆದರೆ( 0==sqlite3_uri_boolean(zName, "exclusive", 0) ){
    dw_Share_ಬಹುಲಕ = FILE_SHARE_READ | FILE_SHARE_WRITE;
  }ಇಲ್ಲದಿದ್ದರೆ{
    dw_Share_ಬಹುಲಕ = 0;
  }

  ಆದರೆ( isDelete ){
    dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು = FILE_ATTRIBUTE_TEMPORARY
                               | FILE_ATTRIBUTE_HIDDEN
                               | FILE_FLAG_DELETE_ON_CLOSE;
  }ಇಲ್ಲದಿದ್ದರೆ{
    dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು = FILE_ATTRIBUTE_ಸಾಮಾನ್ಯ;
  }
  ಮಾಡು{
    h = osCreateFileW(zConverted,
                      dwDesiredAccess,
                      dw_Share_ಬಹುಲಕ, ಖಾಲಿ,
                      dwCreationDisposition,
                      dw_Flags_And_ಗುಣಲಕ್ಷಣಗಳು,
                      ಖಾಲಿ);
    ಆದರೆ( h!=INVALID_HANDLE_ಮೌಲ್ಯ ) ನಿಲ್ಲಿಸು;
    ಆದರೆ( isReadWrite ){
      ಪೂರ್ಣಾಂಕ rc2;
      sqlite3BeginBenignMalloc();
      rc2 = winAccess(pVfs, zUtf8Name, SQLITE_ACCESS_READ|NORETRY, &isRO);
      sqlite3_ಕೊನೆ_Benign_Malloc();
      ಆದರೆ( rc2==SQLITE_OK && isRO ) ನಿಲ್ಲಿಸು;
    }
  }ಆಗ( winRetryIoerr(&cnt, &ಕೊನೆಯ_Errno) );
  winLogIoerr(cnt, __LINE__);

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, rc=%s\n", h, zUtf8Name,
           dwDesiredAccess, (h==INVALID_HANDLE_ಮೌಲ್ಯ) ? "failed" : "ok"));

  ಆದರೆ( h==INVALID_HANDLE_ಮೌಲ್ಯ ){
    sqlite3_free(zConverted);
    sqlite3_free(zTmpname);
    ಆದರೆ( isReadWrite && isRO && !isExclusive ){
      ಹಿಂತಿರುಗಿ winOpen(pVfs, zName, id,
         ((flags|SQLITE_OPEN_READONLY) &
                     ~(SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE)),
         p_ಹೊರಹೋಗು_Flags);
    }ಇಲ್ಲದಿದ್ದರೆ{
      pFile->ಕೊನೆಯ_Errno = ಕೊನೆಯ_Errno;
      winLogError(SQLITE_CANTOPEN, pFile->ಕೊನೆಯ_Errno, "winOpen", zUtf8Name);
      ಹಿಂತಿರುಗಿ SQLITE_CANTOPEN_BKPT;
    }
  }

  ಆದರೆ( p_ಹೊರಹೋಗು_Flags ){
    ಆದರೆ( isReadWrite ){
      *p_ಹೊರಹೋಗು_Flags = SQLITE_OPEN_READWRITE;
    }ಇಲ್ಲದಿದ್ದರೆ{
      *p_ಹೊರಹೋಗು_Flags = SQLITE_OPEN_READONLY;
    }
  }

  OSTRACE(("OPEN file=%p, name=%s, access=%lx, pOutFlags=%p, *pOutFlags=%d, "
           "rc=%s\n", h, zUtf8Name, dwDesiredAccess, p_ಹೊರಹೋಗು_Flags, p_ಹೊರಹೋಗು_Flags ?
           *p_ಹೊರಹೋಗು_Flags : 0, (h==INVALID_HANDLE_ಮೌಲ್ಯ) ? "failed" : "ok"));

  p_App_ದತ್ತಾಂಶ = (win_Vfs_App_ದತ್ತಾಂಶ*)pVfs->p_App_ದತ್ತಾಂಶ;

  sqlite3_free(zConverted);
  sqlite3_free(zTmpname);
  id->pMethods = p_App_ದತ್ತಾಂಶ ? p_App_ದತ್ತಾಂಶ->p_ವಿಧಾನ : &win_Io_ವಿಧಾನ;
  pFile->pVfs = pVfs;
  pFile->h = h;
  ಆದರೆ( isReadonly ){
    pFile->ctrlFlags |= WINFILE_RDONLY;
  }
  ಆದರೆ( (flags & SQLITE_OPEN_MAIN_DB)
   && sqlite3_uri_boolean(zName, "psow", SQLITE_POWERSAFE_OVERWRITE) 
  ){
    pFile->ctrlFlags |= WINFILE_PSOW;
  }
  pFile->ಕೊನೆಯ_Errno = NO_ERROR;
  pFile->z_ಮಾರ್ಗ = zName;
#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  pFile->hMap = ಖಾಲಿ;
  pFile->pMapRegion = 0;
  pFile->mmap_ಗಾತ್ರ = 0;
  pFile->mmap_ಗಾತ್ರ_Max = sqlite3GlobalConfig.szMmap;
#ಆದರೆ_ಅಂತ್ಯ

  OpenCounter(+1);
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winDelete(
  sqlite3_vfs *pVfs,          /* Not used on win32 */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename,      /* Name of file to delete */
  ಪೂರ್ಣಾಂಕ syncDir                 /* Not used on win32 */
){
  ಪೂರ್ಣಾಂಕ cnt = 0;
  ಪೂರ್ಣಾಂಕ rc;
  DWORD attr;
  DWORD ಕೊನೆಯ_Errno = 0;
  LPWSTR zConverted;
  ಬಳಸದ_PARAMETER(pVfs);
  ಬಳಸದ_PARAMETER(syncDir);

  SimulateIOError(ಹಿಂತಿರುಗಿ SQLITE_IOERR_DELETE);
  OSTRACE(("DELETE name=%s, syncDir=%d\n", zFilename, syncDir));

  zConverted = winConvertFromUtf8Filename(zFilename);
  ಆದರೆ( zConverted==0 ){
    OSTRACE(("DELETE name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM_BKPT;
  }
  ಮಾಡು {
    attr = os_Get_File_ಗುಣಲಕ್ಷಣಗಳು_W(zConverted);
    ಆದರೆ ( attr==INVALID_FILE_ಗುಣಲಕ್ಷಣಗಳು ){
      ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
      ಆದರೆ( ಕೊನೆಯ_Errno==ERROR_FILE_NOT_ಸಿಕ್ಕಿತು
       || ಕೊನೆಯ_Errno==ERROR_ಮಾರ್ಗ_NOT_ಸಿಕ್ಕಿತು ){
        rc = SQLITE_IOERR_DELETE_NOENT; /* Already gone? */
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = SQLITE_ERROR;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ ( attr&FILE_ATTRIBUTE_DIRECTORY ){
      rc = SQLITE_ERROR; /* Files only. */
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ ( osDeleteFileW(zConverted) ){
      rc = SQLITE_OK; /* Deleted OK. */
      ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ ( !winRetryIoerr(&cnt, &ಕೊನೆಯ_Errno) ){
      rc = SQLITE_ERROR; /* No more retries. */
      ನಿಲ್ಲಿಸು;
    }
  } ಆಗ(1);
  ಆದರೆ( rc && rc!=SQLITE_IOERR_DELETE_NOENT ){
    rc = winLogError(SQLITE_IOERR_DELETE, ಕೊನೆಯ_Errno, "winDelete", zFilename);
  }ಇಲ್ಲದಿದ್ದರೆ{
    winLogIoerr(cnt, __LINE__);
  }
  sqlite3_free(zConverted);
  OSTRACE(("DELETE name=%s, rc=%s\n", zFilename, sqlite3ErrName(rc)));
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Check the existence and status of a file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winAccess(
  sqlite3_vfs *pVfs,         /* Not used on win32 */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename,     /* Name of file to check */
  ಪೂರ್ಣಾಂಕ flags,                 /* Type of test to make on this file */
  ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು               /* OUT: Result */
){
  DWORD attr;
  ಪೂರ್ಣಾಂಕ rc = 0;
  ಪೂರ್ಣಾಂಕ cnt = 0;
  DWORD ಕೊನೆಯ_Errno = 0;
  LPWSTR zConverted;
  ಪೂರ್ಣಾಂಕ noRetry = 0;           /* Do not use winRetryIoerr() */
  WIN32_FILE_ATTRIBUTE_ದತ್ತಾಂಶ s_Attr_ದತ್ತಾಂಶ;
  ಬಳಸದ_PARAMETER(pVfs);

  ಆದರೆ( (flags & NORETRY)!=0 ){
    noRetry = 1;
    flags &= ~NORETRY;
  }  

  SimulateIOError( ಹಿಂತಿರುಗಿ SQLITE_IOERR_ACCESS; );
  OSTRACE(("ACCESS name=%s, flags=%x, pResOut=%p\n",
           zFilename, flags, p_Res_ಹೊರಹೋಗು));

  ಆದರೆ( zFilename==0 ){
    *p_Res_ಹೊರಹೋಗು = 0;
    OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
             zFilename, p_Res_ಹೊರಹೋಗು, *p_Res_ಹೊರಹೋಗು));
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }

  zConverted = winConvertFromUtf8Filename(zFilename);
  ಆದರೆ( zConverted==0 ){
    OSTRACE(("ACCESS name=%s, rc=SQLITE_IOERR_NOMEM\n", zFilename));
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM_BKPT;
  }
  memset(&s_Attr_ದತ್ತಾಂಶ, 0, ಗಾತ್ರ(s_Attr_ದತ್ತಾಂಶ));
  ಆಗ( !(rc = os_Get_File_ಗುಣಲಕ್ಷಣಗಳು_Ex_W(zConverted,
                           GetFileExInfoStandard,
                           &s_Attr_ದತ್ತಾಂಶ))
     && !noRetry
     && winRetryIoerr(&cnt, &ಕೊನೆಯ_Errno)
  ){ /* Loop until true */}
  ಆದರೆ( rc ){
    /* For an SQLITE_ACCESS_EXISTS query, treat a zero-length file
    ** as if it does not exist.
    */
    ಆದರೆ(    flags==SQLITE_ACCESS_EXISTS
        && s_Attr_ದತ್ತಾಂಶ.n_File_ಗಾತ್ರ_ಮೇಲಿನ==0
        && s_Attr_ದತ್ತಾಂಶ.n_File_ಗಾತ್ರ_ಕೆಳಗಿನ==0 ){
      attr = INVALID_FILE_ಗುಣಲಕ್ಷಣಗಳು;
    }ಇಲ್ಲದಿದ್ದರೆ{
      attr = s_Attr_ದತ್ತಾಂಶ.dw_File_ಗುಣಲಕ್ಷಣಗಳು;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( noRetry ) ಕೊನೆಯ_Errno = os_Get_ಕೊನೆಯ_Error();
    winLogIoerr(cnt, __LINE__);
    ಆದರೆ( ಕೊನೆಯ_Errno!=ERROR_FILE_NOT_ಸಿಕ್ಕಿತು && ಕೊನೆಯ_Errno!=ERROR_ಮಾರ್ಗ_NOT_ಸಿಕ್ಕಿತು ){
      sqlite3_free(zConverted);
      ಹಿಂತಿರುಗಿ winLogError(SQLITE_IOERR_ACCESS, ಕೊನೆಯ_Errno, "winAccess",
                         zFilename);
    }ಇಲ್ಲದಿದ್ದರೆ{
      attr = INVALID_FILE_ಗುಣಲಕ್ಷಣಗಳು;
    }
  }
  sqlite3_free(zConverted);
  ಆಯ್ಕೆ( flags ){
    ಸ್ಥಿತಿ SQLITE_ACCESS_READ:
    ಸ್ಥಿತಿ SQLITE_ACCESS_EXISTS:
      rc = attr!=INVALID_FILE_ಗುಣಲಕ್ಷಣಗಳು;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ SQLITE_ACCESS_READWRITE:
      rc = attr!=INVALID_FILE_ಗುಣಲಕ್ಷಣಗಳು &&
             (attr & FILE_ATTRIBUTE_READONLY)==0;
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಪ್ರತಿಜ್ಞಾ(!"Invalid flags argument");
  }
  *p_Res_ಹೊರಹೋಗು = rc;
  OSTRACE(("ACCESS name=%s, pResOut=%p, *pResOut=%d, rc=SQLITE_OK\n",
           zFilename, p_Res_ಹೊರಹೋಗು, *p_Res_ಹೊರಹೋಗು));
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Returns non-zero if the specified path name starts with the "long path"
** prefix.
*/
ಸ್ಥಿರ BOOL win_Is_Long_ಮಾರ್ಗ_ಪೂರ್ವಪ್ರತ್ಯಯ(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zPathname
){
  ಹಿಂತಿರುಗಿ ( zPathname[0]=='\\' && zPathname[1]=='\\'
        && zPathname[2]=='?'  && zPathname[3]=='\\' );
}

/*
** Returns non-zero if the specified path name starts with a drive letter
** followed by a colon character.
*/
ಸ್ಥಿರ BOOL winIsDriveLetterAndColon(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zPathname
){
  ಹಿಂತಿರುಗಿ ( sqlite3Isalpha(zPathname[0]) && zPathname[1]==':' );
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
/*
** Returns non-zero if the specified path name should be used verbatim.  If
** non-zero is returned from this function, the calling function must simply
** use the provided path name verbatim -OR- resolve it into a full path name
** using the GetFullPathName Win32 API function (if available).
*/
ಸ್ಥಿರ BOOL winIsVerbatimPathname(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zPathname
){
  /*
  ** If the path name starts with a forward slash or a backslash, it is either
  ** a legal UNC name, a volume relative path, or an absolute path name in the
  ** "Unix" format on Windows.  There is no easy way to differentiate between
  ** the final two cases; therefore, we return the safer return value of TRUE
  ** so that callers of this function will simply use it verbatim.
  */
  ಆದರೆ ( winIsDirSep(zPathname[0]) ){
    ಹಿಂತಿರುಗಿ TRUE;
  }

  /*
  ** If the path name starts with a letter and a colon it is either a volume
  ** relative path or an absolute path.  Callers of this function must not
  ** attempt to treat it as a relative path name (i.e. they should simply use
  ** it verbatim).
  */
  ಆದರೆ ( winIsDriveLetterAndColon(zPathname) ){
    ಹಿಂತಿರುಗಿ TRUE;
  }

  /*
  ** If we get to this point, the path name should almost certainly be a purely
  ** relative one (i.e. not a UNC name, not absolute, and not volume relative).
  */
  ಹಿಂತಿರುಗಿ FALSE;
}
#ಆದರೆ_ಅಂತ್ಯ /* _WIN32 */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __CYGWIN__
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
ಸ್ಥಿರ ಶೂನ್ಯ winSimplifyName(ಅಕ್ಷರ *z){
  ಪೂರ್ಣಾಂಕ i, j;
  ಗಾಗಿ(i=j=0; z[i]; ++i){
    ಆದರೆ( winIsDirSep(z[i]) ){
#ಆದರೆ !defined(SQLITE_TEST)
      /* Some test-cases assume that "./foo" and "foo" are different */
      ಆದರೆ( z[i+1]=='.' && winIsDirSep(z[i+2]) ){
        ++i;
        ಮುಂದುವರಿ;
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಆದರೆ( !z[i+1] || (winIsDirSep(z[i+1]) && (i!=0)) ){
        ಮುಂದುವರಿ;
      }
      z[j++] = osGetenv?'/':'\\';
    }ಇಲ್ಲದಿದ್ದರೆ{
      z[j++] = z[i];
    }
  }
  ಆಗ(j<i) z[j++] = '\0';
}

#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MAX_SYMLINKS 100

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ mk_ತುಂಬಿದ_Pathname(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ,              /* Input path */
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು,                     /* Output buffer */
  ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು                        /* Allocated size of buffer zOut */
){
  ಪೂರ್ಣಾಂಕ n_ಮಾರ್ಗ = sqlite3Strlen30(z_ಮಾರ್ಗ);
  ಪೂರ್ಣಾಂಕ iOff = 0;
  ಆದರೆ( z_ಮಾರ್ಗ[0]!='/' ){
    ಆದರೆ( osGetcwd(z_ಹೊರಹೋಗು, n_ಹೊರಹೋಗು-2)==0 ){
      ಹಿಂತಿರುಗಿ winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno, "getcwd", z_ಮಾರ್ಗ);
    }
    iOff = sqlite3Strlen30(z_ಹೊರಹೋಗು);
    z_ಹೊರಹೋಗು[iOff++] = '/';
  }
  ಆದರೆ( (iOff+n_ಮಾರ್ಗ+1)>n_ಹೊರಹೋಗು ){
    /* SQLite assumes that xFullPathname() nul-terminates the output buffer
    ** even if it returns an error.  */
    z_ಹೊರಹೋಗು[iOff] = '\0';
    ಹಿಂತಿರುಗಿ SQLITE_CANTOPEN_BKPT;
  }
  sqlite3_snprintf(n_ಹೊರಹೋಗು-iOff, &z_ಹೊರಹೋಗು[iOff], "%s", z_ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* __CYGWIN__ */

/*
** Turn a relative pathname into a full pathname.  Write the full
** pathname into zOut[].  zOut[] will be at least pVfs->mxPathname
** bytes in size.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_ತುಂಬಿದ_Pathname_No_Mutex(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zRelative,        /* Possibly relative input path */
  ಪೂರ್ಣಾಂಕ n_ತುಂಬಿದ,                    /* Size of output buffer in bytes */
  ಅಕ್ಷರ *z_ತುಂಬಿದ                   /* Output buffer */
){
  ಪೂರ್ಣಾಂಕ nByte;
  LPWSTR zConverted;
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು;

  /* If this path name begins with "/X:" or "\\?\", where "X" is any
  ** alphabetic character, discard the initial "/" from the pathname.
  */
  ಆದರೆ( zRelative[0]=='/' && (winIsDriveLetterAndColon(zRelative+1)
       || win_Is_Long_ಮಾರ್ಗ_ಪೂರ್ವಪ್ರತ್ಯಯ(zRelative+1)) ){
    zRelative++;
  }

  SimulateIOError( ಹಿಂತಿರುಗಿ SQLITE_ERROR );

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __CYGWIN__
  ಆದರೆ( osGetcwd ){
    z_ತುಂಬಿದ[n_ತುಂಬಿದ-1] = '\0';
    ಆದರೆ( !winIsDriveLetterAndColon(zRelative) || !winIsDirSep(zRelative[2]) ){
      ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
      ಪೂರ್ಣಾಂಕ nLink = 1;              /* Number of symbolic links followed so far */
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ = zRelative; /* Input path for each iteration of loop */
      ಅಕ್ಷರ *zDel = 0;
      ರಚನೆ stat buf;

      ಬಳಸದ_PARAMETER(pVfs);

      ಮಾಡು {
        /* Call lstat() on path zIn. Set bLink to true if the path
        ** is a symbolic link, or false otherwise.  */
        ಪೂರ್ಣಾಂಕ bLink = 0;
        ಆದರೆ( osLstat && osReadlink ) {
          ಆದರೆ( osLstat(z_ಒಳ, &buf)!=0 ){
            ಪೂರ್ಣಾಂಕ myErrno = osErrno;
            ಆದರೆ( myErrno!=ENOENT ){
              rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)myErrno,
                               "lstat", z_ಒಳ);
            }
          }ಇಲ್ಲದಿದ್ದರೆ{
            bLink = ((buf.st_ಬಹುಲಕ & 0170000) == 0120000);
          }

          ಆದರೆ( bLink ){
            ಆದರೆ( zDel==0 ){
              zDel = sqlite3MallocZero(n_ತುಂಬಿದ);
              ಆದರೆ( zDel==0 ) rc = SQLITE_NOMEM;
            }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ++nLink>SQLITE_MAX_SYMLINKS ){
              rc = SQLITE_CANTOPEN_BKPT;
            }

            ಆದರೆ( rc==SQLITE_OK ){
              nByte = osReadlink(z_ಒಳ, zDel, n_ತುಂಬಿದ-1);
              ಆದರೆ( nByte ==(DWORD)-1 ){
                rc = winLogError(SQLITE_CANTOPEN_BKPT, (DWORD)osErrno,
                                 "readlink", z_ಒಳ);
              }ಇಲ್ಲದಿದ್ದರೆ{
                ಆದರೆ( zDel[0]!='/' ){
                  ಪೂರ್ಣಾಂಕ n;
                  ಗಾಗಿ(n = sqlite3Strlen30(z_ಒಳ); n>0 && z_ಒಳ[n-1]!='/'; n--);
                  ಆದರೆ( nByte+n+1>n_ತುಂಬಿದ ){
                    rc = SQLITE_CANTOPEN_BKPT;
                  }ಇಲ್ಲದಿದ್ದರೆ{
                    memmove(&zDel[n], zDel, nByte+1);
                    ಸ್ಮರಣೆ_ನಕಲು(zDel, z_ಒಳ, n);
                    nByte += n;
                  }
                }
                zDel[nByte] = '\0';
              }
            }

            z_ಒಳ = zDel;
          }
        }

        ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || z_ಒಳ!=z_ತುಂಬಿದ || z_ಒಳ[0]=='/' );
        ಆದರೆ( rc==SQLITE_OK && z_ಒಳ!=z_ತುಂಬಿದ ){
          rc = mk_ತುಂಬಿದ_Pathname(z_ಒಳ, z_ತುಂಬಿದ, n_ತುಂಬಿದ);
        }
        ಆದರೆ( bLink==0 ) ನಿಲ್ಲಿಸು;
        z_ಒಳ = z_ತುಂಬಿದ;
      }ಆಗ( rc==SQLITE_OK );

      sqlite3_free(zDel);
      winSimplifyName(z_ತುಂಬಿದ);
      ಹಿಂತಿರುಗಿ rc;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ /* __CYGWIN__ */

#ಆದರೆ defined(_WIN32)
  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing. This function could fail if, for example, the
  ** current working directory has been unlinked.
  */
  SimulateIOError( ಹಿಂತಿರುಗಿ SQLITE_ERROR );
  ಆದರೆ ( sqlite3_ದತ್ತಾಂಶ_directory && !winIsVerbatimPathname(zRelative) ){
    /*
    ** NOTE: We are dealing with a relative path name and the data
    **       directory has been set.  Therefore, use it as the basis
    **       for converting the relative path name to an absolute
    **       one by prepending the data directory and a backslash.
    */
    sqlite3_snprintf(MIN(n_ತುಂಬಿದ, pVfs->mxPathname), z_ತುಂಬಿದ, "%s%c%s",
                     sqlite3_ದತ್ತಾಂಶ_directory, winGetDirSep(), zRelative);
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
#ಆದರೆ_ಅಂತ್ಯ
  zConverted = winConvertFromUtf8Filename(zRelative);
  ಆದರೆ( zConverted==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM_BKPT;
  }
  {
    LPWSTR z_ತಾತ್ಕಾಲಿಕ;
    nByte = os_Get_ತುಂಬಿದ_ಮಾರ್ಗ_Name_W(zConverted, 0, 0, 0);
    ಆದರೆ( nByte==0 ){
      sqlite3_free(zConverted);
      ಹಿಂತಿರುಗಿ winLogError(SQLITE_CANTOPEN_FULLPATH, os_Get_ಕೊನೆಯ_Error(),
                         "winFullPathname1", zRelative);
    }
    nByte += 3;
    z_ತಾತ್ಕಾಲಿಕ = sqlite3MallocZero( (u64)nByte*ಗಾತ್ರ(WCHAR) );
    ಆದರೆ( z_ತಾತ್ಕಾಲಿಕ==0 ){
      sqlite3_free(zConverted);
      ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM_BKPT;
    }
    nByte = os_Get_ತುಂಬಿದ_ಮಾರ್ಗ_Name_W(zConverted, nByte, z_ತಾತ್ಕಾಲಿಕ, 0);
    ಆದರೆ( nByte==0 ){
      sqlite3_free(zConverted);
      sqlite3_free(z_ತಾತ್ಕಾಲಿಕ);
      ಹಿಂತಿರುಗಿ winLogError(SQLITE_CANTOPEN_FULLPATH, os_Get_ಕೊನೆಯ_Error(),
                         "winFullPathname2", zRelative);
    }
    sqlite3_free(zConverted);
    z_ಹೊರಹೋಗು = win_Unicode_ಗಮ್ಯ_Utf8(z_ತಾತ್ಕಾಲಿಕ);
    sqlite3_free(z_ತಾತ್ಕಾಲಿಕ);
  }
  ಆದರೆ( z_ಹೊರಹೋಗು ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __CYGWIN__
    ಆದರೆ( memcmp(z_ಹೊರಹೋಗು, "\\\\?\\", 4) ){
      sqlite3_snprintf(MIN(n_ತುಂಬಿದ, pVfs->mxPathname), z_ತುಂಬಿದ, "%s", z_ಹೊರಹೋಗು);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( memcmp(z_ಹೊರಹೋಗು+4, "UNC\\", 4) ){
      sqlite3_snprintf(MIN(n_ತುಂಬಿದ, pVfs->mxPathname), z_ತುಂಬಿದ, "%s", z_ಹೊರಹೋಗು+4);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಅಕ್ಷರ *p = z_ಹೊರಹೋಗು+6;
      *p = '\\';
      ಆದರೆ( osGetcwd ){
        /* On Cygwin, UNC paths use forward slashes */
        ಆಗ( *p ){
          ಆದರೆ( *p=='\\' ) *p = '/';
          ++p;
        }
      }
      sqlite3_snprintf(MIN(n_ತುಂಬಿದ, pVfs->mxPathname), z_ತುಂಬಿದ, "%s", z_ಹೊರಹೋಗು+6);
    }
#ಇಲ್ಲದಿದ್ದರೆ
    sqlite3_snprintf(MIN(n_ತುಂಬಿದ, pVfs->mxPathname), z_ತುಂಬಿದ, "%s", z_ಹೊರಹೋಗು);
#ಆದರೆ_ಅಂತ್ಯ /* __CYGWIN__ */
    sqlite3_free(z_ಹೊರಹೋಗು);
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM_BKPT;
  }
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_ತುಂಬಿದ_Pathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zRelative,        /* Possibly relative input path */
  ಪೂರ್ಣಾಂಕ n_ತುಂಬಿದ,                    /* Size of output buffer in bytes */
  ಅಕ್ಷರ *z_ತುಂಬಿದ                   /* Output buffer */
){
  ಪೂರ್ಣಾಂಕ rc;
  MUTEX_LOGIC( sqlite3_mutex *pMutex; )
  MUTEX_LOGIC( pMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR); )
  sqlite3_mutex_enter(pMutex);
  rc = win_ತುಂಬಿದ_Pathname_No_Mutex(pVfs, zRelative, n_ತುಂಬಿದ, z_ತುಂಬಿದ);
  sqlite3_mutex_leave(pMutex);
  ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *winDlOpen(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename){
  HANDLE h = 0;
  LPWSTR zConverted = winConvertFromUtf8Filename(zFilename);
  ಬಳಸದ_PARAMETER(pVfs);
  ಆದರೆ( zConverted==0 ){
    OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (ಶೂನ್ಯ*)0));
    ಹಿಂತಿರುಗಿ 0;
  }
  h = os_ತುಂಬು_Library_W ? os_ತುಂಬು_Library_W(zConverted) : 0;
  OSTRACE(("DLOPEN name=%s, handle=%p\n", zFilename, (ಶೂನ್ಯ*)h));
  sqlite3_free(zConverted);
  ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)h;
}
ಸ್ಥಿರ ಶೂನ್ಯ winDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nBuf, ಅಕ್ಷರ *z_Buf_ಹೊರಹೋಗು){
  ಬಳಸದ_PARAMETER(pVfs);
  win_Get_ಕೊನೆಯ_Error_ಸಂದೇಶ(os_Get_ಕೊನೆಯ_Error(), nBuf, z_Buf_ಹೊರಹೋಗು);
}
ಸ್ಥಿರ ಶೂನ್ಯ (*winDlSym(sqlite3_vfs *pVfs,ಶೂನ್ಯ *pH,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSym))(ಶೂನ್ಯ){
  FARPROC proc = 0;
  ಬಳಸದ_PARAMETER(pVfs);
  proc = os_Get_Proc_ವಿಳಾಸ_A ? os_Get_Proc_ವಿಳಾಸ_A((HANDLE)pH, zSym) : 0;
  OSTRACE(("DLSYM handle=%p, symbol=%s, address=%p\n",
           (ಶೂನ್ಯ*)pH, zSym, (ಶೂನ್ಯ*)proc));
  ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ(*)(ಶೂನ್ಯ))proc;
}
ಸ್ಥಿರ ಶೂನ್ಯ winDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  ಬಳಸದ_PARAMETER(pVfs);
  ಆದರೆ( osFreeLibrary!=0 ) osFreeLibrary((HANDLE)pHandle);
  OSTRACE(("DLCLOSE handle=%p\n", (ಶೂನ್ಯ*)pHandle));
}
#ಇಲ್ಲದಿದ್ದರೆ /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #ವ್ಯಾಖ್ಯಾನಿಸು winDlOpen  0
  #ವ್ಯಾಖ್ಯಾನಿಸು winDlError 0
  #ವ್ಯಾಖ್ಯಾನಿಸು winDlSym   0
  #ವ್ಯಾಖ್ಯಾನಿಸು winDlClose 0
#ಆದರೆ_ಅಂತ್ಯ

/* State information for the randomness gatherer. */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ EntropyGatherer EntropyGatherer;
ರಚನೆ EntropyGatherer {
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *a;   /* Gather entropy into this buffer */
  ಪೂರ್ಣಾಂಕ na;             /* Size of a[] in bytes */
  ಪೂರ್ಣಾಂಕ i;              /* XOR next input into a[i] */
  ಪೂರ್ಣಾಂಕ nXor;           /* Number of XOR operations done */
};

#ಆದರೆ !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
/* Mix sz bytes of entropy into p. */
ಸ್ಥಿರ ಶೂನ್ಯ xor_ಸ್ಮರಣೆ(EntropyGatherer *p, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *x, ಪೂರ್ಣಾಂಕ sz){
  ಪೂರ್ಣಾಂಕ j, k;
  ಗಾಗಿ(j=0, k=p->i; j<sz; j++){
    p->a[k++] ^= x[j];
    ಆದರೆ( k>=p->na ) k = 0;
  }
  p->i = k;
  p->nXor += sz;
}
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS) */

/*
** Write up to nBuf bytes of randomness into zBuf.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nBuf, ಅಕ್ಷರ *zBuf){
#ಆದರೆ defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS)
  ಬಳಸದ_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  ಹಿಂತಿರುಗಿ nBuf;
#ಇಲ್ಲದಿದ್ದರೆ
  EntropyGatherer e;
  ಬಳಸದ_PARAMETER(pVfs);
  memset(zBuf, 0, nBuf);
  e.a = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)zBuf;
  e.na = nBuf;
  e.nXor = 0;
  e.i = 0;
  {
    FILETIME x;
    os_Get_System_ಸಮಯ_As_File_ಸಮಯ(&x);
    xor_ಸ್ಮರಣೆ(&e, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)&x, ಗಾತ್ರ(x));
  }
  {
    DWORD pid = osGetCurrentProcessId();
    xor_ಸ್ಮರಣೆ(&e, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)&pid, ಗಾತ್ರ(pid));
  }
  {
    ULONGLONG cnt = osGetTickCount64();
    xor_ಸ್ಮರಣೆ(&e, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)&cnt, ಗಾತ್ರ(cnt));
  }
  {
    LARGE_INTEGER i;
    osQueryPerformanceCounter(&i);
    xor_ಸ್ಮರಣೆ(&e, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)&i, ಗಾತ್ರ(i));
  }
#ಆದರೆ SQLITE_WIN32_USE_UUID
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_UWP
# error SQLITE_WIN32_USE_UUID is incompatible with SQLITE_UWP
#ಆದರೆ_ಅಂತ್ಯ
  {
    UUID id;
    memset(&id, 0, ಗಾತ್ರ(id));
    osUuidCreate(&id);
    xor_ಸ್ಮರಣೆ(&e, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)&id, ಗಾತ್ರ(id));
    memset(&id, 0, ಗಾತ್ರ(UUID));
    osUuidCreateSequential(&id);
    xor_ಸ್ಮರಣೆ(&e, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)&id, ಗಾತ್ರ(id));
  }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_WIN32_USE_UUID */
  ಹಿಂತಿರುಗಿ e.nXor>nBuf ? nBuf : e.nXor;
#ಆದರೆ_ಅಂತ್ಯ /* defined(SQLITE_TEST) || defined(SQLITE_OMIT_RANDOMNESS) */
}


/*
** Sleep for a little while.  Return the amount of time slept.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ winSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ microsec){
  osSleep((microsec+999)/1000);
  ಬಳಸದ_PARAMETER(pVfs);
  ಹಿಂತಿರುಗಿ ((microsec+999)/1000)*1000;
}

/*
** The following variable, if set to a non-zero value, is interpreted as
** the number of seconds since 1970 and is used to set the result of
** sqlite3OsCurrentTime() during testing.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
ಪೂರ್ಣಾಂಕ sqlite3_current_ಸಮಯ = 0;  /* Fake system time in seconds since 1970. */
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Current_ಸಮಯ_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *piNow){
  /* FILETIME structure is a 64-bit value representing the number of
     100-nanosecond intervals since January 1, 1601 (= JD 2305813.5).
  */
  FILETIME ft;
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_int64 winFiletimeEpoch = 23058135*(sqlite3_int64)8640000;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
#ಆದರೆ_ಅಂತ್ಯ
  /* 2^32 - to avoid use of LL and warnings in gcc */
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_int64 max32_ಬಿಟ್_ಮೌಲ್ಯ =
      (sqlite3_int64)2000000000 + (sqlite3_int64)2000000000 +
      (sqlite3_int64)294967296;

  os_Get_System_ಸಮಯ_As_File_ಸಮಯ( &ft );
  *piNow = winFiletimeEpoch +
            ((((sqlite3_int64)ft.dw_ಮೇಲಿನ_Date_ಸಮಯ)*max32_ಬಿಟ್_ಮೌಲ್ಯ) +
               (sqlite3_int64)ft.dw_ಕೆಳಗಿನ_Date_ಸಮಯ)/(sqlite3_int64)10000;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  ಆದರೆ( sqlite3_current_ಸಮಯ ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_ಸಮಯ + unixEpoch;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಬಳಸದ_PARAMETER(pVfs);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Current_ಸಮಯ(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *prNow){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_int64 i;
  rc = win_Current_ಸಮಯ_Int64(pVfs, &i);
  ಆದರೆ( !rc ){
    *prNow = i/86400000.0;
  }
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ win_Get_ಕೊನೆಯ_Error(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nBuf, ಅಕ್ಷರ *zBuf){
  DWORD e = os_Get_ಕೊನೆಯ_Error();
  ಬಳಸದ_PARAMETER(pVfs);
  ಆದರೆ( nBuf>0 ) win_Get_ಕೊನೆಯ_Error_ಸಂದೇಶ(e, nBuf, zBuf);
  ಹಿಂತಿರುಗಿ e;
}

/*
** Initialize and deinitialize the operating system interface.
*/
ಪೂರ್ಣಾಂಕ sqlite3_os_init(ಶೂನ್ಯ){
  ಸ್ಥಿರ sqlite3_vfs winVfs = {
    3,                     /* iVersion */
    ಗಾತ್ರ(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_ಮಾರ್ಗ_ಬೈಟುಗಳು, /* mxPathname */
    0,                     /* pNext */
    "win32",               /* zName */
    &win_App_ದತ್ತಾಂಶ,           /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_ತುಂಬಿದ_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_ಸಮಯ,        /* xCurrentTime */
    win_Get_ಕೊನೆಯ_Error,       /* xGetLastError */
    win_Current_ಸಮಯ_Int64,   /* xCurrentTimeInt64 */
    win_ಗಣ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_ಮುಂದಿನ_System_Call,     /* xNextSystemCall */
  };
  ಸ್ಥಿರ sqlite3_vfs win_Long_ಮಾರ್ಗ_Vfs = {
    3,                     /* iVersion */
    ಗಾತ್ರ(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_ಮಾರ್ಗ_ಬೈಟುಗಳು, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath",      /* zName */
    &win_App_ದತ್ತಾಂಶ,           /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_ತುಂಬಿದ_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_ಸಮಯ,        /* xCurrentTime */
    win_Get_ಕೊನೆಯ_Error,       /* xGetLastError */
    win_Current_ಸಮಯ_Int64,   /* xCurrentTimeInt64 */
    win_ಗಣ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_ಮುಂದಿನ_System_Call,     /* xNextSystemCall */
  };
  ಸ್ಥಿರ sqlite3_vfs winNolockVfs = {
    3,                     /* iVersion */
    ಗಾತ್ರ(winFile),       /* szOsFile */
    SQLITE_WIN32_MAX_ಮಾರ್ಗ_ಬೈಟುಗಳು, /* mxPathname */
    0,                     /* pNext */
    "win32-none",          /* zName */
    &win_Nolock_App_ದತ್ತಾಂಶ,     /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_ತುಂಬಿದ_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_ಸಮಯ,        /* xCurrentTime */
    win_Get_ಕೊನೆಯ_Error,       /* xGetLastError */
    win_Current_ಸಮಯ_Int64,   /* xCurrentTimeInt64 */
    win_ಗಣ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_ಮುಂದಿನ_System_Call,     /* xNextSystemCall */
  };
  ಸ್ಥಿರ sqlite3_vfs win_Long_ಮಾರ್ಗ_Nolock_Vfs = {
    3,                     /* iVersion */
    ಗಾತ್ರ(winFile),       /* szOsFile */
    SQLITE_WINNT_MAX_ಮಾರ್ಗ_ಬೈಟುಗಳು, /* mxPathname */
    0,                     /* pNext */
    "win32-longpath-none", /* zName */
    &win_Nolock_App_ದತ್ತಾಂಶ,     /* pAppData */
    winOpen,               /* xOpen */
    winDelete,             /* xDelete */
    winAccess,             /* xAccess */
    win_ತುಂಬಿದ_Pathname,       /* xFullPathname */
    winDlOpen,             /* xDlOpen */
    winDlError,            /* xDlError */
    winDlSym,              /* xDlSym */
    winDlClose,            /* xDlClose */
    winRandomness,         /* xRandomness */
    winSleep,              /* xSleep */
    win_Current_ಸಮಯ,        /* xCurrentTime */
    win_Get_ಕೊನೆಯ_Error,       /* xGetLastError */
    win_Current_ಸಮಯ_Int64,   /* xCurrentTimeInt64 */
    win_ಗಣ_System_Call,      /* xSetSystemCall */
    winGetSystemCall,      /* xGetSystemCall */
    win_ಮುಂದಿನ_System_Call,     /* xNextSystemCall */
  };

  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  ಪ್ರತಿಜ್ಞಾ( Array_ಗಾತ್ರ(aSyscall)==60 );
  ಪ್ರತಿಜ್ಞಾ( strcmp(aSyscall[0].zName,"AreFileApisANSI")==0 );
  ಪ್ರತಿಜ್ಞಾ( strcmp(aSyscall[8].zName,"FreeLibrary")==0 );
  ಪ್ರತಿಜ್ಞಾ( strcmp(aSyscall[16].zName,"GetSystemInfo")==0 );
  ಪ್ರತಿಜ್ಞಾ( strcmp(aSyscall[24].zName,"HeapReAlloc")==0 );
  ಪ್ರತಿಜ್ಞಾ( strcmp(aSyscall[32].zName,"MapViewOfFileFromApp")==0 );
  ಪ್ರತಿಜ್ಞಾ( strcmp(aSyscall[40].zName,"UnmapViewOfFile")==0 );
  ಪ್ರತಿಜ್ಞಾ( strcmp(aSyscall[48].zName,"UuidCreate")==0 );
  ಪ್ರತಿಜ್ಞಾ( strcmp(aSyscall[56].zName,"lstat")==0 );

  /* get memory map allocation granularity */
  memset(&winSysInfo, 0, ಗಾತ್ರ(SYSTEM_INFO));
  osGetSystemInfo(&winSysInfo);
  ಪ್ರತಿಜ್ಞಾ( winSysInfo.dw_ಹಂಚಿಕೆ_Granularity>0 );
  ಪ್ರತಿಜ್ಞಾ( winSysInfo.dw_ಪುಟ_ಗಾತ್ರ>0 );

  sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&winVfs, 1);
  sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&win_Long_ಮಾರ್ಗ_Vfs, 0);
  sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&winNolockVfs, 0);
  sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&win_Long_ಮಾರ್ಗ_Nolock_Vfs, 0);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  win_Big_ಬೀಗ = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಪೂರ್ಣಾಂಕ sqlite3_os_ಕೊನೆ(ಶೂನ್ಯ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  win_Big_ಬೀಗ = 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OS_WIN */
