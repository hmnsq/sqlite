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

#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "sqlite3.h"
#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <assert.h>

#જો SQLITE_OS_UNIX

/* From main.c */
બાહ્ય અચલ અક્ષર *sqlite3ErrName(પૂર્ણાંક);

#અન્તર્ભાવ <sys/mman.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <errno.h>

સ્થિર સંરચના TestSyscallGlobal {
  પૂર્ણાંક bPersist;                   /* 1 for persistent errors, 0 for transient */
  પૂર્ણાંક n_ગણતરી;                     /* Fail after this many more calls */
  પૂર્ણાંક nFail;                      /* Number of failures that have occurred */
  પૂર્ણાંક pgsz;
  sqlite3_syscall_ptr orig_getpagesize;
} gSyscall = { 0, 0, 0, 0, 0 };

સ્થિર પૂર્ણાંક ts_open(અચલ અક્ષર *, પૂર્ણાંક, પૂર્ણાંક);
સ્થિર પૂર્ણાંક ts_close(પૂર્ણાંક fd);
સ્થિર પૂર્ણાંક ts_access(અચલ અક્ષર *zPath, પૂર્ણાંક mode);
સ્થિર અક્ષર *ts_getcwd(અક્ષર *zPath, size_t nPath);
સ્થિર પૂર્ણાંક ts_stat(અચલ અક્ષર *zPath, સંરચના stat *p);
સ્થિર પૂર્ણાંક ts_fstat(પૂર્ણાંક fd, સંરચના stat *p);
સ્થિર પૂર્ણાંક ts_ftruncate(પૂર્ણાંક fd, off_t n);
સ્થિર પૂર્ણાંક ts_fcntl(પૂર્ણાંક fd, પૂર્ણાંક cmd, ... );
સ્થિર ssize_t ts_read(પૂર્ણાંક fd, શૂન્ય *aBuf, size_t nBuf);
સ્થિર ssize_t ts_pread(પૂર્ણાંક fd, શૂન્ય *aBuf, size_t nBuf, off_t off);
/* Note:  pread64() and pwrite64() actually use off64_t as the type on their
** last parameter.  But that datatype is not defined on many systems 
** (ex: Mac, OpenBSD).  So substitute a likely equivalent: sqlite3_uint64 */
સ્થિર ssize_t ts_pread64(પૂર્ણાંક fd, શૂન્ય *aBuf, size_t nBuf, sqlite3_uint64 off);
સ્થિર ssize_t ts_write(પૂર્ણાંક fd, અચલ શૂન્ય *aBuf, size_t nBuf);
સ્થિર ssize_t ts_pwrite(પૂર્ણાંક fd, અચલ શૂન્ય *aBuf, size_t nBuf, off_t off);
સ્થિર ssize_t ts_pwrite64(પૂર્ણાંક fd, અચલ શૂન્ય *aBuf, size_t nBuf, sqlite3_uint64 off);
સ્થિર પૂર્ણાંક ts_fchmod(પૂર્ણાંક fd, mode_t mode);
સ્થિર પૂર્ણાંક ts_fallocate(પૂર્ણાંક fd, off_t off, off_t len);
સ્થિર શૂન્ય *ts_mmap(શૂન્ય *, size_t, પૂર્ણાંક, પૂર્ણાંક, પૂર્ણાંક, off_t);
સ્થિર શૂન્ય *ts_mremap(શૂન્ય*, size_t, size_t, પૂર્ણાંક, ...);

સંરચના TestSyscallArray {
  અચલ અક્ષર *zName;
  sqlite3_syscall_ptr xTest;
  sqlite3_syscall_ptr xOrig;
  પૂર્ણાંક default_errno;              /* Default value for errno following errors */
  પૂર્ણાંક custom_errno;               /* Current value for errno if error */
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

#define orig_open      ((પૂર્ણાંક(*)(અચલ અક્ષર *, પૂર્ણાંક, પૂર્ણાંક))aSyscall[0].xOrig)
#define orig_close     ((પૂર્ણાંક(*)(પૂર્ણાંક))aSyscall[1].xOrig)
#define orig_access    ((પૂર્ણાંક(*)(અચલ અક્ષર*,પૂર્ણાંક))aSyscall[2].xOrig)
#define orig_getcwd    ((અક્ષર*(*)(અક્ષર*,size_t))aSyscall[3].xOrig)
#define orig_stat      ((પૂર્ણાંક(*)(અચલ અક્ષર*,સંરચના stat*))aSyscall[4].xOrig)
#define orig_fstat     ((પૂર્ણાંક(*)(પૂર્ણાંક,સંરચના stat*))aSyscall[5].xOrig)
#define orig_ftruncate ((પૂર્ણાંક(*)(પૂર્ણાંક,off_t))aSyscall[6].xOrig)
#define orig_fcntl     ((પૂર્ણાંક(*)(પૂર્ણાંક,પૂર્ણાંક,...))aSyscall[7].xOrig)
#define orig_read      ((ssize_t(*)(પૂર્ણાંક,શૂન્ય*,size_t))aSyscall[8].xOrig)
#define orig_pread     ((ssize_t(*)(પૂર્ણાંક,શૂન્ય*,size_t,off_t))aSyscall[9].xOrig)
#define orig_pread64   ((ssize_t(*)(પૂર્ણાંક,શૂન્ય*,size_t,sqlite3_uint64))aSyscall[10].xOrig)
#define orig_write     ((ssize_t(*)(પૂર્ણાંક,અચલ શૂન્ય*,size_t))aSyscall[11].xOrig)
#define orig_pwrite    ((ssize_t(*)(પૂર્ણાંક,અચલ શૂન્ય*,size_t,off_t))\
                       aSyscall[12].xOrig)
