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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OS_WIN
#  include <unistd.h>
#  include <errno.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"

/* The following #defines are copied from test_multiplex.c */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MX_CHUNK_ਸੰਖਿਆ 
# define MX_CHUNK_ਸੰਖਿਆ 299
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MULTIPLEX_JOURNAL_8_3_ਵਿੱਥ
# define SQLITE_MULTIPLEX_JOURNAL_8_3_ਵਿੱਥ 400
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MULTIPLEX_WAL_8_3_ਵਿੱਥ
# define SQLITE_MULTIPLEX_WAL_8_3_ਵਿੱਥ 700
#ਜੇ_ਅੰਤ

/*
** This routine is a copy of (most of) the code from SQLite function
** sqlite3FileSuffix3(). It modifies the filename in buffer z in the
** same way as SQLite does when in 8.3 filenames mode.
*/
ਸਥਿਰ ਖਾਲੀ sqlite3Delete83Name(ਅੱਖਰ *z){
  ਪੂਰਨ i, sz;
  sz = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(z);
  ਲਈ(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  ਜੇ( z[i]=='.' && (sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
ਸਥਿਰ ਪੂਰਨ sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  ਅਚਲ ਅੱਖਰ *zFile, 
  ਪੂਰਨ *pbExists
){
  ਪੂਰਨ rc = SQLITE_ERROR;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OS_WIN
  ਜੇ( pVfs ){
    ਜੇ( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    ਜੇ( rc==SQLITE_IOERR_DELETE_NOENT ){
      ਜੇ( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#ਨਹੀਂਤਾਂ
  ਪ੍ਰਤਿਜ੍ਞਾ( pVfs==0 );
  rc = access(zFile, F_OK);
  ਜੇ( rc ){
    ਜੇ( errno==ENOENT ){ 
      ਜੇ( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }ਨਹੀਂਤਾਂ{
    ਜੇ( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API ਪੂਰਨ sqlite3_delete_database(
  ਅਚਲ ਅੱਖਰ *zFile               /* File to delete */
){
  ਅੱਖਰ *zBuf;                     /* Buffer to sprintf() filenames to */
  ਪੂਰਨ nBuf;                       /* Size of buffer in bytes */
  ਪੂਰਨ rc = 0;                     /* System error code */
  ਪੂਰਨ i;                          /* Iterate through azFmt[] and aMFile[] */

  ਅਚਲ ਅੱਖਰ *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  ਬਣਤਰ MFile {
    ਅਚਲ ਅੱਖਰ *zFmt;
    ਪੂਰਨ i_ਵਿੱਥ;
    ਪੂਰਨ b83;
  } aMFile[] = {
    { "%s%03d",         0,   0 },
    { "%s-journal%03d", 0,   0 },
    { "%s-wal%03d",     0,   0 },
    { "%s%03d",         0,   1 },
    { "%s-journal%03d", SQLITE_MULTIPLEX_JOURNAL_8_3_ਵਿੱਥ, 1 },
    { "%s-wal%03d",     SQLITE_MULTIPLEX_WAL_8_3_ਵਿੱਥ, 1 },
  };

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OS_WIN
  sqlite3_vfs *pVfs = sqlite3_vfs_ਲੱਭੋ("win32");
#ਨਹੀਂਤਾਂ
  sqlite3_vfs *pVfs = 0;
#ਜੇ_ਅੰਤ

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zFile) + 100;
  zBuf = (ਅੱਖਰ*)sqlite3_malloc(nBuf);
  ਜੇ( zBuf==0 ) ਵਾਪਸ SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  ਲਈ(i=0; rc==0 && i<ਆਕਾਰ(azFmt)/ਆਕਾਰ(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    ਜੇ( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  ਲਈ(i=0; rc==0 && i<ਆਕਾਰ(aMFile)/ਆਕਾਰ(aMFile[0]); i++){
    ਬਣਤਰ MFile *p = &aMFile[i];
    ਪੂਰਨ iChunk;
    ਲਈ(iChunk=1; iChunk<=MX_CHUNK_ਸੰਖਿਆ; iChunk++){
      ਪੂਰਨ bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->i_ਵਿੱਥ);
      ਜੇ( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      ਜੇ( bExists==0 || rc!=0 ) ਰੋਕੋ;
    }
  }

  sqlite3_free(zBuf);
  ਵਾਪਸ (rc ? SQLITE_ERROR : SQLITE_OK);
}
