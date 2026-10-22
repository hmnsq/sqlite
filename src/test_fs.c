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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <fcntl.h>

#ఒకవేళ !defined(_WIN32) || defined(__MSVCRT__)
# include <unistd.h>
# include <dirent.h>
#లేకపోతే
# include "windirent.h"
# ifndef S_ISREG
#  define S_ISREG(బహుళకం) (((బహుళకం) & S_IFMT) == S_IFREG)
# endif
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE

రకనిర్వచనం నిర్మాణం fs_vtab fs_vtab;
రకనిర్వచనం నిర్మాణం fs_cursor fs_cursor;

/* 
** A fs virtual-table object 
*/
నిర్మాణం fs_vtab {
  sqlite3_vtab పునాది;
  sqlite3 *db;
  అక్షరం *zDb;                      /* Name of db containing zTbl */
  అక్షరం *zTbl;                     /* Name of docid->file map table */
};

/* A fs cursor object */
నిర్మాణం fs_cursor {
  sqlite3_vtab_cursor పునాది;
  sqlite3_stmt *pStmt;
  అక్షరం *zBuf;
  పూర్ణాంకం nBuf;
  పూర్ణాంకం nAlloc;
};

/*************************************************************************
** Start of fsdir implementation.
*/
రకనిర్వచనం నిర్మాణం FsdirVtab FsdirVtab;
రకనిర్వచనం నిర్మాణం FsdirCsr FsdirCsr;
నిర్మాణం FsdirVtab {
  sqlite3_vtab పునాది;
};

