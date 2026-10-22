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
#অন্তর্ভাৱ "sqliteInt.h"
#অন্তর্ভাৱ "tclsqlite.h"
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <fcntl.h>

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

প্রকারনির্দেশ সংরচনা fs_vtab fs_vtab;
প্রকারনির্দেশ সংরচনা fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
সংরচনা fs_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
  ৱর্ণ *zDb;                      /* Name of db containing zTbl */
  ৱর্ণ *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
সংরচনা fs_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
  ৱর্ণ *zBuf;
  পূর্ণাঙ্ক nBuf;
  পূর্ণাঙ্ক nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
প্রকারনির্দেশ সংরচনা FsdirVtab FsdirVtab;
প্রকারনির্দেশ সংরচনা FsdirCsr FsdirCsr;
সংরচনা FsdirVtab {
  sqlite3_vtab base;
};

সংরচনা FsdirCsr {
  sqlite3_vtab_cursor base;
  ৱর্ণ *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  সংরচনা dirent *pEntry;
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
স্থির পূর্ণাঙ্ক fsdirConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণাঙ্ক argc, ধ্রুৱ ৱর্ণ *ধ্রুৱ*argv,
  sqlite3_vtab **ppVtab,
  ৱর্ণ **pzErr
){
  FsdirVtab *pTab;

  যদি( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ঘূৰাই SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(পরিমাণ(FsdirVtab));
  যদি( !pTab ) ঘূৰাই SQLITE_NOMEM;
  memset(pTab, 0, পরিমাণ(FsdirVtab));

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  ঘূৰাই SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
স্থির পূর্ণাঙ্ক fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ঘূৰাই SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
স্থির পূর্ণাঙ্ক fsdirBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  পূর্ণাঙ্ক ii;

  pIdxInfo->estimatedCost = 1000000000.0;

  বাবে(ii=0; ii<pIdxInfo->nConstraint; ii++){
    সংরচনা sqlite3_index_constraint ধ্রুৱ *p = &pIdxInfo->aConstraint[ii];
    যদি( p->iColumn==0 && p->usable && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      সংরচনা sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_অঙ্ক = 1;
      pIdxInfo->estimatedCost = 1.0;
      ভঙ্গ;
    }
  }

  ঘূৰাই SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
স্থির পূর্ণাঙ্ক fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(পরিমাণ(FsdirCsr)+256);
  যদি( pCur==0 ) ঘূৰাই SQLITE_NOMEM;
  memset(pCur, 0, পরিমাণ(FsdirCsr));
  *ppCursor = &pCur->base;
  ঘূৰাই SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
স্থির পূর্ণাঙ্ক fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  যদি( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  ঘূৰাই SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
স্থির পূর্ণাঙ্ক fsdirNext(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  যদি( pCsr->pDir ){
    pCsr->pEntry = readdir(pCsr->pDir);
    যদি( pCsr->pEntry==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  ঘূৰাই SQLITE_OK;
}

/*
** xFilter method implementation.
*/
স্থির পূর্ণাঙ্ক fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণাঙ্ক idx_অঙ্ক, ধ্রুৱ ৱর্ণ *idxStr,
  পূর্ণাঙ্ক argc, sqlite3_মান **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  ধ্রুৱ ৱর্ণ *zDir;
  পূর্ণাঙ্ক nDir;


  যদি( idx_অঙ্ক!=1 || argc!=1 ){
    ঘূৰাই SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  যদি( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (ধ্রুৱ ৱর্ণ*)sqlite3_মান_text(argv[0]);
  nDir = sqlite3_মান_bytes(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  যদি( pCsr->zDir==0 ) ঘূৰাই SQLITE_NOMEM;
  memcpy(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  ঘূৰাই fsdirNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
স্থির পূর্ণাঙ্ক fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  ঘূৰাই pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
স্থির পূর্ণাঙ্ক fsdirColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, পূর্ণাঙ্ক i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  ৱিকল্প( i ){
    পক্ষ 0: /* dir */
      sqlite3_result_text(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      ভঙ্গ;

    পক্ষ 1: /* name */
      sqlite3_result_text(ctx, pCsr->pEntry->d_name, -1, SQLITE_TRANSIENT);
      ভঙ্গ;

    স্ৱভাৱ:
      প্রতিজ্ঞা( 0 );
  }

  ঘূৰাই SQLITE_OK;
}

/*
** xRowid method implementation.
*/
স্থির পূর্ণাঙ্ক fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  ঘূৰাই SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
প্রকারনির্দেশ সংরচনা FstreeVtab FstreeVtab;
প্রকারনির্দেশ সংরচনা FstreeCsr FstreeCsr;
সংরচনা FstreeVtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

সংরচনা FstreeCsr {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  পূর্ণাঙ্ক fd;                         /* File descriptor open on current path */
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
স্থির পূর্ণাঙ্ক fstreeConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণাঙ্ক argc, ধ্রুৱ ৱর্ণ *ধ্রুৱ*argv,
  sqlite3_vtab **ppVtab,
  ৱর্ণ **pzErr
){
  FstreeVtab *pTab;

  যদি( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ঘূৰাই SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(পরিমাণ(FstreeVtab));
  যদি( !pTab ) ঘূৰাই SQLITE_NOMEM;
  memset(pTab, 0, পরিমাণ(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  ঘূৰাই SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
স্থির পূর্ণাঙ্ক fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ঘূৰাই SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
স্থির পূর্ণাঙ্ক fstreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  পূর্ণাঙ্ক ii;

  বাবে(ii=0; ii<pIdxInfo->nConstraint; ii++){
    সংরচনা sqlite3_index_constraint ধ্রুৱ *p = &pIdxInfo->aConstraint[ii];
    যদি( p->iColumn==0 && p->usable && (
          p->op==SQLITE_INDEX_CONSTRAINT_GLOB
       || p->op==SQLITE_INDEX_CONSTRAINT_LIKE
       || p->op==SQLITE_INDEX_CONSTRAINT_EQ
    )){
      সংরচনা sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_অঙ্ক = p->op;
      pUsage->argvIndex = 1;
      pIdxInfo->estimatedCost = 100000.0;
      ঘূৰাই SQLITE_OK;
    }
  }

  pIdxInfo->estimatedCost = 1000000000.0;
  ঘূৰাই SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
স্থির পূর্ণাঙ্ক fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(পরিমাণ(FstreeCsr));
  যদি( pCur==0 ) ঘূৰাই SQLITE_NOMEM;
  memset(pCur, 0, পরিমাণ(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->base;
  ঘূৰাই SQLITE_OK;
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
স্থির পূর্ণাঙ্ক fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  ঘূৰাই SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
স্থির পূর্ণাঙ্ক fstreeNext(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  পূর্ণাঙ্ক rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_step(pCsr->pStmt);
  যদি( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }অন্যথা{
    rc = SQLITE_OK;
    pCsr->fd = open((ধ্রুৱ ৱর্ণ*)sqlite3_column_text(pCsr->pStmt, 0), O_RDONLY);
  }

  ঘূৰাই rc;
}

/*
** xFilter method implementation.
*/
স্থির পূর্ণাঙ্ক fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণাঙ্ক idx_অঙ্ক, ধ্রুৱ ৱর্ণ *idxStr,
  পূর্ণাঙ্ক argc, sqlite3_মান **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->base.pVtab);
  পূর্ণাঙ্ক rc;
  ধ্রুৱ ৱর্ণ *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  ৱর্ণ *z_মূল;
  পূর্ণাঙ্ক n_মূল;
  ৱর্ণ *zPrefix;
  পূর্ণাঙ্ক nPrefix;
  ধ্রুৱ ৱর্ণ *zDir;
  পূর্ণাঙ্ক nDir;
  ৱর্ণ aWild[2] = { '\0', '\0' };

#ifdef _WIN32
  ধ্রুৱ ৱর্ণ *zDrive = getenv("fstreeDrive");
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
  যদি( rc!=SQLITE_OK ) ঘূৰাই rc;

  যদি( idx_অঙ্ক ){
    ধ্রুৱ ৱর্ণ *zQuery = (ধ্রুৱ ৱর্ণ*)sqlite3_মান_text(argv[0]);
    ৱিকল্প( idx_অঙ্ক ){
      পক্ষ SQLITE_INDEX_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        ভঙ্গ;
      পক্ষ SQLITE_INDEX_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        ভঙ্গ;
    }

    যদি( sqlite3_strnicmp(zQuery, zPrefix, nPrefix)==0 ){
      পূর্ণাঙ্ক i;
      বাবে(i=nPrefix; zQuery[i]; i++){
        যদি( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) ভঙ্গ;
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

  ঘূৰাই fstreeNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
স্থির পূর্ণাঙ্ক fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ঘূৰাই pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
স্থির পূর্ণাঙ্ক fstreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, পূর্ণাঙ্ক i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  যদি( i==0 ){      /* path */
    sqlite3_result_মান(ctx, sqlite3_column_মান(pCsr->pStmt, 0));
  }অন্যথা{
    সংরচনা stat sBuf;
    fstat(pCsr->fd, &sBuf);

    যদি( S_ISREG(sBuf.st_mode) ){
      যদি( i==1 ){
        sqlite3_result_int64(ctx, sBuf.st_size);
      }অন্যথা{
        পূর্ণাঙ্ক nRead;
        ৱর্ণ *aBuf = sqlite3_malloc(sBuf.st_mode+1);
        যদি( !aBuf ) ঘূৰাই SQLITE_NOMEM;
        nRead = পঠ(pCsr->fd, aBuf, sBuf.st_mode);
        যদি( nRead!=sBuf.st_mode ){
          ঘূৰাই SQLITE_IOERR;
        }
        sqlite3_result_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  ঘূৰাই SQLITE_OK;
}

/*
** xRowid method implementation.
*/
স্থির পূর্ণাঙ্ক fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  ঘূৰাই SQLITE_OK;
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
স্থির পূর্ণাঙ্ক fsConnect(
  sqlite3 *db,
  শূন্য *pAux,
  পূর্ণাঙ্ক argc, ধ্রুৱ ৱর্ণ *ধ্রুৱ*argv,
  sqlite3_vtab **ppVtab,
  ৱর্ণ **pzErr
){
  fs_vtab *pVtab;
  পূর্ণাঙ্ক nByte;
  ধ্রুৱ ৱর্ণ *zTbl;
  ধ্রুৱ ৱর্ণ *zDb = argv[1];

  যদি( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ঘূৰাই SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = পরিমাণ(fs_vtab) + (পূর্ণাঙ্ক)strlen(zTbl) + 1 + (পূর্ণাঙ্ক)strlen(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  যদি( !pVtab ) ঘূৰাই SQLITE_NOMEM;

  pVtab->zTbl = (ৱর্ণ *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[strlen(zTbl)+1];
  pVtab->db = db;
  memcpy(pVtab->zTbl, zTbl, strlen(zTbl));
  memcpy(pVtab->zDb, zDb, strlen(zDb));
  *ppVtab = &pVtab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  ঘূৰাই SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

স্থির পূর্ণাঙ্ক fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ঘূৰাই SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
স্থির পূর্ণাঙ্ক fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(পরিমাণ(fs_cursor));
  *ppCursor = &pCur->base;
  ঘূৰাই SQLITE_OK;
}

/*
** Close a fs cursor.
*/
স্থির পূর্ণাঙ্ক fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  ঘূৰাই SQLITE_OK;
}

স্থির পূর্ণাঙ্ক fsNext(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  পূর্ণাঙ্ক rc;

  rc = sqlite3_step(pCur->pStmt);
  যদি( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;

  ঘূৰাই rc;
}

স্থির পূর্ণাঙ্ক fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  পূর্ণাঙ্ক idx_অঙ্ক, ধ্রুৱ ৱর্ণ *idxStr,
  পূর্ণাঙ্ক argc, sqlite3_মান **argv
){
  পূর্ণাঙ্ক rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  প্রতিজ্ঞা( (idx_অঙ্ক==0 && argc==0) || (idx_অঙ্ক==1 && argc==1) );
  যদি( idx_অঙ্ক==1 ){
    ৱর্ণ *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    যদি( !zStmt ) ঘূৰাই SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    যদি( rc==SQLITE_OK ){
      sqlite3_bind_মান(pCur->pStmt, 1, argv[0]);
    }
  }অন্যথা{
    ৱর্ণ *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    যদি( !zStmt ) ঘূৰাই SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  যদি( rc==SQLITE_OK ){
    rc = fsNext(pVtabCursor); 
  }
  ঘূৰাই rc;
}

স্থির পূর্ণাঙ্ক fsColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, পূর্ণাঙ্ক i){
  fs_cursor *pCur = (fs_cursor*)cur;

  প্রতিজ্ঞা( i==0 || i==1 || i==2 );
  যদি( i==0 ){
    sqlite3_result_মান(ctx, sqlite3_column_মান(pCur->pStmt, 0));
  }অন্যথা{
    ধ্রুৱ ৱর্ণ *zFile = (ধ্রুৱ ৱর্ণ *)sqlite3_column_text(pCur->pStmt, 1);
    সংরচনা stat sbuf;
    পূর্ণাঙ্ক fd;

    পূর্ণাঙ্ক n;
    fd = open(zFile, O_RDONLY);
    যদি( fd<0 ) ঘূৰাই SQLITE_IOERR;
    fstat(fd, &sbuf);

    যদি( sbuf.st_size>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_size*2;
      ৱর্ণ *zNew;
      যদি( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      যদি( zNew==0 ){
        close(fd);
        ঘূৰাই SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (পূর্ণাঙ্ক)পঠ(fd, pCur->zBuf, sbuf.st_size);
    close(fd);
    যদি( n!=sbuf.st_size ) ঘূৰাই SQLITE_ERROR;
    pCur->nBuf = sbuf.st_size;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_result_text(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  ঘূৰাই SQLITE_OK;
}

স্থির পূর্ণাঙ্ক fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_column_int64(pCur->pStmt, 0);
  ঘূৰাই SQLITE_OK;
}

স্থির পূর্ণাঙ্ক fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  ঘূৰাই (sqlite3_data_গুনতি(pCur->pStmt)==0);
}

স্থির পূর্ণাঙ্ক fsBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  পূর্ণাঙ্ক ii;

  বাবে(ii=0; ii<pIdxInfo->nConstraint; ii++){
    সংরচনা sqlite3_index_constraint ধ্রুৱ *pCons = &pIdxInfo->aConstraint[ii];
    যদি( pCons->iColumn<0 && pCons->usable
           && pCons->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      সংরচনা sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_অঙ্ক = 1;
      pIdxInfo->estimatedCost = 1.0;
      ভঙ্গ;
    }
  }

  ঘূৰাই SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
স্থির sqlite3_module fsModule = {
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

স্থির sqlite3_module fsdirModule = {
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

স্থির sqlite3_module fstreeModule = {
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
বাহ্য পূর্ণাঙ্ক getDbPointer(Tcl_Interp *interp, ধ্রুৱ ৱর্ণ *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI register_fs_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  পূর্ণাঙ্ক objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    ঘূৰাই TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ঘূৰাই TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (শূন্য *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#endif
  ঘূৰাই TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
পূর্ণাঙ্ক Sqlitetestfs_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  স্থির সংরচনা {
     ৱর্ণ *zName;
     Tcl_ObjCmdProc *xProc;
     শূন্য *clientData;
  } aObjCmd[] = {
     { "register_fs_module",   register_fs_module, 0 },
  };
  পূর্ণাঙ্ক i;
  বাবে(i=0; i<পরিমাণ(aObjCmd)/পরিমাণ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  ঘূৰাই TCL_OK;
}
