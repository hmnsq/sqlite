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
#شامل "sqliteInt.h"
#اگر SQLITE_OS_UNIX              /* This file is used on unix only */

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
#اگر !defined(SQLITE_ENABLE_LOCKING_STYLE)
#  اگر defined(__APPLE__)
#    define SQLITE_ENABLE_LOCKING_STYLE 1
#  ورنہ
#    define SQLITE_ENABLE_LOCKING_STYLE 0
#  endif
#اگر_ختم

/* Use pread() and pwrite() if they are available */
#اگر defined(__APPLE__) || defined(__linux__)
# define HAVE_PREAD 1
# define HAVE_PWRITE 1
#اگر_ختم
#اگر defined(HAVE_PREAD64) && defined(HAVE_PWRITE64)
# undef USE_PREAD
# define USE_PREAD64 1
#وگرنہ_اگر defined(HAVE_PREAD) && defined(HAVE_PWRITE)
# undef USE_PREAD64
# define USE_PREAD 1
#اگر_ختم

/*
** standard include files.
*/
#شامل <sys/types.h>   /* amalgamator: keep */
#شامل <sys/stat.h>    /* amalgamator: keep */
#شامل <fcntl.h>
#شامل <sys/ioctl.h>
#شامل <unistd.h>      /* amalgamator: keep */
#شامل <time.h>
#شامل <sys/time.h>    /* amalgamator: keep */
#شامل <errno.h>
#اگر (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_سائز>0) \
  && !defined(SQLITE_WASI)
# include <sys/mman.h>
#اگر_ختم

#اگر SQLITE_ENABLE_LOCKING_STYLE
# include <sys/ioctl.h>
# include <sys/file.h>
# include <sys/param.h>
#اگر_ختم /* SQLITE_ENABLE_LOCKING_STYLE */

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
#اگر_غیر_تعریف_شدہ HAVE_GETHOSTUUID
# define HAVE_GETHOSTUUID 0
# اگر defined(__APPLE__) && ((__MAC_OS_X_VERSION_MIN_REQUIRED > 1050) || \
                            (__IPHONE_OS_VERSION_MIN_REQUIRED > 2000))
#    اگر (!defined(ہدف_OS_EMBEDDED) || (ہدف_OS_EMBEDDED==0)) \
        && (!defined(ہدف_IPHONE_SIMULATOR) || (ہدف_IPHONE_SIMULATOR==0))\
        && (!defined(ہدف_OS_MACCATALYST) || (ہدف_OS_MACCATALYST==0))
#      undef HAVE_GETHOSTUUID
#      define HAVE_GETHOSTUUID 1
#    ورنہ
#      warning "gethostuuid() is disabled."
#    endif
#  endif
#اگر_ختم


#اگر OS_VXWORKS
# include <sys/ioctl.h>
# include <semaphore.h>
# include <limits.h>
#اگر_ختم /* OS_VXWORKS */

#اگر defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
# include <sys/mount.h>
#اگر_ختم

#اگر_تعریف_شدہ HAVE_UTIME
# include <utime.h>
#اگر_ختم

/*
** Allowed values of unixFile.fsFlags
*/
#تعریف_کرو SQLITE_FSFLAGS_IS_MSDOS     0x1

/*
** If we are to be thread-safe, include the pthreads header.
*/
#اگر SQLITE_THREADSAFE
# include <pthread.h>
#اگر_ختم

/*
** Default permissions when creating a new file
*/
#اگر_غیر_تعریف_شدہ SQLITE_DEFAULT_FILE_PERMISSIONS
# define SQLITE_DEFAULT_FILE_PERMISSIONS 0644
#اگر_ختم

/*
** Default permissions when creating auto proxy dir
*/
#اگر_غیر_تعریف_شدہ SQLITE_DEFAULT_PROXYDIR_PERMISSIONS
# define SQLITE_DEFAULT_PROXYDIR_PERMISSIONS 0755
#اگر_ختم

/*
** Maximum supported path-length.
*/
#تعریف_کرو MAX_PATHNAME 512

/*
** Maximum supported symbolic links
*/
#تعریف_کرو SQLITE_MAX_SYMLINKS 100

/*
** Remove and stub certain info for WASI (WebAssembly System
** Interface) builds.
*/
#اگر_تعریف_شدہ SQLITE_WASI
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
#  اگر __LONG_MAX == 0x7fffffffL
#   define F_GETLK 12
#   define F_SETLK 13
#   define F_SETLKW 14
#  ورنہ
#   define F_GETLK 5
#   define F_SETLK 6
#   define F_SETLKW 7
#  endif
# endif
#ورنہ /* !SQLITE_WASI */
# ifndef HAVE_FCHMOD
#  define HAVE_FCHMOD 1
# endif
#اگر_ختم /* SQLITE_WASI */

#اگر_تعریف_شدہ SQLITE_WASI
# define osGetpid(X) (pid_t)1
#ورنہ
/* Always cast the getpid() return type for compatibility with
** kernel modules in VxWorks. */
# define osGetpid(X) (pid_t)getpid()
#اگر_ختم

/*
** Only set the lastErrno if the error code is a real error and not
** a normal expected return code of SQLITE_BUSY or SQLITE_OK
*/
#تعریف_کرو IS_تالا_ERROR(x)  ((x != SQLITE_OK) && (x != SQLITE_BUSY))

/* Forward references */
قسمتعریف ساخت unixShm unixShm;               /* Connection shared memory */
قسمتعریف ساخت unix_Shm_گرہ unix_Shm_گرہ;       /* Shared memory instance */
قسمتعریف ساخت unixInodeInfo unixInodeInfo;   /* An i-node */
قسمتعریف ساخت Unix_غیر_مستعمل_Fd Unix_غیر_مستعمل_Fd;     /* An unused file descriptor */

/*
** Sometimes, after a file handle is closed by SQLite, the file descriptor
** cannot be closed immediately. In these cases, instances of the following
** structure are used to store the file descriptor while waiting for an
** opportunity to either close or reuse it.
*/
ساخت Unix_غیر_مستعمل_Fd {
  صحیح fd;                   /* File descriptor to close */
  صحیح flags;                /* Flags this file descriptor was opened with */
  Unix_غیر_مستعمل_Fd *p_اگلا;      /* Next unused file descriptor on same file */
};

/*
** The unixFile structure is subclass of sqlite3_file specific to the unix
** VFS implementations.
*/
قسمتعریف ساخت unixFile unixFile;
ساخت unixFile {
  sqlite3_io_methods ثابت *p_طریقہ;  /* Always the first entry */
  sqlite3_vfs *pVfs;                  /* The VFS that created this unixFile */
  unixInodeInfo *pInode;              /* Info about locks on this inode */
  صحیح h;                              /* The file descriptor */
  غیرعلامتی حرف e_File_تالا;            /* The type of lock held on this fd */
  غیرعلامتی مختصر صحیح ctrlFlags;       /* Behavioral bits.  UNIXFILE_* flags */
  صحیح آخری_Errno;                      /* The unix errno from last I/O error */
  خالی *lockingContext;               /* Locking style specific state */
  Unix_غیر_مستعمل_Fd *p_Preallocated_غیر_مستعمل;  /* Pre-allocated UnixUnusedFd */
  ثابت حرف *z_راستہ;                  /* Name of the file */
  unixShm *pShm;                      /* Shared memory segment information */
  صحیح szChunk;                        /* Configured by FCNTL_CHUNK_SIZE */
#اگر SQLITE_MAX_MMAP_سائز>0
  صحیح n_Fetch_برآمد;                      /* Number of outstanding xFetch refs */
  sqlite3_int64 mmap_سائز;             /* Usable size of mapping at pMapRegion */
  sqlite3_int64 mmap_سائز_Actual;       /* Actual size of mapping at pMapRegion */
  sqlite3_int64 mmap_سائز_Max;          /* Configured FCNTL_MMAP_SIZE value */
  خالی *pMapRegion;                   /* Memory mapped region */
#اگر_ختم
  صحیح sector_سائز;                     /* Device sector size */
  صحیح deviceCharacteristics;          /* Precomputed device characteristics */
#اگر SQLITE_ENABLE_LOCKING_STYLE
  صحیح openFlags;                      /* The flags specified at open() */
#اگر_ختم
#اگر SQLITE_ENABLE_LOCKING_STYLE || defined(__APPLE__)
  غیرعلامتی fsFlags;                   /* cached details from statfs() */
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
  غیرعلامتی iBusyTimeout;              /* Wait this many millisec on locks */
  صحیح b_بلاک_On_Connect;                /* True to block for SHARED locks */
#اگر_ختم
#اگر OS_VXWORKS
  ساخت vxworksFileId *pId;          /* Unique file ID */
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_DEBUG
  /* The next group of variables are used to track whether or not the
  ** transaction counter in bytes 24-27 of database files are updated
  ** whenever any part of the database changes.  An assertion fault will
  ** occur if a file is updated without also updating the transaction
  ** counter.  This test is made to avoid new problems similar to the
  ** one described by ticket #3584.
  */
  غیرعلامتی حرف transCntrChng;   /* True if the transaction counter changed */
  غیرعلامتی حرف dbUpdate;        /* True if any part of database file changed */
  غیرعلامتی حرف اندر_معمول_Write;   /* True if in a normal write operation */

#اگر_ختم

#اگر_تعریف_شدہ SQLITE_TEST
  /* In test mode, increase the size of this structure a bit so that
  ** it is larger than the struct CrashFile defined in test6.c.
  */
  حرف aPadding[32];
#اگر_ختم
};

/* This variable holds the process id (pid) from when the xRandomness()
** method was called.  If xOpen() is called from a different process id,
** indicating that a fork() has occurred, the PRNG will be reset.
*/
ساکن pid_t randomnessPid = 0;

/*
** Allowed values for the unixFile.ctrlFlags bitmask:
*/
#تعریف_کرو UNIXFILE_EXCL        0x01     /* Connections from one process only */
#تعریف_کرو UNIXFILE_RDONLY      0x02     /* Connection is read only */
#تعریف_کرو UNIXFILE_PERSIST_WAL 0x04     /* Persistent WAL mode */
#اگر !defined(SQLITE_DISABLE_DIRSYNC) && !defined(_AIX)
# define UNIXFILE_DIRSYNC    0x08     /* Directory sync needed */
#ورنہ
# define UNIXFILE_DIRSYNC    0x00
#اگر_ختم
#تعریف_کرو UNIXFILE_PSOW        0x10     /* SQLITE_IOCAP_POWERSAFE_OVERWRITE */
#تعریف_کرو UNIXFILE_DELETE      0x20     /* Delete on close */
#تعریف_کرو UNIXFILE_URI         0x40     /* Filename might have query parameters */
#تعریف_کرو UNIXFILE_NOLOCK      0x80     /* Do no file locking */

/*
** Include code that is common to all os_*.c files
*/
#شامل "os_common.h"

/*
** Define various macros that are missing from some systems.
*/
#اگر_غیر_تعریف_شدہ O_LARGEFILE
# define O_LARGEFILE 0
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_DISABLE_LFS
# undef O_LARGEFILE
# define O_LARGEFILE 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ O_NOFOLLOW
# define O_NOFOLLOW 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ O_ثنائی
# define O_ثنائی 0
#اگر_ختم

/*
** The threadid macro resolves to the thread-id or to 0.  Used for
** testing and debugging only.
*/
#اگر SQLITE_THREADSAFE
#تعریف_کرو threadid pthread_self()
#ورنہ
#تعریف_کرو threadid 0
#اگر_ختم

/*
** HAVE_MREMAP defaults to true on Linux and false everywhere else.
*/
#اگر !defined(HAVE_MREMAP)
# اگر defined(__linux__) && defined(GNU_ماخذ)
#  define HAVE_MREMAP 1
# ورنہ
#  define HAVE_MREMAP 0
# endif
#اگر_ختم

/*
** Explicitly call the 64-bit version of lseek() on Android. Otherwise, lseek()
** is the 32-bit version, even if _FILE_OFFSET_BITS=64 is defined.
*/
#اگر_تعریف_شدہ __ANDROID__
# define lseek lseek64
#اگر_ختم

#اگر_تعریف_شدہ __linux__
/*
** Linux-specific IOCTL magic numbers used for controlling F2FS
*/
#تعریف_کرو F2FS_IOCTL_MAGIC        0xf5
#تعریف_کرو F2_FS_IOC_آغاز_ATOMIC_WRITE     _IO(F2FS_IOCTL_MAGIC, 1)
#تعریف_کرو F2FS_IOC_COMMIT_ATOMIC_WRITE    _IO(F2FS_IOCTL_MAGIC, 2)
#تعریف_کرو F2_FS_IOC_آغاز_VOLATILE_WRITE   _IO(F2FS_IOCTL_MAGIC, 3)
#تعریف_کرو F2FS_IOC_ABORT_VOLATILE_WRITE   _IO(F2FS_IOCTL_MAGIC, 5)
#تعریف_کرو F2FS_IOC_GET_FEATURES           _IOR(F2FS_IOCTL_MAGIC, 12, u32)
#تعریف_کرو F2FS_FEATURE_ATOMIC_WRITE 0x0004
#اگر_ختم /* __linux__ */


/*
** Different Unix systems declare open() in different ways.  Same use
** open(const char*,int,mode_t).  Others use open(const char*,int,...).
** The difference is important when using a pointer to the function.
**
** The safest way to deal with the problem is to always use this wrapper
** which always has the same well-defined interface.
*/
ساکن صحیح posixOpen(ثابت حرف *zFile, صحیح flags, صحیح منوال){
  واپس open(zFile, flags, منوال);
}

/* Forward reference */
ساکن صحیح openDirectory(ثابت حرف*, صحیح*);
ساکن صحیح unixGetpagesize(خالی);

/*
** Many system calls are accessed through pointer-to-functions so that
** they may be overridden at runtime to facilitate fault injection during
** testing and sandboxing.  The following array holds the names and pointers
** to all overrideable system calls.
*/
ساکن ساخت unix_syscall {
  ثابت حرف *zName;            /* Name of the system call */
  sqlite3_syscall_ptr pCurrent; /* Current value of the system call */
  sqlite3_syscall_ptr pDefault; /* Default value */
} aSyscall[] = {
  { "open",         (sqlite3_syscall_ptr)posixOpen,  0  },
#تعریف_کرو osOpen      ((صحیح(*)(ثابت حرف*,صحیح,صحیح))aSyscall[0].pCurrent)

  { "close",        (sqlite3_syscall_ptr)close,      0  },
#تعریف_کرو osClose     ((صحیح(*)(صحیح))aSyscall[1].pCurrent)

  { "access",       (sqlite3_syscall_ptr)access,     0  },
#تعریف_کرو osAccess    ((صحیح(*)(ثابت حرف*,صحیح))aSyscall[2].pCurrent)

  { "getcwd",       (sqlite3_syscall_ptr)getcwd,     0  },
#تعریف_کرو osGetcwd    ((حرف*(*)(حرف*,size_t))aSyscall[3].pCurrent)

  { "stat",         (sqlite3_syscall_ptr)stat,       0  },
#تعریف_کرو osStat      ((صحیح(*)(ثابت حرف*,ساخت stat*))aSyscall[4].pCurrent)

/*
** The DJGPP compiler environment looks mostly like Unix, but it
** lacks the fcntl() system call.  So redefine fcntl() to be something
** that always succeeds.  This means that locking does not occur under
** DJGPP.  But it is DOS - what did you expect?
*/
#اگر_تعریف_شدہ __DJGPP__
  { "fstat",        0,                 0  },
#تعریف_کرو osFstat(a,b,c)    0
#ورنہ   
  { "fstat",        (sqlite3_syscall_ptr)fstat,      0  },
#تعریف_کرو osFstat     ((صحیح(*)(صحیح,ساخت stat*))aSyscall[5].pCurrent)
#اگر_ختم

  { "ftruncate",    (sqlite3_syscall_ptr)ftruncate,  0  },
#تعریف_کرو osFtruncate ((صحیح(*)(صحیح,off_t))aSyscall[6].pCurrent)

  { "fcntl",        (sqlite3_syscall_ptr)fcntl,      0  },
#تعریف_کرو osFcntl     ((صحیح(*)(صحیح,صحیح,...))aSyscall[7].pCurrent)

  { "read",         (sqlite3_syscall_ptr)اقرأ,       0  },
#تعریف_کرو osRead      ((ssize_t(*)(صحیح,خالی*,size_t))aSyscall[8].pCurrent)

#اگر defined(USE_PREAD) || SQLITE_ENABLE_LOCKING_STYLE
  { "pread",        (sqlite3_syscall_ptr)pread,      0  },
#ورنہ
  { "pread",        (sqlite3_syscall_ptr)0,          0  },
#اگر_ختم
#تعریف_کرو osPread     ((ssize_t(*)(صحیح,خالی*,size_t,off_t))aSyscall[9].pCurrent)

#اگر defined(USE_PREAD64)
  { "pread64",      (sqlite3_syscall_ptr)pread64,    0  },
#ورنہ
  { "pread64",      (sqlite3_syscall_ptr)0,          0  },
#اگر_ختم
#تعریف_کرو osPread64 ((ssize_t(*)(صحیح,خالی*,size_t,off64_t))aSyscall[10].pCurrent)

  { "write",        (sqlite3_syscall_ptr)اكتب,      0  },
#تعریف_کرو osWrite     ((ssize_t(*)(صحیح,ثابت خالی*,size_t))aSyscall[11].pCurrent)

#اگر defined(USE_PREAD) || SQLITE_ENABLE_LOCKING_STYLE
  { "pwrite",       (sqlite3_syscall_ptr)pwrite,     0  },
#ورنہ
  { "pwrite",       (sqlite3_syscall_ptr)0,          0  },
#اگر_ختم
#تعریف_کرو osPwrite    ((ssize_t(*)(صحیح,ثابت خالی*,size_t,off_t))\
                    aSyscall[12].pCurrent)

#اگر defined(USE_PREAD64)
  { "pwrite64",     (sqlite3_syscall_ptr)pwrite64,   0  },
#ورنہ
  { "pwrite64",     (sqlite3_syscall_ptr)0,          0  },
#اگر_ختم
#تعریف_کرو osPwrite64  ((ssize_t(*)(صحیح,ثابت خالی*,size_t,off64_t))\
                    aSyscall[13].pCurrent)

#اگر defined(HAVE_FCHMOD)
  { "fchmod",       (sqlite3_syscall_ptr)fchmod,          0  },
#تعریف_کرو osFchmod    ((صحیح(*)(صحیح,منوال_t))aSyscall[14].pCurrent)
#ورنہ
  { "fchmod",       (sqlite3_syscall_ptr)0,               0  },
#تعریف_کرو osFchmod(FID,منوال_2) 0
#اگر_ختم

#اگر defined(HAVE_POSIX_FALLOCATE) && HAVE_POSIX_FALLOCATE
  { "fallocate",    (sqlite3_syscall_ptr)posix_fallocate,  0 },
#ورنہ
  { "fallocate",    (sqlite3_syscall_ptr)0,                0 },
#اگر_ختم
#تعریف_کرو osFallocate ((صحیح(*)(صحیح,off_t,off_t))aSyscall[15].pCurrent)

  { "unlink",       (sqlite3_syscall_ptr)unlink,           0 },
#تعریف_کرو osUnlink    ((صحیح(*)(ثابت حرف*))aSyscall[16].pCurrent)

  { "openDirectory",    (sqlite3_syscall_ptr)openDirectory,      0 },
#تعریف_کرو osOpenDirectory ((صحیح(*)(ثابت حرف*,صحیح*))aSyscall[17].pCurrent)

  { "mkdir",        (sqlite3_syscall_ptr)mkdir,           0 },
#تعریف_کرو osMkdir     ((صحیح(*)(ثابت حرف*,منوال_t))aSyscall[18].pCurrent)

  { "rmdir",        (sqlite3_syscall_ptr)rmdir,           0 },
#تعریف_کرو osRmdir     ((صحیح(*)(ثابت حرف*))aSyscall[19].pCurrent)

#اگر defined(HAVE_FCHOWN)
  { "fchown",       (sqlite3_syscall_ptr)fchown,          0 },
#ورنہ
  { "fchown",       (sqlite3_syscall_ptr)0,               0 },
#اگر_ختم
#تعریف_کرو osFchown    ((صحیح(*)(صحیح,uid_t,gid_t))aSyscall[20].pCurrent)

#اگر defined(HAVE_FCHOWN)
  { "geteuid",      (sqlite3_syscall_ptr)geteuid,         0 },
#ورنہ
  { "geteuid",      (sqlite3_syscall_ptr)0,               0 },
#اگر_ختم
#تعریف_کرو osGeteuid   ((uid_t(*)(خالی))aSyscall[21].pCurrent)

#اگر (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_سائز>0) \
  && !defined(SQLITE_WASI)
  { "mmap",         (sqlite3_syscall_ptr)mmap,            0 },
#ورنہ
  { "mmap",         (sqlite3_syscall_ptr)0,               0 },
#اگر_ختم
#تعریف_کرو osMmap ((خالی*(*)(خالی*,size_t,صحیح,صحیح,صحیح,off_t))aSyscall[22].pCurrent)

#اگر (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_سائز>0) \
  && !defined(SQLITE_WASI)
  { "munmap",       (sqlite3_syscall_ptr)munmap,          0 },
#ورنہ
  { "munmap",       (sqlite3_syscall_ptr)0,               0 },
#اگر_ختم
#تعریف_کرو osMunmap ((صحیح(*)(خالی*,size_t))aSyscall[23].pCurrent)

#اگر HAVE_MREMAP && (!defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_سائز>0)
  { "mremap",       (sqlite3_syscall_ptr)mremap,          0 },
#ورنہ
  { "mremap",       (sqlite3_syscall_ptr)0,               0 },
#اگر_ختم
#تعریف_کرو osMremap ((خالی*(*)(خالی*,size_t,size_t,صحیح,...))aSyscall[24].pCurrent)

#اگر !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_سائز>0
  { "getpagesize",  (sqlite3_syscall_ptr)unixGetpagesize, 0 },
#ورنہ
  { "getpagesize",  (sqlite3_syscall_ptr)0,               0 },
#اگر_ختم
#تعریف_کرو osGetpagesize ((صحیح(*)(خالی))aSyscall[25].pCurrent)

#اگر defined(HAVE_READLINK)
  { "readlink",     (sqlite3_syscall_ptr)readlink,        0 },
#ورنہ
  { "readlink",     (sqlite3_syscall_ptr)0,               0 },
#اگر_ختم
#تعریف_کرو osReadlink ((ssize_t(*)(ثابت حرف*,حرف*,size_t))aSyscall[26].pCurrent)

#اگر defined(HAVE_LSTAT)
  { "lstat",         (sqlite3_syscall_ptr)lstat,          0 },
#ورنہ
  { "lstat",         (sqlite3_syscall_ptr)0,              0 },
#اگر_ختم
#تعریف_کرو osLstat      ((صحیح(*)(ثابت حرف*,ساخت stat*))aSyscall[27].pCurrent)

#اگر defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
# ifdef __ANDROID__
  { "ioctl", (sqlite3_syscall_ptr)(صحیح(*)(صحیح, صحیح, ...))ioctl, 0 },
#تعریف_کرو osIoctl ((صحیح(*)(صحیح,صحیح,...))aSyscall[28].pCurrent)
# ورنہ
  { "ioctl",         (sqlite3_syscall_ptr)ioctl,          0 },
#تعریف_کرو osIoctl ((صحیح(*)(صحیح,غیرعلامتی طویل,...))aSyscall[28].pCurrent)
# endif
#ورنہ
  { "ioctl",         (sqlite3_syscall_ptr)0,              0 },
#اگر_ختم

}; /* End of the overrideable system calls */


#اگر defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
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
ساکن صحیح unixPosixAdvisoryLocks(
  صحیح fd,        /* The file descriptor to analyze */
  حرف a[16]     /* Write a text description of PALs here */
){
  صحیح اندر;
  ssize_t n;
  حرف *p, *p_اگلا, *x;
  حرف z[2000];

        /*             1     */
        /*   012 4 678 01234 */
  حافظہ_نقل(a, "---/-.---.-----", 16);
  sqlite3_snprintf(حجم(z), z, "/proc/%d/fdinfo/%d", getpid(), fd);
  اندر = osOpen(z, O_RDONLY, 0);
  اگر( اندر<0 ){
    واپس SQLITE_ERROR_UNABLE;
  }
  n = osRead(اندر, z, حجم(z)-1);
  osClose(اندر);
  اگر( n<=0 ) واپس SQLITE_ERROR_UNABLE;
  z[n] = 0;

  /* We are looking for lines that begin with "lock:\t".  Examples:
  **
  ** lock: 1: POSIX  ADVISORY  READ 494716 08:02:5277597 1073741826 1073742335
  ** lock: 1: POSIX  ADVISORY  WRITE 494716 08:02:5282282 120 120
  ** lock: 2: POSIX  ADVISORY  READ 494716 08:02:5282282 123 123
  ** lock: 3: POSIX  ADVISORY  READ 494716 08:02:5282282 128 128
  */
  p_اگلا = لڑی_تلاش(z, "lock:\t");
  جب( p_اگلا ){
    حرف cType = 0;
    sqlite3_int64 i_پہلا, i_آخری;
    p = p_اگلا+6;
    p_اگلا = لڑی_تلاش(p, "lock:\t");
    اگر( p_اگلا ) p_اگلا[-1] = 0;
    اگر( (x = لڑی_تلاش(p, " READ "))!=0 ){
      cType = 'r';
      x += 6;
    }ورنہ اگر( (x = لڑی_تلاش(p, " WRITE "))!=0 ){
      cType = 'w';
      x += 7;
    }ورنہ{
      جاری;
    }
    x = strrchr(x, ' ');
    اگر( x==0 ) جاری;
    i_آخری = strtoll(x+1, 0, 10);
    *x = 0;
    x = strrchr(p, ' ');
    اگر( x==0 ) جاری;
    i_پہلا = strtoll(x+1, 0, 10);
    اگر( i_آخری>=PENDING_BYTE ){
      اگر( i_پہلا<=PENDING_BYTE && i_آخری>=PENDING_BYTE )     a[0] = cType;
      اگر( i_پہلا<=PENDING_BYTE+1 && i_آخری>=PENDING_BYTE+1 ) a[1] = cType;
      اگر( i_پہلا<=PENDING_BYTE+2 && i_آخری>=PENDING_BYTE+510 ) a[2] = cType;
    }ورنہ اگر( i_آخری<=128 ){
      اگر( i_پہلا<=128 && i_آخری>=128 ) a[4] = cType;
      اگر( i_پہلا<=120 && i_آخری>=120 ) a[6] = cType;
      اگر( i_پہلا<=121 && i_آخری>=121 ) a[7] = cType;
      اگر( i_پہلا<=122 && i_آخری>=122 ) a[8] = cType;
      اگر( i_پہلا<=123 && i_آخری>=123 ) a[10] = cType;
      اگر( i_پہلا<=124 && i_آخری>=124 ) a[11] = cType;
      اگر( i_پہلا<=125 && i_آخری>=125 ) a[12] = cType;
      اگر( i_پہلا<=126 && i_آخری>=126 ) a[13] = cType;
      اگر( i_پہلا<=127 && i_آخری>=127 ) a[14] = cType;
    }
  }
  واپس SQLITE_OK;
}
#ورنہ
#  define unixPosixAdvisoryLocks(A,B) SQLITE_ERROR_UNABLE
#اگر_ختم /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

/*
** On some systems, calls to fchown() will trigger a message in a security
** log if they come from non-root processes.  So avoid calling fchown() if
** we are not running as root.
*/
ساکن صحیح robustFchown(صحیح fd, uid_t uid, gid_t gid){
#اگر defined(HAVE_FCHOWN)
  واپس osGeteuid() ? 0 : osFchown(fd,uid,gid);
#ورنہ
  واپس 0;
#اگر_ختم
}

