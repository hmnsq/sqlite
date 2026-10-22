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

#ಆದರೆ !defined(SQLITE_TEST) || SQLITE_OS_UNIX

#ಅನ್ತರ್ಭಾವ "sqlite3.h"

#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/file.h>
#ಅನ್ತರ್ಭಾವ <sys/param.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>

/*
** Size of the write buffer used by journal files in bytes.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEMOVFS_BUFFERSZ
# define SQLITE_DEMOVFS_BUFFERSZ 8192
#ಆದರೆ_ಅಂತ್ಯ

/*
** The maximum pathname length supported by this VFS.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAXPATHNAME 512

/*
** When using this VFS, the sqlite3_file* handles that SQLite uses are
** actually pointers to instances of type DemoFile.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ DemoFile DemoFile;
ರಚನೆ DemoFile {
  sqlite3_file ತಳ;              /* Base class. Must be first. */
  ಪೂರ್ಣಾಂಕ fd;                         /* File descriptor */

  ಅಕ್ಷರ *a_ಬಫರ್;                  /* Pointer to malloc'd buffer */
  ಪೂರ್ಣಾಂಕ n_ಬಫರ್;                    /* Valid bytes of data in zBuffer */
  sqlite3_int64 i_ಬಫರ್_Ofst;      /* Offset in file of zBuffer[0] */
};

