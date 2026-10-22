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

#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"   /* Requires access to internal data structures */
#ਜੇ (defined(SQLITE_ENABLE_DBSTAT_VTAB) || defined(SQLITE_TEST)) \
    && !defined(SQLITE_OMIT_VIRTUALTABLE)

/*
** The pager and btree modules arrange objects in memory so that there are
** always approximately 200 bytes of addressable memory following each page
** buffer. This way small buffer overreads caused by corrupt database pages
** do not cause undefined behaviour. This module pads each page buffer
** by the following number of bytes for the same purpose.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBSTAT_ਪੰਨਾ_PADDING_ਬਾਈਟਾਂ 256

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
ਸਥਿਰ ਅਚਲ ਅੱਖਰ zDbstatSchema[] = 
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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Stat_ਸਾਰਣੀ Stat_ਸਾਰਣੀ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ StatCursor StatCursor;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Stat_ਪੰਨਾ Stat_ਪੰਨਾ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ StatCell StatCell;

/* Size information for a single cell within a btree page */
ਬਣਤਰ StatCell {
  ਪੂਰਨ nLocal;                     /* Bytes of local payload */
  u32 i_ਬੱਚਾ_Pg;                   /* Child node (or 0 if this is a leaf) */
  ਪੂਰਨ nOvfl;                      /* Entries in aOvfl[] */
  u32 *aOvfl;                     /* Array of overflow page numbers */
  ਪੂਰਨ n_ਆਖਰੀ_Ovfl;                  /* Bytes of payload on final overflow page */
  ਪੂਰਨ iOvfl;                      /* Iterates through aOvfl[] */
};

/* Size information for a single btree page */
ਬਣਤਰ Stat_ਪੰਨਾ {
  u32 iPgno;                      /* Page number */
  u8 *aPg;                        /* Page buffer from sqlite3_malloc() */
  ਪੂਰਨ iCell;                      /* Current cell */
  ਅੱਖਰ *z_ਰਸਤਾ;                    /* Path to this page */

  /* Variables populated by statDecodePage(): */
  u8 flags;                       /* Copy of flags byte */
  ਪੂਰਨ nCell;                      /* Number of cells on page */
  ਪੂਰਨ n_ਅਣਵਰਤਿਆ;                    /* Number of unused bytes on page */
  StatCell *aCell;                /* Array of parsed cells */
  u32 i_ਸੱਜਾ_ਬੱਚਾ_Pg;              /* Right-child page number (or 0) */
  ਪੂਰਨ nMxPayload;                 /* Largest payload of any cell on the page */
};

/* The cursor for scanning the dbstat virtual table */
ਬਣਤਰ StatCursor {
  sqlite3_vtab_cursor ਅਧਾਰ;       /* base class.  MUST BE FIRST! */
  sqlite3_stmt *pStmt;            /* Iterates through set of root pages */
  u8 isEof;                       /* After pStmt has returned SQLITE_DONE */
  u8 isAgg;                       /* Aggregate results for each table */
  ਪੂਰਨ iDb;                        /* Schema used for this query */

  Stat_ਪੰਨਾ a_ਪੰਨਾ[32];             /* Pages in path to current page */
  ਪੂਰਨ i_ਪੰਨਾ;                      /* Current entry in aPage[] */

  /* Values to return. */
  u32 iPageno;                    /* Value of 'pageno' column */
  ਅੱਖਰ *zName;                    /* Value of 'name' column */
  ਅੱਖਰ *z_ਰਸਤਾ;                    /* Value of 'path' column */
  ਅੱਖਰ *zPagetype;                /* Value of 'pagetype' column */
  ਪੂਰਨ n_ਪੰਨਾ;                      /* Number of pages in current btree */
  ਪੂਰਨ nCell;                      /* Value of 'ncell' column */
  ਪੂਰਨ nMxPayload;                 /* Value of 'mx_payload' column */
  i64 n_ਅਣਵਰਤਿਆ;                    /* Value of 'unused' column */
  i64 nPayload;                   /* Value of 'payload' column */
  i64 i_ਵਿੱਥ;                    /* Value of 'pgOffset' column */
  i64 sz_ਪੰਨਾ;                     /* Value of 'pgSize' column */
};

/* An instance of the DBSTAT virtual table */
ਬਣਤਰ Stat_ਸਾਰਣੀ {
  sqlite3_vtab ਅਧਾਰ;              /* base class.  MUST BE FIRST! */
  sqlite3 *db;                    /* Database connection that owns this vtab */
  ਪੂਰਨ iDb;                        /* Index of database to analyze */
};

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ get2byte
# define get2byte(x)   ((x)[0]<<8 | (x)[1])
#ਜੇ_ਅੰਤ

