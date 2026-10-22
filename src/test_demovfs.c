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

#अगर !defined(SQLITE_TEST) || SQLITE_OS_UNIX

#समावेश "sqlite3.h"

#समावेश <assert.h>
#समावेश <string.h>
#समावेश <sys/types.h>
#समावेश <sys/stat.h>
#समावेश <sys/file.h>
#समावेश <sys/param.h>
#समावेश <unistd.h>
#समावेश <time.h>
#समावेश <errno.h>
#समावेश <fcntl.h>

/*
** Size of the write buffer used by journal files in bytes.
*/
#अगर_अघोषित SQLITE_DEMOVFS_BUFFERSZ
# define SQLITE_DEMOVFS_BUFFERSZ 8192
#पूर्ण_अगर

/*
** The maximum pathname length supported by this VFS.
*/
#घोषणा MAXPATHNAME 512

/*
** When using this VFS, the sqlite3_file* handles that SQLite uses are
** actually pointers to instances of type DemoFile.
*/
प्रकार काष्ठा DemoFile DemoFile;
काष्ठा DemoFile {
  sqlite3_file base;              /* Base class. Must be first. */
  पूर्णांक fd;                         /* File descriptor */

  अक्षर *a_बफर;                  /* Pointer to malloc'd buffer */
  पूर्णांक n_बफर;                    /* Valid bytes of data in zBuffer */
  sqlite3_int64 i_बफर_Ofst;      /* Offset in file of zBuffer[0] */
};

/*
** Write directly to the file passed as the first argument. Even if the
** file has a write-buffer (DemoFile.aBuffer), ignore it.
*/
अटल पूर्णांक demoDirectWrite(
  DemoFile *p,                    /* File handle */
  स्थिर व्योम *zBuf,               /* Buffer containing data to write */
  पूर्णांक iAmt,                       /* Size of data to write in bytes */
  sqlite_int64 iOfst              /* File offset to write to */
){
  off_t ofst;                     /* Return value from lseek() */
  माप_प्रकार nWrite;                  /* Return value from write() */

  ofst = lseek(p->fd, iOfst, शुरू_से);
  अगर( ofst!=iOfst ){
    वापस SQLITE_IOERR_WRITE;
  }

  nWrite = ग_लिखो(p->fd, zBuf, iAmt);
  अगर( nWrite!=iAmt ){
    वापस SQLITE_IOERR_WRITE;
  }

  वापस SQLITE_OK;
}

/*
** Flush the contents of the DemoFile.aBuffer buffer to disk. This is a
** no-op if this particular file does not have a buffer (i.e. it is not
** a journal file) or if the buffer is currently empty.
*/
अटल पूर्णांक demo_Flush_बफर(DemoFile *p){
  पूर्णांक rc = SQLITE_OK;
  अगर( p->n_बफर ){
    rc = demoDirectWrite(p, p->a_बफर, p->n_बफर, p->i_बफर_Ofst);
    p->n_बफर = 0;
  }
  वापस rc;
}

/*
** Close a file.
*/
अटल पूर्णांक demoClose(sqlite3_file *pFile){
  पूर्णांक rc;
  DemoFile *p = (DemoFile*)pFile;
  rc = demo_Flush_बफर(p);
  sqlite3_free(p->a_बफर);
  बंद(p->fd);
  वापस rc;
}

/*
** Read data from a file.
*/
अटल पूर्णांक demoRead(
  sqlite3_file *pFile, 
  व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  off_t ofst;                     /* Return value from lseek() */
  पूर्णांक nRead;                      /* Return value from read() */
  पूर्णांक rc;                         /* Return code from demoFlushBuffer() */

  /* Flush any data in the write buffer to disk in case this operation
  ** is trying to read data the file-region currently cached in the buffer.
  ** It would be possible to detect this case and possibly save an 
  ** unnecessary write here, but in practice SQLite will rarely read from
  ** a journal file when there is data cached in the write-buffer.
  */
  rc = demo_Flush_बफर(p);
  अगर( rc!=SQLITE_OK ){
    वापस rc;
  }

  ofst = lseek(p->fd, iOfst, शुरू_से);
  अगर( ofst!=iOfst ){
    वापस SQLITE_IOERR_READ;
  }
  nRead = पढ़ो(p->fd, zBuf, iAmt);

  अगर( nRead==iAmt ){
    वापस SQLITE_OK;
  }अन्यथा अगर( nRead>=0 ){
    अगर( nRead<iAmt ){
      स_रखो(&((अक्षर*)zBuf)[nRead], 0, iAmt-nRead);
    }
    वापस SQLITE_IOERR_SHORT_READ;
  }

  वापस SQLITE_IOERR_READ;
}

