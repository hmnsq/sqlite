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

#അന്തര്ഭാവ "sqliteInt.h"   /* Requires access to internal data structures */
#എങ്കിൽ (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

/*
** The pager and btree modules arrange objects in memory so that there are
** always approximately 200 bytes of addressable memory following each page
** buffer. This way small buffer overreads caused by corrupt database pages
** do not cause undefined behaviour. This module pads each page buffer
** by the following number of bytes for the same purpose.
*/
#define DBSTAT_PAGE_PADDING_BYTES 256

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
സ്ഥിതം സ്ഥിരം അക്ഷരം zDbstatSchema[] = 
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
തരനിർവചനം ഘടന StatTable StatTable;
തരനിർവചനം ഘടന StatCursor StatCursor;
തരനിർവചനം ഘടന StatPage StatPage;
തരനിർവചനം ഘടന StatCell StatCell;

/* Size information for a single cell within a btree page */
ഘടന StatCell {
  പൂർണ്ണസംഖ്യ nLocal;                     /* Bytes of local payload */
  u32 iChildPg;                   /* Child node (or 0 if this is a leaf) */
  പൂർണ്ണസംഖ്യ nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  പൂർണ്ണസംഖ്യ nLastOvfl;                  /* Bytes of payload on final overflow page */
  പൂർണ്ണസംഖ്യ iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
ഘടന StatPage {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  പൂർണ്ണസംഖ്യ iCell;                      /* Current cell */
  അക്ഷരം *zPath;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  പൂർണ്ണസംഖ്യ nCell;                      /* Number of cells on page */
  പൂർണ്ണസംഖ്യ nUnused;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_വലത്_Child_Pg;              /* Right-child page number (or 0) */
  പൂർണ്ണസംഖ്യ nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
ഘടന StatCursor {
  sqlite3_vtab_cursor base;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  പൂർണ്ണസംഖ്യ iDb;                        /* Schema used for this query */

  StatPage aPage[32];             /* Pages in path to current page */
  പൂർണ്ണസംഖ്യ iPage;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  അക്ഷരം *zName;                    /* Value of 'name' column */
  അക്ഷരം *zPath;                    /* Value of 'path' column */
  അക്ഷരം *zPagetype;                /* Value of 'pagetype' column */
  പൂർണ്ണസംഖ്യ nPage;                      /* Number of pages in current btree */
  പൂർണ്ണസംഖ്യ nCell;                      /* Value of 'ncell' column */
  പൂർണ്ണസംഖ്യ nMxPayload;                 /* Value of 'mx_payload' column */
  i64 nUnused;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 iOffset;                    /* Value of 'pgOffset' column */
  i64 szPage;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
ഘടന StatTable {
  sqlite3_vtab base;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  പൂർണ്ണസംഖ്യ iDb;                        /* Index of database to analyze */
};

#ifndef get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#endif

/*
** Connect to or create a new DBSTAT virtual table.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ statConnect(
  sqlite3 *db,
  ശൂന്യം *pAux,
  പൂർണ്ണസംഖ്യ argc, സ്ഥിരം അക്ഷരം *സ്ഥിരം*argv,
  sqlite3_vtab **ppVtab,
  അക്ഷരം **pzErr
){
  StatTable *pTab = 0;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  പൂർണ്ണസംഖ്യ iDb;
  (ശൂന്യം)pAux;

  എങ്കിൽ( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (അക്ഷരം*)argv[3]);
    iDb = sqlite3FindDb(db, &nm);
    എങ്കിൽ( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      തിരികെ SQLITE_ERROR;
    }
  }അല്ലെങ്കിൽ{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  എങ്കിൽ( rc==SQLITE_OK ){
    pTab = (StatTable *)sqlite3_malloc64(വലുപ്പം(StatTable));
    എങ്കിൽ( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  പ്രതിജ്ഞാ( rc==SQLITE_OK || pTab==0 );
  എങ്കിൽ( rc==SQLITE_OK ){
    memset(pTab, 0, വലുപ്പം(StatTable));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  തിരികെ rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  തിരികെ SQLITE_OK;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ statBestIndex(sqlite3_vtab *tab, sqlite3_index_info *pIdxInfo){
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ iSchema = -1;
  പൂർണ്ണസംഖ്യ iName = -1;
  പൂർണ്ണസംഖ്യ iAgg = -1;
  (ശൂന്യം)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  വേണ്ടി(i=0; i<pIdxInfo->nConstraint; i++){
    എങ്കിൽ( pIdxInfo->aConstraint[i].op!=SQLITE_INDEX_CONSTRAINT_EQ ) തുടരുക;
    എങ്കിൽ( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      തിരികെ SQLITE_CONSTRAINT;
    }
    തിരഞ്ഞെടുപ്പ്( pIdxInfo->aConstraint[i].iColumn ){
      അവസ്ഥ 0: {    /* name */
        iName = i;
        നിർത്തുക;
      }
      അവസ്ഥ 10: {   /* schema */
        iSchema = i;
        നിർത്തുക;
      }
      അവസ്ഥ 11: {   /* aggregate */
        iAgg = i;
        നിർത്തുക;
      }
    }
  }
  i = 0;
  എങ്കിൽ( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argvIndex = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_അക്കം |= 0x01;
  }
  എങ്കിൽ( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argvIndex = ++i;
    pIdxInfo->idx_അക്കം |= 0x02;
  }
  എങ്കിൽ( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argvIndex = ++i;
    pIdxInfo->idx_അക്കം |= 0x04;
  }
  pIdxInfo->estimatedCost = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  എങ്കിൽ( ( pIdxInfo->nOrderBy==1
     && pIdxInfo->aOrderBy[0].iColumn==0
     && pIdxInfo->aOrderBy[0].desc==0
     ) ||
      ( pIdxInfo->nOrderBy==2
     && pIdxInfo->aOrderBy[0].iColumn==0
     && pIdxInfo->aOrderBy[0].desc==0
     && pIdxInfo->aOrderBy[1].iColumn==1
     && pIdxInfo->aOrderBy[1].desc==0
     )
  ){
    pIdxInfo->orderByConsumed = 1;
    pIdxInfo->idx_അക്കം |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_INDEX_SCAN_HEX;

  തിരികെ SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  StatTable *pTab = (StatTable *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(വലുപ്പം(StatCursor));
  എങ്കിൽ( pCsr==0 ){
    തിരികെ SQLITE_NOMEM_BKPT;
  }അല്ലെങ്കിൽ{
    memset(pCsr, 0, വലുപ്പം(StatCursor));
    pCsr->base.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  തിരികെ SQLITE_OK;
}

സ്ഥിതം ശൂന്യം statClearCells(StatPage *p){
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( p->aCell ){
    വേണ്ടി(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

സ്ഥിതം ശൂന്യം statClearPage(StatPage *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->zPath);
  memset(p, 0, വലുപ്പം(StatPage));
  p->aPg = aPg;
}

സ്ഥിതം ശൂന്യം statResetCsr(StatCursor *pCsr){
  പൂർണ്ണസംഖ്യ i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  വേണ്ടി(i=0; i<ArraySize(pCsr->aPage); i++){
    statClearPage(&pCsr->aPage[i]);
    sqlite3_free(pCsr->aPage[i].aPg);
    pCsr->aPage[i].aPg = 0;
  }
  sqlite3_reset(pCsr->pStmt);
  pCsr->iPage = 0;
  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;
  pCsr->isEof = 0;
}

/* Resize the space-used counters inside of the cursor */
സ്ഥിതം ശൂന്യം statResetCounts(StatCursor *pCsr){
  pCsr->nCell = 0;
  pCsr->nMxPayload = 0;
  pCsr->nUnused = 0;
  pCsr->nPayload = 0;
  pCsr->szPage = 0;
  pCsr->nPage = 0;
}

/*
** Close a DBSTAT cursor.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  തിരികെ SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ getLocalPayload(
  പൂർണ്ണസംഖ്യ nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  പൂർണ്ണസംഖ്യ nTotal                      /* Total record (payload) size */
){
  പൂർണ്ണസംഖ്യ nLocal;
  പൂർണ്ണസംഖ്യ nMinLocal;
  പൂർണ്ണസംഖ്യ nMaxLocal;
 
  എങ്കിൽ( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }അല്ലെങ്കിൽ{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (nTotal - nMinLocal) % (nUsable - 4);
  എങ്കിൽ( nLocal>nMaxLocal ) nLocal = nMinLocal;
  തിരികെ nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ statDecodePage(Btree *pBt, StatPage *p){
  പൂർണ്ണസംഖ്യ nUnused;
  പൂർണ്ണസംഖ്യ iOff;
  പൂർണ്ണസംഖ്യ nHdr;
  പൂർണ്ണസംഖ്യ isLeaf;
  പൂർണ്ണസംഖ്യ szPage;

  u8 *aData = p->aPg;
  u8 *aHdr = &aData[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  എങ്കിൽ( p->flags==0x0A || p->flags==0x0D ){
    isLeaf = 1;
    nHdr = 8;
  }അല്ലെങ്കിൽ എങ്കിൽ( p->flags==0x05 || p->flags==0x02 ){
    isLeaf = 0;
    nHdr = 12;
  }അല്ലെങ്കിൽ{
    പോകുക statPageIsCorrupt;
  }
  എങ്കിൽ( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  szPage = sqlite3BtreeGetPageSize(pBt);

  nUnused = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  nUnused += (പൂർണ്ണസംഖ്യ)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  ആകുമ്പോൾ( iOff ){
    പൂർണ്ണസംഖ്യ iNext;
    എങ്കിൽ( iOff>=szPage ) പോകുക statPageIsCorrupt;
    nUnused += get2byte(&aData[iOff+2]);
    iNext = get2byte(&aData[iOff]);
    എങ്കിൽ( iNext<iOff+4 && iNext>0 ) പോകുക statPageIsCorrupt;
    iOff = iNext;
  }
  p->nUnused = nUnused;
  p->i_വലത്_Child_Pg = isLeaf ? 0 : sqlite3Get4byte(&aHdr[8]);

  എങ്കിൽ( p->nCell ){
    പൂർണ്ണസംഖ്യ i;                        /* Used to iterate through cells */
    പൂർണ്ണസംഖ്യ nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = szPage - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * വലുപ്പം(StatCell));
    എങ്കിൽ( p->aCell==0 ) തിരികെ SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * വലുപ്പം(StatCell));

    വേണ്ടി(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&aData[nHdr+i*2]);
      എങ്കിൽ( iOff<nHdr || iOff>=szPage ) പോകുക statPageIsCorrupt;
      എങ്കിൽ( !isLeaf ){
        pCell->iChildPg = sqlite3Get4byte(&aData[iOff]);
        iOff += 4;
      }
      എങ്കിൽ( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }അല്ലെങ്കിൽ{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        പൂർണ്ണസംഖ്യ nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&aData[iOff], nPayload);
        എങ്കിൽ( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&aData[iOff], &dummy);
        }
        എങ്കിൽ( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        എങ്കിൽ( nLocal<0 ) പോകുക statPageIsCorrupt;
        pCell->nLocal = nLocal;
        പ്രതിജ്ഞാ( nPayload>=(u32)nLocal );
        പ്രതിജ്ഞാ( nLocal<=(nUsable-35) );
        എങ്കിൽ( nPayload>(u32)nLocal ){
          പൂർണ്ണസംഖ്യ j;
          പൂർണ്ണസംഖ്യ nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          എങ്കിൽ( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            പോകുക statPageIsCorrupt;
          }
          pCell->nLastOvfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(വലുപ്പം(u32)*nOvfl);
          എങ്കിൽ( pCell->aOvfl==0 ) തിരികെ SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&aData[iOff+nLocal]);
          വേണ്ടി(j=1; j<nOvfl; j++){
            പൂർണ്ണസംഖ്യ rc;
            u32 iPrev = pCell->aOvfl[j-1];
            DbPage *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            എങ്കിൽ( rc!=SQLITE_OK ){
              പ്രതിജ്ഞാ( pPg==0 );
              തിരികെ rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3PagerGetData(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  തിരികെ SQLITE_OK;

statPageIsCorrupt:
  p->flags = 0;
  statClearCells(p);
  തിരികെ SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
സ്ഥിതം ശൂന്യം statSizeAndOffset(StatCursor *pCsr){
  StatTable *pTab = (StatTable *)((sqlite3_vtab_cursor *)pCsr)->pVtab;
  Btree *pBt = pTab->db->aDb[pTab->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);
  sqlite3_file *fd;
  sqlite3_int64 x[2];

  /* If connected to a ZIPVFS backend, find the page size and
  ** offset from ZIPVFS.
  */
  fd = sqlite3PagerFile(pPager);
  x[0] = pCsr->iPageno;
  എങ്കിൽ( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->iOffset = x[0];
    pCsr->szPage += x[1];
  }അല്ലെങ്കിൽ{
    /* Not ZIPVFS: The default page size and offset */
    pCsr->szPage += sqlite3BtreeGetPageSize(pBt);
    pCsr->iOffset = (i64)pCsr->szPage * (pCsr->iPageno - 1);
  }
}

/*
** Load a copy of the page data for page iPg into the buffer belonging
** to page object pPg. Allocate the buffer if necessary. Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ statGetPage(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  StatPage *pPg                   /* Load page into this object */
){
  പൂർണ്ണസംഖ്യ pgsz = sqlite3BtreeGetPageSize(pBt);
  DbPage *pDbPage = 0;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_PAGE_PADDING_BYTES);
    എങ്കിൽ( pPg->aPg==0 ){
      തിരികെ SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_PAGE_PADDING_BYTES);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &pDbPage, 0);
  എങ്കിൽ( rc==SQLITE_OK ){
    സ്ഥിരം u8 *a = sqlite3PagerGetData(pDbPage);
    memcpy(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(pDbPage);
  }

  തിരികെ rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ statNext(sqlite3_vtab_cursor *pCursor){
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ nPayload;
  അക്ഷരം *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->zPath);
  pCsr->zPath = 0;

statNextRestart:
  എങ്കിൽ( pCsr->iPage<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_step(pCsr->pStmt);
    എങ്കിൽ( rc==SQLITE_ROW ){
      പൂർണ്ണസംഖ്യ nPage;
      u32 i_വേര് = (u32)sqlite3_column_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &nPage);
      എങ്കിൽ( nPage==0 ){
        pCsr->isEof = 1;
        തിരികെ sqlite3_reset(pCsr->pStmt);
      }
      rc = statGetPage(pBt, i_വേര്, &pCsr->aPage[0]);
      pCsr->aPage[0].iPgno = i_വേര്;
      pCsr->aPage[0].iCell = 0;
      എങ്കിൽ( !pCsr->isAgg ){
        pCsr->aPage[0].zPath = z = sqlite3_mprintf("/");
        എങ്കിൽ( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->iPage = 0;
      pCsr->nPage = 1;
    }അല്ലെങ്കിൽ{
      pCsr->isEof = 1;
      തിരികെ sqlite3_reset(pCsr->pStmt);
    }
  }അല്ലെങ്കിൽ{
    /* Continue analyzing the btree previously started */
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    എങ്കിൽ( !pCsr->isAgg ) statResetCounts(pCsr);
    ആകുമ്പോൾ( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      ആകുമ്പോൾ( pCell->iOvfl<pCell->nOvfl ){
        പൂർണ്ണസംഖ്യ nUsable;
        പൂർണ്ണസംഖ്യ iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3BtreeGetPageSize(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->nPage++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        statSizeAndOffset(pCsr);
        എങ്കിൽ( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }അല്ലെങ്കിൽ{
          pCsr->nPayload += pCell->nLastOvfl;
          pCsr->nUnused += nUsable - 4 - pCell->nLastOvfl;
        }
        pCell->iOvfl++;
        എങ്കിൽ( !pCsr->isAgg ){
          pCsr->zName = (അക്ഷരം *)sqlite3_column_text(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->zPath = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->zPath, p->iCell, iOvfl
          );
          തിരികെ z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      എങ്കിൽ( p->i_വലത്_Child_Pg ) നിർത്തുക;
      p->iCell++;
    }

    എങ്കിൽ( !p->i_വലത്_Child_Pg || p->iCell>p->nCell ){
      statClearPage(p);
      pCsr->iPage--;
      എങ്കിൽ( pCsr->isAgg && pCsr->iPage<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        തിരികെ SQLITE_OK;
      }
      പോകുക statNextRestart; /* Tail recursion */
    }
    pCsr->iPage++;
    എങ്കിൽ( pCsr->iPage>=ArraySize(pCsr->aPage) ){
      statResetCsr(pCsr);
      തിരികെ SQLITE_CORRUPT_BKPT;
    }
    പ്രതിജ്ഞാ( p==&pCsr->aPage[pCsr->iPage-1] );

    എങ്കിൽ( p->iCell==p->nCell ){
      p[1].iPgno = p->i_വലത്_Child_Pg;
    }അല്ലെങ്കിൽ{
      p[1].iPgno = p->aCell[p->iCell].iChildPg;
    }
    rc = statGetPage(pBt, p[1].iPgno, &p[1]);
    pCsr->nPage++;
    p[1].iCell = 0;
    എങ്കിൽ( !pCsr->isAgg ){
      p[1].zPath = z = sqlite3_mprintf("%s%.3x/", p->zPath, p->iCell);
      എങ്കിൽ( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  എങ്കിൽ( rc==SQLITE_OK ){
    പൂർണ്ണസംഖ്യ i;
    StatPage *p = &pCsr->aPage[pCsr->iPage];
    pCsr->zName = (അക്ഷരം *)sqlite3_column_text(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = statDecodePage(pBt, p);
    എങ്കിൽ( rc==SQLITE_OK ){
      statSizeAndOffset(pCsr);

      തിരഞ്ഞെടുപ്പ്( p->flags ){
        അവസ്ഥ 0x05:             /* table internal */
        അവസ്ഥ 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          നിർത്തുക;
        അവസ്ഥ 0x0D:             /* table leaf */
        അവസ്ഥ 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          നിർത്തുക;
        സ്ഥിരസ്ഥിതി:
          pCsr->zPagetype = "corrupted";
          നിർത്തുക;
      }
      pCsr->nCell += p->nCell;
      pCsr->nUnused += p->nUnused;
      എങ്കിൽ( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      എങ്കിൽ( !pCsr->isAgg ){
        pCsr->zPath = z = sqlite3_mprintf("%s", p->zPath);
        എങ്കിൽ( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      വേണ്ടി(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      എങ്കിൽ( pCsr->isAgg ) പോകുക statNextRestart;
    }
  }

  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  തിരികെ pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ statFilter(
  sqlite3_vtab_cursor *pCursor, 
  പൂർണ്ണസംഖ്യ idx_അക്കം, സ്ഥിരം അക്ഷരം *idxStr,
  പൂർണ്ണസംഖ്യ argc, sqlite3_മൂല്യം **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  StatTable *pTab = (StatTable*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  അക്ഷരം *zSql;             /* String value of pSql */
  പൂർണ്ണസംഖ്യ iArg = 0;           /* Count of argv[] parameters used so far */
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;     /* Result of this operation */
  സ്ഥിരം അക്ഷരം *zName = 0;  /* Only provide analysis of this table */
  (ശൂന്യം)argc;
  (ശൂന്യം)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  എങ്കിൽ( idx_അക്കം & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    സ്ഥിരം അക്ഷരം *zDbase = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[iArg++]);
    pCsr->iDb = sqlite3FindDbName(pTab->db, zDbase);
    എങ്കിൽ( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      തിരികെ SQLITE_OK;
    }
  }അല്ലെങ്കിൽ{
    pCsr->iDb = pTab->iDb;
  }
  എങ്കിൽ( idx_അക്കം & 0x02 ){
    /* name=? constraint is present */
    zName = (സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(argv[iArg++]);
  }
  എങ്കിൽ( idx_അക്കം & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_മൂല്യം_double(argv[iArg++])!=0.0;
  }അല്ലെങ്കിൽ{
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
  എങ്കിൽ( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  എങ്കിൽ( idx_അക്കം & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_finish(pSql);
  എങ്കിൽ( zSql==0 ){
    തിരികെ SQLITE_NOMEM_BKPT;
  }അല്ലെങ്കിൽ{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  എങ്കിൽ( rc==SQLITE_OK ){
    pCsr->iPage = -1;
    rc = statNext(pCursor);
  }
  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ statColumn(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  പൂർണ്ണസംഖ്യ i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  തിരഞ്ഞെടുപ്പ്( i ){
    അവസ്ഥ 0:            /* name */
      sqlite3_result_text(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      നിർത്തുക;
    അവസ്ഥ 1:            /* path */
      എങ്കിൽ( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPath, -1, SQLITE_TRANSIENT);
      }
      നിർത്തുക;
    അവസ്ഥ 2:            /* pageno */
      എങ്കിൽ( pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->nPage);
      }അല്ലെങ്കിൽ{
        sqlite3_result_int64(ctx, pCsr->iPageno);
      }
      നിർത്തുക;
    അവസ്ഥ 3:            /* pagetype */
      എങ്കിൽ( !pCsr->isAgg ){
        sqlite3_result_text(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      നിർത്തുക;
    അവസ്ഥ 4:            /* ncell */
      sqlite3_result_int64(ctx, pCsr->nCell);
      നിർത്തുക;
    അവസ്ഥ 5:            /* payload */
      sqlite3_result_int64(ctx, pCsr->nPayload);
      നിർത്തുക;
    അവസ്ഥ 6:            /* unused */
      sqlite3_result_int64(ctx, pCsr->nUnused);
      നിർത്തുക;
    അവസ്ഥ 7:            /* mx_payload */
      sqlite3_result_int64(ctx, pCsr->nMxPayload);
      നിർത്തുക;
    അവസ്ഥ 8:            /* pgoffset */
      എങ്കിൽ( !pCsr->isAgg ){
        sqlite3_result_int64(ctx, pCsr->iOffset);
      }
      നിർത്തുക;
    അവസ്ഥ 9:            /* pgsize */
      sqlite3_result_int64(ctx, pCsr->szPage);
      നിർത്തുക;
    അവസ്ഥ 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      പൂർണ്ണസംഖ്യ iDb = pCsr->iDb;
      sqlite3_result_text(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {         /* aggregate */
      sqlite3_result_int(ctx, pCsr->isAgg);
      നിർത്തുക;
    }
  }
  തിരികെ SQLITE_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  തിരികെ SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
പൂർണ്ണസംഖ്യ sqlite3DbstatRegister(sqlite3 *db){
  സ്ഥിതം sqlite3_module dbstat_module = {
    0,                            /* iVersion */
    statConnect,                  /* xCreate */
    statConnect,                  /* xConnect */
    statBestIndex,                /* xBestIndex */
    statDisconnect,               /* xDisconnect */
    statDisconnect,               /* xDestroy */
    statOpen,                     /* xOpen - open a cursor */
    statClose,                    /* xClose - close a cursor */
    statFilter,                   /* xFilter - configure scan constraints */
    statNext,                     /* xNext - advance a cursor */
    statEof,                      /* xEof - check for end of scan */
    statColumn,                   /* xColumn - read data */
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
  തിരികെ sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#അല്ലെങ്കിലെങ്കിൽ defined(SQLITE_ENABLE_DBSTAT_VTAB)
പൂർണ്ണസംഖ്യ sqlite3DbstatRegister(sqlite3 *db){ തിരികെ SQLITE_OK; }
#endif /* SQLITE_ENABLE_DBSTAT_VTAB */
