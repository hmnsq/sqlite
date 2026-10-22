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

#যদি !defined(SQLITE_TEST) || SQLITE_OS_UNIX

#অন্তর্ভাব "sqlite3.h"

#অন্তর্ভাব <assert.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/file.h>
#অন্তর্ভাব <sys/param.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fcntl.h>

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
প্রকারনির্দেশ সংরচনা DemoFile DemoFile;
সংরচনা DemoFile {
  sqlite3_file base;              /* Base class. Must be first. */
  পূর্ণাঙ্ক fd;                         /* File descriptor */

  বর্ণ *aBuffer;                  /* Pointer to malloc'd buffer */
  পূর্ণাঙ্ক nBuffer;                    /* Valid bytes of data in zBuffer */
  sqlite3_int64 iBufferOfst;      /* Offset in file of zBuffer[0] */
};

/*
** Write directly to the file passed as the first argument. Even if the
** file has a write-buffer (DemoFile.aBuffer), ignore it.
*/
স্থির পূর্ণাঙ্ক demoDirectWrite(
  DemoFile *p,                    /* File handle */
  ধ্রুব শূন্য *zBuf,               /* Buffer containing data to write */
  পূর্ণাঙ্ক iAmt,                       /* Size of data to write in bytes */
  sqlite_int64 iOfst              /* File offset to write to */
){
  off_t ofst;                     /* Return value from lseek() */
  size_t nWrite;                  /* Return value from write() */

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  যদি( ofst!=iOfst ){
    প্রত্যাগম SQLITE_IOERR_WRITE;
  }

  nWrite = লিখ(p->fd, zBuf, iAmt);
  যদি( nWrite!=iAmt ){
    প্রত্যাগম SQLITE_IOERR_WRITE;
  }

  প্রত্যাগম SQLITE_OK;
}

/*
** Flush the contents of the DemoFile.aBuffer buffer to disk. This is a
** no-op if this particular file does not have a buffer (i.e. it is not
** a journal file) or if the buffer is currently empty.
*/
স্থির পূর্ণাঙ্ক demoFlushBuffer(DemoFile *p){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  যদি( p->nBuffer ){
    rc = demoDirectWrite(p, p->aBuffer, p->nBuffer, p->iBufferOfst);
    p->nBuffer = 0;
  }
  প্রত্যাগম rc;
}

/*
** Close a file.
*/
স্থির পূর্ণাঙ্ক demoClose(sqlite3_file *pFile){
  পূর্ণাঙ্ক rc;
  DemoFile *p = (DemoFile*)pFile;
  rc = demoFlushBuffer(p);
  sqlite3_free(p->aBuffer);
  close(p->fd);
  প্রত্যাগম rc;
}

/*
** Read data from a file.
*/
স্থির পূর্ণাঙ্ক demoRead(
  sqlite3_file *pFile, 
  শূন্য *zBuf, 
  পূর্ণাঙ্ক iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  off_t ofst;                     /* Return value from lseek() */
  পূর্ণাঙ্ক nRead;                      /* Return value from read() */
  পূর্ণাঙ্ক rc;                         /* Return code from demoFlushBuffer() */

  /* Flush any data in the write buffer to disk in case this operation
  ** is trying to read data the file-region currently cached in the buffer.
  ** It would be possible to detect this case and possibly save an 
  ** unnecessary write here, but in practice SQLite will rarely read from
  ** a journal file when there is data cached in the write-buffer.
  */
  rc = demoFlushBuffer(p);
  যদি( rc!=SQLITE_OK ){
    প্রত্যাগম rc;
  }

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  যদি( ofst!=iOfst ){
    প্রত্যাগম SQLITE_IOERR_READ;
  }
  nRead = পঠ(p->fd, zBuf, iAmt);

  যদি( nRead==iAmt ){
    প্রত্যাগম SQLITE_OK;
  }অন্যথা যদি( nRead>=0 ){
    যদি( nRead<iAmt ){
      memset(&((বর্ণ*)zBuf)[nRead], 0, iAmt-nRead);
    }
    প্রত্যাগম SQLITE_IOERR_SHORT_READ;
  }

  প্রত্যাগম SQLITE_IOERR_READ;
}

