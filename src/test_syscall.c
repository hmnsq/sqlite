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

#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>

#ਜੇ SQLITE_OS_UNIX

/* From main.c */
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/mman.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

ਸਥਿਰ ਬਣਤਰ TestSyscallGlobal {
  ਪੂਰਨ bPersist;                   /* 1 for persistent errors, 0 for transient */
  ਪੂਰਨ n_ਗਿਣਤੀ;                     /* Fail after this many more calls */
  ਪੂਰਨ nFail;                      /* Number of failures that have occurred */
  ਪੂਰਨ pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

ਸਥਿਰ ਪੂਰਨ ts_open(ਅਚਲ ਅੱਖਰ *, ਪੂਰਨ, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ ts_close(ਪੂਰਨ fd);
ਸਥਿਰ ਪੂਰਨ ts_access(ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਪੂਰਨ ਬਹੁਲਕ);
ਸਥਿਰ ਅੱਖਰ *ts_getcwd(ਅੱਖਰ *z_ਰਸਤਾ, size_t n_ਰਸਤਾ);
ਸਥਿਰ ਪੂਰਨ ts_stat(ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਬਣਤਰ stat *p);
ਸਥਿਰ ਪੂਰਨ ts_fstat(ਪੂਰਨ fd, ਬਣਤਰ stat *p);
ਸਥਿਰ ਪੂਰਨ ts_ftruncate(ਪੂਰਨ fd, off_t n);
ਸਥਿਰ ਪੂਰਨ ts_fcntl(ਪੂਰਨ fd, ਪੂਰਨ cmd, ... );
ਸਥਿਰ ssize_t ts_read(ਪੂਰਨ fd, ਖਾਲੀ *aBuf, size_t nBuf);
ਸਥਿਰ ssize_t ts_pread(ਪੂਰਨ fd, ਖਾਲੀ *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
ਸਥਿਰ ssize_t ts_pread64(ਪੂਰਨ fd, ਖਾਲੀ *aBuf, size_t nBuf, sqlite3_uint64 off);
ਸਥਿਰ ssize_t ts_write(ਪੂਰਨ fd, ਅਚਲ ਖਾਲੀ *aBuf, size_t nBuf);
ਸਥਿਰ ssize_t ts_pwrite(ਪੂਰਨ fd, ਅਚਲ ਖਾਲੀ *aBuf, size_t nBuf, off_t off);
ਸਥਿਰ ssize_t ts_pwrite64(ਪੂਰਨ fd, ਅਚਲ ਖਾਲੀ *aBuf, size_t nBuf, sqlite3_uint64 off);
ਸਥਿਰ ਪੂਰਨ ts_fchmod(ਪੂਰਨ fd, ਬਹੁਲਕ_t ਬਹੁਲਕ);
ਸਥਿਰ ਪੂਰਨ ts_fallocate(ਪੂਰਨ fd, off_t off, off_t len);
ਸਥਿਰ ਖਾਲੀ *ts_mmap(ਖਾਲੀ *, size_t, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ, off_t);
ਸਥਿਰ ਖਾਲੀ *ts_mremap(ਖਾਲੀ*, size_t, size_t, ਪੂਰਨ, ...);

ਬਣਤਰ TestSyscallArray {
  ਅਚਲ ਅੱਖਰ *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  ਪੂਰਨ default_errno;              /* Default value for errno following errors */
  ਪੂਰਨ custom_errno;               /* Current value for errno if error */
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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_open      ((ਪੂਰਨ(*)(ਅਚਲ ਅੱਖਰ *, ਪੂਰਨ, ਪੂਰਨ))aSyscall[0].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_close     ((ਪੂਰਨ(*)(ਪੂਰਨ))aSyscall[1].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_access    ((ਪੂਰਨ(*)(ਅਚਲ ਅੱਖਰ*,ਪੂਰਨ))aSyscall[2].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_getcwd    ((ਅੱਖਰ*(*)(ਅੱਖਰ*,size_t))aSyscall[3].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_stat      ((ਪੂਰਨ(*)(ਅਚਲ ਅੱਖਰ*,ਬਣਤਰ stat*))aSyscall[4].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_fstat     ((ਪੂਰਨ(*)(ਪੂਰਨ,ਬਣਤਰ stat*))aSyscall[5].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_ftruncate ((ਪੂਰਨ(*)(ਪੂਰਨ,off_t))aSyscall[6].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_fcntl     ((ਪੂਰਨ(*)(ਪੂਰਨ,ਪੂਰਨ,...))aSyscall[7].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_read      ((ssize_t(*)(ਪੂਰਨ,ਖਾਲੀ*,size_t))aSyscall[8].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_pread     ((ssize_t(*)(ਪੂਰਨ,ਖਾਲੀ*,size_t,off_t))aSyscall[9].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_pread64   ((ssize_t(*)(ਪੂਰਨ,ਖਾਲੀ*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_write     ((ssize_t(*)(ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,size_t))aSyscall[11].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_pwrite    ((ssize_t(*)(ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,size_t,off_t))\
                       aSyscall[12].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_pwrite64  ((ssize_t(*)(ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_fchmod    ((ਪੂਰਨ(*)(ਪੂਰਨ,ਬਹੁਲਕ_t))aSyscall[14].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_fallocate ((ਪੂਰਨ(*)(ਪੂਰਨ,off_t,off_t))aSyscall[15].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_mmap      ((ਖਾਲੀ*(*)(ਖਾਲੀ*,size_t,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ,off_t))aSyscall[16].xOrig)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ orig_mremap    ((ਖਾਲੀ*(*)(ਖਾਲੀ*,size_t,size_t,ਪੂਰਨ,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
ਸਥਿਰ ਪੂਰਨ tsIsFail(ਖਾਲੀ){
  gSyscall.n_ਗਿਣਤੀ--;
  ਜੇ( gSyscall.n_ਗਿਣਤੀ==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
ਸਥਿਰ ਪੂਰਨ tsErrno(ਅਚਲ ਅੱਖਰ *zFunc){
  ਪੂਰਨ i;
  size_t nFunc = ਲੜੀ_ਲੰਬਾਈ(zFunc);
  ਲਈ(i=0; aSyscall[i].zName; i++){
    ਜੇ( ਲੜੀ_ਲੰਬਾਈ(aSyscall[i].zName)!=nFunc ) ਜਾਰੀ;
    ਜੇ( memcmp(aSyscall[i].zName, zFunc, nFunc) ) ਜਾਰੀ;
    ਵਾਪਸ aSyscall[i].custom_errno;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ(0);
  ਵਾਪਸ 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
ਸਥਿਰ ਪੂਰਨ tsIsFailErrno(ਅਚਲ ਅੱਖਰ *zFunc){
  ਜੇ( tsIsFail() ){
    errno = tsErrno(zFunc);
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}

/*
** A wrapper around open().
*/
ਸਥਿਰ ਪੂਰਨ ts_open(ਅਚਲ ਅੱਖਰ *zFile, ਪੂਰਨ flags, ਪੂਰਨ ਬਹੁਲਕ){
  ਜੇ( tsIsFailErrno("open") ){
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ orig_open(zFile, flags, ਬਹੁਲਕ);
}

/*
** A wrapper around close().
*/
ਸਥਿਰ ਪੂਰਨ ts_close(ਪੂਰਨ fd){
  ਜੇ( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ orig_close(fd);
}

/*
** A wrapper around access().
*/
ਸਥਿਰ ਪੂਰਨ ts_access(ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਪੂਰਨ ਬਹੁਲਕ){
  ਜੇ( tsIsFail() ){
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ orig_access(z_ਰਸਤਾ, ਬਹੁਲਕ);
}

/*
** A wrapper around getcwd().
*/
ਸਥਿਰ ਅੱਖਰ *ts_getcwd(ਅੱਖਰ *z_ਰਸਤਾ, size_t n_ਰਸਤਾ){
  ਜੇ( tsIsFail() ){
    ਵਾਪਸ ਨਿਰਾ;
  }
  ਵਾਪਸ orig_getcwd(z_ਰਸਤਾ, n_ਰਸਤਾ);
}

/*
** A wrapper around stat().
*/
ਸਥਿਰ ਪੂਰਨ ts_stat(ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਬਣਤਰ stat *p){
  ਜੇ( tsIsFail() ){
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ orig_stat(z_ਰਸਤਾ, p);
}

/*
** A wrapper around fstat().
*/
ਸਥਿਰ ਪੂਰਨ ts_fstat(ਪੂਰਨ fd, ਬਣਤਰ stat *p){
  ਜੇ( tsIsFailErrno("fstat") ){
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
ਸਥਿਰ ਪੂਰਨ ts_ftruncate(ਪੂਰਨ fd, off_t n){
  ਜੇ( tsIsFailErrno("ftruncate") ){
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
ਸਥਿਰ ਪੂਰਨ ts_fcntl(ਪੂਰਨ fd, ਪੂਰਨ cmd, ... ){
  va_list ap;
  ਖਾਲੀ *pArg;
  ਜੇ( tsIsFailErrno("fcntl") ){
    ਵਾਪਸ -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, ਖਾਲੀ *);
  ਵਾਪਸ orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
ਸਥਿਰ ssize_t ts_read(ਪੂਰਨ fd, ਖਾਲੀ *aBuf, size_t nBuf){
  ਜੇ( tsIsFailErrno("read") ){
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
ਸਥਿਰ ssize_t ts_pread(ਪੂਰਨ fd, ਖਾਲੀ *aBuf, size_t nBuf, off_t off){
  ਜੇ( tsIsFailErrno("pread") ){
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
ਸਥਿਰ ssize_t ts_pread64(ਪੂਰਨ fd, ਖਾਲੀ *aBuf, size_t nBuf, sqlite3_uint64 off){
  ਜੇ( tsIsFailErrno("pread64") ){
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
ਸਥਿਰ ssize_t ts_write(ਪੂਰਨ fd, ਅਚਲ ਖਾਲੀ *aBuf, size_t nBuf){
  ਜੇ( tsIsFailErrno("write") ){
    ਜੇ( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
ਸਥਿਰ ssize_t ts_pwrite(ਪੂਰਨ fd, ਅਚਲ ਖਾਲੀ *aBuf, size_t nBuf, off_t off){
  ਜੇ( tsIsFailErrno("pwrite") ){
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
ਸਥਿਰ ssize_t ts_pwrite64(ਪੂਰਨ fd, ਅਚਲ ਖਾਲੀ *aBuf, size_t nBuf, sqlite3_uint64 off){
  ਜੇ( tsIsFailErrno("pwrite64") ){
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
ਸਥਿਰ ਪੂਰਨ ts_fchmod(ਪੂਰਨ fd, ਬਹੁਲਕ_t ਬਹੁਲਕ){
  ਜੇ( tsIsFail() ){
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ orig_fchmod(fd, ਬਹੁਲਕ);
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
ਸਥਿਰ ਪੂਰਨ ts_fallocate(ਪੂਰਨ fd, off_t off, off_t len){
  ਜੇ( tsIsFail() ){
    ਵਾਪਸ tsErrno("fallocate");
  }
  ਵਾਪਸ orig_fallocate(fd, off, len);
}

ਸਥਿਰ ਖਾਲੀ *ts_mmap(
  ਖਾਲੀ *pAddr, 
  size_t nByte, 
  ਪੂਰਨ prot, 
  ਪੂਰਨ flags, 
  ਪੂਰਨ fd, 
  off_t iOff
){
  ਜੇ( tsIsFailErrno("mmap") ){
    ਵਾਪਸ MAP_FAILED;
  }
  ਵਾਪਸ orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

ਸਥਿਰ ਖਾਲੀ *ts_mremap(ਖਾਲੀ *a, size_t b, size_t c, ਪੂਰਨ d, ...){
  ਜੇ( tsIsFailErrno("mremap") ){
    ਵਾਪਸ MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MREMAP_FIXED
  ਪ੍ਰਤਿਜ੍ਞਾ( (d & MREMAP_FIXED)==0 );
#ਜੇ_ਅੰਤ
  ਵਾਪਸ orig_mremap(a, b, c, d);
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_syscall_install(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_ਮਾਪ nElem;
  ਪੂਰਨ i;
  Tcl_Obj **apElem;

  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "SYSCALL-LIST");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    ਵਾਪਸ TCL_ERROR;
  }
  pVfs = sqlite3_vfs_ਲੱਭੋ(0);

  ਲਈ(i=0; i<(ਪੂਰਨ)nElem; i++){
    ਪੂਰਨ iCall;
    ਪੂਰਨ rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj_Struct(interp, 
        apElem[i], aSyscall, ਆਕਾਰ(aSyscall[0]), "system-call", 0, &iCall
    );
    ਜੇ( rc ) ਵਾਪਸ rc;
    ਜੇ( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->x_ਸੈੱਟ_System_Call(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_syscall_uninstall(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  ਪੂਰਨ i;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ਲੱਭੋ(0);
  ਲਈ(i=0; aSyscall[i].zName; i++){
    ਜੇ( aSyscall[i].xOrig ){
      pVfs->x_ਸੈੱਟ_System_Call(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_syscall_reset(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  ਪੂਰਨ i;
  ਪੂਰਨ rc;

  ਜੇ( objc!=2 && objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ਲੱਭੋ(0);
  ਜੇ( objc==2 ){
    rc = pVfs->x_ਸੈੱਟ_System_Call(pVfs, 0, 0);
    ਲਈ(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }ਨਹੀਂਤਾਂ{
    Tcl_ਮਾਪ nFunc;
    ਅੱਖਰ *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->x_ਸੈੱਟ_System_Call(pVfs, Tcl_GetString(objv[2]), 0);
    ਲਈ(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      ਜੇ( ਲੜੀ_ਲੰਬਾਈ(aSyscall[i].zName)!=nFunc ) ਜਾਰੀ;
      ਜੇ( memcmp(aSyscall[i].zName, zFunc, nFunc) ) ਜਾਰੀ;
      aSyscall[i].xOrig = 0;
    }
  }
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ਵਾਪਸ TCL_ERROR;
  }

  Tcl_Reset_ਨਤੀਜਾ(interp);
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_syscall_exists(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ਲੱਭੋ(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewBooleanObj(x!=0));
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_syscall_fault(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ n_ਗਿਣਤੀ = 0;
  ਪੂਰਨ bPersist = 0;

  ਜੇ( objc!=2 && objc!=4 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?COUNT PERSIST?");
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( objc==4 ){
    ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &n_ਗਿਣਤੀ)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      ਵਾਪਸ TCL_ERROR;
    }
  }

  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_ਗਿਣਤੀ = n_ਗਿਣਤੀ;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_syscall_errno(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਪੂਰਨ iCall;
  ਪੂਰਨ iErrno;
  ਪੂਰਨ rc;

  ਬਣਤਰ Errno {
    ਅਚਲ ਅੱਖਰ *z;
    ਪੂਰਨ i;
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

  ਜੇ( objc!=4 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "SYSCALL ERRNO");
    ਵਾਪਸ TCL_ERROR;
  }

  rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj_Struct(interp, 
      objv[2], aSyscall, ਆਕਾਰ(aSyscall[0]), "system-call", 0, &iCall
  );
  ਜੇ( rc!=TCL_OK ) ਵਾਪਸ rc;
  rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj_Struct(interp, 
      objv[3], aErrno, ਆਕਾਰ(aErrno[0]), "errno", 0, &iErrno
  );
  ਜੇ( rc!=TCL_OK ) ਵਾਪਸ rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_syscall_list(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਅਚਲ ਅੱਖਰ *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ਲੱਭੋ(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_ਗਿਣਤੀ(pList);
  ਲਈ(zSys = pVfs->x_ਅਗਲਾ_System_Call(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->x_ਅਗਲਾ_System_Call(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pList);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pList);
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_syscall_defaultvfs(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ਲੱਭੋ(0);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj(pVfs->zName, -1));
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ ts_getpagesize(ਖਾਲੀ){
  ਵਾਪਸ gSyscall.pgsz;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_syscall_pagesize(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_ਲੱਭੋ(0);
  ਪੂਰਨ pgsz;
  ਜੇ( objc!=3 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "PGSZ");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( pgsz<0 ){
    ਜੇ( gSyscall.orig_getpagesize ){
      pVfs->x_ਸੈੱਟ_System_Call(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }ਨਹੀਂਤਾਂ{
    ਜੇ( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_Append_ਨਤੀਜਾ(interp, "pgsz out of range", ਨਿਰਾ);
      ਵਾਪਸ TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->x_ਸੈੱਟ_System_Call(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_syscall(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਬਣਤਰ SyscallCmd {
    ਅਚਲ ਅੱਖਰ *zName;
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
  ਪੂਰਨ iCmd;
  ਪੂਰਨ rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_ਲੱਭੋ(0);

  ਜੇ( objc<2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "SUB-COMMAND ...");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( pVfs->i_ਸੰਸਕਰਣ<3 || pVfs->x_ਸੈੱਟ_System_Call==0 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "VFS does not support xSetSystemCall", ਨਿਰਾ);
    rc = TCL_ERROR;
  }ਨਹੀਂਤਾਂ{
    rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj_Struct(interp, 
        objv[1], aCmd, ਆਕਾਰ(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  ਜੇ( rc!=TCL_OK ) ਵਾਪਸ rc;
  ਵਾਪਸ aCmd[iCmd].xCmd(client_ਅੰਕੜੇ, interp, objc, objv);
}

ਪੂਰਨ SqlitetestSyscall_Init(Tcl_Interp *interp){
  ਬਣਤਰ SyscallCmd {
    ਅਚਲ ਅੱਖਰ *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  ਪੂਰਨ i;

  ਲਈ(i=0; i<ਆਕਾਰ(aCmd)/ਆਕਾਰ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  ਵਾਪਸ TCL_OK;
}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ SqlitetestSyscall_Init(Tcl_Interp *interp){
  ਵਾਪਸ TCL_OK;
}
#ਜੇ_ਅੰਤ
