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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OS_WIN
#  include <unistd.h>
#  include <errno.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ಅನ್ತರ್ಭಾವ "sqlite3.h"

/* The following #defines are copied from test_multiplex.c */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ MX_CHUNK_ಸಂಖ್ಯೆ 
# define MX_CHUNK_ಸಂಖ್ಯೆ 299
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MULTIPLEX_JOURNAL_8_3_ಸ್ಥಾನಾಂತರ
# define SQLITE_MULTIPLEX_JOURNAL_8_3_ಸ್ಥಾನಾಂತರ 400
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MULTIPLEX_WAL_8_3_ಸ್ಥಾನಾಂತರ
# define SQLITE_MULTIPLEX_WAL_8_3_ಸ್ಥಾನಾಂತರ 700
#ಆದರೆ_ಅಂತ್ಯ

/*
** This routine is a copy of (most of) the code from SQLite function
** sqlite3FileSuffix3(). It modifies the filename in buffer z in the
** same way as SQLite does when in 8.3 filenames mode.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3Delete83Name(ಅಕ್ಷರ *z){
  ಪೂರ್ಣಾಂಕ i, sz;
  sz = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z);
  ಗಾಗಿ(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  ಆದರೆ( z[i]=='.' && (sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile, 
  ಪೂರ್ಣಾಂಕ *pbExists
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_ERROR;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OS_WIN
  ಆದರೆ( pVfs ){
    ಆದರೆ( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    ಆದರೆ( rc==SQLITE_IOERR_DELETE_NOENT ){
      ಆದರೆ( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರತಿಜ್ಞಾ( pVfs==0 );
  rc = access(zFile, F_OK);
  ಆದರೆ( rc ){
    ಆದರೆ( errno==ENOENT ){ 
      ಆದರೆ( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API ಪೂರ್ಣಾಂಕ sqlite3_delete_database(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile               /* File to delete */
){
  ಅಕ್ಷರ *zBuf;                     /* Buffer to sprintf() filenames to */
  ಪೂರ್ಣಾಂಕ nBuf;                       /* Size of buffer in bytes */
  ಪೂರ್ಣಾಂಕ rc = 0;                     /* System error code */
  ಪೂರ್ಣಾಂಕ i;                          /* Iterate through azFmt[] and aMFile[] */

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  ರಚನೆ MFile {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFmt;
    ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ;
    ಪೂರ್ಣಾಂಕ b83;
  } aMFile[] = {
    { "%s%03d",         0,   0 },
    { "%s-journal%03d", 0,   0 },
    { "%s-wal%03d",     0,   0 },
    { "%s%03d",         0,   1 },
    { "%s-journal%03d", SQLITE_MULTIPLEX_JOURNAL_8_3_ಸ್ಥಾನಾಂತರ, 1 },
    { "%s-wal%03d",     SQLITE_MULTIPLEX_WAL_8_3_ಸ್ಥಾನಾಂತರ, 1 },
  };

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OS_WIN
  sqlite3_vfs *pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ("win32");
#ಇಲ್ಲದಿದ್ದರೆ
  sqlite3_vfs *pVfs = 0;
#ಆದರೆ_ಅಂತ್ಯ

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zFile) + 100;
  zBuf = (ಅಕ್ಷರ*)sqlite3_malloc(nBuf);
  ಆದರೆ( zBuf==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  ಗಾಗಿ(i=0; rc==0 && i<ಗಾತ್ರ(azFmt)/ಗಾತ್ರ(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    ಆದರೆ( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  ಗಾಗಿ(i=0; rc==0 && i<ಗಾತ್ರ(aMFile)/ಗಾತ್ರ(aMFile[0]); i++){
    ರಚನೆ MFile *p = &aMFile[i];
    ಪೂರ್ಣಾಂಕ iChunk;
    ಗಾಗಿ(iChunk=1; iChunk<=MX_CHUNK_ಸಂಖ್ಯೆ; iChunk++){
      ಪೂರ್ಣಾಂಕ bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->i_ಸ್ಥಾನಾಂತರ);
      ಆದರೆ( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      ಆದರೆ( bExists==0 || rc!=0 ) ನಿಲ್ಲಿಸು;
    }
  }

  sqlite3_free(zBuf);
  ಹಿಂತಿರುಗಿ (rc ? SQLITE_ERROR : SQLITE_OK);
}
