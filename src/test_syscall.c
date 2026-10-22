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

#شامل "sqliteInt.h"
#شامل "sqlite3.h"
#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>
#شامل <assert.h>

#اگر SQLITE_OS_UNIX

/* From main.c */
بیرونی ثابت نویسه *sqlite3ErrName(صحیح);

#شامل <sys/mman.h>
#شامل <sys/types.h>
#شامل <errno.h>

ایستا ساختار TestSyscallGlobal {
  صحیح bPersist;                   /* 1 for persistent errors, 0 for transient */
  صحیح n_شمار;                     /* Fail after this many more calls */
  صحیح nFail;                      /* Number of failures that have occurred */
  صحیح pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

ایستا صحیح ts_open(ثابت نویسه *, صحیح, صحیح);
ایستا صحیح ts_close(صحیح fd);
ایستا صحیح ts_access(ثابت نویسه *zPath, صحیح mode);
ایستا نویسه *ts_getcwd(نویسه *zPath, size_t nPath);
ایستا صحیح ts_stat(ثابت نویسه *zPath, ساختار stat *p);
ایستا صحیح ts_fstat(صحیح fd, ساختار stat *p);
ایستا صحیح ts_ftruncate(صحیح fd, off_t n);
ایستا صحیح ts_fcntl(صحیح fd, صحیح cmd, ... );
ایستا ssize_t ts_read(صحیح fd, تهی *aBuf, size_t nBuf);
ایستا ssize_t ts_pread(صحیح fd, تهی *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
ایستا ssize_t ts_pread64(صحیح fd, تهی *aBuf, size_t nBuf, sqlite3_uint64 off);
ایستا ssize_t ts_write(صحیح fd, ثابت تهی *aBuf, size_t nBuf);
ایستا ssize_t ts_pwrite(صحیح fd, ثابت تهی *aBuf, size_t nBuf, off_t off);
ایستا ssize_t ts_pwrite64(صحیح fd, ثابت تهی *aBuf, size_t nBuf, sqlite3_uint64 off);
ایستا صحیح ts_fchmod(صحیح fd, mode_t mode);
ایستا صحیح ts_fallocate(صحیح fd, off_t off, off_t len);
ایستا تهی *ts_mmap(تهی *, size_t, صحیح, صحیح, صحیح, off_t);
ایستا تهی *ts_mremap(تهی*, size_t, size_t, صحیح, ...);

ساختار TestSyscallArray {
  ثابت نویسه *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  صحیح default_errno;              /* Default value for errno following errors */
  صحیح custom_errno;               /* Current value for errno if error */
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

#define orig_open      ((صحیح(*)(ثابت نویسه *, صحیح, صحیح))aSyscall[0].xOrig)
#define orig_close     ((صحیح(*)(صحیح))aSyscall[1].xOrig)
#define orig_access    ((صحیح(*)(ثابت نویسه*,صحیح))aSyscall[2].xOrig)
#define orig_getcwd    ((نویسه*(*)(نویسه*,size_t))aSyscall[3].xOrig)
#define orig_stat      ((صحیح(*)(ثابت نویسه*,ساختار stat*))aSyscall[4].xOrig)
#define orig_fstat     ((صحیح(*)(صحیح,ساختار stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((صحیح(*)(صحیح,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((صحیح(*)(صحیح,صحیح,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(صحیح,تهی*,size_t))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(صحیح,تهی*,size_t,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(صحیح,تهی*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(صحیح,ثابت تهی*,size_t))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(صحیح,ثابت تهی*,size_t,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(صحیح,ثابت تهی*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((صحیح(*)(صحیح,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((صحیح(*)(صحیح,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((تهی*(*)(تهی*,size_t,صحیح,صحیح,صحیح,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((تهی*(*)(تهی*,size_t,size_t,صحیح,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
ایستا صحیح tsIsFail(تهی){
  gSyscall.n_شمار--;
  اگر( gSyscall.n_شمار==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    بازگشت 1;
  }
  بازگشت 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
ایستا صحیح tsErrno(ثابت نویسه *zFunc){
  صحیح i;
  size_t nFunc = strlen(zFunc);
  برای(i=0; aSyscall[i].zName; i++){
    اگر( strlen(aSyscall[i].zName)!=nFunc ) ادامه;
    اگر( memcmp(aSyscall[i].zName, zFunc, nFunc) ) ادامه;
    بازگشت aSyscall[i].custom_errno;
  }

  أكد(0);
  بازگشت 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
ایستا صحیح tsIsFailErrno(ثابت نویسه *zFunc){
  اگر( tsIsFail() ){
    errno = tsErrno(zFunc);
    بازگشت 1;
  }
  بازگشت 0;
}

/*
** A wrapper around open().
*/
ایستا صحیح ts_open(ثابت نویسه *zFile, صحیح flags, صحیح mode){
  اگر( tsIsFailErrno("open") ){
    بازگشت -1;
  }
  بازگشت orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
ایستا صحیح ts_close(صحیح fd){
  اگر( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    بازگشت -1;
  }
  بازگشت orig_close(fd);
}

/*
** A wrapper around access().
*/
ایستا صحیح ts_access(ثابت نویسه *zPath, صحیح mode){
  اگر( tsIsFail() ){
    بازگشت -1;
  }
  بازگشت orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
ایستا نویسه *ts_getcwd(نویسه *zPath, size_t nPath){
  اگر( tsIsFail() ){
    بازگشت پوچ;
  }
  بازگشت orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
ایستا صحیح ts_stat(ثابت نویسه *zPath, ساختار stat *p){
  اگر( tsIsFail() ){
    بازگشت -1;
  }
  بازگشت orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
ایستا صحیح ts_fstat(صحیح fd, ساختار stat *p){
  اگر( tsIsFailErrno("fstat") ){
    بازگشت -1;
  }
  بازگشت orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
ایستا صحیح ts_ftruncate(صحیح fd, off_t n){
  اگر( tsIsFailErrno("ftruncate") ){
    بازگشت -1;
  }
  بازگشت orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
ایستا صحیح ts_fcntl(صحیح fd, صحیح cmd, ... ){
  va_list ap;
  تهی *pArg;
  اگر( tsIsFailErrno("fcntl") ){
    بازگشت -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, تهی *);
  بازگشت orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
ایستا ssize_t ts_read(صحیح fd, تهی *aBuf, size_t nBuf){
  اگر( tsIsFailErrno("read") ){
    بازگشت -1;
  }
  بازگشت orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
ایستا ssize_t ts_pread(صحیح fd, تهی *aBuf, size_t nBuf, off_t off){
  اگر( tsIsFailErrno("pread") ){
    بازگشت -1;
  }
  بازگشت orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
ایستا ssize_t ts_pread64(صحیح fd, تهی *aBuf, size_t nBuf, sqlite3_uint64 off){
  اگر( tsIsFailErrno("pread64") ){
    بازگشت -1;
  }
  بازگشت orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
ایستا ssize_t ts_write(صحیح fd, ثابت تهی *aBuf, size_t nBuf){
  اگر( tsIsFailErrno("write") ){
    اگر( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    بازگشت -1;
  }
  بازگشت orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
ایستا ssize_t ts_pwrite(صحیح fd, ثابت تهی *aBuf, size_t nBuf, off_t off){
  اگر( tsIsFailErrno("pwrite") ){
    بازگشت -1;
  }
  بازگشت orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
ایستا ssize_t ts_pwrite64(صحیح fd, ثابت تهی *aBuf, size_t nBuf, sqlite3_uint64 off){
  اگر( tsIsFailErrno("pwrite64") ){
    بازگشت -1;
  }
  بازگشت orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
ایستا صحیح ts_fchmod(صحیح fd, mode_t mode){
  اگر( tsIsFail() ){
    بازگشت -1;
  }
  بازگشت orig_fchmod(fd, mode);
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
ایستا صحیح ts_fallocate(صحیح fd, off_t off, off_t len){
  اگر( tsIsFail() ){
    بازگشت tsErrno("fallocate");
  }
  بازگشت orig_fallocate(fd, off, len);
}

ایستا تهی *ts_mmap(
  تهی *pAddr, 
  size_t nByte, 
  صحیح prot, 
  صحیح flags, 
  صحیح fd, 
  off_t iOff
){
  اگر( tsIsFailErrno("mmap") ){
    بازگشت MAP_FAILED;
  }
  بازگشت orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

ایستا تهی *ts_mremap(تهی *a, size_t b, size_t c, صحیح d, ...){
  اگر( tsIsFailErrno("mremap") ){
    بازگشت MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  أكد( (d & MREMAP_FIXED)==0 );
#endif
  بازگشت orig_mremap(a, b, c, d);
}

ایستا صحیح SQLITE_TCLAPI test_syscall_install(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_Size nElem;
  صحیح i;
  Tcl_Obj **apElem;

  اگر( objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 2, objv, "SYSCALL-LIST");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    بازگشت TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  برای(i=0; i<(صحیح)nElem; i++){
    صحیح iCall;
    صحیح rc = Tcl_GetIndexFromObjStruct(interp, 
        apElem[i], aSyscall, اندازه(aSyscall[0]), "system-call", 0, &iCall
    );
    اگر( rc ) بازگشت rc;
    اگر( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_syscall_uninstall(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  صحیح i;

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 2, objv, "");
    بازگشت TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  برای(i=0; aSyscall[i].zName; i++){
    اگر( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_syscall_reset(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  صحیح i;
  صحیح rc;

  اگر( objc!=2 && objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 2, objv, "");
    بازگشت TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  اگر( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    برای(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }وگرنه{
    Tcl_Size nFunc;
    نویسه *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    برای(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      اگر( strlen(aSyscall[i].zName)!=nFunc ) ادامه;
      اگر( memcmp(aSyscall[i].zName, zFunc, nFunc) ) ادامه;
      aSyscall[i].xOrig = 0;
    }
  }
  اگر( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    بازگشت TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_syscall_exists(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  اگر( objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 2, objv, "");
    بازگشت TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(x!=0));
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_syscall_fault(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح n_شمار = 0;
  صحیح bPersist = 0;

  اگر( objc!=2 && objc!=4 ){
    Tcl_Wrong_رقم_Args(interp, 2, objv, "?COUNT PERSIST?");
    بازگشت TCL_ERROR;
  }

  اگر( objc==4 ){
    اگر( Tcl_GetIntFromObj(interp, objv[2], &n_شمار)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      بازگشت TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_شمار = n_شمار;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_syscall_errno(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح iCall;
  صحیح iErrno;
  صحیح rc;

  ساختار Errno {
    ثابت نویسه *z;
    صحیح i;
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

  اگر( objc!=4 ){
    Tcl_Wrong_رقم_Args(interp, 2, objv, "SYSCALL ERRNO");
    بازگشت TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[2], aSyscall, اندازه(aSyscall[0]), "system-call", 0, &iCall
  );
  اگر( rc!=TCL_OK ) بازگشت rc;
  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[3], aErrno, اندازه(aErrno[0]), "errno", 0, &iErrno
  );
  اگر( rc!=TCL_OK ) بازگشت rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_syscall_list(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت نویسه *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 2, objv, "");
    بازگشت TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_شمار(pList);
  برای(zSys = pVfs->xNextSystemCall(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->xNextSystemCall(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_SetObjResult(interp, pList);
  Tcl_Decr_Ref_شمار(pList);
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_syscall_defaultvfs(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 2, objv, "");
    بازگشت TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(pVfs->zName, -1));
  بازگشت TCL_OK;
}

ایستا صحیح ts_getpagesize(تهی){
  بازگشت gSyscall.pgsz;
}

ایستا صحیح SQLITE_TCLAPI test_syscall_pagesize(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  صحیح pgsz;
  اگر( objc!=3 ){
    Tcl_Wrong_رقم_Args(interp, 2, objv, "PGSZ");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    بازگشت TCL_ERROR;
  }

  اگر( pgsz<0 ){
    اگر( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }وگرنه{
    اگر( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_AppendResult(interp, "pgsz out of range", پوچ);
      بازگشت TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_syscall(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ساختار SyscallCmd {
    ثابت نویسه *zName;
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
  صحیح iCmd;
  صحیح rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  اگر( objc<2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "SUB-COMMAND ...");
    بازگشت TCL_ERROR;
  }
  اگر( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_AppendResult(interp, "VFS does not support xSetSystemCall", پوچ);
    rc = TCL_ERROR;
  }وگرنه{
    rc = Tcl_GetIndexFromObjStruct(interp, 
        objv[1], aCmd, اندازه(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  اگر( rc!=TCL_OK ) بازگشت rc;
  بازگشت aCmd[iCmd].xCmd(clientData, interp, objc, objv);
}

صحیح SqlitetestSyscall_Init(Tcl_Interp *interp){
  ساختار SyscallCmd {
    ثابت نویسه *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  صحیح i;

  برای(i=0; i<اندازه(aCmd)/اندازه(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  بازگشت TCL_OK;
}
#وگرنه
صحیح SqlitetestSyscall_Init(Tcl_Interp *interp){
  بازگشت TCL_OK;
}
#endif