#define orig_pwrite64  ((ssize_t(*)(પૂર્ણાંક,અચલ શૂન્ય*,size_t,sqlite3_uint64))\
                       aSyscall[13].xOrig)
#define orig_fchmod    ((પૂર્ણાંક(*)(પૂર્ણાંક,mode_t))aSyscall[14].xOrig)
#define orig_fallocate ((પૂર્ણાંક(*)(પૂર્ણાંક,off_t,off_t))aSyscall[15].xOrig)
#define orig_mmap      ((શૂન્ય*(*)(શૂન્ય*,size_t,પૂર્ણાંક,પૂર્ણાંક,પૂર્ણાંક,off_t))aSyscall[16].xOrig)
#define orig_mremap    ((શૂન્ય*(*)(શૂન્ય*,size_t,size_t,પૂર્ણાંક,...))aSyscall[17].xOrig)

/*
** This function is called exactly once from within each invocation of a
** system call wrapper in this file. It returns 1 if the function should
** fail, or 0 if it should succeed.
*/
સ્થિર પૂર્ણાંક tsIsFail(શૂન્ય){
  gSyscall.n_ગણતરી--;
  જો( gSyscall.n_ગણતરી==0 || (gSyscall.nFail && gSyscall.bPersist) ){
    gSyscall.nFail++;
    પાછા 1;
  }
  પાછા 0;
}

/*
** Return the current error-number value for function zFunc. zFunc must be
** the name of a system call in the aSyscall[] table.
**
** Usually, the current error-number is the value that errno should be set
** to if the named system call fails. The exception is "fallocate". See 
** comments above the implementation of ts_fallocate() for details.
*/
સ્થિર પૂર્ણાંક tsErrno(અચલ અક્ષર *zFunc){
  પૂર્ણાંક i;
  size_t nFunc = strlen(zFunc);
  માટે(i=0; aSyscall[i].zName; i++){
    જો( strlen(aSyscall[i].zName)!=nFunc ) ચાલુ;
    જો( memcmp(aSyscall[i].zName, zFunc, nFunc) ) ચાલુ;
    પાછા aSyscall[i].custom_errno;
  }

  પ્રતિજ્ઞા(0);
  પાછા 0;
}

/*
** A wrapper around tsIsFail(). If tsIsFail() returns non-zero, set the
** value of errno before returning.
*/
સ્થિર પૂર્ણાંક tsIsFailErrno(અચલ અક્ષર *zFunc){
  જો( tsIsFail() ){
    errno = tsErrno(zFunc);
    પાછા 1;
  }
  પાછા 0;
}

/*
** A wrapper around open().
*/
સ્થિર પૂર્ણાંક ts_open(અચલ અક્ષર *zFile, પૂર્ણાંક flags, પૂર્ણાંક mode){
  જો( tsIsFailErrno("open") ){
    પાછા -1;
  }
  પાછા orig_open(zFile, flags, mode);
}

/*
** A wrapper around close().
*/
સ્થિર પૂર્ણાંક ts_close(પૂર્ણાંક fd){
  જો( tsIsFail() ){
    /* Even if simulating an error, close the original file-descriptor. 
    ** This is to stop the test process from running out of file-descriptors
    ** when running a long test. If a call to close() appears to fail, SQLite
    ** never attempts to use the file-descriptor afterwards (or even to close
    ** it a second time).  */
    orig_close(fd);
    પાછા -1;
  }
  પાછા orig_close(fd);
}

