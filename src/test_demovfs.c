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

#ఒకవేళ !defined(SQLITE_TEST) || SQLITE_OS_UNIX

#అన్తర్భావ "sqlite3.h"

#అన్తర్భావ <assert.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/file.h>
#అన్తర్భావ <sys/param.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>

/*
** Size of the write buffer used by journal files in bytes.
*/
#నిర్వచితం_కాకపోతే SQLITE_DEMOVFS_BUFFERSZ
# define SQLITE_DEMOVFS_BUFFERSZ 8192
#అయితే_ముగింపు

/*
** The maximum pathname length supported by this VFS.
*/
#నిర్వచించు MAXPATHNAME 512

/*
** When using this VFS, the sqlite3_file* handles that SQLite uses are
** actually pointers to instances of type DemoFile.
*/
రకనిర్వచనం నిర్మాణం DemoFile DemoFile;
నిర్మాణం DemoFile {
  sqlite3_file పునాది;              /* Base class. Must be first. */
  పూర్ణాంకం fd;                         /* File descriptor */

  అక్షరం *a_బఫరు;                  /* Pointer to malloc'd buffer */
  పూర్ణాంకం n_బఫరు;                    /* Valid bytes of data in zBuffer */
  sqlite3_int64 i_బఫరు_Ofst;      /* Offset in file of zBuffer[0] */
};

/*
** Write directly to the file passed as the first argument. Even if the
** file has a write-buffer (DemoFile.aBuffer), ignore it.
*/
స్థిర పూర్ణాంకం demo_ప్రత్యక్ష_Write(
  DemoFile *p,                    /* File handle */
  స్థిరరాశి శూన్య *zBuf,               /* Buffer containing data to write */
  పూర్ణాంకం iAmt,                       /* Size of data to write in bytes */
  sqlite_int64 iOfst              /* File offset to write to */
){
  off_t ofst;                     /* Return value from lseek() */
  size_t nWrite;                  /* Return value from write() */

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  ఒకవేళ( ofst!=iOfst ){
    తిరిగి SQLITE_IOERR_WRITE;
  }

  nWrite = లిఖ(p->fd, zBuf, iAmt);
  ఒకవేళ( nWrite!=iAmt ){
    తిరిగి SQLITE_IOERR_WRITE;
  }

  తిరిగి SQLITE_OK;
}

/*
** Flush the contents of the DemoFile.aBuffer buffer to disk. This is a
** no-op if this particular file does not have a buffer (i.e. it is not
** a journal file) or if the buffer is currently empty.
*/
స్థిర పూర్ణాంకం demo_Flush_బఫరు(DemoFile *p){
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( p->n_బఫరు ){
    rc = demo_ప్రత్యక్ష_Write(p, p->a_బఫరు, p->n_బఫరు, p->i_బఫరు_Ofst);
    p->n_బఫరు = 0;
  }
  తిరిగి rc;
}

/*
** Close a file.
*/
స్థిర పూర్ణాంకం demoClose(sqlite3_file *pFile){
  పూర్ణాంకం rc;
  DemoFile *p = (DemoFile*)pFile;
  rc = demo_Flush_బఫరు(p);
  sqlite3_free(p->a_బఫరు);
  close(p->fd);
  తిరిగి rc;
}

/*
** Read data from a file.
*/
స్థిర పూర్ణాంకం demoRead(
  sqlite3_file *pFile, 
  శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  off_t ofst;                     /* Return value from lseek() */
  పూర్ణాంకం nRead;                      /* Return value from read() */
  పూర్ణాంకం rc;                         /* Return code from demoFlushBuffer() */

  /* Flush any data in the write buffer to disk in case this operation
  ** is trying to read data the file-region currently cached in the buffer.
  ** It would be possible to detect this case and possibly save an 
  ** unnecessary write here, but in practice SQLite will rarely read from
  ** a journal file when there is data cached in the write-buffer.
  */
  rc = demo_Flush_బఫరు(p);
  ఒకవేళ( rc!=SQLITE_OK ){
    తిరిగి rc;
  }

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  ఒకవేళ( ofst!=iOfst ){
    తిరిగి SQLITE_IOERR_READ;
  }
  nRead = పఠ(p->fd, zBuf, iAmt);

  ఒకవేళ( nRead==iAmt ){
    తిరిగి SQLITE_OK;
  }లేకపోతే ఒకవేళ( nRead>=0 ){
    ఒకవేళ( nRead<iAmt ){
      memset(&((అక్షరం*)zBuf)[nRead], 0, iAmt-nRead);
    }
    తిరిగి SQLITE_IOERR_SHORT_READ;
  }

  తిరిగి SQLITE_IOERR_READ;
}

