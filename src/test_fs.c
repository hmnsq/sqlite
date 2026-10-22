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

#если !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#иначе
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
  целое nBuf;
  целое nAlloc;
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
static целое fsdirConnect(
  sqlite3 *db,
  void *pAux,
  целое argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  FsdirVtab *pTab;

  если( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    вернуть SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(sizeof(FsdirVtab));
  если( !pTab ) вернуть SQLITE_NOMEM;
  memset(pTab, 0, sizeof(FsdirVtab));

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  вернуть SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
static целое fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  вернуть SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
static целое fsdirBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  целое ii;

  pIdxInfo->estimatedCost = 1000000000.0;

  для(ii=0; ii<pIdxInfo->nConstraint; ii++){
    struct sqlite3_index_constraint const *p = &pIdxInfo->aConstraint[ii];
    если( p->iColumn==0 && p->usable && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      struct sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argvIndex = 1;
      pIdxInfo->idxNum = 1;
      pIdxInfo->estimatedCost = 1.0;
      break;
    }
  }

  вернуть SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
static целое fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(sizeof(FsdirCsr)+256);
  если( pCur==0 ) вернуть SQLITE_NOMEM;
  memset(pCur, 0, sizeof(FsdirCsr));
  *ppCursor = &pCur->base;
  вернуть SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
static целое fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  если( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  вернуть SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
static целое fsdirNext(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  если( pCsr->pDir ){
    pCsr->pEntry = readdir(pCsr->pDir);
    если( pCsr->pEntry==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  вернуть SQLITE_OK;
}

/*
** xFilter method implementation.
*/
static целое fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  целое idxNum, const char *idxStr,
  целое argc, sqlite3_value **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  const char *zDir;
  целое nDir;


  если( idxNum!=1 || argc!=1 ){
    вернуть SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  если( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (const char*)sqlite3_value_text(argv[0]);
  nDir = sqlite3_value_bytes(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  если( pCsr->zDir==0 ) вернуть SQLITE_NOMEM;
  memcpy(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  вернуть fsdirNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
static целое fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  вернуть pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
static целое fsdirColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, целое i){
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

  вернуть SQLITE_OK;
}

/*
** xRowid method implementation.
*/
static целое fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  вернуть SQLITE_OK;
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
  целое fd;                         /* File descriptor open on current path */
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
static целое fstreeConnect(
  sqlite3 *db,
  void *pAux,
  целое argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  FstreeVtab *pTab;

  если( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    вернуть SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(sizeof(FstreeVtab));
  если( !pTab ) вернуть SQLITE_NOMEM;
  memset(pTab, 0, sizeof(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  вернуть SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
static целое fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  вернуть SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
static целое fstreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  целое ii;

  для(ii=0; ii<pIdxInfo->nConstraint; ii++){
    struct sqlite3_index_constraint const *p = &pIdxInfo->aConstraint[ii];
    если( p->iColumn==0 && p->usable && (
          p->op==SQLITE_INDEX_CONSTRAINT_GLOB
       || p->op==SQLITE_INDEX_CONSTRAINT_LIKE
       || p->op==SQLITE_INDEX_CONSTRAINT_EQ
    )){
      struct sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idxNum = p->op;
      pUsage->argvIndex = 1;
      pIdxInfo->estimatedCost = 100000.0;
      вернуть SQLITE_OK;
    }
  }

  pIdxInfo->estimatedCost = 1000000000.0;
  вернуть SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
static целое fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(sizeof(FstreeCsr));
  если( pCur==0 ) вернуть SQLITE_NOMEM;
  memset(pCur, 0, sizeof(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->base;
  вернуть SQLITE_OK;
}

static void fstreeCloseFd(FstreeCsr *pCsr){
  если( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
static целое fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  вернуть SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
static целое fstreeNext(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  целое rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_step(pCsr->pStmt);
  если( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }иначе{
    rc = SQLITE_OK;
    pCsr->fd = open((const char*)sqlite3_column_text(pCsr->pStmt, 0), O_RDONLY);
  }

  вернуть rc;
}

/*
** xFilter method implementation.
*/
static целое fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  целое idxNum, const char *idxStr,
  целое argc, sqlite3_value **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->base.pVtab);
  целое rc;
  const char *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  char *zRoot;
  целое nRoot;
  char *zPrefix;
  целое nPrefix;
  const char *zDir;
  целое nDir;
  char aWild[2] = { '\0', '\0' };

#ifdef _WIN32
  const char *zDrive = getenv("fstreeDrive");
  если( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  zRoot = sqlite3_mprintf("%s%c", zDrive, '/');
  nRoot = sqlite3Strlen30(zRoot);
  zPrefix = sqlite3_mprintf("%s", zDrive);
  nPrefix = sqlite3Strlen30(zPrefix);
#иначе
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
  если( rc!=SQLITE_OK ) вернуть rc;

  если( idxNum ){
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

    если( sqlite3_strnicmp(zQuery, zPrefix, nPrefix)==0 ){
      целое i;
      для(i=nPrefix; zQuery[i]; i++){
        если( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) break;
        если( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  если( nDir==0 ) nDir = 1;

  sqlite3_bind_text(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 2, zRoot, nRoot, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 3, zPrefix, nPrefix, SQLITE_TRANSIENT);

#ifdef _WIN32
  sqlite3_free(zPrefix);
  sqlite3_free(zRoot);
#endif

  вернуть fstreeNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
static целое fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  вернуть pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
static целое fstreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, целое i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  если( i==0 ){      /* path */
    sqlite3_result_value(ctx, sqlite3_column_value(pCsr->pStmt, 0));
  }иначе{
    struct stat sBuf;
    fstat(pCsr->fd, &sBuf);

    если( S_ISREG(sBuf.st_mode) ){
      если( i==1 ){
        sqlite3_result_int64(ctx, sBuf.st_size);
      }иначе{
        целое nRead;
        char *aBuf = sqlite3_malloc(sBuf.st_mode+1);
        если( !aBuf ) вернуть SQLITE_NOMEM;
        nRead = read(pCsr->fd, aBuf, sBuf.st_mode);
        если( nRead!=sBuf.st_mode ){
          вернуть SQLITE_IOERR;
        }
        sqlite3_result_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  вернуть SQLITE_OK;
}

/*
** xRowid method implementation.
*/
static целое fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  вернуть SQLITE_OK;
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
static целое fsConnect(
  sqlite3 *db,
  void *pAux,
  целое argc, const char *const*argv,
  sqlite3_vtab **ppVtab,
  char **pzErr
){
  fs_vtab *pVtab;
  целое nByte;
  const char *zTbl;
  const char *zDb = argv[1];

  если( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    вернуть SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = sizeof(fs_vtab) + (целое)strlen(zTbl) + 1 + (целое)strlen(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  если( !pVtab ) вернуть SQLITE_NOMEM;

  pVtab->zTbl = (char *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[strlen(zTbl)+1];
  pVtab->db = db;
  memcpy(pVtab->zTbl, zTbl, strlen(zTbl));
  memcpy(pVtab->zDb, zDb, strlen(zDb));
  *ppVtab = &pVtab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  вернуть SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

static целое fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  вернуть SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
static целое fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(sizeof(fs_cursor));
  *ppCursor = &pCur->base;
  вернуть SQLITE_OK;
}

/*
** Close a fs cursor.
*/
static целое fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  вернуть SQLITE_OK;
}

static целое fsNext(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  целое rc;

  rc = sqlite3_step(pCur->pStmt);
  если( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;

  вернуть rc;
}

static целое fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  целое idxNum, const char *idxStr,
  целое argc, sqlite3_value **argv
){
  целое rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  assert( (idxNum==0 && argc==0) || (idxNum==1 && argc==1) );
  если( idxNum==1 ){
    char *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    если( !zStmt ) вернуть SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    если( rc==SQLITE_OK ){
      sqlite3_bind_value(pCur->pStmt, 1, argv[0]);
    }
  }иначе{
    char *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    если( !zStmt ) вернуть SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  если( rc==SQLITE_OK ){
    rc = fsNext(pVtabCursor); 
  }
  вернуть rc;
}

static целое fsColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, целое i){
  fs_cursor *pCur = (fs_cursor*)cur;

  assert( i==0 || i==1 || i==2 );
  если( i==0 ){
    sqlite3_result_value(ctx, sqlite3_column_value(pCur->pStmt, 0));
  }иначе{
    const char *zFile = (const char *)sqlite3_column_text(pCur->pStmt, 1);
    struct stat sbuf;
    целое fd;

    целое n;
    fd = open(zFile, O_RDONLY);
    если( fd<0 ) вернуть SQLITE_IOERR;
    fstat(fd, &sbuf);

    если( sbuf.st_size>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_size*2;
      char *zNew;
      если( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      если( zNew==0 ){
        close(fd);
        вернуть SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (целое)read(fd, pCur->zBuf, sbuf.st_size);
    close(fd);
    если( n!=sbuf.st_size ) вернуть SQLITE_ERROR;
    pCur->nBuf = sbuf.st_size;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_result_text(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  вернуть SQLITE_OK;
}

static целое fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_column_int64(pCur->pStmt, 0);
  вернуть SQLITE_OK;
}

static целое fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  вернуть (sqlite3_data_count(pCur->pStmt)==0);
}

static целое fsBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  целое ii;

  для(ii=0; ii<pIdxInfo->nConstraint; ii++){
    struct sqlite3_index_constraint const *pCons = &pIdxInfo->aConstraint[ii];
    если( pCons->iColumn<0 && pCons->usable
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

  вернуть SQLITE_OK;
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
extern целое getDbPointer(Tcl_Interp *interp, const char *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
static целое SQLITE_TCLAPI register_fs_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  целое objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  если( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    вернуть TCL_ERROR;
  }
  если( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) вернуть TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (void *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#endif
  вернуть TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
целое Sqlitetestfs_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  static struct {
     char *zName;
     Tcl_ObjCmdProc *xProc;
     void *clientData;
  } aObjCmd[] = {
     { "register_fs_module",   register_fs_module, 0 },
  };
  целое i;
  для(i=0; i<sizeof(aObjCmd)/sizeof(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  вернуть TCL_OK;
}
