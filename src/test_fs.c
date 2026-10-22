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
#include "sqliteInt.h"
#include "tclsqlite.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#නම් !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#නැතහොත්
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
# endif
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE

typedef sa_nracanaa fs_vtab fs_vtab;
typedef sa_nracanaa fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
sa_nracanaa fs_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
  var_na *zDb;                      /* Name of db containing zTbl */
  var_na *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
sa_nracanaa fs_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
  var_na *zBuf;
  පූර්ණ_සංඛ්‍යා nBuf;
  පූර්ණ_සංඛ්‍යා nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
typedef sa_nracanaa FsdirVtab FsdirVtab;
typedef sa_nracanaa FsdirCsr FsdirCsr;
sa_nracanaa FsdirVtab {
  sqlite3_vtab base;
};

sa_nracanaa FsdirCsr {
  sqlite3_vtab_cursor base;
  var_na *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  sa_nracanaa dirent *pEntry;
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
static පූර්ණ_සංඛ්‍යා fsdirConnect(
  sqlite3 *db,
  void *pAux,
  පූර්ණ_සංඛ්‍යා argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  FsdirVtab *pTab;

  නම්( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ආපසු SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(parimaa_na(FsdirVtab));
  නම්( !pTab ) ආපසු SQLITE_NOMEM;
  memset(pTab, 0, parimaa_na(FsdirVtab));

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  ආපසු SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
static පූර්ණ_සංඛ්‍යා fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ආපසු SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
static පූර්ණ_සංඛ්‍යා fsdirBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  පූර්ණ_සංඛ්‍යා ii;

  pIdxInfo->estimatedCost = 1000000000.0;

  සඳහා(ii=0; ii<pIdxInfo->nConstraint; ii++){
    sa_nracanaa sqlite3_index_constraint const *p = &pIdxInfo->aConstraint[ii];
    නම්( p->iColumn==0 && p->usable && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      sa_nracanaa sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argvIndex = 1;
      pIdxInfo->idxNum = 1;
      pIdxInfo->estimatedCost = 1.0;
      bha_nga;
    }
  }

  ආපසු SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
static පූර්ණ_සංඛ්‍යා fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(parimaa_na(FsdirCsr)+256);
  නම්( pCur==0 ) ආපසු SQLITE_NOMEM;
  memset(pCur, 0, parimaa_na(FsdirCsr));
  *ppCursor = &pCur->base;
  ආපසු SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
static පූර්ණ_සංඛ්‍යා fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  නම්( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  ආපසු SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
static පූර්ණ_සංඛ්‍යා fsdirNext(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  නම්( pCsr->pDir ){
    pCsr->pEntry = readdir(pCsr->pDir);
    නම්( pCsr->pEntry==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  ආපසු SQLITE_OK;
}

/*
** xFilter method implementation.
*/
static පූර්ණ_සංඛ්‍යා fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  පූර්ණ_සංඛ්‍යා idxNum, const var_na *idxStr,
  පූර්ණ_සංඛ්‍යා argc, sqlite3_value **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  const var_na *zDir;
  පූර්ණ_සංඛ්‍යා nDir;


  නම්( idxNum!=1 || argc!=1 ){
    ආපසු SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  නම්( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (const var_na*)sqlite3_value_text(argv[0]);
  nDir = sqlite3_value_bytes(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  නම්( pCsr->zDir==0 ) ආපසු SQLITE_NOMEM;
  memcpy(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  ආපසු fsdirNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
static පූර්ණ_සංඛ්‍යා fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  ආපසු pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
static පූර්ණ_සංඛ්‍යා fsdirColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, පූර්ණ_සංඛ්‍යා i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  switch( i ){
    pak_sa 0: /* dir */
      sqlite3_result_text(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      bha_nga;

    pak_sa 1: /* name */
      sqlite3_result_text(ctx, pCsr->pEntry->d_name, -1, SQLITE_TRANSIENT);
      bha_nga;

    default:
      assert( 0 );
  }

  ආපසු SQLITE_OK;
}

/*
** xRowid method implementation.
*/
static පූර්ණ_සංඛ්‍යා fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  ආපසු SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
typedef sa_nracanaa FstreeVtab FstreeVtab;
typedef sa_nracanaa FstreeCsr FstreeCsr;
sa_nracanaa FstreeVtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

sa_nracanaa FstreeCsr {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  පූර්ණ_සංඛ්‍යා fd;                         /* File descriptor open on current path */
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
static පූර්ණ_සංඛ්‍යා fstreeConnect(
  sqlite3 *db,
  void *pAux,
  පූර්ණ_සංඛ්‍යා argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  FstreeVtab *pTab;

  නම්( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ආපසු SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(parimaa_na(FstreeVtab));
  නම්( !pTab ) ආපසු SQLITE_NOMEM;
  memset(pTab, 0, parimaa_na(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  ආපසු SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
static පූර්ණ_සංඛ්‍යා fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ආපසු SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
static පූර්ණ_සංඛ්‍යා fstreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  පූර්ණ_සංඛ්‍යා ii;

  සඳහා(ii=0; ii<pIdxInfo->nConstraint; ii++){
    sa_nracanaa sqlite3_index_constraint const *p = &pIdxInfo->aConstraint[ii];
    නම්( p->iColumn==0 && p->usable && (
          p->op==SQLITE_INDEX_CONSTRAINT_GLOB
       || p->op==SQLITE_INDEX_CONSTRAINT_LIKE
       || p->op==SQLITE_INDEX_CONSTRAINT_EQ
    )){
      sa_nracanaa sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idxNum = p->op;
      pUsage->argvIndex = 1;
      pIdxInfo->estimatedCost = 100000.0;
      ආපසු SQLITE_OK;
    }
  }

  pIdxInfo->estimatedCost = 1000000000.0;
  ආපසු SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
static පූර්ණ_සංඛ්‍යා fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(parimaa_na(FstreeCsr));
  නම්( pCur==0 ) ආපසු SQLITE_NOMEM;
  memset(pCur, 0, parimaa_na(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->base;
  ආපසු SQLITE_OK;
}

static void fstreeCloseFd(FstreeCsr *pCsr){
  නම්( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
static පූර්ණ_සංඛ්‍යා fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  ආපසු SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
static පූර්ණ_සංඛ්‍යා fstreeNext(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  පූර්ණ_සංඛ්‍යා rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_step(pCsr->pStmt);
  නම්( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }නැතහොත්{
    rc = SQLITE_OK;
    pCsr->fd = open((const var_na*)sqlite3_column_text(pCsr->pStmt, 0), O_RDONLY);
  }

  ආපසු rc;
}

/*
** xFilter method implementation.
*/
static පූර්ණ_සංඛ්‍යා fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  පූර්ණ_සංඛ්‍යා idxNum, const var_na *idxStr,
  පූර්ණ_සංඛ්‍යා argc, sqlite3_value **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->base.pVtab);
  පූර්ණ_සංඛ්‍යා rc;
  const var_na *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  var_na *zRoot;
  පූර්ණ_සංඛ්‍යා nRoot;
  var_na *zPrefix;
  පූර්ණ_සංඛ්‍යා nPrefix;
  const var_na *zDir;
  පූර්ණ_සංඛ්‍යා nDir;
  var_na aWild[2] = { '\0', '\0' };

#ifdef _WIN32
  const var_na *zDrive = getenv("fstreeDrive");
  නම්( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  zRoot = sqlite3_mprintf("%s%c", zDrive, '/');
  nRoot = sqlite3Strlen30(zRoot);
  zPrefix = sqlite3_mprintf("%s", zDrive);
  nPrefix = sqlite3Strlen30(zPrefix);
#නැතහොත්
  zRoot = "/";
  nRoot = 1;
  zPrefix = "";
  nPrefix = 0;
#endif

  zDir = zRoot;
  nDir = nRoot;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  නම්( rc!=SQLITE_OK ) ආපසු rc;

  නම්( idxNum ){
    const var_na *zQuery = (const var_na*)sqlite3_value_text(argv[0]);
    switch( idxNum ){
      pak_sa SQLITE_INDEX_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        bha_nga;
      pak_sa SQLITE_INDEX_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        bha_nga;
    }

    නම්( sqlite3_strnicmp(zQuery, zPrefix, nPrefix)==0 ){
      පූර්ණ_සංඛ්‍යා i;
      සඳහා(i=nPrefix; zQuery[i]; i++){
        නම්( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) bha_nga;
        නම්( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  නම්( nDir==0 ) nDir = 1;

  sqlite3_bind_text(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 2, zRoot, nRoot, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 3, zPrefix, nPrefix, SQLITE_TRANSIENT);

#ifdef _WIN32
  sqlite3_free(zPrefix);
  sqlite3_free(zRoot);
#endif

  ආපසු fstreeNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
static පූර්ණ_සංඛ්‍යා fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ආපසු pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
static පූර්ණ_සංඛ්‍යා fstreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, පූර්ණ_සංඛ්‍යා i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  නම්( i==0 ){      /* path */
    sqlite3_result_value(ctx, sqlite3_column_value(pCsr->pStmt, 0));
  }නැතහොත්{
    sa_nracanaa stat sBuf;
    fstat(pCsr->fd, &sBuf);

    නම්( S_ISREG(sBuf.st_mode) ){
      නම්( i==1 ){
        sqlite3_result_int64(ctx, sBuf.st_size);
      }නැතහොත්{
        පූර්ණ_සංඛ්‍යා nRead;
        var_na *aBuf = sqlite3_malloc(sBuf.st_mode+1);
        නම්( !aBuf ) ආපසු SQLITE_NOMEM;
        nRead = read(pCsr->fd, aBuf, sBuf.st_mode);
        නම්( nRead!=sBuf.st_mode ){
          ආපසු SQLITE_IOERR;
        }
        sqlite3_result_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  ආපසු SQLITE_OK;
}

/*
** xRowid method implementation.
*/
static පූර්ණ_සංඛ්‍යා fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  ආපසු SQLITE_OK;
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
static පූර්ණ_සංඛ්‍යා fsConnect(
  sqlite3 *db,
  void *pAux,
  පූර්ණ_සංඛ්‍යා argc, const var_na *const*argv,
  sqlite3_vtab **ppVtab,
  var_na **pzErr
){
  fs_vtab *pVtab;
  පූර්ණ_සංඛ්‍යා nByte;
  const var_na *zTbl;
  const var_na *zDb = argv[1];

  නම්( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ආපසු SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = parimaa_na(fs_vtab) + (පූර්ණ_සංඛ්‍යා)strlen(zTbl) + 1 + (පූර්ණ_සංඛ්‍යා)strlen(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  නම්( !pVtab ) ආපසු SQLITE_NOMEM;

  pVtab->zTbl = (var_na *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[strlen(zTbl)+1];
  pVtab->db = db;
  memcpy(pVtab->zTbl, zTbl, strlen(zTbl));
  memcpy(pVtab->zDb, zDb, strlen(zDb));
  *ppVtab = &pVtab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  ආපසු SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

static පූර්ණ_සංඛ්‍යා fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ආපසු SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
static පූර්ණ_සංඛ්‍යා fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(parimaa_na(fs_cursor));
  *ppCursor = &pCur->base;
  ආපසු SQLITE_OK;
}

/*
** Close a fs cursor.
*/
static පූර්ණ_සංඛ්‍යා fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  ආපසු SQLITE_OK;
}

static පූර්ණ_සංඛ්‍යා fsNext(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  පූර්ණ_සංඛ්‍යා rc;

  rc = sqlite3_step(pCur->pStmt);
  නම්( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;

  ආපසු rc;
}

static පූර්ණ_සංඛ්‍යා fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  පූර්ණ_සංඛ්‍යා idxNum, const var_na *idxStr,
  පූර්ණ_සංඛ්‍යා argc, sqlite3_value **argv
){
  පූර්ණ_සංඛ්‍යා rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  assert( (idxNum==0 && argc==0) || (idxNum==1 && argc==1) );
  නම්( idxNum==1 ){
    var_na *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    නම්( !zStmt ) ආපසු SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    නම්( rc==SQLITE_OK ){
      sqlite3_bind_value(pCur->pStmt, 1, argv[0]);
    }
  }නැතහොත්{
    var_na *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    නම්( !zStmt ) ආපසු SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  නම්( rc==SQLITE_OK ){
    rc = fsNext(pVtabCursor); 
  }
  ආපසු rc;
}

static පූර්ණ_සංඛ්‍යා fsColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, පූර්ණ_සංඛ්‍යා i){
  fs_cursor *pCur = (fs_cursor*)cur;

  assert( i==0 || i==1 || i==2 );
  නම්( i==0 ){
    sqlite3_result_value(ctx, sqlite3_column_value(pCur->pStmt, 0));
  }නැතහොත්{
    const var_na *zFile = (const var_na *)sqlite3_column_text(pCur->pStmt, 1);
    sa_nracanaa stat sbuf;
    පූර්ණ_සංඛ්‍යා fd;

    පූර්ණ_සංඛ්‍යා n;
    fd = open(zFile, O_RDONLY);
    නම්( fd<0 ) ආපසු SQLITE_IOERR;
    fstat(fd, &sbuf);

    නම්( sbuf.st_size>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_size*2;
      var_na *zNew;
      නම්( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      නම්( zNew==0 ){
        close(fd);
        ආපසු SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (පූර්ණ_සංඛ්‍යා)read(fd, pCur->zBuf, sbuf.st_size);
    close(fd);
    නම්( n!=sbuf.st_size ) ආපසු SQLITE_ERROR;
    pCur->nBuf = sbuf.st_size;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_result_text(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  ආපසු SQLITE_OK;
}

static පූර්ණ_සංඛ්‍යා fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_column_int64(pCur->pStmt, 0);
  ආපසු SQLITE_OK;
}

static පූර්ණ_සංඛ්‍යා fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  ආපසු (sqlite3_data_count(pCur->pStmt)==0);
}

static පූර්ණ_සංඛ්‍යා fsBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  පූර්ණ_සංඛ්‍යා ii;

  සඳහා(ii=0; ii<pIdxInfo->nConstraint; ii++){
    sa_nracanaa sqlite3_index_constraint const *pCons = &pIdxInfo->aConstraint[ii];
    නම්( pCons->iColumn<0 && pCons->usable
           && pCons->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      sa_nracanaa sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argvIndex = 1;
      pIdxInfo->idxNum = 1;
      pIdxInfo->estimatedCost = 1.0;
      bha_nga;
    }
  }

  ආපසු SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
static sqlite3_module fsModule = {
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

static sqlite3_module fsdirModule = {
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

static sqlite3_module fstreeModule = {
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
extern පූර්ණ_සංඛ්‍යා getDbPointer(Tcl_Interp *interp, const var_na *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
static පූර්ණ_සංඛ්‍යා SQLITE_TCLAPI register_fs_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  පූර්ණ_සංඛ්‍යා objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  නම්( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    ආපසු TCL_ERROR;
  }
  නම්( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ආපසු TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (void *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#endif
  ආපසු TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
පූර්ණ_සංඛ්‍යා Sqlitetestfs_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  static sa_nracanaa {
     var_na *zName;
     Tcl_ObjCmdProc *xProc;
     void *clientData;
  } aObjCmd[] = {
     { "register_fs_module",   register_fs_module, 0 },
  };
  පූර්ණ_සංඛ්‍යා i;
  සඳහා(i=0; i<parimaa_na(aObjCmd)/parimaa_na(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  ආපසු TCL_OK;
}
