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
প্রকারলেখ কাঠামো DemoFile DemoFile;
কাঠামো DemoFile {
  sqlite3_file base;              /* Base class. Must be first. */
  পূর্ণসংখ্যা fd;                         /* File descriptor */

  অক্ষর *a_বাফার;                  /* Pointer to malloc'd buffer */
  পূর্ণসংখ্যা n_বাফার;                    /* Valid bytes of data in zBuffer */
  sqlite3_int64 i_বাফার_Ofst;      /* Offset in file of zBuffer[0] */
};

/*
** Write directly to the file passed as the first argument. Even if the
** file has a write-buffer (DemoFile.aBuffer), ignore it.
*/
স্থির পূর্ণসংখ্যা demoDirectWrite(
  DemoFile *p,                    /* File handle */
  ধ্রুবক শূন্য *zBuf,               /* Buffer containing data to write */
  পূর্ণসংখ্যা iAmt,                       /* Size of data to write in bytes */
  sqlite_int64 iOfst              /* File offset to write to */
){
  off_t ofst;                     /* Return value from lseek() */
  size_t nWrite;                  /* Return value from write() */

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  যদি( ofst!=iOfst ){
    ফেরত SQLITE_IOERR_WRITE;
  }

  nWrite = লিখ(p->fd, zBuf, iAmt);
  যদি( nWrite!=iAmt ){
    ফেরত SQLITE_IOERR_WRITE;
  }

  ফেরত SQLITE_OK;
}

/*
** Flush the contents of the DemoFile.aBuffer buffer to disk. This is a
** no-op if this particular file does not have a buffer (i.e. it is not
** a journal file) or if the buffer is currently empty.
*/
স্থির পূর্ণসংখ্যা demo_Flush_বাফার(DemoFile *p){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  যদি( p->n_বাফার ){
    rc = demoDirectWrite(p, p->a_বাফার, p->n_বাফার, p->i_বাফার_Ofst);
    p->n_বাফার = 0;
  }
  ফেরত rc;
}

/*
** Close a file.
*/
স্থির পূর্ণসংখ্যা demoClose(sqlite3_file *pFile){
  পূর্ণসংখ্যা rc;
  DemoFile *p = (DemoFile*)pFile;
  rc = demo_Flush_বাফার(p);
  sqlite3_free(p->a_বাফার);
  close(p->fd);
  ফেরত rc;
}

/*
** Read data from a file.
*/
স্থির পূর্ণসংখ্যা demoRead(
  sqlite3_file *pFile, 
  শূন্য *zBuf, 
  পূর্ণসংখ্যা iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  off_t ofst;                     /* Return value from lseek() */
  পূর্ণসংখ্যা nRead;                      /* Return value from read() */
  পূর্ণসংখ্যা rc;                         /* Return code from demoFlushBuffer() */

  /* Flush any data in the write buffer to disk in case this operation
  ** is trying to read data the file-region currently cached in the buffer.
  ** It would be possible to detect this case and possibly save an 
  ** unnecessary write here, but in practice SQLite will rarely read from
  ** a journal file when there is data cached in the write-buffer.
  */
  rc = demo_Flush_বাফার(p);
  যদি( rc!=SQLITE_OK ){
    ফেরত rc;
  }

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  যদি( ofst!=iOfst ){
    ফেরত SQLITE_IOERR_READ;
  }
  nRead = পঠ(p->fd, zBuf, iAmt);

  যদি( nRead==iAmt ){
    ফেরত SQLITE_OK;
  }অন্যথা যদি( nRead>=0 ){
    যদি( nRead<iAmt ){
      memset(&((অক্ষর*)zBuf)[nRead], 0, iAmt-nRead);
    }
    ফেরত SQLITE_IOERR_SHORT_READ;
  }

  ফেরত SQLITE_IOERR_READ;
}