/*
** Write data to a crash-file.
*/
స్థిర పూర్ణాంకం demoWrite(
  sqlite3_file *pFile, 
  స్థిరరాశి శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  
  ఒకవేళ( p->a_బఫరు ){
    అక్షరం *z = (అక్షరం *)zBuf;       /* Pointer to remaining data to write */
    పూర్ణాంకం n = iAmt;                 /* Number of bytes at z */
    sqlite3_int64 i = iOfst;      /* File offset to write to */

    అయితే( n>0 ){
      పూర్ణాంకం nCopy;                  /* Number of bytes to copy into buffer */

      /* If the buffer is full, or if this data is not being written directly
      ** following the data already buffered, flush the buffer. Flushing
      ** the buffer is a no-op if it is empty.  
      */
      ఒకవేళ( p->n_బఫరు==SQLITE_DEMOVFS_BUFFERSZ || p->i_బఫరు_Ofst+p->n_బఫరు!=i ){
        పూర్ణాంకం rc = demo_Flush_బఫరు(p);
        ఒకవేళ( rc!=SQLITE_OK ){
          తిరిగి rc;
        }
      }
      ప్రతిజ్ఞా( p->n_బఫరు==0 || p->i_బఫరు_Ofst+p->n_బఫరు==i );
      p->i_బఫరు_Ofst = i - p->n_బఫరు;

      /* Copy as much data as possible into the buffer. */
      nCopy = SQLITE_DEMOVFS_BUFFERSZ - p->n_బఫరు;
      ఒకవేళ( nCopy>n ){
        nCopy = n;
      }
      జ్ఞాపకం_నకలు(&p->a_బఫరు[p->n_బఫరు], z, nCopy);
      p->n_బఫరు += nCopy;

      n -= nCopy;
      i += nCopy;
      z += nCopy;
    }
  }లేకపోతే{
    తిరిగి demo_ప్రత్యక్ష_Write(p, zBuf, iAmt, iOfst);
  }

  తిరిగి SQLITE_OK;
}

/*
** Truncate a file. This is a no-op for this VFS (see header comments at
** the top of the file).
*/
స్థిర పూర్ణాంకం demoTruncate(sqlite3_file *pFile, sqlite_int64 కొలత){
#ఒకవేళ 0
  ఒకవేళ( ftruncate(((DemoFile *)pFile)->fd, కొలత) ) తిరిగి SQLITE_IOERR_TRUNCATE;
#అయితే_ముగింపు
  తిరిగి SQLITE_OK;
}

/*
** Sync the contents of the file to the persistent media.
*/
స్థిర పూర్ణాంకం demoSync(sqlite3_file *pFile, పూర్ణాంకం flags){
  DemoFile *p = (DemoFile*)pFile;
  పూర్ణాంకం rc;

  rc = demo_Flush_బఫరు(p);
  ఒకవేళ( rc!=SQLITE_OK ){
    తిరిగి rc;
  }

  rc = fsync(p->fd);
  తిరిగి (rc==0 ? SQLITE_OK : SQLITE_IOERR_FSYNC);
}

/*
** Write the size of the file in bytes to *pSize.
*/
స్థిర పూర్ణాంకం demo_File_కొలత(sqlite3_file *pFile, sqlite_int64 *p_కొలత){
  DemoFile *p = (DemoFile*)pFile;
  పూర్ణాంకం rc;                         /* Return code from fstat() call */
  నిర్మాణం stat sStat;              /* Output of fstat() call */

  /* Flush the contents of the buffer to disk. As with the flush in the
  ** demoRead() method, it would be possible to avoid this and save a write
  ** here and there. But in practice this comes up so infrequently it is
  ** not worth the trouble.
  */
  rc = demo_Flush_బఫరు(p);
  ఒకవేళ( rc!=SQLITE_OK ){
    తిరిగి rc;
  }

  rc = fstat(p->fd, &sStat);
  ఒకవేళ( rc!=0 ) తిరిగి SQLITE_IOERR_FSTAT;
  *p_కొలత = sStat.st_కొలత;
  తిరిగి SQLITE_OK;
}