/*
** Write data to a crash-file.
*/
अटल पूर्णांक demoWrite(
  sqlite3_file *pFile, 
  स्थिर व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  DemoFile *p = (DemoFile*)pFile;
  
  अगर( p->a_बफर ){
    अक्षर *z = (अक्षर *)zBuf;       /* Pointer to remaining data to write */
    पूर्णांक n = iAmt;                 /* Number of bytes at z */
    sqlite3_int64 i = iOfst;      /* File offset to write to */

    जबतक( n>0 ){
      पूर्णांक nCopy;                  /* Number of bytes to copy into buffer */

      /* If the buffer is full, or if this data is not being written directly
      ** following the data already buffered, flush the buffer. Flushing
      ** the buffer is a no-op if it is empty.  
      */
      अगर( p->n_बफर==SQLITE_DEMOVFS_BUFFERSZ || p->i_बफर_Ofst+p->n_बफर!=i ){
        पूर्णांक rc = demo_Flush_बफर(p);
        अगर( rc!=SQLITE_OK ){
          वापस rc;
        }
      }
      निश्चित( p->n_बफर==0 || p->i_बफर_Ofst+p->n_बफर==i );
      p->i_बफर_Ofst = i - p->n_बफर;

      /* Copy as much data as possible into the buffer. */
      nCopy = SQLITE_DEMOVFS_BUFFERSZ - p->n_बफर;
      अगर( nCopy>n ){
        nCopy = n;
      }
      स_नकल(&p->a_बफर[p->n_बफर], z, nCopy);
      p->n_बफर += nCopy;

      n -= nCopy;
      i += nCopy;
      z += nCopy;
    }
  }अन्यथा{
    वापस demoDirectWrite(p, zBuf, iAmt, iOfst);
  }

  वापस SQLITE_OK;
}

/*
** Truncate a file. This is a no-op for this VFS (see header comments at
** the top of the file).
*/
अटल पूर्णांक demoTruncate(sqlite3_file *pFile, sqlite_int64 आकार){
#अगर 0
  अगर( ftruncate(((DemoFile *)pFile)->fd, आकार) ) वापस SQLITE_IOERR_TRUNCATE;
#पूर्ण_अगर
  वापस SQLITE_OK;
}

/*
** Sync the contents of the file to the persistent media.
*/
अटल पूर्णांक demoSync(sqlite3_file *pFile, पूर्णांक flags){
  DemoFile *p = (DemoFile*)pFile;
  पूर्णांक rc;

  rc = demo_Flush_बफर(p);
  अगर( rc!=SQLITE_OK ){
    वापस rc;
  }

  rc = fsync(p->fd);
  वापस (rc==0 ? SQLITE_OK : SQLITE_IOERR_FSYNC);
}

/*
** Write the size of the file in bytes to *pSize.
*/
अटल पूर्णांक demo_File_आकार(sqlite3_file *pFile, sqlite_int64 *p_आकार){
  DemoFile *p = (DemoFile*)pFile;
  पूर्णांक rc;                         /* Return code from fstat() call */
  काष्ठा stat sStat;              /* Output of fstat() call */

  /* Flush the contents of the buffer to disk. As with the flush in the
  ** demoRead() method, it would be possible to avoid this and save a write
  ** here and there. But in practice this comes up so infrequently it is
  ** not worth the trouble.
  */
  rc = demo_Flush_बफर(p);
  अगर( rc!=SQLITE_OK ){
    वापस rc;
  }

  rc = ख_स्थिति(p->fd, &sStat);
  अगर( rc!=0 ) वापस SQLITE_IOERR_FSTAT;
  *p_आकार = sStat.st_आकार;
  वापस SQLITE_OK;
}

/*
** Locking functions. The xLock() and xUnlock() methods are both no-ops.
** The xCheckReservedLock() always indicates that no other process holds
** a reserved lock on the database file. This ensures that if a hot-journal
** file is found in the file-system it is rolled back.
*/
अटल पूर्णांक demoLock(sqlite3_file *pFile, पूर्णांक eLock){
  वापस SQLITE_OK;
}
अटल पूर्णांक demoUnlock(sqlite3_file *pFile, पूर्णांक eLock){
  वापस SQLITE_OK;
}
अटल पूर्णांक demoCheckReservedLock(sqlite3_file *pFile, पूर्णांक *pResOut){
  *pResOut = 0;
  वापस SQLITE_OK;
}

/*
** No xFileControl() verbs are implemented by this VFS.
*/
अटल पूर्णांक demoFileControl(sqlite3_file *pFile, पूर्णांक op, व्योम *pArg){
  वापस SQLITE_NOTFOUND;
}

