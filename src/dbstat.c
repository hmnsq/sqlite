/*
** 2010 July 12
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains an implementation of the "dbstat" virtual table.
**
** The dbstat virtual table is used to extract low-level storage
** information from an SQLite database in order to implement the
** "sqlite3_analyzer" utility.  See the ../tool/spaceanal.tcl script
** for an example implementation.
**
** Additional information is available on the "dbstat.html" page of the
** official SQLite documentation.
*/

#అన్తర్భావ "sqliteInt.h"   /* Requires access to internal data structures */
#ఒకవేళ (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

/*
** The pager and btree modules arrange objects in memory so that there are
** always approximately 200 bytes of addressable memory following each page
** buffer. This way small buffer overreads caused by corrupt database pages
** do not cause undefined behaviour. This module pads each page buffer
** by the following number of bytes for the same purpose.
*/
#నిర్వచించు DBSTAT_పుట_PADDING_బైట్లు 256

/*
** Page paths:
** 
**   The value of the 'path' column describes the path taken from the 
**   root-node of the b-tree structure to each page. The value of the 
**   root-node path is '/'.
**
**   The value of the path for the left-most child page of the root of
**   a b-tree is '/000/'. (Btrees store content ordered from left to right
**   so the pages to the left have smaller keys than the pages to the right.)
**   The next to left-most child of the root page is
**   '/001', and so on, each sibling page identified by a 3-digit hex 
**   value. The children of the 451st left-most sibling have paths such
**   as '/1c2/000/, '/1c2/001/' etc.
**
**   Overflow pages are specified by appending a '+' character and a 
**   six-digit hexadecimal value to the path to the cell they are linked
**   from. For example, the three overflow pages in a chain linked from 
**   the left-most cell of the 450th child of the root page are identified
**   by the paths:
**
**      '/1c2/000+000000'         // First page in overflow chain
**      '/1c2/000+000001'         // Second page in overflow chain
**      '/1c2/000+000002'         // Third page in overflow chain
**
**   If the paths are sorted using the BINARY collation sequence, then
**   the overflow pages associated with a cell will appear earlier in the
**   sort-order than its child page:
**
**      '/1c2/000/'               // Left-most child of 451st child of root
*/
స్థిర స్థిరరాశి అక్షరం zDbstatSchema[] = 
  "CREATE TABLE x("
  " name       TEXT,"          /*  0 Name of table or index */
  " path       TEXT,"          /*  1 Path to page from root (NULL for agg) */
  " pageno     INTEGER,"       /*  2 Page number (page count for aggregates) */
  " pagetype   TEXT,"          /*  3 'internal', 'leaf', 'overflow', or NULL */
  " ncell      INTEGER,"       /*  4 Cells on page (0 for overflow) */
  " payload    INTEGER,"       /*  5 Bytes of payload on this page */
  " unused     INTEGER,"       /*  6 Bytes of unused space on this page */
  " mx_payload INTEGER,"       /*  7 Largest payload size of all cells */
  " pgoffset   INTEGER,"       /*  8 Offset of page in file (NULL for agg) */
  " pgsize     INTEGER,"       /*  9 Size of the page (sum for aggregate) */
  " schema     TEXT HIDDEN,"   /* 10 Database schema being analyzed */
  " aggregate  BOOLEAN HIDDEN" /* 11 aggregate info for each table */
  ")"
;

/* Forward reference to data structured used in this module */
రకనిర్వచనం నిర్మాణం Stat_పట్టిక Stat_పట్టిక;
రకనిర్వచనం నిర్మాణం StatCursor StatCursor;
రకనిర్వచనం నిర్మాణం Stat_పుట Stat_పుట;
రకనిర్వచనం నిర్మాణం StatCell StatCell;

