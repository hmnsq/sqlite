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

#जइ !defined(SQLITE_TEST) || SQLITE_OS_UNIX

#अन्तर्भाव "sqlite3.h"

#अन्तर्भाव <assert.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/file.h>
#अन्तर्भाव <sys/param.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>

/*
** Size of the write buffer used by journal files in bytes.
*/
#ifndef SQLITE_DEMOVFS_BUFFERSZ
# define SQLITE_DEMOVFS_BUFFERSZ 8192
#endif

/*
** The maximum pathname length supported by this VFS.
*/
#define MAXPATHNAME 512

/*
** When using this VFS, the sqlite3_file* handles that SQLite uses are
** actually pointers to instances of type DemoFile.
*/
प्रकारनिर्देश संरचना DemoFile DemoFile;
संरचना DemoFile {
  sqlite3_file base;              /* Base class. Must be first. */
  पूर्णाङ्क fd;                         /* File descriptor */

  वर्ण *aBuffer;                  /* Pointer to malloc'd buffer */
  पूर्णाङ्क nBuffer;                    /* Valid bytes of data in zBuffer */
  sqlite3_int64 iBufferOfst;      /* Offset in file of zBuffer[0] */
};

/*
** Write directly to the file passed as the first argument. Even if the
** file has a write-buffer (DemoFile.aBuffer), ignore it.
*/
स्थिर पूर्णाङ्क demoDirectWrite(
  DemoFile *p,                    /* File handle */
  ध्रुव शून्य *zBuf,               /* Buffer containing data to write */
  पूर्णाङ्क iAmt,                       /* Size of data to write in bytes */
  sqlite_int64 iOfst              /* File offset to write to */
){
  off_t ofst;                     /* Return value from lseek() */
  size_t nWrite;                  /* Return value from write() */

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  जइ( ofst!=iOfst ){
    प्रत्यागम SQLITE_IOERR_WRITE;
  }

  nWrite = लिख(p->fd, zBuf, iAmt);
  जइ( nWrite!=iAmt ){
    प्रत्यागम SQLITE_IOERR_WRITE;
  }

  प्रत्यागम SQLITE_OK;
}

/*
** Flush the contents of the DemoFile.aBuffer buffer to disk. This is a
** no-op if this particular file does not have a buffer (i.e. it is not
** a journal file) or if the buffer is currently empty.
*/
स्थिर पूर्णाङ्क demoFlushBuffer(DemoFile *p){
  पूर्णाङ्क rc = SQLITE_OK;
  जइ( p->nBuffer ){
    rc = demoDirectWrite(p, p->aBuffer, p->nBuffer, p->iBufferOfst);
    p->nBuffer = 0;
  }
  प्रत्यागम rc;
}

/*
** Close a file.
*/
स्थिर पूर्णाङ्क demoClose(sqlite3_file *pFile){
  पूर्णाङ्क rc;
  DemoFile *p = (DemoFile*)pFile;
  rc = demoFlushBuffer(p);
  sqlite3_free(p->aBuffer);
  close(p->fd);
  प्रत्यागम rc;
}

/*
** Read data from a file.
*/
स्थिर पूर्णाङ्क demoRead(
  sqlite3_file *pFile, 
  शून्य *zBuf, 
  पूर्णाङ्क iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  off_t ofst;                     /* Return value from lseek() */
  पूर्णाङ्क nRead;                      /* Return value from read() */
  पूर्णाङ्क rc;                         /* Return code from demoFlushBuffer() */

  /* Flush any data in the write buffer to disk in case this operation
  ** is trying to read data the file-region currently cached in the buffer.
  ** It would be possible to detect this case and possibly save an 
  ** unnecessary write here, but in practice SQLite will rarely read from
  ** a journal file when there is data cached in the write-buffer.
  */
  rc = demoFlushBuffer(p);
  जइ( rc!=SQLITE_OK ){
    प्रत्यागम rc;
  }

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  जइ( ofst!=iOfst ){
    प्रत्यागम SQLITE_IOERR_READ;
  }
  nRead = पठ(p->fd, zBuf, iAmt);

  जइ( nRead==iAmt ){
    प्रत्यागम SQLITE_OK;
  }अण्णहा जइ( nRead>=0 ){
    जइ( nRead<iAmt ){
      memset(&((वर्ण*)zBuf)[nRead], 0, iAmt-nRead);
    }
    प्रत्यागम SQLITE_IOERR_SHORT_READ;
  }

  प्रत्यागम SQLITE_IOERR_READ;
}

