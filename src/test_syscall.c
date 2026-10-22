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

#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "sqlite3.h"
#അന്തര്ഭാവ "tclsqlite.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>

#എങ്കിൽ SQLITE_OS_UNIX

/* From main.c */
ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);

#അന്തര്ഭാവ <sys/mman.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <errno.h>

സ്ഥിതം ഘടന TestSyscallGlobal {
  പൂർണ്ണസംഖ്യ bPersist;                   /* 1 for persistent errors, 0 for transient */
  പൂർണ്ണസംഖ്യ n_എണ്ണം;                     /* Fail after this many more calls */
  പൂർണ്ണസംഖ്യ nFail;                      /* Number of failures that have occurred */
  പൂർണ്ണസംഖ്യ pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

സ്ഥിതം പൂർണ്ണസംഖ്യ ts_open(സ്ഥിരം അക്ഷരം *, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_close(പൂർണ്ണസംഖ്യ fd);
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_access(സ്ഥിരം അക്ഷരം *zPath, പൂർണ്ണസംഖ്യ mode);
സ്ഥിതം അക്ഷരം *ts_getcwd(അക്ഷരം *zPath, size_t nPath);
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_stat(സ്ഥിരം അക്ഷരം *zPath, ഘടന stat *p);
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_fstat(പൂർണ്ണസംഖ്യ fd, ഘടന stat *p);
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_ftruncate(പൂർണ്ണസംഖ്യ fd, off_t n);
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_fcntl(പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ cmd, ... );
സ്ഥിതം ssize_t ts_read(പൂർണ്ണസംഖ്യ fd, ശൂന്യം *aBuf, size_t nBuf);
സ്ഥിതം ssize_t ts_pread(പൂർണ്ണസംഖ്യ fd, ശൂന്യം *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
സ്ഥിതം ssize_t ts_pread64(പൂർണ്ണസംഖ്യ fd, ശൂന്യം *aBuf, size_t nBuf, sqlite3_uint64 off);
സ്ഥിതം ssize_t ts_write(പൂർണ്ണസംഖ്യ fd, സ്ഥിരം ശൂന്യം *aBuf, size_t nBuf);
സ്ഥിതം ssize_t ts_pwrite(പൂർണ്ണസംഖ്യ fd, സ്ഥിരം ശൂന്യം *aBuf, size_t nBuf, off_t off);
സ്ഥിതം ssize_t ts_pwrite64(പൂർണ്ണസംഖ്യ fd, സ്ഥിരം ശൂന്യം *aBuf, size_t nBuf, sqlite3_uint64 off);
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_fchmod(പൂർണ്ണസംഖ്യ fd, mode_t mode);
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_fallocate(പൂർണ്ണസംഖ്യ fd, off_t off, off_t len);
സ്ഥിതം ശൂന്യം *ts_mmap(ശൂന്യം *, size_t, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ, off_t);
സ്ഥിതം ശൂന്യം *ts_mremap(ശൂന്യം*, size_t, size_t, പൂർണ്ണസംഖ്യ, ...);

ഘടന TestSyscallArray {
  സ്ഥിരം അക്ഷരം *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  പൂർണ്ണസംഖ്യ default_errno;              /* Default value for errno following errors */
  പൂർണ്ണസംഖ്യ custom_errno;               /* Current value for errno if error */
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

#define orig_open      ((പൂർണ്ണസംഖ്യ(*)(സ്ഥിരം അക്ഷരം *, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ))aSyscall[0].xOrig)
#define orig_close     ((പൂർണ്ണസംഖ്യ(*)(പൂർണ്ണസംഖ്യ))aSyscall[1].xOrig)
#define orig_access    ((പൂർണ്ണസംഖ്യ(*)(സ്ഥിരം അക്ഷരം*,പൂർണ്ണസംഖ്യ))aSyscall[2].xOrig)
#define orig_getcwd    ((അക്ഷരം*(*)(അക്ഷരം*,size_t))aSyscall[3].xOrig)
#define orig_stat      ((പൂർണ്ണസംഖ്യ(*)(സ്ഥിരം അക്ഷരം*,ഘടന stat*))aSyscall[4].xOrig)
#define orig_fstat     ((പൂർണ്ണസംഖ്യ(*)(പൂർണ്ണസംഖ്യ,ഘടന stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((പൂർണ്ണസംഖ്യ(*)(പൂർണ്ണസംഖ്യ,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((പൂർണ്ണസംഖ്യ(*)(പൂർണ്ണസംഖ്യ,പൂർണ്ണസംഖ്യ,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(പൂർണ്ണസംഖ്യ,ശൂന്യം*,size_t))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(പൂർണ്ണസംഖ്യ,ശൂന്യം*,size_t,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(പൂർണ്ണസംഖ്യ,ശൂന്യം*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(പൂർണ്ണസംഖ്യ,സ്ഥിരം ശൂന്യം*,size_t))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(പൂർണ്ണസംഖ്യ,സ്ഥിരം ശൂന്യം*,size_t,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(പൂർണ്ണസംഖ്യ,സ്ഥിരം ശൂന്യം*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((പൂർണ്ണസംഖ്യ(*)(പൂർണ്ണസംഖ്യ,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((പൂർണ്ണസംഖ്യ(*)(പൂർണ്ണസംഖ്യ,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((ശൂന്യം*(*)(ശൂന്യം*,size_t,പൂർണ്ണസംഖ്യ,പൂർണ്ണസംഖ്യ,പൂർണ്ണസംഖ്യ,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((ശൂന്യം*(*)(ശൂന്യം*,size_t,size_t,പൂർണ്ണസംഖ്യ,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tsIsFail(ശൂന്യം){
  gSyscall.n_എണ്ണം--;
  എങ്കിൽ( gSyscall.n_എണ്ണം==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    തിരികെ 1;
  }
  തിരികെ 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tsErrno(സ്ഥിരം അക്ഷരം *zFunc){
  പൂർണ്ണസംഖ്യ i;
  size_t nFunc = strlen(zFunc);
  വേണ്ടി(i=0; aSyscall[i].zName; i++){
    എങ്കിൽ( strlen(aSyscall[i].zName)!=nFunc ) തുടരുക;
    എങ്കിൽ( memcmp(aSyscall[i].zName, zFunc, nFunc) ) തുടരുക;
    തിരികെ aSyscall[i].custom_errno;
  }

  പ്രതിജ്ഞാ(0);
  തിരികെ 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tsIsFailErrno(സ്ഥിരം അക്ഷരം *zFunc){
  എങ്കിൽ( tsIsFail() ){
    errno = tsErrno(zFunc);
    തിരികെ 1;
  }
  തിരികെ 0;
}

/*
** A wrapper around open().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_open(സ്ഥിരം അക്ഷരം *zFile, പൂർണ്ണസംഖ്യ flags, പൂർണ്ണസംഖ്യ mode){
  എങ്കിൽ( tsIsFailErrno("open") ){
    തിരികെ -1;
  }
  തിരികെ orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_close(പൂർണ്ണസംഖ്യ fd){
  എങ്കിൽ( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    തിരികെ -1;
  }
  തിരികെ orig_close(fd);
}

/*
** A wrapper around access().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_access(സ്ഥിരം അക്ഷരം *zPath, പൂർണ്ണസംഖ്യ mode){
  എങ്കിൽ( tsIsFail() ){
    തിരികെ -1;
  }
  തിരികെ orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
സ്ഥിതം അക്ഷരം *ts_getcwd(അക്ഷരം *zPath, size_t nPath){
  എങ്കിൽ( tsIsFail() ){
    തിരികെ ശൂന്യ;
  }
  തിരികെ orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_stat(സ്ഥിരം അക്ഷരം *zPath, ഘടന stat *p){
  എങ്കിൽ( tsIsFail() ){
    തിരികെ -1;
  }
  തിരികെ orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_fstat(പൂർണ്ണസംഖ്യ fd, ഘടന stat *p){
  എങ്കിൽ( tsIsFailErrno("fstat") ){
    തിരികെ -1;
  }
  തിരികെ orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_ftruncate(പൂർണ്ണസംഖ്യ fd, off_t n){
  എങ്കിൽ( tsIsFailErrno("ftruncate") ){
    തിരികെ -1;
  }
  തിരികെ orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_fcntl(പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ cmd, ... ){
  va_list ap;
  ശൂന്യം *pArg;
  എങ്കിൽ( tsIsFailErrno("fcntl") ){
    തിരികെ -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, ശൂന്യം *);
  തിരികെ orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
സ്ഥിതം ssize_t ts_read(പൂർണ്ണസംഖ്യ fd, ശൂന്യം *aBuf, size_t nBuf){
  എങ്കിൽ( tsIsFailErrno("read") ){
    തിരികെ -1;
  }
  തിരികെ orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
സ്ഥിതം ssize_t ts_pread(പൂർണ്ണസംഖ്യ fd, ശൂന്യം *aBuf, size_t nBuf, off_t off){
  എങ്കിൽ( tsIsFailErrno("pread") ){
    തിരികെ -1;
  }
  തിരികെ orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
സ്ഥിതം ssize_t ts_pread64(പൂർണ്ണസംഖ്യ fd, ശൂന്യം *aBuf, size_t nBuf, sqlite3_uint64 off){
  എങ്കിൽ( tsIsFailErrno("pread64") ){
    തിരികെ -1;
  }
  തിരികെ orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
സ്ഥിതം ssize_t ts_write(പൂർണ്ണസംഖ്യ fd, സ്ഥിരം ശൂന്യം *aBuf, size_t nBuf){
  എങ്കിൽ( tsIsFailErrno("write") ){
    എങ്കിൽ( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    തിരികെ -1;
  }
  തിരികെ orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
സ്ഥിതം ssize_t ts_pwrite(പൂർണ്ണസംഖ്യ fd, സ്ഥിരം ശൂന്യം *aBuf, size_t nBuf, off_t off){
  എങ്കിൽ( tsIsFailErrno("pwrite") ){
    തിരികെ -1;
  }
  തിരികെ orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
സ്ഥിതം ssize_t ts_pwrite64(പൂർണ്ണസംഖ്യ fd, സ്ഥിരം ശൂന്യം *aBuf, size_t nBuf, sqlite3_uint64 off){
  എങ്കിൽ( tsIsFailErrno("pwrite64") ){
    തിരികെ -1;
  }
  തിരികെ orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_fchmod(പൂർണ്ണസംഖ്യ fd, mode_t mode){
  എങ്കിൽ( tsIsFail() ){
    തിരികെ -1;
  }
  തിരികെ orig_fchmod(fd, mode);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ ts_fallocate(പൂർണ്ണസംഖ്യ fd, off_t off, off_t len){
  എങ്കിൽ( tsIsFail() ){
    തിരികെ tsErrno("fallocate");
  }
  തിരികെ orig_fallocate(fd, off, len);
}

സ്ഥിതം ശൂന്യം *ts_mmap(
  ശൂന്യം *pAddr, 
  size_t nByte, 
  പൂർണ്ണസംഖ്യ prot, 
  പൂർണ്ണസംഖ്യ flags, 
  പൂർണ്ണസംഖ്യ fd, 
  off_t iOff
){
  എങ്കിൽ( tsIsFailErrno("mmap") ){
    തിരികെ MAP_FAILED;
  }
  തിരികെ orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

സ്ഥിതം ശൂന്യം *ts_mremap(ശൂന്യം *a, size_t b, size_t c, പൂർണ്ണസംഖ്യ d, ...){
  എങ്കിൽ( tsIsFailErrno("mremap") ){
    തിരികെ MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  പ്രതിജ്ഞാ( (d & MREMAP_FIXED)==0 );
#endif
  തിരികെ orig_mremap(a, b, c, d);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_syscall_install(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_Size nElem;
  പൂർണ്ണസംഖ്യ i;
  Tcl_Obj **apElem;

  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 2, objv, "SYSCALL-LIST");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    തിരികെ TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  വേണ്ടി(i=0; i<(പൂർണ്ണസംഖ്യ)nElem; i++){
    പൂർണ്ണസംഖ്യ iCall;
    പൂർണ്ണസംഖ്യ rc = Tcl_GetIndexFromObjStruct(interp, 
        apElem[i], aSyscall, വലുപ്പം(aSyscall[0]), "system-call", 0, &iCall
    );
    എങ്കിൽ( rc ) തിരികെ rc;
    എങ്കിൽ( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_syscall_uninstall(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  പൂർണ്ണസംഖ്യ i;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 2, objv, "");
    തിരികെ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  വേണ്ടി(i=0; aSyscall[i].zName; i++){
    എങ്കിൽ( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_syscall_reset(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=2 && objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 2, objv, "");
    തിരികെ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  എങ്കിൽ( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    വേണ്ടി(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }അല്ലെങ്കിൽ{
    Tcl_Size nFunc;
    അക്ഷരം *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    വേണ്ടി(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      എങ്കിൽ( strlen(aSyscall[i].zName)!=nFunc ) തുടരുക;
      എങ്കിൽ( memcmp(aSyscall[i].zName, zFunc, nFunc) ) തുടരുക;
      aSyscall[i].xOrig = 0;
    }
  }
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    തിരികെ TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_syscall_exists(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 2, objv, "");
    തിരികെ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(x!=0));
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_syscall_fault(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ n_എണ്ണം = 0;
  പൂർണ്ണസംഖ്യ bPersist = 0;

  എങ്കിൽ( objc!=2 && objc!=4 ){
    Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?COUNT PERSIST?");
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( objc==4 ){
    എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &n_എണ്ണം)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      തിരികെ TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_എണ്ണം = n_എണ്ണം;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_syscall_errno(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  പൂർണ്ണസംഖ്യ iCall;
  പൂർണ്ണസംഖ്യ iErrno;
  പൂർണ്ണസംഖ്യ rc;

  ഘടന Errno {
    സ്ഥിരം അക്ഷരം *z;
    പൂർണ്ണസംഖ്യ i;
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

  എങ്കിൽ( objc!=4 ){
    Tcl_Wrong_അക്കം_Args(interp, 2, objv, "SYSCALL ERRNO");
    തിരികെ TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[2], aSyscall, വലുപ്പം(aSyscall[0]), "system-call", 0, &iCall
  );
  എങ്കിൽ( rc!=TCL_OK ) തിരികെ rc;
  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[3], aErrno, വലുപ്പം(aErrno[0]), "errno", 0, &iErrno
  );
  എങ്കിൽ( rc!=TCL_OK ) തിരികെ rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_syscall_list(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  സ്ഥിരം അക്ഷരം *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 2, objv, "");
    തിരികെ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_എണ്ണം(pList);
  വേണ്ടി(zSys = pVfs->xNextSystemCall(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->xNextSystemCall(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_SetObjResult(interp, pList);
  Tcl_Decr_Ref_എണ്ണം(pList);
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_syscall_defaultvfs(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 2, objv, "");
    തിരികെ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(pVfs->zName, -1));
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ ts_getpagesize(ശൂന്യം){
  തിരികെ gSyscall.pgsz;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_syscall_pagesize(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  പൂർണ്ണസംഖ്യ pgsz;
  എങ്കിൽ( objc!=3 ){
    Tcl_Wrong_അക്കം_Args(interp, 2, objv, "PGSZ");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( pgsz<0 ){
    എങ്കിൽ( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }അല്ലെങ്കിൽ{
    എങ്കിൽ( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_AppendResult(interp, "pgsz out of range", ശൂന്യ);
      തിരികെ TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_syscall(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  ഘടന SyscallCmd {
    സ്ഥിരം അക്ഷരം *zName;
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
  പൂർണ്ണസംഖ്യ iCmd;
  പൂർണ്ണസംഖ്യ rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  എങ്കിൽ( objc<2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "SUB-COMMAND ...");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_AppendResult(interp, "VFS does not support xSetSystemCall", ശൂന്യ);
    rc = TCL_ERROR;
  }അല്ലെങ്കിൽ{
    rc = Tcl_GetIndexFromObjStruct(interp, 
        objv[1], aCmd, വലുപ്പം(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  എങ്കിൽ( rc!=TCL_OK ) തിരികെ rc;
  തിരികെ aCmd[iCmd].xCmd(clientData, interp, objc, objv);
}

പൂർണ്ണസംഖ്യ SqlitetestSyscall_Init(Tcl_Interp *interp){
  ഘടന SyscallCmd {
    സ്ഥിരം അക്ഷരം *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  പൂർണ്ണസംഖ്യ i;

  വേണ്ടി(i=0; i<വലുപ്പം(aCmd)/വലുപ്പം(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  തിരികെ TCL_OK;
}
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ SqlitetestSyscall_Init(Tcl_Interp *interp){
  തിരികെ TCL_OK;
}
#endif
