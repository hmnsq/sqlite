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

#જો !defined(SQLITE_TEST) || SQLITE_OS_UNIX

#અન્તર્ભાવ "sqlite3.h"

#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/file.h>
#અન્તર્ભાવ <sys/param.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>

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
પ્રકારવ્યાખ્યા સંરચના DemoFile DemoFile;
સંરચના DemoFile {
  sqlite3_file base;              /* Base class. Must be first. */
  પૂર્ણાંક fd;                         /* File descriptor */

  અક્ષર *aBuffer;                  /* Pointer to malloc'd buffer */
  પૂર્ણાંક nBuffer;                    /* Valid bytes of data in zBuffer */
  sqlite3_int64 iBufferOfst;      /* Offset in file of zBuffer[0] */
};

/*
** Write directly to the file passed as the first argument. Even if the
** file has a write-buffer (DemoFile.aBuffer), ignore it.
*/
સ્થિર પૂર્ણાંક demoDirectWrite(
  DemoFile *p,                    /* File handle */
  અચલ શૂન્ય *zBuf,               /* Buffer containing data to write */
  પૂર્ણાંક iAmt,                       /* Size of data to write in bytes */
  sqlite_int64 iOfst              /* File offset to write to */
){
  off_t ofst;                     /* Return value from lseek() */
  size_t nWrite;                  /* Return value from write() */

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  જો( ofst!=iOfst ){
    પાછા SQLITE_IOERR_WRITE;
  }

  nWrite = લિખ(p->fd, zBuf, iAmt);
  જો( nWrite!=iAmt ){
    પાછા SQLITE_IOERR_WRITE;
  }

  પાછા SQLITE_OK;
}

/*
** Flush the contents of the DemoFile.aBuffer buffer to disk. This is a
** no-op if this particular file does not have a buffer (i.e. it is not
** a journal file) or if the buffer is currently empty.
*/
સ્થિર પૂર્ણાંક demoFlushBuffer(DemoFile *p){
  પૂર્ણાંક rc = SQLITE_OK;
  જો( p->nBuffer ){
    rc = demoDirectWrite(p, p->aBuffer, p->nBuffer, p->iBufferOfst);
    p->nBuffer = 0;
  }
  પાછા rc;
}

/*
** Close a file.
*/
સ્થિર પૂર્ણાંક demoClose(sqlite3_file *pFile){
  પૂર્ણાંક rc;
  DemoFile *p = (DemoFile*)pFile;
  rc = demoFlushBuffer(p);
  sqlite3_free(p->aBuffer);
  close(p->fd);
  પાછા rc;
}

/*
** Read data from a file.
*/
સ્થિર પૂર્ણાંક demoRead(
  sqlite3_file *pFile, 
  શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  off_t ofst;                     /* Return value from lseek() */
  પૂર્ણાંક nRead;                      /* Return value from read() */
  પૂર્ણાંક rc;                         /* Return code from demoFlushBuffer() */

  /* Flush any data in the write buffer to disk in case this operation
  ** is trying to read data the file-region currently cached in the buffer.
  ** It would be possible to detect this case and possibly save an 
  ** unnecessary write here, but in practice SQLite will rarely read from
  ** a journal file when there is data cached in the write-buffer.
  */
  rc = demoFlushBuffer(p);
  જો( rc!=SQLITE_OK ){
    પાછા rc;
  }

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  જો( ofst!=iOfst ){
    પાછા SQLITE_IOERR_READ;
  }
  nRead = પઠ(p->fd, zBuf, iAmt);

  જો( nRead==iAmt ){
    પાછા SQLITE_OK;
  }નહીંતર જો( nRead>=0 ){
    જો( nRead<iAmt ){
      memset(&((અક્ષર*)zBuf)[nRead], 0, iAmt-nRead);
    }
    પાછા SQLITE_IOERR_SHORT_READ;
  }

  પાછા SQLITE_IOERR_READ;
}