/*
** Locking functions. The xLock() and xUnlock() methods are both no-ops.
** The xCheckReservedLock() always indicates that no other process holds
** a reserved lock on the database file. This ensures that if a hot-journal
** file is found in the file-system it is rolled back.
*/
స్థిర పూర్ణాంకం demo_తాళం(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  తిరిగి SQLITE_OK;
}
స్థిర పూర్ణాంకం demoUnlock(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  తిరిగి SQLITE_OK;
}
స్థిర పూర్ణాంకం demo_Check_Reserved_తాళం(sqlite3_file *pFile, పూర్ణాంకం *p_Res_బహిర్గతం){
  *p_Res_బహిర్గతం = 0;
  తిరిగి SQLITE_OK;
}

/*
** No xFileControl() verbs are implemented by this VFS.
*/
స్థిర పూర్ణాంకం demoFileControl(sqlite3_file *pFile, పూర్ణాంకం op, శూన్య *pArg){
  తిరిగి SQLITE_NOTFOUND;
}

/*
** The xSectorSize() and xDeviceCharacteristics() methods. These two
** may return special values allowing SQLite to optimize file-system 
** access to some extent. But it is also safe to simply return 0.
*/
స్థిర పూర్ణాంకం demo_Sector_కొలత(sqlite3_file *pFile){
  తిరిగి 0;
}
స్థిర పూర్ణాంకం demoDeviceCharacteristics(sqlite3_file *pFile){
  తిరిగి 0;
}

/*
** Open a file handle.
*/
స్థిర పూర్ణాంకం demoOpen(
  sqlite3_vfs *pVfs,              /* VFS */
  స్థిరరాశి అక్షరం *zName,              /* File to open, or 0 for a temp file */
  sqlite3_file *pFile,            /* Pointer to DemoFile struct to populate */
  పూర్ణాంకం flags,                      /* Input SQLITE_OPEN_XXX flags */
  పూర్ణాంకం *p_బహిర్గతం_Flags                  /* Output SQLITE_OPEN_XXX flags (or NULL) */
){
  స్థిర స్థిరరాశి sqlite3_io_methods demoio = {
    1,                            /* iVersion */
    demoClose,                    /* xClose */
    demoRead,                     /* xRead */
    demoWrite,                    /* xWrite */
    demoTruncate,                 /* xTruncate */
    demoSync,                     /* xSync */
    demo_File_కొలత,                 /* xFileSize */
    demo_తాళం,                     /* xLock */
    demoUnlock,                   /* xUnlock */
    demo_Check_Reserved_తాళం,        /* xCheckReservedLock */
    demoFileControl,              /* xFileControl */
    demo_Sector_కొలత,               /* xSectorSize */
    demoDeviceCharacteristics     /* xDeviceCharacteristics */
  };

  DemoFile *p = (DemoFile*)pFile; /* Populate this structure */
  పూర్ణాంకం oflags = 0;                 /* flags to pass to open() call */
  అక్షరం *aBuf = 0;

  ఒకవేళ( zName==0 ){
    తిరిగి SQLITE_IOERR;
  }

  ఒకవేళ( flags&SQLITE_OPEN_MAIN_JOURNAL ){
    aBuf = (అక్షరం *)sqlite3_malloc(SQLITE_DEMOVFS_BUFFERSZ);
    ఒకవేళ( !aBuf ){
      తిరిగి SQLITE_NOMEM;
    }
  }

  ఒకవేళ( flags&SQLITE_OPEN_EXCLUSIVE ) oflags |= O_EXCL;
  ఒకవేళ( flags&SQLITE_OPEN_CREATE )    oflags |= O_CREAT;
  ఒకవేళ( flags&SQLITE_OPEN_READONLY )  oflags |= O_RDONLY;
  ఒకవేళ( flags&SQLITE_OPEN_READWRITE ) oflags |= O_RDWR;

  memset(p, 0, పరిమాణం(DemoFile));
  p->fd = open(zName, oflags, 0600);
  ఒకవేళ( p->fd<0 ){
    sqlite3_free(aBuf);
    తిరిగి SQLITE_CANTOPEN;
  }
  p->a_బఫరు = aBuf;

  ఒకవేళ( p_బహిర్గతం_Flags ){
    *p_బహిర్గతం_Flags = flags;
  }
  p->పునాది.pMethods = &demoio;
  తిరిగి SQLITE_OK;
}

