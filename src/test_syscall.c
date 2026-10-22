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
خارجی ثابت حرف *sqlite3ErrName(صحیح);

#شامل <sys/mman.h>
#شامل <sys/types.h>
#شامل <errno.h>

ساکن ساخت TestSyscallGlobal {
  صحیح bPersist;                   /* 1 for persistent errors, 0 for transient */
  صحیح n_گنتی;                     /* Fail after this many more calls */
  صحیح nFail;                      /* Number of failures that have occurred */
  صحیح pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

ساکن صحیح ts_open(ثابت حرف *, صحیح, صحیح);
ساکن صحیح ts_close(صحیح fd);
ساکن صحیح ts_access(ثابت حرف *z_راستہ, صحیح منوال);
ساکن حرف *ts_getcwd(حرف *z_راستہ, size_t n_راستہ);
ساکن صحیح ts_stat(ثابت حرف *z_راستہ, ساخت stat *p);
ساکن صحیح ts_fstat(صحیح fd, ساخت stat *p);
ساکن صحیح ts_ftruncate(صحیح fd, off_t n);
ساکن صحیح ts_fcntl(صحیح fd, صحیح cmd, ... );
ساکن ssize_t ts_read(صحیح fd, خالی *aBuf, size_t nBuf);
ساکن ssize_t ts_pread(صحیح fd, خالی *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
ساکن ssize_t ts_pread64(صحیح fd, خالی *aBuf, size_t nBuf, sqlite3_uint64 off);
ساکن ssize_t ts_write(صحیح fd, ثابت خالی *aBuf, size_t nBuf);
ساکن ssize_t ts_pwrite(صحیح fd, ثابت خالی *aBuf, size_t nBuf, off_t off);
ساکن ssize_t ts_pwrite64(صحیح fd, ثابت خالی *aBuf, size_t nBuf, sqlite3_uint64 off);
ساکن صحیح ts_fchmod(صحیح fd, منوال_t منوال);
ساکن صحیح ts_fallocate(صحیح fd, off_t off, off_t len);
ساکن خالی *ts_mmap(خالی *, size_t, صحیح, صحیح, صحیح, off_t);
ساکن خالی *ts_mremap(خالی*, size_t, size_t, صحیح, ...);

ساخت TestSyscallArray {
  ثابت حرف *zName;
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

#تعریف_کرو orig_open      ((صحیح(*)(ثابت حرف *, صحیح, صحیح))aSyscall[0].xOrig)
#تعریف_کرو orig_close     ((صحیح(*)(صحیح))aSyscall[1].xOrig)
#تعریف_کرو orig_access    ((صحیح(*)(ثابت حرف*,صحیح))aSyscall[2].xOrig)
#تعریف_کرو orig_getcwd    ((حرف*(*)(حرف*,size_t))aSyscall[3].xOrig)
#تعریف_کرو orig_stat      ((صحیح(*)(ثابت حرف*,ساخت stat*))aSyscall[4].xOrig)
#تعریف_کرو orig_fstat     ((صحیح(*)(صحیح,ساخت stat*))aSyscall[5].xOrig)
#تعریف_کرو orig_ftruncate ((صحیح(*)(صحیح,off_t))aSyscall[6].xOrig)
#تعریف_کرو orig_fcntl     ((صحیح(*)(صحیح,صحیح,...))aSyscall[7].xOrig)
#تعریف_کرو orig_read      ((ssize_t(*)(صحیح,خالی*,size_t))aSyscall[8].xOrig)
#تعریف_کرو orig_pread     ((ssize_t(*)(صحیح,خالی*,size_t,off_t))aSyscall[9].xOrig)
#تعریف_کرو orig_pread64   ((ssize_t(*)(صحیح,خالی*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#تعریف_کرو orig_write     ((ssize_t(*)(صحیح,ثابت خالی*,size_t))aSyscall[11].xOrig)
#تعریف_کرو orig_pwrite    ((ssize_t(*)(صحیح,ثابت خالی*,size_t,off_t))\
                       aSyscall[12].xOrig)
#تعریف_کرو orig_pwrite64  ((ssize_t(*)(صحیح,ثابت خالی*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#تعریف_کرو orig_fchmod    ((صحیح(*)(صحیح,منوال_t))aSyscall[14].xOrig)
#تعریف_کرو orig_fallocate ((صحیح(*)(صحیح,off_t,off_t))aSyscall[15].xOrig)
#تعریف_کرو orig_mmap      ((خالی*(*)(خالی*,size_t,صحیح,صحیح,صحیح,off_t))aSyscall[16].xOrig)
#تعریف_کرو orig_mremap    ((خالی*(*)(خالی*,size_t,size_t,صحیح,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
ساکن صحیح tsIsFail(خالی){
  gSyscall.n_گنتی--;
  اگر( gSyscall.n_گنتی==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    واپس 1;
  }
  واپس 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
ساکن صحیح tsErrno(ثابت حرف *zFunc){
  صحیح i;
  size_t nFunc = لڑی_لمبائی(zFunc);
  برائے(i=0; aSyscall[i].zName; i++){
    اگر( لڑی_لمبائی(aSyscall[i].zName)!=nFunc ) جاری;
    اگر( memcmp(aSyscall[i].zName, zFunc, nFunc) ) جاری;
    واپس aSyscall[i].custom_errno;
  }

  أكد(0);
  واپس 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
ساکن صحیح tsIsFailErrno(ثابت حرف *zFunc){
  اگر( tsIsFail() ){
    errno = tsErrno(zFunc);
    واپس 1;
  }
  واپس 0;
}

/*
** A wrapper around open().
*/
ساکن صحیح ts_open(ثابت حرف *zFile, صحیح flags, صحیح منوال){
  اگر( tsIsFailErrno("open") ){
    واپس -1;
  }
  واپس orig_open(zFile, flags, منوال);
}

/*
** A wrapper around close().
*/
ساکن صحیح ts_close(صحیح fd){
  اگر( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    واپس -1;
  }
  واپس orig_close(fd);
}

/*
** A wrapper around access().
*/
ساکن صحیح ts_access(ثابت حرف *z_راستہ, صحیح منوال){
  اگر( tsIsFail() ){
    واپس -1;
  }
  واپس orig_access(z_راستہ, منوال);
}

/*
** A wrapper around getcwd().
*/
ساکن حرف *ts_getcwd(حرف *z_راستہ, size_t n_راستہ){
  اگر( tsIsFail() ){
    واپس تہی;
  }
  واپس orig_getcwd(z_راستہ, n_راستہ);
}

/*
** A wrapper around stat().
*/
ساکن صحیح ts_stat(ثابت حرف *z_راستہ, ساخت stat *p){
  اگر( tsIsFail() ){
    واپس -1;
  }
  واپس orig_stat(z_راستہ, p);
}

/*
** A wrapper around fstat().
*/
ساکن صحیح ts_fstat(صحیح fd, ساخت stat *p){
  اگر( tsIsFailErrno("fstat") ){
    واپس -1;
  }
  واپس orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
ساکن صحیح ts_ftruncate(صحیح fd, off_t n){
  اگر( tsIsFailErrno("ftruncate") ){
    واپس -1;
  }
  واپس orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
ساکن صحیح ts_fcntl(صحیح fd, صحیح cmd, ... ){
  va_list ap;
  خالی *pArg;
  اگر( tsIsFailErrno("fcntl") ){
    واپس -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, خالی *);
  واپس orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
ساکن ssize_t ts_read(صحیح fd, خالی *aBuf, size_t nBuf){
  اگر( tsIsFailErrno("read") ){
    واپس -1;
  }
  واپس orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
ساکن ssize_t ts_pread(صحیح fd, خالی *aBuf, size_t nBuf, off_t off){
  اگر( tsIsFailErrno("pread") ){
    واپس -1;
  }
  واپس orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
ساکن ssize_t ts_pread64(صحیح fd, خالی *aBuf, size_t nBuf, sqlite3_uint64 off){
  اگر( tsIsFailErrno("pread64") ){
    واپس -1;
  }
  واپس orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
ساکن ssize_t ts_write(صحیح fd, ثابت خالی *aBuf, size_t nBuf){
  اگر( tsIsFailErrno("write") ){
    اگر( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    واپس -1;
  }
  واپس orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
ساکن ssize_t ts_pwrite(صحیح fd, ثابت خالی *aBuf, size_t nBuf, off_t off){
  اگر( tsIsFailErrno("pwrite") ){
    واپس -1;
  }
  واپس orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
ساکن ssize_t ts_pwrite64(صحیح fd, ثابت خالی *aBuf, size_t nBuf, sqlite3_uint64 off){
  اگر( tsIsFailErrno("pwrite64") ){
    واپس -1;
  }
  واپس orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
ساکن صحیح ts_fchmod(صحیح fd, منوال_t منوال){
  اگر( tsIsFail() ){
    واپس -1;
  }
  واپس orig_fchmod(fd, منوال);
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
ساکن صحیح ts_fallocate(صحیح fd, off_t off, off_t len){
  اگر( tsIsFail() ){
    واپس tsErrno("fallocate");
  }
  واپس orig_fallocate(fd, off, len);
}

ساکن خالی *ts_mmap(
  خالی *pAddr, 
  size_t nByte, 
  صحیح prot, 
  صحیح flags, 
  صحیح fd, 
  off_t iOff
){
  اگر( tsIsFailErrno("mmap") ){
    واپس MAP_FAILED;
  }
  واپس orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

ساکن خالی *ts_mremap(خالی *a, size_t b, size_t c, صحیح d, ...){
  اگر( tsIsFailErrno("mremap") ){
    واپس MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#اگر_تعریف_شدہ MREMAP_FIXED
  أكد( (d & MREMAP_FIXED)==0 );
#اگر_ختم
  واپس orig_mremap(a, b, c, d);
}

ساکن صحیح SQLITE_TCLAPI test_syscall_install(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_سائز nElem;
  صحیح i;
  Tcl_Obj **apElem;

  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 2, objv, "SYSCALL-LIST");
    واپس TCL_ERROR;
  }
  اگر( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    واپس TCL_ERROR;
  }
  pVfs = sqlite3_vfs_ڈھونڈو(0);

  برائے(i=0; i<(صحیح)nElem; i++){
    صحیح iCall;
    صحیح rc = Tcl_Get_اشاریہ_From_Obj_Struct(interp, 
        apElem[i], aSyscall, حجم(aSyscall[0]), "system-call", 0, &iCall
    );
    اگر( rc ) واپس rc;
    اگر( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->x_سیٹ_System_Call(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_syscall_uninstall(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  صحیح i;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 2, objv, "");
    واپس TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ڈھونڈو(0);
  برائے(i=0; aSyscall[i].zName; i++){
    اگر( aSyscall[i].xOrig ){
      pVfs->x_سیٹ_System_Call(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_syscall_reset(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  صحیح i;
  صحیح rc;

  اگر( objc!=2 && objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 2, objv, "");
    واپس TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ڈھونڈو(0);
  اگر( objc==2 ){
    rc = pVfs->x_سیٹ_System_Call(pVfs, 0, 0);
    برائے(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }ورنہ{
    Tcl_سائز nFunc;
    حرف *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->x_سیٹ_System_Call(pVfs, Tcl_GetString(objv[2]), 0);
    برائے(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      اگر( لڑی_لمبائی(aSyscall[i].zName)!=nFunc ) جاری;
      اگر( memcmp(aSyscall[i].zName, zFunc, nFunc) ) جاری;
      aSyscall[i].xOrig = 0;
    }
  }
  اگر( rc!=SQLITE_OK ){
    Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    واپس TCL_ERROR;
  }

  Tcl_Reset_نتیجہ(interp);
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_syscall_exists(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 2, objv, "");
    واپس TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ڈھونڈو(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewBooleanObj(x!=0));
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_syscall_fault(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح n_گنتی = 0;
  صحیح bPersist = 0;

  اگر( objc!=2 && objc!=4 ){
    Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?COUNT PERSIST?");
    واپس TCL_ERROR;
  }

  اگر( objc==4 ){
    اگر( Tcl_GetIntFromObj(interp, objv[2], &n_گنتی)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      واپس TCL_ERROR;
    }
  }

  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_گنتی = n_گنتی;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_syscall_errno(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  صحیح iCall;
  صحیح iErrno;
  صحیح rc;

  ساخت Errno {
    ثابت حرف *z;
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
    Tcl_Wrong_عدد_دلائل(interp, 2, objv, "SYSCALL ERRNO");
    واپس TCL_ERROR;
  }

  rc = Tcl_Get_اشاریہ_From_Obj_Struct(interp, 
      objv[2], aSyscall, حجم(aSyscall[0]), "system-call", 0, &iCall
  );
  اگر( rc!=TCL_OK ) واپس rc;
  rc = Tcl_Get_اشاریہ_From_Obj_Struct(interp, 
      objv[3], aErrno, حجم(aErrno[0]), "errno", 0, &iErrno
  );
  اگر( rc!=TCL_OK ) واپس rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_syscall_list(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 2, objv, "");
    واپس TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ڈھونڈو(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_گنتی(pList);
  برائے(zSys = pVfs->x_اگلا_System_Call(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->x_اگلا_System_Call(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_سیٹ_Obj_نتیجہ(interp, pList);
  Tcl_Decr_Ref_گنتی(pList);
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_syscall_defaultvfs(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 2, objv, "");
    واپس TCL_ERROR;
  }

  pVfs = sqlite3_vfs_ڈھونڈو(0);
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj(pVfs->zName, -1));
  واپس TCL_OK;
}

ساکن صحیح ts_getpagesize(خالی){
  واپس gSyscall.pgsz;
}

ساکن صحیح SQLITE_TCLAPI test_syscall_pagesize(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_ڈھونڈو(0);
  صحیح pgsz;
  اگر( objc!=3 ){
    Tcl_Wrong_عدد_دلائل(interp, 2, objv, "PGSZ");
    واپس TCL_ERROR;
  }
  اگر( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    واپس TCL_ERROR;
  }

  اگر( pgsz<0 ){
    اگر( gSyscall.orig_getpagesize ){
      pVfs->x_سیٹ_System_Call(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }ورنہ{
    اگر( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_Append_نتیجہ(interp, "pgsz out of range", تہی);
      واپس TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->x_سیٹ_System_Call(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_syscall(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ساخت SyscallCmd {
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
  صحیح iCmd;
  صحیح rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_ڈھونڈو(0);

  اگر( objc<2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "SUB-COMMAND ...");
    واپس TCL_ERROR;
  }
  اگر( pVfs->i_نسخہ<3 || pVfs->x_سیٹ_System_Call==0 ){
    Tcl_Append_نتیجہ(interp, "VFS does not support xSetSystemCall", تہی);
    rc = TCL_ERROR;
  }ورنہ{
    rc = Tcl_Get_اشاریہ_From_Obj_Struct(interp, 
        objv[1], aCmd, حجم(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  اگر( rc!=TCL_OK ) واپس rc;
  واپس aCmd[iCmd].xCmd(client_ڈیٹا, interp, objc, objv);
}

صحیح SqlitetestSyscall_Init(Tcl_Interp *interp){
  ساخت SyscallCmd {
    ثابت حرف *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  صحیح i;

  برائے(i=0; i<حجم(aCmd)/حجم(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  واپس TCL_OK;
}
#ورنہ
صحیح SqlitetestSyscall_Init(Tcl_Interp *interp){
  واپس TCL_OK;
}
#اگر_ختم
