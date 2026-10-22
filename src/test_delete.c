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

#अगर_अघोषित SQLITE_OS_WIN
#  include <unistd.h>
#  include <errno.h>
#पूर्ण_अगर
#समावेश <string.h>
#समावेश <assert.h>
#समावेश "sqlite3.h"

/* The following #defines are copied from test_multiplex.c */
#अगर_अघोषित MX_CHUNK_संख्या 
# define MX_CHUNK_संख्या 299
#पूर्ण_अगर
#अगर_अघोषित SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET
# define SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET 400
#पूर्ण_अगर
#अगर_अघोषित SQLITE_MULTIPLEX_WAL_8_3_OFFSET
# define SQLITE_MULTIPLEX_WAL_8_3_OFFSET 700
#पूर्ण_अगर

/*
** This routine is a copy of (most of) the code from SQLite function
** sqlite3FileSuffix3(). It modifies the filename in buffer z in the
** same way as SQLite does when in 8.3 filenames mode.
*/
अटल व्योम sqlite3Delete83Name(अक्षर *z){
  पूर्णांक i, sz;
  sz = (पूर्णांक)म_माप(z);
  क्रम(i=sz-1; i>0 && z[i]!='/' && z[i]!='.'; i--){}
  अगर( z[i]=='.' && (sz>i+4) ) स_हटाओ(&z[i+1], &z[sz-3], 4);
}

/*
** zFile is a filename. Assuming no error occurs, if this file exists, 
** set *pbExists to true and unlink it. Or, if the file does not exist,
** set *pbExists to false before returning.
**
** If an error occurs, non-zero is returned. Or, if no error occurs, zero.
*/
अटल पूर्णांक sqlite3DeleteUnlinkIfExists(
  sqlite3_vfs *pVfs,
  स्थिर अक्षर *zFile, 
  पूर्णांक *pbExists
){
  पूर्णांक rc = SQLITE_ERROR;
#अगर_घोषित SQLITE_OS_WIN
  अगर( pVfs ){
    अगर( pbExists ) *pbExists = 1;
    rc = pVfs->xDelete(pVfs, zFile, 0);
    अगर( rc==SQLITE_IOERR_DELETE_NOENT ){
      अगर( pbExists ) *pbExists = 0;
      rc = SQLITE_OK;
    }
  }
#अन्यथा
  निश्चित( pVfs==0 );
  rc = access(zFile, F_OK);
  अगर( rc ){
    अगर( त्रुटि_सं==ENOENT ){ 
      अगर( pbExists ) *pbExists = 0;
      rc = SQLITE_OK; 
    }
  }अन्यथा{
    अगर( pbExists ) *pbExists = 1;
    rc = unlink(zFile);
  }
#पूर्ण_अगर
  वापस rc;
}

/*
** Delete the database file identified by the string argument passed to this
** function. The string must contain a filename, not an SQLite URI.
*/
SQLITE_API पूर्णांक sqlite3_delete_database(
  स्थिर अक्षर *zFile               /* File to delete */
){
  अक्षर *zBuf;                     /* Buffer to sprintf() filenames to */
  पूर्णांक nBuf;                       /* Size of buffer in bytes */
  पूर्णांक rc = 0;                     /* System error code */
  पूर्णांक i;                          /* Iterate through azFmt[] and aMFile[] */

  स्थिर अक्षर *azFmt[] = { "%s", "%s-journal", "%s-wal", "%s-shm" };

  काष्ठा MFile {
    स्थिर अक्षर *zFmt;
    पूर्णांक iOffset;
    पूर्णांक b83;
  } aMFile[] = {
    { "%s%03d",         0,   0 },
    { "%s-journal%03d", 0,   0 },
    { "%s-wal%03d",     0,   0 },
    { "%s%03d",         0,   1 },
    { "%s-journal%03d", SQLITE_MULTIPLEX_JOURNAL_8_3_OFFSET, 1 },
    { "%s-wal%03d",     SQLITE_MULTIPLEX_WAL_8_3_OFFSET, 1 },
  };

#अगर_घोषित SQLITE_OS_WIN
  sqlite3_vfs *pVfs = sqlite3_vfs_find("win32");
#अन्यथा
  sqlite3_vfs *pVfs = 0;
#पूर्ण_अगर

  /* Allocate a buffer large enough for any of the files that need to be
  ** deleted.  */
  nBuf = (पूर्णांक)म_माप(zFile) + 100;
  zBuf = (अक्षर*)sqlite3_malloc(nBuf);
  अगर( zBuf==0 ) वापस SQLITE_NOMEM;

  /* Delete both the regular and 8.3 filenames versions of the database,
  ** journal, wal and shm files.  */
  क्रम(i=0; rc==0 && i<माप(azFmt)/माप(azFmt[0]); i++){
    sqlite3_snprintf(nBuf, zBuf, azFmt[i], zFile);
    rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    अगर( rc==0 && i!=0 ){
      sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, 0);
    }
  }

  /* Delete any multiplexor files */
  क्रम(i=0; rc==0 && i<माप(aMFile)/माप(aMFile[0]); i++){
    काष्ठा MFile *p = &aMFile[i];
    पूर्णांक iChunk;
    क्रम(iChunk=1; iChunk<=MX_CHUNK_संख्या; iChunk++){
      पूर्णांक bExists;
      sqlite3_snprintf(nBuf, zBuf, p->zFmt, zFile, iChunk+p->iOffset);
      अगर( p->b83 ) sqlite3Delete83Name(zBuf);
      rc = sqlite3DeleteUnlinkIfExists(pVfs, zBuf, &bExists);
      अगर( bExists==0 || rc!=0 ) अवरोध;
    }
  }

  sqlite3_free(zBuf);
  वापस (rc ? SQLITE_ERROR : SQLITE_OK);
}
