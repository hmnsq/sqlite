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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "whereInt.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_EXPLAIN

/*
** Return the name of the i-th column of the pIdx index.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *explain_ਸੂਚਕਾਂਕ_ਕਾਲਮ_Name(ਸੂਚਕਾਂਕ *pIdx, ਪੂਰਨ i){
  i = pIdx->ai_ਕਾਲਮ[i];
  ਜੇ( i==XN_ਵਿਅੰਜਕ ) ਵਾਪਸ "<expr>";
  ਜੇ( i==XN_ROWID ) ਵਾਪਸ "rowid";
  ਵਾਪਸ pIdx->p_ਸਾਰਣੀ->a_ਖਾਨਾ[i].zCnName;
}

/*
** This routine is a helper for explainIndexRange() below
**
** pStr holds the text of an expression that we are building up one term
** at a time.  This routine adds a new term to the end of the expression.
** Terms are separated by AND so add the "AND" text for second and subsequent
** terms only.
*/
ਸਥਿਰ ਖਾਲੀ explain_Append_ਪਦ(
  StrAccum *pStr,             /* The text expression being built */
  ਸੂਚਕਾਂਕ *pIdx,                /* Index to read column names from */
  ਪੂਰਨ n_ਪਦ,                  /* Number of terms */
  ਪੂਰਨ i_ਪਦ,                  /* Zero-based index of first term. */
  ਪੂਰਨ bAnd,                   /* Non-zero to append " AND " */
  ਅਚਲ ਅੱਖਰ *zOp             /* Name of the operator */
){
  ਪੂਰਨ i;

  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਪਦ>=1 );
  ਜੇ( bAnd ) sqlite3_str_append(pStr, " AND ", 5);

  ਜੇ( n_ਪਦ>1 ) sqlite3_str_append(pStr, "(", 1);
  ਲਈ(i=0; i<n_ਪਦ; i++){
    ਜੇ( i ) sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_appendall(pStr, explain_ਸੂਚਕਾਂਕ_ਕਾਲਮ_Name(pIdx, i_ਪਦ+i));
  }
  ਜੇ( n_ਪਦ>1 ) sqlite3_str_append(pStr, ")", 1);

  sqlite3_str_append(pStr, zOp, 1);

  ਜੇ( n_ਪਦ>1 ) sqlite3_str_append(pStr, "(", 1);
  ਲਈ(i=0; i<n_ਪਦ; i++){
    ਜੇ( i ) sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_append(pStr, "?", 1);
  }
  ਜੇ( n_ਪਦ>1 ) sqlite3_str_append(pStr, ")", 1);
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
ਸਥਿਰ ਖਾਲੀ explain_ਸੂਚਕਾਂਕ_Range(StrAccum *pStr, WhereLoop *pLoop){
  ਸੂਚਕਾਂਕ *p_ਸੂਚਕਾਂਕ = pLoop->u.btree.p_ਸੂਚਕਾਂਕ;
  u16 nEq = pLoop->u.btree.nEq;
  u16 nSkip = pLoop->nSkip;
  ਪੂਰਨ i, j;

  ਜੇ( nEq==0 && (pLoop->wsFlags&(WHERE_BTM_ਹੱਦਬੰਦੀ|WHERE_ਸਿਖਰ_ਹੱਦਬੰਦੀ))==0 ) ਵਾਪਸ;
  sqlite3_str_append(pStr, " (", 2);
  ਲਈ(i=0; i<nEq; i++){
    ਅਚਲ ਅੱਖਰ *z = explain_ਸੂਚਕਾਂਕ_ਕਾਲਮ_Name(p_ਸੂਚਕਾਂਕ, i);
    ਜੇ( i ) sqlite3_str_append(pStr, " AND ", 5);
    sqlite3_str_appendf(pStr, i>=nSkip ? "%s=?" : "ANY(%s)", z);
  }

  j = i;
  ਜੇ( pLoop->wsFlags&WHERE_BTM_ਹੱਦਬੰਦੀ ){
    explain_Append_ਪਦ(pStr, p_ਸੂਚਕਾਂਕ, pLoop->u.btree.nBtm, j, i, ">");
    i = 1;
  }
  ਜੇ( pLoop->wsFlags&WHERE_ਸਿਖਰ_ਹੱਦਬੰਦੀ ){
    explain_Append_ਪਦ(pStr, p_ਸੂਚਕਾਂਕ, pLoop->u.btree.n_ਸਿਖਰ, j, i, "<");
  }
  sqlite3_str_append(pStr, ")", 1);
}

