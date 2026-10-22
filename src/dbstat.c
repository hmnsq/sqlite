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

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"   /* Requires access to internal data structures */
#ಆದರೆ (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

/*
** The pager and btree modules arrange objects in memory so that there are
** always approximately 200 bytes of addressable memory following each page
** buffer. This way small buffer overreads caused by corrupt database pages
** do not cause undefined behaviour. This module pads each page buffer
** by the following number of bytes for the same purpose.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು DBSTAT_ಪುಟ_PADDING_ಬೈಟುಗಳು 256

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
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ zDbstatSchema[] = 
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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Stat_ಕೋಷ್ಟಕ Stat_ಕೋಷ್ಟಕ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ StatCursor StatCursor;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Stat_ಪುಟ Stat_ಪುಟ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ StatCell StatCell;

/* Size information for a single cell within a btree page */
ರಚನೆ StatCell {
  ಪೂರ್ಣಾಂಕ nLocal;                     /* Bytes of local payload */
  u32 i_ಮಗು_Pg;                   /* Child node (or 0 if this is a leaf) */
  ಪೂರ್ಣಾಂಕ nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  ಪೂರ್ಣಾಂಕ n_ಕೊನೆಯ_Ovfl;                  /* Bytes of payload on final overflow page */
  ಪೂರ್ಣಾಂಕ iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
ರಚನೆ Stat_ಪುಟ {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  ಪೂರ್ಣಾಂಕ iCell;                      /* Current cell */
  ಅಕ್ಷರ *z_ಮಾರ್ಗ;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  ಪೂರ್ಣಾಂಕ nCell;                      /* Number of cells on page */
  ಪೂರ್ಣಾಂಕ n_ಬಳಸದ;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_ಬಲ_ಮಗು_Pg;              /* Right-child page number (or 0) */
  ಪೂರ್ಣಾಂಕ nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
ರಚನೆ StatCursor {
  sqlite3_vtab_cursor ತಳ;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  ಪೂರ್ಣಾಂಕ iDb;                        /* Schema used for this query */

  Stat_ಪುಟ a_ಪುಟ[32];             /* Pages in path to current page */
  ಪೂರ್ಣಾಂಕ i_ಪುಟ;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  ಅಕ್ಷರ *zName;                    /* Value of 'name' column */
  ಅಕ್ಷರ *z_ಮಾರ್ಗ;                    /* Value of 'path' column */
  ಅಕ್ಷರ *zPagetype;                /* Value of 'pagetype' column */
  ಪೂರ್ಣಾಂಕ n_ಪುಟ;                      /* Number of pages in current btree */
  ಪೂರ್ಣಾಂಕ nCell;                      /* Value of 'ncell' column */
  ಪೂರ್ಣಾಂಕ nMxPayload;                 /* Value of 'mx_payload' column */
  i64 n_ಬಳಸದ;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 i_ಸ್ಥಾನಾಂತರ;                    /* Value of 'pgOffset' column */
  i64 sz_ಪುಟ;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
ರಚನೆ Stat_ಕೋಷ್ಟಕ {
  sqlite3_vtab ತಳ;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  ಪೂರ್ಣಾಂಕ iDb;                        /* Index of database to analyze */
};

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#ಆದರೆ_ಅಂತ್ಯ

/*
** Connect to or create a new DBSTAT virtual table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ statConnect(
  sqlite3 *db,
  ಶೂನ್ಯ *pAux,
  ಪೂರ್ಣಾಂಕ argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*argv,
  sqlite3_vtab **ppVtab,
  ಅಕ್ಷರ **pzErr
){
  Stat_ಕೋಷ್ಟಕ *pTab = 0;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ iDb;
  (ಶೂನ್ಯ)pAux;

  ಆದರೆ( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (ಅಕ್ಷರ*)argv[3]);
    iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db(db, &nm);
    ಆದರೆ( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      ಹಿಂತಿರುಗಿ SQLITE_ERROR;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  ಆದರೆ( rc==SQLITE_OK ){
    pTab = (Stat_ಕೋಷ್ಟಕ *)sqlite3_malloc64(ಗಾತ್ರ(Stat_ಕೋಷ್ಟಕ));
    ಆದರೆ( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || pTab==0 );
  ಆದರೆ( rc==SQLITE_OK ){
    memset(pTab, 0, ಗಾತ್ರ(Stat_ಕೋಷ್ಟಕ));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stat_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ(sqlite3_vtab *tab, sqlite3_ಸೂಚ್ಯಂಕ_info *pIdxInfo){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ iSchema = -1;
  ಪೂರ್ಣಾಂಕ iName = -1;
  ಪೂರ್ಣಾಂಕ iAgg = -1;
  (ಶೂನ್ಯ)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  ಗಾಗಿ(i=0; i<pIdxInfo->nConstraint; i++){
    ಆದರೆ( pIdxInfo->aConstraint[i].op!=SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_EQ ) ಮುಂದುವರಿ;
    ಆದರೆ( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      ಹಿಂತಿರುಗಿ SQLITE_CONSTRAINT;
    }
    ಆಯ್ಕೆ( pIdxInfo->aConstraint[i].i_ಸ್ತಂಭ ){
      ಸ್ಥಿತಿ 0: {    /* name */
        iName = i;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 10: {   /* schema */
        iSchema = i;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 11: {   /* aggregate */
        iAgg = i;
        ನಿಲ್ಲಿಸು;
      }
    }
  }
  i = 0;
  ಆದರೆ( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argv_ಸೂಚ್ಯಂಕ = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_ಅಂಕೆ |= 0x01;
  }
  ಆದರೆ( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argv_ಸೂಚ್ಯಂಕ = ++i;
    pIdxInfo->idx_ಅಂಕೆ |= 0x02;
  }
  ಆದರೆ( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argv_ಸೂಚ್ಯಂಕ = ++i;
    pIdxInfo->idx_ಅಂಕೆ |= 0x04;
  }
  pIdxInfo->estimated_ವೆಚ್ಚ = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  ಆದರೆ( ( pIdxInfo->n_ಕ್ರಮ_By==1
     && pIdxInfo->a_ಕ್ರಮ_By[0].i_ಸ್ತಂಭ==0
     && pIdxInfo->a_ಕ್ರಮ_By[0].desc==0
     ) ||
      ( pIdxInfo->n_ಕ್ರಮ_By==2
     && pIdxInfo->a_ಕ್ರಮ_By[0].i_ಸ್ತಂಭ==0
     && pIdxInfo->a_ಕ್ರಮ_By[0].desc==0
     && pIdxInfo->a_ಕ್ರಮ_By[1].i_ಸ್ತಂಭ==1
     && pIdxInfo->a_ಕ್ರಮ_By[1].desc==0
     )
  ){
    pIdxInfo->ಕ್ರಮ_By_Consumed = 1;
    pIdxInfo->idx_ಅಂಕೆ |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_ಸೂಚ್ಯಂಕ_ಪರಿಶೀಲನೆ_HEX;

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  Stat_ಕೋಷ್ಟಕ *pTab = (Stat_ಕೋಷ್ಟಕ *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(ಗಾತ್ರ(StatCursor));
  ಆದರೆ( pCsr==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }ಇಲ್ಲದಿದ್ದರೆ{
    memset(pCsr, 0, ಗಾತ್ರ(StatCursor));
    pCsr->ತಳ.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ statClearCells(Stat_ಪುಟ *p){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( p->aCell ){
    ಗಾಗಿ(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ stat_Clear_ಪುಟ(Stat_ಪುಟ *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->z_ಮಾರ್ಗ);
  memset(p, 0, ಗಾತ್ರ(Stat_ಪುಟ));
  p->aPg = aPg;
}

ಸ್ಥಿರ ಶೂನ್ಯ statResetCsr(StatCursor *pCsr){
  ಪೂರ್ಣಾಂಕ i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(pCsr->a_ಪುಟ); i++){
    stat_Clear_ಪುಟ(&pCsr->a_ಪುಟ[i]);
    sqlite3_free(pCsr->a_ಪುಟ[i].aPg);
    pCsr->a_ಪುಟ[i].aPg = 0;
  }
  sqlite3_reset(pCsr->pStmt);
  pCsr->i_ಪುಟ = 0;
  sqlite3_free(pCsr->z_ಮಾರ್ಗ);
  pCsr->z_ಮಾರ್ಗ = 0;
  pCsr->isEof = 0;
}

/* Resize the space-used counters inside of the cursor */
ಸ್ಥಿರ ಶೂನ್ಯ statResetCounts(StatCursor *pCsr){
  pCsr->nCell = 0;
  pCsr->nMxPayload = 0;
  pCsr->n_ಬಳಸದ = 0;
  pCsr->nPayload = 0;
  pCsr->sz_ಪುಟ = 0;
  pCsr->n_ಪುಟ = 0;
}

/*
** Close a DBSTAT cursor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getLocalPayload(
  ಪೂರ್ಣಾಂಕ nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  ಪೂರ್ಣಾಂಕ n_ಒಟ್ಟು                      /* Total record (payload) size */
){
  ಪೂರ್ಣಾಂಕ nLocal;
  ಪೂರ್ಣಾಂಕ nMinLocal;
  ಪೂರ್ಣಾಂಕ nMaxLocal;
 
  ಆದರೆ( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }ಇಲ್ಲದಿದ್ದರೆ{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (n_ಒಟ್ಟು - nMinLocal) % (nUsable - 4);
  ಆದರೆ( nLocal>nMaxLocal ) nLocal = nMinLocal;
  ಹಿಂತಿರುಗಿ nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stat_Decode_ಪುಟ(Btree *pBt, Stat_ಪುಟ *p){
  ಪೂರ್ಣಾಂಕ n_ಬಳಸದ;
  ಪೂರ್ಣಾಂಕ iOff;
  ಪೂರ್ಣಾಂಕ nHdr;
  ಪೂರ್ಣಾಂಕ is_ಎಲೆ;
  ಪೂರ್ಣಾಂಕ sz_ಪುಟ;

  u8 *a_ದತ್ತಾಂಶ = p->aPg;
  u8 *aHdr = &a_ದತ್ತಾಂಶ[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  ಆದರೆ( p->flags==0x0A || p->flags==0x0D ){
    is_ಎಲೆ = 1;
    nHdr = 8;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->flags==0x05 || p->flags==0x02 ){
    is_ಎಲೆ = 0;
    nHdr = 12;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹೋಗು stat_ಪುಟ_Is_Corrupt;
  }
  ಆದರೆ( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  sz_ಪುಟ = sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(pBt);

  n_ಬಳಸದ = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  n_ಬಳಸದ += (ಪೂರ್ಣಾಂಕ)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  ಆಗ( iOff ){
    ಪೂರ್ಣಾಂಕ i_ಮುಂದಿನ;
    ಆದರೆ( iOff>=sz_ಪುಟ ) ಹೋಗು stat_ಪುಟ_Is_Corrupt;
    n_ಬಳಸದ += get2byte(&a_ದತ್ತಾಂಶ[iOff+2]);
    i_ಮುಂದಿನ = get2byte(&a_ದತ್ತಾಂಶ[iOff]);
    ಆದರೆ( i_ಮುಂದಿನ<iOff+4 && i_ಮುಂದಿನ>0 ) ಹೋಗು stat_ಪುಟ_Is_Corrupt;
    iOff = i_ಮುಂದಿನ;
  }
  p->n_ಬಳಸದ = n_ಬಳಸದ;
  p->i_ಬಲ_ಮಗು_Pg = is_ಎಲೆ ? 0 : sqlite3Get4byte(&aHdr[8]);

  ಆದರೆ( p->nCell ){
    ಪೂರ್ಣಾಂಕ i;                        /* Used to iterate through cells */
    ಪೂರ್ಣಾಂಕ nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = sz_ಪುಟ - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * ಗಾತ್ರ(StatCell));
    ಆದರೆ( p->aCell==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * ಗಾತ್ರ(StatCell));

    ಗಾಗಿ(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&a_ದತ್ತಾಂಶ[nHdr+i*2]);
      ಆದರೆ( iOff<nHdr || iOff>=sz_ಪುಟ ) ಹೋಗು stat_ಪುಟ_Is_Corrupt;
      ಆದರೆ( !is_ಎಲೆ ){
        pCell->i_ಮಗು_Pg = sqlite3Get4byte(&a_ದತ್ತಾಂಶ[iOff]);
        iOff += 4;
      }
      ಆದರೆ( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }ಇಲ್ಲದಿದ್ದರೆ{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        ಪೂರ್ಣಾಂಕ nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&a_ದತ್ತಾಂಶ[iOff], nPayload);
        ಆದರೆ( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&a_ದತ್ತಾಂಶ[iOff], &dummy);
        }
        ಆದರೆ( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        ಆದರೆ( nLocal<0 ) ಹೋಗು stat_ಪುಟ_Is_Corrupt;
        pCell->nLocal = nLocal;
        ಪ್ರತಿಜ್ಞಾ( nPayload>=(u32)nLocal );
        ಪ್ರತಿಜ್ಞಾ( nLocal<=(nUsable-35) );
        ಆದರೆ( nPayload>(u32)nLocal ){
          ಪೂರ್ಣಾಂಕ j;
          ಪೂರ್ಣಾಂಕ nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          ಆದರೆ( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            ಹೋಗು stat_ಪುಟ_Is_Corrupt;
          }
          pCell->n_ಕೊನೆಯ_Ovfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(ಗಾತ್ರ(u32)*nOvfl);
          ಆದರೆ( pCell->aOvfl==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&a_ದತ್ತಾಂಶ[iOff+nLocal]);
          ಗಾಗಿ(j=1; j<nOvfl; j++){
            ಪೂರ್ಣಾಂಕ rc;
            u32 iPrev = pCell->aOvfl[j-1];
            Db_ಪುಟ *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            ಆದರೆ( rc!=SQLITE_OK ){
              ಪ್ರತಿಜ್ಞಾ( pPg==0 );
              ಹಿಂತಿರುಗಿ rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3_Pager_Get_ದತ್ತಾಂಶ(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;

stat_ಪುಟ_Is_Corrupt:
  p->flags = 0;
  statClearCells(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
ಸ್ಥಿರ ಶೂನ್ಯ stat_ಗಾತ್ರ_And_ಸ್ಥಾನಾಂತರ(StatCursor *pCsr){
  Stat_ಕೋಷ್ಟಕ *pTab = (Stat_ಕೋಷ್ಟಕ *)((sqlite3_vtab_cursor *)pCsr)->pVtab;
  Btree *pBt = pTab->db->aDb[pTab->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);
  sqlite3_file *fd;
  sqlite3_int64 x[2];

  /* If connected to a ZIPVFS backend, find the page size and
  ** offset from ZIPVFS.
  */
  fd = sqlite3PagerFile(pPager);
  x[0] = pCsr->iPageno;
  ಆದರೆ( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->i_ಸ್ಥಾನಾಂತರ = x[0];
    pCsr->sz_ಪುಟ += x[1];
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Not ZIPVFS: The default page size and offset */
    pCsr->sz_ಪುಟ += sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(pBt);
    pCsr->i_ಸ್ಥಾನಾಂತರ = (i64)pCsr->sz_ಪುಟ * (pCsr->iPageno - 1);
  }
}

/*
** Load a copy of the page data for page iPg into the buffer belonging
** to page object pPg. Allocate the buffer if necessary. Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stat_Get_ಪುಟ(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  Stat_ಪುಟ *pPg                   /* Load page into this object */
){
  ಪೂರ್ಣಾಂಕ pgsz = sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(pBt);
  Db_ಪುಟ *p_Db_ಪುಟ = 0;
  ಪೂರ್ಣಾಂಕ rc;

  ಆದರೆ( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_ಪುಟ_PADDING_ಬೈಟುಗಳು);
    ಆದರೆ( pPg->aPg==0 ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_ಪುಟ_PADDING_ಬೈಟುಗಳು);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &p_Db_ಪುಟ, 0);
  ಆದರೆ( rc==SQLITE_OK ){
    ಸ್ಥಿರಾಂಕ u8 *a = sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_Db_ಪುಟ);
    ಸ್ಮರಣೆ_ನಕಲು(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(p_Db_ಪುಟ);
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stat_ಮುಂದಿನ(sqlite3_vtab_cursor *pCursor){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ nPayload;
  ಅಕ್ಷರ *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  Stat_ಕೋಷ್ಟಕ *pTab = (Stat_ಕೋಷ್ಟಕ *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->z_ಮಾರ್ಗ);
  pCsr->z_ಮಾರ್ಗ = 0;

stat_ಮುಂದಿನ_Restart:
  ಆದರೆ( pCsr->i_ಪುಟ<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_ಹೆಜ್ಜೆ(pCsr->pStmt);
    ಆದರೆ( rc==SQLITE_ಅಡ್ಡಸಾಲು ){
      ಪೂರ್ಣಾಂಕ n_ಪುಟ;
      u32 i_ಬೇರು = (u32)sqlite3_ಸ್ತಂಭ_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &n_ಪುಟ);
      ಆದರೆ( n_ಪುಟ==0 ){
        pCsr->isEof = 1;
        ಹಿಂತಿರುಗಿ sqlite3_reset(pCsr->pStmt);
      }
      rc = stat_Get_ಪುಟ(pBt, i_ಬೇರು, &pCsr->a_ಪುಟ[0]);
      pCsr->a_ಪುಟ[0].iPgno = i_ಬೇರು;
      pCsr->a_ಪುಟ[0].iCell = 0;
      ಆದರೆ( !pCsr->isAgg ){
        pCsr->a_ಪುಟ[0].z_ಮಾರ್ಗ = z = sqlite3_mprintf("/");
        ಆದರೆ( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->i_ಪುಟ = 0;
      pCsr->n_ಪುಟ = 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pCsr->isEof = 1;
      ಹಿಂತಿರುಗಿ sqlite3_reset(pCsr->pStmt);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Continue analyzing the btree previously started */
    Stat_ಪುಟ *p = &pCsr->a_ಪುಟ[pCsr->i_ಪುಟ];
    ಆದರೆ( !pCsr->isAgg ) statResetCounts(pCsr);
    ಆಗ( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      ಆಗ( pCell->iOvfl<pCell->nOvfl ){
        ಪೂರ್ಣಾಂಕ nUsable;
        ಪೂರ್ಣಾಂಕ iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->n_ಪುಟ++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        stat_ಗಾತ್ರ_And_ಸ್ಥಾನಾಂತರ(pCsr);
        ಆದರೆ( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }ಇಲ್ಲದಿದ್ದರೆ{
          pCsr->nPayload += pCell->n_ಕೊನೆಯ_Ovfl;
          pCsr->n_ಬಳಸದ += nUsable - 4 - pCell->n_ಕೊನೆಯ_Ovfl;
        }
        pCell->iOvfl++;
        ಆದರೆ( !pCsr->isAgg ){
          pCsr->zName = (ಅಕ್ಷರ *)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->z_ಮಾರ್ಗ = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->z_ಮಾರ್ಗ, p->iCell, iOvfl
          );
          ಹಿಂತಿರುಗಿ z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      ಆದರೆ( p->i_ಬಲ_ಮಗು_Pg ) ನಿಲ್ಲಿಸು;
      p->iCell++;
    }

    ಆದರೆ( !p->i_ಬಲ_ಮಗು_Pg || p->iCell>p->nCell ){
      stat_Clear_ಪುಟ(p);
      pCsr->i_ಪುಟ--;
      ಆದರೆ( pCsr->isAgg && pCsr->i_ಪುಟ<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        ಹಿಂತಿರುಗಿ SQLITE_OK;
      }
      ಹೋಗು stat_ಮುಂದಿನ_Restart; /* Tail recursion */
    }
    pCsr->i_ಪುಟ++;
    ಆದರೆ( pCsr->i_ಪುಟ>=Array_ಗಾತ್ರ(pCsr->a_ಪುಟ) ){
      statResetCsr(pCsr);
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
    }
    ಪ್ರತಿಜ್ಞಾ( p==&pCsr->a_ಪುಟ[pCsr->i_ಪುಟ-1] );

    ಆದರೆ( p->iCell==p->nCell ){
      p[1].iPgno = p->i_ಬಲ_ಮಗು_Pg;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p[1].iPgno = p->aCell[p->iCell].i_ಮಗು_Pg;
    }
    rc = stat_Get_ಪುಟ(pBt, p[1].iPgno, &p[1]);
    pCsr->n_ಪುಟ++;
    p[1].iCell = 0;
    ಆದರೆ( !pCsr->isAgg ){
      p[1].z_ಮಾರ್ಗ = z = sqlite3_mprintf("%s%.3x/", p->z_ಮಾರ್ಗ, p->iCell);
      ಆದರೆ( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  ಆದರೆ( rc==SQLITE_OK ){
    ಪೂರ್ಣಾಂಕ i;
    Stat_ಪುಟ *p = &pCsr->a_ಪುಟ[pCsr->i_ಪುಟ];
    pCsr->zName = (ಅಕ್ಷರ *)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = stat_Decode_ಪುಟ(pBt, p);
    ಆದರೆ( rc==SQLITE_OK ){
      stat_ಗಾತ್ರ_And_ಸ್ಥಾನಾಂತರ(pCsr);

      ಆಯ್ಕೆ( p->flags ){
        ಸ್ಥಿತಿ 0x05:             /* table internal */
        ಸ್ಥಿತಿ 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ 0x0D:             /* table leaf */
        ಸ್ಥಿತಿ 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
          pCsr->zPagetype = "corrupted";
          ನಿಲ್ಲಿಸು;
      }
      pCsr->nCell += p->nCell;
      pCsr->n_ಬಳಸದ += p->n_ಬಳಸದ;
      ಆದರೆ( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      ಆದರೆ( !pCsr->isAgg ){
        pCsr->z_ಮಾರ್ಗ = z = sqlite3_mprintf("%s", p->z_ಮಾರ್ಗ);
        ಆದರೆ( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      ಗಾಗಿ(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      ಆದರೆ( pCsr->isAgg ) ಹೋಗು stat_ಮುಂದಿನ_Restart;
    }
  }

  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  ಹಿಂತಿರುಗಿ pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ statFilter(
  sqlite3_vtab_cursor *pCursor, 
  ಪೂರ್ಣಾಂಕ idx_ಅಂಕೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *idxStr,
  ಪೂರ್ಣಾಂಕ argc, sqlite3_ಮೌಲ್ಯ **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  Stat_ಕೋಷ್ಟಕ *pTab = (Stat_ಕೋಷ್ಟಕ*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  ಅಕ್ಷರ *zSql;             /* String value of pSql */
  ಪೂರ್ಣಾಂಕ iArg = 0;           /* Count of argv[] parameters used so far */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;     /* Result of this operation */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName = 0;  /* Only provide analysis of this table */
  (ಶೂನ್ಯ)argc;
  (ಶೂನ್ಯ)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  ಆದರೆ( idx_ಅಂಕೆ & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDbase = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[iArg++]);
    pCsr->iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(pTab->db, zDbase);
    ಆದರೆ( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    pCsr->iDb = pTab->iDb;
  }
  ಆದರೆ( idx_ಅಂಕೆ & 0x02 ){
    /* name=? constraint is present */
    zName = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)sqlite3_ಮೌಲ್ಯ_ಪಠ್ಯ(argv[iArg++]);
  }
  ಆದರೆ( idx_ಅಂಕೆ & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_ಮೌಲ್ಯ_double(argv[iArg++])!=0.0;
  }ಇಲ್ಲದಿದ್ದರೆ{
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
  ಆದರೆ( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  ಆದರೆ( idx_ಅಂಕೆ & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_ಮುಕ್ತಾಯ(pSql);
  ಆದರೆ( zSql==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  ಆದರೆ( rc==SQLITE_OK ){
    pCsr->i_ಪುಟ = -1;
    rc = stat_ಮುಂದಿನ(pCursor);
  }
  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stat_ಸ್ತಂಭ(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಂಕ i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  ಆಯ್ಕೆ( i ){
    ಸ್ಥಿತಿ 0:            /* name */
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 1:            /*ಮಾರ್ಗ*/
      ಆದರೆ( !pCsr->isAgg ){
        sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pCsr->z_ಮಾರ್ಗ, -1, SQLITE_TRANSIENT);
      }
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 2:            /* pageno */
      ಆದರೆ( pCsr->isAgg ){
        sqlite3_ಫಲಿತಾಂಶ_int64(ctx, pCsr->n_ಪುಟ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಫಲಿತಾಂಶ_int64(ctx, pCsr->iPageno);
      }
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 3:            /* pagetype */
      ಆದರೆ( !pCsr->isAgg ){
        sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 4:            /* ncell */
      sqlite3_ಫಲಿತಾಂಶ_int64(ctx, pCsr->nCell);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 5:            /* payload */
      sqlite3_ಫಲಿತಾಂಶ_int64(ctx, pCsr->nPayload);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 6:            /*ಬಳಸದ*/
      sqlite3_ಫಲಿತಾಂಶ_int64(ctx, pCsr->n_ಬಳಸದ);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 7:            /* mx_payload */
      sqlite3_ಫಲಿತಾಂಶ_int64(ctx, pCsr->nMxPayload);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 8:            /* pgoffset */
      ಆದರೆ( !pCsr->isAgg ){
        sqlite3_ಫಲಿತಾಂಶ_int64(ctx, pCsr->i_ಸ್ಥಾನಾಂತರ);
      }
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 9:            /* pgsize */
      sqlite3_ಫಲಿತಾಂಶ_int64(ctx, pCsr->sz_ಪುಟ);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      ಪೂರ್ಣಾಂಕ iDb = pCsr->iDb;
      sqlite3_ಫಲಿತಾಂಶ_ಪಠ್ಯ(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {         /* aggregate */
      sqlite3_ಫಲಿತಾಂಶ_int(ctx, pCsr->isAgg);
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
ಪೂರ್ಣಾಂಕ sqlite3_Dbstat_ರಿಜಿಸ್ಟರ್(sqlite3 *db){
  ಸ್ಥಿರ sqlite3_module dbstat_module = {
    0,                            /* iVersion */
    statConnect,                  /* xCreate */
    statConnect,                  /* xConnect */
    stat_ಉತ್ತಮ_ಸೂಚ್ಯಂಕ,                /* xBestIndex */
    statDisconnect,               /* xDisconnect */
    statDisconnect,               /* xDestroy */
    statOpen,                     /* xOpen - open a cursor */
    statClose,                    /* xClose - close a cursor */
    statFilter,                   /* xFilter - configure scan constraints */
    stat_ಮುಂದಿನ,                     /* xNext - advance a cursor */
    statEof,                      /* xEof - check for end of scan */
    stat_ಸ್ತಂಭ,                   /* xColumn - read data */
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
  ಹಿಂತಿರುಗಿ sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#ಅಥವಾಆದರೆ defined(SQLITE_ENABLE_DBSTAT_VTAB)
ಪೂರ್ಣಾಂಕ sqlite3_Dbstat_ರಿಜಿಸ್ಟರ್(sqlite3 *db){ ಹಿಂತಿರುಗಿ SQLITE_OK; }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_DBSTAT_VTAB */
