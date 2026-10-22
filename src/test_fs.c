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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <fcntl.h>

#যদি !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#অন্যথা
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
# endif
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE

প্রকারলেখ কাঠামো fs_vtab fs_vtab;
প্রকারলেখ কাঠামো fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
কাঠামো fs_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
  অক্ষর *zDb;                      /* Name of db containing zTbl */
  অক্ষর *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
কাঠামো fs_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
  অক্ষর *zBuf;
  পূর্ণসংখ্যা nBuf;
  পূর্ণসংখ্যা nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
প্রকারলেখ কাঠামো FsdirVtab FsdirVtab;
প্রকারলেখ কাঠামো FsdirCsr FsdirCsr;
কাঠামো FsdirVtab {
  sqlite3_vtab base;
};

কাঠামো FsdirCsr {
  sqlite3_vtab_cursor base;
  অক্ষর *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  কাঠামো dirent *pEntry;
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
স্থির পূর্ণসংখ্যা fsdirConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  FsdirVtab *pTab;

  যদি( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ফেরত SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(আকার(FsdirVtab));
  যদি( !pTab ) ফেরত SQLITE_NOMEM;
  memset(pTab, 0, আকার(FsdirVtab));

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  ফেরত SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
স্থির পূর্ণসংখ্যা fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ফেরত SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
স্থির পূর্ণসংখ্যা fsdir_Best_সূচক(sqlite3_vtab *tab, sqlite3_সূচক_info *pIdxInfo){
  পূর্ণসংখ্যা ii;

  pIdxInfo->estimatedCost = 1000000000.0;

  জন্য(ii=0; ii<pIdxInfo->nConstraint; ii++){
    কাঠামো sqlite3_সূচক_constraint ধ্রুবক *p = &pIdxInfo->aConstraint[ii];
    যদি( p->iColumn==0 && p->usable && p->op==SQLITE_সূচক_CONSTRAINT_EQ ){
      কাঠামো sqlite3_সূচক_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argv_সূচক = 1;
      pIdxInfo->idx_অঙ্ক = 1;
      pIdxInfo->estimatedCost = 1.0;
      বিরতি;
    }
  }

  ফেরত SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
স্থির পূর্ণসংখ্যা fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(আকার(FsdirCsr)+256);
  যদি( pCur==0 ) ফেরত SQLITE_NOMEM;
  memset(pCur, 0, আকার(FsdirCsr));
  *ppCursor = &pCur->base;
  ফেরত SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
স্থির পূর্ণসংখ্যা fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  যদি( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  ফেরত SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
স্থির পূর্ণসংখ্যা fsdir_পরবর্তী(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  যদি( pCsr->pDir ){
    pCsr->pEntry = readdir(pCsr->pDir);
    যদি( pCsr->pEntry==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  ফেরত SQLITE_OK;
}

/*
** xFilter method implementation.
*/
স্থির পূর্ণসংখ্যা fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  ধ্রুবক অক্ষর *zDir;
  পূর্ণসংখ্যা nDir;


  যদি( idx_অঙ্ক!=1 || argc!=1 ){
    ফেরত SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  যদি( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[0]);
  nDir = sqlite3_মান_bytes(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  যদি( pCsr->zDir==0 ) ফেরত SQLITE_NOMEM;
  memcpy(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  ফেরত fsdir_পরবর্তী(pVtabCursor); 
}

/*
** xEof method implementation.
*/
স্থির পূর্ণসংখ্যা fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  ফেরত pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
স্থির পূর্ণসংখ্যা fsdirColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, পূর্ণসংখ্যা i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  নির্বাচন( i ){
    ক্ষেত্র 0: /* dir */
      sqlite3_ফলাফল_text(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      বিরতি;

    ক্ষেত্র 1: /* name */
      sqlite3_ফলাফল_text(ctx, pCsr->pEntry->d_name, -1, SQLITE_TRANSIENT);
      বিরতি;

    পূর্বনির্ধারিত:
      প্রতিজ্ঞা( 0 );
  }

  ফেরত SQLITE_OK;
}

/*
** xRowid method implementation.
*/
স্থির পূর্ণসংখ্যা fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  ফেরত SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
প্রকারলেখ কাঠামো FstreeVtab FstreeVtab;
প্রকারলেখ কাঠামো FstreeCsr FstreeCsr;
কাঠামো FstreeVtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

কাঠামো FstreeCsr {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  পূর্ণসংখ্যা fd;                         /* File descriptor open on current path */
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
স্থির পূর্ণসংখ্যা fstreeConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  FstreeVtab *pTab;

  যদি( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ফেরত SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(আকার(FstreeVtab));
  যদি( !pTab ) ফেরত SQLITE_NOMEM;
  memset(pTab, 0, আকার(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  ফেরত SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
স্থির পূর্ণসংখ্যা fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ফেরত SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
স্থির পূর্ণসংখ্যা fstree_Best_সূচক(sqlite3_vtab *tab, sqlite3_সূচক_info *pIdxInfo){
  পূর্ণসংখ্যা ii;

  জন্য(ii=0; ii<pIdxInfo->nConstraint; ii++){
    কাঠামো sqlite3_সূচক_constraint ধ্রুবক *p = &pIdxInfo->aConstraint[ii];
    যদি( p->iColumn==0 && p->usable && (
          p->op==SQLITE_সূচক_CONSTRAINT_GLOB
       || p->op==SQLITE_সূচক_CONSTRAINT_LIKE
       || p->op==SQLITE_সূচক_CONSTRAINT_EQ
    )){
      কাঠামো sqlite3_সূচক_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_অঙ্ক = p->op;
      pUsage->argv_সূচক = 1;
      pIdxInfo->estimatedCost = 100000.0;
      ফেরত SQLITE_OK;
    }
  }

  pIdxInfo->estimatedCost = 1000000000.0;
  ফেরত SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
স্থির পূর্ণসংখ্যা fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(আকার(FstreeCsr));
  যদি( pCur==0 ) ফেরত SQLITE_NOMEM;
  memset(pCur, 0, আকার(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->base;
  ফেরত SQLITE_OK;
}

স্থির শূন্য fstreeCloseFd(FstreeCsr *pCsr){
  যদি( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
স্থির পূর্ণসংখ্যা fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  ফেরত SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
স্থির পূর্ণসংখ্যা fstree_পরবর্তী(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  পূর্ণসংখ্যা rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_step(pCsr->pStmt);
  যদি( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }অন্যথা{
    rc = SQLITE_OK;
    pCsr->fd = open((ধ্রুবক অক্ষর*)sqlite3_column_text(pCsr->pStmt, 0), O_RDONLY);
  }

  ফেরত rc;
}

/*
** xFilter method implementation.
*/
স্থির পূর্ণসংখ্যা fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->base.pVtab);
  পূর্ণসংখ্যা rc;
  ধ্রুবক অক্ষর *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  অক্ষর *z_মূল;
  পূর্ণসংখ্যা n_মূল;
  অক্ষর *zPrefix;
  পূর্ণসংখ্যা nPrefix;
  ধ্রুবক অক্ষর *zDir;
  পূর্ণসংখ্যা nDir;
  অক্ষর aWild[2] = { '\0', '\0' };

#ifdef _WIN32
  ধ্রুবক অক্ষর *zDrive = getenv("fstreeDrive");
  যদি( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  z_মূল = sqlite3_mprintf("%s%c", zDrive, '/');
  n_মূল = sqlite3Strlen30(z_মূল);
  zPrefix = sqlite3_mprintf("%s", zDrive);
  nPrefix = sqlite3Strlen30(zPrefix);
#অন্যথা
  z_মূল = "/";
  n_মূল = 1;
  zPrefix = "";
  nPrefix = 0;
#endif

  zDir = z_মূল;
  nDir = n_মূল;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  যদি( rc!=SQLITE_OK ) ফেরত rc;

  যদি( idx_অঙ্ক ){
    ধ্রুবক অক্ষর *zQuery = (ধ্রুবক অক্ষর*)sqlite3_মান_text(argv[0]);
    নির্বাচন( idx_অঙ্ক ){
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        বিরতি;
      ক্ষেত্র SQLITE_সূচক_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        বিরতি;
    }

    যদি( sqlite3_strnicmp(zQuery, zPrefix, nPrefix)==0 ){
      পূর্ণসংখ্যা i;
      জন্য(i=nPrefix; zQuery[i]; i++){
        যদি( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) বিরতি;
        যদি( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  যদি( nDir==0 ) nDir = 1;

  sqlite3_bind_text(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 2, z_মূল, n_মূল, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 3, zPrefix, nPrefix, SQLITE_TRANSIENT);

#ifdef _WIN32
  sqlite3_free(zPrefix);
  sqlite3_free(z_মূল);
#endif

  ফেরত fstree_পরবর্তী(pVtabCursor); 
}

/*
** xEof method implementation.
*/
স্থির পূর্ণসংখ্যা fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ফেরত pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
স্থির পূর্ণসংখ্যা fstreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, পূর্ণসংখ্যা i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  যদি( i==0 ){      /* path */
    sqlite3_ফলাফল_মান(ctx, sqlite3_column_মান(pCsr->pStmt, 0));
  }অন্যথা{
    কাঠামো stat sBuf;
    fstat(pCsr->fd, &sBuf);

    যদি( S_ISREG(sBuf.st_mode) ){
      যদি( i==1 ){
        sqlite3_ফলাফল_int64(ctx, sBuf.st_আকার);
      }অন্যথা{
        পূর্ণসংখ্যা nRead;
        অক্ষর *aBuf = sqlite3_malloc(sBuf.st_mode+1);
        যদি( !aBuf ) ফেরত SQLITE_NOMEM;
        nRead = পঠ(pCsr->fd, aBuf, sBuf.st_mode);
        যদি( nRead!=sBuf.st_mode ){
          ফেরত SQLITE_IOERR;
        }
        sqlite3_ফলাফল_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  ফেরত SQLITE_OK;
}

/*
** xRowid method implementation.
*/
স্থির পূর্ণসংখ্যা fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  ফেরত SQLITE_OK;
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
স্থির পূর্ণসংখ্যা fsConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণসংখ্যা argc, ধ্রুবক অক্ষর *ধ্রুবক*argv,
  sqlite3_vtab **ppVtab,
  অক্ষর **pzErr
){
  fs_vtab *pVtab;
  পূর্ণসংখ্যা nByte;
  ধ্রুবক অক্ষর *zTbl;
  ধ্রুবক অক্ষর *zDb = argv[1];

  যদি( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ফেরত SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = আকার(fs_vtab) + (পূর্ণসংখ্যা)strlen(zTbl) + 1 + (পূর্ণসংখ্যা)strlen(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  যদি( !pVtab ) ফেরত SQLITE_NOMEM;

  pVtab->zTbl = (অক্ষর *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[strlen(zTbl)+1];
  pVtab->db = db;
  memcpy(pVtab->zTbl, zTbl, strlen(zTbl));
  memcpy(pVtab->zDb, zDb, strlen(zDb));
  *ppVtab = &pVtab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  ফেরত SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

স্থির পূর্ণসংখ্যা fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ফেরত SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
স্থির পূর্ণসংখ্যা fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(আকার(fs_cursor));
  *ppCursor = &pCur->base;
  ফেরত SQLITE_OK;
}

/*
** Close a fs cursor.
*/
স্থির পূর্ণসংখ্যা fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা fs_পরবর্তী(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  পূর্ণসংখ্যা rc;

  rc = sqlite3_step(pCur->pStmt);
  যদি( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;

  ফেরত rc;
}

স্থির পূর্ণসংখ্যা fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণসংখ্যা idx_অঙ্ক, ধ্রুবক অক্ষর *idxStr,
  পূর্ণসংখ্যা argc, sqlite3_মান **argv
){
  পূর্ণসংখ্যা rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  প্রতিজ্ঞা( (idx_অঙ্ক==0 && argc==0) || (idx_অঙ্ক==1 && argc==1) );
  যদি( idx_অঙ্ক==1 ){
    অক্ষর *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    যদি( !zStmt ) ফেরত SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    যদি( rc==SQLITE_OK ){
      sqlite3_bind_মান(pCur->pStmt, 1, argv[0]);
    }
  }অন্যথা{
    অক্ষর *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    যদি( !zStmt ) ফেরত SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  যদি( rc==SQLITE_OK ){
    rc = fs_পরবর্তী(pVtabCursor); 
  }
  ফেরত rc;
}

স্থির পূর্ণসংখ্যা fsColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, পূর্ণসংখ্যা i){
  fs_cursor *pCur = (fs_cursor*)cur;

  প্রতিজ্ঞা( i==0 || i==1 || i==2 );
  যদি( i==0 ){
    sqlite3_ফলাফল_মান(ctx, sqlite3_column_মান(pCur->pStmt, 0));
  }অন্যথা{
    ধ্রুবক অক্ষর *zFile = (ধ্রুবক অক্ষর *)sqlite3_column_text(pCur->pStmt, 1);
    কাঠামো stat sbuf;
    পূর্ণসংখ্যা fd;

    পূর্ণসংখ্যা n;
    fd = open(zFile, O_RDONLY);
    যদি( fd<0 ) ফেরত SQLITE_IOERR;
    fstat(fd, &sbuf);

    যদি( sbuf.st_আকার>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_আকার*2;
      অক্ষর *zNew;
      যদি( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      যদি( zNew==0 ){
        close(fd);
        ফেরত SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (পূর্ণসংখ্যা)পঠ(fd, pCur->zBuf, sbuf.st_আকার);
    close(fd);
    যদি( n!=sbuf.st_আকার ) ফেরত SQLITE_ERROR;
    pCur->nBuf = sbuf.st_আকার;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_ফলাফল_text(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_column_int64(pCur->pStmt, 0);
  ফেরত SQLITE_OK;
}

স্থির পূর্ণসংখ্যা fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  ফেরত (sqlite3_তথ্য_গুনতি(pCur->pStmt)==0);
}

স্থির পূর্ণসংখ্যা fs_Best_সূচক(sqlite3_vtab *tab, sqlite3_সূচক_info *pIdxInfo){
  পূর্ণসংখ্যা ii;

  জন্য(ii=0; ii<pIdxInfo->nConstraint; ii++){
    কাঠামো sqlite3_সূচক_constraint ধ্রুবক *pCons = &pIdxInfo->aConstraint[ii];
    যদি( pCons->iColumn<0 && pCons->usable
           && pCons->op==SQLITE_সূচক_CONSTRAINT_EQ ){
      কাঠামো sqlite3_সূচক_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argv_সূচক = 1;
      pIdxInfo->idx_অঙ্ক = 1;
      pIdxInfo->estimatedCost = 1.0;
      বিরতি;
    }
  }

  ফেরত SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
স্থির sqlite3_module fsModule = {
  0,                         /* iVersion */
  fsConnect,
  fsConnect,
  fs_Best_সূচক,
  fsDisconnect, 
  fsDisconnect,
  fsOpen,                      /* xOpen - open a cursor */
  fsClose,                     /* xClose - close a cursor */
  fsFilter,                    /* xFilter - configure scan constraints */
  fs_পরবর্তী,                      /* xNext - advance a cursor */
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

স্থির sqlite3_module fsdirModule = {
  0,                              /* iVersion */
  fsdirConnect,                   /* xCreate */
  fsdirConnect,                   /* xConnect */
  fsdir_Best_সূচক,                 /* xBestIndex */
  fsdirDisconnect,                /* xDisconnect */
  fsdirDisconnect,                /* xDestroy */
  fsdirOpen,                      /* xOpen - open a cursor */
  fsdirClose,                     /* xClose - close a cursor */
  fsdirFilter,                    /* xFilter - configure scan constraints */
  fsdir_পরবর্তী,                      /* xNext - advance a cursor */
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

স্থির sqlite3_module fstreeModule = {
  0,                              /* iVersion */
  fstreeConnect,                  /* xCreate */
  fstreeConnect,                  /* xConnect */
  fstree_Best_সূচক,                /* xBestIndex */
  fstreeDisconnect,               /* xDisconnect */
  fstreeDisconnect,               /* xDestroy */
  fstreeOpen,                     /* xOpen - open a cursor */
  fstreeClose,                    /* xClose - close a cursor */
  fstreeFilter,                   /* xFilter - configure scan constraints */
  fstree_পরবর্তী,                     /* xNext - advance a cursor */
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
বাহ্যিক পূর্ণসংখ্যা getDbPointer(Tcl_Interp *interp, ধ্রুবক অক্ষর *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI register_fs_module(
  Client_তথ্য client_তথ্য, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণসংখ্যা objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    ফেরত TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ফেরত TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (শূন্য *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#endif
  ফেরত TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
পূর্ণসংখ্যা Sqlitetestfs_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  স্থির কাঠামো {
     অক্ষর *zName;
     Tcl_ObjCmdProc *xProc;
     শূন্য *client_তথ্য;
  } aObjCmd[] = {
     { "register_fs_module",   register_fs_module, 0 },
  };
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<আকার(aObjCmd)/আকার(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_তথ্য, 0);
  }
#endif
  ফেরত TCL_OK;
}
