/*
** 2015-06-06
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This module contains C code that generates VDBE code used to process
** the WHERE clause of SQL statements.
**
** This file was split off from where.c on 2015-06-06 in order to reduce the
** size of where.c and make it easier to edit.  This file contains the routines
** that actually generate the bulk of the WHERE loop code.  The original where.c
** file retains the code that does query planning and analysis.
*/
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "whereInt.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_EXPLAIN

/*
** Return the name of the i-th column of the pIdx index.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *explain_ಸೂಚ್ಯಂಕ_ಸ್ತಂಭ_Name(ಸೂಚ್ಯಂಕ *pIdx, ಪೂರ್ಣಾಂಕ i){
  i = pIdx->ai_ಸ್ತಂಭ[i];
  ಆದರೆ( i==XN_ಅಭಿವ್ಯಕ್ತಿ ) ಹಿಂತಿರುಗಿ "<expr>";
  ಆದರೆ( i==XN_ROWID ) ಹಿಂತಿರುಗಿ "rowid";
  ಹಿಂತಿರುಗಿ pIdx->p_ಕೋಷ್ಟಕ->a_ಲಂಬಸಾಲು[i].zCnName;
}

/*
** This routine is a helper for explainIndexRange() below
**
** pStr holds the text of an expression that we are building up one term
** at a time.  This routine adds a new term to the end of the expression.
** Terms are separated by AND so add the "AND" text for second and subsequent
** terms only.
*/
ಸ್ಥಿರ ಶೂನ್ಯ explain_Append_ಪದ(
  StrAccum *pStr,             /* The text expression being built */
  ಸೂಚ್ಯಂಕ *pIdx,                /* Index to read column names from */
  ಪೂರ್ಣಾಂಕ n_ಪದ,                  /* Number of terms */
  ಪೂರ್ಣಾಂಕ i_ಪದ,                  /* Zero-based index of first term. */
  ಪೂರ್ಣಾಂಕ bAnd,                   /* Non-zero to append " AND " */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOp             /* Name of the operator */
){
  ಪೂರ್ಣಾಂಕ i;

  ಪ್ರತಿಜ್ಞಾ( n_ಪದ>=1 );
  ಆದರೆ( bAnd ) sqlite3_str_append(pStr, " AND ", 5);

  ಆದರೆ( n_ಪದ>1 ) sqlite3_str_append(pStr, "(", 1);
  ಗಾಗಿ(i=0; i<n_ಪದ; i++){
    ಆದರೆ( i ) sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_appendall(pStr, explain_ಸೂಚ್ಯಂಕ_ಸ್ತಂಭ_Name(pIdx, i_ಪದ+i));
  }
  ಆದರೆ( n_ಪದ>1 ) sqlite3_str_append(pStr, ")", 1);

  sqlite3_str_append(pStr, zOp, 1);

  ಆದರೆ( n_ಪದ>1 ) sqlite3_str_append(pStr, "(", 1);
  ಗಾಗಿ(i=0; i<n_ಪದ; i++){
    ಆದರೆ( i ) sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_append(pStr, "?", 1);
  }
  ಆದರೆ( n_ಪದ>1 ) sqlite3_str_append(pStr, ")", 1);
}

/*
** Argument pLevel describes a strategy for scanning table pTab. This
** function appends text to pStr that describes the subset of table
** rows scanned by the strategy in the form of an SQL expression.
**
** For example, if the query:
**
**   SELECT * FROM t1 WHERE a=1 AND b>2;
**
** is run and there is an index on (a, b), then this function returns a
** string similar to:
**
**   "a=? AND b>?"
*/
ಸ್ಥಿರ ಶೂನ್ಯ explain_ಸೂಚ್ಯಂಕ_Range(StrAccum *pStr, WhereLoop *pLoop){
  ಸೂಚ್ಯಂಕ *p_ಸೂಚ್ಯಂಕ = pLoop->u.btree.p_ಸೂಚ್ಯಂಕ;
  u16 nEq = pLoop->u.btree.nEq;
  u16 nSkip = pLoop->nSkip;
  ಪೂರ್ಣಾಂಕ i, j;

  ಆದರೆ( nEq==0 && (pLoop->wsFlags&(WHERE_BTM_ಎಲ್ಲೆ|WHERE_ತುದಿ_ಎಲ್ಲೆ))==0 ) ಹಿಂತಿರುಗಿ;
  sqlite3_str_append(pStr, " (", 2);
  ಗಾಗಿ(i=0; i<nEq; i++){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = explain_ಸೂಚ್ಯಂಕ_ಸ್ತಂಭ_Name(p_ಸೂಚ್ಯಂಕ, i);
    ಆದರೆ( i ) sqlite3_str_append(pStr, " AND ", 5);
    sqlite3_str_appendf(pStr, i>=nSkip ? "%s=?" : "ANY(%s)", z);
  }

  j = i;
  ಆದರೆ( pLoop->wsFlags&WHERE_BTM_ಎಲ್ಲೆ ){
    explain_Append_ಪದ(pStr, p_ಸೂಚ್ಯಂಕ, pLoop->u.btree.nBtm, j, i, ">");
    i = 1;
  }
  ಆದರೆ( pLoop->wsFlags&WHERE_ತುದಿ_ಎಲ್ಲೆ ){
    explain_Append_ಪದ(pStr, p_ಸೂಚ್ಯಂಕ, pLoop->u.btree.n_ತುದಿ, j, i, "<");
  }
  sqlite3_str_append(pStr, ")", 1);
}

/*
** This function sets the P4 value of an existing OP_Explain opcode to
** text describing the loop in pLevel. If the OP_Explain opcode already has
** a P4 value, it is freed before it is overwritten.
*/
ಶೂನ್ಯ sqlite3_Where_ಕೂಡಿಸು_Explain_ಪಠ್ಯ(
  Parse *pParse,                  /* Parse context */
  ಪೂರ್ಣಾಂಕ addr,                       /* Address of OP_Explain opcode */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
#ಆದರೆ !defined(SQLITE_DEBUG)
  ಆದರೆ( sqlite3ParseToplevel(pParse)->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#ಆದರೆ_ಅಂತ್ಯ
  {
    VdbeOp *pOp = sqlite3VdbeGetOp(pParse->pVdbe, addr);
    Src_ಅಂಶ *p_ಅಂಶ = &pTabList->a[pLevel->iFrom];
    sqlite3 *db = pParse->db;     /* Database handle */
    ಪೂರ್ಣಾಂಕ is_ಹುಡುಕು;                 /* True for a SEARCH. False for SCAN. */
    WhereLoop *pLoop;             /* The controlling WhereLoop object */
    u32 flags;                    /* Flags that describe this loop */
#ಆದರೆ defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_EXPLAIN)
    ಅಕ್ಷರ *z_ಸಂದೇಶ;                   /* Text to add to EQP output */
#ಆದರೆ_ಅಂತ್ಯ
    StrAccum str;                 /* EQP output string */
    ಅಕ್ಷರ zBuf[100];               /* Initial space for EQP output string */

    ಆದರೆ( db->mallocFailed ) ಹಿಂತಿರುಗಿ;

    pLoop = pLevel->pWLoop;
    flags = pLoop->wsFlags;

    is_ಹುಡುಕು = (flags&(WHERE_BTM_ಎಲ್ಲೆ|WHERE_ತುದಿ_ಎಲ್ಲೆ))!=0
            || ((flags&WHERE_VIRTUALTABLE)==0 && (pLoop->u.btree.nEq>0))
            || (wctrlFlags&(WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX));

    sqlite3StrAccumInit(&str, db, zBuf, ಗಾತ್ರ(zBuf), SQLITE_MAX_ಉದ್ದಳತೆ);
    str.printfFlags = SQLITE_PRINTF_ಆಂತರಿಕ;
    sqlite3_str_appendf(&str, "%s %S%s",
       is_ಹುಡುಕು ? "SEARCH" : "SCAN",
       p_ಅಂಶ,
       p_ಅಂಶ->fg.fromExists ? " EXISTS" : "");
    ಆದರೆ( (flags & (WHERE_IPK|WHERE_VIRTUALTABLE))==0 ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFmt = 0;
      ಸೂಚ್ಯಂಕ *pIdx;

      ಪ್ರತಿಜ್ಞಾ( pLoop->u.btree.p_ಸೂಚ್ಯಂಕ!=0 );
      pIdx = pLoop->u.btree.p_ಸೂಚ್ಯಂಕ;
      ಪ್ರತಿಜ್ಞಾ( !(flags&WHERE_AUTO_ಸೂಚ್ಯಂಕ) || (flags&WHERE_IDX_ONLY) );
      ಆದರೆ( !HasRowid(p_ಅಂಶ->pSTab) && Is_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pIdx) ){
        ಆದರೆ( is_ಹುಡುಕು ){
          zFmt = "PRIMARY KEY";
        }
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flags & WHERE_PARTIALIDX ){
        zFmt = "AUTOMATIC PARTIAL COVERING INDEX";
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flags & WHERE_AUTO_ಸೂಚ್ಯಂಕ ){
        zFmt = "AUTOMATIC COVERING INDEX";
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
        zFmt = "COVERING INDEX %s";
      }ಇಲ್ಲದಿದ್ದರೆ{
        zFmt = "INDEX %s";
      }
      ಆದರೆ( zFmt ){
        sqlite3_str_append(&str, " USING ", 7);
        sqlite3_str_appendf(&str, zFmt, pIdx->zName);
        explain_ಸೂಚ್ಯಂಕ_Range(&str, pLoop);
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (flags & WHERE_IPK)!=0 && (flags & WHERE_CONSTRAINT)!=0 ){
      ಅಕ್ಷರ cRangeOp;
#ಆದರೆ 0  /* Better output, but breaks many tests */
      ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *pTab = p_ಅಂಶ->pTab;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zRowid = pTab->iPKey>=0 ? pTab->a_ಲಂಬಸಾಲು[pTab->iPKey].zCnName:
                              "rowid";
#ಇಲ್ಲದಿದ್ದರೆ
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zRowid = "rowid";
#ಆದರೆ_ಅಂತ್ಯ
      sqlite3_str_appendf(&str, " USING INTEGER PRIMARY KEY (%s", zRowid);
      ಆದರೆ( flags&(WHERE_ಸ್ತಂಭ_EQ|WHERE_ಸ್ತಂಭ_ಒಳ) ){
        cRangeOp = '=';
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (flags&WHERE_BOTH_ಎಲ್ಲೆ)==WHERE_BOTH_ಎಲ್ಲೆ ){
        sqlite3_str_appendf(&str, ">? AND %s", zRowid);
        cRangeOp = '<';
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flags&WHERE_BTM_ಎಲ್ಲೆ ){
        cRangeOp = '>';
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( flags&WHERE_ತುದಿ_ಎಲ್ಲೆ);
        cRangeOp = '<';
      }
      sqlite3_str_appendf(&str, "%c?)", cRangeOp);
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (flags & WHERE_VIRTUALTABLE)!=0 ){
      sqlite3_str_appendall(&str, " VIRTUAL TABLE INDEX ");
      sqlite3_str_appendf(&str,
                  pLoop->u.vtab.b_Idx_ಅಂಕೆ_Hex ? "0x%x:%s" : "%d:%s",
                  pLoop->u.vtab.idx_ಅಂಕೆ, pLoop->u.vtab.idxStr);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( p_ಅಂಶ->fg.jointype & JT_ಎಡ ){
      sqlite3_str_appendf(&str, " LEFT-JOIN");
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EXPLAIN_ESTIMATED_ಅಡ್ಡಸಾಲುಗಳು
    ಆದರೆ( pLoop->n_ಹೊರಹೋಗು>=10 ){
      sqlite3_str_appendf(&str, " (~%llu rows)",
             sqlite3_Log_Est_ಗಮ್ಯ_Int(pLoop->n_ಹೊರಹೋಗು));
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_str_append(&str, " (~1 row)", 9);
    }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_EXPLAIN)
    z_ಸಂದೇಶ = sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&str);
    sqlite3ExplainBreakpoint("",z_ಸಂದೇಶ);
#ಆದರೆ_ಅಂತ್ಯ

    ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_Explain );
    ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_DYNAMIC || pOp->p4.z==0 );
    sqlite3DbFree(db, pOp->p4.z);
    pOp->p4type = P4_DYNAMIC;
    pOp->p4.z = sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&str);
  }
}


/*
** This function is a no-op unless currently processing an EXPLAIN QUERY PLAN
** command, or if stmt_scanstatus_v2() stats are enabled, or if SQLITE_DEBUG
** was defined at compile-time. If it is not a no-op, a single OP_Explain
** opcode is added to the output to describe the table scan strategy in pLevel.
**
** If an OP_Explain opcode is added to the VM, its address is returned.
** Otherwise, if no OP_Explain is coded, zero is returned.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Where_Explain_One_ಪರಿಶೀಲನೆ(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
  ಪೂರ್ಣಾಂಕ ret = 0;
#ಆದರೆ !defined(SQLITE_DEBUG)
  ಆದರೆ( sqlite3ParseToplevel(pParse)->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#ಆದರೆ_ಅಂತ್ಯ
  {
    ಆದರೆ( (pLevel->pWLoop->wsFlags & WHERE_MULTI_OR)==0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
    ){
      Vdbe *v = pParse->pVdbe;
      ಪೂರ್ಣಾಂಕ addr = sqlite3VdbeCurrentAddr(v);
      ret = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(
          v, OP_Explain, addr, pParse->addrExplain, pLevel->pWLoop->r_ಸತತ
      );
      sqlite3_Where_ಕೂಡಿಸು_Explain_ಪಠ್ಯ(pParse, addr, pTabList, pLevel, wctrlFlags);
    }
  }
  ಹಿಂತಿರುಗಿ ret;
}

/*
** Add a single OP_Explain opcode that describes a Bloom filter.
**
** Or if not processing EXPLAIN QUERY PLAN and not in a SQLITE_DEBUG and/or
** SQLITE_ENABLE_STMT_SCANSTATUS build, then OP_Explain opcodes are not
** required and this routine is a no-op.
**
** If an OP_Explain opcode is added to the VM, its address is returned.
** Otherwise, if no OP_Explain is coded, zero is returned.
*/
ಪೂರ್ಣಾಂಕ sqlite3WhereExplainBloomFilter(
  ಸ್ಥಿರಾಂಕ Parse *pParse,               /* Parse context */
  ಸ್ಥಿರಾಂಕ WhereInfo *pWInfo,           /* WHERE clause */
  ಸ್ಥಿರಾಂಕ WhereLevel *pLevel           /* Bloom filter on this level */
){
  ಪೂರ್ಣಾಂಕ ret = 0;
  Src_ಅಂಶ *p_ಅಂಶ = &pWInfo->pTabList->a[pLevel->iFrom];
  Vdbe *v = pParse->pVdbe;      /* VM being constructed */
  sqlite3 *db = pParse->db;     /* Database handle */
  ಅಕ್ಷರ *z_ಸಂದೇಶ;                   /* Text to add to EQP output */
  ಪೂರ್ಣಾಂಕ i;                        /* Loop counter */
  WhereLoop *pLoop;             /* The where loop */
  StrAccum str;                 /* EQP output string */
  ಅಕ್ಷರ zBuf[100];               /* Initial space for EQP output string */

  sqlite3StrAccumInit(&str, db, zBuf, ಗಾತ್ರ(zBuf), SQLITE_MAX_ಉದ್ದಳತೆ);
  str.printfFlags = SQLITE_PRINTF_ಆಂತರಿಕ;
  sqlite3_str_appendf(&str, "BLOOM FILTER ON %S (", p_ಅಂಶ);
  pLoop = pLevel->pWLoop;
  ಆದರೆ( pLoop->wsFlags & WHERE_IPK ){
    ಸ್ಥಿರಾಂಕ ಕೋಷ್ಟಕ *pTab = p_ಅಂಶ->pSTab;
    ಆದರೆ( pTab->iPKey>=0 ){
      sqlite3_str_appendf(&str, "%s=?", pTab->a_ಲಂಬಸಾಲು[pTab->iPKey].zCnName);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_str_appendf(&str, "rowid=?");
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಗಾಗಿ(i=pLoop->nSkip; i<pLoop->u.btree.nEq; i++){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = explain_ಸೂಚ್ಯಂಕ_ಸ್ತಂಭ_Name(pLoop->u.btree.p_ಸೂಚ್ಯಂಕ, i);
      ಆದರೆ( i>pLoop->nSkip ) sqlite3_str_append(&str, " AND ", 5);
      sqlite3_str_appendf(&str, "%s=?", z);
    }
  }
  sqlite3_str_append(&str, ")", 1);
  z_ಸಂದೇಶ = sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&str);
  ret = sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Explain, sqlite3VdbeCurrentAddr(v),
                          pParse->addrExplain, 0, z_ಸಂದೇಶ,P4_DYNAMIC);

  sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ(v, sqlite3VdbeCurrentAddr(v)-1, 0, 0, 0, 0);
  ಹಿಂತಿರುಗಿ ret;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_EXPLAIN */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Configure the VM passed as the first argument with an
** sqlite3_stmt_scanstatus() entry corresponding to the scan used to
** implement level pLvl. Argument pSrclist is a pointer to the FROM
** clause that the scan reads data from.
**
** If argument addrExplain is not 0, it must be the address of an
** OP_Explain instruction that describes the same loop.
*/
ಶೂನ್ಯ sqlite3_Where_ಕೂಡಿಸು_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  ಪೂರ್ಣಾಂಕ addrExplain                 /* Address of OP_Explain (or 0) */
){
  ಆದರೆ( IS_STMT_SCANSTATUS( sqlite3VdbeDb(v) ) ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zObj = 0;
    WhereLoop *pLoop = pLvl->pWLoop;
    ಪೂರ್ಣಾಂಕ wsFlags = pLoop->wsFlags;
    ಪೂರ್ಣಾಂಕ ಮೂಲಕ_Coroutine = 0;

    ಆದರೆ( (wsFlags & WHERE_VIRTUALTABLE)==0  &&  pLoop->u.btree.p_ಸೂಚ್ಯಂಕ!=0 ){
      zObj = pLoop->u.btree.p_ಸೂಚ್ಯಂಕ->zName;
    }ಇಲ್ಲದಿದ್ದರೆ{
      zObj = pSrclist->a[pLvl->iFrom].zName;
      ಮೂಲಕ_Coroutine = pSrclist->a[pLvl->iFrom].fg.ಮೂಲಕ_Coroutine;
    }
    sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ(
        v, addrExplain, pLvl->addrBody, pLvl->addr_ಭೇಟಿ, pLoop->n_ಹೊರಹೋಗು, zObj
    );

    ಆದರೆ( ಮೂಲಕ_Coroutine==0 ){
      ಆದರೆ( (wsFlags & (WHERE_MULTI_OR|WHERE_AUTO_ಸೂಚ್ಯಂಕ))==0 ){
        sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Range(v, addrExplain, -1, pLvl->iTabCur);
      }
      ಆದರೆ( wsFlags & WHERE_ಸೂಚ್ಯಂಕಿತ ){
        sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Range(v, addrExplain, -1, pLvl->iIdxCur);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ addr;
      VdbeOp *pOp;
      ಪ್ರತಿಜ್ಞಾ( pSrclist->a[pLvl->iFrom].fg.isSubquery );
      addr = pSrclist->a[pLvl->iFrom].u4.pSubq->addrFillSub;
      pOp = sqlite3VdbeGetOp(v, addr-1);
      ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeDb(v)->mallocFailed || pOp->opcode==OP_InitCoroutine );
      ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeDb(v)->mallocFailed || pOp->p2>addr );
      sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Range(v, addrExplain, addr, pOp->p2-1);
    }
  }
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** Disable a term in the WHERE clause.  Except, do not disable the term
** if it controls a LEFT OUTER JOIN and it did not originate in the ON
** or USING clause of that join.
**
** Consider the term t2.z='ok' in the following queries:
**
**   (1)  SELECT * FROM t1 LEFT JOIN t2 ON t1.a=t2.x WHERE t2.z='ok'
**   (2)  SELECT * FROM t1 LEFT JOIN t2 ON t1.a=t2.x AND t2.z='ok'
**   (3)  SELECT * FROM t1, t2 WHERE t1.a=t2.x AND t2.z='ok'
**
** The t2.z='ok' is disabled in the in (2) because it originates
** in the ON clause.  The term is disabled in (3) because it is not part
** of a LEFT OUTER JOIN.  In (1), the term is not disabled.
**
** Disabling a term causes that term to not be tested in the inner loop
** of the join.  Disabling is an optimization.  When terms are satisfied
** by indices, we disable them to prevent redundant tests in the inner
** loop.  We would get the correct results if nothing were ever disabled,
** but joins might run a little slower.  The trick is to disable as much
** as we can without disabling too much.  If we disabled in (1), we'd get
** the wrong answer.  See ticket #813.
**
** If all the children of a term are disabled, then that term is also
** automatically disabled.  In this way, terms get disabled if derived
** virtual terms are tested first.  For example:
**
**      x GLOB 'abc*' AND x>='abc' AND x<'acd'
**      \___________/     \______/     \_____/
**         parent          child1       child2
**
** Only the parent term was in the original WHERE clause.  The child1
** and child2 terms were added by the LIKE optimization.  If both of
** the virtual child terms are valid, then testing of the parent can be
** skipped.
**
** Usually the parent term is marked as TERM_CODED.  But if the parent
** term was originally TERM_LIKE, then the parent gets TERM_LIKECOND instead.
** The TERM_LIKECOND marking indicates that the term should be coded inside
** a conditional such that is only evaluated on the second pass of a
** LIKE-optimization loop, when scanning BLOBs instead of strings.
*/
ಸ್ಥಿರ ಶೂನ್ಯ disable_ಪದ(WhereLevel *pLevel, Where_ಪದ *p_ಪದ){
  ಪೂರ್ಣಾಂಕ nLoop = 0;
  ಪ್ರತಿಜ್ಞಾ( p_ಪದ!=0 );
  ಆಗ( (p_ಪದ->wtFlags & ಪದ_CODED)==0
      && (pLevel->i_ಎಡ_Join==0 || ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON))
      && (pLevel->notReady & p_ಪದ->prereqAll)==0
  ){
    ಆದರೆ( nLoop && (p_ಪದ->wtFlags & ಪದ_LIKE)!=0 ){
      p_ಪದ->wtFlags |= ಪದ_LIKECOND;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಪದ->wtFlags |= ಪದ_CODED;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WHERETRACE_ENABLED
    ಆದರೆ( (sqlite3WhereTrace & 0x4001)==0x4001 ){
      sqlite3DebugPrintf("DISABLE-");
      sqlite3_Where_ಪದ_Print(p_ಪದ, (ಪೂರ್ಣಾಂಕ)(p_ಪದ - (p_ಪದ->pWC->a)));
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( p_ಪದ->i_ಪೋಷಕ<0 ) ನಿಲ್ಲಿಸು;
    p_ಪದ = &p_ಪದ->pWC->a[p_ಪದ->i_ಪೋಷಕ];
    ಪ್ರತಿಜ್ಞಾ( p_ಪದ!=0 );
    p_ಪದ->n_ಮಗು--;
    ಆದರೆ( p_ಪದ->n_ಮಗು!=0 ) ನಿಲ್ಲಿಸು;
    nLoop++;
  }
}

/*
** Code an OP_Affinity opcode to apply the column affinity string zAff
** to the n registers starting at base.
**
** As an optimization, SQLITE_AFF_BLOB and SQLITE_AFF_NONE entries (which
** are no-ops) at the beginning and end of zAff are ignored.  If all entries
** in zAff are SQLITE_AFF_BLOB or SQLITE_AFF_NONE, then no code gets generated.
**
** This routine makes its own copy of zAff so that the caller is free
** to modify zAff after this routine returns.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಸಂಕೇತ_Apply_Affinity(Parse *pParse, ಪೂರ್ಣಾಂಕ ತಳ, ಪೂರ್ಣಾಂಕ n, ಅಕ್ಷರ *zAff){
  Vdbe *v = pParse->pVdbe;
  ಆದರೆ( zAff==0 ){
    ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed );
    ಹಿಂತಿರುಗಿ;
  }
  ಪ್ರತಿಜ್ಞಾ( v!=0 );

  /* Adjust base and n to skip over SQLITE_AFF_BLOB and SQLITE_AFF_NONE
  ** entries at the beginning and end of the affinity string.
  */
  ಪ್ರತಿಜ್ಞಾ( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  ಆಗ( n>0 && zAff[0]<=SQLITE_AFF_BLOB ){
    n--;
    ತಳ++;
    zAff++;
  }
  ಆಗ( n>1 && zAff[n-1]<=SQLITE_AFF_BLOB ){
    n--;
  }

  /* Code the OP_Affinity opcode if there is anything left to do. */
  ಆದರೆ( n>0 ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Affinity, ತಳ, n, 0, zAff, n);
  }
}

