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
#অন্তর্ভাব "sqliteInt.h"
#যদি SQLITE_OS_UNIX              /* This file is used on unix only */

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
#যদি !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  যদি defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  অন্যথা
#    define SQLITE_ENABLE_LOCKING_STYLE 0
#  endif
#endif

/* Use pread() and pwrite() if they are available */
#যদি defined(__APPLE__) || defined(__linux__)
# define HAVE_PREAD 1
# define HAVE_PWRITE 1
#endif
#যদি defined(HAVE_PREAD64) && defined(HAVE_PWRITE64)
# undef USE_PREAD
# define USE_PREAD64 1
#নোচেদ্যদি defined(HAVE_PREAD) && defined(HAVE_PWRITE)
# undef USE_PREAD64
# define USE_PREAD 1
#endif

/*
** standard include files.
*/
#অন্তর্ভাব <sys/types.h>   /* amalgamator: keep */
#অন্তর্ভাব <sys/stat.h>    /* amalgamator: keep */
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <sys/ioctl.h>
#অন্তর্ভাব <unistd.h>      /* amalgamator: keep */
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <sys/time.h>    /* amalgamator: keep */
#অন্তর্ভাব <errno.h>
#যদি (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0) \
  && !defined(SQLITE_WASI)
# include <sys/mman.h>
#endif

#যদি SQLITE_ENABLE_LOCKING_STYLE
# include <sys/ioctl.h>
# include <sys/file.h>
# include <sys/param.h>
#endif /* SQLITE_ENABLE_LOCKING_STYLE */

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
#ifndef HAVE_GETHOSTUUID
# define HAVE_GETHOSTUUID 0
# যদি defined(__APPLE__) && ((__MAC_OS_X_VERSION_MIN_REQUIRED > 1050) || \
                            (__IPHONE_OS_VERSION_MIN_REQUIRED > 2000))
#    যদি (!defined(TARGET_OS_EMBEDDED) || (TARGET_OS_EMBEDDED==0)) \
        && (!defined(TARGET_IPHONE_SIMULATOR) || (TARGET_IPHONE_SIMULATOR==0))\
        && (!defined(TARGET_OS_MACCATALYST) || (TARGET_OS_MACCATALYST==0))
#      undef HAVE_GETHOSTUUID
#      define HAVE_GETHOSTUUID 1
#    অন্যথা
#      warning "gethostuuid() is disabled."
#    endif
#  endif
#endif


#যদি OS_VXWORKS
# include <sys/ioctl.h>
# include <semaphore.h>
# include <limits.h>
#endif /* OS_VXWORKS */

#যদি defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
# include <sys/mount.h>
#endif

#ifdef HAVE_UTIME
# include <utime.h>
#endif

/*
** Allowed values of unixFile.fsFlags
*/
#define SQLITE_FSFLAGS_IS_MSDOS     0x1

/*
** If we are to be thread-safe, include the pthreads header.
*/
#যদি SQLITE_THREADSAFE
# include <pthread.h>
#endif

/*
** Default permissions when creating a new file
*/
#ifndef SQLITE_DEFAULT_FILE_PERMISSIONS
# define SQLITE_DEFAULT_FILE_PERMISSIONS 0644
#endif

/*
** Default permissions when creating auto proxy dir
*/
#ifndef SQLITE_DEFAULT_PROXYDIR_PERMISSIONS
# define SQLITE_DEFAULT_PROXYDIR_PERMISSIONS 0755
#endif

/*
** Maximum supported path-length.
*/
#define MAX_PATHNAME 512

/*
** Maximum supported symbolic links
*/
#define SQLITE_MAX_SYMLINKS 100

/*
** Remove and stub certain info for WASI (WebAssembly System
** Interface) builds.
*/
#ifdef SQLITE_WASI
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
#  যদি __LONG_MAX == 0x7fffffffL
#   define F_GETLK 12
#   define F_SETLK 13
#   define F_SETLKW 14
#  অন্যথা
#   define F_GETLK 5
#   define F_SETLK 6
#   define F_SETLKW 7
#  endif
# endif
#অন্যথা /* !SQLITE_WASI */
# ifndef HAVE_FCHMOD
#  define HAVE_FCHMOD 1
# endif
#endif /* SQLITE_WASI */

#ifdef SQLITE_WASI
# define osGetpid(X) (pid_t)1
#অন্যথা
/* Always cast the getpid() return type for compatibility with
** kernel modules in VxWorks. */
# define osGetpid(X) (pid_t)getpid()
#endif

/*
** Only set the lastErrno if the error code is a real error and not
** a normal expected return code of SQLITE_BUSY or SQLITE_OK
*/
#define IS_LOCK_ERROR(x)  ((x != SQLITE_OK) && (x != SQLITE_BUSY))

/* Forward references */
প্রকারনির্দেশ সংরচনা unixShm unixShm;               /* Connection shared memory */
প্রকারনির্দেশ সংরচনা unix_Shm_গ্রন্থি unix_Shm_গ্রন্থি;       /* Shared memory instance */
প্রকারনির্দেশ সংরচনা unixInodeInfo unixInodeInfo;   /* An i-node */
প্রকারনির্দেশ সংরচনা UnixUnusedFd UnixUnusedFd;     /* An unused file descriptor */

/*
** Sometimes, after a file handle is closed by SQLite, the file descriptor
** cannot be closed immediately. In these cases, instances of the following
** structure are used to store the file descriptor while waiting for an
** opportunity to either close or reuse it.
*/
সংরচনা UnixUnusedFd {
  পূর্ণাঙ্ক fd;                   /* File descriptor to close */
  পূর্ণাঙ্ক flags;                /* Flags this file descriptor was opened with */
  UnixUnusedFd *pNext;      /* Next unused file descriptor on same file */
};

/*
** The unixFile structure is subclass of sqlite3_file specific to the unix
** VFS implementations.
*/
প্রকারনির্দেশ সংরচনা unixFile unixFile;
সংরচনা unixFile {
  sqlite3_io_methods ধ্রুব *pMethod;  /* Always the first entry */
  sqlite3_vfs *pVfs;                  /* The VFS that created this unixFile */
  unixInodeInfo *pInode;              /* Info about locks on this inode */
  পূর্ণাঙ্ক h;                              /* The file descriptor */
  unsigned বর্ণ eFileLock;            /* The type of lock held on this fd */
  unsigned short পূর্ণাঙ্ক ctrlFlags;       /* Behavioral bits.  UNIXFILE_* flags */
  পূর্ণাঙ্ক lastErrno;                      /* The unix errno from last I/O error */
  শূন্য *lockingContext;               /* Locking style specific state */
  UnixUnusedFd *pPreallocatedUnused;  /* Pre-allocated UnixUnusedFd */
  ধ্রুব বর্ণ *zPath;                  /* Name of the file */
  unixShm *pShm;                      /* Shared memory segment information */
  পূর্ণাঙ্ক szChunk;                        /* Configured by FCNTL_CHUNK_SIZE */
#যদি SQLITE_MAX_MMAP_SIZE>0
  পূর্ণাঙ্ক nFetchOut;                      /* Number of outstanding xFetch refs */
  sqlite3_int64 mmapSize;             /* Usable size of mapping at pMapRegion */
  sqlite3_int64 mmapSizeActual;       /* Actual size of mapping at pMapRegion */
  sqlite3_int64 mmapSizeMax;          /* Configured FCNTL_MMAP_SIZE value */
  শূন্য *pMapRegion;                   /* Memory mapped region */
#endif
  পূর্ণাঙ্ক sectorSize;                     /* Device sector size */
  পূর্ণাঙ্ক deviceCharacteristics;          /* Precomputed device characteristics */
#যদি SQLITE_ENABLE_LOCKING_STYLE
  পূর্ণাঙ্ক openFlags;                      /* The flags specified at open() */
#endif
#যদি SQLITE_ENABLE_LOCKING_STYLE || defined(__APPLE__)
  unsigned fsFlags;                   /* cached details from statfs() */
#endif
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  unsigned iBusyTimeout;              /* Wait this many millisec on locks */
  পূর্ণাঙ্ক bBlockOnConnect;                /* True to block for SHARED locks */
#endif
#যদি OS_VXWORKS
  সংরচনা vxworksFileId *pId;          /* Unique file ID */
#endif
#ifdef SQLITE_DEBUG
  /* The next group of variables are used to track whether or not the
  ** transaction counter in bytes 24-27 of database files are updated
  ** whenever any part of the database changes.  An assertion fault will
  ** occur if a file is updated without also updating the transaction
  ** counter.  This test is made to avoid new problems similar to the
  ** one described by ticket #3584.
  */
  unsigned বর্ণ transCntrChng;   /* True if the transaction counter changed */
  unsigned বর্ণ dbUpdate;        /* True if any part of database file changed */
  unsigned বর্ণ inNormalWrite;   /* True if in a normal write operation */

#endif

#ifdef SQLITE_TEST
  /* In test mode, increase the size of this structure a bit so that
  ** it is larger than the struct CrashFile defined in test6.c.
  */
  বর্ণ aPadding[32];
#endif
};

/* This variable holds the process id (pid) from when the xRandomness()
** method was called.  If xOpen() is called from a different process id,
** indicating that a fork() has occurred, the PRNG will be reset.
*/
স্থির pid_t randomnessPid = 0;

/*
** Allowed values for the unixFile.ctrlFlags bitmask:
*/
#define UNIXFILE_EXCL        0x01     /* Connections from one process only */
#define UNIXFILE_RDONLY      0x02     /* Connection is read only */
#define UNIXFILE_PERSIST_WAL 0x04     /* Persistent WAL mode */
#যদি !defined(SQLITE_DISABLE_DIRSYNC) && !defined(_AIX)
# define UNIXFILE_DIRSYNC    0x08     /* Directory sync needed */
#অন্যথা
# define UNIXFILE_DIRSYNC    0x00
#endif
#define UNIXFILE_PSOW        0x10     /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */
#define UNIXFILE_DELETE      0x20     /* Delete on close */
#define UNIXFILE_URI         0x40     /* Filename might have query parameters */
#define UNIXFILE_NOLOCK      0x80     /* Do no file locking */

/*
** Include code that is common to all os_*.c files
*/
#অন্তর্ভাব "os_common.h"

/*
** Define various macros that are missing from some systems.
*/
#ifndef O_LARGEFILE
# define O_LARGEFILE 0
#endif
#ifdef SQLITE_DISABLE_LFS
# undef O_LARGEFILE
# define O_LARGEFILE 0
#endif
#ifndef O_NOFOLLOW
# define O_NOFOLLOW 0
#endif
#ifndef O_দ্বিমিক
# define O_দ্বিমিক 0
#endif

/*
** The threadid macro resolves to the thread-id or to 0.  Used for
** testing and debugging only.
*/
#যদি SQLITE_THREADSAFE
#define threadid pthread_self()
#অন্যথা
#define threadid 0
#endif

/*
** HAVE_MREMAP defaults to true on Linux and false everywhere else.
*/
#যদি !defined(HAVE_MREMAP)
# যদি defined(__linux__) && defined(_GNU_SOURCE)
#  define HAVE_MREMAP 1
# অন্যথা
#  define HAVE_MREMAP 0
# endif
#endif

/*
** Explicitly call the 64-bit version of lseek() on Android. Otherwise, lseek()
** is the 32-bit version, even if _FILE_OFFSET_BITS=64 is defined.
*/
#ifdef __ANDROID__
# define lseek lseek64
#endif

#ifdef __linux__
/*
** Linux-specific IOCTL magic numbers used for controlling F2FS
*/
#define F2FS_IOCTL_MAGIC        0xf5
#define F2FS_IOC_START_ATOMIC_WRITE     _IO(F2FS_IOCTL_MAGIC, 1)
#define F2FS_IOC_COMMIT_ATOMIC_WRITE    _IO(F2FS_IOCTL_MAGIC, 2)
#define F2FS_IOC_START_VOLATILE_WRITE   _IO(F2FS_IOCTL_MAGIC, 3)
#define F2FS_IOC_ABORT_VOLATILE_WRITE   _IO(F2FS_IOCTL_MAGIC, 5)
#define F2FS_IOC_GET_FEATURES           _IOR(F2FS_IOCTL_MAGIC, 12, u32)
#define F2FS_FEATURE_ATOMIC_WRITE 0x0004
#endif /* __linux__ */


/*
** Different Unix systems declare open() in different ways.  Same use
** open(const char*,int,mode_t).  Others use open(const char*,int,...).
** The difference is important when using a pointer to the function.
**
** The safest way to deal with the problem is to always use this wrapper
** which always has the same well-defined interface.
*/
স্থির পূর্ণাঙ্ক posixOpen(ধ্রুব বর্ণ *zFile, পূর্ণাঙ্ক flags, পূর্ণাঙ্ক mode){
  প্রত্যাগম open(zFile, flags, mode);
}

/* Forward reference */
স্থির পূর্ণাঙ্ক openDirectory(ধ্রুব বর্ণ*, পূর্ণাঙ্ক*);
স্থির পূর্ণাঙ্ক unixGetpagesize(শূন্য);

/*
** Many system calls are accessed through pointer-to-functions so that
** they may be overridden at runtime to facilitate fault injection during
** testing and sandboxing.  The following array holds the names and pointers
** to all overrideable system calls.
*/
স্থির সংরচনা unix_syscall {
  ধ্রুব বর্ণ *zName;            /* Name of the system call */
  sqlite3_syscall_ptr pCurrent; /* Current value of the system call */
  sqlite3_syscall_ptr pDefault; /* Default value */
} aSyscall[] = {
  { "open",         (sqlite3_syscall_ptr)posixOpen,  0  },
#define osOpen      ((পূর্ণাঙ্ক(*)(ধ্রুব বর্ণ*,পূর্ণাঙ্ক,পূর্ণাঙ্ক))aSyscall[0].pCurrent)

  { "close",        (sqlite3_syscall_ptr)close,      0  },
#define osClose     ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক))aSyscall[1].pCurrent)

  { "access",       (sqlite3_syscall_ptr)access,     0  },
#define osAccess    ((পূর্ণাঙ্ক(*)(ধ্রুব বর্ণ*,পূর্ণাঙ্ক))aSyscall[2].pCurrent)

  { "getcwd",       (sqlite3_syscall_ptr)getcwd,     0  },
#define osGetcwd    ((বর্ণ*(*)(বর্ণ*,size_t))aSyscall[3].pCurrent)

  { "stat",         (sqlite3_syscall_ptr)stat,       0  },
#define osStat      ((পূর্ণাঙ্ক(*)(ধ্রুব বর্ণ*,সংরচনা stat*))aSyscall[4].pCurrent)

/*
** The DJGPP compiler environment looks mostly like Unix, but it
** lacks the fcntl() system call.  So redefine fcntl() to be something
** that always succeeds.  This means that locking does not occur under
** DJGPP.  But it is DOS - what did you expect?
*/
#ifdef __DJGPP__
  { "fstat",        0,                 0  },
#define osFstat(a,b,c)    0
#অন্যথা   
  { "fstat",        (sqlite3_syscall_ptr)fstat,      0  },
#define osFstat     ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক,সংরচনা stat*))aSyscall[5].pCurrent)
#endif

  { "ftruncate",    (sqlite3_syscall_ptr)ftruncate,  0  },
#define osFtruncate ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক,off_t))aSyscall[6].pCurrent)

  { "fcntl",        (sqlite3_syscall_ptr)fcntl,      0  },
#define osFcntl     ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক,পূর্ণাঙ্ক,...))aSyscall[7].pCurrent)

  { "read",         (sqlite3_syscall_ptr)পঠ,       0  },
#define osRead      ((ssize_t(*)(পূর্ণাঙ্ক,শূন্য*,size_t))aSyscall[8].pCurrent)

#যদি defined(USE_PREAD) || SQLITE_ENABLE_LOCKING_STYLE
  { "pread",        (sqlite3_syscall_ptr)pread,      0  },
#অন্যথা
  { "pread",        (sqlite3_syscall_ptr)0,          0  },
#endif
#define osPread     ((ssize_t(*)(পূর্ণাঙ্ক,শূন্য*,size_t,off_t))aSyscall[9].pCurrent)

#যদি defined(USE_PREAD64)
  { "pread64",      (sqlite3_syscall_ptr)pread64,    0  },
#অন্যথা
  { "pread64",      (sqlite3_syscall_ptr)0,          0  },
#endif
#define osPread64 ((ssize_t(*)(পূর্ণাঙ্ক,শূন্য*,size_t,off64_t))aSyscall[10].pCurrent)

  { "write",        (sqlite3_syscall_ptr)লিখ,      0  },
#define osWrite     ((ssize_t(*)(পূর্ণাঙ্ক,ধ্রুব শূন্য*,size_t))aSyscall[11].pCurrent)

#যদি defined(USE_PREAD) || SQLITE_ENABLE_LOCKING_STYLE
  { "pwrite",       (sqlite3_syscall_ptr)pwrite,     0  },
#অন্যথা
  { "pwrite",       (sqlite3_syscall_ptr)0,          0  },
#endif
#define osPwrite    ((ssize_t(*)(পূর্ণাঙ্ক,ধ্রুব শূন্য*,size_t,off_t))\
                    aSyscall[12].pCurrent)

#যদি defined(USE_PREAD64)
  { "pwrite64",     (sqlite3_syscall_ptr)pwrite64,   0  },
#অন্যথা
  { "pwrite64",     (sqlite3_syscall_ptr)0,          0  },
#endif
#define osPwrite64  ((ssize_t(*)(পূর্ণাঙ্ক,ধ্রুব শূন্য*,size_t,off64_t))\
                    aSyscall[13].pCurrent)

#যদি defined(HAVE_FCHMOD)
  { "fchmod",       (sqlite3_syscall_ptr)fchmod,          0  },
#define osFchmod    ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক,mode_t))aSyscall[14].pCurrent)
#অন্যথা
  { "fchmod",       (sqlite3_syscall_ptr)0,               0  },
#define osFchmod(FID,MODE) 0
#endif

#যদি defined(HAVE_POSIX_FALLOCATE) && HAVE_POSIX_FALLOCATE
  { "fallocate",    (sqlite3_syscall_ptr)posix_fallocate,  0 },
#অন্যথা
  { "fallocate",    (sqlite3_syscall_ptr)0,                0 },
#endif
#define osFallocate ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক,off_t,off_t))aSyscall[15].pCurrent)

  { "unlink",       (sqlite3_syscall_ptr)unlink,           0 },
#define osUnlink    ((পূর্ণাঙ্ক(*)(ধ্রুব বর্ণ*))aSyscall[16].pCurrent)

  { "openDirectory",    (sqlite3_syscall_ptr)openDirectory,      0 },
#define osOpenDirectory ((পূর্ণাঙ্ক(*)(ধ্রুব বর্ণ*,পূর্ণাঙ্ক*))aSyscall[17].pCurrent)

  { "mkdir",        (sqlite3_syscall_ptr)mkdir,           0 },
#define osMkdir     ((পূর্ণাঙ্ক(*)(ধ্রুব বর্ণ*,mode_t))aSyscall[18].pCurrent)

  { "rmdir",        (sqlite3_syscall_ptr)rmdir,           0 },
#define osRmdir     ((পূর্ণাঙ্ক(*)(ধ্রুব বর্ণ*))aSyscall[19].pCurrent)

#যদি defined(HAVE_FCHOWN)
  { "fchown",       (sqlite3_syscall_ptr)fchown,          0 },
#অন্যথা
  { "fchown",       (sqlite3_syscall_ptr)0,               0 },
#endif
#define osFchown    ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক,uid_t,gid_t))aSyscall[20].pCurrent)

#যদি defined(HAVE_FCHOWN)
  { "geteuid",      (sqlite3_syscall_ptr)geteuid,         0 },
#অন্যথা
  { "geteuid",      (sqlite3_syscall_ptr)0,               0 },
#endif
#define osGeteuid   ((uid_t(*)(শূন্য))aSyscall[21].pCurrent)

#যদি (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0) \
  && !defined(SQLITE_WASI)
  { "mmap",         (sqlite3_syscall_ptr)mmap,            0 },
#অন্যথা
  { "mmap",         (sqlite3_syscall_ptr)0,               0 },
#endif
#define osMmap ((শূন্য*(*)(শূন্য*,size_t,পূর্ণাঙ্ক,পূর্ণাঙ্ক,পূর্ণাঙ্ক,off_t))aSyscall[22].pCurrent)

#যদি (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0) \
  && !defined(SQLITE_WASI)
  { "munmap",       (sqlite3_syscall_ptr)munmap,          0 },
#অন্যথা
  { "munmap",       (sqlite3_syscall_ptr)0,               0 },
#endif
#define osMunmap ((পূর্ণাঙ্ক(*)(শূন্য*,size_t))aSyscall[23].pCurrent)

#যদি HAVE_MREMAP && (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0)
  { "mremap",       (sqlite3_syscall_ptr)mremap,          0 },
#অন্যথা
  { "mremap",       (sqlite3_syscall_ptr)0,               0 },
#endif
#define osMremap ((শূন্য*(*)(শূন্য*,size_t,size_t,পূর্ণাঙ্ক,...))aSyscall[24].pCurrent)

#যদি !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0
  { "getpagesize",  (sqlite3_syscall_ptr)unixGetpagesize, 0 },
#অন্যথা
  { "getpagesize",  (sqlite3_syscall_ptr)0,               0 },
#endif
#define osGetpagesize ((পূর্ণাঙ্ক(*)(শূন্য))aSyscall[25].pCurrent)

#যদি defined(HAVE_READLINK)
  { "readlink",     (sqlite3_syscall_ptr)readlink,        0 },
#অন্যথা
  { "readlink",     (sqlite3_syscall_ptr)0,               0 },
#endif
#define osReadlink ((ssize_t(*)(ধ্রুব বর্ণ*,বর্ণ*,size_t))aSyscall[26].pCurrent)

#যদি defined(HAVE_LSTAT)
  { "lstat",         (sqlite3_syscall_ptr)lstat,          0 },
#অন্যথা
  { "lstat",         (sqlite3_syscall_ptr)0,              0 },
#endif
#define osLstat      ((পূর্ণাঙ্ক(*)(ধ্রুব বর্ণ*,সংরচনা stat*))aSyscall[27].pCurrent)

#যদি defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
# ifdef __ANDROID__
  { "ioctl", (sqlite3_syscall_ptr)(পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক, পূর্ণাঙ্ক, ...))ioctl, 0 },
#define osIoctl ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক,পূর্ণাঙ্ক,...))aSyscall[28].pCurrent)
# অন্যথা
  { "ioctl",         (sqlite3_syscall_ptr)ioctl,          0 },
#define osIoctl ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক,unsigned long,...))aSyscall[28].pCurrent)
# endif
#অন্যথা
  { "ioctl",         (sqlite3_syscall_ptr)0,              0 },
#endif

}; /* End of the overrideable system calls */


#যদি defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
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
স্থির পূর্ণাঙ্ক unixPosixAdvisoryLocks(
  পূর্ণাঙ্ক fd,        /* The file descriptor to analyze */
  বর্ণ a[16]     /* Write a text description of PALs here */
){
  পূর্ণাঙ্ক in;
  ssize_t n;
  বর্ণ *p, *pNext, *x;
  বর্ণ z[2000];

        /*             1     */
        /*   012 4 678 01234 */
  memcpy(a, "---/-.---.-----", 16);
  sqlite3_snprintf(পরিমাণ(z), z, "/proc/%d/fdinfo/%d", getpid(), fd);
  in = osOpen(z, O_RDONLY, 0);
  যদি( in<0 ){
    প্রত্যাগম SQLITE_ERROR_UNABLE;
  }
  n = osRead(in, z, পরিমাণ(z)-1);
  osClose(in);
  যদি( n<=0 ) প্রত্যাগম SQLITE_ERROR_UNABLE;
  z[n] = 0;

  /* We are looking for lines that begin with "lock:\t".  Examples:
  **
  ** lock: 1: POSIX  ADVISORY  READ 494716 08:02:5277597 1073741826 1073742335
  ** lock: 1: POSIX  ADVISORY  WRITE 494716 08:02:5282282 120 120
  ** lock: 2: POSIX  ADVISORY  READ 494716 08:02:5282282 123 123
  ** lock: 3: POSIX  ADVISORY  READ 494716 08:02:5282282 128 128
  */
  pNext = strstr(z, "lock:\t");
  যাবত্( pNext ){
    বর্ণ cType = 0;
    sqlite3_int64 iFirst, iLast;
    p = pNext+6;
    pNext = strstr(p, "lock:\t");
    যদি( pNext ) pNext[-1] = 0;
    যদি( (x = strstr(p, " READ "))!=0 ){
      cType = 'r';
      x += 6;
    }অন্যথা যদি( (x = strstr(p, " WRITE "))!=0 ){
      cType = 'w';
      x += 7;
    }অন্যথা{
      অনুবর্ত;
    }
    x = strrchr(x, ' ');
    যদি( x==0 ) অনুবর্ত;
    iLast = strtoll(x+1, 0, 10);
    *x = 0;
    x = strrchr(p, ' ');
    যদি( x==0 ) অনুবর্ত;
    iFirst = strtoll(x+1, 0, 10);
    যদি( iLast>=PENDING_BYTE ){
      যদি( iFirst<=PENDING_BYTE && iLast>=PENDING_BYTE )     a[0] = cType;
      যদি( iFirst<=PENDING_BYTE+1 && iLast>=PENDING_BYTE+1 ) a[1] = cType;
      যদি( iFirst<=PENDING_BYTE+2 && iLast>=PENDING_BYTE+510 ) a[2] = cType;
    }অন্যথা যদি( iLast<=128 ){
      যদি( iFirst<=128 && iLast>=128 ) a[4] = cType;
      যদি( iFirst<=120 && iLast>=120 ) a[6] = cType;
      যদি( iFirst<=121 && iLast>=121 ) a[7] = cType;
      যদি( iFirst<=122 && iLast>=122 ) a[8] = cType;
      যদি( iFirst<=123 && iLast>=123 ) a[10] = cType;
      যদি( iFirst<=124 && iLast>=124 ) a[11] = cType;
      যদি( iFirst<=125 && iLast>=125 ) a[12] = cType;
      যদি( iFirst<=126 && iLast>=126 ) a[13] = cType;
      যদি( iFirst<=127 && iLast>=127 ) a[14] = cType;
    }
  }
  প্রত্যাগম SQLITE_OK;
}
#অন্যথা
#  define unixPosixAdvisoryLocks(A,B) SQLITE_ERROR_UNABLE
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

