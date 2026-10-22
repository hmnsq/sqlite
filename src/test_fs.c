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

#ಆದರೆ !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#ಇಲ್ಲದಿದ್ದರೆ
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(ಬಹುಲಕ) (((ಬಹುಲಕ) & S_IFMT) == S_IFREG)
# endif
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ fs_vtab fs_vtab;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
ರಚನೆ fs_vtab {
  sqlite3_vtab ತಳ;
  sqlite3 *db;
  ಅಕ್ಷರ *zDb;                      /* Name of db containing zTbl */
  ಅಕ್ಷರ *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
ರಚನೆ fs_cursor {
  sqlite3_vtab_cursor ತಳ;
  sqlite3_stmt *pStmt;
  ಅಕ್ಷರ *zBuf;
  ಪೂರ್ಣಾಂಕ nBuf;
  ಪೂರ್ಣಾಂಕ nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ FsdirVtab FsdirVtab;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ FsdirCsr FsdirCsr;
ರಚನೆ FsdirVtab {
  sqlite3_vtab ತಳ;
};

ರಚನೆ FsdirCsr {
  sqlite3_vtab_cursor ತಳ;
  ಅಕ್ಷರ *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  ರಚನೆ dirent *p_ನಮೂದು;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsdirConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  FsdirVtab *pTab;

  ಆದರೆ( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(ಗಾತ್ರ(FsdirVtab));
  ಆದರೆ( !pTab ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  memset(pTab, 0, ಗಾತ್ರ(FsdirVtab));

  *ppVtab = &pTab->ತಳ;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsdir_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(sqlite3_vtab *tab, sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo){
  ಪೂರ್ಣಾಂಕ ii;

  pIdxInfo->estimated_ವೆಚ್ಚ = 1000000000.0;

  ಗಾಗಿ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint ಸ್ಥಿರಾಂಕ *p = &pIdxInfo->aConstraint[ii];
    ಆದರೆ( p->i_ಸ್ತಂಭ==0 && p->usable && p->op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_EQ ){
      ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argv_ಸೂಚ್ಯಂಕ = 1;
      pIdxInfo->idx_ಅಂಕೆ = 1;
      pIdxInfo->estimated_ವೆಚ್ಚ = 1.0;
      ನಿಲ್ಲಿಸು;
    }
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(ಗಾತ್ರ(FsdirCsr)+256);
  ಆದರೆ( pCur==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  memset(pCur, 0, ಗಾತ್ರ(FsdirCsr));
  *ppCursor = &pCur->ತಳ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  ಆದರೆ( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsdir_ಮುಂದಿನ(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  ಆದರೆ( pCsr->pDir ){
    pCsr->p_ನಮೂದು = readdir(pCsr->pDir);
    ಆದರೆ( pCsr->p_ನಮೂದು==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** xFilter method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDir;
  ಪೂರ್ಣಾಂಕ nDir;


  ಆದರೆ( idx_ಅಂಕೆ!=1 || argc!=1 ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  ಆದರೆ( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
  nDir = sqlite3_ಮೌಲ್ಯ_ಬೈಟುಗಳು(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  ಆದರೆ( pCsr->zDir==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  ಸ್ಮರಣೆ_ನಕಲು(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  ಹಿಂತಿರುಗಿ fsdir_ಮುಂದಿನ(pVtabCursor); 
}

/*
** xEof method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  ಹಿಂತಿರುಗಿ pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsdir_ಸ್ತಂಭ(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ಪೂರ್ಣಾಂಕ i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  ಆಯ್ಕೆ( i ){
    ಸ್ಥಿತಿ 0: /* dir */
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ 1: /* name */
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pCsr->p_ನಮೂದು->d_name, -1, SQLITE_TRANSIENT);
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      ಪ್ರತಿಜ್ಞಾ( 0 );
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** xRowid method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ FstreeVtab FstreeVtab;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ FstreeCsr FstreeCsr;
ರಚನೆ FstreeVtab {
  sqlite3_vtab ತಳ;
  sqlite3 *db;
};

ರಚನೆ FstreeCsr {
  sqlite3_vtab_cursor ತಳ;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  ಪೂರ್ಣಾಂಕ fd;                         /* File descriptor open on current path */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fstreeConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  FstreeVtab *pTab;

  ಆದರೆ( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(ಗಾತ್ರ(FstreeVtab));
  ಆದರೆ( !pTab ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  memset(pTab, 0, ಗಾತ್ರ(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->ತಳ;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fstree_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(sqlite3_vtab *tab, sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo){
  ಪೂರ್ಣಾಂಕ ii;

  ಗಾಗಿ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint ಸ್ಥಿರಾಂಕ *p = &pIdxInfo->aConstraint[ii];
    ಆದರೆ( p->i_ಸ್ತಂಭ==0 && p->usable && (
          p->op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_GLOB
       || p->op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_LIKE
       || p->op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_EQ
    )){
      ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_ಅಂಕೆ = p->op;
      pUsage->argv_ಸೂಚ್ಯಂಕ = 1;
      pIdxInfo->estimated_ವೆಚ್ಚ = 100000.0;
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
  }

  pIdxInfo->estimated_ವೆಚ್ಚ = 1000000000.0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(ಗಾತ್ರ(FstreeCsr));
  ಆದರೆ( pCur==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  memset(pCur, 0, ಗಾತ್ರ(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->ತಳ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ fstreeCloseFd(FstreeCsr *pCsr){
  ಆದರೆ( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fstree_ಮುಂದಿನ(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ಪೂರ್ಣಾಂಕ rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_ಹೆಜ್ಜೆ(pCsr->pStmt);
  ಆದರೆ( rc!=SQLITE_ಅಡ್ಡಸಾಲು ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = SQLITE_OK;
    pCsr->fd = open((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pCsr->pStmt, 0), O_RDONLY);
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** xFilter method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->ತಳ.pVtab);
  ಪೂರ್ಣಾಂಕ rc;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  ಅಕ್ಷರ *z_ಬೇರು;
  ಪೂರ್ಣಾಂಕ n_ಬೇರು;
  ಅಕ್ಷರ *z_ಪೂರ್ವಪ್ರತ್ಯಯ;
  ಪೂರ್ಣಾಂಕ n_ಪೂರ್ವಪ್ರತ್ಯಯ;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDir;
  ಪೂರ್ಣಾಂಕ nDir;
  ಅಕ್ಷರ aWild[2] = { '\0', '\0' };

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDrive = getenv("fstreeDrive");
  ಆದರೆ( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  z_ಬೇರು = sqlite3_mprintf("%s%c", zDrive, '/');
  n_ಬೇರು = sqlite3Strlen30(z_ಬೇರು);
  z_ಪೂರ್ವಪ್ರತ್ಯಯ = sqlite3_mprintf("%s", zDrive);
  n_ಪೂರ್ವಪ್ರತ್ಯಯ = sqlite3Strlen30(z_ಪೂರ್ವಪ್ರತ್ಯಯ);
#ಇಲ್ಲದಿದ್ದರೆ
  z_ಬೇರು = "/";
  n_ಬೇರು = 1;
  z_ಪೂರ್ವಪ್ರತ್ಯಯ = "";
  n_ಪೂರ್ವಪ್ರತ್ಯಯ = 0;
#ಆದರೆ_ಅಂತ್ಯ

  zDir = z_ಬೇರು;
  nDir = n_ಬೇರು;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;

  ಆದರೆ( idx_ಅಂಕೆ ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zQuery = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[0]);
    ಆಯ್ಕೆ( idx_ಅಂಕೆ ){
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        ನಿಲ್ಲಿಸು;
    }

    ಆದರೆ( sqlite3_strnicmp(zQuery, z_ಪೂರ್ವಪ್ರತ್ಯಯ, n_ಪೂರ್ವಪ್ರತ್ಯಯ)==0 ){
      ಪೂರ್ಣಾಂಕ i;
      ಗಾಗಿ(i=n_ಪೂರ್ವಪ್ರತ್ಯಯ; zQuery[i]; i++){
        ಆದರೆ( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) ನಿಲ್ಲಿಸು;
        ಆದರೆ( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  ಆದರೆ( nDir==0 ) nDir = 1;

  sqlite3_bind_ಪಠ್ಯ(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_ಪಠ್ಯ(pCsr->pStmt, 2, z_ಬೇರು, n_ಬೇರು, SQLITE_TRANSIENT);
  sqlite3_bind_ಪಠ್ಯ(pCsr->pStmt, 3, z_ಪೂರ್ವಪ್ರತ್ಯಯ, n_ಪೂರ್ವಪ್ರತ್ಯಯ, SQLITE_TRANSIENT);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  sqlite3_free(z_ಪೂರ್ವಪ್ರತ್ಯಯ);
  sqlite3_free(z_ಬೇರು);
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ fstree_ಮುಂದಿನ(pVtabCursor); 
}

/*
** xEof method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ಹಿಂತಿರುಗಿ pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fstree_ಸ್ತಂಭ(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ಪೂರ್ಣಾಂಕ i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ಆದರೆ( i==0 ){      /*ಮಾರ್ಗ*/
    sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(ctx, sqlite3_ಸ್ತಂಭ_ಮೌಲ್ಯ(pCsr->pStmt, 0));
  }ಇಲ್ಲದಿದ್ದರೆ{
    ರಚನೆ stat sBuf;
    fstat(pCsr->fd, &sBuf);

    ಆದರೆ( S_ISREG(sBuf.st_ಬಹುಲಕ) ){
      ಆದರೆ( i==1 ){
        sqlite3_ಫಲಿತಾಂಶ_int64(ctx, sBuf.st_ಗಾತ್ರ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ nRead;
        ಅಕ್ಷರ *aBuf = sqlite3_malloc(sBuf.st_ಬಹುಲಕ+1);
        ಆದರೆ( !aBuf ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
        nRead = ಪಠ(pCsr->fd, aBuf, sBuf.st_ಬಹುಲಕ);
        ಆದರೆ( nRead!=sBuf.st_ಬಹುಲಕ ){
          ಹಿಂತಿರುಗಿ SQLITE_IOERR;
        }
        sqlite3_ಫಲಿತಾಂಶ_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** xRowid method implementation.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  fs_vtab *pVtab;
  ಪೂರ್ಣಾಂಕ nByte;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTbl;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb = argv[1];

  ಆದರೆ( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = ಗಾತ್ರ(fs_vtab) + (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zTbl) + 1 + (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  ಆದರೆ( !pVtab ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;

  pVtab->zTbl = (ಅಕ್ಷರ *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zTbl)+1];
  pVtab->db = db;
  ಸ್ಮರಣೆ_ನಕಲು(pVtab->zTbl, zTbl, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zTbl));
  ಸ್ಮರಣೆ_ನಕಲು(pVtab->zDb, zDb, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zDb));
  *ppVtab = &pVtab->ತಳ;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(ಗಾತ್ರ(fs_cursor));
  *ppCursor = &pCur->ತಳ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Close a fs cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_ಮುಂದಿನ(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  ಪೂರ್ಣಾಂಕ rc;

  rc = sqlite3_ಹೆಜ್ಜೆ(pCur->pStmt);
  ಆದರೆ( rc==SQLITE_ಅಡ್ಡಸಾಲು || rc==SQLITE_ಮುಗಿದ ) rc = SQLITE_OK;

  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  ಪೂರ್ಣಾಂಕ rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  ಪ್ರತಿಜ್ಞಾ( (idx_ಅಂಕೆ==0 && argc==0) || (idx_ಅಂಕೆ==1 && argc==1) );
  ಆದರೆ( idx_ಅಂಕೆ==1 ){
    ಅಕ್ಷರ *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    ಆದರೆ( !zStmt ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    ಆದರೆ( rc==SQLITE_OK ){
      sqlite3_bind_ಮೌಲ್ಯ(pCur->pStmt, 1, argv[0]);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಅಕ್ಷರ *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    ಆದರೆ( !zStmt ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  ಆದರೆ( rc==SQLITE_OK ){
    rc = fs_ಮುಂದಿನ(pVtabCursor); 
  }
  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_ಸ್ತಂಭ(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ಪೂರ್ಣಾಂಕ i){
  fs_cursor *pCur = (fs_cursor*)cur;

  ಪ್ರತಿಜ್ಞಾ( i==0 || i==1 || i==2 );
  ಆದರೆ( i==0 ){
    sqlite3_ಫಲಿತಾಂಶ_ಮೌಲ್ಯ(ctx, sqlite3_ಸ್ತಂಭ_ಮೌಲ್ಯ(pCur->pStmt, 0));
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFile = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pCur->pStmt, 1);
    ರಚನೆ stat sbuf;
    ಪೂರ್ಣಾಂಕ fd;

    ಪೂರ್ಣಾಂಕ n;
    fd = open(zFile, O_RDONLY);
    ಆದರೆ( fd<0 ) ಹಿಂತಿರುಗಿ SQLITE_IOERR;
    fstat(fd, &sbuf);

    ಆದರೆ( sbuf.st_ಗಾತ್ರ>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_ಗಾತ್ರ*2;
      ಅಕ್ಷರ *zNew;
      ಆದರೆ( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      ಆದರೆ( zNew==0 ){
        close(fd);
        ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (ಪೂರ್ಣಾಂಕ)ಪಠ(fd, pCur->zBuf, sbuf.st_ಗಾತ್ರ);
    close(fd);
    ಆದರೆ( n!=sbuf.st_ಗಾತ್ರ ) ಹಿಂತಿರುಗಿ SQLITE_ERROR;
    pCur->nBuf = sbuf.st_ಗಾತ್ರ;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_ಸ್ತಂಭ_int64(pCur->pStmt, 0);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  ಹಿಂತಿರುಗಿ (sqlite3_ದತ್ತಾಂಶ_ಎಣಿಕೆ(pCur->pStmt)==0);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fs_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(sqlite3_vtab *tab, sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo){
  ಪೂರ್ಣಾಂಕ ii;

  ಗಾಗಿ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint ಸ್ಥಿರಾಂಕ *pCons = &pIdxInfo->aConstraint[ii];
    ಆದರೆ( pCons->i_ಸ್ತಂಭ<0 && pCons->usable
           && pCons->op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_EQ ){
      ರಚನೆ sqlite3_ಸೂಚ್ಯಂಕ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argv_ಸೂಚ್ಯಂಕ = 1;
      pIdxInfo->idx_ಅಂಕೆ = 1;
      pIdxInfo->estimated_ವೆಚ್ಚ = 1.0;
      ನಿಲ್ಲಿಸು;
    }
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ಸ್ಥಿರ sqlite3_module fsModule = {
  0,                         /* iVersion */
  fsConnect,
  fsConnect,
  fs_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,
  fsDisconnect, 
  fsDisconnect,
  fsOpen,                      /* xOpen - open a cursor */
  fsClose,                     /* xClose - close a cursor */
  fsFilter,                    /* xFilter - configure scan constraints */
  fs_ಮುಂದಿನ,                      /* xNext - advance a cursor */
  fsEof,                       /* xEof - check for end of scan */
  fs_ಸ್ತಂಭ,                    /* xColumn - read data */
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
  fsdir_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,                 /* xBestIndex */
  fsdirDisconnect,                /* xDisconnect */
  fsdirDisconnect,                /* xDestroy */
  fsdirOpen,                      /* xOpen - open a cursor */
  fsdirClose,                     /* xClose - close a cursor */
  fsdirFilter,                    /* xFilter - configure scan constraints */
  fsdir_ಮುಂದಿನ,                      /* xNext - advance a cursor */
  fsdirEof,                       /* xEof - check for end of scan */
  fsdir_ಸ್ತಂಭ,                    /* xColumn - read data */
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
  fstree_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,                /* xBestIndex */
  fstreeDisconnect,               /* xDisconnect */
  fstreeDisconnect,               /* xDestroy */
  fstreeOpen,                     /* xOpen - open a cursor */
  fstreeClose,                    /* xClose - close a cursor */
  fstreeFilter,                   /* xFilter - configure scan constraints */
  fstree_ಮುಂದಿನ,                     /* xNext - advance a cursor */
  fstreeEof,                      /* xEof - check for end of scan */
  fstree_ಸ್ತಂಭ,                   /* xColumn - read data */
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
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp *interp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI ರಿಜಿಸ್ಟರ್_fs_module(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ಪೂರ್ಣಾಂಕ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ಆದರೆ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) ) ಹಿಂತಿರುಗಿ TCL_ERROR;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (ಶೂನ್ಯ *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ಆದರೆ_ಅಂತ್ಯ


/*
** Register commands with the TCL interpreter.
*/
ಪೂರ್ಣಾಂಕ Sqlitetestfs_Init(Tcl_Interp *interp){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಸ್ಥಿರ ರಚನೆ {
     ಅಕ್ಷರ *zName;
     Tcl_ObjCmdProc *xProc;
     ಶೂನ್ಯ *client_ದತ್ತಾಂಶ;
  } aObjCmd[] = {
     { "register_fs_module",   ರಿಜಿಸ್ಟರ್_fs_module, 0 },
  };
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aObjCmd)/ಗಾತ್ರ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ದತ್ತಾಂಶ, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ TCL_OK;
}
