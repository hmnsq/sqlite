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

#ਜੇ !defined(SQLITE_TEST) || SQLITE_OS_UNIX

#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/file.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/param.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>

/*
** Size of the write buffer used by journal files in bytes.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEMOVFS_BUFFERSZ
# define SQLITE_DEMOVFS_BUFFERSZ 8192
#ਜੇ_ਅੰਤ

/*
** The maximum pathname length supported by this VFS.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXPATHNAME 512

/*
** When using this VFS, the sqlite3_file* handles that SQLite uses are
** actually pointers to instances of type DemoFile.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ DemoFile DemoFile;
ਬਣਤਰ DemoFile {
  sqlite3_file ਅਧਾਰ;              /* Base class. Must be first. */
  ਪੂਰਨ fd;                         /* File descriptor */

  ਅੱਖਰ *a_ਬਫਰ;                  /* Pointer to malloc'd buffer */
  ਪੂਰਨ n_ਬਫਰ;                    /* Valid bytes of data in zBuffer */
  sqlite3_int64 i_ਬਫਰ_Ofst;      /* Offset in file of zBuffer[0] */
};

/*
** Write directly to the file passed as the first argument. Even if the
** file has a write-buffer (DemoFile.aBuffer), ignore it.
*/
ਸਥਿਰ ਪੂਰਨ demo_ਸਿੱਧਾ_Write(
  DemoFile *p,                    /* File handle */
  ਅਚਲ ਖਾਲੀ *zBuf,               /* Buffer containing data to write */
  ਪੂਰਨ iAmt,                       /* Size of data to write in bytes */
  sqlite_int64 iOfst              /* File offset to write to */
){
  off_t ofst;                     /* Return value from lseek() */
  size_t nWrite;                  /* Return value from write() */

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  ਜੇ( ofst!=iOfst ){
    ਵਾਪਸ SQLITE_IOERR_WRITE;
  }

  nWrite = ਲਿਖ(p->fd, zBuf, iAmt);
  ਜੇ( nWrite!=iAmt ){
    ਵਾਪਸ SQLITE_IOERR_WRITE;
  }

  ਵਾਪਸ SQLITE_OK;
}

/*
** Flush the contents of the DemoFile.aBuffer buffer to disk. This is a
** no-op if this particular file does not have a buffer (i.e. it is not
** a journal file) or if the buffer is currently empty.
*/
ਸਥਿਰ ਪੂਰਨ demo_Flush_ਬਫਰ(DemoFile *p){
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( p->n_ਬਫਰ ){
    rc = demo_ਸਿੱਧਾ_Write(p, p->a_ਬਫਰ, p->n_ਬਫਰ, p->i_ਬਫਰ_Ofst);
    p->n_ਬਫਰ = 0;
  }
  ਵਾਪਸ rc;
}

/*
** Close a file.
*/
ਸਥਿਰ ਪੂਰਨ demoClose(sqlite3_file *pFile){
  ਪੂਰਨ rc;
  DemoFile *p = (DemoFile*)pFile;
  rc = demo_Flush_ਬਫਰ(p);
  sqlite3_free(p->a_ਬਫਰ);
  close(p->fd);
  ਵਾਪਸ rc;
}

/*
** Read data from a file.
*/
ਸਥਿਰ ਪੂਰਨ demoRead(
  sqlite3_file *pFile, 
  ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  off_t ofst;                     /* Return value from lseek() */
  ਪੂਰਨ nRead;                      /* Return value from read() */
  ਪੂਰਨ rc;                         /* Return code from demoFlushBuffer() */

  /* Flush any data in the write buffer to disk in case this operation
  ** is trying to read data the file-region currently cached in the buffer.
  ** It would be possible to detect this case and possibly save an 
  ** unnecessary write here, but in practice SQLite will rarely read from
  ** a journal file when there is data cached in the write-buffer.
  */
  rc = demo_Flush_ਬਫਰ(p);
  ਜੇ( rc!=SQLITE_OK ){
    ਵਾਪਸ rc;
  }

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  ਜੇ( ofst!=iOfst ){
    ਵਾਪਸ SQLITE_IOERR_READ;
  }
  nRead = ਪਠ(p->fd, zBuf, iAmt);

  ਜੇ( nRead==iAmt ){
    ਵਾਪਸ SQLITE_OK;
  }ਨਹੀਂਤਾਂ ਜੇ( nRead>=0 ){
    ਜੇ( nRead<iAmt ){
      memset(&((ਅੱਖਰ*)zBuf)[nRead], 0, iAmt-nRead);
    }
    ਵਾਪਸ SQLITE_IOERR_SHORT_READ;
  }

  ਵਾਪਸ SQLITE_IOERR_READ;
}