/*
** Expression pRight, which is the RHS of a comparison operation, is
** either a vector of n elements or, if n==1, a scalar expression.
** Before the comparison operation, affinity zAff is to be applied
** to the pRight values. This function modifies characters within the
** affinity string to SQLITE_AFF_BLOB if either:
**
**   * the comparison will be performed with no affinity, or
**   * the affinity change in zAff is guaranteed not to change the value.
*/
ಸ್ಥಿರ ಶೂನ್ಯ updateRangeAffinityStr(
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ,                   /* RHS of comparison */
  ಪೂರ್ಣಾಂಕ n,                          /* Number of vector elements in comparison */
  ಅಕ್ಷರ *zAff                      /* Affinity string to modify */
){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<n; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p = sqlite3_ಸದಿಶ_ಕ್ಷೇತ್ರ_Subexpr(p_ಬಲ, i);
    ಆದರೆ( sqlite3CompareAffinity(p, zAff[i])==SQLITE_AFF_BLOB
     || sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Needs_No_Affinity_Change(p, zAff[i])
    ){
      zAff[i] = SQLITE_AFF_BLOB;
    }
  }
}

/*
** The pOrderBy->a[].u.x.iOrderByCol values might be incorrect because
** columns might have been rearranged in the result set.  This routine
** fixes them up.
**
** pEList is the new result set.  The pEList->a[].u.x.iOrderByCol values
** contain the *old* locations of each expression.  This is a temporary
** use of u.x.iOrderByCol, not its intended use.  The caller must reset
** u.x.iOrderByCol back to zero for all entries in pEList before the
** caller returns.
**
** This routine changes pOrderBy->a[].u.x.iOrderByCol values from
** pEList->a[N].u.x.iOrderByCol into N+1.  (The "+1" is because of the 1-based
** indexing used by iOrderByCol.)  Or if no match, iOrderByCol is set to zero.
*/
ಸ್ಥಿರ ಶೂನ್ಯ adjust_ಕ್ರಮ_By_ಲಂಬಸಾಲು(ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By, ಅಭಿವ್ಯಕ್ತಿ_List *pEList){
  ಪೂರ್ಣಾಂಕ i, j;
  ಆದರೆ( p_ಕ್ರಮ_By==0 ) ಹಿಂತಿರುಗಿ;
  ಗಾಗಿ(i=0; i<p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಪೂರ್ಣಾಂಕ t = p_ಕ್ರಮ_By->a[i].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು;
    ಆದರೆ( t==0 ) ಮುಂದುವರಿ;
    ಗಾಗಿ(j=0; j<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
      ಆದರೆ( pEList->a[j].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು==t ){
        p_ಕ್ರಮ_By->a[i].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = j+1;
        ನಿಲ್ಲಿಸು;
      }
    }
    ಆದರೆ( j>=pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
      p_ಕ್ರಮ_By->a[i].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = 0;
    }
  }
}


/*
** pX is an expression of the form:  (vector) IN (SELECT ...)
** In other words, it is a vector IN operator with a SELECT clause on the
** RHS.  But not all terms in the vector are indexable and the terms might
** not be in the correct order for indexing.
**
** This routine makes a copy of the input pX expression and then adjusts
** the vector on the LHS with corresponding changes to the SELECT so that
** the vector contains only index terms and those terms are in the correct
** order.  The modified IN expression is returned.  The caller is responsible
** for deleting the returned expression.
**
** Example:
**
**    CREATE TABLE t1(a,b,c,d,e,f);
**    CREATE INDEX t1x1 ON t1(e,c);
**    SELECT * FROM t1 WHERE (a,b,c,d,e) IN (SELECT v,w,x,y,z FROM t2)
**                           \_______________________________________/
**                                     The pX expression
**
** Since only columns e and c can be used with the index, in that order,
** the modified IN expression that is returned will be:
**
**        (e,c) IN (SELECT z,x FROM t2)
**
** The reduced pX is different from the original (obviously) and thus is
** only used for indexing, to improve performance.  The original unaltered
** IN expression must also be run on each output row for correctness.
*/
ಸ್ಥಿರ ಅಭಿವ್ಯಕ್ತಿ *remove_Unindexable_ಒಳ_Clause_Terms(
  Parse *pParse,        /* The parsing context */
  ಪೂರ್ಣಾಂಕ iEq,              /* Look at loop terms starting here */
  WhereLoop *pLoop,     /* The current loop */
  ಅಭಿವ್ಯಕ್ತಿ *pX              /* The IN expression to be reduced */
){
  sqlite3 *db = pParse->db;
  Select *pSelect;            /* Pointer to the SELECT on the RHS */
  ಅಭಿವ್ಯಕ್ತಿ *pNew;
  pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pX, 0);
  ಆದರೆ( db->mallocFailed==0 ){
    ಗಾಗಿ(pSelect=pNew->x.pSelect; pSelect; pSelect=pSelect->p_ಪೂರ್ವ){
      ಅಭಿವ್ಯಕ್ತಿ_List *pOrigRhs;         /* Original unmodified RHS */
      ಅಭಿವ್ಯಕ್ತಿ_List *pOrigLhs = 0;     /* Original unmodified LHS */
      ಅಭಿವ್ಯಕ್ತಿ_List *pRhs = 0;         /* New RHS after modifications */
      ಅಭಿವ್ಯಕ್ತಿ_List *pLhs = 0;         /* New LHS after mods */
      ಪೂರ್ಣಾಂಕ i;                      /* Loop counter */
      ಪೂರ್ಣಾಂಕ nRhs = 0;               /* Number of RHS terms added so far */

      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pNew) );
      pOrigRhs = pSelect->pEList;
      ಪ್ರತಿಜ್ಞಾ( pNew->p_ಎಡ!=0 );
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pNew->p_ಎಡ) );
      ಆದರೆ( pSelect==pNew->x.pSelect ){
        pOrigLhs = pNew->p_ಎಡ->x.pList;
      }
      ಗಾಗಿ(i=iEq; i<pLoop->nLTerm; i++){
        ಆದರೆ( pLoop->aLTerm[i]->p_ಅಭಿವ್ಯಕ್ತಿ==pX ){
          ಪೂರ್ಣಾಂಕ i_ಕ್ಷೇತ್ರ;
          ಪ್ರತಿಜ್ಞಾ( (pLoop->aLTerm[i]->eOperator & (WO_OR|WO_AND))==0 );
          i_ಕ್ಷೇತ್ರ = pLoop->aLTerm[i]->u.x.i_ಕ್ಷೇತ್ರ - 1;
          ಆದರೆ( i_ಕ್ಷೇತ್ರ!=nRhs ){
            ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(pNew, EP_Subrtn);
          }
          ಆದರೆ( NEVER(pOrigRhs->a[i_ಕ್ಷೇತ್ರ].p_ಅಭಿವ್ಯಕ್ತಿ==0) ){
            ಮುಂದುವರಿ; /* Duplicate PK column */
          }
          pRhs = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pRhs, pOrigRhs->a[i_ಕ್ಷೇತ್ರ].p_ಅಭಿವ್ಯಕ್ತಿ);
          pOrigRhs->a[i_ಕ್ಷೇತ್ರ].p_ಅಭಿವ್ಯಕ್ತಿ = 0;
          ಆದರೆ( pRhs ) pRhs->a[pRhs->n_ಅಭಿವ್ಯಕ್ತಿ-1].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = i_ಕ್ಷೇತ್ರ+1;
          ಆದರೆ( pOrigLhs ){
            ಪ್ರತಿಜ್ಞಾ( pOrigLhs->a[i_ಕ್ಷೇತ್ರ].p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
            pLhs = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse,pLhs,pOrigLhs->a[i_ಕ್ಷೇತ್ರ].p_ಅಭಿವ್ಯಕ್ತಿ);
            pOrigLhs->a[i_ಕ್ಷೇತ್ರ].p_ಅಭಿವ್ಯಕ್ತಿ = 0;
          }
          nRhs++;
        }
      }
      ಆದರೆ( nRhs<pOrigRhs->n_ಅಭಿವ್ಯಕ್ತಿ ){
        ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(pNew, EP_Subrtn); /* See bug 2026-10-16T22:07:15Z */
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pOrigRhs);
      ಆದರೆ( pOrigLhs ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pOrigLhs);
        pNew->p_ಎಡ->x.pList = pLhs;
      }
      pSelect->pEList = pRhs;
      pSelect->selId = ++pParse->nSelect; /* Req'd for SubrtnSig validity */
      ಆದರೆ( pLhs && pLhs->n_ಅಭಿವ್ಯಕ್ತಿ==1 ){
        /* Take care here not to generate a TK_VECTOR containing only a
        ** single value. Since the parser never creates such a vector, some
        ** of the subroutines do not handle this case.  */
        ಅಭಿವ್ಯಕ್ತಿ *p = pLhs->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
        pLhs->a[0].p_ಅಭಿವ್ಯಕ್ತಿ = 0;
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pNew->p_ಎಡ);
        pNew->p_ಎಡ = p;
      }

      /* If either the ORDER BY clause or the GROUP BY clause contains
      ** references to result-set columns, those references might now be
      ** obsolete.  So fix them up.
      */
      ಪ್ರತಿಜ್ಞಾ( pRhs!=0 || db->mallocFailed );
      ಆದರೆ( pRhs ){
        adjust_ಕ್ರಮ_By_ಲಂಬಸಾಲು(pSelect->p_ಕ್ರಮ_By, pRhs);
        adjust_ಕ್ರಮ_By_ಲಂಬಸಾಲು(pSelect->p_ಗುಂಪು_By, pRhs);
        ಗಾಗಿ(i=0; i<pRhs->n_ಅಭಿವ್ಯಕ್ತಿ; i++) pRhs->a[i].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = 0;
      }

#ಆದರೆ 0
      ಬರೆ("For indexing, change the IN expr:\n");
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(0, pX, 0);
      ಬರೆ("Into:\n");
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(0, pNew, 0);
#ಆದರೆ_ಅಂತ್ಯ
    }
  }
  ಹಿಂತಿರುಗಿ pNew;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
/*
** Generate code for a single X IN (....) term of the WHERE clause.
**
** This is a special-case of codeEqualityTerm() that works for IN operators
** only.  It is broken out into a subroutine because this case is
** uncommon and by splitting it off into a subroutine, the common case
** runs faster.
**
** The current value for the constraint is left in  register iTarget.
** This routine sets up a loop that will iterate over all values of X.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ ಸಂಕೇತ_INTerm(
  Parse *pParse,      /* The parsing context */
  Where_ಪದ *p_ಪದ,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  ಪೂರ್ಣಾಂಕ iEq,            /* Index of the equality term within this level */
  ಪೂರ್ಣಾಂಕ bRev,           /* True for reverse-order IN operations */
  ಪೂರ್ಣಾಂಕ i_ಗುರಿ         /* Attempt to leave results in this register */
){
  ಅಭಿವ್ಯಕ್ತಿ *pX = p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ;
  ಪೂರ್ಣಾಂಕ eType = ಒಳ_ಸೂಚ್ಯಂಕ_NOOP;
  ಪೂರ್ಣಾಂಕ iTab;
  ರಚನೆ ಒಳ_Loop *p_ಒಳ;
  WhereLoop *pLoop = pLevel->pWLoop;
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ nEq = 0;
  ಪೂರ್ಣಾಂಕ *aiMap = 0;

  ಆದರೆ( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
    && pLoop->u.btree.p_ಸೂಚ್ಯಂಕ!=0
    && pLoop->u.btree.p_ಸೂಚ್ಯಂಕ->a_ವಿಂಗಡಿಸು_ಕ್ರಮ[iEq]
  ){
    testcase( iEq==0 );
    testcase( bRev );
    bRev = !bRev;
  }
  ಪ್ರತಿಜ್ಞಾ( pX->op==TK_ಒಳ );

  ಗಾಗಿ(i=0; i<iEq; i++){
    ಆದರೆ( pLoop->aLTerm[i] && pLoop->aLTerm[i]->p_ಅಭಿವ್ಯಕ್ತಿ==pX ){
      disable_ಪದ(pLevel, p_ಪದ);
      ಹಿಂತಿರುಗಿ;
    }
  }
  ಗಾಗಿ(i=iEq; i<pLoop->nLTerm; i++){
    ಪ್ರತಿಜ್ಞಾ( pLoop->aLTerm[i]!=0 );
    ಆದರೆ( pLoop->aLTerm[i]->p_ಅಭಿವ್ಯಕ್ತಿ==pX ) nEq++;
  }

  iTab = 0;
  ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pX) || pX->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ==1 ){
    eType = sqlite3_ಕಂಡುಹಿಡಿ_ಒಳ_ಸೂಚ್ಯಂಕ(pParse, pX, ಒಳ_ಸೂಚ್ಯಂಕ_LOOP, 0, 0, &iTab);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3 *db = pParse->db;
    ಅಭಿವ್ಯಕ್ತಿ *pXMod = remove_Unindexable_ಒಳ_Clause_Terms(pParse, iEq, pLoop, pX);
    ಆದರೆ( nEq>1 ){
      /* If this IN(SELECT ...) expression drives more than one column of
      ** the index, disable the seek-scan optimization. The reasons for this
      ** are that (a) it is only possible to make this happen by populating
      ** the sqlite_stat1 table with inconsistent information, and (b) it
      ** would require sqlite3FindInIndex() to find an index that is not
      ** only unique for the columns in question, but also delivers them
      ** in sorted order (requires checking asc/desc, and rejecting cases
      ** where the indexed columns are not in the right order).  */
      pLoop->wsFlags &= ~WHERE_ಒಳ_SEEKSCAN;
    }
    ಆದರೆ( !db->mallocFailed ){
      ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = pX->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
      aiMap = (ಪೂರ್ಣಾಂಕ*)sqlite3DbMallocZero(db, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)*n_ಲಂಬಸಾಲು);
      eType = sqlite3_ಕಂಡುಹಿಡಿ_ಒಳ_ಸೂಚ್ಯಂಕ(pParse, pXMod, ಒಳ_ಸೂಚ್ಯಂಕ_LOOP, 0, aiMap, &iTab);
    }
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pXMod);
  }

  ಆದರೆ( eType==ಒಳ_ಸೂಚ್ಯಂಕ_ಸೂಚ್ಯಂಕ_DESC ){
    testcase( bRev );
    bRev = !bRev;
  }
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, bRev ? OP_ಕೊನೆಯ : OP_Rewind, iTab, 0);
  VdbeCoverageIf(v, bRev);
  VdbeCoverageIf(v, !bRev);

  ಪ್ರತಿಜ್ಞಾ( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );
  pLoop->wsFlags |= WHERE_ಒಳ_ABLE;
  ಆದರೆ( pLevel->u.ಒಳ.n_ಒಳ==0 ){
    pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
  }
  ಆದರೆ( iEq>0 && (pLoop->wsFlags & WHERE_ಒಳ_SEEKSCAN)==0 ){
    pLoop->wsFlags |= WHERE_ಒಳ_EARLYOUT;
  }

  i = pLevel->u.ಒಳ.n_ಒಳ;
  pLevel->u.ಒಳ.n_ಒಳ += nEq;
  pLevel->u.ಒಳ.a_ಒಳ_Loop =
     sqlite3WhereRealloc(p_ಪದ->pWC->pWInfo,
                         pLevel->u.ಒಳ.a_ಒಳ_Loop,
                         ಗಾತ್ರ(pLevel->u.ಒಳ.a_ಒಳ_Loop[0])*pLevel->u.ಒಳ.n_ಒಳ);
  p_ಒಳ = pLevel->u.ಒಳ.a_ಒಳ_Loop;
  ಆದರೆ( p_ಒಳ ){
    ಪೂರ್ಣಾಂಕ iMap = 0;               /* Index in aiMap[] */
    p_ಒಳ += i;
    ಗಾಗಿ(i=iEq; i<pLoop->nLTerm; i++){
      ಆದರೆ( pLoop->aLTerm[i]->p_ಅಭಿವ್ಯಕ್ತಿ==pX ){
        ಪೂರ್ಣಾಂಕ i_ಹೊರಹೋಗು = i_ಗುರಿ + i - iEq;
        ಆದರೆ( eType==ಒಳ_ಸೂಚ್ಯಂಕ_ROWID ){
          p_ಒಳ->addr_ಒಳ_ತುದಿ = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rowid, iTab, i_ಹೊರಹೋಗು);
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = aiMap ? aiMap[iMap++] : 0;
          p_ಒಳ->addr_ಒಳ_ತುದಿ = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v,OP_ಸ್ತಂಭ,iTab, i_ಲಂಬಸಾಲು, i_ಹೊರಹೋಗು);
        }
        sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_IsNull, i_ಹೊರಹೋಗು); VdbeCoverage(v);
        ಆದರೆ( i==iEq ){
          p_ಒಳ->iCur = iTab;
          p_ಒಳ->e_ಕೊನೆ_Loop_Op = bRev ? OP_Prev : OP_ಮುಂದಿನ;
          ಆದರೆ( iEq>0 ){
            p_ಒಳ->i_ತಳ = i_ಗುರಿ - i;
            p_ಒಳ->n_ಪೂರ್ವಪ್ರತ್ಯಯ = i;
          }ಇಲ್ಲದಿದ್ದರೆ{
            p_ಒಳ->n_ಪೂರ್ವಪ್ರತ್ಯಯ = 0;
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          p_ಒಳ->e_ಕೊನೆ_Loop_Op = OP_Noop;
        }
        p_ಒಳ++;
      }
    }
    testcase( iEq>0
              && (pLoop->wsFlags & WHERE_ಒಳ_SEEKSCAN)==0
              && (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 );
    ಆದರೆ( iEq>0
     && (pLoop->wsFlags & (WHERE_ಒಳ_SEEKSCAN|WHERE_VIRTUALTABLE))==0
    ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Seek_ದೊರಕಿತು, pLevel->iIdxCur, 0, iEq);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    pLevel->u.ಒಳ.n_ಒಳ = 0;
  }
  sqlite3DbFree(pParse->db, aiMap);
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** Generate code for a single equality term of the WHERE clause.  An equality
** term can be either X=expr or X IN (...).   pTerm is the term to be
** coded.
**
** The current value for the constraint is left in a register, the index
** of which is returned.  An attempt is made store the result in iTarget but
** this is only guaranteed for TK_ISNULL and TK_IN constraints.  If the
** constraint is a TK_EQ or TK_IS, then the current value might be left in
** some other register and it is the caller's responsibility to compensate.
**
** For a constraint of the form X=expr, the expression is evaluated in
** straight-line code.  For constraints of the form X IN (...)
** this routine sets up a loop that will iterate over all values of X.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_Equality_ಪದ(
  Parse *pParse,      /* The parsing context */
  Where_ಪದ *p_ಪದ,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  ಪೂರ್ಣಾಂಕ iEq,            /* Index of the equality term within this level */
  ಪೂರ್ಣಾಂಕ bRev,           /* True for reverse-order IN operations */
  ಪೂರ್ಣಾಂಕ i_ಗುರಿ         /* Attempt to leave results in this register */
){
  ಅಭಿವ್ಯಕ್ತಿ *pX = p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ;
  ಪೂರ್ಣಾಂಕ iReg;                  /* Register holding results */

  ಪ್ರತಿಜ್ಞಾ( pLevel->pWLoop->aLTerm[iEq]==p_ಪದ );
  ಪ್ರತಿಜ್ಞಾ( i_ಗುರಿ>0 );
  ಆದರೆ( pX->op==TK_EQ || pX->op==TK_IS ){
    iReg = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pParse, pX->p_ಬಲ, i_ಗುರಿ);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pX->op==TK_ISNULL ){
    iReg = i_ಗುರಿ;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(pParse->pVdbe, OP_Null, 0, iReg);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pX->op==TK_ಒಳ );
    iReg = i_ಗುರಿ;
    ಸಂಕೇತ_INTerm(pParse, p_ಪದ, pLevel, iEq, bRev, i_ಗುರಿ);
