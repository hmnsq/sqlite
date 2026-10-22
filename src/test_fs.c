/*
** 2013 Jan 11
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Code for testing the virtual table interfaces.  This code
** is not included in the SQLite library.  It is used for automated
** testing of the SQLite library.
**
** The FS virtual table is created as follows:
**
**   CREATE VIRTUAL TABLE tbl USING fs(idx);
**
** where idx is the name of a table in the db with 2 columns.  The virtual
** table also has two columns - file path and file contents.
**
** The first column of table idx must be an IPK, and the second contains file
** paths. For example:
**
**   CREATE TABLE idx(id INTEGER PRIMARY KEY, path TEXT);
**   INSERT INTO idx VALUES(4, '/etc/passwd');
**
** Adding the row to the idx table automatically creates a row in the 
** virtual table with rowid=4, path=/etc/passwd and a text field that 
** contains data read from file /etc/passwd on disk.
**
*************************************************************************
** Virtual table module "fsdir"
**
** This module is designed to be used as a read-only eponymous virtual table.
** Its schema is as follows:
**
**   CREATE TABLE fsdir(dir TEXT, name TEXT);
**
** When queried, a WHERE term of the form "dir = $dir" must be provided. The
** virtual table then appears to have one row for each entry in file-system
** directory $dir. Column dir contains a copy of $dir, and column "name"
** contains the name of the directory entry.
**
** If the specified $dir cannot be opened or is not a directory, it is not
** an error. The virtual table appears to be empty in this case.
**
*************************************************************************
** Virtual table module "fstree"
**
** This module is also a read-only eponymous virtual table with the 
** following schema:
**
**   CREATE TABLE fstree(path TEXT, size INT, data BLOB);
**
** Running a "SELECT * FROM fstree" query on this table returns the entire
** contents of the file-system, starting at "/". To restrict the search
** space, the virtual table supports LIKE and GLOB constraints on the
** 'path' column. For example:
**
**   SELECT * FROM fstree WHERE path LIKE '/home/dan/sqlite/%'
*/
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>

#ಯದಿ !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#ಅನ್ಯಥಾ
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
# endif
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ fs_vtab fs_vtab;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
ಸಂರಚನಾ fs_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
  ವರ್ಣ *zDb;                      /* Name of db containing zTbl */
  ವರ್ಣ *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
ಸಂರಚನಾ fs_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
  ವರ್ಣ *zBuf;
  ಪೂರ್ಣಾಙ್ಕ nBuf;
  ಪೂರ್ಣಾಙ್ಕ nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ FsdirVtab FsdirVtab;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ FsdirCsr FsdirCsr;
ಸಂರಚನಾ FsdirVtab {
  sqlite3_vtab base;
};

ಸಂರಚನಾ FsdirCsr {
  sqlite3_vtab_cursor base;
  ವರ್ಣ *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  ಸಂರಚನಾ dirent *pEntry;
};