/*
** The xSectorSize() and xDeviceCharacteristics() methods. These two
** may return special values allowing SQLite to optimize file-system 
** access to some extent. But it is also safe to simply return 0.
*/
अटल पूर्णांक demo_Sector_आकार(sqlite3_file *pFile){
  वापस 0;
}
अटल पूर्णांक demoDeviceCharacteristics(sqlite3_file *pFile){
  वापस 0;
}

/*
** Open a file handle.
*/
अटल पूर्णांक demoOpen(
  sqlite3_vfs *pVfs,              /* VFS */
  स्थिर अक्षर *zName,              /* File to open, or 0 for a temp file */
  sqlite3_file *pFile,            /* Pointer to DemoFile struct to populate */
  पूर्णांक flags,                      /* Input SQLITE_OPEN_XXX flags */
  पूर्णांक *pOutFlags                  /* Output SQLITE_OPEN_XXX flags (or NULL) */
){
  अटल स्थिर sqlite3_io_methods demoio = {
    1,                            /* iVersion */
    demoClose,                    /* xClose */
    demoRead,                     /* xRead */
    demoWrite,                    /* xWrite */
    demoTruncate,                 /* xTruncate */
    demoSync,                     /* xSync */
    demo_File_आकार,                 /* xFileSize */
    demoLock,                     /* xLock */
    demoUnlock,                   /* xUnlock */
    demoCheckReservedLock,        /* xCheckReservedLock */
    demoFileControl,              /* xFileControl */
    demo_Sector_आकार,               /* xSectorSize */
    demoDeviceCharacteristics     /* xDeviceCharacteristics */
  };

  DemoFile *p = (DemoFile*)pFile; /* Populate this structure */
  पूर्णांक oflags = 0;                 /* flags to pass to open() call */
  अक्षर *aBuf = 0;

  अगर( zName==0 ){
    वापस SQLITE_IOERR;
  }

  अगर( flags&SQLITE_OPEN_MAIN_JOURNAL ){
    aBuf = (अक्षर *)sqlite3_malloc(SQLITE_DEMOVFS_BUFFERSZ);
    अगर( !aBuf ){
      वापस SQLITE_NOMEM;
    }
  }

  अगर( flags&SQLITE_OPEN_EXCLUSIVE ) oflags |= O_EXCL;
  अगर( flags&SQLITE_OPEN_CREATE )    oflags |= O_CREAT;
  अगर( flags&SQLITE_OPEN_READONLY )  oflags |= O_RDONLY;
  अगर( flags&SQLITE_OPEN_READWRITE ) oflags |= O_RDWR;

  स_रखो(p, 0, माप(DemoFile));
  p->fd = खोलो(zName, oflags, 0600);
  अगर( p->fd<0 ){
    sqlite3_free(aBuf);
    वापस SQLITE_CANTOPEN;
  }
  p->a_बफर = aBuf;

  अगर( pOutFlags ){
    *pOutFlags = flags;
  }
  p->base.pMethods = &demoio;
  वापस SQLITE_OK;
}

/*
** Delete the file identified by argument zPath. If the dirSync parameter
** is non-zero, then ensure the file-system modification to delete the
** file has been synced to disk before returning.
*/
अटल पूर्णांक demoDelete(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath, पूर्णांक dirSync){
  पूर्णांक rc;                         /* Return code */

  rc = unlink(zPath);
  अगर( rc!=0 && त्रुटि_सं==ENOENT ) वापस SQLITE_OK;

  अगर( rc==0 && dirSync ){
    पूर्णांक dfd;                      /* File descriptor open on directory */
    अक्षर *zSlash;
    अक्षर zDir[MAXPATHNAME+1];     /* Name of directory containing file zPath */

    /* Figure out the directory name from the path of the file deleted. */
    sqlite3_snprintf(MAXPATHNAME, zDir, "%s", zPath);
    zDir[MAXPATHNAME] = '\0';
    zSlash = म_खोजप(zDir,'/');
    अगर( zSlash ){
      /* Open a file-descriptor on the directory. Sync. Close. */
      zSlash[0] = 0;
      dfd = खोलो(zDir, O_RDONLY, 0);
      अगर( dfd<0 ){
        rc = -1;
      }अन्यथा{
        rc = fsync(dfd);
        बंद(dfd);
      }
    }
  }
  वापस (rc==0 ? SQLITE_OK : SQLITE_IOERR_DELETE);
}

#अगर_अघोषित F_OK
# define F_OK 0
#पूर्ण_अगर
#अगर_अघोषित R_OK
# define R_OK 4
#पूर्ण_अगर
#अगर_अघोषित W_OK
# define W_OK 2
#पूर्ण_अगर

