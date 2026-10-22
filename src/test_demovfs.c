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

#എങ്കിൽ !defined(SQLITE_TEST) || SQLITE_OS_UNIX

#അന്തര്ഭാവ "sqlite3.h"

#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/file.h>
#അന്തര്ഭാവ <sys/param.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>

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
തരനിർവചനം ഘടന DemoFile DemoFile;
ഘടന DemoFile {
  sqlite3_file base;              /* Base class. Must be first. */
  പൂർണ്ണസംഖ്യ fd;                         /* File descriptor */

  അക്ഷരം *aBuffer;                  /* Pointer to malloc'd buffer */
  പൂർണ്ണസംഖ്യ nBuffer;                    /* Valid bytes of data in zBuffer */
  sqlite3_int64 iBufferOfst;      /* Offset in file of zBuffer[0] */
};

/*
** Write directly to the file passed as the first argument. Even if the
** file has a write-buffer (DemoFile.aBuffer), ignore it.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoDirectWrite(
  DemoFile *p,                    /* File handle */
  സ്ഥിരം ശൂന്യം *zBuf,               /* Buffer containing data to write */
  പൂർണ്ണസംഖ്യ iAmt,                       /* Size of data to write in bytes */
  sqlite_int64 iOfst              /* File offset to write to */
){
  off_t ofst;                     /* Return value from lseek() */
  size_t nWrite;                  /* Return value from write() */

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  എങ്കിൽ( ofst!=iOfst ){
    തിരികെ SQLITE_IOERR_WRITE;
  }

  nWrite = ലിഖ(p->fd, zBuf, iAmt);
  എങ്കിൽ( nWrite!=iAmt ){
    തിരികെ SQLITE_IOERR_WRITE;
  }

  തിരികെ SQLITE_OK;
}

/*
** Flush the contents of the DemoFile.aBuffer buffer to disk. This is a
** no-op if this particular file does not have a buffer (i.e. it is not
** a journal file) or if the buffer is currently empty.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoFlushBuffer(DemoFile *p){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  എങ്കിൽ( p->nBuffer ){
    rc = demoDirectWrite(p, p->aBuffer, p->nBuffer, p->iBufferOfst);
    p->nBuffer = 0;
  }
  തിരികെ rc;
}

/*
** Close a file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoClose(sqlite3_file *pFile){
  പൂർണ്ണസംഖ്യ rc;
  DemoFile *p = (DemoFile*)pFile;
  rc = demoFlushBuffer(p);
  sqlite3_free(p->aBuffer);
  close(p->fd);
  തിരികെ rc;
}

/*
** Read data from a file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoRead(
  sqlite3_file *pFile, 
  ശൂന്യം *zBuf, 
  പൂർണ്ണസംഖ്യ iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  off_t ofst;                     /* Return value from lseek() */
  പൂർണ്ണസംഖ്യ nRead;                      /* Return value from read() */
  പൂർണ്ണസംഖ്യ rc;                         /* Return code from demoFlushBuffer() */

  /* Flush any data in the write buffer to disk in case this operation
  ** is trying to read data the file-region currently cached in the buffer.
  ** It would be possible to detect this case and possibly save an 
  ** unnecessary write here, but in practice SQLite will rarely read from
  ** a journal file when there is data cached in the write-buffer.
  */
  rc = demoFlushBuffer(p);
  എങ്കിൽ( rc!=SQLITE_OK ){
    തിരികെ rc;
  }

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  എങ്കിൽ( ofst!=iOfst ){
    തിരികെ SQLITE_IOERR_READ;
  }
  nRead = പഠ(p->fd, zBuf, iAmt);

  എങ്കിൽ( nRead==iAmt ){
    തിരികെ SQLITE_OK;
  }അല്ലെങ്കിൽ എങ്കിൽ( nRead>=0 ){
    എങ്കിൽ( nRead<iAmt ){
      memset(&((അക്ഷരം*)zBuf)[nRead], 0, iAmt-nRead);
    }
    തിരികെ SQLITE_IOERR_SHORT_READ;
  }

  തിരികെ SQLITE_IOERR_READ;
}

