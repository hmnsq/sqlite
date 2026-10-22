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
#include <string.h>
#include <assert.h>
#include "sqlite3.h"

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
static void sqlite3Delete83Name(char *z){
  целое i, sz;
  sz = (целое)strlen(z);
  для(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  если( z[i]=='.' && (sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
static целое sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  const char *zFile, 
  целое *pbExists
){
  целое rc = SQLITE_ERROR;
#ifdef SQLITE_OS_WIN
  если( pVfs ){
    если( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    если( rc==SQLITE_IOERR_DELETE_NOENT ){
      если( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#иначе
  assert( pVfs==0 );
  rc = access(zFile, F_OK);
  если( rc ){
    если( errno==ENOENT ){ 
      если( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }иначе{
    если( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#endif
  вернуть rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API целое sqlite3_delete_database(
  const char *zFile               /* File to delete */
){
  char *zBuf;                     /* Buffer to sprintf() filenames to */
  целое nBuf;                       /* Size of buffer in bytes */
  целое rc = 0;                     /* System error code */
  целое i;                          /* Iterate through azFmt[] and aMFile[] */

  const char *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  struct MFile {
    const char *zFmt;
    целое iOffset;
    целое b83;
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
#иначе
  sqlite3_vfs *pVfs = 0;
#endif

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (целое)strlen(zFile) + 100;
  zBuf = (char*)sqlite3_malloc(nBuf);
  если( zBuf==0 ) вернуть SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  для(i=0; rc==0 && i<sizeof(azFmt)/sizeof(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    если( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  для(i=0; rc==0 && i<sizeof(aMFile)/sizeof(aMFile[0]); i++){
    struct MFile *p = &aMFile[i];
    целое iChunk;
    для(iChunk=1; iChunk<=MX_CHUNK_NUMBER; iChunk++){
      целое bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->iOffset);
      если( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      если( bExists==0 || rc!=0 ) break;
    }
  }

  sqlite3_free(zBuf);
  вернуть (rc ? SQLITE_ERROR : SQLITE_OK);
}