#ಆದರೆ_ಅಂತ್ಯ
  }

  /* As an optimization, try to disable the WHERE clause term that is
  ** driving the index as it will always be true.  The correct answer is
  ** obtained regardless, but we might get the answer with fewer CPU cycles
  ** by omitting the term.
  **
  ** But do not disable the term unless we are certain that the term is
  ** not a transitive constraint.  For an example of where that does not
  ** work, see https://sqlite.org/forum/forumpost/eb8613976a (2021-05-04)
  */
  ಆದರೆ( (pLevel->pWLoop->wsFlags & WHERE_TRANSCONS)==0
   || (p_ಪದ->eOperator & WO_EQUIV)==0
  ){
    disable_ಪದ(pLevel, p_ಪದ);
  }

  ಹಿಂತಿರುಗಿ iReg;
}

/*
** Generate code that will evaluate all == and IN constraints for an
** index scan.
**
** For example, consider table t1(a,b,c,d,e,f) with index i1(a,b,c).
** Suppose the WHERE clause is this:  a==5 AND b IN (1,2,3) AND c>5 AND c<10
** The index has as many as three equality constraints, but in this
** example, the third "c" value is an inequality.  So only two
** constraints are coded.  This routine will generate code to evaluate
** a==5 and b IN (1,2,3).  The current values for a and b will be stored
** in consecutive registers and the index of the first register is returned.
**
** In the example above nEq==2.  But this subroutine works for any value
** of nEq including 0.  If nEq==0, this routine is nearly a no-op.
** The only thing it does is allocate the pLevel->iMem memory cell and
** compute the affinity string.
**
** The nExtraReg parameter is 0 or 1.  It is 0 if all WHERE clause constraints
** are == or IN and are covered by the nEq.  nExtraReg is 1 if there is
** an inequality constraint (such as the "c>=5 AND c<10" in the example) that
** occurs after the nEq quality constraints.
**
** This routine allocates a range of nEq+nExtraReg memory cells and returns
** the index of the first memory cell in that range. The code that
** calls this routine will use that memory range to store keys for
** start and termination conditions of the loop.
** key value of the loop.  If one or more IN operators appear, then
** this routine allocates an additional nEq memory cells for internal
** use.
**
** Before returning, *pzAff is set to point to a buffer containing a
** copy of the column affinity string of the index allocated using
** sqlite3DbMalloc(). Except, entries in the copy of the string associated
** with equality constraints that use BLOB or NONE affinity are set to
** SQLITE_AFF_BLOB. This is to deal with SQL such as the following:
**
**   CREATE TABLE t1(a TEXT PRIMARY KEY, b);
**   SELECT ... FROM t1 AS t2, t1 WHERE t1.a = t2.b;
**
** In the example above, the index on t1(a) has TEXT affinity. But since
** the right hand side of the equality constraint (t2.b) has BLOB/NONE affinity,
** no conversion should be attempted before using a t2.b value as part of
** a key to search the index. Hence the first byte in the returned affinity
** string in this example would be set to SQLITE_AFF_BLOB.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_All_Equality_Terms(
  Parse *pParse,        /* Parsing context */
  WhereLevel *pLevel,   /* Which nested loop of the FROM we are coding */
  ಪೂರ್ಣಾಂಕ bRev,             /* Reverse the order of IN operators */
  ಪೂರ್ಣಾಂಕ nExtraReg,        /* Number of extra registers to allocate */
  ಅಕ್ಷರ **pzAff          /* OUT: Set to point to affinity string */
){
  u16 nEq;                      /* The number of == or IN constraints to code */
  u16 nSkip;                    /* Number of left-most columns to skip */
  Vdbe *v = pParse->pVdbe;      /* The vm under construction */
  ಸೂಚ್ಯಂಕ *pIdx;                  /* The index being used for this loop */
  Where_ಪದ *p_ಪದ;             /* A single constraint term */
  WhereLoop *pLoop;             /* The WhereLoop object */
  ಪೂರ್ಣಾಂಕ j;                        /* Loop counter */
  ಪೂರ್ಣಾಂಕ reg_ತಳ;                  /* Base register */
  ಪೂರ್ಣಾಂಕ nReg;                     /* Number of registers to allocate */
  ಅಕ್ಷರ *zAff;                   /* Affinity string to return */

  /* This module is only called on query plans that use an index. */
  pLoop = pLevel->pWLoop;
  ಪ್ರತಿಜ್ಞಾ( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0 );
  nEq = pLoop->u.btree.nEq;
  nSkip = pLoop->nSkip;
  pIdx = pLoop->u.btree.p_ಸೂಚ್ಯಂಕ;
  ಪ್ರತಿಜ್ಞಾ( pIdx!=0 );

  /* Figure out how many memory cells we will need then allocate them.
  */
  reg_ತಳ = pParse->nMem + 1;
  nReg = nEq + nExtraReg;
  pParse->nMem += nReg;

  zAff = sqlite3DbStrDup(pParse->db,sqlite3_ಸೂಚ್ಯಂಕ_Affinity_Str(pParse->db,pIdx));
  ಪ್ರತಿಜ್ಞಾ( zAff!=0 || pParse->db->mallocFailed );

  ಆದರೆ( nSkip ){
    ಪೂರ್ಣಾಂಕ iIdxCur = pLevel->iIdxCur;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Null, 0, reg_ತಳ, reg_ತಳ+nSkip-1);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, (bRev?OP_ಕೊನೆಯ:OP_Rewind), iIdxCur);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    VdbeComment((v, "begin skip-scan on %s", pIdx->zName));
    j = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Goto);
    ಪ್ರತಿಜ್ಞಾ( pLevel->addrSkip==0 );
    pLevel->addrSkip = sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, (bRev?OP_SeekLT:OP_SeekGT),
                            iIdxCur, 0, reg_ತಳ, nSkip);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, j);
    ಗಾಗಿ(j=0; j<nSkip; j++){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iIdxCur, j, reg_ತಳ+j);
      testcase( pIdx->ai_ಸ್ತಂಭ[j]==XN_ಅಭಿವ್ಯಕ್ತಿ );
      VdbeComment((v, "%s", explain_ಸೂಚ್ಯಂಕ_ಸ್ತಂಭ_Name(pIdx, j)));
    }
  }

  /* Evaluate the equality constraints
  */
  ಪ್ರತಿಜ್ಞಾ( zAff==0 || (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zAff)>=nEq );
  ಗಾಗಿ(j=nSkip; j<nEq; j++){
    ಪೂರ್ಣಾಂಕ r1;
    p_ಪದ = pLoop->aLTerm[j];
    ಪ್ರತಿಜ್ಞಾ( p_ಪದ!=0 );
    /* The following testcase is true for indices with redundant columns.
    ** Ex: CREATE INDEX i1 ON t1(a,b,a); SELECT * FROM t1 WHERE a=0 AND b=0; */
    testcase( (p_ಪದ->wtFlags & ಪದ_CODED)!=0 );
    testcase( p_ಪದ->wtFlags & ಪದ_VIRTUAL );
    r1 = ಸಂಕೇತ_Equality_ಪದ(pParse, p_ಪದ, pLevel, j, bRev, reg_ತಳ+j);
    ಆದರೆ( r1!=reg_ತಳ+j ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Copy, r1, reg_ತಳ+j);
    }
    ಆದರೆ( p_ಪದ->eOperator & WO_ಒಳ ){
      ಆದರೆ( p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ->flags & EP_xIsSelect ){
        /* No affinity ever needs to be (or should be) applied to a value
        ** from the RHS of an "? IN (SELECT ...)" expression. The
        ** sqlite3FindInIndex() routine has already ensured that the
        ** affinity of the comparison has been applied to the value.  */
        ಆದರೆ( zAff ) zAff[j] = SQLITE_AFF_BLOB;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (p_ಪದ->eOperator & WO_ISNULL)==0 ){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
      ಆದರೆ( (p_ಪದ->wtFlags & ಪದ_IS)==0 && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(p_ಬಲ) ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, reg_ತಳ+j, pLevel->addrBrk);
        VdbeCoverage(v);
      }
      ಆದರೆ( pParse->nErr==0 ){
        ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed==0 );
        ಆದರೆ( sqlite3CompareAffinity(p_ಬಲ, zAff[j])==SQLITE_AFF_BLOB ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
        ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Needs_No_Affinity_Change(p_ಬಲ, zAff[j]) ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
      }
    }
  }
  *pzAff = zAff;
  ಹಿಂತಿರುಗಿ reg_ತಳ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_LIKE_DOESNT_ಹೊಂದಿಕೆ_BLOBS
/*
** If the most recently coded instruction is a constant range constraint
** (a string literal) that originated from the LIKE optimization, then
** set P3 and P5 on the OP_String opcode so that the string will be cast
** to a BLOB at appropriate times.
**
** The LIKE optimization trys to evaluate "x LIKE 'abc%'" as a range
** expression: "x>='ABC' AND x<'abd'".  But this requires that the range
** scan loop run twice, once for strings and a second time for BLOBs.
** The OP_String opcodes on the second pass convert the upper and lower
** bound string constants to blobs.  This routine makes the necessary changes
** to the OP_String opcodes for that to happen.
**
** Except, of course, if SQLITE_LIKE_DOESNT_MATCH_BLOBS is defined, then
** only the one pass through the string space is required, so this routine
** becomes a no-op.
*/
ಸ್ಥಿರ ಶೂನ್ಯ whereLikeOptimizationStringFixup(
  Vdbe *v,                /* prepared statement under construction */
  WhereLevel *pLevel,     /* The loop that contains the LIKE operator */
  Where_ಪದ *p_ಪದ        /* The upper or lower bound just coded */
){
  ಆದರೆ( p_ಪದ->wtFlags & ಪದ_LIKEOPT ){
    VdbeOp *pOp;
    ಪ್ರತಿಜ್ಞಾ( pLevel->iLikeRepCntr>0 );
    pOp = sqlite3_Vdbe_Get_ಕೊನೆಯ_Op(v);
    ಪ್ರತಿಜ್ಞಾ( pOp!=0 );
    ಪ್ರತಿಜ್ಞಾ( pOp->opcode==OP_String8
            || p_ಪದ->pWC->pWInfo->pParse->db->mallocFailed );
    pOp->p3 = (ಪೂರ್ಣಾಂಕ)(pLevel->iLikeRepCntr>>1);  /* Register holding counter */
    pOp->p5 = (u8)(pLevel->iLikeRepCntr&1);    /* ASC or DESC */
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define whereLikeOptimizationStringFixup(A,B,C)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_CURSOR_HINTS
/*
** Information is passed from codeCursorHint() down to individual nodes of
** the expression tree (by sqlite3WalkExpr()) using an instance of this
** structure.
*/
ರಚನೆ CCurHint {
  ಪೂರ್ಣಾಂಕ iTabCur;    /* Cursor for the main table */
  ಪೂರ್ಣಾಂಕ iIdxCur;    /* Cursor for the index, if pIdx!=0.  Unused otherwise */
  ಸೂಚ್ಯಂಕ *pIdx;    /* The index used to access the table */
};

/*
** This function is called for every node of an expression that is a candidate
** for a cursor hint on an index cursor.  For TK_COLUMN nodes that reference
** the table CCurHint.iTabCur, verify that the same column can be
** accessed through the index.  If it cannot, then set pWalker->eCode to 1.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_Cursor_Hint_Check_ಅಭಿವ್ಯಕ್ತಿ(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ರಚನೆ CCurHint *pHint = pWalker->u.pCCurHint;
  ಪ್ರತಿಜ್ಞಾ( pHint->pIdx!=0 );
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ
   && p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ==pHint->iTabCur
   && sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pHint->pIdx, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ)<0
  ){
    pWalker->e_ಸಂಕೇತ = 1;
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/*
** Test whether or not expression pExpr, which was part of a WHERE clause,
** should be included in the cursor-hint for a table that is on the rhs
** of a LEFT JOIN. Set Walker.eCode to non-zero before returning if the
** expression is not suitable.
**
** An expression is unsuitable if it might evaluate to non NULL even if
** a TK_COLUMN node that does affect the value of the expression is set
** to NULL. For example:
**
**   col IS NULL
**   col IS NOT NULL
**   coalesce(col, 1)
**   CASE WHEN col THEN 0 ELSE 1 END
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_Cursor_Hint_Is_Or_Function(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS
   || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNULL || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNOT
   || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NOTNULL || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_CASE
  ){
    pWalker->e_ಸಂಕೇತ = 1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION ){
    ಪೂರ್ಣಾಂಕ d1;
    ಅಕ್ಷರ d2[4];
    ಆದರೆ( 0==sqlite3IsLikeFunction(pWalker->pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ, &d1, d2) ){
      pWalker->e_ಸಂಕೇತ = 1;
    }
  }

  ಹಿಂತಿರುಗಿ WRC_Continue;
}


/*
** This function is called on every node of an expression tree used as an
** argument to the OP_CursorHint instruction. If the node is a TK_COLUMN
** that accesses any table other than the one identified by
** CCurHint.iTabCur, then do the following:
**
**   1) allocate a register and code an OP_Column instruction to read
**      the specified column into the new register, and
**
**   2) transform the expression node to a TK_REGISTER node that reads
**      from the newly populated register.
**
** Also, if the node is a TK_COLUMN that does access the table identified
** by pCCurHint.iTabCur, and an index is being used (which we will
** know because CCurHint.pIdx!=0) then transform the TK_COLUMN into
** an access of the index rather than the original table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_Cursor_Hint_Fix_ಅಭಿವ್ಯಕ್ತಿ(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಂಕ rc = WRC_Continue;
  ಪೂರ್ಣಾಂಕ reg;
  ರಚನೆ CCurHint *pHint = pWalker->u.pCCurHint;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ ){
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ!=pHint->iTabCur ){
      reg = ++pWalker->pParse->nMem;   /* Register for column value */
      reg = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pWalker->pParse, p_ಅಭಿವ್ಯಕ್ತಿ, reg);
      p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_ರಿಜಿಸ್ಟರ್;
      p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = reg;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pHint->pIdx!=0 ){
      p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = pHint->iIdxCur;
      p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pHint->pIdx, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ);
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ>=0 );
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo ){
    rc = WRC_Prune;
    reg = ++pWalker->pParse->nMem;   /* Register for column value */
    reg = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಗುರಿ(pWalker->pParse, p_ಅಭಿವ್ಯಕ್ತಿ, reg);
    p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_ರಿಜಿಸ್ಟರ್;
    p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = reg;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_TRUEFALSE ){
    /* Do not walk disabled expressions.  tag-20230504-1 */
    ಹಿಂತಿರುಗಿ WRC_Prune;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Insert an OP_CursorHint instruction if it is appropriate to do so.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಸಂಕೇತ_Cursor_Hint(
  Src_ಅಂಶ *p_Tab_ಅಂಶ,  /* FROM clause item */
  WhereInfo *pWInfo,    /* The where clause */
  WhereLevel *pLevel,   /* Which loop to provide hints for */
  Where_ಪದ *p_ಕೊನೆ_Range  /* Hint this end-of-scan boundary term if not NULL */
){
  Parse *pParse = pWInfo->pParse;
  sqlite3 *db = pParse->db;
  Vdbe *v = pParse->pVdbe;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = 0;
  WhereLoop *pLoop = pLevel->pWLoop;
  ಪೂರ್ಣಾಂಕ iCur;
  WhereClause *pWC;
  Where_ಪದ *p_ಪದ;
  ಪೂರ್ಣಾಂಕ i, j;
  ರಚನೆ CCurHint sHint;
  Walker sWalker;

  ಆದರೆ( OptimizationDisabled(db, SQLITE_CursorHints) ) ಹಿಂತಿರುಗಿ;
  iCur = pLevel->iTabCur;
  ಪ್ರತಿಜ್ಞಾ( iCur==pWInfo->pTabList->a[pLevel->iFrom].iCursor );
  sHint.iTabCur = iCur;
  sHint.iIdxCur = pLevel->iIdxCur;
  sHint.pIdx = pLoop->u.btree.p_ಸೂಚ್ಯಂಕ;
  memset(&sWalker, 0, ಗಾತ್ರ(sWalker));
  sWalker.pParse = pParse;
  sWalker.u.pCCurHint = &sHint;
  pWC = &pWInfo->sWC;
  ಗಾಗಿ(i=0; i<pWC->n_ತಳ; i++){
    p_ಪದ = &pWC->a[i];
    ಆದರೆ( p_ಪದ->wtFlags & (ಪದ_VIRTUAL|ಪದ_CODED) ) ಮುಂದುವರಿ;
    ಆದರೆ( p_ಪದ->prereqAll & pLevel->notReady ) ಮುಂದುವರಿ;

    /* Any terms specified as part of the ON(...) clause for any LEFT
    ** JOIN for which the current table is not the rhs are omitted
    ** from the cursor-hint.
    **
    ** If this table is the rhs of a LEFT JOIN, "IS" or "IS NULL" terms
    ** that were specified as part of the WHERE clause must be excluded.
    ** This is to address the following:
    **
    **   SELECT ... t1 LEFT JOIN t2 ON (t1.a=t2.b) WHERE t2.c IS NULL;
    **
    ** Say there is a single row in t2 that matches (t1.a=t2.b), but its
    ** t2.c values is not NULL. If the (t2.c IS NULL) constraint is
    ** pushed down to the cursor, this row is filtered out, causing
    ** SQLite to synthesize a row of NULL values. Which does match the
    ** WHERE clause, and so the query returns a row. Which is incorrect.
    **
    ** For the same reason, WHERE terms such as:
    **
    **   WHERE 1 = (t2.c IS NULL)
    **
    ** are also excluded. See codeCursorHintIsOrFunction() for details.
    */
    ಆದರೆ( p_Tab_ಅಂಶ->fg.jointype & JT_ಎಡ ){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ;
      ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON)
       || p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin!=p_Tab_ಅಂಶ->iCursor
      ){
        sWalker.e_ಸಂಕೇತ = 0;
        sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಸಂಕೇತ_Cursor_Hint_Is_Or_Function;
        sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ);
        ಆದರೆ( sWalker.e_ಸಂಕೇತ ) ಮುಂದುವರಿ;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) ) ಮುಂದುವರಿ;
    }

    /* All terms in pWLoop->aLTerm[] except pEndRange are used to initialize
    ** the cursor.  These terms are not needed as hints for a pure range
    ** scan (that has no == terms) so omit them. */
    ಆದರೆ( pLoop->u.btree.nEq==0 && p_ಪದ!=p_ಕೊನೆ_Range ){
      ಗಾಗಿ(j=0; j<pLoop->nLTerm && pLoop->aLTerm[j]!=p_ಪದ; j++){}
      ಆದರೆ( j<pLoop->nLTerm ) ಮುಂದುವರಿ;
    }

    /* No subqueries or non-deterministic functions allowed */
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಒಳಗೊಂಡಿದೆ_Subquery(p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ) ) ಮುಂದುವರಿ;

    /* For an index scan, make sure referenced columns are actually in
    ** the index. */
    ಆದರೆ( sHint.pIdx!=0 ){
      sWalker.e_ಸಂಕೇತ = 0;
      sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಸಂಕೇತ_Cursor_Hint_Check_ಅಭಿವ್ಯಕ್ತಿ;
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ);
      ಆದರೆ( sWalker.e_ಸಂಕೇತ ) ಮುಂದುವರಿ;
    }

    /* If we survive all prior tests, that means this term is worth hinting */
    p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ, 0));
  }
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 ){
    sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = ಸಂಕೇತ_Cursor_Hint_Fix_ಅಭಿವ್ಯಕ್ತಿ;
    ಆದರೆ( pParse->nErr==0 ) sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_CursorHint,
                      (sHint.pIdx ? sHint.iIdxCur : sHint.iTabCur), 0, 0,
                      (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*)p_ಅಭಿವ್ಯಕ್ತಿ, P4_ಅಭಿವ್ಯಕ್ತಿ);
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define ಸಂಕೇತ_Cursor_Hint(A,B,C,D)  /* No-op */
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_CURSOR_HINTS */

