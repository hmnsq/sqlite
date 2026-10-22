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

#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "sqlite3.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>

#ଯଦି SQLITE_OS_UNIX

/* From main.c */
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3ErrName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

#ଅନ୍ତର୍ଭାଵ <sys/mman.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>

ସ୍ଥିର ସଂରଚନା TestSyscallGlobal {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bPersist;                   /* 1 for persistent errors, 0 for transient */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଗଣନା;                     /* Fail after this many more calls */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFail;                      /* Number of failures that have occurred */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_open(ଅଚଳ ଅକ୍ଷର *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_close(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_access(ଅଚଳ ଅକ୍ଷର *zPath, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode);
ସ୍ଥିର ଅକ୍ଷର *ts_getcwd(ଅକ୍ଷର *zPath, size_t nPath);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_stat(ଅଚଳ ଅକ୍ଷର *zPath, ସଂରଚନା stat *p);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_fstat(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ସଂରଚନା stat *p);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_ftruncate(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, off_t n);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_fcntl(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd, ... );
ସ୍ଥିର ssize_t ts_read(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଶୂନ୍ୟ *aBuf, size_t nBuf);
ସ୍ଥିର ssize_t ts_pread(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଶୂନ୍ୟ *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
ସ୍ଥିର ssize_t ts_pread64(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଶୂନ୍ୟ *aBuf, size_t nBuf, sqlite3_uint64 off);
ସ୍ଥିର ssize_t ts_write(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଅଚଳ ଶୂନ୍ୟ *aBuf, size_t nBuf);
ସ୍ଥିର ssize_t ts_pwrite(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଅଚଳ ଶୂନ୍ୟ *aBuf, size_t nBuf, off_t off);
ସ୍ଥିର ssize_t ts_pwrite64(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଅଚଳ ଶୂନ୍ୟ *aBuf, size_t nBuf, sqlite3_uint64 off);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_fchmod(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, mode_t mode);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_fallocate(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, off_t off, off_t len);
ସ୍ଥିର ଶୂନ୍ୟ *ts_mmap(ଶୂନ୍ୟ *, size_t, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, off_t);
ସ୍ଥିର ଶୂନ୍ୟ *ts_mremap(ଶୂନ୍ୟ*, size_t, size_t, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ...);

ସଂରଚନା TestSyscallArray {
  ଅଚଳ ଅକ୍ଷର *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା default_errno;              /* Default value for errno following errors */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା custom_errno;               /* Current value for errno if error */
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

#define orig_open      ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ଅଚଳ ଅକ୍ଷର *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା))aSyscall[0].xOrig)
#define orig_close     ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))aSyscall[1].xOrig)
#define orig_access    ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ଅଚଳ ଅକ୍ଷର*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା))aSyscall[2].xOrig)
#define orig_getcwd    ((ଅକ୍ଷର*(*)(ଅକ୍ଷର*,size_t))aSyscall[3].xOrig)
#define orig_stat      ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ଅଚଳ ଅକ୍ଷର*,ସଂରଚନା stat*))aSyscall[4].xOrig)
#define orig_fstat     ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ସଂରଚନା stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*,size_t))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*,size_t,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଶୂନ୍ୟ*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଶୂନ୍ୟ*,size_t))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଶୂନ୍ୟ*,size_t,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ଅଚଳ ଶୂନ୍ୟ*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା(*)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((ଶୂନ୍ୟ*(*)(ଶୂନ୍ୟ*,size_t,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((ଶୂନ୍ୟ*(*)(ଶୂନ୍ୟ*,size_t,size_t,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tsIsFail(ଶୂନ୍ୟ){
  gSyscall.n_ଗଣନା--;
  ଯଦି( gSyscall.n_ଗଣନା==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tsErrno(ଅଚଳ ଅକ୍ଷର *zFunc){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  size_t nFunc = strlen(zFunc);
  ପାଇଁ(i=0; aSyscall[i].zName; i++){
    ଯଦି( strlen(aSyscall[i].zName)!=nFunc ) ଜାରି;
    ଯଦି( memcmp(aSyscall[i].zName, zFunc, nFunc) ) ଜାରି;
    ଫେରନ୍ତୁ aSyscall[i].custom_errno;
  }

  ପ୍ରତିଜ୍ଞା(0);
  ଫେରନ୍ତୁ 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tsIsFailErrno(ଅଚଳ ଅକ୍ଷର *zFunc){
  ଯଦି( tsIsFail() ){
    errno = tsErrno(zFunc);
    ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
}

/*
** A wrapper around open().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_open(ଅଚଳ ଅକ୍ଷର *zFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode){
  ଯଦି( tsIsFailErrno("open") ){
    ଫେରନ୍ତୁ -1;
  }
  ଫେରନ୍ତୁ orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_close(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd){
  ଯଦି( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    ଫେରନ୍ତୁ -1;
  }
  ଫେରନ୍ତୁ orig_close(fd);
}

/*
** A wrapper around access().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_access(ଅଚଳ ଅକ୍ଷର *zPath, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode){
  ଯଦି( tsIsFail() ){
    ଫେରନ୍ତୁ -1;
  }
  ଫେରନ୍ତୁ orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
ସ୍ଥିର ଅକ୍ଷର *ts_getcwd(ଅକ୍ଷର *zPath, size_t nPath){
  ଯଦି( tsIsFail() ){
    ଫେରନ୍ତୁ ଖାଲି;
  }
  ଫେରନ୍ତୁ orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_stat(ଅଚଳ ଅକ୍ଷର *zPath, ସଂରଚନା stat *p){
  ଯଦି( tsIsFail() ){
    ଫେରନ୍ତୁ -1;
  }
  ଫେରନ୍ତୁ orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_fstat(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ସଂରଚନା stat *p){
  ଯଦି( tsIsFailErrno("fstat") ){
    ଫେରନ୍ତୁ -1;
  }
  ଫେରନ୍ତୁ orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_ftruncate(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, off_t n){
  ଯଦି( tsIsFailErrno("ftruncate") ){
    ଫେରନ୍ତୁ -1;
  }
  ଫେରନ୍ତୁ orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_fcntl(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd, ... ){
  va_list ap;
  ଶୂନ୍ୟ *pArg;
  ଯଦି( tsIsFailErrno("fcntl") ){
    ଫେରନ୍ତୁ -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, ଶୂନ୍ୟ *);
  ଫେରନ୍ତୁ orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
ସ୍ଥିର ssize_t ts_read(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଶୂନ୍ୟ *aBuf, size_t nBuf){
  ଯଦି( tsIsFailErrno("read") ){
    ଫେରନ୍ତୁ -1;
  }
  ଫେରନ୍ତୁ orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
ସ୍ଥିର ssize_t ts_pread(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଶୂନ୍ୟ *aBuf, size_t nBuf, off_t off){
  ଯଦି( tsIsFailErrno("pread") ){
    ଫେରନ୍ତୁ -1;
  }
  ଫେରନ୍ତୁ orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
ସ୍ଥିର ssize_t ts_pread64(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଶୂନ୍ୟ *aBuf, size_t nBuf, sqlite3_uint64 off){
  ଯଦି( tsIsFailErrno("pread64") ){
    ଫେରନ୍ତୁ -1;
  }
  ଫେରନ୍ତୁ orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
ସ୍ଥିର ssize_t ts_write(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଅଚଳ ଶୂନ୍ୟ *aBuf, size_t nBuf){
  ଯଦି( tsIsFailErrno("write") ){
    ଯଦି( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    ଫେରନ୍ତୁ -1;
  }
  ଫେରନ୍ତୁ orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
ସ୍ଥିର ssize_t ts_pwrite(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଅଚଳ ଶୂନ୍ୟ *aBuf, size_t nBuf, off_t off){
  ଯଦି( tsIsFailErrno("pwrite") ){
    ଫେରନ୍ତୁ -1;
  }
  ଫେରନ୍ତୁ orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
ସ୍ଥିର ssize_t ts_pwrite64(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଅଚଳ ଶୂନ୍ୟ *aBuf, size_t nBuf, sqlite3_uint64 off){
  ଯଦି( tsIsFailErrno("pwrite64") ){
    ଫେରନ୍ତୁ -1;
  }
  ଫେରନ୍ତୁ orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_fchmod(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, mode_t mode){
  ଯଦି( tsIsFail() ){
    ଫେରନ୍ତୁ -1;
  }
  ଫେରନ୍ତୁ orig_fchmod(fd, mode);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_fallocate(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, off_t off, off_t len){
  ଯଦି( tsIsFail() ){
    ଫେରନ୍ତୁ tsErrno("fallocate");
  }
  ଫେରନ୍ତୁ orig_fallocate(fd, off, len);
}

ସ୍ଥିର ଶୂନ୍ୟ *ts_mmap(
  ଶୂନ୍ୟ *pAddr, 
  size_t nByte, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା prot, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, 
  off_t iOff
){
  ଯଦି( tsIsFailErrno("mmap") ){
    ଫେରନ୍ତୁ MAP_FAILED;
  }
  ଫେରନ୍ତୁ orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

ସ୍ଥିର ଶୂନ୍ୟ *ts_mremap(ଶୂନ୍ୟ *a, size_t b, size_t c, ପୂର୍ଣ୍ଣସଂଖ୍ୟା d, ...){
  ଯଦି( tsIsFailErrno("mremap") ){
    ଫେରନ୍ତୁ MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  ପ୍ରତିଜ୍ଞା( (d & MREMAP_FIXED)==0 );
#endif
  ଫେରନ୍ତୁ orig_mremap(a, b, c, d);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_syscall_install(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_Size nElem;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  Tcl_Obj **apElem;

  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "SYSCALL-LIST");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  ପାଇଁ(i=0; i<(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nElem; i++){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCall;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = Tcl_GetIndexFromObjStruct(interp, 
        apElem[i], aSyscall, ଆକାର(aSyscall[0]), "system-call", 0, &iCall
    );
    ଯଦି( rc ) ଫେରନ୍ତୁ rc;
    ଯଦି( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_syscall_uninstall(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  ପାଇଁ(i=0; aSyscall[i].zName; i++){
    ଯଦି( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_syscall_reset(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( objc!=2 && objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  ଯଦି( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    ପାଇଁ(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }ଅନ୍ୟଥା{
    Tcl_Size nFunc;
    ଅକ୍ଷର *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    ପାଇଁ(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      ଯଦି( strlen(aSyscall[i].zName)!=nFunc ) ଜାରି;
      ଯଦି( memcmp(aSyscall[i].zName, zFunc, nFunc) ) ଜାରି;
      aSyscall[i].xOrig = 0;
    }
  }
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_syscall_exists(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(x!=0));
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_syscall_fault(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଗଣନା = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bPersist = 0;

  ଯଦି( objc!=2 && objc!=4 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?COUNT PERSIST?");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( objc==4 ){
    ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &n_ଗଣନା)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      ଫେରନ୍ତୁ TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_ଗଣନା = n_ଗଣନା;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_syscall_errno(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCall;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iErrno;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ସଂରଚନା Errno {
    ଅଚଳ ଅକ୍ଷର *z;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
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

  ଯଦି( objc!=4 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "SYSCALL ERRNO");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[2], aSyscall, ଆକାର(aSyscall[0]), "system-call", 0, &iCall
  );
  ଯଦି( rc!=TCL_OK ) ଫେରନ୍ତୁ rc;
  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[3], aErrno, ଆକାର(aErrno[0]), "errno", 0, &iErrno
  );
  ଯଦି( rc!=TCL_OK ) ଫେରନ୍ତୁ rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_syscall_list(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଅଚଳ ଅକ୍ଷର *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_ଗଣନା(pList);
  ପାଇଁ(zSys = pVfs->xNextSystemCall(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->xNextSystemCall(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_SetObjResult(interp, pList);
  Tcl_Decr_Ref_ଗଣନା(pList);
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_syscall_defaultvfs(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(pVfs->zName, -1));
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ts_getpagesize(ଶୂନ୍ୟ){
  ଫେରନ୍ତୁ gSyscall.pgsz;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_syscall_pagesize(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgsz;
  ଯଦି( objc!=3 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "PGSZ");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( pgsz<0 ){
    ଯଦି( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }ଅନ୍ୟଥା{
    ଯଦି( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_AppendResult(interp, "pgsz out of range", ଖାଲି);
      ଫେରନ୍ତୁ TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_syscall(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ସଂରଚନା SyscallCmd {
    ଅଚଳ ଅକ୍ଷର *zName;
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCmd;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  ଯଦି( objc<2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "SUB-COMMAND ...");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_AppendResult(interp, "VFS does not support xSetSystemCall", ଖାଲି);
    rc = TCL_ERROR;
  }ଅନ୍ୟଥା{
    rc = Tcl_GetIndexFromObjStruct(interp, 
        objv[1], aCmd, ଆକାର(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  ଯଦି( rc!=TCL_OK ) ଫେରନ୍ତୁ rc;
  ଫେରନ୍ତୁ aCmd[iCmd].xCmd(clientData, interp, objc, objv);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା SqlitetestSyscall_Init(Tcl_Interp *interp){
  ସଂରଚନା SyscallCmd {
    ଅଚଳ ଅକ୍ଷର *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ପାଇଁ(i=0; i<ଆକାର(aCmd)/ଆକାର(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  ଫେରନ୍ତୁ TCL_OK;
}
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା SqlitetestSyscall_Init(Tcl_Interp *interp){
  ଫେରନ୍ତୁ TCL_OK;
}
#endif
