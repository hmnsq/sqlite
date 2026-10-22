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

#만약 SQLITE_OS_UNIX

/* From main.c */
extern const char *sqlite3ErrName(정수);

#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

static struct TestSyscallGlobal {
  정수 bPersist;                   /* 1 for persistent errors, 0 for transient */
  정수 nCount;                     /* Fail after this many more calls */
  정수 nFail;                      /* Number of failures that have occurred */
  정수 pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

static 정수 ts_open(const char *, 정수, 정수);
static 정수 ts_close(정수 fd);
static 정수 ts_access(const char *zPath, 정수 mode);
static char *ts_getcwd(char *zPath, size_t nPath);
static 정수 ts_stat(const char *zPath, struct stat *p);
static 정수 ts_fstat(정수 fd, struct stat *p);
static 정수 ts_ftruncate(정수 fd, off_t n);
static 정수 ts_fcntl(정수 fd, 정수 cmd, ... );
static ssize_t ts_read(정수 fd, void *aBuf, size_t nBuf);
static ssize_t ts_pread(정수 fd, void *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
static ssize_t ts_pread64(정수 fd, void *aBuf, size_t nBuf, sqlite3_uint64 off);
static ssize_t ts_write(정수 fd, const void *aBuf, size_t nBuf);
static ssize_t ts_pwrite(정수 fd, const void *aBuf, size_t nBuf, off_t off);
static ssize_t ts_pwrite64(정수 fd, const void *aBuf, size_t nBuf, sqlite3_uint64 off);
static 정수 ts_fchmod(정수 fd, mode_t mode);
static 정수 ts_fallocate(정수 fd, off_t off, off_t len);
static void *ts_mmap(void *, size_t, 정수, 정수, 정수, off_t);
static void *ts_mremap(void*, size_t, size_t, 정수, ...);

struct TestSyscallArray {
  const char *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  정수 default_errno;              /* Default value for errno following errors */
  정수 custom_errno;               /* Current value for errno if error */
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

#define orig_open      ((정수(*)(const char *, 정수, 정수))aSyscall[0].xOrig)
#define orig_close     ((정수(*)(정수))aSyscall[1].xOrig)
#define orig_access    ((정수(*)(const char*,정수))aSyscall[2].xOrig)
#define orig_getcwd    ((char*(*)(char*,size_t))aSyscall[3].xOrig)
#define orig_stat      ((정수(*)(const char*,struct stat*))aSyscall[4].xOrig)
#define orig_fstat     ((정수(*)(정수,struct stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((정수(*)(정수,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((정수(*)(정수,정수,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(정수,void*,size_t))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(정수,void*,size_t,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(정수,void*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(정수,const void*,size_t))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(정수,const void*,size_t,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(정수,const void*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((정수(*)(정수,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((정수(*)(정수,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((void*(*)(void*,size_t,정수,정수,정수,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((void*(*)(void*,size_t,size_t,정수,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
static 정수 tsIsFail(void){
  gSyscall.nCount--;
  만약( gSyscall.nCount==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    반환 1;
  }
  반환 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
static 정수 tsErrno(const char *zFunc){
  정수 i;
  size_t nFunc = strlen(zFunc);
  위해(i=0; aSyscall[i].zName; i++){
    만약( strlen(aSyscall[i].zName)!=nFunc ) continue;
    만약( memcmp(aSyscall[i].zName, zFunc, nFunc) ) continue;
    반환 aSyscall[i].custom_errno;
  }

  assert(0);
  반환 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
static 정수 tsIsFailErrno(const char *zFunc){
  만약( tsIsFail() ){
    errno = tsErrno(zFunc);
    반환 1;
  }
  반환 0;
}

/*
** A wrapper around open().
*/
static 정수 ts_open(const char *zFile, 정수 flags, 정수 mode){
  만약( tsIsFailErrno("open") ){
    반환 -1;
  }
  반환 orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
static 정수 ts_close(정수 fd){
  만약( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    반환 -1;
  }
  반환 orig_close(fd);
}

/*
** A wrapper around access().
*/
static 정수 ts_access(const char *zPath, 정수 mode){
  만약( tsIsFail() ){
    반환 -1;
  }
  반환 orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
static char *ts_getcwd(char *zPath, size_t nPath){
  만약( tsIsFail() ){
    반환 NULL;
  }
  반환 orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
static 정수 ts_stat(const char *zPath, struct stat *p){
  만약( tsIsFail() ){
    반환 -1;
  }
  반환 orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
static 정수 ts_fstat(정수 fd, struct stat *p){
  만약( tsIsFailErrno("fstat") ){
    반환 -1;
  }
  반환 orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
static 정수 ts_ftruncate(정수 fd, off_t n){
  만약( tsIsFailErrno("ftruncate") ){
    반환 -1;
  }
  반환 orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
static 정수 ts_fcntl(정수 fd, 정수 cmd, ... ){
  va_list ap;
  void *pArg;
  만약( tsIsFailErrno("fcntl") ){
    반환 -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, void *);
  반환 orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
static ssize_t ts_read(정수 fd, void *aBuf, size_t nBuf){
  만약( tsIsFailErrno("read") ){
    반환 -1;
  }
  반환 orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
static ssize_t ts_pread(정수 fd, void *aBuf, size_t nBuf, off_t off){
  만약( tsIsFailErrno("pread") ){
    반환 -1;
  }
  반환 orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
static ssize_t ts_pread64(정수 fd, void *aBuf, size_t nBuf, sqlite3_uint64 off){
  만약( tsIsFailErrno("pread64") ){
    반환 -1;
  }
  반환 orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
static ssize_t ts_write(정수 fd, const void *aBuf, size_t nBuf){
  만약( tsIsFailErrno("write") ){
    만약( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    반환 -1;
  }
  반환 orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
static ssize_t ts_pwrite(정수 fd, const void *aBuf, size_t nBuf, off_t off){
  만약( tsIsFailErrno("pwrite") ){
    반환 -1;
  }
  반환 orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
static ssize_t ts_pwrite64(정수 fd, const void *aBuf, size_t nBuf, sqlite3_uint64 off){
  만약( tsIsFailErrno("pwrite64") ){
    반환 -1;
  }
  반환 orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
static 정수 ts_fchmod(정수 fd, mode_t mode){
  만약( tsIsFail() ){
    반환 -1;
  }
  반환 orig_fchmod(fd, mode);
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
static 정수 ts_fallocate(정수 fd, off_t off, off_t len){
  만약( tsIsFail() ){
    반환 tsErrno("fallocate");
  }
  반환 orig_fallocate(fd, off, len);
}

static void *ts_mmap(
  void *pAddr, 
  size_t nByte, 
  정수 prot, 
  정수 flags, 
  정수 fd, 
  off_t iOff
){
  만약( tsIsFailErrno("mmap") ){
    반환 MAP_FAILED;
  }
  반환 orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

static void *ts_mremap(void *a, size_t b, size_t c, 정수 d, ...){
  만약( tsIsFailErrno("mremap") ){
    반환 MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  assert( (d & MREMAP_FIXED)==0 );
#endif
  반환 orig_mremap(a, b, c, d);
}

static 정수 SQLITE_TCLAPI test_syscall_install(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_Size nElem;
  정수 i;
  Tcl_Obj **apElem;

  만약( objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "SYSCALL-LIST");
    반환 TCL_ERROR;
  }
  만약( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    반환 TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  위해(i=0; i<(정수)nElem; i++){
    정수 iCall;
    정수 rc = Tcl_GetIndexFromObjStruct(interp, 
        apElem[i], aSyscall, sizeof(aSyscall[0]), "system-call", 0, &iCall
    );
    만약( rc ) 반환 rc;
    만약( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  반환 TCL_OK;
}

static 정수 SQLITE_TCLAPI test_syscall_uninstall(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  정수 i;

  만약( objc!=2 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    반환 TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  위해(i=0; aSyscall[i].zName; i++){
    만약( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  반환 TCL_OK;
}

static 정수 SQLITE_TCLAPI test_syscall_reset(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  정수 i;
  정수 rc;

  만약( objc!=2 && objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    반환 TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  만약( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    위해(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }아니면{
    Tcl_Size nFunc;
    char *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    위해(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      만약( strlen(aSyscall[i].zName)!=nFunc ) continue;
      만약( memcmp(aSyscall[i].zName, zFunc, nFunc) ) continue;
      aSyscall[i].xOrig = 0;
    }
  }
  만약( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    반환 TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  반환 TCL_OK;
}

static 정수 SQLITE_TCLAPI test_syscall_exists(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  만약( objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    반환 TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(x!=0));
  반환 TCL_OK;
}

static 정수 SQLITE_TCLAPI test_syscall_fault(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  정수 nCount = 0;
  정수 bPersist = 0;

  만약( objc!=2 && objc!=4 ){
    Tcl_WrongNumArgs(interp, 2, objv, "?COUNT PERSIST?");
    반환 TCL_ERROR;
  }

  만약( objc==4 ){
    만약( Tcl_GetIntFromObj(interp, objv[2], &nCount)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      반환 TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.nCount = nCount;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  반환 TCL_OK;
}

static 정수 SQLITE_TCLAPI test_syscall_errno(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  정수 iCall;
  정수 iErrno;
  정수 rc;

  struct Errno {
    const char *z;
    정수 i;
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

  만약( objc!=4 ){
    Tcl_WrongNumArgs(interp, 2, objv, "SYSCALL ERRNO");
    반환 TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[2], aSyscall, sizeof(aSyscall[0]), "system-call", 0, &iCall
  );
  만약( rc!=TCL_OK ) 반환 rc;
  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[3], aErrno, sizeof(aErrno[0]), "errno", 0, &iErrno
  );
  만약( rc!=TCL_OK ) 반환 rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  반환 TCL_OK;
}

static 정수 SQLITE_TCLAPI test_syscall_list(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  const char *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  만약( objc!=2 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    반환 TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_IncrRefCount(pList);
  위해(zSys = pVfs->xNextSystemCall(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->xNextSystemCall(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_SetObjResult(interp, pList);
  Tcl_DecrRefCount(pList);
  반환 TCL_OK;
}

static 정수 SQLITE_TCLAPI test_syscall_defaultvfs(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  만약( objc!=2 ){
    Tcl_WrongNumArgs(interp, 2, objv, "");
    반환 TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(pVfs->zName, -1));
  반환 TCL_OK;
}

static 정수 ts_getpagesize(void){
  반환 gSyscall.pgsz;
}

static 정수 SQLITE_TCLAPI test_syscall_pagesize(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  정수 pgsz;
  만약( objc!=3 ){
    Tcl_WrongNumArgs(interp, 2, objv, "PGSZ");
    반환 TCL_ERROR;
  }
  만약( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    반환 TCL_ERROR;
  }

  만약( pgsz<0 ){
    만약( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }아니면{
    만약( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_AppendResult(interp, "pgsz out of range", NULL);
      반환 TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  반환 TCL_OK;
}

static 정수 SQLITE_TCLAPI test_syscall(
  void * clientData,
  Tcl_Interp *interp,
  정수 objc,
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
  정수 iCmd;
  정수 rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  만약( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUB-COMMAND ...");
    반환 TCL_ERROR;
  }
  만약( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_AppendResult(interp, "VFS does not support xSetSystemCall", NULL);
    rc = TCL_ERROR;
  }아니면{
    rc = Tcl_GetIndexFromObjStruct(interp, 
        objv[1], aCmd, sizeof(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  만약( rc!=TCL_OK ) 반환 rc;
  반환 aCmd[iCmd].xCmd(clientData, interp, objc, objv);
}

정수 SqlitetestSyscall_Init(Tcl_Interp *interp){
  struct SyscallCmd {
    const char *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  정수 i;

  위해(i=0; i<sizeof(aCmd)/sizeof(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  반환 TCL_OK;
}
#아니면
정수 SqlitetestSyscall_Init(Tcl_Interp *interp){
  반환 TCL_OK;
}
#endif
