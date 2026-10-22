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

#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "sqlite3.h"
#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <assert.h>

#যদি SQLITE_OS_UNIX

/* From main.c */
বাহ্যিক ধ্রুবক অক্ষর *sqlite3ErrName(পূর্ণসংখ্যা);

#অন্তর্ভাব <sys/mman.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <errno.h>

স্থির কাঠামো TestSyscallGlobal {
  পূর্ণসংখ্যা bPersist;                   /* 1 for persistent errors, 0 for transient */
  পূর্ণসংখ্যা n_গুনতি;                     /* Fail after this many more calls */
  পূর্ণসংখ্যা nFail;                      /* Number of failures that have occurred */
  পূর্ণসংখ্যা pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

স্থির পূর্ণসংখ্যা ts_open(ধ্রুবক অক্ষর *, পূর্ণসংখ্যা, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা ts_close(পূর্ণসংখ্যা fd);
স্থির পূর্ণসংখ্যা ts_access(ধ্রুবক অক্ষর *zPath, পূর্ণসংখ্যা mode);
স্থির অক্ষর *ts_getcwd(অক্ষর *zPath, size_t nPath);
স্থির পূর্ণসংখ্যা ts_stat(ধ্রুবক অক্ষর *zPath, কাঠামো stat *p);
স্থির পূর্ণসংখ্যা ts_fstat(পূর্ণসংখ্যা fd, কাঠামো stat *p);
স্থির পূর্ণসংখ্যা ts_ftruncate(পূর্ণসংখ্যা fd, off_t n);
স্থির পূর্ণসংখ্যা ts_fcntl(পূর্ণসংখ্যা fd, পূর্ণসংখ্যা cmd, ... );
স্থির ssize_t ts_read(পূর্ণসংখ্যা fd, শূন্য *aBuf, size_t nBuf);
স্থির ssize_t ts_pread(পূর্ণসংখ্যা fd, শূন্য *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
স্থির ssize_t ts_pread64(পূর্ণসংখ্যা fd, শূন্য *aBuf, size_t nBuf, sqlite3_uint64 off);
স্থির ssize_t ts_write(পূর্ণসংখ্যা fd, ধ্রুবক শূন্য *aBuf, size_t nBuf);
স্থির ssize_t ts_pwrite(পূর্ণসংখ্যা fd, ধ্রুবক শূন্য *aBuf, size_t nBuf, off_t off);
স্থির ssize_t ts_pwrite64(পূর্ণসংখ্যা fd, ধ্রুবক শূন্য *aBuf, size_t nBuf, sqlite3_uint64 off);
স্থির পূর্ণসংখ্যা ts_fchmod(পূর্ণসংখ্যা fd, mode_t mode);
স্থির পূর্ণসংখ্যা ts_fallocate(পূর্ণসংখ্যা fd, off_t off, off_t len);
স্থির শূন্য *ts_mmap(শূন্য *, size_t, পূর্ণসংখ্যা, পূর্ণসংখ্যা, পূর্ণসংখ্যা, off_t);
স্থির শূন্য *ts_mremap(শূন্য*, size_t, size_t, পূর্ণসংখ্যা, ...);

কাঠামো TestSyscallArray {
  ধ্রুবক অক্ষর *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  পূর্ণসংখ্যা default_errno;              /* Default value for errno following errors */
  পূর্ণসংখ্যা custom_errno;               /* Current value for errno if error */
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

#define orig_open      ((পূর্ণসংখ্যা(*)(ধ্রুবক অক্ষর *, পূর্ণসংখ্যা, পূর্ণসংখ্যা))aSyscall[0].xOrig)
#define orig_close     ((পূর্ণসংখ্যা(*)(পূর্ণসংখ্যা))aSyscall[1].xOrig)
#define orig_access    ((পূর্ণসংখ্যা(*)(ধ্রুবক অক্ষর*,পূর্ণসংখ্যা))aSyscall[2].xOrig)
#define orig_getcwd    ((অক্ষর*(*)(অক্ষর*,size_t))aSyscall[3].xOrig)
#define orig_stat      ((পূর্ণসংখ্যা(*)(ধ্রুবক অক্ষর*,কাঠামো stat*))aSyscall[4].xOrig)
#define orig_fstat     ((পূর্ণসংখ্যা(*)(পূর্ণসংখ্যা,কাঠামো stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((পূর্ণসংখ্যা(*)(পূর্ণসংখ্যা,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((পূর্ণসংখ্যা(*)(পূর্ণসংখ্যা,পূর্ণসংখ্যা,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(পূর্ণসংখ্যা,শূন্য*,size_t))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(পূর্ণসংখ্যা,শূন্য*,size_t,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(পূর্ণসংখ্যা,শূন্য*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(পূর্ণসংখ্যা,ধ্রুবক শূন্য*,size_t))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(পূর্ণসংখ্যা,ধ্রুবক শূন্য*,size_t,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(পূর্ণসংখ্যা,ধ্রুবক শূন্য*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((পূর্ণসংখ্যা(*)(পূর্ণসংখ্যা,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((পূর্ণসংখ্যা(*)(পূর্ণসংখ্যা,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((শূন্য*(*)(শূন্য*,size_t,পূর্ণসংখ্যা,পূর্ণসংখ্যা,পূর্ণসংখ্যা,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((শূন্য*(*)(শূন্য*,size_t,size_t,পূর্ণসংখ্যা,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
স্থির পূর্ণসংখ্যা tsIsFail(শূন্য){
  gSyscall.n_গুনতি--;
  যদি( gSyscall.n_গুনতি==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    ফেরত 1;
  }
  ফেরত 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
স্থির পূর্ণসংখ্যা tsErrno(ধ্রুবক অক্ষর *zFunc){
  পূর্ণসংখ্যা i;
  size_t nFunc = strlen(zFunc);
  জন্য(i=0; aSyscall[i].zName; i++){
    যদি( strlen(aSyscall[i].zName)!=nFunc ) চালু;
    যদি( memcmp(aSyscall[i].zName, zFunc, nFunc) ) চালু;
    ফেরত aSyscall[i].custom_errno;
  }

  প্রতিজ্ঞা(0);
  ফেরত 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
স্থির পূর্ণসংখ্যা tsIsFailErrno(ধ্রুবক অক্ষর *zFunc){
  যদি( tsIsFail() ){
    errno = tsErrno(zFunc);
    ফেরত 1;
  }
  ফেরত 0;
}

/*
** A wrapper around open().
*/
স্থির পূর্ণসংখ্যা ts_open(ধ্রুবক অক্ষর *zFile, পূর্ণসংখ্যা flags, পূর্ণসংখ্যা mode){
  যদি( tsIsFailErrno("open") ){
    ফেরত -1;
  }
  ফেরত orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
স্থির পূর্ণসংখ্যা ts_close(পূর্ণসংখ্যা fd){
  যদি( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    ফেরত -1;
  }
  ফেরত orig_close(fd);
}

/*
** A wrapper around access().
*/
স্থির পূর্ণসংখ্যা ts_access(ধ্রুবক অক্ষর *zPath, পূর্ণসংখ্যা mode){
  যদি( tsIsFail() ){
    ফেরত -1;
  }
  ফেরত orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
স্থির অক্ষর *ts_getcwd(অক্ষর *zPath, size_t nPath){
  যদি( tsIsFail() ){
    ফেরত ফাঁকা;
  }
  ফেরত orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
স্থির পূর্ণসংখ্যা ts_stat(ধ্রুবক অক্ষর *zPath, কাঠামো stat *p){
  যদি( tsIsFail() ){
    ফেরত -1;
  }
  ফেরত orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
স্থির পূর্ণসংখ্যা ts_fstat(পূর্ণসংখ্যা fd, কাঠামো stat *p){
  যদি( tsIsFailErrno("fstat") ){
    ফেরত -1;
  }
  ফেরত orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
স্থির পূর্ণসংখ্যা ts_ftruncate(পূর্ণসংখ্যা fd, off_t n){
  যদি( tsIsFailErrno("ftruncate") ){
    ফেরত -1;
  }
  ফেরত orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
স্থির পূর্ণসংখ্যা ts_fcntl(পূর্ণসংখ্যা fd, পূর্ণসংখ্যা cmd, ... ){
  va_list ap;
  শূন্য *pArg;
  যদি( tsIsFailErrno("fcntl") ){
    ফেরত -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, শূন্য *);
  ফেরত orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
স্থির ssize_t ts_read(পূর্ণসংখ্যা fd, শূন্য *aBuf, size_t nBuf){
  যদি( tsIsFailErrno("read") ){
    ফেরত -1;
  }
  ফেরত orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
স্থির ssize_t ts_pread(পূর্ণসংখ্যা fd, শূন্য *aBuf, size_t nBuf, off_t off){
  যদি( tsIsFailErrno("pread") ){
    ফেরত -1;
  }
  ফেরত orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
স্থির ssize_t ts_pread64(পূর্ণসংখ্যা fd, শূন্য *aBuf, size_t nBuf, sqlite3_uint64 off){
  যদি( tsIsFailErrno("pread64") ){
    ফেরত -1;
  }
  ফেরত orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
স্থির ssize_t ts_write(পূর্ণসংখ্যা fd, ধ্রুবক শূন্য *aBuf, size_t nBuf){
  যদি( tsIsFailErrno("write") ){
    যদি( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    ফেরত -1;
  }
  ফেরত orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
স্থির ssize_t ts_pwrite(পূর্ণসংখ্যা fd, ধ্রুবক শূন্য *aBuf, size_t nBuf, off_t off){
  যদি( tsIsFailErrno("pwrite") ){
    ফেরত -1;
  }
  ফেরত orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
স্থির ssize_t ts_pwrite64(পূর্ণসংখ্যা fd, ধ্রুবক শূন্য *aBuf, size_t nBuf, sqlite3_uint64 off){
  যদি( tsIsFailErrno("pwrite64") ){
    ফেরত -1;
  }
  ফেরত orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
স্থির পূর্ণসংখ্যা ts_fchmod(পূর্ণসংখ্যা fd, mode_t mode){
  যদি( tsIsFail() ){
    ফেরত -1;
  }
  ফেরত orig_fchmod(fd, mode);
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
স্থির পূর্ণসংখ্যা ts_fallocate(পূর্ণসংখ্যা fd, off_t off, off_t len){
  যদি( tsIsFail() ){
    ফেরত tsErrno("fallocate");
  }
  ফেরত orig_fallocate(fd, off, len);
}

স্থির শূন্য *ts_mmap(
  শূন্য *pAddr, 
  size_t nByte, 
  পূর্ণসংখ্যা prot, 
  পূর্ণসংখ্যা flags, 
  পূর্ণসংখ্যা fd, 
  off_t iOff
){
  যদি( tsIsFailErrno("mmap") ){
    ফেরত MAP_FAILED;
  }
  ফেরত orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

স্থির শূন্য *ts_mremap(শূন্য *a, size_t b, size_t c, পূর্ণসংখ্যা d, ...){
  যদি( tsIsFailErrno("mremap") ){
    ফেরত MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  প্রতিজ্ঞা( (d & MREMAP_FIXED)==0 );
#endif
  ফেরত orig_mremap(a, b, c, d);
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_syscall_install(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_আকার nElem;
  পূর্ণসংখ্যা i;
  Tcl_Obj **apElem;

  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "SYSCALL-LIST");
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    ফেরত TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  জন্য(i=0; i<(পূর্ণসংখ্যা)nElem; i++){
    পূর্ণসংখ্যা iCall;
    পূর্ণসংখ্যা rc = Tcl_Get_সূচক_From_Obj_Struct(interp, 
        apElem[i], aSyscall, আকার(aSyscall[0]), "system-call", 0, &iCall
    );
    যদি( rc ) ফেরত rc;
    যদি( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_syscall_uninstall(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  পূর্ণসংখ্যা i;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "");
    ফেরত TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  জন্য(i=0; aSyscall[i].zName; i++){
    যদি( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_syscall_reset(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা rc;

  যদি( objc!=2 && objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "");
    ফেরত TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  যদি( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    জন্য(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }অন্যথা{
    Tcl_আকার nFunc;
    অক্ষর *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    জন্য(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      যদি( strlen(aSyscall[i].zName)!=nFunc ) চালু;
      যদি( memcmp(aSyscall[i].zName, zFunc, nFunc) ) চালু;
      aSyscall[i].xOrig = 0;
    }
  }
  যদি( rc!=SQLITE_OK ){
    Tcl_Set_Obj_ফলাফল(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ফেরত TCL_ERROR;
  }

  Tcl_Reset_ফলাফল(interp);
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_syscall_exists(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "");
    ফেরত TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewBooleanObj(x!=0));
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_syscall_fault(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা n_গুনতি = 0;
  পূর্ণসংখ্যা bPersist = 0;

  যদি( objc!=2 && objc!=4 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?COUNT PERSIST?");
    ফেরত TCL_ERROR;
  }

  যদি( objc==4 ){
    যদি( Tcl_GetIntFromObj(interp, objv[2], &n_গুনতি)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      ফেরত TCL_ERROR;
    }
  }

  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_গুনতি = n_গুনতি;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_syscall_errno(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  পূর্ণসংখ্যা iCall;
  পূর্ণসংখ্যা iErrno;
  পূর্ণসংখ্যা rc;

  কাঠামো Errno {
    ধ্রুবক অক্ষর *z;
    পূর্ণসংখ্যা i;
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
    ফেরত TCL_ERROR;
  }

  rc = Tcl_Get_সূচক_From_Obj_Struct(interp, 
      objv[2], aSyscall, আকার(aSyscall[0]), "system-call", 0, &iCall
  );
  যদি( rc!=TCL_OK ) ফেরত rc;
  rc = Tcl_Get_সূচক_From_Obj_Struct(interp, 
      objv[3], aErrno, আকার(aErrno[0]), "errno", 0, &iErrno
  );
  যদি( rc!=TCL_OK ) ফেরত rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_syscall_list(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  ধ্রুবক অক্ষর *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "");
    ফেরত TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_গুনতি(pList);
  জন্য(zSys = pVfs->x_পরবর্তী_System_Call(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->x_পরবর্তী_System_Call(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_Set_Obj_ফলাফল(interp, pList);
  Tcl_Decr_Ref_গুনতি(pList);
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_syscall_defaultvfs(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "");
    ফেরত TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewStringObj(pVfs->zName, -1));
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা ts_getpagesize(শূন্য){
  ফেরত gSyscall.pgsz;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_syscall_pagesize(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  পূর্ণসংখ্যা pgsz;
  যদি( objc!=3 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "PGSZ");
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    ফেরত TCL_ERROR;
  }

  যদি( pgsz<0 ){
    যদি( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }অন্যথা{
    যদি( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_Append_ফলাফল(interp, "pgsz out of range", ফাঁকা);
      ফেরত TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_syscall(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  কাঠামো SyscallCmd {
    ধ্রুবক অক্ষর *zName;
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
  পূর্ণসংখ্যা iCmd;
  পূর্ণসংখ্যা rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  যদি( objc<2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "SUB-COMMAND ...");
    ফেরত TCL_ERROR;
  }
  যদি( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_Append_ফলাফল(interp, "VFS does not support xSetSystemCall", ফাঁকা);
    rc = TCL_ERROR;
  }অন্যথা{
    rc = Tcl_Get_সূচক_From_Obj_Struct(interp, 
        objv[1], aCmd, আকার(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  যদি( rc!=TCL_OK ) ফেরত rc;
  ফেরত aCmd[iCmd].xCmd(client_তথ্য, interp, objc, objv);
}

পূর্ণসংখ্যা SqlitetestSyscall_Init(Tcl_Interp *interp){
  কাঠামো SyscallCmd {
    ধ্রুবক অক্ষর *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  পূর্ণসংখ্যা i;

  জন্য(i=0; i<আকার(aCmd)/আকার(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  ফেরত TCL_OK;
}
#অন্যথা
পূর্ণসংখ্যা SqlitetestSyscall_Init(Tcl_Interp *interp){
  ফেরত TCL_OK;
}
#endif