/*
** Write data to a crash-file.
*/
স্থির পূর্ণাঙ্ক demoWrite(
  sqlite3_file *pFile, 
  ধ্রুব শূন্য *zBuf, 
  পূর্ণাঙ্ক iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  
  যদি( p->aBuffer ){
    বর্ণ *z = (বর্ণ *)zBuf;       /* Pointer to remaining data to write */
    পূর্ণাঙ্ক n = iAmt;                 /* Number of bytes at z */
    sqlite3_int64 i = iOfst;      /* File offset to write to */

    যাবত্( n>0 ){
      পূর্ণাঙ্ক nCopy;                  /* Number of bytes to copy into buffer */

      /* If the buffer is full, or if this data is not being written directly
      ** following the data already buffered, flush the buffer. Flushing
      ** the buffer is a no-op if it is empty.  
      */
      যদি( p->nBuffer==SQLITE_DEMOVFS_BUFFERSZ || p->iBufferOfst+p->nBuffer!=i ){
        পূর্ণাঙ্ক rc = demoFlushBuffer(p);
        যদি( rc!=SQLITE_OK ){
          প্রত্যাগম rc;
        }
      }
      প্রতিজ্ঞা( p->nBuffer==0 || p->iBufferOfst+p->nBuffer==i );
      p->iBufferOfst = i - p->nBuffer;

      /* Copy as much data as possible into the buffer. */
      nCopy = SQLITE_DEMOVFS_BUFFERSZ - p->nBuffer;
      যদি( nCopy>n ){
        nCopy = n;
      }
      memcpy(&p->aBuffer[p->nBuffer], z, nCopy);
      p->nBuffer += nCopy;

      n -= nCopy;
      i += nCopy;
      z += nCopy;
    }
  }অন্যথা{
    প্রত্যাগম demoDirectWrite(p, zBuf, iAmt, iOfst);
  }

  প্রত্যাগম SQLITE_OK;
}

/*
** Truncate a file. This is a no-op for this VFS (see header comments at
** the top of the file).
*/
স্থির পূর্ণাঙ্ক demoTruncate(sqlite3_file *pFile, sqlite_int64 size){
#যদি 0
  যদি( ftruncate(((DemoFile *)pFile)->fd, size) ) প্রত্যাগম SQLITE_IOERR_TRUNCATE;
#endif
  প্রত্যাগম SQLITE_OK;
}

/*
** Sync the contents of the file to the persistent media.
*/
স্থির পূর্ণাঙ্ক demoSync(sqlite3_file *pFile, পূর্ণাঙ্ক flags){
  DemoFile *p = (DemoFile*)pFile;
  পূর্ণাঙ্ক rc;

  rc = demoFlushBuffer(p);
  যদি( rc!=SQLITE_OK ){
    প্রত্যাগম rc;
  }

  rc = fsync(p->fd);
  প্রত্যাগম (rc==0 ? SQLITE_OK : SQLITE_IOERR_FSYNC);
}

/*
** Write the size of the file in bytes to *pSize.
*/
স্থির পূর্ণাঙ্ক demoFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  DemoFile *p = (DemoFile*)pFile;
  পূর্ণাঙ্ক rc;                         /* Return code from fstat() call */
  সংরচনা stat sStat;              /* Output of fstat() call */

  /* Flush the contents of the buffer to disk. As with the flush in the
  ** demoRead() method, it would be possible to avoid this and save a write
  ** here and there. But in practice this comes up so infrequently it is
  ** not worth the trouble.
  */
  rc = demoFlushBuffer(p);
  যদি( rc!=SQLITE_OK ){
    প্রত্যাগম rc;
  }

  rc = fstat(p->fd, &sStat);
  যদি( rc!=0 ) প্রত্যাগম SQLITE_IOERR_FSTAT;
  *pSize = sStat.st_size;
  প্রত্যাগম SQLITE_OK;
}

/*
** Locking functions. The xLock() and xUnlock() methods are both no-ops.
** The xCheckReservedLock() always indicates that no other process holds
** a reserved lock on the database file. This ensures that if a hot-journal
** file is found in the file-system it is rolled back.
*/
স্থির পূর্ণাঙ্ক demoLock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  প্রত্যাগম SQLITE_OK;
}
স্থির পূর্ণাঙ্ক demoUnlock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  প্রত্যাগম SQLITE_OK;
}
স্থির পূর্ণাঙ্ক demoCheckReservedLock(sqlite3_file *pFile, পূর্ণাঙ্ক *pResOut){
  *pResOut = 0;
  প্রত্যাগম SQLITE_OK;
}

/*
** No xFileControl() verbs are implemented by this VFS.
*/
স্থির পূর্ণাঙ্ক demoFileControl(sqlite3_file *pFile, পূর্ণাঙ্ক op, শূন্য *pArg){
  প্রত্যাগম SQLITE_NOTFOUND;
}

