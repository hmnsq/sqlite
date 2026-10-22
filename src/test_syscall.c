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

#include "sqliteInt.h"
#include "sqlite3.h"
#include "tclsqlite.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ប្រសិនបើ SQLITE_OS_UNIX

/* From main.c */
extern const var_na *sqlite3ErrName(ចំនួនគត់);

#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

static sa_nracanaa TestSyscallGlobal {
  ចំនួនគត់ bPersist;                   /* 1 for persistent errors, 0 for transient */
  ចំនួនគត់ nCount;                     /* Fail after this many more calls */
  ចំនួនគត់ nFail;                      /* Number of failures that have occurred */
  ចំនួនគត់ pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

static ចំនួនគត់ ts_open(const var_na *, ចំនួនគត់, ចំនួនគត់);
static ចំនួនគត់ ts_close(ចំនួនគត់ fd);
static ចំនួនគត់ ts_access(const var_na *zPath, ចំនួនគត់ mode);
static var_na *ts_getcwd(var_na *zPath, size_t nPath);
static ចំនួនគត់ ts_stat(const var_na *zPath, sa_nracanaa stat *p);
static ចំនួនគត់ ts_fstat(ចំនួនគត់ fd, sa_nracanaa stat *p);
static ចំនួនគត់ ts_ftruncate(ចំនួនគត់ fd, off_t n);
static ចំនួនគត់ ts_fcntl(ចំនួនគត់ fd, ចំនួនគត់ cmd, ... );
static ssize_t ts_read(ចំនួនគត់ fd, void *aBuf, size_t nBuf);
static ssize_t ts_pread(ចំនួនគត់ fd, void *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
static ssize_t ts_pread64(ចំនួនគត់ fd, void *aBuf, size_t nBuf, sqlite3_uint64 off);
static ssize_t ts_write(ចំនួនគត់ fd, const void *aBuf, size_t nBuf);
static ssize_t ts_pwrite(ចំនួនគត់ fd, const void *aBuf, size_t nBuf, off_t off);
static ssize_t ts_pwrite64(ចំនួនគត់ fd, const void *aBuf, size_t nBuf, sqlite3_uint64 off);
static ចំនួនគត់ ts_fchmod(ចំនួនគត់ fd, mode_t mode);
static ចំនួនគត់ ts_fallocate(ចំនួនគត់ fd, off_t off, off_t len);
static void *ts_mmap(void *, size_t, ចំនួនគត់, ចំនួនគត់, ចំនួនគត់, off_t);
static void *ts_mremap(void*, size_t, size_t, ចំនួនគត់, ...);

sa_nracanaa TestSyscallArray {
  const var_na *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  ចំនួនគត់ default_errno;              /* Default value for errno following errors */
  ចំនួនគត់ custom_errno;               /* Current value for errno if error */
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

#define orig_open      ((ចំនួនគត់(*)(const var_na *, ចំនួនគត់, ចំនួនគត់))aSyscall[0].xOrig)
#define orig_close     ((ចំនួនគត់(*)(ចំនួនគត់))aSyscall[1].xOrig)
#define orig_access    ((ចំនួនគត់(*)(const var_na*,ចំនួនគត់))aSyscall[2].xOrig)
#define orig_getcwd    ((var_na*(*)(var_na*,size_t))aSyscall[3].xOrig)
#define orig_stat      ((ចំនួនគត់(*)(const var_na*,sa_nracanaa stat*))aSyscall[4].xOrig)
#define orig_fstat     ((ចំនួនគត់(*)(ចំនួនគត់,sa_nracanaa stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((ចំនួនគត់(*)(ចំនួនគត់,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((ចំនួនគត់(*)(ចំនួនគត់,ចំនួនគត់,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(ចំនួនគត់,void*,size_t))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(ចំនួនគត់,void*,size_t,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(ចំនួនគត់,void*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(ចំនួនគត់,const void*,size_t))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(ចំនួនគត់,const void*,size_t,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(ចំនួនគត់,const void*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((ចំនួនគត់(*)(ចំនួនគត់,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((ចំនួនគត់(*)(ចំនួនគត់,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((void*(*)(void*,size_t,ចំនួនគត់,ចំនួនគត់,ចំនួនគត់,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((void*(*)(void*,size_t,size_t,ចំនួនគត់,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
static ចំនួនគត់ tsIsFail(void){
  gSyscall.nCount--;
  ប្រសិនបើ( gSyscall.nCount==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    ត្រឡប់ 1;
  }
  ត្រឡប់ 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
static ចំនួនគត់ tsErrno(const var_na *zFunc){
  ចំនួនគត់ i;
  size_t nFunc = strlen(zFunc);
  សម្រាប់(i=0; aSyscall[i].zName; i++){
    ប្រសិនបើ( strlen(aSyscall[i].zName)!=nFunc ) continue;
    ប្រសិនបើ( memcmp(aSyscall[i].zName, zFunc, nFunc) ) continue;
    ត្រឡប់ aSyscall[i].custom_errno;
  }

  assert(0);
  ត្រឡប់ 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
static ចំនួនគត់ tsIsFailErrno(const var_na *zFunc){
  ប្រសិនបើ( tsIsFail() ){
    errno = tsErrno(zFunc);
    ត្រឡប់ 1;
  }
  ត្រឡប់ 0;
}

/*
** A wrapper around open().
*/
static ចំនួនគត់ ts_open(const var_na *zFile, ចំនួនគត់ flags, ចំនួនគត់ mode){
  ប្រសិនបើ( tsIsFailErrno("open") ){
    ត្រឡប់ -1;
  }
  ត្រឡប់ orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
static ចំនួនគត់ ts_close(ចំនួនគត់ fd){
  ប្រសិនបើ( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    ត្រឡប់ -1;
  }
  ត្រឡប់ orig_close(fd);
}

/*
** A wrapper around access().
*/
static ចំនួនគត់ ts_access(const var_na *zPath, ចំនួនគត់ mode){
  ប្រសិនបើ( tsIsFail() ){
    ត្រឡប់ -1;
  }
  ត្រឡប់ orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
static var_na *ts_getcwd(var_na *zPath, size_t nPath){
  ប្រសិនបើ( tsIsFail() ){
    ត្រឡប់ NULL;
  }
  ត្រឡប់ orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
static ចំនួនគត់ ts_stat(const var_na *zPath, sa_nracanaa stat *p){
  ប្រសិនបើ( tsIsFail() ){
    ត្រឡប់ -1;
  }
  ត្រឡប់ orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
static ចំនួនគត់ ts_fstat(ចំនួនគត់ fd, sa_nracanaa stat *p){
  ប្រសិនបើ( tsIsFailErrno("fstat") ){
    ត្រឡប់ -1;
  }
  ត្រឡប់ orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
static ចំនួនគត់ ts_ftruncate(ចំនួនគត់ fd, off_t n){
  ប្រសិនបើ( tsIsFailErrno("ftruncate") ){
    ត្រឡប់ -1;
  }
  ត្រឡប់ orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
static ចំនួនគត់ ts_fcntl(ចំនួនគត់ fd, ចំនួនគត់ cmd, ... ){
  va_list ap;
  void *pArg;
  ប្រសិនបើ( tsIsFailErrno("fcntl") ){
    ត្រឡប់ -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, void *);
  ត្រឡប់ orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
static ssize_t ts_read(ចំនួនគត់ fd, void *aBuf, size_t nBuf){
  ប្រសិនបើ( tsIsFailErrno("read") ){
    ត្រឡប់ -1;
  }
  ត្រឡប់ orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
static ssize_t ts_pread(ចំនួនគត់ fd, void *aBuf, size_t nBuf, off_t off){
  ប្រសិនបើ( tsIsFailErrno("pread") ){
    ត្រឡប់ -1;
  }
  ត្រឡប់ orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
static ssize_t ts_pread64(ចំនួនគត់ fd, void *aBuf, size_t nBuf, sqlite3_uint64 off){
  ប្រសិនបើ( tsIsFailErrno("pread64") ){
    ត្រឡប់ -1;
  }
  ត្រឡប់ orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
static ssize_t ts_write(ចំនួនគត់ fd, const void *aBuf, size_t nBuf){
  ប្រសិនបើ( tsIsFailErrno("write") ){
    ប្រសិនបើ( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    ត្រឡប់ -1;
  }
  ត្រឡប់ orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
static ssize_t ts_pwrite(ចំនួនគត់ fd, const void *aBuf, size_t nBuf, off_t off){
  ប្រសិនបើ( tsIsFailErrno("pwrite") ){
    ត្រឡប់ -1;
  }
  ត្រឡប់ orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
static ssize_t ts_pwrite64(ចំនួនគត់ fd, const void *aBuf, size_t nBuf, sqlite3_uint64 off){
  ប្រសិនបើ( tsIsFailErrno("pwrite64") ){
    ត្រឡប់ -1;
  }
  ត្រឡប់ orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
static ចំនួនគត់ ts_fchmod(ចំនួនគត់ fd, mode_t mode){
  ប្រសិនបើ( tsIsFail() ){
    ត្រឡប់ -1;
  }
  ត្រឡប់ orig_fchmod(fd, mode);
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
static ចំនួនគត់ ts_fallocate(ចំនួនគត់ fd, off_t off, off_t len){
  ប្រសិនបើ( tsIsFail() ){
    ត្រឡប់ tsErrno("fallocate");
  }
  ត្រឡប់ orig_fallocate(fd, off, len);
}

static void *ts_mmap(
  void *pAddr, 
  size_t nByte, 
  ចំនួនគត់ prot, 
  ចំនួនគត់ flags, 
  ចំនួនគត់ fd, 
  off_t iOff
){
  ប្រសិនបើ( tsIsFailErrno("mmap") ){
    ត្រឡប់ MAP_FAILED;
  }
  ត្រឡប់ orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

static void *ts_mremap(void *a, size_t b, size_t c, ចំនួនគត់ d, ...){
  ប្រសិនបើ( tsIsFailErrno("mremap") ){
    ត្រឡប់ MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  assert( (d & MREMAP_FIXED)==0 );
#endif
  ត្រឡប់ orig_mremap(a, b, c, d);
}

static ចំនួនគត់ SQLITE_TCLAPI test_syscall_install(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_Size nElem;
  ចំនួនគត់ i;
  Tcl_Obj **apElem;

  ប្រសិនបើ( objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "SYSCALL-LIST");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    ត្រឡប់ TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  សម្រាប់(i=0; i<(ចំនួនគត់)nElem; i++){
    ចំនួនគត់ iCall;
    ចំនួនគត់ rc = Tcl_GetIndexFromObjStruct(interp, 
        apElem[i], aSyscall, parimaa_na(aSyscall[0]), "system-call", 0, &iCall
    );
    ប្រសិនបើ( rc ) ត្រឡប់ rc;
    ប្រសិនបើ( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  ត្រឡប់ TCL_OK;
}

static ចំនួនគត់ SQLITE_TCLAPI test_syscall_uninstall(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  ចំនួនគត់ i;

  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    ត្រឡប់ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  សម្រាប់(i=0; aSyscall[i].zName; i++){
    ប្រសិនបើ( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  ត្រឡប់ TCL_OK;
}

static ចំនួនគត់ SQLITE_TCLAPI test_syscall_reset(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  ចំនួនគត់ i;
  ចំនួនគត់ rc;

  ប្រសិនបើ( objc!=2 && objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    ត្រឡប់ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  ប្រសិនបើ( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    សម្រាប់(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }បើមិនដូច្នេះ{
    Tcl_Size nFunc;
    var_na *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    សម្រាប់(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      ប្រសិនបើ( strlen(aSyscall[i].zName)!=nFunc ) continue;
      ប្រសិនបើ( memcmp(aSyscall[i].zName, zFunc, nFunc) ) continue;
      aSyscall[i].xOrig = 0;
    }
  }
  ប្រសិនបើ( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ត្រឡប់ TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  ត្រឡប់ TCL_OK;
}

static ចំនួនគត់ SQLITE_TCLAPI test_syscall_exists(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  ប្រសិនបើ( objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    ត្រឡប់ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(x!=0));
  ត្រឡប់ TCL_OK;
}

static ចំនួនគត់ SQLITE_TCLAPI test_syscall_fault(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  ចំនួនគត់ nCount = 0;
  ចំនួនគត់ bPersist = 0;

  ប្រសិនបើ( objc!=2 && objc!=4 ){
    Tcl_WrongNumArgs(interp, 2, objv, "?COUNT PERSIST?");
    ត្រឡប់ TCL_ERROR;
  }

  ប្រសិនបើ( objc==4 ){
    ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[2], &nCount)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      ត្រឡប់ TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.nCount = nCount;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  ត្រឡប់ TCL_OK;
}

static ចំនួនគត់ SQLITE_TCLAPI test_syscall_errno(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  ចំនួនគត់ iCall;
  ចំនួនគត់ iErrno;
  ចំនួនគត់ rc;

  sa_nracanaa Errno {
    const var_na *z;
    ចំនួនគត់ i;
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

  ប្រសិនបើ( objc!=4 ){
    Tcl_WrongNumArgs(interp, 2, objv, "SYSCALL ERRNO");
    ត្រឡប់ TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[2], aSyscall, parimaa_na(aSyscall[0]), "system-call", 0, &iCall
  );
  ប្រសិនបើ( rc!=TCL_OK ) ត្រឡប់ rc;
  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[3], aErrno, parimaa_na(aErrno[0]), "errno", 0, &iErrno
  );
  ប្រសិនបើ( rc!=TCL_OK ) ត្រឡប់ rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  ត្រឡប់ TCL_OK;
}

static ចំនួនគត់ SQLITE_TCLAPI test_syscall_list(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  const var_na *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    ត្រឡប់ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_IncrRefCount(pList);
  សម្រាប់(zSys = pVfs->xNextSystemCall(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->xNextSystemCall(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_SetObjResult(interp, pList);
  Tcl_DecrRefCount(pList);
  ត្រឡប់ TCL_OK;
}

static ចំនួនគត់ SQLITE_TCLAPI test_syscall_defaultvfs(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  ប្រសិនបើ( objc!=2 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    ត្រឡប់ TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(pVfs->zName, -1));
  ត្រឡប់ TCL_OK;
}

static ចំនួនគត់ ts_getpagesize(void){
  ត្រឡប់ gSyscall.pgsz;
}

static ចំនួនគត់ SQLITE_TCLAPI test_syscall_pagesize(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  ចំនួនគត់ pgsz;
  ប្រសិនបើ( objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "PGSZ");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    ត្រឡប់ TCL_ERROR;
  }

  ប្រសិនបើ( pgsz<0 ){
    ប្រសិនបើ( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }បើមិនដូច្នេះ{
    ប្រសិនបើ( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_AppendResult(interp, "pgsz out of range", NULL);
      ត្រឡប់ TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  ត្រឡប់ TCL_OK;
}

static ចំនួនគត់ SQLITE_TCLAPI test_syscall(
  void * clientData,
  Tcl_Interp *interp,
  ចំនួនគត់ objc,
  Tcl_Obj *CONST objv[]
){
  sa_nracanaa SyscallCmd {
    const var_na *zName;
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
  ចំនួនគត់ iCmd;
  ចំនួនគត់ rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  ប្រសិនបើ( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUB-COMMAND ...");
    ត្រឡប់ TCL_ERROR;
  }
  ប្រសិនបើ( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_AppendResult(interp, "VFS does not support xSetSystemCall", NULL);
    rc = TCL_ERROR;
  }បើមិនដូច្នេះ{
    rc = Tcl_GetIndexFromObjStruct(interp, 
        objv[1], aCmd, parimaa_na(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  ប្រសិនបើ( rc!=TCL_OK ) ត្រឡប់ rc;
  ត្រឡប់ aCmd[iCmd].xCmd(clientData, interp, objc, objv);
}

ចំនួនគត់ SqlitetestSyscall_Init(Tcl_Interp *interp){
  sa_nracanaa SyscallCmd {
    const var_na *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  ចំនួនគត់ i;

  សម្រាប់(i=0; i<parimaa_na(aCmd)/parimaa_na(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  ត្រឡប់ TCL_OK;
}
#បើមិនដូច្នេះ
ចំនួនគត់ SqlitetestSyscall_Init(Tcl_Interp *interp){
  ត្រឡប់ TCL_OK;
}
#endif
