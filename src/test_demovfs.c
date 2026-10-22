/*
** 2010 April 7
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
** This file implements an example of a simple VFS implementation that 
** omits complex features often not required or not possible on embedded
** platforms.  Code is included to buffer writes to the journal file, 
** which can be a significant performance improvement on some embedded
** platforms.
**
** OVERVIEW
**
**   The code in this file implements a minimal SQLite VFS that can be 
**   used on Linux and other posix-like operating systems. The following 
**   system calls are used:
**
**    File-system: access(), unlink(), getcwd()
**    File IO:     open(), read(), write(), fsync(), close(), fstat()
**    Other:       sleep(), usleep(), time()
**
**   The following VFS features are omitted:
**
**     1. File locking. The user must ensure that there is at most one
**        connection to each database when using this VFS. Multiple
**        connections to a single shared-cache count as a single connection
**        for the purposes of the previous statement.
**
**     2. The loading of dynamic extensions (shared libraries).
**
**     3. Temporary files. The user must configure SQLite to use in-memory
**        temp files when using this VFS. The easiest way to do this is to
**        compile with:
**
**          -DSQLITE_TEMP_STORE=3
**
**     4. File truncation. As of version 3.6.24, SQLite may run without
**        a working xTruncate() call, providing the user does not configure
**        SQLite to use "journal_mode=truncate", or use both
**        "journal_mode=persist" and ATTACHed databases.
**
**   It is assumed that the system uses UNIX-like path-names. Specifically,
**   that '/' characters are used to separate path components and that
**   a path-name is a relative path unless it begins with a '/'. And that
**   no UTF-8 encoded paths are greater than 512 bytes in length.
**
** JOURNAL WRITE-BUFFERING
**
**   To commit a transaction to the database, SQLite first writes rollback
**   information into the journal file. This usually consists of 4 steps:
**
**     1. The rollback information is sequentially written into the journal
**        file, starting at the start of the file.
**     2. The journal file is synced to disk.
**     3. A modification is made to the first few bytes of the journal file.
**     4. The journal file is synced to disk again.
**
**   Most of the data is written in step 1 using a series of calls to the
**   VFS xWrite() method. The buffers passed to the xWrite() calls are of
**   various sizes. For example, as of version 3.6.24, when committing a 
**   transaction that modifies 3 pages of a database file that uses 4096 
**   byte pages residing on a media with 512 byte sectors, SQLite makes 
**   eleven calls to the xWrite() method to create the rollback journal, 
**   as follows:
**
**             Write offset | Bytes written
**             ----------------------------
**                        0            512
**                      512              4
**                      516           4096
**                     4612              4
**                     4616              4
**                     4620           4096
**                     8716              4
**                     8720              4
**                     8724           4096
**                    12820              4
**             ++++++++++++SYNC+++++++++++
**                        0             12
**             ++++++++++++SYNC+++++++++++
**
**   On many operating systems, this is an efficient way to write to a file.
**   However, on some embedded systems that do not cache writes in OS 
**   buffers it is much more efficient to write data in blocks that are
**   an integer multiple of the sector-size in size and aligned at the
**   start of a sector.
**
**   To work around this, the code in this file allocates a fixed size
**   buffer of SQLITE_DEMOVFS_BUFFERSZ using sqlite3_malloc() whenever a 
**   journal file is opened. It uses the buffer to coalesce sequential
**   writes into aligned SQLITE_DEMOVFS_BUFFERSZ blocks. When SQLite
**   invokes the xSync() method to sync the contents of the file to disk,
**   all accumulated data is written out, even if it does not constitute
**   a complete block. This means the actual IO to create the rollback 
**   journal for the example transaction above is this:
**
**             Write offset | Bytes written
**             ----------------------------
**                        0           8192
**                     8192           4632
**             ++++++++++++SYNC+++++++++++
**                        0             12
**             ++++++++++++SYNC+++++++++++
**
**   Much more efficient if the underlying OS is not caching write 
**   operations.
*/