/*
** A wrapper around access().
*/
સ્થિર પૂર્ણાંક ts_access(અચલ અક્ષર *zPath, પૂર્ણાંક mode){
  જો( tsIsFail() ){
    પાછા -1;
  }
  પાછા orig_access(zPath, mode);
}

/*
** A wrapper around getcwd().
*/
સ્થિર અક્ષર *ts_getcwd(અક્ષર *zPath, size_t nPath){
  જો( tsIsFail() ){
    પાછા ખાલી;
  }
  પાછા orig_getcwd(zPath, nPath);
}

/*
** A wrapper around stat().
*/
સ્થિર પૂર્ણાંક ts_stat(અચલ અક્ષર *zPath, સંરચના stat *p){
  જો( tsIsFail() ){
    પાછા -1;
  }
  પાછા orig_stat(zPath, p);
}

/*
** A wrapper around fstat().
*/
સ્થિર પૂર્ણાંક ts_fstat(પૂર્ણાંક fd, સંરચના stat *p){
  જો( tsIsFailErrno("fstat") ){
    પાછા -1;
  }
  પાછા orig_fstat(fd, p);
}

/*
** A wrapper around ftruncate().
*/
સ્થિર પૂર્ણાંક ts_ftruncate(પૂર્ણાંક fd, off_t n){
  જો( tsIsFailErrno("ftruncate") ){
    પાછા -1;
  }
  પાછા orig_ftruncate(fd, n);
}

/*
** A wrapper around fcntl().
*/
સ્થિર પૂર્ણાંક ts_fcntl(પૂર્ણાંક fd, પૂર્ણાંક cmd, ... ){
  va_list ap;
  શૂન્ય *pArg;
  જો( tsIsFailErrno("fcntl") ){
    પાછા -1;
  }
  va_start(ap, cmd);
  pArg = va_arg(ap, શૂન્ય *);
  પાછા orig_fcntl(fd, cmd, pArg);
}

/*
** A wrapper around read().
*/
સ્થિર ssize_t ts_read(પૂર્ણાંક fd, શૂન્ય *aBuf, size_t nBuf){
  જો( tsIsFailErrno("read") ){
    પાછા -1;
  }
  પાછા orig_read(fd, aBuf, nBuf);
}

