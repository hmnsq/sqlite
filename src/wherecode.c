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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "whereInt.h"

#ifndef SQLITE_OMIT_EXPLAIN

/*
** Return the name of the i-th column of the pIdx index.
*/
સ્થિર અચલ અક્ષર *explainIndexColumnName(Index *pIdx, પૂર્ણાંક i){
  i = pIdx->aiColumn[i];
  જો( i==XN_પદાવલિ ) પાછા "<expr>";
  જો( i==XN_ROWID ) પાછા "rowid";
  પાછા pIdx->pTable->aCol[i].zCnName;
}

/*
** This routine is a helper for explainIndexRange() below
**
** pStr holds the text of an expression that we are building up one term
** at a time.  This routine adds a new term to the end of the expression.
** Terms are separated by AND so add the "AND" text for second and subsequent
** terms only.
*/
સ્થિર શૂન્ય explainAppendTerm(
  StrAccum *pStr,             /* The text expression being built */
  Index *pIdx,                /* Index to read column names from */
  પૂર્ણાંક nTerm,                  /* Number of terms */
  પૂર્ણાંક iTerm,                  /* Zero-based index of first term. */
  પૂર્ણાંક bAnd,                   /* Non-zero to append " AND " */
  અચલ અક્ષર *zOp             /* Name of the operator */
){
  પૂર્ણાંક i;

  પ્રતિજ્ઞા( nTerm>=1 );
  જો( bAnd ) sqlite3_str_append(pStr, " AND ", 5);

  જો( nTerm>1 ) sqlite3_str_append(pStr, "(", 1);
  માટે(i=0; i<nTerm; i++){
    જો( i ) sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_appendall(pStr, explainIndexColumnName(pIdx, iTerm+i));
  }
  જો( nTerm>1 ) sqlite3_str_append(pStr, ")", 1);

  sqlite3_str_append(pStr, zOp, 1);

  જો( nTerm>1 ) sqlite3_str_append(pStr, "(", 1);
  માટે(i=0; i<nTerm; i++){
    જો( i ) sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_append(pStr, "?", 1);
  }
  જો( nTerm>1 ) sqlite3_str_append(pStr, ")", 1);
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
સ્થિર શૂન્ય explainIndexRange(StrAccum *pStr, WhereLoop *pLoop){
  Index *pIndex = pLoop->u.btree.pIndex;
  u16 nEq = pLoop->u.btree.nEq;
  u16 nSkip = pLoop->nSkip;
  પૂર્ણાંક i, j;

  જો( nEq==0 && (pLoop->wsFlags&(WHERE_BTM_LIMIT|WHERE_ટોચ_LIMIT))==0 ) પાછા;
  sqlite3_str_append(pStr, " (", 2);
  માટે(i=0; i<nEq; i++){
    અચલ અક્ષર *z = explainIndexColumnName(pIndex, i);
    જો( i ) sqlite3_str_append(pStr, " AND ", 5);
    sqlite3_str_appendf(pStr, i>=nSkip ? "%s=?" : "ANY(%s)", z);
  }

  j = i;
  જો( pLoop->wsFlags&WHERE_BTM_LIMIT ){
    explainAppendTerm(pStr, pIndex, pLoop->u.btree.nBtm, j, i, ">");
    i = 1;
  }
  જો( pLoop->wsFlags&WHERE_ટોચ_LIMIT ){
    explainAppendTerm(pStr, pIndex, pLoop->u.btree.n_ટોચ, j, i, "<");
  }
  sqlite3_str_append(pStr, ")", 1);
}

/*
** This function sets the P4 value of an existing OP_Explain opcode to
** text describing the loop in pLevel. If the OP_Explain opcode already has
** a P4 value, it is freed before it is overwritten.
*/
શૂન્ય sqlite3WhereAddExplainText(
  Parse *pParse,                  /* Parse context */
  પૂર્ણાંક addr,                       /* Address of OP_Explain opcode */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
#જો !defined(SQLITE_DEBUG)
  જો( sqlite3ParseToplevel(pParse)->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#endif
  {
    VdbeOp *pOp = sqlite3VdbeGetOp(pParse->pVdbe, addr);
    SrcItem *pItem = &pTabList->a[pLevel->iFrom];
    sqlite3 *db = pParse->db;     /* Database handle */
    પૂર્ણાંક is_શોધ;                 /* True for a SEARCH. False for SCAN. */
    WhereLoop *pLoop;             /* The controlling WhereLoop object */
    u32 flags;                    /* Flags that describe this loop */
#જો defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_EXPLAIN)
    અક્ષર *z_સંદેશ;                   /* Text to add to EQP output */
#endif
    StrAccum str;                 /* EQP output string */
    અક્ષર zBuf[100];               /* Initial space for EQP output string */

    જો( db->mallocFailed ) પાછા;

    pLoop = pLevel->pWLoop;
    flags = pLoop->wsFlags;

    is_શોધ = (flags&(WHERE_BTM_LIMIT|WHERE_ટોચ_LIMIT))!=0
            || ((flags&WHERE_VIRTUALTABLE)==0 && (pLoop->u.btree.nEq>0))
            || (wctrlFlags&(WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX));

    sqlite3StrAccumInit(&str, db, zBuf, કદ(zBuf), SQLITE_MAX_LENGTH);
    str.printfFlags = SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&str, "%s %S%s",
       is_શોધ ? "SEARCH" : "SCAN",
       pItem,
       pItem->fg.fromExists ? " EXISTS" : "");
    જો( (flags & (WHERE_IPK|WHERE_VIRTUALTABLE))==0 ){
      અચલ અક્ષર *zFmt = 0;
      Index *pIdx;

      પ્રતિજ્ઞા( pLoop->u.btree.pIndex!=0 );
      pIdx = pLoop->u.btree.pIndex;
      પ્રતિજ્ઞા( !(flags&WHERE_AUTO_INDEX) || (flags&WHERE_IDX_ONLY) );
      જો( !HasRowid(pItem->pSTab) && Is_Primary_ચાવી_Index(pIdx) ){
        જો( is_શોધ ){
          zFmt = "PRIMARY KEY";
        }
      }નહીંતર જો( flags & WHERE_PARTIALIDX ){
        zFmt = "AUTOMATIC PARTIAL COVERING INDEX";
      }નહીંતર જો( flags & WHERE_AUTO_INDEX ){
        zFmt = "AUTOMATIC COVERING INDEX";
      }નહીંતર જો( flags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
        zFmt = "COVERING INDEX %s";
      }નહીંતર{
        zFmt = "INDEX %s";
      }
      જો( zFmt ){
        sqlite3_str_append(&str, " USING ", 7);
        sqlite3_str_appendf(&str, zFmt, pIdx->zName);
        explainIndexRange(&str, pLoop);
      }
    }નહીંતર જો( (flags & WHERE_IPK)!=0 && (flags & WHERE_CONSTRAINT)!=0 ){
      અક્ષર cRangeOp;
#જો 0  /* Better output, but breaks many tests */
      અચલ Table *pTab = pItem->pTab;
      અચલ અક્ષર *zRowid = pTab->iPKey>=0 ? pTab->aCol[pTab->iPKey].zCnName:
                              "rowid";
#નહીંતર
      અચલ અક્ષર *zRowid = "rowid";
#endif
      sqlite3_str_appendf(&str, " USING INTEGER PRIMARY KEY (%s", zRowid);
      જો( flags&(WHERE_COLUMN_EQ|WHERE_COLUMN_IN) ){
        cRangeOp = '=';
      }નહીંતર જો( (flags&WHERE_BOTH_LIMIT)==WHERE_BOTH_LIMIT ){
        sqlite3_str_appendf(&str, ">? AND %s", zRowid);
        cRangeOp = '<';
      }નહીંતર જો( flags&WHERE_BTM_LIMIT ){
        cRangeOp = '>';
      }નહીંતર{
        પ્રતિજ્ઞા( flags&WHERE_ટોચ_LIMIT);
        cRangeOp = '<';
      }
      sqlite3_str_appendf(&str, "%c?)", cRangeOp);
    }
#ifndef SQLITE_OMIT_VIRTUALTABLE
    નહીંતર જો( (flags & WHERE_VIRTUALTABLE)!=0 ){
      sqlite3_str_appendall(&str, " VIRTUAL TABLE INDEX ");
      sqlite3_str_appendf(&str,
                  pLoop->u.vtab.b_Idx_અંક_Hex ? "0x%x:%s" : "%d:%s",
                  pLoop->u.vtab.idx_અંક, pLoop->u.vtab.idxStr);
    }
#endif
    જો( pItem->fg.jointype & JT_ડાબું ){
      sqlite3_str_appendf(&str, " LEFT-JOIN");
    }
#ifdef SQLITE_EXPLAIN_ESTIMATED_ROWS
    જો( pLoop->nOut>=10 ){
      sqlite3_str_appendf(&str, " (~%llu rows)",
             sqlite3LogEstToInt(pLoop->nOut));
    }નહીંતર{
      sqlite3_str_append(&str, " (~1 row)", 9);
    }
#endif
#જો defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_EXPLAIN)
    z_સંદેશ = sqlite3StrAccumFinish(&str);
    sqlite3ExplainBreakpoint("",z_સંદેશ);
#endif

    પ્રતિજ્ઞા( pOp->opcode==OP_Explain );
    પ્રતિજ્ઞા( pOp->p4type==P4_DYNAMIC || pOp->p4.z==0 );
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
પૂર્ણાંક sqlite3WhereExplainOneScan(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
  પૂર્ણાંક ret = 0;
#જો !defined(SQLITE_DEBUG)
  જો( sqlite3ParseToplevel(pParse)->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#endif
  {
    જો( (pLevel->pWLoop->wsFlags & WHERE_MULTI_OR)==0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
    ){
      Vdbe *v = pParse->pVdbe;
      પૂર્ણાંક addr = sqlite3VdbeCurrentAddr(v);
      ret = sqlite3VdbeAddOp3(
          v, OP_Explain, addr, pParse->addrExplain, pLevel->pWLoop->rRun
      );
      sqlite3WhereAddExplainText(pParse, addr, pTabList, pLevel, wctrlFlags);
    }
  }
  પાછા ret;
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
પૂર્ણાંક sqlite3WhereExplainBloomFilter(
  અચલ Parse *pParse,               /* Parse context */
  અચલ WhereInfo *pWInfo,           /* WHERE clause */
  અચલ WhereLevel *pLevel           /* Bloom filter on this level */
){
  પૂર્ણાંક ret = 0;
  SrcItem *pItem = &pWInfo->pTabList->a[pLevel->iFrom];
  Vdbe *v = pParse->pVdbe;      /* VM being constructed */
  sqlite3 *db = pParse->db;     /* Database handle */
  અક્ષર *z_સંદેશ;                   /* Text to add to EQP output */
  પૂર્ણાંક i;                        /* Loop counter */
  WhereLoop *pLoop;             /* The where loop */
  StrAccum str;                 /* EQP output string */
  અક્ષર zBuf[100];               /* Initial space for EQP output string */

  sqlite3StrAccumInit(&str, db, zBuf, કદ(zBuf), SQLITE_MAX_LENGTH);
  str.printfFlags = SQLITE_PRINTF_INTERNAL;
  sqlite3_str_appendf(&str, "BLOOM FILTER ON %S (", pItem);
  pLoop = pLevel->pWLoop;
  જો( pLoop->wsFlags & WHERE_IPK ){
    અચલ Table *pTab = pItem->pSTab;
    જો( pTab->iPKey>=0 ){
      sqlite3_str_appendf(&str, "%s=?", pTab->aCol[pTab->iPKey].zCnName);
    }નહીંતર{
      sqlite3_str_appendf(&str, "rowid=?");
    }
  }નહીંતર{
    માટે(i=pLoop->nSkip; i<pLoop->u.btree.nEq; i++){
      અચલ અક્ષર *z = explainIndexColumnName(pLoop->u.btree.pIndex, i);
      જો( i>pLoop->nSkip ) sqlite3_str_append(&str, " AND ", 5);
      sqlite3_str_appendf(&str, "%s=?", z);
    }
  }
  sqlite3_str_append(&str, ")", 1);
  z_સંદેશ = sqlite3StrAccumFinish(&str);
  ret = sqlite3VdbeAddOp4(v, OP_Explain, sqlite3VdbeCurrentAddr(v),
                          pParse->addrExplain, 0, z_સંદેશ,P4_DYNAMIC);

  sqlite3VdbeScanStatus(v, sqlite3VdbeCurrentAddr(v)-1, 0, 0, 0, 0);
  પાછા ret;
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
શૂન્ય sqlite3WhereAddScanStatus(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  પૂર્ણાંક addrExplain                 /* Address of OP_Explain (or 0) */
){
  જો( IS_STMT_SCANSTATUS( sqlite3VdbeDb(v) ) ){
    અચલ અક્ષર *zObj = 0;
    WhereLoop *pLoop = pLvl->pWLoop;
    પૂર્ણાંક wsFlags = pLoop->wsFlags;
    પૂર્ણાંક viaCoroutine = 0;

    જો( (wsFlags & WHERE_VIRTUALTABLE)==0  &&  pLoop->u.btree.pIndex!=0 ){
      zObj = pLoop->u.btree.pIndex->zName;
    }નહીંતર{
      zObj = pSrclist->a[pLvl->iFrom].zName;
      viaCoroutine = pSrclist->a[pLvl->iFrom].fg.viaCoroutine;
    }
    sqlite3VdbeScanStatus(
        v, addrExplain, pLvl->addrBody, pLvl->addrVisit, pLoop->nOut, zObj
    );

    જો( viaCoroutine==0 ){
      જો( (wsFlags & (WHERE_MULTI_OR|WHERE_AUTO_INDEX))==0 ){
        sqlite3VdbeScanStatusRange(v, addrExplain, -1, pLvl->iTabCur);
      }
      જો( wsFlags & WHERE_INDEXED ){
        sqlite3VdbeScanStatusRange(v, addrExplain, -1, pLvl->iIdxCur);
      }
    }નહીંતર{
      પૂર્ણાંક addr;
      VdbeOp *pOp;
      પ્રતિજ્ઞા( pSrclist->a[pLvl->iFrom].fg.isSubquery );
      addr = pSrclist->a[pLvl->iFrom].u4.pSubq->addrFillSub;
      pOp = sqlite3VdbeGetOp(v, addr-1);
      પ્રતિજ્ઞા( sqlite3VdbeDb(v)->mallocFailed || pOp->opcode==OP_InitCoroutine );
      પ્રતિજ્ઞા( sqlite3VdbeDb(v)->mallocFailed || pOp->p2>addr );
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
સ્થિર શૂન્ય disableTerm(WhereLevel *pLevel, WhereTerm *pTerm){
  પૂર્ણાંક nLoop = 0;
  પ્રતિજ્ઞા( pTerm!=0 );
  જ્યારે( (pTerm->wtFlags & TERM_CODED)==0
      && (pLevel->i_ડાબું_Join==0 || પદાવલિ_Has_Property(pTerm->p_પદાવલિ, EP_OuterON))
      && (pLevel->notReady & pTerm->prereqAll)==0
  ){
    જો( nLoop && (pTerm->wtFlags & TERM_LIKE)!=0 ){
      pTerm->wtFlags |= TERM_LIKECOND;
    }નહીંતર{
      pTerm->wtFlags |= TERM_CODED;
    }
#ifdef WHERETRACE_ENABLED
    જો( (sqlite3WhereTrace & 0x4001)==0x4001 ){
      sqlite3DebugPrintf("DISABLE-");
      sqlite3WhereTermPrint(pTerm, (પૂર્ણાંક)(pTerm - (pTerm->pWC->a)));
    }
#endif
    જો( pTerm->iParent<0 ) તોડો;
    pTerm = &pTerm->pWC->a[pTerm->iParent];
    પ્રતિજ્ઞા( pTerm!=0 );
    pTerm->nChild--;
    જો( pTerm->nChild!=0 ) તોડો;
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
સ્થિર શૂન્ય codeApplyAffinity(Parse *pParse, પૂર્ણાંક base, પૂર્ણાંક n, અક્ષર *zAff){
  Vdbe *v = pParse->pVdbe;
  જો( zAff==0 ){
    પ્રતિજ્ઞા( pParse->db->mallocFailed );
    પાછા;
  }
  પ્રતિજ્ઞા( v!=0 );

  /* Adjust base and n to skip over SQLITE_AFF_BLOB and SQLITE_AFF_NONE
  ** entries at the beginning and end of the affinity string.
  */
  પ્રતિજ્ઞા( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  જ્યારે( n>0 && zAff[0]<=SQLITE_AFF_BLOB ){
    n--;
    base++;
    zAff++;
  }
  જ્યારે( n>1 && zAff[n-1]<=SQLITE_AFF_BLOB ){
    n--;
  }

  /* Code the OP_Affinity opcode if there is anything left to do. */
  જો( n>0 ){
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
સ્થિર શૂન્ય updateRangeAffinityStr(
  પદાવલિ *p_જમણું,                   /* RHS of comparison */
  પૂર્ણાંક n,                          /* Number of vector elements in comparison */
  અક્ષર *zAff                      /* Affinity string to modify */
){
  પૂર્ણાંક i;
  માટે(i=0; i<n; i++){
    પદાવલિ *p = sqlite3VectorFieldSubexpr(p_જમણું, i);
    જો( sqlite3CompareAffinity(p, zAff[i])==SQLITE_AFF_BLOB
     || sqlite3_પદાવલિ_Needs_No_Affinity_Change(p, zAff[i])
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
સ્થિર શૂન્ય adjustOrderByCol(પદાવલિ_List *pOrderBy, પદાવલિ_List *pEList){
  પૂર્ણાંક i, j;
  જો( pOrderBy==0 ) પાછા;
  માટે(i=0; i<pOrderBy->n_પદાવલિ; i++){
    પૂર્ણાંક t = pOrderBy->a[i].u.x.iOrderByCol;
    જો( t==0 ) ચાલુ;
    માટે(j=0; j<pEList->n_પદાવલિ; j++){
      જો( pEList->a[j].u.x.iOrderByCol==t ){
        pOrderBy->a[i].u.x.iOrderByCol = j+1;
        તોડો;
      }
    }
    જો( j>=pEList->n_પદાવલિ ){
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
સ્થિર પદાવલિ *removeUnindexableInClauseTerms(
  Parse *pParse,        /* The parsing context */
  પૂર્ણાંક iEq,              /* Look at loop terms starting here */
  WhereLoop *pLoop,     /* The current loop */
  પદાવલિ *pX              /* The IN expression to be reduced */
){
  sqlite3 *db = pParse->db;
  Select *pSelect;            /* Pointer to the SELECT on the RHS */
  પદાવલિ *pNew;
  pNew = sqlite3_પદાવલિ_Dup(db, pX, 0);
  જો( db->mallocFailed==0 ){
    માટે(pSelect=pNew->x.pSelect; pSelect; pSelect=pSelect->pPrior){
      પદાવલિ_List *pOrigRhs;         /* Original unmodified RHS */
      પદાવલિ_List *pOrigLhs = 0;     /* Original unmodified LHS */
      પદાવલિ_List *pRhs = 0;         /* New RHS after modifications */
      પદાવલિ_List *pLhs = 0;         /* New LHS after mods */
      પૂર્ણાંક i;                      /* Loop counter */
      પૂર્ણાંક nRhs = 0;               /* Number of RHS terms added so far */

      પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(pNew) );
      pOrigRhs = pSelect->pEList;
      પ્રતિજ્ઞા( pNew->p_ડાબું!=0 );
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(pNew->p_ડાબું) );
      જો( pSelect==pNew->x.pSelect ){
        pOrigLhs = pNew->p_ડાબું->x.pList;
      }
      માટે(i=iEq; i<pLoop->nLTerm; i++){
        જો( pLoop->aLTerm[i]->p_પદાવલિ==pX ){
          પૂર્ણાંક iField;
          પ્રતિજ્ઞા( (pLoop->aLTerm[i]->eOperator & (WO_OR|WO_AND))==0 );
          iField = pLoop->aLTerm[i]->u.x.iField - 1;
          જો( iField!=nRhs ){
            પદાવલિ_Clear_Property(pNew, EP_Subrtn);
          }
          જો( NEVER(pOrigRhs->a[iField].p_પદાવલિ==0) ){
            ચાલુ; /* Duplicate PK column */
          }
          pRhs = sqlite3_પદાવલિ_List_Append(pParse, pRhs, pOrigRhs->a[iField].p_પદાવલિ);
          pOrigRhs->a[iField].p_પદાવલિ = 0;
          જો( pRhs ) pRhs->a[pRhs->n_પદાવલિ-1].u.x.iOrderByCol = iField+1;
          જો( pOrigLhs ){
            પ્રતિજ્ઞા( pOrigLhs->a[iField].p_પદાવલિ!=0 );
            pLhs = sqlite3_પદાવલિ_List_Append(pParse,pLhs,pOrigLhs->a[iField].p_પદાવલિ);
            pOrigLhs->a[iField].p_પદાવલિ = 0;
          }
          nRhs++;
        }
      }
      જો( nRhs<pOrigRhs->n_પદાવલિ ){
        પદાવલિ_Clear_Property(pNew, EP_Subrtn); /* See bug 2026-10-16T22:07:15Z */
      }
      sqlite3_પદાવલિ_List_Delete(db, pOrigRhs);
      જો( pOrigLhs ){
        sqlite3_પદાવલિ_List_Delete(db, pOrigLhs);
        pNew->p_ડાબું->x.pList = pLhs;
      }
      pSelect->pEList = pRhs;
      pSelect->selId = ++pParse->nSelect; /* Req'd for SubrtnSig validity */
      જો( pLhs && pLhs->n_પદાવલિ==1 ){
        /* Take care here not to generate a TK_VECTOR containing only a
        ** single value. Since the parser never creates such a vector, some
        ** of the subroutines do not handle this case.  */
        પદાવલિ *p = pLhs->a[0].p_પદાવલિ;
        pLhs->a[0].p_પદાવલિ = 0;
        sqlite3_પદાવલિ_Delete(db, pNew->p_ડાબું);
        pNew->p_ડાબું = p;
      }

      /* If either the ORDER BY clause or the GROUP BY clause contains
      ** references to result-set columns, those references might now be
      ** obsolete.  So fix them up.
      */
      પ્રતિજ્ઞા( pRhs!=0 || db->mallocFailed );
      જો( pRhs ){
        adjustOrderByCol(pSelect->pOrderBy, pRhs);
        adjustOrderByCol(pSelect->pGroupBy, pRhs);
        માટે(i=0; i<pRhs->n_પદાવલિ; i++) pRhs->a[i].u.x.iOrderByCol = 0;
      }

#જો 0
      printf("For indexing, change the IN expr:\n");
      sqlite3_વૃક્ષ_View_પદાવલિ(0, pX, 0);
      printf("Into:\n");
      sqlite3_વૃક્ષ_View_પદાવલિ(0, pNew, 0);
#endif
    }
  }
  પાછા pNew;
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
સ્થિર SQLITE_NOINLINE શૂન્ય codeINTerm(
  Parse *pParse,      /* The parsing context */
  WhereTerm *pTerm,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  પૂર્ણાંક iEq,            /* Index of the equality term within this level */
  પૂર્ણાંક bRev,           /* True for reverse-order IN operations */
  પૂર્ણાંક iTarget         /* Attempt to leave results in this register */
){
  પદાવલિ *pX = pTerm->p_પદાવલિ;
  પૂર્ણાંક eType = IN_INDEX_NOOP;
  પૂર્ણાંક iTab;
  સંરચના InLoop *pIn;
  WhereLoop *pLoop = pLevel->pWLoop;
  Vdbe *v = pParse->pVdbe;
  પૂર્ણાંક i;
  પૂર્ણાંક nEq = 0;
  પૂર્ણાંક *aiMap = 0;

  જો( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
    && pLoop->u.btree.pIndex!=0
    && pLoop->u.btree.pIndex->aSortOrder[iEq]
  ){
    testcase( iEq==0 );
    testcase( bRev );
    bRev = !bRev;
  }
  પ્રતિજ્ઞા( pX->op==TK_IN );

  માટે(i=0; i<iEq; i++){
    જો( pLoop->aLTerm[i] && pLoop->aLTerm[i]->p_પદાવલિ==pX ){
      disableTerm(pLevel, pTerm);
      પાછા;
    }
  }
  માટે(i=iEq; i<pLoop->nLTerm; i++){
    પ્રતિજ્ઞા( pLoop->aLTerm[i]!=0 );
    જો( pLoop->aLTerm[i]->p_પદાવલિ==pX ) nEq++;
  }

  iTab = 0;
  જો( !પદાવલિ_Use_XSelect(pX) || pX->x.pSelect->pEList->n_પદાવલિ==1 ){
    eType = sqlite3FindInIndex(pParse, pX, IN_INDEX_LOOP, 0, 0, &iTab);
  }નહીંતર{
    sqlite3 *db = pParse->db;
    પદાવલિ *pXMod = removeUnindexableInClauseTerms(pParse, iEq, pLoop, pX);
    જો( nEq>1 ){
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
    જો( !db->mallocFailed ){
      પૂર્ણાંક nCol = pX->x.pSelect->pEList->n_પદાવલિ;
      aiMap = (પૂર્ણાંક*)sqlite3DbMallocZero(db, કદ(પૂર્ણાંક)*nCol);
      eType = sqlite3FindInIndex(pParse, pXMod, IN_INDEX_LOOP, 0, aiMap, &iTab);
    }
    sqlite3_પદાવલિ_Delete(db, pXMod);
  }

  જો( eType==IN_INDEX_INDEX_DESC ){
    testcase( bRev );
    bRev = !bRev;
  }
  sqlite3VdbeAddOp2(v, bRev ? OP_Last : OP_Rewind, iTab, 0);
  VdbeCoverageIf(v, bRev);
  VdbeCoverageIf(v, !bRev);

  પ્રતિજ્ઞા( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );
  pLoop->wsFlags |= WHERE_IN_ABLE;
  જો( pLevel->u.in.nIn==0 ){
    pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
  }
  જો( iEq>0 && (pLoop->wsFlags & WHERE_IN_SEEKSCAN)==0 ){
    pLoop->wsFlags |= WHERE_IN_EARLYOUT;
  }

  i = pLevel->u.in.nIn;
  pLevel->u.in.nIn += nEq;
  pLevel->u.in.aInLoop =
     sqlite3WhereRealloc(pTerm->pWC->pWInfo,
                         pLevel->u.in.aInLoop,
                         કદ(pLevel->u.in.aInLoop[0])*pLevel->u.in.nIn);
  pIn = pLevel->u.in.aInLoop;
  જો( pIn ){
    પૂર્ણાંક iMap = 0;               /* Index in aiMap[] */
    pIn += i;
    માટે(i=iEq; i<pLoop->nLTerm; i++){
      જો( pLoop->aLTerm[i]->p_પદાવલિ==pX ){
        પૂર્ણાંક iOut = iTarget + i - iEq;
        જો( eType==IN_INDEX_ROWID ){
          pIn->addr_In_ટોચ = sqlite3VdbeAddOp2(v, OP_Rowid, iTab, iOut);
        }નહીંતર{
          પૂર્ણાંક iCol = aiMap ? aiMap[iMap++] : 0;
          pIn->addr_In_ટોચ = sqlite3VdbeAddOp3(v,OP_Column,iTab, iCol, iOut);
        }
        sqlite3VdbeAddOp1(v, OP_IsNull, iOut); VdbeCoverage(v);
        જો( i==iEq ){
          pIn->iCur = iTab;
          pIn->eEndLoopOp = bRev ? OP_Prev : OP_Next;
          જો( iEq>0 ){
            pIn->iBase = iTarget - i;
            pIn->nPrefix = i;
          }નહીંતર{
            pIn->nPrefix = 0;
          }
        }નહીંતર{
          pIn->eEndLoopOp = OP_Noop;
        }
        pIn++;
      }
    }
    testcase( iEq>0
              && (pLoop->wsFlags & WHERE_IN_SEEKSCAN)==0
              && (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 );
    જો( iEq>0
     && (pLoop->wsFlags & (WHERE_IN_SEEKSCAN|WHERE_VIRTUALTABLE))==0
    ){
      sqlite3VdbeAddOp3(v, OP_SeekHit, pLevel->iIdxCur, 0, iEq);
    }
  }નહીંતર{
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
સ્થિર પૂર્ણાંક codeEqualityTerm(
  Parse *pParse,      /* The parsing context */
  WhereTerm *pTerm,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  પૂર્ણાંક iEq,            /* Index of the equality term within this level */
  પૂર્ણાંક bRev,           /* True for reverse-order IN operations */
  પૂર્ણાંક iTarget         /* Attempt to leave results in this register */
){
  પદાવલિ *pX = pTerm->p_પદાવલિ;
  પૂર્ણાંક iReg;                  /* Register holding results */

  પ્રતિજ્ઞા( pLevel->pWLoop->aLTerm[iEq]==pTerm );
  પ્રતિજ્ઞા( iTarget>0 );
  જો( pX->op==TK_EQ || pX->op==TK_IS ){
    iReg = sqlite3_પદાવલિ_Code_Target(pParse, pX->p_જમણું, iTarget);
  }નહીંતર જો( pX->op==TK_ISNULL ){
    iReg = iTarget;
    sqlite3VdbeAddOp2(pParse->pVdbe, OP_Null, 0, iReg);
#ifndef SQLITE_OMIT_SUBQUERY
  }નહીંતર{
    પ્રતિજ્ઞા( pX->op==TK_IN );
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
  જો( (pLevel->pWLoop->wsFlags & WHERE_TRANSCONS)==0
   || (pTerm->eOperator & WO_EQUIV)==0
  ){
    disableTerm(pLevel, pTerm);
  }

  પાછા iReg;
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
સ્થિર પૂર્ણાંક codeAllEqualityTerms(
  Parse *pParse,        /* Parsing context */
  WhereLevel *pLevel,   /* Which nested loop of the FROM we are coding */
  પૂર્ણાંક bRev,             /* Reverse the order of IN operators */
  પૂર્ણાંક nExtraReg,        /* Number of extra registers to allocate */
  અક્ષર **pzAff          /* OUT: Set to point to affinity string */
){
  u16 nEq;                      /* The number of == or IN constraints to code */
  u16 nSkip;                    /* Number of left-most columns to skip */
  Vdbe *v = pParse->pVdbe;      /* The vm under construction */
  Index *pIdx;                  /* The index being used for this loop */
  WhereTerm *pTerm;             /* A single constraint term */
  WhereLoop *pLoop;             /* The WhereLoop object */
  પૂર્ણાંક j;                        /* Loop counter */
  પૂર્ણાંક regBase;                  /* Base register */
  પૂર્ણાંક nReg;                     /* Number of registers to allocate */
  અક્ષર *zAff;                   /* Affinity string to return */

  /* This module is only called on query plans that use an index. */
  pLoop = pLevel->pWLoop;
  પ્રતિજ્ઞા( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0 );
  nEq = pLoop->u.btree.nEq;
  nSkip = pLoop->nSkip;
  pIdx = pLoop->u.btree.pIndex;
  પ્રતિજ્ઞા( pIdx!=0 );

  /* Figure out how many memory cells we will need then allocate them.
  */
  regBase = pParse->nMem + 1;
  nReg = nEq + nExtraReg;
  pParse->nMem += nReg;

  zAff = sqlite3DbStrDup(pParse->db,sqlite3IndexAffinityStr(pParse->db,pIdx));
  પ્રતિજ્ઞા( zAff!=0 || pParse->db->mallocFailed );

  જો( nSkip ){
    પૂર્ણાંક iIdxCur = pLevel->iIdxCur;
    sqlite3VdbeAddOp3(v, OP_Null, 0, regBase, regBase+nSkip-1);
    sqlite3VdbeAddOp1(v, (bRev?OP_Last:OP_Rewind), iIdxCur);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    VdbeComment((v, "begin skip-scan on %s", pIdx->zName));
    j = sqlite3VdbeAddOp0(v, OP_Goto);
    પ્રતિજ્ઞા( pLevel->addrSkip==0 );
    pLevel->addrSkip = sqlite3VdbeAddOp4Int(v, (bRev?OP_SeekLT:OP_SeekGT),
                            iIdxCur, 0, regBase, nSkip);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    sqlite3VdbeJumpHere(v, j);
    માટે(j=0; j<nSkip; j++){
      sqlite3VdbeAddOp3(v, OP_Column, iIdxCur, j, regBase+j);
      testcase( pIdx->aiColumn[j]==XN_પદાવલિ );
      VdbeComment((v, "%s", explainIndexColumnName(pIdx, j)));
    }
  }

  /* Evaluate the equality constraints
  */
  પ્રતિજ્ઞા( zAff==0 || (પૂર્ણાંક)strlen(zAff)>=nEq );
  માટે(j=nSkip; j<nEq; j++){
    પૂર્ણાંક r1;
    pTerm = pLoop->aLTerm[j];
    પ્રતિજ્ઞા( pTerm!=0 );
    /* The following testcase is true for indices with redundant columns.
    ** Ex: CREATE INDEX i1 ON t1(a,b,a); SELECT * FROM t1 WHERE a=0 AND b=0; */
    testcase( (pTerm->wtFlags & TERM_CODED)!=0 );
    testcase( pTerm->wtFlags & TERM_VIRTUAL );
    r1 = codeEqualityTerm(pParse, pTerm, pLevel, j, bRev, regBase+j);
    જો( r1!=regBase+j ){
      sqlite3VdbeAddOp2(v, OP_Copy, r1, regBase+j);
    }
    જો( pTerm->eOperator & WO_IN ){
      જો( pTerm->p_પદાવલિ->flags & EP_xIsSelect ){
        /* No affinity ever needs to be (or should be) applied to a value
        ** from the RHS of an "? IN (SELECT ...)" expression. The
        ** sqlite3FindInIndex() routine has already ensured that the
        ** affinity of the comparison has been applied to the value.  */
        જો( zAff ) zAff[j] = SQLITE_AFF_BLOB;
      }
    }નહીંતર જો( (pTerm->eOperator & WO_ISNULL)==0 ){
      પદાવલિ *p_જમણું = pTerm->p_પદાવલિ->p_જમણું;
      જો( (pTerm->wtFlags & TERM_IS)==0 && sqlite3_પદાવલિ_Can_Be_Null(p_જમણું) ){
        sqlite3VdbeAddOp2(v, OP_IsNull, regBase+j, pLevel->addrBrk);
        VdbeCoverage(v);
      }
      જો( pParse->nErr==0 ){
        પ્રતિજ્ઞા( pParse->db->mallocFailed==0 );
        જો( sqlite3CompareAffinity(p_જમણું, zAff[j])==SQLITE_AFF_BLOB ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
        જો( sqlite3_પદાવલિ_Needs_No_Affinity_Change(p_જમણું, zAff[j]) ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
      }
    }
  }
  *pzAff = zAff;
  પાછા regBase;
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
સ્થિર શૂન્ય whereLikeOptimizationStringFixup(
  Vdbe *v,                /* prepared statement under construction */
  WhereLevel *pLevel,     /* The loop that contains the LIKE operator */
  WhereTerm *pTerm        /* The upper or lower bound just coded */
){
  જો( pTerm->wtFlags & TERM_LIKEOPT ){
    VdbeOp *pOp;
    પ્રતિજ્ઞા( pLevel->iLikeRepCntr>0 );
    pOp = sqlite3VdbeGetLastOp(v);
    પ્રતિજ્ઞા( pOp!=0 );
    પ્રતિજ્ઞા( pOp->opcode==OP_String8
            || pTerm->pWC->pWInfo->pParse->db->mallocFailed );
    pOp->p3 = (પૂર્ણાંક)(pLevel->iLikeRepCntr>>1);  /* Register holding counter */
    pOp->p5 = (u8)(pLevel->iLikeRepCntr&1);    /* ASC or DESC */
  }
}
#નહીંતર
# define whereLikeOptimizationStringFixup(A,B,C)
#endif

#ifdef SQLITE_ENABLE_CURSOR_HINTS
/*
** Information is passed from codeCursorHint() down to individual nodes of
** the expression tree (by sqlite3WalkExpr()) using an instance of this
** structure.
*/
સંરચના CCurHint {
  પૂર્ણાંક iTabCur;    /* Cursor for the main table */
  પૂર્ણાંક iIdxCur;    /* Cursor for the index, if pIdx!=0.  Unused otherwise */
  Index *pIdx;    /* The index used to access the table */
};

/*
** This function is called for every node of an expression that is a candidate
** for a cursor hint on an index cursor.  For TK_COLUMN nodes that reference
** the table CCurHint.iTabCur, verify that the same column can be
** accessed through the index.  If it cannot, then set pWalker->eCode to 1.
*/
સ્થિર પૂર્ણાંક code_Cursor_Hint_Check_પદાવલિ(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  સંરચના CCurHint *pHint = pWalker->u.pCCurHint;
  પ્રતિજ્ઞા( pHint->pIdx!=0 );
  જો( p_પદાવલિ->op==TK_COLUMN
   && p_પદાવલિ->iTable==pHint->iTabCur
   && sqlite3TableColumnToIndex(pHint->pIdx, p_પદાવલિ->iColumn)<0
  ){
    pWalker->eCode = 1;
  }
  પાછા WRC_Continue;
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
સ્થિર પૂર્ણાંક codeCursorHintIsOrFunction(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  જો( p_પદાવલિ->op==TK_IS
   || p_પદાવલિ->op==TK_ISNULL || p_પદાવલિ->op==TK_ISNOT
   || p_પદાવલિ->op==TK_NOTNULL || p_પદાવલિ->op==TK_CASE
  ){
    pWalker->eCode = 1;
  }નહીંતર જો( p_પદાવલિ->op==TK_FUNCTION ){
    પૂર્ણાંક d1;
    અક્ષર d2[4];
    જો( 0==sqlite3IsLikeFunction(pWalker->pParse->db, p_પદાવલિ, &d1, d2) ){
      pWalker->eCode = 1;
    }
  }

  પાછા WRC_Continue;
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
સ્થિર પૂર્ણાંક code_Cursor_Hint_Fix_પદાવલિ(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  પૂર્ણાંક rc = WRC_Continue;
  પૂર્ણાંક reg;
  સંરચના CCurHint *pHint = pWalker->u.pCCurHint;
  જો( p_પદાવલિ->op==TK_COLUMN ){
    જો( p_પદાવલિ->iTable!=pHint->iTabCur ){
      reg = ++pWalker->pParse->nMem;   /* Register for column value */
      reg = sqlite3_પદાવલિ_Code_Target(pWalker->pParse, p_પદાવલિ, reg);
      p_પદાવલિ->op = TK_REGISTER;
      p_પદાવલિ->iTable = reg;
    }નહીંતર જો( pHint->pIdx!=0 ){
      p_પદાવલિ->iTable = pHint->iIdxCur;
      p_પદાવલિ->iColumn = sqlite3TableColumnToIndex(pHint->pIdx, p_પદાવલિ->iColumn);
      પ્રતિજ્ઞા( p_પદાવલિ->iColumn>=0 );
    }
  }નહીંતર જો( p_પદાવલિ->pAggInfo ){
    rc = WRC_Prune;
    reg = ++pWalker->pParse->nMem;   /* Register for column value */
    reg = sqlite3_પદાવલિ_Code_Target(pWalker->pParse, p_પદાવલિ, reg);
    p_પદાવલિ->op = TK_REGISTER;
    p_પદાવલિ->iTable = reg;
  }નહીંતર જો( p_પદાવલિ->op==TK_TRUEFALSE ){
    /* Do not walk disabled expressions.  tag-20230504-1 */
    પાછા WRC_Prune;
  }
  પાછા rc;
}

/*
** Insert an OP_CursorHint instruction if it is appropriate to do so.
*/
સ્થિર શૂન્ય codeCursorHint(
  SrcItem *pTabItem,  /* FROM clause item */
  WhereInfo *pWInfo,    /* The where clause */
  WhereLevel *pLevel,   /* Which loop to provide hints for */
  WhereTerm *pEndRange  /* Hint this end-of-scan boundary term if not NULL */
){
  Parse *pParse = pWInfo->pParse;
  sqlite3 *db = pParse->db;
  Vdbe *v = pParse->pVdbe;
  પદાવલિ *p_પદાવલિ = 0;
  WhereLoop *pLoop = pLevel->pWLoop;
  પૂર્ણાંક iCur;
  WhereClause *pWC;
  WhereTerm *pTerm;
  પૂર્ણાંક i, j;
  સંરચના CCurHint sHint;
  Walker sWalker;

  જો( OptimizationDisabled(db, SQLITE_CursorHints) ) પાછા;
  iCur = pLevel->iTabCur;
  પ્રતિજ્ઞા( iCur==pWInfo->pTabList->a[pLevel->iFrom].iCursor );
  sHint.iTabCur = iCur;
  sHint.iIdxCur = pLevel->iIdxCur;
  sHint.pIdx = pLoop->u.btree.pIndex;
  memset(&sWalker, 0, કદ(sWalker));
  sWalker.pParse = pParse;
  sWalker.u.pCCurHint = &sHint;
  pWC = &pWInfo->sWC;
  માટે(i=0; i<pWC->nBase; i++){
    pTerm = &pWC->a[i];
    જો( pTerm->wtFlags & (TERM_VIRTUAL|TERM_CODED) ) ચાલુ;
    જો( pTerm->prereqAll & pLevel->notReady ) ચાલુ;

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
    જો( pTabItem->fg.jointype & JT_ડાબું ){
      પદાવલિ *p_પદાવલિ = pTerm->p_પદાવલિ;
      જો( !પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON)
       || p_પદાવલિ->w.iJoin!=pTabItem->iCursor
      ){
        sWalker.eCode = 0;
        sWalker.x_પદાવલિ_Callback = codeCursorHintIsOrFunction;
        sqlite3_Walk_પદાવલિ(&sWalker, pTerm->p_પદાવલિ);
        જો( sWalker.eCode ) ચાલુ;
      }
    }નહીંતર{
      જો( પદાવલિ_Has_Property(pTerm->p_પદાવલિ, EP_OuterON) ) ચાલુ;
    }

    /* All terms in pWLoop->aLTerm[] except pEndRange are used to initialize
    ** the cursor.  These terms are not needed as hints for a pure range
    ** scan (that has no == terms) so omit them. */
    જો( pLoop->u.btree.nEq==0 && pTerm!=pEndRange ){
      માટે(j=0; j<pLoop->nLTerm && pLoop->aLTerm[j]!=pTerm; j++){}
      જો( j<pLoop->nLTerm ) ચાલુ;
    }

    /* No subqueries or non-deterministic functions allowed */
    જો( sqlite3_પદાવલિ_Contains_Subquery(pTerm->p_પદાવલિ) ) ચાલુ;

    /* For an index scan, make sure referenced columns are actually in
    ** the index. */
    જો( sHint.pIdx!=0 ){
      sWalker.eCode = 0;
      sWalker.x_પદાવલિ_Callback = code_Cursor_Hint_Check_પદાવલિ;
      sqlite3_Walk_પદાવલિ(&sWalker, pTerm->p_પદાવલિ);
      જો( sWalker.eCode ) ચાલુ;
    }

    /* If we survive all prior tests, that means this term is worth hinting */
    p_પદાવલિ = sqlite3_પદાવલિ_And(pParse, p_પદાવલિ, sqlite3_પદાવલિ_Dup(db, pTerm->p_પદાવલિ, 0));
  }
  જો( p_પદાવલિ!=0 ){
    sWalker.x_પદાવલિ_Callback = code_Cursor_Hint_Fix_પદાવલિ;
    જો( pParse->nErr==0 ) sqlite3_Walk_પદાવલિ(&sWalker, p_પદાવલિ);
    sqlite3VdbeAddOp4(v, OP_CursorHint,
                      (sHint.pIdx ? sHint.iIdxCur : sHint.iTabCur), 0, 0,
                      (અચલ અક્ષર*)p_પદાવલિ, P4_પદાવલિ);
  }
}
#નહીંતર
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
સ્થિર શૂન્ય codeDeferredSeek(
  WhereInfo *pWInfo,              /* Where clause context */
  Index *pIdx,                    /* Index scan is using */
  પૂર્ણાંક iCur,                       /* Cursor for IPK b-tree */
  પૂર્ણાંક iIdxCur                     /* Index cursor */
){
  Parse *pParse = pWInfo->pParse; /* Parse context */
  Vdbe *v = pParse->pVdbe;        /* Vdbe to generate code within */

  પ્રતિજ્ઞા( iIdxCur>0 );
  પ્રતિજ્ઞા( pIdx->aiColumn[pIdx->nColumn-1]==-1 );

  pWInfo->bDeferredSeek = 1;
  sqlite3VdbeAddOp3(v, OP_DeferredSeek, iIdxCur, 0, iCur);
  જો( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_જમણું_JOIN))
   && DbMaskAllZero(sqlite3ParseToplevel(pParse)->writeMask)
  ){
    પૂર્ણાંક i;
    Table *pTab = pIdx->pTable;
    u32 *ai = (u32*)sqlite3DbMallocZero(pParse->db, કદ(u32)*(pTab->nCol+1));
    જો( ai ){
      ai[0] = pTab->nCol;
      માટે(i=0; i<pIdx->nColumn-1; i++){
        પૂર્ણાંક x1, x2;
        પ્રતિજ્ઞા( pIdx->aiColumn[i]<pTab->nCol );
        x1 = pIdx->aiColumn[i];
        x2 = sqlite3TableColumnToStorage(pTab, x1);
        testcase( x1!=x2 );
        જો( x1>=0 ) ai[x2+1] = i+1;
      }
      sqlite3VdbeChangeP4(v, -1, (અક્ષર*)ai, P4_INTARRAY);
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
સ્થિર શૂન્ય code_પદાવલિ_Or_Vector(Parse *pParse, પદાવલિ *p, પૂર્ણાંક iReg, પૂર્ણાંક nReg){
  પ્રતિજ્ઞા( nReg>0 );
  જો( p && sqlite3_પદાવલિ_Is_Vector(p) ){
#ifndef SQLITE_OMIT_SUBQUERY
    જો( પદાવલિ_Use_XSelect(p) ){
      Vdbe *v = pParse->pVdbe;
      પૂર્ણાંક iSelect;
      પ્રતિજ્ઞા( p->op==TK_SELECT );
      iSelect = sqlite3CodeSubselect(pParse, p);
      sqlite3VdbeAddOp3(v, OP_Copy, iSelect, iReg, nReg-1);
    }નહીંતર
#endif
    {
      પૂર્ણાંક i;
      અચલ પદાવલિ_List *pList;
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p) );
      pList = p->x.pList;
      પ્રતિજ્ઞા( nReg<=pList->n_પદાવલિ );
      માટે(i=0; i<nReg; i++){
        sqlite3_પદાવલિ_Code(pParse, pList->a[i].p_પદાવલિ, iReg+i);
      }
    }
  }નહીંતર{
    પ્રતિજ્ઞા( nReg==1 || pParse->nErr );
    sqlite3_પદાવલિ_Code(pParse, p, iReg);
  }
}

/*
** The pTruth expression is always true because it is the WHERE clause
** a partial index that is driving a query loop.  Look through all of the
** WHERE clause terms on the query, and if any of those terms must be
** true because pTruth is true, then mark those WHERE clause terms as
** coded.
*/
સ્થિર શૂન્ય whereApplyPartialIndexConstraints(
  પદાવલિ *pTruth,
  પૂર્ણાંક iTabCur,
  WhereClause *pWC
){
  પૂર્ણાંક i;
  WhereTerm *pTerm;
  જ્યારે( pTruth->op==TK_AND ){
    whereApplyPartialIndexConstraints(pTruth->p_ડાબું, iTabCur, pWC);
    pTruth = pTruth->p_જમણું;
  }
  માટે(i=0, pTerm=pWC->a; i<pWC->nTerm; i++, pTerm++){
    પદાવલિ *p_પદાવલિ;
    જો( pTerm->wtFlags & TERM_CODED ) ચાલુ;
    p_પદાવલિ = pTerm->p_પદાવલિ;
    જો( sqlite3_પદાવલિ_Compare(0, p_પદાવલિ, pTruth, iTabCur)==0 ){
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
સ્થિર SQLITE_NOINLINE શૂન્ય filterPullDown(
  Parse *pParse,       /* Parsing context */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  પૂર્ણાંક iLevel,          /* Which level of pWInfo->a[] should be coded */
  પૂર્ણાંક addrNxt,         /* Jump here to bypass inner loops */
  Bitmask notReady     /* Loops that are not ready */
){
  પૂર્ણાંક saved_addrBrk;
  જ્યારે( ++iLevel < pWInfo->nLevel ){
    WhereLevel *pLevel = &pWInfo->a[iLevel];
    WhereLoop *pLoop = pLevel->pWLoop;
    જો( pLevel->regFilter==0 ) ચાલુ;
    જો( pLevel->pWLoop->nSkip ) ચાલુ;
    /*         ,--- Because sqlite3ConstructBloomFilter() has will not have set
    **  vvvvv--'    pLevel->regFilter if this were true. */
    જો( NEVER(pLoop->prereq & notReady) ) ચાલુ;
    saved_addrBrk = pLevel->addrBrk;
    pLevel->addrBrk = addrNxt;
    જો( pLoop->wsFlags & WHERE_IPK ){
      WhereTerm *pTerm = pLoop->aLTerm[0];
      પૂર્ણાંક regRowid;
      પ્રતિજ્ઞા( pTerm!=0 );
      પ્રતિજ્ઞા( pTerm->p_પદાવલિ!=0 );
      testcase( pTerm->wtFlags & TERM_VIRTUAL );
      regRowid = sqlite3GetTempReg(pParse);
      regRowid = codeEqualityTerm(pParse, pTerm, pLevel, 0, 0, regRowid);
      sqlite3VdbeAddOp2(pParse->pVdbe, OP_MustBeInt, regRowid, addrNxt);
      VdbeCoverage(pParse->pVdbe);
      પ્રતિજ્ઞા( sqlite3WhereLoopBloomable(pLoop) );
      sqlite3VdbeAddOp4Int(pParse->pVdbe, OP_Filter, pLevel->regFilter,
                           addrNxt, regRowid, 1);
      VdbeCoverage(pParse->pVdbe);
    }નહીંતર{
      u16 nEq = pLoop->u.btree.nEq;
      પૂર્ણાંક r1;
      અક્ષર *zStartAff;

      પ્રતિજ્ઞા( pLoop->wsFlags & WHERE_INDEXED );
      પ્રતિજ્ઞા( (pLoop->wsFlags & WHERE_COLUMN_IN)==0 );
      r1 = codeAllEqualityTerms(pParse,pLevel,0,0,&zStartAff);
      codeApplyAffinity(pParse, r1, nEq, zStartAff);
      sqlite3DbFree(pParse->db, zStartAff);
      પ્રતિજ્ઞા( sqlite3WhereLoopBloomable(pLoop) );
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
સ્થિર પૂર્ણાંક whereLoopIsOneRow(WhereLoop *pLoop){
  જો( pLoop->u.btree.pIndex->onError
   && pLoop->nSkip==0
   && pLoop->u.btree.nEq==pLoop->u.btree.pIndex->n_ચાવી_Col
  ){
    પૂર્ણાંક ii;
    માટે(ii=0; ii<pLoop->u.btree.nEq; ii++){
      જો( pLoop->aLTerm[ii]->eOperator & (WO_IS|WO_ISNULL) ){
        પાછા 0;
      }
    }
    પાછા 1;
  }
  પાછા 0;
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
સ્થિર પૂર્ણાંક where_પદાવલિ_Is_Ready(
  WhereInfo *pWInfo, 
  WhereLevel *pLevel,
  SrcItem *pTabItem, 
  પદાવલિ *pE
){
  u8 jtype = pTabItem->fg.jointype;
  જો( jtype & (JT_ડાબું|JT_LTORJ|JT_જમણું) ){
    જો( !પદાવલિ_Has_Property(pE,EP_OuterON|EP_InnerON) ){
      /* Defer processing WHERE clause constraints until after outer
      ** join processing.  tag-20220513a */
      પાછા 0;
    }નહીંતર જો( (jtype & JT_ડાબું) && !પદાવલિ_Has_Property(pE,EP_OuterON) ){
      પાછા 0;
    }નહીંતર{
      Bitmask m = sqlite3WhereGetMask(&pWInfo->sMaskSet, pE->w.iJoin);
      જો( m & pLevel->notReady ){
        /* An ON clause that is not ripe */
        પાછા 0;
      }
    }
  }
  પાછા 1;
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
સ્થિર પૂર્ણાંક findNoMatchContinuation(
  WhereInfo *pWInfo,         /* WHERE clause */
  પૂર્ણાંક iLevel                 /* Which loop we are talking about */
){
  WhereLoop *pLoop;          /* Description of this loop */
  WhereLevel *pParent;       /* Controlling parent loop */
  SrcList *pTabList;         /* Complete FROM clause */
  Bitmask m_ચાવી;              /* Mask of dependent tables */
  પૂર્ણાંક iParent;               /* Inner-most loop affecting the key to iLevel */
  પૂર્ણાંક i;                     /* Loop counter */

  જો( iLevel<2 ){
    /* If there are less than two outer loops, this optimization is no help */
    પાછા 0;
  }

  /* The OP_SeekGE and OP_SeekLE opcodes only work right for this
  ** optimization on a pure equality constraint, so skip the optimization
  ** for anything else.
  */
  pLoop = pWInfo->a[iLevel].pWLoop;
  જો( (pLoop->wsFlags & WHERE_CONSTRAINT)!=WHERE_COLUMN_EQ ) પાછા 0;
  જો( pLoop->wsFlags & WHERE_SKIPSCAN ) પાછા 0;
  જો( pLoop->wsFlags & WHERE_BIGNULL_SORT ) પાછા 0;
  પ્રતિજ્ઞા( (pLoop->wsFlags & (WHERE_VIRTUALTABLE|WHERE_MULTI_OR))==0 );
  પ્રતિજ્ઞા( pLoop->u.btree.nEq>0 );

  /* Find a mask of all outer tables that influence the key.  At the same
  ** time, check to see if any keys values are computed from subqueries and
  ** fail out (return 0) if there are any. */
  m_ચાવી = 0;
  માટે(i=0; i<pLoop->nLTerm; i++){
    જો( sqlite3_પદાવલિ_Contains_Subquery(pLoop->aLTerm[i]->p_પદાવલિ) ){
      પાછા 0;
    }
    m_ચાવી |= pLoop->aLTerm[i]->prereq_જમણું;
  }

  /* Look back through outer loops.  Find the inner-most outer loop
  ** that influences the key. Make iParent be the index of that inner-most
  ** outer loop.
  */
  pTabList = pWInfo->pTabList;
  iParent = iLevel-1;
  જ્યારે( iParent>0 && (pWInfo->a[iParent].pWLoop->maskSelf & m_ચાવી)==0 ){
    iParent--;
  }
  પ્રતિજ્ઞા( iParent>=0 && iParent<=iLevel );

  /* Everything needs to be an INNER join.  Fail if there are any
  ** OUTER joins in between iParent and iLevel. */
  માટે(i=iParent; i<=iLevel; i++){
    જો( pTabList->a[pWInfo->a[i].iFrom].fg.jointype & JT_OUTER ) પાછા 0;
  }

  /* If we reach this point, that means the conditions of the optimization
  ** have been met.  Even so, disallow the optimization if it is disabled.
  */
  જો( OptimizationDisabled(pWInfo->pParse->db, SQLITE_JoinMiss) ){
    પાછા 0;
  }

  /* If the inner-most outer loop that influences the key for iLevel
  ** is controlled by an automatic index and if the automatic index is
  ** the only outer loop that influences the key, then arrange to delete
  ** the automatic index entry if there is no match on the key.
  */
  pParent = &pWInfo->a[iParent];
  જો( (pParent->pWLoop->wsFlags & WHERE_AUTO_INDEX)!=0
   && (m_ચાવી & (m_ચાવી-1))==0
  ){
    માટે(i=0; i<pLoop->nLTerm; i++){
      પદાવલિ *pX = pLoop->aLTerm[i]->p_પદાવલિ;
      જો( !sqlite3_પદાવલિ_Is_Table_Constant(pX->p_જમણું,pParent->iTabCur,0) ) તોડો;
    }
    જો( i==pLoop->nLTerm ){    
      જો( pParent->addrDeleteCont==0 ){
        pParent->addrDeleteCont = sqlite3VdbeMakeLabel(pWInfo->pParse);
      }
      પાછા pParent->addrDeleteCont;
    }
  }

  testcase( iParent<iLevel-1 );
  પાછા pParent->addrCont;
}

/*
** Generate code for the start of the iLevel-th loop in the WHERE clause
** implementation described by pWInfo.
*/
Bitmask sqlite3WhereCodeOneLoopStart(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  પૂર્ણાંક iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
){
  પૂર્ણાંક j, k;            /* Loop counters */
  પૂર્ણાંક iCur;            /* The VDBE cursor for the table */
  પૂર્ણાંક addrNxt;         /* Where to jump to continue with the next IN case */
  પૂર્ણાંક bRev;            /* True if we need to scan in reverse order */
  WhereLoop *pLoop;    /* The WhereLoop object being coded */
  WhereClause *pWC;    /* Decomposition of the entire WHERE clause */
  WhereTerm *pTerm;         /* A WHERE clause term */
  sqlite3 *db;              /* Database connection */
  SrcItem *pTabItem;        /* FROM clause term being coded */
  પૂર્ણાંક addrBrk;              /* Jump here to break out of the loop */
  પૂર્ણાંક addrCont;             /* Jump here to continue with next cycle */
  પૂર્ણાંક addrMiss;             /* addrCont enhanced for no-matching rows */
  પૂર્ણાંક iRowidReg = 0;        /* Rowid is stored in this register, if not zero */
  પૂર્ણાંક iReleaseReg = 0;      /* Temp register to free before returning */
  Index *pIdx = 0;          /* Index used by loop (if any) */
  પૂર્ણાંક iLoop;                /* Iteration of constraint generator loop */

  pWC = &pWInfo->sWC;
  db = pParse->db;
  pLoop = pLevel->pWLoop;
  pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
  iCur = pTabItem->iCursor;
  pLevel->notReady = notReady & ~sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur);
  bRev = (pWInfo->revMask>>iLevel)&1;
  VdbeModuleComment((v, "Begin WHERE-loop%d: %s",
                     iLevel, pTabItem->pSTab->zName));
#જો WHERETRACE_ENABLED /* 0x4001 */
  જો( sqlite3WhereTrace & 0x1 ){
    sqlite3DebugPrintf("Coding level %d of %d:  notReady=%llx  iFrom=%d\n",
       iLevel, pWInfo->nLevel, (u64)notReady, pLevel->iFrom);
    જો( sqlite3WhereTrace & 0x1000 ){
      sqlite3WhereLoopPrint(pLoop, pWC);
    }
  }
  જો( (sqlite3WhereTrace & 0x4001)==0x4001 ){
    જો( iLevel==0 ){
      sqlite3DebugPrintf("WHERE clause being coded:\n");
      sqlite3_વૃક્ષ_View_પદાવલિ(0, pWInfo->pWhere, 0);
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
  પ્રતિજ્ઞા( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_જમણું_JOIN))
       || pLevel->iFrom>0 || (pTabItem[0].fg.jointype & JT_ડાબું)==0
  );
  જો( pLevel->iFrom>0 && (pTabItem[0].fg.jointype & JT_ડાબું)!=0 ){
    pLevel->i_ડાબું_Join = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, pLevel->i_ડાબું_Join);
    VdbeComment((v, "init LEFT JOIN match flag"));
    જો( pTabItem->fg.viaCoroutine ){
      sqlite3VdbeAddOp2(v, OP_NullRow, pTabItem->iCursor, 1);
    }
  }

  /* Special case of a FROM clause subquery implemented as a co-routine */
  જો( pTabItem->fg.viaCoroutine ){
    પૂર્ણાંક regYield;
    Subquery *pSubq;
    પ્રતિજ્ઞા( pTabItem->fg.isSubquery && pTabItem->u4.pSubq!=0 );
    pSubq = pTabItem->u4.pSubq;
    regYield = pSubq->regReturn;
    sqlite3VdbeAddOp3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    pLevel->p2 =  sqlite3VdbeAddOp2(v, OP_Yield, regYield, addrBrk);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", pTabItem->pSTab->zName));
    pLevel->op = OP_Goto;
  }નહીંતર

#ifndef SQLITE_OMIT_VIRTUALTABLE
  જો(  (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
    /* Case 1:  The table is a virtual-table.  Use the VFilter and VNext
    **          to access the data.
    */
    પૂર્ણાંક iReg;   /* P3 Value for OP_VFilter */
    પૂર્ણાંક addrNotFound;
    પૂર્ણાંક nConstraint = pLoop->nLTerm;

    iReg = sqlite3GetTempRange(pParse, nConstraint+2);
    addrNotFound = pLevel->addrBrk;
    માટે(j=0; j<nConstraint; j++){
      પૂર્ણાંક iTarget = iReg+j+2;
      pTerm = pLoop->aLTerm[j];
      જો( NEVER(pTerm==0) ) ચાલુ;
      જો( pTerm->eOperator & WO_IN ){
        જો( SMASKBIT32(j) & pLoop->u.vtab.mHandleIn ){
          પૂર્ણાંક iTab = pParse->nTab++;
          પૂર્ણાંક iCache = ++pParse->nMem;
          sqlite3CodeRhsOfIN(pParse, pTerm->p_પદાવલિ, iTab, 0);
          sqlite3VdbeAddOp3(v, OP_VInitIn, iTab, iTarget, iCache);
        }નહીંતર{
          codeEqualityTerm(pParse, pTerm, pLevel, j, bRev, iTarget);
          addrNotFound = pLevel->addrNxt;
        }
      }નહીંતર{
        પદાવલિ *p_જમણું = pTerm->p_પદાવલિ->p_જમણું;
        code_પદાવલિ_Or_Vector(pParse, p_જમણું, iTarget, 1);
        જો( pTerm->eMatchOp==SQLITE_INDEX_CONSTRAINT_OFFSET
         && pLoop->u.vtab.bOmitOffset
        ){
          પ્રતિજ્ઞા( pTerm->eOperator==WO_AUX );
          પ્રતિજ્ઞા( pWInfo->pSelect!=0 );
          પ્રતિજ્ઞા( pWInfo->pSelect->iOffset>0 );
          sqlite3VdbeAddOp2(v, OP_Integer, 0, pWInfo->pSelect->iOffset);
          VdbeComment((v,"Zero OFFSET counter"));
        }
      }
    }
    sqlite3VdbeAddOp2(v, OP_Integer, pLoop->u.vtab.idx_અંક, iReg);
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
    જો( db->mallocFailed ) pLoop->u.vtab.idxStr = 0;
    pLevel->p1 = iCur;
    pLevel->op = pWInfo->eOnePass ? OP_Noop : OP_VNext;
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);
    પ્રતિજ્ઞા( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );

    માટે(j=0; j<nConstraint; j++){
      pTerm = pLoop->aLTerm[j];
      જો( j<16 && (pLoop->u.vtab.omitMask>>j)&1 ){
        disableTerm(pLevel, pTerm);
        ચાલુ;
      }
      જો( (pTerm->eOperator & WO_IN)!=0
       && (SMASKBIT32(j) & pLoop->u.vtab.mHandleIn)==0
       && !db->mallocFailed
      ){
        પદાવલિ *pCompare;  /* The comparison operator */
        પદાવલિ *p_જમણું;    /* RHS of the comparison */
        VdbeOp *pOp;     /* Opcode to access the value of the IN constraint */
        પૂર્ણાંક iIn;         /* IN loop corresponding to the j-th constraint */

        /* Reload the constraint value into reg[iReg+j+2].  The same value
        ** was loaded into the same register prior to the OP_VFilter, but
        ** the xFilter implementation might have changed the datatype or
        ** encoding of the value in the register, so it *must* be reloaded.
        */
        માટે(iIn=0; ALWAYS(iIn<pLevel->u.in.nIn); iIn++){
          pOp = sqlite3VdbeGetOp(v, pLevel->u.in.aInLoop[iIn].addr_In_ટોચ);
          જો( (pOp->opcode==OP_Column && pOp->p3==iReg+j+2)
           || (pOp->opcode==OP_Rowid && pOp->p2==iReg+j+2)
          ){
            testcase( pOp->opcode==OP_Rowid );
            sqlite3VdbeAddOp3(v, pOp->opcode, pOp->p1, pOp->p2, pOp->p3);
            તોડો;
          }
        }

        /* Generate code that will continue to the next row if
        ** the IN constraint is not satisfied
        */
        pCompare = sqlite3PExpr(pParse, TK_EQ, 0, 0);
        જો( !db->mallocFailed ){
          પૂર્ણાંક iFld = pTerm->u.x.iField;
          પદાવલિ *p_ડાબું = pTerm->p_પદાવલિ->p_ડાબું;
          પ્રતિજ્ઞા( p_ડાબું!=0 );
          જો( iFld>0 ){
            પ્રતિજ્ઞા( p_ડાબું->op==TK_VECTOR );
            પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_ડાબું) );
            પ્રતિજ્ઞા( iFld<=p_ડાબું->x.pList->n_પદાવલિ );
            pCompare->p_ડાબું = p_ડાબું->x.pList->a[iFld-1].p_પદાવલિ;
          }નહીંતર{
            pCompare->p_ડાબું = p_ડાબું;
          }
          pCompare->p_જમણું = p_જમણું = sqlite3_પદાવલિ(db, TK_REGISTER, 0);
          જો( p_જમણું ){
            p_જમણું->iTable = iReg+j+2;
            sqlite3_પદાવલિ_If_False(
                pParse, pCompare, pLevel->addrCont, SQLITE_JUMPIFNULL
            );
          }
          pCompare->p_ડાબું = 0;
        }
        sqlite3_પદાવલિ_Delete(db, pCompare);
      }
    }

    /* These registers need to be preserved in case there is an IN operator
    ** loop.  So we could deallocate the registers here (and potentially
    ** reuse them later) if (pLoop->wsFlags & WHERE_IN_ABLE)==0.  But it seems
    ** simpler and safer to simply not reuse the registers.
    **
    **    sqlite3ReleaseTempRange(pParse, iReg, nConstraint+2);
    */
  }નહીંતર
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  જો( (pLoop->wsFlags & WHERE_IPK)!=0
   && (pLoop->wsFlags & (WHERE_COLUMN_IN|WHERE_COLUMN_EQ))!=0
  ){
    /* Case 2:  We can directly reference a single row using an
    **          equality comparison against the ROWID field.  Or
    **          we reference multiple rows using a "rowid IN (...)"
    **          construct.
    */
    પ્રતિજ્ઞા( pLoop->u.btree.nEq==1 );
    pTerm = pLoop->aLTerm[0];
    પ્રતિજ્ઞા( pTerm!=0 );
    પ્રતિજ્ઞા( pTerm->p_પદાવલિ!=0 );
    testcase( pTerm->wtFlags & TERM_VIRTUAL );
    iReleaseReg = ++pParse->nMem;
    iRowidReg = codeEqualityTerm(pParse, pTerm, pLevel, 0, bRev, iReleaseReg);
    જો( iRowidReg!=iReleaseReg ) sqlite3ReleaseTempReg(pParse, iReleaseReg);
    addrNxt = pLevel->addrNxt;
    જો( addrMiss==0 ) addrMiss = addrNxt;
    જો( pLevel->regFilter ){
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
  }નહીંતર જો( (pLoop->wsFlags & WHERE_IPK)!=0
         && (pLoop->wsFlags & WHERE_COLUMN_RANGE)!=0
  ){
    /* Case 3:  We have an inequality comparison against the ROWID field.
    */
    પૂર્ણાંક testOp = OP_Noop;
    પૂર્ણાંક start;
    પૂર્ણાંક mem_End_કિંમત = 0;
    WhereTerm *pStart, *pEnd;

    j = 0;
    pStart = pEnd = 0;
    જો( pLoop->wsFlags & WHERE_BTM_LIMIT ) pStart = pLoop->aLTerm[j++];
    જો( pLoop->wsFlags & WHERE_ટોચ_LIMIT ) pEnd = pLoop->aLTerm[j++];
    પ્રતિજ્ઞા( pStart!=0 || pEnd!=0 );
    જો( bRev ){
      pTerm = pStart;
      pStart = pEnd;
      pEnd = pTerm;
    }
    codeCursorHint(pTabItem, pWInfo, pLevel, pEnd);
    જો( pStart ){
      પદાવલિ *pX;             /* The expression that defines the start bound */
      પૂર્ણાંક r1, rTemp;        /* Registers for holding the start boundary */
      પૂર્ણાંક op;               /* Cursor seek operation */

      /* The following constant maps TK_xx codes into corresponding
      ** seek opcodes.  It depends on a particular ordering of TK_xx
      */
      અચલ u8 aMoveOp[] = {
           /* TK_GT */  OP_SeekGT,
           /* TK_LE */  OP_SeekLE,
           /* TK_LT */  OP_SeekLT,
           /* TK_GE */  OP_SeekGE
      };
      પ્રતિજ્ઞા( TK_LE==TK_GT+1 );      /* Make sure the ordering.. */
      પ્રતિજ્ઞા( TK_LT==TK_GT+2 );      /*  ... of the TK_xx values... */
      પ્રતિજ્ઞા( TK_GE==TK_GT+3 );      /*  ... is correct. */

      પ્રતિજ્ઞા( (pStart->wtFlags & TERM_VNULL)==0 );
      testcase( pStart->wtFlags & TERM_VIRTUAL );
      pX = pStart->p_પદાવલિ;
      પ્રતિજ્ઞા( pX!=0 );
      testcase( pStart->ડાબું_Cursor!=iCur ); /* transitive constraints */
      જો( sqlite3_પદાવલિ_Is_Vector(pX->p_જમણું) ){
        r1 = rTemp = sqlite3GetTempReg(pParse);
        code_પદાવલિ_Or_Vector(pParse, pX->p_જમણું, r1, 1);
        testcase( pX->op==TK_GT );
        testcase( pX->op==TK_GE );
        testcase( pX->op==TK_LT );
        testcase( pX->op==TK_LE );
        op = aMoveOp[((pX->op - TK_GT - 1) & 0x3) | 0x1];
        પ્રતિજ્ઞા( pX->op!=TK_GT || op==OP_SeekGE );
        પ્રતિજ્ઞા( pX->op!=TK_GE || op==OP_SeekGE );
        પ્રતિજ્ઞા( pX->op!=TK_LT || op==OP_SeekLE );
        પ્રતિજ્ઞા( pX->op!=TK_LE || op==OP_SeekLE );
      }નહીંતર{
        r1 = sqlite3_પદાવલિ_Code_Temp(pParse, pX->p_જમણું, &rTemp);
        disableTerm(pLevel, pStart);
        op = aMoveOp[(pX->op - TK_GT)];
      }
      sqlite3VdbeAddOp3(v, op, iCur, addrBrk, r1);
      VdbeComment((v, "pk"));
      VdbeCoverageIf(v, pX->op==TK_GT);
      VdbeCoverageIf(v, pX->op==TK_LE);
      VdbeCoverageIf(v, pX->op==TK_LT);
      VdbeCoverageIf(v, pX->op==TK_GE);
      sqlite3ReleaseTempReg(pParse, rTemp);
    }નહીંતર{
      sqlite3VdbeAddOp2(v, bRev ? OP_Last : OP_Rewind, iCur, pLevel->addrHalt);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
    }
    જો( pEnd ){
      પદાવલિ *pX;
      pX = pEnd->p_પદાવલિ;
      પ્રતિજ્ઞા( pX!=0 );
      પ્રતિજ્ઞા( (pEnd->wtFlags & TERM_VNULL)==0 );
      testcase( pEnd->ડાબું_Cursor!=iCur ); /* Transitive constraints */
      testcase( pEnd->wtFlags & TERM_VIRTUAL );
      mem_End_કિંમત = ++pParse->nMem;
      code_પદાવલિ_Or_Vector(pParse, pX->p_જમણું, mem_End_કિંમત, 1);
      જો( 0==sqlite3_પદાવલિ_Is_Vector(pX->p_જમણું)
       && (pX->op==TK_LT || pX->op==TK_GT)
      ){
        testOp = bRev ? OP_Le : OP_Ge;
      }નહીંતર{
        testOp = bRev ? OP_Lt : OP_Gt;
      }
      જો( 0==sqlite3_પદાવલિ_Is_Vector(pX->p_જમણું) ){
        disableTerm(pLevel, pEnd);
      }
    }
    start = sqlite3VdbeCurrentAddr(v);
    pLevel->op = bRev ? OP_Prev : OP_Next;
    pLevel->p1 = iCur;
    pLevel->p2 = start;
    પ્રતિજ્ઞા( pLevel->p5==0 );
    જો( testOp!=OP_Noop ){
      iRowidReg = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, iRowidReg);
      sqlite3VdbeAddOp3(v, testOp, mem_End_કિંમત, addrBrk, iRowidReg);
      VdbeCoverageIf(v, testOp==OP_Le);
      VdbeCoverageIf(v, testOp==OP_Lt);
      VdbeCoverageIf(v, testOp==OP_Ge);
      VdbeCoverageIf(v, testOp==OP_Gt);
      sqlite3VdbeChangeP5(v, SQLITE_AFF_NUMERIC | SQLITE_JUMPIFNULL);
    }
  }નહીંતર જો( pLoop->wsFlags & WHERE_INDEXED ){
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
    સ્થિર અચલ u8 aStartOp[] = {
      0,
      0,
      OP_Rewind,           /* 2: (!start_constraints && startEq &&  !bRev) */
      OP_Last,             /* 3: (!start_constraints && startEq &&   bRev) */
      OP_SeekGT,           /* 4: (start_constraints  && !startEq && !bRev) */
      OP_SeekLT,           /* 5: (start_constraints  && !startEq &&  bRev) */
      OP_SeekGE,           /* 6: (start_constraints  &&  startEq && !bRev) */
      OP_SeekLE            /* 7: (start_constraints  &&  startEq &&  bRev) */
    };
    સ્થિર અચલ u8 aEndOp[] = {
      OP_IdxGE,            /* 0: (end_constraints && !bRev && !endEq) */
      OP_IdxGT,            /* 1: (end_constraints && !bRev &&  endEq) */
      OP_IdxLE,            /* 2: (end_constraints &&  bRev && !endEq) */
      OP_IdxLT,            /* 3: (end_constraints &&  bRev &&  endEq) */
    };
    u16 nEq = pLoop->u.btree.nEq;     /* Number of == or IN terms */
    u16 nBtm = pLoop->u.btree.nBtm;   /* Length of BTM vector */
    u16 n_ટોચ = pLoop->u.btree.n_ટોચ;   /* Length of TOP vector */
    પૂર્ણાંક regBase;                 /* Base register holding constraint values */
    WhereTerm *pRangeStart = 0;  /* Inequality constraint at range start */
    WhereTerm *pRangeEnd = 0;    /* Inequality constraint at range end */
    પૂર્ણાંક startEq;                 /* True if range start uses ==, >= or <= */
    પૂર્ણાંક endEq;                   /* True if range end uses ==, >= or <= */
    પૂર્ણાંક start_constraints;       /* Start of range is constrained */
    પૂર્ણાંક nConstraint;             /* Number of constraint terms */
    પૂર્ણાંક iIdxCur;                 /* The VDBE cursor for the index */
    પૂર્ણાંક nExtraReg = 0;           /* Number of extra registers needed */
    પૂર્ણાંક op;                      /* Instruction opcode */
    અક્ષર *zStartAff;             /* Affinity for start of range constraint */
    અક્ષર *zEndAff = 0;           /* Affinity for end of range constraint */
    u8 bSeekPastNull = 0;        /* True to seek past initial nulls */
    u8 bStopAtNull = 0;          /* Add condition to terminate at NULLs */
    પૂર્ણાંક omitTable;               /* True if we use the index only */
    પૂર્ણાંક regBignull = 0;          /* big-null flag register */
    પૂર્ણાંક addrSeekScan = 0;        /* Opcode of the OP_SeekScan, if any */

    pIdx = pLoop->u.btree.pIndex;
    iIdxCur = pLevel->iIdxCur;
    પ્રતિજ્ઞા( nEq>=pLoop->nSkip );

    /* Find any inequality constraint terms for the start and end
    ** of the range.
    */
    j = nEq;
    જો( pLoop->wsFlags & WHERE_BTM_LIMIT ){
      pRangeStart = pLoop->aLTerm[j++];
      nExtraReg = MAX(nExtraReg, pLoop->u.btree.nBtm);
      /* Like optimization range constraints always occur in pairs */
      પ્રતિજ્ઞા( (pRangeStart->wtFlags & TERM_LIKEOPT)==0 ||
              (pLoop->wsFlags & WHERE_ટોચ_LIMIT)!=0 );
    }
    જો( pLoop->wsFlags & WHERE_ટોચ_LIMIT ){
      pRangeEnd = pLoop->aLTerm[j++];
      nExtraReg = MAX(nExtraReg, pLoop->u.btree.n_ટોચ);
#ifndef SQLITE_LIKE_DOESNT_MATCH_BLOBS
      જો( (pRangeEnd->wtFlags & TERM_LIKEOPT)!=0 ){
        પ્રતિજ્ઞા( pRangeStart!=0 );                     /* LIKE opt constraints */
        પ્રતિજ્ઞા( pRangeStart->wtFlags & TERM_LIKEOPT );   /* occur in pairs */
        pLevel->iLikeRepCntr = (u32)++pParse->nMem;
        /* iLikeRepCntr actually stores 2x the counter register number.  The
        ** bottom bit indicates whether the search order is ASC or DESC. */
        testcase( bRev );
        testcase( pIdx->aSortOrder[nEq]==SQLITE_SO_DESC );
        પ્રતિજ્ઞા( (bRev & ~1)==0 );
        pLevel->iLikeRepCntr <<=1;
        pLevel->iLikeRepCntr |= bRev ^ (pIdx->aSortOrder[nEq]==SQLITE_SO_DESC);
      }
#endif
      જો( pRangeStart==0 ){
        j = pIdx->aiColumn[nEq];
        જો( (j>=0 && pIdx->pTable->aCol[j].notNull==0) || j==XN_પદાવલિ ){
          bSeekPastNull = 1;
        }
      }
    }
    પ્રતિજ્ઞા( pRangeEnd==0 || (pRangeEnd->wtFlags & TERM_VNULL)==0 );

    /* If the WHERE_BIGNULL_SORT flag is set, then index column nEq uses
    ** a non-default "big-null" sort (either ASC NULLS LAST or DESC NULLS
    ** FIRST). In both cases separate ordered scans are made of those
    ** index entries for which the column is null and for those for which
    ** it is not. For an ASC sort, the non-NULL entries are scanned first.
    ** For DESC, NULL entries are scanned first.
    */
    જો( (pLoop->wsFlags & (WHERE_ટોચ_LIMIT|WHERE_BTM_LIMIT))==0
     && (pLoop->wsFlags & WHERE_BIGNULL_SORT)!=0
    ){
      પ્રતિજ્ઞા( bSeekPastNull==0 && nExtraReg==0 && nBtm==0 && n_ટોચ==0 );
      પ્રતિજ્ઞા( pRangeEnd==0 && pRangeStart==0 );
      testcase( pLoop->nSkip>0 );
      nExtraReg = 1;
      bSeekPastNull = 1;
      pLevel->regBignull = regBignull = ++pParse->nMem;
      જો( pLevel->i_ડાબું_Join ){
        sqlite3VdbeAddOp2(v, OP_Integer, 0, regBignull);
      }
      pLevel->addrBignull = sqlite3VdbeMakeLabel(pParse);
    }

    /* If we are doing a reverse order scan on an ascending index, or
    ** a forward order scan on a descending index, interchange the
    ** start and end terms (pRangeStart and pRangeEnd).
    */
    જો( (nEq<pIdx->nColumn && bRev==(pIdx->aSortOrder[nEq]==SQLITE_SO_ASC)) ){
      SWAP(WhereTerm *, pRangeEnd, pRangeStart);
      SWAP(u8, bSeekPastNull, bStopAtNull);
      SWAP(u8, nBtm, n_ટોચ);
    }

    જો( iLevel>0 && (pLoop->wsFlags & WHERE_IN_SEEKSCAN)!=0 ){
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
    પ્રતિજ્ઞા( zStartAff==0 || sqlite3Strlen30(zStartAff)>=nEq );
#ifndef SQLITE_LIKE_DOESNT_MATCH_BLOBS
    જો( pLevel->iLikeRepCntr ){
      /* The LIKE optimization loop must be the innermost of any IN(...)
      ** or skip-scan loops. Otherwise rows may not be visited in index 
      ** order. This means it needs an addrNxt label for the same reason
      ** IN(...) scans do.  */
      જો( pLevel->addrNxt==pLevel->addrBrk ){
        pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3VdbeAddOp2(v, OP_Integer, 1, (પૂર્ણાંક)(pLevel->iLikeRepCntr>>1));
      VdbeComment((v, "LIKE loop counter"));
      pLevel->addrLikeRep = sqlite3VdbeCurrentAddr(v);
    }
#endif
    જો( zStartAff && n_ટોચ ){
      zEndAff = sqlite3DbStrDup(db, &zStartAff[nEq]);
    }
    addrNxt = (regBignull ? pLevel->addrBignull : pLevel->addrNxt);
    જો( addrMiss==0 ) addrMiss = addrNxt;

    testcase( pRangeStart && (pRangeStart->eOperator & WO_LE)!=0 );
    testcase( pRangeStart && (pRangeStart->eOperator & WO_GE)!=0 );
    testcase( pRangeEnd && (pRangeEnd->eOperator & WO_LE)!=0 );
    testcase( pRangeEnd && (pRangeEnd->eOperator & WO_GE)!=0 );
    startEq = !pRangeStart || pRangeStart->eOperator & (WO_LE|WO_GE);
    endEq =   !pRangeEnd || pRangeEnd->eOperator & (WO_LE|WO_GE);
    start_constraints = pRangeStart || nEq>0;

    /* Seek the index cursor to the start of the range. */
    nConstraint = nEq;
    જો( pRangeStart ){
      પદાવલિ *p_જમણું = pRangeStart->p_પદાવલિ->p_જમણું;
      code_પદાવલિ_Or_Vector(pParse, p_જમણું, regBase+nEq, nBtm);
      whereLikeOptimizationStringFixup(v, pLevel, pRangeStart);
      જો( (pRangeStart->wtFlags & TERM_VNULL)==0
       && sqlite3_પદાવલિ_Can_Be_Null(p_જમણું)
      ){
        sqlite3VdbeAddOp2(v, OP_IsNull, regBase+nEq, addrNxt);
        VdbeCoverage(v);
      }
      જો( zStartAff ){
        updateRangeAffinityStr(p_જમણું, nBtm, &zStartAff[nEq]);
      }
      nConstraint += nBtm;
      testcase( pRangeStart->wtFlags & TERM_VIRTUAL );
      જો( sqlite3_પદાવલિ_Is_Vector(p_જમણું)==0 ){
        disableTerm(pLevel, pRangeStart);
      }નહીંતર{
        startEq = 1;
      }
      bSeekPastNull = 0;
    }નહીંતર જો( bSeekPastNull ){
      startEq = 0;
      sqlite3VdbeAddOp2(v, OP_Null, 0, regBase+nEq);
      start_constraints = 1;
      nConstraint++;
    }નહીંતર જો( regBignull ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, regBase+nEq);
      start_constraints = 1;
      nConstraint++;
    }
    codeApplyAffinity(pParse, regBase, nConstraint - bSeekPastNull, zStartAff);
    જો( pLoop->nSkip>0 && nConstraint==pLoop->nSkip ){
      /* The skip-scan logic inside the call to codeAllEqualityConstraints()
      ** above has already left the cursor sitting on the correct row,
      ** so no further seeking is needed */
    }નહીંતર{
      જો( regBignull ){
        sqlite3VdbeAddOp2(v, OP_Integer, 1, regBignull);
        VdbeComment((v, "NULL-scan pass ctr"));
      }
      જો( pLevel->regFilter ){
        પ્રતિજ્ઞા( sqlite3WhereLoopBloomable(pLoop) );
        sqlite3VdbeAddOp4Int(v, OP_Filter, pLevel->regFilter, addrMiss,
                             regBase, nEq);
        VdbeCoverage(v);
        filterPullDown(pParse, pWInfo, iLevel, addrNxt, notReady);
      }

      op = aStartOp[(start_constraints<<2) + (startEq<<1) + bRev];
      પ્રતિજ્ઞા( op!=0 );
      જો( (pLoop->wsFlags & WHERE_IN_SEEKSCAN)!=0 && op==OP_SeekGE ){
        પ્રતિજ્ઞા( regBignull==0 );
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
        જો( pRangeStart || pRangeEnd ){
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

      પ્રતિજ્ઞા( bSeekPastNull==0 || bStopAtNull==0 );
      જો( regBignull ){
        પ્રતિજ્ઞા( bSeekPastNull==1 || bStopAtNull==1 );
        પ્રતિજ્ઞા( bSeekPastNull==!bStopAtNull );
        પ્રતિજ્ઞા( bStopAtNull==startEq );
        sqlite3VdbeAddOp2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        op = aStartOp[(nConstraint>1)*4 + 2 + bRev];
        sqlite3VdbeAddOp4Int(v, op, iIdxCur, addrNxt, regBase,
                             nConstraint-startEq);
        VdbeCoverage(v);
        VdbeCoverageIf(v, op==OP_Rewind);  testcase( op==OP_Rewind );
        VdbeCoverageIf(v, op==OP_Last);    testcase( op==OP_Last );
        VdbeCoverageIf(v, op==OP_SeekGE);  testcase( op==OP_SeekGE );
        VdbeCoverageIf(v, op==OP_SeekLE);  testcase( op==OP_SeekLE );
        પ્રતિજ્ઞા( op==OP_Rewind || op==OP_Last || op==OP_SeekGE || op==OP_SeekLE);
      }
    }

    /* Load the value for the inequality constraint at the end of the
    ** range (if any).
    */
    nConstraint = nEq;
    પ્રતિજ્ઞા( pLevel->p2==0 );
    જો( pRangeEnd ){
      પદાવલિ *p_જમણું = pRangeEnd->p_પદાવલિ->p_જમણું;
      પ્રતિજ્ઞા( addrSeekScan==0 );
      code_પદાવલિ_Or_Vector(pParse, p_જમણું, regBase+nEq, n_ટોચ);
      whereLikeOptimizationStringFixup(v, pLevel, pRangeEnd);
      જો( (pRangeEnd->wtFlags & TERM_VNULL)==0
       && sqlite3_પદાવલિ_Can_Be_Null(p_જમણું)
      ){
        sqlite3VdbeAddOp2(v, OP_IsNull, regBase+nEq, addrNxt);
        VdbeCoverage(v);
      }
      જો( zEndAff ){
        updateRangeAffinityStr(p_જમણું, n_ટોચ, zEndAff);
        codeApplyAffinity(pParse, regBase+nEq, n_ટોચ, zEndAff);
      }નહીંતર{
        પ્રતિજ્ઞા( pParse->db->mallocFailed );
      }
      nConstraint += n_ટોચ;
      testcase( pRangeEnd->wtFlags & TERM_VIRTUAL );

      જો( sqlite3_પદાવલિ_Is_Vector(p_જમણું)==0 ){
        disableTerm(pLevel, pRangeEnd);
      }નહીંતર{
        endEq = 1;
      }
    }નહીંતર જો( bStopAtNull ){
      જો( regBignull==0 ){
        sqlite3VdbeAddOp2(v, OP_Null, 0, regBase+nEq);
        endEq = 0;
      }
      nConstraint++;
    }
    જો( zStartAff ) sqlite3DbNNFreeNN(db, zStartAff);
    જો( zEndAff ) sqlite3DbNNFreeNN(db, zEndAff);

    /* Top of the loop body */
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    /* Check if the index cursor is past the end of the range. */
    જો( nConstraint ){
      જો( regBignull ){
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
      જો( addrSeekScan ) sqlite3VdbeJumpHere(v, addrSeekScan);
    }
    જો( regBignull ){
      /* During a NULL-scan, check to see if we have reached the end of
      ** the NULLs */
      પ્રતિજ્ઞા( bSeekPastNull==!bStopAtNull );
      પ્રતિજ્ઞા( bSeekPastNull+bStopAtNull==1 );
      પ્રતિજ્ઞા( nConstraint+bSeekPastNull>0 );
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

    જો( (pLoop->wsFlags & WHERE_IN_EARLYOUT)!=0 ){
      sqlite3VdbeAddOp3(v, OP_SeekHit, iIdxCur, nEq, nEq);
    }

    /* Seek the table cursor, if required */
    omitTable = (pLoop->wsFlags & WHERE_IDX_ONLY)!=0
           && (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_જમણું_JOIN))==0;
    જો( omitTable ){
      /* pIdx is a covering index.  No need to access the main table. */
    }નહીંતર જો( HasRowid(pIdx->pTable) ){
      codeDeferredSeek(pWInfo, pIdx, iCur, iIdxCur);
    }નહીંતર જો( iCur!=iIdxCur ){
      Index *pPk = sqlite3_Primary_ચાવી_Index(pIdx->pTable);
      iRowidReg = sqlite3GetTempRange(pParse, pPk->n_ચાવી_Col);
      માટે(j=0; j<pPk->n_ચાવી_Col; j++){
        k = sqlite3TableColumnToIndex(pIdx, pPk->aiColumn[j]);
        sqlite3VdbeAddOp3(v, OP_Column, iIdxCur, k, iRowidReg+j);
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iCur, addrCont,
                           iRowidReg, pPk->n_ચાવી_Col); VdbeCoverage(v);
    }

    જો( pLevel->i_ડાબું_Join==0 ){
      /* If a partial index is driving the loop, try to eliminate WHERE clause
      ** terms from the query that must be true due to the WHERE clause of
      ** the partial index.  This optimization does not work on an outer join,
      ** as shown by:
      **
      ** 2019-11-02 ticket 623eff57e76d45f6      (LEFT JOIN)
      ** 2025-05-29 forum post 7dee41d32506c4ae  (RIGHT JOIN)
      */
      જો( pIdx->pPartIdxWhere && pLevel->pRJ==0 ){
        whereApplyPartialIndexConstraints(pIdx->pPartIdxWhere, iCur, pWC);
      }
    }નહીંતર{
      testcase( pIdx->pPartIdxWhere );
      /* The following assert() is not a requirement, merely an observation:
      ** The OR-optimization doesn't work for the right hand table of
      ** a LEFT JOIN: */
      પ્રતિજ્ઞા( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_જમણું_JOIN))==0 );
    }

    /* Record the instruction used to terminate the loop. */
    જો( (pLoop->wsFlags & WHERE_ONEROW)
     || (pLevel->u.in.nIn && regBignull==0 && whereLoopIsOneRow(pLoop))
    ){
      pLevel->op = OP_Noop;
    }નહીંતર જો( bRev ){
      pLevel->op = OP_Prev;
    }નહીંતર{
      pLevel->op = OP_Next;
    }
    pLevel->p1 = iIdxCur;
    pLevel->p3 = (pLoop->wsFlags&WHERE_UNQ_WANTED)!=0 ? 1:0;
    જો( (pLoop->wsFlags & WHERE_CONSTRAINT)==0 ){
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_STEP;
    }નહીંતર{
      પ્રતિજ્ઞા( pLevel->p5==0 );
    }
    જો( omitTable ) pIdx = 0;
  }નહીંતર

#ifndef SQLITE_OMIT_OR_OPTIMIZATION
  જો( pLoop->wsFlags & WHERE_MULTI_OR ){
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
    Index *pCov = 0;             /* Potential covering index (or NULL) */
    પૂર્ણાંક iCovCur = pParse->nTab++;  /* Cursor used for index scans (if any) */

    પૂર્ણાંક regReturn = ++pParse->nMem;           /* Register used with OP_Gosub */
    પૂર્ણાંક regRowset = 0;                        /* Register for RowSet object */
    પૂર્ણાંક regRowid = 0;                         /* Register holding rowid */
    પૂર્ણાંક iLoopBody = sqlite3VdbeMakeLabel(pParse);/* Start of loop body */
    પૂર્ણાંક iRetInit;                             /* Address of regReturn init */
    પૂર્ણાંક untestedTerms = 0;             /* Some terms not completely tested */
    પૂર્ણાંક ii;                            /* Loop counter */
    પદાવલિ *p_And_પદાવલિ = 0;                /* An ".. AND (...)" expression */
    Table *pTab = pTabItem->pSTab;

    pTerm = pLoop->aLTerm[0];
    પ્રતિજ્ઞા( pTerm!=0 );
    પ્રતિજ્ઞા( pTerm->eOperator & WO_OR );
    પ્રતિજ્ઞા( (pTerm->wtFlags & TERM_ORINFO)!=0 );
    pOrWc = &pTerm->u.pOrInfo->wc;
    pLevel->op = OP_Return;
    pLevel->p1 = regReturn;

    /* Set up a new SrcList in pOrTab containing the table being scanned
    ** by this loop in the a[0] slot and all notReady tables in a[1..] slots.
    ** This becomes the SrcList in the recursive call to sqlite3WhereBegin().
    */
    જો( pWInfo->nLevel>1 || pTabItem->fg.fromExists ){
      પૂર્ણાંક nNotReady;                 /* The number of notReady tables */
      SrcItem *origSrc;              /* Original list of tables */
      nNotReady = pWInfo->nLevel - iLevel - 1;
      pOrTab = sqlite3DbMallocRawNN(db, SZ_SRCLIST(nNotReady+1));
      જો( pOrTab==0 ) પાછા notReady;
      pOrTab->nAlloc = (u8)(nNotReady + 1);
      pOrTab->nSrc = pOrTab->nAlloc;
      memcpy(pOrTab->a, pTabItem, કદ(*pTabItem));
      origSrc = pWInfo->pTabList->a;
      માટે(k=1; k<=nNotReady; k++){
        memcpy(&pOrTab->a[k], &origSrc[pLevel[k].iFrom], કદ(pOrTab->a[k]));
      }

      /* Clear the fromExists flag on the OR-optimized table entry so that
      ** the calls to sqlite3WhereEnd() do not code early-exits after the
      ** first row is visited. The early exit applies to this table's
      ** overall loop - including the multiple OR branches and any WHERE
      ** conditions not passed to the sub-loops - not to the sub-loops.  */
      pOrTab->a[0].fg.fromExists = 0;
    }નહીંતર{
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
    જો( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
      જો( HasRowid(pTab) ){
        regRowset = ++pParse->nMem;
        sqlite3VdbeAddOp2(v, OP_Null, 0, regRowset);
      }નહીંતર{
        Index *pPk = sqlite3_Primary_ચાવી_Index(pTab);
        regRowset = pParse->nTab++;
        sqlite3VdbeAddOp2(v, OP_OpenEphemeral, regRowset, pPk->n_ચાવી_Col);
        sqlite3_Vdbe_Set_P4_ચાવી_Info(pParse, pPk);
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
    જો( pWC->nTerm>1 ){
      પૂર્ણાંક iTerm;
      માટે(iTerm=0; iTerm<pWC->nTerm; iTerm++){
        પદાવલિ *p_પદાવલિ = pWC->a[iTerm].p_પદાવલિ;
        જો( &pWC->a[iTerm] == pTerm ) ચાલુ;
        testcase( pWC->a[iTerm].wtFlags & TERM_VIRTUAL );
        testcase( pWC->a[iTerm].wtFlags & TERM_CODED );
        testcase( pWC->a[iTerm].wtFlags & TERM_SLICE );
        જો( (pWC->a[iTerm].wtFlags & (TERM_VIRTUAL|TERM_CODED|TERM_SLICE))!=0 ){
          ચાલુ;
        }
        જો( (pWC->a[iTerm].eOperator & WO_ALL)==0 ) ચાલુ;
        જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_Subquery) ) ચાલુ;  /* tag-20220303a */
        જો( where_પદાવલિ_Is_Ready(pWInfo, pLevel, pTabItem, p_પદાવલિ)==0 ) ચાલુ;
        p_પદાવલિ = sqlite3_પદાવલિ_Dup(db, p_પદાવલિ, 0);
        p_And_પદાવલિ = sqlite3_પદાવલિ_And(pParse, p_And_પદાવલિ, p_પદાવલિ);
      }
      જો( p_And_પદાવલિ ){
        /* The extra 0x10000 bit on the opcode is masked off and does not
        ** become part of the new Expr.op.  However, it does make the
        ** op==TK_AND comparison inside of sqlite3PExpr() false, and this
        ** prevents sqlite3PExpr() from applying the AND short-circuit
        ** optimization, which we do not want here. */
        p_And_પદાવલિ = sqlite3PExpr(pParse, TK_AND|0x10000, 0, p_And_પદાવલિ);
      }
    }

    /* Run a separate WHERE clause for each term of the OR clause.  After
    ** eliminating duplicates from other WHERE clauses, the action for each
    ** sub-WHERE clause is to to invoke the main loop body as a subroutine.
    */
    ExplainQueryPlan((pParse, 1, "MULTI-INDEX OR"));
    માટે(ii=0; ii<pOrWc->nTerm; ii++){
      WhereTerm *pOrTerm = &pOrWc->a[ii];
      જો( pOrTerm->ડાબું_Cursor==iCur || (pOrTerm->eOperator & WO_AND)!=0 ){
        WhereInfo *pSubWInfo;           /* Info for single OR-term scan */
        પદાવલિ *p_Or_પદાવલિ = pOrTerm->p_પદાવલિ; /* Current OR clause term */
        પદાવલિ *pDelete;                  /* Local copy of OR clause term */
        પૂર્ણાંક jmp1 = 0;                   /* Address of jump operation */
        testcase( (pTabItem[0].fg.jointype & JT_ડાબું)!=0
               && !પદાવલિ_Has_Property(p_Or_પદાવલિ, EP_OuterON)
        ); /* See TH3 vtab25.400 and ticket 614b25314c766238 */
        pDelete = p_Or_પદાવલિ = sqlite3_પદાવલિ_Dup(db, p_Or_પદાવલિ, 0);
        જો( db->mallocFailed ){
          sqlite3_પદાવલિ_Delete(db, pDelete);
          ચાલુ;
        }
        જો( p_And_પદાવલિ ){
          p_And_પદાવલિ->p_ડાબું = p_Or_પદાવલિ;
          p_Or_પદાવલિ = p_And_પદાવલિ;
        }
        /* Loop through table entries that match term pOrTerm. */
        ExplainQueryPlan((pParse, 1, "INDEX %d", ii+1));
        WHERETRACE(0xffffffff, ("Subplan for OR-clause:\n"));
        pSubWInfo = sqlite3WhereBegin(pParse, pOrTab, p_Or_પદાવલિ, 0, 0, 0,
                                      WHERE_OR_SUBCLAUSE, iCovCur);
        પ્રતિજ્ઞા( pSubWInfo || pParse->nErr );
        જો( pSubWInfo ){
          WhereLoop *pSubLoop;
          પૂર્ણાંક addrExplain = sqlite3WhereExplainOneScan(
              pParse, pOrTab, &pSubWInfo->a[0], 0
          );
          sqlite3WhereAddScanStatus(v, pOrTab, &pSubWInfo->a[0], addrExplain);

          /* This is the sub-WHERE clause body.  First skip over
          ** duplicate rows from prior sub-WHERE clauses, and record the
          ** rowid (or PRIMARY KEY) for the current row so that the same
          ** row will be skipped in subsequent sub-WHERE clauses.
          */
          જો( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
            પૂર્ણાંક iSet = ((ii==pOrWc->nTerm-1)?-1:ii);
            જો( HasRowid(pTab) ){
              sqlite3_પદાવલિ_Code_Get_Column_Of_Table(v, pTab, iCur, -1, regRowid);
              jmp1 = sqlite3VdbeAddOp4Int(v, OP_RowSetTest, regRowset, 0,
                                          regRowid, iSet);
              VdbeCoverage(v);
            }નહીંતર{
              Index *pPk = sqlite3_Primary_ચાવી_Index(pTab);
              પૂર્ણાંક nPk = pPk->n_ચાવી_Col;
              પૂર્ણાંક iPk;
              પૂર્ણાંક r;

              /* Read the PK into an array of temp registers. */
              r = sqlite3GetTempRange(pParse, nPk);
              માટે(iPk=0; iPk<nPk; iPk++){
                પૂર્ણાંક iCol = pPk->aiColumn[iPk];
                sqlite3_પદાવલિ_Code_Get_Column_Of_Table(v, pTab, iCur, iCol,r+iPk);
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
              જો( iSet ){
                jmp1 = sqlite3VdbeAddOp4Int(v, OP_Found, regRowset, 0, r, nPk);
                VdbeCoverage(v);
              }
              જો( iSet>=0 ){
                sqlite3VdbeAddOp3(v, OP_MakeRecord, r, nPk, regRowid);
                sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, regRowset, regRowid,
                                     r, nPk);
                જો( iSet ) sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
              }

              /* Release the array of temp registers */
              sqlite3ReleaseTempRange(pParse, r, nPk);
            }
          }

          /* Invoke the main loop body as a subroutine */
          sqlite3VdbeAddOp2(v, OP_Gosub, regReturn, iLoopBody);

          /* Jump here (skipping the main loop body subroutine) if the
          ** current sub-WHERE row is a duplicate from prior sub-WHEREs. */
          જો( jmp1 ) sqlite3VdbeJumpHere(v, jmp1);

          /* The pSubWInfo->untestedTerms flag means that this OR term
          ** contained one or more AND term from a notReady table.  The
          ** terms from the notReady table could not be tested and will
          ** need to be tested later.
          */
          જો( pSubWInfo->untestedTerms ) untestedTerms = 1;

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
          પ્રતિજ્ઞા( (pSubLoop->wsFlags & WHERE_AUTO_INDEX)==0 );
          જો( (pSubLoop->wsFlags & WHERE_INDEXED)!=0
           && (ii==0 || pSubLoop->u.btree.pIndex==pCov)
           && (HasRowid(pTab) || !Is_Primary_ચાવી_Index(pSubLoop->u.btree.pIndex))
          ){
            પ્રતિજ્ઞા( pSubWInfo->a[0].iIdxCur==iCovCur );
            pCov = pSubLoop->u.btree.pIndex;
          }નહીંતર{
            pCov = 0;
          }
          જો( sqlite3WhereUsesDeferredSeek(pSubWInfo) ){
            pWInfo->bDeferredSeek = 1;
          }

          /* Finish the loop through table entries that match term pOrTerm. */
          sqlite3WhereEnd(pSubWInfo);
          Explain_Query_Plan_કાઢો(pParse);
        }
        sqlite3_પદાવલિ_Delete(db, pDelete);
      }
    }
    Explain_Query_Plan_કાઢો(pParse);
    પ્રતિજ્ઞા( pLevel->pWLoop==pLoop );
    પ્રતિજ્ઞા( (pLoop->wsFlags & WHERE_MULTI_OR)!=0 );
    પ્રતિજ્ઞા( (pLoop->wsFlags & WHERE_IN_ABLE)==0 );
    pLevel->u.pCoveringIdx = pCov;
    જો( pCov ) pLevel->iIdxCur = iCovCur;
    જો( p_And_પદાવલિ ){
      p_And_પદાવલિ->p_ડાબું = 0;
      sqlite3_પદાવલિ_Delete(db, p_And_પદાવલિ);
    }
    sqlite3VdbeChangeP1(v, iRetInit, sqlite3VdbeCurrentAddr(v));
    sqlite3VdbeGoto(v, pLevel->addrBrk);
    sqlite3VdbeResolveLabel(v, iLoopBody);

    /* Set the P2 operand of the OP_Return opcode that will end the current
    ** loop to point to this spot, which is the top of the next containing
    ** loop.  The byte-code formatter will use that P2 value as a hint to
    ** indent everything in between the this point and the final OP_Return.
    ** See tag-20220407a in vdbe.c and shell.c */
    પ્રતિજ્ઞા( pLevel->op==OP_Return );
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    જો( pWInfo->pTabList!=pOrTab ){ sqlite3DbFreeNN(db, pOrTab); }
    જો( !untestedTerms ) disableTerm(pLevel, pTerm);
  }નહીંતર
#endif /* SQLITE_OMIT_OR_OPTIMIZATION */

  {
    /* Case 6:  There is no usable index.  We must do a complete
    **          scan of the entire table.
    */
    સ્થિર અચલ u8 aStep[] = { OP_Next, OP_Prev };
    સ્થિર અચલ u8 aStart[] = { OP_Rewind, OP_Last };
    પ્રતિજ્ઞા( bRev==0 || bRev==1 );
    જો( pTabItem->fg.isRecursive ){
      /* Tables marked isRecursive have only a single row that is stored in
      ** a pseudo-cursor.  No need to Rewind or Next such cursors. */
      pLevel->op = OP_Noop;
    }નહીંતર{
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
  કરો{
    પૂર્ણાંક iNext = 0;                /* Next value for iLoop */
    માટે(pTerm=pWC->a, j=pWC->nTerm; j>0; j--, pTerm++){
      પદાવલિ *pE;
      પૂર્ણાંક skipLikeAddr = 0;
      testcase( pTerm->wtFlags & TERM_VIRTUAL );
      testcase( pTerm->wtFlags & TERM_CODED );
      જો( pTerm->wtFlags & (TERM_VIRTUAL|TERM_CODED) ) ચાલુ;
      જો( (pTerm->prereqAll & pLevel->notReady)!=0 ){
        testcase( pWInfo->untestedTerms==0
            && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 );
        pWInfo->untestedTerms = 1;
        ચાલુ;
      }
      pE = pTerm->p_પદાવલિ;
      પ્રતિજ્ઞા( pE!=0 );
      જો( where_પદાવલિ_Is_Ready(pWInfo, pLevel, pTabItem, pE)==0 ) ચાલુ;
      જો( iLoop==1 && !sqlite3_પદાવલિ_Covered_By_Index(pE, pLevel->iTabCur, pIdx) ){
        iNext = 2;
        ચાલુ;
      }
      જો( iLoop<3 && (pTerm->wtFlags & TERM_VARSELECT) ){
        જો( iNext==0 ) iNext = 3;
        ચાલુ;
      }

      જો( (pTerm->wtFlags & TERM_LIKECOND)!=0 ){
        /* If the TERM_LIKECOND flag is set, that means that the range search
        ** is sufficient to guarantee that the LIKE operator is true, so we
        ** can skip the call to the like(A,B) function.  But this only works
        ** for strings.  So do not skip the call to the function on the pass
        ** that compares BLOBs. */
#ifdef SQLITE_LIKE_DOESNT_MATCH_BLOBS
        ચાલુ;
#નહીંતર
        u32 x = pLevel->iLikeRepCntr;
        જો( x>0 ){
          skipLikeAddr = sqlite3VdbeAddOp1(v, (x&1)?OP_IfNot:OP_If,(પૂર્ણાંક)(x>>1));
          VdbeCoverageIf(v, (x&1)==1);
          VdbeCoverageIf(v, (x&1)==0);
        }
#endif
      }
#ifdef WHERETRACE_ENABLED /* 0xffffffff */
      જો( sqlite3WhereTrace ){
        VdbeNoopComment((v, "WhereTerm[%d] (%p) priority=%d",
                         pWC->nTerm-j, pTerm, iLoop));
      }
      જો( sqlite3WhereTrace & 0x4000 ){
        sqlite3DebugPrintf("Coding auxiliary constraint:\n");
        sqlite3WhereTermPrint(pTerm, pWC->nTerm-j);
      }
#endif
      sqlite3_પદાવલિ_If_False(pParse, pE, addrCont, SQLITE_JUMPIFNULL);
      જો( skipLikeAddr ) sqlite3VdbeJumpHere(v, skipLikeAddr);
      pTerm->wtFlags |= TERM_CODED;
    }
    iLoop = iNext;
  }જ્યારે( iLoop>0 );

  /* Insert code to test for implied constraints based on transitivity
  ** of the "==" operator.
  **
  ** Example: If the WHERE clause contains "t1.a=t2.b" and "t2.b=123"
  ** and we are coding the t1 loop and the t2 loop has not yet coded,
  ** then we cannot use the "t1.a=t2.b" constraint, but we can code
  ** the implied "t1.a=123" constraint.
  */
  માટે(pTerm=pWC->a, j=pWC->nBase; j>0; j--, pTerm++){
    પદાવલિ *pE, sEAlt;
    WhereTerm *pAlt;
    જો( pTerm->wtFlags & (TERM_VIRTUAL|TERM_CODED) ) ચાલુ;
    જો( (pTerm->eOperator & (WO_EQ|WO_IS))==0 ) ચાલુ;
    જો( (pTerm->eOperator & WO_EQUIV)==0 ) ચાલુ;
    જો( pTerm->ડાબું_Cursor!=iCur ) ચાલુ;
    જો( pTabItem->fg.jointype & (JT_ડાબું|JT_LTORJ|JT_જમણું) ) ચાલુ;
    pE = pTerm->p_પદાવલિ;
#ifdef WHERETRACE_ENABLED /* 0x4001 */
    જો( (sqlite3WhereTrace & 0x4001)==0x4001 ){
      sqlite3DebugPrintf("Coding transitive constraint:\n");
      sqlite3WhereTermPrint(pTerm, pWC->nTerm-j);
    }
#endif
    પ્રતિજ્ઞા( !પદાવલિ_Has_Property(pE, EP_OuterON) );
    પ્રતિજ્ઞા( (pTerm->prereq_જમણું & pLevel->notReady)!=0 );
    પ્રતિજ્ઞા( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
    pAlt = sqlite3WhereFindTerm(pWC, iCur, pTerm->u.x.ડાબું_Column, notReady,
                    WO_EQ|WO_IN|WO_IS, 0);
    જો( pAlt==0 ) ચાલુ;
    જો( pAlt->wtFlags & (TERM_CODED) ) ચાલુ;
    જો( પદાવલિ_Has_Property(pAlt->p_પદાવલિ, EP_Collate) ) ચાલુ;
    જો( (pAlt->eOperator & WO_IN)
     && પદાવલિ_Use_XSelect(pAlt->p_પદાવલિ)
     && (pAlt->p_પદાવલિ->x.pSelect->pEList->n_પદાવલિ>1)
    ){
      ચાલુ;
    }
    testcase( pAlt->eOperator & WO_EQ );
    testcase( pAlt->eOperator & WO_IS );
    testcase( pAlt->eOperator & WO_IN );
    VdbeModuleComment((v, "begin transitive constraint"));
    sEAlt = *pAlt->p_પદાવલિ;
    sEAlt.p_ડાબું = pE->p_ડાબું;
    sqlite3_પદાવલિ_If_False(pParse, &sEAlt, addrCont, SQLITE_JUMPIFNULL);
    pAlt->wtFlags |= TERM_CODED;
  }

  /* For a RIGHT OUTER JOIN, record the fact that the current row has
  ** been matched at least once.
  */
  જો( pLevel->pRJ ){
    Table *pTab;
    પૂર્ણાંક nPk;
    પૂર્ણાંક r;
    પૂર્ણાંક jmp1 = 0;
    Where_જમણું_Join *pRJ = pLevel->pRJ;

    /* pTab is the right-hand table of the RIGHT JOIN.  Generate code that
    ** will record that the current row of that table has been matched at
    ** least once.  This is accomplished by storing the PK for the row in
    ** both the iMatch index and the regBloom Bloom filter.
    */
    pTab = pWInfo->pTabList->a[pLevel->iFrom].pSTab;
    જો( HasRowid(pTab) ){
      r = sqlite3GetTempRange(pParse, 2);
      sqlite3_પદાવલિ_Code_Get_Column_Of_Table(v, pTab, pLevel->iTabCur, -1, r+1);
      nPk = 1;
    }નહીંતર{
      પૂર્ણાંક iPk;
      Index *pPk = sqlite3_Primary_ચાવી_Index(pTab);
      nPk = pPk->n_ચાવી_Col;
      r = sqlite3GetTempRange(pParse, nPk+1);
      માટે(iPk=0; iPk<nPk; iPk++){
        પૂર્ણાંક iCol = pPk->aiColumn[iPk];
        sqlite3_પદાવલિ_Code_Get_Column_Of_Table(v, pTab, iCur, iCol,r+1+iPk);
      }
    }
    jmp1 = sqlite3VdbeAddOp4Int(v, OP_Found, pRJ->iMatch, 0, r+1, nPk);
    VdbeCoverage(v);
    VdbeComment((v, "match against %s", pTab->zName));
    sqlite3VdbeAddOp3(v, OP_MakeRecord, r+1, nPk, r);
    sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, pRJ->iMatch, r, r+1, nPk);
    જો( pRJ->regBloom ){
      sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pRJ->regBloom, 0, r+1, nPk);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
    }
    sqlite3VdbeJumpHere(v, jmp1);
    sqlite3ReleaseTempRange(pParse, r, nPk+1);
  }

  /* For a LEFT OUTER JOIN, generate code that will record the fact that
  ** at least one row of the right table has matched the left table.
  */
  જો( pLevel->i_ડાબું_Join ){
    pLevel->addrFirst = sqlite3VdbeCurrentAddr(v);
    sqlite3VdbeAddOp2(v, OP_Integer, 1, pLevel->i_ડાબું_Join);
    VdbeComment((v, "record LEFT JOIN hit"));
    જો( pLevel->pRJ==0 ){
      જાઓ code_outer_join_constraints; /* WHERE clause constraints */
    }
  }

  જો( pLevel->pRJ ){
    /* Create a subroutine used to process all interior loops and code
    ** of the RIGHT JOIN.  During normal operation, the subroutine will
    ** be in-line with the rest of the code.  But at the end, a separate
    ** loop will run that invokes this subroutine for unmatched rows
    ** of pTab, with all tables to left begin set to NULL.
    */
    {
      Where_જમણું_Join *pRJ = pLevel->pRJ;
      sqlite3VdbeAddOp2(v, OP_BeginSubrtn, 0, pRJ->regReturn);
      pRJ->addrSubrtn = sqlite3VdbeCurrentAddr(v);
    }
    પ્રતિજ્ઞા( pParse->withinRJSubrtn < 255 );
    pParse->withinRJSubrtn++;

    /* WHERE clause constraints must be deferred until after outer join
    ** row elimination has completed, since WHERE clause constraints apply
    ** to the results of the OUTER JOIN.  The following loop generates the
    ** appropriate WHERE clause constraint checks.  tag-20220513a.
    */
  code_outer_join_constraints:
    માટે(pTerm=pWC->a, j=0; j<pWC->nBase; j++, pTerm++){
      testcase( pTerm->wtFlags & TERM_VIRTUAL );
      testcase( pTerm->wtFlags & TERM_CODED );
      જો( pTerm->wtFlags & (TERM_VIRTUAL|TERM_CODED) ) ચાલુ;
      જો( (pTerm->prereqAll & pLevel->notReady)!=0 ){
        પ્રતિજ્ઞા( pWInfo->untestedTerms );
        ચાલુ;
      }
      જો( pTabItem->fg.jointype & JT_LTORJ ) ચાલુ;
      પ્રતિજ્ઞા( pTerm->p_પદાવલિ );
      sqlite3_પદાવલિ_If_False(pParse, pTerm->p_પદાવલિ, addrCont, SQLITE_JUMPIFNULL);
      pTerm->wtFlags |= TERM_CODED;
    }
  }

#જો WHERETRACE_ENABLED /* 0x4001 */
  જો( sqlite3WhereTrace & 0x4000 ){
    sqlite3DebugPrintf("All WHERE-clause terms after coding level %d:\n",
                       iLevel);
    sqlite3WhereClausePrint(pWC);
  }
  જો( sqlite3WhereTrace & 0x1 ){
    sqlite3DebugPrintf("End Coding level %d:  notReady=%llx\n",
       iLevel, (u64)pLevel->notReady);
  }
#endif
  પાછા pLevel->notReady;
}

/*
** Generate the code for the loop that finds all non-matched terms
** for a RIGHT JOIN.
*/
SQLITE_NOINLINE શૂન્ય sqlite3_Where_જમણું_Join_Loop(
  WhereInfo *pWInfo,
  પૂર્ણાંક iLevel,
  WhereLevel *pLevel
){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  Where_જમણું_Join *pRJ = pLevel->pRJ;
  પદાવલિ *pSubWhere = 0;
  WhereClause *pWC = &pWInfo->sWC;
  WhereInfo *pSubWInfo;
  WhereLoop *pLoop = pLevel->pWLoop;
  SrcItem *pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
  SrcList *pFrom;
  સંઘ {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;
  Bitmask mAll = 0;
  પૂર્ણાંક k;

  ExplainQueryPlan((pParse, 1, "RIGHT-JOIN %s", pTabItem->pSTab->zName));
  sqlite3VdbeNoJumpsOutsideSubrtn(v, pRJ->addrSubrtn, pRJ->endSubrtn,
                                  pRJ->regReturn);
  માટે(k=0; k<iLevel; k++){
    પૂર્ણાંક iIdxCur;
    SrcItem *p_જમણું;
    પ્રતિજ્ઞા( pWInfo->a[k].pWLoop->iTab == pWInfo->a[k].iFrom );
    p_જમણું = &pWInfo->pTabList->a[pWInfo->a[k].iFrom];
    mAll |= pWInfo->a[k].pWLoop->maskSelf;
    જો( p_જમણું->fg.viaCoroutine ){
      Subquery *pSubq;
      પ્રતિજ્ઞા( p_જમણું->fg.isSubquery && p_જમણું->u4.pSubq!=0 );
      pSubq = p_જમણું->u4.pSubq;
      પ્રતિજ્ઞા( pSubq->pSelect!=0 && pSubq->pSelect->pEList!=0 );
      sqlite3VdbeAddOp3(
          v, OP_Null, 0, pSubq->regResult,
          pSubq->regResult + pSubq->pSelect->pEList->n_પદાવલિ-1
      );
    }
    sqlite3VdbeAddOp1(v, OP_NullRow, pWInfo->a[k].iTabCur);
    iIdxCur = pWInfo->a[k].iIdxCur;
    જો( iIdxCur ){
      sqlite3VdbeAddOp1(v, OP_NullRow, iIdxCur);
    }
  }
  જો( (pTabItem->fg.jointype & JT_LTORJ)==0 ){
    mAll |= pLoop->maskSelf;
    માટે(k=0; k<pWC->nTerm; k++){
      WhereTerm *pTerm = &pWC->a[k];
      જો( (pTerm->wtFlags & (TERM_VIRTUAL|TERM_SLICE))!=0
       && pTerm->eOperator!=WO_ROWVAL
      ){
        તોડો;
      }
      જો( pTerm->prereqAll & ~mAll ) ચાલુ;
      જો( પદાવલિ_Has_Property(pTerm->p_પદાવલિ, EP_OuterON|EP_InnerON) ) ચાલુ;
      pSubWhere = sqlite3_પદાવલિ_And(pParse, pSubWhere,
                                 sqlite3_પદાવલિ_Dup(pParse->db, pTerm->p_પદાવલિ, 0));
    }
  }
  જો( pLevel->iIdxCur ){
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
  memcpy(&pFrom->a[0], pTabItem, કદ(SrcItem));
  pFrom->a[0].fg.jointype = 0;
  જો( pParse->withinRJSubrtn >= 100 ){
    /* This limit --------------^^^
    ** is based on an historical assert().  It is not compile-time or
    ** run-time configurable.  It could perhaps be raised as high as 254,
    ** but only an attack robot would ever do even 100 RIGHT JOINS within
    ** a single query, so we'll just leave it as it is. */
    sqlite3_Error_સંદેશ(pParse, "too many RIGHT JOINs");
    પાછા;
  }
  pParse->withinRJSubrtn++;
  pSubWInfo = sqlite3WhereBegin(pParse, pFrom, pSubWhere, 0, 0, 0,
                                WHERE_જમણું_JOIN, 0);
  જો( pSubWInfo ){
    પૂર્ણાંક iCur = pLevel->iTabCur;  /* Table on RHS of RIGHT JOIN &*/
    પૂર્ણાંક r = ++pParse->nMem;      /* Register range to hold primary key */
    પૂર્ણાંક nPk;                     /* Number of values in the primary key */
    પૂર્ણાંક r2;                      /* Register holding record for primary key */
    પૂર્ણાંક addrCont = sqlite3WhereContinueLabel(pSubWInfo);
    Table *pTab = pTabItem->pSTab;  /* Table on RHS of RIGHT JOIN */
    જો( HasRowid(pTab) ){
      sqlite3_પદાવલિ_Code_Get_Column_Of_Table(v, pTab, iCur, -1, r);
      nPk = 1;
    }નહીંતર{
      પૂર્ણાંક iPk;
      Index *pPk = sqlite3_Primary_ચાવી_Index(pTab);
      nPk = pPk->n_ચાવી_Col;
      pParse->nMem += nPk - 1;
      માટે(iPk=0; iPk<nPk; iPk++){
        પૂર્ણાંક iCol = pPk->aiColumn[iPk];
        sqlite3_પદાવલિ_Code_Get_Column_Of_Table(v, pTab, iCur, iCol,r+iPk);
      }
    }

    /* Generate code that checks to see if the current row of the RHS table
    ** has appeared in any prior output row. */
    જો( pRJ->regBloom ){
      sqlite3VdbeAddOp4Int(v, OP_Filter, pRJ->regBloom, 
                 sqlite3VdbeCurrentAddr(v)+2, r, nPk);
      VdbeCoverage(v);
    }
    sqlite3VdbeAddOp4Int(v, OP_Found, pRJ->iMatch, addrCont, r, nPk);
    VdbeCoverage(v);
    r2 = sqlite3GetTempReg(pParse);

    /* Generate code that inserts the PK of the RHS table into the
    ** pRH->iMatch index to indicate that the current row has appeared
    ** in the output set. */
    sqlite3VdbeAddOp3(v, OP_MakeRecord, r, nPk, r2);
    sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, pRJ->iMatch, r2, r, nPk);
    sqlite3ReleaseTempReg(pParse, r2);
    જો( pRJ->regBloom ){
      sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pRJ->regBloom, 0, r, nPk);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
    }

    /* Invoke the subroutine that actually puts the current RHS table row
    ** into the output set, with NULLs for the LHS. */
    sqlite3VdbeAddOp2(v, OP_Gosub, pRJ->regReturn, pRJ->addrSubrtn);
    sqlite3WhereEnd(pSubWInfo);
  }
  sqlite3_પદાવલિ_Delete(pParse->db, pSubWhere);
  Explain_Query_Plan_કાઢો(pParse);
  પ્રતિજ્ઞા( pParse->withinRJSubrtn>0 );
  pParse->withinRJSubrtn--;
}