#اگر !defined(SQLITE_TEST) || SQLITE_OS_UNIX

#شامل "sqlite3.h"

#شامل <assert.h>
#شامل <string.h>
#شامل <sys/types.h>
#شامل <sys/stat.h>
#شامل <sys/file.h>
#شامل <sys/param.h>
#شامل <unistd.h>
#شامل <time.h>
#شامل <errno.h>
#شامل <fcntl.h>

/*
** Size of the write buffer used by journal files in bytes.
*/
#اگر_غیر_تعریف_شدہ SQLITE_DEMOVFS_BUFFERSZ
# define SQLITE_DEMOVFS_BUFFERSZ 8192
#اگر_ختم

/*
** The maximum pathname length supported by this VFS.
*/
#تعریف_کرو MAXPATHNAME 512

/*
** When using this VFS, the sqlite3_file* handles that SQLite uses are
** actually pointers to instances of type DemoFile.
*/
قسمتعریف ساخت DemoFile DemoFile;
ساخت DemoFile {
  sqlite3_file بنیاد;              /* Base class. Must be first. */
  صحیح fd;                         /* File descriptor */

  حرف *a_بفر;                  /* Pointer to malloc'd buffer */
  صحیح n_بفر;                    /* Valid bytes of data in zBuffer */
  sqlite3_int64 i_بفر_Ofst;      /* Offset in file of zBuffer[0] */
};

/*
** Write directly to the file passed as the first argument. Even if the
** file has a write-buffer (DemoFile.aBuffer), ignore it.
*/
ساکن صحیح demo_براہ_راست_Write(
  DemoFile *p,                    /* File handle */
  ثابت خالی *zBuf,               /* Buffer containing data to write */
  صحیح iAmt,                       /* Size of data to write in bytes */
  sqlite_int64 iOfst              /* File offset to write to */
){
  off_t ofst;                     /* Return value from lseek() */
  size_t nWrite;                  /* Return value from write() */

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  اگر( ofst!=iOfst ){
    واپس SQLITE_IOERR_WRITE;
  }

  nWrite = اكتب(p->fd, zBuf, iAmt);
  اگر( nWrite!=iAmt ){
    واپس SQLITE_IOERR_WRITE;
  }

  واپس SQLITE_OK;
}

/*
** Flush the contents of the DemoFile.aBuffer buffer to disk. This is a
** no-op if this particular file does not have a buffer (i.e. it is not
** a journal file) or if the buffer is currently empty.
*/
ساکن صحیح demo_Flush_بفر(DemoFile *p){
  صحیح rc = SQLITE_OK;
  اگر( p->n_بفر ){
    rc = demo_براہ_راست_Write(p, p->a_بفر, p->n_بفر, p->i_بفر_Ofst);
    p->n_بفر = 0;
  }
  واپس rc;
}

/*
** Close a file.
*/
ساکن صحیح demoClose(sqlite3_file *pFile){
  صحیح rc;
  DemoFile *p = (DemoFile*)pFile;
  rc = demo_Flush_بفر(p);
  sqlite3_free(p->a_بفر);
  close(p->fd);
  واپس rc;
}

/*
** Read data from a file.
*/
ساکن صحیح demoRead(
  sqlite3_file *pFile, 
  خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  off_t ofst;                     /* Return value from lseek() */
  صحیح nRead;                      /* Return value from read() */
  صحیح rc;                         /* Return code from demoFlushBuffer() */

  /* Flush any data in the write buffer to disk in case this operation
  ** is trying to read data the file-region currently cached in the buffer.
  ** It would be possible to detect this case and possibly save an 
  ** unnecessary write here, but in practice SQLite will rarely read from
  ** a journal file when there is data cached in the write-buffer.
  */
  rc = demo_Flush_بفر(p);
  اگر( rc!=SQLITE_OK ){
    واپس rc;
  }

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  اگر( ofst!=iOfst ){
    واپس SQLITE_IOERR_READ;
  }
  nRead = اقرأ(p->fd, zBuf, iAmt);

  اگر( nRead==iAmt ){
    واپس SQLITE_OK;
  }ورنہ اگر( nRead>=0 ){
    اگر( nRead<iAmt ){
      memset(&((حرف*)zBuf)[nRead], 0, iAmt-nRead);
    }
    واپس SQLITE_IOERR_SHORT_READ;
  }

  واپس SQLITE_IOERR_READ;
}