/*
** This is the xSetSystemCall() method of sqlite3_vfs for all of the
** "unix" VFSes.  Return SQLITE_OK upon successfully updating the
** system call pointer, or SQLITE_NOTFOUND if there is no configurable
** system call named zName.
*/
ساکن صحیح unix_سیٹ_System_Call(
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
ساکن sqlite3_syscall_ptr unixGetSystemCall(
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
ساکن ثابت حرف *unix_اگلا_System_Call(sqlite3_vfs *p, ثابت حرف *zName){
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

/*
** Do not accept any file descriptor less than this value, in order to avoid
** opening database file using file descriptors that are commonly used for
** standard input, output, and error.
*/
#اگر_غیر_تعریف_شدہ SQLITE_MINIMUM_FILE_DESCRIPTOR
# define SQLITE_MINIMUM_FILE_DESCRIPTOR 3
#اگر_ختم

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
ساکن صحیح robust_open(ثابت حرف *z, صحیح f, منوال_t m){
  صحیح fd;
  منوال_t m2 = m ? m : SQLITE_DEFAULT_FILE_PERMISSIONS;
  جب(1){
#اگر defined(O_CLOEXEC)
    fd = osOpen(z,f|O_CLOEXEC,m2);
#ورنہ
    fd = osOpen(z,f,m2);
#اگر_ختم
    اگر( fd<0 ){
      اگر( errno==EINTR ) جاری;
      توڑو;
    }
    اگر( fd>=SQLITE_MINIMUM_FILE_DESCRIPTOR ) توڑو;
    اگر( (f & (O_EXCL|O_CREAT))==(O_EXCL|O_CREAT) ){
      (خالی)osUnlink(z);
    }
    osClose(fd);
    sqlite3_log(SQLITE_WARNING,
                "attempt to open \"%s\" as file descriptor %d", z, fd);
    fd = -1;
    اگر( osOpen("/dev/null", O_RDONLY, m)<0 ) توڑو;
  }
  اگر( fd>=0 ){
    اگر( m!=0 ){
      ساخت stat statbuf;
      اگر( osFstat(fd, &statbuf)==0
       && statbuf.st_سائز==0
       && (statbuf.st_منوال&0777)!=m
      ){
        osFchmod(fd, m);
      }
    }
#اگر defined(FD_CLOEXEC) && (!defined(O_CLOEXEC) || O_CLOEXEC==0)
    osFcntl(fd, F_SETFD, osFcntl(fd, F_GETFD, 0) | FD_CLOEXEC);
#اگر_ختم
  }
  واپس fd;
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
ساکن sqlite3_mutex *unix_Big_تالا = 0;
ساکن خالی unixEnterMutex(خالی){
  أكد( sqlite3_mutex_notheld(unix_Big_تالا) );  /* Not a recursive mutex */
  sqlite3_mutex_enter(unix_Big_تالا);
}
ساکن خالی unixLeaveMutex(خالی){
  أكد( sqlite3_mutex_held(unix_Big_تالا) );
  sqlite3_mutex_leave(unix_Big_تالا);
}
#اگر_تعریف_شدہ SQLITE_DEBUG
ساکن صحیح unixMutexHeld(خالی) {
  واپس sqlite3_mutex_held(unix_Big_تالا);
}
#اگر_ختم


#اگر_تعریف_شدہ SQLITE_HAVE_OS_TRACE
/*
** Helper function for printing out trace information from debugging
** binaries. This returns the string representation of the supplied
** integer lock-type.
*/
ساکن ثابت حرف *az_File_تالا(صحیح e_File_تالا){
  انتخاب( e_File_تالا ){
    حالت NO_تالا: واپس "NONE";
    حالت SHARED_تالا: واپس "SHARED";
    حالت RESERVED_تالا: واپس "RESERVED";
    حالت PENDING_تالا: واپس "PENDING";
    حالت EXCLUSIVE_تالا: واپس "EXCLUSIVE";
  }
  واپس "ERROR";
}
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_تالا_TRACE
/*
** Print out information about all locking operations.
**
** This routine is used for troubleshooting locks on multithreaded
** platforms.  Enable by compiling with the -DSQLITE_LOCK_TRACE
** command-line option on the compiler.  This code is normally
** turned off.
*/
ساکن صحیح تالا_Trace(صحیح fd, صحیح op, ساخت flock *p){
  حرف *zOpName, *zType;
  صحیح s;
  صحیح savedErrno;
  اگر( op==F_GETLK ){
    zOpName = "GETLK";
  }ورنہ اگر( op==F_SETLK ){
    zOpName = "SETLK";
  }ورنہ{
    s = osFcntl(fd, op, p);
    sqlite3DebugPrintf("fcntl unknown %d %d %d\n", fd, op, s);
    واپس s;
  }
  اگر( p->l_type==F_RDLCK ){
    zType = "RDLCK";
  }ورنہ اگر( p->l_type==F_WRLCK ){
    zType = "WRLCK";
  }ورنہ اگر( p->l_type==F_UNLCK ){
    zType = "UNLCK";
  }ورنہ{
    أكد( 0 );
  }
  أكد( p->l_whence==SEEK_SET );
  s = osFcntl(fd, op, p);
  savedErrno = errno;
  sqlite3DebugPrintf("fcntl %d %d %s %s %d %d %d %d\n",
     threadid, fd, zOpName, zType, (صحیح)p->l_آغاز, (صحیح)p->l_len,
     (صحیح)p->l_pid, s);
  اگر( s==(-1) && op==F_SETLK && (p->l_type==F_RDLCK || p->l_type==F_WRLCK) ){
    ساخت flock l2;
    l2 = *p;
    osFcntl(fd, F_GETLK, &l2);
    اگر( l2.l_type==F_RDLCK ){
      zType = "RDLCK";
    }ورنہ اگر( l2.l_type==F_WRLCK ){
      zType = "WRLCK";
    }ورنہ اگر( l2.l_type==F_UNLCK ){
      zType = "UNLCK";
    }ورنہ{
      أكد( 0 );
    }
    sqlite3DebugPrintf("fcntl-failure-reason: %s %d %d %d\n",
       zType, (صحیح)l2.l_آغاز, (صحیح)l2.l_len, (صحیح)l2.l_pid);
  }
  errno = savedErrno;
  واپس s;
}
#تعریف_منسوخ osFcntl
#تعریف_کرو osFcntl تالا_Trace
#اگر_ختم /* SQLITE_LOCK_TRACE */

/*
** Retry ftruncate() calls that fail due to EINTR
**
** All calls to ftruncate() within this file should be made through
** this wrapper.  On the Android platform, bypassing the logic below
** could lead to a corrupt database.
*/
ساکن صحیح robust_ftruncate(صحیح h, sqlite3_int64 sz){
  صحیح rc;
#اگر_تعریف_شدہ __ANDROID__
  /* On Android, ftruncate() always uses 32-bit offsets, even if
  ** _FILE_OFFSET_BITS=64 is defined. This means it is unsafe to attempt to
  ** truncate a file to any size larger than 2GiB. Silently ignore any
  ** such attempts.  */
  اگر( sz>(sqlite3_int64)0x7FFFFFFF ){
    rc = SQLITE_OK;
  }ورنہ
#اگر_ختم
  کرو{ rc = osFtruncate(h,sz); }جب( rc<0 && errno==EINTR );
  واپس rc;
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
ساکن صحیح sqliteErrorFromPosixError(صحیح posixError, صحیح sqliteIOErr) {
  أكد( (sqliteIOErr == SQLITE_IOERR_تالا) ||
          (sqliteIOErr == SQLITE_IOERR_UNLOCK) ||
          (sqliteIOErr == SQLITE_IOERR_RDLOCK) ||
          (sqliteIOErr == SQLITE_IOERR_CHECKRESERVEDLOCK) );
  انتخاب (posixError) {
  حالت EACCES:
  حالت EAGAIN:
  حالت ETIMEDOUT:
  حالت EBUSY:
  حالت EINTR:
  حالت ENOLCK:
    /* random NFS retry error, unless during file system support
     * introspection, in which it actually means what it says */
    واپس SQLITE_BUSY;
  
  حالت EPERM:
    واپس SQLITE_PERM;
  
  طےشدہ:
    واپس sqliteIOErr;
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
ساخت vxworksFileId {
  ساخت vxworksFileId *p_اگلا;  /* Next in a list of them all */
  صحیح nRef;                     /* Number of references to this one */
  صحیح nName;                    /* Length of the zCanonicalName[] string */
  حرف *zCanonicalName;         /* Canonical filename */
};

#اگر OS_VXWORKS
/*
** All unique filenames are held on a linked list headed by this
** variable:
*/
ساکن ساخت vxworksFileId *vxworksFileList = 0;
ساکن sqlite3_mutex *vxworksMutex = 0;

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
ساکن صحیح vxworksSimplifyName(حرف *z, صحیح n){
  صحیح i, j;
  جب( n>1 && z[n-1]=='/' ){ n--; }
  برائے(i=j=0; i<n; i++){
    اگر( z[i]=='/' ){
      اگر( z[i+1]=='/' ) جاری;
      اگر( z[i+1]=='.' && i+2<n && z[i+2]=='/' ){
        i += 1;
        جاری;
      }
      اگر( z[i+1]=='.' && i+3<n && z[i+2]=='.' && z[i+3]=='/' ){
        جب( j>0 && z[j-1]!='/' ){ j--; }
        اگر( j>0 ){ j--; }
        i += 2;
        جاری;
      }
    }
    z[j++] = z[i];
  }
  z[j] = 0;
  واپس j;
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
ساکن ساخت vxworksFileId *vxworks_ڈھونڈو_File_Id(ثابت حرف *zAbsoluteName){
  ساخت vxworksFileId *pNew;         /* search key and new file ID */
  ساخت vxworksFileId *pCandidate;   /* For looping over existing file IDs */
  صحیح n;                              /* Length of zAbsoluteName string */

  أكد( zAbsoluteName[0]=='/' );
  n = (صحیح)لڑی_لمبائی(zAbsoluteName);
  pNew = sqlite3_malloc64( حجم(*pNew) + (n+1) );
  اگر( pNew==0 ) واپس 0;
  pNew->zCanonicalName = (حرف*)&pNew[1];
  حافظہ_نقل(pNew->zCanonicalName, zAbsoluteName, n+1);
  n = vxworksSimplifyName(pNew->zCanonicalName, n);

  /* Search for an existing entry that matching the canonical name.
  ** If found, increment the reference count and return a pointer to
  ** the existing file ID.
  */
  sqlite3_mutex_enter(vxworksMutex);
  برائے(pCandidate=vxworksFileList; pCandidate; pCandidate=pCandidate->p_اگلا){
    اگر( pCandidate->nName==n
     && memcmp(pCandidate->zCanonicalName, pNew->zCanonicalName, n)==0
    ){
       sqlite3_free(pNew);
       pCandidate->nRef++;
       sqlite3_mutex_leave(vxworksMutex);
       واپس pCandidate;
    }
  }

  /* No match was found.  We will make a new file ID */
  pNew->nRef = 1;
  pNew->nName = n;
  pNew->p_اگلا = vxworksFileList;
  vxworksFileList = pNew;
  sqlite3_mutex_leave(vxworksMutex);
  واپس pNew;
}

/*
** Decrement the reference count on a vxworksFileId object.  Free
** the object when the reference count reaches zero.
*/
ساکن خالی vxworksReleaseFileId(ساخت vxworksFileId *pId){
  sqlite3_mutex_enter(vxworksMutex);
  أكد( pId->nRef>0 );
  pId->nRef--;
  اگر( pId->nRef==0 ){
    ساخت vxworksFileId **pp;
    برائے(pp=&vxworksFileList; *pp && *pp!=pId; pp = &((*pp)->p_اگلا)){}
    أكد( *pp==pId );
    *pp = pId->p_اگلا;
    sqlite3_free(pId);
  }
  sqlite3_mutex_leave(vxworksMutex);
}
#اگر_ختم /* OS_VXWORKS */
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
ساخت unixFileId {
  dev_t dev;                  /* Device number */
#اگر OS_VXWORKS
  ساخت vxworksFileId *pId;  /* Unique file ID for vxworks. */
#ورنہ
  /* We are told that some versions of Android contain a bug that
  ** sizes ino_t at only 32-bits instead of 64-bits. (See
  ** https://android-review.googlesource.com/#/c/115351/3/dist/sqlite3.c)
  ** To work around this, always allocate 64-bits for the inode number.
  ** On small machines that only have 32-bit inodes, this wastes 4 bytes,
  ** but that should not be a big deal. */
  /* WAS:  ino_t ino;   */
  u64 ino;                   /* Inode number */
#اگر_ختم
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
ساخت unixInodeInfo {
  ساخت unixFileId fileId;       /* The lookup key */
  sqlite3_mutex *p_تالا_Mutex;      /* Hold this mutex for... */
  صحیح nShared;                      /* Number of SHARED locks held */
  صحیح n_تالا;                        /* Number of outstanding file locks */
  غیرعلامتی حرف e_File_تالا;          /* One of SHARED_LOCK, RESERVED_LOCK etc. */
  غیرعلامتی حرف b_Process_تالا;       /* An exclusive process lock is held */
  Unix_غیر_مستعمل_Fd *p_غیر_مستعمل;            /* Unused file descriptors to close */
  صحیح nRef;                       /* Number of pointers to this structure */
  unix_Shm_گرہ *p_Shm_گرہ;          /* Shared memory associated with this inode */
  unixInodeInfo *p_اگلا;           /* List of all unixInodeInfo objects */
  unixInodeInfo *p_پچھلا;           /*    .... doubly linked */
#اگر SQLITE_ENABLE_LOCKING_STYLE
  غیرعلامتی طویل طویل sharedByte;  /* for AFP simulated shared lock */
#اگر_ختم
#اگر OS_VXWORKS
  sem_t *pSem;                    /* Named POSIX semaphore */
  حرف aSemName[MAX_PATHNAME+2];  /* Name of that semaphore */
#اگر_ختم
};

/*
** A lists of all unixInodeInfo objects.
**
** Must hold unixBigLock in order to read or write this variable.
*/
ساکن unixInodeInfo *inodeList = 0;  /* All unixInodeInfo objects */

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** True if the inode mutex (on the unixFile.pFileMutex field) is held, or not.
** This routine is used only within assert() to help verify correct mutex
** usage.
*/
صحیح unixFileMutexHeld(unixFile *pFile){
  أكد( pFile->pInode );
  واپس sqlite3_mutex_held(pFile->pInode->p_تالا_Mutex);
}
صحیح unixFileMutexNotheld(unixFile *pFile){
  أكد( pFile->pInode );
  واپس sqlite3_mutex_notheld(pFile->pInode->p_تالا_Mutex);
}
#اگر_ختم

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
#تعریف_کرو unixLogError(a,b,c)     unix_Log_Error_At_سطر(a,b,c,__LINE__)
ساکن صحیح unix_Log_Error_At_سطر(
  صحیح errcode,                    /* SQLite error code */
  ثابت حرف *zFunc,              /* Name of OS function that failed */
  ثابت حرف *z_راستہ,              /* File path associated with error */
  صحیح i_سطر                       /* Source line number where error occurred */
){
  حرف *zErr;                     /* Message from strerror() or equivalent */
  صحیح iErrno = errno;             /* Saved syscall error number */

  /* If this is not a threadsafe build (SQLITE_THREADSAFE==0), then use
  ** the strerror() function to obtain the human-readable error message
  ** equivalent to errno. Otherwise, use strerror_r().
  */
#اگر SQLITE_THREADSAFE && defined(HAVE_STRERROR_R)
  حرف aErr[80];
  memset(aErr, 0, حجم(aErr));
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
#اگر (defined(STRERROR_R_CHAR_P) || defined(__USE_GNU)) \
  && !defined(ANDROID) && !defined(__ANDROID__)
  zErr =
# endif
  strerror_r(iErrno, aErr, حجم(aErr)-1);

#وگرنہ_اگر SQLITE_THREADSAFE
  /* This is a threadsafe build, but strerror_r() is not available. */
  zErr = "";
#ورنہ
  /* Non-threadsafe build, use strerror(). */
  zErr = strerror(iErrno);
#اگر_ختم

  اگر( z_راستہ==0 ) z_راستہ = "";
  sqlite3_log(errcode,
      "os_unix.c:%d: (%d) %s(%s) - %s",
      i_سطر, iErrno, zFunc, z_راستہ, zErr
  );

  واپس errcode;
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
ساکن خالی robust_close(unixFile *pFile, صحیح h, صحیح lineno){
  اگر( osClose(h) ){
    unix_Log_Error_At_سطر(SQLITE_IOERR_CLOSE, "close",
                       pFile ? pFile->z_راستہ : 0, lineno);
  }
}

/*
** Set the pFile->lastErrno.  Do this in a subroutine as that provides
** a convenient place to set a breakpoint.
*/
ساکن خالی store_آخری_Errno(unixFile *pFile, صحیح error){
  pFile->آخری_Errno = error;
}

/*
** Close all file descriptors accumulated in the unixInodeInfo->pUnused list.
*/
ساکن خالی closePendingFds(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  Unix_غیر_مستعمل_Fd *p;
  Unix_غیر_مستعمل_Fd *p_اگلا;
  أكد( unixFileMutexHeld(pFile) );
  برائے(p=pInode->p_غیر_مستعمل; p; p=p_اگلا){
    p_اگلا = p->p_اگلا;
    robust_close(pFile, p->fd, __LINE__);
    sqlite3_free(p);
  }
  pInode->p_غیر_مستعمل = 0;
}

/*
** Release a unixInodeInfo structure previously allocated by findInodeInfo().
**
** The global mutex must be held when this routine is called, but the mutex
** on the inode being deleted must NOT be held.
*/
ساکن خالی releaseInodeInfo(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  أكد( unixMutexHeld() );
  أكد( unixFileMutexNotheld(pFile) );
  اگر( ALWAYS(pInode) ){
    pInode->nRef--;
    اگر( pInode->nRef==0 ){
      أكد( pInode->p_Shm_گرہ==0 );
      sqlite3_mutex_enter(pInode->p_تالا_Mutex);
      closePendingFds(pFile);
      sqlite3_mutex_leave(pInode->p_تالا_Mutex);
      اگر( pInode->p_پچھلا ){
        أكد( pInode->p_پچھلا->p_اگلا==pInode );
        pInode->p_پچھلا->p_اگلا = pInode->p_اگلا;
      }ورنہ{
        أكد( inodeList==pInode );
        inodeList = pInode->p_اگلا;
      }
      اگر( pInode->p_اگلا ){
        أكد( pInode->p_اگلا->p_پچھلا==pInode );
        pInode->p_اگلا->p_پچھلا = pInode->p_پچھلا;
      }
      sqlite3_mutex_free(pInode->p_تالا_Mutex);
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
ساکن صحیح ڈھونڈو_Inode_Info(
  unixFile *pFile,               /* Unix file with file desc used in the key */
  unixInodeInfo **ppInode        /* Return the unixInodeInfo object here */
){
  صحیح rc;                        /* System call return code */
  صحیح fd;                        /* The file descriptor for pFile */
  ساخت unixFileId fileId;      /* Lookup key for the unixInodeInfo */
  ساخت stat statbuf;           /* Low-level file information */
  unixInodeInfo *pInode = 0;     /* Candidate unixInodeInfo object */

  أكد( unixMutexHeld() );

  /* Get low-level information about the file that we can used to
  ** create a unique name for the file.
  */
  fd = pFile->h;
  rc = osFstat(fd, &statbuf);
  اگر( rc!=0 ){
    store_آخری_Errno(pFile, errno);
#اگر defined(EOVERFLOW) && defined(SQLITE_DISABLE_LFS)
    اگر( pFile->آخری_Errno==EOVERFLOW ) واپس SQLITE_NOLFS;
#اگر_ختم
    واپس SQLITE_IOERR;
  }

#اگر_تعریف_شدہ __APPLE__
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
  اگر( statbuf.st_سائز==0 && (pFile->fsFlags & SQLITE_FSFLAGS_IS_MSDOS)!=0 ){
    کرو{ rc = osWrite(fd, "S", 1); }جب( rc<0 && errno==EINTR );
    اگر( rc!=1 ){
      store_آخری_Errno(pFile, errno);
      واپس SQLITE_IOERR;
    }
    اگر( fsync(fd) ){
      store_آخری_Errno(pFile, errno);
      واپس SQLITE_IOERR_FSYNC;
    }
    rc = osFstat(fd, &statbuf);
    اگر( rc!=0 ){
      store_آخری_Errno(pFile, errno);
      واپس SQLITE_IOERR;
    }
  }
#اگر_ختم

  memset(&fileId, 0, حجم(fileId));
  fileId.dev = statbuf.st_dev;
#اگر OS_VXWORKS
  fileId.pId = pFile->pId;
#ورنہ
  fileId.ino = (u64)statbuf.st_ino;
#اگر_ختم
  أكد( unixMutexHeld() );
  pInode = inodeList;
  جب( pInode && memcmp(&fileId, &pInode->fileId, حجم(fileId)) ){
    pInode = pInode->p_اگلا;
  }
  اگر( pInode==0 ){
    pInode = sqlite3_malloc64( حجم(*pInode) );
    اگر( pInode==0 ){
      واپس SQLITE_NOMEM_BKPT;
    }
    memset(pInode, 0, حجم(*pInode));
    حافظہ_نقل(&pInode->fileId, &fileId, حجم(fileId));
    اگر( sqlite3GlobalConfig.bCoreMutex ){
      pInode->p_تالا_Mutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      اگر( pInode->p_تالا_Mutex==0 ){
        sqlite3_free(pInode);
        واپس SQLITE_NOMEM_BKPT;
      }
    }
    pInode->nRef = 1;
    أكد( unixMutexHeld() );
    pInode->p_اگلا = inodeList;
    pInode->p_پچھلا = 0;
    اگر( inodeList ) inodeList->p_پچھلا = pInode;
    inodeList = pInode;
  }ورنہ{
    pInode->nRef++;
  }
  *ppInode = pInode;
  واپس SQLITE_OK;
}

/*
** Return TRUE if pFile has been renamed or unlinked since it was first opened.
*/
ساکن صحیح fileHasMoved(unixFile *pFile){
#اگر OS_VXWORKS
  واپس pFile->pInode!=0 && pFile->pId!=pFile->pInode->fileId.pId;
#ورنہ
  ساخت stat buf;
  واپس pFile->pInode!=0 &&
      (osStat(pFile->z_راستہ, &buf)!=0
         || (u64)buf.st_ino!=pFile->pInode->fileId.ino);
#اگر_ختم
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
ساکن خالی verifyDbFile(unixFile *pFile){
  ساخت stat buf;
  صحیح rc;

  /* These verifications occurs for the main database only */
  اگر( pFile->ctrlFlags & UNIXFILE_NOLOCK ) واپس;

  rc = osFstat(pFile->h, &buf);
  اگر( rc!=0 ){
    sqlite3_log(SQLITE_WARNING, "cannot fstat db file %s", pFile->z_راستہ);
    واپس;
  }
  اگر( buf.st_nlink==0 ){
    sqlite3_log(SQLITE_WARNING, "file unlinked while open: %s", pFile->z_راستہ);
    واپس;
  }
  اگر( buf.st_nlink>1 ){
    sqlite3_log(SQLITE_WARNING, "multiple links to file: %s", pFile->z_راستہ);
    واپس;
  }
  اگر( fileHasMoved(pFile) ){
    sqlite3_log(SQLITE_WARNING, "file renamed while open: %s", pFile->z_راستہ);
    واپس;
  }
}


/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
ساکن صحیح unix_Check_Reserved_تالا(sqlite3_file *id, صحیح *p_Res_برآمد){
  صحیح rc = SQLITE_OK;
  صحیح reserved = 0;
  unixFile *pFile = (unixFile*)id;

  SimulateIOError( واپس SQLITE_IOERR_CHECKRESERVEDLOCK; );

  أكد( pFile );
  أكد( pFile->e_File_تالا<=SHARED_تالا );
  sqlite3_mutex_enter(pFile->pInode->p_تالا_Mutex);

  /* Check if a thread in this process holds such a lock */
  اگر( pFile->pInode->e_File_تالا>SHARED_تالا ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it.
  */
#اگر_غیر_تعریف_شدہ __DJGPP__
  اگر( !reserved && !pFile->pInode->b_Process_تالا ){
    ساخت flock تالا;
    تالا.l_whence = SEEK_SET;
    تالا.l_آغاز = RESERVED_BYTE;
    تالا.l_len = 1;
    تالا.l_type = F_WRLCK;
    اگر( osFcntl(pFile->h, F_GETLK, &تالا) ){
      rc = SQLITE_IOERR_CHECKRESERVEDLOCK;
      store_آخری_Errno(pFile, errno);
    } ورنہ اگر( تالا.l_type!=F_UNLCK ){
      reserved = 1;
    }
  }
#اگر_ختم

  sqlite3_mutex_leave(pFile->pInode->p_تالا_Mutex);
  OSTRACE(("TEST WR-LOCK %d %d %d (unix)\n", pFile->h, rc, reserved));

  *p_Res_برآمد = reserved;
  واپس rc;
}

/* Forward declaration*/
ساکن صحیح unixSleep(sqlite3_vfs*,صحیح);

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
#اگر_غیر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
# define os_سیٹ_Posix_Advisory_تالا(h,x,t) osFcntl(h,F_SETLK,x)
#ورنہ
ساکن صحیح os_سیٹ_Posix_Advisory_تالا(
  صحیح h,                /* The file descriptor on which to take the lock */
  ساخت flock *p_تالا,  /* The description of the lock */
  unixFile *pFile       /* Structure holding timeout value */
){
  صحیح rc = 0;

  اگر( pFile->iBusyTimeout==0 ){
    /* unixFile->iBusyTimeout is set to 0. In this case, attempt a 
    ** non-blocking lock. */
    rc = osFcntl(h,F_SETLK,p_تالا);
  }ورنہ{
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
#اگر_تعریف_شدہ SQLITE_TEST
    صحیح tm = pFile->iBusyTimeout;
    جب( tm>0 ){
      rc = osFcntl(h,F_SETLK,p_تالا);
      اگر( rc==0 ) توڑو;
      unixSleep(0,1000);
      tm--;
    }
#ورنہ
    rc = osFcntl(h,F_SETLK,p_تالا);
    pFile->iBusyTimeout = 0;
#اگر_ختم
    /* End of code to replace with real blocking-locks code. */
  }

  واپس rc;
}
#اگر_ختم /* SQLITE_ENABLE_SETLK_TIMEOUT */


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
ساکن صحیح unix_File_تالا(unixFile *pFile, ساخت flock *p_تالا){
  صحیح rc;
  unixInodeInfo *pInode = pFile->pInode;
  أكد( pInode!=0 );
  أكد( sqlite3_mutex_held(pInode->p_تالا_Mutex) );
  اگر( (pFile->ctrlFlags & (UNIXFILE_EXCL|UNIXFILE_RDONLY))==UNIXFILE_EXCL ){
    اگر( pInode->b_Process_تالا==0 ){
      ساخت flock تالا;
      /* assert( pInode->nLock==0 ); <-- Not true if unix-excl READONLY used */
      تالا.l_whence = SEEK_SET;
      تالا.l_آغاز = SHARED_پہلا;
      تالا.l_len = SHARED_سائز;
      تالا.l_type = F_WRLCK;
      rc = os_سیٹ_Posix_Advisory_تالا(pFile->h, &تالا, pFile);
      اگر( rc<0 ) واپس rc;
      pInode->b_Process_تالا = 1;
      pInode->n_تالا++;
    }ورنہ{
      rc = 0;
    }
  }ورنہ{
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
    اگر( pFile->b_بلاک_On_Connect && p_تالا->l_type==F_RDLCK
     && p_تالا->l_آغاز==SHARED_پہلا && p_تالا->l_len==SHARED_سائز
    ){
      rc = osFcntl(pFile->h, F_SETLKW, p_تالا);
    }ورنہ
#اگر_ختم
    rc = os_سیٹ_Posix_Advisory_تالا(pFile->h, p_تالا, pFile);
  }
  واپس rc;
}

#اگر !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
/* Forward reference */
ساکن صحیح unix_Is_Sharing_Shm_گرہ(unixFile*);
#ورنہ
#تعریف_کرو unix_Is_Sharing_Shm_گرہ(pFile) (0)
#اگر_ختم

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
ساکن صحیح unix_تالا(sqlite3_file *id, صحیح e_File_تالا){
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
  صحیح rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  ساخت flock تالا;
  صحیح tErrno = 0;

  أكد( pFile );
  OSTRACE(("LOCK    %d %s was %s(%s,%d) pid=%d (unix)\n", pFile->h,
      az_File_تالا(e_File_تالا), az_File_تالا(pFile->e_File_تالا),
      az_File_تالا(pFile->pInode->e_File_تالا), pFile->pInode->nShared,
      osGetpid(0)));

  /* If there is already a lock of this type or more restrictive on the
  ** unixFile, do nothing. Don't use the end_lock: exit path, as
  ** unixEnterMutex() hasn't been called yet.
  */
  اگر( pFile->e_File_تالا>=e_File_تالا ){
    OSTRACE(("LOCK    %d %s ok (already held) (unix)\n", pFile->h,
            az_File_تالا(e_File_تالا)));
    واپس SQLITE_OK;
  }

  /* Make sure the locking sequence is correct.
  **  (1) We never move from unlocked to anything higher than shared lock.
  **  (2) SQLite never explicitly requests a pending lock.
  **  (3) A shared lock is always held when a reserve lock is requested.
  */
  أكد( pFile->e_File_تالا!=NO_تالا || e_File_تالا==SHARED_تالا );
  أكد( e_File_تالا!=PENDING_تالا );
  أكد( e_File_تالا!=RESERVED_تالا || pFile->e_File_تالا==SHARED_تالا );

  /* This mutex is needed because pFile->pInode is shared across threads
  */
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->p_تالا_Mutex);

  /* If some thread using this PID has a lock via a different unixFile*
  ** handle that precludes the requested lock, return BUSY.
  */
  اگر( (pFile->e_File_تالا!=pInode->e_File_تالا &&
          (pInode->e_File_تالا>=PENDING_تالا || e_File_تالا>SHARED_تالا))
  ){
    rc = SQLITE_BUSY;
    جاؤ انتہا_تالا;
  }

  /* If a SHARED lock is requested, and some thread using this PID already
  ** has a SHARED or RESERVED lock, then increment reference counts and
  ** return SQLITE_OK.
  */
  اگر( e_File_تالا==SHARED_تالا &&
      (pInode->e_File_تالا==SHARED_تالا || pInode->e_File_تالا==RESERVED_تالا) ){
    أكد( e_File_تالا==SHARED_تالا );
    أكد( pFile->e_File_تالا==0 );
    أكد( pInode->nShared>0 );
    pFile->e_File_تالا = SHARED_تالا;
    pInode->nShared++;
    pInode->n_تالا++;
    جاؤ انتہا_تالا;
  }


  /* A PENDING lock is needed before acquiring a SHARED lock and before
  ** acquiring an EXCLUSIVE lock.  For the SHARED lock, the PENDING will
  ** be released.
  */
  تالا.l_len = 1L;
  تالا.l_whence = SEEK_SET;
  اگر( e_File_تالا==SHARED_تالا
   || (e_File_تالا==EXCLUSIVE_تالا && pFile->e_File_تالا==RESERVED_تالا)
  ){
    تالا.l_type = (e_File_تالا==SHARED_تالا?F_RDLCK:F_WRLCK);
    تالا.l_آغاز = PENDING_BYTE;
    اگر( unix_File_تالا(pFile, &تالا) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_تالا);
      اگر( rc!=SQLITE_BUSY ){
        store_آخری_Errno(pFile, tErrno);
      }
      جاؤ انتہا_تالا;
    }ورنہ اگر( e_File_تالا==EXCLUSIVE_تالا ){
      pFile->e_File_تالا = PENDING_تالا;
      pInode->e_File_تالا = PENDING_تالا;
    }
  }


  /* If control gets to this point, then actually go ahead and make
  ** operating system calls for the specified lock.
  */
  اگر( e_File_تالا==SHARED_تالا ){
    أكد( pInode->nShared==0 );
    أكد( pInode->e_File_تالا==0 );
    أكد( rc==SQLITE_OK );

    /* Now get the read-lock */
    تالا.l_آغاز = SHARED_پہلا;
    تالا.l_len = SHARED_سائز;
    اگر( unix_File_تالا(pFile, &تالا) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_تالا);
    }

    /* Drop the temporary PENDING lock */
    تالا.l_آغاز = PENDING_BYTE;
    تالا.l_len = 1L;
    تالا.l_type = F_UNLCK;
    اگر( unix_File_تالا(pFile, &تالا) && rc==SQLITE_OK ){
      /* This could happen with a network mount */
      tErrno = errno;
      rc = SQLITE_IOERR_UNLOCK;
    }

    اگر( rc ){
      اگر( rc!=SQLITE_BUSY ){
        store_آخری_Errno(pFile, tErrno);
      }
      جاؤ انتہا_تالا;
    }ورنہ{
      pFile->e_File_تالا = SHARED_تالا;
      pInode->n_تالا++;
      pInode->nShared = 1;
    }
  }ورنہ اگر( e_File_تالا==EXCLUSIVE_تالا && pInode->nShared>1 ){
    /* We are trying for an exclusive lock but another thread in this
    ** same process is still holding a shared lock. */
    rc = SQLITE_BUSY;
  }ورنہ اگر( unix_Is_Sharing_Shm_گرہ(pFile) ){
    /* We are in WAL mode and attempting to delete the SHM and WAL
    ** files due to closing the connection or changing out of WAL mode,
    ** but another process still holds locks on the SHM file, thus
    ** indicating that database locks have been broken, perhaps due
    ** to a rogue close(open(dbFile)) or similar.
    */
    rc = SQLITE_BUSY;
  }ورنہ{
    /* The request was for a RESERVED or EXCLUSIVE lock.  It is
    ** assumed that there is a SHARED or greater lock on the file
    ** already.
    */
    أكد( 0!=pFile->e_File_تالا );
    تالا.l_type = F_WRLCK;

    أكد( e_File_تالا==RESERVED_تالا || e_File_تالا==EXCLUSIVE_تالا );
    اگر( e_File_تالا==RESERVED_تالا ){
      تالا.l_آغاز = RESERVED_BYTE;
      تالا.l_len = 1L;
    }ورنہ{
      تالا.l_آغاز = SHARED_پہلا;
      تالا.l_len = SHARED_سائز;
    }

    اگر( unix_File_تالا(pFile, &تالا) ){
      tErrno = errno;
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_تالا);
      اگر( rc!=SQLITE_BUSY ){
        store_آخری_Errno(pFile, tErrno);
      }
    }
  }


#اگر_تعریف_شدہ SQLITE_DEBUG
  /* Set up the transaction-counter change checking flags when
  ** transitioning from a SHARED to a RESERVED lock.  The change
  ** from SHARED to RESERVED marks the beginning of a normal
  ** write operation (not a hot journal rollback).
  */
  اگر( rc==SQLITE_OK
   && pFile->e_File_تالا<=SHARED_تالا
   && e_File_تالا==RESERVED_تالا
  ){
    pFile->transCntrChng = 0;
    pFile->dbUpdate = 0;
    pFile->اندر_معمول_Write = 1;
  }
#اگر_ختم

  اگر( rc==SQLITE_OK ){
    pFile->e_File_تالا = e_File_تالا;
    pInode->e_File_تالا = e_File_تالا;
  }

انتہا_تالا:
  sqlite3_mutex_leave(pInode->p_تالا_Mutex);
  OSTRACE(("LOCK    %d %s %s (unix)\n", pFile->h, az_File_تالا(e_File_تالا),
      rc==SQLITE_OK ? "ok" : "failed"));
  واپس rc;
}

/*
** Add the file descriptor used by file handle pFile to the corresponding
** pUnused list.
*/
ساکن خالی سیٹ_Pending_Fd(unixFile *pFile){
  unixInodeInfo *pInode = pFile->pInode;
  Unix_غیر_مستعمل_Fd *p = pFile->p_Preallocated_غیر_مستعمل;
  أكد( unixFileMutexHeld(pFile) );
  p->p_اگلا = pInode->p_غیر_مستعمل;
  pInode->p_غیر_مستعمل = p;
  pFile->h = -1;
  pFile->p_Preallocated_غیر_مستعمل = 0;
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
ساکن صحیح posixUnlock(sqlite3_file *id, صحیح e_File_تالا, صحیح handleNFSUnlock){
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  ساخت flock تالا;
  صحیح rc = SQLITE_OK;

  أكد( pFile );
  OSTRACE(("UNLOCK  %d %d was %d(%d,%d) pid=%d (unix)\n", pFile->h, e_File_تالا,
      pFile->e_File_تالا, pFile->pInode->e_File_تالا, pFile->pInode->nShared,
      osGetpid(0)));

  أكد( e_File_تالا<=SHARED_تالا );
  اگر( pFile->e_File_تالا<=e_File_تالا ){
    واپس SQLITE_OK;
  }
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->p_تالا_Mutex);
  أكد( pInode->nShared!=0 );
  اگر( pFile->e_File_تالا>SHARED_تالا ){
    أكد( pInode->e_File_تالا==pFile->e_File_تالا );

#اگر_تعریف_شدہ SQLITE_DEBUG
    /* When reducing a lock such that other processes can start
    ** reading the database file again, make sure that the
    ** transaction counter was updated if any part of the database
    ** file changed.  If the transaction counter is not updated,
    ** other connections to the same file might not realize that
    ** the file has changed and hence might not know to flush their
    ** cache.  The use of a stale cache can lead to database corruption.
    */
    pFile->اندر_معمول_Write = 0;
#اگر_ختم

    /* downgrading to a shared lock on NFS involves clearing the write lock
    ** before establishing the readlock - to avoid a race condition we downgrade
    ** the lock in 2 blocks, so that part of the range will be covered by a
    ** write lock until the rest is covered by a read lock:
    **  1:   [WWWWW]
    **  2:   [....W]
    **  3:   [RRRRW]
    **  4:   [RRRR.]
    */
    اگر( e_File_تالا==SHARED_تالا ){
#اگر !defined(__APPLE__) || !SQLITE_ENABLE_LOCKING_STYLE
      (خالی)handleNFSUnlock;
      أكد( handleNFSUnlock==0 );
#اگر_ختم
#اگر defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
      اگر( handleNFSUnlock ){
        صحیح tErrno;               /* Error code from system call errors */
        off_t div_سائز = SHARED_سائز - 1;
      
        تالا.l_type = F_UNLCK;
        تالا.l_whence = SEEK_SET;
        تالا.l_آغاز = SHARED_پہلا;
        تالا.l_len = div_سائز;
        اگر( unix_File_تالا(pFile, &تالا)==(-1) ){
          tErrno = errno;
          rc = SQLITE_IOERR_UNLOCK;
          store_آخری_Errno(pFile, tErrno);
          جاؤ انتہا_unlock;
        }
        تالا.l_type = F_RDLCK;
        تالا.l_whence = SEEK_SET;
        تالا.l_آغاز = SHARED_پہلا;
        تالا.l_len = div_سائز;
        اگر( unix_File_تالا(pFile, &تالا)==(-1) ){
          tErrno = errno;
          rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_RDLOCK);
          اگر( IS_تالا_ERROR(rc) ){
            store_آخری_Errno(pFile, tErrno);
          }
          جاؤ انتہا_unlock;
        }
        تالا.l_type = F_UNLCK;
        تالا.l_whence = SEEK_SET;
        تالا.l_آغاز = SHARED_پہلا+div_سائز;
        تالا.l_len = SHARED_سائز-div_سائز;
        اگر( unix_File_تالا(pFile, &تالا)==(-1) ){
          tErrno = errno;
          rc = SQLITE_IOERR_UNLOCK;
          store_آخری_Errno(pFile, tErrno);
          جاؤ انتہا_unlock;
        }
      }ورنہ
#اگر_ختم /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
      {
        تالا.l_type = F_RDLCK;
        تالا.l_whence = SEEK_SET;
        تالا.l_آغاز = SHARED_پہلا;
        تالا.l_len = SHARED_سائز;
        اگر( unix_File_تالا(pFile, &تالا) ){
          /* In theory, the call to unixFileLock() cannot fail because another
          ** process is holding an incompatible lock. If it does, this
          ** indicates that the other process is not following the locking
          ** protocol. If this happens, return SQLITE_IOERR_RDLOCK. Returning
          ** SQLITE_BUSY would confuse the upper layer (in practice it causes
          ** an assert to fail). */
          rc = SQLITE_IOERR_RDLOCK;
          store_آخری_Errno(pFile, errno);
          جاؤ انتہا_unlock;
        }
      }
    }
    تالا.l_type = F_UNLCK;
    تالا.l_whence = SEEK_SET;
    تالا.l_آغاز = PENDING_BYTE;
    تالا.l_len = 2L;  أكد( PENDING_BYTE+1==RESERVED_BYTE );
    اگر( unix_File_تالا(pFile, &تالا)==0 ){
      pInode->e_File_تالا = SHARED_تالا;
    }ورنہ{
      rc = SQLITE_IOERR_UNLOCK;
      store_آخری_Errno(pFile, errno);
      جاؤ انتہا_unlock;
    }
  }
  اگر( e_File_تالا==NO_تالا ){
    /* Decrement the shared lock counter.  Release the lock using an
    ** OS call only when all threads in this same process have released
    ** the lock.
    */
    pInode->nShared--;
    اگر( pInode->nShared==0 ){
      تالا.l_type = F_UNLCK;
      تالا.l_whence = SEEK_SET;
      تالا.l_آغاز = تالا.l_len = 0L;
      اگر( unix_File_تالا(pFile, &تالا)==0 ){
        pInode->e_File_تالا = NO_تالا;
      }ورنہ{
        rc = SQLITE_IOERR_UNLOCK;
        store_آخری_Errno(pFile, errno);
        pInode->e_File_تالا = NO_تالا;
        pFile->e_File_تالا = NO_تالا;
      }
    }

    /* Decrement the count of locks against this same file.  When the
    ** count reaches zero, close any other file descriptors whose close
    ** was deferred because of outstanding locks.
    */
    pInode->n_تالا--;
    أكد( pInode->n_تالا>=0 );
    اگر( pInode->n_تالا==0 ) closePendingFds(pFile);
  }

انتہا_unlock:
  sqlite3_mutex_leave(pInode->p_تالا_Mutex);
  اگر( rc==SQLITE_OK ){
    pFile->e_File_تالا = e_File_تالا;
  }
  واپس rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
ساکن صحیح unixUnlock(sqlite3_file *id, صحیح e_File_تالا){
#اگر SQLITE_MAX_MMAP_سائز>0
  أكد( e_File_تالا==SHARED_تالا || ((unixFile *)id)->n_Fetch_برآمد==0 );
#اگر_ختم
  واپس posixUnlock(id, e_File_تالا, 0);
}

#اگر SQLITE_MAX_MMAP_سائز>0
ساکن صحیح unixMapfile(unixFile *pFd, i64 nByte);
ساکن خالی unixUnmapfile(unixFile *pFd);
#اگر_ختم

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
ساکن صحیح closeUnixFile(sqlite3_file *id){
  unixFile *pFile = (unixFile*)id;
#اگر SQLITE_MAX_MMAP_سائز>0
  unixUnmapfile(pFile);
#اگر_ختم
  اگر( pFile->h>=0 ){
    robust_close(pFile, pFile->h, __LINE__);
    pFile->h = -1;
  }
#اگر OS_VXWORKS
  اگر( pFile->pId ){
    اگر( pFile->ctrlFlags & UNIXFILE_DELETE ){
      osUnlink(pFile->pId->zCanonicalName);
    }
    vxworksReleaseFileId(pFile->pId);
    pFile->pId = 0;
  }
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_UNLINK_AFTER_CLOSE
  اگر( pFile->ctrlFlags & UNIXFILE_DELETE ){
    osUnlink(pFile->z_راستہ);
    sqlite3_free(*(حرف**)&pFile->z_راستہ);
    pFile->z_راستہ = 0;
  }
#اگر_ختم
  OSTRACE(("CLOSE   %-3d\n", pFile->h));
  Open_شمار(-1);
  sqlite3_free(pFile->p_Preallocated_غیر_مستعمل);
  memset(pFile, 0, حجم(unixFile));
  واپس SQLITE_OK;
}

/*
** Close a file.
*/
ساکن صحیح unixClose(sqlite3_file *id){
  صحیح rc = SQLITE_OK;
  unixFile *pFile = (unixFile *)id;
  unixInodeInfo *pInode = pFile->pInode;

  أكد( pInode!=0 );
  verifyDbFile(pFile);
  unixUnlock(id, NO_تالا);
  أكد( unixFileMutexNotheld(pFile) );
  unixEnterMutex();

  /* unixFile.pInode is always valid here. Otherwise, a different close
  ** routine (e.g. nolockClose()) would be called instead.
  */
  أكد( pFile->pInode->n_تالا>0 || pFile->pInode->b_Process_تالا==0 );
  sqlite3_mutex_enter(pInode->p_تالا_Mutex);
  اگر( pInode->n_تالا ){
    /* If there are outstanding locks, do not actually close the file just
    ** yet because that would clear those locks.  Instead, add the file
    ** descriptor to pInode->pUnused list.  It will be automatically closed
    ** when the last lock is cleared.
    */
    سیٹ_Pending_Fd(pFile);
  }
  sqlite3_mutex_leave(pInode->p_تالا_Mutex);
  releaseInodeInfo(pFile);
  أكد( pFile->pShm==0 );
  rc = closeUnixFile(id);
  unixLeaveMutex();
  واپس rc;
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

ساکن صحیح nolock_Check_Reserved_تالا(sqlite3_file *Not_مستعمل, صحیح *p_Res_برآمد){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  *p_Res_برآمد = 0;
  واپس SQLITE_OK;
}
ساکن صحیح nolock_تالا(sqlite3_file *Not_مستعمل, صحیح NotUsed2){
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  واپس SQLITE_OK;
}
ساکن صحیح nolockUnlock(sqlite3_file *Not_مستعمل, صحیح NotUsed2){
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  واپس SQLITE_OK;
}

/*
** Close the file.
*/
ساکن صحیح nolockClose(sqlite3_file *id) {
  واپس closeUnixFile(id);
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
#تعریف_کرو DOTLOCK_SUFFIX ".lock"

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If the caller holds a SHARED
** or greater lock when it is called, then it is assumed that no other
** client may hold RESERVED. Or, if the caller holds no lock, then it
** is assumed another client holds RESERVED if the lock-file exists.
*/
ساکن صحیح dotlock_Check_Reserved_تالا(sqlite3_file *id, صحیح *p_Res_برآمد) {
  unixFile *pFile = (unixFile*)id;
  SimulateIOError( واپس SQLITE_IOERR_CHECKRESERVEDLOCK; );

  اگر( pFile->e_File_تالا>=SHARED_تالا ){
    *p_Res_برآمد = 0;
  }ورنہ{
    *p_Res_برآمد = osAccess((ثابت حرف*)pFile->lockingContext, 0)==0;
  }
  OSTRACE(("TEST WR-LOCK %d %d %d (dotlock)\n", pFile->h, 0, *p_Res_برآمد));
  واپس SQLITE_OK;
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
ساکن صحیح dotlock_تالا(sqlite3_file *id, صحیح e_File_تالا) {
  unixFile *pFile = (unixFile*)id;
  حرف *z_تالا_File = (حرف *)pFile->lockingContext;
  صحیح rc = SQLITE_OK;


  /* If we have any lock, then the lock file already exists.  All we have
  ** to do is adjust our internal record of the lock level.
  */
  اگر( pFile->e_File_تالا > NO_تالا ){
    pFile->e_File_تالا = e_File_تالا;
    /* Always update the timestamp on the old file */
#اگر_تعریف_شدہ HAVE_UTIME
    utime(z_تالا_File, تہی);
#ورنہ
    utimes(z_تالا_File, تہی);
#اگر_ختم
    واپس SQLITE_OK;
  }

  /* grab an exclusive lock */
  rc = osMkdir(z_تالا_File, 0777);
  اگر( rc<0 ){
    /* failed to open/create the lock directory */
    صحیح tErrno = errno;
    اگر( EEXIST == tErrno ){
      rc = SQLITE_BUSY;
    } ورنہ {
      rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_تالا);
      اگر( rc!=SQLITE_BUSY ){
        store_آخری_Errno(pFile, tErrno);
      }
    }
    واپس rc;
  }

  /* got it, set the type and return ok */
  pFile->e_File_تالا = e_File_تالا;
  واپس rc;
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
ساکن صحیح dotlockUnlock(sqlite3_file *id, صحیح e_File_تالا) {
  unixFile *pFile = (unixFile*)id;
  حرف *z_تالا_File = (حرف *)pFile->lockingContext;
  صحیح rc;

  أكد( pFile );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (dotlock)\n", pFile->h, e_File_تالا,
           pFile->e_File_تالا, osGetpid(0)));
  أكد( e_File_تالا<=SHARED_تالا );

  /* no-op if possible */
  اگر( pFile->e_File_تالا==e_File_تالا ){
    واپس SQLITE_OK;
  }

  /* To downgrade to shared, simply update our internal notion of the
  ** lock state.  No need to mess with the file on disk.
  */
  اگر( e_File_تالا==SHARED_تالا ){
    pFile->e_File_تالا = SHARED_تالا;
    واپس SQLITE_OK;
  }

  /* To fully unlock the database, delete the lock file */
  أكد( e_File_تالا==NO_تالا );
  rc = osRmdir(z_تالا_File);
  اگر( rc<0 ){
    صحیح tErrno = errno;
    اگر( tErrno==ENOENT ){
      rc = SQLITE_OK;
    }ورنہ{
      rc = SQLITE_IOERR_UNLOCK;
      store_آخری_Errno(pFile, tErrno);
    }
    واپس rc;
  }
  pFile->e_File_تالا = NO_تالا;
  واپس SQLITE_OK;
}

/*
** Close a file.  Make sure the lock has been released before closing.
*/
ساکن صحیح dotlockClose(sqlite3_file *id) {
  unixFile *pFile = (unixFile*)id;
  أكد( id!=0 );
  dotlockUnlock(id, NO_تالا);
  sqlite3_free(pFile->lockingContext);
  واپس closeUnixFile(id);
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
#اگر SQLITE_ENABLE_LOCKING_STYLE

/*
** Retry flock() calls that fail with EINTR
*/
#اگر_تعریف_شدہ EINTR
ساکن صحیح robust_flock(صحیح fd, صحیح op){
  صحیح rc;
  کرو{ rc = flock(fd,op); }جب( rc<0 && errno==EINTR );
  واپس rc;
}
#ورنہ
# define robust_flock(a,b) flock(a,b)
#اگر_ختم
   

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
ساکن صحیح flock_Check_Reserved_تالا(sqlite3_file *id, صحیح *p_Res_برآمد){
#اگر_تعریف_شدہ SQLITE_DEBUG
  unixFile *pFile = (unixFile*)id;
#ورنہ
  غیر_مستعمل_PARAMETER(id);
#اگر_ختم

  SimulateIOError( واپس SQLITE_IOERR_CHECKRESERVEDLOCK; );

  أكد( pFile );
  أكد( pFile->e_File_تالا<=SHARED_تالا );

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
  *p_Res_برآمد = 0;

  واپس SQLITE_OK;
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
ساکن صحیح flock_تالا(sqlite3_file *id, صحیح e_File_تالا) {
  صحیح rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;

  أكد( pFile );

  /* if we already have a lock, it is exclusive.
  ** Just adjust level and punt on outta here. */
  اگر (pFile->e_File_تالا > NO_تالا) {
    pFile->e_File_تالا = e_File_تالا;
    واپس SQLITE_OK;
  }

  /* grab an exclusive lock */

  اگر (robust_flock(pFile->h, LOCK_EX | LOCK_NB)) {
    صحیح tErrno = errno;
    /* didn't get, must be busy */
    rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_تالا);
    اگر( IS_تالا_ERROR(rc) ){
      store_آخری_Errno(pFile, tErrno);
    }
  } ورنہ {
    /* got it, set the type and return ok */
    pFile->e_File_تالا = e_File_تالا;
  }
  OSTRACE(("LOCK    %d %s %s (flock)\n", pFile->h, az_File_تالا(e_File_تالا),
           rc==SQLITE_OK ? "ok" : "failed"));
#اگر_تعریف_شدہ SQLITE_IGNORE_FLOCK_تالا_ERRORS
  اگر( (rc & 0xff) == SQLITE_IOERR ){
    rc = SQLITE_BUSY;
  }
#اگر_ختم /* SQLITE_IGNORE_FLOCK_LOCK_ERRORS */
  واپس rc;
}


/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
ساکن صحیح flockUnlock(sqlite3_file *id, صحیح e_File_تالا) {
  unixFile *pFile = (unixFile*)id;

  أكد( pFile );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (flock)\n", pFile->h, e_File_تالا,
           pFile->e_File_تالا, osGetpid(0)));
  أكد( e_File_تالا<=SHARED_تالا );

  /* no-op if possible */
  اگر( pFile->e_File_تالا==e_File_تالا ){
    واپس SQLITE_OK;
  }

  /* shared can just be set because we always have an exclusive */
  اگر (e_File_تالا==SHARED_تالا) {
    pFile->e_File_تالا = e_File_تالا;
    واپس SQLITE_OK;
  }

  /* no, really, unlock. */
  اگر( robust_flock(pFile->h, LOCK_UN) ){
#اگر_تعریف_شدہ SQLITE_IGNORE_FLOCK_تالا_ERRORS
    واپس SQLITE_OK;
#اگر_ختم /* SQLITE_IGNORE_FLOCK_LOCK_ERRORS */
    واپس SQLITE_IOERR_UNLOCK;
  }ورنہ{
    pFile->e_File_تالا = NO_تالا;
    واپس SQLITE_OK;
  }
}

/*
** Close a file.
*/
ساکن صحیح flockClose(sqlite3_file *id) {
  أكد( id!=0 );
  flockUnlock(id, NO_تالا);
  واپس closeUnixFile(id);
}

#اگر_ختم /* SQLITE_ENABLE_LOCKING_STYLE && !OS_VXWORK */

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
#اگر OS_VXWORKS

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
ساکن صحیح sem_XCheck_Reserved_تالا(sqlite3_file *id, صحیح *p_Res_برآمد) {
  صحیح rc = SQLITE_OK;
  صحیح reserved = 0;
  unixFile *pFile = (unixFile*)id;

  SimulateIOError( واپس SQLITE_IOERR_CHECKRESERVEDLOCK; );

  أكد( pFile );

  /* Check if a thread in this process holds such a lock */
  اگر( pFile->e_File_تالا>SHARED_تالا ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it. */
  اگر( !reserved ){
    sem_t *pSem = pFile->pInode->pSem;

    اگر( sem_trywait(pSem)==-1 ){
      صحیح tErrno = errno;
      اگر( EAGAIN != tErrno ){
        rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_CHECKRESERVEDLOCK);
        store_آخری_Errno(pFile, tErrno);
      } ورنہ {
        /* someone else has the lock when we are in NO_LOCK */
        reserved = (pFile->e_File_تالا < SHARED_تالا);
      }
    }ورنہ{
      /* we could have it if we want it */
      sem_post(pSem);
    }
  }
  OSTRACE(("TEST WR-LOCK %d %d %d (sem)\n", pFile->h, rc, reserved));

  *p_Res_برآمد = reserved;
  واپس rc;
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
ساکن صحیح semXLock(sqlite3_file *id, صحیح e_File_تالا) {
  unixFile *pFile = (unixFile*)id;
  sem_t *pSem = pFile->pInode->pSem;
  صحیح rc = SQLITE_OK;

  /* if we already have a lock, it is exclusive.
  ** Just adjust level and punt on outta here. */
  اگر (pFile->e_File_تالا > NO_تالا) {
    pFile->e_File_تالا = e_File_تالا;
    rc = SQLITE_OK;
    جاؤ sem_انتہا_تالا;
  }

  /* lock semaphore now but bail out when already locked. */
  اگر( sem_trywait(pSem)==-1 ){
    rc = SQLITE_BUSY;
    جاؤ sem_انتہا_تالا;
  }

  /* got it, set the type and return ok */
  pFile->e_File_تالا = e_File_تالا;

 sem_انتہا_تالا:
  واپس rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
ساکن صحیح semXUnlock(sqlite3_file *id, صحیح e_File_تالا) {
  unixFile *pFile = (unixFile*)id;
  sem_t *pSem = pFile->pInode->pSem;

  أكد( pFile );
  أكد( pSem );
  OSTRACE(("UNLOCK  %d %d was %d pid=%d (sem)\n", pFile->h, e_File_تالا,
           pFile->e_File_تالا, osGetpid(0)));
  أكد( e_File_تالا<=SHARED_تالا );

  /* no-op if possible */
  اگر( pFile->e_File_تالا==e_File_تالا ){
    واپس SQLITE_OK;
  }

  /* shared can just be set because we always have an exclusive */
  اگر (e_File_تالا==SHARED_تالا) {
    pFile->e_File_تالا = e_File_تالا;
    واپس SQLITE_OK;
  }

  /* no, really unlock. */
  اگر ( sem_post(pSem)==-1 ) {
    صحیح rc, tErrno = errno;
    rc = sqliteErrorFromPosixError(tErrno, SQLITE_IOERR_UNLOCK);
    اگر( IS_تالا_ERROR(rc) ){
      store_آخری_Errno(pFile, tErrno);
    }
    واپس rc;
  }
  pFile->e_File_تالا = NO_تالا;
  واپس SQLITE_OK;
}

/*
 ** Close a file.
 */
ساکن صحیح semXClose(sqlite3_file *id) {
  اگر( id ){
    unixFile *pFile = (unixFile*)id;
    semXUnlock(id, NO_تالا);
    أكد( pFile );
    أكد( unixFileMutexNotheld(pFile) );
    unixEnterMutex();
    releaseInodeInfo(pFile);
    unixLeaveMutex();
    closeUnixFile(id);
  }
  واپس SQLITE_OK;
}

#اگر_ختم /* OS_VXWORKS */
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

#اگر defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
** The afpLockingContext structure contains all afp lock specific state
*/
قسمتعریف ساخت afpLockingContext afpLockingContext;
ساخت afpLockingContext {
  صحیح reserved;
  ثابت حرف *db_راستہ;             /* Name of the open file */
};

ساخت Byte_Range_تالا_PB2
{
  غیرعلامتی طویل طویل انحراف;        /* offset to first byte to lock */
  غیرعلامتی طویل طویل لمبائی;        /* nbr of bytes to lock */
  غیرعلامتی طویل طویل ret_Range_آغاز; /* nbr of 1st byte locked if successful */
  غیرعلامتی حرف un_تالا_Flag;         /* 1 = unlock, 0 = lock */
  غیرعلامتی حرف آغاز_انتہا_Flag;       /* 1=rel to end of fork, 0=rel to start */
  صحیح fd;                           /* file desc to assoc this lock with */
};

#تعریف_کرو afpfsByteRangeLock2FSCTL        _IOWR('z', 23, ساخت Byte_Range_تالا_PB2)

/*
** This is a utility for setting or clearing a bit-range lock on an
** AFP filesystem.
**
** Return SQLITE_OK on success, SQLITE_BUSY on failure.
*/
ساکن صحیح afp_سیٹ_تالا(
  ثابت حرف *راستہ,              /* Name of the file to be locked or unlocked */
  unixFile *pFile,               /* Open file descriptor on path */
  غیرعلامتی طویل طویل انحراف,     /* First byte to be locked */
  غیرعلامتی طویل طویل لمبائی,     /* Number of bytes to lock */
  صحیح سیٹ_تالا_Flag                /* True to set lock.  False to clear lock */
){
  ساخت Byte_Range_تالا_PB2 pb;
  صحیح err;

  pb.un_تالا_Flag = سیٹ_تالا_Flag ? 0 : 1;
  pb.آغاز_انتہا_Flag = 0;
  pb.انحراف = انحراف;
  pb.لمبائی = لمبائی;
  pb.fd = pFile->h;

  OSTRACE(("AFPSETLOCK [%s] for %d%s in range %llx:%llx\n",
    (سیٹ_تالا_Flag?"ON":"OFF"), pFile->h, (pb.fd==-1?"[testval-1]":""),
    انحراف, لمبائی));
  err = fsctl(راستہ, afpfsByteRangeLock2FSCTL, &pb, 0);
  اگر ( err==-1 ) {
    صحیح rc;
    صحیح tErrno = errno;
    OSTRACE(("AFPSETLOCK failed to fsctl() '%s' %d %s\n",
             راستہ, tErrno, strerror(tErrno)));
#اگر_تعریف_شدہ SQLITE_IGNORE_AFP_تالا_ERRORS
    rc = SQLITE_BUSY;
#ورنہ
    rc = sqliteErrorFromPosixError(tErrno,
                    سیٹ_تالا_Flag ? SQLITE_IOERR_تالا : SQLITE_IOERR_UNLOCK);
#اگر_ختم /* SQLITE_IGNORE_AFP_LOCK_ERRORS */
    اگر( IS_تالا_ERROR(rc) ){
      store_آخری_Errno(pFile, tErrno);
    }
    واپس rc;
  } ورنہ {
    واپس SQLITE_OK;
  }
}

/*
** This routine checks if there is a RESERVED lock held on the specified
** file by this or any other process. If such a lock is held, set *pResOut
** to a non-zero value otherwise *pResOut is set to zero.  The return value
** is set to SQLITE_OK unless an I/O error occurs during lock checking.
*/
ساکن صحیح afp_Check_Reserved_تالا(sqlite3_file *id, صحیح *p_Res_برآمد){
  صحیح rc = SQLITE_OK;
  صحیح reserved = 0;
  unixFile *pFile = (unixFile*)id;
  afpLockingContext *context;

  SimulateIOError( واپس SQLITE_IOERR_CHECKRESERVEDLOCK; );

  أكد( pFile );
  context = (afpLockingContext *) pFile->lockingContext;
  اگر( context->reserved ){
    *p_Res_برآمد = 1;
    واپس SQLITE_OK;
  }
  sqlite3_mutex_enter(pFile->pInode->p_تالا_Mutex);
  /* Check if a thread in this process holds such a lock */
  اگر( pFile->pInode->e_File_تالا>SHARED_تالا ){
    reserved = 1;
  }

  /* Otherwise see if some other process holds it.
   */
  اگر( !reserved ){
    /* lock the RESERVED byte */
    صحیح lrc = afp_سیٹ_تالا(context->db_راستہ, pFile, RESERVED_BYTE, 1,1);
    اگر( SQLITE_OK==lrc ){
      /* if we succeeded in taking the reserved lock, unlock it to restore
      ** the original state */
      lrc = afp_سیٹ_تالا(context->db_راستہ, pFile, RESERVED_BYTE, 1, 0);
    } ورنہ {
      /* if we failed to get the lock then someone else must have it */
      reserved = 1;
    }
    اگر( IS_تالا_ERROR(lrc) ){
      rc=lrc;
    }
  }

  sqlite3_mutex_leave(pFile->pInode->p_تالا_Mutex);
  OSTRACE(("TEST WR-LOCK %d %d %d (afp)\n", pFile->h, rc, reserved));

  *p_Res_برآمد = reserved;
  واپس rc;
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
ساکن صحیح afp_تالا(sqlite3_file *id, صحیح e_File_تالا){
  صحیح rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode = pFile->pInode;
  afpLockingContext *context = (afpLockingContext *) pFile->lockingContext;

  أكد( pFile );
  OSTRACE(("LOCK    %d %s was %s(%s,%d) pid=%d (afp)\n", pFile->h,
           az_File_تالا(e_File_تالا), az_File_تالا(pFile->e_File_تالا),
           az_File_تالا(pInode->e_File_تالا), pInode->nShared , osGetpid(0)));

  /* If there is already a lock of this type or more restrictive on the
  ** unixFile, do nothing. Don't use the afp_end_lock: exit path, as
  ** unixEnterMutex() hasn't been called yet.
  */
  اگر( pFile->e_File_تالا>=e_File_تالا ){
    OSTRACE(("LOCK    %d %s ok (already held) (afp)\n", pFile->h,
           az_File_تالا(e_File_تالا)));
    واپس SQLITE_OK;
  }

  /* Make sure the locking sequence is correct
  **  (1) We never move from unlocked to anything higher than shared lock.
  **  (2) SQLite never explicitly requests a pending lock.
  **  (3) A shared lock is always held when a reserve lock is requested.
  */
  أكد( pFile->e_File_تالا!=NO_تالا || e_File_تالا==SHARED_تالا );
  أكد( e_File_تالا!=PENDING_تالا );
  أكد( e_File_تالا!=RESERVED_تالا || pFile->e_File_تالا==SHARED_تالا );

  /* This mutex is needed because pFile->pInode is shared across threads
  */
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->p_تالا_Mutex);

  /* If some thread using this PID has a lock via a different unixFile*
  ** handle that precludes the requested lock, return BUSY.
  */
  اگر( (pFile->e_File_تالا!=pInode->e_File_تالا &&
       (pInode->e_File_تالا>=PENDING_تالا || e_File_تالا>SHARED_تالا))
     ){
    rc = SQLITE_BUSY;
    جاؤ afp_انتہا_تالا;
  }

  /* If a SHARED lock is requested, and some thread using this PID already
  ** has a SHARED or RESERVED lock, then increment reference counts and
  ** return SQLITE_OK.
  */
  اگر( e_File_تالا==SHARED_تالا &&
     (pInode->e_File_تالا==SHARED_تالا || pInode->e_File_تالا==RESERVED_تالا) ){
    أكد( e_File_تالا==SHARED_تالا );
    أكد( pFile->e_File_تالا==0 );
    أكد( pInode->nShared>0 );
    pFile->e_File_تالا = SHARED_تالا;
    pInode->nShared++;
    pInode->n_تالا++;
    جاؤ afp_انتہا_تالا;
  }
  
  /* A PENDING lock is needed before acquiring a SHARED lock and before
  ** acquiring an EXCLUSIVE lock.  For the SHARED lock, the PENDING will
  ** be released.
  */
  اگر( e_File_تالا==SHARED_تالا
      || (e_File_تالا==EXCLUSIVE_تالا && pFile->e_File_تالا<PENDING_تالا)
  ){
    صحیح failed;
    failed = afp_سیٹ_تالا(context->db_راستہ, pFile, PENDING_BYTE, 1, 1);
    اگر (failed) {
      rc = failed;
      جاؤ afp_انتہا_تالا;
    }
  }

  /* If control gets to this point, then actually go ahead and make
  ** operating system calls for the specified lock.
  */
  اگر( e_File_تالا==SHARED_تالا ){
    صحیح lrc1, lrc2, lrc1Errno = 0;
    طویل lk, نقاب;
  
    أكد( pInode->nShared==0 );
    أكد( pInode->e_File_تالا==0 );
      
    نقاب = (حجم(طویل)==8) ? سب_سے_بڑا_INT64 : 0x7fffffff;
    /* Now get the read-lock SHARED_LOCK */
    /* note that the quality of the randomness doesn't matter that much */
    lk = random();
    pInode->sharedByte = (lk & نقاب)%(SHARED_سائز - 1);
    lrc1 = afp_سیٹ_تالا(context->db_راستہ, pFile,
          SHARED_پہلا+pInode->sharedByte, 1, 1);
    اگر( IS_تالا_ERROR(lrc1) ){
      lrc1Errno = pFile->آخری_Errno;
    }
    /* Drop the temporary PENDING lock */
    lrc2 = afp_سیٹ_تالا(context->db_راستہ, pFile, PENDING_BYTE, 1, 0);
  
    اگر( IS_تالا_ERROR(lrc1) ) {
      store_آخری_Errno(pFile, lrc1Errno);
      rc = lrc1;
      جاؤ afp_انتہا_تالا;
    } ورنہ اگر( IS_تالا_ERROR(lrc2) ){
      rc = lrc2;
      جاؤ afp_انتہا_تالا;
    } ورنہ اگر( lrc1 != SQLITE_OK ) {
      rc = lrc1;
    } ورنہ {
      pFile->e_File_تالا = SHARED_تالا;
      pInode->n_تالا++;
      pInode->nShared = 1;
    }
  }ورنہ اگر( e_File_تالا==EXCLUSIVE_تالا && pInode->nShared>1 ){
    /* We are trying for an exclusive lock but another thread in this
     ** same process is still holding a shared lock. */
    rc = SQLITE_BUSY;
  }ورنہ{
    /* The request was for a RESERVED or EXCLUSIVE lock.  It is
    ** assumed that there is a SHARED or greater lock on the file
    ** already.
    */
    صحیح failed = 0;
    أكد( 0!=pFile->e_File_تالا );
    اگر (e_File_تالا >= RESERVED_تالا && pFile->e_File_تالا < RESERVED_تالا) {
        /* Acquire a RESERVED lock */
        failed = afp_سیٹ_تالا(context->db_راستہ, pFile, RESERVED_BYTE, 1,1);
      اگر( !failed ){
        context->reserved = 1;
      }
    }
    اگر (!failed && e_File_تالا == EXCLUSIVE_تالا) {
      /* Acquire an EXCLUSIVE lock */

      /* Remove the shared lock before trying the range.  we'll need to
      ** reestablish the shared lock if we can't get the  afpUnlock
      */
      اگر( !(failed = afp_سیٹ_تالا(context->db_راستہ, pFile, SHARED_پہلا +
                         pInode->sharedByte, 1, 0)) ){
        صحیح failed2 = SQLITE_OK;
        /* now attempt to get the exclusive lock range */
        failed = afp_سیٹ_تالا(context->db_راستہ, pFile, SHARED_پہلا,
                               SHARED_سائز, 1);
        اگر( failed && (failed2 = afp_سیٹ_تالا(context->db_راستہ, pFile,
                       SHARED_پہلا + pInode->sharedByte, 1, 1)) ){
          /* Can't reestablish the shared lock.  Sqlite can't deal, this is
          ** a critical I/O error
          */
          rc = ((failed & 0xff) == SQLITE_IOERR) ? failed2 :
               SQLITE_IOERR_تالا;
          جاؤ afp_انتہا_تالا;
        }
      }ورنہ{
        rc = failed;
      }
    }
    اگر( failed ){
      rc = failed;
    }
  }

  اگر( rc==SQLITE_OK ){
    pFile->e_File_تالا = e_File_تالا;
    pInode->e_File_تالا = e_File_تالا;
  }ورنہ اگر( e_File_تالا==EXCLUSIVE_تالا ){
    pFile->e_File_تالا = PENDING_تالا;
    pInode->e_File_تالا = PENDING_تالا;
  }

afp_انتہا_تالا:
  sqlite3_mutex_leave(pInode->p_تالا_Mutex);
  OSTRACE(("LOCK    %d %s %s (afp)\n", pFile->h, az_File_تالا(e_File_تالا),
         rc==SQLITE_OK ? "ok" : "failed"));
  واپس rc;
}

/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
ساکن صحیح afpUnlock(sqlite3_file *id, صحیح e_File_تالا) {
  صحیح rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  unixInodeInfo *pInode;
  afpLockingContext *context = (afpLockingContext *) pFile->lockingContext;
  صحیح skipShared = 0;

  أكد( pFile );
  OSTRACE(("UNLOCK  %d %d was %d(%d,%d) pid=%d (afp)\n", pFile->h, e_File_تالا,
           pFile->e_File_تالا, pFile->pInode->e_File_تالا, pFile->pInode->nShared,
           osGetpid(0)));

  أكد( e_File_تالا<=SHARED_تالا );
  اگر( pFile->e_File_تالا<=e_File_تالا ){
    واپس SQLITE_OK;
  }
  pInode = pFile->pInode;
  sqlite3_mutex_enter(pInode->p_تالا_Mutex);
  أكد( pInode->nShared!=0 );
  اگر( pFile->e_File_تالا>SHARED_تالا ){
    أكد( pInode->e_File_تالا==pFile->e_File_تالا );
  
#اگر_تعریف_شدہ SQLITE_DEBUG
    /* When reducing a lock such that other processes can start
    ** reading the database file again, make sure that the
    ** transaction counter was updated if any part of the database
    ** file changed.  If the transaction counter is not updated,
    ** other connections to the same file might not realize that
    ** the file has changed and hence might not know to flush their
    ** cache.  The use of a stale cache can lead to database corruption.
    */
    أكد( pFile->اندر_معمول_Write==0
           || pFile->dbUpdate==0
           || pFile->transCntrChng==1 );
    pFile->اندر_معمول_Write = 0;
#اگر_ختم
  
    اگر( pFile->e_File_تالا==EXCLUSIVE_تالا ){
      rc = afp_سیٹ_تالا(context->db_راستہ, pFile, SHARED_پہلا, SHARED_سائز, 0);
      اگر( rc==SQLITE_OK && (e_File_تالا==SHARED_تالا || pInode->nShared>1) ){
        /* only re-establish the shared lock if necessary */
        صحیح shared_تالا_Byte = SHARED_پہلا+pInode->sharedByte;
        rc = afp_سیٹ_تالا(context->db_راستہ, pFile, shared_تالا_Byte, 1, 1);
      } ورنہ {
        skipShared = 1;
      }
    }
    اگر( rc==SQLITE_OK && pFile->e_File_تالا>=PENDING_تالا ){
      rc = afp_سیٹ_تالا(context->db_راستہ, pFile, PENDING_BYTE, 1, 0);
    }
    اگر( rc==SQLITE_OK && pFile->e_File_تالا>=RESERVED_تالا && context->reserved ){
      rc = afp_سیٹ_تالا(context->db_راستہ, pFile, RESERVED_BYTE, 1, 0);
      اگر( !rc ){
        context->reserved = 0;
      }
    }
    اگر( rc==SQLITE_OK && (e_File_تالا==SHARED_تالا || pInode->nShared>1)){
      pInode->e_File_تالا = SHARED_تالا;
    }
  }
  اگر( rc==SQLITE_OK && e_File_تالا==NO_تالا ){

    /* Decrement the shared lock counter.  Release the lock using an
    ** OS call only when all threads in this same process have released
    ** the lock.
    */
    غیرعلامتی طویل طویل shared_تالا_Byte = SHARED_پہلا+pInode->sharedByte;
    pInode->nShared--;
    اگر( pInode->nShared==0 ){
      اگر( !skipShared ){
        rc = afp_سیٹ_تالا(context->db_راستہ, pFile, shared_تالا_Byte, 1, 0);
      }
      اگر( !rc ){
        pInode->e_File_تالا = NO_تالا;
        pFile->e_File_تالا = NO_تالا;
      }
    }
    اگر( rc==SQLITE_OK ){
      pInode->n_تالا--;
      أكد( pInode->n_تالا>=0 );
      اگر( pInode->n_تالا==0 ) closePendingFds(pFile);
    }
  }

  sqlite3_mutex_leave(pInode->p_تالا_Mutex);
  اگر( rc==SQLITE_OK ){
    pFile->e_File_تالا = e_File_تالا;
  }
  واپس rc;
}

/*
** Close a file & cleanup AFP specific locking context
*/
ساکن صحیح afpClose(sqlite3_file *id) {
  صحیح rc = SQLITE_OK;
  unixFile *pFile = (unixFile*)id;
  أكد( id!=0 );
  afpUnlock(id, NO_تالا);
  أكد( unixFileMutexNotheld(pFile) );
  unixEnterMutex();
  اگر( pFile->pInode ){
    unixInodeInfo *pInode = pFile->pInode;
    sqlite3_mutex_enter(pInode->p_تالا_Mutex);
    اگر( pInode->n_تالا ){
      /* If there are outstanding locks, do not actually close the file just
      ** yet because that would clear those locks.  Instead, add the file
      ** descriptor to pInode->aPending.  It will be automatically closed when
      ** the last lock is cleared.
      */
      سیٹ_Pending_Fd(pFile);
    }
    sqlite3_mutex_leave(pInode->p_تالا_Mutex);
  }
  releaseInodeInfo(pFile);
  sqlite3_free(pFile->lockingContext);
  rc = closeUnixFile(id);
  unixLeaveMutex();
  واپس rc;
}

#اگر_ختم /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
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

#اگر defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
 ** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
 ** must be either NO_LOCK or SHARED_LOCK.
 **
 ** If the locking level of the file descriptor is already at or below
 ** the requested locking level, this routine is a no-op.
 */
ساکن صحیح nfsUnlock(sqlite3_file *id, صحیح e_File_تالا){
  واپس posixUnlock(id, e_File_تالا, 1);
}

#اگر_ختم /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
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
ساکن صحیح seekAndRead(unixFile *id, sqlite3_int64 انحراف, خالی *pBuf, صحیح cnt){
  صحیح got;
  صحیح پیشگی = 0;
#اگر (!defined(USE_PREAD) && !defined(USE_PREAD64))
  i64 new_انحراف;
#اگر_ختم
  TIMER_آغاز;
  أكد( cnt==(cnt&0x1ffff) );
  أكد( id->h>2 );
  کرو{
#اگر defined(USE_PREAD)
    got = osPread(id->h, pBuf, cnt, انحراف);
    SimulateIOError( got = -1 );
#وگرنہ_اگر defined(USE_PREAD64)
    got = osPread64(id->h, pBuf, cnt, انحراف);
    SimulateIOError( got = -1 );
#ورنہ
    new_انحراف = lseek(id->h, انحراف, SEEK_SET);
    SimulateIOError( new_انحراف = -1 );
    اگر( new_انحراف<0 ){
      store_آخری_Errno((unixFile*)id, errno);
      واپس -1;
    }
    got = osRead(id->h, pBuf, cnt);
#اگر_ختم
    اگر( got==cnt ) توڑو;
    اگر( got<0 ){
      اگر( errno==EINTR ){ got = 1; جاری; }
      پیشگی = 0;
      store_آخری_Errno((unixFile*)id,  errno);
      توڑو;
    }ورنہ اگر( got>0 ){
      cnt -= got;
      انحراف += got;
      پیشگی += got;
      pBuf = (خالی*)(got + (حرف*)pBuf);
    }
  }جب( got>0 );
  TIMER_انتہا;
  OSTRACE(("READ    %-3d %5d %7lld %llu\n",
            id->h, got+پیشگی, انحراف-پیشگی, TIMER_ELAPSED));
  واپس got+پیشگی;
}

/*
** Read data from a file into a buffer.  Return SQLITE_OK if all
** bytes were read successfully and SQLITE_IOERR if anything goes
** wrong.
*/
ساکن صحیح unixRead(
  sqlite3_file *id,
  خالی *pBuf,
  صحیح amt,
  sqlite3_int64 انحراف
){
  unixFile *pFile = (unixFile *)id;
  صحیح got;
  أكد( id );
  أكد( انحراف>=0 );
  أكد( amt>0 );

  /* If this is a database file (not a journal, super-journal or temp
  ** file), the bytes in the locking range should never be read or written. */
#اگر 0
  أكد( pFile->p_Preallocated_غیر_مستعمل==0
       || انحراف>=PENDING_BYTE+512
       || انحراف+amt<=PENDING_BYTE
  );
#اگر_ختم

#اگر SQLITE_MAX_MMAP_سائز>0
  /* Deal with as much of this read request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  اگر( انحراف<pFile->mmap_سائز ){
    اگر( انحراف+amt <= pFile->mmap_سائز ){
      حافظہ_نقل(pBuf, &((u8 *)(pFile->pMapRegion))[انحراف], amt);
      واپس SQLITE_OK;
    }ورنہ{
      صحیح nCopy = pFile->mmap_سائز - انحراف;
      حافظہ_نقل(pBuf, &((u8 *)(pFile->pMapRegion))[انحراف], nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      انحراف += nCopy;
    }
  }
#اگر_ختم

  got = seekAndRead(pFile, انحراف, pBuf, amt);
  اگر( got==amt ){
    واپس SQLITE_OK;
  }ورنہ اگر( got<0 ){
    /* pFile->lastErrno has been set by seekAndRead().
    ** Usually we return SQLITE_IOERR_READ here, though for some
    ** kinds of errors we return SQLITE_IOERR_CORRUPTFS.  The
    ** SQLITE_IOERR_CORRUPTFS will be converted into SQLITE_CORRUPT
    ** prior to returning to the application by the sqlite3ApiExit()
    ** routine.
    */
    انتخاب( pFile->آخری_Errno ){
      حالت ERANGE:
      حالت EIO:
#اگر_تعریف_شدہ ENXIO
      حالت ENXIO:
#اگر_ختم
#اگر_تعریف_شدہ EDEVERR
      حالت EDEVERR:
#اگر_ختم
        واپس SQLITE_IOERR_CORRUPTFS;
    }
    واپس SQLITE_IOERR_READ;
  }ورنہ{
    store_آخری_Errno(pFile, 0);   /* not a system error */
    /* Unread parts of the buffer must be zero-filled */
    memset(&((حرف*)pBuf)[got], 0, amt-got);
    واپس SQLITE_IOERR_SHORT_READ;
  }
}

/*
** Attempt to seek the file-descriptor passed as the first argument to
** absolute offset iOff, then attempt to write nBuf bytes of data from
** pBuf to it. If an error occurs, return -1 and set *piErrno. Otherwise,
** return the actual number of bytes written (which may be less than
** nBuf).
*/
ساکن صحیح seekAndWriteFd(
  صحیح fd,                         /* File descriptor to write to */
  i64 iOff,                       /* File offset to begin writing at */
  ثابت خالی *pBuf,               /* Copy data from this buffer to the file */
  صحیح nBuf,                       /* Size of buffer pBuf in bytes */
  صحیح *piErrno                    /* OUT: Error number if error occurs */
){
  صحیح rc = 0;                     /* Value returned by system call */

  أكد( nBuf==(nBuf&0x1ffff) );
  أكد( fd>2 );
  أكد( piErrno!=0 );
  nBuf &= 0x1ffff;
  TIMER_آغاز;

#اگر defined(USE_PREAD)
  کرو{ rc = (صحیح)osPwrite(fd, pBuf, nBuf, iOff); }جب( rc<0 && errno==EINTR );
#وگرنہ_اگر defined(USE_PREAD64)
  کرو{ rc = (صحیح)osPwrite64(fd, pBuf, nBuf, iOff);}جب( rc<0 && errno==EINTR);
#ورنہ
  کرو{
    i64 iSeek = lseek(fd, iOff, SEEK_SET);
    SimulateIOError( iSeek = -1 );
    اگر( iSeek<0 ){
      rc = -1;
      توڑو;
    }
    rc = osWrite(fd, pBuf, nBuf);
  }جب( rc<0 && errno==EINTR );
#اگر_ختم

  TIMER_انتہا;
  OSTRACE(("WRITE   %-3d %5d %7lld %llu\n", fd, rc, iOff, TIMER_ELAPSED));

  اگر( rc<0 ) *piErrno = errno;
  واپس rc;
}


/*
** Seek to the offset in id->offset then read cnt bytes into pBuf.
** Return the number of bytes actually read.  Update the offset.
**
** To avoid stomping the errno value on a failed write the lastErrno value
** is set before returning.
*/
ساکن صحیح seekAndWrite(unixFile *id, i64 انحراف, ثابت خالی *pBuf, صحیح cnt){
  واپس seekAndWriteFd(id->h, انحراف, pBuf, cnt, &id->آخری_Errno);
}


/*
** Write data from a buffer into a file.  Return SQLITE_OK on success
** or some other error code on failure.
*/
ساکن صحیح unixWrite(
  sqlite3_file *id,
  ثابت خالی *pBuf,
  صحیح amt,
  sqlite3_int64 انحراف
){
  unixFile *pFile = (unixFile*)id;
  صحیح wrote = 0;
  أكد( id );
  أكد( amt>0 );

  /* If this is a database file (not a journal, super-journal or temp
  ** file), the bytes in the locking range should never be read or written. */
#اگر 0
  أكد( pFile->p_Preallocated_غیر_مستعمل==0
       || انحراف>=PENDING_BYTE+512
       || انحراف+amt<=PENDING_BYTE
  );
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DEBUG
  /* If we are doing a normal write to a database file (as opposed to
  ** doing a hot-journal rollback or a write to some file other than a
  ** normal database file) then record the fact that the database
  ** has changed.  If the transaction counter is modified, record that
  ** fact too.
  */
  اگر( pFile->اندر_معمول_Write ){
    pFile->dbUpdate = 1;  /* The database has been modified */
    اگر( انحراف<=24 && انحراف+amt>=27 ){
      صحیح rc;
      حرف oldCntr[4];
      SimulateIOErrorBenign(1);
      rc = seekAndRead(pFile, 24, oldCntr, 4);
      SimulateIOErrorBenign(0);
      اگر( rc!=4 || memcmp(oldCntr, &((حرف*)pBuf)[24-انحراف], 4)!=0 ){
        pFile->transCntrChng = 1;  /* The transaction counter has changed */
      }
    }
  }
#اگر_ختم

#اگر defined(SQLITE_MMAP_READWRITE) && SQLITE_MAX_MMAP_سائز>0
  /* Deal with as much of this write request as possible by transferring
  ** data from the memory mapping using memcpy().  */
  اگر( انحراف<pFile->mmap_سائز ){
    اگر( انحراف+amt <= pFile->mmap_سائز ){
      حافظہ_نقل(&((u8 *)(pFile->pMapRegion))[انحراف], pBuf, amt);
      واپس SQLITE_OK;
    }ورنہ{
      صحیح nCopy = pFile->mmap_سائز - انحراف;
      حافظہ_نقل(&((u8 *)(pFile->pMapRegion))[انحراف], pBuf, nCopy);
      pBuf = &((u8 *)pBuf)[nCopy];
      amt -= nCopy;
      انحراف += nCopy;
    }
  }
#اگر_ختم

  جب( (wrote = seekAndWrite(pFile, انحراف, pBuf, amt))<amt && wrote>0 ){
    amt -= wrote;
    انحراف += wrote;
    pBuf = &((حرف*)pBuf)[wrote];
  }
  SimulateIOError(( wrote=(-1), amt=1 ));
  SimulateDiskfullError(( wrote=0, amt=1 ));

  اگر( amt>wrote ){
    اگر( wrote<0 && pFile->آخری_Errno!=ENOSPC ){
      /* lastErrno set by seekAndWrite */
      واپس SQLITE_IOERR_WRITE;
    }ورنہ{
      store_آخری_Errno(pFile, 0); /* not a system error */
      واپس SQLITE_بھرا;
    }
  }

  واپس SQLITE_OK;
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
** We do not trust systems to provide a working fdatasync().  Some do.
** Others do no.  To be safe, we will stick with the (slightly slower)
** fsync(). If you know that your system does support fdatasync() correctly,
** then simply compile with -Dfdatasync=fdatasync or -DHAVE_FDATASYNC
*/
#اگر !defined(fdatasync) && !HAVE_FDATASYNC
# define fdatasync fsync
#اگر_ختم

/*
** Define HAVE_FULLFSYNC to 0 or 1 depending on whether or not
** the F_FULLFSYNC macro is defined.  F_FULLFSYNC is currently
** only available on Mac OS X.  But that could change.
*/
#اگر_تعریف_شدہ F_FULLFSYNC
# define HAVE_FULLFSYNC 1
#ورنہ
# define HAVE_FULLFSYNC 0
#اگر_ختم


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
ساکن صحیح بھرا_fsync(صحیح fd, صحیح بھرا_Sync, صحیح ڈیٹا_Only){
  صحیح rc;

  /* The following "ifdef/elif/else/" block has the same structure as
  ** the one below. It is replicated here solely to avoid cluttering
  ** up the real code with the UNUSED_PARAMETER() macros.
  */
#اگر_تعریف_شدہ SQLITE_NO_SYNC
  غیر_مستعمل_PARAMETER(fd);
  غیر_مستعمل_PARAMETER(بھرا_Sync);
  غیر_مستعمل_PARAMETER(ڈیٹا_Only);
#وگرنہ_اگر HAVE_FULLFSYNC
  غیر_مستعمل_PARAMETER(ڈیٹا_Only);
#ورنہ
  غیر_مستعمل_PARAMETER(بھرا_Sync);
  غیر_مستعمل_PARAMETER(ڈیٹا_Only);
#اگر_ختم

  /* Record the number of times that we do a normal fsync() and
  ** FULLSYNC.  This is used during testing to verify that this procedure
  ** gets called with the correct arguments.
  */
#اگر_تعریف_شدہ SQLITE_TEST
  اگر( بھرا_Sync ) sqlite3_fullsync_گنتی++;
  sqlite3_sync_گنتی++;
#اگر_ختم

  /* If we compiled with the SQLITE_NO_SYNC flag, then syncing is a
  ** no-op.  But go ahead and call fstat() to validate the file
  ** descriptor as we need a method to provoke a failure during
  ** coverage testing.
  */
#اگر_تعریف_شدہ SQLITE_NO_SYNC
  {
    ساخت stat buf;
    rc = osFstat(fd, &buf);
  }
#وگرنہ_اگر HAVE_FULLFSYNC
  اگر( بھرا_Sync ){
    rc = osFcntl(fd, F_FULLFSYNC, 0);
  }ورنہ{
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
  اگر( rc ) rc = fsync(fd);

#وگرنہ_اگر defined(__APPLE__)
  /* fdatasync() on HFS+ doesn't yet flush the file size if it changed correctly
  ** so currently we default to the macro that redefines fdatasync to fsync
  */
  rc = fsync(fd);
#ورنہ
  rc = fdatasync(fd);
#اگر OS_VXWORKS
  اگر( rc==-1 && errno==ENOTSUP ){
    rc = fsync(fd);
  }
#اگر_ختم /* OS_VXWORKS */
#اگر_ختم /* ifdef SQLITE_NO_SYNC elif HAVE_FULLFSYNC */

  اگر( OS_VXWORKS && rc!= -1 ){
    rc = 0;
  }
  واپس rc;
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
ساکن صحیح openDirectory(ثابت حرف *zFilename, صحیح *pFd){
  صحیح ii;
  صحیح fd = -1;
  حرف zDirname[MAX_PATHNAME+1];

  sqlite3_snprintf(MAX_PATHNAME, zDirname, "%s", zFilename);
  برائے(ii=(صحیح)لڑی_لمبائی(zDirname); ii>0 && zDirname[ii]!='/'; ii--);
  اگر( ii>0 ){
    zDirname[ii] = '\0';
  }ورنہ{
    اگر( zDirname[0]!='/' ) zDirname[0] = '.';
    zDirname[1] = 0;
  }
  fd = robust_open(zDirname, O_RDONLY|O_ثنائی, 0);
  اگر( fd>=0 ){
    OSTRACE(("OPENDIR %-3d %s\n", fd, zDirname));
  }
  *pFd = fd;
  اگر( fd>=0 ) واپس SQLITE_OK;
  واپس unixLogError(SQLITE_CANTOPEN_BKPT, "openDirectory", zDirname);
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
ساکن صحیح unixSync(sqlite3_file *id, صحیح flags){
  صحیح rc;
  unixFile *pFile = (unixFile*)id;

  صحیح is_ڈیٹا_Only = (flags&SQLITE_SYNC_DATAONLY);
  صحیح isFullsync = (flags&0x0F)==SQLITE_SYNC_بھرا;

  /* Check that one of SQLITE_SYNC_NORMAL or FULL was passed */
  أكد((flags&0x0F)==SQLITE_SYNC_معمول
      || (flags&0x0F)==SQLITE_SYNC_بھرا
  );

  /* Unix cannot, but some systems may return SQLITE_FULL from here. This
  ** line is to test that doing so does not cause any problems.
  */
  SimulateDiskfullError( واپس SQLITE_بھرا );

  أكد( pFile );
  OSTRACE(("SYNC    %-3d\n", pFile->h));
  rc = بھرا_fsync(pFile->h, isFullsync, is_ڈیٹا_Only);
  SimulateIOError( rc=1 );
  اگر( rc ){
    store_آخری_Errno(pFile, errno);
    واپس unixLogError(SQLITE_IOERR_FSYNC, "full_fsync", pFile->z_راستہ);
  }

  /* Also fsync the directory containing the file if the DIRSYNC flag
  ** is set.  This is a one-time occurrence.  Many systems (examples: AIX)
  ** are unable to fsync a directory, so ignore errors on the fsync.
  */
  اگر( pFile->ctrlFlags & UNIXFILE_DIRSYNC ){
    صحیح dirfd;
    OSTRACE(("DIRSYNC %s (have_fullfsync=%d fullsync=%d)\n", pFile->z_راستہ,
            HAVE_FULLFSYNC, isFullsync));
    rc = osOpenDirectory(pFile->z_راستہ, &dirfd);
    اگر( rc==SQLITE_OK ){
      بھرا_fsync(dirfd, 0, 0);
      robust_close(pFile, dirfd, __LINE__);
    }ورنہ{
      أكد( rc==SQLITE_CANTOPEN );
      rc = SQLITE_OK;
    }
    pFile->ctrlFlags &= ~UNIXFILE_DIRSYNC;
  }
  واپس rc;
}

/*
** Truncate an open file to a specified size
*/
ساکن صحیح unixTruncate(sqlite3_file *id, i64 nByte){
  unixFile *pFile = (unixFile *)id;
  صحیح rc;
  أكد( pFile );
  SimulateIOError( واپس SQLITE_IOERR_TRUNCATE );

  /* If the user has configured a chunk-size for this file, truncate the
  ** file so that it consists of an integer number of chunks (i.e. the
  ** actual file size after the operation may be larger than the requested
  ** size).
  */
  اگر( pFile->szChunk>0 ){
    nByte = ((nByte + pFile->szChunk - 1)/pFile->szChunk) * pFile->szChunk;
  }

  rc = robust_ftruncate(pFile->h, nByte);
  اگر( rc ){
    store_آخری_Errno(pFile, errno);
    واپس unixLogError(SQLITE_IOERR_TRUNCATE, "ftruncate", pFile->z_راستہ);
  }ورنہ{
#اگر_تعریف_شدہ SQLITE_DEBUG
    /* If we are doing a normal write to a database file (as opposed to
    ** doing a hot-journal rollback or a write to some file other than a
    ** normal database file) and we truncate the file to zero length,
    ** that effectively updates the change counter.  This might happen
    ** when restoring a database using the backup API from a zero-length
    ** source.
    */
    اگر( pFile->اندر_معمول_Write && nByte==0 ){
      pFile->transCntrChng = 1;
    }
#اگر_ختم

#اگر SQLITE_MAX_MMAP_سائز>0
    /* If the file was just truncated to a size smaller than the currently
    ** mapped region, reduce the effective mapping size as well. SQLite will
    ** use read() and write() to access data beyond this point from now on.
    */
    اگر( nByte<pFile->mmap_سائز ){
      pFile->mmap_سائز = nByte;
    }
#اگر_ختم

    واپس SQLITE_OK;
  }
}

/*
** Determine the current size of a file in bytes
*/
ساکن صحیح unix_File_سائز(sqlite3_file *id, i64 *p_سائز){
  صحیح rc;
  ساخت stat buf;
  أكد( id );
  rc = osFstat(((unixFile*)id)->h, &buf);
  SimulateIOError( rc=1 );
  اگر( rc!=0 ){
    store_آخری_Errno((unixFile*)id, errno);
    واپس SQLITE_IOERR_FSTAT;
  }
  *p_سائز = buf.st_سائز;

  /* When opening a zero-size database, the findInodeInfo() procedure
  ** writes a single byte into that file in order to work around a bug
  ** in the OS-X msdos filesystem.  In order to avoid problems with upper
  ** layers, we need to report this file size as zero even though it is
  ** really 1.   Ticket #3260.
  */
  اگر( *p_سائز==1 ) *p_سائز = 0;


  واپس SQLITE_OK;
}

#اگر SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
/*
** Handler for proxy-locking file-control verbs.  Defined below in the
** proxying locking division.
*/
ساکن صحیح proxyFileControl(sqlite3_file*,صحیح,خالی*);
#اگر_ختم

/*
** This function is called to handle the SQLITE_FCNTL_SIZE_HINT
** file-control operation.  Enlarge the database to nBytes in size
** (rounded up to the next chunk-size).  If the database is already
** nBytes or larger, this routine is a no-op.
*/
ساکن صحیح fcntl_سائز_Hint(unixFile *pFile, i64 nByte){
  اگر( pFile->szChunk>0 ){
    i64 n_سائز;                    /* Required file size */
    ساخت stat buf;              /* Used to hold return values of fstat() */
 
    اگر( osFstat(pFile->h, &buf) ){
      واپس SQLITE_IOERR_FSTAT;
    }

    n_سائز = ((nByte+pFile->szChunk-1) / pFile->szChunk) * pFile->szChunk;
    اگر( n_سائز>(i64)buf.st_سائز ){

#اگر defined(HAVE_POSIX_FALLOCATE) && HAVE_POSIX_FALLOCATE
      /* The code below is handling the return value of osFallocate()
      ** correctly. posix_fallocate() is defined to "returns zero on success,
      ** or an error number on  failure". See the manpage for details. */
      صحیح err;
      کرو{
        err = osFallocate(pFile->h, buf.st_سائز, n_سائز-buf.st_سائز);
      }جب( err==EINTR );
      اگر( err && err!=EINVAL ) واپس SQLITE_IOERR_WRITE;
#ورنہ
      /* If the OS does not have posix_fallocate(), fake it. Write a
      ** single byte to the last byte in each block that falls entirely
      ** within the extended region. Then, if required, a single byte
      ** at offset (nSize-1), to set the size of the file correctly.
      ** This is a similar technique to that used by glibc on systems
      ** that do not have a real fallocate() call.
      */
      صحیح nBlk = buf.st_blksize;  /* File-system block size */
      صحیح nWrite = 0;             /* Number of bytes written by seekAndWrite */
      i64 iWrite;                 /* Next offset to write to */

      iWrite = (buf.st_سائز/nBlk)*nBlk + nBlk - 1;
      أكد( iWrite>=buf.st_سائز );
      أكد( ((iWrite+1)%nBlk)==0 );
      برائے(/*no-op*/; iWrite<n_سائز+nBlk-1; iWrite+=nBlk ){
        اگر( iWrite>=n_سائز ) iWrite = n_سائز - 1;
        nWrite = seekAndWrite(pFile, iWrite, "", 1);
        اگر( nWrite!=1 ) واپس SQLITE_IOERR_WRITE;
      }
#اگر_ختم
    }
  }

#اگر SQLITE_MAX_MMAP_سائز>0
  اگر( pFile->mmap_سائز_Max>0 && nByte>pFile->mmap_سائز ){
    صحیح rc;
    اگر( pFile->szChunk<=0 ){
      اگر( robust_ftruncate(pFile->h, nByte) ){
        store_آخری_Errno(pFile, errno);
        واپس unixLogError(SQLITE_IOERR_TRUNCATE, "ftruncate", pFile->z_راستہ);
      }
    }

    rc = unixMapfile(pFile, nByte);
    واپس rc;
  }
#اگر_ختم

  واپس SQLITE_OK;
}

/*
** If *pArg is initially negative then this is a query.  Set *pArg to
** 1 or 0 depending on whether or not bit mask of pFile->ctrlFlags is set.
**
** If *pArg is 0 or 1, then clear or set the mask bit of pFile->ctrlFlags.
*/
ساکن خالی unix_منوال_ثنائی_ہندسہ(unixFile *pFile, غیرعلامتی حرف نقاب, صحیح *pArg){
  اگر( *pArg<0 ){
    *pArg = (pFile->ctrlFlags & نقاب)!=0;
  }ورنہ اگر( (*pArg)==0 ){
    pFile->ctrlFlags &= ~نقاب;
  }ورنہ{
    pFile->ctrlFlags |= نقاب;
  }
}

/* Forward declaration */
ساکن صحیح unixGetTempname(صحیح nBuf, حرف *zBuf);
#اگر !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
 ساکن صحیح unixFcntlExternalReader(unixFile*, صحیح*);
#اگر_ختم
#اگر defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
 ساکن خالی unixDescribeShm(sqlite3_str*,unixShm*);
#اگر_ختم


/*
** Information and control of an open file handle.
*/
ساکن صحیح unixFileControl(sqlite3_file *id, صحیح op, خالی *pArg){
  unixFile *pFile = (unixFile*)id;
  انتخاب( op ){
#اگر defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
    حالت SQLITE_FCNTL_BEGIN_ATOMIC_WRITE: {
      صحیح rc = osIoctl(pFile->h, F2_FS_IOC_آغاز_ATOMIC_WRITE);
      واپس rc ? SQLITE_IOERR_BEGIN_ATOMIC : SQLITE_OK;
    }
    حالت SQLITE_FCNTL_COMMIT_ATOMIC_WRITE: {
      صحیح rc = osIoctl(pFile->h, F2FS_IOC_COMMIT_ATOMIC_WRITE);
      واپس rc ? SQLITE_IOERR_COMMIT_ATOMIC : SQLITE_OK;
    }
    حالت SQLITE_FCNTL_ROLLBACK_ATOMIC_WRITE: {
      صحیح rc = osIoctl(pFile->h, F2FS_IOC_ABORT_VOLATILE_WRITE);
      واپس rc ? SQLITE_IOERR_ROLLBACK_ATOMIC : SQLITE_OK;
    }
#اگر_ختم /* __linux__ && SQLITE_ENABLE_BATCH_ATOMIC_WRITE */

    حالت SQLITE_FCNTL_NULL_IO: {
      osClose(pFile->h);
      pFile->h = -1;
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_LOCKSTATE: {
      *(صحیح*)pArg = pFile->e_File_تالا;
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_آخری_ERRNO: {
      *(صحیح*)pArg = pFile->آخری_Errno;
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_CHUNK_سائز: {
      pFile->szChunk = *(صحیح *)pArg;
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_سائز_HINT: {
      صحیح rc;
      SimulateIOErrorBenign(1);
      rc = fcntl_سائز_Hint(pFile, *(i64 *)pArg);
      SimulateIOErrorBenign(0);
      واپس rc;
    }
    حالت SQLITE_FCNTL_PERSIST_WAL: {
      unix_منوال_ثنائی_ہندسہ(pFile, UNIXFILE_PERSIST_WAL, (صحیح*)pArg);
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_POWERSAFE_OVERWRITE: {
      unix_منوال_ثنائی_ہندسہ(pFile, UNIXFILE_PSOW, (صحیح*)pArg);
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_VFSNAME: {
      *(حرف**)pArg = sqlite3_mprintf("%s", pFile->pVfs->zName);
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_TEMPFILENAME: {
      حرف *zTFile = sqlite3_malloc64( pFile->pVfs->mxPathname );
      اگر( zTFile ){
        unixGetTempname(pFile->pVfs->mxPathname, zTFile);
        *(حرف**)pArg = zTFile;
      }
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_HAS_MOVED: {
      *(صحیح*)pArg = fileHasMoved(pFile);
      واپس SQLITE_OK;
    }
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
    حالت SQLITE_FCNTL_تالا_TIMEOUT: {
      صحیح iOld = pFile->iBusyTimeout;
      صحیح iNew = *(صحیح*)pArg;
#اگر SQLITE_ENABLE_SETLK_TIMEOUT==1
      pFile->iBusyTimeout = iNew<0 ? 0x7FFFFFFF : (غیرعلامتی)iNew;
#وگرنہ_اگر SQLITE_ENABLE_SETLK_TIMEOUT==2
      pFile->iBusyTimeout = !!(*(صحیح*)pArg);
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
#اگر SQLITE_MAX_MMAP_سائز>0
    حالت SQLITE_FCNTL_MMAP_سائز: {
      i64 new_حد_بندی = *(i64*)pArg;
      صحیح rc = SQLITE_OK;
      اگر( new_حد_بندی>sqlite3GlobalConfig.mxMmap ){
        new_حد_بندی = sqlite3GlobalConfig.mxMmap;
      }

      /* The value of newLimit may be eventually cast to (size_t) and passed
      ** to mmap(). Restrict its value to 2GB if (size_t) is not at least a
      ** 64-bit type. */
      اگر( new_حد_بندی>0 && حجم(size_t)<8 ){
        new_حد_بندی = (new_حد_بندی & 0x7FFFFFFF);
      }

      *(i64*)pArg = pFile->mmap_سائز_Max;
      اگر( new_حد_بندی>=0 && new_حد_بندی!=pFile->mmap_سائز_Max && pFile->n_Fetch_برآمد==0 ){
        pFile->mmap_سائز_Max = new_حد_بندی;
        اگر( pFile->mmap_سائز>0 ){
          unixUnmapfile(pFile);
          rc = unixMapfile(pFile, -1);
        }
      }
      واپس rc;
    }
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_DEBUG
    /* The pager calls this method to signal that it has done
    ** a rollback and that the database is therefore unchanged and
    ** it hence it is OK for the transaction change counter to be
    ** unchanged.
    */
    حالت SQLITE_FCNTL_DB_UNCHANGED: {
      ((unixFile*)id)->dbUpdate = 0;
      واپس SQLITE_OK;
    }
#اگر_ختم
#اگر SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    حالت SQLITE_FCNTL_سیٹ_LOCKPROXYFILE:
    حالت SQLITE_FCNTL_GET_LOCKPROXYFILE: {
      واپس proxyFileControl(id,op,pArg);
    }
#اگر_ختم /* SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__) */

    حالت SQLITE_FCNTL_EXTERNAL_READER: {
#اگر !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)
      واپس unixFcntlExternalReader((unixFile*)id, (صحیح*)pArg);
#ورنہ
      *(صحیح*)pArg = 0;
      واپس SQLITE_OK;
#اگر_ختم
    }

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
    حالت SQLITE_FCNTL_FILESTAT: {
      sqlite3_str *pStr = (sqlite3_str*)pArg;
      حرف aLck[16];
      unixInodeInfo *pInode;
      ساکن ثابت حرف *az_تالا[] = { "SHARED", "RESERVED",
                                      "PENDING", "EXCLUSIVE" };
      sqlite3_str_appendf(pStr, "{\"h\":%d", pFile->h);
      sqlite3_str_appendf(pStr, ",\"vfs\":\"%s\"", pFile->pVfs->zName);
      اگر( pFile->e_File_تالا ){
        sqlite3_str_appendf(pStr, ",\"eFileLock\":\"%s\"", 
                                  az_تالا[pFile->e_File_تالا-1]);
        اگر( unixPosixAdvisoryLocks(pFile->h, aLck)==SQLITE_OK ){
          sqlite3_str_appendf(pStr, ",\"pal\":\"%s\"", aLck);
        }
      }
      unixEnterMutex();
      اگر( pFile->pShm ){
        sqlite3_str_appendall(pStr, ",\"shm\":");
        unixDescribeShm(pStr, pFile->pShm);
      }
#اگر SQLITE_MAX_MMAP_سائز>0
      اگر( pFile->mmap_سائز ){
        sqlite3_str_appendf(pStr, ",\"mmapSize\":%lld", pFile->mmap_سائز);
        sqlite3_str_appendf(pStr, ",\"nFetchOut\":%d", pFile->n_Fetch_برآمد);
      }
#اگر_ختم
      اگر( (pInode = pFile->pInode)!=0 ){
        sqlite3_str_appendf(pStr, ",\"inode\":{\"nRef\":%d",pInode->nRef);
        sqlite3_mutex_enter(pInode->p_تالا_Mutex);
        sqlite3_str_appendf(pStr, ",\"nShared\":%d", pInode->nShared);
        اگر( pInode->e_File_تالا ){
          sqlite3_str_appendf(pStr, ",\"eFileLock\":\"%s\"", 
                                  az_تالا[pInode->e_File_تالا-1]);
        }
        اگر( pInode->p_غیر_مستعمل ){
          حرف cSep = '[';
          Unix_غیر_مستعمل_Fd *pUFd = pFile->pInode->p_غیر_مستعمل;
          sqlite3_str_appendall(pStr, ",\"unusedFd\":");
          جب( pUFd ){
            sqlite3_str_appendf(pStr, "%c{\"fd\":%d,\"flags\":%d",
                                cSep, pUFd->fd, pUFd->flags);
            cSep = ',';
            اگر( unixPosixAdvisoryLocks(pUFd->fd, aLck)==SQLITE_OK ){
              sqlite3_str_appendf(pStr, ",\"pal\":\"%s\"", aLck);
            }
            sqlite3_str_append(pStr, "}", 1);
            pUFd = pUFd->p_اگلا;
          }
          sqlite3_str_append(pStr, "]", 1);
        }
        sqlite3_mutex_leave(pInode->p_تالا_Mutex);
        sqlite3_str_append(pStr, "}", 1);
      }
      unixLeaveMutex();
      sqlite3_str_append(pStr, "}", 1);
      واپس SQLITE_OK;
    }
#اگر_ختم /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */
  }
  واپس SQLITE_NOTFOUND;
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
#اگر_غیر_تعریف_شدہ __QNXNTO__
ساکن خالی سیٹ_Device_Characteristics(unixFile *pFd){
  أكد( pFd->deviceCharacteristics==0 || pFd->sector_سائز!=0 );
  اگر( pFd->sector_سائز==0 ){
#اگر defined(__linux__) && defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
    صحیح res;
    u32 f = 0;

    /* Check for support for F2FS atomic batch writes. */
    res = osIoctl(pFd->h, F2FS_IOC_GET_FEATURES, &f);
    اگر( res==0 && (f & F2FS_FEATURE_ATOMIC_WRITE) ){
      pFd->deviceCharacteristics = SQLITE_IOCAP_BATCH_ATOMIC;
    }
#اگر_ختم /* __linux__ && SQLITE_ENABLE_BATCH_ATOMIC_WRITE */

    /* Set the POWERSAFE_OVERWRITE flag if requested. */
    اگر( pFd->ctrlFlags & UNIXFILE_PSOW ){
      pFd->deviceCharacteristics |= SQLITE_IOCAP_POWERSAFE_OVERWRITE;
    }
    pFd->deviceCharacteristics |= SQLITE_IOCAP_SUBPAGE_READ;

    pFd->sector_سائز = SQLITE_DEFAULT_SECTOR_سائز;
  }
}
#ورنہ
#شامل <sys/dcmd_blk.h>
#شامل <sys/statvfs.h>
ساکن خالی سیٹ_Device_Characteristics(unixFile *pFile){
  اگر( pFile->sector_سائز == 0 ){
    ساخت statvfs fsInfo;

    /* Set defaults for non-supported filesystems */
    pFile->sector_سائز = SQLITE_DEFAULT_SECTOR_سائز;
    pFile->deviceCharacteristics = 0;
    اگر( fstatvfs(pFile->h, &fsInfo) == -1 ) {
      واپس;
    }

    اگر( !strcmp(fsInfo.f_basetype, "tmp") ) {
      pFile->sector_سائز = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC4K |       /* All ram filesystem writes are atomic */
        SQLITE_IOCAP_محفوظ_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }ورنہ اگر( لڑی_تلاش(fsInfo.f_basetype, "etfs") ){
      pFile->sector_سائز = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* etfs cluster size writes are atomic */
        (pFile->sector_سائز / 512 * SQLITE_IOCAP_ATOMIC512) |
        SQLITE_IOCAP_محفوظ_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }ورنہ اگر( !strcmp(fsInfo.f_basetype, "qnx6") ){
      pFile->sector_سائز = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC |         /* All filesystem writes are atomic */
        SQLITE_IOCAP_محفوظ_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }ورنہ اگر( !strcmp(fsInfo.f_basetype, "qnx4") ){
      pFile->sector_سائز = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* full bitset of atomics from max sector size and smaller */
        (((pFile->sector_سائز / 512 * SQLITE_IOCAP_ATOMIC512) << 1) - 2) |
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }ورنہ اگر( لڑی_تلاش(fsInfo.f_basetype, "dos") ){
      pFile->sector_سائز = fsInfo.f_bsize;
      pFile->deviceCharacteristics =
        /* full bitset of atomics from max sector size and smaller */
        (((pFile->sector_سائز / 512 * SQLITE_IOCAP_ATOMIC512) << 1) - 2) |
        SQLITE_IOCAP_SEQUENTIAL |     /* The ram filesystem has no write behind
                                      ** so it is ordered */
        0;
    }ورنہ{
      pFile->deviceCharacteristics =
        SQLITE_IOCAP_ATOMIC512 |      /* blocks are atomic */
        SQLITE_IOCAP_محفوظ_APPEND |    /* growing the file does not occur until
                                      ** the write succeeds */
        0;
    }
  }
  /* Last chance verification.  If the sector size isn't a multiple of 512
  ** then it isn't valid.*/
  اگر( pFile->sector_سائز % 512 != 0 ){
    pFile->deviceCharacteristics = 0;
    pFile->sector_سائز = SQLITE_DEFAULT_SECTOR_سائز;
  }
}
#اگر_ختم

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
ساکن صحیح unix_Sector_سائز(sqlite3_file *id){
  unixFile *pFd = (unixFile*)id;
  سیٹ_Device_Characteristics(pFd);
  واپس pFd->sector_سائز;
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
ساکن صحیح unixDeviceCharacteristics(sqlite3_file *id){
  unixFile *pFd = (unixFile*)id;
  سیٹ_Device_Characteristics(pFd);
  واپس pFd->deviceCharacteristics;
}

#اگر !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_سائز>0

/*
** Return the system page size.
**
** This function should not be called directly by other code in this file.
** Instead, it should be called via macro osGetpagesize().
*/
ساکن صحیح unixGetpagesize(خالی){
#اگر OS_VXWORKS
  واپس 1024;
#وگرنہ_اگر defined(BSD_ماخذ)
  واپس getpagesize();
#ورنہ
  واپس (صحیح)sysconf(_SC_PAGESIZE);
#اگر_ختم
}

#اگر_ختم /* !defined(SQLITE_OMIT_WAL) || SQLITE_MAX_MMAP_SIZE>0 */

#اگر !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL)

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
ساخت unix_Shm_گرہ {
  unixInodeInfo *pInode;     /* unixInodeInfo that owns this SHM node */
  sqlite3_mutex *pShmMutex;  /* Mutex to access this object */
  حرف *zFilename;           /* Name of the mmapped file */
  صحیح hShm;                  /* Open file descriptor */
  صحیح szRegion;              /* Size of shared-memory regions */
  u16 nRegion;               /* Size of array apRegion */
  u8 isReadonly;             /* True if read-only */
  u8 isUnlocked;             /* True if no DMS lock held */
  حرف **apRegion;           /* Array of mapped shared-memory regions */
  صحیح nRef;                  /* Number of unixShm objects pointing to this */
  unixShm *p_پہلا;           /* All unixShm objects pointing to this */
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
  sqlite3_mutex *aMutex[SQLITE_SHM_NLOCK];
#اگر_ختم
  صحیح a_تالا[SQLITE_SHM_NLOCK];  /* # shared locks on slot, -1==excl lock */
#اگر_تعریف_شدہ SQLITE_DEBUG
  u8 اگلا_Shm_Id;              /* Next available unixShm.id value */
#اگر_ختم
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
ساخت unixShm {
  unix_Shm_گرہ *p_Shm_گرہ;     /* The underlying unixShmNode object */
  unixShm *p_اگلا;            /* Next unixShm with the same unixShmNode */
  u8 hasMutex;               /* True if holding the unixShmNode->pShmMutex */
  u8 id;                     /* Id of this connection within its unixShmNode */
  u16 shared_نقاب;            /* Mask of shared locks held */
  u16 excl_نقاب;              /* Mask of exclusive locks held */
};

/*
** Constants used for locking
*/
#تعریف_کرو UNIX_SHM_بنیاد   ((22+SQLITE_SHM_NLOCK)*4)         /* first lock byte */
#تعریف_کرو UNIX_SHM_DMS    (UNIX_SHM_بنیاد+SQLITE_SHM_NLOCK)  /* deadman switch */

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_FILESTAT)
/*
** Describe the pShm object using JSON.  Used for diagnostics only.
*/
ساکن خالی unixDescribeShm(sqlite3_str *pStr, unixShm *pShm){
  unix_Shm_گرہ *p_گرہ = pShm->p_Shm_گرہ;
  حرف aLck[16];
  sqlite3_str_appendf(pStr, "{\"h\":%d", p_گرہ->hShm);
  أكد( unixMutexHeld() );
  sqlite3_str_appendf(pStr, ",\"nRef\":%d", p_گرہ->nRef);
  sqlite3_str_appendf(pStr, ",\"id\":%d", pShm->id);
  sqlite3_str_appendf(pStr, ",\"sharedMask\":%d", pShm->shared_نقاب);
  sqlite3_str_appendf(pStr, ",\"exclMask\":%d", pShm->excl_نقاب);
  اگر( unixPosixAdvisoryLocks(p_گرہ->hShm, aLck)==SQLITE_OK ){
    sqlite3_str_appendf(pStr, ",\"pal\":\"%s\"", aLck);
  }
  sqlite3_str_append(pStr, "}", 1);
}
#اگر_ختم /* SQLITE_DEBUG || SQLITE_ENABLE_FILESTAT */

/*
** Use F_GETLK to check whether or not there are any readers with open
** wal-mode transactions in other processes on database file pFile. If
** no error occurs, return SQLITE_OK and set (*piOut) to 1 if there are
** such transactions, or 0 otherwise. If an error occurs, return an
** SQLite error code. The final value of *piOut is undefined in this
** case.
*/
ساکن صحیح unixFcntlExternalReader(unixFile *pFile, صحیح *pi_برآمد){
  صحیح rc = SQLITE_OK;
  *pi_برآمد = 0;
  اگر( pFile->pShm){
    unix_Shm_گرہ *p_Shm_گرہ = pFile->pShm->p_Shm_گرہ;
    ساخت flock f;

    memset(&f, 0, حجم(f));
    f.l_type = F_WRLCK;
    f.l_whence = SEEK_SET;
    f.l_آغاز = UNIX_SHM_بنیاد + 3;
    f.l_len = SQLITE_SHM_NLOCK - 3;

    sqlite3_mutex_enter(p_Shm_گرہ->pShmMutex);
    اگر( osFcntl(p_Shm_گرہ->hShm, F_GETLK, &f)<0 ){
      rc = SQLITE_IOERR_تالا;
    }ورنہ{
      *pi_برآمد = (f.l_type!=F_UNLCK);
    }
    sqlite3_mutex_leave(p_Shm_گرہ->pShmMutex);
  }

  واپس rc;
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
ساکن صحیح unix_Is_Sharing_Shm_گرہ(unixFile *pFile){
  unix_Shm_گرہ *p_Shm_گرہ;
  ساخت flock تالا;
  اگر( pFile->pShm==0 ) واپس 0;
  اگر( pFile->ctrlFlags & UNIXFILE_EXCL ) واپس 0;
  p_Shm_گرہ = pFile->pShm->p_Shm_گرہ;
#اگر SQLITE_ATOMIC_INTRINSICS
  أكد( Atomic_بھرو(&p_Shm_گرہ->nRef)==1 );
#اگر_ختم
  memset(&تالا, 0, حجم(تالا));
  تالا.l_whence = SEEK_SET;
  تالا.l_آغاز = UNIX_SHM_DMS;
  تالا.l_len = 1;
  تالا.l_type = F_WRLCK;
  osFcntl(p_Shm_گرہ->hShm, F_GETLK, &تالا);
  واپس (تالا.l_type!=F_UNLCK);
}

/*
** Apply posix advisory locks for all bytes from ofst through ofst+n-1.
**
** Locks block if the mask is exactly UNIX_SHM_C and are non-blocking
** otherwise.
*/
ساکن صحیح unix_Shm_System_تالا(
  unixFile *pFile,       /* Open connection to the WAL file */
  صحیح تالا_Type,          /* F_UNLCK, F_RDLCK, or F_WRLCK */
  صحیح ofst,              /* First byte of the locking range */
  صحیح n                  /* Number of bytes to lock */
){
  unix_Shm_گرہ *p_Shm_گرہ; /* Apply locks to this open shared-memory segment */
  ساخت flock f;        /* The posix advisory locking structure */
  صحیح rc = SQLITE_OK;    /* Result code form fcntl() */

  p_Shm_گرہ = pFile->pInode->p_Shm_گرہ;

  /* Assert that the parameters are within expected range and that the
  ** correct mutex or mutexes are held. */
  أكد( p_Shm_گرہ->nRef>=0 );
  أكد( (ofst==UNIX_SHM_DMS && n==1)
       || (ofst>=UNIX_SHM_بنیاد && ofst+n<=(UNIX_SHM_بنیاد+SQLITE_SHM_NLOCK))
  );
  اگر( ofst==UNIX_SHM_DMS ){
    أكد( p_Shm_گرہ->nRef>0 || unixMutexHeld() );
    أكد( p_Shm_گرہ->nRef==0 || sqlite3_mutex_held(p_Shm_گرہ->pShmMutex) );
  }ورنہ{
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
    صحیح ii;
    برائے(ii=ofst-UNIX_SHM_بنیاد; ii<ofst-UNIX_SHM_بنیاد+n; ii++){
      أكد( sqlite3_mutex_held(p_Shm_گرہ->aMutex[ii]) );
    }
#ورنہ
    أكد( sqlite3_mutex_held(p_Shm_گرہ->pShmMutex) );
    أكد( p_Shm_گرہ->nRef>0 );
#اگر_ختم
  }

  /* Shared locks never span more than one byte */
  أكد( n==1 || تالا_Type!=F_RDLCK );

  /* Locks are within range */
  أكد( n>=1 && n<=SQLITE_SHM_NLOCK );
  أكد( ofst>=UNIX_SHM_بنیاد && ofst<=UNIX_SHM_DMS );
  أكد( ofst+n-1<=UNIX_SHM_DMS );

  اگر( p_Shm_گرہ->hShm>=0 ){
    صحیح res;
    /* Initialize the locking parameters */
    f.l_type = تالا_Type;
    f.l_whence = SEEK_SET;
    f.l_آغاز = ofst;
    f.l_len = n;
    res = os_سیٹ_Posix_Advisory_تالا(p_Shm_گرہ->hShm, &f, pFile);
    اگر( res==-1 ){
#اگر defined(SQLITE_ENABLE_SETLK_TIMEOUT) && SQLITE_ENABLE_SETLK_TIMEOUT==1
      rc = (pFile->iBusyTimeout ? SQLITE_BUSY_TIMEOUT : SQLITE_BUSY);
#ورنہ
      rc = SQLITE_BUSY;
#اگر_ختم
    }
  }

  /* Do debug tracing */
#اگر_تعریف_شدہ SQLITE_DEBUG
  OSTRACE(("SHM-LOCK "));
  اگر( rc==SQLITE_OK ){
    اگر( تالا_Type==F_UNLCK ){
      OSTRACE(("unlock %d..%d ok\n", ofst, ofst+n-1));
    }ورنہ اگر( تالا_Type==F_RDLCK ){
      OSTRACE(("read-lock %d..%d ok\n", ofst, ofst+n-1));
    }ورنہ{
      أكد( تالا_Type==F_WRLCK );
      OSTRACE(("write-lock %d..%d ok\n", ofst, ofst+n-1));
    }
  }ورنہ{
    اگر( تالا_Type==F_UNLCK ){
      OSTRACE(("unlock %d..%d failed\n", ofst, ofst+n-1));
    }ورنہ اگر( تالا_Type==F_RDLCK ){
      OSTRACE(("read-lock %d..%d failed\n", ofst, ofst+n-1));
    }ورنہ{
      أكد( تالا_Type==F_WRLCK );
      OSTRACE(("write-lock %d..%d failed\n", ofst, ofst+n-1));
    }
  }
#اگر_ختم

  واپس rc;
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
ساکن صحیح unix_Shm_Region_فی_Map(خالی){
  صحیح shmsz = 32*1024;            /* SHM region size */
  صحیح pgsz = osGetpagesize();   /* System page size */
  أكد( ((pgsz-1)&pgsz)==0 );   /* Page size must be a power of 2 */
  اگر( pgsz<shmsz ) واپس 1;
  واپس pgsz/shmsz;
}

/*
** Purge the unixShmNodeList list of all entries with unixShmNode.nRef==0.
**
** This is not a VFS shared-memory method; it is a utility function called
** by VFS shared-memory methods.
*/
ساکن خالی unixShmPurge(unixFile *pFd){
  unix_Shm_گرہ *p = pFd->pInode->p_Shm_گرہ;
  أكد( unixMutexHeld() );
  اگر( p && ALWAYS(p->nRef==0) ){
    صحیح n_Shm_فی_Map = unix_Shm_Region_فی_Map();
    صحیح i;
    أكد( p->pInode==pFd->pInode );
    sqlite3_mutex_free(p->pShmMutex);
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
    برائے(i=0; i<SQLITE_SHM_NLOCK; i++){
      sqlite3_mutex_free(p->aMutex[i]);
    }
#اگر_ختم
    برائے(i=0; i<p->nRegion; i+=n_Shm_فی_Map){
      اگر( p->hShm>=0 ){
        osMunmap(p->apRegion[i], p->szRegion);
      }ورنہ{
        sqlite3_free(p->apRegion[i]);
      }
    }
    sqlite3_free(p->apRegion);
    اگر( p->hShm>=0 ){
      robust_close(pFd, p->hShm, __LINE__);
      p->hShm = -1;
    }
    p->pInode->p_Shm_گرہ = 0;
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
ساکن صحیح unix_تالا_Shared_حافظہ(unixFile *pDbFd, unix_Shm_گرہ *p_Shm_گرہ){
  ساخت flock تالا;
  صحیح rc = SQLITE_OK;

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
  تالا.l_whence = SEEK_SET;
  تالا.l_آغاز = UNIX_SHM_DMS;
  تالا.l_len = 1;
  تالا.l_type = F_WRLCK;
  اگر( osFcntl(p_Shm_گرہ->hShm, F_GETLK, &تالا)!=0 ) {
    rc = SQLITE_IOERR_تالا;
  }ورنہ اگر( تالا.l_type==F_UNLCK ){
    اگر( p_Shm_گرہ->isReadonly ){
      p_Shm_گرہ->isUnlocked = 1;
      rc = SQLITE_READONLY_CANTINIT;
    }ورنہ{
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
      /* Do not use a blocking lock here. If the lock cannot be obtained
      ** immediately, it means some other connection is truncating the
      ** *-shm file. And after it has done so, it will not release its
      ** lock, but only downgrade it to a shared lock. So no point in
      ** blocking here. The call below to obtain the shared DMS lock may 
      ** use a blocking lock. */
      صحیح iSaveTimeout = pDbFd->iBusyTimeout;
      pDbFd->iBusyTimeout = 0;
#اگر_ختم
      rc = unix_Shm_System_تالا(pDbFd, F_WRLCK, UNIX_SHM_DMS, 1);
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
      pDbFd->iBusyTimeout = iSaveTimeout;
#اگر_ختم
      /* The first connection to attach must truncate the -shm file.  We
      ** truncate to 3 bytes (an arbitrary small number, less than the
      ** -shm header size) rather than 0 as a system debugging aid, to
      ** help detect if a -shm file truncation is legitimate or is the work
      ** or a rogue process. */
      اگر( rc==SQLITE_OK && robust_ftruncate(p_Shm_گرہ->hShm, 3) ){
        rc = unixLogError(SQLITE_IOERR_SHMOPEN,"ftruncate",p_Shm_گرہ->zFilename);
      }
    }
  }ورنہ اگر( تالا.l_type==F_WRLCK ){
    rc = SQLITE_BUSY;
  }

  اگر( rc==SQLITE_OK ){
    أكد( تالا.l_type==F_UNLCK || تالا.l_type==F_RDLCK );
    rc = unix_Shm_System_تالا(pDbFd, F_RDLCK, UNIX_SHM_DMS, 1);
  }
  واپس rc;
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
ساکن صحیح unix_Open_Shared_حافظہ(unixFile *pDbFd){
  ساخت unixShm *p = 0;          /* The connection to be opened */
  ساخت unix_Shm_گرہ *p_Shm_گرہ;   /* The underlying mmapped file */
  صحیح rc = SQLITE_OK;             /* Result code */
  unixInodeInfo *pInode;          /* The inode of fd */
  حرف *zShm;             /* Name of the file used for SHM */
  صحیح nShmFilename;               /* Size of the SHM filename in bytes */

  /* Allocate space for the new unixShm object. */
  p = sqlite3_malloc64( حجم(*p) );
  اگر( p==0 ) واپس SQLITE_NOMEM_BKPT;
  memset(p, 0, حجم(*p));
  أكد( pDbFd->pShm==0 );

  /* Check to see if a unixShmNode object already exists. Reuse an existing
  ** one if present. Create a new one if necessary.
  */
  أكد( unixFileMutexNotheld(pDbFd) );
  unixEnterMutex();
  pInode = pDbFd->pInode;
  p_Shm_گرہ = pInode->p_Shm_گرہ;
  اگر( p_Shm_گرہ==0 ){
    ساخت stat sStat;                 /* fstat() info for database file */
#اگر_غیر_تعریف_شدہ SQLITE_SHM_DIRECTORY
    ثابت حرف *z_بنیاد_راستہ = pDbFd->z_راستہ;
#اگر_ختم

    /* Call fstat() to figure out the permissions on the database file. If
    ** a new *-shm file is created, an attempt will be made to create it
    ** with the same permissions.
    */
    اگر( osFstat(pDbFd->h, &sStat) ){
      rc = SQLITE_IOERR_FSTAT;
      جاؤ shm_open_err;
    }

#اگر_تعریف_شدہ SQLITE_SHM_DIRECTORY
    nShmFilename = حجم(SQLITE_SHM_DIRECTORY) + 31;
#ورنہ
    nShmFilename = 6 + (صحیح)لڑی_لمبائی(z_بنیاد_راستہ);
#اگر_ختم
    p_Shm_گرہ = sqlite3_malloc64( حجم(*p_Shm_گرہ) + nShmFilename );
    اگر( p_Shm_گرہ==0 ){
      rc = SQLITE_NOMEM_BKPT;
      جاؤ shm_open_err;
    }
    memset(p_Shm_گرہ, 0, حجم(*p_Shm_گرہ)+nShmFilename);
    zShm = p_Shm_گرہ->zFilename = (حرف*)&p_Shm_گرہ[1];
#اگر_تعریف_شدہ SQLITE_SHM_DIRECTORY
    sqlite3_snprintf(nShmFilename, zShm,
                     SQLITE_SHM_DIRECTORY "/sqlite-shm-%x-%x",
                     (u32)sStat.st_ino, (u32)sStat.st_dev);
#ورنہ
    sqlite3_snprintf(nShmFilename, zShm, "%s-shm", z_بنیاد_راستہ);
    sqlite3FileSuffix3(pDbFd->z_راستہ, zShm);
#اگر_ختم
    p_Shm_گرہ->hShm = -1;
    pDbFd->pInode->p_Shm_گرہ = p_Shm_گرہ;
    p_Shm_گرہ->pInode = pDbFd->pInode;
    اگر( sqlite3GlobalConfig.bCoreMutex ){
      p_Shm_گرہ->pShmMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      اگر( p_Shm_گرہ->pShmMutex==0 ){
        rc = SQLITE_NOMEM_BKPT;
        جاؤ shm_open_err;
      }
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
      {
        صحیح ii;
        برائے(ii=0; ii<SQLITE_SHM_NLOCK; ii++){
          p_Shm_گرہ->aMutex[ii] = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
          اگر( p_Shm_گرہ->aMutex[ii]==0 ){
            rc = SQLITE_NOMEM_BKPT;
            جاؤ shm_open_err;
          }
        }
      }
#اگر_ختم
    }

    اگر( pInode->b_Process_تالا==0 ){
      اگر( 0==sqlite3_uri_boolean(pDbFd->z_راستہ, "readonly_shm", 0) ){
        p_Shm_گرہ->hShm = robust_open(zShm, O_RDWR|O_CREAT|O_NOFOLLOW,
                                     (sStat.st_منوال&0777));
      }
      اگر( p_Shm_گرہ->hShm<0 ){
        p_Shm_گرہ->hShm = robust_open(zShm, O_RDONLY|O_NOFOLLOW,
                                     (sStat.st_منوال&0777));
        اگر( p_Shm_گرہ->hShm<0 ){
          rc = unixLogError(SQLITE_CANTOPEN_BKPT, "open", zShm);
          جاؤ shm_open_err;
        }
        p_Shm_گرہ->isReadonly = 1;
      }

      /* If this process is running as root, make sure that the SHM file
      ** is owned by the same user that owns the original database.  Otherwise,
      ** the original owner will not be able to connect.
      */
      robustFchown(p_Shm_گرہ->hShm, sStat.st_uid, sStat.st_gid);

      rc = unix_تالا_Shared_حافظہ(pDbFd, p_Shm_گرہ);
      اگر( rc!=SQLITE_OK && rc!=SQLITE_READONLY_CANTINIT ) جاؤ shm_open_err;
    }
  }

  /* Make the new connection a child of the unixShmNode */
  p->p_Shm_گرہ = p_Shm_گرہ;
#اگر_تعریف_شدہ SQLITE_DEBUG
  p->id = p_Shm_گرہ->اگلا_Shm_Id++;
#اگر_ختم
  p_Shm_گرہ->nRef++;
  pDbFd->pShm = p;
  unixLeaveMutex();

  /* The reference count on pShmNode has already been incremented under
  ** the cover of the unixEnterMutex() mutex and the pointer from the
  ** new (struct unixShm) object to the pShmNode has been set. All that is
  ** left to do is to link the new object into the linked list starting
  ** at pShmNode->pFirst. This must be done while holding the
  ** pShmNode->pShmMutex.
  */
  sqlite3_mutex_enter(p_Shm_گرہ->pShmMutex);
  p->p_اگلا = p_Shm_گرہ->p_پہلا;
  p_Shm_گرہ->p_پہلا = p;
  sqlite3_mutex_leave(p_Shm_گرہ->pShmMutex);
  واپس rc;

  /* Jump here on any error */
shm_open_err:
  unixShmPurge(pDbFd);       /* This call frees pShmNode if required */
  sqlite3_free(p);
  unixLeaveMutex();
  واپس rc;
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
ساکن صحیح unixShmMap(
  sqlite3_file *fd,               /* Handle open on database file */
  صحیح iRegion,                    /* Region to retrieve */
  صحیح szRegion,                   /* Size of regions */
  صحیح bExtend,                    /* True to extend file if necessary */
  خالی متلون **pp              /* OUT: Mapped memory */
){
  unixFile *pDbFd = (unixFile*)fd;
  unixShm *p;
  unix_Shm_گرہ *p_Shm_گرہ;
  صحیح rc = SQLITE_OK;
  صحیح n_Shm_فی_Map = unix_Shm_Region_فی_Map();
  صحیح nReqRegion;

  /* If the shared-memory file has not yet been opened, open it now. */
  اگر( pDbFd->pShm==0 ){
    rc = unix_Open_Shared_حافظہ(pDbFd);
    اگر( rc!=SQLITE_OK ) واپس rc;
  }

  p = pDbFd->pShm;
  p_Shm_گرہ = p->p_Shm_گرہ;
  sqlite3_mutex_enter(p_Shm_گرہ->pShmMutex);
  اگر( p_Shm_گرہ->isUnlocked ){
    rc = unix_تالا_Shared_حافظہ(pDbFd, p_Shm_گرہ);
    اگر( rc!=SQLITE_OK ) جاؤ shmpage_برآمد;
    p_Shm_گرہ->isUnlocked = 0;
  }
  أكد( szRegion==p_Shm_گرہ->szRegion || p_Shm_گرہ->nRegion==0 );
  أكد( p_Shm_گرہ->pInode==pDbFd->pInode );
  أكد( p_Shm_گرہ->hShm>=0 || pDbFd->pInode->b_Process_تالا==1 );
  أكد( p_Shm_گرہ->hShm<0 || pDbFd->pInode->b_Process_تالا==0 );

  /* Minimum number of regions required to be mapped. */
  nReqRegion = ((iRegion+n_Shm_فی_Map) / n_Shm_فی_Map) * n_Shm_فی_Map;

  اگر( p_Shm_گرہ->nRegion<nReqRegion ){
    حرف **apNew;                         /* New apRegion[] array */
    i64 nByte = nReqRegion*(i64)szRegion; /* Minimum required file size */
    ساخت stat sStat;                    /* Used by fstat() */

    p_Shm_گرہ->szRegion = szRegion;

    اگر( p_Shm_گرہ->hShm>=0 ){
      /* The requested region is not mapped into this processes address space.
      ** Check to see if it has been allocated (i.e. if the wal-index file is
      ** large enough to contain the requested region).
      */
      اگر( osFstat(p_Shm_گرہ->hShm, &sStat) ){
        rc = SQLITE_IOERR_SHMSIZE;
        جاؤ shmpage_برآمد;
      }

      اگر( sStat.st_سائز<nByte ){
        /* The requested memory region does not exist. If bExtend is set to
        ** false, exit early. *pp will be set to NULL and SQLITE_OK returned.
        */
        اگر( !bExtend ){
          جاؤ shmpage_برآمد;
        }

        /* Alternatively, if bExtend is true, extend the file. Do this by
        ** writing a single byte to the end of each (OS) page being
        ** allocated or extended. Technically, we need only write to the
        ** last page in order to extend the file. But writing to all new
        ** pages forces the OS to allocate them immediately, which reduces
        ** the chances of SIGBUS while accessing the mapped region later on.
        */
        ورنہ{
          ساکن ثابت صحیح pgsz = 4096;
          i64 iPg;

          /* Write to the last byte of each newly allocated or extended page */
          أكد( (nByte % pgsz)==0 );
          برائے(iPg=(sStat.st_سائز/pgsz); iPg<(nByte/pgsz); iPg++){
            صحیح x = 0;
            اگر( seekAndWriteFd(p_Shm_گرہ->hShm, iPg*pgsz + pgsz-1,"",1,&x)!=1 ){
              ثابت حرف *zFile = p_Shm_گرہ->zFilename;
              rc = unixLogError(SQLITE_IOERR_SHMSIZE, "write", zFile);
              جاؤ shmpage_برآمد;
            }
          }
        }
      }
    }

    /* Map the requested memory region into this processes address space. */
    apNew = (حرف **)sqlite3_realloc64(
        p_Shm_گرہ->apRegion, nReqRegion*حجم(حرف *)
    );
    اگر( !apNew ){
      rc = SQLITE_IOERR_NOMEM_BKPT;
      جاؤ shmpage_برآمد;
    }
    p_Shm_گرہ->apRegion = apNew;
    جب( p_Shm_گرہ->nRegion<nReqRegion ){
      i64 nMap = (i64)szRegion*(i64)n_Shm_فی_Map;
      i64 i;
      خالی *pMem;
      اگر( p_Shm_گرہ->hShm>=0 ){
        pMem = osMmap(0, nMap,
            p_Shm_گرہ->isReadonly ? PROT_READ : PROT_READ|PROT_WRITE,
            MAP_SHARED, p_Shm_گرہ->hShm, szRegion*(i64)p_Shm_گرہ->nRegion
        );
        اگر( pMem==MAP_FAILED ){
          rc = unixLogError(SQLITE_IOERR_SHMMAP, "mmap", p_Shm_گرہ->zFilename);
          جاؤ shmpage_برآمد;
        }
      }ورنہ{
        pMem = sqlite3_malloc64(nMap);
        اگر( pMem==0 ){
          rc = SQLITE_NOMEM_BKPT;
          جاؤ shmpage_برآمد;
        }
        memset(pMem, 0, nMap);
      }

      برائے(i=0; i<n_Shm_فی_Map; i++){
        p_Shm_گرہ->apRegion[p_Shm_گرہ->nRegion+i] = &((حرف*)pMem)[szRegion*i];
      }
      p_Shm_گرہ->nRegion += n_Shm_فی_Map;
    }
  }

shmpage_برآمد:
  اگر( p_Shm_گرہ->nRegion>iRegion ){
    *pp = p_Shm_گرہ->apRegion[iRegion];
  }ورنہ{
    *pp = 0;
  }
  اگر( p_Shm_گرہ->isReadonly && rc==SQLITE_OK ) rc = SQLITE_READONLY;
  sqlite3_mutex_leave(p_Shm_گرہ->pShmMutex);
  واپس rc;
}

/*
** Check that the pShmNode->aLock[] array comports with the locking bitmasks
** held by each client. Return true if it does, or false otherwise. This
** is to be used in an assert(). e.g.
**
**     assert( assertLockingArrayOk(pShmNode) );
*/
#اگر_تعریف_شدہ SQLITE_DEBUG
ساکن صحیح assertLockingArrayOk(unix_Shm_گرہ *p_Shm_گرہ){
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
  واپس 1;
#ورنہ
  unixShm *pX;
  صحیح a_تالا[SQLITE_SHM_NLOCK];

  memset(a_تالا, 0, حجم(a_تالا));
  برائے(pX=p_Shm_گرہ->p_پہلا; pX; pX=pX->p_اگلا){
    صحیح i;
    برائے(i=0; i<SQLITE_SHM_NLOCK; i++){
      اگر( pX->excl_نقاب & (1<<i) ){
        أكد( a_تالا[i]==0 );
        a_تالا[i] = -1;
      }ورنہ اگر( pX->shared_نقاب & (1<<i) ){
        أكد( a_تالا[i]>=0 );
        a_تالا[i]++;
      }
    }
  }

  أكد( 0==memcmp(p_Shm_گرہ->a_تالا, a_تالا, حجم(a_تالا)) );
  واپس (memcmp(p_Shm_گرہ->a_تالا, a_تالا, حجم(a_تالا))==0);
#اگر_ختم
}
#اگر_ختم /* !defined(SQLITE_WASI) && !defined(SQLITE_OMIT_WAL) */

/*
** Change the lock state for a shared-memory segment.
**
** Note that the relationship between SHARED and EXCLUSIVE locks is a little
** different here than in posix.  In xShmLock(), one can go from unlocked
** to shared and back or from unlocked to exclusive and back.  But one may
** not go from shared to exclusive or from exclusive to shared.
*/
ساکن صحیح unix_Shm_تالا(
  sqlite3_file *fd,          /* Database file holding the shared memory */
  صحیح ofst,                  /* First lock to acquire or release */
  صحیح n,                     /* Number of locks to acquire or release */
  صحیح flags                  /* What to do with the lock */
){
  unixFile *pDbFd = (unixFile*)fd;      /* Connection holding shared memory */
  unixShm *p;                           /* The shared memory being locked */
  unix_Shm_گرہ *p_Shm_گرہ;                /* The underlying file iNode */
  صحیح rc = SQLITE_OK;                   /* Result code */
  u16 نقاب = (1<<(ofst+n)) - (1<<ofst); /* Mask of locks to take or release */
  صحیح *a_تالا;

  p = pDbFd->pShm;
  اگر( p==0 ) واپس SQLITE_IOERR_SHMLOCK;
  p_Shm_گرہ = p->p_Shm_گرہ;
  اگر( NEVER(p_Shm_گرہ==0) ) واپس SQLITE_IOERR_SHMLOCK;
  a_تالا = p_Shm_گرہ->a_تالا;

  أكد( p_Shm_گرہ==pDbFd->pInode->p_Shm_گرہ );
  أكد( p_Shm_گرہ->pInode==pDbFd->pInode );
  أكد( ofst>=0 && ofst+n<=SQLITE_SHM_NLOCK );
  أكد( n>=1 );
  أكد( flags==(SQLITE_SHM_تالا | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_تالا | SQLITE_SHM_EXCLUSIVE)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED)
       || flags==(SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE) );
  أكد( n==1 || (flags & SQLITE_SHM_EXCLUSIVE)!=0 );
  أكد( p_Shm_گرہ->hShm>=0 || pDbFd->pInode->b_Process_تالا==1 );
  أكد( p_Shm_گرہ->hShm<0 || pDbFd->pInode->b_Process_تالا==0 );

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
  ** This is assert()ed below.
  */
  أكد( flags!=(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_تالا) 
       || 0==(p->excl_نقاب & نقاب) 
  );
  اگر( ((flags & SQLITE_SHM_UNLOCK) && ((p->excl_نقاب|p->shared_نقاب) & نقاب))
   || (flags==(SQLITE_SHM_SHARED|SQLITE_SHM_تالا) && 0==(p->shared_نقاب & نقاب))
   || (flags==(SQLITE_SHM_EXCLUSIVE|SQLITE_SHM_تالا))
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
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
    صحیح iMutex;
    برائے(iMutex=ofst; iMutex<ofst+n; iMutex++){
      اگر( flags==(SQLITE_SHM_تالا|SQLITE_SHM_EXCLUSIVE) ){
        rc = sqlite3_mutex_try(p_Shm_گرہ->aMutex[iMutex]);
        اگر( rc!=SQLITE_OK ) جاؤ leave_shmnode_mutexes;
      }ورنہ{
        sqlite3_mutex_enter(p_Shm_گرہ->aMutex[iMutex]);
      }
    }
#ورنہ
    sqlite3_mutex_enter(p_Shm_گرہ->pShmMutex);
#اگر_ختم

    اگر( ALWAYS(rc==SQLITE_OK) ){
      اگر( flags & SQLITE_SHM_UNLOCK ){
        /* Case (a) - unlock.  */
        صحیح bUnlock = 1;
        أكد( (p->excl_نقاب & p->shared_نقاب)==0 );
        أكد( !(flags & SQLITE_SHM_EXCLUSIVE) || (p->excl_نقاب & نقاب)==نقاب );
        أكد( !(flags & SQLITE_SHM_SHARED) || (p->shared_نقاب & نقاب)==نقاب );
  
        /* If this is a SHARED lock being unlocked, it is possible that other
        ** clients within this process are holding the same SHARED lock. In
        ** this case, set bUnlock to 0 so that the posix lock is not removed
        ** from the file-descriptor below.  */
        اگر( flags & SQLITE_SHM_SHARED ){
          أكد( n==1 );
          أكد( a_تالا[ofst]>=1 );
          اگر( a_تالا[ofst]>1 ){
            bUnlock = 0;
            a_تالا[ofst]--;
            p->shared_نقاب &= ~نقاب;
          }
        }
  
        اگر( bUnlock ){
          rc = unix_Shm_System_تالا(pDbFd, F_UNLCK, ofst+UNIX_SHM_بنیاد, n);
          اگر( rc==SQLITE_OK ){
            memset(&a_تالا[ofst], 0, حجم(صحیح)*n);
            p->shared_نقاب &= ~نقاب;
            p->excl_نقاب &= ~نقاب;
          }
        }
      }ورنہ اگر( flags & SQLITE_SHM_SHARED ){
        /* Case (b) - a shared lock.  */
  
        اگر( a_تالا[ofst]<0 ){
          /* An exclusive lock is held by some other connection. BUSY. */ 
          rc = SQLITE_BUSY;
        }ورنہ اگر( a_تالا[ofst]==0 ){
          rc = unix_Shm_System_تالا(pDbFd, F_RDLCK, ofst+UNIX_SHM_بنیاد, n);
        }
  
        /* Get the local shared locks */
        اگر( rc==SQLITE_OK ){
          p->shared_نقاب |= نقاب;
          a_تالا[ofst]++;
        }
      }ورنہ{
        /* Case (c) - an exclusive lock.  */
        صحیح ii;
  
        أكد( flags==(SQLITE_SHM_تالا|SQLITE_SHM_EXCLUSIVE) );
        أكد( (p->shared_نقاب & نقاب)==0 );
        أكد( (p->excl_نقاب & نقاب)==0 );
  
        /* Make sure no sibling connections hold locks that will block this
        ** lock.  If any do, return SQLITE_BUSY right away.  */
        برائے(ii=ofst; ii<ofst+n; ii++){
          اگر( a_تالا[ii] ){
            rc = SQLITE_BUSY;
            توڑو;
          }
        }
  
        /* Get the exclusive locks at the system level. Then if successful
        ** also update the in-memory values. */
        اگر( rc==SQLITE_OK ){
          rc = unix_Shm_System_تالا(pDbFd, F_WRLCK, ofst+UNIX_SHM_بنیاد, n);
          اگر( rc==SQLITE_OK ){
            p->excl_نقاب |= نقاب;
            برائے(ii=ofst; ii<ofst+n; ii++){
              a_تالا[ii] = -1;
            }
          }
        }
      }
      أكد( assertLockingArrayOk(p_Shm_گرہ) );
    }

    /* Drop the mutexes acquired above. */
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
  leave_shmnode_mutexes:
    برائے(iMutex--; iMutex>=ofst; iMutex--){
      sqlite3_mutex_leave(p_Shm_گرہ->aMutex[iMutex]);
    }
#ورنہ
    sqlite3_mutex_leave(p_Shm_گرہ->pShmMutex);
#اگر_ختم
  }

  OSTRACE(("SHM-LOCK shmid-%d, pid-%d got %03x,%03x\n",
           p->id, osGetpid(0), p->shared_نقاب, p->excl_نقاب));
  واپس rc;
}

/*
** Implement a memory barrier or memory fence on shared memory.
**
** All loads and stores begun before the barrier must complete before
** any load or store begun after the barrier.
*/
ساکن خالی unixShmBarrier(
  sqlite3_file *fd                /* Database file holding the shared memory */
){
  غیر_مستعمل_PARAMETER(fd);
  sqlite3_حافظہ_Barrier();         /* compiler-defined memory barrier */
  أكد( fd->pMethods->x_تالا==nolock_تالا
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
ساکن صحیح unixShmUnmap(
  sqlite3_file *fd,               /* The underlying database file */
  صحیح deleteFlag                  /* Delete shared-memory if true */
){
  unixShm *p;                     /* The connection to be closed */
  unix_Shm_گرہ *p_Shm_گرہ;          /* The underlying shared-memory file */
  unixShm **pp;                   /* For looping over sibling connections */
  unixFile *pDbFd;                /* The underlying database file */

  pDbFd = (unixFile*)fd;
  p = pDbFd->pShm;
  اگر( p==0 ) واپس SQLITE_OK;
  p_Shm_گرہ = p->p_Shm_گرہ;

  أكد( p_Shm_گرہ==pDbFd->pInode->p_Shm_گرہ );
  أكد( p_Shm_گرہ->pInode==pDbFd->pInode );

  /* Remove connection p from the set of connections associated
  ** with pShmNode */
  sqlite3_mutex_enter(p_Shm_گرہ->pShmMutex);
  برائے(pp=&p_Shm_گرہ->p_پہلا; (*pp)!=p; pp = &(*pp)->p_اگلا){}
  *pp = p->p_اگلا;

  /* Free the connection p */
  sqlite3_free(p);
  pDbFd->pShm = 0;
  sqlite3_mutex_leave(p_Shm_گرہ->pShmMutex);

  /* If pShmNode->nRef has reached 0, then close the underlying
  ** shared-memory file, too */
  أكد( unixFileMutexNotheld(pDbFd) );
  unixEnterMutex();
  أكد( p_Shm_گرہ->nRef>0 );
  p_Shm_گرہ->nRef--;
  اگر( p_Shm_گرہ->nRef==0 ){
    اگر( deleteFlag && p_Shm_گرہ->hShm>=0 ){
      osUnlink(p_Shm_گرہ->zFilename);
    }
    unixShmPurge(pDbFd);
  }
  unixLeaveMutex();

  واپس SQLITE_OK;
}


#ورنہ
# define unixShmMap     0
# define unix_Shm_تالا    0
# define unixShmBarrier 0
# define unixShmUnmap   0
#اگر_ختم /* #ifndef SQLITE_OMIT_WAL */

#اگر SQLITE_MAX_MMAP_سائز>0
/*
** If it is currently memory mapped, unmap file pFd.
*/
ساکن خالی unixUnmapfile(unixFile *pFd){
  أكد( pFd->n_Fetch_برآمد==0 );
  اگر( pFd->pMapRegion ){
    osMunmap(pFd->pMapRegion, pFd->mmap_سائز_Actual);
    pFd->pMapRegion = 0;
    pFd->mmap_سائز = 0;
    pFd->mmap_سائز_Actual = 0;
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
ساکن خالی unixRemapfile(
  unixFile *pFd,                  /* File descriptor object */
  i64 nNew                        /* Required mapping size */
){
  ثابت حرف *zErr = "mmap";
  صحیح h = pFd->h;                      /* File descriptor open on db file */
  u8 *pOrig = (u8 *)pFd->pMapRegion;   /* Pointer to current file mapping */
  u8 *pNew = 0;                        /* Location of new mapping */
  صحیح flags = PROT_READ;               /* Flags to pass to mmap() */

  أكد( pFd->n_Fetch_برآمد==0 );
  أكد( nNew>pFd->mmap_سائز );
  أكد( nNew<=pFd->mmap_سائز_Max );
  أكد( nNew>0 );
  أكد( pFd->mmap_سائز_Actual>=pFd->mmap_سائز );
  أكد( MAP_FAILED!=0 );

#اگر_تعریف_شدہ SQLITE_MMAP_READWRITE
  اگر( (pFd->ctrlFlags & UNIXFILE_RDONLY)==0 ) flags |= PROT_WRITE;
#اگر_ختم

  اگر( pOrig ){
    i64 nReuse = pFd->mmap_سائز & ~((صحیح)osGetpagesize()-1);

    /* Unmap any part of the current mapping that will not be reused */
    اگر( nReuse<pFd->mmap_سائز_Actual ){
      osMunmap(&pOrig[nReuse], pFd->mmap_سائز_Actual - nReuse);
    }

    /* If nReuse is now zero, then the entire mapping was unmapped by the
    ** call above. In this case do not attempt to extend pOrig. */
    اگر( nReuse>0 ){
#اگر HAVE_MREMAP
      pNew = osMremap(pOrig, nReuse, nNew, MREMAP_MAYMOVE);
      zErr = "mremap";
#ورنہ
      pNew = osMmap(&pOrig[nReuse], nNew-nReuse, flags, MAP_SHARED, h, nReuse);
      اگر( pNew!=MAP_FAILED ){
        اگر( pNew!=&pOrig[nReuse] ){
          osMunmap(pNew, nNew - nReuse);
          pNew = 0;
        }ورنہ{
          pNew = pOrig;
        }
      }
#اگر_ختم

      /* The attempt to extend the existing mapping failed. Free it. */
      اگر( pNew==MAP_FAILED || pNew==0 ){
        osMunmap(pOrig, nReuse);
      }
    }
  }

  /* If pNew is still NULL, try to create an entirely new mapping. */
  اگر( pNew==0 ){
    pNew = osMmap(0, nNew, flags, MAP_SHARED, h, 0);
  }

  اگر( pNew==MAP_FAILED ){
    pNew = 0;
    nNew = 0;
    unixLogError(SQLITE_OK, zErr, pFd->z_راستہ);

    /* If the mmap() above failed, assume that all subsequent mmap() calls
    ** will probably fail too. Fall back to using xRead/xWrite exclusively
    ** in this case.  */
    pFd->mmap_سائز_Max = 0;
  }
  pFd->pMapRegion = (خالی *)pNew;
  pFd->mmap_سائز = pFd->mmap_سائز_Actual = nNew;
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
ساکن صحیح unixMapfile(unixFile *pFd, i64 nMap){
  أكد( nMap>=0 || pFd->n_Fetch_برآمد==0 );
  أكد( nMap>0 || (pFd->mmap_سائز==0 && pFd->pMapRegion==0) );
  اگر( pFd->n_Fetch_برآمد>0 ) واپس SQLITE_OK;

  اگر( nMap<0 ){
    ساخت stat statbuf;          /* Low-level file information */
    اگر( osFstat(pFd->h, &statbuf) ){
      واپس SQLITE_IOERR_FSTAT;
    }
    nMap = statbuf.st_سائز;
  }
  اگر( nMap>pFd->mmap_سائز_Max ){
    nMap = pFd->mmap_سائز_Max;
  }

  أكد( nMap>0 || (pFd->mmap_سائز==0 && pFd->pMapRegion==0) );
  اگر( nMap!=pFd->mmap_سائز ){
    unixRemapfile(pFd, nMap);
  }

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
** release the reference by calling unixUnfetch().
*/
ساکن صحیح unixFetch(sqlite3_file *fd, i64 iOff, صحیح nAmt, خالی **pp){
#اگر SQLITE_MAX_MMAP_سائز>0
  unixFile *pFd = (unixFile *)fd;   /* The underlying database file */
#اگر_ختم
  *pp = 0;

#اگر SQLITE_MAX_MMAP_سائز>0
  اگر( pFd->mmap_سائز_Max>0 ){
    /* Ensure that there is always at least a 256 byte buffer of addressable
    ** memory following the returned page. If the database is corrupt,
    ** SQLite may overread the page slightly (in practice only a few bytes,
    ** but 256 is safe, round, number).  */
    ثابت صحیح n_Eof_بفر = 256;
    اگر( pFd->pMapRegion==0 ){
      صحیح rc = unixMapfile(pFd, -1);
      اگر( rc!=SQLITE_OK ) واپس rc;
    }
    اگر( pFd->mmap_سائز >= (iOff+nAmt+n_Eof_بفر) ){
      *pp = &((u8 *)pFd->pMapRegion)[iOff];
      pFd->n_Fetch_برآمد++;
    }
  }
#اگر_ختم
  واپس SQLITE_OK;
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
ساکن صحیح unixUnfetch(sqlite3_file *fd, i64 iOff, خالی *p){
#اگر SQLITE_MAX_MMAP_سائز>0
  unixFile *pFd = (unixFile *)fd;   /* The underlying database file */
  غیر_مستعمل_PARAMETER(iOff);

  /* If p==0 (unmap the entire file) then there must be no outstanding
  ** xFetch references. Or, if p!=0 (meaning it is an xFetch reference),
  ** then there must be at least one outstanding.  */
  أكد( (p==0)==(pFd->n_Fetch_برآمد==0) );

  /* If p!=0, it must match the iOff value. */
  أكد( p==0 || p==&((u8 *)pFd->pMapRegion)[iOff] );

  اگر( p ){
    pFd->n_Fetch_برآمد--;
  }ورنہ{
    unixUnmapfile(pFd);
  }

  أكد( pFd->n_Fetch_برآمد>=0 );
#ورنہ
  غیر_مستعمل_PARAMETER(fd);
  غیر_مستعمل_PARAMETER(p);
  غیر_مستعمل_PARAMETER(iOff);
#اگر_ختم
  واپس SQLITE_OK;
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
#تعریف_کرو IOMETHODS(FINDER,طریقہ,نسخہ,CLOSE,تالا_2,UNLOCK,CKLOCK,SHMMAP)     \
ساکن ثابت sqlite3_io_methods طریقہ = {                                   \
   نسخہ,                    /* iVersion */                                \
   CLOSE,                      /* xClose */                                  \
   unixRead,                   /* xRead */                                   \
   unixWrite,                  /* xWrite */                                  \
   unixTruncate,               /* xTruncate */                               \
   unixSync,                   /* xSync */                                   \
   unix_File_سائز,               /* xFileSize */                               \
   تالا_2,                       /* xLock */                                   \
   UNLOCK,                     /* xUnlock */                                 \
   CKLOCK,                     /* xCheckReservedLock */                      \
   unixFileControl,            /* xFileControl */                            \
   unix_Sector_سائز,             /* xSectorSize */                             \
   unixDeviceCharacteristics,  /* xDeviceCapabilities */                     \
   SHMMAP,                     /* xShmMap */                                 \
   unix_Shm_تالا,                /* xShmLock */                                \
   unixShmBarrier,             /* xShmBarrier */                             \
   unixShmUnmap,               /* xShmUnmap */                               \
   unixFetch,                  /* xFetch */                                  \
   unixUnfetch,                /* xUnfetch */                                \
};                                                                           \
ساکن ثابت sqlite3_io_methods *FINDER##Impl(ثابت حرف *z, unixFile *p){   \
  غیر_مستعمل_PARAMETER(z); غیر_مستعمل_PARAMETER(p);                                  \
  واپس &طریقہ;                                                            \
}                                                                            \
ساکن ثابت sqlite3_io_methods *(*ثابت FINDER)(ثابت حرف*,unixFile *p)    \
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
  unix_تالا,                 /* xLock method */
  unixUnlock,               /* xUnlock method */
  unix_Check_Reserved_تالا,    /* xCheckReservedLock method */
  unixShmMap                /* xShmMap method */
)
IOMETHODS(
  nolockIoFinder,           /* Finder function name */
  nolockIoMethods,          /* sqlite3_io_methods object name */
  3,                        /* shared memory and mmap are enabled */
  nolockClose,              /* xClose method */
  nolock_تالا,               /* xLock method */
  nolockUnlock,             /* xUnlock method */
  nolock_Check_Reserved_تالا,  /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
IOMETHODS(
  dotlockIoFinder,          /* Finder function name */
  dotlockIoMethods,         /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  dotlockClose,             /* xClose method */
  dotlock_تالا,              /* xLock method */
  dotlockUnlock,            /* xUnlock method */
  dotlock_Check_Reserved_تالا, /* xCheckReservedLock method */
  0                         /* xShmMap method */
)

#اگر SQLITE_ENABLE_LOCKING_STYLE
IOMETHODS(
  flockIoFinder,            /* Finder function name */
  flockIoMethods,           /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  flockClose,               /* xClose method */
  flock_تالا,                /* xLock method */
  flockUnlock,              /* xUnlock method */
  flock_Check_Reserved_تالا,   /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#اگر_ختم

#اگر OS_VXWORKS
IOMETHODS(
  semIoFinder,              /* Finder function name */
  semIoMethods,             /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  semXClose,                /* xClose method */
  semXLock,                 /* xLock method */
  semXUnlock,               /* xUnlock method */
  sem_XCheck_Reserved_تالا,    /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#اگر_ختم

#اگر defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
IOMETHODS(
  afpIoFinder,              /* Finder function name */
  afpIoMethods,             /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  afpClose,                 /* xClose method */
  afp_تالا,                  /* xLock method */
  afpUnlock,                /* xUnlock method */
  afp_Check_Reserved_تالا,     /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#اگر_ختم

/*
** The proxy locking method is a "super-method" in the sense that it
** opens secondary file descriptors for the conch and lock files and
** it uses proxy, dot-file, AFP, and flock() locking methods on those
** secondary files.  For this reason, the division that implements
** proxy locking is located much further down in the file.  But we need
** to go ahead and define the sqlite3_io_methods and finder function
** for proxy locking here.  So we forward declare the I/O methods.
*/
#اگر defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
ساکن صحیح proxyClose(sqlite3_file*);
ساکن صحیح proxy_تالا(sqlite3_file*, صحیح);
ساکن صحیح proxyUnlock(sqlite3_file*, صحیح);
ساکن صحیح proxy_Check_Reserved_تالا(sqlite3_file*, صحیح*);
IOMETHODS(
  proxyIoFinder,            /* Finder function name */
  proxyIoMethods,           /* sqlite3_io_methods object name */
  1,                        /* shared memory is disabled */
  proxyClose,               /* xClose method */
  proxy_تالا,                /* xLock method */
  proxyUnlock,              /* xUnlock method */
  proxy_Check_Reserved_تالا,   /* xCheckReservedLock method */
  0                         /* xShmMap method */
)
#اگر_ختم

/* nfs lockd on OSX 10.3+ doesn't clear write locks when a read lock is set */
#اگر defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
IOMETHODS(
  nfsIoFinder,               /* Finder function name */
  nfsIoMethods,              /* sqlite3_io_methods object name */
  1,                         /* shared memory is disabled */
  unixClose,                 /* xClose method */
  unix_تالا,                  /* xLock method */
  nfsUnlock,                 /* xUnlock method */
  unix_Check_Reserved_تالا,     /* xCheckReservedLock method */
  0                          /* xShmMap method */
)
#اگر_ختم

#اگر defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
/*
** This "finder" function attempts to determine the best locking strategy
** for the database file "filePath".  It then returns the sqlite3_io_methods
** object that implements that strategy.
**
** This is for MacOSX only.
*/
ساکن ثابت sqlite3_io_methods *autolockIoFinderImpl(
  ثابت حرف *file_راستہ,    /* name of the database file */
  unixFile *pNew           /* open file object for the database file */
){
  ساکن ثابت ساخت Mapping {
    ثابت حرف *zFilesystem;              /* Filesystem type name */
    ثابت sqlite3_io_methods *pMethods;   /* Appropriate locking method */
  } aMap[] = {
    { "hfs",    &posixIoMethods },
    { "ufs",    &posixIoMethods },
    { "afpfs",  &afpIoMethods },
    { "smbfs",  &afpIoMethods },
    { "webdav", &nolockIoMethods },
    { 0, 0 }
  };
  صحیح i;
  ساخت statfs fsInfo;
  ساخت flock تالا_Info;

  اگر( !file_راستہ ){
    /* If filePath==NULL that means we are dealing with a transient file
    ** that does not need to be locked. */
    واپس &nolockIoMethods;
  }
  اگر( statfs(file_راستہ, &fsInfo) != -1 ){
    اگر( fsInfo.f_flags & MNT_RDONLY ){
      واپس &nolockIoMethods;
    }
    برائے(i=0; aMap[i].zFilesystem; i++){
      اگر( strcmp(fsInfo.f_fstypename, aMap[i].zFilesystem)==0 ){
        واپس aMap[i].pMethods;
      }
    }
  }

  /* Default case. Handles, amongst others, "nfs".
  ** Test byte-range lock using fcntl(). If the call succeeds,
  ** assume that the file-system supports POSIX style locks.
  */
  تالا_Info.l_len = 1;
  تالا_Info.l_آغاز = 0;
  تالا_Info.l_whence = SEEK_SET;
  تالا_Info.l_type = F_RDLCK;
  اگر( osFcntl(pNew->h, F_GETLK, &تالا_Info)!=-1 ) {
    اگر( strcmp(fsInfo.f_fstypename, "nfs")==0 ){
      واپس &nfsIoMethods;
    } ورنہ {
      واپس &posixIoMethods;
    }
  }ورنہ{
    واپس &dotlockIoMethods;
  }
}
ساکن ثابت sqlite3_io_methods
  *(*ثابت autolockIoFinder)(ثابت حرف*,unixFile*) = autolockIoFinderImpl;

#اگر_ختم /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */

#اگر OS_VXWORKS
/*
** This "finder" function for VxWorks checks to see if posix advisory
** locking works.  If it does, then that is what is used.  If it does not
** work, then fallback to named semaphore locking.
*/
ساکن ثابت sqlite3_io_methods *vxworksIoFinderImpl(
  ثابت حرف *file_راستہ,    /* name of the database file */
  unixFile *pNew           /* the open file object */
){
  ساخت flock تالا_Info;

  اگر( !file_راستہ ){
    /* If filePath==NULL that means we are dealing with a transient file
    ** that does not need to be locked. */
    واپس &nolockIoMethods;
  }

  /* Test if fcntl() is supported and use POSIX style locks.
  ** Otherwise fall back to the named semaphore method.
  */
  تالا_Info.l_len = 1;
  تالا_Info.l_آغاز = 0;
  تالا_Info.l_whence = SEEK_SET;
  تالا_Info.l_type = F_RDLCK;
  اگر( osFcntl(pNew->h, F_GETLK, &تالا_Info)!=-1 ) {
    واپس &posixIoMethods;
  }ورنہ{
    واپس &semIoMethods;
  }
}
ساکن ثابت sqlite3_io_methods
  *(*ثابت vxworksIoFinder)(ثابت حرف*,unixFile*) = vxworksIoFinderImpl;

#اگر_ختم /* OS_VXWORKS */

/*
** An abstract type for a pointer to an IO method finder function:
*/
قسمتعریف ثابت sqlite3_io_methods *(*finder_type)(ثابت حرف*,unixFile*);


/****************************************************************************
**************************** sqlite3_vfs methods ****************************
**
** This division contains the implementation of methods on the
** sqlite3_vfs object.
*/

/*
** Initialize the contents of the unixFile structure pointed to by pId.
*/
ساکن صحیح fill_اندر_Unix_File(
  sqlite3_vfs *pVfs,      /* Pointer to vfs object */
  صحیح h,                  /* Open file descriptor of file being opened */
  sqlite3_file *pId,      /* Write to the unixFile structure here */
  ثابت حرف *zFilename,  /* Name of the file being opened */
  صحیح ctrlFlags           /* Zero or more UNIXFILE_* values */
){
  ثابت sqlite3_io_methods *pLockingStyle;
  unixFile *pNew = (unixFile *)pId;
  صحیح rc = SQLITE_OK;

  أكد( pNew->pInode==تہی );

  /* No locking occurs in temporary files */
  أكد( zFilename!=0 || (ctrlFlags & UNIXFILE_NOLOCK)!=0 );

  OSTRACE(("OPEN    %-3d %s\n", h, zFilename));
  pNew->h = h;
  pNew->pVfs = pVfs;
  pNew->z_راستہ = zFilename;
  pNew->ctrlFlags = (u8)ctrlFlags;
#اگر SQLITE_MAX_MMAP_سائز>0
  pNew->mmap_سائز_Max = sqlite3GlobalConfig.szMmap;
#اگر_ختم
  اگر( sqlite3_uri_boolean(((ctrlFlags & UNIXFILE_URI) ? zFilename : 0),
                           "psow", SQLITE_POWERSAFE_OVERWRITE) ){
    pNew->ctrlFlags |= UNIXFILE_PSOW;
  }
  اگر( strcmp(pVfs->zName,"unix-excl")==0 ){
    pNew->ctrlFlags |= UNIXFILE_EXCL;
  }

#اگر OS_VXWORKS
  pNew->pId = vxworks_ڈھونڈو_File_Id(zFilename);
  اگر( pNew->pId==0 ){
    ctrlFlags |= UNIXFILE_NOLOCK;
    rc = SQLITE_NOMEM_BKPT;
  }
#اگر_ختم

  اگر( ctrlFlags & UNIXFILE_NOLOCK ){
    pLockingStyle = &nolockIoMethods;
  }ورنہ{
    pLockingStyle = (**(finder_type*)pVfs->p_App_ڈیٹا)(zFilename, pNew);
#اگر SQLITE_ENABLE_LOCKING_STYLE
    /* Cache zFilename in the locking context (AFP and dotlock override) for
    ** proxyLock activation is possible (remote proxy is based on db name)
    ** zFilename remains valid until file is closed, to support */
    pNew->lockingContext = (خالی*)zFilename;
#اگر_ختم
  }

  اگر( pLockingStyle == &posixIoMethods
#اگر defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE
    || pLockingStyle == &nfsIoMethods
#اگر_ختم
  ){
    unixEnterMutex();
    rc = ڈھونڈو_Inode_Info(pNew, &pNew->pInode);
    اگر( rc!=SQLITE_OK ){
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

#اگر SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
  ورنہ اگر( pLockingStyle == &afpIoMethods ){
    /* AFP locking uses the file path so it needs to be included in
    ** the afpLockingContext.
    */
    afpLockingContext *pCtx;
    pNew->lockingContext = pCtx = sqlite3_malloc64( حجم(*pCtx) );
    اگر( pCtx==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }ورنہ{
      /* NB: zFilename exists and remains valid until the file is closed
      ** according to requirement F11141.  So we do not need to make a
      ** copy of the filename. */
      pCtx->db_راستہ = zFilename;
      pCtx->reserved = 0;
      srandomdev();
      unixEnterMutex();
      rc = ڈھونڈو_Inode_Info(pNew, &pNew->pInode);
      اگر( rc!=SQLITE_OK ){
        sqlite3_free(pNew->lockingContext);
        robust_close(pNew, h, __LINE__);
        h = -1;
      }
      unixLeaveMutex();      
    }
  }
#اگر_ختم

  ورنہ اگر( pLockingStyle == &dotlockIoMethods ){
    /* Dotfile locking uses the file path so it needs to be included in
    ** the dotlockLockingContext
    */
    حرف *z_تالا_File;
    صحیح nFilename;
    أكد( zFilename!=0 );
    nFilename = (صحیح)لڑی_لمبائی(zFilename) + 6;
    z_تالا_File = (حرف *)sqlite3_malloc64(nFilename);
    اگر( z_تالا_File==0 ){
      rc = SQLITE_NOMEM_BKPT;
    }ورنہ{
      sqlite3_snprintf(nFilename, z_تالا_File, "%s" DOTLOCK_SUFFIX, zFilename);
    }
    pNew->lockingContext = z_تالا_File;
  }

#اگر OS_VXWORKS
  ورنہ اگر( pLockingStyle == &semIoMethods ){
    /* Named semaphore locking uses the file path so it needs to be
    ** included in the semLockingContext
    */
    unixEnterMutex();
    rc = ڈھونڈو_Inode_Info(pNew, &pNew->pInode);
    اگر( (rc==SQLITE_OK) && (pNew->pInode->pSem==تہی) ){
      حرف *zSemName = pNew->pInode->aSemName;
      صحیح n;
      sqlite3_snprintf(MAX_PATHNAME, zSemName, "/%s.sem",
                       pNew->pId->zCanonicalName);
      برائے( n=1; zSemName[n]; n++ )
        اگر( zSemName[n]=='/' ) zSemName[n] = '_';
      pNew->pInode->pSem = sem_open(zSemName, O_CREAT, 0666, 1);
      اگر( pNew->pInode->pSem == SEM_FAILED ){
        rc = SQLITE_NOMEM_BKPT;
        pNew->pInode->aSemName[0] = '\0';
      }
    }
    unixLeaveMutex();
  }
#اگر_ختم

  store_آخری_Errno(pNew, 0);
#اگر OS_VXWORKS
  اگر( rc!=SQLITE_OK ){
    اگر( h>=0 ){
      robust_close(pNew, h, __LINE__);
      h = -1;
    }
    اگر( pNew->ctrlFlags & UNIXFILE_DELETE ){
      osUnlink(zFilename);
    }
    اگر( pNew->pId ){
      vxworksReleaseFileId(pNew->pId);
      pNew->pId = 0;
    }
  }
#اگر_ختم
  اگر( rc!=SQLITE_OK ){
    اگر( h>=0 ) robust_close(pNew, h, __LINE__);
  }ورنہ{
    pId->pMethods = pLockingStyle;
    Open_شمار(+1);
    verifyDbFile(pNew);
  }
  واپس rc;
}

/*
** Return the name of a directory in which to put temporary files.
** If no suitable temporary file directory can be found, return NULL.
**
** The return value might be a string obtained from getenv() and so
** the return value should not be used after any call to setenv() or
** putenv() as that value might have been freed.
*/
ساکن ثابت حرف *unix_عارضی_File_Dir(خالی){
  غیرعلامتی صحیح i;
  ساخت stat buf;
  ثابت حرف *zDir;

  برائے(i=0; i<7; i++){
    انتخاب( i ){
      حالت 0:  zDir = sqlite3_عارضی_directory;   توڑو;
      حالت 1:  zDir = getenv("SQLITE_TMPDIR");  توڑو;
      حالت 2:  zDir = getenv("TMPDIR");         توڑو;
      حالت 3:  zDir = "/var/tmp";               توڑو;
      حالت 4:  zDir = "/usr/tmp";               توڑو;
      حالت 5:  zDir = "/tmp";                   توڑو;
      طےشدہ: zDir = ".";                      توڑو;
    }
    اگر( zDir!=0
#اگر OS_VXWORKS
     && zDir[0]=='/'
#اگر_ختم
     && osStat(zDir, &buf)==0
     && S_ISDIR(buf.st_منوال)
     && osAccess(zDir, 03)==0
    ){
      واپس zDir;
    }
  }
  واپس 0;
}

/*
** Create a temporary file name in zBuf.  zBuf must be allocated
** by the calling process and must be big enough to hold at least
** pVfs->mxPathname bytes.
*/
ساکن صحیح unixGetTempname(صحیح nBuf, حرف *zBuf){
  ثابت حرف *zDir;
  صحیح i_حد_بندی = 0;
  صحیح rc = SQLITE_OK;

  /* It's odd to simulate an io-error here, but really this is just
  ** using the io-error infrastructure to test that SQLite handles this
  ** function failing.
  */
  zBuf[0] = 0;
  SimulateIOError( واپس SQLITE_IOERR );

  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  zDir = unix_عارضی_File_Dir();
  اگر( zDir==0 ){
    rc = SQLITE_IOERR_GETTEMPPATH;
  }ورنہ{
    کرو{
      u64 r;
      sqlite3_randomness(حجم(r), &r);
      أكد( nBuf>2 );
      zBuf[nBuf-2] = 0;
      sqlite3_snprintf(nBuf, zBuf, "%s/"SQLITE_عارضی_FILE_سابقہ"%llx%c",
                       zDir, r, 0);
      اگر( zBuf[nBuf-2]!=0 || (i_حد_بندی++)>10 ){
        rc = SQLITE_ERROR;
        توڑو;
      }
    }جب( osAccess(zBuf,0)==0 );
  }
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_TEMPDIR));
  واپس rc;
}

#اگر SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
/*
** Routine to transform a unixFile into a proxy-locking unixFile.
** Implementation in the proxy-lock division, but used by unixOpen()
** if SQLITE_PREFER_PROXY_LOCKING is defined.
*/
ساکن صحیح proxyTransformUnixFile(unixFile*, ثابت حرف*);
#اگر_ختم

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
ساکن Unix_غیر_مستعمل_Fd *ڈھونڈو_Reusable_Fd(ثابت حرف *z_راستہ, صحیح flags){
  Unix_غیر_مستعمل_Fd *p_غیر_مستعمل = 0;

  /* Do not search for an unused file descriptor on vxworks. Not because
  ** vxworks would not benefit from the change (it might, we're not sure),
  ** but because no way to test it is currently available. It is better
  ** not to risk breaking vxworks support for the sake of such an obscure
  ** feature.  */
#اگر !OS_VXWORKS
  ساخت stat sStat;                   /* Results of stat() call */

  unixEnterMutex();

  /* A stat() call may fail for various reasons. If this happens, it is
  ** almost certain that an open() call on the same path will also fail.
  ** For this reason, if an error occurs in the stat() call here, it is
  ** ignored and -1 is returned. The caller will try to open a new file
  ** descriptor on the same path, fail, and return an error to SQLite.
  **
  ** Even if a subsequent open() call does succeed, the consequences of
  ** not searching for a reusable file descriptor are not dire.  */
  اگر( inodeList!=0 && 0==osStat(z_راستہ, &sStat) ){
    unixInodeInfo *pInode;

    pInode = inodeList;
    جب( pInode && (pInode->fileId.dev!=sStat.st_dev
                     || pInode->fileId.ino!=(u64)sStat.st_ino) ){
       pInode = pInode->p_اگلا;
    }
    اگر( pInode ){
      Unix_غیر_مستعمل_Fd **pp;
      أكد( sqlite3_mutex_notheld(pInode->p_تالا_Mutex) );
      sqlite3_mutex_enter(pInode->p_تالا_Mutex);
      flags &= (SQLITE_OPEN_READONLY|SQLITE_OPEN_READWRITE);
      برائے(pp=&pInode->p_غیر_مستعمل; *pp && (*pp)->flags!=flags; pp=&((*pp)->p_اگلا));
      p_غیر_مستعمل = *pp;
      اگر( p_غیر_مستعمل ){
        *pp = p_غیر_مستعمل->p_اگلا;
      }
      sqlite3_mutex_leave(pInode->p_تالا_Mutex);
    }
  }
  unixLeaveMutex();
#اگر_ختم    /* if !OS_VXWORKS */
  واپس p_غیر_مستعمل;
}

/*
** Find the mode, uid and gid of file zFile.
*/
ساکن صحیح get_File_منوال(
  ثابت حرف *zFile,              /* File name */
  منوال_t *p_منوال,                  /* OUT: Permissions of zFile */
  uid_t *pUid,                    /* OUT: uid of zFile. */
  gid_t *pGid                     /* OUT: gid of zFile. */
){
  ساخت stat sStat;              /* Output of stat() on database file */
  صحیح rc = SQLITE_OK;
  اگر( 0==osStat(zFile, &sStat) ){
    *p_منوال = sStat.st_منوال & 0777;
    *pUid = sStat.st_uid;
    *pGid = sStat.st_gid;
  }ورنہ{
    rc = SQLITE_IOERR_FSTAT;
  }
  واپس rc;
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
ساکن صحیح ڈھونڈو_Create_File_منوال(
  ثابت حرف *z_راستہ,              /* Path of file (possibly) being created */
  صحیح flags,                      /* Flags passed as 4th argument to xOpen() */
  منوال_t *p_منوال,                  /* OUT: Permissions to open file with */
  uid_t *pUid,                    /* OUT: uid to set on the file */
  gid_t *pGid                     /* OUT: gid to set on the file */
){
  صحیح rc = SQLITE_OK;             /* Return Code */
  *p_منوال = 0;
  *pUid = 0;
  *pGid = 0;
  اگر( flags & (SQLITE_OPEN_WAL|SQLITE_OPEN_MAIN_JOURNAL) ){
    حرف zDb[MAX_PATHNAME+1];     /* Database file path */
    صحیح nDb;                      /* Number of valid bytes in zDb */

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
    nDb = sqlite3Strlen30(z_راستہ) - 1;
    جب( nDb>0 && z_راستہ[nDb]!='.' ){
      اگر( z_راستہ[nDb]=='-' ){
        حافظہ_نقل(zDb, z_راستہ, nDb);
        zDb[nDb] = '\0';
        rc = get_File_منوال(zDb, p_منوال, pUid, pGid);
        توڑو;
      }
      nDb--;
    }
  }ورنہ اگر( flags & SQLITE_OPEN_DELETEONCLOSE ){
    *p_منوال = 0600;
  }ورنہ اگر( flags & SQLITE_OPEN_URI ){
    /* If this is a main database file and the file was opened using a URI
    ** filename, check for the "modeof" parameter. If present, interpret
    ** its value as a filename and try to copy the mode, uid and gid from
    ** that file.  */
    ثابت حرف *z = sqlite3_uri_parameter(z_راستہ, "modeof");
    اگر( z ){
      rc = get_File_منوال(z, p_منوال, pUid, pGid);
    }
  }
  واپس rc;
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
ساکن صحیح unixOpen(
  sqlite3_vfs *pVfs,           /* The VFS for which this is the xOpen method */
  ثابت حرف *z_راستہ,           /* Pathname of file to be opened */
  sqlite3_file *pFile,         /* The file descriptor to be filled in */
  صحیح flags,                   /* Input flags to control the opening */
  صحیح *p_برآمد_Flags               /* Output flags returned to SQLite core */
){
  unixFile *p = (unixFile *)pFile;
  صحیح fd = -1;                   /* File descriptor returned by open() */
  صحیح openFlags = 0;             /* Flags to pass to open() */
  صحیح eType = flags&0x0FFF00;  /* Type of file to open */
  صحیح no_تالا;                    /* True to omit locking primitives */
  صحیح rc = SQLITE_OK;            /* Function Return Code */
  صحیح ctrlFlags = 0;             /* UNIXFILE_* flags */

  صحیح isExclusive  = (flags & SQLITE_OPEN_EXCLUSIVE);
  صحیح isDelete     = (flags & SQLITE_OPEN_DELETEONCLOSE);
  صحیح isCreate     = (flags & SQLITE_OPEN_CREATE);
  صحیح isReadonly   = (flags & SQLITE_OPEN_READONLY);
  صحیح isReadWrite  = (flags & SQLITE_OPEN_READWRITE);
#اگر SQLITE_ENABLE_LOCKING_STYLE
  صحیح isAutoProxy  = (flags & SQLITE_OPEN_AUTOPROXY);
#اگر_ختم
#اگر defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
  ساخت statfs fsInfo;
#اگر_ختم

  /* If creating a super- or main-file journal, this function will open
  ** a file-descriptor on the directory too. The first time unixSync()
  ** is called the directory file descriptor will be fsync()ed and close()d.
  */
  صحیح isNewJrnl = (isCreate && (
        eType==SQLITE_OPEN_SUPER_JOURNAL
     || eType==SQLITE_OPEN_MAIN_JOURNAL
     || eType==SQLITE_OPEN_WAL
  ));

  /* If argument zPath is a NULL pointer, this function is required to open
  ** a temporary file. Use this buffer to store the file name in.
  */
  حرف zTmpname[MAX_PATHNAME+2];
  ثابت حرف *zName = z_راستہ;

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

#اگر OS_VXWORKS
  /* The file-ID mechanism used in Vxworks requires that all pathnames
  ** provided to unixOpen must be absolute pathnames. */
  اگر( z_راستہ!=0 && z_راستہ[0]!='/' ){ واپس SQLITE_CANTOPEN; }
#اگر_ختم

  /* Detect a pid change and reset the PRNG.  There is a race condition
  ** here such that two or more threads all trying to open databases at
  ** the same instant might all reset the PRNG.  But multiple resets
  ** are harmless.
  */
  اگر( randomnessPid!=osGetpid(0) ){
    randomnessPid = osGetpid(0);
    sqlite3_randomness(0,0);
  }
  memset(p, 0, حجم(unixFile));

#اگر_تعریف_شدہ SQLITE_ASSERT_NO_FILES
  /* Applications that never read or write a persistent disk files */
  أكد( zName==0 );
#اگر_ختم

  اگر( eType==SQLITE_OPEN_MAIN_DB ){
    Unix_غیر_مستعمل_Fd *p_غیر_مستعمل;
    p_غیر_مستعمل = ڈھونڈو_Reusable_Fd(zName, flags);
    اگر( p_غیر_مستعمل ){
      fd = p_غیر_مستعمل->fd;
    }ورنہ{
      p_غیر_مستعمل = sqlite3_malloc64(حجم(*p_غیر_مستعمل));
      اگر( !p_غیر_مستعمل ){
        واپس SQLITE_NOMEM_BKPT;
      }
    }
    p->p_Preallocated_غیر_مستعمل = p_غیر_مستعمل;

    /* Database filenames are double-zero terminated if they are not
    ** URIs with parameters.  Hence, they can always be passed into
    ** sqlite3_uri_parameter(). */
    أكد( (flags & SQLITE_OPEN_URI) || zName[لڑی_لمبائی(zName)+1]==0 );

  }ورنہ اگر( !zName ){
    /* If zName is NULL, the upper layer is requesting a temp file. */
    أكد( isDelete );
    أكد( !isNewJrnl );
    أكد( isExclusive );
    أكد( isReadWrite );
#اگر defined(__linux__) && defined(O_TMPFILE)
    /* On systems that support O_TMPFILE, use that flag to create a more
    ** secure temporary file that cannot be accessed by other processes
    */
    zName = unix_عارضی_File_Dir();
    اگر( zName
     && (fd = robust_open(zName, O_RDWR|O_EXCL|O_TMPFILE, 0600))>=0
    ){
      rc = fill_اندر_Unix_File(pVfs, fd, pFile, z_راستہ, ctrlFlags|UNIXFILE_NOLOCK);
      جاؤ open_ختم_شدہ;
    }
#اگر_ختم
    rc = unixGetTempname(pVfs->mxPathname, zTmpname);
    اگر( rc!=SQLITE_OK ){
      واپس rc;
    }
    zName = zTmpname;

    /* Generated temporary filenames are always double-zero terminated
    ** for use by sqlite3_uri_parameter(). */
    أكد( zName[لڑی_لمبائی(zName)+1]==0 );
  }

  /* Determine the value of the flags parameter passed to POSIX function
  ** open(). These must be calculated even if open() is not called, as
  ** they may be stored as part of the file handle and used by the
  ** 'conch file' locking functions later on.  */
  اگر( isReadonly )  openFlags |= O_RDONLY;
  اگر( isReadWrite ) openFlags |= O_RDWR;
  اگر( isCreate )    openFlags |= O_CREAT;
  اگر( isExclusive ) openFlags |= (O_EXCL|O_NOFOLLOW);
  openFlags |= (O_LARGEFILE|O_ثنائی|O_NOFOLLOW);

  اگر( fd<0 ){
    منوال_t open_منوال;              /* Permissions to create file with */
    uid_t uid;                    /* Userid for the file */
    gid_t gid;                    /* Groupid for the file */
    rc = ڈھونڈو_Create_File_منوال(zName, flags, &open_منوال, &uid, &gid);
    اگر( rc!=SQLITE_OK ){
      أكد( !p->p_Preallocated_غیر_مستعمل );
      أكد( eType==SQLITE_OPEN_WAL || eType==SQLITE_OPEN_MAIN_JOURNAL );
      واپس rc;
    }
    fd = robust_open(zName, openFlags, open_منوال);
    OSTRACE(("OPENX   %-3d %s 0%o\n", fd, zName, openFlags));
    أكد( !isExclusive || (openFlags & O_CREAT)!=0 );
    اگر( fd<0 ){
      اگر( isNewJrnl && errno==EACCES && osAccess(zName, F_OK) ){
        /* If unable to create a journal because the directory is not
        ** writable, change the error code to indicate that. */
        rc = SQLITE_READONLY_DIRECTORY;
      }ورنہ اگر( errno!=EISDIR && isReadWrite ){
        /* Failed to open the file for read/write access. Try read-only. */
        Unix_غیر_مستعمل_Fd *pReadonly = 0;
        flags &= ~(SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE);
        openFlags &= ~(O_RDWR|O_CREAT);
        flags |= SQLITE_OPEN_READONLY;
        openFlags |= O_RDONLY;
        isReadonly = 1;
        pReadonly = ڈھونڈو_Reusable_Fd(zName, flags);
        اگر( pReadonly ){
          fd = pReadonly->fd;
          sqlite3_free(pReadonly);
        }ورنہ{
          fd = robust_open(zName, openFlags, open_منوال);
        }
      }
    }
    اگر( fd<0 ){
      صحیح rc2 = unixLogError(SQLITE_CANTOPEN_BKPT, "open", zName);
      اگر( rc==SQLITE_OK ) rc = rc2;
      جاؤ open_ختم_شدہ;
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
    اگر( open_منوال && (flags & (SQLITE_OPEN_WAL|SQLITE_OPEN_MAIN_JOURNAL))!=0 ){
      robustFchown(fd, uid, gid);
    }
  }
  أكد( fd>=0 );
  اگر( p_برآمد_Flags ){
    *p_برآمد_Flags = flags;
  }

  اگر( p->p_Preallocated_غیر_مستعمل ){
    p->p_Preallocated_غیر_مستعمل->fd = fd;
    p->p_Preallocated_غیر_مستعمل->flags =
                          flags & (SQLITE_OPEN_READONLY|SQLITE_OPEN_READWRITE);
  }

  اگر( isDelete ){
#اگر OS_VXWORKS
    z_راستہ = zName;
#وگرنہ_اگر defined(SQLITE_UNLINK_AFTER_CLOSE)
    z_راستہ = sqlite3_mprintf("%s", zName);
    اگر( z_راستہ==0 ){
      robust_close(p, fd, __LINE__);
      واپس SQLITE_NOMEM_BKPT;
    }
#ورنہ
    osUnlink(zName);
#اگر_ختم
  }
#اگر SQLITE_ENABLE_LOCKING_STYLE
  ورنہ{
    p->openFlags = openFlags;
  }
#اگر_ختم

#اگر defined(__APPLE__) || SQLITE_ENABLE_LOCKING_STYLE
  اگر( fstatfs(fd, &fsInfo) == -1 ){
    store_آخری_Errno(p, errno);
    robust_close(p, fd, __LINE__);
    واپس SQLITE_IOERR_ACCESS;
  }
  اگر (0 == لڑی_موازنہ("msdos", fsInfo.f_fstypename, 5)) {
    ((unixFile*)pFile)->fsFlags |= SQLITE_FSFLAGS_IS_MSDOS;
  }
  اگر (0 == لڑی_موازنہ("exfat", fsInfo.f_fstypename, 5)) {
    ((unixFile*)pFile)->fsFlags |= SQLITE_FSFLAGS_IS_MSDOS;
  }
#اگر_ختم

  /* Set up appropriate ctrlFlags */
  اگر( isDelete )                ctrlFlags |= UNIXFILE_DELETE;
  اگر( isReadonly )              ctrlFlags |= UNIXFILE_RDONLY;
  no_تالا = eType!=SQLITE_OPEN_MAIN_DB;
  اگر( no_تالا )                  ctrlFlags |= UNIXFILE_NOLOCK;
  اگر( isNewJrnl )               ctrlFlags |= UNIXFILE_DIRSYNC;
  اگر( flags & SQLITE_OPEN_URI ) ctrlFlags |= UNIXFILE_URI;

#اگر SQLITE_ENABLE_LOCKING_STYLE
#اگر SQLITE_PREFER_PROXY_LOCKING
  isAutoProxy = 1;
#اگر_ختم
  اگر( isAutoProxy && (z_راستہ!=تہی) && (!no_تالا) && pVfs->xOpen ){
    حرف *envforce = getenv("SQLITE_FORCE_PROXY_LOCKING");
    صحیح useProxy = 0;

    /* SQLITE_FORCE_PROXY_LOCKING==1 means force always use proxy, 0 means
    ** never use proxy, NULL means use proxy for non-local files only.  */
    اگر( envforce!=تہی ){
      useProxy = atoi(envforce)>0;
    }ورنہ{
      useProxy = !(fsInfo.f_flags&MNT_LOCAL);
    }
    اگر( useProxy ){
      rc = fill_اندر_Unix_File(pVfs, fd, pFile, z_راستہ, ctrlFlags);
      اگر( rc==SQLITE_OK ){
        rc = proxyTransformUnixFile((unixFile*)pFile, ":auto:");
        اگر( rc!=SQLITE_OK ){
          /* Use unixClose to clean up the resources added in fillInUnixFile
          ** and clear all the structure's references.  Specifically,
          ** pFile->pMethods will be NULL so sqlite3OsClose will be a no-op
          */
          unixClose(pFile);
          واپس rc;
        }
      }
      جاؤ open_ختم_شدہ;
    }
  }
#اگر_ختم

  أكد( z_راستہ==0
       || z_راستہ[0]=='/'
       || eType==SQLITE_OPEN_SUPER_JOURNAL
       || eType==SQLITE_OPEN_MAIN_JOURNAL
       || eType==SQLITE_OPEN_عارضی_JOURNAL
  );
  rc = fill_اندر_Unix_File(pVfs, fd, pFile, z_راستہ, ctrlFlags);

open_ختم_شدہ:
  اگر( rc!=SQLITE_OK ){
    sqlite3_free(p->p_Preallocated_غیر_مستعمل);
  }
  واپس rc;
}


/*
** Delete the file at zPath. If the dirSync argument is true, fsync()
** the directory after deleting the file.
*/
ساکن صحیح unixDelete(
  sqlite3_vfs *Not_مستعمل,     /* VFS containing this as the xDelete method */
  ثابت حرف *z_راستہ,        /* Name of file to be deleted */
  صحیح dirSync               /* If true, fsync() directory after deleting file */
){
  صحیح rc = SQLITE_OK;
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  SimulateIOError(واپس SQLITE_IOERR_DELETE);
  اگر( osUnlink(z_راستہ)==(-1) ){
    اگر( errno==ENOENT
#اگر OS_VXWORKS
        || osAccess(z_راستہ,0)!=0
#اگر_ختم
    ){
      rc = SQLITE_IOERR_DELETE_NOENT;
    }ورنہ{
      rc = unixLogError(SQLITE_IOERR_DELETE, "unlink", z_راستہ);
    }
    واپس rc;
  }
#اگر_غیر_تعریف_شدہ SQLITE_DISABLE_DIRSYNC
  اگر( (dirSync & 1)!=0 ){
    صحیح fd;
    rc = osOpenDirectory(z_راستہ, &fd);
    اگر( rc==SQLITE_OK ){
      اگر( بھرا_fsync(fd,0,0) ){
        rc = unixLogError(SQLITE_IOERR_DIR_FSYNC, "fsync", z_راستہ);
      }
      robust_close(0, fd, __LINE__);
    }ورنہ{
      أكد( rc==SQLITE_CANTOPEN );
      rc = SQLITE_OK;
    }
  }
#اگر_ختم
  واپس rc;
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
ساکن صحیح unixAccess(
  sqlite3_vfs *Not_مستعمل,   /* The VFS containing this xAccess method */
  ثابت حرف *z_راستہ,      /* Path of the file to examine */
  صحیح flags,              /* What do we want to learn about the zPath file? */
  صحیح *p_Res_برآمد            /* Write result boolean here */
){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  SimulateIOError( واپس SQLITE_IOERR_ACCESS; );
  أكد( p_Res_برآمد!=0 );

  /* The spec says there are three possible values for flags.  But only
  ** two of them are actually used */
  أكد( flags==SQLITE_ACCESS_EXISTS || flags==SQLITE_ACCESS_READWRITE );

  اگر( flags==SQLITE_ACCESS_EXISTS ){
    ساخت stat buf;
    *p_Res_برآمد = 0==osStat(z_راستہ, &buf) &&
                (!S_ISREG(buf.st_منوال) || buf.st_سائز>0);
  }ورنہ{
    *p_Res_برآمد = osAccess(z_راستہ, W_OK|R_OK)==0;
  }
  واپس SQLITE_OK;
}

/*
** A pathname under construction
*/
قسمتعریف ساخت Db_راستہ Db_راستہ;
ساخت Db_راستہ {
  صحیح rc;           /* Non-zero following any error */
  صحیح nSymlink;     /* Number of symlinks resolved */
  حرف *z_برآمد;       /* Write the pathname here */
  صحیح n_برآمد;         /* Bytes of space available to zOut[] */
  صحیح n_مستعمل;        /* Bytes of zOut[] currently being used */
};

/* Forward reference */
ساکن خالی append_All_راستہ_Elements(Db_راستہ*,ثابت حرف*);

/*
** Append a single path element to the DbPath under construction
*/
ساکن خالی append_One_راستہ_Element(
  Db_راستہ *p_راستہ,       /* Path under construction, to which to append zName */
  ثابت حرف *zName,   /* Name to append to pPath.  Not zero-terminated */
  صحیح nName            /* Number of significant bytes in zName */
){
  أكد( nName>0 );
  أكد( zName!=0 );
  اگر( zName[0]=='.' ){
    اگر( nName==1 ) واپس;
    اگر( zName[1]=='.' && nName==2 ){
      اگر( p_راستہ->n_مستعمل>1 ){
        أكد( p_راستہ->z_برآمد[0]=='/' );
        جب( p_راستہ->z_برآمد[--p_راستہ->n_مستعمل]!='/' ){}
      }
      واپس;
    }
  }
  اگر( p_راستہ->n_مستعمل + nName + 2 >= p_راستہ->n_برآمد ){
    p_راستہ->rc = SQLITE_ERROR;
    واپس;
  }
  p_راستہ->z_برآمد[p_راستہ->n_مستعمل++] = '/';
  حافظہ_نقل(&p_راستہ->z_برآمد[p_راستہ->n_مستعمل], zName, nName);
  p_راستہ->n_مستعمل += nName;
#اگر defined(HAVE_READLINK) && defined(HAVE_LSTAT)
  اگر( p_راستہ->rc==SQLITE_OK ){
    ثابت حرف *z_اندر;
    ساخت stat buf;
    p_راستہ->z_برآمد[p_راستہ->n_مستعمل] = 0;
    z_اندر = p_راستہ->z_برآمد;
    اگر( osLstat(z_اندر, &buf)!=0 ){
      اگر( errno!=ENOENT ){
        p_راستہ->rc = unixLogError(SQLITE_CANTOPEN_BKPT, "lstat", z_اندر);
      }
    }ورنہ اگر( S_ISLNK(buf.st_منوال) ){
      ssize_t got;
      حرف zLnk[SQLITE_MAX_PATHLEN+2];
      اگر( p_راستہ->nSymlink++ > SQLITE_MAX_SYMLINK ){
        p_راستہ->rc = SQLITE_CANTOPEN_BKPT;
        واپس;
      }
      got = osReadlink(z_اندر, zLnk, حجم(zLnk)-2);
      اگر( got<=0 || got>=(ssize_t)حجم(zLnk)-2 ){
        p_راستہ->rc = unixLogError(SQLITE_CANTOPEN_BKPT, "readlink", z_اندر);
        واپس;
      }
      zLnk[got] = 0;
      اگر( zLnk[0]=='/' ){
        p_راستہ->n_مستعمل = 0;
      }ورنہ{
        p_راستہ->n_مستعمل -= nName + 1;
      }
      append_All_راستہ_Elements(p_راستہ, zLnk);
    }
  }
#اگر_ختم
}

/*
** Append all path elements in zPath to the DbPath under construction.
*/
ساکن خالی append_All_راستہ_Elements(
  Db_راستہ *p_راستہ,       /* Path under construction, to which to append zName */
  ثابت حرف *z_راستہ    /* Path to append to pPath.  Is zero-terminated */
){
  صحیح i = 0;
  صحیح j = 0;
  کرو{
    جب( z_راستہ[i] && z_راستہ[i]!='/' ){ i++; }
    اگر( i>j ){
      append_One_راستہ_Element(p_راستہ, &z_راستہ[j], i-j);
    }
    j = i+1;
  }جب( z_راستہ[i++] );
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
ساکن صحیح unix_بھرا_Pathname(
  sqlite3_vfs *pVfs,            /* Pointer to vfs object */
  ثابت حرف *z_راستہ,            /* Possibly relative input path */
  صحیح n_برآمد,                     /* Size of output buffer in bytes */
  حرف *z_برآمد                    /* Output buffer */
){
  Db_راستہ راستہ;
  غیر_مستعمل_PARAMETER(pVfs);
  راستہ.rc = 0;
  راستہ.n_مستعمل = 0;
  راستہ.nSymlink = 0;
  راستہ.n_برآمد = n_برآمد;
  راستہ.z_برآمد = z_برآمد;
  اگر( z_راستہ[0]!='/' ){
    حرف zPwd[SQLITE_MAX_PATHLEN+2];
    اگر( osGetcwd(zPwd, حجم(zPwd)-2)==0 ){
      واپس unixLogError(SQLITE_CANTOPEN_BKPT, "getcwd", z_راستہ);
    }
    append_All_راستہ_Elements(&راستہ, zPwd);
  }
  append_All_راستہ_Elements(&راستہ, z_راستہ);
  z_برآمد[راستہ.n_مستعمل] = 0;
  اگر( راستہ.rc || راستہ.n_مستعمل<2 ) واپس SQLITE_CANTOPEN_BKPT;
  اگر( راستہ.nSymlink ) واپس SQLITE_OK_SYMLINK;
  واپس SQLITE_OK;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
/*
** Interfaces for opening a shared library, finding entry points
** within the shared library, and closing the shared library.
*/
#شامل <dlfcn.h>
ساکن خالی *unixDlOpen(sqlite3_vfs *Not_مستعمل, ثابت حرف *zFilename){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  واپس dlopen(zFilename, RTLD_NOW | RTLD_GLOBAL);
}

/*
** SQLite calls this function immediately after a call to unixDlSym() or
** unixDlOpen() fails (returns a null pointer). If a more detailed error
** message is available, it is written to zBufOut. If no error message
** is available, zBufOut is left unmodified and SQLite uses a default
** error message.
*/
ساکن خالی unixDlError(sqlite3_vfs *Not_مستعمل, صحیح nBuf, حرف *z_Buf_برآمد){
  ثابت حرف *zErr;
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  unixEnterMutex();
  zErr = dlerror();
  اگر( zErr ){
    sqlite3_snprintf(nBuf, z_Buf_برآمد, "%s", zErr);
  }
  unixLeaveMutex();
}
ساکن خالی (*unixDlSym(sqlite3_vfs *Not_مستعمل, خالی *p, ثابت حرف*zSym))(خالی){
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
  خالی (*(*x)(خالی*,ثابت حرف*))(خالی);
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  x = (خالی(*(*)(خالی*,ثابت حرف*))(خالی))dlsym;
  واپس (*x)(p, zSym);
}
ساکن خالی unixDlClose(sqlite3_vfs *Not_مستعمل, خالی *pHandle){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  dlclose(pHandle);
}
#ورنہ /* if SQLITE_OMIT_LOAD_EXTENSION is defined: */
  #تعریف_کرو unixDlOpen  0
  #تعریف_کرو unixDlError 0
  #تعریف_کرو unixDlSym   0
  #تعریف_کرو unixDlClose 0
#اگر_ختم

/*
** Write nBuf bytes of random data to the supplied buffer zBuf.
*/
ساکن صحیح unixRandomness(sqlite3_vfs *Not_مستعمل, صحیح nBuf, حرف *zBuf){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  أكد((size_t)nBuf>=(حجم(وقت_t)+حجم(صحیح)));

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
#اگر !defined(SQLITE_TEST) && !defined(SQLITE_OMIT_RANDOMNESS)
  {
    صحیح fd, got;
    fd = robust_open("/dev/urandom", O_RDONLY, 0);
    اگر( fd<0 ){
      وقت_t t;
      وقت(&t);
      حافظہ_نقل(zBuf, &t, حجم(t));
      حافظہ_نقل(&zBuf[حجم(t)], &randomnessPid, حجم(randomnessPid));
      أكد( حجم(t)+حجم(randomnessPid)<=(size_t)nBuf );
      nBuf = حجم(t) + حجم(randomnessPid);
    }ورنہ{
      کرو{ got = osRead(fd, zBuf, nBuf); }جب( got<0 && errno==EINTR );
      robust_close(0, fd, __LINE__);
    }
  }
#اگر_ختم
  واپس nBuf;
}


/*
** Sleep for a little while.  Return the amount of time slept.
** The argument is the number of microseconds we want to sleep.
** The return value is the number of microseconds of sleep actually
** requested from the underlying operating system, a number which
** might be greater than or equal to the argument, but not less
** than the argument.
*/
ساکن صحیح unixSleep(sqlite3_vfs *Not_مستعمل, صحیح microseconds){
#اگر !defined(HAVE_NANOSLEEP) || HAVE_NANOSLEEP+0
  ساخت timespec sp;
  sp.tv_sec = microseconds / 1000000;
  sp.tv_nsec = (microseconds % 1000000) * 1000;

  /* Almost all modern unix systems support nanosleep().  But if you are
  ** compiling for one of the rare exceptions, you can use
  ** -DHAVE_NANOSLEEP=0 (perhaps in conjunction with -DHAVE_USLEEP if
  ** usleep() is available) in order to bypass the use of nanosleep() */
  nanosleep(&sp, تہی);

  غیر_مستعمل_PARAMETER(Not_مستعمل);
  واپس microseconds;
#وگرنہ_اگر defined(HAVE_USLEEP) && HAVE_USLEEP
  اگر( microseconds>=1000000 ) sleep(microseconds/1000000);
  اگر( microseconds%1000000 ) usleep(microseconds%1000000);
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  واپس microseconds;
#ورنہ
  صحیح seconds = (microseconds+999999)/1000000;
  sleep(seconds);
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  واپس seconds*1000000;
#اگر_ختم
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
ساکن صحیح unix_Current_وقت_Int64(sqlite3_vfs *Not_مستعمل, sqlite3_int64 *piNow){
  ساکن ثابت sqlite3_int64 unixEpoch = 24405875*(sqlite3_int64)8640000;
  صحیح rc = SQLITE_OK;
#اگر defined(NO_GETTOD)
  وقت_t t;
  وقت(&t);
  *piNow = ((sqlite3_int64)t)*1000 + unixEpoch;
#وگرنہ_اگر OS_VXWORKS
  ساخت timespec sNow;
  clock_gettime(CLOCK_REALTIME, &sNow);
  *piNow = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_nsec/1000000;
#ورنہ
  ساخت timeval sNow;
  (خالی)gettimeofday(&sNow, 0);  /* Cannot fail given valid arguments */
  *piNow = unixEpoch + 1000*(sqlite3_int64)sNow.tv_sec + sNow.tv_usec/1000;
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_TEST
  اگر( sqlite3_current_وقت ){
    *piNow = 1000*(sqlite3_int64)sqlite3_current_وقت + unixEpoch;
  }
#اگر_ختم
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  واپس rc;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DEPRECATED
/*
** Find the current time (in Universal Coordinated Time).  Write the
** current time and date as a Julian Day number into *prNow and
** return 0.  Return 1 if the time and date cannot be found.
*/
ساکن صحیح unix_Current_وقت(sqlite3_vfs *Not_مستعمل, دوہرا *prNow){
  sqlite3_int64 i = 0;
  صحیح rc;
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  rc = unix_Current_وقت_Int64(0, &i);
  *prNow = i/86400000.0;
  واپس rc;
}
#ورنہ
# define unix_Current_وقت 0
#اگر_ختم

/*
** The xGetLastError() method is designed to return a better
** low-level error message when operating-system problems come up
** during SQLite operation.  Only the integer return code is currently
** used.
*/
ساکن صحیح unix_Get_آخری_Error(sqlite3_vfs *Not_مستعمل, صحیح NotUsed2, حرف *NotUsed3){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  غیر_مستعمل_PARAMETER(NotUsed2);
  غیر_مستعمل_PARAMETER(NotUsed3);
  واپس errno;
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
#اگر defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE

/*
** The proxyLockingContext has the path and file structures for the remote
** and local proxy files in it
*/
قسمتعریف ساخت proxyLockingContext proxyLockingContext;
ساخت proxyLockingContext {
  unixFile *conchFile;         /* Open conch file */
  حرف *conch_File_راستہ;         /* Name of the conch file */
  unixFile *تالا_Proxy;         /* Open proxy lock file */
  حرف *تالا_Proxy_راستہ;         /* Name of the proxy lock file */
  حرف *db_راستہ;                /* Name of the open file */
  صحیح conchHeld;               /* 1 if the conch is held, -1 if lockless */
  صحیح nFails;                  /* Number of conch taking failures */
  خالی *oldLockingContext;     /* Original lockingcontext to restore on close */
  sqlite3_io_methods ثابت *p_Old_طریقہ;     /* Original I/O methods for close */
};

/*
** The proxy lock file path for the database at dbPath is written into lPath,
** which must point to valid, writable memory large enough for a maxLen length
** file path.
*/
ساکن صحیح proxy_Get_تالا_راستہ(ثابت حرف *db_راستہ, حرف *l_راستہ, size_t maxLen){
  صحیح len;
  صحیح dbLen;
  صحیح i;

#اگر_تعریف_شدہ LOCKPROXYDIR
  len = strlcpy(l_راستہ, LOCKPROXYDIR, maxLen);
#ورنہ
# ifdef CS_DARWIN_USER_عارضی_DIR
  {
    اگر( !confstr(CS_DARWIN_USER_عارضی_DIR, l_راستہ, maxLen) ){
      OSTRACE(("GETLOCKPATH  failed %s errno=%d pid=%d\n",
               l_راستہ, errno, osGetpid(0)));
      واپس SQLITE_IOERR_تالا;
    }
    len = strlcat(l_راستہ, "sqliteplocks", maxLen);  
  }
# ورنہ
  len = strlcpy(l_راستہ, "/tmp/", maxLen);
# endif
#اگر_ختم

  اگر( l_راستہ[len-1]!='/' ){
    len = strlcat(l_راستہ, "/", maxLen);
  }

  /* transform the db path to a unique cache name */
  dbLen = (صحیح)لڑی_لمبائی(db_راستہ);
  برائے( i=0; i<dbLen && (i+len+7)<(صحیح)maxLen; i++){
    حرف c = db_راستہ[i];
    l_راستہ[i+len] = (c=='/')?'_':c;
  }
  l_راستہ[i+len]='\0';
  strlcat(l_راستہ, ":auto:", maxLen);
  OSTRACE(("GETLOCKPATH  proxy lock path=%s pid=%d\n", l_راستہ, osGetpid(0)));
  واپس SQLITE_OK;
}

/*
 ** Creates the lock file and any missing directories in lockPath
 */
ساکن صحیح proxy_Create_تالا_راستہ(ثابت حرف *تالا_راستہ){
  صحیح i, len;
  حرف buf[MAXPATHLEN];
  صحیح آغاز = 0;

  أكد(تالا_راستہ!=تہی);
  /* try to create all the intermediate directories */
  len = (صحیح)لڑی_لمبائی(تالا_راستہ);
  buf[0] = تالا_راستہ[0];
  برائے( i=1; i<len; i++ ){
    اگر( تالا_راستہ[i] == '/' && (i - آغاز > 0) ){
      /* only mkdir if leaf dir != "." or "/" or ".." */
      اگر( i-آغاز>2 || (i-آغاز==1 && buf[آغاز] != '.' && buf[آغاز] != '/')
         || (i-آغاز==2 && buf[آغاز] != '.' && buf[آغاز+1] != '.') ){
        buf[i]='\0';
        اگر( osMkdir(buf, SQLITE_DEFAULT_PROXYDIR_PERMISSIONS) ){
          صحیح err=errno;
          اگر( err!=EEXIST ) {
            OSTRACE(("CREATELOCKPATH  FAILED creating %s, "
                     "'%s' proxy lock path=%s pid=%d\n",
                     buf, strerror(err), تالا_راستہ, osGetpid(0)));
            واپس err;
          }
        }
      }
      آغاز=i+1;
    }
    buf[i] = تالا_راستہ[i];
  }
  OSTRACE(("CREATELOCKPATH  proxy lock path=%s pid=%d\n",تالا_راستہ,osGetpid(0)));
  واپس 0;
}

/*
** Create a new VFS file descriptor (stored in memory obtained from
** sqlite3_malloc) and open the file named "path" in the file descriptor.
**
** The caller is responsible not only for closing the file descriptor
** but also for freeing the memory associated with the file descriptor.
*/
ساکن صحیح proxyCreateUnixFile(
    ثابت حرف *راستہ,        /* path for the new unixFile */
    unixFile **ppFile,       /* unixFile created and returned by ref */
    صحیح islockfile           /* if non zero missing dirs will be created */
) {
  صحیح fd = -1;
  unixFile *pNew;
  صحیح rc = SQLITE_OK;
  صحیح openFlags = O_RDWR | O_CREAT | O_NOFOLLOW;
  sqlite3_vfs dummyVfs;
  صحیح terrno = 0;
  Unix_غیر_مستعمل_Fd *p_غیر_مستعمل = تہی;

  /* 1. first try to open/create the file
  ** 2. if that fails, and this is a lock file (not-conch), try creating
  ** the parent directories and then try again.
  ** 3. if that fails, try to open the file read-only
  ** otherwise return BUSY (if lock file) or CANTOPEN for the conch file
  */
  p_غیر_مستعمل = ڈھونڈو_Reusable_Fd(راستہ, openFlags);
  اگر( p_غیر_مستعمل ){
    fd = p_غیر_مستعمل->fd;
  }ورنہ{
    p_غیر_مستعمل = sqlite3_malloc64(حجم(*p_غیر_مستعمل));
    اگر( !p_غیر_مستعمل ){
      واپس SQLITE_NOMEM_BKPT;
    }
  }
  اگر( fd<0 ){
    fd = robust_open(راستہ, openFlags, 0);
    terrno = errno;
    اگر( fd<0 && errno==ENOENT && islockfile ){
      اگر( proxy_Create_تالا_راستہ(راستہ) == SQLITE_OK ){
        fd = robust_open(راستہ, openFlags, 0);
      }
    }
  }
  اگر( fd<0 ){
    openFlags = O_RDONLY | O_NOFOLLOW;
    fd = robust_open(راستہ, openFlags, 0);
    terrno = errno;
  }
  اگر( fd<0 ){
    اگر( islockfile ){
      واپس SQLITE_BUSY;
    }
    انتخاب (terrno) {
      حالت EACCES:
        واپس SQLITE_PERM;
      حالت EIO:
        واپس SQLITE_IOERR_تالا; /* even though it is the conch */
      طےشدہ:
        واپس SQLITE_CANTOPEN_BKPT;
    }
  }

  pNew = (unixFile *)sqlite3_malloc64(حجم(*pNew));
  اگر( pNew==تہی ){
    rc = SQLITE_NOMEM_BKPT;
    جاؤ انتہا_create_proxy;
  }
  memset(pNew, 0, حجم(unixFile));
  pNew->openFlags = openFlags;
  memset(&dummyVfs, 0, حجم(dummyVfs));
  dummyVfs.p_App_ڈیٹا = (خالی*)&autolockIoFinder;
  dummyVfs.zName = "dummy";
  p_غیر_مستعمل->fd = fd;
  p_غیر_مستعمل->flags = openFlags;
  pNew->p_Preallocated_غیر_مستعمل = p_غیر_مستعمل;

  rc = fill_اندر_Unix_File(&dummyVfs, fd, (sqlite3_file*)pNew, راستہ, 0);
  اگر( rc==SQLITE_OK ){
    *ppFile = pNew;
    واپس SQLITE_OK;
  }
انتہا_create_proxy:  
  robust_close(pNew, fd, __LINE__);
  sqlite3_free(pNew);
  sqlite3_free(p_غیر_مستعمل);
  واپس rc;
}

#اگر_تعریف_شدہ SQLITE_TEST
/* simulate multiple hosts by creating unique hostid file paths */
صحیح sqlite3_hostid_عدد = 0;
#اگر_ختم

#تعریف_کرو PROXY_HOSTIDLEN    16  /* conch file host id length */

#اگر HAVE_GETHOSTUUID
/* Not always defined in the headers as it ought to be */
خارجی صحیح gethostuuid(uuid_t id, ثابت ساخت timespec *wait);
#اگر_ختم

/* get the host ID via gethostuuid(), pHostID must point to PROXY_HOSTIDLEN
** bytes of writable memory.
*/
ساکن صحیح proxyGetHostID(غیرعلامتی حرف *pHostID, صحیح *pError){
  أكد(PROXY_HOSTIDLEN == حجم(uuid_t));
  memset(pHostID, 0, PROXY_HOSTIDLEN);
#اگر HAVE_GETHOSTUUID
  {
    ساخت timespec timeout = {1, 0}; /* 1 sec timeout */
    اگر( gethostuuid(pHostID, &timeout) ){
      صحیح err = errno;
      اگر( pError ){
        *pError = err;
      }
      واپس SQLITE_IOERR;
    }
  }
#ورنہ
  غیر_مستعمل_PARAMETER(pError);
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_TEST
  /* simulate multiple hosts by creating unique hostid file paths */
  اگر( sqlite3_hostid_عدد != 0){
    pHostID[0] = (حرف)(pHostID[0] + (حرف)(sqlite3_hostid_عدد & 0xFF));
  }
#اگر_ختم

  واپس SQLITE_OK;
}

/* The conch file contains the header, host id and lock file path
 */
#تعریف_کرو PROXY_CONCHVERSION 2   /* 1-byte header, 16-byte host id, path */
#تعریف_کرو PROXY_HEADERLEN    1   /* conch file header length */
#تعریف_کرو PROXY_PATHINDEX    (PROXY_HEADERLEN+PROXY_HOSTIDLEN)
#تعریف_کرو PROXY_MAXCONCHLEN  (PROXY_HEADERLEN+PROXY_HOSTIDLEN+MAXPATHLEN)

/*
** Takes an open conch file, copies the contents to a new path and then moves
** it back.  The newly created file's file descriptor is assigned to the
** conch file structure and finally the original conch file descriptor is
** closed.  Returns zero if successful.
*/
ساکن صحیح proxy_Break_Conch_تالا(unixFile *pFile, uuid_t myHostID){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
  unixFile *conchFile = pCtx->conchFile;
  حرف t_راستہ[MAXPATHLEN];
  حرف buf[PROXY_MAXCONCHLEN];
  حرف *c_راستہ = pCtx->conch_File_راستہ;
  size_t readLen = 0;
  size_t راستہ_Len = 0;
  حرف errmsg[64] = "";
  صحیح fd = -1;
  صحیح rc = -1;
  غیر_مستعمل_PARAMETER(myHostID);

  /* create a new path by replace the trailing '-conch' with '-break' */
  راستہ_Len = strlcpy(t_راستہ, c_راستہ, MAXPATHLEN);
  اگر( راستہ_Len>MAXPATHLEN || راستہ_Len<6 ||
     (strlcpy(&t_راستہ[راستہ_Len-5], "break", 6) != 5) ){
    sqlite3_snprintf(حجم(errmsg),errmsg,"path error (len %d)",(صحیح)راستہ_Len);
    جاؤ انتہا_breaklock;
  }
  /* read the conch content */
  readLen = osPread(conchFile->h, buf, PROXY_MAXCONCHLEN, 0);
  اگر( readLen<PROXY_PATHINDEX ){
    sqlite3_snprintf(حجم(errmsg),errmsg,"read error (len %d)",(صحیح)readLen);
    جاؤ انتہا_breaklock;
  }
  /* write it out to the temporary break file */
  fd = robust_open(t_راستہ, (O_RDWR|O_CREAT|O_EXCL|O_NOFOLLOW), 0);
  اگر( fd<0 ){
    sqlite3_snprintf(حجم(errmsg), errmsg, "create failed (%d)", errno);
    جاؤ انتہا_breaklock;
  }
  اگر( osPwrite(fd, buf, readLen, 0) != (ssize_t)readLen ){
    sqlite3_snprintf(حجم(errmsg), errmsg, "write failed (%d)", errno);
    جاؤ انتہا_breaklock;
  }
  اگر( rename(t_راستہ, c_راستہ) ){
    sqlite3_snprintf(حجم(errmsg), errmsg, "rename failed (%d)", errno);
    جاؤ انتہا_breaklock;
  }
  rc = 0;
  fprintf(stderr, "broke stale lock on %s\n", c_راستہ);
  robust_close(pFile, conchFile->h, __LINE__);
  conchFile->h = fd;
  conchFile->openFlags = O_RDWR | O_CREAT;

انتہا_breaklock:
  اگر( rc ){
    اگر( fd>=0 ){
      osUnlink(t_راستہ);
      robust_close(pFile, fd, __LINE__);
    }
    fprintf(stderr, "failed to break stale lock on %s, %s\n", c_راستہ, errmsg);
  }
  واپس rc;
}

/* Take the requested lock on the conch file and break a stale lock if the
** host id matches.
*/
ساکن صحیح proxy_Conch_تالا(unixFile *pFile, uuid_t myHostID, صحیح تالا_Type){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
  unixFile *conchFile = pCtx->conchFile;
  صحیح rc = SQLITE_OK;
  صحیح nTries = 0;
  ساخت timespec conch_Mod_وقت;

  memset(&conch_Mod_وقت, 0, حجم(conch_Mod_وقت));
  کرو {
    rc = conchFile->p_طریقہ->x_تالا((sqlite3_file*)conchFile, تالا_Type);
    nTries ++;
    اگر( rc==SQLITE_BUSY ){
      /* If the lock failed (busy):
       * 1st try: get the mod time of the conch, wait 0.5s and try again.
       * 2nd try: fail if the mod time changed or host id is different, wait
       *           10 sec and try again
       * 3rd try: break the lock unless the mod time has changed.
       */
      ساخت stat buf;
      اگر( osFstat(conchFile->h, &buf) ){
        store_آخری_Errno(pFile, errno);
        واپس SQLITE_IOERR_تالا;
      }
    
      اگر( nTries==1 ){
        conch_Mod_وقت = buf.st_mtimespec;
        unixSleep(0,500000); /* wait 0.5 sec and try the lock again*/
        جاری;
      }

      أكد( nTries>1 );
      اگر( conch_Mod_وقت.tv_sec != buf.st_mtimespec.tv_sec ||
         conch_Mod_وقت.tv_nsec != buf.st_mtimespec.tv_nsec ){
        واپس SQLITE_BUSY;
      }
    
      اگر( nTries==2 ){
        حرف tBuf[PROXY_MAXCONCHLEN];
        صحیح len = osPread(conchFile->h, tBuf, PROXY_MAXCONCHLEN, 0);
        اگر( len<0 ){
          store_آخری_Errno(pFile, errno);
          واپس SQLITE_IOERR_تالا;
        }
        اگر( len>PROXY_PATHINDEX && tBuf[0]==(حرف)PROXY_CONCHVERSION){
          /* don't break the lock if the host id doesn't match */
          اگر( 0!=memcmp(&tBuf[PROXY_HEADERLEN], myHostID, PROXY_HOSTIDLEN) ){
            واپس SQLITE_BUSY;
          }
        }ورنہ{
          /* don't break the lock on short read or a version mismatch */
          واپس SQLITE_BUSY;
        }
        unixSleep(0,10000000); /* wait 10 sec and try the lock again */
        جاری;
      }
    
      أكد( nTries==3 );
      اگر( 0==proxy_Break_Conch_تالا(pFile, myHostID) ){
        rc = SQLITE_OK;
        اگر( تالا_Type==EXCLUSIVE_تالا ){
          rc = conchFile->p_طریقہ->x_تالا((sqlite3_file*)conchFile, SHARED_تالا);
        }
        اگر( !rc ){
          rc = conchFile->p_طریقہ->x_تالا((sqlite3_file*)conchFile, تالا_Type);
        }
      }
    }
  } جب( rc==SQLITE_BUSY && nTries<3 );

  واپس rc;
}

/* Takes the conch by taking a shared lock and read the contents conch, if
** lockPath is non-NULL, the host ID and lock file path must match.  A NULL
** lockPath means that the lockPath in the conch file will be used if the
** host IDs match, or a new lock path will be generated automatically
** and written to the conch file.
*/
ساکن صحیح proxyTakeConch(unixFile *pFile){
  proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;

  اگر( pCtx->conchHeld!=0 ){
    واپس SQLITE_OK;
  }ورنہ{
    unixFile *conchFile = pCtx->conchFile;
    uuid_t myHostID;
    صحیح pError = 0;
    حرف readBuf[PROXY_MAXCONCHLEN];
    حرف تالا_راستہ[MAXPATHLEN];
    حرف *عارضی_تالا_راستہ = تہی;
    صحیح rc = SQLITE_OK;
    صحیح createConch = 0;
    صحیح host_Id_مطابقت = 0;
    صحیح readLen = 0;
    صحیح try_Old_تالا_راستہ = 0;
    صحیح force_New_تالا_راستہ = 0;
  
    OSTRACE(("TAKECONCH  %d for %s pid=%d\n", conchFile->h,
             (pCtx->تالا_Proxy_راستہ ? pCtx->تالا_Proxy_راستہ : ":auto:"),
             osGetpid(0)));

    rc = proxyGetHostID(myHostID, &pError);
    اگر( (rc&0xff)==SQLITE_IOERR ){
      store_آخری_Errno(pFile, pError);
      جاؤ انتہا_takeconch;
    }
    rc = proxy_Conch_تالا(pFile, myHostID, SHARED_تالا);
    اگر( rc!=SQLITE_OK ){
      جاؤ انتہا_takeconch;
    }
    /* read the existing conch file */
    readLen = seekAndRead((unixFile*)conchFile, 0, readBuf, PROXY_MAXCONCHLEN);
    اگر( readLen<0 ){
      /* I/O error: lastErrno set by seekAndRead */
      store_آخری_Errno(pFile, conchFile->آخری_Errno);
      rc = SQLITE_IOERR_READ;
      جاؤ انتہا_takeconch;
    }ورنہ اگر( readLen<=(PROXY_HEADERLEN+PROXY_HOSTIDLEN) ||
             readBuf[0]!=(حرف)PROXY_CONCHVERSION ){
      /* a short read or version format mismatch means we need to create a new
      ** conch file.
      */
      createConch = 1;
    }
    /* if the host id matches and the lock path already exists in the conch
    ** we'll try to use the path there, if we can't open that path, we'll
    ** retry with a new auto-generated path
    */
    کرو { /* in case we need to try again for an :auto: named lock file */

      اگر( !createConch && !force_New_تالا_راستہ ){
        host_Id_مطابقت = !memcmp(&readBuf[PROXY_HEADERLEN], myHostID,
                                  PROXY_HOSTIDLEN);
        /* if the conch has data compare the contents */
        اگر( !pCtx->تالا_Proxy_راستہ ){
          /* for auto-named local lock file, just check the host ID and we'll
           ** use the local lock file path that's already in there
           */
          اگر( host_Id_مطابقت ){
            size_t راستہ_Len = (readLen - PROXY_PATHINDEX);
          
            اگر( راستہ_Len>=MAXPATHLEN ){
              راستہ_Len=MAXPATHLEN-1;
            }
            حافظہ_نقل(تالا_راستہ, &readBuf[PROXY_PATHINDEX], راستہ_Len);
            تالا_راستہ[راستہ_Len] = 0;
            عارضی_تالا_راستہ = تالا_راستہ;
            try_Old_تالا_راستہ = 1;
            /* create a copy of the lock path if the conch is taken */
            جاؤ انتہا_takeconch;
          }
        }ورنہ اگر( host_Id_مطابقت
               && !لڑی_موازنہ(pCtx->تالا_Proxy_راستہ, &readBuf[PROXY_PATHINDEX],
                           readLen-PROXY_PATHINDEX)
        ){
          /* conch host and lock path match */
          جاؤ انتہا_takeconch;
        }
      }
    
      /* if the conch isn't writable and doesn't match, we can't take it */
      اگر( (conchFile->openFlags&O_RDWR) == 0 ){
        rc = SQLITE_BUSY;
        جاؤ انتہا_takeconch;
      }
    
      /* either the conch didn't match or we need to create a new one */
      اگر( !pCtx->تالا_Proxy_راستہ ){
        proxy_Get_تالا_راستہ(pCtx->db_راستہ, تالا_راستہ, MAXPATHLEN);
        عارضی_تالا_راستہ = تالا_راستہ;
        /* create a copy of the lock path _only_ if the conch is taken */
      }
    
      /* update conch with host and path (this will fail if other process
      ** has a shared lock already), if the host id matches, use the big
      ** stick.
      */
      futimes(conchFile->h, تہی);
      اگر( host_Id_مطابقت && !createConch ){
        اگر( conchFile->pInode && conchFile->pInode->nShared>1 ){
          /* We are trying for an exclusive lock but another thread in this
           ** same process is still holding a shared lock. */
          rc = SQLITE_BUSY;
        } ورنہ {        
          rc = proxy_Conch_تالا(pFile, myHostID, EXCLUSIVE_تالا);
        }
      }ورنہ{
        rc = proxy_Conch_تالا(pFile, myHostID, EXCLUSIVE_تالا);
      }
      اگر( rc==SQLITE_OK ){
        حرف write_بفر[PROXY_MAXCONCHLEN];
        صحیح write_سائز = 0;
      
        write_بفر[0] = (حرف)PROXY_CONCHVERSION;
        حافظہ_نقل(&write_بفر[PROXY_HEADERLEN], myHostID, PROXY_HOSTIDLEN);
        اگر( pCtx->تالا_Proxy_راستہ!=تہی ){
          strlcpy(&write_بفر[PROXY_PATHINDEX], pCtx->تالا_Proxy_راستہ,
                  MAXPATHLEN);
        }ورنہ{
          strlcpy(&write_بفر[PROXY_PATHINDEX], عارضی_تالا_راستہ, MAXPATHLEN);
        }
        write_سائز = PROXY_PATHINDEX + لڑی_لمبائی(&write_بفر[PROXY_PATHINDEX]);
        robust_ftruncate(conchFile->h, write_سائز);
        rc = unixWrite((sqlite3_file *)conchFile, write_بفر, write_سائز, 0);
        بھرا_fsync(conchFile->h,0,0);
        /* If we created a new conch file (not just updated the contents of a
         ** valid conch file), try to match the permissions of the database
         */
        اگر( rc==SQLITE_OK && createConch ){
          ساخت stat buf;
          صحیح err = osFstat(pFile->h, &buf);
          اگر( err==0 ){
            منوال_t cmode = buf.st_منوال&(S_IRUSR|S_IWUSR | S_IRGRP|S_IWGRP |
                                        S_IROTH|S_IWOTH);
            /* try to match the database file R/W permissions, ignore failure */
#اگر_غیر_تعریف_شدہ SQLITE_PROXY_DEBUG
            osFchmod(conchFile->h, cmode);
#ورنہ
            کرو{
              rc = osFchmod(conchFile->h, cmode);
            }جب( rc==(-1) && errno==EINTR );
            اگر( rc!=0 ){
              صحیح رمز = errno;
              fprintf(stderr, "fchmod %o FAILED with %d %s\n",
                      cmode, رمز, strerror(رمز));
            } ورنہ {
              fprintf(stderr, "fchmod %o SUCCEDED\n",cmode);
            }
          }ورنہ{
            صحیح رمز = errno;
            fprintf(stderr, "STAT FAILED[%d] with %d %s\n",
                    err, رمز, strerror(رمز));
#اگر_ختم
          }
        }
      }
      conchFile->p_طریقہ->xUnlock((sqlite3_file*)conchFile, SHARED_تالا);
    
    انتہا_takeconch:
      OSTRACE(("TRANSPROXY: CLOSE  %d\n", pFile->h));
      اگر( rc==SQLITE_OK && pFile->openFlags ){
        صحیح fd;
        اگر( pFile->h>=0 ){
          robust_close(pFile, pFile->h, __LINE__);
        }
        pFile->h = -1;
        fd = robust_open(pCtx->db_راستہ, pFile->openFlags, 0);
        OSTRACE(("TRANSPROXY: OPEN  %d\n", fd));
        اگر( fd>=0 ){
          pFile->h = fd;
        }ورنہ{
          rc=SQLITE_CANTOPEN_BKPT; /* SQLITE_BUSY? proxyTakeConch called
           during locking */
        }
      }
      اگر( rc==SQLITE_OK && !pCtx->تالا_Proxy ){
        حرف *راستہ = عارضی_تالا_راستہ ? عارضی_تالا_راستہ : pCtx->تالا_Proxy_راستہ;
        rc = proxyCreateUnixFile(راستہ, &pCtx->تالا_Proxy, 1);
        اگر( rc!=SQLITE_OK && rc!=SQLITE_NOMEM && try_Old_تالا_راستہ ){
          /* we couldn't create the proxy lock file with the old lock file path
           ** so try again via auto-naming
           */
          force_New_تالا_راستہ = 1;
          try_Old_تالا_راستہ = 0;
          جاری; /* go back to the do {} while start point, try again */
        }
      }
      اگر( rc==SQLITE_OK ){
        /* Need to make a copy of path if we extracted the value
         ** from the conch file or the path was allocated on the stack
         */
        اگر( عارضی_تالا_راستہ ){
          pCtx->تالا_Proxy_راستہ = sqlite3DbStrDup(0, عارضی_تالا_راستہ);
          اگر( !pCtx->تالا_Proxy_راستہ ){
            rc = SQLITE_NOMEM_BKPT;
          }
        }
      }
      اگر( rc==SQLITE_OK ){
        pCtx->conchHeld = 1;
      
        اگر( pCtx->تالا_Proxy->p_طریقہ == &afpIoMethods ){
          afpLockingContext *afpCtx;
          afpCtx = (afpLockingContext *)pCtx->تالا_Proxy->lockingContext;
          afpCtx->db_راستہ = pCtx->تالا_Proxy_راستہ;
        }
      } ورنہ {
        conchFile->p_طریقہ->xUnlock((sqlite3_file*)conchFile, NO_تالا);
      }
      OSTRACE(("TAKECONCH  %d %s\n", conchFile->h,
               rc==SQLITE_OK?"ok":"failed"));
      واپس rc;
    } جب (1); /* in case we need to retry the :auto: lock file -
                 ** we should never get here except via the 'continue' call. */
  }
}

/*
** If pFile holds a lock on a conch file, then release that lock.
*/
ساکن صحیح proxyReleaseConch(unixFile *pFile){
  صحیح rc = SQLITE_OK;         /* Subroutine return code */
  proxyLockingContext *pCtx;  /* The locking context for the proxy lock */
  unixFile *conchFile;        /* Name of the conch file */

  pCtx = (proxyLockingContext *)pFile->lockingContext;
  conchFile = pCtx->conchFile;
  OSTRACE(("RELEASECONCH  %d for %s pid=%d\n", conchFile->h,
           (pCtx->تالا_Proxy_راستہ ? pCtx->تالا_Proxy_راستہ : ":auto:"),
           osGetpid(0)));
  اگر( pCtx->conchHeld>0 ){
    rc = conchFile->p_طریقہ->xUnlock((sqlite3_file*)conchFile, NO_تالا);
  }
  pCtx->conchHeld = 0;
  OSTRACE(("RELEASECONCH  %d %s\n", conchFile->h,
           (rc==SQLITE_OK ? "ok" : "failed")));
  واپس rc;
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
ساکن صحیح proxyCreateConchPathname(حرف *db_راستہ, حرف **p_Conch_راستہ){
  صحیح i;                        /* Loop counter */
  صحیح len = (صحیح)لڑی_لمبائی(db_راستہ); /* Length of database filename - dbPath */
  حرف *conch_راستہ;              /* buffer in which to construct conch name */

  /* Allocate space for the conch filename and initialize the name to
  ** the name of the original database file. */
  *p_Conch_راستہ = conch_راستہ = (حرف *)sqlite3_malloc64(len + 8);
  اگر( conch_راستہ==0 ){
    واپس SQLITE_NOMEM_BKPT;
  }
  حافظہ_نقل(conch_راستہ, db_راستہ, len+1);

  /* now insert a "." before the last / character */
  برائے( i=(len-1); i>=0; i-- ){
    اگر( conch_راستہ[i]=='/' ){
      i++;
      توڑو;
    }
  }
  conch_راستہ[i]='.';
  جب ( i<len ){
    conch_راستہ[i+1]=db_راستہ[i];
    i++;
  }

  /* append the "-conch" suffix to the file */
  حافظہ_نقل(&conch_راستہ[i+1], "-conch", 7);
  أكد( (صحیح)لڑی_لمبائی(conch_راستہ) == len+7 );

  واپس SQLITE_OK;
}


/* Takes a fully configured proxy locking-style unix file and switches
** the local lock file path
*/
ساکن صحیح switch_تالا_Proxy_راستہ(unixFile *pFile, ثابت حرف *راستہ) {
  proxyLockingContext *pCtx = (proxyLockingContext*)pFile->lockingContext;
  حرف *old_راستہ = pCtx->تالا_Proxy_راستہ;
  صحیح rc = SQLITE_OK;

  اگر( pFile->e_File_تالا!=NO_تالا ){
    واپس SQLITE_BUSY;
  }

  /* nothing to do if the path is NULL, :auto: or matches the existing path */
  اگر( !راستہ || راستہ[0]=='\0' || !strcmp(راستہ, ":auto:") ||
    (old_راستہ && !لڑی_موازنہ(old_راستہ, راستہ, MAXPATHLEN)) ){
    واپس SQLITE_OK;
  }ورنہ{
    unixFile *تالا_Proxy = pCtx->تالا_Proxy;
    pCtx->تالا_Proxy=تہی;
    pCtx->conchHeld = 0;
    اگر( تالا_Proxy!=تہی ){
      rc=تالا_Proxy->p_طریقہ->xClose((sqlite3_file *)تالا_Proxy);
      اگر( rc ) واپس rc;
      sqlite3_free(تالا_Proxy);
    }
    sqlite3_free(old_راستہ);
    pCtx->تالا_Proxy_راستہ = sqlite3DbStrDup(0, راستہ);
  }

  واپس rc;
}

/*
** pFile is a file that has been opened by a prior xOpen call.  dbPath
** is a string buffer at least MAXPATHLEN+1 characters in size.
**
** This routine find the filename associated with pFile and writes it
** int dbPath.
*/
ساکن صحیح proxy_Get_Db_راستہ_For_Unix_File(unixFile *pFile, حرف *db_راستہ){
#اگر defined(__APPLE__)
  اگر( pFile->p_طریقہ == &afpIoMethods ){
    /* afp style keeps a reference to the db path in the filePath field
    ** of the struct */
    أكد( (صحیح)لڑی_لمبائی((حرف*)pFile->lockingContext)<=MAXPATHLEN );
    strlcpy(db_راستہ, ((afpLockingContext *)pFile->lockingContext)->db_راستہ,
            MAXPATHLEN);
  } ورنہ
#اگر_ختم
  اگر( pFile->p_طریقہ == &dotlockIoMethods ){
    /* dot lock style uses the locking context to store the dot lock
    ** file path */
    صحیح len = لڑی_لمبائی((حرف *)pFile->lockingContext) - لڑی_لمبائی(DOTLOCK_SUFFIX);
    حافظہ_نقل(db_راستہ, (حرف *)pFile->lockingContext, len + 1);
  }ورنہ{
    /* all other styles use the locking context to store the db file path */
    أكد( لڑی_لمبائی((حرف*)pFile->lockingContext)<=MAXPATHLEN );
    strlcpy(db_راستہ, (حرف *)pFile->lockingContext, MAXPATHLEN);
  }
  واپس SQLITE_OK;
}

/*
** Takes an already filled in unix file and alters it so all file locking
** will be performed on the local proxy lock file.  The following fields
** are preserved in the locking context so that they can be restored and
** the unix structure properly cleaned up at close time:
**  ->lockingContext
**  ->pMethod
*/
ساکن صحیح proxyTransformUnixFile(unixFile *pFile, ثابت حرف *راستہ) {
  proxyLockingContext *pCtx;
  حرف db_راستہ[MAXPATHLEN+1];       /* Name of the database file */
  حرف *تالا_راستہ=تہی;
  صحیح rc = SQLITE_OK;

  اگر( pFile->e_File_تالا!=NO_تالا ){
    واپس SQLITE_BUSY;
  }
  proxy_Get_Db_راستہ_For_Unix_File(pFile, db_راستہ);
  اگر( !راستہ || راستہ[0]=='\0' || !strcmp(راستہ, ":auto:") ){
    تالا_راستہ=تہی;
  }ورنہ{
    تالا_راستہ=(حرف *)راستہ;
  }

  OSTRACE(("TRANSPROXY  %d for %s pid=%d\n", pFile->h,
           (تالا_راستہ ? تالا_راستہ : ":auto:"), osGetpid(0)));

  pCtx = sqlite3_malloc64( حجم(*pCtx) );
  اگر( pCtx==0 ){
    واپس SQLITE_NOMEM_BKPT;
  }
  memset(pCtx, 0, حجم(*pCtx));

  rc = proxyCreateConchPathname(db_راستہ, &pCtx->conch_File_راستہ);
  اگر( rc==SQLITE_OK ){
    rc = proxyCreateUnixFile(pCtx->conch_File_راستہ, &pCtx->conchFile, 0);
    اگر( rc==SQLITE_CANTOPEN && ((pFile->openFlags&O_RDWR) == 0) ){
      /* if (a) the open flags are not O_RDWR, (b) the conch isn't there, and
      ** (c) the file system is read-only, then enable no-locking access.
      ** Ugh, since O_RDONLY==0x0000 we test for !O_RDWR since unixOpen asserts
      ** that openFlags will have only one of O_RDONLY or O_RDWR.
      */
      ساخت statfs fsInfo;
      ساخت stat conchInfo;
      صحیح goLockless = 0;

      اگر( osStat(pCtx->conch_File_راستہ, &conchInfo) == -1 ) {
        صحیح err = errno;
        اگر( (err==ENOENT) && (statfs(db_راستہ, &fsInfo) != -1) ){
          goLockless = (fsInfo.f_flags&MNT_RDONLY) == MNT_RDONLY;
        }
      }
      اگر( goLockless ){
        pCtx->conchHeld = -1; /* read only FS/ lockless */
        rc = SQLITE_OK;
      }
    }
  }
  اگر( rc==SQLITE_OK && تالا_راستہ ){
    pCtx->تالا_Proxy_راستہ = sqlite3DbStrDup(0, تالا_راستہ);
  }

  اگر( rc==SQLITE_OK ){
    pCtx->db_راستہ = sqlite3DbStrDup(0, db_راستہ);
    اگر( pCtx->db_راستہ==تہی ){
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  اگر( rc==SQLITE_OK ){
    /* all memory is allocated, proxys are created and assigned,
    ** switch the locking context and pMethod then return.
    */
    pCtx->oldLockingContext = pFile->lockingContext;
    pFile->lockingContext = pCtx;
    pCtx->p_Old_طریقہ = pFile->p_طریقہ;
    pFile->p_طریقہ = &proxyIoMethods;
  }ورنہ{
    اگر( pCtx->conchFile ){
      pCtx->conchFile->p_طریقہ->xClose((sqlite3_file *)pCtx->conchFile);
      sqlite3_free(pCtx->conchFile);
    }
    sqlite3DbFree(0, pCtx->تالا_Proxy_راستہ);
    sqlite3_free(pCtx->conch_File_راستہ);
    sqlite3_free(pCtx);
  }
  OSTRACE(("TRANSPROXY  %d %s\n", pFile->h,
           (rc==SQLITE_OK ? "ok" : "failed")));
  واپس rc;
}


/*
** This routine handles sqlite3_file_control() calls that are specific
** to proxy locking.
*/
ساکن صحیح proxyFileControl(sqlite3_file *id, صحیح op, خالی *pArg){
  انتخاب( op ){
    حالت SQLITE_FCNTL_GET_LOCKPROXYFILE: {
      unixFile *pFile = (unixFile*)id;
      اگر( pFile->p_طریقہ == &proxyIoMethods ){
        proxyLockingContext *pCtx = (proxyLockingContext*)pFile->lockingContext;
        proxyTakeConch(pFile);
        اگر( pCtx->تالا_Proxy_راستہ ){
          *(ثابت حرف **)pArg = pCtx->تالا_Proxy_راستہ;
        }ورنہ{
          *(ثابت حرف **)pArg = ":auto: (not held)";
        }
      } ورنہ {
        *(ثابت حرف **)pArg = تہی;
      }
      واپس SQLITE_OK;
    }
    حالت SQLITE_FCNTL_سیٹ_LOCKPROXYFILE: {
      unixFile *pFile = (unixFile*)id;
      صحیح rc = SQLITE_OK;
      صحیح isProxyStyle = (pFile->p_طریقہ == &proxyIoMethods);
      اگر( pArg==تہی || (ثابت حرف *)pArg==0 ){
        اگر( isProxyStyle ){
          /* turn off proxy locking - not supported.  If support is added for
          ** switching proxy locking mode off then it will need to fail if
          ** the journal mode is WAL mode.
          */
          rc = SQLITE_ERROR /*SQLITE_PROTOCOL? SQLITE_MISUSE?*/;
        }ورنہ{
          /* turn off proxy locking - already off - NOOP */
          rc = SQLITE_OK;
        }
      }ورنہ{
        ثابت حرف *proxy_راستہ = (ثابت حرف *)pArg;
        اگر( isProxyStyle ){
          proxyLockingContext *pCtx =
            (proxyLockingContext*)pFile->lockingContext;
          اگر( !strcmp(pArg, ":auto:")
           || (pCtx->تالا_Proxy_راستہ &&
               !لڑی_موازنہ(pCtx->تالا_Proxy_راستہ, proxy_راستہ, MAXPATHLEN))
          ){
            rc = SQLITE_OK;
          }ورنہ{
            rc = switch_تالا_Proxy_راستہ(pFile, proxy_راستہ);
          }
        }ورنہ{
          /* turn on proxy file locking */
          rc = proxyTransformUnixFile(pFile, proxy_راستہ);
        }
      }
      واپس rc;
    }
    طےشدہ: {
      أكد( 0 );  /* The call assures that only valid opcodes are sent */
    }
  }
  /*NOTREACHED*/ أكد(0);
  واپس SQLITE_ERROR;
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
ساکن صحیح proxy_Check_Reserved_تالا(sqlite3_file *id, صحیح *p_Res_برآمد) {
  unixFile *pFile = (unixFile*)id;
  صحیح rc = proxyTakeConch(pFile);
  اگر( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    اگر( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->تالا_Proxy;
      واپس proxy->p_طریقہ->x_Check_Reserved_تالا((sqlite3_file*)proxy, p_Res_برآمد);
    }ورنہ{ /* conchHeld < 0 is lockless */
      p_Res_برآمد=0;
    }
  }
  واپس rc;
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
ساکن صحیح proxy_تالا(sqlite3_file *id, صحیح e_File_تالا) {
  unixFile *pFile = (unixFile*)id;
  صحیح rc = proxyTakeConch(pFile);
  اگر( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    اگر( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->تالا_Proxy;
      rc = proxy->p_طریقہ->x_تالا((sqlite3_file*)proxy, e_File_تالا);
      pFile->e_File_تالا = proxy->e_File_تالا;
    }ورنہ{
      /* conchHeld < 0 is lockless */
    }
  }
  واپس rc;
}


/*
** Lower the locking level on file descriptor pFile to eFileLock.  eFileLock
** must be either NO_LOCK or SHARED_LOCK.
**
** If the locking level of the file descriptor is already at or below
** the requested locking level, this routine is a no-op.
*/
ساکن صحیح proxyUnlock(sqlite3_file *id, صحیح e_File_تالا) {
  unixFile *pFile = (unixFile*)id;
  صحیح rc = proxyTakeConch(pFile);
  اگر( rc==SQLITE_OK ){
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    اگر( pCtx->conchHeld>0 ){
      unixFile *proxy = pCtx->تالا_Proxy;
      rc = proxy->p_طریقہ->xUnlock((sqlite3_file*)proxy, e_File_تالا);
      pFile->e_File_تالا = proxy->e_File_تالا;
    }ورنہ{
      /* conchHeld < 0 is lockless */
    }
  }
  واپس rc;
}

/*
** Close a file that uses proxy locks.
*/
ساکن صحیح proxyClose(sqlite3_file *id) {
  اگر( ALWAYS(id) ){
    unixFile *pFile = (unixFile*)id;
    proxyLockingContext *pCtx = (proxyLockingContext *)pFile->lockingContext;
    unixFile *تالا_Proxy = pCtx->تالا_Proxy;
    unixFile *conchFile = pCtx->conchFile;
    صحیح rc = SQLITE_OK;

    اگر( تالا_Proxy ){
      rc = تالا_Proxy->p_طریقہ->xUnlock((sqlite3_file*)تالا_Proxy, NO_تالا);
      اگر( rc ) واپس rc;
      rc = تالا_Proxy->p_طریقہ->xClose((sqlite3_file*)تالا_Proxy);
      اگر( rc ) واپس rc;
      sqlite3_free(تالا_Proxy);
      pCtx->تالا_Proxy = 0;
    }
    اگر( conchFile ){
      اگر( pCtx->conchHeld ){
        rc = proxyReleaseConch(pFile);
        اگر( rc ) واپس rc;
      }
      rc = conchFile->p_طریقہ->xClose((sqlite3_file*)conchFile);
      اگر( rc ) واپس rc;
      sqlite3_free(conchFile);
    }
    sqlite3DbFree(0, pCtx->تالا_Proxy_راستہ);
    sqlite3_free(pCtx->conch_File_راستہ);
    sqlite3DbFree(0, pCtx->db_راستہ);
    /* restore the original locking context and pMethod then close it */
    pFile->lockingContext = pCtx->oldLockingContext;
    pFile->p_طریقہ = pCtx->p_Old_طریقہ;
    sqlite3_free(pCtx);
    واپس pFile->p_طریقہ->xClose(id);
  }
  واپس SQLITE_OK;
}



#اگر_ختم /* defined(__APPLE__) && SQLITE_ENABLE_LOCKING_STYLE */
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
صحیح sqlite3_os_init(خالی){
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
  #تعریف_کرو UNIXVFS(VFSNAME, FINDER) {                        \
    3,                    /* iVersion */                    \
    حجم(unixFile),     /* szOsFile */                    \
    MAX_PATHNAME,         /* mxPathname */                  \
    0,                    /* pNext */                       \
    VFSNAME,              /* zName */                       \
    (خالی*)&FINDER,       /* pAppData */                    \
    unixOpen,             /* xOpen */                       \
    unixDelete,           /* xDelete */                     \
    unixAccess,           /* xAccess */                     \
    unix_بھرا_Pathname,     /* xFullPathname */               \
    unixDlOpen,           /* xDlOpen */                     \
    unixDlError,          /* xDlError */                    \
    unixDlSym,            /* xDlSym */                      \
    unixDlClose,          /* xDlClose */                    \
    unixRandomness,       /* xRandomness */                 \
    unixSleep,            /* xSleep */                      \
    unix_Current_وقت,      /* xCurrentTime */                \
    unix_Get_آخری_Error,     /* xGetLastError */               \
    unix_Current_وقت_Int64, /* xCurrentTimeInt64 */           \
    unix_سیٹ_System_Call,    /* xSetSystemCall */              \
    unixGetSystemCall,    /* xGetSystemCall */              \
    unix_اگلا_System_Call,   /* xNextSystemCall */             \
  }

  /*
  ** All default VFSes for unix are contained in the following array.
  **
  ** Note that the sqlite3_vfs.pNext field of the VFS object is modified
  ** by the SQLite core when the VFS is registered.  So the following
  ** array cannot be const.
  */
  ساکن sqlite3_vfs aVfs[] = {
#اگر SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    UNIXVFS("unix",          autolockIoFinder ),
#وگرنہ_اگر OS_VXWORKS
    UNIXVFS("unix",          vxworksIoFinder ),
#ورنہ
    UNIXVFS("unix",          posixIoFinder ),
#اگر_ختم
    UNIXVFS("unix-none",     nolockIoFinder ),
    UNIXVFS("unix-dotfile",  dotlockIoFinder ),
    UNIXVFS("unix-excl",     posixIoFinder ),
#اگر OS_VXWORKS
    UNIXVFS("unix-namedsem", semIoFinder ),
#اگر_ختم
#اگر SQLITE_ENABLE_LOCKING_STYLE || OS_VXWORKS
    UNIXVFS("unix-posix",    posixIoFinder ),
#اگر_ختم
#اگر SQLITE_ENABLE_LOCKING_STYLE
    UNIXVFS("unix-flock",    flockIoFinder ),
#اگر_ختم
#اگر SQLITE_ENABLE_LOCKING_STYLE && defined(__APPLE__)
    UNIXVFS("unix-afp",      afpIoFinder ),
    UNIXVFS("unix-nfs",      nfsIoFinder ),
    UNIXVFS("unix-proxy",    proxyIoFinder ),
#اگر_ختم
  };
  غیرعلامتی صحیح i;          /* Loop counter */

  /* Double-check that the aSyscall[] array has been constructed
  ** correctly.  See ticket [bb3a86e890c8e96ab] */
  أكد( Array_سائز(aSyscall)==29 );

  /* Register all VFSes defined in the aVfs[] array */
  برائے(i=0; i<(حجم(aVfs)/حجم(sqlite3_vfs)); i++){
#اگر_تعریف_شدہ SQLITE_DEFAULT_UNIX_VFS
    sqlite3_vfs_رجسٹر(&aVfs[i],
           0==strcmp(aVfs[i].zName,SQLITE_DEFAULT_UNIX_VFS));
#ورنہ
    sqlite3_vfs_رجسٹر(&aVfs[i], i==0);
#اگر_ختم
  }
#اگر_تعریف_شدہ SQLITE_OS_KV_OPTIONAL
  sqlite3KvvfsInit();
#اگر_ختم
  unix_Big_تالا = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#اگر OS_VXWORKS
  vxworksMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS2);
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
  /* Validate lock assumptions */
  أكد( SQLITE_SHM_NLOCK==8 );  /* Number of available locks */
  أكد( UNIX_SHM_بنیاد==120  );  /* Start of locking area */
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
  أكد( UNIX_SHM_DMS==128   );  /* Byte offset of the deadman-switch */
#اگر_ختم

  واپس SQLITE_OK;
}

/*
** Shutdown the operating system interface.
**
** Some operating systems might need to do some cleanup in this routine,
** to release dynamically allocated objects.  But not on unix.
** This routine is a no-op for unix.
*/
صحیح sqlite3_os_انتہا(خالی){
  unix_Big_تالا = 0;
#اگر OS_VXWORKS
  vxworksMutex = 0;
#اگر_ختم
  واپس SQLITE_OK;
}

#اگر_ختم /* SQLITE_OS_UNIX */