/*
** Write data to a crash-file.
*/
સ્થિર પૂર્ણાંક demoWrite(
  sqlite3_file *pFile, 
  અચલ શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  
  જો( p->aBuffer ){
    અક્ષર *z = (અક્ષર *)zBuf;       /* Pointer to remaining data to write */
    પૂર્ણાંક n = iAmt;                 /* Number of bytes at z */
    sqlite3_int64 i = iOfst;      /* File offset to write to */

    જ્યારે( n>0 ){
      પૂર્ણાંક nCopy;                  /* Number of bytes to copy into buffer */

      /* If the buffer is full, or if this data is not being written directly
      ** following the data already buffered, flush the buffer. Flushing
      ** the buffer is a no-op if it is empty.  
      */
      જો( p->nBuffer==SQLITE_DEMOVFS_BUFFERSZ || p->iBufferOfst+p->nBuffer!=i ){
        પૂર્ણાંક rc = demoFlushBuffer(p);
        જો( rc!=SQLITE_OK ){
          પાછા rc;
        }
      }
      પ્રતિજ્ઞા( p->nBuffer==0 || p->iBufferOfst+p->nBuffer==i );
      p->iBufferOfst = i - p->nBuffer;

      /* Copy as much data as possible into the buffer. */
      nCopy = SQLITE_DEMOVFS_BUFFERSZ - p->nBuffer;
      જો( nCopy>n ){
        nCopy = n;
      }
      memcpy(&p->aBuffer[p->nBuffer], z, nCopy);
      p->nBuffer += nCopy;

      n -= nCopy;
      i += nCopy;
      z += nCopy;
    }
  }નહીંતર{
    પાછા demoDirectWrite(p, zBuf, iAmt, iOfst);
  }

  પાછા SQLITE_OK;
}

/*
** Truncate a file. This is a no-op for this VFS (see header comments at
** the top of the file).
*/
સ્થિર પૂર્ણાંક demoTruncate(sqlite3_file *pFile, sqlite_int64 size){
#જો 0
  જો( ftruncate(((DemoFile *)pFile)->fd, size) ) પાછા SQLITE_IOERR_TRUNCATE;
#endif
  પાછા SQLITE_OK;
}

/*
** Sync the contents of the file to the persistent media.
*/
સ્થિર પૂર્ણાંક demoSync(sqlite3_file *pFile, પૂર્ણાંક flags){
  DemoFile *p = (DemoFile*)pFile;
  પૂર્ણાંક rc;

  rc = demoFlushBuffer(p);
  જો( rc!=SQLITE_OK ){
    પાછા rc;
  }

  rc = fsync(p->fd);
  પાછા (rc==0 ? SQLITE_OK : SQLITE_IOERR_FSYNC);
}

/*
** Write the size of the file in bytes to *pSize.
*/
સ્થિર પૂર્ણાંક demoFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  DemoFile *p = (DemoFile*)pFile;
  પૂર્ણાંક rc;                         /* Return code from fstat() call */
  સંરચના stat sStat;              /* Output of fstat() call */

  /* Flush the contents of the buffer to disk. As with the flush in the
  ** demoRead() method, it would be possible to avoid this and save a write
  ** here and there. But in practice this comes up so infrequently it is
  ** not worth the trouble.
  */
  rc = demoFlushBuffer(p);
  જો( rc!=SQLITE_OK ){
    પાછા rc;
  }

  rc = fstat(p->fd, &sStat);
  જો( rc!=0 ) પાછા SQLITE_IOERR_FSTAT;
  *pSize = sStat.st_size;
  પાછા SQLITE_OK;
}

/*
** Locking functions. The xLock() and xUnlock() methods are both no-ops.
** The xCheckReservedLock() always indicates that no other process holds
** a reserved lock on the database file. This ensures that if a hot-journal
** file is found in the file-system it is rolled back.
*/
સ્થિર પૂર્ણાંક demoLock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  પાછા SQLITE_OK;
}
સ્થિર પૂર્ણાંક demoUnlock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  પાછા SQLITE_OK;
}
સ્થિર પૂર્ણાંક demoCheckReservedLock(sqlite3_file *pFile, પૂર્ણાંક *pResOut){
  *pResOut = 0;
  પાછા SQLITE_OK;
}

/*
** No xFileControl() verbs are implemented by this VFS.
*/
સ્થિર પૂર્ણાંક demoFileControl(sqlite3_file *pFile, પૂર્ણાંક op, શૂન્ય *pArg){
  પાછા SQLITE_NOTFOUND;
}

