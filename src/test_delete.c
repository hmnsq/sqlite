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
static void sqlite3Delete83Name(var_na *z){
  පූර්ණ_සංඛ්‍යා i, sz;
  sz = (පූර්ණ_සංඛ්‍යා)strlen(z);
  සඳහා(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  නම්( z[i]=='.' && (sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
static පූර්ණ_සංඛ්‍යා sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  const var_na *zFile, 
  පූර්ණ_සංඛ්‍යා *pbExists
){
  පූර්ණ_සංඛ්‍යා rc = SQLITE_ERROR;
#ifdef SQLITE_OS_WIN
  නම්( pVfs ){
    නම්( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    නම්( rc==SQLITE_IOERR_DELETE_NOENT ){
      නම්( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#නැතහොත්
  assert( pVfs==0 );
  rc = access(zFile, F_OK);
  නම්( rc ){
    නම්( errno==ENOENT ){ 
      නම්( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }නැතහොත්{
    නම්( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#endif
  ආපසු rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API පූර්ණ_සංඛ්‍යා sqlite3_delete_database(
  const var_na *zFile               /* File to delete */
){
  var_na *zBuf;                     /* Buffer to sprintf() filenames to */
  පූර්ණ_සංඛ්‍යා nBuf;                       /* Size of buffer in bytes */
  පූර්ණ_සංඛ්‍යා rc = 0;                     /* System error code */
  පූර්ණ_සංඛ්‍යා i;                          /* Iterate through azFmt[] and aMFile[] */

  const var_na *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  sa_nracanaa MFile {
    const var_na *zFmt;
    පූර්ණ_සංඛ්‍යා iOffset;
    පූර්ණ_සංඛ්‍යා b83;
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
#නැතහොත්
  sqlite3_vfs *pVfs = 0;
#endif

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (පූර්ණ_සංඛ්‍යා)strlen(zFile) + 100;
  zBuf = (var_na*)sqlite3_malloc(nBuf);
  නම්( zBuf==0 ) ආපසු SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  සඳහා(i=0; rc==0 && i<parimaa_na(azFmt)/parimaa_na(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    නම්( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  සඳහා(i=0; rc==0 && i<parimaa_na(aMFile)/parimaa_na(aMFile[0]); i++){
    sa_nracanaa MFile *p = &aMFile[i];
    පූර්ණ_සංඛ්‍යා iChunk;
    සඳහා(iChunk=1; iChunk<=MX_CHUNK_NUMBER; iChunk++){
      පූර්ණ_සංඛ්‍යා bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->iOffset);
      නම්( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      නම්( bExists==0 || rc!=0 ) bha_nga;
    }
  }

  sqlite3_free(zBuf);
  ආපසු (rc ? SQLITE_ERROR : SQLITE_OK);
}
