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
#అన్తర్భావ "sqliteInt.h"
#ఒకవేళ SQLITE_OS_UNIX              /* This file is used on unix only */

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
#ఒకవేళ !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  ఒకవేళ defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  లేకపోతే
#    define SQLITE_ENABLE_LOCKING_STYLE 0
#  endif
#అయితే_ముగింపు

/* Use pread() and pwrite() if they are available */
#ఒకవేళ defined(__APPLE__) || defined(__linux__)
# define HAVE_PREAD 1
# define HAVE_PWRITE 1
#అయితే_ముగింపు
#ఒకవేళ defined(HAVE_PREAD64) && defined(HAVE_PWRITE64)
# undef USE_PREAD
# define USE_PREAD64 1
#లేదావేళ defined(HAVE_PREAD) && defined(HAVE_PWRITE)
# undef USE_PREAD64
# define USE_PREAD 1
#అయితే_ముగింపు

/*
** standard include files.
*/
#అన్తర్భావ <sys/types.h>   /* amalgamator: keep */
#అన్తర్భావ <sys/stat.h>    /* amalgamator: keep */
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <sys/ioctl.h>
#అన్తర్భావ <unistd.h>      /* amalgamator: keep */
#అన్తర్భావ <time.h>
#అన్తర్భావ <sys/time.h>    /* amalgamator: keep */
#అన్తర్భావ <errno.h>
#ఒకవేళ (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_కొలత>0) \
  && !defined(SQLITE_WASI)
# include <sys/mman.h>
#అయితే_ముగింపు

#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE
# include <sys/ioctl.h>
# include <sys/file.h>
# include <sys/param.h>
#అయితే_ముగింపు /* SQLITE_ENABLE_LOCKING_STYLE */

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
#నిర్వచితం_కాకపోతే HAVE_GETHOSTUUID
# define HAVE_GETHOSTUUID 0
# ఒకవేళ defined(__APPLE__) && ((__MAC_OS_X_VERSION_MIN_REQUIRED > 1050) || \
                            (__IPHONE_OS_VERSION_MIN_REQUIRED > 2000))
#    ఒకవేళ (!defined(లక్ష్యం_OS_EMBEDDED) || (లక్ష్యం_OS_EMBEDDED==0)) \
        && (!defined(లక్ష్యం_IPHONE_SIMULATOR) || (లక్ష్యం_IPHONE_SIMULATOR==0))\
        && (!defined(లక్ష్యం_OS_MACCATALYST) || (లక్ష్యం_OS_MACCATALYST==0))
#      undef HAVE_GETHOSTUUID
#      define HAVE_GETHOSTUUID 1
#    లేకపోతే
#      warning "gethostuuid() is disabled."
#    endif
#  endif
#అయితే_ముగింపు


#ఒకవేళ OS_VXWORKS
# include <sys/ioctl.h>
# include <semaphore.h>
# include <limits.h>
#అయితే_ముగింపు /* OS_VXWORKS */

#ఒకవేళ defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
# include <sys/mount.h>
#అయితే_ముగింపు

#నిర్వచితమైతే HAVE_UTIME
# include <utime.h>
#అయితే_ముగింపు

/*
** Allowed values of unixFile.fsFlags
*/
#నిర్వచించు SQLITE_FSFLAGS_IS_MSDOS     0x1

/*
** If we are to be thread-safe, include the pthreads header.
*/
#ఒకవేళ SQLITE_THREADSAFE
# include <pthread.h>
#అయితే_ముగింపు

/*
** Default permissions when creating a new file
*/
#నిర్వచితం_కాకపోతే SQLITE_DEFAULT_FILE_PERMISSIONS
# define SQLITE_DEFAULT_FILE_PERMISSIONS 0644
#అయితే_ముగింపు

/*
** Default permissions when creating auto proxy dir
*/
#నిర్వచితం_కాకపోతే SQLITE_DEFAULT_PROXYDIR_PERMISSIONS
# define SQLITE_DEFAULT_PROXYDIR_PERMISSIONS 0755
#అయితే_ముగింపు

/*
** Maximum supported path-length.
*/
#నిర్వచించు MAX_PATHNAME 512

/*
** Maximum supported symbolic links
*/
#నిర్వచించు SQLITE_MAX_SYMLINKS 100

/*
** Remove and stub certain info for WASI (WebAssembly System
** Interface) builds.
*/
#నిర్వచితమైతే SQLITE_WASI
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
#  ఒకవేళ __LONG_MAX == 0x7fffffffL
#   define F_GETLK 12
#   define F_SETLK 13
#   define F_SETLKW 14
#  లేకపోతే
#   define F_GETLK 5
#   define F_SETLK 6
#   define F_SETLKW 7
#  endif
# endif
#లేకపోతే /* !SQLITE_WASI */
# ifndef HAVE_FCHMOD
#  define HAVE_FCHMOD 1
# endif
#అయితే_ముగింపు /* SQLITE_WASI */

#నిర్వచితమైతే SQLITE_WASI
# define osGetpid(X) (pid_t)1
#లేకపోతే
/* Always cast the getpid() return type for compatibility with
** kernel modules in VxWorks. */
# define osGetpid(X) (pid_t)getpid()
#అయితే_ముగింపు

/*
** Only set the lastErrno if the error code is a real error and not
** a normal expected return code of SQLITE_BUSY or SQLITE_OK
*/
#నిర్వచించు IS_తాళం_ERROR(x)  ((x != SQLITE_OK) && (x != SQLITE_BUSY))

/* Forward references */
రకనిర్వచనం నిర్మాణం unixShm unixShm;               /* Connection shared memory */
రకనిర్వచనం నిర్మాణం unix_Shm_కణుపు unix_Shm_కణుపు;       /* Shared memory instance */
రకనిర్వచనం నిర్మాణం unixInodeInfo unixInodeInfo;   /* An i-node */
రకనిర్వచనం నిర్మాణం Unix_వాడని_Fd Unix_వాడని_Fd;     /* An unused file descriptor */

/*
** Sometimes, after a file handle is closed by SQLite, the file descriptor
** cannot be closed immediately. In these cases, instances of the following
** structure are used to store the file descriptor while waiting for an
** opportunity to either close or reuse it.
*/
నిర్మాణం Unix_వాడని_Fd {
  పూర్ణాంకం fd;                   /* File descriptor to close */
  పూర్ణాంకం flags;                /* Flags this file descriptor was opened with */
  Unix_వాడని_Fd *p_తదుపరి;      /* Next unused file descriptor on same file */
};

/*
** The unixFile structure is subclass of sqlite3_file specific to the unix
** VFS implementations.
*/
రకనిర్వచనం నిర్మాణం unixFile unixFile;
నిర్మాణం unixFile {
  sqlite3_io_methods స్థిరరాశి *p_పద్ధతి;  /* Always the first entry */
  sqlite3_vfs *pVfs;                  /* The VFS that created this unixFile */
  unixInodeInfo *pInode;              /* Info about locks on this inode */
  పూర్ణాంకం h;                              /* The file descriptor */
  గుర్తులేని అక్షరం e_File_తాళం;            /* The type of lock held on this fd */
  గుర్తులేని పొట్టి పూర్ణాంకం ctrlFlags;       /* Behavioral bits.  UNIXFILE_* flags */
  పూర్ణాంకం చివరి_Errno;                      /* The unix errno from last I/O error */
  శూన్య *lockingContext;               /* Locking style specific state */
  Unix_వాడని_Fd *p_Preallocated_వాడని;  /* Pre-allocated UnixUnusedFd */
  స్థిరరాశి అక్షరం *z_మార్గం;                  /* Name of the file */
  unixShm *pShm;                      /* Shared memory segment information */
  పూర్ణాంకం szChunk;                        /* Configured by FCNTL_CHUNK_SIZE */
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  పూర్ణాంకం n_Fetch_బహిర్గతం;                      /* Number of outstanding xFetch refs */
  sqlite3_int64 mmap_కొలత;             /* Usable size of mapping at pMapRegion */
  sqlite3_int64 mmap_కొలత_Actual;       /* Actual size of mapping at pMapRegion */
  sqlite3_int64 mmap_కొలత_Max;          /* Configured FCNTL_MMAP_SIZE value */
  శూన్య *pMapRegion;                   /* Memory mapped region */
#అయితే_ముగింపు
  పూర్ణాంకం sector_కొలత;                     /* Device sector size */
  పూర్ణాంకం deviceCharacteristics;          /* Precomputed device characteristics */
#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE
  పూర్ణాంకం openFlags;                      /* The flags specified at open() */
#అయితే_ముగింపు
#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE || defined(__APPLE__)
  గుర్తులేని fsFlags;                   /* cached details from statfs() */
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  గుర్తులేని iBusyTimeout;              /* Wait this many millisec on locks */
  పూర్ణాంకం b_ఖండం_On_Connect;                /* True to block for SHARED locks */
#అయితే_ముగింపు
#ఒకవేళ OS_VXWORKS
  నిర్మాణం vxworksFileId *pId;          /* Unique file ID */
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_DEBUG
  /* The next group of variables are used to track whether or not the
  ** transaction counter in bytes 24-27 of database files are updated
  ** whenever any part of the database changes.  An assertion fault will
  ** occur if a file is updated without also updating the transaction
  ** counter.  This test is made to avoid new problems similar to the
  ** one described by ticket #3584.
  */
  గుర్తులేని అక్షరం transCntrChng;   /* True if the transaction counter changed */
  గుర్తులేని అక్షరం dbUpdate;        /* True if any part of database file changed */
  గుర్తులేని అక్షరం లోపల_సాధారణ_Write;   /* True if in a normal write operation */

#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_TEST
  /* In test mode, increase the size of this structure a bit so that
  ** it is larger than the struct CrashFile defined in test6.c.
  */
  అక్షరం aPadding[32];
#అయితే_ముగింపు
};

/* This variable holds the process id (pid) from when the xRandomness()
** method was called.  If xOpen() is called from a different process id,
** indicating that a fork() has occurred, the PRNG will be reset.
*/
స్థిర pid_t randomnessPid = 0;

/*
** Allowed values for the unixFile.ctrlFlags bitmask:
*/
#నిర్వచించు UNIXFILE_EXCL        0x01     /* Connections from one process only */
#నిర్వచించు UNIXFILE_RDONLY      0x02     /* Connection is read only */
#నిర్వచించు UNIXFILE_PERSIST_WAL 0x04     /* Persistent WAL mode */
#ఒకవేళ !defined(SQLITE_DISABLE_DIRSYNC) && !defined(_AIX)
# define UNIXFILE_DIRSYNC    0x08     /* Directory sync needed */
#లేకపోతే
# define UNIXFILE_DIRSYNC    0x00
#అయితే_ముగింపు
#నిర్వచించు UNIXFILE_PSOW        0x10     /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */
#నిర్వచించు UNIXFILE_DELETE      0x20     /* Delete on close */
#నిర్వచించు UNIXFILE_URI         0x40     /* Filename might have query parameters */
#నిర్వచించు UNIXFILE_NOLOCK      0x80     /* Do no file locking */

/*
** Include code that is common to all os_*.c files
*/
#అన్తర్భావ "os_common.h"

/*
** Define various macros that are missing from some systems.
*/
#నిర్వచితం_కాకపోతే O_LARGEFILE
# define O_LARGEFILE 0
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_DISABLE_LFS
# undef O_LARGEFILE
# define O_LARGEFILE 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే O_NOFOLLOW
# define O_NOFOLLOW 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే O_ద్విమాన
# define O_ద్విమాన 0
#అయితే_ముగింపు

/*
** The threadid macro resolves to the thread-id or to 0.  Used for
** testing and debugging only.
*/
#ఒకవేళ SQLITE_THREADSAFE
#నిర్వచించు threadid pthread_self()
#లేకపోతే
#నిర్వచించు threadid 0
#అయితే_ముగింపు

/*
** HAVE_MREMAP defaults to true on Linux and false everywhere else.
*/
#ఒకవేళ !defined(HAVE_MREMAP)
# ఒకవేళ defined(__linux__) && defined(GNU_ఆధారం)
#  define HAVE_MREMAP 1
# లేకపోతే
#  define HAVE_MREMAP 0
# endif
#అయితే_ముగింపు

/*
** Explicitly call the 64-bit version of lseek() on Android. Otherwise, lseek()
** is the 32-bit version, even if _FILE_OFFSET_BITS=64 is defined.
*/
#నిర్వచితమైతే __ANDROID__
# define lseek lseek64
#అయితే_ముగింపు

#నిర్వచితమైతే __linux__
/*
** Linux-specific IOCTL magic numbers used for controlling F2FS
*/
#నిర్వచించు F2FS_IOCTL_MAGIC        0xf5
#నిర్వచించు F2_FS_IOC_ప్రారంభం_ATOMIC_WRITE     _IO(F2FS_IOCTL_MAGIC, 1)
#నిర్వచించు F2FS_IOC_COMMIT_ATOMIC_WRITE    _IO(F2FS_IOCTL_MAGIC, 2)
#నిర్వచించు F2_FS_IOC_ప్రారంభం_VOLATILE_WRITE   _IO(F2FS_IOCTL_MAGIC, 3)
#నిర్వచించు F2FS_IOC_ABORT_VOLATILE_WRITE   _IO(F2FS_IOCTL_MAGIC, 5)
#నిర్వచించు F2FS_IOC_GET_FEATURES           _IOR(F2FS_IOCTL_MAGIC, 12, u32)
#నిర్వచించు F2FS_FEATURE_ATOMIC_WRITE 0x0004
#అయితే_ముగింపు /* __linux__ */


/*
** Different Unix systems declare open() in different ways.  Same use
** open(const char*,int,mode_t).  Others use open(const char*,int,...).
** The difference is important when using a pointer to the function.
**
** The safest way to deal with the problem is to always use this wrapper
** which always has the same well-defined interface.
*/
స్థిర పూర్ణాంకం posixOpen(స్థిరరాశి అక్షరం *zFile, పూర్ణాంకం flags, పూర్ణాంకం బహుళకం){
  తిరిగి open(zFile, flags, బహుళకం);
}

/* Forward reference */
స్థిర పూర్ణాంకం openDirectory(స్థిరరాశి అక్షరం*, పూర్ణాంకం*);
స్థిర పూర్ణాంకం unixGetpagesize(శూన్య);

/*
** Many system calls are accessed through pointer-to-functions so that
** they may be overridden at runtime to facilitate fault injection during
** testing and sandboxing.  The following array holds the names and pointers
** to all overrideable system calls.
*/
స్థిర నిర్మాణం unix_syscall {
  స్థిరరాశి అక్షరం *zName;            /* Name of the system call */
  sqlite3_syscall_ptr pCurrent; /* Current value of the system call */
  sqlite3_syscall_ptr pDefault; /* Default value */
} aSyscall[] = {
  { "open",         (sqlite3_syscall_ptr)posixOpen,  0  },
#నిర్వచించు osOpen      ((పూర్ణాంకం(*)(స్థిరరాశి అక్షరం*,పూర్ణాంకం,పూర్ణాంకం))aSyscall[0].pCurrent)

  { "close",        (sqlite3_syscall_ptr)close,      0  },
#నిర్వచించు osClose     ((పూర్ణాంకం(*)(పూర్ణాంకం))aSyscall[1].pCurrent)

  { "access",       (sqlite3_syscall_ptr)access,     0  },
#నిర్వచించు osAccess    ((పూర్ణాంకం(*)(స్థిరరాశి అక్షరం*,పూర్ణాంకం))aSyscall[2].pCurrent)

  { "getcwd",       (sqlite3_syscall_ptr)getcwd,     0  },
#నిర్వచించు osGetcwd    ((అక్షరం*(*)(అక్షరం*,size_t))aSyscall[3].pCurrent)

  { "stat",         (sqlite3_syscall_ptr)stat,       0  },
#నిర్వచించు osStat      ((పూర్ణాంకం(*)(స్థిరరాశి అక్షరం*,నిర్మాణం stat*))aSyscall[4].pCurrent)

/*
** The DJGPP compiler environment looks mostly like Unix, but it
** lacks the fcntl() system call.  So redefine fcntl() to be something
** that always succeeds.  This means that locking does not occur under
** DJGPP.  But it is DOS - what did you expect?
*/
#నిర్వచితమైతే __DJGPP__
  { "fstat",        0,                 0  },
#నిర్వచించు osFstat(a,b,c)    0
#లేకపోతే   
  { "fstat",        (sqlite3_syscall_ptr)fstat,      0  },
#నిర్వచించు osFstat     ((పూర్ణాంకం(*)(పూర్ణాంకం,నిర్మాణం stat*))aSyscall[5].pCurrent)
#అయితే_ముగింపు

  { "ftruncate",    (sqlite3_syscall_ptr)ftruncate,  0  },
#నిర్వచించు osFtruncate ((పూర్ణాంకం(*)(పూర్ణాంకం,off_t))aSyscall[6].pCurrent)

  { "fcntl",        (sqlite3_syscall_ptr)fcntl,      0  },
#నిర్వచించు osFcntl     ((పూర్ణాంకం(*)(పూర్ణాంకం,పూర్ణాంకం,...))aSyscall[7].pCurrent)

  { "read",         (sqlite3_syscall_ptr)పఠ,       0  },
#నిర్వచించు osRead      ((ssize_t(*)(పూర్ణాంకం,శూన్య*,size_t))aSyscall[8].pCurrent)

#ఒకవేళ defined(USE_PREAD) || SQLITE_ENABLE_LOCKING_STYLE
  { "pread",        (sqlite3_syscall_ptr)pread,      0  },
#లేకపోతే
  { "pread",        (sqlite3_syscall_ptr)0,          0  },
#అయితే_ముగింపు
#నిర్వచించు osPread     ((ssize_t(*)(పూర్ణాంకం,శూన్య*,size_t,off_t))aSyscall[9].pCurrent)

#ఒకవేళ defined(USE_PREAD64)
  { "pread64",      (sqlite3_syscall_ptr)pread64,    0  },
#లేకపోతే
  { "pread64",      (sqlite3_syscall_ptr)0,          0  },
#అయితే_ముగింపు
#నిర్వచించు osPread64 ((ssize_t(*)(పూర్ణాంకం,శూన్య*,size_t,off64_t))aSyscall[10].pCurrent)

  { "write",        (sqlite3_syscall_ptr)లిఖ,      0  },
#నిర్వచించు osWrite     ((ssize_t(*)(పూర్ణాంకం,స్థిరరాశి శూన్య*,size_t))aSyscall[11].pCurrent)

#ఒకవేళ defined(USE_PREAD) || SQLITE_ENABLE_LOCKING_STYLE
  { "pwrite",       (sqlite3_syscall_ptr)pwrite,     0  },
#లేకపోతే
  { "pwrite",       (sqlite3_syscall_ptr)0,          0  },
#అయితే_ముగింపు
#నిర్వచించు osPwrite    ((ssize_t(*)(పూర్ణాంకం,స్థిరరాశి శూన్య*,size_t,off_t))\
                    aSyscall[12].pCurrent)

#ఒకవేళ defined(USE_PREAD64)
  { "pwrite64",     (sqlite3_syscall_ptr)pwrite64,   0  },
#లేకపోతే
  { "pwrite64",     (sqlite3_syscall_ptr)0,          0  },
#అయితే_ముగింపు
#నిర్వచించు osPwrite64  ((ssize_t(*)(పూర్ణాంకం,స్థిరరాశి శూన్య*,size_t,off64_t))\
                    aSyscall[13].pCurrent)

#ఒకవేళ defined(HAVE_FCHMOD)
  { "fchmod",       (sqlite3_syscall_ptr)fchmod,          0  },
#నిర్వచించు osFchmod    ((పూర్ణాంకం(*)(పూర్ణాంకం,బహుళకం_t))aSyscall[14].pCurrent)
#లేకపోతే
  { "fchmod",       (sqlite3_syscall_ptr)0,               0  },
#నిర్వచించు osFchmod(FID,బహుళకం_2) 0
#అయితే_ముగింపు

#ఒకవేళ defined(HAVE_POSIX_FALLOCATE) && HAVE_POSIX_FALLOCATE
  { "fallocate",    (sqlite3_syscall_ptr)posix_fallocate,  0 },
#లేకపోతే
  { "fallocate",    (sqlite3_syscall_ptr)0,                0 },
#అయితే_ముగింపు
#నిర్వచించు osFallocate ((పూర్ణాంకం(*)(పూర్ణాంకం,off_t,off_t))aSyscall[15].pCurrent)

  { "unlink",       (sqlite3_syscall_ptr)unlink,           0 },
#నిర్వచించు osUnlink    ((పూర్ణాంకం(*)(స్థిరరాశి అక్షరం*))aSyscall[16].pCurrent)

  { "openDirectory",    (sqlite3_syscall_ptr)openDirectory,      0 },
#నిర్వచించు osOpenDirectory ((పూర్ణాంకం(*)(స్థిరరాశి అక్షరం*,పూర్ణాంకం*))aSyscall[17].pCurrent)

  { "mkdir",        (sqlite3_syscall_ptr)mkdir,           0 },
#నిర్వచించు osMkdir     ((పూర్ణాంకం(*)(స్థిరరాశి అక్షరం*,బహుళకం_t))aSyscall[18].pCurrent)

  { "rmdir",        (sqlite3_syscall_ptr)rmdir,           0 },
#నిర్వచించు osRmdir     ((పూర్ణాంకం(*)(స్థిరరాశి అక్షరం*))aSyscall[19].pCurrent)

#ఒకవేళ defined(HAVE_FCHOWN)
  { "fchown",       (sqlite3_syscall_ptr)fchown,          0 },
#లేకపోతే
  { "fchown",       (sqlite3_syscall_ptr)0,               0 },
#అయితే_ముగింపు
#నిర్వచించు osFchown    ((పూర్ణాంకం(*)(పూర్ణాంకం,uid_t,gid_t))aSyscall[20].pCurrent)

#ఒకవేళ defined(HAVE_FCHOWN)
  { "geteuid",      (sqlite3_syscall_ptr)geteuid,         0 },
#లేకపోతే
  { "geteuid",      (sqlite3_syscall_ptr)0,               0 },
#అయితే_ముగింపు
#నిర్వచించు osGeteuid   ((uid_t(*)(శూన్య))aSyscall[21].pCurrent)

#ఒకవేళ (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_కొలత>0) \
  && !defined(SQLITE_WASI)
  { "mmap",         (sqlite3_syscall_ptr)mmap,            0 },
#లేకపోతే
  { "mmap",         (sqlite3_syscall_ptr)0,               0 },
#అయితే_ముగింపు
#నిర్వచించు osMmap ((శూన్య*(*)(శూన్య*,size_t,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం,off_t))aSyscall[22].pCurrent)

#ఒకవేళ (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_కొలత>0) \
  && !defined(SQLITE_WASI)
  { "munmap",       (sqlite3_syscall_ptr)munmap,          0 },
#లేకపోతే
  { "munmap",       (sqlite3_syscall_ptr)0,               0 },
#అయితే_ముగింపు
#నిర్వచించు osMunmap ((పూర్ణాంకం(*)(శూన్య*,size_t))aSyscall[23].pCurrent)

#ఒకవేళ HAVE_MREMAP && (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_కొలత>0)
  { "mremap",       (sqlite3_syscall_ptr)mremap,          0 },
#లేకపోతే
  { "mremap",       (sqlite3_syscall_ptr)0,               0 },
#అయితే_ముగింపు
#నిర్వచించు osMremap ((శూన్య*(*)(శూన్య*,size_t,size_t,పూర్ణాంకం,...))aSyscall[24].pCurrent)

#ఒకవేళ !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_కొలత>0
  { "getpagesize",  (sqlite3_syscall_ptr)unixGetpagesize, 0 },
#లేకపోతే
  { "getpagesize",  (sqlite3_syscall_ptr)0,               0 },
#అయితే_ముగింపు
#నిర్వచించు osGetpagesize ((పూర్ణాంకం(*)(శూన్య))aSyscall[25].pCurrent)

#ఒకవేళ defined(HAVE_READLINK)
  { "readlink",     (sqlite3_syscall_ptr)readlink,        0 },
#లేకపోతే
  { "readlink",     (sqlite3_syscall_ptr)0,               0 },
#అయితే_ముగింపు
#నిర్వచించు osReadlink ((ssize_t(*)(స్థిరరాశి అక్షరం*,అక్షరం*,size_t))aSyscall[26].pCurrent)

#ఒకవేళ defined(HAVE_LSTAT)
  { "lstat",         (sqlite3_syscall_ptr)lstat,          0 },
#లేకపోతే
  { "lstat",         (sqlite3_syscall_ptr)0,              0 },
#అయితే_ముగింపు
#నిర్వచించు osLstat      ((పూర్ణాంకం(*)(స్థిరరాశి అక్షరం*,నిర్మాణం stat*))aSyscall[27].pCurrent)

#ఒకవేళ defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
# ifdef __ANDROID__
  { "ioctl", (sqlite3_syscall_ptr)(పూర్ణాంకం(*)(పూర్ణాంకం, పూర్ణాంకం, ...))ioctl, 0 },
#నిర్వచించు osIoctl ((పూర్ణాంకం(*)(పూర్ణాంకం,పూర్ణాంకం,...))aSyscall[28].pCurrent)
# లేకపోతే
  { "ioctl",         (sqlite3_syscall_ptr)ioctl,          0 },
#నిర్వచించు osIoctl ((పూర్ణాంకం(*)(పూర్ణాంకం,గుర్తులేని దీర్ఘ,...))aSyscall[28].pCurrent)
# endif
#లేకపోతే
  { "ioctl",         (sqlite3_syscall_ptr)0,              0 },
#అయితే_ముగింపు

}; /* End of the overrideable system calls */


#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
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
స్థిర పూర్ణాంకం unixPosixAdvisoryLocks(
  పూర్ణాంకం fd,        /* The file descriptor to analyze */
  అక్షరం a[16]     /* Write a text description of PALs here */
){
  పూర్ణాంకం లోపల;
  ssize_t n;
  అక్షరం *p, *p_తదుపరి, *x;
  అక్షరం z[2000];

        /*             1     */
        /*   012 4 678 01234 */
  జ్ఞాపకం_నకలు(a, "---/-.---.-----", 16);
  sqlite3_snprintf(పరిమాణం(z), z, "/proc/%d/fdinfo/%d", getpid(), fd);
  లోపల = osOpen(z, O_RDONLY, 0);
  ఒకవేళ( లోపల<0 ){
    తిరిగి SQLITE_ERROR_UNABLE;
  }
  n = osRead(లోపల, z, పరిమాణం(z)-1);
  osClose(లోపల);
  ఒకవేళ( n<=0 ) తిరిగి SQLITE_ERROR_UNABLE;
  z[n] = 0;

  /* We are looking for lines that begin with "lock:\t".  Examples:
  **
  ** lock: 1: POSIX  ADVISORY  READ 494716 08:02:5277597 1073741826 1073742335
  ** lock: 1: POSIX  ADVISORY  WRITE 494716 08:02:5282282 120 120
  ** lock: 2: POSIX  ADVISORY  READ 494716 08:02:5282282 123 123
  ** lock: 3: POSIX  ADVISORY  READ 494716 08:02:5282282 128 128
  */
  p_తదుపరి = అక్షరశ్రేణి_శోధన(z, "lock:\t");
  అయితే( p_తదుపరి ){
    అక్షరం cType = 0;
    sqlite3_int64 i_మొదటి, i_చివరి;
    p = p_తదుపరి+6;
    p_తదుపరి = అక్షరశ్రేణి_శోధన(p, "lock:\t");
    ఒకవేళ( p_తదుపరి ) p_తదుపరి[-1] = 0;
    ఒకవేళ( (x = అక్షరశ్రేణి_శోధన(p, " READ "))!=0 ){
      cType = 'r';
      x += 6;
    }లేకపోతే ఒకవేళ( (x = అక్షరశ్రేణి_శోధన(p, " WRITE "))!=0 ){
      cType = 'w';
      x += 7;
    }లేకపోతే{
      కొనసాగు;
    }
    x = strrchr(x, ' ');
    ఒకవేళ( x==0 ) కొనసాగు;
    i_చివరి = strtoll(x+1, 0, 10);
    *x = 0;
    x = strrchr(p, ' ');
    ఒకవేళ( x==0 ) కొనసాగు;
    i_మొదటి = strtoll(x+1, 0, 10);
    ఒకవేళ( i_చివరి>=PENDING_BYTE ){
      ఒకవేళ( i_మొదటి<=PENDING_BYTE && i_చివరి>=PENDING_BYTE )     a[0] = cType;
      ఒకవేళ( i_మొదటి<=PENDING_BYTE+1 && i_చివరి>=PENDING_BYTE+1 ) a[1] = cType;
      ఒకవేళ( i_మొదటి<=PENDING_BYTE+2 && i_చివరి>=PENDING_BYTE+510 ) a[2] = cType;
    }లేకపోతే ఒకవేళ( i_చివరి<=128 ){
      ఒకవేళ( i_మొదటి<=128 && i_చివరి>=128 ) a[4] = cType;
      ఒకవేళ( i_మొదటి<=120 && i_చివరి>=120 ) a[6] = cType;
      ఒకవేళ( i_మొదటి<=121 && i_చివరి>=121 ) a[7] = cType;
      ఒకవేళ( i_మొదటి<=122 && i_చివరి>=122 ) a[8] = cType;
      ఒకవేళ( i_మొదటి<=123 && i_చివరి>=123 ) a[10] = cType;
      ఒకవేళ( i_మొదటి<=124 && i_చివరి>=124 ) a[11] = cType;
      ఒకవేళ( i_మొదటి<=125 && i_చివరి>=125 ) a[12] = cType;
      ఒకవేళ( i_మొదటి<=126 && i_చివరి>=126 ) a[13] = cType;
      ఒకవేళ( i_మొదటి<=127 && i_చివరి>=127 ) a[14] = cType;
    }
  }
  తిరిగి SQLITE_OK;
}
#లేకపోతే
#  define unixPosixAdvisoryLocks(A,B) SQLITE_ERROR_UNABLE
#అయితే_ముగింపు /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

