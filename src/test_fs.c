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

#もし !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#否则
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
# endif
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE

typedef struct fs_vtab fs_vtab;
typedef struct fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
struct fs_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
  char *zDb;                      /* Name of db containing zTbl */
  char *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
struct fs_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
  char *zBuf;
  整数 nBuf;
  整数 nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
typedef struct FsdirVtab FsdirVtab;
typedef struct FsdirCsr FsdirCsr;
struct FsdirVtab {
  sqlite3_vtab base;
};

struct FsdirCsr {
  sqlite3_vtab_cursor base;
  char *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  struct dirent *pEntry;
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
static 整数 fsdirConnect(
  sqlite3 *db,
  void *pAux,
  整数 argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  FsdirVtab *pTab;

  もし( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    戻る SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(sizeof(FsdirVtab));
  もし( !pTab ) 戻る SQLITE_NOMEM;
  memset(pTab, 0, sizeof(FsdirVtab));

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  戻る SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
static 整数 fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  戻る SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
static 整数 fsdirBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  整数 ii;

  pIdxInfo->estimatedCost = 1000000000.0;

  ために(ii=0; ii<pIdxInfo->nConstraint; ii++){
    struct sqlite3_index_constraint const *p = &pIdxInfo->aConstraint[ii];
    もし( p->iColumn==0 && p->usable && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      struct sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argvIndex = 1;
      pIdxInfo->idxNum = 1;
      pIdxInfo->estimatedCost = 1.0;
      break;
    }
  }

  戻る SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
static 整数 fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(sizeof(FsdirCsr)+256);
  もし( pCur==0 ) 戻る SQLITE_NOMEM;
  memset(pCur, 0, sizeof(FsdirCsr));
  *ppCursor = &pCur->base;
  戻る SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
static 整数 fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  もし( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  戻る SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
static 整数 fsdirNext(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  もし( pCsr->pDir ){
    pCsr->pEntry = readdir(pCsr->pDir);
    もし( pCsr->pEntry==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  戻る SQLITE_OK;
}

/*
** xFilter method implementation.
*/
static 整数 fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  整数 idxNum, const char *idxStr,
  整数 argc, sqlite3_value **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  const char *zDir;
  整数 nDir;


  もし( idxNum!=1 || argc!=1 ){
    戻る SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  もし( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (const char*)sqlite3_value_text(argv[0]);
  nDir = sqlite3_value_bytes(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  もし( pCsr->zDir==0 ) 戻る SQLITE_NOMEM;
  memcpy(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  戻る fsdirNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
static 整数 fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  戻る pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
static 整数 fsdirColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, 整数 i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  switch( i ){
    case 0: /* dir */
      sqlite3_result_text(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      break;

    case 1: /* name */
      sqlite3_result_text(ctx, pCsr->pEntry->d_name, -1, SQLITE_TRANSIENT);
      break;

    default:
      assert( 0 );
  }

  戻る SQLITE_OK;
}

/*
** xRowid method implementation.
*/
static 整数 fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  戻る SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
typedef struct FstreeVtab FstreeVtab;
typedef struct FstreeCsr FstreeCsr;
struct FstreeVtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

struct FstreeCsr {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  整数 fd;                         /* File descriptor open on current path */
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
static 整数 fstreeConnect(
  sqlite3 *db,
  void *pAux,
  整数 argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  FstreeVtab *pTab;

  もし( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    戻る SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(sizeof(FstreeVtab));
  もし( !pTab ) 戻る SQLITE_NOMEM;
  memset(pTab, 0, sizeof(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  戻る SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
static 整数 fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  戻る SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
static 整数 fstreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  整数 ii;

  ために(ii=0; ii<pIdxInfo->nConstraint; ii++){
    struct sqlite3_index_constraint const *p = &pIdxInfo->aConstraint[ii];
    もし( p->iColumn==0 && p->usable && (
          p->op==SQLITE_INDEX_CONSTRAINT_GLOB
       || p->op==SQLITE_INDEX_CONSTRAINT_LIKE
       || p->op==SQLITE_INDEX_CONSTRAINT_EQ
    )){
      struct sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idxNum = p->op;
      pUsage->argvIndex = 1;
      pIdxInfo->estimatedCost = 100000.0;
      戻る SQLITE_OK;
    }
  }

  pIdxInfo->estimatedCost = 1000000000.0;
  戻る SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
static 整数 fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(sizeof(FstreeCsr));
  もし( pCur==0 ) 戻る SQLITE_NOMEM;
  memset(pCur, 0, sizeof(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->base;
  戻る SQLITE_OK;
}

static void fstreeCloseFd(FstreeCsr *pCsr){
  もし( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
static 整数 fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  戻る SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
static 整数 fstreeNext(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  整数 rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_step(pCsr->pStmt);
  もし( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }否则{
    rc = SQLITE_OK;
    pCsr->fd = open((const char*)sqlite3_column_text(pCsr->pStmt, 0), O_RDONLY);
  }

  戻る rc;
}

/*
** xFilter method implementation.
*/
static 整数 fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  整数 idxNum, const char *idxStr,
  整数 argc, sqlite3_value **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->base.pVtab);
  整数 rc;
  const char *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  char *zRoot;
  整数 nRoot;
  char *zPrefix;
  整数 nPrefix;
  const char *zDir;
  整数 nDir;
  char aWild[2] = { '\0', '\0' };

#ifdef _WIN32
  const char *zDrive = getenv("fstreeDrive");
  もし( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  zRoot = sqlite3_mprintf("%s%c", zDrive, '/');
  nRoot = sqlite3Strlen30(zRoot);
  zPrefix = sqlite3_mprintf("%s", zDrive);
  nPrefix = sqlite3Strlen30(zPrefix);
#否则
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
  もし( rc!=SQLITE_OK ) 戻る rc;

  もし( idxNum ){
    const char *zQuery = (const char*)sqlite3_value_text(argv[0]);
    switch( idxNum ){
      case SQLITE_INDEX_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        break;
      case SQLITE_INDEX_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        break;
    }

    もし( sqlite3_strnicmp(zQuery, zPrefix, nPrefix)==0 ){
      整数 i;
      ために(i=nPrefix; zQuery[i]; i++){
        もし( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) break;
        もし( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  もし( nDir==0 ) nDir = 1;

  sqlite3_bind_text(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 2, zRoot, nRoot, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 3, zPrefix, nPrefix, SQLITE_TRANSIENT);

#ifdef _WIN32
  sqlite3_free(zPrefix);
  sqlite3_free(zRoot);
#endif

  戻る fstreeNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
static 整数 fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  戻る pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
static 整数 fstreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, 整数 i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  もし( i==0 ){      /* path */
    sqlite3_result_value(ctx, sqlite3_column_value(pCsr->pStmt, 0));
  }否则{
    struct stat sBuf;
    fstat(pCsr->fd, &sBuf);

    もし( S_ISREG(sBuf.st_mode) ){
      もし( i==1 ){
        sqlite3_result_int64(ctx, sBuf.st_size);
      }否则{
        整数 nRead;
        char *aBuf = sqlite3_malloc(sBuf.st_mode+1);
        もし( !aBuf ) 戻る SQLITE_NOMEM;
        nRead = read(pCsr->fd, aBuf, sBuf.st_mode);
        もし( nRead!=sBuf.st_mode ){
          戻る SQLITE_IOERR;
        }
        sqlite3_result_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  戻る SQLITE_OK;
}

/*
** xRowid method implementation.
*/
static 整数 fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  戻る SQLITE_OK;
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
static 整数 fsConnect(
  sqlite3 *db,
  void *pAux,
  整数 argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  fs_vtab *pVtab;
  整数 nByte;
  const char *zTbl;
  const char *zDb = argv[1];

  もし( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    戻る SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = sizeof(fs_vtab) + (整数)strlen(zTbl) + 1 + (整数)strlen(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  もし( !pVtab ) 戻る SQLITE_NOMEM;

  pVtab->zTbl = (char *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[strlen(zTbl)+1];
  pVtab->db = db;
  memcpy(pVtab->zTbl, zTbl, strlen(zTbl));
  memcpy(pVtab->zDb, zDb, strlen(zDb));
  *ppVtab = &pVtab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  戻る SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

static 整数 fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  戻る SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
static 整数 fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(sizeof(fs_cursor));
  *ppCursor = &pCur->base;
  戻る SQLITE_OK;
}

/*
** Close a fs cursor.
*/
static 整数 fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  戻る SQLITE_OK;
}

static 整数 fsNext(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  整数 rc;

  rc = sqlite3_step(pCur->pStmt);
  もし( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;

  戻る rc;
}

static 整数 fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  整数 idxNum, const char *idxStr,
  整数 argc, sqlite3_value **argv
){
  整数 rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  assert( (idxNum==0 && argc==0) || (idxNum==1 && argc==1) );
  もし( idxNum==1 ){
    char *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    もし( !zStmt ) 戻る SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    もし( rc==SQLITE_OK ){
      sqlite3_bind_value(pCur->pStmt, 1, argv[0]);
    }
  }否则{
    char *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    もし( !zStmt ) 戻る SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  もし( rc==SQLITE_OK ){
    rc = fsNext(pVtabCursor); 
  }
  戻る rc;
}

static 整数 fsColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, 整数 i){
  fs_cursor *pCur = (fs_cursor*)cur;

  assert( i==0 || i==1 || i==2 );
  もし( i==0 ){
    sqlite3_result_value(ctx, sqlite3_column_value(pCur->pStmt, 0));
  }否则{
    const char *zFile = (const char *)sqlite3_column_text(pCur->pStmt, 1);
    struct stat sbuf;
    整数 fd;

    整数 n;
    fd = open(zFile, O_RDONLY);
    もし( fd<0 ) 戻る SQLITE_IOERR;
    fstat(fd, &sbuf);

    もし( sbuf.st_size>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_size*2;
      char *zNew;
      もし( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      もし( zNew==0 ){
        close(fd);
        戻る SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (整数)read(fd, pCur->zBuf, sbuf.st_size);
    close(fd);
    もし( n!=sbuf.st_size ) 戻る SQLITE_ERROR;
    pCur->nBuf = sbuf.st_size;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_result_text(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  戻る SQLITE_OK;
}

static 整数 fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_column_int64(pCur->pStmt, 0);
  戻る SQLITE_OK;
}

static 整数 fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  戻る (sqlite3_data_count(pCur->pStmt)==0);
}

static 整数 fsBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  整数 ii;

  ために(ii=0; ii<pIdxInfo->nConstraint; ii++){
    struct sqlite3_index_constraint const *pCons = &pIdxInfo->aConstraint[ii];
    もし( pCons->iColumn<0 && pCons->usable
           && pCons->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      struct sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argvIndex = 1;
      pIdxInfo->idxNum = 1;
      pIdxInfo->estimatedCost = 1.0;
      break;
    }
  }

  戻る SQLITE_OK;
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
extern 整数 getDbPointer(Tcl_Interp *interp, const char *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
static 整数 SQLITE_TCLAPI register_fs_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  整数 objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  もし( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    戻る TCL_ERROR;
  }
  もし( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) 戻る TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (void *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#endif
  戻る TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
整数 Sqlitetestfs_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
     void *clientData;
  } aObjCmd[] = {
     { "register_fs_module",   register_fs_module, 0 },
  };
  整数 i;
  ために(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  戻る TCL_OK;
}
