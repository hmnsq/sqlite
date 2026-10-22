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
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ "sqlite3.h"

/* The following #defines are copied from test_multiplex.c */
#ifndef MX_CHUNK_ಸಂಖ್ಯೆ 
# define MX_CHUNK_ಸಂಖ್ಯೆ 299
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
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3Delete83Name(ವರ್ಣ *z){
  ಪೂರ್ಣಾಙ್ಕ i, sz;
  sz = (ಪೂರ್ಣಾಙ್ಕ)strlen(z);
  ಪ್ರತಿ(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  ಯದಿ( z[i]=='.' && (sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  ಧ್ರುವ ವರ್ಣ *zFile, 
  ಪೂರ್ಣಾಙ್ಕ *pbExists
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_ERROR;
#ifdef SQLITE_OS_WIN
  ಯದಿ( pVfs ){
    ಯದಿ( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    ಯದಿ( rc==SQLITE_IOERR_DELETE_NOENT ){
      ಯದಿ( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#ಅನ್ಯಥಾ
  ಪ್ರತಿಜ್ಞಾ( pVfs==0 );
  rc = access(zFile, F_OK);
  ಯದಿ( rc ){
    ಯದಿ( errno==ENOENT ){ 
      ಯದಿ( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }ಅನ್ಯಥಾ{
    ಯದಿ( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API ಪೂರ್ಣಾಙ್ಕ sqlite3_delete_database(
  ಧ್ರುವ ವರ್ಣ *zFile               /* File to delete */
){
  ವರ್ಣ *zBuf;                     /* Buffer to sprintf() filenames to */
  ಪೂರ್ಣಾಙ್ಕ nBuf;                       /* Size of buffer in bytes */
  ಪೂರ್ಣಾಙ್ಕ rc = 0;                     /* System error code */
  ಪೂರ್ಣಾಙ್ಕ i;                          /* Iterate through azFmt[] and aMFile[] */

  ಧ್ರುವ ವರ್ಣ *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  ಸಂರಚನಾ MFile {
    ಧ್ರುವ ವರ್ಣ *zFmt;
    ಪೂರ್ಣಾಙ್ಕ iOffset;
    ಪೂರ್ಣಾಙ್ಕ b83;
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
#ಅನ್ಯಥಾ
  sqlite3_vfs *pVfs = 0;
#endif

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (ಪೂರ್ಣಾಙ್ಕ)strlen(zFile) + 100;
  zBuf = (ವರ್ಣ*)sqlite3_malloc(nBuf);
  ಯದಿ( zBuf==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  ಪ್ರತಿ(i=0; rc==0 && i<ಪರಿಮಾಣ(azFmt)/ಪರಿಮಾಣ(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    ಯದಿ( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  ಪ್ರತಿ(i=0; rc==0 && i<ಪರಿಮಾಣ(aMFile)/ಪರಿಮಾಣ(aMFile[0]); i++){
    ಸಂರಚನಾ MFile *p = &aMFile[i];
    ಪೂರ್ಣಾಙ್ಕ iChunk;
    ಪ್ರತಿ(iChunk=1; iChunk<=MX_CHUNK_ಸಂಖ್ಯೆ; iChunk++){
      ಪೂರ್ಣಾಙ್ಕ bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->iOffset);
      ಯದಿ( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      ಯದಿ( bExists==0 || rc!=0 ) ಭಙ್ಗ;
    }
  }

  sqlite3_free(zBuf);
  ಪ್ರತ್ಯಾಗಮ (rc ? SQLITE_ERROR : SQLITE_OK);
}