/*
** Write data to a crash-file.
*/
স্থির পূর্ণসংখ্যা demoWrite(
  sqlite3_file *pFile, 
  ধ্রুবক শূন্য *zBuf, 
  পূর্ণসংখ্যা iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  
  যদি( p->a_বাফার ){
    অক্ষর *z = (অক্ষর *)zBuf;       /* Pointer to remaining data to write */
    পূর্ণসংখ্যা n = iAmt;                 /* Number of bytes at z */
    sqlite3_int64 i = iOfst;      /* File offset to write to */

    যখন( n>0 ){
      পূর্ণসংখ্যা nCopy;                  /* Number of bytes to copy into buffer */

      /* If the buffer is full, or if this data is not being written directly
      ** following the data already buffered, flush the buffer. Flushing
      ** the buffer is a no-op if it is empty.  
      */
      যদি( p->n_বাফার==SQLITE_DEMOVFS_BUFFERSZ || p->i_বাফার_Ofst+p->n_বাফার!=i ){
        পূর্ণসংখ্যা rc = demo_Flush_বাফার(p);
        যদি( rc!=SQLITE_OK ){
          ফেরত rc;
        }
      }
      প্রতিজ্ঞা( p->n_বাফার==0 || p->i_বাফার_Ofst+p->n_বাফার==i );
      p->i_বাফার_Ofst = i - p->n_বাফার;

      /* Copy as much data as possible into the buffer. */
      nCopy = SQLITE_DEMOVFS_BUFFERSZ - p->n_বাফার;
      যদি( nCopy>n ){
        nCopy = n;
      }
      memcpy(&p->a_বাফার[p->n_বাফার], z, nCopy);
      p->n_বাফার += nCopy;

      n -= nCopy;
      i += nCopy;
      z += nCopy;
    }
  }অন্যথা{
    ফেরত demoDirectWrite(p, zBuf, iAmt, iOfst);
  }

  ফেরত SQLITE_OK;
}

/*
** Truncate a file. This is a no-op for this VFS (see header comments at
** the top of the file).
*/
স্থির পূর্ণসংখ্যা demoTruncate(sqlite3_file *pFile, sqlite_int64 আকার_2){
#যদি 0
  যদি( ftruncate(((DemoFile *)pFile)->fd, আকার_2) ) ফেরত SQLITE_IOERR_TRUNCATE;
#endif
  ফেরত SQLITE_OK;
}

/*
** Sync the contents of the file to the persistent media.
*/
স্থির পূর্ণসংখ্যা demoSync(sqlite3_file *pFile, পূর্ণসংখ্যা flags){
  DemoFile *p = (DemoFile*)pFile;
  পূর্ণসংখ্যা rc;

  rc = demo_Flush_বাফার(p);
  যদি( rc!=SQLITE_OK ){
    ফেরত rc;
  }

  rc = fsync(p->fd);
  ফেরত (rc==0 ? SQLITE_OK : SQLITE_IOERR_FSYNC);
}

/*
** Write the size of the file in bytes to *pSize.
*/
স্থির পূর্ণসংখ্যা demo_File_আকার(sqlite3_file *pFile, sqlite_int64 *p_আকার){
  DemoFile *p = (DemoFile*)pFile;
  পূর্ণসংখ্যা rc;                         /* Return code from fstat() call */
  কাঠামো stat sStat;              /* Output of fstat() call */

  /* Flush the contents of the buffer to disk. As with the flush in the
  ** demoRead() method, it would be possible to avoid this and save a write
  ** here and there. But in practice this comes up so infrequently it is
  ** not worth the trouble.
  */
  rc = demo_Flush_বাফার(p);
  যদি( rc!=SQLITE_OK ){
    ফেরত rc;
  }

  rc = fstat(p->fd, &sStat);
  যদি( rc!=0 ) ফেরত SQLITE_IOERR_FSTAT;
  *p_আকার = sStat.st_আকার;
  ফেরত SQLITE_OK;
}

/*
** Locking functions. The xLock() and xUnlock() methods are both no-ops.
** The xCheckReservedLock() always indicates that no other process holds
** a reserved lock on the database file. This ensures that if a hot-journal
** file is found in the file-system it is rolled back.
*/
স্থির পূর্ণসংখ্যা demoLock(sqlite3_file *pFile, পূর্ণসংখ্যা eLock){
  ফেরত SQLITE_OK;
}
স্থির পূর্ণসংখ্যা demoUnlock(sqlite3_file *pFile, পূর্ণসংখ্যা eLock){
  ফেরত SQLITE_OK;
}
স্থির পূর্ণসংখ্যা demoCheckReservedLock(sqlite3_file *pFile, পূর্ণসংখ্যা *pResOut){
  *pResOut = 0;
  ফেরত SQLITE_OK;
}

