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
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <assert.h>
#অন্তর্ভাব "sqlite3.h"

/* The following #defines are copied from test_multiplex.c */
#ifndef MX_CHUNK_সংখ্যা 
# define MX_CHUNK_সংখ্যা 299
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
স্থির শূন্য sqlite3Delete83Name(অক্ষর *z){
  পূর্ণসংখ্যা i, sz;
  sz = (পূর্ণসংখ্যা)strlen(z);
  জন্য(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  যদি( z[i]=='.' && (sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
স্থির পূর্ণসংখ্যা sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  ধ্রুবক অক্ষর *zFile, 
  পূর্ণসংখ্যা *pbExists
){
  পূর্ণসংখ্যা rc = SQLITE_ERROR;
#ifdef SQLITE_OS_WIN
  যদি( pVfs ){
    যদি( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    যদি( rc==SQLITE_IOERR_DELETE_NOENT ){
      যদি( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#অন্যথা
  প্রতিজ্ঞা( pVfs==0 );
  rc = access(zFile, F_OK);
  যদি( rc ){
    যদি( errno==ENOENT ){ 
      যদি( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }অন্যথা{
    যদি( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#endif
  ফেরত rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API পূর্ণসংখ্যা sqlite3_delete_database(
  ধ্রুবক অক্ষর *zFile               /* File to delete */
){
  অক্ষর *zBuf;                     /* Buffer to sprintf() filenames to */
  পূর্ণসংখ্যা nBuf;                       /* Size of buffer in bytes */
  পূর্ণসংখ্যা rc = 0;                     /* System error code */
  পূর্ণসংখ্যা i;                          /* Iterate through azFmt[] and aMFile[] */

  ধ্রুবক অক্ষর *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  কাঠামো MFile {
    ধ্রুবক অক্ষর *zFmt;
    পূর্ণসংখ্যা iOffset;
    পূর্ণসংখ্যা b83;
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
#অন্যথা
  sqlite3_vfs *pVfs = 0;
#endif

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (পূর্ণসংখ্যা)strlen(zFile) + 100;
  zBuf = (অক্ষর*)sqlite3_malloc(nBuf);
  যদি( zBuf==0 ) ফেরত SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  জন্য(i=0; rc==0 && i<আকার(azFmt)/আকার(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    যদি( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  জন্য(i=0; rc==0 && i<আকার(aMFile)/আকার(aMFile[0]); i++){
    কাঠামো MFile *p = &aMFile[i];
    পূর্ণসংখ্যা iChunk;
    জন্য(iChunk=1; iChunk<=MX_CHUNK_সংখ্যা; iChunk++){
      পূর্ণসংখ্যা bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->iOffset);
      যদি( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      যদি( bExists==0 || rc!=0 ) বিরতি;
    }
  }

  sqlite3_free(zBuf);
  ফেরত (rc ? SQLITE_ERROR : SQLITE_OK);
}
