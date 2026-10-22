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

#समावेश "sqliteInt.h"
#समावेश "sqlite3.h"
#समावेश "tclsqlite.h"
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <assert.h>

#अगर SQLITE_OS_UNIX

/* From main.c */
बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);

#समावेश <sys/mman.h>
#समावेश <sys/types.h>
#समावेश <errno.h>

अटल काष्ठा TestSyscallGlobal {
  पूर्णांक bPersist;                   /* 1 for persistent errors, 0 for transient */
  पूर्णांक n_गिनती;                     /* Fail after this many more calls */
  पूर्णांक nFail;                      /* Number of failures that have occurred */
  पूर्णांक pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

अटल पूर्णांक ts_open(स्थिर अक्षर *, पूर्णांक, पूर्णांक);
अटल पूर्णांक ts_close(पूर्णांक fd);
अटल पूर्णांक ts_access(स्थिर अक्षर *zPath, पूर्णांक mode);
अटल अक्षर *ts_getcwd(अक्षर *zPath, माप_प्रकार nPath);
अटल पूर्णांक ts_stat(स्थिर अक्षर *zPath, काष्ठा stat *p);
अटल पूर्णांक ts_fstat(पूर्णांक fd, काष्ठा stat *p);
अटल पूर्णांक ts_ftruncate(पूर्णांक fd, off_t n);
अटल पूर्णांक ts_fcntl(पूर्णांक fd, पूर्णांक cmd, ... );
अटल ssize_t ts_read(पूर्णांक fd, व्योम *aBuf, माप_प्रकार nBuf);
अटल ssize_t ts_pread(पूर्णांक fd, व्योम *aBuf, माप_प्रकार nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
अटल ssize_t ts_pread64(पूर्णांक fd, व्योम *aBuf, माप_प्रकार nBuf, sqlite3_uint64 off);
अटल ssize_t ts_write(पूर्णांक fd, स्थिर व्योम *aBuf, माप_प्रकार nBuf);
अटल ssize_t ts_pwrite(पूर्णांक fd, स्थिर व्योम *aBuf, माप_प्रकार nBuf, off_t off);
अटल ssize_t ts_pwrite64(पूर्णांक fd, स्थिर व्योम *aBuf, माप_प्रकार nBuf, sqlite3_uint64 off);
अटल पूर्णांक ts_fchmod(पूर्णांक fd, mode_t mode);
अटल पूर्णांक ts_fallocate(पूर्णांक fd, off_t off, off_t len);
अटल व्योम *ts_mmap(व्योम *, माप_प्रकार, पूर्णांक, पूर्णांक, पूर्णांक, off_t);
अटल व्योम *ts_mremap(व्योम*, माप_प्रकार, माप_प्रकार, पूर्णांक, ...);

काष्ठा TestSyscallArray {
  स्थिर अक्षर *zName;
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

#घोषणा orig_open      ((पूर्णांक(*)(स्थिर अक्षर *, पूर्णांक, पूर्णांक))aSyscall[0].xOrig)
#घोषणा orig_close     ((पूर्णांक(*)(पूर्णांक))aSyscall[1].xOrig)
#घोषणा orig_access    ((पूर्णांक(*)(स्थिर अक्षर*,पूर्णांक))aSyscall[2].xOrig)
#घोषणा orig_getcwd    ((अक्षर*(*)(अक्षर*,माप_प्रकार))aSyscall[3].xOrig)
#घोषणा orig_stat      ((पूर्णांक(*)(स्थिर अक्षर*,काष्ठा stat*))aSyscall[4].xOrig)
#घोषणा orig_fstat     ((पूर्णांक(*)(पूर्णांक,काष्ठा stat*))aSyscall[5].xOrig)
#घोषणा orig_ftruncate ((पूर्णांक(*)(पूर्णांक,off_t))aSyscall[6].xOrig)
#घोषणा orig_fcntl     ((पूर्णांक(*)(पूर्णांक,पूर्णांक,...))aSyscall[7].xOrig)
#घोषणा orig_read      ((ssize_t(*)(पूर्णांक,व्योम*,माप_प्रकार))aSyscall[8].xOrig)
#घोषणा orig_pread     ((ssize_t(*)(पूर्णांक,व्योम*,माप_प्रकार,off_t))aSyscall[9].xOrig)
#घोषणा orig_pread64   ((ssize_t(*)(पूर्णांक,व्योम*,माप_प्रकार,sqlite3_uint64))aSyscall[10].xOrig)
#घोषणा orig_write     ((ssize_t(*)(पूर्णांक,स्थिर व्योम*,माप_प्रकार))aSyscall[11].xOrig)
#घोषणा orig_pwrite    ((ssize_t(*)(पूर्णांक,स्थिर व्योम*,माप_प्रकार,off_t))\
                       aSyscall[12].xOrig)
#घोषणा orig_pwrite64  ((ssize_t(*)(पूर्णांक,स्थिर व्योम*,माप_प्रकार,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#घोषणा orig_fchmod    ((पूर्णांक(*)(पूर्णांक,mode_t))aSyscall[14].xOrig)
#घोषणा orig_fallocate ((पूर्णांक(*)(पूर्णांक,off_t,off_t))aSyscall[15].xOrig)
#घोषणा orig_mmap      ((व्योम*(*)(व्योम*,माप_प्रकार,पूर्णांक,पूर्णांक,पूर्णांक,off_t))aSyscall[16].xOrig)
#घोषणा orig_mremap    ((व्योम*(*)(व्योम*,माप_प्रकार,माप_प्रकार,पूर्णांक,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
अटल पूर्णांक tsIsFail(व्योम){
  gSyscall.n_गिनती--;
  अगर( gSyscall.n_गिनती==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    वापस 1;
  }
  वापस 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
अटल पूर्णांक tsErrno(स्थिर अक्षर *zFunc){
  पूर्णांक i;
  माप_प्रकार nFunc = म_माप(zFunc);
  क्रम(i=0; aSyscall[i].zName; i++){
    अगर( म_माप(aSyscall[i].zName)!=nFunc ) जारी;
    अगर( स_भेद(aSyscall[i].zName, zFunc, nFunc) ) जारी;
    वापस aSyscall[i].custom_errno;
  }

  निश्चित(0);
  वापस 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
अटल पूर्णांक tsIsFailErrno(स्थिर अक्षर *zFunc){
  अगर( tsIsFail() ){
    त्रुटि_सं = tsErrno(zFunc);
    वापस 1;
  }
  वापस 0;
}

/*
** A wrapper around open().
*/
अटल पूर्णांक ts_open(स्थिर अक्षर *zFile, पूर्णांक flags, पूर्णांक mode){
  अगर( tsIsFailErrno("open") ){
    वापस -1;
  }
  वापस orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
अटल पूर्णांक ts_close(पूर्णांक fd){
  अगर( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    वापस -1;
  }
  वापस orig_close(fd);
}

/*
** A wrapper around access().
*/
अटल पूर्णांक ts_access(स्थिर अक्षर *zPath, पूर्णांक mode){
  अगर( tsIsFail() ){
    वापस -1;
  }
  वापस orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
अटल अक्षर *ts_getcwd(अक्षर *zPath, माप_प्रकार nPath){
  अगर( tsIsFail() ){
    वापस शून्य;
  }
  वापस orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
अटल पूर्णांक ts_stat(स्थिर अक्षर *zPath, काष्ठा stat *p){
  अगर( tsIsFail() ){
    वापस -1;
  }
  वापस orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
अटल पूर्णांक ts_fstat(पूर्णांक fd, काष्ठा stat *p){
  अगर( tsIsFailErrno("fstat") ){
    वापस -1;
  }
  वापस orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
अटल पूर्णांक ts_ftruncate(पूर्णांक fd, off_t n){
  अगर( tsIsFailErrno("ftruncate") ){
    वापस -1;
  }
  वापस orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
अटल पूर्णांक ts_fcntl(पूर्णांक fd, पूर्णांक cmd, ... ){
  बहु_सूची ap;
  व्योम *pArg;
  अगर( tsIsFailErrno("fcntl") ){
    वापस -1;
  }
  बहु_शुरू(ap, cmd);
  pArg = बहु_तर्क(ap, व्योम *);
  वापस orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
अटल ssize_t ts_read(पूर्णांक fd, व्योम *aBuf, माप_प्रकार nBuf){
  अगर( tsIsFailErrno("read") ){
    वापस -1;
  }
  वापस orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
अटल ssize_t ts_pread(पूर्णांक fd, व्योम *aBuf, माप_प्रकार nBuf, off_t off){
  अगर( tsIsFailErrno("pread") ){
    वापस -1;
  }
  वापस orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
अटल ssize_t ts_pread64(पूर्णांक fd, व्योम *aBuf, माप_प्रकार nBuf, sqlite3_uint64 off){
  अगर( tsIsFailErrno("pread64") ){
    वापस -1;
  }
  वापस orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
अटल ssize_t ts_write(पूर्णांक fd, स्थिर व्योम *aBuf, माप_प्रकार nBuf){
  अगर( tsIsFailErrno("write") ){
    अगर( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    वापस -1;
  }
  वापस orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
अटल ssize_t ts_pwrite(पूर्णांक fd, स्थिर व्योम *aBuf, माप_प्रकार nBuf, off_t off){
  अगर( tsIsFailErrno("pwrite") ){
    वापस -1;
  }
  वापस orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
अटल ssize_t ts_pwrite64(पूर्णांक fd, स्थिर व्योम *aBuf, माप_प्रकार nBuf, sqlite3_uint64 off){
  अगर( tsIsFailErrno("pwrite64") ){
    वापस -1;
  }
  वापस orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
अटल पूर्णांक ts_fchmod(पूर्णांक fd, mode_t mode){
  अगर( tsIsFail() ){
    वापस -1;
  }
  वापस orig_fchmod(fd, mode);
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
अटल पूर्णांक ts_fallocate(पूर्णांक fd, off_t off, off_t len){
  अगर( tsIsFail() ){
    वापस tsErrno("fallocate");
  }
  वापस orig_fallocate(fd, off, len);
}

अटल व्योम *ts_mmap(
  व्योम *pAddr, 
  माप_प्रकार nByte, 
  पूर्णांक prot, 
  पूर्णांक flags, 
  पूर्णांक fd, 
  off_t iOff
){
  अगर( tsIsFailErrno("mmap") ){
    वापस MAP_FAILED;
  }
  वापस orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

अटल व्योम *ts_mremap(व्योम *a, माप_प्रकार b, माप_प्रकार c, पूर्णांक d, ...){
  अगर( tsIsFailErrno("mremap") ){
    वापस MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#अगर_घोषित MREMAP_FIXED
  निश्चित( (d & MREMAP_FIXED)==0 );
#पूर्ण_अगर
  वापस orig_mremap(a, b, c, d);
}

अटल पूर्णांक SQLITE_TCLAPI test_syscall_install(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_आकार nElem;
  पूर्णांक i;
  Tcl_Obj **apElem;

  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "SYSCALL-LIST");
    वापस TCL_ERROR;
  }
  अगर( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    वापस TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  क्रम(i=0; i<(पूर्णांक)nElem; i++){
    पूर्णांक iCall;
    पूर्णांक rc = Tcl_Get_सूचकांक_From_Obj_Struct(interp, 
        apElem[i], aSyscall, माप(aSyscall[0]), "system-call", 0, &iCall
    );
    अगर( rc ) वापस rc;
    अगर( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_syscall_uninstall(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  पूर्णांक i;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    वापस TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  क्रम(i=0; aSyscall[i].zName; i++){
    अगर( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_syscall_reset(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  पूर्णांक i;
  पूर्णांक rc;

  अगर( objc!=2 && objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    वापस TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  अगर( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    क्रम(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }अन्यथा{
    Tcl_आकार nFunc;
    अक्षर *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    क्रम(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      अगर( म_माप(aSyscall[i].zName)!=nFunc ) जारी;
      अगर( स_भेद(aSyscall[i].zName, zFunc, nFunc) ) जारी;
      aSyscall[i].xOrig = 0;
    }
  }
  अगर( rc!=SQLITE_OK ){
    Tcl_Set_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    वापस TCL_ERROR;
  }

  Tcl_Reset_परिणाम(interp);
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_syscall_exists(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    वापस TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_Set_Obj_परिणाम(interp, Tcl_NewBooleanObj(x!=0));
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_syscall_fault(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक n_गिनती = 0;
  पूर्णांक bPersist = 0;

  अगर( objc!=2 && objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "?COUNT PERSIST?");
    वापस TCL_ERROR;
  }

  अगर( objc==4 ){
    अगर( Tcl_GetIntFromObj(interp, objv[2], &n_गिनती)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      वापस TCL_ERROR;
    }
  }

  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_गिनती = n_गिनती;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_syscall_errno(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णांक iCall;
  पूर्णांक iErrno;
  पूर्णांक rc;

  काष्ठा Errno {
    स्थिर अक्षर *z;
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

  अगर( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "SYSCALL ERRNO");
    वापस TCL_ERROR;
  }

  rc = Tcl_Get_सूचकांक_From_Obj_Struct(interp, 
      objv[2], aSyscall, माप(aSyscall[0]), "system-call", 0, &iCall
  );
  अगर( rc!=TCL_OK ) वापस rc;
  rc = Tcl_Get_सूचकांक_From_Obj_Struct(interp, 
      objv[3], aErrno, माप(aErrno[0]), "errno", 0, &iErrno
  );
  अगर( rc!=TCL_OK ) वापस rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_syscall_list(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर अक्षर *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    वापस TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_गिनती(pList);
  क्रम(zSys = pVfs->x_अग्र_System_Call(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->x_अग्र_System_Call(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_Set_Obj_परिणाम(interp, pList);
  Tcl_Decr_Ref_गिनती(pList);
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_syscall_defaultvfs(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    वापस TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewStringObj(pVfs->zName, -1));
  वापस TCL_OK;
}

अटल पूर्णांक ts_getpagesize(व्योम){
  वापस gSyscall.pgsz;
}

अटल पूर्णांक SQLITE_TCLAPI test_syscall_pagesize(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  पूर्णांक pgsz;
  अगर( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "PGSZ");
    वापस TCL_ERROR;
  }
  अगर( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    वापस TCL_ERROR;
  }

  अगर( pgsz<0 ){
    अगर( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }अन्यथा{
    अगर( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_Append_परिणाम(interp, "pgsz out of range", शून्य);
      वापस TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_syscall(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  काष्ठा SyscallCmd {
    स्थिर अक्षर *zName;
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

  अगर( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SUB-COMMAND ...");
    वापस TCL_ERROR;
  }
  अगर( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_Append_परिणाम(interp, "VFS does not support xSetSystemCall", शून्य);
    rc = TCL_ERROR;
  }अन्यथा{
    rc = Tcl_Get_सूचकांक_From_Obj_Struct(interp, 
        objv[1], aCmd, माप(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  अगर( rc!=TCL_OK ) वापस rc;
  वापस aCmd[iCmd].xCmd(client_तथ्य, interp, objc, objv);
}

पूर्णांक SqlitetestSyscall_Init(Tcl_Interp *interp){
  काष्ठा SyscallCmd {
    स्थिर अक्षर *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  पूर्णांक i;

  क्रम(i=0; i<माप(aCmd)/माप(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  वापस TCL_OK;
}
#अन्यथा
पूर्णांक SqlitetestSyscall_Init(Tcl_Interp *interp){
  वापस TCL_OK;
}
#पूर्ण_अगर
