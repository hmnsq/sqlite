/*
** 2011 March 28
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** The code in this file implements a Tcl interface used to test error
** handling in the os_unix.c module. Wrapper functions that support fault
** injection are registered as the low-level OS functions using the 
** xSetSystemCall() method of the VFS. The Tcl interface is as follows:
**
**
**   test_syscall install LIST
**     Install wrapper functions for all system calls in argument LIST.
**     LIST must be a list consisting of zero or more of the following
**     literal values:
**
**         open        close      access   getcwd   stat      fstat    
**         ftruncate   fcntl      read     pread    pread64   write
**         pwrite      pwrite64   fchmod   fallocate mmap
**
**   test_syscall uninstall
**     Uninstall all wrapper functions.
**
**   test_syscall fault ?COUNT PERSIST?
**     If [test_syscall fault] is invoked without the two arguments, fault
**     injection is disabled. Otherwise, fault injection is configured to
**     cause a failure on the COUNT'th next call to a system call with a
**     wrapper function installed. A COUNT value of 1 means fail the next
**     system call. 
** 
**     Argument PERSIST is interpreted as a boolean. If true, the all
**     system calls following the initial failure also fail. Otherwise, only
**     the single transient failure is injected.
**
**   test_syscall errno CALL ERRNO
**     Set the value that the global "errno" is set to following a fault
**     in call CALL. Argument CALL must be one of the system call names
**     listed above (under [test_syscall install]). ERRNO is a symbolic
**     name (i.e. "EACCES"). Not all errno codes are supported. Add extra
**     to the aErrno table in function test_syscall_errno() below as 
**     required.
**
**   test_syscall reset ?SYSTEM-CALL?
**     With no argument, this is an alias for the [uninstall] command. However,
**     this command uses a VFS call of the form:
**
**       xSetSystemCall(pVfs, 0, 0);
**
**     To restore the default system calls. The [uninstall] command restores
**     each system call individually by calling (i.e.):
**
**       xSetSystemCall(pVfs, "open", 0);
**
**     With an argument, this command attempts to reset the system call named
**     by the parameter using the same method as [uninstall].
**
**   test_syscall exists SYSTEM-CALL
**     Return true if the named system call exists. Or false otherwise.
**
**   test_syscall list
**     Return a list of all system calls. The list is constructed using
**     the xNextSystemCall() VFS method.
**
**   test_syscall pagesize PGSZ
**     If PGSZ is a power of two greater than 256, install a wrapper around
**     OS function getpagesize() that reports the system page size as PGSZ.
**     Or, if PGSZ is less than zero, remove any wrapper already installed.
*/

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>

#ಯದಿ SQLITE_OS_UNIX

/* From main.c */
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);

#ಅನ್ತರ್ಭಾವ <sys/mman.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