/*
** This function is the implementation of both the xConnect and xCreate
** methods of the fsdir virtual table.
**
** The argv[] array contains the following:
**
**   argv[0]   -> module name  ("fs")
**   argv[1]   -> database name
**   argv[2]   -> table name
**   argv[...] -> other module argument fields.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsdirConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  FsdirVtab *pTab;

  ಯದಿ( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(ಪರಿಮಾಣ(FsdirVtab));
  ಯದಿ( !pTab ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  memset(pTab, 0, ಪರಿಮಾಣ(FsdirVtab));

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsdirBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ಪೂರ್ಣಾಙ್ಕ ii;

  pIdxInfo->estimatedCost = 1000000000.0;

  ಪ್ರತಿ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ಸಂರಚನಾ sqlite3_index_constraint ಧ್ರುವ *p = &pIdxInfo->aConstraint[ii];
    ಯದಿ( p->iColumn==0 && p->usable && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      ಸಂರಚನಾ sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_ಅಂಕೆ = 1;
      pIdxInfo->estimatedCost = 1.0;
      ಭಙ್ಗ;
    }
  }

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(ಪರಿಮಾಣ(FsdirCsr)+256);
  ಯದಿ( pCur==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  memset(pCur, 0, ಪರಿಮಾಣ(FsdirCsr));
  *ppCursor = &pCur->base;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  ಯದಿ( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsdirNext(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  ಯದಿ( pCsr->pDir ){
    pCsr->pEntry = readdir(pCsr->pDir);
    ಯದಿ( pCsr->pEntry==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** xFilter method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಙ್ಕ idx_ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *idxStr,
  ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  ಧ್ರುವ ವರ್ಣ *zDir;
  ಪೂರ್ಣಾಙ್ಕ nDir;


  ಯದಿ( idx_ಅಂಕೆ!=1 || argc!=1 ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  ಯದಿ( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
  nDir = sqlite3_ಮೌಲ್ಯ_bytes(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  ಯದಿ( pCsr->zDir==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  memcpy(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  ಪ್ರತ್ಯಾಗಮ fsdirNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  ಪ್ರತ್ಯಾಗಮ pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsdirColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ಪೂರ್ಣಾಙ್ಕ i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  ವಿಕಲ್ಪ( i ){
    ಪಕ್ಷ 0: /* dir */
      sqlite3_result_text(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      ಭಙ್ಗ;

    ಪಕ್ಷ 1: /* name */
      sqlite3_result_text(ctx, pCsr->pEntry->d_name, -1, SQLITE_TRANSIENT);
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      ಪ್ರತಿಜ್ಞಾ( 0 );
  }

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** xRowid method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ FstreeVtab FstreeVtab;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ FstreeCsr FstreeCsr;
ಸಂರಚನಾ FstreeVtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

ಸಂರಚನಾ FstreeCsr {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  ಪೂರ್ಣಾಙ್ಕ fd;                         /* File descriptor open on current path */
};