నిర్మాణం FsdirCsr {
  sqlite3_vtab_cursor పునాది;
  అక్షరం *zDir;                     /* Buffer containing directory scanned */
  DIR *pDir;                      /* Open directory */
  sqlite3_int64 iRowid;
  నిర్మాణం dirent *p_నమోదు;
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
స్థిర పూర్ణాంకం fsdirConnect(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  FsdirVtab *pTab;

  ఒకవేళ( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    తిరిగి SQLITE_ERROR;
  }

  pTab = (FsdirVtab *)sqlite3_malloc(పరిమాణం(FsdirVtab));
  ఒకవేళ( !pTab ) తిరిగి SQLITE_NOMEM;
  memset(pTab, 0, పరిమాణం(FsdirVtab));

  *ppVtab = &pTab->పునాది;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(dir, name);");

  తిరిగి SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
స్థిర పూర్ణాంకం fsdirDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  తిరిగి SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
స్థిర పూర్ణాంకం fsdir_ఉత్తమ_సూచీ(sqlite3_vtab *tab, sqlite3_సూచీ_info *pIdxInfo){
  పూర్ణాంకం ii;

  pIdxInfo->estimated_ఖర్చు = 1000000000.0;

  కొరకు(ii=0; ii<pIdxInfo->nConstraint; ii++){
    నిర్మాణం sqlite3_సూచీ_constraint స్థిరరాశి *p = &pIdxInfo->aConstraint[ii];
    ఒకవేళ( p->i_స్తంభం==0 && p->usable && p->op==SQLITE_సూచీ_CONSTRAINT_EQ ){
      నిర్మాణం sqlite3_సూచీ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 1;
      pUsage->argv_సూచీ = 1;
      pIdxInfo->idx_అంకె = 1;
      pIdxInfo->estimated_ఖర్చు = 1.0;
      ఆపు;
    }
  }

  తిరిగి SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fsdir cursor.
*/
స్థిర పూర్ణాంకం fsdirOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FsdirCsr *pCur;
  /* Allocate an extra 256 bytes because it is undefined how big dirent.d_name
  ** is and we need enough space.  Linux provides plenty already, but
  ** Solaris only provides one byte. */
  pCur = (FsdirCsr*)sqlite3_malloc(పరిమాణం(FsdirCsr)+256);
  ఒకవేళ( pCur==0 ) తిరిగి SQLITE_NOMEM;
  memset(pCur, 0, పరిమాణం(FsdirCsr));
  *ppCursor = &pCur->పునాది;
  తిరిగి SQLITE_OK;
}

/*
** Close a fsdir cursor.
*/
స్థిర పూర్ణాంకం fsdirClose(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCur = (FsdirCsr*)cur;
  ఒకవేళ( pCur->pDir ) closedir(pCur->pDir);
  sqlite3_free(pCur->zDir);
  sqlite3_free(pCur);
  తిరిగి SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
స్థిర పూర్ణాంకం fsdir_తదుపరి(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;

  ఒకవేళ( pCsr->pDir ){
    pCsr->p_నమోదు = readdir(pCsr->pDir);
    ఒకవేళ( pCsr->p_నమోదు==0 ){
      closedir(pCsr->pDir);
      pCsr->pDir = 0;
    }
    pCsr->iRowid++;
  }

  తిరిగి SQLITE_OK;
}

/*
** xFilter method implementation.
*/
స్థిర పూర్ణాంకం fsdirFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  FsdirCsr *pCsr = (FsdirCsr*)pVtabCursor;
  స్థిరరాశి అక్షరం *zDir;
  పూర్ణాంకం nDir;


  ఒకవేళ( idx_అంకె!=1 || argc!=1 ){
    తిరిగి SQLITE_ERROR;
  }

  pCsr->iRowid = 0;
  sqlite3_free(pCsr->zDir);
  ఒకవేళ( pCsr->pDir ){
    closedir(pCsr->pDir);
    pCsr->pDir = 0;
  }

  zDir = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
  nDir = sqlite3_విలువ_బైట్లు(argv[0]);
  pCsr->zDir = sqlite3_malloc(nDir+1);
  ఒకవేళ( pCsr->zDir==0 ) తిరిగి SQLITE_NOMEM;
  జ్ఞాపకం_నకలు(pCsr->zDir, zDir, nDir+1);

  pCsr->pDir = opendir(pCsr->zDir);
  తిరిగి fsdir_తదుపరి(pVtabCursor); 
}

/*
** xEof method implementation.
*/
స్థిర పూర్ణాంకం fsdirEof(sqlite3_vtab_cursor *cur){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  తిరిగి pCsr->pDir==0;
}

/*
** xColumn method implementation.
*/
స్థిర పూర్ణాంకం fsdir_స్తంభం(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, పూర్ణాంకం i){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  ఎంపిక( i ){
    స్థితి 0: /* dir */
      sqlite3_ఫలితం_పాఠ్యం(ctx, pCsr->zDir, -1, SQLITE_STATIC);
      ఆపు;

    స్థితి 1: /* name */
      sqlite3_ఫలితం_పాఠ్యం(ctx, pCsr->p_నమోదు->d_name, -1, SQLITE_TRANSIENT);
      ఆపు;

    అప్రమేయ:
      ప్రతిజ్ఞా( 0 );
  }

  తిరిగి SQLITE_OK;
}

/*
** xRowid method implementation.
*/
స్థిర పూర్ణాంకం fsdirRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  FsdirCsr *pCsr = (FsdirCsr*)cur;
  *pRowid = pCsr->iRowid;
  తిరిగి SQLITE_OK;
}
/*
** End of fsdir implementation.
*************************************************************************/

/*************************************************************************
** Start of fstree implementation.
*/
రకనిర్వచనం నిర్మాణం FstreeVtab FstreeVtab;
రకనిర్వచనం నిర్మాణం FstreeCsr FstreeCsr;
నిర్మాణం FstreeVtab {
  sqlite3_vtab పునాది;
  sqlite3 *db;
};

నిర్మాణం FstreeCsr {
  sqlite3_vtab_cursor పునాది;
  sqlite3_stmt *pStmt;            /* Statement to list paths */
  పూర్ణాంకం fd;                         /* File descriptor open on current path */
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
స్థిర పూర్ణాంకం fstreeConnect(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  FstreeVtab *pTab;

  ఒకవేళ( argc!=3 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    తిరిగి SQLITE_ERROR;
  }

  pTab = (FstreeVtab *)sqlite3_malloc(పరిమాణం(FstreeVtab));
  ఒకవేళ( !pTab ) తిరిగి SQLITE_NOMEM;
  memset(pTab, 0, పరిమాణం(FstreeVtab));
  pTab->db = db;

  *ppVtab = &pTab->పునాది;
  sqlite3_declare_vtab(db, "CREATE TABLE xyz(path, size, data);");

  తిరిగి SQLITE_OK;
}

/*
** xDestroy/xDisconnect implementation.
*/
స్థిర పూర్ణాంకం fstreeDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  తిరిగి SQLITE_OK;
}

/*
** xBestIndex implementation. The only constraint supported is:
**
**   (dir = ?)
*/
స్థిర పూర్ణాంకం fstree_ఉత్తమ_సూచీ(sqlite3_vtab *tab, sqlite3_సూచీ_info *pIdxInfo){
  పూర్ణాంకం ii;

  కొరకు(ii=0; ii<pIdxInfo->nConstraint; ii++){
    నిర్మాణం sqlite3_సూచీ_constraint స్థిరరాశి *p = &pIdxInfo->aConstraint[ii];
    ఒకవేళ( p->i_స్తంభం==0 && p->usable && (
          p->op==SQLITE_సూచీ_CONSTRAINT_GLOB
       || p->op==SQLITE_సూచీ_CONSTRAINT_LIKE
       || p->op==SQLITE_సూచీ_CONSTRAINT_EQ
    )){
      నిర్మాణం sqlite3_సూచీ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pIdxInfo->idx_అంకె = p->op;
      pUsage->argv_సూచీ = 1;
      pIdxInfo->estimated_ఖర్చు = 100000.0;
      తిరిగి SQLITE_OK;
    }
  }

  pIdxInfo->estimated_ఖర్చు = 1000000000.0;
  తిరిగి SQLITE_OK;
}

/*
** xOpen implementation.
**
** Open a new fstree cursor.
*/
స్థిర పూర్ణాంకం fstreeOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  FstreeCsr *pCur;
  pCur = (FstreeCsr*)sqlite3_malloc(పరిమాణం(FstreeCsr));
  ఒకవేళ( pCur==0 ) తిరిగి SQLITE_NOMEM;
  memset(pCur, 0, పరిమాణం(FstreeCsr));
  pCur->fd = -1;
  *ppCursor = &pCur->పునాది;
  తిరిగి SQLITE_OK;
}

