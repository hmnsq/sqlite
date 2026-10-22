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
#ورنہ
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(منوال) (((منوال) & S_IFMT) == S_IFREG)
# endif
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE

قسمتعریف ساخت fs_vtab fs_vtab;
قسمتعریف ساخت fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
ساخت fs_vtab {
  sqlite3_vtab بنیاد;
  sqlite3 *db;
  حرف *zDb;                      /* Name of db containing zTbl */
  حرف *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
ساخت fs_cursor {
  sqlite3_vtab_cursor بنیاد;
  sqlite3_stmt *pStmt;
  حرف *zBuf;
  صحیح nBuf;
  صحیح nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
قسمتعریف ساخت FsdirVtab FsdirVtab;
قسمتعریف ساخت FsdirCsr FsdirCsr;
ساخت FsdirVtab {
  sqlite3_vtab بنیاد;
};

ساخت FsdirCsr {
  sqlite3_vtab_cursor بنیاد;
  حرف *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  ساخت dirent *p_اندراج;
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
ساکن صحیح fsdirConnect(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  FsdirVtab *pTab;

  اگر( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    واپس SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(حجم(FsdirVtab));
  اگر( !pTab ) واپس SQLITE_NOMEM;
  memset(pTab, 0, حجم(FsdirVtab));

  *ppVtab = &pTab->بنیاد;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  واپس SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
ساکن صحیح fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  واپس SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
ساکن صحیح fsdir_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  صحیح ii;

  pIdxInfo->estimated_لاگت = 1000000000.0;

  برائے(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ساخت sqlite3_اشاریہ_constraint ثابت *p = &pIdxInfo->aConstraint[ii];
    اگر( p->i_عمود==0 && p->usable && p->op==SQLITE_اشاریہ_CONSTRAINT_EQ ){
      ساخت sqlite3_اشاریہ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argv_اشاریہ = 1;
      pIdxInfo->idx_عدد = 1;
      pIdxInfo->estimated_لاگت = 1.0;
      توڑو;
    }
  }

  واپس SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
ساکن صحیح fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(حجم(FsdirCsr)+256);
  اگر( pCur==0 ) واپس SQLITE_NOMEM;
  memset(pCur, 0, حجم(FsdirCsr));
  *ppCursor = &pCur->بنیاد;
  واپس SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
ساکن صحیح fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  اگر( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  واپس SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
ساکن صحیح fsdir_اگلا(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  اگر( pCsr->pDir ){
    pCsr->p_اندراج = readdir(pCsr->pDir);
    اگر( pCsr->p_اندراج==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  واپس SQLITE_OK;
}

/*
** xFilter method implementation.
*/
ساکن صحیح fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_عدد, ثابت حرف *idxStr,
  صحیح argc, sqlite3_قدر **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  ثابت حرف *zDir;
  صحیح nDir;


  اگر( idx_عدد!=1 || argc!=1 ){
    واپس SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  اگر( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  nDir = sqlite3_قدر_بائٹیں(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  اگر( pCsr->zDir==0 ) واپس SQLITE_NOMEM;
  حافظہ_نقل(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  واپس fsdir_اگلا(pVtabCursor); 
}

/*
** xEof method implementation.
*/
ساکن صحیح fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  واپس pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
ساکن صحیح fsdir_عمود(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحیح i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  انتخاب( i ){
    حالت 0: /* dir */
      sqlite3_نتیجہ_متن(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      توڑو;

    حالت 1: /* name */
      sqlite3_نتیجہ_متن(ctx, pCsr->p_اندراج->d_name, -1, SQLITE_TRANSIENT);
      توڑو;

    طےشدہ:
      أكد( 0 );
  }

  واپس SQLITE_OK;
}

/*
** xRowid method implementation.
*/
ساکن صحیح fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  واپس SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
قسمتعریف ساخت FstreeVtab FstreeVtab;
قسمتعریف ساخت FstreeCsr FstreeCsr;
ساخت FstreeVtab {
  sqlite3_vtab بنیاد;
  sqlite3 *db;
};

ساخت FstreeCsr {
  sqlite3_vtab_cursor بنیاد;
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
ساکن صحیح fstreeConnect(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  FstreeVtab *pTab;

  اگر( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    واپس SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(حجم(FstreeVtab));
  اگر( !pTab ) واپس SQLITE_NOMEM;
  memset(pTab, 0, حجم(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->بنیاد;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  واپس SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
ساکن صحیح fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  واپس SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
ساکن صحیح fstree_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  صحیح ii;

  برائے(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ساخت sqlite3_اشاریہ_constraint ثابت *p = &pIdxInfo->aConstraint[ii];
    اگر( p->i_عمود==0 && p->usable && (
          p->op==SQLITE_اشاریہ_CONSTRAINT_GLOB
       || p->op==SQLITE_اشاریہ_CONSTRAINT_LIKE
       || p->op==SQLITE_اشاریہ_CONSTRAINT_EQ
    )){
      ساخت sqlite3_اشاریہ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_عدد = p->op;
      pUsage->argv_اشاریہ = 1;
      pIdxInfo->estimated_لاگت = 100000.0;
      واپس SQLITE_OK;
    }
  }

  pIdxInfo->estimated_لاگت = 1000000000.0;
  واپس SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
ساکن صحیح fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(حجم(FstreeCsr));
  اگر( pCur==0 ) واپس SQLITE_NOMEM;
  memset(pCur, 0, حجم(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->بنیاد;
  واپس SQLITE_OK;
}

ساکن خالی fstreeCloseFd(FstreeCsr *pCsr){
  اگر( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
ساکن صحیح fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  واپس SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
ساکن صحیح fstree_اگلا(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  صحیح rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_قدم(pCsr->pStmt);
  اگر( rc!=SQLITE_صف ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }ورنہ{
    rc = SQLITE_OK;
    pCsr->fd = open((ثابت حرف*)sqlite3_عمود_متن(pCsr->pStmt, 0), O_RDONLY);
  }

  واپس rc;
}

/*
** xFilter method implementation.
*/
ساکن صحیح fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_عدد, ثابت حرف *idxStr,
  صحیح argc, sqlite3_قدر **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->بنیاد.pVtab);
  صحیح rc;
  ثابت حرف *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  حرف *z_جڑ;
  صحیح n_جڑ;
  حرف *z_سابقہ;
  صحیح n_سابقہ;
  ثابت حرف *zDir;
  صحیح nDir;
  حرف aWild[2] = { '\0', '\0' };

#اگر_تعریف_شدہ _WIN32
  ثابت حرف *zDrive = getenv("fstreeDrive");
  اگر( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  z_جڑ = sqlite3_mprintf("%s%c", zDrive, '/');
  n_جڑ = sqlite3Strlen30(z_جڑ);
  z_سابقہ = sqlite3_mprintf("%s", zDrive);
  n_سابقہ = sqlite3Strlen30(z_سابقہ);
#ورنہ
  z_جڑ = "/";
  n_جڑ = 1;
  z_سابقہ = "";
  n_سابقہ = 0;
#اگر_ختم

  zDir = z_جڑ;
  nDir = n_جڑ;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  اگر( rc!=SQLITE_OK ) واپس rc;

  اگر( idx_عدد ){
    ثابت حرف *zQuery = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
    انتخاب( idx_عدد ){
      حالت SQLITE_اشاریہ_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        توڑو;
      حالت SQLITE_اشاریہ_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        توڑو;
    }

    اگر( sqlite3_strnicmp(zQuery, z_سابقہ, n_سابقہ)==0 ){
      صحیح i;
      برائے(i=n_سابقہ; zQuery[i]; i++){
        اگر( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) توڑو;
        اگر( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  اگر( nDir==0 ) nDir = 1;

  sqlite3_bind_متن(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_متن(pCsr->pStmt, 2, z_جڑ, n_جڑ, SQLITE_TRANSIENT);
  sqlite3_bind_متن(pCsr->pStmt, 3, z_سابقہ, n_سابقہ, SQLITE_TRANSIENT);

#اگر_تعریف_شدہ _WIN32
  sqlite3_free(z_سابقہ);
  sqlite3_free(z_جڑ);
#اگر_ختم

  واپس fstree_اگلا(pVtabCursor); 
}

/*
** xEof method implementation.
*/
ساکن صحیح fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  واپس pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
ساکن صحیح fstree_عمود(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحیح i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  اگر( i==0 ){      /*راستہ*/
    sqlite3_نتیجہ_قدر(ctx, sqlite3_عمود_قدر(pCsr->pStmt, 0));
  }ورنہ{
    ساخت stat sBuf;
    fstat(pCsr->fd, &sBuf);

    اگر( S_ISREG(sBuf.st_منوال) ){
      اگر( i==1 ){
        sqlite3_نتیجہ_int64(ctx, sBuf.st_سائز);
      }ورنہ{
        صحیح nRead;
        حرف *aBuf = sqlite3_malloc(sBuf.st_منوال+1);
        اگر( !aBuf ) واپس SQLITE_NOMEM;
        nRead = اقرأ(pCsr->fd, aBuf, sBuf.st_منوال);
        اگر( nRead!=sBuf.st_منوال ){
          واپس SQLITE_IOERR;
        }
        sqlite3_نتیجہ_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  واپس SQLITE_OK;
}

/*
** xRowid method implementation.
*/
ساکن صحیح fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  واپس SQLITE_OK;
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
ساکن صحیح fsConnect(
  sqlite3 *db,
  خالی *pAux,
  صحیح argc, ثابت حرف *ثابت*argv,
  sqlite3_vtab **ppVtab,
  حرف **pzErr
){
  fs_vtab *pVtab;
  صحیح nByte;
  ثابت حرف *zTbl;
  ثابت حرف *zDb = argv[1];

  اگر( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    واپس SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = حجم(fs_vtab) + (صحیح)لڑی_لمبائی(zTbl) + 1 + (صحیح)لڑی_لمبائی(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  اگر( !pVtab ) واپس SQLITE_NOMEM;

  pVtab->zTbl = (حرف *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[لڑی_لمبائی(zTbl)+1];
  pVtab->db = db;
  حافظہ_نقل(pVtab->zTbl, zTbl, لڑی_لمبائی(zTbl));
  حافظہ_نقل(pVtab->zDb, zDb, لڑی_لمبائی(zDb));
  *ppVtab = &pVtab->بنیاد;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  واپس SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

ساکن صحیح fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  واپس SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
ساکن صحیح fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(حجم(fs_cursor));
  *ppCursor = &pCur->بنیاد;
  واپس SQLITE_OK;
}

/*
** Close a fs cursor.
*/
ساکن صحیح fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  واپس SQLITE_OK;
}

ساکن صحیح fs_اگلا(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  صحیح rc;

  rc = sqlite3_قدم(pCur->pStmt);
  اگر( rc==SQLITE_صف || rc==SQLITE_مکمل ) rc = SQLITE_OK;

  واپس rc;
}

ساکن صحیح fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحیح idx_عدد, ثابت حرف *idxStr,
  صحیح argc, sqlite3_قدر **argv
){
  صحیح rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  أكد( (idx_عدد==0 && argc==0) || (idx_عدد==1 && argc==1) );
  اگر( idx_عدد==1 ){
    حرف *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    اگر( !zStmt ) واپس SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    اگر( rc==SQLITE_OK ){
      sqlite3_bind_قدر(pCur->pStmt, 1, argv[0]);
    }
  }ورنہ{
    حرف *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    اگر( !zStmt ) واپس SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  اگر( rc==SQLITE_OK ){
    rc = fs_اگلا(pVtabCursor); 
  }
  واپس rc;
}

ساکن صحیح fs_عمود(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحیح i){
  fs_cursor *pCur = (fs_cursor*)cur;

  أكد( i==0 || i==1 || i==2 );
  اگر( i==0 ){
    sqlite3_نتیجہ_قدر(ctx, sqlite3_عمود_قدر(pCur->pStmt, 0));
  }ورنہ{
    ثابت حرف *zFile = (ثابت حرف *)sqlite3_عمود_متن(pCur->pStmt, 1);
    ساخت stat sbuf;
    صحیح fd;

    صحیح n;
    fd = open(zFile, O_RDONLY);
    اگر( fd<0 ) واپس SQLITE_IOERR;
    fstat(fd, &sbuf);

    اگر( sbuf.st_سائز>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_سائز*2;
      حرف *zNew;
      اگر( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      اگر( zNew==0 ){
        close(fd);
        واپس SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (صحیح)اقرأ(fd, pCur->zBuf, sbuf.st_سائز);
    close(fd);
    اگر( n!=sbuf.st_سائز ) واپس SQLITE_ERROR;
    pCur->nBuf = sbuf.st_سائز;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_نتیجہ_متن(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  واپس SQLITE_OK;
}

ساکن صحیح fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_عمود_int64(pCur->pStmt, 0);
  واپس SQLITE_OK;
}

ساکن صحیح fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  واپس (sqlite3_ڈیٹا_گنتی(pCur->pStmt)==0);
}

ساکن صحیح fs_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  صحیح ii;

  برائے(ii=0; ii<pIdxInfo->nConstraint; ii++){
    ساخت sqlite3_اشاریہ_constraint ثابت *pCons = &pIdxInfo->aConstraint[ii];
    اگر( pCons->i_عمود<0 && pCons->usable
           && pCons->op==SQLITE_اشاریہ_CONSTRAINT_EQ ){
      ساخت sqlite3_اشاریہ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argv_اشاریہ = 1;
      pIdxInfo->idx_عدد = 1;
      pIdxInfo->estimated_لاگت = 1.0;
      توڑو;
    }
  }

  واپس SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
ساکن sqlite3_module fsModule = {
  0,                         /* iVersion */
  fsConnect,
  fsConnect,
  fs_بہترین_اشاریہ,
  fsDisconnect, 
  fsDisconnect,
  fsOpen,                      /* xOpen - open a cursor */
  fsClose,                     /* xClose - close a cursor */
  fsFilter,                    /* xFilter - configure scan constraints */
  fs_اگلا,                      /* xNext - advance a cursor */
  fsEof,                       /* xEof - check for end of scan */
  fs_عمود,                    /* xColumn - read data */
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

ساکن sqlite3_module fsdirModule = {
  0,                              /* iVersion */
  fsdirConnect,                   /* xCreate */
  fsdirConnect,                   /* xConnect */
  fsdir_بہترین_اشاریہ,                 /* xBestIndex */
  fsdirDisconnect,                /* xDisconnect */
  fsdirDisconnect,                /* xDestroy */
  fsdirOpen,                      /* xOpen - open a cursor */
  fsdirClose,                     /* xClose - close a cursor */
  fsdirFilter,                    /* xFilter - configure scan constraints */
  fsdir_اگلا,                      /* xNext - advance a cursor */
  fsdirEof,                       /* xEof - check for end of scan */
  fsdir_عمود,                    /* xColumn - read data */
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

ساکن sqlite3_module fstreeModule = {
  0,                              /* iVersion */
  fstreeConnect,                  /* xCreate */
  fstreeConnect,                  /* xConnect */
  fstree_بہترین_اشاریہ,                /* xBestIndex */
  fstreeDisconnect,               /* xDisconnect */
  fstreeDisconnect,               /* xDestroy */
  fstreeOpen,                     /* xOpen - open a cursor */
  fstreeClose,                    /* xClose - close a cursor */
  fstreeFilter,                   /* xFilter - configure scan constraints */
  fstree_اگلا,                     /* xNext - advance a cursor */
  fstreeEof,                      /* xEof - check for end of scan */
  fstree_عمود,                   /* xColumn - read data */
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
خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ساکن صحیح SQLITE_TCLAPI رجسٹر_fs_module(
  Client_ڈیٹا client_ڈیٹا, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحیح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (خالی *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#اگر_ختم
  واپس TCL_OK;
}

#اگر_ختم


/*
** Register commands with the TCL interpreter.
*/
صحیح Sqlitetestfs_Init(Tcl_Interp *interp){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     خالی *client_ڈیٹا;
  } aObjCmd[] = {
     { "register_fs_module",   رجسٹر_fs_module, 0 },
  };
  صحیح i;
  برائے(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_ڈیٹا, 0);
  }
#اگر_ختم
  واپس TCL_OK;
}
