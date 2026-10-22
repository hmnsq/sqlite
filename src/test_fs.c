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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>

#ଯଦି !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#ଅନ୍ୟଥା
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
# endif
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା fs_vtab fs_vtab;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
ସଂରଚନା fs_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
  ଅକ୍ଷର *zDb;                      /* Name of db containing zTbl */
  ଅକ୍ଷର *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
ସଂରଚନା fs_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
  ଅକ୍ଷର *zBuf;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBuf;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା FsdirVtab FsdirVtab;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା FsdirCsr FsdirCsr;
ସଂରଚନା FsdirVtab {
  sqlite3_vtab base;
};

ସଂରଚନା FsdirCsr {
  sqlite3_vtab_cursor base;
  ଅକ୍ଷର *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  ସଂରଚନା dirent *pEntry;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsdirConnect(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  FsdirVtab *pTab;

  ଯଦି( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(ଆକାର(FsdirVtab));
  ଯଦି( !pTab ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  memset(pTab, 0, ଆକାର(FsdirVtab));

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsdirBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;

  pIdxInfo->estimatedCost = 1000000000.0;

  ପାଇଁ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ସଂରଚନା sqlite3_index_constraint ଅଚଳ *p = &pIdxInfo->aConstraint[ii];
    ଯଦି( p->iColumn==0 && p->usable && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      ସଂରଚନା sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_ଅଙ୍କ = 1;
      pIdxInfo->estimatedCost = 1.0;
      ଭାଙ୍ଗ;
    }
  }

  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(ଆକାର(FsdirCsr)+256);
  ଯଦି( pCur==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  memset(pCur, 0, ଆକାର(FsdirCsr));
  *ppCursor = &pCur->base;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  ଯଦି( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsdirNext(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  ଯଦି( pCsr->pDir ){
    pCsr->pEntry = readdir(pCsr->pDir);
    ଯଦି( pCsr->pEntry==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** xFilter method implementation.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx_ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *idxStr,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  ଅଚଳ ଅକ୍ଷର *zDir;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nDir;


  ଯଦି( idx_ଅଙ୍କ!=1 || argc!=1 ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  ଯଦି( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
  nDir = sqlite3_ମୂଲ୍ୟ_bytes(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  ଯଦି( pCsr->zDir==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  memcpy(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  ଫେରନ୍ତୁ fsdirNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  ଫେରନ୍ତୁ pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsdirColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  ବାଛ( i ){
    ସ୍ଥିତି 0: /* dir */
      sqlite3_result_text(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      ଭାଙ୍ଗ;

    ସ୍ଥିତି 1: /* name */
      sqlite3_result_text(ctx, pCsr->pEntry->d_name, -1, SQLITE_TRANSIENT);
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ପ୍ରତିଜ୍ଞା( 0 );
  }

  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** xRowid method implementation.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  ଫେରନ୍ତୁ SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା FstreeVtab FstreeVtab;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା FstreeCsr FstreeCsr;
ସଂରଚନା FstreeVtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

ସଂରଚନା FstreeCsr {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;                         /* File descriptor open on current path */
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fstreeConnect(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  FstreeVtab *pTab;

  ଯଦି( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(ଆକାର(FstreeVtab));
  ଯଦି( !pTab ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  memset(pTab, 0, ଆକାର(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fstreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;

  ପାଇଁ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ସଂରଚନା sqlite3_index_constraint ଅଚଳ *p = &pIdxInfo->aConstraint[ii];
    ଯଦି( p->iColumn==0 && p->usable && (
          p->op==SQLITE_INDEX_CONSTRAINT_GLOB
       || p->op==SQLITE_INDEX_CONSTRAINT_LIKE
       || p->op==SQLITE_INDEX_CONSTRAINT_EQ
    )){
      ସଂରଚନା sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_ଅଙ୍କ = p->op;
      pUsage->argvIndex = 1;
      pIdxInfo->estimatedCost = 100000.0;
      ଫେରନ୍ତୁ SQLITE_OK;
    }
  }

  pIdxInfo->estimatedCost = 1000000000.0;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(ଆକାର(FstreeCsr));
  ଯଦି( pCur==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  memset(pCur, 0, ଆକାର(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->base;
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ଶୂନ୍ୟ fstreeCloseFd(FstreeCsr *pCsr){
  ଯଦି( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fstreeNext(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_step(pCsr->pStmt);
  ଯଦି( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }ଅନ୍ୟଥା{
    rc = SQLITE_OK;
    pCsr->fd = open((ଅଚଳ ଅକ୍ଷର*)sqlite3_column_text(pCsr->pStmt, 0), O_RDONLY);
  }

  ଫେରନ୍ତୁ rc;
}

/*
** xFilter method implementation.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx_ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *idxStr,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->base.pVtab);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଅଚଳ ଅକ୍ଷର *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  ଅକ୍ଷର *z_ମୂଳ;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ମୂଳ;
  ଅକ୍ଷର *zPrefix;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPrefix;
  ଅଚଳ ଅକ୍ଷର *zDir;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nDir;
  ଅକ୍ଷର aWild[2] = { '\0', '\0' };

#ifdef _WIN32
  ଅଚଳ ଅକ୍ଷର *zDrive = getenv("fstreeDrive");
  ଯଦି( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  z_ମୂଳ = sqlite3_mprintf("%s%c", zDrive, '/');
  n_ମୂଳ = sqlite3Strlen30(z_ମୂଳ);
  zPrefix = sqlite3_mprintf("%s", zDrive);
  nPrefix = sqlite3Strlen30(zPrefix);
#ଅନ୍ୟଥା
  z_ମୂଳ = "/";
  n_ମୂଳ = 1;
  zPrefix = "";
  nPrefix = 0;
#endif

  zDir = z_ମୂଳ;
  nDir = n_ମୂଳ;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  ଯଦି( rc!=SQLITE_OK ) ଫେରନ୍ତୁ rc;

  ଯଦି( idx_ଅଙ୍କ ){
    ଅଚଳ ଅକ୍ଷର *zQuery = (ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(argv[0]);
    ବାଛ( idx_ଅଙ୍କ ){
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_INDEX_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        ଭାଙ୍ଗ;
    }

    ଯଦି( sqlite3_strnicmp(zQuery, zPrefix, nPrefix)==0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      ପାଇଁ(i=nPrefix; zQuery[i]; i++){
        ଯଦି( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) ଭାଙ୍ଗ;
        ଯଦି( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  ଯଦି( nDir==0 ) nDir = 1;

  sqlite3_bind_text(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 2, z_ମୂଳ, n_ମୂଳ, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 3, zPrefix, nPrefix, SQLITE_TRANSIENT);

#ifdef _WIN32
  sqlite3_free(zPrefix);
  sqlite3_free(z_ମୂଳ);
#endif

  ଫେରନ୍ତୁ fstreeNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ଫେରନ୍ତୁ pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fstreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ଯଦି( i==0 ){      /* path */
    sqlite3_result_ମୂଲ୍ୟ(ctx, sqlite3_column_ମୂଲ୍ୟ(pCsr->pStmt, 0));
  }ଅନ୍ୟଥା{
    ସଂରଚନା stat sBuf;
    fstat(pCsr->fd, &sBuf);

    ଯଦି( S_ISREG(sBuf.st_mode) ){
      ଯଦି( i==1 ){
        sqlite3_result_int64(ctx, sBuf.st_size);
      }ଅନ୍ୟଥା{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRead;
        ଅକ୍ଷର *aBuf = sqlite3_malloc(sBuf.st_mode+1);
        ଯଦି( !aBuf ) ଫେରନ୍ତୁ SQLITE_NOMEM;
        nRead = ପଠ(pCsr->fd, aBuf, sBuf.st_mode);
        ଯଦି( nRead!=sBuf.st_mode ){
          ଫେରନ୍ତୁ SQLITE_IOERR;
        }
        sqlite3_result_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** xRowid method implementation.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  ଫେରନ୍ତୁ SQLITE_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsConnect(
  sqlite3 *db,
  ଶୂନ୍ୟ *pAux,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅଚଳ ଅକ୍ଷର *ଅଚଳ*argv,
  sqlite3_vtab **ppVtab,
  ଅକ୍ଷର **pzErr
){
  fs_vtab *pVtab;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;
  ଅଚଳ ଅକ୍ଷର *zTbl;
  ଅଚଳ ଅକ୍ଷର *zDb = argv[1];

  ଯଦି( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = ଆକାର(fs_vtab) + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zTbl) + 1 + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  ଯଦି( !pVtab ) ଫେରନ୍ତୁ SQLITE_NOMEM;

  pVtab->zTbl = (ଅକ୍ଷର *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[strlen(zTbl)+1];
  pVtab->db = db;
  memcpy(pVtab->zTbl, zTbl, strlen(zTbl));
  memcpy(pVtab->zDb, zDb, strlen(zDb));
  *ppVtab = &pVtab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  ଫେରନ୍ତୁ SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ଫେରନ୍ତୁ SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(ଆକାର(fs_cursor));
  *ppCursor = &pCur->base;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Close a fs cursor.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsNext(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  rc = sqlite3_step(pCur->pStmt);
  ଯଦି( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;

  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx_ଅଙ୍କ, ଅଚଳ ଅକ୍ଷର *idxStr,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, sqlite3_ମୂଲ୍ୟ **argv
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  ପ୍ରତିଜ୍ଞା( (idx_ଅଙ୍କ==0 && argc==0) || (idx_ଅଙ୍କ==1 && argc==1) );
  ଯଦି( idx_ଅଙ୍କ==1 ){
    ଅକ୍ଷର *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    ଯଦି( !zStmt ) ଫେରନ୍ତୁ SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    ଯଦି( rc==SQLITE_OK ){
      sqlite3_bind_ମୂଲ୍ୟ(pCur->pStmt, 1, argv[0]);
    }
  }ଅନ୍ୟଥା{
    ଅକ୍ଷର *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    ଯଦି( !zStmt ) ଫେରନ୍ତୁ SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  ଯଦି( rc==SQLITE_OK ){
    rc = fsNext(pVtabCursor); 
  }
  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i){
  fs_cursor *pCur = (fs_cursor*)cur;

  ପ୍ରତିଜ୍ଞା( i==0 || i==1 || i==2 );
  ଯଦି( i==0 ){
    sqlite3_result_ମୂଲ୍ୟ(ctx, sqlite3_column_ମୂଲ୍ୟ(pCur->pStmt, 0));
  }ଅନ୍ୟଥା{
    ଅଚଳ ଅକ୍ଷର *zFile = (ଅଚଳ ଅକ୍ଷର *)sqlite3_column_text(pCur->pStmt, 1);
    ସଂରଚନା stat sbuf;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
    fd = open(zFile, O_RDONLY);
    ଯଦି( fd<0 ) ଫେରନ୍ତୁ SQLITE_IOERR;
    fstat(fd, &sbuf);

    ଯଦି( sbuf.st_size>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_size*2;
      ଅକ୍ଷର *zNew;
      ଯଦି( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      ଯଦି( zNew==0 ){
        close(fd);
        ଫେରନ୍ତୁ SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ପଠ(fd, pCur->zBuf, sbuf.st_size);
    close(fd);
    ଯଦି( n!=sbuf.st_size ) ଫେରନ୍ତୁ SQLITE_ERROR;
    pCur->nBuf = sbuf.st_size;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_result_text(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_column_int64(pCur->pStmt, 0);
  ଫେରନ୍ତୁ SQLITE_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  ଫେରନ୍ତୁ (sqlite3_data_ଗଣନା(pCur->pStmt)==0);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fsBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;

  ପାଇଁ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ସଂରଚନା sqlite3_index_constraint ଅଚଳ *pCons = &pIdxInfo->aConstraint[ii];
    ଯଦି( pCons->iColumn<0 && pCons->usable
           && pCons->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      ସଂରଚନା sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_ଅଙ୍କ = 1;
      pIdxInfo->estimatedCost = 1.0;
      ଭାଙ୍ଗ;
    }
  }

  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ସ୍ଥିର sqlite3_module fsModule = {
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

ସ୍ଥିର sqlite3_module fsdirModule = {
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

ସ୍ଥିର sqlite3_module fstreeModule = {
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
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp *interp, ଅଚଳ ଅକ୍ଷର *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI register_fs_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (ଶୂନ୍ୟ *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#endif
  ଫେରନ୍ତୁ TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetestfs_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
     Tcl_ObjCmdProc *xProc;
     ଶୂନ୍ୟ *clientData;
  } aObjCmd[] = {
     { "register_fs_module",   register_fs_module, 0 },
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<ଆକାର(aObjCmd)/ଆକାର(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  ଫେରନ୍ତୁ TCL_OK;
}
