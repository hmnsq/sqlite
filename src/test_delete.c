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

#నిర్వచితం_కాకపోతే SQLITE_OS_WIN
#  include <unistd.h>
#  include <errno.h>
#అయితే_ముగింపు
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>
#అన్తర్భావ "sqlite3.h"

/* The following #defines are copied from test_multiplex.c */
#నిర్వచితం_కాకపోతే MX_CHUNK_సంఖ్య 
# define MX_CHUNK_సంఖ్య 299
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_MULTIPLEX_JOURNAL_8_3_స్థానభ్రంశం
# define SQLITE_MULTIPLEX_JOURNAL_8_3_స్థానభ్రంశం 400
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_MULTIPLEX_WAL_8_3_స్థానభ్రంశం
# define SQLITE_MULTIPLEX_WAL_8_3_స్థానభ్రంశం 700
#అయితే_ముగింపు

/*
** This routine is a copy of (most of) the code from SQLite function
** sqlite3FileSuffix3(). It modifies the filename in buffer z in the
** same way as SQLite does when in 8.3 filenames mode.
*/
స్థిర శూన్య sqlite3Delete83Name(అక్షరం *z){
  పూర్ణాంకం i, sz;
  sz = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(z);
  కొరకు(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  ఒకవేళ( z[i]=='.' && (sz>i+4) ) memmove(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
స్థిర పూర్ణాంకం sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  స్థిరరాశి అక్షరం *zFile, 
  పూర్ణాంకం *pbExists
){
  పూర్ణాంకం rc = SQLITE_ERROR;
#నిర్వచితమైతే SQLITE_OS_WIN
  ఒకవేళ( pVfs ){
    ఒకవేళ( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    ఒకవేళ( rc==SQLITE_IOERR_DELETE_NOENT ){
      ఒకవేళ( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#లేకపోతే
  ప్రతిజ్ఞా( pVfs==0 );
  rc = access(zFile, F_OK);
  ఒకవేళ( rc ){
    ఒకవేళ( errno==ENOENT ){ 
      ఒకవేళ( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }లేకపోతే{
    ఒకవేళ( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#అయితే_ముగింపు
  తిరిగి rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API పూర్ణాంకం sqlite3_delete_database(
  స్థిరరాశి అక్షరం *zFile               /* File to delete */
){
  అక్షరం *zBuf;                     /* Buffer to sprintf() filenames to */
  పూర్ణాంకం nBuf;                       /* Size of buffer in bytes */
  పూర్ణాంకం rc = 0;                     /* System error code */
  పూర్ణాంకం i;                          /* Iterate through azFmt[] and aMFile[] */

  స్థిరరాశి అక్షరం *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  నిర్మాణం MFile {
    స్థిరరాశి అక్షరం *zFmt;
    పూర్ణాంకం i_స్థానభ్రంశం;
    పూర్ణాంకం b83;
  } aMFile[] = {
    { "%s%03d",         0,   0 },
    { "%s-journal%03d", 0,   0 },
    { "%s-wal%03d",     0,   0 },
    { "%s%03d",         0,   1 },
    { "%s-journal%03d", SQLITE_MULTIPLEX_JOURNAL_8_3_స్థానభ్రంశం, 1 },
    { "%s-wal%03d",     SQLITE_MULTIPLEX_WAL_8_3_స్థానభ్రంశం, 1 },
  };

#నిర్వచితమైతే SQLITE_OS_WIN
  sqlite3_vfs *pVfs = sqlite3_vfs_కనుగొను("win32");
#లేకపోతే
  sqlite3_vfs *pVfs = 0;
#అయితే_ముగింపు

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zFile) + 100;
  zBuf = (అక్షరం*)sqlite3_malloc(nBuf);
  ఒకవేళ( zBuf==0 ) తిరిగి SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  కొరకు(i=0; rc==0 && i<పరిమాణం(azFmt)/పరిమాణం(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    ఒకవేళ( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  కొరకు(i=0; rc==0 && i<పరిమాణం(aMFile)/పరిమాణం(aMFile[0]); i++){
    నిర్మాణం MFile *p = &aMFile[i];
    పూర్ణాంకం iChunk;
    కొరకు(iChunk=1; iChunk<=MX_CHUNK_సంఖ్య; iChunk++){
      పూర్ణాంకం bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->i_స్థానభ్రంశం);
      ఒకవేళ( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      ఒకవేళ( bExists==0 || rc!=0 ) ఆపు;
    }
  }

  sqlite3_free(zBuf);
  తిరిగి (rc ? SQLITE_ERROR : SQLITE_OK);
}