/*
** Write data to a crash-file.
*/
ساکن صحیح demoWrite(
  sqlite3_file *pFile, 
  ثابت خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  
  اگر( p->a_بفر ){
    حرف *z = (حرف *)zBuf;       /* Pointer to remaining data to write */
    صحیح n = iAmt;                 /* Number of bytes at z */
    sqlite3_int64 i = iOfst;      /* File offset to write to */

    جب( n>0 ){
      صحیح nCopy;                  /* Number of bytes to copy into buffer */

      /* If the buffer is full, or if this data is not being written directly
      ** following the data already buffered, flush the buffer. Flushing
      ** the buffer is a no-op if it is empty.  
      */
      اگر( p->n_بفر==SQLITE_DEMOVFS_BUFFERSZ || p->i_بفر_Ofst+p->n_بفر!=i ){
        صحیح rc = demo_Flush_بفر(p);
        اگر( rc!=SQLITE_OK ){
          واپس rc;
        }
      }
      أكد( p->n_بفر==0 || p->i_بفر_Ofst+p->n_بفر==i );
      p->i_بفر_Ofst = i - p->n_بفر;

      /* Copy as much data as possible into the buffer. */
      nCopy = SQLITE_DEMOVFS_BUFFERSZ - p->n_بفر;
      اگر( nCopy>n ){
        nCopy = n;
      }
      حافظہ_نقل(&p->a_بفر[p->n_بفر], z, nCopy);
      p->n_بفر += nCopy;

      n -= nCopy;
      i += nCopy;
      z += nCopy;
    }
  }ورنہ{
    واپس demo_براہ_راست_Write(p, zBuf, iAmt, iOfst);
  }

  واپس SQLITE_OK;
}

/*
** Truncate a file. This is a no-op for this VFS (see header comments at
** the top of the file).
*/
ساکن صحیح demoTruncate(sqlite3_file *pFile, sqlite_int64 سائز){
#اگر 0
  اگر( ftruncate(((DemoFile *)pFile)->fd, سائز) ) واپس SQLITE_IOERR_TRUNCATE;
#اگر_ختم
  واپس SQLITE_OK;
}

/*
** Sync the contents of the file to the persistent media.
*/
ساکن صحیح demoSync(sqlite3_file *pFile, صحیح flags){
  DemoFile *p = (DemoFile*)pFile;
  صحیح rc;

  rc = demo_Flush_بفر(p);
  اگر( rc!=SQLITE_OK ){
    واپس rc;
  }

  rc = fsync(p->fd);
  واپس (rc==0 ? SQLITE_OK : SQLITE_IOERR_FSYNC);
}

/*
** Write the size of the file in bytes to *pSize.
*/
ساکن صحیح demo_File_سائز(sqlite3_file *pFile, sqlite_int64 *p_سائز){
  DemoFile *p = (DemoFile*)pFile;
  صحیح rc;                         /* Return code from fstat() call */
  ساخت stat sStat;              /* Output of fstat() call */

  /* Flush the contents of the buffer to disk. As with the flush in the
  ** demoRead() method, it would be possible to avoid this and save a write
  ** here and there. But in practice this comes up so infrequently it is
  ** not worth the trouble.
  */
  rc = demo_Flush_بفر(p);
  اگر( rc!=SQLITE_OK ){
    واپس rc;
  }

  rc = fstat(p->fd, &sStat);
  اگر( rc!=0 ) واپس SQLITE_IOERR_FSTAT;
  *p_سائز = sStat.st_سائز;
  واپس SQLITE_OK;
}

