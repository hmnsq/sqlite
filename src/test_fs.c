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
#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <sys/types.h>
#ضمن <sys/stat.h>
#ضمن <fcntl.h>

#إذا !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#وإلا
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
# endif
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE

تعريفنوع بنية fs_vtab fs_vtab;
تعريفنوع بنية fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
بنية fs_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
  حرف *zDb;                      /* Name of db containing zTbl */
  حرف *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
بنية fs_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
  حرف *zBuf;
  صحيح nBuf;
  صحيح nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
تعريفنوع بنية FsdirVtab FsdirVtab;
تعريفنوع بنية FsdirCsr FsdirCsr;
بنية FsdirVtab {
  sqlite3_vtab base;
};

بنية FsdirCsr {
  sqlite3_vtab_cursor base;
  حرف *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  بنية dirent *pEntry;
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
ساكن صحيح fsdirConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  FsdirVtab *pTab;

  إذا( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    أرجع SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(حجم(FsdirVtab));
  إذا( !pTab ) أرجع SQLITE_NOMEM;
  memset(pTab, 0, حجم(FsdirVtab));

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  أرجع SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
ساكن صحيح fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  أرجع SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
ساكن صحيح fsdirBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  صحيح ii;

  pIdxInfo->estimatedCost = 1000000000.0;

  لكل(ii=0; ii<pIdxInfo->nConstraint; ii++){
    بنية sqlite3_index_constraint ثابت *p = &pIdxInfo->aConstraint[ii];
    إذا( p->iColumn==0 && p->usable && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      بنية sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_رقم = 1;
      pIdxInfo->estimatedCost = 1.0;
      اكسر;
    }
  }

  أرجع SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
ساكن صحيح fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(حجم(FsdirCsr)+256);
  إذا( pCur==0 ) أرجع SQLITE_NOMEM;
  memset(pCur, 0, حجم(FsdirCsr));
  *ppCursor = &pCur->base;
  أرجع SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
ساكن صحيح fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  إذا( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  أرجع SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
ساكن صحيح fsdirNext(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  إذا( pCsr->pDir ){
    pCsr->pEntry = readdir(pCsr->pDir);
    إذا( pCsr->pEntry==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  أرجع SQLITE_OK;
}

/*
** xFilter method implementation.
*/
ساكن صحيح fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحيح idx_رقم, ثابت حرف *idxStr,
  صحيح argc, sqlite3_قيمة **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  ثابت حرف *zDir;
  صحيح nDir;


  إذا( idx_رقم!=1 || argc!=1 ){
    أرجع SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  إذا( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (ثابت حرف*)sqlite3_قيمة_text(argv[0]);
  nDir = sqlite3_قيمة_bytes(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  إذا( pCsr->zDir==0 ) أرجع SQLITE_NOMEM;
  memcpy(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  أرجع fsdirNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
ساكن صحيح fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  أرجع pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
ساكن صحيح fsdirColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحيح i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  بدل( i ){
    حالة 0: /* dir */
      sqlite3_result_text(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      اكسر;

    حالة 1: /* name */
      sqlite3_result_text(ctx, pCsr->pEntry->d_name, -1, SQLITE_TRANSIENT);
      اكسر;

    افتراضي:
      أكد( 0 );
  }

  أرجع SQLITE_OK;
}

/*
** xRowid method implementation.
*/
ساكن صحيح fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  أرجع SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
تعريفنوع بنية FstreeVtab FstreeVtab;
تعريفنوع بنية FstreeCsr FstreeCsr;
بنية FstreeVtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

بنية FstreeCsr {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  صحيح fd;                         /* File descriptor open on current path */
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
ساكن صحيح fstreeConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  FstreeVtab *pTab;

  إذا( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    أرجع SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(حجم(FstreeVtab));
  إذا( !pTab ) أرجع SQLITE_NOMEM;
  memset(pTab, 0, حجم(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  أرجع SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
ساكن صحيح fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  أرجع SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
ساكن صحيح fstreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  صحيح ii;

  لكل(ii=0; ii<pIdxInfo->nConstraint; ii++){
    بنية sqlite3_index_constraint ثابت *p = &pIdxInfo->aConstraint[ii];
    إذا( p->iColumn==0 && p->usable && (
          p->op==SQLITE_INDEX_CONSTRAINT_GLOB
       || p->op==SQLITE_INDEX_CONSTRAINT_LIKE
       || p->op==SQLITE_INDEX_CONSTRAINT_EQ
    )){
      بنية sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_رقم = p->op;
      pUsage->argvIndex = 1;
      pIdxInfo->estimatedCost = 100000.0;
      أرجع SQLITE_OK;
    }
  }

  pIdxInfo->estimatedCost = 1000000000.0;
  أرجع SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
ساكن صحيح fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(حجم(FstreeCsr));
  إذا( pCur==0 ) أرجع SQLITE_NOMEM;
  memset(pCur, 0, حجم(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->base;
  أرجع SQLITE_OK;
}

ساكن فراغ fstreeCloseFd(FstreeCsr *pCsr){
  إذا( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
ساكن صحيح fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  أرجع SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
ساكن صحيح fstreeNext(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  صحيح rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_step(pCsr->pStmt);
  إذا( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }وإلا{
    rc = SQLITE_OK;
    pCsr->fd = open((ثابت حرف*)sqlite3_column_text(pCsr->pStmt, 0), O_RDONLY);
  }

  أرجع rc;
}

/*
** xFilter method implementation.
*/
ساكن صحيح fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحيح idx_رقم, ثابت حرف *idxStr,
  صحيح argc, sqlite3_قيمة **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->base.pVtab);
  صحيح rc;
  ثابت حرف *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  حرف *z_جذر;
  صحيح n_جذر;
  حرف *zPrefix;
  صحيح nPrefix;
  ثابت حرف *zDir;
  صحيح nDir;
  حرف aWild[2] = { '\0', '\0' };

#ifdef _WIN32
  ثابت حرف *zDrive = getenv("fstreeDrive");
  إذا( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  z_جذر = sqlite3_mprintf("%s%c", zDrive, '/');
  n_جذر = sqlite3Strlen30(z_جذر);
  zPrefix = sqlite3_mprintf("%s", zDrive);
  nPrefix = sqlite3Strlen30(zPrefix);
#وإلا
  z_جذر = "/";
  n_جذر = 1;
  zPrefix = "";
  nPrefix = 0;
#endif

  zDir = z_جذر;
  nDir = n_جذر;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  إذا( rc!=SQLITE_OK ) أرجع rc;

  إذا( idx_رقم ){
    ثابت حرف *zQuery = (ثابت حرف*)sqlite3_قيمة_text(argv[0]);
    بدل( idx_رقم ){
      حالة SQLITE_INDEX_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        اكسر;
      حالة SQLITE_INDEX_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        اكسر;
    }

    إذا( sqlite3_strnicmp(zQuery, zPrefix, nPrefix)==0 ){
      صحيح i;
      لكل(i=nPrefix; zQuery[i]; i++){
        إذا( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) اكسر;
        إذا( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  إذا( nDir==0 ) nDir = 1;

  sqlite3_bind_text(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 2, z_جذر, n_جذر, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 3, zPrefix, nPrefix, SQLITE_TRANSIENT);

#ifdef _WIN32
  sqlite3_free(zPrefix);
  sqlite3_free(z_جذر);
#endif

  أرجع fstreeNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
ساكن صحيح fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  أرجع pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
ساكن صحيح fstreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحيح i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  إذا( i==0 ){      /* path */
    sqlite3_result_قيمة(ctx, sqlite3_column_قيمة(pCsr->pStmt, 0));
  }وإلا{
    بنية stat sBuf;
    fstat(pCsr->fd, &sBuf);

    إذا( S_ISREG(sBuf.st_mode) ){
      إذا( i==1 ){
        sqlite3_result_int64(ctx, sBuf.st_size);
      }وإلا{
        صحيح nRead;
        حرف *aBuf = sqlite3_malloc(sBuf.st_mode+1);
        إذا( !aBuf ) أرجع SQLITE_NOMEM;
        nRead = اقرأ(pCsr->fd, aBuf, sBuf.st_mode);
        إذا( nRead!=sBuf.st_mode ){
          أرجع SQLITE_IOERR;
        }
        sqlite3_result_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  أرجع SQLITE_OK;
}

/*
** xRowid method implementation.
*/
ساكن صحيح fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  أرجع SQLITE_OK;
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
ساكن صحيح fsConnect(
  sqlite3 *db,
  فراغ *pAux,
  صحيح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  fs_vtab *pVtab;
  صحيح nByte;
  ثابت حرف *zTbl;
  ثابت حرف *zDb = argv[1];

  إذا( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    أرجع SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = حجم(fs_vtab) + (صحيح)strlen(zTbl) + 1 + (صحيح)strlen(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  إذا( !pVtab ) أرجع SQLITE_NOMEM;

  pVtab->zTbl = (حرف *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[strlen(zTbl)+1];
  pVtab->db = db;
  memcpy(pVtab->zTbl, zTbl, strlen(zTbl));
  memcpy(pVtab->zDb, zDb, strlen(zDb));
  *ppVtab = &pVtab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  أرجع SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

ساكن صحيح fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  أرجع SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
ساكن صحيح fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(حجم(fs_cursor));
  *ppCursor = &pCur->base;
  أرجع SQLITE_OK;
}

/*
** Close a fs cursor.
*/
ساكن صحيح fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  أرجع SQLITE_OK;
}

ساكن صحيح fsNext(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  صحيح rc;

  rc = sqlite3_step(pCur->pStmt);
  إذا( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;

  أرجع rc;
}

ساكن صحيح fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحيح idx_رقم, ثابت حرف *idxStr,
  صحيح argc, sqlite3_قيمة **argv
){
  صحيح rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  أكد( (idx_رقم==0 && argc==0) || (idx_رقم==1 && argc==1) );
  إذا( idx_رقم==1 ){
    حرف *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    إذا( !zStmt ) أرجع SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    إذا( rc==SQLITE_OK ){
      sqlite3_bind_قيمة(pCur->pStmt, 1, argv[0]);
    }
  }وإلا{
    حرف *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    إذا( !zStmt ) أرجع SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  إذا( rc==SQLITE_OK ){
    rc = fsNext(pVtabCursor); 
  }
  أرجع rc;
}

ساكن صحيح fsColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحيح i){
  fs_cursor *pCur = (fs_cursor*)cur;

  أكد( i==0 || i==1 || i==2 );
  إذا( i==0 ){
    sqlite3_result_قيمة(ctx, sqlite3_column_قيمة(pCur->pStmt, 0));
  }وإلا{
    ثابت حرف *zFile = (ثابت حرف *)sqlite3_column_text(pCur->pStmt, 1);
    بنية stat sbuf;
    صحيح fd;

    صحيح n;
    fd = open(zFile, O_RDONLY);
    إذا( fd<0 ) أرجع SQLITE_IOERR;
    fstat(fd, &sbuf);

    إذا( sbuf.st_size>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_size*2;
      حرف *zNew;
      إذا( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      إذا( zNew==0 ){
        close(fd);
        أرجع SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (صحيح)اقرأ(fd, pCur->zBuf, sbuf.st_size);
    close(fd);
    إذا( n!=sbuf.st_size ) أرجع SQLITE_ERROR;
    pCur->nBuf = sbuf.st_size;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_result_text(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  أرجع SQLITE_OK;
}

ساكن صحيح fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_column_int64(pCur->pStmt, 0);
  أرجع SQLITE_OK;
}

ساكن صحيح fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  أرجع (sqlite3_data_عدد(pCur->pStmt)==0);
}

ساكن صحيح fsBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  صحيح ii;

  لكل(ii=0; ii<pIdxInfo->nConstraint; ii++){
    بنية sqlite3_index_constraint ثابت *pCons = &pIdxInfo->aConstraint[ii];
    إذا( pCons->iColumn<0 && pCons->usable
           && pCons->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      بنية sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_رقم = 1;
      pIdxInfo->estimatedCost = 1.0;
      اكسر;
    }
  }

  أرجع SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ساكن sqlite3_module fsModule = {
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

ساكن sqlite3_module fsdirModule = {
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

ساكن sqlite3_module fstreeModule = {
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
خارجي صحيح getDbPointer(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ساكن صحيح SQLITE_TCLAPI register_fs_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  إذا( objc!=2 ){
    Tcl_Wrong_رقم_وسائط(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (فراغ *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#endif
  أرجع TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetestfs_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     فراغ *clientData;
  } aObjCmd[] = {
     { "register_fs_module",   register_fs_module, 0 },
  };
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  أرجع TCL_OK;
}
