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
#include "sqliteInt.h"
#གལ་སྲིད SQLITE_OS_UNIX              /* This file is used on unix only */

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
#གལ་སྲིད !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  གལ་སྲིད defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  དེ་མིན
#    define SQLITE_ENABLE_LOCKING_STYLE 0
#  endif
#endif

/* Use pread() and pwrite() if they are available */
#གལ་སྲིད defined(__APPLE__) || defined(__linux__)
# define HAVE_PREAD 1
# define HAVE_PWRITE 1
#endif
#གལ་སྲིད defined(HAVE_PREAD64) && defined(HAVE_PWRITE64)
# undef USE_PREAD
# define USE_PREAD64 1
#elif defined(HAVE_PREAD) && defined(HAVE_PWRITE)
# undef USE_PREAD64
# define USE_PREAD 1
#endif

/*
** standard include files.
*/
#include <sys/types.h>   /* amalgamator: keep */
#include <sys/stat.h>    /* amalgamator: keep */
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>      /* amalgamator: keep */
#include <time.h>
#include <sys/time.h>    /* amalgamator: keep */
#include <errno.h>
#གལ་སྲིད (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0) \
  && !defined(SQLITE_WASI)
# include <sys/mman.h>
#endif

#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE
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
# གལ་སྲིད defined(__APPLE__) && ((__MAC_OS_X_VERSION_MIN_REQUIRED > 1050) || \
                            (__IPHONE_OS_VERSION_MIN_REQUIRED > 2000))
#    གལ་སྲིད (!defined(TARGET_OS_EMBEDDED) || (TARGET_OS_EMBEDDED==0)) \
        && (!defined(TARGET_IPHONE_SIMULATOR) || (TARGET_IPHONE_SIMULATOR==0))\
        && (!defined(TARGET_OS_MACCATALYST) || (TARGET_OS_MACCATALYST==0))
#      undef HAVE_GETHOSTUUID
#      define HAVE_GETHOSTUUID 1
#    དེ་མིན
#      warning "gethostuuid() is disabled."
#    endif
#  endif
#endif


#གལ་སྲིད OS_VXWORKS
# include <sys/ioctl.h>
# include <semaphore.h>
# include <limits.h>
#endif /* OS_VXWORKS */

#གལ་སྲིད defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
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
#གལ་སྲིད SQLITE_THREADSAFE
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
#  གལ་སྲིད __LONG_MAX == 0x7fffffffL
#   define F_GETLK 12
#   define F_SETLK 13
#   define F_SETLKW 14
#  དེ་མིན
#   define F_GETLK 5
#   define F_SETLK 6
#   define F_SETLKW 7
#  endif
# endif
#དེ་མིན /* !SQLITE_WASI */
# ifndef HAVE_FCHMOD
#  define HAVE_FCHMOD 1
# endif
#endif /* SQLITE_WASI */

#ifdef SQLITE_WASI
# define osGetpid(X) (pid_t)1
#དེ་མིན
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
typedef sa_nracanaa unixShm unixShm;               /* Connection shared memory */
typedef sa_nracanaa unixShmNode unixShmNode;       /* Shared memory instance */
typedef sa_nracanaa unixInodeInfo unixInodeInfo;   /* An i-node */
typedef sa_nracanaa UnixUnusedFd UnixUnusedFd;     /* An unused file descriptor */

/*
** Sometimes, after a file handle is closed by SQLite, the file descriptor
** cannot be closed immediately. In these cases, instances of the following
** structure are used to store the file descriptor while waiting for an
** opportunity to either close or reuse it.
*/
sa_nracanaa UnixUnusedFd {
  ཧྲིལ fd;                   /* File descriptor to close */
  ཧྲིལ flags;                /* Flags this file descriptor was opened with */
  UnixUnusedFd *pNext;      /* Next unused file descriptor on same file */
};

/*
** The unixFile structure is subclass of sqlite3_file specific to the unix
** VFS implementations.
*/
typedef sa_nracanaa unixFile unixFile;
sa_nracanaa unixFile {
  sqlite3_io_methods const *pMethod;  /* Always the first entry */
  sqlite3_vfs *pVfs;                  /* The VFS that created this unixFile */
  unixInodeInfo *pInode;              /* Info about locks on this inode */
  ཧྲིལ h;                              /* The file descriptor */
  unsigned var_na eFileLock;            /* The type of lock held on this fd */
  unsigned short ཧྲིལ ctrlFlags;       /* Behavioral bits.  UNIXFILE_* flags */
  ཧྲིལ lastErrno;                      /* The unix errno from last I/O error */
  void *lockingContext;               /* Locking style specific state */
  UnixUnusedFd *pPreallocatedUnused;  /* Pre-allocated UnixUnusedFd */
  const var_na *zPath;                  /* Name of the file */
  unixShm *pShm;                      /* Shared memory segment information */
  ཧྲིལ szChunk;                        /* Configured by FCNTL_CHUNK_SIZE */
#གལ་སྲིད SQLITE_MAX_MMAP_SIZE>0
  ཧྲིལ nFetchOut;                      /* Number of outstanding xFetch refs */
  sqlite3_int64 mmapSize;             /* Usable size of mapping at pMapRegion */
  sqlite3_int64 mmapSizeActual;       /* Actual size of mapping at pMapRegion */
  sqlite3_int64 mmapSizeMax;          /* Configured FCNTL_MMAP_SIZE value */
  void *pMapRegion;                   /* Memory mapped region */
#endif
  ཧྲིལ sectorSize;                     /* Device sector size */
  ཧྲིལ deviceCharacteristics;          /* Precomputed device characteristics */
#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE
  ཧྲིལ openFlags;                      /* The flags specified at open() */
#endif
#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE || defined(__APPLE__)
  unsigned fsFlags;                   /* cached details from statfs() */
#endif
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  unsigned iBusyTimeout;              /* Wait this many millisec on locks */
  ཧྲིལ bBlockOnConnect;                /* True to block for SHARED locks */
#endif
#གལ་སྲིད OS_VXWORKS
  sa_nracanaa vxworksFileId *pId;          /* Unique file ID */
#endif
#ifdef SQLITE_DEBUG
  /* The next group of variables are used to track whether or not the
  ** transaction counter in bytes 24-27 of database files are updated
  ** whenever any part of the database changes.  An assertion fault will
  ** occur if a file is updated without also updating the transaction
  ** counter.  This test is made to avoid new problems similar to the
  ** one described by ticket #3584.
  */
  unsigned var_na transCntrChng;   /* True if the transaction counter changed */
  unsigned var_na dbUpdate;        /* True if any part of database file changed */
  unsigned var_na inNormalWrite;   /* True if in a normal write operation */

#endif

#ifdef SQLITE_TEST
  /* In test mode, increase the size of this structure a bit so that
  ** it is larger than the struct CrashFile defined in test6.c.
  */
  var_na aPadding[32];
#endif
};

/* This variable holds the process id (pid) from when the xRandomness()
** method was called.  If xOpen() is called from a different process id,
** indicating that a fork() has occurred, the PRNG will be reset.
*/
static pid_t randomnessPid = 0;

/*
** Allowed values for the unixFile.ctrlFlags bitmask:
*/
#define UNIXFILE_EXCL        0x01     /* Connections from one process only */
#define UNIXFILE_RDONLY      0x02     /* Connection is read only */
#define UNIXFILE_PERSIST_WAL 0x04     /* Persistent WAL mode */
#གལ་སྲིད !defined(SQLITE_DISABLE_DIRSYNC) && !defined(_AIX)
# define UNIXFILE_DIRSYNC    0x08     /* Directory sync needed */
#དེ་མིན
# define UNIXFILE_DIRSYNC    0x00
#endif
#define UNIXFILE_PSOW        0x10     /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */
#define UNIXFILE_DELETE      0x20     /* Delete on close */
#define UNIXFILE_URI         0x40     /* Filename might have query parameters */
#define UNIXFILE_NOLOCK      0x80     /* Do no file locking */

/*
** Include code that is common to all os_*.c files
*/
#include "os_common.h"

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
#ifndef O_BINARY
# define O_BINARY 0
#endif

/*
** The threadid macro resolves to the thread-id or to 0.  Used for
** testing and debugging only.
*/
#གལ་སྲིད SQLITE_THREADSAFE
#define threadid pthread_self()
#དེ་མིན
#define threadid 0
#endif

/*
** HAVE_MREMAP defaults to true on Linux and false everywhere else.
*/
#གལ་སྲིད !defined(HAVE_MREMAP)
# གལ་སྲིད defined(__linux__) && defined(_GNU_SOURCE)
#  define HAVE_MREMAP 1
# དེ་མིན
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
static ཧྲིལ posixOpen(const var_na *zFile, ཧྲིལ flags, ཧྲིལ mode){
  ལོག open(zFile, flags, mode);
}

/* Forward reference */
static ཧྲིལ openDirectory(const var_na*, ཧྲིལ*);
static ཧྲིལ unixGetpagesize(void);

/*
** Many system calls are accessed through pointer-to-functions so that
** they may be overridden at runtime to facilitate fault injection during
** testing and sandboxing.  The following array holds the names and pointers
** to all overrideable system calls.
*/
static sa_nracanaa unix_syscall {
  const var_na *zName;            /* Name of the system call */
  sqlite3_syscall_ptr pCurrent; /* Current value of the system call */
  sqlite3_syscall_ptr pDefault; /* Default value */
} aSyscall[] = {
  { "open",         (sqlite3_syscall_ptr)posixOpen,  0  },
#define osOpen      ((ཧྲིལ(*)(const var_na*,ཧྲིལ,ཧྲིལ))aSyscall[0].pCurrent)

  { "close",        (sqlite3_syscall_ptr)close,      0  },
#define osClose     ((ཧྲིལ(*)(ཧྲིལ))aSyscall[1].pCurrent)

  { "access",       (sqlite3_syscall_ptr)access,     0  },
#define osAccess    ((ཧྲིལ(*)(const var_na*,ཧྲིལ))aSyscall[2].pCurrent)

  { "getcwd",       (sqlite3_syscall_ptr)getcwd,     0  },
#define osGetcwd    ((var_na*(*)(var_na*,size_t))aSyscall[3].pCurrent)

  { "stat",         (sqlite3_syscall_ptr)stat,       0  },
#define osStat      ((ཧྲིལ(*)(const var_na*,sa_nracanaa stat*))aSyscall[4].pCurrent)

/*
** The DJGPP compiler environment looks mostly like Unix, but it
** lacks the fcntl() system call.  So redefine fcntl() to be something
** that always succeeds.  This means that locking does not occur under
** DJGPP.  But it is DOS - what did you expect?
*/
#ifdef __DJGPP__
  { "fstat",        0,                 0  },
#define osFstat(a,b,c)    0
#དེ་མིན   
  { "fstat",        (sqlite3_syscall_ptr)fstat,      0  },
#define osFstat     ((ཧྲིལ(*)(ཧྲིལ,sa_nracanaa stat*))aSyscall[5].pCurrent)
#endif

  { "ftruncate",    (sqlite3_syscall_ptr)ftruncate,  0  },
#define osFtruncate ((ཧྲིལ(*)(ཧྲིལ,off_t))aSyscall[6].pCurrent)

  { "fcntl",        (sqlite3_syscall_ptr)fcntl,      0  },
#define osFcntl     ((ཧྲིལ(*)(ཧྲིལ,ཧྲིལ,...))aSyscall[7].pCurrent)

  { "read",         (sqlite3_syscall_ptr)read,       0  },
#define osRead      ((ssize_t(*)(ཧྲིལ,void*,size_t))aSyscall[8].pCurrent)

#གལ་སྲིད defined(USE_PREAD) || SQLITE_ENABLE_LOCKING_STYLE
  { "pread",        (sqlite3_syscall_ptr)pread,      0  },
#དེ་མིན
  { "pread",        (sqlite3_syscall_ptr)0,          0  },
#endif
#define osPread     ((ssize_t(*)(ཧྲིལ,void*,size_t,off_t))aSyscall[9].pCurrent)

#གལ་སྲིད defined(USE_PREAD64)
  { "pread64",      (sqlite3_syscall_ptr)pread64,    0  },
#དེ་མིན
  { "pread64",      (sqlite3_syscall_ptr)0,          0  },
#endif
#define osPread64 ((ssize_t(*)(ཧྲིལ,void*,size_t,off64_t))aSyscall[10].pCurrent)

  { "write",        (sqlite3_syscall_ptr)write,      0  },
#define osWrite     ((ssize_t(*)(ཧྲིལ,const void*,size_t))aSyscall[11].pCurrent)

#གལ་སྲིད defined(USE_PREAD) || SQLITE_ENABLE_LOCKING_STYLE
  { "pwrite",       (sqlite3_syscall_ptr)pwrite,     0  },
#དེ་མིན
  { "pwrite",       (sqlite3_syscall_ptr)0,          0  },
#endif
#define osPwrite    ((ssize_t(*)(ཧྲིལ,const void*,size_t,off_t))\
                    aSyscall[12].pCurrent)

#གལ་སྲིད defined(USE_PREAD64)
  { "pwrite64",     (sqlite3_syscall_ptr)pwrite64,   0  },
#དེ་མིན
  { "pwrite64",     (sqlite3_syscall_ptr)0,          0  },
#endif
#define osPwrite64  ((ssize_t(*)(ཧྲིལ,const void*,size_t,off64_t))\
                    aSyscall[13].pCurrent)

#གལ་སྲིད defined(HAVE_FCHMOD)
  { "fchmod",       (sqlite3_syscall_ptr)fchmod,          0  },
#define osFchmod    ((ཧྲིལ(*)(ཧྲིལ,mode_t))aSyscall[14].pCurrent)
#དེ་མིན
  { "fchmod",       (sqlite3_syscall_ptr)0,               0  },
#define osFchmod(FID,MODE) 0
#endif

#གལ་སྲིད defined(HAVE_POSIX_FALLOCATE) && HAVE_POSIX_FALLOCATE
  { "fallocate",    (sqlite3_syscall_ptr)posix_fallocate,  0 },
#དེ་མིན
  { "fallocate",    (sqlite3_syscall_ptr)0,                0 },
#endif
#define osFallocate ((ཧྲིལ(*)(ཧྲིལ,off_t,off_t))aSyscall[15].pCurrent)

  { "unlink",       (sqlite3_syscall_ptr)unlink,           0 },
#define osUnlink    ((ཧྲིལ(*)(const var_na*))aSyscall[16].pCurrent)

  { "openDirectory",    (sqlite3_syscall_ptr)openDirectory,      0 },
#define osOpenDirectory ((ཧྲིལ(*)(const var_na*,ཧྲིལ*))aSyscall[17].pCurrent)

  { "mkdir",        (sqlite3_syscall_ptr)mkdir,           0 },
#define osMkdir     ((ཧྲིལ(*)(const var_na*,mode_t))aSyscall[18].pCurrent)

  { "rmdir",        (sqlite3_syscall_ptr)rmdir,           0 },
#define osRmdir     ((ཧྲིལ(*)(const var_na*))aSyscall[19].pCurrent)

#གལ་སྲིད defined(HAVE_FCHOWN)
  { "fchown",       (sqlite3_syscall_ptr)fchown,          0 },
#དེ་མིན
  { "fchown",       (sqlite3_syscall_ptr)0,               0 },
#endif
#define osFchown    ((ཧྲིལ(*)(ཧྲིལ,uid_t,gid_t))aSyscall[20].pCurrent)

#གལ་སྲིད defined(HAVE_FCHOWN)
  { "geteuid",      (sqlite3_syscall_ptr)geteuid,         0 },
#དེ་མིན
  { "geteuid",      (sqlite3_syscall_ptr)0,               0 },
#endif
#define osGeteuid   ((uid_t(*)(void))aSyscall[21].pCurrent)

#གལ་སྲིད (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0) \
  && !defined(SQLITE_WASI)
  { "mmap",         (sqlite3_syscall_ptr)mmap,            0 },
#དེ་མིན
  { "mmap",         (sqlite3_syscall_ptr)0,               0 },
#endif
#define osMmap ((void*(*)(void*,size_t,ཧྲིལ,ཧྲིལ,ཧྲིལ,off_t))aSyscall[22].pCurrent)

#གལ་སྲིད (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0) \
  && !defined(SQLITE_WASI)
  { "munmap",       (sqlite3_syscall_ptr)munmap,          0 },
#དེ་མིན
  { "munmap",       (sqlite3_syscall_ptr)0,               0 },
#endif
#define osMunmap ((ཧྲིལ(*)(void*,size_t))aSyscall[23].pCurrent)

#གལ་སྲིད HAVE_MREMAP && (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0)
  { "mremap",       (sqlite3_syscall_ptr)mremap,          0 },
#དེ་མིན
  { "mremap",       (sqlite3_syscall_ptr)0,               0 },
#endif
#define osMremap ((void*(*)(void*,size_t,size_t,ཧྲིལ,...))aSyscall[24].pCurrent)

#གལ་སྲིད !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0
  { "getpagesize",  (sqlite3_syscall_ptr)unixGetpagesize, 0 },
#དེ་མིན
  { "getpagesize",  (sqlite3_syscall_ptr)0,               0 },
#endif
#define osGetpagesize ((ཧྲིལ(*)(void))aSyscall[25].pCurrent)

#གལ་སྲིད defined(HAVE_READLINK)
  { "readlink",     (sqlite3_syscall_ptr)readlink,        0 },
#དེ་མིན
  { "readlink",     (sqlite3_syscall_ptr)0,               0 },
#endif
#define osReadlink ((ssize_t(*)(const var_na*,var_na*,size_t))aSyscall[26].pCurrent)

#གལ་སྲིད defined(HAVE_LSTAT)
  { "lstat",         (sqlite3_syscall_ptr)lstat,          0 },
#དེ་མིན
  { "lstat",         (sqlite3_syscall_ptr)0,              0 },
#endif
#define osLstat      ((ཧྲིལ(*)(const var_na*,sa_nracanaa stat*))aSyscall[27].pCurrent)

#གལ་སྲིད defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
# ifdef __ANDROID__
  { "ioctl", (sqlite3_syscall_ptr)(ཧྲིལ(*)(ཧྲིལ, ཧྲིལ, ...))ioctl, 0 },
#define osIoctl ((ཧྲིལ(*)(ཧྲིལ,ཧྲིལ,...))aSyscall[28].pCurrent)
# དེ་མིན
  { "ioctl",         (sqlite3_syscall_ptr)ioctl,          0 },
#define osIoctl ((ཧྲིལ(*)(ཧྲིལ,unsigned long,...))aSyscall[28].pCurrent)
# endif
#དེ་མིན
  { "ioctl",         (sqlite3_syscall_ptr)0,              0 },
#endif

}; /* End of the overrideable system calls */


#གལ་སྲིད defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
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
static ཧྲིལ unixPosixAdvisoryLocks(
  ཧྲིལ fd,        /* The file descriptor to analyze */
  var_na a[16]     /* Write a text description of PALs here */
){
  ཧྲིལ in;
  ssize_t n;
  var_na *p, *pNext, *x;
  var_na z[2000];

        /*             1     */
        /*   012 4 678 01234 */
  memcpy(a, "---/-.---.-----", 16);
  sqlite3_snprintf(parimaa_na(z), z, "/proc/%d/fdinfo/%d", getpid(), fd);
  in = osOpen(z, O_RDONLY, 0);
  གལ་སྲིད( in<0 ){
    ལོག SQLITE_ERROR_UNABLE;
  }
  n = osRead(in, z, parimaa_na(z)-1);
  osClose(in);
  གལ་སྲིད( n<=0 ) ལོག SQLITE_ERROR_UNABLE;
  z[n] = 0;

  /* We are looking for lines that begin with "lock:\t".  Examples:
  **
  ** lock: 1: POSIX  ADVISORY  READ 494716 08:02:5277597 1073741826 1073742335
  ** lock: 1: POSIX  ADVISORY  WRITE 494716 08:02:5282282 120 120
  ** lock: 2: POSIX  ADVISORY  READ 494716 08:02:5282282 123 123
  ** lock: 3: POSIX  ADVISORY  READ 494716 08:02:5282282 128 128
  */
  pNext = strstr(z, "lock:\t");
  སྐབས( pNext ){
    var_na cType = 0;
    sqlite3_int64 iFirst, iLast;
    p = pNext+6;
    pNext = strstr(p, "lock:\t");
    གལ་སྲིད( pNext ) pNext[-1] = 0;
    གལ་སྲིད( (x = strstr(p, " READ "))!=0 ){
      cType = 'r';
      x += 6;
    }དེ་མིན གལ་སྲིད( (x = strstr(p, " WRITE "))!=0 ){
      cType = 'w';
      x += 7;
    }དེ་མིན{
      continue;
    }
    x = strrchr(x, ' ');
    གལ་སྲིད( x==0 ) continue;
    iLast = strtoll(x+1, 0, 10);
    *x = 0;
    x = strrchr(p, ' ');
    གལ་སྲིད( x==0 ) continue;
    iFirst = strtoll(x+1, 0, 10);
    གལ་སྲིད( iLast>=PENDING_BYTE ){
      གལ་སྲིད( iFirst<=PENDING_BYTE && iLast>=PENDING_BYTE )     a[0] = cType;
      གལ་སྲིད( iFirst<=PENDING_BYTE+1 && iLast>=PENDING_BYTE+1 ) a[1] = cType;
      གལ་སྲིད( iFirst<=PENDING_BYTE+2 && iLast>=PENDING_BYTE+510 ) a[2] = cType;
    }དེ་མིན གལ་སྲིད( iLast<=128 ){
      གལ་སྲིད( iFirst<=128 && iLast>=128 ) a[4] = cType;
      གལ་སྲིད( iFirst<=120 && iLast>=120 ) a[6] = cType;
      གལ་སྲིད( iFirst<=121 && iLast>=121 ) a[7] = cType;
      གལ་སྲིད( iFirst<=122 && iLast>=122 ) a[8] = cType;
      གལ་སྲིད( iFirst<=123 && iLast>=123 ) a[10] = cType;
      གལ་སྲིད( iFirst<=124 && iLast>=124 ) a[11] = cType;
      གལ་སྲིད( iFirst<=125 && iLast>=125 ) a[12] = cType;
      གལ་སྲིད( iFirst<=126 && iLast>=126 ) a[13] = cType;
      གལ་སྲིད( iFirst<=127 && iLast>=127 ) a[14] = cType;
    }
  }
  ལོག SQLITE_OK;
}
#དེ་མིན
#  define unixPosixAdvisoryLocks(A,B) SQLITE_ERROR_UNABLE
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

/*
** On some systems, calls to fchown() will trigger a message in a security
** log if they come from non-root processes.  So avoid calling fchown() if
** we are not running as root.
*/
static ཧྲིལ robustFchown(ཧྲིལ fd, uid_t uid, gid_t gid){
#གལ་སྲིད defined(HAVE_FCHOWN)
  ལོག osGeteuid() ? 0 : osFchown(fd,uid,gid);
#དེ་མིན
  ལོག 0;
#endif
}

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "unix" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
static ཧྲིལ unixSetSystemCall(
  sqlite3_vfs *pNotUsed,        /* The VFS pointer.  Not used */
  const var_na *zName,            /* Name of system call to override */
  sqlite3_syscall_ptr pNewFunc  /* Pointer to new system call value */
){
  unsigned ཧྲིལ i;
  ཧྲིལ rc = SQLITE_NOTFOUND;

  UNUSED_PARAMETER(pNotUsed);
  གལ་སྲིད( zName==0 ){
    /* If no zName is given, restore all system calls to their default
    ** settings and return NULL
    */
    rc = SQLITE_OK;
    དོན་ལུ(i=0; i<parimaa_na(aSyscall)/parimaa_na(aSyscall[0]); i++){
      གལ་སྲིད( aSyscall[i].pDefault ){
        aSyscall[i].pCurrent = aSyscall[i].pDefault;
      }
    }
  }དེ་མིན{
    /* If zName is specified, operate on only the one system call
    ** specified.
    */
    དོན་ལུ(i=0; i<parimaa_na(aSyscall)/parimaa_na(aSyscall[0]); i++){
      གལ་སྲིད( strcmp(zName, aSyscall[i].zName)==0 ){
        གལ་སྲིད( aSyscall[i].pDefault==0 ){
          aSyscall[i].pDefault = aSyscall[i].pCurrent;
        }
        rc = SQLITE_OK;
        གལ་སྲིད( pNewFunc==0 ) pNewFunc = aSyscall[i].pDefault;
        aSyscall[i].pCurrent = pNewFunc;
        bha_nga;
      }
    }
  }
  ལོག rc;
}

/*
** Return the value of a system call.  Return NULL if zName is not a
** recognized system call name.  NULL is also returned if the system call
** is currently undefined.
*/
static sqlite3_syscall_ptr unixGetSystemCall(
  sqlite3_vfs *pNotUsed,
  const var_na *zName
){
  unsigned ཧྲིལ i;

  UNUSED_PARAMETER(pNotUsed);
  དོན་ལུ(i=0; i<parimaa_na(aSyscall)/parimaa_na(aSyscall[0]); i++){
    གལ་སྲིད( strcmp(zName, aSyscall[i].zName)==0 ) ལོག aSyscall[i].pCurrent;
  }
  ལོག 0;
}