/*
** On some systems, calls to fchown() will trigger a message in a security
** log if they come from non-root processes.  So avoid calling fchown() if
** we are not running as root.
*/
স্থির পূর্ণাঙ্ক robustFchown(পূর্ণাঙ্ক fd, uid_t uid, gid_t gid){
#যদি defined(HAVE_FCHOWN)
  প্রত্যাগম osGeteuid() ? 0 : osFchown(fd,uid,gid);
#অন্যথা
  প্রত্যাগম 0;
#endif
}

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "unix" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
স্থির পূর্ণাঙ্ক unixSetSystemCall(
  sqlite3_vfs *pNotUsed,        /* The VFS pointer.  Not used */
  ধ্রুব বর্ণ *zName,            /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
){
  unsigned পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক rc = SQLITE_NOTFOUND;

  UNUSED_PARAMETER(pNotUsed);
  যদি( zName==0 ){
    /* If no zName is given, restore all system calls to their default
    ** settings and return NULL
    */
    rc = SQLITE_OK;
    প্রতি(i=0; i<পরিমাণ(aSyscall)/পরিমাণ(aSyscall[0]); i++){
      যদি( aSyscall[i].pDefault ){
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  }অন্যথা{
    /* If zName is specified, operate on only the one system call
    ** specified.
    */
    প্রতি(i=0; i<পরিমাণ(aSyscall)/পরিমাণ(aSyscall[0]); i++){
      যদি( strcmp(zName, aSyscall[i].zName)==0 ){
        যদি( aSyscall[i].pDefault==0 ){
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = SQLITE_OK;
        যদি( pNewFunc==0 ) pNewFunc = aSyscall[i].pDefault;
        aSyscall[i].pCurrent = pNewFunc;
        ভঙ্গ;
      }
    }
  }
  প্রত্যাগম rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
স্থির sqlite3_syscall_ptr unixGetSystemCall(
  sqlite3_vfs *pNotUsed,
  ধ্রুব বর্ণ *zName
){
  unsigned পূর্ণাঙ্ক i;

  UNUSED_PARAMETER(pNotUsed);
  প্রতি(i=0; i<পরিমাণ(aSyscall)/পরিমাণ(aSyscall[0]); i++){
    যদি( strcmp(zName, aSyscall[i].zName)==0 ) প্রত্যাগম aSyscall[i].pCurrent;
  }
  প্রত্যাগম 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
স্থির ধ্রুব বর্ণ *unixNextSystemCall(sqlite3_vfs *p, ধ্রুব বর্ণ *zName){
  পূর্ণাঙ্ক i = -1;

  UNUSED_PARAMETER(p);
  যদি( zName ){
    প্রতি(i=0; i<ArraySize(aSyscall)-1; i++){
      যদি( strcmp(zName, aSyscall[i].zName)==0 ) ভঙ্গ;
    }
  }
  প্রতি(i++; i<ArraySize(aSyscall); i++){
    যদি( aSyscall[i].pCurrent!=0 ) প্রত্যাগম aSyscall[i].zName;
  }
  প্রত্যাগম 0;
}

/*
** Do not accept any file descriptor less than this value, in order to avoid
** opening database file using file descriptors that are commonly used for
** standard input, output, and error.
*/
#ifndef SQLITE_MINIMUM_FILE_DESCRIPTOR
# define SQLITE_MINIMUM_FILE_DESCRIPTOR 3
#endif

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
স্থির পূর্ণাঙ্ক robust_open(ধ্রুব বর্ণ *z, পূর্ণাঙ্ক f, mode_t m){
  পূর্ণাঙ্ক fd;
  mode_t m2 = m ? m : SQLITE_DEFAULT_FILE_PERMISSIONS;
  যাবত্(1){
#যদি defined(O_CLOEXEC)
    fd = osOpen(z,f|O_CLOEXEC,m2);
#অন্যথা
    fd = osOpen(z,f,m2);
#endif
    যদি( fd<0 ){
      যদি( errno==EINTR ) অনুবর্ত;
      ভঙ্গ;
    }
    যদি( fd>=SQLITE_MINIMUM_FILE_DESCRIPTOR ) ভঙ্গ;
    যদি( (f & (O_EXCL|O_CREAT))==(O_EXCL|O_CREAT) ){
      (শূন্য)osUnlink(z);
    }
    osClose(fd);
    sqlite3_log(SQLITE_WARNING,
                "attempt to open \"%s\" as file descriptor %d", z, fd);
    fd = -1;
    যদি( osOpen("/dev/null", O_RDONLY, m)<0 ) ভঙ্গ;
  }
  যদি( fd>=0 ){
    যদি( m!=0 ){
      সংরচনা stat statbuf;
      যদি( osFstat(fd, &statbuf)==0
       && statbuf.st_size==0
       && (statbuf.st_mode&0777)!=m
      ){
        osFchmod(fd, m);
      }
    }
#যদি defined(FD_CLOEXEC) && (!defined(O_CLOEXEC) || O_CLOEXEC==0)
    osFcntl(fd, F_SETFD, osFcntl(fd, F_GETFD, 0) | FD_CLOEXEC);
#endif
  }
  প্রত্যাগম fd;
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
স্থির sqlite3_mutex *unixBigLock = 0;
স্থির শূন্য unixEnterMutex(শূন্য){
  প্রতিজ্ঞা( sqlite3_mutex_notheld(unixBigLock) );  /* Not a recursive mutex */
  sqlite3_mutex_enter(unixBigLock);
}
স্থির শূন্য unixLeaveMutex(শূন্য){
  প্রতিজ্ঞা( sqlite3_mutex_held(unixBigLock) );
  sqlite3_mutex_leave(unixBigLock);
}
#ifdef SQLITE_DEBUG
স্থির পূর্ণাঙ্ক unixMutexHeld(শূন্য) {
  প্রত্যাগম sqlite3_mutex_held(unixBigLock);
}
#endif


#ifdef SQLITE_HAVE_OS_TRACE
/*
** Helper function for printing out trace information from debugging
** binaries. This returns the string representation of the supplied
** integer lock-type.
*/
স্থির ধ্রুব বর্ণ *azFileLock(পূর্ণাঙ্ক eFileLock){
  বিকল্প( eFileLock ){
    পক্ষ NO_LOCK: প্রত্যাগম "NONE";
    পক্ষ SHARED_LOCK: প্রত্যাগম "SHARED";
    পক্ষ RESERVED_LOCK: প্রত্যাগম "RESERVED";
    পক্ষ PENDING_LOCK: প্রত্যাগম "PENDING";
    পক্ষ EXCLUSIVE_LOCK: প্রত্যাগম "EXCLUSIVE";
  }
  প্রত্যাগম "ERROR";
}
#endif

#ifdef SQLITE_LOCK_TRACE
/*
** Print out information about all locking operations.
**
** This routine is used for troubleshooting locks on multithreaded
** platforms.  Enable by compiling with the -DSQLITE_LOCK_TRACE
** command-line option on the compiler.  This code is normally
** turned off.
*/
স্থির পূর্ণাঙ্ক lockTrace(পূর্ণাঙ্ক fd, পূর্ণাঙ্ক op, সংরচনা flock *p){
  বর্ণ *zOpName, *zType;
  পূর্ণাঙ্ক s;
  পূর্ণাঙ্ক savedErrno;
  যদি( op==F_GETLK ){
    zOpName = "GETLK";
  }অন্যথা যদি( op==F_SETLK ){
    zOpName = "SETLK";
  }অন্যথা{
    s = osFcntl(fd, op, p);
    sqlite3DebugPrintf("fcntl unknown %d %d %d\n", fd, op, s);
    প্রত্যাগম s;
  }
  যদি( p->l_type==F_RDLCK ){
    zType = "RDLCK";
  }অন্যথা যদি( p->l_type==F_WRLCK ){
    zType = "WRLCK";
  }অন্যথা যদি( p->l_type==F_UNLCK ){
    zType = "UNLCK";
  }অন্যথা{
    প্রতিজ্ঞা( 0 );
  }
  প্রতিজ্ঞা( p->l_whence==SEEK_SET );
  s = osFcntl(fd, op, p);
  savedErrno = errno;
  sqlite3DebugPrintf("fcntl %d %d %s %s %d %d %d %d\n",
     threadid, fd, zOpName, zType, (পূর্ণাঙ্ক)p->l_start, (পূর্ণাঙ্ক)p->l_len,
     (পূর্ণাঙ্ক)p->l_pid, s);
  যদি( s==(-1) && op==F_SETLK && (p->l_type==F_RDLCK || p->l_type==F_WRLCK) ){
    সংরচনা flock l2;
    l2 = *p;
    osFcntl(fd, F_GETLK, &l2);
    যদি( l2.l_type==F_RDLCK ){
      zType = "RDLCK";
    }অন্যথা যদি( l2.l_type==F_WRLCK ){
      zType = "WRLCK";
    }অন্যথা যদি( l2.l_type==F_UNLCK ){
      zType = "UNLCK";
    }অন্যথা{
      প্রতিজ্ঞা( 0 );
    }
    sqlite3DebugPrintf("fcntl-failure-reason: %s %d %d %d\n",
       zType, (পূর্ণাঙ্ক)l2.l_start, (পূর্ণাঙ্ক)l2.l_len, (পূর্ণাঙ্ক)l2.l_pid);
  }
  errno = savedErrno;
  প্রত্যাগম s;
}
#undef osFcntl
#define osFcntl lockTrace
#endif /* SQLITE_LOCK_TRACE */

/*
** Retry ftruncate() calls that fail due to EINTR
**
** All calls to ftruncate() within this file should be made through
** this wrapper.  On the Android platform, bypassing the logic below
** could lead to a corrupt database.
*/
স্থির পূর্ণাঙ্ক robust_ftruncate(পূর্ণাঙ্ক h, sqlite3_int64 sz){
  পূর্ণাঙ্ক rc;
#ifdef __ANDROID__
  /* On Android, ftruncate() always uses 32-bit offsets, even if
  ** _FILE_OFFSET_BITS=64 is defined. This means it is unsafe to attempt to
  ** truncate a file to any size larger than 2GiB. Silently ignore any
  ** such attempts.  */
  যদি( sz>(sqlite3_int64)0x7FFFFFFF ){
    rc = SQLITE_OK;
  }অন্যথা
#endif
  কুরু{ rc = osFtruncate(h,sz); }যাবত্( rc<0 && errno==EINTR );
  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক sqliteErrorFromPosixError(পূর্ণাঙ্ক posixError, পূর্ণাঙ্ক sqliteIOErr) {
  প্রতিজ্ঞা( (sqliteIOErr == SQLITE_IOERR_LOCK) ||
          (sqliteIOErr == SQLITE_IOERR_UNLOCK) ||
          (sqliteIOErr == SQLITE_IOERR_RDLOCK) ||
          (sqliteIOErr == SQLITE_IOERR_CHECKRESERVEDLOCK) );
  বিকল্প (posixError) {
  পক্ষ EACCES:
  পক্ষ EAGAIN:
  পক্ষ ETIMEDOUT:
  পক্ষ EBUSY:
  পক্ষ EINTR:
  পক্ষ ENOLCK:
    /* random NFS retry error, unless during file system support
     * introspection, in which it actually means what it says */
    প্রত্যাগম SQLITE_BUSY;
  
  পক্ষ EPERM:
    প্রত্যাগম SQLITE_PERM;
  
  স্বভাব:
    প্রত্যাগম sqliteIOErr;
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
সংরচনা vxworksFileId {
  সংরচনা vxworksFileId *pNext;  /* Next in a list of them all */
  পূর্ণাঙ্ক nRef;                     /* Number of references to this one */
  পূর্ণাঙ্ক nName;                    /* Length of the zCanonicalName[] string */
  বর্ণ *zCanonicalName;         /* Canonical filename */
};

#যদি OS_VXWORKS
/*
** All unique filenames are held on a linked list headed by this
** variable:
*/
স্থির সংরচনা vxworksFileId *vxworksFileList = 0;
স্থির sqlite3_mutex *vxworksMutex = 0;

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
স্থির পূর্ণাঙ্ক vxworksSimplifyName(বর্ণ *z, পূর্ণাঙ্ক n){
  পূর্ণাঙ্ক i, j;
  যাবত্( n>1 && z[n-1]=='/' ){ n--; }
  প্রতি(i=j=0; i<n; i++){
    যদি( z[i]=='/' ){
      যদি( z[i+1]=='/' ) অনুবর্ত;
      যদি( z[i+1]=='.' && i+2<n && z[i+2]=='/' ){
        i += 1;
        অনুবর্ত;
      }
      যদি( z[i+1]=='.' && i+3<n && z[i+2]=='.' && z[i+3]=='/' ){
        যাবত্( j>0 && z[j-1]!='/' ){ j--; }
        যদি( j>0 ){ j--; }
        i += 2;
        অনুবর্ত;
      }
    }
    z[j++] = z[i];
  }
  z[j] = 0;
  প্রত্যাগম j;
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
স্থির সংরচনা vxworksFileId *vxworksFindFileId(ধ্রুব বর্ণ *zAbsoluteName){
  সংরচনা vxworksFileId *pNew;         /* search key and new file ID */
  সংরচনা vxworksFileId *pCandidate;   /* For looping over existing file IDs */
  পূর্ণাঙ্ক n;                              /* Length of zAbsoluteName string */

  প্রতিজ্ঞা( zAbsoluteName[0]=='/' );
  n = (পূর্ণাঙ্ক)strlen(zAbsoluteName);
  pNew = sqlite3_malloc64( পরিমাণ(*pNew) + (n+1) );
  যদি( pNew==0 ) প্রত্যাগম 0;
  pNew->zCanonicalName = (বর্ণ*)&pNew[1];
  memcpy(pNew->zCanonicalName, zAbsoluteName, n+1);
  n = vxworksSimplifyName(pNew->zCanonicalName, n);

  /* Search for an existing entry that matching the canonical name.
  ** If found, increment the reference count and return a pointer to
  ** the existing file ID.
  */
  sqlite3_mutex_enter(vxworksMutex);
  প্রতি(pCandidate=vxworksFileList; pCandidate; pCandidate=pCandidate->pNext){
    যদি( pCandidate->nName==n
     && memcmp(pCandidate->zCanonicalName, pNew->zCanonicalName, n)==0
    ){
       sqlite3_free(pNew);
       pCandidate->nRef++;
       sqlite3_mutex_leave(vxworksMutex);
       প্রত্যাগম pCandidate;
    }
  }

  /* No match was found.  We will make a new file ID */
  pNew->nRef = 1;
  pNew->nName = n;
  pNew->pNext = vxworksFileList;
  vxworksFileList = pNew;
  sqlite3_mutex_leave(vxworksMutex);
  প্রত্যাগম pNew;
}

/*
** Decrement the reference count on a vxworksFileId object.  Free
** the object when the reference count reaches zero.
*/
স্থির শূন্য vxworksReleaseFileId(সংরচনা vxworksFileId *pId){
  sqlite3_mutex_enter(vxworksMutex);
  প্রতিজ্ঞা( pId->nRef>0 );
  pId->nRef--;
  যদি( pId->nRef==0 ){
    সংরচনা vxworksFileId **pp;
    প্রতি(pp=&vxworksFileList; *pp && *pp!=pId; pp = &((*pp)->pNext)){}
    প্রতিজ্ঞা( *pp==pId );
    *pp = pId->pNext;
    sqlite3_free(pId);
  }
  sqlite3_mutex_leave(vxworksMutex);
}
#endif /* OS_VXWORKS */
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
সংরচনা unixFileId {
  dev_t dev;                  /* Device number */
#যদি OS_VXWORKS
  সংরচনা vxworksFileId *pId;  /* Unique file ID for vxworks. */
#অন্যথা
  /* We are told that some versions of Android contain a bug that
  ** sizes ino_t at only 32-bits instead of 64-bits. (See
  ** https://android-review.googlesource.com/#/c/115351/3/dist/sqlite3.c)
  ** To work around this, always allocate 64-bits for the inode number.
  ** On small machines that only have 32-bit inodes, this wastes 4 bytes,
  ** but that should not be a big deal. */
  /* WAS:  ino_t ino;   */
  u64 ino;                   /* Inode number */
#endif
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
সংরচনা unixInodeInfo {
  সংরচনা unixFileId fileId;       /* The lookup key */
  sqlite3_mutex *pLockMutex;      /* Hold this mutex for... */
  পূর্ণাঙ্ক nShared;                      /* Number of SHARED locks held */
  পূর্ণাঙ্ক nLock;                        /* Number of outstanding file locks */
  unsigned বর্ণ eFileLock;          /* One of SHARED_LOCK, RESERVED_LOCK etc. */
  unsigned বর্ণ bProcessLock;       /* An exclusive process lock is held */
  UnixUnusedFd *pUnused;            /* Unused file descriptors to close */
  পূর্ণাঙ্ক nRef;                       /* Number of pointers to this structure */
  unix_Shm_গ্রন্থি *p_Shm_গ্রন্থি;          /* Shared memory associated with this inode */
  unixInodeInfo *pNext;           /* List of all unixInodeInfo objects */
  unixInodeInfo *pPrev;           /*    .... doubly linked */
#যদি SQLITE_ENABLE_LOCKING_STYLE
  unsigned long long sharedByte;  /* for AFP simulated shared lock */
#endif
#যদি OS_VXWORKS
  sem_t *pSem;                    /* Named POSIX semaphore */
  বর্ণ aSemName[MAX_PATHNAME+2];  /* Name of that semaphore */
#endif
};

/*
** A lists of all unixInodeInfo objects.
**
** Must hold unixBigLock in order to read or write this variable.
*/
স্থির unixInodeInfo *inodeList = 0;  /* All unixInodeInfo objects */

#ifdef SQLITE_DEBUG
/*
** True if the inode mutex (on the unixFile.pFileMutex field) is held, or not.
** This routine is used only within assert() to help verify correct mutex
** usage.
*/
পূর্ণাঙ্ক unixFileMutexHeld(unixFile *pFile){
  প্রতিজ্ঞা( pFile->pInode );
  প্রত্যাগম sqlite3_mutex_held(pFile->pInode->pLockMutex);
}
পূর্ণাঙ্ক unixFileMutexNotheld(unixFile *pFile){
  প্রতিজ্ঞা( pFile->pInode );
  প্রত্যাগম sqlite3_mutex_notheld(pFile->pInode->pLockMutex);
}
#endif

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
#define unixLogError(a,b,c)     unix_Log_Error_At_সারি(a,b,c,__LINE__)
স্থির পূর্ণাঙ্ক unix_Log_Error_At_সারি(
  পূর্ণাঙ্ক errcode,                    /* SQLite error code */
  ধ্রুব বর্ণ *zFunc,              /* Name of OS function that failed */
  ধ্রুব বর্ণ *zPath,              /* File path associated with error */
  পূর্ণাঙ্ক i_সারি                       /* Source line number where error occurred */
){
  বর্ণ *zErr;                     /* Message from strerror() or equivalent */
  পূর্ণাঙ্ক iErrno = errno;             /* Saved syscall error number */

  /* If this is not a threadsafe build (SQLITE_THREADSAFE==0), then use
  ** the strerror() function to obtain the human-readable error message
  ** equivalent to errno. Otherwise, use strerror_r().
  */
#যদি SQLITE_THREADSAFE && defined(HAVE_STRERROR_R)
  বর্ণ aErr[80];
  memset(aErr, 0, পরিমাণ(aErr));
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
#যদি (defined(STRERROR_R_CHAR_P) || defined(__USE_GNU)) \
  && !defined(ANDROID) && !defined(__ANDROID__)
  zErr =
# endif
  strerror_r(iErrno, aErr, পরিমাণ(aErr)-1);

#নোচেদ্যদি SQLITE_THREADSAFE
  /* This is a threadsafe build, but strerror_r() is not available. */
  zErr = "";
#অন্যথা
  /* Non-threadsafe build, use strerror(). */
  zErr = strerror(iErrno);
#endif

  যদি( zPath==0 ) zPath = "";
  sqlite3_log(errcode,
      "os_unix.c:%d: (%d) %s(%s) - %s",
      i_সারি, iErrno, zFunc, zPath, zErr
  );

  প্রত্যাগম errcode;
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
স্থির শূন্য robust_close(unixFile *pFile, পূর্ণাঙ্ক h, পূর্ণাঙ্ক lineno){
  যদি( osClose(h) ){
    unix_Log_Error_At_সারি(SQLITE_IOERR_CLOSE, "close",
                       pFile ? pFile->zPath : 0, lineno);
  }
}

/*
** Set the pFile->lastErrno.  Do this in a subroutine as that provides
** a convenient place to set a breakpoint.
*/
স্থির শূন্য storeLastErrno(unixFile *pFile, পূর্ণাঙ্ক error){
  pFile->lastErrno = error;
}

/*
** Close all file descriptors accumulated in the unixInodeInfo->pUnused list.
*/
স্থির শূন্য closePendingFds(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  UnixUnusedFd *p;
  UnixUnusedFd *pNext;
  প্রতিজ্ঞা( unixFileMutexHeld(pFile) );
  প্রতি(p=pInode->pUnused; p; p=pNext){
    pNext = p->pNext;
    robust_close(pFile, p->fd, __LINE__);
    sqlite3_free(p);
  }
  pInode->pUnused = 0;
}

/*
** Release a unixInodeInfo structure previously allocated by findInodeInfo().
**
** The global mutex must be held when this routine is called, but the mutex
** on the inode being deleted must NOT be held.
*/
স্থির শূন্য releaseInodeInfo(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  প্রতিজ্ঞা( unixMutexHeld() );
  প্রতিজ্ঞা( unixFileMutexNotheld(pFile) );
  যদি( ALWAYS(pInode) ){
    pInode->nRef--;
    যদি( pInode->nRef==0 ){
      প্রতিজ্ঞা( pInode->p_Shm_গ্রন্থি==0 );
      sqlite3_mutex_enter(pInode->pLockMutex);
      closePendingFds(pFile);
      sqlite3_mutex_leave(pInode->pLockMutex);
      যদি( pInode->pPrev ){
        প্রতিজ্ঞা( pInode->pPrev->pNext==pInode );
        pInode->pPrev->pNext = pInode->pNext;
      }অন্যথা{
        প্রতিজ্ঞা( inodeList==pInode );
        inodeList = pInode->pNext;
      }
      যদি( pInode->pNext ){
        প্রতিজ্ঞা( pInode->pNext->pPrev==pInode );
        pInode->pNext->pPrev = pInode->pPrev;
      }
      sqlite3_mutex_free(pInode->pLockMutex);
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
স্থির পূর্ণাঙ্ক findInodeInfo(
  unixFile *pFile,               /* Unix file with file desc used in the key */
  unixInodeInfo **ppInode        /* Return the unixInodeInfo object here */
){
  পূর্ণাঙ্ক rc;                        /* System call return code */
  পূর্ণাঙ্ক fd;                        /* The file descriptor for pFile */
  সংরচনা unixFileId fileId;      /* Lookup key for the unixInodeInfo */
  সংরচনা stat statbuf;           /* Low-level file information */
  unixInodeInfo *pInode = 0;     /* Candidate unixInodeInfo object */

  প্রতিজ্ঞা( unixMutexHeld() );

  /* Get low-level information about the file that we can used to
  ** create a unique name for the file.
  */
  fd = pFile->h;
  rc = osFstat(fd, &statbuf);
  যদি( rc!=0 ){
    storeLastErrno(pFile, errno);
#যদি defined(EOVERFLOW) && defined(SQLITE_DISABLE_LFS)
    যদি( pFile->lastErrno==EOVERFLOW ) প্রত্যাগম SQLITE_NOLFS;
#endif
    প্রত্যাগম SQLITE_IOERR;
  }

#ifdef __APPLE__
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
  যদি( statbuf.st_size==0 && (pFile->fsFlags & SQLITE_FSFLAGS_IS_MSDOS)!=0 ){
    কুরু{ rc = osWrite(fd, "S", 1); }যাবত্( rc<0 && errno==EINTR );
    যদি( rc!=1 ){
      storeLastErrno(pFile, errno);
      প্রত্যাগম SQLITE_IOERR;
    }
    যদি( fsync(fd) ){
      storeLastErrno(pFile, errno);
      প্রত্যাগম SQLITE_IOERR_FSYNC;
    }
    rc = osFstat(fd, &statbuf);
    যদি( rc!=0 ){
      storeLastErrno(pFile, errno);
      প্রত্যাগম SQLITE_IOERR;
    }
  }
#endif

  memset(&fileId, 0, পরিমাণ(fileId));
  fileId.dev = statbuf.st_dev;
#যদি OS_VXWORKS
  fileId.pId = pFile->pId;
#অন্যথা
  fileId.ino = (u64)statbuf.st_ino;
#endif
  প্রতিজ্ঞা( unixMutexHeld() );
  pInode = inodeList;
  যাবত্( pInode && memcmp(&fileId, &pInode->fileId, পরিমাণ(fileId)) ){
    pInode = pInode->pNext;
  }
  যদি( pInode==0 ){
    pInode = sqlite3_malloc64( পরিমাণ(*pInode) );
    যদি( pInode==0 ){
      প্রত্যাগম SQLITE_NOMEM_BKPT;
    }
    memset(pInode, 0, পরিমাণ(*pInode));
    memcpy(&pInode->fileId, &fileId, পরিমাণ(fileId));
    যদি( sqlite3GlobalConfig.bCoreMutex ){
      pInode->pLockMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      যদি( pInode->pLockMutex==0 ){
        sqlite3_free(pInode);
        প্রত্যাগম SQLITE_NOMEM_BKPT;
      }
    }
    pInode->nRef = 1;
    প্রতিজ্ঞা( unixMutexHeld() );
    pInode->pNext = inodeList;
    pInode->pPrev = 0;
    যদি( inodeList ) inodeList->pPrev = pInode;
    inodeList = pInode;
  }অন্যথা{
    pInode->nRef++;
  }
  *ppInode = pInode;
  প্রত্যাগম SQLITE_OK;
}

/*
** Return TRUE if pFile has been renamed or unlinked since it was first opened.
*/
স্থির পূর্ণাঙ্ক fileHasMoved(unixFile *pFile){
#যদি OS_VXWORKS
  প্রত্যাগম pFile->pInode!=0 && pFile->pId!=pFile->pInode->fileId.pId;
#অন্যথা
  সংরচনা stat buf;
  প্রত্যাগম pFile->pInode!=0 &&
      (osStat(pFile->zPath, &buf)!=0
         || (u64)buf.st_ino!=pFile->pInode->fileId.ino);
#endif
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
স্থির শূন্য verifyDbFile(unixFile *pFile){
  সংরচনা stat buf;
  পূর্ণাঙ্ক rc;

  /* These verifications occurs for the main database only */
  যদি( pFile->ctrlFlags & UNIXFILE_NOLOCK ) প্রত্যাগম;

  rc = osFstat(pFile->h, &buf);
  যদি( rc!=0 ){
    sqlite3_log(SQLITE_WARNING, "cannot fstat db file %s", pFile->zPath);
    প্রত্যাগম;
  }
  যদি( buf.st_nlink==0 ){
    sqlite3_log(SQLITE_WARNING, "file unlinked while open: %s", pFile->zPath);
    প্রত্যাগম;
  }
  যদি( buf.st_nlink>1 ){
    sqlite3_log(SQLITE_WARNING, "multiple links to file: %s", pFile->zPath);
    প্রত্যাগম;
  }
  যদি( fileHasMoved(pFile) ){
    sqlite3_log(SQLITE_WARNING, "file renamed while open: %s", pFile->zPath);
    প্রত্যাগম;
  }
}


/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
স্থির পূর্ণাঙ্ক unixCheckReservedLock(sqlite3_file *id, পূর্ণাঙ্ক *pResOut){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক reserved = 0;
  unixFile *pFile = (unixFile*)id;

  SimulateIOError( প্রত্যাগম SQLITE_IOERR_CHECKRESERVEDLOCK; );

  প্রতিজ্ঞা( pFile );
  প্রতিজ্ঞা( pFile->eFileLock<=SHARED_LOCK );
  sqlite3_mutex_enter(pFile->pInode->pLockMutex);

  /* Check if a thread in this process holds such a lock */
  যদি( pFile->pInode->eFileLock>SHARED_LOCK ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it.
  */
#ifndef __DJGPP__
  যদি( !reserved && !pFile->pInode->bProcessLock ){
    সংরচনা flock lock;
    lock.l_whence = SEEK_SET;
    lock.l_start = RESERVED_BYTE;
    lock.l_len = 1;
    lock.l_type = F_WRLCK;
    যদি( osFcntl(pFile->h, F_GETLK, &lock) ){
      rc = SQLITE_IOERR_CHECKRESERVEDLOCK;
      storeLastErrno(pFile, errno);
    } অন্যথা যদি( lock.l_type!=F_UNLCK ){
      reserved = 1;
    }
  }
#endif

  sqlite3_mutex_leave(pFile->pInode->pLockMutex);
  OSTRACE(("TEST WR-LOCK %d %d %d (unix)\n", pFile->h, rc, reserved));

  *pResOut = reserved;
  প্রত্যাগম rc;
}

/* Forward declaration*/
স্থির পূর্ণাঙ্ক unixSleep(sqlite3_vfs*,পূর্ণাঙ্ক);

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
#ifndef SQLITE_ENABLE_SETLK_TIMEOUT
# define osSetPosixAdvisoryLock(h,x,t) osFcntl(h,F_SETLK,x)
#অন্যথা
স্থির পূর্ণাঙ্ক osSetPosixAdvisoryLock(
  পূর্ণাঙ্ক h,                /* The file descriptor on which to take the lock */
  সংরচনা flock *pLock,  /* The description of the lock */
  unixFile *pFile       /* Structure holding timeout value */
){
  পূর্ণাঙ্ক rc = 0;

  যদি( pFile->iBusyTimeout==0 ){
    /* unixFile->iBusyTimeout is set to 0. In this case, attempt a 
    ** non-blocking lock. */
    rc = osFcntl(h,F_SETLK,pLock);
  }অন্যথা{
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
#ifdef SQLITE_TEST
    পূর্ণাঙ্ক tm = pFile->iBusyTimeout;
    যাবত্( tm>0 ){
      rc = osFcntl(h,F_SETLK,pLock);
      যদি( rc==0 ) ভঙ্গ;
      unixSleep(0,1000);
      tm--;
    }
#অন্যথা
    rc = osFcntl(h,F_SETLK,pLock);
    pFile->iBusyTimeout = 0;
#endif
    /* End of code to replace with real blocking-locks code. */
  }

  প্রত্যাগম rc;
}
#endif /* SQLITE_ENABLE_SETLK_TIMEOUT */


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
স্থির পূর্ণাঙ্ক unixFileLock(unixFile *pFile, সংরচনা flock *pLock){
  পূর্ণাঙ্ক rc;
  unixInodeInfo *pInode = pFile->pInode;
  প্রতিজ্ঞা( pInode!=0 );
  প্রতিজ্ঞা( sqlite3_mutex_held(pInode->pLockMutex) );
  যদি( (pFile->ctrlFlags & (UNIXFILE_EXCL|UNIXFILE_RDONLY))==UNIXFILE_EXCL ){
    যদি( pInode->bProcessLock==0 ){
      সংরচনা flock lock;
      /* assert( pInode->nLock==0 ); <-- Not true if unix-excl READONLY used */
      lock.l_whence = SEEK_SET;
      lock.l_start = SHARED_FIRST;
      lock.l_len = SHARED_SIZE;
      lock.l_type = F_WRLCK;
      rc = osSetPosixAdvisoryLock(pFile->h, &lock, pFile);
      যদি( rc<0 ) প্রত্যাগম rc;
      pInode->bProcessLock = 1;
      pInode->nLock++;
    }অন্যথা{
      rc = 0;
    }
  }অন্যথা{
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    যদি( pFile->bBlockOnConnect && pLock->l_type==F_RDLCK
     && pLock->l_start==SHARED_FIRST && pLock->l_len==SHARED_SIZE
    ){
      rc = osFcntl(pFile->h, F_SETLKW, pLock);
    }অন্যথা
#endif
    rc = osSetPosixAdvisoryLock(pFile->h, pLock, pFile);
  }
  প্রত্যাগম rc;
}

#যদি !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
/* Forward reference */
স্থির পূর্ণাঙ্ক unix_Is_Sharing_Shm_গ্রন্থি(unixFile*);
#অন্যথা
#define unix_Is_Sharing_Shm_গ্রন্থি(pFile) (0)
#endif

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
স্থির পূর্ণাঙ্ক unixLock(sqlite3_file *id, পূর্ণাঙ্ক eFileLock){
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
  পূর্ণাঙ্ক rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  সংরচনা flock lock;
  পূর্ণাঙ্ক tErrno = 0;

  প্রতিজ্ঞা( pFile );
  OSTRACE(("LOCK    %d %s was %s(%s,%d) pid=%d (unix)\n", pFile->h,
      azFileLock(eFileLock), azFileLock(pFile->eFileLock),
      azFileLock(pFile->pInode->eFileLock), pFile->pInode->nShared,
      osGetpid(0)));

  /* If there is already a lock of this type or more restrictive on the
  ** unixFile, do nothing. Don't use the end_lock: exit path, as
  ** unixEnterMutex() hasn't been called yet.
  */
  যদি( pFile->eFileLock>=eFileLock ){
    OSTRACE(("LOCK    %d %s ok (already held) (unix)\n", pFile->h,
            azFileLock(eFileLock)));
    প্রত্যাগম SQLITE_OK;
  }

  /* Make sure the locking sequence is correct.
  **  (1) We never move from unlocked to anything higher than shared lock.
  **  (2) SQLite never explicitly requests a pending lock.
  **  (3) A shared lock is always held when a reserve lock is requested.
  */
  প্রতিজ্ঞা( pFile->eFileLock!=NO_LOCK || eFileLock==SHARED_LOCK );
  প্রতিজ্ঞা( eFileLock!=PENDING_LOCK );
  প্রতিজ্ঞা( eFileLock!=RESERVED_LOCK || pFile->eFileLock==SHARED_LOCK );

  /* This mutex is needed because pFile->pInode is shared across threads
  */
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->pLockMutex);

  /* If some thread using this PID has a lock via a different unixFile*
  ** handle that precludes the requested lock, return BUSY.
  */
  যদি( (pFile->eFileLock!=pInode->eFileLock &&
          (pInode->eFileLock>=PENDING_LOCK || eFileLock>SHARED_LOCK))
  ){
    rc = SQLITE_BUSY;
    গচ্ছ end_lock;
  }

  /* If a SHARED lock is requested, and some thread using this PID already
  ** has a SHARED or RESERVED lock, then increment reference counts and
  ** return SQLITE_OK.
  */
  যদি( eFileLock==SHARED_LOCK &&
      (pInode->eFileLock==SHARED_LOCK || pInode->eFileLock==RESERVED_LOCK) ){
    প্রতিজ্ঞা( eFileLock==SHARED_LOCK );
    প্রতিজ্ঞা( pFile->eFileLock==0 );
    প্রতিজ্ঞা( pInode->nShared>0 );
    pFile->eFileLock = SHARED_LOCK;
    pInode->nShared++;
    pInode->nLock++;
    গচ্ছ end_lock;
  }


  /* A PENDING lock is needed before acquiring a SHARED lock and before
  ** acquiring an EXCLUSIVE lock.  For the SHARED lock, the PENDING will
  ** be released.
  */
  lock.l_len = 1L;
  lock.l_whence = SEEK_SET;
  যদি( eFileLock==SHARED_LOCK
   || (eFileLock==EXCLUSIVE_LOCK && pFile->eFileLock==RESERVED_LOCK)
  ){
    lock.l_type = (eFileLock==SHARED_LOCK?F_RDLCK:F_WRLCK);
    lock.l_start = PENDING_BYTE;
    যদি( unixFileLock(pFile, &lock) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_LOCK);
      যদি( rc!=SQLITE_BUSY ){
        storeLastErrno(pFile, tErrno);
      }
      গচ্ছ end_lock;
    }অন্যথা যদি( eFileLock==EXCLUSIVE_LOCK ){
      pFile->eFileLock = PENDING_LOCK;
      pInode->eFileLock = PENDING_LOCK;
    }
  }


  /* If control gets to this point, then actually go ahead and make
  ** operating system calls for the specified lock.
  */
  যদি( eFileLock==SHARED_LOCK ){
    প্রতিজ্ঞা( pInode->nShared==0 );
    প্রতিজ্ঞা( pInode->eFileLock==0 );
    প্রতিজ্ঞা( rc==SQLITE_OK );

    /* Now get the read-lock */
    lock.l_start = SHARED_FIRST;
    lock.l_len = SHARED_SIZE;
    যদি( unixFileLock(pFile, &lock) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_LOCK);
    }

    /* Drop the temporary PENDING lock */
    lock.l_start = PENDING_BYTE;
    lock.l_len = 1L;
    lock.l_type = F_UNLCK;
    যদি( unixFileLock(pFile, &lock) && rc==SQLITE_OK ){
      /* This could happen with a network mount */
      tErrno = errno;
      rc = SQLITE_IOERR_UNLOCK;
    }

    যদি( rc ){
      যদি( rc!=SQLITE_BUSY ){
        storeLastErrno(pFile, tErrno);
      }
      গচ্ছ end_lock;
    }অন্যথা{
      pFile->eFileLock = SHARED_LOCK;
      pInode->nLock++;
      pInode->nShared = 1;
    }
  }অন্যথা যদি( eFileLock==EXCLUSIVE_LOCK && pInode->nShared>1 ){
    /* We are trying for an exclusive lock but another thread in this
    ** same process is still holding a shared lock. */
    rc = SQLITE_BUSY;
  }অন্যথা যদি( unix_Is_Sharing_Shm_গ্রন্থি(pFile) ){
    /* We are in WAL mode and attempting to delete the SHM and WAL
    ** files due to closing the connection or changing out of WAL mode,
    ** but another process still holds locks on the SHM file, thus
    ** indicating that database locks have been broken, perhaps due
    ** to a rogue close(open(dbFile)) or similar.
    */
    rc = SQLITE_BUSY;
  }অন্যথা{
    /* The request was for a RESERVED or EXCLUSIVE lock.  It is
    ** assumed that there is a SHARED or greater lock on the file
    ** already.
    */
    প্রতিজ্ঞা( 0!=pFile->eFileLock );
    lock.l_type = F_WRLCK;

    প্রতিজ্ঞা( eFileLock==RESERVED_LOCK || eFileLock==EXCLUSIVE_LOCK );
    যদি( eFileLock==RESERVED_LOCK ){
      lock.l_start = RESERVED_BYTE;
      lock.l_len = 1L;
    }অন্যথা{
      lock.l_start = SHARED_FIRST;
      lock.l_len = SHARED_SIZE;
    }

    যদি( unixFileLock(pFile, &lock) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_LOCK);
      যদি( rc!=SQLITE_BUSY ){
        storeLastErrno(pFile, tErrno);
      }
    }
  }


#ifdef SQLITE_DEBUG
  /* Set up the transaction-counter change checking flags when
  ** transitioning from a SHARED to a RESERVED lock.  The change
  ** from SHARED to RESERVED marks the beginning of a normal
  ** write operation (not a hot journal rollback).
  */
  যদি( rc==SQLITE_OK
   && pFile->eFileLock<=SHARED_LOCK
   && eFileLock==RESERVED_LOCK
  ){
    pFile->transCntrChng = 0;
    pFile->dbUpdate = 0;
    pFile->inNormalWrite = 1;
  }
#endif

  যদি( rc==SQLITE_OK ){
    pFile->eFileLock = eFileLock;
    pInode->eFileLock = eFileLock;
  }

end_lock:
  sqlite3_mutex_leave(pInode->pLockMutex);
  OSTRACE(("LOCK    %d %s %s (unix)\n", pFile->h, azFileLock(eFileLock),
      rc==SQLITE_OK ? "ok" : "failed"));
  প্রত্যাগম rc;
}

/*
** Add the file descriptor used by file handle pFile to the corresponding
** pUnused list.
*/
স্থির শূন্য setPendingFd(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  UnixUnusedFd *p = pFile->pPreallocatedUnused;
  প্রতিজ্ঞা( unixFileMutexHeld(pFile) );
  p->pNext = pInode->pUnused;
  pInode->pUnused = p;
  pFile->h = -1;
  pFile->pPreallocatedUnused = 0;
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
স্থির পূর্ণাঙ্ক posixUnlock(sqlite3_file *id, পূর্ণাঙ্ক eFileLock, পূর্ণাঙ্ক handleNFSUnlock){
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  সংরচনা flock lock;
  পূর্ণাঙ্ক rc = SQLITE_OK;

  প্রতিজ্ঞা( pFile );
  OSTRACE(("UNLOCK  %d %d was %d(%d,%d) pid=%d (unix)\n", pFile->h, eFileLock,
      pFile->eFileLock, pFile->pInode->eFileLock, pFile->pInode->nShared,
      osGetpid(0)));

  প্রতিজ্ঞা( eFileLock<=SHARED_LOCK );
  যদি( pFile->eFileLock<=eFileLock ){
    প্রত্যাগম SQLITE_OK;
  }
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->pLockMutex);
  প্রতিজ্ঞা( pInode->nShared!=0 );
  যদি( pFile->eFileLock>SHARED_LOCK ){
    প্রতিজ্ঞা( pInode->eFileLock==pFile->eFileLock );

#ifdef SQLITE_DEBUG
    /* When reducing a lock such that other processes can start
    ** reading the database file again, make sure that the
    ** transaction counter was updated if any part of the database
    ** file changed.  If the transaction counter is not updated,
    ** other connections to the same file might not realize that
    ** the file has changed and hence might not know to flush their
    ** cache.  The use of a stale cache can lead to database corruption.
    */
    pFile->inNormalWrite = 0;
#endif

    /* downgrading to a shared lock on NFS involves clearing the write lock
    ** before establishing the readlock - to avoid a race condition we downgrade
    ** the lock in 2 blocks, so that part of the range will be covered by a
    ** write lock until the rest is covered by a read lock:
    **  1:   [WWWWW]
    **  2:   [....W]
    **  3:   [RRRRW]
    **  4:   [RRRR.]
    */
    যদি( eFileLock==SHARED_LOCK ){
#যদি !defined(__APPLE__) || !SQLITE_ENABLE_LOCKING_STYLE
      (শূন্য)handleNFSUnlock;
      প্রতিজ্ঞা( handleNFSUnlock==0 );
#endif
#যদি defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
      যদি( handleNFSUnlock ){
        পূর্ণাঙ্ক tErrno;               /* Error code from system call errors */
        off_t divSize = SHARED_SIZE - 1;
      
        lock.l_type = F_UNLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = SHARED_FIRST;
        lock.l_len = divSize;
        যদি( unixFileLock(pFile, &lock)==(-1) ){
          tErrno = errno;
          rc = SQLITE_IOERR_UNLOCK;
          storeLastErrno(pFile, tErrno);
          গচ্ছ end_unlock;
        }
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = SHARED_FIRST;
        lock.l_len = divSize;
        যদি( unixFileLock(pFile, &lock)==(-1) ){
          tErrno = errno;
          rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_RDLOCK);
          যদি( IS_LOCK_ERROR(rc) ){
            storeLastErrno(pFile, tErrno);
          }
          গচ্ছ end_unlock;
        }
        lock.l_type = F_UNLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = SHARED_FIRST+divSize;
        lock.l_len = SHARED_SIZE-divSize;
        যদি( unixFileLock(pFile, &lock)==(-1) ){
          tErrno = errno;
          rc = SQLITE_IOERR_UNLOCK;
          storeLastErrno(pFile, tErrno);
          গচ্ছ end_unlock;
        }
      }অন্যথা
#endif /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
      {
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = SHARED_FIRST;
        lock.l_len = SHARED_SIZE;
        যদি( unixFileLock(pFile, &lock) ){
          /* In theory, the call to unixFileLock() cannot fail because another
          ** process is holding an incompatible lock. If it does, this
          ** indicates that the other process is not following the locking
          ** protocol. If this happens, return SQLITE_IOERR_RDLOCK. Returning
          ** SQLITE_BUSY would confuse the upper layer (in practice it causes
          ** an assert to fail). */
          rc = SQLITE_IOERR_RDLOCK;
          storeLastErrno(pFile, errno);
          গচ্ছ end_unlock;
        }
      }
    }
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = PENDING_BYTE;
    lock.l_len = 2L;  প্রতিজ্ঞা( PENDING_BYTE+1==RESERVED_BYTE );
    যদি( unixFileLock(pFile, &lock)==0 ){
      pInode->eFileLock = SHARED_LOCK;
    }অন্যথা{
      rc = SQLITE_IOERR_UNLOCK;
      storeLastErrno(pFile, errno);
      গচ্ছ end_unlock;
    }
  }
  যদি( eFileLock==NO_LOCK ){
    /* Decrement the shared lock counter.  Release the lock using an
    ** OS call only when all threads in this same process have released
    ** the lock.
    */
    pInode->nShared--;
    যদি( pInode->nShared==0 ){
      lock.l_type = F_UNLCK;
      lock.l_whence = SEEK_SET;
      lock.l_start = lock.l_len = 0L;
      যদি( unixFileLock(pFile, &lock)==0 ){
        pInode->eFileLock = NO_LOCK;
      }অন্যথা{
        rc = SQLITE_IOERR_UNLOCK;
        storeLastErrno(pFile, errno);
        pInode->eFileLock = NO_LOCK;
        pFile->eFileLock = NO_LOCK;
      }
    }

    /* Decrement the count of locks against this same file.  When the
    ** count reaches zero, close any other file descriptors whose close
    ** was deferred because of outstanding locks.
    */
    pInode->nLock--;
    প্রতিজ্ঞা( pInode->nLock>=0 );
    যদি( pInode->nLock==0 ) closePendingFds(pFile);
  }

end_unlock:
  sqlite3_mutex_leave(pInode->pLockMutex);
  যদি( rc==SQLITE_OK ){
    pFile->eFileLock = eFileLock;
  }
  প্রত্যাগম rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
স্থির পূর্ণাঙ্ক unixUnlock(sqlite3_file *id, পূর্ণাঙ্ক eFileLock){
#যদি SQLITE_MAX_MMAP_SIZE>0
  প্রতিজ্ঞা( eFileLock==SHARED_LOCK || ((unixFile *)id)->nFetchOut==0 );
#endif
  প্রত্যাগম posixUnlock(id, eFileLock, 0);
}

#যদি SQLITE_MAX_MMAP_SIZE>0
স্থির পূর্ণাঙ্ক unixMapfile(unixFile *pFd, i64 nByte);
স্থির শূন্য unixUnmapfile(unixFile *pFd);
#endif

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
স্থির পূর্ণাঙ্ক closeUnixFile(sqlite3_file *id){
  unixFile *pFile = (unixFile*)id;
#যদি SQLITE_MAX_MMAP_SIZE>0
  unixUnmapfile(pFile);
#endif
  যদি( pFile->h>=0 ){
    robust_close(pFile, pFile->h, __LINE__);
    pFile->h = -1;
  }
#যদি OS_VXWORKS
  যদি( pFile->pId ){
    যদি( pFile->ctrlFlags & UNIXFILE_DELETE ){
      osUnlink(pFile->pId->zCanonicalName);
    }
    vxworksReleaseFileId(pFile->pId);
    pFile->pId = 0;
  }
#endif
#ifdef SQLITE_UNLINK_AFTER_CLOSE
  যদি( pFile->ctrlFlags & UNIXFILE_DELETE ){
    osUnlink(pFile->zPath);
    sqlite3_free(*(বর্ণ**)&pFile->zPath);
    pFile->zPath = 0;
  }
#endif
  OSTRACE(("CLOSE   %-3d\n", pFile->h));
  OpenCounter(-1);
  sqlite3_free(pFile->pPreallocatedUnused);
  memset(pFile, 0, পরিমাণ(unixFile));
  প্রত্যাগম SQLITE_OK;
}

/*
** Close a file.
*/
স্থির পূর্ণাঙ্ক unixClose(sqlite3_file *id){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  unixFile *pFile = (unixFile *)id;
  unixInodeInfo *pInode = pFile->pInode;

  প্রতিজ্ঞা( pInode!=0 );
  verifyDbFile(pFile);
  unixUnlock(id, NO_LOCK);
  প্রতিজ্ঞা( unixFileMutexNotheld(pFile) );
  unixEnterMutex();

  /* unixFile.pInode is always valid here. Otherwise, a different close
  ** routine (e.g. nolockClose()) would be called instead.
  */
  প্রতিজ্ঞা( pFile->pInode->nLock>0 || pFile->pInode->bProcessLock==0 );
  sqlite3_mutex_enter(pInode->pLockMutex);
  যদি( pInode->nLock ){
    /* If there are outstanding locks, do not actually close the file just
    ** yet because that would clear those locks.  Instead, add the file
    ** descriptor to pInode->pUnused list.  It will be automatically closed
    ** when the last lock is cleared.
    */
    setPendingFd(pFile);
  }
  sqlite3_mutex_leave(pInode->pLockMutex);
  releaseInodeInfo(pFile);
  প্রতিজ্ঞা( pFile->pShm==0 );
  rc = closeUnixFile(id);
  unixLeaveMutex();
  প্রত্যাগম rc;
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

স্থির পূর্ণাঙ্ক nolockCheckReservedLock(sqlite3_file *NotUsed, পূর্ণাঙ্ক *pResOut){
  UNUSED_PARAMETER(NotUsed);
  *pResOut = 0;
  প্রত্যাগম SQLITE_OK;
}
স্থির পূর্ণাঙ্ক nolockLock(sqlite3_file *NotUsed, পূর্ণাঙ্ক NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  প্রত্যাগম SQLITE_OK;
}
স্থির পূর্ণাঙ্ক nolockUnlock(sqlite3_file *NotUsed, পূর্ণাঙ্ক NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  প্রত্যাগম SQLITE_OK;
}

/*
** Close the file.
*/
স্থির পূর্ণাঙ্ক nolockClose(sqlite3_file *id) {
  প্রত্যাগম closeUnixFile(id);
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
#define DOTLOCK_SUFFIX ".lock"

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If the caller holds a SHARED
** or greater lock when it is called, then it is assumed that no other
** client may hold RESERVED. Or, if the caller holds no lock, then it
** is assumed another client holds RESERVED if the lock-file exists.
*/
স্থির পূর্ণাঙ্ক dotlockCheckReservedLock(sqlite3_file *id, পূর্ণাঙ্ক *pResOut) {
  unixFile *pFile = (unixFile*)id;
  SimulateIOError( প্রত্যাগম SQLITE_IOERR_CHECKRESERVEDLOCK; );

  যদি( pFile->eFileLock>=SHARED_LOCK ){
    *pResOut = 0;
  }অন্যথা{
    *pResOut = osAccess((ধ্রুব বর্ণ*)pFile->lockingContext, 0)==0;
  }
  OSTRACE(("TEST WR-LOCK %d %d %d (dotlock)\n", pFile->h, 0, *pResOut));
  প্রত্যাগম SQLITE_OK;
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
স্থির পূর্ণাঙ্ক dotlockLock(sqlite3_file *id, পূর্ণাঙ্ক eFileLock) {
  unixFile *pFile = (unixFile*)id;
  বর্ণ *zLockFile = (বর্ণ *)pFile->lockingContext;
  পূর্ণাঙ্ক rc = SQLITE_OK;


  /* If we have any lock, then the lock file already exists.  All we have
  ** to do is adjust our internal record of the lock level.
  */
  যদি( pFile->eFileLock > NO_LOCK ){
    pFile->eFileLock = eFileLock;
    /* Always update the timestamp on the old file */
#ifdef HAVE_UTIME
    utime(zLockFile, রিক্ত);
#অন্যথা
    utimes(zLockFile, রিক্ত);
#endif
    প্রত্যাগম SQLITE_OK;
  }

  /* grab an exclusive lock */
  rc = osMkdir(zLockFile, 0777);
  যদি( rc<0 ){
    /* failed to open/create the lock directory */
    পূর্ণাঙ্ক tErrno = errno;
    যদি( EEXIST == tErrno ){
      rc = SQLITE_BUSY;
    } অন্যথা {
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_LOCK);
      যদি( rc!=SQLITE_BUSY ){
        storeLastErrno(pFile, tErrno);
      }
    }
    প্রত্যাগম rc;
  }

  /* got it, set the type and return ok */
  pFile->eFileLock = eFileLock;
  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক dotlockUnlock(sqlite3_file *id, পূর্ণাঙ্ক eFileLock) {
  unixFile *pFile = (unixFile*)id;
  বর্ণ *zLockFile = (বর্ণ *)pFile->lockingContext;
  পূর্ণাঙ্ক rc;

  প্রতিজ্ঞা( pFile );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (dotlock)\n", pFile->h, eFileLock,
           pFile->eFileLock, osGetpid(0)));
  প্রতিজ্ঞা( eFileLock<=SHARED_LOCK );

  /* no-op if possible */
  যদি( pFile->eFileLock==eFileLock ){
    প্রত্যাগম SQLITE_OK;
  }

  /* To downgrade to shared, simply update our internal notion of the
  ** lock state.  No need to mess with the file on disk.
  */
  যদি( eFileLock==SHARED_LOCK ){
    pFile->eFileLock = SHARED_LOCK;
    প্রত্যাগম SQLITE_OK;
  }

  /* To fully unlock the database, delete the lock file */
  প্রতিজ্ঞা( eFileLock==NO_LOCK );
  rc = osRmdir(zLockFile);
  যদি( rc<0 ){
    পূর্ণাঙ্ক tErrno = errno;
    যদি( tErrno==ENOENT ){
      rc = SQLITE_OK;
    }অন্যথা{
      rc = SQLITE_IOERR_UNLOCK;
      storeLastErrno(pFile, tErrno);
    }
    প্রত্যাগম rc;
  }
  pFile->eFileLock = NO_LOCK;
  প্রত্যাগম SQLITE_OK;
}

/*
** Close a file.  Make sure the lock has been released before closing.
*/
স্থির পূর্ণাঙ্ক dotlockClose(sqlite3_file *id) {
  unixFile *pFile = (unixFile*)id;
  প্রতিজ্ঞা( id!=0 );
  dotlockUnlock(id, NO_LOCK);
  sqlite3_free(pFile->lockingContext);
  প্রত্যাগম closeUnixFile(id);
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
#যদি SQLITE_ENABLE_LOCKING_STYLE

/*
** Retry flock() calls that fail with EINTR
*/
#ifdef EINTR
স্থির পূর্ণাঙ্ক robust_flock(পূর্ণাঙ্ক fd, পূর্ণাঙ্ক op){
  পূর্ণাঙ্ক rc;
  কুরু{ rc = flock(fd,op); }যাবত্( rc<0 && errno==EINTR );
  প্রত্যাগম rc;
}
#অন্যথা
# define robust_flock(a,b) flock(a,b)
#endif
   

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
স্থির পূর্ণাঙ্ক flockCheckReservedLock(sqlite3_file *id, পূর্ণাঙ্ক *pResOut){
#ifdef SQLITE_DEBUG
  unixFile *pFile = (unixFile*)id;
#অন্যথা
  UNUSED_PARAMETER(id);
#endif

  SimulateIOError( প্রত্যাগম SQLITE_IOERR_CHECKRESERVEDLOCK; );

  প্রতিজ্ঞা( pFile );
  প্রতিজ্ঞা( pFile->eFileLock<=SHARED_LOCK );

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
  *pResOut = 0;

  প্রত্যাগম SQLITE_OK;
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
স্থির পূর্ণাঙ্ক flockLock(sqlite3_file *id, পূর্ণাঙ্ক eFileLock) {
  পূর্ণাঙ্ক rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;

  প্রতিজ্ঞা( pFile );

  /* if we already have a lock, it is exclusive.
  ** Just adjust level and punt on outta here. */
  যদি (pFile->eFileLock > NO_LOCK) {
    pFile->eFileLock = eFileLock;
    প্রত্যাগম SQLITE_OK;
  }

  /* grab an exclusive lock */

  যদি (robust_flock(pFile->h, LOCK_EX | LOCK_NB)) {
    পূর্ণাঙ্ক tErrno = errno;
    /* didn't get, must be busy */
    rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_LOCK);
    যদি( IS_LOCK_ERROR(rc) ){
      storeLastErrno(pFile, tErrno);
    }
  } অন্যথা {
    /* got it, set the type and return ok */
    pFile->eFileLock = eFileLock;
  }
  OSTRACE(("LOCK    %d %s %s (flock)\n", pFile->h, azFileLock(eFileLock),
           rc==SQLITE_OK ? "ok" : "failed"));
#ifdef SQLITE_IGNORE_FLOCK_LOCK_ERRORS
  যদি( (rc & 0xff) == SQLITE_IOERR ){
    rc = SQLITE_BUSY;
  }
#endif /* SQLITE_IGNORE_FLOCK_LOCK_ERRORS */
  প্রত্যাগম rc;
}


/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
স্থির পূর্ণাঙ্ক flockUnlock(sqlite3_file *id, পূর্ণাঙ্ক eFileLock) {
  unixFile *pFile = (unixFile*)id;

  প্রতিজ্ঞা( pFile );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (flock)\n", pFile->h, eFileLock,
           pFile->eFileLock, osGetpid(0)));
  প্রতিজ্ঞা( eFileLock<=SHARED_LOCK );

  /* no-op if possible */
  যদি( pFile->eFileLock==eFileLock ){
    প্রত্যাগম SQLITE_OK;
  }

  /* shared can just be set because we always have an exclusive */
  যদি (eFileLock==SHARED_LOCK) {
    pFile->eFileLock = eFileLock;
    প্রত্যাগম SQLITE_OK;
  }

  /* no, really, unlock. */
  যদি( robust_flock(pFile->h, LOCK_UN) ){
#ifdef SQLITE_IGNORE_FLOCK_LOCK_ERRORS
    প্রত্যাগম SQLITE_OK;
#endif /* SQLITE_IGNORE_FLOCK_LOCK_ERRORS */
    প্রত্যাগম SQLITE_IOERR_UNLOCK;
  }অন্যথা{
    pFile->eFileLock = NO_LOCK;
    প্রত্যাগম SQLITE_OK;
  }
}

/*
** Close a file.
*/
স্থির পূর্ণাঙ্ক flockClose(sqlite3_file *id) {
  প্রতিজ্ঞা( id!=0 );
  flockUnlock(id, NO_LOCK);
  প্রত্যাগম closeUnixFile(id);
}

#endif /* SQLITE_ENABLE_LOCKING_STYLE && !OS_VXWORK */

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
#যদি OS_VXWORKS

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
স্থির পূর্ণাঙ্ক semXCheckReservedLock(sqlite3_file *id, পূর্ণাঙ্ক *pResOut) {
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক reserved = 0;
  unixFile *pFile = (unixFile*)id;

  SimulateIOError( প্রত্যাগম SQLITE_IOERR_CHECKRESERVEDLOCK; );

  প্রতিজ্ঞা( pFile );

  /* Check if a thread in this process holds such a lock */
  যদি( pFile->eFileLock>SHARED_LOCK ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it. */
  যদি( !reserved ){
    sem_t *pSem = pFile->pInode->pSem;

    যদি( sem_trywait(pSem)==-1 ){
      পূর্ণাঙ্ক tErrno = errno;
      যদি( EAGAIN != tErrno ){
        rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_CHECKRESERVEDLOCK);
        storeLastErrno(pFile, tErrno);
      } অন্যথা {
        /* someone else has the lock when we are in NO_LOCK */
        reserved = (pFile->eFileLock < SHARED_LOCK);
      }
    }অন্যথা{
      /* we could have it if we want it */
      sem_post(pSem);
    }
  }
  OSTRACE(("TEST WR-LOCK %d %d %d (sem)\n", pFile->h, rc, reserved));

  *pResOut = reserved;
  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক semXLock(sqlite3_file *id, পূর্ণাঙ্ক eFileLock) {
  unixFile *pFile = (unixFile*)id;
  sem_t *pSem = pFile->pInode->pSem;
  পূর্ণাঙ্ক rc = SQLITE_OK;

  /* if we already have a lock, it is exclusive.
  ** Just adjust level and punt on outta here. */
  যদি (pFile->eFileLock > NO_LOCK) {
    pFile->eFileLock = eFileLock;
    rc = SQLITE_OK;
    গচ্ছ sem_end_lock;
  }

  /* lock semaphore now but bail out when already locked. */
  যদি( sem_trywait(pSem)==-1 ){
    rc = SQLITE_BUSY;
    গচ্ছ sem_end_lock;
  }

  /* got it, set the type and return ok */
  pFile->eFileLock = eFileLock;

 sem_end_lock:
  প্রত্যাগম rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
স্থির পূর্ণাঙ্ক semXUnlock(sqlite3_file *id, পূর্ণাঙ্ক eFileLock) {
  unixFile *pFile = (unixFile*)id;
  sem_t *pSem = pFile->pInode->pSem;

  প্রতিজ্ঞা( pFile );
  প্রতিজ্ঞা( pSem );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (sem)\n", pFile->h, eFileLock,
           pFile->eFileLock, osGetpid(0)));
  প্রতিজ্ঞা( eFileLock<=SHARED_LOCK );

  /* no-op if possible */
  যদি( pFile->eFileLock==eFileLock ){
    প্রত্যাগম SQLITE_OK;
  }

  /* shared can just be set because we always have an exclusive */
  যদি (eFileLock==SHARED_LOCK) {
    pFile->eFileLock = eFileLock;
    প্রত্যাগম SQLITE_OK;
  }

  /* no, really unlock. */
  যদি ( sem_post(pSem)==-1 ) {
    পূর্ণাঙ্ক rc, tErrno = errno;
    rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_UNLOCK);
    যদি( IS_LOCK_ERROR(rc) ){
      storeLastErrno(pFile, tErrno);
    }
    প্রত্যাগম rc;
  }
  pFile->eFileLock = NO_LOCK;
  প্রত্যাগম SQLITE_OK;
}

/*
 ** Close a file.
 */
স্থির পূর্ণাঙ্ক semXClose(sqlite3_file *id) {
  যদি( id ){
    unixFile *pFile = (unixFile*)id;
    semXUnlock(id, NO_LOCK);
    প্রতিজ্ঞা( pFile );
    প্রতিজ্ঞা( unixFileMutexNotheld(pFile) );
    unixEnterMutex();
    releaseInodeInfo(pFile);
    unixLeaveMutex();
    closeUnixFile(id);
  }
  প্রত্যাগম SQLITE_OK;
}

#endif /* OS_VXWORKS */
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

#যদি defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
** The afpLockingContext structure contains all afp lock specific state
*/
প্রকারনির্দেশ সংরচনা afpLockingContext afpLockingContext;
সংরচনা afpLockingContext {
  পূর্ণাঙ্ক reserved;
  ধ্রুব বর্ণ *dbPath;             /* Name of the open file */
};

সংরচনা ByteRangeLockPB2
{
  unsigned long long offset;        /* offset to first byte to lock */
  unsigned long long length;        /* nbr of bytes to lock */
  unsigned long long retRangeStart; /* nbr of 1st byte locked if successful */
  unsigned বর্ণ unLockFlag;         /* 1 = unlock, 0 = lock */
  unsigned বর্ণ startEndFlag;       /* 1=rel to end of fork, 0=rel to start */
  পূর্ণাঙ্ক fd;                           /* file desc to assoc this lock with */
};

#define afpfsByteRangeLock2FSCTL        _IOWR('z', 23, সংরচনা ByteRangeLockPB2)

/*
** This is a utility for setting or clearing a bit-range lock on an
** AFP filesystem.
**
** Return SQLITE_OK on success, SQLITE_BUSY on failure.
*/
স্থির পূর্ণাঙ্ক afpSetLock(
  ধ্রুব বর্ণ *path,              /* Name of the file to be locked or unlocked */
  unixFile *pFile,               /* Open file descriptor on path */
  unsigned long long offset,     /* First byte to be locked */
  unsigned long long length,     /* Number of bytes to lock */
  পূর্ণাঙ্ক setLockFlag                /* True to set lock.  False to clear lock */
){
  সংরচনা ByteRangeLockPB2 pb;
  পূর্ণাঙ্ক err;

  pb.unLockFlag = setLockFlag ? 0 : 1;
  pb.startEndFlag = 0;
  pb.offset = offset;
  pb.length = length;
  pb.fd = pFile->h;

  OSTRACE(("AFPSETLOCK [%s] for %d%s in range %llx:%llx\n",
    (setLockFlag?"ON":"OFF"), pFile->h, (pb.fd==-1?"[testval-1]":""),
    offset, length));
  err = fsctl(path, afpfsByteRangeLock2FSCTL, &pb, 0);
  যদি ( err==-1 ) {
    পূর্ণাঙ্ক rc;
    পূর্ণাঙ্ক tErrno = errno;
    OSTRACE(("AFPSETLOCK failed to fsctl() '%s' %d %s\n",
             path, tErrno, strerror(tErrno)));
#ifdef SQLITE_IGNORE_AFP_LOCK_ERRORS
    rc = SQLITE_BUSY;
#অন্যথা
    rc = sqliteErrorFromPosixError(tErrno,
                    setLockFlag ? SQLITE_IOERR_LOCK : SQLITE_IOERR_UNLOCK);
#endif /* SQLITE_IGNORE_AFP_LOCK_ERRORS */
    যদি( IS_LOCK_ERROR(rc) ){
      storeLastErrno(pFile, tErrno);
    }
    প্রত্যাগম rc;
  } অন্যথা {
    প্রত্যাগম SQLITE_OK;
  }
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
স্থির পূর্ণাঙ্ক afpCheckReservedLock(sqlite3_file *id, পূর্ণাঙ্ক *pResOut){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক reserved = 0;
  unixFile *pFile = (unixFile*)id;
  afpLockingContext *context;

  SimulateIOError( প্রত্যাগম SQLITE_IOERR_CHECKRESERVEDLOCK; );

  প্রতিজ্ঞা( pFile );
  context = (afpLockingContext *) pFile->lockingContext;
  যদি( context->reserved ){
    *pResOut = 1;
    প্রত্যাগম SQLITE_OK;
  }
  sqlite3_mutex_enter(pFile->pInode->pLockMutex);
  /* Check if a thread in this process holds such a lock */
  যদি( pFile->pInode->eFileLock>SHARED_LOCK ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it.
   */
  যদি( !reserved ){
    /* lock the RESERVED byte */
    পূর্ণাঙ্ক lrc = afpSetLock(context->dbPath, pFile, RESERVED_BYTE, 1,1);
    যদি( SQLITE_OK==lrc ){
      /* if we succeeded in taking the reserved lock, unlock it to restore
      ** the original state */
      lrc = afpSetLock(context->dbPath, pFile, RESERVED_BYTE, 1, 0);
    } অন্যথা {
      /* if we failed to get the lock then someone else must have it */
      reserved = 1;
    }
    যদি( IS_LOCK_ERROR(lrc) ){
      rc=lrc;
    }
  }

  sqlite3_mutex_leave(pFile->pInode->pLockMutex);
  OSTRACE(("TEST WR-LOCK %d %d %d (afp)\n", pFile->h, rc, reserved));

  *pResOut = reserved;
  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক afpLock(sqlite3_file *id, পূর্ণাঙ্ক eFileLock){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode = pFile->pInode;
  afpLockingContext *context = (afpLockingContext *) pFile->lockingContext;

  প্রতিজ্ঞা( pFile );
  OSTRACE(("LOCK    %d %s was %s(%s,%d) pid=%d (afp)\n", pFile->h,
           azFileLock(eFileLock), azFileLock(pFile->eFileLock),
           azFileLock(pInode->eFileLock), pInode->nShared , osGetpid(0)));

  /* If there is already a lock of this type or more restrictive on the
  ** unixFile, do nothing. Don't use the afp_end_lock: exit path, as
  ** unixEnterMutex() hasn't been called yet.
  */
  যদি( pFile->eFileLock>=eFileLock ){
    OSTRACE(("LOCK    %d %s ok (already held) (afp)\n", pFile->h,
           azFileLock(eFileLock)));
    প্রত্যাগম SQLITE_OK;
  }

  /* Make sure the locking sequence is correct
  **  (1) We never move from unlocked to anything higher than shared lock.
  **  (2) SQLite never explicitly requests a pending lock.
  **  (3) A shared lock is always held when a reserve lock is requested.
  */
  প্রতিজ্ঞা( pFile->eFileLock!=NO_LOCK || eFileLock==SHARED_LOCK );
  প্রতিজ্ঞা( eFileLock!=PENDING_LOCK );
  প্রতিজ্ঞা( eFileLock!=RESERVED_LOCK || pFile->eFileLock==SHARED_LOCK );

  /* This mutex is needed because pFile->pInode is shared across threads
  */
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->pLockMutex);

  /* If some thread using this PID has a lock via a different unixFile*
  ** handle that precludes the requested lock, return BUSY.
  */
  যদি( (pFile->eFileLock!=pInode->eFileLock &&
       (pInode->eFileLock>=PENDING_LOCK || eFileLock>SHARED_LOCK))
     ){
    rc = SQLITE_BUSY;
    গচ্ছ afp_end_lock;
  }

  /* If a SHARED lock is requested, and some thread using this PID already
  ** has a SHARED or RESERVED lock, then increment reference counts and
  ** return SQLITE_OK.
  */
  যদি( eFileLock==SHARED_LOCK &&
     (pInode->eFileLock==SHARED_LOCK || pInode->eFileLock==RESERVED_LOCK) ){
    প্রতিজ্ঞা( eFileLock==SHARED_LOCK );
    প্রতিজ্ঞা( pFile->eFileLock==0 );
    প্রতিজ্ঞা( pInode->nShared>0 );
    pFile->eFileLock = SHARED_LOCK;
    pInode->nShared++;
    pInode->nLock++;
    গচ্ছ afp_end_lock;
  }
  
  /* A PENDING lock is needed before acquiring a SHARED lock and before
  ** acquiring an EXCLUSIVE lock.  For the SHARED lock, the PENDING will
  ** be released.
  */
  যদি( eFileLock==SHARED_LOCK
      || (eFileLock==EXCLUSIVE_LOCK && pFile->eFileLock<PENDING_LOCK)
  ){
    পূর্ণাঙ্ক failed;
    failed = afpSetLock(context->dbPath, pFile, PENDING_BYTE, 1, 1);
    যদি (failed) {
      rc = failed;
      গচ্ছ afp_end_lock;
    }
  }

  /* If control gets to this point, then actually go ahead and make
  ** operating system calls for the specified lock.
  */
  যদি( eFileLock==SHARED_LOCK ){
    পূর্ণাঙ্ক lrc1, lrc2, lrc1Errno = 0;
    long lk, mask;
  
    প্রতিজ্ঞা( pInode->nShared==0 );
    প্রতিজ্ঞা( pInode->eFileLock==0 );
      
    mask = (পরিমাণ(long)==8) ? LARGEST_INT64 : 0x7fffffff;
    /* Now get the read-lock SHARED_LOCK */
    /* note that the quality of the randomness doesn't matter that much */
    lk = random();
    pInode->sharedByte = (lk & mask)%(SHARED_SIZE - 1);
    lrc1 = afpSetLock(context->dbPath, pFile,
          SHARED_FIRST+pInode->sharedByte, 1, 1);
    যদি( IS_LOCK_ERROR(lrc1) ){
      lrc1Errno = pFile->lastErrno;
    }
    /* Drop the temporary PENDING lock */
    lrc2 = afpSetLock(context->dbPath, pFile, PENDING_BYTE, 1, 0);
  
    যদি( IS_LOCK_ERROR(lrc1) ) {
      storeLastErrno(pFile, lrc1Errno);
      rc = lrc1;
      গচ্ছ afp_end_lock;
    } অন্যথা যদি( IS_LOCK_ERROR(lrc2) ){
      rc = lrc2;
      গচ্ছ afp_end_lock;
    } অন্যথা যদি( lrc1 != SQLITE_OK ) {
      rc = lrc1;
    } অন্যথা {
      pFile->eFileLock = SHARED_LOCK;
      pInode->nLock++;
      pInode->nShared = 1;
    }
  }অন্যথা যদি( eFileLock==EXCLUSIVE_LOCK && pInode->nShared>1 ){
    /* We are trying for an exclusive lock but another thread in this
     ** same process is still holding a shared lock. */
    rc = SQLITE_BUSY;
  }অন্যথা{
    /* The request was for a RESERVED or EXCLUSIVE lock.  It is
    ** assumed that there is a SHARED or greater lock on the file
    ** already.
    */
    পূর্ণাঙ্ক failed = 0;
    প্রতিজ্ঞা( 0!=pFile->eFileLock );
    যদি (eFileLock >= RESERVED_LOCK && pFile->eFileLock < RESERVED_LOCK) {
        /* Acquire a RESERVED lock */
        failed = afpSetLock(context->dbPath, pFile, RESERVED_BYTE, 1,1);
      যদি( !failed ){
        context->reserved = 1;
      }
    }
    যদি (!failed && eFileLock == EXCLUSIVE_LOCK) {
      /* Acquire an EXCLUSIVE lock */

      /* Remove the shared lock before trying the range.  we'll need to
      ** reestablish the shared lock if we can't get the  afpUnlock
      */
      যদি( !(failed = afpSetLock(context->dbPath, pFile, SHARED_FIRST +
                         pInode->sharedByte, 1, 0)) ){
        পূর্ণাঙ্ক failed2 = SQLITE_OK;
        /* now attempt to get the exclusive lock range */
        failed = afpSetLock(context->dbPath, pFile, SHARED_FIRST,
                               SHARED_SIZE, 1);
        যদি( failed && (failed2 = afpSetLock(context->dbPath, pFile,
                       SHARED_FIRST + pInode->sharedByte, 1, 1)) ){
          /* Can't reestablish the shared lock.  Sqlite can't deal, this is
          ** a critical I/O error
          */
          rc = ((failed & 0xff) == SQLITE_IOERR) ? failed2 :
               SQLITE_IOERR_LOCK;
          গচ্ছ afp_end_lock;
        }
      }অন্যথা{
        rc = failed;
      }
    }
    যদি( failed ){
      rc = failed;
    }
  }

  যদি( rc==SQLITE_OK ){
    pFile->eFileLock = eFileLock;
    pInode->eFileLock = eFileLock;
  }অন্যথা যদি( eFileLock==EXCLUSIVE_LOCK ){
    pFile->eFileLock = PENDING_LOCK;
    pInode->eFileLock = PENDING_LOCK;
  }

afp_end_lock:
  sqlite3_mutex_leave(pInode->pLockMutex);
  OSTRACE(("LOCK    %d %s %s (afp)\n", pFile->h, azFileLock(eFileLock),
         rc==SQLITE_OK ? "ok" : "failed"));
  প্রত্যাগম rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
স্থির পূর্ণাঙ্ক afpUnlock(sqlite3_file *id, পূর্ণাঙ্ক eFileLock) {
  পূর্ণাঙ্ক rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  afpLockingContext *context = (afpLockingContext *) pFile->lockingContext;
  পূর্ণাঙ্ক skipShared = 0;

  প্রতিজ্ঞা( pFile );
  OSTRACE(("UNLOCK  %d %d was %d(%d,%d) pid=%d (afp)\n", pFile->h, eFileLock,
           pFile->eFileLock, pFile->pInode->eFileLock, pFile->pInode->nShared,
           osGetpid(0)));

  প্রতিজ্ঞা( eFileLock<=SHARED_LOCK );
  যদি( pFile->eFileLock<=eFileLock ){
    প্রত্যাগম SQLITE_OK;
  }
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->pLockMutex);
  প্রতিজ্ঞা( pInode->nShared!=0 );
  যদি( pFile->eFileLock>SHARED_LOCK ){
    প্রতিজ্ঞা( pInode->eFileLock==pFile->eFileLock );
  
#ifdef SQLITE_DEBUG
    /* When reducing a lock such that other processes can start
    ** reading the database file again, make sure that the
    ** transaction counter was updated if any part of the database
    ** file changed.  If the transaction counter is not updated,
    ** other connections to the same file might not realize that
    ** the file has changed and hence might not know to flush their
    ** cache.  The use of a stale cache can lead to database corruption.
    */
    প্রতিজ্ঞা( pFile->inNormalWrite==0
           || pFile->dbUpdate==0
           || pFile->transCntrChng==1 );
    pFile->inNormalWrite = 0;
#endif
  
    যদি( pFile->eFileLock==EXCLUSIVE_LOCK ){
      rc = afpSetLock(context->dbPath, pFile, SHARED_FIRST, SHARED_SIZE, 0);
      যদি( rc==SQLITE_OK && (eFileLock==SHARED_LOCK || pInode->nShared>1) ){
        /* only re-establish the shared lock if necessary */
        পূর্ণাঙ্ক sharedLockByte = SHARED_FIRST+pInode->sharedByte;
        rc = afpSetLock(context->dbPath, pFile, sharedLockByte, 1, 1);
      } অন্যথা {
        skipShared = 1;
      }
    }
    যদি( rc==SQLITE_OK && pFile->eFileLock>=PENDING_LOCK ){
      rc = afpSetLock(context->dbPath, pFile, PENDING_BYTE, 1, 0);
    }
    যদি( rc==SQLITE_OK && pFile->eFileLock>=RESERVED_LOCK && context->reserved ){
      rc = afpSetLock(context->dbPath, pFile, RESERVED_BYTE, 1, 0);
      যদি( !rc ){
        context->reserved = 0;
      }
    }
    যদি( rc==SQLITE_OK && (eFileLock==SHARED_LOCK || pInode->nShared>1)){
      pInode->eFileLock = SHARED_LOCK;
    }
  }
  যদি( rc==SQLITE_OK && eFileLock==NO_LOCK ){

    /* Decrement the shared lock counter.  Release the lock using an
    ** OS call only when all threads in this same process have released
    ** the lock.
    */
    unsigned long long sharedLockByte = SHARED_FIRST+pInode->sharedByte;
    pInode->nShared--;
    যদি( pInode->nShared==0 ){
      যদি( !skipShared ){
        rc = afpSetLock(context->dbPath, pFile, sharedLockByte, 1, 0);
      }
      যদি( !rc ){
        pInode->eFileLock = NO_LOCK;
        pFile->eFileLock = NO_LOCK;
      }
    }
    যদি( rc==SQLITE_OK ){
      pInode->nLock--;
      প্রতিজ্ঞা( pInode->nLock>=0 );
      যদি( pInode->nLock==0 ) closePendingFds(pFile);
    }
  }

  sqlite3_mutex_leave(pInode->pLockMutex);
  যদি( rc==SQLITE_OK ){
    pFile->eFileLock = eFileLock;
  }
  প্রত্যাগম rc;
}

/*
** Close a file & cleanup AFP specific locking context
*/
স্থির পূর্ণাঙ্ক afpClose(sqlite3_file *id) {
  পূর্ণাঙ্ক rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  প্রতিজ্ঞা( id!=0 );
  afpUnlock(id, NO_LOCK);
  প্রতিজ্ঞা( unixFileMutexNotheld(pFile) );
  unixEnterMutex();
  যদি( pFile->pInode ){
    unixInodeInfo *pInode = pFile->pInode;
    sqlite3_mutex_enter(pInode->pLockMutex);
    যদি( pInode->nLock ){
      /* If there are outstanding locks, do not actually close the file just
      ** yet because that would clear those locks.  Instead, add the file
      ** descriptor to pInode->aPending.  It will be automatically closed when
      ** the last lock is cleared.
      */
      setPendingFd(pFile);
    }
    sqlite3_mutex_leave(pInode->pLockMutex);
  }
  releaseInodeInfo(pFile);
  sqlite3_free(pFile->lockingContext);
  rc = closeUnixFile(id);
  unixLeaveMutex();
  প্রত্যাগম rc;
}

#endif /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
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

#যদি defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
 ** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
 ** must be either NO_LOCK or SHARED_LOCK.
 **
 ** If the locking level of the file descriptor is already at or below
 ** the requested locking level, this routine is a no-op.
 */
স্থির পূর্ণাঙ্ক nfsUnlock(sqlite3_file *id, পূর্ণাঙ্ক eFileLock){
  প্রত্যাগম posixUnlock(id, eFileLock, 1);
}

#endif /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
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
স্থির পূর্ণাঙ্ক seekAndRead(unixFile *id, sqlite3_int64 offset, শূন্য *pBuf, পূর্ণাঙ্ক cnt){
  পূর্ণাঙ্ক got;
  পূর্ণাঙ্ক prior = 0;
#যদি (!defined(USE_PREAD) && !defined(USE_PREAD64))
  i64 newOffset;
#endif
  TIMER_START;
  প্রতিজ্ঞা( cnt==(cnt&0x1ffff) );
  প্রতিজ্ঞা( id->h>2 );
  কুরু{
#যদি defined(USE_PREAD)
    got = osPread(id->h, pBuf, cnt, offset);
    SimulateIOError( got = -1 );
#নোচেদ্যদি defined(USE_PREAD64)
    got = osPread64(id->h, pBuf, cnt, offset);
    SimulateIOError( got = -1 );
#অন্যথা
    newOffset = lseek(id->h, offset, SEEK_SET);
    SimulateIOError( newOffset = -1 );
    যদি( newOffset<0 ){
      storeLastErrno((unixFile*)id, errno);
      প্রত্যাগম -1;
    }
    got = osRead(id->h, pBuf, cnt);
#endif
    যদি( got==cnt ) ভঙ্গ;
    যদি( got<0 ){
      যদি( errno==EINTR ){ got = 1; অনুবর্ত; }
      prior = 0;
      storeLastErrno((unixFile*)id,  errno);
      ভঙ্গ;
    }অন্যথা যদি( got>0 ){
      cnt -= got;
      offset += got;
      prior += got;
      pBuf = (শূন্য*)(got + (বর্ণ*)pBuf);
    }
  }যাবত্( got>0 );
  TIMER_END;
  OSTRACE(("READ    %-3d %5d %7lld %llu\n",
            id->h, got+prior, offset-prior, TIMER_ELAPSED));
  প্রত্যাগম got+prior;
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
স্থির পূর্ণাঙ্ক unixRead(
  sqlite3_file *id,
  শূন্য *pBuf,
  পূর্ণাঙ্ক amt,
  sqlite3_int64 offset
){
  unixFile *pFile = (unixFile *)id;
  পূর্ণাঙ্ক got;
  প্রতিজ্ঞা( id );
  প্রতিজ্ঞা( offset>=0 );
  প্রতিজ্ঞা( amt>0 );

  /* If this is a database file (not a journal, super-journal or temp
  ** file), the bytes in the locking range should never be read or written. */
#যদি 0
  প্রতিজ্ঞা( pFile->pPreallocatedUnused==0
       || offset>=PENDING_BYTE+512
       || offset+amt<=PENDING_BYTE
  );
#endif

#যদি SQLITE_MAX_MMAP_SIZE>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  যদি( offset<pFile->mmapSize ){
    যদি( offset+amt <= pFile->mmapSize ){
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], amt);
      প্রত্যাগম SQLITE_OK;
    }অন্যথা{
      পূর্ণাঙ্ক nCopy = pFile->mmapSize - offset;
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#endif

  got = seekAndRead(pFile, offset, pBuf, amt);
  যদি( got==amt ){
    প্রত্যাগম SQLITE_OK;
  }অন্যথা যদি( got<0 ){
    /* pFile->lastErrno has been set by seekAndRead().
    ** Usually we return SQLITE_IOERR_READ here, though for some
    ** kinds of errors we return SQLITE_IOERR_CORRUPTFS.  The
    ** SQLITE_IOERR_CORRUPTFS will be converted into SQLITE_CORRUPT
    ** prior to returning to the application by the sqlite3ApiExit()
    ** routine.
    */
    বিকল্প( pFile->lastErrno ){
      পক্ষ ERANGE:
      পক্ষ EIO:
#ifdef ENXIO
      পক্ষ ENXIO:
#endif
#ifdef EDEVERR
      পক্ষ EDEVERR:
#endif
        প্রত্যাগম SQLITE_IOERR_CORRUPTFS;
    }
    প্রত্যাগম SQLITE_IOERR_READ;
  }অন্যথা{
    storeLastErrno(pFile, 0);   /* not a system error */
    /* Unread parts of the buffer must be zero-filled */
    memset(&((বর্ণ*)pBuf)[got], 0, amt-got);
    প্রত্যাগম SQLITE_IOERR_SHORT_READ;
  }
}

/*
** Attempt to seek the file-descriptor passed as the first argument to
** absolute offset iOff, then attempt to write nBuf bytes of data from
** pBuf to it. If an error occurs, return -1 and set *piErrno. Otherwise,
** return the actual number of bytes written (which may be less than
** nBuf).
*/
স্থির পূর্ণাঙ্ক seekAndWriteFd(
  পূর্ণাঙ্ক fd,                         /* File descriptor to write to */
  i64 iOff,                       /* File offset to begin writing at */
  ধ্রুব শূন্য *pBuf,               /* Copy data from this buffer to the file */
  পূর্ণাঙ্ক nBuf,                       /* Size of buffer pBuf in bytes */
  পূর্ণাঙ্ক *piErrno                    /* OUT: Error number if error occurs */
){
  পূর্ণাঙ্ক rc = 0;                     /* Value returned by system call */

  প্রতিজ্ঞা( nBuf==(nBuf&0x1ffff) );
  প্রতিজ্ঞা( fd>2 );
  প্রতিজ্ঞা( piErrno!=0 );
  nBuf &= 0x1ffff;
  TIMER_START;

#যদি defined(USE_PREAD)
  কুরু{ rc = (পূর্ণাঙ্ক)osPwrite(fd, pBuf, nBuf, iOff); }যাবত্( rc<0 && errno==EINTR );
#নোচেদ্যদি defined(USE_PREAD64)
  কুরু{ rc = (পূর্ণাঙ্ক)osPwrite64(fd, pBuf, nBuf, iOff);}যাবত্( rc<0 && errno==EINTR);
#অন্যথা
  কুরু{
    i64 iSeek = lseek(fd, iOff, SEEK_SET);
    SimulateIOError( iSeek = -1 );
    যদি( iSeek<0 ){
      rc = -1;
      ভঙ্গ;
    }
    rc = osWrite(fd, pBuf, nBuf);
  }যাবত্( rc<0 && errno==EINTR );
#endif

  TIMER_END;
  OSTRACE(("WRITE   %-3d %5d %7lld %llu\n", fd, rc, iOff, TIMER_ELAPSED));

  যদি( rc<0 ) *piErrno = errno;
  প্রত্যাগম rc;
}


/*
** Seek to the offset in id->offset then read cnt bytes into pBuf.
** Return the number of bytes actually read.  Update the offset.
**
** To avoid stomping the errno value on a failed write the lastErrno value
** is set before returning.
*/
স্থির পূর্ণাঙ্ক seekAndWrite(unixFile *id, i64 offset, ধ্রুব শূন্য *pBuf, পূর্ণাঙ্ক cnt){
  প্রত্যাগম seekAndWriteFd(id->h, offset, pBuf, cnt, &id->lastErrno);
}


/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
স্থির পূর্ণাঙ্ক unixWrite(
  sqlite3_file *id,
  ধ্রুব শূন্য *pBuf,
  পূর্ণাঙ্ক amt,
  sqlite3_int64 offset
){
  unixFile *pFile = (unixFile*)id;
  পূর্ণাঙ্ক wrote = 0;
  প্রতিজ্ঞা( id );
  প্রতিজ্ঞা( amt>0 );

  /* If this is a database file (not a journal, super-journal or temp
  ** file), the bytes in the locking range should never be read or written. */
#যদি 0
  প্রতিজ্ঞা( pFile->pPreallocatedUnused==0
       || offset>=PENDING_BYTE+512
       || offset+amt<=PENDING_BYTE
  );
#endif

#ifdef SQLITE_DEBUG
  /* If we are doing a normal write to a database file (as opposed to
  ** doing a hot-journal rollback or a write to some file other than a
  ** normal database file) then record the fact that the database
  ** has changed.  If the transaction counter is modified, record that
  ** fact too.
  */
  যদি( pFile->inNormalWrite ){
    pFile->dbUpdate = 1;  /* The database has been modified */
    যদি( offset<=24 && offset+amt>=27 ){
      পূর্ণাঙ্ক rc;
      বর্ণ oldCntr[4];
      SimulateIOErrorBenign(1);
      rc = seekAndRead(pFile, 24, oldCntr, 4);
      SimulateIOErrorBenign(0);
      যদি( rc!=4 || memcmp(oldCntr, &((বর্ণ*)pBuf)[24-offset], 4)!=0 ){
        pFile->transCntrChng = 1;  /* The transaction counter has changed */
      }
    }
  }
#endif

#যদি defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_SIZE>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  যদি( offset<pFile->mmapSize ){
    যদি( offset+amt <= pFile->mmapSize ){
      memcpy(&((u8 *)(pFile->pMapRegion))[offset], pBuf, amt);
      প্রত্যাগম SQLITE_OK;
    }অন্যথা{
      পূর্ণাঙ্ক nCopy = pFile->mmapSize - offset;
      memcpy(&((u8 *)(pFile->pMapRegion))[offset], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#endif

  যাবত্( (wrote = seekAndWrite(pFile, offset, pBuf, amt))<amt && wrote>0 ){
    amt -= wrote;
    offset += wrote;
    pBuf = &((বর্ণ*)pBuf)[wrote];
  }
  SimulateIOError(( wrote=(-1), amt=1 ));
  SimulateDiskfullError(( wrote=0, amt=1 ));

  যদি( amt>wrote ){
    যদি( wrote<0 && pFile->lastErrno!=ENOSPC ){
      /* lastErrno set by seekAndWrite */
      প্রত্যাগম SQLITE_IOERR_WRITE;
    }অন্যথা{
      storeLastErrno(pFile, 0); /* not a system error */
      প্রত্যাগম SQLITE_FULL;
    }
  }

  প্রত্যাগম SQLITE_OK;
}

#ifdef SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occurring at the right times.
*/
পূর্ণাঙ্ক sqlite3_sync_গুনতি = 0;
পূর্ণাঙ্ক sqlite3_fullsync_গুনতি = 0;
#endif

/*
** We do not trust systems to provide a working fdatasync().  Some do.
** Others do no.  To be safe, we will stick with the (slightly slower)
** fsync(). If you know that your system does support fdatasync() correctly,
** then simply compile with -Dfdatasync=fdatasync or -DHAVE_FDATASYNC
*/
#যদি !defined(fdatasync) && !HAVE_FDATASYNC
# define fdatasync fsync
#endif

/*
** Define HAVE_FULLFSYNC to 0 or 1 depending on whether or not
** the F_FULLFSYNC macro is defined.  F_FULLFSYNC is currently
** only available on Mac OS X.  But that could change.
*/
#ifdef F_FULLFSYNC
# define HAVE_FULLFSYNC 1
#অন্যথা
# define HAVE_FULLFSYNC 0
#endif


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
স্থির পূর্ণাঙ্ক full_fsync(পূর্ণাঙ্ক fd, পূর্ণাঙ্ক fullSync, পূর্ণাঙ্ক dataOnly){
  পূর্ণাঙ্ক rc;

  /* The following "ifdef/elif/else/" block has the same structure as
  ** the one below. It is replicated here solely to avoid cluttering
  ** up the real code with the UNUSED_PARAMETER() macros.
  */
#ifdef SQLITE_NO_SYNC
  UNUSED_PARAMETER(fd);
  UNUSED_PARAMETER(fullSync);
  UNUSED_PARAMETER(dataOnly);
#নোচেদ্যদি HAVE_FULLFSYNC
  UNUSED_PARAMETER(dataOnly);
#অন্যথা
  UNUSED_PARAMETER(fullSync);
  UNUSED_PARAMETER(dataOnly);
#endif

  /* Record the number of times that we do a normal fsync() and
  ** FULLSYNC.  This is used during testing to verify that this procedure
  ** gets called with the correct arguments.
  */
#ifdef SQLITE_TEST
  যদি( fullSync ) sqlite3_fullsync_গুনতি++;
  sqlite3_sync_গুনতি++;
#endif

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op.  But go ahead and call fstat() to validate the file
  ** descriptor as we need a method to provoke a failure during
  ** coverage testing.
  */
#ifdef SQLITE_NO_SYNC
  {
    সংরচনা stat buf;
    rc = osFstat(fd, &buf);
  }
#নোচেদ্যদি HAVE_FULLFSYNC
  যদি( fullSync ){
    rc = osFcntl(fd, F_FULLFSYNC, 0);
  }অন্যথা{
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
  যদি( rc ) rc = fsync(fd);

#নোচেদ্যদি defined(__APPLE__)
  /* fdatasync() on HFS+ doesn't yet flush the file size if it changed correctly
  ** so currently we default to the macro that redefines fdatasync to fsync
  */
  rc = fsync(fd);
#অন্যথা
  rc = fdatasync(fd);
#যদি OS_VXWORKS
  যদি( rc==-1 && errno==ENOTSUP ){
    rc = fsync(fd);
  }
#endif /* OS_VXWORKS */
#endif /* ifdef SQLITE_NO_SYNC elif HAVE_FULLFSYNC */

  যদি( OS_VXWORKS && rc!= -1 ){
    rc = 0;
  }
  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক openDirectory(ধ্রুব বর্ণ *zFilename, পূর্ণাঙ্ক *pFd){
  পূর্ণাঙ্ক ii;
  পূর্ণাঙ্ক fd = -1;
  বর্ণ zDirname[MAX_PATHNAME+1];

  sqlite3_snprintf(MAX_PATHNAME, zDirname, "%s", zFilename);
  প্রতি(ii=(পূর্ণাঙ্ক)strlen(zDirname); ii>0 && zDirname[ii]!='/'; ii--);
  যদি( ii>0 ){
    zDirname[ii] = '\0';
  }অন্যথা{
    যদি( zDirname[0]!='/' ) zDirname[0] = '.';
    zDirname[1] = 0;
  }
  fd = robust_open(zDirname, O_RDONLY|O_দ্বিমিক, 0);
  যদি( fd>=0 ){
    OSTRACE(("OPENDIR %-3d %s\n", fd, zDirname));
  }
  *pFd = fd;
  যদি( fd>=0 ) প্রত্যাগম SQLITE_OK;
  প্রত্যাগম unixLogError(SQLITE_CANTOPEN_BKPT, "openDirectory", zDirname);
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
স্থির পূর্ণাঙ্ক unixSync(sqlite3_file *id, পূর্ণাঙ্ক flags){
  পূর্ণাঙ্ক rc;
  unixFile *pFile = (unixFile*)id;

  পূর্ণাঙ্ক isDataOnly = (flags&SQLITE_SYNC_DATAONLY);
  পূর্ণাঙ্ক isFullsync = (flags&0x0F)==SQLITE_SYNC_FULL;

  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  প্রতিজ্ঞা((flags&0x0F)==SQLITE_SYNC_NORMAL
      || (flags&0x0F)==SQLITE_SYNC_FULL
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( প্রত্যাগম SQLITE_FULL );

  প্রতিজ্ঞা( pFile );
  OSTRACE(("SYNC    %-3d\n", pFile->h));
  rc = full_fsync(pFile->h, isFullsync, isDataOnly);
  SimulateIOError( rc=1 );
  যদি( rc ){
    storeLastErrno(pFile, errno);
    প্রত্যাগম unixLogError(SQLITE_IOERR_FSYNC, "full_fsync", pFile->zPath);
  }

  /* Also fsync the directory containing the file if the DIRSYNC flag
  ** is set.  This is a one-time occurrence.  Many systems (examples: AIX)
  ** are unable to fsync a directory, so ignore errors on the fsync.
  */
  যদি( pFile->ctrlFlags & UNIXFILE_DIRSYNC ){
    পূর্ণাঙ্ক dirfd;
    OSTRACE(("DIRSYNC %s (have_fullfsync=%d fullsync=%d)\n", pFile->zPath,
            HAVE_FULLFSYNC, isFullsync));
    rc = osOpenDirectory(pFile->zPath, &dirfd);
    যদি( rc==SQLITE_OK ){
      full_fsync(dirfd, 0, 0);
      robust_close(pFile, dirfd, __LINE__);
    }অন্যথা{
      প্রতিজ্ঞা( rc==SQLITE_CANTOPEN );
      rc = SQLITE_OK;
    }
    pFile->ctrlFlags &= ~UNIXFILE_DIRSYNC;
  }
  প্রত্যাগম rc;
}

/*
** Truncate an open file to a specified size
*/
স্থির পূর্ণাঙ্ক unixTruncate(sqlite3_file *id, i64 nByte){
  unixFile *pFile = (unixFile *)id;
  পূর্ণাঙ্ক rc;
  প্রতিজ্ঞা( pFile );
  SimulateIOError( প্রত্যাগম SQLITE_IOERR_TRUNCATE );

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  যদি( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

  rc = robust_ftruncate(pFile->h, nByte);
  যদি( rc ){
    storeLastErrno(pFile, errno);
    প্রত্যাগম unixLogError(SQLITE_IOERR_TRUNCATE, "ftruncate", pFile->zPath);
  }অন্যথা{
#ifdef SQLITE_DEBUG
    /* If we are doing a normal write to a database file (as opposed to
    ** doing a hot-journal rollback or a write to some file other than a
    ** normal database file) and we truncate the file to zero length,
    ** that effectively updates the change counter.  This might happen
    ** when restoring a database using the backup API from a zero-length
    ** source.
    */
    যদি( pFile->inNormalWrite && nByte==0 ){
      pFile->transCntrChng = 1;
    }
#endif

#যদি SQLITE_MAX_MMAP_SIZE>0
    /* If the file was just truncated to a size smaller than the currently
    ** mapped region, reduce the effective mapping size as well. SQLite will
    ** use read() and write() to access data beyond this point from now on.
    */
    যদি( nByte<pFile->mmapSize ){
      pFile->mmapSize = nByte;
    }
#endif

    প্রত্যাগম SQLITE_OK;
  }
}

/*
** Determine the current size of a file in bytes
*/
স্থির পূর্ণাঙ্ক unixFileSize(sqlite3_file *id, i64 *pSize){
  পূর্ণাঙ্ক rc;
  সংরচনা stat buf;
  প্রতিজ্ঞা( id );
  rc = osFstat(((unixFile*)id)->h, &buf);
  SimulateIOError( rc=1 );
  যদি( rc!=0 ){
    storeLastErrno((unixFile*)id, errno);
    প্রত্যাগম SQLITE_IOERR_FSTAT;
  }
  *pSize = buf.st_size;

  /* When opening a zero-size database, the findInodeInfo() procedure
  ** writes a single byte into that file in order to work around a bug
  ** in the OS-X msdos filesystem.  In order to avoid problems with upper
  ** layers, we need to report this file size as zero even though it is
  ** really 1.   Ticket #3260.
  */
  যদি( *pSize==1 ) *pSize = 0;


  প্রত্যাগম SQLITE_OK;
}

#যদি SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
/*
** Handler for proxy-locking file-control verbs.  Defined below in the
** proxying locking division.
*/
স্থির পূর্ণাঙ্ক proxyFileControl(sqlite3_file*,পূর্ণাঙ্ক,শূন্য*);
#endif

/*
** This function is called to handle the SQLITE_FCNTL_SIZE_HINT
** file-control operation.  Enlarge the database to nBytes in size
** (rounded up to the next chunk-size).  If the database is already
** nBytes or larger, this routine is a no-op.
*/
স্থির পূর্ণাঙ্ক fcntlSizeHint(unixFile *pFile, i64 nByte){
  যদি( pFile->szChunk>0 ){
    i64 nSize;                    /* Required file size */
    সংরচনা stat buf;              /* Used to hold return values of fstat() */
 
    যদি( osFstat(pFile->h, &buf) ){
      প্রত্যাগম SQLITE_IOERR_FSTAT;
    }

    nSize = ((nByte+pFile->szChunk-1) / pFile->szChunk) * pFile->szChunk;
    যদি( nSize>(i64)buf.st_size ){

#যদি defined(HAVE_POSIX_FALLOCATE) && HAVE_POSIX_FALLOCATE
      /* The code below is handling the return value of osFallocate()
      ** correctly. posix_fallocate() is defined to "returns zero on success,
      ** or an error number on  failure". See the manpage for details. */
      পূর্ণাঙ্ক err;
      কুরু{
        err = osFallocate(pFile->h, buf.st_size, nSize-buf.st_size);
      }যাবত্( err==EINTR );
      যদি( err && err!=EINVAL ) প্রত্যাগম SQLITE_IOERR_WRITE;
#অন্যথা
      /* If the OS does not have posix_fallocate(), fake it. Write a
      ** single byte to the last byte in each block that falls entirely
      ** within the extended region. Then, if required, a single byte
      ** at offset (nSize-1), to set the size of the file correctly.
      ** This is a similar technique to that used by glibc on systems
      ** that do not have a real fallocate() call.
      */
      পূর্ণাঙ্ক nBlk = buf.st_blksize;  /* File-system block size */
      পূর্ণাঙ্ক nWrite = 0;             /* Number of bytes written by seekAndWrite */
      i64 iWrite;                 /* Next offset to write to */

      iWrite = (buf.st_size/nBlk)*nBlk + nBlk - 1;
      প্রতিজ্ঞা( iWrite>=buf.st_size );
      প্রতিজ্ঞা( ((iWrite+1)%nBlk)==0 );
      প্রতি(/*no-op*/; iWrite<nSize+nBlk-1; iWrite+=nBlk ){
        যদি( iWrite>=nSize ) iWrite = nSize - 1;
        nWrite = seekAndWrite(pFile, iWrite, "", 1);
        যদি( nWrite!=1 ) প্রত্যাগম SQLITE_IOERR_WRITE;
      }
#endif
    }
  }

#যদি SQLITE_MAX_MMAP_SIZE>0
  যদি( pFile->mmapSizeMax>0 && nByte>pFile->mmapSize ){
    পূর্ণাঙ্ক rc;
    যদি( pFile->szChunk<=0 ){
      যদি( robust_ftruncate(pFile->h, nByte) ){
        storeLastErrno(pFile, errno);
        প্রত্যাগম unixLogError(SQLITE_IOERR_TRUNCATE, "ftruncate", pFile->zPath);
      }
    }

    rc = unixMapfile(pFile, nByte);
    প্রত্যাগম rc;
  }
#endif

  প্রত্যাগম SQLITE_OK;
}

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
স্থির শূন্য unixModeBit(unixFile *pFile, unsigned বর্ণ mask, পূর্ণাঙ্ক *pArg){
  যদি( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & mask)!=0;
  }অন্যথা যদি( (*pArg)==0 ){
    pFile->ctrlFlags &= ~mask;
  }অন্যথা{
    pFile->ctrlFlags |= mask;
  }
}

/* Forward declaration */
স্থির পূর্ণাঙ্ক unixGetTempname(পূর্ণাঙ্ক nBuf, বর্ণ *zBuf);
#যদি !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
 স্থির পূর্ণাঙ্ক unixFcntlExternalReader(unixFile*, পূর্ণাঙ্ক*);
#endif
#যদি defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
 স্থির শূন্য unixDescribeShm(sqlite3_str*,unixShm*);
#endif


/*
** Information and control of an open file handle.
*/
স্থির পূর্ণাঙ্ক unixFileControl(sqlite3_file *id, পূর্ণাঙ্ক op, শূন্য *pArg){
  unixFile *pFile = (unixFile*)id;
  বিকল্প( op ){
#যদি defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
    পক্ষ SQLITE_FCNTL_BEGIN_ATOMIC_WRITE: {
      পূর্ণাঙ্ক rc = osIoctl(pFile->h, F2FS_IOC_START_ATOMIC_WRITE);
      প্রত্যাগম rc ? SQLITE_IOERR_BEGIN_ATOMIC : SQLITE_OK;
    }
    পক্ষ SQLITE_FCNTL_COMMIT_ATOMIC_WRITE: {
      পূর্ণাঙ্ক rc = osIoctl(pFile->h, F2FS_IOC_COMMIT_ATOMIC_WRITE);
      প্রত্যাগম rc ? SQLITE_IOERR_COMMIT_ATOMIC : SQLITE_OK;
    }
    পক্ষ SQLITE_FCNTL_ROLLBACK_ATOMIC_WRITE: {
      পূর্ণাঙ্ক rc = osIoctl(pFile->h, F2FS_IOC_ABORT_VOLATILE_WRITE);
      প্রত্যাগম rc ? SQLITE_IOERR_ROLLBACK_ATOMIC : SQLITE_OK;
    }
#endif /* __linux__ && SQLITE_ENABLE_BATCH_ATOMIC_WRITE */

    পক্ষ SQLITE_FCNTL_NULL_IO: {
      osClose(pFile->h);
      pFile->h = -1;
      প্রত্যাগম SQLITE_OK;
    }
    পক্ষ SQLITE_FCNTL_LOCKSTATE: {
      *(পূর্ণাঙ্ক*)pArg = pFile->eFileLock;
      প্রত্যাগম SQLITE_OK;
    }
    পক্ষ SQLITE_FCNTL_LAST_ERRNO: {
      *(পূর্ণাঙ্ক*)pArg = pFile->lastErrno;
      প্রত্যাগম SQLITE_OK;
    }
    পক্ষ SQLITE_FCNTL_CHUNK_SIZE: {
      pFile->szChunk = *(পূর্ণাঙ্ক *)pArg;
      প্রত্যাগম SQLITE_OK;
    }
    পক্ষ SQLITE_FCNTL_SIZE_HINT: {
      পূর্ণাঙ্ক rc;
      SimulateIOErrorBenign(1);
      rc = fcntlSizeHint(pFile, *(i64 *)pArg);
      SimulateIOErrorBenign(0);
      প্রত্যাগম rc;
    }
    পক্ষ SQLITE_FCNTL_PERSIST_WAL: {
      unixModeBit(pFile, UNIXFILE_PERSIST_WAL, (পূর্ণাঙ্ক*)pArg);
      প্রত্যাগম SQLITE_OK;
    }
    পক্ষ SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      unixModeBit(pFile, UNIXFILE_PSOW, (পূর্ণাঙ্ক*)pArg);
      প্রত্যাগম SQLITE_OK;
    }
    পক্ষ SQLITE_FCNTL_VFSNAME: {
      *(বর্ণ**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      প্রত্যাগম SQLITE_OK;
    }
    পক্ষ SQLITE_FCNTL_TEMPFILENAME: {
      বর্ণ *zTFile = sqlite3_malloc64( pFile->pVfs->mxPathname );
      যদি( zTFile ){
        unixGetTempname(pFile->pVfs->mxPathname, zTFile);
        *(বর্ণ**)pArg = zTFile;
      }
      প্রত্যাগম SQLITE_OK;
    }
    পক্ষ SQLITE_FCNTL_HAS_MOVED: {
      *(পূর্ণাঙ্ক*)pArg = fileHasMoved(pFile);
      প্রত্যাগম SQLITE_OK;
    }
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    পক্ষ SQLITE_FCNTL_LOCK_TIMEOUT: {
      পূর্ণাঙ্ক iOld = pFile->iBusyTimeout;
      পূর্ণাঙ্ক iNew = *(পূর্ণাঙ্ক*)pArg;
#যদি SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = iNew<0 ? 0x7FFFFFFF : (unsigned)iNew;
#নোচেদ্যদি SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = !!(*(পূর্ণাঙ্ক*)pArg);
#অন্যথা
# error "SQLITE_ENABLE_SETLK_TIMEOUT must be set to 1 or 2"
#endif
      *(পূর্ণাঙ্ক*)pArg = iOld;
      প্রত্যাগম SQLITE_OK;
    }
    পক্ষ SQLITE_FCNTL_BLOCK_ON_CONNECT: {
      পূর্ণাঙ্ক iNew = *(পূর্ণাঙ্ক*)pArg;
      pFile->bBlockOnConnect = iNew;
      প্রত্যাগম SQLITE_OK;
    }
#endif /* SQLITE_ENABLE_SETLK_TIMEOUT */
#যদি SQLITE_MAX_MMAP_SIZE>0
    পক্ষ SQLITE_FCNTL_MMAP_SIZE: {
      i64 newLimit = *(i64*)pArg;
      পূর্ণাঙ্ক rc = SQLITE_OK;
      যদি( newLimit>sqlite3GlobalConfig.mxMmap ){
        newLimit = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (size_t) and passed
      ** to mmap(). Restrict its value to 2GB if (size_t) is not at least a
      ** 64-bit type. */
      যদি( newLimit>0 && পরিমাণ(size_t)<8 ){
        newLimit = (newLimit & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmapSizeMax;
      যদি( newLimit>=0 && newLimit!=pFile->mmapSizeMax && pFile->nFetchOut==0 ){
        pFile->mmapSizeMax = newLimit;
        যদি( pFile->mmapSize>0 ){
          unixUnmapfile(pFile);
          rc = unixMapfile(pFile, -1);
        }
      }
      প্রত্যাগম rc;
    }
#endif
#ifdef SQLITE_DEBUG
    /* The pager calls this method to signal that it has done
    ** a rollback and that the database is therefore unchanged and
    ** it hence it is OK for the transaction change counter to be
    ** unchanged.
    */
    পক্ষ SQLITE_FCNTL_DB_UNCHANGED: {
      ((unixFile*)id)->dbUpdate = 0;
      প্রত্যাগম SQLITE_OK;
    }
#endif
#যদি SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    পক্ষ SQLITE_FCNTL_SET_LOCKPROXYFILE:
    পক্ষ SQLITE_FCNTL_GET_LOCKPROXYFILE: {
      প্রত্যাগম proxyFileControl(id,op,pArg);
    }
#endif /* SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__) */

    পক্ষ SQLITE_FCNTL_EXTERNAL_READER: {
#যদি !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
      প্রত্যাগম unixFcntlExternalReader((unixFile*)id, (পূর্ণাঙ্ক*)pArg);
#অন্যথা
      *(পূর্ণাঙ্ক*)pArg = 0;
      প্রত্যাগম SQLITE_OK;
#endif
    }

#যদি defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    পক্ষ SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      বর্ণ aLck[16];
      unixInodeInfo *pInode;
      স্থির ধ্রুব বর্ণ *azLock[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
      sqlite3_str_appendf(pStr, "{\"h\":%d", pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      যদি( pFile->eFileLock ){
        sqlite3_str_appendf(pStr, ",\"eFileLock\":\"%s\"", 
                                  azLock[pFile->eFileLock-1]);
        যদি( unixPosixAdvisoryLocks(pFile->h, aLck)==SQLITE_OK ){
          sqlite3_str_appendf(pStr, ",\"pal\":\"%s\"", aLck);
        }
      }
      unixEnterMutex();
      যদি( pFile->pShm ){
        sqlite3_str_appendall(pStr, ",\"shm\":");
        unixDescribeShm(pStr, pFile->pShm);
      }
#যদি SQLITE_MAX_MMAP_SIZE>0
      যদি( pFile->mmapSize ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmapSize);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->nFetchOut);
      }
#endif
      যদি( (pInode = pFile->pInode)!=0 ){
        sqlite3_str_appendf(pStr, ",\"inode\":{\"nRef\":%d",pInode->nRef);
        sqlite3_mutex_enter(pInode->pLockMutex);
        sqlite3_str_appendf(pStr, ",\"nShared\":%d", pInode->nShared);
        যদি( pInode->eFileLock ){
          sqlite3_str_appendf(pStr, ",\"eFileLock\":\"%s\"", 
                                  azLock[pInode->eFileLock-1]);
        }
        যদি( pInode->pUnused ){
          বর্ণ cSep = '[';
          UnixUnusedFd *pUFd = pFile->pInode->pUnused;
          sqlite3_str_appendall(pStr, ",\"unusedFd\":");
          যাবত্( pUFd ){
            sqlite3_str_appendf(pStr, "%c{\"fd\":%d,\"flags\":%d",
                                cSep, pUFd->fd, pUFd->flags);
            cSep = ',';
            যদি( unixPosixAdvisoryLocks(pUFd->fd, aLck)==SQLITE_OK ){
              sqlite3_str_appendf(pStr, ",\"pal\":\"%s\"", aLck);
            }
            sqlite3_str_append(pStr, "}", 1);
            pUFd = pUFd->pNext;
          }
          sqlite3_str_append(pStr, "]", 1);
        }
        sqlite3_mutex_leave(pInode->pLockMutex);
        sqlite3_str_append(pStr, "}", 1);
      }
      unixLeaveMutex();
      sqlite3_str_append(pStr, "}", 1);
      প্রত্যাগম SQLITE_OK;
    }
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */
  }
  প্রত্যাগম SQLITE_NOTFOUND;
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
#ifndef __QNXNTO__
স্থির শূন্য setDeviceCharacteristics(unixFile *pFd){
  প্রতিজ্ঞা( pFd->deviceCharacteristics==0 || pFd->sectorSize!=0 );
  যদি( pFd->sectorSize==0 ){
#যদি defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
    পূর্ণাঙ্ক res;
    u32 f = 0;

    /* Check for support for F2FS atomic batch writes. */
    res = osIoctl(pFd->h, F2FS_IOC_GET_FEATURES, &f);
    যদি( res==0 && (f & F2FS_FEATURE_ATOMIC_WRITE) ){
      pFd->deviceCharacteristics = SQLITE_IOCAP_BATCH_ATOMIC;
    }
#endif /* __linux__ && SQLITE_ENABLE_BATCH_ATOMIC_WRITE */

    /* Set the POWERSAFE_OVERWRITE flag if requested. */
    যদি( pFd->ctrlFlags & UNIXFILE_PSOW ){
      pFd->deviceCharacteristics |= SQLITE_IOCAP_POWERSAFE_OVERWRITE;
    }
    pFd->deviceCharacteristics |= SQLITE_IOCAP_SUBPAGE_READ;

    pFd->sectorSize = SQLITE_DEFAULT_SECTOR_SIZE;
  }
}
#অন্যথা
#অন্তর্ভাব <sys/dcmd_blk.h>
#অন্তর্ভাব <sys/statvfs.h>
স্থির শূন্য setDeviceCharacteristics(unixFile *pFile){
  যদি( pFile->sectorSize == 0 ){
    সংরচনা statvfs fsInfo;

    /* Set defaults for non-supported filesystems */
    pFile->sectorSize = SQLITE_DEFAULT_SECTOR_SIZE;
    pFile->deviceCharacteristics = 0;
    যদি( fstatvfs(pFile->h, &fsInfo) == -1 ) {
      প্রত্যাগম;
    }

    যদি( !strcmp(fsInfo.f_basetype, "tmp") ) {
      pFile->sectorSize = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC4K |       /* All ram filesystem writes are atomic */
        SQLITE_IOCAP_SAFE_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }অন্যথা যদি( strstr(fsInfo.f_basetype, "etfs") ){
      pFile->sectorSize = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* etfs cluster size writes are atomic */
        (pFile->sectorSize / 512 * SQLITE_IOCAP_ATOMIC512) |
        SQLITE_IOCAP_SAFE_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }অন্যথা যদি( !strcmp(fsInfo.f_basetype, "qnx6") ){
      pFile->sectorSize = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC |         /* All filesystem writes are atomic */
        SQLITE_IOCAP_SAFE_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }অন্যথা যদি( !strcmp(fsInfo.f_basetype, "qnx4") ){
      pFile->sectorSize = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* full bitset of atomics from max sector size and smaller */
        (((pFile->sectorSize / 512 * SQLITE_IOCAP_ATOMIC512) << 1) - 2) |
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }অন্যথা যদি( strstr(fsInfo.f_basetype, "dos") ){
      pFile->sectorSize = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* full bitset of atomics from max sector size and smaller */
        (((pFile->sectorSize / 512 * SQLITE_IOCAP_ATOMIC512) << 1) - 2) |
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }অন্যথা{
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC512 |      /* blocks are atomic */
        SQLITE_IOCAP_SAFE_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        0;
    }
  }
  /* Last chance verification.  If the sector size isn't a multiple of 512
  ** then it isn't valid.*/
  যদি( pFile->sectorSize % 512 != 0 ){
    pFile->deviceCharacteristics = 0;
    pFile->sectorSize = SQLITE_DEFAULT_SECTOR_SIZE;
  }
}
#endif

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
স্থির পূর্ণাঙ্ক unixSectorSize(sqlite3_file *id){
  unixFile *pFd = (unixFile*)id;
  setDeviceCharacteristics(pFd);
  প্রত্যাগম pFd->sectorSize;
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
স্থির পূর্ণাঙ্ক unixDeviceCharacteristics(sqlite3_file *id){
  unixFile *pFd = (unixFile*)id;
  setDeviceCharacteristics(pFd);
  প্রত্যাগম pFd->deviceCharacteristics;
}

#যদি !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0

/*
** Return the system page size.
**
** This function should not be called directly by other code in this file.
** Instead, it should be called via macro osGetpagesize().
*/
স্থির পূর্ণাঙ্ক unixGetpagesize(শূন্য){
#যদি OS_VXWORKS
  প্রত্যাগম 1024;
#নোচেদ্যদি defined(_BSD_SOURCE)
  প্রত্যাগম getpagesize();
#অন্যথা
  প্রত্যাগম (পূর্ণাঙ্ক)sysconf(_SC_PAGESIZE);
#endif
}

#endif /* !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0 */

#যদি !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)

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
সংরচনা unix_Shm_গ্রন্থি {
  unixInodeInfo *pInode;     /* unixInodeInfo that owns this SHM node */
  sqlite3_mutex *pShmMutex;  /* Mutex to access this object */
  বর্ণ *zFilename;           /* Name of the mmapped file */
  পূর্ণাঙ্ক hShm;                  /* Open file descriptor */
  পূর্ণাঙ্ক szRegion;              /* Size of shared-memory regions */
  u16 nRegion;               /* Size of array apRegion */
  u8 isReadonly;             /* True if read-only */
  u8 isUnlocked;             /* True if no DMS lock held */
  বর্ণ **apRegion;           /* Array of mapped shared-memory regions */
  পূর্ণাঙ্ক nRef;                  /* Number of unixShm objects pointing to this */
  unixShm *pFirst;           /* All unixShm objects pointing to this */
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  sqlite3_mutex *aMutex[SQLITE_SHM_NLOCK];
#endif
  পূর্ণাঙ্ক aLock[SQLITE_SHM_NLOCK];  /* # shared locks on slot, -1==excl lock */
#ifdef SQLITE_DEBUG
  u8 nextShmId;              /* Next available unixShm.id value */
#endif
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
সংরচনা unixShm {
  unix_Shm_গ্রন্থি *p_Shm_গ্রন্থি;     /* The underlying unixShmNode object */
  unixShm *pNext;            /* Next unixShm with the same unixShmNode */
  u8 hasMutex;               /* True if holding the unixShmNode->pShmMutex */
  u8 id;                     /* Id of this connection within its unixShmNode */
  u16 sharedMask;            /* Mask of shared locks held */
  u16 exclMask;              /* Mask of exclusive locks held */
};

/*
** Constants used for locking
*/
#define UNIX_SHM_BASE   ((22+SQLITE_SHM_NLOCK)*4)         /* first lock byte */
#define UNIX_SHM_DMS    (UNIX_SHM_BASE+SQLITE_SHM_NLOCK)  /* deadman switch */

#যদি defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Describe the pShm object using JSON.  Used for diagnostics only.
*/
স্থির শূন্য unixDescribeShm(sqlite3_str *pStr, unixShm *pShm){
  unix_Shm_গ্রন্থি *p_গ্রন্থি = pShm->p_Shm_গ্রন্থি;
  বর্ণ aLck[16];
  sqlite3_str_appendf(pStr, "{\"h\":%d", p_গ্রন্থি->hShm);
  প্রতিজ্ঞা( unixMutexHeld() );
  sqlite3_str_appendf(pStr, ",\"nRef\":%d", p_গ্রন্থি->nRef);
  sqlite3_str_appendf(pStr, ",\"id\":%d", pShm->id);
  sqlite3_str_appendf(pStr, ",\"sharedMask\":%d", pShm->sharedMask);
  sqlite3_str_appendf(pStr, ",\"exclMask\":%d", pShm->exclMask);
  যদি( unixPosixAdvisoryLocks(p_গ্রন্থি->hShm, aLck)==SQLITE_OK ){
    sqlite3_str_appendf(pStr, ",\"pal\":\"%s\"", aLck);
  }
  sqlite3_str_append(pStr, "}", 1);
}
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

/*
** Use F_GETLK to check whether or not there are any readers with open
** wal-mode transactions in other processes on database file pFile. If
** no error occurs, return SQLITE_OK and set (*piOut) to 1 if there are
** such transactions, or 0 otherwise. If an error occurs, return an
** SQLite error code. The final value of *piOut is undefined in this
** case.
*/
স্থির পূর্ণাঙ্ক unixFcntlExternalReader(unixFile *pFile, পূর্ণাঙ্ক *piOut){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  *piOut = 0;
  যদি( pFile->pShm){
    unix_Shm_গ্রন্থি *p_Shm_গ্রন্থি = pFile->pShm->p_Shm_গ্রন্থি;
    সংরচনা flock f;

    memset(&f, 0, পরিমাণ(f));
    f.l_type = F_WRLCK;
    f.l_whence = SEEK_SET;
    f.l_start = UNIX_SHM_BASE + 3;
    f.l_len = SQLITE_SHM_NLOCK - 3;

    sqlite3_mutex_enter(p_Shm_গ্রন্থি->pShmMutex);
    যদি( osFcntl(p_Shm_গ্রন্থি->hShm, F_GETLK, &f)<0 ){
      rc = SQLITE_IOERR_LOCK;
    }অন্যথা{
      *piOut = (f.l_type!=F_UNLCK);
    }
    sqlite3_mutex_leave(p_Shm_গ্রন্থি->pShmMutex);
  }

  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক unix_Is_Sharing_Shm_গ্রন্থি(unixFile *pFile){
  unix_Shm_গ্রন্থি *p_Shm_গ্রন্থি;
  সংরচনা flock lock;
  যদি( pFile->pShm==0 ) প্রত্যাগম 0;
  যদি( pFile->ctrlFlags & UNIXFILE_EXCL ) প্রত্যাগম 0;
  p_Shm_গ্রন্থি = pFile->pShm->p_Shm_গ্রন্থি;
#যদি SQLITE_ATOMIC_INTRINSICS
  প্রতিজ্ঞা( AtomicLoad(&p_Shm_গ্রন্থি->nRef)==1 );
#endif
  memset(&lock, 0, পরিমাণ(lock));
  lock.l_whence = SEEK_SET;
  lock.l_start = UNIX_SHM_DMS;
  lock.l_len = 1;
  lock.l_type = F_WRLCK;
  osFcntl(p_Shm_গ্রন্থি->hShm, F_GETLK, &lock);
  প্রত্যাগম (lock.l_type!=F_UNLCK);
}

/*
** Apply posix advisory locks for all bytes from ofst through ofst+n-1.
**
** Locks block if the mask is exactly UNIX_SHM_C and are non-blocking
** otherwise.
*/
স্থির পূর্ণাঙ্ক unixShmSystemLock(
  unixFile *pFile,       /* Open connection to the WAL file */
  পূর্ণাঙ্ক lockType,          /* F_UNLCK, F_RDLCK, or F_WRLCK */
  পূর্ণাঙ্ক ofst,              /* First byte of the locking range */
  পূর্ণাঙ্ক n                  /* Number of bytes to lock */
){
  unix_Shm_গ্রন্থি *p_Shm_গ্রন্থি; /* Apply locks to this open shared-memory segment */
  সংরচনা flock f;        /* The posix advisory locking structure */
  পূর্ণাঙ্ক rc = SQLITE_OK;    /* Result code form fcntl() */

  p_Shm_গ্রন্থি = pFile->pInode->p_Shm_গ্রন্থি;

  /* Assert that the parameters are within expected range and that the
  ** correct mutex or mutexes are held. */
  প্রতিজ্ঞা( p_Shm_গ্রন্থি->nRef>=0 );
  প্রতিজ্ঞা( (ofst==UNIX_SHM_DMS && n==1)
       || (ofst>=UNIX_SHM_BASE && ofst+n<=(UNIX_SHM_BASE+SQLITE_SHM_NLOCK))
  );
  যদি( ofst==UNIX_SHM_DMS ){
    প্রতিজ্ঞা( p_Shm_গ্রন্থি->nRef>0 || unixMutexHeld() );
    প্রতিজ্ঞা( p_Shm_গ্রন্থি->nRef==0 || sqlite3_mutex_held(p_Shm_গ্রন্থি->pShmMutex) );
  }অন্যথা{
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    পূর্ণাঙ্ক ii;
    প্রতি(ii=ofst-UNIX_SHM_BASE; ii<ofst-UNIX_SHM_BASE+n; ii++){
      প্রতিজ্ঞা( sqlite3_mutex_held(p_Shm_গ্রন্থি->aMutex[ii]) );
    }
#অন্যথা
    প্রতিজ্ঞা( sqlite3_mutex_held(p_Shm_গ্রন্থি->pShmMutex) );
    প্রতিজ্ঞা( p_Shm_গ্রন্থি->nRef>0 );
#endif
  }

  /* Shared locks never span more than one byte */
  প্রতিজ্ঞা( n==1 || lockType!=F_RDLCK );

  /* Locks are within range */
  প্রতিজ্ঞা( n>=1 && n<=SQLITE_SHM_NLOCK );
  প্রতিজ্ঞা( ofst>=UNIX_SHM_BASE && ofst<=UNIX_SHM_DMS );
  প্রতিজ্ঞা( ofst+n-1<=UNIX_SHM_DMS );

  যদি( p_Shm_গ্রন্থি->hShm>=0 ){
    পূর্ণাঙ্ক res;
    /* Initialize the locking parameters */
    f.l_type = lockType;
    f.l_whence = SEEK_SET;
    f.l_start = ofst;
    f.l_len = n;
    res = osSetPosixAdvisoryLock(p_Shm_গ্রন্থি->hShm, &f, pFile);
    যদি( res==-1 ){
#যদি defined(SQLITE_ENABLE_SETLK_TIMEOUT) && SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = (pFile->iBusyTimeout ? SQLITE_BUSY_TIMEOUT : SQLITE_BUSY);
#অন্যথা
      rc = SQLITE_BUSY;
#endif
    }
  }

  /* Do debug tracing */
#ifdef SQLITE_DEBUG
  OSTRACE(("SHM-LOCK "));
  যদি( rc==SQLITE_OK ){
    যদি( lockType==F_UNLCK ){
      OSTRACE(("unlock %d..%d ok\n", ofst, ofst+n-1));
    }অন্যথা যদি( lockType==F_RDLCK ){
      OSTRACE(("read-lock %d..%d ok\n", ofst, ofst+n-1));
    }অন্যথা{
      প্রতিজ্ঞা( lockType==F_WRLCK );
      OSTRACE(("write-lock %d..%d ok\n", ofst, ofst+n-1));
    }
  }অন্যথা{
    যদি( lockType==F_UNLCK ){
      OSTRACE(("unlock %d..%d failed\n", ofst, ofst+n-1));
    }অন্যথা যদি( lockType==F_RDLCK ){
      OSTRACE(("read-lock %d..%d failed\n", ofst, ofst+n-1));
    }অন্যথা{
      প্রতিজ্ঞা( lockType==F_WRLCK );
      OSTRACE(("write-lock %d..%d failed\n", ofst, ofst+n-1));
    }
  }
#endif

  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক unixShmRegionPerMap(শূন্য){
  পূর্ণাঙ্ক shmsz = 32*1024;            /* SHM region size */
  পূর্ণাঙ্ক pgsz = osGetpagesize();   /* System page size */
  প্রতিজ্ঞা( ((pgsz-1)&pgsz)==0 );   /* Page size must be a power of 2 */
  যদি( pgsz<shmsz ) প্রত্যাগম 1;
  প্রত্যাগম pgsz/shmsz;
}

/*
** Purge the unixShmNodeList list of all entries with unixShmNode.nRef==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
স্থির শূন্য unixShmPurge(unixFile *pFd){
  unix_Shm_গ্রন্থি *p = pFd->pInode->p_Shm_গ্রন্থি;
  প্রতিজ্ঞা( unixMutexHeld() );
  যদি( p && ALWAYS(p->nRef==0) ){
    পূর্ণাঙ্ক nShmPerMap = unixShmRegionPerMap();
    পূর্ণাঙ্ক i;
    প্রতিজ্ঞা( p->pInode==pFd->pInode );
    sqlite3_mutex_free(p->pShmMutex);
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    প্রতি(i=0; i<SQLITE_SHM_NLOCK; i++){
      sqlite3_mutex_free(p->aMutex[i]);
    }
#endif
    প্রতি(i=0; i<p->nRegion; i+=nShmPerMap){
      যদি( p->hShm>=0 ){
        osMunmap(p->apRegion[i], p->szRegion);
      }অন্যথা{
        sqlite3_free(p->apRegion[i]);
      }
    }
    sqlite3_free(p->apRegion);
    যদি( p->hShm>=0 ){
      robust_close(pFd, p->hShm, __LINE__);
      p->hShm = -1;
    }
    p->pInode->p_Shm_গ্রন্থি = 0;
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
স্থির পূর্ণাঙ্ক unixLockSharedMemory(unixFile *pDbFd, unix_Shm_গ্রন্থি *p_Shm_গ্রন্থি){
  সংরচনা flock lock;
  পূর্ণাঙ্ক rc = SQLITE_OK;

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
  lock.l_whence = SEEK_SET;
  lock.l_start = UNIX_SHM_DMS;
  lock.l_len = 1;
  lock.l_type = F_WRLCK;
  যদি( osFcntl(p_Shm_গ্রন্থি->hShm, F_GETLK, &lock)!=0 ) {
    rc = SQLITE_IOERR_LOCK;
  }অন্যথা যদি( lock.l_type==F_UNLCK ){
    যদি( p_Shm_গ্রন্থি->isReadonly ){
      p_Shm_গ্রন্থি->isUnlocked = 1;
      rc = SQLITE_READONLY_CANTINIT;
    }অন্যথা{
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
      /* Do not use a blocking lock here. If the lock cannot be obtained
      ** immediately, it means some other connection is truncating the
      ** *-shm file. And after it has done so, it will not release its
      ** lock, but only downgrade it to a shared lock. So no point in
      ** blocking here. The call below to obtain the shared DMS lock may 
      ** use a blocking lock. */
      পূর্ণাঙ্ক iSaveTimeout = pDbFd->iBusyTimeout;
      pDbFd->iBusyTimeout = 0;
#endif
      rc = unixShmSystemLock(pDbFd, F_WRLCK, UNIX_SHM_DMS, 1);
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
      pDbFd->iBusyTimeout = iSaveTimeout;
#endif
      /* The first connection to attach must truncate the -shm file.  We
      ** truncate to 3 bytes (an arbitrary small number, less than the
      ** -shm header size) rather than 0 as a system debugging aid, to
      ** help detect if a -shm file truncation is legitimate or is the work
      ** or a rogue process. */
      যদি( rc==SQLITE_OK && robust_ftruncate(p_Shm_গ্রন্থি->hShm, 3) ){
        rc = unixLogError(SQLITE_IOERR_SHMOPEN,"ftruncate",p_Shm_গ্রন্থি->zFilename);
      }
    }
  }অন্যথা যদি( lock.l_type==F_WRLCK ){
    rc = SQLITE_BUSY;
  }

  যদি( rc==SQLITE_OK ){
    প্রতিজ্ঞা( lock.l_type==F_UNLCK || lock.l_type==F_RDLCK );
    rc = unixShmSystemLock(pDbFd, F_RDLCK, UNIX_SHM_DMS, 1);
  }
  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক unixOpenSharedMemory(unixFile *pDbFd){
  সংরচনা unixShm *p = 0;          /* The connection to be opened */
  সংরচনা unix_Shm_গ্রন্থি *p_Shm_গ্রন্থি;   /* The underlying mmapped file */
  পূর্ণাঙ্ক rc = SQLITE_OK;             /* Result code */
  unixInodeInfo *pInode;          /* The inode of fd */
  বর্ণ *zShm;             /* Name of the file used for SHM */
  পূর্ণাঙ্ক nShmFilename;               /* Size of the SHM filename in bytes */

  /* Allocate space for the new unixShm object. */
  p = sqlite3_malloc64( পরিমাণ(*p) );
  যদি( p==0 ) প্রত্যাগম SQLITE_NOMEM_BKPT;
  memset(p, 0, পরিমাণ(*p));
  প্রতিজ্ঞা( pDbFd->pShm==0 );

  /* Check to see if a unixShmNode object already exists. Reuse an existing
  ** one if present. Create a new one if necessary.
  */
  প্রতিজ্ঞা( unixFileMutexNotheld(pDbFd) );
  unixEnterMutex();
  pInode = pDbFd->pInode;
  p_Shm_গ্রন্থি = pInode->p_Shm_গ্রন্থি;
  যদি( p_Shm_গ্রন্থি==0 ){
    সংরচনা stat sStat;                 /* fstat() info for database file */
#ifndef SQLITE_SHM_DIRECTORY
    ধ্রুব বর্ণ *zBasePath = pDbFd->zPath;
#endif

    /* Call fstat() to figure out the permissions on the database file. If
    ** a new *-shm file is created, an attempt will be made to create it
    ** with the same permissions.
    */
    যদি( osFstat(pDbFd->h, &sStat) ){
      rc = SQLITE_IOERR_FSTAT;
      গচ্ছ shm_open_err;
    }

#ifdef SQLITE_SHM_DIRECTORY
    nShmFilename = পরিমাণ(SQLITE_SHM_DIRECTORY) + 31;
#অন্যথা
    nShmFilename = 6 + (পূর্ণাঙ্ক)strlen(zBasePath);
#endif
    p_Shm_গ্রন্থি = sqlite3_malloc64( পরিমাণ(*p_Shm_গ্রন্থি) + nShmFilename );
    যদি( p_Shm_গ্রন্থি==0 ){
      rc = SQLITE_NOMEM_BKPT;
      গচ্ছ shm_open_err;
    }
    memset(p_Shm_গ্রন্থি, 0, পরিমাণ(*p_Shm_গ্রন্থি)+nShmFilename);
    zShm = p_Shm_গ্রন্থি->zFilename = (বর্ণ*)&p_Shm_গ্রন্থি[1];
#ifdef SQLITE_SHM_DIRECTORY
    sqlite3_snprintf(nShmFilename, zShm,
                     SQLITE_SHM_DIRECTORY "/sqlite-shm-%x-%x",
                     (u32)sStat.st_ino, (u32)sStat.st_dev);
#অন্যথা
    sqlite3_snprintf(nShmFilename, zShm, "%s-shm", zBasePath);
    sqlite3FileSuffix3(pDbFd->zPath, zShm);
#endif
    p_Shm_গ্রন্থি->hShm = -1;
    pDbFd->pInode->p_Shm_গ্রন্থি = p_Shm_গ্রন্থি;
    p_Shm_গ্রন্থি->pInode = pDbFd->pInode;
    যদি( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_গ্রন্থি->pShmMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      যদি( p_Shm_গ্রন্থি->pShmMutex==0 ){
        rc = SQLITE_NOMEM_BKPT;
        গচ্ছ shm_open_err;
      }
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
      {
        পূর্ণাঙ্ক ii;
        প্রতি(ii=0; ii<SQLITE_SHM_NLOCK; ii++){
          p_Shm_গ্রন্থি->aMutex[ii] = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
          যদি( p_Shm_গ্রন্থি->aMutex[ii]==0 ){
            rc = SQLITE_NOMEM_BKPT;
            গচ্ছ shm_open_err;
          }
        }
      }
#endif
    }

    যদি( pInode->bProcessLock==0 ){
      যদি( 0==sqlite3_uri_boolean(pDbFd->zPath, "readonly_shm", 0) ){
        p_Shm_গ্রন্থি->hShm = robust_open(zShm, O_RDWR|O_CREAT|O_NOFOLLOW,
                                     (sStat.st_mode&0777));
      }
      যদি( p_Shm_গ্রন্থি->hShm<0 ){
        p_Shm_গ্রন্থি->hShm = robust_open(zShm, O_RDONLY|O_NOFOLLOW,
                                     (sStat.st_mode&0777));
        যদি( p_Shm_গ্রন্থি->hShm<0 ){
          rc = unixLogError(SQLITE_CANTOPEN_BKPT, "open", zShm);
          গচ্ছ shm_open_err;
        }
        p_Shm_গ্রন্থি->isReadonly = 1;
      }

      /* If this process is running as root, make sure that the SHM file
      ** is owned by the same user that owns the original database.  Otherwise,
      ** the original owner will not be able to connect.
      */
      robustFchown(p_Shm_গ্রন্থি->hShm, sStat.st_uid, sStat.st_gid);

      rc = unixLockSharedMemory(pDbFd, p_Shm_গ্রন্থি);
      যদি( rc!=SQLITE_OK && rc!=SQLITE_READONLY_CANTINIT ) গচ্ছ shm_open_err;
    }
  }

  /* Make the new connection a child of the unixShmNode */
  p->p_Shm_গ্রন্থি = p_Shm_গ্রন্থি;
#ifdef SQLITE_DEBUG
  p->id = p_Shm_গ্রন্থি->nextShmId++;
#endif
  p_Shm_গ্রন্থি->nRef++;
  pDbFd->pShm = p;
  unixLeaveMutex();

  /* The reference count on pShmNode has already been incremented under
  ** the cover of the unixEnterMutex() mutex and the pointer from the
  ** new (struct unixShm) object to the pShmNode has been set. All that is
  ** left to do is to link the new object into the linked list starting
  ** at pShmNode->pFirst. This must be done while holding the
  ** pShmNode->pShmMutex.
  */
  sqlite3_mutex_enter(p_Shm_গ্রন্থি->pShmMutex);
  p->pNext = p_Shm_গ্রন্থি->pFirst;
  p_Shm_গ্রন্থি->pFirst = p;
  sqlite3_mutex_leave(p_Shm_গ্রন্থি->pShmMutex);
  প্রত্যাগম rc;

  /* Jump here on any error */
shm_open_err:
  unixShmPurge(pDbFd);       /* This call frees pShmNode if required */
  sqlite3_free(p);
  unixLeaveMutex();
  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক unixShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  পূর্ণাঙ্ক iRegion,                    /* Region to retrieve */
  পূর্ণাঙ্ক szRegion,                   /* Size of regions */
  পূর্ণাঙ্ক bExtend,                    /* True to extend file if necessary */
  শূন্য চঞ্চল **pp              /* OUT: Mapped memory */
){
  unixFile *pDbFd = (unixFile*)fd;
  unixShm *p;
  unix_Shm_গ্রন্থি *p_Shm_গ্রন্থি;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক nShmPerMap = unixShmRegionPerMap();
  পূর্ণাঙ্ক nReqRegion;

  /* If the shared-memory file has not yet been opened, open it now. */
  যদি( pDbFd->pShm==0 ){
    rc = unixOpenSharedMemory(pDbFd);
    যদি( rc!=SQLITE_OK ) প্রত্যাগম rc;
  }

  p = pDbFd->pShm;
  p_Shm_গ্রন্থি = p->p_Shm_গ্রন্থি;
  sqlite3_mutex_enter(p_Shm_গ্রন্থি->pShmMutex);
  যদি( p_Shm_গ্রন্থি->isUnlocked ){
    rc = unixLockSharedMemory(pDbFd, p_Shm_গ্রন্থি);
    যদি( rc!=SQLITE_OK ) গচ্ছ shmpage_out;
    p_Shm_গ্রন্থি->isUnlocked = 0;
  }
  প্রতিজ্ঞা( szRegion==p_Shm_গ্রন্থি->szRegion || p_Shm_গ্রন্থি->nRegion==0 );
  প্রতিজ্ঞা( p_Shm_গ্রন্থি->pInode==pDbFd->pInode );
  প্রতিজ্ঞা( p_Shm_গ্রন্থি->hShm>=0 || pDbFd->pInode->bProcessLock==1 );
  প্রতিজ্ঞা( p_Shm_গ্রন্থি->hShm<0 || pDbFd->pInode->bProcessLock==0 );

  /* Minimum number of regions required to be mapped. */
  nReqRegion = ((iRegion+nShmPerMap) / nShmPerMap) * nShmPerMap;

  যদি( p_Shm_গ্রন্থি->nRegion<nReqRegion ){
    বর্ণ **apNew;                         /* New apRegion[] array */
    i64 nByte = nReqRegion*(i64)szRegion; /* Minimum required file size */
    সংরচনা stat sStat;                    /* Used by fstat() */

    p_Shm_গ্রন্থি->szRegion = szRegion;

    যদি( p_Shm_গ্রন্থি->hShm>=0 ){
      /* The requested region is not mapped into this processes address space.
      ** Check to see if it has been allocated (i.e. if the wal-index file is
      ** large enough to contain the requested region).
      */
      যদি( osFstat(p_Shm_গ্রন্থি->hShm, &sStat) ){
        rc = SQLITE_IOERR_SHMSIZE;
        গচ্ছ shmpage_out;
      }

      যদি( sStat.st_size<nByte ){
        /* The requested memory region does not exist. If bExtend is set to
        ** false, exit early. *pp will be set to NULL and SQLITE_OK returned.
        */
        যদি( !bExtend ){
          গচ্ছ shmpage_out;
        }

        /* Alternatively, if bExtend is true, extend the file. Do this by
        ** writing a single byte to the end of each (OS) page being
        ** allocated or extended. Technically, we need only write to the
        ** last page in order to extend the file. But writing to all new
        ** pages forces the OS to allocate them immediately, which reduces
        ** the chances of SIGBUS while accessing the mapped region later on.
        */
        অন্যথা{
          স্থির ধ্রুব পূর্ণাঙ্ক pgsz = 4096;
          i64 iPg;

          /* Write to the last byte of each newly allocated or extended page */
          প্রতিজ্ঞা( (nByte % pgsz)==0 );
          প্রতি(iPg=(sStat.st_size/pgsz); iPg<(nByte/pgsz); iPg++){
            পূর্ণাঙ্ক x = 0;
            যদি( seekAndWriteFd(p_Shm_গ্রন্থি->hShm, iPg*pgsz + pgsz-1,"",1,&x)!=1 ){
              ধ্রুব বর্ণ *zFile = p_Shm_গ্রন্থি->zFilename;
              rc = unixLogError(SQLITE_IOERR_SHMSIZE, "write", zFile);
              গচ্ছ shmpage_out;
            }
          }
        }
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (বর্ণ **)sqlite3_realloc64(
        p_Shm_গ্রন্থি->apRegion, nReqRegion*পরিমাণ(বর্ণ *)
    );
    যদি( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      গচ্ছ shmpage_out;
    }
    p_Shm_গ্রন্থি->apRegion = apNew;
    যাবত্( p_Shm_গ্রন্থি->nRegion<nReqRegion ){
      i64 nMap = (i64)szRegion*(i64)nShmPerMap;
      i64 i;
      শূন্য *pMem;
      যদি( p_Shm_গ্রন্থি->hShm>=0 ){
        pMem = osMmap(0, nMap,
            p_Shm_গ্রন্থি->isReadonly ? PROT_READ : PROT_READ|PROT_WRITE,
            MAP_SHARED, p_Shm_গ্রন্থি->hShm, szRegion*(i64)p_Shm_গ্রন্থি->nRegion
        );
        যদি( pMem==MAP_FAILED ){
          rc = unixLogError(SQLITE_IOERR_SHMMAP, "mmap", p_Shm_গ্রন্থি->zFilename);
          গচ্ছ shmpage_out;
        }
      }অন্যথা{
        pMem = sqlite3_malloc64(nMap);
        যদি( pMem==0 ){
          rc = SQLITE_NOMEM_BKPT;
          গচ্ছ shmpage_out;
        }
        memset(pMem, 0, nMap);
      }

      প্রতি(i=0; i<nShmPerMap; i++){
        p_Shm_গ্রন্থি->apRegion[p_Shm_গ্রন্থি->nRegion+i] = &((বর্ণ*)pMem)[szRegion*i];
      }
      p_Shm_গ্রন্থি->nRegion += nShmPerMap;
    }
  }

shmpage_out:
  যদি( p_Shm_গ্রন্থি->nRegion>iRegion ){
    *pp = p_Shm_গ্রন্থি->apRegion[iRegion];
  }অন্যথা{
    *pp = 0;
  }
  যদি( p_Shm_গ্রন্থি->isReadonly && rc==SQLITE_OK ) rc = SQLITE_READONLY;
  sqlite3_mutex_leave(p_Shm_গ্রন্থি->pShmMutex);
  প্রত্যাগম rc;
}

/*
** Check that the pShmNode->aLock[] array comports with the locking bitmasks
** held by each client. Return true if it does, or false otherwise. This
** is to be used in an assert(). e.g.
**
**     assert( assertLockingArrayOk(pShmNode) );
*/
#ifdef SQLITE_DEBUG
স্থির পূর্ণাঙ্ক assertLockingArrayOk(unix_Shm_গ্রন্থি *p_Shm_গ্রন্থি){
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  প্রত্যাগম 1;
#অন্যথা
  unixShm *pX;
  পূর্ণাঙ্ক aLock[SQLITE_SHM_NLOCK];

  memset(aLock, 0, পরিমাণ(aLock));
  প্রতি(pX=p_Shm_গ্রন্থি->pFirst; pX; pX=pX->pNext){
    পূর্ণাঙ্ক i;
    প্রতি(i=0; i<SQLITE_SHM_NLOCK; i++){
      যদি( pX->exclMask & (1<<i) ){
        প্রতিজ্ঞা( aLock[i]==0 );
        aLock[i] = -1;
      }অন্যথা যদি( pX->sharedMask & (1<<i) ){
        প্রতিজ্ঞা( aLock[i]>=0 );
        aLock[i]++;
      }
    }
  }

  প্রতিজ্ঞা( 0==memcmp(p_Shm_গ্রন্থি->aLock, aLock, পরিমাণ(aLock)) );
  প্রত্যাগম (memcmp(p_Shm_গ্রন্থি->aLock, aLock, পরিমাণ(aLock))==0);
#endif
}
#endif /* !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL) */

/*
** Change the lock state for a shared-memory segment.
**
** Note that the relationship between SHARED and EXCLUSIVE locks is a little
** different here than in posix.  In xShmLock(), one can go from unlocked
** to shared and back or from unlocked to exclusive and back.  But one may
** not go from shared to exclusive or from exclusive to shared.
*/
স্থির পূর্ণাঙ্ক unixShmLock(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  পূর্ণাঙ্ক ofst,                  /* First lock to acquire or release */
  পূর্ণাঙ্ক n,                     /* Number of locks to acquire or release */
  পূর্ণাঙ্ক flags                  /* What to do with the lock */
){
  unixFile *pDbFd = (unixFile*)fd;      /* Connection holding shared memory */
  unixShm *p;                           /* The shared memory being locked */
  unix_Shm_গ্রন্থি *p_Shm_গ্রন্থি;                /* The underlying file iNode */
  পূর্ণাঙ্ক rc = SQLITE_OK;                   /* Result code */
  u16 mask = (1<<(ofst+n)) - (1<<ofst); /* Mask of locks to take or release */
  পূর্ণাঙ্ক *aLock;

  p = pDbFd->pShm;
  যদি( p==0 ) প্রত্যাগম SQLITE_IOERR_SHMLOCK;
  p_Shm_গ্রন্থি = p->p_Shm_গ্রন্থি;
  যদি( NEVER(p_Shm_গ্রন্থি==0) ) প্রত্যাগম SQLITE_IOERR_SHMLOCK;
  aLock = p_Shm_গ্রন্থি->aLock;

  প্রতিজ্ঞা( p_Shm_গ্রন্থি==pDbFd->pInode->p_Shm_গ্রন্থি );
  প্রতিজ্ঞা( p_Shm_গ্রন্থি->pInode==pDbFd->pInode );
  প্রতিজ্ঞা( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  প্রতিজ্ঞা( n>=1 );
  প্রতিজ্ঞা( flags==(SQLITE_SHM_LOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_LOCK | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  প্রতিজ্ঞা( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );
  প্রতিজ্ঞা( p_Shm_গ্রন্থি->hShm>=0 || pDbFd->pInode->bProcessLock==1 );
  প্রতিজ্ঞা( p_Shm_গ্রন্থি->hShm<0 || pDbFd->pInode->bProcessLock==0 );

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
#যদি defined(SQLITE_ENABLE_SETLK_TIMEOUT) && defined(SQLITE_DEBUG)
  {
    u16 lockMask = (p->exclMask|p->sharedMask);
    প্রতিজ্ঞা( (flags & SQLITE_SHM_UNLOCK) || pDbFd->iBusyTimeout==0 || (
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
  ** This is assert()ed below.
  */
  প্রতিজ্ঞা( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK) 
       || 0==(p->exclMask & mask) 
  );
  যদি( ((flags & SQLITE_SHM_UNLOCK) && ((p->exclMask|p->sharedMask) & mask))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_LOCK) && 0==(p->sharedMask & mask))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK))
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
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    পূর্ণাঙ্ক iMutex;
    প্রতি(iMutex=ofst; iMutex<ofst+n; iMutex++){
      যদি( flags==(SQLITE_SHM_LOCK|SQLITE_SHM_EXCLUSIVE) ){
        rc = sqlite3_mutex_try(p_Shm_গ্রন্থি->aMutex[iMutex]);
        যদি( rc!=SQLITE_OK ) গচ্ছ leave_shmnode_mutexes;
      }অন্যথা{
        sqlite3_mutex_enter(p_Shm_গ্রন্থি->aMutex[iMutex]);
      }
    }
#অন্যথা
    sqlite3_mutex_enter(p_Shm_গ্রন্থি->pShmMutex);
#endif

    যদি( ALWAYS(rc==SQLITE_OK) ){
      যদি( flags & SQLITE_SHM_UNLOCK ){
        /* Case (a) - unlock.  */
        পূর্ণাঙ্ক bUnlock = 1;
        প্রতিজ্ঞা( (p->exclMask & p->sharedMask)==0 );
        প্রতিজ্ঞা( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->exclMask & mask)==mask );
        প্রতিজ্ঞা( !(flags & SQLITE_SHM_SHARED) || (p->sharedMask & mask)==mask );
  
        /* If this is a SHARED lock being unlocked, it is possible that other
        ** clients within this process are holding the same SHARED lock. In
        ** this case, set bUnlock to 0 so that the posix lock is not removed
        ** from the file-descriptor below.  */
        যদি( flags & SQLITE_SHM_SHARED ){
          প্রতিজ্ঞা( n==1 );
          প্রতিজ্ঞা( aLock[ofst]>=1 );
          যদি( aLock[ofst]>1 ){
            bUnlock = 0;
            aLock[ofst]--;
            p->sharedMask &= ~mask;
          }
        }
  
        যদি( bUnlock ){
          rc = unixShmSystemLock(pDbFd, F_UNLCK, ofst+UNIX_SHM_BASE, n);
          যদি( rc==SQLITE_OK ){
            memset(&aLock[ofst], 0, পরিমাণ(পূর্ণাঙ্ক)*n);
            p->sharedMask &= ~mask;
            p->exclMask &= ~mask;
          }
        }
      }অন্যথা যদি( flags & SQLITE_SHM_SHARED ){
        /* Case (b) - a shared lock.  */
  
        যদি( aLock[ofst]<0 ){
          /* An exclusive lock is held by some other connection. BUSY. */ 
          rc = SQLITE_BUSY;
        }অন্যথা যদি( aLock[ofst]==0 ){
          rc = unixShmSystemLock(pDbFd, F_RDLCK, ofst+UNIX_SHM_BASE, n);
        }
  
        /* Get the local shared locks */
        যদি( rc==SQLITE_OK ){
          p->sharedMask |= mask;
          aLock[ofst]++;
        }
      }অন্যথা{
        /* Case (c) - an exclusive lock.  */
        পূর্ণাঙ্ক ii;
  
        প্রতিজ্ঞা( flags==(SQLITE_SHM_LOCK|SQLITE_SHM_EXCLUSIVE) );
        প্রতিজ্ঞা( (p->sharedMask & mask)==0 );
        প্রতিজ্ঞা( (p->exclMask & mask)==0 );
  
        /* Make sure no sibling connections hold locks that will block this
        ** lock.  If any do, return SQLITE_BUSY right away.  */
        প্রতি(ii=ofst; ii<ofst+n; ii++){
          যদি( aLock[ii] ){
            rc = SQLITE_BUSY;
            ভঙ্গ;
          }
        }
  
        /* Get the exclusive locks at the system level. Then if successful
        ** also update the in-memory values. */
        যদি( rc==SQLITE_OK ){
          rc = unixShmSystemLock(pDbFd, F_WRLCK, ofst+UNIX_SHM_BASE, n);
          যদি( rc==SQLITE_OK ){
            p->exclMask |= mask;
            প্রতি(ii=ofst; ii<ofst+n; ii++){
              aLock[ii] = -1;
            }
          }
        }
      }
      প্রতিজ্ঞা( assertLockingArrayOk(p_Shm_গ্রন্থি) );
    }

    /* Drop the mutexes acquired above. */
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  leave_shmnode_mutexes:
    প্রতি(iMutex--; iMutex>=ofst; iMutex--){
      sqlite3_mutex_leave(p_Shm_গ্রন্থি->aMutex[iMutex]);
    }
#অন্যথা
    sqlite3_mutex_leave(p_Shm_গ্রন্থি->pShmMutex);
#endif
  }

  OSTRACE(("SHM-LOCK shmid-%d, pid-%d got %03x,%03x\n",
           p->id, osGetpid(0), p->sharedMask, p->exclMask));
  প্রত্যাগম rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
স্থির শূন্য unixShmBarrier(
  sqlite3_file *fd                /* Database file holding the shared memory */
){
  UNUSED_PARAMETER(fd);
  sqlite3MemoryBarrier();         /* compiler-defined memory barrier */
  প্রতিজ্ঞা( fd->pMethods->xLock==nolockLock
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
স্থির পূর্ণাঙ্ক unixShmUnmap(
  sqlite3_file *fd,               /* The underlying database file */
  পূর্ণাঙ্ক deleteFlag                  /* Delete shared-memory if true */
){
  unixShm *p;                     /* The connection to be closed */
  unix_Shm_গ্রন্থি *p_Shm_গ্রন্থি;          /* The underlying shared-memory file */
  unixShm **pp;                   /* For looping over sibling connections */
  unixFile *pDbFd;                /* The underlying database file */

  pDbFd = (unixFile*)fd;
  p = pDbFd->pShm;
  যদি( p==0 ) প্রত্যাগম SQLITE_OK;
  p_Shm_গ্রন্থি = p->p_Shm_গ্রন্থি;

  প্রতিজ্ঞা( p_Shm_গ্রন্থি==pDbFd->pInode->p_Shm_গ্রন্থি );
  প্রতিজ্ঞা( p_Shm_গ্রন্থি->pInode==pDbFd->pInode );

  /* Remove connection p from the set of connections associated
  ** with pShmNode */
  sqlite3_mutex_enter(p_Shm_গ্রন্থি->pShmMutex);
  প্রতি(pp=&p_Shm_গ্রন্থি->pFirst; (*pp)!=p; pp = &(*pp)->pNext){}
  *pp = p->pNext;

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  sqlite3_mutex_leave(p_Shm_গ্রন্থি->pShmMutex);

  /* If pShmNode->nRef has reached 0, then close the underlying
  ** shared-memory file, too */
  প্রতিজ্ঞা( unixFileMutexNotheld(pDbFd) );
  unixEnterMutex();
  প্রতিজ্ঞা( p_Shm_গ্রন্থি->nRef>0 );
  p_Shm_গ্রন্থি->nRef--;
  যদি( p_Shm_গ্রন্থি->nRef==0 ){
    যদি( deleteFlag && p_Shm_গ্রন্থি->hShm>=0 ){
      osUnlink(p_Shm_গ্রন্থি->zFilename);
    }
    unixShmPurge(pDbFd);
  }
  unixLeaveMutex();

  প্রত্যাগম SQLITE_OK;
}


#অন্যথা
# define unixShmMap     0
# define unixShmLock    0
# define unixShmBarrier 0
# define unixShmUnmap   0
#endif /* #ifndef SQLITE_OMIT_WAL */

#যদি SQLITE_MAX_MMAP_SIZE>0
/*
** If it is currently memory mapped, unmap file pFd.
*/
স্থির শূন্য unixUnmapfile(unixFile *pFd){
  প্রতিজ্ঞা( pFd->nFetchOut==0 );
  যদি( pFd->pMapRegion ){
    osMunmap(pFd->pMapRegion, pFd->mmapSizeActual);
    pFd->pMapRegion = 0;
    pFd->mmapSize = 0;
    pFd->mmapSizeActual = 0;
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
স্থির শূন্য unixRemapfile(
  unixFile *pFd,                  /* File descriptor object */
  i64 nNew                        /* Required mapping size */
){
  ধ্রুব বর্ণ *zErr = "mmap";
  পূর্ণাঙ্ক h = pFd->h;                      /* File descriptor open on db file */
  u8 *pOrig = (u8 *)pFd->pMapRegion;   /* Pointer to current file mapping */
  u8 *pNew = 0;                        /* Location of new mapping */
  পূর্ণাঙ্ক flags = PROT_READ;               /* Flags to pass to mmap() */

  প্রতিজ্ঞা( pFd->nFetchOut==0 );
  প্রতিজ্ঞা( nNew>pFd->mmapSize );
  প্রতিজ্ঞা( nNew<=pFd->mmapSizeMax );
  প্রতিজ্ঞা( nNew>0 );
  প্রতিজ্ঞা( pFd->mmapSizeActual>=pFd->mmapSize );
  প্রতিজ্ঞা( MAP_FAILED!=0 );

#ifdef SQLITE_MMAP_READWRITE
  যদি( (pFd->ctrlFlags & UNIXFILE_RDONLY)==0 ) flags |= PROT_WRITE;
#endif

  যদি( pOrig ){
    i64 nReuse = pFd->mmapSize & ~((পূর্ণাঙ্ক)osGetpagesize()-1);

    /* Unmap any part of the current mapping that will not be reused */
    যদি( nReuse<pFd->mmapSizeActual ){
      osMunmap(&pOrig[nReuse], pFd->mmapSizeActual - nReuse);
    }

    /* If nReuse is now zero, then the entire mapping was unmapped by the
    ** call above. In this case do not attempt to extend pOrig. */
    যদি( nReuse>0 ){
#যদি HAVE_MREMAP
      pNew = osMremap(pOrig, nReuse, nNew, MREMAP_MAYMOVE);
      zErr = "mremap";
#অন্যথা
      pNew = osMmap(&pOrig[nReuse], nNew-nReuse, flags, MAP_SHARED, h, nReuse);
      যদি( pNew!=MAP_FAILED ){
        যদি( pNew!=&pOrig[nReuse] ){
          osMunmap(pNew, nNew - nReuse);
          pNew = 0;
        }অন্যথা{
          pNew = pOrig;
        }
      }
#endif

      /* The attempt to extend the existing mapping failed. Free it. */
      যদি( pNew==MAP_FAILED || pNew==0 ){
        osMunmap(pOrig, nReuse);
      }
    }
  }

  /* If pNew is still NULL, try to create an entirely new mapping. */
  যদি( pNew==0 ){
    pNew = osMmap(0, nNew, flags, MAP_SHARED, h, 0);
  }

  যদি( pNew==MAP_FAILED ){
    pNew = 0;
    nNew = 0;
    unixLogError(SQLITE_OK, zErr, pFd->zPath);

    /* If the mmap() above failed, assume that all subsequent mmap() calls
    ** will probably fail too. Fall back to using xRead/xWrite exclusively
    ** in this case.  */
    pFd->mmapSizeMax = 0;
  }
  pFd->pMapRegion = (শূন্য *)pNew;
  pFd->mmapSize = pFd->mmapSizeActual = nNew;
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
স্থির পূর্ণাঙ্ক unixMapfile(unixFile *pFd, i64 nMap){
  প্রতিজ্ঞা( nMap>=0 || pFd->nFetchOut==0 );
  প্রতিজ্ঞা( nMap>0 || (pFd->mmapSize==0 && pFd->pMapRegion==0) );
  যদি( pFd->nFetchOut>0 ) প্রত্যাগম SQLITE_OK;

  যদি( nMap<0 ){
    সংরচনা stat statbuf;          /* Low-level file information */
    যদি( osFstat(pFd->h, &statbuf) ){
      প্রত্যাগম SQLITE_IOERR_FSTAT;
    }
    nMap = statbuf.st_size;
  }
  যদি( nMap>pFd->mmapSizeMax ){
    nMap = pFd->mmapSizeMax;
  }

  প্রতিজ্ঞা( nMap>0 || (pFd->mmapSize==0 && pFd->pMapRegion==0) );
  যদি( nMap!=pFd->mmapSize ){
    unixRemapfile(pFd, nMap);
  }

  প্রত্যাগম SQLITE_OK;
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
** release the reference by calling unixUnfetch().
*/
স্থির পূর্ণাঙ্ক unixFetch(sqlite3_file *fd, i64 iOff, পূর্ণাঙ্ক nAmt, শূন্য **pp){
#যদি SQLITE_MAX_MMAP_SIZE>0
  unixFile *pFd = (unixFile *)fd;   /* The underlying database file */
#endif
  *pp = 0;

#যদি SQLITE_MAX_MMAP_SIZE>0
  যদি( pFd->mmapSizeMax>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    ধ্রুব পূর্ণাঙ্ক nEofBuffer = 256;
    যদি( pFd->pMapRegion==0 ){
      পূর্ণাঙ্ক rc = unixMapfile(pFd, -1);
      যদি( rc!=SQLITE_OK ) প্রত্যাগম rc;
    }
    যদি( pFd->mmapSize >= (iOff+nAmt+nEofBuffer) ){
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->nFetchOut++;
    }
  }
#endif
  প্রত্যাগম SQLITE_OK;
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
স্থির পূর্ণাঙ্ক unixUnfetch(sqlite3_file *fd, i64 iOff, শূন্য *p){
#যদি SQLITE_MAX_MMAP_SIZE>0
  unixFile *pFd = (unixFile *)fd;   /* The underlying database file */
  UNUSED_PARAMETER(iOff);

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  প্রতিজ্ঞা( (p==0)==(pFd->nFetchOut==0) );

  /* If p!=0, it must match the iOff value. */
  প্রতিজ্ঞা( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  যদি( p ){
    pFd->nFetchOut--;
  }অন্যথা{
    unixUnmapfile(pFd);
  }

  প্রতিজ্ঞা( pFd->nFetchOut>=0 );
#অন্যথা
  UNUSED_PARAMETER(fd);
  UNUSED_PARAMETER(p);
  UNUSED_PARAMETER(iOff);
#endif
  প্রত্যাগম SQLITE_OK;
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
#define IOMETHODS(FINDER,METHOD,VERSION,CLOSE,LOCK,UNLOCK,CKLOCK,SHMMAP)     \
স্থির ধ্রুব sqlite3_io_methods METHOD = {                                   \
   VERSION,                    /* iVersion */                                \
   CLOSE,                      /* xClose */                                  \
   unixRead,                   /* xRead */                                   \
   unixWrite,                  /* xWrite */                                  \
   unixTruncate,               /* xTruncate */                               \
   unixSync,                   /* xSync */                                   \
   unixFileSize,               /* xFileSize */                               \
   LOCK,                       /* xLock */                                   \
   UNLOCK,                     /* xUnlock */                                 \
   CKLOCK,                     /* xCheckReservedLock */                      \
   unixFileControl,            /* xFileControl */                            \
   unixSectorSize,             /* xSectorSize */                             \
   unixDeviceCharacteristics,  /* xDeviceCapabilities */                     \
   SHMMAP,                     /* xShmMap */                                 \
   unixShmLock,                /* xShmLock */                                \
   unixShmBarrier,             /* xShmBarrier */                             \
   unixShmUnmap,               /* xShmUnmap */                               \
   unixFetch,                  /* xFetch */                                  \
   unixUnfetch,                /* xUnfetch */                                \
};                                                                           \
স্থির ধ্রুব sqlite3_io_methods *FINDER##Impl(ধ্রুব বর্ণ *z, unixFile *p){   \
  UNUSED_PARAMETER(z); UNUSED_PARAMETER(p);                                  \
  প্রত্যাগম &METHOD;                                                            \
}                                                                            \
স্থির ধ্রুব sqlite3_io_methods *(*ধ্রুব FINDER)(ধ্রুব বর্ণ*,unixFile *p)    \
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
  unixLock,                 /* xLock method */
  unixUnlock,               /* xUnlock method */
  unixCheckReservedLock,    /* xCheckReservedLock method */
  unixShmMap                /* xShmMap method */
)
IOMETHODS(
  nolockIoFinder,           /* Finder function name */
  nolockIoMethods,          /* sqlite3_io_methods object name */
  3,                        /* shared memory and mmap are enabled */
  nolockClose,              /* xClose method */
  nolockLock,               /* xLock method */
  nolockUnlock,             /* xUnlock method */
  nolockCheckReservedLock,  /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
IOMETHODS(
  dotlockIoFinder,          /* Finder function name */
  dotlockIoMethods,         /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  dotlockClose,             /* xClose method */
  dotlockLock,              /* xLock method */
  dotlockUnlock,            /* xUnlock method */
  dotlockCheckReservedLock, /* xCheckReservedLock method */
  0                         /* xShmMap method */
)

#যদি SQLITE_ENABLE_LOCKING_STYLE
IOMETHODS(
  flockIoFinder,            /* Finder function name */
  flockIoMethods,           /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  flockClose,               /* xClose method */
  flockLock,                /* xLock method */
  flockUnlock,              /* xUnlock method */
  flockCheckReservedLock,   /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#endif

#যদি OS_VXWORKS
IOMETHODS(
  semIoFinder,              /* Finder function name */
  semIoMethods,             /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  semXClose,                /* xClose method */
  semXLock,                 /* xLock method */
  semXUnlock,               /* xUnlock method */
  semXCheckReservedLock,    /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#endif

#যদি defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
IOMETHODS(
  afpIoFinder,              /* Finder function name */
  afpIoMethods,             /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  afpClose,                 /* xClose method */
  afpLock,                  /* xLock method */
  afpUnlock,                /* xUnlock method */
  afpCheckReservedLock,     /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#endif

/*
** The proxy locking method is a "super-method" in the sense that it
** opens secondary file descriptors for the conch and lock files and
** it uses proxy, dot-file, AFP, and flock() locking methods on those
** secondary files.  For this reason, the division that implements
** proxy locking is located much further down in the file.  But we need
** to go ahead and define the sqlite3_io_methods and finder function
** for proxy locking here.  So we forward declare the I/O methods.
*/
#যদি defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
স্থির পূর্ণাঙ্ক proxyClose(sqlite3_file*);
স্থির পূর্ণাঙ্ক proxyLock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক proxyUnlock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক proxyCheckReservedLock(sqlite3_file*, পূর্ণাঙ্ক*);
IOMETHODS(
  proxyIoFinder,            /* Finder function name */
  proxyIoMethods,           /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  proxyClose,               /* xClose method */
  proxyLock,                /* xLock method */
  proxyUnlock,              /* xUnlock method */
  proxyCheckReservedLock,   /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#endif

/* nfs lockd on OSX 10.3+ doesn't clear write locks when a read lock is set */
#যদি defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
IOMETHODS(
  nfsIoFinder,               /* Finder function name */
  nfsIoMethods,              /* sqlite3_io_methods object name */
  1,                         /* shared memory is disabled */
  unixClose,                 /* xClose method */
  unixLock,                  /* xLock method */
  nfsUnlock,                 /* xUnlock method */
  unixCheckReservedLock,     /* xCheckReservedLock method */
  0                          /* xShmMap method */
)
#endif

#যদি defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
** This "finder" function attempts to determine the best locking strategy
** for the database file "filePath".  It then returns the sqlite3_io_methods
** object that implements that strategy.
**
** This is for MacOSX only.
*/
স্থির ধ্রুব sqlite3_io_methods *autolockIoFinderImpl(
  ধ্রুব বর্ণ *filePath,    /* name of the database file */
  unixFile *pNew           /* open file object for the database file */
){
  স্থির ধ্রুব সংরচনা Mapping {
    ধ্রুব বর্ণ *zFilesystem;              /* Filesystem type name */
    ধ্রুব sqlite3_io_methods *pMethods;   /* Appropriate locking method */
  } aMap[] = {
    { "hfs",    &posixIoMethods },
    { "ufs",    &posixIoMethods },
    { "afpfs",  &afpIoMethods },
    { "smbfs",  &afpIoMethods },
    { "webdav", &nolockIoMethods },
    { 0, 0 }
  };
  পূর্ণাঙ্ক i;
  সংরচনা statfs fsInfo;
  সংরচনা flock lockInfo;

  যদি( !filePath ){
    /* If filePath==NULL that means we are dealing with a transient file
    ** that does not need to be locked. */
    প্রত্যাগম &nolockIoMethods;
  }
  যদি( statfs(filePath, &fsInfo) != -1 ){
    যদি( fsInfo.f_flags & MNT_RDONLY ){
      প্রত্যাগম &nolockIoMethods;
    }
    প্রতি(i=0; aMap[i].zFilesystem; i++){
      যদি( strcmp(fsInfo.f_fstypename, aMap[i].zFilesystem)==0 ){
        প্রত্যাগম aMap[i].pMethods;
      }
    }
  }

  /* Default case. Handles, amongst others, "nfs".
  ** Test byte-range lock using fcntl(). If the call succeeds,
  ** assume that the file-system supports POSIX style locks.
  */
  lockInfo.l_len = 1;
  lockInfo.l_start = 0;
  lockInfo.l_whence = SEEK_SET;
  lockInfo.l_type = F_RDLCK;
  যদি( osFcntl(pNew->h, F_GETLK, &lockInfo)!=-1 ) {
    যদি( strcmp(fsInfo.f_fstypename, "nfs")==0 ){
      প্রত্যাগম &nfsIoMethods;
    } অন্যথা {
      প্রত্যাগম &posixIoMethods;
    }
  }অন্যথা{
    প্রত্যাগম &dotlockIoMethods;
  }
}
স্থির ধ্রুব sqlite3_io_methods
  *(*ধ্রুব autolockIoFinder)(ধ্রুব বর্ণ*,unixFile*) = autolockIoFinderImpl;

#endif /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */

#যদি OS_VXWORKS
/*
** This "finder" function for VxWorks checks to see if posix advisory
** locking works.  If it does, then that is what is used.  If it does not
** work, then fallback to named semaphore locking.
*/
স্থির ধ্রুব sqlite3_io_methods *vxworksIoFinderImpl(
  ধ্রুব বর্ণ *filePath,    /* name of the database file */
  unixFile *pNew           /* the open file object */
){
  সংরচনা flock lockInfo;

  যদি( !filePath ){
    /* If filePath==NULL that means we are dealing with a transient file
    ** that does not need to be locked. */
    প্রত্যাগম &nolockIoMethods;
  }

  /* Test if fcntl() is supported and use POSIX style locks.
  ** Otherwise fall back to the named semaphore method.
  */
  lockInfo.l_len = 1;
  lockInfo.l_start = 0;
  lockInfo.l_whence = SEEK_SET;
  lockInfo.l_type = F_RDLCK;
  যদি( osFcntl(pNew->h, F_GETLK, &lockInfo)!=-1 ) {
    প্রত্যাগম &posixIoMethods;
  }অন্যথা{
    প্রত্যাগম &semIoMethods;
  }
}
স্থির ধ্রুব sqlite3_io_methods
  *(*ধ্রুব vxworksIoFinder)(ধ্রুব বর্ণ*,unixFile*) = vxworksIoFinderImpl;

#endif /* OS_VXWORKS */

/*
** An abstract type for a pointer to an IO method finder function:
*/
প্রকারনির্দেশ ধ্রুব sqlite3_io_methods *(*finder_type)(ধ্রুব বর্ণ*,unixFile*);


/****************************************************************************
**************************** sqlite3_vfs methods ****************************
**
** This division contains the implementation of methods on the
** sqlite3_vfs object.
*/

/*
** Initialize the contents of the unixFile structure pointed to by pId.
*/
স্থির পূর্ণাঙ্ক fillInUnixFile(
  sqlite3_vfs *pVfs,      /* Pointer to vfs object */
  পূর্ণাঙ্ক h,                  /* Open file descriptor of file being opened */
  sqlite3_file *pId,      /* Write to the unixFile structure here */
  ধ্রুব বর্ণ *zFilename,  /* Name of the file being opened */
  পূর্ণাঙ্ক ctrlFlags           /* Zero or more UNIXFILE_* values */
){
  ধ্রুব sqlite3_io_methods *pLockingStyle;
  unixFile *pNew = (unixFile *)pId;
  পূর্ণাঙ্ক rc = SQLITE_OK;

  প্রতিজ্ঞা( pNew->pInode==রিক্ত );

  /* No locking occurs in temporary files */
  প্রতিজ্ঞা( zFilename!=0 || (ctrlFlags & UNIXFILE_NOLOCK)!=0 );

  OSTRACE(("OPEN    %-3d %s\n", h, zFilename));
  pNew->h = h;
  pNew->pVfs = pVfs;
  pNew->zPath = zFilename;
  pNew->ctrlFlags = (u8)ctrlFlags;
#যদি SQLITE_MAX_MMAP_SIZE>0
  pNew->mmapSizeMax = sqlite3GlobalConfig.szMmap;
#endif
  যদি( sqlite3_uri_boolean(((ctrlFlags & UNIXFILE_URI) ? zFilename : 0),
                           "psow", SQLITE_POWERSAFE_OVERWRITE) ){
    pNew->ctrlFlags |= UNIXFILE_PSOW;
  }
  যদি( strcmp(pVfs->zName,"unix-excl")==0 ){
    pNew->ctrlFlags |= UNIXFILE_EXCL;
  }

#যদি OS_VXWORKS
  pNew->pId = vxworksFindFileId(zFilename);
  যদি( pNew->pId==0 ){
    ctrlFlags |= UNIXFILE_NOLOCK;
    rc = SQLITE_NOMEM_BKPT;
  }
#endif

  যদি( ctrlFlags & UNIXFILE_NOLOCK ){
    pLockingStyle = &nolockIoMethods;
  }অন্যথা{
    pLockingStyle = (**(finder_type*)pVfs->pAppData)(zFilename, pNew);
#যদি SQLITE_ENABLE_LOCKING_STYLE
    /* Cache zFilename in the locking context (AFP and dotlock override) for
    ** proxyLock activation is possible (remote proxy is based on db name)
    ** zFilename remains valid until file is closed, to support */
    pNew->lockingContext = (শূন্য*)zFilename;
#endif
  }

  যদি( pLockingStyle == &posixIoMethods
#যদি defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
    || pLockingStyle == &nfsIoMethods
#endif
  ){
    unixEnterMutex();
    rc = findInodeInfo(pNew, &pNew->pInode);
    যদি( rc!=SQLITE_OK ){
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

#যদি SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
  অন্যথা যদি( pLockingStyle == &afpIoMethods ){
    /* AFP locking uses the file path so it needs to be included in
    ** the afpLockingContext.
    */
    afpLockingContext *pCtx;
    pNew->lockingContext = pCtx = sqlite3_malloc64( পরিমাণ(*pCtx) );
    যদি( pCtx==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }অন্যথা{
      /* NB: zFilename exists and remains valid until the file is closed
      ** according to requirement F11141.  So we do not need to make a
      ** copy of the filename. */
      pCtx->dbPath = zFilename;
      pCtx->reserved = 0;
      srandomdev();
      unixEnterMutex();
      rc = findInodeInfo(pNew, &pNew->pInode);
      যদি( rc!=SQLITE_OK ){
        sqlite3_free(pNew->lockingContext);
        robust_close(pNew, h, __LINE__);
        h = -1;
      }
      unixLeaveMutex();      
    }
  }
#endif

  অন্যথা যদি( pLockingStyle == &dotlockIoMethods ){
    /* Dotfile locking uses the file path so it needs to be included in
    ** the dotlockLockingContext
    */
    বর্ণ *zLockFile;
    পূর্ণাঙ্ক nFilename;
    প্রতিজ্ঞা( zFilename!=0 );
    nFilename = (পূর্ণাঙ্ক)strlen(zFilename) + 6;
    zLockFile = (বর্ণ *)sqlite3_malloc64(nFilename);
    যদি( zLockFile==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }অন্যথা{
      sqlite3_snprintf(nFilename, zLockFile, "%s" DOTLOCK_SUFFIX, zFilename);
    }
    pNew->lockingContext = zLockFile;
  }

#যদি OS_VXWORKS
  অন্যথা যদি( pLockingStyle == &semIoMethods ){
    /* Named semaphore locking uses the file path so it needs to be
    ** included in the semLockingContext
    */
    unixEnterMutex();
    rc = findInodeInfo(pNew, &pNew->pInode);
    যদি( (rc==SQLITE_OK) && (pNew->pInode->pSem==রিক্ত) ){
      বর্ণ *zSemName = pNew->pInode->aSemName;
      পূর্ণাঙ্ক n;
      sqlite3_snprintf(MAX_PATHNAME, zSemName, "/%s.sem",
                       pNew->pId->zCanonicalName);
      প্রতি( n=1; zSemName[n]; n++ )
        যদি( zSemName[n]=='/' ) zSemName[n] = '_';
      pNew->pInode->pSem = sem_open(zSemName, O_CREAT, 0666, 1);
      যদি( pNew->pInode->pSem == SEM_FAILED ){
        rc = SQLITE_NOMEM_BKPT;
        pNew->pInode->aSemName[0] = '\0';
      }
    }
    unixLeaveMutex();
  }
#endif

  storeLastErrno(pNew, 0);
#যদি OS_VXWORKS
  যদি( rc!=SQLITE_OK ){
    যদি( h>=0 ){
      robust_close(pNew, h, __LINE__);
      h = -1;
    }
    যদি( pNew->ctrlFlags & UNIXFILE_DELETE ){
      osUnlink(zFilename);
    }
    যদি( pNew->pId ){
      vxworksReleaseFileId(pNew->pId);
      pNew->pId = 0;
    }
  }
#endif
  যদি( rc!=SQLITE_OK ){
    যদি( h>=0 ) robust_close(pNew, h, __LINE__);
  }অন্যথা{
    pId->pMethods = pLockingStyle;
    OpenCounter(+1);
    verifyDbFile(pNew);
  }
  প্রত্যাগম rc;
}

/*
** Return the name of a directory in which to put temporary files.
** If no suitable temporary file directory can be found, return NULL.
**
** The return value might be a string obtained from getenv() and so
** the return value should not be used after any call to setenv() or
** putenv() as that value might have been freed.
*/
স্থির ধ্রুব বর্ণ *unixTempFileDir(শূন্য){
  unsigned পূর্ণাঙ্ক i;
  সংরচনা stat buf;
  ধ্রুব বর্ণ *zDir;

  প্রতি(i=0; i<7; i++){
    বিকল্প( i ){
      পক্ষ 0:  zDir = sqlite3_temp_directory;   ভঙ্গ;
      পক্ষ 1:  zDir = getenv("SQLITE_TMPDIR");  ভঙ্গ;
      পক্ষ 2:  zDir = getenv("TMPDIR");         ভঙ্গ;
      পক্ষ 3:  zDir = "/var/tmp";               ভঙ্গ;
      পক্ষ 4:  zDir = "/usr/tmp";               ভঙ্গ;
      পক্ষ 5:  zDir = "/tmp";                   ভঙ্গ;
      স্বভাব: zDir = ".";                      ভঙ্গ;
    }
    যদি( zDir!=0
#যদি OS_VXWORKS
     && zDir[0]=='/'
#endif
     && osStat(zDir, &buf)==0
     && S_ISDIR(buf.st_mode)
     && osAccess(zDir, 03)==0
    ){
      প্রত্যাগম zDir;
    }
  }
  প্রত্যাগম 0;
}

/*
** Create a temporary file name in zBuf.  zBuf must be allocated
** by the calling process and must be big enough to hold at least
** pVfs->mxPathname bytes.
*/
স্থির পূর্ণাঙ্ক unixGetTempname(পূর্ণাঙ্ক nBuf, বর্ণ *zBuf){
  ধ্রুব বর্ণ *zDir;
  পূর্ণাঙ্ক iLimit = 0;
  পূর্ণাঙ্ক rc = SQLITE_OK;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  zBuf[0] = 0;
  SimulateIOError( প্রত্যাগম SQLITE_IOERR );

  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  zDir = unixTempFileDir();
  যদি( zDir==0 ){
    rc = SQLITE_IOERR_GETTEMPPATH;
  }অন্যথা{
    কুরু{
      u64 r;
      sqlite3_randomness(পরিমাণ(r), &r);
      প্রতিজ্ঞা( nBuf>2 );
      zBuf[nBuf-2] = 0;
      sqlite3_snprintf(nBuf, zBuf, "%s/"SQLITE_TEMP_FILE_PREFIX"%llx%c",
                       zDir, r, 0);
      যদি( zBuf[nBuf-2]!=0 || (iLimit++)>10 ){
        rc = SQLITE_ERROR;
        ভঙ্গ;
      }
    }যাবত্( osAccess(zBuf,0)==0 );
  }
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  প্রত্যাগম rc;
}

#যদি SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
/*
** Routine to transform a unixFile into a proxy-locking unixFile.
** Implementation in the proxy-lock division, but used by unixOpen()
** if SQLITE_PREFER_PROXY_LOCKING is defined.
*/
স্থির পূর্ণাঙ্ক proxyTransformUnixFile(unixFile*, ধ্রুব বর্ণ*);
#endif

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
স্থির UnixUnusedFd *findReusableFd(ধ্রুব বর্ণ *zPath, পূর্ণাঙ্ক flags){
  UnixUnusedFd *pUnused = 0;

  /* Do not search for an unused file descriptor on vxworks. Not because
  ** vxworks would not benefit from the change (it might, we're not sure),
  ** but because no way to test it is currently available. It is better
  ** not to risk breaking vxworks support for the sake of such an obscure
  ** feature.  */
#যদি !OS_VXWORKS
  সংরচনা stat sStat;                   /* Results of stat() call */

  unixEnterMutex();

  /* A stat() call may fail for various reasons. If this happens, it is
  ** almost certain that an open() call on the same path will also fail.
  ** For this reason, if an error occurs in the stat() call here, it is
  ** ignored and -1 is returned. The caller will try to open a new file
  ** descriptor on the same path, fail, and return an error to SQLite.
  **
  ** Even if a subsequent open() call does succeed, the consequences of
  ** not searching for a reusable file descriptor are not dire.  */
  যদি( inodeList!=0 && 0==osStat(zPath, &sStat) ){
    unixInodeInfo *pInode;

    pInode = inodeList;
    যাবত্( pInode && (pInode->fileId.dev!=sStat.st_dev
                     || pInode->fileId.ino!=(u64)sStat.st_ino) ){
       pInode = pInode->pNext;
    }
    যদি( pInode ){
      UnixUnusedFd **pp;
      প্রতিজ্ঞা( sqlite3_mutex_notheld(pInode->pLockMutex) );
      sqlite3_mutex_enter(pInode->pLockMutex);
      flags &= (SQLITE_OPEN_READONLY|SQLITE_OPEN_READWRITE);
      প্রতি(pp=&pInode->pUnused; *pp && (*pp)->flags!=flags; pp=&((*pp)->pNext));
      pUnused = *pp;
      যদি( pUnused ){
        *pp = pUnused->pNext;
      }
      sqlite3_mutex_leave(pInode->pLockMutex);
    }
  }
  unixLeaveMutex();
#endif    /* if !OS_VXWORKS */
  প্রত্যাগম pUnused;
}

/*
** Find the mode, uid and gid of file zFile.
*/
স্থির পূর্ণাঙ্ক getFileMode(
  ধ্রুব বর্ণ *zFile,              /* File name */
  mode_t *pMode,                  /* OUT: Permissions of zFile */
  uid_t *pUid,                    /* OUT: uid of zFile. */
  gid_t *pGid                     /* OUT: gid of zFile. */
){
  সংরচনা stat sStat;              /* Output of stat() on database file */
  পূর্ণাঙ্ক rc = SQLITE_OK;
  যদি( 0==osStat(zFile, &sStat) ){
    *pMode = sStat.st_mode & 0777;
    *pUid = sStat.st_uid;
    *pGid = sStat.st_gid;
  }অন্যথা{
    rc = SQLITE_IOERR_FSTAT;
  }
  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক findCreateFileMode(
  ধ্রুব বর্ণ *zPath,              /* Path of file (possibly) being created */
  পূর্ণাঙ্ক flags,                      /* Flags passed as 4th argument to xOpen() */
  mode_t *pMode,                  /* OUT: Permissions to open file with */
  uid_t *pUid,                    /* OUT: uid to set on the file */
  gid_t *pGid                     /* OUT: gid to set on the file */
){
  পূর্ণাঙ্ক rc = SQLITE_OK;             /* Return Code */
  *pMode = 0;
  *pUid = 0;
  *pGid = 0;
  যদি( flags & (SQLITE_OPEN_WAL|SQLITE_OPEN_MAIN_JOURNAL) ){
    বর্ণ zDb[MAX_PATHNAME+1];     /* Database file path */
    পূর্ণাঙ্ক nDb;                      /* Number of valid bytes in zDb */

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
    nDb = sqlite3Strlen30(zPath) - 1;
    যাবত্( nDb>0 && zPath[nDb]!='.' ){
      যদি( zPath[nDb]=='-' ){
        memcpy(zDb, zPath, nDb);
        zDb[nDb] = '\0';
        rc = getFileMode(zDb, pMode, pUid, pGid);
        ভঙ্গ;
      }
      nDb--;
    }
  }অন্যথা যদি( flags & SQLITE_OPEN_DELETEONCLOSE ){
    *pMode = 0600;
  }অন্যথা যদি( flags & SQLITE_OPEN_URI ){
    /* If this is a main database file and the file was opened using a URI
    ** filename, check for the "modeof" parameter. If present, interpret
    ** its value as a filename and try to copy the mode, uid and gid from
    ** that file.  */
    ধ্রুব বর্ণ *z = sqlite3_uri_parameter(zPath, "modeof");
    যদি( z ){
      rc = getFileMode(z, pMode, pUid, pGid);
    }
  }
  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক unixOpen(
  sqlite3_vfs *pVfs,           /* The VFS for which this is the xOpen method */
  ধ্রুব বর্ণ *zPath,           /* Pathname of file to be opened */
  sqlite3_file *pFile,         /* The file descriptor to be filled in */
  পূর্ণাঙ্ক flags,                   /* Input flags to control the opening */
  পূর্ণাঙ্ক *pOutFlags               /* Output flags returned to SQLite core */
){
  unixFile *p = (unixFile *)pFile;
  পূর্ণাঙ্ক fd = -1;                   /* File descriptor returned by open() */
  পূর্ণাঙ্ক openFlags = 0;             /* Flags to pass to open() */
  পূর্ণাঙ্ক eType = flags&0x0FFF00;  /* Type of file to open */
  পূর্ণাঙ্ক noLock;                    /* True to omit locking primitives */
  পূর্ণাঙ্ক rc = SQLITE_OK;            /* Function Return Code */
  পূর্ণাঙ্ক ctrlFlags = 0;             /* UNIXFILE_* flags */

  পূর্ণাঙ্ক isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  পূর্ণাঙ্ক isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  পূর্ণাঙ্ক isCreate     = (flags & SQLITE_OPEN_CREATE);
  পূর্ণাঙ্ক isReadonly   = (flags & SQLITE_OPEN_READONLY);
  পূর্ণাঙ্ক isReadWrite  = (flags & SQLITE_OPEN_READWRITE);
#যদি SQLITE_ENABLE_LOCKING_STYLE
  পূর্ণাঙ্ক isAutoProxy  = (flags & SQLITE_OPEN_AUTOPROXY);
#endif
#যদি defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
  সংরচনা statfs fsInfo;
#endif

  /* If creating a super- or main-file journal, this function will open
  ** a file-descriptor on the directory too. The first time unixSync()
  ** is called the directory file descriptor will be fsync()ed and close()d.
  */
  পূর্ণাঙ্ক isNewJrnl = (isCreate && (
        eType==SQLITE_OPEN_SUPER_JOURNAL
     || eType==SQLITE_OPEN_MAIN_JOURNAL
     || eType==SQLITE_OPEN_WAL
  ));

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  বর্ণ zTmpname[MAX_PATHNAME+2];
  ধ্রুব বর্ণ *zName = zPath;

  /* Check the following statements are true:
  **
  **   (a) Exactly one of the READWRITE and READONLY flags must be set, and
  **   (b) if CREATE is set, then READWRITE must also be set, and
  **   (c) if EXCLUSIVE is set, then CREATE must also be set.
  **   (d) if DELETEONCLOSE is set, then CREATE must also be set.
  */
  প্রতিজ্ঞা((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
  প্রতিজ্ঞা(isCreate==0 || isReadWrite);
  প্রতিজ্ঞা(isExclusive==0 || isCreate);
  প্রতিজ্ঞা(isDelete==0 || isCreate);

  /* The main DB, main journal, WAL file and super-journal are never
  ** automatically deleted. Nor are they ever temporary files.  */
  প্রতিজ্ঞা( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_DB );
  প্রতিজ্ঞা( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_JOURNAL );
  প্রতিজ্ঞা( (!isDelete && zName) || eType!=SQLITE_OPEN_SUPER_JOURNAL );
  প্রতিজ্ঞা( (!isDelete && zName) || eType!=SQLITE_OPEN_WAL );

  /* Assert that the upper layer has set one of the "file-type" flags. */
  প্রতিজ্ঞা( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_TEMP_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_TEMP_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );

#যদি OS_VXWORKS
  /* The file-ID mechanism used in Vxworks requires that all pathnames
  ** provided to unixOpen must be absolute pathnames. */
  যদি( zPath!=0 && zPath[0]!='/' ){ প্রত্যাগম SQLITE_CANTOPEN; }
#endif

  /* Detect a pid change and reset the PRNG.  There is a race condition
  ** here such that two or more threads all trying to open databases at
  ** the same instant might all reset the PRNG.  But multiple resets
  ** are harmless.
  */
  যদি( randomnessPid!=osGetpid(0) ){
    randomnessPid = osGetpid(0);
    sqlite3_randomness(0,0);
  }
  memset(p, 0, পরিমাণ(unixFile));

#ifdef SQLITE_ASSERT_NO_FILES
  /* Applications that never read or write a persistent disk files */
  প্রতিজ্ঞা( zName==0 );
#endif

  যদি( eType==SQLITE_OPEN_MAIN_DB ){
    UnixUnusedFd *pUnused;
    pUnused = findReusableFd(zName, flags);
    যদি( pUnused ){
      fd = pUnused->fd;
    }অন্যথা{
      pUnused = sqlite3_malloc64(পরিমাণ(*pUnused));
      যদি( !pUnused ){
        প্রত্যাগম SQLITE_NOMEM_BKPT;
      }
    }
    p->pPreallocatedUnused = pUnused;

    /* Database filenames are double-zero terminated if they are not
    ** URIs with parameters.  Hence, they can always be passed into
    ** sqlite3_uri_parameter(). */
    প্রতিজ্ঞা( (flags & SQLITE_OPEN_URI) || zName[strlen(zName)+1]==0 );

  }অন্যথা যদি( !zName ){
    /* If zName is NULL, the upper layer is requesting a temp file. */
    প্রতিজ্ঞা( isDelete );
    প্রতিজ্ঞা( !isNewJrnl );
    প্রতিজ্ঞা( isExclusive );
    প্রতিজ্ঞা( isReadWrite );
#যদি defined(__linux__) && defined(O_TMPFILE)
    /* On systems that support O_TMPFILE, use that flag to create a more
    ** secure temporary file that cannot be accessed by other processes
    */
    zName = unixTempFileDir();
    যদি( zName
     && (fd = robust_open(zName, O_RDWR|O_EXCL|O_TMPFILE, 0600))>=0
    ){
      rc = fillInUnixFile(pVfs, fd, pFile, zPath, ctrlFlags|UNIXFILE_NOLOCK);
      গচ্ছ open_finished;
    }
#endif
    rc = unixGetTempname(pVfs->mxPathname, zTmpname);
    যদি( rc!=SQLITE_OK ){
      প্রত্যাগম rc;
    }
    zName = zTmpname;

    /* Generated temporary filenames are always double-zero terminated
    ** for use by sqlite3_uri_parameter(). */
    প্রতিজ্ঞা( zName[strlen(zName)+1]==0 );
  }

  /* Determine the value of the flags parameter passed to POSIX function
  ** open(). These must be calculated even if open() is not called, as
  ** they may be stored as part of the file handle and used by the
  ** 'conch file' locking functions later on.  */
  যদি( isReadonly )  openFlags |= O_RDONLY;
  যদি( isReadWrite ) openFlags |= O_RDWR;
  যদি( isCreate )    openFlags |= O_CREAT;
  যদি( isExclusive ) openFlags |= (O_EXCL|O_NOFOLLOW);
  openFlags |= (O_LARGEFILE|O_দ্বিমিক|O_NOFOLLOW);

  যদি( fd<0 ){
    mode_t openMode;              /* Permissions to create file with */
    uid_t uid;                    /* Userid for the file */
    gid_t gid;                    /* Groupid for the file */
    rc = findCreateFileMode(zName, flags, &openMode, &uid, &gid);
    যদি( rc!=SQLITE_OK ){
      প্রতিজ্ঞা( !p->pPreallocatedUnused );
      প্রতিজ্ঞা( eType==SQLITE_OPEN_WAL || eType==SQLITE_OPEN_MAIN_JOURNAL );
      প্রত্যাগম rc;
    }
    fd = robust_open(zName, openFlags, openMode);
    OSTRACE(("OPENX   %-3d %s 0%o\n", fd, zName, openFlags));
    প্রতিজ্ঞা( !isExclusive || (openFlags & O_CREAT)!=0 );
    যদি( fd<0 ){
      যদি( isNewJrnl && errno==EACCES && osAccess(zName, F_OK) ){
        /* If unable to create a journal because the directory is not
        ** writable, change the error code to indicate that. */
        rc = SQLITE_READONLY_DIRECTORY;
      }অন্যথা যদি( errno!=EISDIR && isReadWrite ){
        /* Failed to open the file for read/write access. Try read-only. */
        UnixUnusedFd *pReadonly = 0;
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        openFlags &= ~(O_RDWR|O_CREAT);
        flags |= SQLITE_OPEN_READONLY;
        openFlags |= O_RDONLY;
        isReadonly = 1;
        pReadonly = findReusableFd(zName, flags);
        যদি( pReadonly ){
          fd = pReadonly->fd;
          sqlite3_free(pReadonly);
        }অন্যথা{
          fd = robust_open(zName, openFlags, openMode);
        }
      }
    }
    যদি( fd<0 ){
      পূর্ণাঙ্ক rc2 = unixLogError(SQLITE_CANTOPEN_BKPT, "open", zName);
      যদি( rc==SQLITE_OK ) rc = rc2;
      গচ্ছ open_finished;
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
    যদি( openMode && (flags & (SQLITE_OPEN_WAL|SQLITE_OPEN_MAIN_JOURNAL))!=0 ){
      robustFchown(fd, uid, gid);
    }
  }
  প্রতিজ্ঞা( fd>=0 );
  যদি( pOutFlags ){
    *pOutFlags = flags;
  }

  যদি( p->pPreallocatedUnused ){
    p->pPreallocatedUnused->fd = fd;
    p->pPreallocatedUnused->flags =
                          flags & (SQLITE_OPEN_READONLY|SQLITE_OPEN_READWRITE);
  }

  যদি( isDelete ){
#যদি OS_VXWORKS
    zPath = zName;
#নোচেদ্যদি defined(SQLITE_UNLINK_AFTER_CLOSE)
    zPath = sqlite3_mprintf("%s", zName);
    যদি( zPath==0 ){
      robust_close(p, fd, __LINE__);
      প্রত্যাগম SQLITE_NOMEM_BKPT;
    }
#অন্যথা
    osUnlink(zName);
#endif
  }
#যদি SQLITE_ENABLE_LOCKING_STYLE
  অন্যথা{
    p->openFlags = openFlags;
  }
#endif

#যদি defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
  যদি( fstatfs(fd, &fsInfo) == -1 ){
    storeLastErrno(p, errno);
    robust_close(p, fd, __LINE__);
    প্রত্যাগম SQLITE_IOERR_ACCESS;
  }
  যদি (0 == strncmp("msdos", fsInfo.f_fstypename, 5)) {
    ((unixFile*)pFile)->fsFlags |= SQLITE_FSFLAGS_IS_MSDOS;
  }
  যদি (0 == strncmp("exfat", fsInfo.f_fstypename, 5)) {
    ((unixFile*)pFile)->fsFlags |= SQLITE_FSFLAGS_IS_MSDOS;
  }
#endif

  /* Set up appropriate ctrlFlags */
  যদি( isDelete )                ctrlFlags |= UNIXFILE_DELETE;
  যদি( isReadonly )              ctrlFlags |= UNIXFILE_RDONLY;
  noLock = eType!=SQLITE_OPEN_MAIN_DB;
  যদি( noLock )                  ctrlFlags |= UNIXFILE_NOLOCK;
  যদি( isNewJrnl )               ctrlFlags |= UNIXFILE_DIRSYNC;
  যদি( flags & SQLITE_OPEN_URI ) ctrlFlags |= UNIXFILE_URI;

#যদি SQLITE_ENABLE_LOCKING_STYLE
#যদি SQLITE_PREFER_PROXY_LOCKING
  isAutoProxy = 1;
#endif
  যদি( isAutoProxy && (zPath!=রিক্ত) && (!noLock) && pVfs->xOpen ){
    বর্ণ *envforce = getenv("SQLITE_FORCE_PROXY_LOCKING");
    পূর্ণাঙ্ক useProxy = 0;

    /* SQLITE_FORCE_PROXY_LOCKING==1 means force always use proxy, 0 means
    ** never use proxy, NULL means use proxy for non-local files only.  */
    যদি( envforce!=রিক্ত ){
      useProxy = atoi(envforce)>0;
    }অন্যথা{
      useProxy = !(fsInfo.f_flags&MNT_LOCAL);
    }
    যদি( useProxy ){
      rc = fillInUnixFile(pVfs, fd, pFile, zPath, ctrlFlags);
      যদি( rc==SQLITE_OK ){
        rc = proxyTransformUnixFile((unixFile*)pFile, ":auto:");
        যদি( rc!=SQLITE_OK ){
          /* Use unixClose to clean up the resources added in fillInUnixFile
          ** and clear all the structure's references.  Specifically,
          ** pFile->pMethods will be NULL so sqlite3OsClose will be a no-op
          */
          unixClose(pFile);
          প্রত্যাগম rc;
        }
      }
      গচ্ছ open_finished;
    }
  }
#endif

  প্রতিজ্ঞা( zPath==0
       || zPath[0]=='/'
       || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_MAIN_JOURNAL
       || eType==SQLITE_OPEN_TEMP_JOURNAL
  );
  rc = fillInUnixFile(pVfs, fd, pFile, zPath, ctrlFlags);

open_finished:
  যদি( rc!=SQLITE_OK ){
    sqlite3_free(p->pPreallocatedUnused);
  }
  প্রত্যাগম rc;
}


/*
** Delete the file at zPath. If the dirSync argument is true, fsync()
** the directory after deleting the file.
*/
স্থির পূর্ণাঙ্ক unixDelete(
  sqlite3_vfs *NotUsed,     /* VFS containing this as the xDelete method */
  ধ্রুব বর্ণ *zPath,        /* Name of file to be deleted */
  পূর্ণাঙ্ক dirSync               /* If true, fsync() directory after deleting file */
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  UNUSED_PARAMETER(NotUsed);
  SimulateIOError(প্রত্যাগম SQLITE_IOERR_DELETE);
  যদি( osUnlink(zPath)==(-1) ){
    যদি( errno==ENOENT
#যদি OS_VXWORKS
        || osAccess(zPath,0)!=0
#endif
    ){
      rc = SQLITE_IOERR_DELETE_NOENT;
    }অন্যথা{
      rc = unixLogError(SQLITE_IOERR_DELETE, "unlink", zPath);
    }
    প্রত্যাগম rc;
  }
#ifndef SQLITE_DISABLE_DIRSYNC
  যদি( (dirSync & 1)!=0 ){
    পূর্ণাঙ্ক fd;
    rc = osOpenDirectory(zPath, &fd);
    যদি( rc==SQLITE_OK ){
      যদি( full_fsync(fd,0,0) ){
        rc = unixLogError(SQLITE_IOERR_DIR_FSYNC, "fsync", zPath);
      }
      robust_close(0, fd, __LINE__);
    }অন্যথা{
      প্রতিজ্ঞা( rc==SQLITE_CANTOPEN );
      rc = SQLITE_OK;
    }
  }
#endif
  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক unixAccess(
  sqlite3_vfs *NotUsed,   /* The VFS containing this xAccess method */
  ধ্রুব বর্ণ *zPath,      /* Path of the file to examine */
  পূর্ণাঙ্ক flags,              /* What do we want to learn about the zPath file? */
  পূর্ণাঙ্ক *pResOut            /* Write result boolean here */
){
  UNUSED_PARAMETER(NotUsed);
  SimulateIOError( প্রত্যাগম SQLITE_IOERR_ACCESS; );
  প্রতিজ্ঞা( pResOut!=0 );

  /* The spec says there are three possible values for flags.  But only
  ** two of them are actually used */
  প্রতিজ্ঞা( flags==SQLITE_ACCESS_EXISTS || flags==SQLITE_ACCESS_READWRITE );

  যদি( flags==SQLITE_ACCESS_EXISTS ){
    সংরচনা stat buf;
    *pResOut = 0==osStat(zPath, &buf) &&
                (!S_ISREG(buf.st_mode) || buf.st_size>0);
  }অন্যথা{
    *pResOut = osAccess(zPath, W_OK|R_OK)==0;
  }
  প্রত্যাগম SQLITE_OK;
}

/*
** A pathname under construction
*/
প্রকারনির্দেশ সংরচনা DbPath DbPath;
সংরচনা DbPath {
  পূর্ণাঙ্ক rc;           /* Non-zero following any error */
  পূর্ণাঙ্ক nSymlink;     /* Number of symlinks resolved */
  বর্ণ *zOut;       /* Write the pathname here */
  পূর্ণাঙ্ক nOut;         /* Bytes of space available to zOut[] */
  পূর্ণাঙ্ক nUsed;        /* Bytes of zOut[] currently being used */
};

/* Forward reference */
স্থির শূন্য appendAllPathElements(DbPath*,ধ্রুব বর্ণ*);

/*
** Append a single path element to the DbPath under construction
*/
স্থির শূন্য appendOnePathElement(
  DbPath *pPath,       /* Path under construction, to which to append zName */
  ধ্রুব বর্ণ *zName,   /* Name to append to pPath.  Not zero-terminated */
  পূর্ণাঙ্ক nName            /* Number of significant bytes in zName */
){
  প্রতিজ্ঞা( nName>0 );
  প্রতিজ্ঞা( zName!=0 );
  যদি( zName[0]=='.' ){
    যদি( nName==1 ) প্রত্যাগম;
    যদি( zName[1]=='.' && nName==2 ){
      যদি( pPath->nUsed>1 ){
        প্রতিজ্ঞা( pPath->zOut[0]=='/' );
        যাবত্( pPath->zOut[--pPath->nUsed]!='/' ){}
      }
      প্রত্যাগম;
    }
  }
  যদি( pPath->nUsed + nName + 2 >= pPath->nOut ){
    pPath->rc = SQLITE_ERROR;
    প্রত্যাগম;
  }
  pPath->zOut[pPath->nUsed++] = '/';
  memcpy(&pPath->zOut[pPath->nUsed], zName, nName);
  pPath->nUsed += nName;
#যদি defined(HAVE_READLINK) && defined(HAVE_LSTAT)
  যদি( pPath->rc==SQLITE_OK ){
    ধ্রুব বর্ণ *zIn;
    সংরচনা stat buf;
    pPath->zOut[pPath->nUsed] = 0;
    zIn = pPath->zOut;
    যদি( osLstat(zIn, &buf)!=0 ){
      যদি( errno!=ENOENT ){
        pPath->rc = unixLogError(SQLITE_CANTOPEN_BKPT, "lstat", zIn);
      }
    }অন্যথা যদি( S_ISLNK(buf.st_mode) ){
      ssize_t got;
      বর্ণ zLnk[SQLITE_MAX_PATHLEN+2];
      যদি( pPath->nSymlink++ > SQLITE_MAX_SYMLINK ){
        pPath->rc = SQLITE_CANTOPEN_BKPT;
        প্রত্যাগম;
      }
      got = osReadlink(zIn, zLnk, পরিমাণ(zLnk)-2);
      যদি( got<=0 || got>=(ssize_t)পরিমাণ(zLnk)-2 ){
        pPath->rc = unixLogError(SQLITE_CANTOPEN_BKPT, "readlink", zIn);
        প্রত্যাগম;
      }
      zLnk[got] = 0;
      যদি( zLnk[0]=='/' ){
        pPath->nUsed = 0;
      }অন্যথা{
        pPath->nUsed -= nName + 1;
      }
      appendAllPathElements(pPath, zLnk);
    }
  }
#endif
}

/*
** Append all path elements in zPath to the DbPath under construction.
*/
স্থির শূন্য appendAllPathElements(
  DbPath *pPath,       /* Path under construction, to which to append zName */
  ধ্রুব বর্ণ *zPath    /* Path to append to pPath.  Is zero-terminated */
){
  পূর্ণাঙ্ক i = 0;
  পূর্ণাঙ্ক j = 0;
  কুরু{
    যাবত্( zPath[i] && zPath[i]!='/' ){ i++; }
    যদি( i>j ){
      appendOnePathElement(pPath, &zPath[j], i-j);
    }
    j = i+1;
  }যাবত্( zPath[i++] );
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
স্থির পূর্ণাঙ্ক unixFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ধ্রুব বর্ণ *zPath,            /* Possibly relative input path */
  পূর্ণাঙ্ক nOut,                     /* Size of output buffer in bytes */
  বর্ণ *zOut                    /* Output buffer */
){
  DbPath path;
  UNUSED_PARAMETER(pVfs);
  path.rc = 0;
  path.nUsed = 0;
  path.nSymlink = 0;
  path.nOut = nOut;
  path.zOut = zOut;
  যদি( zPath[0]!='/' ){
    বর্ণ zPwd[SQLITE_MAX_PATHLEN+2];
    যদি( osGetcwd(zPwd, পরিমাণ(zPwd)-2)==0 ){
      প্রত্যাগম unixLogError(SQLITE_CANTOPEN_BKPT, "getcwd", zPath);
    }
    appendAllPathElements(&path, zPwd);
  }
  appendAllPathElements(&path, zPath);
  zOut[path.nUsed] = 0;
  যদি( path.rc || path.nUsed<2 ) প্রত্যাগম SQLITE_CANTOPEN_BKPT;
  যদি( path.nSymlink ) প্রত্যাগম SQLITE_OK_SYMLINK;
  প্রত্যাগম SQLITE_OK;
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
#অন্তর্ভাব <dlfcn.h>
স্থির শূন্য *unixDlOpen(sqlite3_vfs *NotUsed, ধ্রুব বর্ণ *zFilename){
  UNUSED_PARAMETER(NotUsed);
  প্রত্যাগম dlopen(zFilename, RTLD_NOW | RTLD_GLOBAL);
}

/*
** SQLite calls this function immediately after a call to unixDlSym() or
** unixDlOpen() fails (returns a null pointer). If a more detailed error
** message is available, it is written to zBufOut. If no error message
** is available, zBufOut is left unmodified and SQLite uses a default
** error message.
*/
স্থির শূন্য unixDlError(sqlite3_vfs *NotUsed, পূর্ণাঙ্ক nBuf, বর্ণ *zBufOut){
  ধ্রুব বর্ণ *zErr;
  UNUSED_PARAMETER(NotUsed);
  unixEnterMutex();
  zErr = dlerror();
  যদি( zErr ){
    sqlite3_snprintf(nBuf, zBufOut, "%s", zErr);
  }
  unixLeaveMutex();
}
স্থির শূন্য (*unixDlSym(sqlite3_vfs *NotUsed, শূন্য *p, ধ্রুব বর্ণ*zSym))(শূন্য){
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
  শূন্য (*(*x)(শূন্য*,ধ্রুব বর্ণ*))(শূন্য);
  UNUSED_PARAMETER(NotUsed);
  x = (শূন্য(*(*)(শূন্য*,ধ্রুব বর্ণ*))(শূন্য))dlsym;
  প্রত্যাগম (*x)(p, zSym);
}
স্থির শূন্য unixDlClose(sqlite3_vfs *NotUsed, শূন্য *pHandle){
  UNUSED_PARAMETER(NotUsed);
  dlclose(pHandle);
}
#অন্যথা /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #define unixDlOpen  0
  #define unixDlError 0
  #define unixDlSym   0
  #define unixDlClose 0
#endif

/*
** Write nBuf bytes of random data to the supplied buffer zBuf.
*/
স্থির পূর্ণাঙ্ক unixRandomness(sqlite3_vfs *NotUsed, পূর্ণাঙ্ক nBuf, বর্ণ *zBuf){
  UNUSED_PARAMETER(NotUsed);
  প্রতিজ্ঞা((size_t)nBuf>=(পরিমাণ(time_t)+পরিমাণ(পূর্ণাঙ্ক)));

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
#যদি !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
  {
    পূর্ণাঙ্ক fd, got;
    fd = robust_open("/dev/urandom", O_RDONLY, 0);
    যদি( fd<0 ){
      time_t t;
      time(&t);
      memcpy(zBuf, &t, পরিমাণ(t));
      memcpy(&zBuf[পরিমাণ(t)], &randomnessPid, পরিমাণ(randomnessPid));
      প্রতিজ্ঞা( পরিমাণ(t)+পরিমাণ(randomnessPid)<=(size_t)nBuf );
      nBuf = পরিমাণ(t) + পরিমাণ(randomnessPid);
    }অন্যথা{
      কুরু{ got = osRead(fd, zBuf, nBuf); }যাবত্( got<0 && errno==EINTR );
      robust_close(0, fd, __LINE__);
    }
  }
#endif
  প্রত্যাগম nBuf;
}


/*
** Sleep for a little while.  Return the amount of time slept.
** The argument is the number of microseconds we want to sleep.
** The return value is the number of microseconds of sleep actually
** requested from the underlying operating system, a number which
** might be greater than or equal to the argument, but not less
** than the argument.
*/
স্থির পূর্ণাঙ্ক unixSleep(sqlite3_vfs *NotUsed, পূর্ণাঙ্ক microseconds){
#যদি !defined(HAVE_NANOSLEEP) || HAVE_NANOSLEEP+0
  সংরচনা timespec sp;
  sp.tv_sec = microseconds / 1000000;
  sp.tv_nsec = (microseconds % 1000000) * 1000;

  /* Almost all modern unix systems support nanosleep().  But if you are
  ** compiling for one of the rare exceptions, you can use
  ** -DHAVE_NANOSLEEP=0 (perhaps in conjunction with -DHAVE_USLEEP if
  ** usleep() is available) in order to bypass the use of nanosleep() */
  nanosleep(&sp, রিক্ত);

  UNUSED_PARAMETER(NotUsed);
  প্রত্যাগম microseconds;
#নোচেদ্যদি defined(HAVE_USLEEP) && HAVE_USLEEP
  যদি( microseconds>=1000000 ) sleep(microseconds/1000000);
  যদি( microseconds%1000000 ) usleep(microseconds%1000000);
  UNUSED_PARAMETER(NotUsed);
  প্রত্যাগম microseconds;
#অন্যথা
  পূর্ণাঙ্ক seconds = (microseconds+999999)/1000000;
  sleep(seconds);
  UNUSED_PARAMETER(NotUsed);
  প্রত্যাগম seconds*1000000;
#endif
}

/*
** The following variable, if set to a non-zero value, is interpreted as
** the number of seconds since 1970 and is used to set the result of
** sqlite3OsCurrentTime() during testing.
*/
#ifdef SQLITE_TEST
পূর্ণাঙ্ক sqlite3_current_time = 0;  /* Fake system time in seconds since 1970. */
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
স্থির পূর্ণাঙ্ক unixCurrentTimeInt64(sqlite3_vfs *NotUsed, sqlite3_int64 *piNow){
  স্থির ধ্রুব sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  পূর্ণাঙ্ক rc = SQLITE_OK;
#যদি defined(NO_GETTOD)
  time_t t;
  time(&t);
  *piNow = ((sqlite3_int64)t)*1000 + unixEpoch;
#নোচেদ্যদি OS_VXWORKS
  সংরচনা timespec sNow;
  clock_gettime(CLOCK_REALTIME, &sNow);
  *piNow = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_nsec/1000000;
#অন্যথা
  সংরচনা timeval sNow;
  (শূন্য)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *piNow = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
#endif

#ifdef SQLITE_TEST
  যদি( sqlite3_current_time ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_time + unixEpoch;
  }
#endif
  UNUSED_PARAMETER(NotUsed);
  প্রত্যাগম rc;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
স্থির পূর্ণাঙ্ক unixCurrentTime(sqlite3_vfs *NotUsed, দ্বিগুণ *prNow){
  sqlite3_int64 i = 0;
  পূর্ণাঙ্ক rc;
  UNUSED_PARAMETER(NotUsed);
  rc = unixCurrentTimeInt64(0, &i);
  *prNow = i/86400000.0;
  প্রত্যাগম rc;
}
#অন্যথা
# define unixCurrentTime 0
#endif

/*
** The xGetLastError() method is designed to return a better
** low-level error message when operating-system problems come up
** during SQLite operation.  Only the integer return code is currently
** used.
*/
স্থির পূর্ণাঙ্ক unixGetLastError(sqlite3_vfs *NotUsed, পূর্ণাঙ্ক NotUsed2, বর্ণ *NotUsed3){
  UNUSED_PARAMETER(NotUsed);
  UNUSED_PARAMETER(NotUsed2);
  UNUSED_PARAMETER(NotUsed3);
  প্রত্যাগম errno;
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
#যদি defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE

/*
** The proxyLockingContext has the path and file structures for the remote
** and local proxy files in it
*/
প্রকারনির্দেশ সংরচনা proxyLockingContext proxyLockingContext;
সংরচনা proxyLockingContext {
  unixFile *conchFile;         /* Open conch file */
  বর্ণ *conchFilePath;         /* Name of the conch file */
  unixFile *lockProxy;         /* Open proxy lock file */
  বর্ণ *lockProxyPath;         /* Name of the proxy lock file */
  বর্ণ *dbPath;                /* Name of the open file */
  পূর্ণাঙ্ক conchHeld;               /* 1 if the conch is held, -1 if lockless */
  পূর্ণাঙ্ক nFails;                  /* Number of conch taking failures */
  শূন্য *oldLockingContext;     /* Original lockingcontext to restore on close */
  sqlite3_io_methods ধ্রুব *pOldMethod;     /* Original I/O methods for close */
};

/*
** The proxy lock file path for the database at dbPath is written into lPath,
** which must point to valid, writable memory large enough for a maxLen length
** file path.
*/
স্থির পূর্ণাঙ্ক proxyGetLockPath(ধ্রুব বর্ণ *dbPath, বর্ণ *lPath, size_t maxLen){
  পূর্ণাঙ্ক len;
  পূর্ণাঙ্ক dbLen;
  পূর্ণাঙ্ক i;

#ifdef LOCKPROXYDIR
  len = strlcpy(lPath, LOCKPROXYDIR, maxLen);
#অন্যথা
# ifdef _CS_DARWIN_USER_TEMP_DIR
  {
    যদি( !confstr(_CS_DARWIN_USER_TEMP_DIR, lPath, maxLen) ){
      OSTRACE(("GETLOCKPATH  failed %s errno=%d pid=%d\n",
               lPath, errno, osGetpid(0)));
      প্রত্যাগম SQLITE_IOERR_LOCK;
    }
    len = strlcat(lPath, "sqliteplocks", maxLen);  
  }
# অন্যথা
  len = strlcpy(lPath, "/tmp/", maxLen);
# endif
#endif

  যদি( lPath[len-1]!='/' ){
    len = strlcat(lPath, "/", maxLen);
  }

  /* transform the db path to a unique cache name */
  dbLen = (পূর্ণাঙ্ক)strlen(dbPath);
  প্রতি( i=0; i<dbLen && (i+len+7)<(পূর্ণাঙ্ক)maxLen; i++){
    বর্ণ c = dbPath[i];
    lPath[i+len] = (c=='/')?'_':c;
  }
  lPath[i+len]='\0';
  strlcat(lPath, ":auto:", maxLen);
  OSTRACE(("GETLOCKPATH  proxy lock path=%s pid=%d\n", lPath, osGetpid(0)));
  প্রত্যাগম SQLITE_OK;
}

/*
 ** Creates the lock file and any missing directories in lockPath
 */
স্থির পূর্ণাঙ্ক proxyCreateLockPath(ধ্রুব বর্ণ *lockPath){
  পূর্ণাঙ্ক i, len;
  বর্ণ buf[MAXPATHLEN];
  পূর্ণাঙ্ক start = 0;

  প্রতিজ্ঞা(lockPath!=রিক্ত);
  /* try to create all the intermediate directories */
  len = (পূর্ণাঙ্ক)strlen(lockPath);
  buf[0] = lockPath[0];
  প্রতি( i=1; i<len; i++ ){
    যদি( lockPath[i] == '/' && (i - start > 0) ){
      /* only mkdir if leaf dir != "." or "/" or ".." */
      যদি( i-start>2 || (i-start==1 && buf[start] != '.' && buf[start] != '/')
         || (i-start==2 && buf[start] != '.' && buf[start+1] != '.') ){
        buf[i]='\0';
        যদি( osMkdir(buf, SQLITE_DEFAULT_PROXYDIR_PERMISSIONS) ){
          পূর্ণাঙ্ক err=errno;
          যদি( err!=EEXIST ) {
            OSTRACE(("CREATELOCKPATH  FAILED creating %s, "
                     "'%s' proxy lock path=%s pid=%d\n",
                     buf, strerror(err), lockPath, osGetpid(0)));
            প্রত্যাগম err;
          }
        }
      }
      start=i+1;
    }
    buf[i] = lockPath[i];
  }
  OSTRACE(("CREATELOCKPATH  proxy lock path=%s pid=%d\n",lockPath,osGetpid(0)));
  প্রত্যাগম 0;
}

/*
** Create a new VFS file descriptor (stored in memory obtained from
** sqlite3_malloc) and open the file named "path" in the file descriptor.
**
** The caller is responsible not only for closing the file descriptor
** but also for freeing the memory associated with the file descriptor.
*/
স্থির পূর্ণাঙ্ক proxyCreateUnixFile(
    ধ্রুব বর্ণ *path,        /* path for the new unixFile */
    unixFile **ppFile,       /* unixFile created and returned by ref */
    পূর্ণাঙ্ক islockfile           /* if non zero missing dirs will be created */
) {
  পূর্ণাঙ্ক fd = -1;
  unixFile *pNew;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক openFlags = O_RDWR | O_CREAT | O_NOFOLLOW;
  sqlite3_vfs dummyVfs;
  পূর্ণাঙ্ক terrno = 0;
  UnixUnusedFd *pUnused = রিক্ত;

  /* 1. first try to open/create the file
  ** 2. if that fails, and this is a lock file (not-conch), try creating
  ** the parent directories and then try again.
  ** 3. if that fails, try to open the file read-only
  ** otherwise return BUSY (if lock file) or CANTOPEN for the conch file
  */
  pUnused = findReusableFd(path, openFlags);
  যদি( pUnused ){
    fd = pUnused->fd;
  }অন্যথা{
    pUnused = sqlite3_malloc64(পরিমাণ(*pUnused));
    যদি( !pUnused ){
      প্রত্যাগম SQLITE_NOMEM_BKPT;
    }
  }
  যদি( fd<0 ){
    fd = robust_open(path, openFlags, 0);
    terrno = errno;
    যদি( fd<0 && errno==ENOENT && islockfile ){
      যদি( proxyCreateLockPath(path) == SQLITE_OK ){
        fd = robust_open(path, openFlags, 0);
      }
    }
  }
  যদি( fd<0 ){
    openFlags = O_RDONLY | O_NOFOLLOW;
    fd = robust_open(path, openFlags, 0);
    terrno = errno;
  }
  যদি( fd<0 ){
    যদি( islockfile ){
      প্রত্যাগম SQLITE_BUSY;
    }
    বিকল্প (terrno) {
      পক্ষ EACCES:
        প্রত্যাগম SQLITE_PERM;
      পক্ষ EIO:
        প্রত্যাগম SQLITE_IOERR_LOCK; /* even though it is the conch */
      স্বভাব:
        প্রত্যাগম SQLITE_CANTOPEN_BKPT;
    }
  }

  pNew = (unixFile *)sqlite3_malloc64(পরিমাণ(*pNew));
  যদি( pNew==রিক্ত ){
    rc = SQLITE_NOMEM_BKPT;
    গচ্ছ end_create_proxy;
  }
  memset(pNew, 0, পরিমাণ(unixFile));
  pNew->openFlags = openFlags;
  memset(&dummyVfs, 0, পরিমাণ(dummyVfs));
  dummyVfs.pAppData = (শূন্য*)&autolockIoFinder;
  dummyVfs.zName = "dummy";
  pUnused->fd = fd;
  pUnused->flags = openFlags;
  pNew->pPreallocatedUnused = pUnused;

  rc = fillInUnixFile(&dummyVfs, fd, (sqlite3_file*)pNew, path, 0);
  যদি( rc==SQLITE_OK ){
    *ppFile = pNew;
    প্রত্যাগম SQLITE_OK;
  }
end_create_proxy:  
  robust_close(pNew, fd, __LINE__);
  sqlite3_free(pNew);
  sqlite3_free(pUnused);
  প্রত্যাগম rc;
}

#ifdef SQLITE_TEST
/* simulate multiple hosts by creating unique hostid file paths */
পূর্ণাঙ্ক sqlite3_hostid_অঙ্ক = 0;
#endif

#define PROXY_HOSTIDLEN    16  /* conch file host id length */

#যদি HAVE_GETHOSTUUID
/* Not always defined in the headers as it ought to be */
বাহ্য পূর্ণাঙ্ক gethostuuid(uuid_t id, ধ্রুব সংরচনা timespec *wait);
#endif

/* get the host ID via gethostuuid(), pHostID must point to PROXY_HOSTIDLEN
** bytes of writable memory.
*/
স্থির পূর্ণাঙ্ক proxyGetHostID(unsigned বর্ণ *pHostID, পূর্ণাঙ্ক *pError){
  প্রতিজ্ঞা(PROXY_HOSTIDLEN == পরিমাণ(uuid_t));
  memset(pHostID, 0, PROXY_HOSTIDLEN);
#যদি HAVE_GETHOSTUUID
  {
    সংরচনা timespec timeout = {1, 0}; /* 1 sec timeout */
    যদি( gethostuuid(pHostID, &timeout) ){
      পূর্ণাঙ্ক err = errno;
      যদি( pError ){
        *pError = err;
      }
      প্রত্যাগম SQLITE_IOERR;
    }
  }
#অন্যথা
  UNUSED_PARAMETER(pError);
#endif
#ifdef SQLITE_TEST
  /* simulate multiple hosts by creating unique hostid file paths */
  যদি( sqlite3_hostid_অঙ্ক != 0){
    pHostID[0] = (বর্ণ)(pHostID[0] + (বর্ণ)(sqlite3_hostid_অঙ্ক & 0xFF));
  }
#endif

  প্রত্যাগম SQLITE_OK;
}

/* The conch file contains the header, host id and lock file path
 */
#define PROXY_CONCHVERSION 2   /* 1-byte header, 16-byte host id, path */
#define PROXY_HEADERLEN    1   /* conch file header length */
#define PROXY_PATHINDEX    (PROXY_HEADERLEN+PROXY_HOSTIDLEN)
#define PROXY_MAXCONCHLEN  (PROXY_HEADERLEN+PROXY_HOSTIDLEN+MAXPATHLEN)

/*
** Takes an open conch file, copies the contents to a new path and then moves
** it back.  The newly created file's file descriptor is assigned to the
** conch file structure and finally the original conch file descriptor is
** closed.  Returns zero if successful.
*/
স্থির পূর্ণাঙ্ক proxyBreakConchLock(unixFile *pFile, uuid_t myHostID){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
  unixFile *conchFile = pCtx->conchFile;
  বর্ণ tPath[MAXPATHLEN];
  বর্ণ buf[PROXY_MAXCONCHLEN];
  বর্ণ *cPath = pCtx->conchFilePath;
  size_t readLen = 0;
  size_t pathLen = 0;
  বর্ণ errmsg[64] = "";
  পূর্ণাঙ্ক fd = -1;
  পূর্ণাঙ্ক rc = -1;
  UNUSED_PARAMETER(myHostID);

  /* create a new path by replace the trailing '-conch' with '-break' */
  pathLen = strlcpy(tPath, cPath, MAXPATHLEN);
  যদি( pathLen>MAXPATHLEN || pathLen<6 ||
     (strlcpy(&tPath[pathLen-5], "break", 6) != 5) ){
    sqlite3_snprintf(পরিমাণ(errmsg),errmsg,"path error (len %d)",(পূর্ণাঙ্ক)pathLen);
    গচ্ছ end_breaklock;
  }
  /* read the conch content */
  readLen = osPread(conchFile->h, buf, PROXY_MAXCONCHLEN, 0);
  যদি( readLen<PROXY_PATHINDEX ){
    sqlite3_snprintf(পরিমাণ(errmsg),errmsg,"read error (len %d)",(পূর্ণাঙ্ক)readLen);
    গচ্ছ end_breaklock;
  }
  /* write it out to the temporary break file */
  fd = robust_open(tPath, (O_RDWR|O_CREAT|O_EXCL|O_NOFOLLOW), 0);
  যদি( fd<0 ){
    sqlite3_snprintf(পরিমাণ(errmsg), errmsg, "create failed (%d)", errno);
    গচ্ছ end_breaklock;
  }
  যদি( osPwrite(fd, buf, readLen, 0) != (ssize_t)readLen ){
    sqlite3_snprintf(পরিমাণ(errmsg), errmsg, "write failed (%d)", errno);
    গচ্ছ end_breaklock;
  }
  যদি( rename(tPath, cPath) ){
    sqlite3_snprintf(পরিমাণ(errmsg), errmsg, "rename failed (%d)", errno);
    গচ্ছ end_breaklock;
  }
  rc = 0;
  fprintf(stderr, "broke stale lock on %s\n", cPath);
  robust_close(pFile, conchFile->h, __LINE__);
  conchFile->h = fd;
  conchFile->openFlags = O_RDWR | O_CREAT;

end_breaklock:
  যদি( rc ){
    যদি( fd>=0 ){
      osUnlink(tPath);
      robust_close(pFile, fd, __LINE__);
    }
    fprintf(stderr, "failed to break stale lock on %s, %s\n", cPath, errmsg);
  }
  প্রত্যাগম rc;
}

/* Take the requested lock on the conch file and break a stale lock if the
** host id matches.
*/
স্থির পূর্ণাঙ্ক proxyConchLock(unixFile *pFile, uuid_t myHostID, পূর্ণাঙ্ক lockType){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
  unixFile *conchFile = pCtx->conchFile;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  পূর্ণাঙ্ক nTries = 0;
  সংরচনা timespec conchModTime;

  memset(&conchModTime, 0, পরিমাণ(conchModTime));
  কুরু {
    rc = conchFile->pMethod->xLock((sqlite3_file*)conchFile, lockType);
    nTries ++;
    যদি( rc==SQLITE_BUSY ){
      /* If the lock failed (busy):
       * 1st try: get the mod time of the conch, wait 0.5s and try again.
       * 2nd try: fail if the mod time changed or host id is different, wait
       *           10 sec and try again
       * 3rd try: break the lock unless the mod time has changed.
       */
      সংরচনা stat buf;
      যদি( osFstat(conchFile->h, &buf) ){
        storeLastErrno(pFile, errno);
        প্রত্যাগম SQLITE_IOERR_LOCK;
      }
    
      যদি( nTries==1 ){
        conchModTime = buf.st_mtimespec;
        unixSleep(0,500000); /* wait 0.5 sec and try the lock again*/
        অনুবর্ত;
      }

      প্রতিজ্ঞা( nTries>1 );
      যদি( conchModTime.tv_sec != buf.st_mtimespec.tv_sec ||
         conchModTime.tv_nsec != buf.st_mtimespec.tv_nsec ){
        প্রত্যাগম SQLITE_BUSY;
      }
    
      যদি( nTries==2 ){
        বর্ণ tBuf[PROXY_MAXCONCHLEN];
        পূর্ণাঙ্ক len = osPread(conchFile->h, tBuf, PROXY_MAXCONCHLEN, 0);
        যদি( len<0 ){
          storeLastErrno(pFile, errno);
          প্রত্যাগম SQLITE_IOERR_LOCK;
        }
        যদি( len>PROXY_PATHINDEX && tBuf[0]==(বর্ণ)PROXY_CONCHVERSION){
          /* don't break the lock if the host id doesn't match */
          যদি( 0!=memcmp(&tBuf[PROXY_HEADERLEN], myHostID, PROXY_HOSTIDLEN) ){
            প্রত্যাগম SQLITE_BUSY;
          }
        }অন্যথা{
          /* don't break the lock on short read or a version mismatch */
          প্রত্যাগম SQLITE_BUSY;
        }
        unixSleep(0,10000000); /* wait 10 sec and try the lock again */
        অনুবর্ত;
      }
    
      প্রতিজ্ঞা( nTries==3 );
      যদি( 0==proxyBreakConchLock(pFile, myHostID) ){
        rc = SQLITE_OK;
        যদি( lockType==EXCLUSIVE_LOCK ){
          rc = conchFile->pMethod->xLock((sqlite3_file*)conchFile, SHARED_LOCK);
        }
        যদি( !rc ){
          rc = conchFile->pMethod->xLock((sqlite3_file*)conchFile, lockType);
        }
      }
    }
  } যাবত্( rc==SQLITE_BUSY && nTries<3 );

  প্রত্যাগম rc;
}

/* Takes the conch by taking a shared lock and read the contents conch, if
** lockPath is non-NULL, the host ID and lock file path must match.  A NULL
** lockPath means that the lockPath in the conch file will be used if the
** host IDs match, or a new lock path will be generated automatically
** and written to the conch file.
*/
স্থির পূর্ণাঙ্ক proxyTakeConch(unixFile *pFile){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;

  যদি( pCtx->conchHeld!=0 ){
    প্রত্যাগম SQLITE_OK;
  }অন্যথা{
    unixFile *conchFile = pCtx->conchFile;
    uuid_t myHostID;
    পূর্ণাঙ্ক pError = 0;
    বর্ণ readBuf[PROXY_MAXCONCHLEN];
    বর্ণ lockPath[MAXPATHLEN];
    বর্ণ *tempLockPath = রিক্ত;
    পূর্ণাঙ্ক rc = SQLITE_OK;
    পূর্ণাঙ্ক createConch = 0;
    পূর্ণাঙ্ক hostIdMatch = 0;
    পূর্ণাঙ্ক readLen = 0;
    পূর্ণাঙ্ক tryOldLockPath = 0;
    পূর্ণাঙ্ক forceNewLockPath = 0;
  
    OSTRACE(("TAKECONCH  %d for %s pid=%d\n", conchFile->h,
             (pCtx->lockProxyPath ? pCtx->lockProxyPath : ":auto:"),
             osGetpid(0)));

    rc = proxyGetHostID(myHostID, &pError);
    যদি( (rc&0xff)==SQLITE_IOERR ){
      storeLastErrno(pFile, pError);
      গচ্ছ end_takeconch;
    }
    rc = proxyConchLock(pFile, myHostID, SHARED_LOCK);
    যদি( rc!=SQLITE_OK ){
      গচ্ছ end_takeconch;
    }
    /* read the existing conch file */
    readLen = seekAndRead((unixFile*)conchFile, 0, readBuf, PROXY_MAXCONCHLEN);
    যদি( readLen<0 ){
      /* I/O error: lastErrno set by seekAndRead */
      storeLastErrno(pFile, conchFile->lastErrno);
      rc = SQLITE_IOERR_READ;
      গচ্ছ end_takeconch;
    }অন্যথা যদি( readLen<=(PROXY_HEADERLEN+PROXY_HOSTIDLEN) ||
             readBuf[0]!=(বর্ণ)PROXY_CONCHVERSION ){
      /* a short read or version format mismatch means we need to create a new
      ** conch file.
      */
      createConch = 1;
    }
    /* if the host id matches and the lock path already exists in the conch
    ** we'll try to use the path there, if we can't open that path, we'll
    ** retry with a new auto-generated path
    */
    কুরু { /* in case we need to try again for an :auto: named lock file */

      যদি( !createConch && !forceNewLockPath ){
        hostIdMatch = !memcmp(&readBuf[PROXY_HEADERLEN], myHostID,
                                  PROXY_HOSTIDLEN);
        /* if the conch has data compare the contents */
        যদি( !pCtx->lockProxyPath ){
          /* for auto-named local lock file, just check the host ID and we'll
           ** use the local lock file path that's already in there
           */
          যদি( hostIdMatch ){
            size_t pathLen = (readLen - PROXY_PATHINDEX);
          
            যদি( pathLen>=MAXPATHLEN ){
              pathLen=MAXPATHLEN-1;
            }
            memcpy(lockPath, &readBuf[PROXY_PATHINDEX], pathLen);
            lockPath[pathLen] = 0;
            tempLockPath = lockPath;
            tryOldLockPath = 1;
            /* create a copy of the lock path if the conch is taken */
            গচ্ছ end_takeconch;
          }
        }অন্যথা যদি( hostIdMatch
               && !strncmp(pCtx->lockProxyPath, &readBuf[PROXY_PATHINDEX],
                           readLen-PROXY_PATHINDEX)
        ){
          /* conch host and lock path match */
          গচ্ছ end_takeconch;
        }
      }
    
      /* if the conch isn't writable and doesn't match, we can't take it */
      যদি( (conchFile->openFlags&O_RDWR) == 0 ){
        rc = SQLITE_BUSY;
        গচ্ছ end_takeconch;
      }
    
      /* either the conch didn't match or we need to create a new one */
      যদি( !pCtx->lockProxyPath ){
        proxyGetLockPath(pCtx->dbPath, lockPath, MAXPATHLEN);
        tempLockPath = lockPath;
        /* create a copy of the lock path _only_ if the conch is taken */
      }
    
      /* update conch with host and path (this will fail if other process
      ** has a shared lock already), if the host id matches, use the big
      ** stick.
      */
      futimes(conchFile->h, রিক্ত);
      যদি( hostIdMatch && !createConch ){
        যদি( conchFile->pInode && conchFile->pInode->nShared>1 ){
          /* We are trying for an exclusive lock but another thread in this
           ** same process is still holding a shared lock. */
          rc = SQLITE_BUSY;
        } অন্যথা {        
          rc = proxyConchLock(pFile, myHostID, EXCLUSIVE_LOCK);
        }
      }অন্যথা{
        rc = proxyConchLock(pFile, myHostID, EXCLUSIVE_LOCK);
      }
      যদি( rc==SQLITE_OK ){
        বর্ণ writeBuffer[PROXY_MAXCONCHLEN];
        পূর্ণাঙ্ক writeSize = 0;
      
        writeBuffer[0] = (বর্ণ)PROXY_CONCHVERSION;
        memcpy(&writeBuffer[PROXY_HEADERLEN], myHostID, PROXY_HOSTIDLEN);
        যদি( pCtx->lockProxyPath!=রিক্ত ){
          strlcpy(&writeBuffer[PROXY_PATHINDEX], pCtx->lockProxyPath,
                  MAXPATHLEN);
        }অন্যথা{
          strlcpy(&writeBuffer[PROXY_PATHINDEX], tempLockPath, MAXPATHLEN);
        }
        writeSize = PROXY_PATHINDEX + strlen(&writeBuffer[PROXY_PATHINDEX]);
        robust_ftruncate(conchFile->h, writeSize);
        rc = unixWrite((sqlite3_file *)conchFile, writeBuffer, writeSize, 0);
        full_fsync(conchFile->h,0,0);
        /* If we created a new conch file (not just updated the contents of a
         ** valid conch file), try to match the permissions of the database
         */
        যদি( rc==SQLITE_OK && createConch ){
          সংরচনা stat buf;
          পূর্ণাঙ্ক err = osFstat(pFile->h, &buf);
          যদি( err==0 ){
            mode_t cmode = buf.st_mode&(S_IRUSR|S_IWUSR | S_IRGRP|S_IWGRP |
                                        S_IROTH|S_IWOTH);
            /* try to match the database file R/W permissions, ignore failure */
#ifndef SQLITE_PROXY_DEBUG
            osFchmod(conchFile->h, cmode);
#অন্যথা
            কুরু{
              rc = osFchmod(conchFile->h, cmode);
            }যাবত্( rc==(-1) && errno==EINTR );
            যদি( rc!=0 ){
              পূর্ণাঙ্ক code = errno;
              fprintf(stderr, "fchmod %o FAILED with %d %s\n",
                      cmode, code, strerror(code));
            } অন্যথা {
              fprintf(stderr, "fchmod %o SUCCEDED\n",cmode);
            }
          }অন্যথা{
            পূর্ণাঙ্ক code = errno;
            fprintf(stderr, "STAT FAILED[%d] with %d %s\n",
                    err, code, strerror(code));
#endif
          }
        }
      }
      conchFile->pMethod->xUnlock((sqlite3_file*)conchFile, SHARED_LOCK);
    
    end_takeconch:
      OSTRACE(("TRANSPROXY: CLOSE  %d\n", pFile->h));
      যদি( rc==SQLITE_OK && pFile->openFlags ){
        পূর্ণাঙ্ক fd;
        যদি( pFile->h>=0 ){
          robust_close(pFile, pFile->h, __LINE__);
        }
        pFile->h = -1;
        fd = robust_open(pCtx->dbPath, pFile->openFlags, 0);
        OSTRACE(("TRANSPROXY: OPEN  %d\n", fd));
        যদি( fd>=0 ){
          pFile->h = fd;
        }অন্যথা{
          rc=SQLITE_CANTOPEN_BKPT; /* SQLITE_BUSY? proxyTakeConch called
           during locking */
        }
      }
      যদি( rc==SQLITE_OK && !pCtx->lockProxy ){
        বর্ণ *path = tempLockPath ? tempLockPath : pCtx->lockProxyPath;
        rc = proxyCreateUnixFile(path, &pCtx->lockProxy, 1);
        যদি( rc!=SQLITE_OK && rc!=SQLITE_NOMEM && tryOldLockPath ){
          /* we couldn't create the proxy lock file with the old lock file path
           ** so try again via auto-naming
           */
          forceNewLockPath = 1;
          tryOldLockPath = 0;
          অনুবর্ত; /* go back to the do {} while start point, try again */
        }
      }
      যদি( rc==SQLITE_OK ){
        /* Need to make a copy of path if we extracted the value
         ** from the conch file or the path was allocated on the stack
         */
        যদি( tempLockPath ){
          pCtx->lockProxyPath = sqlite3DbStrDup(0, tempLockPath);
          যদি( !pCtx->lockProxyPath ){
            rc = SQLITE_NOMEM_BKPT;
          }
        }
      }
      যদি( rc==SQLITE_OK ){
        pCtx->conchHeld = 1;
      
        যদি( pCtx->lockProxy->pMethod == &afpIoMethods ){
          afpLockingContext *afpCtx;
          afpCtx = (afpLockingContext *)pCtx->lockProxy->lockingContext;
          afpCtx->dbPath = pCtx->lockProxyPath;
        }
      } অন্যথা {
        conchFile->pMethod->xUnlock((sqlite3_file*)conchFile, NO_LOCK);
      }
      OSTRACE(("TAKECONCH  %d %s\n", conchFile->h,
               rc==SQLITE_OK?"ok":"failed"));
      প্রত্যাগম rc;
    } যাবত্ (1); /* in case we need to retry the :auto: lock file -
                 ** we should never get here except via the 'continue' call. */
  }
}

/*
** If pFile holds a lock on a conch file, then release that lock.
*/
স্থির পূর্ণাঙ্ক proxyReleaseConch(unixFile *pFile){
  পূর্ণাঙ্ক rc = SQLITE_OK;         /* Subroutine return code */
  proxyLockingContext *pCtx;  /* The locking context for the proxy lock */
  unixFile *conchFile;        /* Name of the conch file */

  pCtx = (proxyLockingContext *)pFile->lockingContext;
  conchFile = pCtx->conchFile;
  OSTRACE(("RELEASECONCH  %d for %s pid=%d\n", conchFile->h,
           (pCtx->lockProxyPath ? pCtx->lockProxyPath : ":auto:"),
           osGetpid(0)));
  যদি( pCtx->conchHeld>0 ){
    rc = conchFile->pMethod->xUnlock((sqlite3_file*)conchFile, NO_LOCK);
  }
  pCtx->conchHeld = 0;
  OSTRACE(("RELEASECONCH  %d %s\n", conchFile->h,
           (rc==SQLITE_OK ? "ok" : "failed")));
  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক proxyCreateConchPathname(বর্ণ *dbPath, বর্ণ **pConchPath){
  পূর্ণাঙ্ক i;                        /* Loop counter */
  পূর্ণাঙ্ক len = (পূর্ণাঙ্ক)strlen(dbPath); /* Length of database filename - dbPath */
  বর্ণ *conchPath;              /* buffer in which to construct conch name */

  /* Allocate space for the conch filename and initialize the name to
  ** the name of the original database file. */
  *pConchPath = conchPath = (বর্ণ *)sqlite3_malloc64(len + 8);
  যদি( conchPath==0 ){
    প্রত্যাগম SQLITE_NOMEM_BKPT;
  }
  memcpy(conchPath, dbPath, len+1);

  /* now insert a "." before the last / character */
  প্রতি( i=(len-1); i>=0; i-- ){
    যদি( conchPath[i]=='/' ){
      i++;
      ভঙ্গ;
    }
  }
  conchPath[i]='.';
  যাবত্ ( i<len ){
    conchPath[i+1]=dbPath[i];
    i++;
  }

  /* append the "-conch" suffix to the file */
  memcpy(&conchPath[i+1], "-conch", 7);
  প্রতিজ্ঞা( (পূর্ণাঙ্ক)strlen(conchPath) == len+7 );

  প্রত্যাগম SQLITE_OK;
}


/* Takes a fully configured proxy locking-style unix file and switches
** the local lock file path
*/
স্থির পূর্ণাঙ্ক switchLockProxyPath(unixFile *pFile, ধ্রুব বর্ণ *path) {
  proxyLockingContext *pCtx = (proxyLockingContext*)pFile->lockingContext;
  বর্ণ *oldPath = pCtx->lockProxyPath;
  পূর্ণাঙ্ক rc = SQLITE_OK;

  যদি( pFile->eFileLock!=NO_LOCK ){
    প্রত্যাগম SQLITE_BUSY;
  }

  /* nothing to do if the path is NULL, :auto: or matches the existing path */
  যদি( !path || path[0]=='\0' || !strcmp(path, ":auto:") ||
    (oldPath && !strncmp(oldPath, path, MAXPATHLEN)) ){
    প্রত্যাগম SQLITE_OK;
  }অন্যথা{
    unixFile *lockProxy = pCtx->lockProxy;
    pCtx->lockProxy=রিক্ত;
    pCtx->conchHeld = 0;
    যদি( lockProxy!=রিক্ত ){
      rc=lockProxy->pMethod->xClose((sqlite3_file *)lockProxy);
      যদি( rc ) প্রত্যাগম rc;
      sqlite3_free(lockProxy);
    }
    sqlite3_free(oldPath);
    pCtx->lockProxyPath = sqlite3DbStrDup(0, path);
  }

  প্রত্যাগম rc;
}

/*
** pFile is a file that has been opened by a prior xOpen call.  dbPath
** is a string buffer at least MAXPATHLEN+1 characters in size.
**
** This routine find the filename associated with pFile and writes it
** int dbPath.
*/
স্থির পূর্ণাঙ্ক proxyGetDbPathForUnixFile(unixFile *pFile, বর্ণ *dbPath){
#যদি defined(__APPLE__)
  যদি( pFile->pMethod == &afpIoMethods ){
    /* afp style keeps a reference to the db path in the filePath field
    ** of the struct */
    প্রতিজ্ঞা( (পূর্ণাঙ্ক)strlen((বর্ণ*)pFile->lockingContext)<=MAXPATHLEN );
    strlcpy(dbPath, ((afpLockingContext *)pFile->lockingContext)->dbPath,
            MAXPATHLEN);
  } অন্যথা
#endif
  যদি( pFile->pMethod == &dotlockIoMethods ){
    /* dot lock style uses the locking context to store the dot lock
    ** file path */
    পূর্ণাঙ্ক len = strlen((বর্ণ *)pFile->lockingContext) - strlen(DOTLOCK_SUFFIX);
    memcpy(dbPath, (বর্ণ *)pFile->lockingContext, len + 1);
  }অন্যথা{
    /* all other styles use the locking context to store the db file path */
    প্রতিজ্ঞা( strlen((বর্ণ*)pFile->lockingContext)<=MAXPATHLEN );
    strlcpy(dbPath, (বর্ণ *)pFile->lockingContext, MAXPATHLEN);
  }
  প্রত্যাগম SQLITE_OK;
}

/*
** Takes an already filled in unix file and alters it so all file locking
** will be performed on the local proxy lock file.  The following fields
** are preserved in the locking context so that they can be restored and
** the unix structure properly cleaned up at close time:
**  ->lockingContext
**  ->pMethod
*/
স্থির পূর্ণাঙ্ক proxyTransformUnixFile(unixFile *pFile, ধ্রুব বর্ণ *path) {
  proxyLockingContext *pCtx;
  বর্ণ dbPath[MAXPATHLEN+1];       /* Name of the database file */
  বর্ণ *lockPath=রিক্ত;
  পূর্ণাঙ্ক rc = SQLITE_OK;

  যদি( pFile->eFileLock!=NO_LOCK ){
    প্রত্যাগম SQLITE_BUSY;
  }
  proxyGetDbPathForUnixFile(pFile, dbPath);
  যদি( !path || path[0]=='\0' || !strcmp(path, ":auto:") ){
    lockPath=রিক্ত;
  }অন্যথা{
    lockPath=(বর্ণ *)path;
  }

  OSTRACE(("TRANSPROXY  %d for %s pid=%d\n", pFile->h,
           (lockPath ? lockPath : ":auto:"), osGetpid(0)));

  pCtx = sqlite3_malloc64( পরিমাণ(*pCtx) );
  যদি( pCtx==0 ){
    প্রত্যাগম SQLITE_NOMEM_BKPT;
  }
  memset(pCtx, 0, পরিমাণ(*pCtx));

  rc = proxyCreateConchPathname(dbPath, &pCtx->conchFilePath);
  যদি( rc==SQLITE_OK ){
    rc = proxyCreateUnixFile(pCtx->conchFilePath, &pCtx->conchFile, 0);
    যদি( rc==SQLITE_CANTOPEN && ((pFile->openFlags&O_RDWR) == 0) ){
      /* if (a) the open flags are not O_RDWR, (b) the conch isn't there, and
      ** (c) the file system is read-only, then enable no-locking access.
      ** Ugh, since O_RDONLY==0x0000 we test for !O_RDWR since unixOpen asserts
      ** that openFlags will have only one of O_RDONLY or O_RDWR.
      */
      সংরচনা statfs fsInfo;
      সংরচনা stat conchInfo;
      পূর্ণাঙ্ক goLockless = 0;

      যদি( osStat(pCtx->conchFilePath, &conchInfo) == -1 ) {
        পূর্ণাঙ্ক err = errno;
        যদি( (err==ENOENT) && (statfs(dbPath, &fsInfo) != -1) ){
          goLockless = (fsInfo.f_flags&MNT_RDONLY) == MNT_RDONLY;
        }
      }
      যদি( goLockless ){
        pCtx->conchHeld = -1; /* read only FS/ lockless */
        rc = SQLITE_OK;
      }
    }
  }
  যদি( rc==SQLITE_OK && lockPath ){
    pCtx->lockProxyPath = sqlite3DbStrDup(0, lockPath);
  }

  যদি( rc==SQLITE_OK ){
    pCtx->dbPath = sqlite3DbStrDup(0, dbPath);
    যদি( pCtx->dbPath==রিক্ত ){
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  যদি( rc==SQLITE_OK ){
    /* all memory is allocated, proxys are created and assigned,
    ** switch the locking context and pMethod then return.
    */
    pCtx->oldLockingContext = pFile->lockingContext;
    pFile->lockingContext = pCtx;
    pCtx->pOldMethod = pFile->pMethod;
    pFile->pMethod = &proxyIoMethods;
  }অন্যথা{
    যদি( pCtx->conchFile ){
      pCtx->conchFile->pMethod->xClose((sqlite3_file *)pCtx->conchFile);
      sqlite3_free(pCtx->conchFile);
    }
    sqlite3DbFree(0, pCtx->lockProxyPath);
    sqlite3_free(pCtx->conchFilePath);
    sqlite3_free(pCtx);
  }
  OSTRACE(("TRANSPROXY  %d %s\n", pFile->h,
           (rc==SQLITE_OK ? "ok" : "failed")));
  প্রত্যাগম rc;
}


/*
** This routine handles sqlite3_file_control() calls that are specific
** to proxy locking.
*/
স্থির পূর্ণাঙ্ক proxyFileControl(sqlite3_file *id, পূর্ণাঙ্ক op, শূন্য *pArg){
  বিকল্প( op ){
    পক্ষ SQLITE_FCNTL_GET_LOCKPROXYFILE: {
      unixFile *pFile = (unixFile*)id;
      যদি( pFile->pMethod == &proxyIoMethods ){
        proxyLockingContext *pCtx = (proxyLockingContext*)pFile->lockingContext;
        proxyTakeConch(pFile);
        যদি( pCtx->lockProxyPath ){
          *(ধ্রুব বর্ণ **)pArg = pCtx->lockProxyPath;
        }অন্যথা{
          *(ধ্রুব বর্ণ **)pArg = ":auto: (not held)";
        }
      } অন্যথা {
        *(ধ্রুব বর্ণ **)pArg = রিক্ত;
      }
      প্রত্যাগম SQLITE_OK;
    }
    পক্ষ SQLITE_FCNTL_SET_LOCKPROXYFILE: {
      unixFile *pFile = (unixFile*)id;
      পূর্ণাঙ্ক rc = SQLITE_OK;
      পূর্ণাঙ্ক isProxyStyle = (pFile->pMethod == &proxyIoMethods);
      যদি( pArg==রিক্ত || (ধ্রুব বর্ণ *)pArg==0 ){
        যদি( isProxyStyle ){
          /* turn off proxy locking - not supported.  If support is added for
          ** switching proxy locking mode off then it will need to fail if
          ** the journal mode is WAL mode.
          */
          rc = SQLITE_ERROR /*SQLITE_PROTOCOL? SQLITE_MISUSE?*/;
        }অন্যথা{
          /* turn off proxy locking - already off - NOOP */
          rc = SQLITE_OK;
        }
      }অন্যথা{
        ধ্রুব বর্ণ *proxyPath = (ধ্রুব বর্ণ *)pArg;
        যদি( isProxyStyle ){
          proxyLockingContext *pCtx =
            (proxyLockingContext*)pFile->lockingContext;
          যদি( !strcmp(pArg, ":auto:")
           || (pCtx->lockProxyPath &&
               !strncmp(pCtx->lockProxyPath, proxyPath, MAXPATHLEN))
          ){
            rc = SQLITE_OK;
          }অন্যথা{
            rc = switchLockProxyPath(pFile, proxyPath);
          }
        }অন্যথা{
          /* turn on proxy file locking */
          rc = proxyTransformUnixFile(pFile, proxyPath);
        }
      }
      প্রত্যাগম rc;
    }
    স্বভাব: {
      প্রতিজ্ঞা( 0 );  /* The call assures that only valid opcodes are sent */
    }
  }
  /*NOTREACHED*/ প্রতিজ্ঞা(0);
  প্রত্যাগম SQLITE_ERROR;
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
স্থির পূর্ণাঙ্ক proxyCheckReservedLock(sqlite3_file *id, পূর্ণাঙ্ক *pResOut) {
  unixFile *pFile = (unixFile*)id;
  পূর্ণাঙ্ক rc = proxyTakeConch(pFile);
  যদি( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    যদি( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->lockProxy;
      প্রত্যাগম proxy->pMethod->xCheckReservedLock((sqlite3_file*)proxy, pResOut);
    }অন্যথা{ /* conchHeld < 0 is lockless */
      pResOut=0;
    }
  }
  প্রত্যাগম rc;
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
স্থির পূর্ণাঙ্ক proxyLock(sqlite3_file *id, পূর্ণাঙ্ক eFileLock) {
  unixFile *pFile = (unixFile*)id;
  পূর্ণাঙ্ক rc = proxyTakeConch(pFile);
  যদি( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    যদি( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->lockProxy;
      rc = proxy->pMethod->xLock((sqlite3_file*)proxy, eFileLock);
      pFile->eFileLock = proxy->eFileLock;
    }অন্যথা{
      /* conchHeld < 0 is lockless */
    }
  }
  প্রত্যাগম rc;
}


/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
স্থির পূর্ণাঙ্ক proxyUnlock(sqlite3_file *id, পূর্ণাঙ্ক eFileLock) {
  unixFile *pFile = (unixFile*)id;
  পূর্ণাঙ্ক rc = proxyTakeConch(pFile);
  যদি( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    যদি( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->lockProxy;
      rc = proxy->pMethod->xUnlock((sqlite3_file*)proxy, eFileLock);
      pFile->eFileLock = proxy->eFileLock;
    }অন্যথা{
      /* conchHeld < 0 is lockless */
    }
  }
  প্রত্যাগম rc;
}

/*
** Close a file that uses proxy locks.
*/
স্থির পূর্ণাঙ্ক proxyClose(sqlite3_file *id) {
  যদি( ALWAYS(id) ){
    unixFile *pFile = (unixFile*)id;
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    unixFile *lockProxy = pCtx->lockProxy;
    unixFile *conchFile = pCtx->conchFile;
    পূর্ণাঙ্ক rc = SQLITE_OK;

    যদি( lockProxy ){
      rc = lockProxy->pMethod->xUnlock((sqlite3_file*)lockProxy, NO_LOCK);
      যদি( rc ) প্রত্যাগম rc;
      rc = lockProxy->pMethod->xClose((sqlite3_file*)lockProxy);
      যদি( rc ) প্রত্যাগম rc;
      sqlite3_free(lockProxy);
      pCtx->lockProxy = 0;
    }
    যদি( conchFile ){
      যদি( pCtx->conchHeld ){
        rc = proxyReleaseConch(pFile);
        যদি( rc ) প্রত্যাগম rc;
      }
      rc = conchFile->pMethod->xClose((sqlite3_file*)conchFile);
      যদি( rc ) প্রত্যাগম rc;
      sqlite3_free(conchFile);
    }
    sqlite3DbFree(0, pCtx->lockProxyPath);
    sqlite3_free(pCtx->conchFilePath);
    sqlite3DbFree(0, pCtx->dbPath);
    /* restore the original locking context and pMethod then close it */
    pFile->lockingContext = pCtx->oldLockingContext;
    pFile->pMethod = pCtx->pOldMethod;
    sqlite3_free(pCtx);
    প্রত্যাগম pFile->pMethod->xClose(id);
  }
  প্রত্যাগম SQLITE_OK;
}



#endif /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
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
পূর্ণাঙ্ক sqlite3_os_init(শূন্য){
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
  #define UNIXVFS(VFSNAME, FINDER) {                        \
    3,                    /* iVersion */                    \
    পরিমাণ(unixFile),     /* szOsFile */                    \
    MAX_PATHNAME,         /* mxPathname */                  \
    0,                    /* pNext */                       \
    VFSNAME,              /* zName */                       \
    (শূন্য*)&FINDER,       /* pAppData */                    \
    unixOpen,             /* xOpen */                       \
    unixDelete,           /* xDelete */                     \
    unixAccess,           /* xAccess */                     \
    unixFullPathname,     /* xFullPathname */               \
    unixDlOpen,           /* xDlOpen */                     \
    unixDlError,          /* xDlError */                    \
    unixDlSym,            /* xDlSym */                      \
    unixDlClose,          /* xDlClose */                    \
    unixRandomness,       /* xRandomness */                 \
    unixSleep,            /* xSleep */                      \
    unixCurrentTime,      /* xCurrentTime */                \
    unixGetLastError,     /* xGetLastError */               \
    unixCurrentTimeInt64, /* xCurrentTimeInt64 */           \
    unixSetSystemCall,    /* xSetSystemCall */              \
    unixGetSystemCall,    /* xGetSystemCall */              \
    unixNextSystemCall,   /* xNextSystemCall */             \
  }

  /*
  ** All default VFSes for unix are contained in the following array.
  **
  ** Note that the sqlite3_vfs.pNext field of the VFS object is modified
  ** by the SQLite core when the VFS is registered.  So the following
  ** array cannot be const.
  */
  স্থির sqlite3_vfs aVfs[] = {
#যদি SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    UNIXVFS("unix",          autolockIoFinder ),
#নোচেদ্যদি OS_VXWORKS
    UNIXVFS("unix",          vxworksIoFinder ),
#অন্যথা
    UNIXVFS("unix",          posixIoFinder ),
#endif
    UNIXVFS("unix-none",     nolockIoFinder ),
    UNIXVFS("unix-dotfile",  dotlockIoFinder ),
    UNIXVFS("unix-excl",     posixIoFinder ),
#যদি OS_VXWORKS
    UNIXVFS("unix-namedsem", semIoFinder ),
#endif
#যদি SQLITE_ENABLE_LOCKING_STYLE || OS_VXWORKS
    UNIXVFS("unix-posix",    posixIoFinder ),
#endif
#যদি SQLITE_ENABLE_LOCKING_STYLE
    UNIXVFS("unix-flock",    flockIoFinder ),
#endif
#যদি SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    UNIXVFS("unix-afp",      afpIoFinder ),
    UNIXVFS("unix-nfs",      nfsIoFinder ),
    UNIXVFS("unix-proxy",    proxyIoFinder ),
#endif
  };
  unsigned পূর্ণাঙ্ক i;          /* Loop counter */

  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  প্রতিজ্ঞা( ArraySize(aSyscall)==29 );

  /* Register all VFSes defined in the aVfs[] array */
  প্রতি(i=0; i<(পরিমাণ(aVfs)/পরিমাণ(sqlite3_vfs)); i++){
#ifdef SQLITE_DEFAULT_UNIX_VFS
    sqlite3_vfs_register(&aVfs[i],
           0==strcmp(aVfs[i].zName,SQLITE_DEFAULT_UNIX_VFS));
#অন্যথা
    sqlite3_vfs_register(&aVfs[i], i==0);
#endif
  }
#ifdef SQLITE_OS_KV_OPTIONAL
  sqlite3KvvfsInit();
#endif
  unixBigLock = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#যদি OS_VXWORKS
  vxworksMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS2);
#endif

#ifndef SQLITE_OMIT_WAL
  /* Validate lock assumptions */
  প্রতিজ্ঞা( SQLITE_SHM_NLOCK==8 );  /* Number of available locks */
  প্রতিজ্ঞা( UNIX_SHM_BASE==120  );  /* Start of locking area */
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
  প্রতিজ্ঞা( UNIX_SHM_DMS==128   );  /* Byte offset of the deadman-switch */
#endif

  প্রত্যাগম SQLITE_OK;
}

/*
** Shutdown the operating system interface.
**
** Some operating systems might need to do some cleanup in this routine,
** to release dynamically allocated objects.  But not on unix.
** This routine is a no-op for unix.
*/
পূর্ণাঙ্ক sqlite3_os_end(শূন্য){
  unixBigLock = 0;
#যদি OS_VXWORKS
  vxworksMutex = 0;
#endif
  প্রত্যাগম SQLITE_OK;
}

#endif /* SQLITE_OS_UNIX */