స్థిర శూన్య fstreeCloseFd(FstreeCsr *pCsr){
  ఒకవేళ( pCsr->fd>=0 ){
    close(pCsr->fd);
    pCsr->fd = -1;
  }
}

/*
** Close a fstree cursor.
*/
స్థిర పూర్ణాంకం fstreeClose(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  sqlite3_finalize(pCsr->pStmt);
  fstreeCloseFd(pCsr);
  sqlite3_free(pCsr);
  తిరిగి SQLITE_OK;
}

/*
** Skip the cursor to the next entry.
*/
స్థిర పూర్ణాంకం fstree_తదుపరి(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  పూర్ణాంకం rc;

  fstreeCloseFd(pCsr);
  rc = sqlite3_అడుగు(pCsr->pStmt);
  ఒకవేళ( rc!=SQLITE_అడ్డువరుస ){
    rc = sqlite3_finalize(pCsr->pStmt);
    pCsr->pStmt = 0;
  }లేకపోతే{
    rc = SQLITE_OK;
    pCsr->fd = open((స్థిరరాశి అక్షరం*)sqlite3_స్తంభం_పాఠ్యం(pCsr->pStmt, 0), O_RDONLY);
  }

  తిరిగి rc;
}

/*
** xFilter method implementation.
*/
స్థిర పూర్ణాంకం fstreeFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  FstreeCsr *pCsr = (FstreeCsr*)pVtabCursor;
  FstreeVtab *pTab = (FstreeVtab*)(pCsr->పునాది.pVtab);
  పూర్ణాంకం rc;
  స్థిరరాశి అక్షరం *zSql = 