/*
** Write data to a crash-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoWrite(
  sqlite3_file *pFile, 
  സ്ഥിരം ശൂന്യം *zBuf, 
  പൂർണ്ണസംഖ്യ iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  
  എങ്കിൽ( p->aBuffer ){
    അക്ഷരം *z = (അക്ഷരം *)zBuf;       /* Pointer to remaining data to write */
    പൂർണ്ണസംഖ്യ n = iAmt;                 /* Number of bytes at z */
    sqlite3_int64 i = iOfst;      /* File offset to write to */

    ആകുമ്പോൾ( n>0 ){
      പൂർണ്ണസംഖ്യ nCopy;                  /* Number of bytes to copy into buffer */

      /* If the buffer is full, or if this data is not being written directly
      ** following the data already buffered, flush the buffer. Flushing
      ** the buffer is a no-op if it is empty.  
      */
      എങ്കിൽ( p->nBuffer==SQLITE_DEMOVFS_BUFFERSZ || p->iBufferOfst+p->nBuffer!=i ){
        പൂർണ്ണസംഖ്യ rc = demoFlushBuffer(p);
        എങ്കിൽ( rc!=SQLITE_OK ){
          തിരികെ rc;
        }
      }
      പ്രതിജ്ഞാ( p->nBuffer==0 || p->iBufferOfst+p->nBuffer==i );
      p->iBufferOfst = i - p->nBuffer;

      /* Copy as much data as possible into the buffer. */
      nCopy = SQLITE_DEMOVFS_BUFFERSZ - p->nBuffer;
      എങ്കിൽ( nCopy>n ){
        nCopy = n;
      }
      memcpy(&p->aBuffer[p->nBuffer], z, nCopy);
      p->nBuffer += nCopy;

      n -= nCopy;
      i += nCopy;
      z += nCopy;
    }
  }അല്ലെങ്കിൽ{
    തിരികെ demoDirectWrite(p, zBuf, iAmt, iOfst);
  }

  തിരികെ SQLITE_OK;
}

/*
** Truncate a file. This is a no-op for this VFS (see header comments at
** the top of the file).
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoTruncate(sqlite3_file *pFile, sqlite_int64 size){
#എങ്കിൽ 0
  എങ്കിൽ( ftruncate(((DemoFile *)pFile)->fd, size) ) തിരികെ SQLITE_IOERR_TRUNCATE;
#endif
  തിരികെ SQLITE_OK;
}

/*
** Sync the contents of the file to the persistent media.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoSync(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ flags){
  DemoFile *p = (DemoFile*)pFile;
  പൂർണ്ണസംഖ്യ rc;

  rc = demoFlushBuffer(p);
  എങ്കിൽ( rc!=SQLITE_OK ){
    തിരികെ rc;
  }

  rc = fsync(p->fd);
  തിരികെ (rc==0 ? SQLITE_OK : SQLITE_IOERR_FSYNC);
}

/*
** Write the size of the file in bytes to *pSize.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  DemoFile *p = (DemoFile*)pFile;
  പൂർണ്ണസംഖ്യ rc;                         /* Return code from fstat() call */
  ഘടന stat sStat;              /* Output of fstat() call */

  /* Flush the contents of the buffer to disk. As with the flush in the
  ** demoRead() method, it would be possible to avoid this and save a write
  ** here and there. But in practice this comes up so infrequently it is
  ** not worth the trouble.
  */
  rc = demoFlushBuffer(p);
  എങ്കിൽ( rc!=SQLITE_OK ){
    തിരികെ rc;
  }

  rc = fstat(p->fd, &sStat);
  എങ്കിൽ( rc!=0 ) തിരികെ SQLITE_IOERR_FSTAT;
  *pSize = sStat.st_size;
  തിരികെ SQLITE_OK;
}

