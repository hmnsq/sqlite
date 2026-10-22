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

#অন্তর্ভাৱ "sqliteInt.h"
#অন্তর্ভাৱ "sqlite3.h"
#অন্তর্ভাৱ "tclsqlite.h"
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <assert.h>

#যদি SQLITE_OS_UNIX

/* From main.c */
বাহ্য ধ্রুৱ ৱর্ণ *sqlite3ErrName(পূর্ণাঙ্ক);

#অন্তর্ভাৱ <sys/mman.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <errno.h>

স্থির সংরচনা TestSyscallGlobal {
  পূর্ণাঙ্ক bPersist;                   /* 1 for persistent errors, 0 for transient */
  পূর্ণাঙ্ক n_গুনতি;                     /* Fail after this many more calls */
  পূর্ণাঙ্ক nFail;                      /* Number of failures that have occurred */
  পূর্ণাঙ্ক pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

স্থির পূর্ণাঙ্ক ts_open(ধ্রুৱ ৱর্ণ *, পূর্ণাঙ্ক, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক ts_close(পূর্ণাঙ্ক fd);
স্থির পূর্ণাঙ্ক ts_access(ধ্রুৱ ৱর্ণ *zPath, পূর্ণাঙ্ক mode);
স্থির ৱর্ণ *ts_getcwd(ৱর্ণ *zPath, size_t nPath);
স্থির পূর্ণাঙ্ক ts_stat(ধ্রুৱ ৱর্ণ *zPath, সংরচনা stat *p);
স্থির পূর্ণাঙ্ক ts_fstat(পূর্ণাঙ্ক fd, সংরচনা stat *p);
স্থির পূর্ণাঙ্ক ts_ftruncate(পূর্ণাঙ্ক fd, off_t n);
স্থির পূর্ণাঙ্ক ts_fcntl(পূর্ণাঙ্ক fd, পূর্ণাঙ্ক cmd, ... );
স্থির ssize_t ts_read(পূর্ণাঙ্ক fd, শূন্য *aBuf, size_t nBuf);
স্থির ssize_t ts_pread(পূর্ণাঙ্ক fd, শূন্য *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
স্থির ssize_t ts_pread64(পূর্ণাঙ্ক fd, শূন্য *aBuf, size_t nBuf, sqlite3_uint64 off);
স্থির ssize_t ts_write(পূর্ণাঙ্ক fd, ধ্রুৱ শূন্য *aBuf, size_t nBuf);
স্থির ssize_t ts_pwrite(পূর্ণাঙ্ক fd, ধ্রুৱ শূন্য *aBuf, size_t nBuf, off_t off);
স্থির ssize_t ts_pwrite64(পূর্ণাঙ্ক fd, ধ্রুৱ শূন্য *aBuf, size_t nBuf, sqlite3_uint64 off);
স্থির পূর্ণাঙ্ক ts_fchmod(পূর্ণাঙ্ক fd, mode_t mode);
স্থির পূর্ণাঙ্ক ts_fallocate(পূর্ণাঙ্ক fd, off_t off, off_t len);
স্থির শূন্য *ts_mmap(শূন্য *, size_t, পূর্ণাঙ্ক, পূর্ণাঙ্ক, পূর্ণাঙ্ক, off_t);
স্থির শূন্য *ts_mremap(শূন্য*, size_t, size_t, পূর্ণাঙ্ক, ...);

সংরচনা TestSyscallArray {
  ধ্রুৱ ৱর্ণ *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  পূর্ণাঙ্ক default_errno;              /* Default value for errno following errors */
  পূর্ণাঙ্ক custom_errno;               /* Current value for errno if error */
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

#define orig_open      ((পূর্ণাঙ্ক(*)(ধ্রুৱ ৱর্ণ *, পূর্ণাঙ্ক, পূর্ণাঙ্ক))aSyscall[0].xOrig)
#define orig_close     ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক))aSyscall[1].xOrig)
#define orig_access    ((পূর্ণাঙ্ক(*)(ধ্রুৱ ৱর্ণ*,পূর্ণাঙ্ক))aSyscall[2].xOrig)
#define orig_getcwd    ((ৱর্ণ*(*)(ৱর্ণ*,size_t))aSyscall[3].xOrig)
#define orig_stat      ((পূর্ণাঙ্ক(*)(ধ্রুৱ ৱর্ণ*,সংরচনা stat*))aSyscall[4].xOrig)
#define orig_fstat     ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক,সংরচনা stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক,পূর্ণাঙ্ক,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(পূর্ণাঙ্ক,শূন্য*,size_t))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(পূর্ণাঙ্ক,শূন্য*,size_t,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(পূর্ণাঙ্ক,শূন্য*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(পূর্ণাঙ্ক,ধ্রুৱ শূন্য*,size_t))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(পূর্ণাঙ্ক,ধ্রুৱ শূন্য*,size_t,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(পূর্ণাঙ্ক,ধ্রুৱ শূন্য*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((পূর্ণাঙ্ক(*)(পূর্ণাঙ্ক,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((শূন্য*(*)(শূন্য*,size_t,পূর্ণাঙ্ক,পূর্ণাঙ্ক,পূর্ণাঙ্ক,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((শূন্য*(*)(শূন্য*,size_t,size_t,পূর্ণাঙ্ক,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
স্থির পূর্ণাঙ্ক tsIsFail(শূন্য){
  gSyscall.n_গুনতি--;
  যদি( gSyscall.n_গুনতি==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    ঘূৰাই 1;
  }
  ঘূৰাই 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
স্থির পূর্ণাঙ্ক tsErrno(ধ্রুৱ ৱর্ণ *zFunc){
  পূর্ণাঙ্ক i;
  size_t nFunc = strlen(zFunc);
  বাবে(i=0; aSyscall[i].zName; i++){
    যদি( strlen(aSyscall[i].zName)!=nFunc ) অনুৱর্ত;
    যদি( memcmp(aSyscall[i].zName, zFunc, nFunc) ) অনুৱর্ত;
    ঘূৰাই aSyscall[i].custom_errno;
  }

  প্রতিজ্ঞা(0);
  ঘূৰাই 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
স্থির পূর্ণাঙ্ক tsIsFailErrno(ধ্রুৱ ৱর্ণ *zFunc){
  যদি( tsIsFail() ){
    errno = tsErrno(zFunc);
    ঘূৰাই 1;
  }
  ঘূৰাই 0;
}

/*
** A wrapper around open().
*/
স্থির পূর্ণাঙ্ক ts_open(ধ্রুৱ ৱর্ণ *zFile, পূর্ণাঙ্ক flags, পূর্ণাঙ্ক mode){
  যদি( tsIsFailErrno("open") ){
    ঘূৰাই -1;
  }
  ঘূৰাই orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
স্থির পূর্ণাঙ্ক ts_close(পূর্ণাঙ্ক fd){
  যদি( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    ঘূৰাই -1;
  }
  ঘূৰাই orig_close(fd);
}

/*
** A wrapper around access().
*/
স্থির পূর্ণাঙ্ক ts_access(ধ্রুৱ ৱর্ণ *zPath, পূর্ণাঙ্ক mode){
  যদি( tsIsFail() ){
    ঘূৰাই -1;
  }
  ঘূৰাই orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
স্থির ৱর্ণ *ts_getcwd(ৱর্ণ *zPath, size_t nPath){
  যদি( tsIsFail() ){
    ঘূৰাই রিক্ত;
  }
  ঘূৰাই orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
স্থির পূর্ণাঙ্ক ts_stat(ধ্রুৱ ৱর্ণ *zPath, সংরচনা stat *p){
  যদি( tsIsFail() ){
    ঘূৰাই -1;
  }
  ঘূৰাই orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
স্থির পূর্ণাঙ্ক ts_fstat(পূর্ণাঙ্ক fd, সংরচনা stat *p){
  যদি( tsIsFailErrno("fstat") ){
    ঘূৰাই -1;
  }
  ঘূৰাই orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
স্থির পূর্ণাঙ্ক ts_ftruncate(পূর্ণাঙ্ক fd, off_t n){
  যদি( tsIsFailErrno("ftruncate") ){
    ঘূৰাই -1;
  }
  ঘূৰাই orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
স্থির পূর্ণাঙ্ক ts_fcntl(পূর্ণাঙ্ক fd, পূর্ণাঙ্ক cmd, ... ){
  va_list ap;
  শূন্য *pArg;
  যদি( tsIsFailErrno("fcntl") ){
    ঘূৰাই -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, শূন্য *);
  ঘূৰাই orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
স্থির ssize_t ts_read(পূর্ণাঙ্ক fd, শূন্য *aBuf, size_t nBuf){
  যদি( tsIsFailErrno("read") ){
    ঘূৰাই -1;
  }
  ঘূৰাই orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
স্থির ssize_t ts_pread(পূর্ণাঙ্ক fd, শূন্য *aBuf, size_t nBuf, off_t off){
  যদি( tsIsFailErrno("pread") ){
    ঘূৰাই -1;
  }
  ঘূৰাই orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
স্থির ssize_t ts_pread64(পূর্ণাঙ্ক fd, শূন্য *aBuf, size_t nBuf, sqlite3_uint64 off){
  যদি( tsIsFailErrno("pread64") ){
    ঘূৰাই -1;
  }
  ঘূৰাই orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
স্থির ssize_t ts_write(পূর্ণাঙ্ক fd, ধ্রুৱ শূন্য *aBuf, size_t nBuf){
  যদি( tsIsFailErrno("write") ){
    যদি( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    ঘূৰাই -1;
  }
  ঘূৰাই orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
স্থির ssize_t ts_pwrite(পূর্ণাঙ্ক fd, ধ্রুৱ শূন্য *aBuf, size_t nBuf, off_t off){
  যদি( tsIsFailErrno("pwrite") ){
    ঘূৰাই -1;
  }
  ঘূৰাই orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
স্থির ssize_t ts_pwrite64(পূর্ণাঙ্ক fd, ধ্রুৱ শূন্য *aBuf, size_t nBuf, sqlite3_uint64 off){
  যদি( tsIsFailErrno("pwrite64") ){
    ঘূৰাই -1;
  }
  ঘূৰাই orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
স্থির পূর্ণাঙ্ক ts_fchmod(পূর্ণাঙ্ক fd, mode_t mode){
  যদি( tsIsFail() ){
    ঘূৰাই -1;
  }
  ঘূৰাই orig_fchmod(fd, mode);
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
স্থির পূর্ণাঙ্ক ts_fallocate(পূর্ণাঙ্ক fd, off_t off, off_t len){
  যদি( tsIsFail() ){
    ঘূৰাই tsErrno("fallocate");
  }
  ঘূৰাই orig_fallocate(fd, off, len);
}

স্থির শূন্য *ts_mmap(
  শূন্য *pAddr, 
  size_t nByte, 
  পূর্ণাঙ্ক prot, 
  পূর্ণাঙ্ক flags, 
  পূর্ণাঙ্ক fd, 
  off_t iOff
){
  যদি( tsIsFailErrno("mmap") ){
    ঘূৰাই MAP_FAILED;
  }
  ঘূৰাই orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

স্থির শূন্য *ts_mremap(শূন্য *a, size_t b, size_t c, পূর্ণাঙ্ক d, ...){
  যদি( tsIsFailErrno("mremap") ){
    ঘূৰাই MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  প্রতিজ্ঞা( (d & MREMAP_FIXED)==0 );
#endif
  ঘূৰাই orig_mremap(a, b, c, d);
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_syscall_install(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_Size nElem;
  পূর্ণাঙ্ক i;
  Tcl_Obj **apElem;

  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "SYSCALL-LIST");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    ঘূৰাই TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  বাবে(i=0; i<(পূর্ণাঙ্ক)nElem; i++){
    পূর্ণাঙ্ক iCall;
    পূর্ণাঙ্ক rc = Tcl_GetIndexFromObjStruct(interp, 
        apElem[i], aSyscall, পরিমাণ(aSyscall[0]), "system-call", 0, &iCall
    );
    যদি( rc ) ঘূৰাই rc;
    যদি( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_syscall_uninstall(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  পূর্ণাঙ্ক i;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "");
    ঘূৰাই TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  বাবে(i=0; aSyscall[i].zName; i++){
    যদি( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_syscall_reset(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক rc;

  যদি( objc!=2 && objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "");
    ঘূৰাই TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  যদি( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    বাবে(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }অন্যথা{
    Tcl_Size nFunc;
    ৱর্ণ *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    বাবে(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      যদি( strlen(aSyscall[i].zName)!=nFunc ) অনুৱর্ত;
      যদি( memcmp(aSyscall[i].zName, zFunc, nFunc) ) অনুৱর্ত;
      aSyscall[i].xOrig = 0;
    }
  }
  যদি( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ঘূৰাই TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_syscall_exists(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "");
    ঘূৰাই TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(x!=0));
  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_syscall_fault(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক n_গুনতি = 0;
  পূর্ণাঙ্ক bPersist = 0;

  যদি( objc!=2 && objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?COUNT PERSIST?");
    ঘূৰাই TCL_ERROR;
  }

  যদি( objc==4 ){
    যদি( Tcl_GetIntFromObj(interp, objv[2], &n_গুনতি)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      ঘূৰাই TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_গুনতি = n_গুনতি;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_syscall_errno(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণাঙ্ক iCall;
  পূর্ণাঙ্ক iErrno;
  পূর্ণাঙ্ক rc;

  সংরচনা Errno {
    ধ্রুৱ ৱর্ণ *z;
    পূর্ণাঙ্ক i;
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

  যদি( objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "SYSCALL ERRNO");
    ঘূৰাই TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[2], aSyscall, পরিমাণ(aSyscall[0]), "system-call", 0, &iCall
  );
  যদি( rc!=TCL_OK ) ঘূৰাই rc;
  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[3], aErrno, পরিমাণ(aErrno[0]), "errno", 0, &iErrno
  );
  যদি( rc!=TCL_OK ) ঘূৰাই rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_syscall_list(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  ধ্রুৱ ৱর্ণ *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "");
    ঘূৰাই TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_গুনতি(pList);
  বাবে(zSys = pVfs->xNextSystemCall(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->xNextSystemCall(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_SetObjResult(interp, pList);
  Tcl_Decr_Ref_গুনতি(pList);
  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_syscall_defaultvfs(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "");
    ঘূৰাই TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(pVfs->zName, -1));
  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক ts_getpagesize(শূন্য){
  ঘূৰাই gSyscall.pgsz;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_syscall_pagesize(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  পূর্ণাঙ্ক pgsz;
  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "PGSZ");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    ঘূৰাই TCL_ERROR;
  }

  যদি( pgsz<0 ){
    যদি( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }অন্যথা{
    যদি( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_AppendResult(interp, "pgsz out of range", রিক্ত);
      ঘূৰাই TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_syscall(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  সংরচনা SyscallCmd {
    ধ্রুৱ ৱর্ণ *zName;
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
  পূর্ণাঙ্ক iCmd;
  পূর্ণাঙ্ক rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  যদি( objc<2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "SUB-COMMAND ...");
    ঘূৰাই TCL_ERROR;
  }
  যদি( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_AppendResult(interp, "VFS does not support xSetSystemCall", রিক্ত);
    rc = TCL_ERROR;
  }অন্যথা{
    rc = Tcl_GetIndexFromObjStruct(interp, 
        objv[1], aCmd, পরিমাণ(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  যদি( rc!=TCL_OK ) ঘূৰাই rc;
  ঘূৰাই aCmd[iCmd].xCmd(clientData, interp, objc, objv);
}

পূর্ণাঙ্ক SqlitetestSyscall_Init(Tcl_Interp *interp){
  সংরচনা SyscallCmd {
    ধ্রুৱ ৱর্ণ *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  পূর্ণাঙ্ক i;

  বাবে(i=0; i<পরিমাণ(aCmd)/পরিমাণ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  ঘূৰাই TCL_OK;
}
#অন্যথা
পূর্ণাঙ্ক SqlitetestSyscall_Init(Tcl_Interp *interp){
  ঘূৰাই TCL_OK;
}
#endif