"WITH r(d) AS ("
"  SELECT CASE WHEN dir=?2 THEN ?3 ELSE dir END || '/' || name "
"    FROM fsdir WHERE dir=?1 AND name NOT LIKE '.%'"
"  UNION ALL"
"  SELECT dir || '/' || name FROM r, fsdir WHERE dir=d AND name NOT LIKE '.%'"
") SELECT d FROM r;";

  అక్షరం *z_మూలం;
  పూర్ణాంకం n_మూలం;
  అక్షరం *z_ఉపసర్గ;
  పూర్ణాంకం n_ఉపసర్గ;
  స్థిరరాశి అక్షరం *zDir;
  పూర్ణాంకం nDir;
  అక్షరం aWild[2] = { '\0', '\0' };

#నిర్వచితమైతే _WIN32
  స్థిరరాశి అక్షరం *zDrive = getenv("fstreeDrive");
  ఒకవేళ( zDrive==0 ){
    zDrive = getenv("SystemDrive");
  }
  z_మూలం = sqlite3_mprintf("%s%c", zDrive, '/');
  n_మూలం = sqlite3Strlen30(z_మూలం);
  z_ఉపసర్గ = sqlite3_mprintf("%s", zDrive);
  n_ఉపసర్గ = sqlite3Strlen30(z_ఉపసర్గ);
#లేకపోతే
  z_మూలం = "/";
  n_మూలం = 1;
  z_ఉపసర్గ = "";
  n_ఉపసర్గ = 0;
#అయితే_ముగింపు

  zDir = z_మూలం;
  nDir = n_మూలం;

  fstreeCloseFd(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
  ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;

  ఒకవేళ( idx_అంకె ){
    స్థిరరాశి అక్షరం *zQuery = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[0]);
    ఎంపిక( idx_అంకె ){
      స్థితి SQLITE_సూచీ_CONSTRAINT_GLOB:
        aWild[0] = '*';
        aWild[1] = '?';
        ఆపు;
      స్థితి SQLITE_సూచీ_CONSTRAINT_LIKE:
        aWild[0] = '_';
        aWild[1] = '%';
        ఆపు;
    }

    ఒకవేళ( sqlite3_strnicmp(zQuery, z_ఉపసర్గ, n_ఉపసర్గ)==0 ){
      పూర్ణాంకం i;
      కొరకు(i=n_ఉపసర్గ; zQuery[i]; i++){
        ఒకవేళ( zQuery[i]==aWild[0] || zQuery[i]==aWild[1] ) ఆపు;
        ఒకవేళ( zQuery[i]=='/' ) nDir = i;
      }
      zDir = zQuery;
    }
  }
  ఒకవేళ( nDir==0 ) nDir = 1;

  sqlite3_bind_పాఠ్యం(pCsr->pStmt, 1, zDir, nDir, SQLITE_TRANSIENT);
  sqlite3_bind_పాఠ్యం(pCsr->pStmt, 2, z_మూలం, n_మూలం, SQLITE_TRANSIENT);
  sqlite3_bind_పాఠ్యం(pCsr->pStmt, 3, z_ఉపసర్గ, n_ఉపసర్గ, SQLITE_TRANSIENT);

#నిర్వచితమైతే _WIN32
  sqlite3_free(z_ఉపసర్గ);
  sqlite3_free(z_మూలం);
#అయితే_ముగింపు

  తిరిగి fstree_తదుపరి(pVtabCursor); 
}

/*
** xEof method implementation.
*/
స్థిర పూర్ణాంకం fstreeEof(sqlite3_vtab_cursor *cur){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  తిరిగి pCsr->pStmt==0;
}