/*
** Write directly to the file passed as the first argument. Even if the
** file has a write-buffer (DemoFile.aBuffer), ignore it.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demo_ನೇರ_Write(
  DemoFile *p,                    /* File handle */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zBuf,               /* Buffer containing data to write */
  ಪೂರ್ಣಾಂಕ iAmt,                       /* Size of data to write in bytes */
  sqlite_int64 iOfst              /* File offset to write to */
){
  off_t ofst;                     /* Return value from lseek() */
  size_t nWrite;                  /* Return value from write() */

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  ಆದರೆ( ofst!=iOfst ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_WRITE;
  }

  nWrite = ಲಿಖ(p->fd, zBuf, iAmt);
  ಆದರೆ( nWrite!=iAmt ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_WRITE;
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Flush the contents of the DemoFile.aBuffer buffer to disk. This is a
** no-op if this particular file does not have a buffer (i.e. it is not
** a journal file) or if the buffer is currently empty.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demo_Flush_ಬಫರ್(DemoFile *p){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( p->n_ಬಫರ್ ){
    rc = demo_ನೇರ_Write(p, p->a_ಬಫರ್, p->n_ಬಫರ್, p->i_ಬಫರ್_Ofst);
    p->n_ಬಫರ್ = 0;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Close a file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demoClose(sqlite3_file *pFile){
  ಪೂರ್ಣಾಂಕ rc;
  DemoFile *p = (DemoFile*)pFile;
  rc = demo_Flush_ಬಫರ್(p);
  sqlite3_free(p->a_ಬಫರ್);
  close(p->fd);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Read data from a file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demoRead(
  sqlite3_file *pFile, 
  ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  off_t ofst;                     /* Return value from lseek() */
  ಪೂರ್ಣಾಂಕ nRead;                      /* Return value from read() */
  ಪೂರ್ಣಾಂಕ rc;                         /* Return code from demoFlushBuffer() */

  /* Flush any data in the write buffer to disk in case this operation
  ** is trying to read data the file-region currently cached in the buffer.
  ** It would be possible to detect this case and possibly save an 
  ** unnecessary write here, but in practice SQLite will rarely read from
  ** a journal file when there is data cached in the write-buffer.
  */
  rc = demo_Flush_ಬಫರ್(p);
  ಆದರೆ( rc!=SQLITE_OK ){
    ಹಿಂತಿರುಗಿ rc;
  }

  ofst = lseek(p->fd, iOfst, SEEK_SET);
  ಆದರೆ( ofst!=iOfst ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_READ;
  }
  nRead = ಪಠ(p->fd, zBuf, iAmt);

  ಆದರೆ( nRead==iAmt ){
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nRead>=0 ){
    ಆದರೆ( nRead<iAmt ){
      memset(&((ಅಕ್ಷರ*)zBuf)[nRead], 0, iAmt-nRead);
    }
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_SHORT_READ;
  }

  ಹಿಂತಿರುಗಿ SQLITE_IOERR_READ;
}

/*
** Write data to a crash-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demoWrite(
  sqlite3_file *pFile, 
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  
  ಆದರೆ( p->a_ಬಫರ್ ){
    ಅಕ್ಷರ *z = (ಅಕ್ಷರ *)zBuf;       /* Pointer to remaining data to write */
    ಪೂರ್ಣಾಂಕ n = iAmt;                 /* Number of bytes at z */
    sqlite3_int64 i = iOfst;      /* File offset to write to */

    ಆಗ( n>0 ){
      ಪೂರ್ಣಾಂಕ nCopy;                  /* Number of bytes to copy into buffer */

      /* If the buffer is full, or if this data is not being written directly
      ** following the data already buffered, flush the buffer. Flushing
      ** the buffer is a no-op if it is empty.  
      */
      ಆದರೆ( p->n_ಬಫರ್==SQLITE_DEMOVFS_BUFFERSZ || p->i_ಬಫರ್_Ofst+p->n_ಬಫರ್!=i ){
        ಪೂರ್ಣಾಂಕ rc = demo_Flush_ಬಫರ್(p);
        ಆದರೆ( rc!=SQLITE_OK ){
          ಹಿಂತಿರುಗಿ rc;
        }
      }
      ಪ್ರತಿಜ್ಞಾ( p->n_ಬಫರ್==0 || p->i_ಬಫರ್_Ofst+p->n_ಬಫರ್==i );
      p->i_ಬಫರ್_Ofst = i - p->n_ಬಫರ್;

      /* Copy as much data as possible into the buffer. */
      nCopy = SQLITE_DEMOVFS_BUFFERSZ - p->n_ಬಫರ್;
      ಆದರೆ( nCopy>n ){
        nCopy = n;
      }
      ಸ್ಮರಣೆ_ನಕಲು(&p->a_ಬಫರ್[p->n_ಬಫರ್], z, nCopy);
      p->n_ಬಫರ್ += nCopy;

      n -= nCopy;
      i += nCopy;
      z += nCopy;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ demo_ನೇರ_Write(p, zBuf, iAmt, iOfst);
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Truncate a file. This is a no-op for this VFS (see header comments at
** the top of the file).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demoTruncate(sqlite3_file *pFile, sqlite_int64 ಗಾತ್ರ_2){
#ಆದರೆ 0
  ಆದರೆ( ftruncate(((DemoFile *)pFile)->fd, ಗಾತ್ರ_2) ) ಹಿಂತಿರುಗಿ SQLITE_IOERR_TRUNCATE;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Sync the contents of the file to the persistent media.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demoSync(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ flags){
  DemoFile *p = (DemoFile*)pFile;
  ಪೂರ್ಣಾಂಕ rc;

  rc = demo_Flush_ಬಫರ್(p);
  ಆದರೆ( rc!=SQLITE_OK ){
    ಹಿಂತಿರುಗಿ rc;
  }

  rc = fsync(p->fd);
  ಹಿಂತಿರುಗಿ (rc==0 ? SQLITE_OK : SQLITE_IOERR_FSYNC);
}

/*
** Write the size of the file in bytes to *pSize.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demo_File_ಗಾತ್ರ(sqlite3_file *pFile, sqlite_int64 *p_ಗಾತ್ರ){
  DemoFile *p = (DemoFile*)pFile;
  ಪೂರ್ಣಾಂಕ rc;                         /* Return code from fstat() call */
  ರಚನೆ stat sStat;              /* Output of fstat() call */

  /* Flush the contents of the buffer to disk. As with the flush in the
  ** demoRead() method, it would be possible to avoid this and save a write
  ** here and there. But in practice this comes up so infrequently it is
  ** not worth the trouble.
  */
  rc = demo_Flush_ಬಫರ್(p);
  ಆದರೆ( rc!=SQLITE_OK ){
    ಹಿಂತಿರುಗಿ rc;
  }

  rc = fstat(p->fd, &sStat);
  ಆದರೆ( rc!=0 ) ಹಿಂತಿರುಗಿ SQLITE_IOERR_FSTAT;
  *p_ಗಾತ್ರ = sStat.st_ಗಾತ್ರ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Locking functions. The xLock() and xUnlock() methods are both no-ops.
** The xCheckReservedLock() always indicates that no other process holds
** a reserved lock on the database file. This ensures that if a hot-journal
** file is found in the file-system it is rolled back.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demo_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demoUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demo_Check_Reserved_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  *p_Res_ಹೊರಹೋಗು = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** No xFileControl() verbs are implemented by this VFS.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demoFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  ಹಿಂತಿರುಗಿ SQLITE_NOTFOUND;
}

/*
** The xSectorSize() and xDeviceCharacteristics() methods. These two
** may return special values allowing SQLite to optimize file-system 
** access to some extent. But it is also safe to simply return 0.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demo_Sector_ಗಾತ್ರ(sqlite3_file *pFile){
  ಹಿಂತಿರುಗಿ 0;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demoDeviceCharacteristics(sqlite3_file *pFile){
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Open a file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demoOpen(
  sqlite3_vfs *pVfs,              /* VFS */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,              /* File to open, or 0 for a temp file */
  sqlite3_file *pFile,            /* Pointer to DemoFile struct to populate */
  ಪೂರ್ಣಾಂಕ flags,                      /* Input SQLITE_OPEN_XXX flags */
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags                  /* Output SQLITE_OPEN_XXX flags (or NULL) */
){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_io_methods demoio = {
    1,                            /* iVersion */
    demoClose,                    /* xClose */
    demoRead,                     /* xRead */
    demoWrite,                    /* xWrite */
    demoTruncate,                 /* xTruncate */
    demoSync,                     /* xSync */
    demo_File_ಗಾತ್ರ,                 /* xFileSize */
    demo_ಬೀಗ,                     /* xLock */
    demoUnlock,                   /* xUnlock */
    demo_Check_Reserved_ಬೀಗ,        /* xCheckReservedLock */
    demoFileControl,              /* xFileControl */
    demo_Sector_ಗಾತ್ರ,               /* xSectorSize */
    demoDeviceCharacteristics     /* xDeviceCharacteristics */
  };

  DemoFile *p = (DemoFile*)pFile; /* Populate this structure */
  ಪೂರ್ಣಾಂಕ oflags = 0;                 /* flags to pass to open() call */
  ಅಕ್ಷರ *aBuf = 0;

  ಆದರೆ( zName==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR;
  }

  ಆದರೆ( flags&SQLITE_OPEN_MAIN_JOURNAL ){
    aBuf = (ಅಕ್ಷರ *)sqlite3_malloc(SQLITE_DEMOVFS_BUFFERSZ);
    ಆದರೆ( !aBuf ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }
  }

  ಆದರೆ( flags&SQLITE_OPEN_EXCLUSIVE ) oflags |= O_EXCL;
  ಆದರೆ( flags&SQLITE_OPEN_CREATE )    oflags |= O_CREAT;
  ಆದರೆ( flags&SQLITE_OPEN_READONLY )  oflags |= O_RDONLY;
  ಆದರೆ( flags&SQLITE_OPEN_READWRITE ) oflags |= O_RDWR;

  memset(p, 0, ಗಾತ್ರ(DemoFile));
  p->fd = open(zName, oflags, 0600);
  ಆದರೆ( p->fd<0 ){
    sqlite3_free(aBuf);
    ಹಿಂತಿರುಗಿ SQLITE_CANTOPEN;
  }
  p->a_ಬಫರ್ = aBuf;

  ಆದರೆ( p_ಹೊರಹೋಗು_Flags ){
    *p_ಹೊರಹೋಗು_Flags = flags;
  }
  p->ತಳ.pMethods = &demoio;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Delete the file identified by argument zPath. If the dirSync parameter
** is non-zero, then ensure the file-system modification to delete the
** file has been synced to disk before returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demoDelete(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ dirSync){
  ಪೂರ್ಣಾಂಕ rc;                         /* Return code */

  rc = unlink(z_ಮಾರ್ಗ);
  ಆದರೆ( rc!=0 && errno==ENOENT ) ಹಿಂತಿರುಗಿ SQLITE_OK;

  ಆದರೆ( rc==0 && dirSync ){
    ಪೂರ್ಣಾಂಕ dfd;                      /* File descriptor open on directory */
    ಅಕ್ಷರ *zSlash;
    ಅಕ್ಷರ zDir[MAXPATHNAME+1];     /* Name of directory containing file zPath */

    /* Figure out the directory name from the path of the file deleted. */
    sqlite3_snprintf(MAXPATHNAME, zDir, "%s", z_ಮಾರ್ಗ);
    zDir[MAXPATHNAME] = '\0';
    zSlash = strrchr(zDir,'/');
    ಆದರೆ( zSlash ){
      /* Open a file-descriptor on the directory. Sync. Close. */
      zSlash[0] = 0;
      dfd = open(zDir, O_RDONLY, 0);
      ಆದರೆ( dfd<0 ){
        rc = -1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = fsync(dfd);
        close(dfd);
      }
    }
  }
  ಹಿಂತಿರುಗಿ (rc==0 ? SQLITE_OK : SQLITE_IOERR_DELETE);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ F_OK
# define F_OK 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ R_OK
# define R_OK 4
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ W_OK
# define W_OK 2
#ಆದರೆ_ಅಂತ್ಯ

/*
** Query the file-system to see if the named file exists, is readable or
** is both readable and writable.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demoAccess(
  sqlite3_vfs *pVfs, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, 
  ಪೂರ್ಣಾಂಕ flags, 
  ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು
){
  ಪೂರ್ಣಾಂಕ rc;                         /* access() return code */
  ಪೂರ್ಣಾಂಕ eAccess = F_OK;             /* Second argument to access() */

  ಪ್ರತಿಜ್ಞಾ( flags==SQLITE_ACCESS_EXISTS       /* access(zPath, F_OK) */
       || flags==SQLITE_ACCESS_READ         /* access(zPath, R_OK) */
       || flags==SQLITE_ACCESS_READWRITE    /* access(zPath, R_OK|W_OK) */
  );

  ಆದರೆ( flags==SQLITE_ACCESS_READWRITE ) eAccess = R_OK|W_OK;
  ಆದರೆ( flags==SQLITE_ACCESS_READ )      eAccess = R_OK;

  rc = access(z_ಮಾರ್ಗ, eAccess);
  *p_Res_ಹೊರಹೋಗು = (rc==0);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demo_ತುಂಬಿದ_Pathname(
  sqlite3_vfs *pVfs,              /* VFS */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ,              /* Input path (possibly a relative path) */
  ಪೂರ್ಣಾಂಕ n_ಮಾರ್ಗ_ಹೊರಹೋಗು,                   /* Size of output buffer in bytes */
  ಅಕ್ಷರ *z_ಮಾರ್ಗ_ಹೊರಹೋಗು                  /* Pointer to output buffer */
){
  ಅಕ್ಷರ zDir[MAXPATHNAME+1];
  ಆದರೆ( z_ಮಾರ್ಗ[0]=='/' ){
    zDir[0] = '\0';
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( getcwd(zDir, ಗಾತ್ರ(zDir))==0 ) ಹಿಂತಿರುಗಿ SQLITE_IOERR;
  }
  zDir[MAXPATHNAME] = '\0';

  sqlite3_snprintf(n_ಮಾರ್ಗ_ಹೊರಹೋಗು, z_ಮಾರ್ಗ_ಹೊರಹೋಗು, "%s/%s", zDir, z_ಮಾರ್ಗ);
  z_ಮಾರ್ಗ_ಹೊರಹೋಗು[n_ಮಾರ್ಗ_ಹೊರಹೋಗು-1] = '\0';

  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಶೂನ್ಯ *demoDlOpen(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ){
  ಹಿಂತಿರುಗಿ 0;
}
ಸ್ಥಿರ ಶೂನ್ಯ demoDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Err_ಸಂದೇಶ){
  sqlite3_snprintf(nByte, z_Err_ಸಂದೇಶ, "Loadable extensions are not supported");
  z_Err_ಸಂದೇಶ[nByte-1] = '\0';
}
ಸ್ಥಿರ ಶೂನ್ಯ (*demoDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pH, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z))(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ 0;
}
ಸ್ಥಿರ ಶೂನ್ಯ demoDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  ಹಿಂತಿರುಗಿ;
}

/*
** Parameter zByte points to a buffer nByte bytes in size. Populate this
** buffer with pseudo-random data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demoRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *zByte){
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Sleep for at least nMicro microseconds. Return the (approximate) number 
** of microseconds slept for.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demoSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nMicro){
  sleep(nMicro / 1000000);
  usleep(nMicro % 1000000);
  ಹಿಂತಿರುಗಿ nMicro;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ demo_Current_ಸಮಯ(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *p_ಸಮಯ){
  ಸಮಯ_t t = ಸಮಯ(0);
  *p_ಸಮಯ = t/86400.0 + 2440587.5; 
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** This function returns a pointer to the VFS implemented in this file.
** To make the VFS available to SQLite:
**
**   sqlite3_vfs_register(sqlite3_demovfs(), 0);
*/
sqlite3_vfs *sqlite3_demovfs(ಶೂನ್ಯ){
  ಸ್ಥಿರ sqlite3_vfs demovfs = {
    1,                            /* iVersion */
    ಗಾತ್ರ(DemoFile),             /* szOsFile */
    MAXPATHNAME,                  /* mxPathname */
    0,                            /* pNext */
    "demo",                       /* zName */
    0,                            /* pAppData */
    demoOpen,                     /* xOpen */
    demoDelete,                   /* xDelete */
    demoAccess,                   /* xAccess */
    demo_ತುಂಬಿದ_Pathname,             /* xFullPathname */
    demoDlOpen,                   /* xDlOpen */
    demoDlError,                  /* xDlError */
    demoDlSym,                    /* xDlSym */
    demoDlClose,                  /* xDlClose */
    demoRandomness,               /* xRandomness */
    demoSleep,                    /* xSleep */
    demo_Current_ಸಮಯ,              /* xCurrentTime */
  };
  ಹಿಂತಿರುಗಿ &demovfs;
}

#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_TEST) || SQLITE_OS_UNIX */


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST

#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

#ಆದರೆ SQLITE_OS_UNIX
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI ರಿಜಿಸ್ಟರ್_demovfs(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_ರಿಜಿಸ್ಟರ್(sqlite3_demovfs(), 1);
  ಹಿಂತಿರುಗಿ TCL_OK;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI unregister_demovfs(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_unregister(sqlite3_demovfs());
  ಹಿಂತಿರುಗಿ TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetest_demovfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_demovfs", ರಿಜಿಸ್ಟರ್_demovfs, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_demovfs", unregister_demovfs, 0, 0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ Sqlitetest_demovfs_Init(Tcl_Interp *interp){ ಹಿಂತಿರುಗಿ TCL_OK; }
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_TEST */