/*
** This function is the implementation of both the xConnect and xCreate
** methods of the fstree virtual table.
**
** The argv[] array contains the following:
**
**   argv[0]   -> module name  ("fs")
**   argv[1]   -> database name
**   argv[2]   -> table name
**   argv[...] -> other module argument fields.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fstreeConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  FstreeVtab *pTab;

  ಯದಿ( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(ಪರಿಮಾಣ(FstreeVtab));
  ಯದಿ( !pTab ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  memset(pTab, 0, ಪರಿಮಾಣ(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fstreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ಪೂರ್ಣಾಙ್ಕ ii;

  ಪ್ರತಿ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ಸಂರಚನಾ sqlite3_index_constraint ಧ್ರುವ *p = &pIdxInfo->aConstraint[ii];
    ಯದಿ( p->iColumn==0 && p->usable && (
          p->op==SQLITE_INDEX_CONSTRAINT_GLOB
       || p->op==SQLITE_INDEX_CONSTRAINT_LIKE
       || p->op==SQLITE_INDEX_CONSTRAINT_EQ
    )){
      ಸಂರಚನಾ sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_ಅಂಕೆ = p->op;
      pUsage->argvIndex = 1;
      pIdxInfo->estimatedCost = 100000.0;
      ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
    }
  }

  pIdxInfo->estimatedCost = 1000000000.0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(ಪರಿಮಾಣ(FstreeCsr));
  ಯದಿ( pCur==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  memset(pCur, 0, ಪರಿಮಾಣ(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->base;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ fstreeCloseFd(FstreeCsr *pCsr){
  ಯದಿ( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fstreeNext(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ಪೂರ್ಣಾಙ್ಕ rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_step(pCsr->pStmt);
  ಯದಿ( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }ಅನ್ಯಥಾ{
    rc = SQLITE_OK;
    pCsr->fd = open((ಧ್ರುವ ವರ್ಣ*)sqlite3_column_text(pCsr->pStmt, 0), O_RDONLY);
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** xFilter method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಙ್ಕ idx_ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *idxStr,
  ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->base.pVtab);
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಧ್ರುವ ವರ್ಣ *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  ವರ್ಣ *z_ಬೇರು;
  ಪೂರ್ಣಾಙ್ಕ n_ಬೇರು;
  ವರ್ಣ *zPrefix;
  ಪೂರ್ಣಾಙ್ಕ nPrefix;
  ಧ್ರುವ ವರ್ಣ *zDir;
  ಪೂರ್ಣಾಙ್ಕ nDir;
  ವರ್ಣ aWild[2] = { '\0', '\0' };

#ifdef _WIN32
  ಧ್ರುವ ವರ್ಣ *zDrive = getenv("fstreeDrive");
  ಯದಿ( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  z_ಬೇರು = sqlite3_mprintf("%s%c", zDrive, '/');
  n_ಬೇರು = sqlite3Strlen30(z_ಬೇರು);
  zPrefix = sqlite3_mprintf("%s", zDrive);
  nPrefix = sqlite3Strlen30(zPrefix);
#ಅನ್ಯಥಾ
  z_ಬೇರು = "/";
  n_ಬೇರು = 1;
  zPrefix = "";
  nPrefix = 0;
#endif

  zDir = z_ಬೇರು;
  nDir = n_ಬೇರು;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  ಯದಿ( rc!=SQLITE_OK ) ಪ್ರತ್ಯಾಗಮ rc;

  ಯದಿ( idx_ಅಂಕೆ ){
    ಧ್ರುವ ವರ್ಣ *zQuery = (ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(argv[0]);
    ವಿಕಲ್ಪ( idx_ಅಂಕೆ ){
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_INDEX_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        ಭಙ್ಗ;
    }

    ಯದಿ( sqlite3_strnicmp(zQuery, zPrefix, nPrefix)==0 ){
      ಪೂರ್ಣಾಙ್ಕ i;
      ಪ್ರತಿ(i=nPrefix; zQuery[i]; i++){
        ಯದಿ( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) ಭಙ್ಗ;
        ಯದಿ( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  ಯದಿ( nDir==0 ) nDir = 1;

  sqlite3_bind_text(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 2, z_ಬೇರು, n_ಬೇರು, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 3, zPrefix, nPrefix, SQLITE_TRANSIENT);

#ifdef _WIN32
  sqlite3_free(zPrefix);
  sqlite3_free(z_ಬೇರು);
#endif

  ಪ್ರತ್ಯಾಗಮ fstreeNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ಪ್ರತ್ಯಾಗಮ pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fstreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ಪೂರ್ಣಾಙ್ಕ i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ಯದಿ( i==0 ){      /* path */
    sqlite3_result_ಮೌಲ್ಯ(ctx, sqlite3_column_ಮೌಲ್ಯ(pCsr->pStmt, 0));
  }ಅನ್ಯಥಾ{
    ಸಂರಚನಾ stat sBuf;
    fstat(pCsr->fd, &sBuf);

    ಯದಿ( S_ISREG(sBuf.st_mode) ){
      ಯದಿ( i==1 ){
        sqlite3_result_int64(ctx, sBuf.st_size);
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ nRead;
        ವರ್ಣ *aBuf = sqlite3_malloc(sBuf.st_mode+1);
        ಯದಿ( !aBuf ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
        nRead = ಪಠ(pCsr->fd, aBuf, sBuf.st_mode);
        ಯದಿ( nRead!=sBuf.st_mode ){
          ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR;
        }
        sqlite3_result_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** xRowid method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
/*
** End of fstree implementation.
*************************************************************************/




/*
** This function is the implementation of both the xConnect and xCreate
** methods of the fs virtual table.
**
** The argv[] array contains the following:
**
**   argv[0]   -> module name  ("fs")
**   argv[1]   -> database name
**   argv[2]   -> table name
**   argv[...] -> other module argument fields.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಙ್ಕ argc, ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ*argv,
  sqlite3_vtab **ppVtab,
  ವರ್ಣ **pzErr
){
  fs_vtab *pVtab;
  ಪೂರ್ಣಾಙ್ಕ nByte;
  ಧ್ರುವ ವರ್ಣ *zTbl;
  ಧ್ರುವ ವರ್ಣ *zDb = argv[1];

  ಯದಿ( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = ಪರಿಮಾಣ(fs_vtab) + (ಪೂರ್ಣಾಙ್ಕ)strlen(zTbl) + 1 + (ಪೂರ್ಣಾಙ್ಕ)strlen(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  ಯದಿ( !pVtab ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;

  pVtab->zTbl = (ವರ್ಣ *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[strlen(zTbl)+1];
  pVtab->db = db;
  memcpy(pVtab->zTbl, zTbl, strlen(zTbl));
  memcpy(pVtab->zDb, zDb, strlen(zDb));
  *ppVtab = &pVtab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(ಪರಿಮಾಣ(fs_cursor));
  *ppCursor = &pCur->base;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Close a fs cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsNext(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  ಪೂರ್ಣಾಙ್ಕ rc;

  rc = sqlite3_step(pCur->pStmt);
  ಯದಿ( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;

  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಙ್ಕ idx_ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *idxStr,
  ಪೂರ್ಣಾಙ್ಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  ಪ್ರತಿಜ್ಞಾ( (idx_ಅಂಕೆ==0 && argc==0) || (idx_ಅಂಕೆ==1 && argc==1) );
  ಯದಿ( idx_ಅಂಕೆ==1 ){
    ವರ್ಣ *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    ಯದಿ( !zStmt ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    ಯದಿ( rc==SQLITE_OK ){
      sqlite3_bind_ಮೌಲ್ಯ(pCur->pStmt, 1, argv[0]);
    }
  }ಅನ್ಯಥಾ{
    ವರ್ಣ *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    ಯದಿ( !zStmt ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  ಯದಿ( rc==SQLITE_OK ){
    rc = fsNext(pVtabCursor); 
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ಪೂರ್ಣಾಙ್ಕ i){
  fs_cursor *pCur = (fs_cursor*)cur;

  ಪ್ರತಿಜ್ಞಾ( i==0 || i==1 || i==2 );
  ಯದಿ( i==0 ){
    sqlite3_result_ಮೌಲ್ಯ(ctx, sqlite3_column_ಮೌಲ್ಯ(pCur->pStmt, 0));
  }ಅನ್ಯಥಾ{
    ಧ್ರುವ ವರ್ಣ *zFile = (ಧ್ರುವ ವರ್ಣ *)sqlite3_column_text(pCur->pStmt, 1);
    ಸಂರಚನಾ stat sbuf;
    ಪೂರ್ಣಾಙ್ಕ fd;

    ಪೂರ್ಣಾಙ್ಕ n;
    fd = open(zFile, O_RDONLY);
    ಯದಿ( fd<0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR;
    fstat(fd, &sbuf);

    ಯದಿ( sbuf.st_size>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_size*2;
      ವರ್ಣ *zNew;
      ಯದಿ( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      ಯದಿ( zNew==0 ){
        close(fd);
        ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (ಪೂರ್ಣಾಙ್ಕ)ಪಠ(fd, pCur->zBuf, sbuf.st_size);
    close(fd);
    ಯದಿ( n!=sbuf.st_size ) ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
    pCur->nBuf = sbuf.st_size;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_result_text(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_column_int64(pCur->pStmt, 0);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  ಪ್ರತ್ಯಾಗಮ (sqlite3_data_ಎಣಿಕೆ(pCur->pStmt)==0);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fsBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ಪೂರ್ಣಾಙ್ಕ ii;

  ಪ್ರತಿ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ಸಂರಚನಾ sqlite3_index_constraint ಧ್ರುವ *pCons = &pIdxInfo->aConstraint[ii];
    ಯದಿ( pCons->iColumn<0 && pCons->usable
           && pCons->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      ಸಂರಚನಾ sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_ಅಂಕೆ = 1;
      pIdxInfo->estimatedCost = 1.0;
      ಭಙ್ಗ;
    }
  }

  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ಸ್ಥಿರ sqlite3_module fsModule = {
  0,                         /* iVersion */
  fsConnect,
  fsConnect,
  fsBestIndex,
  fsDisconnect, 
  fsDisconnect,
  fsOpen,                      /* xOpen - open a cursor */
  fsClose,                     /* xClose - close a cursor */
  fsFilter,                    /* xFilter - configure scan constraints */
  fsNext,                      /* xNext - advance a cursor */
  fsEof,                       /* xEof - check for end of scan */
  fsColumn,                    /* xColumn - read data */
  fsRowid,                     /* xRowid - read data */
  0,                           /* xUpdate */
  0,                           /* xBegin */
  0,                           /* xSync */
  0,                           /* xCommit */
  0,                           /* xRollback */
  0,                           /* xFindMethod */
  0,                           /* xRename */
  0,                           /* xSavepoint */
  0,                           /* xRelease */
  0,                           /* xRollbackTo */
  0,                           /* xShadowName */
  0                            /* xIntegrity */
};

ಸ್ಥಿರ sqlite3_module fsdirModule = {
  0,                              /* iVersion */
  fsdirConnect,                   /* xCreate */
  fsdirConnect,                   /* xConnect */
  fsdirBestIndex,                 /* xBestIndex */
  fsdirDisconnect,                /* xDisconnect */
  fsdirDisconnect,                /* xDestroy */
  fsdirOpen,                      /* xOpen - open a cursor */
  fsdirClose,                     /* xClose - close a cursor */
  fsdirFilter,                    /* xFilter - configure scan constraints */
  fsdirNext,                      /* xNext - advance a cursor */
  fsdirEof,                       /* xEof - check for end of scan */
  fsdirColumn,                    /* xColumn - read data */
  fsdirRowid,                     /* xRowid - read data */
  0,                              /* xUpdate */
  0,                              /* xBegin */
  0,                              /* xSync */
  0,                              /* xCommit */
  0,                              /* xRollback */
  0,                              /* xFindMethod */
  0,                              /* xRename */
  0,                              /* xSavepoint */
  0,                              /* xRelease */
  0,                              /* xRollbackTo */
  0,                              /* xShadowName */
  0                               /* xIntegrity */
};

ಸ್ಥಿರ sqlite3_module fstreeModule = {
  0,                              /* iVersion */
  fstreeConnect,                  /* xCreate */
  fstreeConnect,                  /* xConnect */
  fstreeBestIndex,                /* xBestIndex */
  fstreeDisconnect,               /* xDisconnect */
  fstreeDisconnect,               /* xDestroy */
  fstreeOpen,                     /* xOpen - open a cursor */
  fstreeClose,                    /* xClose - close a cursor */
  fstreeFilter,                   /* xFilter - configure scan constraints */
  fstreeNext,                     /* xNext - advance a cursor */
  fstreeEof,                      /* xEof - check for end of scan */
  fstreeColumn,                   /* xColumn - read data */
  fstreeRowid,                    /* xRowid - read data */
  0,                              /* xUpdate */
  0,                              /* xBegin */
  0,                              /* xSync */
  0,                              /* xCommit */
  0,                              /* xRollback */
  0,                              /* xFindMethod */
  0,                              /* xRename */
  0,                              /* xSavepoint */
  0,                              /* xRelease */
  0,                              /* xRollbackTo */
  0,                              /* xShadowName */
  0                               /* xIntegrity */
};

/*
** Decode a pointer to an sqlite3 object.
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp *interp, ಧ್ರುವ ವರ್ಣ *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI register_fs_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಙ್ಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (ಶೂನ್ಯ *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಙ್ಕ Sqlitetestfs_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_ObjCmdProc *xProc;
     ಶೂನ್ಯ *clientData;
  } aObjCmd[] = {
     { "register_fs_module",   register_fs_module, 0 },
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aObjCmd)/ಪರಿಮಾಣ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