/*
** xColumn method implementation.
*/
స్థిర పూర్ణాంకం fstree_స్తంభం(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, పూర్ణాంకం i){
  FstreeCsr *pCsr = (FstreeCsr*)cur;
  ఒకవేళ( i==0 ){      /*మార్గం*/
    sqlite3_ఫలితం_విలువ(ctx, sqlite3_స్తంభం_విలువ(pCsr->pStmt, 0));
  }లేకపోతే{
    నిర్మాణం stat sBuf;
    fstat(pCsr->fd, &sBuf);

    ఒకవేళ( S_ISREG(sBuf.st_బహుళకం) ){
      ఒకవేళ( i==1 ){
        sqlite3_ఫలితం_int64(ctx, sBuf.st_కొలత);
      }లేకపోతే{
        పూర్ణాంకం nRead;
        అక్షరం *aBuf = sqlite3_malloc(sBuf.st_బహుళకం+1);
        ఒకవేళ( !aBuf ) తిరిగి SQLITE_NOMEM;
        nRead = పఠ(pCsr->fd, aBuf, sBuf.st_బహుళకం);
        ఒకవేళ( nRead!=sBuf.st_బహుళకం ){
          తిరిగి SQLITE_IOERR;
        }
        sqlite3_ఫలితం_blob(ctx, aBuf, nRead, SQLITE_TRANSIENT);
        sqlite3_free(aBuf);
      }
    }
  }

  తిరిగి SQLITE_OK;
}

/*
** xRowid method implementation.
*/
స్థిర పూర్ణాంకం fstreeRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  *pRowid = 0;
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం fsConnect(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  fs_vtab *pVtab;
  పూర్ణాంకం nByte;
  స్థిరరాశి అక్షరం *zTbl;
  స్థిరరాశి అక్షరం *zDb = argv[1];

  ఒకవేళ( argc!=4 ){
    *pzErr = sqlite3_mprintf("wrong number of arguments");
    తిరిగి SQLITE_ERROR;
  }
  zTbl = argv[3];

  nByte = పరిమాణం(fs_vtab) + (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zTbl) + 1 + (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zDb) + 1;
  pVtab = (fs_vtab *)sqlite3MallocZero( nByte );
  ఒకవేళ( !pVtab ) తిరిగి SQLITE_NOMEM;

  pVtab->zTbl = (అక్షరం *)&pVtab[1];
  pVtab->zDb = &pVtab->zTbl[అక్షరశ్రేణి_పొడవు(zTbl)+1];
  pVtab->db = db;
  జ్ఞాపకం_నకలు(pVtab->zTbl, zTbl, అక్షరశ్రేణి_పొడవు(zTbl));
  జ్ఞాపకం_నకలు(pVtab->zDb, zDb, అక్షరశ్రేణి_పొడవు(zDb));
  *ppVtab = &pVtab->పునాది;
  sqlite3_declare_vtab(db, "CREATE TABLE x(path TEXT, data TEXT)");

  తిరిగి SQLITE_OK;
}
/* Note that for this virtual table, the xCreate and xConnect
** methods are identical. */

స్థిర పూర్ణాంకం fsDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  తిరిగి SQLITE_OK;
}
/* The xDisconnect and xDestroy methods are also the same */

/*
** Open a new fs cursor.
*/
స్థిర పూర్ణాంకం fsOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  fs_cursor *pCur;
  pCur = sqlite3MallocZero(పరిమాణం(fs_cursor));
  *ppCursor = &pCur->పునాది;
  తిరిగి SQLITE_OK;
}

/*
** Close a fs cursor.
*/
స్థిర పూర్ణాంకం fsClose(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  sqlite3_finalize(pCur->pStmt);
  sqlite3_free(pCur->zBuf);
  sqlite3_free(pCur);
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం fs_తదుపరి(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor *)cur;
  పూర్ణాంకం rc;

  rc = sqlite3_అడుగు(pCur->pStmt);
  ఒకవేళ( rc==SQLITE_అడ్డువరుస || rc==SQLITE_పూర్తి ) rc = SQLITE_OK;

  తిరిగి rc;
}