/*
** Write data to a crash-file.
*/
स्थिर पूर्णाङ्क demoWrite(
  sqlite3_file *pFile, 
  ध्रुव शून्य *zBuf, 
  पूर्णाङ्क iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  
  जइ( p->aBuffer ){
    वर्ण *z = (वर्ण *)zBuf;       /* Pointer to remaining data to write */
    पूर्णाङ्क n = iAmt;                 /* Number of bytes at z */
    sqlite3_int64 i = iOfst;      /* File offset to write to */

    जाव( n>0 ){
      पूर्णाङ्क nCopy;                  /* Number of bytes to copy into buffer */

      /* If the buffer is full, or if this data is not being written directly
      ** following the data already buffered, flush the buffer. Flushing
      ** the buffer is a no-op if it is empty.  
      */
      जइ( p->nBuffer==SQLITE_DEMOVFS_BUFFERSZ || p->iBufferOfst+p->nBuffer!=i ){
        पूर्णाङ्क rc = demoFlushBuffer(p);
        जइ( rc!=SQLITE_OK ){
          प्रत्यागम rc;
        }
      }
      प्रतिज्ञा( p->nBuffer==0 || p->iBufferOfst+p->nBuffer==i );
      p->iBufferOfst = i - p->nBuffer;

      /* Copy as much data as possible into the buffer. */
      nCopy = SQLITE_DEMOVFS_BUFFERSZ - p->nBuffer;
      जइ( nCopy>n ){
        nCopy = n;
      }
      memcpy(&p->aBuffer[p->nBuffer], z, nCopy);
      p->nBuffer += nCopy;

      n -= nCopy;
      i += nCopy;
      z += nCopy;
    }
  }अण्णहा{
    प्रत्यागम demoDirectWrite(p, zBuf, iAmt, iOfst);
  }

  प्रत्यागम SQLITE_OK;
}

/*
** Truncate a file. This is a no-op for this VFS (see header comments at
** the top of the file).
*/
स्थिर पूर्णाङ्क demoTruncate(sqlite3_file *pFile, sqlite_int64 size){
#जइ 0
  जइ( ftruncate(((DemoFile *)pFile)->fd, size) ) प्रत्यागम SQLITE_IOERR_TRUNCATE;
#endif
  प्रत्यागम SQLITE_OK;
}

/*
** Sync the contents of the file to the persistent media.
*/
स्थिर पूर्णाङ्क demoSync(sqlite3_file *pFile, पूर्णाङ्क flags){
  DemoFile *p = (DemoFile*)pFile;
  पूर्णाङ्क rc;

  rc = demoFlushBuffer(p);
  जइ( rc!=SQLITE_OK ){
    प्रत्यागम rc;
  }

  rc = fsync(p->fd);
  प्रत्यागम (rc==0 ? SQLITE_OK : SQLITE_IOERR_FSYNC);
}

/*
** Write the size of the file in bytes to *pSize.
*/
स्थिर पूर्णाङ्क demoFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  DemoFile *p = (DemoFile*)pFile;
  पूर्णाङ्क rc;                         /* Return code from fstat() call */
  संरचना stat sStat;              /* Output of fstat() call */

  /* Flush the contents of the buffer to disk. As with the flush in the
  ** demoRead() method, it would be possible to avoid this and save a write
  ** here and there. But in practice this comes up so infrequently it is
  ** not worth the trouble.
  */
  rc = demoFlushBuffer(p);
  जइ( rc!=SQLITE_OK ){
    प्रत्यागम rc;
  }

  rc = fstat(p->fd, &sStat);
  जइ( rc!=0 ) प्रत्यागम SQLITE_IOERR_FSTAT;
  *pSize = sStat.st_size;
  प्रत्यागम SQLITE_OK;
}