/*
** Connect to or create a new DBSTAT virtual table.
*/
ਸਥਿਰ ਪੂਰਨ statConnect(
  sqlite3 *db,
  ਖਾਲੀ *pAux,
  ਪੂਰਨ argc, ਅਚਲ ਅੱਖਰ *ਅਚਲ*argv,
  sqlite3_vtab **ppVtab,
  ਅੱਖਰ **pzErr
){
  Stat_ਸਾਰਣੀ *pTab = 0;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ iDb;
  (ਖਾਲੀ)pAux;

  ਜੇ( argc>=4 ){
    Token nm;
    sqlite3TokenInit(&nm, (ਅੱਖਰ*)argv[3]);
    iDb = sqlite3_ਲੱਭੋ_Db(db, &nm);
    ਜੇ( iDb<0 ){
      *pzErr = sqlite3_mprintf("no such database: %s", argv[3]);
      ਵਾਪਸ SQLITE_ERROR;
    }
  }ਨਹੀਂਤਾਂ{
    iDb = 0;
  }
  sqlite3_vtab_config(db, SQLITE_VTAB_DIRECTONLY);
  rc = sqlite3_declare_vtab(db, zDbstatSchema);
  ਜੇ( rc==SQLITE_OK ){
    pTab = (Stat_ਸਾਰਣੀ *)sqlite3_malloc64(ਆਕਾਰ(Stat_ਸਾਰਣੀ));
    ਜੇ( pTab==0 ) rc = SQLITE_NOMEM_BKPT;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || pTab==0 );
  ਜੇ( rc==SQLITE_OK ){
    memset(pTab, 0, ਆਕਾਰ(Stat_ਸਾਰਣੀ));
    pTab->db = db;
    pTab->iDb = iDb;
  }

  *ppVtab = (sqlite3_vtab*)pTab;
  ਵਾਪਸ rc;
}

/*
** Disconnect from or destroy the DBSTAT virtual table.
*/
ਸਥਿਰ ਪੂਰਨ statDisconnect(sqlite3_vtab *pVtab){
  sqlite3_free(pVtab);
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ stat_ਉੱਤਮ_ਸੂਚਕਾਂਕ(sqlite3_vtab *tab, sqlite3_ਸੂਚਕਾਂਕ_info *pIdxInfo){
  ਪੂਰਨ i;
  ਪੂਰਨ iSchema = -1;
  ਪੂਰਨ iName = -1;
  ਪੂਰਨ iAgg = -1;
  (ਖਾਲੀ)tab;

  /* Look for a valid schema=? constraint.  If found, change the idxNum to
  ** 1 and request the value of that constraint be sent to xFilter.  And
  ** lower the cost estimate to encourage the constrained version to be
  ** used.
  */
  ਲਈ(i=0; i<pIdxInfo->nConstraint; i++){
    ਜੇ( pIdxInfo->aConstraint[i].op!=SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_EQ ) ਜਾਰੀ;
    ਜੇ( pIdxInfo->aConstraint[i].usable==0 ){
      /* Force DBSTAT table should always be the right-most table in a join */
      ਵਾਪਸ SQLITE_CONSTRAINT;
    }
    ਚੋਣ( pIdxInfo->aConstraint[i].i_ਕਾਲਮ ){
      ਹਾਲਤ 0: {    /* name */
        iName = i;
        ਰੋਕੋ;
      }
      ਹਾਲਤ 10: {   /* schema */
        iSchema = i;
        ਰੋਕੋ;
      }
      ਹਾਲਤ 11: {   /* aggregate */
        iAgg = i;
        ਰੋਕੋ;
      }
    }
  }
  i = 0;
  ਜੇ( iSchema>=0 ){
    pIdxInfo->aConstraintUsage[iSchema].argv_ਸੂਚਕਾਂਕ = ++i;
    pIdxInfo->aConstraintUsage[iSchema].omit = 1;
    pIdxInfo->idx_ਅੰਕ |= 0x01;
  }
  ਜੇ( iName>=0 ){
    pIdxInfo->aConstraintUsage[iName].argv_ਸੂਚਕਾਂਕ = ++i;
    pIdxInfo->idx_ਅੰਕ |= 0x02;
  }
  ਜੇ( iAgg>=0 ){
    pIdxInfo->aConstraintUsage[iAgg].argv_ਸੂਚਕਾਂਕ = ++i;
    pIdxInfo->idx_ਅੰਕ |= 0x04;
  }
  pIdxInfo->estimated_ਲਾਗਤ = 1.0;

  /* Records are always returned in ascending order of (name, path). 
  ** If this will satisfy the client, set the orderByConsumed flag so that 
  ** SQLite does not do an external sort.
  */
  ਜੇ( ( pIdxInfo->n_ਕ੍ਰਮ_By==1
     && pIdxInfo->a_ਕ੍ਰਮ_By[0].i_ਕਾਲਮ==0
     && pIdxInfo->a_ਕ੍ਰਮ_By[0].desc==0
     ) ||
      ( pIdxInfo->n_ਕ੍ਰਮ_By==2
     && pIdxInfo->a_ਕ੍ਰਮ_By[0].i_ਕਾਲਮ==0
     && pIdxInfo->a_ਕ੍ਰਮ_By[0].desc==0
     && pIdxInfo->a_ਕ੍ਰਮ_By[1].i_ਕਾਲਮ==1
     && pIdxInfo->a_ਕ੍ਰਮ_By[1].desc==0
     )
  ){
    pIdxInfo->ਕ੍ਰਮ_By_Consumed = 1;
    pIdxInfo->idx_ਅੰਕ |= 0x08;
  }
  pIdxInfo->idxFlags |= SQLITE_ਸੂਚਕਾਂਕ_ਜਾਂਚ_HEX;

  ਵਾਪਸ SQLITE_OK;
}

/*
** Open a new DBSTAT cursor.
*/
ਸਥਿਰ ਪੂਰਨ statOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor){
  Stat_ਸਾਰਣੀ *pTab = (Stat_ਸਾਰਣੀ *)pVTab;
  StatCursor *pCsr;

  pCsr = (StatCursor *)sqlite3_malloc64(ਆਕਾਰ(StatCursor));
  ਜੇ( pCsr==0 ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }ਨਹੀਂਤਾਂ{
    memset(pCsr, 0, ਆਕਾਰ(StatCursor));
    pCsr->ਅਧਾਰ.pVtab = pVTab;
    pCsr->iDb = pTab->iDb;
  }

  *ppCursor = (sqlite3_vtab_cursor *)pCsr;
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਖਾਲੀ statClearCells(Stat_ਪੰਨਾ *p){
  ਪੂਰਨ i;
  ਜੇ( p->aCell ){
    ਲਈ(i=0; i<p->nCell; i++){
      sqlite3_free(p->aCell[i].aOvfl);
    }
    sqlite3_free(p->aCell);
  }
  p->nCell = 0;
  p->aCell = 0;
}

ਸਥਿਰ ਖਾਲੀ stat_Clear_ਪੰਨਾ(Stat_ਪੰਨਾ *p){
  u8 *aPg = p->aPg;
  statClearCells(p);
  sqlite3_free(p->z_ਰਸਤਾ);
  memset(p, 0, ਆਕਾਰ(Stat_ਪੰਨਾ));
  p->aPg = aPg;
}

ਸਥਿਰ ਖਾਲੀ statResetCsr(StatCursor *pCsr){
  ਪੂਰਨ i;
  /* In some circumstances, specifically if an OOM has occurred, the call
  ** to sqlite3_reset() may cause the pager to be reset (emptied). It is
  ** important that statClearPage() is called to free any page refs before
  ** this happens. dbsqlfuzz 9ed3e4e3816219d3509d711636c38542bf3f40b1. */
  ਲਈ(i=0; i<Array_ਮਾਪ(pCsr->a_ਪੰਨਾ); i++){
    stat_Clear_ਪੰਨਾ(&pCsr->a_ਪੰਨਾ[i]);
    sqlite3_free(pCsr->a_ਪੰਨਾ[i].aPg);
    pCsr->a_ਪੰਨਾ[i].aPg = 0;
  }
  sqlite3_reset(pCsr->pStmt);
  pCsr->i_ਪੰਨਾ = 0;
  sqlite3_free(pCsr->z_ਰਸਤਾ);
  pCsr->z_ਰਸਤਾ = 0;
  pCsr->isEof = 0;
}

/* Resize the space-used counters inside of the cursor */
ਸਥਿਰ ਖਾਲੀ statResetCounts(StatCursor *pCsr){
  pCsr->nCell = 0;
  pCsr->nMxPayload = 0;
  pCsr->n_ਅਣਵਰਤਿਆ = 0;
  pCsr->nPayload = 0;
  pCsr->sz_ਪੰਨਾ = 0;
  pCsr->n_ਪੰਨਾ = 0;
}

/*
** Close a DBSTAT cursor.
*/
ਸਥਿਰ ਪੂਰਨ statClose(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr);
  ਵਾਪਸ SQLITE_OK;
}