/*
** No xFileControl() verbs are implemented by this VFS.
*/
স্থির পূর্ণসংখ্যা demoFileControl(sqlite3_file *pFile, পূর্ণসংখ্যা op, শূন্য *pArg){
  ফেরত SQLITE_NOTFOUND;
}

/*
** The xSectorSize() and xDeviceCharacteristics() methods. These two
** may return special values allowing SQLite to optimize file-system 
** access to some extent. But it is also safe to simply return 0.
*/
স্থির পূর্ণসংখ্যা demo_Sector_আকার(sqlite3_file *pFile){
  ফেরত 0;
}
স্থির পূর্ণসংখ্যা demoDeviceCharacteristics(sqlite3_file *pFile){
  ফেরত 0;
}

/*
** Open a file handle.
*/
স্থির পূর্ণসংখ্যা demoOpen(
  sqlite3_vfs *pVfs,              /* VFS */
  ধ্রুবক অক্ষর *zName,              /* File to open, or 0 for a temp file */
  sqlite3_file *pFile,            /* Pointer to DemoFile struct to populate */
  পূর্ণসংখ্যা flags,                      /* Input SQLITE_OPEN_XXX flags */
  পূর্ণসংখ্যা *pOutFlags                  /* Output SQLITE_OPEN_XXX flags (or NULL) */
){
  স্থির ধ্রুবক sqlite3_io_methods demoio = {
    1,                            /* iVersion */
    demoClose,                    /* xClose */
    demoRead,                     /* xRead */
    demoWrite,                    /* xWrite */
    demoTruncate,                 /* xTruncate */
    demoSync,                     /* xSync */
    demo_File_আকার,                 /* xFileSize */
    demoLock,                     /* xLock */
    demoUnlock,                   /* xUnlock */
    demoCheckReservedLock,        /* xCheckReservedLock */
    demoFileControl,              /* xFileControl */
    demo_Sector_আকার,               /* xSectorSize */
    demoDeviceCharacteristics     /* xDeviceCharacteristics */
  };

  DemoFile *p = (DemoFile*)pFile; /* Populate this structure */
  পূর্ণসংখ্যা oflags = 0;                 /* flags to pass to open() call */
  অক্ষর *aBuf = 0;

  যদি( zName==0 ){
    ফেরত SQLITE_IOERR;
  }

  যদি( flags&SQLITE_OPEN_MAIN_JOURNAL ){
    aBuf = (অক্ষর *)sqlite3_malloc(SQLITE_DEMOVFS_BUFFERSZ);
    যদি( !aBuf ){
      ফেরত SQLITE_NOMEM;
    }
  }

  যদি( flags&SQLITE_OPEN_EXCLUSIVE ) oflags |= O_EXCL;
  যদি( flags&SQLITE_OPEN_CREATE )    oflags |= O_CREAT;
  যদি( flags&SQLITE_OPEN_READONLY )  oflags |= O_RDONLY;
  যদি( flags&SQLITE_OPEN_READWRITE ) oflags |= O_RDWR;

  memset(p, 0, আকার(DemoFile));
  p->fd = open(zName, oflags, 0600);
  যদি( p->fd<0 ){
    sqlite3_free(aBuf);
    ফেরত SQLITE_CANTOPEN;
  }
  p->a_বাফার = aBuf;

  যদি( pOutFlags ){
    *pOutFlags = flags;
  }
  p->base.pMethods = &demoio;
  ফেরত SQLITE_OK;
}