/*
** Locking functions. The xLock() and xUnlock() methods are both no-ops.
** The xCheckReservedLock() always indicates that no other process holds
** a reserved lock on the database file. This ensures that if a hot-journal
** file is found in the file-system it is rolled back.
*/
ساکن صحیح demo_تالا(sqlite3_file *pFile, صحیح e_تالا){
  واپس SQLITE_OK;
}
ساکن صحیح demoUnlock(sqlite3_file *pFile, صحیح e_تالا){
  واپس SQLITE_OK;
}
ساکن صحیح demo_Check_Reserved_تالا(sqlite3_file *pFile, صحیح *p_Res_برآمد){
  *p_Res_برآمد = 0;
  واپس SQLITE_OK;
}

/*
** No xFileControl() verbs are implemented by this VFS.
*/
ساکن صحیح demoFileControl(sqlite3_file *pFile, صحیح op, خالی *pArg){
  واپس SQLITE_NOTFOUND;
}

/*
** The xSectorSize() and xDeviceCharacteristics() methods. These two
** may return special values allowing SQLite to optimize file-system 
** access to some extent. But it is also safe to simply return 0.
*/
ساکن صحیح demo_Sector_سائز(sqlite3_file *pFile){
  واپس 0;
}
ساکن صحیح demoDeviceCharacteristics(sqlite3_file *pFile){
  واپس 0;
}

/*
** Open a file handle.
*/
ساکن صحیح demoOpen(
  sqlite3_vfs *pVfs,              /* VFS */
  ثابت حرف *zName,              /* File to open, or 0 for a temp file */
  sqlite3_file *pFile,            /* Pointer to DemoFile struct to populate */
  صحیح flags,                      /* Input SQLITE_OPEN_XXX flags */
  صحیح *p_برآمد_Flags                  /* Output SQLITE_OPEN_XXX flags (or NULL) */
){
  ساکن ثابت sqlite3_io_methods demoio = {
    1,                            /* iVersion */
    demoClose,                    /* xClose */
    demoRead,                     /* xRead */
    demoWrite,                    /* xWrite */
    demoTruncate,                 /* xTruncate */
    demoSync,                     /* xSync */
    demo_File_سائز,                 /* xFileSize */
    demo_تالا,                     /* xLock */
    demoUnlock,                   /* xUnlock */
    demo_Check_Reserved_تالا,        /* xCheckReservedLock */
    demoFileControl,              /* xFileControl */
    demo_Sector_سائز,               /* xSectorSize */
    demoDeviceCharacteristics     /* xDeviceCharacteristics */
  };

  DemoFile *p = (DemoFile*)pFile; /* Populate this structure */
  صحیح oflags = 0;                 /* flags to pass to open() call */
  حرف *aBuf = 0;

  اگر( zName==0 ){
    واپس SQLITE_IOERR;
  }

  اگر( flags&SQLITE_OPEN_MAIN_JOURNAL ){
    aBuf = (حرف *)sqlite3_malloc(SQLITE_DEMOVFS_BUFFERSZ);
    اگر( !aBuf ){
      واپس SQLITE_NOMEM;
    }
  }

  اگر( flags&SQLITE_OPEN_EXCLUSIVE ) oflags |= O_EXCL;
  اگر( flags&SQLITE_OPEN_CREATE )    oflags |= O_CREAT;
  اگر( flags&SQLITE_OPEN_READONLY )  oflags |= O_RDONLY;
  اگر( flags&SQLITE_OPEN_READWRITE ) oflags |= O_RDWR;

  memset(p, 0, حجم(DemoFile));
  p->fd = open(zName, oflags, 0600);
  اگر( p->fd<0 ){
    sqlite3_free(aBuf);
    واپس SQLITE_CANTOPEN;
  }
  p->a_بفر = aBuf;

  اگر( p_برآمد_Flags ){
    *p_برآمد_Flags = flags;
  }
  p->بنیاد.pMethods = &demoio;
  واپس SQLITE_OK;
}