/*
** For a single cell on a btree page, compute the number of bytes of
** content (payload) stored on that page.  That is to say, compute the
** number of bytes of content not found on overflow pages.
*/
ਸਥਿਰ ਪੂਰਨ getLocalPayload(
  ਪੂਰਨ nUsable,                    /* Usable bytes per page */
  u8 flags,                       /* Page flags */
  ਪੂਰਨ n_ਕੁੱਲ                      /* Total record (payload) size */
){
  ਪੂਰਨ nLocal;
  ਪੂਰਨ nMinLocal;
  ਪੂਰਨ nMaxLocal;
 
  ਜੇ( flags==0x0D ){              /* Table leaf node */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = nUsable - 35;
  }ਨਹੀਂਤਾਂ{                          /* Index interior and leaf nodes */
    nMinLocal = (nUsable - 12) * 32 / 255 - 23;
    nMaxLocal = (nUsable - 12) * 64 / 255 - 23;
  }

  nLocal = nMinLocal + (n_ਕੁੱਲ - nMinLocal) % (nUsable - 4);
  ਜੇ( nLocal>nMaxLocal ) nLocal = nMinLocal;
  ਵਾਪਸ nLocal;
}

/* Populate the StatPage object with information about the all
** cells found on the page currently under analysis.
*/
ਸਥਿਰ ਪੂਰਨ stat_Decode_ਪੰਨਾ(Btree *pBt, Stat_ਪੰਨਾ *p){
  ਪੂਰਨ n_ਅਣਵਰਤਿਆ;
  ਪੂਰਨ iOff;
  ਪੂਰਨ nHdr;
  ਪੂਰਨ is_ਪੱਤੀ;
  ਪੂਰਨ sz_ਪੰਨਾ;

  u8 *a_ਅੰਕੜੇ = p->aPg;
  u8 *aHdr = &a_ਅੰਕੜੇ[p->iPgno==1 ? 100 : 0];

  p->flags = aHdr[0];
  ਜੇ( p->flags==0x0A || p->flags==0x0D ){
    is_ਪੱਤੀ = 1;
    nHdr = 8;
  }ਨਹੀਂਤਾਂ ਜੇ( p->flags==0x05 || p->flags==0x02 ){
    is_ਪੱਤੀ = 0;
    nHdr = 12;
  }ਨਹੀਂਤਾਂ{
    ਜਾਓ stat_ਪੰਨਾ_Is_Corrupt;
  }
  ਜੇ( p->iPgno==1 ) nHdr += 100;
  p->nCell = get2byte(&aHdr[3]);
  p->nMxPayload = 0;
  sz_ਪੰਨਾ = sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(pBt);

  n_ਅਣਵਰਤਿਆ = get2byte(&aHdr[5]) - nHdr - 2*p->nCell;
  n_ਅਣਵਰਤਿਆ += (ਪੂਰਨ)aHdr[7];
  iOff = get2byte(&aHdr[1]);
  ਜਦੋਂ( iOff ){
    ਪੂਰਨ i_ਅਗਲਾ;
    ਜੇ( iOff>=sz_ਪੰਨਾ ) ਜਾਓ stat_ਪੰਨਾ_Is_Corrupt;
    n_ਅਣਵਰਤਿਆ += get2byte(&a_ਅੰਕੜੇ[iOff+2]);
    i_ਅਗਲਾ = get2byte(&a_ਅੰਕੜੇ[iOff]);
    ਜੇ( i_ਅਗਲਾ<iOff+4 && i_ਅਗਲਾ>0 ) ਜਾਓ stat_ਪੰਨਾ_Is_Corrupt;
    iOff = i_ਅਗਲਾ;
  }
  p->n_ਅਣਵਰਤਿਆ = n_ਅਣਵਰਤਿਆ;
  p->i_ਸੱਜਾ_ਬੱਚਾ_Pg = is_ਪੱਤੀ ? 0 : sqlite3Get4byte(&aHdr[8]);

  ਜੇ( p->nCell ){
    ਪੂਰਨ i;                        /* Used to iterate through cells */
    ਪੂਰਨ nUsable;                  /* Usable bytes per page */

    sqlite3BtreeEnter(pBt);
    nUsable = sz_ਪੰਨਾ - sqlite3BtreeGetReserveNoMutex(pBt);
    sqlite3BtreeLeave(pBt);
    p->aCell = sqlite3_malloc64((p->nCell+1) * ਆਕਾਰ(StatCell));
    ਜੇ( p->aCell==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
    memset(p->aCell, 0, (p->nCell+1) * ਆਕਾਰ(StatCell));

    ਲਈ(i=0; i<p->nCell; i++){
      StatCell *pCell = &p->aCell[i];

      iOff = get2byte(&a_ਅੰਕੜੇ[nHdr+i*2]);
      ਜੇ( iOff<nHdr || iOff>=sz_ਪੰਨਾ ) ਜਾਓ stat_ਪੰਨਾ_Is_Corrupt;
      ਜੇ( !is_ਪੱਤੀ ){
        pCell->i_ਬੱਚਾ_Pg = sqlite3Get4byte(&a_ਅੰਕੜੇ[iOff]);
        iOff += 4;
      }
      ਜੇ( p->flags==0x05 ){
        /* A table interior node. nPayload==0. */
      }ਨਹੀਂਤਾਂ{
        u32 nPayload;             /* Bytes of payload total (local+overflow) */
        ਪੂਰਨ nLocal;               /* Bytes of payload stored locally */
        iOff += getVarint32(&a_ਅੰਕੜੇ[iOff], nPayload);
        ਜੇ( p->flags==0x0D ){
          u64 dummy;
          iOff += sqlite3GetVarint(&a_ਅੰਕੜੇ[iOff], &dummy);
        }
        ਜੇ( nPayload>(u32)p->nMxPayload ) p->nMxPayload = nPayload;
        nLocal = getLocalPayload(nUsable, p->flags, nPayload);
        ਜੇ( nLocal<0 ) ਜਾਓ stat_ਪੰਨਾ_Is_Corrupt;
        pCell->nLocal = nLocal;
        ਪ੍ਰਤਿਜ੍ਞਾ( nPayload>=(u32)nLocal );
        ਪ੍ਰਤਿਜ੍ਞਾ( nLocal<=(nUsable-35) );
        ਜੇ( nPayload>(u32)nLocal ){
          ਪੂਰਨ j;
          ਪੂਰਨ nOvfl = ((nPayload - nLocal) + nUsable-4 - 1) / (nUsable - 4);
          ਜੇ( iOff+nLocal+4>nUsable || nPayload>0x7fffffff ){
            ਜਾਓ stat_ਪੰਨਾ_Is_Corrupt;
          }
          pCell->n_ਆਖਰੀ_Ovfl = (nPayload-nLocal) - (nOvfl-1) * (nUsable-4);
          pCell->nOvfl = nOvfl;
          pCell->aOvfl = sqlite3_malloc64(ਆਕਾਰ(u32)*nOvfl);
          ਜੇ( pCell->aOvfl==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
          pCell->aOvfl[0] = sqlite3Get4byte(&a_ਅੰਕੜੇ[iOff+nLocal]);
          ਲਈ(j=1; j<nOvfl; j++){
            ਪੂਰਨ rc;
            u32 iPrev = pCell->aOvfl[j-1];
            Db_ਪੰਨਾ *pPg = 0;
            rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPrev, &pPg, 0);
            ਜੇ( rc!=SQLITE_OK ){
              ਪ੍ਰਤਿਜ੍ਞਾ( pPg==0 );
              ਵਾਪਸ rc;
            } 
            pCell->aOvfl[j] = sqlite3Get4byte(sqlite3_Pager_Get_ਅੰਕੜੇ(pPg));
            sqlite3PagerUnref(pPg);
          }
        }
      }
    }
  }

  ਵਾਪਸ SQLITE_OK;

stat_ਪੰਨਾ_Is_Corrupt:
  p->flags = 0;
  statClearCells(p);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Populate the pCsr->iOffset and pCsr->szPage member variables. Based on
** the current value of pCsr->iPageno.
*/
ਸਥਿਰ ਖਾਲੀ stat_ਮਾਪ_And_ਵਿੱਥ(StatCursor *pCsr){
  Stat_ਸਾਰਣੀ *pTab = (Stat_ਸਾਰਣੀ *)((sqlite3_vtab_cursor *)pCsr)->pVtab;
  Btree *pBt = pTab->db->aDb[pTab->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);
  sqlite3_file *fd;
  sqlite3_int64 x[2];

  /* If connected to a ZIPVFS backend, find the page size and
  ** offset from ZIPVFS.
  */
  fd = sqlite3PagerFile(pPager);
  x[0] = pCsr->iPageno;
  ਜੇ( sqlite3OsFileControl(fd, 230440, &x)==SQLITE_OK ){
    pCsr->i_ਵਿੱਥ = x[0];
    pCsr->sz_ਪੰਨਾ += x[1];
  }ਨਹੀਂਤਾਂ{
    /* Not ZIPVFS: The default page size and offset */
    pCsr->sz_ਪੰਨਾ += sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(pBt);
    pCsr->i_ਵਿੱਥ = (i64)pCsr->sz_ਪੰਨਾ * (pCsr->iPageno - 1);
  }
}

/*
** Load a copy of the page data for page iPg into the buffer belonging
** to page object pPg. Allocate the buffer if necessary. Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
ਸਥਿਰ ਪੂਰਨ stat_Get_ਪੰਨਾ(
  Btree *pBt,                     /* Load page from this b-tree */
  u32 iPg,                        /* Page number to load */
  Stat_ਪੰਨਾ *pPg                   /* Load page into this object */
){
  ਪੂਰਨ pgsz = sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(pBt);
  Db_ਪੰਨਾ *p_Db_ਪੰਨਾ = 0;
  ਪੂਰਨ rc;

  ਜੇ( pPg->aPg==0 ){
    pPg->aPg = (u8*)sqlite3_malloc(pgsz + DBSTAT_ਪੰਨਾ_PADDING_ਬਾਈਟਾਂ);
    ਜੇ( pPg->aPg==0 ){
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
    memset(&pPg->aPg[pgsz], 0, DBSTAT_ਪੰਨਾ_PADDING_ਬਾਈਟਾਂ);
  }

  rc = sqlite3PagerGet(sqlite3BtreePager(pBt), iPg, &p_Db_ਪੰਨਾ, 0);
  ਜੇ( rc==SQLITE_OK ){
    ਅਚਲ u8 *a = sqlite3_Pager_Get_ਅੰਕੜੇ(p_Db_ਪੰਨਾ);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pPg->aPg, a, pgsz);
    sqlite3PagerUnref(p_Db_ਪੰਨਾ);
  }

  ਵਾਪਸ rc;
}

/*
** Move a DBSTAT cursor to the next entry.  Normally, the next
** entry will be the next page, but in aggregated mode (pCsr->isAgg!=0),
** the next entry is the next btree.
*/
ਸਥਿਰ ਪੂਰਨ stat_ਅਗਲਾ(sqlite3_vtab_cursor *pCursor){
  ਪੂਰਨ rc;
  ਪੂਰਨ nPayload;
  ਅੱਖਰ *z;
  StatCursor *pCsr = (StatCursor *)pCursor;
  Stat_ਸਾਰਣੀ *pTab = (Stat_ਸਾਰਣੀ *)pCursor->pVtab;
  Btree *pBt = pTab->db->aDb[pCsr->iDb].pBt;
  Pager *pPager = sqlite3BtreePager(pBt);

  sqlite3_free(pCsr->z_ਰਸਤਾ);
  pCsr->z_ਰਸਤਾ = 0;

stat_ਅਗਲਾ_Restart:
  ਜੇ( pCsr->i_ਪੰਨਾ<0 ){
    /* Start measuring space on the next btree */
    statResetCounts(pCsr);
    rc = sqlite3_ਕਦਮ(pCsr->pStmt);
    ਜੇ( rc==SQLITE_ਪੰਕਤੀ ){
      ਪੂਰਨ n_ਪੰਨਾ;
      u32 i_ਜੜ੍ਹ = (u32)sqlite3_ਕਾਲਮ_int64(pCsr->pStmt, 1);
      sqlite3PagerPagecount(pPager, &n_ਪੰਨਾ);
      ਜੇ( n_ਪੰਨਾ==0 ){
        pCsr->isEof = 1;
        ਵਾਪਸ sqlite3_reset(pCsr->pStmt);
      }
      rc = stat_Get_ਪੰਨਾ(pBt, i_ਜੜ੍ਹ, &pCsr->a_ਪੰਨਾ[0]);
      pCsr->a_ਪੰਨਾ[0].iPgno = i_ਜੜ੍ਹ;
      pCsr->a_ਪੰਨਾ[0].iCell = 0;
      ਜੇ( !pCsr->isAgg ){
        pCsr->a_ਪੰਨਾ[0].z_ਰਸਤਾ = z = sqlite3_mprintf("/");
        ਜੇ( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      pCsr->i_ਪੰਨਾ = 0;
      pCsr->n_ਪੰਨਾ = 1;
    }ਨਹੀਂਤਾਂ{
      pCsr->isEof = 1;
      ਵਾਪਸ sqlite3_reset(pCsr->pStmt);
    }
  }ਨਹੀਂਤਾਂ{
    /* Continue analyzing the btree previously started */
    Stat_ਪੰਨਾ *p = &pCsr->a_ਪੰਨਾ[pCsr->i_ਪੰਨਾ];
    ਜੇ( !pCsr->isAgg ) statResetCounts(pCsr);
    ਜਦੋਂ( p->iCell<p->nCell ){
      StatCell *pCell = &p->aCell[p->iCell];
      ਜਦੋਂ( pCell->iOvfl<pCell->nOvfl ){
        ਪੂਰਨ nUsable;
        ਪੂਰਨ iOvfl = pCell->iOvfl;
        sqlite3BtreeEnter(pBt);
        nUsable = sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(pBt) - 
                        sqlite3BtreeGetReserveNoMutex(pBt);
        sqlite3BtreeLeave(pBt);
        pCsr->n_ਪੰਨਾ++;
        pCsr->iPageno = pCell->aOvfl[iOvfl];
        stat_ਮਾਪ_And_ਵਿੱਥ(pCsr);
        ਜੇ( pCell->iOvfl<pCell->nOvfl-1 ){
          pCsr->nPayload += nUsable - 4;
        }ਨਹੀਂਤਾਂ{
          pCsr->nPayload += pCell->n_ਆਖਰੀ_Ovfl;
          pCsr->n_ਅਣਵਰਤਿਆ += nUsable - 4 - pCell->n_ਆਖਰੀ_Ovfl;
        }
        pCell->iOvfl++;
        ਜੇ( !pCsr->isAgg ){
          pCsr->zName = (ਅੱਖਰ *)sqlite3_ਕਾਲਮ_ਪਾਠ(pCsr->pStmt, 0);
          pCsr->zPagetype = "overflow";
          pCsr->z_ਰਸਤਾ = z = sqlite3_mprintf(
              "%s%.3x+%.6x", p->z_ਰਸਤਾ, p->iCell, iOvfl
          );
          ਵਾਪਸ z==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
        }
      }
      ਜੇ( p->i_ਸੱਜਾ_ਬੱਚਾ_Pg ) ਰੋਕੋ;
      p->iCell++;
    }

    ਜੇ( !p->i_ਸੱਜਾ_ਬੱਚਾ_Pg || p->iCell>p->nCell ){
      stat_Clear_ਪੰਨਾ(p);
      pCsr->i_ਪੰਨਾ--;
      ਜੇ( pCsr->isAgg && pCsr->i_ਪੰਨਾ<0 ){
        /* label-statNext-done:  When computing aggregate space usage over
        ** an entire btree, this is the exit point from this function */
        ਵਾਪਸ SQLITE_OK;
      }
      ਜਾਓ stat_ਅਗਲਾ_Restart; /* Tail recursion */
    }
    pCsr->i_ਪੰਨਾ++;
    ਜੇ( pCsr->i_ਪੰਨਾ>=Array_ਮਾਪ(pCsr->a_ਪੰਨਾ) ){
      statResetCsr(pCsr);
      ਵਾਪਸ SQLITE_CORRUPT_BKPT;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( p==&pCsr->a_ਪੰਨਾ[pCsr->i_ਪੰਨਾ-1] );

    ਜੇ( p->iCell==p->nCell ){
      p[1].iPgno = p->i_ਸੱਜਾ_ਬੱਚਾ_Pg;
    }ਨਹੀਂਤਾਂ{
      p[1].iPgno = p->aCell[p->iCell].i_ਬੱਚਾ_Pg;
    }
    rc = stat_Get_ਪੰਨਾ(pBt, p[1].iPgno, &p[1]);
    pCsr->n_ਪੰਨਾ++;
    p[1].iCell = 0;
    ਜੇ( !pCsr->isAgg ){
      p[1].z_ਰਸਤਾ = z = sqlite3_mprintf("%s%.3x/", p->z_ਰਸਤਾ, p->iCell);
      ਜੇ( z==0 ) rc = SQLITE_NOMEM_BKPT;
    }
    p->iCell++;
  }


  /* Populate the StatCursor fields with the values to be returned
  ** by the xColumn() and xRowid() methods.
  */
  ਜੇ( rc==SQLITE_OK ){
    ਪੂਰਨ i;
    Stat_ਪੰਨਾ *p = &pCsr->a_ਪੰਨਾ[pCsr->i_ਪੰਨਾ];
    pCsr->zName = (ਅੱਖਰ *)sqlite3_ਕਾਲਮ_ਪਾਠ(pCsr->pStmt, 0);
    pCsr->iPageno = p->iPgno;

    rc = stat_Decode_ਪੰਨਾ(pBt, p);
    ਜੇ( rc==SQLITE_OK ){
      stat_ਮਾਪ_And_ਵਿੱਥ(pCsr);

      ਚੋਣ( p->flags ){
        ਹਾਲਤ 0x05:             /* table internal */
        ਹਾਲਤ 0x02:             /* index internal */
          pCsr->zPagetype = "internal";
          ਰੋਕੋ;
        ਹਾਲਤ 0x0D:             /* table leaf */
        ਹਾਲਤ 0x0A:             /* index leaf */
          pCsr->zPagetype = "leaf";
          ਰੋਕੋ;
        ਮੂਲ:
          pCsr->zPagetype = "corrupted";
          ਰੋਕੋ;
      }
      pCsr->nCell += p->nCell;
      pCsr->n_ਅਣਵਰਤਿਆ += p->n_ਅਣਵਰਤਿਆ;
      ਜੇ( p->nMxPayload>pCsr->nMxPayload ) pCsr->nMxPayload = p->nMxPayload;
      ਜੇ( !pCsr->isAgg ){
        pCsr->z_ਰਸਤਾ = z = sqlite3_mprintf("%s", p->z_ਰਸਤਾ);
        ਜੇ( z==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      nPayload = 0;
      ਲਈ(i=0; i<p->nCell; i++){
        nPayload += p->aCell[i].nLocal;
      }
      pCsr->nPayload += nPayload;

      /* If computing aggregate space usage by btree, continue with the
      ** next page.  The loop will exit via the return at label-statNext-done
      */
      ਜੇ( pCsr->isAgg ) ਜਾਓ stat_ਅਗਲਾ_Restart;
    }
  }

  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ statEof(sqlite3_vtab_cursor *pCursor){
  StatCursor *pCsr = (StatCursor *)pCursor;
  ਵਾਪਸ pCsr->isEof;
}

/* Initialize a cursor according to the query plan idxNum using the
** arguments in argv[0].  See statBestIndex() for a description of the
** meaning of the bits in idxNum.
*/
ਸਥਿਰ ਪੂਰਨ statFilter(
  sqlite3_vtab_cursor *pCursor, 
  ਪੂਰਨ idx_ਅੰਕ, ਅਚਲ ਅੱਖਰ *idxStr,
  ਪੂਰਨ argc, sqlite3_ਮੁੱਲ **argv
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  Stat_ਸਾਰਣੀ *pTab = (Stat_ਸਾਰਣੀ*)(pCursor->pVtab);
  sqlite3_str *pSql;      /* Query of btrees to analyze */
  ਅੱਖਰ *zSql;             /* String value of pSql */
  ਪੂਰਨ iArg = 0;           /* Count of argv[] parameters used so far */
  ਪੂਰਨ rc = SQLITE_OK;     /* Result of this operation */
  ਅਚਲ ਅੱਖਰ *zName = 0;  /* Only provide analysis of this table */
  (ਖਾਲੀ)argc;
  (ਖਾਲੀ)idxStr;

  statResetCsr(pCsr);
  sqlite3_finalize(pCsr->pStmt);
  pCsr->pStmt = 0;
  ਜੇ( idx_ਅੰਕ & 0x01 ){
    /* schema=? constraint is present.  Get its value */
    ਅਚਲ ਅੱਖਰ *zDbase = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[iArg++]);
    pCsr->iDb = sqlite3_ਲੱਭੋ_Db_Name(pTab->db, zDbase);
    ਜੇ( pCsr->iDb<0 ){
      pCsr->iDb = 0;
      pCsr->isEof = 1;
      ਵਾਪਸ SQLITE_OK;
    }
  }ਨਹੀਂਤਾਂ{
    pCsr->iDb = pTab->iDb;
  }
  ਜੇ( idx_ਅੰਕ & 0x02 ){
    /* name=? constraint is present */
    zName = (ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(argv[iArg++]);
  }
  ਜੇ( idx_ਅੰਕ & 0x04 ){
    /* aggregate=? constraint is present */
    pCsr->isAgg = sqlite3_ਮੁੱਲ_double(argv[iArg++])!=0.0;
  }ਨਹੀਂਤਾਂ{
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
  ਜੇ( zName ){
    sqlite3_str_appendf(pSql, "WHERE name=%Q", zName);
  }
  ਜੇ( idx_ਅੰਕ & 0x08 ){
    sqlite3_str_appendf(pSql, " ORDER BY name");
  }
  zSql = sqlite3_str_ਅੰਤ(pSql);
  ਜੇ( zSql==0 ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }ਨਹੀਂਤਾਂ{
    rc = sqlite3_prepare_v2(pTab->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }

  ਜੇ( rc==SQLITE_OK ){
    pCsr->i_ਪੰਨਾ = -1;
    rc = stat_ਅਗਲਾ(pCursor);
  }
  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ stat_ਕਾਲਮ(
  sqlite3_vtab_cursor *pCursor, 
  sqlite3_context *ctx, 
  ਪੂਰਨ i
){
  StatCursor *pCsr = (StatCursor *)pCursor;
  ਚੋਣ( i ){
    ਹਾਲਤ 0:            /* name */
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pCsr->zName, -1, SQLITE_TRANSIENT);
      ਰੋਕੋ;
    ਹਾਲਤ 1:            /*ਰਸਤਾ*/
      ਜੇ( !pCsr->isAgg ){
        sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pCsr->z_ਰਸਤਾ, -1, SQLITE_TRANSIENT);
      }
      ਰੋਕੋ;
    ਹਾਲਤ 2:            /* pageno */
      ਜੇ( pCsr->isAgg ){
        sqlite3_ਨਤੀਜਾ_int64(ctx, pCsr->n_ਪੰਨਾ);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਨਤੀਜਾ_int64(ctx, pCsr->iPageno);
      }
      ਰੋਕੋ;
    ਹਾਲਤ 3:            /* pagetype */
      ਜੇ( !pCsr->isAgg ){
        sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, pCsr->zPagetype, -1, SQLITE_STATIC);
      }
      ਰੋਕੋ;
    ਹਾਲਤ 4:            /* ncell */
      sqlite3_ਨਤੀਜਾ_int64(ctx, pCsr->nCell);
      ਰੋਕੋ;
    ਹਾਲਤ 5:            /* payload */
      sqlite3_ਨਤੀਜਾ_int64(ctx, pCsr->nPayload);
      ਰੋਕੋ;
    ਹਾਲਤ 6:            /*ਅਣਵਰਤਿਆ*/
      sqlite3_ਨਤੀਜਾ_int64(ctx, pCsr->n_ਅਣਵਰਤਿਆ);
      ਰੋਕੋ;
    ਹਾਲਤ 7:            /* mx_payload */
      sqlite3_ਨਤੀਜਾ_int64(ctx, pCsr->nMxPayload);
      ਰੋਕੋ;
    ਹਾਲਤ 8:            /* pgoffset */
      ਜੇ( !pCsr->isAgg ){
        sqlite3_ਨਤੀਜਾ_int64(ctx, pCsr->i_ਵਿੱਥ);
      }
      ਰੋਕੋ;
    ਹਾਲਤ 9:            /* pgsize */
      sqlite3_ਨਤੀਜਾ_int64(ctx, pCsr->sz_ਪੰਨਾ);
      ਰੋਕੋ;
    ਹਾਲਤ 10: {         /* schema */
      sqlite3 *db = sqlite3_context_db_handle(ctx);
      ਪੂਰਨ iDb = pCsr->iDb;
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, db->aDb[iDb].zDbSName, -1, SQLITE_STATIC);
      ਰੋਕੋ;
    }
    ਮੂਲ: {         /* aggregate */
      sqlite3_ਨਤੀਜਾ_int(ctx, pCsr->isAgg);
      ਰੋਕੋ;
    }
  }
  ਵਾਪਸ SQLITE_OK;
}

ਸਥਿਰ ਪੂਰਨ statRowid(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  StatCursor *pCsr = (StatCursor *)pCursor;
  *pRowid = pCsr->iPageno;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Invoke this routine to register the "dbstat" virtual table module
*/
ਪੂਰਨ sqlite3_Dbstat_ਰਜਿਸਟਰ(sqlite3 *db){
  ਸਥਿਰ sqlite3_module dbstat_module = {
    0,                            /* iVersion */
    statConnect,                  /* xCreate */
    statConnect,                  /* xConnect */
    stat_ਉੱਤਮ_ਸੂਚਕਾਂਕ,                /* xBestIndex */
    statDisconnect,               /* xDisconnect */
    statDisconnect,               /* xDestroy */
    statOpen,                     /* xOpen - open a cursor */
    statClose,                    /* xClose - close a cursor */
    statFilter,                   /* xFilter - configure scan constraints */
    stat_ਅਗਲਾ,                     /* xNext - advance a cursor */
    statEof,                      /* xEof - check for end of scan */
    stat_ਕਾਲਮ,                   /* xColumn - read data */
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
  ਵਾਪਸ sqlite3_create_module(db, "dbstat", &dbstat_module, 0);
}
#ਜਾਂਜੇ defined(SQLITE_ENABLE_DBSTAT_VTAB)
ਪੂਰਨ sqlite3_Dbstat_ਰਜਿਸਟਰ(sqlite3 *db){ ਵਾਪਸ SQLITE_OK; }
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_DBSTAT_VTAB */