స్థిర పూర్ణాంకం fsFilter(
  sqlite3_vtab_cursor *pVtabCursor, 
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  పూర్ణాంకం rc;
  fs_cursor *pCur = (fs_cursor *)pVtabCursor;
  fs_vtab *p = (fs_vtab *)(pVtabCursor->pVtab);

  ప్రతిజ్ఞా( (idx_అంకె==0 && argc==0) || (idx_అంకె==1 && argc==1) );
  ఒకవేళ( idx_అంకె==1 ){
    అక్షరం *zStmt = sqlite3_mprintf(
        "SELECT * FROM %Q.%Q WHERE rowid=?", p->zDb, p->zTbl);
    ఒకవేళ( !zStmt ) తిరిగి SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
    ఒకవేళ( rc==SQLITE_OK ){
      sqlite3_bind_విలువ(pCur->pStmt, 1, argv[0]);
    }
  }లేకపోతే{
    అక్షరం *zStmt = sqlite3_mprintf("SELECT * FROM %Q.%Q", p->zDb, p->zTbl);
    ఒకవేళ( !zStmt ) తిరిగి SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zStmt, -1, &pCur->pStmt, 0);
    sqlite3_free(zStmt);
  }

  ఒకవేళ( rc==SQLITE_OK ){
    rc = fs_తదుపరి(pVtabCursor); 
  }
  తిరిగి rc;
}