/*
** Cursor iCur is open on an intkey b-tree (a table). Register iRowid contains
** a rowid value just read from cursor iIdxCur, open on index pIdx. This
** function generates code to do a deferred seek of cursor iCur to the
** rowid stored in register iRowid.
**
** Normally, this is just:
**
**   OP_DeferredSeek $iCur $iRowid
**
** Which causes a seek on $iCur to the row with rowid $iRowid.
**
** However, if the scan currently being coded is a branch of an OR-loop and
** the statement currently being coded is a SELECT, then additional information
** is added that might allow OP_Column to omit the seek and instead do its
** lookup on the index, thus avoiding an expensive seek operation.  To
** enable this optimization, the P3 of OP_DeferredSeek is set to iIdxCur
** and P4 is set to an array of integers containing one entry for each column
** in the table.  For each table column, if the column is the i'th
** column of the index, then the corresponding array entry is set to (i+1).
** If the column does not appear in the index at all, the array entry is set
** to 0.  The OP_Column opcode can check this array to see if the column it
** wants is in the index and if it is, it will substitute the index cursor
** and column number and continue with those new values, rather than seeking
** the table cursor.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಸಂಕೇತ_Deferred_Seek(
  WhereInfo *pWInfo,              /* Where clause context */
  ಸೂಚ್ಯಂಕ *pIdx,                    /* Index scan is using */
  ಪೂರ್ಣಾಂಕ iCur,                       /* Cursor for IPK b-tree */
  ಪೂರ್ಣಾಂಕ iIdxCur                     /* Index cursor */
){
  Parse *pParse = pWInfo->pParse; /* Parse context */
  Vdbe *v = pParse->pVdbe;        /* Vdbe to generate code within */

  ಪ್ರತಿಜ್ಞಾ( iIdxCur>0 );
  ಪ್ರತಿಜ್ಞಾ( pIdx->ai_ಸ್ತಂಭ[pIdx->n_ಸ್ತಂಭ-1]==-1 );

  pWInfo->bDeferredSeek = 1;
  sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_DeferredSeek, iIdxCur, 0, iCur);
  ಆದರೆ( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_ಬಲ_JOIN))
   && Db_ಮುಖವಾಡ_All_Zero(sqlite3ParseToplevel(pParse)->write_ಮುಖವಾಡ)
  ){
    ಪೂರ್ಣಾಂಕ i;
    ಕೋಷ್ಟಕ *pTab = pIdx->p_ಕೋಷ್ಟಕ;
    u32 *ai = (u32*)sqlite3DbMallocZero(pParse->db, ಗಾತ್ರ(u32)*(pTab->n_ಲಂಬಸಾಲು+1));
    ಆದರೆ( ai ){
      ai[0] = pTab->n_ಲಂಬಸಾಲು;
      ಗಾಗಿ(i=0; i<pIdx->n_ಸ್ತಂಭ-1; i++){
        ಪೂರ್ಣಾಂಕ x1, x2;
        ಪ್ರತಿಜ್ಞಾ( pIdx->ai_ಸ್ತಂಭ[i]<pTab->n_ಲಂಬಸಾಲು );
        x1 = pIdx->ai_ಸ್ತಂಭ[i];
        x2 = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, x1);
        testcase( x1!=x2 );
        ಆದರೆ( x1>=0 ) ai[x2+1] = i+1;
      }
      sqlite3VdbeChangeP4(v, -1, (ಅಕ್ಷರ*)ai, P4_INTARRAY);
    }
  }
}

/*
** If the expression passed as the second argument is a vector, generate
** code to write the first nReg elements of the vector into an array
** of registers starting with iReg.
**
** If the expression is not a vector, then nReg must be passed 1. In
** this case, generate code to evaluate the expression and leave the
** result in register iReg.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_Or_ಸದಿಶ(Parse *pParse, ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಂಕ iReg, ಪೂರ್ಣಾಂಕ nReg){
  ಪ್ರತಿಜ್ಞಾ( nReg>0 );
  ಆದರೆ( p && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಸದಿಶ(p) ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p) ){
      Vdbe *v = pParse->pVdbe;
      ಪೂರ್ಣಾಂಕ iSelect;
      ಪ್ರತಿಜ್ಞಾ( p->op==TK_SELECT );
      iSelect = sqlite3_ಸಂಕೇತ_Subselect(pParse, p);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Copy, iSelect, iReg, nReg-1);
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
      ಪೂರ್ಣಾಂಕ i;
      ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *pList;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) );
      pList = p->x.pList;
      ಪ್ರತಿಜ್ಞಾ( nReg<=pList->n_ಅಭಿವ್ಯಕ್ತಿ );
      ಗಾಗಿ(i=0; i<nReg; i++){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, iReg+i);
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( nReg==1 || pParse->nErr );
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, p, iReg);
  }
}

/*
** The pTruth expression is always true because it is the WHERE clause
** a partial index that is driving a query loop.  Look through all of the
** WHERE clause terms on the query, and if any of those terms must be
** true because pTruth is true, then mark those WHERE clause terms as
** coded.
*/
ಸ್ಥಿರ ಶೂನ್ಯ where_Apply_Partial_ಸೂಚ್ಯಂಕ_Constraints(
  ಅಭಿವ್ಯಕ್ತಿ *pTruth,
  ಪೂರ್ಣಾಂಕ iTabCur,
  WhereClause *pWC
){
  ಪೂರ್ಣಾಂಕ i;
  Where_ಪದ *p_ಪದ;
  ಆಗ( pTruth->op==TK_AND ){
    where_Apply_Partial_ಸೂಚ್ಯಂಕ_Constraints(pTruth->p_ಎಡ, iTabCur, pWC);
    pTruth = pTruth->p_ಬಲ;
  }
  ಗಾಗಿ(i=0, p_ಪದ=pWC->a; i<pWC->n_ಪದ; i++, p_ಪದ++){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ;
    ಆದರೆ( p_ಪದ->wtFlags & ಪದ_CODED ) ಮುಂದುವರಿ;
    p_ಅಭಿವ್ಯಕ್ತಿ = p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ;
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, p_ಅಭಿವ್ಯಕ್ತಿ, pTruth, iTabCur)==0 ){
      p_ಪದ->wtFlags |= ಪದ_CODED;
    }
  }
}

/*
** This routine is called right after An OP_Filter has been generated and
** before the corresponding index search has been performed.  This routine
** checks to see if there are additional Bloom filters in inner loops that
** can be checked prior to doing the index lookup.  If there are available
** inner-loop Bloom filters, then evaluate those filters now, before the
** index lookup.  The idea is that a Bloom filter check is way faster than
** an index lookup, and the Bloom filter might return false, meaning that
** the index lookup can be skipped.
**
** We know that an inner loop uses a Bloom filter because it has the
** WhereLevel.regFilter set.  If an inner-loop Bloom filter is checked,
** then clear the WhereLevel.regFilter value to prevent the Bloom filter
** from being checked a second time when the inner loop is evaluated.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ filter_Pull_ಕೆಳಕ್ಕೆ(
  Parse *pParse,       /* Parsing context */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  ಪೂರ್ಣಾಂಕ iLevel,          /* Which level of pWInfo->a[] should be coded */
  ಪೂರ್ಣಾಂಕ addrNxt,         /* Jump here to bypass inner loops */
  Bitmask notReady     /* Loops that are not ready */
){
  ಪೂರ್ಣಾಂಕ saved_addrBrk;
  ಆಗ( ++iLevel < pWInfo->nLevel ){
    WhereLevel *pLevel = &pWInfo->a[iLevel];
    WhereLoop *pLoop = pLevel->pWLoop;
    ಆದರೆ( pLevel->regFilter==0 ) ಮುಂದುವರಿ;
    ಆದರೆ( pLevel->pWLoop->nSkip ) ಮುಂದುವರಿ;
    /*         ,--- Because sqlite3ConstructBloomFilter() has will not have set
    **  vvvvv--'    pLevel->regFilter if this were true. */
    ಆದರೆ( NEVER(pLoop->prereq & notReady) ) ಮುಂದುವರಿ;
    saved_addrBrk = pLevel->addrBrk;
    pLevel->addrBrk = addrNxt;
    ಆದರೆ( pLoop->wsFlags & WHERE_IPK ){
      Where_ಪದ *p_ಪದ = pLoop->aLTerm[0];
      ಪೂರ್ಣಾಂಕ regRowid;
      ಪ್ರತಿಜ್ಞಾ( p_ಪದ!=0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
      testcase( p_ಪದ->wtFlags & ಪದ_VIRTUAL );
      regRowid = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
      regRowid = ಸಂಕೇತ_Equality_ಪದ(pParse, p_ಪದ, pLevel, 0, 0, regRowid);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(pParse->pVdbe, OP_MustBeInt, regRowid, addrNxt);
      VdbeCoverage(pParse->pVdbe);
      ಪ್ರತಿಜ್ಞಾ( sqlite3WhereLoopBloomable(pLoop) );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(pParse->pVdbe, OP_Filter, pLevel->regFilter,
                           addrNxt, regRowid, 1);
      VdbeCoverage(pParse->pVdbe);
    }ಇಲ್ಲದಿದ್ದರೆ{
      u16 nEq = pLoop->u.btree.nEq;
      ಪೂರ್ಣಾಂಕ r1;
      ಅಕ್ಷರ *z_ಆರಂಭ_Aff;

      ಪ್ರತಿಜ್ಞಾ( pLoop->wsFlags & WHERE_ಸೂಚ್ಯಂಕಿತ );
      ಪ್ರತಿಜ್ಞಾ( (pLoop->wsFlags & WHERE_ಸ್ತಂಭ_ಒಳ)==0 );
      r1 = ಸಂಕೇತ_All_Equality_Terms(pParse,pLevel,0,0,&z_ಆರಂಭ_Aff);
      ಸಂಕೇತ_Apply_Affinity(pParse, r1, nEq, z_ಆರಂಭ_Aff);
      sqlite3DbFree(pParse->db, z_ಆರಂಭ_Aff);
      ಪ್ರತಿಜ್ಞಾ( sqlite3WhereLoopBloomable(pLoop) );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(pParse->pVdbe, OP_Filter, pLevel->regFilter,
                           addrNxt, r1, nEq);
      VdbeCoverage(pParse->pVdbe);
    }
    pLevel->regFilter = 0;
    pLevel->addrBrk = saved_addrBrk;
  }
}

/*
** Loop pLoop is a WHERE_INDEXED level that uses at least one IN(...)
** operator. Return true if level pLoop is guaranteed to visit only one
** row for each key generated for the index.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ where_Loop_Is_One_ಅಡ್ಡಸಾಲು(WhereLoop *pLoop){
  ಆದರೆ( pLoop->u.btree.p_ಸೂಚ್ಯಂಕ->onError
   && pLoop->nSkip==0
   && pLoop->u.btree.nEq==pLoop->u.btree.p_ಸೂಚ್ಯಂಕ->n_ಕೀಲಿ_ಲಂಬಸಾಲು
  ){
    ಪೂರ್ಣಾಂಕ ii;
    ಗಾಗಿ(ii=0; ii<pLoop->u.btree.nEq; ii++){
      ಆದರೆ( pLoop->aLTerm[ii]->eOperator & (WO_IS|WO_ISNULL) ){
        ಹಿಂತಿರುಗಿ 0;
      }
    }
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** This is called while coding loop pLevel, which scans FROM clause item
** pTabItem. pE is an expression for which all the prerequisites are
** available. This function tests if pE can be coded as part of the current
** loop, or whether it needs to be deferred to ensure outer joins are
** processed correctly. This function returns non-zero if the expression
** can be coded as part of the loop, or 0 if it must be deferred.
**
** The expression must be deferred if:
**
**   * There are any RIGHT joins in the FROM clause and the expression
**     was not part of an ON clause, or was part of an ON clause to the 
**     right of the join.
**
**   * The table is the RHS of a LEFT JOIN and the expression was not
**     part of an ON clause on an OUTER join, or was part of an ON clause 
**     on an OUTER join to the right of the join.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ where_ಅಭಿವ್ಯಕ್ತಿ_Is_Ready(
  WhereInfo *pWInfo, 
  WhereLevel *pLevel,
  Src_ಅಂಶ *p_Tab_ಅಂಶ, 
  ಅಭಿವ್ಯಕ್ತಿ *pE
){
  u8 jtype = p_Tab_ಅಂಶ->fg.jointype;
  ಆದರೆ( jtype & (JT_ಎಡ|JT_LTORJ|JT_ಬಲ) ){
    ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pE,EP_OuterON|EP_InnerON) ){
      /* Defer processing WHERE clause constraints until after outer
      ** join processing.  tag-20220513a */
      ಹಿಂತಿರುಗಿ 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (jtype & JT_ಎಡ) && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pE,EP_OuterON) ){
      ಹಿಂತಿರುಗಿ 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      Bitmask m = sqlite3_Where_Get_ಮುಖವಾಡ(&pWInfo->s_ಮುಖವಾಡ_ಗಣ, pE->w.iJoin);
      ಆದರೆ( m & pLevel->notReady ){
        /* An ON clause that is not ripe */
        ಹಿಂತಿರುಗಿ 0;
      }
    }
  }
  ಹಿಂತಿರುಗಿ 1;
}

