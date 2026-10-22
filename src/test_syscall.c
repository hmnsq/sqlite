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

#ಆದರೆ SQLITE_OS_UNIX

/* From main.c */
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);

#ಅನ್ತರ್ಭಾವ <sys/mman.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

ಸ್ಥಿರ ರಚನೆ TestSyscallGlobal {
  ಪೂರ್ಣಾಂಕ bPersist;                   /* 1 for persistent errors, 0 for transient */
  ಪೂರ್ಣಾಂಕ n_ಎಣಿಕೆ;                     /* Fail after this many more calls */
  ಪೂರ್ಣಾಂಕ nFail;                      /* Number of failures that have occurred */
  ಪೂರ್ಣಾಂಕ pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_open(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_close(ಪೂರ್ಣಾಂಕ fd);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_access(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ);
ಸ್ಥಿರ ಅಕ್ಷರ *ts_getcwd(ಅಕ್ಷರ *z_ಮಾರ್ಗ, size_t n_ಮಾರ್ಗ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_stat(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, ರಚನೆ stat *p);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_fstat(ಪೂರ್ಣಾಂಕ fd, ರಚನೆ stat *p);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_ftruncate(ಪೂರ್ಣಾಂಕ fd, off_t n);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_fcntl(ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ cmd, ... );
ಸ್ಥಿರ ssize_t ts_read(ಪೂರ್ಣಾಂಕ fd, ಶೂನ್ಯ *aBuf, size_t nBuf);
ಸ್ಥಿರ ssize_t ts_pread(ಪೂರ್ಣಾಂಕ fd, ಶೂನ್ಯ *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
ಸ್ಥಿರ ssize_t ts_pread64(ಪೂರ್ಣಾಂಕ fd, ಶೂನ್ಯ *aBuf, size_t nBuf, sqlite3_uint64 off);
ಸ್ಥಿರ ssize_t ts_write(ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *aBuf, size_t nBuf);
ಸ್ಥಿರ ssize_t ts_pwrite(ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *aBuf, size_t nBuf, off_t off);
ಸ್ಥಿರ ssize_t ts_pwrite64(ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *aBuf, size_t nBuf, sqlite3_uint64 off);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_fchmod(ಪೂರ್ಣಾಂಕ fd, ಬಹುಲಕ_t ಬಹುಲಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_fallocate(ಪೂರ್ಣಾಂಕ fd, off_t off, off_t len);
ಸ್ಥಿರ ಶೂನ್ಯ *ts_mmap(ಶೂನ್ಯ *, size_t, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, off_t);
ಸ್ಥಿರ ಶೂನ್ಯ *ts_mremap(ಶೂನ್ಯ*, size_t, size_t, ಪೂರ್ಣಾಂಕ, ...);

ರಚನೆ TestSyscallArray {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  ಪೂರ್ಣಾಂಕ default_errno;              /* Default value for errno following errors */
  ಪೂರ್ಣಾಂಕ custom_errno;               /* Current value for errno if error */
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

#ವ್ಯಾಖ್ಯಾನಿಸು orig_open      ((ಪೂರ್ಣಾಂಕ(*)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ))aSyscall[0].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_close     ((ಪೂರ್ಣಾಂಕ(*)(ಪೂರ್ಣಾಂಕ))aSyscall[1].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_access    ((ಪೂರ್ಣಾಂಕ(*)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ಪೂರ್ಣಾಂಕ))aSyscall[2].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_getcwd    ((ಅಕ್ಷರ*(*)(ಅಕ್ಷರ*,size_t))aSyscall[3].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_stat      ((ಪೂರ್ಣಾಂಕ(*)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,ರಚನೆ stat*))aSyscall[4].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_fstat     ((ಪೂರ್ಣಾಂಕ(*)(ಪೂರ್ಣಾಂಕ,ರಚನೆ stat*))aSyscall[5].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_ftruncate ((ಪೂರ್ಣಾಂಕ(*)(ಪೂರ್ಣಾಂಕ,off_t))aSyscall[6].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_fcntl     ((ಪೂರ್ಣಾಂಕ(*)(ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,...))aSyscall[7].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_read      ((ssize_t(*)(ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*,size_t))aSyscall[8].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_pread     ((ssize_t(*)(ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*,size_t,off_t))aSyscall[9].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_pread64   ((ssize_t(*)(ಪೂರ್ಣಾಂಕ,ಶೂನ್ಯ*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_write     ((ssize_t(*)(ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,size_t))aSyscall[11].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_pwrite    ((ssize_t(*)(ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,size_t,off_t))\
                       aSyscall[12].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_pwrite64  ((ssize_t(*)(ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_fchmod    ((ಪೂರ್ಣಾಂಕ(*)(ಪೂರ್ಣಾಂಕ,ಬಹುಲಕ_t))aSyscall[14].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_fallocate ((ಪೂರ್ಣಾಂಕ(*)(ಪೂರ್ಣಾಂಕ,off_t,off_t))aSyscall[15].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_mmap      ((ಶೂನ್ಯ*(*)(ಶೂನ್ಯ*,size_t,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,off_t))aSyscall[16].xOrig)
#ವ್ಯಾಖ್ಯಾನಿಸು orig_mremap    ((ಶೂನ್ಯ*(*)(ಶೂನ್ಯ*,size_t,size_t,ಪೂರ್ಣಾಂಕ,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tsIsFail(ಶೂನ್ಯ){
  gSyscall.n_ಎಣಿಕೆ--;
  ಆದರೆ( gSyscall.n_ಎಣಿಕೆ==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tsErrno(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFunc){
  ಪೂರ್ಣಾಂಕ i;
  size_t nFunc = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zFunc);
  ಗಾಗಿ(i=0; aSyscall[i].zName; i++){
    ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(aSyscall[i].zName)!=nFunc ) ಮುಂದುವರಿ;
    ಆದರೆ( memcmp(aSyscall[i].zName, zFunc, nFunc) ) ಮುಂದುವರಿ;
    ಹಿಂತಿರುಗಿ aSyscall[i].custom_errno;
  }

  ಪ್ರತಿಜ್ಞಾ(0);
  ಹಿಂತಿರುಗಿ 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tsIsFailErrno(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFunc){
  ಆದರೆ( tsIsFail() ){
    errno = tsErrno(zFunc);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** A wrapper around open().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_open(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile, ಪೂರ್ಣಾಂಕ flags, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ){
  ಆದರೆ( tsIsFailErrno("open") ){
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ orig_open(zFile, flags, ಬಹುಲಕ);
}

/*
** A wrapper around close().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_close(ಪೂರ್ಣಾಂಕ fd){
  ಆದರೆ( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ orig_close(fd);
}

/*
** A wrapper around access().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_access(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ ಬಹುಲಕ){
  ಆದರೆ( tsIsFail() ){
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ orig_access(z_ಮಾರ್ಗ, ಬಹುಲಕ);
}

/*
** A wrapper around getcwd().
*/
ಸ್ಥಿರ ಅಕ್ಷರ *ts_getcwd(ಅಕ್ಷರ *z_ಮಾರ್ಗ, size_t n_ಮಾರ್ಗ){
  ಆದರೆ( tsIsFail() ){
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }
  ಹಿಂತಿರುಗಿ orig_getcwd(z_ಮಾರ್ಗ, n_ಮಾರ್ಗ);
}

/*
** A wrapper around stat().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_stat(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, ರಚನೆ stat *p){
  ಆದರೆ( tsIsFail() ){
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ orig_stat(z_ಮಾರ್ಗ, p);
}

/*
** A wrapper around fstat().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_fstat(ಪೂರ್ಣಾಂಕ fd, ರಚನೆ stat *p){
  ಆದರೆ( tsIsFailErrno("fstat") ){
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_ftruncate(ಪೂರ್ಣಾಂಕ fd, off_t n){
  ಆದರೆ( tsIsFailErrno("ftruncate") ){
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_fcntl(ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ cmd, ... ){
  va_list ap;
  ಶೂನ್ಯ *pArg;
  ಆದರೆ( tsIsFailErrno("fcntl") ){
    ಹಿಂತಿರುಗಿ -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, ಶೂನ್ಯ *);
  ಹಿಂತಿರುಗಿ orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
ಸ್ಥಿರ ssize_t ts_read(ಪೂರ್ಣಾಂಕ fd, ಶೂನ್ಯ *aBuf, size_t nBuf){
  ಆದರೆ( tsIsFailErrno("read") ){
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
ಸ್ಥಿರ ssize_t ts_pread(ಪೂರ್ಣಾಂಕ fd, ಶೂನ್ಯ *aBuf, size_t nBuf, off_t off){
  ಆದರೆ( tsIsFailErrno("pread") ){
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
ಸ್ಥಿರ ssize_t ts_pread64(ಪೂರ್ಣಾಂಕ fd, ಶೂನ್ಯ *aBuf, size_t nBuf, sqlite3_uint64 off){
  ಆದರೆ( tsIsFailErrno("pread64") ){
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
ಸ್ಥಿರ ssize_t ts_write(ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *aBuf, size_t nBuf){
  ಆದರೆ( tsIsFailErrno("write") ){
    ಆದರೆ( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
ಸ್ಥಿರ ssize_t ts_pwrite(ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *aBuf, size_t nBuf, off_t off){
  ಆದರೆ( tsIsFailErrno("pwrite") ){
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
ಸ್ಥಿರ ssize_t ts_pwrite64(ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *aBuf, size_t nBuf, sqlite3_uint64 off){
  ಆದರೆ( tsIsFailErrno("pwrite64") ){
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_fchmod(ಪೂರ್ಣಾಂಕ fd, ಬಹುಲಕ_t ಬಹುಲಕ){
  ಆದರೆ( tsIsFail() ){
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ orig_fchmod(fd, ಬಹುಲಕ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_fallocate(ಪೂರ್ಣಾಂಕ fd, off_t off, off_t len){
  ಆದರೆ( tsIsFail() ){
    ಹಿಂತಿರುಗಿ tsErrno("fallocate");
  }
  ಹಿಂತಿರುಗಿ orig_fallocate(fd, off, len);
}

ಸ್ಥಿರ ಶೂನ್ಯ *ts_mmap(
  ಶೂನ್ಯ *pAddr, 
  size_t nByte, 
  ಪೂರ್ಣಾಂಕ prot, 
  ಪೂರ್ಣಾಂಕ flags, 
  ಪೂರ್ಣಾಂಕ fd, 
  off_t iOff
){
  ಆದರೆ( tsIsFailErrno("mmap") ){
    ಹಿಂತಿರುಗಿ MAP_FAILED;
  }
  ಹಿಂತಿರುಗಿ orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

ಸ್ಥಿರ ಶೂನ್ಯ *ts_mremap(ಶೂನ್ಯ *a, size_t b, size_t c, ಪೂರ್ಣಾಂಕ d, ...){
  ಆದರೆ( tsIsFailErrno("mremap") ){
    ಹಿಂತಿರುಗಿ MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MREMAP_FIXED
  ಪ್ರತಿಜ್ಞಾ( (d & MREMAP_FIXED)==0 );
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ orig_mremap(a, b, c, d);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_syscall_install(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_ಗಾತ್ರ nElem;
  ಪೂರ್ಣಾಂಕ i;
  Tcl_Obj **apElem;

  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "SYSCALL-LIST");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);

  ಗಾಗಿ(i=0; i<(ಪೂರ್ಣಾಂಕ)nElem; i++){
    ಪೂರ್ಣಾಂಕ iCall;
    ಪೂರ್ಣಾಂಕ rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj_Struct(interp, 
        apElem[i], aSyscall, ಗಾತ್ರ(aSyscall[0]), "system-call", 0, &iCall
    );
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    ಆದರೆ( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->x_ಗಣ_System_Call(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_syscall_uninstall(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  ಪೂರ್ಣಾಂಕ i;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
  ಗಾಗಿ(i=0; aSyscall[i].zName; i++){
    ಆದರೆ( aSyscall[i].xOrig ){
      pVfs->x_ಗಣ_System_Call(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_syscall_reset(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
  ಆದರೆ( objc==2 ){
    rc = pVfs->x_ಗಣ_System_Call(pVfs, 0, 0);
    ಗಾಗಿ(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    Tcl_ಗಾತ್ರ nFunc;
    ಅಕ್ಷರ *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->x_ಗಣ_System_Call(pVfs, Tcl_GetString(objv[2]), 0);
    ಗಾಗಿ(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(aSyscall[i].zName)!=nFunc ) ಮುಂದುವರಿ;
      ಆದರೆ( memcmp(aSyscall[i].zName, zFunc, nFunc) ) ಮುಂದುವರಿ;
      aSyscall[i].xOrig = 0;
    }
  }
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  Tcl_Reset_ಫಲಿತಾಂಶ(interp);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_syscall_exists(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewBooleanObj(x!=0));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_syscall_fault(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ n_ಎಣಿಕೆ = 0;
  ಪೂರ್ಣಾಂಕ bPersist = 0;

  ಆದರೆ( objc!=2 && objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?COUNT PERSIST?");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( objc==4 ){
    ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &n_ಎಣಿಕೆ)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
  }

  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_ಎಣಿಕೆ = n_ಎಣಿಕೆ;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_syscall_errno(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಪೂರ್ಣಾಂಕ iCall;
  ಪೂರ್ಣಾಂಕ iErrno;
  ಪೂರ್ಣಾಂಕ rc;

  ರಚನೆ Errno {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;
    ಪೂರ್ಣಾಂಕ i;
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

  ಆದರೆ( objc!=4 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "SYSCALL ERRNO");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj_Struct(interp, 
      objv[2], aSyscall, ಗಾತ್ರ(aSyscall[0]), "system-call", 0, &iCall
  );
  ಆದರೆ( rc!=TCL_OK ) ಹಿಂತಿರುಗಿ rc;
  rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj_Struct(interp, 
      objv[3], aErrno, ಗಾತ್ರ(aErrno[0]), "errno", 0, &iErrno
  );
  ಆದರೆ( rc!=TCL_OK ) ಹಿಂತಿರುಗಿ rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_syscall_list(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_ಎಣಿಕೆ(pList);
  ಗಾಗಿ(zSys = pVfs->x_ಮುಂದಿನ_System_Call(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->x_ಮುಂದಿನ_System_Call(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pList);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pList);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_syscall_defaultvfs(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj(pVfs->zName, -1));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ts_getpagesize(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ gSyscall.pgsz;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_syscall_pagesize(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
  ಪೂರ್ಣಾಂಕ pgsz;
  ಆದರೆ( objc!=3 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "PGSZ");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆದರೆ( pgsz<0 ){
    ಆದರೆ( gSyscall.orig_getpagesize ){
      pVfs->x_ಗಣ_System_Call(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_Append_ಫಲಿತಾಂಶ(interp, "pgsz out of range", ಖಾಲಿ);
      ಹಿಂತಿರುಗಿ TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->x_ಗಣ_System_Call(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_syscall(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ರಚನೆ SyscallCmd {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
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
  ಪೂರ್ಣಾಂಕ iCmd;
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);

  ಆದರೆ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "SUB-COMMAND ...");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( pVfs->i_ಆವೃತ್ತಿ<3 || pVfs->x_ಗಣ_System_Call==0 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "VFS does not support xSetSystemCall", ಖಾಲಿ);
    rc = TCL_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj_Struct(interp, 
        objv[1], aCmd, ಗಾತ್ರ(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  ಆದರೆ( rc!=TCL_OK ) ಹಿಂತಿರುಗಿ rc;
  ಹಿಂತಿರುಗಿ aCmd[iCmd].xCmd(client_ದತ್ತಾಂಶ, interp, objc, objv);
}

ಪೂರ್ಣಾಂಕ SqlitetestSyscall_Init(Tcl_Interp *interp){
  ರಚನೆ SyscallCmd {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  ಪೂರ್ಣಾಂಕ i;

  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aCmd)/ಗಾತ್ರ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  ಹಿಂತಿರುಗಿ TCL_OK;
}
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ SqlitetestSyscall_Init(Tcl_Interp *interp){
  ಹಿಂತಿರುಗಿ TCL_OK;
}
#ಆದರೆ_ಅಂತ್ಯ