/*
** Locking functions. The xLock() and xUnlock() methods are both no-ops.
** The xCheckReservedLock() always indicates that no other process holds
** a reserved lock on the database file. This ensures that if a hot-journal
** file is found in the file-system it is rolled back.
*/
स्थिर पूर्णाङ्क demoLock(sqlite3_file *pFile, पूर्णाङ्क eLock){
  प्रत्यागम SQLITE_OK;
}
स्थिर पूर्णाङ्क demoUnlock(sqlite3_file *pFile, पूर्णाङ्क eLock){
  प्रत्यागम SQLITE_OK;
}
स्थिर पूर्णाङ्क demoCheckReservedLock(sqlite3_file *pFile, पूर्णाङ्क *pResOut){
  *pResOut = 0;
  प्रत्यागम SQLITE_OK;
}

/*
** No xFileControl() verbs are implemented by this VFS.
*/
स्थिर पूर्णाङ्क demoFileControl(sqlite3_file *pFile, पूर्णाङ्क op, शून्य *pArg){
  प्रत्यागम SQLITE_NOTFOUND;
}

/*
** The xSectorSize() and xDeviceCharacteristics() methods. These two
** may return special values allowing SQLite to optimize file-system 
** access to some extent. But it is also safe to simply return 0.
*/
स्थिर पूर्णाङ्क demoSectorSize(sqlite3_file *pFile){
  प्रत्यागम 0;
}
स्थिर पूर्णाङ्क demoDeviceCharacteristics(sqlite3_file *pFile){
  प्रत्यागम 0;
}

/*
** Open a file handle.
*/
स्थिर पूर्णाङ्क demoOpen(
  sqlite3_vfs *pVfs,              /* VFS */
  ध्रुव वर्ण *zName,              /* File to open, or 0 for a temp file */
  sqlite3_file *pFile,            /* Pointer to DemoFile struct to populate */
  पूर्णाङ्क flags,                      /* Input SQLITE_OPEN_XXX flags */
  पूर्णाङ्क *pOutFlags                  /* Output SQLITE_OPEN_XXX flags (or NULL) */
){
  स्थिर ध्रुव sqlite3_io_methods demoio = {
    1,                            /* iVersion */
    demoClose,                    /* xClose */
    demoRead,                     /* xRead */
    demoWrite,                    /* xWrite */
    demoTruncate,                 /* xTruncate */
    demoSync,                     /* xSync */
    demoFileSize,                 /* xFileSize */
    demoLock,                     /* xLock */
    demoUnlock,                   /* xUnlock */
    demoCheckReservedLock,        /* xCheckReservedLock */
    demoFileControl,              /* xFileControl */
    demoSectorSize,               /* xSectorSize */
    demoDeviceCharacteristics     /* xDeviceCharacteristics */
  };

  DemoFile *p = (DemoFile*)pFile; /* Populate this structure */
  पूर्णाङ्क oflags = 0;                 /* flags to pass to open() call */
  वर्ण *aBuf = 0;

  जइ( zName==0 ){
    प्रत्यागम SQLITE_IOERR;
  }

  जइ( flags&SQLITE_OPEN_MAIN_JOURNAL ){
    aBuf = (वर्ण *)sqlite3_malloc(SQLITE_DEMOVFS_BUFFERSZ);
    जइ( !aBuf ){
      प्रत्यागम SQLITE_NOMEM;
    }
  }

  जइ( flags&SQLITE_OPEN_EXCLUSIVE ) oflags |= O_EXCL;
  जइ( flags&SQLITE_OPEN_CREATE )    oflags |= O_CREAT;
  जइ( flags&SQLITE_OPEN_READONLY )  oflags |= O_RDONLY;
  जइ( flags&SQLITE_OPEN_READWRITE ) oflags |= O_RDWR;

  memset(p, 0, परिमाण(DemoFile));
  p->fd = open(zName, oflags, 0600);
  जइ( p->fd<0 ){
    sqlite3_free(aBuf);
    प्रत्यागम SQLITE_CANTOPEN;
  }
  p->aBuffer = aBuf;

  जइ( pOutFlags ){
    *pOutFlags = flags;
  }
  p->base.pMethods = &demoio;
  प्रत्यागम SQLITE_OK;
}