స్థిర పూర్ణాంకం fs_స్తంభం(sqlite3_vtab_cursor *cur, sqlite3_context *ctx, పూర్ణాంకం i){
  fs_cursor *pCur = (fs_cursor*)cur;

  ప్రతిజ్ఞా( i==0 || i==1 || i==2 );
  ఒకవేళ( i==0 ){
    sqlite3_ఫలితం_విలువ(ctx, sqlite3_స్తంభం_విలువ(pCur->pStmt, 0));
  }లేకపోతే{
    స్థిరరాశి అక్షరం *zFile = (స్థిరరాశి అక్షరం *)sqlite3_స్తంభం_పాఠ్యం(pCur->pStmt, 1);
    నిర్మాణం stat sbuf;
    పూర్ణాంకం fd;

    పూర్ణాంకం n;
    fd = open(zFile, O_RDONLY);
    ఒకవేళ( fd<0 ) తిరిగి SQLITE_IOERR;
    fstat(fd, &sbuf);

    ఒకవేళ( sbuf.st_కొలత>=pCur->nAlloc ){
      sqlite3_int64 nNew = sbuf.st_కొలత*2;
      అక్షరం *zNew;
      ఒకవేళ( nNew<1024 ) nNew = 1024;

      zNew = sqlite3Realloc(pCur->zBuf, nNew);
      ఒకవేళ( zNew==0 ){
        close(fd);
        తిరిగి SQLITE_NOMEM;
      }
      pCur->zBuf = zNew;
      pCur->nAlloc = nNew;
    }

    n = (పూర్ణాంకం)పఠ(fd, pCur->zBuf, sbuf.st_కొలత);
    close(fd);
    ఒకవేళ( n!=sbuf.st_కొలత ) తిరిగి SQLITE_ERROR;
    pCur->nBuf = sbuf.st_కొలత;
    pCur->zBuf[pCur->nBuf] = '\0';

    sqlite3_ఫలితం_పాఠ్యం(ctx, pCur->zBuf, -1, SQLITE_TRANSIENT);
  }
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం fsRowid(sqlite3_vtab_cursor *cur, sqlite_int64 *pRowid){
  fs_cursor *pCur = (fs_cursor*)cur;
  *pRowid = sqlite3_స్తంభం_int64(pCur->pStmt, 0);
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం fsEof(sqlite3_vtab_cursor *cur){
  fs_cursor *pCur = (fs_cursor*)cur;
  తిరిగి (sqlite3_దత్తాంశం_లెక్క(pCur->pStmt)==0);
}

స్థిర పూర్ణాంకం fs_ఉత్తమ_సూచీ(sqlite3_vtab *tab, sqlite3_సూచీ_info *pIdxInfo){
  పూర్ణాంకం ii;

  కొరకు(ii=0; ii<pIdxInfo->nConstraint; ii++){
    నిర్మాణం sqlite3_సూచీ_constraint స్థిరరాశి *pCons = &pIdxInfo->aConstraint[ii];
    ఒకవేళ( pCons->i_స్తంభం<0 && pCons->usable
           && pCons->op==SQLITE_సూచీ_CONSTRAINT_EQ ){
      నిర్మాణం sqlite3_సూచీ_constraint_usage *pUsage;
      pUsage = &pIdxInfo->aConstraintUsage[ii];
      pUsage->omit = 0;
      pUsage->argv_సూచీ = 1;
      pIdxInfo->idx_అంకె = 1;
      pIdxInfo->estimated_ఖర్చు = 1.0;
      ఆపు;
    }
  }

  తిరిగి SQLITE_OK;
}

/*
** A virtual table module that provides read-only access to a
** Tcl global variable namespace.
*/
స్థిర sqlite3_module fsModule = {
  0,                         /* iVersion */
  fsConnect,
  fsConnect,
  fs_ఉత్తమ_సూచీ,
  fsDisconnect, 
  fsDisconnect,
  fsOpen,                      /* xOpen - open a cursor */
  fsClose,                     /* xClose - close a cursor */
  fsFilter,                    /* xFilter - configure scan constraints */
  fs_తదుపరి,                      /* xNext - advance a cursor */
  fsEof,                       /* xEof - check for end of scan */
  fs_స్తంభం,                    /* xColumn - read data */
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

స్థిర sqlite3_module fsdirModule = {
  0,                              /* iVersion */
  fsdirConnect,                   /* xCreate */
  fsdirConnect,                   /* xConnect */
  fsdir_ఉత్తమ_సూచీ,                 /* xBestIndex */
  fsdirDisconnect,                /* xDisconnect */
  fsdirDisconnect,                /* xDestroy */
  fsdirOpen,                      /* xOpen - open a cursor */
  fsdirClose,                     /* xClose - close a cursor */
  fsdirFilter,                    /* xFilter - configure scan constraints */
  fsdir_తదుపరి,                      /* xNext - advance a cursor */
  fsdirEof,                       /* xEof - check for end of scan */
  fsdir_స్తంభం,                    /* xColumn - read data */
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

స్థిర sqlite3_module fstreeModule = {
  0,                              /* iVersion */
  fstreeConnect,                  /* xCreate */
  fstreeConnect,                  /* xConnect */
  fstree_ఉత్తమ_సూచీ,                /* xBestIndex */
  fstreeDisconnect,               /* xDisconnect */
  fstreeDisconnect,               /* xDestroy */
  fstreeOpen,                     /* xOpen - open a cursor */
  fstreeClose,                    /* xClose - close a cursor */
  fstreeFilter,                   /* xFilter - configure scan constraints */
  fstree_తదుపరి,                     /* xNext - advance a cursor */
  fstreeEof,                      /* xEof - check for end of scan */
  fstree_స్తంభం,                   /* xColumn - read data */
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
బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp *interp, స్థిరరాశి అక్షరం *zA, sqlite3 **ppDb);

/*
** Register the echo virtual table module.
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI రిజిస్టరు_fs_module(
  Client_దత్తాంశం client_దత్తాంశం, /* Pointer to sqlite3_enable_XXX function */
  Tcl_Interp *interp,    /* The TCL interpreter that invoked this command */
  పూర్ణాంకం objc,              /* Number of arguments */
  Tcl_Obj *CONST objv[]  /* Command arguments */
){
  sqlite3 *db;
  ఒకవేళ( objc!=2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) ) తిరిగి TCL_ERROR;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  sqlite3_create_module(db, "fs", &fsModule, (శూన్య *)interp);
  sqlite3_create_module(db, "fsdir", &fsdirModule, 0);
  sqlite3_create_module(db, "fstree", &fstreeModule, 0);
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}

#అయితే_ముగింపు


/*
** Register commands with the TCL interpreter.
*/
పూర్ణాంకం Sqlitetestfs_Init(Tcl_Interp *interp){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  స్థిర నిర్మాణం {
     అక్షరం *zName;
     Tcl_ObjCmdProc *xProc;
     శూన్య *client_దత్తాంశం;
  } aObjCmd[] = {
     { "register_fs_module",   రిజిస్టరు_fs_module, 0 },
  };
  పూర్ణాంకం i;
  కొరకు(i=0; i<పరిమాణం(aObjCmd)/పరిమాణం(aObjCmd[0]); i++){
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, 
        aObjCmd[i].xProc, aObjCmd[i].client_దత్తాంశం, 0);
  }
#అయితే_ముగింపు
  తిరిగి TCL_OK;
}
