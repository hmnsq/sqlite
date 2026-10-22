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

#නම් SQLITE_OS_UNIX

/* From main.c */
extern const var_na *sqlite3ErrName(පූර්ණ_සංඛ්‍යා);

#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

static sa_nracanaa TestSyscallGlobal {
  පූර්ණ_සංඛ්‍යා bPersist;                   /* 1 for persistent errors, 0 for transient */
  පූර්ණ_සංඛ්‍යා nCount;                     /* Fail after this many more calls */
  පූර්ණ_සංඛ්‍යා nFail;                      /* Number of failures that have occurred */
  පූර්ණ_සංඛ්‍යා pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

static පූර්ණ_සංඛ්‍යා ts_open(const var_na *, පූර්ණ_සංඛ්‍යා, පූර්ණ_සංඛ්‍යා);
static පූර්ණ_සංඛ්‍යා ts_close(පූර්ණ_සංඛ්‍යා fd);
static පූර්ණ_සංඛ්‍යා ts_access(const var_na *zPath, පූර්ණ_සංඛ්‍යා mode);
static var_na *ts_getcwd(var_na *zPath, size_t nPath);
static පූර්ණ_සංඛ්‍යා ts_stat(const var_na *zPath, sa_nracanaa stat *p);
static පූර්ණ_සංඛ්‍යා ts_fstat(පූර්ණ_සංඛ්‍යා fd, sa_nracanaa stat *p);
static පූර්ණ_සංඛ්‍යා ts_ftruncate(පූර්ණ_සංඛ්‍යා fd, off_t n);
static පූර්ණ_සංඛ්‍යා ts_fcntl(පූර්ණ_සංඛ්‍යා fd, පූර්ණ_සංඛ්‍යා cmd, ... );
static ssize_t ts_read(පූර්ණ_සංඛ්‍යා fd, void *aBuf, size_t nBuf);
static ssize_t ts_pread(පූර්ණ_සංඛ්‍යා fd, void *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
static ssize_t ts_pread64(පූර්ණ_සංඛ්‍යා fd, void *aBuf, size_t nBuf, sqlite3_uint64 off);
static ssize_t ts_write(පූර්ණ_සංඛ්‍යා fd, const void *aBuf, size_t nBuf);
static ssize_t ts_pwrite(පූර්ණ_සංඛ්‍යා fd, const void *aBuf, size_t nBuf, off_t off);
static ssize_t ts_pwrite64(පූර්ණ_සංඛ්‍යා fd, const void *aBuf, size_t nBuf, sqlite3_uint64 off);
static පූර්ණ_සංඛ්‍යා ts_fchmod(පූර්ණ_සංඛ්‍යා fd, mode_t mode);
static පූර්ණ_සංඛ්‍යා ts_fallocate(පූර්ණ_සංඛ්‍යා fd, off_t off, off_t len);
static void *ts_mmap(void *, size_t, පූර්ණ_සංඛ්‍යා, පූර්ණ_සංඛ්‍යා, පූර්ණ_සංඛ්‍යා, off_t);
static void *ts_mremap(void*, size_t, size_t, පූර්ණ_සංඛ්‍යා, ...);

sa_nracanaa TestSyscallArray {
  const var_na *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  පූර්ණ_සංඛ්‍යා default_errno;              /* Default value for errno following errors */
  පූර්ණ_සංඛ්‍යා custom_errno;               /* Current value for errno if error */
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

#define orig_open      ((පූර්ණ_සංඛ්‍යා(*)(const var_na *, පූර්ණ_සංඛ්‍යා, පූර්ණ_සංඛ්‍යා))aSyscall[0].xOrig)
#define orig_close     ((පූර්ණ_සංඛ්‍යා(*)(පූර්ණ_සංඛ්‍යා))aSyscall[1].xOrig)
#define orig_access    ((පූර්ණ_සංඛ්‍යා(*)(const var_na*,පූර්ණ_සංඛ්‍යා))aSyscall[2].xOrig)
#define orig_getcwd    ((var_na*(*)(var_na*,size_t))aSyscall[3].xOrig)
#define orig_stat      ((පූර්ණ_සංඛ්‍යා(*)(const var_na*,sa_nracanaa stat*))aSyscall[4].xOrig)
#define orig_fstat     ((පූර්ණ_සංඛ්‍යා(*)(පූර්ණ_සංඛ්‍යා,sa_nracanaa stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((පූර්ණ_සංඛ්‍යා(*)(පූර්ණ_සංඛ්‍යා,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((පූර්ණ_සංඛ්‍යා(*)(පූර්ණ_සංඛ්‍යා,පූර්ණ_සංඛ්‍යා,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(පූර්ණ_සංඛ්‍යා,void*,size_t))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(පූර්ණ_සංඛ්‍යා,void*,size_t,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(පූර්ණ_සංඛ්‍යා,void*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(පූර්ණ_සංඛ්‍යා,const void*,size_t))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(පූර්ණ_සංඛ්‍යා,const void*,size_t,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(පූර්ණ_සංඛ්‍යා,const void*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((පූර්ණ_සංඛ්‍යා(*)(පූර්ණ_සංඛ්‍යා,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((පූර්ණ_සංඛ්‍යා(*)(පූර්ණ_සංඛ්‍යා,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((void*(*)(void*,size_t,පූර්ණ_සංඛ්‍යා,පූර්ණ_සංඛ්‍යා,පූර්ණ_සංඛ්‍යා,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((void*(*)(void*,size_t,size_t,පූර්ණ_සංඛ්‍යා,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
static පූර්ණ_සංඛ්‍යා tsIsFail(void){
  gSyscall.nCount--;
  නම්( gSyscall.nCount==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    ආපසු 1;
  }
  ආපසු 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
static පූර්ණ_සංඛ්‍යා tsErrno(const var_na *zFunc){
  පූර්ණ_සංඛ්‍යා i;
  size_t nFunc = strlen(zFunc);
  සඳහා(i=0; aSyscall[i].zName; i++){
    නම්( strlen(aSyscall[i].zName)!=nFunc ) continue;
    නම්( memcmp(aSyscall[i].zName, zFunc, nFunc) ) continue;
    ආපසු aSyscall[i].custom_errno;
  }

  assert(0);
  ආපසු 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
static පූර්ණ_සංඛ්‍යා tsIsFailErrno(const var_na *zFunc){
  නම්( tsIsFail() ){
    errno = tsErrno(zFunc);
    ආපසු 1;
  }
  ආපසු 0;
}

/*
** A wrapper around open().
*/
static පූර්ණ_සංඛ්‍යා ts_open(const var_na *zFile, පූර්ණ_සංඛ්‍යා flags, පූර්ණ_සංඛ්‍යා mode){
  නම්( tsIsFailErrno("open") ){
    ආපසු -1;
  }
  ආපසු orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
static පූර්ණ_සංඛ්‍යා ts_close(පූර්ණ_සංඛ්‍යා fd){
  නම්( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    ආපසු -1;
  }
  ආපසු orig_close(fd);
}

/*
** A wrapper around access().
*/
static පූර්ණ_සංඛ්‍යා ts_access(const var_na *zPath, පූර්ණ_සංඛ්‍යා mode){
  නම්( tsIsFail() ){
    ආපසු -1;
  }
  ආපසු orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
static var_na *ts_getcwd(var_na *zPath, size_t nPath){
  නම්( tsIsFail() ){
    ආපසු NULL;
  }
  ආපසු orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
static පූර්ණ_සංඛ්‍යා ts_stat(const var_na *zPath, sa_nracanaa stat *p){
  නම්( tsIsFail() ){
    ආපසු -1;
  }
  ආපසු orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
static පූර්ණ_සංඛ්‍යා ts_fstat(පූර්ණ_සංඛ්‍යා fd, sa_nracanaa stat *p){
  නම්( tsIsFailErrno("fstat") ){
    ආපසු -1;
  }
  ආපසු orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
static පූර්ණ_සංඛ්‍යා ts_ftruncate(පූර්ණ_සංඛ්‍යා fd, off_t n){
  නම්( tsIsFailErrno("ftruncate") ){
    ආපසු -1;
  }
  ආපසු orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
static පූර්ණ_සංඛ්‍යා ts_fcntl(පූර්ණ_සංඛ්‍යා fd, පූර්ණ_සංඛ්‍යා cmd, ... ){
  va_list ap;
  void *pArg;
  නම්( tsIsFailErrno("fcntl") ){
    ආපසු -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, void *);
  ආපසු orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
static ssize_t ts_read(පූර්ණ_සංඛ්‍යා fd, void *aBuf, size_t nBuf){
  නම්( tsIsFailErrno("read") ){
    ආපසු -1;
  }
  ආපසු orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
static ssize_t ts_pread(පූර්ණ_සංඛ්‍යා fd, void *aBuf, size_t nBuf, off_t off){
  නම්( tsIsFailErrno("pread") ){
    ආපසු -1;
  }
  ආපසු orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
static ssize_t ts_pread64(පූර්ණ_සංඛ්‍යා fd, void *aBuf, size_t nBuf, sqlite3_uint64 off){
  නම්( tsIsFailErrno("pread64") ){
    ආපසු -1;
  }
  ආපසු orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
static ssize_t ts_write(පූර්ණ_සංඛ්‍යා fd, const void *aBuf, size_t nBuf){
  නම්( tsIsFailErrno("write") ){
    නම්( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    ආපසු -1;
  }
  ආපසු orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
static ssize_t ts_pwrite(පූර්ණ_සංඛ්‍යා fd, const void *aBuf, size_t nBuf, off_t off){
  නම්( tsIsFailErrno("pwrite") ){
    ආපසු -1;
  }
  ආපසු orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
static ssize_t ts_pwrite64(පූර්ණ_සංඛ්‍යා fd, const void *aBuf, size_t nBuf, sqlite3_uint64 off){
  නම්( tsIsFailErrno("pwrite64") ){
    ආපසු -1;
  }
  ආපසු orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
static පූර්ණ_සංඛ්‍යා ts_fchmod(පූර්ණ_සංඛ්‍යා fd, mode_t mode){
  නම්( tsIsFail() ){
    ආපසු -1;
  }
  ආපසු orig_fchmod(fd, mode);
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
static පූර්ණ_සංඛ්‍යා ts_fallocate(පූර්ණ_සංඛ්‍යා fd, off_t off, off_t len){
  නම්( tsIsFail() ){
    ආපසු tsErrno("fallocate");
  }
  ආපසු orig_fallocate(fd, off, len);
}

static void *ts_mmap(
  void *pAddr, 
  size_t nByte, 
  පූර්ණ_සංඛ්‍යා prot, 
  පූර්ණ_සංඛ්‍යා flags, 
  පූර්ණ_සංඛ්‍යා fd, 
  off_t iOff
){
  නම්( tsIsFailErrno("mmap") ){
    ආපසු MAP_FAILED;
  }
  ආපසු orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

static void *ts_mremap(void *a, size_t b, size_t c, පූර්ණ_සංඛ්‍යා d, ...){
  නම්( tsIsFailErrno("mremap") ){
    ආපසු MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  assert( (d & MREMAP_FIXED)==0 );
#endif
  ආපසු orig_mremap(a, b, c, d);
}

static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI test_syscall_install(
  void * clientData,
  Tcl_Interp *interp,
  පූර්ණ_සංඛ්‍යා objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_Size nElem;
  පූර්ණ_සංඛ්‍යා i;
  Tcl_Obj **apElem;

  නම්( objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "SYSCALL-LIST");
    ආපසු TCL_ERROR;
  }
  නම්( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    ආපසු TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  සඳහා(i=0; i<(පූර්ණ_සංඛ්‍යා)nElem; i++){
    පූර්ණ_සංඛ්‍යා iCall;
    පූර්ණ_සංඛ්‍යා rc = Tcl_GetIndexFromObjStruct(interp, 
        apElem[i], aSyscall, parimaa_na(aSyscall[0]), "system-call", 0, &iCall
    );
    නම්( rc ) ආපසු rc;
    නම්( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  ආපසු TCL_OK;
}

static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI test_syscall_uninstall(
  void * clientData,
  Tcl_Interp *interp,
  පූර්ණ_සංඛ්‍යා objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  පූර්ණ_සංඛ්‍යා i;

  නම්( objc!=2 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    ආපසු TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  සඳහා(i=0; aSyscall[i].zName; i++){
    නම්( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  ආපසු TCL_OK;
}

static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI test_syscall_reset(
  void * clientData,
  Tcl_Interp *interp,
  පූර්ණ_සංඛ්‍යා objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  පූර්ණ_සංඛ්‍යා i;
  පූර්ණ_සංඛ්‍යා rc;

  නම්( objc!=2 && objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    ආපසු TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  නම්( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    සඳහා(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }නැතහොත්{
    Tcl_Size nFunc;
    var_na *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    සඳහා(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      නම්( strlen(aSyscall[i].zName)!=nFunc ) continue;
      නම්( memcmp(aSyscall[i].zName, zFunc, nFunc) ) continue;
      aSyscall[i].xOrig = 0;
    }
  }
  නම්( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ආපසු TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  ආපසු TCL_OK;
}

static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI test_syscall_exists(
  void * clientData,
  Tcl_Interp *interp,
  පූර්ණ_සංඛ්‍යා objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  නම්( objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    ආපසු TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(x!=0));
  ආපසු TCL_OK;
}

static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI test_syscall_fault(
  void * clientData,
  Tcl_Interp *interp,
  පූර්ණ_සංඛ්‍යා objc,
  Tcl_Obj *CONST objv[]
){
  පූර්ණ_සංඛ්‍යා nCount = 0;
  පූර්ණ_සංඛ්‍යා bPersist = 0;

  නම්( objc!=2 && objc!=4 ){
    Tcl_WrongNumArgs(interp, 2, objv, "?COUNT PERSIST?");
    ආපසු TCL_ERROR;
  }

  නම්( objc==4 ){
    නම්( Tcl_GetIntFromObj(interp, objv[2], &nCount)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      ආපසු TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.nCount = nCount;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  ආපසු TCL_OK;
}

static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI test_syscall_errno(
  void * clientData,
  Tcl_Interp *interp,
  පූර්ණ_සංඛ්‍යා objc,
  Tcl_Obj *CONST objv[]
){
  පූර්ණ_සංඛ්‍යා iCall;
  පූර්ණ_සංඛ්‍යා iErrno;
  පූර්ණ_සංඛ්‍යා rc;

  sa_nracanaa Errno {
    const var_na *z;
    පූර්ණ_සංඛ්‍යා i;
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

  නම්( objc!=4 ){
    Tcl_WrongNumArgs(interp, 2, objv, "SYSCALL ERRNO");
    ආපසු TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[2], aSyscall, parimaa_na(aSyscall[0]), "system-call", 0, &iCall
  );
  නම්( rc!=TCL_OK ) ආපසු rc;
  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[3], aErrno, parimaa_na(aErrno[0]), "errno", 0, &iErrno
  );
  නම්( rc!=TCL_OK ) ආපසු rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  ආපසු TCL_OK;
}

static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI test_syscall_list(
  void * clientData,
  Tcl_Interp *interp,
  පූර්ණ_සංඛ්‍යා objc,
  Tcl_Obj *CONST objv[]
){
  const var_na *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  නම්( objc!=2 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    ආපසු TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_IncrRefCount(pList);
  සඳහා(zSys = pVfs->xNextSystemCall(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->xNextSystemCall(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_SetObjResult(interp, pList);
  Tcl_DecrRefCount(pList);
  ආපසු TCL_OK;
}

static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI test_syscall_defaultvfs(
  void * clientData,
  Tcl_Interp *interp,
  පූර්ණ_සංඛ්‍යා objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  නම්( objc!=2 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    ආපසු TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(pVfs->zName, -1));
  ආපසු TCL_OK;
}

static පූර්ණ_සංඛ්‍යා ts_getpagesize(void){
  ආපසු gSyscall.pgsz;
}

static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI test_syscall_pagesize(
  void * clientData,
  Tcl_Interp *interp,
  පූර්ණ_සංඛ්‍යා objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  පූර්ණ_සංඛ්‍යා pgsz;
  නම්( objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "PGSZ");
    ආපසු TCL_ERROR;
  }
  නම්( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    ආපසු TCL_ERROR;
  }

  නම්( pgsz<0 ){
    නම්( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }නැතහොත්{
    නම්( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_AppendResult(interp, "pgsz out of range", NULL);
      ආපසු TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  ආපසු TCL_OK;
}

static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI test_syscall(
  void * clientData,
  Tcl_Interp *interp,
  පූර්ණ_සංඛ්‍යා objc,
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
  පූර්ණ_සංඛ්‍යා iCmd;
  පූර්ණ_සංඛ්‍යා rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  නම්( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUB-COMMAND ...");
    ආපසු TCL_ERROR;
  }
  නම්( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_AppendResult(interp, "VFS does not support xSetSystemCall", NULL);
    rc = TCL_ERROR;
  }නැතහොත්{
    rc = Tcl_GetIndexFromObjStruct(interp, 
        objv[1], aCmd, parimaa_na(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  නම්( rc!=TCL_OK ) ආපසු rc;
  ආපසු aCmd[iCmd].xCmd(clientData, interp, objc, objv);
}

පූර්ණ_සංඛ්‍යා SqlitetestSyscall_Init(Tcl_Interp *interp){
  sa_nracanaa SyscallCmd {
    const var_na *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  පූර්ණ_සංඛ්‍යා i;

  සඳහා(i=0; i<parimaa_na(aCmd)/parimaa_na(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  ආපසු TCL_OK;
}
#නැතහොත්
පූර්ණ_සංඛ්‍යා SqlitetestSyscall_Init(Tcl_Interp *interp){
  ආපසු TCL_OK;
}
#endif
