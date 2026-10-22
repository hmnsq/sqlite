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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>

#ਜੇ !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#ਨਹੀਂਤਾਂ
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(ਬਹੁਲਕ) (((ਬਹੁਲਕ) & S_IFMT) == S_IFREG)
# endif
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ fs_vtab fs_vtab;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
ਬਣਤਰ fs_vtab {
  sqlite3_vtab ਅਧਾਰ;
  sqlite3 *db;
  ਅੱਖਰ *zDb;                      /* Name of db containing zTbl */
  ਅੱਖਰ *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
ਬਣਤਰ fs_cursor {
  sqlite3_vtab_cursor ਅਧਾਰ;
  sqlite3_stmt *pStmt;
  ਅੱਖਰ *zBuf;
  ਪੂਰਨ nBuf;
  ਪੂਰਨ nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ FsdirVtab FsdirVtab;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ FsdirCsr FsdirCsr;
ਬਣਤਰ FsdirVtab {
  sqlite3_vtab ਅਧਾਰ;
};

ਬਣਤਰ FsdirCsr {
  sqlite3_vtab_cursor ਅਧਾਰ;
  ਅੱਖਰ *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  ਬਣਤਰ dirent *p_ਇੰਦਰਾਜ;
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
ਸਥਿਰ ਪੂਰਨ fsdirConnect(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  FsdirVtab *pTab;

  ਜੇ( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ਵਾਪਸ SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(ਆਕਾਰ(FsdirVtab));
  ਜੇ( !pTab ) ਵਾਪਸ SQLITE_NOMEM;
  memset(pTab, 0, ਆਕਾਰ(FsdirVtab));

  *ppVtab = &pTab->ਅਧਾਰ;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  ਵਾਪਸ SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
ਸਥਿਰ ਪੂਰਨ fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ਵਾਪਸ SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
ਸਥਿਰ ਪੂਰਨ fsdir_ਉੱਤਮ_ਸੂਚਕਾਂਕ(sqlite3_vtab *tab, sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  ਪੂਰਨ ii;

  pIdxInfo->estimated_ਲਾਗਤ = 1000000000.0;

  ਲਈ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint ਅਚਲ *p = &pIdxInfo->aConstraint[ii];
    ਜੇ( p->i_ਕਾਲਮ==0 && p->usable && p->op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_EQ ){
      ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argv_ਸੂਚਕਾਂਕ = 1;
      pIdxInfo->idx_ਅੰਕ = 1;
      pIdxInfo->estimated_ਲਾਗਤ = 1.0;
      ਰੋਕੋ;
    }
  }

  ਵਾਪਸ SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
ਸਥਿਰ ਪੂਰਨ fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(ਆਕਾਰ(FsdirCsr)+256);
  ਜੇ( pCur==0 ) ਵਾਪਸ SQLITE_NOMEM;
  memset(pCur, 0, ਆਕਾਰ(FsdirCsr));
  *ppCursor = &pCur->ਅਧਾਰ;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
ਸਥਿਰ ਪੂਰਨ fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  ਜੇ( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
ਸਥਿਰ ਪੂਰਨ fsdir_ਅਗਲਾ(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  ਜੇ( pCsr->pDir ){
    pCsr->p_ਇੰਦਰਾਜ = readdir(pCsr->pDir);
    ਜੇ( pCsr->p_ਇੰਦਰਾਜ==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  ਵਾਪਸ SQLITE_OK;
}

/*
** xFilter method implementation.
*/
ਸਥਿਰ ਪੂਰਨ fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  ਅਚਲ ਅੱਖਰ *zDir;
  ਪੂਰਨ nDir;


  ਜੇ( idx_ਅੰਕ!=1 || argc!=1 ){
    ਵਾਪਸ SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  ਜੇ( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
  nDir = sqlite3_ਮੁੱਲ_ਬਾਈਟਾਂ(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  ਜੇ( pCsr->zDir==0 ) ਵਾਪਸ SQLITE_NOMEM;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  ਵਾਪਸ fsdir_ਅਗਲਾ(pVtabCursor); 
}

/*
** xEof method implementation.
*/
ਸਥਿਰ ਪੂਰਨ fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  ਵਾਪਸ pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
ਸਥਿਰ ਪੂਰਨ fsdir_ਕਾਲਮ(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ਪੂਰਨ i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  ਚੋਣ( i ){
    ਹਾਲਤ 0: /* dir */
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      ਰੋਕੋ;

    ਹਾਲਤ 1: /* name */
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pCsr->p_ਇੰਦਰਾਜ->d_name, -1, SQLITE_TRANSIENT);
      ਰੋਕੋ;

    ਮੂਲ:
      ਪ੍ਰਤਿਜ੍ਞਾ( 0 );
  }

  ਵਾਪਸ SQLITE_OK;
}

/*
** xRowid method implementation.
*/
ਸਥਿਰ ਪੂਰਨ fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  ਵਾਪਸ SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ FstreeVtab FstreeVtab;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ FstreeCsr FstreeCsr;
ਬਣਤਰ FstreeVtab {
  sqlite3_vtab ਅਧਾਰ;
  sqlite3 *db;
};

ਬਣਤਰ FstreeCsr {
  sqlite3_vtab_cursor ਅਧਾਰ;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  ਪੂਰਨ fd;                         /* File descriptor open on current path */
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
ਸਥਿਰ ਪੂਰਨ fstreeConnect(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  FstreeVtab *pTab;

  ਜੇ( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ਵਾਪਸ SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(ਆਕਾਰ(FstreeVtab));
  ਜੇ( !pTab ) ਵਾਪਸ SQLITE_NOMEM;
  memset(pTab, 0, ਆਕਾਰ(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->ਅਧਾਰ;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  ਵਾਪਸ SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
ਸਥਿਰ ਪੂਰਨ fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ਵਾਪਸ SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
ਸਥਿਰ ਪੂਰਨ fstree_ਉੱਤਮ_ਸੂਚਕਾਂਕ(sqlite3_vtab *tab, sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  ਪੂਰਨ ii;

  ਲਈ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint ਅਚਲ *p = &pIdxInfo->aConstraint[ii];
    ਜੇ( p->i_ਕਾਲਮ==0 && p->usable && (
          p->op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_GLOB
       || p->op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_LIKE
       || p->op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_EQ
    )){
      ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_ਅੰਕ = p->op;
      pUsage->argv_ਸੂਚਕਾਂਕ = 1;
      pIdxInfo->estimated_ਲਾਗਤ = 100000.0;
      ਵਾਪਸ SQLITE_OK;
    }
  }

  pIdxInfo->estimated_ਲਾਗਤ = 1000000000.0;
  ਵਾਪਸ SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
ਸਥਿਰ ਪੂਰਨ fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(ਆਕਾਰ(FstreeCsr));
  ਜੇ( pCur==0 ) ਵਾਪਸ SQLITE_NOMEM;
  memset(pCur, 0, ਆਕਾਰ(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->ਅਧਾਰ;
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਖਾਲੀ fstreeCloseFd(FstreeCsr *pCsr){
  ਜੇ( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
ਸਥਿਰ ਪੂਰਨ fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
ਸਥਿਰ ਪੂਰਨ fstree_ਅਗਲਾ(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ਪੂਰਨ rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_ਕਦਮ(pCsr->pStmt);
  ਜੇ( rc!=SQLITE_ਪੰਕਤੀ ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }ਨਹੀਂਤਾਂ{
    rc = SQLITE_OK;
    pCsr->fd = open((ਅਚਲ ਅੱਖਰ*)sqlite3_ਕਾਲਮ_ਪਾਠ(pCsr->pStmt, 0), O_RDONLY);
  }

  ਵਾਪਸ rc;
}

/*
** xFilter method implementation.
*/
ਸਥਿਰ ਪੂਰਨ fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->ਅਧਾਰ.pVtab);
  ਪੂਰਨ rc;
  ਅਚਲ ਅੱਖਰ *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  ਅੱਖਰ *z_ਜੜ੍ਹ;
  ਪੂਰਨ n_ਜੜ੍ਹ;
  ਅੱਖਰ *z_ਅਗੇਤਰ;
  ਪੂਰਨ n_ਅਗੇਤਰ;
  ਅਚਲ ਅੱਖਰ *zDir;
  ਪੂਰਨ nDir;
  ਅੱਖਰ aWild[2] = { '\0', '\0' };

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਅਚਲ ਅੱਖਰ *zDrive = getenv("fstreeDrive");
  ਜੇ( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  z_ਜੜ੍ਹ = sqlite3_mprintf("%s%c", zDrive, '/');
  n_ਜੜ੍ਹ = sqlite3Strlen30(z_ਜੜ੍ਹ);
  z_ਅਗੇਤਰ = sqlite3_mprintf("%s", zDrive);
  n_ਅਗੇਤਰ = sqlite3Strlen30(z_ਅਗੇਤਰ);
#ਨਹੀਂਤਾਂ
  z_ਜੜ੍ਹ = "/";
  n_ਜੜ੍ਹ = 1;
  z_ਅਗੇਤਰ = "";
  n_ਅਗੇਤਰ = 0;
#ਜੇ_ਅੰਤ

  zDir = z_ਜੜ੍ਹ;
  nDir = n_ਜੜ੍ਹ;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;

  ਜੇ( idx_ਅੰਕ ){
    ਅਚਲ ਅੱਖਰ *zQuery = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[0]);
    ਚੋਣ( idx_ਅੰਕ ){
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        ਰੋਕੋ;
      ਹਾਲਤ SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        ਰੋਕੋ;
    }

    ਜੇ( sqlite3_strnicmp(zQuery, z_ਅਗੇਤਰ, n_ਅਗੇਤਰ)==0 ){
      ਪੂਰਨ i;
      ਲਈ(i=n_ਅਗੇਤਰ; zQuery[i]; i++){
        ਜੇ( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) ਰੋਕੋ;
        ਜੇ( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  ਜੇ( nDir==0 ) nDir = 1;

  sqlite3_bind_ਪਾਠ(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_ਪਾਠ(pCsr->pStmt, 2, z_ਜੜ੍ਹ, n_ਜੜ੍ਹ, SQLITE_TRANSIENT);
  sqlite3_bind_ਪਾਠ(pCsr->pStmt, 3, z_ਅਗੇਤਰ, n_ਅਗੇਤਰ, SQLITE_TRANSIENT);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  sqlite3_free(z_ਅਗੇਤਰ);
  sqlite3_free(z_ਜੜ੍ਹ);
#ਜੇ_ਅੰਤ

  ਵਾਪਸ fstree_ਅਗਲਾ(pVtabCursor); 
}

/*
** xEof method implementation.
*/
ਸਥਿਰ ਪੂਰਨ fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ਵਾਪਸ pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
ਸਥਿਰ ਪੂਰਨ fstree_ਕਾਲਮ(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ਪੂਰਨ i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ਜੇ( i==0 ){      /*ਰਸਤਾ*/
    sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(ctx, sqlite3_ਕਾਲਮ_ਮੁੱਲ(pCsr->pStmt, 0));
  }ਨਹੀਂਤਾਂ{
    ਬਣਤਰ stat sBuf;
    fstat(pCsr->fd, &sBuf);

    ਜੇ( S_ISREG(sBuf.st_ਬਹੁਲਕ) ){
      ਜੇ( i==1 ){
        sqlite3_ਨਤੀਜਾ_int64(ctx, sBuf.st_ਮਾਪ);
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ nRead;
        ਅੱਖਰ *aBuf = sqlite3_malloc(sBuf.st_ਬਹੁਲਕ+1);
        ਜੇ( !aBuf ) ਵਾਪਸ SQLITE_NOMEM;
        nRead = ਪਠ(pCsr->fd, aBuf, sBuf.st_ਬਹੁਲਕ);
        ਜੇ( nRead!=sBuf.st_ਬਹੁਲਕ ){
          ਵਾਪਸ SQLITE_IOERR;
        }
        sqlite3_ਨਤੀਜਾ_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  ਵਾਪਸ SQLITE_OK;
}

/*
** xRowid method implementation.
*/
ਸਥਿਰ ਪੂਰਨ fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ fsConnect(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  fs_vtab *pVtab;
  ਪੂਰਨ nByte;
  ਅਚਲ ਅੱਖਰ *zTbl;
  ਅਚਲ ਅੱਖਰ *zDb = argv[1];

  ਜੇ( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    ਵਾਪਸ SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = ਆਕਾਰ(fs_vtab) + (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zTbl) + 1 + (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  ਜੇ( !pVtab ) ਵਾਪਸ SQLITE_NOMEM;

  pVtab->zTbl = (ਅੱਖਰ *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[ਲੜੀ_ਲੰਬਾਈ(zTbl)+1];
  pVtab->db = db;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pVtab->zTbl, zTbl, ਲੜੀ_ਲੰਬਾਈ(zTbl));
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pVtab->zDb, zDb, ਲੜੀ_ਲੰਬਾਈ(zDb));
  *ppVtab = &pVtab->ਅਧਾਰ;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  ਵਾਪਸ SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

ਸਥਿਰ ਪੂਰਨ fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ਵਾਪਸ SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
ਸਥਿਰ ਪੂਰਨ fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(ਆਕਾਰ(fs_cursor));
  *ppCursor = &pCur->ਅਧਾਰ;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Close a fs cursor.
*/
ਸਥਿਰ ਪੂਰਨ fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ fs_ਅਗਲਾ(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  ਪੂਰਨ rc;

  rc = sqlite3_ਕਦਮ(pCur->pStmt);
  ਜੇ( rc==SQLITE_ਪੰਕਤੀ || rc==SQLITE_ਪੂਰਾ ) rc = SQLITE_OK;

  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  ਪੂਰਨ rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  ਪ੍ਰਤਿਜ੍ਞਾ( (idx_ਅੰਕ==0 && argc==0) || (idx_ਅੰਕ==1 && argc==1) );
  ਜੇ( idx_ਅੰਕ==1 ){
    ਅੱਖਰ *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    ਜੇ( !zStmt ) ਵਾਪਸ SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    ਜੇ( rc==SQLITE_OK ){
      sqlite3_bind_ਮੁੱਲ(pCur->pStmt, 1, argv[0]);
    }
  }ਨਹੀਂਤਾਂ{
    ਅੱਖਰ *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    ਜੇ( !zStmt ) ਵਾਪਸ SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  ਜੇ( rc==SQLITE_OK ){
    rc = fs_ਅਗਲਾ(pVtabCursor); 
  }
  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ fs_ਕਾਲਮ(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, ਪੂਰਨ i){
  fs_cursor *pCur = (fs_cursor*)cur;

  ਪ੍ਰਤਿਜ੍ਞਾ( i==0 || i==1 || i==2 );
  ਜੇ( i==0 ){
    sqlite3_ਨਤੀਜਾ_ਮੁੱਲ(ctx, sqlite3_ਕਾਲਮ_ਮੁੱਲ(pCur->pStmt, 0));
  }ਨਹੀਂਤਾਂ{
    ਅਚਲ ਅੱਖਰ *zFile = (ਅਚਲ ਅੱਖਰ *)sqlite3_ਕਾਲਮ_ਪਾਠ(pCur->pStmt, 1);
    ਬਣਤਰ stat sbuf;
    ਪੂਰਨ fd;

    ਪੂਰਨ n;
    fd = open(zFile, O_RDONLY);
    ਜੇ( fd<0 ) ਵਾਪਸ SQLITE_IOERR;
    fstat(fd, &sbuf);

    ਜੇ( sbuf.st_ਮਾਪ>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_ਮਾਪ*2;
      ਅੱਖਰ *zNew;
      ਜੇ( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      ਜੇ( zNew==0 ){
        close(fd);
        ਵਾਪਸ SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (ਪੂਰਨ)ਪਠ(fd, pCur->zBuf, sbuf.st_ਮਾਪ);
    close(fd);
    ਜੇ( n!=sbuf.st_ਮਾਪ ) ਵਾਪਸ SQLITE_ERROR;
    pCur->nBuf = sbuf.st_ਮਾਪ;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_ਕਾਲਮ_int64(pCur->pStmt, 0);
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  ਵਾਪਸ (sqlite3_ਅੰਕੜੇ_ਗਿਣਤੀ(pCur->pStmt)==0);
}

ਸਥਿਰ ਪੂਰਨ fs_ਉੱਤਮ_ਸੂਚਕਾਂਕ(sqlite3_vtab *tab, sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  ਪੂਰਨ ii;

  ਲਈ(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint ਅਚਲ *pCons = &pIdxInfo->aConstraint[ii];
    ਜੇ( pCons->i_ਕਾਲਮ<0 && pCons->usable
           && pCons->op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_EQ ){
      ਬਣਤਰ sqlite3_ਸੂਚਕਾਂਕ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argv_ਸੂਚਕਾਂਕ = 1;
      pIdxInfo->idx_ਅੰਕ = 1;
      pIdxInfo->estimated_ਲਾਗਤ = 1.0;
      ਰੋਕੋ;
    }
  }

  ਵਾਪਸ SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ਸਥਿਰ sqlite3_module fsModule = {
  0,                         /* iVersion */
  fsConnect,
  fsConnect,
  fs_ਉੱਤਮ_ਸੂਚਕਾਂਕ,
  fsDisconnect, 
  fsDisconnect,
  fsOpen,                      /* xOpen - open a cursor */
  fsClose,                     /* xClose - close a cursor */
  fsFilter,                    /* xFilter - configure scan constraints */
  fs_ਅਗਲਾ,                      /* xNext - advance a cursor */
  fsEof,                       /* xEof - check for end of scan */
  fs_ਕਾਲਮ,                    /* xColumn - read data */
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

ਸਥਿਰ sqlite3_module fsdirModule = {
  0,                              /* iVersion */
  fsdirConnect,                   /* xCreate */
  fsdirConnect,                   /* xConnect */
  fsdir_ਉੱਤਮ_ਸੂਚਕਾਂਕ,                 /* xBestIndex */
  fsdirDisconnect,                /* xDisconnect */
  fsdirDisconnect,                /* xDestroy */
  fsdirOpen,                      /* xOpen - open a cursor */
  fsdirClose,                     /* xClose - close a cursor */
  fsdirFilter,                    /* xFilter - configure scan constraints */
  fsdir_ਅਗਲਾ,                      /* xNext - advance a cursor */
  fsdirEof,                       /* xEof - check for end of scan */
  fsdir_ਕਾਲਮ,                    /* xColumn - read data */
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

ਸਥਿਰ sqlite3_module fstreeModule = {
  0,                              /* iVersion */
  fstreeConnect,                  /* xCreate */
  fstreeConnect,                  /* xConnect */
  fstree_ਉੱਤਮ_ਸੂਚਕਾਂਕ,                /* xBestIndex */
  fstreeDisconnect,               /* xDisconnect */
  fstreeDisconnect,               /* xDestroy */
  fstreeOpen,                     /* xOpen - open a cursor */
  fstreeClose,                    /* xClose - close a cursor */
  fstreeFilter,                   /* xFilter - configure scan constraints */
  fstree_ਅਗਲਾ,                     /* xNext - advance a cursor */
  fstreeEof,                      /* xEof - check for end of scan */
  fstree_ਕਾਲਮ,                   /* xColumn - read data */
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
ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp *interp, ਅਚਲ ਅੱਖਰ *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI ਰਜਿਸਟਰ_fs_module(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  ਪੂਰਨ objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (ਖਾਲੀ *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}

#ਜੇ_ਅੰਤ


/*
** Register commands with the TCL interpreter.
*/
ਪੂਰਨ Sqlitetestfs_Init(Tcl_Interp *interp){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
     ਖਾਲੀ *client_ਅੰਕੜੇ;
  } aObjCmd[] = {
     { "register_fs_module",   ਰਜਿਸਟਰ_fs_module, 0 },
  };
  ਪੂਰਨ i;
  ਲਈ(i=0; i<ਆਕਾਰ(aObjCmd)/ਆਕਾਰ(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ਅੰਕੜੇ, 0);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ TCL_OK;
}
