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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "tclsqlite.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <fcntl.h>

#എങ്കിൽ !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#അല്ലെങ്കിൽ
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
# endif
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE

തരനിർവചനം ഘടന fs_vtab fs_vtab;
തരനിർവചനം ഘടന fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
ഘടന fs_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
  അക്ഷരം *zDb;                      /* Name of db containing zTbl */
  അക്ഷരം *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
ഘടന fs_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
  അക്ഷരം *zBuf;
  പൂർണ്ണസംഖ്യ nBuf;
  പൂർണ്ണസംഖ്യ nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
തരനിർവചനം ഘടന FsdirVtab FsdirVtab;
തരനിർവചനം ഘടന FsdirCsr FsdirCsr;
ഘടന FsdirVtab {
  sqlite3_vtab base;
};

ഘടന FsdirCsr {
  sqlite3_vtab_cursor base;
  അക്ഷരം *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  ഘടന dirent *pEntry;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ fsdirConnect(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  FsdirVtab *pTab;

  എങ്കിൽ( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    തിരികെ SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(വലുപ്പം(FsdirVtab));
  എങ്കിൽ( !pTab ) തിരികെ SQLITE_NOMEM;
  memset(pTab, 0, വലുപ്പം(FsdirVtab));

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  തിരികെ SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  തിരികെ SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsdirBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  പൂർണ്ണസംഖ്യ ii;

  pIdxInfo->estimatedCost = 1000000000.0;

  വേണ്ടി(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ഘടന sqlite3_index_constraint സ്ഥിരം *p = &pIdxInfo->aConstraint[ii];
    എങ്കിൽ( p->iColumn==0 && p->usable && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      ഘടന sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_അക്കം = 1;
      pIdxInfo->estimatedCost = 1.0;
      നിർത്തുക;
    }
  }

  തിരികെ SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(വലുപ്പം(FsdirCsr)+256);
  എങ്കിൽ( pCur==0 ) തിരികെ SQLITE_NOMEM;
  memset(pCur, 0, വലുപ്പം(FsdirCsr));
  *ppCursor = &pCur->base;
  തിരികെ SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  എങ്കിൽ( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  തിരികെ SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsdirNext(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  എങ്കിൽ( pCsr->pDir ){
    pCsr->pEntry = readdir(pCsr->pDir);
    എങ്കിൽ( pCsr->pEntry==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  തിരികെ SQLITE_OK;
}

/*
** xFilter method implementation.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  സ്ഥിരം അക്ഷരം *zDir;
  പൂർണ്ണസംഖ്യ nDir;


  എങ്കിൽ( idx_അക്കം!=1 || argc!=1 ){
    തിരികെ SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  എങ്കിൽ( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
  nDir = sqlite3_മൂല്യം_bytes(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  എങ്കിൽ( pCsr->zDir==0 ) തിരികെ SQLITE_NOMEM;
  memcpy(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  തിരികെ fsdirNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  തിരികെ pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsdirColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, പൂർണ്ണസംഖ്യ i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  തിരഞ്ഞെടുപ്പ്( i ){
    അവസ്ഥ 0: /* dir */
      sqlite3_result_text(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      നിർത്തുക;

    അവസ്ഥ 1: /* name */
      sqlite3_result_text(ctx, pCsr->pEntry->d_name, -1, SQLITE_TRANSIENT);
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      പ്രതിജ്ഞാ( 0 );
  }

  തിരികെ SQLITE_OK;
}

/*
** xRowid method implementation.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  തിരികെ SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
തരനിർവചനം ഘടന FstreeVtab FstreeVtab;
തരനിർവചനം ഘടന FstreeCsr FstreeCsr;
ഘടന FstreeVtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

ഘടന FstreeCsr {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  പൂർണ്ണസംഖ്യ fd;                         /* File descriptor open on current path */
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
സ്ഥിതം പൂർണ്ണസംഖ്യ fstreeConnect(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  FstreeVtab *pTab;

  എങ്കിൽ( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    തിരികെ SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(വലുപ്പം(FstreeVtab));
  എങ്കിൽ( !pTab ) തിരികെ SQLITE_NOMEM;
  memset(pTab, 0, വലുപ്പം(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  തിരികെ SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  തിരികെ SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fstreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  പൂർണ്ണസംഖ്യ ii;

  വേണ്ടി(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ഘടന sqlite3_index_constraint സ്ഥിരം *p = &pIdxInfo->aConstraint[ii];
    എങ്കിൽ( p->iColumn==0 && p->usable && (
          p->op==SQLITE_INDEX_CONSTRAINT_GLOB
       || p->op==SQLITE_INDEX_CONSTRAINT_LIKE
       || p->op==SQLITE_INDEX_CONSTRAINT_EQ
    )){
      ഘടന sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_അക്കം = p->op;
      pUsage->argvIndex = 1;
      pIdxInfo->estimatedCost = 100000.0;
      തിരികെ SQLITE_OK;
    }
  }

  pIdxInfo->estimatedCost = 1000000000.0;
  തിരികെ SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(വലുപ്പം(FstreeCsr));
  എങ്കിൽ( pCur==0 ) തിരികെ SQLITE_NOMEM;
  memset(pCur, 0, വലുപ്പം(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->base;
  തിരികെ SQLITE_OK;
}

സ്ഥിതം ശൂന്യം fstreeCloseFd(FstreeCsr *pCsr){
  എങ്കിൽ( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  തിരികെ SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fstreeNext(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  പൂർണ്ണസംഖ്യ rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_step(pCsr->pStmt);
  എങ്കിൽ( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }അല്ലെങ്കിൽ{
    rc = SQLITE_OK;
    pCsr->fd = open((സ്ഥിരം അക്ഷരം*)sqlite3_column_text(pCsr->pStmt, 0), O_RDONLY);
  }

  തിരികെ rc;
}

/*
** xFilter method implementation.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->base.pVtab);
  പൂർണ്ണസംഖ്യ rc;
  സ്ഥിരം അക്ഷരം *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  അക്ഷരം *z_വേര്;
  പൂർണ്ണസംഖ്യ n_വേര്;
  അക്ഷരം *zPrefix;
  പൂർണ്ണസംഖ്യ nPrefix;
  സ്ഥിരം അക്ഷരം *zDir;
  പൂർണ്ണസംഖ്യ nDir;
  അക്ഷരം aWild[2] = { '\0', '\0' };

#ifdef _WIN32
  സ്ഥിരം അക്ഷരം *zDrive = getenv("fstreeDrive");
  എങ്കിൽ( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  z_വേര് = sqlite3_mprintf("%s%c", zDrive, '/');
  n_വേര് = sqlite3Strlen30(z_വേര്);
  zPrefix = sqlite3_mprintf("%s", zDrive);
  nPrefix = sqlite3Strlen30(zPrefix);
#അല്ലെങ്കിൽ
  z_വേര് = "/";
  n_വേര് = 1;
  zPrefix = "";
  nPrefix = 0;
#endif

  zDir = z_വേര്;
  nDir = n_വേര്;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  എങ്കിൽ( rc!=SQLITE_OK ) തിരികെ rc;

  എങ്കിൽ( idx_അക്കം ){
    സ്ഥിരം അക്ഷരം *zQuery = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[0]);
    തിരഞ്ഞെടുപ്പ്( idx_അക്കം ){
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        നിർത്തുക;
      അവസ്ഥ SQLITE_INDEX_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        നിർത്തുക;
    }

    എങ്കിൽ( sqlite3_strnicmp(zQuery, zPrefix, nPrefix)==0 ){
      പൂർണ്ണസംഖ്യ i;
      വേണ്ടി(i=nPrefix; zQuery[i]; i++){
        എങ്കിൽ( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) നിർത്തുക;
        എങ്കിൽ( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  എങ്കിൽ( nDir==0 ) nDir = 1;

  sqlite3_bind_text(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 2, z_വേര്, n_വേര്, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 3, zPrefix, nPrefix, SQLITE_TRANSIENT);

#ifdef _WIN32
  sqlite3_free(zPrefix);
  sqlite3_free(z_വേര്);
#endif

  തിരികെ fstreeNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  തിരികെ pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fstreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, പൂർണ്ണസംഖ്യ i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  എങ്കിൽ( i==0 ){      /* path */
    sqlite3_result_മൂല്യം(ctx, sqlite3_column_മൂല്യം(pCsr->pStmt, 0));
  }അല്ലെങ്കിൽ{
    ഘടന stat sBuf;
    fstat(pCsr->fd, &sBuf);

    എങ്കിൽ( S_ISREG(sBuf.st_mode) ){
      എങ്കിൽ( i==1 ){
        sqlite3_result_int64(ctx, sBuf.st_size);
      }അല്ലെങ്കിൽ{
        പൂർണ്ണസംഖ്യ nRead;
        അക്ഷരം *aBuf = sqlite3_malloc(sBuf.st_mode+1);
        എങ്കിൽ( !aBuf ) തിരികെ SQLITE_NOMEM;
        nRead = പഠ(pCsr->fd, aBuf, sBuf.st_mode);
        എങ്കിൽ( nRead!=sBuf.st_mode ){
          തിരികെ SQLITE_IOERR;
        }
        sqlite3_result_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  തിരികെ SQLITE_OK;
}

/*
** xRowid method implementation.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  തിരികെ SQLITE_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ fsConnect(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  fs_vtab *pVtab;
  പൂർണ്ണസംഖ്യ nByte;
  സ്ഥിരം അക്ഷരം *zTbl;
  സ്ഥിരം അക്ഷരം *zDb = argv[1];

  എങ്കിൽ( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    തിരികെ SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = വലുപ്പം(fs_vtab) + (പൂർണ്ണസംഖ്യ)strlen(zTbl) + 1 + (പൂർണ്ണസംഖ്യ)strlen(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  എങ്കിൽ( !pVtab ) തിരികെ SQLITE_NOMEM;

  pVtab->zTbl = (അക്ഷരം *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[strlen(zTbl)+1];
  pVtab->db = db;
  memcpy(pVtab->zTbl, zTbl, strlen(zTbl));
  memcpy(pVtab->zDb, zDb, strlen(zDb));
  *ppVtab = &pVtab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  തിരികെ SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

സ്ഥിതം പൂർണ്ണസംഖ്യ fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  തിരികെ SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(വലുപ്പം(fs_cursor));
  *ppCursor = &pCur->base;
  തിരികെ SQLITE_OK;
}

/*
** Close a fs cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ fsNext(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  പൂർണ്ണസംഖ്യ rc;

  rc = sqlite3_step(pCur->pStmt);
  എങ്കിൽ( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;

  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  പൂർണ്ണസംഖ്യ rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  പ്രതിജ്ഞാ( (idx_അക്കം==0 && argc==0) || (idx_അക്കം==1 && argc==1) );
  എങ്കിൽ( idx_അക്കം==1 ){
    അക്ഷരം *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    എങ്കിൽ( !zStmt ) തിരികെ SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    എങ്കിൽ( rc==SQLITE_OK ){
      sqlite3_bind_മൂല്യം(pCur->pStmt, 1, argv[0]);
    }
  }അല്ലെങ്കിൽ{
    അക്ഷരം *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    എങ്കിൽ( !zStmt ) തിരികെ SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  എങ്കിൽ( rc==SQLITE_OK ){
    rc = fsNext(pVtabCursor); 
  }
  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ fsColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, പൂർണ്ണസംഖ്യ i){
  fs_cursor *pCur = (fs_cursor*)cur;

  പ്രതിജ്ഞാ( i==0 || i==1 || i==2 );
  എങ്കിൽ( i==0 ){
    sqlite3_result_മൂല്യം(ctx, sqlite3_column_മൂല്യം(pCur->pStmt, 0));
  }അല്ലെങ്കിൽ{
    സ്ഥിരം അക്ഷരം *zFile = (സ്ഥിരം അക്ഷരം *)sqlite3_column_text(pCur->pStmt, 1);
    ഘടന stat sbuf;
    പൂർണ്ണസംഖ്യ fd;

    പൂർണ്ണസംഖ്യ n;
    fd = open(zFile, O_RDONLY);
    എങ്കിൽ( fd<0 ) തിരികെ SQLITE_IOERR;
    fstat(fd, &sbuf);

    എങ്കിൽ( sbuf.st_size>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_size*2;
      അക്ഷരം *zNew;
      എങ്കിൽ( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      എങ്കിൽ( zNew==0 ){
        close(fd);
        തിരികെ SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (പൂർണ്ണസംഖ്യ)പഠ(fd, pCur->zBuf, sbuf.st_size);
    close(fd);
    എങ്കിൽ( n!=sbuf.st_size ) തിരികെ SQLITE_ERROR;
    pCur->nBuf = sbuf.st_size;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_result_text(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_column_int64(pCur->pStmt, 0);
  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  തിരികെ (sqlite3_data_എണ്ണം(pCur->pStmt)==0);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ fsBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  പൂർണ്ണസംഖ്യ ii;

  വേണ്ടി(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ഘടന sqlite3_index_constraint സ്ഥിരം *pCons = &pIdxInfo->aConstraint[ii];
    എങ്കിൽ( pCons->iColumn<0 && pCons->usable
           && pCons->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      ഘടന sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_അക്കം = 1;
      pIdxInfo->estimatedCost = 1.0;
      നിർത്തുക;
    }
  }

  തിരികെ SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
സ്ഥിതം sqlite3_module fsModule = {
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

സ്ഥിതം sqlite3_module fsdirModule = {
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

സ്ഥിതം sqlite3_module fstreeModule = {
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
ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp *interp, സ്ഥിരം അക്ഷരം *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI register_fs_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  പൂർണ്ണസംഖ്യ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (ശൂന്യം *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#endif
  തിരികെ TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
പൂർണ്ണസംഖ്യ Sqlitetestfs_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
     Tcl_ObjCmdProc *xProc;
     ശൂന്യം *clientData;
  } aObjCmd[] = {
     { "register_fs_module",   register_fs_module, 0 },
  };
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<വലുപ്പം(aObjCmd)/വലുപ്പം(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  തിരികെ TCL_OK;
}