/*
** Delete the file identified by argument zPath. If the dirSync parameter
** is non-zero, then ensure the file-system modification to delete the
** file has been synced to disk before returning.
*/
స్థిర పూర్ణాంకం demoDelete(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం, పూర్ణాంకం dirSync){
  పూర్ణాంకం rc;                         /* Return code */

  rc = unlink(z_మార్గం);
  ఒకవేళ( rc!=0 && errno==ENOENT ) తిరిగి SQLITE_OK;

  ఒకవేళ( rc==0 && dirSync ){
    పూర్ణాంకం dfd;                      /* File descriptor open on directory */
    అక్షరం *zSlash;
    అక్షరం zDir[MAXPATHNAME+1];     /* Name of directory containing file zPath */

    /* Figure out the directory name from the path of the file deleted. */
    sqlite3_snprintf(MAXPATHNAME, zDir, "%s", z_మార్గం);
    zDir[MAXPATHNAME] = '\0';
    zSlash = strrchr(zDir,'/');
    ఒకవేళ( zSlash ){
      /* Open a file-descriptor on the directory. Sync. Close. */
      zSlash[0] = 0;
      dfd = open(zDir, O_RDONLY, 0);
      ఒకవేళ( dfd<0 ){
        rc = -1;
      }లేకపోతే{
        rc = fsync(dfd);
        close(dfd);
      }
    }
  }
  తిరిగి (rc==0 ? SQLITE_OK : SQLITE_IOERR_DELETE);
}

#నిర్వచితం_కాకపోతే F_OK
# define F_OK 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే R_OK
# define R_OK 4
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే W_OK
# define W_OK 2
#అయితే_ముగింపు

