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

#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "sqlite3.h"
#அந்தர்பாவ "tclsqlite.h"
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <assert.h>

#எனில் SQLITE_OS_UNIX

/* From main.c */
வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);

#அந்தர்பாவ <sys/mman.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <errno.h>

நிலையான கட்டமைப்பு TestSyscallGlobal {
  முழு bPersist;                   /* 1 for persistent errors, 0 for transient */
  முழு n_மொத்தம்;                     /* Fail after this many more calls */
  முழு nFail;                      /* Number of failures that have occurred */
  முழு pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

நிலையான முழு ts_open(மாறிலி எழுத்து *, முழு, முழு);
நிலையான முழு ts_close(முழு fd);
நிலையான முழு ts_access(மாறிலி எழுத்து *zPath, முழு mode);
நிலையான எழுத்து *ts_getcwd(எழுத்து *zPath, size_t nPath);
நிலையான முழு ts_stat(மாறிலி எழுத்து *zPath, கட்டமைப்பு stat *p);
நிலையான முழு ts_fstat(முழு fd, கட்டமைப்பு stat *p);
நிலையான முழு ts_ftruncate(முழு fd, off_t n);
நிலையான முழு ts_fcntl(முழு fd, முழு cmd, ... );
நிலையான ssize_t ts_read(முழு fd, வெறுமை *aBuf, size_t nBuf);
நிலையான ssize_t ts_pread(முழு fd, வெறுமை *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
நிலையான ssize_t ts_pread64(முழு fd, வெறுமை *aBuf, size_t nBuf, sqlite3_uint64 off);
நிலையான ssize_t ts_write(முழு fd, மாறிலி வெறுமை *aBuf, size_t nBuf);
நிலையான ssize_t ts_pwrite(முழு fd, மாறிலி வெறுமை *aBuf, size_t nBuf, off_t off);
நிலையான ssize_t ts_pwrite64(முழு fd, மாறிலி வெறுமை *aBuf, size_t nBuf, sqlite3_uint64 off);
நிலையான முழு ts_fchmod(முழு fd, mode_t mode);
நிலையான முழு ts_fallocate(முழு fd, off_t off, off_t len);
நிலையான வெறுமை *ts_mmap(வெறுமை *, size_t, முழு, முழு, முழு, off_t);
நிலையான வெறுமை *ts_mremap(வெறுமை*, size_t, size_t, முழு, ...);

கட்டமைப்பு TestSyscallArray {
  மாறிலி எழுத்து *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  முழு default_errno;              /* Default value for errno following errors */
  முழு custom_errno;               /* Current value for errno if error */
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

#define orig_open      ((முழு(*)(மாறிலி எழுத்து *, முழு, முழு))aSyscall[0].xOrig)
#define orig_close     ((முழு(*)(முழு))aSyscall[1].xOrig)
#define orig_access    ((முழு(*)(மாறிலி எழுத்து*,முழு))aSyscall[2].xOrig)
#define orig_getcwd    ((எழுத்து*(*)(எழுத்து*,size_t))aSyscall[3].xOrig)
#define orig_stat      ((முழு(*)(மாறிலி எழுத்து*,கட்டமைப்பு stat*))aSyscall[4].xOrig)
#define orig_fstat     ((முழு(*)(முழு,கட்டமைப்பு stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((முழு(*)(முழு,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((முழு(*)(முழு,முழு,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(முழு,வெறுமை*,size_t))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(முழு,வெறுமை*,size_t,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(முழு,வெறுமை*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(முழு,மாறிலி வெறுமை*,size_t))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(முழு,மாறிலி வெறுமை*,size_t,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(முழு,மாறிலி வெறுமை*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((முழு(*)(முழு,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((முழு(*)(முழு,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((வெறுமை*(*)(வெறுமை*,size_t,முழு,முழு,முழு,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((வெறுமை*(*)(வெறுமை*,size_t,size_t,முழு,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
நிலையான முழு tsIsFail(வெறுமை){
  gSyscall.n_மொத்தம்--;
  எனில்( gSyscall.n_மொத்தம்==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    திரும்பு 1;
  }
  திரும்பு 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
நிலையான முழு tsErrno(மாறிலி எழுத்து *zFunc){
  முழு i;
  size_t nFunc = strlen(zFunc);
  க்கு(i=0; aSyscall[i].zName; i++){
    எனில்( strlen(aSyscall[i].zName)!=nFunc ) தொடர்;
    எனில்( memcmp(aSyscall[i].zName, zFunc, nFunc) ) தொடர்;
    திரும்பு aSyscall[i].custom_errno;
  }

  ப்ரதிஜ்ஞா(0);
  திரும்பு 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
நிலையான முழு tsIsFailErrno(மாறிலி எழுத்து *zFunc){
  எனில்( tsIsFail() ){
    errno = tsErrno(zFunc);
    திரும்பு 1;
  }
  திரும்பு 0;
}

/*
** A wrapper around open().
*/
நிலையான முழு ts_open(மாறிலி எழுத்து *zFile, முழு flags, முழு mode){
  எனில்( tsIsFailErrno("open") ){
    திரும்பு -1;
  }
  திரும்பு orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
நிலையான முழு ts_close(முழு fd){
  எனில்( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    திரும்பு -1;
  }
  திரும்பு orig_close(fd);
}

/*
** A wrapper around access().
*/
நிலையான முழு ts_access(மாறிலி எழுத்து *zPath, முழு mode){
  எனில்( tsIsFail() ){
    திரும்பு -1;
  }
  திரும்பு orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
நிலையான எழுத்து *ts_getcwd(எழுத்து *zPath, size_t nPath){
  எனில்( tsIsFail() ){
    திரும்பு வெற்று;
  }
  திரும்பு orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
நிலையான முழு ts_stat(மாறிலி எழுத்து *zPath, கட்டமைப்பு stat *p){
  எனில்( tsIsFail() ){
    திரும்பு -1;
  }
  திரும்பு orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
நிலையான முழு ts_fstat(முழு fd, கட்டமைப்பு stat *p){
  எனில்( tsIsFailErrno("fstat") ){
    திரும்பு -1;
  }
  திரும்பு orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
நிலையான முழு ts_ftruncate(முழு fd, off_t n){
  எனில்( tsIsFailErrno("ftruncate") ){
    திரும்பு -1;
  }
  திரும்பு orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
நிலையான முழு ts_fcntl(முழு fd, முழு cmd, ... ){
  va_list ap;
  வெறுமை *pArg;
  எனில்( tsIsFailErrno("fcntl") ){
    திரும்பு -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, வெறுமை *);
  திரும்பு orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
நிலையான ssize_t ts_read(முழு fd, வெறுமை *aBuf, size_t nBuf){
  எனில்( tsIsFailErrno("read") ){
    திரும்பு -1;
  }
  திரும்பு orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
நிலையான ssize_t ts_pread(முழு fd, வெறுமை *aBuf, size_t nBuf, off_t off){
  எனில்( tsIsFailErrno("pread") ){
    திரும்பு -1;
  }
  திரும்பு orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
நிலையான ssize_t ts_pread64(முழு fd, வெறுமை *aBuf, size_t nBuf, sqlite3_uint64 off){
  எனில்( tsIsFailErrno("pread64") ){
    திரும்பு -1;
  }
  திரும்பு orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
நிலையான ssize_t ts_write(முழு fd, மாறிலி வெறுமை *aBuf, size_t nBuf){
  எனில்( tsIsFailErrno("write") ){
    எனில்( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    திரும்பு -1;
  }
  திரும்பு orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
நிலையான ssize_t ts_pwrite(முழு fd, மாறிலி வெறுமை *aBuf, size_t nBuf, off_t off){
  எனில்( tsIsFailErrno("pwrite") ){
    திரும்பு -1;
  }
  திரும்பு orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
நிலையான ssize_t ts_pwrite64(முழு fd, மாறிலி வெறுமை *aBuf, size_t nBuf, sqlite3_uint64 off){
  எனில்( tsIsFailErrno("pwrite64") ){
    திரும்பு -1;
  }
  திரும்பு orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
நிலையான முழு ts_fchmod(முழு fd, mode_t mode){
  எனில்( tsIsFail() ){
    திரும்பு -1;
  }
  திரும்பு orig_fchmod(fd, mode);
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
நிலையான முழு ts_fallocate(முழு fd, off_t off, off_t len){
  எனில்( tsIsFail() ){
    திரும்பு tsErrno("fallocate");
  }
  திரும்பு orig_fallocate(fd, off, len);
}

நிலையான வெறுமை *ts_mmap(
  வெறுமை *pAddr, 
  size_t nByte, 
  முழு prot, 
  முழு flags, 
  முழு fd, 
  off_t iOff
){
  எனில்( tsIsFailErrno("mmap") ){
    திரும்பு MAP_FAILED;
  }
  திரும்பு orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

நிலையான வெறுமை *ts_mremap(வெறுமை *a, size_t b, size_t c, முழு d, ...){
  எனில்( tsIsFailErrno("mremap") ){
    திரும்பு MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  ப்ரதிஜ்ஞா( (d & MREMAP_FIXED)==0 );
#endif
  திரும்பு orig_mremap(a, b, c, d);
}

நிலையான முழு SQLITE_TCLAPI test_syscall_install(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_Size nElem;
  முழு i;
  Tcl_Obj **apElem;

  எனில்( objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "SYSCALL-LIST");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    திரும்பு TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  க்கு(i=0; i<(முழு)nElem; i++){
    முழு iCall;
    முழு rc = Tcl_GetIndexFromObjStruct(interp, 
        apElem[i], aSyscall, அளவு(aSyscall[0]), "system-call", 0, &iCall
    );
    எனில்( rc ) திரும்பு rc;
    எனில்( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_syscall_uninstall(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  முழு i;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "");
    திரும்பு TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  க்கு(i=0; aSyscall[i].zName; i++){
    எனில்( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_syscall_reset(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  முழு i;
  முழு rc;

  எனில்( objc!=2 && objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "");
    திரும்பு TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  எனில்( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    க்கு(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }இல்லையெனில்{
    Tcl_Size nFunc;
    எழுத்து *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    க்கு(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      எனில்( strlen(aSyscall[i].zName)!=nFunc ) தொடர்;
      எனில்( memcmp(aSyscall[i].zName, zFunc, nFunc) ) தொடர்;
      aSyscall[i].xOrig = 0;
    }
  }
  எனில்( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    திரும்பு TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_syscall_exists(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  எனில்( objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "");
    திரும்பு TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(x!=0));
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_syscall_fault(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு n_மொத்தம் = 0;
  முழு bPersist = 0;

  எனில்( objc!=2 && objc!=4 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?COUNT PERSIST?");
    திரும்பு TCL_ERROR;
  }

  எனில்( objc==4 ){
    எனில்( Tcl_GetIntFromObj(interp, objv[2], &n_மொத்தம்)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      திரும்பு TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_மொத்தம் = n_மொத்தம்;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_syscall_errno(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  முழு iCall;
  முழு iErrno;
  முழு rc;

  கட்டமைப்பு Errno {
    மாறிலி எழுத்து *z;
    முழு i;
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

  எனில்( objc!=4 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "SYSCALL ERRNO");
    திரும்பு TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[2], aSyscall, அளவு(aSyscall[0]), "system-call", 0, &iCall
  );
  எனில்( rc!=TCL_OK ) திரும்பு rc;
  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[3], aErrno, அளவு(aErrno[0]), "errno", 0, &iErrno
  );
  எனில்( rc!=TCL_OK ) திரும்பு rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_syscall_list(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  மாறிலி எழுத்து *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "");
    திரும்பு TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_மொத்தம்(pList);
  க்கு(zSys = pVfs->xNextSystemCall(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->xNextSystemCall(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_SetObjResult(interp, pList);
  Tcl_Decr_Ref_மொத்தம்(pList);
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_syscall_defaultvfs(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "");
    திரும்பு TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(pVfs->zName, -1));
  திரும்பு TCL_OK;
}

நிலையான முழு ts_getpagesize(வெறுமை){
  திரும்பு gSyscall.pgsz;
}

நிலையான முழு SQLITE_TCLAPI test_syscall_pagesize(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  முழு pgsz;
  எனில்( objc!=3 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "PGSZ");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    திரும்பு TCL_ERROR;
  }

  எனில்( pgsz<0 ){
    எனில்( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }இல்லையெனில்{
    எனில்( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_AppendResult(interp, "pgsz out of range", வெற்று);
      திரும்பு TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_syscall(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  கட்டமைப்பு SyscallCmd {
    மாறிலி எழுத்து *zName;
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
  முழு iCmd;
  முழு rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  எனில்( objc<2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "SUB-COMMAND ...");
    திரும்பு TCL_ERROR;
  }
  எனில்( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_AppendResult(interp, "VFS does not support xSetSystemCall", வெற்று);
    rc = TCL_ERROR;
  }இல்லையெனில்{
    rc = Tcl_GetIndexFromObjStruct(interp, 
        objv[1], aCmd, அளவு(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  எனில்( rc!=TCL_OK ) திரும்பு rc;
  திரும்பு aCmd[iCmd].xCmd(clientData, interp, objc, objv);
}

முழு SqlitetestSyscall_Init(Tcl_Interp *interp){
  கட்டமைப்பு SyscallCmd {
    மாறிலி எழுத்து *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  முழு i;

  க்கு(i=0; i<அளவு(aCmd)/அளவு(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  திரும்பு TCL_OK;
}
#இல்லையெனில்
முழு SqlitetestSyscall_Init(Tcl_Interp *interp){
  திரும்பு TCL_OK;
}
#endif
