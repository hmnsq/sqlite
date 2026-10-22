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
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>
#അന്തര്ഭാവ "sqlite3.h"

/* The following #defines are copied from test_multiplex.c */
#ifndef MX_CHUNK_സംഖ്യ 
# define MX_CHUNK_സംഖ്യ 299
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
സ്ഥിതം ശൂന്യം sqlite3Delete83Name(അക്ഷരം *z){
  പൂർണ്ണസംഖ്യ i, sz;
  sz = (പൂർണ്ണസംഖ്യ)strlen(z);
  വേണ്ടി(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  എങ്കിൽ( z[i]=='.' && (sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  സ്ഥിരം അക്ഷരം *zFile, 
  പൂർണ്ണസംഖ്യ *pbExists
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_ERROR;
#ifdef SQLITE_OS_WIN
  എങ്കിൽ( pVfs ){
    എങ്കിൽ( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    എങ്കിൽ( rc==SQLITE_IOERR_DELETE_NOENT ){
      എങ്കിൽ( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#അല്ലെങ്കിൽ
  പ്രതിജ്ഞാ( pVfs==0 );
  rc = access(zFile, F_OK);
  എങ്കിൽ( rc ){
    എങ്കിൽ( errno==ENOENT ){ 
      എങ്കിൽ( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }അല്ലെങ്കിൽ{
    എങ്കിൽ( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#endif
  തിരികെ rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API പൂർണ്ണസംഖ്യ sqlite3_delete_database(
  സ്ഥിരം അക്ഷരം *zFile               /* File to delete */
){
  അക്ഷരം *zBuf;                     /* Buffer to sprintf() filenames to */
  പൂർണ്ണസംഖ്യ nBuf;                       /* Size of buffer in bytes */
  പൂർണ്ണസംഖ്യ rc = 0;                     /* System error code */
  പൂർണ്ണസംഖ്യ i;                          /* Iterate through azFmt[] and aMFile[] */

  സ്ഥിരം അക്ഷരം *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  ഘടന MFile {
    സ്ഥിരം അക്ഷരം *zFmt;
    പൂർണ്ണസംഖ്യ iOffset;
    പൂർണ്ണസംഖ്യ b83;
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
#അല്ലെങ്കിൽ
  sqlite3_vfs *pVfs = 0;
#endif

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (പൂർണ്ണസംഖ്യ)strlen(zFile) + 100;
  zBuf = (അക്ഷരം*)sqlite3_malloc(nBuf);
  എങ്കിൽ( zBuf==0 ) തിരികെ SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  വേണ്ടി(i=0; rc==0 && i<വലുപ്പം(azFmt)/വലുപ്പം(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    എങ്കിൽ( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  വേണ്ടി(i=0; rc==0 && i<വലുപ്പം(aMFile)/വലുപ്പം(aMFile[0]); i++){
    ഘടന MFile *p = &aMFile[i];
    പൂർണ്ണസംഖ്യ iChunk;
    വേണ്ടി(iChunk=1; iChunk<=MX_CHUNK_സംഖ്യ; iChunk++){
      പൂർണ്ണസംഖ്യ bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->iOffset);
      എങ്കിൽ( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      എങ്കിൽ( bExists==0 || rc!=0 ) നിർത്തുക;
    }
  }

  sqlite3_free(zBuf);
  തിരികെ (rc ? SQLITE_ERROR : SQLITE_OK);
}