/*
** Find the best address for an inner loop to jump to if its join
** constraint matches no rows.
**
** This is an optimization.  The default behavior is to continue with
** the inner-most of the outer loops.  But if the join constraint is not
** influenced by the inner-most outer loop, we might as well skip that
** loop too, and continue with the second inner-most outer loop.  And
** so forth through subsequent outer loops.
**
** Return 0 to use the default behavior (continue with the inner-most
** outer loop.  Or return an optimized address to jump to instead if
** we can do better.
**
** Conditions for returning non-zero:
**
**    1.   The SQLITE_JoinMiss optimization must be enabled.
**
**    2.   The iLevel loop must have at least two outer loops, otherwise
**         this optimization is pointless.
**
**    3.   We are not attempting to do one-pass
**
**    4.   The join constraints may not contain subqueries
**
** If the inner-most outer loop that influences iLevel is also the
** only loop that influences iLevel, and if that loop uses an
** automatic index, then also arrange to delete the currently used
** entry from that automatic index, since we know that it can never
** be successful.
**
** This optimization inspired by the paper:  "TreeTracker Join: Simple,
** Optimal, Fast" by Hu, Wang, and Miranker, 2025.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕಂಡುಹಿಡಿ_No_ಹೊಂದಿಕೆ_Continuation(
  WhereInfo *pWInfo,         /* WHERE clause */
  ಪೂರ್ಣಾಂಕ iLevel                 /* Which loop we are talking about */
){
  WhereLoop *pLoop;          /* Description of this loop */
  WhereLevel *p_ಪೋಷಕ;       /* Controlling parent loop */
  SrcList *pTabList;         /* Complete FROM clause */
  Bitmask m_ಕೀಲಿ;              /* Mask of dependent tables */
  ಪೂರ್ಣಾಂಕ i_ಪೋಷಕ;               /* Inner-most loop affecting the key to iLevel */
  ಪೂರ್ಣಾಂಕ i;                     /* Loop counter */

  ಆದರೆ( iLevel<2 ){
    /* If there are less than two outer loops, this optimization is no help */
    ಹಿಂತಿರುಗಿ 0;
  }

  /* The OP_SeekGE and OP_SeekLE opcodes only work right for this
  ** optimization on a pure equality constraint, so skip the optimization
  ** for anything else.
  */
  pLoop = pWInfo->a[iLevel].pWLoop;
  ಆದರೆ( (pLoop->wsFlags & WHERE_CONSTRAINT)!=WHERE_ಸ್ತಂಭ_EQ ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( pLoop->wsFlags & WHERE_SKIPSCAN ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( pLoop->wsFlags & WHERE_BIGNULL_ವಿಂಗಡಿಸು ) ಹಿಂತಿರುಗಿ 0;
  ಪ್ರತಿಜ್ಞಾ( (pLoop->wsFlags & (WHERE_VIRTUALTABLE|WHERE_MULTI_OR))==0 );
  ಪ್ರತಿಜ್ಞಾ( pLoop->u.btree.nEq>0 );

  /* Find a mask of all outer tables that influence the key.  At the same
  ** time, check to see if any keys values are computed from subqueries and
  ** fail out (return 0) if there are any. */
  m_ಕೀಲಿ = 0;
  ಗಾಗಿ(i=0; i<pLoop->nLTerm; i++){
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಒಳಗೊಂಡಿದೆ_Subquery(pLoop->aLTerm[i]->p_ಅಭಿವ್ಯಕ್ತಿ) ){
      ಹಿಂತಿರುಗಿ 0;
    }
    m_ಕೀಲಿ |= pLoop->aLTerm[i]->prereq_ಬಲ;
  }

  /* Look back through outer loops.  Find the inner-most outer loop
  ** that influences the key. Make iParent be the index of that inner-most
  ** outer loop.
  */
  pTabList = pWInfo->pTabList;
  i_ಪೋಷಕ = iLevel-1;
  ಆಗ( i_ಪೋಷಕ>0 && (pWInfo->a[i_ಪೋಷಕ].pWLoop->ಮುಖವಾಡ_Self & m_ಕೀಲಿ)==0 ){
    i_ಪೋಷಕ--;
  }
  ಪ್ರತಿಜ್ಞಾ( i_ಪೋಷಕ>=0 && i_ಪೋಷಕ<=iLevel );

  /* Everything needs to be an INNER join.  Fail if there are any
  ** OUTER joins in between iParent and iLevel. */
  ಗಾಗಿ(i=i_ಪೋಷಕ; i<=iLevel; i++){
    ಆದರೆ( pTabList->a[pWInfo->a[i].iFrom].fg.jointype & JT_OUTER ) ಹಿಂತಿರುಗಿ 0;
  }

  /* If we reach this point, that means the conditions of the optimization
  ** have been met.  Even so, disallow the optimization if it is disabled.
  */
  ಆದರೆ( OptimizationDisabled(pWInfo->pParse->db, SQLITE_Join_ತಪ್ಪಿದ) ){
    ಹಿಂತಿರುಗಿ 0;
  }

  /* If the inner-most outer loop that influences the key for iLevel
  ** is controlled by an automatic index and if the automatic index is
  ** the only outer loop that influences the key, then arrange to delete
  ** the automatic index entry if there is no match on the key.
  */
  p_ಪೋಷಕ = &pWInfo->a[i_ಪೋಷಕ];
  ಆದರೆ( (p_ಪೋಷಕ->pWLoop->wsFlags & WHERE_AUTO_ಸೂಚ್ಯಂಕ)!=0
   && (m_ಕೀಲಿ & (m_ಕೀಲಿ-1))==0
  ){
    ಗಾಗಿ(i=0; i<pLoop->nLTerm; i++){
      ಅಭಿವ್ಯಕ್ತಿ *pX = pLoop->aLTerm[i]->p_ಅಭಿವ್ಯಕ್ತಿ;
      ಆದರೆ( !sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಕೋಷ್ಟಕ_Constant(pX->p_ಬಲ,p_ಪೋಷಕ->iTabCur,0) ) ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( i==pLoop->nLTerm ){    
      ಆದರೆ( p_ಪೋಷಕ->addrDeleteCont==0 ){
        p_ಪೋಷಕ->addrDeleteCont = sqlite3VdbeMakeLabel(pWInfo->pParse);
      }
      ಹಿಂತಿರುಗಿ p_ಪೋಷಕ->addrDeleteCont;
    }
  }

  testcase( i_ಪೋಷಕ<iLevel-1 );
  ಹಿಂತಿರುಗಿ p_ಪೋಷಕ->addrCont;
}

/*
** Generate code for the start of the iLevel-th loop in the WHERE clause
** implementation described by pWInfo.
*/
Bitmask sqlite3_Where_ಸಂಕೇತ_One_Loop_ಆರಂಭ(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  ಪೂರ್ಣಾಂಕ iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
){
  ಪೂರ್ಣಾಂಕ j, k;            /* Loop counters */
  ಪೂರ್ಣಾಂಕ iCur;            /* The VDBE cursor for the table */
  ಪೂರ್ಣಾಂಕ addrNxt;         /* Where to jump to continue with the next IN case */
  ಪೂರ್ಣಾಂಕ bRev;            /* True if we need to scan in reverse order */
  WhereLoop *pLoop;    /* The WhereLoop object being coded */
  WhereClause *pWC;    /* Decomposition of the entire WHERE clause */
  Where_ಪದ *p_ಪದ;         /* A WHERE clause term */
  sqlite3 *db;              /* Database connection */
  Src_ಅಂಶ *p_Tab_ಅಂಶ;        /* FROM clause term being coded */
  ಪೂರ್ಣಾಂಕ addrBrk;              /* Jump here to break out of the loop */
  ಪೂರ್ಣಾಂಕ addrCont;             /* Jump here to continue with next cycle */
  ಪೂರ್ಣಾಂಕ addr_ತಪ್ಪಿದ;             /* addrCont enhanced for no-matching rows */
  ಪೂರ್ಣಾಂಕ iRowidReg = 0;        /* Rowid is stored in this register, if not zero */
  ಪೂರ್ಣಾಂಕ iReleaseReg = 0;      /* Temp register to free before returning */
  ಸೂಚ್ಯಂಕ *pIdx = 0;          /* Index used by loop (if any) */
  ಪೂರ್ಣಾಂಕ iLoop;                /* Iteration of constraint generator loop */

  pWC = &pWInfo->sWC;
  db = pParse->db;
  pLoop = pLevel->pWLoop;
  p_Tab_ಅಂಶ = &pWInfo->pTabList->a[pLevel->iFrom];
  iCur = p_Tab_ಅಂಶ->iCursor;
  pLevel->notReady = notReady & ~sqlite3_Where_Get_ಮುಖವಾಡ(&pWInfo->s_ಮುಖವಾಡ_ಗಣ, iCur);
  bRev = (pWInfo->rev_ಮುಖವಾಡ>>iLevel)&1;
  VdbeModuleComment((v, "Begin WHERE-loop%d: %s",
                     iLevel, p_Tab_ಅಂಶ->pSTab->zName));
#ಆದರೆ WHERETRACE_ENABLED /* 0x4001 */
  ಆದರೆ( sqlite3WhereTrace & 0x1 ){
    sqlite3DebugPrintf("Coding level %d of %d:  notReady=%llx  iFrom=%d\n",
       iLevel, pWInfo->nLevel, (u64)notReady, pLevel->iFrom);
    ಆದರೆ( sqlite3WhereTrace & 0x1000 ){
      sqlite3WhereLoopPrint(pLoop, pWC);
    }
  }
  ಆದರೆ( (sqlite3WhereTrace & 0x4001)==0x4001 ){
    ಆದರೆ( iLevel==0 ){
      sqlite3DebugPrintf("WHERE clause being coded:\n");
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(0, pWInfo->pWhere, 0);
    }
    sqlite3DebugPrintf("All WHERE-clause terms before coding:\n");
    sqlite3WhereClausePrint(pWC);
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Create labels for the "break" and "continue" instructions
  ** for the current loop.  Jump to addrBrk to break out of a loop.
  ** Jump to cont to go immediately to the next iteration of the
  ** loop.
  **
  ** When there is an IN operator, we also have a "addrNxt" label that
  ** means to continue with the next IN value combination.  When
  ** there are no IN operators in the constraints, the "addrNxt" label
  ** is the same as "addrBrk".
  */
  addrBrk = pLevel->addrNxt = pLevel->addrBrk;
  addrCont = pLevel->addrCont = sqlite3VdbeMakeLabel(pParse);
  addr_ತಪ್ಪಿದ = ಕಂಡುಹಿಡಿ_No_ಹೊಂದಿಕೆ_Continuation(pWInfo, iLevel);

  /* If this is the right table of a LEFT OUTER JOIN, allocate and
  ** initialize a memory cell that records if this table matches any
  ** row of the left table of the join.
  */
  ಪ್ರತಿಜ್ಞಾ( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_ಬಲ_JOIN))
       || pLevel->iFrom>0 || (p_Tab_ಅಂಶ[0].fg.jointype & JT_ಎಡ)==0
  );
  ಆದರೆ( pLevel->iFrom>0 && (p_Tab_ಅಂಶ[0].fg.jointype & JT_ಎಡ)!=0 ){
    pLevel->i_ಎಡ_Join = ++pParse->nMem;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, pLevel->i_ಎಡ_Join);
    VdbeComment((v, "init LEFT JOIN match flag"));
    ಆದರೆ( p_Tab_ಅಂಶ->fg.ಮೂಲಕ_Coroutine ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null_ಅಡ್ಡಸಾಲು, p_Tab_ಅಂಶ->iCursor, 1);
    }
  }

  /* Special case of a FROM clause subquery implemented as a co-routine */
  ಆದರೆ( p_Tab_ಅಂಶ->fg.ಮೂಲಕ_Coroutine ){
    ಪೂರ್ಣಾಂಕ regYield;
    Subquery *pSubq;
    ಪ್ರತಿಜ್ಞಾ( p_Tab_ಅಂಶ->fg.isSubquery && p_Tab_ಅಂಶ->u4.pSubq!=0 );
    pSubq = p_Tab_ಅಂಶ->u4.pSubq;
    regYield = pSubq->regReturn;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    pLevel->p2 =  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Yield, regYield, addrBrk);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", p_Tab_ಅಂಶ->pSTab->zName));
    pLevel->op = OP_Goto;
  }ಇಲ್ಲದಿದ್ದರೆ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
  ಆದರೆ(  (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
    /* Case 1:  The table is a virtual-table.  Use the VFilter and VNext
    **          to access the data.
    */
    ಪೂರ್ಣಾಂಕ iReg;   /* P3 Value for OP_VFilter */
    ಪೂರ್ಣಾಂಕ addr_Not_ಸಿಕ್ಕಿತು;
    ಪೂರ್ಣಾಂಕ nConstraint = pLoop->nLTerm;

    iReg = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, nConstraint+2);
    addr_Not_ಸಿಕ್ಕಿತು = pLevel->addrBrk;
    ಗಾಗಿ(j=0; j<nConstraint; j++){
      ಪೂರ್ಣಾಂಕ i_ಗುರಿ = iReg+j+2;
      p_ಪದ = pLoop->aLTerm[j];
      ಆದರೆ( NEVER(p_ಪದ==0) ) ಮುಂದುವರಿ;
      ಆದರೆ( p_ಪದ->eOperator & WO_ಒಳ ){
        ಆದರೆ( SMASKBIT32(j) & pLoop->u.vtab.m_Handle_ಒಳ ){
          ಪೂರ್ಣಾಂಕ iTab = pParse->nTab++;
          ಪೂರ್ಣಾಂಕ iCache = ++pParse->nMem;
          sqlite3_ಸಂಕೇತ_Rhs_ದ_ಒಳ(pParse, p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ, iTab, 0);
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_VInit_ಒಳ, iTab, i_ಗುರಿ, iCache);
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಸಂಕೇತ_Equality_ಪದ(pParse, p_ಪದ, pLevel, j, bRev, i_ಗುರಿ);
          addr_Not_ಸಿಕ್ಕಿತು = pLevel->addrNxt;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_Or_ಸದಿಶ(pParse, p_ಬಲ, i_ಗುರಿ, 1);
        ಆದರೆ( p_ಪದ->e_ಹೊಂದಿಕೆ_Op==SQLITE_ಸೂಚ್ಯಂಕ_CONSTRAINT_ಸ್ಥಾನಾಂತರ
         && pLoop->u.vtab.b_Omit_ಸ್ಥಾನಾಂತರ
        ){
          ಪ್ರತಿಜ್ಞಾ( p_ಪದ->eOperator==WO_AUX );
          ಪ್ರತಿಜ್ಞಾ( pWInfo->pSelect!=0 );
          ಪ್ರತಿಜ್ಞಾ( pWInfo->pSelect->i_ಸ್ಥಾನಾಂತರ>0 );
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, pWInfo->pSelect->i_ಸ್ಥಾನಾಂತರ);
          VdbeComment((v,"Zero OFFSET counter"));
        }
      }
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, pLoop->u.vtab.idx_ಅಂಕೆ, iReg);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, nConstraint, iReg+1);
    /* The instruction immediately prior to OP_VFilter must be an OP_Integer
    ** that sets the "argc" value for xVFilter.  This is necessary for
    ** resolveP2() to work correctly.  See tag-20250207a. */
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_VFilter, iCur, addr_Not_ಸಿಕ್ಕಿತು, iReg,
                      pLoop->u.vtab.idxStr,
                      pLoop->u.vtab.needFree ? P4_DYNAMIC : P4_STATIC);
    VdbeCoverage(v);
    pLoop->u.vtab.needFree = 0;
    /* An OOM inside of AddOp4(OP_VFilter) instruction above might have freed
    ** the u.vtab.idxStr.  NULL it out to prevent a use-after-free */
    ಆದರೆ( db->mallocFailed ) pLoop->u.vtab.idxStr = 0;
    pLevel->p1 = iCur;
    pLevel->op = pWInfo->eOnePass ? OP_Noop : OP_VNext;
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);
    ಪ್ರತಿಜ್ಞಾ( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );

    ಗಾಗಿ(j=0; j<nConstraint; j++){
      p_ಪದ = pLoop->aLTerm[j];
      ಆದರೆ( j<16 && (pLoop->u.vtab.omit_ಮುಖವಾಡ>>j)&1 ){
        disable_ಪದ(pLevel, p_ಪದ);
        ಮುಂದುವರಿ;
      }
      ಆದರೆ( (p_ಪದ->eOperator & WO_ಒಳ)!=0
       && (SMASKBIT32(j) & pLoop->u.vtab.m_Handle_ಒಳ)==0
       && !db->mallocFailed
      ){
        ಅಭಿವ್ಯಕ್ತಿ *pCompare;  /* The comparison operator */
        ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ;    /* RHS of the comparison */
        VdbeOp *pOp;     /* Opcode to access the value of the IN constraint */
        ಪೂರ್ಣಾಂಕ i_ಒಳ;         /* IN loop corresponding to the j-th constraint */

        /* Reload the constraint value into reg[iReg+j+2].  The same value
        ** was loaded into the same register prior to the OP_VFilter, but
        ** the xFilter implementation might have changed the datatype or
        ** encoding of the value in the register, so it *must* be reloaded.
        */
        ಗಾಗಿ(i_ಒಳ=0; ALWAYS(i_ಒಳ<pLevel->u.ಒಳ.n_ಒಳ); i_ಒಳ++){
          pOp = sqlite3VdbeGetOp(v, pLevel->u.ಒಳ.a_ಒಳ_Loop[i_ಒಳ].addr_ಒಳ_ತುದಿ);
          ಆದರೆ( (pOp->opcode==OP_ಸ್ತಂಭ && pOp->p3==iReg+j+2)
           || (pOp->opcode==OP_Rowid && pOp->p2==iReg+j+2)
          ){
            testcase( pOp->opcode==OP_Rowid );
            sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, pOp->opcode, pOp->p1, pOp->p2, pOp->p3);
            ನಿಲ್ಲಿಸು;
          }
        }

        /* Generate code that will continue to the next row if
        ** the IN constraint is not satisfied
        */
        pCompare = sqlite3PExpr(pParse, TK_EQ, 0, 0);
        ಆದರೆ( !db->mallocFailed ){
          ಪೂರ್ಣಾಂಕ iFld = p_ಪದ->u.x.i_ಕ್ಷೇತ್ರ;
          ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
          ಪ್ರತಿಜ್ಞಾ( p_ಎಡ!=0 );
          ಆದರೆ( iFld>0 ){
            ಪ್ರತಿಜ್ಞಾ( p_ಎಡ->op==TK_ಸದಿಶ );
            ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಎಡ) );
            ಪ್ರತಿಜ್ಞಾ( iFld<=p_ಎಡ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ );
            pCompare->p_ಎಡ = p_ಎಡ->x.pList->a[iFld-1].p_ಅಭಿವ್ಯಕ್ತಿ;
          }ಇಲ್ಲದಿದ್ದರೆ{
            pCompare->p_ಎಡ = p_ಎಡ;
          }
          pCompare->p_ಬಲ = p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ರಿಜಿಸ್ಟರ್, 0);
          ಆದರೆ( p_ಬಲ ){
            p_ಬಲ->i_ಕೋಷ್ಟಕ = iReg+j+2;
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(
                pParse, pCompare, pLevel->addrCont, SQLITE_JUMPIFNULL
            );
          }
          pCompare->p_ಎಡ = 0;
        }
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pCompare);
      }
    }

    /* These registers need to be preserved in case there is an IN operator
    ** loop.  So we could deallocate the registers here (and potentially
    ** reuse them later) if (pLoop->wsFlags & WHERE_IN_ABLE)==0.  But it seems
    ** simpler and safer to simply not reuse the registers.
    **
    **    sqlite3ReleaseTempRange(pParse, iReg, nConstraint+2);
    */
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_VIRTUALTABLE */

  ಆದರೆ( (pLoop->wsFlags & WHERE_IPK)!=0
   && (pLoop->wsFlags & (WHERE_ಸ್ತಂಭ_ಒಳ|WHERE_ಸ್ತಂಭ_EQ))!=0
  ){
    /* Case 2:  We can directly reference a single row using an
    **          equality comparison against the ROWID field.  Or
    **          we reference multiple rows using a "rowid IN (...)"
    **          construct.
    */
    ಪ್ರತಿಜ್ಞಾ( pLoop->u.btree.nEq==1 );
    p_ಪದ = pLoop->aLTerm[0];
    ಪ್ರತಿಜ್ಞಾ( p_ಪದ!=0 );
    ಪ್ರತಿಜ್ಞಾ( p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
    testcase( p_ಪದ->wtFlags & ಪದ_VIRTUAL );
    iReleaseReg = ++pParse->nMem;
    iRowidReg = ಸಂಕೇತ_Equality_ಪದ(pParse, p_ಪದ, pLevel, 0, bRev, iReleaseReg);
    ಆದರೆ( iRowidReg!=iReleaseReg ) sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, iReleaseReg);
    addrNxt = pLevel->addrNxt;
    ಆದರೆ( addr_ತಪ್ಪಿದ==0 ) addr_ತಪ್ಪಿದ = addrNxt;
    ಆದರೆ( pLevel->regFilter ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_MustBeInt, iRowidReg, addr_ತಪ್ಪಿದ);
      VdbeCoverage(v);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Filter, pLevel->regFilter, addr_ತಪ್ಪಿದ,
                           iRowidReg, 1);
      VdbeCoverage(v);
      filter_Pull_ಕೆಳಕ್ಕೆ(pParse, pWInfo, iLevel, addrNxt, notReady);
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_SeekRowid, iCur, addr_ತಪ್ಪಿದ, iRowidReg);
    VdbeCoverage(v);
    pLevel->op = OP_Noop;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (pLoop->wsFlags & WHERE_IPK)!=0
         && (pLoop->wsFlags & WHERE_ಸ್ತಂಭ_RANGE)!=0
  ){
    /* Case 3:  We have an inequality comparison against the ROWID field.
    */
    ಪೂರ್ಣಾಂಕ testOp = OP_Noop;
    ಪೂರ್ಣಾಂಕ ಆರಂಭ;
    ಪೂರ್ಣಾಂಕ mem_ಕೊನೆ_ಮೌಲ್ಯ = 0;
    Where_ಪದ *p_ಆರಂಭ, *p_ಕೊನೆ;

    j = 0;
    p_ಆರಂಭ = p_ಕೊನೆ = 0;
    ಆದರೆ( pLoop->wsFlags & WHERE_BTM_ಎಲ್ಲೆ ) p_ಆರಂಭ = pLoop->aLTerm[j++];
    ಆದರೆ( pLoop->wsFlags & WHERE_ತುದಿ_ಎಲ್ಲೆ ) p_ಕೊನೆ = pLoop->aLTerm[j++];
    ಪ್ರತಿಜ್ಞಾ( p_ಆರಂಭ!=0 || p_ಕೊನೆ!=0 );
    ಆದರೆ( bRev ){
      p_ಪದ = p_ಆರಂಭ;
      p_ಆರಂಭ = p_ಕೊನೆ;
      p_ಕೊನೆ = p_ಪದ;
    }
    ಸಂಕೇತ_Cursor_Hint(p_Tab_ಅಂಶ, pWInfo, pLevel, p_ಕೊನೆ);
    ಆದರೆ( p_ಆರಂಭ ){
      ಅಭಿವ್ಯಕ್ತಿ *pX;             /* The expression that defines the start bound */
      ಪೂರ್ಣಾಂಕ r1, r_ತಾತ್ಕಾಲಿಕ;        /* Registers for holding the start boundary */
      ಪೂರ್ಣಾಂಕ op;               /* Cursor seek operation */

      /* The following constant maps TK_xx codes into corresponding
      ** seek opcodes.  It depends on a particular ordering of TK_xx
      */
      ಸ್ಥಿರಾಂಕ u8 aMoveOp[] = {
           /* TK_GT */  OP_SeekGT,
           /* TK_LE */  OP_SeekLE,
           /* TK_LT */  OP_SeekLT,
           /* TK_GE */  OP_SeekGE
      };
      ಪ್ರತಿಜ್ಞಾ( TK_LE==TK_GT+1 );      /* Make sure the ordering.. */
      ಪ್ರತಿಜ್ಞಾ( TK_LT==TK_GT+2 );      /*  ... of the TK_xx values... */
      ಪ್ರತಿಜ್ಞಾ( TK_GE==TK_GT+3 );      /*  ... is correct. */

      ಪ್ರತಿಜ್ಞಾ( (p_ಆರಂಭ->wtFlags & ಪದ_VNULL)==0 );
      testcase( p_ಆರಂಭ->wtFlags & ಪದ_VIRTUAL );
      pX = p_ಆರಂಭ->p_ಅಭಿವ್ಯಕ್ತಿ;
      ಪ್ರತಿಜ್ಞಾ( pX!=0 );
      testcase( p_ಆರಂಭ->ಎಡ_Cursor!=iCur ); /* transitive constraints */
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಸದಿಶ(pX->p_ಬಲ) ){
        r1 = r_ತಾತ್ಕಾಲಿಕ = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
        ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_Or_ಸದಿಶ(pParse, pX->p_ಬಲ, r1, 1);
        testcase( pX->op==TK_GT );
        testcase( pX->op==TK_GE );
        testcase( pX->op==TK_LT );
        testcase( pX->op==TK_LE );
        op = aMoveOp[((pX->op - TK_GT - 1) & 0x3) | 0x1];
        ಪ್ರತಿಜ್ಞಾ( pX->op!=TK_GT || op==OP_SeekGE );
        ಪ್ರತಿಜ್ಞಾ( pX->op!=TK_GE || op==OP_SeekGE );
        ಪ್ರತಿಜ್ಞಾ( pX->op!=TK_LT || op==OP_SeekLE );
        ಪ್ರತಿಜ್ಞಾ( pX->op!=TK_LE || op==OP_SeekLE );
      }ಇಲ್ಲದಿದ್ದರೆ{
        r1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ತಾತ್ಕಾಲಿಕ(pParse, pX->p_ಬಲ, &r_ತಾತ್ಕಾಲಿಕ);
        disable_ಪದ(pLevel, p_ಆರಂಭ);
        op = aMoveOp[(pX->op - TK_GT)];
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, op, iCur, addrBrk, r1);
      VdbeComment((v, "pk"));
      VdbeCoverageIf(v, pX->op==TK_GT);
      VdbeCoverageIf(v, pX->op==TK_LE);
      VdbeCoverageIf(v, pX->op==TK_LT);
      VdbeCoverageIf(v, pX->op==TK_GE);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r_ತಾತ್ಕಾಲಿಕ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, bRev ? OP_ಕೊನೆಯ : OP_Rewind, iCur, pLevel->addrHalt);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
    }
    ಆದರೆ( p_ಕೊನೆ ){
      ಅಭಿವ್ಯಕ್ತಿ *pX;
      pX = p_ಕೊನೆ->p_ಅಭಿವ್ಯಕ್ತಿ;
      ಪ್ರತಿಜ್ಞಾ( pX!=0 );
      ಪ್ರತಿಜ್ಞಾ( (p_ಕೊನೆ->wtFlags & ಪದ_VNULL)==0 );
      testcase( p_ಕೊನೆ->ಎಡ_Cursor!=iCur ); /* Transitive constraints */
      testcase( p_ಕೊನೆ->wtFlags & ಪದ_VIRTUAL );
      mem_ಕೊನೆ_ಮೌಲ್ಯ = ++pParse->nMem;
      ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_Or_ಸದಿಶ(pParse, pX->p_ಬಲ, mem_ಕೊನೆ_ಮೌಲ್ಯ, 1);
      ಆದರೆ( 0==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಸದಿಶ(pX->p_ಬಲ)
       && (pX->op==TK_LT || pX->op==TK_GT)
      ){
        testOp = bRev ? OP_Le : OP_Ge;
      }ಇಲ್ಲದಿದ್ದರೆ{
        testOp = bRev ? OP_Lt : OP_Gt;
      }
      ಆದರೆ( 0==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಸದಿಶ(pX->p_ಬಲ) ){
        disable_ಪದ(pLevel, p_ಕೊನೆ);
      }
    }
    ಆರಂಭ = sqlite3VdbeCurrentAddr(v);
    pLevel->op = bRev ? OP_Prev : OP_ಮುಂದಿನ;
    pLevel->p1 = iCur;
    pLevel->p2 = ಆರಂಭ;
    ಪ್ರತಿಜ್ಞಾ( pLevel->p5==0 );
    ಆದರೆ( testOp!=OP_Noop ){
      iRowidReg = ++pParse->nMem;
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rowid, iCur, iRowidReg);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, testOp, mem_ಕೊನೆ_ಮೌಲ್ಯ, addrBrk, iRowidReg);
      VdbeCoverageIf(v, testOp==OP_Le);
      VdbeCoverageIf(v, testOp==OP_Lt);
      VdbeCoverageIf(v, testOp==OP_Ge);
      VdbeCoverageIf(v, testOp==OP_Gt);
      sqlite3VdbeChangeP5(v, SQLITE_AFF_NUMERIC | SQLITE_JUMPIFNULL);
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pLoop->wsFlags & WHERE_ಸೂಚ್ಯಂಕಿತ ){
    /* Case 4: Search using an index.
    **
    ** The WHERE clause may contain zero or more equality
    ** terms ("==" or "IN" or "IS" operators) that refer to the N
    ** left-most columns of the index. It may also contain
    ** inequality constraints (>, <, >= or <=) on the indexed
    ** column that immediately follows the N equalities. Only
    ** the right-most column can be an inequality - the rest must
    ** use the "==", "IN", or "IS" operators. For example, if the
    ** index is on (x,y,z), then the following clauses are all
    ** optimized:
    **
    **    x=5
    **    x=5 AND y=10
    **    x=5 AND y<10
    **    x=5 AND y>5 AND y<10
    **    x=5 AND y=5 AND z<=10
    **
    ** The z<10 term of the following cannot be used, only
    ** the x=5 term:
    **
    **    x=5 AND z<10
    **
    ** N may be zero if there are inequality constraints.
    ** If there are no inequality constraints, then N is at
    ** least one.
    **
    ** This case is also used when there are no WHERE clause
    ** constraints but an index is selected anyway, in order
    ** to force the output order to conform to an ORDER BY.
    */
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 a_ಆರಂಭ_Op[] = {
      0,
      0,
      OP_Rewind,           /* 2: (!start_constraints && startEq &&  !bRev) */
      OP_ಕೊನೆಯ,             /* 3: (!start_constraints && startEq &&   bRev) */
      OP_SeekGT,           /* 4: (start_constraints  && !startEq && !bRev) */
      OP_SeekLT,           /* 5: (start_constraints  && !startEq &&  bRev) */
      OP_SeekGE,           /* 6: (start_constraints  &&  startEq && !bRev) */
      OP_SeekLE            /* 7: (start_constraints  &&  startEq &&  bRev) */
    };
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 a_ಕೊನೆ_Op[] = {
      OP_IdxGE,            /* 0: (end_constraints && !bRev && !endEq) */
      OP_IdxGT,            /* 1: (end_constraints && !bRev &&  endEq) */
      OP_IdxLE,            /* 2: (end_constraints &&  bRev && !endEq) */
      OP_IdxLT,            /* 3: (end_constraints &&  bRev &&  endEq) */
    };
    u16 nEq = pLoop->u.btree.nEq;     /* Number of == or IN terms */
    u16 nBtm = pLoop->u.btree.nBtm;   /* Length of BTM vector */
    u16 n_ತುದಿ = pLoop->u.btree.n_ತುದಿ;   /* Length of TOP vector */
    ಪೂರ್ಣಾಂಕ reg_ತಳ;                 /* Base register holding constraint values */
    Where_ಪದ *p_Range_ಆರಂಭ = 0;  /* Inequality constraint at range start */
    Where_ಪದ *p_Range_ಕೊನೆ = 0;    /* Inequality constraint at range end */
    ಪೂರ್ಣಾಂಕ ಆರಂಭ_Eq;                 /* True if range start uses ==, >= or <= */
    ಪೂರ್ಣಾಂಕ ಕೊನೆ_Eq;                   /* True if range end uses ==, >= or <= */
    ಪೂರ್ಣಾಂಕ ಆರಂಭ_constraints;       /* Start of range is constrained */
    ಪೂರ್ಣಾಂಕ nConstraint;             /* Number of constraint terms */
    ಪೂರ್ಣಾಂಕ iIdxCur;                 /* The VDBE cursor for the index */
    ಪೂರ್ಣಾಂಕ nExtraReg = 0;           /* Number of extra registers needed */
    ಪೂರ್ಣಾಂಕ op;                      /* Instruction opcode */
    ಅಕ್ಷರ *z_ಆರಂಭ_Aff;             /* Affinity for start of range constraint */
    ಅಕ್ಷರ *z_ಕೊನೆ_Aff = 0;           /* Affinity for end of range constraint */
    u8 bSeekPastNull = 0;        /* True to seek past initial nulls */
    u8 bStopAtNull = 0;          /* Add condition to terminate at NULLs */
    ಪೂರ್ಣಾಂಕ omit_ಕೋಷ್ಟಕ;               /* True if we use the index only */
    ಪೂರ್ಣಾಂಕ regBignull = 0;          /* big-null flag register */
    ಪೂರ್ಣಾಂಕ addr_Seek_ಪರಿಶೀಲನೆ = 0;        /* Opcode of the OP_SeekScan, if any */

    pIdx = pLoop->u.btree.p_ಸೂಚ್ಯಂಕ;
    iIdxCur = pLevel->iIdxCur;
    ಪ್ರತಿಜ್ಞಾ( nEq>=pLoop->nSkip );

    /* Find any inequality constraint terms for the start and end
    ** of the range.
    */
    j = nEq;
    ಆದರೆ( pLoop->wsFlags & WHERE_BTM_ಎಲ್ಲೆ ){
      p_Range_ಆರಂಭ = pLoop->aLTerm[j++];
      nExtraReg = MAX(nExtraReg, pLoop->u.btree.nBtm);
      /* Like optimization range constraints always occur in pairs */
      ಪ್ರತಿಜ್ಞಾ( (p_Range_ಆರಂಭ->wtFlags & ಪದ_LIKEOPT)==0 ||
              (pLoop->wsFlags & WHERE_ತುದಿ_ಎಲ್ಲೆ)!=0 );
    }
    ಆದರೆ( pLoop->wsFlags & WHERE_ತುದಿ_ಎಲ್ಲೆ ){
      p_Range_ಕೊನೆ = pLoop->aLTerm[j++];
      nExtraReg = MAX(nExtraReg, pLoop->u.btree.n_ತುದಿ);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_LIKE_DOESNT_ಹೊಂದಿಕೆ_BLOBS
      ಆದರೆ( (p_Range_ಕೊನೆ->wtFlags & ಪದ_LIKEOPT)!=0 ){
        ಪ್ರತಿಜ್ಞಾ( p_Range_ಆರಂಭ!=0 );                     /* LIKE opt constraints */
        ಪ್ರತಿಜ್ಞಾ( p_Range_ಆರಂಭ->wtFlags & ಪದ_LIKEOPT );   /* occur in pairs */
        pLevel->iLikeRepCntr = (u32)++pParse->nMem;
        /* iLikeRepCntr actually stores 2x the counter register number.  The
        ** bottom bit indicates whether the search order is ASC or DESC. */
        testcase( bRev );
        testcase( pIdx->a_ವಿಂಗಡಿಸು_ಕ್ರಮ[nEq]==SQLITE_SO_DESC );
        ಪ್ರತಿಜ್ಞಾ( (bRev & ~1)==0 );
        pLevel->iLikeRepCntr <<=1;
        pLevel->iLikeRepCntr |= bRev ^ (pIdx->a_ವಿಂಗಡಿಸು_ಕ್ರಮ[nEq]==SQLITE_SO_DESC);
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಆದರೆ( p_Range_ಆರಂಭ==0 ){
        j = pIdx->ai_ಸ್ತಂಭ[nEq];
        ಆದರೆ( (j>=0 && pIdx->p_ಕೋಷ್ಟಕ->a_ಲಂಬಸಾಲು[j].notNull==0) || j==XN_ಅಭಿವ್ಯಕ್ತಿ ){
          bSeekPastNull = 1;
        }
      }
    }
    ಪ್ರತಿಜ್ಞಾ( p_Range_ಕೊನೆ==0 || (p_Range_ಕೊನೆ->wtFlags & ಪದ_VNULL)==0 );

    /* If the WHERE_BIGNULL_SORT flag is set, then index column nEq uses
    ** a non-default "big-null" sort (either ASC NULLS LAST or DESC NULLS
    ** FIRST). In both cases separate ordered scans are made of those
    ** index entries for which the column is null and for those for which
    ** it is not. For an ASC sort, the non-NULL entries are scanned first.
    ** For DESC, NULL entries are scanned first.
    */
    ಆದರೆ( (pLoop->wsFlags & (WHERE_ತುದಿ_ಎಲ್ಲೆ|WHERE_BTM_ಎಲ್ಲೆ))==0
     && (pLoop->wsFlags & WHERE_BIGNULL_ವಿಂಗಡಿಸು)!=0
    ){
      ಪ್ರತಿಜ್ಞಾ( bSeekPastNull==0 && nExtraReg==0 && nBtm==0 && n_ತುದಿ==0 );
      ಪ್ರತಿಜ್ಞಾ( p_Range_ಕೊನೆ==0 && p_Range_ಆರಂಭ==0 );
      testcase( pLoop->nSkip>0 );
      nExtraReg = 1;
      bSeekPastNull = 1;
      pLevel->regBignull = regBignull = ++pParse->nMem;
      ಆದರೆ( pLevel->i_ಎಡ_Join ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, regBignull);
      }
      pLevel->addrBignull = sqlite3VdbeMakeLabel(pParse);
    }

    /* If we are doing a reverse order scan on an ascending index, or
    ** a forward order scan on a descending index, interchange the
    ** start and end terms (pRangeStart and pRangeEnd).
    */
    ಆದರೆ( (nEq<pIdx->n_ಸ್ತಂಭ && bRev==(pIdx->a_ವಿಂಗಡಿಸು_ಕ್ರಮ[nEq]==SQLITE_SO_ASC)) ){
      ಅದಲುಬದಲು(Where_ಪದ *, p_Range_ಕೊನೆ, p_Range_ಆರಂಭ);
      ಅದಲುಬದಲು(u8, bSeekPastNull, bStopAtNull);
      ಅದಲುಬದಲು(u8, nBtm, n_ತುದಿ);
    }

    ಆದರೆ( iLevel>0 && (pLoop->wsFlags & WHERE_ಒಳ_SEEKSCAN)!=0 ){
      /* In case OP_SeekScan is used, ensure that the index cursor does not
      ** point to a valid row for the first iteration of this loop. */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Null_ಅಡ್ಡಸಾಲು, iIdxCur);
    }

    /* Generate code to evaluate all constraint terms using == or IN
    ** and store the values of those terms in an array of registers
    ** starting at regBase.
    */
    ಸಂಕೇತ_Cursor_Hint(p_Tab_ಅಂಶ, pWInfo, pLevel, p_Range_ಕೊನೆ);
    reg_ತಳ = ಸಂಕೇತ_All_Equality_Terms(pParse,pLevel,bRev,nExtraReg,&z_ಆರಂಭ_Aff);
    ಪ್ರತಿಜ್ಞಾ( z_ಆರಂಭ_Aff==0 || sqlite3Strlen30(z_ಆರಂಭ_Aff)>=nEq );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_LIKE_DOESNT_ಹೊಂದಿಕೆ_BLOBS
    ಆದರೆ( pLevel->iLikeRepCntr ){
      /* The LIKE optimization loop must be the innermost of any IN(...)
      ** or skip-scan loops. Otherwise rows may not be visited in index 
      ** order. This means it needs an addrNxt label for the same reason
      ** IN(...) scans do.  */
      ಆದರೆ( pLevel->addrNxt==pLevel->addrBrk ){
        pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 1, (ಪೂರ್ಣಾಂಕ)(pLevel->iLikeRepCntr>>1));
      VdbeComment((v, "LIKE loop counter"));
      pLevel->addrLikeRep = sqlite3VdbeCurrentAddr(v);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( z_ಆರಂಭ_Aff && n_ತುದಿ ){
      z_ಕೊನೆ_Aff = sqlite3DbStrDup(db, &z_ಆರಂಭ_Aff[nEq]);
    }
    addrNxt = (regBignull ? pLevel->addrBignull : pLevel->addrNxt);
    ಆದರೆ( addr_ತಪ್ಪಿದ==0 ) addr_ತಪ್ಪಿದ = addrNxt;

    testcase( p_Range_ಆರಂಭ && (p_Range_ಆರಂಭ->eOperator & WO_LE)!=0 );
    testcase( p_Range_ಆರಂಭ && (p_Range_ಆರಂಭ->eOperator & WO_GE)!=0 );
    testcase( p_Range_ಕೊನೆ && (p_Range_ಕೊನೆ->eOperator & WO_LE)!=0 );
    testcase( p_Range_ಕೊನೆ && (p_Range_ಕೊನೆ->eOperator & WO_GE)!=0 );
    ಆರಂಭ_Eq = !p_Range_ಆರಂಭ || p_Range_ಆರಂಭ->eOperator & (WO_LE|WO_GE);
    ಕೊನೆ_Eq =   !p_Range_ಕೊನೆ || p_Range_ಕೊನೆ->eOperator & (WO_LE|WO_GE);
    ಆರಂಭ_constraints = p_Range_ಆರಂಭ || nEq>0;

    /* Seek the index cursor to the start of the range. */
    nConstraint = nEq;
    ಆದರೆ( p_Range_ಆರಂಭ ){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = p_Range_ಆರಂಭ->p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
      ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_Or_ಸದಿಶ(pParse, p_ಬಲ, reg_ತಳ+nEq, nBtm);
      whereLikeOptimizationStringFixup(v, pLevel, p_Range_ಆರಂಭ);
      ಆದರೆ( (p_Range_ಆರಂಭ->wtFlags & ಪದ_VNULL)==0
       && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(p_ಬಲ)
      ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, reg_ತಳ+nEq, addrNxt);
        VdbeCoverage(v);
      }
      ಆದರೆ( z_ಆರಂಭ_Aff ){
        updateRangeAffinityStr(p_ಬಲ, nBtm, &z_ಆರಂಭ_Aff[nEq]);
      }
      nConstraint += nBtm;
      testcase( p_Range_ಆರಂಭ->wtFlags & ಪದ_VIRTUAL );
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಸದಿಶ(p_ಬಲ)==0 ){
        disable_ಪದ(pLevel, p_Range_ಆರಂಭ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆರಂಭ_Eq = 1;
      }
      bSeekPastNull = 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( bSeekPastNull ){
      ಆರಂಭ_Eq = 0;
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, reg_ತಳ+nEq);
      ಆರಂಭ_constraints = 1;
      nConstraint++;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( regBignull ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, reg_ತಳ+nEq);
      ಆರಂಭ_constraints = 1;
      nConstraint++;
    }
    ಸಂಕೇತ_Apply_Affinity(pParse, reg_ತಳ, nConstraint - bSeekPastNull, z_ಆರಂಭ_Aff);
    ಆದರೆ( pLoop->nSkip>0 && nConstraint==pLoop->nSkip ){
      /* The skip-scan logic inside the call to codeAllEqualityConstraints()
      ** above has already left the cursor sitting on the correct row,
      ** so no further seeking is needed */
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( regBignull ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 1, regBignull);
        VdbeComment((v, "NULL-scan pass ctr"));
      }
      ಆದರೆ( pLevel->regFilter ){
        ಪ್ರತಿಜ್ಞಾ( sqlite3WhereLoopBloomable(pLoop) );
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Filter, pLevel->regFilter, addr_ತಪ್ಪಿದ,
                             reg_ತಳ, nEq);
        VdbeCoverage(v);
        filter_Pull_ಕೆಳಕ್ಕೆ(pParse, pWInfo, iLevel, addrNxt, notReady);
      }

      op = a_ಆರಂಭ_Op[(ಆರಂಭ_constraints<<2) + (ಆರಂಭ_Eq<<1) + bRev];
      ಪ್ರತಿಜ್ಞಾ( op!=0 );
      ಆದರೆ( (pLoop->wsFlags & WHERE_ಒಳ_SEEKSCAN)!=0 && op==OP_SeekGE ){
        ಪ್ರತಿಜ್ಞಾ( regBignull==0 );
        /* TUNING:  The OP_SeekScan opcode seeks to reduce the number
        ** of expensive seek operations by replacing a single seek with
        ** 1 or more step operations.  The question is, how many steps
        ** should we try before giving up and going with a seek.  The cost
        ** of a seek is proportional to the logarithm of the of the number
        ** of entries in the tree, so basing the number of steps to try
        ** on the estimated number of rows in the btree seems like a good
        ** guess. */
        addr_Seek_ಪರಿಶೀಲನೆ = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Seek_ಪರಿಶೀಲನೆ,
                                         (pIdx->ai_ಅಡ್ಡಸಾಲು_Log_Est[0]+9)/10);
        ಆದರೆ( p_Range_ಆರಂಭ || p_Range_ಕೊನೆ ){
          sqlite3VdbeChangeP5(v, 1);
          sqlite3VdbeChangeP2(v, addr_Seek_ಪರಿಶೀಲನೆ, sqlite3VdbeCurrentAddr(v)+1);
          addr_Seek_ಪರಿಶೀಲನೆ = 0;
        }
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, op, iIdxCur, addr_ತಪ್ಪಿದ, reg_ತಳ, nConstraint);
      VdbeCoverage(v);
      VdbeCoverageIf(v, op==OP_Rewind);  testcase( op==OP_Rewind );
      VdbeCoverageIf(v, op==OP_ಕೊನೆಯ);    testcase( op==OP_ಕೊನೆಯ );
      VdbeCoverageIf(v, op==OP_SeekGT);  testcase( op==OP_SeekGT );
      VdbeCoverageIf(v, op==OP_SeekGE);  testcase( op==OP_SeekGE );
      VdbeCoverageIf(v, op==OP_SeekLE);  testcase( op==OP_SeekLE );
      VdbeCoverageIf(v, op==OP_SeekLT);  testcase( op==OP_SeekLT );

      ಪ್ರತಿಜ್ಞಾ( bSeekPastNull==0 || bStopAtNull==0 );
      ಆದರೆ( regBignull ){
        ಪ್ರತಿಜ್ಞಾ( bSeekPastNull==1 || bStopAtNull==1 );
        ಪ್ರತಿಜ್ಞಾ( bSeekPastNull==!bStopAtNull );
        ಪ್ರತಿಜ್ಞಾ( bStopAtNull==ಆರಂಭ_Eq );
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        op = a_ಆರಂಭ_Op[(nConstraint>1)*4 + 2 + bRev];
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, op, iIdxCur, addrNxt, reg_ತಳ,
                             nConstraint-ಆರಂಭ_Eq);
        VdbeCoverage(v);
        VdbeCoverageIf(v, op==OP_Rewind);  testcase( op==OP_Rewind );
        VdbeCoverageIf(v, op==OP_ಕೊನೆಯ);    testcase( op==OP_ಕೊನೆಯ );
        VdbeCoverageIf(v, op==OP_SeekGE);  testcase( op==OP_SeekGE );
        VdbeCoverageIf(v, op==OP_SeekLE);  testcase( op==OP_SeekLE );
        ಪ್ರತಿಜ್ಞಾ( op==OP_Rewind || op==OP_ಕೊನೆಯ || op==OP_SeekGE || op==OP_SeekLE);
      }
    }

    /* Load the value for the inequality constraint at the end of the
    ** range (if any).
    */
    nConstraint = nEq;
    ಪ್ರತಿಜ್ಞಾ( pLevel->p2==0 );
    ಆದರೆ( p_Range_ಕೊನೆ ){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = p_Range_ಕೊನೆ->p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
      ಪ್ರತಿಜ್ಞಾ( addr_Seek_ಪರಿಶೀಲನೆ==0 );
      ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_Or_ಸದಿಶ(pParse, p_ಬಲ, reg_ತಳ+nEq, n_ತುದಿ);
      whereLikeOptimizationStringFixup(v, pLevel, p_Range_ಕೊನೆ);
      ಆದರೆ( (p_Range_ಕೊನೆ->wtFlags & ಪದ_VNULL)==0
       && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(p_ಬಲ)
      ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, reg_ತಳ+nEq, addrNxt);
        VdbeCoverage(v);
      }
      ಆದರೆ( z_ಕೊನೆ_Aff ){
        updateRangeAffinityStr(p_ಬಲ, n_ತುದಿ, z_ಕೊನೆ_Aff);
        ಸಂಕೇತ_Apply_Affinity(pParse, reg_ತಳ+nEq, n_ತುದಿ, z_ಕೊನೆ_Aff);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed );
      }
      nConstraint += n_ತುದಿ;
      testcase( p_Range_ಕೊನೆ->wtFlags & ಪದ_VIRTUAL );

      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಸದಿಶ(p_ಬಲ)==0 ){
        disable_ಪದ(pLevel, p_Range_ಕೊನೆ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಕೊನೆ_Eq = 1;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( bStopAtNull ){
      ಆದರೆ( regBignull==0 ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, reg_ತಳ+nEq);
        ಕೊನೆ_Eq = 0;
      }
      nConstraint++;
    }
    ಆದರೆ( z_ಆರಂಭ_Aff ) sqlite3DbNNFreeNN(db, z_ಆರಂಭ_Aff);
    ಆದರೆ( z_ಕೊನೆ_Aff ) sqlite3DbNNFreeNN(db, z_ಕೊನೆ_Aff);

    /* Top of the loop body */
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    /* Check if the index cursor is past the end of the range. */
    ಆದರೆ( nConstraint ){
      ಆದರೆ( regBignull ){
        /* Except, skip the end-of-range check while doing the NULL-scan */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IfNot, regBignull, sqlite3VdbeCurrentAddr(v)+3);
        VdbeComment((v, "If NULL-scan 2nd pass"));
        VdbeCoverage(v);
      }
      op = a_ಕೊನೆ_Op[bRev*2 + ಕೊನೆ_Eq];
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, op, iIdxCur, addrNxt, reg_ತಳ, nConstraint);
      testcase( op==OP_IdxGT );  VdbeCoverageIf(v, op==OP_IdxGT );
      testcase( op==OP_IdxGE );  VdbeCoverageIf(v, op==OP_IdxGE );
      testcase( op==OP_IdxLT );  VdbeCoverageIf(v, op==OP_IdxLT );
      testcase( op==OP_IdxLE );  VdbeCoverageIf(v, op==OP_IdxLE );
      ಆದರೆ( addr_Seek_ಪರಿಶೀಲನೆ ) sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr_Seek_ಪರಿಶೀಲನೆ);
    }
    ಆದರೆ( regBignull ){
      /* During a NULL-scan, check to see if we have reached the end of
      ** the NULLs */
      ಪ್ರತಿಜ್ಞಾ( bSeekPastNull==!bStopAtNull );
      ಪ್ರತಿಜ್ಞಾ( bSeekPastNull+bStopAtNull==1 );
      ಪ್ರತಿಜ್ಞಾ( nConstraint+bSeekPastNull>0 );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_If, regBignull, sqlite3VdbeCurrentAddr(v)+2);
      VdbeComment((v, "If NULL-scan 1st pass"));
      VdbeCoverage(v);
      op = a_ಕೊನೆ_Op[bRev*2 + bSeekPastNull];
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, op, iIdxCur, addrNxt, reg_ತಳ,
                           nConstraint+bSeekPastNull);
      testcase( op==OP_IdxGT );  VdbeCoverageIf(v, op==OP_IdxGT );
      testcase( op==OP_IdxGE );  VdbeCoverageIf(v, op==OP_IdxGE );
      testcase( op==OP_IdxLT );  VdbeCoverageIf(v, op==OP_IdxLT );
      testcase( op==OP_IdxLE );  VdbeCoverageIf(v, op==OP_IdxLE );
    }

    ಆದರೆ( (pLoop->wsFlags & WHERE_ಒಳ_EARLYOUT)!=0 ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Seek_ದೊರಕಿತು, iIdxCur, nEq, nEq);
    }

    /* Seek the table cursor, if required */
    omit_ಕೋಷ್ಟಕ = (pLoop->wsFlags & WHERE_IDX_ONLY)!=0
           && (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_ಬಲ_JOIN))==0;
    ಆದರೆ( omit_ಕೋಷ್ಟಕ ){
      /* pIdx is a covering index.  No need to access the main table. */
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( HasRowid(pIdx->p_ಕೋಷ್ಟಕ) ){
      ಸಂಕೇತ_Deferred_Seek(pWInfo, pIdx, iCur, iIdxCur);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( iCur!=iIdxCur ){
      ಸೂಚ್ಯಂಕ *pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pIdx->p_ಕೋಷ್ಟಕ);
      iRowidReg = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು);
      ಗಾಗಿ(j=0; j<pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು; j++){
        k = sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pIdx, pPk->ai_ಸ್ತಂಭ[j]);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, iIdxCur, k, iRowidReg+j);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Not_ಸಿಕ್ಕಿತು, iCur, addrCont,
                           iRowidReg, pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು); VdbeCoverage(v);
    }

    ಆದರೆ( pLevel->i_ಎಡ_Join==0 ){
      /* If a partial index is driving the loop, try to eliminate WHERE clause
      ** terms from the query that must be true due to the WHERE clause of
      ** the partial index.  This optimization does not work on an outer join,
      ** as shown by:
      **
      ** 2019-11-02 ticket 623eff57e76d45f6      (LEFT JOIN)
      ** 2025-05-29 forum post 7dee41d32506c4ae  (RIGHT JOIN)
      */
      ಆದರೆ( pIdx->pPartIdxWhere && pLevel->pRJ==0 ){
        where_Apply_Partial_ಸೂಚ್ಯಂಕ_Constraints(pIdx->pPartIdxWhere, iCur, pWC);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      testcase( pIdx->pPartIdxWhere );
      /* The following assert() is not a requirement, merely an observation:
      ** The OR-optimization doesn't work for the right hand table of
      ** a LEFT JOIN: */
      ಪ್ರತಿಜ್ಞಾ( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_ಬಲ_JOIN))==0 );
    }

    /* Record the instruction used to terminate the loop. */
    ಆದರೆ( (pLoop->wsFlags & WHERE_ONEROW)
     || (pLevel->u.ಒಳ.n_ಒಳ && regBignull==0 && where_Loop_Is_One_ಅಡ್ಡಸಾಲು(pLoop))
    ){
      pLevel->op = OP_Noop;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( bRev ){
      pLevel->op = OP_Prev;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pLevel->op = OP_ಮುಂದಿನ;
    }
    pLevel->p1 = iIdxCur;
    pLevel->p3 = (pLoop->wsFlags&WHERE_UNQ_WANTED)!=0 ? 1:0;
    ಆದರೆ( (pLoop->wsFlags & WHERE_CONSTRAINT)==0 ){
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_ಹೆಜ್ಜೆ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( pLevel->p5==0 );
    }
    ಆದರೆ( omit_ಕೋಷ್ಟಕ ) pIdx = 0;
  }ಇಲ್ಲದಿದ್ದರೆ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_OR_OPTIMIZATION
  ಆದರೆ( pLoop->wsFlags & WHERE_MULTI_OR ){
    /* Case 5:  Two or more separately indexed terms connected by OR
    **
    ** Example:
    **
    **   CREATE TABLE t1(a,b,c,d);
    **   CREATE INDEX i1 ON t1(a);
    **   CREATE INDEX i2 ON t1(b);
    **   CREATE INDEX i3 ON t1(c);
    **
    **   SELECT * FROM t1 WHERE a=5 OR b=7 OR (c=11 AND d=13)
    **
    ** In the example, there are three indexed terms connected by OR.
    ** The top of the loop looks like this:
    **
    **          Null       1                # Zero the rowset in reg 1
    **
    ** Then, for each indexed term, the following. The arguments to
    ** RowSetTest are such that the rowid of the current row is inserted
    ** into the RowSet. If it is already present, control skips the
    ** Gosub opcode and jumps straight to the code generated by WhereEnd().
    **
    **        sqlite3WhereBegin(<term>)
    **          RowSetTest                  # Insert rowid into rowset
    **          Gosub      2 A
    **        sqlite3WhereEnd()
    **
    ** Following the above, code to terminate the loop. Label A, the target
    ** of the Gosub above, jumps to the instruction right after the Goto.
    **
    **          Null       1                # Zero the rowset in reg 1
    **          Goto       B                # The loop is finished.
    **
    **       A: <loop body>                 # Return data, whatever.
    **
    **          Return     2                # Jump back to the Gosub
    **
    **       B: <after the loop>
    **
    ** Added 2014-05-26: If the table is a WITHOUT ROWID table, then
    ** use an ephemeral index instead of a RowSet to record the primary
    ** keys of the rows we have already seen.
    **
    */
    WhereClause *pOrWc;    /* The OR-clause broken out into subterms */
    SrcList *pOrTab;       /* Shortened table list or OR-clause generation */
    ಸೂಚ್ಯಂಕ *pCov = 0;             /* Potential covering index (or NULL) */
    ಪೂರ್ಣಾಂಕ iCovCur = pParse->nTab++;  /* Cursor used for index scans (if any) */

    ಪೂರ್ಣಾಂಕ regReturn = ++pParse->nMem;           /* Register used with OP_Gosub */
    ಪೂರ್ಣಾಂಕ regRowset = 0;                        /* Register for RowSet object */
    ಪೂರ್ಣಾಂಕ regRowid = 0;                         /* Register holding rowid */
    ಪೂರ್ಣಾಂಕ iLoopBody = sqlite3VdbeMakeLabel(pParse);/* Start of loop body */
    ಪೂರ್ಣಾಂಕ iRetInit;                             /* Address of regReturn init */
    ಪೂರ್ಣಾಂಕ untestedTerms = 0;             /* Some terms not completely tested */
    ಪೂರ್ಣಾಂಕ ii;                            /* Loop counter */
    ಅಭಿವ್ಯಕ್ತಿ *p_And_ಅಭಿವ್ಯಕ್ತಿ = 0;                /* An ".. AND (...)" expression */
    ಕೋಷ್ಟಕ *pTab = p_Tab_ಅಂಶ->pSTab;

    p_ಪದ = pLoop->aLTerm[0];
    ಪ್ರತಿಜ್ಞಾ( p_ಪದ!=0 );
    ಪ್ರತಿಜ್ಞಾ( p_ಪದ->eOperator & WO_OR );
    ಪ್ರತಿಜ್ಞಾ( (p_ಪದ->wtFlags & ಪದ_ORINFO)!=0 );
    pOrWc = &p_ಪದ->u.pOrInfo->wc;
    pLevel->op = OP_Return;
    pLevel->p1 = regReturn;

    /* Set up a new SrcList in pOrTab containing the table being scanned
    ** by this loop in the a[0] slot and all notReady tables in a[1..] slots.
    ** This becomes the SrcList in the recursive call to sqlite3WhereBegin().
    */
    ಆದರೆ( pWInfo->nLevel>1 || p_Tab_ಅಂಶ->fg.fromExists ){
      ಪೂರ್ಣಾಂಕ nNotReady;                 /* The number of notReady tables */
      Src_ಅಂಶ *origSrc;              /* Original list of tables */
      nNotReady = pWInfo->nLevel - iLevel - 1;
      pOrTab = sqlite3DbMallocRawNN(db, SZ_SRCLIST(nNotReady+1));
      ಆದರೆ( pOrTab==0 ) ಹಿಂತಿರುಗಿ notReady;
      pOrTab->nAlloc = (u8)(nNotReady + 1);
      pOrTab->nSrc = pOrTab->nAlloc;
      ಸ್ಮರಣೆ_ನಕಲು(pOrTab->a, p_Tab_ಅಂಶ, ಗಾತ್ರ(*p_Tab_ಅಂಶ));
      origSrc = pWInfo->pTabList->a;
      ಗಾಗಿ(k=1; k<=nNotReady; k++){
        ಸ್ಮರಣೆ_ನಕಲು(&pOrTab->a[k], &origSrc[pLevel[k].iFrom], ಗಾತ್ರ(pOrTab->a[k]));
      }

      /* Clear the fromExists flag on the OR-optimized table entry so that
      ** the calls to sqlite3WhereEnd() do not code early-exits after the
      ** first row is visited. The early exit applies to this table's
      ** overall loop - including the multiple OR branches and any WHERE
      ** conditions not passed to the sub-loops - not to the sub-loops.  */
      pOrTab->a[0].fg.fromExists = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pOrTab = pWInfo->pTabList;
    }

    /* Initialize the rowset register to contain NULL. An SQL NULL is
    ** equivalent to an empty rowset.  Or, create an ephemeral index
    ** capable of holding primary keys in the case of a WITHOUT ROWID.
    **
    ** Also initialize regReturn to contain the address of the instruction
    ** immediately following the OP_Return at the bottom of the loop. This
    ** is required in a few obscure LEFT JOIN cases where control jumps
    ** over the top of the loop into the body of it. In this case the
    ** correct response for the end-of-loop code (the OP_Return) is to
    ** fall through to the next instruction, just as an OP_Next does if
    ** called on an uninitialized cursor.
    */
    ಆದರೆ( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
      ಆದರೆ( HasRowid(pTab) ){
        regRowset = ++pParse->nMem;
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, regRowset);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಸೂಚ್ಯಂಕ *pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
        regRowset = pParse->nTab++;
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_OpenEphemeral, regRowset, pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು);
        sqlite3_Vdbe_ಗಣ_P4_ಕೀಲಿ_Info(pParse, pPk);
      }
      regRowid = ++pParse->nMem;
    }
    iRetInit = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, regReturn);

    /* If the original WHERE clause is z of the form:  (x1 OR x2 OR ...) AND y
    ** Then for every term xN, evaluate as the subexpression: xN AND y
    ** That way, terms in y that are factored into the disjunction will
    ** be picked up by the recursive calls to sqlite3WhereBegin() below.
    **
    ** Actually, each subexpression is converted to "xN AND w" where w is
    ** the "interesting" terms of z - terms that did not originate in the
    ** ON or USING clause of a LEFT JOIN, and terms that are usable as
    ** indices.
    **
    ** This optimization also only applies if the (x1 OR x2 OR ...) term
    ** is not contained in the ON clause of a LEFT JOIN.
    ** See ticket http://sqlite.org/src/info/f2369304e4
    **
    ** 2022-02-04:  Do not push down slices of a row-value comparison.
    ** In other words, "w" or "y" may not be a slice of a vector.  Otherwise,
    ** the initialization of the right-hand operand of the vector comparison
    ** might not occur, or might occur only in an OR branch that is not
    ** taken.  dbsqlfuzz 80a9fade844b4fb43564efc972bcb2c68270f5d1.
    **
    ** 2022-03-03:  Do not push down expressions that involve subqueries.
    ** The subquery might get coded as a subroutine.  Any table-references
    ** in the subquery might be resolved to index-references for the index on
    ** the OR branch in which the subroutine is coded.  But if the subroutine
    ** is invoked from a different OR branch that uses a different index, such
    ** index-references will not work.  tag-20220303a
    ** https://sqlite.org/forum/forumpost/36937b197273d403
    */
    ಆದರೆ( pWC->n_ಪದ>1 ){
      ಪೂರ್ಣಾಂಕ i_ಪದ;
      ಗಾಗಿ(i_ಪದ=0; i_ಪದ<pWC->n_ಪದ; i_ಪದ++){
        ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pWC->a[i_ಪದ].p_ಅಭಿವ್ಯಕ್ತಿ;
        ಆದರೆ( &pWC->a[i_ಪದ] == p_ಪದ ) ಮುಂದುವರಿ;
        testcase( pWC->a[i_ಪದ].wtFlags & ಪದ_VIRTUAL );
        testcase( pWC->a[i_ಪದ].wtFlags & ಪದ_CODED );
        testcase( pWC->a[i_ಪದ].wtFlags & ಪದ_ತುಂಡು );
        ಆದರೆ( (pWC->a[i_ಪದ].wtFlags & (ಪದ_VIRTUAL|ಪದ_CODED|ಪದ_ತುಂಡು))!=0 ){
          ಮುಂದುವರಿ;
        }
        ಆದರೆ( (pWC->a[i_ಪದ].eOperator & WO_ALL)==0 ) ಮುಂದುವರಿ;
        ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subquery) ) ಮುಂದುವರಿ;  /* tag-20220303a */
        ಆದರೆ( where_ಅಭಿವ್ಯಕ್ತಿ_Is_Ready(pWInfo, pLevel, p_Tab_ಅಂಶ, p_ಅಭಿವ್ಯಕ್ತಿ)==0 ) ಮುಂದುವರಿ;
        p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
        p_And_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, p_And_ಅಭಿವ್ಯಕ್ತಿ, p_ಅಭಿವ್ಯಕ್ತಿ);
      }
      ಆದರೆ( p_And_ಅಭಿವ್ಯಕ್ತಿ ){
        /* The extra 0x10000 bit on the opcode is masked off and does not
        ** become part of the new Expr.op.  However, it does make the
        ** op==TK_AND comparison inside of sqlite3PExpr() false, and this
        ** prevents sqlite3PExpr() from applying the AND short-circuit
        ** optimization, which we do not want here. */
        p_And_ಅಭಿವ್ಯಕ್ತಿ = sqlite3PExpr(pParse, TK_AND|0x10000, 0, p_And_ಅಭಿವ್ಯಕ್ತಿ);
      }
    }

    /* Run a separate WHERE clause for each term of the OR clause.  After
    ** eliminating duplicates from other WHERE clauses, the action for each
    ** sub-WHERE clause is to to invoke the main loop body as a subroutine.
    */
    ExplainQueryPlan((pParse, 1, "MULTI-INDEX OR"));
    ಗಾಗಿ(ii=0; ii<pOrWc->n_ಪದ; ii++){
      Where_ಪದ *p_Or_ಪದ = &pOrWc->a[ii];
      ಆದರೆ( p_Or_ಪದ->ಎಡ_Cursor==iCur || (p_Or_ಪದ->eOperator & WO_AND)!=0 ){
        WhereInfo *pSubWInfo;           /* Info for single OR-term scan */
        ಅಭಿವ್ಯಕ್ತಿ *p_Or_ಅಭಿವ್ಯಕ್ತಿ = p_Or_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ; /* Current OR clause term */
        ಅಭಿವ್ಯಕ್ತಿ *pDelete;                  /* Local copy of OR clause term */
        ಪೂರ್ಣಾಂಕ jmp1 = 0;                   /* Address of jump operation */
        testcase( (p_Tab_ಅಂಶ[0].fg.jointype & JT_ಎಡ)!=0
               && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_Or_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON)
        ); /* See TH3 vtab25.400 and ticket 614b25314c766238 */
        pDelete = p_Or_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_Or_ಅಭಿವ್ಯಕ್ತಿ, 0);
        ಆದರೆ( db->mallocFailed ){
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pDelete);
          ಮುಂದುವರಿ;
        }
        ಆದರೆ( p_And_ಅಭಿವ್ಯಕ್ತಿ ){
          p_And_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = p_Or_ಅಭಿವ್ಯಕ್ತಿ;
          p_Or_ಅಭಿವ್ಯಕ್ತಿ = p_And_ಅಭಿವ್ಯಕ್ತಿ;
        }
        /* Loop through table entries that match term pOrTerm. */
        ExplainQueryPlan((pParse, 1, "INDEX %d", ii+1));
        WHERETRACE(0xffffffff, ("Subplan for OR-clause:\n"));
        pSubWInfo = sqlite3WhereBegin(pParse, pOrTab, p_Or_ಅಭಿವ್ಯಕ್ತಿ, 0, 0, 0,
                                      WHERE_OR_SUBCLAUSE, iCovCur);
        ಪ್ರತಿಜ್ಞಾ( pSubWInfo || pParse->nErr );
        ಆದರೆ( pSubWInfo ){
          WhereLoop *pSubLoop;
          ಪೂರ್ಣಾಂಕ addrExplain = sqlite3_Where_Explain_One_ಪರಿಶೀಲನೆ(
              pParse, pOrTab, &pSubWInfo->a[0], 0
          );
          sqlite3_Where_ಕೂಡಿಸು_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ(v, pOrTab, &pSubWInfo->a[0], addrExplain);

          /* This is the sub-WHERE clause body.  First skip over
          ** duplicate rows from prior sub-WHERE clauses, and record the
          ** rowid (or PRIMARY KEY) for the current row so that the same
          ** row will be skipped in subsequent sub-WHERE clauses.
          */
          ಆದರೆ( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
            ಪೂರ್ಣಾಂಕ i_ಗಣ = ((ii==pOrWc->n_ಪದ-1)?-1:ii);
            ಆದರೆ( HasRowid(pTab) ){
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, iCur, -1, regRowid);
              jmp1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_ಅಡ್ಡಸಾಲು_ಗಣ_Test, regRowset, 0,
                                          regRowid, i_ಗಣ);
              VdbeCoverage(v);
            }ಇಲ್ಲದಿದ್ದರೆ{
              ಸೂಚ್ಯಂಕ *pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
              ಪೂರ್ಣಾಂಕ nPk = pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು;
              ಪೂರ್ಣಾಂಕ iPk;
              ಪೂರ್ಣಾಂಕ r;

              /* Read the PK into an array of temp registers. */
              r = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, nPk);
              ಗಾಗಿ(iPk=0; iPk<nPk; iPk++){
                ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = pPk->ai_ಸ್ತಂಭ[iPk];
                sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, iCur, i_ಲಂಬಸಾಲು,r+iPk);
              }

              /* Check if the temp table already contains this key. If so,
              ** the row has already been included in the result set and
              ** can be ignored (by jumping past the Gosub below). Otherwise,
              ** insert the key into the temp table and proceed with processing
              ** the row.
              **
              ** Use some of the same optimizations as OP_RowSetTest: If iSet
              ** is zero, assume that the key cannot already be present in
              ** the temp table. And if iSet is -1, assume that there is no
              ** need to insert the key into the temp table, as it will never
              ** be tested for.  */
              ಆದರೆ( i_ಗಣ ){
                jmp1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_ಸಿಕ್ಕಿತು, regRowset, 0, r, nPk);
                VdbeCoverage(v);
              }
              ಆದರೆ( i_ಗಣ>=0 ){
                sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, r, nPk, regRowid);
                sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, regRowset, regRowid,
                                     r, nPk);
                ಆದರೆ( i_ಗಣ ) sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
              }

              /* Release the array of temp registers */
              sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, r, nPk);
            }
          }

          /* Invoke the main loop body as a subroutine */
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, regReturn, iLoopBody);

          /* Jump here (skipping the main loop body subroutine) if the
          ** current sub-WHERE row is a duplicate from prior sub-WHEREs. */
          ಆದರೆ( jmp1 ) sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, jmp1);

          /* The pSubWInfo->untestedTerms flag means that this OR term
          ** contained one or more AND term from a notReady table.  The
          ** terms from the notReady table could not be tested and will
          ** need to be tested later.
          */
          ಆದರೆ( pSubWInfo->untestedTerms ) untestedTerms = 1;

          /* If all of the OR-connected terms are optimized using the same
          ** index, and the index is opened using the same cursor number
          ** by each call to sqlite3WhereBegin() made by this loop, it may
          ** be possible to use that index as a covering index.
          **
          ** If the call to sqlite3WhereBegin() above resulted in a scan that
          ** uses an index, and this is either the first OR-connected term
          ** processed or the index is the same as that used by all previous
          ** terms, set pCov to the candidate covering index. Otherwise, set
          ** pCov to NULL to indicate that no candidate covering index will
          ** be available.
          */
          pSubLoop = pSubWInfo->a[0].pWLoop;
          ಪ್ರತಿಜ್ಞಾ( (pSubLoop->wsFlags & WHERE_AUTO_ಸೂಚ್ಯಂಕ)==0 );
          ಆದರೆ( (pSubLoop->wsFlags & WHERE_ಸೂಚ್ಯಂಕಿತ)!=0
           && (ii==0 || pSubLoop->u.btree.p_ಸೂಚ್ಯಂಕ==pCov)
           && (HasRowid(pTab) || !Is_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pSubLoop->u.btree.p_ಸೂಚ್ಯಂಕ))
          ){
            ಪ್ರತಿಜ್ಞಾ( pSubWInfo->a[0].iIdxCur==iCovCur );
            pCov = pSubLoop->u.btree.p_ಸೂಚ್ಯಂಕ;
          }ಇಲ್ಲದಿದ್ದರೆ{
            pCov = 0;
          }
          ಆದರೆ( sqlite3_Where_ಬಳಸುತ್ತದೆ_Deferred_Seek(pSubWInfo) ){
            pWInfo->bDeferredSeek = 1;
          }

          /* Finish the loop through table entries that match term pOrTerm. */
          sqlite3_Where_ಕೊನೆ(pSubWInfo);
          Explain_Query_Plan_ತೆಗೆ(pParse);
        }
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pDelete);
      }
    }
    Explain_Query_Plan_ತೆಗೆ(pParse);
    ಪ್ರತಿಜ್ಞಾ( pLevel->pWLoop==pLoop );
    ಪ್ರತಿಜ್ಞಾ( (pLoop->wsFlags & WHERE_MULTI_OR)!=0 );
    ಪ್ರತಿಜ್ಞಾ( (pLoop->wsFlags & WHERE_ಒಳ_ABLE)==0 );
    pLevel->u.pCoveringIdx = pCov;
    ಆದರೆ( pCov ) pLevel->iIdxCur = iCovCur;
    ಆದರೆ( p_And_ಅಭಿವ್ಯಕ್ತಿ ){
      p_And_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = 0;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_And_ಅಭಿವ್ಯಕ್ತಿ);
    }
    sqlite3VdbeChangeP1(v, iRetInit, sqlite3VdbeCurrentAddr(v));
    sqlite3VdbeGoto(v, pLevel->addrBrk);
    sqlite3VdbeResolveLabel(v, iLoopBody);

    /* Set the P2 operand of the OP_Return opcode that will end the current
    ** loop to point to this spot, which is the top of the next containing
    ** loop.  The byte-code formatter will use that P2 value as a hint to
    ** indent everything in between the this point and the final OP_Return.
    ** See tag-20220407a in vdbe.c and shell.c */
    ಪ್ರತಿಜ್ಞಾ( pLevel->op==OP_Return );
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    ಆದರೆ( pWInfo->pTabList!=pOrTab ){ sqlite3DbFreeNN(db, pOrTab); }
    ಆದರೆ( !untestedTerms ) disable_ಪದ(pLevel, p_ಪದ);
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_OR_OPTIMIZATION */

  {
    /* Case 6:  There is no usable index.  We must do a complete
    **          scan of the entire table.
    */
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 a_ಹೆಜ್ಜೆ[] = { OP_ಮುಂದಿನ, OP_Prev };
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 a_ಆರಂಭ[] = { OP_Rewind, OP_ಕೊನೆಯ };
    ಪ್ರತಿಜ್ಞಾ( bRev==0 || bRev==1 );
    ಆದರೆ( p_Tab_ಅಂಶ->fg.isRecursive ){
      /* Tables marked isRecursive have only a single row that is stored in
      ** a pseudo-cursor.  No need to Rewind or Next such cursors. */
      pLevel->op = OP_Noop;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಸಂಕೇತ_Cursor_Hint(p_Tab_ಅಂಶ, pWInfo, pLevel, 0);
      pLevel->op = a_ಹೆಜ್ಜೆ[bRev];
      pLevel->p1 = iCur;
      pLevel->p2 = 1 + sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, a_ಆರಂಭ[bRev],iCur,pLevel->addrHalt);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_ಹೆಜ್ಜೆ;
    }
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
  pLevel->addr_ಭೇಟಿ = sqlite3VdbeCurrentAddr(v);
