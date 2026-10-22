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

#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>

#ఒకవేళ SQLITE_OS_UNIX

/* From main.c */
బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);

#అన్తర్భావ <sys/mman.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <errno.h>

స్థిర నిర్మాణం TestSyscallGlobal {
  పూర్ణాంకం bPersist;                   /* 1 for persistent errors, 0 for transient */
  పూర్ణాంకం n_లెక్క;                     /* Fail after this many more calls */
  పూర్ణాంకం nFail;                      /* Number of failures that have occurred */
  పూర్ణాంకం pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

స్థిర పూర్ణాంకం ts_open(స్థిరరాశి అక్షరం *, పూర్ణాంకం, పూర్ణాంకం);
స్థిర పూర్ణాంకం ts_close(పూర్ణాంకం fd);
స్థిర పూర్ణాంకం ts_access(స్థిరరాశి అక్షరం *z_మార్గం, పూర్ణాంకం బహుళకం);
స్థిర అక్షరం *ts_getcwd(అక్షరం *z_మార్గం, size_t n_మార్గం);
స్థిర పూర్ణాంకం ts_stat(స్థిరరాశి అక్షరం *z_మార్గం, నిర్మాణం stat *p);
స్థిర పూర్ణాంకం ts_fstat(పూర్ణాంకం fd, నిర్మాణం stat *p);
స్థిర పూర్ణాంకం ts_ftruncate(పూర్ణాంకం fd, off_t n);
స్థిర పూర్ణాంకం ts_fcntl(పూర్ణాంకం fd, పూర్ణాంకం cmd, ... );
స్థిర ssize_t ts_read(పూర్ణాంకం fd, శూన్య *aBuf, size_t nBuf);
స్థిర ssize_t ts_pread(పూర్ణాంకం fd, శూన్య *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
స్థిర ssize_t ts_pread64(పూర్ణాంకం fd, శూన్య *aBuf, size_t nBuf, sqlite3_uint64 off);
స్థిర ssize_t ts_write(పూర్ణాంకం fd, స్థిరరాశి శూన్య *aBuf, size_t nBuf);
స్థిర ssize_t ts_pwrite(పూర్ణాంకం fd, స్థిరరాశి శూన్య *aBuf, size_t nBuf, off_t off);
స్థిర ssize_t ts_pwrite64(పూర్ణాంకం fd, స్థిరరాశి శూన్య *aBuf, size_t nBuf, sqlite3_uint64 off);
స్థిర పూర్ణాంకం ts_fchmod(పూర్ణాంకం fd, బహుళకం_t బహుళకం);
స్థిర పూర్ణాంకం ts_fallocate(పూర్ణాంకం fd, off_t off, off_t len);
స్థిర శూన్య *ts_mmap(శూన్య *, size_t, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం, off_t);
స్థిర శూన్య *ts_mremap(శూన్య*, size_t, size_t, పూర్ణాంకం, ...);

నిర్మాణం TestSyscallArray {
  స్థిరరాశి అక్షరం *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  పూర్ణాంకం default_errno;              /* Default value for errno following errors */
  పూర్ణాంకం custom_errno;               /* Current value for errno if error */
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

#నిర్వచించు orig_open      ((పూర్ణాంకం(*)(స్థిరరాశి అక్షరం *, పూర్ణాంకం, పూర్ణాంకం))aSyscall[0].xOrig)
#నిర్వచించు orig_close     ((పూర్ణాంకం(*)(పూర్ణాంకం))aSyscall[1].xOrig)
#నిర్వచించు orig_access    ((పూర్ణాంకం(*)(స్థిరరాశి అక్షరం*,పూర్ణాంకం))aSyscall[2].xOrig)
#నిర్వచించు orig_getcwd    ((అక్షరం*(*)(అక్షరం*,size_t))aSyscall[3].xOrig)
#నిర్వచించు orig_stat      ((పూర్ణాంకం(*)(స్థిరరాశి అక్షరం*,నిర్మాణం stat*))aSyscall[4].xOrig)
#నిర్వచించు orig_fstat     ((పూర్ణాంకం(*)(పూర్ణాంకం,నిర్మాణం stat*))aSyscall[5].xOrig)
#నిర్వచించు orig_ftruncate ((పూర్ణాంకం(*)(పూర్ణాంకం,off_t))aSyscall[6].xOrig)
#నిర్వచించు orig_fcntl     ((పూర్ణాంకం(*)(పూర్ణాంకం,పూర్ణాంకం,...))aSyscall[7].xOrig)
#నిర్వచించు orig_read      ((ssize_t(*)(పూర్ణాంకం,శూన్య*,size_t))aSyscall[8].xOrig)
#నిర్వచించు orig_pread     ((ssize_t(*)(పూర్ణాంకం,శూన్య*,size_t,off_t))aSyscall[9].xOrig)
#నిర్వచించు orig_pread64   ((ssize_t(*)(పూర్ణాంకం,శూన్య*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#నిర్వచించు orig_write     ((ssize_t(*)(పూర్ణాంకం,స్థిరరాశి శూన్య*,size_t))aSyscall[11].xOrig)
#నిర్వచించు orig_pwrite    ((ssize_t(*)(పూర్ణాంకం,స్థిరరాశి శూన్య*,size_t,off_t))\
                       aSyscall[12].xOrig)
#నిర్వచించు orig_pwrite64  ((ssize_t(*)(పూర్ణాంకం,స్థిరరాశి శూన్య*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#నిర్వచించు orig_fchmod    ((పూర్ణాంకం(*)(పూర్ణాంకం,బహుళకం_t))aSyscall[14].xOrig)
#నిర్వచించు orig_fallocate ((పూర్ణాంకం(*)(పూర్ణాంకం,off_t,off_t))aSyscall[15].xOrig)
#నిర్వచించు orig_mmap      ((శూన్య*(*)(శూన్య*,size_t,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం,off_t))aSyscall[16].xOrig)
#నిర్వచించు orig_mremap    ((శూన్య*(*)(శూన్య*,size_t,size_t,పూర్ణాంకం,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
స్థిర పూర్ణాంకం tsIsFail(శూన్య){
  gSyscall.n_లెక్క--;
  ఒకవేళ( gSyscall.n_లెక్క==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    తిరిగి 1;
  }
  తిరిగి 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
స్థిర పూర్ణాంకం tsErrno(స్థిరరాశి అక్షరం *zFunc){
  పూర్ణాంకం i;
  size_t nFunc = అక్షరశ్రేణి_పొడవు(zFunc);
  కొరకు(i=0; aSyscall[i].zName; i++){
    ఒకవేళ( అక్షరశ్రేణి_పొడవు(aSyscall[i].zName)!=nFunc ) కొనసాగు;
    ఒకవేళ( memcmp(aSyscall[i].zName, zFunc, nFunc) ) కొనసాగు;
    తిరిగి aSyscall[i].custom_errno;
  }

  ప్రతిజ్ఞా(0);
  తిరిగి 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
స్థిర పూర్ణాంకం tsIsFailErrno(స్థిరరాశి అక్షరం *zFunc){
  ఒకవేళ( tsIsFail() ){
    errno = tsErrno(zFunc);
    తిరిగి 1;
  }
  తిరిగి 0;
}

/*
** A wrapper around open().
*/
స్థిర పూర్ణాంకం ts_open(స్థిరరాశి అక్షరం *zFile, పూర్ణాంకం flags, పూర్ణాంకం బహుళకం){
  ఒకవేళ( tsIsFailErrno("open") ){
    తిరిగి -1;
  }
  తిరిగి orig_open(zFile, flags, బహుళకం);
}

/*
** A wrapper around close().
*/
స్థిర పూర్ణాంకం ts_close(పూర్ణాంకం fd){
  ఒకవేళ( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    తిరిగి -1;
  }
  తిరిగి orig_close(fd);
}

/*
** A wrapper around access().
*/
స్థిర పూర్ణాంకం ts_access(స్థిరరాశి అక్షరం *z_మార్గం, పూర్ణాంకం బహుళకం){
  ఒకవేళ( tsIsFail() ){
    తిరిగి -1;
  }
  తిరిగి orig_access(z_మార్గం, బహుళకం);
}

/*
** A wrapper around getcwd().
*/
స్థిర అక్షరం *ts_getcwd(అక్షరం *z_మార్గం, size_t n_మార్గం){
  ఒకవేళ( tsIsFail() ){
    తిరిగి ఖాళీ;
  }
  తిరిగి orig_getcwd(z_మార్గం, n_మార్గం);
}

/*
** A wrapper around stat().
*/
స్థిర పూర్ణాంకం ts_stat(స్థిరరాశి అక్షరం *z_మార్గం, నిర్మాణం stat *p){
  ఒకవేళ( tsIsFail() ){
    తిరిగి -1;
  }
  తిరిగి orig_stat(z_మార్గం, p);
}

/*
** A wrapper around fstat().
*/
స్థిర పూర్ణాంకం ts_fstat(పూర్ణాంకం fd, నిర్మాణం stat *p){
  ఒకవేళ( tsIsFailErrno("fstat") ){
    తిరిగి -1;
  }
  తిరిగి orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
స్థిర పూర్ణాంకం ts_ftruncate(పూర్ణాంకం fd, off_t n){
  ఒకవేళ( tsIsFailErrno("ftruncate") ){
    తిరిగి -1;
  }
  తిరిగి orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
స్థిర పూర్ణాంకం ts_fcntl(పూర్ణాంకం fd, పూర్ణాంకం cmd, ... ){
  va_list ap;
  శూన్య *pArg;
  ఒకవేళ( tsIsFailErrno("fcntl") ){
    తిరిగి -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, శూన్య *);
  తిరిగి orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
స్థిర ssize_t ts_read(పూర్ణాంకం fd, శూన్య *aBuf, size_t nBuf){
  ఒకవేళ( tsIsFailErrno("read") ){
    తిరిగి -1;
  }
  తిరిగి orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
స్థిర ssize_t ts_pread(పూర్ణాంకం fd, శూన్య *aBuf, size_t nBuf, off_t off){
  ఒకవేళ( tsIsFailErrno("pread") ){
    తిరిగి -1;
  }
  తిరిగి orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
స్థిర ssize_t ts_pread64(పూర్ణాంకం fd, శూన్య *aBuf, size_t nBuf, sqlite3_uint64 off){
  ఒకవేళ( tsIsFailErrno("pread64") ){
    తిరిగి -1;
  }
  తిరిగి orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
స్థిర ssize_t ts_write(పూర్ణాంకం fd, స్థిరరాశి శూన్య *aBuf, size_t nBuf){
  ఒకవేళ( tsIsFailErrno("write") ){
    ఒకవేళ( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    తిరిగి -1;
  }
  తిరిగి orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
స్థిర ssize_t ts_pwrite(పూర్ణాంకం fd, స్థిరరాశి శూన్య *aBuf, size_t nBuf, off_t off){
  ఒకవేళ( tsIsFailErrno("pwrite") ){
    తిరిగి -1;
  }
  తిరిగి orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
స్థిర ssize_t ts_pwrite64(పూర్ణాంకం fd, స్థిరరాశి శూన్య *aBuf, size_t nBuf, sqlite3_uint64 off){
  ఒకవేళ( tsIsFailErrno("pwrite64") ){
    తిరిగి -1;
  }
  తిరిగి orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
స్థిర పూర్ణాంకం ts_fchmod(పూర్ణాంకం fd, బహుళకం_t బహుళకం){
  ఒకవేళ( tsIsFail() ){
    తిరిగి -1;
  }
  తిరిగి orig_fchmod(fd, బహుళకం);
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
స్థిర పూర్ణాంకం ts_fallocate(పూర్ణాంకం fd, off_t off, off_t len){
  ఒకవేళ( tsIsFail() ){
    తిరిగి tsErrno("fallocate");
  }
  తిరిగి orig_fallocate(fd, off, len);
}

స్థిర శూన్య *ts_mmap(
  శూన్య *pAddr, 
  size_t nByte, 
  పూర్ణాంకం prot, 
  పూర్ణాంకం flags, 
  పూర్ణాంకం fd, 
  off_t iOff
){
  ఒకవేళ( tsIsFailErrno("mmap") ){
    తిరిగి MAP_FAILED;
  }
  తిరిగి orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

స్థిర శూన్య *ts_mremap(శూన్య *a, size_t b, size_t c, పూర్ణాంకం d, ...){
  ఒకవేళ( tsIsFailErrno("mremap") ){
    తిరిగి MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#నిర్వచితమైతే MREMAP_FIXED
  ప్రతిజ్ఞా( (d & MREMAP_FIXED)==0 );
#అయితే_ముగింపు
  తిరిగి orig_mremap(a, b, c, d);
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_syscall_install(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_కొలత nElem;
  పూర్ణాంకం i;
  Tcl_Obj **apElem;

  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "SYSCALL-LIST");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    తిరిగి TCL_ERROR;
  }
  pVfs = sqlite3_vfs_కనుగొను(0);

  కొరకు(i=0; i<(పూర్ణాంకం)nElem; i++){
    పూర్ణాంకం iCall;
    పూర్ణాంకం rc = Tcl_Get_సూచీ_From_Obj_Struct(interp, 
        apElem[i], aSyscall, పరిమాణం(aSyscall[0]), "system-call", 0, &iCall
    );
    ఒకవేళ( rc ) తిరిగి rc;
    ఒకవేళ( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->x_సమితి_System_Call(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_syscall_uninstall(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  పూర్ణాంకం i;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "");
    తిరిగి TCL_ERROR;
  }

  pVfs = sqlite3_vfs_కనుగొను(0);
  కొరకు(i=0; aSyscall[i].zName; i++){
    ఒకవేళ( aSyscall[i].xOrig ){
      pVfs->x_సమితి_System_Call(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_syscall_reset(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  పూర్ణాంకం i;
  పూర్ణాంకం rc;

  ఒకవేళ( objc!=2 && objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "");
    తిరిగి TCL_ERROR;
  }

  pVfs = sqlite3_vfs_కనుగొను(0);
  ఒకవేళ( objc==2 ){
    rc = pVfs->x_సమితి_System_Call(pVfs, 0, 0);
    కొరకు(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }లేకపోతే{
    Tcl_కొలత nFunc;
    అక్షరం *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->x_సమితి_System_Call(pVfs, Tcl_GetString(objv[2]), 0);
    కొరకు(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      ఒకవేళ( అక్షరశ్రేణి_పొడవు(aSyscall[i].zName)!=nFunc ) కొనసాగు;
      ఒకవేళ( memcmp(aSyscall[i].zName, zFunc, nFunc) ) కొనసాగు;
      aSyscall[i].xOrig = 0;
    }
  }
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    తిరిగి TCL_ERROR;
  }

  Tcl_Reset_ఫలితం(interp);
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_syscall_exists(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "");
    తిరిగి TCL_ERROR;
  }

  pVfs = sqlite3_vfs_కనుగొను(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewBooleanObj(x!=0));
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_syscall_fault(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం n_లెక్క = 0;
  పూర్ణాంకం bPersist = 0;

  ఒకవేళ( objc!=2 && objc!=4 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?COUNT PERSIST?");
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( objc==4 ){
    ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &n_లెక్క)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      తిరిగి TCL_ERROR;
    }
  }

  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_లెక్క = n_లెక్క;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_syscall_errno(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  పూర్ణాంకం iCall;
  పూర్ణాంకం iErrno;
  పూర్ణాంకం rc;

  నిర్మాణం Errno {
    స్థిరరాశి అక్షరం *z;
    పూర్ణాంకం i;
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

  ఒకవేళ( objc!=4 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "SYSCALL ERRNO");
    తిరిగి TCL_ERROR;
  }

  rc = Tcl_Get_సూచీ_From_Obj_Struct(interp, 
      objv[2], aSyscall, పరిమాణం(aSyscall[0]), "system-call", 0, &iCall
  );
  ఒకవేళ( rc!=TCL_OK ) తిరిగి rc;
  rc = Tcl_Get_సూచీ_From_Obj_Struct(interp, 
      objv[3], aErrno, పరిమాణం(aErrno[0]), "errno", 0, &iErrno
  );
  ఒకవేళ( rc!=TCL_OK ) తిరిగి rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_syscall_list(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  స్థిరరాశి అక్షరం *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "");
    తిరిగి TCL_ERROR;
  }

  pVfs = sqlite3_vfs_కనుగొను(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_లెక్క(pList);
  కొరకు(zSys = pVfs->x_తదుపరి_System_Call(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->x_తదుపరి_System_Call(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_సమితి_Obj_ఫలితం(interp, pList);
  Tcl_Decr_Ref_లెక్క(pList);
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_syscall_defaultvfs(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "");
    తిరిగి TCL_ERROR;
  }

  pVfs = sqlite3_vfs_కనుగొను(0);
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj(pVfs->zName, -1));
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం ts_getpagesize(శూన్య){
  తిరిగి gSyscall.pgsz;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_syscall_pagesize(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_కనుగొను(0);
  పూర్ణాంకం pgsz;
  ఒకవేళ( objc!=3 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "PGSZ");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    తిరిగి TCL_ERROR;
  }

  ఒకవేళ( pgsz<0 ){
    ఒకవేళ( gSyscall.orig_getpagesize ){
      pVfs->x_సమితి_System_Call(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }లేకపోతే{
    ఒకవేళ( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_Append_ఫలితం(interp, "pgsz out of range", ఖాళీ);
      తిరిగి TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->x_సమితి_System_Call(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_syscall(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  నిర్మాణం SyscallCmd {
    స్థిరరాశి అక్షరం *zName;
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
  పూర్ణాంకం iCmd;
  పూర్ణాంకం rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_కనుగొను(0);

  ఒకవేళ( objc<2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "SUB-COMMAND ...");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( pVfs->i_సంస్కరణ<3 || pVfs->x_సమితి_System_Call==0 ){
    Tcl_Append_ఫలితం(interp, "VFS does not support xSetSystemCall", ఖాళీ);
    rc = TCL_ERROR;
  }లేకపోతే{
    rc = Tcl_Get_సూచీ_From_Obj_Struct(interp, 
        objv[1], aCmd, పరిమాణం(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  ఒకవేళ( rc!=TCL_OK ) తిరిగి rc;
  తిరిగి aCmd[iCmd].xCmd(client_దత్తాంశం, interp, objc, objv);
}

పూర్ణాంకం SqlitetestSyscall_Init(Tcl_Interp *interp){
  నిర్మాణం SyscallCmd {
    స్థిరరాశి అక్షరం *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  పూర్ణాంకం i;

  కొరకు(i=0; i<పరిమాణం(aCmd)/పరిమాణం(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  తిరిగి TCL_OK;
}
#లేకపోతే
పూర్ణాంకం SqlitetestSyscall_Init(Tcl_Interp *interp){
  తిరిగి TCL_OK;
}
#అయితే_ముగింపు
