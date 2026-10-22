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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "whereInt.h"

#ifndef SQLITE_OMIT_EXPLAIN

/*
** Return the name of the i-th column of the pIdx index.
*/
স্থির ধ্রুবক অক্ষর *explain_সূচক_Column_Name(সূচক *pIdx, পূর্ণসংখ্যা i){
  i = pIdx->aiColumn[i];
  যদি( i==XN_রাশিমালা ) ফেরত "<expr>";
  যদি( i==XN_ROWID ) ফেরত "rowid";
  ফেরত pIdx->pTable->aCol[i].zCnName;
}

/*
** This routine is a helper for explainIndexRange() below
**
** pStr holds the text of an expression that we are building up one term
** at a time.  This routine adds a new term to the end of the expression.
** Terms are separated by AND so add the "AND" text for second and subsequent
** terms only.
*/
স্থির শূন্য explainAppendTerm(
  StrAccum *pStr,             /* The text expression being built */
  সূচক *pIdx,                /* Index to read column names from */
  পূর্ণসংখ্যা nTerm,                  /* Number of terms */
  পূর্ণসংখ্যা iTerm,                  /* Zero-based index of first term. */
  পূর্ণসংখ্যা bAnd,                   /* Non-zero to append " AND " */
  ধ্রুবক অক্ষর *zOp             /* Name of the operator */
){
  পূর্ণসংখ্যা i;

  প্রতিজ্ঞা( nTerm>=1 );
  যদি( bAnd ) sqlite3_str_append(pStr, " AND ", 5);

  যদি( nTerm>1 ) sqlite3_str_append(pStr, "(", 1);
  জন্য(i=0; i<nTerm; i++){
    যদি( i ) sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_appendall(pStr, explain_সূচক_Column_Name(pIdx, iTerm+i));
  }
  যদি( nTerm>1 ) sqlite3_str_append(pStr, ")", 1);

  sqlite3_str_append(pStr, zOp, 1);

  যদি( nTerm>1 ) sqlite3_str_append(pStr, "(", 1);
  জন্য(i=0; i<nTerm; i++){
    যদি( i ) sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_append(pStr, "?", 1);
  }
  যদি( nTerm>1 ) sqlite3_str_append(pStr, ")", 1);
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
স্থির শূন্য explain_সূচক_Range(StrAccum *pStr, WhereLoop *pLoop){
  সূচক *p_সূচক = pLoop->u.btree.p_সূচক;
  u16 nEq = pLoop->u.btree.nEq;
  u16 nSkip = pLoop->nSkip;
  পূর্ণসংখ্যা i, j;

  যদি( nEq==0 && (pLoop->wsFlags&(WHERE_BTM_LIMIT|WHERE_শীর্ষ_LIMIT))==0 ) ফেরত;
  sqlite3_str_append(pStr, " (", 2);
  জন্য(i=0; i<nEq; i++){
    ধ্রুবক অক্ষর *z = explain_সূচক_Column_Name(p_সূচক, i);
    যদি( i ) sqlite3_str_append(pStr, " AND ", 5);
    sqlite3_str_appendf(pStr, i>=nSkip ? "%s=?" : "ANY(%s)", z);
  }

  j = i;
  যদি( pLoop->wsFlags&WHERE_BTM_LIMIT ){
    explainAppendTerm(pStr, p_সূচক, pLoop->u.btree.nBtm, j, i, ">");
    i = 1;
  }
  যদি( pLoop->wsFlags&WHERE_শীর্ষ_LIMIT ){
    explainAppendTerm(pStr, p_সূচক, pLoop->u.btree.n_শীর্ষ, j, i, "<");
  }
  sqlite3_str_append(pStr, ")", 1);
}

/*
** This function sets the P4 value of an existing OP_Explain opcode to
** text describing the loop in pLevel. If the OP_Explain opcode already has
** a P4 value, it is freed before it is overwritten.
*/
শূন্য sqlite3WhereAddExplainText(
  Parse *pParse,                  /* Parse context */
  পূর্ণসংখ্যা addr,                       /* Address of OP_Explain opcode */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
#যদি !defined(SQLITE_DEBUG)
  যদি( sqlite3ParseToplevel(pParse)->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#endif
  {
    VdbeOp *pOp = sqlite3VdbeGetOp(pParse->pVdbe, addr);
    SrcItem *pItem = &pTabList->a[pLevel->iFrom];
    sqlite3 *db = pParse->db;     /* Database handle */
    পূর্ণসংখ্যা is_অনুসন্ধান;                 /* True for a SEARCH. False for SCAN. */
    WhereLoop *pLoop;             /* The controlling WhereLoop object */
    u32 flags;                    /* Flags that describe this loop */
#যদি defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_EXPLAIN)
    অক্ষর *z_বার্তা;                   /* Text to add to EQP output */
#endif
    StrAccum str;                 /* EQP output string */
    অক্ষর zBuf[100];               /* Initial space for EQP output string */

    যদি( db->mallocFailed ) ফেরত;

    pLoop = pLevel->pWLoop;
    flags = pLoop->wsFlags;

    is_অনুসন্ধান = (flags&(WHERE_BTM_LIMIT|WHERE_শীর্ষ_LIMIT))!=0
            || ((flags&WHERE_VIRTUALTABLE)==0 && (pLoop->u.btree.nEq>0))
            || (wctrlFlags&(WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX));

    sqlite3StrAccumInit(&str, db, zBuf, আকার(zBuf), SQLITE_MAX_দৈর্ঘ্য);
    str.printfFlags = SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&str, "%s %S%s",
       is_অনুসন্ধান ? "SEARCH" : "SCAN",
       pItem,
       pItem->fg.fromExists ? " EXISTS" : "");
    যদি( (flags & (WHERE_IPK|WHERE_VIRTUALTABLE))==0 ){
      ধ্রুবক অক্ষর *zFmt = 0;
      সূচক *pIdx;

      প্রতিজ্ঞা( pLoop->u.btree.p_সূচক!=0 );
      pIdx = pLoop->u.btree.p_সূচক;
      প্রতিজ্ঞা( !(flags&WHERE_AUTO_সূচক) || (flags&WHERE_IDX_ONLY) );
      যদি( !HasRowid(pItem->pSTab) && Is_Primary_চাবি_সূচক(pIdx) ){
        যদি( is_অনুসন্ধান ){
          zFmt = "PRIMARY KEY";
        }
      }অন্যথা যদি( flags & WHERE_PARTIALIDX ){
        zFmt = "AUTOMATIC PARTIAL COVERING INDEX";
      }অন্যথা যদি( flags & WHERE_AUTO_সূচক ){
        zFmt = "AUTOMATIC COVERING INDEX";
      }অন্যথা যদি( flags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
        zFmt = "COVERING INDEX %s";
      }অন্যথা{
        zFmt = "INDEX %s";
      }
      যদি( zFmt ){
        sqlite3_str_append(&str, " USING ", 7);
        sqlite3_str_appendf(&str, zFmt, pIdx->zName);
        explain_সূচক_Range(&str, pLoop);
      }
    }অন্যথা যদি( (flags & WHERE_IPK)!=0 && (flags & WHERE_CONSTRAINT)!=0 ){
      অক্ষর cRangeOp;
#যদি 0  /* Better output, but breaks many tests */
      ধ্রুবক Table *pTab = pItem->pTab;
      ধ্রুবক অক্ষর *zRowid = pTab->iPKey>=0 ? pTab->aCol[pTab->iPKey].zCnName:
                              "rowid";
#অন্যথা
      ধ্রুবক অক্ষর *zRowid = "rowid";
#endif
      sqlite3_str_appendf(&str, " USING INTEGER PRIMARY KEY (%s", zRowid);
      যদি( flags&(WHERE_COLUMN_EQ|WHERE_COLUMN_IN) ){
        cRangeOp = '=';
      }অন্যথা যদি( (flags&WHERE_BOTH_LIMIT)==WHERE_BOTH_LIMIT ){
        sqlite3_str_appendf(&str, ">? AND %s", zRowid);
        cRangeOp = '<';
      }অন্যথা যদি( flags&WHERE_BTM_LIMIT ){
        cRangeOp = '>';
      }অন্যথা{
        প্রতিজ্ঞা( flags&WHERE_শীর্ষ_LIMIT);
        cRangeOp = '<';
      }
      sqlite3_str_appendf(&str, "%c?)", cRangeOp);
    }
#ifndef SQLITE_OMIT_VIRTUALTABLE
    অন্যথা যদি( (flags & WHERE_VIRTUALTABLE)!=0 ){
      sqlite3_str_appendall(&str, " VIRTUAL TABLE INDEX ");
      sqlite3_str_appendf(&str,
                  pLoop->u.vtab.b_Idx_অঙ্ক_Hex ? "0x%x:%s" : "%d:%s",
                  pLoop->u.vtab.idx_অঙ্ক, pLoop->u.vtab.idxStr);
    }
#endif
    যদি( pItem->fg.jointype & JT_বাম ){
      sqlite3_str_appendf(&str, " LEFT-JOIN");
    }
#ifdef SQLITE_EXPLAIN_ESTIMATED_ROWS
    যদি( pLoop->nOut>=10 ){
      sqlite3_str_appendf(&str, " (~%llu rows)",
             sqlite3LogEstToInt(pLoop->nOut));
    }অন্যথা{
      sqlite3_str_append(&str, " (~1 row)", 9);
    }
#endif
#যদি defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_EXPLAIN)
    z_বার্তা = sqlite3StrAccumFinish(&str);
    sqlite3ExplainBreakpoint("",z_বার্তা);
#endif

    প্রতিজ্ঞা( pOp->opcode==OP_Explain );
    প্রতিজ্ঞা( pOp->p4type==P4_DYNAMIC || pOp->p4.z==0 );
    sqlite3DbFree(db, pOp->p4.z);
    pOp->p4type = P4_DYNAMIC;
    pOp->p4.z = sqlite3StrAccumFinish(&str);
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
পূর্ণসংখ্যা sqlite3WhereExplainOneScan(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
  পূর্ণসংখ্যা ret = 0;
#যদি !defined(SQLITE_DEBUG)
  যদি( sqlite3ParseToplevel(pParse)->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#endif
  {
    যদি( (pLevel->pWLoop->wsFlags & WHERE_MULTI_OR)==0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
    ){
      Vdbe *v = pParse->pVdbe;
      পূর্ণসংখ্যা addr = sqlite3VdbeCurrentAddr(v);
      ret = sqlite3VdbeAddOp3(
          v, OP_Explain, addr, pParse->addrExplain, pLevel->pWLoop->rRun
      );
      sqlite3WhereAddExplainText(pParse, addr, pTabList, pLevel, wctrlFlags);
    }
  }
  ফেরত ret;
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
পূর্ণসংখ্যা sqlite3WhereExplainBloomFilter(
  ধ্রুবক Parse *pParse,               /* Parse context */
  ধ্রুবক WhereInfo *pWInfo,           /* WHERE clause */
  ধ্রুবক WhereLevel *pLevel           /* Bloom filter on this level */
){
  পূর্ণসংখ্যা ret = 0;
  SrcItem *pItem = &pWInfo->pTabList->a[pLevel->iFrom];
  Vdbe *v = pParse->pVdbe;      /* VM being constructed */
  sqlite3 *db = pParse->db;     /* Database handle */
  অক্ষর *z_বার্তা;                   /* Text to add to EQP output */
  পূর্ণসংখ্যা i;                        /* Loop counter */
  WhereLoop *pLoop;             /* The where loop */
  StrAccum str;                 /* EQP output string */
  অক্ষর zBuf[100];               /* Initial space for EQP output string */

  sqlite3StrAccumInit(&str, db, zBuf, আকার(zBuf), SQLITE_MAX_দৈর্ঘ্য);
  str.printfFlags = SQLITE_PRINTF_INTERNAL;
  sqlite3_str_appendf(&str, "BLOOM FILTER ON %S (", pItem);
  pLoop = pLevel->pWLoop;
  যদি( pLoop->wsFlags & WHERE_IPK ){
    ধ্রুবক Table *pTab = pItem->pSTab;
    যদি( pTab->iPKey>=0 ){
      sqlite3_str_appendf(&str, "%s=?", pTab->aCol[pTab->iPKey].zCnName);
    }অন্যথা{
      sqlite3_str_appendf(&str, "rowid=?");
    }
  }অন্যথা{
    জন্য(i=pLoop->nSkip; i<pLoop->u.btree.nEq; i++){
      ধ্রুবক অক্ষর *z = explain_সূচক_Column_Name(pLoop->u.btree.p_সূচক, i);
      যদি( i>pLoop->nSkip ) sqlite3_str_append(&str, " AND ", 5);
      sqlite3_str_appendf(&str, "%s=?", z);
    }
  }
  sqlite3_str_append(&str, ")", 1);
  z_বার্তা = sqlite3StrAccumFinish(&str);
  ret = sqlite3VdbeAddOp4(v, OP_Explain, sqlite3VdbeCurrentAddr(v),
                          pParse->addrExplain, 0, z_বার্তা,P4_DYNAMIC);

  sqlite3VdbeScanStatus(v, sqlite3VdbeCurrentAddr(v)-1, 0, 0, 0, 0);
  ফেরত ret;
}
#endif /* SQLITE_OMIT_EXPLAIN */

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Configure the VM passed as the first argument with an
** sqlite3_stmt_scanstatus() entry corresponding to the scan used to
** implement level pLvl. Argument pSrclist is a pointer to the FROM
** clause that the scan reads data from.
**
** If argument addrExplain is not 0, it must be the address of an
** OP_Explain instruction that describes the same loop.
*/
শূন্য sqlite3WhereAddScanStatus(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  পূর্ণসংখ্যা addrExplain                 /* Address of OP_Explain (or 0) */
){
  যদি( IS_STMT_SCANSTATUS( sqlite3VdbeDb(v) ) ){
    ধ্রুবক অক্ষর *zObj = 0;
    WhereLoop *pLoop = pLvl->pWLoop;
    পূর্ণসংখ্যা wsFlags = pLoop->wsFlags;
    পূর্ণসংখ্যা viaCoroutine = 0;

    যদি( (wsFlags & WHERE_VIRTUALTABLE)==0  &&  pLoop->u.btree.p_সূচক!=0 ){
      zObj = pLoop->u.btree.p_সূচক->zName;
    }অন্যথা{
      zObj = pSrclist->a[pLvl->iFrom].zName;
      viaCoroutine = pSrclist->a[pLvl->iFrom].fg.viaCoroutine;
    }
    sqlite3VdbeScanStatus(
        v, addrExplain, pLvl->addrBody, pLvl->addrVisit, pLoop->nOut, zObj
    );

    যদি( viaCoroutine==0 ){
      যদি( (wsFlags & (WHERE_MULTI_OR|WHERE_AUTO_সূচক))==0 ){
        sqlite3VdbeScanStatusRange(v, addrExplain, -1, pLvl->iTabCur);
      }
      যদি( wsFlags & WHERE_INDEXED ){
        sqlite3VdbeScanStatusRange(v, addrExplain, -1, pLvl->iIdxCur);
      }
    }অন্যথা{
      পূর্ণসংখ্যা addr;
      VdbeOp *pOp;
      প্রতিজ্ঞা( pSrclist->a[pLvl->iFrom].fg.isSubquery );
      addr = pSrclist->a[pLvl->iFrom].u4.pSubq->addrFillSub;
      pOp = sqlite3VdbeGetOp(v, addr-1);
      প্রতিজ্ঞা( sqlite3VdbeDb(v)->mallocFailed || pOp->opcode==OP_InitCoroutine );
      প্রতিজ্ঞা( sqlite3VdbeDb(v)->mallocFailed || pOp->p2>addr );
      sqlite3VdbeScanStatusRange(v, addrExplain, addr, pOp->p2-1);
    }
  }
}
#endif


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
স্থির শূন্য disableTerm(WhereLevel *pLevel, WhereTerm *pTerm){
  পূর্ণসংখ্যা nLoop = 0;
  প্রতিজ্ঞা( pTerm!=0 );
  যখন( (pTerm->wtFlags & TERM_CODED)==0
      && (pLevel->i_বাম_Join==0 || রাশিমালা_Has_Property(pTerm->p_রাশিমালা, EP_OuterON))
      && (pLevel->notReady & pTerm->prereqAll)==0
  ){
    যদি( nLoop && (pTerm->wtFlags & TERM_LIKE)!=0 ){
      pTerm->wtFlags |= TERM_LIKECOND;
    }অন্যথা{
      pTerm->wtFlags |= TERM_CODED;
    }
#ifdef WHERETRACE_ENABLED
    যদি( (sqlite3WhereTrace & 0x4001)==0x4001 ){
      sqlite3DebugPrintf("DISABLE-");
      sqlite3WhereTermPrint(pTerm, (পূর্ণসংখ্যা)(pTerm - (pTerm->pWC->a)));
    }
#endif
    যদি( pTerm->iParent<0 ) বিরতি;
    pTerm = &pTerm->pWC->a[pTerm->iParent];
    প্রতিজ্ঞা( pTerm!=0 );
    pTerm->nChild--;
    যদি( pTerm->nChild!=0 ) বিরতি;
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
স্থির শূন্য codeApplyAffinity(Parse *pParse, পূর্ণসংখ্যা base, পূর্ণসংখ্যা n, অক্ষর *zAff){
  Vdbe *v = pParse->pVdbe;
  যদি( zAff==0 ){
    প্রতিজ্ঞা( pParse->db->mallocFailed );
    ফেরত;
  }
  প্রতিজ্ঞা( v!=0 );

  /* Adjust base and n to skip over SQLITE_AFF_BLOB and SQLITE_AFF_NONE
  ** entries at the beginning and end of the affinity string.
  */
  প্রতিজ্ঞা( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  যখন( n>0 && zAff[0]<=SQLITE_AFF_BLOB ){
    n--;
    base++;
    zAff++;
  }
  যখন( n>1 && zAff[n-1]<=SQLITE_AFF_BLOB ){
    n--;
  }

  /* Code the OP_Affinity opcode if there is anything left to do. */
  যদি( n>0 ){
    sqlite3VdbeAddOp4(v, OP_Affinity, base, n, 0, zAff, n);
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
স্থির শূন্য updateRangeAffinityStr(
  রাশিমালা *p_ডান,                   /* RHS of comparison */
  পূর্ণসংখ্যা n,                          /* Number of vector elements in comparison */
  অক্ষর *zAff                      /* Affinity string to modify */
){
  পূর্ণসংখ্যা i;
  জন্য(i=0; i<n; i++){
    রাশিমালা *p = sqlite3VectorFieldSubexpr(p_ডান, i);
    যদি( sqlite3CompareAffinity(p, zAff[i])==SQLITE_AFF_BLOB
     || sqlite3_রাশিমালা_Needs_No_Affinity_Change(p, zAff[i])
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
স্থির শূন্য adjustOrderByCol(রাশিমালা_List *pOrderBy, রাশিমালা_List *pEList){
  পূর্ণসংখ্যা i, j;
  যদি( pOrderBy==0 ) ফেরত;
  জন্য(i=0; i<pOrderBy->n_রাশিমালা; i++){
    পূর্ণসংখ্যা t = pOrderBy->a[i].u.x.iOrderByCol;
    যদি( t==0 ) চালু;
    জন্য(j=0; j<pEList->n_রাশিমালা; j++){
      যদি( pEList->a[j].u.x.iOrderByCol==t ){
        pOrderBy->a[i].u.x.iOrderByCol = j+1;
        বিরতি;
      }
    }
    যদি( j>=pEList->n_রাশিমালা ){
      pOrderBy->a[i].u.x.iOrderByCol = 0;
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
স্থির রাশিমালা *removeUnindexableInClauseTerms(
  Parse *pParse,        /* The parsing context */
  পূর্ণসংখ্যা iEq,              /* Look at loop terms starting here */
  WhereLoop *pLoop,     /* The current loop */
  রাশিমালা *pX              /* The IN expression to be reduced */
){
  sqlite3 *db = pParse->db;
  Select *pSelect;            /* Pointer to the SELECT on the RHS */
  রাশিমালা *pNew;
  pNew = sqlite3_রাশিমালা_Dup(db, pX, 0);
  যদি( db->mallocFailed==0 ){
    জন্য(pSelect=pNew->x.pSelect; pSelect; pSelect=pSelect->pPrior){
      রাশিমালা_List *pOrigRhs;         /* Original unmodified RHS */
      রাশিমালা_List *pOrigLhs = 0;     /* Original unmodified LHS */
      রাশিমালা_List *pRhs = 0;         /* New RHS after modifications */
      রাশিমালা_List *pLhs = 0;         /* New LHS after mods */
      পূর্ণসংখ্যা i;                      /* Loop counter */
      পূর্ণসংখ্যা nRhs = 0;               /* Number of RHS terms added so far */

      প্রতিজ্ঞা( রাশিমালা_Use_XSelect(pNew) );
      pOrigRhs = pSelect->pEList;
      প্রতিজ্ঞা( pNew->p_বাম!=0 );
      প্রতিজ্ঞা( রাশিমালা_Use_XList(pNew->p_বাম) );
      যদি( pSelect==pNew->x.pSelect ){
        pOrigLhs = pNew->p_বাম->x.pList;
      }
      জন্য(i=iEq; i<pLoop->nLTerm; i++){
        যদি( pLoop->aLTerm[i]->p_রাশিমালা==pX ){
          পূর্ণসংখ্যা iField;
          প্রতিজ্ঞা( (pLoop->aLTerm[i]->eOperator & (WO_OR|WO_AND))==0 );
          iField = pLoop->aLTerm[i]->u.x.iField - 1;
          যদি( iField!=nRhs ){
            রাশিমালা_Clear_Property(pNew, EP_Subrtn);
          }
          যদি( NEVER(pOrigRhs->a[iField].p_রাশিমালা==0) ){
            চালু; /* Duplicate PK column */
          }
          pRhs = sqlite3_রাশিমালা_List_Append(pParse, pRhs, pOrigRhs->a[iField].p_রাশিমালা);
          pOrigRhs->a[iField].p_রাশিমালা = 0;
          যদি( pRhs ) pRhs->a[pRhs->n_রাশিমালা-1].u.x.iOrderByCol = iField+1;
          যদি( pOrigLhs ){
            প্রতিজ্ঞা( pOrigLhs->a[iField].p_রাশিমালা!=0 );
            pLhs = sqlite3_রাশিমালা_List_Append(pParse,pLhs,pOrigLhs->a[iField].p_রাশিমালা);
            pOrigLhs->a[iField].p_রাশিমালা = 0;
          }
          nRhs++;
        }
      }
      যদি( nRhs<pOrigRhs->n_রাশিমালা ){
        রাশিমালা_Clear_Property(pNew, EP_Subrtn); /* See bug 2026-10-16T22:07:15Z */
      }
      sqlite3_রাশিমালা_List_Delete(db, pOrigRhs);
      যদি( pOrigLhs ){
        sqlite3_রাশিমালা_List_Delete(db, pOrigLhs);
        pNew->p_বাম->x.pList = pLhs;
      }
      pSelect->pEList = pRhs;
      pSelect->selId = ++pParse->nSelect; /* Req'd for SubrtnSig validity */
      যদি( pLhs && pLhs->n_রাশিমালা==1 ){
        /* Take care here not to generate a TK_VECTOR containing only a
        ** single value. Since the parser never creates such a vector, some
        ** of the subroutines do not handle this case.  */
        রাশিমালা *p = pLhs->a[0].p_রাশিমালা;
        pLhs->a[0].p_রাশিমালা = 0;
        sqlite3_রাশিমালা_Delete(db, pNew->p_বাম);
        pNew->p_বাম = p;
      }

      /* If either the ORDER BY clause or the GROUP BY clause contains
      ** references to result-set columns, those references might now be
      ** obsolete.  So fix them up.
      */
      প্রতিজ্ঞা( pRhs!=0 || db->mallocFailed );
      যদি( pRhs ){
        adjustOrderByCol(pSelect->pOrderBy, pRhs);
        adjustOrderByCol(pSelect->pGroupBy, pRhs);
        জন্য(i=0; i<pRhs->n_রাশিমালা; i++) pRhs->a[i].u.x.iOrderByCol = 0;
      }

#যদি 0
      printf("For indexing, change the IN expr:\n");
      sqlite3_বৃক্ষ_View_রাশিমালা(0, pX, 0);
      printf("Into:\n");
      sqlite3_বৃক্ষ_View_রাশিমালা(0, pNew, 0);
#endif
    }
  }
  ফেরত pNew;
}


#ifndef SQLITE_OMIT_SUBQUERY
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
স্থির SQLITE_NOINLINE শূন্য codeINTerm(
  Parse *pParse,      /* The parsing context */
  WhereTerm *pTerm,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  পূর্ণসংখ্যা iEq,            /* Index of the equality term within this level */
  পূর্ণসংখ্যা bRev,           /* True for reverse-order IN operations */
  পূর্ণসংখ্যা iTarget         /* Attempt to leave results in this register */
){
  রাশিমালা *pX = pTerm->p_রাশিমালা;
  পূর্ণসংখ্যা eType = IN_সূচক_NOOP;
  পূর্ণসংখ্যা iTab;
  কাঠামো InLoop *pIn;
  WhereLoop *pLoop = pLevel->pWLoop;
  Vdbe *v = pParse->pVdbe;
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা nEq = 0;
  পূর্ণসংখ্যা *aiMap = 0;

  যদি( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
    && pLoop->u.btree.p_সূচক!=0
    && pLoop->u.btree.p_সূচক->aSortOrder[iEq]
  ){
    testcase( iEq==0 );
    testcase( bRev );
    bRev = !bRev;
  }
  প্রতিজ্ঞা( pX->op==TK_IN );

  জন্য(i=0; i<iEq; i++){
    যদি( pLoop->aLTerm[i] && pLoop->aLTerm[i]->p_রাশিমালা==pX ){
      disableTerm(pLevel, pTerm);
      ফেরত;
    }
  }
  জন্য(i=iEq; i<pLoop->nLTerm; i++){
    প্রতিজ্ঞা( pLoop->aLTerm[i]!=0 );
    যদি( pLoop->aLTerm[i]->p_রাশিমালা==pX ) nEq++;
  }

  iTab = 0;
  যদি( !রাশিমালা_Use_XSelect(pX) || pX->x.pSelect->pEList->n_রাশিমালা==1 ){
    eType = sqlite3_Find_In_সূচক(pParse, pX, IN_সূচক_LOOP, 0, 0, &iTab);
  }অন্যথা{
    sqlite3 *db = pParse->db;
    রাশিমালা *pXMod = removeUnindexableInClauseTerms(pParse, iEq, pLoop, pX);
    যদি( nEq>1 ){
      /* If this IN(SELECT ...) expression drives more than one column of
      ** the index, disable the seek-scan optimization. The reasons for this
      ** are that (a) it is only possible to make this happen by populating
      ** the sqlite_stat1 table with inconsistent information, and (b) it
      ** would require sqlite3FindInIndex() to find an index that is not
      ** only unique for the columns in question, but also delivers them
      ** in sorted order (requires checking asc/desc, and rejecting cases
      ** where the indexed columns are not in the right order).  */
      pLoop->wsFlags &= ~WHERE_IN_SEEKSCAN;
    }
    যদি( !db->mallocFailed ){
      পূর্ণসংখ্যা nCol = pX->x.pSelect->pEList->n_রাশিমালা;
      aiMap = (পূর্ণসংখ্যা*)sqlite3DbMallocZero(db, আকার(পূর্ণসংখ্যা)*nCol);
      eType = sqlite3_Find_In_সূচক(pParse, pXMod, IN_সূচক_LOOP, 0, aiMap, &iTab);
    }
    sqlite3_রাশিমালা_Delete(db, pXMod);
  }

  যদি( eType==IN_সূচক_সূচক_DESC ){
    testcase( bRev );
    bRev = !bRev;
  }
  sqlite3VdbeAddOp2(v, bRev ? OP_Last : OP_Rewind, iTab, 0);
  VdbeCoverageIf(v, bRev);
  VdbeCoverageIf(v, !bRev);

  প্রতিজ্ঞা( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );
  pLoop->wsFlags |= WHERE_IN_ABLE;
  যদি( pLevel->u.in.nIn==0 ){
    pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
  }
  যদি( iEq>0 && (pLoop->wsFlags & WHERE_IN_SEEKSCAN)==0 ){
    pLoop->wsFlags |= WHERE_IN_EARLYOUT;
  }

  i = pLevel->u.in.nIn;
  pLevel->u.in.nIn += nEq;
  pLevel->u.in.aInLoop =
     sqlite3WhereRealloc(pTerm->pWC->pWInfo,
                         pLevel->u.in.aInLoop,
                         আকার(pLevel->u.in.aInLoop[0])*pLevel->u.in.nIn);
  pIn = pLevel->u.in.aInLoop;
  যদি( pIn ){
    পূর্ণসংখ্যা iMap = 0;               /* Index in aiMap[] */
    pIn += i;
    জন্য(i=iEq; i<pLoop->nLTerm; i++){
      যদি( pLoop->aLTerm[i]->p_রাশিমালা==pX ){
        পূর্ণসংখ্যা iOut = iTarget + i - iEq;
        যদি( eType==IN_সূচক_ROWID ){
          pIn->addr_In_শীর্ষ = sqlite3VdbeAddOp2(v, OP_Rowid, iTab, iOut);
        }অন্যথা{
          পূর্ণসংখ্যা iCol = aiMap ? aiMap[iMap++] : 0;
          pIn->addr_In_শীর্ষ = sqlite3VdbeAddOp3(v,OP_Column,iTab, iCol, iOut);
        }
        sqlite3VdbeAddOp1(v, OP_IsNull, iOut); VdbeCoverage(v);
        যদি( i==iEq ){
          pIn->iCur = iTab;
          pIn->eEndLoopOp = bRev ? OP_পূর্ব : OP_পরবর্তী;
          যদি( iEq>0 ){
            pIn->iBase = iTarget - i;
            pIn->nPrefix = i;
          }অন্যথা{
            pIn->nPrefix = 0;
          }
        }অন্যথা{
          pIn->eEndLoopOp = OP_Noop;
        }
        pIn++;
      }
    }
    testcase( iEq>0
              && (pLoop->wsFlags & WHERE_IN_SEEKSCAN)==0
              && (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 );
    যদি( iEq>0
     && (pLoop->wsFlags & (WHERE_IN_SEEKSCAN|WHERE_VIRTUALTABLE))==0
    ){
      sqlite3VdbeAddOp3(v, OP_SeekHit, pLevel->iIdxCur, 0, iEq);
    }
  }অন্যথা{
    pLevel->u.in.nIn = 0;
  }
  sqlite3DbFree(pParse->db, aiMap);
}
#endif


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
স্থির পূর্ণসংখ্যা codeEqualityTerm(
  Parse *pParse,      /* The parsing context */
  WhereTerm *pTerm,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  পূর্ণসংখ্যা iEq,            /* Index of the equality term within this level */
  পূর্ণসংখ্যা bRev,           /* True for reverse-order IN operations */
  পূর্ণসংখ্যা iTarget         /* Attempt to leave results in this register */
){
  রাশিমালা *pX = pTerm->p_রাশিমালা;
  পূর্ণসংখ্যা iReg;                  /* Register holding results */

  প্রতিজ্ঞা( pLevel->pWLoop->aLTerm[iEq]==pTerm );
  প্রতিজ্ঞা( iTarget>0 );
  যদি( pX->op==TK_EQ || pX->op==TK_IS ){
    iReg = sqlite3_রাশিমালা_Code_Target(pParse, pX->p_ডান, iTarget);
  }অন্যথা যদি( pX->op==TK_ISNULL ){
    iReg = iTarget;
    sqlite3VdbeAddOp2(pParse->pVdbe, OP_Null, 0, iReg);
#ifndef SQLITE_OMIT_SUBQUERY
  }অন্যথা{
    প্রতিজ্ঞা( pX->op==TK_IN );
    iReg = iTarget;
    codeINTerm(pParse, pTerm, pLevel, iEq, bRev, iTarget);
#endif
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
  যদি( (pLevel->pWLoop->wsFlags & WHERE_TRANSCONS)==0
   || (pTerm->eOperator & WO_EQUIV)==0
  ){
    disableTerm(pLevel, pTerm);
  }

  ফেরত iReg;
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
স্থির পূর্ণসংখ্যা codeAllEqualityTerms(
  Parse *pParse,        /* Parsing context */
  WhereLevel *pLevel,   /* Which nested loop of the FROM we are coding */
  পূর্ণসংখ্যা bRev,             /* Reverse the order of IN operators */
  পূর্ণসংখ্যা nExtraReg,        /* Number of extra registers to allocate */
  অক্ষর **pzAff          /* OUT: Set to point to affinity string */
){
  u16 nEq;                      /* The number of == or IN constraints to code */
  u16 nSkip;                    /* Number of left-most columns to skip */
  Vdbe *v = pParse->pVdbe;      /* The vm under construction */
  সূচক *pIdx;                  /* The index being used for this loop */
  WhereTerm *pTerm;             /* A single constraint term */
  WhereLoop *pLoop;             /* The WhereLoop object */
  পূর্ণসংখ্যা j;                        /* Loop counter */
  পূর্ণসংখ্যা regBase;                  /* Base register */
  পূর্ণসংখ্যা nReg;                     /* Number of registers to allocate */
  অক্ষর *zAff;                   /* Affinity string to return */

  /* This module is only called on query plans that use an index. */
  pLoop = pLevel->pWLoop;
  প্রতিজ্ঞা( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0 );
  nEq = pLoop->u.btree.nEq;
  nSkip = pLoop->nSkip;
  pIdx = pLoop->u.btree.p_সূচক;
  প্রতিজ্ঞা( pIdx!=0 );

  /* Figure out how many memory cells we will need then allocate them.
  */
  regBase = pParse->nMem + 1;
  nReg = nEq + nExtraReg;
  pParse->nMem += nReg;

  zAff = sqlite3DbStrDup(pParse->db,sqlite3_সূচক_Affinity_Str(pParse->db,pIdx));
  প্রতিজ্ঞা( zAff!=0 || pParse->db->mallocFailed );

  যদি( nSkip ){
    পূর্ণসংখ্যা iIdxCur = pLevel->iIdxCur;
    sqlite3VdbeAddOp3(v, OP_Null, 0, regBase, regBase+nSkip-1);
    sqlite3VdbeAddOp1(v, (bRev?OP_Last:OP_Rewind), iIdxCur);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    VdbeComment((v, "begin skip-scan on %s", pIdx->zName));
    j = sqlite3VdbeAddOp0(v, OP_Goto);
    প্রতিজ্ঞা( pLevel->addrSkip==0 );
    pLevel->addrSkip = sqlite3VdbeAddOp4Int(v, (bRev?OP_SeekLT:OP_SeekGT),
                            iIdxCur, 0, regBase, nSkip);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    sqlite3VdbeJumpHere(v, j);
    জন্য(j=0; j<nSkip; j++){
      sqlite3VdbeAddOp3(v, OP_Column, iIdxCur, j, regBase+j);
      testcase( pIdx->aiColumn[j]==XN_রাশিমালা );
      VdbeComment((v, "%s", explain_সূচক_Column_Name(pIdx, j)));
    }
  }

  /* Evaluate the equality constraints
  */
  প্রতিজ্ঞা( zAff==0 || (পূর্ণসংখ্যা)strlen(zAff)>=nEq );
  জন্য(j=nSkip; j<nEq; j++){
    পূর্ণসংখ্যা r1;
    pTerm = pLoop->aLTerm[j];
    প্রতিজ্ঞা( pTerm!=0 );
    /* The following testcase is true for indices with redundant columns.
    ** Ex: CREATE INDEX i1 ON t1(a,b,a); SELECT * FROM t1 WHERE a=0 AND b=0; */
    testcase( (pTerm->wtFlags & TERM_CODED)!=0 );
    testcase( pTerm->wtFlags & TERM_VIRTUAL );
    r1 = codeEqualityTerm(pParse, pTerm, pLevel, j, bRev, regBase+j);
    যদি( r1!=regBase+j ){
      sqlite3VdbeAddOp2(v, OP_Copy, r1, regBase+j);
    }
    যদি( pTerm->eOperator & WO_IN ){
      যদি( pTerm->p_রাশিমালা->flags & EP_xIsSelect ){
        /* No affinity ever needs to be (or should be) applied to a value
        ** from the RHS of an "? IN (SELECT ...)" expression. The
        ** sqlite3FindInIndex() routine has already ensured that the
        ** affinity of the comparison has been applied to the value.  */
        যদি( zAff ) zAff[j] = SQLITE_AFF_BLOB;
      }
    }অন্যথা যদি( (pTerm->eOperator & WO_ISNULL)==0 ){
      রাশিমালা *p_ডান = pTerm->p_রাশিমালা->p_ডান;
      যদি( (pTerm->wtFlags & TERM_IS)==0 && sqlite3_রাশিমালা_Can_Be_Null(p_ডান) ){
        sqlite3VdbeAddOp2(v, OP_IsNull, regBase+j, pLevel->addrBrk);
        VdbeCoverage(v);
      }
      যদি( pParse->nErr==0 ){
        প্রতিজ্ঞা( pParse->db->mallocFailed==0 );
        যদি( sqlite3CompareAffinity(p_ডান, zAff[j])==SQLITE_AFF_BLOB ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
        যদি( sqlite3_রাশিমালা_Needs_No_Affinity_Change(p_ডান, zAff[j]) ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
      }
    }
  }
  *pzAff = zAff;
  ফেরত regBase;
}

#ifndef SQLITE_LIKE_DOESNT_MATCH_BLOBS
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
স্থির শূন্য whereLikeOptimizationStringFixup(
  Vdbe *v,                /* prepared statement under construction */
  WhereLevel *pLevel,     /* The loop that contains the LIKE operator */
  WhereTerm *pTerm        /* The upper or lower bound just coded */
){
  যদি( pTerm->wtFlags & TERM_LIKEOPT ){
    VdbeOp *pOp;
    প্রতিজ্ঞা( pLevel->iLikeRepCntr>0 );
    pOp = sqlite3VdbeGetLastOp(v);
    প্রতিজ্ঞা( pOp!=0 );
    প্রতিজ্ঞা( pOp->opcode==OP_String8
            || pTerm->pWC->pWInfo->pParse->db->mallocFailed );
    pOp->p3 = (পূর্ণসংখ্যা)(pLevel->iLikeRepCntr>>1);  /* Register holding counter */
    pOp->p5 = (u8)(pLevel->iLikeRepCntr&1);    /* ASC or DESC */
  }
}
#অন্যথা
# define whereLikeOptimizationStringFixup(A,B,C)
#endif

#ifdef SQLITE_ENABLE_CURSOR_HINTS
/*
** Information is passed from codeCursorHint() down to individual nodes of
** the expression tree (by sqlite3WalkExpr()) using an instance of this
** structure.
*/
কাঠামো CCurHint {
  পূর্ণসংখ্যা iTabCur;    /* Cursor for the main table */
  পূর্ণসংখ্যা iIdxCur;    /* Cursor for the index, if pIdx!=0.  Unused otherwise */
  সূচক *pIdx;    /* The index used to access the table */
};

/*
** This function is called for every node of an expression that is a candidate
** for a cursor hint on an index cursor.  For TK_COLUMN nodes that reference
** the table CCurHint.iTabCur, verify that the same column can be
** accessed through the index.  If it cannot, then set pWalker->eCode to 1.
*/
স্থির পূর্ণসংখ্যা code_Cursor_Hint_Check_রাশিমালা(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  কাঠামো CCurHint *pHint = pWalker->u.pCCurHint;
  প্রতিজ্ঞা( pHint->pIdx!=0 );
  যদি( p_রাশিমালা->op==TK_COLUMN
   && p_রাশিমালা->iTable==pHint->iTabCur
   && sqlite3_Table_Column_To_সূচক(pHint->pIdx, p_রাশিমালা->iColumn)<0
  ){
    pWalker->eCode = 1;
  }
  ফেরত WRC_Continue;
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
স্থির পূর্ণসংখ্যা codeCursorHintIsOrFunction(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  যদি( p_রাশিমালা->op==TK_IS
   || p_রাশিমালা->op==TK_ISNULL || p_রাশিমালা->op==TK_ISNOT
   || p_রাশিমালা->op==TK_NOTNULL || p_রাশিমালা->op==TK_CASE
  ){
    pWalker->eCode = 1;
  }অন্যথা যদি( p_রাশিমালা->op==TK_FUNCTION ){
    পূর্ণসংখ্যা d1;
    অক্ষর d2[4];
    যদি( 0==sqlite3IsLikeFunction(pWalker->pParse->db, p_রাশিমালা, &d1, d2) ){
      pWalker->eCode = 1;
    }
  }

  ফেরত WRC_Continue;
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
স্থির পূর্ণসংখ্যা code_Cursor_Hint_Fix_রাশিমালা(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  পূর্ণসংখ্যা rc = WRC_Continue;
  পূর্ণসংখ্যা reg;
  কাঠামো CCurHint *pHint = pWalker->u.pCCurHint;
  যদি( p_রাশিমালা->op==TK_COLUMN ){
    যদি( p_রাশিমালা->iTable!=pHint->iTabCur ){
      reg = ++pWalker->pParse->nMem;   /* Register for column value */
      reg = sqlite3_রাশিমালা_Code_Target(pWalker->pParse, p_রাশিমালা, reg);
      p_রাশিমালা->op = TK_REGISTER;
      p_রাশিমালা->iTable = reg;
    }অন্যথা যদি( pHint->pIdx!=0 ){
      p_রাশিমালা->iTable = pHint->iIdxCur;
      p_রাশিমালা->iColumn = sqlite3_Table_Column_To_সূচক(pHint->pIdx, p_রাশিমালা->iColumn);
      প্রতিজ্ঞা( p_রাশিমালা->iColumn>=0 );
    }
  }অন্যথা যদি( p_রাশিমালা->pAggInfo ){
    rc = WRC_Prune;
    reg = ++pWalker->pParse->nMem;   /* Register for column value */
    reg = sqlite3_রাশিমালা_Code_Target(pWalker->pParse, p_রাশিমালা, reg);
    p_রাশিমালা->op = TK_REGISTER;
    p_রাশিমালা->iTable = reg;
  }অন্যথা যদি( p_রাশিমালা->op==TK_TRUEFALSE ){
    /* Do not walk disabled expressions.  tag-20230504-1 */
    ফেরত WRC_Prune;
  }
  ফেরত rc;
}

/*
** Insert an OP_CursorHint instruction if it is appropriate to do so.
*/
স্থির শূন্য codeCursorHint(
  SrcItem *pTabItem,  /* FROM clause item */
  WhereInfo *pWInfo,    /* The where clause */
  WhereLevel *pLevel,   /* Which loop to provide hints for */
  WhereTerm *pEndRange  /* Hint this end-of-scan boundary term if not NULL */
){
  Parse *pParse = pWInfo->pParse;
  sqlite3 *db = pParse->db;
  Vdbe *v = pParse->pVdbe;
  রাশিমালা *p_রাশিমালা = 0;
  WhereLoop *pLoop = pLevel->pWLoop;
  পূর্ণসংখ্যা iCur;
  WhereClause *pWC;
  WhereTerm *pTerm;
  পূর্ণসংখ্যা i, j;
  কাঠামো CCurHint sHint;
  Walker sWalker;

  যদি( OptimizationDisabled(db, SQLITE_CursorHints) ) ফেরত;
  iCur = pLevel->iTabCur;
  প্রতিজ্ঞা( iCur==pWInfo->pTabList->a[pLevel->iFrom].iCursor );
  sHint.iTabCur = iCur;
  sHint.iIdxCur = pLevel->iIdxCur;
  sHint.pIdx = pLoop->u.btree.p_সূচক;
  memset(&sWalker, 0, আকার(sWalker));
  sWalker.pParse = pParse;
  sWalker.u.pCCurHint = &sHint;
  pWC = &pWInfo->sWC;
  জন্য(i=0; i<pWC->nBase; i++){
    pTerm = &pWC->a[i];
    যদি( pTerm->wtFlags & (TERM_VIRTUAL|TERM_CODED) ) চালু;
    যদি( pTerm->prereqAll & pLevel->notReady ) চালু;

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
    যদি( pTabItem->fg.jointype & JT_বাম ){
      রাশিমালা *p_রাশিমালা = pTerm->p_রাশিমালা;
      যদি( !রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON)
       || p_রাশিমালা->w.iJoin!=pTabItem->iCursor
      ){
        sWalker.eCode = 0;
        sWalker.x_রাশিমালা_Callback = codeCursorHintIsOrFunction;
        sqlite3_Walk_রাশিমালা(&sWalker, pTerm->p_রাশিমালা);
        যদি( sWalker.eCode ) চালু;
      }
    }অন্যথা{
      যদি( রাশিমালা_Has_Property(pTerm->p_রাশিমালা, EP_OuterON) ) চালু;
    }

    /* All terms in pWLoop->aLTerm[] except pEndRange are used to initialize
    ** the cursor.  These terms are not needed as hints for a pure range
    ** scan (that has no == terms) so omit them. */
    যদি( pLoop->u.btree.nEq==0 && pTerm!=pEndRange ){
      জন্য(j=0; j<pLoop->nLTerm && pLoop->aLTerm[j]!=pTerm; j++){}
      যদি( j<pLoop->nLTerm ) চালু;
    }

    /* No subqueries or non-deterministic functions allowed */
    যদি( sqlite3_রাশিমালা_Contains_Subquery(pTerm->p_রাশিমালা) ) চালু;

    /* For an index scan, make sure referenced columns are actually in
    ** the index. */
    যদি( sHint.pIdx!=0 ){
      sWalker.eCode = 0;
      sWalker.x_রাশিমালা_Callback = code_Cursor_Hint_Check_রাশিমালা;
      sqlite3_Walk_রাশিমালা(&sWalker, pTerm->p_রাশিমালা);
      যদি( sWalker.eCode ) চালু;
    }

    /* If we survive all prior tests, that means this term is worth hinting */
    p_রাশিমালা = sqlite3_রাশিমালা_And(pParse, p_রাশিমালা, sqlite3_রাশিমালা_Dup(db, pTerm->p_রাশিমালা, 0));
  }
  যদি( p_রাশিমালা!=0 ){
    sWalker.x_রাশিমালা_Callback = code_Cursor_Hint_Fix_রাশিমালা;
    যদি( pParse->nErr==0 ) sqlite3_Walk_রাশিমালা(&sWalker, p_রাশিমালা);
    sqlite3VdbeAddOp4(v, OP_CursorHint,
                      (sHint.pIdx ? sHint.iIdxCur : sHint.iTabCur), 0, 0,
                      (ধ্রুবক অক্ষর*)p_রাশিমালা, P4_রাশিমালা);
  }
}
#অন্যথা
# define codeCursorHint(A,B,C,D)  /* No-op */
#endif /* SQLITE_ENABLE_CURSOR_HINTS */

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
স্থির শূন্য codeDeferredSeek(
  WhereInfo *pWInfo,              /* Where clause context */
  সূচক *pIdx,                    /* Index scan is using */
  পূর্ণসংখ্যা iCur,                       /* Cursor for IPK b-tree */
  পূর্ণসংখ্যা iIdxCur                     /* Index cursor */
){
  Parse *pParse = pWInfo->pParse; /* Parse context */
  Vdbe *v = pParse->pVdbe;        /* Vdbe to generate code within */

  প্রতিজ্ঞা( iIdxCur>0 );
  প্রতিজ্ঞা( pIdx->aiColumn[pIdx->nColumn-1]==-1 );

  pWInfo->bDeferredSeek = 1;
  sqlite3VdbeAddOp3(v, OP_DeferredSeek, iIdxCur, 0, iCur);
  যদি( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_ডান_JOIN))
   && DbMaskAllZero(sqlite3ParseToplevel(pParse)->writeMask)
  ){
    পূর্ণসংখ্যা i;
    Table *pTab = pIdx->pTable;
    u32 *ai = (u32*)sqlite3DbMallocZero(pParse->db, আকার(u32)*(pTab->nCol+1));
    যদি( ai ){
      ai[0] = pTab->nCol;
      জন্য(i=0; i<pIdx->nColumn-1; i++){
        পূর্ণসংখ্যা x1, x2;
        প্রতিজ্ঞা( pIdx->aiColumn[i]<pTab->nCol );
        x1 = pIdx->aiColumn[i];
        x2 = sqlite3TableColumnToStorage(pTab, x1);
        testcase( x1!=x2 );
        যদি( x1>=0 ) ai[x2+1] = i+1;
      }
      sqlite3VdbeChangeP4(v, -1, (অক্ষর*)ai, P4_INTARRAY);
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
স্থির শূন্য code_রাশিমালা_Or_Vector(Parse *pParse, রাশিমালা *p, পূর্ণসংখ্যা iReg, পূর্ণসংখ্যা nReg){
  প্রতিজ্ঞা( nReg>0 );
  যদি( p && sqlite3_রাশিমালা_Is_Vector(p) ){
#ifndef SQLITE_OMIT_SUBQUERY
    যদি( রাশিমালা_Use_XSelect(p) ){
      Vdbe *v = pParse->pVdbe;
      পূর্ণসংখ্যা iSelect;
      প্রতিজ্ঞা( p->op==TK_SELECT );
      iSelect = sqlite3CodeSubselect(pParse, p);
      sqlite3VdbeAddOp3(v, OP_Copy, iSelect, iReg, nReg-1);
    }অন্যথা
#endif
    {
      পূর্ণসংখ্যা i;
      ধ্রুবক রাশিমালা_List *pList;
      প্রতিজ্ঞা( রাশিমালা_Use_XList(p) );
      pList = p->x.pList;
      প্রতিজ্ঞা( nReg<=pList->n_রাশিমালা );
      জন্য(i=0; i<nReg; i++){
        sqlite3_রাশিমালা_Code(pParse, pList->a[i].p_রাশিমালা, iReg+i);
      }
    }
  }অন্যথা{
    প্রতিজ্ঞা( nReg==1 || pParse->nErr );
    sqlite3_রাশিমালা_Code(pParse, p, iReg);
  }
}

/*
** The pTruth expression is always true because it is the WHERE clause
** a partial index that is driving a query loop.  Look through all of the
** WHERE clause terms on the query, and if any of those terms must be
** true because pTruth is true, then mark those WHERE clause terms as
** coded.
*/
স্থির শূন্য where_Apply_Partial_সূচক_Constraints(
  রাশিমালা *pTruth,
  পূর্ণসংখ্যা iTabCur,
  WhereClause *pWC
){
  পূর্ণসংখ্যা i;
  WhereTerm *pTerm;
  যখন( pTruth->op==TK_AND ){
    where_Apply_Partial_সূচক_Constraints(pTruth->p_বাম, iTabCur, pWC);
    pTruth = pTruth->p_ডান;
  }
  জন্য(i=0, pTerm=pWC->a; i<pWC->nTerm; i++, pTerm++){
    রাশিমালা *p_রাশিমালা;
    যদি( pTerm->wtFlags & TERM_CODED ) চালু;
    p_রাশিমালা = pTerm->p_রাশিমালা;
    যদি( sqlite3_রাশিমালা_Compare(0, p_রাশিমালা, pTruth, iTabCur)==0 ){
      pTerm->wtFlags |= TERM_CODED;
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
স্থির SQLITE_NOINLINE শূন্য filterPullDown(
  Parse *pParse,       /* Parsing context */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  পূর্ণসংখ্যা iLevel,          /* Which level of pWInfo->a[] should be coded */
  পূর্ণসংখ্যা addrNxt,         /* Jump here to bypass inner loops */
  Bitmask notReady     /* Loops that are not ready */
){
  পূর্ণসংখ্যা saved_addrBrk;
  যখন( ++iLevel < pWInfo->nLevel ){
    WhereLevel *pLevel = &pWInfo->a[iLevel];
    WhereLoop *pLoop = pLevel->pWLoop;
    যদি( pLevel->regFilter==0 ) চালু;
    যদি( pLevel->pWLoop->nSkip ) চালু;
    /*         ,--- Because sqlite3ConstructBloomFilter() has will not have set
    **  vvvvv--'    pLevel->regFilter if this were true. */
    যদি( NEVER(pLoop->prereq & notReady) ) চালু;
    saved_addrBrk = pLevel->addrBrk;
    pLevel->addrBrk = addrNxt;
    যদি( pLoop->wsFlags & WHERE_IPK ){
      WhereTerm *pTerm = pLoop->aLTerm[0];
      পূর্ণসংখ্যা regRowid;
      প্রতিজ্ঞা( pTerm!=0 );
      প্রতিজ্ঞা( pTerm->p_রাশিমালা!=0 );
      testcase( pTerm->wtFlags & TERM_VIRTUAL );
      regRowid = sqlite3_Get_অস্থায়ী_Reg(pParse);
      regRowid = codeEqualityTerm(pParse, pTerm, pLevel, 0, 0, regRowid);
      sqlite3VdbeAddOp2(pParse->pVdbe, OP_MustBeInt, regRowid, addrNxt);
      VdbeCoverage(pParse->pVdbe);
      প্রতিজ্ঞা( sqlite3WhereLoopBloomable(pLoop) );
      sqlite3VdbeAddOp4Int(pParse->pVdbe, OP_Filter, pLevel->regFilter,
                           addrNxt, regRowid, 1);
      VdbeCoverage(pParse->pVdbe);
    }অন্যথা{
      u16 nEq = pLoop->u.btree.nEq;
      পূর্ণসংখ্যা r1;
      অক্ষর *zStartAff;

      প্রতিজ্ঞা( pLoop->wsFlags & WHERE_INDEXED );
      প্রতিজ্ঞা( (pLoop->wsFlags & WHERE_COLUMN_IN)==0 );
      r1 = codeAllEqualityTerms(pParse,pLevel,0,0,&zStartAff);
      codeApplyAffinity(pParse, r1, nEq, zStartAff);
      sqlite3DbFree(pParse->db, zStartAff);
      প্রতিজ্ঞা( sqlite3WhereLoopBloomable(pLoop) );
      sqlite3VdbeAddOp4Int(pParse->pVdbe, OP_Filter, pLevel->regFilter,
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
স্থির পূর্ণসংখ্যা whereLoopIsOneRow(WhereLoop *pLoop){
  যদি( pLoop->u.btree.p_সূচক->onError
   && pLoop->nSkip==0
   && pLoop->u.btree.nEq==pLoop->u.btree.p_সূচক->n_চাবি_Col
  ){
    পূর্ণসংখ্যা ii;
    জন্য(ii=0; ii<pLoop->u.btree.nEq; ii++){
      যদি( pLoop->aLTerm[ii]->eOperator & (WO_IS|WO_ISNULL) ){
        ফেরত 0;
      }
    }
    ফেরত 1;
  }
  ফেরত 0;
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
স্থির পূর্ণসংখ্যা where_রাশিমালা_Is_Ready(
  WhereInfo *pWInfo, 
  WhereLevel *pLevel,
  SrcItem *pTabItem, 
  রাশিমালা *pE
){
  u8 jtype = pTabItem->fg.jointype;
  যদি( jtype & (JT_বাম|JT_LTORJ|JT_ডান) ){
    যদি( !রাশিমালা_Has_Property(pE,EP_OuterON|EP_InnerON) ){
      /* Defer processing WHERE clause constraints until after outer
      ** join processing.  tag-20220513a */
      ফেরত 0;
    }অন্যথা যদি( (jtype & JT_বাম) && !রাশিমালা_Has_Property(pE,EP_OuterON) ){
      ফেরত 0;
    }অন্যথা{
      Bitmask m = sqlite3WhereGetMask(&pWInfo->sMaskSet, pE->w.iJoin);
      যদি( m & pLevel->notReady ){
        /* An ON clause that is not ripe */
        ফেরত 0;
      }
    }
  }
  ফেরত 1;
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
স্থির পূর্ণসংখ্যা findNoMatchContinuation(
  WhereInfo *pWInfo,         /* WHERE clause */
  পূর্ণসংখ্যা iLevel                 /* Which loop we are talking about */
){
  WhereLoop *pLoop;          /* Description of this loop */
  WhereLevel *pParent;       /* Controlling parent loop */
  SrcList *pTabList;         /* Complete FROM clause */
  Bitmask m_চাবি;              /* Mask of dependent tables */
  পূর্ণসংখ্যা iParent;               /* Inner-most loop affecting the key to iLevel */
  পূর্ণসংখ্যা i;                     /* Loop counter */

  যদি( iLevel<2 ){
    /* If there are less than two outer loops, this optimization is no help */
    ফেরত 0;
  }

  /* The OP_SeekGE and OP_SeekLE opcodes only work right for this
  ** optimization on a pure equality constraint, so skip the optimization
  ** for anything else.
  */
  pLoop = pWInfo->a[iLevel].pWLoop;
  যদি( (pLoop->wsFlags & WHERE_CONSTRAINT)!=WHERE_COLUMN_EQ ) ফেরত 0;
  যদি( pLoop->wsFlags & WHERE_SKIPSCAN ) ফেরত 0;
  যদি( pLoop->wsFlags & WHERE_BIGNULL_SORT ) ফেরত 0;
  প্রতিজ্ঞা( (pLoop->wsFlags & (WHERE_VIRTUALTABLE|WHERE_MULTI_OR))==0 );
  প্রতিজ্ঞা( pLoop->u.btree.nEq>0 );

  /* Find a mask of all outer tables that influence the key.  At the same
  ** time, check to see if any keys values are computed from subqueries and
  ** fail out (return 0) if there are any. */
  m_চাবি = 0;
  জন্য(i=0; i<pLoop->nLTerm; i++){
    যদি( sqlite3_রাশিমালা_Contains_Subquery(pLoop->aLTerm[i]->p_রাশিমালা) ){
      ফেরত 0;
    }
    m_চাবি |= pLoop->aLTerm[i]->prereq_ডান;
  }

  /* Look back through outer loops.  Find the inner-most outer loop
  ** that influences the key. Make iParent be the index of that inner-most
  ** outer loop.
  */
  pTabList = pWInfo->pTabList;
  iParent = iLevel-1;
  যখন( iParent>0 && (pWInfo->a[iParent].pWLoop->maskSelf & m_চাবি)==0 ){
    iParent--;
  }
  প্রতিজ্ঞা( iParent>=0 && iParent<=iLevel );

  /* Everything needs to be an INNER join.  Fail if there are any
  ** OUTER joins in between iParent and iLevel. */
  জন্য(i=iParent; i<=iLevel; i++){
    যদি( pTabList->a[pWInfo->a[i].iFrom].fg.jointype & JT_OUTER ) ফেরত 0;
  }

  /* If we reach this point, that means the conditions of the optimization
  ** have been met.  Even so, disallow the optimization if it is disabled.
  */
  যদি( OptimizationDisabled(pWInfo->pParse->db, SQLITE_JoinMiss) ){
    ফেরত 0;
  }

  /* If the inner-most outer loop that influences the key for iLevel
  ** is controlled by an automatic index and if the automatic index is
  ** the only outer loop that influences the key, then arrange to delete
  ** the automatic index entry if there is no match on the key.
  */
  pParent = &pWInfo->a[iParent];
  যদি( (pParent->pWLoop->wsFlags & WHERE_AUTO_সূচক)!=0
   && (m_চাবি & (m_চাবি-1))==0
  ){
    জন্য(i=0; i<pLoop->nLTerm; i++){
      রাশিমালা *pX = pLoop->aLTerm[i]->p_রাশিমালা;
      যদি( !sqlite3_রাশিমালা_Is_Table_Constant(pX->p_ডান,pParent->iTabCur,0) ) বিরতি;
    }
    যদি( i==pLoop->nLTerm ){    
      যদি( pParent->addrDeleteCont==0 ){
        pParent->addrDeleteCont = sqlite3VdbeMakeLabel(pWInfo->pParse);
      }
      ফেরত pParent->addrDeleteCont;
    }
  }

  testcase( iParent<iLevel-1 );
  ফেরত pParent->addrCont;
}

/*
** Generate code for the start of the iLevel-th loop in the WHERE clause
** implementation described by pWInfo.
*/
Bitmask sqlite3WhereCodeOneLoopStart(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  পূর্ণসংখ্যা iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
){
  পূর্ণসংখ্যা j, k;            /* Loop counters */
  পূর্ণসংখ্যা iCur;            /* The VDBE cursor for the table */
  পূর্ণসংখ্যা addrNxt;         /* Where to jump to continue with the next IN case */
  পূর্ণসংখ্যা bRev;            /* True if we need to scan in reverse order */
  WhereLoop *pLoop;    /* The WhereLoop object being coded */
  WhereClause *pWC;    /* Decomposition of the entire WHERE clause */
  WhereTerm *pTerm;         /* A WHERE clause term */
  sqlite3 *db;              /* Database connection */
  SrcItem *pTabItem;        /* FROM clause term being coded */
  পূর্ণসংখ্যা addrBrk;              /* Jump here to break out of the loop */
  পূর্ণসংখ্যা addrCont;             /* Jump here to continue with next cycle */
  পূর্ণসংখ্যা addrMiss;             /* addrCont enhanced for no-matching rows */
  পূর্ণসংখ্যা iRowidReg = 0;        /* Rowid is stored in this register, if not zero */
  পূর্ণসংখ্যা iReleaseReg = 0;      /* Temp register to free before returning */
  সূচক *pIdx = 0;          /* Index used by loop (if any) */
  পূর্ণসংখ্যা iLoop;                /* Iteration of constraint generator loop */

  pWC = &pWInfo->sWC;
  db = pParse->db;
  pLoop = pLevel->pWLoop;
  pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
  iCur = pTabItem->iCursor;
  pLevel->notReady = notReady & ~sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur);
  bRev = (pWInfo->revMask>>iLevel)&1;
  VdbeModuleComment((v, "Begin WHERE-loop%d: %s",
                     iLevel, pTabItem->pSTab->zName));
#যদি WHERETRACE_ENABLED /* 0x4001 */
  যদি( sqlite3WhereTrace & 0x1 ){
    sqlite3DebugPrintf("Coding level %d of %d:  notReady=%llx  iFrom=%d\n",
       iLevel, pWInfo->nLevel, (u64)notReady, pLevel->iFrom);
    যদি( sqlite3WhereTrace & 0x1000 ){
      sqlite3WhereLoopPrint(pLoop, pWC);
    }
  }
  যদি( (sqlite3WhereTrace & 0x4001)==0x4001 ){
    যদি( iLevel==0 ){
      sqlite3DebugPrintf("WHERE clause being coded:\n");
      sqlite3_বৃক্ষ_View_রাশিমালা(0, pWInfo->pWhere, 0);
    }
    sqlite3DebugPrintf("All WHERE-clause terms before coding:\n");
    sqlite3WhereClausePrint(pWC);
  }
#endif

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
  addrMiss = findNoMatchContinuation(pWInfo, iLevel);

  /* If this is the right table of a LEFT OUTER JOIN, allocate and
  ** initialize a memory cell that records if this table matches any
  ** row of the left table of the join.
  */
  প্রতিজ্ঞা( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_ডান_JOIN))
       || pLevel->iFrom>0 || (pTabItem[0].fg.jointype & JT_বাম)==0
  );
  যদি( pLevel->iFrom>0 && (pTabItem[0].fg.jointype & JT_বাম)!=0 ){
    pLevel->i_বাম_Join = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, pLevel->i_বাম_Join);
    VdbeComment((v, "init LEFT JOIN match flag"));
    যদি( pTabItem->fg.viaCoroutine ){
      sqlite3VdbeAddOp2(v, OP_NullRow, pTabItem->iCursor, 1);
    }
  }

  /* Special case of a FROM clause subquery implemented as a co-routine */
  যদি( pTabItem->fg.viaCoroutine ){
    পূর্ণসংখ্যা regYield;
    Subquery *pSubq;
    প্রতিজ্ঞা( pTabItem->fg.isSubquery && pTabItem->u4.pSubq!=0 );
    pSubq = pTabItem->u4.pSubq;
    regYield = pSubq->regReturn;
    sqlite3VdbeAddOp3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    pLevel->p2 =  sqlite3VdbeAddOp2(v, OP_Yield, regYield, addrBrk);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", pTabItem->pSTab->zName));
    pLevel->op = OP_Goto;
  }অন্যথা

#ifndef SQLITE_OMIT_VIRTUALTABLE
  যদি(  (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
    /* Case 1:  The table is a virtual-table.  Use the VFilter and VNext
    **          to access the data.
    */
    পূর্ণসংখ্যা iReg;   /* P3 Value for OP_VFilter */
    পূর্ণসংখ্যা addrNotFound;
    পূর্ণসংখ্যা nConstraint = pLoop->nLTerm;

    iReg = sqlite3_Get_অস্থায়ী_Range(pParse, nConstraint+2);
    addrNotFound = pLevel->addrBrk;
    জন্য(j=0; j<nConstraint; j++){
      পূর্ণসংখ্যা iTarget = iReg+j+2;
      pTerm = pLoop->aLTerm[j];
      যদি( NEVER(pTerm==0) ) চালু;
      যদি( pTerm->eOperator & WO_IN ){
        যদি( SMASKBIT32(j) & pLoop->u.vtab.mHandleIn ){
          পূর্ণসংখ্যা iTab = pParse->nTab++;
          পূর্ণসংখ্যা iCache = ++pParse->nMem;
          sqlite3CodeRhsOfIN(pParse, pTerm->p_রাশিমালা, iTab, 0);
          sqlite3VdbeAddOp3(v, OP_VInitIn, iTab, iTarget, iCache);
        }অন্যথা{
          codeEqualityTerm(pParse, pTerm, pLevel, j, bRev, iTarget);
          addrNotFound = pLevel->addrNxt;
        }
      }অন্যথা{
        রাশিমালা *p_ডান = pTerm->p_রাশিমালা->p_ডান;
        code_রাশিমালা_Or_Vector(pParse, p_ডান, iTarget, 1);
        যদি( pTerm->eMatchOp==SQLITE_সূচক_CONSTRAINT_OFFSET
         && pLoop->u.vtab.bOmitOffset
        ){
          প্রতিজ্ঞা( pTerm->eOperator==WO_AUX );
          প্রতিজ্ঞা( pWInfo->pSelect!=0 );
          প্রতিজ্ঞা( pWInfo->pSelect->iOffset>0 );
          sqlite3VdbeAddOp2(v, OP_Integer, 0, pWInfo->pSelect->iOffset);
          VdbeComment((v,"Zero OFFSET counter"));
        }
      }
    }
    sqlite3VdbeAddOp2(v, OP_Integer, pLoop->u.vtab.idx_অঙ্ক, iReg);
    sqlite3VdbeAddOp2(v, OP_Integer, nConstraint, iReg+1);
    /* The instruction immediately prior to OP_VFilter must be an OP_Integer
    ** that sets the "argc" value for xVFilter.  This is necessary for
    ** resolveP2() to work correctly.  See tag-20250207a. */
    sqlite3VdbeAddOp4(v, OP_VFilter, iCur, addrNotFound, iReg,
                      pLoop->u.vtab.idxStr,
                      pLoop->u.vtab.needFree ? P4_DYNAMIC : P4_STATIC);
    VdbeCoverage(v);
    pLoop->u.vtab.needFree = 0;
    /* An OOM inside of AddOp4(OP_VFilter) instruction above might have freed
    ** the u.vtab.idxStr.  NULL it out to prevent a use-after-free */
    যদি( db->mallocFailed ) pLoop->u.vtab.idxStr = 0;
    pLevel->p1 = iCur;
    pLevel->op = pWInfo->eOnePass ? OP_Noop : OP_VNext;
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);
    প্রতিজ্ঞা( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );

    জন্য(j=0; j<nConstraint; j++){
      pTerm = pLoop->aLTerm[j];
      যদি( j<16 && (pLoop->u.vtab.omitMask>>j)&1 ){
        disableTerm(pLevel, pTerm);
        চালু;
      }
      যদি( (pTerm->eOperator & WO_IN)!=0
       && (SMASKBIT32(j) & pLoop->u.vtab.mHandleIn)==0
       && !db->mallocFailed
      ){
        রাশিমালা *pCompare;  /* The comparison operator */
        রাশিমালা *p_ডান;    /* RHS of the comparison */
        VdbeOp *pOp;     /* Opcode to access the value of the IN constraint */
        পূর্ণসংখ্যা iIn;         /* IN loop corresponding to the j-th constraint */

        /* Reload the constraint value into reg[iReg+j+2].  The same value
        ** was loaded into the same register prior to the OP_VFilter, but
        ** the xFilter implementation might have changed the datatype or
        ** encoding of the value in the register, so it *must* be reloaded.
        */
        জন্য(iIn=0; ALWAYS(iIn<pLevel->u.in.nIn); iIn++){
          pOp = sqlite3VdbeGetOp(v, pLevel->u.in.aInLoop[iIn].addr_In_শীর্ষ);
          যদি( (pOp->opcode==OP_Column && pOp->p3==iReg+j+2)
           || (pOp->opcode==OP_Rowid && pOp->p2==iReg+j+2)
          ){
            testcase( pOp->opcode==OP_Rowid );
            sqlite3VdbeAddOp3(v, pOp->opcode, pOp->p1, pOp->p2, pOp->p3);
            বিরতি;
          }
        }

        /* Generate code that will continue to the next row if
        ** the IN constraint is not satisfied
        */
        pCompare = sqlite3PExpr(pParse, TK_EQ, 0, 0);
        যদি( !db->mallocFailed ){
          পূর্ণসংখ্যা iFld = pTerm->u.x.iField;
          রাশিমালা *p_বাম = pTerm->p_রাশিমালা->p_বাম;
          প্রতিজ্ঞা( p_বাম!=0 );
          যদি( iFld>0 ){
            প্রতিজ্ঞা( p_বাম->op==TK_VECTOR );
            প্রতিজ্ঞা( রাশিমালা_Use_XList(p_বাম) );
            প্রতিজ্ঞা( iFld<=p_বাম->x.pList->n_রাশিমালা );
            pCompare->p_বাম = p_বাম->x.pList->a[iFld-1].p_রাশিমালা;
          }অন্যথা{
            pCompare->p_বাম = p_বাম;
          }
          pCompare->p_ডান = p_ডান = sqlite3_রাশিমালা(db, TK_REGISTER, 0);
          যদি( p_ডান ){
            p_ডান->iTable = iReg+j+2;
            sqlite3_রাশিমালা_If_False(
                pParse, pCompare, pLevel->addrCont, SQLITE_JUMPIFNULL
            );
          }
          pCompare->p_বাম = 0;
        }
        sqlite3_রাশিমালা_Delete(db, pCompare);
      }
    }

    /* These registers need to be preserved in case there is an IN operator
    ** loop.  So we could deallocate the registers here (and potentially
    ** reuse them later) if (pLoop->wsFlags & WHERE_IN_ABLE)==0.  But it seems
    ** simpler and safer to simply not reuse the registers.
    **
    **    sqlite3ReleaseTempRange(pParse, iReg, nConstraint+2);
    */
  }অন্যথা
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  যদি( (pLoop->wsFlags & WHERE_IPK)!=0
   && (pLoop->wsFlags & (WHERE_COLUMN_IN|WHERE_COLUMN_EQ))!=0
  ){
    /* Case 2:  We can directly reference a single row using an
    **          equality comparison against the ROWID field.  Or
    **          we reference multiple rows using a "rowid IN (...)"
    **          construct.
    */
    প্রতিজ্ঞা( pLoop->u.btree.nEq==1 );
    pTerm = pLoop->aLTerm[0];
    প্রতিজ্ঞা( pTerm!=0 );
    প্রতিজ্ঞা( pTerm->p_রাশিমালা!=0 );
    testcase( pTerm->wtFlags & TERM_VIRTUAL );
    iReleaseReg = ++pParse->nMem;
    iRowidReg = codeEqualityTerm(pParse, pTerm, pLevel, 0, bRev, iReleaseReg);
    যদি( iRowidReg!=iReleaseReg ) sqlite3_Release_অস্থায়ী_Reg(pParse, iReleaseReg);
    addrNxt = pLevel->addrNxt;
    যদি( addrMiss==0 ) addrMiss = addrNxt;
    যদি( pLevel->regFilter ){
      sqlite3VdbeAddOp2(v, OP_MustBeInt, iRowidReg, addrMiss);
      VdbeCoverage(v);
      sqlite3VdbeAddOp4Int(v, OP_Filter, pLevel->regFilter, addrMiss,
                           iRowidReg, 1);
      VdbeCoverage(v);
      filterPullDown(pParse, pWInfo, iLevel, addrNxt, notReady);
    }
    sqlite3VdbeAddOp3(v, OP_SeekRowid, iCur, addrMiss, iRowidReg);
    VdbeCoverage(v);
    pLevel->op = OP_Noop;
  }অন্যথা যদি( (pLoop->wsFlags & WHERE_IPK)!=0
         && (pLoop->wsFlags & WHERE_COLUMN_RANGE)!=0
  ){
    /* Case 3:  We have an inequality comparison against the ROWID field.
    */
    পূর্ণসংখ্যা testOp = OP_Noop;
    পূর্ণসংখ্যা start;
    পূর্ণসংখ্যা mem_End_মান = 0;
    WhereTerm *pStart, *pEnd;

    j = 0;
    pStart = pEnd = 0;
    যদি( pLoop->wsFlags & WHERE_BTM_LIMIT ) pStart = pLoop->aLTerm[j++];
    যদি( pLoop->wsFlags & WHERE_শীর্ষ_LIMIT ) pEnd = pLoop->aLTerm[j++];
    প্রতিজ্ঞা( pStart!=0 || pEnd!=0 );
    যদি( bRev ){
      pTerm = pStart;
      pStart = pEnd;
      pEnd = pTerm;
    }
    codeCursorHint(pTabItem, pWInfo, pLevel, pEnd);
    যদি( pStart ){
      রাশিমালা *pX;             /* The expression that defines the start bound */
      পূর্ণসংখ্যা r1, r_অস্থায়ী;        /* Registers for holding the start boundary */
      পূর্ণসংখ্যা op;               /* Cursor seek operation */

      /* The following constant maps TK_xx codes into corresponding
      ** seek opcodes.  It depends on a particular ordering of TK_xx
      */
      ধ্রুবক u8 aMoveOp[] = {
           /* TK_GT */  OP_SeekGT,
           /* TK_LE */  OP_SeekLE,
           /* TK_LT */  OP_SeekLT,
           /* TK_GE */  OP_SeekGE
      };
      প্রতিজ্ঞা( TK_LE==TK_GT+1 );      /* Make sure the ordering.. */
      প্রতিজ্ঞা( TK_LT==TK_GT+2 );      /*  ... of the TK_xx values... */
      প্রতিজ্ঞা( TK_GE==TK_GT+3 );      /*  ... is correct. */

      প্রতিজ্ঞা( (pStart->wtFlags & TERM_VNULL)==0 );
      testcase( pStart->wtFlags & TERM_VIRTUAL );
      pX = pStart->p_রাশিমালা;
      প্রতিজ্ঞা( pX!=0 );
      testcase( pStart->বাম_Cursor!=iCur ); /* transitive constraints */
      যদি( sqlite3_রাশিমালা_Is_Vector(pX->p_ডান) ){
        r1 = r_অস্থায়ী = sqlite3_Get_অস্থায়ী_Reg(pParse);
        code_রাশিমালা_Or_Vector(pParse, pX->p_ডান, r1, 1);
        testcase( pX->op==TK_GT );
        testcase( pX->op==TK_GE );
        testcase( pX->op==TK_LT );
        testcase( pX->op==TK_LE );
        op = aMoveOp[((pX->op - TK_GT - 1) & 0x3) | 0x1];
        প্রতিজ্ঞা( pX->op!=TK_GT || op==OP_SeekGE );
        প্রতিজ্ঞা( pX->op!=TK_GE || op==OP_SeekGE );
        প্রতিজ্ঞা( pX->op!=TK_LT || op==OP_SeekLE );
        প্রতিজ্ঞা( pX->op!=TK_LE || op==OP_SeekLE );
      }অন্যথা{
        r1 = sqlite3_রাশিমালা_Code_অস্থায়ী(pParse, pX->p_ডান, &r_অস্থায়ী);
        disableTerm(pLevel, pStart);
        op = aMoveOp[(pX->op - TK_GT)];
      }
      sqlite3VdbeAddOp3(v, op, iCur, addrBrk, r1);
      VdbeComment((v, "pk"));
      VdbeCoverageIf(v, pX->op==TK_GT);
      VdbeCoverageIf(v, pX->op==TK_LE);
      VdbeCoverageIf(v, pX->op==TK_LT);
      VdbeCoverageIf(v, pX->op==TK_GE);
      sqlite3_Release_অস্থায়ী_Reg(pParse, r_অস্থায়ী);
    }অন্যথা{
      sqlite3VdbeAddOp2(v, bRev ? OP_Last : OP_Rewind, iCur, pLevel->addrHalt);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
    }
    যদি( pEnd ){
      রাশিমালা *pX;
      pX = pEnd->p_রাশিমালা;
      প্রতিজ্ঞা( pX!=0 );
      প্রতিজ্ঞা( (pEnd->wtFlags & TERM_VNULL)==0 );
      testcase( pEnd->বাম_Cursor!=iCur ); /* Transitive constraints */
      testcase( pEnd->wtFlags & TERM_VIRTUAL );
      mem_End_মান = ++pParse->nMem;
      code_রাশিমালা_Or_Vector(pParse, pX->p_ডান, mem_End_মান, 1);
      যদি( 0==sqlite3_রাশিমালা_Is_Vector(pX->p_ডান)
       && (pX->op==TK_LT || pX->op==TK_GT)
      ){
        testOp = bRev ? OP_Le : OP_Ge;
      }অন্যথা{
        testOp = bRev ? OP_Lt : OP_Gt;
      }
      যদি( 0==sqlite3_রাশিমালা_Is_Vector(pX->p_ডান) ){
        disableTerm(pLevel, pEnd);
      }
    }
    start = sqlite3VdbeCurrentAddr(v);
    pLevel->op = bRev ? OP_পূর্ব : OP_পরবর্তী;
    pLevel->p1 = iCur;
    pLevel->p2 = start;
    প্রতিজ্ঞা( pLevel->p5==0 );
    যদি( testOp!=OP_Noop ){
      iRowidReg = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, iRowidReg);
      sqlite3VdbeAddOp3(v, testOp, mem_End_মান, addrBrk, iRowidReg);
      VdbeCoverageIf(v, testOp==OP_Le);
      VdbeCoverageIf(v, testOp==OP_Lt);
      VdbeCoverageIf(v, testOp==OP_Ge);
      VdbeCoverageIf(v, testOp==OP_Gt);
      sqlite3VdbeChangeP5(v, SQLITE_AFF_NUMERIC | SQLITE_JUMPIFNULL);
    }
  }অন্যথা যদি( pLoop->wsFlags & WHERE_INDEXED ){
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
    স্থির ধ্রুবক u8 aStartOp[] = {
      0,
      0,
      OP_Rewind,           /* 2: (!start_constraints && startEq &&  !bRev) */
      OP_Last,             /* 3: (!start_constraints && startEq &&   bRev) */
      OP_SeekGT,           /* 4: (start_constraints  && !startEq && !bRev) */
      OP_SeekLT,           /* 5: (start_constraints  && !startEq &&  bRev) */
      OP_SeekGE,           /* 6: (start_constraints  &&  startEq && !bRev) */
      OP_SeekLE            /* 7: (start_constraints  &&  startEq &&  bRev) */
    };
    স্থির ধ্রুবক u8 aEndOp[] = {
      OP_IdxGE,            /* 0: (end_constraints && !bRev && !endEq) */
      OP_IdxGT,            /* 1: (end_constraints && !bRev &&  endEq) */
      OP_IdxLE,            /* 2: (end_constraints &&  bRev && !endEq) */
      OP_IdxLT,            /* 3: (end_constraints &&  bRev &&  endEq) */
    };
    u16 nEq = pLoop->u.btree.nEq;     /* Number of == or IN terms */
    u16 nBtm = pLoop->u.btree.nBtm;   /* Length of BTM vector */
    u16 n_শীর্ষ = pLoop->u.btree.n_শীর্ষ;   /* Length of TOP vector */
    পূর্ণসংখ্যা regBase;                 /* Base register holding constraint values */
    WhereTerm *pRangeStart = 0;  /* Inequality constraint at range start */
    WhereTerm *pRangeEnd = 0;    /* Inequality constraint at range end */
    পূর্ণসংখ্যা startEq;                 /* True if range start uses ==, >= or <= */
    পূর্ণসংখ্যা endEq;                   /* True if range end uses ==, >= or <= */
    পূর্ণসংখ্যা start_constraints;       /* Start of range is constrained */
    পূর্ণসংখ্যা nConstraint;             /* Number of constraint terms */
    পূর্ণসংখ্যা iIdxCur;                 /* The VDBE cursor for the index */
    পূর্ণসংখ্যা nExtraReg = 0;           /* Number of extra registers needed */
    পূর্ণসংখ্যা op;                      /* Instruction opcode */
    অক্ষর *zStartAff;             /* Affinity for start of range constraint */
    অক্ষর *zEndAff = 0;           /* Affinity for end of range constraint */
    u8 bSeekPastNull = 0;        /* True to seek past initial nulls */
    u8 bStopAtNull = 0;          /* Add condition to terminate at NULLs */
    পূর্ণসংখ্যা omitTable;               /* True if we use the index only */
    পূর্ণসংখ্যা regBignull = 0;          /* big-null flag register */
    পূর্ণসংখ্যা addrSeekScan = 0;        /* Opcode of the OP_SeekScan, if any */

    pIdx = pLoop->u.btree.p_সূচক;
    iIdxCur = pLevel->iIdxCur;
    প্রতিজ্ঞা( nEq>=pLoop->nSkip );

    /* Find any inequality constraint terms for the start and end
    ** of the range.
    */
    j = nEq;
    যদি( pLoop->wsFlags & WHERE_BTM_LIMIT ){
      pRangeStart = pLoop->aLTerm[j++];
      nExtraReg = MAX(nExtraReg, pLoop->u.btree.nBtm);
      /* Like optimization range constraints always occur in pairs */
      প্রতিজ্ঞা( (pRangeStart->wtFlags & TERM_LIKEOPT)==0 ||
              (pLoop->wsFlags & WHERE_শীর্ষ_LIMIT)!=0 );
    }
    যদি( pLoop->wsFlags & WHERE_শীর্ষ_LIMIT ){
      pRangeEnd = pLoop->aLTerm[j++];
      nExtraReg = MAX(nExtraReg, pLoop->u.btree.n_শীর্ষ);
#ifndef SQLITE_LIKE_DOESNT_MATCH_BLOBS
      যদি( (pRangeEnd->wtFlags & TERM_LIKEOPT)!=0 ){
        প্রতিজ্ঞা( pRangeStart!=0 );                     /* LIKE opt constraints */
        প্রতিজ্ঞা( pRangeStart->wtFlags & TERM_LIKEOPT );   /* occur in pairs */
        pLevel->iLikeRepCntr = (u32)++pParse->nMem;
        /* iLikeRepCntr actually stores 2x the counter register number.  The
        ** bottom bit indicates whether the search order is ASC or DESC. */
        testcase( bRev );
        testcase( pIdx->aSortOrder[nEq]==SQLITE_SO_DESC );
        প্রতিজ্ঞা( (bRev & ~1)==0 );
        pLevel->iLikeRepCntr <<=1;
        pLevel->iLikeRepCntr |= bRev ^ (pIdx->aSortOrder[nEq]==SQLITE_SO_DESC);
      }
#endif
      যদি( pRangeStart==0 ){
        j = pIdx->aiColumn[nEq];
        যদি( (j>=0 && pIdx->pTable->aCol[j].notNull==0) || j==XN_রাশিমালা ){
          bSeekPastNull = 1;
        }
      }
    }
    প্রতিজ্ঞা( pRangeEnd==0 || (pRangeEnd->wtFlags & TERM_VNULL)==0 );

    /* If the WHERE_BIGNULL_SORT flag is set, then index column nEq uses
    ** a non-default "big-null" sort (either ASC NULLS LAST or DESC NULLS
    ** FIRST). In both cases separate ordered scans are made of those
    ** index entries for which the column is null and for those for which
    ** it is not. For an ASC sort, the non-NULL entries are scanned first.
    ** For DESC, NULL entries are scanned first.
    */
    যদি( (pLoop->wsFlags & (WHERE_শীর্ষ_LIMIT|WHERE_BTM_LIMIT))==0
     && (pLoop->wsFlags & WHERE_BIGNULL_SORT)!=0
    ){
      প্রতিজ্ঞা( bSeekPastNull==0 && nExtraReg==0 && nBtm==0 && n_শীর্ষ==0 );
      প্রতিজ্ঞা( pRangeEnd==0 && pRangeStart==0 );
      testcase( pLoop->nSkip>0 );
      nExtraReg = 1;
      bSeekPastNull = 1;
      pLevel->regBignull = regBignull = ++pParse->nMem;
      যদি( pLevel->i_বাম_Join ){
        sqlite3VdbeAddOp2(v, OP_Integer, 0, regBignull);
      }
      pLevel->addrBignull = sqlite3VdbeMakeLabel(pParse);
    }

    /* If we are doing a reverse order scan on an ascending index, or
    ** a forward order scan on a descending index, interchange the
    ** start and end terms (pRangeStart and pRangeEnd).
    */
    যদি( (nEq<pIdx->nColumn && bRev==(pIdx->aSortOrder[nEq]==SQLITE_SO_ASC)) ){
      SWAP(WhereTerm *, pRangeEnd, pRangeStart);
      SWAP(u8, bSeekPastNull, bStopAtNull);
      SWAP(u8, nBtm, n_শীর্ষ);
    }

    যদি( iLevel>0 && (pLoop->wsFlags & WHERE_IN_SEEKSCAN)!=0 ){
      /* In case OP_SeekScan is used, ensure that the index cursor does not
      ** point to a valid row for the first iteration of this loop. */
      sqlite3VdbeAddOp1(v, OP_NullRow, iIdxCur);
    }

    /* Generate code to evaluate all constraint terms using == or IN
    ** and store the values of those terms in an array of registers
    ** starting at regBase.
    */
    codeCursorHint(pTabItem, pWInfo, pLevel, pRangeEnd);
    regBase = codeAllEqualityTerms(pParse,pLevel,bRev,nExtraReg,&zStartAff);
    প্রতিজ্ঞা( zStartAff==0 || sqlite3Strlen30(zStartAff)>=nEq );
#ifndef SQLITE_LIKE_DOESNT_MATCH_BLOBS
    যদি( pLevel->iLikeRepCntr ){
      /* The LIKE optimization loop must be the innermost of any IN(...)
      ** or skip-scan loops. Otherwise rows may not be visited in index 
      ** order. This means it needs an addrNxt label for the same reason
      ** IN(...) scans do.  */
      যদি( pLevel->addrNxt==pLevel->addrBrk ){
        pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3VdbeAddOp2(v, OP_Integer, 1, (পূর্ণসংখ্যা)(pLevel->iLikeRepCntr>>1));
      VdbeComment((v, "LIKE loop counter"));
      pLevel->addrLikeRep = sqlite3VdbeCurrentAddr(v);
    }
#endif
    যদি( zStartAff && n_শীর্ষ ){
      zEndAff = sqlite3DbStrDup(db, &zStartAff[nEq]);
    }
    addrNxt = (regBignull ? pLevel->addrBignull : pLevel->addrNxt);
    যদি( addrMiss==0 ) addrMiss = addrNxt;

    testcase( pRangeStart && (pRangeStart->eOperator & WO_LE)!=0 );
    testcase( pRangeStart && (pRangeStart->eOperator & WO_GE)!=0 );
    testcase( pRangeEnd && (pRangeEnd->eOperator & WO_LE)!=0 );
    testcase( pRangeEnd && (pRangeEnd->eOperator & WO_GE)!=0 );
    startEq = !pRangeStart || pRangeStart->eOperator & (WO_LE|WO_GE);
    endEq =   !pRangeEnd || pRangeEnd->eOperator & (WO_LE|WO_GE);
    start_constraints = pRangeStart || nEq>0;

    /* Seek the index cursor to the start of the range. */
    nConstraint = nEq;
    যদি( pRangeStart ){
      রাশিমালা *p_ডান = pRangeStart->p_রাশিমালা->p_ডান;
      code_রাশিমালা_Or_Vector(pParse, p_ডান, regBase+nEq, nBtm);
      whereLikeOptimizationStringFixup(v, pLevel, pRangeStart);
      যদি( (pRangeStart->wtFlags & TERM_VNULL)==0
       && sqlite3_রাশিমালা_Can_Be_Null(p_ডান)
      ){
        sqlite3VdbeAddOp2(v, OP_IsNull, regBase+nEq, addrNxt);
        VdbeCoverage(v);
      }
      যদি( zStartAff ){
        updateRangeAffinityStr(p_ডান, nBtm, &zStartAff[nEq]);
      }
      nConstraint += nBtm;
      testcase( pRangeStart->wtFlags & TERM_VIRTUAL );
      যদি( sqlite3_রাশিমালা_Is_Vector(p_ডান)==0 ){
        disableTerm(pLevel, pRangeStart);
      }অন্যথা{
        startEq = 1;
      }
      bSeekPastNull = 0;
    }অন্যথা যদি( bSeekPastNull ){
      startEq = 0;
      sqlite3VdbeAddOp2(v, OP_Null, 0, regBase+nEq);
      start_constraints = 1;
      nConstraint++;
    }অন্যথা যদি( regBignull ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, regBase+nEq);
      start_constraints = 1;
      nConstraint++;
    }
    codeApplyAffinity(pParse, regBase, nConstraint - bSeekPastNull, zStartAff);
    যদি( pLoop->nSkip>0 && nConstraint==pLoop->nSkip ){
      /* The skip-scan logic inside the call to codeAllEqualityConstraints()
      ** above has already left the cursor sitting on the correct row,
      ** so no further seeking is needed */
    }অন্যথা{
      যদি( regBignull ){
        sqlite3VdbeAddOp2(v, OP_Integer, 1, regBignull);
        VdbeComment((v, "NULL-scan pass ctr"));
      }
      যদি( pLevel->regFilter ){
        প্রতিজ্ঞা( sqlite3WhereLoopBloomable(pLoop) );
        sqlite3VdbeAddOp4Int(v, OP_Filter, pLevel->regFilter, addrMiss,
                             regBase, nEq);
        VdbeCoverage(v);
        filterPullDown(pParse, pWInfo, iLevel, addrNxt, notReady);
      }

      op = aStartOp[(start_constraints<<2) + (startEq<<1) + bRev];
      প্রতিজ্ঞা( op!=0 );
      যদি( (pLoop->wsFlags & WHERE_IN_SEEKSCAN)!=0 && op==OP_SeekGE ){
        প্রতিজ্ঞা( regBignull==0 );
        /* TUNING:  The OP_SeekScan opcode seeks to reduce the number
        ** of expensive seek operations by replacing a single seek with
        ** 1 or more step operations.  The question is, how many steps
        ** should we try before giving up and going with a seek.  The cost
        ** of a seek is proportional to the logarithm of the of the number
        ** of entries in the tree, so basing the number of steps to try
        ** on the estimated number of rows in the btree seems like a good
        ** guess. */
        addrSeekScan = sqlite3VdbeAddOp1(v, OP_SeekScan,
                                         (pIdx->aiRowLogEst[0]+9)/10);
        যদি( pRangeStart || pRangeEnd ){
          sqlite3VdbeChangeP5(v, 1);
          sqlite3VdbeChangeP2(v, addrSeekScan, sqlite3VdbeCurrentAddr(v)+1);
          addrSeekScan = 0;
        }
        VdbeCoverage(v);
      }
      sqlite3VdbeAddOp4Int(v, op, iIdxCur, addrMiss, regBase, nConstraint);
      VdbeCoverage(v);
      VdbeCoverageIf(v, op==OP_Rewind);  testcase( op==OP_Rewind );
      VdbeCoverageIf(v, op==OP_Last);    testcase( op==OP_Last );
      VdbeCoverageIf(v, op==OP_SeekGT);  testcase( op==OP_SeekGT );
      VdbeCoverageIf(v, op==OP_SeekGE);  testcase( op==OP_SeekGE );
      VdbeCoverageIf(v, op==OP_SeekLE);  testcase( op==OP_SeekLE );
      VdbeCoverageIf(v, op==OP_SeekLT);  testcase( op==OP_SeekLT );

      প্রতিজ্ঞা( bSeekPastNull==0 || bStopAtNull==0 );
      যদি( regBignull ){
        প্রতিজ্ঞা( bSeekPastNull==1 || bStopAtNull==1 );
        প্রতিজ্ঞা( bSeekPastNull==!bStopAtNull );
        প্রতিজ্ঞা( bStopAtNull==startEq );
        sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        op = aStartOp[(nConstraint>1)*4 + 2 + bRev];
        sqlite3VdbeAddOp4Int(v, op, iIdxCur, addrNxt, regBase,
                             nConstraint-startEq);
        VdbeCoverage(v);
        VdbeCoverageIf(v, op==OP_Rewind);  testcase( op==OP_Rewind );
        VdbeCoverageIf(v, op==OP_Last);    testcase( op==OP_Last );
        VdbeCoverageIf(v, op==OP_SeekGE);  testcase( op==OP_SeekGE );
        VdbeCoverageIf(v, op==OP_SeekLE);  testcase( op==OP_SeekLE );
        প্রতিজ্ঞা( op==OP_Rewind || op==OP_Last || op==OP_SeekGE || op==OP_SeekLE);
      }
    }

    /* Load the value for the inequality constraint at the end of the
    ** range (if any).
    */
    nConstraint = nEq;
    প্রতিজ্ঞা( pLevel->p2==0 );
    যদি( pRangeEnd ){
      রাশিমালা *p_ডান = pRangeEnd->p_রাশিমালা->p_ডান;
      প্রতিজ্ঞা( addrSeekScan==0 );
      code_রাশিমালা_Or_Vector(pParse, p_ডান, regBase+nEq, n_শীর্ষ);
      whereLikeOptimizationStringFixup(v, pLevel, pRangeEnd);
      যদি( (pRangeEnd->wtFlags & TERM_VNULL)==0
       && sqlite3_রাশিমালা_Can_Be_Null(p_ডান)
      ){
        sqlite3VdbeAddOp2(v, OP_IsNull, regBase+nEq, addrNxt);
        VdbeCoverage(v);
      }
      যদি( zEndAff ){
        updateRangeAffinityStr(p_ডান, n_শীর্ষ, zEndAff);
        codeApplyAffinity(pParse, regBase+nEq, n_শীর্ষ, zEndAff);
      }অন্যথা{
        প্রতিজ্ঞা( pParse->db->mallocFailed );
      }
      nConstraint += n_শীর্ষ;
      testcase( pRangeEnd->wtFlags & TERM_VIRTUAL );

      যদি( sqlite3_রাশিমালা_Is_Vector(p_ডান)==0 ){
        disableTerm(pLevel, pRangeEnd);
      }অন্যথা{
        endEq = 1;
      }
    }অন্যথা যদি( bStopAtNull ){
      যদি( regBignull==0 ){
        sqlite3VdbeAddOp2(v, OP_Null, 0, regBase+nEq);
        endEq = 0;
      }
      nConstraint++;
    }
    যদি( zStartAff ) sqlite3DbNNFreeNN(db, zStartAff);
    যদি( zEndAff ) sqlite3DbNNFreeNN(db, zEndAff);

    /* Top of the loop body */
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    /* Check if the index cursor is past the end of the range. */
    যদি( nConstraint ){
      যদি( regBignull ){
        /* Except, skip the end-of-range check while doing the NULL-scan */
        sqlite3VdbeAddOp2(v, OP_IfNot, regBignull, sqlite3VdbeCurrentAddr(v)+3);
        VdbeComment((v, "If NULL-scan 2nd pass"));
        VdbeCoverage(v);
      }
      op = aEndOp[bRev*2 + endEq];
      sqlite3VdbeAddOp4Int(v, op, iIdxCur, addrNxt, regBase, nConstraint);
      testcase( op==OP_IdxGT );  VdbeCoverageIf(v, op==OP_IdxGT );
      testcase( op==OP_IdxGE );  VdbeCoverageIf(v, op==OP_IdxGE );
      testcase( op==OP_IdxLT );  VdbeCoverageIf(v, op==OP_IdxLT );
      testcase( op==OP_IdxLE );  VdbeCoverageIf(v, op==OP_IdxLE );
      যদি( addrSeekScan ) sqlite3VdbeJumpHere(v, addrSeekScan);
    }
    যদি( regBignull ){
      /* During a NULL-scan, check to see if we have reached the end of
      ** the NULLs */
      প্রতিজ্ঞা( bSeekPastNull==!bStopAtNull );
      প্রতিজ্ঞা( bSeekPastNull+bStopAtNull==1 );
      প্রতিজ্ঞা( nConstraint+bSeekPastNull>0 );
      sqlite3VdbeAddOp2(v, OP_If, regBignull, sqlite3VdbeCurrentAddr(v)+2);
      VdbeComment((v, "If NULL-scan 1st pass"));
      VdbeCoverage(v);
      op = aEndOp[bRev*2 + bSeekPastNull];
      sqlite3VdbeAddOp4Int(v, op, iIdxCur, addrNxt, regBase,
                           nConstraint+bSeekPastNull);
      testcase( op==OP_IdxGT );  VdbeCoverageIf(v, op==OP_IdxGT );
      testcase( op==OP_IdxGE );  VdbeCoverageIf(v, op==OP_IdxGE );
      testcase( op==OP_IdxLT );  VdbeCoverageIf(v, op==OP_IdxLT );
      testcase( op==OP_IdxLE );  VdbeCoverageIf(v, op==OP_IdxLE );
    }

    যদি( (pLoop->wsFlags & WHERE_IN_EARLYOUT)!=0 ){
      sqlite3VdbeAddOp3(v, OP_SeekHit, iIdxCur, nEq, nEq);
    }

    /* Seek the table cursor, if required */
    omitTable = (pLoop->wsFlags & WHERE_IDX_ONLY)!=0
           && (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_ডান_JOIN))==0;
    যদি( omitTable ){
      /* pIdx is a covering index.  No need to access the main table. */
    }অন্যথা যদি( HasRowid(pIdx->pTable) ){
      codeDeferredSeek(pWInfo, pIdx, iCur, iIdxCur);
    }অন্যথা যদি( iCur!=iIdxCur ){
      সূচক *pPk = sqlite3_Primary_চাবি_সূচক(pIdx->pTable);
      iRowidReg = sqlite3_Get_অস্থায়ী_Range(pParse, pPk->n_চাবি_Col);
      জন্য(j=0; j<pPk->n_চাবি_Col; j++){
        k = sqlite3_Table_Column_To_সূচক(pIdx, pPk->aiColumn[j]);
        sqlite3VdbeAddOp3(v, OP_Column, iIdxCur, k, iRowidReg+j);
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iCur, addrCont,
                           iRowidReg, pPk->n_চাবি_Col); VdbeCoverage(v);
    }

    যদি( pLevel->i_বাম_Join==0 ){
      /* If a partial index is driving the loop, try to eliminate WHERE clause
      ** terms from the query that must be true due to the WHERE clause of
      ** the partial index.  This optimization does not work on an outer join,
      ** as shown by:
      **
      ** 2019-11-02 ticket 623eff57e76d45f6      (LEFT JOIN)
      ** 2025-05-29 forum post 7dee41d32506c4ae  (RIGHT JOIN)
      */
      যদি( pIdx->pPartIdxWhere && pLevel->pRJ==0 ){
        where_Apply_Partial_সূচক_Constraints(pIdx->pPartIdxWhere, iCur, pWC);
      }
    }অন্যথা{
      testcase( pIdx->pPartIdxWhere );
      /* The following assert() is not a requirement, merely an observation:
      ** The OR-optimization doesn't work for the right hand table of
      ** a LEFT JOIN: */
      প্রতিজ্ঞা( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_ডান_JOIN))==0 );
    }

    /* Record the instruction used to terminate the loop. */
    যদি( (pLoop->wsFlags & WHERE_ONEROW)
     || (pLevel->u.in.nIn && regBignull==0 && whereLoopIsOneRow(pLoop))
    ){
      pLevel->op = OP_Noop;
    }অন্যথা যদি( bRev ){
      pLevel->op = OP_পূর্ব;
    }অন্যথা{
      pLevel->op = OP_পরবর্তী;
    }
    pLevel->p1 = iIdxCur;
    pLevel->p3 = (pLoop->wsFlags&WHERE_UNQ_WANTED)!=0 ? 1:0;
    যদি( (pLoop->wsFlags & WHERE_CONSTRAINT)==0 ){
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_STEP;
    }অন্যথা{
      প্রতিজ্ঞা( pLevel->p5==0 );
    }
    যদি( omitTable ) pIdx = 0;
  }অন্যথা

#ifndef SQLITE_OMIT_OR_OPTIMIZATION
  যদি( pLoop->wsFlags & WHERE_MULTI_OR ){
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
    সূচক *pCov = 0;             /* Potential covering index (or NULL) */
    পূর্ণসংখ্যা iCovCur = pParse->nTab++;  /* Cursor used for index scans (if any) */

    পূর্ণসংখ্যা regReturn = ++pParse->nMem;           /* Register used with OP_Gosub */
    পূর্ণসংখ্যা regRowset = 0;                        /* Register for RowSet object */
    পূর্ণসংখ্যা regRowid = 0;                         /* Register holding rowid */
    পূর্ণসংখ্যা iLoopBody = sqlite3VdbeMakeLabel(pParse);/* Start of loop body */
    পূর্ণসংখ্যা iRetInit;                             /* Address of regReturn init */
    পূর্ণসংখ্যা untestedTerms = 0;             /* Some terms not completely tested */
    পূর্ণসংখ্যা ii;                            /* Loop counter */
    রাশিমালা *p_And_রাশিমালা = 0;                /* An ".. AND (...)" expression */
    Table *pTab = pTabItem->pSTab;

    pTerm = pLoop->aLTerm[0];
    প্রতিজ্ঞা( pTerm!=0 );
    প্রতিজ্ঞা( pTerm->eOperator & WO_OR );
    প্রতিজ্ঞা( (pTerm->wtFlags & TERM_ORINFO)!=0 );
    pOrWc = &pTerm->u.pOrInfo->wc;
    pLevel->op = OP_Return;
    pLevel->p1 = regReturn;

    /* Set up a new SrcList in pOrTab containing the table being scanned
    ** by this loop in the a[0] slot and all notReady tables in a[1..] slots.
    ** This becomes the SrcList in the recursive call to sqlite3WhereBegin().
    */
    যদি( pWInfo->nLevel>1 || pTabItem->fg.fromExists ){
      পূর্ণসংখ্যা nNotReady;                 /* The number of notReady tables */
      SrcItem *origSrc;              /* Original list of tables */
      nNotReady = pWInfo->nLevel - iLevel - 1;
      pOrTab = sqlite3DbMallocRawNN(db, SZ_SRCLIST(nNotReady+1));
      যদি( pOrTab==0 ) ফেরত notReady;
      pOrTab->nAlloc = (u8)(nNotReady + 1);
      pOrTab->nSrc = pOrTab->nAlloc;
      memcpy(pOrTab->a, pTabItem, আকার(*pTabItem));
      origSrc = pWInfo->pTabList->a;
      জন্য(k=1; k<=nNotReady; k++){
        memcpy(&pOrTab->a[k], &origSrc[pLevel[k].iFrom], আকার(pOrTab->a[k]));
      }

      /* Clear the fromExists flag on the OR-optimized table entry so that
      ** the calls to sqlite3WhereEnd() do not code early-exits after the
      ** first row is visited. The early exit applies to this table's
      ** overall loop - including the multiple OR branches and any WHERE
      ** conditions not passed to the sub-loops - not to the sub-loops.  */
      pOrTab->a[0].fg.fromExists = 0;
    }অন্যথা{
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
    যদি( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
      যদি( HasRowid(pTab) ){
        regRowset = ++pParse->nMem;
        sqlite3VdbeAddOp2(v, OP_Null, 0, regRowset);
      }অন্যথা{
        সূচক *pPk = sqlite3_Primary_চাবি_সূচক(pTab);
        regRowset = pParse->nTab++;
        sqlite3VdbeAddOp2(v, OP_OpenEphemeral, regRowset, pPk->n_চাবি_Col);
        sqlite3_Vdbe_Set_P4_চাবি_Info(pParse, pPk);
      }
      regRowid = ++pParse->nMem;
    }
    iRetInit = sqlite3VdbeAddOp2(v, OP_Integer, 0, regReturn);

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
    যদি( pWC->nTerm>1 ){
      পূর্ণসংখ্যা iTerm;
      জন্য(iTerm=0; iTerm<pWC->nTerm; iTerm++){
        রাশিমালা *p_রাশিমালা = pWC->a[iTerm].p_রাশিমালা;
        যদি( &pWC->a[iTerm] == pTerm ) চালু;
        testcase( pWC->a[iTerm].wtFlags & TERM_VIRTUAL );
        testcase( pWC->a[iTerm].wtFlags & TERM_CODED );
        testcase( pWC->a[iTerm].wtFlags & TERM_SLICE );
        যদি( (pWC->a[iTerm].wtFlags & (TERM_VIRTUAL|TERM_CODED|TERM_SLICE))!=0 ){
          চালু;
        }
        যদি( (pWC->a[iTerm].eOperator & WO_ALL)==0 ) চালু;
        যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_Subquery) ) চালু;  /* tag-20220303a */
        যদি( where_রাশিমালা_Is_Ready(pWInfo, pLevel, pTabItem, p_রাশিমালা)==0 ) চালু;
        p_রাশিমালা = sqlite3_রাশিমালা_Dup(db, p_রাশিমালা, 0);
        p_And_রাশিমালা = sqlite3_রাশিমালা_And(pParse, p_And_রাশিমালা, p_রাশিমালা);
      }
      যদি( p_And_রাশিমালা ){
        /* The extra 0x10000 bit on the opcode is masked off and does not
        ** become part of the new Expr.op.  However, it does make the
        ** op==TK_AND comparison inside of sqlite3PExpr() false, and this
        ** prevents sqlite3PExpr() from applying the AND short-circuit
        ** optimization, which we do not want here. */
        p_And_রাশিমালা = sqlite3PExpr(pParse, TK_AND|0x10000, 0, p_And_রাশিমালা);
      }
    }

    /* Run a separate WHERE clause for each term of the OR clause.  After
    ** eliminating duplicates from other WHERE clauses, the action for each
    ** sub-WHERE clause is to to invoke the main loop body as a subroutine.
    */
    ExplainQueryPlan((pParse, 1, "MULTI-INDEX OR"));
    জন্য(ii=0; ii<pOrWc->nTerm; ii++){
      WhereTerm *pOrTerm = &pOrWc->a[ii];
      যদি( pOrTerm->বাম_Cursor==iCur || (pOrTerm->eOperator & WO_AND)!=0 ){
        WhereInfo *pSubWInfo;           /* Info for single OR-term scan */
        রাশিমালা *p_Or_রাশিমালা = pOrTerm->p_রাশিমালা; /* Current OR clause term */
        রাশিমালা *pDelete;                  /* Local copy of OR clause term */
        পূর্ণসংখ্যা jmp1 = 0;                   /* Address of jump operation */
        testcase( (pTabItem[0].fg.jointype & JT_বাম)!=0
               && !রাশিমালা_Has_Property(p_Or_রাশিমালা, EP_OuterON)
        ); /* See TH3 vtab25.400 and ticket 614b25314c766238 */
        pDelete = p_Or_রাশিমালা = sqlite3_রাশিমালা_Dup(db, p_Or_রাশিমালা, 0);
        যদি( db->mallocFailed ){
          sqlite3_রাশিমালা_Delete(db, pDelete);
          চালু;
        }
        যদি( p_And_রাশিমালা ){
          p_And_রাশিমালা->p_বাম = p_Or_রাশিমালা;
          p_Or_রাশিমালা = p_And_রাশিমালা;
        }
        /* Loop through table entries that match term pOrTerm. */
        ExplainQueryPlan((pParse, 1, "INDEX %d", ii+1));
        WHERETRACE(0xffffffff, ("Subplan for OR-clause:\n"));
        pSubWInfo = sqlite3WhereBegin(pParse, pOrTab, p_Or_রাশিমালা, 0, 0, 0,
                                      WHERE_OR_SUBCLAUSE, iCovCur);
        প্রতিজ্ঞা( pSubWInfo || pParse->nErr );
        যদি( pSubWInfo ){
          WhereLoop *pSubLoop;
          পূর্ণসংখ্যা addrExplain = sqlite3WhereExplainOneScan(
              pParse, pOrTab, &pSubWInfo->a[0], 0
          );
          sqlite3WhereAddScanStatus(v, pOrTab, &pSubWInfo->a[0], addrExplain);

          /* This is the sub-WHERE clause body.  First skip over
          ** duplicate rows from prior sub-WHERE clauses, and record the
          ** rowid (or PRIMARY KEY) for the current row so that the same
          ** row will be skipped in subsequent sub-WHERE clauses.
          */
          যদি( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
            পূর্ণসংখ্যা iSet = ((ii==pOrWc->nTerm-1)?-1:ii);
            যদি( HasRowid(pTab) ){
              sqlite3_রাশিমালা_Code_Get_Column_Of_Table(v, pTab, iCur, -1, regRowid);
              jmp1 = sqlite3VdbeAddOp4Int(v, OP_RowSetTest, regRowset, 0,
                                          regRowid, iSet);
              VdbeCoverage(v);
            }অন্যথা{
              সূচক *pPk = sqlite3_Primary_চাবি_সূচক(pTab);
              পূর্ণসংখ্যা nPk = pPk->n_চাবি_Col;
              পূর্ণসংখ্যা iPk;
              পূর্ণসংখ্যা r;

              /* Read the PK into an array of temp registers. */
              r = sqlite3_Get_অস্থায়ী_Range(pParse, nPk);
              জন্য(iPk=0; iPk<nPk; iPk++){
                পূর্ণসংখ্যা iCol = pPk->aiColumn[iPk];
                sqlite3_রাশিমালা_Code_Get_Column_Of_Table(v, pTab, iCur, iCol,r+iPk);
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
              যদি( iSet ){
                jmp1 = sqlite3VdbeAddOp4Int(v, OP_Found, regRowset, 0, r, nPk);
                VdbeCoverage(v);
              }
              যদি( iSet>=0 ){
                sqlite3VdbeAddOp3(v, OP_MakeRecord, r, nPk, regRowid);
                sqlite3VdbeAddOp4Int(v, OP_Idx_ঢোকাও, regRowset, regRowid,
                                     r, nPk);
                যদি( iSet ) sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
              }

              /* Release the array of temp registers */
              sqlite3_Release_অস্থায়ী_Range(pParse, r, nPk);
            }
          }

          /* Invoke the main loop body as a subroutine */
          sqlite3VdbeAddOp2(v, OP_Gosub, regReturn, iLoopBody);

          /* Jump here (skipping the main loop body subroutine) if the
          ** current sub-WHERE row is a duplicate from prior sub-WHEREs. */
          যদি( jmp1 ) sqlite3VdbeJumpHere(v, jmp1);

          /* The pSubWInfo->untestedTerms flag means that this OR term
          ** contained one or more AND term from a notReady table.  The
          ** terms from the notReady table could not be tested and will
          ** need to be tested later.
          */
          যদি( pSubWInfo->untestedTerms ) untestedTerms = 1;

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
          প্রতিজ্ঞা( (pSubLoop->wsFlags & WHERE_AUTO_সূচক)==0 );
          যদি( (pSubLoop->wsFlags & WHERE_INDEXED)!=0
           && (ii==0 || pSubLoop->u.btree.p_সূচক==pCov)
           && (HasRowid(pTab) || !Is_Primary_চাবি_সূচক(pSubLoop->u.btree.p_সূচক))
          ){
            প্রতিজ্ঞা( pSubWInfo->a[0].iIdxCur==iCovCur );
            pCov = pSubLoop->u.btree.p_সূচক;
          }অন্যথা{
            pCov = 0;
          }
          যদি( sqlite3WhereUsesDeferredSeek(pSubWInfo) ){
            pWInfo->bDeferredSeek = 1;
          }

          /* Finish the loop through table entries that match term pOrTerm. */
          sqlite3WhereEnd(pSubWInfo);
          Explain_Query_Plan_বের_করো(pParse);
        }
        sqlite3_রাশিমালা_Delete(db, pDelete);
      }
    }
    Explain_Query_Plan_বের_করো(pParse);
    প্রতিজ্ঞা( pLevel->pWLoop==pLoop );
    প্রতিজ্ঞা( (pLoop->wsFlags & WHERE_MULTI_OR)!=0 );
    প্রতিজ্ঞা( (pLoop->wsFlags & WHERE_IN_ABLE)==0 );
    pLevel->u.pCoveringIdx = pCov;
    যদি( pCov ) pLevel->iIdxCur = iCovCur;
    যদি( p_And_রাশিমালা ){
      p_And_রাশিমালা->p_বাম = 0;
      sqlite3_রাশিমালা_Delete(db, p_And_রাশিমালা);
    }
    sqlite3VdbeChangeP1(v, iRetInit, sqlite3VdbeCurrentAddr(v));
    sqlite3VdbeGoto(v, pLevel->addrBrk);
    sqlite3VdbeResolveLabel(v, iLoopBody);

    /* Set the P2 operand of the OP_Return opcode that will end the current
    ** loop to point to this spot, which is the top of the next containing
    ** loop.  The byte-code formatter will use that P2 value as a hint to
    ** indent everything in between the this point and the final OP_Return.
    ** See tag-20220407a in vdbe.c and shell.c */
    প্রতিজ্ঞা( pLevel->op==OP_Return );
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    যদি( pWInfo->pTabList!=pOrTab ){ sqlite3DbFreeNN(db, pOrTab); }
    যদি( !untestedTerms ) disableTerm(pLevel, pTerm);
  }অন্যথা
#endif /* SQLITE_OMIT_OR_OPTIMIZATION */

  {
    /* Case 6:  There is no usable index.  We must do a complete
    **          scan of the entire table.
    */
    স্থির ধ্রুবক u8 aStep[] = { OP_পরবর্তী, OP_পূর্ব };
    স্থির ধ্রুবক u8 aStart[] = { OP_Rewind, OP_Last };
    প্রতিজ্ঞা( bRev==0 || bRev==1 );
    যদি( pTabItem->fg.isRecursive ){
      /* Tables marked isRecursive have only a single row that is stored in
      ** a pseudo-cursor.  No need to Rewind or Next such cursors. */
      pLevel->op = OP_Noop;
    }অন্যথা{
      codeCursorHint(pTabItem, pWInfo, pLevel, 0);
      pLevel->op = aStep[bRev];
      pLevel->p1 = iCur;
      pLevel->p2 = 1 + sqlite3VdbeAddOp2(v, aStart[bRev],iCur,pLevel->addrHalt);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_STEP;
    }
  }

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  pLevel->addrVisit = sqlite3VdbeCurrentAddr(v);
#endif

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
  করো{
    পূর্ণসংখ্যা i_পরবর্তী = 0;                /* Next value for iLoop */
    জন্য(pTerm=pWC->a, j=pWC->nTerm; j>0; j--, pTerm++){
      রাশিমালা *pE;
      পূর্ণসংখ্যা skipLikeAddr = 0;
      testcase( pTerm->wtFlags & TERM_VIRTUAL );
      testcase( pTerm->wtFlags & TERM_CODED );
      যদি( pTerm->wtFlags & (TERM_VIRTUAL|TERM_CODED) ) চালু;
      যদি( (pTerm->prereqAll & pLevel->notReady)!=0 ){
        testcase( pWInfo->untestedTerms==0
            && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 );
        pWInfo->untestedTerms = 1;
        চালু;
      }
      pE = pTerm->p_রাশিমালা;
      প্রতিজ্ঞা( pE!=0 );
      যদি( where_রাশিমালা_Is_Ready(pWInfo, pLevel, pTabItem, pE)==0 ) চালু;
      যদি( iLoop==1 && !sqlite3_রাশিমালা_Covered_By_সূচক(pE, pLevel->iTabCur, pIdx) ){
        i_পরবর্তী = 2;
        চালু;
      }
      যদি( iLoop<3 && (pTerm->wtFlags & TERM_VARSELECT) ){
        যদি( i_পরবর্তী==0 ) i_পরবর্তী = 3;
        চালু;
      }

      যদি( (pTerm->wtFlags & TERM_LIKECOND)!=0 ){
        /* If the TERM_LIKECOND flag is set, that means that the range search
        ** is sufficient to guarantee that the LIKE operator is true, so we
        ** can skip the call to the like(A,B) function.  But this only works
        ** for strings.  So do not skip the call to the function on the pass
        ** that compares BLOBs. */
#ifdef SQLITE_LIKE_DOESNT_MATCH_BLOBS
        চালু;
#অন্যথা
        u32 x = pLevel->iLikeRepCntr;
        যদি( x>0 ){
          skipLikeAddr = sqlite3VdbeAddOp1(v, (x&1)?OP_IfNot:OP_If,(পূর্ণসংখ্যা)(x>>1));
          VdbeCoverageIf(v, (x&1)==1);
          VdbeCoverageIf(v, (x&1)==0);
        }
#endif
      }
#ifdef WHERETRACE_ENABLED /* 0xffffffff */
      যদি( sqlite3WhereTrace ){
        VdbeNoopComment((v, "WhereTerm[%d] (%p) priority=%d",
                         pWC->nTerm-j, pTerm, iLoop));
      }
      যদি( sqlite3WhereTrace & 0x4000 ){
        sqlite3DebugPrintf("Coding auxiliary constraint:\n");
        sqlite3WhereTermPrint(pTerm, pWC->nTerm-j);
      }
#endif
      sqlite3_রাশিমালা_If_False(pParse, pE, addrCont, SQLITE_JUMPIFNULL);
      যদি( skipLikeAddr ) sqlite3VdbeJumpHere(v, skipLikeAddr);
      pTerm->wtFlags |= TERM_CODED;
    }
    iLoop = i_পরবর্তী;
  }যখন( iLoop>0 );

  /* Insert code to test for implied constraints based on transitivity
  ** of the "==" operator.
  **
  ** Example: If the WHERE clause contains "t1.a=t2.b" and "t2.b=123"
  ** and we are coding the t1 loop and the t2 loop has not yet coded,
  ** then we cannot use the "t1.a=t2.b" constraint, but we can code
  ** the implied "t1.a=123" constraint.
  */
  জন্য(pTerm=pWC->a, j=pWC->nBase; j>0; j--, pTerm++){
    রাশিমালা *pE, sEAlt;
    WhereTerm *pAlt;
    যদি( pTerm->wtFlags & (TERM_VIRTUAL|TERM_CODED) ) চালু;
    যদি( (pTerm->eOperator & (WO_EQ|WO_IS))==0 ) চালু;
    যদি( (pTerm->eOperator & WO_EQUIV)==0 ) চালু;
    যদি( pTerm->বাম_Cursor!=iCur ) চালু;
    যদি( pTabItem->fg.jointype & (JT_বাম|JT_LTORJ|JT_ডান) ) চালু;
    pE = pTerm->p_রাশিমালা;
#ifdef WHERETRACE_ENABLED /* 0x4001 */
    যদি( (sqlite3WhereTrace & 0x4001)==0x4001 ){
      sqlite3DebugPrintf("Coding transitive constraint:\n");
      sqlite3WhereTermPrint(pTerm, pWC->nTerm-j);
    }
#endif
    প্রতিজ্ঞা( !রাশিমালা_Has_Property(pE, EP_OuterON) );
    প্রতিজ্ঞা( (pTerm->prereq_ডান & pLevel->notReady)!=0 );
    প্রতিজ্ঞা( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
    pAlt = sqlite3WhereFindTerm(pWC, iCur, pTerm->u.x.বাম_Column, notReady,
                    WO_EQ|WO_IN|WO_IS, 0);
    যদি( pAlt==0 ) চালু;
    যদি( pAlt->wtFlags & (TERM_CODED) ) চালু;
    যদি( রাশিমালা_Has_Property(pAlt->p_রাশিমালা, EP_Collate) ) চালু;
    যদি( (pAlt->eOperator & WO_IN)
     && রাশিমালা_Use_XSelect(pAlt->p_রাশিমালা)
     && (pAlt->p_রাশিমালা->x.pSelect->pEList->n_রাশিমালা>1)
    ){
      চালু;
    }
    testcase( pAlt->eOperator & WO_EQ );
    testcase( pAlt->eOperator & WO_IS );
    testcase( pAlt->eOperator & WO_IN );
    VdbeModuleComment((v, "begin transitive constraint"));
    sEAlt = *pAlt->p_রাশিমালা;
    sEAlt.p_বাম = pE->p_বাম;
    sqlite3_রাশিমালা_If_False(pParse, &sEAlt, addrCont, SQLITE_JUMPIFNULL);
    pAlt->wtFlags |= TERM_CODED;
  }

  /* For a RIGHT OUTER JOIN, record the fact that the current row has
  ** been matched at least once.
  */
  যদি( pLevel->pRJ ){
    Table *pTab;
    পূর্ণসংখ্যা nPk;
    পূর্ণসংখ্যা r;
    পূর্ণসংখ্যা jmp1 = 0;
    Where_ডান_Join *pRJ = pLevel->pRJ;

    /* pTab is the right-hand table of the RIGHT JOIN.  Generate code that
    ** will record that the current row of that table has been matched at
    ** least once.  This is accomplished by storing the PK for the row in
    ** both the iMatch index and the regBloom Bloom filter.
    */
    pTab = pWInfo->pTabList->a[pLevel->iFrom].pSTab;
    যদি( HasRowid(pTab) ){
      r = sqlite3_Get_অস্থায়ী_Range(pParse, 2);
      sqlite3_রাশিমালা_Code_Get_Column_Of_Table(v, pTab, pLevel->iTabCur, -1, r+1);
      nPk = 1;
    }অন্যথা{
      পূর্ণসংখ্যা iPk;
      সূচক *pPk = sqlite3_Primary_চাবি_সূচক(pTab);
      nPk = pPk->n_চাবি_Col;
      r = sqlite3_Get_অস্থায়ী_Range(pParse, nPk+1);
      জন্য(iPk=0; iPk<nPk; iPk++){
        পূর্ণসংখ্যা iCol = pPk->aiColumn[iPk];
        sqlite3_রাশিমালা_Code_Get_Column_Of_Table(v, pTab, iCur, iCol,r+1+iPk);
      }
    }
    jmp1 = sqlite3VdbeAddOp4Int(v, OP_Found, pRJ->iMatch, 0, r+1, nPk);
    VdbeCoverage(v);
    VdbeComment((v, "match against %s", pTab->zName));
    sqlite3VdbeAddOp3(v, OP_MakeRecord, r+1, nPk, r);
    sqlite3VdbeAddOp4Int(v, OP_Idx_ঢোকাও, pRJ->iMatch, r, r+1, nPk);
    যদি( pRJ->regBloom ){
      sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pRJ->regBloom, 0, r+1, nPk);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
    }
    sqlite3VdbeJumpHere(v, jmp1);
    sqlite3_Release_অস্থায়ী_Range(pParse, r, nPk+1);
  }

  /* For a LEFT OUTER JOIN, generate code that will record the fact that
  ** at least one row of the right table has matched the left table.
  */
  যদি( pLevel->i_বাম_Join ){
    pLevel->addrFirst = sqlite3VdbeCurrentAddr(v);
    sqlite3VdbeAddOp2(v, OP_Integer, 1, pLevel->i_বাম_Join);
    VdbeComment((v, "record LEFT JOIN hit"));
    যদি( pLevel->pRJ==0 ){
      যাও code_outer_join_constraints; /* WHERE clause constraints */
    }
  }

  যদি( pLevel->pRJ ){
    /* Create a subroutine used to process all interior loops and code
    ** of the RIGHT JOIN.  During normal operation, the subroutine will
    ** be in-line with the rest of the code.  But at the end, a separate
    ** loop will run that invokes this subroutine for unmatched rows
    ** of pTab, with all tables to left begin set to NULL.
    */
    {
      Where_ডান_Join *pRJ = pLevel->pRJ;
      sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, pRJ->regReturn);
      pRJ->addrSubrtn = sqlite3VdbeCurrentAddr(v);
    }
    প্রতিজ্ঞা( pParse->withinRJSubrtn < 255 );
    pParse->withinRJSubrtn++;

    /* WHERE clause constraints must be deferred until after outer join
    ** row elimination has completed, since WHERE clause constraints apply
    ** to the results of the OUTER JOIN.  The following loop generates the
    ** appropriate WHERE clause constraint checks.  tag-20220513a.
    */
  code_outer_join_constraints:
    জন্য(pTerm=pWC->a, j=0; j<pWC->nBase; j++, pTerm++){
      testcase( pTerm->wtFlags & TERM_VIRTUAL );
      testcase( pTerm->wtFlags & TERM_CODED );
      যদি( pTerm->wtFlags & (TERM_VIRTUAL|TERM_CODED) ) চালু;
      যদি( (pTerm->prereqAll & pLevel->notReady)!=0 ){
        প্রতিজ্ঞা( pWInfo->untestedTerms );
        চালু;
      }
      যদি( pTabItem->fg.jointype & JT_LTORJ ) চালু;
      প্রতিজ্ঞা( pTerm->p_রাশিমালা );
      sqlite3_রাশিমালা_If_False(pParse, pTerm->p_রাশিমালা, addrCont, SQLITE_JUMPIFNULL);
      pTerm->wtFlags |= TERM_CODED;
    }
  }

#যদি WHERETRACE_ENABLED /* 0x4001 */
  যদি( sqlite3WhereTrace & 0x4000 ){
    sqlite3DebugPrintf("All WHERE-clause terms after coding level %d:\n",
                       iLevel);
    sqlite3WhereClausePrint(pWC);
  }
  যদি( sqlite3WhereTrace & 0x1 ){
    sqlite3DebugPrintf("End Coding level %d:  notReady=%llx\n",
       iLevel, (u64)pLevel->notReady);
  }
#endif
  ফেরত pLevel->notReady;
}

/*
** Generate the code for the loop that finds all non-matched terms
** for a RIGHT JOIN.
*/
SQLITE_NOINLINE শূন্য sqlite3_Where_ডান_Join_Loop(
  WhereInfo *pWInfo,
  পূর্ণসংখ্যা iLevel,
  WhereLevel *pLevel
){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  Where_ডান_Join *pRJ = pLevel->pRJ;
  রাশিমালা *pSubWhere = 0;
  WhereClause *pWC = &pWInfo->sWC;
  WhereInfo *pSubWInfo;
  WhereLoop *pLoop = pLevel->pWLoop;
  SrcItem *pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
  SrcList *pFrom;
  সংঘ {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;
  Bitmask mAll = 0;
  পূর্ণসংখ্যা k;

  ExplainQueryPlan((pParse, 1, "RIGHT-JOIN %s", pTabItem->pSTab->zName));
  sqlite3VdbeNoJumpsOutsideSubrtn(v, pRJ->addrSubrtn, pRJ->endSubrtn,
                                  pRJ->regReturn);
  জন্য(k=0; k<iLevel; k++){
    পূর্ণসংখ্যা iIdxCur;
    SrcItem *p_ডান;
    প্রতিজ্ঞা( pWInfo->a[k].pWLoop->iTab == pWInfo->a[k].iFrom );
    p_ডান = &pWInfo->pTabList->a[pWInfo->a[k].iFrom];
    mAll |= pWInfo->a[k].pWLoop->maskSelf;
    যদি( p_ডান->fg.viaCoroutine ){
      Subquery *pSubq;
      প্রতিজ্ঞা( p_ডান->fg.isSubquery && p_ডান->u4.pSubq!=0 );
      pSubq = p_ডান->u4.pSubq;
      প্রতিজ্ঞা( pSubq->pSelect!=0 && pSubq->pSelect->pEList!=0 );
      sqlite3VdbeAddOp3(
          v, OP_Null, 0, pSubq->reg_ফলাফল,
          pSubq->reg_ফলাফল + pSubq->pSelect->pEList->n_রাশিমালা-1
      );
    }
    sqlite3VdbeAddOp1(v, OP_NullRow, pWInfo->a[k].iTabCur);
    iIdxCur = pWInfo->a[k].iIdxCur;
    যদি( iIdxCur ){
      sqlite3VdbeAddOp1(v, OP_NullRow, iIdxCur);
    }
  }
  যদি( (pTabItem->fg.jointype & JT_LTORJ)==0 ){
    mAll |= pLoop->maskSelf;
    জন্য(k=0; k<pWC->nTerm; k++){
      WhereTerm *pTerm = &pWC->a[k];
      যদি( (pTerm->wtFlags & (TERM_VIRTUAL|TERM_SLICE))!=0
       && pTerm->eOperator!=WO_ROWVAL
      ){
        বিরতি;
      }
      যদি( pTerm->prereqAll & ~mAll ) চালু;
      যদি( রাশিমালা_Has_Property(pTerm->p_রাশিমালা, EP_OuterON|EP_InnerON) ) চালু;
      pSubWhere = sqlite3_রাশিমালা_And(pParse, pSubWhere,
                                 sqlite3_রাশিমালা_Dup(pParse->db, pTerm->p_রাশিমালা, 0));
    }
  }
  যদি( pLevel->iIdxCur ){
    /* pSubWhere may contain expressions that read from an index on the
    ** table on the RHS of the right join. All such expressions first test
    ** if the index is pointing at a NULL row, and if so, read from the
    ** table cursor instead. So ensure that the index cursor really is 
    ** pointing at a NULL row here, so that no values are read from it during
    ** the scan of the RHS of the RIGHT join below.  */
    sqlite3VdbeAddOp1(v, OP_NullRow, pLevel->iIdxCur);
  }
  pFrom = &uSrc.sSrc;
  pFrom->nSrc = 1;
  pFrom->nAlloc = 1;
  memcpy(&pFrom->a[0], pTabItem, আকার(SrcItem));
  pFrom->a[0].fg.jointype = 0;
  যদি( pParse->withinRJSubrtn >= 100 ){
    /* This limit --------------^^^
    ** is based on an historical assert().  It is not compile-time or
    ** run-time configurable.  It could perhaps be raised as high as 254,
    ** but only an attack robot would ever do even 100 RIGHT JOINS within
    ** a single query, so we'll just leave it as it is. */
    sqlite3_Error_বার্তা(pParse, "too many RIGHT JOINs");
    ফেরত;
  }
  pParse->withinRJSubrtn++;
  pSubWInfo = sqlite3WhereBegin(pParse, pFrom, pSubWhere, 0, 0, 0,
                                WHERE_ডান_JOIN, 0);
  যদি( pSubWInfo ){
    পূর্ণসংখ্যা iCur = pLevel->iTabCur;  /* Table on RHS of RIGHT JOIN &*/
    পূর্ণসংখ্যা r = ++pParse->nMem;      /* Register range to hold primary key */
    পূর্ণসংখ্যা nPk;                     /* Number of values in the primary key */
    পূর্ণসংখ্যা r2;                      /* Register holding record for primary key */
    পূর্ণসংখ্যা addrCont = sqlite3WhereContinueLabel(pSubWInfo);
    Table *pTab = pTabItem->pSTab;  /* Table on RHS of RIGHT JOIN */
    যদি( HasRowid(pTab) ){
      sqlite3_রাশিমালা_Code_Get_Column_Of_Table(v, pTab, iCur, -1, r);
      nPk = 1;
    }অন্যথা{
      পূর্ণসংখ্যা iPk;
      সূচক *pPk = sqlite3_Primary_চাবি_সূচক(pTab);
      nPk = pPk->n_চাবি_Col;
      pParse->nMem += nPk - 1;
      জন্য(iPk=0; iPk<nPk; iPk++){
        পূর্ণসংখ্যা iCol = pPk->aiColumn[iPk];
        sqlite3_রাশিমালা_Code_Get_Column_Of_Table(v, pTab, iCur, iCol,r+iPk);
      }
    }

    /* Generate code that checks to see if the current row of the RHS table
    ** has appeared in any prior output row. */
    যদি( pRJ->regBloom ){
      sqlite3VdbeAddOp4Int(v, OP_Filter, pRJ->regBloom, 
                 sqlite3VdbeCurrentAddr(v)+2, r, nPk);
      VdbeCoverage(v);
    }
    sqlite3VdbeAddOp4Int(v, OP_Found, pRJ->iMatch, addrCont, r, nPk);
    VdbeCoverage(v);
    r2 = sqlite3_Get_অস্থায়ী_Reg(pParse);

    /* Generate code that inserts the PK of the RHS table into the
    ** pRH->iMatch index to indicate that the current row has appeared
    ** in the output set. */
    sqlite3VdbeAddOp3(v, OP_MakeRecord, r, nPk, r2);
    sqlite3VdbeAddOp4Int(v, OP_Idx_ঢোকাও, pRJ->iMatch, r2, r, nPk);
    sqlite3_Release_অস্থায়ী_Reg(pParse, r2);
    যদি( pRJ->regBloom ){
      sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pRJ->regBloom, 0, r, nPk);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
    }

    /* Invoke the subroutine that actually puts the current RHS table row
    ** into the output set, with NULLs for the LHS. */
    sqlite3VdbeAddOp2(v, OP_Gosub, pRJ->regReturn, pRJ->addrSubrtn);
    sqlite3WhereEnd(pSubWInfo);
  }
  sqlite3_রাশিমালা_Delete(pParse->db, pSubWhere);
  Explain_Query_Plan_বের_করো(pParse);
  প্রতিজ্ঞা( pParse->withinRJSubrtn>0 );
  pParse->withinRJSubrtn--;
}