/*
** Delete the file identified by argument zPath. If the dirSync parameter
** is non-zero, then ensure the file-system modification to delete the
** file has been synced to disk before returning.
*/
ساکن صحیح demoDelete(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ, صحیح dirSync){
  صحیح rc;                         /* Return code */

  rc = unlink(z_راستہ);
  اگر( rc!=0 && errno==ENOENT ) واپس SQLITE_OK;

  اگر( rc==0 && dirSync ){
    صحیح dfd;                      /* File descriptor open on directory */
    حرف *zSlash;
    حرف zDir[MAXPATHNAME+1];     /* Name of directory containing file zPath */

    /* Figure out the directory name from the path of the file deleted. */
    sqlite3_snprintf(MAXPATHNAME, zDir, "%s", z_راستہ);
    zDir[MAXPATHNAME] = '\0';
    zSlash = strrchr(zDir,'/');
    اگر( zSlash ){
      /* Open a file-descriptor on the directory. Sync. Close. */
      zSlash[0] = 0;
      dfd = open(zDir, O_RDONLY, 0);
      اگر( dfd<0 ){
        rc = -1;
      }ورنہ{
        rc = fsync(dfd);
        close(dfd);
      }
    }
  }
  واپس (rc==0 ? SQLITE_OK : SQLITE_IOERR_DELETE);
}

#اگر_غیر_تعریف_شدہ F_OK
# define F_OK 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ R_OK
# define R_OK 4
#اگر_ختم
#اگر_غیر_تعریف_شدہ W_OK
# define W_OK 2
#اگر_ختم

/*
** Query the file-system to see if the named file exists, is readable or
** is both readable and writable.
*/
ساکن صحیح demoAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_راستہ, 
  صحیح flags, 
  صحیح *p_Res_برآمد
){
  صحیح rc;                         /* access() return code */
  صحیح eAccess = F_OK;             /* Second argument to access() */

  أكد( flags==SQLITE_ACCESS_EXISTS       /* access(zPath, F_OK) */
       || flags==SQLITE_ACCESS_READ         /* access(zPath, R_OK) */
       || flags==SQLITE_ACCESS_READWRITE    /* access(zPath, R_OK|W_OK) */
  );

  اگر( flags==SQLITE_ACCESS_READWRITE ) eAccess = R_OK|W_OK;
  اگر( flags==SQLITE_ACCESS_READ )      eAccess = R_OK;

  rc = access(z_راستہ, eAccess);
  *p_Res_برآمد = (rc==0);
  واپس SQLITE_OK;
}

/*
** Argument zPath points to a nul-terminated string containing a file path.
** If zPath is an absolute path, then it is copied as is into the output 
** buffer. Otherwise, if it is a relative path, then the equivalent full
** path is written to the output buffer.
**
** This function assumes that paths are UNIX style. Specifically, that:
**
**   1. Path components are separated by a '/'. and 
**   2. Full paths begin with a '/' character.
*/
ساکن صحیح demo_بھرا_Pathname(
  sqlite3_vfs *pVfs,              /* VFS */
  ثابت حرف *z_راستہ,              /* Input path (possibly a relative path) */
  صحیح n_راستہ_برآمد,                   /* Size of output buffer in bytes */
  حرف *z_راستہ_برآمد                  /* Pointer to output buffer */
){
  حرف zDir[MAXPATHNAME+1];
  اگر( z_راستہ[0]=='/' ){
    zDir[0] = '\0';
  }ورنہ{
    اگر( getcwd(zDir, حجم(zDir))==0 ) واپس SQLITE_IOERR;
  }
  zDir[MAXPATHNAME] = '\0';

  sqlite3_snprintf(n_راستہ_برآمد, z_راستہ_برآمد, "%s/%s", zDir, z_راستہ);
  z_راستہ_برآمد[n_راستہ_برآمد-1] = '\0';

  واپس SQLITE_OK;
}

