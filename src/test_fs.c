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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <fcntl.h>

#எனில் !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#இல்லையெனில்
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
# endif
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE

வகைவரை கட்டமைப்பு fs_vtab fs_vtab;
வகைவரை கட்டமைப்பு fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
கட்டமைப்பு fs_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
  எழுத்து *zDb;                      /* Name of db containing zTbl */
  எழுத்து *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
கட்டமைப்பு fs_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
  எழுத்து *zBuf;
  முழு nBuf;
  முழு nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
வகைவரை கட்டமைப்பு FsdirVtab FsdirVtab;
வகைவரை கட்டமைப்பு FsdirCsr FsdirCsr;
கட்டமைப்பு FsdirVtab {
  sqlite3_vtab base;
};

கட்டமைப்பு FsdirCsr {
  sqlite3_vtab_cursor base;
  எழுத்து *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  கட்டமைப்பு dirent *pEntry;
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
நிலையான முழு fsdirConnect(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  FsdirVtab *pTab;

  எனில்( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    திரும்பு SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(அளவு(FsdirVtab));
  எனில்( !pTab ) திரும்பு SQLITE_NOMEM;
  memset(pTab, 0, அளவு(FsdirVtab));

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  திரும்பு SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
நிலையான முழு fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  திரும்பு SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
நிலையான முழு fsdirBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  முழு ii;

  pIdxInfo->estimatedCost = 1000000000.0;

  க்கு(ii=0; ii<pIdxInfo->nConstraint; ii++){
    கட்டமைப்பு sqlite3_index_constraint மாறிலி *p = &pIdxInfo->aConstraint[ii];
    எனில்( p->iColumn==0 && p->usable && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      கட்டமைப்பு sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_இலக்கம் = 1;
      pIdxInfo->estimatedCost = 1.0;
      நிறுத்து;
    }
  }

  திரும்பு SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
நிலையான முழு fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(அளவு(FsdirCsr)+256);
  எனில்( pCur==0 ) திரும்பு SQLITE_NOMEM;
  memset(pCur, 0, அளவு(FsdirCsr));
  *ppCursor = &pCur->base;
  திரும்பு SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
நிலையான முழு fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  எனில்( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  திரும்பு SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
நிலையான முழு fsdirNext(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  எனில்( pCsr->pDir ){
    pCsr->pEntry = readdir(pCsr->pDir);
    எனில்( pCsr->pEntry==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  திரும்பு SQLITE_OK;
}

/*
** xFilter method implementation.
*/
நிலையான முழு fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  முழு idx_இலக்கம், மாறிலி எழுத்து *idxStr,
  முழு argc, sqlite3_மதிப்பு **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  மாறிலி எழுத்து *zDir;
  முழு nDir;


  எனில்( idx_இலக்கம்!=1 || argc!=1 ){
    திரும்பு SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  எனில்( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
  nDir = sqlite3_மதிப்பு_bytes(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  எனில்( pCsr->zDir==0 ) திரும்பு SQLITE_NOMEM;
  memcpy(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  திரும்பு fsdirNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
நிலையான முழு fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  திரும்பு pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
நிலையான முழு fsdirColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, முழு i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  தேர்வு( i ){
    நிலை 0: /* dir */
      sqlite3_result_text(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      நிறுத்து;

    நிலை 1: /* name */
      sqlite3_result_text(ctx, pCsr->pEntry->d_name, -1, SQLITE_TRANSIENT);
      நிறுத்து;

    இயல்பு:
      ப்ரதிஜ்ஞா( 0 );
  }

  திரும்பு SQLITE_OK;
}

/*
** xRowid method implementation.
*/
நிலையான முழு fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  திரும்பு SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
வகைவரை கட்டமைப்பு FstreeVtab FstreeVtab;
வகைவரை கட்டமைப்பு FstreeCsr FstreeCsr;
கட்டமைப்பு FstreeVtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

கட்டமைப்பு FstreeCsr {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  முழு fd;                         /* File descriptor open on current path */
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
நிலையான முழு fstreeConnect(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  FstreeVtab *pTab;

  எனில்( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    திரும்பு SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(அளவு(FstreeVtab));
  எனில்( !pTab ) திரும்பு SQLITE_NOMEM;
  memset(pTab, 0, அளவு(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  திரும்பு SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
நிலையான முழு fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  திரும்பு SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
நிலையான முழு fstreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  முழு ii;

  க்கு(ii=0; ii<pIdxInfo->nConstraint; ii++){
    கட்டமைப்பு sqlite3_index_constraint மாறிலி *p = &pIdxInfo->aConstraint[ii];
    எனில்( p->iColumn==0 && p->usable && (
          p->op==SQLITE_INDEX_CONSTRAINT_GLOB
       || p->op==SQLITE_INDEX_CONSTRAINT_LIKE
       || p->op==SQLITE_INDEX_CONSTRAINT_EQ
    )){
      கட்டமைப்பு sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_இலக்கம் = p->op;
      pUsage->argvIndex = 1;
      pIdxInfo->estimatedCost = 100000.0;
      திரும்பு SQLITE_OK;
    }
  }

  pIdxInfo->estimatedCost = 1000000000.0;
  திரும்பு SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
நிலையான முழு fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(அளவு(FstreeCsr));
  எனில்( pCur==0 ) திரும்பு SQLITE_NOMEM;
  memset(pCur, 0, அளவு(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->base;
  திரும்பு SQLITE_OK;
}

நிலையான வெறுமை fstreeCloseFd(FstreeCsr *pCsr){
  எனில்( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
நிலையான முழு fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  திரும்பு SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
நிலையான முழு fstreeNext(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  முழு rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_step(pCsr->pStmt);
  எனில்( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }இல்லையெனில்{
    rc = SQLITE_OK;
    pCsr->fd = open((மாறிலி எழுத்து*)sqlite3_column_text(pCsr->pStmt, 0), O_RDONLY);
  }

  திரும்பு rc;
}

/*
** xFilter method implementation.
*/
நிலையான முழு fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  முழு idx_இலக்கம், மாறிலி எழுத்து *idxStr,
  முழு argc, sqlite3_மதிப்பு **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->base.pVtab);
  முழு rc;
  மாறிலி எழுத்து *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  எழுத்து *z_வேர்;
  முழு n_வேர்;
  எழுத்து *zPrefix;
  முழு nPrefix;
  மாறிலி எழுத்து *zDir;
  முழு nDir;
  எழுத்து aWild[2] = { '\0', '\0' };

#ifdef _WIN32
  மாறிலி எழுத்து *zDrive = getenv("fstreeDrive");
  எனில்( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  z_வேர் = sqlite3_mprintf("%s%c", zDrive, '/');
  n_வேர் = sqlite3Strlen30(z_வேர்);
  zPrefix = sqlite3_mprintf("%s", zDrive);
  nPrefix = sqlite3Strlen30(zPrefix);
#இல்லையெனில்
  z_வேர் = "/";
  n_வேர் = 1;
  zPrefix = "";
  nPrefix = 0;
#endif

  zDir = z_வேர்;
  nDir = n_வேர்;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  எனில்( rc!=SQLITE_OK ) திரும்பு rc;

  எனில்( idx_இலக்கம் ){
    மாறிலி எழுத்து *zQuery = (மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(argv[0]);
    தேர்வு( idx_இலக்கம் ){
      நிலை SQLITE_INDEX_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        நிறுத்து;
      நிலை SQLITE_INDEX_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        நிறுத்து;
    }

    எனில்( sqlite3_strnicmp(zQuery, zPrefix, nPrefix)==0 ){
      முழு i;
      க்கு(i=nPrefix; zQuery[i]; i++){
        எனில்( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) நிறுத்து;
        எனில்( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  எனில்( nDir==0 ) nDir = 1;

  sqlite3_bind_text(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 2, z_வேர், n_வேர், SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 3, zPrefix, nPrefix, SQLITE_TRANSIENT);

#ifdef _WIN32
  sqlite3_free(zPrefix);
  sqlite3_free(z_வேர்);
#endif

  திரும்பு fstreeNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
நிலையான முழு fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  திரும்பு pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
நிலையான முழு fstreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, முழு i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  எனில்( i==0 ){      /* path */
    sqlite3_result_மதிப்பு(ctx, sqlite3_column_மதிப்பு(pCsr->pStmt, 0));
  }இல்லையெனில்{
    கட்டமைப்பு stat sBuf;
    fstat(pCsr->fd, &sBuf);

    எனில்( S_ISREG(sBuf.st_mode) ){
      எனில்( i==1 ){
        sqlite3_result_int64(ctx, sBuf.st_size);
      }இல்லையெனில்{
        முழு nRead;
        எழுத்து *aBuf = sqlite3_malloc(sBuf.st_mode+1);
        எனில்( !aBuf ) திரும்பு SQLITE_NOMEM;
        nRead = பட(pCsr->fd, aBuf, sBuf.st_mode);
        எனில்( nRead!=sBuf.st_mode ){
          திரும்பு SQLITE_IOERR;
        }
        sqlite3_result_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  திரும்பு SQLITE_OK;
}

/*
** xRowid method implementation.
*/
நிலையான முழு fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  திரும்பு SQLITE_OK;
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
நிலையான முழு fsConnect(
  sqlite3 *db,
  வெறுமை *pAux,
  முழு argc, மாறிலி எழுத்து *மாறிலி*argv,
  sqlite3_vtab **ppVtab,
  எழுத்து **pzErr
){
  fs_vtab *pVtab;
  முழு nByte;
  மாறிலி எழுத்து *zTbl;
  மாறிலி எழுத்து *zDb = argv[1];

  எனில்( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    திரும்பு SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = அளவு(fs_vtab) + (முழு)strlen(zTbl) + 1 + (முழு)strlen(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  எனில்( !pVtab ) திரும்பு SQLITE_NOMEM;

  pVtab->zTbl = (எழுத்து *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[strlen(zTbl)+1];
  pVtab->db = db;
  memcpy(pVtab->zTbl, zTbl, strlen(zTbl));
  memcpy(pVtab->zDb, zDb, strlen(zDb));
  *ppVtab = &pVtab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  திரும்பு SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

நிலையான முழு fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  திரும்பு SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
நிலையான முழு fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(அளவு(fs_cursor));
  *ppCursor = &pCur->base;
  திரும்பு SQLITE_OK;
}

/*
** Close a fs cursor.
*/
நிலையான முழு fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  திரும்பு SQLITE_OK;
}

நிலையான முழு fsNext(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  முழு rc;

  rc = sqlite3_step(pCur->pStmt);
  எனில்( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;

  திரும்பு rc;
}

நிலையான முழு fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  முழு idx_இலக்கம், மாறிலி எழுத்து *idxStr,
  முழு argc, sqlite3_மதிப்பு **argv
){
  முழு rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  ப்ரதிஜ்ஞா( (idx_இலக்கம்==0 && argc==0) || (idx_இலக்கம்==1 && argc==1) );
  எனில்( idx_இலக்கம்==1 ){
    எழுத்து *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    எனில்( !zStmt ) திரும்பு SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    எனில்( rc==SQLITE_OK ){
      sqlite3_bind_மதிப்பு(pCur->pStmt, 1, argv[0]);
    }
  }இல்லையெனில்{
    எழுத்து *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    எனில்( !zStmt ) திரும்பு SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  எனில்( rc==SQLITE_OK ){
    rc = fsNext(pVtabCursor); 
  }
  திரும்பு rc;
}

நிலையான முழு fsColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, முழு i){
  fs_cursor *pCur = (fs_cursor*)cur;

  ப்ரதிஜ்ஞா( i==0 || i==1 || i==2 );
  எனில்( i==0 ){
    sqlite3_result_மதிப்பு(ctx, sqlite3_column_மதிப்பு(pCur->pStmt, 0));
  }இல்லையெனில்{
    மாறிலி எழுத்து *zFile = (மாறிலி எழுத்து *)sqlite3_column_text(pCur->pStmt, 1);
    கட்டமைப்பு stat sbuf;
    முழு fd;

    முழு n;
    fd = open(zFile, O_RDONLY);
    எனில்( fd<0 ) திரும்பு SQLITE_IOERR;
    fstat(fd, &sbuf);

    எனில்( sbuf.st_size>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_size*2;
      எழுத்து *zNew;
      எனில்( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      எனில்( zNew==0 ){
        close(fd);
        திரும்பு SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (முழு)பட(fd, pCur->zBuf, sbuf.st_size);
    close(fd);
    எனில்( n!=sbuf.st_size ) திரும்பு SQLITE_ERROR;
    pCur->nBuf = sbuf.st_size;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_result_text(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  திரும்பு SQLITE_OK;
}

நிலையான முழு fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_column_int64(pCur->pStmt, 0);
  திரும்பு SQLITE_OK;
}

நிலையான முழு fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  திரும்பு (sqlite3_data_மொத்தம்(pCur->pStmt)==0);
}

நிலையான முழு fsBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  முழு ii;

  க்கு(ii=0; ii<pIdxInfo->nConstraint; ii++){
    கட்டமைப்பு sqlite3_index_constraint மாறிலி *pCons = &pIdxInfo->aConstraint[ii];
    எனில்( pCons->iColumn<0 && pCons->usable
           && pCons->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      கட்டமைப்பு sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_இலக்கம் = 1;
      pIdxInfo->estimatedCost = 1.0;
      நிறுத்து;
    }
  }

  திரும்பு SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
நிலையான sqlite3_module fsModule = {
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

நிலையான sqlite3_module fsdirModule = {
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

நிலையான sqlite3_module fstreeModule = {
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
வெளி முழு getDbPointer(Tcl_Interp *interp, மாறிலி எழுத்து *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
நிலையான முழு SQLITE_TCLAPI register_fs_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  முழு objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (வெறுமை *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#endif
  திரும்பு TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
முழு Sqlitetestfs_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  நிலையான கட்டமைப்பு {
     எழுத்து *zName;
     Tcl_ObjCmdProc *xProc;
     வெறுமை *clientData;
  } aObjCmd[] = {
     { "register_fs_module",   register_fs_module, 0 },
  };
  முழு i;
  க்கு(i=0; i<அளவு(aObjCmd)/அளவு(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  திரும்பு TCL_OK;
}
