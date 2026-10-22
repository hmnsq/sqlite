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
  ကိန်းပြည့် i, sz;
  sz = (ကိန်းပြည့်)strlen(z);
  အတွက်(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  အကယ်၍( z[i]=='.' && (sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
static ကိန်းပြည့် sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  const var_na *zFile, 
  ကိန်းပြည့် *pbExists
){
  ကိန်းပြည့် rc = SQLITE_ERROR;
#ifdef SQLITE_OS_WIN
  အကယ်၍( pVfs ){
    အကယ်၍( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    အကယ်၍( rc==SQLITE_IOERR_DELETE_NOENT ){
      အကယ်၍( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#မဟုတ်ပါက
  assert( pVfs==0 );
  rc = access(zFile, F_OK);
  အကယ်၍( rc ){
    အကယ်၍( errno==ENOENT ){ 
      အကယ်၍( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }မဟုတ်ပါက{
    အကယ်၍( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#endif
  ပြန်ပေး rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API ကိန်းပြည့် sqlite3_delete_database(
  const var_na *zFile               /* File to delete */
){
  var_na *zBuf;                     /* Buffer to sprintf() filenames to */
  ကိန်းပြည့် nBuf;                       /* Size of buffer in bytes */
  ကိန်းပြည့် rc = 0;                     /* System error code */
  ကိန်းပြည့် i;                          /* Iterate through azFmt[] and aMFile[] */

  const var_na *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  sa_nracanaa MFile {
    const var_na *zFmt;
    ကိန်းပြည့် iOffset;
    ကိန်းပြည့် b83;
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
#မဟုတ်ပါက
  sqlite3_vfs *pVfs = 0;
#endif

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (ကိန်းပြည့်)strlen(zFile) + 100;
  zBuf = (var_na*)sqlite3_malloc(nBuf);
  အကယ်၍( zBuf==0 ) ပြန်ပေး SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  အတွက်(i=0; rc==0 && i<parimaa_na(azFmt)/parimaa_na(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    အကယ်၍( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  အတွက်(i=0; rc==0 && i<parimaa_na(aMFile)/parimaa_na(aMFile[0]); i++){
    sa_nracanaa MFile *p = &aMFile[i];
    ကိန်းပြည့် iChunk;
    အတွက်(iChunk=1; iChunk<=MX_CHUNK_NUMBER; iChunk++){
      ကိန်းပြည့် bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->iOffset);
      အကယ်၍( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      အကယ်၍( bExists==0 || rc!=0 ) bha_nga;
    }
  }

  sqlite3_free(zBuf);
  ပြန်ပေး (rc ? SQLITE_ERROR : SQLITE_OK);
}