/*
** Write data to a crash-file.
*/
ਸਥਿਰ ਪੂਰਨ demoWrite(
  sqlite3_file *pFile, 
  ਅਚਲ ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  
  ਜੇ( p->a_ਬਫਰ ){
    ਅੱਖਰ *z = (ਅੱਖਰ *)zBuf;       /* Pointer to remaining data to write */
    ਪੂਰਨ n = iAmt;                 /* Number of bytes at z */
    sqlite3_int64 i = iOfst;      /* File offset to write to */

    ਜਦੋਂ( n>0 ){
      ਪੂਰਨ nCopy;                  /* Number of bytes to copy into buffer */

      /* If the buffer is full, or if this data is not being written directly
      ** following the data already buffered, flush the buffer. Flushing
      ** the buffer is a no-op if it is empty.  
      */
      ਜੇ( p->n_ਬਫਰ==SQLITE_DEMOVFS_BUFFERSZ || p->i_ਬਫਰ_Ofst+p->n_ਬਫਰ!=i ){
        ਪੂਰਨ rc = demo_Flush_ਬਫਰ(p);
        ਜੇ( rc!=SQLITE_OK ){
          ਵਾਪਸ rc;
        }
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( p->n_ਬਫਰ==0 || p->i_ਬਫਰ_Ofst+p->n_ਬਫਰ==i );
      p->i_ਬਫਰ_Ofst = i - p->n_ਬਫਰ;

      /* Copy as much data as possible into the buffer. */
      nCopy = SQLITE_DEMOVFS_BUFFERSZ - p->n_ਬਫਰ;
      ਜੇ( nCopy>n ){
        nCopy = n;
      }
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&p->a_ਬਫਰ[p->n_ਬਫਰ], z, nCopy);
      p->n_ਬਫਰ += nCopy;

      n -= nCopy;
      i += nCopy;
      z += nCopy;
    }
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ demo_ਸਿੱਧਾ_Write(p, zBuf, iAmt, iOfst);
  }

  ਵਾਪਸ SQLITE_OK;
}

/*
** Truncate a file. This is a no-op for this VFS (see header comments at
** the top of the file).
*/
ਸਥਿਰ ਪੂਰਨ demoTruncate(sqlite3_file *pFile, sqlite_int64 ਮਾਪ){
#ਜੇ 0
  ਜੇ( ftruncate(((DemoFile *)pFile)->fd, ਮਾਪ) ) ਵਾਪਸ SQLITE_IOERR_TRUNCATE;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ SQLITE_OK;
}

/*
** Sync the contents of the file to the persistent media.
*/
ਸਥਿਰ ਪੂਰਨ demoSync(sqlite3_file *pFile, ਪੂਰਨ flags){
  DemoFile *p = (DemoFile*)pFile;
  ਪੂਰਨ rc;

  rc = demo_Flush_ਬਫਰ(p);
  ਜੇ( rc!=SQLITE_OK ){
    ਵਾਪਸ rc;
  }

  rc = fsync(p->fd);
  ਵਾਪਸ (rc==0 ? SQLITE_OK : SQLITE_IOERR_FSYNC);
}

