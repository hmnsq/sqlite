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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <fcntl.h>

#જો !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#નહીંતર
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
# endif
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE

પ્રકારવ્યાખ્યા સંરચના fs_vtab fs_vtab;
પ્રકારવ્યાખ્યા સંરચના fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
સંરચના fs_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
  અક્ષર *zDb;                      /* Name of db containing zTbl */
  અક્ષર *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
સંરચના fs_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
  અક્ષર *zBuf;
  પૂર્ણાંક nBuf;
  પૂર્ણાંક nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
પ્રકારવ્યાખ્યા સંરચના FsdirVtab FsdirVtab;
પ્રકારવ્યાખ્યા સંરચના FsdirCsr FsdirCsr;
સંરચના FsdirVtab {
  sqlite3_vtab base;
};

સંરચના FsdirCsr {
  sqlite3_vtab_cursor base;
  અક્ષર *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  સંરચના dirent *pEntry;
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
સ્થિર પૂર્ણાંક fsdirConnect(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  FsdirVtab *pTab;

  જો( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    પાછા SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(કદ(FsdirVtab));
  જો( !pTab ) પાછા SQLITE_NOMEM;
  memset(pTab, 0, કદ(FsdirVtab));

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  પાછા SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
સ્થિર પૂર્ણાંક fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  પાછા SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
સ્થિર પૂર્ણાંક fsdirBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  પૂર્ણાંક ii;

  pIdxInfo->estimatedCost = 1000000000.0;

  માટે(ii=0; ii<pIdxInfo->nConstraint; ii++){
    સંરચના sqlite3_index_constraint અચલ *p = &pIdxInfo->aConstraint[ii];
    જો( p->iColumn==0 && p->usable && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      સંરચના sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_અંક = 1;
      pIdxInfo->estimatedCost = 1.0;
      તોડો;
    }
  }

  પાછા SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
સ્થિર પૂર્ણાંક fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(કદ(FsdirCsr)+256);
  જો( pCur==0 ) પાછા SQLITE_NOMEM;
  memset(pCur, 0, કદ(FsdirCsr));
  *ppCursor = &pCur->base;
  પાછા SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
સ્થિર પૂર્ણાંક fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  જો( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  પાછા SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
સ્થિર પૂર્ણાંક fsdirNext(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  જો( pCsr->pDir ){
    pCsr->pEntry = readdir(pCsr->pDir);
    જો( pCsr->pEntry==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  પાછા SQLITE_OK;
}

/*
** xFilter method implementation.
*/
સ્થિર પૂર્ણાંક fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  પૂર્ણાંક idx_અંક, અચલ અક્ષર *idxStr,
  પૂર્ણાંક argc, sqlite3_કિંમત **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  અચલ અક્ષર *zDir;
  પૂર્ણાંક nDir;


  જો( idx_અંક!=1 || argc!=1 ){
    પાછા SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  જો( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
  nDir = sqlite3_કિંમત_bytes(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  જો( pCsr->zDir==0 ) પાછા SQLITE_NOMEM;
  memcpy(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  પાછા fsdirNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
સ્થિર પૂર્ણાંક fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  પાછા pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
સ્થિર પૂર્ણાંક fsdirColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, પૂર્ણાંક i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  પસંદગી( i ){
    સ્થિતિ 0: /* dir */
      sqlite3_result_text(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      તોડો;

    સ્થિતિ 1: /* name */
      sqlite3_result_text(ctx, pCsr->pEntry->d_name, -1, SQLITE_TRANSIENT);
      તોડો;

    મૂળભૂત:
      પ્રતિજ્ઞા( 0 );
  }

  પાછા SQLITE_OK;
}

/*
** xRowid method implementation.
*/
સ્થિર પૂર્ણાંક fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  પાછા SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
પ્રકારવ્યાખ્યા સંરચના FstreeVtab FstreeVtab;
પ્રકારવ્યાખ્યા સંરચના FstreeCsr FstreeCsr;
સંરચના FstreeVtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

સંરચના FstreeCsr {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  પૂર્ણાંક fd;                         /* File descriptor open on current path */
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
સ્થિર પૂર્ણાંક fstreeConnect(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  FstreeVtab *pTab;

  જો( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    પાછા SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(કદ(FstreeVtab));
  જો( !pTab ) પાછા SQLITE_NOMEM;
  memset(pTab, 0, કદ(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  પાછા SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
સ્થિર પૂર્ણાંક fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  પાછા SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
સ્થિર પૂર્ણાંક fstreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  પૂર્ણાંક ii;

  માટે(ii=0; ii<pIdxInfo->nConstraint; ii++){
    સંરચના sqlite3_index_constraint અચલ *p = &pIdxInfo->aConstraint[ii];
    જો( p->iColumn==0 && p->usable && (
          p->op==SQLITE_INDEX_CONSTRAINT_GLOB
       || p->op==SQLITE_INDEX_CONSTRAINT_LIKE
       || p->op==SQLITE_INDEX_CONSTRAINT_EQ
    )){
      સંરચના sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_અંક = p->op;
      pUsage->argvIndex = 1;
      pIdxInfo->estimatedCost = 100000.0;
      પાછા SQLITE_OK;
    }
  }

  pIdxInfo->estimatedCost = 1000000000.0;
  પાછા SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
સ્થિર પૂર્ણાંક fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(કદ(FstreeCsr));
  જો( pCur==0 ) પાછા SQLITE_NOMEM;
  memset(pCur, 0, કદ(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->base;
  પાછા SQLITE_OK;
}

સ્થિર શૂન્ય fstreeCloseFd(FstreeCsr *pCsr){
  જો( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
સ્થિર પૂર્ણાંક fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  પાછા SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
સ્થિર પૂર્ણાંક fstreeNext(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  પૂર્ણાંક rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_step(pCsr->pStmt);
  જો( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }નહીંતર{
    rc = SQLITE_OK;
    pCsr->fd = open((અચલ અક્ષર*)sqlite3_column_text(pCsr->pStmt, 0), O_RDONLY);
  }

  પાછા rc;
}

/*
** xFilter method implementation.
*/
સ્થિર પૂર્ણાંક fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  પૂર્ણાંક idx_અંક, અચલ અક્ષર *idxStr,
  પૂર્ણાંક argc, sqlite3_કિંમત **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->base.pVtab);
  પૂર્ણાંક rc;
  અચલ અક્ષર *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  અક્ષર *z_મૂળ;
  પૂર્ણાંક n_મૂળ;
  અક્ષર *zPrefix;
  પૂર્ણાંક nPrefix;
  અચલ અક્ષર *zDir;
  પૂર્ણાંક nDir;
  અક્ષર aWild[2] = { '\0', '\0' };

#ifdef _WIN32
  અચલ અક્ષર *zDrive = getenv("fstreeDrive");
  જો( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  z_મૂળ = sqlite3_mprintf("%s%c", zDrive, '/');
  n_મૂળ = sqlite3Strlen30(z_મૂળ);
  zPrefix = sqlite3_mprintf("%s", zDrive);
  nPrefix = sqlite3Strlen30(zPrefix);
#નહીંતર
  z_મૂળ = "/";
  n_મૂળ = 1;
  zPrefix = "";
  nPrefix = 0;
#endif

  zDir = z_મૂળ;
  nDir = n_મૂળ;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  જો( rc!=SQLITE_OK ) પાછા rc;

  જો( idx_અંક ){
    અચલ અક્ષર *zQuery = (અચલ અક્ષર*)sqlite3_કિંમત_text(argv[0]);
    પસંદગી( idx_અંક ){
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        તોડો;
      સ્થિતિ SQLITE_INDEX_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        તોડો;
    }

    જો( sqlite3_strnicmp(zQuery, zPrefix, nPrefix)==0 ){
      પૂર્ણાંક i;
      માટે(i=nPrefix; zQuery[i]; i++){
        જો( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) તોડો;
        જો( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  જો( nDir==0 ) nDir = 1;

  sqlite3_bind_text(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 2, z_મૂળ, n_મૂળ, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 3, zPrefix, nPrefix, SQLITE_TRANSIENT);

#ifdef _WIN32
  sqlite3_free(zPrefix);
  sqlite3_free(z_મૂળ);
#endif

  પાછા fstreeNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
સ્થિર પૂર્ણાંક fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  પાછા pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
સ્થિર પૂર્ણાંક fstreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, પૂર્ણાંક i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  જો( i==0 ){      /* path */
    sqlite3_result_કિંમત(ctx, sqlite3_column_કિંમત(pCsr->pStmt, 0));
  }નહીંતર{
    સંરચના stat sBuf;
    fstat(pCsr->fd, &sBuf);

    જો( S_ISREG(sBuf.st_mode) ){
      જો( i==1 ){
        sqlite3_result_int64(ctx, sBuf.st_size);
      }નહીંતર{
        પૂર્ણાંક nRead;
        અક્ષર *aBuf = sqlite3_malloc(sBuf.st_mode+1);
        જો( !aBuf ) પાછા SQLITE_NOMEM;
        nRead = પઠ(pCsr->fd, aBuf, sBuf.st_mode);
        જો( nRead!=sBuf.st_mode ){
          પાછા SQLITE_IOERR;
        }
        sqlite3_result_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  પાછા SQLITE_OK;
}

/*
** xRowid method implementation.
*/
સ્થિર પૂર્ણાંક fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  પાછા SQLITE_OK;
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
સ્થિર પૂર્ણાંક fsConnect(
  sqlite3 *db,
  શૂન્ય *pAux,
  પૂર્ણાંક argc, અચલ અક્ષર *અચલ*argv,
  sqlite3_vtab **ppVtab,
  અક્ષર **pzErr
){
  fs_vtab *pVtab;
  પૂર્ણાંક nByte;
  અચલ અક્ષર *zTbl;
  અચલ અક્ષર *zDb = argv[1];

  જો( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    પાછા SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = કદ(fs_vtab) + (પૂર્ણાંક)strlen(zTbl) + 1 + (પૂર્ણાંક)strlen(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  જો( !pVtab ) પાછા SQLITE_NOMEM;

  pVtab->zTbl = (અક્ષર *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[strlen(zTbl)+1];
  pVtab->db = db;
  memcpy(pVtab->zTbl, zTbl, strlen(zTbl));
  memcpy(pVtab->zDb, zDb, strlen(zDb));
  *ppVtab = &pVtab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  પાછા SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

સ્થિર પૂર્ણાંક fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  પાછા SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
સ્થિર પૂર્ણાંક fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(કદ(fs_cursor));
  *ppCursor = &pCur->base;
  પાછા SQLITE_OK;
}

/*
** Close a fs cursor.
*/
સ્થિર પૂર્ણાંક fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક fsNext(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  પૂર્ણાંક rc;

  rc = sqlite3_step(pCur->pStmt);
  જો( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;

  પાછા rc;
}

સ્થિર પૂર્ણાંક fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  પૂર્ણાંક idx_અંક, અચલ અક્ષર *idxStr,
  પૂર્ણાંક argc, sqlite3_કિંમત **argv
){
  પૂર્ણાંક rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  પ્રતિજ્ઞા( (idx_અંક==0 && argc==0) || (idx_અંક==1 && argc==1) );
  જો( idx_અંક==1 ){
    અક્ષર *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    જો( !zStmt ) પાછા SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    જો( rc==SQLITE_OK ){
      sqlite3_bind_કિંમત(pCur->pStmt, 1, argv[0]);
    }
  }નહીંતર{
    અક્ષર *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    જો( !zStmt ) પાછા SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  જો( rc==SQLITE_OK ){
    rc = fsNext(pVtabCursor); 
  }
  પાછા rc;
}

સ્થિર પૂર્ણાંક fsColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, પૂર્ણાંક i){
  fs_cursor *pCur = (fs_cursor*)cur;

  પ્રતિજ્ઞા( i==0 || i==1 || i==2 );
  જો( i==0 ){
    sqlite3_result_કિંમત(ctx, sqlite3_column_કિંમત(pCur->pStmt, 0));
  }નહીંતર{
    અચલ અક્ષર *zFile = (અચલ અક્ષર *)sqlite3_column_text(pCur->pStmt, 1);
    સંરચના stat sbuf;
    પૂર્ણાંક fd;

    પૂર્ણાંક n;
    fd = open(zFile, O_RDONLY);
    જો( fd<0 ) પાછા SQLITE_IOERR;
    fstat(fd, &sbuf);

    જો( sbuf.st_size>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_size*2;
      અક્ષર *zNew;
      જો( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      જો( zNew==0 ){
        close(fd);
        પાછા SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (પૂર્ણાંક)પઠ(fd, pCur->zBuf, sbuf.st_size);
    close(fd);
    જો( n!=sbuf.st_size ) પાછા SQLITE_ERROR;
    pCur->nBuf = sbuf.st_size;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_result_text(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_column_int64(pCur->pStmt, 0);
  પાછા SQLITE_OK;
}

સ્થિર પૂર્ણાંક fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  પાછા (sqlite3_data_ગણતરી(pCur->pStmt)==0);
}

સ્થિર પૂર્ણાંક fsBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  પૂર્ણાંક ii;

  માટે(ii=0; ii<pIdxInfo->nConstraint; ii++){
    સંરચના sqlite3_index_constraint અચલ *pCons = &pIdxInfo->aConstraint[ii];
    જો( pCons->iColumn<0 && pCons->usable
           && pCons->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      સંરચના sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_અંક = 1;
      pIdxInfo->estimatedCost = 1.0;
      તોડો;
    }
  }

  પાછા SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
સ્થિર sqlite3_module fsModule = {
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

સ્થિર sqlite3_module fsdirModule = {
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

સ્થિર sqlite3_module fstreeModule = {
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
બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp *interp, અચલ અક્ષર *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI register_fs_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  પૂર્ણાંક objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB");
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (શૂન્ય *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#endif
  પાછા TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
પૂર્ણાંક Sqlitetestfs_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  સ્થિર સંરચના {
     અક્ષર *zName;
     Tcl_ObjCmdProc *xProc;
     શૂન્ય *clientData;
  } aObjCmd[] = {
     { "register_fs_module",   register_fs_module, 0 },
  };
  પૂર્ણાંક i;
  માટે(i=0; i<કદ(aObjCmd)/કદ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  પાછા TCL_OK;
}