/*
** Query the file-system to see if the named file exists, is readable or
** is both readable and writable.
*/
స్థిర పూర్ణాంకం demoAccess(
  sqlite3_vfs *pVfs, 
  స్థిరరాశి అక్షరం *z_మార్గం, 
  పూర్ణాంకం flags, 
  పూర్ణాంకం *p_Res_బహిర్గతం
){
  పూర్ణాంకం rc;                         /* access() return code */
  పూర్ణాంకం eAccess = F_OK;             /* Second argument to access() */

  ప్రతిజ్ఞా( flags==SQLITE_ACCESS_EXISTS       /* access(zPath, F_OK) */
       || flags==SQLITE_ACCESS_READ         /* access(zPath, R_OK) */
       || flags==SQLITE_ACCESS_READWRITE    /* access(zPath, R_OK|W_OK) */
  );

  ఒకవేళ( flags==SQLITE_ACCESS_READWRITE ) eAccess = R_OK|W_OK;
  ఒకవేళ( flags==SQLITE_ACCESS_READ )      eAccess = R_OK;

  rc = access(z_మార్గం, eAccess);
  *p_Res_బహిర్గతం = (rc==0);
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం demo_నిండిన_Pathname(
  sqlite3_vfs *pVfs,              /* VFS */
  స్థిరరాశి అక్షరం *z_మార్గం,              /* Input path (possibly a relative path) */
  పూర్ణాంకం n_మార్గం_బహిర్గతం,                   /* Size of output buffer in bytes */
  అక్షరం *z_మార్గం_బహిర్గతం                  /* Pointer to output buffer */
){
  అక్షరం zDir[MAXPATHNAME+1];
  ఒకవేళ( z_మార్గం[0]=='/' ){
    zDir[0] = '\0';
  }లేకపోతే{
    ఒకవేళ( getcwd(zDir, పరిమాణం(zDir))==0 ) తిరిగి SQLITE_IOERR;
  }
  zDir[MAXPATHNAME] = '\0';

  sqlite3_snprintf(n_మార్గం_బహిర్గతం, z_మార్గం_బహిర్గతం, "%s/%s", zDir, z_మార్గం);
  z_మార్గం_బహిర్గతం[n_మార్గం_బహిర్గతం-1] = '\0';

  తిరిగి SQLITE_OK;
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
స్థిర శూన్య *demoDlOpen(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం){
  తిరిగి 0;
}
స్థిర శూన్య demoDlError(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Err_సందేశం){
  sqlite3_snprintf(nByte, z_Err_సందేశం, "Loadable extensions are not supported");
  z_Err_సందేశం[nByte-1] = '\0';
}
స్థిర శూన్య (*demoDlSym(sqlite3_vfs *pVfs, శూన్య *pH, స్థిరరాశి అక్షరం *z))(శూన్య){
  తిరిగి 0;
}
స్థిర శూన్య demoDlClose(sqlite3_vfs *pVfs, శూన్య *pHandle){
  తిరిగి;
}

/*
** Parameter zByte points to a buffer nByte bytes in size. Populate this
** buffer with pseudo-random data.
*/
స్థిర పూర్ణాంకం demoRandomness(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *zByte){
  తిరిగి SQLITE_OK;
}

/*
** Sleep for at least nMicro microseconds. Return the (approximate) number 
** of microseconds slept for.
*/
స్థిర పూర్ణాంకం demoSleep(sqlite3_vfs *pVfs, పూర్ణాంకం nMicro){
  sleep(nMicro / 1000000);
  usleep(nMicro % 1000000);
  తిరిగి nMicro;
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
స్థిర పూర్ణాంకం demo_Current_సమయం(sqlite3_vfs *pVfs, ద్విగుణ *p_సమయం){
  సమయం_t t = సమయం(0);
  *p_సమయం = t/86400.0 + 2440587.5; 
  తిరిగి SQLITE_OK;
}

/*
** This function returns a pointer to the VFS implemented in this file.
** To make the VFS available to SQLite:
**
**   sqlite3_vfs_register(sqlite3_demovfs(), 0);
*/
sqlite3_vfs *sqlite3_demovfs(శూన్య){
  స్థిర sqlite3_vfs demovfs = {
    1,                            /* iVersion */
    పరిమాణం(DemoFile),             /* szOsFile */
    MAXPATHNAME,                  /* mxPathname */
    0,                            /* pNext */
    "demo",                       /* zName */
    0,                            /* pAppData */
    demoOpen,                     /* xOpen */
    demoDelete,                   /* xDelete */
    demoAccess,                   /* xAccess */
    demo_నిండిన_Pathname,             /* xFullPathname */
    demoDlOpen,                   /* xDlOpen */
    demoDlError,                  /* xDlError */
    demoDlSym,                    /* xDlSym */
    demoDlClose,                  /* xDlClose */
    demoRandomness,               /* xRandomness */
    demoSleep,                    /* xSleep */
    demo_Current_సమయం,              /* xCurrentTime */
  };
  తిరిగి &demovfs;
}

#అయితే_ముగింపు /* !defined(SQLITE_TEST) || SQLITE_OS_UNIX */


#నిర్వచితమైతే SQLITE_TEST

#అన్తర్భావ "tclsqlite.h"

#ఒకవేళ SQLITE_OS_UNIX
స్థిర పూర్ణాంకం SQLITE_TCLAPI రిజిస్టరు_demovfs(
  Client_దత్తాంశం client_దత్తాంశం, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_రిజిస్టరు(sqlite3_demovfs(), 1);
  తిరిగి TCL_OK;
}
స్థిర పూర్ణాంకం SQLITE_TCLAPI unregister_demovfs(
  Client_దత్తాంశం client_దత్తాంశం, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_unregister(sqlite3_demovfs());
  తిరిగి TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetest_demovfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_demovfs", రిజిస్టరు_demovfs, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_demovfs", unregister_demovfs, 0, 0);
  తిరిగి TCL_OK;
}

#లేకపోతే
పూర్ణాంకం Sqlitetest_demovfs_Init(Tcl_Interp *interp){ తిరిగి TCL_OK; }
#అయితే_ముగింపు

#అయితే_ముగింపు /* SQLITE_TEST */
