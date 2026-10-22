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
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <assert.h>
#அந்தர்பாவ "sqlite3.h"

/* The following #defines are copied from test_multiplex.c */
#ifndef MX_CHUNK_எண் 
# define MX_CHUNK_எண் 299
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
நிலையான வெறுமை sqlite3Delete83Name(எழுத்து *z){
  முழு i, sz;
  sz = (முழு)strlen(z);
  க்கு(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  எனில்( z[i]=='.' && (sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
நிலையான முழு sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  மாறிலி எழுத்து *zFile, 
  முழு *pbExists
){
  முழு rc = SQLITE_ERROR;
#ifdef SQLITE_OS_WIN
  எனில்( pVfs ){
    எனில்( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    எனில்( rc==SQLITE_IOERR_DELETE_NOENT ){
      எனில்( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#இல்லையெனில்
  ப்ரதிஜ்ஞா( pVfs==0 );
  rc = access(zFile, F_OK);
  எனில்( rc ){
    எனில்( errno==ENOENT ){ 
      எனில்( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }இல்லையெனில்{
    எனில்( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#endif
  திரும்பு rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API முழு sqlite3_delete_database(
  மாறிலி எழுத்து *zFile               /* File to delete */
){
  எழுத்து *zBuf;                     /* Buffer to sprintf() filenames to */
  முழு nBuf;                       /* Size of buffer in bytes */
  முழு rc = 0;                     /* System error code */
  முழு i;                          /* Iterate through azFmt[] and aMFile[] */

  மாறிலி எழுத்து *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  கட்டமைப்பு MFile {
    மாறிலி எழுத்து *zFmt;
    முழு iOffset;
    முழு b83;
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
#இல்லையெனில்
  sqlite3_vfs *pVfs = 0;
#endif

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (முழு)strlen(zFile) + 100;
  zBuf = (எழுத்து*)sqlite3_malloc(nBuf);
  எனில்( zBuf==0 ) திரும்பு SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  க்கு(i=0; rc==0 && i<அளவு(azFmt)/அளவு(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    எனில்( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  க்கு(i=0; rc==0 && i<அளவு(aMFile)/அளவு(aMFile[0]); i++){
    கட்டமைப்பு MFile *p = &aMFile[i];
    முழு iChunk;
    க்கு(iChunk=1; iChunk<=MX_CHUNK_எண்; iChunk++){
      முழு bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->iOffset);
      எனில்( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      எனில்( bExists==0 || rc!=0 ) நிறுத்து;
    }
  }

  sqlite3_free(zBuf);
  திரும்பு (rc ? SQLITE_ERROR : SQLITE_OK);
}