/*
** Locking functions. The xLock() and xUnlock() methods are both no-ops.
** The xCheckReservedLock() always indicates that no other process holds
** a reserved lock on the database file. This ensures that if a hot-journal
** file is found in the file-system it is rolled back.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ eLock){
  തിരികെ SQLITE_OK;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ demoUnlock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ eLock){
  തിരികെ SQLITE_OK;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ demoCheckReservedLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ *pResOut){
  *pResOut = 0;
  തിരികെ SQLITE_OK;
}

/*
** No xFileControl() verbs are implemented by this VFS.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoFileControl(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg){
  തിരികെ SQLITE_NOTFOUND;
}

/*
** The xSectorSize() and xDeviceCharacteristics() methods. These two
** may return special values allowing SQLite to optimize file-system 
** access to some extent. But it is also safe to simply return 0.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoSectorSize(sqlite3_file *pFile){
  തിരികെ 0;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ demoDeviceCharacteristics(sqlite3_file *pFile){
  തിരികെ 0;
}

/*
** Open a file handle.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoOpen(
  sqlite3_vfs *pVfs,              /* VFS */
  സ്ഥിരം അക്ഷരം *zName,              /* File to open, or 0 for a temp file */
  sqlite3_file *pFile,            /* Pointer to DemoFile struct to populate */
  പൂർണ്ണസംഖ്യ flags,                      /* Input SQLITE_OPEN_XXX flags */
  പൂർണ്ണസംഖ്യ *pOutFlags                  /* Output SQLITE_OPEN_XXX flags (or NULL) */
){
  സ്ഥിതം സ്ഥിരം sqlite3_io_methods demoio = {
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
  പൂർണ്ണസംഖ്യ oflags = 0;                 /* flags to pass to open() call */
  അക്ഷരം *aBuf = 0;

  എങ്കിൽ( zName==0 ){
    തിരികെ SQLITE_IOERR;
  }

  എങ്കിൽ( flags&SQLITE_OPEN_MAIN_JOURNAL ){
    aBuf = (അക്ഷരം *)sqlite3_malloc(SQLITE_DEMOVFS_BUFFERSZ);
    എങ്കിൽ( !aBuf ){
      തിരികെ SQLITE_NOMEM;
    }
  }

  എങ്കിൽ( flags&SQLITE_OPEN_EXCLUSIVE ) oflags |= O_EXCL;
  എങ്കിൽ( flags&SQLITE_OPEN_CREATE )    oflags |= O_CREAT;
  എങ്കിൽ( flags&SQLITE_OPEN_READONLY )  oflags |= O_RDONLY;
  എങ്കിൽ( flags&SQLITE_OPEN_READWRITE ) oflags |= O_RDWR;

  memset(p, 0, വലുപ്പം(DemoFile));
  p->fd = open(zName, oflags, 0600);
  എങ്കിൽ( p->fd<0 ){
    sqlite3_free(aBuf);
    തിരികെ SQLITE_CANTOPEN;
  }
  p->aBuffer = aBuf;

  എങ്കിൽ( pOutFlags ){
    *pOutFlags = flags;
  }
  p->base.pMethods = &demoio;
  തിരികെ SQLITE_OK;
}

/*
** Delete the file identified by argument zPath. If the dirSync parameter
** is non-zero, then ensure the file-system modification to delete the
** file has been synced to disk before returning.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoDelete(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath, പൂർണ്ണസംഖ്യ dirSync){
  പൂർണ്ണസംഖ്യ rc;                         /* Return code */

  rc = unlink(zPath);
  എങ്കിൽ( rc!=0 && errno==ENOENT ) തിരികെ SQLITE_OK;

  എങ്കിൽ( rc==0 && dirSync ){
    പൂർണ്ണസംഖ്യ dfd;                      /* File descriptor open on directory */
    അക്ഷരം *zSlash;
    അക്ഷരം zDir[MAXPATHNAME+1];     /* Name of directory containing file zPath */

    /* Figure out the directory name from the path of the file deleted. */
    sqlite3_snprintf(MAXPATHNAME, zDir, "%s", zPath);
    zDir[MAXPATHNAME] = '\0';
    zSlash = strrchr(zDir,'/');
    എങ്കിൽ( zSlash ){
      /* Open a file-descriptor on the directory. Sync. Close. */
      zSlash[0] = 0;
      dfd = open(zDir, O_RDONLY, 0);
      എങ്കിൽ( dfd<0 ){
        rc = -1;
      }അല്ലെങ്കിൽ{
        rc = fsync(dfd);
        close(dfd);
      }
    }
  }
  തിരികെ (rc==0 ? SQLITE_OK : SQLITE_IOERR_DELETE);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ demoAccess(
  sqlite3_vfs *pVfs, 
  സ്ഥിരം അക്ഷരം *zPath, 
  പൂർണ്ണസംഖ്യ flags, 
  പൂർണ്ണസംഖ്യ *pResOut
){
  പൂർണ്ണസംഖ്യ rc;                         /* access() return code */
  പൂർണ്ണസംഖ്യ eAccess = F_OK;             /* Second argument to access() */

  പ്രതിജ്ഞാ( flags==SQLITE_ACCESS_EXISTS       /* access(zPath, F_OK) */
       || flags==SQLITE_ACCESS_READ         /* access(zPath, R_OK) */
       || flags==SQLITE_ACCESS_READWRITE    /* access(zPath, R_OK|W_OK) */
  );

  എങ്കിൽ( flags==SQLITE_ACCESS_READWRITE ) eAccess = R_OK|W_OK;
  എങ്കിൽ( flags==SQLITE_ACCESS_READ )      eAccess = R_OK;

  rc = access(zPath, eAccess);
  *pResOut = (rc==0);
  തിരികെ SQLITE_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ demoFullPathname(
  sqlite3_vfs *pVfs,              /* VFS */
  സ്ഥിരം അക്ഷരം *zPath,              /* Input path (possibly a relative path) */
  പൂർണ്ണസംഖ്യ nPathOut,                   /* Size of output buffer in bytes */
  അക്ഷരം *zPathOut                  /* Pointer to output buffer */
){
  അക്ഷരം zDir[MAXPATHNAME+1];
  എങ്കിൽ( zPath[0]=='/' ){
    zDir[0] = '\0';
  }അല്ലെങ്കിൽ{
    എങ്കിൽ( getcwd(zDir, വലുപ്പം(zDir))==0 ) തിരികെ SQLITE_IOERR;
  }
  zDir[MAXPATHNAME] = '\0';

  sqlite3_snprintf(nPathOut, zPathOut, "%s/%s", zDir, zPath);
  zPathOut[nPathOut-1] = '\0';

  തിരികെ SQLITE_OK;
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
സ്ഥിതം ശൂന്യം *demoDlOpen(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath){
  തിരികെ 0;
}
സ്ഥിതം ശൂന്യം demoDlError(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *z_Err_സന്ദേശം){
  sqlite3_snprintf(nByte, z_Err_സന്ദേശം, "Loadable extensions are not supported");
  z_Err_സന്ദേശം[nByte-1] = '\0';
}
സ്ഥിതം ശൂന്യം (*demoDlSym(sqlite3_vfs *pVfs, ശൂന്യം *pH, സ്ഥിരം അക്ഷരം *z))(ശൂന്യം){
  തിരികെ 0;
}
സ്ഥിതം ശൂന്യം demoDlClose(sqlite3_vfs *pVfs, ശൂന്യം *pHandle){
  തിരികെ;
}

/*
** Parameter zByte points to a buffer nByte bytes in size. Populate this
** buffer with pseudo-random data.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoRandomness(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zByte){
  തിരികെ SQLITE_OK;
}

/*
** Sleep for at least nMicro microseconds. Return the (approximate) number 
** of microseconds slept for.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ demoSleep(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nMicro){
  sleep(nMicro / 1000000);
  usleep(nMicro % 1000000);
  തിരികെ nMicro;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ demoCurrentTime(sqlite3_vfs *pVfs, ഇരട്ട *pTime){
  time_t t = time(0);
  *pTime = t/86400.0 + 2440587.5; 
  തിരികെ SQLITE_OK;
}

/*
** This function returns a pointer to the VFS implemented in this file.
** To make the VFS available to SQLite:
**
**   sqlite3_vfs_register(sqlite3_demovfs(), 0);
*/
sqlite3_vfs *sqlite3_demovfs(ശൂന്യം){
  സ്ഥിതം sqlite3_vfs demovfs = {
    1,                            /* iVersion */
    വലുപ്പം(DemoFile),             /* szOsFile */
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
  തിരികെ &demovfs;
}

#endif /* !defined(SQLITE_TEST) || SQLITE_OS_UNIX */


#ifdef SQLITE_TEST

#അന്തര്ഭാവ "tclsqlite.h"

#എങ്കിൽ SQLITE_OS_UNIX
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI register_demovfs(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_register(sqlite3_demovfs(), 1);
  തിരികെ TCL_OK;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI unregister_demovfs(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_unregister(sqlite3_demovfs());
  തിരികെ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetest_demovfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_demovfs", register_demovfs, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_demovfs", unregister_demovfs, 0, 0);
  തിരികെ TCL_OK;
}

#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ Sqlitetest_demovfs_Init(Tcl_Interp *interp){ തിരികെ TCL_OK; }
#endif

#endif /* SQLITE_TEST */
