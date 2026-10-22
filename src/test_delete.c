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
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>
#ଅନ୍ତର୍ଭାଵ "sqlite3.h"

/* The following #defines are copied from test_multiplex.c */
#ifndef MX_CHUNK_ସଂଖ୍ୟା 
# define MX_CHUNK_ସଂଖ୍ୟା 299
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
ସ୍ଥିର ଶୂନ୍ୟ sqlite3Delete83Name(ଅକ୍ଷର *z){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, sz;
  sz = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(z);
  ପାଇଁ(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  ଯଦି( z[i]=='.' && (sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  ଅଚଳ ଅକ୍ଷର *zFile, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pbExists
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_ERROR;
#ifdef SQLITE_OS_WIN
  ଯଦି( pVfs ){
    ଯଦି( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    ଯଦି( rc==SQLITE_IOERR_DELETE_NOENT ){
      ଯଦି( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#ଅନ୍ୟଥା
  ପ୍ରତିଜ୍ଞା( pVfs==0 );
  rc = access(zFile, F_OK);
  ଯଦି( rc ){
    ଯଦି( errno==ENOENT ){ 
      ଯଦି( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }ଅନ୍ୟଥା{
    ଯଦି( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#endif
  ଫେରନ୍ତୁ rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_delete_database(
  ଅଚଳ ଅକ୍ଷର *zFile               /* File to delete */
){
  ଅକ୍ଷର *zBuf;                     /* Buffer to sprintf() filenames to */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf;                       /* Size of buffer in bytes */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;                     /* System error code */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                          /* Iterate through azFmt[] and aMFile[] */

  ଅଚଳ ଅକ୍ଷର *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  ସଂରଚନା MFile {
    ଅଚଳ ଅକ୍ଷର *zFmt;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOffset;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା b83;
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
#ଅନ୍ୟଥା
  sqlite3_vfs *pVfs = 0;
#endif

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zFile) + 100;
  zBuf = (ଅକ୍ଷର*)sqlite3_malloc(nBuf);
  ଯଦି( zBuf==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  ପାଇଁ(i=0; rc==0 && i<ଆକାର(azFmt)/ଆକାର(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    ଯଦି( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  ପାଇଁ(i=0; rc==0 && i<ଆକାର(aMFile)/ଆକାର(aMFile[0]); i++){
    ସଂରଚନା MFile *p = &aMFile[i];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iChunk;
    ପାଇଁ(iChunk=1; iChunk<=MX_CHUNK_ସଂଖ୍ୟା; iChunk++){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->iOffset);
      ଯଦି( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      ଯଦି( bExists==0 || rc!=0 ) ଭାଙ୍ଗ;
    }
  }

  sqlite3_free(zBuf);
  ଫେରନ୍ତୁ (rc ? SQLITE_ERROR : SQLITE_OK);
}