/*
** This function sets the P4 value of an existing OP_Explain opcode to
** text describing the loop in pLevel. If the OP_Explain opcode already has
** a P4 value, it is freed before it is overwritten.
*/
ਖਾਲੀ sqlite3_Where_ਜੋੜੋ_Explain_ਪਾਠ(
  Parse *pParse,                  /* Parse context */
  ਪੂਰਨ addr,                       /* Address of OP_Explain opcode */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
#ਜੇ !defined(SQLITE_DEBUG)
  ਜੇ( sqlite3ParseToplevel(pParse)->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#ਜੇ_ਅੰਤ
  {
    VdbeOp *pOp = sqlite3VdbeGetOp(pParse->pVdbe, addr);
    Src_ਵਸਤੂ *p_ਵਸਤੂ = &pTabList->a[pLevel->iFrom];
    sqlite3 *db = pParse->db;     /* Database handle */
    ਪੂਰਨ is_ਖੋਜ;                 /* True for a SEARCH. False for SCAN. */
    WhereLoop *pLoop;             /* The controlling WhereLoop object */
    u32 flags;                    /* Flags that describe this loop */
#ਜੇ defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_EXPLAIN)
    ਅੱਖਰ *z_ਸੁਨੇਹਾ;                   /* Text to add to EQP output */
#ਜੇ_ਅੰਤ
    StrAccum str;                 /* EQP output string */
    ਅੱਖਰ zBuf[100];               /* Initial space for EQP output string */

    ਜੇ( db->mallocFailed ) ਵਾਪਸ;

    pLoop = pLevel->pWLoop;
    flags = pLoop->wsFlags;

    is_ਖੋਜ = (flags&(WHERE_BTM_ਹੱਦਬੰਦੀ|WHERE_ਸਿਖਰ_ਹੱਦਬੰਦੀ))!=0
            || ((flags&WHERE_VIRTUALTABLE)==0 && (pLoop->u.btree.nEq>0))
            || (wctrlFlags&(WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX));

    sqlite3StrAccumInit(&str, db, zBuf, ਆਕਾਰ(zBuf), SQLITE_MAX_ਲੰਮਾਈ);
    str.printfFlags = SQLITE_PRINTF_ਅੰਦਰੂਨੀ;
    sqlite3_str_appendf(&str, "%s %S%s",
       is_ਖੋਜ ? "SEARCH" : "SCAN",
       p_ਵਸਤੂ,
       p_ਵਸਤੂ->fg.fromExists ? " EXISTS" : "");
    ਜੇ( (flags & (WHERE_IPK|WHERE_VIRTUALTABLE))==0 ){
      ਅਚਲ ਅੱਖਰ *zFmt = 0;
      ਸੂਚਕਾਂਕ *pIdx;

      ਪ੍ਰਤਿਜ੍ਞਾ( pLoop->u.btree.p_ਸੂਚਕਾਂਕ!=0 );
      pIdx = pLoop->u.btree.p_ਸੂਚਕਾਂਕ;
      ਪ੍ਰਤਿਜ੍ਞਾ( !(flags&WHERE_AUTO_ਸੂਚਕਾਂਕ) || (flags&WHERE_IDX_ONLY) );
      ਜੇ( !HasRowid(p_ਵਸਤੂ->pSTab) && Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pIdx) ){
        ਜੇ( is_ਖੋਜ ){
          zFmt = "PRIMARY KEY";
        }
      }ਨਹੀਂਤਾਂ ਜੇ( flags & WHERE_PARTIALIDX ){
        zFmt = "AUTOMATIC PARTIAL COVERING INDEX";
      }ਨਹੀਂਤਾਂ ਜੇ( flags & WHERE_AUTO_ਸੂਚਕਾਂਕ ){
        zFmt = "AUTOMATIC COVERING INDEX";
      }ਨਹੀਂਤਾਂ ਜੇ( flags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
        zFmt = "COVERING INDEX %s";
      }ਨਹੀਂਤਾਂ{
        zFmt = "INDEX %s";
      }
      ਜੇ( zFmt ){
        sqlite3_str_append(&str, " USING ", 7);
        sqlite3_str_appendf(&str, zFmt, pIdx->zName);
        explain_ਸੂਚਕਾਂਕ_Range(&str, pLoop);
      }
    }ਨਹੀਂਤਾਂ ਜੇ( (flags & WHERE_IPK)!=0 && (flags & WHERE_CONSTRAINT)!=0 ){
      ਅੱਖਰ cRangeOp;
#ਜੇ 0  /* Better output, but breaks many tests */
      ਅਚਲ ਸਾਰਣੀ *pTab = p_ਵਸਤੂ->pTab;
      ਅਚਲ ਅੱਖਰ *zRowid = pTab->iPKey>=0 ? pTab->a_ਖਾਨਾ[pTab->iPKey].zCnName:
                              "rowid";
#ਨਹੀਂਤਾਂ
      ਅਚਲ ਅੱਖਰ *zRowid = "rowid";
#ਜੇ_ਅੰਤ
      sqlite3_str_appendf(&str, " USING INTEGER PRIMARY KEY (%s", zRowid);
      ਜੇ( flags&(WHERE_ਕਾਲਮ_EQ|WHERE_ਕਾਲਮ_ਵਿੱਚ) ){
        cRangeOp = '=';
      }ਨਹੀਂਤਾਂ ਜੇ( (flags&WHERE_BOTH_ਹੱਦਬੰਦੀ)==WHERE_BOTH_ਹੱਦਬੰਦੀ ){
        sqlite3_str_appendf(&str, ">? AND %s", zRowid);
        cRangeOp = '<';
      }ਨਹੀਂਤਾਂ ਜੇ( flags&WHERE_BTM_ਹੱਦਬੰਦੀ ){
        cRangeOp = '>';
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( flags&WHERE_ਸਿਖਰ_ਹੱਦਬੰਦੀ);
        cRangeOp = '<';
      }
      sqlite3_str_appendf(&str, "%c?)", cRangeOp);
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
    ਨਹੀਂਤਾਂ ਜੇ( (flags & WHERE_VIRTUALTABLE)!=0 ){
      sqlite3_str_appendall(&str, " VIRTUAL TABLE INDEX ");
      sqlite3_str_appendf(&str,
                  pLoop->u.vtab.b_Idx_ਅੰਕ_Hex ? "0x%x:%s" : "%d:%s",
                  pLoop->u.vtab.idx_ਅੰਕ, pLoop->u.vtab.idxStr);
    }
#ਜੇ_ਅੰਤ
    ਜੇ( p_ਵਸਤੂ->fg.jointype & JT_ਖੱਬਾ ){
      sqlite3_str_appendf(&str, " LEFT-JOIN");
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EXPLAIN_ESTIMATED_ਪੰਕਤੀਆਂ
    ਜੇ( pLoop->n_ਬਾਹਰਲਾ>=10 ){
      sqlite3_str_appendf(&str, " (~%llu rows)",
             sqlite3_Log_Est_ਮੰਜ਼ਿਲ_Int(pLoop->n_ਬਾਹਰਲਾ));
    }ਨਹੀਂਤਾਂ{
      sqlite3_str_append(&str, " (~1 row)", 9);
    }
#ਜੇ_ਅੰਤ
#ਜੇ defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_EXPLAIN)
    z_ਸੁਨੇਹਾ = sqlite3_Str_Accum_ਅੰਤ(&str);
    sqlite3ExplainBreakpoint("",z_ਸੁਨੇਹਾ);
#ਜੇ_ਅੰਤ

    ਪ੍ਰਤਿਜ੍ਞਾ( pOp->opcode==OP_Explain );
    ਪ੍ਰਤਿਜ੍ਞਾ( pOp->p4type==P4_DYNAMIC || pOp->p4.z==0 );
    sqlite3DbFree(db, pOp->p4.z);
    pOp->p4type = P4_DYNAMIC;
    pOp->p4.z = sqlite3_Str_Accum_ਅੰਤ(&str);
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
ਪੂਰਨ sqlite3_Where_Explain_One_ਜਾਂਚ(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
  ਪੂਰਨ ret = 0;
#ਜੇ !defined(SQLITE_DEBUG)
  ਜੇ( sqlite3ParseToplevel(pParse)->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#ਜੇ_ਅੰਤ
  {
    ਜੇ( (pLevel->pWLoop->wsFlags & WHERE_MULTI_OR)==0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
    ){
      Vdbe *v = pParse->pVdbe;
      ਪੂਰਨ addr = sqlite3VdbeCurrentAddr(v);
      ret = sqlite3_Vdbe_ਜੋੜੋ_Op3(
          v, OP_Explain, addr, pParse->addrExplain, pLevel->pWLoop->r_ਲੜੀਵਾਰ
      );
      sqlite3_Where_ਜੋੜੋ_Explain_ਪਾਠ(pParse, addr, pTabList, pLevel, wctrlFlags);
    }
  }
  ਵਾਪਸ ret;
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
ਪੂਰਨ sqlite3WhereExplainBloomFilter(
  ਅਚਲ Parse *pParse,               /* Parse context */
  ਅਚਲ WhereInfo *pWInfo,           /* WHERE clause */
  ਅਚਲ WhereLevel *pLevel           /* Bloom filter on this level */
){
  ਪੂਰਨ ret = 0;
  Src_ਵਸਤੂ *p_ਵਸਤੂ = &pWInfo->pTabList->a[pLevel->iFrom];
  Vdbe *v = pParse->pVdbe;      /* VM being constructed */
  sqlite3 *db = pParse->db;     /* Database handle */
  ਅੱਖਰ *z_ਸੁਨੇਹਾ;                   /* Text to add to EQP output */
  ਪੂਰਨ i;                        /* Loop counter */
  WhereLoop *pLoop;             /* The where loop */
  StrAccum str;                 /* EQP output string */
  ਅੱਖਰ zBuf[100];               /* Initial space for EQP output string */

  sqlite3StrAccumInit(&str, db, zBuf, ਆਕਾਰ(zBuf), SQLITE_MAX_ਲੰਮਾਈ);
  str.printfFlags = SQLITE_PRINTF_ਅੰਦਰੂਨੀ;
  sqlite3_str_appendf(&str, "BLOOM FILTER ON %S (", p_ਵਸਤੂ);
  pLoop = pLevel->pWLoop;
  ਜੇ( pLoop->wsFlags & WHERE_IPK ){
    ਅਚਲ ਸਾਰਣੀ *pTab = p_ਵਸਤੂ->pSTab;
    ਜੇ( pTab->iPKey>=0 ){
      sqlite3_str_appendf(&str, "%s=?", pTab->a_ਖਾਨਾ[pTab->iPKey].zCnName);
    }ਨਹੀਂਤਾਂ{
      sqlite3_str_appendf(&str, "rowid=?");
    }
  }ਨਹੀਂਤਾਂ{
    ਲਈ(i=pLoop->nSkip; i<pLoop->u.btree.nEq; i++){
      ਅਚਲ ਅੱਖਰ *z = explain_ਸੂਚਕਾਂਕ_ਕਾਲਮ_Name(pLoop->u.btree.p_ਸੂਚਕਾਂਕ, i);
      ਜੇ( i>pLoop->nSkip ) sqlite3_str_append(&str, " AND ", 5);
      sqlite3_str_appendf(&str, "%s=?", z);
    }
  }
  sqlite3_str_append(&str, ")", 1);
  z_ਸੁਨੇਹਾ = sqlite3_Str_Accum_ਅੰਤ(&str);
  ret = sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Explain, sqlite3VdbeCurrentAddr(v),
                          pParse->addrExplain, 0, z_ਸੁਨੇਹਾ,P4_DYNAMIC);

  sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ(v, sqlite3VdbeCurrentAddr(v)-1, 0, 0, 0, 0);
  ਵਾਪਸ ret;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_EXPLAIN */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Configure the VM passed as the first argument with an
** sqlite3_stmt_scanstatus() entry corresponding to the scan used to
** implement level pLvl. Argument pSrclist is a pointer to the FROM
** clause that the scan reads data from.
**
** If argument addrExplain is not 0, it must be the address of an
** OP_Explain instruction that describes the same loop.
*/
ਖਾਲੀ sqlite3_Where_ਜੋੜੋ_ਜਾਂਚ_ਸਥਿਤੀ(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  ਪੂਰਨ addrExplain                 /* Address of OP_Explain (or 0) */
){
  ਜੇ( IS_STMT_SCANSTATUS( sqlite3VdbeDb(v) ) ){
    ਅਚਲ ਅੱਖਰ *zObj = 0;
    WhereLoop *pLoop = pLvl->pWLoop;
    ਪੂਰਨ wsFlags = pLoop->wsFlags;
    ਪੂਰਨ ਰਾਹੀਂ_Coroutine = 0;

    ਜੇ( (wsFlags & WHERE_VIRTUALTABLE)==0  &&  pLoop->u.btree.p_ਸੂਚਕਾਂਕ!=0 ){
      zObj = pLoop->u.btree.p_ਸੂਚਕਾਂਕ->zName;
    }ਨਹੀਂਤਾਂ{
      zObj = pSrclist->a[pLvl->iFrom].zName;
      ਰਾਹੀਂ_Coroutine = pSrclist->a[pLvl->iFrom].fg.ਰਾਹੀਂ_Coroutine;
    }
    sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ(
        v, addrExplain, pLvl->addrBody, pLvl->addr_ਦੇਖੋ, pLoop->n_ਬਾਹਰਲਾ, zObj
    );

    ਜੇ( ਰਾਹੀਂ_Coroutine==0 ){
      ਜੇ( (wsFlags & (WHERE_MULTI_OR|WHERE_AUTO_ਸੂਚਕਾਂਕ))==0 ){
        sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Range(v, addrExplain, -1, pLvl->iTabCur);
      }
      ਜੇ( wsFlags & WHERE_ਸੂਚਕਾਂਕਿਤ ){
        sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Range(v, addrExplain, -1, pLvl->iIdxCur);
      }
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ addr;
      VdbeOp *pOp;
      ਪ੍ਰਤਿਜ੍ਞਾ( pSrclist->a[pLvl->iFrom].fg.isSubquery );
      addr = pSrclist->a[pLvl->iFrom].u4.pSubq->addrFillSub;
      pOp = sqlite3VdbeGetOp(v, addr-1);
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3VdbeDb(v)->mallocFailed || pOp->opcode==OP_InitCoroutine );
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3VdbeDb(v)->mallocFailed || pOp->p2>addr );
      sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Range(v, addrExplain, addr, pOp->p2-1);
    }
  }
}
#ਜੇ_ਅੰਤ


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
ਸਥਿਰ ਖਾਲੀ disable_ਪਦ(WhereLevel *pLevel, Where_ਪਦ *p_ਪਦ){
  ਪੂਰਨ nLoop = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ!=0 );
  ਜਦੋਂ( (p_ਪਦ->wtFlags & ਪਦ_CODED)==0
      && (pLevel->i_ਖੱਬਾ_Join==0 || ਵਿਅੰਜਕ_Has_Property(p_ਪਦ->p_ਵਿਅੰਜਕ, EP_OuterON))
      && (pLevel->notReady & p_ਪਦ->prereqAll)==0
  ){
    ਜੇ( nLoop && (p_ਪਦ->wtFlags & ਪਦ_LIKE)!=0 ){
      p_ਪਦ->wtFlags |= ਪਦ_LIKECOND;
    }ਨਹੀਂਤਾਂ{
      p_ਪਦ->wtFlags |= ਪਦ_CODED;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED
    ਜੇ( (sqlite3WhereTrace & 0x4001)==0x4001 ){
      sqlite3DebugPrintf("DISABLE-");
      sqlite3_Where_ਪਦ_Print(p_ਪਦ, (ਪੂਰਨ)(p_ਪਦ - (p_ਪਦ->pWC->a)));
    }
#ਜੇ_ਅੰਤ
    ਜੇ( p_ਪਦ->i_ਮਾਪਾ<0 ) ਰੋਕੋ;
    p_ਪਦ = &p_ਪਦ->pWC->a[p_ਪਦ->i_ਮਾਪਾ];
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ!=0 );
    p_ਪਦ->n_ਬੱਚਾ--;
    ਜੇ( p_ਪਦ->n_ਬੱਚਾ!=0 ) ਰੋਕੋ;
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
ਸਥਿਰ ਖਾਲੀ ਕੋਡ_Apply_Affinity(Parse *pParse, ਪੂਰਨ ਅਧਾਰ, ਪੂਰਨ n, ਅੱਖਰ *zAff){
  Vdbe *v = pParse->pVdbe;
  ਜੇ( zAff==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->mallocFailed );
    ਵਾਪਸ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );

  /* Adjust base and n to skip over SQLITE_AFF_BLOB and SQLITE_AFF_NONE
  ** entries at the beginning and end of the affinity string.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  ਜਦੋਂ( n>0 && zAff[0]<=SQLITE_AFF_BLOB ){
    n--;
    ਅਧਾਰ++;
    zAff++;
  }
  ਜਦੋਂ( n>1 && zAff[n-1]<=SQLITE_AFF_BLOB ){
    n--;
  }

  /* Code the OP_Affinity opcode if there is anything left to do. */
  ਜੇ( n>0 ){
    sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Affinity, ਅਧਾਰ, n, 0, zAff, n);
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
ਸਥਿਰ ਖਾਲੀ updateRangeAffinityStr(
  ਵਿਅੰਜਕ *p_ਸੱਜਾ,                   /* RHS of comparison */
  ਪੂਰਨ n,                          /* Number of vector elements in comparison */
  ਅੱਖਰ *zAff                      /* Affinity string to modify */
){
  ਪੂਰਨ i;
  ਲਈ(i=0; i<n; i++){
    ਵਿਅੰਜਕ *p = sqlite3_ਸਦਿਸ਼_ਖੇਤਰ_Subexpr(p_ਸੱਜਾ, i);
    ਜੇ( sqlite3CompareAffinity(p, zAff[i])==SQLITE_AFF_BLOB
     || sqlite3_ਵਿਅੰਜਕ_Needs_No_Affinity_Change(p, zAff[i])
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
ਸਥਿਰ ਖਾਲੀ adjust_ਕ੍ਰਮ_By_ਖਾਨਾ(ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By, ਵਿਅੰਜਕ_List *pEList){
  ਪੂਰਨ i, j;
  ਜੇ( p_ਕ੍ਰਮ_By==0 ) ਵਾਪਸ;
  ਲਈ(i=0; i<p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ; i++){
    ਪੂਰਨ t = p_ਕ੍ਰਮ_By->a[i].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ;
    ਜੇ( t==0 ) ਜਾਰੀ;
    ਲਈ(j=0; j<pEList->n_ਵਿਅੰਜਕ; j++){
      ਜੇ( pEList->a[j].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ==t ){
        p_ਕ੍ਰਮ_By->a[i].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ = j+1;
        ਰੋਕੋ;
      }
    }
    ਜੇ( j>=pEList->n_ਵਿਅੰਜਕ ){
      p_ਕ੍ਰਮ_By->a[i].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ = 0;
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
ਸਥਿਰ ਵਿਅੰਜਕ *remove_Unindexable_ਵਿੱਚ_Clause_Terms(
  Parse *pParse,        /* The parsing context */
  ਪੂਰਨ iEq,              /* Look at loop terms starting here */
  WhereLoop *pLoop,     /* The current loop */
  ਵਿਅੰਜਕ *pX              /* The IN expression to be reduced */
){
  sqlite3 *db = pParse->db;
  Select *pSelect;            /* Pointer to the SELECT on the RHS */
  ਵਿਅੰਜਕ *pNew;
  pNew = sqlite3_ਵਿਅੰਜਕ_Dup(db, pX, 0);
  ਜੇ( db->mallocFailed==0 ){
    ਲਈ(pSelect=pNew->x.pSelect; pSelect; pSelect=pSelect->p_ਪੂਰਵ){
      ਵਿਅੰਜਕ_List *pOrigRhs;         /* Original unmodified RHS */
      ਵਿਅੰਜਕ_List *pOrigLhs = 0;     /* Original unmodified LHS */
      ਵਿਅੰਜਕ_List *pRhs = 0;         /* New RHS after modifications */
      ਵਿਅੰਜਕ_List *pLhs = 0;         /* New LHS after mods */
      ਪੂਰਨ i;                      /* Loop counter */
      ਪੂਰਨ nRhs = 0;               /* Number of RHS terms added so far */

      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(pNew) );
      pOrigRhs = pSelect->pEList;
      ਪ੍ਰਤਿਜ੍ਞਾ( pNew->p_ਖੱਬਾ!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(pNew->p_ਖੱਬਾ) );
      ਜੇ( pSelect==pNew->x.pSelect ){
        pOrigLhs = pNew->p_ਖੱਬਾ->x.pList;
      }
      ਲਈ(i=iEq; i<pLoop->nLTerm; i++){
        ਜੇ( pLoop->aLTerm[i]->p_ਵਿਅੰਜਕ==pX ){
          ਪੂਰਨ i_ਖੇਤਰ;
          ਪ੍ਰਤਿਜ੍ਞਾ( (pLoop->aLTerm[i]->eOperator & (WO_OR|WO_AND))==0 );
          i_ਖੇਤਰ = pLoop->aLTerm[i]->u.x.i_ਖੇਤਰ - 1;
          ਜੇ( i_ਖੇਤਰ!=nRhs ){
            ਵਿਅੰਜਕ_Clear_Property(pNew, EP_Subrtn);
          }
          ਜੇ( NEVER(pOrigRhs->a[i_ਖੇਤਰ].p_ਵਿਅੰਜਕ==0) ){
            ਜਾਰੀ; /* Duplicate PK column */
          }
          pRhs = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pRhs, pOrigRhs->a[i_ਖੇਤਰ].p_ਵਿਅੰਜਕ);
          pOrigRhs->a[i_ਖੇਤਰ].p_ਵਿਅੰਜਕ = 0;
          ਜੇ( pRhs ) pRhs->a[pRhs->n_ਵਿਅੰਜਕ-1].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ = i_ਖੇਤਰ+1;
          ਜੇ( pOrigLhs ){
            ਪ੍ਰਤਿਜ੍ਞਾ( pOrigLhs->a[i_ਖੇਤਰ].p_ਵਿਅੰਜਕ!=0 );
            pLhs = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse,pLhs,pOrigLhs->a[i_ਖੇਤਰ].p_ਵਿਅੰਜਕ);
            pOrigLhs->a[i_ਖੇਤਰ].p_ਵਿਅੰਜਕ = 0;
          }
          nRhs++;
        }
      }
      ਜੇ( nRhs<pOrigRhs->n_ਵਿਅੰਜਕ ){
        ਵਿਅੰਜਕ_Clear_Property(pNew, EP_Subrtn); /* See bug 2026-10-16T22:07:15Z */
      }
      sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pOrigRhs);
      ਜੇ( pOrigLhs ){
        sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pOrigLhs);
        pNew->p_ਖੱਬਾ->x.pList = pLhs;
      }
      pSelect->pEList = pRhs;
      pSelect->selId = ++pParse->nSelect; /* Req'd for SubrtnSig validity */
      ਜੇ( pLhs && pLhs->n_ਵਿਅੰਜਕ==1 ){
        /* Take care here not to generate a TK_VECTOR containing only a
        ** single value. Since the parser never creates such a vector, some
        ** of the subroutines do not handle this case.  */
        ਵਿਅੰਜਕ *p = pLhs->a[0].p_ਵਿਅੰਜਕ;
        pLhs->a[0].p_ਵਿਅੰਜਕ = 0;
        sqlite3_ਵਿਅੰਜਕ_Delete(db, pNew->p_ਖੱਬਾ);
        pNew->p_ਖੱਬਾ = p;
      }

      /* If either the ORDER BY clause or the GROUP BY clause contains
      ** references to result-set columns, those references might now be
      ** obsolete.  So fix them up.
      */
      ਪ੍ਰਤਿਜ੍ਞਾ( pRhs!=0 || db->mallocFailed );
      ਜੇ( pRhs ){
        adjust_ਕ੍ਰਮ_By_ਖਾਨਾ(pSelect->p_ਕ੍ਰਮ_By, pRhs);
        adjust_ਕ੍ਰਮ_By_ਖਾਨਾ(pSelect->p_ਸਮੂਹ_By, pRhs);
        ਲਈ(i=0; i<pRhs->n_ਵਿਅੰਜਕ; i++) pRhs->a[i].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ = 0;
      }

#ਜੇ 0
      ਲਿਖੋ("For indexing, change the IN expr:\n");
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(0, pX, 0);
      ਲਿਖੋ("Into:\n");
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(0, pNew, 0);
#ਜੇ_ਅੰਤ
    }
  }
  ਵਾਪਸ pNew;
}


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
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
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ ਕੋਡ_INTerm(
  Parse *pParse,      /* The parsing context */
  Where_ਪਦ *p_ਪਦ,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  ਪੂਰਨ iEq,            /* Index of the equality term within this level */
  ਪੂਰਨ bRev,           /* True for reverse-order IN operations */
  ਪੂਰਨ i_ਨਿਸ਼ਾਨਾ         /* Attempt to leave results in this register */
){
  ਵਿਅੰਜਕ *pX = p_ਪਦ->p_ਵਿਅੰਜਕ;
  ਪੂਰਨ eType = ਵਿੱਚ_ਸੂਚਕਾਂਕ_NOOP;
  ਪੂਰਨ iTab;
  ਬਣਤਰ ਵਿੱਚ_Loop *p_ਵਿੱਚ;
  WhereLoop *pLoop = pLevel->pWLoop;
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ i;
  ਪੂਰਨ nEq = 0;
  ਪੂਰਨ *aiMap = 0;

  ਜੇ( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
    && pLoop->u.btree.p_ਸੂਚਕਾਂਕ!=0
    && pLoop->u.btree.p_ਸੂਚਕਾਂਕ->a_ਛਾਂਟੋ_ਕ੍ਰਮ[iEq]
  ){
    testcase( iEq==0 );
    testcase( bRev );
    bRev = !bRev;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pX->op==TK_ਵਿੱਚ );

  ਲਈ(i=0; i<iEq; i++){
    ਜੇ( pLoop->aLTerm[i] && pLoop->aLTerm[i]->p_ਵਿਅੰਜਕ==pX ){
      disable_ਪਦ(pLevel, p_ਪਦ);
      ਵਾਪਸ;
    }
  }
  ਲਈ(i=iEq; i<pLoop->nLTerm; i++){
    ਪ੍ਰਤਿਜ੍ਞਾ( pLoop->aLTerm[i]!=0 );
    ਜੇ( pLoop->aLTerm[i]->p_ਵਿਅੰਜਕ==pX ) nEq++;
  }

  iTab = 0;
  ਜੇ( !ਵਿਅੰਜਕ_Use_XSelect(pX) || pX->x.pSelect->pEList->n_ਵਿਅੰਜਕ==1 ){
    eType = sqlite3_ਲੱਭੋ_ਵਿੱਚ_ਸੂਚਕਾਂਕ(pParse, pX, ਵਿੱਚ_ਸੂਚਕਾਂਕ_LOOP, 0, 0, &iTab);
  }ਨਹੀਂਤਾਂ{
    sqlite3 *db = pParse->db;
    ਵਿਅੰਜਕ *pXMod = remove_Unindexable_ਵਿੱਚ_Clause_Terms(pParse, iEq, pLoop, pX);
    ਜੇ( nEq>1 ){
      /* If this IN(SELECT ...) expression drives more than one column of
      ** the index, disable the seek-scan optimization. The reasons for this
      ** are that (a) it is only possible to make this happen by populating
      ** the sqlite_stat1 table with inconsistent information, and (b) it
      ** would require sqlite3FindInIndex() to find an index that is not
      ** only unique for the columns in question, but also delivers them
      ** in sorted order (requires checking asc/desc, and rejecting cases
      ** where the indexed columns are not in the right order).  */
      pLoop->wsFlags &= ~WHERE_ਵਿੱਚ_SEEKSCAN;
    }
    ਜੇ( !db->mallocFailed ){
      ਪੂਰਨ n_ਖਾਨਾ = pX->x.pSelect->pEList->n_ਵਿਅੰਜਕ;
      aiMap = (ਪੂਰਨ*)sqlite3DbMallocZero(db, ਆਕਾਰ(ਪੂਰਨ)*n_ਖਾਨਾ);
      eType = sqlite3_ਲੱਭੋ_ਵਿੱਚ_ਸੂਚਕਾਂਕ(pParse, pXMod, ਵਿੱਚ_ਸੂਚਕਾਂਕ_LOOP, 0, aiMap, &iTab);
    }
    sqlite3_ਵਿਅੰਜਕ_Delete(db, pXMod);
  }

  ਜੇ( eType==ਵਿੱਚ_ਸੂਚਕਾਂਕ_ਸੂਚਕਾਂਕ_DESC ){
    testcase( bRev );
    bRev = !bRev;
  }
  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, bRev ? OP_ਆਖਰੀ : OP_Rewind, iTab, 0);
  VdbeCoverageIf(v, bRev);
  VdbeCoverageIf(v, !bRev);

  ਪ੍ਰਤਿਜ੍ਞਾ( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );
  pLoop->wsFlags |= WHERE_ਵਿੱਚ_ABLE;
  ਜੇ( pLevel->u.ਵਿੱਚ.n_ਵਿੱਚ==0 ){
    pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
  }
  ਜੇ( iEq>0 && (pLoop->wsFlags & WHERE_ਵਿੱਚ_SEEKSCAN)==0 ){
    pLoop->wsFlags |= WHERE_ਵਿੱਚ_EARLYOUT;
  }

  i = pLevel->u.ਵਿੱਚ.n_ਵਿੱਚ;
  pLevel->u.ਵਿੱਚ.n_ਵਿੱਚ += nEq;
  pLevel->u.ਵਿੱਚ.a_ਵਿੱਚ_Loop =
     sqlite3WhereRealloc(p_ਪਦ->pWC->pWInfo,
                         pLevel->u.ਵਿੱਚ.a_ਵਿੱਚ_Loop,
                         ਆਕਾਰ(pLevel->u.ਵਿੱਚ.a_ਵਿੱਚ_Loop[0])*pLevel->u.ਵਿੱਚ.n_ਵਿੱਚ);
  p_ਵਿੱਚ = pLevel->u.ਵਿੱਚ.a_ਵਿੱਚ_Loop;
  ਜੇ( p_ਵਿੱਚ ){
    ਪੂਰਨ iMap = 0;               /* Index in aiMap[] */
    p_ਵਿੱਚ += i;
    ਲਈ(i=iEq; i<pLoop->nLTerm; i++){
      ਜੇ( pLoop->aLTerm[i]->p_ਵਿਅੰਜਕ==pX ){
        ਪੂਰਨ i_ਬਾਹਰਲਾ = i_ਨਿਸ਼ਾਨਾ + i - iEq;
        ਜੇ( eType==ਵਿੱਚ_ਸੂਚਕਾਂਕ_ROWID ){
          p_ਵਿੱਚ->addr_ਵਿੱਚ_ਸਿਖਰ = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rowid, iTab, i_ਬਾਹਰਲਾ);
        }ਨਹੀਂਤਾਂ{
          ਪੂਰਨ i_ਖਾਨਾ = aiMap ? aiMap[iMap++] : 0;
          p_ਵਿੱਚ->addr_ਵਿੱਚ_ਸਿਖਰ = sqlite3_Vdbe_ਜੋੜੋ_Op3(v,OP_ਕਾਲਮ,iTab, i_ਖਾਨਾ, i_ਬਾਹਰਲਾ);
        }
        sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IsNull, i_ਬਾਹਰਲਾ); VdbeCoverage(v);
        ਜੇ( i==iEq ){
          p_ਵਿੱਚ->iCur = iTab;
          p_ਵਿੱਚ->e_ਸਿਰਾ_Loop_Op = bRev ? OP_Prev : OP_ਅਗਲਾ;
          ਜੇ( iEq>0 ){
            p_ਵਿੱਚ->i_ਅਧਾਰ = i_ਨਿਸ਼ਾਨਾ - i;
            p_ਵਿੱਚ->n_ਅਗੇਤਰ = i;
          }ਨਹੀਂਤਾਂ{
            p_ਵਿੱਚ->n_ਅਗੇਤਰ = 0;
          }
        }ਨਹੀਂਤਾਂ{
          p_ਵਿੱਚ->e_ਸਿਰਾ_Loop_Op = OP_Noop;
        }
        p_ਵਿੱਚ++;
      }
    }
    testcase( iEq>0
              && (pLoop->wsFlags & WHERE_ਵਿੱਚ_SEEKSCAN)==0
              && (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 );
    ਜੇ( iEq>0
     && (pLoop->wsFlags & (WHERE_ਵਿੱਚ_SEEKSCAN|WHERE_VIRTUALTABLE))==0
    ){
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Seek_ਪ੍ਰਾਪਤੀ, pLevel->iIdxCur, 0, iEq);
    }
  }ਨਹੀਂਤਾਂ{
    pLevel->u.ਵਿੱਚ.n_ਵਿੱਚ = 0;
  }
  sqlite3DbFree(pParse->db, aiMap);
}
#ਜੇ_ਅੰਤ


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
ਸਥਿਰ ਪੂਰਨ ਕੋਡ_Equality_ਪਦ(
  Parse *pParse,      /* The parsing context */
  Where_ਪਦ *p_ਪਦ,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  ਪੂਰਨ iEq,            /* Index of the equality term within this level */
  ਪੂਰਨ bRev,           /* True for reverse-order IN operations */
  ਪੂਰਨ i_ਨਿਸ਼ਾਨਾ         /* Attempt to leave results in this register */
){
  ਵਿਅੰਜਕ *pX = p_ਪਦ->p_ਵਿਅੰਜਕ;
  ਪੂਰਨ iReg;                  /* Register holding results */

  ਪ੍ਰਤਿਜ੍ਞਾ( pLevel->pWLoop->aLTerm[iEq]==p_ਪਦ );
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਨਿਸ਼ਾਨਾ>0 );
  ਜੇ( pX->op==TK_EQ || pX->op==TK_IS ){
    iReg = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pParse, pX->p_ਸੱਜਾ, i_ਨਿਸ਼ਾਨਾ);
  }ਨਹੀਂਤਾਂ ਜੇ( pX->op==TK_ISNULL ){
    iReg = i_ਨਿਸ਼ਾਨਾ;
    sqlite3_Vdbe_ਜੋੜੋ_Op2(pParse->pVdbe, OP_Null, 0, iReg);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( pX->op==TK_ਵਿੱਚ );
    iReg = i_ਨਿਸ਼ਾਨਾ;
    ਕੋਡ_INTerm(pParse, p_ਪਦ, pLevel, iEq, bRev, i_ਨਿਸ਼ਾਨਾ);
#ਜੇ_ਅੰਤ
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
  ਜੇ( (pLevel->pWLoop->wsFlags & WHERE_TRANSCONS)==0
   || (p_ਪਦ->eOperator & WO_EQUIV)==0
  ){
    disable_ਪਦ(pLevel, p_ਪਦ);
  }

  ਵਾਪਸ iReg;
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
ਸਥਿਰ ਪੂਰਨ ਕੋਡ_All_Equality_Terms(
  Parse *pParse,        /* Parsing context */
  WhereLevel *pLevel,   /* Which nested loop of the FROM we are coding */
  ਪੂਰਨ bRev,             /* Reverse the order of IN operators */
  ਪੂਰਨ nExtraReg,        /* Number of extra registers to allocate */
  ਅੱਖਰ **pzAff          /* OUT: Set to point to affinity string */
){
  u16 nEq;                      /* The number of == or IN constraints to code */
  u16 nSkip;                    /* Number of left-most columns to skip */
  Vdbe *v = pParse->pVdbe;      /* The vm under construction */
  ਸੂਚਕਾਂਕ *pIdx;                  /* The index being used for this loop */
  Where_ਪਦ *p_ਪਦ;             /* A single constraint term */
  WhereLoop *pLoop;             /* The WhereLoop object */
  ਪੂਰਨ j;                        /* Loop counter */
  ਪੂਰਨ reg_ਅਧਾਰ;                  /* Base register */
  ਪੂਰਨ nReg;                     /* Number of registers to allocate */
  ਅੱਖਰ *zAff;                   /* Affinity string to return */

  /* This module is only called on query plans that use an index. */
  pLoop = pLevel->pWLoop;
  ਪ੍ਰਤਿਜ੍ਞਾ( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0 );
  nEq = pLoop->u.btree.nEq;
  nSkip = pLoop->nSkip;
  pIdx = pLoop->u.btree.p_ਸੂਚਕਾਂਕ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdx!=0 );

  /* Figure out how many memory cells we will need then allocate them.
  */
  reg_ਅਧਾਰ = pParse->nMem + 1;
  nReg = nEq + nExtraReg;
  pParse->nMem += nReg;

  zAff = sqlite3DbStrDup(pParse->db,sqlite3_ਸੂਚਕਾਂਕ_Affinity_Str(pParse->db,pIdx));
  ਪ੍ਰਤਿਜ੍ਞਾ( zAff!=0 || pParse->db->mallocFailed );

  ਜੇ( nSkip ){
    ਪੂਰਨ iIdxCur = pLevel->iIdxCur;
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Null, 0, reg_ਅਧਾਰ, reg_ਅਧਾਰ+nSkip-1);
    sqlite3_Vdbe_ਜੋੜੋ_Op1(v, (bRev?OP_ਆਖਰੀ:OP_Rewind), iIdxCur);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    VdbeComment((v, "begin skip-scan on %s", pIdx->zName));
    j = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Goto);
    ਪ੍ਰਤਿਜ੍ਞਾ( pLevel->addrSkip==0 );
    pLevel->addrSkip = sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, (bRev?OP_SeekLT:OP_SeekGT),
                            iIdxCur, 0, reg_ਅਧਾਰ, nSkip);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, j);
    ਲਈ(j=0; j<nSkip; j++){
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iIdxCur, j, reg_ਅਧਾਰ+j);
      testcase( pIdx->ai_ਕਾਲਮ[j]==XN_ਵਿਅੰਜਕ );
      VdbeComment((v, "%s", explain_ਸੂਚਕਾਂਕ_ਕਾਲਮ_Name(pIdx, j)));
    }
  }

  /* Evaluate the equality constraints
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( zAff==0 || (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zAff)>=nEq );
  ਲਈ(j=nSkip; j<nEq; j++){
    ਪੂਰਨ r1;
    p_ਪਦ = pLoop->aLTerm[j];
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ!=0 );
    /* The following testcase is true for indices with redundant columns.
    ** Ex: CREATE INDEX i1 ON t1(a,b,a); SELECT * FROM t1 WHERE a=0 AND b=0; */
    testcase( (p_ਪਦ->wtFlags & ਪਦ_CODED)!=0 );
    testcase( p_ਪਦ->wtFlags & ਪਦ_VIRTUAL );
    r1 = ਕੋਡ_Equality_ਪਦ(pParse, p_ਪਦ, pLevel, j, bRev, reg_ਅਧਾਰ+j);
    ਜੇ( r1!=reg_ਅਧਾਰ+j ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Copy, r1, reg_ਅਧਾਰ+j);
    }
    ਜੇ( p_ਪਦ->eOperator & WO_ਵਿੱਚ ){
      ਜੇ( p_ਪਦ->p_ਵਿਅੰਜਕ->flags & EP_xIsSelect ){
        /* No affinity ever needs to be (or should be) applied to a value
        ** from the RHS of an "? IN (SELECT ...)" expression. The
        ** sqlite3FindInIndex() routine has already ensured that the
        ** affinity of the comparison has been applied to the value.  */
        ਜੇ( zAff ) zAff[j] = SQLITE_AFF_BLOB;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( (p_ਪਦ->eOperator & WO_ISNULL)==0 ){
      ਵਿਅੰਜਕ *p_ਸੱਜਾ = p_ਪਦ->p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
      ਜੇ( (p_ਪਦ->wtFlags & ਪਦ_IS)==0 && sqlite3_ਵਿਅੰਜਕ_Can_Be_Null(p_ਸੱਜਾ) ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, reg_ਅਧਾਰ+j, pLevel->addrBrk);
        VdbeCoverage(v);
      }
      ਜੇ( pParse->nErr==0 ){
        ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->mallocFailed==0 );
        ਜੇ( sqlite3CompareAffinity(p_ਸੱਜਾ, zAff[j])==SQLITE_AFF_BLOB ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
        ਜੇ( sqlite3_ਵਿਅੰਜਕ_Needs_No_Affinity_Change(p_ਸੱਜਾ, zAff[j]) ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
      }
    }
  }
  *pzAff = zAff;
  ਵਾਪਸ reg_ਅਧਾਰ;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_LIKE_DOESNT_ਮੇਲ_BLOBS
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
ਸਥਿਰ ਖਾਲੀ whereLikeOptimizationStringFixup(
  Vdbe *v,                /* prepared statement under construction */
  WhereLevel *pLevel,     /* The loop that contains the LIKE operator */
  Where_ਪਦ *p_ਪਦ        /* The upper or lower bound just coded */
){
  ਜੇ( p_ਪਦ->wtFlags & ਪਦ_LIKEOPT ){
    VdbeOp *pOp;
    ਪ੍ਰਤਿਜ੍ਞਾ( pLevel->iLikeRepCntr>0 );
    pOp = sqlite3_Vdbe_Get_ਆਖਰੀ_Op(v);
    ਪ੍ਰਤਿਜ੍ਞਾ( pOp!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pOp->opcode==OP_String8
            || p_ਪਦ->pWC->pWInfo->pParse->db->mallocFailed );
    pOp->p3 = (ਪੂਰਨ)(pLevel->iLikeRepCntr>>1);  /* Register holding counter */
    pOp->p5 = (u8)(pLevel->iLikeRepCntr&1);    /* ASC or DESC */
  }
}
#ਨਹੀਂਤਾਂ
# define whereLikeOptimizationStringFixup(A,B,C)
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_CURSOR_HINTS
/*
** Information is passed from codeCursorHint() down to individual nodes of
** the expression tree (by sqlite3WalkExpr()) using an instance of this
** structure.
*/
ਬਣਤਰ CCurHint {
  ਪੂਰਨ iTabCur;    /* Cursor for the main table */
  ਪੂਰਨ iIdxCur;    /* Cursor for the index, if pIdx!=0.  Unused otherwise */
  ਸੂਚਕਾਂਕ *pIdx;    /* The index used to access the table */
};

/*
** This function is called for every node of an expression that is a candidate
** for a cursor hint on an index cursor.  For TK_COLUMN nodes that reference
** the table CCurHint.iTabCur, verify that the same column can be
** accessed through the index.  If it cannot, then set pWalker->eCode to 1.
*/
ਸਥਿਰ ਪੂਰਨ ਕੋਡ_Cursor_Hint_Check_ਵਿਅੰਜਕ(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਬਣਤਰ CCurHint *pHint = pWalker->u.pCCurHint;
  ਪ੍ਰਤਿਜ੍ਞਾ( pHint->pIdx!=0 );
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ
   && p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==pHint->iTabCur
   && sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pHint->pIdx, p_ਵਿਅੰਜਕ->i_ਕਾਲਮ)<0
  ){
    pWalker->e_ਕੋਡ = 1;
  }
  ਵਾਪਸ WRC_Continue;
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
ਸਥਿਰ ਪੂਰਨ ਕੋਡ_Cursor_Hint_Is_Or_Function(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_IS
   || p_ਵਿਅੰਜਕ->op==TK_ISNULL || p_ਵਿਅੰਜਕ->op==TK_ISNOT
   || p_ਵਿਅੰਜਕ->op==TK_NOTNULL || p_ਵਿਅੰਜਕ->op==TK_CASE
  ){
    pWalker->e_ਕੋਡ = 1;
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op==TK_FUNCTION ){
    ਪੂਰਨ d1;
    ਅੱਖਰ d2[4];
    ਜੇ( 0==sqlite3IsLikeFunction(pWalker->pParse->db, p_ਵਿਅੰਜਕ, &d1, d2) ){
      pWalker->e_ਕੋਡ = 1;
    }
  }

  ਵਾਪਸ WRC_Continue;
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
ਸਥਿਰ ਪੂਰਨ ਕੋਡ_Cursor_Hint_Fix_ਵਿਅੰਜਕ(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਪੂਰਨ rc = WRC_Continue;
  ਪੂਰਨ reg;
  ਬਣਤਰ CCurHint *pHint = pWalker->u.pCCurHint;
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ ){
    ਜੇ( p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ!=pHint->iTabCur ){
      reg = ++pWalker->pParse->nMem;   /* Register for column value */
      reg = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pWalker->pParse, p_ਵਿਅੰਜਕ, reg);
      p_ਵਿਅੰਜਕ->op = TK_ਰਜਿਸਟਰ;
      p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = reg;
    }ਨਹੀਂਤਾਂ ਜੇ( pHint->pIdx!=0 ){
      p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = pHint->iIdxCur;
      p_ਵਿਅੰਜਕ->i_ਕਾਲਮ = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pHint->pIdx, p_ਵਿਅੰਜਕ->i_ਕਾਲਮ);
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ>=0 );
    }
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->pAggInfo ){
    rc = WRC_Prune;
    reg = ++pWalker->pParse->nMem;   /* Register for column value */
    reg = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਨਿਸ਼ਾਨਾ(pWalker->pParse, p_ਵਿਅੰਜਕ, reg);
    p_ਵਿਅੰਜਕ->op = TK_ਰਜਿਸਟਰ;
    p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = reg;
  }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op==TK_TRUEFALSE ){
    /* Do not walk disabled expressions.  tag-20230504-1 */
    ਵਾਪਸ WRC_Prune;
  }
  ਵਾਪਸ rc;
}

