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

#ضمن "sqliteInt.h"
#ضمن "sqlite3.h"
#ضمن "tclsqlite.h"
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <assert.h>

#إذا SQLITE_OS_UNIX

/* From main.c */
خارجي ثابت حرف *sqlite3ErrName(صحيح);

#ضمن <sys/mman.h>
#ضمن <sys/types.h>
#ضمن <errno.h>

ساكن بنية TestSyscallGlobal {
  صحيح bPersist;                   /* 1 for persistent errors, 0 for transient */
  صحيح n_عدد;                     /* Fail after this many more calls */
  صحيح nFail;                      /* Number of failures that have occurred */
  صحيح pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

ساكن صحيح ts_open(ثابت حرف *, صحيح, صحيح);
ساكن صحيح ts_close(صحيح fd);
ساكن صحيح ts_access(ثابت حرف *zPath, صحيح mode);
ساكن حرف *ts_getcwd(حرف *zPath, size_t nPath);
ساكن صحيح ts_stat(ثابت حرف *zPath, بنية stat *p);
ساكن صحيح ts_fstat(صحيح fd, بنية stat *p);
ساكن صحيح ts_ftruncate(صحيح fd, off_t n);
ساكن صحيح ts_fcntl(صحيح fd, صحيح cmd, ... );
ساكن ssize_t ts_read(صحيح fd, فراغ *aBuf, size_t nBuf);
ساكن ssize_t ts_pread(صحيح fd, فراغ *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
ساكن ssize_t ts_pread64(صحيح fd, فراغ *aBuf, size_t nBuf, sqlite3_uint64 off);
ساكن ssize_t ts_write(صحيح fd, ثابت فراغ *aBuf, size_t nBuf);
ساكن ssize_t ts_pwrite(صحيح fd, ثابت فراغ *aBuf, size_t nBuf, off_t off);
ساكن ssize_t ts_pwrite64(صحيح fd, ثابت فراغ *aBuf, size_t nBuf, sqlite3_uint64 off);
ساكن صحيح ts_fchmod(صحيح fd, mode_t mode);
ساكن صحيح ts_fallocate(صحيح fd, off_t off, off_t len);
ساكن فراغ *ts_mmap(فراغ *, size_t, صحيح, صحيح, صحيح, off_t);
ساكن فراغ *ts_mremap(فراغ*, size_t, size_t, صحيح, ...);

بنية TestSyscallArray {
  ثابت حرف *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  صحيح default_errno;              /* Default value for errno following errors */
  صحيح custom_errno;               /* Current value for errno if error */
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

#define orig_open      ((صحيح(*)(ثابت حرف *, صحيح, صحيح))aSyscall[0].xOrig)
#define orig_close     ((صحيح(*)(صحيح))aSyscall[1].xOrig)
#define orig_access    ((صحيح(*)(ثابت حرف*,صحيح))aSyscall[2].xOrig)
#define orig_getcwd    ((حرف*(*)(حرف*,size_t))aSyscall[3].xOrig)
#define orig_stat      ((صحيح(*)(ثابت حرف*,بنية stat*))aSyscall[4].xOrig)
#define orig_fstat     ((صحيح(*)(صحيح,بنية stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((صحيح(*)(صحيح,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((صحيح(*)(صحيح,صحيح,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(صحيح,فراغ*,size_t))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(صحيح,فراغ*,size_t,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(صحيح,فراغ*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(صحيح,ثابت فراغ*,size_t))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(صحيح,ثابت فراغ*,size_t,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(صحيح,ثابت فراغ*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((صحيح(*)(صحيح,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((صحيح(*)(صحيح,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((فراغ*(*)(فراغ*,size_t,صحيح,صحيح,صحيح,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((فراغ*(*)(فراغ*,size_t,size_t,صحيح,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
ساكن صحيح tsIsFail(فراغ){
  gSyscall.n_عدد--;
  إذا( gSyscall.n_عدد==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    أرجع 1;
  }
  أرجع 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
ساكن صحيح tsErrno(ثابت حرف *zFunc){
  صحيح i;
  size_t nFunc = strlen(zFunc);
  لكل(i=0; aSyscall[i].zName; i++){
    إذا( strlen(aSyscall[i].zName)!=nFunc ) استمر;
    إذا( memcmp(aSyscall[i].zName, zFunc, nFunc) ) استمر;
    أرجع aSyscall[i].custom_errno;
  }

  أكد(0);
  أرجع 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
ساكن صحيح tsIsFailErrno(ثابت حرف *zFunc){
  إذا( tsIsFail() ){
    errno = tsErrno(zFunc);
    أرجع 1;
  }
  أرجع 0;
}

/*
** A wrapper around open().
*/
ساكن صحيح ts_open(ثابت حرف *zFile, صحيح flags, صحيح mode){
  إذا( tsIsFailErrno("open") ){
    أرجع -1;
  }
  أرجع orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
ساكن صحيح ts_close(صحيح fd){
  إذا( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    أرجع -1;
  }
  أرجع orig_close(fd);
}

/*
** A wrapper around access().
*/
ساكن صحيح ts_access(ثابت حرف *zPath, صحيح mode){
  إذا( tsIsFail() ){
    أرجع -1;
  }
  أرجع orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
ساكن حرف *ts_getcwd(حرف *zPath, size_t nPath){
  إذا( tsIsFail() ){
    أرجع عدم;
  }
  أرجع orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
ساكن صحيح ts_stat(ثابت حرف *zPath, بنية stat *p){
  إذا( tsIsFail() ){
    أرجع -1;
  }
  أرجع orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
ساكن صحيح ts_fstat(صحيح fd, بنية stat *p){
  إذا( tsIsFailErrno("fstat") ){
    أرجع -1;
  }
  أرجع orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
ساكن صحيح ts_ftruncate(صحيح fd, off_t n){
  إذا( tsIsFailErrno("ftruncate") ){
    أرجع -1;
  }
  أرجع orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
ساكن صحيح ts_fcntl(صحيح fd, صحيح cmd, ... ){
  va_list ap;
  فراغ *pArg;
  إذا( tsIsFailErrno("fcntl") ){
    أرجع -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, فراغ *);
  أرجع orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
ساكن ssize_t ts_read(صحيح fd, فراغ *aBuf, size_t nBuf){
  إذا( tsIsFailErrno("read") ){
    أرجع -1;
  }
  أرجع orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
ساكن ssize_t ts_pread(صحيح fd, فراغ *aBuf, size_t nBuf, off_t off){
  إذا( tsIsFailErrno("pread") ){
    أرجع -1;
  }
  أرجع orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
ساكن ssize_t ts_pread64(صحيح fd, فراغ *aBuf, size_t nBuf, sqlite3_uint64 off){
  إذا( tsIsFailErrno("pread64") ){
    أرجع -1;
  }
  أرجع orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
ساكن ssize_t ts_write(صحيح fd, ثابت فراغ *aBuf, size_t nBuf){
  إذا( tsIsFailErrno("write") ){
    إذا( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    أرجع -1;
  }
  أرجع orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
ساكن ssize_t ts_pwrite(صحيح fd, ثابت فراغ *aBuf, size_t nBuf, off_t off){
  إذا( tsIsFailErrno("pwrite") ){
    أرجع -1;
  }
  أرجع orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
ساكن ssize_t ts_pwrite64(صحيح fd, ثابت فراغ *aBuf, size_t nBuf, sqlite3_uint64 off){
  إذا( tsIsFailErrno("pwrite64") ){
    أرجع -1;
  }
  أرجع orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
ساكن صحيح ts_fchmod(صحيح fd, mode_t mode){
  إذا( tsIsFail() ){
    أرجع -1;
  }
  أرجع orig_fchmod(fd, mode);
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
ساكن صحيح ts_fallocate(صحيح fd, off_t off, off_t len){
  إذا( tsIsFail() ){
    أرجع tsErrno("fallocate");
  }
  أرجع orig_fallocate(fd, off, len);
}

ساكن فراغ *ts_mmap(
  فراغ *pAddr, 
  size_t nByte, 
  صحيح prot, 
  صحيح flags, 
  صحيح fd, 
  off_t iOff
){
  إذا( tsIsFailErrno("mmap") ){
    أرجع MAP_FAILED;
  }
  أرجع orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

ساكن فراغ *ts_mremap(فراغ *a, size_t b, size_t c, صحيح d, ...){
  إذا( tsIsFailErrno("mremap") ){
    أرجع MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  أكد( (d & MREMAP_FIXED)==0 );
#endif
  أرجع orig_mremap(a, b, c, d);
}

ساكن صحيح SQLITE_TCLAPI test_syscall_install(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_Size nElem;
  صحيح i;
  Tcl_Obj **apElem;

  إذا( objc!=3 ){
    Tcl_Wrong_رقم_وسائط(interp, 2, objv, "SYSCALL-LIST");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    أرجع TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  لكل(i=0; i<(صحيح)nElem; i++){
    صحيح iCall;
    صحيح rc = Tcl_GetIndexFromObjStruct(interp, 
        apElem[i], aSyscall, حجم(aSyscall[0]), "system-call", 0, &iCall
    );
    إذا( rc ) أرجع rc;
    إذا( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_syscall_uninstall(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  صحيح i;

  إذا( objc!=2 ){
    Tcl_Wrong_رقم_وسائط(interp, 2, objv, "");
    أرجع TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  لكل(i=0; aSyscall[i].zName; i++){
    إذا( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_syscall_reset(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  صحيح i;
  صحيح rc;

  إذا( objc!=2 && objc!=3 ){
    Tcl_Wrong_رقم_وسائط(interp, 2, objv, "");
    أرجع TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  إذا( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    لكل(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }وإلا{
    Tcl_Size nFunc;
    حرف *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    لكل(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      إذا( strlen(aSyscall[i].zName)!=nFunc ) استمر;
      إذا( memcmp(aSyscall[i].zName, zFunc, nFunc) ) استمر;
      aSyscall[i].xOrig = 0;
    }
  }
  إذا( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    أرجع TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_syscall_exists(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  إذا( objc!=3 ){
    Tcl_Wrong_رقم_وسائط(interp, 2, objv, "");
    أرجع TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(x!=0));
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_syscall_fault(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح n_عدد = 0;
  صحيح bPersist = 0;

  إذا( objc!=2 && objc!=4 ){
    Tcl_Wrong_رقم_وسائط(interp, 2, objv, "?COUNT PERSIST?");
    أرجع TCL_ERROR;
  }

  إذا( objc==4 ){
    إذا( Tcl_GetIntFromObj(interp, objv[2], &n_عدد)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      أرجع TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_عدد = n_عدد;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_syscall_errno(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  صحيح iCall;
  صحيح iErrno;
  صحيح rc;

  بنية Errno {
    ثابت حرف *z;
    صحيح i;
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

  إذا( objc!=4 ){
    Tcl_Wrong_رقم_وسائط(interp, 2, objv, "SYSCALL ERRNO");
    أرجع TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[2], aSyscall, حجم(aSyscall[0]), "system-call", 0, &iCall
  );
  إذا( rc!=TCL_OK ) أرجع rc;
  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[3], aErrno, حجم(aErrno[0]), "errno", 0, &iErrno
  );
  إذا( rc!=TCL_OK ) أرجع rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_syscall_list(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  إذا( objc!=2 ){
    Tcl_Wrong_رقم_وسائط(interp, 2, objv, "");
    أرجع TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_عدد(pList);
  لكل(zSys = pVfs->xNextSystemCall(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->xNextSystemCall(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_SetObjResult(interp, pList);
  Tcl_Decr_Ref_عدد(pList);
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_syscall_defaultvfs(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  إذا( objc!=2 ){
    Tcl_Wrong_رقم_وسائط(interp, 2, objv, "");
    أرجع TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(pVfs->zName, -1));
  أرجع TCL_OK;
}

ساكن صحيح ts_getpagesize(فراغ){
  أرجع gSyscall.pgsz;
}

ساكن صحيح SQLITE_TCLAPI test_syscall_pagesize(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  صحيح pgsz;
  إذا( objc!=3 ){
    Tcl_Wrong_رقم_وسائط(interp, 2, objv, "PGSZ");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    أرجع TCL_ERROR;
  }

  إذا( pgsz<0 ){
    إذا( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }وإلا{
    إذا( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_AppendResult(interp, "pgsz out of range", عدم);
      أرجع TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_syscall(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  بنية SyscallCmd {
    ثابت حرف *zName;
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
  صحيح iCmd;
  صحيح rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  إذا( objc<2 ){
    Tcl_Wrong_رقم_وسائط(interp, 1, objv, "SUB-COMMAND ...");
    أرجع TCL_ERROR;
  }
  إذا( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_AppendResult(interp, "VFS does not support xSetSystemCall", عدم);
    rc = TCL_ERROR;
  }وإلا{
    rc = Tcl_GetIndexFromObjStruct(interp, 
        objv[1], aCmd, حجم(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  إذا( rc!=TCL_OK ) أرجع rc;
  أرجع aCmd[iCmd].xCmd(clientData, interp, objc, objv);
}

صحيح SqlitetestSyscall_Init(Tcl_Interp *interp){
  بنية SyscallCmd {
    ثابت حرف *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  صحيح i;

  لكل(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  أرجع TCL_OK;
}
#وإلا
صحيح SqlitetestSyscall_Init(Tcl_Interp *interp){
  أرجع TCL_OK;
}
#endif