/*
** Write the size of the file in bytes to *pSize.
*/
ਸਥਿਰ ਪੂਰਨ demo_File_ਮਾਪ(sqlite3_file *pFile, sqlite_int64 *p_ਮਾਪ){
  DemoFile *p = (DemoFile*)pFile;
  ਪੂਰਨ rc;                         /* Return code from fstat() call */
  ਬਣਤਰ stat sStat;              /* Output of fstat() call */

  /* Flush the contents of the buffer to disk. As with the flush in the
  ** demoRead() method, it would be possible to avoid this and save a write
  ** here and there. But in practice this comes up so infrequently it is
  ** not worth the trouble.
  */
  rc = demo_Flush_ਬਫਰ(p);
  ਜੇ( rc!=SQLITE_OK ){
    ਵਾਪਸ rc;
  }

  rc = fstat(p->fd, &sStat);
  ਜੇ( rc!=0 ) ਵਾਪਸ SQLITE_IOERR_FSTAT;
  *p_ਮਾਪ = sStat.st_ਮਾਪ;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Locking functions. The xLock() and xUnlock() methods are both no-ops.
** The xCheckReservedLock() always indicates that no other process holds
** a reserved lock on the database file. This ensures that if a hot-journal
** file is found in the file-system it is rolled back.
*/
ਸਥਿਰ ਪੂਰਨ demo_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  ਵਾਪਸ SQLITE_OK;
}
ਸਥਿਰ ਪੂਰਨ demoUnlock(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  ਵਾਪਸ SQLITE_OK;
}
ਸਥਿਰ ਪੂਰਨ demo_Check_Reserved_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  *p_Res_ਬਾਹਰਲਾ = 0;
  ਵਾਪਸ SQLITE_OK;
}

/*
** No xFileControl() verbs are implemented by this VFS.
*/
ਸਥਿਰ ਪੂਰਨ demoFileControl(sqlite3_file *pFile, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  ਵਾਪਸ SQLITE_NOTFOUND;
}

/*
** The xSectorSize() and xDeviceCharacteristics() methods. These two
** may return special values allowing SQLite to optimize file-system 
** access to some extent. But it is also safe to simply return 0.
*/
ਸਥਿਰ ਪੂਰਨ demo_Sector_ਮਾਪ(sqlite3_file *pFile){
  ਵਾਪਸ 0;
}
ਸਥਿਰ ਪੂਰਨ demoDeviceCharacteristics(sqlite3_file *pFile){
  ਵਾਪਸ 0;
}