/* Size information for a single cell within a btree page */
నిర్మాణం StatCell {
  పూర్ణాంకం nLocal;                     /* Bytes of local payload */
  u32 i_బిడ్డ_Pg;                   /* Child node (or 0 if this is a leaf) */
  పూర్ణాంకం nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  పూర్ణాంకం n_చివరి_Ovfl;                  /* Bytes of payload on final overflow page */
  పూర్ణాంకం iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
నిర్మాణం Stat_పుట {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  పూర్ణాంకం iCell;                      /* Current cell */
  అక్షరం *z_మార్గం;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  పూర్ణాంకం nCell;                      /* Number of cells on page */
  పూర్ణాంకం n_వాడని;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_కుడి_బిడ్డ_Pg;              /* Right-child page number (or 0) */
  పూర్ణాంకం nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
నిర్మాణం StatCursor {
  sqlite3_vtab_cursor పునాది;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  పూర్ణాంకం iDb;                        /* Schema used for this query */

  Stat_పుట a_పుట[32];             /* Pages in path to current page */
  పూర్ణాంకం i_పుట;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  అక్షరం *zName;                    /* Value of 'name' column */
  అక్షరం *z_మార్గం;                    /* Value of 'path' column */
  అక్షరం *zPagetype;                /* Value of 'pagetype' column */
  పూర్ణాంకం n_పుట;                      /* Number of pages in current btree */
  పూర్ణాంకం nCell;                      /* Value of 'ncell' column */
  పూర్ణాంకం nMxPayload;                 /* Value of 'mx_payload' column */
  i64 n_వాడని;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 i_స్థానభ్రంశం;                    /* Value of 'pgOffset' column */
  i64 sz_పుట;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
నిర్మాణం Stat_పట్టిక {
  sqlite3_vtab పునాది;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  పూర్ణాంకం iDb;                        /* Index of database to analyze */
};

#నిర్వచితం_కాకపోతే get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#అయితే_ముగింపు

/*
** Connect to or create a new DBSTAT virtual table.
*/
స్థిర పూర్ణాంకం statConnect(
  sqlite3 *db,
  శూన్య *pAux,
  పూర్ణాంకం argc, స్థిరరాశి అక్షరం *స్థిరరాశి*argv,
  sqlite3_vtab **ppVtab,
  అక్షరం **pzErr
){
  Stat_పట్టిక *pTab = 0;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం iDb;
  (శూన్య)pAux;

  ఒకవేళ( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (అక్షరం*)argv[3]);
    iDb = sqlite3_కనుగొను_Db(db, &nm);
    ఒకవేళ( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      తిరిగి SQLITE_ERROR;
    }
  }లేకపోతే{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  ఒకవేళ( rc==SQLITE_OK ){
    pTab = (Stat_పట్టిక *)sqlite3_malloc64(పరిమాణం(Stat_పట్టిక));
    ఒకవేళ( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  ప్రతిజ్ఞా( rc==SQLITE_OK || pTab==0 );
  ఒకవేళ( rc==SQLITE_OK ){
    memset(pTab, 0, పరిమాణం(Stat_పట్టిక));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  తిరిగి rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
స్థిర పూర్ణాంకం statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  తిరిగి SQLITE_OK;
}

/*
** Compute the best query strategy and return the result in idxNum.
**
**   idxNum-Bit        Meaning
**   ----------        ----------------------------------------------
**      0x01           There is a schema=? term in the WHERE clause
**      0x02           There is a name=? term in the WHERE clause
**      0x04           There is an aggregate=? term in the WHERE clause
**      0x08           Output should be ordered by name and path
*/
స్థిర పూర్ణాంకం stat_ఉత్తమ_సూచీ(sqlite3_vtab *tab, sqlite3_సూచీ_info *pIdxInfo){
  పూర్ణాంకం i;
  పూర్ణాంకం iSchema = -1;
  పూర్ణాంకం iName = -1;
  పూర్ణాంకం iAgg = -1;
  (శూన్య)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  కొరకు(i=0; i<pIdxInfo->nConstraint; i++){
    ఒకవేళ( pIdxInfo->aConstraint[i].op!=SQLITE_సూచీ_CONSTRAINT_EQ ) కొనసాగు;
    ఒకవేళ( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      తిరిగి SQLITE_CONSTRAINT;
    }
    ఎంపిక( pIdxInfo->aConstraint[i].i_స్తంభం ){
      స్థితి 0: {    /* name */
        iName = i;
        ఆపు;
      }
      స్థితి 10: {   /* schema */
        iSchema = i;
        ఆపు;
      }
      స్థితి 11: {   /* aggregate */
        iAgg = i;
        ఆపు;
      }
    }
  }
  i = 0;
  ఒకవేళ( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argv_సూచీ = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_అంకె |= 0x01;
  }
  ఒకవేళ( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argv_సూచీ = ++i;
    pIdxInfo->idx_అంకె |= 0x02;
  }
  ఒకవేళ( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argv_సూచీ = ++i;
    pIdxInfo->idx_అంకె |= 0x04;
  }
  pIdxInfo->estimated_ఖర్చు = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  ఒకవేళ( ( pIdxInfo->n_క్రమం_By==1
     && pIdxInfo->a_క్రమం_By[0].i_స్తంభం==0
     && pIdxInfo->a_క్రమం_By[0].desc==0
     ) ||
      ( pIdxInfo->n_క్రమం_By==2
     && pIdxInfo->a_క్రమం_By[0].i_స్తంభం==0
     && pIdxInfo->a_క్రమం_By[0].desc==0
     && pIdxInfo->a_క్రమం_By[1].i_స్తంభం==1
     && pIdxInfo->a_క్రమం_By[1].desc==0
     )
  ){
    pIdxInfo->క్రమం_By_Consumed = 1;
    pIdxInfo->idx_అంకె |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_సూచీ_పరిశీలన_HEX;

  తిరిగి SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
స్థిర పూర్ణాంకం statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  Stat_పట్టిక *pTab = (Stat_పట్టిక *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(పరిమాణం(StatCursor));
  ఒకవేళ( pCsr==0 ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }లేకపోతే{
    memset(pCsr, 0, పరిమాణం(StatCursor));
    pCsr->పునాది.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  తిరిగి SQLITE_OK;
}

స్థిర శూన్య statClearCells(Stat_పుట *p){
  పూర్ణాంకం i;
  ఒకవేళ( p->aCell ){
    కొరకు(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

స్థిర శూన్య stat_Clear_పుట(Stat_పుట *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->z_మార్గం);
  memset(p, 0, పరిమాణం(Stat_పుట));
  p->aPg = aPg;
}

స్థిర శూన్య statResetCsr(StatCursor *pCsr){
  పూర్ణాంకం i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  కొరకు(i=0; i<Array_కొలత(pCsr->a_పుట); i++){
    stat_Clear_పుట(&pCsr->a_పుట[i]);
    sqlite3_free(pCsr->a_పుట[i].aPg);
    pCsr->a_పుట[i].aPg = 0;
  }
  sqlite3_reset(pCsr->pStmt);
  pCsr->i_పుట = 0;
  sqlite3_free(pCsr->z_మార్గం);
  pCsr->z_మార్గం = 0;
  pCsr->isEof = 0;
}

/* Resize the space-used counters inside of the cursor */
స్థిర శూన్య statResetCounts(StatCursor *pCsr){
  pCsr->nCell = 0;
  pCsr->nMxPayload = 0;
  pCsr->n_వాడని = 0;
  pCsr->nPayload = 0;
  pCsr->sz_పుట = 0;
  pCsr->n_పుట = 0;
}

/*
** Close a DBSTAT cursor.
*/
స్థిర పూర్ణాంకం statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  తిరిగి SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
స్థిర పూర్ణాంకం getLocalPayload(
  పూర్ణాంకం nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  పూర్ణాంకం n_మొత్తం                      /* Total record (payload) size */
){
  పూర్ణాంకం nLocal;
  పూర్ణాంకం nMinLocal;
  పూర్ణాంకం nMaxLocal;
 
  ఒకవేళ( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }లేకపోతే{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (n_మొత్తం - nMinLocal) % (nUsable - 4);
  ఒకవేళ( nLocal>nMaxLocal ) nLocal = nMinLocal;
  తిరిగి nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
స్థిర పూర్ణాంకం stat_Decode_పుట(Btree *pBt, Stat_పుట *p){
  పూర్ణాంకం n_వాడని;
  పూర్ణాంకం iOff;
  పూర్ణాంకం nHdr;
  పూర్ణాంకం is_ఆకు;
  పూర్ణాంకం sz_పుట;

  u8 *a_దత్తాంశం = p->aPg;
  u8 *aHdr = &a_దత్తాంశం[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  ఒకవేళ( p->flags==0x0A || p->flags==0x0D ){
    is_ఆకు = 1;
    nHdr = 8;
  }లేకపోతే ఒకవేళ( p->flags==0x05 || p->flags==0x02 ){
    is_ఆకు = 0;
    nHdr = 12;
  }లేకపోతే{
    వెళ్ళు stat_పుట_Is_Corrupt;
  }
  ఒకవేళ( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  sz_పుట = sqlite3_Btree_Get_పుట_కొలత(pBt);

  n_వాడని = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  n_వాడని += (పూర్ణాంకం)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  అయితే( iOff ){
    పూర్ణాంకం i_తదుపరి;
    ఒకవేళ( iOff>=sz_పుట ) వెళ్ళు stat_పుట_Is_Corrupt;
    n_వాడని += get2byte(&a_దత్తాంశం[iOff+2]);
    i_తదుపరి = get2byte(&a_దత్తాంశం[iOff]);
    ఒకవేళ( i_తదుపరి<iOff+4 && i_తదుపరి>0 ) వెళ్ళు stat_పుట_Is_Corrupt;
    iOff = i_తదుపరి;
  }
  p->n_వాడని = n_వాడని;
  p->i_కుడి_బిడ్డ_Pg = is_ఆకు ? 0 : sqlite3Get4byte(&aHdr[8]);

  ఒకవేళ( p->nCell ){
    పూర్ణాంకం i;                        /* Used to iterate through cells */
    పూర్ణాంకం nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = sz_పుట - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * పరిమాణం(StatCell));
    ఒకవేళ( p->aCell==0 ) తిరిగి SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * పరిమాణం(StatCell));

    కొరకు(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&a_దత్తాంశం[nHdr+i*2]);
      ఒకవేళ( iOff<nHdr || iOff>=sz_పుట ) వెళ్ళు stat_పుట_Is_Corrupt;
      ఒకవేళ( !is_ఆకు ){
        pCell->i_బిడ్డ_Pg = sqlite3Get4byte(&a_దత్తాంశం[iOff]);
        iOff += 4;
      }
      ఒకవేళ( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }లేకపోతే{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        పూర్ణాంకం nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&a_దత్తాంశం[iOff], nPayload);
        ఒకవేళ( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&a_దత్తాంశం[iOff], &dummy);
        }
        ఒకవేళ( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        ఒకవేళ( nLocal<0 ) వెళ్ళు stat_పుట_Is_Corrupt;
        pCell->nLocal = nLocal;
        ప్రతిజ్ఞా( nPayload>=(u32)nLocal );
        ప్రతిజ్ఞా( nLocal<=(nUsable-35) );
        ఒకవేళ( nPayload>(u32)nLocal ){
          పూర్ణాంకం j;
          పూర్ణాంకం nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          ఒకవేళ( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            వెళ్ళు stat_పుట_Is_Corrupt;
          }
          pCell->n_చివరి_Ovfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(పరిమాణం(u32)*nOvfl);
          ఒకవేళ( pCell->aOvfl==0 ) తిరిగి SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&a_దత్తాంశం[iOff+nLocal]);
          కొరకు(j=1; j<nOvfl; j++){
            పూర్ణాంకం rc;
            u32 iPrev = pCell->aOvfl[j-1];
            Db_పుట *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            ఒకవేళ( rc!=SQLITE_OK ){
              ప్రతిజ్ఞా( pPg==0 );
              తిరిగి rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3_Pager_Get_దత్తాంశం(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  తిరిగి SQLITE_OK;

stat_పుట_Is_Corrupt:
  p->flags = 0;
  statClearCells(p);
  తిరిగి SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
స్థిర శూన్య stat_కొలత_And_స్థానభ్రంశం(StatCursor *pCsr){
  Stat_పట్టిక *pTab = (Stat_పట్టిక *)((sqlite3_vtab_cursor *)pCsr)->pVtab;
  Btree *pBt = pTab->db->aDb[pTab->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);
  sqlite3_file *fd;
  sqlite3_int64 x[2];

  /* If connected to a ZIPVFS backend, find the page size and
  ** offset from ZIPVFS.
  */
  fd = sqlite3PagerFile(pPager);
  x[0] = pCsr->iPageno;
  ఒకవేళ( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->i_స్థానభ్రంశం = x[0];
    pCsr->sz_పుట += x[1];
  }లేకపోతే{
    /* Not ZIPVFS: The default page size and offset */
    pCsr->sz_పుట += sqlite3_Btree_Get_పుట_కొలత(pBt);
    pCsr->i_స్థానభ్రంశం = (i64)pCsr->sz_పుట * (pCsr->iPageno - 1);
  }
}

/*
** Load a copy of the page data for page iPg into the buffer belonging
** to page object pPg. Allocate the buffer if necessary. Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
స్థిర పూర్ణాంకం stat_Get_పుట(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  Stat_పుట *pPg                   /* Load page into this object */
){
  పూర్ణాంకం pgsz = sqlite3_Btree_Get_పుట_కొలత(pBt);
  Db_పుట *p_Db_పుట = 0;
  పూర్ణాంకం rc;

  ఒకవేళ( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_పుట_PADDING_బైట్లు);
    ఒకవేళ( pPg->aPg==0 ){
      తిరిగి SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_పుట_PADDING_బైట్లు);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &p_Db_పుట, 0);
  ఒకవేళ( rc==SQLITE_OK ){
    స్థిరరాశి u8 *a = sqlite3_Pager_Get_దత్తాంశం(p_Db_పుట);
    జ్ఞాపకం_నకలు(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(p_Db_పుట);
  }

  తిరిగి rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
స్థిర పూర్ణాంకం stat_తదుపరి(sqlite3_vtab_cursor *pCursor){
  పూర్ణాంకం rc;
  పూర్ణాంకం nPayload;
  అక్షరం *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  Stat_పట్టిక *pTab = (Stat_పట్టిక *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->z_మార్గం);
  pCsr->z_మార్గం = 0;

stat_తదుపరి_Restart:
  ఒకవేళ( pCsr->i_పుట<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_అడుగు(pCsr->pStmt);
    ఒకవేళ( rc==SQLITE_అడ్డువరుస ){
      పూర్ణాంకం n_పుట;
      u32 i_మూలం = (u32)sqlite3_స్తంభం_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &n_పుట);
      ఒకవేళ( n_పుట==0 ){
        pCsr->isEof = 1;
        తిరిగి sqlite3_reset(pCsr->pStmt);
      }
      rc = stat_Get_పుట(pBt, i_మూలం, &pCsr->a_పుట[0]);
      pCsr->a_పుట[0].iPgno = i_మూలం;
      pCsr->a_పుట[0].iCell = 0;
      ఒకవేళ( !pCsr->isAgg ){
        pCsr->a_పుట[0].z_మార్గం = z = sqlite3_mprintf("/");
        ఒకవేళ( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->i_పుట = 0;
      pCsr->n_పుట = 1;
    }లేకపోతే{
      pCsr->isEof = 1;
      తిరిగి sqlite3_reset(pCsr->pStmt);
    }
  }లేకపోతే{
    /* Continue analyzing the btree previously started */
    Stat_పుట *p = &pCsr->a_పుట[pCsr->i_పుట];
    ఒకవేళ( !pCsr->isAgg ) statResetCounts(pCsr);
    అయితే( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      అయితే( pCell->iOvfl<pCell->nOvfl ){
        పూర్ణాంకం nUsable;
        పూర్ణాంకం iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3_Btree_Get_పుట_కొలత(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->n_పుట++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        stat_కొలత_And_స్థానభ్రంశం(pCsr);
        ఒకవేళ( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }లేకపోతే{
          pCsr->nPayload += pCell->n_చివరి_Ovfl;
          pCsr->n_వాడని += nUsable - 4 - pCell->n_చివరి_Ovfl;
        }
        pCell->iOvfl++;
        ఒకవేళ( !pCsr->isAgg ){
          pCsr->zName = (అక్షరం *)sqlite3_స్తంభం_పాఠ్యం(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->z_మార్గం = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->z_మార్గం, p->iCell, iOvfl
          );
          తిరిగి z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      ఒకవేళ( p->i_కుడి_బిడ్డ_Pg ) ఆపు;
      p->iCell++;
    }

    ఒకవేళ( !p->i_కుడి_బిడ్డ_Pg || p->iCell>p->nCell ){
      stat_Clear_పుట(p);
      pCsr->i_పుట--;
      ఒకవేళ( pCsr->isAgg && pCsr->i_పుట<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        తిరిగి SQLITE_OK;
      }
      వెళ్ళు stat_తదుపరి_Restart; /* Tail recursion */
    }
    pCsr->i_పుట++;
    ఒకవేళ( pCsr->i_పుట>=Array_కొలత(pCsr->a_పుట) ){
      statResetCsr(pCsr);
      తిరిగి SQLITE_CORRUPT_BKPT;
    }
    ప్రతిజ్ఞా( p==&pCsr->a_పుట[pCsr->i_పుట-1] );

    ఒకవేళ( p->iCell==p->nCell ){
      p[1].iPgno = p->i_కుడి_బిడ్డ_Pg;
    }లేకపోతే{
      p[1].iPgno = p->aCell[p->iCell].i_బిడ్డ_Pg;
    }
    rc = stat_Get_పుట(pBt, p[1].iPgno, &p[1]);
    pCsr->n_పుట++;
    p[1].iCell = 0;
    ఒకవేళ( !pCsr->isAgg ){
      p[1].z_మార్గం = z = sqlite3_mprintf("%s%.3x/", p->z_మార్గం, p->iCell);
      ఒకవేళ( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  ఒకవేళ( rc==SQLITE_OK ){
    పూర్ణాంకం i;
    Stat_పుట *p = &pCsr->a_పుట[pCsr->i_పుట];
    pCsr->zName = (అక్షరం *)sqlite3_స్తంభం_పాఠ్యం(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = stat_Decode_పుట(pBt, p);
    ఒకవేళ( rc==SQLITE_OK ){
      stat_కొలత_And_స్థానభ్రంశం(pCsr);

      ఎంపిక( p->flags ){
        స్థితి 0x05:             /* table internal */
        స్థితి 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          ఆపు;
        స్థితి 0x0D:             /* table leaf */
        స్థితి 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          ఆపు;
        అప్రమేయ:
          pCsr->zPagetype = "corrupted";
          ఆపు;
      }
      pCsr->nCell += p->nCell;
      pCsr->n_వాడని += p->n_వాడని;
      ఒకవేళ( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      ఒకవేళ( !pCsr->isAgg ){
        pCsr->z_మార్గం = z = sqlite3_mprintf("%s", p->z_మార్గం);
        ఒకవేళ( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      కొరకు(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      ఒకవేళ( pCsr->isAgg ) వెళ్ళు stat_తదుపరి_Restart;
    }
  }

  తిరిగి rc;
}

స్థిర పూర్ణాంకం statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  తిరిగి pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
స్థిర పూర్ణాంకం statFilter(
  sqlite3_vtab_cursor *pCursor, 
  పూర్ణాంకం idx_అంకె, స్థిరరాశి అక్షరం *idxStr,
  పూర్ణాంకం argc, sqlite3_విలువ **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  Stat_పట్టిక *pTab = (Stat_పట్టిక*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  అక్షరం *zSql;             /* String value of pSql */
  పూర్ణాంకం iArg = 0;           /* Count of argv[] parameters used so far */
  పూర్ణాంకం rc = SQLITE_OK;     /* Result of this operation */
  స్థిరరాశి అక్షరం *zName = 0;  /* Only provide analysis of this table */
  (శూన్య)argc;
  (శూన్య)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  ఒకవేళ( idx_అంకె & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    స్థిరరాశి అక్షరం *zDbase = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[iArg++]);
    pCsr->iDb = sqlite3_కనుగొను_Db_Name(pTab->db, zDbase);
    ఒకవేళ( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      తిరిగి SQLITE_OK;
    }
  }లేకపోతే{
    pCsr->iDb = pTab->iDb;
  }
  ఒకవేళ( idx_అంకె & 0x02 ){
    /* name=? constraint is present */
    zName = (స్థిరరాశి అక్షరం*)sqlite3_విలువ_పాఠ్యం(argv[iArg++]);
  }
  ఒకవేళ( idx_అంకె & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_విలువ_double(argv[iArg++])!=0.0;
  }లేకపోతే{
    pCsr->isAgg = 0;
  }
  pSql = sqlite3_str_new(pTab->db);
  sqlite3_str_appendf(pSql,
      "SELECT * FROM ("
        "SELECT 'sqlite_schema' AS name,1 AS rootpage,'table' AS type"
        " UNION ALL "
        "SELECT name,rootpage,type"
        " FROM \"%w\".sqlite_schema WHERE rootpage!=0)",
      pTab->db->aDb[pCsr->iDb].zDbSName);
  ఒకవేళ( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  ఒకవేళ( idx_అంకె & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_ముగింపు(pSql);
  ఒకవేళ( zSql==0 ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }లేకపోతే{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  ఒకవేళ( rc==SQLITE_OK ){
    pCsr->i_పుట = -1;
    rc = stat_తదుపరి(pCursor);
  }
  తిరిగి rc;
}

స్థిర పూర్ణాంకం stat_స్తంభం(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  పూర్ణాంకం i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  ఎంపిక( i ){
    స్థితి 0:            /* name */
      sqlite3_ఫలితం_పాఠ్యం(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      ఆపు;
    స్థితి 1:            /*మార్గం*/
      ఒకవేళ( !pCsr->isAgg ){
        sqlite3_ఫలితం_పాఠ్యం(ctx, pCsr->z_మార్గం, -1, SQLITE_TRANSIENT);
      }
      ఆపు;
    స్థితి 2:            /* pageno */
      ఒకవేళ( pCsr->isAgg ){
        sqlite3_ఫలితం_int64(ctx, pCsr->n_పుట);
      }లేకపోతే{
        sqlite3_ఫలితం_int64(ctx, pCsr->iPageno);
      }
      ఆపు;
    స్థితి 3:            /* pagetype */
      ఒకవేళ( !pCsr->isAgg ){
        sqlite3_ఫలితం_పాఠ్యం(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      ఆపు;
    స్థితి 4:            /* ncell */
      sqlite3_ఫలితం_int64(ctx, pCsr->nCell);
      ఆపు;
    స్థితి 5:            /* payload */
      sqlite3_ఫలితం_int64(ctx, pCsr->nPayload);
      ఆపు;
    స్థితి 6:            /*వాడని*/
      sqlite3_ఫలితం_int64(ctx, pCsr->n_వాడని);
      ఆపు;
    స్థితి 7:            /* mx_payload */
      sqlite3_ఫలితం_int64(ctx, pCsr->nMxPayload);
      ఆపు;
    స్థితి 8:            /* pgoffset */
      ఒకవేళ( !pCsr->isAgg ){
        sqlite3_ఫలితం_int64(ctx, pCsr->i_స్థానభ్రంశం);
      }
      ఆపు;
    స్థితి 9:            /* pgsize */
      sqlite3_ఫలితం_int64(ctx, pCsr->sz_పుట);
      ఆపు;
    స్థితి 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      పూర్ణాంకం iDb = pCsr->iDb;
      sqlite3_ఫలితం_పాఠ్యం(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      ఆపు;
    }
    అప్రమేయ: {         /* aggregate */
      sqlite3_ఫలితం_int(ctx, pCsr->isAgg);
      ఆపు;
    }
  }
  తిరిగి SQLITE_OK;
}

స్థిర పూర్ణాంకం statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  తిరిగి SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
పూర్ణాంకం sqlite3_Dbstat_రిజిస్టరు(sqlite3 *db){
  స్థిర sqlite3_module dbstat_module = {
    0,                            /* iVersion */
    statConnect,                  /* xCreate */
    statConnect,                  /* xConnect */
    stat_ఉత్తమ_సూచీ,                /* xBestIndex */
    statDisconnect,               /* xDisconnect */
    statDisconnect,               /* xDestroy */
    statOpen,                     /* xOpen - open a cursor */
    statClose,                    /* xClose - close a cursor */
    statFilter,                   /* xFilter - configure scan constraints */
    stat_తదుపరి,                     /* xNext - advance a cursor */
    statEof,                      /* xEof - check for end of scan */
    stat_స్తంభం,                   /* xColumn - read data */
    statRowid,                    /* xRowid - read data */
    0,                            /* xUpdate */
    0,                            /* xBegin */
    0,                            /* xSync */
    0,                            /* xCommit */
    0,                            /* xRollback */
    0,                            /* xFindMethod */
    0,                            /* xRename */
    0,                            /* xSavepoint */
    0,                            /* xRelease */
    0,                            /* xRollbackTo */
    0,                            /* xShadowName */
    0                             /* xIntegrity */
  };
  తిరిగి sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#లేదావేళ defined(SQLITE_ENABLE_DBSTAT_VTAB)
పూర్ణాంకం sqlite3_Dbstat_రిజిస్టరు(sqlite3 *db){ తిరిగి SQLITE_OK; }
#అయితే_ముగింపు /* SQLITE_ENABLE_DBSTAT_VTAB */