#ಆದರೆ_ಅಂತ್ಯ

  /* Insert code to test every subexpression that can be completely
  ** computed using the current set of tables.
  **
  ** This loop may run between one and three times, depending on the
  ** constraints to be generated. The value of stack variable iLoop
  ** determines the constraints coded by each iteration, as follows:
  **
  ** iLoop==1: Code only expressions that are entirely covered by pIdx.
  ** iLoop==2: Code remaining expressions that do not contain correlated
  **           sub-queries.
  ** iLoop==3: Code all remaining expressions.
  **
  ** An effort is made to skip unnecessary iterations of the loop.
  **
  ** This optimization of causing simple query restrictions to occur before
  ** more complex one is call the "push-down" optimization in MySQL.  Here
  ** in SQLite, the name is "MySQL push-down", since there is also another
  ** totally unrelated optimization called "WHERE-clause push-down".
  ** Sometimes the qualifier is omitted, resulting in an ambiguity, so beware.
  */
  iLoop = (pIdx ? 1 : 2);
  ಮಾಡು{
    ಪೂರ್ಣಾಂಕ i_ಮುಂದಿನ = 0;                /* Next value for iLoop */
    ಗಾಗಿ(p_ಪದ=pWC->a, j=pWC->n_ಪದ; j>0; j--, p_ಪದ++){
      ಅಭಿವ್ಯಕ್ತಿ *pE;
      ಪೂರ್ಣಾಂಕ skipLikeAddr = 0;
      testcase( p_ಪದ->wtFlags & ಪದ_VIRTUAL );
      testcase( p_ಪದ->wtFlags & ಪದ_CODED );
      ಆದರೆ( p_ಪದ->wtFlags & (ಪದ_VIRTUAL|ಪದ_CODED) ) ಮುಂದುವರಿ;
      ಆದರೆ( (p_ಪದ->prereqAll & pLevel->notReady)!=0 ){
        testcase( pWInfo->untestedTerms==0
            && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 );
        pWInfo->untestedTerms = 1;
        ಮುಂದುವರಿ;
      }
      pE = p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ;
      ಪ್ರತಿಜ್ಞಾ( pE!=0 );
      ಆದರೆ( where_ಅಭಿವ್ಯಕ್ತಿ_Is_Ready(pWInfo, pLevel, p_Tab_ಅಂಶ, pE)==0 ) ಮುಂದುವರಿ;
      ಆದರೆ( iLoop==1 && !sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Covered_By_ಸೂಚ್ಯಂಕ(pE, pLevel->iTabCur, pIdx) ){
        i_ಮುಂದಿನ = 2;
        ಮುಂದುವರಿ;
      }
      ಆದರೆ( iLoop<3 && (p_ಪದ->wtFlags & ಪದ_VARSELECT) ){
        ಆದರೆ( i_ಮುಂದಿನ==0 ) i_ಮುಂದಿನ = 3;
        ಮುಂದುವರಿ;
      }

      ಆದರೆ( (p_ಪದ->wtFlags & ಪದ_LIKECOND)!=0 ){
        /* If the TERM_LIKECOND flag is set, that means that the range search
        ** is sufficient to guarantee that the LIKE operator is true, so we
        ** can skip the call to the like(A,B) function.  But this only works
        ** for strings.  So do not skip the call to the function on the pass
        ** that compares BLOBs. */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_LIKE_DOESNT_ಹೊಂದಿಕೆ_BLOBS
        ಮುಂದುವರಿ;
#ಇಲ್ಲದಿದ್ದರೆ
        u32 x = pLevel->iLikeRepCntr;
        ಆದರೆ( x>0 ){
          skipLikeAddr = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, (x&1)?OP_IfNot:OP_If,(ಪೂರ್ಣಾಂಕ)(x>>1));
          VdbeCoverageIf(v, (x&1)==1);
          VdbeCoverageIf(v, (x&1)==0);
        }
#ಆದರೆ_ಅಂತ್ಯ
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WHERETRACE_ENABLED /* 0xffffffff */
      ಆದರೆ( sqlite3WhereTrace ){
        VdbeNoopComment((v, "WhereTerm[%d] (%p) priority=%d",
                         pWC->n_ಪದ-j, p_ಪದ, iLoop));
      }
      ಆದರೆ( sqlite3WhereTrace & 0x4000 ){
        sqlite3DebugPrintf("Coding auxiliary constraint:\n");
        sqlite3_Where_ಪದ_Print(p_ಪದ, pWC->n_ಪದ-j);
      }
#ಆದರೆ_ಅಂತ್ಯ
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pE, addrCont, SQLITE_JUMPIFNULL);
      ಆದರೆ( skipLikeAddr ) sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, skipLikeAddr);
      p_ಪದ->wtFlags |= ಪದ_CODED;
    }
    iLoop = i_ಮುಂದಿನ;
  }ಆಗ( iLoop>0 );

  /* Insert code to test for implied constraints based on transitivity
  ** of the "==" operator.
  **
  ** Example: If the WHERE clause contains "t1.a=t2.b" and "t2.b=123"
  ** and we are coding the t1 loop and the t2 loop has not yet coded,
  ** then we cannot use the "t1.a=t2.b" constraint, but we can code
  ** the implied "t1.a=123" constraint.
  */
  ಗಾಗಿ(p_ಪದ=pWC->a, j=pWC->n_ತಳ; j>0; j--, p_ಪದ++){
    ಅಭಿವ್ಯಕ್ತಿ *pE, sEAlt;
    Where_ಪದ *pAlt;
    ಆದರೆ( p_ಪದ->wtFlags & (ಪದ_VIRTUAL|ಪದ_CODED) ) ಮುಂದುವರಿ;
    ಆದರೆ( (p_ಪದ->eOperator & (WO_EQ|WO_IS))==0 ) ಮುಂದುವರಿ;
    ಆದರೆ( (p_ಪದ->eOperator & WO_EQUIV)==0 ) ಮುಂದುವರಿ;
    ಆದರೆ( p_ಪದ->ಎಡ_Cursor!=iCur ) ಮುಂದುವರಿ;
    ಆದರೆ( p_Tab_ಅಂಶ->fg.jointype & (JT_ಎಡ|JT_LTORJ|JT_ಬಲ) ) ಮುಂದುವರಿ;
    pE = p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ WHERETRACE_ENABLED /* 0x4001 */
    ಆದರೆ( (sqlite3WhereTrace & 0x4001)==0x4001 ){
      sqlite3DebugPrintf("Coding transitive constraint:\n");
      sqlite3_Where_ಪದ_Print(p_ಪದ, pWC->n_ಪದ-j);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pE, EP_OuterON) );
    ಪ್ರತಿಜ್ಞಾ( (p_ಪದ->prereq_ಬಲ & pLevel->notReady)!=0 );
    ಪ್ರತಿಜ್ಞಾ( (p_ಪದ->eOperator & (WO_OR|WO_AND))==0 );
    pAlt = sqlite3_Where_ಕಂಡುಹಿಡಿ_ಪದ(pWC, iCur, p_ಪದ->u.x.ಎಡ_ಸ್ತಂಭ, notReady,
                    WO_EQ|WO_ಒಳ|WO_IS, 0);
    ಆದರೆ( pAlt==0 ) ಮುಂದುವರಿ;
    ಆದರೆ( pAlt->wtFlags & (ಪದ_CODED) ) ಮುಂದುವರಿ;
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pAlt->p_ಅಭಿವ್ಯಕ್ತಿ, EP_Collate) ) ಮುಂದುವರಿ;
    ಆದರೆ( (pAlt->eOperator & WO_ಒಳ)
     && ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pAlt->p_ಅಭಿವ್ಯಕ್ತಿ)
     && (pAlt->p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ>1)
    ){
      ಮುಂದುವರಿ;
    }
    testcase( pAlt->eOperator & WO_EQ );
    testcase( pAlt->eOperator & WO_IS );
    testcase( pAlt->eOperator & WO_ಒಳ );
    VdbeModuleComment((v, "begin transitive constraint"));
    sEAlt = *pAlt->p_ಅಭಿವ್ಯಕ್ತಿ;
    sEAlt.p_ಎಡ = pE->p_ಎಡ;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, &sEAlt, addrCont, SQLITE_JUMPIFNULL);
    pAlt->wtFlags |= ಪದ_CODED;
  }

  /* For a RIGHT OUTER JOIN, record the fact that the current row has
  ** been matched at least once.
  */
  ಆದರೆ( pLevel->pRJ ){
    ಕೋಷ್ಟಕ *pTab;
    ಪೂರ್ಣಾಂಕ nPk;
    ಪೂರ್ಣಾಂಕ r;
    ಪೂರ್ಣಾಂಕ jmp1 = 0;
    Where_ಬಲ_Join *pRJ = pLevel->pRJ;

    /* pTab is the right-hand table of the RIGHT JOIN.  Generate code that
    ** will record that the current row of that table has been matched at
    ** least once.  This is accomplished by storing the PK for the row in
    ** both the iMatch index and the regBloom Bloom filter.
    */
    pTab = pWInfo->pTabList->a[pLevel->iFrom].pSTab;
    ಆದರೆ( HasRowid(pTab) ){
      r = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, 2);
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, pLevel->iTabCur, -1, r+1);
      nPk = 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ iPk;
      ಸೂಚ್ಯಂಕ *pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
      nPk = pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು;
      r = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, nPk+1);
      ಗಾಗಿ(iPk=0; iPk<nPk; iPk++){
        ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = pPk->ai_ಸ್ತಂಭ[iPk];
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, iCur, i_ಲಂಬಸಾಲು,r+1+iPk);
      }
    }
    jmp1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_ಸಿಕ್ಕಿತು, pRJ->i_ಹೊಂದಿಕೆ, 0, r+1, nPk);
    VdbeCoverage(v);
    VdbeComment((v, "match against %s", pTab->zName));
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, r+1, nPk, r);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, pRJ->i_ಹೊಂದಿಕೆ, r, r+1, nPk);
    ಆದರೆ( pRJ->regBloom ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Filter_ಕೂಡಿಸು, pRJ->regBloom, 0, r+1, nPk);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
    }
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, jmp1);
    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, r, nPk+1);
  }

  /* For a LEFT OUTER JOIN, generate code that will record the fact that
  ** at least one row of the right table has matched the left table.
  */
  ಆದರೆ( pLevel->i_ಎಡ_Join ){
    pLevel->addr_ಮೊದಲ = sqlite3VdbeCurrentAddr(v);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 1, pLevel->i_ಎಡ_Join);
    VdbeComment((v, "record LEFT JOIN hit"));
    ಆದರೆ( pLevel->pRJ==0 ){
      ಹೋಗು ಸಂಕೇತ_outer_join_constraints; /* WHERE clause constraints */
    }
  }

  ಆದರೆ( pLevel->pRJ ){
    /* Create a subroutine used to process all interior loops and code
    ** of the RIGHT JOIN.  During normal operation, the subroutine will
    ** be in-line with the rest of the code.  But at the end, a separate
    ** loop will run that invokes this subroutine for unmatched rows
    ** of pTab, with all tables to left begin set to NULL.
    */
    {
      Where_ಬಲ_Join *pRJ = pLevel->pRJ;
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_BeginSubrtn, 0, pRJ->regReturn);
      pRJ->addrSubrtn = sqlite3VdbeCurrentAddr(v);
    }
    ಪ್ರತಿಜ್ಞಾ( pParse->withinRJSubrtn < 255 );
    pParse->withinRJSubrtn++;

    /* WHERE clause constraints must be deferred until after outer join
    ** row elimination has completed, since WHERE clause constraints apply
    ** to the results of the OUTER JOIN.  The following loop generates the
    ** appropriate WHERE clause constraint checks.  tag-20220513a.
    */
  ಸಂಕೇತ_outer_join_constraints:
    ಗಾಗಿ(p_ಪದ=pWC->a, j=0; j<pWC->n_ತಳ; j++, p_ಪದ++){
      testcase( p_ಪದ->wtFlags & ಪದ_VIRTUAL );
      testcase( p_ಪದ->wtFlags & ಪದ_CODED );
      ಆದರೆ( p_ಪದ->wtFlags & (ಪದ_VIRTUAL|ಪದ_CODED) ) ಮುಂದುವರಿ;
      ಆದರೆ( (p_ಪದ->prereqAll & pLevel->notReady)!=0 ){
        ಪ್ರತಿಜ್ಞಾ( pWInfo->untestedTerms );
        ಮುಂದುವರಿ;
      }
      ಆದರೆ( p_Tab_ಅಂಶ->fg.jointype & JT_LTORJ ) ಮುಂದುವರಿ;
      ಪ್ರತಿಜ್ಞಾ( p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ );
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ, addrCont, SQLITE_JUMPIFNULL);
      p_ಪದ->wtFlags |= ಪದ_CODED;
    }
  }