/*
** Open a file handle.
*/
ਸਥਿਰ ਪੂਰਨ demoOpen(
  sqlite3_vfs *pVfs,              /* VFS */
  ਅਚਲ ਅੱਖਰ *zName,              /* File to open, or 0 for a temp file */
  sqlite3_file *pFile,            /* Pointer to DemoFile struct to populate */
  ਪੂਰਨ flags,                      /* Input SQLITE_OPEN_XXX flags */
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags                  /* Output SQLITE_OPEN_XXX flags (or NULL) */
){
  ਸਥਿਰ ਅਚਲ sqlite3_io_methods demoio = {
    1,                            /* iVersion */
    demoClose,                    /* xClose */
    demoRead,                     /* xRead */
    demoWrite,                    /* xWrite */
    demoTruncate,                 /* xTruncate */
    demoSync,                     /* xSync */
    demo_File_ਮਾਪ,                 /* xFileSize */
    demo_ਤਾਲਾ,                     /* xLock */
    demoUnlock,                   /* xUnlock */
    demo_Check_Reserved_ਤਾਲਾ,        /* xCheckReservedLock */
    demoFileControl,              /* xFileControl */
    demo_Sector_ਮਾਪ,               /* xSectorSize */
    demoDeviceCharacteristics     /* xDeviceCharacteristics */
  };

  DemoFile *p = (DemoFile*)pFile; /* Populate this structure */
  ਪੂਰਨ oflags = 0;                 /* flags to pass to open() call */
  ਅੱਖਰ *aBuf = 0;

  ਜੇ( zName==0 ){
    ਵਾਪਸ SQLITE_IOERR;
  }

  ਜੇ( flags&SQLITE_OPEN_MAIN_JOURNAL ){
    aBuf = (ਅੱਖਰ *)sqlite3_malloc(SQLITE_DEMOVFS_BUFFERSZ);
    ਜੇ( !aBuf ){
      ਵਾਪਸ SQLITE_NOMEM;
    }
  }

  ਜੇ( flags&SQLITE_OPEN_EXCLUSIVE ) oflags |= O_EXCL;
  ਜੇ( flags&SQLITE_OPEN_CREATE )    oflags |= O_CREAT;
  ਜੇ( flags&SQLITE_OPEN_READONLY )  oflags |= O_RDONLY;
  ਜੇ( flags&SQLITE_OPEN_READWRITE ) oflags |= O_RDWR;

  memset(p, 0, ਆਕਾਰ(DemoFile));
  p->fd = open(zName, oflags, 0600);
  ਜੇ( p->fd<0 ){
    sqlite3_free(aBuf);
    ਵਾਪਸ SQLITE_CANTOPEN;
  }
  p->a_ਬਫਰ = aBuf;

  ਜੇ( p_ਬਾਹਰਲਾ_Flags ){
    *p_ਬਾਹਰਲਾ_Flags = flags;
  }
  p->ਅਧਾਰ.pMethods = &demoio;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Delete the file identified by argument zPath. If the dirSync parameter
** is non-zero, then ensure the file-system modification to delete the
** file has been synced to disk before returning.
*/
ਸਥਿਰ ਪੂਰਨ demoDelete(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਪੂਰਨ dirSync){
  ਪੂਰਨ rc;                         /* Return code */

  rc = unlink(z_ਰਸਤਾ);
  ਜੇ( rc!=0 && errno==ENOENT ) ਵਾਪਸ SQLITE_OK;

  ਜੇ( rc==0 && dirSync ){
    ਪੂਰਨ dfd;                      /* File descriptor open on directory */
    ਅੱਖਰ *zSlash;
    ਅੱਖਰ zDir[MAXPATHNAME+1];     /* Name of directory containing file zPath */

    /* Figure out the directory name from the path of the file deleted. */
    sqlite3_snprintf(MAXPATHNAME, zDir, "%s", z_ਰਸਤਾ);
    zDir[MAXPATHNAME] = '\0';
    zSlash = strrchr(zDir,'/');
    ਜੇ( zSlash ){
      /* Open a file-descriptor on the directory. Sync. Close. */
      zSlash[0] = 0;
      dfd = open(zDir, O_RDONLY, 0);
      ਜੇ( dfd<0 ){
        rc = -1;
      }ਨਹੀਂਤਾਂ{
        rc = fsync(dfd);
        close(dfd);
      }
    }
  }
  ਵਾਪਸ (rc==0 ? SQLITE_OK : SQLITE_IOERR_DELETE);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ F_OK
# define F_OK 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ R_OK
# define R_OK 4
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ W_OK
# define W_OK 2
#ਜੇ_ਅੰਤ

/*
** Query the file-system to see if the named file exists, is readable or
** is both readable and writable.
*/
ਸਥਿਰ ਪੂਰਨ demoAccess(
  sqlite3_vfs *pVfs, 
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, 
  ਪੂਰਨ flags, 
  ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ
){
  ਪੂਰਨ rc;                         /* access() return code */
  ਪੂਰਨ eAccess = F_OK;             /* Second argument to access() */

  ਪ੍ਰਤਿਜ੍ਞਾ( flags==SQLITE_ACCESS_EXISTS       /* access(zPath, F_OK) */
       || flags==SQLITE_ACCESS_READ         /* access(zPath, R_OK) */
       || flags==SQLITE_ACCESS_READWRITE    /* access(zPath, R_OK|W_OK) */
  );

  ਜੇ( flags==SQLITE_ACCESS_READWRITE ) eAccess = R_OK|W_OK;
  ਜੇ( flags==SQLITE_ACCESS_READ )      eAccess = R_OK;

  rc = access(z_ਰਸਤਾ, eAccess);
  *p_Res_ਬਾਹਰਲਾ = (rc==0);
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ demo_ਭਰਿਆ_Pathname(
  sqlite3_vfs *pVfs,              /* VFS */
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,              /* Input path (possibly a relative path) */
  ਪੂਰਨ n_ਰਸਤਾ_ਬਾਹਰਲਾ,                   /* Size of output buffer in bytes */
  ਅੱਖਰ *z_ਰਸਤਾ_ਬਾਹਰਲਾ                  /* Pointer to output buffer */
){
  ਅੱਖਰ zDir[MAXPATHNAME+1];
  ਜੇ( z_ਰਸਤਾ[0]=='/' ){
    zDir[0] = '\0';
  }ਨਹੀਂਤਾਂ{
    ਜੇ( getcwd(zDir, ਆਕਾਰ(zDir))==0 ) ਵਾਪਸ SQLITE_IOERR;
  }
  zDir[MAXPATHNAME] = '\0';

  sqlite3_snprintf(n_ਰਸਤਾ_ਬਾਹਰਲਾ, z_ਰਸਤਾ_ਬਾਹਰਲਾ, "%s/%s", zDir, z_ਰਸਤਾ);
  z_ਰਸਤਾ_ਬਾਹਰਲਾ[n_ਰਸਤਾ_ਬਾਹਰਲਾ-1] = '\0';

  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਖਾਲੀ *demoDlOpen(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ){
  ਵਾਪਸ 0;
}
ਸਥਿਰ ਖਾਲੀ demoDlError(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ){
  sqlite3_snprintf(nByte, z_Err_ਸੁਨੇਹਾ, "Loadable extensions are not supported");
  z_Err_ਸੁਨੇਹਾ[nByte-1] = '\0';
}
ਸਥਿਰ ਖਾਲੀ (*demoDlSym(sqlite3_vfs *pVfs, ਖਾਲੀ *pH, ਅਚਲ ਅੱਖਰ *z))(ਖਾਲੀ){
  ਵਾਪਸ 0;
}
ਸਥਿਰ ਖਾਲੀ demoDlClose(sqlite3_vfs *pVfs, ਖਾਲੀ *pHandle){
  ਵਾਪਸ;
}

/*
** Parameter zByte points to a buffer nByte bytes in size. Populate this
** buffer with pseudo-random data.
*/
ਸਥਿਰ ਪੂਰਨ demoRandomness(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *zByte){
  ਵਾਪਸ SQLITE_OK;
}

/*
** Sleep for at least nMicro microseconds. Return the (approximate) number 
** of microseconds slept for.
*/
ਸਥਿਰ ਪੂਰਨ demoSleep(sqlite3_vfs *pVfs, ਪੂਰਨ nMicro){
  sleep(nMicro / 1000000);
  usleep(nMicro % 1000000);
  ਵਾਪਸ nMicro;
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
ਸਥਿਰ ਪੂਰਨ demo_Current_ਸਮਾਂ(sqlite3_vfs *pVfs, ਦੁੱਗਣਾ *p_ਸਮਾਂ){
  ਸਮਾਂ_t t = ਸਮਾਂ(0);
  *p_ਸਮਾਂ = t/86400.0 + 2440587.5; 
  ਵਾਪਸ SQLITE_OK;
}

/*
** This function returns a pointer to the VFS implemented in this file.
** To make the VFS available to SQLite:
**
**   sqlite3_vfs_register(sqlite3_demovfs(), 0);
*/
sqlite3_vfs *sqlite3_demovfs(ਖਾਲੀ){
  ਸਥਿਰ sqlite3_vfs demovfs = {
    1,                            /* iVersion */
    ਆਕਾਰ(DemoFile),             /* szOsFile */
    MAXPATHNAME,                  /* mxPathname */
    0,                            /* pNext */
    "demo",                       /* zName */
    0,                            /* pAppData */
    demoOpen,                     /* xOpen */
    demoDelete,                   /* xDelete */
    demoAccess,                   /* xAccess */
    demo_ਭਰਿਆ_Pathname,             /* xFullPathname */
    demoDlOpen,                   /* xDlOpen */
    demoDlError,                  /* xDlError */
    demoDlSym,                    /* xDlSym */
    demoDlClose,                  /* xDlClose */
    demoRandomness,               /* xRandomness */
    demoSleep,                    /* xSleep */
    demo_Current_ਸਮਾਂ,              /* xCurrentTime */
  };
  ਵਾਪਸ &demovfs;
}

#ਜੇ_ਅੰਤ /* !defined(SQLITE_TEST) || SQLITE_OS_UNIX */


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST

#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

#ਜੇ SQLITE_OS_UNIX
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਰਜਿਸਟਰ_demovfs(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_ਰਜਿਸਟਰ(sqlite3_demovfs(), 1);
  ਵਾਪਸ TCL_OK;
}
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI unregister_demovfs(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_unregister(sqlite3_demovfs());
  ਵਾਪਸ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetest_demovfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_demovfs", ਰਜਿਸਟਰ_demovfs, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_demovfs", unregister_demovfs, 0, 0);
  ਵਾਪਸ TCL_OK;
}

#ਨਹੀਂਤਾਂ
ਪੂਰਨ Sqlitetest_demovfs_Init(Tcl_Interp *interp){ ਵਾਪਸ TCL_OK; }
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* SQLITE_TEST */