/*
** On some systems, calls to fchown() will trigger a message in a security
** log if they come from non-root processes.  So avoid calling fchown() if
** we are not running as root.
*/
స్థిర పూర్ణాంకం robustFchown(పూర్ణాంకం fd, uid_t uid, gid_t gid){
#ఒకవేళ defined(HAVE_FCHOWN)
  తిరిగి osGeteuid() ? 0 : osFchown(fd,uid,gid);
#లేకపోతే
  తిరిగి 0;
#అయితే_ముగింపు
}

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "unix" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
స్థిర పూర్ణాంకం unix_సమితి_System_Call(
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
స్థిర sqlite3_syscall_ptr unixGetSystemCall(
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
స్థిర స్థిరరాశి అక్షరం *unix_తదుపరి_System_Call(sqlite3_vfs *p, స్థిరరాశి అక్షరం *zName){
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

/*
** Do not accept any file descriptor less than this value, in order to avoid
** opening database file using file descriptors that are commonly used for
** standard input, output, and error.
*/
#నిర్వచితం_కాకపోతే SQLITE_MINIMUM_FILE_DESCRIPTOR
# define SQLITE_MINIMUM_FILE_DESCRIPTOR 3
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం robust_open(స్థిరరాశి అక్షరం *z, పూర్ణాంకం f, బహుళకం_t m){
  పూర్ణాంకం fd;
  బహుళకం_t m2 = m ? m : SQLITE_DEFAULT_FILE_PERMISSIONS;
  అయితే(1){
#ఒకవేళ defined(O_CLOEXEC)
    fd = osOpen(z,f|O_CLOEXEC,m2);
#లేకపోతే
    fd = osOpen(z,f,m2);
#అయితే_ముగింపు
    ఒకవేళ( fd<0 ){
      ఒకవేళ( errno==EINTR ) కొనసాగు;
      ఆపు;
    }
    ఒకవేళ( fd>=SQLITE_MINIMUM_FILE_DESCRIPTOR ) ఆపు;
    ఒకవేళ( (f & (O_EXCL|O_CREAT))==(O_EXCL|O_CREAT) ){
      (శూన్య)osUnlink(z);
    }
    osClose(fd);
    sqlite3_log(SQLITE_WARNING,
                "attempt to open \"%s\" as file descriptor %d", z, fd);
    fd = -1;
    ఒకవేళ( osOpen("/dev/null", O_RDONLY, m)<0 ) ఆపు;
  }
  ఒకవేళ( fd>=0 ){
    ఒకవేళ( m!=0 ){
      నిర్మాణం stat statbuf;
      ఒకవేళ( osFstat(fd, &statbuf)==0
       && statbuf.st_కొలత==0
       && (statbuf.st_బహుళకం&0777)!=m
      ){
        osFchmod(fd, m);
      }
    }
#ఒకవేళ defined(FD_CLOEXEC) && (!defined(O_CLOEXEC) || O_CLOEXEC==0)
    osFcntl(fd, F_SETFD, osFcntl(fd, F_GETFD, 0) | FD_CLOEXEC);
#అయితే_ముగింపు
  }
  తిరిగి fd;
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
స్థిర sqlite3_mutex *unix_Big_తాళం = 0;
స్థిర శూన్య unixEnterMutex(శూన్య){
  ప్రతిజ్ఞా( sqlite3_mutex_notheld(unix_Big_తాళం) );  /* Not a recursive mutex */
  sqlite3_mutex_enter(unix_Big_తాళం);
}
స్థిర శూన్య unixLeaveMutex(శూన్య){
  ప్రతిజ్ఞా( sqlite3_mutex_held(unix_Big_తాళం) );
  sqlite3_mutex_leave(unix_Big_తాళం);
}
#నిర్వచితమైతే SQLITE_DEBUG
స్థిర పూర్ణాంకం unixMutexHeld(శూన్య) {
  తిరిగి sqlite3_mutex_held(unix_Big_తాళం);
}
#అయితే_ముగింపు


#నిర్వచితమైతే SQLITE_HAVE_OS_TRACE
/*
** Helper function for printing out trace information from debugging
** binaries. This returns the string representation of the supplied
** integer lock-type.
*/
స్థిర స్థిరరాశి అక్షరం *az_File_తాళం(పూర్ణాంకం e_File_తాళం){
  ఎంపిక( e_File_తాళం ){
    స్థితి NO_తాళం: తిరిగి "NONE";
    స్థితి SHARED_తాళం: తిరిగి "SHARED";
    స్థితి RESERVED_తాళం: తిరిగి "RESERVED";
    స్థితి PENDING_తాళం: తిరిగి "PENDING";
    స్థితి EXCLUSIVE_తాళం: తిరిగి "EXCLUSIVE";
  }
  తిరిగి "ERROR";
}
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_తాళం_TRACE
/*
** Print out information about all locking operations.
**
** This routine is used for troubleshooting locks on multithreaded
** platforms.  Enable by compiling with the -DSQLITE_LOCK_TRACE
** command-line option on the compiler.  This code is normally
** turned off.
*/
స్థిర పూర్ణాంకం తాళం_Trace(పూర్ణాంకం fd, పూర్ణాంకం op, నిర్మాణం flock *p){
  అక్షరం *zOpName, *zType;
  పూర్ణాంకం s;
  పూర్ణాంకం savedErrno;
  ఒకవేళ( op==F_GETLK ){
    zOpName = "GETLK";
  }లేకపోతే ఒకవేళ( op==F_SETLK ){
    zOpName = "SETLK";
  }లేకపోతే{
    s = osFcntl(fd, op, p);
    sqlite3DebugPrintf("fcntl unknown %d %d %d\n", fd, op, s);
    తిరిగి s;
  }
  ఒకవేళ( p->l_type==F_RDLCK ){
    zType = "RDLCK";
  }లేకపోతే ఒకవేళ( p->l_type==F_WRLCK ){
    zType = "WRLCK";
  }లేకపోతే ఒకవేళ( p->l_type==F_UNLCK ){
    zType = "UNLCK";
  }లేకపోతే{
    ప్రతిజ్ఞా( 0 );
  }
  ప్రతిజ్ఞా( p->l_whence==SEEK_SET );
  s = osFcntl(fd, op, p);
  savedErrno = errno;
  sqlite3DebugPrintf("fcntl %d %d %s %s %d %d %d %d\n",
     threadid, fd, zOpName, zType, (పూర్ణాంకం)p->l_ప్రారంభం, (పూర్ణాంకం)p->l_len,
     (పూర్ణాంకం)p->l_pid, s);
  ఒకవేళ( s==(-1) && op==F_SETLK && (p->l_type==F_RDLCK || p->l_type==F_WRLCK) ){
    నిర్మాణం flock l2;
    l2 = *p;
    osFcntl(fd, F_GETLK, &l2);
    ఒకవేళ( l2.l_type==F_RDLCK ){
      zType = "RDLCK";
    }లేకపోతే ఒకవేళ( l2.l_type==F_WRLCK ){
      zType = "WRLCK";
    }లేకపోతే ఒకవేళ( l2.l_type==F_UNLCK ){
      zType = "UNLCK";
    }లేకపోతే{
      ప్రతిజ్ఞా( 0 );
    }
    sqlite3DebugPrintf("fcntl-failure-reason: %s %d %d %d\n",
       zType, (పూర్ణాంకం)l2.l_ప్రారంభం, (పూర్ణాంకం)l2.l_len, (పూర్ణాంకం)l2.l_pid);
  }
  errno = savedErrno;
  తిరిగి s;
}
#నిర్వచనం_రద్దు osFcntl
#నిర్వచించు osFcntl తాళం_Trace
#అయితే_ముగింపు /* SQLITE_LOCK_TRACE */

/*
** Retry ftruncate() calls that fail due to EINTR
**
** All calls to ftruncate() within this file should be made through
** this wrapper.  On the Android platform, bypassing the logic below
** could lead to a corrupt database.
*/
స్థిర పూర్ణాంకం robust_ftruncate(పూర్ణాంకం h, sqlite3_int64 sz){
  పూర్ణాంకం rc;
#నిర్వచితమైతే __ANDROID__
  /* On Android, ftruncate() always uses 32-bit offsets, even if
  ** _FILE_OFFSET_BITS=64 is defined. This means it is unsafe to attempt to
  ** truncate a file to any size larger than 2GiB. Silently ignore any
  ** such attempts.  */
  ఒకవేళ( sz>(sqlite3_int64)0x7FFFFFFF ){
    rc = SQLITE_OK;
  }లేకపోతే
#అయితే_ముగింపు
  చేయి{ rc = osFtruncate(h,sz); }అయితే( rc<0 && errno==EINTR );
  తిరిగి rc;
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
స్థిర పూర్ణాంకం sqliteErrorFromPosixError(పూర్ణాంకం posixError, పూర్ణాంకం sqliteIOErr) {
  ప్రతిజ్ఞా( (sqliteIOErr == SQLITE_IOERR_తాళం) ||
          (sqliteIOErr == SQLITE_IOERR_UNLOCK) ||
          (sqliteIOErr == SQLITE_IOERR_RDLOCK) ||
          (sqliteIOErr == SQLITE_IOERR_CHECKRESERVEDLOCK) );
  ఎంపిక (posixError) {
  స్థితి EACCES:
  స్థితి EAGAIN:
  స్థితి ETIMEDOUT:
  స్థితి EBUSY:
  స్థితి EINTR:
  స్థితి ENOLCK:
    /* random NFS retry error, unless during file system support
     * introspection, in which it actually means what it says */
    తిరిగి SQLITE_BUSY;
  
  స్థితి EPERM:
    తిరిగి SQLITE_PERM;
  
  అప్రమేయ:
    తిరిగి sqliteIOErr;
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
నిర్మాణం vxworksFileId {
  నిర్మాణం vxworksFileId *p_తదుపరి;  /* Next in a list of them all */
  పూర్ణాంకం nRef;                     /* Number of references to this one */
  పూర్ణాంకం nName;                    /* Length of the zCanonicalName[] string */
  అక్షరం *zCanonicalName;         /* Canonical filename */
};

#ఒకవేళ OS_VXWORKS
/*
** All unique filenames are held on a linked list headed by this
** variable:
*/
స్థిర నిర్మాణం vxworksFileId *vxworksFileList = 0;
స్థిర sqlite3_mutex *vxworksMutex = 0;

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
స్థిర పూర్ణాంకం vxworksSimplifyName(అక్షరం *z, పూర్ణాంకం n){
  పూర్ణాంకం i, j;
  అయితే( n>1 && z[n-1]=='/' ){ n--; }
  కొరకు(i=j=0; i<n; i++){
    ఒకవేళ( z[i]=='/' ){
      ఒకవేళ( z[i+1]=='/' ) కొనసాగు;
      ఒకవేళ( z[i+1]=='.' && i+2<n && z[i+2]=='/' ){
        i += 1;
        కొనసాగు;
      }
      ఒకవేళ( z[i+1]=='.' && i+3<n && z[i+2]=='.' && z[i+3]=='/' ){
        అయితే( j>0 && z[j-1]!='/' ){ j--; }
        ఒకవేళ( j>0 ){ j--; }
        i += 2;
        కొనసాగు;
      }
    }
    z[j++] = z[i];
  }
  z[j] = 0;
  తిరిగి j;
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
స్థిర నిర్మాణం vxworksFileId *vxworks_కనుగొను_File_Id(స్థిరరాశి అక్షరం *zAbsoluteName){
  నిర్మాణం vxworksFileId *pNew;         /* search key and new file ID */
  నిర్మాణం vxworksFileId *pCandidate;   /* For looping over existing file IDs */
  పూర్ణాంకం n;                              /* Length of zAbsoluteName string */

  ప్రతిజ్ఞా( zAbsoluteName[0]=='/' );
  n = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zAbsoluteName);
  pNew = sqlite3_malloc64( పరిమాణం(*pNew) + (n+1) );
  ఒకవేళ( pNew==0 ) తిరిగి 0;
  pNew->zCanonicalName = (అక్షరం*)&pNew[1];
  జ్ఞాపకం_నకలు(pNew->zCanonicalName, zAbsoluteName, n+1);
  n = vxworksSimplifyName(pNew->zCanonicalName, n);

  /* Search for an existing entry that matching the canonical name.
  ** If found, increment the reference count and return a pointer to
  ** the existing file ID.
  */
  sqlite3_mutex_enter(vxworksMutex);
  కొరకు(pCandidate=vxworksFileList; pCandidate; pCandidate=pCandidate->p_తదుపరి){
    ఒకవేళ( pCandidate->nName==n
     && memcmp(pCandidate->zCanonicalName, pNew->zCanonicalName, n)==0
    ){
       sqlite3_free(pNew);
       pCandidate->nRef++;
       sqlite3_mutex_leave(vxworksMutex);
       తిరిగి pCandidate;
    }
  }

  /* No match was found.  We will make a new file ID */
  pNew->nRef = 1;
  pNew->nName = n;
  pNew->p_తదుపరి = vxworksFileList;
  vxworksFileList = pNew;
  sqlite3_mutex_leave(vxworksMutex);
  తిరిగి pNew;
}

/*
** Decrement the reference count on a vxworksFileId object.  Free
** the object when the reference count reaches zero.
*/
స్థిర శూన్య vxworksReleaseFileId(నిర్మాణం vxworksFileId *pId){
  sqlite3_mutex_enter(vxworksMutex);
  ప్రతిజ్ఞా( pId->nRef>0 );
  pId->nRef--;
  ఒకవేళ( pId->nRef==0 ){
    నిర్మాణం vxworksFileId **pp;
    కొరకు(pp=&vxworksFileList; *pp && *pp!=pId; pp = &((*pp)->p_తదుపరి)){}
    ప్రతిజ్ఞా( *pp==pId );
    *pp = pId->p_తదుపరి;
    sqlite3_free(pId);
  }
  sqlite3_mutex_leave(vxworksMutex);
}
#అయితే_ముగింపు /* OS_VXWORKS */
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
నిర్మాణం unixFileId {
  dev_t dev;                  /* Device number */
#ఒకవేళ OS_VXWORKS
  నిర్మాణం vxworksFileId *pId;  /* Unique file ID for vxworks. */
#లేకపోతే
  /* We are told that some versions of Android contain a bug that
  ** sizes ino_t at only 32-bits instead of 64-bits. (See
  ** https://android-review.googlesource.com/#/c/115351/3/dist/sqlite3.c)
  ** To work around this, always allocate 64-bits for the inode number.
  ** On small machines that only have 32-bit inodes, this wastes 4 bytes,
  ** but that should not be a big deal. */
  /* WAS:  ino_t ino;   */
  u64 ino;                   /* Inode number */
#అయితే_ముగింపు
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
నిర్మాణం unixInodeInfo {
  నిర్మాణం unixFileId fileId;       /* The lookup key */
  sqlite3_mutex *p_తాళం_Mutex;      /* Hold this mutex for... */
  పూర్ణాంకం nShared;                      /* Number of SHARED locks held */
  పూర్ణాంకం n_తాళం;                        /* Number of outstanding file locks */
  గుర్తులేని అక్షరం e_File_తాళం;          /* One of SHARED_LOCK, RESERVED_LOCK etc. */
  గుర్తులేని అక్షరం b_Process_తాళం;       /* An exclusive process lock is held */
  Unix_వాడని_Fd *p_వాడని;            /* Unused file descriptors to close */
  పూర్ణాంకం nRef;                       /* Number of pointers to this structure */
  unix_Shm_కణుపు *p_Shm_కణుపు;          /* Shared memory associated with this inode */
  unixInodeInfo *p_తదుపరి;           /* List of all unixInodeInfo objects */
  unixInodeInfo *pPrev;           /*    .... doubly linked */
#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE
  గుర్తులేని దీర్ఘ దీర్ఘ sharedByte;  /* for AFP simulated shared lock */
#అయితే_ముగింపు
#ఒకవేళ OS_VXWORKS
  sem_t *pSem;                    /* Named POSIX semaphore */
  అక్షరం aSemName[MAX_PATHNAME+2];  /* Name of that semaphore */
#అయితే_ముగింపు
};

/*
** A lists of all unixInodeInfo objects.
**
** Must hold unixBigLock in order to read or write this variable.
*/
స్థిర unixInodeInfo *inodeList = 0;  /* All unixInodeInfo objects */

#నిర్వచితమైతే SQLITE_DEBUG
/*
** True if the inode mutex (on the unixFile.pFileMutex field) is held, or not.
** This routine is used only within assert() to help verify correct mutex
** usage.
*/
పూర్ణాంకం unixFileMutexHeld(unixFile *pFile){
  ప్రతిజ్ఞా( pFile->pInode );
  తిరిగి sqlite3_mutex_held(pFile->pInode->p_తాళం_Mutex);
}
పూర్ణాంకం unixFileMutexNotheld(unixFile *pFile){
  ప్రతిజ్ఞా( pFile->pInode );
  తిరిగి sqlite3_mutex_notheld(pFile->pInode->p_తాళం_Mutex);
}
#అయితే_ముగింపు

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
#నిర్వచించు unixLogError(a,b,c)     unix_Log_Error_At_పంక్తి(a,b,c,__LINE__)
స్థిర పూర్ణాంకం unix_Log_Error_At_పంక్తి(
  పూర్ణాంకం errcode,                    /* SQLite error code */
  స్థిరరాశి అక్షరం *zFunc,              /* Name of OS function that failed */
  స్థిరరాశి అక్షరం *z_మార్గం,              /* File path associated with error */
  పూర్ణాంకం i_పంక్తి                       /* Source line number where error occurred */
){
  అక్షరం *zErr;                     /* Message from strerror() or equivalent */
  పూర్ణాంకం iErrno = errno;             /* Saved syscall error number */

  /* If this is not a threadsafe build (SQLITE_THREADSAFE==0), then use
  ** the strerror() function to obtain the human-readable error message
  ** equivalent to errno. Otherwise, use strerror_r().
  */
#ఒకవేళ SQLITE_THREADSAFE && defined(HAVE_STRERROR_R)
  అక్షరం aErr[80];
  memset(aErr, 0, పరిమాణం(aErr));
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
#ఒకవేళ (defined(STRERROR_R_CHAR_P) || defined(__USE_GNU)) \
  && !defined(ANDROID) && !defined(__ANDROID__)
  zErr =
# endif
  strerror_r(iErrno, aErr, పరిమాణం(aErr)-1);

#లేదావేళ SQLITE_THREADSAFE
  /* This is a threadsafe build, but strerror_r() is not available. */
  zErr = "";
#లేకపోతే
  /* Non-threadsafe build, use strerror(). */
  zErr = strerror(iErrno);
#అయితే_ముగింపు

  ఒకవేళ( z_మార్గం==0 ) z_మార్గం = "";
  sqlite3_log(errcode,
      "os_unix.c:%d: (%d) %s(%s) - %s",
      i_పంక్తి, iErrno, zFunc, z_మార్గం, zErr
  );

  తిరిగి errcode;
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
స్థిర శూన్య robust_close(unixFile *pFile, పూర్ణాంకం h, పూర్ణాంకం lineno){
  ఒకవేళ( osClose(h) ){
    unix_Log_Error_At_పంక్తి(SQLITE_IOERR_CLOSE, "close",
                       pFile ? pFile->z_మార్గం : 0, lineno);
  }
}

/*
** Set the pFile->lastErrno.  Do this in a subroutine as that provides
** a convenient place to set a breakpoint.
*/
స్థిర శూన్య store_చివరి_Errno(unixFile *pFile, పూర్ణాంకం error){
  pFile->చివరి_Errno = error;
}

/*
** Close all file descriptors accumulated in the unixInodeInfo->pUnused list.
*/
స్థిర శూన్య closePendingFds(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  Unix_వాడని_Fd *p;
  Unix_వాడని_Fd *p_తదుపరి;
  ప్రతిజ్ఞా( unixFileMutexHeld(pFile) );
  కొరకు(p=pInode->p_వాడని; p; p=p_తదుపరి){
    p_తదుపరి = p->p_తదుపరి;
    robust_close(pFile, p->fd, __LINE__);
    sqlite3_free(p);
  }
  pInode->p_వాడని = 0;
}

/*
** Release a unixInodeInfo structure previously allocated by findInodeInfo().
**
** The global mutex must be held when this routine is called, but the mutex
** on the inode being deleted must NOT be held.
*/
స్థిర శూన్య releaseInodeInfo(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  ప్రతిజ్ఞా( unixMutexHeld() );
  ప్రతిజ్ఞా( unixFileMutexNotheld(pFile) );
  ఒకవేళ( ALWAYS(pInode) ){
    pInode->nRef--;
    ఒకవేళ( pInode->nRef==0 ){
      ప్రతిజ్ఞా( pInode->p_Shm_కణుపు==0 );
      sqlite3_mutex_enter(pInode->p_తాళం_Mutex);
      closePendingFds(pFile);
      sqlite3_mutex_leave(pInode->p_తాళం_Mutex);
      ఒకవేళ( pInode->pPrev ){
        ప్రతిజ్ఞా( pInode->pPrev->p_తదుపరి==pInode );
        pInode->pPrev->p_తదుపరి = pInode->p_తదుపరి;
      }లేకపోతే{
        ప్రతిజ్ఞా( inodeList==pInode );
        inodeList = pInode->p_తదుపరి;
      }
      ఒకవేళ( pInode->p_తదుపరి ){
        ప్రతిజ్ఞా( pInode->p_తదుపరి->pPrev==pInode );
        pInode->p_తదుపరి->pPrev = pInode->pPrev;
      }
      sqlite3_mutex_free(pInode->p_తాళం_Mutex);
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
స్థిర పూర్ణాంకం కనుగొను_Inode_Info(
  unixFile *pFile,               /* Unix file with file desc used in the key */
  unixInodeInfo **ppInode        /* Return the unixInodeInfo object here */
){
  పూర్ణాంకం rc;                        /* System call return code */
  పూర్ణాంకం fd;                        /* The file descriptor for pFile */
  నిర్మాణం unixFileId fileId;      /* Lookup key for the unixInodeInfo */
  నిర్మాణం stat statbuf;           /* Low-level file information */
  unixInodeInfo *pInode = 0;     /* Candidate unixInodeInfo object */

  ప్రతిజ్ఞా( unixMutexHeld() );

  /* Get low-level information about the file that we can used to
  ** create a unique name for the file.
  */
  fd = pFile->h;
  rc = osFstat(fd, &statbuf);
  ఒకవేళ( rc!=0 ){
    store_చివరి_Errno(pFile, errno);
#ఒకవేళ defined(EOVERFLOW) && defined(SQLITE_DISABLE_LFS)
    ఒకవేళ( pFile->చివరి_Errno==EOVERFLOW ) తిరిగి SQLITE_NOLFS;
#అయితే_ముగింపు
    తిరిగి SQLITE_IOERR;
  }

#నిర్వచితమైతే __APPLE__
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
  ఒకవేళ( statbuf.st_కొలత==0 && (pFile->fsFlags & SQLITE_FSFLAGS_IS_MSDOS)!=0 ){
    చేయి{ rc = osWrite(fd, "S", 1); }అయితే( rc<0 && errno==EINTR );
    ఒకవేళ( rc!=1 ){
      store_చివరి_Errno(pFile, errno);
      తిరిగి SQLITE_IOERR;
    }
    ఒకవేళ( fsync(fd) ){
      store_చివరి_Errno(pFile, errno);
      తిరిగి SQLITE_IOERR_FSYNC;
    }
    rc = osFstat(fd, &statbuf);
    ఒకవేళ( rc!=0 ){
      store_చివరి_Errno(pFile, errno);
      తిరిగి SQLITE_IOERR;
    }
  }
#అయితే_ముగింపు

  memset(&fileId, 0, పరిమాణం(fileId));
  fileId.dev = statbuf.st_dev;
#ఒకవేళ OS_VXWORKS
  fileId.pId = pFile->pId;
#లేకపోతే
  fileId.ino = (u64)statbuf.st_ino;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( unixMutexHeld() );
  pInode = inodeList;
  అయితే( pInode && memcmp(&fileId, &pInode->fileId, పరిమాణం(fileId)) ){
    pInode = pInode->p_తదుపరి;
  }
  ఒకవేళ( pInode==0 ){
    pInode = sqlite3_malloc64( పరిమాణం(*pInode) );
    ఒకవేళ( pInode==0 ){
      తిరిగి SQLITE_NOMEM_BKPT;
    }
    memset(pInode, 0, పరిమాణం(*pInode));
    జ్ఞాపకం_నకలు(&pInode->fileId, &fileId, పరిమాణం(fileId));
    ఒకవేళ( sqlite3GlobalConfig.bCoreMutex ){
      pInode->p_తాళం_Mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      ఒకవేళ( pInode->p_తాళం_Mutex==0 ){
        sqlite3_free(pInode);
        తిరిగి SQLITE_NOMEM_BKPT;
      }
    }
    pInode->nRef = 1;
    ప్రతిజ్ఞా( unixMutexHeld() );
    pInode->p_తదుపరి = inodeList;
    pInode->pPrev = 0;
    ఒకవేళ( inodeList ) inodeList->pPrev = pInode;
    inodeList = pInode;
  }లేకపోతే{
    pInode->nRef++;
  }
  *ppInode = pInode;
  తిరిగి SQLITE_OK;
}

/*
** Return TRUE if pFile has been renamed or unlinked since it was first opened.
*/
స్థిర పూర్ణాంకం fileHasMoved(unixFile *pFile){
#ఒకవేళ OS_VXWORKS
  తిరిగి pFile->pInode!=0 && pFile->pId!=pFile->pInode->fileId.pId;
#లేకపోతే
  నిర్మాణం stat buf;
  తిరిగి pFile->pInode!=0 &&
      (osStat(pFile->z_మార్గం, &buf)!=0
         || (u64)buf.st_ino!=pFile->pInode->fileId.ino);
#అయితే_ముగింపు
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
స్థిర శూన్య verifyDbFile(unixFile *pFile){
  నిర్మాణం stat buf;
  పూర్ణాంకం rc;

  /* These verifications occurs for the main database only */
  ఒకవేళ( pFile->ctrlFlags & UNIXFILE_NOLOCK ) తిరిగి;

  rc = osFstat(pFile->h, &buf);
  ఒకవేళ( rc!=0 ){
    sqlite3_log(SQLITE_WARNING, "cannot fstat db file %s", pFile->z_మార్గం);
    తిరిగి;
  }
  ఒకవేళ( buf.st_nlink==0 ){
    sqlite3_log(SQLITE_WARNING, "file unlinked while open: %s", pFile->z_మార్గం);
    తిరిగి;
  }
  ఒకవేళ( buf.st_nlink>1 ){
    sqlite3_log(SQLITE_WARNING, "multiple links to file: %s", pFile->z_మార్గం);
    తిరిగి;
  }
  ఒకవేళ( fileHasMoved(pFile) ){
    sqlite3_log(SQLITE_WARNING, "file renamed while open: %s", pFile->z_మార్గం);
    తిరిగి;
  }
}


/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
స్థిర పూర్ణాంకం unix_Check_Reserved_తాళం(sqlite3_file *id, పూర్ణాంకం *p_Res_బహిర్గతం){
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం reserved = 0;
  unixFile *pFile = (unixFile*)id;

  SimulateIOError( తిరిగి SQLITE_IOERR_CHECKRESERVEDLOCK; );

  ప్రతిజ్ఞా( pFile );
  ప్రతిజ్ఞా( pFile->e_File_తాళం<=SHARED_తాళం );
  sqlite3_mutex_enter(pFile->pInode->p_తాళం_Mutex);

  /* Check if a thread in this process holds such a lock */
  ఒకవేళ( pFile->pInode->e_File_తాళం>SHARED_తాళం ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it.
  */
#నిర్వచితం_కాకపోతే __DJGPP__
  ఒకవేళ( !reserved && !pFile->pInode->b_Process_తాళం ){
    నిర్మాణం flock తాళం;
    తాళం.l_whence = SEEK_SET;
    తాళం.l_ప్రారంభం = RESERVED_BYTE;
    తాళం.l_len = 1;
    తాళం.l_type = F_WRLCK;
    ఒకవేళ( osFcntl(pFile->h, F_GETLK, &తాళం) ){
      rc = SQLITE_IOERR_CHECKRESERVEDLOCK;
      store_చివరి_Errno(pFile, errno);
    } లేకపోతే ఒకవేళ( తాళం.l_type!=F_UNLCK ){
      reserved = 1;
    }
  }
#అయితే_ముగింపు

  sqlite3_mutex_leave(pFile->pInode->p_తాళం_Mutex);
  OSTRACE(("TEST WR-LOCK %d %d %d (unix)\n", pFile->h, rc, reserved));

  *p_Res_బహిర్గతం = reserved;
  తిరిగి rc;
}

/* Forward declaration*/
స్థిర పూర్ణాంకం unixSleep(sqlite3_vfs*,పూర్ణాంకం);

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
#నిర్వచితం_కాకపోతే SQLITE_ENABLE_SETLK_TIMEOUT
# define os_సమితి_Posix_Advisory_తాళం(h,x,t) osFcntl(h,F_SETLK,x)
#లేకపోతే
స్థిర పూర్ణాంకం os_సమితి_Posix_Advisory_తాళం(
  పూర్ణాంకం h,                /* The file descriptor on which to take the lock */
  నిర్మాణం flock *p_తాళం,  /* The description of the lock */
  unixFile *pFile       /* Structure holding timeout value */
){
  పూర్ణాంకం rc = 0;

  ఒకవేళ( pFile->iBusyTimeout==0 ){
    /* unixFile->iBusyTimeout is set to 0. In this case, attempt a 
    ** non-blocking lock. */
    rc = osFcntl(h,F_SETLK,p_తాళం);
  }లేకపోతే{
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
#నిర్వచితమైతే SQLITE_TEST
    పూర్ణాంకం tm = pFile->iBusyTimeout;
    అయితే( tm>0 ){
      rc = osFcntl(h,F_SETLK,p_తాళం);
      ఒకవేళ( rc==0 ) ఆపు;
      unixSleep(0,1000);
      tm--;
    }
#లేకపోతే
    rc = osFcntl(h,F_SETLK,p_తాళం);
    pFile->iBusyTimeout = 0;
#అయితే_ముగింపు
    /* End of code to replace with real blocking-locks code. */
  }

  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_SETLK_TIMEOUT */


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
స్థిర పూర్ణాంకం unix_File_తాళం(unixFile *pFile, నిర్మాణం flock *p_తాళం){
  పూర్ణాంకం rc;
  unixInodeInfo *pInode = pFile->pInode;
  ప్రతిజ్ఞా( pInode!=0 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pInode->p_తాళం_Mutex) );
  ఒకవేళ( (pFile->ctrlFlags & (UNIXFILE_EXCL|UNIXFILE_RDONLY))==UNIXFILE_EXCL ){
    ఒకవేళ( pInode->b_Process_తాళం==0 ){
      నిర్మాణం flock తాళం;
      /* assert( pInode->nLock==0 ); <-- Not true if unix-excl READONLY used */
      తాళం.l_whence = SEEK_SET;
      తాళం.l_ప్రారంభం = SHARED_మొదటి;
      తాళం.l_len = SHARED_కొలత;
      తాళం.l_type = F_WRLCK;
      rc = os_సమితి_Posix_Advisory_తాళం(pFile->h, &తాళం, pFile);
      ఒకవేళ( rc<0 ) తిరిగి rc;
      pInode->b_Process_తాళం = 1;
      pInode->n_తాళం++;
    }లేకపోతే{
      rc = 0;
    }
  }లేకపోతే{
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
    ఒకవేళ( pFile->b_ఖండం_On_Connect && p_తాళం->l_type==F_RDLCK
     && p_తాళం->l_ప్రారంభం==SHARED_మొదటి && p_తాళం->l_len==SHARED_కొలత
    ){
      rc = osFcntl(pFile->h, F_SETLKW, p_తాళం);
    }లేకపోతే
#అయితే_ముగింపు
    rc = os_సమితి_Posix_Advisory_తాళం(pFile->h, p_తాళం, pFile);
  }
  తిరిగి rc;
}

#ఒకవేళ !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
/* Forward reference */
స్థిర పూర్ణాంకం unix_Is_Sharing_Shm_కణుపు(unixFile*);
#లేకపోతే
#నిర్వచించు unix_Is_Sharing_Shm_కణుపు(pFile) (0)
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం unix_తాళం(sqlite3_file *id, పూర్ణాంకం e_File_తాళం){
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
  పూర్ణాంకం rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  నిర్మాణం flock తాళం;
  పూర్ణాంకం tErrno = 0;

  ప్రతిజ్ఞా( pFile );
  OSTRACE(("LOCK    %d %s was %s(%s,%d) pid=%d (unix)\n", pFile->h,
      az_File_తాళం(e_File_తాళం), az_File_తాళం(pFile->e_File_తాళం),
      az_File_తాళం(pFile->pInode->e_File_తాళం), pFile->pInode->nShared,
      osGetpid(0)));

  /* If there is already a lock of this type or more restrictive on the
  ** unixFile, do nothing. Don't use the end_lock: exit path, as
  ** unixEnterMutex() hasn't been called yet.
  */
  ఒకవేళ( pFile->e_File_తాళం>=e_File_తాళం ){
    OSTRACE(("LOCK    %d %s ok (already held) (unix)\n", pFile->h,
            az_File_తాళం(e_File_తాళం)));
    తిరిగి SQLITE_OK;
  }

  /* Make sure the locking sequence is correct.
  **  (1) We never move from unlocked to anything higher than shared lock.
  **  (2) SQLite never explicitly requests a pending lock.
  **  (3) A shared lock is always held when a reserve lock is requested.
  */
  ప్రతిజ్ఞా( pFile->e_File_తాళం!=NO_తాళం || e_File_తాళం==SHARED_తాళం );
  ప్రతిజ్ఞా( e_File_తాళం!=PENDING_తాళం );
  ప్రతిజ్ఞా( e_File_తాళం!=RESERVED_తాళం || pFile->e_File_తాళం==SHARED_తాళం );

  /* This mutex is needed because pFile->pInode is shared across threads
  */
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->p_తాళం_Mutex);

  /* If some thread using this PID has a lock via a different unixFile*
  ** handle that precludes the requested lock, return BUSY.
  */
  ఒకవేళ( (pFile->e_File_తాళం!=pInode->e_File_తాళం &&
          (pInode->e_File_తాళం>=PENDING_తాళం || e_File_తాళం>SHARED_తాళం))
  ){
    rc = SQLITE_BUSY;
    వెళ్ళు చివర_తాళం;
  }

  /* If a SHARED lock is requested, and some thread using this PID already
  ** has a SHARED or RESERVED lock, then increment reference counts and
  ** return SQLITE_OK.
  */
  ఒకవేళ( e_File_తాళం==SHARED_తాళం &&
      (pInode->e_File_తాళం==SHARED_తాళం || pInode->e_File_తాళం==RESERVED_తాళం) ){
    ప్రతిజ్ఞా( e_File_తాళం==SHARED_తాళం );
    ప్రతిజ్ఞా( pFile->e_File_తాళం==0 );
    ప్రతిజ్ఞా( pInode->nShared>0 );
    pFile->e_File_తాళం = SHARED_తాళం;
    pInode->nShared++;
    pInode->n_తాళం++;
    వెళ్ళు చివర_తాళం;
  }


  /* A PENDING lock is needed before acquiring a SHARED lock and before
  ** acquiring an EXCLUSIVE lock.  For the SHARED lock, the PENDING will
  ** be released.
  */
  తాళం.l_len = 1L;
  తాళం.l_whence = SEEK_SET;
  ఒకవేళ( e_File_తాళం==SHARED_తాళం
   || (e_File_తాళం==EXCLUSIVE_తాళం && pFile->e_File_తాళం==RESERVED_తాళం)
  ){
    తాళం.l_type = (e_File_తాళం==SHARED_తాళం?F_RDLCK:F_WRLCK);
    తాళం.l_ప్రారంభం = PENDING_BYTE;
    ఒకవేళ( unix_File_తాళం(pFile, &తాళం) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_తాళం);
      ఒకవేళ( rc!=SQLITE_BUSY ){
        store_చివరి_Errno(pFile, tErrno);
      }
      వెళ్ళు చివర_తాళం;
    }లేకపోతే ఒకవేళ( e_File_తాళం==EXCLUSIVE_తాళం ){
      pFile->e_File_తాళం = PENDING_తాళం;
      pInode->e_File_తాళం = PENDING_తాళం;
    }
  }


  /* If control gets to this point, then actually go ahead and make
  ** operating system calls for the specified lock.
  */
  ఒకవేళ( e_File_తాళం==SHARED_తాళం ){
    ప్రతిజ్ఞా( pInode->nShared==0 );
    ప్రతిజ్ఞా( pInode->e_File_తాళం==0 );
    ప్రతిజ్ఞా( rc==SQLITE_OK );

    /* Now get the read-lock */
    తాళం.l_ప్రారంభం = SHARED_మొదటి;
    తాళం.l_len = SHARED_కొలత;
    ఒకవేళ( unix_File_తాళం(pFile, &తాళం) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_తాళం);
    }

    /* Drop the temporary PENDING lock */
    తాళం.l_ప్రారంభం = PENDING_BYTE;
    తాళం.l_len = 1L;
    తాళం.l_type = F_UNLCK;
    ఒకవేళ( unix_File_తాళం(pFile, &తాళం) && rc==SQLITE_OK ){
      /* This could happen with a network mount */
      tErrno = errno;
      rc = SQLITE_IOERR_UNLOCK;
    }

    ఒకవేళ( rc ){
      ఒకవేళ( rc!=SQLITE_BUSY ){
        store_చివరి_Errno(pFile, tErrno);
      }
      వెళ్ళు చివర_తాళం;
    }లేకపోతే{
      pFile->e_File_తాళం = SHARED_తాళం;
      pInode->n_తాళం++;
      pInode->nShared = 1;
    }
  }లేకపోతే ఒకవేళ( e_File_తాళం==EXCLUSIVE_తాళం && pInode->nShared>1 ){
    /* We are trying for an exclusive lock but another thread in this
    ** same process is still holding a shared lock. */
    rc = SQLITE_BUSY;
  }లేకపోతే ఒకవేళ( unix_Is_Sharing_Shm_కణుపు(pFile) ){
    /* We are in WAL mode and attempting to delete the SHM and WAL
    ** files due to closing the connection or changing out of WAL mode,
    ** but another process still holds locks on the SHM file, thus
    ** indicating that database locks have been broken, perhaps due
    ** to a rogue close(open(dbFile)) or similar.
    */
    rc = SQLITE_BUSY;
  }లేకపోతే{
    /* The request was for a RESERVED or EXCLUSIVE lock.  It is
    ** assumed that there is a SHARED or greater lock on the file
    ** already.
    */
    ప్రతిజ్ఞా( 0!=pFile->e_File_తాళం );
    తాళం.l_type = F_WRLCK;

    ప్రతిజ్ఞా( e_File_తాళం==RESERVED_తాళం || e_File_తాళం==EXCLUSIVE_తాళం );
    ఒకవేళ( e_File_తాళం==RESERVED_తాళం ){
      తాళం.l_ప్రారంభం = RESERVED_BYTE;
      తాళం.l_len = 1L;
    }లేకపోతే{
      తాళం.l_ప్రారంభం = SHARED_మొదటి;
      తాళం.l_len = SHARED_కొలత;
    }

    ఒకవేళ( unix_File_తాళం(pFile, &తాళం) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_తాళం);
      ఒకవేళ( rc!=SQLITE_BUSY ){
        store_చివరి_Errno(pFile, tErrno);
      }
    }
  }


#నిర్వచితమైతే SQLITE_DEBUG
  /* Set up the transaction-counter change checking flags when
  ** transitioning from a SHARED to a RESERVED lock.  The change
  ** from SHARED to RESERVED marks the beginning of a normal
  ** write operation (not a hot journal rollback).
  */
  ఒకవేళ( rc==SQLITE_OK
   && pFile->e_File_తాళం<=SHARED_తాళం
   && e_File_తాళం==RESERVED_తాళం
  ){
    pFile->transCntrChng = 0;
    pFile->dbUpdate = 0;
    pFile->లోపల_సాధారణ_Write = 1;
  }
#అయితే_ముగింపు

  ఒకవేళ( rc==SQLITE_OK ){
    pFile->e_File_తాళం = e_File_తాళం;
    pInode->e_File_తాళం = e_File_తాళం;
  }

చివర_తాళం:
  sqlite3_mutex_leave(pInode->p_తాళం_Mutex);
  OSTRACE(("LOCK    %d %s %s (unix)\n", pFile->h, az_File_తాళం(e_File_తాళం),
      rc==SQLITE_OK ? "ok" : "failed"));
  తిరిగి rc;
}

/*
** Add the file descriptor used by file handle pFile to the corresponding
** pUnused list.
*/
స్థిర శూన్య సమితి_Pending_Fd(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  Unix_వాడని_Fd *p = pFile->p_Preallocated_వాడని;
  ప్రతిజ్ఞా( unixFileMutexHeld(pFile) );
  p->p_తదుపరి = pInode->p_వాడని;
  pInode->p_వాడని = p;
  pFile->h = -1;
  pFile->p_Preallocated_వాడని = 0;
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
స్థిర పూర్ణాంకం posixUnlock(sqlite3_file *id, పూర్ణాంకం e_File_తాళం, పూర్ణాంకం handleNFSUnlock){
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  నిర్మాణం flock తాళం;
  పూర్ణాంకం rc = SQLITE_OK;

  ప్రతిజ్ఞా( pFile );
  OSTRACE(("UNLOCK  %d %d was %d(%d,%d) pid=%d (unix)\n", pFile->h, e_File_తాళం,
      pFile->e_File_తాళం, pFile->pInode->e_File_తాళం, pFile->pInode->nShared,
      osGetpid(0)));

  ప్రతిజ్ఞా( e_File_తాళం<=SHARED_తాళం );
  ఒకవేళ( pFile->e_File_తాళం<=e_File_తాళం ){
    తిరిగి SQLITE_OK;
  }
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->p_తాళం_Mutex);
  ప్రతిజ్ఞా( pInode->nShared!=0 );
  ఒకవేళ( pFile->e_File_తాళం>SHARED_తాళం ){
    ప్రతిజ్ఞా( pInode->e_File_తాళం==pFile->e_File_తాళం );

#నిర్వచితమైతే SQLITE_DEBUG
    /* When reducing a lock such that other processes can start
    ** reading the database file again, make sure that the
    ** transaction counter was updated if any part of the database
    ** file changed.  If the transaction counter is not updated,
    ** other connections to the same file might not realize that
    ** the file has changed and hence might not know to flush their
    ** cache.  The use of a stale cache can lead to database corruption.
    */
    pFile->లోపల_సాధారణ_Write = 0;
#అయితే_ముగింపు

    /* downgrading to a shared lock on NFS involves clearing the write lock
    ** before establishing the readlock - to avoid a race condition we downgrade
    ** the lock in 2 blocks, so that part of the range will be covered by a
    ** write lock until the rest is covered by a read lock:
    **  1:   [WWWWW]
    **  2:   [....W]
    **  3:   [RRRRW]
    **  4:   [RRRR.]
    */
    ఒకవేళ( e_File_తాళం==SHARED_తాళం ){
#ఒకవేళ !defined(__APPLE__) || !SQLITE_ENABLE_LOCKING_STYLE
      (శూన్య)handleNFSUnlock;
      ప్రతిజ్ఞా( handleNFSUnlock==0 );
#అయితే_ముగింపు
#ఒకవేళ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
      ఒకవేళ( handleNFSUnlock ){
        పూర్ణాంకం tErrno;               /* Error code from system call errors */
        off_t div_కొలత = SHARED_కొలత - 1;
      
        తాళం.l_type = F_UNLCK;
        తాళం.l_whence = SEEK_SET;
        తాళం.l_ప్రారంభం = SHARED_మొదటి;
        తాళం.l_len = div_కొలత;
        ఒకవేళ( unix_File_తాళం(pFile, &తాళం)==(-1) ){
          tErrno = errno;
          rc = SQLITE_IOERR_UNLOCK;
          store_చివరి_Errno(pFile, tErrno);
          వెళ్ళు చివర_unlock;
        }
        తాళం.l_type = F_RDLCK;
        తాళం.l_whence = SEEK_SET;
        తాళం.l_ప్రారంభం = SHARED_మొదటి;
        తాళం.l_len = div_కొలత;
        ఒకవేళ( unix_File_తాళం(pFile, &తాళం)==(-1) ){
          tErrno = errno;
          rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_RDLOCK);
          ఒకవేళ( IS_తాళం_ERROR(rc) ){
            store_చివరి_Errno(pFile, tErrno);
          }
          వెళ్ళు చివర_unlock;
        }
        తాళం.l_type = F_UNLCK;
        తాళం.l_whence = SEEK_SET;
        తాళం.l_ప్రారంభం = SHARED_మొదటి+div_కొలత;
        తాళం.l_len = SHARED_కొలత-div_కొలత;
        ఒకవేళ( unix_File_తాళం(pFile, &తాళం)==(-1) ){
          tErrno = errno;
          rc = SQLITE_IOERR_UNLOCK;
          store_చివరి_Errno(pFile, tErrno);
          వెళ్ళు చివర_unlock;
        }
      }లేకపోతే
#అయితే_ముగింపు /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
      {
        తాళం.l_type = F_RDLCK;
        తాళం.l_whence = SEEK_SET;
        తాళం.l_ప్రారంభం = SHARED_మొదటి;
        తాళం.l_len = SHARED_కొలత;
        ఒకవేళ( unix_File_తాళం(pFile, &తాళం) ){
          /* In theory, the call to unixFileLock() cannot fail because another
          ** process is holding an incompatible lock. If it does, this
          ** indicates that the other process is not following the locking
          ** protocol. If this happens, return SQLITE_IOERR_RDLOCK. Returning
          ** SQLITE_BUSY would confuse the upper layer (in practice it causes
          ** an assert to fail). */
          rc = SQLITE_IOERR_RDLOCK;
          store_చివరి_Errno(pFile, errno);
          వెళ్ళు చివర_unlock;
        }
      }
    }
    తాళం.l_type = F_UNLCK;
    తాళం.l_whence = SEEK_SET;
    తాళం.l_ప్రారంభం = PENDING_BYTE;
    తాళం.l_len = 2L;  ప్రతిజ్ఞా( PENDING_BYTE+1==RESERVED_BYTE );
    ఒకవేళ( unix_File_తాళం(pFile, &తాళం)==0 ){
      pInode->e_File_తాళం = SHARED_తాళం;
    }లేకపోతే{
      rc = SQLITE_IOERR_UNLOCK;
      store_చివరి_Errno(pFile, errno);
      వెళ్ళు చివర_unlock;
    }
  }
  ఒకవేళ( e_File_తాళం==NO_తాళం ){
    /* Decrement the shared lock counter.  Release the lock using an
    ** OS call only when all threads in this same process have released
    ** the lock.
    */
    pInode->nShared--;
    ఒకవేళ( pInode->nShared==0 ){
      తాళం.l_type = F_UNLCK;
      తాళం.l_whence = SEEK_SET;
      తాళం.l_ప్రారంభం = తాళం.l_len = 0L;
      ఒకవేళ( unix_File_తాళం(pFile, &తాళం)==0 ){
        pInode->e_File_తాళం = NO_తాళం;
      }లేకపోతే{
        rc = SQLITE_IOERR_UNLOCK;
        store_చివరి_Errno(pFile, errno);
        pInode->e_File_తాళం = NO_తాళం;
        pFile->e_File_తాళం = NO_తాళం;
      }
    }

    /* Decrement the count of locks against this same file.  When the
    ** count reaches zero, close any other file descriptors whose close
    ** was deferred because of outstanding locks.
    */
    pInode->n_తాళం--;
    ప్రతిజ్ఞా( pInode->n_తాళం>=0 );
    ఒకవేళ( pInode->n_తాళం==0 ) closePendingFds(pFile);
  }

చివర_unlock:
  sqlite3_mutex_leave(pInode->p_తాళం_Mutex);
  ఒకవేళ( rc==SQLITE_OK ){
    pFile->e_File_తాళం = e_File_తాళం;
  }
  తిరిగి rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
స్థిర పూర్ణాంకం unixUnlock(sqlite3_file *id, పూర్ణాంకం e_File_తాళం){
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  ప్రతిజ్ఞా( e_File_తాళం==SHARED_తాళం || ((unixFile *)id)->n_Fetch_బహిర్గతం==0 );
#అయితే_ముగింపు
  తిరిగి posixUnlock(id, e_File_తాళం, 0);
}

#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
స్థిర పూర్ణాంకం unixMapfile(unixFile *pFd, i64 nByte);
స్థిర శూన్య unixUnmapfile(unixFile *pFd);
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం closeUnixFile(sqlite3_file *id){
  unixFile *pFile = (unixFile*)id;
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  unixUnmapfile(pFile);
#అయితే_ముగింపు
  ఒకవేళ( pFile->h>=0 ){
    robust_close(pFile, pFile->h, __LINE__);
    pFile->h = -1;
  }
#ఒకవేళ OS_VXWORKS
  ఒకవేళ( pFile->pId ){
    ఒకవేళ( pFile->ctrlFlags & UNIXFILE_DELETE ){
      osUnlink(pFile->pId->zCanonicalName);
    }
    vxworksReleaseFileId(pFile->pId);
    pFile->pId = 0;
  }
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_UNLINK_AFTER_CLOSE
  ఒకవేళ( pFile->ctrlFlags & UNIXFILE_DELETE ){
    osUnlink(pFile->z_మార్గం);
    sqlite3_free(*(అక్షరం**)&pFile->z_మార్గం);
    pFile->z_మార్గం = 0;
  }
#అయితే_ముగింపు
  OSTRACE(("CLOSE   %-3d\n", pFile->h));
  OpenCounter(-1);
  sqlite3_free(pFile->p_Preallocated_వాడని);
  memset(pFile, 0, పరిమాణం(unixFile));
  తిరిగి SQLITE_OK;
}

/*
** Close a file.
*/
స్థిర పూర్ణాంకం unixClose(sqlite3_file *id){
  పూర్ణాంకం rc = SQLITE_OK;
  unixFile *pFile = (unixFile *)id;
  unixInodeInfo *pInode = pFile->pInode;

  ప్రతిజ్ఞా( pInode!=0 );
  verifyDbFile(pFile);
  unixUnlock(id, NO_తాళం);
  ప్రతిజ్ఞా( unixFileMutexNotheld(pFile) );
  unixEnterMutex();

  /* unixFile.pInode is always valid here. Otherwise, a different close
  ** routine (e.g. nolockClose()) would be called instead.
  */
  ప్రతిజ్ఞా( pFile->pInode->n_తాళం>0 || pFile->pInode->b_Process_తాళం==0 );
  sqlite3_mutex_enter(pInode->p_తాళం_Mutex);
  ఒకవేళ( pInode->n_తాళం ){
    /* If there are outstanding locks, do not actually close the file just
    ** yet because that would clear those locks.  Instead, add the file
    ** descriptor to pInode->pUnused list.  It will be automatically closed
    ** when the last lock is cleared.
    */
    సమితి_Pending_Fd(pFile);
  }
  sqlite3_mutex_leave(pInode->p_తాళం_Mutex);
  releaseInodeInfo(pFile);
  ప్రతిజ్ఞా( pFile->pShm==0 );
  rc = closeUnixFile(id);
  unixLeaveMutex();
  తిరిగి rc;
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

స్థిర పూర్ణాంకం nolock_Check_Reserved_తాళం(sqlite3_file *Not_వాడిన, పూర్ణాంకం *p_Res_బహిర్గతం){
  వాడని_PARAMETER(Not_వాడిన);
  *p_Res_బహిర్గతం = 0;
  తిరిగి SQLITE_OK;
}
స్థిర పూర్ణాంకం nolock_తాళం(sqlite3_file *Not_వాడిన, పూర్ణాంకం NotUsed2){
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
  తిరిగి SQLITE_OK;
}
స్థిర పూర్ణాంకం nolockUnlock(sqlite3_file *Not_వాడిన, పూర్ణాంకం NotUsed2){
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
  తిరిగి SQLITE_OK;
}

/*
** Close the file.
*/
స్థిర పూర్ణాంకం nolockClose(sqlite3_file *id) {
  తిరిగి closeUnixFile(id);
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
#నిర్వచించు DOTLOCK_SUFFIX ".lock"

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If the caller holds a SHARED
** or greater lock when it is called, then it is assumed that no other
** client may hold RESERVED. Or, if the caller holds no lock, then it
** is assumed another client holds RESERVED if the lock-file exists.
*/
స్థిర పూర్ణాంకం dotlock_Check_Reserved_తాళం(sqlite3_file *id, పూర్ణాంకం *p_Res_బహిర్గతం) {
  unixFile *pFile = (unixFile*)id;
  SimulateIOError( తిరిగి SQLITE_IOERR_CHECKRESERVEDLOCK; );

  ఒకవేళ( pFile->e_File_తాళం>=SHARED_తాళం ){
    *p_Res_బహిర్గతం = 0;
  }లేకపోతే{
    *p_Res_బహిర్గతం = osAccess((స్థిరరాశి అక్షరం*)pFile->lockingContext, 0)==0;
  }
  OSTRACE(("TEST WR-LOCK %d %d %d (dotlock)\n", pFile->h, 0, *p_Res_బహిర్గతం));
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం dotlock_తాళం(sqlite3_file *id, పూర్ణాంకం e_File_తాళం) {
  unixFile *pFile = (unixFile*)id;
  అక్షరం *z_తాళం_File = (అక్షరం *)pFile->lockingContext;
  పూర్ణాంకం rc = SQLITE_OK;


  /* If we have any lock, then the lock file already exists.  All we have
  ** to do is adjust our internal record of the lock level.
  */
  ఒకవేళ( pFile->e_File_తాళం > NO_తాళం ){
    pFile->e_File_తాళం = e_File_తాళం;
    /* Always update the timestamp on the old file */
#నిర్వచితమైతే HAVE_UTIME
    utime(z_తాళం_File, ఖాళీ);
#లేకపోతే
    utimes(z_తాళం_File, ఖాళీ);
#అయితే_ముగింపు
    తిరిగి SQLITE_OK;
  }

  /* grab an exclusive lock */
  rc = osMkdir(z_తాళం_File, 0777);
  ఒకవేళ( rc<0 ){
    /* failed to open/create the lock directory */
    పూర్ణాంకం tErrno = errno;
    ఒకవేళ( EEXIST == tErrno ){
      rc = SQLITE_BUSY;
    } లేకపోతే {
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_తాళం);
      ఒకవేళ( rc!=SQLITE_BUSY ){
        store_చివరి_Errno(pFile, tErrno);
      }
    }
    తిరిగి rc;
  }

  /* got it, set the type and return ok */
  pFile->e_File_తాళం = e_File_తాళం;
  తిరిగి rc;
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
స్థిర పూర్ణాంకం dotlockUnlock(sqlite3_file *id, పూర్ణాంకం e_File_తాళం) {
  unixFile *pFile = (unixFile*)id;
  అక్షరం *z_తాళం_File = (అక్షరం *)pFile->lockingContext;
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( pFile );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (dotlock)\n", pFile->h, e_File_తాళం,
           pFile->e_File_తాళం, osGetpid(0)));
  ప్రతిజ్ఞా( e_File_తాళం<=SHARED_తాళం );

  /* no-op if possible */
  ఒకవేళ( pFile->e_File_తాళం==e_File_తాళం ){
    తిరిగి SQLITE_OK;
  }

  /* To downgrade to shared, simply update our internal notion of the
  ** lock state.  No need to mess with the file on disk.
  */
  ఒకవేళ( e_File_తాళం==SHARED_తాళం ){
    pFile->e_File_తాళం = SHARED_తాళం;
    తిరిగి SQLITE_OK;
  }

  /* To fully unlock the database, delete the lock file */
  ప్రతిజ్ఞా( e_File_తాళం==NO_తాళం );
  rc = osRmdir(z_తాళం_File);
  ఒకవేళ( rc<0 ){
    పూర్ణాంకం tErrno = errno;
    ఒకవేళ( tErrno==ENOENT ){
      rc = SQLITE_OK;
    }లేకపోతే{
      rc = SQLITE_IOERR_UNLOCK;
      store_చివరి_Errno(pFile, tErrno);
    }
    తిరిగి rc;
  }
  pFile->e_File_తాళం = NO_తాళం;
  తిరిగి SQLITE_OK;
}

/*
** Close a file.  Make sure the lock has been released before closing.
*/
స్థిర పూర్ణాంకం dotlockClose(sqlite3_file *id) {
  unixFile *pFile = (unixFile*)id;
  ప్రతిజ్ఞా( id!=0 );
  dotlockUnlock(id, NO_తాళం);
  sqlite3_free(pFile->lockingContext);
  తిరిగి closeUnixFile(id);
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
#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE

/*
** Retry flock() calls that fail with EINTR
*/
#నిర్వచితమైతే EINTR
స్థిర పూర్ణాంకం robust_flock(పూర్ణాంకం fd, పూర్ణాంకం op){
  పూర్ణాంకం rc;
  చేయి{ rc = flock(fd,op); }అయితే( rc<0 && errno==EINTR );
  తిరిగి rc;
}
#లేకపోతే
# define robust_flock(a,b) flock(a,b)
#అయితే_ముగింపు
   

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
స్థిర పూర్ణాంకం flock_Check_Reserved_తాళం(sqlite3_file *id, పూర్ణాంకం *p_Res_బహిర్గతం){
#నిర్వచితమైతే SQLITE_DEBUG
  unixFile *pFile = (unixFile*)id;
#లేకపోతే
  వాడని_PARAMETER(id);
#అయితే_ముగింపు

  SimulateIOError( తిరిగి SQLITE_IOERR_CHECKRESERVEDLOCK; );

  ప్రతిజ్ఞా( pFile );
  ప్రతిజ్ఞా( pFile->e_File_తాళం<=SHARED_తాళం );

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
  *p_Res_బహిర్గతం = 0;

  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం flock_తాళం(sqlite3_file *id, పూర్ణాంకం e_File_తాళం) {
  పూర్ణాంకం rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;

  ప్రతిజ్ఞా( pFile );

  /* if we already have a lock, it is exclusive.
  ** Just adjust level and punt on outta here. */
  ఒకవేళ (pFile->e_File_తాళం > NO_తాళం) {
    pFile->e_File_తాళం = e_File_తాళం;
    తిరిగి SQLITE_OK;
  }

  /* grab an exclusive lock */

  ఒకవేళ (robust_flock(pFile->h, LOCK_EX | LOCK_NB)) {
    పూర్ణాంకం tErrno = errno;
    /* didn't get, must be busy */
    rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_తాళం);
    ఒకవేళ( IS_తాళం_ERROR(rc) ){
      store_చివరి_Errno(pFile, tErrno);
    }
  } లేకపోతే {
    /* got it, set the type and return ok */
    pFile->e_File_తాళం = e_File_తాళం;
  }
  OSTRACE(("LOCK    %d %s %s (flock)\n", pFile->h, az_File_తాళం(e_File_తాళం),
           rc==SQLITE_OK ? "ok" : "failed"));
#నిర్వచితమైతే SQLITE_IGNORE_FLOCK_తాళం_ERRORS
  ఒకవేళ( (rc & 0xff) == SQLITE_IOERR ){
    rc = SQLITE_BUSY;
  }
#అయితే_ముగింపు /* SQLITE_IGNORE_FLOCK_LOCK_ERRORS */
  తిరిగి rc;
}


/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
స్థిర పూర్ణాంకం flockUnlock(sqlite3_file *id, పూర్ణాంకం e_File_తాళం) {
  unixFile *pFile = (unixFile*)id;

  ప్రతిజ్ఞా( pFile );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (flock)\n", pFile->h, e_File_తాళం,
           pFile->e_File_తాళం, osGetpid(0)));
  ప్రతిజ్ఞా( e_File_తాళం<=SHARED_తాళం );

  /* no-op if possible */
  ఒకవేళ( pFile->e_File_తాళం==e_File_తాళం ){
    తిరిగి SQLITE_OK;
  }

  /* shared can just be set because we always have an exclusive */
  ఒకవేళ (e_File_తాళం==SHARED_తాళం) {
    pFile->e_File_తాళం = e_File_తాళం;
    తిరిగి SQLITE_OK;
  }

  /* no, really, unlock. */
  ఒకవేళ( robust_flock(pFile->h, LOCK_UN) ){
#నిర్వచితమైతే SQLITE_IGNORE_FLOCK_తాళం_ERRORS
    తిరిగి SQLITE_OK;
#అయితే_ముగింపు /* SQLITE_IGNORE_FLOCK_LOCK_ERRORS */
    తిరిగి SQLITE_IOERR_UNLOCK;
  }లేకపోతే{
    pFile->e_File_తాళం = NO_తాళం;
    తిరిగి SQLITE_OK;
  }
}

/*
** Close a file.
*/
స్థిర పూర్ణాంకం flockClose(sqlite3_file *id) {
  ప్రతిజ్ఞా( id!=0 );
  flockUnlock(id, NO_తాళం);
  తిరిగి closeUnixFile(id);
}

#అయితే_ముగింపు /* SQLITE_ENABLE_LOCKING_STYLE && !OS_VXWORK */

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
#ఒకవేళ OS_VXWORKS

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
స్థిర పూర్ణాంకం sem_XCheck_Reserved_తాళం(sqlite3_file *id, పూర్ణాంకం *p_Res_బహిర్గతం) {
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం reserved = 0;
  unixFile *pFile = (unixFile*)id;

  SimulateIOError( తిరిగి SQLITE_IOERR_CHECKRESERVEDLOCK; );

  ప్రతిజ్ఞా( pFile );

  /* Check if a thread in this process holds such a lock */
  ఒకవేళ( pFile->e_File_తాళం>SHARED_తాళం ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it. */
  ఒకవేళ( !reserved ){
    sem_t *pSem = pFile->pInode->pSem;

    ఒకవేళ( sem_trywait(pSem)==-1 ){
      పూర్ణాంకం tErrno = errno;
      ఒకవేళ( EAGAIN != tErrno ){
        rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_CHECKRESERVEDLOCK);
        store_చివరి_Errno(pFile, tErrno);
      } లేకపోతే {
        /* someone else has the lock when we are in NO_LOCK */
        reserved = (pFile->e_File_తాళం < SHARED_తాళం);
      }
    }లేకపోతే{
      /* we could have it if we want it */
      sem_post(pSem);
    }
  }
  OSTRACE(("TEST WR-LOCK %d %d %d (sem)\n", pFile->h, rc, reserved));

  *p_Res_బహిర్గతం = reserved;
  తిరిగి rc;
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
స్థిర పూర్ణాంకం semXLock(sqlite3_file *id, పూర్ణాంకం e_File_తాళం) {
  unixFile *pFile = (unixFile*)id;
  sem_t *pSem = pFile->pInode->pSem;
  పూర్ణాంకం rc = SQLITE_OK;

  /* if we already have a lock, it is exclusive.
  ** Just adjust level and punt on outta here. */
  ఒకవేళ (pFile->e_File_తాళం > NO_తాళం) {
    pFile->e_File_తాళం = e_File_తాళం;
    rc = SQLITE_OK;
    వెళ్ళు sem_చివర_తాళం;
  }

  /* lock semaphore now but bail out when already locked. */
  ఒకవేళ( sem_trywait(pSem)==-1 ){
    rc = SQLITE_BUSY;
    వెళ్ళు sem_చివర_తాళం;
  }

  /* got it, set the type and return ok */
  pFile->e_File_తాళం = e_File_తాళం;

 sem_చివర_తాళం:
  తిరిగి rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
స్థిర పూర్ణాంకం semXUnlock(sqlite3_file *id, పూర్ణాంకం e_File_తాళం) {
  unixFile *pFile = (unixFile*)id;
  sem_t *pSem = pFile->pInode->pSem;

  ప్రతిజ్ఞా( pFile );
  ప్రతిజ్ఞా( pSem );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (sem)\n", pFile->h, e_File_తాళం,
           pFile->e_File_తాళం, osGetpid(0)));
  ప్రతిజ్ఞా( e_File_తాళం<=SHARED_తాళం );

  /* no-op if possible */
  ఒకవేళ( pFile->e_File_తాళం==e_File_తాళం ){
    తిరిగి SQLITE_OK;
  }

  /* shared can just be set because we always have an exclusive */
  ఒకవేళ (e_File_తాళం==SHARED_తాళం) {
    pFile->e_File_తాళం = e_File_తాళం;
    తిరిగి SQLITE_OK;
  }

  /* no, really unlock. */
  ఒకవేళ ( sem_post(pSem)==-1 ) {
    పూర్ణాంకం rc, tErrno = errno;
    rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_UNLOCK);
    ఒకవేళ( IS_తాళం_ERROR(rc) ){
      store_చివరి_Errno(pFile, tErrno);
    }
    తిరిగి rc;
  }
  pFile->e_File_తాళం = NO_తాళం;
  తిరిగి SQLITE_OK;
}

/*
 ** Close a file.
 */
స్థిర పూర్ణాంకం semXClose(sqlite3_file *id) {
  ఒకవేళ( id ){
    unixFile *pFile = (unixFile*)id;
    semXUnlock(id, NO_తాళం);
    ప్రతిజ్ఞా( pFile );
    ప్రతిజ్ఞా( unixFileMutexNotheld(pFile) );
    unixEnterMutex();
    releaseInodeInfo(pFile);
    unixLeaveMutex();
    closeUnixFile(id);
  }
  తిరిగి SQLITE_OK;
}

#అయితే_ముగింపు /* OS_VXWORKS */
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

#ఒకవేళ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
** The afpLockingContext structure contains all afp lock specific state
*/
రకనిర్వచనం నిర్మాణం afpLockingContext afpLockingContext;
నిర్మాణం afpLockingContext {
  పూర్ణాంకం reserved;
  స్థిరరాశి అక్షరం *db_మార్గం;             /* Name of the open file */
};

నిర్మాణం Byte_Range_తాళం_PB2
{
  గుర్తులేని దీర్ఘ దీర్ఘ స్థానభ్రంశం;        /* offset to first byte to lock */
  గుర్తులేని దీర్ఘ దీర్ఘ నిడివి;        /* nbr of bytes to lock */
  గుర్తులేని దీర్ఘ దీర్ఘ ret_Range_ప్రారంభం; /* nbr of 1st byte locked if successful */
  గుర్తులేని అక్షరం un_తాళం_Flag;         /* 1 = unlock, 0 = lock */
  గుర్తులేని అక్షరం ప్రారంభం_చివర_Flag;       /* 1=rel to end of fork, 0=rel to start */
  పూర్ణాంకం fd;                           /* file desc to assoc this lock with */
};

#నిర్వచించు afpfsByteRangeLock2FSCTL        _IOWR('z', 23, నిర్మాణం Byte_Range_తాళం_PB2)

/*
** This is a utility for setting or clearing a bit-range lock on an
** AFP filesystem.
**
** Return SQLITE_OK on success, SQLITE_BUSY on failure.
*/
స్థిర పూర్ణాంకం afp_సమితి_తాళం(
  స్థిరరాశి అక్షరం *మార్గం,              /* Name of the file to be locked or unlocked */
  unixFile *pFile,               /* Open file descriptor on path */
  గుర్తులేని దీర్ఘ దీర్ఘ స్థానభ్రంశం,     /* First byte to be locked */
  గుర్తులేని దీర్ఘ దీర్ఘ నిడివి,     /* Number of bytes to lock */
  పూర్ణాంకం సమితి_తాళం_Flag                /* True to set lock.  False to clear lock */
){
  నిర్మాణం Byte_Range_తాళం_PB2 pb;
  పూర్ణాంకం err;

  pb.un_తాళం_Flag = సమితి_తాళం_Flag ? 0 : 1;
  pb.ప్రారంభం_చివర_Flag = 0;
  pb.స్థానభ్రంశం = స్థానభ్రంశం;
  pb.నిడివి = నిడివి;
  pb.fd = pFile->h;

  OSTRACE(("AFPSETLOCK [%s] for %d%s in range %llx:%llx\n",
    (సమితి_తాళం_Flag?"ON":"OFF"), pFile->h, (pb.fd==-1?"[testval-1]":""),
    స్థానభ్రంశం, నిడివి));
  err = fsctl(మార్గం, afpfsByteRangeLock2FSCTL, &pb, 0);
  ఒకవేళ ( err==-1 ) {
    పూర్ణాంకం rc;
    పూర్ణాంకం tErrno = errno;
    OSTRACE(("AFPSETLOCK failed to fsctl() '%s' %d %s\n",
             మార్గం, tErrno, strerror(tErrno)));
#నిర్వచితమైతే SQLITE_IGNORE_AFP_తాళం_ERRORS
    rc = SQLITE_BUSY;
#లేకపోతే
    rc = sqliteErrorFromPosixError(tErrno,
                    సమితి_తాళం_Flag ? SQLITE_IOERR_తాళం : SQLITE_IOERR_UNLOCK);
#అయితే_ముగింపు /* SQLITE_IGNORE_AFP_LOCK_ERRORS */
    ఒకవేళ( IS_తాళం_ERROR(rc) ){
      store_చివరి_Errno(pFile, tErrno);
    }
    తిరిగి rc;
  } లేకపోతే {
    తిరిగి SQLITE_OK;
  }
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
స్థిర పూర్ణాంకం afp_Check_Reserved_తాళం(sqlite3_file *id, పూర్ణాంకం *p_Res_బహిర్గతం){
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం reserved = 0;
  unixFile *pFile = (unixFile*)id;
  afpLockingContext *context;

  SimulateIOError( తిరిగి SQLITE_IOERR_CHECKRESERVEDLOCK; );

  ప్రతిజ్ఞా( pFile );
  context = (afpLockingContext *) pFile->lockingContext;
  ఒకవేళ( context->reserved ){
    *p_Res_బహిర్గతం = 1;
    తిరిగి SQLITE_OK;
  }
  sqlite3_mutex_enter(pFile->pInode->p_తాళం_Mutex);
  /* Check if a thread in this process holds such a lock */
  ఒకవేళ( pFile->pInode->e_File_తాళం>SHARED_తాళం ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it.
   */
  ఒకవేళ( !reserved ){
    /* lock the RESERVED byte */
    పూర్ణాంకం lrc = afp_సమితి_తాళం(context->db_మార్గం, pFile, RESERVED_BYTE, 1,1);
    ఒకవేళ( SQLITE_OK==lrc ){
      /* if we succeeded in taking the reserved lock, unlock it to restore
      ** the original state */
      lrc = afp_సమితి_తాళం(context->db_మార్గం, pFile, RESERVED_BYTE, 1, 0);
    } లేకపోతే {
      /* if we failed to get the lock then someone else must have it */
      reserved = 1;
    }
    ఒకవేళ( IS_తాళం_ERROR(lrc) ){
      rc=lrc;
    }
  }

  sqlite3_mutex_leave(pFile->pInode->p_తాళం_Mutex);
  OSTRACE(("TEST WR-LOCK %d %d %d (afp)\n", pFile->h, rc, reserved));

  *p_Res_బహిర్గతం = reserved;
  తిరిగి rc;
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
స్థిర పూర్ణాంకం afp_తాళం(sqlite3_file *id, పూర్ణాంకం e_File_తాళం){
  పూర్ణాంకం rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode = pFile->pInode;
  afpLockingContext *context = (afpLockingContext *) pFile->lockingContext;

  ప్రతిజ్ఞా( pFile );
  OSTRACE(("LOCK    %d %s was %s(%s,%d) pid=%d (afp)\n", pFile->h,
           az_File_తాళం(e_File_తాళం), az_File_తాళం(pFile->e_File_తాళం),
           az_File_తాళం(pInode->e_File_తాళం), pInode->nShared , osGetpid(0)));

  /* If there is already a lock of this type or more restrictive on the
  ** unixFile, do nothing. Don't use the afp_end_lock: exit path, as
  ** unixEnterMutex() hasn't been called yet.
  */
  ఒకవేళ( pFile->e_File_తాళం>=e_File_తాళం ){
    OSTRACE(("LOCK    %d %s ok (already held) (afp)\n", pFile->h,
           az_File_తాళం(e_File_తాళం)));
    తిరిగి SQLITE_OK;
  }

  /* Make sure the locking sequence is correct
  **  (1) We never move from unlocked to anything higher than shared lock.
  **  (2) SQLite never explicitly requests a pending lock.
  **  (3) A shared lock is always held when a reserve lock is requested.
  */
  ప్రతిజ్ఞా( pFile->e_File_తాళం!=NO_తాళం || e_File_తాళం==SHARED_తాళం );
  ప్రతిజ్ఞా( e_File_తాళం!=PENDING_తాళం );
  ప్రతిజ్ఞా( e_File_తాళం!=RESERVED_తాళం || pFile->e_File_తాళం==SHARED_తాళం );

  /* This mutex is needed because pFile->pInode is shared across threads
  */
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->p_తాళం_Mutex);

  /* If some thread using this PID has a lock via a different unixFile*
  ** handle that precludes the requested lock, return BUSY.
  */
  ఒకవేళ( (pFile->e_File_తాళం!=pInode->e_File_తాళం &&
       (pInode->e_File_తాళం>=PENDING_తాళం || e_File_తాళం>SHARED_తాళం))
     ){
    rc = SQLITE_BUSY;
    వెళ్ళు afp_చివర_తాళం;
  }

  /* If a SHARED lock is requested, and some thread using this PID already
  ** has a SHARED or RESERVED lock, then increment reference counts and
  ** return SQLITE_OK.
  */
  ఒకవేళ( e_File_తాళం==SHARED_తాళం &&
     (pInode->e_File_తాళం==SHARED_తాళం || pInode->e_File_తాళం==RESERVED_తాళం) ){
    ప్రతిజ్ఞా( e_File_తాళం==SHARED_తాళం );
    ప్రతిజ్ఞా( pFile->e_File_తాళం==0 );
    ప్రతిజ్ఞా( pInode->nShared>0 );
    pFile->e_File_తాళం = SHARED_తాళం;
    pInode->nShared++;
    pInode->n_తాళం++;
    వెళ్ళు afp_చివర_తాళం;
  }
  
  /* A PENDING lock is needed before acquiring a SHARED lock and before
  ** acquiring an EXCLUSIVE lock.  For the SHARED lock, the PENDING will
  ** be released.
  */
  ఒకవేళ( e_File_తాళం==SHARED_తాళం
      || (e_File_తాళం==EXCLUSIVE_తాళం && pFile->e_File_తాళం<PENDING_తాళం)
  ){
    పూర్ణాంకం failed;
    failed = afp_సమితి_తాళం(context->db_మార్గం, pFile, PENDING_BYTE, 1, 1);
    ఒకవేళ (failed) {
      rc = failed;
      వెళ్ళు afp_చివర_తాళం;
    }
  }

  /* If control gets to this point, then actually go ahead and make
  ** operating system calls for the specified lock.
  */
  ఒకవేళ( e_File_తాళం==SHARED_తాళం ){
    పూర్ణాంకం lrc1, lrc2, lrc1Errno = 0;
    దీర్ఘ lk, ముసుగు;
  
    ప్రతిజ్ఞా( pInode->nShared==0 );
    ప్రతిజ్ఞా( pInode->e_File_తాళం==0 );
      
    ముసుగు = (పరిమాణం(దీర్ఘ)==8) ? అతిపెద్ద_INT64 : 0x7fffffff;
    /* Now get the read-lock SHARED_LOCK */
    /* note that the quality of the randomness doesn't matter that much */
    lk = random();
    pInode->sharedByte = (lk & ముసుగు)%(SHARED_కొలత - 1);
    lrc1 = afp_సమితి_తాళం(context->db_మార్గం, pFile,
          SHARED_మొదటి+pInode->sharedByte, 1, 1);
    ఒకవేళ( IS_తాళం_ERROR(lrc1) ){
      lrc1Errno = pFile->చివరి_Errno;
    }
    /* Drop the temporary PENDING lock */
    lrc2 = afp_సమితి_తాళం(context->db_మార్గం, pFile, PENDING_BYTE, 1, 0);
  
    ఒకవేళ( IS_తాళం_ERROR(lrc1) ) {
      store_చివరి_Errno(pFile, lrc1Errno);
      rc = lrc1;
      వెళ్ళు afp_చివర_తాళం;
    } లేకపోతే ఒకవేళ( IS_తాళం_ERROR(lrc2) ){
      rc = lrc2;
      వెళ్ళు afp_చివర_తాళం;
    } లేకపోతే ఒకవేళ( lrc1 != SQLITE_OK ) {
      rc = lrc1;
    } లేకపోతే {
      pFile->e_File_తాళం = SHARED_తాళం;
      pInode->n_తాళం++;
      pInode->nShared = 1;
    }
  }లేకపోతే ఒకవేళ( e_File_తాళం==EXCLUSIVE_తాళం && pInode->nShared>1 ){
    /* We are trying for an exclusive lock but another thread in this
     ** same process is still holding a shared lock. */
    rc = SQLITE_BUSY;
  }లేకపోతే{
    /* The request was for a RESERVED or EXCLUSIVE lock.  It is
    ** assumed that there is a SHARED or greater lock on the file
    ** already.
    */
    పూర్ణాంకం failed = 0;
    ప్రతిజ్ఞా( 0!=pFile->e_File_తాళం );
    ఒకవేళ (e_File_తాళం >= RESERVED_తాళం && pFile->e_File_తాళం < RESERVED_తాళం) {
        /* Acquire a RESERVED lock */
        failed = afp_సమితి_తాళం(context->db_మార్గం, pFile, RESERVED_BYTE, 1,1);
      ఒకవేళ( !failed ){
        context->reserved = 1;
      }
    }
    ఒకవేళ (!failed && e_File_తాళం == EXCLUSIVE_తాళం) {
      /* Acquire an EXCLUSIVE lock */

      /* Remove the shared lock before trying the range.  we'll need to
      ** reestablish the shared lock if we can't get the  afpUnlock
      */
      ఒకవేళ( !(failed = afp_సమితి_తాళం(context->db_మార్గం, pFile, SHARED_మొదటి +
                         pInode->sharedByte, 1, 0)) ){
        పూర్ణాంకం failed2 = SQLITE_OK;
        /* now attempt to get the exclusive lock range */
        failed = afp_సమితి_తాళం(context->db_మార్గం, pFile, SHARED_మొదటి,
                               SHARED_కొలత, 1);
        ఒకవేళ( failed && (failed2 = afp_సమితి_తాళం(context->db_మార్గం, pFile,
                       SHARED_మొదటి + pInode->sharedByte, 1, 1)) ){
          /* Can't reestablish the shared lock.  Sqlite can't deal, this is
          ** a critical I/O error
          */
          rc = ((failed & 0xff) == SQLITE_IOERR) ? failed2 :
               SQLITE_IOERR_తాళం;
          వెళ్ళు afp_చివర_తాళం;
        }
      }లేకపోతే{
        rc = failed;
      }
    }
    ఒకవేళ( failed ){
      rc = failed;
    }
  }

  ఒకవేళ( rc==SQLITE_OK ){
    pFile->e_File_తాళం = e_File_తాళం;
    pInode->e_File_తాళం = e_File_తాళం;
  }లేకపోతే ఒకవేళ( e_File_తాళం==EXCLUSIVE_తాళం ){
    pFile->e_File_తాళం = PENDING_తాళం;
    pInode->e_File_తాళం = PENDING_తాళం;
  }

afp_చివర_తాళం:
  sqlite3_mutex_leave(pInode->p_తాళం_Mutex);
  OSTRACE(("LOCK    %d %s %s (afp)\n", pFile->h, az_File_తాళం(e_File_తాళం),
         rc==SQLITE_OK ? "ok" : "failed"));
  తిరిగి rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
స్థిర పూర్ణాంకం afpUnlock(sqlite3_file *id, పూర్ణాంకం e_File_తాళం) {
  పూర్ణాంకం rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  afpLockingContext *context = (afpLockingContext *) pFile->lockingContext;
  పూర్ణాంకం skipShared = 0;

  ప్రతిజ్ఞా( pFile );
  OSTRACE(("UNLOCK  %d %d was %d(%d,%d) pid=%d (afp)\n", pFile->h, e_File_తాళం,
           pFile->e_File_తాళం, pFile->pInode->e_File_తాళం, pFile->pInode->nShared,
           osGetpid(0)));

  ప్రతిజ్ఞా( e_File_తాళం<=SHARED_తాళం );
  ఒకవేళ( pFile->e_File_తాళం<=e_File_తాళం ){
    తిరిగి SQLITE_OK;
  }
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->p_తాళం_Mutex);
  ప్రతిజ్ఞా( pInode->nShared!=0 );
  ఒకవేళ( pFile->e_File_తాళం>SHARED_తాళం ){
    ప్రతిజ్ఞా( pInode->e_File_తాళం==pFile->e_File_తాళం );
  
#నిర్వచితమైతే SQLITE_DEBUG
    /* When reducing a lock such that other processes can start
    ** reading the database file again, make sure that the
    ** transaction counter was updated if any part of the database
    ** file changed.  If the transaction counter is not updated,
    ** other connections to the same file might not realize that
    ** the file has changed and hence might not know to flush their
    ** cache.  The use of a stale cache can lead to database corruption.
    */
    ప్రతిజ్ఞా( pFile->లోపల_సాధారణ_Write==0
           || pFile->dbUpdate==0
           || pFile->transCntrChng==1 );
    pFile->లోపల_సాధారణ_Write = 0;
#అయితే_ముగింపు
  
    ఒకవేళ( pFile->e_File_తాళం==EXCLUSIVE_తాళం ){
      rc = afp_సమితి_తాళం(context->db_మార్గం, pFile, SHARED_మొదటి, SHARED_కొలత, 0);
      ఒకవేళ( rc==SQLITE_OK && (e_File_తాళం==SHARED_తాళం || pInode->nShared>1) ){
        /* only re-establish the shared lock if necessary */
        పూర్ణాంకం shared_తాళం_Byte = SHARED_మొదటి+pInode->sharedByte;
        rc = afp_సమితి_తాళం(context->db_మార్గం, pFile, shared_తాళం_Byte, 1, 1);
      } లేకపోతే {
        skipShared = 1;
      }
    }
    ఒకవేళ( rc==SQLITE_OK && pFile->e_File_తాళం>=PENDING_తాళం ){
      rc = afp_సమితి_తాళం(context->db_మార్గం, pFile, PENDING_BYTE, 1, 0);
    }
    ఒకవేళ( rc==SQLITE_OK && pFile->e_File_తాళం>=RESERVED_తాళం && context->reserved ){
      rc = afp_సమితి_తాళం(context->db_మార్గం, pFile, RESERVED_BYTE, 1, 0);
      ఒకవేళ( !rc ){
        context->reserved = 0;
      }
    }
    ఒకవేళ( rc==SQLITE_OK && (e_File_తాళం==SHARED_తాళం || pInode->nShared>1)){
      pInode->e_File_తాళం = SHARED_తాళం;
    }
  }
  ఒకవేళ( rc==SQLITE_OK && e_File_తాళం==NO_తాళం ){

    /* Decrement the shared lock counter.  Release the lock using an
    ** OS call only when all threads in this same process have released
    ** the lock.
    */
    గుర్తులేని దీర్ఘ దీర్ఘ shared_తాళం_Byte = SHARED_మొదటి+pInode->sharedByte;
    pInode->nShared--;
    ఒకవేళ( pInode->nShared==0 ){
      ఒకవేళ( !skipShared ){
        rc = afp_సమితి_తాళం(context->db_మార్గం, pFile, shared_తాళం_Byte, 1, 0);
      }
      ఒకవేళ( !rc ){
        pInode->e_File_తాళం = NO_తాళం;
        pFile->e_File_తాళం = NO_తాళం;
      }
    }
    ఒకవేళ( rc==SQLITE_OK ){
      pInode->n_తాళం--;
      ప్రతిజ్ఞా( pInode->n_తాళం>=0 );
      ఒకవేళ( pInode->n_తాళం==0 ) closePendingFds(pFile);
    }
  }

  sqlite3_mutex_leave(pInode->p_తాళం_Mutex);
  ఒకవేళ( rc==SQLITE_OK ){
    pFile->e_File_తాళం = e_File_తాళం;
  }
  తిరిగి rc;
}

/*
** Close a file & cleanup AFP specific locking context
*/
స్థిర పూర్ణాంకం afpClose(sqlite3_file *id) {
  పూర్ణాంకం rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  ప్రతిజ్ఞా( id!=0 );
  afpUnlock(id, NO_తాళం);
  ప్రతిజ్ఞా( unixFileMutexNotheld(pFile) );
  unixEnterMutex();
  ఒకవేళ( pFile->pInode ){
    unixInodeInfo *pInode = pFile->pInode;
    sqlite3_mutex_enter(pInode->p_తాళం_Mutex);
    ఒకవేళ( pInode->n_తాళం ){
      /* If there are outstanding locks, do not actually close the file just
      ** yet because that would clear those locks.  Instead, add the file
      ** descriptor to pInode->aPending.  It will be automatically closed when
      ** the last lock is cleared.
      */
      సమితి_Pending_Fd(pFile);
    }
    sqlite3_mutex_leave(pInode->p_తాళం_Mutex);
  }
  releaseInodeInfo(pFile);
  sqlite3_free(pFile->lockingContext);
  rc = closeUnixFile(id);
  unixLeaveMutex();
  తిరిగి rc;
}

#అయితే_ముగింపు /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
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

#ఒకవేళ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
 ** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
 ** must be either NO_LOCK or SHARED_LOCK.
 **
 ** If the locking level of the file descriptor is already at or below
 ** the requested locking level, this routine is a no-op.
 */
స్థిర పూర్ణాంకం nfsUnlock(sqlite3_file *id, పూర్ణాంకం e_File_తాళం){
  తిరిగి posixUnlock(id, e_File_తాళం, 1);
}

#అయితే_ముగింపు /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
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
స్థిర పూర్ణాంకం seekAndRead(unixFile *id, sqlite3_int64 స్థానభ్రంశం, శూన్య *pBuf, పూర్ణాంకం cnt){
  పూర్ణాంకం got;
  పూర్ణాంకం పూర్వ = 0;
#ఒకవేళ (!defined(USE_PREAD) && !defined(USE_PREAD64))
  i64 new_స్థానభ్రంశం;
#అయితే_ముగింపు
  TIMER_ప్రారంభం;
  ప్రతిజ్ఞా( cnt==(cnt&0x1ffff) );
  ప్రతిజ్ఞా( id->h>2 );
  చేయి{
#ఒకవేళ defined(USE_PREAD)
    got = osPread(id->h, pBuf, cnt, స్థానభ్రంశం);
    SimulateIOError( got = -1 );
#లేదావేళ defined(USE_PREAD64)
    got = osPread64(id->h, pBuf, cnt, స్థానభ్రంశం);
    SimulateIOError( got = -1 );
#లేకపోతే
    new_స్థానభ్రంశం = lseek(id->h, స్థానభ్రంశం, SEEK_SET);
    SimulateIOError( new_స్థానభ్రంశం = -1 );
    ఒకవేళ( new_స్థానభ్రంశం<0 ){
      store_చివరి_Errno((unixFile*)id, errno);
      తిరిగి -1;
    }
    got = osRead(id->h, pBuf, cnt);
#అయితే_ముగింపు
    ఒకవేళ( got==cnt ) ఆపు;
    ఒకవేళ( got<0 ){
      ఒకవేళ( errno==EINTR ){ got = 1; కొనసాగు; }
      పూర్వ = 0;
      store_చివరి_Errno((unixFile*)id,  errno);
      ఆపు;
    }లేకపోతే ఒకవేళ( got>0 ){
      cnt -= got;
      స్థానభ్రంశం += got;
      పూర్వ += got;
      pBuf = (శూన్య*)(got + (అక్షరం*)pBuf);
    }
  }అయితే( got>0 );
  TIMER_చివర;
  OSTRACE(("READ    %-3d %5d %7lld %llu\n",
            id->h, got+పూర్వ, స్థానభ్రంశం-పూర్వ, TIMER_ELAPSED));
  తిరిగి got+పూర్వ;
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
స్థిర పూర్ణాంకం unixRead(
  sqlite3_file *id,
  శూన్య *pBuf,
  పూర్ణాంకం amt,
  sqlite3_int64 స్థానభ్రంశం
){
  unixFile *pFile = (unixFile *)id;
  పూర్ణాంకం got;
  ప్రతిజ్ఞా( id );
  ప్రతిజ్ఞా( స్థానభ్రంశం>=0 );
  ప్రతిజ్ఞా( amt>0 );

  /* If this is a database file (not a journal, super-journal or temp
  ** file), the bytes in the locking range should never be read or written. */
#ఒకవేళ 0
  ప్రతిజ్ఞా( pFile->p_Preallocated_వాడని==0
       || స్థానభ్రంశం>=PENDING_BYTE+512
       || స్థానభ్రంశం+amt<=PENDING_BYTE
  );
#అయితే_ముగింపు

#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  ఒకవేళ( స్థానభ్రంశం<pFile->mmap_కొలత ){
    ఒకవేళ( స్థానభ్రంశం+amt <= pFile->mmap_కొలత ){
      జ్ఞాపకం_నకలు(pBuf, &((u8 *)(pFile->pMapRegion))[స్థానభ్రంశం], amt);
      తిరిగి SQLITE_OK;
    }లేకపోతే{
      పూర్ణాంకం nCopy = pFile->mmap_కొలత - స్థానభ్రంశం;
      జ్ఞాపకం_నకలు(pBuf, &((u8 *)(pFile->pMapRegion))[స్థానభ్రంశం], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      స్థానభ్రంశం += nCopy;
    }
  }
#అయితే_ముగింపు

  got = seekAndRead(pFile, స్థానభ్రంశం, pBuf, amt);
  ఒకవేళ( got==amt ){
    తిరిగి SQLITE_OK;
  }లేకపోతే ఒకవేళ( got<0 ){
    /* pFile->lastErrno has been set by seekAndRead().
    ** Usually we return SQLITE_IOERR_READ here, though for some
    ** kinds of errors we return SQLITE_IOERR_CORRUPTFS.  The
    ** SQLITE_IOERR_CORRUPTFS will be converted into SQLITE_CORRUPT
    ** prior to returning to the application by the sqlite3ApiExit()
    ** routine.
    */
    ఎంపిక( pFile->చివరి_Errno ){
      స్థితి ERANGE:
      స్థితి EIO:
#నిర్వచితమైతే ENXIO
      స్థితి ENXIO:
#అయితే_ముగింపు
#నిర్వచితమైతే EDEVERR
      స్థితి EDEVERR:
#అయితే_ముగింపు
        తిరిగి SQLITE_IOERR_CORRUPTFS;
    }
    తిరిగి SQLITE_IOERR_READ;
  }లేకపోతే{
    store_చివరి_Errno(pFile, 0);   /* not a system error */
    /* Unread parts of the buffer must be zero-filled */
    memset(&((అక్షరం*)pBuf)[got], 0, amt-got);
    తిరిగి SQLITE_IOERR_SHORT_READ;
  }
}

/*
** Attempt to seek the file-descriptor passed as the first argument to
** absolute offset iOff, then attempt to write nBuf bytes of data from
** pBuf to it. If an error occurs, return -1 and set *piErrno. Otherwise,
** return the actual number of bytes written (which may be less than
** nBuf).
*/
స్థిర పూర్ణాంకం seekAndWriteFd(
  పూర్ణాంకం fd,                         /* File descriptor to write to */
  i64 iOff,                       /* File offset to begin writing at */
  స్థిరరాశి శూన్య *pBuf,               /* Copy data from this buffer to the file */
  పూర్ణాంకం nBuf,                       /* Size of buffer pBuf in bytes */
  పూర్ణాంకం *piErrno                    /* OUT: Error number if error occurs */
){
  పూర్ణాంకం rc = 0;                     /* Value returned by system call */

  ప్రతిజ్ఞా( nBuf==(nBuf&0x1ffff) );
  ప్రతిజ్ఞా( fd>2 );
  ప్రతిజ్ఞా( piErrno!=0 );
  nBuf &= 0x1ffff;
  TIMER_ప్రారంభం;

#ఒకవేళ defined(USE_PREAD)
  చేయి{ rc = (పూర్ణాంకం)osPwrite(fd, pBuf, nBuf, iOff); }అయితే( rc<0 && errno==EINTR );
#లేదావేళ defined(USE_PREAD64)
  చేయి{ rc = (పూర్ణాంకం)osPwrite64(fd, pBuf, nBuf, iOff);}అయితే( rc<0 && errno==EINTR);
#లేకపోతే
  చేయి{
    i64 iSeek = lseek(fd, iOff, SEEK_SET);
    SimulateIOError( iSeek = -1 );
    ఒకవేళ( iSeek<0 ){
      rc = -1;
      ఆపు;
    }
    rc = osWrite(fd, pBuf, nBuf);
  }అయితే( rc<0 && errno==EINTR );
#అయితే_ముగింపు

  TIMER_చివర;
  OSTRACE(("WRITE   %-3d %5d %7lld %llu\n", fd, rc, iOff, TIMER_ELAPSED));

  ఒకవేళ( rc<0 ) *piErrno = errno;
  తిరిగి rc;
}


/*
** Seek to the offset in id->offset then read cnt bytes into pBuf.
** Return the number of bytes actually read.  Update the offset.
**
** To avoid stomping the errno value on a failed write the lastErrno value
** is set before returning.
*/
స్థిర పూర్ణాంకం seekAndWrite(unixFile *id, i64 స్థానభ్రంశం, స్థిరరాశి శూన్య *pBuf, పూర్ణాంకం cnt){
  తిరిగి seekAndWriteFd(id->h, స్థానభ్రంశం, pBuf, cnt, &id->చివరి_Errno);
}


/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
స్థిర పూర్ణాంకం unixWrite(
  sqlite3_file *id,
  స్థిరరాశి శూన్య *pBuf,
  పూర్ణాంకం amt,
  sqlite3_int64 స్థానభ్రంశం
){
  unixFile *pFile = (unixFile*)id;
  పూర్ణాంకం wrote = 0;
  ప్రతిజ్ఞా( id );
  ప్రతిజ్ఞా( amt>0 );

  /* If this is a database file (not a journal, super-journal or temp
  ** file), the bytes in the locking range should never be read or written. */
#ఒకవేళ 0
  ప్రతిజ్ఞా( pFile->p_Preallocated_వాడని==0
       || స్థానభ్రంశం>=PENDING_BYTE+512
       || స్థానభ్రంశం+amt<=PENDING_BYTE
  );
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_DEBUG
  /* If we are doing a normal write to a database file (as opposed to
  ** doing a hot-journal rollback or a write to some file other than a
  ** normal database file) then record the fact that the database
  ** has changed.  If the transaction counter is modified, record that
  ** fact too.
  */
  ఒకవేళ( pFile->లోపల_సాధారణ_Write ){
    pFile->dbUpdate = 1;  /* The database has been modified */
    ఒకవేళ( స్థానభ్రంశం<=24 && స్థానభ్రంశం+amt>=27 ){
      పూర్ణాంకం rc;
      అక్షరం oldCntr[4];
      SimulateIOErrorBenign(1);
      rc = seekAndRead(pFile, 24, oldCntr, 4);
      SimulateIOErrorBenign(0);
      ఒకవేళ( rc!=4 || memcmp(oldCntr, &((అక్షరం*)pBuf)[24-స్థానభ్రంశం], 4)!=0 ){
        pFile->transCntrChng = 1;  /* The transaction counter has changed */
      }
    }
  }
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_కొలత>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  ఒకవేళ( స్థానభ్రంశం<pFile->mmap_కొలత ){
    ఒకవేళ( స్థానభ్రంశం+amt <= pFile->mmap_కొలత ){
      జ్ఞాపకం_నకలు(&((u8 *)(pFile->pMapRegion))[స్థానభ్రంశం], pBuf, amt);
      తిరిగి SQLITE_OK;
    }లేకపోతే{
      పూర్ణాంకం nCopy = pFile->mmap_కొలత - స్థానభ్రంశం;
      జ్ఞాపకం_నకలు(&((u8 *)(pFile->pMapRegion))[స్థానభ్రంశం], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      స్థానభ్రంశం += nCopy;
    }
  }
#అయితే_ముగింపు

  అయితే( (wrote = seekAndWrite(pFile, స్థానభ్రంశం, pBuf, amt))<amt && wrote>0 ){
    amt -= wrote;
    స్థానభ్రంశం += wrote;
    pBuf = &((అక్షరం*)pBuf)[wrote];
  }
  SimulateIOError(( wrote=(-1), amt=1 ));
  SimulateDiskfullError(( wrote=0, amt=1 ));

  ఒకవేళ( amt>wrote ){
    ఒకవేళ( wrote<0 && pFile->చివరి_Errno!=ENOSPC ){
      /* lastErrno set by seekAndWrite */
      తిరిగి SQLITE_IOERR_WRITE;
    }లేకపోతే{
      store_చివరి_Errno(pFile, 0); /* not a system error */
      తిరిగి SQLITE_నిండిన;
    }
  }

  తిరిగి SQLITE_OK;
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
** We do not trust systems to provide a working fdatasync().  Some do.
** Others do no.  To be safe, we will stick with the (slightly slower)
** fsync(). If you know that your system does support fdatasync() correctly,
** then simply compile with -Dfdatasync=fdatasync or -DHAVE_FDATASYNC
*/
#ఒకవేళ !defined(fdatasync) && !HAVE_FDATASYNC
# define fdatasync fsync
#అయితే_ముగింపు

/*
** Define HAVE_FULLFSYNC to 0 or 1 depending on whether or not
** the F_FULLFSYNC macro is defined.  F_FULLFSYNC is currently
** only available on Mac OS X.  But that could change.
*/
#నిర్వచితమైతే F_FULLFSYNC
# define HAVE_FULLFSYNC 1
#లేకపోతే
# define HAVE_FULLFSYNC 0
#అయితే_ముగింపు


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
స్థిర పూర్ణాంకం నిండిన_fsync(పూర్ణాంకం fd, పూర్ణాంకం నిండిన_Sync, పూర్ణాంకం దత్తాంశం_Only){
  పూర్ణాంకం rc;

  /* The following "ifdef/elif/else/" block has the same structure as
  ** the one below. It is replicated here solely to avoid cluttering
  ** up the real code with the UNUSED_PARAMETER() macros.
  */
#నిర్వచితమైతే SQLITE_NO_SYNC
  వాడని_PARAMETER(fd);
  వాడని_PARAMETER(నిండిన_Sync);
  వాడని_PARAMETER(దత్తాంశం_Only);
#లేదావేళ HAVE_FULLFSYNC
  వాడని_PARAMETER(దత్తాంశం_Only);
#లేకపోతే
  వాడని_PARAMETER(నిండిన_Sync);
  వాడని_PARAMETER(దత్తాంశం_Only);
#అయితే_ముగింపు

  /* Record the number of times that we do a normal fsync() and
  ** FULLSYNC.  This is used during testing to verify that this procedure
  ** gets called with the correct arguments.
  */
#నిర్వచితమైతే SQLITE_TEST
  ఒకవేళ( నిండిన_Sync ) sqlite3_fullsync_లెక్క++;
  sqlite3_sync_లెక్క++;
#అయితే_ముగింపు

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op.  But go ahead and call fstat() to validate the file
  ** descriptor as we need a method to provoke a failure during
  ** coverage testing.
  */
#నిర్వచితమైతే SQLITE_NO_SYNC
  {
    నిర్మాణం stat buf;
    rc = osFstat(fd, &buf);
  }
#లేదావేళ HAVE_FULLFSYNC
  ఒకవేళ( నిండిన_Sync ){
    rc = osFcntl(fd, F_FULLFSYNC, 0);
  }లేకపోతే{
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
  ఒకవేళ( rc ) rc = fsync(fd);

#లేదావేళ defined(__APPLE__)
  /* fdatasync() on HFS+ doesn't yet flush the file size if it changed correctly
  ** so currently we default to the macro that redefines fdatasync to fsync
  */
  rc = fsync(fd);
#లేకపోతే
  rc = fdatasync(fd);
#ఒకవేళ OS_VXWORKS
  ఒకవేళ( rc==-1 && errno==ENOTSUP ){
    rc = fsync(fd);
  }
#అయితే_ముగింపు /* OS_VXWORKS */
#అయితే_ముగింపు /* ifdef SQLITE_NO_SYNC elif HAVE_FULLFSYNC */

  ఒకవేళ( OS_VXWORKS && rc!= -1 ){
    rc = 0;
  }
  తిరిగి rc;
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
స్థిర పూర్ణాంకం openDirectory(స్థిరరాశి అక్షరం *zFilename, పూర్ణాంకం *pFd){
  పూర్ణాంకం ii;
  పూర్ణాంకం fd = -1;
  అక్షరం zDirname[MAX_PATHNAME+1];

  sqlite3_snprintf(MAX_PATHNAME, zDirname, "%s", zFilename);
  కొరకు(ii=(పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zDirname); ii>0 && zDirname[ii]!='/'; ii--);
  ఒకవేళ( ii>0 ){
    zDirname[ii] = '\0';
  }లేకపోతే{
    ఒకవేళ( zDirname[0]!='/' ) zDirname[0] = '.';
    zDirname[1] = 0;
  }
  fd = robust_open(zDirname, O_RDONLY|O_ద్విమాన, 0);
  ఒకవేళ( fd>=0 ){
    OSTRACE(("OPENDIR %-3d %s\n", fd, zDirname));
  }
  *pFd = fd;
  ఒకవేళ( fd>=0 ) తిరిగి SQLITE_OK;
  తిరిగి unixLogError(SQLITE_CANTOPEN_BKPT, "openDirectory", zDirname);
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
స్థిర పూర్ణాంకం unixSync(sqlite3_file *id, పూర్ణాంకం flags){
  పూర్ణాంకం rc;
  unixFile *pFile = (unixFile*)id;

  పూర్ణాంకం is_దత్తాంశం_Only = (flags&SQLITE_SYNC_DATAONLY);
  పూర్ణాంకం isFullsync = (flags&0x0F)==SQLITE_SYNC_నిండిన;

  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  ప్రతిజ్ఞా((flags&0x0F)==SQLITE_SYNC_సాధారణ
      || (flags&0x0F)==SQLITE_SYNC_నిండిన
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( తిరిగి SQLITE_నిండిన );

  ప్రతిజ్ఞా( pFile );
  OSTRACE(("SYNC    %-3d\n", pFile->h));
  rc = నిండిన_fsync(pFile->h, isFullsync, is_దత్తాంశం_Only);
  SimulateIOError( rc=1 );
  ఒకవేళ( rc ){
    store_చివరి_Errno(pFile, errno);
    తిరిగి unixLogError(SQLITE_IOERR_FSYNC, "full_fsync", pFile->z_మార్గం);
  }

  /* Also fsync the directory containing the file if the DIRSYNC flag
  ** is set.  This is a one-time occurrence.  Many systems (examples: AIX)
  ** are unable to fsync a directory, so ignore errors on the fsync.
  */
  ఒకవేళ( pFile->ctrlFlags & UNIXFILE_DIRSYNC ){
    పూర్ణాంకం dirfd;
    OSTRACE(("DIRSYNC %s (have_fullfsync=%d fullsync=%d)\n", pFile->z_మార్గం,
            HAVE_FULLFSYNC, isFullsync));
    rc = osOpenDirectory(pFile->z_మార్గం, &dirfd);
    ఒకవేళ( rc==SQLITE_OK ){
      నిండిన_fsync(dirfd, 0, 0);
      robust_close(pFile, dirfd, __LINE__);
    }లేకపోతే{
      ప్రతిజ్ఞా( rc==SQLITE_CANTOPEN );
      rc = SQLITE_OK;
    }
    pFile->ctrlFlags &= ~UNIXFILE_DIRSYNC;
  }
  తిరిగి rc;
}

/*
** Truncate an open file to a specified size
*/
స్థిర పూర్ణాంకం unixTruncate(sqlite3_file *id, i64 nByte){
  unixFile *pFile = (unixFile *)id;
  పూర్ణాంకం rc;
  ప్రతిజ్ఞా( pFile );
  SimulateIOError( తిరిగి SQLITE_IOERR_TRUNCATE );

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  ఒకవేళ( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

  rc = robust_ftruncate(pFile->h, nByte);
  ఒకవేళ( rc ){
    store_చివరి_Errno(pFile, errno);
    తిరిగి unixLogError(SQLITE_IOERR_TRUNCATE, "ftruncate", pFile->z_మార్గం);
  }లేకపోతే{
#నిర్వచితమైతే SQLITE_DEBUG
    /* If we are doing a normal write to a database file (as opposed to
    ** doing a hot-journal rollback or a write to some file other than a
    ** normal database file) and we truncate the file to zero length,
    ** that effectively updates the change counter.  This might happen
    ** when restoring a database using the backup API from a zero-length
    ** source.
    */
    ఒకవేళ( pFile->లోపల_సాధారణ_Write && nByte==0 ){
      pFile->transCntrChng = 1;
    }
#అయితే_ముగింపు

#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
    /* If the file was just truncated to a size smaller than the currently
    ** mapped region, reduce the effective mapping size as well. SQLite will
    ** use read() and write() to access data beyond this point from now on.
    */
    ఒకవేళ( nByte<pFile->mmap_కొలత ){
      pFile->mmap_కొలత = nByte;
    }
#అయితే_ముగింపు

    తిరిగి SQLITE_OK;
  }
}

/*
** Determine the current size of a file in bytes
*/
స్థిర పూర్ణాంకం unix_File_కొలత(sqlite3_file *id, i64 *p_కొలత){
  పూర్ణాంకం rc;
  నిర్మాణం stat buf;
  ప్రతిజ్ఞా( id );
  rc = osFstat(((unixFile*)id)->h, &buf);
  SimulateIOError( rc=1 );
  ఒకవేళ( rc!=0 ){
    store_చివరి_Errno((unixFile*)id, errno);
    తిరిగి SQLITE_IOERR_FSTAT;
  }
  *p_కొలత = buf.st_కొలత;

  /* When opening a zero-size database, the findInodeInfo() procedure
  ** writes a single byte into that file in order to work around a bug
  ** in the OS-X msdos filesystem.  In order to avoid problems with upper
  ** layers, we need to report this file size as zero even though it is
  ** really 1.   Ticket #3260.
  */
  ఒకవేళ( *p_కొలత==1 ) *p_కొలత = 0;


  తిరిగి SQLITE_OK;
}

#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
/*
** Handler for proxy-locking file-control verbs.  Defined below in the
** proxying locking division.
*/
స్థిర పూర్ణాంకం proxyFileControl(sqlite3_file*,పూర్ణాంకం,శూన్య*);
#అయితే_ముగింపు

/*
** This function is called to handle the SQLITE_FCNTL_SIZE_HINT
** file-control operation.  Enlarge the database to nBytes in size
** (rounded up to the next chunk-size).  If the database is already
** nBytes or larger, this routine is a no-op.
*/
స్థిర పూర్ణాంకం fcntl_కొలత_Hint(unixFile *pFile, i64 nByte){
  ఒకవేళ( pFile->szChunk>0 ){
    i64 n_కొలత;                    /* Required file size */
    నిర్మాణం stat buf;              /* Used to hold return values of fstat() */
 
    ఒకవేళ( osFstat(pFile->h, &buf) ){
      తిరిగి SQLITE_IOERR_FSTAT;
    }

    n_కొలత = ((nByte+pFile->szChunk-1) / pFile->szChunk) * pFile->szChunk;
    ఒకవేళ( n_కొలత>(i64)buf.st_కొలత ){

#ఒకవేళ defined(HAVE_POSIX_FALLOCATE) && HAVE_POSIX_FALLOCATE
      /* The code below is handling the return value of osFallocate()
      ** correctly. posix_fallocate() is defined to "returns zero on success,
      ** or an error number on  failure". See the manpage for details. */
      పూర్ణాంకం err;
      చేయి{
        err = osFallocate(pFile->h, buf.st_కొలత, n_కొలత-buf.st_కొలత);
      }అయితే( err==EINTR );
      ఒకవేళ( err && err!=EINVAL ) తిరిగి SQLITE_IOERR_WRITE;
#లేకపోతే
      /* If the OS does not have posix_fallocate(), fake it. Write a
      ** single byte to the last byte in each block that falls entirely
      ** within the extended region. Then, if required, a single byte
      ** at offset (nSize-1), to set the size of the file correctly.
      ** This is a similar technique to that used by glibc on systems
      ** that do not have a real fallocate() call.
      */
      పూర్ణాంకం nBlk = buf.st_blksize;  /* File-system block size */
      పూర్ణాంకం nWrite = 0;             /* Number of bytes written by seekAndWrite */
      i64 iWrite;                 /* Next offset to write to */

      iWrite = (buf.st_కొలత/nBlk)*nBlk + nBlk - 1;
      ప్రతిజ్ఞా( iWrite>=buf.st_కొలత );
      ప్రతిజ్ఞా( ((iWrite+1)%nBlk)==0 );
      కొరకు(/*no-op*/; iWrite<n_కొలత+nBlk-1; iWrite+=nBlk ){
        ఒకవేళ( iWrite>=n_కొలత ) iWrite = n_కొలత - 1;
        nWrite = seekAndWrite(pFile, iWrite, "", 1);
        ఒకవేళ( nWrite!=1 ) తిరిగి SQLITE_IOERR_WRITE;
      }
#అయితే_ముగింపు
    }
  }

#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  ఒకవేళ( pFile->mmap_కొలత_Max>0 && nByte>pFile->mmap_కొలత ){
    పూర్ణాంకం rc;
    ఒకవేళ( pFile->szChunk<=0 ){
      ఒకవేళ( robust_ftruncate(pFile->h, nByte) ){
        store_చివరి_Errno(pFile, errno);
        తిరిగి unixLogError(SQLITE_IOERR_TRUNCATE, "ftruncate", pFile->z_మార్గం);
      }
    }

    rc = unixMapfile(pFile, nByte);
    తిరిగి rc;
  }
#అయితే_ముగింపు

  తిరిగి SQLITE_OK;
}

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
స్థిర శూన్య unix_బహుళకం_బిట్టు(unixFile *pFile, గుర్తులేని అక్షరం ముసుగు, పూర్ణాంకం *pArg){
  ఒకవేళ( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & ముసుగు)!=0;
  }లేకపోతే ఒకవేళ( (*pArg)==0 ){
    pFile->ctrlFlags &= ~ముసుగు;
  }లేకపోతే{
    pFile->ctrlFlags |= ముసుగు;
  }
}

/* Forward declaration */
స్థిర పూర్ణాంకం unixGetTempname(పూర్ణాంకం nBuf, అక్షరం *zBuf);
#ఒకవేళ !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
 స్థిర పూర్ణాంకం unixFcntlExternalReader(unixFile*, పూర్ణాంకం*);
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
 స్థిర శూన్య unixDescribeShm(sqlite3_str*,unixShm*);
#అయితే_ముగింపు


/*
** Information and control of an open file handle.
*/
స్థిర పూర్ణాంకం unixFileControl(sqlite3_file *id, పూర్ణాంకం op, శూన్య *pArg){
  unixFile *pFile = (unixFile*)id;
  ఎంపిక( op ){
#ఒకవేళ defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
    స్థితి SQLITE_FCNTL_BEGIN_ATOMIC_WRITE: {
      పూర్ణాంకం rc = osIoctl(pFile->h, F2_FS_IOC_ప్రారంభం_ATOMIC_WRITE);
      తిరిగి rc ? SQLITE_IOERR_BEGIN_ATOMIC : SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_COMMIT_ATOMIC_WRITE: {
      పూర్ణాంకం rc = osIoctl(pFile->h, F2FS_IOC_COMMIT_ATOMIC_WRITE);
      తిరిగి rc ? SQLITE_IOERR_COMMIT_ATOMIC : SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_ROLLBACK_ATOMIC_WRITE: {
      పూర్ణాంకం rc = osIoctl(pFile->h, F2FS_IOC_ABORT_VOLATILE_WRITE);
      తిరిగి rc ? SQLITE_IOERR_ROLLBACK_ATOMIC : SQLITE_OK;
    }
#అయితే_ముగింపు /* __linux__ && SQLITE_ENABLE_BATCH_ATOMIC_WRITE */

    స్థితి SQLITE_FCNTL_NULL_IO: {
      osClose(pFile->h);
      pFile->h = -1;
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_LOCKSTATE: {
      *(పూర్ణాంకం*)pArg = pFile->e_File_తాళం;
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_చివరి_ERRNO: {
      *(పూర్ణాంకం*)pArg = pFile->చివరి_Errno;
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_CHUNK_కొలత: {
      pFile->szChunk = *(పూర్ణాంకం *)pArg;
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_కొలత_HINT: {
      పూర్ణాంకం rc;
      SimulateIOErrorBenign(1);
      rc = fcntl_కొలత_Hint(pFile, *(i64 *)pArg);
      SimulateIOErrorBenign(0);
      తిరిగి rc;
    }
    స్థితి SQLITE_FCNTL_PERSIST_WAL: {
      unix_బహుళకం_బిట్టు(pFile, UNIXFILE_PERSIST_WAL, (పూర్ణాంకం*)pArg);
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      unix_బహుళకం_బిట్టు(pFile, UNIXFILE_PSOW, (పూర్ణాంకం*)pArg);
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_VFSNAME: {
      *(అక్షరం**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_TEMPFILENAME: {
      అక్షరం *zTFile = sqlite3_malloc64( pFile->pVfs->mxPathname );
      ఒకవేళ( zTFile ){
        unixGetTempname(pFile->pVfs->mxPathname, zTFile);
        *(అక్షరం**)pArg = zTFile;
      }
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_HAS_MOVED: {
      *(పూర్ణాంకం*)pArg = fileHasMoved(pFile);
      తిరిగి SQLITE_OK;
    }
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
    స్థితి SQLITE_FCNTL_తాళం_TIMEOUT: {
      పూర్ణాంకం iOld = pFile->iBusyTimeout;
      పూర్ణాంకం iNew = *(పూర్ణాంకం*)pArg;
#ఒకవేళ SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = iNew<0 ? 0x7FFFFFFF : (గుర్తులేని)iNew;
#లేదావేళ SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = !!(*(పూర్ణాంకం*)pArg);
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
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
    స్థితి SQLITE_FCNTL_MMAP_కొలత: {
      i64 new_హద్దు = *(i64*)pArg;
      పూర్ణాంకం rc = SQLITE_OK;
      ఒకవేళ( new_హద్దు>sqlite3GlobalConfig.mxMmap ){
        new_హద్దు = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (size_t) and passed
      ** to mmap(). Restrict its value to 2GB if (size_t) is not at least a
      ** 64-bit type. */
      ఒకవేళ( new_హద్దు>0 && పరిమాణం(size_t)<8 ){
        new_హద్దు = (new_హద్దు & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmap_కొలత_Max;
      ఒకవేళ( new_హద్దు>=0 && new_హద్దు!=pFile->mmap_కొలత_Max && pFile->n_Fetch_బహిర్గతం==0 ){
        pFile->mmap_కొలత_Max = new_హద్దు;
        ఒకవేళ( pFile->mmap_కొలత>0 ){
          unixUnmapfile(pFile);
          rc = unixMapfile(pFile, -1);
        }
      }
      తిరిగి rc;
    }
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_DEBUG
    /* The pager calls this method to signal that it has done
    ** a rollback and that the database is therefore unchanged and
    ** it hence it is OK for the transaction change counter to be
    ** unchanged.
    */
    స్థితి SQLITE_FCNTL_DB_UNCHANGED: {
      ((unixFile*)id)->dbUpdate = 0;
      తిరిగి SQLITE_OK;
    }
#అయితే_ముగింపు
#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    స్థితి SQLITE_FCNTL_సమితి_LOCKPROXYFILE:
    స్థితి SQLITE_FCNTL_GET_LOCKPROXYFILE: {
      తిరిగి proxyFileControl(id,op,pArg);
    }
#అయితే_ముగింపు /* SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__) */

    స్థితి SQLITE_FCNTL_EXTERNAL_READER: {
#ఒకవేళ !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
      తిరిగి unixFcntlExternalReader((unixFile*)id, (పూర్ణాంకం*)pArg);
#లేకపోతే
      *(పూర్ణాంకం*)pArg = 0;
      తిరిగి SQLITE_OK;
#అయితే_ముగింపు
    }

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    స్థితి SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      అక్షరం aLck[16];
      unixInodeInfo *pInode;
      స్థిర స్థిరరాశి అక్షరం *az_తాళం[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
      sqlite3_str_appendf(pStr, "{\"h\":%d", pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      ఒకవేళ( pFile->e_File_తాళం ){
        sqlite3_str_appendf(pStr, ",\"eFileLock\":\"%s\"", 
                                  az_తాళం[pFile->e_File_తాళం-1]);
        ఒకవేళ( unixPosixAdvisoryLocks(pFile->h, aLck)==SQLITE_OK ){
          sqlite3_str_appendf(pStr, ",\"pal\":\"%s\"", aLck);
        }
      }
      unixEnterMutex();
      ఒకవేళ( pFile->pShm ){
        sqlite3_str_appendall(pStr, ",\"shm\":");
        unixDescribeShm(pStr, pFile->pShm);
      }
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
      ఒకవేళ( pFile->mmap_కొలత ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmap_కొలత);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->n_Fetch_బహిర్గతం);
      }
#అయితే_ముగింపు
      ఒకవేళ( (pInode = pFile->pInode)!=0 ){
        sqlite3_str_appendf(pStr, ",\"inode\":{\"nRef\":%d",pInode->nRef);
        sqlite3_mutex_enter(pInode->p_తాళం_Mutex);
        sqlite3_str_appendf(pStr, ",\"nShared\":%d", pInode->nShared);
        ఒకవేళ( pInode->e_File_తాళం ){
          sqlite3_str_appendf(pStr, ",\"eFileLock\":\"%s\"", 
                                  az_తాళం[pInode->e_File_తాళం-1]);
        }
        ఒకవేళ( pInode->p_వాడని ){
          అక్షరం cSep = '[';
          Unix_వాడని_Fd *pUFd = pFile->pInode->p_వాడని;
          sqlite3_str_appendall(pStr, ",\"unusedFd\":");
          అయితే( pUFd ){
            sqlite3_str_appendf(pStr, "%c{\"fd\":%d,\"flags\":%d",
                                cSep, pUFd->fd, pUFd->flags);
            cSep = ',';
            ఒకవేళ( unixPosixAdvisoryLocks(pUFd->fd, aLck)==SQLITE_OK ){
              sqlite3_str_appendf(pStr, ",\"pal\":\"%s\"", aLck);
            }
            sqlite3_str_append(pStr, "}", 1);
            pUFd = pUFd->p_తదుపరి;
          }
          sqlite3_str_append(pStr, "]", 1);
        }
        sqlite3_mutex_leave(pInode->p_తాళం_Mutex);
        sqlite3_str_append(pStr, "}", 1);
      }
      unixLeaveMutex();
      sqlite3_str_append(pStr, "}", 1);
      తిరిగి SQLITE_OK;
    }
#అయితే_ముగింపు /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */
  }
  తిరిగి SQLITE_NOTFOUND;
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
#నిర్వచితం_కాకపోతే __QNXNTO__
స్థిర శూన్య సమితి_Device_Characteristics(unixFile *pFd){
  ప్రతిజ్ఞా( pFd->deviceCharacteristics==0 || pFd->sector_కొలత!=0 );
  ఒకవేళ( pFd->sector_కొలత==0 ){
#ఒకవేళ defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
    పూర్ణాంకం res;
    u32 f = 0;

    /* Check for support for F2FS atomic batch writes. */
    res = osIoctl(pFd->h, F2FS_IOC_GET_FEATURES, &f);
    ఒకవేళ( res==0 && (f & F2FS_FEATURE_ATOMIC_WRITE) ){
      pFd->deviceCharacteristics = SQLITE_IOCAP_BATCH_ATOMIC;
    }
#అయితే_ముగింపు /* __linux__ && SQLITE_ENABLE_BATCH_ATOMIC_WRITE */

    /* Set the POWERSAFE_OVERWRITE flag if requested. */
    ఒకవేళ( pFd->ctrlFlags & UNIXFILE_PSOW ){
      pFd->deviceCharacteristics |= SQLITE_IOCAP_POWERSAFE_OVERWRITE;
    }
    pFd->deviceCharacteristics |= SQLITE_IOCAP_SUBPAGE_READ;

    pFd->sector_కొలత = SQLITE_DEFAULT_SECTOR_కొలత;
  }
}
#లేకపోతే
#అన్తర్భావ <sys/dcmd_blk.h>
#అన్తర్భావ <sys/statvfs.h>
స్థిర శూన్య సమితి_Device_Characteristics(unixFile *pFile){
  ఒకవేళ( pFile->sector_కొలత == 0 ){
    నిర్మాణం statvfs fsInfo;

    /* Set defaults for non-supported filesystems */
    pFile->sector_కొలత = SQLITE_DEFAULT_SECTOR_కొలత;
    pFile->deviceCharacteristics = 0;
    ఒకవేళ( fstatvfs(pFile->h, &fsInfo) == -1 ) {
      తిరిగి;
    }

    ఒకవేళ( !strcmp(fsInfo.f_basetype, "tmp") ) {
      pFile->sector_కొలత = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC4K |       /* All ram filesystem writes are atomic */
        SQLITE_IOCAP_సురక్షిత_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }లేకపోతే ఒకవేళ( అక్షరశ్రేణి_శోధన(fsInfo.f_basetype, "etfs") ){
      pFile->sector_కొలత = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* etfs cluster size writes are atomic */
        (pFile->sector_కొలత / 512 * SQLITE_IOCAP_ATOMIC512) |
        SQLITE_IOCAP_సురక్షిత_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }లేకపోతే ఒకవేళ( !strcmp(fsInfo.f_basetype, "qnx6") ){
      pFile->sector_కొలత = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC |         /* All filesystem writes are atomic */
        SQLITE_IOCAP_సురక్షిత_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }లేకపోతే ఒకవేళ( !strcmp(fsInfo.f_basetype, "qnx4") ){
      pFile->sector_కొలత = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* full bitset of atomics from max sector size and smaller */
        (((pFile->sector_కొలత / 512 * SQLITE_IOCAP_ATOMIC512) << 1) - 2) |
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }లేకపోతే ఒకవేళ( అక్షరశ్రేణి_శోధన(fsInfo.f_basetype, "dos") ){
      pFile->sector_కొలత = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* full bitset of atomics from max sector size and smaller */
        (((pFile->sector_కొలత / 512 * SQLITE_IOCAP_ATOMIC512) << 1) - 2) |
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }లేకపోతే{
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC512 |      /* blocks are atomic */
        SQLITE_IOCAP_సురక్షిత_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        0;
    }
  }
  /* Last chance verification.  If the sector size isn't a multiple of 512
  ** then it isn't valid.*/
  ఒకవేళ( pFile->sector_కొలత % 512 != 0 ){
    pFile->deviceCharacteristics = 0;
    pFile->sector_కొలత = SQLITE_DEFAULT_SECTOR_కొలత;
  }
}
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం unix_Sector_కొలత(sqlite3_file *id){
  unixFile *pFd = (unixFile*)id;
  సమితి_Device_Characteristics(pFd);
  తిరిగి pFd->sector_కొలత;
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
స్థిర పూర్ణాంకం unixDeviceCharacteristics(sqlite3_file *id){
  unixFile *pFd = (unixFile*)id;
  సమితి_Device_Characteristics(pFd);
  తిరిగి pFd->deviceCharacteristics;
}

#ఒకవేళ !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_కొలత>0

/*
** Return the system page size.
**
** This function should not be called directly by other code in this file.
** Instead, it should be called via macro osGetpagesize().
*/
స్థిర పూర్ణాంకం unixGetpagesize(శూన్య){
#ఒకవేళ OS_VXWORKS
  తిరిగి 1024;
#లేదావేళ defined(BSD_ఆధారం)
  తిరిగి getpagesize();
#లేకపోతే
  తిరిగి (పూర్ణాంకం)sysconf(_SC_PAGESIZE);
#అయితే_ముగింపు
}

#అయితే_ముగింపు /* !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0 */

#ఒకవేళ !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)

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
నిర్మాణం unix_Shm_కణుపు {
  unixInodeInfo *pInode;     /* unixInodeInfo that owns this SHM node */
  sqlite3_mutex *pShmMutex;  /* Mutex to access this object */
  అక్షరం *zFilename;           /* Name of the mmapped file */
  పూర్ణాంకం hShm;                  /* Open file descriptor */
  పూర్ణాంకం szRegion;              /* Size of shared-memory regions */
  u16 nRegion;               /* Size of array apRegion */
  u8 isReadonly;             /* True if read-only */
  u8 isUnlocked;             /* True if no DMS lock held */
  అక్షరం **apRegion;           /* Array of mapped shared-memory regions */
  పూర్ణాంకం nRef;                  /* Number of unixShm objects pointing to this */
  unixShm *p_మొదటి;           /* All unixShm objects pointing to this */
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  sqlite3_mutex *aMutex[SQLITE_SHM_NLOCK];
#అయితే_ముగింపు
  పూర్ణాంకం a_తాళం[SQLITE_SHM_NLOCK];  /* # shared locks on slot, -1==excl lock */
#నిర్వచితమైతే SQLITE_DEBUG
  u8 తదుపరి_Shm_Id;              /* Next available unixShm.id value */
#అయితే_ముగింపు
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
నిర్మాణం unixShm {
  unix_Shm_కణుపు *p_Shm_కణుపు;     /* The underlying unixShmNode object */
  unixShm *p_తదుపరి;            /* Next unixShm with the same unixShmNode */
  u8 hasMutex;               /* True if holding the unixShmNode->pShmMutex */
  u8 id;                     /* Id of this connection within its unixShmNode */
  u16 shared_ముసుగు;            /* Mask of shared locks held */
  u16 excl_ముసుగు;              /* Mask of exclusive locks held */
};

/*
** Constants used for locking
*/
#నిర్వచించు UNIX_SHM_పునాది   ((22+SQLITE_SHM_NLOCK)*4)         /* first lock byte */
#నిర్వచించు UNIX_SHM_DMS    (UNIX_SHM_పునాది+SQLITE_SHM_NLOCK)  /* deadman switch */

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Describe the pShm object using JSON.  Used for diagnostics only.
*/
స్థిర శూన్య unixDescribeShm(sqlite3_str *pStr, unixShm *pShm){
  unix_Shm_కణుపు *p_కణుపు = pShm->p_Shm_కణుపు;
  అక్షరం aLck[16];
  sqlite3_str_appendf(pStr, "{\"h\":%d", p_కణుపు->hShm);
  ప్రతిజ్ఞా( unixMutexHeld() );
  sqlite3_str_appendf(pStr, ",\"nRef\":%d", p_కణుపు->nRef);
  sqlite3_str_appendf(pStr, ",\"id\":%d", pShm->id);
  sqlite3_str_appendf(pStr, ",\"sharedMask\":%d", pShm->shared_ముసుగు);
  sqlite3_str_appendf(pStr, ",\"exclMask\":%d", pShm->excl_ముసుగు);
  ఒకవేళ( unixPosixAdvisoryLocks(p_కణుపు->hShm, aLck)==SQLITE_OK ){
    sqlite3_str_appendf(pStr, ",\"pal\":\"%s\"", aLck);
  }
  sqlite3_str_append(pStr, "}", 1);
}
#అయితే_ముగింపు /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

/*
** Use F_GETLK to check whether or not there are any readers with open
** wal-mode transactions in other processes on database file pFile. If
** no error occurs, return SQLITE_OK and set (*piOut) to 1 if there are
** such transactions, or 0 otherwise. If an error occurs, return an
** SQLite error code. The final value of *piOut is undefined in this
** case.
*/
స్థిర పూర్ణాంకం unixFcntlExternalReader(unixFile *pFile, పూర్ణాంకం *pi_బహిర్గతం){
  పూర్ణాంకం rc = SQLITE_OK;
  *pi_బహిర్గతం = 0;
  ఒకవేళ( pFile->pShm){
    unix_Shm_కణుపు *p_Shm_కణుపు = pFile->pShm->p_Shm_కణుపు;
    నిర్మాణం flock f;

    memset(&f, 0, పరిమాణం(f));
    f.l_type = F_WRLCK;
    f.l_whence = SEEK_SET;
    f.l_ప్రారంభం = UNIX_SHM_పునాది + 3;
    f.l_len = SQLITE_SHM_NLOCK - 3;

    sqlite3_mutex_enter(p_Shm_కణుపు->pShmMutex);
    ఒకవేళ( osFcntl(p_Shm_కణుపు->hShm, F_GETLK, &f)<0 ){
      rc = SQLITE_IOERR_తాళం;
    }లేకపోతే{
      *pi_బహిర్గతం = (f.l_type!=F_UNLCK);
    }
    sqlite3_mutex_leave(p_Shm_కణుపు->pShmMutex);
  }

  తిరిగి rc;
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
స్థిర పూర్ణాంకం unix_Is_Sharing_Shm_కణుపు(unixFile *pFile){
  unix_Shm_కణుపు *p_Shm_కణుపు;
  నిర్మాణం flock తాళం;
  ఒకవేళ( pFile->pShm==0 ) తిరిగి 0;
  ఒకవేళ( pFile->ctrlFlags & UNIXFILE_EXCL ) తిరిగి 0;
  p_Shm_కణుపు = pFile->pShm->p_Shm_కణుపు;
#ఒకవేళ SQLITE_ATOMIC_INTRINSICS
  ప్రతిజ్ఞా( Atomic_ఎక్కించు(&p_Shm_కణుపు->nRef)==1 );
#అయితే_ముగింపు
  memset(&తాళం, 0, పరిమాణం(తాళం));
  తాళం.l_whence = SEEK_SET;
  తాళం.l_ప్రారంభం = UNIX_SHM_DMS;
  తాళం.l_len = 1;
  తాళం.l_type = F_WRLCK;
  osFcntl(p_Shm_కణుపు->hShm, F_GETLK, &తాళం);
  తిరిగి (తాళం.l_type!=F_UNLCK);
}

/*
** Apply posix advisory locks for all bytes from ofst through ofst+n-1.
**
** Locks block if the mask is exactly UNIX_SHM_C and are non-blocking
** otherwise.
*/
స్థిర పూర్ణాంకం unix_Shm_System_తాళం(
  unixFile *pFile,       /* Open connection to the WAL file */
  పూర్ణాంకం తాళం_Type,          /* F_UNLCK, F_RDLCK, or F_WRLCK */
  పూర్ణాంకం ofst,              /* First byte of the locking range */
  పూర్ణాంకం n                  /* Number of bytes to lock */
){
  unix_Shm_కణుపు *p_Shm_కణుపు; /* Apply locks to this open shared-memory segment */
  నిర్మాణం flock f;        /* The posix advisory locking structure */
  పూర్ణాంకం rc = SQLITE_OK;    /* Result code form fcntl() */

  p_Shm_కణుపు = pFile->pInode->p_Shm_కణుపు;

  /* Assert that the parameters are within expected range and that the
  ** correct mutex or mutexes are held. */
  ప్రతిజ్ఞా( p_Shm_కణుపు->nRef>=0 );
  ప్రతిజ్ఞా( (ofst==UNIX_SHM_DMS && n==1)
       || (ofst>=UNIX_SHM_పునాది && ofst+n<=(UNIX_SHM_పునాది+SQLITE_SHM_NLOCK))
  );
  ఒకవేళ( ofst==UNIX_SHM_DMS ){
    ప్రతిజ్ఞా( p_Shm_కణుపు->nRef>0 || unixMutexHeld() );
    ప్రతిజ్ఞా( p_Shm_కణుపు->nRef==0 || sqlite3_mutex_held(p_Shm_కణుపు->pShmMutex) );
  }లేకపోతే{
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
    పూర్ణాంకం ii;
    కొరకు(ii=ofst-UNIX_SHM_పునాది; ii<ofst-UNIX_SHM_పునాది+n; ii++){
      ప్రతిజ్ఞా( sqlite3_mutex_held(p_Shm_కణుపు->aMutex[ii]) );
    }
#లేకపోతే
    ప్రతిజ్ఞా( sqlite3_mutex_held(p_Shm_కణుపు->pShmMutex) );
    ప్రతిజ్ఞా( p_Shm_కణుపు->nRef>0 );
#అయితే_ముగింపు
  }

  /* Shared locks never span more than one byte */
  ప్రతిజ్ఞా( n==1 || తాళం_Type!=F_RDLCK );

  /* Locks are within range */
  ప్రతిజ్ఞా( n>=1 && n<=SQLITE_SHM_NLOCK );
  ప్రతిజ్ఞా( ofst>=UNIX_SHM_పునాది && ofst<=UNIX_SHM_DMS );
  ప్రతిజ్ఞా( ofst+n-1<=UNIX_SHM_DMS );

  ఒకవేళ( p_Shm_కణుపు->hShm>=0 ){
    పూర్ణాంకం res;
    /* Initialize the locking parameters */
    f.l_type = తాళం_Type;
    f.l_whence = SEEK_SET;
    f.l_ప్రారంభం = ofst;
    f.l_len = n;
    res = os_సమితి_Posix_Advisory_తాళం(p_Shm_కణుపు->hShm, &f, pFile);
    ఒకవేళ( res==-1 ){
#ఒకవేళ defined(SQLITE_ENABLE_SETLK_TIMEOUT) && SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = (pFile->iBusyTimeout ? SQLITE_BUSY_TIMEOUT : SQLITE_BUSY);
#లేకపోతే
      rc = SQLITE_BUSY;
#అయితే_ముగింపు
    }
  }

  /* Do debug tracing */
#నిర్వచితమైతే SQLITE_DEBUG
  OSTRACE(("SHM-LOCK "));
  ఒకవేళ( rc==SQLITE_OK ){
    ఒకవేళ( తాళం_Type==F_UNLCK ){
      OSTRACE(("unlock %d..%d ok\n", ofst, ofst+n-1));
    }లేకపోతే ఒకవేళ( తాళం_Type==F_RDLCK ){
      OSTRACE(("read-lock %d..%d ok\n", ofst, ofst+n-1));
    }లేకపోతే{
      ప్రతిజ్ఞా( తాళం_Type==F_WRLCK );
      OSTRACE(("write-lock %d..%d ok\n", ofst, ofst+n-1));
    }
  }లేకపోతే{
    ఒకవేళ( తాళం_Type==F_UNLCK ){
      OSTRACE(("unlock %d..%d failed\n", ofst, ofst+n-1));
    }లేకపోతే ఒకవేళ( తాళం_Type==F_RDLCK ){
      OSTRACE(("read-lock %d..%d failed\n", ofst, ofst+n-1));
    }లేకపోతే{
      ప్రతిజ్ఞా( తాళం_Type==F_WRLCK );
      OSTRACE(("write-lock %d..%d failed\n", ofst, ofst+n-1));
    }
  }
#అయితే_ముగింపు

  తిరిగి rc;
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
స్థిర పూర్ణాంకం unix_Shm_Region_చొప్పున_Map(శూన్య){
  పూర్ణాంకం shmsz = 32*1024;            /* SHM region size */
  పూర్ణాంకం pgsz = osGetpagesize();   /* System page size */
  ప్రతిజ్ఞా( ((pgsz-1)&pgsz)==0 );   /* Page size must be a power of 2 */
  ఒకవేళ( pgsz<shmsz ) తిరిగి 1;
  తిరిగి pgsz/shmsz;
}

/*
** Purge the unixShmNodeList list of all entries with unixShmNode.nRef==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
స్థిర శూన్య unixShmPurge(unixFile *pFd){
  unix_Shm_కణుపు *p = pFd->pInode->p_Shm_కణుపు;
  ప్రతిజ్ఞా( unixMutexHeld() );
  ఒకవేళ( p && ALWAYS(p->nRef==0) ){
    పూర్ణాంకం n_Shm_చొప్పున_Map = unix_Shm_Region_చొప్పున_Map();
    పూర్ణాంకం i;
    ప్రతిజ్ఞా( p->pInode==pFd->pInode );
    sqlite3_mutex_free(p->pShmMutex);
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
    కొరకు(i=0; i<SQLITE_SHM_NLOCK; i++){
      sqlite3_mutex_free(p->aMutex[i]);
    }
#అయితే_ముగింపు
    కొరకు(i=0; i<p->nRegion; i+=n_Shm_చొప్పున_Map){
      ఒకవేళ( p->hShm>=0 ){
        osMunmap(p->apRegion[i], p->szRegion);
      }లేకపోతే{
        sqlite3_free(p->apRegion[i]);
      }
    }
    sqlite3_free(p->apRegion);
    ఒకవేళ( p->hShm>=0 ){
      robust_close(pFd, p->hShm, __LINE__);
      p->hShm = -1;
    }
    p->pInode->p_Shm_కణుపు = 0;
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
స్థిర పూర్ణాంకం unix_తాళం_Shared_జ్ఞాపకం(unixFile *pDbFd, unix_Shm_కణుపు *p_Shm_కణుపు){
  నిర్మాణం flock తాళం;
  పూర్ణాంకం rc = SQLITE_OK;

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
  తాళం.l_whence = SEEK_SET;
  తాళం.l_ప్రారంభం = UNIX_SHM_DMS;
  తాళం.l_len = 1;
  తాళం.l_type = F_WRLCK;
  ఒకవేళ( osFcntl(p_Shm_కణుపు->hShm, F_GETLK, &తాళం)!=0 ) {
    rc = SQLITE_IOERR_తాళం;
  }లేకపోతే ఒకవేళ( తాళం.l_type==F_UNLCK ){
    ఒకవేళ( p_Shm_కణుపు->isReadonly ){
      p_Shm_కణుపు->isUnlocked = 1;
      rc = SQLITE_READONLY_CANTINIT;
    }లేకపోతే{
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
      /* Do not use a blocking lock here. If the lock cannot be obtained
      ** immediately, it means some other connection is truncating the
      ** *-shm file. And after it has done so, it will not release its
      ** lock, but only downgrade it to a shared lock. So no point in
      ** blocking here. The call below to obtain the shared DMS lock may 
      ** use a blocking lock. */
      పూర్ణాంకం iSaveTimeout = pDbFd->iBusyTimeout;
      pDbFd->iBusyTimeout = 0;
#అయితే_ముగింపు
      rc = unix_Shm_System_తాళం(pDbFd, F_WRLCK, UNIX_SHM_DMS, 1);
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
      pDbFd->iBusyTimeout = iSaveTimeout;
#అయితే_ముగింపు
      /* The first connection to attach must truncate the -shm file.  We
      ** truncate to 3 bytes (an arbitrary small number, less than the
      ** -shm header size) rather than 0 as a system debugging aid, to
      ** help detect if a -shm file truncation is legitimate or is the work
      ** or a rogue process. */
      ఒకవేళ( rc==SQLITE_OK && robust_ftruncate(p_Shm_కణుపు->hShm, 3) ){
        rc = unixLogError(SQLITE_IOERR_SHMOPEN,"ftruncate",p_Shm_కణుపు->zFilename);
      }
    }
  }లేకపోతే ఒకవేళ( తాళం.l_type==F_WRLCK ){
    rc = SQLITE_BUSY;
  }

  ఒకవేళ( rc==SQLITE_OK ){
    ప్రతిజ్ఞా( తాళం.l_type==F_UNLCK || తాళం.l_type==F_RDLCK );
    rc = unix_Shm_System_తాళం(pDbFd, F_RDLCK, UNIX_SHM_DMS, 1);
  }
  తిరిగి rc;
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
స్థిర పూర్ణాంకం unix_Open_Shared_జ్ఞాపకం(unixFile *pDbFd){
  నిర్మాణం unixShm *p = 0;          /* The connection to be opened */
  నిర్మాణం unix_Shm_కణుపు *p_Shm_కణుపు;   /* The underlying mmapped file */
  పూర్ణాంకం rc = SQLITE_OK;             /* Result code */
  unixInodeInfo *pInode;          /* The inode of fd */
  అక్షరం *zShm;             /* Name of the file used for SHM */
  పూర్ణాంకం nShmFilename;               /* Size of the SHM filename in bytes */

  /* Allocate space for the new unixShm object. */
  p = sqlite3_malloc64( పరిమాణం(*p) );
  ఒకవేళ( p==0 ) తిరిగి SQLITE_NOMEM_BKPT;
  memset(p, 0, పరిమాణం(*p));
  ప్రతిజ్ఞా( pDbFd->pShm==0 );

  /* Check to see if a unixShmNode object already exists. Reuse an existing
  ** one if present. Create a new one if necessary.
  */
  ప్రతిజ్ఞా( unixFileMutexNotheld(pDbFd) );
  unixEnterMutex();
  pInode = pDbFd->pInode;
  p_Shm_కణుపు = pInode->p_Shm_కణుపు;
  ఒకవేళ( p_Shm_కణుపు==0 ){
    నిర్మాణం stat sStat;                 /* fstat() info for database file */
#నిర్వచితం_కాకపోతే SQLITE_SHM_DIRECTORY
    స్థిరరాశి అక్షరం *z_పునాది_మార్గం = pDbFd->z_మార్గం;
#అయితే_ముగింపు

    /* Call fstat() to figure out the permissions on the database file. If
    ** a new *-shm file is created, an attempt will be made to create it
    ** with the same permissions.
    */
    ఒకవేళ( osFstat(pDbFd->h, &sStat) ){
      rc = SQLITE_IOERR_FSTAT;
      వెళ్ళు shm_open_err;
    }

#నిర్వచితమైతే SQLITE_SHM_DIRECTORY
    nShmFilename = పరిమాణం(SQLITE_SHM_DIRECTORY) + 31;
#లేకపోతే
    nShmFilename = 6 + (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(z_పునాది_మార్గం);
#అయితే_ముగింపు
    p_Shm_కణుపు = sqlite3_malloc64( పరిమాణం(*p_Shm_కణుపు) + nShmFilename );
    ఒకవేళ( p_Shm_కణుపు==0 ){
      rc = SQLITE_NOMEM_BKPT;
      వెళ్ళు shm_open_err;
    }
    memset(p_Shm_కణుపు, 0, పరిమాణం(*p_Shm_కణుపు)+nShmFilename);
    zShm = p_Shm_కణుపు->zFilename = (అక్షరం*)&p_Shm_కణుపు[1];
#నిర్వచితమైతే SQLITE_SHM_DIRECTORY
    sqlite3_snprintf(nShmFilename, zShm,
                     SQLITE_SHM_DIRECTORY "/sqlite-shm-%x-%x",
                     (u32)sStat.st_ino, (u32)sStat.st_dev);
#లేకపోతే
    sqlite3_snprintf(nShmFilename, zShm, "%s-shm", z_పునాది_మార్గం);
    sqlite3FileSuffix3(pDbFd->z_మార్గం, zShm);
#అయితే_ముగింపు
    p_Shm_కణుపు->hShm = -1;
    pDbFd->pInode->p_Shm_కణుపు = p_Shm_కణుపు;
    p_Shm_కణుపు->pInode = pDbFd->pInode;
    ఒకవేళ( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_కణుపు->pShmMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      ఒకవేళ( p_Shm_కణుపు->pShmMutex==0 ){
        rc = SQLITE_NOMEM_BKPT;
        వెళ్ళు shm_open_err;
      }
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
      {
        పూర్ణాంకం ii;
        కొరకు(ii=0; ii<SQLITE_SHM_NLOCK; ii++){
          p_Shm_కణుపు->aMutex[ii] = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
          ఒకవేళ( p_Shm_కణుపు->aMutex[ii]==0 ){
            rc = SQLITE_NOMEM_BKPT;
            వెళ్ళు shm_open_err;
          }
        }
      }
#అయితే_ముగింపు
    }

    ఒకవేళ( pInode->b_Process_తాళం==0 ){
      ఒకవేళ( 0==sqlite3_uri_boolean(pDbFd->z_మార్గం, "readonly_shm", 0) ){
        p_Shm_కణుపు->hShm = robust_open(zShm, O_RDWR|O_CREAT|O_NOFOLLOW,
                                     (sStat.st_బహుళకం&0777));
      }
      ఒకవేళ( p_Shm_కణుపు->hShm<0 ){
        p_Shm_కణుపు->hShm = robust_open(zShm, O_RDONLY|O_NOFOLLOW,
                                     (sStat.st_బహుళకం&0777));
        ఒకవేళ( p_Shm_కణుపు->hShm<0 ){
          rc = unixLogError(SQLITE_CANTOPEN_BKPT, "open", zShm);
          వెళ్ళు shm_open_err;
        }
        p_Shm_కణుపు->isReadonly = 1;
      }

      /* If this process is running as root, make sure that the SHM file
      ** is owned by the same user that owns the original database.  Otherwise,
      ** the original owner will not be able to connect.
      */
      robustFchown(p_Shm_కణుపు->hShm, sStat.st_uid, sStat.st_gid);

      rc = unix_తాళం_Shared_జ్ఞాపకం(pDbFd, p_Shm_కణుపు);
      ఒకవేళ( rc!=SQLITE_OK && rc!=SQLITE_READONLY_CANTINIT ) వెళ్ళు shm_open_err;
    }
  }

  /* Make the new connection a child of the unixShmNode */
  p->p_Shm_కణుపు = p_Shm_కణుపు;
#నిర్వచితమైతే SQLITE_DEBUG
  p->id = p_Shm_కణుపు->తదుపరి_Shm_Id++;
#అయితే_ముగింపు
  p_Shm_కణుపు->nRef++;
  pDbFd->pShm = p;
  unixLeaveMutex();

  /* The reference count on pShmNode has already been incremented under
  ** the cover of the unixEnterMutex() mutex and the pointer from the
  ** new (struct unixShm) object to the pShmNode has been set. All that is
  ** left to do is to link the new object into the linked list starting
  ** at pShmNode->pFirst. This must be done while holding the
  ** pShmNode->pShmMutex.
  */
  sqlite3_mutex_enter(p_Shm_కణుపు->pShmMutex);
  p->p_తదుపరి = p_Shm_కణుపు->p_మొదటి;
  p_Shm_కణుపు->p_మొదటి = p;
  sqlite3_mutex_leave(p_Shm_కణుపు->pShmMutex);
  తిరిగి rc;

  /* Jump here on any error */
shm_open_err:
  unixShmPurge(pDbFd);       /* This call frees pShmNode if required */
  sqlite3_free(p);
  unixLeaveMutex();
  తిరిగి rc;
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
స్థిర పూర్ణాంకం unixShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  పూర్ణాంకం iRegion,                    /* Region to retrieve */
  పూర్ణాంకం szRegion,                   /* Size of regions */
  పూర్ణాంకం bExtend,                    /* True to extend file if necessary */
  శూన్య అస్థిర **pp              /* OUT: Mapped memory */
){
  unixFile *pDbFd = (unixFile*)fd;
  unixShm *p;
  unix_Shm_కణుపు *p_Shm_కణుపు;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం n_Shm_చొప్పున_Map = unix_Shm_Region_చొప్పున_Map();
  పూర్ణాంకం nReqRegion;

  /* If the shared-memory file has not yet been opened, open it now. */
  ఒకవేళ( pDbFd->pShm==0 ){
    rc = unix_Open_Shared_జ్ఞాపకం(pDbFd);
    ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
  }

  p = pDbFd->pShm;
  p_Shm_కణుపు = p->p_Shm_కణుపు;
  sqlite3_mutex_enter(p_Shm_కణుపు->pShmMutex);
  ఒకవేళ( p_Shm_కణుపు->isUnlocked ){
    rc = unix_తాళం_Shared_జ్ఞాపకం(pDbFd, p_Shm_కణుపు);
    ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు shmpage_బహిర్గతం;
    p_Shm_కణుపు->isUnlocked = 0;
  }
  ప్రతిజ్ఞా( szRegion==p_Shm_కణుపు->szRegion || p_Shm_కణుపు->nRegion==0 );
  ప్రతిజ్ఞా( p_Shm_కణుపు->pInode==pDbFd->pInode );
  ప్రతిజ్ఞా( p_Shm_కణుపు->hShm>=0 || pDbFd->pInode->b_Process_తాళం==1 );
  ప్రతిజ్ఞా( p_Shm_కణుపు->hShm<0 || pDbFd->pInode->b_Process_తాళం==0 );

  /* Minimum number of regions required to be mapped. */
  nReqRegion = ((iRegion+n_Shm_చొప్పున_Map) / n_Shm_చొప్పున_Map) * n_Shm_చొప్పున_Map;

  ఒకవేళ( p_Shm_కణుపు->nRegion<nReqRegion ){
    అక్షరం **apNew;                         /* New apRegion[] array */
    i64 nByte = nReqRegion*(i64)szRegion; /* Minimum required file size */
    నిర్మాణం stat sStat;                    /* Used by fstat() */

    p_Shm_కణుపు->szRegion = szRegion;

    ఒకవేళ( p_Shm_కణుపు->hShm>=0 ){
      /* The requested region is not mapped into this processes address space.
      ** Check to see if it has been allocated (i.e. if the wal-index file is
      ** large enough to contain the requested region).
      */
      ఒకవేళ( osFstat(p_Shm_కణుపు->hShm, &sStat) ){
        rc = SQLITE_IOERR_SHMSIZE;
        వెళ్ళు shmpage_బహిర్గతం;
      }

      ఒకవేళ( sStat.st_కొలత<nByte ){
        /* The requested memory region does not exist. If bExtend is set to
        ** false, exit early. *pp will be set to NULL and SQLITE_OK returned.
        */
        ఒకవేళ( !bExtend ){
          వెళ్ళు shmpage_బహిర్గతం;
        }

        /* Alternatively, if bExtend is true, extend the file. Do this by
        ** writing a single byte to the end of each (OS) page being
        ** allocated or extended. Technically, we need only write to the
        ** last page in order to extend the file. But writing to all new
        ** pages forces the OS to allocate them immediately, which reduces
        ** the chances of SIGBUS while accessing the mapped region later on.
        */
        లేకపోతే{
          స్థిర స్థిరరాశి పూర్ణాంకం pgsz = 4096;
          i64 iPg;

          /* Write to the last byte of each newly allocated or extended page */
          ప్రతిజ్ఞా( (nByte % pgsz)==0 );
          కొరకు(iPg=(sStat.st_కొలత/pgsz); iPg<(nByte/pgsz); iPg++){
            పూర్ణాంకం x = 0;
            ఒకవేళ( seekAndWriteFd(p_Shm_కణుపు->hShm, iPg*pgsz + pgsz-1,"",1,&x)!=1 ){
              స్థిరరాశి అక్షరం *zFile = p_Shm_కణుపు->zFilename;
              rc = unixLogError(SQLITE_IOERR_SHMSIZE, "write", zFile);
              వెళ్ళు shmpage_బహిర్గతం;
            }
          }
        }
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (అక్షరం **)sqlite3_realloc64(
        p_Shm_కణుపు->apRegion, nReqRegion*పరిమాణం(అక్షరం *)
    );
    ఒకవేళ( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      వెళ్ళు shmpage_బహిర్గతం;
    }
    p_Shm_కణుపు->apRegion = apNew;
    అయితే( p_Shm_కణుపు->nRegion<nReqRegion ){
      i64 nMap = (i64)szRegion*(i64)n_Shm_చొప్పున_Map;
      i64 i;
      శూన్య *pMem;
      ఒకవేళ( p_Shm_కణుపు->hShm>=0 ){
        pMem = osMmap(0, nMap,
            p_Shm_కణుపు->isReadonly ? PROT_READ : PROT_READ|PROT_WRITE,
            MAP_SHARED, p_Shm_కణుపు->hShm, szRegion*(i64)p_Shm_కణుపు->nRegion
        );
        ఒకవేళ( pMem==MAP_FAILED ){
          rc = unixLogError(SQLITE_IOERR_SHMMAP, "mmap", p_Shm_కణుపు->zFilename);
          వెళ్ళు shmpage_బహిర్గతం;
        }
      }లేకపోతే{
        pMem = sqlite3_malloc64(nMap);
        ఒకవేళ( pMem==0 ){
          rc = SQLITE_NOMEM_BKPT;
          వెళ్ళు shmpage_బహిర్గతం;
        }
        memset(pMem, 0, nMap);
      }

      కొరకు(i=0; i<n_Shm_చొప్పున_Map; i++){
        p_Shm_కణుపు->apRegion[p_Shm_కణుపు->nRegion+i] = &((అక్షరం*)pMem)[szRegion*i];
      }
      p_Shm_కణుపు->nRegion += n_Shm_చొప్పున_Map;
    }
  }

shmpage_బహిర్గతం:
  ఒకవేళ( p_Shm_కణుపు->nRegion>iRegion ){
    *pp = p_Shm_కణుపు->apRegion[iRegion];
  }లేకపోతే{
    *pp = 0;
  }
  ఒకవేళ( p_Shm_కణుపు->isReadonly && rc==SQLITE_OK ) rc = SQLITE_READONLY;
  sqlite3_mutex_leave(p_Shm_కణుపు->pShmMutex);
  తిరిగి rc;
}

/*
** Check that the pShmNode->aLock[] array comports with the locking bitmasks
** held by each client. Return true if it does, or false otherwise. This
** is to be used in an assert(). e.g.
**
**     assert( assertLockingArrayOk(pShmNode) );
*/
#నిర్వచితమైతే SQLITE_DEBUG
స్థిర పూర్ణాంకం assertLockingArrayOk(unix_Shm_కణుపు *p_Shm_కణుపు){
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  తిరిగి 1;
#లేకపోతే
  unixShm *pX;
  పూర్ణాంకం a_తాళం[SQLITE_SHM_NLOCK];

  memset(a_తాళం, 0, పరిమాణం(a_తాళం));
  కొరకు(pX=p_Shm_కణుపు->p_మొదటి; pX; pX=pX->p_తదుపరి){
    పూర్ణాంకం i;
    కొరకు(i=0; i<SQLITE_SHM_NLOCK; i++){
      ఒకవేళ( pX->excl_ముసుగు & (1<<i) ){
        ప్రతిజ్ఞా( a_తాళం[i]==0 );
        a_తాళం[i] = -1;
      }లేకపోతే ఒకవేళ( pX->shared_ముసుగు & (1<<i) ){
        ప్రతిజ్ఞా( a_తాళం[i]>=0 );
        a_తాళం[i]++;
      }
    }
  }

  ప్రతిజ్ఞా( 0==memcmp(p_Shm_కణుపు->a_తాళం, a_తాళం, పరిమాణం(a_తాళం)) );
  తిరిగి (memcmp(p_Shm_కణుపు->a_తాళం, a_తాళం, పరిమాణం(a_తాళం))==0);
#అయితే_ముగింపు
}
#అయితే_ముగింపు /* !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL) */

/*
** Change the lock state for a shared-memory segment.
**
** Note that the relationship between SHARED and EXCLUSIVE locks is a little
** different here than in posix.  In xShmLock(), one can go from unlocked
** to shared and back or from unlocked to exclusive and back.  But one may
** not go from shared to exclusive or from exclusive to shared.
*/
స్థిర పూర్ణాంకం unix_Shm_తాళం(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  పూర్ణాంకం ofst,                  /* First lock to acquire or release */
  పూర్ణాంకం n,                     /* Number of locks to acquire or release */
  పూర్ణాంకం flags                  /* What to do with the lock */
){
  unixFile *pDbFd = (unixFile*)fd;      /* Connection holding shared memory */
  unixShm *p;                           /* The shared memory being locked */
  unix_Shm_కణుపు *p_Shm_కణుపు;                /* The underlying file iNode */
  పూర్ణాంకం rc = SQLITE_OK;                   /* Result code */
  u16 ముసుగు = (1<<(ofst+n)) - (1<<ofst); /* Mask of locks to take or release */
  పూర్ణాంకం *a_తాళం;

  p = pDbFd->pShm;
  ఒకవేళ( p==0 ) తిరిగి SQLITE_IOERR_SHMLOCK;
  p_Shm_కణుపు = p->p_Shm_కణుపు;
  ఒకవేళ( NEVER(p_Shm_కణుపు==0) ) తిరిగి SQLITE_IOERR_SHMLOCK;
  a_తాళం = p_Shm_కణుపు->a_తాళం;

  ప్రతిజ్ఞా( p_Shm_కణుపు==pDbFd->pInode->p_Shm_కణుపు );
  ప్రతిజ్ఞా( p_Shm_కణుపు->pInode==pDbFd->pInode );
  ప్రతిజ్ఞా( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  ప్రతిజ్ఞా( n>=1 );
  ప్రతిజ్ఞా( flags==(SQLITE_SHM_తాళం | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_తాళం | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  ప్రతిజ్ఞా( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );
  ప్రతిజ్ఞా( p_Shm_కణుపు->hShm>=0 || pDbFd->pInode->b_Process_తాళం==1 );
  ప్రతిజ్ఞా( p_Shm_కణుపు->hShm<0 || pDbFd->pInode->b_Process_తాళం==0 );

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
  ** This is assert()ed below.
  */
  ప్రతిజ్ఞా( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_తాళం) 
       || 0==(p->excl_ముసుగు & ముసుగు) 
  );
  ఒకవేళ( ((flags & SQLITE_SHM_UNLOCK) && ((p->excl_ముసుగు|p->shared_ముసుగు) & ముసుగు))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_తాళం) && 0==(p->shared_ముసుగు & ముసుగు))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_తాళం))
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
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
    పూర్ణాంకం iMutex;
    కొరకు(iMutex=ofst; iMutex<ofst+n; iMutex++){
      ఒకవేళ( flags==(SQLITE_SHM_తాళం|SQLITE_SHM_EXCLUSIVE) ){
        rc = sqlite3_mutex_try(p_Shm_కణుపు->aMutex[iMutex]);
        ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు leave_shmnode_mutexes;
      }లేకపోతే{
        sqlite3_mutex_enter(p_Shm_కణుపు->aMutex[iMutex]);
      }
    }
#లేకపోతే
    sqlite3_mutex_enter(p_Shm_కణుపు->pShmMutex);
#అయితే_ముగింపు

    ఒకవేళ( ALWAYS(rc==SQLITE_OK) ){
      ఒకవేళ( flags & SQLITE_SHM_UNLOCK ){
        /* Case (a) - unlock.  */
        పూర్ణాంకం bUnlock = 1;
        ప్రతిజ్ఞా( (p->excl_ముసుగు & p->shared_ముసుగు)==0 );
        ప్రతిజ్ఞా( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->excl_ముసుగు & ముసుగు)==ముసుగు );
        ప్రతిజ్ఞా( !(flags & SQLITE_SHM_SHARED) || (p->shared_ముసుగు & ముసుగు)==ముసుగు );
  
        /* If this is a SHARED lock being unlocked, it is possible that other
        ** clients within this process are holding the same SHARED lock. In
        ** this case, set bUnlock to 0 so that the posix lock is not removed
        ** from the file-descriptor below.  */
        ఒకవేళ( flags & SQLITE_SHM_SHARED ){
          ప్రతిజ్ఞా( n==1 );
          ప్రతిజ్ఞా( a_తాళం[ofst]>=1 );
          ఒకవేళ( a_తాళం[ofst]>1 ){
            bUnlock = 0;
            a_తాళం[ofst]--;
            p->shared_ముసుగు &= ~ముసుగు;
          }
        }
  
        ఒకవేళ( bUnlock ){
          rc = unix_Shm_System_తాళం(pDbFd, F_UNLCK, ofst+UNIX_SHM_పునాది, n);
          ఒకవేళ( rc==SQLITE_OK ){
            memset(&a_తాళం[ofst], 0, పరిమాణం(పూర్ణాంకం)*n);
            p->shared_ముసుగు &= ~ముసుగు;
            p->excl_ముసుగు &= ~ముసుగు;
          }
        }
      }లేకపోతే ఒకవేళ( flags & SQLITE_SHM_SHARED ){
        /* Case (b) - a shared lock.  */
  
        ఒకవేళ( a_తాళం[ofst]<0 ){
          /* An exclusive lock is held by some other connection. BUSY. */ 
          rc = SQLITE_BUSY;
        }లేకపోతే ఒకవేళ( a_తాళం[ofst]==0 ){
          rc = unix_Shm_System_తాళం(pDbFd, F_RDLCK, ofst+UNIX_SHM_పునాది, n);
        }
  
        /* Get the local shared locks */
        ఒకవేళ( rc==SQLITE_OK ){
          p->shared_ముసుగు |= ముసుగు;
          a_తాళం[ofst]++;
        }
      }లేకపోతే{
        /* Case (c) - an exclusive lock.  */
        పూర్ణాంకం ii;
  
        ప్రతిజ్ఞా( flags==(SQLITE_SHM_తాళం|SQLITE_SHM_EXCLUSIVE) );
        ప్రతిజ్ఞా( (p->shared_ముసుగు & ముసుగు)==0 );
        ప్రతిజ్ఞా( (p->excl_ముసుగు & ముసుగు)==0 );
  
        /* Make sure no sibling connections hold locks that will block this
        ** lock.  If any do, return SQLITE_BUSY right away.  */
        కొరకు(ii=ofst; ii<ofst+n; ii++){
          ఒకవేళ( a_తాళం[ii] ){
            rc = SQLITE_BUSY;
            ఆపు;
          }
        }
  
        /* Get the exclusive locks at the system level. Then if successful
        ** also update the in-memory values. */
        ఒకవేళ( rc==SQLITE_OK ){
          rc = unix_Shm_System_తాళం(pDbFd, F_WRLCK, ofst+UNIX_SHM_పునాది, n);
          ఒకవేళ( rc==SQLITE_OK ){
            p->excl_ముసుగు |= ముసుగు;
            కొరకు(ii=ofst; ii<ofst+n; ii++){
              a_తాళం[ii] = -1;
            }
          }
        }
      }
      ప్రతిజ్ఞా( assertLockingArrayOk(p_Shm_కణుపు) );
    }

    /* Drop the mutexes acquired above. */
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  leave_shmnode_mutexes:
    కొరకు(iMutex--; iMutex>=ofst; iMutex--){
      sqlite3_mutex_leave(p_Shm_కణుపు->aMutex[iMutex]);
    }
#లేకపోతే
    sqlite3_mutex_leave(p_Shm_కణుపు->pShmMutex);
#అయితే_ముగింపు
  }

  OSTRACE(("SHM-LOCK shmid-%d, pid-%d got %03x,%03x\n",
           p->id, osGetpid(0), p->shared_ముసుగు, p->excl_ముసుగు));
  తిరిగి rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
స్థిర శూన్య unixShmBarrier(
  sqlite3_file *fd                /* Database file holding the shared memory */
){
  వాడని_PARAMETER(fd);
  sqlite3_జ్ఞాపకం_Barrier();         /* compiler-defined memory barrier */
  ప్రతిజ్ఞా( fd->pMethods->x_తాళం==nolock_తాళం
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
స్థిర పూర్ణాంకం unixShmUnmap(
  sqlite3_file *fd,               /* The underlying database file */
  పూర్ణాంకం deleteFlag                  /* Delete shared-memory if true */
){
  unixShm *p;                     /* The connection to be closed */
  unix_Shm_కణుపు *p_Shm_కణుపు;          /* The underlying shared-memory file */
  unixShm **pp;                   /* For looping over sibling connections */
  unixFile *pDbFd;                /* The underlying database file */

  pDbFd = (unixFile*)fd;
  p = pDbFd->pShm;
  ఒకవేళ( p==0 ) తిరిగి SQLITE_OK;
  p_Shm_కణుపు = p->p_Shm_కణుపు;

  ప్రతిజ్ఞా( p_Shm_కణుపు==pDbFd->pInode->p_Shm_కణుపు );
  ప్రతిజ్ఞా( p_Shm_కణుపు->pInode==pDbFd->pInode );

  /* Remove connection p from the set of connections associated
  ** with pShmNode */
  sqlite3_mutex_enter(p_Shm_కణుపు->pShmMutex);
  కొరకు(pp=&p_Shm_కణుపు->p_మొదటి; (*pp)!=p; pp = &(*pp)->p_తదుపరి){}
  *pp = p->p_తదుపరి;

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  sqlite3_mutex_leave(p_Shm_కణుపు->pShmMutex);

  /* If pShmNode->nRef has reached 0, then close the underlying
  ** shared-memory file, too */
  ప్రతిజ్ఞా( unixFileMutexNotheld(pDbFd) );
  unixEnterMutex();
  ప్రతిజ్ఞా( p_Shm_కణుపు->nRef>0 );
  p_Shm_కణుపు->nRef--;
  ఒకవేళ( p_Shm_కణుపు->nRef==0 ){
    ఒకవేళ( deleteFlag && p_Shm_కణుపు->hShm>=0 ){
      osUnlink(p_Shm_కణుపు->zFilename);
    }
    unixShmPurge(pDbFd);
  }
  unixLeaveMutex();

  తిరిగి SQLITE_OK;
}


#లేకపోతే
# define unixShmMap     0
# define unix_Shm_తాళం    0
# define unixShmBarrier 0
# define unixShmUnmap   0
#అయితే_ముగింపు /* #ifndef SQLITE_OMIT_WAL */

#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
/*
** If it is currently memory mapped, unmap file pFd.
*/
స్థిర శూన్య unixUnmapfile(unixFile *pFd){
  ప్రతిజ్ఞా( pFd->n_Fetch_బహిర్గతం==0 );
  ఒకవేళ( pFd->pMapRegion ){
    osMunmap(pFd->pMapRegion, pFd->mmap_కొలత_Actual);
    pFd->pMapRegion = 0;
    pFd->mmap_కొలత = 0;
    pFd->mmap_కొలత_Actual = 0;
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
స్థిర శూన్య unixRemapfile(
  unixFile *pFd,                  /* File descriptor object */
  i64 nNew                        /* Required mapping size */
){
  స్థిరరాశి అక్షరం *zErr = "mmap";
  పూర్ణాంకం h = pFd->h;                      /* File descriptor open on db file */
  u8 *pOrig = (u8 *)pFd->pMapRegion;   /* Pointer to current file mapping */
  u8 *pNew = 0;                        /* Location of new mapping */
  పూర్ణాంకం flags = PROT_READ;               /* Flags to pass to mmap() */

  ప్రతిజ్ఞా( pFd->n_Fetch_బహిర్గతం==0 );
  ప్రతిజ్ఞా( nNew>pFd->mmap_కొలత );
  ప్రతిజ్ఞా( nNew<=pFd->mmap_కొలత_Max );
  ప్రతిజ్ఞా( nNew>0 );
  ప్రతిజ్ఞా( pFd->mmap_కొలత_Actual>=pFd->mmap_కొలత );
  ప్రతిజ్ఞా( MAP_FAILED!=0 );

#నిర్వచితమైతే SQLITE_MMAP_READWRITE
  ఒకవేళ( (pFd->ctrlFlags & UNIXFILE_RDONLY)==0 ) flags |= PROT_WRITE;
#అయితే_ముగింపు

  ఒకవేళ( pOrig ){
    i64 nReuse = pFd->mmap_కొలత & ~((పూర్ణాంకం)osGetpagesize()-1);

    /* Unmap any part of the current mapping that will not be reused */
    ఒకవేళ( nReuse<pFd->mmap_కొలత_Actual ){
      osMunmap(&pOrig[nReuse], pFd->mmap_కొలత_Actual - nReuse);
    }

    /* If nReuse is now zero, then the entire mapping was unmapped by the
    ** call above. In this case do not attempt to extend pOrig. */
    ఒకవేళ( nReuse>0 ){
#ఒకవేళ HAVE_MREMAP
      pNew = osMremap(pOrig, nReuse, nNew, MREMAP_MAYMOVE);
      zErr = "mremap";
#లేకపోతే
      pNew = osMmap(&pOrig[nReuse], nNew-nReuse, flags, MAP_SHARED, h, nReuse);
      ఒకవేళ( pNew!=MAP_FAILED ){
        ఒకవేళ( pNew!=&pOrig[nReuse] ){
          osMunmap(pNew, nNew - nReuse);
          pNew = 0;
        }లేకపోతే{
          pNew = pOrig;
        }
      }
#అయితే_ముగింపు

      /* The attempt to extend the existing mapping failed. Free it. */
      ఒకవేళ( pNew==MAP_FAILED || pNew==0 ){
        osMunmap(pOrig, nReuse);
      }
    }
  }

  /* If pNew is still NULL, try to create an entirely new mapping. */
  ఒకవేళ( pNew==0 ){
    pNew = osMmap(0, nNew, flags, MAP_SHARED, h, 0);
  }

  ఒకవేళ( pNew==MAP_FAILED ){
    pNew = 0;
    nNew = 0;
    unixLogError(SQLITE_OK, zErr, pFd->z_మార్గం);

    /* If the mmap() above failed, assume that all subsequent mmap() calls
    ** will probably fail too. Fall back to using xRead/xWrite exclusively
    ** in this case.  */
    pFd->mmap_కొలత_Max = 0;
  }
  pFd->pMapRegion = (శూన్య *)pNew;
  pFd->mmap_కొలత = pFd->mmap_కొలత_Actual = nNew;
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
స్థిర పూర్ణాంకం unixMapfile(unixFile *pFd, i64 nMap){
  ప్రతిజ్ఞా( nMap>=0 || pFd->n_Fetch_బహిర్గతం==0 );
  ప్రతిజ్ఞా( nMap>0 || (pFd->mmap_కొలత==0 && pFd->pMapRegion==0) );
  ఒకవేళ( pFd->n_Fetch_బహిర్గతం>0 ) తిరిగి SQLITE_OK;

  ఒకవేళ( nMap<0 ){
    నిర్మాణం stat statbuf;          /* Low-level file information */
    ఒకవేళ( osFstat(pFd->h, &statbuf) ){
      తిరిగి SQLITE_IOERR_FSTAT;
    }
    nMap = statbuf.st_కొలత;
  }
  ఒకవేళ( nMap>pFd->mmap_కొలత_Max ){
    nMap = pFd->mmap_కొలత_Max;
  }

  ప్రతిజ్ఞా( nMap>0 || (pFd->mmap_కొలత==0 && pFd->pMapRegion==0) );
  ఒకవేళ( nMap!=pFd->mmap_కొలత ){
    unixRemapfile(pFd, nMap);
  }

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
** release the reference by calling unixUnfetch().
*/
స్థిర పూర్ణాంకం unixFetch(sqlite3_file *fd, i64 iOff, పూర్ణాంకం nAmt, శూన్య **pp){
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  unixFile *pFd = (unixFile *)fd;   /* The underlying database file */
#అయితే_ముగింపు
  *pp = 0;

#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  ఒకవేళ( pFd->mmap_కొలత_Max>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    స్థిరరాశి పూర్ణాంకం n_Eof_బఫరు = 256;
    ఒకవేళ( pFd->pMapRegion==0 ){
      పూర్ణాంకం rc = unixMapfile(pFd, -1);
      ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
    }
    ఒకవేళ( pFd->mmap_కొలత >= (iOff+nAmt+n_Eof_బఫరు) ){
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->n_Fetch_బహిర్గతం++;
    }
  }
#అయితే_ముగింపు
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం unixUnfetch(sqlite3_file *fd, i64 iOff, శూన్య *p){
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  unixFile *pFd = (unixFile *)fd;   /* The underlying database file */
  వాడని_PARAMETER(iOff);

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  ప్రతిజ్ఞా( (p==0)==(pFd->n_Fetch_బహిర్గతం==0) );

  /* If p!=0, it must match the iOff value. */
  ప్రతిజ్ఞా( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  ఒకవేళ( p ){
    pFd->n_Fetch_బహిర్గతం--;
  }లేకపోతే{
    unixUnmapfile(pFd);
  }

  ప్రతిజ్ఞా( pFd->n_Fetch_బహిర్గతం>=0 );
#లేకపోతే
  వాడని_PARAMETER(fd);
  వాడని_PARAMETER(p);
  వాడని_PARAMETER(iOff);
#అయితే_ముగింపు
  తిరిగి SQLITE_OK;
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
#నిర్వచించు IOMETHODS(FINDER,పద్ధతి,సంస్కరణ,CLOSE,తాళం_2,UNLOCK,CKLOCK,SHMMAP)     \
స్థిర స్థిరరాశి sqlite3_io_methods పద్ధతి = {                                   \
   సంస్కరణ,                    /* iVersion */                                \
   CLOSE,                      /* xClose */                                  \
   unixRead,                   /* xRead */                                   \
   unixWrite,                  /* xWrite */                                  \
   unixTruncate,               /* xTruncate */                               \
   unixSync,                   /* xSync */                                   \
   unix_File_కొలత,               /* xFileSize */                               \
   తాళం_2,                       /* xLock */                                   \
   UNLOCK,                     /* xUnlock */                                 \
   CKLOCK,                     /* xCheckReservedLock */                      \
   unixFileControl,            /* xFileControl */                            \
   unix_Sector_కొలత,             /* xSectorSize */                             \
   unixDeviceCharacteristics,  /* xDeviceCapabilities */                     \
   SHMMAP,                     /* xShmMap */                                 \
   unix_Shm_తాళం,                /* xShmLock */                                \
   unixShmBarrier,             /* xShmBarrier */                             \
   unixShmUnmap,               /* xShmUnmap */                               \
   unixFetch,                  /* xFetch */                                  \
   unixUnfetch,                /* xUnfetch */                                \
};                                                                           \
స్థిర స్థిరరాశి sqlite3_io_methods *FINDER##Impl(స్థిరరాశి అక్షరం *z, unixFile *p){   \
  వాడని_PARAMETER(z); వాడని_PARAMETER(p);                                  \
  తిరిగి &పద్ధతి;                                                            \
}                                                                            \
స్థిర స్థిరరాశి sqlite3_io_methods *(*స్థిరరాశి FINDER)(స్థిరరాశి అక్షరం*,unixFile *p)    \
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
  unix_తాళం,                 /* xLock method */
  unixUnlock,               /* xUnlock method */
  unix_Check_Reserved_తాళం,    /* xCheckReservedLock method */
  unixShmMap                /* xShmMap method */
)
IOMETHODS(
  nolockIoFinder,           /* Finder function name */
  nolockIoMethods,          /* sqlite3_io_methods object name */
  3,                        /* shared memory and mmap are enabled */
  nolockClose,              /* xClose method */
  nolock_తాళం,               /* xLock method */
  nolockUnlock,             /* xUnlock method */
  nolock_Check_Reserved_తాళం,  /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
IOMETHODS(
  dotlockIoFinder,          /* Finder function name */
  dotlockIoMethods,         /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  dotlockClose,             /* xClose method */
  dotlock_తాళం,              /* xLock method */
  dotlockUnlock,            /* xUnlock method */
  dotlock_Check_Reserved_తాళం, /* xCheckReservedLock method */
  0                         /* xShmMap method */
)

#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE
IOMETHODS(
  flockIoFinder,            /* Finder function name */
  flockIoMethods,           /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  flockClose,               /* xClose method */
  flock_తాళం,                /* xLock method */
  flockUnlock,              /* xUnlock method */
  flock_Check_Reserved_తాళం,   /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#అయితే_ముగింపు

#ఒకవేళ OS_VXWORKS
IOMETHODS(
  semIoFinder,              /* Finder function name */
  semIoMethods,             /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  semXClose,                /* xClose method */
  semXLock,                 /* xLock method */
  semXUnlock,               /* xUnlock method */
  sem_XCheck_Reserved_తాళం,    /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#అయితే_ముగింపు

#ఒకవేళ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
IOMETHODS(
  afpIoFinder,              /* Finder function name */
  afpIoMethods,             /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  afpClose,                 /* xClose method */
  afp_తాళం,                  /* xLock method */
  afpUnlock,                /* xUnlock method */
  afp_Check_Reserved_తాళం,     /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#అయితే_ముగింపు

/*
** The proxy locking method is a "super-method" in the sense that it
** opens secondary file descriptors for the conch and lock files and
** it uses proxy, dot-file, AFP, and flock() locking methods on those
** secondary files.  For this reason, the division that implements
** proxy locking is located much further down in the file.  But we need
** to go ahead and define the sqlite3_io_methods and finder function
** for proxy locking here.  So we forward declare the I/O methods.
*/
#ఒకవేళ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
స్థిర పూర్ణాంకం proxyClose(sqlite3_file*);
స్థిర పూర్ణాంకం proxy_తాళం(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం proxyUnlock(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం proxy_Check_Reserved_తాళం(sqlite3_file*, పూర్ణాంకం*);
IOMETHODS(
  proxyIoFinder,            /* Finder function name */
  proxyIoMethods,           /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  proxyClose,               /* xClose method */
  proxy_తాళం,                /* xLock method */
  proxyUnlock,              /* xUnlock method */
  proxy_Check_Reserved_తాళం,   /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#అయితే_ముగింపు

/* nfs lockd on OSX 10.3+ doesn't clear write locks when a read lock is set */
#ఒకవేళ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
IOMETHODS(
  nfsIoFinder,               /* Finder function name */
  nfsIoMethods,              /* sqlite3_io_methods object name */
  1,                         /* shared memory is disabled */
  unixClose,                 /* xClose method */
  unix_తాళం,                  /* xLock method */
  nfsUnlock,                 /* xUnlock method */
  unix_Check_Reserved_తాళం,     /* xCheckReservedLock method */
  0                          /* xShmMap method */
)
#అయితే_ముగింపు

#ఒకవేళ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
** This "finder" function attempts to determine the best locking strategy
** for the database file "filePath".  It then returns the sqlite3_io_methods
** object that implements that strategy.
**
** This is for MacOSX only.
*/
స్థిర స్థిరరాశి sqlite3_io_methods *autolockIoFinderImpl(
  స్థిరరాశి అక్షరం *file_మార్గం,    /* name of the database file */
  unixFile *pNew           /* open file object for the database file */
){
  స్థిర స్థిరరాశి నిర్మాణం Mapping {
    స్థిరరాశి అక్షరం *zFilesystem;              /* Filesystem type name */
    స్థిరరాశి sqlite3_io_methods *pMethods;   /* Appropriate locking method */
  } aMap[] = {
    { "hfs",    &posixIoMethods },
    { "ufs",    &posixIoMethods },
    { "afpfs",  &afpIoMethods },
    { "smbfs",  &afpIoMethods },
    { "webdav", &nolockIoMethods },
    { 0, 0 }
  };
  పూర్ణాంకం i;
  నిర్మాణం statfs fsInfo;
  నిర్మాణం flock తాళం_Info;

  ఒకవేళ( !file_మార్గం ){
    /* If filePath==NULL that means we are dealing with a transient file
    ** that does not need to be locked. */
    తిరిగి &nolockIoMethods;
  }
  ఒకవేళ( statfs(file_మార్గం, &fsInfo) != -1 ){
    ఒకవేళ( fsInfo.f_flags & MNT_RDONLY ){
      తిరిగి &nolockIoMethods;
    }
    కొరకు(i=0; aMap[i].zFilesystem; i++){
      ఒకవేళ( strcmp(fsInfo.f_fstypename, aMap[i].zFilesystem)==0 ){
        తిరిగి aMap[i].pMethods;
      }
    }
  }

  /* Default case. Handles, amongst others, "nfs".
  ** Test byte-range lock using fcntl(). If the call succeeds,
  ** assume that the file-system supports POSIX style locks.
  */
  తాళం_Info.l_len = 1;
  తాళం_Info.l_ప్రారంభం = 0;
  తాళం_Info.l_whence = SEEK_SET;
  తాళం_Info.l_type = F_RDLCK;
  ఒకవేళ( osFcntl(pNew->h, F_GETLK, &తాళం_Info)!=-1 ) {
    ఒకవేళ( strcmp(fsInfo.f_fstypename, "nfs")==0 ){
      తిరిగి &nfsIoMethods;
    } లేకపోతే {
      తిరిగి &posixIoMethods;
    }
  }లేకపోతే{
    తిరిగి &dotlockIoMethods;
  }
}
స్థిర స్థిరరాశి sqlite3_io_methods
  *(*స్థిరరాశి autolockIoFinder)(స్థిరరాశి అక్షరం*,unixFile*) = autolockIoFinderImpl;

#అయితే_ముగింపు /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */

#ఒకవేళ OS_VXWORKS
/*
** This "finder" function for VxWorks checks to see if posix advisory
** locking works.  If it does, then that is what is used.  If it does not
** work, then fallback to named semaphore locking.
*/
స్థిర స్థిరరాశి sqlite3_io_methods *vxworksIoFinderImpl(
  స్థిరరాశి అక్షరం *file_మార్గం,    /* name of the database file */
  unixFile *pNew           /* the open file object */
){
  నిర్మాణం flock తాళం_Info;

  ఒకవేళ( !file_మార్గం ){
    /* If filePath==NULL that means we are dealing with a transient file
    ** that does not need to be locked. */
    తిరిగి &nolockIoMethods;
  }

  /* Test if fcntl() is supported and use POSIX style locks.
  ** Otherwise fall back to the named semaphore method.
  */
  తాళం_Info.l_len = 1;
  తాళం_Info.l_ప్రారంభం = 0;
  తాళం_Info.l_whence = SEEK_SET;
  తాళం_Info.l_type = F_RDLCK;
  ఒకవేళ( osFcntl(pNew->h, F_GETLK, &తాళం_Info)!=-1 ) {
    తిరిగి &posixIoMethods;
  }లేకపోతే{
    తిరిగి &semIoMethods;
  }
}
స్థిర స్థిరరాశి sqlite3_io_methods
  *(*స్థిరరాశి vxworksIoFinder)(స్థిరరాశి అక్షరం*,unixFile*) = vxworksIoFinderImpl;

#అయితే_ముగింపు /* OS_VXWORKS */

/*
** An abstract type for a pointer to an IO method finder function:
*/
రకనిర్వచనం స్థిరరాశి sqlite3_io_methods *(*finder_type)(స్థిరరాశి అక్షరం*,unixFile*);


/****************************************************************************
**************************** sqlite3_vfs methods ****************************
**
** This division contains the implementation of methods on the
** sqlite3_vfs object.
*/

/*
** Initialize the contents of the unixFile structure pointed to by pId.
*/
స్థిర పూర్ణాంకం fill_లోపల_Unix_File(
  sqlite3_vfs *pVfs,      /* Pointer to vfs object */
  పూర్ణాంకం h,                  /* Open file descriptor of file being opened */
  sqlite3_file *pId,      /* Write to the unixFile structure here */
  స్థిరరాశి అక్షరం *zFilename,  /* Name of the file being opened */
  పూర్ణాంకం ctrlFlags           /* Zero or more UNIXFILE_* values */
){
  స్థిరరాశి sqlite3_io_methods *pLockingStyle;
  unixFile *pNew = (unixFile *)pId;
  పూర్ణాంకం rc = SQLITE_OK;

  ప్రతిజ్ఞా( pNew->pInode==ఖాళీ );

  /* No locking occurs in temporary files */
  ప్రతిజ్ఞా( zFilename!=0 || (ctrlFlags & UNIXFILE_NOLOCK)!=0 );

  OSTRACE(("OPEN    %-3d %s\n", h, zFilename));
  pNew->h = h;
  pNew->pVfs = pVfs;
  pNew->z_మార్గం = zFilename;
  pNew->ctrlFlags = (u8)ctrlFlags;
#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
  pNew->mmap_కొలత_Max = sqlite3GlobalConfig.szMmap;
#అయితే_ముగింపు
  ఒకవేళ( sqlite3_uri_boolean(((ctrlFlags & UNIXFILE_URI) ? zFilename : 0),
                           "psow", SQLITE_POWERSAFE_OVERWRITE) ){
    pNew->ctrlFlags |= UNIXFILE_PSOW;
  }
  ఒకవేళ( strcmp(pVfs->zName,"unix-excl")==0 ){
    pNew->ctrlFlags |= UNIXFILE_EXCL;
  }

#ఒకవేళ OS_VXWORKS
  pNew->pId = vxworks_కనుగొను_File_Id(zFilename);
  ఒకవేళ( pNew->pId==0 ){
    ctrlFlags |= UNIXFILE_NOLOCK;
    rc = SQLITE_NOMEM_BKPT;
  }
#అయితే_ముగింపు

  ఒకవేళ( ctrlFlags & UNIXFILE_NOLOCK ){
    pLockingStyle = &nolockIoMethods;
  }లేకపోతే{
    pLockingStyle = (**(finder_type*)pVfs->p_App_దత్తాంశం)(zFilename, pNew);
#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE
    /* Cache zFilename in the locking context (AFP and dotlock override) for
    ** proxyLock activation is possible (remote proxy is based on db name)
    ** zFilename remains valid until file is closed, to support */
    pNew->lockingContext = (శూన్య*)zFilename;
#అయితే_ముగింపు
  }

  ఒకవేళ( pLockingStyle == &posixIoMethods
#ఒకవేళ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
    || pLockingStyle == &nfsIoMethods
#అయితే_ముగింపు
  ){
    unixEnterMutex();
    rc = కనుగొను_Inode_Info(pNew, &pNew->pInode);
    ఒకవేళ( rc!=SQLITE_OK ){
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

#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
  లేకపోతే ఒకవేళ( pLockingStyle == &afpIoMethods ){
    /* AFP locking uses the file path so it needs to be included in
    ** the afpLockingContext.
    */
    afpLockingContext *pCtx;
    pNew->lockingContext = pCtx = sqlite3_malloc64( పరిమాణం(*pCtx) );
    ఒకవేళ( pCtx==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }లేకపోతే{
      /* NB: zFilename exists and remains valid until the file is closed
      ** according to requirement F11141.  So we do not need to make a
      ** copy of the filename. */
      pCtx->db_మార్గం = zFilename;
      pCtx->reserved = 0;
      srandomdev();
      unixEnterMutex();
      rc = కనుగొను_Inode_Info(pNew, &pNew->pInode);
      ఒకవేళ( rc!=SQLITE_OK ){
        sqlite3_free(pNew->lockingContext);
        robust_close(pNew, h, __LINE__);
        h = -1;
      }
      unixLeaveMutex();      
    }
  }
#అయితే_ముగింపు

  లేకపోతే ఒకవేళ( pLockingStyle == &dotlockIoMethods ){
    /* Dotfile locking uses the file path so it needs to be included in
    ** the dotlockLockingContext
    */
    అక్షరం *z_తాళం_File;
    పూర్ణాంకం nFilename;
    ప్రతిజ్ఞా( zFilename!=0 );
    nFilename = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zFilename) + 6;
    z_తాళం_File = (అక్షరం *)sqlite3_malloc64(nFilename);
    ఒకవేళ( z_తాళం_File==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }లేకపోతే{
      sqlite3_snprintf(nFilename, z_తాళం_File, "%s" DOTLOCK_SUFFIX, zFilename);
    }
    pNew->lockingContext = z_తాళం_File;
  }

#ఒకవేళ OS_VXWORKS
  లేకపోతే ఒకవేళ( pLockingStyle == &semIoMethods ){
    /* Named semaphore locking uses the file path so it needs to be
    ** included in the semLockingContext
    */
    unixEnterMutex();
    rc = కనుగొను_Inode_Info(pNew, &pNew->pInode);
    ఒకవేళ( (rc==SQLITE_OK) && (pNew->pInode->pSem==ఖాళీ) ){
      అక్షరం *zSemName = pNew->pInode->aSemName;
      పూర్ణాంకం n;
      sqlite3_snprintf(MAX_PATHNAME, zSemName, "/%s.sem",
                       pNew->pId->zCanonicalName);
      కొరకు( n=1; zSemName[n]; n++ )
        ఒకవేళ( zSemName[n]=='/' ) zSemName[n] = '_';
      pNew->pInode->pSem = sem_open(zSemName, O_CREAT, 0666, 1);
      ఒకవేళ( pNew->pInode->pSem == SEM_FAILED ){
        rc = SQLITE_NOMEM_BKPT;
        pNew->pInode->aSemName[0] = '\0';
      }
    }
    unixLeaveMutex();
  }
#అయితే_ముగింపు

  store_చివరి_Errno(pNew, 0);
#ఒకవేళ OS_VXWORKS
  ఒకవేళ( rc!=SQLITE_OK ){
    ఒకవేళ( h>=0 ){
      robust_close(pNew, h, __LINE__);
      h = -1;
    }
    ఒకవేళ( pNew->ctrlFlags & UNIXFILE_DELETE ){
      osUnlink(zFilename);
    }
    ఒకవేళ( pNew->pId ){
      vxworksReleaseFileId(pNew->pId);
      pNew->pId = 0;
    }
  }
#అయితే_ముగింపు
  ఒకవేళ( rc!=SQLITE_OK ){
    ఒకవేళ( h>=0 ) robust_close(pNew, h, __LINE__);
  }లేకపోతే{
    pId->pMethods = pLockingStyle;
    OpenCounter(+1);
    verifyDbFile(pNew);
  }
  తిరిగి rc;
}

/*
** Return the name of a directory in which to put temporary files.
** If no suitable temporary file directory can be found, return NULL.
**
** The return value might be a string obtained from getenv() and so
** the return value should not be used after any call to setenv() or
** putenv() as that value might have been freed.
*/
స్థిర స్థిరరాశి అక్షరం *unix_తాత్కాలిక_File_Dir(శూన్య){
  గుర్తులేని పూర్ణాంకం i;
  నిర్మాణం stat buf;
  స్థిరరాశి అక్షరం *zDir;

  కొరకు(i=0; i<7; i++){
    ఎంపిక( i ){
      స్థితి 0:  zDir = sqlite3_తాత్కాలిక_directory;   ఆపు;
      స్థితి 1:  zDir = getenv("SQLITE_TMPDIR");  ఆపు;
      స్థితి 2:  zDir = getenv("TMPDIR");         ఆపు;
      స్థితి 3:  zDir = "/var/tmp";               ఆపు;
      స్థితి 4:  zDir = "/usr/tmp";               ఆపు;
      స్థితి 5:  zDir = "/tmp";                   ఆపు;
      అప్రమేయ: zDir = ".";                      ఆపు;
    }
    ఒకవేళ( zDir!=0
#ఒకవేళ OS_VXWORKS
     && zDir[0]=='/'
#అయితే_ముగింపు
     && osStat(zDir, &buf)==0
     && S_ISDIR(buf.st_బహుళకం)
     && osAccess(zDir, 03)==0
    ){
      తిరిగి zDir;
    }
  }
  తిరిగి 0;
}

/*
** Create a temporary file name in zBuf.  zBuf must be allocated
** by the calling process and must be big enough to hold at least
** pVfs->mxPathname bytes.
*/
స్థిర పూర్ణాంకం unixGetTempname(పూర్ణాంకం nBuf, అక్షరం *zBuf){
  స్థిరరాశి అక్షరం *zDir;
  పూర్ణాంకం i_హద్దు = 0;
  పూర్ణాంకం rc = SQLITE_OK;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  zBuf[0] = 0;
  SimulateIOError( తిరిగి SQLITE_IOERR );

  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  zDir = unix_తాత్కాలిక_File_Dir();
  ఒకవేళ( zDir==0 ){
    rc = SQLITE_IOERR_GETTEMPPATH;
  }లేకపోతే{
    చేయి{
      u64 r;
      sqlite3_randomness(పరిమాణం(r), &r);
      ప్రతిజ్ఞా( nBuf>2 );
      zBuf[nBuf-2] = 0;
      sqlite3_snprintf(nBuf, zBuf, "%s/"SQLITE_తాత్కాలిక_FILE_ఉపసర్గ"%llx%c",
                       zDir, r, 0);
      ఒకవేళ( zBuf[nBuf-2]!=0 || (i_హద్దు++)>10 ){
        rc = SQLITE_ERROR;
        ఆపు;
      }
    }అయితే( osAccess(zBuf,0)==0 );
  }
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  తిరిగి rc;
}

#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
/*
** Routine to transform a unixFile into a proxy-locking unixFile.
** Implementation in the proxy-lock division, but used by unixOpen()
** if SQLITE_PREFER_PROXY_LOCKING is defined.
*/
స్థిర పూర్ణాంకం proxyTransformUnixFile(unixFile*, స్థిరరాశి అక్షరం*);
#అయితే_ముగింపు

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
స్థిర Unix_వాడని_Fd *కనుగొను_Reusable_Fd(స్థిరరాశి అక్షరం *z_మార్గం, పూర్ణాంకం flags){
  Unix_వాడని_Fd *p_వాడని = 0;

  /* Do not search for an unused file descriptor on vxworks. Not because
  ** vxworks would not benefit from the change (it might, we're not sure),
  ** but because no way to test it is currently available. It is better
  ** not to risk breaking vxworks support for the sake of such an obscure
  ** feature.  */
#ఒకవేళ !OS_VXWORKS
  నిర్మాణం stat sStat;                   /* Results of stat() call */

  unixEnterMutex();

  /* A stat() call may fail for various reasons. If this happens, it is
  ** almost certain that an open() call on the same path will also fail.
  ** For this reason, if an error occurs in the stat() call here, it is
  ** ignored and -1 is returned. The caller will try to open a new file
  ** descriptor on the same path, fail, and return an error to SQLite.
  **
  ** Even if a subsequent open() call does succeed, the consequences of
  ** not searching for a reusable file descriptor are not dire.  */
  ఒకవేళ( inodeList!=0 && 0==osStat(z_మార్గం, &sStat) ){
    unixInodeInfo *pInode;

    pInode = inodeList;
    అయితే( pInode && (pInode->fileId.dev!=sStat.st_dev
                     || pInode->fileId.ino!=(u64)sStat.st_ino) ){
       pInode = pInode->p_తదుపరి;
    }
    ఒకవేళ( pInode ){
      Unix_వాడని_Fd **pp;
      ప్రతిజ్ఞా( sqlite3_mutex_notheld(pInode->p_తాళం_Mutex) );
      sqlite3_mutex_enter(pInode->p_తాళం_Mutex);
      flags &= (SQLITE_OPEN_READONLY|SQLITE_OPEN_READWRITE);
      కొరకు(pp=&pInode->p_వాడని; *pp && (*pp)->flags!=flags; pp=&((*pp)->p_తదుపరి));
      p_వాడని = *pp;
      ఒకవేళ( p_వాడని ){
        *pp = p_వాడని->p_తదుపరి;
      }
      sqlite3_mutex_leave(pInode->p_తాళం_Mutex);
    }
  }
  unixLeaveMutex();
#అయితే_ముగింపు    /* if !OS_VXWORKS */
  తిరిగి p_వాడని;
}

/*
** Find the mode, uid and gid of file zFile.
*/
స్థిర పూర్ణాంకం get_File_బహుళకం(
  స్థిరరాశి అక్షరం *zFile,              /* File name */
  బహుళకం_t *p_బహుళకం,                  /* OUT: Permissions of zFile */
  uid_t *pUid,                    /* OUT: uid of zFile. */
  gid_t *pGid                     /* OUT: gid of zFile. */
){
  నిర్మాణం stat sStat;              /* Output of stat() on database file */
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( 0==osStat(zFile, &sStat) ){
    *p_బహుళకం = sStat.st_బహుళకం & 0777;
    *pUid = sStat.st_uid;
    *pGid = sStat.st_gid;
  }లేకపోతే{
    rc = SQLITE_IOERR_FSTAT;
  }
  తిరిగి rc;
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
స్థిర పూర్ణాంకం కనుగొను_Create_File_బహుళకం(
  స్థిరరాశి అక్షరం *z_మార్గం,              /* Path of file (possibly) being created */
  పూర్ణాంకం flags,                      /* Flags passed as 4th argument to xOpen() */
  బహుళకం_t *p_బహుళకం,                  /* OUT: Permissions to open file with */
  uid_t *pUid,                    /* OUT: uid to set on the file */
  gid_t *pGid                     /* OUT: gid to set on the file */
){
  పూర్ణాంకం rc = SQLITE_OK;             /* Return Code */
  *p_బహుళకం = 0;
  *pUid = 0;
  *pGid = 0;
  ఒకవేళ( flags & (SQLITE_OPEN_WAL|SQLITE_OPEN_MAIN_JOURNAL) ){
    అక్షరం zDb[MAX_PATHNAME+1];     /* Database file path */
    పూర్ణాంకం nDb;                      /* Number of valid bytes in zDb */

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
    nDb = sqlite3Strlen30(z_మార్గం) - 1;
    అయితే( nDb>0 && z_మార్గం[nDb]!='.' ){
      ఒకవేళ( z_మార్గం[nDb]=='-' ){
        జ్ఞాపకం_నకలు(zDb, z_మార్గం, nDb);
        zDb[nDb] = '\0';
        rc = get_File_బహుళకం(zDb, p_బహుళకం, pUid, pGid);
        ఆపు;
      }
      nDb--;
    }
  }లేకపోతే ఒకవేళ( flags & SQLITE_OPEN_DELETEONCLOSE ){
    *p_బహుళకం = 0600;
  }లేకపోతే ఒకవేళ( flags & SQLITE_OPEN_URI ){
    /* If this is a main database file and the file was opened using a URI
    ** filename, check for the "modeof" parameter. If present, interpret
    ** its value as a filename and try to copy the mode, uid and gid from
    ** that file.  */
    స్థిరరాశి అక్షరం *z = sqlite3_uri_parameter(z_మార్గం, "modeof");
    ఒకవేళ( z ){
      rc = get_File_బహుళకం(z, p_బహుళకం, pUid, pGid);
    }
  }
  తిరిగి rc;
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
స్థిర పూర్ణాంకం unixOpen(
  sqlite3_vfs *pVfs,           /* The VFS for which this is the xOpen method */
  స్థిరరాశి అక్షరం *z_మార్గం,           /* Pathname of file to be opened */
  sqlite3_file *pFile,         /* The file descriptor to be filled in */
  పూర్ణాంకం flags,                   /* Input flags to control the opening */
  పూర్ణాంకం *p_బహిర్గతం_Flags               /* Output flags returned to SQLite core */
){
  unixFile *p = (unixFile *)pFile;
  పూర్ణాంకం fd = -1;                   /* File descriptor returned by open() */
  పూర్ణాంకం openFlags = 0;             /* Flags to pass to open() */
  పూర్ణాంకం eType = flags&0x0FFF00;  /* Type of file to open */
  పూర్ణాంకం no_తాళం;                    /* True to omit locking primitives */
  పూర్ణాంకం rc = SQLITE_OK;            /* Function Return Code */
  పూర్ణాంకం ctrlFlags = 0;             /* UNIXFILE_* flags */

  పూర్ణాంకం isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  పూర్ణాంకం isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  పూర్ణాంకం isCreate     = (flags & SQLITE_OPEN_CREATE);
  పూర్ణాంకం isReadonly   = (flags & SQLITE_OPEN_READONLY);
  పూర్ణాంకం isReadWrite  = (flags & SQLITE_OPEN_READWRITE);
#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE
  పూర్ణాంకం isAutoProxy  = (flags & SQLITE_OPEN_AUTOPROXY);
#అయితే_ముగింపు
#ఒకవేళ defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
  నిర్మాణం statfs fsInfo;
#అయితే_ముగింపు

  /* If creating a super- or main-file journal, this function will open
  ** a file-descriptor on the directory too. The first time unixSync()
  ** is called the directory file descriptor will be fsync()ed and close()d.
  */
  పూర్ణాంకం isNewJrnl = (isCreate && (
        eType==SQLITE_OPEN_SUPER_JOURNAL
     || eType==SQLITE_OPEN_MAIN_JOURNAL
     || eType==SQLITE_OPEN_WAL
  ));

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  అక్షరం zTmpname[MAX_PATHNAME+2];
  స్థిరరాశి అక్షరం *zName = z_మార్గం;

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

#ఒకవేళ OS_VXWORKS
  /* The file-ID mechanism used in Vxworks requires that all pathnames
  ** provided to unixOpen must be absolute pathnames. */
  ఒకవేళ( z_మార్గం!=0 && z_మార్గం[0]!='/' ){ తిరిగి SQLITE_CANTOPEN; }
#అయితే_ముగింపు

  /* Detect a pid change and reset the PRNG.  There is a race condition
  ** here such that two or more threads all trying to open databases at
  ** the same instant might all reset the PRNG.  But multiple resets
  ** are harmless.
  */
  ఒకవేళ( randomnessPid!=osGetpid(0) ){
    randomnessPid = osGetpid(0);
    sqlite3_randomness(0,0);
  }
  memset(p, 0, పరిమాణం(unixFile));

#నిర్వచితమైతే SQLITE_ASSERT_NO_FILES
  /* Applications that never read or write a persistent disk files */
  ప్రతిజ్ఞా( zName==0 );
#అయితే_ముగింపు

  ఒకవేళ( eType==SQLITE_OPEN_MAIN_DB ){
    Unix_వాడని_Fd *p_వాడని;
    p_వాడని = కనుగొను_Reusable_Fd(zName, flags);
    ఒకవేళ( p_వాడని ){
      fd = p_వాడని->fd;
    }లేకపోతే{
      p_వాడని = sqlite3_malloc64(పరిమాణం(*p_వాడని));
      ఒకవేళ( !p_వాడని ){
        తిరిగి SQLITE_NOMEM_BKPT;
      }
    }
    p->p_Preallocated_వాడని = p_వాడని;

    /* Database filenames are double-zero terminated if they are not
    ** URIs with parameters.  Hence, they can always be passed into
    ** sqlite3_uri_parameter(). */
    ప్రతిజ్ఞా( (flags & SQLITE_OPEN_URI) || zName[అక్షరశ్రేణి_పొడవు(zName)+1]==0 );

  }లేకపోతే ఒకవేళ( !zName ){
    /* If zName is NULL, the upper layer is requesting a temp file. */
    ప్రతిజ్ఞా( isDelete );
    ప్రతిజ్ఞా( !isNewJrnl );
    ప్రతిజ్ఞా( isExclusive );
    ప్రతిజ్ఞా( isReadWrite );
#ఒకవేళ defined(__linux__) && defined(O_TMPFILE)
    /* On systems that support O_TMPFILE, use that flag to create a more
    ** secure temporary file that cannot be accessed by other processes
    */
    zName = unix_తాత్కాలిక_File_Dir();
    ఒకవేళ( zName
     && (fd = robust_open(zName, O_RDWR|O_EXCL|O_TMPFILE, 0600))>=0
    ){
      rc = fill_లోపల_Unix_File(pVfs, fd, pFile, z_మార్గం, ctrlFlags|UNIXFILE_NOLOCK);
      వెళ్ళు open_ముగిసిన;
    }
#అయితే_ముగింపు
    rc = unixGetTempname(pVfs->mxPathname, zTmpname);
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }
    zName = zTmpname;

    /* Generated temporary filenames are always double-zero terminated
    ** for use by sqlite3_uri_parameter(). */
    ప్రతిజ్ఞా( zName[అక్షరశ్రేణి_పొడవు(zName)+1]==0 );
  }

  /* Determine the value of the flags parameter passed to POSIX function
  ** open(). These must be calculated even if open() is not called, as
  ** they may be stored as part of the file handle and used by the
  ** 'conch file' locking functions later on.  */
  ఒకవేళ( isReadonly )  openFlags |= O_RDONLY;
  ఒకవేళ( isReadWrite ) openFlags |= O_RDWR;
  ఒకవేళ( isCreate )    openFlags |= O_CREAT;
  ఒకవేళ( isExclusive ) openFlags |= (O_EXCL|O_NOFOLLOW);
  openFlags |= (O_LARGEFILE|O_ద్విమాన|O_NOFOLLOW);

  ఒకవేళ( fd<0 ){
    బహుళకం_t open_బహుళకం;              /* Permissions to create file with */
    uid_t uid;                    /* Userid for the file */
    gid_t gid;                    /* Groupid for the file */
    rc = కనుగొను_Create_File_బహుళకం(zName, flags, &open_బహుళకం, &uid, &gid);
    ఒకవేళ( rc!=SQLITE_OK ){
      ప్రతిజ్ఞా( !p->p_Preallocated_వాడని );
      ప్రతిజ్ఞా( eType==SQLITE_OPEN_WAL || eType==SQLITE_OPEN_MAIN_JOURNAL );
      తిరిగి rc;
    }
    fd = robust_open(zName, openFlags, open_బహుళకం);
    OSTRACE(("OPENX   %-3d %s 0%o\n", fd, zName, openFlags));
    ప్రతిజ్ఞా( !isExclusive || (openFlags & O_CREAT)!=0 );
    ఒకవేళ( fd<0 ){
      ఒకవేళ( isNewJrnl && errno==EACCES && osAccess(zName, F_OK) ){
        /* If unable to create a journal because the directory is not
        ** writable, change the error code to indicate that. */
        rc = SQLITE_READONLY_DIRECTORY;
      }లేకపోతే ఒకవేళ( errno!=EISDIR && isReadWrite ){
        /* Failed to open the file for read/write access. Try read-only. */
        Unix_వాడని_Fd *pReadonly = 0;
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        openFlags &= ~(O_RDWR|O_CREAT);
        flags |= SQLITE_OPEN_READONLY;
        openFlags |= O_RDONLY;
        isReadonly = 1;
        pReadonly = కనుగొను_Reusable_Fd(zName, flags);
        ఒకవేళ( pReadonly ){
          fd = pReadonly->fd;
          sqlite3_free(pReadonly);
        }లేకపోతే{
          fd = robust_open(zName, openFlags, open_బహుళకం);
        }
      }
    }
    ఒకవేళ( fd<0 ){
      పూర్ణాంకం rc2 = unixLogError(SQLITE_CANTOPEN_BKPT, "open", zName);
      ఒకవేళ( rc==SQLITE_OK ) rc = rc2;
      వెళ్ళు open_ముగిసిన;
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
    ఒకవేళ( open_బహుళకం && (flags & (SQLITE_OPEN_WAL|SQLITE_OPEN_MAIN_JOURNAL))!=0 ){
      robustFchown(fd, uid, gid);
    }
  }
  ప్రతిజ్ఞా( fd>=0 );
  ఒకవేళ( p_బహిర్గతం_Flags ){
    *p_బహిర్గతం_Flags = flags;
  }

  ఒకవేళ( p->p_Preallocated_వాడని ){
    p->p_Preallocated_వాడని->fd = fd;
    p->p_Preallocated_వాడని->flags =
                          flags & (SQLITE_OPEN_READONLY|SQLITE_OPEN_READWRITE);
  }

  ఒకవేళ( isDelete ){
#ఒకవేళ OS_VXWORKS
    z_మార్గం = zName;
#లేదావేళ defined(SQLITE_UNLINK_AFTER_CLOSE)
    z_మార్గం = sqlite3_mprintf("%s", zName);
    ఒకవేళ( z_మార్గం==0 ){
      robust_close(p, fd, __LINE__);
      తిరిగి SQLITE_NOMEM_BKPT;
    }
#లేకపోతే
    osUnlink(zName);
#అయితే_ముగింపు
  }
#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE
  లేకపోతే{
    p->openFlags = openFlags;
  }
#అయితే_ముగింపు

#ఒకవేళ defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
  ఒకవేళ( fstatfs(fd, &fsInfo) == -1 ){
    store_చివరి_Errno(p, errno);
    robust_close(p, fd, __LINE__);
    తిరిగి SQLITE_IOERR_ACCESS;
  }
  ఒకవేళ (0 == అక్షరశ్రేణి_పోలిక("msdos", fsInfo.f_fstypename, 5)) {
    ((unixFile*)pFile)->fsFlags |= SQLITE_FSFLAGS_IS_MSDOS;
  }
  ఒకవేళ (0 == అక్షరశ్రేణి_పోలిక("exfat", fsInfo.f_fstypename, 5)) {
    ((unixFile*)pFile)->fsFlags |= SQLITE_FSFLAGS_IS_MSDOS;
  }
#అయితే_ముగింపు

  /* Set up appropriate ctrlFlags */
  ఒకవేళ( isDelete )                ctrlFlags |= UNIXFILE_DELETE;
  ఒకవేళ( isReadonly )              ctrlFlags |= UNIXFILE_RDONLY;
  no_తాళం = eType!=SQLITE_OPEN_MAIN_DB;
  ఒకవేళ( no_తాళం )                  ctrlFlags |= UNIXFILE_NOLOCK;
  ఒకవేళ( isNewJrnl )               ctrlFlags |= UNIXFILE_DIRSYNC;
  ఒకవేళ( flags & SQLITE_OPEN_URI ) ctrlFlags |= UNIXFILE_URI;

#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE
#ఒకవేళ SQLITE_PREFER_PROXY_LOCKING
  isAutoProxy = 1;
#అయితే_ముగింపు
  ఒకవేళ( isAutoProxy && (z_మార్గం!=ఖాళీ) && (!no_తాళం) && pVfs->xOpen ){
    అక్షరం *envforce = getenv("SQLITE_FORCE_PROXY_LOCKING");
    పూర్ణాంకం useProxy = 0;

    /* SQLITE_FORCE_PROXY_LOCKING==1 means force always use proxy, 0 means
    ** never use proxy, NULL means use proxy for non-local files only.  */
    ఒకవేళ( envforce!=ఖాళీ ){
      useProxy = atoi(envforce)>0;
    }లేకపోతే{
      useProxy = !(fsInfo.f_flags&MNT_LOCAL);
    }
    ఒకవేళ( useProxy ){
      rc = fill_లోపల_Unix_File(pVfs, fd, pFile, z_మార్గం, ctrlFlags);
      ఒకవేళ( rc==SQLITE_OK ){
        rc = proxyTransformUnixFile((unixFile*)pFile, ":auto:");
        ఒకవేళ( rc!=SQLITE_OK ){
          /* Use unixClose to clean up the resources added in fillInUnixFile
          ** and clear all the structure's references.  Specifically,
          ** pFile->pMethods will be NULL so sqlite3OsClose will be a no-op
          */
          unixClose(pFile);
          తిరిగి rc;
        }
      }
      వెళ్ళు open_ముగిసిన;
    }
  }
#అయితే_ముగింపు

  ప్రతిజ్ఞా( z_మార్గం==0
       || z_మార్గం[0]=='/'
       || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_MAIN_JOURNAL
       || eType==SQLITE_OPEN_తాత్కాలిక_JOURNAL
  );
  rc = fill_లోపల_Unix_File(pVfs, fd, pFile, z_మార్గం, ctrlFlags);

open_ముగిసిన:
  ఒకవేళ( rc!=SQLITE_OK ){
    sqlite3_free(p->p_Preallocated_వాడని);
  }
  తిరిగి rc;
}


/*
** Delete the file at zPath. If the dirSync argument is true, fsync()
** the directory after deleting the file.
*/
స్థిర పూర్ణాంకం unixDelete(
  sqlite3_vfs *Not_వాడిన,     /* VFS containing this as the xDelete method */
  స్థిరరాశి అక్షరం *z_మార్గం,        /* Name of file to be deleted */
  పూర్ణాంకం dirSync               /* If true, fsync() directory after deleting file */
){
  పూర్ణాంకం rc = SQLITE_OK;
  వాడని_PARAMETER(Not_వాడిన);
  SimulateIOError(తిరిగి SQLITE_IOERR_DELETE);
  ఒకవేళ( osUnlink(z_మార్గం)==(-1) ){
    ఒకవేళ( errno==ENOENT
#ఒకవేళ OS_VXWORKS
        || osAccess(z_మార్గం,0)!=0
#అయితే_ముగింపు
    ){
      rc = SQLITE_IOERR_DELETE_NOENT;
    }లేకపోతే{
      rc = unixLogError(SQLITE_IOERR_DELETE, "unlink", z_మార్గం);
    }
    తిరిగి rc;
  }
#నిర్వచితం_కాకపోతే SQLITE_DISABLE_DIRSYNC
  ఒకవేళ( (dirSync & 1)!=0 ){
    పూర్ణాంకం fd;
    rc = osOpenDirectory(z_మార్గం, &fd);
    ఒకవేళ( rc==SQLITE_OK ){
      ఒకవేళ( నిండిన_fsync(fd,0,0) ){
        rc = unixLogError(SQLITE_IOERR_DIR_FSYNC, "fsync", z_మార్గం);
      }
      robust_close(0, fd, __LINE__);
    }లేకపోతే{
      ప్రతిజ్ఞా( rc==SQLITE_CANTOPEN );
      rc = SQLITE_OK;
    }
  }
#అయితే_ముగింపు
  తిరిగి rc;
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
స్థిర పూర్ణాంకం unixAccess(
  sqlite3_vfs *Not_వాడిన,   /* The VFS containing this xAccess method */
  స్థిరరాశి అక్షరం *z_మార్గం,      /* Path of the file to examine */
  పూర్ణాంకం flags,              /* What do we want to learn about the zPath file? */
  పూర్ణాంకం *p_Res_బహిర్గతం            /* Write result boolean here */
){
  వాడని_PARAMETER(Not_వాడిన);
  SimulateIOError( తిరిగి SQLITE_IOERR_ACCESS; );
  ప్రతిజ్ఞా( p_Res_బహిర్గతం!=0 );

  /* The spec says there are three possible values for flags.  But only
  ** two of them are actually used */
  ప్రతిజ్ఞా( flags==SQLITE_ACCESS_EXISTS || flags==SQLITE_ACCESS_READWRITE );

  ఒకవేళ( flags==SQLITE_ACCESS_EXISTS ){
    నిర్మాణం stat buf;
    *p_Res_బహిర్గతం = 0==osStat(z_మార్గం, &buf) &&
                (!S_ISREG(buf.st_బహుళకం) || buf.st_కొలత>0);
  }లేకపోతే{
    *p_Res_బహిర్గతం = osAccess(z_మార్గం, W_OK|R_OK)==0;
  }
  తిరిగి SQLITE_OK;
}

/*
** A pathname under construction
*/
రకనిర్వచనం నిర్మాణం Db_మార్గం Db_మార్గం;
నిర్మాణం Db_మార్గం {
  పూర్ణాంకం rc;           /* Non-zero following any error */
  పూర్ణాంకం nSymlink;     /* Number of symlinks resolved */
  అక్షరం *z_బహిర్గతం;       /* Write the pathname here */
  పూర్ణాంకం n_బహిర్గతం;         /* Bytes of space available to zOut[] */
  పూర్ణాంకం n_వాడిన;        /* Bytes of zOut[] currently being used */
};

/* Forward reference */
స్థిర శూన్య append_All_మార్గం_Elements(Db_మార్గం*,స్థిరరాశి అక్షరం*);

/*
** Append a single path element to the DbPath under construction
*/
స్థిర శూన్య append_One_మార్గం_Element(
  Db_మార్గం *p_మార్గం,       /* Path under construction, to which to append zName */
  స్థిరరాశి అక్షరం *zName,   /* Name to append to pPath.  Not zero-terminated */
  పూర్ణాంకం nName            /* Number of significant bytes in zName */
){
  ప్రతిజ్ఞా( nName>0 );
  ప్రతిజ్ఞా( zName!=0 );
  ఒకవేళ( zName[0]=='.' ){
    ఒకవేళ( nName==1 ) తిరిగి;
    ఒకవేళ( zName[1]=='.' && nName==2 ){
      ఒకవేళ( p_మార్గం->n_వాడిన>1 ){
        ప్రతిజ్ఞా( p_మార్గం->z_బహిర్గతం[0]=='/' );
        అయితే( p_మార్గం->z_బహిర్గతం[--p_మార్గం->n_వాడిన]!='/' ){}
      }
      తిరిగి;
    }
  }
  ఒకవేళ( p_మార్గం->n_వాడిన + nName + 2 >= p_మార్గం->n_బహిర్గతం ){
    p_మార్గం->rc = SQLITE_ERROR;
    తిరిగి;
  }
  p_మార్గం->z_బహిర్గతం[p_మార్గం->n_వాడిన++] = '/';
  జ్ఞాపకం_నకలు(&p_మార్గం->z_బహిర్గతం[p_మార్గం->n_వాడిన], zName, nName);
  p_మార్గం->n_వాడిన += nName;
#ఒకవేళ defined(HAVE_READLINK) && defined(HAVE_LSTAT)
  ఒకవేళ( p_మార్గం->rc==SQLITE_OK ){
    స్థిరరాశి అక్షరం *z_లోపల;
    నిర్మాణం stat buf;
    p_మార్గం->z_బహిర్గతం[p_మార్గం->n_వాడిన] = 0;
    z_లోపల = p_మార్గం->z_బహిర్గతం;
    ఒకవేళ( osLstat(z_లోపల, &buf)!=0 ){
      ఒకవేళ( errno!=ENOENT ){
        p_మార్గం->rc = unixLogError(SQLITE_CANTOPEN_BKPT, "lstat", z_లోపల);
      }
    }లేకపోతే ఒకవేళ( S_ISLNK(buf.st_బహుళకం) ){
      ssize_t got;
      అక్షరం zLnk[SQLITE_MAX_PATHLEN+2];
      ఒకవేళ( p_మార్గం->nSymlink++ > SQLITE_MAX_SYMLINK ){
        p_మార్గం->rc = SQLITE_CANTOPEN_BKPT;
        తిరిగి;
      }
      got = osReadlink(z_లోపల, zLnk, పరిమాణం(zLnk)-2);
      ఒకవేళ( got<=0 || got>=(ssize_t)పరిమాణం(zLnk)-2 ){
        p_మార్గం->rc = unixLogError(SQLITE_CANTOPEN_BKPT, "readlink", z_లోపల);
        తిరిగి;
      }
      zLnk[got] = 0;
      ఒకవేళ( zLnk[0]=='/' ){
        p_మార్గం->n_వాడిన = 0;
      }లేకపోతే{
        p_మార్గం->n_వాడిన -= nName + 1;
      }
      append_All_మార్గం_Elements(p_మార్గం, zLnk);
    }
  }
#అయితే_ముగింపు
}

/*
** Append all path elements in zPath to the DbPath under construction.
*/
స్థిర శూన్య append_All_మార్గం_Elements(
  Db_మార్గం *p_మార్గం,       /* Path under construction, to which to append zName */
  స్థిరరాశి అక్షరం *z_మార్గం    /* Path to append to pPath.  Is zero-terminated */
){
  పూర్ణాంకం i = 0;
  పూర్ణాంకం j = 0;
  చేయి{
    అయితే( z_మార్గం[i] && z_మార్గం[i]!='/' ){ i++; }
    ఒకవేళ( i>j ){
      append_One_మార్గం_Element(p_మార్గం, &z_మార్గం[j], i-j);
    }
    j = i+1;
  }అయితే( z_మార్గం[i++] );
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
స్థిర పూర్ణాంకం unix_నిండిన_Pathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  స్థిరరాశి అక్షరం *z_మార్గం,            /* Possibly relative input path */
  పూర్ణాంకం n_బహిర్గతం,                     /* Size of output buffer in bytes */
  అక్షరం *z_బహిర్గతం                    /* Output buffer */
){
  Db_మార్గం మార్గం;
  వాడని_PARAMETER(pVfs);
  మార్గం.rc = 0;
  మార్గం.n_వాడిన = 0;
  మార్గం.nSymlink = 0;
  మార్గం.n_బహిర్గతం = n_బహిర్గతం;
  మార్గం.z_బహిర్గతం = z_బహిర్గతం;
  ఒకవేళ( z_మార్గం[0]!='/' ){
    అక్షరం zPwd[SQLITE_MAX_PATHLEN+2];
    ఒకవేళ( osGetcwd(zPwd, పరిమాణం(zPwd)-2)==0 ){
      తిరిగి unixLogError(SQLITE_CANTOPEN_BKPT, "getcwd", z_మార్గం);
    }
    append_All_మార్గం_Elements(&మార్గం, zPwd);
  }
  append_All_మార్గం_Elements(&మార్గం, z_మార్గం);
  z_బహిర్గతం[మార్గం.n_వాడిన] = 0;
  ఒకవేళ( మార్గం.rc || మార్గం.n_వాడిన<2 ) తిరిగి SQLITE_CANTOPEN_BKPT;
  ఒకవేళ( మార్గం.nSymlink ) తిరిగి SQLITE_OK_SYMLINK;
  తిరిగి SQLITE_OK;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
#అన్తర్భావ <dlfcn.h>
స్థిర శూన్య *unixDlOpen(sqlite3_vfs *Not_వాడిన, స్థిరరాశి అక్షరం *zFilename){
  వాడని_PARAMETER(Not_వాడిన);
  తిరిగి dlopen(zFilename, RTLD_NOW | RTLD_GLOBAL);
}

/*
** SQLite calls this function immediately after a call to unixDlSym() or
** unixDlOpen() fails (returns a null pointer). If a more detailed error
** message is available, it is written to zBufOut. If no error message
** is available, zBufOut is left unmodified and SQLite uses a default
** error message.
*/
స్థిర శూన్య unixDlError(sqlite3_vfs *Not_వాడిన, పూర్ణాంకం nBuf, అక్షరం *z_Buf_బహిర్గతం){
  స్థిరరాశి అక్షరం *zErr;
  వాడని_PARAMETER(Not_వాడిన);
  unixEnterMutex();
  zErr = dlerror();
  ఒకవేళ( zErr ){
    sqlite3_snprintf(nBuf, z_Buf_బహిర్గతం, "%s", zErr);
  }
  unixLeaveMutex();
}
స్థిర శూన్య (*unixDlSym(sqlite3_vfs *Not_వాడిన, శూన్య *p, స్థిరరాశి అక్షరం*zSym))(శూన్య){
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
  శూన్య (*(*x)(శూన్య*,స్థిరరాశి అక్షరం*))(శూన్య);
  వాడని_PARAMETER(Not_వాడిన);
  x = (శూన్య(*(*)(శూన్య*,స్థిరరాశి అక్షరం*))(శూన్య))dlsym;
  తిరిగి (*x)(p, zSym);
}
స్థిర శూన్య unixDlClose(sqlite3_vfs *Not_వాడిన, శూన్య *pHandle){
  వాడని_PARAMETER(Not_వాడిన);
  dlclose(pHandle);
}
#లేకపోతే /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #నిర్వచించు unixDlOpen  0
  #నిర్వచించు unixDlError 0
  #నిర్వచించు unixDlSym   0
  #నిర్వచించు unixDlClose 0
#అయితే_ముగింపు

/*
** Write nBuf bytes of random data to the supplied buffer zBuf.
*/
స్థిర పూర్ణాంకం unixRandomness(sqlite3_vfs *Not_వాడిన, పూర్ణాంకం nBuf, అక్షరం *zBuf){
  వాడని_PARAMETER(Not_వాడిన);
  ప్రతిజ్ఞా((size_t)nBuf>=(పరిమాణం(సమయం_t)+పరిమాణం(పూర్ణాంకం)));

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
#ఒకవేళ !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
  {
    పూర్ణాంకం fd, got;
    fd = robust_open("/dev/urandom", O_RDONLY, 0);
    ఒకవేళ( fd<0 ){
      సమయం_t t;
      సమయం(&t);
      జ్ఞాపకం_నకలు(zBuf, &t, పరిమాణం(t));
      జ్ఞాపకం_నకలు(&zBuf[పరిమాణం(t)], &randomnessPid, పరిమాణం(randomnessPid));
      ప్రతిజ్ఞా( పరిమాణం(t)+పరిమాణం(randomnessPid)<=(size_t)nBuf );
      nBuf = పరిమాణం(t) + పరిమాణం(randomnessPid);
    }లేకపోతే{
      చేయి{ got = osRead(fd, zBuf, nBuf); }అయితే( got<0 && errno==EINTR );
      robust_close(0, fd, __LINE__);
    }
  }
#అయితే_ముగింపు
  తిరిగి nBuf;
}


/*
** Sleep for a little while.  Return the amount of time slept.
** The argument is the number of microseconds we want to sleep.
** The return value is the number of microseconds of sleep actually
** requested from the underlying operating system, a number which
** might be greater than or equal to the argument, but not less
** than the argument.
*/
స్థిర పూర్ణాంకం unixSleep(sqlite3_vfs *Not_వాడిన, పూర్ణాంకం microseconds){
#ఒకవేళ !defined(HAVE_NANOSLEEP) || HAVE_NANOSLEEP+0
  నిర్మాణం timespec sp;
  sp.tv_sec = microseconds / 1000000;
  sp.tv_nsec = (microseconds % 1000000) * 1000;

  /* Almost all modern unix systems support nanosleep().  But if you are
  ** compiling for one of the rare exceptions, you can use
  ** -DHAVE_NANOSLEEP=0 (perhaps in conjunction with -DHAVE_USLEEP if
  ** usleep() is available) in order to bypass the use of nanosleep() */
  nanosleep(&sp, ఖాళీ);

  వాడని_PARAMETER(Not_వాడిన);
  తిరిగి microseconds;
#లేదావేళ defined(HAVE_USLEEP) && HAVE_USLEEP
  ఒకవేళ( microseconds>=1000000 ) sleep(microseconds/1000000);
  ఒకవేళ( microseconds%1000000 ) usleep(microseconds%1000000);
  వాడని_PARAMETER(Not_వాడిన);
  తిరిగి microseconds;
#లేకపోతే
  పూర్ణాంకం seconds = (microseconds+999999)/1000000;
  sleep(seconds);
  వాడని_PARAMETER(Not_వాడిన);
  తిరిగి seconds*1000000;
#అయితే_ముగింపు
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
స్థిర పూర్ణాంకం unix_Current_సమయం_Int64(sqlite3_vfs *Not_వాడిన, sqlite3_int64 *piNow){
  స్థిర స్థిరరాశి sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  పూర్ణాంకం rc = SQLITE_OK;
#ఒకవేళ defined(NO_GETTOD)
  సమయం_t t;
  సమయం(&t);
  *piNow = ((sqlite3_int64)t)*1000 + unixEpoch;
#లేదావేళ OS_VXWORKS
  నిర్మాణం timespec sNow;
  clock_gettime(CLOCK_REALTIME, &sNow);
  *piNow = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_nsec/1000000;
#లేకపోతే
  నిర్మాణం timeval sNow;
  (శూన్య)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *piNow = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_TEST
  ఒకవేళ( sqlite3_current_సమయం ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_సమయం + unixEpoch;
  }
#అయితే_ముగింపు
  వాడని_PARAMETER(Not_వాడిన);
  తిరిగి rc;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_DEPRECATED
/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
స్థిర పూర్ణాంకం unix_Current_సమయం(sqlite3_vfs *Not_వాడిన, ద్విగుణ *prNow){
  sqlite3_int64 i = 0;
  పూర్ణాంకం rc;
  వాడని_PARAMETER(Not_వాడిన);
  rc = unix_Current_సమయం_Int64(0, &i);
  *prNow = i/86400000.0;
  తిరిగి rc;
}
#లేకపోతే
# define unix_Current_సమయం 0
#అయితే_ముగింపు

/*
** The xGetLastError() method is designed to return a better
** low-level error message when operating-system problems come up
** during SQLite operation.  Only the integer return code is currently
** used.
*/
స్థిర పూర్ణాంకం unix_Get_చివరి_Error(sqlite3_vfs *Not_వాడిన, పూర్ణాంకం NotUsed2, అక్షరం *NotUsed3){
  వాడని_PARAMETER(Not_వాడిన);
  వాడని_PARAMETER(NotUsed2);
  వాడని_PARAMETER(NotUsed3);
  తిరిగి errno;
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
#ఒకవేళ defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE

/*
** The proxyLockingContext has the path and file structures for the remote
** and local proxy files in it
*/
రకనిర్వచనం నిర్మాణం proxyLockingContext proxyLockingContext;
నిర్మాణం proxyLockingContext {
  unixFile *conchFile;         /* Open conch file */
  అక్షరం *conch_File_మార్గం;         /* Name of the conch file */
  unixFile *తాళం_Proxy;         /* Open proxy lock file */
  అక్షరం *తాళం_Proxy_మార్గం;         /* Name of the proxy lock file */
  అక్షరం *db_మార్గం;                /* Name of the open file */
  పూర్ణాంకం conchHeld;               /* 1 if the conch is held, -1 if lockless */
  పూర్ణాంకం nFails;                  /* Number of conch taking failures */
  శూన్య *oldLockingContext;     /* Original lockingcontext to restore on close */
  sqlite3_io_methods స్థిరరాశి *p_Old_పద్ధతి;     /* Original I/O methods for close */
};

/*
** The proxy lock file path for the database at dbPath is written into lPath,
** which must point to valid, writable memory large enough for a maxLen length
** file path.
*/
స్థిర పూర్ణాంకం proxy_Get_తాళం_మార్గం(స్థిరరాశి అక్షరం *db_మార్గం, అక్షరం *l_మార్గం, size_t maxLen){
  పూర్ణాంకం len;
  పూర్ణాంకం dbLen;
  పూర్ణాంకం i;

#నిర్వచితమైతే LOCKPROXYDIR
  len = strlcpy(l_మార్గం, LOCKPROXYDIR, maxLen);
#లేకపోతే
# ifdef CS_DARWIN_USER_తాత్కాలిక_DIR
  {
    ఒకవేళ( !confstr(CS_DARWIN_USER_తాత్కాలిక_DIR, l_మార్గం, maxLen) ){
      OSTRACE(("GETLOCKPATH  failed %s errno=%d pid=%d\n",
               l_మార్గం, errno, osGetpid(0)));
      తిరిగి SQLITE_IOERR_తాళం;
    }
    len = strlcat(l_మార్గం, "sqliteplocks", maxLen);  
  }
# లేకపోతే
  len = strlcpy(l_మార్గం, "/tmp/", maxLen);
# endif
#అయితే_ముగింపు

  ఒకవేళ( l_మార్గం[len-1]!='/' ){
    len = strlcat(l_మార్గం, "/", maxLen);
  }

  /* transform the db path to a unique cache name */
  dbLen = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(db_మార్గం);
  కొరకు( i=0; i<dbLen && (i+len+7)<(పూర్ణాంకం)maxLen; i++){
    అక్షరం c = db_మార్గం[i];
    l_మార్గం[i+len] = (c=='/')?'_':c;
  }
  l_మార్గం[i+len]='\0';
  strlcat(l_మార్గం, ":auto:", maxLen);
  OSTRACE(("GETLOCKPATH  proxy lock path=%s pid=%d\n", l_మార్గం, osGetpid(0)));
  తిరిగి SQLITE_OK;
}

/*
 ** Creates the lock file and any missing directories in lockPath
 */
స్థిర పూర్ణాంకం proxy_Create_తాళం_మార్గం(స్థిరరాశి అక్షరం *తాళం_మార్గం){
  పూర్ణాంకం i, len;
  అక్షరం buf[MAXPATHLEN];
  పూర్ణాంకం ప్రారంభం = 0;

  ప్రతిజ్ఞా(తాళం_మార్గం!=ఖాళీ);
  /* try to create all the intermediate directories */
  len = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(తాళం_మార్గం);
  buf[0] = తాళం_మార్గం[0];
  కొరకు( i=1; i<len; i++ ){
    ఒకవేళ( తాళం_మార్గం[i] == '/' && (i - ప్రారంభం > 0) ){
      /* only mkdir if leaf dir != "." or "/" or ".." */
      ఒకవేళ( i-ప్రారంభం>2 || (i-ప్రారంభం==1 && buf[ప్రారంభం] != '.' && buf[ప్రారంభం] != '/')
         || (i-ప్రారంభం==2 && buf[ప్రారంభం] != '.' && buf[ప్రారంభం+1] != '.') ){
        buf[i]='\0';
        ఒకవేళ( osMkdir(buf, SQLITE_DEFAULT_PROXYDIR_PERMISSIONS) ){
          పూర్ణాంకం err=errno;
          ఒకవేళ( err!=EEXIST ) {
            OSTRACE(("CREATELOCKPATH  FAILED creating %s, "
                     "'%s' proxy lock path=%s pid=%d\n",
                     buf, strerror(err), తాళం_మార్గం, osGetpid(0)));
            తిరిగి err;
          }
        }
      }
      ప్రారంభం=i+1;
    }
    buf[i] = తాళం_మార్గం[i];
  }
  OSTRACE(("CREATELOCKPATH  proxy lock path=%s pid=%d\n",తాళం_మార్గం,osGetpid(0)));
  తిరిగి 0;
}

/*
** Create a new VFS file descriptor (stored in memory obtained from
** sqlite3_malloc) and open the file named "path" in the file descriptor.
**
** The caller is responsible not only for closing the file descriptor
** but also for freeing the memory associated with the file descriptor.
*/
స్థిర పూర్ణాంకం proxyCreateUnixFile(
    స్థిరరాశి అక్షరం *మార్గం,        /* path for the new unixFile */
    unixFile **ppFile,       /* unixFile created and returned by ref */
    పూర్ణాంకం islockfile           /* if non zero missing dirs will be created */
) {
  పూర్ణాంకం fd = -1;
  unixFile *pNew;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం openFlags = O_RDWR | O_CREAT | O_NOFOLLOW;
  sqlite3_vfs dummyVfs;
  పూర్ణాంకం terrno = 0;
  Unix_వాడని_Fd *p_వాడని = ఖాళీ;

  /* 1. first try to open/create the file
  ** 2. if that fails, and this is a lock file (not-conch), try creating
  ** the parent directories and then try again.
  ** 3. if that fails, try to open the file read-only
  ** otherwise return BUSY (if lock file) or CANTOPEN for the conch file
  */
  p_వాడని = కనుగొను_Reusable_Fd(మార్గం, openFlags);
  ఒకవేళ( p_వాడని ){
    fd = p_వాడని->fd;
  }లేకపోతే{
    p_వాడని = sqlite3_malloc64(పరిమాణం(*p_వాడని));
    ఒకవేళ( !p_వాడని ){
      తిరిగి SQLITE_NOMEM_BKPT;
    }
  }
  ఒకవేళ( fd<0 ){
    fd = robust_open(మార్గం, openFlags, 0);
    terrno = errno;
    ఒకవేళ( fd<0 && errno==ENOENT && islockfile ){
      ఒకవేళ( proxy_Create_తాళం_మార్గం(మార్గం) == SQLITE_OK ){
        fd = robust_open(మార్గం, openFlags, 0);
      }
    }
  }
  ఒకవేళ( fd<0 ){
    openFlags = O_RDONLY | O_NOFOLLOW;
    fd = robust_open(మార్గం, openFlags, 0);
    terrno = errno;
  }
  ఒకవేళ( fd<0 ){
    ఒకవేళ( islockfile ){
      తిరిగి SQLITE_BUSY;
    }
    ఎంపిక (terrno) {
      స్థితి EACCES:
        తిరిగి SQLITE_PERM;
      స్థితి EIO:
        తిరిగి SQLITE_IOERR_తాళం; /* even though it is the conch */
      అప్రమేయ:
        తిరిగి SQLITE_CANTOPEN_BKPT;
    }
  }

  pNew = (unixFile *)sqlite3_malloc64(పరిమాణం(*pNew));
  ఒకవేళ( pNew==ఖాళీ ){
    rc = SQLITE_NOMEM_BKPT;
    వెళ్ళు చివర_create_proxy;
  }
  memset(pNew, 0, పరిమాణం(unixFile));
  pNew->openFlags = openFlags;
  memset(&dummyVfs, 0, పరిమాణం(dummyVfs));
  dummyVfs.p_App_దత్తాంశం = (శూన్య*)&autolockIoFinder;
  dummyVfs.zName = "dummy";
  p_వాడని->fd = fd;
  p_వాడని->flags = openFlags;
  pNew->p_Preallocated_వాడని = p_వాడని;

  rc = fill_లోపల_Unix_File(&dummyVfs, fd, (sqlite3_file*)pNew, మార్గం, 0);
  ఒకవేళ( rc==SQLITE_OK ){
    *ppFile = pNew;
    తిరిగి SQLITE_OK;
  }
చివర_create_proxy:  
  robust_close(pNew, fd, __LINE__);
  sqlite3_free(pNew);
  sqlite3_free(p_వాడని);
  తిరిగి rc;
}

#నిర్వచితమైతే SQLITE_TEST
/* simulate multiple hosts by creating unique hostid file paths */
పూర్ణాంకం sqlite3_hostid_అంకె = 0;
#అయితే_ముగింపు

#నిర్వచించు PROXY_HOSTIDLEN    16  /* conch file host id length */

#ఒకవేళ HAVE_GETHOSTUUID
/* Not always defined in the headers as it ought to be */
బాహ్య పూర్ణాంకం gethostuuid(uuid_t id, స్థిరరాశి నిర్మాణం timespec *wait);
#అయితే_ముగింపు

/* get the host ID via gethostuuid(), pHostID must point to PROXY_HOSTIDLEN
** bytes of writable memory.
*/
స్థిర పూర్ణాంకం proxyGetHostID(గుర్తులేని అక్షరం *pHostID, పూర్ణాంకం *pError){
  ప్రతిజ్ఞా(PROXY_HOSTIDLEN == పరిమాణం(uuid_t));
  memset(pHostID, 0, PROXY_HOSTIDLEN);
#ఒకవేళ HAVE_GETHOSTUUID
  {
    నిర్మాణం timespec timeout = {1, 0}; /* 1 sec timeout */
    ఒకవేళ( gethostuuid(pHostID, &timeout) ){
      పూర్ణాంకం err = errno;
      ఒకవేళ( pError ){
        *pError = err;
      }
      తిరిగి SQLITE_IOERR;
    }
  }
#లేకపోతే
  వాడని_PARAMETER(pError);
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_TEST
  /* simulate multiple hosts by creating unique hostid file paths */
  ఒకవేళ( sqlite3_hostid_అంకె != 0){
    pHostID[0] = (అక్షరం)(pHostID[0] + (అక్షరం)(sqlite3_hostid_అంకె & 0xFF));
  }
#అయితే_ముగింపు

  తిరిగి SQLITE_OK;
}

/* The conch file contains the header, host id and lock file path
 */
#నిర్వచించు PROXY_CONCHVERSION 2   /* 1-byte header, 16-byte host id, path */
#నిర్వచించు PROXY_HEADERLEN    1   /* conch file header length */
#నిర్వచించు PROXY_PATHINDEX    (PROXY_HEADERLEN+PROXY_HOSTIDLEN)
#నిర్వచించు PROXY_MAXCONCHLEN  (PROXY_HEADERLEN+PROXY_HOSTIDLEN+MAXPATHLEN)

/*
** Takes an open conch file, copies the contents to a new path and then moves
** it back.  The newly created file's file descriptor is assigned to the
** conch file structure and finally the original conch file descriptor is
** closed.  Returns zero if successful.
*/
స్థిర పూర్ణాంకం proxy_Break_Conch_తాళం(unixFile *pFile, uuid_t myHostID){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
  unixFile *conchFile = pCtx->conchFile;
  అక్షరం t_మార్గం[MAXPATHLEN];
  అక్షరం buf[PROXY_MAXCONCHLEN];
  అక్షరం *c_మార్గం = pCtx->conch_File_మార్గం;
  size_t readLen = 0;
  size_t మార్గం_Len = 0;
  అక్షరం errmsg[64] = "";
  పూర్ణాంకం fd = -1;
  పూర్ణాంకం rc = -1;
  వాడని_PARAMETER(myHostID);

  /* create a new path by replace the trailing '-conch' with '-break' */
  మార్గం_Len = strlcpy(t_మార్గం, c_మార్గం, MAXPATHLEN);
  ఒకవేళ( మార్గం_Len>MAXPATHLEN || మార్గం_Len<6 ||
     (strlcpy(&t_మార్గం[మార్గం_Len-5], "break", 6) != 5) ){
    sqlite3_snprintf(పరిమాణం(errmsg),errmsg,"path error (len %d)",(పూర్ణాంకం)మార్గం_Len);
    వెళ్ళు చివర_breaklock;
  }
  /* read the conch content */
  readLen = osPread(conchFile->h, buf, PROXY_MAXCONCHLEN, 0);
  ఒకవేళ( readLen<PROXY_PATHINDEX ){
    sqlite3_snprintf(పరిమాణం(errmsg),errmsg,"read error (len %d)",(పూర్ణాంకం)readLen);
    వెళ్ళు చివర_breaklock;
  }
  /* write it out to the temporary break file */
  fd = robust_open(t_మార్గం, (O_RDWR|O_CREAT|O_EXCL|O_NOFOLLOW), 0);
  ఒకవేళ( fd<0 ){
    sqlite3_snprintf(పరిమాణం(errmsg), errmsg, "create failed (%d)", errno);
    వెళ్ళు చివర_breaklock;
  }
  ఒకవేళ( osPwrite(fd, buf, readLen, 0) != (ssize_t)readLen ){
    sqlite3_snprintf(పరిమాణం(errmsg), errmsg, "write failed (%d)", errno);
    వెళ్ళు చివర_breaklock;
  }
  ఒకవేళ( rename(t_మార్గం, c_మార్గం) ){
    sqlite3_snprintf(పరిమాణం(errmsg), errmsg, "rename failed (%d)", errno);
    వెళ్ళు చివర_breaklock;
  }
  rc = 0;
  fprintf(stderr, "broke stale lock on %s\n", c_మార్గం);
  robust_close(pFile, conchFile->h, __LINE__);
  conchFile->h = fd;
  conchFile->openFlags = O_RDWR | O_CREAT;

చివర_breaklock:
  ఒకవేళ( rc ){
    ఒకవేళ( fd>=0 ){
      osUnlink(t_మార్గం);
      robust_close(pFile, fd, __LINE__);
    }
    fprintf(stderr, "failed to break stale lock on %s, %s\n", c_మార్గం, errmsg);
  }
  తిరిగి rc;
}

/* Take the requested lock on the conch file and break a stale lock if the
** host id matches.
*/
స్థిర పూర్ణాంకం proxy_Conch_తాళం(unixFile *pFile, uuid_t myHostID, పూర్ణాంకం తాళం_Type){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
  unixFile *conchFile = pCtx->conchFile;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం nTries = 0;
  నిర్మాణం timespec conch_Mod_సమయం;

  memset(&conch_Mod_సమయం, 0, పరిమాణం(conch_Mod_సమయం));
  చేయి {
    rc = conchFile->p_పద్ధతి->x_తాళం((sqlite3_file*)conchFile, తాళం_Type);
    nTries ++;
    ఒకవేళ( rc==SQLITE_BUSY ){
      /* If the lock failed (busy):
       * 1st try: get the mod time of the conch, wait 0.5s and try again.
       * 2nd try: fail if the mod time changed or host id is different, wait
       *           10 sec and try again
       * 3rd try: break the lock unless the mod time has changed.
       */
      నిర్మాణం stat buf;
      ఒకవేళ( osFstat(conchFile->h, &buf) ){
        store_చివరి_Errno(pFile, errno);
        తిరిగి SQLITE_IOERR_తాళం;
      }
    
      ఒకవేళ( nTries==1 ){
        conch_Mod_సమయం = buf.st_mtimespec;
        unixSleep(0,500000); /* wait 0.5 sec and try the lock again*/
        కొనసాగు;
      }

      ప్రతిజ్ఞా( nTries>1 );
      ఒకవేళ( conch_Mod_సమయం.tv_sec != buf.st_mtimespec.tv_sec ||
         conch_Mod_సమయం.tv_nsec != buf.st_mtimespec.tv_nsec ){
        తిరిగి SQLITE_BUSY;
      }
    
      ఒకవేళ( nTries==2 ){
        అక్షరం tBuf[PROXY_MAXCONCHLEN];
        పూర్ణాంకం len = osPread(conchFile->h, tBuf, PROXY_MAXCONCHLEN, 0);
        ఒకవేళ( len<0 ){
          store_చివరి_Errno(pFile, errno);
          తిరిగి SQLITE_IOERR_తాళం;
        }
        ఒకవేళ( len>PROXY_PATHINDEX && tBuf[0]==(అక్షరం)PROXY_CONCHVERSION){
          /* don't break the lock if the host id doesn't match */
          ఒకవేళ( 0!=memcmp(&tBuf[PROXY_HEADERLEN], myHostID, PROXY_HOSTIDLEN) ){
            తిరిగి SQLITE_BUSY;
          }
        }లేకపోతే{
          /* don't break the lock on short read or a version mismatch */
          తిరిగి SQLITE_BUSY;
        }
        unixSleep(0,10000000); /* wait 10 sec and try the lock again */
        కొనసాగు;
      }
    
      ప్రతిజ్ఞా( nTries==3 );
      ఒకవేళ( 0==proxy_Break_Conch_తాళం(pFile, myHostID) ){
        rc = SQLITE_OK;
        ఒకవేళ( తాళం_Type==EXCLUSIVE_తాళం ){
          rc = conchFile->p_పద్ధతి->x_తాళం((sqlite3_file*)conchFile, SHARED_తాళం);
        }
        ఒకవేళ( !rc ){
          rc = conchFile->p_పద్ధతి->x_తాళం((sqlite3_file*)conchFile, తాళం_Type);
        }
      }
    }
  } అయితే( rc==SQLITE_BUSY && nTries<3 );

  తిరిగి rc;
}

/* Takes the conch by taking a shared lock and read the contents conch, if
** lockPath is non-NULL, the host ID and lock file path must match.  A NULL
** lockPath means that the lockPath in the conch file will be used if the
** host IDs match, or a new lock path will be generated automatically
** and written to the conch file.
*/
స్థిర పూర్ణాంకం proxyTakeConch(unixFile *pFile){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;

  ఒకవేళ( pCtx->conchHeld!=0 ){
    తిరిగి SQLITE_OK;
  }లేకపోతే{
    unixFile *conchFile = pCtx->conchFile;
    uuid_t myHostID;
    పూర్ణాంకం pError = 0;
    అక్షరం readBuf[PROXY_MAXCONCHLEN];
    అక్షరం తాళం_మార్గం[MAXPATHLEN];
    అక్షరం *తాత్కాలిక_తాళం_మార్గం = ఖాళీ;
    పూర్ణాంకం rc = SQLITE_OK;
    పూర్ణాంకం createConch = 0;
    పూర్ణాంకం host_Id_సరిపోలిక = 0;
    పూర్ణాంకం readLen = 0;
    పూర్ణాంకం try_Old_తాళం_మార్గం = 0;
    పూర్ణాంకం force_New_తాళం_మార్గం = 0;
  
    OSTRACE(("TAKECONCH  %d for %s pid=%d\n", conchFile->h,
             (pCtx->తాళం_Proxy_మార్గం ? pCtx->తాళం_Proxy_మార్గం : ":auto:"),
             osGetpid(0)));

    rc = proxyGetHostID(myHostID, &pError);
    ఒకవేళ( (rc&0xff)==SQLITE_IOERR ){
      store_చివరి_Errno(pFile, pError);
      వెళ్ళు చివర_takeconch;
    }
    rc = proxy_Conch_తాళం(pFile, myHostID, SHARED_తాళం);
    ఒకవేళ( rc!=SQLITE_OK ){
      వెళ్ళు చివర_takeconch;
    }
    /* read the existing conch file */
    readLen = seekAndRead((unixFile*)conchFile, 0, readBuf, PROXY_MAXCONCHLEN);
    ఒకవేళ( readLen<0 ){
      /* I/O error: lastErrno set by seekAndRead */
      store_చివరి_Errno(pFile, conchFile->చివరి_Errno);
      rc = SQLITE_IOERR_READ;
      వెళ్ళు చివర_takeconch;
    }లేకపోతే ఒకవేళ( readLen<=(PROXY_HEADERLEN+PROXY_HOSTIDLEN) ||
             readBuf[0]!=(అక్షరం)PROXY_CONCHVERSION ){
      /* a short read or version format mismatch means we need to create a new
      ** conch file.
      */
      createConch = 1;
    }
    /* if the host id matches and the lock path already exists in the conch
    ** we'll try to use the path there, if we can't open that path, we'll
    ** retry with a new auto-generated path
    */
    చేయి { /* in case we need to try again for an :auto: named lock file */

      ఒకవేళ( !createConch && !force_New_తాళం_మార్గం ){
        host_Id_సరిపోలిక = !memcmp(&readBuf[PROXY_HEADERLEN], myHostID,
                                  PROXY_HOSTIDLEN);
        /* if the conch has data compare the contents */
        ఒకవేళ( !pCtx->తాళం_Proxy_మార్గం ){
          /* for auto-named local lock file, just check the host ID and we'll
           ** use the local lock file path that's already in there
           */
          ఒకవేళ( host_Id_సరిపోలిక ){
            size_t మార్గం_Len = (readLen - PROXY_PATHINDEX);
          
            ఒకవేళ( మార్గం_Len>=MAXPATHLEN ){
              మార్గం_Len=MAXPATHLEN-1;
            }
            జ్ఞాపకం_నకలు(తాళం_మార్గం, &readBuf[PROXY_PATHINDEX], మార్గం_Len);
            తాళం_మార్గం[మార్గం_Len] = 0;
            తాత్కాలిక_తాళం_మార్గం = తాళం_మార్గం;
            try_Old_తాళం_మార్గం = 1;
            /* create a copy of the lock path if the conch is taken */
            వెళ్ళు చివర_takeconch;
          }
        }లేకపోతే ఒకవేళ( host_Id_సరిపోలిక
               && !అక్షరశ్రేణి_పోలిక(pCtx->తాళం_Proxy_మార్గం, &readBuf[PROXY_PATHINDEX],
                           readLen-PROXY_PATHINDEX)
        ){
          /* conch host and lock path match */
          వెళ్ళు చివర_takeconch;
        }
      }
    
      /* if the conch isn't writable and doesn't match, we can't take it */
      ఒకవేళ( (conchFile->openFlags&O_RDWR) == 0 ){
        rc = SQLITE_BUSY;
        వెళ్ళు చివర_takeconch;
      }
    
      /* either the conch didn't match or we need to create a new one */
      ఒకవేళ( !pCtx->తాళం_Proxy_మార్గం ){
        proxy_Get_తాళం_మార్గం(pCtx->db_మార్గం, తాళం_మార్గం, MAXPATHLEN);
        తాత్కాలిక_తాళం_మార్గం = తాళం_మార్గం;
        /* create a copy of the lock path _only_ if the conch is taken */
      }
    
      /* update conch with host and path (this will fail if other process
      ** has a shared lock already), if the host id matches, use the big
      ** stick.
      */
      futimes(conchFile->h, ఖాళీ);
      ఒకవేళ( host_Id_సరిపోలిక && !createConch ){
        ఒకవేళ( conchFile->pInode && conchFile->pInode->nShared>1 ){
          /* We are trying for an exclusive lock but another thread in this
           ** same process is still holding a shared lock. */
          rc = SQLITE_BUSY;
        } లేకపోతే {        
          rc = proxy_Conch_తాళం(pFile, myHostID, EXCLUSIVE_తాళం);
        }
      }లేకపోతే{
        rc = proxy_Conch_తాళం(pFile, myHostID, EXCLUSIVE_తాళం);
      }
      ఒకవేళ( rc==SQLITE_OK ){
        అక్షరం write_బఫరు[PROXY_MAXCONCHLEN];
        పూర్ణాంకం write_కొలత = 0;
      
        write_బఫరు[0] = (అక్షరం)PROXY_CONCHVERSION;
        జ్ఞాపకం_నకలు(&write_బఫరు[PROXY_HEADERLEN], myHostID, PROXY_HOSTIDLEN);
        ఒకవేళ( pCtx->తాళం_Proxy_మార్గం!=ఖాళీ ){
          strlcpy(&write_బఫరు[PROXY_PATHINDEX], pCtx->తాళం_Proxy_మార్గం,
                  MAXPATHLEN);
        }లేకపోతే{
          strlcpy(&write_బఫరు[PROXY_PATHINDEX], తాత్కాలిక_తాళం_మార్గం, MAXPATHLEN);
        }
        write_కొలత = PROXY_PATHINDEX + అక్షరశ్రేణి_పొడవు(&write_బఫరు[PROXY_PATHINDEX]);
        robust_ftruncate(conchFile->h, write_కొలత);
        rc = unixWrite((sqlite3_file *)conchFile, write_బఫరు, write_కొలత, 0);
        నిండిన_fsync(conchFile->h,0,0);
        /* If we created a new conch file (not just updated the contents of a
         ** valid conch file), try to match the permissions of the database
         */
        ఒకవేళ( rc==SQLITE_OK && createConch ){
          నిర్మాణం stat buf;
          పూర్ణాంకం err = osFstat(pFile->h, &buf);
          ఒకవేళ( err==0 ){
            బహుళకం_t cmode = buf.st_బహుళకం&(S_IRUSR|S_IWUSR | S_IRGRP|S_IWGRP |
                                        S_IROTH|S_IWOTH);
            /* try to match the database file R/W permissions, ignore failure */
#నిర్వచితం_కాకపోతే SQLITE_PROXY_DEBUG
            osFchmod(conchFile->h, cmode);
#లేకపోతే
            చేయి{
              rc = osFchmod(conchFile->h, cmode);
            }అయితే( rc==(-1) && errno==EINTR );
            ఒకవేళ( rc!=0 ){
              పూర్ణాంకం సంకేతం_2 = errno;
              fprintf(stderr, "fchmod %o FAILED with %d %s\n",
                      cmode, సంకేతం_2, strerror(సంకేతం_2));
            } లేకపోతే {
              fprintf(stderr, "fchmod %o SUCCEDED\n",cmode);
            }
          }లేకపోతే{
            పూర్ణాంకం సంకేతం_2 = errno;
            fprintf(stderr, "STAT FAILED[%d] with %d %s\n",
                    err, సంకేతం_2, strerror(సంకేతం_2));
#అయితే_ముగింపు
          }
        }
      }
      conchFile->p_పద్ధతి->xUnlock((sqlite3_file*)conchFile, SHARED_తాళం);
    
    చివర_takeconch:
      OSTRACE(("TRANSPROXY: CLOSE  %d\n", pFile->h));
      ఒకవేళ( rc==SQLITE_OK && pFile->openFlags ){
        పూర్ణాంకం fd;
        ఒకవేళ( pFile->h>=0 ){
          robust_close(pFile, pFile->h, __LINE__);
        }
        pFile->h = -1;
        fd = robust_open(pCtx->db_మార్గం, pFile->openFlags, 0);
        OSTRACE(("TRANSPROXY: OPEN  %d\n", fd));
        ఒకవేళ( fd>=0 ){
          pFile->h = fd;
        }లేకపోతే{
          rc=SQLITE_CANTOPEN_BKPT; /* SQLITE_BUSY? proxyTakeConch called
           during locking */
        }
      }
      ఒకవేళ( rc==SQLITE_OK && !pCtx->తాళం_Proxy ){
        అక్షరం *మార్గం = తాత్కాలిక_తాళం_మార్గం ? తాత్కాలిక_తాళం_మార్గం : pCtx->తాళం_Proxy_మార్గం;
        rc = proxyCreateUnixFile(మార్గం, &pCtx->తాళం_Proxy, 1);
        ఒకవేళ( rc!=SQLITE_OK && rc!=SQLITE_NOMEM && try_Old_తాళం_మార్గం ){
          /* we couldn't create the proxy lock file with the old lock file path
           ** so try again via auto-naming
           */
          force_New_తాళం_మార్గం = 1;
          try_Old_తాళం_మార్గం = 0;
          కొనసాగు; /* go back to the do {} while start point, try again */
        }
      }
      ఒకవేళ( rc==SQLITE_OK ){
        /* Need to make a copy of path if we extracted the value
         ** from the conch file or the path was allocated on the stack
         */
        ఒకవేళ( తాత్కాలిక_తాళం_మార్గం ){
          pCtx->తాళం_Proxy_మార్గం = sqlite3DbStrDup(0, తాత్కాలిక_తాళం_మార్గం);
          ఒకవేళ( !pCtx->తాళం_Proxy_మార్గం ){
            rc = SQLITE_NOMEM_BKPT;
          }
        }
      }
      ఒకవేళ( rc==SQLITE_OK ){
        pCtx->conchHeld = 1;
      
        ఒకవేళ( pCtx->తాళం_Proxy->p_పద్ధతి == &afpIoMethods ){
          afpLockingContext *afpCtx;
          afpCtx = (afpLockingContext *)pCtx->తాళం_Proxy->lockingContext;
          afpCtx->db_మార్గం = pCtx->తాళం_Proxy_మార్గం;
        }
      } లేకపోతే {
        conchFile->p_పద్ధతి->xUnlock((sqlite3_file*)conchFile, NO_తాళం);
      }
      OSTRACE(("TAKECONCH  %d %s\n", conchFile->h,
               rc==SQLITE_OK?"ok":"failed"));
      తిరిగి rc;
    } అయితే (1); /* in case we need to retry the :auto: lock file -
                 ** we should never get here except via the 'continue' call. */
  }
}

/*
** If pFile holds a lock on a conch file, then release that lock.
*/
స్థిర పూర్ణాంకం proxyReleaseConch(unixFile *pFile){
  పూర్ణాంకం rc = SQLITE_OK;         /* Subroutine return code */
  proxyLockingContext *pCtx;  /* The locking context for the proxy lock */
  unixFile *conchFile;        /* Name of the conch file */

  pCtx = (proxyLockingContext *)pFile->lockingContext;
  conchFile = pCtx->conchFile;
  OSTRACE(("RELEASECONCH  %d for %s pid=%d\n", conchFile->h,
           (pCtx->తాళం_Proxy_మార్గం ? pCtx->తాళం_Proxy_మార్గం : ":auto:"),
           osGetpid(0)));
  ఒకవేళ( pCtx->conchHeld>0 ){
    rc = conchFile->p_పద్ధతి->xUnlock((sqlite3_file*)conchFile, NO_తాళం);
  }
  pCtx->conchHeld = 0;
  OSTRACE(("RELEASECONCH  %d %s\n", conchFile->h,
           (rc==SQLITE_OK ? "ok" : "failed")));
  తిరిగి rc;
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
స్థిర పూర్ణాంకం proxyCreateConchPathname(అక్షరం *db_మార్గం, అక్షరం **p_Conch_మార్గం){
  పూర్ణాంకం i;                        /* Loop counter */
  పూర్ణాంకం len = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(db_మార్గం); /* Length of database filename - dbPath */
  అక్షరం *conch_మార్గం;              /* buffer in which to construct conch name */

  /* Allocate space for the conch filename and initialize the name to
  ** the name of the original database file. */
  *p_Conch_మార్గం = conch_మార్గం = (అక్షరం *)sqlite3_malloc64(len + 8);
  ఒకవేళ( conch_మార్గం==0 ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  జ్ఞాపకం_నకలు(conch_మార్గం, db_మార్గం, len+1);

  /* now insert a "." before the last / character */
  కొరకు( i=(len-1); i>=0; i-- ){
    ఒకవేళ( conch_మార్గం[i]=='/' ){
      i++;
      ఆపు;
    }
  }
  conch_మార్గం[i]='.';
  అయితే ( i<len ){
    conch_మార్గం[i+1]=db_మార్గం[i];
    i++;
  }

  /* append the "-conch" suffix to the file */
  జ్ఞాపకం_నకలు(&conch_మార్గం[i+1], "-conch", 7);
  ప్రతిజ్ఞా( (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(conch_మార్గం) == len+7 );

  తిరిగి SQLITE_OK;
}


/* Takes a fully configured proxy locking-style unix file and switches
** the local lock file path
*/
స్థిర పూర్ణాంకం switch_తాళం_Proxy_మార్గం(unixFile *pFile, స్థిరరాశి అక్షరం *మార్గం) {
  proxyLockingContext *pCtx = (proxyLockingContext*)pFile->lockingContext;
  అక్షరం *old_మార్గం = pCtx->తాళం_Proxy_మార్గం;
  పూర్ణాంకం rc = SQLITE_OK;

  ఒకవేళ( pFile->e_File_తాళం!=NO_తాళం ){
    తిరిగి SQLITE_BUSY;
  }

  /* nothing to do if the path is NULL, :auto: or matches the existing path */
  ఒకవేళ( !మార్గం || మార్గం[0]=='\0' || !strcmp(మార్గం, ":auto:") ||
    (old_మార్గం && !అక్షరశ్రేణి_పోలిక(old_మార్గం, మార్గం, MAXPATHLEN)) ){
    తిరిగి SQLITE_OK;
  }లేకపోతే{
    unixFile *తాళం_Proxy = pCtx->తాళం_Proxy;
    pCtx->తాళం_Proxy=ఖాళీ;
    pCtx->conchHeld = 0;
    ఒకవేళ( తాళం_Proxy!=ఖాళీ ){
      rc=తాళం_Proxy->p_పద్ధతి->xClose((sqlite3_file *)తాళం_Proxy);
      ఒకవేళ( rc ) తిరిగి rc;
      sqlite3_free(తాళం_Proxy);
    }
    sqlite3_free(old_మార్గం);
    pCtx->తాళం_Proxy_మార్గం = sqlite3DbStrDup(0, మార్గం);
  }

  తిరిగి rc;
}

/*
** pFile is a file that has been opened by a prior xOpen call.  dbPath
** is a string buffer at least MAXPATHLEN+1 characters in size.
**
** This routine find the filename associated with pFile and writes it
** int dbPath.
*/
స్థిర పూర్ణాంకం proxy_Get_Db_మార్గం_For_Unix_File(unixFile *pFile, అక్షరం *db_మార్గం){
#ఒకవేళ defined(__APPLE__)
  ఒకవేళ( pFile->p_పద్ధతి == &afpIoMethods ){
    /* afp style keeps a reference to the db path in the filePath field
    ** of the struct */
    ప్రతిజ్ఞా( (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు((అక్షరం*)pFile->lockingContext)<=MAXPATHLEN );
    strlcpy(db_మార్గం, ((afpLockingContext *)pFile->lockingContext)->db_మార్గం,
            MAXPATHLEN);
  } లేకపోతే
#అయితే_ముగింపు
  ఒకవేళ( pFile->p_పద్ధతి == &dotlockIoMethods ){
    /* dot lock style uses the locking context to store the dot lock
    ** file path */
    పూర్ణాంకం len = అక్షరశ్రేణి_పొడవు((అక్షరం *)pFile->lockingContext) - అక్షరశ్రేణి_పొడవు(DOTLOCK_SUFFIX);
    జ్ఞాపకం_నకలు(db_మార్గం, (అక్షరం *)pFile->lockingContext, len + 1);
  }లేకపోతే{
    /* all other styles use the locking context to store the db file path */
    ప్రతిజ్ఞా( అక్షరశ్రేణి_పొడవు((అక్షరం*)pFile->lockingContext)<=MAXPATHLEN );
    strlcpy(db_మార్గం, (అక్షరం *)pFile->lockingContext, MAXPATHLEN);
  }
  తిరిగి SQLITE_OK;
}

/*
** Takes an already filled in unix file and alters it so all file locking
** will be performed on the local proxy lock file.  The following fields
** are preserved in the locking context so that they can be restored and
** the unix structure properly cleaned up at close time:
**  ->lockingContext
**  ->pMethod
*/
స్థిర పూర్ణాంకం proxyTransformUnixFile(unixFile *pFile, స్థిరరాశి అక్షరం *మార్గం) {
  proxyLockingContext *pCtx;
  అక్షరం db_మార్గం[MAXPATHLEN+1];       /* Name of the database file */
  అక్షరం *తాళం_మార్గం=ఖాళీ;
  పూర్ణాంకం rc = SQLITE_OK;

  ఒకవేళ( pFile->e_File_తాళం!=NO_తాళం ){
    తిరిగి SQLITE_BUSY;
  }
  proxy_Get_Db_మార్గం_For_Unix_File(pFile, db_మార్గం);
  ఒకవేళ( !మార్గం || మార్గం[0]=='\0' || !strcmp(మార్గం, ":auto:") ){
    తాళం_మార్గం=ఖాళీ;
  }లేకపోతే{
    తాళం_మార్గం=(అక్షరం *)మార్గం;
  }

  OSTRACE(("TRANSPROXY  %d for %s pid=%d\n", pFile->h,
           (తాళం_మార్గం ? తాళం_మార్గం : ":auto:"), osGetpid(0)));

  pCtx = sqlite3_malloc64( పరిమాణం(*pCtx) );
  ఒకవేళ( pCtx==0 ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  memset(pCtx, 0, పరిమాణం(*pCtx));

  rc = proxyCreateConchPathname(db_మార్గం, &pCtx->conch_File_మార్గం);
  ఒకవేళ( rc==SQLITE_OK ){
    rc = proxyCreateUnixFile(pCtx->conch_File_మార్గం, &pCtx->conchFile, 0);
    ఒకవేళ( rc==SQLITE_CANTOPEN && ((pFile->openFlags&O_RDWR) == 0) ){
      /* if (a) the open flags are not O_RDWR, (b) the conch isn't there, and
      ** (c) the file system is read-only, then enable no-locking access.
      ** Ugh, since O_RDONLY==0x0000 we test for !O_RDWR since unixOpen asserts
      ** that openFlags will have only one of O_RDONLY or O_RDWR.
      */
      నిర్మాణం statfs fsInfo;
      నిర్మాణం stat conchInfo;
      పూర్ణాంకం goLockless = 0;

      ఒకవేళ( osStat(pCtx->conch_File_మార్గం, &conchInfo) == -1 ) {
        పూర్ణాంకం err = errno;
        ఒకవేళ( (err==ENOENT) && (statfs(db_మార్గం, &fsInfo) != -1) ){
          goLockless = (fsInfo.f_flags&MNT_RDONLY) == MNT_RDONLY;
        }
      }
      ఒకవేళ( goLockless ){
        pCtx->conchHeld = -1; /* read only FS/ lockless */
        rc = SQLITE_OK;
      }
    }
  }
  ఒకవేళ( rc==SQLITE_OK && తాళం_మార్గం ){
    pCtx->తాళం_Proxy_మార్గం = sqlite3DbStrDup(0, తాళం_మార్గం);
  }

  ఒకవేళ( rc==SQLITE_OK ){
    pCtx->db_మార్గం = sqlite3DbStrDup(0, db_మార్గం);
    ఒకవేళ( pCtx->db_మార్గం==ఖాళీ ){
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  ఒకవేళ( rc==SQLITE_OK ){
    /* all memory is allocated, proxys are created and assigned,
    ** switch the locking context and pMethod then return.
    */
    pCtx->oldLockingContext = pFile->lockingContext;
    pFile->lockingContext = pCtx;
    pCtx->p_Old_పద్ధతి = pFile->p_పద్ధతి;
    pFile->p_పద్ధతి = &proxyIoMethods;
  }లేకపోతే{
    ఒకవేళ( pCtx->conchFile ){
      pCtx->conchFile->p_పద్ధతి->xClose((sqlite3_file *)pCtx->conchFile);
      sqlite3_free(pCtx->conchFile);
    }
    sqlite3DbFree(0, pCtx->తాళం_Proxy_మార్గం);
    sqlite3_free(pCtx->conch_File_మార్గం);
    sqlite3_free(pCtx);
  }
  OSTRACE(("TRANSPROXY  %d %s\n", pFile->h,
           (rc==SQLITE_OK ? "ok" : "failed")));
  తిరిగి rc;
}


/*
** This routine handles sqlite3_file_control() calls that are specific
** to proxy locking.
*/
స్థిర పూర్ణాంకం proxyFileControl(sqlite3_file *id, పూర్ణాంకం op, శూన్య *pArg){
  ఎంపిక( op ){
    స్థితి SQLITE_FCNTL_GET_LOCKPROXYFILE: {
      unixFile *pFile = (unixFile*)id;
      ఒకవేళ( pFile->p_పద్ధతి == &proxyIoMethods ){
        proxyLockingContext *pCtx = (proxyLockingContext*)pFile->lockingContext;
        proxyTakeConch(pFile);
        ఒకవేళ( pCtx->తాళం_Proxy_మార్గం ){
          *(స్థిరరాశి అక్షరం **)pArg = pCtx->తాళం_Proxy_మార్గం;
        }లేకపోతే{
          *(స్థిరరాశి అక్షరం **)pArg = ":auto: (not held)";
        }
      } లేకపోతే {
        *(స్థిరరాశి అక్షరం **)pArg = ఖాళీ;
      }
      తిరిగి SQLITE_OK;
    }
    స్థితి SQLITE_FCNTL_సమితి_LOCKPROXYFILE: {
      unixFile *pFile = (unixFile*)id;
      పూర్ణాంకం rc = SQLITE_OK;
      పూర్ణాంకం isProxyStyle = (pFile->p_పద్ధతి == &proxyIoMethods);
      ఒకవేళ( pArg==ఖాళీ || (స్థిరరాశి అక్షరం *)pArg==0 ){
        ఒకవేళ( isProxyStyle ){
          /* turn off proxy locking - not supported.  If support is added for
          ** switching proxy locking mode off then it will need to fail if
          ** the journal mode is WAL mode.
          */
          rc = SQLITE_ERROR /*SQLITE_PROTOCOL? SQLITE_MISUSE?*/;
        }లేకపోతే{
          /* turn off proxy locking - already off - NOOP */
          rc = SQLITE_OK;
        }
      }లేకపోతే{
        స్థిరరాశి అక్షరం *proxy_మార్గం = (స్థిరరాశి అక్షరం *)pArg;
        ఒకవేళ( isProxyStyle ){
          proxyLockingContext *pCtx =
            (proxyLockingContext*)pFile->lockingContext;
          ఒకవేళ( !strcmp(pArg, ":auto:")
           || (pCtx->తాళం_Proxy_మార్గం &&
               !అక్షరశ్రేణి_పోలిక(pCtx->తాళం_Proxy_మార్గం, proxy_మార్గం, MAXPATHLEN))
          ){
            rc = SQLITE_OK;
          }లేకపోతే{
            rc = switch_తాళం_Proxy_మార్గం(pFile, proxy_మార్గం);
          }
        }లేకపోతే{
          /* turn on proxy file locking */
          rc = proxyTransformUnixFile(pFile, proxy_మార్గం);
        }
      }
      తిరిగి rc;
    }
    అప్రమేయ: {
      ప్రతిజ్ఞా( 0 );  /* The call assures that only valid opcodes are sent */
    }
  }
  /*NOTREACHED*/ ప్రతిజ్ఞా(0);
  తిరిగి SQLITE_ERROR;
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
స్థిర పూర్ణాంకం proxy_Check_Reserved_తాళం(sqlite3_file *id, పూర్ణాంకం *p_Res_బహిర్గతం) {
  unixFile *pFile = (unixFile*)id;
  పూర్ణాంకం rc = proxyTakeConch(pFile);
  ఒకవేళ( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    ఒకవేళ( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->తాళం_Proxy;
      తిరిగి proxy->p_పద్ధతి->x_Check_Reserved_తాళం((sqlite3_file*)proxy, p_Res_బహిర్గతం);
    }లేకపోతే{ /* conchHeld < 0 is lockless */
      p_Res_బహిర్గతం=0;
    }
  }
  తిరిగి rc;
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
స్థిర పూర్ణాంకం proxy_తాళం(sqlite3_file *id, పూర్ణాంకం e_File_తాళం) {
  unixFile *pFile = (unixFile*)id;
  పూర్ణాంకం rc = proxyTakeConch(pFile);
  ఒకవేళ( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    ఒకవేళ( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->తాళం_Proxy;
      rc = proxy->p_పద్ధతి->x_తాళం((sqlite3_file*)proxy, e_File_తాళం);
      pFile->e_File_తాళం = proxy->e_File_తాళం;
    }లేకపోతే{
      /* conchHeld < 0 is lockless */
    }
  }
  తిరిగి rc;
}


/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
స్థిర పూర్ణాంకం proxyUnlock(sqlite3_file *id, పూర్ణాంకం e_File_తాళం) {
  unixFile *pFile = (unixFile*)id;
  పూర్ణాంకం rc = proxyTakeConch(pFile);
  ఒకవేళ( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    ఒకవేళ( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->తాళం_Proxy;
      rc = proxy->p_పద్ధతి->xUnlock((sqlite3_file*)proxy, e_File_తాళం);
      pFile->e_File_తాళం = proxy->e_File_తాళం;
    }లేకపోతే{
      /* conchHeld < 0 is lockless */
    }
  }
  తిరిగి rc;
}

/*
** Close a file that uses proxy locks.
*/
స్థిర పూర్ణాంకం proxyClose(sqlite3_file *id) {
  ఒకవేళ( ALWAYS(id) ){
    unixFile *pFile = (unixFile*)id;
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    unixFile *తాళం_Proxy = pCtx->తాళం_Proxy;
    unixFile *conchFile = pCtx->conchFile;
    పూర్ణాంకం rc = SQLITE_OK;

    ఒకవేళ( తాళం_Proxy ){
      rc = తాళం_Proxy->p_పద్ధతి->xUnlock((sqlite3_file*)తాళం_Proxy, NO_తాళం);
      ఒకవేళ( rc ) తిరిగి rc;
      rc = తాళం_Proxy->p_పద్ధతి->xClose((sqlite3_file*)తాళం_Proxy);
      ఒకవేళ( rc ) తిరిగి rc;
      sqlite3_free(తాళం_Proxy);
      pCtx->తాళం_Proxy = 0;
    }
    ఒకవేళ( conchFile ){
      ఒకవేళ( pCtx->conchHeld ){
        rc = proxyReleaseConch(pFile);
        ఒకవేళ( rc ) తిరిగి rc;
      }
      rc = conchFile->p_పద్ధతి->xClose((sqlite3_file*)conchFile);
      ఒకవేళ( rc ) తిరిగి rc;
      sqlite3_free(conchFile);
    }
    sqlite3DbFree(0, pCtx->తాళం_Proxy_మార్గం);
    sqlite3_free(pCtx->conch_File_మార్గం);
    sqlite3DbFree(0, pCtx->db_మార్గం);
    /* restore the original locking context and pMethod then close it */
    pFile->lockingContext = pCtx->oldLockingContext;
    pFile->p_పద్ధతి = pCtx->p_Old_పద్ధతి;
    sqlite3_free(pCtx);
    తిరిగి pFile->p_పద్ధతి->xClose(id);
  }
  తిరిగి SQLITE_OK;
}



#అయితే_ముగింపు /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
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
పూర్ణాంకం sqlite3_os_init(శూన్య){
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
  #నిర్వచించు UNIXVFS(VFSNAME, FINDER) {                        \
    3,                    /* iVersion */                    \
    పరిమాణం(unixFile),     /* szOsFile */                    \
    MAX_PATHNAME,         /* mxPathname */                  \
    0,                    /* pNext */                       \
    VFSNAME,              /* zName */                       \
    (శూన్య*)&FINDER,       /* pAppData */                    \
    unixOpen,             /* xOpen */                       \
    unixDelete,           /* xDelete */                     \
    unixAccess,           /* xAccess */                     \
    unix_నిండిన_Pathname,     /* xFullPathname */               \
    unixDlOpen,           /* xDlOpen */                     \
    unixDlError,          /* xDlError */                    \
    unixDlSym,            /* xDlSym */                      \
    unixDlClose,          /* xDlClose */                    \
    unixRandomness,       /* xRandomness */                 \
    unixSleep,            /* xSleep */                      \
    unix_Current_సమయం,      /* xCurrentTime */                \
    unix_Get_చివరి_Error,     /* xGetLastError */               \
    unix_Current_సమయం_Int64, /* xCurrentTimeInt64 */           \
    unix_సమితి_System_Call,    /* xSetSystemCall */              \
    unixGetSystemCall,    /* xGetSystemCall */              \
    unix_తదుపరి_System_Call,   /* xNextSystemCall */             \
  }

  /*
  ** All default VFSes for unix are contained in the following array.
  **
  ** Note that the sqlite3_vfs.pNext field of the VFS object is modified
  ** by the SQLite core when the VFS is registered.  So the following
  ** array cannot be const.
  */
  స్థిర sqlite3_vfs aVfs[] = {
#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    UNIXVFS("unix",          autolockIoFinder ),
#లేదావేళ OS_VXWORKS
    UNIXVFS("unix",          vxworksIoFinder ),
#లేకపోతే
    UNIXVFS("unix",          posixIoFinder ),
#అయితే_ముగింపు
    UNIXVFS("unix-none",     nolockIoFinder ),
    UNIXVFS("unix-dotfile",  dotlockIoFinder ),
    UNIXVFS("unix-excl",     posixIoFinder ),
#ఒకవేళ OS_VXWORKS
    UNIXVFS("unix-namedsem", semIoFinder ),
#అయితే_ముగింపు
#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE || OS_VXWORKS
    UNIXVFS("unix-posix",    posixIoFinder ),
#అయితే_ముగింపు
#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE
    UNIXVFS("unix-flock",    flockIoFinder ),
#అయితే_ముగింపు
#ఒకవేళ SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    UNIXVFS("unix-afp",      afpIoFinder ),
    UNIXVFS("unix-nfs",      nfsIoFinder ),
    UNIXVFS("unix-proxy",    proxyIoFinder ),
#అయితే_ముగింపు
  };
  గుర్తులేని పూర్ణాంకం i;          /* Loop counter */

  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  ప్రతిజ్ఞా( Array_కొలత(aSyscall)==29 );

  /* Register all VFSes defined in the aVfs[] array */
  కొరకు(i=0; i<(పరిమాణం(aVfs)/పరిమాణం(sqlite3_vfs)); i++){
#నిర్వచితమైతే SQLITE_DEFAULT_UNIX_VFS
    sqlite3_vfs_రిజిస్టరు(&aVfs[i],
           0==strcmp(aVfs[i].zName,SQLITE_DEFAULT_UNIX_VFS));
#లేకపోతే
    sqlite3_vfs_రిజిస్టరు(&aVfs[i], i==0);
#అయితే_ముగింపు
  }
#నిర్వచితమైతే SQLITE_OS_KV_OPTIONAL
  sqlite3KvvfsInit();
#అయితే_ముగింపు
  unix_Big_తాళం = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#ఒకవేళ OS_VXWORKS
  vxworksMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS2);
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  /* Validate lock assumptions */
  ప్రతిజ్ఞా( SQLITE_SHM_NLOCK==8 );  /* Number of available locks */
  ప్రతిజ్ఞా( UNIX_SHM_పునాది==120  );  /* Start of locking area */
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
  ప్రతిజ్ఞా( UNIX_SHM_DMS==128   );  /* Byte offset of the deadman-switch */
#అయితే_ముగింపు

  తిరిగి SQLITE_OK;
}

/*
** Shutdown the operating system interface.
**
** Some operating systems might need to do some cleanup in this routine,
** to release dynamically allocated objects.  But not on unix.
** This routine is a no-op for unix.
*/
పూర్ణాంకం sqlite3_os_చివర(శూన్య){
  unix_Big_తాళం = 0;
#ఒకవేళ OS_VXWORKS
  vxworksMutex = 0;
#అయితే_ముగింపు
  తిరిగి SQLITE_OK;
}

#అయితే_ముగింపు /* SQLITE_OS_UNIX */