/*
** The following four VFS methods:
**
**   xDlOpen
**   xDlError
**   xDlSym
**   xDlClose
**
** are supposed to implement the functionality needed by SQLite to load
** extensions compiled as shared objects. This simple VFS does not support
** this functionality, so the following functions are no-ops.
*/
ساکن خالی *demoDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ){
  واپس 0;
}
ساکن خالی demoDlError(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Err_پیغام){
  sqlite3_snprintf(nByte, z_Err_پیغام, "Loadable extensions are not supported");
  z_Err_پیغام[nByte-1] = '\0';
}
ساکن خالی (*demoDlSym(sqlite3_vfs *pVfs, خالی *pH, ثابت حرف *z))(خالی){
  واپس 0;
}
ساکن خالی demoDlClose(sqlite3_vfs *pVfs, خالی *pHandle){
  واپس;
}

/*
** Parameter zByte points to a buffer nByte bytes in size. Populate this
** buffer with pseudo-random data.
*/
ساکن صحیح demoRandomness(sqlite3_vfs *pVfs, صحیح nByte, حرف *zByte){
  واپس SQLITE_OK;
}

/*
** Sleep for at least nMicro microseconds. Return the (approximate) number 
** of microseconds slept for.
*/
ساکن صحیح demoSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  sleep(nMicro / 1000000);
  usleep(nMicro % 1000000);
  واپس nMicro;
}

/*
** Set *pTime to the current UTC time expressed as a Julian day. Return
** SQLITE_OK if successful, or an error code otherwise.
**
**   http://en.wikipedia.org/wiki/Julian_day
**
** This implementation is not very good. The current time is rounded to
** an integer number of seconds. Also, assuming time_t is a signed 32-bit 
** value, it will stop working some time in the year 2038 AD (the so-called
** "year 2038" problem that afflicts systems that store time this way). 
*/
ساکن صحیح demo_Current_وقت(sqlite3_vfs *pVfs, دوہرا *p_وقت){
  وقت_t t = وقت(0);
  *p_وقت = t/86400.0 + 2440587.5; 
  واپس SQLITE_OK;
}

/*
** This function returns a pointer to the VFS implemented in this file.
** To make the VFS available to SQLite:
**
**   sqlite3_vfs_register(sqlite3_demovfs(), 0);
*/
sqlite3_vfs *sqlite3_demovfs(خالی){
  ساکن sqlite3_vfs demovfs = {
    1,                            /* iVersion */
    حجم(DemoFile),             /* szOsFile */
    MAXPATHNAME,                  /* mxPathname */
    0,                            /* pNext */
    "demo",                       /* zName */
    0,                            /* pAppData */
    demoOpen,                     /* xOpen */
    demoDelete,                   /* xDelete */
    demoAccess,                   /* xAccess */
    demo_بھرا_Pathname,             /* xFullPathname */
    demoDlOpen,                   /* xDlOpen */
    demoDlError,                  /* xDlError */
    demoDlSym,                    /* xDlSym */
    demoDlClose,                  /* xDlClose */
    demoRandomness,               /* xRandomness */
    demoSleep,                    /* xSleep */
    demo_Current_وقت,              /* xCurrentTime */
  };
  واپس &demovfs;
}

#اگر_ختم /* !defined(SQLITE_TEST) || SQLITE_OS_UNIX */


#اگر_تعریف_شدہ SQLITE_TEST

#شامل "tclsqlite.h"

#اگر SQLITE_OS_UNIX
ساکن صحیح SQLITE_TCLAPI رجسٹر_demovfs(
  Client_ڈیٹا client_ڈیٹا, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_رجسٹر(sqlite3_demovfs(), 1);
  واپس TCL_OK;
}
ساکن صحیح SQLITE_TCLAPI unregister_demovfs(
  Client_ڈیٹا client_ڈیٹا, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_unregister(sqlite3_demovfs());
  واپس TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetest_demovfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_demovfs", رجسٹر_demovfs, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_demovfs", unregister_demovfs, 0, 0);
  واپس TCL_OK;
}

#ورنہ
صحیح Sqlitetest_demovfs_Init(Tcl_Interp *interp){ واپس TCL_OK; }
#اگر_ختم

#اگر_ختم /* SQLITE_TEST */