ಸ್ಥಿರ ಸಂರಚನಾ TestSyscallGlobal {
  ಪೂರ್ಣಾಙ್ಕ bPersist;                   /* 1 for persistent errors, 0 for transient */
  ಪೂರ್ಣಾಙ್ಕ n_ಎಣಿಕೆ;                     /* Fail after this many more calls */
  ಪೂರ್ಣಾಙ್ಕ nFail;                      /* Number of failures that have occurred */
  ಪೂರ್ಣಾಙ್ಕ pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_open(ಧ್ರುವ ವರ್ಣ *, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_close(ಪೂರ್ಣಾಙ್ಕ fd);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_access(ಧ್ರುವ ವರ್ಣ *zPath, ಪೂರ್ಣಾಙ್ಕ mode);
ಸ್ಥಿರ ವರ್ಣ *ts_getcwd(ವರ್ಣ *zPath, size_t nPath);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_stat(ಧ್ರುವ ವರ್ಣ *zPath, ಸಂರಚನಾ stat *p);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_fstat(ಪೂರ್ಣಾಙ್ಕ fd, ಸಂರಚನಾ stat *p);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_ftruncate(ಪೂರ್ಣಾಙ್ಕ fd, off_t n);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_fcntl(ಪೂರ್ಣಾಙ್ಕ fd, ಪೂರ್ಣಾಙ್ಕ cmd, ... );
ಸ್ಥಿರ ssize_t ts_read(ಪೂರ್ಣಾಙ್ಕ fd, ಶೂನ್ಯ *aBuf, size_t nBuf);
ಸ್ಥಿರ ssize_t ts_pread(ಪೂರ್ಣಾಙ್ಕ fd, ಶೂನ್ಯ *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
ಸ್ಥಿರ ssize_t ts_pread64(ಪೂರ್ಣಾಙ್ಕ fd, ಶೂನ್ಯ *aBuf, size_t nBuf, sqlite3_uint64 off);
ಸ್ಥಿರ ssize_t ts_write(ಪೂರ್ಣಾಙ್ಕ fd, ಧ್ರುವ ಶೂನ್ಯ *aBuf, size_t nBuf);
ಸ್ಥಿರ ssize_t ts_pwrite(ಪೂರ್ಣಾಙ್ಕ fd, ಧ್ರುವ ಶೂನ್ಯ *aBuf, size_t nBuf, off_t off);
ಸ್ಥಿರ ssize_t ts_pwrite64(ಪೂರ್ಣಾಙ್ಕ fd, ಧ್ರುವ ಶೂನ್ಯ *aBuf, size_t nBuf, sqlite3_uint64 off);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_fchmod(ಪೂರ್ಣಾಙ್ಕ fd, mode_t mode);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_fallocate(ಪೂರ್ಣಾಙ್ಕ fd, off_t off, off_t len);
ಸ್ಥಿರ ಶೂನ್ಯ *ts_mmap(ಶೂನ್ಯ *, size_t, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ, off_t);
ಸ್ಥಿರ ಶೂನ್ಯ *ts_mremap(ಶೂನ್ಯ*, size_t, size_t, ಪೂರ್ಣಾಙ್ಕ, ...);

ಸಂರಚನಾ TestSyscallArray {
  ಧ್ರುವ ವರ್ಣ *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  ಪೂರ್ಣಾಙ್ಕ default_errno;              /* Default value for errno following errors */
  ಪೂರ್ಣಾಙ್ಕ custom_errno;               /* Current value for errno if error */
} aSyscall[] = {
  /*  0 */ { "open",      (sqlite3_syscall_ptr)ts_open,      0, EACCES, 0 },
  /*  1 */ { "close",     (sqlite3_syscall_ptr)ts_close,     0, 0, 0 },
  /*  2 */ { "access",    (sqlite3_syscall_ptr)ts_access,    0, 0, 0 },
  /*  3 */ { "getcwd",    (sqlite3_syscall_ptr)ts_getcwd,    0, 0, 0 },
  /*  4 */ { "stat",      (sqlite3_syscall_ptr)ts_stat,      0, 0, 0 },
  /*  5 */ { "fstat",     (sqlite3_syscall_ptr)ts_fstat,     0, 0, 0 },
  /*  6 */ { "ftruncate", (sqlite3_syscall_ptr)ts_ftruncate, 0, EIO, 0 },
  /*  7 */ { "fcntl",     (sqlite3_syscall_ptr)ts_fcntl,     0, EACCES, 0 },
  /*  8 */ { "read",      (sqlite3_syscall_ptr)ts_read,      0, 0, 0 },
  /*  9 */ { "pread",     (sqlite3_syscall_ptr)ts_pread,     0, 0, 0 },
  /* 10 */ { "pread64",   (sqlite3_syscall_ptr)ts_pread64,   0, 0, 0 },
  /* 11 */ { "write",     (sqlite3_syscall_ptr)ts_write,     0, 0, 0 },
  /* 12 */ { "pwrite",    (sqlite3_syscall_ptr)ts_pwrite,    0, 0, 0 },
  /* 13 */ { "pwrite64",  (sqlite3_syscall_ptr)ts_pwrite64,  0, 0, 0 },
  /* 14 */ { "fchmod",    (sqlite3_syscall_ptr)ts_fchmod,    0, 0, 0 },
  /* 15 */ { "fallocate", (sqlite3_syscall_ptr)ts_fallocate, 0, 0, 0 },
  /* 16 */ { "mmap",      (sqlite3_syscall_ptr)ts_mmap,      0, 0, 0 },
  /* 17 */ { "mremap",    (sqlite3_syscall_ptr)ts_mremap,    0, 0, 0 },
           { 0, 0, 0, 0, 0 }
};

#define orig_open      ((ಪೂರ್ಣಾಙ್ಕ(*)(ಧ್ರುವ ವರ್ಣ *, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ))aSyscall[0].xOrig)
#define orig_close     ((ಪೂರ್ಣಾಙ್ಕ(*)(ಪೂರ್ಣಾಙ್ಕ))aSyscall[1].xOrig)
#define orig_access    ((ಪೂರ್ಣಾಙ್ಕ(*)(ಧ್ರುವ ವರ್ಣ*,ಪೂರ್ಣಾಙ್ಕ))aSyscall[2].xOrig)
#define orig_getcwd    ((ವರ್ಣ*(*)(ವರ್ಣ*,size_t))aSyscall[3].xOrig)
#define orig_stat      ((ಪೂರ್ಣಾಙ್ಕ(*)(ಧ್ರುವ ವರ್ಣ*,ಸಂರಚನಾ stat*))aSyscall[4].xOrig)
#define orig_fstat     ((ಪೂರ್ಣಾಙ್ಕ(*)(ಪೂರ್ಣಾಙ್ಕ,ಸಂರಚನಾ stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((ಪೂರ್ಣಾಙ್ಕ(*)(ಪೂರ್ಣಾಙ್ಕ,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((ಪೂರ್ಣಾಙ್ಕ(*)(ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*,size_t))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*,size_t,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(ಪೂರ್ಣಾಙ್ಕ,ಶೂನ್ಯ*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ಶೂನ್ಯ*,size_t))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ಶೂನ್ಯ*,size_t,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(ಪೂರ್ಣಾಙ್ಕ,ಧ್ರುವ ಶೂನ್ಯ*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((ಪೂರ್ಣಾಙ್ಕ(*)(ಪೂರ್ಣಾಙ್ಕ,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((ಪೂರ್ಣಾಙ್ಕ(*)(ಪೂರ್ಣಾಙ್ಕ,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((ಶೂನ್ಯ*(*)(ಶೂನ್ಯ*,size_t,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((ಶೂನ್ಯ*(*)(ಶೂನ್ಯ*,size_t,size_t,ಪೂರ್ಣಾಙ್ಕ,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tsIsFail(ಶೂನ್ಯ){
  gSyscall.n_ಎಣಿಕೆ--;
  ಯದಿ( gSyscall.n_ಎಣಿಕೆ==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tsErrno(ಧ್ರುವ ವರ್ಣ *zFunc){
  ಪೂರ್ಣಾಙ್ಕ i;
  size_t nFunc = strlen(zFunc);
  ಪ್ರತಿ(i=0; aSyscall[i].zName; i++){
    ಯದಿ( strlen(aSyscall[i].zName)!=nFunc ) ಅನುವರ್ತ;
    ಯದಿ( memcmp(aSyscall[i].zName, zFunc, nFunc) ) ಅನುವರ್ತ;
    ಪ್ರತ್ಯಾಗಮ aSyscall[i].custom_errno;
  }

  ಪ್ರತಿಜ್ಞಾ(0);
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tsIsFailErrno(ಧ್ರುವ ವರ್ಣ *zFunc){
  ಯದಿ( tsIsFail() ){
    errno = tsErrno(zFunc);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** A wrapper around open().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_open(ಧ್ರುವ ವರ್ಣ *zFile, ಪೂರ್ಣಾಙ್ಕ flags, ಪೂರ್ಣಾಙ್ಕ mode){
  ಯದಿ( tsIsFailErrno("open") ){
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪ್ರತ್ಯಾಗಮ orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_close(ಪೂರ್ಣಾಙ್ಕ fd){
  ಯದಿ( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪ್ರತ್ಯಾಗಮ orig_close(fd);
}

/*
** A wrapper around access().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_access(ಧ್ರುವ ವರ್ಣ *zPath, ಪೂರ್ಣಾಙ್ಕ mode){
  ಯದಿ( tsIsFail() ){
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪ್ರತ್ಯಾಗಮ orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
ಸ್ಥಿರ ವರ್ಣ *ts_getcwd(ವರ್ಣ *zPath, size_t nPath){
  ಯದಿ( tsIsFail() ){
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  }
  ಪ್ರತ್ಯಾಗಮ orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_stat(ಧ್ರುವ ವರ್ಣ *zPath, ಸಂರಚನಾ stat *p){
  ಯದಿ( tsIsFail() ){
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪ್ರತ್ಯಾಗಮ orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_fstat(ಪೂರ್ಣಾಙ್ಕ fd, ಸಂರಚನಾ stat *p){
  ಯದಿ( tsIsFailErrno("fstat") ){
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪ್ರತ್ಯಾಗಮ orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_ftruncate(ಪೂರ್ಣಾಙ್ಕ fd, off_t n){
  ಯದಿ( tsIsFailErrno("ftruncate") ){
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪ್ರತ್ಯಾಗಮ orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_fcntl(ಪೂರ್ಣಾಙ್ಕ fd, ಪೂರ್ಣಾಙ್ಕ cmd, ... ){
  va_list ap;
  ಶೂನ್ಯ *pArg;
  ಯದಿ( tsIsFailErrno("fcntl") ){
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, ಶೂನ್ಯ *);
  ಪ್ರತ್ಯಾಗಮ orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
ಸ್ಥಿರ ssize_t ts_read(ಪೂರ್ಣಾಙ್ಕ fd, ಶೂನ್ಯ *aBuf, size_t nBuf){
  ಯದಿ( tsIsFailErrno("read") ){
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪ್ರತ್ಯಾಗಮ orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
ಸ್ಥಿರ ssize_t ts_pread(ಪೂರ್ಣಾಙ್ಕ fd, ಶೂನ್ಯ *aBuf, size_t nBuf, off_t off){
  ಯದಿ( tsIsFailErrno("pread") ){
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪ್ರತ್ಯಾಗಮ orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
ಸ್ಥಿರ ssize_t ts_pread64(ಪೂರ್ಣಾಙ್ಕ fd, ಶೂನ್ಯ *aBuf, size_t nBuf, sqlite3_uint64 off){
  ಯದಿ( tsIsFailErrno("pread64") ){
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪ್ರತ್ಯಾಗಮ orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
ಸ್ಥಿರ ssize_t ts_write(ಪೂರ್ಣಾಙ್ಕ fd, ಧ್ರುವ ಶೂನ್ಯ *aBuf, size_t nBuf){
  ಯದಿ( tsIsFailErrno("write") ){
    ಯದಿ( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪ್ರತ್ಯಾಗಮ orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
ಸ್ಥಿರ ssize_t ts_pwrite(ಪೂರ್ಣಾಙ್ಕ fd, ಧ್ರುವ ಶೂನ್ಯ *aBuf, size_t nBuf, off_t off){
  ಯದಿ( tsIsFailErrno("pwrite") ){
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪ್ರತ್ಯಾಗಮ orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
ಸ್ಥಿರ ssize_t ts_pwrite64(ಪೂರ್ಣಾಙ್ಕ fd, ಧ್ರುವ ಶೂನ್ಯ *aBuf, size_t nBuf, sqlite3_uint64 off){
  ಯದಿ( tsIsFailErrno("pwrite64") ){
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪ್ರತ್ಯಾಗಮ orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_fchmod(ಪೂರ್ಣಾಙ್ಕ fd, mode_t mode){
  ಯದಿ( tsIsFail() ){
    ಪ್ರತ್ಯಾಗಮ -1;
  }
  ಪ್ರತ್ಯಾಗಮ orig_fchmod(fd, mode);
}

/*
** A wrapper around fallocate().
**
** SQLite assumes that the fallocate() function is compatible with
** posix_fallocate(). According to the Linux man page (2009-09-30):
**
**   posix_fallocate() returns  zero on success, or an error number on
**   failure. Note that errno is not set.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_fallocate(ಪೂರ್ಣಾಙ್ಕ fd, off_t off, off_t len){
  ಯದಿ( tsIsFail() ){
    ಪ್ರತ್ಯಾಗಮ tsErrno("fallocate");
  }
  ಪ್ರತ್ಯಾಗಮ orig_fallocate(fd, off, len);
}

ಸ್ಥಿರ ಶೂನ್ಯ *ts_mmap(
  ಶೂನ್ಯ *pAddr, 
  size_t nByte, 
  ಪೂರ್ಣಾಙ್ಕ prot, 
  ಪೂರ್ಣಾಙ್ಕ flags, 
  ಪೂರ್ಣಾಙ್ಕ fd, 
  off_t iOff
){
  ಯದಿ( tsIsFailErrno("mmap") ){
    ಪ್ರತ್ಯಾಗಮ MAP_FAILED;
  }
  ಪ್ರತ್ಯಾಗಮ orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

ಸ್ಥಿರ ಶೂನ್ಯ *ts_mremap(ಶೂನ್ಯ *a, size_t b, size_t c, ಪೂರ್ಣಾಙ್ಕ d, ...){
  ಯದಿ( tsIsFailErrno("mremap") ){
    ಪ್ರತ್ಯಾಗಮ MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  ಪ್ರತಿಜ್ಞಾ( (d & MREMAP_FIXED)==0 );
#endif
  ಪ್ರತ್ಯಾಗಮ orig_mremap(a, b, c, d);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_syscall_install(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_Size nElem;
  ಪೂರ್ಣಾಙ್ಕ i;
  Tcl_Obj **apElem;

  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "SYSCALL-LIST");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  ಪ್ರತಿ(i=0; i<(ಪೂರ್ಣಾಙ್ಕ)nElem; i++){
    ಪೂರ್ಣಾಙ್ಕ iCall;
    ಪೂರ್ಣಾಙ್ಕ rc = Tcl_GetIndexFromObjStruct(interp, 
        apElem[i], aSyscall, ಪರಿಮಾಣ(aSyscall[0]), "system-call", 0, &iCall
    );
    ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc;
    ಯದಿ( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_syscall_uninstall(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  ಪೂರ್ಣಾಙ್ಕ i;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  ಪ್ರತಿ(i=0; aSyscall[i].zName; i++){
    ಯದಿ( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_syscall_reset(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  ಯದಿ( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    ಪ್ರತಿ(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }ಅನ್ಯಥಾ{
    Tcl_Size nFunc;
    ವರ್ಣ *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    ಪ್ರತಿ(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      ಯದಿ( strlen(aSyscall[i].zName)!=nFunc ) ಅನುವರ್ತ;
      ಯದಿ( memcmp(aSyscall[i].zName, zFunc, nFunc) ) ಅನುವರ್ತ;
      aSyscall[i].xOrig = 0;
    }
  }
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_syscall_exists(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(x!=0));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_syscall_fault(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ n_ಎಣಿಕೆ = 0;
  ಪೂರ್ಣಾಙ್ಕ bPersist = 0;

  ಯದಿ( objc!=2 && objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "?COUNT PERSIST?");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( objc==4 ){
    ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &n_ಎಣಿಕೆ)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_ಎಣಿಕೆ = n_ಎಣಿಕೆ;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_syscall_errno(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಙ್ಕ iCall;
  ಪೂರ್ಣಾಙ್ಕ iErrno;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಸಂರಚನಾ Errno {
    ಧ್ರುವ ವರ್ಣ *z;
    ಪೂರ್ಣಾಙ್ಕ i;
  } aErrno[] = {
    { "EACCES",    EACCES },
    { "EINTR",     EINTR },
    { "EIO",       EIO },
    { "EOVERFLOW", EOVERFLOW },
    { "ENOMEM",    ENOMEM },
    { "EAGAIN",    EAGAIN },
    { "ETIMEDOUT", ETIMEDOUT },
    { "EBUSY",     EBUSY },
    { "EPERM",     EPERM },
    { "EDEADLK",   EDEADLK },
    { "ENOLCK",    ENOLCK },
    { 0, 0 }
  };

  ಯದಿ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "SYSCALL ERRNO");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[2], aSyscall, ಪರಿಮಾಣ(aSyscall[0]), "system-call", 0, &iCall
  );
  ಯದಿ( rc!=TCL_OK ) ಪ್ರತ್ಯಾಗಮ rc;
  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[3], aErrno, ಪರಿಮಾಣ(aErrno[0]), "errno", 0, &iErrno
  );
  ಯದಿ( rc!=TCL_OK ) ಪ್ರತ್ಯಾಗಮ rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_syscall_list(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಧ್ರುವ ವರ್ಣ *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pList);
  ಪ್ರತಿ(zSys = pVfs->xNextSystemCall(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->xNextSystemCall(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_SetObjResult(interp, pList);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pList);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_syscall_defaultvfs(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(pVfs->zName, -1));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ts_getpagesize(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ gSyscall.pgsz;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_syscall_pagesize(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  ಪೂರ್ಣಾಙ್ಕ pgsz;
  ಯದಿ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "PGSZ");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( pgsz<0 ){
    ಯದಿ( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }ಅನ್ಯಥಾ{
    ಯದಿ( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_AppendResult(interp, "pgsz out of range", ರಿಕ್ತ);
      ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_syscall(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸಂರಚನಾ SyscallCmd {
    ಧ್ರುವ ವರ್ಣ *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "fault",      test_syscall_fault },
    { "install",    test_syscall_install },
    { "uninstall",  test_syscall_uninstall },
    { "reset",      test_syscall_reset },
    { "errno",      test_syscall_errno },
    { "exists",     test_syscall_exists },
    { "list",       test_syscall_list },
    { "defaultvfs", test_syscall_defaultvfs },
    { "pagesize",   test_syscall_pagesize },
    { 0, 0 }
  };
  ಪೂರ್ಣಾಙ್ಕ iCmd;
  ಪೂರ್ಣಾಙ್ಕ rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  ಯದಿ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "SUB-COMMAND ...");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_AppendResult(interp, "VFS does not support xSetSystemCall", ರಿಕ್ತ);
    rc = TCL_ERROR;
  }ಅನ್ಯಥಾ{
    rc = Tcl_GetIndexFromObjStruct(interp, 
        objv[1], aCmd, ಪರಿಮಾಣ(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  ಯದಿ( rc!=TCL_OK ) ಪ್ರತ್ಯಾಗಮ rc;
  ಪ್ರತ್ಯಾಗಮ aCmd[iCmd].xCmd(clientData, interp, objc, objv);
}

ಪೂರ್ಣಾಙ್ಕ SqlitetestSyscall_Init(Tcl_Interp *interp){
  ಸಂರಚನಾ SyscallCmd {
    ಧ್ರುವ ವರ್ಣ *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  ಪೂರ್ಣಾಙ್ಕ i;

  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aCmd)/ಪರಿಮಾಣ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ SqlitetestSyscall_Init(Tcl_Interp *interp){
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
#endif
