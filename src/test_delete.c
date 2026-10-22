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
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <assert.h>
#અન્તર્ભાવ "sqlite3.h"

/* The following #defines are copied from test_multiplex.c */
#ifndef MX_CHUNK_સંખ્યા 
# define MX_CHUNK_સંખ્યા 299
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
સ્થિર શૂન્ય sqlite3Delete83Name(અક્ષર *z){
  પૂર્ણાંક i, sz;
  sz = (પૂર્ણાંક)strlen(z);
  માટે(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  જો( z[i]=='.' && (sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
સ્થિર પૂર્ણાંક sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  અચલ અક્ષર *zFile, 
  પૂર્ણાંક *pbExists
){
  પૂર્ણાંક rc = SQLITE_ERROR;
#ifdef SQLITE_OS_WIN
  જો( pVfs ){
    જો( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    જો( rc==SQLITE_IOERR_DELETE_NOENT ){
      જો( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#નહીંતર
  પ્રતિજ્ઞા( pVfs==0 );
  rc = access(zFile, F_OK);
  જો( rc ){
    જો( errno==ENOENT ){ 
      જો( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }નહીંતર{
    જો( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#endif
  પાછા rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API પૂર્ણાંક sqlite3_delete_database(
  અચલ અક્ષર *zFile               /* File to delete */
){
  અક્ષર *zBuf;                     /* Buffer to sprintf() filenames to */
  પૂર્ણાંક nBuf;                       /* Size of buffer in bytes */
  પૂર્ણાંક rc = 0;                     /* System error code */
  પૂર્ણાંક i;                          /* Iterate through azFmt[] and aMFile[] */

  અચલ અક્ષર *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  સંરચના MFile {
    અચલ અક્ષર *zFmt;
    પૂર્ણાંક iOffset;
    પૂર્ણાંક b83;
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
#નહીંતર
  sqlite3_vfs *pVfs = 0;
#endif

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (પૂર્ણાંક)strlen(zFile) + 100;
  zBuf = (અક્ષર*)sqlite3_malloc(nBuf);
  જો( zBuf==0 ) પાછા SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  માટે(i=0; rc==0 && i<કદ(azFmt)/કદ(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    જો( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  માટે(i=0; rc==0 && i<કદ(aMFile)/કદ(aMFile[0]); i++){
    સંરચના MFile *p = &aMFile[i];
    પૂર્ણાંક iChunk;
    માટે(iChunk=1; iChunk<=MX_CHUNK_સંખ્યા; iChunk++){
      પૂર્ણાંક bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->iOffset);
      જો( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      જો( bExists==0 || rc!=0 ) તોડો;
    }
  }

  sqlite3_free(zBuf);
  પાછા (rc ? SQLITE_ERROR : SQLITE_OK);
}
