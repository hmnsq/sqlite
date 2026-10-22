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
#  define S_ISREG(منوال) (((منوال) & S_IFMT) == S_IFREG)
# endif
#جے_انت

#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE

تعريفنوع بنية fs_vtab fs_vtab;
تعريفنوع بنية fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
بنية fs_vtab {
  sqlite3_vtab ادھار;
  sqlite3 *db;
  حرف *zDb;                      /* Name of db containing zTbl */
  حرف *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
بنية fs_cursor {
  sqlite3_vtab_cursor ادھار;
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
  sqlite3_vtab ادھار;
};

بنية FsdirCsr {
  sqlite3_vtab_cursor ادھار;
  حرف *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  بنية dirent *p_اندراج;
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

  *ppVtab = &pTab->ادھار;
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
ساكن صحيح fsdir_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  صحيح ii;

  pIdxInfo->estimated_لاگت = 1000000000.0;

  لكل(ii=0; ii<pIdxInfo->nConstraint; ii++){
    بنية sqlite3_اشاریہ_constraint ثابت *p = &pIdxInfo->aConstraint[ii];
    إذا( p->i_کالم==0 && p->usable && p->op==SQLITE_اشاریہ_CONSTRAINT_EQ ){
      بنية sqlite3_اشاریہ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argv_اشاریہ = 1;
      pIdxInfo->idx_عدد = 1;
      pIdxInfo->estimated_لاگت = 1.0;
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
  *ppCursor = &pCur->ادھار;
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
ساكن صحيح fsdir_اگلا(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  إذا( pCsr->pDir ){
    pCsr->p_اندراج = readdir(pCsr->pDir);
    إذا( pCsr->p_اندراج==0 ){
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
  صحيح idx_عدد, ثابت حرف *idxStr,
  صحيح argc, sqlite3_قدر **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  ثابت حرف *zDir;
  صحيح nDir;


  إذا( idx_عدد!=1 || argc!=1 ){
    أرجع SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  إذا( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
  nDir = sqlite3_قدر_بائٹاں(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  إذا( pCsr->zDir==0 ) أرجع SQLITE_NOMEM;
  حافظہ_نقل(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  أرجع fsdir_اگلا(pVtabCursor); 
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
ساكن صحيح fsdir_کالم(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحيح i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  بدل( i ){
    حالة 0: /* dir */
      sqlite3_نتیجہ_متن(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      اكسر;

    حالة 1: /* name */
      sqlite3_نتیجہ_متن(ctx, pCsr->p_اندراج->d_name, -1, SQLITE_TRANSIENT);
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
  sqlite3_vtab ادھار;
  sqlite3 *db;
};

بنية FstreeCsr {
  sqlite3_vtab_cursor ادھار;
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

  *ppVtab = &pTab->ادھار;
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
ساكن صحيح fstree_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  صحيح ii;

  لكل(ii=0; ii<pIdxInfo->nConstraint; ii++){
    بنية sqlite3_اشاریہ_constraint ثابت *p = &pIdxInfo->aConstraint[ii];
    إذا( p->i_کالم==0 && p->usable && (
          p->op==SQLITE_اشاریہ_CONSTRAINT_GLOB
       || p->op==SQLITE_اشاریہ_CONSTRAINT_LIKE
       || p->op==SQLITE_اشاریہ_CONSTRAINT_EQ
    )){
      بنية sqlite3_اشاریہ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_عدد = p->op;
      pUsage->argv_اشاریہ = 1;
      pIdxInfo->estimated_لاگت = 100000.0;
      أرجع SQLITE_OK;
    }
  }

  pIdxInfo->estimated_لاگت = 1000000000.0;
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
  *ppCursor = &pCur->ادھار;
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
ساكن صحيح fstree_اگلا(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  صحيح rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_قدم(pCsr->pStmt);
  إذا( rc!=SQLITE_صف ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }وإلا{
    rc = SQLITE_OK;
    pCsr->fd = open((ثابت حرف*)sqlite3_کالم_متن(pCsr->pStmt, 0), O_RDONLY);
  }

  أرجع rc;
}

/*
** xFilter method implementation.
*/
ساكن صحيح fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحيح idx_عدد, ثابت حرف *idxStr,
  صحيح argc, sqlite3_قدر **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->ادھار.pVtab);
  صحيح rc;
  ثابت حرف *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  حرف *z_جڑ;
  صحيح n_جڑ;
  حرف *z_اگیتر;
  صحيح n_اگیتر;
  ثابت حرف *zDir;
  صحيح nDir;
  حرف aWild[2] = { '\0', '\0' };

#جے_تعریف_شدہ _WIN32
  ثابت حرف *zDrive = getenv("fstreeDrive");
  إذا( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  z_جڑ = sqlite3_mprintf("%s%c", zDrive, '/');
  n_جڑ = sqlite3Strlen30(z_جڑ);
  z_اگیتر = sqlite3_mprintf("%s", zDrive);
  n_اگیتر = sqlite3Strlen30(z_اگیتر);
#وإلا
  z_جڑ = "/";
  n_جڑ = 1;
  z_اگیتر = "";
  n_اگیتر = 0;
#جے_انت

  zDir = z_جڑ;
  nDir = n_جڑ;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  إذا( rc!=SQLITE_OK ) أرجع rc;

  إذا( idx_عدد ){
    ثابت حرف *zQuery = (ثابت حرف*)sqlite3_قدر_متن(argv[0]);
    بدل( idx_عدد ){
      حالة SQLITE_اشاریہ_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        اكسر;
      حالة SQLITE_اشاریہ_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        اكسر;
    }

    إذا( sqlite3_strnicmp(zQuery, z_اگیتر, n_اگیتر)==0 ){
      صحيح i;
      لكل(i=n_اگیتر; zQuery[i]; i++){
        إذا( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) اكسر;
        إذا( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  إذا( nDir==0 ) nDir = 1;

  sqlite3_bind_متن(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_متن(pCsr->pStmt, 2, z_جڑ, n_جڑ, SQLITE_TRANSIENT);
  sqlite3_bind_متن(pCsr->pStmt, 3, z_اگیتر, n_اگیتر, SQLITE_TRANSIENT);

#جے_تعریف_شدہ _WIN32
  sqlite3_free(z_اگیتر);
  sqlite3_free(z_جڑ);
#جے_انت

  أرجع fstree_اگلا(pVtabCursor); 
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
ساكن صحيح fstree_کالم(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحيح i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  إذا( i==0 ){      /*رستہ*/
    sqlite3_نتیجہ_قدر(ctx, sqlite3_کالم_قدر(pCsr->pStmt, 0));
  }وإلا{
    بنية stat sBuf;
    fstat(pCsr->fd, &sBuf);

    إذا( S_ISREG(sBuf.st_منوال) ){
      إذا( i==1 ){
        sqlite3_نتیجہ_int64(ctx, sBuf.st_ناپ);
      }وإلا{
        صحيح nRead;
        حرف *aBuf = sqlite3_malloc(sBuf.st_منوال+1);
        إذا( !aBuf ) أرجع SQLITE_NOMEM;
        nRead = اقرأ(pCsr->fd, aBuf, sBuf.st_منوال);
        إذا( nRead!=sBuf.st_منوال ){
          أرجع SQLITE_IOERR;
        }
        sqlite3_نتیجہ_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
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

  nByte = حجم(fs_vtab) + (صحيح)لڑی_لمبائی(zTbl) + 1 + (صحيح)لڑی_لمبائی(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  إذا( !pVtab ) أرجع SQLITE_NOMEM;

  pVtab->zTbl = (حرف *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[لڑی_لمبائی(zTbl)+1];
  pVtab->db = db;
  حافظہ_نقل(pVtab->zTbl, zTbl, لڑی_لمبائی(zTbl));
  حافظہ_نقل(pVtab->zDb, zDb, لڑی_لمبائی(zDb));
  *ppVtab = &pVtab->ادھار;
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
  *ppCursor = &pCur->ادھار;
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

ساكن صحيح fs_اگلا(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  صحيح rc;

  rc = sqlite3_قدم(pCur->pStmt);
  إذا( rc==SQLITE_صف || rc==SQLITE_پورا ) rc = SQLITE_OK;

  أرجع rc;
}

ساكن صحيح fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  صحيح idx_عدد, ثابت حرف *idxStr,
  صحيح argc, sqlite3_قدر **argv
){
  صحيح rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  أكد( (idx_عدد==0 && argc==0) || (idx_عدد==1 && argc==1) );
  إذا( idx_عدد==1 ){
    حرف *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    إذا( !zStmt ) أرجع SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    إذا( rc==SQLITE_OK ){
      sqlite3_bind_قدر(pCur->pStmt, 1, argv[0]);
    }
  }وإلا{
    حرف *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    إذا( !zStmt ) أرجع SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  إذا( rc==SQLITE_OK ){
    rc = fs_اگلا(pVtabCursor); 
  }
  أرجع rc;
}

ساكن صحيح fs_کالم(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, صحيح i){
  fs_cursor *pCur = (fs_cursor*)cur;

  أكد( i==0 || i==1 || i==2 );
  إذا( i==0 ){
    sqlite3_نتیجہ_قدر(ctx, sqlite3_کالم_قدر(pCur->pStmt, 0));
  }وإلا{
    ثابت حرف *zFile = (ثابت حرف *)sqlite3_کالم_متن(pCur->pStmt, 1);
    بنية stat sbuf;
    صحيح fd;

    صحيح n;
    fd = open(zFile, O_RDONLY);
    إذا( fd<0 ) أرجع SQLITE_IOERR;
    fstat(fd, &sbuf);

    إذا( sbuf.st_ناپ>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_ناپ*2;
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

    n = (صحيح)اقرأ(fd, pCur->zBuf, sbuf.st_ناپ);
    close(fd);
    إذا( n!=sbuf.st_ناپ ) أرجع SQLITE_ERROR;
    pCur->nBuf = sbuf.st_ناپ;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_نتیجہ_متن(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  أرجع SQLITE_OK;
}

ساكن صحيح fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_کالم_int64(pCur->pStmt, 0);
  أرجع SQLITE_OK;
}

ساكن صحيح fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  أرجع (sqlite3_انکڑے_گنتی(pCur->pStmt)==0);
}

ساكن صحيح fs_بہترین_اشاریہ(sqlite3_vtab *tab, sqlite3_اشاریہ_info *pIdxInfo){
  صحيح ii;

  لكل(ii=0; ii<pIdxInfo->nConstraint; ii++){
    بنية sqlite3_اشاریہ_constraint ثابت *pCons = &pIdxInfo->aConstraint[ii];
    إذا( pCons->i_کالم<0 && pCons->usable
           && pCons->op==SQLITE_اشاریہ_CONSTRAINT_EQ ){
      بنية sqlite3_اشاریہ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argv_اشاریہ = 1;
      pIdxInfo->idx_عدد = 1;
      pIdxInfo->estimated_لاگت = 1.0;
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
  fs_بہترین_اشاریہ,
  fsDisconnect, 
  fsDisconnect,
  fsOpen,                      /* xOpen - open a cursor */
  fsClose,                     /* xClose - close a cursor */
  fsFilter,                    /* xFilter - configure scan constraints */
  fs_اگلا,                      /* xNext - advance a cursor */
  fsEof,                       /* xEof - check for end of scan */
  fs_کالم,                    /* xColumn - read data */
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
  fsdir_بہترین_اشاریہ,                 /* xBestIndex */
  fsdirDisconnect,                /* xDisconnect */
  fsdirDisconnect,                /* xDestroy */
  fsdirOpen,                      /* xOpen - open a cursor */
  fsdirClose,                     /* xClose - close a cursor */
  fsdirFilter,                    /* xFilter - configure scan constraints */
  fsdir_اگلا,                      /* xNext - advance a cursor */
  fsdirEof,                       /* xEof - check for end of scan */
  fsdir_کالم,                    /* xColumn - read data */
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
  fstree_بہترین_اشاریہ,                /* xBestIndex */
  fstreeDisconnect,               /* xDisconnect */
  fstreeDisconnect,               /* xDestroy */
  fstreeOpen,                     /* xOpen - open a cursor */
  fstreeClose,                    /* xClose - close a cursor */
  fstreeFilter,                   /* xFilter - configure scan constraints */
  fstree_اگلا,                     /* xNext - advance a cursor */
  fstreeEof,                      /* xEof - check for end of scan */
  fstree_کالم,                   /* xColumn - read data */
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
خارجي صحيح get_Db_اشارہ_گر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
ساكن صحيح SQLITE_TCLAPI رجسٹر_fs_module(
  Client_انکڑے client_انکڑے, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  صحيح objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلاں(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (فراغ *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#جے_انت
  أرجع TCL_OK;
}

#جے_انت


/*
** Register commands with the TCL interpreter.
*/
صحيح Sqlitetestfs_Init(Tcl_Interp *interp){
#جے_بنا_تعریف SQLITE_OMIT_VIRTUALTABLE
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     فراغ *client_انکڑے;
  } aObjCmd[] = {
     { "register_fs_module",   رجسٹر_fs_module, 0 },
  };
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_انکڑے, 0);
  }
#جے_انت
  أرجع TCL_OK;
}