/*
** Return the name of the first system call after zName.  If zName==NULL
** then return the name of the first system call.  Return NULL if zName
** is the last system call or if zName is not the name of a valid
** system call.
*/
static const var_na *unixNextSystemCall(sqlite3_vfs *p, const var_na *zName){
  ཧྲིལ i = -1;

  UNUSED_PARAMETER(p);
  གལ་སྲིད( zName ){
    དོན་ལུ(i=0; i<ArraySize(aSyscall)-1; i++){
      གལ་སྲིད( strcmp(zName, aSyscall[i].zName)==0 ) bha_nga;
    }
  }
  དོན་ལུ(i++; i<ArraySize(aSyscall); i++){
    གལ་སྲིད( aSyscall[i].pCurrent!=0 ) ལོག aSyscall[i].zName;
  }
  ལོག 0;
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
static ཧྲིལ robust_open(const var_na *z, ཧྲིལ f, mode_t m){
  ཧྲིལ fd;
  mode_t m2 = m ? m : SQLITE_DEFAULT_FILE_PERMISSIONS;
  སྐབས(1){
#གལ་སྲིད defined(O_CLOEXEC)
    fd = osOpen(z,f|O_CLOEXEC,m2);
#དེ་མིན
    fd = osOpen(z,f,m2);
#endif
    གལ་སྲིད( fd<0 ){
      གལ་སྲིད( errno==EINTR ) continue;
      bha_nga;
    }
    གལ་སྲིད( fd>=SQLITE_MINIMUM_FILE_DESCRIPTOR ) bha_nga;
    གལ་སྲིད( (f & (O_EXCL|O_CREAT))==(O_EXCL|O_CREAT) ){
      (void)osUnlink(z);
    }
    osClose(fd);
    sqlite3_log(SQLITE_WARNING,
                "attempt to open \"%s\" as file descriptor %d", z, fd);
    fd = -1;
    གལ་སྲིད( osOpen("/dev/null", O_RDONLY, m)<0 ) bha_nga;
  }
  གལ་སྲིད( fd>=0 ){
    གལ་སྲིད( m!=0 ){
      sa_nracanaa stat statbuf;
      གལ་སྲིད( osFstat(fd, &statbuf)==0
       && statbuf.st_size==0
       && (statbuf.st_mode&0777)!=m
      ){
        osFchmod(fd, m);
      }
    }
#གལ་སྲིད defined(FD_CLOEXEC) && (!defined(O_CLOEXEC) || O_CLOEXEC==0)
    osFcntl(fd, F_SETFD, osFcntl(fd, F_GETFD, 0) | FD_CLOEXEC);
#endif
  }
  ལོག fd;
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
static sqlite3_mutex *unixBigLock = 0;
static void unixEnterMutex(void){
  assert( sqlite3_mutex_notheld(unixBigLock) );  /* Not a recursive mutex */
  sqlite3_mutex_enter(unixBigLock);
}
static void unixLeaveMutex(void){
  assert( sqlite3_mutex_held(unixBigLock) );
  sqlite3_mutex_leave(unixBigLock);
}
#ifdef SQLITE_DEBUG
static ཧྲིལ unixMutexHeld(void) {
  ལོག sqlite3_mutex_held(unixBigLock);
}
#endif


#ifdef SQLITE_HAVE_OS_TRACE
/*
** Helper function for printing out trace information from debugging
** binaries. This returns the string representation of the supplied
** integer lock-type.
*/
static const var_na *azFileLock(ཧྲིལ eFileLock){
  switch( eFileLock ){
    pak_sa NO_LOCK: ལོག "NONE";
    pak_sa SHARED_LOCK: ལོག "SHARED";
    pak_sa RESERVED_LOCK: ལོག "RESERVED";
    pak_sa PENDING_LOCK: ལོག "PENDING";
    pak_sa EXCLUSIVE_LOCK: ལོག "EXCLUSIVE";
  }
  ལོག "ERROR";
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
static ཧྲིལ lockTrace(ཧྲིལ fd, ཧྲིལ op, sa_nracanaa flock *p){
  var_na *zOpName, *zType;
  ཧྲིལ s;
  ཧྲིལ savedErrno;
  གལ་སྲིད( op==F_GETLK ){
    zOpName = "GETLK";
  }དེ་མིན གལ་སྲིད( op==F_SETLK ){
    zOpName = "SETLK";
  }དེ་མིན{
    s = osFcntl(fd, op, p);
    sqlite3DebugPrintf("fcntl unknown %d %d %d\n", fd, op, s);
    ལོག s;
  }
  གལ་སྲིད( p->l_type==F_RDLCK ){
    zType = "RDLCK";
  }དེ་མིན གལ་སྲིད( p->l_type==F_WRLCK ){
    zType = "WRLCK";
  }དེ་མིན གལ་སྲིད( p->l_type==F_UNLCK ){
    zType = "UNLCK";
  }དེ་མིན{
    assert( 0 );
  }
  assert( p->l_whence==SEEK_SET );
  s = osFcntl(fd, op, p);
  savedErrno = errno;
  sqlite3DebugPrintf("fcntl %d %d %s %s %d %d %d %d\n",
     threadid, fd, zOpName, zType, (ཧྲིལ)p->l_start, (ཧྲིལ)p->l_len,
     (ཧྲིལ)p->l_pid, s);
  གལ་སྲིད( s==(-1) && op==F_SETLK && (p->l_type==F_RDLCK || p->l_type==F_WRLCK) ){
    sa_nracanaa flock l2;
    l2 = *p;
    osFcntl(fd, F_GETLK, &l2);
    གལ་སྲིད( l2.l_type==F_RDLCK ){
      zType = "RDLCK";
    }དེ་མིན གལ་སྲིད( l2.l_type==F_WRLCK ){
      zType = "WRLCK";
    }དེ་མིན གལ་སྲིད( l2.l_type==F_UNLCK ){
      zType = "UNLCK";
    }དེ་མིན{
      assert( 0 );
    }
    sqlite3DebugPrintf("fcntl-failure-reason: %s %d %d %d\n",
       zType, (ཧྲིལ)l2.l_start, (ཧྲིལ)l2.l_len, (ཧྲིལ)l2.l_pid);
  }
  errno = savedErrno;
  ལོག s;
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
static ཧྲིལ robust_ftruncate(ཧྲིལ h, sqlite3_int64 sz){
  ཧྲིལ rc;
#ifdef __ANDROID__
  /* On Android, ftruncate() always uses 32-bit offsets, even if
  ** _FILE_OFFSET_BITS=64 is defined. This means it is unsafe to attempt to
  ** truncate a file to any size larger than 2GiB. Silently ignore any
  ** such attempts.  */
  གལ་སྲིད( sz>(sqlite3_int64)0x7FFFFFFF ){
    rc = SQLITE_OK;
  }དེ་མིན
#endif
  do{ rc = osFtruncate(h,sz); }སྐབས( rc<0 && errno==EINTR );
  ལོག rc;
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
static ཧྲིལ sqliteErrorFromPosixError(ཧྲིལ posixError, ཧྲིལ sqliteIOErr) {
  assert( (sqliteIOErr == SQLITE_IOERR_LOCK) ||
          (sqliteIOErr == SQLITE_IOERR_UNLOCK) ||
          (sqliteIOErr == SQLITE_IOERR_RDLOCK) ||
          (sqliteIOErr == SQLITE_IOERR_CHECKRESERVEDLOCK) );
  switch (posixError) {
  pak_sa EACCES:
  pak_sa EAGAIN:
  pak_sa ETIMEDOUT:
  pak_sa EBUSY:
  pak_sa EINTR:
  pak_sa ENOLCK:
    /* random NFS retry error, unless during file system support
     * introspection, in which it actually means what it says */
    ལོག SQLITE_BUSY;
  
  pak_sa EPERM:
    ལོག SQLITE_PERM;
  
  default:
    ལོག sqliteIOErr;
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
sa_nracanaa vxworksFileId {
  sa_nracanaa vxworksFileId *pNext;  /* Next in a list of them all */
  ཧྲིལ nRef;                     /* Number of references to this one */
  ཧྲིལ nName;                    /* Length of the zCanonicalName[] string */
  var_na *zCanonicalName;         /* Canonical filename */
};

#གལ་སྲིད OS_VXWORKS
/*
** All unique filenames are held on a linked list headed by this
** variable:
*/
static sa_nracanaa vxworksFileId *vxworksFileList = 0;
static sqlite3_mutex *vxworksMutex = 0;

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
static ཧྲིལ vxworksSimplifyName(var_na *z, ཧྲིལ n){
  ཧྲིལ i, j;
  སྐབས( n>1 && z[n-1]=='/' ){ n--; }
  དོན་ལུ(i=j=0; i<n; i++){
    གལ་སྲིད( z[i]=='/' ){
      གལ་སྲིད( z[i+1]=='/' ) continue;
      གལ་སྲིད( z[i+1]=='.' && i+2<n && z[i+2]=='/' ){
        i += 1;
        continue;
      }
      གལ་སྲིད( z[i+1]=='.' && i+3<n && z[i+2]=='.' && z[i+3]=='/' ){
        སྐབས( j>0 && z[j-1]!='/' ){ j--; }
        གལ་སྲིད( j>0 ){ j--; }
        i += 2;
        continue;
      }
    }
    z[j++] = z[i];
  }
  z[j] = 0;
  ལོག j;
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
static sa_nracanaa vxworksFileId *vxworksFindFileId(const var_na *zAbsoluteName){
  sa_nracanaa vxworksFileId *pNew;         /* search key and new file ID */
  sa_nracanaa vxworksFileId *pCandidate;   /* For looping over existing file IDs */
  ཧྲིལ n;                              /* Length of zAbsoluteName string */

  assert( zAbsoluteName[0]=='/' );
  n = (ཧྲིལ)strlen(zAbsoluteName);
  pNew = sqlite3_malloc64( parimaa_na(*pNew) + (n+1) );
  གལ་སྲིད( pNew==0 ) ལོག 0;
  pNew->zCanonicalName = (var_na*)&pNew[1];
  memcpy(pNew->zCanonicalName, zAbsoluteName, n+1);
  n = vxworksSimplifyName(pNew->zCanonicalName, n);

  /* Search for an existing entry that matching the canonical name.
  ** If found, increment the reference count and return a pointer to
  ** the existing file ID.
  */
  sqlite3_mutex_enter(vxworksMutex);
  དོན་ལུ(pCandidate=vxworksFileList; pCandidate; pCandidate=pCandidate->pNext){
    གལ་སྲིད( pCandidate->nName==n
     && memcmp(pCandidate->zCanonicalName, pNew->zCanonicalName, n)==0
    ){
       sqlite3_free(pNew);
       pCandidate->nRef++;
       sqlite3_mutex_leave(vxworksMutex);
       ལོག pCandidate;
    }
  }

  /* No match was found.  We will make a new file ID */
  pNew->nRef = 1;
  pNew->nName = n;
  pNew->pNext = vxworksFileList;
  vxworksFileList = pNew;
  sqlite3_mutex_leave(vxworksMutex);
  ལོག pNew;
}

/*
** Decrement the reference count on a vxworksFileId object.  Free
** the object when the reference count reaches zero.
*/
static void vxworksReleaseFileId(sa_nracanaa vxworksFileId *pId){
  sqlite3_mutex_enter(vxworksMutex);
  assert( pId->nRef>0 );
  pId->nRef--;
  གལ་སྲིད( pId->nRef==0 ){
    sa_nracanaa vxworksFileId **pp;
    དོན་ལུ(pp=&vxworksFileList; *pp && *pp!=pId; pp = &((*pp)->pNext)){}
    assert( *pp==pId );
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
sa_nracanaa unixFileId {
  dev_t dev;                  /* Device number */
#གལ་སྲིད OS_VXWORKS
  sa_nracanaa vxworksFileId *pId;  /* Unique file ID for vxworks. */
#དེ་མིན
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
sa_nracanaa unixInodeInfo {
  sa_nracanaa unixFileId fileId;       /* The lookup key */
  sqlite3_mutex *pLockMutex;      /* Hold this mutex for... */
  ཧྲིལ nShared;                      /* Number of SHARED locks held */
  ཧྲིལ nLock;                        /* Number of outstanding file locks */
  unsigned var_na eFileLock;          /* One of SHARED_LOCK, RESERVED_LOCK etc. */
  unsigned var_na bProcessLock;       /* An exclusive process lock is held */
  UnixUnusedFd *pUnused;            /* Unused file descriptors to close */
  ཧྲིལ nRef;                       /* Number of pointers to this structure */
  unixShmNode *pShmNode;          /* Shared memory associated with this inode */
  unixInodeInfo *pNext;           /* List of all unixInodeInfo objects */
  unixInodeInfo *pPrev;           /*    .... doubly linked */
#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE
  unsigned long long sharedByte;  /* for AFP simulated shared lock */
#endif
#གལ་སྲིད OS_VXWORKS
  sem_t *pSem;                    /* Named POSIX semaphore */
  var_na aSemName[MAX_PATHNAME+2];  /* Name of that semaphore */
#endif
};

/*
** A lists of all unixInodeInfo objects.
**
** Must hold unixBigLock in order to read or write this variable.
*/
static unixInodeInfo *inodeList = 0;  /* All unixInodeInfo objects */

#ifdef SQLITE_DEBUG
/*
** True if the inode mutex (on the unixFile.pFileMutex field) is held, or not.
** This routine is used only within assert() to help verify correct mutex
** usage.
*/
ཧྲིལ unixFileMutexHeld(unixFile *pFile){
  assert( pFile->pInode );
  ལོག sqlite3_mutex_held(pFile->pInode->pLockMutex);
}
ཧྲིལ unixFileMutexNotheld(unixFile *pFile){
  assert( pFile->pInode );
  ལོག sqlite3_mutex_notheld(pFile->pInode->pLockMutex);
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
#define unixLogError(a,b,c)     unixLogErrorAtLine(a,b,c,__LINE__)
static ཧྲིལ unixLogErrorAtLine(
  ཧྲིལ errcode,                    /* SQLite error code */
  const var_na *zFunc,              /* Name of OS function that failed */
  const var_na *zPath,              /* File path associated with error */
  ཧྲིལ iLine                       /* Source line number where error occurred */
){
  var_na *zErr;                     /* Message from strerror() or equivalent */
  ཧྲིལ iErrno = errno;             /* Saved syscall error number */

  /* If this is not a threadsafe build (SQLITE_THREADSAFE==0), then use
  ** the strerror() function to obtain the human-readable error message
  ** equivalent to errno. Otherwise, use strerror_r().
  */
#གལ་སྲིད SQLITE_THREADSAFE && defined(HAVE_STRERROR_R)
  var_na aErr[80];
  memset(aErr, 0, parimaa_na(aErr));
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
#གལ་སྲིད (defined(STRERROR_R_CHAR_P) || defined(__USE_GNU)) \
  && !defined(ANDROID) && !defined(__ANDROID__)
  zErr =
# endif
  strerror_r(iErrno, aErr, parimaa_na(aErr)-1);

#elif SQLITE_THREADSAFE
  /* This is a threadsafe build, but strerror_r() is not available. */
  zErr = "";
#དེ་མིན
  /* Non-threadsafe build, use strerror(). */
  zErr = strerror(iErrno);
#endif

  གལ་སྲིད( zPath==0 ) zPath = "";
  sqlite3_log(errcode,
      "os_unix.c:%d: (%d) %s(%s) - %s",
      iLine, iErrno, zFunc, zPath, zErr
  );

  ལོག errcode;
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
static void robust_close(unixFile *pFile, ཧྲིལ h, ཧྲིལ lineno){
  གལ་སྲིད( osClose(h) ){
    unixLogErrorAtLine(SQLITE_IOERR_CLOSE, "close",
                       pFile ? pFile->zPath : 0, lineno);
  }
}

/*
** Set the pFile->lastErrno.  Do this in a subroutine as that provides
** a convenient place to set a breakpoint.
*/
static void storeLastErrno(unixFile *pFile, ཧྲིལ error){
  pFile->lastErrno = error;
}

/*
** Close all file descriptors accumulated in the unixInodeInfo->pUnused list.
*/
static void closePendingFds(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  UnixUnusedFd *p;
  UnixUnusedFd *pNext;
  assert( unixFileMutexHeld(pFile) );
  དོན་ལུ(p=pInode->pUnused; p; p=pNext){
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
static void releaseInodeInfo(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  assert( unixMutexHeld() );
  assert( unixFileMutexNotheld(pFile) );
  གལ་སྲིད( ALWAYS(pInode) ){
    pInode->nRef--;
    གལ་སྲིད( pInode->nRef==0 ){
      assert( pInode->pShmNode==0 );
      sqlite3_mutex_enter(pInode->pLockMutex);
      closePendingFds(pFile);
      sqlite3_mutex_leave(pInode->pLockMutex);
      གལ་སྲིད( pInode->pPrev ){
        assert( pInode->pPrev->pNext==pInode );
        pInode->pPrev->pNext = pInode->pNext;
      }དེ་མིན{
        assert( inodeList==pInode );
        inodeList = pInode->pNext;
      }
      གལ་སྲིད( pInode->pNext ){
        assert( pInode->pNext->pPrev==pInode );
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
static ཧྲིལ findInodeInfo(
  unixFile *pFile,               /* Unix file with file desc used in the key */
  unixInodeInfo **ppInode        /* Return the unixInodeInfo object here */
){
  ཧྲིལ rc;                        /* System call return code */
  ཧྲིལ fd;                        /* The file descriptor for pFile */
  sa_nracanaa unixFileId fileId;      /* Lookup key for the unixInodeInfo */
  sa_nracanaa stat statbuf;           /* Low-level file information */
  unixInodeInfo *pInode = 0;     /* Candidate unixInodeInfo object */

  assert( unixMutexHeld() );

  /* Get low-level information about the file that we can used to
  ** create a unique name for the file.
  */
  fd = pFile->h;
  rc = osFstat(fd, &statbuf);
  གལ་སྲིད( rc!=0 ){
    storeLastErrno(pFile, errno);
#གལ་སྲིད defined(EOVERFLOW) && defined(SQLITE_DISABLE_LFS)
    གལ་སྲིད( pFile->lastErrno==EOVERFLOW ) ལོག SQLITE_NOLFS;
#endif
    ལོག SQLITE_IOERR;
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
  གལ་སྲིད( statbuf.st_size==0 && (pFile->fsFlags & SQLITE_FSFLAGS_IS_MSDOS)!=0 ){
    do{ rc = osWrite(fd, "S", 1); }སྐབས( rc<0 && errno==EINTR );
    གལ་སྲིད( rc!=1 ){
      storeLastErrno(pFile, errno);
      ལོག SQLITE_IOERR;
    }
    གལ་སྲིད( fsync(fd) ){
      storeLastErrno(pFile, errno);
      ལོག SQLITE_IOERR_FSYNC;
    }
    rc = osFstat(fd, &statbuf);
    གལ་སྲིད( rc!=0 ){
      storeLastErrno(pFile, errno);
      ལོག SQLITE_IOERR;
    }
  }
#endif

  memset(&fileId, 0, parimaa_na(fileId));
  fileId.dev = statbuf.st_dev;
#གལ་སྲིད OS_VXWORKS
  fileId.pId = pFile->pId;
#དེ་མིན
  fileId.ino = (u64)statbuf.st_ino;
#endif
  assert( unixMutexHeld() );
  pInode = inodeList;
  སྐབས( pInode && memcmp(&fileId, &pInode->fileId, parimaa_na(fileId)) ){
    pInode = pInode->pNext;
  }
  གལ་སྲིད( pInode==0 ){
    pInode = sqlite3_malloc64( parimaa_na(*pInode) );
    གལ་སྲིད( pInode==0 ){
      ལོག SQLITE_NOMEM_BKPT;
    }
    memset(pInode, 0, parimaa_na(*pInode));
    memcpy(&pInode->fileId, &fileId, parimaa_na(fileId));
    གལ་སྲིད( sqlite3GlobalConfig.bCoreMutex ){
      pInode->pLockMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      གལ་སྲིད( pInode->pLockMutex==0 ){
        sqlite3_free(pInode);
        ལོག SQLITE_NOMEM_BKPT;
      }
    }
    pInode->nRef = 1;
    assert( unixMutexHeld() );
    pInode->pNext = inodeList;
    pInode->pPrev = 0;
    གལ་སྲིད( inodeList ) inodeList->pPrev = pInode;
    inodeList = pInode;
  }དེ་མིན{
    pInode->nRef++;
  }
  *ppInode = pInode;
  ལོག SQLITE_OK;
}

/*
** Return TRUE if pFile has been renamed or unlinked since it was first opened.
*/
static ཧྲིལ fileHasMoved(unixFile *pFile){
#གལ་སྲིད OS_VXWORKS
  ལོག pFile->pInode!=0 && pFile->pId!=pFile->pInode->fileId.pId;
#དེ་མིན
  sa_nracanaa stat buf;
  ལོག pFile->pInode!=0 &&
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
static void verifyDbFile(unixFile *pFile){
  sa_nracanaa stat buf;
  ཧྲིལ rc;

  /* These verifications occurs for the main database only */
  གལ་སྲིད( pFile->ctrlFlags & UNIXFILE_NOLOCK ) ལོག;

  rc = osFstat(pFile->h, &buf);
  གལ་སྲིད( rc!=0 ){
    sqlite3_log(SQLITE_WARNING, "cannot fstat db file %s", pFile->zPath);
    ལོག;
  }
  གལ་སྲིད( buf.st_nlink==0 ){
    sqlite3_log(SQLITE_WARNING, "file unlinked while open: %s", pFile->zPath);
    ལོག;
  }
  གལ་སྲིད( buf.st_nlink>1 ){
    sqlite3_log(SQLITE_WARNING, "multiple links to file: %s", pFile->zPath);
    ལོག;
  }
  གལ་སྲིད( fileHasMoved(pFile) ){
    sqlite3_log(SQLITE_WARNING, "file renamed while open: %s", pFile->zPath);
    ལོག;
  }
}


/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
static ཧྲིལ unixCheckReservedLock(sqlite3_file *id, ཧྲིལ *pResOut){
  ཧྲིལ rc = SQLITE_OK;
  ཧྲིལ reserved = 0;
  unixFile *pFile = (unixFile*)id;

  SimulateIOError( ལོག SQLITE_IOERR_CHECKRESERVEDLOCK; );

  assert( pFile );
  assert( pFile->eFileLock<=SHARED_LOCK );
  sqlite3_mutex_enter(pFile->pInode->pLockMutex);

  /* Check if a thread in this process holds such a lock */
  གལ་སྲིད( pFile->pInode->eFileLock>SHARED_LOCK ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it.
  */
#ifndef __DJGPP__
  གལ་སྲིད( !reserved && !pFile->pInode->bProcessLock ){
    sa_nracanaa flock lock;
    lock.l_whence = SEEK_SET;
    lock.l_start = RESERVED_BYTE;
    lock.l_len = 1;
    lock.l_type = F_WRLCK;
    གལ་སྲིད( osFcntl(pFile->h, F_GETLK, &lock) ){
      rc = SQLITE_IOERR_CHECKRESERVEDLOCK;
      storeLastErrno(pFile, errno);
    } དེ་མིན གལ་སྲིད( lock.l_type!=F_UNLCK ){
      reserved = 1;
    }
  }
#endif

  sqlite3_mutex_leave(pFile->pInode->pLockMutex);
  OSTRACE(("TEST WR-LOCK %d %d %d (unix)\n", pFile->h, rc, reserved));

  *pResOut = reserved;
  ལོག rc;
}

/* Forward declaration*/
static ཧྲིལ unixSleep(sqlite3_vfs*,ཧྲིལ);

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
#དེ་མིན
static ཧྲིལ osSetPosixAdvisoryLock(
  ཧྲིལ h,                /* The file descriptor on which to take the lock */
  sa_nracanaa flock *pLock,  /* The description of the lock */
  unixFile *pFile       /* Structure holding timeout value */
){
  ཧྲིལ rc = 0;

  གལ་སྲིད( pFile->iBusyTimeout==0 ){
    /* unixFile->iBusyTimeout is set to 0. In this case, attempt a 
    ** non-blocking lock. */
    rc = osFcntl(h,F_SETLK,pLock);
  }དེ་མིན{
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
    ཧྲིལ tm = pFile->iBusyTimeout;
    སྐབས( tm>0 ){
      rc = osFcntl(h,F_SETLK,pLock);
      གལ་སྲིད( rc==0 ) bha_nga;
      unixSleep(0,1000);
      tm--;
    }
#དེ་མིན
    rc = osFcntl(h,F_SETLK,pLock);
    pFile->iBusyTimeout = 0;
#endif
    /* End of code to replace with real blocking-locks code. */
  }

  ལོག rc;
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
static ཧྲིལ unixFileLock(unixFile *pFile, sa_nracanaa flock *pLock){
  ཧྲིལ rc;
  unixInodeInfo *pInode = pFile->pInode;
  assert( pInode!=0 );
  assert( sqlite3_mutex_held(pInode->pLockMutex) );
  གལ་སྲིད( (pFile->ctrlFlags & (UNIXFILE_EXCL|UNIXFILE_RDONLY))==UNIXFILE_EXCL ){
    གལ་སྲིད( pInode->bProcessLock==0 ){
      sa_nracanaa flock lock;
      /* assert( pInode->nLock==0 ); <-- Not true if unix-excl READONLY used */
      lock.l_whence = SEEK_SET;
      lock.l_start = SHARED_FIRST;
      lock.l_len = SHARED_SIZE;
      lock.l_type = F_WRLCK;
      rc = osSetPosixAdvisoryLock(pFile->h, &lock, pFile);
      གལ་སྲིད( rc<0 ) ལོག rc;
      pInode->bProcessLock = 1;
      pInode->nLock++;
    }དེ་མིན{
      rc = 0;
    }
  }དེ་མིན{
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    གལ་སྲིད( pFile->bBlockOnConnect && pLock->l_type==F_RDLCK
     && pLock->l_start==SHARED_FIRST && pLock->l_len==SHARED_SIZE
    ){
      rc = osFcntl(pFile->h, F_SETLKW, pLock);
    }དེ་མིན
#endif
    rc = osSetPosixAdvisoryLock(pFile->h, pLock, pFile);
  }
  ལོག rc;
}

#གལ་སྲིད !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
/* Forward reference */
static ཧྲིལ unixIsSharingShmNode(unixFile*);
#དེ་མིན
#define unixIsSharingShmNode(pFile) (0)
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
static ཧྲིལ unixLock(sqlite3_file *id, ཧྲིལ eFileLock){
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
  ཧྲིལ rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  sa_nracanaa flock lock;
  ཧྲིལ tErrno = 0;

  assert( pFile );
  OSTRACE(("LOCK    %d %s was %s(%s,%d) pid=%d (unix)\n", pFile->h,
      azFileLock(eFileLock), azFileLock(pFile->eFileLock),
      azFileLock(pFile->pInode->eFileLock), pFile->pInode->nShared,
      osGetpid(0)));

  /* If there is already a lock of this type or more restrictive on the
  ** unixFile, do nothing. Don't use the end_lock: exit path, as
  ** unixEnterMutex() hasn't been called yet.
  */
  གལ་སྲིད( pFile->eFileLock>=eFileLock ){
    OSTRACE(("LOCK    %d %s ok (already held) (unix)\n", pFile->h,
            azFileLock(eFileLock)));
    ལོག SQLITE_OK;
  }

  /* Make sure the locking sequence is correct.
  **  (1) We never move from unlocked to anything higher than shared lock.
  **  (2) SQLite never explicitly requests a pending lock.
  **  (3) A shared lock is always held when a reserve lock is requested.
  */
  assert( pFile->eFileLock!=NO_LOCK || eFileLock==SHARED_LOCK );
  assert( eFileLock!=PENDING_LOCK );
  assert( eFileLock!=RESERVED_LOCK || pFile->eFileLock==SHARED_LOCK );

  /* This mutex is needed because pFile->pInode is shared across threads
  */
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->pLockMutex);

  /* If some thread using this PID has a lock via a different unixFile*
  ** handle that precludes the requested lock, return BUSY.
  */
  གལ་སྲིད( (pFile->eFileLock!=pInode->eFileLock &&
          (pInode->eFileLock>=PENDING_LOCK || eFileLock>SHARED_LOCK))
  ){
    rc = SQLITE_BUSY;
    goto end_lock;
  }

  /* If a SHARED lock is requested, and some thread using this PID already
  ** has a SHARED or RESERVED lock, then increment reference counts and
  ** return SQLITE_OK.
  */
  གལ་སྲིད( eFileLock==SHARED_LOCK &&
      (pInode->eFileLock==SHARED_LOCK || pInode->eFileLock==RESERVED_LOCK) ){
    assert( eFileLock==SHARED_LOCK );
    assert( pFile->eFileLock==0 );
    assert( pInode->nShared>0 );
    pFile->eFileLock = SHARED_LOCK;
    pInode->nShared++;
    pInode->nLock++;
    goto end_lock;
  }


  /* A PENDING lock is needed before acquiring a SHARED lock and before
  ** acquiring an EXCLUSIVE lock.  For the SHARED lock, the PENDING will
  ** be released.
  */
  lock.l_len = 1L;
  lock.l_whence = SEEK_SET;
  གལ་སྲིད( eFileLock==SHARED_LOCK
   || (eFileLock==EXCLUSIVE_LOCK && pFile->eFileLock==RESERVED_LOCK)
  ){
    lock.l_type = (eFileLock==SHARED_LOCK?F_RDLCK:F_WRLCK);
    lock.l_start = PENDING_BYTE;
    གལ་སྲིད( unixFileLock(pFile, &lock) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_LOCK);
      གལ་སྲིད( rc!=SQLITE_BUSY ){
        storeLastErrno(pFile, tErrno);
      }
      goto end_lock;
    }དེ་མིན གལ་སྲིད( eFileLock==EXCLUSIVE_LOCK ){
      pFile->eFileLock = PENDING_LOCK;
      pInode->eFileLock = PENDING_LOCK;
    }
  }


  /* If control gets to this point, then actually go ahead and make
  ** operating system calls for the specified lock.
  */
  གལ་སྲིད( eFileLock==SHARED_LOCK ){
    assert( pInode->nShared==0 );
    assert( pInode->eFileLock==0 );
    assert( rc==SQLITE_OK );

    /* Now get the read-lock */
    lock.l_start = SHARED_FIRST;
    lock.l_len = SHARED_SIZE;
    གལ་སྲིད( unixFileLock(pFile, &lock) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_LOCK);
    }

    /* Drop the temporary PENDING lock */
    lock.l_start = PENDING_BYTE;
    lock.l_len = 1L;
    lock.l_type = F_UNLCK;
    གལ་སྲིད( unixFileLock(pFile, &lock) && rc==SQLITE_OK ){
      /* This could happen with a network mount */
      tErrno = errno;
      rc = SQLITE_IOERR_UNLOCK;
    }

    གལ་སྲིད( rc ){
      གལ་སྲིད( rc!=SQLITE_BUSY ){
        storeLastErrno(pFile, tErrno);
      }
      goto end_lock;
    }དེ་མིན{
      pFile->eFileLock = SHARED_LOCK;
      pInode->nLock++;
      pInode->nShared = 1;
    }
  }དེ་མིན གལ་སྲིད( eFileLock==EXCLUSIVE_LOCK && pInode->nShared>1 ){
    /* We are trying for an exclusive lock but another thread in this
    ** same process is still holding a shared lock. */
    rc = SQLITE_BUSY;
  }དེ་མིན གལ་སྲིད( unixIsSharingShmNode(pFile) ){
    /* We are in WAL mode and attempting to delete the SHM and WAL
    ** files due to closing the connection or changing out of WAL mode,
    ** but another process still holds locks on the SHM file, thus
    ** indicating that database locks have been broken, perhaps due
    ** to a rogue close(open(dbFile)) or similar.
    */
    rc = SQLITE_BUSY;
  }དེ་མིན{
    /* The request was for a RESERVED or EXCLUSIVE lock.  It is
    ** assumed that there is a SHARED or greater lock on the file
    ** already.
    */
    assert( 0!=pFile->eFileLock );
    lock.l_type = F_WRLCK;

    assert( eFileLock==RESERVED_LOCK || eFileLock==EXCLUSIVE_LOCK );
    གལ་སྲིད( eFileLock==RESERVED_LOCK ){
      lock.l_start = RESERVED_BYTE;
      lock.l_len = 1L;
    }དེ་མིན{
      lock.l_start = SHARED_FIRST;
      lock.l_len = SHARED_SIZE;
    }

    གལ་སྲིད( unixFileLock(pFile, &lock) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_LOCK);
      གལ་སྲིད( rc!=SQLITE_BUSY ){
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
  གལ་སྲིད( rc==SQLITE_OK
   && pFile->eFileLock<=SHARED_LOCK
   && eFileLock==RESERVED_LOCK
  ){
    pFile->transCntrChng = 0;
    pFile->dbUpdate = 0;
    pFile->inNormalWrite = 1;
  }
#endif

  གལ་སྲིད( rc==SQLITE_OK ){
    pFile->eFileLock = eFileLock;
    pInode->eFileLock = eFileLock;
  }

end_lock:
  sqlite3_mutex_leave(pInode->pLockMutex);
  OSTRACE(("LOCK    %d %s %s (unix)\n", pFile->h, azFileLock(eFileLock),
      rc==SQLITE_OK ? "ok" : "failed"));
  ལོག rc;
}

/*
** Add the file descriptor used by file handle pFile to the corresponding
** pUnused list.
*/
static void setPendingFd(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  UnixUnusedFd *p = pFile->pPreallocatedUnused;
  assert( unixFileMutexHeld(pFile) );
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
static ཧྲིལ posixUnlock(sqlite3_file *id, ཧྲིལ eFileLock, ཧྲིལ handleNFSUnlock){
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  sa_nracanaa flock lock;
  ཧྲིལ rc = SQLITE_OK;

  assert( pFile );
  OSTRACE(("UNLOCK  %d %d was %d(%d,%d) pid=%d (unix)\n", pFile->h, eFileLock,
      pFile->eFileLock, pFile->pInode->eFileLock, pFile->pInode->nShared,
      osGetpid(0)));

  assert( eFileLock<=SHARED_LOCK );
  གལ་སྲིད( pFile->eFileLock<=eFileLock ){
    ལོག SQLITE_OK;
  }
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->pLockMutex);
  assert( pInode->nShared!=0 );
  གལ་སྲིད( pFile->eFileLock>SHARED_LOCK ){
    assert( pInode->eFileLock==pFile->eFileLock );

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
    གལ་སྲིད( eFileLock==SHARED_LOCK ){
#གལ་སྲིད !defined(__APPLE__) || !SQLITE_ENABLE_LOCKING_STYLE
      (void)handleNFSUnlock;
      assert( handleNFSUnlock==0 );
#endif
#གལ་སྲིད defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
      གལ་སྲིད( handleNFSUnlock ){
        ཧྲིལ tErrno;               /* Error code from system call errors */
        off_t divSize = SHARED_SIZE - 1;
      
        lock.l_type = F_UNLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = SHARED_FIRST;
        lock.l_len = divSize;
        གལ་སྲིད( unixFileLock(pFile, &lock)==(-1) ){
          tErrno = errno;
          rc = SQLITE_IOERR_UNLOCK;
          storeLastErrno(pFile, tErrno);
          goto end_unlock;
        }
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = SHARED_FIRST;
        lock.l_len = divSize;
        གལ་སྲིད( unixFileLock(pFile, &lock)==(-1) ){
          tErrno = errno;
          rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_RDLOCK);
          གལ་སྲིད( IS_LOCK_ERROR(rc) ){
            storeLastErrno(pFile, tErrno);
          }
          goto end_unlock;
        }
        lock.l_type = F_UNLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = SHARED_FIRST+divSize;
        lock.l_len = SHARED_SIZE-divSize;
        གལ་སྲིད( unixFileLock(pFile, &lock)==(-1) ){
          tErrno = errno;
          rc = SQLITE_IOERR_UNLOCK;
          storeLastErrno(pFile, tErrno);
          goto end_unlock;
        }
      }དེ་མིན
#endif /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
      {
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = SHARED_FIRST;
        lock.l_len = SHARED_SIZE;
        གལ་སྲིད( unixFileLock(pFile, &lock) ){
          /* In theory, the call to unixFileLock() cannot fail because another
          ** process is holding an incompatible lock. If it does, this
          ** indicates that the other process is not following the locking
          ** protocol. If this happens, return SQLITE_IOERR_RDLOCK. Returning
          ** SQLITE_BUSY would confuse the upper layer (in practice it causes
          ** an assert to fail). */
          rc = SQLITE_IOERR_RDLOCK;
          storeLastErrno(pFile, errno);
          goto end_unlock;
        }
      }
    }
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = PENDING_BYTE;
    lock.l_len = 2L;  assert( PENDING_BYTE+1==RESERVED_BYTE );
    གལ་སྲིད( unixFileLock(pFile, &lock)==0 ){
      pInode->eFileLock = SHARED_LOCK;
    }དེ་མིན{
      rc = SQLITE_IOERR_UNLOCK;
      storeLastErrno(pFile, errno);
      goto end_unlock;
    }
  }
  གལ་སྲིད( eFileLock==NO_LOCK ){
    /* Decrement the shared lock counter.  Release the lock using an
    ** OS call only when all threads in this same process have released
    ** the lock.
    */
    pInode->nShared--;
    གལ་སྲིད( pInode->nShared==0 ){
      lock.l_type = F_UNLCK;
      lock.l_whence = SEEK_SET;
      lock.l_start = lock.l_len = 0L;
      གལ་སྲིད( unixFileLock(pFile, &lock)==0 ){
        pInode->eFileLock = NO_LOCK;
      }དེ་མིན{
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
    assert( pInode->nLock>=0 );
    གལ་སྲིད( pInode->nLock==0 ) closePendingFds(pFile);
  }

end_unlock:
  sqlite3_mutex_leave(pInode->pLockMutex);
  གལ་སྲིད( rc==SQLITE_OK ){
    pFile->eFileLock = eFileLock;
  }
  ལོག rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
static ཧྲིལ unixUnlock(sqlite3_file *id, ཧྲིལ eFileLock){
#གལ་སྲིད SQLITE_MAX_MMAP_SIZE>0
  assert( eFileLock==SHARED_LOCK || ((unixFile *)id)->nFetchOut==0 );
#endif
  ལོག posixUnlock(id, eFileLock, 0);
}

#གལ་སྲིད SQLITE_MAX_MMAP_SIZE>0
static ཧྲིལ unixMapfile(unixFile *pFd, i64 nByte);
static void unixUnmapfile(unixFile *pFd);
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
static ཧྲིལ closeUnixFile(sqlite3_file *id){
  unixFile *pFile = (unixFile*)id;
#གལ་སྲིད SQLITE_MAX_MMAP_SIZE>0
  unixUnmapfile(pFile);
#endif
  གལ་སྲིད( pFile->h>=0 ){
    robust_close(pFile, pFile->h, __LINE__);
    pFile->h = -1;
  }
#གལ་སྲིད OS_VXWORKS
  གལ་སྲིད( pFile->pId ){
    གལ་སྲིད( pFile->ctrlFlags & UNIXFILE_DELETE ){
      osUnlink(pFile->pId->zCanonicalName);
    }
    vxworksReleaseFileId(pFile->pId);
    pFile->pId = 0;
  }
#endif
#ifdef SQLITE_UNLINK_AFTER_CLOSE
  གལ་སྲིད( pFile->ctrlFlags & UNIXFILE_DELETE ){
    osUnlink(pFile->zPath);
    sqlite3_free(*(var_na**)&pFile->zPath);
    pFile->zPath = 0;
  }
#endif
  OSTRACE(("CLOSE   %-3d\n", pFile->h));
  OpenCounter(-1);
  sqlite3_free(pFile->pPreallocatedUnused);
  memset(pFile, 0, parimaa_na(unixFile));
  ལོག SQLITE_OK;
}

/*
** Close a file.
*/
static ཧྲིལ unixClose(sqlite3_file *id){
  ཧྲིལ rc = SQLITE_OK;
  unixFile *pFile = (unixFile *)id;
  unixInodeInfo *pInode = pFile->pInode;

  assert( pInode!=0 );
  verifyDbFile(pFile);
  unixUnlock(id, NO_LOCK);
  assert( unixFileMutexNotheld(pFile) );
  unixEnterMutex();

  /* unixFile.pInode is always valid here. Otherwise, a different close
  ** routine (e.g. nolockClose()) would be called instead.
  */
  assert( pFile->pInode->nLock>0 || pFile->pInode->bProcessLock==0 );
  sqlite3_mutex_enter(pInode->pLockMutex);
  གལ་སྲིད( pInode->nLock ){
    /* If there are outstanding locks, do not actually close the file just
    ** yet because that would clear those locks.  Instead, add the file
    ** descriptor to pInode->pUnused list.  It will be automatically closed
    ** when the last lock is cleared.
    */
    setPendingFd(pFile);
  }
  sqlite3_mutex_leave(pInode->pLockMutex);
  releaseInodeInfo(pFile);
  assert( pFile->pShm==0 );
  rc = closeUnixFile(id);
  unixLeaveMutex();
  ལོག rc;
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

static ཧྲིལ nolockCheckReservedLock(sqlite3_file *NotUsed, ཧྲིལ *pResOut){
  UNUSED_PARAMETER(NotUsed);
  *pResOut = 0;
  ལོག SQLITE_OK;
}
static ཧྲིལ nolockLock(sqlite3_file *NotUsed, ཧྲིལ NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ལོག SQLITE_OK;
}
static ཧྲིལ nolockUnlock(sqlite3_file *NotUsed, ཧྲིལ NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ལོག SQLITE_OK;
}

/*
** Close the file.
*/
static ཧྲིལ nolockClose(sqlite3_file *id) {
  ལོག closeUnixFile(id);
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
static ཧྲིལ dotlockCheckReservedLock(sqlite3_file *id, ཧྲིལ *pResOut) {
  unixFile *pFile = (unixFile*)id;
  SimulateIOError( ལོག SQLITE_IOERR_CHECKRESERVEDLOCK; );

  གལ་སྲིད( pFile->eFileLock>=SHARED_LOCK ){
    *pResOut = 0;
  }དེ་མིན{
    *pResOut = osAccess((const var_na*)pFile->lockingContext, 0)==0;
  }
  OSTRACE(("TEST WR-LOCK %d %d %d (dotlock)\n", pFile->h, 0, *pResOut));
  ལོག SQLITE_OK;
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
static ཧྲིལ dotlockLock(sqlite3_file *id, ཧྲིལ eFileLock) {
  unixFile *pFile = (unixFile*)id;
  var_na *zLockFile = (var_na *)pFile->lockingContext;
  ཧྲིལ rc = SQLITE_OK;


  /* If we have any lock, then the lock file already exists.  All we have
  ** to do is adjust our internal record of the lock level.
  */
  གལ་སྲིད( pFile->eFileLock > NO_LOCK ){
    pFile->eFileLock = eFileLock;
    /* Always update the timestamp on the old file */
#ifdef HAVE_UTIME
    utime(zLockFile, NULL);
#དེ་མིན
    utimes(zLockFile, NULL);
#endif
    ལོག SQLITE_OK;
  }

  /* grab an exclusive lock */
  rc = osMkdir(zLockFile, 0777);
  གལ་སྲིད( rc<0 ){
    /* failed to open/create the lock directory */
    ཧྲིལ tErrno = errno;
    གལ་སྲིད( EEXIST == tErrno ){
      rc = SQLITE_BUSY;
    } དེ་མིན {
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_LOCK);
      གལ་སྲིད( rc!=SQLITE_BUSY ){
        storeLastErrno(pFile, tErrno);
      }
    }
    ལོག rc;
  }

  /* got it, set the type and return ok */
  pFile->eFileLock = eFileLock;
  ལོག rc;
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
static ཧྲིལ dotlockUnlock(sqlite3_file *id, ཧྲིལ eFileLock) {
  unixFile *pFile = (unixFile*)id;
  var_na *zLockFile = (var_na *)pFile->lockingContext;
  ཧྲིལ rc;

  assert( pFile );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (dotlock)\n", pFile->h, eFileLock,
           pFile->eFileLock, osGetpid(0)));
  assert( eFileLock<=SHARED_LOCK );

  /* no-op if possible */
  གལ་སྲིད( pFile->eFileLock==eFileLock ){
    ལོག SQLITE_OK;
  }

  /* To downgrade to shared, simply update our internal notion of the
  ** lock state.  No need to mess with the file on disk.
  */
  གལ་སྲིད( eFileLock==SHARED_LOCK ){
    pFile->eFileLock = SHARED_LOCK;
    ལོག SQLITE_OK;
  }

  /* To fully unlock the database, delete the lock file */
  assert( eFileLock==NO_LOCK );
  rc = osRmdir(zLockFile);
  གལ་སྲིད( rc<0 ){
    ཧྲིལ tErrno = errno;
    གལ་སྲིད( tErrno==ENOENT ){
      rc = SQLITE_OK;
    }དེ་མིན{
      rc = SQLITE_IOERR_UNLOCK;
      storeLastErrno(pFile, tErrno);
    }
    ལོག rc;
  }
  pFile->eFileLock = NO_LOCK;
  ལོག SQLITE_OK;
}

/*
** Close a file.  Make sure the lock has been released before closing.
*/
static ཧྲིལ dotlockClose(sqlite3_file *id) {
  unixFile *pFile = (unixFile*)id;
  assert( id!=0 );
  dotlockUnlock(id, NO_LOCK);
  sqlite3_free(pFile->lockingContext);
  ལོག closeUnixFile(id);
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
#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE

/*
** Retry flock() calls that fail with EINTR
*/
#ifdef EINTR
static ཧྲིལ robust_flock(ཧྲིལ fd, ཧྲིལ op){
  ཧྲིལ rc;
  do{ rc = flock(fd,op); }སྐབས( rc<0 && errno==EINTR );
  ལོག rc;
}
#དེ་མིན
# define robust_flock(a,b) flock(a,b)
#endif
   

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
static ཧྲིལ flockCheckReservedLock(sqlite3_file *id, ཧྲིལ *pResOut){
#ifdef SQLITE_DEBUG
  unixFile *pFile = (unixFile*)id;
#དེ་མིན
  UNUSED_PARAMETER(id);
#endif

  SimulateIOError( ལོག SQLITE_IOERR_CHECKRESERVEDLOCK; );

  assert( pFile );
  assert( pFile->eFileLock<=SHARED_LOCK );

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

  ལོག SQLITE_OK;
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
static ཧྲིལ flockLock(sqlite3_file *id, ཧྲིལ eFileLock) {
  ཧྲིལ rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;

  assert( pFile );

  /* if we already have a lock, it is exclusive.
  ** Just adjust level and punt on outta here. */
  གལ་སྲིད (pFile->eFileLock > NO_LOCK) {
    pFile->eFileLock = eFileLock;
    ལོག SQLITE_OK;
  }

  /* grab an exclusive lock */

  གལ་སྲིད (robust_flock(pFile->h, LOCK_EX | LOCK_NB)) {
    ཧྲིལ tErrno = errno;
    /* didn't get, must be busy */
    rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_LOCK);
    གལ་སྲིད( IS_LOCK_ERROR(rc) ){
      storeLastErrno(pFile, tErrno);
    }
  } དེ་མིན {
    /* got it, set the type and return ok */
    pFile->eFileLock = eFileLock;
  }
  OSTRACE(("LOCK    %d %s %s (flock)\n", pFile->h, azFileLock(eFileLock),
           rc==SQLITE_OK ? "ok" : "failed"));
#ifdef SQLITE_IGNORE_FLOCK_LOCK_ERRORS
  གལ་སྲིད( (rc & 0xff) == SQLITE_IOERR ){
    rc = SQLITE_BUSY;
  }
#endif /* SQLITE_IGNORE_FLOCK_LOCK_ERRORS */
  ལོག rc;
}


/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
static ཧྲིལ flockUnlock(sqlite3_file *id, ཧྲིལ eFileLock) {
  unixFile *pFile = (unixFile*)id;

  assert( pFile );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (flock)\n", pFile->h, eFileLock,
           pFile->eFileLock, osGetpid(0)));
  assert( eFileLock<=SHARED_LOCK );

  /* no-op if possible */
  གལ་སྲིད( pFile->eFileLock==eFileLock ){
    ལོག SQLITE_OK;
  }

  /* shared can just be set because we always have an exclusive */
  གལ་སྲིད (eFileLock==SHARED_LOCK) {
    pFile->eFileLock = eFileLock;
    ལོག SQLITE_OK;
  }

  /* no, really, unlock. */
  གལ་སྲིད( robust_flock(pFile->h, LOCK_UN) ){
#ifdef SQLITE_IGNORE_FLOCK_LOCK_ERRORS
    ལོག SQLITE_OK;
#endif /* SQLITE_IGNORE_FLOCK_LOCK_ERRORS */
    ལོག SQLITE_IOERR_UNLOCK;
  }དེ་མིན{
    pFile->eFileLock = NO_LOCK;
    ལོག SQLITE_OK;
  }
}

/*
** Close a file.
*/
static ཧྲིལ flockClose(sqlite3_file *id) {
  assert( id!=0 );
  flockUnlock(id, NO_LOCK);
  ལོག closeUnixFile(id);
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
#གལ་སྲིད OS_VXWORKS

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
static ཧྲིལ semXCheckReservedLock(sqlite3_file *id, ཧྲིལ *pResOut) {
  ཧྲིལ rc = SQLITE_OK;
  ཧྲིལ reserved = 0;
  unixFile *pFile = (unixFile*)id;

  SimulateIOError( ལོག SQLITE_IOERR_CHECKRESERVEDLOCK; );

  assert( pFile );

  /* Check if a thread in this process holds such a lock */
  གལ་སྲིད( pFile->eFileLock>SHARED_LOCK ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it. */
  གལ་སྲིད( !reserved ){
    sem_t *pSem = pFile->pInode->pSem;

    གལ་སྲིད( sem_trywait(pSem)==-1 ){
      ཧྲིལ tErrno = errno;
      གལ་སྲིད( EAGAIN != tErrno ){
        rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_CHECKRESERVEDLOCK);
        storeLastErrno(pFile, tErrno);
      } དེ་མིན {
        /* someone else has the lock when we are in NO_LOCK */
        reserved = (pFile->eFileLock < SHARED_LOCK);
      }
    }དེ་མིན{
      /* we could have it if we want it */
      sem_post(pSem);
    }
  }
  OSTRACE(("TEST WR-LOCK %d %d %d (sem)\n", pFile->h, rc, reserved));

  *pResOut = reserved;
  ལོག rc;
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
static ཧྲིལ semXLock(sqlite3_file *id, ཧྲིལ eFileLock) {
  unixFile *pFile = (unixFile*)id;
  sem_t *pSem = pFile->pInode->pSem;
  ཧྲིལ rc = SQLITE_OK;

  /* if we already have a lock, it is exclusive.
  ** Just adjust level and punt on outta here. */
  གལ་སྲིད (pFile->eFileLock > NO_LOCK) {
    pFile->eFileLock = eFileLock;
    rc = SQLITE_OK;
    goto sem_end_lock;
  }

  /* lock semaphore now but bail out when already locked. */
  གལ་སྲིད( sem_trywait(pSem)==-1 ){
    rc = SQLITE_BUSY;
    goto sem_end_lock;
  }

  /* got it, set the type and return ok */
  pFile->eFileLock = eFileLock;

 sem_end_lock:
  ལོག rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
static ཧྲིལ semXUnlock(sqlite3_file *id, ཧྲིལ eFileLock) {
  unixFile *pFile = (unixFile*)id;
  sem_t *pSem = pFile->pInode->pSem;

  assert( pFile );
  assert( pSem );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (sem)\n", pFile->h, eFileLock,
           pFile->eFileLock, osGetpid(0)));
  assert( eFileLock<=SHARED_LOCK );

  /* no-op if possible */
  གལ་སྲིད( pFile->eFileLock==eFileLock ){
    ལོག SQLITE_OK;
  }

  /* shared can just be set because we always have an exclusive */
  གལ་སྲིད (eFileLock==SHARED_LOCK) {
    pFile->eFileLock = eFileLock;
    ལོག SQLITE_OK;
  }

  /* no, really unlock. */
  གལ་སྲིད ( sem_post(pSem)==-1 ) {
    ཧྲིལ rc, tErrno = errno;
    rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_UNLOCK);
    གལ་སྲིད( IS_LOCK_ERROR(rc) ){
      storeLastErrno(pFile, tErrno);
    }
    ལོག rc;
  }
  pFile->eFileLock = NO_LOCK;
  ལོག SQLITE_OK;
}

/*
 ** Close a file.
 */
static ཧྲིལ semXClose(sqlite3_file *id) {
  གལ་སྲིད( id ){
    unixFile *pFile = (unixFile*)id;
    semXUnlock(id, NO_LOCK);
    assert( pFile );
    assert( unixFileMutexNotheld(pFile) );
    unixEnterMutex();
    releaseInodeInfo(pFile);
    unixLeaveMutex();
    closeUnixFile(id);
  }
  ལོག SQLITE_OK;
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

#གལ་སྲིད defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
** The afpLockingContext structure contains all afp lock specific state
*/
typedef sa_nracanaa afpLockingContext afpLockingContext;
sa_nracanaa afpLockingContext {
  ཧྲིལ reserved;
  const var_na *dbPath;             /* Name of the open file */
};

sa_nracanaa ByteRangeLockPB2
{
  unsigned long long offset;        /* offset to first byte to lock */
  unsigned long long length;        /* nbr of bytes to lock */
  unsigned long long retRangeStart; /* nbr of 1st byte locked if successful */
  unsigned var_na unLockFlag;         /* 1 = unlock, 0 = lock */
  unsigned var_na startEndFlag;       /* 1=rel to end of fork, 0=rel to start */
  ཧྲིལ fd;                           /* file desc to assoc this lock with */
};

#define afpfsByteRangeLock2FSCTL        _IOWR('z', 23, sa_nracanaa ByteRangeLockPB2)

/*
** This is a utility for setting or clearing a bit-range lock on an
** AFP filesystem.
**
** Return SQLITE_OK on success, SQLITE_BUSY on failure.
*/
static ཧྲིལ afpSetLock(
  const var_na *path,              /* Name of the file to be locked or unlocked */
  unixFile *pFile,               /* Open file descriptor on path */
  unsigned long long offset,     /* First byte to be locked */
  unsigned long long length,     /* Number of bytes to lock */
  ཧྲིལ setLockFlag                /* True to set lock.  False to clear lock */
){
  sa_nracanaa ByteRangeLockPB2 pb;
  ཧྲིལ err;

  pb.unLockFlag = setLockFlag ? 0 : 1;
  pb.startEndFlag = 0;
  pb.offset = offset;
  pb.length = length;
  pb.fd = pFile->h;

  OSTRACE(("AFPSETLOCK [%s] for %d%s in range %llx:%llx\n",
    (setLockFlag?"ON":"OFF"), pFile->h, (pb.fd==-1?"[testval-1]":""),
    offset, length));
  err = fsctl(path, afpfsByteRangeLock2FSCTL, &pb, 0);
  གལ་སྲིད ( err==-1 ) {
    ཧྲིལ rc;
    ཧྲིལ tErrno = errno;
    OSTRACE(("AFPSETLOCK failed to fsctl() '%s' %d %s\n",
             path, tErrno, strerror(tErrno)));
#ifdef SQLITE_IGNORE_AFP_LOCK_ERRORS
    rc = SQLITE_BUSY;
#དེ་མིན
    rc = sqliteErrorFromPosixError(tErrno,
                    setLockFlag ? SQLITE_IOERR_LOCK : SQLITE_IOERR_UNLOCK);
#endif /* SQLITE_IGNORE_AFP_LOCK_ERRORS */
    གལ་སྲིད( IS_LOCK_ERROR(rc) ){
      storeLastErrno(pFile, tErrno);
    }
    ལོག rc;
  } དེ་མིན {
    ལོག SQLITE_OK;
  }
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
static ཧྲིལ afpCheckReservedLock(sqlite3_file *id, ཧྲིལ *pResOut){
  ཧྲིལ rc = SQLITE_OK;
  ཧྲིལ reserved = 0;
  unixFile *pFile = (unixFile*)id;
  afpLockingContext *context;

  SimulateIOError( ལོག SQLITE_IOERR_CHECKRESERVEDLOCK; );

  assert( pFile );
  context = (afpLockingContext *) pFile->lockingContext;
  གལ་སྲིད( context->reserved ){
    *pResOut = 1;
    ལོག SQLITE_OK;
  }
  sqlite3_mutex_enter(pFile->pInode->pLockMutex);
  /* Check if a thread in this process holds such a lock */
  གལ་སྲིད( pFile->pInode->eFileLock>SHARED_LOCK ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it.
   */
  གལ་སྲིད( !reserved ){
    /* lock the RESERVED byte */
    ཧྲིལ lrc = afpSetLock(context->dbPath, pFile, RESERVED_BYTE, 1,1);
    གལ་སྲིད( SQLITE_OK==lrc ){
      /* if we succeeded in taking the reserved lock, unlock it to restore
      ** the original state */
      lrc = afpSetLock(context->dbPath, pFile, RESERVED_BYTE, 1, 0);
    } དེ་མིན {
      /* if we failed to get the lock then someone else must have it */
      reserved = 1;
    }
    གལ་སྲིད( IS_LOCK_ERROR(lrc) ){
      rc=lrc;
    }
  }

  sqlite3_mutex_leave(pFile->pInode->pLockMutex);
  OSTRACE(("TEST WR-LOCK %d %d %d (afp)\n", pFile->h, rc, reserved));

  *pResOut = reserved;
  ལོག rc;
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
static ཧྲིལ afpLock(sqlite3_file *id, ཧྲིལ eFileLock){
  ཧྲིལ rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode = pFile->pInode;
  afpLockingContext *context = (afpLockingContext *) pFile->lockingContext;

  assert( pFile );
  OSTRACE(("LOCK    %d %s was %s(%s,%d) pid=%d (afp)\n", pFile->h,
           azFileLock(eFileLock), azFileLock(pFile->eFileLock),
           azFileLock(pInode->eFileLock), pInode->nShared , osGetpid(0)));

  /* If there is already a lock of this type or more restrictive on the
  ** unixFile, do nothing. Don't use the afp_end_lock: exit path, as
  ** unixEnterMutex() hasn't been called yet.
  */
  གལ་སྲིད( pFile->eFileLock>=eFileLock ){
    OSTRACE(("LOCK    %d %s ok (already held) (afp)\n", pFile->h,
           azFileLock(eFileLock)));
    ལོག SQLITE_OK;
  }

  /* Make sure the locking sequence is correct
  **  (1) We never move from unlocked to anything higher than shared lock.
  **  (2) SQLite never explicitly requests a pending lock.
  **  (3) A shared lock is always held when a reserve lock is requested.
  */
  assert( pFile->eFileLock!=NO_LOCK || eFileLock==SHARED_LOCK );
  assert( eFileLock!=PENDING_LOCK );
  assert( eFileLock!=RESERVED_LOCK || pFile->eFileLock==SHARED_LOCK );

  /* This mutex is needed because pFile->pInode is shared across threads
  */
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->pLockMutex);

  /* If some thread using this PID has a lock via a different unixFile*
  ** handle that precludes the requested lock, return BUSY.
  */
  གལ་སྲིད( (pFile->eFileLock!=pInode->eFileLock &&
       (pInode->eFileLock>=PENDING_LOCK || eFileLock>SHARED_LOCK))
     ){
    rc = SQLITE_BUSY;
    goto afp_end_lock;
  }

  /* If a SHARED lock is requested, and some thread using this PID already
  ** has a SHARED or RESERVED lock, then increment reference counts and
  ** return SQLITE_OK.
  */
  གལ་སྲིད( eFileLock==SHARED_LOCK &&
     (pInode->eFileLock==SHARED_LOCK || pInode->eFileLock==RESERVED_LOCK) ){
    assert( eFileLock==SHARED_LOCK );
    assert( pFile->eFileLock==0 );
    assert( pInode->nShared>0 );
    pFile->eFileLock = SHARED_LOCK;
    pInode->nShared++;
    pInode->nLock++;
    goto afp_end_lock;
  }
  
  /* A PENDING lock is needed before acquiring a SHARED lock and before
  ** acquiring an EXCLUSIVE lock.  For the SHARED lock, the PENDING will
  ** be released.
  */
  གལ་སྲིད( eFileLock==SHARED_LOCK
      || (eFileLock==EXCLUSIVE_LOCK && pFile->eFileLock<PENDING_LOCK)
  ){
    ཧྲིལ failed;
    failed = afpSetLock(context->dbPath, pFile, PENDING_BYTE, 1, 1);
    གལ་སྲིད (failed) {
      rc = failed;
      goto afp_end_lock;
    }
  }

  /* If control gets to this point, then actually go ahead and make
  ** operating system calls for the specified lock.
  */
  གལ་སྲིད( eFileLock==SHARED_LOCK ){
    ཧྲིལ lrc1, lrc2, lrc1Errno = 0;
    long lk, mask;
  
    assert( pInode->nShared==0 );
    assert( pInode->eFileLock==0 );
      
    mask = (parimaa_na(long)==8) ? LARGEST_INT64 : 0x7fffffff;
    /* Now get the read-lock SHARED_LOCK */
    /* note that the quality of the randomness doesn't matter that much */
    lk = random();
    pInode->sharedByte = (lk & mask)%(SHARED_SIZE - 1);
    lrc1 = afpSetLock(context->dbPath, pFile,
          SHARED_FIRST+pInode->sharedByte, 1, 1);
    གལ་སྲིད( IS_LOCK_ERROR(lrc1) ){
      lrc1Errno = pFile->lastErrno;
    }
    /* Drop the temporary PENDING lock */
    lrc2 = afpSetLock(context->dbPath, pFile, PENDING_BYTE, 1, 0);
  
    གལ་སྲིད( IS_LOCK_ERROR(lrc1) ) {
      storeLastErrno(pFile, lrc1Errno);
      rc = lrc1;
      goto afp_end_lock;
    } དེ་མིན གལ་སྲིད( IS_LOCK_ERROR(lrc2) ){
      rc = lrc2;
      goto afp_end_lock;
    } དེ་མིན གལ་སྲིད( lrc1 != SQLITE_OK ) {
      rc = lrc1;
    } དེ་མིན {
      pFile->eFileLock = SHARED_LOCK;
      pInode->nLock++;
      pInode->nShared = 1;
    }
  }དེ་མིན གལ་སྲིད( eFileLock==EXCLUSIVE_LOCK && pInode->nShared>1 ){
    /* We are trying for an exclusive lock but another thread in this
     ** same process is still holding a shared lock. */
    rc = SQLITE_BUSY;
  }དེ་མིན{
    /* The request was for a RESERVED or EXCLUSIVE lock.  It is
    ** assumed that there is a SHARED or greater lock on the file
    ** already.
    */
    ཧྲིལ failed = 0;
    assert( 0!=pFile->eFileLock );
    གལ་སྲིད (eFileLock >= RESERVED_LOCK && pFile->eFileLock < RESERVED_LOCK) {
        /* Acquire a RESERVED lock */
        failed = afpSetLock(context->dbPath, pFile, RESERVED_BYTE, 1,1);
      གལ་སྲིད( !failed ){
        context->reserved = 1;
      }
    }
    གལ་སྲིད (!failed && eFileLock == EXCLUSIVE_LOCK) {
      /* Acquire an EXCLUSIVE lock */

      /* Remove the shared lock before trying the range.  we'll need to
      ** reestablish the shared lock if we can't get the  afpUnlock
      */
      གལ་སྲིད( !(failed = afpSetLock(context->dbPath, pFile, SHARED_FIRST +
                         pInode->sharedByte, 1, 0)) ){
        ཧྲིལ failed2 = SQLITE_OK;
        /* now attempt to get the exclusive lock range */
        failed = afpSetLock(context->dbPath, pFile, SHARED_FIRST,
                               SHARED_SIZE, 1);
        གལ་སྲིད( failed && (failed2 = afpSetLock(context->dbPath, pFile,
                       SHARED_FIRST + pInode->sharedByte, 1, 1)) ){
          /* Can't reestablish the shared lock.  Sqlite can't deal, this is
          ** a critical I/O error
          */
          rc = ((failed & 0xff) == SQLITE_IOERR) ? failed2 :
               SQLITE_IOERR_LOCK;
          goto afp_end_lock;
        }
      }དེ་མིན{
        rc = failed;
      }
    }
    གལ་སྲིད( failed ){
      rc = failed;
    }
  }

  གལ་སྲིད( rc==SQLITE_OK ){
    pFile->eFileLock = eFileLock;
    pInode->eFileLock = eFileLock;
  }དེ་མིན གལ་སྲིད( eFileLock==EXCLUSIVE_LOCK ){
    pFile->eFileLock = PENDING_LOCK;
    pInode->eFileLock = PENDING_LOCK;
  }

afp_end_lock:
  sqlite3_mutex_leave(pInode->pLockMutex);
  OSTRACE(("LOCK    %d %s %s (afp)\n", pFile->h, azFileLock(eFileLock),
         rc==SQLITE_OK ? "ok" : "failed"));
  ལོག rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
static ཧྲིལ afpUnlock(sqlite3_file *id, ཧྲིལ eFileLock) {
  ཧྲིལ rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  afpLockingContext *context = (afpLockingContext *) pFile->lockingContext;
  ཧྲིལ skipShared = 0;

  assert( pFile );
  OSTRACE(("UNLOCK  %d %d was %d(%d,%d) pid=%d (afp)\n", pFile->h, eFileLock,
           pFile->eFileLock, pFile->pInode->eFileLock, pFile->pInode->nShared,
           osGetpid(0)));

  assert( eFileLock<=SHARED_LOCK );
  གལ་སྲིད( pFile->eFileLock<=eFileLock ){
    ལོག SQLITE_OK;
  }
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->pLockMutex);
  assert( pInode->nShared!=0 );
  གལ་སྲིད( pFile->eFileLock>SHARED_LOCK ){
    assert( pInode->eFileLock==pFile->eFileLock );
  
#ifdef SQLITE_DEBUG
    /* When reducing a lock such that other processes can start
    ** reading the database file again, make sure that the
    ** transaction counter was updated if any part of the database
    ** file changed.  If the transaction counter is not updated,
    ** other connections to the same file might not realize that
    ** the file has changed and hence might not know to flush their
    ** cache.  The use of a stale cache can lead to database corruption.
    */
    assert( pFile->inNormalWrite==0
           || pFile->dbUpdate==0
           || pFile->transCntrChng==1 );
    pFile->inNormalWrite = 0;
#endif
  
    གལ་སྲིད( pFile->eFileLock==EXCLUSIVE_LOCK ){
      rc = afpSetLock(context->dbPath, pFile, SHARED_FIRST, SHARED_SIZE, 0);
      གལ་སྲིད( rc==SQLITE_OK && (eFileLock==SHARED_LOCK || pInode->nShared>1) ){
        /* only re-establish the shared lock if necessary */
        ཧྲིལ sharedLockByte = SHARED_FIRST+pInode->sharedByte;
        rc = afpSetLock(context->dbPath, pFile, sharedLockByte, 1, 1);
      } དེ་མིན {
        skipShared = 1;
      }
    }
    གལ་སྲིད( rc==SQLITE_OK && pFile->eFileLock>=PENDING_LOCK ){
      rc = afpSetLock(context->dbPath, pFile, PENDING_BYTE, 1, 0);
    }
    གལ་སྲིད( rc==SQLITE_OK && pFile->eFileLock>=RESERVED_LOCK && context->reserved ){
      rc = afpSetLock(context->dbPath, pFile, RESERVED_BYTE, 1, 0);
      གལ་སྲིད( !rc ){
        context->reserved = 0;
      }
    }
    གལ་སྲིད( rc==SQLITE_OK && (eFileLock==SHARED_LOCK || pInode->nShared>1)){
      pInode->eFileLock = SHARED_LOCK;
    }
  }
  གལ་སྲིད( rc==SQLITE_OK && eFileLock==NO_LOCK ){

    /* Decrement the shared lock counter.  Release the lock using an
    ** OS call only when all threads in this same process have released
    ** the lock.
    */
    unsigned long long sharedLockByte = SHARED_FIRST+pInode->sharedByte;
    pInode->nShared--;
    གལ་སྲིད( pInode->nShared==0 ){
      གལ་སྲིད( !skipShared ){
        rc = afpSetLock(context->dbPath, pFile, sharedLockByte, 1, 0);
      }
      གལ་སྲིད( !rc ){
        pInode->eFileLock = NO_LOCK;
        pFile->eFileLock = NO_LOCK;
      }
    }
    གལ་སྲིད( rc==SQLITE_OK ){
      pInode->nLock--;
      assert( pInode->nLock>=0 );
      གལ་སྲིད( pInode->nLock==0 ) closePendingFds(pFile);
    }
  }

  sqlite3_mutex_leave(pInode->pLockMutex);
  གལ་སྲིད( rc==SQLITE_OK ){
    pFile->eFileLock = eFileLock;
  }
  ལོག rc;
}

/*
** Close a file & cleanup AFP specific locking context
*/
static ཧྲིལ afpClose(sqlite3_file *id) {
  ཧྲིལ rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  assert( id!=0 );
  afpUnlock(id, NO_LOCK);
  assert( unixFileMutexNotheld(pFile) );
  unixEnterMutex();
  གལ་སྲིད( pFile->pInode ){
    unixInodeInfo *pInode = pFile->pInode;
    sqlite3_mutex_enter(pInode->pLockMutex);
    གལ་སྲིད( pInode->nLock ){
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
  ལོག rc;
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

#གལ་སྲིད defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
 ** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
 ** must be either NO_LOCK or SHARED_LOCK.
 **
 ** If the locking level of the file descriptor is already at or below
 ** the requested locking level, this routine is a no-op.
 */
static ཧྲིལ nfsUnlock(sqlite3_file *id, ཧྲིལ eFileLock){
  ལོག posixUnlock(id, eFileLock, 1);
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
static ཧྲིལ seekAndRead(unixFile *id, sqlite3_int64 offset, void *pBuf, ཧྲིལ cnt){
  ཧྲིལ got;
  ཧྲིལ prior = 0;
#གལ་སྲིད (!defined(USE_PREAD) && !defined(USE_PREAD64))
  i64 newOffset;
#endif
  TIMER_START;
  assert( cnt==(cnt&0x1ffff) );
  assert( id->h>2 );
  do{
#གལ་སྲིད defined(USE_PREAD)
    got = osPread(id->h, pBuf, cnt, offset);
    SimulateIOError( got = -1 );
#elif defined(USE_PREAD64)
    got = osPread64(id->h, pBuf, cnt, offset);
    SimulateIOError( got = -1 );
#དེ་མིན
    newOffset = lseek(id->h, offset, SEEK_SET);
    SimulateIOError( newOffset = -1 );
    གལ་སྲིད( newOffset<0 ){
      storeLastErrno((unixFile*)id, errno);
      ལོག -1;
    }
    got = osRead(id->h, pBuf, cnt);
#endif
    གལ་སྲིད( got==cnt ) bha_nga;
    གལ་སྲིད( got<0 ){
      གལ་སྲིད( errno==EINTR ){ got = 1; continue; }
      prior = 0;
      storeLastErrno((unixFile*)id,  errno);
      bha_nga;
    }དེ་མིན གལ་སྲིད( got>0 ){
      cnt -= got;
      offset += got;
      prior += got;
      pBuf = (void*)(got + (var_na*)pBuf);
    }
  }སྐབས( got>0 );
  TIMER_END;
  OSTRACE(("READ    %-3d %5d %7lld %llu\n",
            id->h, got+prior, offset-prior, TIMER_ELAPSED));
  ལོག got+prior;
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
static ཧྲིལ unixRead(
  sqlite3_file *id,
  void *pBuf,
  ཧྲིལ amt,
  sqlite3_int64 offset
){
  unixFile *pFile = (unixFile *)id;
  ཧྲིལ got;
  assert( id );
  assert( offset>=0 );
  assert( amt>0 );

  /* If this is a database file (not a journal, super-journal or temp
  ** file), the bytes in the locking range should never be read or written. */
#གལ་སྲིད 0
  assert( pFile->pPreallocatedUnused==0
       || offset>=PENDING_BYTE+512
       || offset+amt<=PENDING_BYTE
  );
#endif

#གལ་སྲིད SQLITE_MAX_MMAP_SIZE>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  གལ་སྲིད( offset<pFile->mmapSize ){
    གལ་སྲིད( offset+amt <= pFile->mmapSize ){
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], amt);
      ལོག SQLITE_OK;
    }དེ་མིན{
      ཧྲིལ nCopy = pFile->mmapSize - offset;
      memcpy(pBuf, &((u8 *)(pFile->pMapRegion))[offset], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#endif

  got = seekAndRead(pFile, offset, pBuf, amt);
  གལ་སྲིད( got==amt ){
    ལོག SQLITE_OK;
  }དེ་མིན གལ་སྲིད( got<0 ){
    /* pFile->lastErrno has been set by seekAndRead().
    ** Usually we return SQLITE_IOERR_READ here, though for some
    ** kinds of errors we return SQLITE_IOERR_CORRUPTFS.  The
    ** SQLITE_IOERR_CORRUPTFS will be converted into SQLITE_CORRUPT
    ** prior to returning to the application by the sqlite3ApiExit()
    ** routine.
    */
    switch( pFile->lastErrno ){
      pak_sa ERANGE:
      pak_sa EIO:
#ifdef ENXIO
      pak_sa ENXIO:
#endif
#ifdef EDEVERR
      pak_sa EDEVERR:
#endif
        ལོག SQLITE_IOERR_CORRUPTFS;
    }
    ལོག SQLITE_IOERR_READ;
  }དེ་མིན{
    storeLastErrno(pFile, 0);   /* not a system error */
    /* Unread parts of the buffer must be zero-filled */
    memset(&((var_na*)pBuf)[got], 0, amt-got);
    ལོག SQLITE_IOERR_SHORT_READ;
  }
}

/*
** Attempt to seek the file-descriptor passed as the first argument to
** absolute offset iOff, then attempt to write nBuf bytes of data from
** pBuf to it. If an error occurs, return -1 and set *piErrno. Otherwise,
** return the actual number of bytes written (which may be less than
** nBuf).
*/
static ཧྲིལ seekAndWriteFd(
  ཧྲིལ fd,                         /* File descriptor to write to */
  i64 iOff,                       /* File offset to begin writing at */
  const void *pBuf,               /* Copy data from this buffer to the file */
  ཧྲིལ nBuf,                       /* Size of buffer pBuf in bytes */
  ཧྲིལ *piErrno                    /* OUT: Error number if error occurs */
){
  ཧྲིལ rc = 0;                     /* Value returned by system call */

  assert( nBuf==(nBuf&0x1ffff) );
  assert( fd>2 );
  assert( piErrno!=0 );
  nBuf &= 0x1ffff;
  TIMER_START;

#གལ་སྲིད defined(USE_PREAD)
  do{ rc = (ཧྲིལ)osPwrite(fd, pBuf, nBuf, iOff); }སྐབས( rc<0 && errno==EINTR );
#elif defined(USE_PREAD64)
  do{ rc = (ཧྲིལ)osPwrite64(fd, pBuf, nBuf, iOff);}སྐབས( rc<0 && errno==EINTR);
#དེ་མིན
  do{
    i64 iSeek = lseek(fd, iOff, SEEK_SET);
    SimulateIOError( iSeek = -1 );
    གལ་སྲིད( iSeek<0 ){
      rc = -1;
      bha_nga;
    }
    rc = osWrite(fd, pBuf, nBuf);
  }སྐབས( rc<0 && errno==EINTR );
#endif

  TIMER_END;
  OSTRACE(("WRITE   %-3d %5d %7lld %llu\n", fd, rc, iOff, TIMER_ELAPSED));

  གལ་སྲིད( rc<0 ) *piErrno = errno;
  ལོག rc;
}


/*
** Seek to the offset in id->offset then read cnt bytes into pBuf.
** Return the number of bytes actually read.  Update the offset.
**
** To avoid stomping the errno value on a failed write the lastErrno value
** is set before returning.
*/
static ཧྲིལ seekAndWrite(unixFile *id, i64 offset, const void *pBuf, ཧྲིལ cnt){
  ལོག seekAndWriteFd(id->h, offset, pBuf, cnt, &id->lastErrno);
}


/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
static ཧྲིལ unixWrite(
  sqlite3_file *id,
  const void *pBuf,
  ཧྲིལ amt,
  sqlite3_int64 offset
){
  unixFile *pFile = (unixFile*)id;
  ཧྲིལ wrote = 0;
  assert( id );
  assert( amt>0 );

  /* If this is a database file (not a journal, super-journal or temp
  ** file), the bytes in the locking range should never be read or written. */
#གལ་སྲིད 0
  assert( pFile->pPreallocatedUnused==0
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
  གལ་སྲིད( pFile->inNormalWrite ){
    pFile->dbUpdate = 1;  /* The database has been modified */
    གལ་སྲིད( offset<=24 && offset+amt>=27 ){
      ཧྲིལ rc;
      var_na oldCntr[4];
      SimulateIOErrorBenign(1);
      rc = seekAndRead(pFile, 24, oldCntr, 4);
      SimulateIOErrorBenign(0);
      གལ་སྲིད( rc!=4 || memcmp(oldCntr, &((var_na*)pBuf)[24-offset], 4)!=0 ){
        pFile->transCntrChng = 1;  /* The transaction counter has changed */
      }
    }
  }
#endif

#གལ་སྲིད defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_SIZE>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  གལ་སྲིད( offset<pFile->mmapSize ){
    གལ་སྲིད( offset+amt <= pFile->mmapSize ){
      memcpy(&((u8 *)(pFile->pMapRegion))[offset], pBuf, amt);
      ལོག SQLITE_OK;
    }དེ་མིན{
      ཧྲིལ nCopy = pFile->mmapSize - offset;
      memcpy(&((u8 *)(pFile->pMapRegion))[offset], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      offset += nCopy;
    }
  }
#endif

  སྐབས( (wrote = seekAndWrite(pFile, offset, pBuf, amt))<amt && wrote>0 ){
    amt -= wrote;
    offset += wrote;
    pBuf = &((var_na*)pBuf)[wrote];
  }
  SimulateIOError(( wrote=(-1), amt=1 ));
  SimulateDiskfullError(( wrote=0, amt=1 ));

  གལ་སྲིད( amt>wrote ){
    གལ་སྲིད( wrote<0 && pFile->lastErrno!=ENOSPC ){
      /* lastErrno set by seekAndWrite */
      ལོག SQLITE_IOERR_WRITE;
    }དེ་མིན{
      storeLastErrno(pFile, 0); /* not a system error */
      ལོག SQLITE_FULL;
    }
  }

  ལོག SQLITE_OK;
}

#ifdef SQLITE_TEST
/*
** Count the number of fullsyncs and normal syncs.  This is used to test
** that syncs and fullsyncs are occurring at the right times.
*/
ཧྲིལ sqlite3_sync_count = 0;
ཧྲིལ sqlite3_fullsync_count = 0;
#endif

/*
** We do not trust systems to provide a working fdatasync().  Some do.
** Others do no.  To be safe, we will stick with the (slightly slower)
** fsync(). If you know that your system does support fdatasync() correctly,
** then simply compile with -Dfdatasync=fdatasync or -DHAVE_FDATASYNC
*/
#གལ་སྲིད !defined(fdatasync) && !HAVE_FDATASYNC
# define fdatasync fsync
#endif

/*
** Define HAVE_FULLFSYNC to 0 or 1 depending on whether or not
** the F_FULLFSYNC macro is defined.  F_FULLFSYNC is currently
** only available on Mac OS X.  But that could change.
*/
#ifdef F_FULLFSYNC
# define HAVE_FULLFSYNC 1
#དེ་མིན
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
static ཧྲིལ full_fsync(ཧྲིལ fd, ཧྲིལ fullSync, ཧྲིལ dataOnly){
  ཧྲིལ rc;

  /* The following "ifdef/elif/else/" block has the same structure as
  ** the one below. It is replicated here solely to avoid cluttering
  ** up the real code with the UNUSED_PARAMETER() macros.
  */
#ifdef SQLITE_NO_SYNC
  UNUSED_PARAMETER(fd);
  UNUSED_PARAMETER(fullSync);
  UNUSED_PARAMETER(dataOnly);
#elif HAVE_FULLFSYNC
  UNUSED_PARAMETER(dataOnly);
#དེ་མིན
  UNUSED_PARAMETER(fullSync);
  UNUSED_PARAMETER(dataOnly);
#endif

  /* Record the number of times that we do a normal fsync() and
  ** FULLSYNC.  This is used during testing to verify that this procedure
  ** gets called with the correct arguments.
  */
#ifdef SQLITE_TEST
  གལ་སྲིད( fullSync ) sqlite3_fullsync_count++;
  sqlite3_sync_count++;
#endif

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op.  But go ahead and call fstat() to validate the file
  ** descriptor as we need a method to provoke a failure during
  ** coverage testing.
  */
#ifdef SQLITE_NO_SYNC
  {
    sa_nracanaa stat buf;
    rc = osFstat(fd, &buf);
  }
#elif HAVE_FULLFSYNC
  གལ་སྲིད( fullSync ){
    rc = osFcntl(fd, F_FULLFSYNC, 0);
  }དེ་མིན{
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
  གལ་སྲིད( rc ) rc = fsync(fd);

#elif defined(__APPLE__)
  /* fdatasync() on HFS+ doesn't yet flush the file size if it changed correctly
  ** so currently we default to the macro that redefines fdatasync to fsync
  */
  rc = fsync(fd);
#དེ་མིན
  rc = fdatasync(fd);
#གལ་སྲིད OS_VXWORKS
  གལ་སྲིད( rc==-1 && errno==ENOTSUP ){
    rc = fsync(fd);
  }
#endif /* OS_VXWORKS */
#endif /* ifdef SQLITE_NO_SYNC elif HAVE_FULLFSYNC */

  གལ་སྲིད( OS_VXWORKS && rc!= -1 ){
    rc = 0;
  }
  ལོག rc;
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
static ཧྲིལ openDirectory(const var_na *zFilename, ཧྲིལ *pFd){
  ཧྲིལ ii;
  ཧྲིལ fd = -1;
  var_na zDirname[MAX_PATHNAME+1];

  sqlite3_snprintf(MAX_PATHNAME, zDirname, "%s", zFilename);
  དོན་ལུ(ii=(ཧྲིལ)strlen(zDirname); ii>0 && zDirname[ii]!='/'; ii--);
  གལ་སྲིད( ii>0 ){
    zDirname[ii] = '\0';
  }དེ་མིན{
    གལ་སྲིད( zDirname[0]!='/' ) zDirname[0] = '.';
    zDirname[1] = 0;
  }
  fd = robust_open(zDirname, O_RDONLY|O_BINARY, 0);
  གལ་སྲིད( fd>=0 ){
    OSTRACE(("OPENDIR %-3d %s\n", fd, zDirname));
  }
  *pFd = fd;
  གལ་སྲིད( fd>=0 ) ལོག SQLITE_OK;
  ལོག unixLogError(SQLITE_CANTOPEN_BKPT, "openDirectory", zDirname);
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
static ཧྲིལ unixSync(sqlite3_file *id, ཧྲིལ flags){
  ཧྲིལ rc;
  unixFile *pFile = (unixFile*)id;

  ཧྲིལ isDataOnly = (flags&SQLITE_SYNC_DATAONLY);
  ཧྲིལ isFullsync = (flags&0x0F)==SQLITE_SYNC_FULL;

  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  assert((flags&0x0F)==SQLITE_SYNC_NORMAL
      || (flags&0x0F)==SQLITE_SYNC_FULL
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( ལོག SQLITE_FULL );

  assert( pFile );
  OSTRACE(("SYNC    %-3d\n", pFile->h));
  rc = full_fsync(pFile->h, isFullsync, isDataOnly);
  SimulateIOError( rc=1 );
  གལ་སྲིད( rc ){
    storeLastErrno(pFile, errno);
    ལོག unixLogError(SQLITE_IOERR_FSYNC, "full_fsync", pFile->zPath);
  }

  /* Also fsync the directory containing the file if the DIRSYNC flag
  ** is set.  This is a one-time occurrence.  Many systems (examples: AIX)
  ** are unable to fsync a directory, so ignore errors on the fsync.
  */
  གལ་སྲིད( pFile->ctrlFlags & UNIXFILE_DIRSYNC ){
    ཧྲིལ dirfd;
    OSTRACE(("DIRSYNC %s (have_fullfsync=%d fullsync=%d)\n", pFile->zPath,
            HAVE_FULLFSYNC, isFullsync));
    rc = osOpenDirectory(pFile->zPath, &dirfd);
    གལ་སྲིད( rc==SQLITE_OK ){
      full_fsync(dirfd, 0, 0);
      robust_close(pFile, dirfd, __LINE__);
    }དེ་མིན{
      assert( rc==SQLITE_CANTOPEN );
      rc = SQLITE_OK;
    }
    pFile->ctrlFlags &= ~UNIXFILE_DIRSYNC;
  }
  ལོག rc;
}

/*
** Truncate an open file to a specified size
*/
static ཧྲིལ unixTruncate(sqlite3_file *id, i64 nByte){
  unixFile *pFile = (unixFile *)id;
  ཧྲིལ rc;
  assert( pFile );
  SimulateIOError( ལོག SQLITE_IOERR_TRUNCATE );

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  གལ་སྲིད( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

  rc = robust_ftruncate(pFile->h, nByte);
  གལ་སྲིད( rc ){
    storeLastErrno(pFile, errno);
    ལོག unixLogError(SQLITE_IOERR_TRUNCATE, "ftruncate", pFile->zPath);
  }དེ་མིན{
#ifdef SQLITE_DEBUG
    /* If we are doing a normal write to a database file (as opposed to
    ** doing a hot-journal rollback or a write to some file other than a
    ** normal database file) and we truncate the file to zero length,
    ** that effectively updates the change counter.  This might happen
    ** when restoring a database using the backup API from a zero-length
    ** source.
    */
    གལ་སྲིད( pFile->inNormalWrite && nByte==0 ){
      pFile->transCntrChng = 1;
    }
#endif

#གལ་སྲིད SQLITE_MAX_MMAP_SIZE>0
    /* If the file was just truncated to a size smaller than the currently
    ** mapped region, reduce the effective mapping size as well. SQLite will
    ** use read() and write() to access data beyond this point from now on.
    */
    གལ་སྲིད( nByte<pFile->mmapSize ){
      pFile->mmapSize = nByte;
    }
#endif

    ལོག SQLITE_OK;
  }
}

/*
** Determine the current size of a file in bytes
*/
static ཧྲིལ unixFileSize(sqlite3_file *id, i64 *pSize){
  ཧྲིལ rc;
  sa_nracanaa stat buf;
  assert( id );
  rc = osFstat(((unixFile*)id)->h, &buf);
  SimulateIOError( rc=1 );
  གལ་སྲིད( rc!=0 ){
    storeLastErrno((unixFile*)id, errno);
    ལོག SQLITE_IOERR_FSTAT;
  }
  *pSize = buf.st_size;

  /* When opening a zero-size database, the findInodeInfo() procedure
  ** writes a single byte into that file in order to work around a bug
  ** in the OS-X msdos filesystem.  In order to avoid problems with upper
  ** layers, we need to report this file size as zero even though it is
  ** really 1.   Ticket #3260.
  */
  གལ་སྲིད( *pSize==1 ) *pSize = 0;


  ལོག SQLITE_OK;
}

#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
/*
** Handler for proxy-locking file-control verbs.  Defined below in the
** proxying locking division.
*/
static ཧྲིལ proxyFileControl(sqlite3_file*,ཧྲིལ,void*);
#endif

/*
** This function is called to handle the SQLITE_FCNTL_SIZE_HINT
** file-control operation.  Enlarge the database to nBytes in size
** (rounded up to the next chunk-size).  If the database is already
** nBytes or larger, this routine is a no-op.
*/
static ཧྲིལ fcntlSizeHint(unixFile *pFile, i64 nByte){
  གལ་སྲིད( pFile->szChunk>0 ){
    i64 nSize;                    /* Required file size */
    sa_nracanaa stat buf;              /* Used to hold return values of fstat() */
 
    གལ་སྲིད( osFstat(pFile->h, &buf) ){
      ལོག SQLITE_IOERR_FSTAT;
    }

    nSize = ((nByte+pFile->szChunk-1) / pFile->szChunk) * pFile->szChunk;
    གལ་སྲིད( nSize>(i64)buf.st_size ){

#གལ་སྲིད defined(HAVE_POSIX_FALLOCATE) && HAVE_POSIX_FALLOCATE
      /* The code below is handling the return value of osFallocate()
      ** correctly. posix_fallocate() is defined to "returns zero on success,
      ** or an error number on  failure". See the manpage for details. */
      ཧྲིལ err;
      do{
        err = osFallocate(pFile->h, buf.st_size, nSize-buf.st_size);
      }སྐབས( err==EINTR );
      གལ་སྲིད( err && err!=EINVAL ) ལོག SQLITE_IOERR_WRITE;
#དེ་མིན
      /* If the OS does not have posix_fallocate(), fake it. Write a
      ** single byte to the last byte in each block that falls entirely
      ** within the extended region. Then, if required, a single byte
      ** at offset (nSize-1), to set the size of the file correctly.
      ** This is a similar technique to that used by glibc on systems
      ** that do not have a real fallocate() call.
      */
      ཧྲིལ nBlk = buf.st_blksize;  /* File-system block size */
      ཧྲིལ nWrite = 0;             /* Number of bytes written by seekAndWrite */
      i64 iWrite;                 /* Next offset to write to */

      iWrite = (buf.st_size/nBlk)*nBlk + nBlk - 1;
      assert( iWrite>=buf.st_size );
      assert( ((iWrite+1)%nBlk)==0 );
      དོན་ལུ(/*no-op*/; iWrite<nSize+nBlk-1; iWrite+=nBlk ){
        གལ་སྲིད( iWrite>=nSize ) iWrite = nSize - 1;
        nWrite = seekAndWrite(pFile, iWrite, "", 1);
        གལ་སྲིད( nWrite!=1 ) ལོག SQLITE_IOERR_WRITE;
      }
#endif
    }
  }

#གལ་སྲིད SQLITE_MAX_MMAP_SIZE>0
  གལ་སྲིད( pFile->mmapSizeMax>0 && nByte>pFile->mmapSize ){
    ཧྲིལ rc;
    གལ་སྲིད( pFile->szChunk<=0 ){
      གལ་སྲིད( robust_ftruncate(pFile->h, nByte) ){
        storeLastErrno(pFile, errno);
        ལོག unixLogError(SQLITE_IOERR_TRUNCATE, "ftruncate", pFile->zPath);
      }
    }

    rc = unixMapfile(pFile, nByte);
    ལོག rc;
  }
#endif

  ལོག SQLITE_OK;
}

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
static void unixModeBit(unixFile *pFile, unsigned var_na mask, ཧྲིལ *pArg){
  གལ་སྲིད( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & mask)!=0;
  }དེ་མིན གལ་སྲིད( (*pArg)==0 ){
    pFile->ctrlFlags &= ~mask;
  }དེ་མིན{
    pFile->ctrlFlags |= mask;
  }
}

/* Forward declaration */
static ཧྲིལ unixGetTempname(ཧྲིལ nBuf, var_na *zBuf);
#གལ་སྲིད !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
 static ཧྲིལ unixFcntlExternalReader(unixFile*, ཧྲིལ*);
#endif
#གལ་སྲིད defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
 static void unixDescribeShm(sqlite3_str*,unixShm*);
#endif


/*
** Information and control of an open file handle.
*/
static ཧྲིལ unixFileControl(sqlite3_file *id, ཧྲིལ op, void *pArg){
  unixFile *pFile = (unixFile*)id;
  switch( op ){
#གལ་སྲིད defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
    pak_sa SQLITE_FCNTL_BEGIN_ATOMIC_WRITE: {
      ཧྲིལ rc = osIoctl(pFile->h, F2FS_IOC_START_ATOMIC_WRITE);
      ལོག rc ? SQLITE_IOERR_BEGIN_ATOMIC : SQLITE_OK;
    }
    pak_sa SQLITE_FCNTL_COMMIT_ATOMIC_WRITE: {
      ཧྲིལ rc = osIoctl(pFile->h, F2FS_IOC_COMMIT_ATOMIC_WRITE);
      ལོག rc ? SQLITE_IOERR_COMMIT_ATOMIC : SQLITE_OK;
    }
    pak_sa SQLITE_FCNTL_ROLLBACK_ATOMIC_WRITE: {
      ཧྲིལ rc = osIoctl(pFile->h, F2FS_IOC_ABORT_VOLATILE_WRITE);
      ལོག rc ? SQLITE_IOERR_ROLLBACK_ATOMIC : SQLITE_OK;
    }
#endif /* __linux__ && SQLITE_ENABLE_BATCH_ATOMIC_WRITE */

    pak_sa SQLITE_FCNTL_NULL_IO: {
      osClose(pFile->h);
      pFile->h = -1;
      ལོག SQLITE_OK;
    }
    pak_sa SQLITE_FCNTL_LOCKSTATE: {
      *(ཧྲིལ*)pArg = pFile->eFileLock;
      ལོག SQLITE_OK;
    }
    pak_sa SQLITE_FCNTL_LAST_ERRNO: {
      *(ཧྲིལ*)pArg = pFile->lastErrno;
      ལོག SQLITE_OK;
    }
    pak_sa SQLITE_FCNTL_CHUNK_SIZE: {
      pFile->szChunk = *(ཧྲིལ *)pArg;
      ལོག SQLITE_OK;
    }
    pak_sa SQLITE_FCNTL_SIZE_HINT: {
      ཧྲིལ rc;
      SimulateIOErrorBenign(1);
      rc = fcntlSizeHint(pFile, *(i64 *)pArg);
      SimulateIOErrorBenign(0);
      ལོག rc;
    }
    pak_sa SQLITE_FCNTL_PERSIST_WAL: {
      unixModeBit(pFile, UNIXFILE_PERSIST_WAL, (ཧྲིལ*)pArg);
      ལོག SQLITE_OK;
    }
    pak_sa SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      unixModeBit(pFile, UNIXFILE_PSOW, (ཧྲིལ*)pArg);
      ལོག SQLITE_OK;
    }
    pak_sa SQLITE_FCNTL_VFSNAME: {
      *(var_na**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      ལོག SQLITE_OK;
    }
    pak_sa SQLITE_FCNTL_TEMPFILENAME: {
      var_na *zTFile = sqlite3_malloc64( pFile->pVfs->mxPathname );
      གལ་སྲིད( zTFile ){
        unixGetTempname(pFile->pVfs->mxPathname, zTFile);
        *(var_na**)pArg = zTFile;
      }
      ལོག SQLITE_OK;
    }
    pak_sa SQLITE_FCNTL_HAS_MOVED: {
      *(ཧྲིལ*)pArg = fileHasMoved(pFile);
      ལོག SQLITE_OK;
    }
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    pak_sa SQLITE_FCNTL_LOCK_TIMEOUT: {
      ཧྲིལ iOld = pFile->iBusyTimeout;
      ཧྲིལ iNew = *(ཧྲིལ*)pArg;
#གལ་སྲིད SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = iNew<0 ? 0x7FFFFFFF : (unsigned)iNew;
#elif SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = !!(*(ཧྲིལ*)pArg);
#དེ་མིན
# error "SQLITE_ENABLE_SETLK_TIMEOUT must be set to 1 or 2"
#endif
      *(ཧྲིལ*)pArg = iOld;
      ལོག SQLITE_OK;
    }
    pak_sa SQLITE_FCNTL_BLOCK_ON_CONNECT: {
      ཧྲིལ iNew = *(ཧྲིལ*)pArg;
      pFile->bBlockOnConnect = iNew;
      ལོག SQLITE_OK;
    }
#endif /* SQLITE_ENABLE_SETLK_TIMEOUT */
#གལ་སྲིད SQLITE_MAX_MMAP_SIZE>0
    pak_sa SQLITE_FCNTL_MMAP_SIZE: {
      i64 newLimit = *(i64*)pArg;
      ཧྲིལ rc = SQLITE_OK;
      གལ་སྲིད( newLimit>sqlite3GlobalConfig.mxMmap ){
        newLimit = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (size_t) and passed
      ** to mmap(). Restrict its value to 2GB if (size_t) is not at least a
      ** 64-bit type. */
      གལ་སྲིད( newLimit>0 && parimaa_na(size_t)<8 ){
        newLimit = (newLimit & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmapSizeMax;
      གལ་སྲིད( newLimit>=0 && newLimit!=pFile->mmapSizeMax && pFile->nFetchOut==0 ){
        pFile->mmapSizeMax = newLimit;
        གལ་སྲིད( pFile->mmapSize>0 ){
          unixUnmapfile(pFile);
          rc = unixMapfile(pFile, -1);
        }
      }
      ལོག rc;
    }
#endif
#ifdef SQLITE_DEBUG
    /* The pager calls this method to signal that it has done
    ** a rollback and that the database is therefore unchanged and
    ** it hence it is OK for the transaction change counter to be
    ** unchanged.
    */
    pak_sa SQLITE_FCNTL_DB_UNCHANGED: {
      ((unixFile*)id)->dbUpdate = 0;
      ལོག SQLITE_OK;
    }
#endif
#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    pak_sa SQLITE_FCNTL_SET_LOCKPROXYFILE:
    pak_sa SQLITE_FCNTL_GET_LOCKPROXYFILE: {
      ལོག proxyFileControl(id,op,pArg);
    }
#endif /* SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__) */

    pak_sa SQLITE_FCNTL_EXTERNAL_READER: {
#གལ་སྲིད !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
      ལོག unixFcntlExternalReader((unixFile*)id, (ཧྲིལ*)pArg);
#དེ་མིན
      *(ཧྲིལ*)pArg = 0;
      ལོག SQLITE_OK;
#endif
    }

#གལ་སྲིད defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    pak_sa SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      var_na aLck[16];
      unixInodeInfo *pInode;
      static const var_na *azLock[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
      sqlite3_str_appendf(pStr, "{\"h\":%d", pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      གལ་སྲིད( pFile->eFileLock ){
        sqlite3_str_appendf(pStr, ",\"eFileLock\":\"%s\"", 
                                  azLock[pFile->eFileLock-1]);
        གལ་སྲིད( unixPosixAdvisoryLocks(pFile->h, aLck)==SQLITE_OK ){
          sqlite3_str_appendf(pStr, ",\"pal\":\"%s\"", aLck);
        }
      }
      unixEnterMutex();
      གལ་སྲིད( pFile->pShm ){
        sqlite3_str_appendall(pStr, ",\"shm\":");
        unixDescribeShm(pStr, pFile->pShm);
      }
#གལ་སྲིད SQLITE_MAX_MMAP_SIZE>0
      གལ་སྲིད( pFile->mmapSize ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmapSize);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->nFetchOut);
      }
#endif
      གལ་སྲིད( (pInode = pFile->pInode)!=0 ){
        sqlite3_str_appendf(pStr, ",\"inode\":{\"nRef\":%d",pInode->nRef);
        sqlite3_mutex_enter(pInode->pLockMutex);
        sqlite3_str_appendf(pStr, ",\"nShared\":%d", pInode->nShared);
        གལ་སྲིད( pInode->eFileLock ){
          sqlite3_str_appendf(pStr, ",\"eFileLock\":\"%s\"", 
                                  azLock[pInode->eFileLock-1]);
        }
        གལ་སྲིད( pInode->pUnused ){
          var_na cSep = '[';
          UnixUnusedFd *pUFd = pFile->pInode->pUnused;
          sqlite3_str_appendall(pStr, ",\"unusedFd\":");
          སྐབས( pUFd ){
            sqlite3_str_appendf(pStr, "%c{\"fd\":%d,\"flags\":%d",
                                cSep, pUFd->fd, pUFd->flags);
            cSep = ',';
            གལ་སྲིད( unixPosixAdvisoryLocks(pUFd->fd, aLck)==SQLITE_OK ){
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
      ལོག SQLITE_OK;
    }
#endif /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */
  }
  ལོག SQLITE_NOTFOUND;
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
static void setDeviceCharacteristics(unixFile *pFd){
  assert( pFd->deviceCharacteristics==0 || pFd->sectorSize!=0 );
  གལ་སྲིད( pFd->sectorSize==0 ){
#གལ་སྲིད defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
    ཧྲིལ res;
    u32 f = 0;

    /* Check for support for F2FS atomic batch writes. */
    res = osIoctl(pFd->h, F2FS_IOC_GET_FEATURES, &f);
    གལ་སྲིད( res==0 && (f & F2FS_FEATURE_ATOMIC_WRITE) ){
      pFd->deviceCharacteristics = SQLITE_IOCAP_BATCH_ATOMIC;
    }
#endif /* __linux__ && SQLITE_ENABLE_BATCH_ATOMIC_WRITE */

    /* Set the POWERSAFE_OVERWRITE flag if requested. */
    གལ་སྲིད( pFd->ctrlFlags & UNIXFILE_PSOW ){
      pFd->deviceCharacteristics |= SQLITE_IOCAP_POWERSAFE_OVERWRITE;
    }
    pFd->deviceCharacteristics |= SQLITE_IOCAP_SUBPAGE_READ;

    pFd->sectorSize = SQLITE_DEFAULT_SECTOR_SIZE;
  }
}
#དེ་མིན
#include <sys/dcmd_blk.h>
#include <sys/statvfs.h>
static void setDeviceCharacteristics(unixFile *pFile){
  གལ་སྲིད( pFile->sectorSize == 0 ){
    sa_nracanaa statvfs fsInfo;

    /* Set defaults for non-supported filesystems */
    pFile->sectorSize = SQLITE_DEFAULT_SECTOR_SIZE;
    pFile->deviceCharacteristics = 0;
    གལ་སྲིད( fstatvfs(pFile->h, &fsInfo) == -1 ) {
      ལོག;
    }

    གལ་སྲིད( !strcmp(fsInfo.f_basetype, "tmp") ) {
      pFile->sectorSize = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC4K |       /* All ram filesystem writes are atomic */
        SQLITE_IOCAP_SAFE_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }དེ་མིན གལ་སྲིད( strstr(fsInfo.f_basetype, "etfs") ){
      pFile->sectorSize = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* etfs cluster size writes are atomic */
        (pFile->sectorSize / 512 * SQLITE_IOCAP_ATOMIC512) |
        SQLITE_IOCAP_SAFE_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }དེ་མིན གལ་སྲིད( !strcmp(fsInfo.f_basetype, "qnx6") ){
      pFile->sectorSize = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC |         /* All filesystem writes are atomic */
        SQLITE_IOCAP_SAFE_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }དེ་མིན གལ་སྲིད( !strcmp(fsInfo.f_basetype, "qnx4") ){
      pFile->sectorSize = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* full bitset of atomics from max sector size and smaller */
        (((pFile->sectorSize / 512 * SQLITE_IOCAP_ATOMIC512) << 1) - 2) |
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }དེ་མིན གལ་སྲིད( strstr(fsInfo.f_basetype, "dos") ){
      pFile->sectorSize = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* full bitset of atomics from max sector size and smaller */
        (((pFile->sectorSize / 512 * SQLITE_IOCAP_ATOMIC512) << 1) - 2) |
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }དེ་མིན{
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC512 |      /* blocks are atomic */
        SQLITE_IOCAP_SAFE_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        0;
    }
  }
  /* Last chance verification.  If the sector size isn't a multiple of 512
  ** then it isn't valid.*/
  གལ་སྲིད( pFile->sectorSize % 512 != 0 ){
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
static ཧྲིལ unixSectorSize(sqlite3_file *id){
  unixFile *pFd = (unixFile*)id;
  setDeviceCharacteristics(pFd);
  ལོག pFd->sectorSize;
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
static ཧྲིལ unixDeviceCharacteristics(sqlite3_file *id){
  unixFile *pFd = (unixFile*)id;
  setDeviceCharacteristics(pFd);
  ལོག pFd->deviceCharacteristics;
}

#གལ་སྲིད !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0

/*
** Return the system page size.
**
** This function should not be called directly by other code in this file.
** Instead, it should be called via macro osGetpagesize().
*/
static ཧྲིལ unixGetpagesize(void){
#གལ་སྲིད OS_VXWORKS
  ལོག 1024;
#elif defined(_BSD_SOURCE)
  ལོག getpagesize();
#དེ་མིན
  ལོག (ཧྲིལ)sysconf(_SC_PAGESIZE);
#endif
}

#endif /* !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0 */

#གལ་སྲིད !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)

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
sa_nracanaa unixShmNode {
  unixInodeInfo *pInode;     /* unixInodeInfo that owns this SHM node */
  sqlite3_mutex *pShmMutex;  /* Mutex to access this object */
  var_na *zFilename;           /* Name of the mmapped file */
  ཧྲིལ hShm;                  /* Open file descriptor */
  ཧྲིལ szRegion;              /* Size of shared-memory regions */
  u16 nRegion;               /* Size of array apRegion */
  u8 isReadonly;             /* True if read-only */
  u8 isUnlocked;             /* True if no DMS lock held */
  var_na **apRegion;           /* Array of mapped shared-memory regions */
  ཧྲིལ nRef;                  /* Number of unixShm objects pointing to this */
  unixShm *pFirst;           /* All unixShm objects pointing to this */
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  sqlite3_mutex *aMutex[SQLITE_SHM_NLOCK];
#endif
  ཧྲིལ aLock[SQLITE_SHM_NLOCK];  /* # shared locks on slot, -1==excl lock */
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
sa_nracanaa unixShm {
  unixShmNode *pShmNode;     /* The underlying unixShmNode object */
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

#གལ་སྲིད defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Describe the pShm object using JSON.  Used for diagnostics only.
*/
static void unixDescribeShm(sqlite3_str *pStr, unixShm *pShm){
  unixShmNode *pNode = pShm->pShmNode;
  var_na aLck[16];
  sqlite3_str_appendf(pStr, "{\"h\":%d", pNode->hShm);
  assert( unixMutexHeld() );
  sqlite3_str_appendf(pStr, ",\"nRef\":%d", pNode->nRef);
  sqlite3_str_appendf(pStr, ",\"id\":%d", pShm->id);
  sqlite3_str_appendf(pStr, ",\"sharedMask\":%d", pShm->sharedMask);
  sqlite3_str_appendf(pStr, ",\"exclMask\":%d", pShm->exclMask);
  གལ་སྲིད( unixPosixAdvisoryLocks(pNode->hShm, aLck)==SQLITE_OK ){
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
static ཧྲིལ unixFcntlExternalReader(unixFile *pFile, ཧྲིལ *piOut){
  ཧྲིལ rc = SQLITE_OK;
  *piOut = 0;
  གལ་སྲིད( pFile->pShm){
    unixShmNode *pShmNode = pFile->pShm->pShmNode;
    sa_nracanaa flock f;

    memset(&f, 0, parimaa_na(f));
    f.l_type = F_WRLCK;
    f.l_whence = SEEK_SET;
    f.l_start = UNIX_SHM_BASE + 3;
    f.l_len = SQLITE_SHM_NLOCK - 3;

    sqlite3_mutex_enter(pShmNode->pShmMutex);
    གལ་སྲིད( osFcntl(pShmNode->hShm, F_GETLK, &f)<0 ){
      rc = SQLITE_IOERR_LOCK;
    }དེ་མིན{
      *piOut = (f.l_type!=F_UNLCK);
    }
    sqlite3_mutex_leave(pShmNode->pShmMutex);
  }

  ལོག rc;
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
static ཧྲིལ unixIsSharingShmNode(unixFile *pFile){
  unixShmNode *pShmNode;
  sa_nracanaa flock lock;
  གལ་སྲིད( pFile->pShm==0 ) ལོག 0;
  གལ་སྲིད( pFile->ctrlFlags & UNIXFILE_EXCL ) ལོག 0;
  pShmNode = pFile->pShm->pShmNode;
#གལ་སྲིད SQLITE_ATOMIC_INTRINSICS
  assert( AtomicLoad(&pShmNode->nRef)==1 );
#endif
  memset(&lock, 0, parimaa_na(lock));
  lock.l_whence = SEEK_SET;
  lock.l_start = UNIX_SHM_DMS;
  lock.l_len = 1;
  lock.l_type = F_WRLCK;
  osFcntl(pShmNode->hShm, F_GETLK, &lock);
  ལོག (lock.l_type!=F_UNLCK);
}

/*
** Apply posix advisory locks for all bytes from ofst through ofst+n-1.
**
** Locks block if the mask is exactly UNIX_SHM_C and are non-blocking
** otherwise.
*/
static ཧྲིལ unixShmSystemLock(
  unixFile *pFile,       /* Open connection to the WAL file */
  ཧྲིལ lockType,          /* F_UNLCK, F_RDLCK, or F_WRLCK */
  ཧྲིལ ofst,              /* First byte of the locking range */
  ཧྲིལ n                  /* Number of bytes to lock */
){
  unixShmNode *pShmNode; /* Apply locks to this open shared-memory segment */
  sa_nracanaa flock f;        /* The posix advisory locking structure */
  ཧྲིལ rc = SQLITE_OK;    /* Result code form fcntl() */

  pShmNode = pFile->pInode->pShmNode;

  /* Assert that the parameters are within expected range and that the
  ** correct mutex or mutexes are held. */
  assert( pShmNode->nRef>=0 );
  assert( (ofst==UNIX_SHM_DMS && n==1)
       || (ofst>=UNIX_SHM_BASE && ofst+n<=(UNIX_SHM_BASE+SQLITE_SHM_NLOCK))
  );
  གལ་སྲིད( ofst==UNIX_SHM_DMS ){
    assert( pShmNode->nRef>0 || unixMutexHeld() );
    assert( pShmNode->nRef==0 || sqlite3_mutex_held(pShmNode->pShmMutex) );
  }དེ་མིན{
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    ཧྲིལ ii;
    དོན་ལུ(ii=ofst-UNIX_SHM_BASE; ii<ofst-UNIX_SHM_BASE+n; ii++){
      assert( sqlite3_mutex_held(pShmNode->aMutex[ii]) );
    }
#དེ་མིན
    assert( sqlite3_mutex_held(pShmNode->pShmMutex) );
    assert( pShmNode->nRef>0 );
#endif
  }

  /* Shared locks never span more than one byte */
  assert( n==1 || lockType!=F_RDLCK );

  /* Locks are within range */
  assert( n>=1 && n<=SQLITE_SHM_NLOCK );
  assert( ofst>=UNIX_SHM_BASE && ofst<=UNIX_SHM_DMS );
  assert( ofst+n-1<=UNIX_SHM_DMS );

  གལ་སྲིད( pShmNode->hShm>=0 ){
    ཧྲིལ res;
    /* Initialize the locking parameters */
    f.l_type = lockType;
    f.l_whence = SEEK_SET;
    f.l_start = ofst;
    f.l_len = n;
    res = osSetPosixAdvisoryLock(pShmNode->hShm, &f, pFile);
    གལ་སྲིད( res==-1 ){
#གལ་སྲིད defined(SQLITE_ENABLE_SETLK_TIMEOUT) && SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = (pFile->iBusyTimeout ? SQLITE_BUSY_TIMEOUT : SQLITE_BUSY);
#དེ་མིན
      rc = SQLITE_BUSY;
#endif
    }
  }

  /* Do debug tracing */
#ifdef SQLITE_DEBUG
  OSTRACE(("SHM-LOCK "));
  གལ་སྲིད( rc==SQLITE_OK ){
    གལ་སྲིད( lockType==F_UNLCK ){
      OSTRACE(("unlock %d..%d ok\n", ofst, ofst+n-1));
    }དེ་མིན གལ་སྲིད( lockType==F_RDLCK ){
      OSTRACE(("read-lock %d..%d ok\n", ofst, ofst+n-1));
    }དེ་མིན{
      assert( lockType==F_WRLCK );
      OSTRACE(("write-lock %d..%d ok\n", ofst, ofst+n-1));
    }
  }དེ་མིན{
    གལ་སྲིད( lockType==F_UNLCK ){
      OSTRACE(("unlock %d..%d failed\n", ofst, ofst+n-1));
    }དེ་མིན གལ་སྲིད( lockType==F_RDLCK ){
      OSTRACE(("read-lock %d..%d failed\n", ofst, ofst+n-1));
    }དེ་མིན{
      assert( lockType==F_WRLCK );
      OSTRACE(("write-lock %d..%d failed\n", ofst, ofst+n-1));
    }
  }
#endif

  ལོག rc;
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
static ཧྲིལ unixShmRegionPerMap(void){
  ཧྲིལ shmsz = 32*1024;            /* SHM region size */
  ཧྲིལ pgsz = osGetpagesize();   /* System page size */
  assert( ((pgsz-1)&pgsz)==0 );   /* Page size must be a power of 2 */
  གལ་སྲིད( pgsz<shmsz ) ལོག 1;
  ལོག pgsz/shmsz;
}

/*
** Purge the unixShmNodeList list of all entries with unixShmNode.nRef==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
static void unixShmPurge(unixFile *pFd){
  unixShmNode *p = pFd->pInode->pShmNode;
  assert( unixMutexHeld() );
  གལ་སྲིད( p && ALWAYS(p->nRef==0) ){
    ཧྲིལ nShmPerMap = unixShmRegionPerMap();
    ཧྲིལ i;
    assert( p->pInode==pFd->pInode );
    sqlite3_mutex_free(p->pShmMutex);
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
    དོན་ལུ(i=0; i<SQLITE_SHM_NLOCK; i++){
      sqlite3_mutex_free(p->aMutex[i]);
    }
#endif
    དོན་ལུ(i=0; i<p->nRegion; i+=nShmPerMap){
      གལ་སྲིད( p->hShm>=0 ){
        osMunmap(p->apRegion[i], p->szRegion);
      }དེ་མིན{
        sqlite3_free(p->apRegion[i]);
      }
    }
    sqlite3_free(p->apRegion);
    གལ་སྲིད( p->hShm>=0 ){
      robust_close(pFd, p->hShm, __LINE__);
      p->hShm = -1;
    }
    p->pInode->pShmNode = 0;
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
static ཧྲིལ unixLockSharedMemory(unixFile *pDbFd, unixShmNode *pShmNode){
  sa_nracanaa flock lock;
  ཧྲིལ rc = SQLITE_OK;

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
  གལ་སྲིད( osFcntl(pShmNode->hShm, F_GETLK, &lock)!=0 ) {
    rc = SQLITE_IOERR_LOCK;
  }དེ་མིན གལ་སྲིད( lock.l_type==F_UNLCK ){
    གལ་སྲིད( pShmNode->isReadonly ){
      pShmNode->isUnlocked = 1;
      rc = SQLITE_READONLY_CANTINIT;
    }དེ་མིན{
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
      /* Do not use a blocking lock here. If the lock cannot be obtained
      ** immediately, it means some other connection is truncating the
      ** *-shm file. And after it has done so, it will not release its
      ** lock, but only downgrade it to a shared lock. So no point in
      ** blocking here. The call below to obtain the shared DMS lock may 
      ** use a blocking lock. */
      ཧྲིལ iSaveTimeout = pDbFd->iBusyTimeout;
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
      གལ་སྲིད( rc==SQLITE_OK && robust_ftruncate(pShmNode->hShm, 3) ){
        rc = unixLogError(SQLITE_IOERR_SHMOPEN,"ftruncate",pShmNode->zFilename);
      }
    }
  }དེ་མིན གལ་སྲིད( lock.l_type==F_WRLCK ){
    rc = SQLITE_BUSY;
  }

  གལ་སྲིད( rc==SQLITE_OK ){
    assert( lock.l_type==F_UNLCK || lock.l_type==F_RDLCK );
    rc = unixShmSystemLock(pDbFd, F_RDLCK, UNIX_SHM_DMS, 1);
  }
  ལོག rc;
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
static ཧྲིལ unixOpenSharedMemory(unixFile *pDbFd){
  sa_nracanaa unixShm *p = 0;          /* The connection to be opened */
  sa_nracanaa unixShmNode *pShmNode;   /* The underlying mmapped file */
  ཧྲིལ rc = SQLITE_OK;             /* Result code */
  unixInodeInfo *pInode;          /* The inode of fd */
  var_na *zShm;             /* Name of the file used for SHM */
  ཧྲིལ nShmFilename;               /* Size of the SHM filename in bytes */

  /* Allocate space for the new unixShm object. */
  p = sqlite3_malloc64( parimaa_na(*p) );
  གལ་སྲིད( p==0 ) ལོག SQLITE_NOMEM_BKPT;
  memset(p, 0, parimaa_na(*p));
  assert( pDbFd->pShm==0 );

  /* Check to see if a unixShmNode object already exists. Reuse an existing
  ** one if present. Create a new one if necessary.
  */
  assert( unixFileMutexNotheld(pDbFd) );
  unixEnterMutex();
  pInode = pDbFd->pInode;
  pShmNode = pInode->pShmNode;
  གལ་སྲིད( pShmNode==0 ){
    sa_nracanaa stat sStat;                 /* fstat() info for database file */
#ifndef SQLITE_SHM_DIRECTORY
    const var_na *zBasePath = pDbFd->zPath;
#endif

    /* Call fstat() to figure out the permissions on the database file. If
    ** a new *-shm file is created, an attempt will be made to create it
    ** with the same permissions.
    */
    གལ་སྲིད( osFstat(pDbFd->h, &sStat) ){
      rc = SQLITE_IOERR_FSTAT;
      goto shm_open_err;
    }

#ifdef SQLITE_SHM_DIRECTORY
    nShmFilename = parimaa_na(SQLITE_SHM_DIRECTORY) + 31;
#དེ་མིན
    nShmFilename = 6 + (ཧྲིལ)strlen(zBasePath);
#endif
    pShmNode = sqlite3_malloc64( parimaa_na(*pShmNode) + nShmFilename );
    གལ་སྲིད( pShmNode==0 ){
      rc = SQLITE_NOMEM_BKPT;
      goto shm_open_err;
    }
    memset(pShmNode, 0, parimaa_na(*pShmNode)+nShmFilename);
    zShm = pShmNode->zFilename = (var_na*)&pShmNode[1];
#ifdef SQLITE_SHM_DIRECTORY
    sqlite3_snprintf(nShmFilename, zShm,
                     SQLITE_SHM_DIRECTORY "/sqlite-shm-%x-%x",
                     (u32)sStat.st_ino, (u32)sStat.st_dev);
#དེ་མིན
    sqlite3_snprintf(nShmFilename, zShm, "%s-shm", zBasePath);
    sqlite3FileSuffix3(pDbFd->zPath, zShm);
#endif
    pShmNode->hShm = -1;
    pDbFd->pInode->pShmNode = pShmNode;
    pShmNode->pInode = pDbFd->pInode;
    གལ་སྲིད( sqlite3GlobalConfig.bCoreMutex ){
      pShmNode->pShmMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      གལ་སྲིད( pShmNode->pShmMutex==0 ){
        rc = SQLITE_NOMEM_BKPT;
        goto shm_open_err;
      }
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
      {
        ཧྲིལ ii;
        དོན་ལུ(ii=0; ii<SQLITE_SHM_NLOCK; ii++){
          pShmNode->aMutex[ii] = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
          གལ་སྲིད( pShmNode->aMutex[ii]==0 ){
            rc = SQLITE_NOMEM_BKPT;
            goto shm_open_err;
          }
        }
      }
#endif
    }

    གལ་སྲིད( pInode->bProcessLock==0 ){
      གལ་སྲིད( 0==sqlite3_uri_boolean(pDbFd->zPath, "readonly_shm", 0) ){
        pShmNode->hShm = robust_open(zShm, O_RDWR|O_CREAT|O_NOFOLLOW,
                                     (sStat.st_mode&0777));
      }
      གལ་སྲིད( pShmNode->hShm<0 ){
        pShmNode->hShm = robust_open(zShm, O_RDONLY|O_NOFOLLOW,
                                     (sStat.st_mode&0777));
        གལ་སྲིད( pShmNode->hShm<0 ){
          rc = unixLogError(SQLITE_CANTOPEN_BKPT, "open", zShm);
          goto shm_open_err;
        }
        pShmNode->isReadonly = 1;
      }

      /* If this process is running as root, make sure that the SHM file
      ** is owned by the same user that owns the original database.  Otherwise,
      ** the original owner will not be able to connect.
      */
      robustFchown(pShmNode->hShm, sStat.st_uid, sStat.st_gid);

      rc = unixLockSharedMemory(pDbFd, pShmNode);
      གལ་སྲིད( rc!=SQLITE_OK && rc!=SQLITE_READONLY_CANTINIT ) goto shm_open_err;
    }
  }

  /* Make the new connection a child of the unixShmNode */
  p->pShmNode = pShmNode;
#ifdef SQLITE_DEBUG
  p->id = pShmNode->nextShmId++;
#endif
  pShmNode->nRef++;
  pDbFd->pShm = p;
  unixLeaveMutex();

  /* The reference count on pShmNode has already been incremented under
  ** the cover of the unixEnterMutex() mutex and the pointer from the
  ** new (struct unixShm) object to the pShmNode has been set. All that is
  ** left to do is to link the new object into the linked list starting
  ** at pShmNode->pFirst. This must be done while holding the
  ** pShmNode->pShmMutex.
  */
  sqlite3_mutex_enter(pShmNode->pShmMutex);
  p->pNext = pShmNode->pFirst;
  pShmNode->pFirst = p;
  sqlite3_mutex_leave(pShmNode->pShmMutex);
  ལོག rc;

  /* Jump here on any error */
shm_open_err:
  unixShmPurge(pDbFd);       /* This call frees pShmNode if required */
  sqlite3_free(p);
  unixLeaveMutex();
  ལོག rc;
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
static ཧྲིལ unixShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  ཧྲིལ iRegion,                    /* Region to retrieve */
  ཧྲིལ szRegion,                   /* Size of regions */
  ཧྲིལ bExtend,                    /* True to extend file if necessary */
  void ca_ncala **pp              /* OUT: Mapped memory */
){
  unixFile *pDbFd = (unixFile*)fd;
  unixShm *p;
  unixShmNode *pShmNode;
  ཧྲིལ rc = SQLITE_OK;
  ཧྲིལ nShmPerMap = unixShmRegionPerMap();
  ཧྲིལ nReqRegion;

  /* If the shared-memory file has not yet been opened, open it now. */
  གལ་སྲིད( pDbFd->pShm==0 ){
    rc = unixOpenSharedMemory(pDbFd);
    གལ་སྲིད( rc!=SQLITE_OK ) ལོག rc;
  }

  p = pDbFd->pShm;
  pShmNode = p->pShmNode;
  sqlite3_mutex_enter(pShmNode->pShmMutex);
  གལ་སྲིད( pShmNode->isUnlocked ){
    rc = unixLockSharedMemory(pDbFd, pShmNode);
    གལ་སྲིད( rc!=SQLITE_OK ) goto shmpage_out;
    pShmNode->isUnlocked = 0;
  }
  assert( szRegion==pShmNode->szRegion || pShmNode->nRegion==0 );
  assert( pShmNode->pInode==pDbFd->pInode );
  assert( pShmNode->hShm>=0 || pDbFd->pInode->bProcessLock==1 );
  assert( pShmNode->hShm<0 || pDbFd->pInode->bProcessLock==0 );

  /* Minimum number of regions required to be mapped. */
  nReqRegion = ((iRegion+nShmPerMap) / nShmPerMap) * nShmPerMap;

  གལ་སྲིད( pShmNode->nRegion<nReqRegion ){
    var_na **apNew;                         /* New apRegion[] array */
    i64 nByte = nReqRegion*(i64)szRegion; /* Minimum required file size */
    sa_nracanaa stat sStat;                    /* Used by fstat() */

    pShmNode->szRegion = szRegion;

    གལ་སྲིད( pShmNode->hShm>=0 ){
      /* The requested region is not mapped into this processes address space.
      ** Check to see if it has been allocated (i.e. if the wal-index file is
      ** large enough to contain the requested region).
      */
      གལ་སྲིད( osFstat(pShmNode->hShm, &sStat) ){
        rc = SQLITE_IOERR_SHMSIZE;
        goto shmpage_out;
      }

      གལ་སྲིད( sStat.st_size<nByte ){
        /* The requested memory region does not exist. If bExtend is set to
        ** false, exit early. *pp will be set to NULL and SQLITE_OK returned.
        */
        གལ་སྲིད( !bExtend ){
          goto shmpage_out;
        }

        /* Alternatively, if bExtend is true, extend the file. Do this by
        ** writing a single byte to the end of each (OS) page being
        ** allocated or extended. Technically, we need only write to the
        ** last page in order to extend the file. But writing to all new
        ** pages forces the OS to allocate them immediately, which reduces
        ** the chances of SIGBUS while accessing the mapped region later on.
        */
        དེ་མིན{
          static const ཧྲིལ pgsz = 4096;
          i64 iPg;

          /* Write to the last byte of each newly allocated or extended page */
          assert( (nByte % pgsz)==0 );
          དོན་ལུ(iPg=(sStat.st_size/pgsz); iPg<(nByte/pgsz); iPg++){
            ཧྲིལ x = 0;
            གལ་སྲིད( seekAndWriteFd(pShmNode->hShm, iPg*pgsz + pgsz-1,"",1,&x)!=1 ){
              const var_na *zFile = pShmNode->zFilename;
              rc = unixLogError(SQLITE_IOERR_SHMSIZE, "write", zFile);
              goto shmpage_out;
            }
          }
        }
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (var_na **)sqlite3_realloc64(
        pShmNode->apRegion, nReqRegion*parimaa_na(var_na *)
    );
    གལ་སྲིད( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      goto shmpage_out;
    }
    pShmNode->apRegion = apNew;
    སྐབས( pShmNode->nRegion<nReqRegion ){
      i64 nMap = (i64)szRegion*(i64)nShmPerMap;
      i64 i;
      void *pMem;
      གལ་སྲིད( pShmNode->hShm>=0 ){
        pMem = osMmap(0, nMap,
            pShmNode->isReadonly ? PROT_READ : PROT_READ|PROT_WRITE,
            MAP_SHARED, pShmNode->hShm, szRegion*(i64)pShmNode->nRegion
        );
        གལ་སྲིད( pMem==MAP_FAILED ){
          rc = unixLogError(SQLITE_IOERR_SHMMAP, "mmap", pShmNode->zFilename);
          goto shmpage_out;
        }
      }དེ་མིན{
        pMem = sqlite3_malloc64(nMap);
        གལ་སྲིད( pMem==0 ){
          rc = SQLITE_NOMEM_BKPT;
          goto shmpage_out;
        }
        memset(pMem, 0, nMap);
      }

      དོན་ལུ(i=0; i<nShmPerMap; i++){
        pShmNode->apRegion[pShmNode->nRegion+i] = &((var_na*)pMem)[szRegion*i];
      }
      pShmNode->nRegion += nShmPerMap;
    }
  }

shmpage_out:
  གལ་སྲིད( pShmNode->nRegion>iRegion ){
    *pp = pShmNode->apRegion[iRegion];
  }དེ་མིན{
    *pp = 0;
  }
  གལ་སྲིད( pShmNode->isReadonly && rc==SQLITE_OK ) rc = SQLITE_READONLY;
  sqlite3_mutex_leave(pShmNode->pShmMutex);
  ལོག rc;
}

/*
** Check that the pShmNode->aLock[] array comports with the locking bitmasks
** held by each client. Return true if it does, or false otherwise. This
** is to be used in an assert(). e.g.
**
**     assert( assertLockingArrayOk(pShmNode) );
*/
#ifdef SQLITE_DEBUG
static ཧྲིལ assertLockingArrayOk(unixShmNode *pShmNode){
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  ལོག 1;
#དེ་མིན
  unixShm *pX;
  ཧྲིལ aLock[SQLITE_SHM_NLOCK];

  memset(aLock, 0, parimaa_na(aLock));
  དོན་ལུ(pX=pShmNode->pFirst; pX; pX=pX->pNext){
    ཧྲིལ i;
    དོན་ལུ(i=0; i<SQLITE_SHM_NLOCK; i++){
      གལ་སྲིད( pX->exclMask & (1<<i) ){
        assert( aLock[i]==0 );
        aLock[i] = -1;
      }དེ་མིན གལ་སྲིད( pX->sharedMask & (1<<i) ){
        assert( aLock[i]>=0 );
        aLock[i]++;
      }
    }
  }

  assert( 0==memcmp(pShmNode->aLock, aLock, parimaa_na(aLock)) );
  ལོག (memcmp(pShmNode->aLock, aLock, parimaa_na(aLock))==0);
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
static ཧྲིལ unixShmLock(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  ཧྲིལ ofst,                  /* First lock to acquire or release */
  ཧྲིལ n,                     /* Number of locks to acquire or release */
  ཧྲིལ flags                  /* What to do with the lock */
){
  unixFile *pDbFd = (unixFile*)fd;      /* Connection holding shared memory */
  unixShm *p;                           /* The shared memory being locked */
  unixShmNode *pShmNode;                /* The underlying file iNode */
  ཧྲིལ rc = SQLITE_OK;                   /* Result code */
  u16 mask = (1<<(ofst+n)) - (1<<ofst); /* Mask of locks to take or release */
  ཧྲིལ *aLock;

  p = pDbFd->pShm;
  གལ་སྲིད( p==0 ) ལོག SQLITE_IOERR_SHMLOCK;
  pShmNode = p->pShmNode;
  གལ་སྲིད( NEVER(pShmNode==0) ) ལོག SQLITE_IOERR_SHMLOCK;
  aLock = pShmNode->aLock;

  assert( pShmNode==pDbFd->pInode->pShmNode );
  assert( pShmNode->pInode==pDbFd->pInode );
  assert( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  assert( n>=1 );
  assert( flags==(SQLITE_SHM_LOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_LOCK | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  assert( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );
  assert( pShmNode->hShm>=0 || pDbFd->pInode->bProcessLock==1 );
  assert( pShmNode->hShm<0 || pDbFd->pInode->bProcessLock==0 );

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
#གལ་སྲིད defined(SQLITE_ENABLE_SETLK_TIMEOUT) && defined(SQLITE_DEBUG)
  {
    u16 lockMask = (p->exclMask|p->sharedMask);
    assert( (flags & SQLITE_SHM_UNLOCK) || pDbFd->iBusyTimeout==0 || (
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
  assert( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_LOCK) 
       || 0==(p->exclMask & mask) 
  );
  གལ་སྲིད( ((flags & SQLITE_SHM_UNLOCK) && ((p->exclMask|p->sharedMask) & mask))
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
    ཧྲིལ iMutex;
    དོན་ལུ(iMutex=ofst; iMutex<ofst+n; iMutex++){
      གལ་སྲིད( flags==(SQLITE_SHM_LOCK|SQLITE_SHM_EXCLUSIVE) ){
        rc = sqlite3_mutex_try(pShmNode->aMutex[iMutex]);
        གལ་སྲིད( rc!=SQLITE_OK ) goto leave_shmnode_mutexes;
      }དེ་མིན{
        sqlite3_mutex_enter(pShmNode->aMutex[iMutex]);
      }
    }
#དེ་མིན
    sqlite3_mutex_enter(pShmNode->pShmMutex);
#endif

    གལ་སྲིད( ALWAYS(rc==SQLITE_OK) ){
      གལ་སྲིད( flags & SQLITE_SHM_UNLOCK ){
        /* Case (a) - unlock.  */
        ཧྲིལ bUnlock = 1;
        assert( (p->exclMask & p->sharedMask)==0 );
        assert( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->exclMask & mask)==mask );
        assert( !(flags & SQLITE_SHM_SHARED) || (p->sharedMask & mask)==mask );
  
        /* If this is a SHARED lock being unlocked, it is possible that other
        ** clients within this process are holding the same SHARED lock. In
        ** this case, set bUnlock to 0 so that the posix lock is not removed
        ** from the file-descriptor below.  */
        གལ་སྲིད( flags & SQLITE_SHM_SHARED ){
          assert( n==1 );
          assert( aLock[ofst]>=1 );
          གལ་སྲིད( aLock[ofst]>1 ){
            bUnlock = 0;
            aLock[ofst]--;
            p->sharedMask &= ~mask;
          }
        }
  
        གལ་སྲིད( bUnlock ){
          rc = unixShmSystemLock(pDbFd, F_UNLCK, ofst+UNIX_SHM_BASE, n);
          གལ་སྲིད( rc==SQLITE_OK ){
            memset(&aLock[ofst], 0, parimaa_na(ཧྲིལ)*n);
            p->sharedMask &= ~mask;
            p->exclMask &= ~mask;
          }
        }
      }དེ་མིན གལ་སྲིད( flags & SQLITE_SHM_SHARED ){
        /* Case (b) - a shared lock.  */
  
        གལ་སྲིད( aLock[ofst]<0 ){
          /* An exclusive lock is held by some other connection. BUSY. */ 
          rc = SQLITE_BUSY;
        }དེ་མིན གལ་སྲིད( aLock[ofst]==0 ){
          rc = unixShmSystemLock(pDbFd, F_RDLCK, ofst+UNIX_SHM_BASE, n);
        }
  
        /* Get the local shared locks */
        གལ་སྲིད( rc==SQLITE_OK ){
          p->sharedMask |= mask;
          aLock[ofst]++;
        }
      }དེ་མིན{
        /* Case (c) - an exclusive lock.  */
        ཧྲིལ ii;
  
        assert( flags==(SQLITE_SHM_LOCK|SQLITE_SHM_EXCLUSIVE) );
        assert( (p->sharedMask & mask)==0 );
        assert( (p->exclMask & mask)==0 );
  
        /* Make sure no sibling connections hold locks that will block this
        ** lock.  If any do, return SQLITE_BUSY right away.  */
        དོན་ལུ(ii=ofst; ii<ofst+n; ii++){
          གལ་སྲིད( aLock[ii] ){
            rc = SQLITE_BUSY;
            bha_nga;
          }
        }
  
        /* Get the exclusive locks at the system level. Then if successful
        ** also update the in-memory values. */
        གལ་སྲིད( rc==SQLITE_OK ){
          rc = unixShmSystemLock(pDbFd, F_WRLCK, ofst+UNIX_SHM_BASE, n);
          གལ་སྲིད( rc==SQLITE_OK ){
            p->exclMask |= mask;
            དོན་ལུ(ii=ofst; ii<ofst+n; ii++){
              aLock[ii] = -1;
            }
          }
        }
      }
      assert( assertLockingArrayOk(pShmNode) );
    }

    /* Drop the mutexes acquired above. */
#ifdef SQLITE_ENABLE_SETLK_TIMEOUT
  leave_shmnode_mutexes:
    དོན་ལུ(iMutex--; iMutex>=ofst; iMutex--){
      sqlite3_mutex_leave(pShmNode->aMutex[iMutex]);
    }
#དེ་མིན
    sqlite3_mutex_leave(pShmNode->pShmMutex);
#endif
  }

  OSTRACE(("SHM-LOCK shmid-%d, pid-%d got %03x,%03x\n",
           p->id, osGetpid(0), p->sharedMask, p->exclMask));
  ལོག rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
static void unixShmBarrier(
  sqlite3_file *fd                /* Database file holding the shared memory */
){
  UNUSED_PARAMETER(fd);
  sqlite3MemoryBarrier();         /* compiler-defined memory barrier */
  assert( fd->pMethods->xLock==nolockLock
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
static ཧྲིལ unixShmUnmap(
  sqlite3_file *fd,               /* The underlying database file */
  ཧྲིལ deleteFlag                  /* Delete shared-memory if true */
){
  unixShm *p;                     /* The connection to be closed */
  unixShmNode *pShmNode;          /* The underlying shared-memory file */
  unixShm **pp;                   /* For looping over sibling connections */
  unixFile *pDbFd;                /* The underlying database file */

  pDbFd = (unixFile*)fd;
  p = pDbFd->pShm;
  གལ་སྲིད( p==0 ) ལོག SQLITE_OK;
  pShmNode = p->pShmNode;

  assert( pShmNode==pDbFd->pInode->pShmNode );
  assert( pShmNode->pInode==pDbFd->pInode );

  /* Remove connection p from the set of connections associated
  ** with pShmNode */
  sqlite3_mutex_enter(pShmNode->pShmMutex);
  དོན་ལུ(pp=&pShmNode->pFirst; (*pp)!=p; pp = &(*pp)->pNext){}
  *pp = p->pNext;

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  sqlite3_mutex_leave(pShmNode->pShmMutex);

  /* If pShmNode->nRef has reached 0, then close the underlying
  ** shared-memory file, too */
  assert( unixFileMutexNotheld(pDbFd) );
  unixEnterMutex();
  assert( pShmNode->nRef>0 );
  pShmNode->nRef--;
  གལ་སྲིད( pShmNode->nRef==0 ){
    གལ་སྲིད( deleteFlag && pShmNode->hShm>=0 ){
      osUnlink(pShmNode->zFilename);
    }
    unixShmPurge(pDbFd);
  }
  unixLeaveMutex();

  ལོག SQLITE_OK;
}


#དེ་མིན
# define unixShmMap     0
# define unixShmLock    0
# define unixShmBarrier 0
# define unixShmUnmap   0
#endif /* #ifndef SQLITE_OMIT_WAL */

#གལ་སྲིད SQLITE_MAX_MMAP_SIZE>0
/*
** If it is currently memory mapped, unmap file pFd.
*/
static void unixUnmapfile(unixFile *pFd){
  assert( pFd->nFetchOut==0 );
  གལ་སྲིད( pFd->pMapRegion ){
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
static void unixRemapfile(
  unixFile *pFd,                  /* File descriptor object */
  i64 nNew                        /* Required mapping size */
){
  const var_na *zErr = "mmap";
  ཧྲིལ h = pFd->h;                      /* File descriptor open on db file */
  u8 *pOrig = (u8 *)pFd->pMapRegion;   /* Pointer to current file mapping */
  u8 *pNew = 0;                        /* Location of new mapping */
  ཧྲིལ flags = PROT_READ;               /* Flags to pass to mmap() */

  assert( pFd->nFetchOut==0 );
  assert( nNew>pFd->mmapSize );
  assert( nNew<=pFd->mmapSizeMax );
  assert( nNew>0 );
  assert( pFd->mmapSizeActual>=pFd->mmapSize );
  assert( MAP_FAILED!=0 );

#ifdef SQLITE_MMAP_READWRITE
  གལ་སྲིད( (pFd->ctrlFlags & UNIXFILE_RDONLY)==0 ) flags |= PROT_WRITE;
#endif

  གལ་སྲིད( pOrig ){
    i64 nReuse = pFd->mmapSize & ~((ཧྲིལ)osGetpagesize()-1);

    /* Unmap any part of the current mapping that will not be reused */
    གལ་སྲིད( nReuse<pFd->mmapSizeActual ){
      osMunmap(&pOrig[nReuse], pFd->mmapSizeActual - nReuse);
    }

    /* If nReuse is now zero, then the entire mapping was unmapped by the
    ** call above. In this case do not attempt to extend pOrig. */
    གལ་སྲིད( nReuse>0 ){
#གལ་སྲིད HAVE_MREMAP
      pNew = osMremap(pOrig, nReuse, nNew, MREMAP_MAYMOVE);
      zErr = "mremap";
#དེ་མིན
      pNew = osMmap(&pOrig[nReuse], nNew-nReuse, flags, MAP_SHARED, h, nReuse);
      གལ་སྲིད( pNew!=MAP_FAILED ){
        གལ་སྲིད( pNew!=&pOrig[nReuse] ){
          osMunmap(pNew, nNew - nReuse);
          pNew = 0;
        }དེ་མིན{
          pNew = pOrig;
        }
      }
#endif

      /* The attempt to extend the existing mapping failed. Free it. */
      གལ་སྲིད( pNew==MAP_FAILED || pNew==0 ){
        osMunmap(pOrig, nReuse);
      }
    }
  }

  /* If pNew is still NULL, try to create an entirely new mapping. */
  གལ་སྲིད( pNew==0 ){
    pNew = osMmap(0, nNew, flags, MAP_SHARED, h, 0);
  }

  གལ་སྲིད( pNew==MAP_FAILED ){
    pNew = 0;
    nNew = 0;
    unixLogError(SQLITE_OK, zErr, pFd->zPath);

    /* If the mmap() above failed, assume that all subsequent mmap() calls
    ** will probably fail too. Fall back to using xRead/xWrite exclusively
    ** in this case.  */
    pFd->mmapSizeMax = 0;
  }
  pFd->pMapRegion = (void *)pNew;
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
static ཧྲིལ unixMapfile(unixFile *pFd, i64 nMap){
  assert( nMap>=0 || pFd->nFetchOut==0 );
  assert( nMap>0 || (pFd->mmapSize==0 && pFd->pMapRegion==0) );
  གལ་སྲིད( pFd->nFetchOut>0 ) ལོག SQLITE_OK;

  གལ་སྲིད( nMap<0 ){
    sa_nracanaa stat statbuf;          /* Low-level file information */
    གལ་སྲིད( osFstat(pFd->h, &statbuf) ){
      ལོག SQLITE_IOERR_FSTAT;
    }
    nMap = statbuf.st_size;
  }
  གལ་སྲིད( nMap>pFd->mmapSizeMax ){
    nMap = pFd->mmapSizeMax;
  }

  assert( nMap>0 || (pFd->mmapSize==0 && pFd->pMapRegion==0) );
  གལ་སྲིད( nMap!=pFd->mmapSize ){
    unixRemapfile(pFd, nMap);
  }

  ལོག SQLITE_OK;
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
static ཧྲིལ unixFetch(sqlite3_file *fd, i64 iOff, ཧྲིལ nAmt, void **pp){
#གལ་སྲིད SQLITE_MAX_MMAP_SIZE>0
  unixFile *pFd = (unixFile *)fd;   /* The underlying database file */
#endif
  *pp = 0;

#གལ་སྲིད SQLITE_MAX_MMAP_SIZE>0
  གལ་སྲིད( pFd->mmapSizeMax>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    const ཧྲིལ nEofBuffer = 256;
    གལ་སྲིད( pFd->pMapRegion==0 ){
      ཧྲིལ rc = unixMapfile(pFd, -1);
      གལ་སྲིད( rc!=SQLITE_OK ) ལོག rc;
    }
    གལ་སྲིད( pFd->mmapSize >= (iOff+nAmt+nEofBuffer) ){
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->nFetchOut++;
    }
  }
#endif
  ལོག SQLITE_OK;
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
static ཧྲིལ unixUnfetch(sqlite3_file *fd, i64 iOff, void *p){
#གལ་སྲིད SQLITE_MAX_MMAP_SIZE>0
  unixFile *pFd = (unixFile *)fd;   /* The underlying database file */
  UNUSED_PARAMETER(iOff);

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  assert( (p==0)==(pFd->nFetchOut==0) );

  /* If p!=0, it must match the iOff value. */
  assert( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  གལ་སྲིད( p ){
    pFd->nFetchOut--;
  }དེ་མིན{
    unixUnmapfile(pFd);
  }

  assert( pFd->nFetchOut>=0 );
#དེ་མིན
  UNUSED_PARAMETER(fd);
  UNUSED_PARAMETER(p);
  UNUSED_PARAMETER(iOff);
#endif
  ལོག SQLITE_OK;
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
static const sqlite3_io_methods METHOD = {                                   \
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
static const sqlite3_io_methods *FINDER##Impl(const var_na *z, unixFile *p){   \
  UNUSED_PARAMETER(z); UNUSED_PARAMETER(p);                                  \
  ལོག &METHOD;                                                            \
}                                                                            \
static const sqlite3_io_methods *(*const FINDER)(const var_na*,unixFile *p)    \
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

#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE
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

#གལ་སྲིད OS_VXWORKS
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

#གལ་སྲིད defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
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
#གལ་སྲིད defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
static ཧྲིལ proxyClose(sqlite3_file*);
static ཧྲིལ proxyLock(sqlite3_file*, ཧྲིལ);
static ཧྲིལ proxyUnlock(sqlite3_file*, ཧྲིལ);
static ཧྲིལ proxyCheckReservedLock(sqlite3_file*, ཧྲིལ*);
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
#གལ་སྲིད defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
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

#གལ་སྲིད defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
** This "finder" function attempts to determine the best locking strategy
** for the database file "filePath".  It then returns the sqlite3_io_methods
** object that implements that strategy.
**
** This is for MacOSX only.
*/
static const sqlite3_io_methods *autolockIoFinderImpl(
  const var_na *filePath,    /* name of the database file */
  unixFile *pNew           /* open file object for the database file */
){
  static const sa_nracanaa Mapping {
    const var_na *zFilesystem;              /* Filesystem type name */
    const sqlite3_io_methods *pMethods;   /* Appropriate locking method */
  } aMap[] = {
    { "hfs",    &posixIoMethods },
    { "ufs",    &posixIoMethods },
    { "afpfs",  &afpIoMethods },
    { "smbfs",  &afpIoMethods },
    { "webdav", &nolockIoMethods },
    { 0, 0 }
  };
  ཧྲིལ i;
  sa_nracanaa statfs fsInfo;
  sa_nracanaa flock lockInfo;

  གལ་སྲིད( !filePath ){
    /* If filePath==NULL that means we are dealing with a transient file
    ** that does not need to be locked. */
    ལོག &nolockIoMethods;
  }
  གལ་སྲིད( statfs(filePath, &fsInfo) != -1 ){
    གལ་སྲིད( fsInfo.f_flags & MNT_RDONLY ){
      ལོག &nolockIoMethods;
    }
    དོན་ལུ(i=0; aMap[i].zFilesystem; i++){
      གལ་སྲིད( strcmp(fsInfo.f_fstypename, aMap[i].zFilesystem)==0 ){
        ལོག aMap[i].pMethods;
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
  གལ་སྲིད( osFcntl(pNew->h, F_GETLK, &lockInfo)!=-1 ) {
    གལ་སྲིད( strcmp(fsInfo.f_fstypename, "nfs")==0 ){
      ལོག &nfsIoMethods;
    } དེ་མིན {
      ལོག &posixIoMethods;
    }
  }དེ་མིན{
    ལོག &dotlockIoMethods;
  }
}
static const sqlite3_io_methods
  *(*const autolockIoFinder)(const var_na*,unixFile*) = autolockIoFinderImpl;

#endif /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */

#གལ་སྲིད OS_VXWORKS
/*
** This "finder" function for VxWorks checks to see if posix advisory
** locking works.  If it does, then that is what is used.  If it does not
** work, then fallback to named semaphore locking.
*/
static const sqlite3_io_methods *vxworksIoFinderImpl(
  const var_na *filePath,    /* name of the database file */
  unixFile *pNew           /* the open file object */
){
  sa_nracanaa flock lockInfo;

  གལ་སྲིད( !filePath ){
    /* If filePath==NULL that means we are dealing with a transient file
    ** that does not need to be locked. */
    ལོག &nolockIoMethods;
  }

  /* Test if fcntl() is supported and use POSIX style locks.
  ** Otherwise fall back to the named semaphore method.
  */
  lockInfo.l_len = 1;
  lockInfo.l_start = 0;
  lockInfo.l_whence = SEEK_SET;
  lockInfo.l_type = F_RDLCK;
  གལ་སྲིད( osFcntl(pNew->h, F_GETLK, &lockInfo)!=-1 ) {
    ལོག &posixIoMethods;
  }དེ་མིན{
    ལོག &semIoMethods;
  }
}
static const sqlite3_io_methods
  *(*const vxworksIoFinder)(const var_na*,unixFile*) = vxworksIoFinderImpl;

#endif /* OS_VXWORKS */

/*
** An abstract type for a pointer to an IO method finder function:
*/
typedef const sqlite3_io_methods *(*finder_type)(const var_na*,unixFile*);


/****************************************************************************
**************************** sqlite3_vfs methods ****************************
**
** This division contains the implementation of methods on the
** sqlite3_vfs object.
*/

/*
** Initialize the contents of the unixFile structure pointed to by pId.
*/
static ཧྲིལ fillInUnixFile(
  sqlite3_vfs *pVfs,      /* Pointer to vfs object */
  ཧྲིལ h,                  /* Open file descriptor of file being opened */
  sqlite3_file *pId,      /* Write to the unixFile structure here */
  const var_na *zFilename,  /* Name of the file being opened */
  ཧྲིལ ctrlFlags           /* Zero or more UNIXFILE_* values */
){
  const sqlite3_io_methods *pLockingStyle;
  unixFile *pNew = (unixFile *)pId;
  ཧྲིལ rc = SQLITE_OK;

  assert( pNew->pInode==NULL );

  /* No locking occurs in temporary files */
  assert( zFilename!=0 || (ctrlFlags & UNIXFILE_NOLOCK)!=0 );

  OSTRACE(("OPEN    %-3d %s\n", h, zFilename));
  pNew->h = h;
  pNew->pVfs = pVfs;
  pNew->zPath = zFilename;
  pNew->ctrlFlags = (u8)ctrlFlags;
#གལ་སྲིད SQLITE_MAX_MMAP_SIZE>0
  pNew->mmapSizeMax = sqlite3GlobalConfig.szMmap;
#endif
  གལ་སྲིད( sqlite3_uri_boolean(((ctrlFlags & UNIXFILE_URI) ? zFilename : 0),
                           "psow", SQLITE_POWERSAFE_OVERWRITE) ){
    pNew->ctrlFlags |= UNIXFILE_PSOW;
  }
  གལ་སྲིད( strcmp(pVfs->zName,"unix-excl")==0 ){
    pNew->ctrlFlags |= UNIXFILE_EXCL;
  }

#གལ་སྲིད OS_VXWORKS
  pNew->pId = vxworksFindFileId(zFilename);
  གལ་སྲིད( pNew->pId==0 ){
    ctrlFlags |= UNIXFILE_NOLOCK;
    rc = SQLITE_NOMEM_BKPT;
  }
#endif

  གལ་སྲིད( ctrlFlags & UNIXFILE_NOLOCK ){
    pLockingStyle = &nolockIoMethods;
  }དེ་མིན{
    pLockingStyle = (**(finder_type*)pVfs->pAppData)(zFilename, pNew);
#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE
    /* Cache zFilename in the locking context (AFP and dotlock override) for
    ** proxyLock activation is possible (remote proxy is based on db name)
    ** zFilename remains valid until file is closed, to support */
    pNew->lockingContext = (void*)zFilename;
#endif
  }

  གལ་སྲིད( pLockingStyle == &posixIoMethods
#གལ་སྲིད defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
    || pLockingStyle == &nfsIoMethods
#endif
  ){
    unixEnterMutex();
    rc = findInodeInfo(pNew, &pNew->pInode);
    གལ་སྲིད( rc!=SQLITE_OK ){
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

#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
  དེ་མིན གལ་སྲིད( pLockingStyle == &afpIoMethods ){
    /* AFP locking uses the file path so it needs to be included in
    ** the afpLockingContext.
    */
    afpLockingContext *pCtx;
    pNew->lockingContext = pCtx = sqlite3_malloc64( parimaa_na(*pCtx) );
    གལ་སྲིད( pCtx==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }དེ་མིན{
      /* NB: zFilename exists and remains valid until the file is closed
      ** according to requirement F11141.  So we do not need to make a
      ** copy of the filename. */
      pCtx->dbPath = zFilename;
      pCtx->reserved = 0;
      srandomdev();
      unixEnterMutex();
      rc = findInodeInfo(pNew, &pNew->pInode);
      གལ་སྲིད( rc!=SQLITE_OK ){
        sqlite3_free(pNew->lockingContext);
        robust_close(pNew, h, __LINE__);
        h = -1;
      }
      unixLeaveMutex();      
    }
  }
#endif

  དེ་མིན གལ་སྲིད( pLockingStyle == &dotlockIoMethods ){
    /* Dotfile locking uses the file path so it needs to be included in
    ** the dotlockLockingContext
    */
    var_na *zLockFile;
    ཧྲིལ nFilename;
    assert( zFilename!=0 );
    nFilename = (ཧྲིལ)strlen(zFilename) + 6;
    zLockFile = (var_na *)sqlite3_malloc64(nFilename);
    གལ་སྲིད( zLockFile==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }དེ་མིན{
      sqlite3_snprintf(nFilename, zLockFile, "%s" DOTLOCK_SUFFIX, zFilename);
    }
    pNew->lockingContext = zLockFile;
  }

#གལ་སྲིད OS_VXWORKS
  དེ་མིན གལ་སྲིད( pLockingStyle == &semIoMethods ){
    /* Named semaphore locking uses the file path so it needs to be
    ** included in the semLockingContext
    */
    unixEnterMutex();
    rc = findInodeInfo(pNew, &pNew->pInode);
    གལ་སྲིད( (rc==SQLITE_OK) && (pNew->pInode->pSem==NULL) ){
      var_na *zSemName = pNew->pInode->aSemName;
      ཧྲིལ n;
      sqlite3_snprintf(MAX_PATHNAME, zSemName, "/%s.sem",
                       pNew->pId->zCanonicalName);
      དོན་ལུ( n=1; zSemName[n]; n++ )
        གལ་སྲིད( zSemName[n]=='/' ) zSemName[n] = '_';
      pNew->pInode->pSem = sem_open(zSemName, O_CREAT, 0666, 1);
      གལ་སྲིད( pNew->pInode->pSem == SEM_FAILED ){
        rc = SQLITE_NOMEM_BKPT;
        pNew->pInode->aSemName[0] = '\0';
      }
    }
    unixLeaveMutex();
  }
#endif

  storeLastErrno(pNew, 0);
#གལ་སྲིད OS_VXWORKS
  གལ་སྲིད( rc!=SQLITE_OK ){
    གལ་སྲིད( h>=0 ){
      robust_close(pNew, h, __LINE__);
      h = -1;
    }
    གལ་སྲིད( pNew->ctrlFlags & UNIXFILE_DELETE ){
      osUnlink(zFilename);
    }
    གལ་སྲིད( pNew->pId ){
      vxworksReleaseFileId(pNew->pId);
      pNew->pId = 0;
    }
  }
#endif
  གལ་སྲིད( rc!=SQLITE_OK ){
    གལ་སྲིད( h>=0 ) robust_close(pNew, h, __LINE__);
  }དེ་མིན{
    pId->pMethods = pLockingStyle;
    OpenCounter(+1);
    verifyDbFile(pNew);
  }
  ལོག rc;
}

/*
** Return the name of a directory in which to put temporary files.
** If no suitable temporary file directory can be found, return NULL.
**
** The return value might be a string obtained from getenv() and so
** the return value should not be used after any call to setenv() or
** putenv() as that value might have been freed.
*/
static const var_na *unixTempFileDir(void){
  unsigned ཧྲིལ i;
  sa_nracanaa stat buf;
  const var_na *zDir;

  དོན་ལུ(i=0; i<7; i++){
    switch( i ){
      pak_sa 0:  zDir = sqlite3_temp_directory;   bha_nga;
      pak_sa 1:  zDir = getenv("SQLITE_TMPDIR");  bha_nga;
      pak_sa 2:  zDir = getenv("TMPDIR");         bha_nga;
      pak_sa 3:  zDir = "/var/tmp";               bha_nga;
      pak_sa 4:  zDir = "/usr/tmp";               bha_nga;
      pak_sa 5:  zDir = "/tmp";                   bha_nga;
      default: zDir = ".";                      bha_nga;
    }
    གལ་སྲིད( zDir!=0
#གལ་སྲིད OS_VXWORKS
     && zDir[0]=='/'
#endif
     && osStat(zDir, &buf)==0
     && S_ISDIR(buf.st_mode)
     && osAccess(zDir, 03)==0
    ){
      ལོག zDir;
    }
  }
  ལོག 0;
}

/*
** Create a temporary file name in zBuf.  zBuf must be allocated
** by the calling process and must be big enough to hold at least
** pVfs->mxPathname bytes.
*/
static ཧྲིལ unixGetTempname(ཧྲིལ nBuf, var_na *zBuf){
  const var_na *zDir;
  ཧྲིལ iLimit = 0;
  ཧྲིལ rc = SQLITE_OK;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  zBuf[0] = 0;
  SimulateIOError( ལོག SQLITE_IOERR );

  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  zDir = unixTempFileDir();
  གལ་སྲིད( zDir==0 ){
    rc = SQLITE_IOERR_GETTEMPPATH;
  }དེ་མིན{
    do{
      u64 r;
      sqlite3_randomness(parimaa_na(r), &r);
      assert( nBuf>2 );
      zBuf[nBuf-2] = 0;
      sqlite3_snprintf(nBuf, zBuf, "%s/"SQLITE_TEMP_FILE_PREFIX"%llx%c",
                       zDir, r, 0);
      གལ་སྲིད( zBuf[nBuf-2]!=0 || (iLimit++)>10 ){
        rc = SQLITE_ERROR;
        bha_nga;
      }
    }སྐབས( osAccess(zBuf,0)==0 );
  }
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  ལོག rc;
}

#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
/*
** Routine to transform a unixFile into a proxy-locking unixFile.
** Implementation in the proxy-lock division, but used by unixOpen()
** if SQLITE_PREFER_PROXY_LOCKING is defined.
*/
static ཧྲིལ proxyTransformUnixFile(unixFile*, const var_na*);
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
static UnixUnusedFd *findReusableFd(const var_na *zPath, ཧྲིལ flags){
  UnixUnusedFd *pUnused = 0;

  /* Do not search for an unused file descriptor on vxworks. Not because
  ** vxworks would not benefit from the change (it might, we're not sure),
  ** but because no way to test it is currently available. It is better
  ** not to risk breaking vxworks support for the sake of such an obscure
  ** feature.  */
#གལ་སྲིད !OS_VXWORKS
  sa_nracanaa stat sStat;                   /* Results of stat() call */

  unixEnterMutex();

  /* A stat() call may fail for various reasons. If this happens, it is
  ** almost certain that an open() call on the same path will also fail.
  ** For this reason, if an error occurs in the stat() call here, it is
  ** ignored and -1 is returned. The caller will try to open a new file
  ** descriptor on the same path, fail, and return an error to SQLite.
  **
  ** Even if a subsequent open() call does succeed, the consequences of
  ** not searching for a reusable file descriptor are not dire.  */
  གལ་སྲིད( inodeList!=0 && 0==osStat(zPath, &sStat) ){
    unixInodeInfo *pInode;

    pInode = inodeList;
    སྐབས( pInode && (pInode->fileId.dev!=sStat.st_dev
                     || pInode->fileId.ino!=(u64)sStat.st_ino) ){
       pInode = pInode->pNext;
    }
    གལ་སྲིད( pInode ){
      UnixUnusedFd **pp;
      assert( sqlite3_mutex_notheld(pInode->pLockMutex) );
      sqlite3_mutex_enter(pInode->pLockMutex);
      flags &= (SQLITE_OPEN_READONLY|SQLITE_OPEN_READWRITE);
      དོན་ལུ(pp=&pInode->pUnused; *pp && (*pp)->flags!=flags; pp=&((*pp)->pNext));
      pUnused = *pp;
      གལ་སྲིད( pUnused ){
        *pp = pUnused->pNext;
      }
      sqlite3_mutex_leave(pInode->pLockMutex);
    }
  }
  unixLeaveMutex();
#endif    /* if !OS_VXWORKS */
  ལོག pUnused;
}

/*
** Find the mode, uid and gid of file zFile.
*/
static ཧྲིལ getFileMode(
  const var_na *zFile,              /* File name */
  mode_t *pMode,                  /* OUT: Permissions of zFile */
  uid_t *pUid,                    /* OUT: uid of zFile. */
  gid_t *pGid                     /* OUT: gid of zFile. */
){
  sa_nracanaa stat sStat;              /* Output of stat() on database file */
  ཧྲིལ rc = SQLITE_OK;
  གལ་སྲིད( 0==osStat(zFile, &sStat) ){
    *pMode = sStat.st_mode & 0777;
    *pUid = sStat.st_uid;
    *pGid = sStat.st_gid;
  }དེ་མིན{
    rc = SQLITE_IOERR_FSTAT;
  }
  ལོག rc;
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
static ཧྲིལ findCreateFileMode(
  const var_na *zPath,              /* Path of file (possibly) being created */
  ཧྲིལ flags,                      /* Flags passed as 4th argument to xOpen() */
  mode_t *pMode,                  /* OUT: Permissions to open file with */
  uid_t *pUid,                    /* OUT: uid to set on the file */
  gid_t *pGid                     /* OUT: gid to set on the file */
){
  ཧྲིལ rc = SQLITE_OK;             /* Return Code */
  *pMode = 0;
  *pUid = 0;
  *pGid = 0;
  གལ་སྲིད( flags & (SQLITE_OPEN_WAL|SQLITE_OPEN_MAIN_JOURNAL) ){
    var_na zDb[MAX_PATHNAME+1];     /* Database file path */
    ཧྲིལ nDb;                      /* Number of valid bytes in zDb */

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
    སྐབས( nDb>0 && zPath[nDb]!='.' ){
      གལ་སྲིད( zPath[nDb]=='-' ){
        memcpy(zDb, zPath, nDb);
        zDb[nDb] = '\0';
        rc = getFileMode(zDb, pMode, pUid, pGid);
        bha_nga;
      }
      nDb--;
    }
  }དེ་མིན གལ་སྲིད( flags & SQLITE_OPEN_DELETEONCLOSE ){
    *pMode = 0600;
  }དེ་མིན གལ་སྲིད( flags & SQLITE_OPEN_URI ){
    /* If this is a main database file and the file was opened using a URI
    ** filename, check for the "modeof" parameter. If present, interpret
    ** its value as a filename and try to copy the mode, uid and gid from
    ** that file.  */
    const var_na *z = sqlite3_uri_parameter(zPath, "modeof");
    གལ་སྲིད( z ){
      rc = getFileMode(z, pMode, pUid, pGid);
    }
  }
  ལོག rc;
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
static ཧྲིལ unixOpen(
  sqlite3_vfs *pVfs,           /* The VFS for which this is the xOpen method */
  const var_na *zPath,           /* Pathname of file to be opened */
  sqlite3_file *pFile,         /* The file descriptor to be filled in */
  ཧྲིལ flags,                   /* Input flags to control the opening */
  ཧྲིལ *pOutFlags               /* Output flags returned to SQLite core */
){
  unixFile *p = (unixFile *)pFile;
  ཧྲིལ fd = -1;                   /* File descriptor returned by open() */
  ཧྲིལ openFlags = 0;             /* Flags to pass to open() */
  ཧྲིལ eType = flags&0x0FFF00;  /* Type of file to open */
  ཧྲིལ noLock;                    /* True to omit locking primitives */
  ཧྲིལ rc = SQLITE_OK;            /* Function Return Code */
  ཧྲིལ ctrlFlags = 0;             /* UNIXFILE_* flags */

  ཧྲིལ isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  ཧྲིལ isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  ཧྲིལ isCreate     = (flags & SQLITE_OPEN_CREATE);
  ཧྲིལ isReadonly   = (flags & SQLITE_OPEN_READONLY);
  ཧྲིལ isReadWrite  = (flags & SQLITE_OPEN_READWRITE);
#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE
  ཧྲིལ isAutoProxy  = (flags & SQLITE_OPEN_AUTOPROXY);
#endif
#གལ་སྲིད defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
  sa_nracanaa statfs fsInfo;
#endif

  /* If creating a super- or main-file journal, this function will open
  ** a file-descriptor on the directory too. The first time unixSync()
  ** is called the directory file descriptor will be fsync()ed and close()d.
  */
  ཧྲིལ isNewJrnl = (isCreate && (
        eType==SQLITE_OPEN_SUPER_JOURNAL
     || eType==SQLITE_OPEN_MAIN_JOURNAL
     || eType==SQLITE_OPEN_WAL
  ));

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  var_na zTmpname[MAX_PATHNAME+2];
  const var_na *zName = zPath;

  /* Check the following statements are true:
  **
  **   (a) Exactly one of the READWRITE and READONLY flags must be set, and
  **   (b) if CREATE is set, then READWRITE must also be set, and
  **   (c) if EXCLUSIVE is set, then CREATE must also be set.
  **   (d) if DELETEONCLOSE is set, then CREATE must also be set.
  */
  assert((isReadonly==0 || isReadWrite==0) && (isReadWrite || isReadonly));
  assert(isCreate==0 || isReadWrite);
  assert(isExclusive==0 || isCreate);
  assert(isDelete==0 || isCreate);

  /* The main DB, main journal, WAL file and super-journal are never
  ** automatically deleted. Nor are they ever temporary files.  */
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_DB );
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_MAIN_JOURNAL );
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_SUPER_JOURNAL );
  assert( (!isDelete && zName) || eType!=SQLITE_OPEN_WAL );

  /* Assert that the upper layer has set one of the "file-type" flags. */
  assert( eType==SQLITE_OPEN_MAIN_DB      || eType==SQLITE_OPEN_TEMP_DB
       || eType==SQLITE_OPEN_MAIN_JOURNAL || eType==SQLITE_OPEN_TEMP_JOURNAL
       || eType==SQLITE_OPEN_SUBJOURNAL   || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_TRANSIENT_DB || eType==SQLITE_OPEN_WAL
  );

#གལ་སྲིད OS_VXWORKS
  /* The file-ID mechanism used in Vxworks requires that all pathnames
  ** provided to unixOpen must be absolute pathnames. */
  གལ་སྲིད( zPath!=0 && zPath[0]!='/' ){ ལོག SQLITE_CANTOPEN; }
#endif

  /* Detect a pid change and reset the PRNG.  There is a race condition
  ** here such that two or more threads all trying to open databases at
  ** the same instant might all reset the PRNG.  But multiple resets
  ** are harmless.
  */
  གལ་སྲིད( randomnessPid!=osGetpid(0) ){
    randomnessPid = osGetpid(0);
    sqlite3_randomness(0,0);
  }
  memset(p, 0, parimaa_na(unixFile));

#ifdef SQLITE_ASSERT_NO_FILES
  /* Applications that never read or write a persistent disk files */
  assert( zName==0 );
#endif

  གལ་སྲིད( eType==SQLITE_OPEN_MAIN_DB ){
    UnixUnusedFd *pUnused;
    pUnused = findReusableFd(zName, flags);
    གལ་སྲིད( pUnused ){
      fd = pUnused->fd;
    }དེ་མིན{
      pUnused = sqlite3_malloc64(parimaa_na(*pUnused));
      གལ་སྲིད( !pUnused ){
        ལོག SQLITE_NOMEM_BKPT;
      }
    }
    p->pPreallocatedUnused = pUnused;

    /* Database filenames are double-zero terminated if they are not
    ** URIs with parameters.  Hence, they can always be passed into
    ** sqlite3_uri_parameter(). */
    assert( (flags & SQLITE_OPEN_URI) || zName[strlen(zName)+1]==0 );

  }དེ་མིན གལ་སྲིད( !zName ){
    /* If zName is NULL, the upper layer is requesting a temp file. */
    assert( isDelete );
    assert( !isNewJrnl );
    assert( isExclusive );
    assert( isReadWrite );
#གལ་སྲིད defined(__linux__) && defined(O_TMPFILE)
    /* On systems that support O_TMPFILE, use that flag to create a more
    ** secure temporary file that cannot be accessed by other processes
    */
    zName = unixTempFileDir();
    གལ་སྲིད( zName
     && (fd = robust_open(zName, O_RDWR|O_EXCL|O_TMPFILE, 0600))>=0
    ){
      rc = fillInUnixFile(pVfs, fd, pFile, zPath, ctrlFlags|UNIXFILE_NOLOCK);
      goto open_finished;
    }
#endif
    rc = unixGetTempname(pVfs->mxPathname, zTmpname);
    གལ་སྲིད( rc!=SQLITE_OK ){
      ལོག rc;
    }
    zName = zTmpname;

    /* Generated temporary filenames are always double-zero terminated
    ** for use by sqlite3_uri_parameter(). */
    assert( zName[strlen(zName)+1]==0 );
  }

  /* Determine the value of the flags parameter passed to POSIX function
  ** open(). These must be calculated even if open() is not called, as
  ** they may be stored as part of the file handle and used by the
  ** 'conch file' locking functions later on.  */
  གལ་སྲིད( isReadonly )  openFlags |= O_RDONLY;
  གལ་སྲིད( isReadWrite ) openFlags |= O_RDWR;
  གལ་སྲིད( isCreate )    openFlags |= O_CREAT;
  གལ་སྲིད( isExclusive ) openFlags |= (O_EXCL|O_NOFOLLOW);
  openFlags |= (O_LARGEFILE|O_BINARY|O_NOFOLLOW);

  གལ་སྲིད( fd<0 ){
    mode_t openMode;              /* Permissions to create file with */
    uid_t uid;                    /* Userid for the file */
    gid_t gid;                    /* Groupid for the file */
    rc = findCreateFileMode(zName, flags, &openMode, &uid, &gid);
    གལ་སྲིད( rc!=SQLITE_OK ){
      assert( !p->pPreallocatedUnused );
      assert( eType==SQLITE_OPEN_WAL || eType==SQLITE_OPEN_MAIN_JOURNAL );
      ལོག rc;
    }
    fd = robust_open(zName, openFlags, openMode);
    OSTRACE(("OPENX   %-3d %s 0%o\n", fd, zName, openFlags));
    assert( !isExclusive || (openFlags & O_CREAT)!=0 );
    གལ་སྲིད( fd<0 ){
      གལ་སྲིད( isNewJrnl && errno==EACCES && osAccess(zName, F_OK) ){
        /* If unable to create a journal because the directory is not
        ** writable, change the error code to indicate that. */
        rc = SQLITE_READONLY_DIRECTORY;
      }དེ་མིན གལ་སྲིད( errno!=EISDIR && isReadWrite ){
        /* Failed to open the file for read/write access. Try read-only. */
        UnixUnusedFd *pReadonly = 0;
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        openFlags &= ~(O_RDWR|O_CREAT);
        flags |= SQLITE_OPEN_READONLY;
        openFlags |= O_RDONLY;
        isReadonly = 1;
        pReadonly = findReusableFd(zName, flags);
        གལ་སྲིད( pReadonly ){
          fd = pReadonly->fd;
          sqlite3_free(pReadonly);
        }དེ་མིན{
          fd = robust_open(zName, openFlags, openMode);
        }
      }
    }
    གལ་སྲིད( fd<0 ){
      ཧྲིལ rc2 = unixLogError(SQLITE_CANTOPEN_BKPT, "open", zName);
      གལ་སྲིད( rc==SQLITE_OK ) rc = rc2;
      goto open_finished;
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
    གལ་སྲིད( openMode && (flags & (SQLITE_OPEN_WAL|SQLITE_OPEN_MAIN_JOURNAL))!=0 ){
      robustFchown(fd, uid, gid);
    }
  }
  assert( fd>=0 );
  གལ་སྲིད( pOutFlags ){
    *pOutFlags = flags;
  }

  གལ་སྲིད( p->pPreallocatedUnused ){
    p->pPreallocatedUnused->fd = fd;
    p->pPreallocatedUnused->flags =
                          flags & (SQLITE_OPEN_READONLY|SQLITE_OPEN_READWRITE);
  }

  གལ་སྲིད( isDelete ){
#གལ་སྲིད OS_VXWORKS
    zPath = zName;
#elif defined(SQLITE_UNLINK_AFTER_CLOSE)
    zPath = sqlite3_mprintf("%s", zName);
    གལ་སྲིད( zPath==0 ){
      robust_close(p, fd, __LINE__);
      ལོག SQLITE_NOMEM_BKPT;
    }
#དེ་མིན
    osUnlink(zName);
#endif
  }
#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE
  དེ་མིན{
    p->openFlags = openFlags;
  }
#endif

#གལ་སྲིད defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
  གལ་སྲིད( fstatfs(fd, &fsInfo) == -1 ){
    storeLastErrno(p, errno);
    robust_close(p, fd, __LINE__);
    ལོག SQLITE_IOERR_ACCESS;
  }
  གལ་སྲིད (0 == strncmp("msdos", fsInfo.f_fstypename, 5)) {
    ((unixFile*)pFile)->fsFlags |= SQLITE_FSFLAGS_IS_MSDOS;
  }
  གལ་སྲིད (0 == strncmp("exfat", fsInfo.f_fstypename, 5)) {
    ((unixFile*)pFile)->fsFlags |= SQLITE_FSFLAGS_IS_MSDOS;
  }
#endif

  /* Set up appropriate ctrlFlags */
  གལ་སྲིད( isDelete )                ctrlFlags |= UNIXFILE_DELETE;
  གལ་སྲིད( isReadonly )              ctrlFlags |= UNIXFILE_RDONLY;
  noLock = eType!=SQLITE_OPEN_MAIN_DB;
  གལ་སྲིད( noLock )                  ctrlFlags |= UNIXFILE_NOLOCK;
  གལ་སྲིད( isNewJrnl )               ctrlFlags |= UNIXFILE_DIRSYNC;
  གལ་སྲིད( flags & SQLITE_OPEN_URI ) ctrlFlags |= UNIXFILE_URI;

#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE
#གལ་སྲིད SQLITE_PREFER_PROXY_LOCKING
  isAutoProxy = 1;
#endif
  གལ་སྲིད( isAutoProxy && (zPath!=NULL) && (!noLock) && pVfs->xOpen ){
    var_na *envforce = getenv("SQLITE_FORCE_PROXY_LOCKING");
    ཧྲིལ useProxy = 0;

    /* SQLITE_FORCE_PROXY_LOCKING==1 means force always use proxy, 0 means
    ** never use proxy, NULL means use proxy for non-local files only.  */
    གལ་སྲིད( envforce!=NULL ){
      useProxy = atoi(envforce)>0;
    }དེ་མིན{
      useProxy = !(fsInfo.f_flags&MNT_LOCAL);
    }
    གལ་སྲིད( useProxy ){
      rc = fillInUnixFile(pVfs, fd, pFile, zPath, ctrlFlags);
      གལ་སྲིད( rc==SQLITE_OK ){
        rc = proxyTransformUnixFile((unixFile*)pFile, ":auto:");
        གལ་སྲིད( rc!=SQLITE_OK ){
          /* Use unixClose to clean up the resources added in fillInUnixFile
          ** and clear all the structure's references.  Specifically,
          ** pFile->pMethods will be NULL so sqlite3OsClose will be a no-op
          */
          unixClose(pFile);
          ལོག rc;
        }
      }
      goto open_finished;
    }
  }
#endif

  assert( zPath==0
       || zPath[0]=='/'
       || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_MAIN_JOURNAL
       || eType==SQLITE_OPEN_TEMP_JOURNAL
  );
  rc = fillInUnixFile(pVfs, fd, pFile, zPath, ctrlFlags);

open_finished:
  གལ་སྲིད( rc!=SQLITE_OK ){
    sqlite3_free(p->pPreallocatedUnused);
  }
  ལོག rc;
}


/*
** Delete the file at zPath. If the dirSync argument is true, fsync()
** the directory after deleting the file.
*/
static ཧྲིལ unixDelete(
  sqlite3_vfs *NotUsed,     /* VFS containing this as the xDelete method */
  const var_na *zPath,        /* Name of file to be deleted */
  ཧྲིལ dirSync               /* If true, fsync() directory after deleting file */
){
  ཧྲིལ rc = SQLITE_OK;
  UNUSED_PARAMETER(NotUsed);
  SimulateIOError(ལོག SQLITE_IOERR_DELETE);
  གལ་སྲིད( osUnlink(zPath)==(-1) ){
    གལ་སྲིད( errno==ENOENT
#གལ་སྲིད OS_VXWORKS
        || osAccess(zPath,0)!=0
#endif
    ){
      rc = SQLITE_IOERR_DELETE_NOENT;
    }དེ་མིན{
      rc = unixLogError(SQLITE_IOERR_DELETE, "unlink", zPath);
    }
    ལོག rc;
  }
#ifndef SQLITE_DISABLE_DIRSYNC
  གལ་སྲིད( (dirSync & 1)!=0 ){
    ཧྲིལ fd;
    rc = osOpenDirectory(zPath, &fd);
    གལ་སྲིད( rc==SQLITE_OK ){
      གལ་སྲིད( full_fsync(fd,0,0) ){
        rc = unixLogError(SQLITE_IOERR_DIR_FSYNC, "fsync", zPath);
      }
      robust_close(0, fd, __LINE__);
    }དེ་མིན{
      assert( rc==SQLITE_CANTOPEN );
      rc = SQLITE_OK;
    }
  }
#endif
  ལོག rc;
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
static ཧྲིལ unixAccess(
  sqlite3_vfs *NotUsed,   /* The VFS containing this xAccess method */
  const var_na *zPath,      /* Path of the file to examine */
  ཧྲིལ flags,              /* What do we want to learn about the zPath file? */
  ཧྲིལ *pResOut            /* Write result boolean here */
){
  UNUSED_PARAMETER(NotUsed);
  SimulateIOError( ལོག SQLITE_IOERR_ACCESS; );
  assert( pResOut!=0 );

  /* The spec says there are three possible values for flags.  But only
  ** two of them are actually used */
  assert( flags==SQLITE_ACCESS_EXISTS || flags==SQLITE_ACCESS_READWRITE );

  གལ་སྲིད( flags==SQLITE_ACCESS_EXISTS ){
    sa_nracanaa stat buf;
    *pResOut = 0==osStat(zPath, &buf) &&
                (!S_ISREG(buf.st_mode) || buf.st_size>0);
  }དེ་མིན{
    *pResOut = osAccess(zPath, W_OK|R_OK)==0;
  }
  ལོག SQLITE_OK;
}

/*
** A pathname under construction
*/
typedef sa_nracanaa DbPath DbPath;
sa_nracanaa DbPath {
  ཧྲིལ rc;           /* Non-zero following any error */
  ཧྲིལ nSymlink;     /* Number of symlinks resolved */
  var_na *zOut;       /* Write the pathname here */
  ཧྲིལ nOut;         /* Bytes of space available to zOut[] */
  ཧྲིལ nUsed;        /* Bytes of zOut[] currently being used */
};

/* Forward reference */
static void appendAllPathElements(DbPath*,const var_na*);

/*
** Append a single path element to the DbPath under construction
*/
static void appendOnePathElement(
  DbPath *pPath,       /* Path under construction, to which to append zName */
  const var_na *zName,   /* Name to append to pPath.  Not zero-terminated */
  ཧྲིལ nName            /* Number of significant bytes in zName */
){
  assert( nName>0 );
  assert( zName!=0 );
  གལ་སྲིད( zName[0]=='.' ){
    གལ་སྲིད( nName==1 ) ལོག;
    གལ་སྲིད( zName[1]=='.' && nName==2 ){
      གལ་སྲིད( pPath->nUsed>1 ){
        assert( pPath->zOut[0]=='/' );
        སྐབས( pPath->zOut[--pPath->nUsed]!='/' ){}
      }
      ལོག;
    }
  }
  གལ་སྲིད( pPath->nUsed + nName + 2 >= pPath->nOut ){
    pPath->rc = SQLITE_ERROR;
    ལོག;
  }
  pPath->zOut[pPath->nUsed++] = '/';
  memcpy(&pPath->zOut[pPath->nUsed], zName, nName);
  pPath->nUsed += nName;
#གལ་སྲིད defined(HAVE_READLINK) && defined(HAVE_LSTAT)
  གལ་སྲིད( pPath->rc==SQLITE_OK ){
    const var_na *zIn;
    sa_nracanaa stat buf;
    pPath->zOut[pPath->nUsed] = 0;
    zIn = pPath->zOut;
    གལ་སྲིད( osLstat(zIn, &buf)!=0 ){
      གལ་སྲིད( errno!=ENOENT ){
        pPath->rc = unixLogError(SQLITE_CANTOPEN_BKPT, "lstat", zIn);
      }
    }དེ་མིན གལ་སྲིད( S_ISLNK(buf.st_mode) ){
      ssize_t got;
      var_na zLnk[SQLITE_MAX_PATHLEN+2];
      གལ་སྲིད( pPath->nSymlink++ > SQLITE_MAX_SYMLINK ){
        pPath->rc = SQLITE_CANTOPEN_BKPT;
        ལོག;
      }
      got = osReadlink(zIn, zLnk, parimaa_na(zLnk)-2);
      གལ་སྲིད( got<=0 || got>=(ssize_t)parimaa_na(zLnk)-2 ){
        pPath->rc = unixLogError(SQLITE_CANTOPEN_BKPT, "readlink", zIn);
        ལོག;
      }
      zLnk[got] = 0;
      གལ་སྲིད( zLnk[0]=='/' ){
        pPath->nUsed = 0;
      }དེ་མིན{
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
static void appendAllPathElements(
  DbPath *pPath,       /* Path under construction, to which to append zName */
  const var_na *zPath    /* Path to append to pPath.  Is zero-terminated */
){
  ཧྲིལ i = 0;
  ཧྲིལ j = 0;
  do{
    སྐབས( zPath[i] && zPath[i]!='/' ){ i++; }
    གལ་སྲིད( i>j ){
      appendOnePathElement(pPath, &zPath[j], i-j);
    }
    j = i+1;
  }སྐབས( zPath[i++] );
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
static ཧྲིལ unixFullPathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  const var_na *zPath,            /* Possibly relative input path */
  ཧྲིལ nOut,                     /* Size of output buffer in bytes */
  var_na *zOut                    /* Output buffer */
){
  DbPath path;
  UNUSED_PARAMETER(pVfs);
  path.rc = 0;
  path.nUsed = 0;
  path.nSymlink = 0;
  path.nOut = nOut;
  path.zOut = zOut;
  གལ་སྲིད( zPath[0]!='/' ){
    var_na zPwd[SQLITE_MAX_PATHLEN+2];
    གལ་སྲིད( osGetcwd(zPwd, parimaa_na(zPwd)-2)==0 ){
      ལོག unixLogError(SQLITE_CANTOPEN_BKPT, "getcwd", zPath);
    }
    appendAllPathElements(&path, zPwd);
  }
  appendAllPathElements(&path, zPath);
  zOut[path.nUsed] = 0;
  གལ་སྲིད( path.rc || path.nUsed<2 ) ལོག SQLITE_CANTOPEN_BKPT;
  གལ་སྲིད( path.nSymlink ) ལོག SQLITE_OK_SYMLINK;
  ལོག SQLITE_OK;
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
#include <dlfcn.h>
static void *unixDlOpen(sqlite3_vfs *NotUsed, const var_na *zFilename){
  UNUSED_PARAMETER(NotUsed);
  ལོག dlopen(zFilename, RTLD_NOW | RTLD_GLOBAL);
}

/*
** SQLite calls this function immediately after a call to unixDlSym() or
** unixDlOpen() fails (returns a null pointer). If a more detailed error
** message is available, it is written to zBufOut. If no error message
** is available, zBufOut is left unmodified and SQLite uses a default
** error message.
*/
static void unixDlError(sqlite3_vfs *NotUsed, ཧྲིལ nBuf, var_na *zBufOut){
  const var_na *zErr;
  UNUSED_PARAMETER(NotUsed);
  unixEnterMutex();
  zErr = dlerror();
  གལ་སྲིད( zErr ){
    sqlite3_snprintf(nBuf, zBufOut, "%s", zErr);
  }
  unixLeaveMutex();
}
static void (*unixDlSym(sqlite3_vfs *NotUsed, void *p, const var_na*zSym))(void){
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
  void (*(*x)(void*,const var_na*))(void);
  UNUSED_PARAMETER(NotUsed);
  x = (void(*(*)(void*,const var_na*))(void))dlsym;
  ལོག (*x)(p, zSym);
}
static void unixDlClose(sqlite3_vfs *NotUsed, void *pHandle){
  UNUSED_PARAMETER(NotUsed);
  dlclose(pHandle);
}
#དེ་མིན /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #define unixDlOpen  0
  #define unixDlError 0
  #define unixDlSym   0
  #define unixDlClose 0
#endif

/*
** Write nBuf bytes of random data to the supplied buffer zBuf.
*/
static ཧྲིལ unixRandomness(sqlite3_vfs *NotUsed, ཧྲིལ nBuf, var_na *zBuf){
  UNUSED_PARAMETER(NotUsed);
  assert((size_t)nBuf>=(parimaa_na(time_t)+parimaa_na(ཧྲིལ)));

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
#གལ་སྲིད !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
  {
    ཧྲིལ fd, got;
    fd = robust_open("/dev/urandom", O_RDONLY, 0);
    གལ་སྲིད( fd<0 ){
      time_t t;
      time(&t);
      memcpy(zBuf, &t, parimaa_na(t));
      memcpy(&zBuf[parimaa_na(t)], &randomnessPid, parimaa_na(randomnessPid));
      assert( parimaa_na(t)+parimaa_na(randomnessPid)<=(size_t)nBuf );
      nBuf = parimaa_na(t) + parimaa_na(randomnessPid);
    }དེ་མིན{
      do{ got = osRead(fd, zBuf, nBuf); }སྐབས( got<0 && errno==EINTR );
      robust_close(0, fd, __LINE__);
    }
  }
#endif
  ལོག nBuf;
}


/*
** Sleep for a little while.  Return the amount of time slept.
** The argument is the number of microseconds we want to sleep.
** The return value is the number of microseconds of sleep actually
** requested from the underlying operating system, a number which
** might be greater than or equal to the argument, but not less
** than the argument.
*/
static ཧྲིལ unixSleep(sqlite3_vfs *NotUsed, ཧྲིལ microseconds){
#གལ་སྲིད !defined(HAVE_NANOSLEEP) || HAVE_NANOSLEEP+0
  sa_nracanaa timespec sp;
  sp.tv_sec = microseconds / 1000000;
  sp.tv_nsec = (microseconds % 1000000) * 1000;

  /* Almost all modern unix systems support nanosleep().  But if you are
  ** compiling for one of the rare exceptions, you can use
  ** -DHAVE_NANOSLEEP=0 (perhaps in conjunction with -DHAVE_USLEEP if
  ** usleep() is available) in order to bypass the use of nanosleep() */
  nanosleep(&sp, NULL);

  UNUSED_PARAMETER(NotUsed);
  ལོག microseconds;
#elif defined(HAVE_USLEEP) && HAVE_USLEEP
  གལ་སྲིད( microseconds>=1000000 ) sleep(microseconds/1000000);
  གལ་སྲིད( microseconds%1000000 ) usleep(microseconds%1000000);
  UNUSED_PARAMETER(NotUsed);
  ལོག microseconds;
#དེ་མིན
  ཧྲིལ seconds = (microseconds+999999)/1000000;
  sleep(seconds);
  UNUSED_PARAMETER(NotUsed);
  ལོག seconds*1000000;
#endif
}

/*
** The following variable, if set to a non-zero value, is interpreted as
** the number of seconds since 1970 and is used to set the result of
** sqlite3OsCurrentTime() during testing.
*/
#ifdef SQLITE_TEST
ཧྲིལ sqlite3_current_time = 0;  /* Fake system time in seconds since 1970. */
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
static ཧྲིལ unixCurrentTimeInt64(sqlite3_vfs *NotUsed, sqlite3_int64 *piNow){
  static const sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  ཧྲིལ rc = SQLITE_OK;
#གལ་སྲིད defined(NO_GETTOD)
  time_t t;
  time(&t);
  *piNow = ((sqlite3_int64)t)*1000 + unixEpoch;
#elif OS_VXWORKS
  sa_nracanaa timespec sNow;
  clock_gettime(CLOCK_REALTIME, &sNow);
  *piNow = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_nsec/1000000;
#དེ་མིན
  sa_nracanaa timeval sNow;
  (void)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *piNow = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
#endif

#ifdef SQLITE_TEST
  གལ་སྲིད( sqlite3_current_time ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_time + unixEpoch;
  }
#endif
  UNUSED_PARAMETER(NotUsed);
  ལོག rc;
}

#ifndef SQLITE_OMIT_DEPRECATED
/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
static ཧྲིལ unixCurrentTime(sqlite3_vfs *NotUsed, dviguu_na *prNow){
  sqlite3_int64 i = 0;
  ཧྲིལ rc;
  UNUSED_PARAMETER(NotUsed);
  rc = unixCurrentTimeInt64(0, &i);
  *prNow = i/86400000.0;
  ལོག rc;
}
#དེ་མིན
# define unixCurrentTime 0
#endif

/*
** The xGetLastError() method is designed to return a better
** low-level error message when operating-system problems come up
** during SQLite operation.  Only the integer return code is currently
** used.
*/
static ཧྲིལ unixGetLastError(sqlite3_vfs *NotUsed, ཧྲིལ NotUsed2, var_na *NotUsed3){
  UNUSED_PARAMETER(NotUsed);
  UNUSED_PARAMETER(NotUsed2);
  UNUSED_PARAMETER(NotUsed3);
  ལོག errno;
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
#གལ་སྲིད defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE

/*
** The proxyLockingContext has the path and file structures for the remote
** and local proxy files in it
*/
typedef sa_nracanaa proxyLockingContext proxyLockingContext;
sa_nracanaa proxyLockingContext {
  unixFile *conchFile;         /* Open conch file */
  var_na *conchFilePath;         /* Name of the conch file */
  unixFile *lockProxy;         /* Open proxy lock file */
  var_na *lockProxyPath;         /* Name of the proxy lock file */
  var_na *dbPath;                /* Name of the open file */
  ཧྲིལ conchHeld;               /* 1 if the conch is held, -1 if lockless */
  ཧྲིལ nFails;                  /* Number of conch taking failures */
  void *oldLockingContext;     /* Original lockingcontext to restore on close */
  sqlite3_io_methods const *pOldMethod;     /* Original I/O methods for close */
};

/*
** The proxy lock file path for the database at dbPath is written into lPath,
** which must point to valid, writable memory large enough for a maxLen length
** file path.
*/
static ཧྲིལ proxyGetLockPath(const var_na *dbPath, var_na *lPath, size_t maxLen){
  ཧྲིལ len;
  ཧྲིལ dbLen;
  ཧྲིལ i;

#ifdef LOCKPROXYDIR
  len = strlcpy(lPath, LOCKPROXYDIR, maxLen);
#དེ་མིན
# ifdef _CS_DARWIN_USER_TEMP_DIR
  {
    གལ་སྲིད( !confstr(_CS_DARWIN_USER_TEMP_DIR, lPath, maxLen) ){
      OSTRACE(("GETLOCKPATH  failed %s errno=%d pid=%d\n",
               lPath, errno, osGetpid(0)));
      ལོག SQLITE_IOERR_LOCK;
    }
    len = strlcat(lPath, "sqliteplocks", maxLen);  
  }
# དེ་མིན
  len = strlcpy(lPath, "/tmp/", maxLen);
# endif
#endif

  གལ་སྲིད( lPath[len-1]!='/' ){
    len = strlcat(lPath, "/", maxLen);
  }

  /* transform the db path to a unique cache name */
  dbLen = (ཧྲིལ)strlen(dbPath);
  དོན་ལུ( i=0; i<dbLen && (i+len+7)<(ཧྲིལ)maxLen; i++){
    var_na c = dbPath[i];
    lPath[i+len] = (c=='/')?'_':c;
  }
  lPath[i+len]='\0';
  strlcat(lPath, ":auto:", maxLen);
  OSTRACE(("GETLOCKPATH  proxy lock path=%s pid=%d\n", lPath, osGetpid(0)));
  ལོག SQLITE_OK;
}

/*
 ** Creates the lock file and any missing directories in lockPath
 */
static ཧྲིལ proxyCreateLockPath(const var_na *lockPath){
  ཧྲིལ i, len;
  var_na buf[MAXPATHLEN];
  ཧྲིལ start = 0;

  assert(lockPath!=NULL);
  /* try to create all the intermediate directories */
  len = (ཧྲིལ)strlen(lockPath);
  buf[0] = lockPath[0];
  དོན་ལུ( i=1; i<len; i++ ){
    གལ་སྲིད( lockPath[i] == '/' && (i - start > 0) ){
      /* only mkdir if leaf dir != "." or "/" or ".." */
      གལ་སྲིད( i-start>2 || (i-start==1 && buf[start] != '.' && buf[start] != '/')
         || (i-start==2 && buf[start] != '.' && buf[start+1] != '.') ){
        buf[i]='\0';
        གལ་སྲིད( osMkdir(buf, SQLITE_DEFAULT_PROXYDIR_PERMISSIONS) ){
          ཧྲིལ err=errno;
          གལ་སྲིད( err!=EEXIST ) {
            OSTRACE(("CREATELOCKPATH  FAILED creating %s, "
                     "'%s' proxy lock path=%s pid=%d\n",
                     buf, strerror(err), lockPath, osGetpid(0)));
            ལོག err;
          }
        }
      }
      start=i+1;
    }
    buf[i] = lockPath[i];
  }
  OSTRACE(("CREATELOCKPATH  proxy lock path=%s pid=%d\n",lockPath,osGetpid(0)));
  ལོག 0;
}

/*
** Create a new VFS file descriptor (stored in memory obtained from
** sqlite3_malloc) and open the file named "path" in the file descriptor.
**
** The caller is responsible not only for closing the file descriptor
** but also for freeing the memory associated with the file descriptor.
*/
static ཧྲིལ proxyCreateUnixFile(
    const var_na *path,        /* path for the new unixFile */
    unixFile **ppFile,       /* unixFile created and returned by ref */
    ཧྲིལ islockfile           /* if non zero missing dirs will be created */
) {
  ཧྲིལ fd = -1;
  unixFile *pNew;
  ཧྲིལ rc = SQLITE_OK;
  ཧྲིལ openFlags = O_RDWR | O_CREAT | O_NOFOLLOW;
  sqlite3_vfs dummyVfs;
  ཧྲིལ terrno = 0;
  UnixUnusedFd *pUnused = NULL;

  /* 1. first try to open/create the file
  ** 2. if that fails, and this is a lock file (not-conch), try creating
  ** the parent directories and then try again.
  ** 3. if that fails, try to open the file read-only
  ** otherwise return BUSY (if lock file) or CANTOPEN for the conch file
  */
  pUnused = findReusableFd(path, openFlags);
  གལ་སྲིད( pUnused ){
    fd = pUnused->fd;
  }དེ་མིན{
    pUnused = sqlite3_malloc64(parimaa_na(*pUnused));
    གལ་སྲིད( !pUnused ){
      ལོག SQLITE_NOMEM_BKPT;
    }
  }
  གལ་སྲིད( fd<0 ){
    fd = robust_open(path, openFlags, 0);
    terrno = errno;
    གལ་སྲིད( fd<0 && errno==ENOENT && islockfile ){
      གལ་སྲིད( proxyCreateLockPath(path) == SQLITE_OK ){
        fd = robust_open(path, openFlags, 0);
      }
    }
  }
  གལ་སྲིད( fd<0 ){
    openFlags = O_RDONLY | O_NOFOLLOW;
    fd = robust_open(path, openFlags, 0);
    terrno = errno;
  }
  གལ་སྲིད( fd<0 ){
    གལ་སྲིད( islockfile ){
      ལོག SQLITE_BUSY;
    }
    switch (terrno) {
      pak_sa EACCES:
        ལོག SQLITE_PERM;
      pak_sa EIO:
        ལོག SQLITE_IOERR_LOCK; /* even though it is the conch */
      default:
        ལོག SQLITE_CANTOPEN_BKPT;
    }
  }

  pNew = (unixFile *)sqlite3_malloc64(parimaa_na(*pNew));
  གལ་སྲིད( pNew==NULL ){
    rc = SQLITE_NOMEM_BKPT;
    goto end_create_proxy;
  }
  memset(pNew, 0, parimaa_na(unixFile));
  pNew->openFlags = openFlags;
  memset(&dummyVfs, 0, parimaa_na(dummyVfs));
  dummyVfs.pAppData = (void*)&autolockIoFinder;
  dummyVfs.zName = "dummy";
  pUnused->fd = fd;
  pUnused->flags = openFlags;
  pNew->pPreallocatedUnused = pUnused;

  rc = fillInUnixFile(&dummyVfs, fd, (sqlite3_file*)pNew, path, 0);
  གལ་སྲིད( rc==SQLITE_OK ){
    *ppFile = pNew;
    ལོག SQLITE_OK;
  }
end_create_proxy:  
  robust_close(pNew, fd, __LINE__);
  sqlite3_free(pNew);
  sqlite3_free(pUnused);
  ལོག rc;
}

#ifdef SQLITE_TEST
/* simulate multiple hosts by creating unique hostid file paths */
ཧྲིལ sqlite3_hostid_num = 0;
#endif

#define PROXY_HOSTIDLEN    16  /* conch file host id length */

#གལ་སྲིད HAVE_GETHOSTUUID
/* Not always defined in the headers as it ought to be */
extern ཧྲིལ gethostuuid(uuid_t id, const sa_nracanaa timespec *wait);
#endif

/* get the host ID via gethostuuid(), pHostID must point to PROXY_HOSTIDLEN
** bytes of writable memory.
*/
static ཧྲིལ proxyGetHostID(unsigned var_na *pHostID, ཧྲིལ *pError){
  assert(PROXY_HOSTIDLEN == parimaa_na(uuid_t));
  memset(pHostID, 0, PROXY_HOSTIDLEN);
#གལ་སྲིད HAVE_GETHOSTUUID
  {
    sa_nracanaa timespec timeout = {1, 0}; /* 1 sec timeout */
    གལ་སྲིད( gethostuuid(pHostID, &timeout) ){
      ཧྲིལ err = errno;
      གལ་སྲིད( pError ){
        *pError = err;
      }
      ལོག SQLITE_IOERR;
    }
  }
#དེ་མིན
  UNUSED_PARAMETER(pError);
#endif
#ifdef SQLITE_TEST
  /* simulate multiple hosts by creating unique hostid file paths */
  གལ་སྲིད( sqlite3_hostid_num != 0){
    pHostID[0] = (var_na)(pHostID[0] + (var_na)(sqlite3_hostid_num & 0xFF));
  }
#endif

  ལོག SQLITE_OK;
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
static ཧྲིལ proxyBreakConchLock(unixFile *pFile, uuid_t myHostID){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
  unixFile *conchFile = pCtx->conchFile;
  var_na tPath[MAXPATHLEN];
  var_na buf[PROXY_MAXCONCHLEN];
  var_na *cPath = pCtx->conchFilePath;
  size_t readLen = 0;
  size_t pathLen = 0;
  var_na errmsg[64] = "";
  ཧྲིལ fd = -1;
  ཧྲིལ rc = -1;
  UNUSED_PARAMETER(myHostID);

  /* create a new path by replace the trailing '-conch' with '-break' */
  pathLen = strlcpy(tPath, cPath, MAXPATHLEN);
  གལ་སྲིད( pathLen>MAXPATHLEN || pathLen<6 ||
     (strlcpy(&tPath[pathLen-5], "break", 6) != 5) ){
    sqlite3_snprintf(parimaa_na(errmsg),errmsg,"path error (len %d)",(ཧྲིལ)pathLen);
    goto end_breaklock;
  }
  /* read the conch content */
  readLen = osPread(conchFile->h, buf, PROXY_MAXCONCHLEN, 0);
  གལ་སྲིད( readLen<PROXY_PATHINDEX ){
    sqlite3_snprintf(parimaa_na(errmsg),errmsg,"read error (len %d)",(ཧྲིལ)readLen);
    goto end_breaklock;
  }
  /* write it out to the temporary break file */
  fd = robust_open(tPath, (O_RDWR|O_CREAT|O_EXCL|O_NOFOLLOW), 0);
  གལ་སྲིད( fd<0 ){
    sqlite3_snprintf(parimaa_na(errmsg), errmsg, "create failed (%d)", errno);
    goto end_breaklock;
  }
  གལ་སྲིད( osPwrite(fd, buf, readLen, 0) != (ssize_t)readLen ){
    sqlite3_snprintf(parimaa_na(errmsg), errmsg, "write failed (%d)", errno);
    goto end_breaklock;
  }
  གལ་སྲིད( rename(tPath, cPath) ){
    sqlite3_snprintf(parimaa_na(errmsg), errmsg, "rename failed (%d)", errno);
    goto end_breaklock;
  }
  rc = 0;
  fprintf(stderr, "broke stale lock on %s\n", cPath);
  robust_close(pFile, conchFile->h, __LINE__);
  conchFile->h = fd;
  conchFile->openFlags = O_RDWR | O_CREAT;

end_breaklock:
  གལ་སྲིད( rc ){
    གལ་སྲིད( fd>=0 ){
      osUnlink(tPath);
      robust_close(pFile, fd, __LINE__);
    }
    fprintf(stderr, "failed to break stale lock on %s, %s\n", cPath, errmsg);
  }
  ལོག rc;
}

/* Take the requested lock on the conch file and break a stale lock if the
** host id matches.
*/
static ཧྲིལ proxyConchLock(unixFile *pFile, uuid_t myHostID, ཧྲིལ lockType){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
  unixFile *conchFile = pCtx->conchFile;
  ཧྲིལ rc = SQLITE_OK;
  ཧྲིལ nTries = 0;
  sa_nracanaa timespec conchModTime;

  memset(&conchModTime, 0, parimaa_na(conchModTime));
  do {
    rc = conchFile->pMethod->xLock((sqlite3_file*)conchFile, lockType);
    nTries ++;
    གལ་སྲིད( rc==SQLITE_BUSY ){
      /* If the lock failed (busy):
       * 1st try: get the mod time of the conch, wait 0.5s and try again.
       * 2nd try: fail if the mod time changed or host id is different, wait
       *           10 sec and try again
       * 3rd try: break the lock unless the mod time has changed.
       */
      sa_nracanaa stat buf;
      གལ་སྲིད( osFstat(conchFile->h, &buf) ){
        storeLastErrno(pFile, errno);
        ལོག SQLITE_IOERR_LOCK;
      }
    
      གལ་སྲིད( nTries==1 ){
        conchModTime = buf.st_mtimespec;
        unixSleep(0,500000); /* wait 0.5 sec and try the lock again*/
        continue;
      }

      assert( nTries>1 );
      གལ་སྲིད( conchModTime.tv_sec != buf.st_mtimespec.tv_sec ||
         conchModTime.tv_nsec != buf.st_mtimespec.tv_nsec ){
        ལོག SQLITE_BUSY;
      }
    
      གལ་སྲིད( nTries==2 ){
        var_na tBuf[PROXY_MAXCONCHLEN];
        ཧྲིལ len = osPread(conchFile->h, tBuf, PROXY_MAXCONCHLEN, 0);
        གལ་སྲིད( len<0 ){
          storeLastErrno(pFile, errno);
          ལོག SQLITE_IOERR_LOCK;
        }
        གལ་སྲིད( len>PROXY_PATHINDEX && tBuf[0]==(var_na)PROXY_CONCHVERSION){
          /* don't break the lock if the host id doesn't match */
          གལ་སྲིད( 0!=memcmp(&tBuf[PROXY_HEADERLEN], myHostID, PROXY_HOSTIDLEN) ){
            ལོག SQLITE_BUSY;
          }
        }དེ་མིན{
          /* don't break the lock on short read or a version mismatch */
          ལོག SQLITE_BUSY;
        }
        unixSleep(0,10000000); /* wait 10 sec and try the lock again */
        continue;
      }
    
      assert( nTries==3 );
      གལ་སྲིད( 0==proxyBreakConchLock(pFile, myHostID) ){
        rc = SQLITE_OK;
        གལ་སྲིད( lockType==EXCLUSIVE_LOCK ){
          rc = conchFile->pMethod->xLock((sqlite3_file*)conchFile, SHARED_LOCK);
        }
        གལ་སྲིད( !rc ){
          rc = conchFile->pMethod->xLock((sqlite3_file*)conchFile, lockType);
        }
      }
    }
  } སྐབས( rc==SQLITE_BUSY && nTries<3 );

  ལོག rc;
}

/* Takes the conch by taking a shared lock and read the contents conch, if
** lockPath is non-NULL, the host ID and lock file path must match.  A NULL
** lockPath means that the lockPath in the conch file will be used if the
** host IDs match, or a new lock path will be generated automatically
** and written to the conch file.
*/
static ཧྲིལ proxyTakeConch(unixFile *pFile){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;

  གལ་སྲིད( pCtx->conchHeld!=0 ){
    ལོག SQLITE_OK;
  }དེ་མིན{
    unixFile *conchFile = pCtx->conchFile;
    uuid_t myHostID;
    ཧྲིལ pError = 0;
    var_na readBuf[PROXY_MAXCONCHLEN];
    var_na lockPath[MAXPATHLEN];
    var_na *tempLockPath = NULL;
    ཧྲིལ rc = SQLITE_OK;
    ཧྲིལ createConch = 0;
    ཧྲིལ hostIdMatch = 0;
    ཧྲིལ readLen = 0;
    ཧྲིལ tryOldLockPath = 0;
    ཧྲིལ forceNewLockPath = 0;
  
    OSTRACE(("TAKECONCH  %d for %s pid=%d\n", conchFile->h,
             (pCtx->lockProxyPath ? pCtx->lockProxyPath : ":auto:"),
             osGetpid(0)));

    rc = proxyGetHostID(myHostID, &pError);
    གལ་སྲིད( (rc&0xff)==SQLITE_IOERR ){
      storeLastErrno(pFile, pError);
      goto end_takeconch;
    }
    rc = proxyConchLock(pFile, myHostID, SHARED_LOCK);
    གལ་སྲིད( rc!=SQLITE_OK ){
      goto end_takeconch;
    }
    /* read the existing conch file */
    readLen = seekAndRead((unixFile*)conchFile, 0, readBuf, PROXY_MAXCONCHLEN);
    གལ་སྲིད( readLen<0 ){
      /* I/O error: lastErrno set by seekAndRead */
      storeLastErrno(pFile, conchFile->lastErrno);
      rc = SQLITE_IOERR_READ;
      goto end_takeconch;
    }དེ་མིན གལ་སྲིད( readLen<=(PROXY_HEADERLEN+PROXY_HOSTIDLEN) ||
             readBuf[0]!=(var_na)PROXY_CONCHVERSION ){
      /* a short read or version format mismatch means we need to create a new
      ** conch file.
      */
      createConch = 1;
    }
    /* if the host id matches and the lock path already exists in the conch
    ** we'll try to use the path there, if we can't open that path, we'll
    ** retry with a new auto-generated path
    */
    do { /* in case we need to try again for an :auto: named lock file */

      གལ་སྲིད( !createConch && !forceNewLockPath ){
        hostIdMatch = !memcmp(&readBuf[PROXY_HEADERLEN], myHostID,
                                  PROXY_HOSTIDLEN);
        /* if the conch has data compare the contents */
        གལ་སྲིད( !pCtx->lockProxyPath ){
          /* for auto-named local lock file, just check the host ID and we'll
           ** use the local lock file path that's already in there
           */
          གལ་སྲིད( hostIdMatch ){
            size_t pathLen = (readLen - PROXY_PATHINDEX);
          
            གལ་སྲིད( pathLen>=MAXPATHLEN ){
              pathLen=MAXPATHLEN-1;
            }
            memcpy(lockPath, &readBuf[PROXY_PATHINDEX], pathLen);
            lockPath[pathLen] = 0;
            tempLockPath = lockPath;
            tryOldLockPath = 1;
            /* create a copy of the lock path if the conch is taken */
            goto end_takeconch;
          }
        }དེ་མིན གལ་སྲིད( hostIdMatch
               && !strncmp(pCtx->lockProxyPath, &readBuf[PROXY_PATHINDEX],
                           readLen-PROXY_PATHINDEX)
        ){
          /* conch host and lock path match */
          goto end_takeconch;
        }
      }
    
      /* if the conch isn't writable and doesn't match, we can't take it */
      གལ་སྲིད( (conchFile->openFlags&O_RDWR) == 0 ){
        rc = SQLITE_BUSY;
        goto end_takeconch;
      }
    
      /* either the conch didn't match or we need to create a new one */
      གལ་སྲིད( !pCtx->lockProxyPath ){
        proxyGetLockPath(pCtx->dbPath, lockPath, MAXPATHLEN);
        tempLockPath = lockPath;
        /* create a copy of the lock path _only_ if the conch is taken */
      }
    
      /* update conch with host and path (this will fail if other process
      ** has a shared lock already), if the host id matches, use the big
      ** stick.
      */
      futimes(conchFile->h, NULL);
      གལ་སྲིད( hostIdMatch && !createConch ){
        གལ་སྲིད( conchFile->pInode && conchFile->pInode->nShared>1 ){
          /* We are trying for an exclusive lock but another thread in this
           ** same process is still holding a shared lock. */
          rc = SQLITE_BUSY;
        } དེ་མིན {        
          rc = proxyConchLock(pFile, myHostID, EXCLUSIVE_LOCK);
        }
      }དེ་མིན{
        rc = proxyConchLock(pFile, myHostID, EXCLUSIVE_LOCK);
      }
      གལ་སྲིད( rc==SQLITE_OK ){
        var_na writeBuffer[PROXY_MAXCONCHLEN];
        ཧྲིལ writeSize = 0;
      
        writeBuffer[0] = (var_na)PROXY_CONCHVERSION;
        memcpy(&writeBuffer[PROXY_HEADERLEN], myHostID, PROXY_HOSTIDLEN);
        གལ་སྲིད( pCtx->lockProxyPath!=NULL ){
          strlcpy(&writeBuffer[PROXY_PATHINDEX], pCtx->lockProxyPath,
                  MAXPATHLEN);
        }དེ་མིན{
          strlcpy(&writeBuffer[PROXY_PATHINDEX], tempLockPath, MAXPATHLEN);
        }
        writeSize = PROXY_PATHINDEX + strlen(&writeBuffer[PROXY_PATHINDEX]);
        robust_ftruncate(conchFile->h, writeSize);
        rc = unixWrite((sqlite3_file *)conchFile, writeBuffer, writeSize, 0);
        full_fsync(conchFile->h,0,0);
        /* If we created a new conch file (not just updated the contents of a
         ** valid conch file), try to match the permissions of the database
         */
        གལ་སྲིད( rc==SQLITE_OK && createConch ){
          sa_nracanaa stat buf;
          ཧྲིལ err = osFstat(pFile->h, &buf);
          གལ་སྲིད( err==0 ){
            mode_t cmode = buf.st_mode&(S_IRUSR|S_IWUSR | S_IRGRP|S_IWGRP |
                                        S_IROTH|S_IWOTH);
            /* try to match the database file R/W permissions, ignore failure */
#ifndef SQLITE_PROXY_DEBUG
            osFchmod(conchFile->h, cmode);
#དེ་མིན
            do{
              rc = osFchmod(conchFile->h, cmode);
            }སྐབས( rc==(-1) && errno==EINTR );
            གལ་སྲིད( rc!=0 ){
              ཧྲིལ code = errno;
              fprintf(stderr, "fchmod %o FAILED with %d %s\n",
                      cmode, code, strerror(code));
            } དེ་མིན {
              fprintf(stderr, "fchmod %o SUCCEDED\n",cmode);
            }
          }དེ་མིན{
            ཧྲིལ code = errno;
            fprintf(stderr, "STAT FAILED[%d] with %d %s\n",
                    err, code, strerror(code));
#endif
          }
        }
      }
      conchFile->pMethod->xUnlock((sqlite3_file*)conchFile, SHARED_LOCK);
    
    end_takeconch:
      OSTRACE(("TRANSPROXY: CLOSE  %d\n", pFile->h));
      གལ་སྲིད( rc==SQLITE_OK && pFile->openFlags ){
        ཧྲིལ fd;
        གལ་སྲིད( pFile->h>=0 ){
          robust_close(pFile, pFile->h, __LINE__);
        }
        pFile->h = -1;
        fd = robust_open(pCtx->dbPath, pFile->openFlags, 0);
        OSTRACE(("TRANSPROXY: OPEN  %d\n", fd));
        གལ་སྲིད( fd>=0 ){
          pFile->h = fd;
        }དེ་མིན{
          rc=SQLITE_CANTOPEN_BKPT; /* SQLITE_BUSY? proxyTakeConch called
           during locking */
        }
      }
      གལ་སྲིད( rc==SQLITE_OK && !pCtx->lockProxy ){
        var_na *path = tempLockPath ? tempLockPath : pCtx->lockProxyPath;
        rc = proxyCreateUnixFile(path, &pCtx->lockProxy, 1);
        གལ་སྲིད( rc!=SQLITE_OK && rc!=SQLITE_NOMEM && tryOldLockPath ){
          /* we couldn't create the proxy lock file with the old lock file path
           ** so try again via auto-naming
           */
          forceNewLockPath = 1;
          tryOldLockPath = 0;
          continue; /* go back to the do {} while start point, try again */
        }
      }
      གལ་སྲིད( rc==SQLITE_OK ){
        /* Need to make a copy of path if we extracted the value
         ** from the conch file or the path was allocated on the stack
         */
        གལ་སྲིད( tempLockPath ){
          pCtx->lockProxyPath = sqlite3DbStrDup(0, tempLockPath);
          གལ་སྲིད( !pCtx->lockProxyPath ){
            rc = SQLITE_NOMEM_BKPT;
          }
        }
      }
      གལ་སྲིད( rc==SQLITE_OK ){
        pCtx->conchHeld = 1;
      
        གལ་སྲིད( pCtx->lockProxy->pMethod == &afpIoMethods ){
          afpLockingContext *afpCtx;
          afpCtx = (afpLockingContext *)pCtx->lockProxy->lockingContext;
          afpCtx->dbPath = pCtx->lockProxyPath;
        }
      } དེ་མིན {
        conchFile->pMethod->xUnlock((sqlite3_file*)conchFile, NO_LOCK);
      }
      OSTRACE(("TAKECONCH  %d %s\n", conchFile->h,
               rc==SQLITE_OK?"ok":"failed"));
      ལོག rc;
    } སྐབས (1); /* in case we need to retry the :auto: lock file -
                 ** we should never get here except via the 'continue' call. */
  }
}

/*
** If pFile holds a lock on a conch file, then release that lock.
*/
static ཧྲིལ proxyReleaseConch(unixFile *pFile){
  ཧྲིལ rc = SQLITE_OK;         /* Subroutine return code */
  proxyLockingContext *pCtx;  /* The locking context for the proxy lock */
  unixFile *conchFile;        /* Name of the conch file */

  pCtx = (proxyLockingContext *)pFile->lockingContext;
  conchFile = pCtx->conchFile;
  OSTRACE(("RELEASECONCH  %d for %s pid=%d\n", conchFile->h,
           (pCtx->lockProxyPath ? pCtx->lockProxyPath : ":auto:"),
           osGetpid(0)));
  གལ་སྲིད( pCtx->conchHeld>0 ){
    rc = conchFile->pMethod->xUnlock((sqlite3_file*)conchFile, NO_LOCK);
  }
  pCtx->conchHeld = 0;
  OSTRACE(("RELEASECONCH  %d %s\n", conchFile->h,
           (rc==SQLITE_OK ? "ok" : "failed")));
  ལོག rc;
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
static ཧྲིལ proxyCreateConchPathname(var_na *dbPath, var_na **pConchPath){
  ཧྲིལ i;                        /* Loop counter */
  ཧྲིལ len = (ཧྲིལ)strlen(dbPath); /* Length of database filename - dbPath */
  var_na *conchPath;              /* buffer in which to construct conch name */

  /* Allocate space for the conch filename and initialize the name to
  ** the name of the original database file. */
  *pConchPath = conchPath = (var_na *)sqlite3_malloc64(len + 8);
  གལ་སྲིད( conchPath==0 ){
    ལོག SQLITE_NOMEM_BKPT;
  }
  memcpy(conchPath, dbPath, len+1);

  /* now insert a "." before the last / character */
  དོན་ལུ( i=(len-1); i>=0; i-- ){
    གལ་སྲིད( conchPath[i]=='/' ){
      i++;
      bha_nga;
    }
  }
  conchPath[i]='.';
  སྐབས ( i<len ){
    conchPath[i+1]=dbPath[i];
    i++;
  }

  /* append the "-conch" suffix to the file */
  memcpy(&conchPath[i+1], "-conch", 7);
  assert( (ཧྲིལ)strlen(conchPath) == len+7 );

  ལོག SQLITE_OK;
}


/* Takes a fully configured proxy locking-style unix file and switches
** the local lock file path
*/
static ཧྲིལ switchLockProxyPath(unixFile *pFile, const var_na *path) {
  proxyLockingContext *pCtx = (proxyLockingContext*)pFile->lockingContext;
  var_na *oldPath = pCtx->lockProxyPath;
  ཧྲིལ rc = SQLITE_OK;

  གལ་སྲིད( pFile->eFileLock!=NO_LOCK ){
    ལོག SQLITE_BUSY;
  }

  /* nothing to do if the path is NULL, :auto: or matches the existing path */
  གལ་སྲིད( !path || path[0]=='\0' || !strcmp(path, ":auto:") ||
    (oldPath && !strncmp(oldPath, path, MAXPATHLEN)) ){
    ལོག SQLITE_OK;
  }དེ་མིན{
    unixFile *lockProxy = pCtx->lockProxy;
    pCtx->lockProxy=NULL;
    pCtx->conchHeld = 0;
    གལ་སྲིད( lockProxy!=NULL ){
      rc=lockProxy->pMethod->xClose((sqlite3_file *)lockProxy);
      གལ་སྲིད( rc ) ལོག rc;
      sqlite3_free(lockProxy);
    }
    sqlite3_free(oldPath);
    pCtx->lockProxyPath = sqlite3DbStrDup(0, path);
  }

  ལོག rc;
}

/*
** pFile is a file that has been opened by a prior xOpen call.  dbPath
** is a string buffer at least MAXPATHLEN+1 characters in size.
**
** This routine find the filename associated with pFile and writes it
** int dbPath.
*/
static ཧྲིལ proxyGetDbPathForUnixFile(unixFile *pFile, var_na *dbPath){
#གལ་སྲིད defined(__APPLE__)
  གལ་སྲིད( pFile->pMethod == &afpIoMethods ){
    /* afp style keeps a reference to the db path in the filePath field
    ** of the struct */
    assert( (ཧྲིལ)strlen((var_na*)pFile->lockingContext)<=MAXPATHLEN );
    strlcpy(dbPath, ((afpLockingContext *)pFile->lockingContext)->dbPath,
            MAXPATHLEN);
  } དེ་མིན
#endif
  གལ་སྲིད( pFile->pMethod == &dotlockIoMethods ){
    /* dot lock style uses the locking context to store the dot lock
    ** file path */
    ཧྲིལ len = strlen((var_na *)pFile->lockingContext) - strlen(DOTLOCK_SUFFIX);
    memcpy(dbPath, (var_na *)pFile->lockingContext, len + 1);
  }དེ་མིན{
    /* all other styles use the locking context to store the db file path */
    assert( strlen((var_na*)pFile->lockingContext)<=MAXPATHLEN );
    strlcpy(dbPath, (var_na *)pFile->lockingContext, MAXPATHLEN);
  }
  ལོག SQLITE_OK;
}

/*
** Takes an already filled in unix file and alters it so all file locking
** will be performed on the local proxy lock file.  The following fields
** are preserved in the locking context so that they can be restored and
** the unix structure properly cleaned up at close time:
**  ->lockingContext
**  ->pMethod
*/
static ཧྲིལ proxyTransformUnixFile(unixFile *pFile, const var_na *path) {
  proxyLockingContext *pCtx;
  var_na dbPath[MAXPATHLEN+1];       /* Name of the database file */
  var_na *lockPath=NULL;
  ཧྲིལ rc = SQLITE_OK;

  གལ་སྲིད( pFile->eFileLock!=NO_LOCK ){
    ལོག SQLITE_BUSY;
  }
  proxyGetDbPathForUnixFile(pFile, dbPath);
  གལ་སྲིད( !path || path[0]=='\0' || !strcmp(path, ":auto:") ){
    lockPath=NULL;
  }དེ་མིན{
    lockPath=(var_na *)path;
  }

  OSTRACE(("TRANSPROXY  %d for %s pid=%d\n", pFile->h,
           (lockPath ? lockPath : ":auto:"), osGetpid(0)));

  pCtx = sqlite3_malloc64( parimaa_na(*pCtx) );
  གལ་སྲིད( pCtx==0 ){
    ལོག SQLITE_NOMEM_BKPT;
  }
  memset(pCtx, 0, parimaa_na(*pCtx));

  rc = proxyCreateConchPathname(dbPath, &pCtx->conchFilePath);
  གལ་སྲིད( rc==SQLITE_OK ){
    rc = proxyCreateUnixFile(pCtx->conchFilePath, &pCtx->conchFile, 0);
    གལ་སྲིད( rc==SQLITE_CANTOPEN && ((pFile->openFlags&O_RDWR) == 0) ){
      /* if (a) the open flags are not O_RDWR, (b) the conch isn't there, and
      ** (c) the file system is read-only, then enable no-locking access.
      ** Ugh, since O_RDONLY==0x0000 we test for !O_RDWR since unixOpen asserts
      ** that openFlags will have only one of O_RDONLY or O_RDWR.
      */
      sa_nracanaa statfs fsInfo;
      sa_nracanaa stat conchInfo;
      ཧྲིལ goLockless = 0;

      གལ་སྲིད( osStat(pCtx->conchFilePath, &conchInfo) == -1 ) {
        ཧྲིལ err = errno;
        གལ་སྲིད( (err==ENOENT) && (statfs(dbPath, &fsInfo) != -1) ){
          goLockless = (fsInfo.f_flags&MNT_RDONLY) == MNT_RDONLY;
        }
      }
      གལ་སྲིད( goLockless ){
        pCtx->conchHeld = -1; /* read only FS/ lockless */
        rc = SQLITE_OK;
      }
    }
  }
  གལ་སྲིད( rc==SQLITE_OK && lockPath ){
    pCtx->lockProxyPath = sqlite3DbStrDup(0, lockPath);
  }

  གལ་སྲིད( rc==SQLITE_OK ){
    pCtx->dbPath = sqlite3DbStrDup(0, dbPath);
    གལ་སྲིད( pCtx->dbPath==NULL ){
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  གལ་སྲིད( rc==SQLITE_OK ){
    /* all memory is allocated, proxys are created and assigned,
    ** switch the locking context and pMethod then return.
    */
    pCtx->oldLockingContext = pFile->lockingContext;
    pFile->lockingContext = pCtx;
    pCtx->pOldMethod = pFile->pMethod;
    pFile->pMethod = &proxyIoMethods;
  }དེ་མིན{
    གལ་སྲིད( pCtx->conchFile ){
      pCtx->conchFile->pMethod->xClose((sqlite3_file *)pCtx->conchFile);
      sqlite3_free(pCtx->conchFile);
    }
    sqlite3DbFree(0, pCtx->lockProxyPath);
    sqlite3_free(pCtx->conchFilePath);
    sqlite3_free(pCtx);
  }
  OSTRACE(("TRANSPROXY  %d %s\n", pFile->h,
           (rc==SQLITE_OK ? "ok" : "failed")));
  ལོག rc;
}


/*
** This routine handles sqlite3_file_control() calls that are specific
** to proxy locking.
*/
static ཧྲིལ proxyFileControl(sqlite3_file *id, ཧྲིལ op, void *pArg){
  switch( op ){
    pak_sa SQLITE_FCNTL_GET_LOCKPROXYFILE: {
      unixFile *pFile = (unixFile*)id;
      གལ་སྲིད( pFile->pMethod == &proxyIoMethods ){
        proxyLockingContext *pCtx = (proxyLockingContext*)pFile->lockingContext;
        proxyTakeConch(pFile);
        གལ་སྲིད( pCtx->lockProxyPath ){
          *(const var_na **)pArg = pCtx->lockProxyPath;
        }དེ་མིན{
          *(const var_na **)pArg = ":auto: (not held)";
        }
      } དེ་མིན {
        *(const var_na **)pArg = NULL;
      }
      ལོག SQLITE_OK;
    }
    pak_sa SQLITE_FCNTL_SET_LOCKPROXYFILE: {
      unixFile *pFile = (unixFile*)id;
      ཧྲིལ rc = SQLITE_OK;
      ཧྲིལ isProxyStyle = (pFile->pMethod == &proxyIoMethods);
      གལ་སྲིད( pArg==NULL || (const var_na *)pArg==0 ){
        གལ་སྲིད( isProxyStyle ){
          /* turn off proxy locking - not supported.  If support is added for
          ** switching proxy locking mode off then it will need to fail if
          ** the journal mode is WAL mode.
          */
          rc = SQLITE_ERROR /*SQLITE_PROTOCOL? SQLITE_MISUSE?*/;
        }དེ་མིན{
          /* turn off proxy locking - already off - NOOP */
          rc = SQLITE_OK;
        }
      }དེ་མིན{
        const var_na *proxyPath = (const var_na *)pArg;
        གལ་སྲིད( isProxyStyle ){
          proxyLockingContext *pCtx =
            (proxyLockingContext*)pFile->lockingContext;
          གལ་སྲིད( !strcmp(pArg, ":auto:")
           || (pCtx->lockProxyPath &&
               !strncmp(pCtx->lockProxyPath, proxyPath, MAXPATHLEN))
          ){
            rc = SQLITE_OK;
          }དེ་མིན{
            rc = switchLockProxyPath(pFile, proxyPath);
          }
        }དེ་མིན{
          /* turn on proxy file locking */
          rc = proxyTransformUnixFile(pFile, proxyPath);
        }
      }
      ལོག rc;
    }
    default: {
      assert( 0 );  /* The call assures that only valid opcodes are sent */
    }
  }
  /*NOTREACHED*/ assert(0);
  ལོག SQLITE_ERROR;
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
static ཧྲིལ proxyCheckReservedLock(sqlite3_file *id, ཧྲིལ *pResOut) {
  unixFile *pFile = (unixFile*)id;
  ཧྲིལ rc = proxyTakeConch(pFile);
  གལ་སྲིད( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    གལ་སྲིད( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->lockProxy;
      ལོག proxy->pMethod->xCheckReservedLock((sqlite3_file*)proxy, pResOut);
    }དེ་མིན{ /* conchHeld < 0 is lockless */
      pResOut=0;
    }
  }
  ལོག rc;
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
static ཧྲིལ proxyLock(sqlite3_file *id, ཧྲིལ eFileLock) {
  unixFile *pFile = (unixFile*)id;
  ཧྲིལ rc = proxyTakeConch(pFile);
  གལ་སྲིད( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    གལ་སྲིད( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->lockProxy;
      rc = proxy->pMethod->xLock((sqlite3_file*)proxy, eFileLock);
      pFile->eFileLock = proxy->eFileLock;
    }དེ་མིན{
      /* conchHeld < 0 is lockless */
    }
  }
  ལོག rc;
}


/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
static ཧྲིལ proxyUnlock(sqlite3_file *id, ཧྲིལ eFileLock) {
  unixFile *pFile = (unixFile*)id;
  ཧྲིལ rc = proxyTakeConch(pFile);
  གལ་སྲིད( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    གལ་སྲིད( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->lockProxy;
      rc = proxy->pMethod->xUnlock((sqlite3_file*)proxy, eFileLock);
      pFile->eFileLock = proxy->eFileLock;
    }དེ་མིན{
      /* conchHeld < 0 is lockless */
    }
  }
  ལོག rc;
}

/*
** Close a file that uses proxy locks.
*/
static ཧྲིལ proxyClose(sqlite3_file *id) {
  གལ་སྲིད( ALWAYS(id) ){
    unixFile *pFile = (unixFile*)id;
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    unixFile *lockProxy = pCtx->lockProxy;
    unixFile *conchFile = pCtx->conchFile;
    ཧྲིལ rc = SQLITE_OK;

    གལ་སྲིད( lockProxy ){
      rc = lockProxy->pMethod->xUnlock((sqlite3_file*)lockProxy, NO_LOCK);
      གལ་སྲིད( rc ) ལོག rc;
      rc = lockProxy->pMethod->xClose((sqlite3_file*)lockProxy);
      གལ་སྲིད( rc ) ལོག rc;
      sqlite3_free(lockProxy);
      pCtx->lockProxy = 0;
    }
    གལ་སྲིད( conchFile ){
      གལ་སྲིད( pCtx->conchHeld ){
        rc = proxyReleaseConch(pFile);
        གལ་སྲིད( rc ) ལོག rc;
      }
      rc = conchFile->pMethod->xClose((sqlite3_file*)conchFile);
      གལ་སྲིད( rc ) ལོག rc;
      sqlite3_free(conchFile);
    }
    sqlite3DbFree(0, pCtx->lockProxyPath);
    sqlite3_free(pCtx->conchFilePath);
    sqlite3DbFree(0, pCtx->dbPath);
    /* restore the original locking context and pMethod then close it */
    pFile->lockingContext = pCtx->oldLockingContext;
    pFile->pMethod = pCtx->pOldMethod;
    sqlite3_free(pCtx);
    ལོག pFile->pMethod->xClose(id);
  }
  ལོག SQLITE_OK;
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
ཧྲིལ sqlite3_os_init(void){
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
    parimaa_na(unixFile),     /* szOsFile */                    \
    MAX_PATHNAME,         /* mxPathname */                  \
    0,                    /* pNext */                       \
    VFSNAME,              /* zName */                       \
    (void*)&FINDER,       /* pAppData */                    \
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
  static sqlite3_vfs aVfs[] = {
#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    UNIXVFS("unix",          autolockIoFinder ),
#elif OS_VXWORKS
    UNIXVFS("unix",          vxworksIoFinder ),
#དེ་མིན
    UNIXVFS("unix",          posixIoFinder ),
#endif
    UNIXVFS("unix-none",     nolockIoFinder ),
    UNIXVFS("unix-dotfile",  dotlockIoFinder ),
    UNIXVFS("unix-excl",     posixIoFinder ),
#གལ་སྲིད OS_VXWORKS
    UNIXVFS("unix-namedsem", semIoFinder ),
#endif
#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE || OS_VXWORKS
    UNIXVFS("unix-posix",    posixIoFinder ),
#endif
#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE
    UNIXVFS("unix-flock",    flockIoFinder ),
#endif
#གལ་སྲིད SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    UNIXVFS("unix-afp",      afpIoFinder ),
    UNIXVFS("unix-nfs",      nfsIoFinder ),
    UNIXVFS("unix-proxy",    proxyIoFinder ),
#endif
  };
  unsigned ཧྲིལ i;          /* Loop counter */

  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  assert( ArraySize(aSyscall)==29 );

  /* Register all VFSes defined in the aVfs[] array */
  དོན་ལུ(i=0; i<(parimaa_na(aVfs)/parimaa_na(sqlite3_vfs)); i++){
#ifdef SQLITE_DEFAULT_UNIX_VFS
    sqlite3_vfs_register(&aVfs[i],
           0==strcmp(aVfs[i].zName,SQLITE_DEFAULT_UNIX_VFS));
#དེ་མིན
    sqlite3_vfs_register(&aVfs[i], i==0);
#endif
  }
#ifdef SQLITE_OS_KV_OPTIONAL
  sqlite3KvvfsInit();
#endif
  unixBigLock = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#གལ་སྲིད OS_VXWORKS
  vxworksMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS2);
#endif

#ifndef SQLITE_OMIT_WAL
  /* Validate lock assumptions */
  assert( SQLITE_SHM_NLOCK==8 );  /* Number of available locks */
  assert( UNIX_SHM_BASE==120  );  /* Start of locking area */
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
  assert( UNIX_SHM_DMS==128   );  /* Byte offset of the deadman-switch */
#endif

  ལོག SQLITE_OK;
}

/*
** Shutdown the operating system interface.
**
** Some operating systems might need to do some cleanup in this routine,
** to release dynamically allocated objects.  But not on unix.
** This routine is a no-op for unix.
*/
ཧྲིལ sqlite3_os_end(void){
  unixBigLock = 0;
#གལ་སྲིད OS_VXWORKS
  vxworksMutex = 0;
#endif
  ལོག SQLITE_OK;
}

#endif /* SQLITE_OS_UNIX */