/*
** The xSectorSize() and xDeviceCharacteristics() methods. These two
** may return special values allowing SQLite to optimize file-system 
** access to some extent. But it is also safe to simply return 0.
*/
স্থির পূর্ণাঙ্ক demoSectorSize(sqlite3_file *pFile){
  প্রত্যাগম 0;
}
স্থির পূর্ণাঙ্ক demoDeviceCharacteristics(sqlite3_file *pFile){
  প্রত্যাগম 0;
}

/*
** Open a file handle.
*/
স্থির পূর্ণাঙ্ক demoOpen(
  sqlite3_vfs *pVfs,              /* VFS */
  ধ্রুব বর্ণ *zName,              /* File to open, or 0 for a temp file */
  sqlite3_file *pFile,            /* Pointer to DemoFile struct to populate */
  পূর্ণাঙ্ক flags,                      /* Input SQLITE_OPEN_XXX flags */
  পূর্ণাঙ্ক *pOutFlags                  /* Output SQLITE_OPEN_XXX flags (or NULL) */
){
  স্থির ধ্রুব sqlite3_io_methods demoio = {
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
  পূর্ণাঙ্ক oflags = 0;                 /* flags to pass to open() call */
  বর্ণ *aBuf = 0;

  যদি( zName==0 ){
    প্রত্যাগম SQLITE_IOERR;
  }

  যদি( flags&SQLITE_OPEN_MAIN_JOURNAL ){
    aBuf = (বর্ণ *)sqlite3_malloc(SQLITE_DEMOVFS_BUFFERSZ);
    যদি( !aBuf ){
      প্রত্যাগম SQLITE_NOMEM;
    }
  }

  যদি( flags&SQLITE_OPEN_EXCLUSIVE ) oflags |= O_EXCL;
  যদি( flags&SQLITE_OPEN_CREATE )    oflags |= O_CREAT;
  যদি( flags&SQLITE_OPEN_READONLY )  oflags |= O_RDONLY;
  যদি( flags&SQLITE_OPEN_READWRITE ) oflags |= O_RDWR;

  memset(p, 0, পরিমাণ(DemoFile));
  p->fd = open(zName, oflags, 0600);
  যদি( p->fd<0 ){
    sqlite3_free(aBuf);
    প্রত্যাগম SQLITE_CANTOPEN;
  }
  p->aBuffer = aBuf;

  যদি( pOutFlags ){
    *pOutFlags = flags;
  }
  p->base.pMethods = &demoio;
  প্রত্যাগম SQLITE_OK;
}

/*
** Delete the file identified by argument zPath. If the dirSync parameter
** is non-zero, then ensure the file-system modification to delete the
** file has been synced to disk before returning.
*/
স্থির পূর্ণাঙ্ক demoDelete(sqlite3_vfs *pVfs, ধ্রুব বর্ণ *zPath, পূর্ণাঙ্ক dirSync){
  পূর্ণাঙ্ক rc;                         /* Return code */

  rc = unlink(zPath);
  যদি( rc!=0 && errno==ENOENT ) প্রত্যাগম SQLITE_OK;

  যদি( rc==0 && dirSync ){
    পূর্ণাঙ্ক dfd;                      /* File descriptor open on directory */
    বর্ণ *zSlash;
    বর্ণ zDir[MAXPATHNAME+1];     /* Name of directory containing file zPath */

    /* Figure out the directory name from the path of the file deleted. */
    sqlite3_snprintf(MAXPATHNAME, zDir, "%s", zPath);
    zDir[MAXPATHNAME] = '\0';
    zSlash = strrchr(zDir,'/');
    যদি( zSlash ){
      /* Open a file-descriptor on the directory. Sync. Close. */
      zSlash[0] = 0;
      dfd = open(zDir, O_RDONLY, 0);
      যদি( dfd<0 ){
        rc = -1;
      }অন্যথা{
        rc = fsync(dfd);
        close(dfd);
      }
    }
  }
  প্রত্যাগম (rc==0 ? SQLITE_OK : SQLITE_IOERR_DELETE);
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
স্থির পূর্ণাঙ্ক demoAccess(
  sqlite3_vfs *pVfs, 
  ধ্রুব বর্ণ *zPath, 
  পূর্ণাঙ্ক flags, 
  পূর্ণাঙ্ক *pResOut
){
  পূর্ণাঙ্ক rc;                         /* access() return code */
  পূর্ণাঙ্ক eAccess = F_OK;             /* Second argument to access() */

  প্রতিজ্ঞা( flags==SQLITE_ACCESS_EXISTS       /* access(zPath, F_OK) */
       || flags==SQLITE_ACCESS_READ         /* access(zPath, R_OK) */
       || flags==SQLITE_ACCESS_READWRITE    /* access(zPath, R_OK|W_OK) */
  );

  যদি( flags==SQLITE_ACCESS_READWRITE ) eAccess = R_OK|W_OK;
  যদি( flags==SQLITE_ACCESS_READ )      eAccess = R_OK;

  rc = access(zPath, eAccess);
  *pResOut = (rc==0);
  প্রত্যাগম SQLITE_OK;
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
স্থির পূর্ণাঙ্ক demoFullPathname(
  sqlite3_vfs *pVfs,              /* VFS */
  ধ্রুব বর্ণ *zPath,              /* Input path (possibly a relative path) */
  পূর্ণাঙ্ক nPathOut,                   /* Size of output buffer in bytes */
  বর্ণ *zPathOut                  /* Pointer to output buffer */
){
  বর্ণ zDir[MAXPATHNAME+1];
  যদি( zPath[0]=='/' ){
    zDir[0] = '\0';
  }অন্যথা{
    যদি( getcwd(zDir, পরিমাণ(zDir))==0 ) প্রত্যাগম SQLITE_IOERR;
  }
  zDir[MAXPATHNAME] = '\0';

  sqlite3_snprintf(nPathOut, zPathOut, "%s/%s", zDir, zPath);
  zPathOut[nPathOut-1] = '\0';

  প্রত্যাগম SQLITE_OK;
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
স্থির শূন্য *demoDlOpen(sqlite3_vfs *pVfs, ধ্রুব বর্ণ *zPath){
  প্রত্যাগম 0;
}
স্থির শূন্য demoDlError(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, বর্ণ *z_Err_বার্তা){
  sqlite3_snprintf(nByte, z_Err_বার্তা, "Loadable extensions are not supported");
  z_Err_বার্তা[nByte-1] = '\0';
}
স্থির শূন্য (*demoDlSym(sqlite3_vfs *pVfs, শূন্য *pH, ধ্রুব বর্ণ *z))(শূন্য){
  প্রত্যাগম 0;
}
স্থির শূন্য demoDlClose(sqlite3_vfs *pVfs, শূন্য *pHandle){
  প্রত্যাগম;
}

/*
** Parameter zByte points to a buffer nByte bytes in size. Populate this
** buffer with pseudo-random data.
*/
স্থির পূর্ণাঙ্ক demoRandomness(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, বর্ণ *zByte){
  প্রত্যাগম SQLITE_OK;
}

/*
** Sleep for at least nMicro microseconds. Return the (approximate) number 
** of microseconds slept for.
*/
স্থির পূর্ণাঙ্ক demoSleep(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nMicro){
  sleep(nMicro / 1000000);
  usleep(nMicro % 1000000);
  প্রত্যাগম nMicro;
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
স্থির পূর্ণাঙ্ক demoCurrentTime(sqlite3_vfs *pVfs, দ্বিগুণ *pTime){
  time_t t = time(0);
  *pTime = t/86400.0 + 2440587.5; 
  প্রত্যাগম SQLITE_OK;
}

/*
** This function returns a pointer to the VFS implemented in this file.
** To make the VFS available to SQLite:
**
**   sqlite3_vfs_register(sqlite3_demovfs(), 0);
*/
sqlite3_vfs *sqlite3_demovfs(শূন্য){
  স্থির sqlite3_vfs demovfs = {
    1,                            /* iVersion */
    পরিমাণ(DemoFile),             /* szOsFile */
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
  প্রত্যাগম &demovfs;
}

#endif /* !defined(SQLITE_TEST) || SQLITE_OS_UNIX */


#ifdef SQLITE_TEST

#অন্তর্ভাব "tclsqlite.h"

#যদি SQLITE_OS_UNIX
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI register_demovfs(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণাঙ্ক objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_register(sqlite3_demovfs(), 1);
  প্রত্যাগম TCL_OK;
}
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI unregister_demovfs(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণাঙ্ক objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_unregister(sqlite3_demovfs());
  প্রত্যাগম TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
পূর্ণাঙ্ক Sqlitetest_demovfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_demovfs", register_demovfs, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_demovfs", unregister_demovfs, 0, 0);
  প্রত্যাগম TCL_OK;
}

#অন্যথা
পূর্ণাঙ্ক Sqlitetest_demovfs_Init(Tcl_Interp *interp){ প্রত্যাগম TCL_OK; }
#endif

#endif /* SQLITE_TEST */