/*
** Insert an OP_CursorHint instruction if it is appropriate to do so.
*/
ਸਥਿਰ ਖਾਲੀ ਕੋਡ_Cursor_Hint(
  Src_ਵਸਤੂ *p_Tab_ਵਸਤੂ,  /* FROM clause item */
  WhereInfo *pWInfo,    /* The where clause */
  WhereLevel *pLevel,   /* Which loop to provide hints for */
  Where_ਪਦ *p_ਸਿਰਾ_Range  /* Hint this end-of-scan boundary term if not NULL */
){
  Parse *pParse = pWInfo->pParse;
  sqlite3 *db = pParse->db;
  Vdbe *v = pParse->pVdbe;
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = 0;
  WhereLoop *pLoop = pLevel->pWLoop;
  ਪੂਰਨ iCur;
  WhereClause *pWC;
  Where_ਪਦ *p_ਪਦ;
  ਪੂਰਨ i, j;
  ਬਣਤਰ CCurHint sHint;
  Walker sWalker;

  ਜੇ( OptimizationDisabled(db, SQLITE_CursorHints) ) ਵਾਪਸ;
  iCur = pLevel->iTabCur;
  ਪ੍ਰਤਿਜ੍ਞਾ( iCur==pWInfo->pTabList->a[pLevel->iFrom].iCursor );
  sHint.iTabCur = iCur;
  sHint.iIdxCur = pLevel->iIdxCur;
  sHint.pIdx = pLoop->u.btree.p_ਸੂਚਕਾਂਕ;
  memset(&sWalker, 0, ਆਕਾਰ(sWalker));
  sWalker.pParse = pParse;
  sWalker.u.pCCurHint = &sHint;
  pWC = &pWInfo->sWC;
  ਲਈ(i=0; i<pWC->n_ਅਧਾਰ; i++){
    p_ਪਦ = &pWC->a[i];
    ਜੇ( p_ਪਦ->wtFlags & (ਪਦ_VIRTUAL|ਪਦ_CODED) ) ਜਾਰੀ;
    ਜੇ( p_ਪਦ->prereqAll & pLevel->notReady ) ਜਾਰੀ;

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
    ਜੇ( p_Tab_ਵਸਤੂ->fg.jointype & JT_ਖੱਬਾ ){
      ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = p_ਪਦ->p_ਵਿਅੰਜਕ;
      ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON)
       || p_ਵਿਅੰਜਕ->w.iJoin!=p_Tab_ਵਸਤੂ->iCursor
      ){
        sWalker.e_ਕੋਡ = 0;
        sWalker.x_ਵਿਅੰਜਕ_Callback = ਕੋਡ_Cursor_Hint_Is_Or_Function;
        sqlite3_Walk_ਵਿਅੰਜਕ(&sWalker, p_ਪਦ->p_ਵਿਅੰਜਕ);
        ਜੇ( sWalker.e_ਕੋਡ ) ਜਾਰੀ;
      }
    }ਨਹੀਂਤਾਂ{
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਪਦ->p_ਵਿਅੰਜਕ, EP_OuterON) ) ਜਾਰੀ;
    }

    /* All terms in pWLoop->aLTerm[] except pEndRange are used to initialize
    ** the cursor.  These terms are not needed as hints for a pure range
    ** scan (that has no == terms) so omit them. */
    ਜੇ( pLoop->u.btree.nEq==0 && p_ਪਦ!=p_ਸਿਰਾ_Range ){
      ਲਈ(j=0; j<pLoop->nLTerm && pLoop->aLTerm[j]!=p_ਪਦ; j++){}
      ਜੇ( j<pLoop->nLTerm ) ਜਾਰੀ;
    }

    /* No subqueries or non-deterministic functions allowed */
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_ਸ਼ਾਮਲ_ਹੈ_Subquery(p_ਪਦ->p_ਵਿਅੰਜਕ) ) ਜਾਰੀ;

    /* For an index scan, make sure referenced columns are actually in
    ** the index. */
    ਜੇ( sHint.pIdx!=0 ){
      sWalker.e_ਕੋਡ = 0;
      sWalker.x_ਵਿਅੰਜਕ_Callback = ਕੋਡ_Cursor_Hint_Check_ਵਿਅੰਜਕ;
      sqlite3_Walk_ਵਿਅੰਜਕ(&sWalker, p_ਪਦ->p_ਵਿਅੰਜਕ);
      ਜੇ( sWalker.e_ਕੋਡ ) ਜਾਰੀ;
    }

    /* If we survive all prior tests, that means this term is worth hinting */
    p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_And(pParse, p_ਵਿਅੰਜਕ, sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਪਦ->p_ਵਿਅੰਜਕ, 0));
  }
  ਜੇ( p_ਵਿਅੰਜਕ!=0 ){
    sWalker.x_ਵਿਅੰਜਕ_Callback = ਕੋਡ_Cursor_Hint_Fix_ਵਿਅੰਜਕ;
    ਜੇ( pParse->nErr==0 ) sqlite3_Walk_ਵਿਅੰਜਕ(&sWalker, p_ਵਿਅੰਜਕ);
    sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_CursorHint,
                      (sHint.pIdx ? sHint.iIdxCur : sHint.iTabCur), 0, 0,
                      (ਅਚਲ ਅੱਖਰ*)p_ਵਿਅੰਜਕ, P4_ਵਿਅੰਜਕ);
  }
}
#ਨਹੀਂਤਾਂ
# define ਕੋਡ_Cursor_Hint(A,B,C,D)  /* No-op */
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_CURSOR_HINTS */

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
ਸਥਿਰ ਖਾਲੀ ਕੋਡ_Deferred_Seek(
  WhereInfo *pWInfo,              /* Where clause context */
  ਸੂਚਕਾਂਕ *pIdx,                    /* Index scan is using */
  ਪੂਰਨ iCur,                       /* Cursor for IPK b-tree */
  ਪੂਰਨ iIdxCur                     /* Index cursor */
){
  Parse *pParse = pWInfo->pParse; /* Parse context */
  Vdbe *v = pParse->pVdbe;        /* Vdbe to generate code within */

  ਪ੍ਰਤਿਜ੍ਞਾ( iIdxCur>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->ai_ਕਾਲਮ[pIdx->n_ਕਾਲਮ-1]==-1 );

  pWInfo->bDeferredSeek = 1;
  sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_DeferredSeek, iIdxCur, 0, iCur);
  ਜੇ( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_ਸੱਜਾ_JOIN))
   && Db_ਨਕਾਬ_All_Zero(sqlite3ParseToplevel(pParse)->write_ਨਕਾਬ)
  ){
    ਪੂਰਨ i;
    ਸਾਰਣੀ *pTab = pIdx->p_ਸਾਰਣੀ;
    u32 *ai = (u32*)sqlite3DbMallocZero(pParse->db, ਆਕਾਰ(u32)*(pTab->n_ਖਾਨਾ+1));
    ਜੇ( ai ){
      ai[0] = pTab->n_ਖਾਨਾ;
      ਲਈ(i=0; i<pIdx->n_ਕਾਲਮ-1; i++){
        ਪੂਰਨ x1, x2;
        ਪ੍ਰਤਿਜ੍ਞਾ( pIdx->ai_ਕਾਲਮ[i]<pTab->n_ਖਾਨਾ );
        x1 = pIdx->ai_ਕਾਲਮ[i];
        x2 = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, x1);
        testcase( x1!=x2 );
        ਜੇ( x1>=0 ) ai[x2+1] = i+1;
      }
      sqlite3VdbeChangeP4(v, -1, (ਅੱਖਰ*)ai, P4_INTARRAY);
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
ਸਥਿਰ ਖਾਲੀ ਕੋਡ_ਵਿਅੰਜਕ_Or_ਸਦਿਸ਼(Parse *pParse, ਵਿਅੰਜਕ *p, ਪੂਰਨ iReg, ਪੂਰਨ nReg){
  ਪ੍ਰਤਿਜ੍ਞਾ( nReg>0 );
  ਜੇ( p && sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(p) ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
    ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p) ){
      Vdbe *v = pParse->pVdbe;
      ਪੂਰਨ iSelect;
      ਪ੍ਰਤਿਜ੍ਞਾ( p->op==TK_SELECT );
      iSelect = sqlite3_ਕੋਡ_Subselect(pParse, p);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Copy, iSelect, iReg, nReg-1);
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
      ਪੂਰਨ i;
      ਅਚਲ ਵਿਅੰਜਕ_List *pList;
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p) );
      pList = p->x.pList;
      ਪ੍ਰਤਿਜ੍ਞਾ( nReg<=pList->n_ਵਿਅੰਜਕ );
      ਲਈ(i=0; i<nReg; i++){
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, pList->a[i].p_ਵਿਅੰਜਕ, iReg+i);
      }
    }
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( nReg==1 || pParse->nErr );
    sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, p, iReg);
  }
}

