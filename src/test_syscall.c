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

#यदि SQLITE_OS_UNIX

/* From main.c */
बाह्य ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);

#अन्तर्भाव <sys/mman.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <errno.h>

स्थिर संरचना TestSyscallGlobal {
  पूर्णाङ्क bPersist;                   /* 1 for persistent errors, 0 for transient */
  पूर्णाङ्क n_गिनती;                     /* Fail after this many more calls */
  पूर्णाङ्क nFail;                      /* Number of failures that have occurred */
  पूर्णाङ्क pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

स्थिर पूर्णाङ्क ts_open(ध्रुव वर्ण *, पूर्णाङ्क, पूर्णाङ्क);
स्थिर पूर्णाङ्क ts_close(पूर्णाङ्क fd);
स्थिर पूर्णाङ्क ts_access(ध्रुव वर्ण *zPath, पूर्णाङ्क mode);
स्थिर वर्ण *ts_getcwd(वर्ण *zPath, size_t nPath);
स्थिर पूर्णाङ्क ts_stat(ध्रुव वर्ण *zPath, संरचना stat *p);
स्थिर पूर्णाङ्क ts_fstat(पूर्णाङ्क fd, संरचना stat *p);
स्थिर पूर्णाङ्क ts_ftruncate(पूर्णाङ्क fd, off_t n);
स्थिर पूर्णाङ्क ts_fcntl(पूर्णाङ्क fd, पूर्णाङ्क cmd, ... );
स्थिर ssize_t ts_read(पूर्णाङ्क fd, शून्य *aBuf, size_t nBuf);
स्थिर ssize_t ts_pread(पूर्णाङ्क fd, शून्य *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
स्थिर ssize_t ts_pread64(पूर्णाङ्क fd, शून्य *aBuf, size_t nBuf, sqlite3_uint64 off);
स्थिर ssize_t ts_write(पूर्णाङ्क fd, ध्रुव शून्य *aBuf, size_t nBuf);
स्थिर ssize_t ts_pwrite(पूर्णाङ्क fd, ध्रुव शून्य *aBuf, size_t nBuf, off_t off);
स्थिर ssize_t ts_pwrite64(पूर्णाङ्क fd, ध्रुव शून्य *aBuf, size_t nBuf, sqlite3_uint64 off);
स्थिर पूर्णाङ्क ts_fchmod(पूर्णाङ्क fd, mode_t mode);
स्थिर पूर्णाङ्क ts_fallocate(पूर्णाङ्क fd, off_t off, off_t len);
स्थिर शून्य *ts_mmap(शून्य *, size_t, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क, off_t);
स्थिर शून्य *ts_mremap(शून्य*, size_t, size_t, पूर्णाङ्क, ...);

संरचना TestSyscallArray {
  ध्रुव वर्ण *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  पूर्णाङ्क default_errno;              /* Default value for errno following errors */
  पूर्णाङ्क custom_errno;               /* Current value for errno if error */
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

#define orig_open      ((पूर्णाङ्क(*)(ध्रुव वर्ण *, पूर्णाङ्क, पूर्णाङ्क))aSyscall[0].xOrig)
#define orig_close     ((पूर्णाङ्क(*)(पूर्णाङ्क))aSyscall[1].xOrig)
#define orig_access    ((पूर्णाङ्क(*)(ध्रुव वर्ण*,पूर्णाङ्क))aSyscall[2].xOrig)
#define orig_getcwd    ((वर्ण*(*)(वर्ण*,size_t))aSyscall[3].xOrig)
#define orig_stat      ((पूर्णाङ्क(*)(ध्रुव वर्ण*,संरचना stat*))aSyscall[4].xOrig)
#define orig_fstat     ((पूर्णाङ्क(*)(पूर्णाङ्क,संरचना stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((पूर्णाङ्क(*)(पूर्णाङ्क,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((पूर्णाङ्क(*)(पूर्णाङ्क,पूर्णाङ्क,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(पूर्णाङ्क,शून्य*,size_t))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(पूर्णाङ्क,शून्य*,size_t,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(पूर्णाङ्क,शून्य*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(पूर्णाङ्क,ध्रुव शून्य*,size_t))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(पूर्णाङ्क,ध्रुव शून्य*,size_t,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(पूर्णाङ्क,ध्रुव शून्य*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((पूर्णाङ्क(*)(पूर्णाङ्क,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((पूर्णाङ्क(*)(पूर्णाङ्क,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((शून्य*(*)(शून्य*,size_t,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((शून्य*(*)(शून्य*,size_t,size_t,पूर्णाङ्क,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
स्थिर पूर्णाङ्क tsIsFail(शून्य){
  gSyscall.n_गिनती--;
  यदि( gSyscall.n_गिनती==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
स्थिर पूर्णाङ्क tsErrno(ध्रुव वर्ण *zFunc){
  पूर्णाङ्क i;
  size_t nFunc = strlen(zFunc);
  प्रति(i=0; aSyscall[i].zName; i++){
    यदि( strlen(aSyscall[i].zName)!=nFunc ) अनुवर्त;
    यदि( memcmp(aSyscall[i].zName, zFunc, nFunc) ) अनुवर्त;
    प्रत्यागम aSyscall[i].custom_errno;
  }

  प्रतिज्ञा(0);
  प्रत्यागम 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
स्थिर पूर्णाङ्क tsIsFailErrno(ध्रुव वर्ण *zFunc){
  यदि( tsIsFail() ){
    errno = tsErrno(zFunc);
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
}

/*
** A wrapper around open().
*/
स्थिर पूर्णाङ्क ts_open(ध्रुव वर्ण *zFile, पूर्णाङ्क flags, पूर्णाङ्क mode){
  यदि( tsIsFailErrno("open") ){
    प्रत्यागम -1;
  }
  प्रत्यागम orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
स्थिर पूर्णाङ्क ts_close(पूर्णाङ्क fd){
  यदि( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    प्रत्यागम -1;
  }
  प्रत्यागम orig_close(fd);
}

/*
** A wrapper around access().
*/
स्थिर पूर्णाङ्क ts_access(ध्रुव वर्ण *zPath, पूर्णाङ्क mode){
  यदि( tsIsFail() ){
    प्रत्यागम -1;
  }
  प्रत्यागम orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
स्थिर वर्ण *ts_getcwd(वर्ण *zPath, size_t nPath){
  यदि( tsIsFail() ){
    प्रत्यागम रिक्त;
  }
  प्रत्यागम orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
स्थिर पूर्णाङ्क ts_stat(ध्रुव वर्ण *zPath, संरचना stat *p){
  यदि( tsIsFail() ){
    प्रत्यागम -1;
  }
  प्रत्यागम orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
स्थिर पूर्णाङ्क ts_fstat(पूर्णाङ्क fd, संरचना stat *p){
  यदि( tsIsFailErrno("fstat") ){
    प्रत्यागम -1;
  }
  प्रत्यागम orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
स्थिर पूर्णाङ्क ts_ftruncate(पूर्णाङ्क fd, off_t n){
  यदि( tsIsFailErrno("ftruncate") ){
    प्रत्यागम -1;
  }
  प्रत्यागम orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
स्थिर पूर्णाङ्क ts_fcntl(पूर्णाङ्क fd, पूर्णाङ्क cmd, ... ){
  va_list ap;
  शून्य *pArg;
  यदि( tsIsFailErrno("fcntl") ){
    प्रत्यागम -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, शून्य *);
  प्रत्यागम orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
स्थिर ssize_t ts_read(पूर्णाङ्क fd, शून्य *aBuf, size_t nBuf){
  यदि( tsIsFailErrno("read") ){
    प्रत्यागम -1;
  }
  प्रत्यागम orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
स्थिर ssize_t ts_pread(पूर्णाङ्क fd, शून्य *aBuf, size_t nBuf, off_t off){
  यदि( tsIsFailErrno("pread") ){
    प्रत्यागम -1;
  }
  प्रत्यागम orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
स्थिर ssize_t ts_pread64(पूर्णाङ्क fd, शून्य *aBuf, size_t nBuf, sqlite3_uint64 off){
  यदि( tsIsFailErrno("pread64") ){
    प्रत्यागम -1;
  }
  प्रत्यागम orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
स्थिर ssize_t ts_write(पूर्णाङ्क fd, ध्रुव शून्य *aBuf, size_t nBuf){
  यदि( tsIsFailErrno("write") ){
    यदि( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    प्रत्यागम -1;
  }
  प्रत्यागम orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
स्थिर ssize_t ts_pwrite(पूर्णाङ्क fd, ध्रुव शून्य *aBuf, size_t nBuf, off_t off){
  यदि( tsIsFailErrno("pwrite") ){
    प्रत्यागम -1;
  }
  प्रत्यागम orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
स्थिर ssize_t ts_pwrite64(पूर्णाङ्क fd, ध्रुव शून्य *aBuf, size_t nBuf, sqlite3_uint64 off){
  यदि( tsIsFailErrno("pwrite64") ){
    प्रत्यागम -1;
  }
  प्रत्यागम orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
स्थिर पूर्णाङ्क ts_fchmod(पूर्णाङ्क fd, mode_t mode){
  यदि( tsIsFail() ){
    प्रत्यागम -1;
  }
  प्रत्यागम orig_fchmod(fd, mode);
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
स्थिर पूर्णाङ्क ts_fallocate(पूर्णाङ्क fd, off_t off, off_t len){
  यदि( tsIsFail() ){
    प्रत्यागम tsErrno("fallocate");
  }
  प्रत्यागम orig_fallocate(fd, off, len);
}

स्थिर शून्य *ts_mmap(
  शून्य *pAddr, 
  size_t nByte, 
  पूर्णाङ्क prot, 
  पूर्णाङ्क flags, 
  पूर्णाङ्क fd, 
  off_t iOff
){
  यदि( tsIsFailErrno("mmap") ){
    प्रत्यागम MAP_FAILED;
  }
  प्रत्यागम orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

स्थिर शून्य *ts_mremap(शून्य *a, size_t b, size_t c, पूर्णाङ्क d, ...){
  यदि( tsIsFailErrno("mremap") ){
    प्रत्यागम MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  प्रतिज्ञा( (d & MREMAP_FIXED)==0 );
#endif
  प्रत्यागम orig_mremap(a, b, c, d);
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_syscall_install(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_Size nElem;
  पूर्णाङ्क i;
  Tcl_Obj **apElem;

  यदि( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "SYSCALL-LIST");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    प्रत्यागम TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  प्रति(i=0; i<(पूर्णाङ्क)nElem; i++){
    पूर्णाङ्क iCall;
    पूर्णाङ्क rc = Tcl_GetIndexFromObjStruct(interp, 
        apElem[i], aSyscall, परिमाण(aSyscall[0]), "system-call", 0, &iCall
    );
    यदि( rc ) प्रत्यागम rc;
    यदि( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_syscall_uninstall(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  पूर्णाङ्क i;

  यदि( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    प्रत्यागम TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  प्रति(i=0; aSyscall[i].zName; i++){
    यदि( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_syscall_reset(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  पूर्णाङ्क i;
  पूर्णाङ्क rc;

  यदि( objc!=2 && objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    प्रत्यागम TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  यदि( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    प्रति(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }अन्यथा{
    Tcl_Size nFunc;
    वर्ण *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    प्रति(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      यदि( strlen(aSyscall[i].zName)!=nFunc ) अनुवर्त;
      यदि( memcmp(aSyscall[i].zName, zFunc, nFunc) ) अनुवर्त;
      aSyscall[i].xOrig = 0;
    }
  }
  यदि( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    प्रत्यागम TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_syscall_exists(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  यदि( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    प्रत्यागम TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(x!=0));
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_syscall_fault(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क n_गिनती = 0;
  पूर्णाङ्क bPersist = 0;

  यदि( objc!=2 && objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "?COUNT PERSIST?");
    प्रत्यागम TCL_ERROR;
  }

  यदि( objc==4 ){
    यदि( Tcl_GetIntFromObj(interp, objv[2], &n_गिनती)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      प्रत्यागम TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_गिनती = n_गिनती;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_syscall_errno(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  पूर्णाङ्क iCall;
  पूर्णाङ्क iErrno;
  पूर्णाङ्क rc;

  संरचना Errno {
    ध्रुव वर्ण *z;
    पूर्णाङ्क i;
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

  यदि( objc!=4 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "SYSCALL ERRNO");
    प्रत्यागम TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[2], aSyscall, परिमाण(aSyscall[0]), "system-call", 0, &iCall
  );
  यदि( rc!=TCL_OK ) प्रत्यागम rc;
  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[3], aErrno, परिमाण(aErrno[0]), "errno", 0, &iErrno
  );
  यदि( rc!=TCL_OK ) प्रत्यागम rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_syscall_list(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  ध्रुव वर्ण *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  यदि( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    प्रत्यागम TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_गिनती(pList);
  प्रति(zSys = pVfs->xNextSystemCall(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->xNextSystemCall(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_SetObjResult(interp, pList);
  Tcl_Decr_Ref_गिनती(pList);
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_syscall_defaultvfs(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  यदि( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "");
    प्रत्यागम TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(pVfs->zName, -1));
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क ts_getpagesize(शून्य){
  प्रत्यागम gSyscall.pgsz;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_syscall_pagesize(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  पूर्णाङ्क pgsz;
  यदि( objc!=3 ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, "PGSZ");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    प्रत्यागम TCL_ERROR;
  }

  यदि( pgsz<0 ){
    यदि( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }अन्यथा{
    यदि( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_AppendResult(interp, "pgsz out of range", रिक्त);
      प्रत्यागम TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_syscall(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  संरचना SyscallCmd {
    ध्रुव वर्ण *zName;
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
  पूर्णाङ्क iCmd;
  पूर्णाङ्क rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  यदि( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SUB-COMMAND ...");
    प्रत्यागम TCL_ERROR;
  }
  यदि( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_AppendResult(interp, "VFS does not support xSetSystemCall", रिक्त);
    rc = TCL_ERROR;
  }अन्यथा{
    rc = Tcl_GetIndexFromObjStruct(interp, 
        objv[1], aCmd, परिमाण(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  यदि( rc!=TCL_OK ) प्रत्यागम rc;
  प्रत्यागम aCmd[iCmd].xCmd(clientData, interp, objc, objv);
}

पूर्णाङ्क SqlitetestSyscall_Init(Tcl_Interp *interp){
  संरचना SyscallCmd {
    ध्रुव वर्ण *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  पूर्णाङ्क i;

  प्रति(i=0; i<परिमाण(aCmd)/परिमाण(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  प्रत्यागम TCL_OK;
}
#अन्यथा
पूर्णाङ्क SqlitetestSyscall_Init(Tcl_Interp *interp){
  प्रत्यागम TCL_OK;
}
#endif