/*
** Delete the file identified by argument zPath. If the dirSync parameter
** is non-zero, then ensure the file-system modification to delete the
** file has been synced to disk before returning.
*/
स्थिर पूर्णाङ्क demoDelete(sqlite3_vfs *pVfs, ध्रुव वर्ण *zPath, पूर्णाङ्क dirSync){
  पूर्णाङ्क rc;                         /* Return code */

  rc = unlink(zPath);
  जइ( rc!=0 && errno==ENOENT ) प्रत्यागम SQLITE_OK;

  जइ( rc==0 && dirSync ){
    पूर्णाङ्क dfd;                      /* File descriptor open on directory */
    वर्ण *zSlash;
    वर्ण zDir[MAXPATHNAME+1];     /* Name of directory containing file zPath */

    /* Figure out the directory name from the path of the file deleted. */
    sqlite3_snprintf(MAXPATHNAME, zDir, "%s", zPath);
    zDir[MAXPATHNAME] = '\0';
    zSlash = strrchr(zDir,'/');
    जइ( zSlash ){
      /* Open a file-descriptor on the directory. Sync. Close. */
      zSlash[0] = 0;
      dfd = open(zDir, O_RDONLY, 0);
      जइ( dfd<0 ){
        rc = -1;
      }अण्णहा{
        rc = fsync(dfd);
        close(dfd);
      }
    }
  }
  प्रत्यागम (rc==0 ? SQLITE_OK : SQLITE_IOERR_DELETE);
}

#ifndef F_OK
# define F_OK 0
#endif
#ifndef R_OK
# define R_OK 4
#endif
#ifndef W_OK
# define W_OK 2
#endif