/*
** The pTruth expression is always true because it is the WHERE clause
** a partial index that is driving a query loop.  Look through all of the
** WHERE clause terms on the query, and if any of those terms must be
** true because pTruth is true, then mark those WHERE clause terms as
** coded.
*/
ਸਥਿਰ ਖਾਲੀ where_Apply_Partial_ਸੂਚਕਾਂਕ_Constraints(
  ਵਿਅੰਜਕ *pTruth,
  ਪੂਰਨ iTabCur,
  WhereClause *pWC
){
  ਪੂਰਨ i;
  Where_ਪਦ *p_ਪਦ;
  ਜਦੋਂ( pTruth->op==TK_AND ){
    where_Apply_Partial_ਸੂਚਕਾਂਕ_Constraints(pTruth->p_ਖੱਬਾ, iTabCur, pWC);
    pTruth = pTruth->p_ਸੱਜਾ;
  }
  ਲਈ(i=0, p_ਪਦ=pWC->a; i<pWC->n_ਪਦ; i++, p_ਪਦ++){
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ;
    ਜੇ( p_ਪਦ->wtFlags & ਪਦ_CODED ) ਜਾਰੀ;
    p_ਵਿਅੰਜਕ = p_ਪਦ->p_ਵਿਅੰਜਕ;
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(0, p_ਵਿਅੰਜਕ, pTruth, iTabCur)==0 ){
      p_ਪਦ->wtFlags |= ਪਦ_CODED;
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
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ filter_Pull_ਥੱਲੇ(
  Parse *pParse,       /* Parsing context */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  ਪੂਰਨ iLevel,          /* Which level of pWInfo->a[] should be coded */
  ਪੂਰਨ addrNxt,         /* Jump here to bypass inner loops */
  Bitmask notReady     /* Loops that are not ready */
){
  ਪੂਰਨ saved_addrBrk;
  ਜਦੋਂ( ++iLevel < pWInfo->nLevel ){
    WhereLevel *pLevel = &pWInfo->a[iLevel];
    WhereLoop *pLoop = pLevel->pWLoop;
    ਜੇ( pLevel->regFilter==0 ) ਜਾਰੀ;
    ਜੇ( pLevel->pWLoop->nSkip ) ਜਾਰੀ;
    /*         ,--- Because sqlite3ConstructBloomFilter() has will not have set
    **  vvvvv--'    pLevel->regFilter if this were true. */
    ਜੇ( NEVER(pLoop->prereq & notReady) ) ਜਾਰੀ;
    saved_addrBrk = pLevel->addrBrk;
    pLevel->addrBrk = addrNxt;
    ਜੇ( pLoop->wsFlags & WHERE_IPK ){
      Where_ਪਦ *p_ਪਦ = pLoop->aLTerm[0];
      ਪੂਰਨ regRowid;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ->p_ਵਿਅੰਜਕ!=0 );
      testcase( p_ਪਦ->wtFlags & ਪਦ_VIRTUAL );
      regRowid = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
      regRowid = ਕੋਡ_Equality_ਪਦ(pParse, p_ਪਦ, pLevel, 0, 0, regRowid);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(pParse->pVdbe, OP_MustBeInt, regRowid, addrNxt);
      VdbeCoverage(pParse->pVdbe);
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3WhereLoopBloomable(pLoop) );
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(pParse->pVdbe, OP_Filter, pLevel->regFilter,
                           addrNxt, regRowid, 1);
      VdbeCoverage(pParse->pVdbe);
    }ਨਹੀਂਤਾਂ{
      u16 nEq = pLoop->u.btree.nEq;
      ਪੂਰਨ r1;
      ਅੱਖਰ *z_ਸ਼ੁਰੂ_Aff;

      ਪ੍ਰਤਿਜ੍ਞਾ( pLoop->wsFlags & WHERE_ਸੂਚਕਾਂਕਿਤ );
      ਪ੍ਰਤਿਜ੍ਞਾ( (pLoop->wsFlags & WHERE_ਕਾਲਮ_ਵਿੱਚ)==0 );
      r1 = ਕੋਡ_All_Equality_Terms(pParse,pLevel,0,0,&z_ਸ਼ੁਰੂ_Aff);
      ਕੋਡ_Apply_Affinity(pParse, r1, nEq, z_ਸ਼ੁਰੂ_Aff);
      sqlite3DbFree(pParse->db, z_ਸ਼ੁਰੂ_Aff);
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3WhereLoopBloomable(pLoop) );
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(pParse->pVdbe, OP_Filter, pLevel->regFilter,
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
ਸਥਿਰ ਪੂਰਨ where_Loop_Is_One_ਪੰਕਤੀ(WhereLoop *pLoop){
  ਜੇ( pLoop->u.btree.p_ਸੂਚਕਾਂਕ->onError
   && pLoop->nSkip==0
   && pLoop->u.btree.nEq==pLoop->u.btree.p_ਸੂਚਕਾਂਕ->n_ਕੁੰਜੀ_ਖਾਨਾ
  ){
    ਪੂਰਨ ii;
    ਲਈ(ii=0; ii<pLoop->u.btree.nEq; ii++){
      ਜੇ( pLoop->aLTerm[ii]->eOperator & (WO_IS|WO_ISNULL) ){
        ਵਾਪਸ 0;
      }
    }
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ where_ਵਿਅੰਜਕ_Is_Ready(
  WhereInfo *pWInfo, 
  WhereLevel *pLevel,
  Src_ਵਸਤੂ *p_Tab_ਵਸਤੂ, 
  ਵਿਅੰਜਕ *pE
){
  u8 jtype = p_Tab_ਵਸਤੂ->fg.jointype;
  ਜੇ( jtype & (JT_ਖੱਬਾ|JT_LTORJ|JT_ਸੱਜਾ) ){
    ਜੇ( !ਵਿਅੰਜਕ_Has_Property(pE,EP_OuterON|EP_InnerON) ){
      /* Defer processing WHERE clause constraints until after outer
      ** join processing.  tag-20220513a */
      ਵਾਪਸ 0;
    }ਨਹੀਂਤਾਂ ਜੇ( (jtype & JT_ਖੱਬਾ) && !ਵਿਅੰਜਕ_Has_Property(pE,EP_OuterON) ){
      ਵਾਪਸ 0;
    }ਨਹੀਂਤਾਂ{
      Bitmask m = sqlite3_Where_Get_ਨਕਾਬ(&pWInfo->s_ਨਕਾਬ_ਸੈੱਟ, pE->w.iJoin);
      ਜੇ( m & pLevel->notReady ){
        /* An ON clause that is not ripe */
        ਵਾਪਸ 0;
      }
    }
  }
  ਵਾਪਸ 1;
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
ਸਥਿਰ ਪੂਰਨ ਲੱਭੋ_No_ਮੇਲ_Continuation(
  WhereInfo *pWInfo,         /* WHERE clause */
  ਪੂਰਨ iLevel                 /* Which loop we are talking about */
){
  WhereLoop *pLoop;          /* Description of this loop */
  WhereLevel *p_ਮਾਪਾ;       /* Controlling parent loop */
  SrcList *pTabList;         /* Complete FROM clause */
  Bitmask m_ਕੁੰਜੀ;              /* Mask of dependent tables */
  ਪੂਰਨ i_ਮਾਪਾ;               /* Inner-most loop affecting the key to iLevel */
  ਪੂਰਨ i;                     /* Loop counter */

  ਜੇ( iLevel<2 ){
    /* If there are less than two outer loops, this optimization is no help */
    ਵਾਪਸ 0;
  }

  /* The OP_SeekGE and OP_SeekLE opcodes only work right for this
  ** optimization on a pure equality constraint, so skip the optimization
  ** for anything else.
  */
  pLoop = pWInfo->a[iLevel].pWLoop;
  ਜੇ( (pLoop->wsFlags & WHERE_CONSTRAINT)!=WHERE_ਕਾਲਮ_EQ ) ਵਾਪਸ 0;
  ਜੇ( pLoop->wsFlags & WHERE_SKIPSCAN ) ਵਾਪਸ 0;
  ਜੇ( pLoop->wsFlags & WHERE_BIGNULL_ਛਾਂਟੋ ) ਵਾਪਸ 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( (pLoop->wsFlags & (WHERE_VIRTUALTABLE|WHERE_MULTI_OR))==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pLoop->u.btree.nEq>0 );

  /* Find a mask of all outer tables that influence the key.  At the same
  ** time, check to see if any keys values are computed from subqueries and
  ** fail out (return 0) if there are any. */
  m_ਕੁੰਜੀ = 0;
  ਲਈ(i=0; i<pLoop->nLTerm; i++){
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_ਸ਼ਾਮਲ_ਹੈ_Subquery(pLoop->aLTerm[i]->p_ਵਿਅੰਜਕ) ){
      ਵਾਪਸ 0;
    }
    m_ਕੁੰਜੀ |= pLoop->aLTerm[i]->prereq_ਸੱਜਾ;
  }

  /* Look back through outer loops.  Find the inner-most outer loop
  ** that influences the key. Make iParent be the index of that inner-most
  ** outer loop.
  */
  pTabList = pWInfo->pTabList;
  i_ਮਾਪਾ = iLevel-1;
  ਜਦੋਂ( i_ਮਾਪਾ>0 && (pWInfo->a[i_ਮਾਪਾ].pWLoop->ਨਕਾਬ_Self & m_ਕੁੰਜੀ)==0 ){
    i_ਮਾਪਾ--;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਮਾਪਾ>=0 && i_ਮਾਪਾ<=iLevel );

  /* Everything needs to be an INNER join.  Fail if there are any
  ** OUTER joins in between iParent and iLevel. */
  ਲਈ(i=i_ਮਾਪਾ; i<=iLevel; i++){
    ਜੇ( pTabList->a[pWInfo->a[i].iFrom].fg.jointype & JT_OUTER ) ਵਾਪਸ 0;
  }

  /* If we reach this point, that means the conditions of the optimization
  ** have been met.  Even so, disallow the optimization if it is disabled.
  */
  ਜੇ( OptimizationDisabled(pWInfo->pParse->db, SQLITE_Join_ਖੁੰਝ) ){
    ਵਾਪਸ 0;
  }

  /* If the inner-most outer loop that influences the key for iLevel
  ** is controlled by an automatic index and if the automatic index is
  ** the only outer loop that influences the key, then arrange to delete
  ** the automatic index entry if there is no match on the key.
  */
  p_ਮਾਪਾ = &pWInfo->a[i_ਮਾਪਾ];
  ਜੇ( (p_ਮਾਪਾ->pWLoop->wsFlags & WHERE_AUTO_ਸੂਚਕਾਂਕ)!=0
   && (m_ਕੁੰਜੀ & (m_ਕੁੰਜੀ-1))==0
  ){
    ਲਈ(i=0; i<pLoop->nLTerm; i++){
      ਵਿਅੰਜਕ *pX = pLoop->aLTerm[i]->p_ਵਿਅੰਜਕ;
      ਜੇ( !sqlite3_ਵਿਅੰਜਕ_Is_ਸਾਰਣੀ_Constant(pX->p_ਸੱਜਾ,p_ਮਾਪਾ->iTabCur,0) ) ਰੋਕੋ;
    }
    ਜੇ( i==pLoop->nLTerm ){    
      ਜੇ( p_ਮਾਪਾ->addrDeleteCont==0 ){
        p_ਮਾਪਾ->addrDeleteCont = sqlite3VdbeMakeLabel(pWInfo->pParse);
      }
      ਵਾਪਸ p_ਮਾਪਾ->addrDeleteCont;
    }
  }

  testcase( i_ਮਾਪਾ<iLevel-1 );
  ਵਾਪਸ p_ਮਾਪਾ->addrCont;
}

/*
** Generate code for the start of the iLevel-th loop in the WHERE clause
** implementation described by pWInfo.
*/
Bitmask sqlite3_Where_ਕੋਡ_One_Loop_ਸ਼ੁਰੂ(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  ਪੂਰਨ iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
){
  ਪੂਰਨ j, k;            /* Loop counters */
  ਪੂਰਨ iCur;            /* The VDBE cursor for the table */
  ਪੂਰਨ addrNxt;         /* Where to jump to continue with the next IN case */
  ਪੂਰਨ bRev;            /* True if we need to scan in reverse order */
  WhereLoop *pLoop;    /* The WhereLoop object being coded */
  WhereClause *pWC;    /* Decomposition of the entire WHERE clause */
  Where_ਪਦ *p_ਪਦ;         /* A WHERE clause term */
  sqlite3 *db;              /* Database connection */
  Src_ਵਸਤੂ *p_Tab_ਵਸਤੂ;        /* FROM clause term being coded */
  ਪੂਰਨ addrBrk;              /* Jump here to break out of the loop */
  ਪੂਰਨ addrCont;             /* Jump here to continue with next cycle */
  ਪੂਰਨ addr_ਖੁੰਝ;             /* addrCont enhanced for no-matching rows */
  ਪੂਰਨ iRowidReg = 0;        /* Rowid is stored in this register, if not zero */
  ਪੂਰਨ iReleaseReg = 0;      /* Temp register to free before returning */
  ਸੂਚਕਾਂਕ *pIdx = 0;          /* Index used by loop (if any) */
  ਪੂਰਨ iLoop;                /* Iteration of constraint generator loop */

  pWC = &pWInfo->sWC;
  db = pParse->db;
  pLoop = pLevel->pWLoop;
  p_Tab_ਵਸਤੂ = &pWInfo->pTabList->a[pLevel->iFrom];
  iCur = p_Tab_ਵਸਤੂ->iCursor;
  pLevel->notReady = notReady & ~sqlite3_Where_Get_ਨਕਾਬ(&pWInfo->s_ਨਕਾਬ_ਸੈੱਟ, iCur);
  bRev = (pWInfo->rev_ਨਕਾਬ>>iLevel)&1;
  VdbeModuleComment((v, "Begin WHERE-loop%d: %s",
                     iLevel, p_Tab_ਵਸਤੂ->pSTab->zName));
#ਜੇ WHERETRACE_ENABLED /* 0x4001 */
  ਜੇ( sqlite3WhereTrace & 0x1 ){
    sqlite3DebugPrintf("Coding level %d of %d:  notReady=%llx  iFrom=%d\n",
       iLevel, pWInfo->nLevel, (u64)notReady, pLevel->iFrom);
    ਜੇ( sqlite3WhereTrace & 0x1000 ){
      sqlite3WhereLoopPrint(pLoop, pWC);
    }
  }
  ਜੇ( (sqlite3WhereTrace & 0x4001)==0x4001 ){
    ਜੇ( iLevel==0 ){
      sqlite3DebugPrintf("WHERE clause being coded:\n");
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(0, pWInfo->pWhere, 0);
    }
    sqlite3DebugPrintf("All WHERE-clause terms before coding:\n");
    sqlite3WhereClausePrint(pWC);
  }
#ਜੇ_ਅੰਤ

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
  addr_ਖੁੰਝ = ਲੱਭੋ_No_ਮੇਲ_Continuation(pWInfo, iLevel);

  /* If this is the right table of a LEFT OUTER JOIN, allocate and
  ** initialize a memory cell that records if this table matches any
  ** row of the left table of the join.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_ਸੱਜਾ_JOIN))
       || pLevel->iFrom>0 || (p_Tab_ਵਸਤੂ[0].fg.jointype & JT_ਖੱਬਾ)==0
  );
  ਜੇ( pLevel->iFrom>0 && (p_Tab_ਵਸਤੂ[0].fg.jointype & JT_ਖੱਬਾ)!=0 ){
    pLevel->i_ਖੱਬਾ_Join = ++pParse->nMem;
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, pLevel->i_ਖੱਬਾ_Join);
    VdbeComment((v, "init LEFT JOIN match flag"));
    ਜੇ( p_Tab_ਵਸਤੂ->fg.ਰਾਹੀਂ_Coroutine ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null_ਪੰਕਤੀ, p_Tab_ਵਸਤੂ->iCursor, 1);
    }
  }

  /* Special case of a FROM clause subquery implemented as a co-routine */
  ਜੇ( p_Tab_ਵਸਤੂ->fg.ਰਾਹੀਂ_Coroutine ){
    ਪੂਰਨ regYield;
    Subquery *pSubq;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_Tab_ਵਸਤੂ->fg.isSubquery && p_Tab_ਵਸਤੂ->u4.pSubq!=0 );
    pSubq = p_Tab_ਵਸਤੂ->u4.pSubq;
    regYield = pSubq->regReturn;
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    pLevel->p2 =  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Yield, regYield, addrBrk);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", p_Tab_ਵਸਤੂ->pSTab->zName));
    pLevel->op = OP_Goto;
  }ਨਹੀਂਤਾਂ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
  ਜੇ(  (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
    /* Case 1:  The table is a virtual-table.  Use the VFilter and VNext
    **          to access the data.
    */
    ਪੂਰਨ iReg;   /* P3 Value for OP_VFilter */
    ਪੂਰਨ addr_Not_ਮਿਲਿਆ;
    ਪੂਰਨ nConstraint = pLoop->nLTerm;

    iReg = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, nConstraint+2);
    addr_Not_ਮਿਲਿਆ = pLevel->addrBrk;
    ਲਈ(j=0; j<nConstraint; j++){
      ਪੂਰਨ i_ਨਿਸ਼ਾਨਾ = iReg+j+2;
      p_ਪਦ = pLoop->aLTerm[j];
      ਜੇ( NEVER(p_ਪਦ==0) ) ਜਾਰੀ;
      ਜੇ( p_ਪਦ->eOperator & WO_ਵਿੱਚ ){
        ਜੇ( SMASKBIT32(j) & pLoop->u.vtab.m_Handle_ਵਿੱਚ ){
          ਪੂਰਨ iTab = pParse->nTab++;
          ਪੂਰਨ iCache = ++pParse->nMem;
          sqlite3_ਕੋਡ_Rhs_ਦਾ_ਵਿੱਚ(pParse, p_ਪਦ->p_ਵਿਅੰਜਕ, iTab, 0);
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_VInit_ਵਿੱਚ, iTab, i_ਨਿਸ਼ਾਨਾ, iCache);
        }ਨਹੀਂਤਾਂ{
          ਕੋਡ_Equality_ਪਦ(pParse, p_ਪਦ, pLevel, j, bRev, i_ਨਿਸ਼ਾਨਾ);
          addr_Not_ਮਿਲਿਆ = pLevel->addrNxt;
        }
      }ਨਹੀਂਤਾਂ{
        ਵਿਅੰਜਕ *p_ਸੱਜਾ = p_ਪਦ->p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
        ਕੋਡ_ਵਿਅੰਜਕ_Or_ਸਦਿਸ਼(pParse, p_ਸੱਜਾ, i_ਨਿਸ਼ਾਨਾ, 1);
        ਜੇ( p_ਪਦ->e_ਮੇਲ_Op==SQLITE_ਸੂਚਕਾਂਕ_CONSTRAINT_ਵਿੱਥ
         && pLoop->u.vtab.b_Omit_ਵਿੱਥ
        ){
          ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ->eOperator==WO_AUX );
          ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->pSelect!=0 );
          ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->pSelect->i_ਵਿੱਥ>0 );
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, pWInfo->pSelect->i_ਵਿੱਥ);
          VdbeComment((v,"Zero OFFSET counter"));
        }
      }
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, pLoop->u.vtab.idx_ਅੰਕ, iReg);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, nConstraint, iReg+1);
    /* The instruction immediately prior to OP_VFilter must be an OP_Integer
    ** that sets the "argc" value for xVFilter.  This is necessary for
    ** resolveP2() to work correctly.  See tag-20250207a. */
    sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_VFilter, iCur, addr_Not_ਮਿਲਿਆ, iReg,
                      pLoop->u.vtab.idxStr,
                      pLoop->u.vtab.needFree ? P4_DYNAMIC : P4_STATIC);
    VdbeCoverage(v);
    pLoop->u.vtab.needFree = 0;
    /* An OOM inside of AddOp4(OP_VFilter) instruction above might have freed
    ** the u.vtab.idxStr.  NULL it out to prevent a use-after-free */
    ਜੇ( db->mallocFailed ) pLoop->u.vtab.idxStr = 0;
    pLevel->p1 = iCur;
    pLevel->op = pWInfo->eOnePass ? OP_Noop : OP_VNext;
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);
    ਪ੍ਰਤਿਜ੍ਞਾ( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );

    ਲਈ(j=0; j<nConstraint; j++){
      p_ਪਦ = pLoop->aLTerm[j];
      ਜੇ( j<16 && (pLoop->u.vtab.omit_ਨਕਾਬ>>j)&1 ){
        disable_ਪਦ(pLevel, p_ਪਦ);
        ਜਾਰੀ;
      }
      ਜੇ( (p_ਪਦ->eOperator & WO_ਵਿੱਚ)!=0
       && (SMASKBIT32(j) & pLoop->u.vtab.m_Handle_ਵਿੱਚ)==0
       && !db->mallocFailed
      ){
        ਵਿਅੰਜਕ *pCompare;  /* The comparison operator */
        ਵਿਅੰਜਕ *p_ਸੱਜਾ;    /* RHS of the comparison */
        VdbeOp *pOp;     /* Opcode to access the value of the IN constraint */
        ਪੂਰਨ i_ਵਿੱਚ;         /* IN loop corresponding to the j-th constraint */

        /* Reload the constraint value into reg[iReg+j+2].  The same value
        ** was loaded into the same register prior to the OP_VFilter, but
        ** the xFilter implementation might have changed the datatype or
        ** encoding of the value in the register, so it *must* be reloaded.
        */
        ਲਈ(i_ਵਿੱਚ=0; ALWAYS(i_ਵਿੱਚ<pLevel->u.ਵਿੱਚ.n_ਵਿੱਚ); i_ਵਿੱਚ++){
          pOp = sqlite3VdbeGetOp(v, pLevel->u.ਵਿੱਚ.a_ਵਿੱਚ_Loop[i_ਵਿੱਚ].addr_ਵਿੱਚ_ਸਿਖਰ);
          ਜੇ( (pOp->opcode==OP_ਕਾਲਮ && pOp->p3==iReg+j+2)
           || (pOp->opcode==OP_Rowid && pOp->p2==iReg+j+2)
          ){
            testcase( pOp->opcode==OP_Rowid );
            sqlite3_Vdbe_ਜੋੜੋ_Op3(v, pOp->opcode, pOp->p1, pOp->p2, pOp->p3);
            ਰੋਕੋ;
          }
        }

        /* Generate code that will continue to the next row if
        ** the IN constraint is not satisfied
        */
        pCompare = sqlite3PExpr(pParse, TK_EQ, 0, 0);
        ਜੇ( !db->mallocFailed ){
          ਪੂਰਨ iFld = p_ਪਦ->u.x.i_ਖੇਤਰ;
          ਵਿਅੰਜਕ *p_ਖੱਬਾ = p_ਪਦ->p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
          ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖੱਬਾ!=0 );
          ਜੇ( iFld>0 ){
            ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖੱਬਾ->op==TK_ਸਦਿਸ਼ );
            ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਖੱਬਾ) );
            ਪ੍ਰਤਿਜ੍ਞਾ( iFld<=p_ਖੱਬਾ->x.pList->n_ਵਿਅੰਜਕ );
            pCompare->p_ਖੱਬਾ = p_ਖੱਬਾ->x.pList->a[iFld-1].p_ਵਿਅੰਜਕ;
          }ਨਹੀਂਤਾਂ{
            pCompare->p_ਖੱਬਾ = p_ਖੱਬਾ;
          }
          pCompare->p_ਸੱਜਾ = p_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ(db, TK_ਰਜਿਸਟਰ, 0);
          ਜੇ( p_ਸੱਜਾ ){
            p_ਸੱਜਾ->i_ਸਾਰਣੀ = iReg+j+2;
            sqlite3_ਵਿਅੰਜਕ_If_False(
                pParse, pCompare, pLevel->addrCont, SQLITE_JUMPIFNULL
            );
          }
          pCompare->p_ਖੱਬਾ = 0;
        }
        sqlite3_ਵਿਅੰਜਕ_Delete(db, pCompare);
      }
    }

    /* These registers need to be preserved in case there is an IN operator
    ** loop.  So we could deallocate the registers here (and potentially
    ** reuse them later) if (pLoop->wsFlags & WHERE_IN_ABLE)==0.  But it seems
    ** simpler and safer to simply not reuse the registers.
    **
    **    sqlite3ReleaseTempRange(pParse, iReg, nConstraint+2);
    */
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_VIRTUALTABLE */

  ਜੇ( (pLoop->wsFlags & WHERE_IPK)!=0
   && (pLoop->wsFlags & (WHERE_ਕਾਲਮ_ਵਿੱਚ|WHERE_ਕਾਲਮ_EQ))!=0
  ){
    /* Case 2:  We can directly reference a single row using an
    **          equality comparison against the ROWID field.  Or
    **          we reference multiple rows using a "rowid IN (...)"
    **          construct.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( pLoop->u.btree.nEq==1 );
    p_ਪਦ = pLoop->aLTerm[0];
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ->p_ਵਿਅੰਜਕ!=0 );
    testcase( p_ਪਦ->wtFlags & ਪਦ_VIRTUAL );
    iReleaseReg = ++pParse->nMem;
    iRowidReg = ਕੋਡ_Equality_ਪਦ(pParse, p_ਪਦ, pLevel, 0, bRev, iReleaseReg);
    ਜੇ( iRowidReg!=iReleaseReg ) sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, iReleaseReg);
    addrNxt = pLevel->addrNxt;
    ਜੇ( addr_ਖੁੰਝ==0 ) addr_ਖੁੰਝ = addrNxt;
    ਜੇ( pLevel->regFilter ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_MustBeInt, iRowidReg, addr_ਖੁੰਝ);
      VdbeCoverage(v);
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Filter, pLevel->regFilter, addr_ਖੁੰਝ,
                           iRowidReg, 1);
      VdbeCoverage(v);
      filter_Pull_ਥੱਲੇ(pParse, pWInfo, iLevel, addrNxt, notReady);
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_SeekRowid, iCur, addr_ਖੁੰਝ, iRowidReg);
    VdbeCoverage(v);
    pLevel->op = OP_Noop;
  }ਨਹੀਂਤਾਂ ਜੇ( (pLoop->wsFlags & WHERE_IPK)!=0
         && (pLoop->wsFlags & WHERE_ਕਾਲਮ_RANGE)!=0
  ){
    /* Case 3:  We have an inequality comparison against the ROWID field.
    */
    ਪੂਰਨ testOp = OP_Noop;
    ਪੂਰਨ ਸ਼ੁਰੂ;
    ਪੂਰਨ mem_ਸਿਰਾ_ਮੁੱਲ = 0;
    Where_ਪਦ *p_ਸ਼ੁਰੂ, *p_ਸਿਰਾ;

    j = 0;
    p_ਸ਼ੁਰੂ = p_ਸਿਰਾ = 0;
    ਜੇ( pLoop->wsFlags & WHERE_BTM_ਹੱਦਬੰਦੀ ) p_ਸ਼ੁਰੂ = pLoop->aLTerm[j++];
    ਜੇ( pLoop->wsFlags & WHERE_ਸਿਖਰ_ਹੱਦਬੰਦੀ ) p_ਸਿਰਾ = pLoop->aLTerm[j++];
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸ਼ੁਰੂ!=0 || p_ਸਿਰਾ!=0 );
    ਜੇ( bRev ){
      p_ਪਦ = p_ਸ਼ੁਰੂ;
      p_ਸ਼ੁਰੂ = p_ਸਿਰਾ;
      p_ਸਿਰਾ = p_ਪਦ;
    }
    ਕੋਡ_Cursor_Hint(p_Tab_ਵਸਤੂ, pWInfo, pLevel, p_ਸਿਰਾ);
    ਜੇ( p_ਸ਼ੁਰੂ ){
      ਵਿਅੰਜਕ *pX;             /* The expression that defines the start bound */
      ਪੂਰਨ r1, r_ਆਰਜ਼ੀ;        /* Registers for holding the start boundary */
      ਪੂਰਨ op;               /* Cursor seek operation */

      /* The following constant maps TK_xx codes into corresponding
      ** seek opcodes.  It depends on a particular ordering of TK_xx
      */
      ਅਚਲ u8 aMoveOp[] = {
           /* TK_GT */  OP_SeekGT,
           /* TK_LE */  OP_SeekLE,
           /* TK_LT */  OP_SeekLT,
           /* TK_GE */  OP_SeekGE
      };
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_LE==TK_GT+1 );      /* Make sure the ordering.. */
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_LT==TK_GT+2 );      /*  ... of the TK_xx values... */
      ਪ੍ਰਤਿਜ੍ਞਾ( TK_GE==TK_GT+3 );      /*  ... is correct. */

      ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਸ਼ੁਰੂ->wtFlags & ਪਦ_VNULL)==0 );
      testcase( p_ਸ਼ੁਰੂ->wtFlags & ਪਦ_VIRTUAL );
      pX = p_ਸ਼ੁਰੂ->p_ਵਿਅੰਜਕ;
      ਪ੍ਰਤਿਜ੍ਞਾ( pX!=0 );
      testcase( p_ਸ਼ੁਰੂ->ਖੱਬਾ_Cursor!=iCur ); /* transitive constraints */
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(pX->p_ਸੱਜਾ) ){
        r1 = r_ਆਰਜ਼ੀ = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
        ਕੋਡ_ਵਿਅੰਜਕ_Or_ਸਦਿਸ਼(pParse, pX->p_ਸੱਜਾ, r1, 1);
        testcase( pX->op==TK_GT );
        testcase( pX->op==TK_GE );
        testcase( pX->op==TK_LT );
        testcase( pX->op==TK_LE );
        op = aMoveOp[((pX->op - TK_GT - 1) & 0x3) | 0x1];
        ਪ੍ਰਤਿਜ੍ਞਾ( pX->op!=TK_GT || op==OP_SeekGE );
        ਪ੍ਰਤਿਜ੍ਞਾ( pX->op!=TK_GE || op==OP_SeekGE );
        ਪ੍ਰਤਿਜ੍ਞਾ( pX->op!=TK_LT || op==OP_SeekLE );
        ਪ੍ਰਤਿਜ੍ਞਾ( pX->op!=TK_LE || op==OP_SeekLE );
      }ਨਹੀਂਤਾਂ{
        r1 = sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਆਰਜ਼ੀ(pParse, pX->p_ਸੱਜਾ, &r_ਆਰਜ਼ੀ);
        disable_ਪਦ(pLevel, p_ਸ਼ੁਰੂ);
        op = aMoveOp[(pX->op - TK_GT)];
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, op, iCur, addrBrk, r1);
      VdbeComment((v, "pk"));
      VdbeCoverageIf(v, pX->op==TK_GT);
      VdbeCoverageIf(v, pX->op==TK_LE);
      VdbeCoverageIf(v, pX->op==TK_LT);
      VdbeCoverageIf(v, pX->op==TK_GE);
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r_ਆਰਜ਼ੀ);
    }ਨਹੀਂਤਾਂ{
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, bRev ? OP_ਆਖਰੀ : OP_Rewind, iCur, pLevel->addrHalt);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
    }
    ਜੇ( p_ਸਿਰਾ ){
      ਵਿਅੰਜਕ *pX;
      pX = p_ਸਿਰਾ->p_ਵਿਅੰਜਕ;
      ਪ੍ਰਤਿਜ੍ਞਾ( pX!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਸਿਰਾ->wtFlags & ਪਦ_VNULL)==0 );
      testcase( p_ਸਿਰਾ->ਖੱਬਾ_Cursor!=iCur ); /* Transitive constraints */
      testcase( p_ਸਿਰਾ->wtFlags & ਪਦ_VIRTUAL );
      mem_ਸਿਰਾ_ਮੁੱਲ = ++pParse->nMem;
      ਕੋਡ_ਵਿਅੰਜਕ_Or_ਸਦਿਸ਼(pParse, pX->p_ਸੱਜਾ, mem_ਸਿਰਾ_ਮੁੱਲ, 1);
      ਜੇ( 0==sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(pX->p_ਸੱਜਾ)
       && (pX->op==TK_LT || pX->op==TK_GT)
      ){
        testOp = bRev ? OP_Le : OP_Ge;
      }ਨਹੀਂਤਾਂ{
        testOp = bRev ? OP_Lt : OP_Gt;
      }
      ਜੇ( 0==sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(pX->p_ਸੱਜਾ) ){
        disable_ਪਦ(pLevel, p_ਸਿਰਾ);
      }
    }
    ਸ਼ੁਰੂ = sqlite3VdbeCurrentAddr(v);
    pLevel->op = bRev ? OP_Prev : OP_ਅਗਲਾ;
    pLevel->p1 = iCur;
    pLevel->p2 = ਸ਼ੁਰੂ;
    ਪ੍ਰਤਿਜ੍ਞਾ( pLevel->p5==0 );
    ਜੇ( testOp!=OP_Noop ){
      iRowidReg = ++pParse->nMem;
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rowid, iCur, iRowidReg);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, testOp, mem_ਸਿਰਾ_ਮੁੱਲ, addrBrk, iRowidReg);
      VdbeCoverageIf(v, testOp==OP_Le);
      VdbeCoverageIf(v, testOp==OP_Lt);
      VdbeCoverageIf(v, testOp==OP_Ge);
      VdbeCoverageIf(v, testOp==OP_Gt);
      sqlite3VdbeChangeP5(v, SQLITE_AFF_NUMERIC | SQLITE_JUMPIFNULL);
    }
  }ਨਹੀਂਤਾਂ ਜੇ( pLoop->wsFlags & WHERE_ਸੂਚਕਾਂਕਿਤ ){
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
    ਸਥਿਰ ਅਚਲ u8 a_ਸ਼ੁਰੂ_Op[] = {
      0,
      0,
      OP_Rewind,           /* 2: (!start_constraints && startEq &&  !bRev) */
      OP_ਆਖਰੀ,             /* 3: (!start_constraints && startEq &&   bRev) */
      OP_SeekGT,           /* 4: (start_constraints  && !startEq && !bRev) */
      OP_SeekLT,           /* 5: (start_constraints  && !startEq &&  bRev) */
      OP_SeekGE,           /* 6: (start_constraints  &&  startEq && !bRev) */
      OP_SeekLE            /* 7: (start_constraints  &&  startEq &&  bRev) */
    };
    ਸਥਿਰ ਅਚਲ u8 a_ਸਿਰਾ_Op[] = {
      OP_IdxGE,            /* 0: (end_constraints && !bRev && !endEq) */
      OP_IdxGT,            /* 1: (end_constraints && !bRev &&  endEq) */
      OP_IdxLE,            /* 2: (end_constraints &&  bRev && !endEq) */
      OP_IdxLT,            /* 3: (end_constraints &&  bRev &&  endEq) */
    };
    u16 nEq = pLoop->u.btree.nEq;     /* Number of == or IN terms */
    u16 nBtm = pLoop->u.btree.nBtm;   /* Length of BTM vector */
    u16 n_ਸਿਖਰ = pLoop->u.btree.n_ਸਿਖਰ;   /* Length of TOP vector */
    ਪੂਰਨ reg_ਅਧਾਰ;                 /* Base register holding constraint values */
    Where_ਪਦ *p_Range_ਸ਼ੁਰੂ = 0;  /* Inequality constraint at range start */
    Where_ਪਦ *p_Range_ਸਿਰਾ = 0;    /* Inequality constraint at range end */
    ਪੂਰਨ ਸ਼ੁਰੂ_Eq;                 /* True if range start uses ==, >= or <= */
    ਪੂਰਨ ਸਿਰਾ_Eq;                   /* True if range end uses ==, >= or <= */
    ਪੂਰਨ ਸ਼ੁਰੂ_constraints;       /* Start of range is constrained */
    ਪੂਰਨ nConstraint;             /* Number of constraint terms */
    ਪੂਰਨ iIdxCur;                 /* The VDBE cursor for the index */
    ਪੂਰਨ nExtraReg = 0;           /* Number of extra registers needed */
    ਪੂਰਨ op;                      /* Instruction opcode */
    ਅੱਖਰ *z_ਸ਼ੁਰੂ_Aff;             /* Affinity for start of range constraint */
    ਅੱਖਰ *z_ਸਿਰਾ_Aff = 0;           /* Affinity for end of range constraint */
    u8 bSeekPastNull = 0;        /* True to seek past initial nulls */
    u8 bStopAtNull = 0;          /* Add condition to terminate at NULLs */
    ਪੂਰਨ omit_ਸਾਰਣੀ;               /* True if we use the index only */
    ਪੂਰਨ regBignull = 0;          /* big-null flag register */
    ਪੂਰਨ addr_Seek_ਜਾਂਚ = 0;        /* Opcode of the OP_SeekScan, if any */

    pIdx = pLoop->u.btree.p_ਸੂਚਕਾਂਕ;
    iIdxCur = pLevel->iIdxCur;
    ਪ੍ਰਤਿਜ੍ਞਾ( nEq>=pLoop->nSkip );

    /* Find any inequality constraint terms for the start and end
    ** of the range.
    */
    j = nEq;
    ਜੇ( pLoop->wsFlags & WHERE_BTM_ਹੱਦਬੰਦੀ ){
      p_Range_ਸ਼ੁਰੂ = pLoop->aLTerm[j++];
      nExtraReg = MAX(nExtraReg, pLoop->u.btree.nBtm);
      /* Like optimization range constraints always occur in pairs */
      ਪ੍ਰਤਿਜ੍ਞਾ( (p_Range_ਸ਼ੁਰੂ->wtFlags & ਪਦ_LIKEOPT)==0 ||
              (pLoop->wsFlags & WHERE_ਸਿਖਰ_ਹੱਦਬੰਦੀ)!=0 );
    }
    ਜੇ( pLoop->wsFlags & WHERE_ਸਿਖਰ_ਹੱਦਬੰਦੀ ){
      p_Range_ਸਿਰਾ = pLoop->aLTerm[j++];
      nExtraReg = MAX(nExtraReg, pLoop->u.btree.n_ਸਿਖਰ);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_LIKE_DOESNT_ਮੇਲ_BLOBS
      ਜੇ( (p_Range_ਸਿਰਾ->wtFlags & ਪਦ_LIKEOPT)!=0 ){
        ਪ੍ਰਤਿਜ੍ਞਾ( p_Range_ਸ਼ੁਰੂ!=0 );                     /* LIKE opt constraints */
        ਪ੍ਰਤਿਜ੍ਞਾ( p_Range_ਸ਼ੁਰੂ->wtFlags & ਪਦ_LIKEOPT );   /* occur in pairs */
        pLevel->iLikeRepCntr = (u32)++pParse->nMem;
        /* iLikeRepCntr actually stores 2x the counter register number.  The
        ** bottom bit indicates whether the search order is ASC or DESC. */
        testcase( bRev );
        testcase( pIdx->a_ਛਾਂਟੋ_ਕ੍ਰਮ[nEq]==SQLITE_SO_DESC );
        ਪ੍ਰਤਿਜ੍ਞਾ( (bRev & ~1)==0 );
        pLevel->iLikeRepCntr <<=1;
        pLevel->iLikeRepCntr |= bRev ^ (pIdx->a_ਛਾਂਟੋ_ਕ੍ਰਮ[nEq]==SQLITE_SO_DESC);
      }
#ਜੇ_ਅੰਤ
      ਜੇ( p_Range_ਸ਼ੁਰੂ==0 ){
        j = pIdx->ai_ਕਾਲਮ[nEq];
        ਜੇ( (j>=0 && pIdx->p_ਸਾਰਣੀ->a_ਖਾਨਾ[j].notNull==0) || j==XN_ਵਿਅੰਜਕ ){
          bSeekPastNull = 1;
        }
      }
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( p_Range_ਸਿਰਾ==0 || (p_Range_ਸਿਰਾ->wtFlags & ਪਦ_VNULL)==0 );

    /* If the WHERE_BIGNULL_SORT flag is set, then index column nEq uses
    ** a non-default "big-null" sort (either ASC NULLS LAST or DESC NULLS
    ** FIRST). In both cases separate ordered scans are made of those
    ** index entries for which the column is null and for those for which
    ** it is not. For an ASC sort, the non-NULL entries are scanned first.
    ** For DESC, NULL entries are scanned first.
    */
    ਜੇ( (pLoop->wsFlags & (WHERE_ਸਿਖਰ_ਹੱਦਬੰਦੀ|WHERE_BTM_ਹੱਦਬੰਦੀ))==0
     && (pLoop->wsFlags & WHERE_BIGNULL_ਛਾਂਟੋ)!=0
    ){
      ਪ੍ਰਤਿਜ੍ਞਾ( bSeekPastNull==0 && nExtraReg==0 && nBtm==0 && n_ਸਿਖਰ==0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_Range_ਸਿਰਾ==0 && p_Range_ਸ਼ੁਰੂ==0 );
      testcase( pLoop->nSkip>0 );
      nExtraReg = 1;
      bSeekPastNull = 1;
      pLevel->regBignull = regBignull = ++pParse->nMem;
      ਜੇ( pLevel->i_ਖੱਬਾ_Join ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, regBignull);
      }
      pLevel->addrBignull = sqlite3VdbeMakeLabel(pParse);
    }

    /* If we are doing a reverse order scan on an ascending index, or
    ** a forward order scan on a descending index, interchange the
    ** start and end terms (pRangeStart and pRangeEnd).
    */
    ਜੇ( (nEq<pIdx->n_ਕਾਲਮ && bRev==(pIdx->a_ਛਾਂਟੋ_ਕ੍ਰਮ[nEq]==SQLITE_SO_ASC)) ){
      ਅਦਲਾ_ਬਦਲੀ(Where_ਪਦ *, p_Range_ਸਿਰਾ, p_Range_ਸ਼ੁਰੂ);
      ਅਦਲਾ_ਬਦਲੀ(u8, bSeekPastNull, bStopAtNull);
      ਅਦਲਾ_ਬਦਲੀ(u8, nBtm, n_ਸਿਖਰ);
    }

    ਜੇ( iLevel>0 && (pLoop->wsFlags & WHERE_ਵਿੱਚ_SEEKSCAN)!=0 ){
      /* In case OP_SeekScan is used, ensure that the index cursor does not
      ** point to a valid row for the first iteration of this loop. */
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Null_ਪੰਕਤੀ, iIdxCur);
    }

    /* Generate code to evaluate all constraint terms using == or IN
    ** and store the values of those terms in an array of registers
    ** starting at regBase.
    */
    ਕੋਡ_Cursor_Hint(p_Tab_ਵਸਤੂ, pWInfo, pLevel, p_Range_ਸਿਰਾ);
    reg_ਅਧਾਰ = ਕੋਡ_All_Equality_Terms(pParse,pLevel,bRev,nExtraReg,&z_ਸ਼ੁਰੂ_Aff);
    ਪ੍ਰਤਿਜ੍ਞਾ( z_ਸ਼ੁਰੂ_Aff==0 || sqlite3Strlen30(z_ਸ਼ੁਰੂ_Aff)>=nEq );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_LIKE_DOESNT_ਮੇਲ_BLOBS
    ਜੇ( pLevel->iLikeRepCntr ){
      /* The LIKE optimization loop must be the innermost of any IN(...)
      ** or skip-scan loops. Otherwise rows may not be visited in index 
      ** order. This means it needs an addrNxt label for the same reason
      ** IN(...) scans do.  */
      ਜੇ( pLevel->addrNxt==pLevel->addrBrk ){
        pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 1, (ਪੂਰਨ)(pLevel->iLikeRepCntr>>1));
      VdbeComment((v, "LIKE loop counter"));
      pLevel->addrLikeRep = sqlite3VdbeCurrentAddr(v);
    }
