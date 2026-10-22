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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <fcntl.h>

#यदि !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#अन्यथा
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
# endif
#endif

#ifndef SQLITE_OMIT_VIRTUALTABLE

प्रकारनिर्देश संरचना fs_vtab fs_vtab;
प्रकारनिर्देश संरचना fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
संरचना fs_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
  वर्ण *zDb;                      /* Name of db containing zTbl */
  वर्ण *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
संरचना fs_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
  वर्ण *zBuf;
  पूर्णाङ्क nBuf;
  पूर्णाङ्क nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
प्रकारनिर्देश संरचना FsdirVtab FsdirVtab;
प्रकारनिर्देश संरचना FsdirCsr FsdirCsr;
संरचना FsdirVtab {
  sqlite3_vtab base;
};

संरचना FsdirCsr {
  sqlite3_vtab_cursor base;
  वर्ण *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  संरचना dirent *pEntry;
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
स्थिर पूर्णाङ्क fsdirConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  FsdirVtab *pTab;

  यदि( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    प्रत्यागम SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(परिमाण(FsdirVtab));
  यदि( !pTab ) प्रत्यागम SQLITE_NOMEM;
  memset(pTab, 0, परिमाण(FsdirVtab));

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  प्रत्यागम SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
स्थिर पूर्णाङ्क fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  प्रत्यागम SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
स्थिर पूर्णाङ्क fsdirBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  पूर्णाङ्क ii;

  pIdxInfo->estimatedCost = 1000000000.0;

  प्रति(ii=0; ii<pIdxInfo->nConstraint; ii++){
    संरचना sqlite3_index_constraint ध्रुव *p = &pIdxInfo->aConstraint[ii];
    यदि( p->iColumn==0 && p->usable && p->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      संरचना sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_अंक = 1;
      pIdxInfo->estimatedCost = 1.0;
      भङ्ग;
    }
  }

  प्रत्यागम SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
स्थिर पूर्णाङ्क fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(परिमाण(FsdirCsr)+256);
  यदि( pCur==0 ) प्रत्यागम SQLITE_NOMEM;
  memset(pCur, 0, परिमाण(FsdirCsr));
  *ppCursor = &pCur->base;
  प्रत्यागम SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
स्थिर पूर्णाङ्क fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  यदि( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  प्रत्यागम SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
स्थिर पूर्णाङ्क fsdirNext(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  यदि( pCsr->pDir ){
    pCsr->pEntry = readdir(pCsr->pDir);
    यदि( pCsr->pEntry==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  प्रत्यागम SQLITE_OK;
}

/*
** xFilter method implementation.
*/
स्थिर पूर्णाङ्क fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णाङ्क idx_अंक, ध्रुव वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मूल्य **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  ध्रुव वर्ण *zDir;
  पूर्णाङ्क nDir;


  यदि( idx_अंक!=1 || argc!=1 ){
    प्रत्यागम SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  यदि( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[0]);
  nDir = sqlite3_मूल्य_bytes(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  यदि( pCsr->zDir==0 ) प्रत्यागम SQLITE_NOMEM;
  memcpy(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  प्रत्यागम fsdirNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
स्थिर पूर्णाङ्क fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  प्रत्यागम pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
स्थिर पूर्णाङ्क fsdirColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णाङ्क i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  विकल्प( i ){
    पक्ष 0: /* dir */
      sqlite3_result_text(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      भङ्ग;

    पक्ष 1: /* name */
      sqlite3_result_text(ctx, pCsr->pEntry->d_name, -1, SQLITE_TRANSIENT);
      भङ्ग;

    स्वभाव:
      प्रतिज्ञा( 0 );
  }

  प्रत्यागम SQLITE_OK;
}

/*
** xRowid method implementation.
*/
स्थिर पूर्णाङ्क fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  प्रत्यागम SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
प्रकारनिर्देश संरचना FstreeVtab FstreeVtab;
प्रकारनिर्देश संरचना FstreeCsr FstreeCsr;
संरचना FstreeVtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

संरचना FstreeCsr {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  पूर्णाङ्क fd;                         /* File descriptor open on current path */
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
स्थिर पूर्णाङ्क fstreeConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  FstreeVtab *pTab;

  यदि( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    प्रत्यागम SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(परिमाण(FstreeVtab));
  यदि( !pTab ) प्रत्यागम SQLITE_NOMEM;
  memset(pTab, 0, परिमाण(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  प्रत्यागम SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
स्थिर पूर्णाङ्क fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  प्रत्यागम SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
स्थिर पूर्णाङ्क fstreeBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  पूर्णाङ्क ii;

  प्रति(ii=0; ii<pIdxInfo->nConstraint; ii++){
    संरचना sqlite3_index_constraint ध्रुव *p = &pIdxInfo->aConstraint[ii];
    यदि( p->iColumn==0 && p->usable && (
          p->op==SQLITE_INDEX_CONSTRAINT_GLOB
       || p->op==SQLITE_INDEX_CONSTRAINT_LIKE
       || p->op==SQLITE_INDEX_CONSTRAINT_EQ
    )){
      संरचना sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_अंक = p->op;
      pUsage->argvIndex = 1;
      pIdxInfo->estimatedCost = 100000.0;
      प्रत्यागम SQLITE_OK;
    }
  }

  pIdxInfo->estimatedCost = 1000000000.0;
  प्रत्यागम SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
स्थिर पूर्णाङ्क fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(परिमाण(FstreeCsr));
  यदि( pCur==0 ) प्रत्यागम SQLITE_NOMEM;
  memset(pCur, 0, परिमाण(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->base;
  प्रत्यागम SQLITE_OK;
}

स्थिर शून्य fstreeCloseFd(FstreeCsr *pCsr){
  यदि( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
स्थिर पूर्णाङ्क fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  प्रत्यागम SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
स्थिर पूर्णाङ्क fstreeNext(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  पूर्णाङ्क rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_step(pCsr->pStmt);
  यदि( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }अन्यथा{
    rc = SQLITE_OK;
    pCsr->fd = open((ध्रुव वर्ण*)sqlite3_column_text(pCsr->pStmt, 0), O_RDONLY);
  }

  प्रत्यागम rc;
}

/*
** xFilter method implementation.
*/
स्थिर पूर्णाङ्क fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णाङ्क idx_अंक, ध्रुव वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मूल्य **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->base.pVtab);
  पूर्णाङ्क rc;
  ध्रुव वर्ण *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  वर्ण *z_मूळ;
  पूर्णाङ्क n_मूळ;
  वर्ण *zPrefix;
  पूर्णाङ्क nPrefix;
  ध्रुव वर्ण *zDir;
  पूर्णाङ्क nDir;
  वर्ण aWild[2] = { '\0', '\0' };

#ifdef _WIN32
  ध्रुव वर्ण *zDrive = getenv("fstreeDrive");
  यदि( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  z_मूळ = sqlite3_mprintf("%s%c", zDrive, '/');
  n_मूळ = sqlite3Strlen30(z_मूळ);
  zPrefix = sqlite3_mprintf("%s", zDrive);
  nPrefix = sqlite3Strlen30(zPrefix);
#अन्यथा
  z_मूळ = "/";
  n_मूळ = 1;
  zPrefix = "";
  nPrefix = 0;
#endif

  zDir = z_मूळ;
  nDir = n_मूळ;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  यदि( rc!=SQLITE_OK ) प्रत्यागम rc;

  यदि( idx_अंक ){
    ध्रुव वर्ण *zQuery = (ध्रुव वर्ण*)sqlite3_मूल्य_text(argv[0]);
    विकल्प( idx_अंक ){
      पक्ष SQLITE_INDEX_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        भङ्ग;
      पक्ष SQLITE_INDEX_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        भङ्ग;
    }

    यदि( sqlite3_strnicmp(zQuery, zPrefix, nPrefix)==0 ){
      पूर्णाङ्क i;
      प्रति(i=nPrefix; zQuery[i]; i++){
        यदि( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) भङ्ग;
        यदि( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  यदि( nDir==0 ) nDir = 1;

  sqlite3_bind_text(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 2, z_मूळ, n_मूळ, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 3, zPrefix, nPrefix, SQLITE_TRANSIENT);

#ifdef _WIN32
  sqlite3_free(zPrefix);
  sqlite3_free(z_मूळ);
#endif

  प्रत्यागम fstreeNext(pVtabCursor); 
}

/*
** xEof method implementation.
*/
स्थिर पूर्णाङ्क fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  प्रत्यागम pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
स्थिर पूर्णाङ्क fstreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णाङ्क i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  यदि( i==0 ){      /* path */
    sqlite3_result_मूल्य(ctx, sqlite3_column_मूल्य(pCsr->pStmt, 0));
  }अन्यथा{
    संरचना stat sBuf;
    fstat(pCsr->fd, &sBuf);

    यदि( S_ISREG(sBuf.st_mode) ){
      यदि( i==1 ){
        sqlite3_result_int64(ctx, sBuf.st_size);
      }अन्यथा{
        पूर्णाङ्क nRead;
        वर्ण *aBuf = sqlite3_malloc(sBuf.st_mode+1);
        यदि( !aBuf ) प्रत्यागम SQLITE_NOMEM;
        nRead = पठ(pCsr->fd, aBuf, sBuf.st_mode);
        यदि( nRead!=sBuf.st_mode ){
          प्रत्यागम SQLITE_IOERR;
        }
        sqlite3_result_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  प्रत्यागम SQLITE_OK;
}

/*
** xRowid method implementation.
*/
स्थिर पूर्णाङ्क fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  प्रत्यागम SQLITE_OK;
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
स्थिर पूर्णाङ्क fsConnect(
  sqlite3 *db,
  शून्य *pAux,
  पूर्णाङ्क argc, ध्रुव वर्ण *ध्रुव*argv,
  sqlite3_vtab **ppVtab,
  वर्ण **pzErr
){
  fs_vtab *pVtab;
  पूर्णाङ्क nByte;
  ध्रुव वर्ण *zTbl;
  ध्रुव वर्ण *zDb = argv[1];

  यदि( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    प्रत्यागम SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = परिमाण(fs_vtab) + (पूर्णाङ्क)strlen(zTbl) + 1 + (पूर्णाङ्क)strlen(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  यदि( !pVtab ) प्रत्यागम SQLITE_NOMEM;

  pVtab->zTbl = (वर्ण *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[strlen(zTbl)+1];
  pVtab->db = db;
  memcpy(pVtab->zTbl, zTbl, strlen(zTbl));
  memcpy(pVtab->zDb, zDb, strlen(zDb));
  *ppVtab = &pVtab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  प्रत्यागम SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

स्थिर पूर्णाङ्क fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  प्रत्यागम SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
स्थिर पूर्णाङ्क fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(परिमाण(fs_cursor));
  *ppCursor = &pCur->base;
  प्रत्यागम SQLITE_OK;
}

/*
** Close a fs cursor.
*/
स्थिर पूर्णाङ्क fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क fsNext(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  पूर्णाङ्क rc;

  rc = sqlite3_step(pCur->pStmt);
  यदि( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;

  प्रत्यागम rc;
}

स्थिर पूर्णाङ्क fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णाङ्क idx_अंक, ध्रुव वर्ण *idxStr,
  पूर्णाङ्क argc, sqlite3_मूल्य **argv
){
  पूर्णाङ्क rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  प्रतिज्ञा( (idx_अंक==0 && argc==0) || (idx_अंक==1 && argc==1) );
  यदि( idx_अंक==1 ){
    वर्ण *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    यदि( !zStmt ) प्रत्यागम SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    यदि( rc==SQLITE_OK ){
      sqlite3_bind_मूल्य(pCur->pStmt, 1, argv[0]);
    }
  }अन्यथा{
    वर्ण *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    यदि( !zStmt ) प्रत्यागम SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  यदि( rc==SQLITE_OK ){
    rc = fsNext(pVtabCursor); 
  }
  प्रत्यागम rc;
}

स्थिर पूर्णाङ्क fsColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णाङ्क i){
  fs_cursor *pCur = (fs_cursor*)cur;

  प्रतिज्ञा( i==0 || i==1 || i==2 );
  यदि( i==0 ){
    sqlite3_result_मूल्य(ctx, sqlite3_column_मूल्य(pCur->pStmt, 0));
  }अन्यथा{
    ध्रुव वर्ण *zFile = (ध्रुव वर्ण *)sqlite3_column_text(pCur->pStmt, 1);
    संरचना stat sbuf;
    पूर्णाङ्क fd;

    पूर्णाङ्क n;
    fd = open(zFile, O_RDONLY);
    यदि( fd<0 ) प्रत्यागम SQLITE_IOERR;
    fstat(fd, &sbuf);

    यदि( sbuf.st_size>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_size*2;
      वर्ण *zNew;
      यदि( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      यदि( zNew==0 ){
        close(fd);
        प्रत्यागम SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (पूर्णाङ्क)पठ(fd, pCur->zBuf, sbuf.st_size);
    close(fd);
    यदि( n!=sbuf.st_size ) प्रत्यागम SQLITE_ERROR;
    pCur->nBuf = sbuf.st_size;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_result_text(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_column_int64(pCur->pStmt, 0);
  प्रत्यागम SQLITE_OK;
}

स्थिर पूर्णाङ्क fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  प्रत्यागम (sqlite3_data_मोजणी(pCur->pStmt)==0);
}

स्थिर पूर्णाङ्क fsBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  पूर्णाङ्क ii;

  प्रति(ii=0; ii<pIdxInfo->nConstraint; ii++){
    संरचना sqlite3_index_constraint ध्रुव *pCons = &pIdxInfo->aConstraint[ii];
    यदि( pCons->iColumn<0 && pCons->usable
           && pCons->op==SQLITE_INDEX_CONSTRAINT_EQ ){
      संरचना sqlite3_index_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argvIndex = 1;
      pIdxInfo->idx_अंक = 1;
      pIdxInfo->estimatedCost = 1.0;
      भङ्ग;
    }
  }

  प्रत्यागम SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
स्थिर sqlite3_module fsModule = {
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

स्थिर sqlite3_module fsdirModule = {
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

स्थिर sqlite3_module fstreeModule = {
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
बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp *interp, ध्रुव वर्ण *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI register_fs_module(
  ClientData clientData, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णाङ्क objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  यदि( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    प्रत्यागम TCL_ERROR;
  }
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) प्रत्यागम TCL_ERROR;
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (शून्य *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#endif
  प्रत्यागम TCL_OK;
}

#endif


/*
** Register commands with the TCL interpreter.
*/
पूर्णाङ्क Sqlitetestfs_Init(Tcl_Interp *interp){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  स्थिर संरचना {
     वर्ण *zName;
     Tcl_ObjCmdProc *xProc;
     शून्य *clientData;
  } aObjCmd[] = {
     { "register_fs_module",   register_fs_module, 0 },
  };
  पूर्णाङ्क i;
  प्रति(i=0; i<परिमाण(aObjCmd)/परिमाण(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].clientData, 0);
  }
#endif
  प्रत्यागम TCL_OK;
}
