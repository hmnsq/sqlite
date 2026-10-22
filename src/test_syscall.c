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

#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>

#जर SQLITE_OS_UNIX

/* From main.c */
बाह्य स्थिरांक अक्षर *sqlite3ErrName(पूर्णांक);

#अन्तर्भाव <sys/mman.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <errno.h>

स्थिर संरचना TestSyscallGlobal {
  पूर्णांक bPersist;                   /* 1 for persistent errors, 0 for transient */
  पूर्णांक n_मोजणी;                     /* Fail after this many more calls */
  पूर्णांक nFail;                      /* Number of failures that have occurred */
  पूर्णांक pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

स्थिर पूर्णांक ts_open(स्थिरांक अक्षर *, पूर्णांक, पूर्णांक);
स्थिर पूर्णांक ts_close(पूर्णांक fd);
स्थिर पूर्णांक ts_access(स्थिरांक अक्षर *zPath, पूर्णांक mode);
स्थिर अक्षर *ts_getcwd(अक्षर *zPath, माप_प्रकार nPath);
स्थिर पूर्णांक ts_stat(स्थिरांक अक्षर *zPath, संरचना stat *p);
स्थिर पूर्णांक ts_fstat(पूर्णांक fd, संरचना stat *p);
स्थिर पूर्णांक ts_ftruncate(पूर्णांक fd, off_t n);
स्थिर पूर्णांक ts_fcntl(पूर्णांक fd, पूर्णांक cmd, ... );
स्थिर ssize_t ts_read(पूर्णांक fd, शून्य *aBuf, माप_प्रकार nBuf);
स्थिर ssize_t ts_pread(पूर्णांक fd, शून्य *aBuf, माप_प्रकार nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
स्थिर ssize_t ts_pread64(पूर्णांक fd, शून्य *aBuf, माप_प्रकार nBuf, sqlite3_uint64 off);
स्थिर ssize_t ts_write(पूर्णांक fd, स्थिरांक शून्य *aBuf, माप_प्रकार nBuf);
स्थिर ssize_t ts_pwrite(पूर्णांक fd, स्थिरांक शून्य *aBuf, माप_प्रकार nBuf, off_t off);
स्थिर ssize_t ts_pwrite64(पूर्णांक fd, स्थिरांक शून्य *aBuf, माप_प्रकार nBuf, sqlite3_uint64 off);
स्थिर पूर्णांक ts_fchmod(पूर्णांक fd, mode_t mode);
स्थिर पूर्णांक ts_fallocate(पूर्णांक fd, off_t off, off_t len);
स्थिर शून्य *ts_mmap(शून्य *, माप_प्रकार, पूर्णांक, पूर्णांक, पूर्णांक, off_t);
स्थिर शून्य *ts_mremap(शून्य*, माप_प्रकार, माप_प्रकार, पूर्णांक, ...);

संरचना TestSyscallArray {
  स्थिरांक अक्षर *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  पूर्णांक default_errno;              /* Default value for errno following errors */
  पूर्णांक custom_errno;               /* Current value for errno if error */
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

#define orig_open      ((पूर्णांक(*)(स्थिरांक अक्षर *, पूर्णांक, पूर्णांक))aSyscall[0].xOrig)
#define orig_close     ((पूर्णांक(*)(पूर्णांक))aSyscall[1].xOrig)
#define orig_access    ((पूर्णांक(*)(स्थिरांक अक्षर*,पूर्णांक))aSyscall[2].xOrig)
#define orig_getcwd    ((अक्षर*(*)(अक्षर*,माप_प्रकार))aSyscall[3].xOrig)
#define orig_stat      ((पूर्णांक(*)(स्थिरांक अक्षर*,संरचना stat*))aSyscall[4].xOrig)
#define orig_fstat     ((पूर्णांक(*)(पूर्णांक,संरचना stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((पूर्णांक(*)(पूर्णांक,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((पूर्णांक(*)(पूर्णांक,पूर्णांक,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(पूर्णांक,शून्य*,माप_प्रकार))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(पूर्णांक,शून्य*,माप_प्रकार,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(पूर्णांक,शून्य*,माप_प्रकार,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(पूर्णांक,स्थिरांक शून्य*,माप_प्रकार))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(पूर्णांक,स्थिरांक शून्य*,माप_प्रकार,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(पूर्णांक,स्थिरांक शून्य*,माप_प्रकार,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((पूर्णांक(*)(पूर्णांक,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((पूर्णांक(*)(पूर्णांक,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((शून्य*(*)(शून्य*,माप_प्रकार,पूर्णांक,पूर्णांक,पूर्णांक,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((शून्य*(*)(शून्य*,माप_प्रकार,माप_प्रकार,पूर्णांक,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
स्थिर पूर्णांक tsIsFail(शून्य){
  gSyscall.n_मोजणी--;
  जर( gSyscall.n_मोजणी==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    परत 1;
  }
  परत 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
स्थिर पूर्णांक tsErrno(स्थिरांक अक्षर *zFunc){
  पूर्णांक i;
  माप_प्रकार nFunc = म_माप(zFunc);
  साठी(i=0; aSyscall[i].zName; i++){
    जर( म_माप(aSyscall[i].zName)!=nFunc ) सुरूठेवा;
    जर( स_भेद(aSyscall[i].zName, zFunc, nFunc) ) सुरूठेवा;
    परत aSyscall[i].custom_errno;
  }

  प्रतिज्ञा(0);
  परत 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
स्थिर पूर्णांक tsIsFailErrno(स्थिरांक अक्षर *zFunc){
  जर( tsIsFail() ){
    त्रुटि_सं = tsErrno(zFunc);
    परत 1;
  }
  परत 0;
}

/*
** A wrapper around open().
*/
स्थिर पूर्णांक ts_open(स्थिरांक अक्षर *zFile, पूर्णांक flags, पूर्णांक mode){
  जर( tsIsFailErrno("open") ){
    परत -1;
  }
  परत orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
स्थिर पूर्णांक ts_close(पूर्णांक fd){
  जर( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    परत -1;
  }
  परत orig_close(fd);
}

/*
** A wrapper around access().
*/
स्थिर पूर्णांक ts_access(स्थिरांक अक्षर *zPath, पूर्णांक mode){
  जर( tsIsFail() ){
    परत -1;
  }
  परत orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
स्थिर अक्षर *ts_getcwd(अक्षर *zPath, माप_प्रकार nPath){
  जर( tsIsFail() ){
    परत रिक्त;
  }
  परत orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
स्थिर पूर्णांक ts_stat(स्थिरांक अक्षर *zPath, संरचना stat *p){
  जर( tsIsFail() ){
    परत -1;
  }
  परत orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
स्थिर पूर्णांक ts_fstat(पूर्णांक fd, संरचना stat *p){
  जर( tsIsFailErrno("fstat") ){
    परत -1;
  }
  परत orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
स्थिर पूर्णांक ts_ftruncate(पूर्णांक fd, off_t n){
  जर( tsIsFailErrno("ftruncate") ){
    परत -1;
  }
  परत orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
स्थिर पूर्णांक ts_fcntl(पूर्णांक fd, पूर्णांक cmd, ... ){
  बहु_सूची ap;
  शून्य *pArg;
  जर( tsIsFailErrno("fcntl") ){
    परत -1;
  }
  बहु_शुरू(ap, cmd);
  pArg = बहु_तर्क(ap, शून्य *);
  परत orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
स्थिर ssize_t ts_read(पूर्णांक fd, शून्य *aBuf, माप_प्रकार nBuf){
  जर( tsIsFailErrno("read") ){
    परत -1;
  }
  परत orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
स्थिर ssize_t ts_pread(पूर्णांक fd, शून्य *aBuf, माप_प्रकार nBuf, off_t off){
  जर( tsIsFailErrno("pread") ){
    परत -1;
  }
  परत orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
स्थिर ssize_t ts_pread64(पूर्णांक fd, शून्य *aBuf, माप_प्रकार nBuf, sqlite3_uint64 off){
  जर( tsIsFailErrno("pread64") ){
    परत -1;
  }
  परत orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
स्थिर ssize_t ts_write(पूर्णांक fd, स्थिरांक शून्य *aBuf, माप_प्रकार nBuf){
  जर( tsIsFailErrno("write") ){
    जर( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    परत -1;
  }
  परत orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
स्थिर ssize_t ts_pwrite(पूर्णांक fd, स्थिरांक शून्य *aBuf, माप_प्रकार nBuf, off_t off){
  जर( tsIsFailErrno("pwrite") ){
    परत -1;
  }
  परत orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
स्थिर ssize_t ts_pwrite64(पूर्णांक fd, स्थिरांक शून्य *aBuf, माप_प्रकार nBuf, sqlite3_uint64 off){
  जर( tsIsFailErrno("pwrite64") ){
    परत -1;
  }
  परत orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
स्थिर पूर्णांक ts_fchmod(पूर्णांक fd, mode_t mode){
  जर( tsIsFail() ){
    परत -1;
  }
  परत orig_fchmod(fd, mode);
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
स्थिर पूर्णांक ts_fallocate(पूर्णांक fd, off_t off, off_t len){
  जर( tsIsFail() ){
    परत tsErrno("fallocate");
  }
  परत orig_fallocate(fd, off, len);
}

स्थिर शून्य *ts_mmap(
  शून्य *pAddr, 
  माप_प्रकार nByte, 
  पूर्णांक prot, 
  पूर्णांक flags, 
  पूर्णांक fd, 
  off_t iOff
){
  जर( tsIsFailErrno("mmap") ){
    परत MAP_FAILED;
  }
  परत orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

स्थिर शून्य *ts_mremap(शून्य *a, माप_प्रकार b, माप_प्रकार c, पूर्णांक d, ...){
  जर( tsIsFailErrno("mremap") ){
    परत MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  प्रतिज्ञा( (d & MREMAP_FIXED)==0 );
#endif
  परत orig_mremap(a, b, c, d);
}

स्थिर पूर्णांक SQLITE_TCLAPI test_syscall_install(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_Size nElem;
  पूर्णांक i;
  Tcl_Obj **apElem;

  जर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "SYSCALL-LIST");
    परत TCL_ERROR;
  }
  जर( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    परत TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  साठी(i=0; i<(पूर्णांक)nElem; i++){
    पूर्णांक iCall;
    पूर्णांक rc = Tcl_GetIndexFromObjStruct(interp, 
        apElem[i], aSyscall, आकार(aSyscall[0]), "system-call", 0, &iCall
    );
    जर( rc ) परत rc;
    जर( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  परत TCL_OK;
}

स्थिर पूर्णांक SQLITE_TCLAPI test_syscall_uninstall(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  पूर्णांक i;

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    परत TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  साठी(i=0; aSyscall[i].zName; i++){
    जर( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  परत TCL_OK;
}

स्थिर पूर्णांक SQLITE_TCLAPI test_syscall_reset(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  पूर्णांक i;
  पूर्णांक rc;

  जर( objc!=2 && objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    परत TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  जर( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    साठी(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }अन्यथा{
    Tcl_Size nFunc;
    अक्षर *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    साठी(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      जर( म_माप(aSyscall[i].zName)!=nFunc ) सुरूठेवा;
      जर( स_भेद(aSyscall[i].zName, zFunc, nFunc) ) सुरूठेवा;
      aSyscall[i].xOrig = 0;
    }
  }
  जर( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    परत TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  परत TCL_OK;
}

स्थिर पूर्णांक SQLITE_TCLAPI test_syscall_exists(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  जर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    परत TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(x!=0));
  परत TCL_OK;
}

स्थिर पूर्णांक SQLITE_TCLAPI test_syscall_fault(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक n_मोजणी = 0;
  पूर्णांक bPersist = 0;

  जर( objc!=2 && objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "?COUNT PERSIST?");
    परत TCL_ERROR;
  }

  जर( objc==4 ){
    जर( Tcl_GetIntFromObj(interp, objv[2], &n_मोजणी)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      परत TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_मोजणी = n_मोजणी;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  परत TCL_OK;
}

स्थिर पूर्णांक SQLITE_TCLAPI test_syscall_errno(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक iCall;
  पूर्णांक iErrno;
  पूर्णांक rc;

  संरचना Errno {
    स्थिरांक अक्षर *z;
    पूर्णांक i;
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

  जर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "SYSCALL ERRNO");
    परत TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[2], aSyscall, आकार(aSyscall[0]), "system-call", 0, &iCall
  );
  जर( rc!=TCL_OK ) परत rc;
  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[3], aErrno, आकार(aErrno[0]), "errno", 0, &iErrno
  );
  जर( rc!=TCL_OK ) परत rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  परत TCL_OK;
}

स्थिर पूर्णांक SQLITE_TCLAPI test_syscall_list(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिरांक अक्षर *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    परत TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_मोजणी(pList);
  साठी(zSys = pVfs->xNextSystemCall(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->xNextSystemCall(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_SetObjResult(interp, pList);
  Tcl_Decr_Ref_मोजणी(pList);
  परत TCL_OK;
}

स्थिर पूर्णांक SQLITE_TCLAPI test_syscall_defaultvfs(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  जर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    परत TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(pVfs->zName, -1));
  परत TCL_OK;
}

स्थिर पूर्णांक ts_getpagesize(शून्य){
  परत gSyscall.pgsz;
}

स्थिर पूर्णांक SQLITE_TCLAPI test_syscall_pagesize(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  पूर्णांक pgsz;
  जर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "PGSZ");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    परत TCL_ERROR;
  }

  जर( pgsz<0 ){
    जर( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }अन्यथा{
    जर( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_AppendResult(interp, "pgsz out of range", रिक्त);
      परत TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  परत TCL_OK;
}

स्थिर पूर्णांक SQLITE_TCLAPI test_syscall(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  संरचना SyscallCmd {
    स्थिरांक अक्षर *zName;
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
  पूर्णांक iCmd;
  पूर्णांक rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  जर( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SUB-COMMAND ...");
    परत TCL_ERROR;
  }
  जर( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_AppendResult(interp, "VFS does not support xSetSystemCall", रिक्त);
    rc = TCL_ERROR;
  }अन्यथा{
    rc = Tcl_GetIndexFromObjStruct(interp, 
        objv[1], aCmd, आकार(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  जर( rc!=TCL_OK ) परत rc;
  परत aCmd[iCmd].xCmd(clientData, interp, objc, objv);
}

पूर्णांक SqlitetestSyscall_Init(Tcl_Interp *interp){
  संरचना SyscallCmd {
    स्थिरांक अक्षर *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  पूर्णांक i;

  साठी(i=0; i<आकार(aCmd)/आकार(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  परत TCL_OK;
}
#अन्यथा
पूर्णांक SqlitetestSyscall_Init(Tcl_Interp *interp){
  परत TCL_OK;
}
#endif
