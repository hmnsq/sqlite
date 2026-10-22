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
#समावेश "sqliteInt.h"
#समावेश "tclsqlite.h"
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <sys/types.h>
#समावेश <sys/stat.h>
#समावेश <fcntl.h>

#अगर !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#अन्यथा
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
# endif
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE

प्रकार काष्ठा fs_vtab fs_vtab;
प्रकार काष्ठा fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
काष्ठा fs_vtab {
  sqlite3_vtab base;
  sqlite3 *db;
  अक्षर *zDb;                      /* Name of db containing zTbl */
  अक्षर *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
काष्ठा fs_cursor {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;
  अक्षर *zBuf;
  पूर्णांक nBuf;
  पूर्णांक nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
प्रकार काष्ठा FsdirVtab FsdirVtab;
प्रकार काष्ठा FsdirCsr FsdirCsr;
काष्ठा FsdirVtab {
  sqlite3_vtab base;
};

काष्ठा FsdirCsr {
  sqlite3_vtab_cursor base;
  अक्षर *zDir;                     /* Buffer containing directory scanned */
  सूची *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  काष्ठा dirent *pEntry;
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
अटल पूर्णांक fsdirConnect(
  sqlite3 *db,
  व्योम *pAux,
  पूर्णांक argc, स्थिर अक्षर *स्थिर*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  FsdirVtab *pTab;

  अगर( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    वापस SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(माप(FsdirVtab));
  अगर( !pTab ) वापस SQLITE_NOMEM;
  स_रखो(pTab, 0, माप(FsdirVtab));

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  वापस SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
अटल पूर्णांक fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  वापस SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
अटल पूर्णांक fsdir_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
  पूर्णांक ii;

  pIdxInfo->estimatedCost = 1000000000.0;

  क्रम(ii=0; ii<pIdxInfo->nConstraint; ii++){
    काष्ठा sqlite3_सूचकांक_constraint स्थिर *p = &pIdxInfo->aConstraint[ii];
    अगर( p->iColumn==0 && p->usable && p->op==SQLITE_सूचकांक_CONSTRAINT_EQ ){
      काष्ठा sqlite3_सूचकांक_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argv_सूचकांक = 1;
      pIdxInfo->idx_अंक = 1;
      pIdxInfo->estimatedCost = 1.0;
      अवरोध;
    }
  }

  वापस SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
अटल पूर्णांक fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(माप(FsdirCsr)+256);
  अगर( pCur==0 ) वापस SQLITE_NOMEM;
  स_रखो(pCur, 0, माप(FsdirCsr));
  *ppCursor = &pCur->base;
  वापस SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
अटल पूर्णांक fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  अगर( pCur->pDir ) बंद_सूची(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  वापस SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
अटल पूर्णांक fsdir_अग्र(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  अगर( pCsr->pDir ){
    pCsr->pEntry = सूची_पढ़ो(pCsr->pDir);
    अगर( pCsr->pEntry==0 ){
      बंद_सूची(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  वापस SQLITE_OK;
}

/*
** xFilter method implementation.
*/
अटल पूर्णांक fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णांक idx_अंक, स्थिर अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  स्थिर अक्षर *zDir;
  पूर्णांक nDir;


  अगर( idx_अंक!=1 || argc!=1 ){
    वापस SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  अगर( pCsr->pDir ){
    बंद_सूची(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
  nDir = sqlite3_मूल्य_bytes(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  अगर( pCsr->zDir==0 ) वापस SQLITE_NOMEM;
  स_नकल(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = सूची_खोलो(pCsr->zDir);
  वापस fsdir_अग्र(pVtabCursor); 
}

/*
** xEof method implementation.
*/
अटल पूर्णांक fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  वापस pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
अटल पूर्णांक fsdirColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णांक i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  चयन( i ){
    हाल 0: /* dir */
      sqlite3_परिणाम_text(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      अवरोध;

    हाल 1: /* name */
      sqlite3_परिणाम_text(ctx, pCsr->pEntry->d_name, -1, SQLITE_TRANSIENT);
      अवरोध;

    शेष:
      निश्चित( 0 );
  }

  वापस SQLITE_OK;
}

/*
** xRowid method implementation.
*/
अटल पूर्णांक fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  वापस SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
प्रकार काष्ठा FstreeVtab FstreeVtab;
प्रकार काष्ठा FstreeCsr FstreeCsr;
काष्ठा FstreeVtab {
  sqlite3_vtab base;
  sqlite3 *db;
};

काष्ठा FstreeCsr {
  sqlite3_vtab_cursor base;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  पूर्णांक fd;                         /* File descriptor open on current path */
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
अटल पूर्णांक fstreeConnect(
  sqlite3 *db,
  व्योम *pAux,
  पूर्णांक argc, स्थिर अक्षर *स्थिर*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  FstreeVtab *pTab;

  अगर( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    वापस SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(माप(FstreeVtab));
  अगर( !pTab ) वापस SQLITE_NOMEM;
  स_रखो(pTab, 0, माप(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  वापस SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
अटल पूर्णांक fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  वापस SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
अटल पूर्णांक fstree_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
  पूर्णांक ii;

  क्रम(ii=0; ii<pIdxInfo->nConstraint; ii++){
    काष्ठा sqlite3_सूचकांक_constraint स्थिर *p = &pIdxInfo->aConstraint[ii];
    अगर( p->iColumn==0 && p->usable && (
          p->op==SQLITE_सूचकांक_CONSTRAINT_GLOB
       || p->op==SQLITE_सूचकांक_CONSTRAINT_LIKE
       || p->op==SQLITE_सूचकांक_CONSTRAINT_EQ
    )){
      काष्ठा sqlite3_सूचकांक_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_अंक = p->op;
      pUsage->argv_सूचकांक = 1;
      pIdxInfo->estimatedCost = 100000.0;
      वापस SQLITE_OK;
    }
  }

  pIdxInfo->estimatedCost = 1000000000.0;
  वापस SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
अटल पूर्णांक fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(माप(FstreeCsr));
  अगर( pCur==0 ) वापस SQLITE_NOMEM;
  स_रखो(pCur, 0, माप(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->base;
  वापस SQLITE_OK;
}

अटल व्योम fstreeCloseFd(FstreeCsr *pCsr){
  अगर( pCsr->fd>=0 ){
    बंद(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
अटल पूर्णांक fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  वापस SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
अटल पूर्णांक fstree_अग्र(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  पूर्णांक rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_step(pCsr->pStmt);
  अगर( rc!=SQLITE_ROW ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }अन्यथा{
    rc = SQLITE_OK;
    pCsr->fd = खोलो((स्थिर अक्षर*)sqlite3_column_text(pCsr->pStmt, 0), O_RDONLY);
  }

  वापस rc;
}

/*
** xFilter method implementation.
*/
अटल पूर्णांक fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णांक idx_अंक, स्थिर अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->base.pVtab);
  पूर्णांक rc;
  स्थिर अक्षर *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  अक्षर *z_मूल;
  पूर्णांक n_मूल;
  अक्षर *zPrefix;
  पूर्णांक nPrefix;
  स्थिर अक्षर *zDir;
  पूर्णांक nDir;
  अक्षर aWild[2] = { '\0', '\0' };

#अगर_घोषित _WIN32
  स्थिर अक्षर *zDrive = दो_पर्या("fstreeDrive");
  अगर( zDrive==0 ){
    zDrive = दो_पर्या("SystemDrive");
  }
  z_मूल = sqlite3_mprintf("%s%c", zDrive, '/');
  n_मूल = sqlite3Strlen30(z_मूल);
  zPrefix = sqlite3_mprintf("%s", zDrive);
  nPrefix = sqlite3Strlen30(zPrefix);
#अन्यथा
  z_मूल = "/";
  n_मूल = 1;
  zPrefix = "";
  nPrefix = 0;
#पूर्ण_अगर

  zDir = z_मूल;
  nDir = n_मूल;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  अगर( rc!=SQLITE_OK ) वापस rc;

  अगर( idx_अंक ){
    स्थिर अक्षर *zQuery = (स्थिर अक्षर*)sqlite3_मूल्य_text(argv[0]);
    चयन( idx_अंक ){
      हाल SQLITE_सूचकांक_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        अवरोध;
      हाल SQLITE_सूचकांक_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        अवरोध;
    }

    अगर( sqlite3_strnicmp(zQuery, zPrefix, nPrefix)==0 ){
      पूर्णांक i;
      क्रम(i=nPrefix; zQuery[i]; i++){
        अगर( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) अवरोध;
        अगर( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  अगर( nDir==0 ) nDir = 1;

  sqlite3_bind_text(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 2, z_मूल, n_मूल, SQLITE_TRANSIENT);
  sqlite3_bind_text(pCsr->pStmt, 3, zPrefix, nPrefix, SQLITE_TRANSIENT);

#अगर_घोषित _WIN32
  sqlite3_free(zPrefix);
  sqlite3_free(z_मूल);
#पूर्ण_अगर

  वापस fstree_अग्र(pVtabCursor); 
}

/*
** xEof method implementation.
*/
अटल पूर्णांक fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  वापस pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
अटल पूर्णांक fstreeColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णांक i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  अगर( i==0 ){      /* path */
    sqlite3_परिणाम_मूल्य(ctx, sqlite3_column_मूल्य(pCsr->pStmt, 0));
  }अन्यथा{
    काष्ठा stat sBuf;
    ख_स्थिति(pCsr->fd, &sBuf);

    अगर( S_ISREG(sBuf.st_mode) ){
      अगर( i==1 ){
        sqlite3_परिणाम_int64(ctx, sBuf.st_आकार);
      }अन्यथा{
        पूर्णांक nRead;
        अक्षर *aBuf = sqlite3_malloc(sBuf.st_mode+1);
        अगर( !aBuf ) वापस SQLITE_NOMEM;
        nRead = पढ़ो(pCsr->fd, aBuf, sBuf.st_mode);
        अगर( nRead!=sBuf.st_mode ){
          वापस SQLITE_IOERR;
        }
        sqlite3_परिणाम_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  वापस SQLITE_OK;
}

/*
** xRowid method implementation.
*/
अटल पूर्णांक fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  वापस SQLITE_OK;
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
अटल पूर्णांक fsConnect(
  sqlite3 *db,
  व्योम *pAux,
  पूर्णांक argc, स्थिर अक्षर *स्थिर*argv,
  sqlite3_vtab **ppVtab,
  अक्षर **pzErr
){
  fs_vtab *pVtab;
  पूर्णांक nByte;
  स्थिर अक्षर *zTbl;
  स्थिर अक्षर *zDb = argv[1];

  अगर( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    वापस SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = माप(fs_vtab) + (पूर्णांक)म_माप(zTbl) + 1 + (पूर्णांक)म_माप(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  अगर( !pVtab ) वापस SQLITE_NOMEM;

  pVtab->zTbl = (अक्षर *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[म_माप(zTbl)+1];
  pVtab->db = db;
  स_नकल(pVtab->zTbl, zTbl, म_माप(zTbl));
  स_नकल(pVtab->zDb, zDb, म_माप(zDb));
  *ppVtab = &pVtab->base;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  वापस SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

अटल पूर्णांक fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  वापस SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
अटल पूर्णांक fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(माप(fs_cursor));
  *ppCursor = &pCur->base;
  वापस SQLITE_OK;
}

/*
** Close a fs cursor.
*/
अटल पूर्णांक fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  वापस SQLITE_OK;
}

अटल पूर्णांक fs_अग्र(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  पूर्णांक rc;

  rc = sqlite3_step(pCur->pStmt);
  अगर( rc==SQLITE_ROW || rc==SQLITE_DONE ) rc = SQLITE_OK;

  वापस rc;
}

अटल पूर्णांक fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  पूर्णांक idx_अंक, स्थिर अक्षर *idxStr,
  पूर्णांक argc, sqlite3_मूल्य **argv
){
  पूर्णांक rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  निश्चित( (idx_अंक==0 && argc==0) || (idx_अंक==1 && argc==1) );
  अगर( idx_अंक==1 ){
    अक्षर *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    अगर( !zStmt ) वापस SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    अगर( rc==SQLITE_OK ){
      sqlite3_bind_मूल्य(pCur->pStmt, 1, argv[0]);
    }
  }अन्यथा{
    अक्षर *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    अगर( !zStmt ) वापस SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  अगर( rc==SQLITE_OK ){
    rc = fs_अग्र(pVtabCursor); 
  }
  वापस rc;
}

अटल पूर्णांक fsColumn(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, पूर्णांक i){
  fs_cursor *pCur = (fs_cursor*)cur;

  निश्चित( i==0 || i==1 || i==2 );
  अगर( i==0 ){
    sqlite3_परिणाम_मूल्य(ctx, sqlite3_column_मूल्य(pCur->pStmt, 0));
  }अन्यथा{
    स्थिर अक्षर *zFile = (स्थिर अक्षर *)sqlite3_column_text(pCur->pStmt, 1);
    काष्ठा stat sbuf;
    पूर्णांक fd;

    पूर्णांक n;
    fd = खोलो(zFile, O_RDONLY);
    अगर( fd<0 ) वापस SQLITE_IOERR;
    ख_स्थिति(fd, &sbuf);

    अगर( sbuf.st_आकार>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_आकार*2;
      अक्षर *zNew;
      अगर( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      अगर( zNew==0 ){
        बंद(fd);
        वापस SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (पूर्णांक)पढ़ो(fd, pCur->zBuf, sbuf.st_आकार);
    बंद(fd);
    अगर( n!=sbuf.st_आकार ) वापस SQLITE_ERROR;
    pCur->nBuf = sbuf.st_आकार;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_परिणाम_text(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  वापस SQLITE_OK;
}

अटल पूर्णांक fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_column_int64(pCur->pStmt, 0);
  वापस SQLITE_OK;
}

अटल पूर्णांक fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  वापस (sqlite3_तथ्य_गिनती(pCur->pStmt)==0);
}

अटल पूर्णांक fs_Best_सूचकांक(sqlite3_vtab *tab, sqlite3_सूचकांक_info *pIdxInfo){
  पूर्णांक ii;

  क्रम(ii=0; ii<pIdxInfo->nConstraint; ii++){
    काष्ठा sqlite3_सूचकांक_constraint स्थिर *pCons = &pIdxInfo->aConstraint[ii];
    अगर( pCons->iColumn<0 && pCons->usable
           && pCons->op==SQLITE_सूचकांक_CONSTRAINT_EQ ){
      काष्ठा sqlite3_सूचकांक_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argv_सूचकांक = 1;
      pIdxInfo->idx_अंक = 1;
      pIdxInfo->estimatedCost = 1.0;
      अवरोध;
    }
  }

  वापस SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
अटल sqlite3_module fsModule = {
  0,                         /* iVersion */
  fsConnect,
  fsConnect,
  fs_Best_सूचकांक,
  fsDisconnect, 
  fsDisconnect,
  fsOpen,                      /* xOpen - open a cursor */
  fsClose,                     /* xClose - close a cursor */
  fsFilter,                    /* xFilter - configure scan constraints */
  fs_अग्र,                      /* xNext - advance a cursor */
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

अटल sqlite3_module fsdirModule = {
  0,                              /* iVersion */
  fsdirConnect,                   /* xCreate */
  fsdirConnect,                   /* xConnect */
  fsdir_Best_सूचकांक,                 /* xBestIndex */
  fsdirDisconnect,                /* xDisconnect */
  fsdirDisconnect,                /* xDestroy */
  fsdirOpen,                      /* xOpen - open a cursor */
  fsdirClose,                     /* xClose - close a cursor */
  fsdirFilter,                    /* xFilter - configure scan constraints */
  fsdir_अग्र,                      /* xNext - advance a cursor */
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

अटल sqlite3_module fstreeModule = {
  0,                              /* iVersion */
  fstreeConnect,                  /* xCreate */
  fstreeConnect,                  /* xConnect */
  fstree_Best_सूचकांक,                /* xBestIndex */
  fstreeDisconnect,               /* xDisconnect */
  fstreeDisconnect,               /* xDestroy */
  fstreeOpen,                     /* xOpen - open a cursor */
  fstreeClose,                    /* xClose - close a cursor */
  fstreeFilter,                   /* xFilter - configure scan constraints */
  fstree_अग्र,                     /* xNext - advance a cursor */
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
बाह्य पूर्णांक getDbPointer(Tcl_Interp *interp, स्थिर अक्षर *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
अटल पूर्णांक SQLITE_TCLAPI register_fs_module(
  Client_तथ्य client_तथ्य, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  पूर्णांक objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (व्योम *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#पूर्ण_अगर
  वापस TCL_OK;
}

#पूर्ण_अगर


/*
** Register commands with the TCL interpreter.
*/
पूर्णांक Sqlitetestfs_Init(Tcl_Interp *interp){
#अगर_अघोषित SQLITE_OMIT_VIRTUALTABLE
  अटल काष्ठा {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
     व्योम *client_तथ्य;
  } aObjCmd[] = {
     { "register_fs_module",   register_fs_module, 0 },
  };
  पूर्णांक i;
  क्रम(i=0; i<माप(aObjCmd)/माप(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_तथ्य, 0);
  }
#पूर्ण_अगर
  वापस TCL_OK;
}