/*
** A wrapper around pread().
*/
સ્થિર ssize_t ts_pread(પૂર્ણાંક fd, શૂન્ય *aBuf, size_t nBuf, off_t off){
  જો( tsIsFailErrno("pread") ){
    પાછા -1;
  }
  પાછા orig_pread(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pread64().
*/
સ્થિર ssize_t ts_pread64(પૂર્ણાંક fd, શૂન્ય *aBuf, size_t nBuf, sqlite3_uint64 off){
  જો( tsIsFailErrno("pread64") ){
    પાછા -1;
  }
  પાછા orig_pread64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around write().
*/
સ્થિર ssize_t ts_write(પૂર્ણાંક fd, અચલ શૂન્ય *aBuf, size_t nBuf){
  જો( tsIsFailErrno("write") ){
    જો( tsErrno("write")==EINTR ) orig_write(fd, aBuf, nBuf/2);
    પાછા -1;
  }
  પાછા orig_write(fd, aBuf, nBuf);
}

/*
** A wrapper around pwrite().
*/
સ્થિર ssize_t ts_pwrite(પૂર્ણાંક fd, અચલ શૂન્ય *aBuf, size_t nBuf, off_t off){
  જો( tsIsFailErrno("pwrite") ){
    પાછા -1;
  }
  પાછા orig_pwrite(fd, aBuf, nBuf, off);
}

/*
** A wrapper around pwrite64().
*/
સ્થિર ssize_t ts_pwrite64(પૂર્ણાંક fd, અચલ શૂન્ય *aBuf, size_t nBuf, sqlite3_uint64 off){
  જો( tsIsFailErrno("pwrite64") ){
    પાછા -1;
  }
  પાછા orig_pwrite64(fd, aBuf, nBuf, off);
}

/*
** A wrapper around fchmod().
*/
સ્થિર પૂર્ણાંક ts_fchmod(પૂર્ણાંક fd, mode_t mode){
  જો( tsIsFail() ){
    પાછા -1;
  }
  પાછા orig_fchmod(fd, mode);
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
સ્થિર પૂર્ણાંક ts_fallocate(પૂર્ણાંક fd, off_t off, off_t len){
  જો( tsIsFail() ){
    પાછા tsErrno("fallocate");
  }
  પાછા orig_fallocate(fd, off, len);
}

સ્થિર શૂન્ય *ts_mmap(
  શૂન્ય *pAddr, 
  size_t nByte, 
  પૂર્ણાંક prot, 
  પૂર્ણાંક flags, 
  પૂર્ણાંક fd, 
  off_t iOff
){
  જો( tsIsFailErrno("mmap") ){
    પાછા MAP_FAILED;
  }
  પાછા orig_mmap(pAddr, nByte, prot, flags, fd, iOff);
}

સ્થિર શૂન્ય *ts_mremap(શૂન્ય *a, size_t b, size_t c, પૂર્ણાંક d, ...){
  જો( tsIsFailErrno("mremap") ){
    પાછા MAP_FAILED;
  }
  /* mremap() may be called with 4 or 5 arguments (when MREMAP_FIXED) is
  ** set. But SQLite only ever calls the 4 argument form, and attempting to
  ** read another argument using va_arg(void*) when one has not been specified
  ** causes problems for Fil-C. So assume the 4 argument form here.  */
#ifdef MREMAP_FIXED
  પ્રતિજ્ઞા( (d & MREMAP_FIXED)==0 );
#endif
  પાછા orig_mremap(a, b, c, d);
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_syscall_install(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  Tcl_Size nElem;
  પૂર્ણાંક i;
  Tcl_Obj **apElem;

  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 2, objv, "SYSCALL-LIST");
    પાછા TCL_ERROR;
  }
  જો( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
    પાછા TCL_ERROR;
  }
  pVfs = sqlite3_vfs_find(0);

  માટે(i=0; i<(પૂર્ણાંક)nElem; i++){
    પૂર્ણાંક iCall;
    પૂર્ણાંક rc = Tcl_GetIndexFromObjStruct(interp, 
        apElem[i], aSyscall, કદ(aSyscall[0]), "system-call", 0, &iCall
    );
    જો( rc ) પાછા rc;
    જો( aSyscall[iCall].xOrig==0 ){
      aSyscall[iCall].xOrig = pVfs->xGetSystemCall(pVfs, aSyscall[iCall].zName);
      pVfs->xSetSystemCall(pVfs, aSyscall[iCall].zName, aSyscall[iCall].xTest);
    }
    aSyscall[iCall].custom_errno = aSyscall[iCall].default_errno;
  }

  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_syscall_uninstall(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  પૂર્ણાંક i;

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 2, objv, "");
    પાછા TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  માટે(i=0; aSyscall[i].zName; i++){
    જો( aSyscall[i].xOrig ){
      pVfs->xSetSystemCall(pVfs, aSyscall[i].zName, 0);
      aSyscall[i].xOrig = 0;
    }
  }
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_syscall_reset(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  પૂર્ણાંક i;
  પૂર્ણાંક rc;

  જો( objc!=2 && objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 2, objv, "");
    પાછા TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  જો( objc==2 ){
    rc = pVfs->xSetSystemCall(pVfs, 0, 0);
    માટે(i=0; aSyscall[i].zName; i++) aSyscall[i].xOrig = 0;
  }નહીંતર{
    Tcl_Size nFunc;
    અક્ષર *zFunc = Tcl_GetStringFromObj(objv[2], &nFunc);
    rc = pVfs->xSetSystemCall(pVfs, Tcl_GetString(objv[2]), 0);
    માટે(i=0; rc==SQLITE_OK && aSyscall[i].zName; i++){
      જો( strlen(aSyscall[i].zName)!=nFunc ) ચાલુ;
      જો( memcmp(aSyscall[i].zName, zFunc, nFunc) ) ચાલુ;
      aSyscall[i].xOrig = 0;
    }
  }
  જો( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    પાછા TCL_ERROR;
  }

  Tcl_ResetResult(interp);
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_syscall_exists(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 
  sqlite3_syscall_ptr x;

  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 2, objv, "");
    પાછા TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  x = pVfs->xGetSystemCall(pVfs, Tcl_GetString(objv[2]));

  Tcl_SetObjResult(interp, Tcl_NewBooleanObj(x!=0));
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_syscall_fault(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક n_ગણતરી = 0;
  પૂર્ણાંક bPersist = 0;

  જો( objc!=2 && objc!=4 ){
    Tcl_Wrong_અંક_Args(interp, 2, objv, "?COUNT PERSIST?");
    પાછા TCL_ERROR;
  }

  જો( objc==4 ){
    જો( Tcl_GetIntFromObj(interp, objv[2], &n_ગણતરી)
     || Tcl_GetBooleanFromObj(interp, objv[3], &bPersist)
    ){
      પાછા TCL_ERROR;
    }
  }

  Tcl_SetObjResult(interp, Tcl_NewIntObj(gSyscall.nFail));
  gSyscall.n_ગણતરી = n_ગણતરી;
  gSyscall.bPersist = bPersist;
  gSyscall.nFail = 0;
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_syscall_errno(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  પૂર્ણાંક iCall;
  પૂર્ણાંક iErrno;
  પૂર્ણાંક rc;

  સંરચના Errno {
    અચલ અક્ષર *z;
    પૂર્ણાંક i;
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

  જો( objc!=4 ){
    Tcl_Wrong_અંક_Args(interp, 2, objv, "SYSCALL ERRNO");
    પાછા TCL_ERROR;
  }

  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[2], aSyscall, કદ(aSyscall[0]), "system-call", 0, &iCall
  );
  જો( rc!=TCL_OK ) પાછા rc;
  rc = Tcl_GetIndexFromObjStruct(interp, 
      objv[3], aErrno, કદ(aErrno[0]), "errno", 0, &iErrno
  );
  જો( rc!=TCL_OK ) પાછા rc;

  aSyscall[iCall].custom_errno = aErrno[iErrno].i;
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_syscall_list(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  અચલ અક્ષર *zSys;
  sqlite3_vfs *pVfs; 
  Tcl_Obj *pList;

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 2, objv, "");
    પાછા TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  pList = Tcl_NewObj();
  Tcl_Incr_Ref_ગણતરી(pList);
  માટે(zSys = pVfs->xNextSystemCall(pVfs, 0); 
      zSys!=0;
      zSys = pVfs->xNextSystemCall(pVfs, zSys)
  ){
    Tcl_ListObjAppendElement(interp, pList, Tcl_NewStringObj(zSys, -1));
  }

  Tcl_SetObjResult(interp, pList);
  Tcl_Decr_Ref_ગણતરી(pList);
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_syscall_defaultvfs(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs; 

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 2, objv, "");
    પાછા TCL_ERROR;
  }

  pVfs = sqlite3_vfs_find(0);
  Tcl_SetObjResult(interp, Tcl_NewStringObj(pVfs->zName, -1));
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક ts_getpagesize(શૂન્ય){
  પાછા gSyscall.pgsz;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_syscall_pagesize(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  પૂર્ણાંક pgsz;
  જો( objc!=3 ){
    Tcl_Wrong_અંક_Args(interp, 2, objv, "PGSZ");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIntFromObj(interp, objv[2], &pgsz) ){
    પાછા TCL_ERROR;
  }

  જો( pgsz<0 ){
    જો( gSyscall.orig_getpagesize ){
      pVfs->xSetSystemCall(pVfs, "getpagesize", gSyscall.orig_getpagesize);
    }
  }નહીંતર{
    જો( pgsz<512 || (pgsz & (pgsz-1)) ){
      Tcl_AppendResult(interp, "pgsz out of range", ખાલી);
      પાછા TCL_ERROR;
    }
    gSyscall.orig_getpagesize = pVfs->xGetSystemCall(pVfs, "getpagesize");
    gSyscall.pgsz = pgsz;
    pVfs->xSetSystemCall(
        pVfs, "getpagesize", (sqlite3_syscall_ptr)ts_getpagesize
    );
  }

  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_syscall(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  સંરચના SyscallCmd {
    અચલ અક્ષર *zName;
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
  પૂર્ણાંક iCmd;
  પૂર્ણાંક rc;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);

  જો( objc<2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "SUB-COMMAND ...");
    પાછા TCL_ERROR;
  }
  જો( pVfs->iVersion<3 || pVfs->xSetSystemCall==0 ){
    Tcl_AppendResult(interp, "VFS does not support xSetSystemCall", ખાલી);
    rc = TCL_ERROR;
  }નહીંતર{
    rc = Tcl_GetIndexFromObjStruct(interp, 
        objv[1], aCmd, કદ(aCmd[0]), "sub-command", 0, &iCmd
    );
  }
  જો( rc!=TCL_OK ) પાછા rc;
  પાછા aCmd[iCmd].xCmd(clientData, interp, objc, objv);
}

પૂર્ણાંક SqlitetestSyscall_Init(Tcl_Interp *interp){
  સંરચના SyscallCmd {
    અચલ અક્ષર *zName;
    Tcl_ObjCmdProc *xCmd;
  } aCmd[] = {
    { "test_syscall",     test_syscall},
  };
  પૂર્ણાંક i;

  માટે(i=0; i<કદ(aCmd)/કદ(aCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aCmd[i].zName, aCmd[i].xCmd, 0, 0);
  }
  પાછા TCL_OK;
}
#નહીંતર
પૂર્ણાંક SqlitetestSyscall_Init(Tcl_Interp *interp){
  પાછા TCL_OK;
}
#endif
