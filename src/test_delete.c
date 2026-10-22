/*
** 2016 September 10
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains test code to delete an SQLite database and all
** of its associated files. Associated files include:
**
**   * The journal file.
**   * The wal file.
**   * The SQLITE_ENABLE_8_3_NAMES version of the db, journal or wal files.
**   * Files created by the test_multiplex.c module to extend any of the 
**     above.
*/

#ifndef SQLITE_OS_WIN
#  include <unistd.h>
#  include <errno.h>
#endif
#ضمن <string.h>
#ضمن <assert.h>
#ضمن "sqlite3.h"

/* The following #defines are copied from test_multiplex.c */
#ifndef MX_CHUNK_NUMBER 
# define MX_CHUNK_NUMBER 299
#endif
#ifndef SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET
# define SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET 400
#endif
#ifndef SQLITE_MULTIPLEX_WAL_8_3_OFFSET
# define SQLITE_MULTIPLEX_WAL_8_3_OFFSET 700
#endif

/*
** This routine is a copy of (most of) the code from SQLite function
** sqlite3FileSuffix3(). It modifies the filename in buffer z in the
** same way as SQLite does when in 8.3 filenames mode.
*/
ساكن فراغ sqlite3Delete83Name(حرف *z){
  صحيح i, sz;
  sz = (صحيح)strlen(z);
  لكل(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  𐤀𐤌( z[i]=='.' && (sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
ساكن صحيح sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  ثابت حرف *zFile, 
  صحيح *pbExists
){
  صحيح rc = SQLITE_ERROR;
#ifdef SQLITE_OS_WIN
  𐤀𐤌( pVfs ){
    𐤀𐤌( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    𐤀𐤌( rc==SQLITE_IOERR_DELETE_NOENT ){
      𐤀𐤌( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#وإلا
  أكد( pVfs==0 );
  rc = access(zFile, F_OK);
  𐤀𐤌( rc ){
    𐤀𐤌( errno==ENOENT ){ 
      𐤀𐤌( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }وإلا{
    𐤀𐤌( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#endif
  أرجع rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API صحيح sqlite3_delete_database(
  ثابت حرف *zFile               /* File to delete */
){
  حرف *zBuf;                     /* Buffer to sprintf() filenames to */
  صحيح nBuf;                       /* Size of buffer in bytes */
  صحيح rc = 0;                     /* System error code */
  صحيح i;                          /* Iterate through azFmt[] and aMFile[] */

  ثابت حرف *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  بنية MFile {
    ثابت حرف *zFmt;
    صحيح iOffset;
    صحيح b83;
  } aMFile[] = {
    { "%s%03d",         0,   0 },
    { "%s-journal%03d", 0,   0 },
    { "%s-wal%03d",     0,   0 },
    { "%s%03d",         0,   1 },
    { "%s-journal%03d", SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET, 1 },
    { "%s-wal%03d",     SQLITE_MULTIPLEX_WAL_8_3_OFFSET, 1 },
  };

#ifdef SQLITE_OS_WIN
  sqlite3_vfs *pVfs = sqlite3_vfs_find("win32");
#وإلا
  sqlite3_vfs *pVfs = 0;
#endif

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (صحيح)strlen(zFile) + 100;
  zBuf = (حرف*)sqlite3_malloc(nBuf);
  𐤀𐤌( zBuf==0 ) أرجع SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  لكل(i=0; rc==0 && i<حجم(azFmt)/حجم(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    𐤀𐤌( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  لكل(i=0; rc==0 && i<حجم(aMFile)/حجم(aMFile[0]); i++){
    بنية MFile *p = &aMFile[i];
    صحيح iChunk;
    لكل(iChunk=1; iChunk<=MX_CHUNK_NUMBER; iChunk++){
      صحيح bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->iOffset);
      𐤀𐤌( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      𐤀𐤌( bExists==0 || rc!=0 ) اكسر;
    }
  }

  sqlite3_free(zBuf);
  أرجع (rc ? SQLITE_ERROR : SQLITE_OK);
}
