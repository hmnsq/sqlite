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
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/types.h>
#شامل <sys/stat.h>
#شامل <fcntl.h>

#اگر !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#وگرنه
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
# endif
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE

تعریفنوع ساختار fs_vtab fs_vtab;
تعریفنوع ساختار fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
ساختار fs_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
  نویسه *zDb;                      /* Name of db containing zTbl */
  نویسه *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
ساختار fs_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
  نویسه *zBuf;
  صحیح nBuf;
  صحیح nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
تعریفنوع ساختار FsdirVtab FsdirVtab;
تعریفنوع ساختار FsdirCsr FsdirCsr;
ساختار FsdirVtab {
  sqlite3_vtab base;
};

ساختار FsdirCsr {
  sqlite3_vtab_cursor base;
  نویسه *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  ساختار dirent *pEntry;
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
ایستا صحیح fsdirConnect(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  FsdirVtab *pTab;

  اگر( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    بازگشت SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(اندازه(FsdirVtab));
  اگر( !pTab ) بازگشت SQLITE_NOMEM;
  memset(pTab, 0, اندازه(FsdirVtab));

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  بازگشت SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
ایستا صحیح fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  بازگشت SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
ایستا صحیح fsdirBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  صحیح ii;

  pIdxInfo->estimatedCost = 1000000000.0;

  برای(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ساختار sqlite3_index_constraint ثابت *p = &pIdxInfo->aConstraint[ii];
    اگر( p->iColumn==0 && p->usable && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      ساختار sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_رقم = 1;
      pIdxInfo->estimatedCost = 1.0;
      بشکن;
    }
  }

  بازگشت SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
ایستا صحیح fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(اندازه(FsdirCsr)+256);
  اگر( pCur==0 ) بازگشت SQLITE_NOMEM;
  memset(pCur, 0, اندازه(FsdirCsr));
  *ppCursor = &pCur->base;
  بازگشت SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
ایستا صحیح fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  اگر( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  بازگشت SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
ایستا صحیح fsdirNext(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  اگر( pCsr->pDir ){
    pCsr->pEntry = readdir(pCsr->pDir);
    اگر( pCsr->pEntry==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  بازگشت SQLITE_OK;
}

/*
** xFilter method implementation.
*/
ایستا صحیح fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_رقم, ثابت نویسه *idxStr,
  صحیح argc, sqlite3_مقدار **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  ثابت نویسه *zDir;
  صحیح nDir;


  اگر( idx_رقم!=1 || argc!=1 ){
    بازگشت SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  اگر( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
  nDir = sqlite3_مقدار_bytes(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  اگر( pCsr->zDir==0 ) بازگشت SQLITE_NOMEM;
  memcpy(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  بازگشت fsdirNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
ایستا صحیح fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  بازگشت pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
ایستا صحیح fsdirColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحیح i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  انتخاب( i ){
    حالت 0: /* dir */
      sqlite3_result_text(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      بشکن;

    حالت 1: /* name */
      sqlite3_result_text(ctx, pCsr->pEntry->d_name, -1, SQLITE_TRANSIENT);
      بشکن;

    پیشفرض:
      أكد( 0 );
  }

  بازگشت SQLITE_OK;
}

/*
** xRowid method implementation.
*/
ایستا صحیح fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  بازگشت SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
تعریفنوع ساختار FstreeVtab FstreeVtab;
تعریفنوع ساختار FstreeCsr FstreeCsr;
ساختار FstreeVtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

ساختار FstreeCsr {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  صحیح fd;                         /* File descriptor open on current path */
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
ایستا صحیح fstreeConnect(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  FstreeVtab *pTab;

  اگر( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    بازگشت SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(اندازه(FstreeVtab));
  اگر( !pTab ) بازگشت SQLITE_NOMEM;
  memset(pTab, 0, اندازه(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  بازگشت SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
ایستا صحیح fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  بازگشت SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
ایستا صحیح fstreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  صحیح ii;

  برای(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ساختار sqlite3_index_constraint ثابت *p = &pIdxInfo->aConstraint[ii];
    اگر( p->iColumn==0 && p->usable && (
          p->op==SQLITE_INDEX_CONSTRAINT_GLOB
       || p->op==SQLITE_INDEX_CONSTRAINT_LIKE
       || p->op==SQLITE_INDEX_CONSTRAINT_EQ
    )){
      ساختار sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_رقم = p->op;
      pUsage->argvIndex = 1;
      pIdxInfo->estimatedCost = 100000.0;
      بازگشت SQLITE_OK;
    }
  }

  pIdxInfo->estimatedCost = 1000000000.0;
  بازگشت SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
ایستا صحیح fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(اندازه(FstreeCsr));
  اگر( pCur==0 ) بازگشت SQLITE_NOMEM;
  memset(pCur, 0, اندازه(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->base;
  بازگشت SQLITE_OK;
}

ایستا تهی fstreeCloseFd(FstreeCsr *pCsr){
  اگر( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
ایستا صحیح fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  بازگشت SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
ایستا صحیح fstreeNext(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  صحیح rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_step(pCsr->pStmt);
  اگر( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }وگرنه{
    rc = SQLITE_OK;
    pCsr->fd = open((ثابت نویسه*)sqlite3_column_text(pCsr->pStmt, 0), O_RDONLY);
  }

  بازگشت rc;
}

/*
** xFilter method implementation.
*/
ایستا صحیح fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_رقم, ثابت نویسه *idxStr,
  صحیح argc, sqlite3_مقدار **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->base.pVtab);
  صحیح rc;
  ثابت نویسه *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  نویسه *z_ریشه;
  صحیح n_ریشه;
  نویسه *zPrefix;
  صحیح nPrefix;
  ثابت نویسه *zDir;
  صحیح nDir;
  نویسه aWild[2] = { '\0', '\0' };

#ifdef _WIN32
  ثابت نویسه *zDrive = getenv("fstreeDrive");
  اگر( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  z_ریشه = sqlite3_mprintf("%s%c", zDrive, '/');
  n_ریشه = sqlite3Strlen30(z_ریشه);
  zPrefix = sqlite3_mprintf("%s", zDrive);
  nPrefix = sqlite3Strlen30(zPrefix);
#وگرنه
  z_ریشه = "/";
  n_ریشه = 1;
  zPrefix = "";
  nPrefix = 0;
#endif

  zDir = z_ریشه;
  nDir = n_ریشه;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  اگر( rc!=SQLITE_OK ) بازگشت rc;

  اگر( idx_رقم ){
    ثابت نویسه *zQuery = (ثابت نویسه*)sqlite3_مقدار_text(argv[0]);
    انتخاب( idx_رقم ){
      حالت SQLITE_INDEX_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        بشکن;
      حالت SQLITE_INDEX_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        بشکن;
    }

    اگر( sqlite3_strnicmp(zQuery, zPrefix, nPrefix)==0 ){
      صحیح i;
      برای(i=nPrefix; zQuery[i]; i++){
        اگر( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) بشکن;
        اگر( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  اگر( nDir==0 ) nDir = 1;

  sqlite3_bind_text(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 2, z_ریشه, n_ریشه, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 3, zPrefix, nPrefix, SQLITE_TRANSIENT);

#ifdef _WIN32
  sqlite3_free(zPrefix);
  sqlite3_free(z_ریشه);
#endif

  بازگشت fstreeNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
ایستا صحیح fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  بازگشت pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
ایستا صحیح fstreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحیح i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  اگر( i==0 ){      /* path */
    sqlite3_result_مقدار(ctx, sqlite3_column_مقدار(pCsr->pStmt, 0));
  }وگرنه{
    ساختار stat sBuf;
    fstat(pCsr->fd, &sBuf);

    اگر( S_ISREG(sBuf.st_mode) ){
      اگر( i==1 ){
        sqlite3_result_int64(ctx, sBuf.st_size);
      }وگرنه{
        صحیح nRead;
        نویسه *aBuf = sqlite3_malloc(sBuf.st_mode+1);
        اگر( !aBuf ) بازگشت SQLITE_NOMEM;
        nRead = اقرأ(pCsr->fd, aBuf, sBuf.st_mode);
        اگر( nRead!=sBuf.st_mode ){
          بازگشت SQLITE_IOERR;
        }
        sqlite3_result_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  بازگشت SQLITE_OK;
}

/*
** xRowid method implementation.
*/
ایستا صحیح fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  بازگشت SQLITE_OK;
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
ایستا صحیح fsConnect(
  sqlite3 *db,
  تهی *pAux,
  صحیح argc, ثابت نویسه *ثابت*argv,
  sqlite3_vtab **ppVtab,
  نویسه **pzErr
){
  fs_vtab *pVtab;
  صحیح nByte;
  ثابت نویسه *zTbl;
  ثابت نویسه *zDb = argv[1];

  اگر( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    بازگشت SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = اندازه(fs_vtab) + (صحیح)strlen(zTbl) + 1 + (صحیح)strlen(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  اگر( !pVtab ) بازگشت SQLITE_NOMEM;

  pVtab->zTbl = (نویسه *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[strlen(zTbl)+1];
  pVtab->db = db;
  memcpy(pVtab->zTbl, zTbl, strlen(zTbl));
  memcpy(pVtab->zDb, zDb, strlen(zDb));
  *ppVtab = &pVtab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  بازگشت SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

ایستا صحیح fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  بازگشت SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
ایستا صحیح fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(اندازه(fs_cursor));
  *ppCursor = &pCur->base;
  بازگشت SQLITE_OK;
}

/*
** Close a fs cursor.
*/
ایستا صحیح fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  بازگشت SQLITE_OK;
}

ایستا صحیح fsNext(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  صحیح rc;

  rc = sqlite3_step(pCur->pStmt);
  اگر( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;

  بازگشت rc;
}

ایستا صحیح fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_رقم, ثابت نویسه *idxStr,
  صحیح argc, sqlite3_مقدار **argv
){
  صحیح rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  أكد( (idx_رقم==0 && argc==0) || (idx_رقم==1 && argc==1) );
  اگر( idx_رقم==1 ){
    نویسه *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    اگر( !zStmt ) بازگشت SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    اگر( rc==SQLITE_OK ){
      sqlite3_bind_مقدار(pCur->pStmt, 1, argv[0]);
    }
  }وگرنه{
    نویسه *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    اگر( !zStmt ) بازگشت SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  اگر( rc==SQLITE_OK ){
    rc = fsNext(pVtabCursor); 
  }
  بازگشت rc;
}

ایستا صحیح fsColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحیح i){
  fs_cursor *pCur = (fs_cursor*)cur;

  أكد( i==0 || i==1 || i==2 );
  اگر( i==0 ){
    sqlite3_result_مقدار(ctx, sqlite3_column_مقدار(pCur->pStmt, 0));
  }وگرنه{
    ثابت نویسه *zFile = (ثابت نویسه *)sqlite3_column_text(pCur->pStmt, 1);
    ساختار stat sbuf;
    صحیح fd;

    صحیح n;
    fd = open(zFile, O_RDONLY);
    اگر( fd<0 ) بازگشت SQLITE_IOERR;
    fstat(fd, &sbuf);

    اگر( sbuf.st_size>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_size*2;
      نویسه *zNew;
      اگر( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      اگر( zNew==0 ){
        close(fd);
        بازگشت SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (صحیح)اقرأ(fd, pCur->zBuf, sbuf.st_size);
    close(fd);
    اگر( n!=sbuf.st_size ) بازگشت SQLITE_ERROR;
    pCur->nBuf = sbuf.st_size;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_result_text(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  بازگشت SQLITE_OK;
}

ایستا صحیح fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_column_int64(pCur->pStmt, 0);
  بازگشت SQLITE_OK;
}

ایستا صحیح fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  بازگشت (sqlite3_data_شمار(pCur->pStmt)==0);
}

ایستا صحیح fsBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  صحیح ii;

  برای(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ساختار sqlite3_index_constraint ثابت *pCons = &pIdxInfo->aConstraint[ii];
    اگر( pCons->iColumn<0 && pCons->usable
           && pCons->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      ساختار sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_رقم = 1;
      pIdxInfo->estimatedCost = 1.0;
      بشکن;
    }
  }

  بازگشت SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ایستا sqlite3_module fsModule = {
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

ایستا sqlite3_module fsdirModule = {
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

ایستا sqlite3_module fstreeModule = {
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
بیرونی صحیح getDbPointer(Tcl_Interp *interp, ثابت نویسه *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ایستا صحیح SQLITE_TCLAPI register_fs_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DB");
    بازگشت TCL_ERROR;
  }
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (تهی *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#endif
  بازگشت TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetestfs_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ایستا ساختار {
     نویسه *zName;
     Tcl_ObjCmdProc *xProc;
     تهی *clientData;
  } aObjCmd[] = {
     { "register_fs_module",   register_fs_module, 0 },
  };
  صحیح i;
  برای(i=0; i<اندازه(aObjCmd)/اندازه(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  بازگشت TCL_OK;
}
