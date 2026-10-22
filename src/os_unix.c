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
** This file contains the VFS implementation for unix-like operating systems
** include Linux, MacOSX, *BSD, QNX, VxWorks, AIX, HPUX, and others.
**
** There are actually several different VFS implementations in this file.
** The differences are in the way that file locking is done.  The default
** implementation uses Posix Advisory Locks.  Alternative implementations
** use flock(), dot-files, various proprietary locking schemas, or simply
** skip locking all together.
**
** This source file is organized into divisions where the logic for various
** subfunctions is contained within the appropriate division.  PLEASE
** KEEP THE STRUCTURE OF THIS FILE INTACT.  New code should be placed
** in the correct division and should be clearly labelled.
**
** The layout of divisions is as follows:
**
**   *  General-purpose declarations and utility functions.
**   *  Unique file ID logic used by VxWorks.
**   *  Various locking primitive implementations (all except proxy locking):
**      + for Posix Advisory Locks
**      + for no-op locks
**      + for dot-file locks
**      + for flock() locking
**      + for named semaphore locks (VxWorks only)
**      + for AFP filesystem locks (MacOSX only)
**   *  sqlite3_file methods not associated with locking.
**   *  Definitions of sqlite3_io_methods objects for all locking
**      methods plus "finder" functions for each locking method.
**   *  sqlite3_vfs method implementations.
**   *  Locking primitives for the proxy uber-locking-method. (MacOSX only)
**   *  Definitions of sqlite3_vfs objects for all locking methods
**      plus implementations of sqlite3_os_init() and sqlite3_os_end().
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਜੇ SQLITE_OS_UNIX              /* This file is used on unix only */

/*
** There are various methods for file locking used for concurrency
** control:
**
**   1. POSIX locking (the default),
**   2. No locking,
**   3. Dot-file locking,
**   4. flock() locking,
**   5. AFP locking (OSX only),
**   6. Named POSIX semaphores (VXWorks only),
**   7. proxy locking. (OSX only)
**
** Styles 4, 5, and 7 are only available of SQLITE_ENABLE_LOCKING_STYLE
** is defined to 1.  The SQLITE_ENABLE_LOCKING_STYLE also enables automatic
** selection of the appropriate locking style based on the filesystem
** where the database is located.
*/
#ਜੇ !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  ਜੇ defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  ਨਹੀਂਤਾਂ
#    define SQLITE_ENABLE_LOCKING_STYLE 0
#  endif
#ਜੇ_ਅੰਤ

/* Use pread() and pwrite() if they are available */
#ਜੇ defined(__APPLE__) || defined(__linux__)
# define HAVE_PREAD 1
# define HAVE_PWRITE 1
#ਜੇ_ਅੰਤ
#ਜੇ defined(HAVE_PREAD64) && defined(HAVE_PWRITE64)
# undef USE_PREAD
# define USE_PREAD64 1
#ਜਾਂਜੇ defined(HAVE_PREAD) && defined(HAVE_PWRITE)
# undef USE_PREAD64
# define USE_PREAD 1
#ਜੇ_ਅੰਤ

/*
** standard include files.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>   /* amalgamator: keep */
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>    /* amalgamator: keep */
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/ioctl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>      /* amalgamator: keep */
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/time.h>    /* amalgamator: keep */
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਜੇ (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_ਮਾਪ>0) \
  && !defined(SQLITE_WASI)
# include <sys/mman.h>
#ਜੇ_ਅੰਤ

#ਜੇ SQLITE_ENABLE_LOCKING_STYLE
# include <sys/ioctl.h>
# include <sys/file.h>
# include <sys/param.h>
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_LOCKING_STYLE */

/*
** Try to determine if gethostuuid() is available based on standard
** macros.  This might sometimes compute the wrong value for some
** obscure platforms.  For those cases, simply compile with one of
** the following:
**
**    -DHAVE_GETHOSTUUID=0
**    -DHAVE_GETHOSTUUID=1
**
** None if this matters except when building on Apple products with
** -DSQLITE_ENABLE_LOCKING_STYLE.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HAVE_GETHOSTUUID
# define HAVE_GETHOSTUUID 0
# ਜੇ defined(__APPLE__) && ((__MAC_OS_X_VERSION_MIN_REQUIRED > 1050) || \
                            (__IPHONE_OS_VERSION_MIN_REQUIRED > 2000))
#    ਜੇ (!defined(ਨਿਸ਼ਾਨਾ_OS_EMBEDDED) || (ਨਿਸ਼ਾਨਾ_OS_EMBEDDED==0)) \
        && (!defined(ਨਿਸ਼ਾਨਾ_IPHONE_SIMULATOR) || (ਨਿਸ਼ਾਨਾ_IPHONE_SIMULATOR==0))\
        && (!defined(ਨਿਸ਼ਾਨਾ_OS_MACCATALYST) || (ਨਿਸ਼ਾਨਾ_OS_MACCATALYST==0))
#      undef HAVE_GETHOSTUUID
#      define HAVE_GETHOSTUUID 1
#    ਨਹੀਂਤਾਂ
#      warning "gethostuuid() is disabled."
#    endif
#  endif
#ਜੇ_ਅੰਤ


#ਜੇ OS_VXWORKS
# include <sys/ioctl.h>
# include <semaphore.h>
# include <limits.h>
#ਜੇ_ਅੰਤ /* OS_VXWORKS */

#ਜੇ defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
# include <sys/mount.h>
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_UTIME
# include <utime.h>
#ਜੇ_ਅੰਤ

/*
** Allowed values of unixFile.fsFlags
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_FSFLAGS_IS_MSDOS     0x1

/*
** If we are to be thread-safe, include the pthreads header.
*/
#ਜੇ SQLITE_THREADSAFE
# include <pthread.h>
#ਜੇ_ਅੰਤ

/*
** Default permissions when creating a new file
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_FILE_PERMISSIONS
# define SQLITE_DEFAULT_FILE_PERMISSIONS 0644
#ਜੇ_ਅੰਤ

/*
** Default permissions when creating auto proxy dir
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_PROXYDIR_PERMISSIONS
# define SQLITE_DEFAULT_PROXYDIR_PERMISSIONS 0755
#ਜੇ_ਅੰਤ

/*
** Maximum supported path-length.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_PATHNAME 512

/*
** Maximum supported symbolic links
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MAX_SYMLINKS 100

/*
** Remove and stub certain info for WASI (WebAssembly System
** Interface) builds.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_WASI
# undef HAVE_FCHMOD
# undef HAVE_FCHOWN
# undef HAVE_MREMAP
# define HAVE_MREMAP 0
# ifndef SQLITE_DEFAULT_UNIX_VFS
#  define SQLITE_DEFAULT_UNIX_VFS "unix-dotfile"
   /* ^^^ should SQLITE_DEFAULT_UNIX_VFS be "unix-none"? */
# endif
# ifndef F_RDLCK
#  define F_RDLCK 0
#  define F_WRLCK 1
#  define F_UNLCK 2
#  ਜੇ __LONG_MAX == 0x7fffffffL
#   define F_GETLK 12
#   define F_SETLK 13
#   define F_SETLKW 14
#  ਨਹੀਂਤਾਂ
#   define F_GETLK 5
#   define F_SETLK 6
#   define F_SETLKW 7
#  endif
# endif
#ਨਹੀਂਤਾਂ /* !SQLITE_WASI */
# ifndef HAVE_FCHMOD
#  define HAVE_FCHMOD 1
# endif
#ਜੇ_ਅੰਤ /* SQLITE_WASI */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_WASI
# define osGetpid(X) (pid_t)1
#ਨਹੀਂਤਾਂ
/* Always cast the getpid() return type for compatibility with
** kernel modules in VxWorks. */
# define osGetpid(X) (pid_t)getpid()
#ਜੇ_ਅੰਤ

/*
** Only set the lastErrno if the error code is a real error and not
** a normal expected return code of SQLITE_BUSY or SQLITE_OK
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_ਤਾਲਾ_ERROR(x)  ((x != SQLITE_OK) && (x != SQLITE_BUSY))

/* Forward references */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ unixShm unixShm;               /* Connection shared memory */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ unix_Shm_ਗੰਢ unix_Shm_ਗੰਢ;       /* Shared memory instance */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ unixInodeInfo unixInodeInfo;   /* An i-node */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Unix_ਅਣਵਰਤਿਆ_Fd Unix_ਅਣਵਰਤਿਆ_Fd;     /* An unused file descriptor */

/*
** Sometimes, after a file handle is closed by SQLite, the file descriptor
** cannot be closed immediately. In these cases, instances of the following
** structure are used to store the file descriptor while waiting for an
** opportunity to either close or reuse it.
*/
ਬਣਤਰ Unix_ਅਣਵਰਤਿਆ_Fd {
  ਪੂਰਨ fd;                   /* File descriptor to close */
  ਪੂਰਨ flags;                /* Flags this file descriptor was opened with */
  Unix_ਅਣਵਰਤਿਆ_Fd *p_ਅਗਲਾ;      /* Next unused file descriptor on same file */
};

/*
** The unixFile structure is subclass of sqlite3_file specific to the unix
** VFS implementations.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ unixFile unixFile;
ਬਣਤਰ unixFile {
  sqlite3_io_methods ਅਚਲ *p_ਵਿਧੀ;  /* Always the first entry */
  sqlite3_vfs *pVfs;                  /* The VFS that created this unixFile */
  unixInodeInfo *pInode;              /* Info about locks on this inode */
  ਪੂਰਨ h;                              /* The file descriptor */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ e_File_ਤਾਲਾ;            /* The type of lock held on this fd */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਪੂਰਨ ctrlFlags;       /* Behavioral bits.  UNIXFILE_* flags */
  ਪੂਰਨ ਆਖਰੀ_Errno;                      /* The unix errno from last I/O error */
  ਖਾਲੀ *lockingContext;               /* Locking style specific state */
  Unix_ਅਣਵਰਤਿਆ_Fd *p_Preallocated_ਅਣਵਰਤਿਆ;  /* Pre-allocated UnixUnusedFd */
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ;                  /* Name of the file */
  unixShm *pShm;                      /* Shared memory segment information */
  ਪੂਰਨ szChunk;                        /* Configured by FCNTL_CHUNK_SIZE */
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  ਪੂਰਨ n_Fetch_ਬਾਹਰਲਾ;                      /* Number of outstanding xFetch refs */
  sqlite3_int64 mmap_ਮਾਪ;             /* Usable size of mapping at pMapRegion */
  sqlite3_int64 mmap_ਮਾਪ_Actual;       /* Actual size of mapping at pMapRegion */
  sqlite3_int64 mmap_ਮਾਪ_Max;          /* Configured FCNTL_MMAP_SIZE value */
  ਖਾਲੀ *pMapRegion;                   /* Memory mapped region */
#ਜੇ_ਅੰਤ
  ਪੂਰਨ sector_ਮਾਪ;                     /* Device sector size */
  ਪੂਰਨ deviceCharacteristics;          /* Precomputed device characteristics */
#ਜੇ SQLITE_ENABLE_LOCKING_STYLE
  ਪੂਰਨ openFlags;                      /* The flags specified at open() */
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_ENABLE_LOCKING_STYLE || defined(__APPLE__)
  ਬਿਨਾਨਿਸ਼ਾਨੀ fsFlags;                   /* cached details from statfs() */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
  ਬਿਨਾਨਿਸ਼ਾਨੀ iBusyTimeout;              /* Wait this many millisec on locks */
  ਪੂਰਨ b_ਖੰਡ_On_Connect;                /* True to block for SHARED locks */
#ਜੇ_ਅੰਤ
#ਜੇ OS_VXWORKS
  ਬਣਤਰ vxworksFileId *pId;          /* Unique file ID */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* The next group of variables are used to track whether or not the
  ** transaction counter in bytes 24-27 of database files are updated
  ** whenever any part of the database changes.  An assertion fault will
  ** occur if a file is updated without also updating the transaction
  ** counter.  This test is made to avoid new problems similar to the
  ** one described by ticket #3584.
  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ transCntrChng;   /* True if the transaction counter changed */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ dbUpdate;        /* True if any part of database file changed */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਵਿੱਚ_ਸਾਧਾਰਨ_Write;   /* True if in a normal write operation */

#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  /* In test mode, increase the size of this structure a bit so that
  ** it is larger than the struct CrashFile defined in test6.c.
  */
  ਅੱਖਰ aPadding[32];
#ਜੇ_ਅੰਤ
};

/* This variable holds the process id (pid) from when the xRandomness()
** method was called.  If xOpen() is called from a different process id,
** indicating that a fork() has occurred, the PRNG will be reset.
*/
ਸਥਿਰ pid_t randomnessPid = 0;

/*
** Allowed values for the unixFile.ctrlFlags bitmask:
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIXFILE_EXCL        0x01     /* Connections from one process only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIXFILE_RDONLY      0x02     /* Connection is read only */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIXFILE_PERSIST_WAL 0x04     /* Persistent WAL mode */
#ਜੇ !defined(SQLITE_DISABLE_DIRSYNC) && !defined(_AIX)
# define UNIXFILE_DIRSYNC    0x08     /* Directory sync needed */
#ਨਹੀਂਤਾਂ
# define UNIXFILE_DIRSYNC    0x00
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIXFILE_PSOW        0x10     /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIXFILE_DELETE      0x20     /* Delete on close */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIXFILE_URI         0x40     /* Filename might have query parameters */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIXFILE_NOLOCK      0x80     /* Do no file locking */

/*
** Include code that is common to all os_*.c files
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "os_common.h"

/*
** Define various macros that are missing from some systems.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ O_LARGEFILE
# define O_LARGEFILE 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DISABLE_LFS
# undef O_LARGEFILE
# define O_LARGEFILE 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ O_NOFOLLOW
# define O_NOFOLLOW 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ O_ਦੁਆਧਾਰੀ
# define O_ਦੁਆਧਾਰੀ 0
#ਜੇ_ਅੰਤ

/*
** The threadid macro resolves to the thread-id or to 0.  Used for
** testing and debugging only.
*/
#ਜੇ SQLITE_THREADSAFE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ threadid pthread_self()
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ threadid 0
#ਜੇ_ਅੰਤ

/*
** HAVE_MREMAP defaults to true on Linux and false everywhere else.
*/
#ਜੇ !defined(HAVE_MREMAP)
# ਜੇ defined(__linux__) && defined(GNU_ਸਰੋਤ)
#  define HAVE_MREMAP 1
# ਨਹੀਂਤਾਂ
#  define HAVE_MREMAP 0
# endif
#ਜੇ_ਅੰਤ

/*
** Explicitly call the 64-bit version of lseek() on Android. Otherwise, lseek()
** is the 32-bit version, even if _FILE_OFFSET_BITS=64 is defined.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __ANDROID__
# define lseek lseek64
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __linux__
/*
** Linux-specific IOCTL magic numbers used for controlling F2FS
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F2FS_IOCTL_MAGIC        0xf5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F2_FS_IOC_ਸ਼ੁਰੂ_ATOMIC_WRITE     _IO(F2FS_IOCTL_MAGIC, 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F2FS_IOC_COMMIT_ATOMIC_WRITE    _IO(F2FS_IOCTL_MAGIC, 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F2_FS_IOC_ਸ਼ੁਰੂ_VOLATILE_WRITE   _IO(F2FS_IOCTL_MAGIC, 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F2FS_IOC_ABORT_VOLATILE_WRITE   _IO(F2FS_IOCTL_MAGIC, 5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F2FS_IOC_GET_FEATURES           _IOR(F2FS_IOCTL_MAGIC, 12, u32)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ F2FS_FEATURE_ATOMIC_WRITE 0x0004
#ਜੇ_ਅੰਤ /* __linux__ */


/*
** Different Unix systems declare open() in different ways.  Same use
** open(const char*,int,mode_t).  Others use open(const char*,int,...).
** The difference is important when using a pointer to the function.
**
** The safest way to deal with the problem is to always use this wrapper
** which always has the same well-defined interface.
*/
ਸਥਿਰ ਪੂਰਨ posixOpen(ਅਚਲ ਅੱਖਰ *zFile, ਪੂਰਨ flags, ਪੂਰਨ ਬਹੁਲਕ){
  ਵਾਪਸ open(zFile, flags, ਬਹੁਲਕ);
}

/* Forward reference */
ਸਥਿਰ ਪੂਰਨ openDirectory(ਅਚਲ ਅੱਖਰ*, ਪੂਰਨ*);
ਸਥਿਰ ਪੂਰਨ unixGetpagesize(ਖਾਲੀ);

/*
** Many system calls are accessed through pointer-to-functions so that
** they may be overridden at runtime to facilitate fault injection during
** testing and sandboxing.  The following array holds the names and pointers
** to all overrideable system calls.
*/
ਸਥਿਰ ਬਣਤਰ unix_syscall {
  ਅਚਲ ਅੱਖਰ *zName;            /* Name of the system call */
  sqlite3_syscall_ptr pCurrent; /* Current value of the system call */
  sqlite3_syscall_ptr pDefault; /* Default value */
} aSyscall[] = {
  { "open",         (sqlite3_syscall_ptr)posixOpen,  0  },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osOpen      ((ਪੂਰਨ(*)(ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ,ਪੂਰਨ))aSyscall[0].pCurrent)

  { "close",        (sqlite3_syscall_ptr)close,      0  },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osClose     ((ਪੂਰਨ(*)(ਪੂਰਨ))aSyscall[1].pCurrent)

  { "access",       (sqlite3_syscall_ptr)access,     0  },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osAccess    ((ਪੂਰਨ(*)(ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ))aSyscall[2].pCurrent)

  { "getcwd",       (sqlite3_syscall_ptr)getcwd,     0  },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osGetcwd    ((ਅੱਖਰ*(*)(ਅੱਖਰ*,size_t))aSyscall[3].pCurrent)

  { "stat",         (sqlite3_syscall_ptr)stat,       0  },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osStat      ((ਪੂਰਨ(*)(ਅਚਲ ਅੱਖਰ*,ਬਣਤਰ stat*))aSyscall[4].pCurrent)

/*
** The DJGPP compiler environment looks mostly like Unix, but it
** lacks the fcntl() system call.  So redefine fcntl() to be something
** that always succeeds.  This means that locking does not occur under
** DJGPP.  But it is DOS - what did you expect?
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __DJGPP__
  { "fstat",        0,                 0  },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osFstat(a,b,c)    0
#ਨਹੀਂਤਾਂ   
  { "fstat",        (sqlite3_syscall_ptr)fstat,      0  },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osFstat     ((ਪੂਰਨ(*)(ਪੂਰਨ,ਬਣਤਰ stat*))aSyscall[5].pCurrent)
#ਜੇ_ਅੰਤ

  { "ftruncate",    (sqlite3_syscall_ptr)ftruncate,  0  },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osFtruncate ((ਪੂਰਨ(*)(ਪੂਰਨ,off_t))aSyscall[6].pCurrent)

  { "fcntl",        (sqlite3_syscall_ptr)fcntl,      0  },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osFcntl     ((ਪੂਰਨ(*)(ਪੂਰਨ,ਪੂਰਨ,...))aSyscall[7].pCurrent)

  { "read",         (sqlite3_syscall_ptr)ਪਠ,       0  },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osRead      ((ssize_t(*)(ਪੂਰਨ,ਖਾਲੀ*,size_t))aSyscall[8].pCurrent)

#ਜੇ defined(USE_PREAD) || SQLITE_ENABLE_LOCKING_STYLE
  { "pread",        (sqlite3_syscall_ptr)pread,      0  },
#ਨਹੀਂਤਾਂ
  { "pread",        (sqlite3_syscall_ptr)0,          0  },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osPread     ((ssize_t(*)(ਪੂਰਨ,ਖਾਲੀ*,size_t,off_t))aSyscall[9].pCurrent)

#ਜੇ defined(USE_PREAD64)
  { "pread64",      (sqlite3_syscall_ptr)pread64,    0  },
#ਨਹੀਂਤਾਂ
  { "pread64",      (sqlite3_syscall_ptr)0,          0  },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osPread64 ((ssize_t(*)(ਪੂਰਨ,ਖਾਲੀ*,size_t,off64_t))aSyscall[10].pCurrent)

  { "write",        (sqlite3_syscall_ptr)ਲਿਖ,      0  },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osWrite     ((ssize_t(*)(ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,size_t))aSyscall[11].pCurrent)

#ਜੇ defined(USE_PREAD) || SQLITE_ENABLE_LOCKING_STYLE
  { "pwrite",       (sqlite3_syscall_ptr)pwrite,     0  },
#ਨਹੀਂਤਾਂ
  { "pwrite",       (sqlite3_syscall_ptr)0,          0  },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osPwrite    ((ssize_t(*)(ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,size_t,off_t))\
                    aSyscall[12].pCurrent)

#ਜੇ defined(USE_PREAD64)
  { "pwrite64",     (sqlite3_syscall_ptr)pwrite64,   0  },
#ਨਹੀਂਤਾਂ
  { "pwrite64",     (sqlite3_syscall_ptr)0,          0  },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osPwrite64  ((ssize_t(*)(ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,size_t,off64_t))\
                    aSyscall[13].pCurrent)

#ਜੇ defined(HAVE_FCHMOD)
  { "fchmod",       (sqlite3_syscall_ptr)fchmod,          0  },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osFchmod    ((ਪੂਰਨ(*)(ਪੂਰਨ,ਬਹੁਲਕ_t))aSyscall[14].pCurrent)
#ਨਹੀਂਤਾਂ
  { "fchmod",       (sqlite3_syscall_ptr)0,               0  },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osFchmod(FID,ਬਹੁਲਕ_2) 0
#ਜੇ_ਅੰਤ

#ਜੇ defined(HAVE_POSIX_FALLOCATE) && HAVE_POSIX_FALLOCATE
  { "fallocate",    (sqlite3_syscall_ptr)posix_fallocate,  0 },
#ਨਹੀਂਤਾਂ
  { "fallocate",    (sqlite3_syscall_ptr)0,                0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osFallocate ((ਪੂਰਨ(*)(ਪੂਰਨ,off_t,off_t))aSyscall[15].pCurrent)

  { "unlink",       (sqlite3_syscall_ptr)unlink,           0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osUnlink    ((ਪੂਰਨ(*)(ਅਚਲ ਅੱਖਰ*))aSyscall[16].pCurrent)

  { "openDirectory",    (sqlite3_syscall_ptr)openDirectory,      0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osOpenDirectory ((ਪੂਰਨ(*)(ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ*))aSyscall[17].pCurrent)

  { "mkdir",        (sqlite3_syscall_ptr)mkdir,           0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osMkdir     ((ਪੂਰਨ(*)(ਅਚਲ ਅੱਖਰ*,ਬਹੁਲਕ_t))aSyscall[18].pCurrent)

  { "rmdir",        (sqlite3_syscall_ptr)rmdir,           0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osRmdir     ((ਪੂਰਨ(*)(ਅਚਲ ਅੱਖਰ*))aSyscall[19].pCurrent)

#ਜੇ defined(HAVE_FCHOWN)
  { "fchown",       (sqlite3_syscall_ptr)fchown,          0 },
#ਨਹੀਂਤਾਂ
  { "fchown",       (sqlite3_syscall_ptr)0,               0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osFchown    ((ਪੂਰਨ(*)(ਪੂਰਨ,uid_t,gid_t))aSyscall[20].pCurrent)

#ਜੇ defined(HAVE_FCHOWN)
  { "geteuid",      (sqlite3_syscall_ptr)geteuid,         0 },
#ਨਹੀਂਤਾਂ
  { "geteuid",      (sqlite3_syscall_ptr)0,               0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osGeteuid   ((uid_t(*)(ਖਾਲੀ))aSyscall[21].pCurrent)

#ਜੇ (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_ਮਾਪ>0) \
  && !defined(SQLITE_WASI)
  { "mmap",         (sqlite3_syscall_ptr)mmap,            0 },
#ਨਹੀਂਤਾਂ
  { "mmap",         (sqlite3_syscall_ptr)0,               0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osMmap ((ਖਾਲੀ*(*)(ਖਾਲੀ*,size_t,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ,off_t))aSyscall[22].pCurrent)

#ਜੇ (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_ਮਾਪ>0) \
  && !defined(SQLITE_WASI)
  { "munmap",       (sqlite3_syscall_ptr)munmap,          0 },
#ਨਹੀਂਤਾਂ
  { "munmap",       (sqlite3_syscall_ptr)0,               0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osMunmap ((ਪੂਰਨ(*)(ਖਾਲੀ*,size_t))aSyscall[23].pCurrent)

#ਜੇ HAVE_MREMAP && (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_ਮਾਪ>0)
  { "mremap",       (sqlite3_syscall_ptr)mremap,          0 },
#ਨਹੀਂਤਾਂ
  { "mremap",       (sqlite3_syscall_ptr)0,               0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osMremap ((ਖਾਲੀ*(*)(ਖਾਲੀ*,size_t,size_t,ਪੂਰਨ,...))aSyscall[24].pCurrent)

#ਜੇ !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_ਮਾਪ>0
  { "getpagesize",  (sqlite3_syscall_ptr)unixGetpagesize, 0 },
#ਨਹੀਂਤਾਂ
  { "getpagesize",  (sqlite3_syscall_ptr)0,               0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osGetpagesize ((ਪੂਰਨ(*)(ਖਾਲੀ))aSyscall[25].pCurrent)

#ਜੇ defined(HAVE_READLINK)
  { "readlink",     (sqlite3_syscall_ptr)readlink,        0 },
#ਨਹੀਂਤਾਂ
  { "readlink",     (sqlite3_syscall_ptr)0,               0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osReadlink ((ssize_t(*)(ਅਚਲ ਅੱਖਰ*,ਅੱਖਰ*,size_t))aSyscall[26].pCurrent)

#ਜੇ defined(HAVE_LSTAT)
  { "lstat",         (sqlite3_syscall_ptr)lstat,          0 },
#ਨਹੀਂਤਾਂ
  { "lstat",         (sqlite3_syscall_ptr)0,              0 },
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osLstat      ((ਪੂਰਨ(*)(ਅਚਲ ਅੱਖਰ*,ਬਣਤਰ stat*))aSyscall[27].pCurrent)

#ਜੇ defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
# ifdef __ANDROID__
  { "ioctl", (sqlite3_syscall_ptr)(ਪੂਰਨ(*)(ਪੂਰਨ, ਪੂਰਨ, ...))ioctl, 0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osIoctl ((ਪੂਰਨ(*)(ਪੂਰਨ,ਪੂਰਨ,...))aSyscall[28].pCurrent)
# ਨਹੀਂਤਾਂ
  { "ioctl",         (sqlite3_syscall_ptr)ioctl,          0 },
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osIoctl ((ਪੂਰਨ(*)(ਪੂਰਨ,ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ,...))aSyscall[28].pCurrent)
# endif
#ਨਹੀਂਤਾਂ
  { "ioctl",         (sqlite3_syscall_ptr)0,              0 },
#ਜੇ_ਅੰਤ

}; /* End of the overrideable system calls */


#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Extract Posix Advisory Locking information about file description fd
** from the /proc/PID/fdinfo/FD pseudo-file.  Fill the string buffer a[16]
** with characters to indicate which SQLite-relevant locks are held.
** a[16] will be a 15-character zero-terminated string with the following
** schema:
**
**     AAA/B.DDD.DDDDD
**
** Each of character A-D will be "w" or "r" or "-" to indicate either a
** write-lock, a read-lock, or no-lock, respectively.  The "." and "/"
** characters are delimiters intended to make the string more easily
** readable by humans.  Here are the meaning of the specific letters:
**
**     AAA   ->    The main database locks.  PENDING_BYTE, RESERVED_BYTE,
**                 and SHARED_FIRST, respectively.
**
**     B     ->    The deadman switch lock.  Offset 128 of the -shm file.
**
**     CCC   ->    WAL locks:  WRITE, CKPT, RECOVER
**
**     DDDDD ->    WAL read-locks 0 through 5
**
** Note that elements before the "/" apply to the main database file and
** elements after the "/" apply to the -shm file in WAL mode.
**
** Here is another way of thinking about the meaning of the result string:
**
**           AAA/B.CCC.DDDDD
**           ||| | ||| \___/
**  PENDING--'|| | |||   `----- READ 0-5
**  RESERVED--'| | ||`---- RECOVER
**  SHARED ----' | |`----- CKPT
**     DMS ------' `------ WRITE
**
** Return SQLITE_OK on success and SQLITE_ERROR_UNABLE if the /proc
** pseudo-filesystem is unavailable.
*/
ਸਥਿਰ ਪੂਰਨ unixPosixAdvisoryLocks(
  ਪੂਰਨ fd,        /* The file descriptor to analyze */
  ਅੱਖਰ a[16]     /* Write a text description of PALs here */
){
  ਪੂਰਨ ਵਿੱਚ;
  ssize_t n;
  ਅੱਖਰ *p, *p_ਅਗਲਾ, *x;
  ਅੱਖਰ z[2000];

        /*             1     */
        /*   012 4 678 01234 */
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(a, "---/-.---.-----", 16);
  sqlite3_snprintf(ਆਕਾਰ(z), z, "/proc/%d/fdinfo/%d", getpid(), fd);
  ਵਿੱਚ = osOpen(z, O_RDONLY, 0);
  ਜੇ( ਵਿੱਚ<0 ){
    ਵਾਪਸ SQLITE_ERROR_UNABLE;
  }
  n = osRead(ਵਿੱਚ, z, ਆਕਾਰ(z)-1);
  osClose(ਵਿੱਚ);
  ਜੇ( n<=0 ) ਵਾਪਸ SQLITE_ERROR_UNABLE;
  z[n] = 0;

  /* We are looking for lines that begin with "lock:\t".  Examples:
  **
  ** lock: 1: POSIX  ADVISORY  READ 494716 08:02:5277597 1073741826 1073742335
  ** lock: 1: POSIX  ADVISORY  WRITE 494716 08:02:5282282 120 120
  ** lock: 2: POSIX  ADVISORY  READ 494716 08:02:5282282 123 123
  ** lock: 3: POSIX  ADVISORY  READ 494716 08:02:5282282 128 128
  */
  p_ਅਗਲਾ = ਲੜੀ_ਖੋਜ(z, "lock:\t");
  ਜਦੋਂ( p_ਅਗਲਾ ){
    ਅੱਖਰ cType = 0;
    sqlite3_int64 i_ਪਹਿਲਾ, i_ਆਖਰੀ;
    p = p_ਅਗਲਾ+6;
    p_ਅਗਲਾ = ਲੜੀ_ਖੋਜ(p, "lock:\t");
    ਜੇ( p_ਅਗਲਾ ) p_ਅਗਲਾ[-1] = 0;
    ਜੇ( (x = ਲੜੀ_ਖੋਜ(p, " READ "))!=0 ){
      cType = 'r';
      x += 6;
    }ਨਹੀਂਤਾਂ ਜੇ( (x = ਲੜੀ_ਖੋਜ(p, " WRITE "))!=0 ){
      cType = 'w';
      x += 7;
    }ਨਹੀਂਤਾਂ{
      ਜਾਰੀ;
    }
    x = strrchr(x, ' ');
    ਜੇ( x==0 ) ਜਾਰੀ;
    i_ਆਖਰੀ = strtoll(x+1, 0, 10);
    *x = 0;
    x = strrchr(p, ' ');
    ਜੇ( x==0 ) ਜਾਰੀ;
    i_ਪਹਿਲਾ = strtoll(x+1, 0, 10);
    ਜੇ( i_ਆਖਰੀ>=PENDING_BYTE ){
      ਜੇ( i_ਪਹਿਲਾ<=PENDING_BYTE && i_ਆਖਰੀ>=PENDING_BYTE )     a[0] = cType;
      ਜੇ( i_ਪਹਿਲਾ<=PENDING_BYTE+1 && i_ਆਖਰੀ>=PENDING_BYTE+1 ) a[1] = cType;
      ਜੇ( i_ਪਹਿਲਾ<=PENDING_BYTE+2 && i_ਆਖਰੀ>=PENDING_BYTE+510 ) a[2] = cType;
    }ਨਹੀਂਤਾਂ ਜੇ( i_ਆਖਰੀ<=128 ){
      ਜੇ( i_ਪਹਿਲਾ<=128 && i_ਆਖਰੀ>=128 ) a[4] = cType;
      ਜੇ( i_ਪਹਿਲਾ<=120 && i_ਆਖਰੀ>=120 ) a[6] = cType;
      ਜੇ( i_ਪਹਿਲਾ<=121 && i_ਆਖਰੀ>=121 ) a[7] = cType;
      ਜੇ( i_ਪਹਿਲਾ<=122 && i_ਆਖਰੀ>=122 ) a[8] = cType;
      ਜੇ( i_ਪਹਿਲਾ<=123 && i_ਆਖਰੀ>=123 ) a[10] = cType;
      ਜੇ( i_ਪਹਿਲਾ<=124 && i_ਆਖਰੀ>=124 ) a[11] = cType;
      ਜੇ( i_ਪਹਿਲਾ<=125 && i_ਆਖਰੀ>=125 ) a[12] = cType;
      ਜੇ( i_ਪਹਿਲਾ<=126 && i_ਆਖਰੀ>=126 ) a[13] = cType;
      ਜੇ( i_ਪਹਿਲਾ<=127 && i_ਆਖਰੀ>=127 ) a[14] = cType;
    }
  }
  ਵਾਪਸ SQLITE_OK;
}
#ਨਹੀਂਤਾਂ
#  define unixPosixAdvisoryLocks(A,B) SQLITE_ERROR_UNABLE
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

/*
** On some systems, calls to fchown() will trigger a message in a security
** log if they come from non-root processes.  So avoid calling fchown() if
** we are not running as root.
*/
ਸਥਿਰ ਪੂਰਨ robustFchown(ਪੂਰਨ fd, uid_t uid, gid_t gid){
#ਜੇ defined(HAVE_FCHOWN)
  ਵਾਪਸ osGeteuid() ? 0 : osFchown(fd,uid,gid);
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "unix" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
ਸਥਿਰ ਪੂਰਨ unix_ਸੈੱਟ_System_Call(
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
ਸਥਿਰ sqlite3_syscall_ptr unixGetSystemCall(
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
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *unix_ਅਗਲਾ_System_Call(sqlite3_vfs *p, ਅਚਲ ਅੱਖਰ *zName){
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

/*
** Do not accept any file descriptor less than this value, in order to avoid
** opening database file using file descriptors that are commonly used for
** standard input, output, and error.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MINIMUM_FILE_DESCRIPTOR
# define SQLITE_MINIMUM_FILE_DESCRIPTOR 3
#ਜੇ_ਅੰਤ

/*
** Invoke open().  Do so multiple times, until it either succeeds or
** fails for some reason other than EINTR.
**
** If the file creation mode "m" is 0 then set it to the default for
** SQLite.  The default is SQLITE_DEFAULT_FILE_PERMISSIONS (normally
** 0644) as modified by the system umask.  If m is not 0, then
** make the file creation mode be exactly m ignoring the umask.
**
** The m parameter will be non-zero only when creating -wal, -journal,
** and -shm files.  We want those files to have *exactly* the same
** permissions as their original database, unadulterated by the umask.
** In that way, if a database file is -rw-rw-rw or -rw-rw-r-, and a
** transaction crashes and leaves behind hot journals, then any
** process that is able to write to the database will also be able to
** recover the hot journals.
*/
ਸਥਿਰ ਪੂਰਨ robust_open(ਅਚਲ ਅੱਖਰ *z, ਪੂਰਨ f, ਬਹੁਲਕ_t m){
  ਪੂਰਨ fd;
  ਬਹੁਲਕ_t m2 = m ? m : SQLITE_DEFAULT_FILE_PERMISSIONS;
  ਜਦੋਂ(1){
#ਜੇ defined(O_CLOEXEC)
    fd = osOpen(z,f|O_CLOEXEC,m2);
#ਨਹੀਂਤਾਂ
    fd = osOpen(z,f,m2);
#ਜੇ_ਅੰਤ
    ਜੇ( fd<0 ){
      ਜੇ( errno==EINTR ) ਜਾਰੀ;
      ਰੋਕੋ;
    }
    ਜੇ( fd>=SQLITE_MINIMUM_FILE_DESCRIPTOR ) ਰੋਕੋ;
    ਜੇ( (f & (O_EXCL|O_CREAT))==(O_EXCL|O_CREAT) ){
      (ਖਾਲੀ)osUnlink(z);
    }
    osClose(fd);
    sqlite3_log(SQLITE_WARNING,
                "attempt to open \"%s\" as file descriptor %d", z, fd);
    fd = -1;
    ਜੇ( osOpen("/dev/null", O_RDONLY, m)<0 ) ਰੋਕੋ;
  }
  ਜੇ( fd>=0 ){
    ਜੇ( m!=0 ){
      ਬਣਤਰ stat statbuf;
      ਜੇ( osFstat(fd, &statbuf)==0
       && statbuf.st_ਮਾਪ==0
       && (statbuf.st_ਬਹੁਲਕ&0777)!=m
      ){
        osFchmod(fd, m);
      }
    }
#ਜੇ defined(FD_CLOEXEC) && (!defined(O_CLOEXEC) || O_CLOEXEC==0)
    osFcntl(fd, F_SETFD, osFcntl(fd, F_GETFD, 0) | FD_CLOEXEC);
#ਜੇ_ਅੰਤ
  }
  ਵਾਪਸ fd;
}

/*
** Helper functions to obtain and relinquish the global mutex. The
** global mutex is used to protect the unixInodeInfo objects used by
** this file, all of which may be shared by multiple threads.
**
** Function unixMutexHeld() is used to assert() that the global mutex
** is held when required. This function is only used as part of assert()
** statements. e.g.
**
**   unixEnterMutex()
**     assert( unixMutexHeld() );
**   unixEnterLeave()
**
** To prevent deadlock, the global unixBigLock must must be acquired
** before the unixInodeInfo.pLockMutex mutex, if both are held.  It is
** OK to get the pLockMutex without holding unixBigLock first, but if
** that happens, the unixBigLock mutex must not be acquired until after
** pLockMutex is released.
**
**      OK:     enter(unixBigLock),  enter(pLockInfo)
**      OK:     enter(unixBigLock)
**      OK:     enter(pLockInfo)
**   ERROR:     enter(pLockInfo), enter(unixBigLock)
*/
ਸਥਿਰ sqlite3_mutex *unix_Big_ਤਾਲਾ = 0;
ਸਥਿਰ ਖਾਲੀ unixEnterMutex(ਖਾਲੀ){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_notheld(unix_Big_ਤਾਲਾ) );  /* Not a recursive mutex */
  sqlite3_mutex_enter(unix_Big_ਤਾਲਾ);
}
ਸਥਿਰ ਖਾਲੀ unixLeaveMutex(ਖਾਲੀ){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(unix_Big_ਤਾਲਾ) );
  sqlite3_mutex_leave(unix_Big_ਤਾਲਾ);
}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਸਥਿਰ ਪੂਰਨ unixMutexHeld(ਖਾਲੀ) {
  ਵਾਪਸ sqlite3_mutex_held(unix_Big_ਤਾਲਾ);
}
#ਜੇ_ਅੰਤ


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_HAVE_OS_TRACE
/*
** Helper function for printing out trace information from debugging
** binaries. This returns the string representation of the supplied
** integer lock-type.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *az_File_ਤਾਲਾ(ਪੂਰਨ e_File_ਤਾਲਾ){
  ਚੋਣ( e_File_ਤਾਲਾ ){
    ਹਾਲਤ NO_ਤਾਲਾ: ਵਾਪਸ "NONE";
    ਹਾਲਤ SHARED_ਤਾਲਾ: ਵਾਪਸ "SHARED";
    ਹਾਲਤ RESERVED_ਤਾਲਾ: ਵਾਪਸ "RESERVED";
    ਹਾਲਤ PENDING_ਤਾਲਾ: ਵਾਪਸ "PENDING";
    ਹਾਲਤ EXCLUSIVE_ਤਾਲਾ: ਵਾਪਸ "EXCLUSIVE";
  }
  ਵਾਪਸ "ERROR";
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ਤਾਲਾ_TRACE
/*
** Print out information about all locking operations.
**
** This routine is used for troubleshooting locks on multithreaded
** platforms.  Enable by compiling with the -DSQLITE_LOCK_TRACE
** command-line option on the compiler.  This code is normally
** turned off.
*/
ਸਥਿਰ ਪੂਰਨ ਤਾਲਾ_Trace(ਪੂਰਨ fd, ਪੂਰਨ op, ਬਣਤਰ flock *p){
  ਅੱਖਰ *zOpName, *zType;
  ਪੂਰਨ s;
  ਪੂਰਨ savedErrno;
  ਜੇ( op==F_GETLK ){
    zOpName = "GETLK";
  }ਨਹੀਂਤਾਂ ਜੇ( op==F_SETLK ){
    zOpName = "SETLK";
  }ਨਹੀਂਤਾਂ{
    s = osFcntl(fd, op, p);
    sqlite3DebugPrintf("fcntl unknown %d %d %d\n", fd, op, s);
    ਵਾਪਸ s;
  }
  ਜੇ( p->l_type==F_RDLCK ){
    zType = "RDLCK";
  }ਨਹੀਂਤਾਂ ਜੇ( p->l_type==F_WRLCK ){
    zType = "WRLCK";
  }ਨਹੀਂਤਾਂ ਜੇ( p->l_type==F_UNLCK ){
    zType = "UNLCK";
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( 0 );
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p->l_whence==SEEK_SET );
  s = osFcntl(fd, op, p);
  savedErrno = errno;
  sqlite3DebugPrintf("fcntl %d %d %s %s %d %d %d %d\n",
     threadid, fd, zOpName, zType, (ਪੂਰਨ)p->l_ਸ਼ੁਰੂ, (ਪੂਰਨ)p->l_len,
     (ਪੂਰਨ)p->l_pid, s);
  ਜੇ( s==(-1) && op==F_SETLK && (p->l_type==F_RDLCK || p->l_type==F_WRLCK) ){
    ਬਣਤਰ flock l2;
    l2 = *p;
    osFcntl(fd, F_GETLK, &l2);
    ਜੇ( l2.l_type==F_RDLCK ){
      zType = "RDLCK";
    }ਨਹੀਂਤਾਂ ਜੇ( l2.l_type==F_WRLCK ){
      zType = "WRLCK";
    }ਨਹੀਂਤਾਂ ਜੇ( l2.l_type==F_UNLCK ){
      zType = "UNLCK";
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( 0 );
    }
    sqlite3DebugPrintf("fcntl-failure-reason: %s %d %d %d\n",
       zType, (ਪੂਰਨ)l2.l_ਸ਼ੁਰੂ, (ਪੂਰਨ)l2.l_len, (ਪੂਰਨ)l2.l_pid);
  }
  errno = savedErrno;
  ਵਾਪਸ s;
}
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ osFcntl
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ osFcntl ਤਾਲਾ_Trace
#ਜੇ_ਅੰਤ /* SQLITE_LOCK_TRACE */

/*
** Retry ftruncate() calls that fail due to EINTR
**
** All calls to ftruncate() within this file should be made through
** this wrapper.  On the Android platform, bypassing the logic below
** could lead to a corrupt database.
*/
ਸਥਿਰ ਪੂਰਨ robust_ftruncate(ਪੂਰਨ h, sqlite3_int64 sz){
  ਪੂਰਨ rc;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __ANDROID__
  /* On Android, ftruncate() always uses 32-bit offsets, even if
  ** _FILE_OFFSET_BITS=64 is defined. This means it is unsafe to attempt to
  ** truncate a file to any size larger than 2GiB. Silently ignore any
  ** such attempts.  */
  ਜੇ( sz>(sqlite3_int64)0x7FFFFFFF ){
    rc = SQLITE_OK;
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
  ਕਰੋ{ rc = osFtruncate(h,sz); }ਜਦੋਂ( rc<0 && errno==EINTR );
  ਵਾਪਸ rc;
}

/*
** This routine translates a standard POSIX errno code into something
** useful to the clients of the sqlite3 functions.  Specifically, it is
** intended to translate a variety of "try again" errors into SQLITE_BUSY
** and a variety of "please close the file descriptor NOW" errors into
** SQLITE_IOERR
**
** Errors during initialization of locks, or file system support for locks,
** should handle ENOLCK, ENOTSUP, EOPNOTSUPP separately.
*/
ਸਥਿਰ ਪੂਰਨ sqliteErrorFromPosixError(ਪੂਰਨ posixError, ਪੂਰਨ sqliteIOErr) {
  ਪ੍ਰਤਿਜ੍ਞਾ( (sqliteIOErr == SQLITE_IOERR_ਤਾਲਾ) ||
          (sqliteIOErr == SQLITE_IOERR_UNLOCK) ||
          (sqliteIOErr == SQLITE_IOERR_RDLOCK) ||
          (sqliteIOErr == SQLITE_IOERR_CHECKRESERVEDLOCK) );
  ਚੋਣ (posixError) {
  ਹਾਲਤ EACCES:
  ਹਾਲਤ EAGAIN:
  ਹਾਲਤ ETIMEDOUT:
  ਹਾਲਤ EBUSY:
  ਹਾਲਤ EINTR:
  ਹਾਲਤ ENOLCK:
    /* random NFS retry error, unless during file system support
     * introspection, in which it actually means what it says */
    ਵਾਪਸ SQLITE_BUSY;
  
  ਹਾਲਤ EPERM:
    ਵਾਪਸ SQLITE_PERM;
  
  ਮੂਲ:
    ਵਾਪਸ sqliteIOErr;
  }
}


/******************************************************************************
****************** Begin Unique File ID Utility Used By VxWorks ***************
**
** On most versions of unix, we can get a unique ID for a file by concatenating
** the device number and the inode number.  But this does not work on VxWorks.
** On VxWorks, a unique file id must be based on the canonical filename.
**
** A pointer to an instance of the following structure can be used as a
** unique file ID in VxWorks.  Each instance of this structure contains
** a copy of the canonical filename.  There is also a reference count.
** The structure is reclaimed when the number of pointers to it drops to
** zero.
**
** There are never very many files open at one time and lookups are not
** a performance-critical path, so it is sufficient to put these
** structures on a linked list.
*/
ਬਣਤਰ vxworksFileId {
  ਬਣਤਰ vxworksFileId *p_ਅਗਲਾ;  /* Next in a list of them all */
  ਪੂਰਨ nRef;                     /* Number of references to this one */
  ਪੂਰਨ nName;                    /* Length of the zCanonicalName[] string */
  ਅੱਖਰ *zCanonicalName;         /* Canonical filename */
};

#ਜੇ OS_VXWORKS
/*
** All unique filenames are held on a linked list headed by this
** variable:
*/
ਸਥਿਰ ਬਣਤਰ vxworksFileId *vxworksFileList = 0;
ਸਥਿਰ sqlite3_mutex *vxworksMutex = 0;

/*
** Simplify a filename into its canonical form
** by making the following changes:
**
**  * removing any trailing and duplicate /
**  * convert /./ into just /
**  * convert /A/../ where A is any simple name into just /
**
** Changes are made in-place.  Return the new name length.
**
** The original filename is in z[0..n-1].  Return the number of
** characters in the simplified name.
*/
ਸਥਿਰ ਪੂਰਨ vxworksSimplifyName(ਅੱਖਰ *z, ਪੂਰਨ n){
  ਪੂਰਨ i, j;
  ਜਦੋਂ( n>1 && z[n-1]=='/' ){ n--; }
  ਲਈ(i=j=0; i<n; i++){
    ਜੇ( z[i]=='/' ){
      ਜੇ( z[i+1]=='/' ) ਜਾਰੀ;
      ਜੇ( z[i+1]=='.' && i+2<n && z[i+2]=='/' ){
        i += 1;
        ਜਾਰੀ;
      }
      ਜੇ( z[i+1]=='.' && i+3<n && z[i+2]=='.' && z[i+3]=='/' ){
        ਜਦੋਂ( j>0 && z[j-1]!='/' ){ j--; }
        ਜੇ( j>0 ){ j--; }
        i += 2;
        ਜਾਰੀ;
      }
    }
    z[j++] = z[i];
  }
  z[j] = 0;
  ਵਾਪਸ j;
}

/*
** Find a unique file ID for the given absolute pathname.  Return
** a pointer to the vxworksFileId object.  This pointer is the unique
** file ID.
**
** The nRef field of the vxworksFileId object is incremented before
** the object is returned.  A new vxworksFileId object is created
** and added to the global list if necessary.
**
** If a memory allocation error occurs, return NULL.
*/
ਸਥਿਰ ਬਣਤਰ vxworksFileId *vxworks_ਲੱਭੋ_File_Id(ਅਚਲ ਅੱਖਰ *zAbsoluteName){
  ਬਣਤਰ vxworksFileId *pNew;         /* search key and new file ID */
  ਬਣਤਰ vxworksFileId *pCandidate;   /* For looping over existing file IDs */
  ਪੂਰਨ n;                              /* Length of zAbsoluteName string */

  ਪ੍ਰਤਿਜ੍ਞਾ( zAbsoluteName[0]=='/' );
  n = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zAbsoluteName);
  pNew = sqlite3_malloc64( ਆਕਾਰ(*pNew) + (n+1) );
  ਜੇ( pNew==0 ) ਵਾਪਸ 0;
  pNew->zCanonicalName = (ਅੱਖਰ*)&pNew[1];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pNew->zCanonicalName, zAbsoluteName, n+1);
  n = vxworksSimplifyName(pNew->zCanonicalName, n);

  /* Search for an existing entry that matching the canonical name.
  ** If found, increment the reference count and return a pointer to
  ** the existing file ID.
  */
  sqlite3_mutex_enter(vxworksMutex);
  ਲਈ(pCandidate=vxworksFileList; pCandidate; pCandidate=pCandidate->p_ਅਗਲਾ){
    ਜੇ( pCandidate->nName==n
     && memcmp(pCandidate->zCanonicalName, pNew->zCanonicalName, n)==0
    ){
       sqlite3_free(pNew);
       pCandidate->nRef++;
       sqlite3_mutex_leave(vxworksMutex);
       ਵਾਪਸ pCandidate;
    }
  }

  /* No match was found.  We will make a new file ID */
  pNew->nRef = 1;
  pNew->nName = n;
  pNew->p_ਅਗਲਾ = vxworksFileList;
  vxworksFileList = pNew;
  sqlite3_mutex_leave(vxworksMutex);
  ਵਾਪਸ pNew;
}

/*
** Decrement the reference count on a vxworksFileId object.  Free
** the object when the reference count reaches zero.
*/
ਸਥਿਰ ਖਾਲੀ vxworksReleaseFileId(ਬਣਤਰ vxworksFileId *pId){
  sqlite3_mutex_enter(vxworksMutex);
  ਪ੍ਰਤਿਜ੍ਞਾ( pId->nRef>0 );
  pId->nRef--;
  ਜੇ( pId->nRef==0 ){
    ਬਣਤਰ vxworksFileId **pp;
    ਲਈ(pp=&vxworksFileList; *pp && *pp!=pId; pp = &((*pp)->p_ਅਗਲਾ)){}
    ਪ੍ਰਤਿਜ੍ਞਾ( *pp==pId );
    *pp = pId->p_ਅਗਲਾ;
    sqlite3_free(pId);
  }
  sqlite3_mutex_leave(vxworksMutex);
}
#ਜੇ_ਅੰਤ /* OS_VXWORKS */
/*************** End of Unique File ID Utility Used By VxWorks ****************
******************************************************************************/


/******************************************************************************
*************************** Posix Advisory Locking ****************************
**
** POSIX advisory locks are broken by design.  ANSI STD 1003.1 (1996)
** section 6.5.2.2 lines 483 through 490 specify that when a process
** sets or clears a lock, that operation overrides any prior locks set
** by the same process.  It does not explicitly say so, but this implies
** that it overrides locks set by the same process using a different
** file descriptor.  Consider this test case:
**
**       int fd1 = open("./file1", O_RDWR|O_CREAT, 0644);
**       int fd2 = open("./file2", O_RDWR|O_CREAT, 0644);
**
** Suppose ./file1 and ./file2 are really the same file (because
** one is a hard or symbolic link to the other) then if you set
** an exclusive lock on fd1, then try to get an exclusive lock
** on fd2, it works.  I would have expected the second lock to
** fail since there was already a lock on the file due to fd1.
** But not so.  Since both locks came from the same process, the
** second overrides the first, even though they were on different
** file descriptors opened on different file names.
**
** This means that we cannot use POSIX locks to synchronize file access
** among competing threads of the same process.  POSIX locks will work fine
** to synchronize access for threads in separate processes, but not
** threads within the same process.
**
** To work around the problem, SQLite has to manage file locks internally
** on its own.  Whenever a new database is opened, we have to find the
** specific inode of the database file (the inode is determined by the
** st_dev and st_ino fields of the stat structure that fstat() fills in)
** and check for locks already existing on that inode.  When locks are
** created or removed, we have to look at our own internal record of the
** locks to see if another thread has previously set a lock on that same
** inode.
**
** (Aside: The use of inode numbers as unique IDs does not work on VxWorks.
** For VxWorks, we have to use the alternative unique ID system based on
** canonical filename and implemented in the previous division.)
**
** The sqlite3_file structure for POSIX is no longer just an integer file
** descriptor.  It is now a structure that holds the integer file
** descriptor and a pointer to a structure that describes the internal
** locks on the corresponding inode.  There is one locking structure
** per inode, so if the same inode is opened twice, both unixFile structures
** point to the same locking structure.  The locking structure keeps
** a reference count (so we will know when to delete it) and a "cnt"
** field that tells us its internal lock status.  cnt==0 means the
** file is unlocked.  cnt==-1 means the file has an exclusive lock.
** cnt>0 means there are cnt shared locks on the file.
**
** Any attempt to lock or unlock a file first checks the locking
** structure.  The fcntl() system call is only invoked to set a
** POSIX lock if the internal lock structure transitions between
** a locked and an unlocked state.
**
** But wait:  there are yet more problems with POSIX advisory locks.
**
** If you close a file descriptor that points to a file that has locks,
** all locks on that file that are owned by the current process are
** released.  To work around this problem, each unixInodeInfo object
** maintains a count of the number of pending locks on the inode.
** When an attempt is made to close an unixFile, if there are
** other unixFile open on the same inode that are holding locks, the call
** to close() the file descriptor is deferred until all of the locks clear.
** The unixInodeInfo structure keeps a list of file descriptors that need to
** be closed and that list is walked (and cleared) when the last lock
** clears.
**
** Yet another problem:  LinuxThreads do not play well with posix locks.
**
** Many older versions of linux use the LinuxThreads library which is
** not posix compliant.  Under LinuxThreads, a lock created by thread
** A cannot be modified or overridden by a different thread B.
** Only thread A can modify the lock.  Locking behavior is correct
** if the application uses the newer Native Posix Thread Library (NPTL)
** on linux - with NPTL a lock created by thread A can override locks
** in thread B.  But there is no way to know at compile-time which
** threading library is being used.  So there is no way to know at
** compile-time whether or not thread A can override locks on thread B.
** One has to do a run-time check to discover the behavior of the
** current process.
**
** SQLite used to support LinuxThreads.  But support for LinuxThreads
** was dropped beginning with version 3.7.0.  SQLite will still work with
** LinuxThreads provided that (1) there is no more than one connection
** per database file in the same process and (2) database connections
** do not move across threads.
*/

/*
** An instance of the following structure serves as the key used
** to locate a particular unixInodeInfo object.
*/
ਬਣਤਰ unixFileId {
  dev_t dev;                  /* Device number */
#ਜੇ OS_VXWORKS
  ਬਣਤਰ vxworksFileId *pId;  /* Unique file ID for vxworks. */
#ਨਹੀਂਤਾਂ
  /* We are told that some versions of Android contain a bug that
  ** sizes ino_t at only 32-bits instead of 64-bits. (See
  ** https://android-review.googlesource.com/#/c/115351/3/dist/sqlite3.c)
  ** To work around this, always allocate 64-bits for the inode number.
  ** On small machines that only have 32-bit inodes, this wastes 4 bytes,
  ** but that should not be a big deal. */
  /* WAS:  ino_t ino;   */
  u64 ino;                   /* Inode number */
#ਜੇ_ਅੰਤ
};

/*
** An instance of the following structure is allocated for each open
** inode.
**
** A single inode can have multiple file descriptors, so each unixFile
** structure contains a pointer to an instance of this object and this
** object keeps a count of the number of unixFile pointing to it.
**
** Mutex rules:
**
**  (1) Only the pLockMutex mutex must be held in order to read or write
**      any of the locking fields:
**          nShared, nLock, eFileLock, bProcessLock, pUnused
**
**  (2) When nRef>0, then the following fields are unchanging and can
**      be read (but not written) without holding any mutex:
**          fileId, pLockMutex
**
**  (3) With the exceptions above, all the fields may only be read
**      or written while holding the global unixBigLock mutex.
**
** Deadlock prevention:  The global unixBigLock mutex may not
** be acquired while holding the pLockMutex mutex.  If both unixBigLock
** and pLockMutex are needed, then unixBigLock must be acquired first.
*/
ਬਣਤਰ unixInodeInfo {
  ਬਣਤਰ unixFileId fileId;       /* The lookup key */
  sqlite3_mutex *p_ਤਾਲਾ_Mutex;      /* Hold this mutex for... */
  ਪੂਰਨ nShared;                      /* Number of SHARED locks held */
  ਪੂਰਨ n_ਤਾਲਾ;                        /* Number of outstanding file locks */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ e_File_ਤਾਲਾ;          /* One of SHARED_LOCK, RESERVED_LOCK etc. */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ b_Process_ਤਾਲਾ;       /* An exclusive process lock is held */
  Unix_ਅਣਵਰਤਿਆ_Fd *p_ਅਣਵਰਤਿਆ;            /* Unused file descriptors to close */
  ਪੂਰਨ nRef;                       /* Number of pointers to this structure */
  unix_Shm_ਗੰਢ *p_Shm_ਗੰਢ;          /* Shared memory associated with this inode */
  unixInodeInfo *p_ਅਗਲਾ;           /* List of all unixInodeInfo objects */
  unixInodeInfo *pPrev;           /*    .... doubly linked */
#ਜੇ SQLITE_ENABLE_LOCKING_STYLE
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ sharedByte;  /* for AFP simulated shared lock */
#ਜੇ_ਅੰਤ
#ਜੇ OS_VXWORKS
  sem_t *pSem;                    /* Named POSIX semaphore */
  ਅੱਖਰ aSemName[MAX_PATHNAME+2];  /* Name of that semaphore */
#ਜੇ_ਅੰਤ
};

/*
** A lists of all unixInodeInfo objects.
**
** Must hold unixBigLock in order to read or write this variable.
*/
ਸਥਿਰ unixInodeInfo *inodeList = 0;  /* All unixInodeInfo objects */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** True if the inode mutex (on the unixFile.pFileMutex field) is held, or not.
** This routine is used only within assert() to help verify correct mutex
** usage.
*/
ਪੂਰਨ unixFileMutexHeld(unixFile *pFile){
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->pInode );
  ਵਾਪਸ sqlite3_mutex_held(pFile->pInode->p_ਤਾਲਾ_Mutex);
}
ਪੂਰਨ unixFileMutexNotheld(unixFile *pFile){
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->pInode );
  ਵਾਪਸ sqlite3_mutex_notheld(pFile->pInode->p_ਤਾਲਾ_Mutex);
}
#ਜੇ_ਅੰਤ

/*
**
** This function - unixLogErrorAtLine(), is only ever called via the macro
** unixLogError().
**
** It is invoked after an error occurs in an OS function and errno has been
** set. It logs a message using sqlite3_log() containing the current value of
** errno and, if possible, the human-readable equivalent from strerror() or
** strerror_r().
**
** The first argument passed to the macro should be the error code that
** will be returned to SQLite (e.g. SQLITE_IOERR_DELETE, SQLITE_CANTOPEN).
** The two subsequent arguments should be the name of the OS function that
** failed (e.g. "unlink", "open") and the associated file-system path,
** if any.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unixLogError(a,b,c)     unix_Log_Error_At_ਸਤਰ(a,b,c,__LINE__)
ਸਥਿਰ ਪੂਰਨ unix_Log_Error_At_ਸਤਰ(
  ਪੂਰਨ errcode,                    /* SQLite error code */
  ਅਚਲ ਅੱਖਰ *zFunc,              /* Name of OS function that failed */
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,              /* File path associated with error */
  ਪੂਰਨ i_ਸਤਰ                       /* Source line number where error occurred */
){
  ਅੱਖਰ *zErr;                     /* Message from strerror() or equivalent */
  ਪੂਰਨ iErrno = errno;             /* Saved syscall error number */

  /* If this is not a threadsafe build (SQLITE_THREADSAFE==0), then use
  ** the strerror() function to obtain the human-readable error message
  ** equivalent to errno. Otherwise, use strerror_r().
  */
#ਜੇ SQLITE_THREADSAFE && defined(HAVE_STRERROR_R)
  ਅੱਖਰ aErr[80];
  memset(aErr, 0, ਆਕਾਰ(aErr));
  zErr = aErr;

  /* If STRERROR_R_CHAR_P (set by autoconf scripts) or __USE_GNU is defined,
  ** assume that the system provides the GNU version of strerror_r() that
  ** returns a pointer to a buffer containing the error message. That pointer
  ** may point to aErr[], or it may point to some static storage somewhere.
  ** Otherwise, assume that the system provides the POSIX version of
  ** strerror_r(), which always writes an error message into aErr[].
  **
  ** If the code incorrectly assumes that it is the POSIX version that is
  ** available, the error message will often be an empty string. Not a
  ** huge problem. Incorrectly concluding that the GNU version is available
  ** could lead to a segfault though.
  **
  ** Forum post 3f13857fa4062301 reports that the Android SDK may use
  ** int-type return, depending on its version.
  */
#ਜੇ (defined(STRERROR_R_CHAR_P) || defined(__USE_GNU)) \
  && !defined(ANDROID) && !defined(__ANDROID__)
  zErr =
# endif
  strerror_r(iErrno, aErr, ਆਕਾਰ(aErr)-1);

#ਜਾਂਜੇ SQLITE_THREADSAFE
  /* This is a threadsafe build, but strerror_r() is not available. */
  zErr = "";
#ਨਹੀਂਤਾਂ
  /* Non-threadsafe build, use strerror(). */
  zErr = strerror(iErrno);
#ਜੇ_ਅੰਤ

  ਜੇ( z_ਰਸਤਾ==0 ) z_ਰਸਤਾ = "";
  sqlite3_log(errcode,
      "os_unix.c:%d: (%d) %s(%s) - %s",
      i_ਸਤਰ, iErrno, zFunc, z_ਰਸਤਾ, zErr
  );

  ਵਾਪਸ errcode;
}

/*
** Close a file descriptor.
**
** We assume that close() almost always works, since it is only in a
** very sick application or on a very sick platform that it might fail.
** If it does fail, simply leak the file descriptor, but do log the
** error.
**
** Note that it is not safe to retry close() after EINTR since the
** file descriptor might have already been reused by another thread.
** So we don't even try to recover from an EINTR.  Just log the error
** and move on.
*/
ਸਥਿਰ ਖਾਲੀ robust_close(unixFile *pFile, ਪੂਰਨ h, ਪੂਰਨ lineno){
  ਜੇ( osClose(h) ){
    unix_Log_Error_At_ਸਤਰ(SQLITE_IOERR_CLOSE, "close",
                       pFile ? pFile->z_ਰਸਤਾ : 0, lineno);
  }
}

/*
** Set the pFile->lastErrno.  Do this in a subroutine as that provides
** a convenient place to set a breakpoint.
*/
ਸਥਿਰ ਖਾਲੀ store_ਆਖਰੀ_Errno(unixFile *pFile, ਪੂਰਨ error){
  pFile->ਆਖਰੀ_Errno = error;
}

/*
** Close all file descriptors accumulated in the unixInodeInfo->pUnused list.
*/
ਸਥਿਰ ਖਾਲੀ closePendingFds(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  Unix_ਅਣਵਰਤਿਆ_Fd *p;
  Unix_ਅਣਵਰਤਿਆ_Fd *p_ਅਗਲਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( unixFileMutexHeld(pFile) );
  ਲਈ(p=pInode->p_ਅਣਵਰਤਿਆ; p; p=p_ਅਗਲਾ){
    p_ਅਗਲਾ = p->p_ਅਗਲਾ;
    robust_close(pFile, p->fd, __LINE__);
    sqlite3_free(p);
  }
  pInode->p_ਅਣਵਰਤਿਆ = 0;
}

/*
** Release a unixInodeInfo structure previously allocated by findInodeInfo().
**
** The global mutex must be held when this routine is called, but the mutex
** on the inode being deleted must NOT be held.
*/
ਸਥਿਰ ਖਾਲੀ releaseInodeInfo(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  ਪ੍ਰਤਿਜ੍ਞਾ( unixMutexHeld() );
  ਪ੍ਰਤਿਜ੍ਞਾ( unixFileMutexNotheld(pFile) );
  ਜੇ( ALWAYS(pInode) ){
    pInode->nRef--;
    ਜੇ( pInode->nRef==0 ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pInode->p_Shm_ਗੰਢ==0 );
      sqlite3_mutex_enter(pInode->p_ਤਾਲਾ_Mutex);
      closePendingFds(pFile);
      sqlite3_mutex_leave(pInode->p_ਤਾਲਾ_Mutex);
      ਜੇ( pInode->pPrev ){
        ਪ੍ਰਤਿਜ੍ਞਾ( pInode->pPrev->p_ਅਗਲਾ==pInode );
        pInode->pPrev->p_ਅਗਲਾ = pInode->p_ਅਗਲਾ;
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( inodeList==pInode );
        inodeList = pInode->p_ਅਗਲਾ;
      }
      ਜੇ( pInode->p_ਅਗਲਾ ){
        ਪ੍ਰਤਿਜ੍ਞਾ( pInode->p_ਅਗਲਾ->pPrev==pInode );
        pInode->p_ਅਗਲਾ->pPrev = pInode->pPrev;
      }
      sqlite3_mutex_free(pInode->p_ਤਾਲਾ_Mutex);
      sqlite3_free(pInode);
    }
  }
}

/*
** Given a file descriptor, locate the unixInodeInfo object that
** describes that file descriptor.  Create a new one if necessary.  The
** return value might be uninitialized if an error occurs.
**
** The global mutex must held when calling this routine.
**
** Return an appropriate error code.
*/
ਸਥਿਰ ਪੂਰਨ ਲੱਭੋ_Inode_Info(
  unixFile *pFile,               /* Unix file with file desc used in the key */
  unixInodeInfo **ppInode        /* Return the unixInodeInfo object here */
){
  ਪੂਰਨ rc;                        /* System call return code */
  ਪੂਰਨ fd;                        /* The file descriptor for pFile */
  ਬਣਤਰ unixFileId fileId;      /* Lookup key for the unixInodeInfo */
  ਬਣਤਰ stat statbuf;           /* Low-level file information */
  unixInodeInfo *pInode = 0;     /* Candidate unixInodeInfo object */

  ਪ੍ਰਤਿਜ੍ਞਾ( unixMutexHeld() );

  /* Get low-level information about the file that we can used to
  ** create a unique name for the file.
  */
  fd = pFile->h;
  rc = osFstat(fd, &statbuf);
  ਜੇ( rc!=0 ){
    store_ਆਖਰੀ_Errno(pFile, errno);
#ਜੇ defined(EOVERFLOW) && defined(SQLITE_DISABLE_LFS)
    ਜੇ( pFile->ਆਖਰੀ_Errno==EOVERFLOW ) ਵਾਪਸ SQLITE_NOLFS;
#ਜੇ_ਅੰਤ
    ਵਾਪਸ SQLITE_IOERR;
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __APPLE__
  /* On OS X on an msdos filesystem, the inode number is reported
  ** incorrectly for zero-size files.  See ticket #3260.  To work
  ** around this problem (we consider it a bug in OS X, not SQLite)
  ** we always increase the file size to 1 by writing a single byte
  ** prior to accessing the inode number.  The one byte written is
  ** an ASCII 'S' character which also happens to be the first byte
  ** in the header of every SQLite database.  In this way, if there
  ** is a race condition such that another thread has already populated
  ** the first page of the database, no damage is done.
  */
  ਜੇ( statbuf.st_ਮਾਪ==0 && (pFile->fsFlags & SQLITE_FSFLAGS_IS_MSDOS)!=0 ){
    ਕਰੋ{ rc = osWrite(fd, "S", 1); }ਜਦੋਂ( rc<0 && errno==EINTR );
    ਜੇ( rc!=1 ){
      store_ਆਖਰੀ_Errno(pFile, errno);
      ਵਾਪਸ SQLITE_IOERR;
    }
    ਜੇ( fsync(fd) ){
      store_ਆਖਰੀ_Errno(pFile, errno);
      ਵਾਪਸ SQLITE_IOERR_FSYNC;
    }
    rc = osFstat(fd, &statbuf);
    ਜੇ( rc!=0 ){
      store_ਆਖਰੀ_Errno(pFile, errno);
      ਵਾਪਸ SQLITE_IOERR;
    }
  }
#ਜੇ_ਅੰਤ

  memset(&fileId, 0, ਆਕਾਰ(fileId));
  fileId.dev = statbuf.st_dev;
#ਜੇ OS_VXWORKS
  fileId.pId = pFile->pId;
#ਨਹੀਂਤਾਂ
  fileId.ino = (u64)statbuf.st_ino;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( unixMutexHeld() );
  pInode = inodeList;
  ਜਦੋਂ( pInode && memcmp(&fileId, &pInode->fileId, ਆਕਾਰ(fileId)) ){
    pInode = pInode->p_ਅਗਲਾ;
  }
  ਜੇ( pInode==0 ){
    pInode = sqlite3_malloc64( ਆਕਾਰ(*pInode) );
    ਜੇ( pInode==0 ){
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
    memset(pInode, 0, ਆਕਾਰ(*pInode));
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pInode->fileId, &fileId, ਆਕਾਰ(fileId));
    ਜੇ( sqlite3GlobalConfig.bCoreMutex ){
      pInode->p_ਤਾਲਾ_Mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      ਜੇ( pInode->p_ਤਾਲਾ_Mutex==0 ){
        sqlite3_free(pInode);
        ਵਾਪਸ SQLITE_NOMEM_BKPT;
      }
    }
    pInode->nRef = 1;
    ਪ੍ਰਤਿਜ੍ਞਾ( unixMutexHeld() );
    pInode->p_ਅਗਲਾ = inodeList;
    pInode->pPrev = 0;
    ਜੇ( inodeList ) inodeList->pPrev = pInode;
    inodeList = pInode;
  }ਨਹੀਂਤਾਂ{
    pInode->nRef++;
  }
  *ppInode = pInode;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return TRUE if pFile has been renamed or unlinked since it was first opened.
*/
ਸਥਿਰ ਪੂਰਨ fileHasMoved(unixFile *pFile){
#ਜੇ OS_VXWORKS
  ਵਾਪਸ pFile->pInode!=0 && pFile->pId!=pFile->pInode->fileId.pId;
#ਨਹੀਂਤਾਂ
  ਬਣਤਰ stat buf;
  ਵਾਪਸ pFile->pInode!=0 &&
      (osStat(pFile->z_ਰਸਤਾ, &buf)!=0
         || (u64)buf.st_ino!=pFile->pInode->fileId.ino);
#ਜੇ_ਅੰਤ
}


/*
** Check a unixFile that is a database.  Verify the following:
**
** (1) There is exactly one hard link on the file
** (2) The file is not a symbolic link
** (3) The file has not been renamed or unlinked
**
** Issue sqlite3_log(SQLITE_WARNING,...) messages if anything is not right.
*/
ਸਥਿਰ ਖਾਲੀ verifyDbFile(unixFile *pFile){
  ਬਣਤਰ stat buf;
  ਪੂਰਨ rc;

  /* These verifications occurs for the main database only */
  ਜੇ( pFile->ctrlFlags & UNIXFILE_NOLOCK ) ਵਾਪਸ;

  rc = osFstat(pFile->h, &buf);
  ਜੇ( rc!=0 ){
    sqlite3_log(SQLITE_WARNING, "cannot fstat db file %s", pFile->z_ਰਸਤਾ);
    ਵਾਪਸ;
  }
  ਜੇ( buf.st_nlink==0 ){
    sqlite3_log(SQLITE_WARNING, "file unlinked while open: %s", pFile->z_ਰਸਤਾ);
    ਵਾਪਸ;
  }
  ਜੇ( buf.st_nlink>1 ){
    sqlite3_log(SQLITE_WARNING, "multiple links to file: %s", pFile->z_ਰਸਤਾ);
    ਵਾਪਸ;
  }
  ਜੇ( fileHasMoved(pFile) ){
    sqlite3_log(SQLITE_WARNING, "file renamed while open: %s", pFile->z_ਰਸਤਾ);
    ਵਾਪਸ;
  }
}


/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
ਸਥਿਰ ਪੂਰਨ unix_Check_Reserved_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ reserved = 0;
  unixFile *pFile = (unixFile*)id;

  SimulateIOError( ਵਾਪਸ SQLITE_IOERR_CHECKRESERVEDLOCK; );

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->e_File_ਤਾਲਾ<=SHARED_ਤਾਲਾ );
  sqlite3_mutex_enter(pFile->pInode->p_ਤਾਲਾ_Mutex);

  /* Check if a thread in this process holds such a lock */
  ਜੇ( pFile->pInode->e_File_ਤਾਲਾ>SHARED_ਤਾਲਾ ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it.
  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __DJGPP__
  ਜੇ( !reserved && !pFile->pInode->b_Process_ਤਾਲਾ ){
    ਬਣਤਰ flock ਤਾਲਾ;
    ਤਾਲਾ.l_whence = SEEK_SET;
    ਤਾਲਾ.l_ਸ਼ੁਰੂ = RESERVED_BYTE;
    ਤਾਲਾ.l_len = 1;
    ਤਾਲਾ.l_type = F_WRLCK;
    ਜੇ( osFcntl(pFile->h, F_GETLK, &ਤਾਲਾ) ){
      rc = SQLITE_IOERR_CHECKRESERVEDLOCK;
      store_ਆਖਰੀ_Errno(pFile, errno);
    } ਨਹੀਂਤਾਂ ਜੇ( ਤਾਲਾ.l_type!=F_UNLCK ){
      reserved = 1;
    }
  }
#ਜੇ_ਅੰਤ

  sqlite3_mutex_leave(pFile->pInode->p_ਤਾਲਾ_Mutex);
  OSTRACE(("TEST WR-LOCK %d %d %d (unix)\n", pFile->h, rc, reserved));

  *p_Res_ਬਾਹਰਲਾ = reserved;
  ਵਾਪਸ rc;
}

/* Forward declaration*/
ਸਥਿਰ ਪੂਰਨ unixSleep(sqlite3_vfs*,ਪੂਰਨ);

/*
** Set a posix-advisory-lock.
**
** There are two versions of this routine.  If compiled with
** SQLITE_ENABLE_SETLK_TIMEOUT then the routine has an extra parameter
** which is a pointer to a unixFile.  If the unixFile->iBusyTimeout
** value is set, then it is the number of milliseconds to wait before
** failing the lock.  The iBusyTimeout value is always reset back to
** zero on each call.
**
** If SQLITE_ENABLE_SETLK_TIMEOUT is not defined, then do a non-blocking
** attempt to set the lock.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
# define os_ਸੈੱਟ_Posix_Advisory_ਤਾਲਾ(h,x,t) osFcntl(h,F_SETLK,x)
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਪੂਰਨ os_ਸੈੱਟ_Posix_Advisory_ਤਾਲਾ(
  ਪੂਰਨ h,                /* The file descriptor on which to take the lock */
  ਬਣਤਰ flock *p_ਤਾਲਾ,  /* The description of the lock */
  unixFile *pFile       /* Structure holding timeout value */
){
  ਪੂਰਨ rc = 0;

  ਜੇ( pFile->iBusyTimeout==0 ){
    /* unixFile->iBusyTimeout is set to 0. In this case, attempt a 
    ** non-blocking lock. */
    rc = osFcntl(h,F_SETLK,p_ਤਾਲਾ);
  }ਨਹੀਂਤਾਂ{
    /* unixFile->iBusyTimeout is set to greater than zero. In this case,
    ** attempt a blocking-lock with a unixFile->iBusyTimeout ms timeout.
    **
    ** On systems that support some kind of blocking file lock operation,
    ** this block should be replaced by code to attempt a blocking lock
    ** with a timeout of unixFile->iBusyTimeout ms. The code below is 
    ** placeholder code. If SQLITE_TEST is defined, the placeholder code
    ** retries the lock once every 1ms until it succeeds or the timeout
    ** is reached. Or, if SQLITE_TEST is not defined, the placeholder
    ** code attempts a non-blocking lock and sets unixFile->iBusyTimeout
    ** to 0. This causes the caller to return SQLITE_BUSY, instead of
    ** SQLITE_BUSY_TIMEOUT to SQLite - as required by a VFS that does not 
    ** support blocking locks.
    */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
    ਪੂਰਨ tm = pFile->iBusyTimeout;
    ਜਦੋਂ( tm>0 ){
      rc = osFcntl(h,F_SETLK,p_ਤਾਲਾ);
      ਜੇ( rc==0 ) ਰੋਕੋ;
      unixSleep(0,1000);
      tm--;
    }
#ਨਹੀਂਤਾਂ
    rc = osFcntl(h,F_SETLK,p_ਤਾਲਾ);
    pFile->iBusyTimeout = 0;
#ਜੇ_ਅੰਤ
    /* End of code to replace with real blocking-locks code. */
  }

  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_SETLK_TIMEOUT */


/*
** Attempt to set a system-lock on the file pFile.  The lock is
** described by pLock.
**
** If the pFile was opened read/write from unix-excl, then the only lock
** ever obtained is an exclusive lock, and it is obtained exactly once
** the first time any lock is attempted.  All subsequent system locking
** operations become no-ops.  Locking operations still happen internally,
** in order to coordinate access between separate database connections
** within this process, but all of that is handled in memory and the
** operating system does not participate.
**
** This function is a pass-through to fcntl(F_SETLK) if pFile is using
** any VFS other than "unix-excl" or if pFile is opened on "unix-excl"
** and is read-only.
**
** Zero is returned if the call completes successfully, or -1 if a call
** to fcntl() fails. In this case, errno is set appropriately (by fcntl()).
*/
ਸਥਿਰ ਪੂਰਨ unix_File_ਤਾਲਾ(unixFile *pFile, ਬਣਤਰ flock *p_ਤਾਲਾ){
  ਪੂਰਨ rc;
  unixInodeInfo *pInode = pFile->pInode;
  ਪ੍ਰਤਿਜ੍ਞਾ( pInode!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pInode->p_ਤਾਲਾ_Mutex) );
  ਜੇ( (pFile->ctrlFlags & (UNIXFILE_EXCL|UNIXFILE_RDONLY))==UNIXFILE_EXCL ){
    ਜੇ( pInode->b_Process_ਤਾਲਾ==0 ){
      ਬਣਤਰ flock ਤਾਲਾ;
      /* assert( pInode->nLock==0 ); <-- Not true if unix-excl READONLY used */
      ਤਾਲਾ.l_whence = SEEK_SET;
      ਤਾਲਾ.l_ਸ਼ੁਰੂ = SHARED_ਪਹਿਲਾ;
      ਤਾਲਾ.l_len = SHARED_ਮਾਪ;
      ਤਾਲਾ.l_type = F_WRLCK;
      rc = os_ਸੈੱਟ_Posix_Advisory_ਤਾਲਾ(pFile->h, &ਤਾਲਾ, pFile);
      ਜੇ( rc<0 ) ਵਾਪਸ rc;
      pInode->b_Process_ਤਾਲਾ = 1;
      pInode->n_ਤਾਲਾ++;
    }ਨਹੀਂਤਾਂ{
      rc = 0;
    }
  }ਨਹੀਂਤਾਂ{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
    ਜੇ( pFile->b_ਖੰਡ_On_Connect && p_ਤਾਲਾ->l_type==F_RDLCK
     && p_ਤਾਲਾ->l_ਸ਼ੁਰੂ==SHARED_ਪਹਿਲਾ && p_ਤਾਲਾ->l_len==SHARED_ਮਾਪ
    ){
      rc = osFcntl(pFile->h, F_SETLKW, p_ਤਾਲਾ);
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    rc = os_ਸੈੱਟ_Posix_Advisory_ਤਾਲਾ(pFile->h, p_ਤਾਲਾ, pFile);
  }
  ਵਾਪਸ rc;
}

#ਜੇ !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
/* Forward reference */
ਸਥਿਰ ਪੂਰਨ unix_Is_Sharing_Shm_ਗੰਢ(unixFile*);
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unix_Is_Sharing_Shm_ਗੰਢ(pFile) (0)
#ਜੇ_ਅੰਤ

/*
** Lock the file with the lock specified by parameter eFileLock - one
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
**    SHARED -> EXCLUSIVE
**    RESERVED -> (PENDING) -> EXCLUSIVE
**    PENDING -> EXCLUSIVE
**
** This routine will only increase a lock.  Use the sqlite3OsUnlock()
** routine to lower a locking level.
*/
ਸਥਿਰ ਪੂਰਨ unix_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ e_File_ਤਾਲਾ){
  /* The following describes the implementation of the various locks and
  ** lock transitions in terms of the POSIX advisory shared and exclusive
  ** lock primitives (called read-locks and write-locks below, to avoid
  ** confusion with SQLite lock names). The algorithms are complicated
  ** slightly in order to be compatible with Windows95 systems simultaneously
  ** accessing the same database file, in case that is ever required.
  **
  ** Symbols defined in os.h identify the 'pending byte' and the 'reserved
  ** byte', each single bytes at well known offsets, and the 'shared byte
  ** range', a range of 510 bytes at a well known offset.
  **
  ** To obtain a SHARED lock, a read-lock is obtained on the 'pending
  ** byte'.  If this is successful, 'shared byte range' is read-locked
  ** and the lock on the 'pending byte' released.  (Legacy note:  When
  ** SQLite was first developed, Windows95 systems were still very common,
  ** and Windows95 lacks a shared-lock capability.  So on Windows95, a
  ** single randomly selected by from the 'shared byte range' is locked.
  ** Windows95 is now pretty much extinct, but this work-around for the
  ** lack of shared-locks on Windows95 lives on, for backwards
  ** compatibility.)
  **
  ** A process may only obtain a RESERVED lock after it has a SHARED lock.
  ** A RESERVED lock is implemented by grabbing a write-lock on the
  ** 'reserved byte'.
  **
  ** An EXCLUSIVE lock may only be requested after either a SHARED or
  ** RESERVED lock is held. An EXCLUSIVE lock is implemented by obtaining
  ** a write-lock on the entire 'shared byte range'. Since all other locks
  ** require a read-lock on one of the bytes within this range, this ensures
  ** that no other locks are held on the database.
  **
  ** If a process that holds a RESERVED lock requests an EXCLUSIVE, then
  ** a PENDING lock is obtained first. A PENDING lock is implemented by
  ** obtaining a write-lock on the 'pending byte'. This ensures that no new
  ** SHARED locks can be obtained, but existing SHARED locks are allowed to
  ** persist. If the call to this function fails to obtain the EXCLUSIVE
  ** lock in this case, it holds the PENDING lock instead. The client may
  ** then re-attempt the EXCLUSIVE lock later on, after existing SHARED
  ** locks have cleared.
  */
  ਪੂਰਨ rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  ਬਣਤਰ flock ਤਾਲਾ;
  ਪੂਰਨ tErrno = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  OSTRACE(("LOCK    %d %s was %s(%s,%d) pid=%d (unix)\n", pFile->h,
      az_File_ਤਾਲਾ(e_File_ਤਾਲਾ), az_File_ਤਾਲਾ(pFile->e_File_ਤਾਲਾ),
      az_File_ਤਾਲਾ(pFile->pInode->e_File_ਤਾਲਾ), pFile->pInode->nShared,
      osGetpid(0)));

  /* If there is already a lock of this type or more restrictive on the
  ** unixFile, do nothing. Don't use the end_lock: exit path, as
  ** unixEnterMutex() hasn't been called yet.
  */
  ਜੇ( pFile->e_File_ਤਾਲਾ>=e_File_ਤਾਲਾ ){
    OSTRACE(("LOCK    %d %s ok (already held) (unix)\n", pFile->h,
            az_File_ਤਾਲਾ(e_File_ਤਾਲਾ)));
    ਵਾਪਸ SQLITE_OK;
  }

  /* Make sure the locking sequence is correct.
  **  (1) We never move from unlocked to anything higher than shared lock.
  **  (2) SQLite never explicitly requests a pending lock.
  **  (3) A shared lock is always held when a reserve lock is requested.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->e_File_ਤਾਲਾ!=NO_ਤਾਲਾ || e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( e_File_ਤਾਲਾ!=PENDING_ਤਾਲਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( e_File_ਤਾਲਾ!=RESERVED_ਤਾਲਾ || pFile->e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ );

  /* This mutex is needed because pFile->pInode is shared across threads
  */
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->p_ਤਾਲਾ_Mutex);

  /* If some thread using this PID has a lock via a different unixFile*
  ** handle that precludes the requested lock, return BUSY.
  */
  ਜੇ( (pFile->e_File_ਤਾਲਾ!=pInode->e_File_ਤਾਲਾ &&
          (pInode->e_File_ਤਾਲਾ>=PENDING_ਤਾਲਾ || e_File_ਤਾਲਾ>SHARED_ਤਾਲਾ))
  ){
    rc = SQLITE_BUSY;
    ਜਾਓ ਸਿਰਾ_ਤਾਲਾ;
  }

  /* If a SHARED lock is requested, and some thread using this PID already
  ** has a SHARED or RESERVED lock, then increment reference counts and
  ** return SQLITE_OK.
  */
  ਜੇ( e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ &&
      (pInode->e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ || pInode->e_File_ਤਾਲਾ==RESERVED_ਤਾਲਾ) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ );
    ਪ੍ਰਤਿਜ੍ਞਾ( pFile->e_File_ਤਾਲਾ==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pInode->nShared>0 );
    pFile->e_File_ਤਾਲਾ = SHARED_ਤਾਲਾ;
    pInode->nShared++;
    pInode->n_ਤਾਲਾ++;
    ਜਾਓ ਸਿਰਾ_ਤਾਲਾ;
  }


  /* A PENDING lock is needed before acquiring a SHARED lock and before
  ** acquiring an EXCLUSIVE lock.  For the SHARED lock, the PENDING will
  ** be released.
  */
  ਤਾਲਾ.l_len = 1L;
  ਤਾਲਾ.l_whence = SEEK_SET;
  ਜੇ( e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ
   || (e_File_ਤਾਲਾ==EXCLUSIVE_ਤਾਲਾ && pFile->e_File_ਤਾਲਾ==RESERVED_ਤਾਲਾ)
  ){
    ਤਾਲਾ.l_type = (e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ?F_RDLCK:F_WRLCK);
    ਤਾਲਾ.l_ਸ਼ੁਰੂ = PENDING_BYTE;
    ਜੇ( unix_File_ਤਾਲਾ(pFile, &ਤਾਲਾ) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_ਤਾਲਾ);
      ਜੇ( rc!=SQLITE_BUSY ){
        store_ਆਖਰੀ_Errno(pFile, tErrno);
      }
      ਜਾਓ ਸਿਰਾ_ਤਾਲਾ;
    }ਨਹੀਂਤਾਂ ਜੇ( e_File_ਤਾਲਾ==EXCLUSIVE_ਤਾਲਾ ){
      pFile->e_File_ਤਾਲਾ = PENDING_ਤਾਲਾ;
      pInode->e_File_ਤਾਲਾ = PENDING_ਤਾਲਾ;
    }
  }


  /* If control gets to this point, then actually go ahead and make
  ** operating system calls for the specified lock.
  */
  ਜੇ( e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pInode->nShared==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pInode->e_File_ਤਾਲਾ==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK );

    /* Now get the read-lock */
    ਤਾਲਾ.l_ਸ਼ੁਰੂ = SHARED_ਪਹਿਲਾ;
    ਤਾਲਾ.l_len = SHARED_ਮਾਪ;
    ਜੇ( unix_File_ਤਾਲਾ(pFile, &ਤਾਲਾ) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_ਤਾਲਾ);
    }

    /* Drop the temporary PENDING lock */
    ਤਾਲਾ.l_ਸ਼ੁਰੂ = PENDING_BYTE;
    ਤਾਲਾ.l_len = 1L;
    ਤਾਲਾ.l_type = F_UNLCK;
    ਜੇ( unix_File_ਤਾਲਾ(pFile, &ਤਾਲਾ) && rc==SQLITE_OK ){
      /* This could happen with a network mount */
      tErrno = errno;
      rc = SQLITE_IOERR_UNLOCK;
    }

    ਜੇ( rc ){
      ਜੇ( rc!=SQLITE_BUSY ){
        store_ਆਖਰੀ_Errno(pFile, tErrno);
      }
      ਜਾਓ ਸਿਰਾ_ਤਾਲਾ;
    }ਨਹੀਂਤਾਂ{
      pFile->e_File_ਤਾਲਾ = SHARED_ਤਾਲਾ;
      pInode->n_ਤਾਲਾ++;
      pInode->nShared = 1;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( e_File_ਤਾਲਾ==EXCLUSIVE_ਤਾਲਾ && pInode->nShared>1 ){
    /* We are trying for an exclusive lock but another thread in this
    ** same process is still holding a shared lock. */
    rc = SQLITE_BUSY;
  }ਨਹੀਂਤਾਂ ਜੇ( unix_Is_Sharing_Shm_ਗੰਢ(pFile) ){
    /* We are in WAL mode and attempting to delete the SHM and WAL
    ** files due to closing the connection or changing out of WAL mode,
    ** but another process still holds locks on the SHM file, thus
    ** indicating that database locks have been broken, perhaps due
    ** to a rogue close(open(dbFile)) or similar.
    */
    rc = SQLITE_BUSY;
  }ਨਹੀਂਤਾਂ{
    /* The request was for a RESERVED or EXCLUSIVE lock.  It is
    ** assumed that there is a SHARED or greater lock on the file
    ** already.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( 0!=pFile->e_File_ਤਾਲਾ );
    ਤਾਲਾ.l_type = F_WRLCK;

    ਪ੍ਰਤਿਜ੍ਞਾ( e_File_ਤਾਲਾ==RESERVED_ਤਾਲਾ || e_File_ਤਾਲਾ==EXCLUSIVE_ਤਾਲਾ );
    ਜੇ( e_File_ਤਾਲਾ==RESERVED_ਤਾਲਾ ){
      ਤਾਲਾ.l_ਸ਼ੁਰੂ = RESERVED_BYTE;
      ਤਾਲਾ.l_len = 1L;
    }ਨਹੀਂਤਾਂ{
      ਤਾਲਾ.l_ਸ਼ੁਰੂ = SHARED_ਪਹਿਲਾ;
      ਤਾਲਾ.l_len = SHARED_ਮਾਪ;
    }

    ਜੇ( unix_File_ਤਾਲਾ(pFile, &ਤਾਲਾ) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_ਤਾਲਾ);
      ਜੇ( rc!=SQLITE_BUSY ){
        store_ਆਖਰੀ_Errno(pFile, tErrno);
      }
    }
  }


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* Set up the transaction-counter change checking flags when
  ** transitioning from a SHARED to a RESERVED lock.  The change
  ** from SHARED to RESERVED marks the beginning of a normal
  ** write operation (not a hot journal rollback).
  */
  ਜੇ( rc==SQLITE_OK
   && pFile->e_File_ਤਾਲਾ<=SHARED_ਤਾਲਾ
   && e_File_ਤਾਲਾ==RESERVED_ਤਾਲਾ
  ){
    pFile->transCntrChng = 0;
    pFile->dbUpdate = 0;
    pFile->ਵਿੱਚ_ਸਾਧਾਰਨ_Write = 1;
  }
#ਜੇ_ਅੰਤ

  ਜੇ( rc==SQLITE_OK ){
    pFile->e_File_ਤਾਲਾ = e_File_ਤਾਲਾ;
    pInode->e_File_ਤਾਲਾ = e_File_ਤਾਲਾ;
  }

ਸਿਰਾ_ਤਾਲਾ:
  sqlite3_mutex_leave(pInode->p_ਤਾਲਾ_Mutex);
  OSTRACE(("LOCK    %d %s %s (unix)\n", pFile->h, az_File_ਤਾਲਾ(e_File_ਤਾਲਾ),
      rc==SQLITE_OK ? "ok" : "failed"));
  ਵਾਪਸ rc;
}

/*
** Add the file descriptor used by file handle pFile to the corresponding
** pUnused list.
*/
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_Pending_Fd(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  Unix_ਅਣਵਰਤਿਆ_Fd *p = pFile->p_Preallocated_ਅਣਵਰਤਿਆ;
  ਪ੍ਰਤਿਜ੍ਞਾ( unixFileMutexHeld(pFile) );
  p->p_ਅਗਲਾ = pInode->p_ਅਣਵਰਤਿਆ;
  pInode->p_ਅਣਵਰਤਿਆ = p;
  pFile->h = -1;
  pFile->p_Preallocated_ਅਣਵਰਤਿਆ = 0;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
**
** If handleNFSUnlock is true, then on downgrading an EXCLUSIVE_LOCK to SHARED
** the byte range is divided into 2 parts and the first part is unlocked then
** set to a read lock, then the other part is simply unlocked.  This works
** around a bug in BSD NFS lockd (also seen on MacOSX 10.3+) that fails to
** remove the write lock on a region when a read lock is set.
*/
ਸਥਿਰ ਪੂਰਨ posixUnlock(sqlite3_file *id, ਪੂਰਨ e_File_ਤਾਲਾ, ਪੂਰਨ handleNFSUnlock){
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  ਬਣਤਰ flock ਤਾਲਾ;
  ਪੂਰਨ rc = SQLITE_OK;

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  OSTRACE(("UNLOCK  %d %d was %d(%d,%d) pid=%d (unix)\n", pFile->h, e_File_ਤਾਲਾ,
      pFile->e_File_ਤਾਲਾ, pFile->pInode->e_File_ਤਾਲਾ, pFile->pInode->nShared,
      osGetpid(0)));

  ਪ੍ਰਤਿਜ੍ਞਾ( e_File_ਤਾਲਾ<=SHARED_ਤਾਲਾ );
  ਜੇ( pFile->e_File_ਤਾਲਾ<=e_File_ਤਾਲਾ ){
    ਵਾਪਸ SQLITE_OK;
  }
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->p_ਤਾਲਾ_Mutex);
  ਪ੍ਰਤਿਜ੍ਞਾ( pInode->nShared!=0 );
  ਜੇ( pFile->e_File_ਤਾਲਾ>SHARED_ਤਾਲਾ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pInode->e_File_ਤਾਲਾ==pFile->e_File_ਤਾਲਾ );

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    /* When reducing a lock such that other processes can start
    ** reading the database file again, make sure that the
    ** transaction counter was updated if any part of the database
    ** file changed.  If the transaction counter is not updated,
    ** other connections to the same file might not realize that
    ** the file has changed and hence might not know to flush their
    ** cache.  The use of a stale cache can lead to database corruption.
    */
    pFile->ਵਿੱਚ_ਸਾਧਾਰਨ_Write = 0;
#ਜੇ_ਅੰਤ

    /* downgrading to a shared lock on NFS involves clearing the write lock
    ** before establishing the readlock - to avoid a race condition we downgrade
    ** the lock in 2 blocks, so that part of the range will be covered by a
    ** write lock until the rest is covered by a read lock:
    **  1:   [WWWWW]
    **  2:   [....W]
    **  3:   [RRRRW]
    **  4:   [RRRR.]
    */
    ਜੇ( e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ ){
#ਜੇ !defined(__APPLE__) || !SQLITE_ENABLE_LOCKING_STYLE
      (ਖਾਲੀ)handleNFSUnlock;
      ਪ੍ਰਤਿਜ੍ਞਾ( handleNFSUnlock==0 );
#ਜੇ_ਅੰਤ
#ਜੇ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
      ਜੇ( handleNFSUnlock ){
        ਪੂਰਨ tErrno;               /* Error code from system call errors */
        off_t div_ਮਾਪ = SHARED_ਮਾਪ - 1;
      
        ਤਾਲਾ.l_type = F_UNLCK;
        ਤਾਲਾ.l_whence = SEEK_SET;
        ਤਾਲਾ.l_ਸ਼ੁਰੂ = SHARED_ਪਹਿਲਾ;
        ਤਾਲਾ.l_len = div_ਮਾਪ;
        ਜੇ( unix_File_ਤਾਲਾ(pFile, &ਤਾਲਾ)==(-1) ){
          tErrno = errno;
          rc = SQLITE_IOERR_UNLOCK;
          store_ਆਖਰੀ_Errno(pFile, tErrno);
          ਜਾਓ ਸਿਰਾ_unlock;
        }
        ਤਾਲਾ.l_type = F_RDLCK;
        ਤਾਲਾ.l_whence = SEEK_SET;
        ਤਾਲਾ.l_ਸ਼ੁਰੂ = SHARED_ਪਹਿਲਾ;
        ਤਾਲਾ.l_len = div_ਮਾਪ;
        ਜੇ( unix_File_ਤਾਲਾ(pFile, &ਤਾਲਾ)==(-1) ){
          tErrno = errno;
          rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_RDLOCK);
          ਜੇ( IS_ਤਾਲਾ_ERROR(rc) ){
            store_ਆਖਰੀ_Errno(pFile, tErrno);
          }
          ਜਾਓ ਸਿਰਾ_unlock;
        }
        ਤਾਲਾ.l_type = F_UNLCK;
        ਤਾਲਾ.l_whence = SEEK_SET;
        ਤਾਲਾ.l_ਸ਼ੁਰੂ = SHARED_ਪਹਿਲਾ+div_ਮਾਪ;
        ਤਾਲਾ.l_len = SHARED_ਮਾਪ-div_ਮਾਪ;
        ਜੇ( unix_File_ਤਾਲਾ(pFile, &ਤਾਲਾ)==(-1) ){
          tErrno = errno;
          rc = SQLITE_IOERR_UNLOCK;
          store_ਆਖਰੀ_Errno(pFile, tErrno);
          ਜਾਓ ਸਿਰਾ_unlock;
        }
      }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
      {
        ਤਾਲਾ.l_type = F_RDLCK;
        ਤਾਲਾ.l_whence = SEEK_SET;
        ਤਾਲਾ.l_ਸ਼ੁਰੂ = SHARED_ਪਹਿਲਾ;
        ਤਾਲਾ.l_len = SHARED_ਮਾਪ;
        ਜੇ( unix_File_ਤਾਲਾ(pFile, &ਤਾਲਾ) ){
          /* In theory, the call to unixFileLock() cannot fail because another
          ** process is holding an incompatible lock. If it does, this
          ** indicates that the other process is not following the locking
          ** protocol. If this happens, return SQLITE_IOERR_RDLOCK. Returning
          ** SQLITE_BUSY would confuse the upper layer (in practice it causes
          ** an assert to fail). */
          rc = SQLITE_IOERR_RDLOCK;
          store_ਆਖਰੀ_Errno(pFile, errno);
          ਜਾਓ ਸਿਰਾ_unlock;
        }
      }
    }
    ਤਾਲਾ.l_type = F_UNLCK;
    ਤਾਲਾ.l_whence = SEEK_SET;
    ਤਾਲਾ.l_ਸ਼ੁਰੂ = PENDING_BYTE;
    ਤਾਲਾ.l_len = 2L;  ਪ੍ਰਤਿਜ੍ਞਾ( PENDING_BYTE+1==RESERVED_BYTE );
    ਜੇ( unix_File_ਤਾਲਾ(pFile, &ਤਾਲਾ)==0 ){
      pInode->e_File_ਤਾਲਾ = SHARED_ਤਾਲਾ;
    }ਨਹੀਂਤਾਂ{
      rc = SQLITE_IOERR_UNLOCK;
      store_ਆਖਰੀ_Errno(pFile, errno);
      ਜਾਓ ਸਿਰਾ_unlock;
    }
  }
  ਜੇ( e_File_ਤਾਲਾ==NO_ਤਾਲਾ ){
    /* Decrement the shared lock counter.  Release the lock using an
    ** OS call only when all threads in this same process have released
    ** the lock.
    */
    pInode->nShared--;
    ਜੇ( pInode->nShared==0 ){
      ਤਾਲਾ.l_type = F_UNLCK;
      ਤਾਲਾ.l_whence = SEEK_SET;
      ਤਾਲਾ.l_ਸ਼ੁਰੂ = ਤਾਲਾ.l_len = 0L;
      ਜੇ( unix_File_ਤਾਲਾ(pFile, &ਤਾਲਾ)==0 ){
        pInode->e_File_ਤਾਲਾ = NO_ਤਾਲਾ;
      }ਨਹੀਂਤਾਂ{
        rc = SQLITE_IOERR_UNLOCK;
        store_ਆਖਰੀ_Errno(pFile, errno);
        pInode->e_File_ਤਾਲਾ = NO_ਤਾਲਾ;
        pFile->e_File_ਤਾਲਾ = NO_ਤਾਲਾ;
      }
    }

    /* Decrement the count of locks against this same file.  When the
    ** count reaches zero, close any other file descriptors whose close
    ** was deferred because of outstanding locks.
    */
    pInode->n_ਤਾਲਾ--;
    ਪ੍ਰਤਿਜ੍ਞਾ( pInode->n_ਤਾਲਾ>=0 );
    ਜੇ( pInode->n_ਤਾਲਾ==0 ) closePendingFds(pFile);
  }

ਸਿਰਾ_unlock:
  sqlite3_mutex_leave(pInode->p_ਤਾਲਾ_Mutex);
  ਜੇ( rc==SQLITE_OK ){
    pFile->e_File_ਤਾਲਾ = e_File_ਤਾਲਾ;
  }
  ਵਾਪਸ rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
ਸਥਿਰ ਪੂਰਨ unixUnlock(sqlite3_file *id, ਪੂਰਨ e_File_ਤਾਲਾ){
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  ਪ੍ਰਤਿਜ੍ਞਾ( e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ || ((unixFile *)id)->n_Fetch_ਬਾਹਰਲਾ==0 );
#ਜੇ_ਅੰਤ
  ਵਾਪਸ posixUnlock(id, e_File_ਤਾਲਾ, 0);
}

#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
ਸਥਿਰ ਪੂਰਨ unixMapfile(unixFile *pFd, i64 nByte);
ਸਥਿਰ ਖਾਲੀ unixUnmapfile(unixFile *pFd);
#ਜੇ_ਅੰਤ

/*
** This function performs the parts of the "close file" operation
** common to all locking schemes. It closes the directory and file
** handles, if they are valid, and sets all fields of the unixFile
** structure to 0.
**
** It is *not* necessary to hold the mutex when this routine is called,
** even on VxWorks.  A mutex will be acquired on VxWorks by the
** vxworksReleaseFileId() routine.
*/
ਸਥਿਰ ਪੂਰਨ closeUnixFile(sqlite3_file *id){
  unixFile *pFile = (unixFile*)id;
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  unixUnmapfile(pFile);
#ਜੇ_ਅੰਤ
  ਜੇ( pFile->h>=0 ){
    robust_close(pFile, pFile->h, __LINE__);
    pFile->h = -1;
  }
#ਜੇ OS_VXWORKS
  ਜੇ( pFile->pId ){
    ਜੇ( pFile->ctrlFlags & UNIXFILE_DELETE ){
      osUnlink(pFile->pId->zCanonicalName);
    }
    vxworksReleaseFileId(pFile->pId);
    pFile->pId = 0;
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_UNLINK_AFTER_CLOSE
  ਜੇ( pFile->ctrlFlags & UNIXFILE_DELETE ){
    osUnlink(pFile->z_ਰਸਤਾ);
    sqlite3_free(*(ਅੱਖਰ**)&pFile->z_ਰਸਤਾ);
    pFile->z_ਰਸਤਾ = 0;
  }
#ਜੇ_ਅੰਤ
  OSTRACE(("CLOSE   %-3d\n", pFile->h));
  OpenCounter(-1);
  sqlite3_free(pFile->p_Preallocated_ਅਣਵਰਤਿਆ);
  memset(pFile, 0, ਆਕਾਰ(unixFile));
  ਵਾਪਸ SQLITE_OK;
}

/*
** Close a file.
*/
ਸਥਿਰ ਪੂਰਨ unixClose(sqlite3_file *id){
  ਪੂਰਨ rc = SQLITE_OK;
  unixFile *pFile = (unixFile *)id;
  unixInodeInfo *pInode = pFile->pInode;

  ਪ੍ਰਤਿਜ੍ਞਾ( pInode!=0 );
  verifyDbFile(pFile);
  unixUnlock(id, NO_ਤਾਲਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ( unixFileMutexNotheld(pFile) );
  unixEnterMutex();

  /* unixFile.pInode is always valid here. Otherwise, a different close
  ** routine (e.g. nolockClose()) would be called instead.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->pInode->n_ਤਾਲਾ>0 || pFile->pInode->b_Process_ਤਾਲਾ==0 );
  sqlite3_mutex_enter(pInode->p_ਤਾਲਾ_Mutex);
  ਜੇ( pInode->n_ਤਾਲਾ ){
    /* If there are outstanding locks, do not actually close the file just
    ** yet because that would clear those locks.  Instead, add the file
    ** descriptor to pInode->pUnused list.  It will be automatically closed
    ** when the last lock is cleared.
    */
    ਸੈੱਟ_Pending_Fd(pFile);
  }
  sqlite3_mutex_leave(pInode->p_ਤਾਲਾ_Mutex);
  releaseInodeInfo(pFile);
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->pShm==0 );
  rc = closeUnixFile(id);
  unixLeaveMutex();
  ਵਾਪਸ rc;
}

/************** End of the posix advisory lock implementation *****************
******************************************************************************/

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

ਸਥਿਰ ਪੂਰਨ nolock_Check_Reserved_ਤਾਲਾ(sqlite3_file *Not_ਵਰਤਿਆ, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  *p_Res_ਬਾਹਰਲਾ = 0;
  ਵਾਪਸ SQLITE_OK;
}
ਸਥਿਰ ਪੂਰਨ nolock_ਤਾਲਾ(sqlite3_file *Not_ਵਰਤਿਆ, ਪੂਰਨ NotUsed2){
  ਅਣਵਰਤਿਆ_PARAMETER2(Not_ਵਰਤਿਆ, NotUsed2);
  ਵਾਪਸ SQLITE_OK;
}
ਸਥਿਰ ਪੂਰਨ nolockUnlock(sqlite3_file *Not_ਵਰਤਿਆ, ਪੂਰਨ NotUsed2){
  ਅਣਵਰਤਿਆ_PARAMETER2(Not_ਵਰਤਿਆ, NotUsed2);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Close the file.
*/
ਸਥਿਰ ਪੂਰਨ nolockClose(sqlite3_file *id) {
  ਵਾਪਸ closeUnixFile(id);
}

/******************* End of the no-op lock implementation *********************
******************************************************************************/

/******************************************************************************
************************* Begin dot-file Locking ******************************
**
** The dotfile locking implementation uses the existence of separate lock
** files (really a directory) to control access to the database.  This works
** on just about every filesystem imaginable.  But there are serious downsides:
**
**    (1)  There is zero concurrency.  A single reader blocks all other
**         connections from reading or writing the database.
**
**    (2)  An application crash or power loss can leave stale lock files
**         sitting around that need to be cleared manually.
**
** Nevertheless, a dotlock is an appropriate locking mode for use if no
** other locking strategy is available.
**
** Dotfile locking works by creating a subdirectory in the same directory as
** the database and with the same name but with a ".lock" extension added.
** The existence of a lock directory implies an EXCLUSIVE lock.  All other
** lock types (SHARED, RESERVED, PENDING) are mapped into EXCLUSIVE.
*/

/*
** The file suffix added to the data base filename in order to create the
** lock directory.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DOTLOCK_SUFFIX ".lock"

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If the caller holds a SHARED
** or greater lock when it is called, then it is assumed that no other
** client may hold RESERVED. Or, if the caller holds no lock, then it
** is assumed another client holds RESERVED if the lock-file exists.
*/
ਸਥਿਰ ਪੂਰਨ dotlock_Check_Reserved_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ) {
  unixFile *pFile = (unixFile*)id;
  SimulateIOError( ਵਾਪਸ SQLITE_IOERR_CHECKRESERVEDLOCK; );

  ਜੇ( pFile->e_File_ਤਾਲਾ>=SHARED_ਤਾਲਾ ){
    *p_Res_ਬਾਹਰਲਾ = 0;
  }ਨਹੀਂਤਾਂ{
    *p_Res_ਬਾਹਰਲਾ = osAccess((ਅਚਲ ਅੱਖਰ*)pFile->lockingContext, 0)==0;
  }
  OSTRACE(("TEST WR-LOCK %d %d %d (dotlock)\n", pFile->h, 0, *p_Res_ਬਾਹਰਲਾ));
  ਵਾਪਸ SQLITE_OK;
}

/*
** Lock the file with the lock specified by parameter eFileLock - one
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
** This routine will only increase a lock.  Use the sqlite3OsUnlock()
** routine to lower a locking level.
**
** With dotfile locking, we really only support state (4): EXCLUSIVE.
** But we track the other locking levels internally.
*/
ਸਥਿਰ ਪੂਰਨ dotlock_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ e_File_ਤਾਲਾ) {
  unixFile *pFile = (unixFile*)id;
  ਅੱਖਰ *z_ਤਾਲਾ_File = (ਅੱਖਰ *)pFile->lockingContext;
  ਪੂਰਨ rc = SQLITE_OK;


  /* If we have any lock, then the lock file already exists.  All we have
  ** to do is adjust our internal record of the lock level.
  */
  ਜੇ( pFile->e_File_ਤਾਲਾ > NO_ਤਾਲਾ ){
    pFile->e_File_ਤਾਲਾ = e_File_ਤਾਲਾ;
    /* Always update the timestamp on the old file */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_UTIME
    utime(z_ਤਾਲਾ_File, ਨਿਰਾ);
#ਨਹੀਂਤਾਂ
    utimes(z_ਤਾਲਾ_File, ਨਿਰਾ);
#ਜੇ_ਅੰਤ
    ਵਾਪਸ SQLITE_OK;
  }

  /* grab an exclusive lock */
  rc = osMkdir(z_ਤਾਲਾ_File, 0777);
  ਜੇ( rc<0 ){
    /* failed to open/create the lock directory */
    ਪੂਰਨ tErrno = errno;
    ਜੇ( EEXIST == tErrno ){
      rc = SQLITE_BUSY;
    } ਨਹੀਂਤਾਂ {
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_ਤਾਲਾ);
      ਜੇ( rc!=SQLITE_BUSY ){
        store_ਆਖਰੀ_Errno(pFile, tErrno);
      }
    }
    ਵਾਪਸ rc;
  }

  /* got it, set the type and return ok */
  pFile->e_File_ਤਾਲਾ = e_File_ਤਾਲਾ;
  ਵਾਪਸ rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
**
** When the locking level reaches NO_LOCK, delete the lock file.
*/
ਸਥਿਰ ਪੂਰਨ dotlockUnlock(sqlite3_file *id, ਪੂਰਨ e_File_ਤਾਲਾ) {
  unixFile *pFile = (unixFile*)id;
  ਅੱਖਰ *z_ਤਾਲਾ_File = (ਅੱਖਰ *)pFile->lockingContext;
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (dotlock)\n", pFile->h, e_File_ਤਾਲਾ,
           pFile->e_File_ਤਾਲਾ, osGetpid(0)));
  ਪ੍ਰਤਿਜ੍ਞਾ( e_File_ਤਾਲਾ<=SHARED_ਤਾਲਾ );

  /* no-op if possible */
  ਜੇ( pFile->e_File_ਤਾਲਾ==e_File_ਤਾਲਾ ){
    ਵਾਪਸ SQLITE_OK;
  }

  /* To downgrade to shared, simply update our internal notion of the
  ** lock state.  No need to mess with the file on disk.
  */
  ਜੇ( e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ ){
    pFile->e_File_ਤਾਲਾ = SHARED_ਤਾਲਾ;
    ਵਾਪਸ SQLITE_OK;
  }

  /* To fully unlock the database, delete the lock file */
  ਪ੍ਰਤਿਜ੍ਞਾ( e_File_ਤਾਲਾ==NO_ਤਾਲਾ );
  rc = osRmdir(z_ਤਾਲਾ_File);
  ਜੇ( rc<0 ){
    ਪੂਰਨ tErrno = errno;
    ਜੇ( tErrno==ENOENT ){
      rc = SQLITE_OK;
    }ਨਹੀਂਤਾਂ{
      rc = SQLITE_IOERR_UNLOCK;
      store_ਆਖਰੀ_Errno(pFile, tErrno);
    }
    ਵਾਪਸ rc;
  }
  pFile->e_File_ਤਾਲਾ = NO_ਤਾਲਾ;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Close a file.  Make sure the lock has been released before closing.
*/
ਸਥਿਰ ਪੂਰਨ dotlockClose(sqlite3_file *id) {
  unixFile *pFile = (unixFile*)id;
  ਪ੍ਰਤਿਜ੍ਞਾ( id!=0 );
  dotlockUnlock(id, NO_ਤਾਲਾ);
  sqlite3_free(pFile->lockingContext);
  ਵਾਪਸ closeUnixFile(id);
}
/****************** End of the dot-file lock implementation *******************
******************************************************************************/

/******************************************************************************
************************** Begin flock Locking ********************************
**
** Use the flock() system call to do file locking.
**
** flock() locking is like dot-file locking in that the various
** fine-grain locking levels supported by SQLite are collapsed into
** a single exclusive lock.  In other words, SHARED, RESERVED, and
** PENDING locks are the same thing as an EXCLUSIVE lock.  SQLite
** still works when you do this, but concurrency is reduced since
** only a single process can be reading the database at a time.
**
** Omit this section if SQLITE_ENABLE_LOCKING_STYLE is turned off
*/
#ਜੇ SQLITE_ENABLE_LOCKING_STYLE

/*
** Retry flock() calls that fail with EINTR
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ EINTR
ਸਥਿਰ ਪੂਰਨ robust_flock(ਪੂਰਨ fd, ਪੂਰਨ op){
  ਪੂਰਨ rc;
  ਕਰੋ{ rc = flock(fd,op); }ਜਦੋਂ( rc<0 && errno==EINTR );
  ਵਾਪਸ rc;
}
#ਨਹੀਂਤਾਂ
# define robust_flock(a,b) flock(a,b)
#ਜੇ_ਅੰਤ
   

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
ਸਥਿਰ ਪੂਰਨ flock_Check_Reserved_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  unixFile *pFile = (unixFile*)id;
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(id);
#ਜੇ_ਅੰਤ

  SimulateIOError( ਵਾਪਸ SQLITE_IOERR_CHECKRESERVEDLOCK; );

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->e_File_ਤਾਲਾ<=SHARED_ਤਾਲਾ );

  /* The flock VFS only ever takes exclusive locks (see function flockLock).
  ** Therefore, if this connection is holding any lock at all, no other
  ** connection may be holding a RESERVED lock. So set *pResOut to 0
  ** in this case.
  **
  ** Or, this connection may be holding no lock. In that case, set *pResOut to
  ** 0 as well. The caller will then attempt to take an EXCLUSIVE lock on the
  ** db in order to roll the hot journal back. If there is another connection
  ** holding a lock, that attempt will fail and an SQLITE_BUSY returned to
  ** the user. With other VFS, we try to avoid this, in order to allow a reader
  ** to proceed while a writer is preparing its transaction. But that won't
  ** work with the flock VFS - as it always takes EXCLUSIVE locks - so it is
  ** not a problem in this case.  */
  *p_Res_ਬਾਹਰਲਾ = 0;

  ਵਾਪਸ SQLITE_OK;
}

/*
** Lock the file with the lock specified by parameter eFileLock - one
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
** flock() only really support EXCLUSIVE locks.  We track intermediate
** lock states in the sqlite3_file structure, but all locks SHARED or
** above are really EXCLUSIVE locks and exclude all other processes from
** access the file.
**
** This routine will only increase a lock.  Use the sqlite3OsUnlock()
** routine to lower a locking level.
*/
ਸਥਿਰ ਪੂਰਨ flock_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ e_File_ਤਾਲਾ) {
  ਪੂਰਨ rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );

  /* if we already have a lock, it is exclusive.
  ** Just adjust level and punt on outta here. */
  ਜੇ (pFile->e_File_ਤਾਲਾ > NO_ਤਾਲਾ) {
    pFile->e_File_ਤਾਲਾ = e_File_ਤਾਲਾ;
    ਵਾਪਸ SQLITE_OK;
  }

  /* grab an exclusive lock */

  ਜੇ (robust_flock(pFile->h, LOCK_EX | LOCK_NB)) {
    ਪੂਰਨ tErrno = errno;
    /* didn't get, must be busy */
    rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_ਤਾਲਾ);
    ਜੇ( IS_ਤਾਲਾ_ERROR(rc) ){
      store_ਆਖਰੀ_Errno(pFile, tErrno);
    }
  } ਨਹੀਂਤਾਂ {
    /* got it, set the type and return ok */
    pFile->e_File_ਤਾਲਾ = e_File_ਤਾਲਾ;
  }
  OSTRACE(("LOCK    %d %s %s (flock)\n", pFile->h, az_File_ਤਾਲਾ(e_File_ਤਾਲਾ),
           rc==SQLITE_OK ? "ok" : "failed"));
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_IGNORE_FLOCK_ਤਾਲਾ_ERRORS
  ਜੇ( (rc & 0xff) == SQLITE_IOERR ){
    rc = SQLITE_BUSY;
  }
#ਜੇ_ਅੰਤ /* SQLITE_IGNORE_FLOCK_LOCK_ERRORS */
  ਵਾਪਸ rc;
}


/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
ਸਥਿਰ ਪੂਰਨ flockUnlock(sqlite3_file *id, ਪੂਰਨ e_File_ਤਾਲਾ) {
  unixFile *pFile = (unixFile*)id;

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (flock)\n", pFile->h, e_File_ਤਾਲਾ,
           pFile->e_File_ਤਾਲਾ, osGetpid(0)));
  ਪ੍ਰਤਿਜ੍ਞਾ( e_File_ਤਾਲਾ<=SHARED_ਤਾਲਾ );

  /* no-op if possible */
  ਜੇ( pFile->e_File_ਤਾਲਾ==e_File_ਤਾਲਾ ){
    ਵਾਪਸ SQLITE_OK;
  }

  /* shared can just be set because we always have an exclusive */
  ਜੇ (e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ) {
    pFile->e_File_ਤਾਲਾ = e_File_ਤਾਲਾ;
    ਵਾਪਸ SQLITE_OK;
  }

  /* no, really, unlock. */
  ਜੇ( robust_flock(pFile->h, LOCK_UN) ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_IGNORE_FLOCK_ਤਾਲਾ_ERRORS
    ਵਾਪਸ SQLITE_OK;
#ਜੇ_ਅੰਤ /* SQLITE_IGNORE_FLOCK_LOCK_ERRORS */
    ਵਾਪਸ SQLITE_IOERR_UNLOCK;
  }ਨਹੀਂਤਾਂ{
    pFile->e_File_ਤਾਲਾ = NO_ਤਾਲਾ;
    ਵਾਪਸ SQLITE_OK;
  }
}

/*
** Close a file.
*/
ਸਥਿਰ ਪੂਰਨ flockClose(sqlite3_file *id) {
  ਪ੍ਰਤਿਜ੍ਞਾ( id!=0 );
  flockUnlock(id, NO_ਤਾਲਾ);
  ਵਾਪਸ closeUnixFile(id);
}

#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_LOCKING_STYLE && !OS_VXWORK */

/******************* End of the flock lock implementation *********************
******************************************************************************/

/******************************************************************************
************************ Begin Named Semaphore Locking ************************
**
** Named semaphore locking is only supported on VxWorks.
**
** Semaphore locking is like dot-lock and flock in that it really only
** supports EXCLUSIVE locking.  Only a single process can read or write
** the database file at a time.  This reduces potential concurrency, but
** makes the lock implementation much easier.
*/
#ਜੇ OS_VXWORKS

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
ਸਥਿਰ ਪੂਰਨ sem_XCheck_Reserved_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ) {
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ reserved = 0;
  unixFile *pFile = (unixFile*)id;

  SimulateIOError( ਵਾਪਸ SQLITE_IOERR_CHECKRESERVEDLOCK; );

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );

  /* Check if a thread in this process holds such a lock */
  ਜੇ( pFile->e_File_ਤਾਲਾ>SHARED_ਤਾਲਾ ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it. */
  ਜੇ( !reserved ){
    sem_t *pSem = pFile->pInode->pSem;

    ਜੇ( sem_trywait(pSem)==-1 ){
      ਪੂਰਨ tErrno = errno;
      ਜੇ( EAGAIN != tErrno ){
        rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_CHECKRESERVEDLOCK);
        store_ਆਖਰੀ_Errno(pFile, tErrno);
      } ਨਹੀਂਤਾਂ {
        /* someone else has the lock when we are in NO_LOCK */
        reserved = (pFile->e_File_ਤਾਲਾ < SHARED_ਤਾਲਾ);
      }
    }ਨਹੀਂਤਾਂ{
      /* we could have it if we want it */
      sem_post(pSem);
    }
  }
  OSTRACE(("TEST WR-LOCK %d %d %d (sem)\n", pFile->h, rc, reserved));

  *p_Res_ਬਾਹਰਲਾ = reserved;
  ਵਾਪਸ rc;
}

/*
** Lock the file with the lock specified by parameter eFileLock - one
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
** Semaphore locks only really support EXCLUSIVE locks.  We track intermediate
** lock states in the sqlite3_file structure, but all locks SHARED or
** above are really EXCLUSIVE locks and exclude all other processes from
** access the file.
**
** This routine will only increase a lock.  Use the sqlite3OsUnlock()
** routine to lower a locking level.
*/
ਸਥਿਰ ਪੂਰਨ semXLock(sqlite3_file *id, ਪੂਰਨ e_File_ਤਾਲਾ) {
  unixFile *pFile = (unixFile*)id;
  sem_t *pSem = pFile->pInode->pSem;
  ਪੂਰਨ rc = SQLITE_OK;

  /* if we already have a lock, it is exclusive.
  ** Just adjust level and punt on outta here. */
  ਜੇ (pFile->e_File_ਤਾਲਾ > NO_ਤਾਲਾ) {
    pFile->e_File_ਤਾਲਾ = e_File_ਤਾਲਾ;
    rc = SQLITE_OK;
    ਜਾਓ sem_ਸਿਰਾ_ਤਾਲਾ;
  }

  /* lock semaphore now but bail out when already locked. */
  ਜੇ( sem_trywait(pSem)==-1 ){
    rc = SQLITE_BUSY;
    ਜਾਓ sem_ਸਿਰਾ_ਤਾਲਾ;
  }

  /* got it, set the type and return ok */
  pFile->e_File_ਤਾਲਾ = e_File_ਤਾਲਾ;

 sem_ਸਿਰਾ_ਤਾਲਾ:
  ਵਾਪਸ rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
ਸਥਿਰ ਪੂਰਨ semXUnlock(sqlite3_file *id, ਪੂਰਨ e_File_ਤਾਲਾ) {
  unixFile *pFile = (unixFile*)id;
  sem_t *pSem = pFile->pInode->pSem;

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  ਪ੍ਰਤਿਜ੍ਞਾ( pSem );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (sem)\n", pFile->h, e_File_ਤਾਲਾ,
           pFile->e_File_ਤਾਲਾ, osGetpid(0)));
  ਪ੍ਰਤਿਜ੍ਞਾ( e_File_ਤਾਲਾ<=SHARED_ਤਾਲਾ );

  /* no-op if possible */
  ਜੇ( pFile->e_File_ਤਾਲਾ==e_File_ਤਾਲਾ ){
    ਵਾਪਸ SQLITE_OK;
  }

  /* shared can just be set because we always have an exclusive */
  ਜੇ (e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ) {
    pFile->e_File_ਤਾਲਾ = e_File_ਤਾਲਾ;
    ਵਾਪਸ SQLITE_OK;
  }

  /* no, really unlock. */
  ਜੇ ( sem_post(pSem)==-1 ) {
    ਪੂਰਨ rc, tErrno = errno;
    rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_UNLOCK);
    ਜੇ( IS_ਤਾਲਾ_ERROR(rc) ){
      store_ਆਖਰੀ_Errno(pFile, tErrno);
    }
    ਵਾਪਸ rc;
  }
  pFile->e_File_ਤਾਲਾ = NO_ਤਾਲਾ;
  ਵਾਪਸ SQLITE_OK;
}

/*
 ** Close a file.
 */
ਸਥਿਰ ਪੂਰਨ semXClose(sqlite3_file *id) {
  ਜੇ( id ){
    unixFile *pFile = (unixFile*)id;
    semXUnlock(id, NO_ਤਾਲਾ);
    ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
    ਪ੍ਰਤਿਜ੍ਞਾ( unixFileMutexNotheld(pFile) );
    unixEnterMutex();
    releaseInodeInfo(pFile);
    unixLeaveMutex();
    closeUnixFile(id);
  }
  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ_ਅੰਤ /* OS_VXWORKS */
/*
** Named semaphore locking is only available on VxWorks.
**
*************** End of the named semaphore lock implementation ****************
******************************************************************************/


/******************************************************************************
*************************** Begin AFP Locking *********************************
**
** AFP is the Apple Filing Protocol.  AFP is a network filesystem found
** on Apple Macintosh computers - both OS9 and OSX.
**
** Third-party implementations of AFP are available.  But this code here
** only works on OSX.
*/

#ਜੇ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
** The afpLockingContext structure contains all afp lock specific state
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ afpLockingContext afpLockingContext;
ਬਣਤਰ afpLockingContext {
  ਪੂਰਨ reserved;
  ਅਚਲ ਅੱਖਰ *db_ਰਸਤਾ;             /* Name of the open file */
};

ਬਣਤਰ Byte_Range_ਤਾਲਾ_PB2
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਵਿੱਥ;        /* offset to first byte to lock */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਲੰਮਾਈ;        /* nbr of bytes to lock */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ret_Range_ਸ਼ੁਰੂ; /* nbr of 1st byte locked if successful */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ un_ਤਾਲਾ_Flag;         /* 1 = unlock, 0 = lock */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਸ਼ੁਰੂ_ਸਿਰਾ_Flag;       /* 1=rel to end of fork, 0=rel to start */
  ਪੂਰਨ fd;                           /* file desc to assoc this lock with */
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ afpfsByteRangeLock2FSCTL        _IOWR('z', 23, ਬਣਤਰ Byte_Range_ਤਾਲਾ_PB2)

/*
** This is a utility for setting or clearing a bit-range lock on an
** AFP filesystem.
**
** Return SQLITE_OK on success, SQLITE_BUSY on failure.
*/
ਸਥਿਰ ਪੂਰਨ afp_ਸੈੱਟ_ਤਾਲਾ(
  ਅਚਲ ਅੱਖਰ *ਰਸਤਾ,              /* Name of the file to be locked or unlocked */
  unixFile *pFile,               /* Open file descriptor on path */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਵਿੱਥ,     /* First byte to be locked */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਲੰਮਾਈ,     /* Number of bytes to lock */
  ਪੂਰਨ ਸੈੱਟ_ਤਾਲਾ_Flag                /* True to set lock.  False to clear lock */
){
  ਬਣਤਰ Byte_Range_ਤਾਲਾ_PB2 pb;
  ਪੂਰਨ err;

  pb.un_ਤਾਲਾ_Flag = ਸੈੱਟ_ਤਾਲਾ_Flag ? 0 : 1;
  pb.ਸ਼ੁਰੂ_ਸਿਰਾ_Flag = 0;
  pb.ਵਿੱਥ = ਵਿੱਥ;
  pb.ਲੰਮਾਈ = ਲੰਮਾਈ;
  pb.fd = pFile->h;

  OSTRACE(("AFPSETLOCK [%s] for %d%s in range %llx:%llx\n",
    (ਸੈੱਟ_ਤਾਲਾ_Flag?"ON":"OFF"), pFile->h, (pb.fd==-1?"[testval-1]":""),
    ਵਿੱਥ, ਲੰਮਾਈ));
  err = fsctl(ਰਸਤਾ, afpfsByteRangeLock2FSCTL, &pb, 0);
  ਜੇ ( err==-1 ) {
    ਪੂਰਨ rc;
    ਪੂਰਨ tErrno = errno;
    OSTRACE(("AFPSETLOCK failed to fsctl() '%s' %d %s\n",
             ਰਸਤਾ, tErrno, strerror(tErrno)));
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_IGNORE_AFP_ਤਾਲਾ_ERRORS
    rc = SQLITE_BUSY;
#ਨਹੀਂਤਾਂ
    rc = sqliteErrorFromPosixError(tErrno,
                    ਸੈੱਟ_ਤਾਲਾ_Flag ? SQLITE_IOERR_ਤਾਲਾ : SQLITE_IOERR_UNLOCK);
#ਜੇ_ਅੰਤ /* SQLITE_IGNORE_AFP_LOCK_ERRORS */
    ਜੇ( IS_ਤਾਲਾ_ERROR(rc) ){
      store_ਆਖਰੀ_Errno(pFile, tErrno);
    }
    ਵਾਪਸ rc;
  } ਨਹੀਂਤਾਂ {
    ਵਾਪਸ SQLITE_OK;
  }
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
ਸਥਿਰ ਪੂਰਨ afp_Check_Reserved_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ reserved = 0;
  unixFile *pFile = (unixFile*)id;
  afpLockingContext *context;

  SimulateIOError( ਵਾਪਸ SQLITE_IOERR_CHECKRESERVEDLOCK; );

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  context = (afpLockingContext *) pFile->lockingContext;
  ਜੇ( context->reserved ){
    *p_Res_ਬਾਹਰਲਾ = 1;
    ਵਾਪਸ SQLITE_OK;
  }
  sqlite3_mutex_enter(pFile->pInode->p_ਤਾਲਾ_Mutex);
  /* Check if a thread in this process holds such a lock */
  ਜੇ( pFile->pInode->e_File_ਤਾਲਾ>SHARED_ਤਾਲਾ ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it.
   */
  ਜੇ( !reserved ){
    /* lock the RESERVED byte */
    ਪੂਰਨ lrc = afp_ਸੈੱਟ_ਤਾਲਾ(context->db_ਰਸਤਾ, pFile, RESERVED_BYTE, 1,1);
    ਜੇ( SQLITE_OK==lrc ){
      /* if we succeeded in taking the reserved lock, unlock it to restore
      ** the original state */
      lrc = afp_ਸੈੱਟ_ਤਾਲਾ(context->db_ਰਸਤਾ, pFile, RESERVED_BYTE, 1, 0);
    } ਨਹੀਂਤਾਂ {
      /* if we failed to get the lock then someone else must have it */
      reserved = 1;
    }
    ਜੇ( IS_ਤਾਲਾ_ERROR(lrc) ){
      rc=lrc;
    }
  }

  sqlite3_mutex_leave(pFile->pInode->p_ਤਾਲਾ_Mutex);
  OSTRACE(("TEST WR-LOCK %d %d %d (afp)\n", pFile->h, rc, reserved));

  *p_Res_ਬਾਹਰਲਾ = reserved;
  ਵਾਪਸ rc;
}

/*
** Lock the file with the lock specified by parameter eFileLock - one
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
** This routine will only increase a lock.  Use the sqlite3OsUnlock()
** routine to lower a locking level.
*/
ਸਥਿਰ ਪੂਰਨ afp_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ e_File_ਤਾਲਾ){
  ਪੂਰਨ rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode = pFile->pInode;
  afpLockingContext *context = (afpLockingContext *) pFile->lockingContext;

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  OSTRACE(("LOCK    %d %s was %s(%s,%d) pid=%d (afp)\n", pFile->h,
           az_File_ਤਾਲਾ(e_File_ਤਾਲਾ), az_File_ਤਾਲਾ(pFile->e_File_ਤਾਲਾ),
           az_File_ਤਾਲਾ(pInode->e_File_ਤਾਲਾ), pInode->nShared , osGetpid(0)));

  /* If there is already a lock of this type or more restrictive on the
  ** unixFile, do nothing. Don't use the afp_end_lock: exit path, as
  ** unixEnterMutex() hasn't been called yet.
  */
  ਜੇ( pFile->e_File_ਤਾਲਾ>=e_File_ਤਾਲਾ ){
    OSTRACE(("LOCK    %d %s ok (already held) (afp)\n", pFile->h,
           az_File_ਤਾਲਾ(e_File_ਤਾਲਾ)));
    ਵਾਪਸ SQLITE_OK;
  }

  /* Make sure the locking sequence is correct
  **  (1) We never move from unlocked to anything higher than shared lock.
  **  (2) SQLite never explicitly requests a pending lock.
  **  (3) A shared lock is always held when a reserve lock is requested.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->e_File_ਤਾਲਾ!=NO_ਤਾਲਾ || e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( e_File_ਤਾਲਾ!=PENDING_ਤਾਲਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( e_File_ਤਾਲਾ!=RESERVED_ਤਾਲਾ || pFile->e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ );

  /* This mutex is needed because pFile->pInode is shared across threads
  */
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->p_ਤਾਲਾ_Mutex);

  /* If some thread using this PID has a lock via a different unixFile*
  ** handle that precludes the requested lock, return BUSY.
  */
  ਜੇ( (pFile->e_File_ਤਾਲਾ!=pInode->e_File_ਤਾਲਾ &&
       (pInode->e_File_ਤਾਲਾ>=PENDING_ਤਾਲਾ || e_File_ਤਾਲਾ>SHARED_ਤਾਲਾ))
     ){
    rc = SQLITE_BUSY;
    ਜਾਓ afp_ਸਿਰਾ_ਤਾਲਾ;
  }

  /* If a SHARED lock is requested, and some thread using this PID already
  ** has a SHARED or RESERVED lock, then increment reference counts and
  ** return SQLITE_OK.
  */
  ਜੇ( e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ &&
     (pInode->e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ || pInode->e_File_ਤਾਲਾ==RESERVED_ਤਾਲਾ) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ );
    ਪ੍ਰਤਿਜ੍ਞਾ( pFile->e_File_ਤਾਲਾ==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pInode->nShared>0 );
    pFile->e_File_ਤਾਲਾ = SHARED_ਤਾਲਾ;
    pInode->nShared++;
    pInode->n_ਤਾਲਾ++;
    ਜਾਓ afp_ਸਿਰਾ_ਤਾਲਾ;
  }
  
  /* A PENDING lock is needed before acquiring a SHARED lock and before
  ** acquiring an EXCLUSIVE lock.  For the SHARED lock, the PENDING will
  ** be released.
  */
  ਜੇ( e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ
      || (e_File_ਤਾਲਾ==EXCLUSIVE_ਤਾਲਾ && pFile->e_File_ਤਾਲਾ<PENDING_ਤਾਲਾ)
  ){
    ਪੂਰਨ failed;
    failed = afp_ਸੈੱਟ_ਤਾਲਾ(context->db_ਰਸਤਾ, pFile, PENDING_BYTE, 1, 1);
    ਜੇ (failed) {
      rc = failed;
      ਜਾਓ afp_ਸਿਰਾ_ਤਾਲਾ;
    }
  }

  /* If control gets to this point, then actually go ahead and make
  ** operating system calls for the specified lock.
  */
  ਜੇ( e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ ){
    ਪੂਰਨ lrc1, lrc2, lrc1Errno = 0;
    ਲੰਮਾ lk, ਨਕਾਬ;
  
    ਪ੍ਰਤਿਜ੍ਞਾ( pInode->nShared==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pInode->e_File_ਤਾਲਾ==0 );
      
    ਨਕਾਬ = (ਆਕਾਰ(ਲੰਮਾ)==8) ? ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64 : 0x7fffffff;
    /* Now get the read-lock SHARED_LOCK */
    /* note that the quality of the randomness doesn't matter that much */
    lk = random();
    pInode->sharedByte = (lk & ਨਕਾਬ)%(SHARED_ਮਾਪ - 1);
    lrc1 = afp_ਸੈੱਟ_ਤਾਲਾ(context->db_ਰਸਤਾ, pFile,
          SHARED_ਪਹਿਲਾ+pInode->sharedByte, 1, 1);
    ਜੇ( IS_ਤਾਲਾ_ERROR(lrc1) ){
      lrc1Errno = pFile->ਆਖਰੀ_Errno;
    }
    /* Drop the temporary PENDING lock */
    lrc2 = afp_ਸੈੱਟ_ਤਾਲਾ(context->db_ਰਸਤਾ, pFile, PENDING_BYTE, 1, 0);
  
    ਜੇ( IS_ਤਾਲਾ_ERROR(lrc1) ) {
      store_ਆਖਰੀ_Errno(pFile, lrc1Errno);
      rc = lrc1;
      ਜਾਓ afp_ਸਿਰਾ_ਤਾਲਾ;
    } ਨਹੀਂਤਾਂ ਜੇ( IS_ਤਾਲਾ_ERROR(lrc2) ){
      rc = lrc2;
      ਜਾਓ afp_ਸਿਰਾ_ਤਾਲਾ;
    } ਨਹੀਂਤਾਂ ਜੇ( lrc1 != SQLITE_OK ) {
      rc = lrc1;
    } ਨਹੀਂਤਾਂ {
      pFile->e_File_ਤਾਲਾ = SHARED_ਤਾਲਾ;
      pInode->n_ਤਾਲਾ++;
      pInode->nShared = 1;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( e_File_ਤਾਲਾ==EXCLUSIVE_ਤਾਲਾ && pInode->nShared>1 ){
    /* We are trying for an exclusive lock but another thread in this
     ** same process is still holding a shared lock. */
    rc = SQLITE_BUSY;
  }ਨਹੀਂਤਾਂ{
    /* The request was for a RESERVED or EXCLUSIVE lock.  It is
    ** assumed that there is a SHARED or greater lock on the file
    ** already.
    */
    ਪੂਰਨ failed = 0;
    ਪ੍ਰਤਿਜ੍ਞਾ( 0!=pFile->e_File_ਤਾਲਾ );
    ਜੇ (e_File_ਤਾਲਾ >= RESERVED_ਤਾਲਾ && pFile->e_File_ਤਾਲਾ < RESERVED_ਤਾਲਾ) {
        /* Acquire a RESERVED lock */
        failed = afp_ਸੈੱਟ_ਤਾਲਾ(context->db_ਰਸਤਾ, pFile, RESERVED_BYTE, 1,1);
      ਜੇ( !failed ){
        context->reserved = 1;
      }
    }
    ਜੇ (!failed && e_File_ਤਾਲਾ == EXCLUSIVE_ਤਾਲਾ) {
      /* Acquire an EXCLUSIVE lock */

      /* Remove the shared lock before trying the range.  we'll need to
      ** reestablish the shared lock if we can't get the  afpUnlock
      */
      ਜੇ( !(failed = afp_ਸੈੱਟ_ਤਾਲਾ(context->db_ਰਸਤਾ, pFile, SHARED_ਪਹਿਲਾ +
                         pInode->sharedByte, 1, 0)) ){
        ਪੂਰਨ failed2 = SQLITE_OK;
        /* now attempt to get the exclusive lock range */
        failed = afp_ਸੈੱਟ_ਤਾਲਾ(context->db_ਰਸਤਾ, pFile, SHARED_ਪਹਿਲਾ,
                               SHARED_ਮਾਪ, 1);
        ਜੇ( failed && (failed2 = afp_ਸੈੱਟ_ਤਾਲਾ(context->db_ਰਸਤਾ, pFile,
                       SHARED_ਪਹਿਲਾ + pInode->sharedByte, 1, 1)) ){
          /* Can't reestablish the shared lock.  Sqlite can't deal, this is
          ** a critical I/O error
          */
          rc = ((failed & 0xff) == SQLITE_IOERR) ? failed2 :
               SQLITE_IOERR_ਤਾਲਾ;
          ਜਾਓ afp_ਸਿਰਾ_ਤਾਲਾ;
        }
      }ਨਹੀਂਤਾਂ{
        rc = failed;
      }
    }
    ਜੇ( failed ){
      rc = failed;
    }
  }

  ਜੇ( rc==SQLITE_OK ){
    pFile->e_File_ਤਾਲਾ = e_File_ਤਾਲਾ;
    pInode->e_File_ਤਾਲਾ = e_File_ਤਾਲਾ;
  }ਨਹੀਂਤਾਂ ਜੇ( e_File_ਤਾਲਾ==EXCLUSIVE_ਤਾਲਾ ){
    pFile->e_File_ਤਾਲਾ = PENDING_ਤਾਲਾ;
    pInode->e_File_ਤਾਲਾ = PENDING_ਤਾਲਾ;
  }

afp_ਸਿਰਾ_ਤਾਲਾ:
  sqlite3_mutex_leave(pInode->p_ਤਾਲਾ_Mutex);
  OSTRACE(("LOCK    %d %s %s (afp)\n", pFile->h, az_File_ਤਾਲਾ(e_File_ਤਾਲਾ),
         rc==SQLITE_OK ? "ok" : "failed"));
  ਵਾਪਸ rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
ਸਥਿਰ ਪੂਰਨ afpUnlock(sqlite3_file *id, ਪੂਰਨ e_File_ਤਾਲਾ) {
  ਪੂਰਨ rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  afpLockingContext *context = (afpLockingContext *) pFile->lockingContext;
  ਪੂਰਨ skipShared = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  OSTRACE(("UNLOCK  %d %d was %d(%d,%d) pid=%d (afp)\n", pFile->h, e_File_ਤਾਲਾ,
           pFile->e_File_ਤਾਲਾ, pFile->pInode->e_File_ਤਾਲਾ, pFile->pInode->nShared,
           osGetpid(0)));

  ਪ੍ਰਤਿਜ੍ਞਾ( e_File_ਤਾਲਾ<=SHARED_ਤਾਲਾ );
  ਜੇ( pFile->e_File_ਤਾਲਾ<=e_File_ਤਾਲਾ ){
    ਵਾਪਸ SQLITE_OK;
  }
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->p_ਤਾਲਾ_Mutex);
  ਪ੍ਰਤਿਜ੍ਞਾ( pInode->nShared!=0 );
  ਜੇ( pFile->e_File_ਤਾਲਾ>SHARED_ਤਾਲਾ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pInode->e_File_ਤਾਲਾ==pFile->e_File_ਤਾਲਾ );
  
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    /* When reducing a lock such that other processes can start
    ** reading the database file again, make sure that the
    ** transaction counter was updated if any part of the database
    ** file changed.  If the transaction counter is not updated,
    ** other connections to the same file might not realize that
    ** the file has changed and hence might not know to flush their
    ** cache.  The use of a stale cache can lead to database corruption.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( pFile->ਵਿੱਚ_ਸਾਧਾਰਨ_Write==0
           || pFile->dbUpdate==0
           || pFile->transCntrChng==1 );
    pFile->ਵਿੱਚ_ਸਾਧਾਰਨ_Write = 0;
#ਜੇ_ਅੰਤ
  
    ਜੇ( pFile->e_File_ਤਾਲਾ==EXCLUSIVE_ਤਾਲਾ ){
      rc = afp_ਸੈੱਟ_ਤਾਲਾ(context->db_ਰਸਤਾ, pFile, SHARED_ਪਹਿਲਾ, SHARED_ਮਾਪ, 0);
      ਜੇ( rc==SQLITE_OK && (e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ || pInode->nShared>1) ){
        /* only re-establish the shared lock if necessary */
        ਪੂਰਨ shared_ਤਾਲਾ_Byte = SHARED_ਪਹਿਲਾ+pInode->sharedByte;
        rc = afp_ਸੈੱਟ_ਤਾਲਾ(context->db_ਰਸਤਾ, pFile, shared_ਤਾਲਾ_Byte, 1, 1);
      } ਨਹੀਂਤਾਂ {
        skipShared = 1;
      }
    }
    ਜੇ( rc==SQLITE_OK && pFile->e_File_ਤਾਲਾ>=PENDING_ਤਾਲਾ ){
      rc = afp_ਸੈੱਟ_ਤਾਲਾ(context->db_ਰਸਤਾ, pFile, PENDING_BYTE, 1, 0);
    }
    ਜੇ( rc==SQLITE_OK && pFile->e_File_ਤਾਲਾ>=RESERVED_ਤਾਲਾ && context->reserved ){
      rc = afp_ਸੈੱਟ_ਤਾਲਾ(context->db_ਰਸਤਾ, pFile, RESERVED_BYTE, 1, 0);
      ਜੇ( !rc ){
        context->reserved = 0;
      }
    }
    ਜੇ( rc==SQLITE_OK && (e_File_ਤਾਲਾ==SHARED_ਤਾਲਾ || pInode->nShared>1)){
      pInode->e_File_ਤਾਲਾ = SHARED_ਤਾਲਾ;
    }
  }
  ਜੇ( rc==SQLITE_OK && e_File_ਤਾਲਾ==NO_ਤਾਲਾ ){

    /* Decrement the shared lock counter.  Release the lock using an
    ** OS call only when all threads in this same process have released
    ** the lock.
    */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ shared_ਤਾਲਾ_Byte = SHARED_ਪਹਿਲਾ+pInode->sharedByte;
    pInode->nShared--;
    ਜੇ( pInode->nShared==0 ){
      ਜੇ( !skipShared ){
        rc = afp_ਸੈੱਟ_ਤਾਲਾ(context->db_ਰਸਤਾ, pFile, shared_ਤਾਲਾ_Byte, 1, 0);
      }
      ਜੇ( !rc ){
        pInode->e_File_ਤਾਲਾ = NO_ਤਾਲਾ;
        pFile->e_File_ਤਾਲਾ = NO_ਤਾਲਾ;
      }
    }
    ਜੇ( rc==SQLITE_OK ){
      pInode->n_ਤਾਲਾ--;
      ਪ੍ਰਤਿਜ੍ਞਾ( pInode->n_ਤਾਲਾ>=0 );
      ਜੇ( pInode->n_ਤਾਲਾ==0 ) closePendingFds(pFile);
    }
  }

  sqlite3_mutex_leave(pInode->p_ਤਾਲਾ_Mutex);
  ਜੇ( rc==SQLITE_OK ){
    pFile->e_File_ਤਾਲਾ = e_File_ਤਾਲਾ;
  }
  ਵਾਪਸ rc;
}

/*
** Close a file & cleanup AFP specific locking context
*/
ਸਥਿਰ ਪੂਰਨ afpClose(sqlite3_file *id) {
  ਪੂਰਨ rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  ਪ੍ਰਤਿਜ੍ਞਾ( id!=0 );
  afpUnlock(id, NO_ਤਾਲਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ( unixFileMutexNotheld(pFile) );
  unixEnterMutex();
  ਜੇ( pFile->pInode ){
    unixInodeInfo *pInode = pFile->pInode;
    sqlite3_mutex_enter(pInode->p_ਤਾਲਾ_Mutex);
    ਜੇ( pInode->n_ਤਾਲਾ ){
      /* If there are outstanding locks, do not actually close the file just
      ** yet because that would clear those locks.  Instead, add the file
      ** descriptor to pInode->aPending.  It will be automatically closed when
      ** the last lock is cleared.
      */
      ਸੈੱਟ_Pending_Fd(pFile);
    }
    sqlite3_mutex_leave(pInode->p_ਤਾਲਾ_Mutex);
  }
  releaseInodeInfo(pFile);
  sqlite3_free(pFile->lockingContext);
  rc = closeUnixFile(id);
  unixLeaveMutex();
  ਵਾਪਸ rc;
}

#ਜੇ_ਅੰਤ /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
/*
** The code above is the AFP lock implementation.  The code is specific
** to MacOSX and does not work on other unix platforms.  No alternative
** is available.  If you don't compile for a mac, then the "unix-afp"
** VFS is not available.
**
********************* End of the AFP lock implementation **********************
******************************************************************************/

/******************************************************************************
*************************** Begin NFS Locking ********************************/

#ਜੇ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
 ** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
 ** must be either NO_LOCK or SHARED_LOCK.
 **
 ** If the locking level of the file descriptor is already at or below
 ** the requested locking level, this routine is a no-op.
 */
ਸਥਿਰ ਪੂਰਨ nfsUnlock(sqlite3_file *id, ਪੂਰਨ e_File_ਤਾਲਾ){
  ਵਾਪਸ posixUnlock(id, e_File_ਤਾਲਾ, 1);
}

#ਜੇ_ਅੰਤ /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
/*
** The code above is the NFS lock implementation.  The code is specific
** to MacOSX and does not work on other unix platforms.  No alternative
** is available.
**
********************* End of the NFS lock implementation **********************
******************************************************************************/

/******************************************************************************
**************** Non-locking sqlite3_file methods *****************************
**
** The next division contains implementations for all methods of the
** sqlite3_file object other than the locking methods.  The locking
** methods were defined in divisions above (one locking method per
** division).  Those methods that are common to all locking modes
** are gather together into this division.
*/

/*
** Seek to the offset passed as the second argument, then read cnt
** bytes into pBuf. Return the number of bytes actually read.
**
** To avoid stomping the errno value on a failed read the lastErrno value
** is set before returning.
*/
ਸਥਿਰ ਪੂਰਨ seekAndRead(unixFile *id, sqlite3_int64 ਵਿੱਥ, ਖਾਲੀ *pBuf, ਪੂਰਨ cnt){
  ਪੂਰਨ got;
  ਪੂਰਨ ਪੂਰਵ = 0;
#ਜੇ (!defined(USE_PREAD) && !defined(USE_PREAD64))
  i64 new_ਵਿੱਥ;
#ਜੇ_ਅੰਤ
  TIMER_ਸ਼ੁਰੂ;
  ਪ੍ਰਤਿਜ੍ਞਾ( cnt==(cnt&0x1ffff) );
  ਪ੍ਰਤਿਜ੍ਞਾ( id->h>2 );
  ਕਰੋ{
#ਜੇ defined(USE_PREAD)
    got = osPread(id->h, pBuf, cnt, ਵਿੱਥ);
    SimulateIOError( got = -1 );
#ਜਾਂਜੇ defined(USE_PREAD64)
    got = osPread64(id->h, pBuf, cnt, ਵਿੱਥ);
    SimulateIOError( got = -1 );
#ਨਹੀਂਤਾਂ
    new_ਵਿੱਥ = lseek(id->h, ਵਿੱਥ, SEEK_SET);
    SimulateIOError( new_ਵਿੱਥ = -1 );
    ਜੇ( new_ਵਿੱਥ<0 ){
      store_ਆਖਰੀ_Errno((unixFile*)id, errno);
      ਵਾਪਸ -1;
    }
    got = osRead(id->h, pBuf, cnt);
#ਜੇ_ਅੰਤ
    ਜੇ( got==cnt ) ਰੋਕੋ;
    ਜੇ( got<0 ){
      ਜੇ( errno==EINTR ){ got = 1; ਜਾਰੀ; }
      ਪੂਰਵ = 0;
      store_ਆਖਰੀ_Errno((unixFile*)id,  errno);
      ਰੋਕੋ;
    }ਨਹੀਂਤਾਂ ਜੇ( got>0 ){
      cnt -= got;
      ਵਿੱਥ += got;
      ਪੂਰਵ += got;
      pBuf = (ਖਾਲੀ*)(got + (ਅੱਖਰ*)pBuf);
    }
  }ਜਦੋਂ( got>0 );
  TIMER_ਸਿਰਾ;
  OSTRACE(("READ    %-3d %5d %7lld %llu\n",
            id->h, got+ਪੂਰਵ, ਵਿੱਥ-ਪੂਰਵ, TIMER_ELAPSED));
  ਵਾਪਸ got+ਪੂਰਵ;
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
ਸਥਿਰ ਪੂਰਨ unixRead(
  sqlite3_file *id,
  ਖਾਲੀ *pBuf,
  ਪੂਰਨ amt,
  sqlite3_int64 ਵਿੱਥ
){
  unixFile *pFile = (unixFile *)id;
  ਪੂਰਨ got;
  ਪ੍ਰਤਿਜ੍ਞਾ( id );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿੱਥ>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( amt>0 );

  /* If this is a database file (not a journal, super-journal or temp
  ** file), the bytes in the locking range should never be read or written. */
#ਜੇ 0
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->p_Preallocated_ਅਣਵਰਤਿਆ==0
       || ਵਿੱਥ>=PENDING_BYTE+512
       || ਵਿੱਥ+amt<=PENDING_BYTE
  );
#ਜੇ_ਅੰਤ

#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  ਜੇ( ਵਿੱਥ<pFile->mmap_ਮਾਪ ){
    ਜੇ( ਵਿੱਥ+amt <= pFile->mmap_ਮਾਪ ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(pBuf, &((u8 *)(pFile->pMapRegion))[ਵਿੱਥ], amt);
      ਵਾਪਸ SQLITE_OK;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ nCopy = pFile->mmap_ਮਾਪ - ਵਿੱਥ;
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(pBuf, &((u8 *)(pFile->pMapRegion))[ਵਿੱਥ], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      ਵਿੱਥ += nCopy;
    }
  }
#ਜੇ_ਅੰਤ

  got = seekAndRead(pFile, ਵਿੱਥ, pBuf, amt);
  ਜੇ( got==amt ){
    ਵਾਪਸ SQLITE_OK;
  }ਨਹੀਂਤਾਂ ਜੇ( got<0 ){
    /* pFile->lastErrno has been set by seekAndRead().
    ** Usually we return SQLITE_IOERR_READ here, though for some
    ** kinds of errors we return SQLITE_IOERR_CORRUPTFS.  The
    ** SQLITE_IOERR_CORRUPTFS will be converted into SQLITE_CORRUPT
    ** prior to returning to the application by the sqlite3ApiExit()
    ** routine.
    */
    ਚੋਣ( pFile->ਆਖਰੀ_Errno ){
      ਹਾਲਤ ERANGE:
      ਹਾਲਤ EIO:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ENXIO
      ਹਾਲਤ ENXIO:
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ EDEVERR
      ਹਾਲਤ EDEVERR:
#ਜੇ_ਅੰਤ
        ਵਾਪਸ SQLITE_IOERR_CORRUPTFS;
    }
    ਵਾਪਸ SQLITE_IOERR_READ;
  }ਨਹੀਂਤਾਂ{
    store_ਆਖਰੀ_Errno(pFile, 0);   /* not a system error */
    /* Unread parts of the buffer must be zero-filled */
    memset(&((ਅੱਖਰ*)pBuf)[got], 0, amt-got);
    ਵਾਪਸ SQLITE_IOERR_SHORT_READ;
  }
}

/*
** Attempt to seek the file-descriptor passed as the first argument to
** absolute offset iOff, then attempt to write nBuf bytes of data from
** pBuf to it. If an error occurs, return -1 and set *piErrno. Otherwise,
** return the actual number of bytes written (which may be less than
** nBuf).
*/
ਸਥਿਰ ਪੂਰਨ seekAndWriteFd(
  ਪੂਰਨ fd,                         /* File descriptor to write to */
  i64 iOff,                       /* File offset to begin writing at */
  ਅਚਲ ਖਾਲੀ *pBuf,               /* Copy data from this buffer to the file */
  ਪੂਰਨ nBuf,                       /* Size of buffer pBuf in bytes */
  ਪੂਰਨ *piErrno                    /* OUT: Error number if error occurs */
){
  ਪੂਰਨ rc = 0;                     /* Value returned by system call */

  ਪ੍ਰਤਿਜ੍ਞਾ( nBuf==(nBuf&0x1ffff) );
  ਪ੍ਰਤਿਜ੍ਞਾ( fd>2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( piErrno!=0 );
  nBuf &= 0x1ffff;
  TIMER_ਸ਼ੁਰੂ;

#ਜੇ defined(USE_PREAD)
  ਕਰੋ{ rc = (ਪੂਰਨ)osPwrite(fd, pBuf, nBuf, iOff); }ਜਦੋਂ( rc<0 && errno==EINTR );
#ਜਾਂਜੇ defined(USE_PREAD64)
  ਕਰੋ{ rc = (ਪੂਰਨ)osPwrite64(fd, pBuf, nBuf, iOff);}ਜਦੋਂ( rc<0 && errno==EINTR);
#ਨਹੀਂਤਾਂ
  ਕਰੋ{
    i64 iSeek = lseek(fd, iOff, SEEK_SET);
    SimulateIOError( iSeek = -1 );
    ਜੇ( iSeek<0 ){
      rc = -1;
      ਰੋਕੋ;
    }
    rc = osWrite(fd, pBuf, nBuf);
  }ਜਦੋਂ( rc<0 && errno==EINTR );
#ਜੇ_ਅੰਤ

  TIMER_ਸਿਰਾ;
  OSTRACE(("WRITE   %-3d %5d %7lld %llu\n", fd, rc, iOff, TIMER_ELAPSED));

  ਜੇ( rc<0 ) *piErrno = errno;
  ਵਾਪਸ rc;
}


/*
** Seek to the offset in id->offset then read cnt bytes into pBuf.
** Return the number of bytes actually read.  Update the offset.
**
** To avoid stomping the errno value on a failed write the lastErrno value
** is set before returning.
*/
ਸਥਿਰ ਪੂਰਨ seekAndWrite(unixFile *id, i64 ਵਿੱਥ, ਅਚਲ ਖਾਲੀ *pBuf, ਪੂਰਨ cnt){
  ਵਾਪਸ seekAndWriteFd(id->h, ਵਿੱਥ, pBuf, cnt, &id->ਆਖਰੀ_Errno);
}


/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
ਸਥਿਰ ਪੂਰਨ unixWrite(
  sqlite3_file *id,
  ਅਚਲ ਖਾਲੀ *pBuf,
  ਪੂਰਨ amt,
  sqlite3_int64 ਵਿੱਥ
){
  unixFile *pFile = (unixFile*)id;
  ਪੂਰਨ wrote = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( id );
  ਪ੍ਰਤਿਜ੍ਞਾ( amt>0 );

  /* If this is a database file (not a journal, super-journal or temp
  ** file), the bytes in the locking range should never be read or written. */
#ਜੇ 0
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->p_Preallocated_ਅਣਵਰਤਿਆ==0
       || ਵਿੱਥ>=PENDING_BYTE+512
       || ਵਿੱਥ+amt<=PENDING_BYTE
  );
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* If we are doing a normal write to a database file (as opposed to
  ** doing a hot-journal rollback or a write to some file other than a
  ** normal database file) then record the fact that the database
  ** has changed.  If the transaction counter is modified, record that
  ** fact too.
  */
  ਜੇ( pFile->ਵਿੱਚ_ਸਾਧਾਰਨ_Write ){
    pFile->dbUpdate = 1;  /* The database has been modified */
    ਜੇ( ਵਿੱਥ<=24 && ਵਿੱਥ+amt>=27 ){
      ਪੂਰਨ rc;
      ਅੱਖਰ oldCntr[4];
      SimulateIOErrorBenign(1);
      rc = seekAndRead(pFile, 24, oldCntr, 4);
      SimulateIOErrorBenign(0);
      ਜੇ( rc!=4 || memcmp(oldCntr, &((ਅੱਖਰ*)pBuf)[24-ਵਿੱਥ], 4)!=0 ){
        pFile->transCntrChng = 1;  /* The transaction counter has changed */
      }
    }
  }
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_ਮਾਪ>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  ਜੇ( ਵਿੱਥ<pFile->mmap_ਮਾਪ ){
    ਜੇ( ਵਿੱਥ+amt <= pFile->mmap_ਮਾਪ ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&((u8 *)(pFile->pMapRegion))[ਵਿੱਥ], pBuf, amt);
      ਵਾਪਸ SQLITE_OK;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ nCopy = pFile->mmap_ਮਾਪ - ਵਿੱਥ;
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&((u8 *)(pFile->pMapRegion))[ਵਿੱਥ], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      ਵਿੱਥ += nCopy;
    }
  }
#ਜੇ_ਅੰਤ

  ਜਦੋਂ( (wrote = seekAndWrite(pFile, ਵਿੱਥ, pBuf, amt))<amt && wrote>0 ){
    amt -= wrote;
    ਵਿੱਥ += wrote;
    pBuf = &((ਅੱਖਰ*)pBuf)[wrote];
  }
  SimulateIOError(( wrote=(-1), amt=1 ));
  SimulateDiskfullError(( wrote=0, amt=1 ));

  ਜੇ( amt>wrote ){
    ਜੇ( wrote<0 && pFile->ਆਖਰੀ_Errno!=ENOSPC ){
      /* lastErrno set by seekAndWrite */
      ਵਾਪਸ SQLITE_IOERR_WRITE;
    }ਨਹੀਂਤਾਂ{
      store_ਆਖਰੀ_Errno(pFile, 0); /* not a system error */
      ਵਾਪਸ SQLITE_ਭਰਿਆ;
    }
  }

  ਵਾਪਸ SQLITE_OK;
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
** We do not trust systems to provide a working fdatasync().  Some do.
** Others do no.  To be safe, we will stick with the (slightly slower)
** fsync(). If you know that your system does support fdatasync() correctly,
** then simply compile with -Dfdatasync=fdatasync or -DHAVE_FDATASYNC
*/
#ਜੇ !defined(fdatasync) && !HAVE_FDATASYNC
# define fdatasync fsync
#ਜੇ_ਅੰਤ

/*
** Define HAVE_FULLFSYNC to 0 or 1 depending on whether or not
** the F_FULLFSYNC macro is defined.  F_FULLFSYNC is currently
** only available on Mac OS X.  But that could change.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ F_FULLFSYNC
# define HAVE_FULLFSYNC 1
#ਨਹੀਂਤਾਂ
# define HAVE_FULLFSYNC 0
#ਜੇ_ਅੰਤ


/*
** The fsync() system call does not work as advertised on many
** unix systems.  The following procedure is an attempt to make
** it work better.
**
** The SQLITE_NO_SYNC macro disables all fsync()s.  This is useful
** for testing when we want to run through the test suite quickly.
** You are strongly advised *not* to deploy with SQLITE_NO_SYNC
** enabled, however, since with SQLITE_NO_SYNC enabled, an OS crash
** or power failure will likely corrupt the database file.
**
** SQLite sets the dataOnly flag if the size of the file is unchanged.
** The idea behind dataOnly is that it should only write the file content
** to disk, not the inode.  We only set dataOnly if the file size is
** unchanged since the file size is part of the inode.  However,
** Ted Ts'o tells us that fdatasync() will also write the inode if the
** file size has changed.  The only real difference between fdatasync()
** and fsync(), Ted tells us, is that fdatasync() will not flush the
** inode if the mtime or owner or other inode attributes have changed.
** We only care about the file size, not the other file attributes, so
** as far as SQLite is concerned, an fdatasync() is always adequate.
** So, we always use fdatasync() if it is available, regardless of
** the value of the dataOnly flag.
*/
ਸਥਿਰ ਪੂਰਨ ਭਰਿਆ_fsync(ਪੂਰਨ fd, ਪੂਰਨ ਭਰਿਆ_Sync, ਪੂਰਨ ਅੰਕੜੇ_Only){
  ਪੂਰਨ rc;

  /* The following "ifdef/elif/else/" block has the same structure as
  ** the one below. It is replicated here solely to avoid cluttering
  ** up the real code with the UNUSED_PARAMETER() macros.
  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_NO_SYNC
  ਅਣਵਰਤਿਆ_PARAMETER(fd);
  ਅਣਵਰਤਿਆ_PARAMETER(ਭਰਿਆ_Sync);
  ਅਣਵਰਤਿਆ_PARAMETER(ਅੰਕੜੇ_Only);
#ਜਾਂਜੇ HAVE_FULLFSYNC
  ਅਣਵਰਤਿਆ_PARAMETER(ਅੰਕੜੇ_Only);
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(ਭਰਿਆ_Sync);
  ਅਣਵਰਤਿਆ_PARAMETER(ਅੰਕੜੇ_Only);
#ਜੇ_ਅੰਤ

  /* Record the number of times that we do a normal fsync() and
  ** FULLSYNC.  This is used during testing to verify that this procedure
  ** gets called with the correct arguments.
  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  ਜੇ( ਭਰਿਆ_Sync ) sqlite3_fullsync_ਗਿਣਤੀ++;
  sqlite3_sync_ਗਿਣਤੀ++;
#ਜੇ_ਅੰਤ

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op.  But go ahead and call fstat() to validate the file
  ** descriptor as we need a method to provoke a failure during
  ** coverage testing.
  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_NO_SYNC
  {
    ਬਣਤਰ stat buf;
    rc = osFstat(fd, &buf);
  }
#ਜਾਂਜੇ HAVE_FULLFSYNC
  ਜੇ( ਭਰਿਆ_Sync ){
    rc = osFcntl(fd, F_FULLFSYNC, 0);
  }ਨਹੀਂਤਾਂ{
    rc = 1;
  }
  /* If the FULLFSYNC failed, fall back to attempting an fsync().
  ** It shouldn't be possible for fullfsync to fail on the local
  ** file system (on OSX), so failure indicates that FULLFSYNC
  ** isn't supported for this file system. So, attempt an fsync
  ** and (for now) ignore the overhead of a superfluous fcntl call.
  ** It'd be better to detect fullfsync support once and avoid
  ** the fcntl call every time sync is called.
  */
  ਜੇ( rc ) rc = fsync(fd);

#ਜਾਂਜੇ defined(__APPLE__)
  /* fdatasync() on HFS+ doesn't yet flush the file size if it changed correctly
  ** so currently we default to the macro that redefines fdatasync to fsync
  */
  rc = fsync(fd);
#ਨਹੀਂਤਾਂ
  rc = fdatasync(fd);
#ਜੇ OS_VXWORKS
  ਜੇ( rc==-1 && errno==ENOTSUP ){
    rc = fsync(fd);
  }
#ਜੇ_ਅੰਤ /* OS_VXWORKS */
#ਜੇ_ਅੰਤ /* ifdef SQLITE_NO_SYNC elif HAVE_FULLFSYNC */

  ਜੇ( OS_VXWORKS && rc!= -1 ){
    rc = 0;
  }
  ਵਾਪਸ rc;
}

/*
** Open a file descriptor to the directory containing file zFilename.
** If successful, *pFd is set to the opened file descriptor and
** SQLITE_OK is returned. If an error occurs, either SQLITE_NOMEM
** or SQLITE_CANTOPEN is returned and *pFd is set to an undefined
** value.
**
** The directory file descriptor is used for only one thing - to
** fsync() a directory to make sure file creation and deletion events
** are flushed to disk.  Such fsyncs are not needed on newer
** journaling filesystems, but are required on older filesystems.
**
** This routine can be overridden using the xSetSysCall interface.
** The ability to override this routine was added in support of the
** chromium sandbox.  Opening a directory is a security risk (we are
** told) so making it overrideable allows the chromium sandbox to
** replace this routine with a harmless no-op.  To make this routine
** a no-op, replace it with a stub that returns SQLITE_OK but leaves
** *pFd set to a negative number.
**
** If SQLITE_OK is returned, the caller is responsible for closing
** the file descriptor *pFd using close().
*/
ਸਥਿਰ ਪੂਰਨ openDirectory(ਅਚਲ ਅੱਖਰ *zFilename, ਪੂਰਨ *pFd){
  ਪੂਰਨ ii;
  ਪੂਰਨ fd = -1;
  ਅੱਖਰ zDirname[MAX_PATHNAME+1];

  sqlite3_snprintf(MAX_PATHNAME, zDirname, "%s", zFilename);
  ਲਈ(ii=(ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zDirname); ii>0 && zDirname[ii]!='/'; ii--);
  ਜੇ( ii>0 ){
    zDirname[ii] = '\0';
  }ਨਹੀਂਤਾਂ{
    ਜੇ( zDirname[0]!='/' ) zDirname[0] = '.';
    zDirname[1] = 0;
  }
  fd = robust_open(zDirname, O_RDONLY|O_ਦੁਆਧਾਰੀ, 0);
  ਜੇ( fd>=0 ){
    OSTRACE(("OPENDIR %-3d %s\n", fd, zDirname));
  }
  *pFd = fd;
  ਜੇ( fd>=0 ) ਵਾਪਸ SQLITE_OK;
  ਵਾਪਸ unixLogError(SQLITE_CANTOPEN_BKPT, "openDirectory", zDirname);
}

/*
** Make sure all writes to a particular file are committed to disk.
**
** If dataOnly==0 then both the file itself and its metadata (file
** size, access time, etc) are synced.  If dataOnly!=0 then only the
** file data is synced.
**
** Under Unix, also make sure that the directory entry for the file
** has been created by fsync-ing the directory that contains the file.
** If we do not do this and we encounter a power failure, the directory
** entry for the journal might not exist after we reboot.  The next
** SQLite to access the file will not know that the journal exists (because
** the directory entry for the journal was never created) and the transaction
** will not roll back - possibly leading to database corruption.
*/
ਸਥਿਰ ਪੂਰਨ unixSync(sqlite3_file *id, ਪੂਰਨ flags){
  ਪੂਰਨ rc;
  unixFile *pFile = (unixFile*)id;

  ਪੂਰਨ is_ਅੰਕੜੇ_Only = (flags&SQLITE_SYNC_DATAONLY);
  ਪੂਰਨ isFullsync = (flags&0x0F)==SQLITE_SYNC_ਭਰਿਆ;

  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  ਪ੍ਰਤਿਜ੍ਞਾ((flags&0x0F)==SQLITE_SYNC_ਸਾਧਾਰਨ
      || (flags&0x0F)==SQLITE_SYNC_ਭਰਿਆ
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( ਵਾਪਸ SQLITE_ਭਰਿਆ );

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  OSTRACE(("SYNC    %-3d\n", pFile->h));
  rc = ਭਰਿਆ_fsync(pFile->h, isFullsync, is_ਅੰਕੜੇ_Only);
  SimulateIOError( rc=1 );
  ਜੇ( rc ){
    store_ਆਖਰੀ_Errno(pFile, errno);
    ਵਾਪਸ unixLogError(SQLITE_IOERR_FSYNC, "full_fsync", pFile->z_ਰਸਤਾ);
  }

  /* Also fsync the directory containing the file if the DIRSYNC flag
  ** is set.  This is a one-time occurrence.  Many systems (examples: AIX)
  ** are unable to fsync a directory, so ignore errors on the fsync.
  */
  ਜੇ( pFile->ctrlFlags & UNIXFILE_DIRSYNC ){
    ਪੂਰਨ dirfd;
    OSTRACE(("DIRSYNC %s (have_fullfsync=%d fullsync=%d)\n", pFile->z_ਰਸਤਾ,
            HAVE_FULLFSYNC, isFullsync));
    rc = osOpenDirectory(pFile->z_ਰਸਤਾ, &dirfd);
    ਜੇ( rc==SQLITE_OK ){
      ਭਰਿਆ_fsync(dirfd, 0, 0);
      robust_close(pFile, dirfd, __LINE__);
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_CANTOPEN );
      rc = SQLITE_OK;
    }
    pFile->ctrlFlags &= ~UNIXFILE_DIRSYNC;
  }
  ਵਾਪਸ rc;
}

/*
** Truncate an open file to a specified size
*/
ਸਥਿਰ ਪੂਰਨ unixTruncate(sqlite3_file *id, i64 nByte){
  unixFile *pFile = (unixFile *)id;
  ਪੂਰਨ rc;
  ਪ੍ਰਤਿਜ੍ਞਾ( pFile );
  SimulateIOError( ਵਾਪਸ SQLITE_IOERR_TRUNCATE );

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  ਜੇ( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

  rc = robust_ftruncate(pFile->h, nByte);
  ਜੇ( rc ){
    store_ਆਖਰੀ_Errno(pFile, errno);
    ਵਾਪਸ unixLogError(SQLITE_IOERR_TRUNCATE, "ftruncate", pFile->z_ਰਸਤਾ);
  }ਨਹੀਂਤਾਂ{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    /* If we are doing a normal write to a database file (as opposed to
    ** doing a hot-journal rollback or a write to some file other than a
    ** normal database file) and we truncate the file to zero length,
    ** that effectively updates the change counter.  This might happen
    ** when restoring a database using the backup API from a zero-length
    ** source.
    */
    ਜੇ( pFile->ਵਿੱਚ_ਸਾਧਾਰਨ_Write && nByte==0 ){
      pFile->transCntrChng = 1;
    }
#ਜੇ_ਅੰਤ

#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
    /* If the file was just truncated to a size smaller than the currently
    ** mapped region, reduce the effective mapping size as well. SQLite will
    ** use read() and write() to access data beyond this point from now on.
    */
    ਜੇ( nByte<pFile->mmap_ਮਾਪ ){
      pFile->mmap_ਮਾਪ = nByte;
    }
#ਜੇ_ਅੰਤ

    ਵਾਪਸ SQLITE_OK;
  }
}

/*
** Determine the current size of a file in bytes
*/
ਸਥਿਰ ਪੂਰਨ unix_File_ਮਾਪ(sqlite3_file *id, i64 *p_ਮਾਪ){
  ਪੂਰਨ rc;
  ਬਣਤਰ stat buf;
  ਪ੍ਰਤਿਜ੍ਞਾ( id );
  rc = osFstat(((unixFile*)id)->h, &buf);
  SimulateIOError( rc=1 );
  ਜੇ( rc!=0 ){
    store_ਆਖਰੀ_Errno((unixFile*)id, errno);
    ਵਾਪਸ SQLITE_IOERR_FSTAT;
  }
  *p_ਮਾਪ = buf.st_ਮਾਪ;

  /* When opening a zero-size database, the findInodeInfo() procedure
  ** writes a single byte into that file in order to work around a bug
  ** in the OS-X msdos filesystem.  In order to avoid problems with upper
  ** layers, we need to report this file size as zero even though it is
  ** really 1.   Ticket #3260.
  */
  ਜੇ( *p_ਮਾਪ==1 ) *p_ਮਾਪ = 0;


  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
/*
** Handler for proxy-locking file-control verbs.  Defined below in the
** proxying locking division.
*/
ਸਥਿਰ ਪੂਰਨ proxyFileControl(sqlite3_file*,ਪੂਰਨ,ਖਾਲੀ*);
#ਜੇ_ਅੰਤ

/*
** This function is called to handle the SQLITE_FCNTL_SIZE_HINT
** file-control operation.  Enlarge the database to nBytes in size
** (rounded up to the next chunk-size).  If the database is already
** nBytes or larger, this routine is a no-op.
*/
ਸਥਿਰ ਪੂਰਨ fcntl_ਮਾਪ_Hint(unixFile *pFile, i64 nByte){
  ਜੇ( pFile->szChunk>0 ){
    i64 n_ਮਾਪ;                    /* Required file size */
    ਬਣਤਰ stat buf;              /* Used to hold return values of fstat() */
 
    ਜੇ( osFstat(pFile->h, &buf) ){
      ਵਾਪਸ SQLITE_IOERR_FSTAT;
    }

    n_ਮਾਪ = ((nByte+pFile->szChunk-1) / pFile->szChunk) * pFile->szChunk;
    ਜੇ( n_ਮਾਪ>(i64)buf.st_ਮਾਪ ){

#ਜੇ defined(HAVE_POSIX_FALLOCATE) && HAVE_POSIX_FALLOCATE
      /* The code below is handling the return value of osFallocate()
      ** correctly. posix_fallocate() is defined to "returns zero on success,
      ** or an error number on  failure". See the manpage for details. */
      ਪੂਰਨ err;
      ਕਰੋ{
        err = osFallocate(pFile->h, buf.st_ਮਾਪ, n_ਮਾਪ-buf.st_ਮਾਪ);
      }ਜਦੋਂ( err==EINTR );
      ਜੇ( err && err!=EINVAL ) ਵਾਪਸ SQLITE_IOERR_WRITE;
#ਨਹੀਂਤਾਂ
      /* If the OS does not have posix_fallocate(), fake it. Write a
      ** single byte to the last byte in each block that falls entirely
      ** within the extended region. Then, if required, a single byte
      ** at offset (nSize-1), to set the size of the file correctly.
      ** This is a similar technique to that used by glibc on systems
      ** that do not have a real fallocate() call.
      */
      ਪੂਰਨ nBlk = buf.st_blksize;  /* File-system block size */
      ਪੂਰਨ nWrite = 0;             /* Number of bytes written by seekAndWrite */
      i64 iWrite;                 /* Next offset to write to */

      iWrite = (buf.st_ਮਾਪ/nBlk)*nBlk + nBlk - 1;
      ਪ੍ਰਤਿਜ੍ਞਾ( iWrite>=buf.st_ਮਾਪ );
      ਪ੍ਰਤਿਜ੍ਞਾ( ((iWrite+1)%nBlk)==0 );
      ਲਈ(/*no-op*/; iWrite<n_ਮਾਪ+nBlk-1; iWrite+=nBlk ){
        ਜੇ( iWrite>=n_ਮਾਪ ) iWrite = n_ਮਾਪ - 1;
        nWrite = seekAndWrite(pFile, iWrite, "", 1);
        ਜੇ( nWrite!=1 ) ਵਾਪਸ SQLITE_IOERR_WRITE;
      }
#ਜੇ_ਅੰਤ
    }
  }

#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  ਜੇ( pFile->mmap_ਮਾਪ_Max>0 && nByte>pFile->mmap_ਮਾਪ ){
    ਪੂਰਨ rc;
    ਜੇ( pFile->szChunk<=0 ){
      ਜੇ( robust_ftruncate(pFile->h, nByte) ){
        store_ਆਖਰੀ_Errno(pFile, errno);
        ਵਾਪਸ unixLogError(SQLITE_IOERR_TRUNCATE, "ftruncate", pFile->z_ਰਸਤਾ);
      }
    }

    rc = unixMapfile(pFile, nByte);
    ਵਾਪਸ rc;
  }
#ਜੇ_ਅੰਤ

  ਵਾਪਸ SQLITE_OK;
}

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
ਸਥਿਰ ਖਾਲੀ unix_ਬਹੁਲਕ_ਦੋਆਧਾਰੀ_ਅੰਕ(unixFile *pFile, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਨਕਾਬ, ਪੂਰਨ *pArg){
  ਜੇ( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & ਨਕਾਬ)!=0;
  }ਨਹੀਂਤਾਂ ਜੇ( (*pArg)==0 ){
    pFile->ctrlFlags &= ~ਨਕਾਬ;
  }ਨਹੀਂਤਾਂ{
    pFile->ctrlFlags |= ਨਕਾਬ;
  }
}

/* Forward declaration */
ਸਥਿਰ ਪੂਰਨ unixGetTempname(ਪੂਰਨ nBuf, ਅੱਖਰ *zBuf);
#ਜੇ !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
 ਸਥਿਰ ਪੂਰਨ unixFcntlExternalReader(unixFile*, ਪੂਰਨ*);
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
 ਸਥਿਰ ਖਾਲੀ unixDescribeShm(sqlite3_str*,unixShm*);
#ਜੇ_ਅੰਤ


/*
** Information and control of an open file handle.
*/
ਸਥਿਰ ਪੂਰਨ unixFileControl(sqlite3_file *id, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  unixFile *pFile = (unixFile*)id;
  ਚੋਣ( op ){
#ਜੇ defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
    ਹਾਲਤ SQLITE_FCNTL_BEGIN_ATOMIC_WRITE: {
      ਪੂਰਨ rc = osIoctl(pFile->h, F2_FS_IOC_ਸ਼ੁਰੂ_ATOMIC_WRITE);
      ਵਾਪਸ rc ? SQLITE_IOERR_BEGIN_ATOMIC : SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_COMMIT_ATOMIC_WRITE: {
      ਪੂਰਨ rc = osIoctl(pFile->h, F2FS_IOC_COMMIT_ATOMIC_WRITE);
      ਵਾਪਸ rc ? SQLITE_IOERR_COMMIT_ATOMIC : SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_ROLLBACK_ATOMIC_WRITE: {
      ਪੂਰਨ rc = osIoctl(pFile->h, F2FS_IOC_ABORT_VOLATILE_WRITE);
      ਵਾਪਸ rc ? SQLITE_IOERR_ROLLBACK_ATOMIC : SQLITE_OK;
    }
#ਜੇ_ਅੰਤ /* __linux__ && SQLITE_ENABLE_BATCH_ATOMIC_WRITE */

    ਹਾਲਤ SQLITE_FCNTL_NULL_IO: {
      osClose(pFile->h);
      pFile->h = -1;
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_LOCKSTATE: {
      *(ਪੂਰਨ*)pArg = pFile->e_File_ਤਾਲਾ;
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_ਆਖਰੀ_ERRNO: {
      *(ਪੂਰਨ*)pArg = pFile->ਆਖਰੀ_Errno;
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_CHUNK_ਮਾਪ: {
      pFile->szChunk = *(ਪੂਰਨ *)pArg;
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_ਮਾਪ_HINT: {
      ਪੂਰਨ rc;
      SimulateIOErrorBenign(1);
      rc = fcntl_ਮਾਪ_Hint(pFile, *(i64 *)pArg);
      SimulateIOErrorBenign(0);
      ਵਾਪਸ rc;
    }
    ਹਾਲਤ SQLITE_FCNTL_PERSIST_WAL: {
      unix_ਬਹੁਲਕ_ਦੋਆਧਾਰੀ_ਅੰਕ(pFile, UNIXFILE_PERSIST_WAL, (ਪੂਰਨ*)pArg);
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      unix_ਬਹੁਲਕ_ਦੋਆਧਾਰੀ_ਅੰਕ(pFile, UNIXFILE_PSOW, (ਪੂਰਨ*)pArg);
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_VFSNAME: {
      *(ਅੱਖਰ**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_TEMPFILENAME: {
      ਅੱਖਰ *zTFile = sqlite3_malloc64( pFile->pVfs->mxPathname );
      ਜੇ( zTFile ){
        unixGetTempname(pFile->pVfs->mxPathname, zTFile);
        *(ਅੱਖਰ**)pArg = zTFile;
      }
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_HAS_MOVED: {
      *(ਪੂਰਨ*)pArg = fileHasMoved(pFile);
      ਵਾਪਸ SQLITE_OK;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
    ਹਾਲਤ SQLITE_FCNTL_ਤਾਲਾ_TIMEOUT: {
      ਪੂਰਨ iOld = pFile->iBusyTimeout;
      ਪੂਰਨ iNew = *(ਪੂਰਨ*)pArg;
#ਜੇ SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = iNew<0 ? 0x7FFFFFFF : (ਬਿਨਾਨਿਸ਼ਾਨੀ)iNew;
#ਜਾਂਜੇ SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = !!(*(ਪੂਰਨ*)pArg);
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
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
    ਹਾਲਤ SQLITE_FCNTL_MMAP_ਮਾਪ: {
      i64 new_ਹੱਦਬੰਦੀ = *(i64*)pArg;
      ਪੂਰਨ rc = SQLITE_OK;
      ਜੇ( new_ਹੱਦਬੰਦੀ>sqlite3GlobalConfig.mxMmap ){
        new_ਹੱਦਬੰਦੀ = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (size_t) and passed
      ** to mmap(). Restrict its value to 2GB if (size_t) is not at least a
      ** 64-bit type. */
      ਜੇ( new_ਹੱਦਬੰਦੀ>0 && ਆਕਾਰ(size_t)<8 ){
        new_ਹੱਦਬੰਦੀ = (new_ਹੱਦਬੰਦੀ & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmap_ਮਾਪ_Max;
      ਜੇ( new_ਹੱਦਬੰਦੀ>=0 && new_ਹੱਦਬੰਦੀ!=pFile->mmap_ਮਾਪ_Max && pFile->n_Fetch_ਬਾਹਰਲਾ==0 ){
        pFile->mmap_ਮਾਪ_Max = new_ਹੱਦਬੰਦੀ;
        ਜੇ( pFile->mmap_ਮਾਪ>0 ){
          unixUnmapfile(pFile);
          rc = unixMapfile(pFile, -1);
        }
      }
      ਵਾਪਸ rc;
    }
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    /* The pager calls this method to signal that it has done
    ** a rollback and that the database is therefore unchanged and
    ** it hence it is OK for the transaction change counter to be
    ** unchanged.
    */
    ਹਾਲਤ SQLITE_FCNTL_DB_UNCHANGED: {
      ((unixFile*)id)->dbUpdate = 0;
      ਵਾਪਸ SQLITE_OK;
    }
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    ਹਾਲਤ SQLITE_FCNTL_ਸੈੱਟ_LOCKPROXYFILE:
    ਹਾਲਤ SQLITE_FCNTL_GET_LOCKPROXYFILE: {
      ਵਾਪਸ proxyFileControl(id,op,pArg);
    }
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__) */

    ਹਾਲਤ SQLITE_FCNTL_EXTERNAL_READER: {
#ਜੇ !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
      ਵਾਪਸ unixFcntlExternalReader((unixFile*)id, (ਪੂਰਨ*)pArg);
#ਨਹੀਂਤਾਂ
      *(ਪੂਰਨ*)pArg = 0;
      ਵਾਪਸ SQLITE_OK;
#ਜੇ_ਅੰਤ
    }

#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    ਹਾਲਤ SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      ਅੱਖਰ aLck[16];
      unixInodeInfo *pInode;
      ਸਥਿਰ ਅਚਲ ਅੱਖਰ *az_ਤਾਲਾ[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
      sqlite3_str_appendf(pStr, "{\"h\":%d", pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      ਜੇ( pFile->e_File_ਤਾਲਾ ){
        sqlite3_str_appendf(pStr, ",\"eFileLock\":\"%s\"", 
                                  az_ਤਾਲਾ[pFile->e_File_ਤਾਲਾ-1]);
        ਜੇ( unixPosixAdvisoryLocks(pFile->h, aLck)==SQLITE_OK ){
          sqlite3_str_appendf(pStr, ",\"pal\":\"%s\"", aLck);
        }
      }
      unixEnterMutex();
      ਜੇ( pFile->pShm ){
        sqlite3_str_appendall(pStr, ",\"shm\":");
        unixDescribeShm(pStr, pFile->pShm);
      }
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
      ਜੇ( pFile->mmap_ਮਾਪ ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmap_ਮਾਪ);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->n_Fetch_ਬਾਹਰਲਾ);
      }
#ਜੇ_ਅੰਤ
      ਜੇ( (pInode = pFile->pInode)!=0 ){
        sqlite3_str_appendf(pStr, ",\"inode\":{\"nRef\":%d",pInode->nRef);
        sqlite3_mutex_enter(pInode->p_ਤਾਲਾ_Mutex);
        sqlite3_str_appendf(pStr, ",\"nShared\":%d", pInode->nShared);
        ਜੇ( pInode->e_File_ਤਾਲਾ ){
          sqlite3_str_appendf(pStr, ",\"eFileLock\":\"%s\"", 
                                  az_ਤਾਲਾ[pInode->e_File_ਤਾਲਾ-1]);
        }
        ਜੇ( pInode->p_ਅਣਵਰਤਿਆ ){
          ਅੱਖਰ cSep = '[';
          Unix_ਅਣਵਰਤਿਆ_Fd *pUFd = pFile->pInode->p_ਅਣਵਰਤਿਆ;
          sqlite3_str_appendall(pStr, ",\"unusedFd\":");
          ਜਦੋਂ( pUFd ){
            sqlite3_str_appendf(pStr, "%c{\"fd\":%d,\"flags\":%d",
                                cSep, pUFd->fd, pUFd->flags);
            cSep = ',';
            ਜੇ( unixPosixAdvisoryLocks(pUFd->fd, aLck)==SQLITE_OK ){
              sqlite3_str_appendf(pStr, ",\"pal\":\"%s\"", aLck);
            }
            sqlite3_str_append(pStr, "}", 1);
            pUFd = pUFd->p_ਅਗਲਾ;
          }
          sqlite3_str_append(pStr, "]", 1);
        }
        sqlite3_mutex_leave(pInode->p_ਤਾਲਾ_Mutex);
        sqlite3_str_append(pStr, "}", 1);
      }
      unixLeaveMutex();
      sqlite3_str_append(pStr, "}", 1);
      ਵਾਪਸ SQLITE_OK;
    }
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */
  }
  ਵਾਪਸ SQLITE_NOTFOUND;
}

/*
** If pFd->sectorSize is non-zero when this function is called, it is a
** no-op. Otherwise, the values of pFd->sectorSize and
** pFd->deviceCharacteristics are set according to the file-system
** characteristics.
**
** There are two versions of this function. One for QNX and one for all
** other systems.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __QNXNTO__
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_Device_Characteristics(unixFile *pFd){
  ਪ੍ਰਤਿਜ੍ਞਾ( pFd->deviceCharacteristics==0 || pFd->sector_ਮਾਪ!=0 );
  ਜੇ( pFd->sector_ਮਾਪ==0 ){
#ਜੇ defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
    ਪੂਰਨ res;
    u32 f = 0;

    /* Check for support for F2FS atomic batch writes. */
    res = osIoctl(pFd->h, F2FS_IOC_GET_FEATURES, &f);
    ਜੇ( res==0 && (f & F2FS_FEATURE_ATOMIC_WRITE) ){
      pFd->deviceCharacteristics = SQLITE_IOCAP_BATCH_ATOMIC;
    }
#ਜੇ_ਅੰਤ /* __linux__ && SQLITE_ENABLE_BATCH_ATOMIC_WRITE */

    /* Set the POWERSAFE_OVERWRITE flag if requested. */
    ਜੇ( pFd->ctrlFlags & UNIXFILE_PSOW ){
      pFd->deviceCharacteristics |= SQLITE_IOCAP_POWERSAFE_OVERWRITE;
    }
    pFd->deviceCharacteristics |= SQLITE_IOCAP_SUBPAGE_READ;

    pFd->sector_ਮਾਪ = SQLITE_DEFAULT_SECTOR_ਮਾਪ;
  }
}
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/dcmd_blk.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/statvfs.h>
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_Device_Characteristics(unixFile *pFile){
  ਜੇ( pFile->sector_ਮਾਪ == 0 ){
    ਬਣਤਰ statvfs fsInfo;

    /* Set defaults for non-supported filesystems */
    pFile->sector_ਮਾਪ = SQLITE_DEFAULT_SECTOR_ਮਾਪ;
    pFile->deviceCharacteristics = 0;
    ਜੇ( fstatvfs(pFile->h, &fsInfo) == -1 ) {
      ਵਾਪਸ;
    }

    ਜੇ( !strcmp(fsInfo.f_basetype, "tmp") ) {
      pFile->sector_ਮਾਪ = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC4K |       /* All ram filesystem writes are atomic */
        SQLITE_IOCAP_ਸੁਰੱਖਿਅਤ_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }ਨਹੀਂਤਾਂ ਜੇ( ਲੜੀ_ਖੋਜ(fsInfo.f_basetype, "etfs") ){
      pFile->sector_ਮਾਪ = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* etfs cluster size writes are atomic */
        (pFile->sector_ਮਾਪ / 512 * SQLITE_IOCAP_ATOMIC512) |
        SQLITE_IOCAP_ਸੁਰੱਖਿਅਤ_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }ਨਹੀਂਤਾਂ ਜੇ( !strcmp(fsInfo.f_basetype, "qnx6") ){
      pFile->sector_ਮਾਪ = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC |         /* All filesystem writes are atomic */
        SQLITE_IOCAP_ਸੁਰੱਖਿਅਤ_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }ਨਹੀਂਤਾਂ ਜੇ( !strcmp(fsInfo.f_basetype, "qnx4") ){
      pFile->sector_ਮਾਪ = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* full bitset of atomics from max sector size and smaller */
        (((pFile->sector_ਮਾਪ / 512 * SQLITE_IOCAP_ATOMIC512) << 1) - 2) |
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }ਨਹੀਂਤਾਂ ਜੇ( ਲੜੀ_ਖੋਜ(fsInfo.f_basetype, "dos") ){
      pFile->sector_ਮਾਪ = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* full bitset of atomics from max sector size and smaller */
        (((pFile->sector_ਮਾਪ / 512 * SQLITE_IOCAP_ATOMIC512) << 1) - 2) |
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }ਨਹੀਂਤਾਂ{
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC512 |      /* blocks are atomic */
        SQLITE_IOCAP_ਸੁਰੱਖਿਅਤ_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        0;
    }
  }
  /* Last chance verification.  If the sector size isn't a multiple of 512
  ** then it isn't valid.*/
  ਜੇ( pFile->sector_ਮਾਪ % 512 != 0 ){
    pFile->deviceCharacteristics = 0;
    pFile->sector_ਮਾਪ = SQLITE_DEFAULT_SECTOR_ਮਾਪ;
  }
}
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਪੂਰਨ unix_Sector_ਮਾਪ(sqlite3_file *id){
  unixFile *pFd = (unixFile*)id;
  ਸੈੱਟ_Device_Characteristics(pFd);
  ਵਾਪਸ pFd->sector_ਮਾਪ;
}

/*
** Return the device characteristics for the file.
**
** This VFS is set up to return SQLITE_IOCAP_POWERSAFE_OVERWRITE by default.
** However, that choice is controversial since technically the underlying
** file system does not always provide powersafe overwrites.  (In other
** words, after a power-loss event, parts of the file that were never
** written might end up being altered.)  However, non-PSOW behavior is very,
** very rare.  And asserting PSOW makes a large reduction in the amount
** of required I/O for journaling, since a lot of padding is eliminated.
**  Hence, while POWERSAFE_OVERWRITE is on by default, there is a file-control
** available to turn it off and URI query parameter available to turn it off.
*/
ਸਥਿਰ ਪੂਰਨ unixDeviceCharacteristics(sqlite3_file *id){
  unixFile *pFd = (unixFile*)id;
  ਸੈੱਟ_Device_Characteristics(pFd);
  ਵਾਪਸ pFd->deviceCharacteristics;
}

#ਜੇ !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_ਮਾਪ>0

/*
** Return the system page size.
**
** This function should not be called directly by other code in this file.
** Instead, it should be called via macro osGetpagesize().
*/
ਸਥਿਰ ਪੂਰਨ unixGetpagesize(ਖਾਲੀ){
#ਜੇ OS_VXWORKS
  ਵਾਪਸ 1024;
#ਜਾਂਜੇ defined(BSD_ਸਰੋਤ)
  ਵਾਪਸ getpagesize();
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ (ਪੂਰਨ)sysconf(_SC_PAGESIZE);
#ਜੇ_ਅੰਤ
}

#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0 */

#ਜੇ !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)

/*
** Object used to represent an shared memory buffer.
**
** When multiple threads all reference the same wal-index, each thread
** has its own unixShm object, but they all point to a single instance
** of this unixShmNode object.  In other words, each wal-index is opened
** only once per process.
**
** Each unixShmNode object is connected to a single unixInodeInfo object.
** We could coalesce this object into unixInodeInfo, but that would mean
** every open file that does not use shared memory (in other words, most
** open files) would have to carry around this extra information.  So
** the unixInodeInfo object contains a pointer to this unixShmNode object
** and the unixShmNode object is created only when needed.
**
** unixMutexHeld() must be true when creating or destroying
** this object or while reading or writing the following fields:
**
**      nRef
**
** The following fields are read-only after the object is created:
**
**      hShm
**      zFilename
**
** Either unixShmNode.pShmMutex must be held or unixShmNode.nRef==0 and
** unixMutexHeld() is true when reading or writing any other field
** in this structure.
**
** aLock[SQLITE_SHM_NLOCK]:
**   This array records the various locks held by clients on each of the
**   SQLITE_SHM_NLOCK slots. If the aLock[] entry is set to 0, then no
**   locks are held by the process on this slot. If it is set to -1, then
**   some client holds an EXCLUSIVE lock on the locking slot. If the aLock[]
**   value is set to a positive value, then it is the number of shared 
**   locks currently held on the slot.
**
** aMutex[SQLITE_SHM_NLOCK]:
**   Normally, when SQLITE_ENABLE_SETLK_TIMEOUT is not defined, mutex 
**   pShmMutex is used to protect the aLock[] array and the right to
**   call fcntl() on unixShmNode.hShm to obtain or release locks.
**
**   If SQLITE_ENABLE_SETLK_TIMEOUT is defined though, we use an array
**   of mutexes - one for each locking slot. To read or write locking
**   slot aLock[iSlot], the caller must hold the corresponding mutex
**   aMutex[iSlot]. Similarly, to call fcntl() to obtain or release a
**   lock corresponding to slot iSlot, mutex aMutex[iSlot] must be held.
*/
ਬਣਤਰ unix_Shm_ਗੰਢ {
  unixInodeInfo *pInode;     /* unixInodeInfo that owns this SHM node */
  sqlite3_mutex *pShmMutex;  /* Mutex to access this object */
  ਅੱਖਰ *zFilename;           /* Name of the mmapped file */
  ਪੂਰਨ hShm;                  /* Open file descriptor */
  ਪੂਰਨ szRegion;              /* Size of shared-memory regions */
  u16 nRegion;               /* Size of array apRegion */
  u8 isReadonly;             /* True if read-only */
  u8 isUnlocked;             /* True if no DMS lock held */
  ਅੱਖਰ **apRegion;           /* Array of mapped shared-memory regions */
  ਪੂਰਨ nRef;                  /* Number of unixShm objects pointing to this */
  unixShm *p_ਪਹਿਲਾ;           /* All unixShm objects pointing to this */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
  sqlite3_mutex *aMutex[SQLITE_SHM_NLOCK];
#ਜੇ_ਅੰਤ
  ਪੂਰਨ a_ਤਾਲਾ[SQLITE_SHM_NLOCK];  /* # shared locks on slot, -1==excl lock */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  u8 ਅਗਲਾ_Shm_Id;              /* Next available unixShm.id value */
#ਜੇ_ਅੰਤ
};

/*
** Structure used internally by this VFS to record the state of an
** open shared memory connection.
**
** The following fields are initialized when this object is created and
** are read-only thereafter:
**
**    unixShm.pShmNode
**    unixShm.id
**
** All other fields are read/write.  The unixShm.pShmNode->pShmMutex must
** be held while accessing any read/write fields.
*/
ਬਣਤਰ unixShm {
  unix_Shm_ਗੰਢ *p_Shm_ਗੰਢ;     /* The underlying unixShmNode object */
  unixShm *p_ਅਗਲਾ;            /* Next unixShm with the same unixShmNode */
  u8 hasMutex;               /* True if holding the unixShmNode->pShmMutex */
  u8 id;                     /* Id of this connection within its unixShmNode */
  u16 shared_ਨਕਾਬ;            /* Mask of shared locks held */
  u16 excl_ਨਕਾਬ;              /* Mask of exclusive locks held */
};

/*
** Constants used for locking
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX_SHM_ਅਧਾਰ   ((22+SQLITE_SHM_NLOCK)*4)         /* first lock byte */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIX_SHM_DMS    (UNIX_SHM_ਅਧਾਰ+SQLITE_SHM_NLOCK)  /* deadman switch */

#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Describe the pShm object using JSON.  Used for diagnostics only.
*/
ਸਥਿਰ ਖਾਲੀ unixDescribeShm(sqlite3_str *pStr, unixShm *pShm){
  unix_Shm_ਗੰਢ *p_ਗੰਢ = pShm->p_Shm_ਗੰਢ;
  ਅੱਖਰ aLck[16];
  sqlite3_str_appendf(pStr, "{\"h\":%d", p_ਗੰਢ->hShm);
  ਪ੍ਰਤਿਜ੍ਞਾ( unixMutexHeld() );
  sqlite3_str_appendf(pStr, ",\"nRef\":%d", p_ਗੰਢ->nRef);
  sqlite3_str_appendf(pStr, ",\"id\":%d", pShm->id);
  sqlite3_str_appendf(pStr, ",\"sharedMask\":%d", pShm->shared_ਨਕਾਬ);
  sqlite3_str_appendf(pStr, ",\"exclMask\":%d", pShm->excl_ਨਕਾਬ);
  ਜੇ( unixPosixAdvisoryLocks(p_ਗੰਢ->hShm, aLck)==SQLITE_OK ){
    sqlite3_str_appendf(pStr, ",\"pal\":\"%s\"", aLck);
  }
  sqlite3_str_append(pStr, "}", 1);
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

/*
** Use F_GETLK to check whether or not there are any readers with open
** wal-mode transactions in other processes on database file pFile. If
** no error occurs, return SQLITE_OK and set (*piOut) to 1 if there are
** such transactions, or 0 otherwise. If an error occurs, return an
** SQLite error code. The final value of *piOut is undefined in this
** case.
*/
ਸਥਿਰ ਪੂਰਨ unixFcntlExternalReader(unixFile *pFile, ਪੂਰਨ *pi_ਬਾਹਰਲਾ){
  ਪੂਰਨ rc = SQLITE_OK;
  *pi_ਬਾਹਰਲਾ = 0;
  ਜੇ( pFile->pShm){
    unix_Shm_ਗੰਢ *p_Shm_ਗੰਢ = pFile->pShm->p_Shm_ਗੰਢ;
    ਬਣਤਰ flock f;

    memset(&f, 0, ਆਕਾਰ(f));
    f.l_type = F_WRLCK;
    f.l_whence = SEEK_SET;
    f.l_ਸ਼ੁਰੂ = UNIX_SHM_ਅਧਾਰ + 3;
    f.l_len = SQLITE_SHM_NLOCK - 3;

    sqlite3_mutex_enter(p_Shm_ਗੰਢ->pShmMutex);
    ਜੇ( osFcntl(p_Shm_ਗੰਢ->hShm, F_GETLK, &f)<0 ){
      rc = SQLITE_IOERR_ਤਾਲਾ;
    }ਨਹੀਂਤਾਂ{
      *pi_ਬਾਹਰਲਾ = (f.l_type!=F_UNLCK);
    }
    sqlite3_mutex_leave(p_Shm_ਗੰਢ->pShmMutex);
  }

  ਵਾਪਸ rc;
}

/*
** If pFile has a -shm file open and it is sharing that file with some
** other connection, either in the same process or in a separate process,
** then return true.  Return false if either pFile does not have a -shm
** file open or if it is the only connection to that -shm file across the
** entire system.
**
** This routine is not required for correct operation.  It can always return
** false and SQLite will continue to operate according to spec.  However,
** when this routine does its job, it adds extra robustness in cases
** where database file locks have been erroneously deleted in a WAL-mode
** database by doing close(open(DATABASE_PATHNAME)) or similar.
**
** With false negatives, SQLite still operates to spec, though with less
** robustness.  With false positives, the last database connection on a
** WAL-mode database will fail to unlink the -wal and -shm files, which
** is annoying but harmless.  False positives will also prevent a database
** connection from running "PRAGMA journal_mode=DELETE" in order to take
** the database out of WAL mode, which is perhaps more serious, but is
** still not a disaster.
*/
ਸਥਿਰ ਪੂਰਨ unix_Is_Sharing_Shm_ਗੰਢ(unixFile *pFile){
  unix_Shm_ਗੰਢ *p_Shm_ਗੰਢ;
  ਬਣਤਰ flock ਤਾਲਾ;
  ਜੇ( pFile->pShm==0 ) ਵਾਪਸ 0;
  ਜੇ( pFile->ctrlFlags & UNIXFILE_EXCL ) ਵਾਪਸ 0;
  p_Shm_ਗੰਢ = pFile->pShm->p_Shm_ਗੰਢ;
#ਜੇ SQLITE_ATOMIC_INTRINSICS
  ਪ੍ਰਤਿਜ੍ਞਾ( Atomic_ਲੱਦੋ(&p_Shm_ਗੰਢ->nRef)==1 );
#ਜੇ_ਅੰਤ
  memset(&ਤਾਲਾ, 0, ਆਕਾਰ(ਤਾਲਾ));
  ਤਾਲਾ.l_whence = SEEK_SET;
  ਤਾਲਾ.l_ਸ਼ੁਰੂ = UNIX_SHM_DMS;
  ਤਾਲਾ.l_len = 1;
  ਤਾਲਾ.l_type = F_WRLCK;
  osFcntl(p_Shm_ਗੰਢ->hShm, F_GETLK, &ਤਾਲਾ);
  ਵਾਪਸ (ਤਾਲਾ.l_type!=F_UNLCK);
}

/*
** Apply posix advisory locks for all bytes from ofst through ofst+n-1.
**
** Locks block if the mask is exactly UNIX_SHM_C and are non-blocking
** otherwise.
*/
ਸਥਿਰ ਪੂਰਨ unix_Shm_System_ਤਾਲਾ(
  unixFile *pFile,       /* Open connection to the WAL file */
  ਪੂਰਨ ਤਾਲਾ_Type,          /* F_UNLCK, F_RDLCK, or F_WRLCK */
  ਪੂਰਨ ofst,              /* First byte of the locking range */
  ਪੂਰਨ n                  /* Number of bytes to lock */
){
  unix_Shm_ਗੰਢ *p_Shm_ਗੰਢ; /* Apply locks to this open shared-memory segment */
  ਬਣਤਰ flock f;        /* The posix advisory locking structure */
  ਪੂਰਨ rc = SQLITE_OK;    /* Result code form fcntl() */

  p_Shm_ਗੰਢ = pFile->pInode->p_Shm_ਗੰਢ;

  /* Assert that the parameters are within expected range and that the
  ** correct mutex or mutexes are held. */
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ->nRef>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (ofst==UNIX_SHM_DMS && n==1)
       || (ofst>=UNIX_SHM_ਅਧਾਰ && ofst+n<=(UNIX_SHM_ਅਧਾਰ+SQLITE_SHM_NLOCK))
  );
  ਜੇ( ofst==UNIX_SHM_DMS ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ->nRef>0 || unixMutexHeld() );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ->nRef==0 || sqlite3_mutex_held(p_Shm_ਗੰਢ->pShmMutex) );
  }ਨਹੀਂਤਾਂ{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
    ਪੂਰਨ ii;
    ਲਈ(ii=ofst-UNIX_SHM_ਅਧਾਰ; ii<ofst-UNIX_SHM_ਅਧਾਰ+n; ii++){
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_Shm_ਗੰਢ->aMutex[ii]) );
    }
#ਨਹੀਂਤਾਂ
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_Shm_ਗੰਢ->pShmMutex) );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ->nRef>0 );
#ਜੇ_ਅੰਤ
  }

  /* Shared locks never span more than one byte */
  ਪ੍ਰਤਿਜ੍ਞਾ( n==1 || ਤਾਲਾ_Type!=F_RDLCK );

  /* Locks are within range */
  ਪ੍ਰਤਿਜ੍ਞਾ( n>=1 && n<=SQLITE_SHM_NLOCK );
  ਪ੍ਰਤਿਜ੍ਞਾ( ofst>=UNIX_SHM_ਅਧਾਰ && ofst<=UNIX_SHM_DMS );
  ਪ੍ਰਤਿਜ੍ਞਾ( ofst+n-1<=UNIX_SHM_DMS );

  ਜੇ( p_Shm_ਗੰਢ->hShm>=0 ){
    ਪੂਰਨ res;
    /* Initialize the locking parameters */
    f.l_type = ਤਾਲਾ_Type;
    f.l_whence = SEEK_SET;
    f.l_ਸ਼ੁਰੂ = ofst;
    f.l_len = n;
    res = os_ਸੈੱਟ_Posix_Advisory_ਤਾਲਾ(p_Shm_ਗੰਢ->hShm, &f, pFile);
    ਜੇ( res==-1 ){
#ਜੇ defined(SQLITE_ENABLE_SETLK_TIMEOUT) && SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = (pFile->iBusyTimeout ? SQLITE_BUSY_TIMEOUT : SQLITE_BUSY);
#ਨਹੀਂਤਾਂ
      rc = SQLITE_BUSY;
#ਜੇ_ਅੰਤ
    }
  }

  /* Do debug tracing */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  OSTRACE(("SHM-LOCK "));
  ਜੇ( rc==SQLITE_OK ){
    ਜੇ( ਤਾਲਾ_Type==F_UNLCK ){
      OSTRACE(("unlock %d..%d ok\n", ofst, ofst+n-1));
    }ਨਹੀਂਤਾਂ ਜੇ( ਤਾਲਾ_Type==F_RDLCK ){
      OSTRACE(("read-lock %d..%d ok\n", ofst, ofst+n-1));
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( ਤਾਲਾ_Type==F_WRLCK );
      OSTRACE(("write-lock %d..%d ok\n", ofst, ofst+n-1));
    }
  }ਨਹੀਂਤਾਂ{
    ਜੇ( ਤਾਲਾ_Type==F_UNLCK ){
      OSTRACE(("unlock %d..%d failed\n", ofst, ofst+n-1));
    }ਨਹੀਂਤਾਂ ਜੇ( ਤਾਲਾ_Type==F_RDLCK ){
      OSTRACE(("read-lock %d..%d failed\n", ofst, ofst+n-1));
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( ਤਾਲਾ_Type==F_WRLCK );
      OSTRACE(("write-lock %d..%d failed\n", ofst, ofst+n-1));
    }
  }
#ਜੇ_ਅੰਤ

  ਵਾਪਸ rc;
}

/*
** Return the minimum number of 32KB shm regions that should be mapped at
** a time, assuming that each mapping must be an integer multiple of the
** current system page-size.
**
** Usually, this is 1. The exception seems to be systems that are configured
** to use 64KB pages - in this case each mapping must cover at least two
** shm regions.
*/
ਸਥਿਰ ਪੂਰਨ unix_Shm_Region_ਪ੍ਰਤੀ_Map(ਖਾਲੀ){
  ਪੂਰਨ shmsz = 32*1024;            /* SHM region size */
  ਪੂਰਨ pgsz = osGetpagesize();   /* System page size */
  ਪ੍ਰਤਿਜ੍ਞਾ( ((pgsz-1)&pgsz)==0 );   /* Page size must be a power of 2 */
  ਜੇ( pgsz<shmsz ) ਵਾਪਸ 1;
  ਵਾਪਸ pgsz/shmsz;
}

/*
** Purge the unixShmNodeList list of all entries with unixShmNode.nRef==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
ਸਥਿਰ ਖਾਲੀ unixShmPurge(unixFile *pFd){
  unix_Shm_ਗੰਢ *p = pFd->pInode->p_Shm_ਗੰਢ;
  ਪ੍ਰਤਿਜ੍ਞਾ( unixMutexHeld() );
  ਜੇ( p && ALWAYS(p->nRef==0) ){
    ਪੂਰਨ n_Shm_ਪ੍ਰਤੀ_Map = unix_Shm_Region_ਪ੍ਰਤੀ_Map();
    ਪੂਰਨ i;
    ਪ੍ਰਤਿਜ੍ਞਾ( p->pInode==pFd->pInode );
    sqlite3_mutex_free(p->pShmMutex);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
    ਲਈ(i=0; i<SQLITE_SHM_NLOCK; i++){
      sqlite3_mutex_free(p->aMutex[i]);
    }
#ਜੇ_ਅੰਤ
    ਲਈ(i=0; i<p->nRegion; i+=n_Shm_ਪ੍ਰਤੀ_Map){
      ਜੇ( p->hShm>=0 ){
        osMunmap(p->apRegion[i], p->szRegion);
      }ਨਹੀਂਤਾਂ{
        sqlite3_free(p->apRegion[i]);
      }
    }
    sqlite3_free(p->apRegion);
    ਜੇ( p->hShm>=0 ){
      robust_close(pFd, p->hShm, __LINE__);
      p->hShm = -1;
    }
    p->pInode->p_Shm_ਗੰਢ = 0;
    sqlite3_free(p);
  }
}

/*
** The DMS lock has not yet been taken on shm file pShmNode. Attempt to
** take it now. Return SQLITE_OK if successful, or an SQLite error
** code otherwise.
**
** If the DMS cannot be locked because this is a readonly_shm=1
** connection and no other process already holds a lock, return
** SQLITE_READONLY_CANTINIT and set pShmNode->isUnlocked=1.
*/
ਸਥਿਰ ਪੂਰਨ unix_ਤਾਲਾ_Shared_ਸਮ੍ਰਿਤੀ(unixFile *pDbFd, unix_Shm_ਗੰਢ *p_Shm_ਗੰਢ){
  ਬਣਤਰ flock ਤਾਲਾ;
  ਪੂਰਨ rc = SQLITE_OK;

  /* Use F_GETLK to determine the locks other processes are holding
  ** on the DMS byte. If it indicates that another process is holding
  ** a SHARED lock, then this process may also take a SHARED lock
  ** and proceed with opening the *-shm file.
  **
  ** Or, if no other process is holding any lock, then this process
  ** is the first to open it. In this case take an EXCLUSIVE lock on the
  ** DMS byte and truncate the *-shm file to zero bytes in size. Then
  ** downgrade to a SHARED lock on the DMS byte.
  **
  ** If another process is holding an EXCLUSIVE lock on the DMS byte,
  ** return SQLITE_BUSY to the caller (it will try again). An earlier
  ** version of this code attempted the SHARED lock at this point. But
  ** this introduced a subtle race condition: if the process holding
  ** EXCLUSIVE failed just before truncating the *-shm file, then this
  ** process might open and use the *-shm file without truncating it.
  ** And if the *-shm file has been corrupted by a power failure or
  ** system crash, the database itself may also become corrupt.  */
  ਤਾਲਾ.l_whence = SEEK_SET;
  ਤਾਲਾ.l_ਸ਼ੁਰੂ = UNIX_SHM_DMS;
  ਤਾਲਾ.l_len = 1;
  ਤਾਲਾ.l_type = F_WRLCK;
  ਜੇ( osFcntl(p_Shm_ਗੰਢ->hShm, F_GETLK, &ਤਾਲਾ)!=0 ) {
    rc = SQLITE_IOERR_ਤਾਲਾ;
  }ਨਹੀਂਤਾਂ ਜੇ( ਤਾਲਾ.l_type==F_UNLCK ){
    ਜੇ( p_Shm_ਗੰਢ->isReadonly ){
      p_Shm_ਗੰਢ->isUnlocked = 1;
      rc = SQLITE_READONLY_CANTINIT;
    }ਨਹੀਂਤਾਂ{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
      /* Do not use a blocking lock here. If the lock cannot be obtained
      ** immediately, it means some other connection is truncating the
      ** *-shm file. And after it has done so, it will not release its
      ** lock, but only downgrade it to a shared lock. So no point in
      ** blocking here. The call below to obtain the shared DMS lock may 
      ** use a blocking lock. */
      ਪੂਰਨ iSaveTimeout = pDbFd->iBusyTimeout;
      pDbFd->iBusyTimeout = 0;
#ਜੇ_ਅੰਤ
      rc = unix_Shm_System_ਤਾਲਾ(pDbFd, F_WRLCK, UNIX_SHM_DMS, 1);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
      pDbFd->iBusyTimeout = iSaveTimeout;
#ਜੇ_ਅੰਤ
      /* The first connection to attach must truncate the -shm file.  We
      ** truncate to 3 bytes (an arbitrary small number, less than the
      ** -shm header size) rather than 0 as a system debugging aid, to
      ** help detect if a -shm file truncation is legitimate or is the work
      ** or a rogue process. */
      ਜੇ( rc==SQLITE_OK && robust_ftruncate(p_Shm_ਗੰਢ->hShm, 3) ){
        rc = unixLogError(SQLITE_IOERR_SHMOPEN,"ftruncate",p_Shm_ਗੰਢ->zFilename);
      }
    }
  }ਨਹੀਂਤਾਂ ਜੇ( ਤਾਲਾ.l_type==F_WRLCK ){
    rc = SQLITE_BUSY;
  }

  ਜੇ( rc==SQLITE_OK ){
    ਪ੍ਰਤਿਜ੍ਞਾ( ਤਾਲਾ.l_type==F_UNLCK || ਤਾਲਾ.l_type==F_RDLCK );
    rc = unix_Shm_System_ਤਾਲਾ(pDbFd, F_RDLCK, UNIX_SHM_DMS, 1);
  }
  ਵਾਪਸ rc;
}

/*
** Open a shared-memory area associated with open database file pDbFd.
** This particular implementation uses mmapped files.
**
** The file used to implement shared-memory is in the same directory
** as the open database file and has the same name as the open database
** file with the "-shm" suffix added.  For example, if the database file
** is "/home/user1/config.db" then the file that is created and mmapped
** for shared memory will be called "/home/user1/config.db-shm".
**
** Another approach to is to use files in /dev/shm or /dev/tmp or an
** some other tmpfs mount. But if a file in a different directory
** from the database file is used, then differing access permissions
** or a chroot() might cause two different processes on the same
** database to end up using different files for shared memory -
** meaning that their memory would not really be shared - resulting
** in database corruption.  Nevertheless, this tmpfs file usage
** can be enabled at compile-time using -DSQLITE_SHM_DIRECTORY="/dev/shm"
** or the equivalent.  The use of the SQLITE_SHM_DIRECTORY compile-time
** option results in an incompatible build of SQLite;  builds of SQLite
** that with differing SQLITE_SHM_DIRECTORY settings attempt to use the
** same database file at the same time, database corruption will likely
** result. The SQLITE_SHM_DIRECTORY compile-time option is considered
** "unsupported" and may go away in a future SQLite release.
**
** When opening a new shared-memory file, if no other instances of that
** file are currently open, in this process or in other processes, then
** the file must be truncated to zero length or have its header cleared.
**
** If the original database file (pDbFd) is using the "unix-excl" VFS
** that means that an exclusive lock is held on the database file and
** that no other processes are able to read or write the database.  In
** that case, we do not really need shared memory.  No shared memory
** file is created.  The shared memory will be simulated with heap memory.
*/
ਸਥਿਰ ਪੂਰਨ unix_Open_Shared_ਸਮ੍ਰਿਤੀ(unixFile *pDbFd){
  ਬਣਤਰ unixShm *p = 0;          /* The connection to be opened */
  ਬਣਤਰ unix_Shm_ਗੰਢ *p_Shm_ਗੰਢ;   /* The underlying mmapped file */
  ਪੂਰਨ rc = SQLITE_OK;             /* Result code */
  unixInodeInfo *pInode;          /* The inode of fd */
  ਅੱਖਰ *zShm;             /* Name of the file used for SHM */
  ਪੂਰਨ nShmFilename;               /* Size of the SHM filename in bytes */

  /* Allocate space for the new unixShm object. */
  p = sqlite3_malloc64( ਆਕਾਰ(*p) );
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
  memset(p, 0, ਆਕਾਰ(*p));
  ਪ੍ਰਤਿਜ੍ਞਾ( pDbFd->pShm==0 );

  /* Check to see if a unixShmNode object already exists. Reuse an existing
  ** one if present. Create a new one if necessary.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( unixFileMutexNotheld(pDbFd) );
  unixEnterMutex();
  pInode = pDbFd->pInode;
  p_Shm_ਗੰਢ = pInode->p_Shm_ਗੰਢ;
  ਜੇ( p_Shm_ਗੰਢ==0 ){
    ਬਣਤਰ stat sStat;                 /* fstat() info for database file */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_SHM_DIRECTORY
    ਅਚਲ ਅੱਖਰ *z_ਅਧਾਰ_ਰਸਤਾ = pDbFd->z_ਰਸਤਾ;
#ਜੇ_ਅੰਤ

    /* Call fstat() to figure out the permissions on the database file. If
    ** a new *-shm file is created, an attempt will be made to create it
    ** with the same permissions.
    */
    ਜੇ( osFstat(pDbFd->h, &sStat) ){
      rc = SQLITE_IOERR_FSTAT;
      ਜਾਓ shm_open_err;
    }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_SHM_DIRECTORY
    nShmFilename = ਆਕਾਰ(SQLITE_SHM_DIRECTORY) + 31;
#ਨਹੀਂਤਾਂ
    nShmFilename = 6 + (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(z_ਅਧਾਰ_ਰਸਤਾ);
#ਜੇ_ਅੰਤ
    p_Shm_ਗੰਢ = sqlite3_malloc64( ਆਕਾਰ(*p_Shm_ਗੰਢ) + nShmFilename );
    ਜੇ( p_Shm_ਗੰਢ==0 ){
      rc = SQLITE_NOMEM_BKPT;
      ਜਾਓ shm_open_err;
    }
    memset(p_Shm_ਗੰਢ, 0, ਆਕਾਰ(*p_Shm_ਗੰਢ)+nShmFilename);
    zShm = p_Shm_ਗੰਢ->zFilename = (ਅੱਖਰ*)&p_Shm_ਗੰਢ[1];
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_SHM_DIRECTORY
    sqlite3_snprintf(nShmFilename, zShm,
                     SQLITE_SHM_DIRECTORY "/sqlite-shm-%x-%x",
                     (u32)sStat.st_ino, (u32)sStat.st_dev);
#ਨਹੀਂਤਾਂ
    sqlite3_snprintf(nShmFilename, zShm, "%s-shm", z_ਅਧਾਰ_ਰਸਤਾ);
    sqlite3FileSuffix3(pDbFd->z_ਰਸਤਾ, zShm);
#ਜੇ_ਅੰਤ
    p_Shm_ਗੰਢ->hShm = -1;
    pDbFd->pInode->p_Shm_ਗੰਢ = p_Shm_ਗੰਢ;
    p_Shm_ਗੰਢ->pInode = pDbFd->pInode;
    ਜੇ( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_ਗੰਢ->pShmMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      ਜੇ( p_Shm_ਗੰਢ->pShmMutex==0 ){
        rc = SQLITE_NOMEM_BKPT;
        ਜਾਓ shm_open_err;
      }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
      {
        ਪੂਰਨ ii;
        ਲਈ(ii=0; ii<SQLITE_SHM_NLOCK; ii++){
          p_Shm_ਗੰਢ->aMutex[ii] = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
          ਜੇ( p_Shm_ਗੰਢ->aMutex[ii]==0 ){
            rc = SQLITE_NOMEM_BKPT;
            ਜਾਓ shm_open_err;
          }
        }
      }
#ਜੇ_ਅੰਤ
    }

    ਜੇ( pInode->b_Process_ਤਾਲਾ==0 ){
      ਜੇ( 0==sqlite3_uri_boolean(pDbFd->z_ਰਸਤਾ, "readonly_shm", 0) ){
        p_Shm_ਗੰਢ->hShm = robust_open(zShm, O_RDWR|O_CREAT|O_NOFOLLOW,
                                     (sStat.st_ਬਹੁਲਕ&0777));
      }
      ਜੇ( p_Shm_ਗੰਢ->hShm<0 ){
        p_Shm_ਗੰਢ->hShm = robust_open(zShm, O_RDONLY|O_NOFOLLOW,
                                     (sStat.st_ਬਹੁਲਕ&0777));
        ਜੇ( p_Shm_ਗੰਢ->hShm<0 ){
          rc = unixLogError(SQLITE_CANTOPEN_BKPT, "open", zShm);
          ਜਾਓ shm_open_err;
        }
        p_Shm_ਗੰਢ->isReadonly = 1;
      }

      /* If this process is running as root, make sure that the SHM file
      ** is owned by the same user that owns the original database.  Otherwise,
      ** the original owner will not be able to connect.
      */
      robustFchown(p_Shm_ਗੰਢ->hShm, sStat.st_uid, sStat.st_gid);

      rc = unix_ਤਾਲਾ_Shared_ਸਮ੍ਰਿਤੀ(pDbFd, p_Shm_ਗੰਢ);
      ਜੇ( rc!=SQLITE_OK && rc!=SQLITE_READONLY_CANTINIT ) ਜਾਓ shm_open_err;
    }
  }

  /* Make the new connection a child of the unixShmNode */
  p->p_Shm_ਗੰਢ = p_Shm_ਗੰਢ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  p->id = p_Shm_ਗੰਢ->ਅਗਲਾ_Shm_Id++;
#ਜੇ_ਅੰਤ
  p_Shm_ਗੰਢ->nRef++;
  pDbFd->pShm = p;
  unixLeaveMutex();

  /* The reference count on pShmNode has already been incremented under
  ** the cover of the unixEnterMutex() mutex and the pointer from the
  ** new (struct unixShm) object to the pShmNode has been set. All that is
  ** left to do is to link the new object into the linked list starting
  ** at pShmNode->pFirst. This must be done while holding the
  ** pShmNode->pShmMutex.
  */
  sqlite3_mutex_enter(p_Shm_ਗੰਢ->pShmMutex);
  p->p_ਅਗਲਾ = p_Shm_ਗੰਢ->p_ਪਹਿਲਾ;
  p_Shm_ਗੰਢ->p_ਪਹਿਲਾ = p;
  sqlite3_mutex_leave(p_Shm_ਗੰਢ->pShmMutex);
  ਵਾਪਸ rc;

  /* Jump here on any error */
shm_open_err:
  unixShmPurge(pDbFd);       /* This call frees pShmNode if required */
  sqlite3_free(p);
  unixLeaveMutex();
  ਵਾਪਸ rc;
}

/*
** This function is called to obtain a pointer to region iRegion of the
** shared-memory associated with the database file fd. Shared-memory regions
** are numbered starting from zero. Each shared-memory region is szRegion
** bytes in size.
**
** If an error occurs, an error code is returned and *pp is set to NULL.
**
** Otherwise, if the bExtend parameter is 0 and the requested shared-memory
** region has not been allocated (by any client, including one running in a
** separate process), then *pp is set to NULL and SQLITE_OK returned. If
** bExtend is non-zero and the requested shared-memory region has not yet
** been allocated, it is allocated by this function.
**
** If the shared-memory region has already been allocated or is allocated by
** this call as described above, then it is mapped into this processes
** address space (if it is not already), *pp is set to point to the mapped
** memory and SQLITE_OK returned.
*/
ਸਥਿਰ ਪੂਰਨ unixShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  ਪੂਰਨ iRegion,                    /* Region to retrieve */
  ਪੂਰਨ szRegion,                   /* Size of regions */
  ਪੂਰਨ bExtend,                    /* True to extend file if necessary */
  ਖਾਲੀ ਅਸਥਿਰ **pp              /* OUT: Mapped memory */
){
  unixFile *pDbFd = (unixFile*)fd;
  unixShm *p;
  unix_Shm_ਗੰਢ *p_Shm_ਗੰਢ;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ n_Shm_ਪ੍ਰਤੀ_Map = unix_Shm_Region_ਪ੍ਰਤੀ_Map();
  ਪੂਰਨ nReqRegion;

  /* If the shared-memory file has not yet been opened, open it now. */
  ਜੇ( pDbFd->pShm==0 ){
    rc = unix_Open_Shared_ਸਮ੍ਰਿਤੀ(pDbFd);
    ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
  }

  p = pDbFd->pShm;
  p_Shm_ਗੰਢ = p->p_Shm_ਗੰਢ;
  sqlite3_mutex_enter(p_Shm_ਗੰਢ->pShmMutex);
  ਜੇ( p_Shm_ਗੰਢ->isUnlocked ){
    rc = unix_ਤਾਲਾ_Shared_ਸਮ੍ਰਿਤੀ(pDbFd, p_Shm_ਗੰਢ);
    ਜੇ( rc!=SQLITE_OK ) ਜਾਓ shmpage_ਬਾਹਰਲਾ;
    p_Shm_ਗੰਢ->isUnlocked = 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( szRegion==p_Shm_ਗੰਢ->szRegion || p_Shm_ਗੰਢ->nRegion==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ->pInode==pDbFd->pInode );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ->hShm>=0 || pDbFd->pInode->b_Process_ਤਾਲਾ==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ->hShm<0 || pDbFd->pInode->b_Process_ਤਾਲਾ==0 );

  /* Minimum number of regions required to be mapped. */
  nReqRegion = ((iRegion+n_Shm_ਪ੍ਰਤੀ_Map) / n_Shm_ਪ੍ਰਤੀ_Map) * n_Shm_ਪ੍ਰਤੀ_Map;

  ਜੇ( p_Shm_ਗੰਢ->nRegion<nReqRegion ){
    ਅੱਖਰ **apNew;                         /* New apRegion[] array */
    i64 nByte = nReqRegion*(i64)szRegion; /* Minimum required file size */
    ਬਣਤਰ stat sStat;                    /* Used by fstat() */

    p_Shm_ਗੰਢ->szRegion = szRegion;

    ਜੇ( p_Shm_ਗੰਢ->hShm>=0 ){
      /* The requested region is not mapped into this processes address space.
      ** Check to see if it has been allocated (i.e. if the wal-index file is
      ** large enough to contain the requested region).
      */
      ਜੇ( osFstat(p_Shm_ਗੰਢ->hShm, &sStat) ){
        rc = SQLITE_IOERR_SHMSIZE;
        ਜਾਓ shmpage_ਬਾਹਰਲਾ;
      }

      ਜੇ( sStat.st_ਮਾਪ<nByte ){
        /* The requested memory region does not exist. If bExtend is set to
        ** false, exit early. *pp will be set to NULL and SQLITE_OK returned.
        */
        ਜੇ( !bExtend ){
          ਜਾਓ shmpage_ਬਾਹਰਲਾ;
        }

        /* Alternatively, if bExtend is true, extend the file. Do this by
        ** writing a single byte to the end of each (OS) page being
        ** allocated or extended. Technically, we need only write to the
        ** last page in order to extend the file. But writing to all new
        ** pages forces the OS to allocate them immediately, which reduces
        ** the chances of SIGBUS while accessing the mapped region later on.
        */
        ਨਹੀਂਤਾਂ{
          ਸਥਿਰ ਅਚਲ ਪੂਰਨ pgsz = 4096;
          i64 iPg;

          /* Write to the last byte of each newly allocated or extended page */
          ਪ੍ਰਤਿਜ੍ਞਾ( (nByte % pgsz)==0 );
          ਲਈ(iPg=(sStat.st_ਮਾਪ/pgsz); iPg<(nByte/pgsz); iPg++){
            ਪੂਰਨ x = 0;
            ਜੇ( seekAndWriteFd(p_Shm_ਗੰਢ->hShm, iPg*pgsz + pgsz-1,"",1,&x)!=1 ){
              ਅਚਲ ਅੱਖਰ *zFile = p_Shm_ਗੰਢ->zFilename;
              rc = unixLogError(SQLITE_IOERR_SHMSIZE, "write", zFile);
              ਜਾਓ shmpage_ਬਾਹਰਲਾ;
            }
          }
        }
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (ਅੱਖਰ **)sqlite3_realloc64(
        p_Shm_ਗੰਢ->apRegion, nReqRegion*ਆਕਾਰ(ਅੱਖਰ *)
    );
    ਜੇ( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      ਜਾਓ shmpage_ਬਾਹਰਲਾ;
    }
    p_Shm_ਗੰਢ->apRegion = apNew;
    ਜਦੋਂ( p_Shm_ਗੰਢ->nRegion<nReqRegion ){
      i64 nMap = (i64)szRegion*(i64)n_Shm_ਪ੍ਰਤੀ_Map;
      i64 i;
      ਖਾਲੀ *pMem;
      ਜੇ( p_Shm_ਗੰਢ->hShm>=0 ){
        pMem = osMmap(0, nMap,
            p_Shm_ਗੰਢ->isReadonly ? PROT_READ : PROT_READ|PROT_WRITE,
            MAP_SHARED, p_Shm_ਗੰਢ->hShm, szRegion*(i64)p_Shm_ਗੰਢ->nRegion
        );
        ਜੇ( pMem==MAP_FAILED ){
          rc = unixLogError(SQLITE_IOERR_SHMMAP, "mmap", p_Shm_ਗੰਢ->zFilename);
          ਜਾਓ shmpage_ਬਾਹਰਲਾ;
        }
      }ਨਹੀਂਤਾਂ{
        pMem = sqlite3_malloc64(nMap);
        ਜੇ( pMem==0 ){
          rc = SQLITE_NOMEM_BKPT;
          ਜਾਓ shmpage_ਬਾਹਰਲਾ;
        }
        memset(pMem, 0, nMap);
      }

      ਲਈ(i=0; i<n_Shm_ਪ੍ਰਤੀ_Map; i++){
        p_Shm_ਗੰਢ->apRegion[p_Shm_ਗੰਢ->nRegion+i] = &((ਅੱਖਰ*)pMem)[szRegion*i];
      }
      p_Shm_ਗੰਢ->nRegion += n_Shm_ਪ੍ਰਤੀ_Map;
    }
  }

shmpage_ਬਾਹਰਲਾ:
  ਜੇ( p_Shm_ਗੰਢ->nRegion>iRegion ){
    *pp = p_Shm_ਗੰਢ->apRegion[iRegion];
  }ਨਹੀਂਤਾਂ{
    *pp = 0;
  }
  ਜੇ( p_Shm_ਗੰਢ->isReadonly && rc==SQLITE_OK ) rc = SQLITE_READONLY;
  sqlite3_mutex_leave(p_Shm_ਗੰਢ->pShmMutex);
  ਵਾਪਸ rc;
}

/*
** Check that the pShmNode->aLock[] array comports with the locking bitmasks
** held by each client. Return true if it does, or false otherwise. This
** is to be used in an assert(). e.g.
**
**     assert( assertLockingArrayOk(pShmNode) );
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਸਥਿਰ ਪੂਰਨ assertLockingArrayOk(unix_Shm_ਗੰਢ *p_Shm_ਗੰਢ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
  ਵਾਪਸ 1;
#ਨਹੀਂਤਾਂ
  unixShm *pX;
  ਪੂਰਨ a_ਤਾਲਾ[SQLITE_SHM_NLOCK];

  memset(a_ਤਾਲਾ, 0, ਆਕਾਰ(a_ਤਾਲਾ));
  ਲਈ(pX=p_Shm_ਗੰਢ->p_ਪਹਿਲਾ; pX; pX=pX->p_ਅਗਲਾ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<SQLITE_SHM_NLOCK; i++){
      ਜੇ( pX->excl_ਨਕਾਬ & (1<<i) ){
        ਪ੍ਰਤਿਜ੍ਞਾ( a_ਤਾਲਾ[i]==0 );
        a_ਤਾਲਾ[i] = -1;
      }ਨਹੀਂਤਾਂ ਜੇ( pX->shared_ਨਕਾਬ & (1<<i) ){
        ਪ੍ਰਤਿਜ੍ਞਾ( a_ਤਾਲਾ[i]>=0 );
        a_ਤਾਲਾ[i]++;
      }
    }
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( 0==memcmp(p_Shm_ਗੰਢ->a_ਤਾਲਾ, a_ਤਾਲਾ, ਆਕਾਰ(a_ਤਾਲਾ)) );
  ਵਾਪਸ (memcmp(p_Shm_ਗੰਢ->a_ਤਾਲਾ, a_ਤਾਲਾ, ਆਕਾਰ(a_ਤਾਲਾ))==0);
#ਜੇ_ਅੰਤ
}
#ਜੇ_ਅੰਤ /* !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL) */

/*
** Change the lock state for a shared-memory segment.
**
** Note that the relationship between SHARED and EXCLUSIVE locks is a little
** different here than in posix.  In xShmLock(), one can go from unlocked
** to shared and back or from unlocked to exclusive and back.  But one may
** not go from shared to exclusive or from exclusive to shared.
*/
ਸਥਿਰ ਪੂਰਨ unix_Shm_ਤਾਲਾ(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  ਪੂਰਨ ofst,                  /* First lock to acquire or release */
  ਪੂਰਨ n,                     /* Number of locks to acquire or release */
  ਪੂਰਨ flags                  /* What to do with the lock */
){
  unixFile *pDbFd = (unixFile*)fd;      /* Connection holding shared memory */
  unixShm *p;                           /* The shared memory being locked */
  unix_Shm_ਗੰਢ *p_Shm_ਗੰਢ;                /* The underlying file iNode */
  ਪੂਰਨ rc = SQLITE_OK;                   /* Result code */
  u16 ਨਕਾਬ = (1<<(ofst+n)) - (1<<ofst); /* Mask of locks to take or release */
  ਪੂਰਨ *a_ਤਾਲਾ;

  p = pDbFd->pShm;
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_IOERR_SHMLOCK;
  p_Shm_ਗੰਢ = p->p_Shm_ਗੰਢ;
  ਜੇ( NEVER(p_Shm_ਗੰਢ==0) ) ਵਾਪਸ SQLITE_IOERR_SHMLOCK;
  a_ਤਾਲਾ = p_Shm_ਗੰਢ->a_ਤਾਲਾ;

  ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ==pDbFd->pInode->p_Shm_ਗੰਢ );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ->pInode==pDbFd->pInode );
  ਪ੍ਰਤਿਜ੍ਞਾ( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  ਪ੍ਰਤਿਜ੍ਞਾ( n>=1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( flags==(SQLITE_SHM_ਤਾਲਾ | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_ਤਾਲਾ | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  ਪ੍ਰਤਿਜ੍ਞਾ( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ->hShm>=0 || pDbFd->pInode->b_Process_ਤਾਲਾ==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ->hShm<0 || pDbFd->pInode->b_Process_ਤਾਲਾ==0 );

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
  ** This is assert()ed below.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_ਤਾਲਾ) 
       || 0==(p->excl_ਨਕਾਬ & ਨਕਾਬ) 
  );
  ਜੇ( ((flags & SQLITE_SHM_UNLOCK) && ((p->excl_ਨਕਾਬ|p->shared_ਨਕਾਬ) & ਨਕਾਬ))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_ਤਾਲਾ) && 0==(p->shared_ਨਕਾਬ & ਨਕਾਬ))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_ਤਾਲਾ))
  ){

    /* Take the required mutexes. In SETLK_TIMEOUT mode (blocking locks), if
    ** this is an attempt on an exclusive lock use sqlite3_mutex_try(). If any
    ** other thread is holding this mutex, then it is either holding or about
    ** to hold a lock exclusive to the one being requested, and we may
    ** therefore return SQLITE_BUSY to the caller.
    **
    ** Doing this prevents some deadlock scenarios. For example, thread 1 may
    ** be a checkpointer blocked waiting on the WRITER lock. And thread 2
    ** may be a normal SQL client upgrading to a write transaction. In this
    ** case thread 2 does a non-blocking request for the WRITER lock. But -
    ** if it were to use sqlite3_mutex_enter() then it would effectively
    ** become a (doomed) blocking request, as thread 2 would block until thread
    ** 1 obtained WRITER and released the mutex. Since thread 2 already holds
    ** a lock on a read-locking slot at this point, this breaks the
    ** anti-deadlock rules (see above).  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
    ਪੂਰਨ iMutex;
    ਲਈ(iMutex=ofst; iMutex<ofst+n; iMutex++){
      ਜੇ( flags==(SQLITE_SHM_ਤਾਲਾ|SQLITE_SHM_EXCLUSIVE) ){
        rc = sqlite3_mutex_try(p_Shm_ਗੰਢ->aMutex[iMutex]);
        ਜੇ( rc!=SQLITE_OK ) ਜਾਓ leave_shmnode_mutexes;
      }ਨਹੀਂਤਾਂ{
        sqlite3_mutex_enter(p_Shm_ਗੰਢ->aMutex[iMutex]);
      }
    }
#ਨਹੀਂਤਾਂ
    sqlite3_mutex_enter(p_Shm_ਗੰਢ->pShmMutex);
#ਜੇ_ਅੰਤ

    ਜੇ( ALWAYS(rc==SQLITE_OK) ){
      ਜੇ( flags & SQLITE_SHM_UNLOCK ){
        /* Case (a) - unlock.  */
        ਪੂਰਨ bUnlock = 1;
        ਪ੍ਰਤਿਜ੍ਞਾ( (p->excl_ਨਕਾਬ & p->shared_ਨਕਾਬ)==0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->excl_ਨਕਾਬ & ਨਕਾਬ)==ਨਕਾਬ );
        ਪ੍ਰਤਿਜ੍ਞਾ( !(flags & SQLITE_SHM_SHARED) || (p->shared_ਨਕਾਬ & ਨਕਾਬ)==ਨਕਾਬ );
  
        /* If this is a SHARED lock being unlocked, it is possible that other
        ** clients within this process are holding the same SHARED lock. In
        ** this case, set bUnlock to 0 so that the posix lock is not removed
        ** from the file-descriptor below.  */
        ਜੇ( flags & SQLITE_SHM_SHARED ){
          ਪ੍ਰਤਿਜ੍ਞਾ( n==1 );
          ਪ੍ਰਤਿਜ੍ਞਾ( a_ਤਾਲਾ[ofst]>=1 );
          ਜੇ( a_ਤਾਲਾ[ofst]>1 ){
            bUnlock = 0;
            a_ਤਾਲਾ[ofst]--;
            p->shared_ਨਕਾਬ &= ~ਨਕਾਬ;
          }
        }
  
        ਜੇ( bUnlock ){
          rc = unix_Shm_System_ਤਾਲਾ(pDbFd, F_UNLCK, ofst+UNIX_SHM_ਅਧਾਰ, n);
          ਜੇ( rc==SQLITE_OK ){
            memset(&a_ਤਾਲਾ[ofst], 0, ਆਕਾਰ(ਪੂਰਨ)*n);
            p->shared_ਨਕਾਬ &= ~ਨਕਾਬ;
            p->excl_ਨਕਾਬ &= ~ਨਕਾਬ;
          }
        }
      }ਨਹੀਂਤਾਂ ਜੇ( flags & SQLITE_SHM_SHARED ){
        /* Case (b) - a shared lock.  */
  
        ਜੇ( a_ਤਾਲਾ[ofst]<0 ){
          /* An exclusive lock is held by some other connection. BUSY. */ 
          rc = SQLITE_BUSY;
        }ਨਹੀਂਤਾਂ ਜੇ( a_ਤਾਲਾ[ofst]==0 ){
          rc = unix_Shm_System_ਤਾਲਾ(pDbFd, F_RDLCK, ofst+UNIX_SHM_ਅਧਾਰ, n);
        }
  
        /* Get the local shared locks */
        ਜੇ( rc==SQLITE_OK ){
          p->shared_ਨਕਾਬ |= ਨਕਾਬ;
          a_ਤਾਲਾ[ofst]++;
        }
      }ਨਹੀਂਤਾਂ{
        /* Case (c) - an exclusive lock.  */
        ਪੂਰਨ ii;
  
        ਪ੍ਰਤਿਜ੍ਞਾ( flags==(SQLITE_SHM_ਤਾਲਾ|SQLITE_SHM_EXCLUSIVE) );
        ਪ੍ਰਤਿਜ੍ਞਾ( (p->shared_ਨਕਾਬ & ਨਕਾਬ)==0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( (p->excl_ਨਕਾਬ & ਨਕਾਬ)==0 );
  
        /* Make sure no sibling connections hold locks that will block this
        ** lock.  If any do, return SQLITE_BUSY right away.  */
        ਲਈ(ii=ofst; ii<ofst+n; ii++){
          ਜੇ( a_ਤਾਲਾ[ii] ){
            rc = SQLITE_BUSY;
            ਰੋਕੋ;
          }
        }
  
        /* Get the exclusive locks at the system level. Then if successful
        ** also update the in-memory values. */
        ਜੇ( rc==SQLITE_OK ){
          rc = unix_Shm_System_ਤਾਲਾ(pDbFd, F_WRLCK, ofst+UNIX_SHM_ਅਧਾਰ, n);
          ਜੇ( rc==SQLITE_OK ){
            p->excl_ਨਕਾਬ |= ਨਕਾਬ;
            ਲਈ(ii=ofst; ii<ofst+n; ii++){
              a_ਤਾਲਾ[ii] = -1;
            }
          }
        }
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( assertLockingArrayOk(p_Shm_ਗੰਢ) );
    }

    /* Drop the mutexes acquired above. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
  leave_shmnode_mutexes:
    ਲਈ(iMutex--; iMutex>=ofst; iMutex--){
      sqlite3_mutex_leave(p_Shm_ਗੰਢ->aMutex[iMutex]);
    }
#ਨਹੀਂਤਾਂ
    sqlite3_mutex_leave(p_Shm_ਗੰਢ->pShmMutex);
#ਜੇ_ਅੰਤ
  }

  OSTRACE(("SHM-LOCK shmid-%d, pid-%d got %03x,%03x\n",
           p->id, osGetpid(0), p->shared_ਨਕਾਬ, p->excl_ਨਕਾਬ));
  ਵਾਪਸ rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
ਸਥਿਰ ਖਾਲੀ unixShmBarrier(
  sqlite3_file *fd                /* Database file holding the shared memory */
){
  ਅਣਵਰਤਿਆ_PARAMETER(fd);
  sqlite3_ਸਮ੍ਰਿਤੀ_Barrier();         /* compiler-defined memory barrier */
  ਪ੍ਰਤਿਜ੍ਞਾ( fd->pMethods->x_ਤਾਲਾ==nolock_ਤਾਲਾ
       || unixFileMutexNotheld((unixFile*)fd)
  );
  unixEnterMutex();               /* Also mutex, for redundancy */
  unixLeaveMutex();
}

/*
** Close a connection to shared-memory.  Delete the underlying
** storage if deleteFlag is true.
**
** If there is no shared memory associated with the connection then this
** routine is a harmless no-op.
*/
ਸਥਿਰ ਪੂਰਨ unixShmUnmap(
  sqlite3_file *fd,               /* The underlying database file */
  ਪੂਰਨ deleteFlag                  /* Delete shared-memory if true */
){
  unixShm *p;                     /* The connection to be closed */
  unix_Shm_ਗੰਢ *p_Shm_ਗੰਢ;          /* The underlying shared-memory file */
  unixShm **pp;                   /* For looping over sibling connections */
  unixFile *pDbFd;                /* The underlying database file */

  pDbFd = (unixFile*)fd;
  p = pDbFd->pShm;
  ਜੇ( p==0 ) ਵਾਪਸ SQLITE_OK;
  p_Shm_ਗੰਢ = p->p_Shm_ਗੰਢ;

  ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ==pDbFd->pInode->p_Shm_ਗੰਢ );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ->pInode==pDbFd->pInode );

  /* Remove connection p from the set of connections associated
  ** with pShmNode */
  sqlite3_mutex_enter(p_Shm_ਗੰਢ->pShmMutex);
  ਲਈ(pp=&p_Shm_ਗੰਢ->p_ਪਹਿਲਾ; (*pp)!=p; pp = &(*pp)->p_ਅਗਲਾ){}
  *pp = p->p_ਅਗਲਾ;

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  sqlite3_mutex_leave(p_Shm_ਗੰਢ->pShmMutex);

  /* If pShmNode->nRef has reached 0, then close the underlying
  ** shared-memory file, too */
  ਪ੍ਰਤਿਜ੍ਞਾ( unixFileMutexNotheld(pDbFd) );
  unixEnterMutex();
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Shm_ਗੰਢ->nRef>0 );
  p_Shm_ਗੰਢ->nRef--;
  ਜੇ( p_Shm_ਗੰਢ->nRef==0 ){
    ਜੇ( deleteFlag && p_Shm_ਗੰਢ->hShm>=0 ){
      osUnlink(p_Shm_ਗੰਢ->zFilename);
    }
    unixShmPurge(pDbFd);
  }
  unixLeaveMutex();

  ਵਾਪਸ SQLITE_OK;
}


#ਨਹੀਂਤਾਂ
# define unixShmMap     0
# define unix_Shm_ਤਾਲਾ    0
# define unixShmBarrier 0
# define unixShmUnmap   0
#ਜੇ_ਅੰਤ /* #ifndef SQLITE_OMIT_WAL */

#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
/*
** If it is currently memory mapped, unmap file pFd.
*/
ਸਥਿਰ ਖਾਲੀ unixUnmapfile(unixFile *pFd){
  ਪ੍ਰਤਿਜ੍ਞਾ( pFd->n_Fetch_ਬਾਹਰਲਾ==0 );
  ਜੇ( pFd->pMapRegion ){
    osMunmap(pFd->pMapRegion, pFd->mmap_ਮਾਪ_Actual);
    pFd->pMapRegion = 0;
    pFd->mmap_ਮਾਪ = 0;
    pFd->mmap_ਮਾਪ_Actual = 0;
  }
}

/*
** Attempt to set the size of the memory mapping maintained by file
** descriptor pFd to nNew bytes. Any existing mapping is discarded.
**
** If successful, this function sets the following variables:
**
**       unixFile.pMapRegion
**       unixFile.mmapSize
**       unixFile.mmapSizeActual
**
** If unsuccessful, an error message is logged via sqlite3_log() and
** the three variables above are zeroed. In this case SQLite should
** continue accessing the database using the xRead() and xWrite()
** methods.
*/
ਸਥਿਰ ਖਾਲੀ unixRemapfile(
  unixFile *pFd,                  /* File descriptor object */
  i64 nNew                        /* Required mapping size */
){
  ਅਚਲ ਅੱਖਰ *zErr = "mmap";
  ਪੂਰਨ h = pFd->h;                      /* File descriptor open on db file */
  u8 *pOrig = (u8 *)pFd->pMapRegion;   /* Pointer to current file mapping */
  u8 *pNew = 0;                        /* Location of new mapping */
  ਪੂਰਨ flags = PROT_READ;               /* Flags to pass to mmap() */

  ਪ੍ਰਤਿਜ੍ਞਾ( pFd->n_Fetch_ਬਾਹਰਲਾ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( nNew>pFd->mmap_ਮਾਪ );
  ਪ੍ਰਤਿਜ੍ਞਾ( nNew<=pFd->mmap_ਮਾਪ_Max );
  ਪ੍ਰਤਿਜ੍ਞਾ( nNew>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pFd->mmap_ਮਾਪ_Actual>=pFd->mmap_ਮਾਪ );
  ਪ੍ਰਤਿਜ੍ਞਾ( MAP_FAILED!=0 );

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_MMAP_READWRITE
  ਜੇ( (pFd->ctrlFlags & UNIXFILE_RDONLY)==0 ) flags |= PROT_WRITE;
#ਜੇ_ਅੰਤ

  ਜੇ( pOrig ){
    i64 nReuse = pFd->mmap_ਮਾਪ & ~((ਪੂਰਨ)osGetpagesize()-1);

    /* Unmap any part of the current mapping that will not be reused */
    ਜੇ( nReuse<pFd->mmap_ਮਾਪ_Actual ){
      osMunmap(&pOrig[nReuse], pFd->mmap_ਮਾਪ_Actual - nReuse);
    }

    /* If nReuse is now zero, then the entire mapping was unmapped by the
    ** call above. In this case do not attempt to extend pOrig. */
    ਜੇ( nReuse>0 ){
#ਜੇ HAVE_MREMAP
      pNew = osMremap(pOrig, nReuse, nNew, MREMAP_MAYMOVE);
      zErr = "mremap";
#ਨਹੀਂਤਾਂ
      pNew = osMmap(&pOrig[nReuse], nNew-nReuse, flags, MAP_SHARED, h, nReuse);
      ਜੇ( pNew!=MAP_FAILED ){
        ਜੇ( pNew!=&pOrig[nReuse] ){
          osMunmap(pNew, nNew - nReuse);
          pNew = 0;
        }ਨਹੀਂਤਾਂ{
          pNew = pOrig;
        }
      }
#ਜੇ_ਅੰਤ

      /* The attempt to extend the existing mapping failed. Free it. */
      ਜੇ( pNew==MAP_FAILED || pNew==0 ){
        osMunmap(pOrig, nReuse);
      }
    }
  }

  /* If pNew is still NULL, try to create an entirely new mapping. */
  ਜੇ( pNew==0 ){
    pNew = osMmap(0, nNew, flags, MAP_SHARED, h, 0);
  }

  ਜੇ( pNew==MAP_FAILED ){
    pNew = 0;
    nNew = 0;
    unixLogError(SQLITE_OK, zErr, pFd->z_ਰਸਤਾ);

    /* If the mmap() above failed, assume that all subsequent mmap() calls
    ** will probably fail too. Fall back to using xRead/xWrite exclusively
    ** in this case.  */
    pFd->mmap_ਮਾਪ_Max = 0;
  }
  pFd->pMapRegion = (ਖਾਲੀ *)pNew;
  pFd->mmap_ਮਾਪ = pFd->mmap_ਮਾਪ_Actual = nNew;
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
** using SQLITE_FCNTL_MMAP_LIMIT, whichever is smaller.
**
** SQLITE_OK is returned if no error occurs (even if the mapping is not
** recreated as a result of outstanding references) or an SQLite error
** code otherwise.
*/
ਸਥਿਰ ਪੂਰਨ unixMapfile(unixFile *pFd, i64 nMap){
  ਪ੍ਰਤਿਜ੍ਞਾ( nMap>=0 || pFd->n_Fetch_ਬਾਹਰਲਾ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( nMap>0 || (pFd->mmap_ਮਾਪ==0 && pFd->pMapRegion==0) );
  ਜੇ( pFd->n_Fetch_ਬਾਹਰਲਾ>0 ) ਵਾਪਸ SQLITE_OK;

  ਜੇ( nMap<0 ){
    ਬਣਤਰ stat statbuf;          /* Low-level file information */
    ਜੇ( osFstat(pFd->h, &statbuf) ){
      ਵਾਪਸ SQLITE_IOERR_FSTAT;
    }
    nMap = statbuf.st_ਮਾਪ;
  }
  ਜੇ( nMap>pFd->mmap_ਮਾਪ_Max ){
    nMap = pFd->mmap_ਮਾਪ_Max;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( nMap>0 || (pFd->mmap_ਮਾਪ==0 && pFd->pMapRegion==0) );
  ਜੇ( nMap!=pFd->mmap_ਮਾਪ ){
    unixRemapfile(pFd, nMap);
  }

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
** release the reference by calling unixUnfetch().
*/
ਸਥਿਰ ਪੂਰਨ unixFetch(sqlite3_file *fd, i64 iOff, ਪੂਰਨ nAmt, ਖਾਲੀ **pp){
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  unixFile *pFd = (unixFile *)fd;   /* The underlying database file */
#ਜੇ_ਅੰਤ
  *pp = 0;

#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  ਜੇ( pFd->mmap_ਮਾਪ_Max>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    ਅਚਲ ਪੂਰਨ n_Eof_ਬਫਰ = 256;
    ਜੇ( pFd->pMapRegion==0 ){
      ਪੂਰਨ rc = unixMapfile(pFd, -1);
      ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
    }
    ਜੇ( pFd->mmap_ਮਾਪ >= (iOff+nAmt+n_Eof_ਬਫਰ) ){
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->n_Fetch_ਬਾਹਰਲਾ++;
    }
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ SQLITE_OK;
}

/*
** If the third argument is non-NULL, then this function releases a
** reference obtained by an earlier call to unixFetch(). The second
** argument passed to this function must be the same as the corresponding
** argument that was passed to the unixFetch() invocation.
**
** Or, if the third argument is NULL, then this function is being called
** to inform the VFS layer that, according to POSIX, any existing mapping
** may now be invalid and should be unmapped.
*/
ਸਥਿਰ ਪੂਰਨ unixUnfetch(sqlite3_file *fd, i64 iOff, ਖਾਲੀ *p){
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  unixFile *pFd = (unixFile *)fd;   /* The underlying database file */
  ਅਣਵਰਤਿਆ_PARAMETER(iOff);

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( (p==0)==(pFd->n_Fetch_ਬਾਹਰਲਾ==0) );

  /* If p!=0, it must match the iOff value. */
  ਪ੍ਰਤਿਜ੍ਞਾ( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  ਜੇ( p ){
    pFd->n_Fetch_ਬਾਹਰਲਾ--;
  }ਨਹੀਂਤਾਂ{
    unixUnmapfile(pFd);
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( pFd->n_Fetch_ਬਾਹਰਲਾ>=0 );
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(fd);
  ਅਣਵਰਤਿਆ_PARAMETER(p);
  ਅਣਵਰਤਿਆ_PARAMETER(iOff);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ SQLITE_OK;
}

/*
** Here ends the implementation of all sqlite3_file methods.
**
********************** End sqlite3_file Methods *******************************
******************************************************************************/

/*
** This division contains definitions of sqlite3_io_methods objects that
** implement various file locking strategies.  It also contains definitions
** of "finder" functions.  A finder-function is used to locate the appropriate
** sqlite3_io_methods object for a particular database file.  The pAppData
** field of the sqlite3_vfs VFS objects are initialized to be pointers to
** the correct finder-function for that VFS.
**
** Most finder functions return a pointer to a fixed sqlite3_io_methods
** object.  The only interesting finder-function is autolockIoFinder, which
** looks at the filesystem type and tries to guess the best locking
** strategy from that.
**
** For finder-function F, two objects are created:
**
**    (1) The real finder-function named "FImpt()".
**
**    (2) A constant pointer to this function named just "F".
**
**
** A pointer to the F pointer is used as the pAppData value for VFS
** objects.  We have to do this instead of letting pAppData point
** directly at the finder-function since C90 rules prevent a void*
** from be cast into a function pointer.
**
**
** Each instance of this macro generates two objects:
**
**   *  A constant sqlite3_io_methods object call METHOD that has locking
**      methods CLOSE, LOCK, UNLOCK, CKRESLOCK.
**
**   *  An I/O method finder function called FINDER that returns a pointer
**      to the METHOD object in the previous bullet.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IOMETHODS(FINDER,ਵਿਧੀ,ਸੰਸਕਰਣ,CLOSE,ਤਾਲਾ_2,UNLOCK,CKLOCK,SHMMAP)     \
ਸਥਿਰ ਅਚਲ sqlite3_io_methods ਵਿਧੀ = {                                   \
   ਸੰਸਕਰਣ,                    /* iVersion */                                \
   CLOSE,                      /* xClose */                                  \
   unixRead,                   /* xRead */                                   \
   unixWrite,                  /* xWrite */                                  \
   unixTruncate,               /* xTruncate */                               \
   unixSync,                   /* xSync */                                   \
   unix_File_ਮਾਪ,               /* xFileSize */                               \
   ਤਾਲਾ_2,                       /* xLock */                                   \
   UNLOCK,                     /* xUnlock */                                 \
   CKLOCK,                     /* xCheckReservedLock */                      \
   unixFileControl,            /* xFileControl */                            \
   unix_Sector_ਮਾਪ,             /* xSectorSize */                             \
   unixDeviceCharacteristics,  /* xDeviceCapabilities */                     \
   SHMMAP,                     /* xShmMap */                                 \
   unix_Shm_ਤਾਲਾ,                /* xShmLock */                                \
   unixShmBarrier,             /* xShmBarrier */                             \
   unixShmUnmap,               /* xShmUnmap */                               \
   unixFetch,                  /* xFetch */                                  \
   unixUnfetch,                /* xUnfetch */                                \
};                                                                           \
ਸਥਿਰ ਅਚਲ sqlite3_io_methods *FINDER##Impl(ਅਚਲ ਅੱਖਰ *z, unixFile *p){   \
  ਅਣਵਰਤਿਆ_PARAMETER(z); ਅਣਵਰਤਿਆ_PARAMETER(p);                                  \
  ਵਾਪਸ &ਵਿਧੀ;                                                            \
}                                                                            \
ਸਥਿਰ ਅਚਲ sqlite3_io_methods *(*ਅਚਲ FINDER)(ਅਚਲ ਅੱਖਰ*,unixFile *p)    \
    = FINDER##Impl;

/*
** Here are all of the sqlite3_io_methods objects for each of the
** locking strategies.  Functions that return pointers to these methods
** are also created.
*/
IOMETHODS(
  posixIoFinder,            /* Finder function name */
  posixIoMethods,           /* sqlite3_io_methods object name */
  3,                        /* shared memory and mmap are enabled */
  unixClose,                /* xClose method */
  unix_ਤਾਲਾ,                 /* xLock method */
  unixUnlock,               /* xUnlock method */
  unix_Check_Reserved_ਤਾਲਾ,    /* xCheckReservedLock method */
  unixShmMap                /* xShmMap method */
)
IOMETHODS(
  nolockIoFinder,           /* Finder function name */
  nolockIoMethods,          /* sqlite3_io_methods object name */
  3,                        /* shared memory and mmap are enabled */
  nolockClose,              /* xClose method */
  nolock_ਤਾਲਾ,               /* xLock method */
  nolockUnlock,             /* xUnlock method */
  nolock_Check_Reserved_ਤਾਲਾ,  /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
IOMETHODS(
  dotlockIoFinder,          /* Finder function name */
  dotlockIoMethods,         /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  dotlockClose,             /* xClose method */
  dotlock_ਤਾਲਾ,              /* xLock method */
  dotlockUnlock,            /* xUnlock method */
  dotlock_Check_Reserved_ਤਾਲਾ, /* xCheckReservedLock method */
  0                         /* xShmMap method */
)

#ਜੇ SQLITE_ENABLE_LOCKING_STYLE
IOMETHODS(
  flockIoFinder,            /* Finder function name */
  flockIoMethods,           /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  flockClose,               /* xClose method */
  flock_ਤਾਲਾ,                /* xLock method */
  flockUnlock,              /* xUnlock method */
  flock_Check_Reserved_ਤਾਲਾ,   /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#ਜੇ_ਅੰਤ

#ਜੇ OS_VXWORKS
IOMETHODS(
  semIoFinder,              /* Finder function name */
  semIoMethods,             /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  semXClose,                /* xClose method */
  semXLock,                 /* xLock method */
  semXUnlock,               /* xUnlock method */
  sem_XCheck_Reserved_ਤਾਲਾ,    /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#ਜੇ_ਅੰਤ

#ਜੇ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
IOMETHODS(
  afpIoFinder,              /* Finder function name */
  afpIoMethods,             /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  afpClose,                 /* xClose method */
  afp_ਤਾਲਾ,                  /* xLock method */
  afpUnlock,                /* xUnlock method */
  afp_Check_Reserved_ਤਾਲਾ,     /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#ਜੇ_ਅੰਤ

/*
** The proxy locking method is a "super-method" in the sense that it
** opens secondary file descriptors for the conch and lock files and
** it uses proxy, dot-file, AFP, and flock() locking methods on those
** secondary files.  For this reason, the division that implements
** proxy locking is located much further down in the file.  But we need
** to go ahead and define the sqlite3_io_methods and finder function
** for proxy locking here.  So we forward declare the I/O methods.
*/
#ਜੇ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
ਸਥਿਰ ਪੂਰਨ proxyClose(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ proxy_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ proxyUnlock(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ proxy_Check_Reserved_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ*);
IOMETHODS(
  proxyIoFinder,            /* Finder function name */
  proxyIoMethods,           /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  proxyClose,               /* xClose method */
  proxy_ਤਾਲਾ,                /* xLock method */
  proxyUnlock,              /* xUnlock method */
  proxy_Check_Reserved_ਤਾਲਾ,   /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#ਜੇ_ਅੰਤ

/* nfs lockd on OSX 10.3+ doesn't clear write locks when a read lock is set */
#ਜੇ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
IOMETHODS(
  nfsIoFinder,               /* Finder function name */
  nfsIoMethods,              /* sqlite3_io_methods object name */
  1,                         /* shared memory is disabled */
  unixClose,                 /* xClose method */
  unix_ਤਾਲਾ,                  /* xLock method */
  nfsUnlock,                 /* xUnlock method */
  unix_Check_Reserved_ਤਾਲਾ,     /* xCheckReservedLock method */
  0                          /* xShmMap method */
)
#ਜੇ_ਅੰਤ

#ਜੇ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
** This "finder" function attempts to determine the best locking strategy
** for the database file "filePath".  It then returns the sqlite3_io_methods
** object that implements that strategy.
**
** This is for MacOSX only.
*/
ਸਥਿਰ ਅਚਲ sqlite3_io_methods *autolockIoFinderImpl(
  ਅਚਲ ਅੱਖਰ *file_ਰਸਤਾ,    /* name of the database file */
  unixFile *pNew           /* open file object for the database file */
){
  ਸਥਿਰ ਅਚਲ ਬਣਤਰ Mapping {
    ਅਚਲ ਅੱਖਰ *zFilesystem;              /* Filesystem type name */
    ਅਚਲ sqlite3_io_methods *pMethods;   /* Appropriate locking method */
  } aMap[] = {
    { "hfs",    &posixIoMethods },
    { "ufs",    &posixIoMethods },
    { "afpfs",  &afpIoMethods },
    { "smbfs",  &afpIoMethods },
    { "webdav", &nolockIoMethods },
    { 0, 0 }
  };
  ਪੂਰਨ i;
  ਬਣਤਰ statfs fsInfo;
  ਬਣਤਰ flock ਤਾਲਾ_Info;

  ਜੇ( !file_ਰਸਤਾ ){
    /* If filePath==NULL that means we are dealing with a transient file
    ** that does not need to be locked. */
    ਵਾਪਸ &nolockIoMethods;
  }
  ਜੇ( statfs(file_ਰਸਤਾ, &fsInfo) != -1 ){
    ਜੇ( fsInfo.f_flags & MNT_RDONLY ){
      ਵਾਪਸ &nolockIoMethods;
    }
    ਲਈ(i=0; aMap[i].zFilesystem; i++){
      ਜੇ( strcmp(fsInfo.f_fstypename, aMap[i].zFilesystem)==0 ){
        ਵਾਪਸ aMap[i].pMethods;
      }
    }
  }

  /* Default case. Handles, amongst others, "nfs".
  ** Test byte-range lock using fcntl(). If the call succeeds,
  ** assume that the file-system supports POSIX style locks.
  */
  ਤਾਲਾ_Info.l_len = 1;
  ਤਾਲਾ_Info.l_ਸ਼ੁਰੂ = 0;
  ਤਾਲਾ_Info.l_whence = SEEK_SET;
  ਤਾਲਾ_Info.l_type = F_RDLCK;
  ਜੇ( osFcntl(pNew->h, F_GETLK, &ਤਾਲਾ_Info)!=-1 ) {
    ਜੇ( strcmp(fsInfo.f_fstypename, "nfs")==0 ){
      ਵਾਪਸ &nfsIoMethods;
    } ਨਹੀਂਤਾਂ {
      ਵਾਪਸ &posixIoMethods;
    }
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ &dotlockIoMethods;
  }
}
ਸਥਿਰ ਅਚਲ sqlite3_io_methods
  *(*ਅਚਲ autolockIoFinder)(ਅਚਲ ਅੱਖਰ*,unixFile*) = autolockIoFinderImpl;

#ਜੇ_ਅੰਤ /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */

#ਜੇ OS_VXWORKS
/*
** This "finder" function for VxWorks checks to see if posix advisory
** locking works.  If it does, then that is what is used.  If it does not
** work, then fallback to named semaphore locking.
*/
ਸਥਿਰ ਅਚਲ sqlite3_io_methods *vxworksIoFinderImpl(
  ਅਚਲ ਅੱਖਰ *file_ਰਸਤਾ,    /* name of the database file */
  unixFile *pNew           /* the open file object */
){
  ਬਣਤਰ flock ਤਾਲਾ_Info;

  ਜੇ( !file_ਰਸਤਾ ){
    /* If filePath==NULL that means we are dealing with a transient file
    ** that does not need to be locked. */
    ਵਾਪਸ &nolockIoMethods;
  }

  /* Test if fcntl() is supported and use POSIX style locks.
  ** Otherwise fall back to the named semaphore method.
  */
  ਤਾਲਾ_Info.l_len = 1;
  ਤਾਲਾ_Info.l_ਸ਼ੁਰੂ = 0;
  ਤਾਲਾ_Info.l_whence = SEEK_SET;
  ਤਾਲਾ_Info.l_type = F_RDLCK;
  ਜੇ( osFcntl(pNew->h, F_GETLK, &ਤਾਲਾ_Info)!=-1 ) {
    ਵਾਪਸ &posixIoMethods;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ &semIoMethods;
  }
}
ਸਥਿਰ ਅਚਲ sqlite3_io_methods
  *(*ਅਚਲ vxworksIoFinder)(ਅਚਲ ਅੱਖਰ*,unixFile*) = vxworksIoFinderImpl;

#ਜੇ_ਅੰਤ /* OS_VXWORKS */

/*
** An abstract type for a pointer to an IO method finder function:
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਅਚਲ sqlite3_io_methods *(*finder_type)(ਅਚਲ ਅੱਖਰ*,unixFile*);


/****************************************************************************
**************************** sqlite3_vfs methods ****************************
**
** This division contains the implementation of methods on the
** sqlite3_vfs object.
*/

/*
** Initialize the contents of the unixFile structure pointed to by pId.
*/
ਸਥਿਰ ਪੂਰਨ fill_ਵਿੱਚ_Unix_File(
  sqlite3_vfs *pVfs,      /* Pointer to vfs object */
  ਪੂਰਨ h,                  /* Open file descriptor of file being opened */
  sqlite3_file *pId,      /* Write to the unixFile structure here */
  ਅਚਲ ਅੱਖਰ *zFilename,  /* Name of the file being opened */
  ਪੂਰਨ ctrlFlags           /* Zero or more UNIXFILE_* values */
){
  ਅਚਲ sqlite3_io_methods *pLockingStyle;
  unixFile *pNew = (unixFile *)pId;
  ਪੂਰਨ rc = SQLITE_OK;

  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->pInode==ਨਿਰਾ );

  /* No locking occurs in temporary files */
  ਪ੍ਰਤਿਜ੍ਞਾ( zFilename!=0 || (ctrlFlags & UNIXFILE_NOLOCK)!=0 );

  OSTRACE(("OPEN    %-3d %s\n", h, zFilename));
  pNew->h = h;
  pNew->pVfs = pVfs;
  pNew->z_ਰਸਤਾ = zFilename;
  pNew->ctrlFlags = (u8)ctrlFlags;
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  pNew->mmap_ਮਾਪ_Max = sqlite3GlobalConfig.szMmap;
#ਜੇ_ਅੰਤ
  ਜੇ( sqlite3_uri_boolean(((ctrlFlags & UNIXFILE_URI) ? zFilename : 0),
                           "psow", SQLITE_POWERSAFE_OVERWRITE) ){
    pNew->ctrlFlags |= UNIXFILE_PSOW;
  }
  ਜੇ( strcmp(pVfs->zName,"unix-excl")==0 ){
    pNew->ctrlFlags |= UNIXFILE_EXCL;
  }

#ਜੇ OS_VXWORKS
  pNew->pId = vxworks_ਲੱਭੋ_File_Id(zFilename);
  ਜੇ( pNew->pId==0 ){
    ctrlFlags |= UNIXFILE_NOLOCK;
    rc = SQLITE_NOMEM_BKPT;
  }
#ਜੇ_ਅੰਤ

  ਜੇ( ctrlFlags & UNIXFILE_NOLOCK ){
    pLockingStyle = &nolockIoMethods;
  }ਨਹੀਂਤਾਂ{
    pLockingStyle = (**(finder_type*)pVfs->p_App_ਅੰਕੜੇ)(zFilename, pNew);
#ਜੇ SQLITE_ENABLE_LOCKING_STYLE
    /* Cache zFilename in the locking context (AFP and dotlock override) for
    ** proxyLock activation is possible (remote proxy is based on db name)
    ** zFilename remains valid until file is closed, to support */
    pNew->lockingContext = (ਖਾਲੀ*)zFilename;
#ਜੇ_ਅੰਤ
  }

  ਜੇ( pLockingStyle == &posixIoMethods
#ਜੇ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
    || pLockingStyle == &nfsIoMethods
#ਜੇ_ਅੰਤ
  ){
    unixEnterMutex();
    rc = ਲੱਭੋ_Inode_Info(pNew, &pNew->pInode);
    ਜੇ( rc!=SQLITE_OK ){
      /* If an error occurred in findInodeInfo(), close the file descriptor
      ** immediately, before releasing the mutex. findInodeInfo() may fail
      ** in two scenarios:
      **
      **   (a) A call to fstat() failed.
      **   (b) A malloc failed.
      **
      ** Scenario (b) may only occur if the process is holding no other
      ** file descriptors open on the same file. If there were other file
      ** descriptors on this file, then no malloc would be required by
      ** findInodeInfo(). If this is the case, it is quite safe to close
      ** handle h - as it is guaranteed that no posix locks will be released
      ** by doing so.
      **
      ** If scenario (a) caused the error then things are not so safe. The
      ** implicit assumption here is that if fstat() fails, things are in
      ** such bad shape that dropping a lock or two doesn't matter much.
      */
      robust_close(pNew, h, __LINE__);
      h = -1;
    }
    unixLeaveMutex();
  }

#ਜੇ SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
  ਨਹੀਂਤਾਂ ਜੇ( pLockingStyle == &afpIoMethods ){
    /* AFP locking uses the file path so it needs to be included in
    ** the afpLockingContext.
    */
    afpLockingContext *pCtx;
    pNew->lockingContext = pCtx = sqlite3_malloc64( ਆਕਾਰ(*pCtx) );
    ਜੇ( pCtx==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }ਨਹੀਂਤਾਂ{
      /* NB: zFilename exists and remains valid until the file is closed
      ** according to requirement F11141.  So we do not need to make a
      ** copy of the filename. */
      pCtx->db_ਰਸਤਾ = zFilename;
      pCtx->reserved = 0;
      srandomdev();
      unixEnterMutex();
      rc = ਲੱਭੋ_Inode_Info(pNew, &pNew->pInode);
      ਜੇ( rc!=SQLITE_OK ){
        sqlite3_free(pNew->lockingContext);
        robust_close(pNew, h, __LINE__);
        h = -1;
      }
      unixLeaveMutex();      
    }
  }
#ਜੇ_ਅੰਤ

  ਨਹੀਂਤਾਂ ਜੇ( pLockingStyle == &dotlockIoMethods ){
    /* Dotfile locking uses the file path so it needs to be included in
    ** the dotlockLockingContext
    */
    ਅੱਖਰ *z_ਤਾਲਾ_File;
    ਪੂਰਨ nFilename;
    ਪ੍ਰਤਿਜ੍ਞਾ( zFilename!=0 );
    nFilename = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zFilename) + 6;
    z_ਤਾਲਾ_File = (ਅੱਖਰ *)sqlite3_malloc64(nFilename);
    ਜੇ( z_ਤਾਲਾ_File==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }ਨਹੀਂਤਾਂ{
      sqlite3_snprintf(nFilename, z_ਤਾਲਾ_File, "%s" DOTLOCK_SUFFIX, zFilename);
    }
    pNew->lockingContext = z_ਤਾਲਾ_File;
  }

#ਜੇ OS_VXWORKS
  ਨਹੀਂਤਾਂ ਜੇ( pLockingStyle == &semIoMethods ){
    /* Named semaphore locking uses the file path so it needs to be
    ** included in the semLockingContext
    */
    unixEnterMutex();
    rc = ਲੱਭੋ_Inode_Info(pNew, &pNew->pInode);
    ਜੇ( (rc==SQLITE_OK) && (pNew->pInode->pSem==ਨਿਰਾ) ){
      ਅੱਖਰ *zSemName = pNew->pInode->aSemName;
      ਪੂਰਨ n;
      sqlite3_snprintf(MAX_PATHNAME, zSemName, "/%s.sem",
                       pNew->pId->zCanonicalName);
      ਲਈ( n=1; zSemName[n]; n++ )
        ਜੇ( zSemName[n]=='/' ) zSemName[n] = '_';
      pNew->pInode->pSem = sem_open(zSemName, O_CREAT, 0666, 1);
      ਜੇ( pNew->pInode->pSem == SEM_FAILED ){
        rc = SQLITE_NOMEM_BKPT;
        pNew->pInode->aSemName[0] = '\0';
      }
    }
    unixLeaveMutex();
  }
#ਜੇ_ਅੰਤ

  store_ਆਖਰੀ_Errno(pNew, 0);
#ਜੇ OS_VXWORKS
  ਜੇ( rc!=SQLITE_OK ){
    ਜੇ( h>=0 ){
      robust_close(pNew, h, __LINE__);
      h = -1;
    }
    ਜੇ( pNew->ctrlFlags & UNIXFILE_DELETE ){
      osUnlink(zFilename);
    }
    ਜੇ( pNew->pId ){
      vxworksReleaseFileId(pNew->pId);
      pNew->pId = 0;
    }
  }
#ਜੇ_ਅੰਤ
  ਜੇ( rc!=SQLITE_OK ){
    ਜੇ( h>=0 ) robust_close(pNew, h, __LINE__);
  }ਨਹੀਂਤਾਂ{
    pId->pMethods = pLockingStyle;
    OpenCounter(+1);
    verifyDbFile(pNew);
  }
  ਵਾਪਸ rc;
}

/*
** Return the name of a directory in which to put temporary files.
** If no suitable temporary file directory can be found, return NULL.
**
** The return value might be a string obtained from getenv() and so
** the return value should not be used after any call to setenv() or
** putenv() as that value might have been freed.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *unix_ਆਰਜ਼ੀ_File_Dir(ਖਾਲੀ){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਬਣਤਰ stat buf;
  ਅਚਲ ਅੱਖਰ *zDir;

  ਲਈ(i=0; i<7; i++){
    ਚੋਣ( i ){
      ਹਾਲਤ 0:  zDir = sqlite3_ਆਰਜ਼ੀ_directory;   ਰੋਕੋ;
      ਹਾਲਤ 1:  zDir = getenv("SQLITE_TMPDIR");  ਰੋਕੋ;
      ਹਾਲਤ 2:  zDir = getenv("TMPDIR");         ਰੋਕੋ;
      ਹਾਲਤ 3:  zDir = "/var/tmp";               ਰੋਕੋ;
      ਹਾਲਤ 4:  zDir = "/usr/tmp";               ਰੋਕੋ;
      ਹਾਲਤ 5:  zDir = "/tmp";                   ਰੋਕੋ;
      ਮੂਲ: zDir = ".";                      ਰੋਕੋ;
    }
    ਜੇ( zDir!=0
#ਜੇ OS_VXWORKS
     && zDir[0]=='/'
#ਜੇ_ਅੰਤ
     && osStat(zDir, &buf)==0
     && S_ISDIR(buf.st_ਬਹੁਲਕ)
     && osAccess(zDir, 03)==0
    ){
      ਵਾਪਸ zDir;
    }
  }
  ਵਾਪਸ 0;
}

/*
** Create a temporary file name in zBuf.  zBuf must be allocated
** by the calling process and must be big enough to hold at least
** pVfs->mxPathname bytes.
*/
ਸਥਿਰ ਪੂਰਨ unixGetTempname(ਪੂਰਨ nBuf, ਅੱਖਰ *zBuf){
  ਅਚਲ ਅੱਖਰ *zDir;
  ਪੂਰਨ i_ਹੱਦਬੰਦੀ = 0;
  ਪੂਰਨ rc = SQLITE_OK;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  zBuf[0] = 0;
  SimulateIOError( ਵਾਪਸ SQLITE_IOERR );

  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  zDir = unix_ਆਰਜ਼ੀ_File_Dir();
  ਜੇ( zDir==0 ){
    rc = SQLITE_IOERR_GETTEMPPATH;
  }ਨਹੀਂਤਾਂ{
    ਕਰੋ{
      u64 r;
      sqlite3_randomness(ਆਕਾਰ(r), &r);
      ਪ੍ਰਤਿਜ੍ਞਾ( nBuf>2 );
      zBuf[nBuf-2] = 0;
      sqlite3_snprintf(nBuf, zBuf, "%s/"SQLITE_ਆਰਜ਼ੀ_FILE_ਅਗੇਤਰ"%llx%c",
                       zDir, r, 0);
      ਜੇ( zBuf[nBuf-2]!=0 || (i_ਹੱਦਬੰਦੀ++)>10 ){
        rc = SQLITE_ERROR;
        ਰੋਕੋ;
      }
    }ਜਦੋਂ( osAccess(zBuf,0)==0 );
  }
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ਵਾਪਸ rc;
}

#ਜੇ SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
/*
** Routine to transform a unixFile into a proxy-locking unixFile.
** Implementation in the proxy-lock division, but used by unixOpen()
** if SQLITE_PREFER_PROXY_LOCKING is defined.
*/
ਸਥਿਰ ਪੂਰਨ proxyTransformUnixFile(unixFile*, ਅਚਲ ਅੱਖਰ*);
#ਜੇ_ਅੰਤ

/*
** Search for an unused file descriptor that was opened on the database
** file (not a journal or super-journal file) identified by pathname
** zPath with SQLITE_OPEN_XXX flags matching those passed as the second
** argument to this function.
**
** Such a file descriptor may exist if a database connection was closed
** but the associated file descriptor could not be closed because some
** other file descriptor open on the same file is holding a file-lock.
** Refer to comments in the unixClose() function and the lengthy comment
** describing "Posix Advisory Locking" at the start of this file for
** further details. Also, ticket #4018.
**
** If a suitable file descriptor is found, then it is returned. If no
** such file descriptor is located, -1 is returned.
*/
ਸਥਿਰ Unix_ਅਣਵਰਤਿਆ_Fd *ਲੱਭੋ_Reusable_Fd(ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਪੂਰਨ flags){
  Unix_ਅਣਵਰਤਿਆ_Fd *p_ਅਣਵਰਤਿਆ = 0;

  /* Do not search for an unused file descriptor on vxworks. Not because
  ** vxworks would not benefit from the change (it might, we're not sure),
  ** but because no way to test it is currently available. It is better
  ** not to risk breaking vxworks support for the sake of such an obscure
  ** feature.  */
#ਜੇ !OS_VXWORKS
  ਬਣਤਰ stat sStat;                   /* Results of stat() call */

  unixEnterMutex();

  /* A stat() call may fail for various reasons. If this happens, it is
  ** almost certain that an open() call on the same path will also fail.
  ** For this reason, if an error occurs in the stat() call here, it is
  ** ignored and -1 is returned. The caller will try to open a new file
  ** descriptor on the same path, fail, and return an error to SQLite.
  **
  ** Even if a subsequent open() call does succeed, the consequences of
  ** not searching for a reusable file descriptor are not dire.  */
  ਜੇ( inodeList!=0 && 0==osStat(z_ਰਸਤਾ, &sStat) ){
    unixInodeInfo *pInode;

    pInode = inodeList;
    ਜਦੋਂ( pInode && (pInode->fileId.dev!=sStat.st_dev
                     || pInode->fileId.ino!=(u64)sStat.st_ino) ){
       pInode = pInode->p_ਅਗਲਾ;
    }
    ਜੇ( pInode ){
      Unix_ਅਣਵਰਤਿਆ_Fd **pp;
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_notheld(pInode->p_ਤਾਲਾ_Mutex) );
      sqlite3_mutex_enter(pInode->p_ਤਾਲਾ_Mutex);
      flags &= (SQLITE_OPEN_READONLY|SQLITE_OPEN_READWRITE);
      ਲਈ(pp=&pInode->p_ਅਣਵਰਤਿਆ; *pp && (*pp)->flags!=flags; pp=&((*pp)->p_ਅਗਲਾ));
      p_ਅਣਵਰਤਿਆ = *pp;
      ਜੇ( p_ਅਣਵਰਤਿਆ ){
        *pp = p_ਅਣਵਰਤਿਆ->p_ਅਗਲਾ;
      }
      sqlite3_mutex_leave(pInode->p_ਤਾਲਾ_Mutex);
    }
  }
  unixLeaveMutex();
#ਜੇ_ਅੰਤ    /* if !OS_VXWORKS */
  ਵਾਪਸ p_ਅਣਵਰਤਿਆ;
}

/*
** Find the mode, uid and gid of file zFile.
*/
ਸਥਿਰ ਪੂਰਨ get_File_ਬਹੁਲਕ(
  ਅਚਲ ਅੱਖਰ *zFile,              /* File name */
  ਬਹੁਲਕ_t *p_ਬਹੁਲਕ,                  /* OUT: Permissions of zFile */
  uid_t *pUid,                    /* OUT: uid of zFile. */
  gid_t *pGid                     /* OUT: gid of zFile. */
){
  ਬਣਤਰ stat sStat;              /* Output of stat() on database file */
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( 0==osStat(zFile, &sStat) ){
    *p_ਬਹੁਲਕ = sStat.st_ਬਹੁਲਕ & 0777;
    *pUid = sStat.st_uid;
    *pGid = sStat.st_gid;
  }ਨਹੀਂਤਾਂ{
    rc = SQLITE_IOERR_FSTAT;
  }
  ਵਾਪਸ rc;
}

/*
** This function is called by unixOpen() to determine the unix permissions
** to create new files with. If no error occurs, then SQLITE_OK is returned
** and a value suitable for passing as the third argument to open(2) is
** written to *pMode. If an IO error occurs, an SQLite error code is
** returned and the value of *pMode is not modified.
**
** In most cases, this routine sets *pMode to 0, which will become
** an indication to robust_open() to create the file using
** SQLITE_DEFAULT_FILE_PERMISSIONS adjusted by the umask.
** But if the file being opened is a WAL or regular journal file, then
** this function queries the file-system for the permissions on the
** corresponding database file and sets *pMode to this value. Whenever
** possible, WAL and journal files are created using the same permissions
** as the associated database file.
**
** If the SQLITE_ENABLE_8_3_NAMES option is enabled, then the
** original filename is unavailable.  But 8_3_NAMES is only used for
** FAT filesystems and permissions do not matter there, so just use
** the default permissions.  In 8_3_NAMES mode, leave *pMode set to zero.
*/
ਸਥਿਰ ਪੂਰਨ ਲੱਭੋ_Create_File_ਬਹੁਲਕ(
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,              /* Path of file (possibly) being created */
  ਪੂਰਨ flags,                      /* Flags passed as 4th argument to xOpen() */
  ਬਹੁਲਕ_t *p_ਬਹੁਲਕ,                  /* OUT: Permissions to open file with */
  uid_t *pUid,                    /* OUT: uid to set on the file */
  gid_t *pGid                     /* OUT: gid to set on the file */
){
  ਪੂਰਨ rc = SQLITE_OK;             /* Return Code */
  *p_ਬਹੁਲਕ = 0;
  *pUid = 0;
  *pGid = 0;
  ਜੇ( flags & (SQLITE_OPEN_WAL|SQLITE_OPEN_MAIN_JOURNAL) ){
    ਅੱਖਰ zDb[MAX_PATHNAME+1];     /* Database file path */
    ਪੂਰਨ nDb;                      /* Number of valid bytes in zDb */

    /* zPath is a path to a WAL or journal file. The following block derives
    ** the path to the associated database file from zPath. This block handles
    ** the following naming conventions:
    **
    **   "<path to db>-journal"
    **   "<path to db>-wal"
    **   "<path to db>-journalNN"
    **   "<path to db>-walNN"
    **
    ** where NN is a decimal number. The NN naming schemes are
    ** used by the test_multiplex.c module.
    **
    ** In normal operation, the journal file name will always contain
    ** a '-' character.  However in 8+3 filename mode, or if a corrupt
    ** rollback journal specifies a super-journal with a goofy name, then
    ** the '-' might be missing or the '-' might be the first character in
    ** the filename.  In that case, just return SQLITE_OK with *pMode==0.
    */
    nDb = sqlite3Strlen30(z_ਰਸਤਾ) - 1;
    ਜਦੋਂ( nDb>0 && z_ਰਸਤਾ[nDb]!='.' ){
      ਜੇ( z_ਰਸਤਾ[nDb]=='-' ){
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(zDb, z_ਰਸਤਾ, nDb);
        zDb[nDb] = '\0';
        rc = get_File_ਬਹੁਲਕ(zDb, p_ਬਹੁਲਕ, pUid, pGid);
        ਰੋਕੋ;
      }
      nDb--;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( flags & SQLITE_OPEN_DELETEONCLOSE ){
    *p_ਬਹੁਲਕ = 0600;
  }ਨਹੀਂਤਾਂ ਜੇ( flags & SQLITE_OPEN_URI ){
    /* If this is a main database file and the file was opened using a URI
    ** filename, check for the "modeof" parameter. If present, interpret
    ** its value as a filename and try to copy the mode, uid and gid from
    ** that file.  */
    ਅਚਲ ਅੱਖਰ *z = sqlite3_uri_parameter(z_ਰਸਤਾ, "modeof");
    ਜੇ( z ){
      rc = get_File_ਬਹੁਲਕ(z, p_ਬਹੁਲਕ, pUid, pGid);
    }
  }
  ਵਾਪਸ rc;
}

/*
** Open the file zPath.
**
** Previously, the SQLite OS layer used three functions in place of this
** one:
**
**     sqlite3OsOpenReadWrite();
**     sqlite3OsOpenReadOnly();
**     sqlite3OsOpenExclusive();
**
** These calls correspond to the following combinations of flags:
**
**     ReadWrite() ->     (READWRITE | CREATE)
**     ReadOnly()  ->     (READONLY)
**     OpenExclusive() -> (READWRITE | CREATE | EXCLUSIVE)
**
** The old OpenExclusive() accepted a boolean argument - "delFlag". If
** true, the file was configured to be automatically deleted when the
** file handle closed. To achieve the same effect using this new
** interface, add the DELETEONCLOSE flag to those specified above for
** OpenExclusive().
*/
ਸਥਿਰ ਪੂਰਨ unixOpen(
  sqlite3_vfs *pVfs,           /* The VFS for which this is the xOpen method */
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,           /* Pathname of file to be opened */
  sqlite3_file *pFile,         /* The file descriptor to be filled in */
  ਪੂਰਨ flags,                   /* Input flags to control the opening */
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags               /* Output flags returned to SQLite core */
){
  unixFile *p = (unixFile *)pFile;
  ਪੂਰਨ fd = -1;                   /* File descriptor returned by open() */
  ਪੂਰਨ openFlags = 0;             /* Flags to pass to open() */
  ਪੂਰਨ eType = flags&0x0FFF00;  /* Type of file to open */
  ਪੂਰਨ no_ਤਾਲਾ;                    /* True to omit locking primitives */
  ਪੂਰਨ rc = SQLITE_OK;            /* Function Return Code */
  ਪੂਰਨ ctrlFlags = 0;             /* UNIXFILE_* flags */

  ਪੂਰਨ isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  ਪੂਰਨ isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  ਪੂਰਨ isCreate     = (flags & SQLITE_OPEN_CREATE);
  ਪੂਰਨ isReadonly   = (flags & SQLITE_OPEN_READONLY);
  ਪੂਰਨ isReadWrite  = (flags & SQLITE_OPEN_READWRITE);
#ਜੇ SQLITE_ENABLE_LOCKING_STYLE
  ਪੂਰਨ isAutoProxy  = (flags & SQLITE_OPEN_AUTOPROXY);
#ਜੇ_ਅੰਤ
#ਜੇ defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
  ਬਣਤਰ statfs fsInfo;
#ਜੇ_ਅੰਤ

  /* If creating a super- or main-file journal, this function will open
  ** a file-descriptor on the directory too. The first time unixSync()
  ** is called the directory file descriptor will be fsync()ed and close()d.
  */
  ਪੂਰਨ isNewJrnl = (isCreate && (
        eType==SQLITE_OPEN_SUPER_JOURNAL
     || eType==SQLITE_OPEN_MAIN_JOURNAL
     || eType==SQLITE_OPEN_WAL
  ));

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  ਅੱਖਰ zTmpname[MAX_PATHNAME+2];
  ਅਚਲ ਅੱਖਰ *zName = z_ਰਸਤਾ;

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

#ਜੇ OS_VXWORKS
  /* The file-ID mechanism used in Vxworks requires that all pathnames
  ** provided to unixOpen must be absolute pathnames. */
  ਜੇ( z_ਰਸਤਾ!=0 && z_ਰਸਤਾ[0]!='/' ){ ਵਾਪਸ SQLITE_CANTOPEN; }
#ਜੇ_ਅੰਤ

  /* Detect a pid change and reset the PRNG.  There is a race condition
  ** here such that two or more threads all trying to open databases at
  ** the same instant might all reset the PRNG.  But multiple resets
  ** are harmless.
  */
  ਜੇ( randomnessPid!=osGetpid(0) ){
    randomnessPid = osGetpid(0);
    sqlite3_randomness(0,0);
  }
  memset(p, 0, ਆਕਾਰ(unixFile));

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ASSERT_NO_FILES
  /* Applications that never read or write a persistent disk files */
  ਪ੍ਰਤਿਜ੍ਞਾ( zName==0 );
#ਜੇ_ਅੰਤ

  ਜੇ( eType==SQLITE_OPEN_MAIN_DB ){
    Unix_ਅਣਵਰਤਿਆ_Fd *p_ਅਣਵਰਤਿਆ;
    p_ਅਣਵਰਤਿਆ = ਲੱਭੋ_Reusable_Fd(zName, flags);
    ਜੇ( p_ਅਣਵਰਤਿਆ ){
      fd = p_ਅਣਵਰਤਿਆ->fd;
    }ਨਹੀਂਤਾਂ{
      p_ਅਣਵਰਤਿਆ = sqlite3_malloc64(ਆਕਾਰ(*p_ਅਣਵਰਤਿਆ));
      ਜੇ( !p_ਅਣਵਰਤਿਆ ){
        ਵਾਪਸ SQLITE_NOMEM_BKPT;
      }
    }
    p->p_Preallocated_ਅਣਵਰਤਿਆ = p_ਅਣਵਰਤਿਆ;

    /* Database filenames are double-zero terminated if they are not
    ** URIs with parameters.  Hence, they can always be passed into
    ** sqlite3_uri_parameter(). */
    ਪ੍ਰਤਿਜ੍ਞਾ( (flags & SQLITE_OPEN_URI) || zName[ਲੜੀ_ਲੰਬਾਈ(zName)+1]==0 );

  }ਨਹੀਂਤਾਂ ਜੇ( !zName ){
    /* If zName is NULL, the upper layer is requesting a temp file. */
    ਪ੍ਰਤਿਜ੍ਞਾ( isDelete );
    ਪ੍ਰਤਿਜ੍ਞਾ( !isNewJrnl );
    ਪ੍ਰਤਿਜ੍ਞਾ( isExclusive );
    ਪ੍ਰਤਿਜ੍ਞਾ( isReadWrite );
#ਜੇ defined(__linux__) && defined(O_TMPFILE)
    /* On systems that support O_TMPFILE, use that flag to create a more
    ** secure temporary file that cannot be accessed by other processes
    */
    zName = unix_ਆਰਜ਼ੀ_File_Dir();
    ਜੇ( zName
     && (fd = robust_open(zName, O_RDWR|O_EXCL|O_TMPFILE, 0600))>=0
    ){
      rc = fill_ਵਿੱਚ_Unix_File(pVfs, fd, pFile, z_ਰਸਤਾ, ctrlFlags|UNIXFILE_NOLOCK);
      ਜਾਓ open_ਮੁਕੰਮਲ;
    }
#ਜੇ_ਅੰਤ
    rc = unixGetTempname(pVfs->mxPathname, zTmpname);
    ਜੇ( rc!=SQLITE_OK ){
      ਵਾਪਸ rc;
    }
    zName = zTmpname;

    /* Generated temporary filenames are always double-zero terminated
    ** for use by sqlite3_uri_parameter(). */
    ਪ੍ਰਤਿਜ੍ਞਾ( zName[ਲੜੀ_ਲੰਬਾਈ(zName)+1]==0 );
  }

  /* Determine the value of the flags parameter passed to POSIX function
  ** open(). These must be calculated even if open() is not called, as
  ** they may be stored as part of the file handle and used by the
  ** 'conch file' locking functions later on.  */
  ਜੇ( isReadonly )  openFlags |= O_RDONLY;
  ਜੇ( isReadWrite ) openFlags |= O_RDWR;
  ਜੇ( isCreate )    openFlags |= O_CREAT;
  ਜੇ( isExclusive ) openFlags |= (O_EXCL|O_NOFOLLOW);
  openFlags |= (O_LARGEFILE|O_ਦੁਆਧਾਰੀ|O_NOFOLLOW);

  ਜੇ( fd<0 ){
    ਬਹੁਲਕ_t open_ਬਹੁਲਕ;              /* Permissions to create file with */
    uid_t uid;                    /* Userid for the file */
    gid_t gid;                    /* Groupid for the file */
    rc = ਲੱਭੋ_Create_File_ਬਹੁਲਕ(zName, flags, &open_ਬਹੁਲਕ, &uid, &gid);
    ਜੇ( rc!=SQLITE_OK ){
      ਪ੍ਰਤਿਜ੍ਞਾ( !p->p_Preallocated_ਅਣਵਰਤਿਆ );
      ਪ੍ਰਤਿਜ੍ਞਾ( eType==SQLITE_OPEN_WAL || eType==SQLITE_OPEN_MAIN_JOURNAL );
      ਵਾਪਸ rc;
    }
    fd = robust_open(zName, openFlags, open_ਬਹੁਲਕ);
    OSTRACE(("OPENX   %-3d %s 0%o\n", fd, zName, openFlags));
    ਪ੍ਰਤਿਜ੍ਞਾ( !isExclusive || (openFlags & O_CREAT)!=0 );
    ਜੇ( fd<0 ){
      ਜੇ( isNewJrnl && errno==EACCES && osAccess(zName, F_OK) ){
        /* If unable to create a journal because the directory is not
        ** writable, change the error code to indicate that. */
        rc = SQLITE_READONLY_DIRECTORY;
      }ਨਹੀਂਤਾਂ ਜੇ( errno!=EISDIR && isReadWrite ){
        /* Failed to open the file for read/write access. Try read-only. */
        Unix_ਅਣਵਰਤਿਆ_Fd *pReadonly = 0;
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        openFlags &= ~(O_RDWR|O_CREAT);
        flags |= SQLITE_OPEN_READONLY;
        openFlags |= O_RDONLY;
        isReadonly = 1;
        pReadonly = ਲੱਭੋ_Reusable_Fd(zName, flags);
        ਜੇ( pReadonly ){
          fd = pReadonly->fd;
          sqlite3_free(pReadonly);
        }ਨਹੀਂਤਾਂ{
          fd = robust_open(zName, openFlags, open_ਬਹੁਲਕ);
        }
      }
    }
    ਜੇ( fd<0 ){
      ਪੂਰਨ rc2 = unixLogError(SQLITE_CANTOPEN_BKPT, "open", zName);
      ਜੇ( rc==SQLITE_OK ) rc = rc2;
      ਜਾਓ open_ਮੁਕੰਮਲ;
    }

    /* The owner of the rollback journal or WAL file should always be the
    ** same as the owner of the database file.  Try to ensure that this is
    ** the case.  The chown() system call will be a no-op if the current
    ** process lacks root privileges, be we should at least try.  Without
    ** this step, if a root process opens a database file, it can leave
    ** behinds a journal/WAL that is owned by root and hence make the
    ** database inaccessible to unprivileged processes.
    **
    ** If openMode==0, then that means uid and gid are not set correctly
    ** (probably because SQLite is configured to use 8+3 filename mode) and
    ** in that case we do not want to attempt the chown().
    */
    ਜੇ( open_ਬਹੁਲਕ && (flags & (SQLITE_OPEN_WAL|SQLITE_OPEN_MAIN_JOURNAL))!=0 ){
      robustFchown(fd, uid, gid);
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( fd>=0 );
  ਜੇ( p_ਬਾਹਰਲਾ_Flags ){
    *p_ਬਾਹਰਲਾ_Flags = flags;
  }

  ਜੇ( p->p_Preallocated_ਅਣਵਰਤਿਆ ){
    p->p_Preallocated_ਅਣਵਰਤਿਆ->fd = fd;
    p->p_Preallocated_ਅਣਵਰਤਿਆ->flags =
                          flags & (SQLITE_OPEN_READONLY|SQLITE_OPEN_READWRITE);
  }

  ਜੇ( isDelete ){
#ਜੇ OS_VXWORKS
    z_ਰਸਤਾ = zName;
#ਜਾਂਜੇ defined(SQLITE_UNLINK_AFTER_CLOSE)
    z_ਰਸਤਾ = sqlite3_mprintf("%s", zName);
    ਜੇ( z_ਰਸਤਾ==0 ){
      robust_close(p, fd, __LINE__);
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
#ਨਹੀਂਤਾਂ
    osUnlink(zName);
#ਜੇ_ਅੰਤ
  }
#ਜੇ SQLITE_ENABLE_LOCKING_STYLE
  ਨਹੀਂਤਾਂ{
    p->openFlags = openFlags;
  }
#ਜੇ_ਅੰਤ

#ਜੇ defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
  ਜੇ( fstatfs(fd, &fsInfo) == -1 ){
    store_ਆਖਰੀ_Errno(p, errno);
    robust_close(p, fd, __LINE__);
    ਵਾਪਸ SQLITE_IOERR_ACCESS;
  }
  ਜੇ (0 == ਲੜੀ_ਤੁਲਨਾ("msdos", fsInfo.f_fstypename, 5)) {
    ((unixFile*)pFile)->fsFlags |= SQLITE_FSFLAGS_IS_MSDOS;
  }
  ਜੇ (0 == ਲੜੀ_ਤੁਲਨਾ("exfat", fsInfo.f_fstypename, 5)) {
    ((unixFile*)pFile)->fsFlags |= SQLITE_FSFLAGS_IS_MSDOS;
  }
#ਜੇ_ਅੰਤ

  /* Set up appropriate ctrlFlags */
  ਜੇ( isDelete )                ctrlFlags |= UNIXFILE_DELETE;
  ਜੇ( isReadonly )              ctrlFlags |= UNIXFILE_RDONLY;
  no_ਤਾਲਾ = eType!=SQLITE_OPEN_MAIN_DB;
  ਜੇ( no_ਤਾਲਾ )                  ctrlFlags |= UNIXFILE_NOLOCK;
  ਜੇ( isNewJrnl )               ctrlFlags |= UNIXFILE_DIRSYNC;
  ਜੇ( flags & SQLITE_OPEN_URI ) ctrlFlags |= UNIXFILE_URI;

#ਜੇ SQLITE_ENABLE_LOCKING_STYLE
#ਜੇ SQLITE_PREFER_PROXY_LOCKING
  isAutoProxy = 1;
#ਜੇ_ਅੰਤ
  ਜੇ( isAutoProxy && (z_ਰਸਤਾ!=ਨਿਰਾ) && (!no_ਤਾਲਾ) && pVfs->xOpen ){
    ਅੱਖਰ *envforce = getenv("SQLITE_FORCE_PROXY_LOCKING");
    ਪੂਰਨ useProxy = 0;

    /* SQLITE_FORCE_PROXY_LOCKING==1 means force always use proxy, 0 means
    ** never use proxy, NULL means use proxy for non-local files only.  */
    ਜੇ( envforce!=ਨਿਰਾ ){
      useProxy = atoi(envforce)>0;
    }ਨਹੀਂਤਾਂ{
      useProxy = !(fsInfo.f_flags&MNT_LOCAL);
    }
    ਜੇ( useProxy ){
      rc = fill_ਵਿੱਚ_Unix_File(pVfs, fd, pFile, z_ਰਸਤਾ, ctrlFlags);
      ਜੇ( rc==SQLITE_OK ){
        rc = proxyTransformUnixFile((unixFile*)pFile, ":auto:");
        ਜੇ( rc!=SQLITE_OK ){
          /* Use unixClose to clean up the resources added in fillInUnixFile
          ** and clear all the structure's references.  Specifically,
          ** pFile->pMethods will be NULL so sqlite3OsClose will be a no-op
          */
          unixClose(pFile);
          ਵਾਪਸ rc;
        }
      }
      ਜਾਓ open_ਮੁਕੰਮਲ;
    }
  }
#ਜੇ_ਅੰਤ

  ਪ੍ਰਤਿਜ੍ਞਾ( z_ਰਸਤਾ==0
       || z_ਰਸਤਾ[0]=='/'
       || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_MAIN_JOURNAL
       || eType==SQLITE_OPEN_ਆਰਜ਼ੀ_JOURNAL
  );
  rc = fill_ਵਿੱਚ_Unix_File(pVfs, fd, pFile, z_ਰਸਤਾ, ctrlFlags);

open_ਮੁਕੰਮਲ:
  ਜੇ( rc!=SQLITE_OK ){
    sqlite3_free(p->p_Preallocated_ਅਣਵਰਤਿਆ);
  }
  ਵਾਪਸ rc;
}


/*
** Delete the file at zPath. If the dirSync argument is true, fsync()
** the directory after deleting the file.
*/
ਸਥਿਰ ਪੂਰਨ unixDelete(
  sqlite3_vfs *Not_ਵਰਤਿਆ,     /* VFS containing this as the xDelete method */
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,        /* Name of file to be deleted */
  ਪੂਰਨ dirSync               /* If true, fsync() directory after deleting file */
){
  ਪੂਰਨ rc = SQLITE_OK;
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  SimulateIOError(ਵਾਪਸ SQLITE_IOERR_DELETE);
  ਜੇ( osUnlink(z_ਰਸਤਾ)==(-1) ){
    ਜੇ( errno==ENOENT
#ਜੇ OS_VXWORKS
        || osAccess(z_ਰਸਤਾ,0)!=0
#ਜੇ_ਅੰਤ
    ){
      rc = SQLITE_IOERR_DELETE_NOENT;
    }ਨਹੀਂਤਾਂ{
      rc = unixLogError(SQLITE_IOERR_DELETE, "unlink", z_ਰਸਤਾ);
    }
    ਵਾਪਸ rc;
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DISABLE_DIRSYNC
  ਜੇ( (dirSync & 1)!=0 ){
    ਪੂਰਨ fd;
    rc = osOpenDirectory(z_ਰਸਤਾ, &fd);
    ਜੇ( rc==SQLITE_OK ){
      ਜੇ( ਭਰਿਆ_fsync(fd,0,0) ){
        rc = unixLogError(SQLITE_IOERR_DIR_FSYNC, "fsync", z_ਰਸਤਾ);
      }
      robust_close(0, fd, __LINE__);
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_CANTOPEN );
      rc = SQLITE_OK;
    }
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ rc;
}

/*
** Test the existence of or access permissions of file zPath. The
** test performed depends on the value of flags:
**
**     SQLITE_ACCESS_EXISTS: Return 1 if the file exists
**     SQLITE_ACCESS_READWRITE: Return 1 if the file is read and writable.
**     SQLITE_ACCESS_READONLY: Return 1 if the file is readable.
**
** Otherwise return 0.
*/
ਸਥਿਰ ਪੂਰਨ unixAccess(
  sqlite3_vfs *Not_ਵਰਤਿਆ,   /* The VFS containing this xAccess method */
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,      /* Path of the file to examine */
  ਪੂਰਨ flags,              /* What do we want to learn about the zPath file? */
  ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ            /* Write result boolean here */
){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  SimulateIOError( ਵਾਪਸ SQLITE_IOERR_ACCESS; );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Res_ਬਾਹਰਲਾ!=0 );

  /* The spec says there are three possible values for flags.  But only
  ** two of them are actually used */
  ਪ੍ਰਤਿਜ੍ਞਾ( flags==SQLITE_ACCESS_EXISTS || flags==SQLITE_ACCESS_READWRITE );

  ਜੇ( flags==SQLITE_ACCESS_EXISTS ){
    ਬਣਤਰ stat buf;
    *p_Res_ਬਾਹਰਲਾ = 0==osStat(z_ਰਸਤਾ, &buf) &&
                (!S_ISREG(buf.st_ਬਹੁਲਕ) || buf.st_ਮਾਪ>0);
  }ਨਹੀਂਤਾਂ{
    *p_Res_ਬਾਹਰਲਾ = osAccess(z_ਰਸਤਾ, W_OK|R_OK)==0;
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** A pathname under construction
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Db_ਰਸਤਾ Db_ਰਸਤਾ;
ਬਣਤਰ Db_ਰਸਤਾ {
  ਪੂਰਨ rc;           /* Non-zero following any error */
  ਪੂਰਨ nSymlink;     /* Number of symlinks resolved */
  ਅੱਖਰ *z_ਬਾਹਰਲਾ;       /* Write the pathname here */
  ਪੂਰਨ n_ਬਾਹਰਲਾ;         /* Bytes of space available to zOut[] */
  ਪੂਰਨ n_ਵਰਤਿਆ;        /* Bytes of zOut[] currently being used */
};

/* Forward reference */
ਸਥਿਰ ਖਾਲੀ append_All_ਰਸਤਾ_Elements(Db_ਰਸਤਾ*,ਅਚਲ ਅੱਖਰ*);

/*
** Append a single path element to the DbPath under construction
*/
ਸਥਿਰ ਖਾਲੀ append_One_ਰਸਤਾ_Element(
  Db_ਰਸਤਾ *p_ਰਸਤਾ,       /* Path under construction, to which to append zName */
  ਅਚਲ ਅੱਖਰ *zName,   /* Name to append to pPath.  Not zero-terminated */
  ਪੂਰਨ nName            /* Number of significant bytes in zName */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( nName>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( zName!=0 );
  ਜੇ( zName[0]=='.' ){
    ਜੇ( nName==1 ) ਵਾਪਸ;
    ਜੇ( zName[1]=='.' && nName==2 ){
      ਜੇ( p_ਰਸਤਾ->n_ਵਰਤਿਆ>1 ){
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਰਸਤਾ->z_ਬਾਹਰਲਾ[0]=='/' );
        ਜਦੋਂ( p_ਰਸਤਾ->z_ਬਾਹਰਲਾ[--p_ਰਸਤਾ->n_ਵਰਤਿਆ]!='/' ){}
      }
      ਵਾਪਸ;
    }
  }
  ਜੇ( p_ਰਸਤਾ->n_ਵਰਤਿਆ + nName + 2 >= p_ਰਸਤਾ->n_ਬਾਹਰਲਾ ){
    p_ਰਸਤਾ->rc = SQLITE_ERROR;
    ਵਾਪਸ;
  }
  p_ਰਸਤਾ->z_ਬਾਹਰਲਾ[p_ਰਸਤਾ->n_ਵਰਤਿਆ++] = '/';
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&p_ਰਸਤਾ->z_ਬਾਹਰਲਾ[p_ਰਸਤਾ->n_ਵਰਤਿਆ], zName, nName);
  p_ਰਸਤਾ->n_ਵਰਤਿਆ += nName;
#ਜੇ defined(HAVE_READLINK) && defined(HAVE_LSTAT)
  ਜੇ( p_ਰਸਤਾ->rc==SQLITE_OK ){
    ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ;
    ਬਣਤਰ stat buf;
    p_ਰਸਤਾ->z_ਬਾਹਰਲਾ[p_ਰਸਤਾ->n_ਵਰਤਿਆ] = 0;
    z_ਵਿੱਚ = p_ਰਸਤਾ->z_ਬਾਹਰਲਾ;
    ਜੇ( osLstat(z_ਵਿੱਚ, &buf)!=0 ){
      ਜੇ( errno!=ENOENT ){
        p_ਰਸਤਾ->rc = unixLogError(SQLITE_CANTOPEN_BKPT, "lstat", z_ਵਿੱਚ);
      }
    }ਨਹੀਂਤਾਂ ਜੇ( S_ISLNK(buf.st_ਬਹੁਲਕ) ){
      ssize_t got;
      ਅੱਖਰ zLnk[SQLITE_MAX_PATHLEN+2];
      ਜੇ( p_ਰਸਤਾ->nSymlink++ > SQLITE_MAX_SYMLINK ){
        p_ਰਸਤਾ->rc = SQLITE_CANTOPEN_BKPT;
        ਵਾਪਸ;
      }
      got = osReadlink(z_ਵਿੱਚ, zLnk, ਆਕਾਰ(zLnk)-2);
      ਜੇ( got<=0 || got>=(ssize_t)ਆਕਾਰ(zLnk)-2 ){
        p_ਰਸਤਾ->rc = unixLogError(SQLITE_CANTOPEN_BKPT, "readlink", z_ਵਿੱਚ);
        ਵਾਪਸ;
      }
      zLnk[got] = 0;
      ਜੇ( zLnk[0]=='/' ){
        p_ਰਸਤਾ->n_ਵਰਤਿਆ = 0;
      }ਨਹੀਂਤਾਂ{
        p_ਰਸਤਾ->n_ਵਰਤਿਆ -= nName + 1;
      }
      append_All_ਰਸਤਾ_Elements(p_ਰਸਤਾ, zLnk);
    }
  }
#ਜੇ_ਅੰਤ
}

/*
** Append all path elements in zPath to the DbPath under construction.
*/
ਸਥਿਰ ਖਾਲੀ append_All_ਰਸਤਾ_Elements(
  Db_ਰਸਤਾ *p_ਰਸਤਾ,       /* Path under construction, to which to append zName */
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ    /* Path to append to pPath.  Is zero-terminated */
){
  ਪੂਰਨ i = 0;
  ਪੂਰਨ j = 0;
  ਕਰੋ{
    ਜਦੋਂ( z_ਰਸਤਾ[i] && z_ਰਸਤਾ[i]!='/' ){ i++; }
    ਜੇ( i>j ){
      append_One_ਰਸਤਾ_Element(p_ਰਸਤਾ, &z_ਰਸਤਾ[j], i-j);
    }
    j = i+1;
  }ਜਦੋਂ( z_ਰਸਤਾ[i++] );
}

/*
** Turn a relative pathname into a full pathname. The relative path
** is stored as a nul-terminated string in the buffer pointed to by
** zPath.
**
** zOut points to a buffer of at least sqlite3_vfs.mxPathname bytes
** (in this case, MAX_PATHNAME bytes). The full-path is written to
** this buffer before returning.
*/
ਸਥਿਰ ਪੂਰਨ unix_ਭਰਿਆ_Pathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,            /* Possibly relative input path */
  ਪੂਰਨ n_ਬਾਹਰਲਾ,                     /* Size of output buffer in bytes */
  ਅੱਖਰ *z_ਬਾਹਰਲਾ                    /* Output buffer */
){
  Db_ਰਸਤਾ ਰਸਤਾ;
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);
  ਰਸਤਾ.rc = 0;
  ਰਸਤਾ.n_ਵਰਤਿਆ = 0;
  ਰਸਤਾ.nSymlink = 0;
  ਰਸਤਾ.n_ਬਾਹਰਲਾ = n_ਬਾਹਰਲਾ;
  ਰਸਤਾ.z_ਬਾਹਰਲਾ = z_ਬਾਹਰਲਾ;
  ਜੇ( z_ਰਸਤਾ[0]!='/' ){
    ਅੱਖਰ zPwd[SQLITE_MAX_PATHLEN+2];
    ਜੇ( osGetcwd(zPwd, ਆਕਾਰ(zPwd)-2)==0 ){
      ਵਾਪਸ unixLogError(SQLITE_CANTOPEN_BKPT, "getcwd", z_ਰਸਤਾ);
    }
    append_All_ਰਸਤਾ_Elements(&ਰਸਤਾ, zPwd);
  }
  append_All_ਰਸਤਾ_Elements(&ਰਸਤਾ, z_ਰਸਤਾ);
  z_ਬਾਹਰਲਾ[ਰਸਤਾ.n_ਵਰਤਿਆ] = 0;
  ਜੇ( ਰਸਤਾ.rc || ਰਸਤਾ.n_ਵਰਤਿਆ<2 ) ਵਾਪਸ SQLITE_CANTOPEN_BKPT;
  ਜੇ( ਰਸਤਾ.nSymlink ) ਵਾਪਸ SQLITE_OK_SYMLINK;
  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ <dlfcn.h>
ਸਥਿਰ ਖਾਲੀ *unixDlOpen(sqlite3_vfs *Not_ਵਰਤਿਆ, ਅਚਲ ਅੱਖਰ *zFilename){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਵਾਪਸ dlopen(zFilename, RTLD_NOW | RTLD_GLOBAL);
}

/*
** SQLite calls this function immediately after a call to unixDlSym() or
** unixDlOpen() fails (returns a null pointer). If a more detailed error
** message is available, it is written to zBufOut. If no error message
** is available, zBufOut is left unmodified and SQLite uses a default
** error message.
*/
ਸਥਿਰ ਖਾਲੀ unixDlError(sqlite3_vfs *Not_ਵਰਤਿਆ, ਪੂਰਨ nBuf, ਅੱਖਰ *z_Buf_ਬਾਹਰਲਾ){
  ਅਚਲ ਅੱਖਰ *zErr;
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  unixEnterMutex();
  zErr = dlerror();
  ਜੇ( zErr ){
    sqlite3_snprintf(nBuf, z_Buf_ਬਾਹਰਲਾ, "%s", zErr);
  }
  unixLeaveMutex();
}
ਸਥਿਰ ਖਾਲੀ (*unixDlSym(sqlite3_vfs *Not_ਵਰਤਿਆ, ਖਾਲੀ *p, ਅਚਲ ਅੱਖਰ*zSym))(ਖਾਲੀ){
  /*
  ** GCC with -pedantic-errors says that C90 does not allow a void* to be
  ** cast into a pointer to a function.  And yet the library dlsym() routine
  ** returns a void* which is really a pointer to a function.  So how do we
  ** use dlsym() with -pedantic-errors?
  **
  ** Variable x below is defined to be a pointer to a function taking
  ** parameters void* and const char* and returning a pointer to a function.
  ** We initialize x by assigning it a pointer to the dlsym() function.
  ** (That assignment requires a cast.)  Then we call the function that
  ** x points to.
  **
  ** This work-around is unlikely to work correctly on any system where
  ** you really cannot cast a function pointer into void*.  But then, on the
  ** other hand, dlsym() will not work on such a system either, so we have
  ** not really lost anything.
  */
  ਖਾਲੀ (*(*x)(ਖਾਲੀ*,ਅਚਲ ਅੱਖਰ*))(ਖਾਲੀ);
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  x = (ਖਾਲੀ(*(*)(ਖਾਲੀ*,ਅਚਲ ਅੱਖਰ*))(ਖਾਲੀ))dlsym;
  ਵਾਪਸ (*x)(p, zSym);
}
ਸਥਿਰ ਖਾਲੀ unixDlClose(sqlite3_vfs *Not_ਵਰਤਿਆ, ਖਾਲੀ *pHandle){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  dlclose(pHandle);
}
#ਨਹੀਂਤਾਂ /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unixDlOpen  0
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unixDlError 0
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unixDlSym   0
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unixDlClose 0
#ਜੇ_ਅੰਤ

/*
** Write nBuf bytes of random data to the supplied buffer zBuf.
*/
ਸਥਿਰ ਪੂਰਨ unixRandomness(sqlite3_vfs *Not_ਵਰਤਿਆ, ਪੂਰਨ nBuf, ਅੱਖਰ *zBuf){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਪ੍ਰਤਿਜ੍ਞਾ((size_t)nBuf>=(ਆਕਾਰ(ਸਮਾਂ_t)+ਆਕਾਰ(ਪੂਰਨ)));

  /* We have to initialize zBuf to prevent valgrind from reporting
  ** errors.  The reports issued by valgrind are incorrect - we would
  ** prefer that the randomness be increased by making use of the
  ** uninitialized space in zBuf - but valgrind errors tend to worry
  ** some users.  Rather than argue, it seems easier just to initialize
  ** the whole array and silence valgrind, even if that means less randomness
  ** in the random seed.
  **
  ** When testing, initializing zBuf[] to zero is all we do.  That means
  ** that we always use the same random number sequence.  This makes the
  ** tests repeatable.
  */
  memset(zBuf, 0, nBuf);
  randomnessPid = osGetpid(0);
#ਜੇ !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
  {
    ਪੂਰਨ fd, got;
    fd = robust_open("/dev/urandom", O_RDONLY, 0);
    ਜੇ( fd<0 ){
      ਸਮਾਂ_t t;
      ਸਮਾਂ(&t);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(zBuf, &t, ਆਕਾਰ(t));
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&zBuf[ਆਕਾਰ(t)], &randomnessPid, ਆਕਾਰ(randomnessPid));
      ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(t)+ਆਕਾਰ(randomnessPid)<=(size_t)nBuf );
      nBuf = ਆਕਾਰ(t) + ਆਕਾਰ(randomnessPid);
    }ਨਹੀਂਤਾਂ{
      ਕਰੋ{ got = osRead(fd, zBuf, nBuf); }ਜਦੋਂ( got<0 && errno==EINTR );
      robust_close(0, fd, __LINE__);
    }
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ nBuf;
}


/*
** Sleep for a little while.  Return the amount of time slept.
** The argument is the number of microseconds we want to sleep.
** The return value is the number of microseconds of sleep actually
** requested from the underlying operating system, a number which
** might be greater than or equal to the argument, but not less
** than the argument.
*/
ਸਥਿਰ ਪੂਰਨ unixSleep(sqlite3_vfs *Not_ਵਰਤਿਆ, ਪੂਰਨ microseconds){
#ਜੇ !defined(HAVE_NANOSLEEP) || HAVE_NANOSLEEP+0
  ਬਣਤਰ timespec sp;
  sp.tv_sec = microseconds / 1000000;
  sp.tv_nsec = (microseconds % 1000000) * 1000;

  /* Almost all modern unix systems support nanosleep().  But if you are
  ** compiling for one of the rare exceptions, you can use
  ** -DHAVE_NANOSLEEP=0 (perhaps in conjunction with -DHAVE_USLEEP if
  ** usleep() is available) in order to bypass the use of nanosleep() */
  nanosleep(&sp, ਨਿਰਾ);

  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਵਾਪਸ microseconds;
#ਜਾਂਜੇ defined(HAVE_USLEEP) && HAVE_USLEEP
  ਜੇ( microseconds>=1000000 ) sleep(microseconds/1000000);
  ਜੇ( microseconds%1000000 ) usleep(microseconds%1000000);
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਵਾਪਸ microseconds;
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ seconds = (microseconds+999999)/1000000;
  sleep(seconds);
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਵਾਪਸ seconds*1000000;
#ਜੇ_ਅੰਤ
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
ਸਥਿਰ ਪੂਰਨ unix_Current_ਸਮਾਂ_Int64(sqlite3_vfs *Not_ਵਰਤਿਆ, sqlite3_int64 *piNow){
  ਸਥਿਰ ਅਚਲ sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  ਪੂਰਨ rc = SQLITE_OK;
#ਜੇ defined(NO_GETTOD)
  ਸਮਾਂ_t t;
  ਸਮਾਂ(&t);
  *piNow = ((sqlite3_int64)t)*1000 + unixEpoch;
#ਜਾਂਜੇ OS_VXWORKS
  ਬਣਤਰ timespec sNow;
  clock_gettime(CLOCK_REALTIME, &sNow);
  *piNow = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_nsec/1000000;
#ਨਹੀਂਤਾਂ
  ਬਣਤਰ timeval sNow;
  (ਖਾਲੀ)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *piNow = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  ਜੇ( sqlite3_current_ਸਮਾਂ ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_ਸਮਾਂ + unixEpoch;
  }
#ਜੇ_ਅੰਤ
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਵਾਪਸ rc;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DEPRECATED
/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
ਸਥਿਰ ਪੂਰਨ unix_Current_ਸਮਾਂ(sqlite3_vfs *Not_ਵਰਤਿਆ, ਦੁੱਗਣਾ *prNow){
  sqlite3_int64 i = 0;
  ਪੂਰਨ rc;
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  rc = unix_Current_ਸਮਾਂ_Int64(0, &i);
  *prNow = i/86400000.0;
  ਵਾਪਸ rc;
}
#ਨਹੀਂਤਾਂ
# define unix_Current_ਸਮਾਂ 0
#ਜੇ_ਅੰਤ

/*
** The xGetLastError() method is designed to return a better
** low-level error message when operating-system problems come up
** during SQLite operation.  Only the integer return code is currently
** used.
*/
ਸਥਿਰ ਪੂਰਨ unix_Get_ਆਖਰੀ_Error(sqlite3_vfs *Not_ਵਰਤਿਆ, ਪੂਰਨ NotUsed2, ਅੱਖਰ *NotUsed3){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਅਣਵਰਤਿਆ_PARAMETER(NotUsed2);
  ਅਣਵਰਤਿਆ_PARAMETER(NotUsed3);
  ਵਾਪਸ errno;
}


/*
************************ End of sqlite3_vfs methods ***************************
******************************************************************************/

/******************************************************************************
************************** Begin Proxy Locking ********************************
**
** Proxy locking is a "uber-locking-method" in this sense:  It uses the
** other locking methods on secondary lock files.  Proxy locking is a
** meta-layer over top of the primitive locking implemented above.  For
** this reason, the division that implements of proxy locking is deferred
** until late in the file (here) after all of the other I/O methods have
** been defined - so that the primitive locking methods are available
** as services to help with the implementation of proxy locking.
**
****
**
** The default locking schemes in SQLite use byte-range locks on the
** database file to coordinate safe, concurrent access by multiple readers
** and writers [http://sqlite.org/lockingv3.html].  The five file locking
** states (UNLOCKED, PENDING, SHARED, RESERVED, EXCLUSIVE) are implemented
** as POSIX read & write locks over fixed set of locations (via fsctl),
** on AFP and SMB only exclusive byte-range locks are available via fsctl
** with _IOWR('z', 23, struct ByteRangeLockPB2) to track the same 5 states.
** To simulate a F_RDLCK on the shared range, on AFP a randomly selected
** address in the shared range is taken for a SHARED lock, the entire
** shared range is taken for an EXCLUSIVE lock):
**
**      PENDING_BYTE        0x40000000
**      RESERVED_BYTE       0x40000001
**      SHARED_RANGE        0x40000002 -> 0x40000200
**
** This works well on the local file system, but shows a nearly 100x
** slowdown in read performance on AFP because the AFP client disables
** the read cache when byte-range locks are present.  Enabling the read
** cache exposes a cache coherency problem that is present on all OS X
** supported network file systems.  NFS and AFP both observe the
** close-to-open semantics for ensuring cache coherency
** [http://nfs.sourceforge.net/#faq_a8], which does not effectively
** address the requirements for concurrent database access by multiple
** readers and writers
** [http://www.nabble.com/SQLite-on-NFS-cache-coherency-td15655701.html].
**
** To address the performance and cache coherency issues, proxy file locking
** changes the way database access is controlled by limiting access to a
** single host at a time and moving file locks off of the database file
** and onto a proxy file on the local file system.
**
**
** Using proxy locks
** -----------------
**
** C APIs
**
**  sqlite3_file_control(db, dbname, SQLITE_FCNTL_SET_LOCKPROXYFILE,
**                       <proxy_path> | ":auto:");
**  sqlite3_file_control(db, dbname, SQLITE_FCNTL_GET_LOCKPROXYFILE,
**                       &<proxy_path>);
**
**
** SQL pragmas
**
**  PRAGMA [database.]lock_proxy_file=<proxy_path> | :auto:
**  PRAGMA [database.]lock_proxy_file
**
** Specifying ":auto:" means that if there is a conch file with a matching
** host ID in it, the proxy path in the conch file will be used, otherwise
** a proxy path based on the user's temp dir
** (via confstr(_CS_DARWIN_USER_TEMP_DIR,...)) will be used and the
** actual proxy file name is generated from the name and path of the
** database file.  For example:
**
**       For database path "/Users/me/foo.db"
**       The lock path will be "<tmpdir>/sqliteplocks/_Users_me_foo.db:auto:")
**
** Once a lock proxy is configured for a database connection, it can not
** be removed, however it may be switched to a different proxy path via
** the above APIs (assuming the conch file is not being held by another
** connection or process).
**
**
** How proxy locking works
** -----------------------
**
** Proxy file locking relies primarily on two new supporting files:
**
**   *  conch file to limit access to the database file to a single host
**      at a time
**
**   *  proxy file to act as a proxy for the advisory locks normally
**      taken on the database
**
** The conch file - to use a proxy file, sqlite must first "hold the conch"
** by taking an sqlite-style shared lock on the conch file, reading the
** contents and comparing the host's unique host ID (see below) and lock
** proxy path against the values stored in the conch.  The conch file is
** stored in the same directory as the database file and the file name
** is patterned after the database file name as ".<databasename>-conch".
** If the conch file does not exist, or its contents do not match the
** host ID and/or proxy path, then the lock is escalated to an exclusive
** lock and the conch file contents is updated with the host ID and proxy
** path and the lock is downgraded to a shared lock again.  If the conch
** is held by another process (with a shared lock), the exclusive lock
** will fail and SQLITE_BUSY is returned.
**
** The proxy file - a single-byte file used for all advisory file locks
** normally taken on the database file.   This allows for safe sharing
** of the database file for multiple readers and writers on the same
** host (the conch ensures that they all use the same local lock file).
**
** Requesting the lock proxy does not immediately take the conch, it is
** only taken when the first request to lock database file is made.
** This matches the semantics of the traditional locking behavior, where
** opening a connection to a database file does not take a lock on it.
** The shared lock and an open file descriptor are maintained until
** the connection to the database is closed.
**
** The proxy file and the lock file are never deleted so they only need
** to be created the first time they are used.
**
** Configuration options
** ---------------------
**
**  SQLITE_PREFER_PROXY_LOCKING
**
**       Database files accessed on non-local file systems are
**       automatically configured for proxy locking, lock files are
**       named automatically using the same logic as
**       PRAGMA lock_proxy_file=":auto:"
**  
**  SQLITE_PROXY_DEBUG
**
**       Enables the logging of error messages during host id file
**       retrieval and creation
**
**  LOCKPROXYDIR
**
**       Overrides the default directory used for lock proxy files that
**       are named automatically via the ":auto:" setting
**
**  SQLITE_DEFAULT_PROXYDIR_PERMISSIONS
**
**       Permissions to use when creating a directory for storing the
**       lock proxy files, only used when LOCKPROXYDIR is not set.
**  
**  
** As mentioned above, when compiled with SQLITE_PREFER_PROXY_LOCKING,
** setting the environment variable SQLITE_FORCE_PROXY_LOCKING to 1 will
** force proxy locking to be used for every database file opened, and 0
** will force automatic proxy locking to be disabled for all database
** files (explicitly calling the SQLITE_FCNTL_SET_LOCKPROXYFILE pragma or
** sqlite_file_control API is not affected by SQLITE_FORCE_PROXY_LOCKING).
*/

/*
** Proxy locking is only available on MacOSX
*/
#ਜੇ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE

/*
** The proxyLockingContext has the path and file structures for the remote
** and local proxy files in it
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ proxyLockingContext proxyLockingContext;
ਬਣਤਰ proxyLockingContext {
  unixFile *conchFile;         /* Open conch file */
  ਅੱਖਰ *conch_File_ਰਸਤਾ;         /* Name of the conch file */
  unixFile *ਤਾਲਾ_Proxy;         /* Open proxy lock file */
  ਅੱਖਰ *ਤਾਲਾ_Proxy_ਰਸਤਾ;         /* Name of the proxy lock file */
  ਅੱਖਰ *db_ਰਸਤਾ;                /* Name of the open file */
  ਪੂਰਨ conchHeld;               /* 1 if the conch is held, -1 if lockless */
  ਪੂਰਨ nFails;                  /* Number of conch taking failures */
  ਖਾਲੀ *oldLockingContext;     /* Original lockingcontext to restore on close */
  sqlite3_io_methods ਅਚਲ *p_Old_ਵਿਧੀ;     /* Original I/O methods for close */
};

/*
** The proxy lock file path for the database at dbPath is written into lPath,
** which must point to valid, writable memory large enough for a maxLen length
** file path.
*/
ਸਥਿਰ ਪੂਰਨ proxy_Get_ਤਾਲਾ_ਰਸਤਾ(ਅਚਲ ਅੱਖਰ *db_ਰਸਤਾ, ਅੱਖਰ *l_ਰਸਤਾ, size_t maxLen){
  ਪੂਰਨ len;
  ਪੂਰਨ dbLen;
  ਪੂਰਨ i;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LOCKPROXYDIR
  len = strlcpy(l_ਰਸਤਾ, LOCKPROXYDIR, maxLen);
#ਨਹੀਂਤਾਂ
# ifdef CS_DARWIN_USER_ਆਰਜ਼ੀ_DIR
  {
    ਜੇ( !confstr(CS_DARWIN_USER_ਆਰਜ਼ੀ_DIR, l_ਰਸਤਾ, maxLen) ){
      OSTRACE(("GETLOCKPATH  failed %s errno=%d pid=%d\n",
               l_ਰਸਤਾ, errno, osGetpid(0)));
      ਵਾਪਸ SQLITE_IOERR_ਤਾਲਾ;
    }
    len = strlcat(l_ਰਸਤਾ, "sqliteplocks", maxLen);  
  }
# ਨਹੀਂਤਾਂ
  len = strlcpy(l_ਰਸਤਾ, "/tmp/", maxLen);
# endif
#ਜੇ_ਅੰਤ

  ਜੇ( l_ਰਸਤਾ[len-1]!='/' ){
    len = strlcat(l_ਰਸਤਾ, "/", maxLen);
  }

  /* transform the db path to a unique cache name */
  dbLen = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(db_ਰਸਤਾ);
  ਲਈ( i=0; i<dbLen && (i+len+7)<(ਪੂਰਨ)maxLen; i++){
    ਅੱਖਰ c = db_ਰਸਤਾ[i];
    l_ਰਸਤਾ[i+len] = (c=='/')?'_':c;
  }
  l_ਰਸਤਾ[i+len]='\0';
  strlcat(l_ਰਸਤਾ, ":auto:", maxLen);
  OSTRACE(("GETLOCKPATH  proxy lock path=%s pid=%d\n", l_ਰਸਤਾ, osGetpid(0)));
  ਵਾਪਸ SQLITE_OK;
}

/*
 ** Creates the lock file and any missing directories in lockPath
 */
ਸਥਿਰ ਪੂਰਨ proxy_Create_ਤਾਲਾ_ਰਸਤਾ(ਅਚਲ ਅੱਖਰ *ਤਾਲਾ_ਰਸਤਾ){
  ਪੂਰਨ i, len;
  ਅੱਖਰ buf[MAXPATHLEN];
  ਪੂਰਨ ਸ਼ੁਰੂ = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ(ਤਾਲਾ_ਰਸਤਾ!=ਨਿਰਾ);
  /* try to create all the intermediate directories */
  len = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(ਤਾਲਾ_ਰਸਤਾ);
  buf[0] = ਤਾਲਾ_ਰਸਤਾ[0];
  ਲਈ( i=1; i<len; i++ ){
    ਜੇ( ਤਾਲਾ_ਰਸਤਾ[i] == '/' && (i - ਸ਼ੁਰੂ > 0) ){
      /* only mkdir if leaf dir != "." or "/" or ".." */
      ਜੇ( i-ਸ਼ੁਰੂ>2 || (i-ਸ਼ੁਰੂ==1 && buf[ਸ਼ੁਰੂ] != '.' && buf[ਸ਼ੁਰੂ] != '/')
         || (i-ਸ਼ੁਰੂ==2 && buf[ਸ਼ੁਰੂ] != '.' && buf[ਸ਼ੁਰੂ+1] != '.') ){
        buf[i]='\0';
        ਜੇ( osMkdir(buf, SQLITE_DEFAULT_PROXYDIR_PERMISSIONS) ){
          ਪੂਰਨ err=errno;
          ਜੇ( err!=EEXIST ) {
            OSTRACE(("CREATELOCKPATH  FAILED creating %s, "
                     "'%s' proxy lock path=%s pid=%d\n",
                     buf, strerror(err), ਤਾਲਾ_ਰਸਤਾ, osGetpid(0)));
            ਵਾਪਸ err;
          }
        }
      }
      ਸ਼ੁਰੂ=i+1;
    }
    buf[i] = ਤਾਲਾ_ਰਸਤਾ[i];
  }
  OSTRACE(("CREATELOCKPATH  proxy lock path=%s pid=%d\n",ਤਾਲਾ_ਰਸਤਾ,osGetpid(0)));
  ਵਾਪਸ 0;
}

/*
** Create a new VFS file descriptor (stored in memory obtained from
** sqlite3_malloc) and open the file named "path" in the file descriptor.
**
** The caller is responsible not only for closing the file descriptor
** but also for freeing the memory associated with the file descriptor.
*/
ਸਥਿਰ ਪੂਰਨ proxyCreateUnixFile(
    ਅਚਲ ਅੱਖਰ *ਰਸਤਾ,        /* path for the new unixFile */
    unixFile **ppFile,       /* unixFile created and returned by ref */
    ਪੂਰਨ islockfile           /* if non zero missing dirs will be created */
) {
  ਪੂਰਨ fd = -1;
  unixFile *pNew;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ openFlags = O_RDWR | O_CREAT | O_NOFOLLOW;
  sqlite3_vfs dummyVfs;
  ਪੂਰਨ terrno = 0;
  Unix_ਅਣਵਰਤਿਆ_Fd *p_ਅਣਵਰਤਿਆ = ਨਿਰਾ;

  /* 1. first try to open/create the file
  ** 2. if that fails, and this is a lock file (not-conch), try creating
  ** the parent directories and then try again.
  ** 3. if that fails, try to open the file read-only
  ** otherwise return BUSY (if lock file) or CANTOPEN for the conch file
  */
  p_ਅਣਵਰਤਿਆ = ਲੱਭੋ_Reusable_Fd(ਰਸਤਾ, openFlags);
  ਜੇ( p_ਅਣਵਰਤਿਆ ){
    fd = p_ਅਣਵਰਤਿਆ->fd;
  }ਨਹੀਂਤਾਂ{
    p_ਅਣਵਰਤਿਆ = sqlite3_malloc64(ਆਕਾਰ(*p_ਅਣਵਰਤਿਆ));
    ਜੇ( !p_ਅਣਵਰਤਿਆ ){
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
  }
  ਜੇ( fd<0 ){
    fd = robust_open(ਰਸਤਾ, openFlags, 0);
    terrno = errno;
    ਜੇ( fd<0 && errno==ENOENT && islockfile ){
      ਜੇ( proxy_Create_ਤਾਲਾ_ਰਸਤਾ(ਰਸਤਾ) == SQLITE_OK ){
        fd = robust_open(ਰਸਤਾ, openFlags, 0);
      }
    }
  }
  ਜੇ( fd<0 ){
    openFlags = O_RDONLY | O_NOFOLLOW;
    fd = robust_open(ਰਸਤਾ, openFlags, 0);
    terrno = errno;
  }
  ਜੇ( fd<0 ){
    ਜੇ( islockfile ){
      ਵਾਪਸ SQLITE_BUSY;
    }
    ਚੋਣ (terrno) {
      ਹਾਲਤ EACCES:
        ਵਾਪਸ SQLITE_PERM;
      ਹਾਲਤ EIO:
        ਵਾਪਸ SQLITE_IOERR_ਤਾਲਾ; /* even though it is the conch */
      ਮੂਲ:
        ਵਾਪਸ SQLITE_CANTOPEN_BKPT;
    }
  }

  pNew = (unixFile *)sqlite3_malloc64(ਆਕਾਰ(*pNew));
  ਜੇ( pNew==ਨਿਰਾ ){
    rc = SQLITE_NOMEM_BKPT;
    ਜਾਓ ਸਿਰਾ_create_proxy;
  }
  memset(pNew, 0, ਆਕਾਰ(unixFile));
  pNew->openFlags = openFlags;
  memset(&dummyVfs, 0, ਆਕਾਰ(dummyVfs));
  dummyVfs.p_App_ਅੰਕੜੇ = (ਖਾਲੀ*)&autolockIoFinder;
  dummyVfs.zName = "dummy";
  p_ਅਣਵਰਤਿਆ->fd = fd;
  p_ਅਣਵਰਤਿਆ->flags = openFlags;
  pNew->p_Preallocated_ਅਣਵਰਤਿਆ = p_ਅਣਵਰਤਿਆ;

  rc = fill_ਵਿੱਚ_Unix_File(&dummyVfs, fd, (sqlite3_file*)pNew, ਰਸਤਾ, 0);
  ਜੇ( rc==SQLITE_OK ){
    *ppFile = pNew;
    ਵਾਪਸ SQLITE_OK;
  }
ਸਿਰਾ_create_proxy:  
  robust_close(pNew, fd, __LINE__);
  sqlite3_free(pNew);
  sqlite3_free(p_ਅਣਵਰਤਿਆ);
  ਵਾਪਸ rc;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
/* simulate multiple hosts by creating unique hostid file paths */
ਪੂਰਨ sqlite3_hostid_ਅੰਕ = 0;
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PROXY_HOSTIDLEN    16  /* conch file host id length */

#ਜੇ HAVE_GETHOSTUUID
/* Not always defined in the headers as it ought to be */
ਬਾਹਰੀ ਪੂਰਨ gethostuuid(uuid_t id, ਅਚਲ ਬਣਤਰ timespec *wait);
#ਜੇ_ਅੰਤ

/* get the host ID via gethostuuid(), pHostID must point to PROXY_HOSTIDLEN
** bytes of writable memory.
*/
ਸਥਿਰ ਪੂਰਨ proxyGetHostID(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pHostID, ਪੂਰਨ *pError){
  ਪ੍ਰਤਿਜ੍ਞਾ(PROXY_HOSTIDLEN == ਆਕਾਰ(uuid_t));
  memset(pHostID, 0, PROXY_HOSTIDLEN);
#ਜੇ HAVE_GETHOSTUUID
  {
    ਬਣਤਰ timespec timeout = {1, 0}; /* 1 sec timeout */
    ਜੇ( gethostuuid(pHostID, &timeout) ){
      ਪੂਰਨ err = errno;
      ਜੇ( pError ){
        *pError = err;
      }
      ਵਾਪਸ SQLITE_IOERR;
    }
  }
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(pError);
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  /* simulate multiple hosts by creating unique hostid file paths */
  ਜੇ( sqlite3_hostid_ਅੰਕ != 0){
    pHostID[0] = (ਅੱਖਰ)(pHostID[0] + (ਅੱਖਰ)(sqlite3_hostid_ਅੰਕ & 0xFF));
  }
#ਜੇ_ਅੰਤ

  ਵਾਪਸ SQLITE_OK;
}

/* The conch file contains the header, host id and lock file path
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PROXY_CONCHVERSION 2   /* 1-byte header, 16-byte host id, path */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PROXY_HEADERLEN    1   /* conch file header length */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PROXY_PATHINDEX    (PROXY_HEADERLEN+PROXY_HOSTIDLEN)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PROXY_MAXCONCHLEN  (PROXY_HEADERLEN+PROXY_HOSTIDLEN+MAXPATHLEN)

/*
** Takes an open conch file, copies the contents to a new path and then moves
** it back.  The newly created file's file descriptor is assigned to the
** conch file structure and finally the original conch file descriptor is
** closed.  Returns zero if successful.
*/
ਸਥਿਰ ਪੂਰਨ proxy_Break_Conch_ਤਾਲਾ(unixFile *pFile, uuid_t myHostID){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
  unixFile *conchFile = pCtx->conchFile;
  ਅੱਖਰ t_ਰਸਤਾ[MAXPATHLEN];
  ਅੱਖਰ buf[PROXY_MAXCONCHLEN];
  ਅੱਖਰ *c_ਰਸਤਾ = pCtx->conch_File_ਰਸਤਾ;
  size_t readLen = 0;
  size_t ਰਸਤਾ_Len = 0;
  ਅੱਖਰ errmsg[64] = "";
  ਪੂਰਨ fd = -1;
  ਪੂਰਨ rc = -1;
  ਅਣਵਰਤਿਆ_PARAMETER(myHostID);

  /* create a new path by replace the trailing '-conch' with '-break' */
  ਰਸਤਾ_Len = strlcpy(t_ਰਸਤਾ, c_ਰਸਤਾ, MAXPATHLEN);
  ਜੇ( ਰਸਤਾ_Len>MAXPATHLEN || ਰਸਤਾ_Len<6 ||
     (strlcpy(&t_ਰਸਤਾ[ਰਸਤਾ_Len-5], "break", 6) != 5) ){
    sqlite3_snprintf(ਆਕਾਰ(errmsg),errmsg,"path error (len %d)",(ਪੂਰਨ)ਰਸਤਾ_Len);
    ਜਾਓ ਸਿਰਾ_breaklock;
  }
  /* read the conch content */
  readLen = osPread(conchFile->h, buf, PROXY_MAXCONCHLEN, 0);
  ਜੇ( readLen<PROXY_PATHINDEX ){
    sqlite3_snprintf(ਆਕਾਰ(errmsg),errmsg,"read error (len %d)",(ਪੂਰਨ)readLen);
    ਜਾਓ ਸਿਰਾ_breaklock;
  }
  /* write it out to the temporary break file */
  fd = robust_open(t_ਰਸਤਾ, (O_RDWR|O_CREAT|O_EXCL|O_NOFOLLOW), 0);
  ਜੇ( fd<0 ){
    sqlite3_snprintf(ਆਕਾਰ(errmsg), errmsg, "create failed (%d)", errno);
    ਜਾਓ ਸਿਰਾ_breaklock;
  }
  ਜੇ( osPwrite(fd, buf, readLen, 0) != (ssize_t)readLen ){
    sqlite3_snprintf(ਆਕਾਰ(errmsg), errmsg, "write failed (%d)", errno);
    ਜਾਓ ਸਿਰਾ_breaklock;
  }
  ਜੇ( rename(t_ਰਸਤਾ, c_ਰਸਤਾ) ){
    sqlite3_snprintf(ਆਕਾਰ(errmsg), errmsg, "rename failed (%d)", errno);
    ਜਾਓ ਸਿਰਾ_breaklock;
  }
  rc = 0;
  fprintf(stderr, "broke stale lock on %s\n", c_ਰਸਤਾ);
  robust_close(pFile, conchFile->h, __LINE__);
  conchFile->h = fd;
  conchFile->openFlags = O_RDWR | O_CREAT;

ਸਿਰਾ_breaklock:
  ਜੇ( rc ){
    ਜੇ( fd>=0 ){
      osUnlink(t_ਰਸਤਾ);
      robust_close(pFile, fd, __LINE__);
    }
    fprintf(stderr, "failed to break stale lock on %s, %s\n", c_ਰਸਤਾ, errmsg);
  }
  ਵਾਪਸ rc;
}

/* Take the requested lock on the conch file and break a stale lock if the
** host id matches.
*/
ਸਥਿਰ ਪੂਰਨ proxy_Conch_ਤਾਲਾ(unixFile *pFile, uuid_t myHostID, ਪੂਰਨ ਤਾਲਾ_Type){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
  unixFile *conchFile = pCtx->conchFile;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ nTries = 0;
  ਬਣਤਰ timespec conch_Mod_ਸਮਾਂ;

  memset(&conch_Mod_ਸਮਾਂ, 0, ਆਕਾਰ(conch_Mod_ਸਮਾਂ));
  ਕਰੋ {
    rc = conchFile->p_ਵਿਧੀ->x_ਤਾਲਾ((sqlite3_file*)conchFile, ਤਾਲਾ_Type);
    nTries ++;
    ਜੇ( rc==SQLITE_BUSY ){
      /* If the lock failed (busy):
       * 1st try: get the mod time of the conch, wait 0.5s and try again.
       * 2nd try: fail if the mod time changed or host id is different, wait
       *           10 sec and try again
       * 3rd try: break the lock unless the mod time has changed.
       */
      ਬਣਤਰ stat buf;
      ਜੇ( osFstat(conchFile->h, &buf) ){
        store_ਆਖਰੀ_Errno(pFile, errno);
        ਵਾਪਸ SQLITE_IOERR_ਤਾਲਾ;
      }
    
      ਜੇ( nTries==1 ){
        conch_Mod_ਸਮਾਂ = buf.st_mtimespec;
        unixSleep(0,500000); /* wait 0.5 sec and try the lock again*/
        ਜਾਰੀ;
      }

      ਪ੍ਰਤਿਜ੍ਞਾ( nTries>1 );
      ਜੇ( conch_Mod_ਸਮਾਂ.tv_sec != buf.st_mtimespec.tv_sec ||
         conch_Mod_ਸਮਾਂ.tv_nsec != buf.st_mtimespec.tv_nsec ){
        ਵਾਪਸ SQLITE_BUSY;
      }
    
      ਜੇ( nTries==2 ){
        ਅੱਖਰ tBuf[PROXY_MAXCONCHLEN];
        ਪੂਰਨ len = osPread(conchFile->h, tBuf, PROXY_MAXCONCHLEN, 0);
        ਜੇ( len<0 ){
          store_ਆਖਰੀ_Errno(pFile, errno);
          ਵਾਪਸ SQLITE_IOERR_ਤਾਲਾ;
        }
        ਜੇ( len>PROXY_PATHINDEX && tBuf[0]==(ਅੱਖਰ)PROXY_CONCHVERSION){
          /* don't break the lock if the host id doesn't match */
          ਜੇ( 0!=memcmp(&tBuf[PROXY_HEADERLEN], myHostID, PROXY_HOSTIDLEN) ){
            ਵਾਪਸ SQLITE_BUSY;
          }
        }ਨਹੀਂਤਾਂ{
          /* don't break the lock on short read or a version mismatch */
          ਵਾਪਸ SQLITE_BUSY;
        }
        unixSleep(0,10000000); /* wait 10 sec and try the lock again */
        ਜਾਰੀ;
      }
    
      ਪ੍ਰਤਿਜ੍ਞਾ( nTries==3 );
      ਜੇ( 0==proxy_Break_Conch_ਤਾਲਾ(pFile, myHostID) ){
        rc = SQLITE_OK;
        ਜੇ( ਤਾਲਾ_Type==EXCLUSIVE_ਤਾਲਾ ){
          rc = conchFile->p_ਵਿਧੀ->x_ਤਾਲਾ((sqlite3_file*)conchFile, SHARED_ਤਾਲਾ);
        }
        ਜੇ( !rc ){
          rc = conchFile->p_ਵਿਧੀ->x_ਤਾਲਾ((sqlite3_file*)conchFile, ਤਾਲਾ_Type);
        }
      }
    }
  } ਜਦੋਂ( rc==SQLITE_BUSY && nTries<3 );

  ਵਾਪਸ rc;
}

/* Takes the conch by taking a shared lock and read the contents conch, if
** lockPath is non-NULL, the host ID and lock file path must match.  A NULL
** lockPath means that the lockPath in the conch file will be used if the
** host IDs match, or a new lock path will be generated automatically
** and written to the conch file.
*/
ਸਥਿਰ ਪੂਰਨ proxyTakeConch(unixFile *pFile){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;

  ਜੇ( pCtx->conchHeld!=0 ){
    ਵਾਪਸ SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    unixFile *conchFile = pCtx->conchFile;
    uuid_t myHostID;
    ਪੂਰਨ pError = 0;
    ਅੱਖਰ readBuf[PROXY_MAXCONCHLEN];
    ਅੱਖਰ ਤਾਲਾ_ਰਸਤਾ[MAXPATHLEN];
    ਅੱਖਰ *ਆਰਜ਼ੀ_ਤਾਲਾ_ਰਸਤਾ = ਨਿਰਾ;
    ਪੂਰਨ rc = SQLITE_OK;
    ਪੂਰਨ createConch = 0;
    ਪੂਰਨ host_Id_ਮੇਲ = 0;
    ਪੂਰਨ readLen = 0;
    ਪੂਰਨ try_Old_ਤਾਲਾ_ਰਸਤਾ = 0;
    ਪੂਰਨ force_New_ਤਾਲਾ_ਰਸਤਾ = 0;
  
    OSTRACE(("TAKECONCH  %d for %s pid=%d\n", conchFile->h,
             (pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ ? pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ : ":auto:"),
             osGetpid(0)));

    rc = proxyGetHostID(myHostID, &pError);
    ਜੇ( (rc&0xff)==SQLITE_IOERR ){
      store_ਆਖਰੀ_Errno(pFile, pError);
      ਜਾਓ ਸਿਰਾ_takeconch;
    }
    rc = proxy_Conch_ਤਾਲਾ(pFile, myHostID, SHARED_ਤਾਲਾ);
    ਜੇ( rc!=SQLITE_OK ){
      ਜਾਓ ਸਿਰਾ_takeconch;
    }
    /* read the existing conch file */
    readLen = seekAndRead((unixFile*)conchFile, 0, readBuf, PROXY_MAXCONCHLEN);
    ਜੇ( readLen<0 ){
      /* I/O error: lastErrno set by seekAndRead */
      store_ਆਖਰੀ_Errno(pFile, conchFile->ਆਖਰੀ_Errno);
      rc = SQLITE_IOERR_READ;
      ਜਾਓ ਸਿਰਾ_takeconch;
    }ਨਹੀਂਤਾਂ ਜੇ( readLen<=(PROXY_HEADERLEN+PROXY_HOSTIDLEN) ||
             readBuf[0]!=(ਅੱਖਰ)PROXY_CONCHVERSION ){
      /* a short read or version format mismatch means we need to create a new
      ** conch file.
      */
      createConch = 1;
    }
    /* if the host id matches and the lock path already exists in the conch
    ** we'll try to use the path there, if we can't open that path, we'll
    ** retry with a new auto-generated path
    */
    ਕਰੋ { /* in case we need to try again for an :auto: named lock file */

      ਜੇ( !createConch && !force_New_ਤਾਲਾ_ਰਸਤਾ ){
        host_Id_ਮੇਲ = !memcmp(&readBuf[PROXY_HEADERLEN], myHostID,
                                  PROXY_HOSTIDLEN);
        /* if the conch has data compare the contents */
        ਜੇ( !pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ ){
          /* for auto-named local lock file, just check the host ID and we'll
           ** use the local lock file path that's already in there
           */
          ਜੇ( host_Id_ਮੇਲ ){
            size_t ਰਸਤਾ_Len = (readLen - PROXY_PATHINDEX);
          
            ਜੇ( ਰਸਤਾ_Len>=MAXPATHLEN ){
              ਰਸਤਾ_Len=MAXPATHLEN-1;
            }
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਤਾਲਾ_ਰਸਤਾ, &readBuf[PROXY_PATHINDEX], ਰਸਤਾ_Len);
            ਤਾਲਾ_ਰਸਤਾ[ਰਸਤਾ_Len] = 0;
            ਆਰਜ਼ੀ_ਤਾਲਾ_ਰਸਤਾ = ਤਾਲਾ_ਰਸਤਾ;
            try_Old_ਤਾਲਾ_ਰਸਤਾ = 1;
            /* create a copy of the lock path if the conch is taken */
            ਜਾਓ ਸਿਰਾ_takeconch;
          }
        }ਨਹੀਂਤਾਂ ਜੇ( host_Id_ਮੇਲ
               && !ਲੜੀ_ਤੁਲਨਾ(pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ, &readBuf[PROXY_PATHINDEX],
                           readLen-PROXY_PATHINDEX)
        ){
          /* conch host and lock path match */
          ਜਾਓ ਸਿਰਾ_takeconch;
        }
      }
    
      /* if the conch isn't writable and doesn't match, we can't take it */
      ਜੇ( (conchFile->openFlags&O_RDWR) == 0 ){
        rc = SQLITE_BUSY;
        ਜਾਓ ਸਿਰਾ_takeconch;
      }
    
      /* either the conch didn't match or we need to create a new one */
      ਜੇ( !pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ ){
        proxy_Get_ਤਾਲਾ_ਰਸਤਾ(pCtx->db_ਰਸਤਾ, ਤਾਲਾ_ਰਸਤਾ, MAXPATHLEN);
        ਆਰਜ਼ੀ_ਤਾਲਾ_ਰਸਤਾ = ਤਾਲਾ_ਰਸਤਾ;
        /* create a copy of the lock path _only_ if the conch is taken */
      }
    
      /* update conch with host and path (this will fail if other process
      ** has a shared lock already), if the host id matches, use the big
      ** stick.
      */
      futimes(conchFile->h, ਨਿਰਾ);
      ਜੇ( host_Id_ਮੇਲ && !createConch ){
        ਜੇ( conchFile->pInode && conchFile->pInode->nShared>1 ){
          /* We are trying for an exclusive lock but another thread in this
           ** same process is still holding a shared lock. */
          rc = SQLITE_BUSY;
        } ਨਹੀਂਤਾਂ {        
          rc = proxy_Conch_ਤਾਲਾ(pFile, myHostID, EXCLUSIVE_ਤਾਲਾ);
        }
      }ਨਹੀਂਤਾਂ{
        rc = proxy_Conch_ਤਾਲਾ(pFile, myHostID, EXCLUSIVE_ਤਾਲਾ);
      }
      ਜੇ( rc==SQLITE_OK ){
        ਅੱਖਰ write_ਬਫਰ[PROXY_MAXCONCHLEN];
        ਪੂਰਨ write_ਮਾਪ = 0;
      
        write_ਬਫਰ[0] = (ਅੱਖਰ)PROXY_CONCHVERSION;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(&write_ਬਫਰ[PROXY_HEADERLEN], myHostID, PROXY_HOSTIDLEN);
        ਜੇ( pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ!=ਨਿਰਾ ){
          strlcpy(&write_ਬਫਰ[PROXY_PATHINDEX], pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ,
                  MAXPATHLEN);
        }ਨਹੀਂਤਾਂ{
          strlcpy(&write_ਬਫਰ[PROXY_PATHINDEX], ਆਰਜ਼ੀ_ਤਾਲਾ_ਰਸਤਾ, MAXPATHLEN);
        }
        write_ਮਾਪ = PROXY_PATHINDEX + ਲੜੀ_ਲੰਬਾਈ(&write_ਬਫਰ[PROXY_PATHINDEX]);
        robust_ftruncate(conchFile->h, write_ਮਾਪ);
        rc = unixWrite((sqlite3_file *)conchFile, write_ਬਫਰ, write_ਮਾਪ, 0);
        ਭਰਿਆ_fsync(conchFile->h,0,0);
        /* If we created a new conch file (not just updated the contents of a
         ** valid conch file), try to match the permissions of the database
         */
        ਜੇ( rc==SQLITE_OK && createConch ){
          ਬਣਤਰ stat buf;
          ਪੂਰਨ err = osFstat(pFile->h, &buf);
          ਜੇ( err==0 ){
            ਬਹੁਲਕ_t cmode = buf.st_ਬਹੁਲਕ&(S_IRUSR|S_IWUSR | S_IRGRP|S_IWGRP |
                                        S_IROTH|S_IWOTH);
            /* try to match the database file R/W permissions, ignore failure */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_PROXY_DEBUG
            osFchmod(conchFile->h, cmode);
#ਨਹੀਂਤਾਂ
            ਕਰੋ{
              rc = osFchmod(conchFile->h, cmode);
            }ਜਦੋਂ( rc==(-1) && errno==EINTR );
            ਜੇ( rc!=0 ){
              ਪੂਰਨ ਕੋਡ = errno;
              fprintf(stderr, "fchmod %o FAILED with %d %s\n",
                      cmode, ਕੋਡ, strerror(ਕੋਡ));
            } ਨਹੀਂਤਾਂ {
              fprintf(stderr, "fchmod %o SUCCEDED\n",cmode);
            }
          }ਨਹੀਂਤਾਂ{
            ਪੂਰਨ ਕੋਡ = errno;
            fprintf(stderr, "STAT FAILED[%d] with %d %s\n",
                    err, ਕੋਡ, strerror(ਕੋਡ));
#ਜੇ_ਅੰਤ
          }
        }
      }
      conchFile->p_ਵਿਧੀ->xUnlock((sqlite3_file*)conchFile, SHARED_ਤਾਲਾ);
    
    ਸਿਰਾ_takeconch:
      OSTRACE(("TRANSPROXY: CLOSE  %d\n", pFile->h));
      ਜੇ( rc==SQLITE_OK && pFile->openFlags ){
        ਪੂਰਨ fd;
        ਜੇ( pFile->h>=0 ){
          robust_close(pFile, pFile->h, __LINE__);
        }
        pFile->h = -1;
        fd = robust_open(pCtx->db_ਰਸਤਾ, pFile->openFlags, 0);
        OSTRACE(("TRANSPROXY: OPEN  %d\n", fd));
        ਜੇ( fd>=0 ){
          pFile->h = fd;
        }ਨਹੀਂਤਾਂ{
          rc=SQLITE_CANTOPEN_BKPT; /* SQLITE_BUSY? proxyTakeConch called
           during locking */
        }
      }
      ਜੇ( rc==SQLITE_OK && !pCtx->ਤਾਲਾ_Proxy ){
        ਅੱਖਰ *ਰਸਤਾ = ਆਰਜ਼ੀ_ਤਾਲਾ_ਰਸਤਾ ? ਆਰਜ਼ੀ_ਤਾਲਾ_ਰਸਤਾ : pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ;
        rc = proxyCreateUnixFile(ਰਸਤਾ, &pCtx->ਤਾਲਾ_Proxy, 1);
        ਜੇ( rc!=SQLITE_OK && rc!=SQLITE_NOMEM && try_Old_ਤਾਲਾ_ਰਸਤਾ ){
          /* we couldn't create the proxy lock file with the old lock file path
           ** so try again via auto-naming
           */
          force_New_ਤਾਲਾ_ਰਸਤਾ = 1;
          try_Old_ਤਾਲਾ_ਰਸਤਾ = 0;
          ਜਾਰੀ; /* go back to the do {} while start point, try again */
        }
      }
      ਜੇ( rc==SQLITE_OK ){
        /* Need to make a copy of path if we extracted the value
         ** from the conch file or the path was allocated on the stack
         */
        ਜੇ( ਆਰਜ਼ੀ_ਤਾਲਾ_ਰਸਤਾ ){
          pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ = sqlite3DbStrDup(0, ਆਰਜ਼ੀ_ਤਾਲਾ_ਰਸਤਾ);
          ਜੇ( !pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ ){
            rc = SQLITE_NOMEM_BKPT;
          }
        }
      }
      ਜੇ( rc==SQLITE_OK ){
        pCtx->conchHeld = 1;
      
        ਜੇ( pCtx->ਤਾਲਾ_Proxy->p_ਵਿਧੀ == &afpIoMethods ){
          afpLockingContext *afpCtx;
          afpCtx = (afpLockingContext *)pCtx->ਤਾਲਾ_Proxy->lockingContext;
          afpCtx->db_ਰਸਤਾ = pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ;
        }
      } ਨਹੀਂਤਾਂ {
        conchFile->p_ਵਿਧੀ->xUnlock((sqlite3_file*)conchFile, NO_ਤਾਲਾ);
      }
      OSTRACE(("TAKECONCH  %d %s\n", conchFile->h,
               rc==SQLITE_OK?"ok":"failed"));
      ਵਾਪਸ rc;
    } ਜਦੋਂ (1); /* in case we need to retry the :auto: lock file -
                 ** we should never get here except via the 'continue' call. */
  }
}

/*
** If pFile holds a lock on a conch file, then release that lock.
*/
ਸਥਿਰ ਪੂਰਨ proxyReleaseConch(unixFile *pFile){
  ਪੂਰਨ rc = SQLITE_OK;         /* Subroutine return code */
  proxyLockingContext *pCtx;  /* The locking context for the proxy lock */
  unixFile *conchFile;        /* Name of the conch file */

  pCtx = (proxyLockingContext *)pFile->lockingContext;
  conchFile = pCtx->conchFile;
  OSTRACE(("RELEASECONCH  %d for %s pid=%d\n", conchFile->h,
           (pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ ? pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ : ":auto:"),
           osGetpid(0)));
  ਜੇ( pCtx->conchHeld>0 ){
    rc = conchFile->p_ਵਿਧੀ->xUnlock((sqlite3_file*)conchFile, NO_ਤਾਲਾ);
  }
  pCtx->conchHeld = 0;
  OSTRACE(("RELEASECONCH  %d %s\n", conchFile->h,
           (rc==SQLITE_OK ? "ok" : "failed")));
  ਵਾਪਸ rc;
}

/*
** Given the name of a database file, compute the name of its conch file.
** Store the conch filename in memory obtained from sqlite3_malloc64().
** Make *pConchPath point to the new name.  Return SQLITE_OK on success
** or SQLITE_NOMEM if unable to obtain memory.
**
** The caller is responsible for ensuring that the allocated memory
** space is eventually freed.
**
** *pConchPath is set to NULL if a memory allocation error occurs.
*/
ਸਥਿਰ ਪੂਰਨ proxyCreateConchPathname(ਅੱਖਰ *db_ਰਸਤਾ, ਅੱਖਰ **p_Conch_ਰਸਤਾ){
  ਪੂਰਨ i;                        /* Loop counter */
  ਪੂਰਨ len = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(db_ਰਸਤਾ); /* Length of database filename - dbPath */
  ਅੱਖਰ *conch_ਰਸਤਾ;              /* buffer in which to construct conch name */

  /* Allocate space for the conch filename and initialize the name to
  ** the name of the original database file. */
  *p_Conch_ਰਸਤਾ = conch_ਰਸਤਾ = (ਅੱਖਰ *)sqlite3_malloc64(len + 8);
  ਜੇ( conch_ਰਸਤਾ==0 ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(conch_ਰਸਤਾ, db_ਰਸਤਾ, len+1);

  /* now insert a "." before the last / character */
  ਲਈ( i=(len-1); i>=0; i-- ){
    ਜੇ( conch_ਰਸਤਾ[i]=='/' ){
      i++;
      ਰੋਕੋ;
    }
  }
  conch_ਰਸਤਾ[i]='.';
  ਜਦੋਂ ( i<len ){
    conch_ਰਸਤਾ[i+1]=db_ਰਸਤਾ[i];
    i++;
  }

  /* append the "-conch" suffix to the file */
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&conch_ਰਸਤਾ[i+1], "-conch", 7);
  ਪ੍ਰਤਿਜ੍ਞਾ( (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(conch_ਰਸਤਾ) == len+7 );

  ਵਾਪਸ SQLITE_OK;
}


/* Takes a fully configured proxy locking-style unix file and switches
** the local lock file path
*/
ਸਥਿਰ ਪੂਰਨ switch_ਤਾਲਾ_Proxy_ਰਸਤਾ(unixFile *pFile, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ) {
  proxyLockingContext *pCtx = (proxyLockingContext*)pFile->lockingContext;
  ਅੱਖਰ *old_ਰਸਤਾ = pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ;
  ਪੂਰਨ rc = SQLITE_OK;

  ਜੇ( pFile->e_File_ਤਾਲਾ!=NO_ਤਾਲਾ ){
    ਵਾਪਸ SQLITE_BUSY;
  }

  /* nothing to do if the path is NULL, :auto: or matches the existing path */
  ਜੇ( !ਰਸਤਾ || ਰਸਤਾ[0]=='\0' || !strcmp(ਰਸਤਾ, ":auto:") ||
    (old_ਰਸਤਾ && !ਲੜੀ_ਤੁਲਨਾ(old_ਰਸਤਾ, ਰਸਤਾ, MAXPATHLEN)) ){
    ਵਾਪਸ SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    unixFile *ਤਾਲਾ_Proxy = pCtx->ਤਾਲਾ_Proxy;
    pCtx->ਤਾਲਾ_Proxy=ਨਿਰਾ;
    pCtx->conchHeld = 0;
    ਜੇ( ਤਾਲਾ_Proxy!=ਨਿਰਾ ){
      rc=ਤਾਲਾ_Proxy->p_ਵਿਧੀ->xClose((sqlite3_file *)ਤਾਲਾ_Proxy);
      ਜੇ( rc ) ਵਾਪਸ rc;
      sqlite3_free(ਤਾਲਾ_Proxy);
    }
    sqlite3_free(old_ਰਸਤਾ);
    pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ = sqlite3DbStrDup(0, ਰਸਤਾ);
  }

  ਵਾਪਸ rc;
}

/*
** pFile is a file that has been opened by a prior xOpen call.  dbPath
** is a string buffer at least MAXPATHLEN+1 characters in size.
**
** This routine find the filename associated with pFile and writes it
** int dbPath.
*/
ਸਥਿਰ ਪੂਰਨ proxy_Get_Db_ਰਸਤਾ_For_Unix_File(unixFile *pFile, ਅੱਖਰ *db_ਰਸਤਾ){
#ਜੇ defined(__APPLE__)
  ਜੇ( pFile->p_ਵਿਧੀ == &afpIoMethods ){
    /* afp style keeps a reference to the db path in the filePath field
    ** of the struct */
    ਪ੍ਰਤਿਜ੍ਞਾ( (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ((ਅੱਖਰ*)pFile->lockingContext)<=MAXPATHLEN );
    strlcpy(db_ਰਸਤਾ, ((afpLockingContext *)pFile->lockingContext)->db_ਰਸਤਾ,
            MAXPATHLEN);
  } ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
  ਜੇ( pFile->p_ਵਿਧੀ == &dotlockIoMethods ){
    /* dot lock style uses the locking context to store the dot lock
    ** file path */
    ਪੂਰਨ len = ਲੜੀ_ਲੰਬਾਈ((ਅੱਖਰ *)pFile->lockingContext) - ਲੜੀ_ਲੰਬਾਈ(DOTLOCK_SUFFIX);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(db_ਰਸਤਾ, (ਅੱਖਰ *)pFile->lockingContext, len + 1);
  }ਨਹੀਂਤਾਂ{
    /* all other styles use the locking context to store the db file path */
    ਪ੍ਰਤਿਜ੍ਞਾ( ਲੜੀ_ਲੰਬਾਈ((ਅੱਖਰ*)pFile->lockingContext)<=MAXPATHLEN );
    strlcpy(db_ਰਸਤਾ, (ਅੱਖਰ *)pFile->lockingContext, MAXPATHLEN);
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Takes an already filled in unix file and alters it so all file locking
** will be performed on the local proxy lock file.  The following fields
** are preserved in the locking context so that they can be restored and
** the unix structure properly cleaned up at close time:
**  ->lockingContext
**  ->pMethod
*/
ਸਥਿਰ ਪੂਰਨ proxyTransformUnixFile(unixFile *pFile, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ) {
  proxyLockingContext *pCtx;
  ਅੱਖਰ db_ਰਸਤਾ[MAXPATHLEN+1];       /* Name of the database file */
  ਅੱਖਰ *ਤਾਲਾ_ਰਸਤਾ=ਨਿਰਾ;
  ਪੂਰਨ rc = SQLITE_OK;

  ਜੇ( pFile->e_File_ਤਾਲਾ!=NO_ਤਾਲਾ ){
    ਵਾਪਸ SQLITE_BUSY;
  }
  proxy_Get_Db_ਰਸਤਾ_For_Unix_File(pFile, db_ਰਸਤਾ);
  ਜੇ( !ਰਸਤਾ || ਰਸਤਾ[0]=='\0' || !strcmp(ਰਸਤਾ, ":auto:") ){
    ਤਾਲਾ_ਰਸਤਾ=ਨਿਰਾ;
  }ਨਹੀਂਤਾਂ{
    ਤਾਲਾ_ਰਸਤਾ=(ਅੱਖਰ *)ਰਸਤਾ;
  }

  OSTRACE(("TRANSPROXY  %d for %s pid=%d\n", pFile->h,
           (ਤਾਲਾ_ਰਸਤਾ ? ਤਾਲਾ_ਰਸਤਾ : ":auto:"), osGetpid(0)));

  pCtx = sqlite3_malloc64( ਆਕਾਰ(*pCtx) );
  ਜੇ( pCtx==0 ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  memset(pCtx, 0, ਆਕਾਰ(*pCtx));

  rc = proxyCreateConchPathname(db_ਰਸਤਾ, &pCtx->conch_File_ਰਸਤਾ);
  ਜੇ( rc==SQLITE_OK ){
    rc = proxyCreateUnixFile(pCtx->conch_File_ਰਸਤਾ, &pCtx->conchFile, 0);
    ਜੇ( rc==SQLITE_CANTOPEN && ((pFile->openFlags&O_RDWR) == 0) ){
      /* if (a) the open flags are not O_RDWR, (b) the conch isn't there, and
      ** (c) the file system is read-only, then enable no-locking access.
      ** Ugh, since O_RDONLY==0x0000 we test for !O_RDWR since unixOpen asserts
      ** that openFlags will have only one of O_RDONLY or O_RDWR.
      */
      ਬਣਤਰ statfs fsInfo;
      ਬਣਤਰ stat conchInfo;
      ਪੂਰਨ goLockless = 0;

      ਜੇ( osStat(pCtx->conch_File_ਰਸਤਾ, &conchInfo) == -1 ) {
        ਪੂਰਨ err = errno;
        ਜੇ( (err==ENOENT) && (statfs(db_ਰਸਤਾ, &fsInfo) != -1) ){
          goLockless = (fsInfo.f_flags&MNT_RDONLY) == MNT_RDONLY;
        }
      }
      ਜੇ( goLockless ){
        pCtx->conchHeld = -1; /* read only FS/ lockless */
        rc = SQLITE_OK;
      }
    }
  }
  ਜੇ( rc==SQLITE_OK && ਤਾਲਾ_ਰਸਤਾ ){
    pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ = sqlite3DbStrDup(0, ਤਾਲਾ_ਰਸਤਾ);
  }

  ਜੇ( rc==SQLITE_OK ){
    pCtx->db_ਰਸਤਾ = sqlite3DbStrDup(0, db_ਰਸਤਾ);
    ਜੇ( pCtx->db_ਰਸਤਾ==ਨਿਰਾ ){
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  ਜੇ( rc==SQLITE_OK ){
    /* all memory is allocated, proxys are created and assigned,
    ** switch the locking context and pMethod then return.
    */
    pCtx->oldLockingContext = pFile->lockingContext;
    pFile->lockingContext = pCtx;
    pCtx->p_Old_ਵਿਧੀ = pFile->p_ਵਿਧੀ;
    pFile->p_ਵਿਧੀ = &proxyIoMethods;
  }ਨਹੀਂਤਾਂ{
    ਜੇ( pCtx->conchFile ){
      pCtx->conchFile->p_ਵਿਧੀ->xClose((sqlite3_file *)pCtx->conchFile);
      sqlite3_free(pCtx->conchFile);
    }
    sqlite3DbFree(0, pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ);
    sqlite3_free(pCtx->conch_File_ਰਸਤਾ);
    sqlite3_free(pCtx);
  }
  OSTRACE(("TRANSPROXY  %d %s\n", pFile->h,
           (rc==SQLITE_OK ? "ok" : "failed")));
  ਵਾਪਸ rc;
}


/*
** This routine handles sqlite3_file_control() calls that are specific
** to proxy locking.
*/
ਸਥਿਰ ਪੂਰਨ proxyFileControl(sqlite3_file *id, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  ਚੋਣ( op ){
    ਹਾਲਤ SQLITE_FCNTL_GET_LOCKPROXYFILE: {
      unixFile *pFile = (unixFile*)id;
      ਜੇ( pFile->p_ਵਿਧੀ == &proxyIoMethods ){
        proxyLockingContext *pCtx = (proxyLockingContext*)pFile->lockingContext;
        proxyTakeConch(pFile);
        ਜੇ( pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ ){
          *(ਅਚਲ ਅੱਖਰ **)pArg = pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ;
        }ਨਹੀਂਤਾਂ{
          *(ਅਚਲ ਅੱਖਰ **)pArg = ":auto: (not held)";
        }
      } ਨਹੀਂਤਾਂ {
        *(ਅਚਲ ਅੱਖਰ **)pArg = ਨਿਰਾ;
      }
      ਵਾਪਸ SQLITE_OK;
    }
    ਹਾਲਤ SQLITE_FCNTL_ਸੈੱਟ_LOCKPROXYFILE: {
      unixFile *pFile = (unixFile*)id;
      ਪੂਰਨ rc = SQLITE_OK;
      ਪੂਰਨ isProxyStyle = (pFile->p_ਵਿਧੀ == &proxyIoMethods);
      ਜੇ( pArg==ਨਿਰਾ || (ਅਚਲ ਅੱਖਰ *)pArg==0 ){
        ਜੇ( isProxyStyle ){
          /* turn off proxy locking - not supported.  If support is added for
          ** switching proxy locking mode off then it will need to fail if
          ** the journal mode is WAL mode.
          */
          rc = SQLITE_ERROR /*SQLITE_PROTOCOL? SQLITE_MISUSE?*/;
        }ਨਹੀਂਤਾਂ{
          /* turn off proxy locking - already off - NOOP */
          rc = SQLITE_OK;
        }
      }ਨਹੀਂਤਾਂ{
        ਅਚਲ ਅੱਖਰ *proxy_ਰਸਤਾ = (ਅਚਲ ਅੱਖਰ *)pArg;
        ਜੇ( isProxyStyle ){
          proxyLockingContext *pCtx =
            (proxyLockingContext*)pFile->lockingContext;
          ਜੇ( !strcmp(pArg, ":auto:")
           || (pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ &&
               !ਲੜੀ_ਤੁਲਨਾ(pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ, proxy_ਰਸਤਾ, MAXPATHLEN))
          ){
            rc = SQLITE_OK;
          }ਨਹੀਂਤਾਂ{
            rc = switch_ਤਾਲਾ_Proxy_ਰਸਤਾ(pFile, proxy_ਰਸਤਾ);
          }
        }ਨਹੀਂਤਾਂ{
          /* turn on proxy file locking */
          rc = proxyTransformUnixFile(pFile, proxy_ਰਸਤਾ);
        }
      }
      ਵਾਪਸ rc;
    }
    ਮੂਲ: {
      ਪ੍ਰਤਿਜ੍ਞਾ( 0 );  /* The call assures that only valid opcodes are sent */
    }
  }
  /*NOTREACHED*/ ਪ੍ਰਤਿਜ੍ਞਾ(0);
  ਵਾਪਸ SQLITE_ERROR;
}

/*
** Within this division (the proxying locking implementation) the procedures
** above this point are all utilities.  The lock-related methods of the
** proxy-locking sqlite3_io_method object follow.
*/


/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
ਸਥਿਰ ਪੂਰਨ proxy_Check_Reserved_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ) {
  unixFile *pFile = (unixFile*)id;
  ਪੂਰਨ rc = proxyTakeConch(pFile);
  ਜੇ( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    ਜੇ( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->ਤਾਲਾ_Proxy;
      ਵਾਪਸ proxy->p_ਵਿਧੀ->x_Check_Reserved_ਤਾਲਾ((sqlite3_file*)proxy, p_Res_ਬਾਹਰਲਾ);
    }ਨਹੀਂਤਾਂ{ /* conchHeld < 0 is lockless */
      p_Res_ਬਾਹਰਲਾ=0;
    }
  }
  ਵਾਪਸ rc;
}

/*
** Lock the file with the lock specified by parameter eFileLock - one
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
** This routine will only increase a lock.  Use the sqlite3OsUnlock()
** routine to lower a locking level.
*/
ਸਥਿਰ ਪੂਰਨ proxy_ਤਾਲਾ(sqlite3_file *id, ਪੂਰਨ e_File_ਤਾਲਾ) {
  unixFile *pFile = (unixFile*)id;
  ਪੂਰਨ rc = proxyTakeConch(pFile);
  ਜੇ( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    ਜੇ( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->ਤਾਲਾ_Proxy;
      rc = proxy->p_ਵਿਧੀ->x_ਤਾਲਾ((sqlite3_file*)proxy, e_File_ਤਾਲਾ);
      pFile->e_File_ਤਾਲਾ = proxy->e_File_ਤਾਲਾ;
    }ਨਹੀਂਤਾਂ{
      /* conchHeld < 0 is lockless */
    }
  }
  ਵਾਪਸ rc;
}


/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
ਸਥਿਰ ਪੂਰਨ proxyUnlock(sqlite3_file *id, ਪੂਰਨ e_File_ਤਾਲਾ) {
  unixFile *pFile = (unixFile*)id;
  ਪੂਰਨ rc = proxyTakeConch(pFile);
  ਜੇ( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    ਜੇ( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->ਤਾਲਾ_Proxy;
      rc = proxy->p_ਵਿਧੀ->xUnlock((sqlite3_file*)proxy, e_File_ਤਾਲਾ);
      pFile->e_File_ਤਾਲਾ = proxy->e_File_ਤਾਲਾ;
    }ਨਹੀਂਤਾਂ{
      /* conchHeld < 0 is lockless */
    }
  }
  ਵਾਪਸ rc;
}

/*
** Close a file that uses proxy locks.
*/
ਸਥਿਰ ਪੂਰਨ proxyClose(sqlite3_file *id) {
  ਜੇ( ALWAYS(id) ){
    unixFile *pFile = (unixFile*)id;
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    unixFile *ਤਾਲਾ_Proxy = pCtx->ਤਾਲਾ_Proxy;
    unixFile *conchFile = pCtx->conchFile;
    ਪੂਰਨ rc = SQLITE_OK;

    ਜੇ( ਤਾਲਾ_Proxy ){
      rc = ਤਾਲਾ_Proxy->p_ਵਿਧੀ->xUnlock((sqlite3_file*)ਤਾਲਾ_Proxy, NO_ਤਾਲਾ);
      ਜੇ( rc ) ਵਾਪਸ rc;
      rc = ਤਾਲਾ_Proxy->p_ਵਿਧੀ->xClose((sqlite3_file*)ਤਾਲਾ_Proxy);
      ਜੇ( rc ) ਵਾਪਸ rc;
      sqlite3_free(ਤਾਲਾ_Proxy);
      pCtx->ਤਾਲਾ_Proxy = 0;
    }
    ਜੇ( conchFile ){
      ਜੇ( pCtx->conchHeld ){
        rc = proxyReleaseConch(pFile);
        ਜੇ( rc ) ਵਾਪਸ rc;
      }
      rc = conchFile->p_ਵਿਧੀ->xClose((sqlite3_file*)conchFile);
      ਜੇ( rc ) ਵਾਪਸ rc;
      sqlite3_free(conchFile);
    }
    sqlite3DbFree(0, pCtx->ਤਾਲਾ_Proxy_ਰਸਤਾ);
    sqlite3_free(pCtx->conch_File_ਰਸਤਾ);
    sqlite3DbFree(0, pCtx->db_ਰਸਤਾ);
    /* restore the original locking context and pMethod then close it */
    pFile->lockingContext = pCtx->oldLockingContext;
    pFile->p_ਵਿਧੀ = pCtx->p_Old_ਵਿਧੀ;
    sqlite3_free(pCtx);
    ਵਾਪਸ pFile->p_ਵਿਧੀ->xClose(id);
  }
  ਵਾਪਸ SQLITE_OK;
}



#ਜੇ_ਅੰਤ /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
/*
** The proxy locking style is intended for use with AFP filesystems.
** And since AFP is only supported on MacOSX, the proxy locking is also
** restricted to MacOSX.
**
**
******************* End of the proxy lock implementation **********************
******************************************************************************/

/*
** Initialize the operating system interface.
**
** This routine registers all VFS implementations for unix-like operating
** systems.  This routine, and the sqlite3_os_end() routine that follows,
** should be the only routines in this file that are visible from other
** files.
**
** This routine is called once during SQLite initialization and by a
** single thread.  The memory allocation and mutex subsystems have not
** necessarily been initialized when this routine is called, and so they
** should not be used.
*/
ਪੂਰਨ sqlite3_os_init(ਖਾਲੀ){
  /*
  ** The following macro defines an initializer for an sqlite3_vfs object.
  ** The name of the VFS is NAME.  The pAppData is a pointer to a pointer
  ** to the "finder" function.  (pAppData is a pointer to a pointer because
  ** silly C90 rules prohibit a void* from being cast to a function pointer
  ** and so we have to go through the intermediate pointer to avoid problems
  ** when compiling with -pedantic-errors on GCC.)
  **
  ** The FINDER parameter to this macro is the name of the pointer to the
  ** finder-function.  The finder-function returns a pointer to the
  ** sqlite_io_methods object that implements the desired locking
  ** behaviors.  See the division above that contains the IOMETHODS
  ** macro for addition information on finder-functions.
  **
  ** Most finders simply return a pointer to a fixed sqlite3_io_methods
  ** object.  But the "autolockIoFinder" available on MacOSX does a little
  ** more than that; it looks at the filesystem type that hosts the
  ** database file and tries to choose an locking method appropriate for
  ** that filesystem time.
  */
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UNIXVFS(VFSNAME, FINDER) {                        \
    3,                    /* iVersion */                    \
    ਆਕਾਰ(unixFile),     /* szOsFile */                    \
    MAX_PATHNAME,         /* mxPathname */                  \
    0,                    /* pNext */                       \
    VFSNAME,              /* zName */                       \
    (ਖਾਲੀ*)&FINDER,       /* pAppData */                    \
    unixOpen,             /* xOpen */                       \
    unixDelete,           /* xDelete */                     \
    unixAccess,           /* xAccess */                     \
    unix_ਭਰਿਆ_Pathname,     /* xFullPathname */               \
    unixDlOpen,           /* xDlOpen */                     \
    unixDlError,          /* xDlError */                    \
    unixDlSym,            /* xDlSym */                      \
    unixDlClose,          /* xDlClose */                    \
    unixRandomness,       /* xRandomness */                 \
    unixSleep,            /* xSleep */                      \
    unix_Current_ਸਮਾਂ,      /* xCurrentTime */                \
    unix_Get_ਆਖਰੀ_Error,     /* xGetLastError */               \
    unix_Current_ਸਮਾਂ_Int64, /* xCurrentTimeInt64 */           \
    unix_ਸੈੱਟ_System_Call,    /* xSetSystemCall */              \
    unixGetSystemCall,    /* xGetSystemCall */              \
    unix_ਅਗਲਾ_System_Call,   /* xNextSystemCall */             \
  }

  /*
  ** All default VFSes for unix are contained in the following array.
  **
  ** Note that the sqlite3_vfs.pNext field of the VFS object is modified
  ** by the SQLite core when the VFS is registered.  So the following
  ** array cannot be const.
  */
  ਸਥਿਰ sqlite3_vfs aVfs[] = {
#ਜੇ SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    UNIXVFS("unix",          autolockIoFinder ),
#ਜਾਂਜੇ OS_VXWORKS
    UNIXVFS("unix",          vxworksIoFinder ),
#ਨਹੀਂਤਾਂ
    UNIXVFS("unix",          posixIoFinder ),
#ਜੇ_ਅੰਤ
    UNIXVFS("unix-none",     nolockIoFinder ),
    UNIXVFS("unix-dotfile",  dotlockIoFinder ),
    UNIXVFS("unix-excl",     posixIoFinder ),
#ਜੇ OS_VXWORKS
    UNIXVFS("unix-namedsem", semIoFinder ),
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_ENABLE_LOCKING_STYLE || OS_VXWORKS
    UNIXVFS("unix-posix",    posixIoFinder ),
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_ENABLE_LOCKING_STYLE
    UNIXVFS("unix-flock",    flockIoFinder ),
#ਜੇ_ਅੰਤ
#ਜੇ SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    UNIXVFS("unix-afp",      afpIoFinder ),
    UNIXVFS("unix-nfs",      nfsIoFinder ),
    UNIXVFS("unix-proxy",    proxyIoFinder ),
#ਜੇ_ਅੰਤ
  };
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;          /* Loop counter */

  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  ਪ੍ਰਤਿਜ੍ਞਾ( Array_ਮਾਪ(aSyscall)==29 );

  /* Register all VFSes defined in the aVfs[] array */
  ਲਈ(i=0; i<(ਆਕਾਰ(aVfs)/ਆਕਾਰ(sqlite3_vfs)); i++){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_UNIX_VFS
    sqlite3_vfs_ਰਜਿਸਟਰ(&aVfs[i],
           0==strcmp(aVfs[i].zName,SQLITE_DEFAULT_UNIX_VFS));
#ਨਹੀਂਤਾਂ
    sqlite3_vfs_ਰਜਿਸਟਰ(&aVfs[i], i==0);
#ਜੇ_ਅੰਤ
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OS_KV_OPTIONAL
  sqlite3KvvfsInit();
#ਜੇ_ਅੰਤ
  unix_Big_ਤਾਲਾ = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#ਜੇ OS_VXWORKS
  vxworksMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS2);
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  /* Validate lock assumptions */
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_SHM_NLOCK==8 );  /* Number of available locks */
  ਪ੍ਰਤਿਜ੍ਞਾ( UNIX_SHM_ਅਧਾਰ==120  );  /* Start of locking area */
  /* Locks:
  **    WRITE       UNIX_SHM_BASE      120
  **    CKPT        UNIX_SHM_BASE+1    121
  **    RECOVER     UNIX_SHM_BASE+2    122
  **    READ-0      UNIX_SHM_BASE+3    123
  **    READ-1      UNIX_SHM_BASE+4    124
  **    READ-2      UNIX_SHM_BASE+5    125
  **    READ-3      UNIX_SHM_BASE+6    126
  **    READ-4      UNIX_SHM_BASE+7    127
  **    DMS         UNIX_SHM_BASE+8    128
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( UNIX_SHM_DMS==128   );  /* Byte offset of the deadman-switch */
#ਜੇ_ਅੰਤ

  ਵਾਪਸ SQLITE_OK;
}

/*
** Shutdown the operating system interface.
**
** Some operating systems might need to do some cleanup in this routine,
** to release dynamically allocated objects.  But not on unix.
** This routine is a no-op for unix.
*/
ਪੂਰਨ sqlite3_os_ਸਿਰਾ(ਖਾਲੀ){
  unix_Big_ਤਾਲਾ = 0;
#ਜੇ OS_VXWORKS
  vxworksMutex = 0;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ_ਅੰਤ /* SQLITE_OS_UNIX */