/*
** The xSectorSize() and xDeviceCharacteristics() methods. These two
** may return special values allowing SQLite to optimize file-system 
** access to some extent. But it is also safe to simply return 0.
*/
સ્થિર પૂર્ણાંક demoSectorSize(sqlite3_file *pFile){
  પાછા 0;
}
સ્થિર પૂર્ણાંક demoDeviceCharacteristics(sqlite3_file *pFile){
  પાછા 0;
}

/*
** Open a file handle.
*/
સ્થિર પૂર્ણાંક demoOpen(
  sqlite3_vfs *pVfs,              /* VFS */
  અચલ અક્ષર *zName,              /* File to open, or 0 for a temp file */
  sqlite3_file *pFile,            /* Pointer to DemoFile struct to populate */
  પૂર્ણાંક flags,                      /* Input SQLITE_OPEN_XXX flags */
  પૂર્ણાંક *pOutFlags                  /* Output SQLITE_OPEN_XXX flags (or NULL) */
){
  સ્થિર અચલ sqlite3_io_methods demoio = {
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
  પૂર્ણાંક oflags = 0;                 /* flags to pass to open() call */
  અક્ષર *aBuf = 0;

  જો( zName==0 ){
    પાછા SQLITE_IOERR;
  }

  જો( flags&SQLITE_OPEN_MAIN_JOURNAL ){
    aBuf = (અક્ષર *)sqlite3_malloc(SQLITE_DEMOVFS_BUFFERSZ);
    જો( !aBuf ){
      પાછા SQLITE_NOMEM;
    }
  }

  જો( flags&SQLITE_OPEN_EXCLUSIVE ) oflags |= O_EXCL;
  જો( flags&SQLITE_OPEN_CREATE )    oflags |= O_CREAT;
  જો( flags&SQLITE_OPEN_READONLY )  oflags |= O_RDONLY;
  જો( flags&SQLITE_OPEN_READWRITE ) oflags |= O_RDWR;

  memset(p, 0, કદ(DemoFile));
  p->fd = open(zName, oflags, 0600);
  જો( p->fd<0 ){
    sqlite3_free(aBuf);
    પાછા SQLITE_CANTOPEN;
  }
  p->aBuffer = aBuf;

  જો( pOutFlags ){
    *pOutFlags = flags;
  }
  p->base.pMethods = &demoio;
  પાછા SQLITE_OK;
}

/*
** Delete the file identified by argument zPath. If the dirSync parameter
** is non-zero, then ensure the file-system modification to delete the
** file has been synced to disk before returning.
*/
સ્થિર પૂર્ણાંક demoDelete(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath, પૂર્ણાંક dirSync){
  પૂર્ણાંક rc;                         /* Return code */

  rc = unlink(zPath);
  જો( rc!=0 && errno==ENOENT ) પાછા SQLITE_OK;

  જો( rc==0 && dirSync ){
    પૂર્ણાંક dfd;                      /* File descriptor open on directory */
    અક્ષર *zSlash;
    અક્ષર zDir[MAXPATHNAME+1];     /* Name of directory containing file zPath */

    /* Figure out the directory name from the path of the file deleted. */
    sqlite3_snprintf(MAXPATHNAME, zDir, "%s", zPath);
    zDir[MAXPATHNAME] = '\0';
    zSlash = strrchr(zDir,'/');
    જો( zSlash ){
      /* Open a file-descriptor on the directory. Sync. Close. */
      zSlash[0] = 0;
      dfd = open(zDir, O_RDONLY, 0);
      જો( dfd<0 ){
        rc = -1;
      }નહીંતર{
        rc = fsync(dfd);
        close(dfd);
      }
    }
  }
  પાછા (rc==0 ? SQLITE_OK : SQLITE_IOERR_DELETE);
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
સ્થિર પૂર્ણાંક demoAccess(
  sqlite3_vfs *pVfs, 
  અચલ અક્ષર *zPath, 
  પૂર્ણાંક flags, 
  પૂર્ણાંક *pResOut
){
  પૂર્ણાંક rc;                         /* access() return code */
  પૂર્ણાંક eAccess = F_OK;             /* Second argument to access() */

  પ્રતિજ્ઞા( flags==SQLITE_ACCESS_EXISTS       /* access(zPath, F_OK) */
       || flags==SQLITE_ACCESS_READ         /* access(zPath, R_OK) */
       || flags==SQLITE_ACCESS_READWRITE    /* access(zPath, R_OK|W_OK) */
  );

  જો( flags==SQLITE_ACCESS_READWRITE ) eAccess = R_OK|W_OK;
  જો( flags==SQLITE_ACCESS_READ )      eAccess = R_OK;

  rc = access(zPath, eAccess);
  *pResOut = (rc==0);
  પાછા SQLITE_OK;
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
સ્થિર પૂર્ણાંક demoFullPathname(
  sqlite3_vfs *pVfs,              /* VFS */
  અચલ અક્ષર *zPath,              /* Input path (possibly a relative path) */
  પૂર્ણાંક nPathOut,                   /* Size of output buffer in bytes */
  અક્ષર *zPathOut                  /* Pointer to output buffer */
){
  અક્ષર zDir[MAXPATHNAME+1];
  જો( zPath[0]=='/' ){
    zDir[0] = '\0';
  }નહીંતર{
    જો( getcwd(zDir, કદ(zDir))==0 ) પાછા SQLITE_IOERR;
  }
  zDir[MAXPATHNAME] = '\0';

  sqlite3_snprintf(nPathOut, zPathOut, "%s/%s", zDir, zPath);
  zPathOut[nPathOut-1] = '\0';

  પાછા SQLITE_OK;
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
સ્થિર શૂન્ય *demoDlOpen(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath){
  પાછા 0;
}
સ્થિર શૂન્ય demoDlError(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *z_Err_સંદેશ){
  sqlite3_snprintf(nByte, z_Err_સંદેશ, "Loadable extensions are not supported");
  z_Err_સંદેશ[nByte-1] = '\0';
}
સ્થિર શૂન્ય (*demoDlSym(sqlite3_vfs *pVfs, શૂન્ય *pH, અચલ અક્ષર *z))(શૂન્ય){
  પાછા 0;
}
સ્થિર શૂન્ય demoDlClose(sqlite3_vfs *pVfs, શૂન્ય *pHandle){
  પાછા;
}

/*
** Parameter zByte points to a buffer nByte bytes in size. Populate this
** buffer with pseudo-random data.
*/
સ્થિર પૂર્ણાંક demoRandomness(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *zByte){
  પાછા SQLITE_OK;
}

/*
** Sleep for at least nMicro microseconds. Return the (approximate) number 
** of microseconds slept for.
*/
સ્થિર પૂર્ણાંક demoSleep(sqlite3_vfs *pVfs, પૂર્ણાંક nMicro){
  sleep(nMicro / 1000000);
  usleep(nMicro % 1000000);
  પાછા nMicro;
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
સ્થિર પૂર્ણાંક demoCurrentTime(sqlite3_vfs *pVfs, દ્વિગુણ *pTime){
  time_t t = time(0);
  *pTime = t/86400.0 + 2440587.5; 
  પાછા SQLITE_OK;
}

/*
** This function returns a pointer to the VFS implemented in this file.
** To make the VFS available to SQLite:
**
**   sqlite3_vfs_register(sqlite3_demovfs(), 0);
*/
sqlite3_vfs *sqlite3_demovfs(શૂન્ય){
  સ્થિર sqlite3_vfs demovfs = {
    1,                            /* iVersion */
    કદ(DemoFile),             /* szOsFile */
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
  પાછા &demovfs;
}

#endif /* !defined(SQLITE_TEST) || SQLITE_OS_UNIX */


#ifdef SQLITE_TEST

#અન્તર્ભાવ "tclsqlite.h"

#જો SQLITE_OS_UNIX
સ્થિર પૂર્ણાંક SQLITE_TCLAPI register_demovfs(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_register(sqlite3_demovfs(), 1);
  પાછા TCL_OK;
}
સ્થિર પૂર્ણાંક SQLITE_TCLAPI unregister_demovfs(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_unregister(sqlite3_demovfs());
  પાછા TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetest_demovfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_demovfs", register_demovfs, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_demovfs", unregister_demovfs, 0, 0);
  પાછા TCL_OK;
}

#નહીંતર
પૂર્ણાંક Sqlitetest_demovfs_Init(Tcl_Interp *interp){ પાછા TCL_OK; }
#endif

#endif /* SQLITE_TEST */