#ਜੇ_ਅੰਤ
    ਜੇ( z_ਸ਼ੁਰੂ_Aff && n_ਸਿਖਰ ){
      z_ਸਿਰਾ_Aff = sqlite3DbStrDup(db, &z_ਸ਼ੁਰੂ_Aff[nEq]);
    }
    addrNxt = (regBignull ? pLevel->addrBignull : pLevel->addrNxt);
    ਜੇ( addr_ਖੁੰਝ==0 ) addr_ਖੁੰਝ = addrNxt;

    testcase( p_Range_ਸ਼ੁਰੂ && (p_Range_ਸ਼ੁਰੂ->eOperator & WO_LE)!=0 );
    testcase( p_Range_ਸ਼ੁਰੂ && (p_Range_ਸ਼ੁਰੂ->eOperator & WO_GE)!=0 );
    testcase( p_Range_ਸਿਰਾ && (p_Range_ਸਿਰਾ->eOperator & WO_LE)!=0 );
    testcase( p_Range_ਸਿਰਾ && (p_Range_ਸਿਰਾ->eOperator & WO_GE)!=0 );
    ਸ਼ੁਰੂ_Eq = !p_Range_ਸ਼ੁਰੂ || p_Range_ਸ਼ੁਰੂ->eOperator & (WO_LE|WO_GE);
    ਸਿਰਾ_Eq =   !p_Range_ਸਿਰਾ || p_Range_ਸਿਰਾ->eOperator & (WO_LE|WO_GE);
    ਸ਼ੁਰੂ_constraints = p_Range_ਸ਼ੁਰੂ || nEq>0;

    /* Seek the index cursor to the start of the range. */
    nConstraint = nEq;
    ਜੇ( p_Range_ਸ਼ੁਰੂ ){
      ਵਿਅੰਜਕ *p_ਸੱਜਾ = p_Range_ਸ਼ੁਰੂ->p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
      ਕੋਡ_ਵਿਅੰਜਕ_Or_ਸਦਿਸ਼(pParse, p_ਸੱਜਾ, reg_ਅਧਾਰ+nEq, nBtm);
      whereLikeOptimizationStringFixup(v, pLevel, p_Range_ਸ਼ੁਰੂ);
      ਜੇ( (p_Range_ਸ਼ੁਰੂ->wtFlags & ਪਦ_VNULL)==0
       && sqlite3_ਵਿਅੰਜਕ_Can_Be_Null(p_ਸੱਜਾ)
      ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, reg_ਅਧਾਰ+nEq, addrNxt);
        VdbeCoverage(v);
      }
      ਜੇ( z_ਸ਼ੁਰੂ_Aff ){
        updateRangeAffinityStr(p_ਸੱਜਾ, nBtm, &z_ਸ਼ੁਰੂ_Aff[nEq]);
      }
      nConstraint += nBtm;
      testcase( p_Range_ਸ਼ੁਰੂ->wtFlags & ਪਦ_VIRTUAL );
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(p_ਸੱਜਾ)==0 ){
        disable_ਪਦ(pLevel, p_Range_ਸ਼ੁਰੂ);
      }ਨਹੀਂਤਾਂ{
        ਸ਼ੁਰੂ_Eq = 1;
      }
      bSeekPastNull = 0;
    }ਨਹੀਂਤਾਂ ਜੇ( bSeekPastNull ){
      ਸ਼ੁਰੂ_Eq = 0;
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, reg_ਅਧਾਰ+nEq);
      ਸ਼ੁਰੂ_constraints = 1;
      nConstraint++;
    }ਨਹੀਂਤਾਂ ਜੇ( regBignull ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, reg_ਅਧਾਰ+nEq);
      ਸ਼ੁਰੂ_constraints = 1;
      nConstraint++;
    }
    ਕੋਡ_Apply_Affinity(pParse, reg_ਅਧਾਰ, nConstraint - bSeekPastNull, z_ਸ਼ੁਰੂ_Aff);
    ਜੇ( pLoop->nSkip>0 && nConstraint==pLoop->nSkip ){
      /* The skip-scan logic inside the call to codeAllEqualityConstraints()
      ** above has already left the cursor sitting on the correct row,
      ** so no further seeking is needed */
    }ਨਹੀਂਤਾਂ{
      ਜੇ( regBignull ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 1, regBignull);
        VdbeComment((v, "NULL-scan pass ctr"));
      }
      ਜੇ( pLevel->regFilter ){
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3WhereLoopBloomable(pLoop) );
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Filter, pLevel->regFilter, addr_ਖੁੰਝ,
                             reg_ਅਧਾਰ, nEq);
        VdbeCoverage(v);
        filter_Pull_ਥੱਲੇ(pParse, pWInfo, iLevel, addrNxt, notReady);
      }

      op = a_ਸ਼ੁਰੂ_Op[(ਸ਼ੁਰੂ_constraints<<2) + (ਸ਼ੁਰੂ_Eq<<1) + bRev];
      ਪ੍ਰਤਿਜ੍ਞਾ( op!=0 );
      ਜੇ( (pLoop->wsFlags & WHERE_ਵਿੱਚ_SEEKSCAN)!=0 && op==OP_SeekGE ){
        ਪ੍ਰਤਿਜ੍ਞਾ( regBignull==0 );
        /* TUNING:  The OP_SeekScan opcode seeks to reduce the number
        ** of expensive seek operations by replacing a single seek with
        ** 1 or more step operations.  The question is, how many steps
        ** should we try before giving up and going with a seek.  The cost
        ** of a seek is proportional to the logarithm of the of the number
        ** of entries in the tree, so basing the number of steps to try
        ** on the estimated number of rows in the btree seems like a good
        ** guess. */
        addr_Seek_ਜਾਂਚ = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Seek_ਜਾਂਚ,
                                         (pIdx->ai_ਪੰਕਤੀ_Log_Est[0]+9)/10);
        ਜੇ( p_Range_ਸ਼ੁਰੂ || p_Range_ਸਿਰਾ ){
          sqlite3VdbeChangeP5(v, 1);
          sqlite3VdbeChangeP2(v, addr_Seek_ਜਾਂਚ, sqlite3VdbeCurrentAddr(v)+1);
          addr_Seek_ਜਾਂਚ = 0;
        }
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, op, iIdxCur, addr_ਖੁੰਝ, reg_ਅਧਾਰ, nConstraint);
      VdbeCoverage(v);
      VdbeCoverageIf(v, op==OP_Rewind);  testcase( op==OP_Rewind );
      VdbeCoverageIf(v, op==OP_ਆਖਰੀ);    testcase( op==OP_ਆਖਰੀ );
      VdbeCoverageIf(v, op==OP_SeekGT);  testcase( op==OP_SeekGT );
      VdbeCoverageIf(v, op==OP_SeekGE);  testcase( op==OP_SeekGE );
      VdbeCoverageIf(v, op==OP_SeekLE);  testcase( op==OP_SeekLE );
      VdbeCoverageIf(v, op==OP_SeekLT);  testcase( op==OP_SeekLT );

      ਪ੍ਰਤਿਜ੍ਞਾ( bSeekPastNull==0 || bStopAtNull==0 );
      ਜੇ( regBignull ){
        ਪ੍ਰਤਿਜ੍ਞਾ( bSeekPastNull==1 || bStopAtNull==1 );
        ਪ੍ਰਤਿਜ੍ਞਾ( bSeekPastNull==!bStopAtNull );
        ਪ੍ਰਤਿਜ੍ਞਾ( bStopAtNull==ਸ਼ੁਰੂ_Eq );
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        op = a_ਸ਼ੁਰੂ_Op[(nConstraint>1)*4 + 2 + bRev];
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, op, iIdxCur, addrNxt, reg_ਅਧਾਰ,
                             nConstraint-ਸ਼ੁਰੂ_Eq);
        VdbeCoverage(v);
        VdbeCoverageIf(v, op==OP_Rewind);  testcase( op==OP_Rewind );
        VdbeCoverageIf(v, op==OP_ਆਖਰੀ);    testcase( op==OP_ਆਖਰੀ );
        VdbeCoverageIf(v, op==OP_SeekGE);  testcase( op==OP_SeekGE );
        VdbeCoverageIf(v, op==OP_SeekLE);  testcase( op==OP_SeekLE );
        ਪ੍ਰਤਿਜ੍ਞਾ( op==OP_Rewind || op==OP_ਆਖਰੀ || op==OP_SeekGE || op==OP_SeekLE);
      }
    }

    /* Load the value for the inequality constraint at the end of the
    ** range (if any).
    */
    nConstraint = nEq;
    ਪ੍ਰਤਿਜ੍ਞਾ( pLevel->p2==0 );
    ਜੇ( p_Range_ਸਿਰਾ ){
      ਵਿਅੰਜਕ *p_ਸੱਜਾ = p_Range_ਸਿਰਾ->p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( addr_Seek_ਜਾਂਚ==0 );
      ਕੋਡ_ਵਿਅੰਜਕ_Or_ਸਦਿਸ਼(pParse, p_ਸੱਜਾ, reg_ਅਧਾਰ+nEq, n_ਸਿਖਰ);
      whereLikeOptimizationStringFixup(v, pLevel, p_Range_ਸਿਰਾ);
      ਜੇ( (p_Range_ਸਿਰਾ->wtFlags & ਪਦ_VNULL)==0
       && sqlite3_ਵਿਅੰਜਕ_Can_Be_Null(p_ਸੱਜਾ)
      ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, reg_ਅਧਾਰ+nEq, addrNxt);
        VdbeCoverage(v);
      }
      ਜੇ( z_ਸਿਰਾ_Aff ){
        updateRangeAffinityStr(p_ਸੱਜਾ, n_ਸਿਖਰ, z_ਸਿਰਾ_Aff);
        ਕੋਡ_Apply_Affinity(pParse, reg_ਅਧਾਰ+nEq, n_ਸਿਖਰ, z_ਸਿਰਾ_Aff);
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->mallocFailed );
      }
      nConstraint += n_ਸਿਖਰ;
      testcase( p_Range_ਸਿਰਾ->wtFlags & ਪਦ_VIRTUAL );

      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(p_ਸੱਜਾ)==0 ){
        disable_ਪਦ(pLevel, p_Range_ਸਿਰਾ);
      }ਨਹੀਂਤਾਂ{
        ਸਿਰਾ_Eq = 1;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( bStopAtNull ){
      ਜੇ( regBignull==0 ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, reg_ਅਧਾਰ+nEq);
        ਸਿਰਾ_Eq = 0;
      }
      nConstraint++;
    }
    ਜੇ( z_ਸ਼ੁਰੂ_Aff ) sqlite3DbNNFreeNN(db, z_ਸ਼ੁਰੂ_Aff);
    ਜੇ( z_ਸਿਰਾ_Aff ) sqlite3DbNNFreeNN(db, z_ਸਿਰਾ_Aff);

    /* Top of the loop body */
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    /* Check if the index cursor is past the end of the range. */
    ਜੇ( nConstraint ){
      ਜੇ( regBignull ){
        /* Except, skip the end-of-range check while doing the NULL-scan */
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IfNot, regBignull, sqlite3VdbeCurrentAddr(v)+3);
        VdbeComment((v, "If NULL-scan 2nd pass"));
        VdbeCoverage(v);
      }
      op = a_ਸਿਰਾ_Op[bRev*2 + ਸਿਰਾ_Eq];
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, op, iIdxCur, addrNxt, reg_ਅਧਾਰ, nConstraint);
      testcase( op==OP_IdxGT );  VdbeCoverageIf(v, op==OP_IdxGT );
      testcase( op==OP_IdxGE );  VdbeCoverageIf(v, op==OP_IdxGE );
      testcase( op==OP_IdxLT );  VdbeCoverageIf(v, op==OP_IdxLT );
      testcase( op==OP_IdxLE );  VdbeCoverageIf(v, op==OP_IdxLE );
      ਜੇ( addr_Seek_ਜਾਂਚ ) sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr_Seek_ਜਾਂਚ);
    }
    ਜੇ( regBignull ){
      /* During a NULL-scan, check to see if we have reached the end of
      ** the NULLs */
      ਪ੍ਰਤਿਜ੍ਞਾ( bSeekPastNull==!bStopAtNull );
      ਪ੍ਰਤਿਜ੍ਞਾ( bSeekPastNull+bStopAtNull==1 );
      ਪ੍ਰਤਿਜ੍ਞਾ( nConstraint+bSeekPastNull>0 );
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_If, regBignull, sqlite3VdbeCurrentAddr(v)+2);
      VdbeComment((v, "If NULL-scan 1st pass"));
      VdbeCoverage(v);
      op = a_ਸਿਰਾ_Op[bRev*2 + bSeekPastNull];
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, op, iIdxCur, addrNxt, reg_ਅਧਾਰ,
                           nConstraint+bSeekPastNull);
      testcase( op==OP_IdxGT );  VdbeCoverageIf(v, op==OP_IdxGT );
      testcase( op==OP_IdxGE );  VdbeCoverageIf(v, op==OP_IdxGE );
      testcase( op==OP_IdxLT );  VdbeCoverageIf(v, op==OP_IdxLT );
      testcase( op==OP_IdxLE );  VdbeCoverageIf(v, op==OP_IdxLE );
    }

    ਜੇ( (pLoop->wsFlags & WHERE_ਵਿੱਚ_EARLYOUT)!=0 ){
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Seek_ਪ੍ਰਾਪਤੀ, iIdxCur, nEq, nEq);
    }

    /* Seek the table cursor, if required */
    omit_ਸਾਰਣੀ = (pLoop->wsFlags & WHERE_IDX_ONLY)!=0
           && (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_ਸੱਜਾ_JOIN))==0;
    ਜੇ( omit_ਸਾਰਣੀ ){
      /* pIdx is a covering index.  No need to access the main table. */
    }ਨਹੀਂਤਾਂ ਜੇ( HasRowid(pIdx->p_ਸਾਰਣੀ) ){
      ਕੋਡ_Deferred_Seek(pWInfo, pIdx, iCur, iIdxCur);
    }ਨਹੀਂਤਾਂ ਜੇ( iCur!=iIdxCur ){
      ਸੂਚਕਾਂਕ *pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pIdx->p_ਸਾਰਣੀ);
      iRowidReg = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, pPk->n_ਕੁੰਜੀ_ਖਾਨਾ);
      ਲਈ(j=0; j<pPk->n_ਕੁੰਜੀ_ਖਾਨਾ; j++){
        k = sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pIdx, pPk->ai_ਕਾਲਮ[j]);
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, iIdxCur, k, iRowidReg+j);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Not_ਮਿਲਿਆ, iCur, addrCont,
                           iRowidReg, pPk->n_ਕੁੰਜੀ_ਖਾਨਾ); VdbeCoverage(v);
    }

    ਜੇ( pLevel->i_ਖੱਬਾ_Join==0 ){
      /* If a partial index is driving the loop, try to eliminate WHERE clause
      ** terms from the query that must be true due to the WHERE clause of
      ** the partial index.  This optimization does not work on an outer join,
      ** as shown by:
      **
      ** 2019-11-02 ticket 623eff57e76d45f6      (LEFT JOIN)
      ** 2025-05-29 forum post 7dee41d32506c4ae  (RIGHT JOIN)
      */
      ਜੇ( pIdx->pPartIdxWhere && pLevel->pRJ==0 ){
        where_Apply_Partial_ਸੂਚਕਾਂਕ_Constraints(pIdx->pPartIdxWhere, iCur, pWC);
      }
    }ਨਹੀਂਤਾਂ{
      testcase( pIdx->pPartIdxWhere );
      /* The following assert() is not a requirement, merely an observation:
      ** The OR-optimization doesn't work for the right hand table of
      ** a LEFT JOIN: */
      ਪ੍ਰਤਿਜ੍ਞਾ( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_ਸੱਜਾ_JOIN))==0 );
    }

    /* Record the instruction used to terminate the loop. */
    ਜੇ( (pLoop->wsFlags & WHERE_ONEROW)
     || (pLevel->u.ਵਿੱਚ.n_ਵਿੱਚ && regBignull==0 && where_Loop_Is_One_ਪੰਕਤੀ(pLoop))
    ){
      pLevel->op = OP_Noop;
    }ਨਹੀਂਤਾਂ ਜੇ( bRev ){
      pLevel->op = OP_Prev;
    }ਨਹੀਂਤਾਂ{
      pLevel->op = OP_ਅਗਲਾ;
    }
    pLevel->p1 = iIdxCur;
    pLevel->p3 = (pLoop->wsFlags&WHERE_UNQ_WANTED)!=0 ? 1:0;
    ਜੇ( (pLoop->wsFlags & WHERE_CONSTRAINT)==0 ){
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_ਕਦਮ;
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( pLevel->p5==0 );
    }
    ਜੇ( omit_ਸਾਰਣੀ ) pIdx = 0;
  }ਨਹੀਂਤਾਂ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_OR_OPTIMIZATION
  ਜੇ( pLoop->wsFlags & WHERE_MULTI_OR ){
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
    ਸੂਚਕਾਂਕ *pCov = 0;             /* Potential covering index (or NULL) */
    ਪੂਰਨ iCovCur = pParse->nTab++;  /* Cursor used for index scans (if any) */

    ਪੂਰਨ regReturn = ++pParse->nMem;           /* Register used with OP_Gosub */
    ਪੂਰਨ regRowset = 0;                        /* Register for RowSet object */
    ਪੂਰਨ regRowid = 0;                         /* Register holding rowid */
    ਪੂਰਨ iLoopBody = sqlite3VdbeMakeLabel(pParse);/* Start of loop body */
    ਪੂਰਨ iRetInit;                             /* Address of regReturn init */
    ਪੂਰਨ untestedTerms = 0;             /* Some terms not completely tested */
    ਪੂਰਨ ii;                            /* Loop counter */
    ਵਿਅੰਜਕ *p_And_ਵਿਅੰਜਕ = 0;                /* An ".. AND (...)" expression */
    ਸਾਰਣੀ *pTab = p_Tab_ਵਸਤੂ->pSTab;

    p_ਪਦ = pLoop->aLTerm[0];
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ->eOperator & WO_OR );
    ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਪਦ->wtFlags & ਪਦ_ORINFO)!=0 );
    pOrWc = &p_ਪਦ->u.pOrInfo->wc;
    pLevel->op = OP_Return;
    pLevel->p1 = regReturn;

    /* Set up a new SrcList in pOrTab containing the table being scanned
    ** by this loop in the a[0] slot and all notReady tables in a[1..] slots.
    ** This becomes the SrcList in the recursive call to sqlite3WhereBegin().
    */
    ਜੇ( pWInfo->nLevel>1 || p_Tab_ਵਸਤੂ->fg.fromExists ){
      ਪੂਰਨ nNotReady;                 /* The number of notReady tables */
      Src_ਵਸਤੂ *origSrc;              /* Original list of tables */
      nNotReady = pWInfo->nLevel - iLevel - 1;
      pOrTab = sqlite3DbMallocRawNN(db, SZ_SRCLIST(nNotReady+1));
      ਜੇ( pOrTab==0 ) ਵਾਪਸ notReady;
      pOrTab->nAlloc = (u8)(nNotReady + 1);
      pOrTab->nSrc = pOrTab->nAlloc;
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(pOrTab->a, p_Tab_ਵਸਤੂ, ਆਕਾਰ(*p_Tab_ਵਸਤੂ));
      origSrc = pWInfo->pTabList->a;
      ਲਈ(k=1; k<=nNotReady; k++){
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pOrTab->a[k], &origSrc[pLevel[k].iFrom], ਆਕਾਰ(pOrTab->a[k]));
      }

      /* Clear the fromExists flag on the OR-optimized table entry so that
      ** the calls to sqlite3WhereEnd() do not code early-exits after the
      ** first row is visited. The early exit applies to this table's
      ** overall loop - including the multiple OR branches and any WHERE
      ** conditions not passed to the sub-loops - not to the sub-loops.  */
      pOrTab->a[0].fg.fromExists = 0;
    }ਨਹੀਂਤਾਂ{
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
    ਜੇ( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
      ਜੇ( HasRowid(pTab) ){
        regRowset = ++pParse->nMem;
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, regRowset);
      }ਨਹੀਂਤਾਂ{
        ਸੂਚਕਾਂਕ *pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
        regRowset = pParse->nTab++;
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenEphemeral, regRowset, pPk->n_ਕੁੰਜੀ_ਖਾਨਾ);
        sqlite3_Vdbe_ਸੈੱਟ_P4_ਕੁੰਜੀ_Info(pParse, pPk);
      }
      regRowid = ++pParse->nMem;
    }
    iRetInit = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, regReturn);

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
    ਜੇ( pWC->n_ਪਦ>1 ){
      ਪੂਰਨ i_ਪਦ;
      ਲਈ(i_ਪਦ=0; i_ਪਦ<pWC->n_ਪਦ; i_ਪਦ++){
        ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = pWC->a[i_ਪਦ].p_ਵਿਅੰਜਕ;
        ਜੇ( &pWC->a[i_ਪਦ] == p_ਪਦ ) ਜਾਰੀ;
        testcase( pWC->a[i_ਪਦ].wtFlags & ਪਦ_VIRTUAL );
        testcase( pWC->a[i_ਪਦ].wtFlags & ਪਦ_CODED );
        testcase( pWC->a[i_ਪਦ].wtFlags & ਪਦ_ਟੁਕੜਾ );
        ਜੇ( (pWC->a[i_ਪਦ].wtFlags & (ਪਦ_VIRTUAL|ਪਦ_CODED|ਪਦ_ਟੁਕੜਾ))!=0 ){
          ਜਾਰੀ;
        }
        ਜੇ( (pWC->a[i_ਪਦ].eOperator & WO_ALL)==0 ) ਜਾਰੀ;
        ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Subquery) ) ਜਾਰੀ;  /* tag-20220303a */
        ਜੇ( where_ਵਿਅੰਜਕ_Is_Ready(pWInfo, pLevel, p_Tab_ਵਸਤੂ, p_ਵਿਅੰਜਕ)==0 ) ਜਾਰੀ;
        p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਵਿਅੰਜਕ, 0);
        p_And_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_And(pParse, p_And_ਵਿਅੰਜਕ, p_ਵਿਅੰਜਕ);
      }
      ਜੇ( p_And_ਵਿਅੰਜਕ ){
        /* The extra 0x10000 bit on the opcode is masked off and does not
        ** become part of the new Expr.op.  However, it does make the
        ** op==TK_AND comparison inside of sqlite3PExpr() false, and this
        ** prevents sqlite3PExpr() from applying the AND short-circuit
        ** optimization, which we do not want here. */
        p_And_ਵਿਅੰਜਕ = sqlite3PExpr(pParse, TK_AND|0x10000, 0, p_And_ਵਿਅੰਜਕ);
      }
    }

    /* Run a separate WHERE clause for each term of the OR clause.  After
    ** eliminating duplicates from other WHERE clauses, the action for each
    ** sub-WHERE clause is to to invoke the main loop body as a subroutine.
    */
    ExplainQueryPlan((pParse, 1, "MULTI-INDEX OR"));
    ਲਈ(ii=0; ii<pOrWc->n_ਪਦ; ii++){
      Where_ਪਦ *p_Or_ਪਦ = &pOrWc->a[ii];
      ਜੇ( p_Or_ਪਦ->ਖੱਬਾ_Cursor==iCur || (p_Or_ਪਦ->eOperator & WO_AND)!=0 ){
        WhereInfo *pSubWInfo;           /* Info for single OR-term scan */
        ਵਿਅੰਜਕ *p_Or_ਵਿਅੰਜਕ = p_Or_ਪਦ->p_ਵਿਅੰਜਕ; /* Current OR clause term */
        ਵਿਅੰਜਕ *pDelete;                  /* Local copy of OR clause term */
        ਪੂਰਨ jmp1 = 0;                   /* Address of jump operation */
        testcase( (p_Tab_ਵਸਤੂ[0].fg.jointype & JT_ਖੱਬਾ)!=0
               && !ਵਿਅੰਜਕ_Has_Property(p_Or_ਵਿਅੰਜਕ, EP_OuterON)
        ); /* See TH3 vtab25.400 and ticket 614b25314c766238 */
        pDelete = p_Or_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_Or_ਵਿਅੰਜਕ, 0);
        ਜੇ( db->mallocFailed ){
          sqlite3_ਵਿਅੰਜਕ_Delete(db, pDelete);
          ਜਾਰੀ;
        }
        ਜੇ( p_And_ਵਿਅੰਜਕ ){
          p_And_ਵਿਅੰਜਕ->p_ਖੱਬਾ = p_Or_ਵਿਅੰਜਕ;
          p_Or_ਵਿਅੰਜਕ = p_And_ਵਿਅੰਜਕ;
        }
        /* Loop through table entries that match term pOrTerm. */
        ExplainQueryPlan((pParse, 1, "INDEX %d", ii+1));
        WHERETRACE(0xffffffff, ("Subplan for OR-clause:\n"));
        pSubWInfo = sqlite3WhereBegin(pParse, pOrTab, p_Or_ਵਿਅੰਜਕ, 0, 0, 0,
                                      WHERE_OR_SUBCLAUSE, iCovCur);
        ਪ੍ਰਤਿਜ੍ਞਾ( pSubWInfo || pParse->nErr );
        ਜੇ( pSubWInfo ){
          WhereLoop *pSubLoop;
          ਪੂਰਨ addrExplain = sqlite3_Where_Explain_One_ਜਾਂਚ(
              pParse, pOrTab, &pSubWInfo->a[0], 0
          );
          sqlite3_Where_ਜੋੜੋ_ਜਾਂਚ_ਸਥਿਤੀ(v, pOrTab, &pSubWInfo->a[0], addrExplain);

          /* This is the sub-WHERE clause body.  First skip over
          ** duplicate rows from prior sub-WHERE clauses, and record the
          ** rowid (or PRIMARY KEY) for the current row so that the same
          ** row will be skipped in subsequent sub-WHERE clauses.
          */
          ਜੇ( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
            ਪੂਰਨ i_ਸੈੱਟ = ((ii==pOrWc->n_ਪਦ-1)?-1:ii);
            ਜੇ( HasRowid(pTab) ){
              sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, iCur, -1, regRowid);
              jmp1 = sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_ਪੰਕਤੀ_ਸੈੱਟ_Test, regRowset, 0,
                                          regRowid, i_ਸੈੱਟ);
              VdbeCoverage(v);
            }ਨਹੀਂਤਾਂ{
              ਸੂਚਕਾਂਕ *pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
              ਪੂਰਨ nPk = pPk->n_ਕੁੰਜੀ_ਖਾਨਾ;
              ਪੂਰਨ iPk;
              ਪੂਰਨ r;

              /* Read the PK into an array of temp registers. */
              r = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, nPk);
              ਲਈ(iPk=0; iPk<nPk; iPk++){
                ਪੂਰਨ i_ਖਾਨਾ = pPk->ai_ਕਾਲਮ[iPk];
                sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, iCur, i_ਖਾਨਾ,r+iPk);
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
              ਜੇ( i_ਸੈੱਟ ){
                jmp1 = sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_ਮਿਲਿਆ, regRowset, 0, r, nPk);
                VdbeCoverage(v);
              }
              ਜੇ( i_ਸੈੱਟ>=0 ){
                sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, r, nPk, regRowid);
                sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, regRowset, regRowid,
                                     r, nPk);
                ਜੇ( i_ਸੈੱਟ ) sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
              }

              /* Release the array of temp registers */
              sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, r, nPk);
            }
          }

          /* Invoke the main loop body as a subroutine */
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, regReturn, iLoopBody);

          /* Jump here (skipping the main loop body subroutine) if the
          ** current sub-WHERE row is a duplicate from prior sub-WHEREs. */
          ਜੇ( jmp1 ) sqlite3_Vdbe_Jump_ਇੱਥੇ(v, jmp1);

          /* The pSubWInfo->untestedTerms flag means that this OR term
          ** contained one or more AND term from a notReady table.  The
          ** terms from the notReady table could not be tested and will
          ** need to be tested later.
          */
          ਜੇ( pSubWInfo->untestedTerms ) untestedTerms = 1;

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
          ਪ੍ਰਤਿਜ੍ਞਾ( (pSubLoop->wsFlags & WHERE_AUTO_ਸੂਚਕਾਂਕ)==0 );
          ਜੇ( (pSubLoop->wsFlags & WHERE_ਸੂਚਕਾਂਕਿਤ)!=0
           && (ii==0 || pSubLoop->u.btree.p_ਸੂਚਕਾਂਕ==pCov)
           && (HasRowid(pTab) || !Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pSubLoop->u.btree.p_ਸੂਚਕਾਂਕ))
          ){
            ਪ੍ਰਤਿਜ੍ਞਾ( pSubWInfo->a[0].iIdxCur==iCovCur );
            pCov = pSubLoop->u.btree.p_ਸੂਚਕਾਂਕ;
          }ਨਹੀਂਤਾਂ{
            pCov = 0;
          }
          ਜੇ( sqlite3_Where_ਵਰਤਦਾ_Deferred_Seek(pSubWInfo) ){
            pWInfo->bDeferredSeek = 1;
          }

          /* Finish the loop through table entries that match term pOrTerm. */
          sqlite3_Where_ਸਿਰਾ(pSubWInfo);
          Explain_Query_Plan_ਕੱਢੋ(pParse);
        }
        sqlite3_ਵਿਅੰਜਕ_Delete(db, pDelete);
      }
    }
    Explain_Query_Plan_ਕੱਢੋ(pParse);
    ਪ੍ਰਤਿਜ੍ਞਾ( pLevel->pWLoop==pLoop );
    ਪ੍ਰਤਿਜ੍ਞਾ( (pLoop->wsFlags & WHERE_MULTI_OR)!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( (pLoop->wsFlags & WHERE_ਵਿੱਚ_ABLE)==0 );
    pLevel->u.pCoveringIdx = pCov;
    ਜੇ( pCov ) pLevel->iIdxCur = iCovCur;
    ਜੇ( p_And_ਵਿਅੰਜਕ ){
      p_And_ਵਿਅੰਜਕ->p_ਖੱਬਾ = 0;
      sqlite3_ਵਿਅੰਜਕ_Delete(db, p_And_ਵਿਅੰਜਕ);
    }
    sqlite3VdbeChangeP1(v, iRetInit, sqlite3VdbeCurrentAddr(v));
    sqlite3VdbeGoto(v, pLevel->addrBrk);
    sqlite3VdbeResolveLabel(v, iLoopBody);

    /* Set the P2 operand of the OP_Return opcode that will end the current
    ** loop to point to this spot, which is the top of the next containing
    ** loop.  The byte-code formatter will use that P2 value as a hint to
    ** indent everything in between the this point and the final OP_Return.
    ** See tag-20220407a in vdbe.c and shell.c */
    ਪ੍ਰਤਿਜ੍ਞਾ( pLevel->op==OP_Return );
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    ਜੇ( pWInfo->pTabList!=pOrTab ){ sqlite3DbFreeNN(db, pOrTab); }
    ਜੇ( !untestedTerms ) disable_ਪਦ(pLevel, p_ਪਦ);
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_OR_OPTIMIZATION */

  {
    /* Case 6:  There is no usable index.  We must do a complete
    **          scan of the entire table.
    */
    ਸਥਿਰ ਅਚਲ u8 a_ਕਦਮ[] = { OP_ਅਗਲਾ, OP_Prev };
    ਸਥਿਰ ਅਚਲ u8 a_ਸ਼ੁਰੂ[] = { OP_Rewind, OP_ਆਖਰੀ };
    ਪ੍ਰਤਿਜ੍ਞਾ( bRev==0 || bRev==1 );
    ਜੇ( p_Tab_ਵਸਤੂ->fg.isRecursive ){
      /* Tables marked isRecursive have only a single row that is stored in
      ** a pseudo-cursor.  No need to Rewind or Next such cursors. */
      pLevel->op = OP_Noop;
    }ਨਹੀਂਤਾਂ{
      ਕੋਡ_Cursor_Hint(p_Tab_ਵਸਤੂ, pWInfo, pLevel, 0);
      pLevel->op = a_ਕਦਮ[bRev];
      pLevel->p1 = iCur;
      pLevel->p2 = 1 + sqlite3_Vdbe_ਜੋੜੋ_Op2(v, a_ਸ਼ੁਰੂ[bRev],iCur,pLevel->addrHalt);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_ਕਦਮ;
    }
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
  pLevel->addr_ਦੇਖੋ = sqlite3VdbeCurrentAddr(v);
#ਜੇ_ਅੰਤ

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
  ਕਰੋ{
    ਪੂਰਨ i_ਅਗਲਾ = 0;                /* Next value for iLoop */
    ਲਈ(p_ਪਦ=pWC->a, j=pWC->n_ਪਦ; j>0; j--, p_ਪਦ++){
      ਵਿਅੰਜਕ *pE;
      ਪੂਰਨ skipLikeAddr = 0;
      testcase( p_ਪਦ->wtFlags & ਪਦ_VIRTUAL );
      testcase( p_ਪਦ->wtFlags & ਪਦ_CODED );
      ਜੇ( p_ਪਦ->wtFlags & (ਪਦ_VIRTUAL|ਪਦ_CODED) ) ਜਾਰੀ;
      ਜੇ( (p_ਪਦ->prereqAll & pLevel->notReady)!=0 ){
        testcase( pWInfo->untestedTerms==0
            && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 );
        pWInfo->untestedTerms = 1;
        ਜਾਰੀ;
      }
      pE = p_ਪਦ->p_ਵਿਅੰਜਕ;
      ਪ੍ਰਤਿਜ੍ਞਾ( pE!=0 );
      ਜੇ( where_ਵਿਅੰਜਕ_Is_Ready(pWInfo, pLevel, p_Tab_ਵਸਤੂ, pE)==0 ) ਜਾਰੀ;
      ਜੇ( iLoop==1 && !sqlite3_ਵਿਅੰਜਕ_Covered_By_ਸੂਚਕਾਂਕ(pE, pLevel->iTabCur, pIdx) ){
        i_ਅਗਲਾ = 2;
        ਜਾਰੀ;
      }
      ਜੇ( iLoop<3 && (p_ਪਦ->wtFlags & ਪਦ_VARSELECT) ){
        ਜੇ( i_ਅਗਲਾ==0 ) i_ਅਗਲਾ = 3;
        ਜਾਰੀ;
      }

      ਜੇ( (p_ਪਦ->wtFlags & ਪਦ_LIKECOND)!=0 ){
        /* If the TERM_LIKECOND flag is set, that means that the range search
        ** is sufficient to guarantee that the LIKE operator is true, so we
        ** can skip the call to the like(A,B) function.  But this only works
        ** for strings.  So do not skip the call to the function on the pass
        ** that compares BLOBs. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_LIKE_DOESNT_ਮੇਲ_BLOBS
        ਜਾਰੀ;
#ਨਹੀਂਤਾਂ
        u32 x = pLevel->iLikeRepCntr;
        ਜੇ( x>0 ){
          skipLikeAddr = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, (x&1)?OP_IfNot:OP_If,(ਪੂਰਨ)(x>>1));
          VdbeCoverageIf(v, (x&1)==1);
          VdbeCoverageIf(v, (x&1)==0);
        }
#ਜੇ_ਅੰਤ
      }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED /* 0xffffffff */
      ਜੇ( sqlite3WhereTrace ){
        VdbeNoopComment((v, "WhereTerm[%d] (%p) priority=%d",
                         pWC->n_ਪਦ-j, p_ਪਦ, iLoop));
      }
      ਜੇ( sqlite3WhereTrace & 0x4000 ){
        sqlite3DebugPrintf("Coding auxiliary constraint:\n");
        sqlite3_Where_ਪਦ_Print(p_ਪਦ, pWC->n_ਪਦ-j);
      }
#ਜੇ_ਅੰਤ
      sqlite3_ਵਿਅੰਜਕ_If_False(pParse, pE, addrCont, SQLITE_JUMPIFNULL);
      ਜੇ( skipLikeAddr ) sqlite3_Vdbe_Jump_ਇੱਥੇ(v, skipLikeAddr);
      p_ਪਦ->wtFlags |= ਪਦ_CODED;
    }
    iLoop = i_ਅਗਲਾ;
  }ਜਦੋਂ( iLoop>0 );

  /* Insert code to test for implied constraints based on transitivity
  ** of the "==" operator.
  **
  ** Example: If the WHERE clause contains "t1.a=t2.b" and "t2.b=123"
  ** and we are coding the t1 loop and the t2 loop has not yet coded,
  ** then we cannot use the "t1.a=t2.b" constraint, but we can code
  ** the implied "t1.a=123" constraint.
  */
  ਲਈ(p_ਪਦ=pWC->a, j=pWC->n_ਅਧਾਰ; j>0; j--, p_ਪਦ++){
    ਵਿਅੰਜਕ *pE, sEAlt;
    Where_ਪਦ *pAlt;
    ਜੇ( p_ਪਦ->wtFlags & (ਪਦ_VIRTUAL|ਪਦ_CODED) ) ਜਾਰੀ;
    ਜੇ( (p_ਪਦ->eOperator & (WO_EQ|WO_IS))==0 ) ਜਾਰੀ;
    ਜੇ( (p_ਪਦ->eOperator & WO_EQUIV)==0 ) ਜਾਰੀ;
    ਜੇ( p_ਪਦ->ਖੱਬਾ_Cursor!=iCur ) ਜਾਰੀ;
    ਜੇ( p_Tab_ਵਸਤੂ->fg.jointype & (JT_ਖੱਬਾ|JT_LTORJ|JT_ਸੱਜਾ) ) ਜਾਰੀ;
    pE = p_ਪਦ->p_ਵਿਅੰਜਕ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WHERETRACE_ENABLED /* 0x4001 */
    ਜੇ( (sqlite3WhereTrace & 0x4001)==0x4001 ){
      sqlite3DebugPrintf("Coding transitive constraint:\n");
      sqlite3_Where_ਪਦ_Print(p_ਪਦ, pWC->n_ਪਦ-j);
    }
#ਜੇ_ਅੰਤ
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(pE, EP_OuterON) );
    ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਪਦ->prereq_ਸੱਜਾ & pLevel->notReady)!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਪਦ->eOperator & (WO_OR|WO_AND))==0 );
    pAlt = sqlite3_Where_ਲੱਭੋ_ਪਦ(pWC, iCur, p_ਪਦ->u.x.ਖੱਬਾ_ਕਾਲਮ, notReady,
                    WO_EQ|WO_ਵਿੱਚ|WO_IS, 0);
    ਜੇ( pAlt==0 ) ਜਾਰੀ;
    ਜੇ( pAlt->wtFlags & (ਪਦ_CODED) ) ਜਾਰੀ;
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(pAlt->p_ਵਿਅੰਜਕ, EP_Collate) ) ਜਾਰੀ;
    ਜੇ( (pAlt->eOperator & WO_ਵਿੱਚ)
     && ਵਿਅੰਜਕ_Use_XSelect(pAlt->p_ਵਿਅੰਜਕ)
     && (pAlt->p_ਵਿਅੰਜਕ->x.pSelect->pEList->n_ਵਿਅੰਜਕ>1)
    ){
      ਜਾਰੀ;
    }
    testcase( pAlt->eOperator & WO_EQ );
    testcase( pAlt->eOperator & WO_IS );
    testcase( pAlt->eOperator & WO_ਵਿੱਚ );
    VdbeModuleComment((v, "begin transitive constraint"));
    sEAlt = *pAlt->p_ਵਿਅੰਜਕ;
    sEAlt.p_ਖੱਬਾ = pE->p_ਖੱਬਾ;
    sqlite3_ਵਿਅੰਜਕ_If_False(pParse, &sEAlt, addrCont, SQLITE_JUMPIFNULL);
    pAlt->wtFlags |= ਪਦ_CODED;
  }

  /* For a RIGHT OUTER JOIN, record the fact that the current row has
  ** been matched at least once.
  */
  ਜੇ( pLevel->pRJ ){
    ਸਾਰਣੀ *pTab;
    ਪੂਰਨ nPk;
    ਪੂਰਨ r;
    ਪੂਰਨ jmp1 = 0;
    Where_ਸੱਜਾ_Join *pRJ = pLevel->pRJ;

    /* pTab is the right-hand table of the RIGHT JOIN.  Generate code that
    ** will record that the current row of that table has been matched at
    ** least once.  This is accomplished by storing the PK for the row in
    ** both the iMatch index and the regBloom Bloom filter.
    */
    pTab = pWInfo->pTabList->a[pLevel->iFrom].pSTab;
    ਜੇ( HasRowid(pTab) ){
      r = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, 2);
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, pLevel->iTabCur, -1, r+1);
      nPk = 1;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ iPk;
      ਸੂਚਕਾਂਕ *pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
      nPk = pPk->n_ਕੁੰਜੀ_ਖਾਨਾ;
      r = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, nPk+1);
      ਲਈ(iPk=0; iPk<nPk; iPk++){
        ਪੂਰਨ i_ਖਾਨਾ = pPk->ai_ਕਾਲਮ[iPk];
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, iCur, i_ਖਾਨਾ,r+1+iPk);
      }
    }
    jmp1 = sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_ਮਿਲਿਆ, pRJ->i_ਮੇਲ, 0, r+1, nPk);
    VdbeCoverage(v);
    VdbeComment((v, "match against %s", pTab->zName));
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, r+1, nPk, r);
    sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, pRJ->i_ਮੇਲ, r, r+1, nPk);
    ਜੇ( pRJ->regBloom ){
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Filter_ਜੋੜੋ, pRJ->regBloom, 0, r+1, nPk);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
    }
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, jmp1);
    sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, r, nPk+1);
  }

  /* For a LEFT OUTER JOIN, generate code that will record the fact that
  ** at least one row of the right table has matched the left table.
  */
  ਜੇ( pLevel->i_ਖੱਬਾ_Join ){
    pLevel->addr_ਪਹਿਲਾ = sqlite3VdbeCurrentAddr(v);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 1, pLevel->i_ਖੱਬਾ_Join);
    VdbeComment((v, "record LEFT JOIN hit"));
    ਜੇ( pLevel->pRJ==0 ){
      ਜਾਓ ਕੋਡ_outer_join_constraints; /* WHERE clause constraints */
    }
  }

  ਜੇ( pLevel->pRJ ){
    /* Create a subroutine used to process all interior loops and code
    ** of the RIGHT JOIN.  During normal operation, the subroutine will
    ** be in-line with the rest of the code.  But at the end, a separate
    ** loop will run that invokes this subroutine for unmatched rows
    ** of pTab, with all tables to left begin set to NULL.
    */
    {
      Where_ਸੱਜਾ_Join *pRJ = pLevel->pRJ;
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_BeginSubrtn, 0, pRJ->regReturn);
      pRJ->addrSubrtn = sqlite3VdbeCurrentAddr(v);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pParse->withinRJSubrtn < 255 );
    pParse->withinRJSubrtn++;

    /* WHERE clause constraints must be deferred until after outer join
    ** row elimination has completed, since WHERE clause constraints apply
    ** to the results of the OUTER JOIN.  The following loop generates the
    ** appropriate WHERE clause constraint checks.  tag-20220513a.
    */
  ਕੋਡ_outer_join_constraints:
    ਲਈ(p_ਪਦ=pWC->a, j=0; j<pWC->n_ਅਧਾਰ; j++, p_ਪਦ++){
      testcase( p_ਪਦ->wtFlags & ਪਦ_VIRTUAL );
      testcase( p_ਪਦ->wtFlags & ਪਦ_CODED );
      ਜੇ( p_ਪਦ->wtFlags & (ਪਦ_VIRTUAL|ਪਦ_CODED) ) ਜਾਰੀ;
      ਜੇ( (p_ਪਦ->prereqAll & pLevel->notReady)!=0 ){
        ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->untestedTerms );
        ਜਾਰੀ;
      }
      ਜੇ( p_Tab_ਵਸਤੂ->fg.jointype & JT_LTORJ ) ਜਾਰੀ;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਦ->p_ਵਿਅੰਜਕ );
      sqlite3_ਵਿਅੰਜਕ_If_False(pParse, p_ਪਦ->p_ਵਿਅੰਜਕ, addrCont, SQLITE_JUMPIFNULL);
      p_ਪਦ->wtFlags |= ਪਦ_CODED;
    }
  }