/*
** Delete the file identified by argument zPath. If the dirSync parameter
** is non-zero, then ensure the file-system modification to delete the
** file has been synced to disk before returning.
*/
স্থির পূর্ণসংখ্যা demoDelete(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zPath, পূর্ণসংখ্যা dirSync){
  পূর্ণসংখ্যা rc;                         /* Return code */

  rc = unlink(zPath);
  যদি( rc!=0 && errno==ENOENT ) ফেরত SQLITE_OK;

  যদি( rc==0 && dirSync ){
    পূর্ণসংখ্যা dfd;                      /* File descriptor open on directory */
    অক্ষর *zSlash;
    অক্ষর zDir[MAXPATHNAME+1];     /* Name of directory containing file zPath */

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
  ফেরত (rc==0 ? SQLITE_OK : SQLITE_IOERR_DELETE);
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
স্থির পূর্ণসংখ্যা demoAccess(
  sqlite3_vfs *pVfs, 
  ধ্রুবক অক্ষর *zPath, 
  পূর্ণসংখ্যা flags, 
  পূর্ণসংখ্যা *pResOut
){
  পূর্ণসংখ্যা rc;                         /* access() return code */
  পূর্ণসংখ্যা eAccess = F_OK;             /* Second argument to access() */

  প্রতিজ্ঞা( flags==SQLITE_ACCESS_EXISTS       /* access(zPath, F_OK) */
       || flags==SQLITE_ACCESS_READ         /* access(zPath, R_OK) */
       || flags==SQLITE_ACCESS_READWRITE    /* access(zPath, R_OK|W_OK) */
  );

  যদি( flags==SQLITE_ACCESS_READWRITE ) eAccess = R_OK|W_OK;
  যদি( flags==SQLITE_ACCESS_READ )      eAccess = R_OK;

  rc = access(zPath, eAccess);
  *pResOut = (rc==0);
  ফেরত SQLITE_OK;
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
স্থির পূর্ণসংখ্যা demoFullPathname(
  sqlite3_vfs *pVfs,              /* VFS */
  ধ্রুবক অক্ষর *zPath,              /* Input path (possibly a relative path) */
  পূর্ণসংখ্যা nPathOut,                   /* Size of output buffer in bytes */
  অক্ষর *zPathOut                  /* Pointer to output buffer */
){
  অক্ষর zDir[MAXPATHNAME+1];
  যদি( zPath[0]=='/' ){
    zDir[0] = '\0';
  }অন্যথা{
    যদি( getcwd(zDir, আকার(zDir))==0 ) ফেরত SQLITE_IOERR;
  }
  zDir[MAXPATHNAME] = '\0';

  sqlite3_snprintf(nPathOut, zPathOut, "%s/%s", zDir, zPath);
  zPathOut[nPathOut-1] = '\0';

  ফেরত SQLITE_OK;
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
স্থির শূন্য *demoDlOpen(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zPath){
  ফেরত 0;
}
স্থির শূন্য demoDlError(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nByte, অক্ষর *z_Err_বার্তা){
  sqlite3_snprintf(nByte, z_Err_বার্তা, "Loadable extensions are not supported");
  z_Err_বার্তা[nByte-1] = '\0';
}
স্থির শূন্য (*demoDlSym(sqlite3_vfs *pVfs, শূন্য *pH, ধ্রুবক অক্ষর *z))(শূন্য){
  ফেরত 0;
}
স্থির শূন্য demoDlClose(sqlite3_vfs *pVfs, শূন্য *pHandle){
  ফেরত;
}

/*
** Parameter zByte points to a buffer nByte bytes in size. Populate this
** buffer with pseudo-random data.
*/
স্থির পূর্ণসংখ্যা demoRandomness(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nByte, অক্ষর *zByte){
  ফেরত SQLITE_OK;
}

/*
** Sleep for at least nMicro microseconds. Return the (approximate) number 
** of microseconds slept for.
*/
স্থির পূর্ণসংখ্যা demoSleep(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nMicro){
  sleep(nMicro / 1000000);
  usleep(nMicro % 1000000);
  ফেরত nMicro;
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
স্থির পূর্ণসংখ্যা demoCurrentTime(sqlite3_vfs *pVfs, দ্বিগুণ *pTime){
  time_t t = time(0);
  *pTime = t/86400.0 + 2440587.5; 
  ফেরত SQLITE_OK;
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
    আকার(DemoFile),             /* szOsFile */
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
  ফেরত &demovfs;
}

#endif /* !defined(SQLITE_TEST) || SQLITE_OS_UNIX */


#ifdef SQLITE_TEST

#অন্তর্ভাব "tclsqlite.h"

#যদি SQLITE_OS_UNIX
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI register_demovfs(
  Client_তথ্য client_তথ্য, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_register(sqlite3_demovfs(), 1);
  ফেরত TCL_OK;
}
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI unregister_demovfs(
  Client_তথ্য client_তথ্য, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_unregister(sqlite3_demovfs());
  ফেরত TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetest_demovfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_demovfs", register_demovfs, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_demovfs", unregister_demovfs, 0, 0);
  ফেরত TCL_OK;
}

#অন্যথা
পূর্ণসংখ্যা Sqlitetest_demovfs_Init(Tcl_Interp *interp){ ফেরত TCL_OK; }
#endif

#endif /* SQLITE_TEST */
