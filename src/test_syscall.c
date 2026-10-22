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

#якщо SQLITE_OS_UNIX

/* From main.c */
extern const char *sqlite3ErrName(ціле);

#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

static struct TestSyscallGlobal {
  ціле bPersist;                   /* 1 for persistent errors, 0 for transient */
  ціле nCount;                     /* Fail after this many more calls */
  ціле nFail;                      /* Number of failures that have occurred */
  ціле pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

static ціле ts_open(const char *, ціле, ціле);
static ціле ts_close(ціле fd);
static ціле ts_access(const char *zPath, ціле mode);
static char *ts_getcwd(char *zPath, size_t nPath);
static ціле ts_stat(const char *zPath, struct stat *p);
static ціле ts_fstat(ціле fd, struct stat *p);
static ціле ts_ftruncate(ціле fd, off_t n);
static ціле ts_fcntl(ціле fd, ціле cmd, ... );
static ssize_t ts_read(ціле fd, void *aBuf, size_t nBuf);
static ssize_t ts_pread(ціле fd, void *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
static ssize_t ts_pread64(ціле fd, void *aBuf, size_t nBuf, sqlite3_uint64 off);
static ssize_t ts_write(ціле fd, const void *aBuf, size_t nBuf);
static ssize_t ts_pwrite(ціле fd, const void *aBuf, size_t nBuf, off_t off);
static ssize_t ts_pwrite64(ціле fd, const void *aBuf, size_t nBuf, sqlite3_uint64 off);
static ціле ts_fchmod(ціле fd, mode_t mode);
static ціле ts_fallocate(ціле fd, off_t off, off_t len);
static void *ts_mmap(void *, size_t, ціле, ціле, ціле, off_t);
static void *ts_mremap(void*, size_t, size_t, ціле, ...);

struct TestSyscallArray {
  const char *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  ціле default_errno;              /* Default value for errno following errors */
  ціле custom_errno;               /* Current value for errno if error */
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

#define orig_open      ((ціле(*)(const char *, ціле, ціле))aSyscall[0].xOrig)
#define orig_close     ((ціле(*)(ціле))aSyscall[1].xOrig)
#define orig_access    ((ціле(*)(const char*,ціле))aSyscall[2].xOrig)
#define orig_getcwd    ((char*(*)(char*,size_t))aSyscall[3].xOrig)
#define orig_stat      ((ціле(*)(const char*,struct stat*))aSyscall[4].xOrig)
#define orig_fstat     ((ціле(*)(ціле,struct stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((ціле(*)(ціле,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((ціле(*)(ціле,ціле,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(ціле,void*,size_t))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(ціле,void*,size_t,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(ціле,void*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(ціле,const void*,size_t))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(ціле,const void*,size_t,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(ціле,const void*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((ціле(*)(ціле,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((ціле(*)(ціле,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((void*(*)(void*,size_t,ціле,ціле,ціле,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((void*(*)(void*,size_t,size_t,ціле,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
static ціле tsIsFail(void){
  gSyscall.nCount--;
  якщо( gSyscall.nCount==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    повернути 1;
  }
  повернути 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
static ціле tsErrno(const char *zFunc){
  ціле i;
  size_t nFunc = strlen(zFunc);
  для(i=0; aSyscall[i].zName; i++){
    якщо( strlen(aSyscall[i].zName)!=nFunc ) continue;
    якщо( memcmp(aSyscall[i].zName, zFunc, nFunc) ) continue;
    повернути aSyscall[i].custom_errno;
  }

  assert(0);
  повернути 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
static ціле tsIsFailErrno(const char *zFunc){
  якщо( tsIsFail() ){
    errno = tsErrno(zFunc);
    повернути 1;
  }
  повернути 0;
}

/*
** A wrapper around open().
*/
static ціле ts_open(const char *zFile, ціле flags, ціле mode){
  якщо( tsIsFailErrno("open") ){
    повернути -1;
  }
  повернути orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
static ціле ts_close(ціле fd){
  якщо( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    повернути -1;
  }
  повернути orig_close(fd);
}

/*
** A wrapper around access().
*/
static ціле ts_access(const char *zPath, ціле mode){
  якщо( tsIsFail() ){
    повернути -1;
  }
  повернути orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
static char *ts_getcwd(char *zPath, size_t nPath){
  якщо( tsIsFail() ){
    повернути NULL;
  }
  повернути orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
static ціле ts_stat(const char *zPath, struct stat *p){
  якщо( tsIsFail() ){
    повернути -1;
  }
  повернути orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
static ціле ts_fstat(ціле fd, struct stat *p){
  якщо( tsIsFailErrno("fstat") ){
    повернути -1;
  }
  повернути orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
static ціле ts_ftruncate(ціле fd, off_t n){
  якщо( tsIsFailErrno("ftruncate") ){
    повернути -1;
  }
  повернути orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
static ціле ts_fcntl(ціле fd, ціле cmd, ... ){
  va_list ap;
  void *pArg;
  якщо( tsIsFailErrno("fcntl") ){
    повернути -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, void *);
  повернути orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
static ssize_t ts_read(ціле fd, void *aBuf, size_t nBuf){
  якщо( tsIsFailErrno("read") ){
    повернути -1;
  }
  повернути orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
static ssize_t ts_pread(ціле fd, void *aBuf, size_t nBuf, off_t off){
  якщо( tsIsFailErrno("pread") ){
    повернути -1;
  }
  повернути orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
static ssize_t ts_pread64(ціле fd, void *aBuf, size_t nBuf, sqlite3_uint64 off){
  якщо( tsIsFailErrno("pread64") ){
    повернути -1;
  }
  повернути orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
static ssize_t ts_write(ціле fd, const void *aBuf, size_t nBuf){
  якщо( tsIsFailErrno("write") ){
    якщо( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    повернути -1;
  }
  повернути orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
static ssize_t ts_pwrite(ціле fd, const void *aBuf, size_t nBuf, off_t off){
  якщо( tsIsFailErrno("pwrite") ){
    повернути -1;
  }
  повернути orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
static ssize_t ts_pwrite64(ціле fd, const void *aBuf, size_t nBuf, sqlite3_uint64 off){
  якщо( tsIsFailErrno("pwrite64") ){
    повернути -1;
  }
  повернути orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
static ціле ts_fchmod(ціле fd, mode_t mode){
  якщо( tsIsFail() ){
    повернути -1;
  }
  повернути orig_fchmod(fd, mode);
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
static ціле ts_fallocate(ціле fd, off_t off, off_t len){
  якщо( tsIsFail() ){
    повернути tsErrno("fallocate");
  }
  повернути orig_fallocate(fd, off, len);
}

static void *ts_mmap(
  void *pAddr, 
  size_t nByte, 
  ціле prot, 
  ціле flags, 
  ціле fd, 
  off_t iOff
){
  якщо( tsIsFailErrno("mmap") ){
    повернути MAP_FAILED;
  }
  повернути orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

static void *ts_mremap(void *a, size_t b, size_t c, ціле d, ...){
  якщо( tsIsFailErrno("mremap") ){
    повернути MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  assert( (d & MREMAP_FIXED)==0 );
#endif
  повернути orig_mremap(a, b, c, d);
}

static ціле SQLITE_TCLAPI test_syscall_install(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_Size nElem;
  ціле i;
  Tcl_Obj **apElem;

  якщо( objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "SYSCALL-LIST");
    повернути TCL_ERROR;
  }
  якщо( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    повернути TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  для(i=0; i<(ціле)nElem; i++){
    ціле iCall;
    ціле rc = Tcl_GetIndexFromObjStruct(interp, 
        apElem[i], aSyscall, sizeof(aSyscall[0]), "system-call", 0, &iCall
    );
    якщо( rc ) повернути rc;
    якщо( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  повернути TCL_OK;
}

static ціле SQLITE_TCLAPI test_syscall_uninstall(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  ціле i;

  якщо( objc!=2 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    повернути TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  для(i=0; aSyscall[i].zName; i++){
    якщо( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  повернути TCL_OK;
}

static ціле SQLITE_TCLAPI test_syscall_reset(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  ціле i;
  ціле rc;

  якщо( objc!=2 && objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    повернути TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  якщо( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    для(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }інакше{
    Tcl_Size nFunc;
    char *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    для(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      якщо( strlen(aSyscall[i].zName)!=nFunc ) continue;
      якщо( memcmp(aSyscall[i].zName, zFunc, nFunc) ) continue;
      aSyscall[i].xOrig = 0;
    }
  }
  якщо( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    повернути TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  повернути TCL_OK;
}

static ціле SQLITE_TCLAPI test_syscall_exists(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  якщо( objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    повернути TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(x!=0));
  повернути TCL_OK;
}

static ціле SQLITE_TCLAPI test_syscall_fault(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле nCount = 0;
  ціле bPersist = 0;

  якщо( objc!=2 && objc!=4 ){
    Tcl_WrongNumArgs(interp, 2, objv, "?COUNT PERSIST?");
    повернути TCL_ERROR;
  }

  якщо( objc==4 ){
    якщо( Tcl_GetIntFromObj(interp, objv[2], &nCount)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      повернути TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.nCount = nCount;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  повернути TCL_OK;
}

static ціле SQLITE_TCLAPI test_syscall_errno(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  ціле iCall;
  ціле iErrno;
  ціле rc;

  struct Errno {
    const char *z;
    ціле i;
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

  якщо( objc!=4 ){
    Tcl_WrongNumArgs(interp, 2, objv, "SYSCALL ERRNO");
    повернути TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[2], aSyscall, sizeof(aSyscall[0]), "system-call", 0, &iCall
  );
  якщо( rc!=TCL_OK ) повернути rc;
  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[3], aErrno, sizeof(aErrno[0]), "errno", 0, &iErrno
  );
  якщо( rc!=TCL_OK ) повернути rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  повернути TCL_OK;
}

static ціле SQLITE_TCLAPI test_syscall_list(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  const char *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  якщо( objc!=2 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    повернути TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_IncrRefCount(pList);
  для(zSys = pVfs->xNextSystemCall(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->xNextSystemCall(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_SetObjResult(interp, pList);
  Tcl_DecrRefCount(pList);
  повернути TCL_OK;
}

static ціле SQLITE_TCLAPI test_syscall_defaultvfs(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  якщо( objc!=2 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    повернути TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(pVfs->zName, -1));
  повернути TCL_OK;
}

static ціле ts_getpagesize(void){
  повернути gSyscall.pgsz;
}

static ціле SQLITE_TCLAPI test_syscall_pagesize(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  ціле pgsz;
  якщо( objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "PGSZ");
    повернути TCL_ERROR;
  }
  якщо( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    повернути TCL_ERROR;
  }

  якщо( pgsz<0 ){
    якщо( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }інакше{
    якщо( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_AppendResult(interp, "pgsz out of range", NULL);
      повернути TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  повернути TCL_OK;
}

static ціле SQLITE_TCLAPI test_syscall(
  void * clientData,
  Tcl_Interp *interp,
  ціле objc,
  Tcl_Obj *CONST objv[]
){
  struct SyscallCmd {
    const char *zName;
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
  ціле iCmd;
  ціле rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  якщо( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUB-COMMAND ...");
    повернути TCL_ERROR;
  }
  якщо( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_AppendResult(interp, "VFS does not support xSetSystemCall", NULL);
    rc = TCL_ERROR;
  }інакше{
    rc = Tcl_GetIndexFromObjStruct(interp, 
        objv[1], aCmd, sizeof(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  якщо( rc!=TCL_OK ) повернути rc;
  повернути aCmd[iCmd].xCmd(clientData, interp, objc, objv);
}

ціле SqlitetestSyscall_Init(Tcl_Interp *interp){
  struct SyscallCmd {
    const char *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  ціле i;

  для(i=0; i<sizeof(aCmd)/sizeof(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  повернути TCL_OK;
}
#інакше
ціле SqlitetestSyscall_Init(Tcl_Interp *interp){
  повернути TCL_OK;
}
#endif