#ਜੇ WHERETRACE_ENABLED /* 0x4001 */
  ਜੇ( sqlite3WhereTrace & 0x4000 ){
    sqlite3DebugPrintf("All WHERE-clause terms after coding level %d:\n",
                       iLevel);
    sqlite3WhereClausePrint(pWC);
  }
  ਜੇ( sqlite3WhereTrace & 0x1 ){
    sqlite3DebugPrintf("End Coding level %d:  notReady=%llx\n",
       iLevel, (u64)pLevel->notReady);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ pLevel->notReady;
}

/*
** Generate the code for the loop that finds all non-matched terms
** for a RIGHT JOIN.
*/
SQLITE_NOINLINE ਖਾਲੀ sqlite3_Where_ਸੱਜਾ_Join_Loop(
  WhereInfo *pWInfo,
  ਪੂਰਨ iLevel,
  WhereLevel *pLevel
){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  Where_ਸੱਜਾ_Join *pRJ = pLevel->pRJ;
  ਵਿਅੰਜਕ *pSubWhere = 0;
  WhereClause *pWC = &pWInfo->sWC;
  WhereInfo *pSubWInfo;
  WhereLoop *pLoop = pLevel->pWLoop;
  Src_ਵਸਤੂ *p_Tab_ਵਸਤੂ = &pWInfo->pTabList->a[pLevel->iFrom];
  SrcList *pFrom;
  ਸੰਘ {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;
  Bitmask mAll = 0;
  ਪੂਰਨ k;

  ExplainQueryPlan((pParse, 1, "RIGHT-JOIN %s", p_Tab_ਵਸਤੂ->pSTab->zName));
  sqlite3VdbeNoJumpsOutsideSubrtn(v, pRJ->addrSubrtn, pRJ->ਸਿਰਾ_Subrtn,
                                  pRJ->regReturn);
  ਲਈ(k=0; k<iLevel; k++){
    ਪੂਰਨ iIdxCur;
    Src_ਵਸਤੂ *p_ਸੱਜਾ;
    ਪ੍ਰਤਿਜ੍ਞਾ( pWInfo->a[k].pWLoop->iTab == pWInfo->a[k].iFrom );
    p_ਸੱਜਾ = &pWInfo->pTabList->a[pWInfo->a[k].iFrom];
    mAll |= pWInfo->a[k].pWLoop->ਨਕਾਬ_Self;
    ਜੇ( p_ਸੱਜਾ->fg.ਰਾਹੀਂ_Coroutine ){
      Subquery *pSubq;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸੱਜਾ->fg.isSubquery && p_ਸੱਜਾ->u4.pSubq!=0 );
      pSubq = p_ਸੱਜਾ->u4.pSubq;
      ਪ੍ਰਤਿਜ੍ਞਾ( pSubq->pSelect!=0 && pSubq->pSelect->pEList!=0 );
      sqlite3_Vdbe_ਜੋੜੋ_Op3(
          v, OP_Null, 0, pSubq->reg_ਨਤੀਜਾ,
          pSubq->reg_ਨਤੀਜਾ + pSubq->pSelect->pEList->n_ਵਿਅੰਜਕ-1
      );
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Null_ਪੰਕਤੀ, pWInfo->a[k].iTabCur);
    iIdxCur = pWInfo->a[k].iIdxCur;
    ਜੇ( iIdxCur ){
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Null_ਪੰਕਤੀ, iIdxCur);
    }
  }
  ਜੇ( (p_Tab_ਵਸਤੂ->fg.jointype & JT_LTORJ)==0 ){
    mAll |= pLoop->ਨਕਾਬ_Self;
    ਲਈ(k=0; k<pWC->n_ਪਦ; k++){
      Where_ਪਦ *p_ਪਦ = &pWC->a[k];
      ਜੇ( (p_ਪਦ->wtFlags & (ਪਦ_VIRTUAL|ਪਦ_ਟੁਕੜਾ))!=0
       && p_ਪਦ->eOperator!=WO_ROWVAL
      ){
        ਰੋਕੋ;
      }
      ਜੇ( p_ਪਦ->prereqAll & ~mAll ) ਜਾਰੀ;
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਪਦ->p_ਵਿਅੰਜਕ, EP_OuterON|EP_InnerON) ) ਜਾਰੀ;
      pSubWhere = sqlite3_ਵਿਅੰਜਕ_And(pParse, pSubWhere,
                                 sqlite3_ਵਿਅੰਜਕ_Dup(pParse->db, p_ਪਦ->p_ਵਿਅੰਜਕ, 0));
    }
  }
  ਜੇ( pLevel->iIdxCur ){
    /* pSubWhere may contain expressions that read from an index on the
    ** table on the RHS of the right join. All such expressions first test
    ** if the index is pointing at a NULL row, and if so, read from the
    ** table cursor instead. So ensure that the index cursor really is 
    ** pointing at a NULL row here, so that no values are read from it during
    ** the scan of the RHS of the RIGHT join below.  */
    sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Null_ਪੰਕਤੀ, pLevel->iIdxCur);
  }
  pFrom = &uSrc.sSrc;
  pFrom->nSrc = 1;
  pFrom->nAlloc = 1;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pFrom->a[0], p_Tab_ਵਸਤੂ, ਆਕਾਰ(Src_ਵਸਤੂ));
  pFrom->a[0].fg.jointype = 0;
  ਜੇ( pParse->withinRJSubrtn >= 100 ){
    /* This limit --------------^^^
    ** is based on an historical assert().  It is not compile-time or
    ** run-time configurable.  It could perhaps be raised as high as 254,
    ** but only an attack robot would ever do even 100 RIGHT JOINS within
    ** a single query, so we'll just leave it as it is. */
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "too many RIGHT JOINs");
    ਵਾਪਸ;
  }
  pParse->withinRJSubrtn++;
  pSubWInfo = sqlite3WhereBegin(pParse, pFrom, pSubWhere, 0, 0, 0,
                                WHERE_ਸੱਜਾ_JOIN, 0);
  ਜੇ( pSubWInfo ){
    ਪੂਰਨ iCur = pLevel->iTabCur;  /* Table on RHS of RIGHT JOIN &*/
    ਪੂਰਨ r = ++pParse->nMem;      /* Register range to hold primary key */
    ਪੂਰਨ nPk;                     /* Number of values in the primary key */
    ਪੂਰਨ r2;                      /* Register holding record for primary key */
    ਪੂਰਨ addrCont = sqlite3WhereContinueLabel(pSubWInfo);
    ਸਾਰਣੀ *pTab = p_Tab_ਵਸਤੂ->pSTab;  /* Table on RHS of RIGHT JOIN */
    ਜੇ( HasRowid(pTab) ){
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, iCur, -1, r);
      nPk = 1;
    }ਨਹੀਂਤਾਂ{
      ਪੂਰਨ iPk;
      ਸੂਚਕਾਂਕ *pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
      nPk = pPk->n_ਕੁੰਜੀ_ਖਾਨਾ;
      pParse->nMem += nPk - 1;
      ਲਈ(iPk=0; iPk<nPk; iPk++){
        ਪੂਰਨ i_ਖਾਨਾ = pPk->ai_ਕਾਲਮ[iPk];
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Get_ਕਾਲਮ_ਦਾ_ਸਾਰਣੀ(v, pTab, iCur, i_ਖਾਨਾ,r+iPk);
      }
    }

    /* Generate code that checks to see if the current row of the RHS table
    ** has appeared in any prior output row. */
    ਜੇ( pRJ->regBloom ){
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Filter, pRJ->regBloom, 
                 sqlite3VdbeCurrentAddr(v)+2, r, nPk);
      VdbeCoverage(v);
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_ਮਿਲਿਆ, pRJ->i_ਮੇਲ, addrCont, r, nPk);
    VdbeCoverage(v);
    r2 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);

    /* Generate code that inserts the PK of the RHS table into the
    ** pRH->iMatch index to indicate that the current row has appeared
    ** in the output set. */
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, r, nPk, r2);
    sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, pRJ->i_ਮੇਲ, r2, r, nPk);
    sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r2);
    ਜੇ( pRJ->regBloom ){
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Filter_ਜੋੜੋ, pRJ->regBloom, 0, r, nPk);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
    }

    /* Invoke the subroutine that actually puts the current RHS table row
    ** into the output set, with NULLs for the LHS. */
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, pRJ->regReturn, pRJ->addrSubrtn);
    sqlite3_Where_ਸਿਰਾ(pSubWInfo);
  }
  sqlite3_ਵਿਅੰਜਕ_Delete(pParse->db, pSubWhere);
  Explain_Query_Plan_ਕੱਢੋ(pParse);
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->withinRJSubrtn>0 );
  pParse->withinRJSubrtn--;
}