#ಆದರೆ WHERETRACE_ENABLED /* 0x4001 */
  ಆದರೆ( sqlite3WhereTrace & 0x4000 ){
    sqlite3DebugPrintf("All WHERE-clause terms after coding level %d:\n",
                       iLevel);
    sqlite3WhereClausePrint(pWC);
  }
  ಆದರೆ( sqlite3WhereTrace & 0x1 ){
    sqlite3DebugPrintf("End Coding level %d:  notReady=%llx\n",
       iLevel, (u64)pLevel->notReady);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ pLevel->notReady;
}

/*
** Generate the code for the loop that finds all non-matched terms
** for a RIGHT JOIN.
*/
SQLITE_NOINLINE ಶೂನ್ಯ sqlite3_Where_ಬಲ_Join_Loop(
  WhereInfo *pWInfo,
  ಪೂರ್ಣಾಂಕ iLevel,
  WhereLevel *pLevel
){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  Where_ಬಲ_Join *pRJ = pLevel->pRJ;
  ಅಭಿವ್ಯಕ್ತಿ *pSubWhere = 0;
  WhereClause *pWC = &pWInfo->sWC;
  WhereInfo *pSubWInfo;
  WhereLoop *pLoop = pLevel->pWLoop;
  Src_ಅಂಶ *p_Tab_ಅಂಶ = &pWInfo->pTabList->a[pLevel->iFrom];
  SrcList *pFrom;
  ಒಕ್ಕೂಟ {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;
  Bitmask mAll = 0;
  ಪೂರ್ಣಾಂಕ k;

  ExplainQueryPlan((pParse, 1, "RIGHT-JOIN %s", p_Tab_ಅಂಶ->pSTab->zName));
  sqlite3VdbeNoJumpsOutsideSubrtn(v, pRJ->addrSubrtn, pRJ->ಕೊನೆ_Subrtn,
                                  pRJ->regReturn);
  ಗಾಗಿ(k=0; k<iLevel; k++){
    ಪೂರ್ಣಾಂಕ iIdxCur;
    Src_ಅಂಶ *p_ಬಲ;
    ಪ್ರತಿಜ್ಞಾ( pWInfo->a[k].pWLoop->iTab == pWInfo->a[k].iFrom );
    p_ಬಲ = &pWInfo->pTabList->a[pWInfo->a[k].iFrom];
    mAll |= pWInfo->a[k].pWLoop->ಮುಖವಾಡ_Self;
    ಆದರೆ( p_ಬಲ->fg.ಮೂಲಕ_Coroutine ){
      Subquery *pSubq;
      ಪ್ರತಿಜ್ಞಾ( p_ಬಲ->fg.isSubquery && p_ಬಲ->u4.pSubq!=0 );
      pSubq = p_ಬಲ->u4.pSubq;
      ಪ್ರತಿಜ್ಞಾ( pSubq->pSelect!=0 && pSubq->pSelect->pEList!=0 );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(
          v, OP_Null, 0, pSubq->reg_ಫಲಿತಾಂಶ,
          pSubq->reg_ಫಲಿತಾಂಶ + pSubq->pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ-1
      );
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Null_ಅಡ್ಡಸಾಲು, pWInfo->a[k].iTabCur);
    iIdxCur = pWInfo->a[k].iIdxCur;
    ಆದರೆ( iIdxCur ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Null_ಅಡ್ಡಸಾಲು, iIdxCur);
    }
  }
  ಆದರೆ( (p_Tab_ಅಂಶ->fg.jointype & JT_LTORJ)==0 ){
    mAll |= pLoop->ಮುಖವಾಡ_Self;
    ಗಾಗಿ(k=0; k<pWC->n_ಪದ; k++){
      Where_ಪದ *p_ಪದ = &pWC->a[k];
      ಆದರೆ( (p_ಪದ->wtFlags & (ಪದ_VIRTUAL|ಪದ_ತುಂಡು))!=0
       && p_ಪದ->eOperator!=WO_ROWVAL
      ){
        ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( p_ಪದ->prereqAll & ~mAll ) ಮುಂದುವರಿ;
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON|EP_InnerON) ) ಮುಂದುವರಿ;
      pSubWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, pSubWhere,
                                 sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(pParse->db, p_ಪದ->p_ಅಭಿವ್ಯಕ್ತಿ, 0));
    }
  }
  ಆದರೆ( pLevel->iIdxCur ){
    /* pSubWhere may contain expressions that read from an index on the
    ** table on the RHS of the right join. All such expressions first test
    ** if the index is pointing at a NULL row, and if so, read from the
    ** table cursor instead. So ensure that the index cursor really is 
    ** pointing at a NULL row here, so that no values are read from it during
    ** the scan of the RHS of the RIGHT join below.  */
    sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Null_ಅಡ್ಡಸಾಲು, pLevel->iIdxCur);
  }
  pFrom = &uSrc.sSrc;
  pFrom->nSrc = 1;
  pFrom->nAlloc = 1;
  ಸ್ಮರಣೆ_ನಕಲು(&pFrom->a[0], p_Tab_ಅಂಶ, ಗಾತ್ರ(Src_ಅಂಶ));
  pFrom->a[0].fg.jointype = 0;
  ಆದರೆ( pParse->withinRJSubrtn >= 100 ){
    /* This limit --------------^^^
    ** is based on an historical assert().  It is not compile-time or
    ** run-time configurable.  It could perhaps be raised as high as 254,
    ** but only an attack robot would ever do even 100 RIGHT JOINS within
    ** a single query, so we'll just leave it as it is. */
    sqlite3_Error_ಸಂದೇಶ(pParse, "too many RIGHT JOINs");
    ಹಿಂತಿರುಗಿ;
  }
  pParse->withinRJSubrtn++;
  pSubWInfo = sqlite3WhereBegin(pParse, pFrom, pSubWhere, 0, 0, 0,
                                WHERE_ಬಲ_JOIN, 0);
  ಆದರೆ( pSubWInfo ){
    ಪೂರ್ಣಾಂಕ iCur = pLevel->iTabCur;  /* Table on RHS of RIGHT JOIN &*/
    ಪೂರ್ಣಾಂಕ r = ++pParse->nMem;      /* Register range to hold primary key */
    ಪೂರ್ಣಾಂಕ nPk;                     /* Number of values in the primary key */
    ಪೂರ್ಣಾಂಕ r2;                      /* Register holding record for primary key */
    ಪೂರ್ಣಾಂಕ addrCont = sqlite3WhereContinueLabel(pSubWInfo);
    ಕೋಷ್ಟಕ *pTab = p_Tab_ಅಂಶ->pSTab;  /* Table on RHS of RIGHT JOIN */
    ಆದರೆ( HasRowid(pTab) ){
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, iCur, -1, r);
      nPk = 1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪೂರ್ಣಾಂಕ iPk;
      ಸೂಚ್ಯಂಕ *pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
      nPk = pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು;
      pParse->nMem += nPk - 1;
      ಗಾಗಿ(iPk=0; iPk<nPk; iPk++){
        ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = pPk->ai_ಸ್ತಂಭ[iPk];
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Get_ಸ್ತಂಭ_ದ_ಕೋಷ್ಟಕ(v, pTab, iCur, i_ಲಂಬಸಾಲು,r+iPk);
      }
    }

    /* Generate code that checks to see if the current row of the RHS table
    ** has appeared in any prior output row. */
    ಆದರೆ( pRJ->regBloom ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Filter, pRJ->regBloom, 
                 sqlite3VdbeCurrentAddr(v)+2, r, nPk);
      VdbeCoverage(v);
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_ಸಿಕ್ಕಿತು, pRJ->i_ಹೊಂದಿಕೆ, addrCont, r, nPk);
    VdbeCoverage(v);
    r2 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);

    /* Generate code that inserts the PK of the RHS table into the
    ** pRH->iMatch index to indicate that the current row has appeared
    ** in the output set. */
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, r, nPk, r2);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, pRJ->i_ಹೊಂದಿಕೆ, r2, r, nPk);
    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r2);
    ಆದರೆ( pRJ->regBloom ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Filter_ಕೂಡಿಸು, pRJ->regBloom, 0, r, nPk);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
    }

    /* Invoke the subroutine that actually puts the current RHS table row
    ** into the output set, with NULLs for the LHS. */
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, pRJ->regReturn, pRJ->addrSubrtn);
    sqlite3_Where_ಕೊನೆ(pSubWInfo);
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, pSubWhere);
  Explain_Query_Plan_ತೆಗೆ(pParse);
  ಪ್ರತಿಜ್ಞಾ( pParse->withinRJSubrtn>0 );
  pParse->withinRJSubrtn--;
}
