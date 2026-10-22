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
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>
#अन्तर्भाव "sqlite3.h"

/* The following #defines are copied from test_multiplex.c */
#ifndef MX_CHUNK_सङ्ख्या 
# define MX_CHUNK_सङ्ख्या 299
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
स्थिर शून्य sqlite3Delete83Name(वर्ण *z){
  पूर्णाङ्क i, sz;
  sz = (पूर्णाङ्क)strlen(z);
  अत्थाय(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  सचे( z[i]=='.' && (sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
स्थिर पूर्णाङ्क sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  ध्रुव वर्ण *zFile, 
  पूर्णाङ्क *pbExists
){
  पूर्णाङ्क rc = SQLITE_ERROR;
#ifdef SQLITE_OS_WIN
  सचे( pVfs ){
    सचे( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    सचे( rc==SQLITE_IOERR_DELETE_NOENT ){
      सचे( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#अञ्ञथा
  प्रतिज्ञा( pVfs==0 );
  rc = access(zFile, F_OK);
  सचे( rc ){
    सचे( errno==ENOENT ){ 
      सचे( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }अञ्ञथा{
    सचे( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#endif
  पटिनिवत्त rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API पूर्णाङ्क sqlite3_delete_database(
  ध्रुव वर्ण *zFile               /* File to delete */
){
  वर्ण *zBuf;                     /* Buffer to sprintf() filenames to */
  पूर्णाङ्क nBuf;                       /* Size of buffer in bytes */
  पूर्णाङ्क rc = 0;                     /* System error code */
  पूर्णाङ्क i;                          /* Iterate through azFmt[] and aMFile[] */

  ध्रुव वर्ण *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  संरचना MFile {
    ध्रुव वर्ण *zFmt;
    पूर्णाङ्क iOffset;
    पूर्णाङ्क b83;
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
#अञ्ञथा
  sqlite3_vfs *pVfs = 0;
#endif

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (पूर्णाङ्क)strlen(zFile) + 100;
  zBuf = (वर्ण*)sqlite3_malloc(nBuf);
  सचे( zBuf==0 ) पटिनिवत्त SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  अत्थाय(i=0; rc==0 && i<परिमाण(azFmt)/परिमाण(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    सचे( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  अत्थाय(i=0; rc==0 && i<परिमाण(aMFile)/परिमाण(aMFile[0]); i++){
    संरचना MFile *p = &aMFile[i];
    पूर्णाङ्क iChunk;
    अत्थाय(iChunk=1; iChunk<=MX_CHUNK_सङ्ख्या; iChunk++){
      पूर्णाङ्क bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->iOffset);
      सचे( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      सचे( bExists==0 || rc!=0 ) भङ्ग;
    }
  }

  sqlite3_free(zBuf);
  पटिनिवत्त (rc ? SQLITE_ERROR : SQLITE_OK);
}