/*
** Query the file-system to see if the named file exists, is readable or
** is both readable and writable.
*/
स्थिर पूर्णाङ्क demoAccess(
  sqlite3_vfs *pVfs, 
  ध्रुव वर्ण *zPath, 
  पूर्णाङ्क flags, 
  पूर्णाङ्क *pResOut
){
  पूर्णाङ्क rc;                         /* access() return code */
  पूर्णाङ्क eAccess = F_OK;             /* Second argument to access() */

  प्रतिज्ञा( flags==SQLITE_ACCESS_EXISTS       /* access(zPath, F_OK) */
       || flags==SQLITE_ACCESS_READ         /* access(zPath, R_OK) */
       || flags==SQLITE_ACCESS_READWRITE    /* access(zPath, R_OK|W_OK) */
  );

  जइ( flags==SQLITE_ACCESS_READWRITE ) eAccess = R_OK|W_OK;
  जइ( flags==SQLITE_ACCESS_READ )      eAccess = R_OK;

  rc = access(zPath, eAccess);
  *pResOut = (rc==0);
  प्रत्यागम SQLITE_OK;
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
स्थिर पूर्णाङ्क demoFullPathname(
  sqlite3_vfs *pVfs,              /* VFS */
  ध्रुव वर्ण *zPath,              /* Input path (possibly a relative path) */
  पूर्णाङ्क nPathOut,                   /* Size of output buffer in bytes */
  वर्ण *zPathOut                  /* Pointer to output buffer */
){
  वर्ण zDir[MAXPATHNAME+1];
  जइ( zPath[0]=='/' ){
    zDir[0] = '\0';
  }अण्णहा{
    जइ( getcwd(zDir, परिमाण(zDir))==0 ) प्रत्यागम SQLITE_IOERR;
  }
  zDir[MAXPATHNAME] = '\0';

  sqlite3_snprintf(nPathOut, zPathOut, "%s/%s", zDir, zPath);
  zPathOut[nPathOut-1] = '\0';

  प्रत्यागम SQLITE_OK;
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
स्थिर शून्य *demoDlOpen(sqlite3_vfs *pVfs, ध्रुव वर्ण *zPath){
  प्रत्यागम 0;
}
स्थिर शून्य demoDlError(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *z_Err_सन्देशः){
  sqlite3_snprintf(nByte, z_Err_सन्देशः, "Loadable extensions are not supported");
  z_Err_सन्देशः[nByte-1] = '\0';
}
स्थिर शून्य (*demoDlSym(sqlite3_vfs *pVfs, शून्य *pH, ध्रुव वर्ण *z))(शून्य){
  प्रत्यागम 0;
}
स्थिर शून्य demoDlClose(sqlite3_vfs *pVfs, शून्य *pHandle){
  प्रत्यागम;
}

/*
** Parameter zByte points to a buffer nByte bytes in size. Populate this
** buffer with pseudo-random data.
*/
स्थिर पूर्णाङ्क demoRandomness(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *zByte){
  प्रत्यागम SQLITE_OK;
}

/*
** Sleep for at least nMicro microseconds. Return the (approximate) number 
** of microseconds slept for.
*/
स्थिर पूर्णाङ्क demoSleep(sqlite3_vfs *pVfs, पूर्णाङ्क nMicro){
  sleep(nMicro / 1000000);
  usleep(nMicro % 1000000);
  प्रत्यागम nMicro;
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
स्थिर पूर्णाङ्क demoCurrentTime(sqlite3_vfs *pVfs, द्विगुण *pTime){
  time_t t = time(0);
  *pTime = t/86400.0 + 2440587.5; 
  प्रत्यागम SQLITE_OK;
}

/*
** This function returns a pointer to the VFS implemented in this file.
** To make the VFS available to SQLite:
**
**   sqlite3_vfs_register(sqlite3_demovfs(), 0);
*/
sqlite3_vfs *sqlite3_demovfs(शून्य){
  स्थिर sqlite3_vfs demovfs = {
    1,                            /* iVersion */
    परिमाण(DemoFile),             /* szOsFile */
    MAXPATHNAME,                  /* mxPathname */
    0,                            /* pNext */
    "demo",                       /* zName */
    0,                            /* pAppData */
    demoOpen,                     /* xOpen */
    demoDelete,                   /* xDelete */
    demoAccess,                   /* xAccess */
    demoFullPathname,             /* xFullPathname */
    demoDlOpen,                   /* xDlOpen */
    demoDlError,                  /* xDlError */
    demoDlSym,                    /* xDlSym */
    demoDlClose,                  /* xDlClose */
    demoRandomness,               /* xRandomness */
    demoSleep,                    /* xSleep */
    demoCurrentTime,              /* xCurrentTime */
  };
  प्रत्यागम &demovfs;
}

#endif /* !defined(SQLITE_TEST) || SQLITE_OS_UNIX */


#ifdef SQLITE_TEST

#अन्तर्भाव "tclsqlite.h"

#जइ SQLITE_OS_UNIX
स्थिर पूर्णाङ्क SQLITE_TCLAPI register_demovfs(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_register(sqlite3_demovfs(), 1);
  प्रत्यागम TCL_OK;
}
स्थिर पूर्णाङ्क SQLITE_TCLAPI unregister_demovfs(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_unregister(sqlite3_demovfs());
  प्रत्यागम TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetest_demovfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_demovfs", register_demovfs, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_demovfs", unregister_demovfs, 0, 0);
  प्रत्यागम TCL_OK;
}

#अण्णहा
पूर्णाङ्क Sqlitetest_demovfs_Init(Tcl_Interp *interp){ प्रत्यागम TCL_OK; }
#endif

#endif /* SQLITE_TEST */