/*
** Query the file-system to see if the named file exists, is readable or
** is both readable and writable.
*/
अटल पूर्णांक demoAccess(
  sqlite3_vfs *pVfs, 
  स्थिर अक्षर *zPath, 
  पूर्णांक flags, 
  पूर्णांक *pResOut
){
  पूर्णांक rc;                         /* access() return code */
  पूर्णांक eAccess = F_OK;             /* Second argument to access() */

  निश्चित( flags==SQLITE_ACCESS_EXISTS       /* access(zPath, F_OK) */
       || flags==SQLITE_ACCESS_READ         /* access(zPath, R_OK) */
       || flags==SQLITE_ACCESS_READWRITE    /* access(zPath, R_OK|W_OK) */
  );

  अगर( flags==SQLITE_ACCESS_READWRITE ) eAccess = R_OK|W_OK;
  अगर( flags==SQLITE_ACCESS_READ )      eAccess = R_OK;

  rc = access(zPath, eAccess);
  *pResOut = (rc==0);
  वापस SQLITE_OK;
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
अटल पूर्णांक demoFullPathname(
  sqlite3_vfs *pVfs,              /* VFS */
  स्थिर अक्षर *zPath,              /* Input path (possibly a relative path) */
  पूर्णांक nPathOut,                   /* Size of output buffer in bytes */
  अक्षर *zPathOut                  /* Pointer to output buffer */
){
  अक्षर zDir[MAXPATHNAME+1];
  अगर( zPath[0]=='/' ){
    zDir[0] = '\0';
  }अन्यथा{
    अगर( getcwd(zDir, माप(zDir))==0 ) वापस SQLITE_IOERR;
  }
  zDir[MAXPATHNAME] = '\0';

  sqlite3_snprintf(nPathOut, zPathOut, "%s/%s", zDir, zPath);
  zPathOut[nPathOut-1] = '\0';

  वापस SQLITE_OK;
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
अटल व्योम *demoDlOpen(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath){
  वापस 0;
}
अटल व्योम demoDlError(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *z_Err_संदेश){
  sqlite3_snprintf(nByte, z_Err_संदेश, "Loadable extensions are not supported");
  z_Err_संदेश[nByte-1] = '\0';
}
अटल व्योम (*demoDlSym(sqlite3_vfs *pVfs, व्योम *pH, स्थिर अक्षर *z))(व्योम){
  वापस 0;
}
अटल व्योम demoDlClose(sqlite3_vfs *pVfs, व्योम *pHandle){
  वापस;
}

/*
** Parameter zByte points to a buffer nByte bytes in size. Populate this
** buffer with pseudo-random data.
*/
अटल पूर्णांक demoRandomness(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zByte){
  वापस SQLITE_OK;
}

/*
** Sleep for at least nMicro microseconds. Return the (approximate) number 
** of microseconds slept for.
*/
अटल पूर्णांक demoSleep(sqlite3_vfs *pVfs, पूर्णांक nMicro){
  sleep(nMicro / 1000000);
  usleep(nMicro % 1000000);
  वापस nMicro;
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
अटल पूर्णांक demoCurrentTime(sqlite3_vfs *pVfs, द्विगुन *pTime){
  समय_प्रकार t = समय(0);
  *pTime = t/86400.0 + 2440587.5; 
  वापस SQLITE_OK;
}

/*
** This function returns a pointer to the VFS implemented in this file.
** To make the VFS available to SQLite:
**
**   sqlite3_vfs_register(sqlite3_demovfs(), 0);
*/
sqlite3_vfs *sqlite3_demovfs(व्योम){
  अटल sqlite3_vfs demovfs = {
    1,                            /* iVersion */
    माप(DemoFile),             /* szOsFile */
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
  वापस &demovfs;
}

#पूर्ण_अगर /* !defined(SQLITE_TEST) || SQLITE_OS_UNIX */


#अगर_घोषित SQLITE_TEST

#समावेश "tclsqlite.h"

#अगर SQLITE_OS_UNIX
अटल पूर्णांक SQLITE_TCLAPI register_demovfs(
  Client_तथ्य client_तथ्य, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_register(sqlite3_demovfs(), 1);
  वापस TCL_OK;
}
अटल पूर्णांक SQLITE_TCLAPI unregister_demovfs(
  Client_तथ्य client_तथ्य, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3_vfs_unregister(sqlite3_demovfs());
  वापस TCL_OK;
}

/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetest_demovfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "register_demovfs", register_demovfs, 0, 0);
  Tcl_CreateObjCommand(interp, "unregister_demovfs", unregister_demovfs, 0, 0);
  वापस TCL_OK;
}

#अन्यथा
पूर्णांक Sqlitetest_demovfs_Init(Tcl_Interp *interp){ वापस TCL_OK; }
#पूर्ण_अगर

#पूर्ण_अगर /* SQLITE_TEST */
