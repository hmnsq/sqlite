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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "whereInt.h"

#నిర్వచితం_కాకపోతే SQLITE_OMIT_EXPLAIN

/*
** Return the name of the i-th column of the pIdx index.
*/
స్థిర స్థిరరాశి అక్షరం *explain_సూచీ_స్తంభం_Name(సూచీ *pIdx, పూర్ణాంకం i){
  i = pIdx->ai_స్తంభం[i];
  ఒకవేళ( i==XN_సమాసం ) తిరిగి "<expr>";
  ఒకవేళ( i==XN_ROWID ) తిరిగి "rowid";
  తిరిగి pIdx->p_పట్టిక->a_నిలువు[i].zCnName;
}

/*
** This routine is a helper for explainIndexRange() below
**
** pStr holds the text of an expression that we are building up one term
** at a time.  This routine adds a new term to the end of the expression.
** Terms are separated by AND so add the "AND" text for second and subsequent
** terms only.
*/
స్థిర శూన్య explain_Append_పదం(
  StrAccum *pStr,             /* The text expression being built */
  సూచీ *pIdx,                /* Index to read column names from */
  పూర్ణాంకం n_పదం,                  /* Number of terms */
  పూర్ణాంకం i_పదం,                  /* Zero-based index of first term. */
  పూర్ణాంకం bAnd,                   /* Non-zero to append " AND " */
  స్థిరరాశి అక్షరం *zOp             /* Name of the operator */
){
  పూర్ణాంకం i;

  ప్రతిజ్ఞా( n_పదం>=1 );
  ఒకవేళ( bAnd ) sqlite3_str_append(pStr, " AND ", 5);

  ఒకవేళ( n_పదం>1 ) sqlite3_str_append(pStr, "(", 1);
  కొరకు(i=0; i<n_పదం; i++){
    ఒకవేళ( i ) sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_appendall(pStr, explain_సూచీ_స్తంభం_Name(pIdx, i_పదం+i));
  }
  ఒకవేళ( n_పదం>1 ) sqlite3_str_append(pStr, ")", 1);

  sqlite3_str_append(pStr, zOp, 1);

  ఒకవేళ( n_పదం>1 ) sqlite3_str_append(pStr, "(", 1);
  కొరకు(i=0; i<n_పదం; i++){
    ఒకవేళ( i ) sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_append(pStr, "?", 1);
  }
  ఒకవేళ( n_పదం>1 ) sqlite3_str_append(pStr, ")", 1);
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
స్థిర శూన్య explain_సూచీ_Range(StrAccum *pStr, WhereLoop *pLoop){
  సూచీ *p_సూచీ = pLoop->u.btree.p_సూచీ;
  u16 nEq = pLoop->u.btree.nEq;
  u16 nSkip = pLoop->nSkip;
  పూర్ణాంకం i, j;

  ఒకవేళ( nEq==0 && (pLoop->wsFlags&(WHERE_BTM_హద్దు|WHERE_శిఖరం_హద్దు))==0 ) తిరిగి;
  sqlite3_str_append(pStr, " (", 2);
  కొరకు(i=0; i<nEq; i++){
    స్థిరరాశి అక్షరం *z = explain_సూచీ_స్తంభం_Name(p_సూచీ, i);
    ఒకవేళ( i ) sqlite3_str_append(pStr, " AND ", 5);
    sqlite3_str_appendf(pStr, i>=nSkip ? "%s=?" : "ANY(%s)", z);
  }

  j = i;
  ఒకవేళ( pLoop->wsFlags&WHERE_BTM_హద్దు ){
    explain_Append_పదం(pStr, p_సూచీ, pLoop->u.btree.nBtm, j, i, ">");
    i = 1;
  }
  ఒకవేళ( pLoop->wsFlags&WHERE_శిఖరం_హద్దు ){
    explain_Append_పదం(pStr, p_సూచీ, pLoop->u.btree.n_శిఖరం, j, i, "<");
  }
  sqlite3_str_append(pStr, ")", 1);
}

/*
** This function sets the P4 value of an existing OP_Explain opcode to
** text describing the loop in pLevel. If the OP_Explain opcode already has
** a P4 value, it is freed before it is overwritten.
*/
శూన్య sqlite3_Where_కూడు_Explain_పాఠ్యం(
  Parse *pParse,                  /* Parse context */
  పూర్ణాంకం addr,                       /* Address of OP_Explain opcode */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
#ఒకవేళ !defined(SQLITE_DEBUG)
  ఒకవేళ( sqlite3ParseToplevel(pParse)->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#అయితే_ముగింపు
  {
    VdbeOp *pOp = sqlite3VdbeGetOp(pParse->pVdbe, addr);
    Src_అంశం *p_అంశం = &pTabList->a[pLevel->iFrom];
    sqlite3 *db = pParse->db;     /* Database handle */
    పూర్ణాంకం is_శోధన;                 /* True for a SEARCH. False for SCAN. */
    WhereLoop *pLoop;             /* The controlling WhereLoop object */
    u32 flags;                    /* Flags that describe this loop */
#ఒకవేళ defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_EXPLAIN)
    అక్షరం *z_సందేశం;                   /* Text to add to EQP output */
#అయితే_ముగింపు
    StrAccum str;                 /* EQP output string */
    అక్షరం zBuf[100];               /* Initial space for EQP output string */

    ఒకవేళ( db->mallocFailed ) తిరిగి;

    pLoop = pLevel->pWLoop;
    flags = pLoop->wsFlags;

    is_శోధన = (flags&(WHERE_BTM_హద్దు|WHERE_శిఖరం_హద్దు))!=0
            || ((flags&WHERE_VIRTUALTABLE)==0 && (pLoop->u.btree.nEq>0))
            || (wctrlFlags&(WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX));

    sqlite3StrAccumInit(&str, db, zBuf, పరిమాణం(zBuf), SQLITE_MAX_నిడివి);
    str.printfFlags = SQLITE_PRINTF_అంతర్గత;
    sqlite3_str_appendf(&str, "%s %S%s",
       is_శోధన ? "SEARCH" : "SCAN",
       p_అంశం,
       p_అంశం->fg.fromExists ? " EXISTS" : "");
    ఒకవేళ( (flags & (WHERE_IPK|WHERE_VIRTUALTABLE))==0 ){
      స్థిరరాశి అక్షరం *zFmt = 0;
      సూచీ *pIdx;

      ప్రతిజ్ఞా( pLoop->u.btree.p_సూచీ!=0 );
      pIdx = pLoop->u.btree.p_సూచీ;
      ప్రతిజ్ఞా( !(flags&WHERE_AUTO_సూచీ) || (flags&WHERE_IDX_ONLY) );
      ఒకవేళ( !HasRowid(p_అంశం->pSTab) && Is_Primary_కీలకం_సూచీ(pIdx) ){
        ఒకవేళ( is_శోధన ){
          zFmt = "PRIMARY KEY";
        }
      }లేకపోతే ఒకవేళ( flags & WHERE_PARTIALIDX ){
        zFmt = "AUTOMATIC PARTIAL COVERING INDEX";
      }లేకపోతే ఒకవేళ( flags & WHERE_AUTO_సూచీ ){
        zFmt = "AUTOMATIC COVERING INDEX";
      }లేకపోతే ఒకవేళ( flags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
        zFmt = "COVERING INDEX %s";
      }లేకపోతే{
        zFmt = "INDEX %s";
      }
      ఒకవేళ( zFmt ){
        sqlite3_str_append(&str, " USING ", 7);
        sqlite3_str_appendf(&str, zFmt, pIdx->zName);
        explain_సూచీ_Range(&str, pLoop);
      }
    }లేకపోతే ఒకవేళ( (flags & WHERE_IPK)!=0 && (flags & WHERE_CONSTRAINT)!=0 ){
      అక్షరం cRangeOp;
#ఒకవేళ 0  /* Better output, but breaks many tests */
      స్థిరరాశి పట్టిక *pTab = p_అంశం->pTab;
      స్థిరరాశి అక్షరం *zRowid = pTab->iPKey>=0 ? pTab->a_నిలువు[pTab->iPKey].zCnName:
                              "rowid";
#లేకపోతే
      స్థిరరాశి అక్షరం *zRowid = "rowid";
#అయితే_ముగింపు
      sqlite3_str_appendf(&str, " USING INTEGER PRIMARY KEY (%s", zRowid);
      ఒకవేళ( flags&(WHERE_స్తంభం_EQ|WHERE_స్తంభం_లోపల) ){
        cRangeOp = '=';
      }లేకపోతే ఒకవేళ( (flags&WHERE_BOTH_హద్దు)==WHERE_BOTH_హద్దు ){
        sqlite3_str_appendf(&str, ">? AND %s", zRowid);
        cRangeOp = '<';
      }లేకపోతే ఒకవేళ( flags&WHERE_BTM_హద్దు ){
        cRangeOp = '>';
      }లేకపోతే{
        ప్రతిజ్ఞా( flags&WHERE_శిఖరం_హద్దు);
        cRangeOp = '<';
      }
      sqlite3_str_appendf(&str, "%c?)", cRangeOp);
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
    లేకపోతే ఒకవేళ( (flags & WHERE_VIRTUALTABLE)!=0 ){
      sqlite3_str_appendall(&str, " VIRTUAL TABLE INDEX ");
      sqlite3_str_appendf(&str,
                  pLoop->u.vtab.b_Idx_అంకె_Hex ? "0x%x:%s" : "%d:%s",
                  pLoop->u.vtab.idx_అంకె, pLoop->u.vtab.idxStr);
    }
#అయితే_ముగింపు
    ఒకవేళ( p_అంశం->fg.jointype & JT_ఎడమ ){
      sqlite3_str_appendf(&str, " LEFT-JOIN");
    }
#నిర్వచితమైతే SQLITE_EXPLAIN_ESTIMATED_అడ్డువరుసలు
    ఒకవేళ( pLoop->n_బహిర్గతం>=10 ){
      sqlite3_str_appendf(&str, " (~%llu rows)",
             sqlite3_Log_Est_గమ్యం_Int(pLoop->n_బహిర్గతం));
    }లేకపోతే{
      sqlite3_str_append(&str, " (~1 row)", 9);
    }
#అయితే_ముగింపు
#ఒకవేళ defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_EXPLAIN)
    z_సందేశం = sqlite3_Str_Accum_ముగింపు(&str);
    sqlite3ExplainBreakpoint("",z_సందేశం);
#అయితే_ముగింపు

    ప్రతిజ్ఞా( pOp->opcode==OP_Explain );
    ప్రతిజ్ఞా( pOp->p4type==P4_DYNAMIC || pOp->p4.z==0 );
    sqlite3DbFree(db, pOp->p4.z);
    pOp->p4type = P4_DYNAMIC;
    pOp->p4.z = sqlite3_Str_Accum_ముగింపు(&str);
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
పూర్ణాంకం sqlite3_Where_Explain_One_పరిశీలన(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
  పూర్ణాంకం ret = 0;
#ఒకవేళ !defined(SQLITE_DEBUG)
  ఒకవేళ( sqlite3ParseToplevel(pParse)->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#అయితే_ముగింపు
  {
    ఒకవేళ( (pLevel->pWLoop->wsFlags & WHERE_MULTI_OR)==0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
    ){
      Vdbe *v = pParse->pVdbe;
      పూర్ణాంకం addr = sqlite3VdbeCurrentAddr(v);
      ret = sqlite3_Vdbe_కూడు_Op3(
          v, OP_Explain, addr, pParse->addrExplain, pLevel->pWLoop->r_నిరంతరం
      );
      sqlite3_Where_కూడు_Explain_పాఠ్యం(pParse, addr, pTabList, pLevel, wctrlFlags);
    }
  }
  తిరిగి ret;
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
పూర్ణాంకం sqlite3WhereExplainBloomFilter(
  స్థిరరాశి Parse *pParse,               /* Parse context */
  స్థిరరాశి WhereInfo *pWInfo,           /* WHERE clause */
  స్థిరరాశి WhereLevel *pLevel           /* Bloom filter on this level */
){
  పూర్ణాంకం ret = 0;
  Src_అంశం *p_అంశం = &pWInfo->pTabList->a[pLevel->iFrom];
  Vdbe *v = pParse->pVdbe;      /* VM being constructed */
  sqlite3 *db = pParse->db;     /* Database handle */
  అక్షరం *z_సందేశం;                   /* Text to add to EQP output */
  పూర్ణాంకం i;                        /* Loop counter */
  WhereLoop *pLoop;             /* The where loop */
  StrAccum str;                 /* EQP output string */
  అక్షరం zBuf[100];               /* Initial space for EQP output string */

  sqlite3StrAccumInit(&str, db, zBuf, పరిమాణం(zBuf), SQLITE_MAX_నిడివి);
  str.printfFlags = SQLITE_PRINTF_అంతర్గత;
  sqlite3_str_appendf(&str, "BLOOM FILTER ON %S (", p_అంశం);
  pLoop = pLevel->pWLoop;
  ఒకవేళ( pLoop->wsFlags & WHERE_IPK ){
    స్థిరరాశి పట్టిక *pTab = p_అంశం->pSTab;
    ఒకవేళ( pTab->iPKey>=0 ){
      sqlite3_str_appendf(&str, "%s=?", pTab->a_నిలువు[pTab->iPKey].zCnName);
    }లేకపోతే{
      sqlite3_str_appendf(&str, "rowid=?");
    }
  }లేకపోతే{
    కొరకు(i=pLoop->nSkip; i<pLoop->u.btree.nEq; i++){
      స్థిరరాశి అక్షరం *z = explain_సూచీ_స్తంభం_Name(pLoop->u.btree.p_సూచీ, i);
      ఒకవేళ( i>pLoop->nSkip ) sqlite3_str_append(&str, " AND ", 5);
      sqlite3_str_appendf(&str, "%s=?", z);
    }
  }
  sqlite3_str_append(&str, ")", 1);
  z_సందేశం = sqlite3_Str_Accum_ముగింపు(&str);
  ret = sqlite3_Vdbe_కూడు_Op4(v, OP_Explain, sqlite3VdbeCurrentAddr(v),
                          pParse->addrExplain, 0, z_సందేశం,P4_DYNAMIC);

  sqlite3_Vdbe_పరిశీలన_స్థితి(v, sqlite3VdbeCurrentAddr(v)-1, 0, 0, 0, 0);
  తిరిగి ret;
}
#అయితే_ముగింపు /* SQLITE_OMIT_EXPLAIN */

#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Configure the VM passed as the first argument with an
** sqlite3_stmt_scanstatus() entry corresponding to the scan used to
** implement level pLvl. Argument pSrclist is a pointer to the FROM
** clause that the scan reads data from.
**
** If argument addrExplain is not 0, it must be the address of an
** OP_Explain instruction that describes the same loop.
*/
శూన్య sqlite3_Where_కూడు_పరిశీలన_స్థితి(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  పూర్ణాంకం addrExplain                 /* Address of OP_Explain (or 0) */
){
  ఒకవేళ( IS_STMT_SCANSTATUS( sqlite3VdbeDb(v) ) ){
    స్థిరరాశి అక్షరం *zObj = 0;
    WhereLoop *pLoop = pLvl->pWLoop;
    పూర్ణాంకం wsFlags = pLoop->wsFlags;
    పూర్ణాంకం ద్వారా_Coroutine = 0;

    ఒకవేళ( (wsFlags & WHERE_VIRTUALTABLE)==0  &&  pLoop->u.btree.p_సూచీ!=0 ){
      zObj = pLoop->u.btree.p_సూచీ->zName;
    }లేకపోతే{
      zObj = pSrclist->a[pLvl->iFrom].zName;
      ద్వారా_Coroutine = pSrclist->a[pLvl->iFrom].fg.ద్వారా_Coroutine;
    }
    sqlite3_Vdbe_పరిశీలన_స్థితి(
        v, addrExplain, pLvl->addrBody, pLvl->addr_సందర్శించు, pLoop->n_బహిర్గతం, zObj
    );

    ఒకవేళ( ద్వారా_Coroutine==0 ){
      ఒకవేళ( (wsFlags & (WHERE_MULTI_OR|WHERE_AUTO_సూచీ))==0 ){
        sqlite3_Vdbe_పరిశీలన_స్థితి_Range(v, addrExplain, -1, pLvl->iTabCur);
      }
      ఒకవేళ( wsFlags & WHERE_సూచీకృత ){
        sqlite3_Vdbe_పరిశీలన_స్థితి_Range(v, addrExplain, -1, pLvl->iIdxCur);
      }
    }లేకపోతే{
      పూర్ణాంకం addr;
      VdbeOp *pOp;
      ప్రతిజ్ఞా( pSrclist->a[pLvl->iFrom].fg.isSubquery );
      addr = pSrclist->a[pLvl->iFrom].u4.pSubq->addrFillSub;
      pOp = sqlite3VdbeGetOp(v, addr-1);
      ప్రతిజ్ఞా( sqlite3VdbeDb(v)->mallocFailed || pOp->opcode==OP_InitCoroutine );
      ప్రతిజ్ఞా( sqlite3VdbeDb(v)->mallocFailed || pOp->p2>addr );
      sqlite3_Vdbe_పరిశీలన_స్థితి_Range(v, addrExplain, addr, pOp->p2-1);
    }
  }
}
#అయితే_ముగింపు


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
స్థిర శూన్య disable_పదం(WhereLevel *pLevel, Where_పదం *p_పదం){
  పూర్ణాంకం nLoop = 0;
  ప్రతిజ్ఞా( p_పదం!=0 );
  అయితే( (p_పదం->wtFlags & పదం_CODED)==0
      && (pLevel->i_ఎడమ_Join==0 || సమాసం_Has_Property(p_పదం->p_సమాసం, EP_OuterON))
      && (pLevel->notReady & p_పదం->prereqAll)==0
  ){
    ఒకవేళ( nLoop && (p_పదం->wtFlags & పదం_LIKE)!=0 ){
      p_పదం->wtFlags |= పదం_LIKECOND;
    }లేకపోతే{
      p_పదం->wtFlags |= పదం_CODED;
    }
#నిర్వచితమైతే WHERETRACE_ENABLED
    ఒకవేళ( (sqlite3WhereTrace & 0x4001)==0x4001 ){
      sqlite3DebugPrintf("DISABLE-");
      sqlite3_Where_పదం_Print(p_పదం, (పూర్ణాంకం)(p_పదం - (p_పదం->pWC->a)));
    }
#అయితే_ముగింపు
    ఒకవేళ( p_పదం->i_మాతృ<0 ) ఆపు;
    p_పదం = &p_పదం->pWC->a[p_పదం->i_మాతృ];
    ప్రతిజ్ఞా( p_పదం!=0 );
    p_పదం->n_బిడ్డ--;
    ఒకవేళ( p_పదం->n_బిడ్డ!=0 ) ఆపు;
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
స్థిర శూన్య సంకేతం_Apply_Affinity(Parse *pParse, పూర్ణాంకం పునాది, పూర్ణాంకం n, అక్షరం *zAff){
  Vdbe *v = pParse->pVdbe;
  ఒకవేళ( zAff==0 ){
    ప్రతిజ్ఞా( pParse->db->mallocFailed );
    తిరిగి;
  }
  ప్రతిజ్ఞా( v!=0 );

  /* Adjust base and n to skip over SQLITE_AFF_BLOB and SQLITE_AFF_NONE
  ** entries at the beginning and end of the affinity string.
  */
  ప్రతిజ్ఞా( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  అయితే( n>0 && zAff[0]<=SQLITE_AFF_BLOB ){
    n--;
    పునాది++;
    zAff++;
  }
  అయితే( n>1 && zAff[n-1]<=SQLITE_AFF_BLOB ){
    n--;
  }

  /* Code the OP_Affinity opcode if there is anything left to do. */
  ఒకవేళ( n>0 ){
    sqlite3_Vdbe_కూడు_Op4(v, OP_Affinity, పునాది, n, 0, zAff, n);
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
స్థిర శూన్య updateRangeAffinityStr(
  సమాసం *p_కుడి,                   /* RHS of comparison */
  పూర్ణాంకం n,                          /* Number of vector elements in comparison */
  అక్షరం *zAff                      /* Affinity string to modify */
){
  పూర్ణాంకం i;
  కొరకు(i=0; i<n; i++){
    సమాసం *p = sqlite3_సదిశ_క్షేత్రం_Subexpr(p_కుడి, i);
    ఒకవేళ( sqlite3CompareAffinity(p, zAff[i])==SQLITE_AFF_BLOB
     || sqlite3_సమాసం_Needs_No_Affinity_Change(p, zAff[i])
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
స్థిర శూన్య adjust_క్రమం_By_నిలువు(సమాసం_List *p_క్రమం_By, సమాసం_List *pEList){
  పూర్ణాంకం i, j;
  ఒకవేళ( p_క్రమం_By==0 ) తిరిగి;
  కొరకు(i=0; i<p_క్రమం_By->n_సమాసం; i++){
    పూర్ణాంకం t = p_క్రమం_By->a[i].u.x.i_క్రమం_By_నిలువు;
    ఒకవేళ( t==0 ) కొనసాగు;
    కొరకు(j=0; j<pEList->n_సమాసం; j++){
      ఒకవేళ( pEList->a[j].u.x.i_క్రమం_By_నిలువు==t ){
        p_క్రమం_By->a[i].u.x.i_క్రమం_By_నిలువు = j+1;
        ఆపు;
      }
    }
    ఒకవేళ( j>=pEList->n_సమాసం ){
      p_క్రమం_By->a[i].u.x.i_క్రమం_By_నిలువు = 0;
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
స్థిర సమాసం *remove_Unindexable_లోపల_Clause_Terms(
  Parse *pParse,        /* The parsing context */
  పూర్ణాంకం iEq,              /* Look at loop terms starting here */
  WhereLoop *pLoop,     /* The current loop */
  సమాసం *pX              /* The IN expression to be reduced */
){
  sqlite3 *db = pParse->db;
  Select *pSelect;            /* Pointer to the SELECT on the RHS */
  సమాసం *pNew;
  pNew = sqlite3_సమాసం_Dup(db, pX, 0);
  ఒకవేళ( db->mallocFailed==0 ){
    కొరకు(pSelect=pNew->x.pSelect; pSelect; pSelect=pSelect->p_పూర్వ){
      సమాసం_List *pOrigRhs;         /* Original unmodified RHS */
      సమాసం_List *pOrigLhs = 0;     /* Original unmodified LHS */
      సమాసం_List *pRhs = 0;         /* New RHS after modifications */
      సమాసం_List *pLhs = 0;         /* New LHS after mods */
      పూర్ణాంకం i;                      /* Loop counter */
      పూర్ణాంకం nRhs = 0;               /* Number of RHS terms added so far */

      ప్రతిజ్ఞా( సమాసం_Use_XSelect(pNew) );
      pOrigRhs = pSelect->pEList;
      ప్రతిజ్ఞా( pNew->p_ఎడమ!=0 );
      ప్రతిజ్ఞా( సమాసం_Use_XList(pNew->p_ఎడమ) );
      ఒకవేళ( pSelect==pNew->x.pSelect ){
        pOrigLhs = pNew->p_ఎడమ->x.pList;
      }
      కొరకు(i=iEq; i<pLoop->nLTerm; i++){
        ఒకవేళ( pLoop->aLTerm[i]->p_సమాసం==pX ){
          పూర్ణాంకం i_క్షేత్రం;
          ప్రతిజ్ఞా( (pLoop->aLTerm[i]->eOperator & (WO_OR|WO_AND))==0 );
          i_క్షేత్రం = pLoop->aLTerm[i]->u.x.i_క్షేత్రం - 1;
          ఒకవేళ( i_క్షేత్రం!=nRhs ){
            సమాసం_Clear_Property(pNew, EP_Subrtn);
          }
          ఒకవేళ( NEVER(pOrigRhs->a[i_క్షేత్రం].p_సమాసం==0) ){
            కొనసాగు; /* Duplicate PK column */
          }
          pRhs = sqlite3_సమాసం_List_Append(pParse, pRhs, pOrigRhs->a[i_క్షేత్రం].p_సమాసం);
          pOrigRhs->a[i_క్షేత్రం].p_సమాసం = 0;
          ఒకవేళ( pRhs ) pRhs->a[pRhs->n_సమాసం-1].u.x.i_క్రమం_By_నిలువు = i_క్షేత్రం+1;
          ఒకవేళ( pOrigLhs ){
            ప్రతిజ్ఞా( pOrigLhs->a[i_క్షేత్రం].p_సమాసం!=0 );
            pLhs = sqlite3_సమాసం_List_Append(pParse,pLhs,pOrigLhs->a[i_క్షేత్రం].p_సమాసం);
            pOrigLhs->a[i_క్షేత్రం].p_సమాసం = 0;
          }
          nRhs++;
        }
      }
      ఒకవేళ( nRhs<pOrigRhs->n_సమాసం ){
        సమాసం_Clear_Property(pNew, EP_Subrtn); /* See bug 2026-10-16T22:07:15Z */
      }
      sqlite3_సమాసం_List_Delete(db, pOrigRhs);
      ఒకవేళ( pOrigLhs ){
        sqlite3_సమాసం_List_Delete(db, pOrigLhs);
        pNew->p_ఎడమ->x.pList = pLhs;
      }
      pSelect->pEList = pRhs;
      pSelect->selId = ++pParse->nSelect; /* Req'd for SubrtnSig validity */
      ఒకవేళ( pLhs && pLhs->n_సమాసం==1 ){
        /* Take care here not to generate a TK_VECTOR containing only a
        ** single value. Since the parser never creates such a vector, some
        ** of the subroutines do not handle this case.  */
        సమాసం *p = pLhs->a[0].p_సమాసం;
        pLhs->a[0].p_సమాసం = 0;
        sqlite3_సమాసం_Delete(db, pNew->p_ఎడమ);
        pNew->p_ఎడమ = p;
      }

      /* If either the ORDER BY clause or the GROUP BY clause contains
      ** references to result-set columns, those references might now be
      ** obsolete.  So fix them up.
      */
      ప్రతిజ్ఞా( pRhs!=0 || db->mallocFailed );
      ఒకవేళ( pRhs ){
        adjust_క్రమం_By_నిలువు(pSelect->p_క్రమం_By, pRhs);
        adjust_క్రమం_By_నిలువు(pSelect->p_సమూహం_By, pRhs);
        కొరకు(i=0; i<pRhs->n_సమాసం; i++) pRhs->a[i].u.x.i_క్రమం_By_నిలువు = 0;
      }

#ఒకవేళ 0
      రాయి("For indexing, change the IN expr:\n");
      sqlite3_వృక్షం_View_సమాసం(0, pX, 0);
      రాయి("Into:\n");
      sqlite3_వృక్షం_View_సమాసం(0, pNew, 0);
#అయితే_ముగింపు
    }
  }
  తిరిగి pNew;
}


#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
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
స్థిర SQLITE_NOINLINE శూన్య సంకేతం_INTerm(
  Parse *pParse,      /* The parsing context */
  Where_పదం *p_పదం,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  పూర్ణాంకం iEq,            /* Index of the equality term within this level */
  పూర్ణాంకం bRev,           /* True for reverse-order IN operations */
  పూర్ణాంకం i_లక్ష్యం         /* Attempt to leave results in this register */
){
  సమాసం *pX = p_పదం->p_సమాసం;
  పూర్ణాంకం eType = లోపల_సూచీ_NOOP;
  పూర్ణాంకం iTab;
  నిర్మాణం లోపల_Loop *p_లోపల;
  WhereLoop *pLoop = pLevel->pWLoop;
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం i;
  పూర్ణాంకం nEq = 0;
  పూర్ణాంకం *aiMap = 0;

  ఒకవేళ( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
    && pLoop->u.btree.p_సూచీ!=0
    && pLoop->u.btree.p_సూచీ->a_క్రమీకరించు_క్రమం[iEq]
  ){
    testcase( iEq==0 );
    testcase( bRev );
    bRev = !bRev;
  }
  ప్రతిజ్ఞా( pX->op==TK_లోపల );

  కొరకు(i=0; i<iEq; i++){
    ఒకవేళ( pLoop->aLTerm[i] && pLoop->aLTerm[i]->p_సమాసం==pX ){
      disable_పదం(pLevel, p_పదం);
      తిరిగి;
    }
  }
  కొరకు(i=iEq; i<pLoop->nLTerm; i++){
    ప్రతిజ్ఞా( pLoop->aLTerm[i]!=0 );
    ఒకవేళ( pLoop->aLTerm[i]->p_సమాసం==pX ) nEq++;
  }

  iTab = 0;
  ఒకవేళ( !సమాసం_Use_XSelect(pX) || pX->x.pSelect->pEList->n_సమాసం==1 ){
    eType = sqlite3_కనుగొను_లోపల_సూచీ(pParse, pX, లోపల_సూచీ_LOOP, 0, 0, &iTab);
  }లేకపోతే{
    sqlite3 *db = pParse->db;
    సమాసం *pXMod = remove_Unindexable_లోపల_Clause_Terms(pParse, iEq, pLoop, pX);
    ఒకవేళ( nEq>1 ){
      /* If this IN(SELECT ...) expression drives more than one column of
      ** the index, disable the seek-scan optimization. The reasons for this
      ** are that (a) it is only possible to make this happen by populating
      ** the sqlite_stat1 table with inconsistent information, and (b) it
      ** would require sqlite3FindInIndex() to find an index that is not
      ** only unique for the columns in question, but also delivers them
      ** in sorted order (requires checking asc/desc, and rejecting cases
      ** where the indexed columns are not in the right order).  */
      pLoop->wsFlags &= ~WHERE_లోపల_SEEKSCAN;
    }
    ఒకవేళ( !db->mallocFailed ){
      పూర్ణాంకం n_నిలువు = pX->x.pSelect->pEList->n_సమాసం;
      aiMap = (పూర్ణాంకం*)sqlite3DbMallocZero(db, పరిమాణం(పూర్ణాంకం)*n_నిలువు);
      eType = sqlite3_కనుగొను_లోపల_సూచీ(pParse, pXMod, లోపల_సూచీ_LOOP, 0, aiMap, &iTab);
    }
    sqlite3_సమాసం_Delete(db, pXMod);
  }

  ఒకవేళ( eType==లోపల_సూచీ_సూచీ_DESC ){
    testcase( bRev );
    bRev = !bRev;
  }
  sqlite3_Vdbe_కూడు_Op2(v, bRev ? OP_చివరి : OP_Rewind, iTab, 0);
  VdbeCoverageIf(v, bRev);
  VdbeCoverageIf(v, !bRev);

  ప్రతిజ్ఞా( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );
  pLoop->wsFlags |= WHERE_లోపల_ABLE;
  ఒకవేళ( pLevel->u.లోపల.n_లోపల==0 ){
    pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
  }
  ఒకవేళ( iEq>0 && (pLoop->wsFlags & WHERE_లోపల_SEEKSCAN)==0 ){
    pLoop->wsFlags |= WHERE_లోపల_EARLYOUT;
  }

  i = pLevel->u.లోపల.n_లోపల;
  pLevel->u.లోపల.n_లోపల += nEq;
  pLevel->u.లోపల.a_లోపల_Loop =
     sqlite3WhereRealloc(p_పదం->pWC->pWInfo,
                         pLevel->u.లోపల.a_లోపల_Loop,
                         పరిమాణం(pLevel->u.లోపల.a_లోపల_Loop[0])*pLevel->u.లోపల.n_లోపల);
  p_లోపల = pLevel->u.లోపల.a_లోపల_Loop;
  ఒకవేళ( p_లోపల ){
    పూర్ణాంకం iMap = 0;               /* Index in aiMap[] */
    p_లోపల += i;
    కొరకు(i=iEq; i<pLoop->nLTerm; i++){
      ఒకవేళ( pLoop->aLTerm[i]->p_సమాసం==pX ){
        పూర్ణాంకం i_బహిర్గతం = i_లక్ష్యం + i - iEq;
        ఒకవేళ( eType==లోపల_సూచీ_ROWID ){
          p_లోపల->addr_లోపల_శిఖరం = sqlite3_Vdbe_కూడు_Op2(v, OP_Rowid, iTab, i_బహిర్గతం);
        }లేకపోతే{
          పూర్ణాంకం i_నిలువు = aiMap ? aiMap[iMap++] : 0;
          p_లోపల->addr_లోపల_శిఖరం = sqlite3_Vdbe_కూడు_Op3(v,OP_స్తంభం,iTab, i_నిలువు, i_బహిర్గతం);
        }
        sqlite3_Vdbe_కూడు_Op1(v, OP_IsNull, i_బహిర్గతం); VdbeCoverage(v);
        ఒకవేళ( i==iEq ){
          p_లోపల->iCur = iTab;
          p_లోపల->e_చివర_Loop_Op = bRev ? OP_Prev : OP_తదుపరి;
          ఒకవేళ( iEq>0 ){
            p_లోపల->i_పునాది = i_లక్ష్యం - i;
            p_లోపల->n_ఉపసర్గ = i;
          }లేకపోతే{
            p_లోపల->n_ఉపసర్గ = 0;
          }
        }లేకపోతే{
          p_లోపల->e_చివర_Loop_Op = OP_Noop;
        }
        p_లోపల++;
      }
    }
    testcase( iEq>0
              && (pLoop->wsFlags & WHERE_లోపల_SEEKSCAN)==0
              && (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 );
    ఒకవేళ( iEq>0
     && (pLoop->wsFlags & (WHERE_లోపల_SEEKSCAN|WHERE_VIRTUALTABLE))==0
    ){
      sqlite3_Vdbe_కూడు_Op3(v, OP_Seek_తగిలింది, pLevel->iIdxCur, 0, iEq);
    }
  }లేకపోతే{
    pLevel->u.లోపల.n_లోపల = 0;
  }
  sqlite3DbFree(pParse->db, aiMap);
}
#అయితే_ముగింపు


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
స్థిర పూర్ణాంకం సంకేతం_Equality_పదం(
  Parse *pParse,      /* The parsing context */
  Where_పదం *p_పదం,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  పూర్ణాంకం iEq,            /* Index of the equality term within this level */
  పూర్ణాంకం bRev,           /* True for reverse-order IN operations */
  పూర్ణాంకం i_లక్ష్యం         /* Attempt to leave results in this register */
){
  సమాసం *pX = p_పదం->p_సమాసం;
  పూర్ణాంకం iReg;                  /* Register holding results */

  ప్రతిజ్ఞా( pLevel->pWLoop->aLTerm[iEq]==p_పదం );
  ప్రతిజ్ఞా( i_లక్ష్యం>0 );
  ఒకవేళ( pX->op==TK_EQ || pX->op==TK_IS ){
    iReg = sqlite3_సమాసం_సంకేతం_లక్ష్యం(pParse, pX->p_కుడి, i_లక్ష్యం);
  }లేకపోతే ఒకవేళ( pX->op==TK_ISNULL ){
    iReg = i_లక్ష్యం;
    sqlite3_Vdbe_కూడు_Op2(pParse->pVdbe, OP_Null, 0, iReg);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
  }లేకపోతే{
    ప్రతిజ్ఞా( pX->op==TK_లోపల );
    iReg = i_లక్ష్యం;
    సంకేతం_INTerm(pParse, p_పదం, pLevel, iEq, bRev, i_లక్ష్యం);
#అయితే_ముగింపు
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
  ఒకవేళ( (pLevel->pWLoop->wsFlags & WHERE_TRANSCONS)==0
   || (p_పదం->eOperator & WO_EQUIV)==0
  ){
    disable_పదం(pLevel, p_పదం);
  }

  తిరిగి iReg;
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
స్థిర పూర్ణాంకం సంకేతం_All_Equality_Terms(
  Parse *pParse,        /* Parsing context */
  WhereLevel *pLevel,   /* Which nested loop of the FROM we are coding */
  పూర్ణాంకం bRev,             /* Reverse the order of IN operators */
  పూర్ణాంకం nExtraReg,        /* Number of extra registers to allocate */
  అక్షరం **pzAff          /* OUT: Set to point to affinity string */
){
  u16 nEq;                      /* The number of == or IN constraints to code */
  u16 nSkip;                    /* Number of left-most columns to skip */
  Vdbe *v = pParse->pVdbe;      /* The vm under construction */
  సూచీ *pIdx;                  /* The index being used for this loop */
  Where_పదం *p_పదం;             /* A single constraint term */
  WhereLoop *pLoop;             /* The WhereLoop object */
  పూర్ణాంకం j;                        /* Loop counter */
  పూర్ణాంకం reg_పునాది;                  /* Base register */
  పూర్ణాంకం nReg;                     /* Number of registers to allocate */
  అక్షరం *zAff;                   /* Affinity string to return */

  /* This module is only called on query plans that use an index. */
  pLoop = pLevel->pWLoop;
  ప్రతిజ్ఞా( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0 );
  nEq = pLoop->u.btree.nEq;
  nSkip = pLoop->nSkip;
  pIdx = pLoop->u.btree.p_సూచీ;
  ప్రతిజ్ఞా( pIdx!=0 );

  /* Figure out how many memory cells we will need then allocate them.
  */
  reg_పునాది = pParse->nMem + 1;
  nReg = nEq + nExtraReg;
  pParse->nMem += nReg;

  zAff = sqlite3DbStrDup(pParse->db,sqlite3_సూచీ_Affinity_Str(pParse->db,pIdx));
  ప్రతిజ్ఞా( zAff!=0 || pParse->db->mallocFailed );

  ఒకవేళ( nSkip ){
    పూర్ణాంకం iIdxCur = pLevel->iIdxCur;
    sqlite3_Vdbe_కూడు_Op3(v, OP_Null, 0, reg_పునాది, reg_పునాది+nSkip-1);
    sqlite3_Vdbe_కూడు_Op1(v, (bRev?OP_చివరి:OP_Rewind), iIdxCur);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    VdbeComment((v, "begin skip-scan on %s", pIdx->zName));
    j = sqlite3_Vdbe_కూడు_Op0(v, OP_Goto);
    ప్రతిజ్ఞా( pLevel->addrSkip==0 );
    pLevel->addrSkip = sqlite3_Vdbe_కూడు_Op4_Int(v, (bRev?OP_SeekLT:OP_SeekGT),
                            iIdxCur, 0, reg_పునాది, nSkip);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    sqlite3_Vdbe_Jump_ఇక్కడ(v, j);
    కొరకు(j=0; j<nSkip; j++){
      sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iIdxCur, j, reg_పునాది+j);
      testcase( pIdx->ai_స్తంభం[j]==XN_సమాసం );
      VdbeComment((v, "%s", explain_సూచీ_స్తంభం_Name(pIdx, j)));
    }
  }

  /* Evaluate the equality constraints
  */
  ప్రతిజ్ఞా( zAff==0 || (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zAff)>=nEq );
  కొరకు(j=nSkip; j<nEq; j++){
    పూర్ణాంకం r1;
    p_పదం = pLoop->aLTerm[j];
    ప్రతిజ్ఞా( p_పదం!=0 );
    /* The following testcase is true for indices with redundant columns.
    ** Ex: CREATE INDEX i1 ON t1(a,b,a); SELECT * FROM t1 WHERE a=0 AND b=0; */
    testcase( (p_పదం->wtFlags & పదం_CODED)!=0 );
    testcase( p_పదం->wtFlags & పదం_VIRTUAL );
    r1 = సంకేతం_Equality_పదం(pParse, p_పదం, pLevel, j, bRev, reg_పునాది+j);
    ఒకవేళ( r1!=reg_పునాది+j ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_Copy, r1, reg_పునాది+j);
    }
    ఒకవేళ( p_పదం->eOperator & WO_లోపల ){
      ఒకవేళ( p_పదం->p_సమాసం->flags & EP_xIsSelect ){
        /* No affinity ever needs to be (or should be) applied to a value
        ** from the RHS of an "? IN (SELECT ...)" expression. The
        ** sqlite3FindInIndex() routine has already ensured that the
        ** affinity of the comparison has been applied to the value.  */
        ఒకవేళ( zAff ) zAff[j] = SQLITE_AFF_BLOB;
      }
    }లేకపోతే ఒకవేళ( (p_పదం->eOperator & WO_ISNULL)==0 ){
      సమాసం *p_కుడి = p_పదం->p_సమాసం->p_కుడి;
      ఒకవేళ( (p_పదం->wtFlags & పదం_IS)==0 && sqlite3_సమాసం_Can_Be_Null(p_కుడి) ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, reg_పునాది+j, pLevel->addrBrk);
        VdbeCoverage(v);
      }
      ఒకవేళ( pParse->nErr==0 ){
        ప్రతిజ్ఞా( pParse->db->mallocFailed==0 );
        ఒకవేళ( sqlite3CompareAffinity(p_కుడి, zAff[j])==SQLITE_AFF_BLOB ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
        ఒకవేళ( sqlite3_సమాసం_Needs_No_Affinity_Change(p_కుడి, zAff[j]) ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
      }
    }
  }
  *pzAff = zAff;
  తిరిగి reg_పునాది;
}

#నిర్వచితం_కాకపోతే SQLITE_LIKE_DOESNT_సరిపోలిక_BLOBS
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
స్థిర శూన్య whereLikeOptimizationStringFixup(
  Vdbe *v,                /* prepared statement under construction */
  WhereLevel *pLevel,     /* The loop that contains the LIKE operator */
  Where_పదం *p_పదం        /* The upper or lower bound just coded */
){
  ఒకవేళ( p_పదం->wtFlags & పదం_LIKEOPT ){
    VdbeOp *pOp;
    ప్రతిజ్ఞా( pLevel->iLikeRepCntr>0 );
    pOp = sqlite3_Vdbe_Get_చివరి_Op(v);
    ప్రతిజ్ఞా( pOp!=0 );
    ప్రతిజ్ఞా( pOp->opcode==OP_String8
            || p_పదం->pWC->pWInfo->pParse->db->mallocFailed );
    pOp->p3 = (పూర్ణాంకం)(pLevel->iLikeRepCntr>>1);  /* Register holding counter */
    pOp->p5 = (u8)(pLevel->iLikeRepCntr&1);    /* ASC or DESC */
  }
}
#లేకపోతే
# define whereLikeOptimizationStringFixup(A,B,C)
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ENABLE_CURSOR_HINTS
/*
** Information is passed from codeCursorHint() down to individual nodes of
** the expression tree (by sqlite3WalkExpr()) using an instance of this
** structure.
*/
నిర్మాణం CCurHint {
  పూర్ణాంకం iTabCur;    /* Cursor for the main table */
  పూర్ణాంకం iIdxCur;    /* Cursor for the index, if pIdx!=0.  Unused otherwise */
  సూచీ *pIdx;    /* The index used to access the table */
};

/*
** This function is called for every node of an expression that is a candidate
** for a cursor hint on an index cursor.  For TK_COLUMN nodes that reference
** the table CCurHint.iTabCur, verify that the same column can be
** accessed through the index.  If it cannot, then set pWalker->eCode to 1.
*/
స్థిర పూర్ణాంకం సంకేతం_Cursor_Hint_Check_సమాసం(Walker *pWalker, సమాసం *p_సమాసం){
  నిర్మాణం CCurHint *pHint = pWalker->u.pCCurHint;
  ప్రతిజ్ఞా( pHint->pIdx!=0 );
  ఒకవేళ( p_సమాసం->op==TK_స్తంభం
   && p_సమాసం->i_పట్టిక==pHint->iTabCur
   && sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(pHint->pIdx, p_సమాసం->i_స్తంభం)<0
  ){
    pWalker->e_సంకేతం = 1;
  }
  తిరిగి WRC_Continue;
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
స్థిర పూర్ణాంకం సంకేతం_Cursor_Hint_Is_Or_Function(Walker *pWalker, సమాసం *p_సమాసం){
  ఒకవేళ( p_సమాసం->op==TK_IS
   || p_సమాసం->op==TK_ISNULL || p_సమాసం->op==TK_ISNOT
   || p_సమాసం->op==TK_NOTNULL || p_సమాసం->op==TK_CASE
  ){
    pWalker->e_సంకేతం = 1;
  }లేకపోతే ఒకవేళ( p_సమాసం->op==TK_FUNCTION ){
    పూర్ణాంకం d1;
    అక్షరం d2[4];
    ఒకవేళ( 0==sqlite3IsLikeFunction(pWalker->pParse->db, p_సమాసం, &d1, d2) ){
      pWalker->e_సంకేతం = 1;
    }
  }

  తిరిగి WRC_Continue;
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
స్థిర పూర్ణాంకం సంకేతం_Cursor_Hint_Fix_సమాసం(Walker *pWalker, సమాసం *p_సమాసం){
  పూర్ణాంకం rc = WRC_Continue;
  పూర్ణాంకం reg;
  నిర్మాణం CCurHint *pHint = pWalker->u.pCCurHint;
  ఒకవేళ( p_సమాసం->op==TK_స్తంభం ){
    ఒకవేళ( p_సమాసం->i_పట్టిక!=pHint->iTabCur ){
      reg = ++pWalker->pParse->nMem;   /* Register for column value */
      reg = sqlite3_సమాసం_సంకేతం_లక్ష్యం(pWalker->pParse, p_సమాసం, reg);
      p_సమాసం->op = TK_రిజిస్టరు;
      p_సమాసం->i_పట్టిక = reg;
    }లేకపోతే ఒకవేళ( pHint->pIdx!=0 ){
      p_సమాసం->i_పట్టిక = pHint->iIdxCur;
      p_సమాసం->i_స్తంభం = sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(pHint->pIdx, p_సమాసం->i_స్తంభం);
      ప్రతిజ్ఞా( p_సమాసం->i_స్తంభం>=0 );
    }
  }లేకపోతే ఒకవేళ( p_సమాసం->pAggInfo ){
    rc = WRC_Prune;
    reg = ++pWalker->pParse->nMem;   /* Register for column value */
    reg = sqlite3_సమాసం_సంకేతం_లక్ష్యం(pWalker->pParse, p_సమాసం, reg);
    p_సమాసం->op = TK_రిజిస్టరు;
    p_సమాసం->i_పట్టిక = reg;
  }లేకపోతే ఒకవేళ( p_సమాసం->op==TK_TRUEFALSE ){
    /* Do not walk disabled expressions.  tag-20230504-1 */
    తిరిగి WRC_Prune;
  }
  తిరిగి rc;
}

/*
** Insert an OP_CursorHint instruction if it is appropriate to do so.
*/
స్థిర శూన్య సంకేతం_Cursor_Hint(
  Src_అంశం *p_Tab_అంశం,  /* FROM clause item */
  WhereInfo *pWInfo,    /* The where clause */
  WhereLevel *pLevel,   /* Which loop to provide hints for */
  Where_పదం *p_చివర_Range  /* Hint this end-of-scan boundary term if not NULL */
){
  Parse *pParse = pWInfo->pParse;
  sqlite3 *db = pParse->db;
  Vdbe *v = pParse->pVdbe;
  సమాసం *p_సమాసం = 0;
  WhereLoop *pLoop = pLevel->pWLoop;
  పూర్ణాంకం iCur;
  WhereClause *pWC;
  Where_పదం *p_పదం;
  పూర్ణాంకం i, j;
  నిర్మాణం CCurHint sHint;
  Walker sWalker;

  ఒకవేళ( OptimizationDisabled(db, SQLITE_CursorHints) ) తిరిగి;
  iCur = pLevel->iTabCur;
  ప్రతిజ్ఞా( iCur==pWInfo->pTabList->a[pLevel->iFrom].iCursor );
  sHint.iTabCur = iCur;
  sHint.iIdxCur = pLevel->iIdxCur;
  sHint.pIdx = pLoop->u.btree.p_సూచీ;
  memset(&sWalker, 0, పరిమాణం(sWalker));
  sWalker.pParse = pParse;
  sWalker.u.pCCurHint = &sHint;
  pWC = &pWInfo->sWC;
  కొరకు(i=0; i<pWC->n_పునాది; i++){
    p_పదం = &pWC->a[i];
    ఒకవేళ( p_పదం->wtFlags & (పదం_VIRTUAL|పదం_CODED) ) కొనసాగు;
    ఒకవేళ( p_పదం->prereqAll & pLevel->notReady ) కొనసాగు;

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
    ఒకవేళ( p_Tab_అంశం->fg.jointype & JT_ఎడమ ){
      సమాసం *p_సమాసం = p_పదం->p_సమాసం;
      ఒకవేళ( !సమాసం_Has_Property(p_సమాసం, EP_OuterON)
       || p_సమాసం->w.iJoin!=p_Tab_అంశం->iCursor
      ){
        sWalker.e_సంకేతం = 0;
        sWalker.x_సమాసం_Callback = సంకేతం_Cursor_Hint_Is_Or_Function;
        sqlite3_Walk_సమాసం(&sWalker, p_పదం->p_సమాసం);
        ఒకవేళ( sWalker.e_సంకేతం ) కొనసాగు;
      }
    }లేకపోతే{
      ఒకవేళ( సమాసం_Has_Property(p_పదం->p_సమాసం, EP_OuterON) ) కొనసాగు;
    }

    /* All terms in pWLoop->aLTerm[] except pEndRange are used to initialize
    ** the cursor.  These terms are not needed as hints for a pure range
    ** scan (that has no == terms) so omit them. */
    ఒకవేళ( pLoop->u.btree.nEq==0 && p_పదం!=p_చివర_Range ){
      కొరకు(j=0; j<pLoop->nLTerm && pLoop->aLTerm[j]!=p_పదం; j++){}
      ఒకవేళ( j<pLoop->nLTerm ) కొనసాగు;
    }

    /* No subqueries or non-deterministic functions allowed */
    ఒకవేళ( sqlite3_సమాసం_కలిగియుంది_Subquery(p_పదం->p_సమాసం) ) కొనసాగు;

    /* For an index scan, make sure referenced columns are actually in
    ** the index. */
    ఒకవేళ( sHint.pIdx!=0 ){
      sWalker.e_సంకేతం = 0;
      sWalker.x_సమాసం_Callback = సంకేతం_Cursor_Hint_Check_సమాసం;
      sqlite3_Walk_సమాసం(&sWalker, p_పదం->p_సమాసం);
      ఒకవేళ( sWalker.e_సంకేతం ) కొనసాగు;
    }

    /* If we survive all prior tests, that means this term is worth hinting */
    p_సమాసం = sqlite3_సమాసం_And(pParse, p_సమాసం, sqlite3_సమాసం_Dup(db, p_పదం->p_సమాసం, 0));
  }
  ఒకవేళ( p_సమాసం!=0 ){
    sWalker.x_సమాసం_Callback = సంకేతం_Cursor_Hint_Fix_సమాసం;
    ఒకవేళ( pParse->nErr==0 ) sqlite3_Walk_సమాసం(&sWalker, p_సమాసం);
    sqlite3_Vdbe_కూడు_Op4(v, OP_CursorHint,
                      (sHint.pIdx ? sHint.iIdxCur : sHint.iTabCur), 0, 0,
                      (స్థిరరాశి అక్షరం*)p_సమాసం, P4_సమాసం);
  }
}
#లేకపోతే
# define సంకేతం_Cursor_Hint(A,B,C,D)  /* No-op */
#అయితే_ముగింపు /* SQLITE_ENABLE_CURSOR_HINTS */

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
స్థిర శూన్య సంకేతం_Deferred_Seek(
  WhereInfo *pWInfo,              /* Where clause context */
  సూచీ *pIdx,                    /* Index scan is using */
  పూర్ణాంకం iCur,                       /* Cursor for IPK b-tree */
  పూర్ణాంకం iIdxCur                     /* Index cursor */
){
  Parse *pParse = pWInfo->pParse; /* Parse context */
  Vdbe *v = pParse->pVdbe;        /* Vdbe to generate code within */

  ప్రతిజ్ఞా( iIdxCur>0 );
  ప్రతిజ్ఞా( pIdx->ai_స్తంభం[pIdx->n_స్తంభం-1]==-1 );

  pWInfo->bDeferredSeek = 1;
  sqlite3_Vdbe_కూడు_Op3(v, OP_DeferredSeek, iIdxCur, 0, iCur);
  ఒకవేళ( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_కుడి_JOIN))
   && Db_ముసుగు_All_Zero(sqlite3ParseToplevel(pParse)->write_ముసుగు)
  ){
    పూర్ణాంకం i;
    పట్టిక *pTab = pIdx->p_పట్టిక;
    u32 *ai = (u32*)sqlite3DbMallocZero(pParse->db, పరిమాణం(u32)*(pTab->n_నిలువు+1));
    ఒకవేళ( ai ){
      ai[0] = pTab->n_నిలువు;
      కొరకు(i=0; i<pIdx->n_స్తంభం-1; i++){
        పూర్ణాంకం x1, x2;
        ప్రతిజ్ఞా( pIdx->ai_స్తంభం[i]<pTab->n_నిలువు );
        x1 = pIdx->ai_స్తంభం[i];
        x2 = sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, x1);
        testcase( x1!=x2 );
        ఒకవేళ( x1>=0 ) ai[x2+1] = i+1;
      }
      sqlite3VdbeChangeP4(v, -1, (అక్షరం*)ai, P4_INTARRAY);
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
స్థిర శూన్య సంకేతం_సమాసం_Or_సదిశ(Parse *pParse, సమాసం *p, పూర్ణాంకం iReg, పూర్ణాంకం nReg){
  ప్రతిజ్ఞా( nReg>0 );
  ఒకవేళ( p && sqlite3_సమాసం_Is_సదిశ(p) ){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
    ఒకవేళ( సమాసం_Use_XSelect(p) ){
      Vdbe *v = pParse->pVdbe;
      పూర్ణాంకం iSelect;
      ప్రతిజ్ఞా( p->op==TK_SELECT );
      iSelect = sqlite3_సంకేతం_Subselect(pParse, p);
      sqlite3_Vdbe_కూడు_Op3(v, OP_Copy, iSelect, iReg, nReg-1);
    }లేకపోతే
#అయితే_ముగింపు
    {
      పూర్ణాంకం i;
      స్థిరరాశి సమాసం_List *pList;
      ప్రతిజ్ఞా( సమాసం_Use_XList(p) );
      pList = p->x.pList;
      ప్రతిజ్ఞా( nReg<=pList->n_సమాసం );
      కొరకు(i=0; i<nReg; i++){
        sqlite3_సమాసం_సంకేతం(pParse, pList->a[i].p_సమాసం, iReg+i);
      }
    }
  }లేకపోతే{
    ప్రతిజ్ఞా( nReg==1 || pParse->nErr );
    sqlite3_సమాసం_సంకేతం(pParse, p, iReg);
  }
}

/*
** The pTruth expression is always true because it is the WHERE clause
** a partial index that is driving a query loop.  Look through all of the
** WHERE clause terms on the query, and if any of those terms must be
** true because pTruth is true, then mark those WHERE clause terms as
** coded.
*/
స్థిర శూన్య where_Apply_Partial_సూచీ_Constraints(
  సమాసం *pTruth,
  పూర్ణాంకం iTabCur,
  WhereClause *pWC
){
  పూర్ణాంకం i;
  Where_పదం *p_పదం;
  అయితే( pTruth->op==TK_AND ){
    where_Apply_Partial_సూచీ_Constraints(pTruth->p_ఎడమ, iTabCur, pWC);
    pTruth = pTruth->p_కుడి;
  }
  కొరకు(i=0, p_పదం=pWC->a; i<pWC->n_పదం; i++, p_పదం++){
    సమాసం *p_సమాసం;
    ఒకవేళ( p_పదం->wtFlags & పదం_CODED ) కొనసాగు;
    p_సమాసం = p_పదం->p_సమాసం;
    ఒకవేళ( sqlite3_సమాసం_Compare(0, p_సమాసం, pTruth, iTabCur)==0 ){
      p_పదం->wtFlags |= పదం_CODED;
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
స్థిర SQLITE_NOINLINE శూన్య filter_Pull_కిందికి(
  Parse *pParse,       /* Parsing context */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  పూర్ణాంకం iLevel,          /* Which level of pWInfo->a[] should be coded */
  పూర్ణాంకం addrNxt,         /* Jump here to bypass inner loops */
  Bitmask notReady     /* Loops that are not ready */
){
  పూర్ణాంకం saved_addrBrk;
  అయితే( ++iLevel < pWInfo->nLevel ){
    WhereLevel *pLevel = &pWInfo->a[iLevel];
    WhereLoop *pLoop = pLevel->pWLoop;
    ఒకవేళ( pLevel->regFilter==0 ) కొనసాగు;
    ఒకవేళ( pLevel->pWLoop->nSkip ) కొనసాగు;
    /*         ,--- Because sqlite3ConstructBloomFilter() has will not have set
    **  vvvvv--'    pLevel->regFilter if this were true. */
    ఒకవేళ( NEVER(pLoop->prereq & notReady) ) కొనసాగు;
    saved_addrBrk = pLevel->addrBrk;
    pLevel->addrBrk = addrNxt;
    ఒకవేళ( pLoop->wsFlags & WHERE_IPK ){
      Where_పదం *p_పదం = pLoop->aLTerm[0];
      పూర్ణాంకం regRowid;
      ప్రతిజ్ఞా( p_పదం!=0 );
      ప్రతిజ్ఞా( p_పదం->p_సమాసం!=0 );
      testcase( p_పదం->wtFlags & పదం_VIRTUAL );
      regRowid = sqlite3_Get_తాత్కాలిక_Reg(pParse);
      regRowid = సంకేతం_Equality_పదం(pParse, p_పదం, pLevel, 0, 0, regRowid);
      sqlite3_Vdbe_కూడు_Op2(pParse->pVdbe, OP_MustBeInt, regRowid, addrNxt);
      VdbeCoverage(pParse->pVdbe);
      ప్రతిజ్ఞా( sqlite3WhereLoopBloomable(pLoop) );
      sqlite3_Vdbe_కూడు_Op4_Int(pParse->pVdbe, OP_Filter, pLevel->regFilter,
                           addrNxt, regRowid, 1);
      VdbeCoverage(pParse->pVdbe);
    }లేకపోతే{
      u16 nEq = pLoop->u.btree.nEq;
      పూర్ణాంకం r1;
      అక్షరం *z_ప్రారంభం_Aff;

      ప్రతిజ్ఞా( pLoop->wsFlags & WHERE_సూచీకృత );
      ప్రతిజ్ఞా( (pLoop->wsFlags & WHERE_స్తంభం_లోపల)==0 );
      r1 = సంకేతం_All_Equality_Terms(pParse,pLevel,0,0,&z_ప్రారంభం_Aff);
      సంకేతం_Apply_Affinity(pParse, r1, nEq, z_ప్రారంభం_Aff);
      sqlite3DbFree(pParse->db, z_ప్రారంభం_Aff);
      ప్రతిజ్ఞా( sqlite3WhereLoopBloomable(pLoop) );
      sqlite3_Vdbe_కూడు_Op4_Int(pParse->pVdbe, OP_Filter, pLevel->regFilter,
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
స్థిర పూర్ణాంకం where_Loop_Is_One_అడ్డువరుస(WhereLoop *pLoop){
  ఒకవేళ( pLoop->u.btree.p_సూచీ->onError
   && pLoop->nSkip==0
   && pLoop->u.btree.nEq==pLoop->u.btree.p_సూచీ->n_కీలకం_నిలువు
  ){
    పూర్ణాంకం ii;
    కొరకు(ii=0; ii<pLoop->u.btree.nEq; ii++){
      ఒకవేళ( pLoop->aLTerm[ii]->eOperator & (WO_IS|WO_ISNULL) ){
        తిరిగి 0;
      }
    }
    తిరిగి 1;
  }
  తిరిగి 0;
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
స్థిర పూర్ణాంకం where_సమాసం_Is_Ready(
  WhereInfo *pWInfo, 
  WhereLevel *pLevel,
  Src_అంశం *p_Tab_అంశం, 
  సమాసం *pE
){
  u8 jtype = p_Tab_అంశం->fg.jointype;
  ఒకవేళ( jtype & (JT_ఎడమ|JT_LTORJ|JT_కుడి) ){
    ఒకవేళ( !సమాసం_Has_Property(pE,EP_OuterON|EP_InnerON) ){
      /* Defer processing WHERE clause constraints until after outer
      ** join processing.  tag-20220513a */
      తిరిగి 0;
    }లేకపోతే ఒకవేళ( (jtype & JT_ఎడమ) && !సమాసం_Has_Property(pE,EP_OuterON) ){
      తిరిగి 0;
    }లేకపోతే{
      Bitmask m = sqlite3_Where_Get_ముసుగు(&pWInfo->s_ముసుగు_సమితి, pE->w.iJoin);
      ఒకవేళ( m & pLevel->notReady ){
        /* An ON clause that is not ripe */
        తిరిగి 0;
      }
    }
  }
  తిరిగి 1;
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
స్థిర పూర్ణాంకం కనుగొను_No_సరిపోలిక_Continuation(
  WhereInfo *pWInfo,         /* WHERE clause */
  పూర్ణాంకం iLevel                 /* Which loop we are talking about */
){
  WhereLoop *pLoop;          /* Description of this loop */
  WhereLevel *p_మాతృ;       /* Controlling parent loop */
  SrcList *pTabList;         /* Complete FROM clause */
  Bitmask m_కీలకం;              /* Mask of dependent tables */
  పూర్ణాంకం i_మాతృ;               /* Inner-most loop affecting the key to iLevel */
  పూర్ణాంకం i;                     /* Loop counter */

  ఒకవేళ( iLevel<2 ){
    /* If there are less than two outer loops, this optimization is no help */
    తిరిగి 0;
  }

  /* The OP_SeekGE and OP_SeekLE opcodes only work right for this
  ** optimization on a pure equality constraint, so skip the optimization
  ** for anything else.
  */
  pLoop = pWInfo->a[iLevel].pWLoop;
  ఒకవేళ( (pLoop->wsFlags & WHERE_CONSTRAINT)!=WHERE_స్తంభం_EQ ) తిరిగి 0;
  ఒకవేళ( pLoop->wsFlags & WHERE_SKIPSCAN ) తిరిగి 0;
  ఒకవేళ( pLoop->wsFlags & WHERE_BIGNULL_క్రమీకరించు ) తిరిగి 0;
  ప్రతిజ్ఞా( (pLoop->wsFlags & (WHERE_VIRTUALTABLE|WHERE_MULTI_OR))==0 );
  ప్రతిజ్ఞా( pLoop->u.btree.nEq>0 );

  /* Find a mask of all outer tables that influence the key.  At the same
  ** time, check to see if any keys values are computed from subqueries and
  ** fail out (return 0) if there are any. */
  m_కీలకం = 0;
  కొరకు(i=0; i<pLoop->nLTerm; i++){
    ఒకవేళ( sqlite3_సమాసం_కలిగియుంది_Subquery(pLoop->aLTerm[i]->p_సమాసం) ){
      తిరిగి 0;
    }
    m_కీలకం |= pLoop->aLTerm[i]->prereq_కుడి;
  }

  /* Look back through outer loops.  Find the inner-most outer loop
  ** that influences the key. Make iParent be the index of that inner-most
  ** outer loop.
  */
  pTabList = pWInfo->pTabList;
  i_మాతృ = iLevel-1;
  అయితే( i_మాతృ>0 && (pWInfo->a[i_మాతృ].pWLoop->ముసుగు_Self & m_కీలకం)==0 ){
    i_మాతృ--;
  }
  ప్రతిజ్ఞా( i_మాతృ>=0 && i_మాతృ<=iLevel );

  /* Everything needs to be an INNER join.  Fail if there are any
  ** OUTER joins in between iParent and iLevel. */
  కొరకు(i=i_మాతృ; i<=iLevel; i++){
    ఒకవేళ( pTabList->a[pWInfo->a[i].iFrom].fg.jointype & JT_OUTER ) తిరిగి 0;
  }

  /* If we reach this point, that means the conditions of the optimization
  ** have been met.  Even so, disallow the optimization if it is disabled.
  */
  ఒకవేళ( OptimizationDisabled(pWInfo->pParse->db, SQLITE_Join_తప్పిపోవు) ){
    తిరిగి 0;
  }

  /* If the inner-most outer loop that influences the key for iLevel
  ** is controlled by an automatic index and if the automatic index is
  ** the only outer loop that influences the key, then arrange to delete
  ** the automatic index entry if there is no match on the key.
  */
  p_మాతృ = &pWInfo->a[i_మాతృ];
  ఒకవేళ( (p_మాతృ->pWLoop->wsFlags & WHERE_AUTO_సూచీ)!=0
   && (m_కీలకం & (m_కీలకం-1))==0
  ){
    కొరకు(i=0; i<pLoop->nLTerm; i++){
      సమాసం *pX = pLoop->aLTerm[i]->p_సమాసం;
      ఒకవేళ( !sqlite3_సమాసం_Is_పట్టిక_Constant(pX->p_కుడి,p_మాతృ->iTabCur,0) ) ఆపు;
    }
    ఒకవేళ( i==pLoop->nLTerm ){    
      ఒకవేళ( p_మాతృ->addrDeleteCont==0 ){
        p_మాతృ->addrDeleteCont = sqlite3VdbeMakeLabel(pWInfo->pParse);
      }
      తిరిగి p_మాతృ->addrDeleteCont;
    }
  }

  testcase( i_మాతృ<iLevel-1 );
  తిరిగి p_మాతృ->addrCont;
}

/*
** Generate code for the start of the iLevel-th loop in the WHERE clause
** implementation described by pWInfo.
*/
Bitmask sqlite3_Where_సంకేతం_One_Loop_ప్రారంభం(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  పూర్ణాంకం iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
){
  పూర్ణాంకం j, k;            /* Loop counters */
  పూర్ణాంకం iCur;            /* The VDBE cursor for the table */
  పూర్ణాంకం addrNxt;         /* Where to jump to continue with the next IN case */
  పూర్ణాంకం bRev;            /* True if we need to scan in reverse order */
  WhereLoop *pLoop;    /* The WhereLoop object being coded */
  WhereClause *pWC;    /* Decomposition of the entire WHERE clause */
  Where_పదం *p_పదం;         /* A WHERE clause term */
  sqlite3 *db;              /* Database connection */
  Src_అంశం *p_Tab_అంశం;        /* FROM clause term being coded */
  పూర్ణాంకం addrBrk;              /* Jump here to break out of the loop */
  పూర్ణాంకం addrCont;             /* Jump here to continue with next cycle */
  పూర్ణాంకం addr_తప్పిపోవు;             /* addrCont enhanced for no-matching rows */
  పూర్ణాంకం iRowidReg = 0;        /* Rowid is stored in this register, if not zero */
  పూర్ణాంకం iReleaseReg = 0;      /* Temp register to free before returning */
  సూచీ *pIdx = 0;          /* Index used by loop (if any) */
  పూర్ణాంకం iLoop;                /* Iteration of constraint generator loop */

  pWC = &pWInfo->sWC;
  db = pParse->db;
  pLoop = pLevel->pWLoop;
  p_Tab_అంశం = &pWInfo->pTabList->a[pLevel->iFrom];
  iCur = p_Tab_అంశం->iCursor;
  pLevel->notReady = notReady & ~sqlite3_Where_Get_ముసుగు(&pWInfo->s_ముసుగు_సమితి, iCur);
  bRev = (pWInfo->rev_ముసుగు>>iLevel)&1;
  VdbeModuleComment((v, "Begin WHERE-loop%d: %s",
                     iLevel, p_Tab_అంశం->pSTab->zName));
#ఒకవేళ WHERETRACE_ENABLED /* 0x4001 */
  ఒకవేళ( sqlite3WhereTrace & 0x1 ){
    sqlite3DebugPrintf("Coding level %d of %d:  notReady=%llx  iFrom=%d\n",
       iLevel, pWInfo->nLevel, (u64)notReady, pLevel->iFrom);
    ఒకవేళ( sqlite3WhereTrace & 0x1000 ){
      sqlite3WhereLoopPrint(pLoop, pWC);
    }
  }
  ఒకవేళ( (sqlite3WhereTrace & 0x4001)==0x4001 ){
    ఒకవేళ( iLevel==0 ){
      sqlite3DebugPrintf("WHERE clause being coded:\n");
      sqlite3_వృక్షం_View_సమాసం(0, pWInfo->pWhere, 0);
    }
    sqlite3DebugPrintf("All WHERE-clause terms before coding:\n");
    sqlite3WhereClausePrint(pWC);
  }
#అయితే_ముగింపు

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
  addr_తప్పిపోవు = కనుగొను_No_సరిపోలిక_Continuation(pWInfo, iLevel);

  /* If this is the right table of a LEFT OUTER JOIN, allocate and
  ** initialize a memory cell that records if this table matches any
  ** row of the left table of the join.
  */
  ప్రతిజ్ఞా( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_కుడి_JOIN))
       || pLevel->iFrom>0 || (p_Tab_అంశం[0].fg.jointype & JT_ఎడమ)==0
  );
  ఒకవేళ( pLevel->iFrom>0 && (p_Tab_అంశం[0].fg.jointype & JT_ఎడమ)!=0 ){
    pLevel->i_ఎడమ_Join = ++pParse->nMem;
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, pLevel->i_ఎడమ_Join);
    VdbeComment((v, "init LEFT JOIN match flag"));
    ఒకవేళ( p_Tab_అంశం->fg.ద్వారా_Coroutine ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_Null_అడ్డువరుస, p_Tab_అంశం->iCursor, 1);
    }
  }

  /* Special case of a FROM clause subquery implemented as a co-routine */
  ఒకవేళ( p_Tab_అంశం->fg.ద్వారా_Coroutine ){
    పూర్ణాంకం regYield;
    Subquery *pSubq;
    ప్రతిజ్ఞా( p_Tab_అంశం->fg.isSubquery && p_Tab_అంశం->u4.pSubq!=0 );
    pSubq = p_Tab_అంశం->u4.pSubq;
    regYield = pSubq->regReturn;
    sqlite3_Vdbe_కూడు_Op3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    pLevel->p2 =  sqlite3_Vdbe_కూడు_Op2(v, OP_Yield, regYield, addrBrk);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", p_Tab_అంశం->pSTab->zName));
    pLevel->op = OP_Goto;
  }లేకపోతే

#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
  ఒకవేళ(  (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
    /* Case 1:  The table is a virtual-table.  Use the VFilter and VNext
    **          to access the data.
    */
    పూర్ణాంకం iReg;   /* P3 Value for OP_VFilter */
    పూర్ణాంకం addr_Not_దొరికింది;
    పూర్ణాంకం nConstraint = pLoop->nLTerm;

    iReg = sqlite3_Get_తాత్కాలిక_Range(pParse, nConstraint+2);
    addr_Not_దొరికింది = pLevel->addrBrk;
    కొరకు(j=0; j<nConstraint; j++){
      పూర్ణాంకం i_లక్ష్యం = iReg+j+2;
      p_పదం = pLoop->aLTerm[j];
      ఒకవేళ( NEVER(p_పదం==0) ) కొనసాగు;
      ఒకవేళ( p_పదం->eOperator & WO_లోపల ){
        ఒకవేళ( SMASKBIT32(j) & pLoop->u.vtab.m_Handle_లోపల ){
          పూర్ణాంకం iTab = pParse->nTab++;
          పూర్ణాంకం iCache = ++pParse->nMem;
          sqlite3_సంకేతం_Rhs_యొక్క_లోపల(pParse, p_పదం->p_సమాసం, iTab, 0);
          sqlite3_Vdbe_కూడు_Op3(v, OP_VInit_లోపల, iTab, i_లక్ష్యం, iCache);
        }లేకపోతే{
          సంకేతం_Equality_పదం(pParse, p_పదం, pLevel, j, bRev, i_లక్ష్యం);
          addr_Not_దొరికింది = pLevel->addrNxt;
        }
      }లేకపోతే{
        సమాసం *p_కుడి = p_పదం->p_సమాసం->p_కుడి;
        సంకేతం_సమాసం_Or_సదిశ(pParse, p_కుడి, i_లక్ష్యం, 1);
        ఒకవేళ( p_పదం->e_సరిపోలిక_Op==SQLITE_సూచీ_CONSTRAINT_స్థానభ్రంశం
         && pLoop->u.vtab.b_Omit_స్థానభ్రంశం
        ){
          ప్రతిజ్ఞా( p_పదం->eOperator==WO_AUX );
          ప్రతిజ్ఞా( pWInfo->pSelect!=0 );
          ప్రతిజ్ఞా( pWInfo->pSelect->i_స్థానభ్రంశం>0 );
          sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, pWInfo->pSelect->i_స్థానభ్రంశం);
          VdbeComment((v,"Zero OFFSET counter"));
        }
      }
    }
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, pLoop->u.vtab.idx_అంకె, iReg);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, nConstraint, iReg+1);
    /* The instruction immediately prior to OP_VFilter must be an OP_Integer
    ** that sets the "argc" value for xVFilter.  This is necessary for
    ** resolveP2() to work correctly.  See tag-20250207a. */
    sqlite3_Vdbe_కూడు_Op4(v, OP_VFilter, iCur, addr_Not_దొరికింది, iReg,
                      pLoop->u.vtab.idxStr,
                      pLoop->u.vtab.needFree ? P4_DYNAMIC : P4_STATIC);
    VdbeCoverage(v);
    pLoop->u.vtab.needFree = 0;
    /* An OOM inside of AddOp4(OP_VFilter) instruction above might have freed
    ** the u.vtab.idxStr.  NULL it out to prevent a use-after-free */
    ఒకవేళ( db->mallocFailed ) pLoop->u.vtab.idxStr = 0;
    pLevel->p1 = iCur;
    pLevel->op = pWInfo->eOnePass ? OP_Noop : OP_VNext;
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);
    ప్రతిజ్ఞా( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );

    కొరకు(j=0; j<nConstraint; j++){
      p_పదం = pLoop->aLTerm[j];
      ఒకవేళ( j<16 && (pLoop->u.vtab.omit_ముసుగు>>j)&1 ){
        disable_పదం(pLevel, p_పదం);
        కొనసాగు;
      }
      ఒకవేళ( (p_పదం->eOperator & WO_లోపల)!=0
       && (SMASKBIT32(j) & pLoop->u.vtab.m_Handle_లోపల)==0
       && !db->mallocFailed
      ){
        సమాసం *pCompare;  /* The comparison operator */
        సమాసం *p_కుడి;    /* RHS of the comparison */
        VdbeOp *pOp;     /* Opcode to access the value of the IN constraint */
        పూర్ణాంకం i_లోపల;         /* IN loop corresponding to the j-th constraint */

        /* Reload the constraint value into reg[iReg+j+2].  The same value
        ** was loaded into the same register prior to the OP_VFilter, but
        ** the xFilter implementation might have changed the datatype or
        ** encoding of the value in the register, so it *must* be reloaded.
        */
        కొరకు(i_లోపల=0; ALWAYS(i_లోపల<pLevel->u.లోపల.n_లోపల); i_లోపల++){
          pOp = sqlite3VdbeGetOp(v, pLevel->u.లోపల.a_లోపల_Loop[i_లోపల].addr_లోపల_శిఖరం);
          ఒకవేళ( (pOp->opcode==OP_స్తంభం && pOp->p3==iReg+j+2)
           || (pOp->opcode==OP_Rowid && pOp->p2==iReg+j+2)
          ){
            testcase( pOp->opcode==OP_Rowid );
            sqlite3_Vdbe_కూడు_Op3(v, pOp->opcode, pOp->p1, pOp->p2, pOp->p3);
            ఆపు;
          }
        }

        /* Generate code that will continue to the next row if
        ** the IN constraint is not satisfied
        */
        pCompare = sqlite3PExpr(pParse, TK_EQ, 0, 0);
        ఒకవేళ( !db->mallocFailed ){
          పూర్ణాంకం iFld = p_పదం->u.x.i_క్షేత్రం;
          సమాసం *p_ఎడమ = p_పదం->p_సమాసం->p_ఎడమ;
          ప్రతిజ్ఞా( p_ఎడమ!=0 );
          ఒకవేళ( iFld>0 ){
            ప్రతిజ్ఞా( p_ఎడమ->op==TK_సదిశ );
            ప్రతిజ్ఞా( సమాసం_Use_XList(p_ఎడమ) );
            ప్రతిజ్ఞా( iFld<=p_ఎడమ->x.pList->n_సమాసం );
            pCompare->p_ఎడమ = p_ఎడమ->x.pList->a[iFld-1].p_సమాసం;
          }లేకపోతే{
            pCompare->p_ఎడమ = p_ఎడమ;
          }
          pCompare->p_కుడి = p_కుడి = sqlite3_సమాసం(db, TK_రిజిస్టరు, 0);
          ఒకవేళ( p_కుడి ){
            p_కుడి->i_పట్టిక = iReg+j+2;
            sqlite3_సమాసం_If_False(
                pParse, pCompare, pLevel->addrCont, SQLITE_JUMPIFNULL
            );
          }
          pCompare->p_ఎడమ = 0;
        }
        sqlite3_సమాసం_Delete(db, pCompare);
      }
    }

    /* These registers need to be preserved in case there is an IN operator
    ** loop.  So we could deallocate the registers here (and potentially
    ** reuse them later) if (pLoop->wsFlags & WHERE_IN_ABLE)==0.  But it seems
    ** simpler and safer to simply not reuse the registers.
    **
    **    sqlite3ReleaseTempRange(pParse, iReg, nConstraint+2);
    */
  }లేకపోతే
#అయితే_ముగింపు /* SQLITE_OMIT_VIRTUALTABLE */

  ఒకవేళ( (pLoop->wsFlags & WHERE_IPK)!=0
   && (pLoop->wsFlags & (WHERE_స్తంభం_లోపల|WHERE_స్తంభం_EQ))!=0
  ){
    /* Case 2:  We can directly reference a single row using an
    **          equality comparison against the ROWID field.  Or
    **          we reference multiple rows using a "rowid IN (...)"
    **          construct.
    */
    ప్రతిజ్ఞా( pLoop->u.btree.nEq==1 );
    p_పదం = pLoop->aLTerm[0];
    ప్రతిజ్ఞా( p_పదం!=0 );
    ప్రతిజ్ఞా( p_పదం->p_సమాసం!=0 );
    testcase( p_పదం->wtFlags & పదం_VIRTUAL );
    iReleaseReg = ++pParse->nMem;
    iRowidReg = సంకేతం_Equality_పదం(pParse, p_పదం, pLevel, 0, bRev, iReleaseReg);
    ఒకవేళ( iRowidReg!=iReleaseReg ) sqlite3_Release_తాత్కాలిక_Reg(pParse, iReleaseReg);
    addrNxt = pLevel->addrNxt;
    ఒకవేళ( addr_తప్పిపోవు==0 ) addr_తప్పిపోవు = addrNxt;
    ఒకవేళ( pLevel->regFilter ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_MustBeInt, iRowidReg, addr_తప్పిపోవు);
      VdbeCoverage(v);
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Filter, pLevel->regFilter, addr_తప్పిపోవు,
                           iRowidReg, 1);
      VdbeCoverage(v);
      filter_Pull_కిందికి(pParse, pWInfo, iLevel, addrNxt, notReady);
    }
    sqlite3_Vdbe_కూడు_Op3(v, OP_SeekRowid, iCur, addr_తప్పిపోవు, iRowidReg);
    VdbeCoverage(v);
    pLevel->op = OP_Noop;
  }లేకపోతే ఒకవేళ( (pLoop->wsFlags & WHERE_IPK)!=0
         && (pLoop->wsFlags & WHERE_స్తంభం_RANGE)!=0
  ){
    /* Case 3:  We have an inequality comparison against the ROWID field.
    */
    పూర్ణాంకం testOp = OP_Noop;
    పూర్ణాంకం ప్రారంభం;
    పూర్ణాంకం mem_చివర_విలువ = 0;
    Where_పదం *p_ప్రారంభం, *p_చివర;

    j = 0;
    p_ప్రారంభం = p_చివర = 0;
    ఒకవేళ( pLoop->wsFlags & WHERE_BTM_హద్దు ) p_ప్రారంభం = pLoop->aLTerm[j++];
    ఒకవేళ( pLoop->wsFlags & WHERE_శిఖరం_హద్దు ) p_చివర = pLoop->aLTerm[j++];
    ప్రతిజ్ఞా( p_ప్రారంభం!=0 || p_చివర!=0 );
    ఒకవేళ( bRev ){
      p_పదం = p_ప్రారంభం;
      p_ప్రారంభం = p_చివర;
      p_చివర = p_పదం;
    }
    సంకేతం_Cursor_Hint(p_Tab_అంశం, pWInfo, pLevel, p_చివర);
    ఒకవేళ( p_ప్రారంభం ){
      సమాసం *pX;             /* The expression that defines the start bound */
      పూర్ణాంకం r1, r_తాత్కాలిక;        /* Registers for holding the start boundary */
      పూర్ణాంకం op;               /* Cursor seek operation */

      /* The following constant maps TK_xx codes into corresponding
      ** seek opcodes.  It depends on a particular ordering of TK_xx
      */
      స్థిరరాశి u8 aMoveOp[] = {
           /* TK_GT */  OP_SeekGT,
           /* TK_LE */  OP_SeekLE,
           /* TK_LT */  OP_SeekLT,
           /* TK_GE */  OP_SeekGE
      };
      ప్రతిజ్ఞా( TK_LE==TK_GT+1 );      /* Make sure the ordering.. */
      ప్రతిజ్ఞా( TK_LT==TK_GT+2 );      /*  ... of the TK_xx values... */
      ప్రతిజ్ఞా( TK_GE==TK_GT+3 );      /*  ... is correct. */

      ప్రతిజ్ఞా( (p_ప్రారంభం->wtFlags & పదం_VNULL)==0 );
      testcase( p_ప్రారంభం->wtFlags & పదం_VIRTUAL );
      pX = p_ప్రారంభం->p_సమాసం;
      ప్రతిజ్ఞా( pX!=0 );
      testcase( p_ప్రారంభం->ఎడమ_Cursor!=iCur ); /* transitive constraints */
      ఒకవేళ( sqlite3_సమాసం_Is_సదిశ(pX->p_కుడి) ){
        r1 = r_తాత్కాలిక = sqlite3_Get_తాత్కాలిక_Reg(pParse);
        సంకేతం_సమాసం_Or_సదిశ(pParse, pX->p_కుడి, r1, 1);
        testcase( pX->op==TK_GT );
        testcase( pX->op==TK_GE );
        testcase( pX->op==TK_LT );
        testcase( pX->op==TK_LE );
        op = aMoveOp[((pX->op - TK_GT - 1) & 0x3) | 0x1];
        ప్రతిజ్ఞా( pX->op!=TK_GT || op==OP_SeekGE );
        ప్రతిజ్ఞా( pX->op!=TK_GE || op==OP_SeekGE );
        ప్రతిజ్ఞా( pX->op!=TK_LT || op==OP_SeekLE );
        ప్రతిజ్ఞా( pX->op!=TK_LE || op==OP_SeekLE );
      }లేకపోతే{
        r1 = sqlite3_సమాసం_సంకేతం_తాత్కాలిక(pParse, pX->p_కుడి, &r_తాత్కాలిక);
        disable_పదం(pLevel, p_ప్రారంభం);
        op = aMoveOp[(pX->op - TK_GT)];
      }
      sqlite3_Vdbe_కూడు_Op3(v, op, iCur, addrBrk, r1);
      VdbeComment((v, "pk"));
      VdbeCoverageIf(v, pX->op==TK_GT);
      VdbeCoverageIf(v, pX->op==TK_LE);
      VdbeCoverageIf(v, pX->op==TK_LT);
      VdbeCoverageIf(v, pX->op==TK_GE);
      sqlite3_Release_తాత్కాలిక_Reg(pParse, r_తాత్కాలిక);
    }లేకపోతే{
      sqlite3_Vdbe_కూడు_Op2(v, bRev ? OP_చివరి : OP_Rewind, iCur, pLevel->addrHalt);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
    }
    ఒకవేళ( p_చివర ){
      సమాసం *pX;
      pX = p_చివర->p_సమాసం;
      ప్రతిజ్ఞా( pX!=0 );
      ప్రతిజ్ఞా( (p_చివర->wtFlags & పదం_VNULL)==0 );
      testcase( p_చివర->ఎడమ_Cursor!=iCur ); /* Transitive constraints */
      testcase( p_చివర->wtFlags & పదం_VIRTUAL );
      mem_చివర_విలువ = ++pParse->nMem;
      సంకేతం_సమాసం_Or_సదిశ(pParse, pX->p_కుడి, mem_చివర_విలువ, 1);
      ఒకవేళ( 0==sqlite3_సమాసం_Is_సదిశ(pX->p_కుడి)
       && (pX->op==TK_LT || pX->op==TK_GT)
      ){
        testOp = bRev ? OP_Le : OP_Ge;
      }లేకపోతే{
        testOp = bRev ? OP_Lt : OP_Gt;
      }
      ఒకవేళ( 0==sqlite3_సమాసం_Is_సదిశ(pX->p_కుడి) ){
        disable_పదం(pLevel, p_చివర);
      }
    }
    ప్రారంభం = sqlite3VdbeCurrentAddr(v);
    pLevel->op = bRev ? OP_Prev : OP_తదుపరి;
    pLevel->p1 = iCur;
    pLevel->p2 = ప్రారంభం;
    ప్రతిజ్ఞా( pLevel->p5==0 );
    ఒకవేళ( testOp!=OP_Noop ){
      iRowidReg = ++pParse->nMem;
      sqlite3_Vdbe_కూడు_Op2(v, OP_Rowid, iCur, iRowidReg);
      sqlite3_Vdbe_కూడు_Op3(v, testOp, mem_చివర_విలువ, addrBrk, iRowidReg);
      VdbeCoverageIf(v, testOp==OP_Le);
      VdbeCoverageIf(v, testOp==OP_Lt);
      VdbeCoverageIf(v, testOp==OP_Ge);
      VdbeCoverageIf(v, testOp==OP_Gt);
      sqlite3VdbeChangeP5(v, SQLITE_AFF_NUMERIC | SQLITE_JUMPIFNULL);
    }
  }లేకపోతే ఒకవేళ( pLoop->wsFlags & WHERE_సూచీకృత ){
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
    స్థిర స్థిరరాశి u8 a_ప్రారంభం_Op[] = {
      0,
      0,
      OP_Rewind,           /* 2: (!start_constraints && startEq &&  !bRev) */
      OP_చివరి,             /* 3: (!start_constraints && startEq &&   bRev) */
      OP_SeekGT,           /* 4: (start_constraints  && !startEq && !bRev) */
      OP_SeekLT,           /* 5: (start_constraints  && !startEq &&  bRev) */
      OP_SeekGE,           /* 6: (start_constraints  &&  startEq && !bRev) */
      OP_SeekLE            /* 7: (start_constraints  &&  startEq &&  bRev) */
    };
    స్థిర స్థిరరాశి u8 a_చివర_Op[] = {
      OP_IdxGE,            /* 0: (end_constraints && !bRev && !endEq) */
      OP_IdxGT,            /* 1: (end_constraints && !bRev &&  endEq) */
      OP_IdxLE,            /* 2: (end_constraints &&  bRev && !endEq) */
      OP_IdxLT,            /* 3: (end_constraints &&  bRev &&  endEq) */
    };
    u16 nEq = pLoop->u.btree.nEq;     /* Number of == or IN terms */
    u16 nBtm = pLoop->u.btree.nBtm;   /* Length of BTM vector */
    u16 n_శిఖరం = pLoop->u.btree.n_శిఖరం;   /* Length of TOP vector */
    పూర్ణాంకం reg_పునాది;                 /* Base register holding constraint values */
    Where_పదం *p_Range_ప్రారంభం = 0;  /* Inequality constraint at range start */
    Where_పదం *p_Range_చివర = 0;    /* Inequality constraint at range end */
    పూర్ణాంకం ప్రారంభం_Eq;                 /* True if range start uses ==, >= or <= */
    పూర్ణాంకం చివర_Eq;                   /* True if range end uses ==, >= or <= */
    పూర్ణాంకం ప్రారంభం_constraints;       /* Start of range is constrained */
    పూర్ణాంకం nConstraint;             /* Number of constraint terms */
    పూర్ణాంకం iIdxCur;                 /* The VDBE cursor for the index */
    పూర్ణాంకం nExtraReg = 0;           /* Number of extra registers needed */
    పూర్ణాంకం op;                      /* Instruction opcode */
    అక్షరం *z_ప్రారంభం_Aff;             /* Affinity for start of range constraint */
    అక్షరం *z_చివర_Aff = 0;           /* Affinity for end of range constraint */
    u8 bSeekPastNull = 0;        /* True to seek past initial nulls */
    u8 bStopAtNull = 0;          /* Add condition to terminate at NULLs */
    పూర్ణాంకం omit_పట్టిక;               /* True if we use the index only */
    పూర్ణాంకం regBignull = 0;          /* big-null flag register */
    పూర్ణాంకం addr_Seek_పరిశీలన = 0;        /* Opcode of the OP_SeekScan, if any */

    pIdx = pLoop->u.btree.p_సూచీ;
    iIdxCur = pLevel->iIdxCur;
    ప్రతిజ్ఞా( nEq>=pLoop->nSkip );

    /* Find any inequality constraint terms for the start and end
    ** of the range.
    */
    j = nEq;
    ఒకవేళ( pLoop->wsFlags & WHERE_BTM_హద్దు ){
      p_Range_ప్రారంభం = pLoop->aLTerm[j++];
      nExtraReg = MAX(nExtraReg, pLoop->u.btree.nBtm);
      /* Like optimization range constraints always occur in pairs */
      ప్రతిజ్ఞా( (p_Range_ప్రారంభం->wtFlags & పదం_LIKEOPT)==0 ||
              (pLoop->wsFlags & WHERE_శిఖరం_హద్దు)!=0 );
    }
    ఒకవేళ( pLoop->wsFlags & WHERE_శిఖరం_హద్దు ){
      p_Range_చివర = pLoop->aLTerm[j++];
      nExtraReg = MAX(nExtraReg, pLoop->u.btree.n_శిఖరం);
#నిర్వచితం_కాకపోతే SQLITE_LIKE_DOESNT_సరిపోలిక_BLOBS
      ఒకవేళ( (p_Range_చివర->wtFlags & పదం_LIKEOPT)!=0 ){
        ప్రతిజ్ఞా( p_Range_ప్రారంభం!=0 );                     /* LIKE opt constraints */
        ప్రతిజ్ఞా( p_Range_ప్రారంభం->wtFlags & పదం_LIKEOPT );   /* occur in pairs */
        pLevel->iLikeRepCntr = (u32)++pParse->nMem;
        /* iLikeRepCntr actually stores 2x the counter register number.  The
        ** bottom bit indicates whether the search order is ASC or DESC. */
        testcase( bRev );
        testcase( pIdx->a_క్రమీకరించు_క్రమం[nEq]==SQLITE_SO_DESC );
        ప్రతిజ్ఞా( (bRev & ~1)==0 );
        pLevel->iLikeRepCntr <<=1;
        pLevel->iLikeRepCntr |= bRev ^ (pIdx->a_క్రమీకరించు_క్రమం[nEq]==SQLITE_SO_DESC);
      }
#అయితే_ముగింపు
      ఒకవేళ( p_Range_ప్రారంభం==0 ){
        j = pIdx->ai_స్తంభం[nEq];
        ఒకవేళ( (j>=0 && pIdx->p_పట్టిక->a_నిలువు[j].notNull==0) || j==XN_సమాసం ){
          bSeekPastNull = 1;
        }
      }
    }
    ప్రతిజ్ఞా( p_Range_చివర==0 || (p_Range_చివర->wtFlags & పదం_VNULL)==0 );

    /* If the WHERE_BIGNULL_SORT flag is set, then index column nEq uses
    ** a non-default "big-null" sort (either ASC NULLS LAST or DESC NULLS
    ** FIRST). In both cases separate ordered scans are made of those
    ** index entries for which the column is null and for those for which
    ** it is not. For an ASC sort, the non-NULL entries are scanned first.
    ** For DESC, NULL entries are scanned first.
    */
    ఒకవేళ( (pLoop->wsFlags & (WHERE_శిఖరం_హద్దు|WHERE_BTM_హద్దు))==0
     && (pLoop->wsFlags & WHERE_BIGNULL_క్రమీకరించు)!=0
    ){
      ప్రతిజ్ఞా( bSeekPastNull==0 && nExtraReg==0 && nBtm==0 && n_శిఖరం==0 );
      ప్రతిజ్ఞా( p_Range_చివర==0 && p_Range_ప్రారంభం==0 );
      testcase( pLoop->nSkip>0 );
      nExtraReg = 1;
      bSeekPastNull = 1;
      pLevel->regBignull = regBignull = ++pParse->nMem;
      ఒకవేళ( pLevel->i_ఎడమ_Join ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, regBignull);
      }
      pLevel->addrBignull = sqlite3VdbeMakeLabel(pParse);
    }

    /* If we are doing a reverse order scan on an ascending index, or
    ** a forward order scan on a descending index, interchange the
    ** start and end terms (pRangeStart and pRangeEnd).
    */
    ఒకవేళ( (nEq<pIdx->n_స్తంభం && bRev==(pIdx->a_క్రమీకరించు_క్రమం[nEq]==SQLITE_SO_ASC)) ){
      మార్పిడి(Where_పదం *, p_Range_చివర, p_Range_ప్రారంభం);
      మార్పిడి(u8, bSeekPastNull, bStopAtNull);
      మార్పిడి(u8, nBtm, n_శిఖరం);
    }

    ఒకవేళ( iLevel>0 && (pLoop->wsFlags & WHERE_లోపల_SEEKSCAN)!=0 ){
      /* In case OP_SeekScan is used, ensure that the index cursor does not
      ** point to a valid row for the first iteration of this loop. */
      sqlite3_Vdbe_కూడు_Op1(v, OP_Null_అడ్డువరుస, iIdxCur);
    }

    /* Generate code to evaluate all constraint terms using == or IN
    ** and store the values of those terms in an array of registers
    ** starting at regBase.
    */
    సంకేతం_Cursor_Hint(p_Tab_అంశం, pWInfo, pLevel, p_Range_చివర);
    reg_పునాది = సంకేతం_All_Equality_Terms(pParse,pLevel,bRev,nExtraReg,&z_ప్రారంభం_Aff);
    ప్రతిజ్ఞా( z_ప్రారంభం_Aff==0 || sqlite3Strlen30(z_ప్రారంభం_Aff)>=nEq );
#నిర్వచితం_కాకపోతే SQLITE_LIKE_DOESNT_సరిపోలిక_BLOBS
    ఒకవేళ( pLevel->iLikeRepCntr ){
      /* The LIKE optimization loop must be the innermost of any IN(...)
      ** or skip-scan loops. Otherwise rows may not be visited in index 
      ** order. This means it needs an addrNxt label for the same reason
      ** IN(...) scans do.  */
      ఒకవేళ( pLevel->addrNxt==pLevel->addrBrk ){
        pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 1, (పూర్ణాంకం)(pLevel->iLikeRepCntr>>1));
      VdbeComment((v, "LIKE loop counter"));
      pLevel->addrLikeRep = sqlite3VdbeCurrentAddr(v);
    }
#అయితే_ముగింపు
    ఒకవేళ( z_ప్రారంభం_Aff && n_శిఖరం ){
      z_చివర_Aff = sqlite3DbStrDup(db, &z_ప్రారంభం_Aff[nEq]);
    }
    addrNxt = (regBignull ? pLevel->addrBignull : pLevel->addrNxt);
    ఒకవేళ( addr_తప్పిపోవు==0 ) addr_తప్పిపోవు = addrNxt;

    testcase( p_Range_ప్రారంభం && (p_Range_ప్రారంభం->eOperator & WO_LE)!=0 );
    testcase( p_Range_ప్రారంభం && (p_Range_ప్రారంభం->eOperator & WO_GE)!=0 );
    testcase( p_Range_చివర && (p_Range_చివర->eOperator & WO_LE)!=0 );
    testcase( p_Range_చివర && (p_Range_చివర->eOperator & WO_GE)!=0 );
    ప్రారంభం_Eq = !p_Range_ప్రారంభం || p_Range_ప్రారంభం->eOperator & (WO_LE|WO_GE);
    చివర_Eq =   !p_Range_చివర || p_Range_చివర->eOperator & (WO_LE|WO_GE);
    ప్రారంభం_constraints = p_Range_ప్రారంభం || nEq>0;

    /* Seek the index cursor to the start of the range. */
    nConstraint = nEq;
    ఒకవేళ( p_Range_ప్రారంభం ){
      సమాసం *p_కుడి = p_Range_ప్రారంభం->p_సమాసం->p_కుడి;
      సంకేతం_సమాసం_Or_సదిశ(pParse, p_కుడి, reg_పునాది+nEq, nBtm);
      whereLikeOptimizationStringFixup(v, pLevel, p_Range_ప్రారంభం);
      ఒకవేళ( (p_Range_ప్రారంభం->wtFlags & పదం_VNULL)==0
       && sqlite3_సమాసం_Can_Be_Null(p_కుడి)
      ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, reg_పునాది+nEq, addrNxt);
        VdbeCoverage(v);
      }
      ఒకవేళ( z_ప్రారంభం_Aff ){
        updateRangeAffinityStr(p_కుడి, nBtm, &z_ప్రారంభం_Aff[nEq]);
      }
      nConstraint += nBtm;
      testcase( p_Range_ప్రారంభం->wtFlags & పదం_VIRTUAL );
      ఒకవేళ( sqlite3_సమాసం_Is_సదిశ(p_కుడి)==0 ){
        disable_పదం(pLevel, p_Range_ప్రారంభం);
      }లేకపోతే{
        ప్రారంభం_Eq = 1;
      }
      bSeekPastNull = 0;
    }లేకపోతే ఒకవేళ( bSeekPastNull ){
      ప్రారంభం_Eq = 0;
      sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, reg_పునాది+nEq);
      ప్రారంభం_constraints = 1;
      nConstraint++;
    }లేకపోతే ఒకవేళ( regBignull ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, reg_పునాది+nEq);
      ప్రారంభం_constraints = 1;
      nConstraint++;
    }
    సంకేతం_Apply_Affinity(pParse, reg_పునాది, nConstraint - bSeekPastNull, z_ప్రారంభం_Aff);
    ఒకవేళ( pLoop->nSkip>0 && nConstraint==pLoop->nSkip ){
      /* The skip-scan logic inside the call to codeAllEqualityConstraints()
      ** above has already left the cursor sitting on the correct row,
      ** so no further seeking is needed */
    }లేకపోతే{
      ఒకవేళ( regBignull ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 1, regBignull);
        VdbeComment((v, "NULL-scan pass ctr"));
      }
      ఒకవేళ( pLevel->regFilter ){
        ప్రతిజ్ఞా( sqlite3WhereLoopBloomable(pLoop) );
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Filter, pLevel->regFilter, addr_తప్పిపోవు,
                             reg_పునాది, nEq);
        VdbeCoverage(v);
        filter_Pull_కిందికి(pParse, pWInfo, iLevel, addrNxt, notReady);
      }

      op = a_ప్రారంభం_Op[(ప్రారంభం_constraints<<2) + (ప్రారంభం_Eq<<1) + bRev];
      ప్రతిజ్ఞా( op!=0 );
      ఒకవేళ( (pLoop->wsFlags & WHERE_లోపల_SEEKSCAN)!=0 && op==OP_SeekGE ){
        ప్రతిజ్ఞా( regBignull==0 );
        /* TUNING:  The OP_SeekScan opcode seeks to reduce the number
        ** of expensive seek operations by replacing a single seek with
        ** 1 or more step operations.  The question is, how many steps
        ** should we try before giving up and going with a seek.  The cost
        ** of a seek is proportional to the logarithm of the of the number
        ** of entries in the tree, so basing the number of steps to try
        ** on the estimated number of rows in the btree seems like a good
        ** guess. */
        addr_Seek_పరిశీలన = sqlite3_Vdbe_కూడు_Op1(v, OP_Seek_పరిశీలన,
                                         (pIdx->ai_అడ్డువరుస_Log_Est[0]+9)/10);
        ఒకవేళ( p_Range_ప్రారంభం || p_Range_చివర ){
          sqlite3VdbeChangeP5(v, 1);
          sqlite3VdbeChangeP2(v, addr_Seek_పరిశీలన, sqlite3VdbeCurrentAddr(v)+1);
          addr_Seek_పరిశీలన = 0;
        }
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_కూడు_Op4_Int(v, op, iIdxCur, addr_తప్పిపోవు, reg_పునాది, nConstraint);
      VdbeCoverage(v);
      VdbeCoverageIf(v, op==OP_Rewind);  testcase( op==OP_Rewind );
      VdbeCoverageIf(v, op==OP_చివరి);    testcase( op==OP_చివరి );
      VdbeCoverageIf(v, op==OP_SeekGT);  testcase( op==OP_SeekGT );
      VdbeCoverageIf(v, op==OP_SeekGE);  testcase( op==OP_SeekGE );
      VdbeCoverageIf(v, op==OP_SeekLE);  testcase( op==OP_SeekLE );
      VdbeCoverageIf(v, op==OP_SeekLT);  testcase( op==OP_SeekLT );

      ప్రతిజ్ఞా( bSeekPastNull==0 || bStopAtNull==0 );
      ఒకవేళ( regBignull ){
        ప్రతిజ్ఞా( bSeekPastNull==1 || bStopAtNull==1 );
        ప్రతిజ్ఞా( bSeekPastNull==!bStopAtNull );
        ప్రతిజ్ఞా( bStopAtNull==ప్రారంభం_Eq );
        sqlite3_Vdbe_కూడు_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        op = a_ప్రారంభం_Op[(nConstraint>1)*4 + 2 + bRev];
        sqlite3_Vdbe_కూడు_Op4_Int(v, op, iIdxCur, addrNxt, reg_పునాది,
                             nConstraint-ప్రారంభం_Eq);
        VdbeCoverage(v);
        VdbeCoverageIf(v, op==OP_Rewind);  testcase( op==OP_Rewind );
        VdbeCoverageIf(v, op==OP_చివరి);    testcase( op==OP_చివరి );
        VdbeCoverageIf(v, op==OP_SeekGE);  testcase( op==OP_SeekGE );
        VdbeCoverageIf(v, op==OP_SeekLE);  testcase( op==OP_SeekLE );
        ప్రతిజ్ఞా( op==OP_Rewind || op==OP_చివరి || op==OP_SeekGE || op==OP_SeekLE);
      }
    }

    /* Load the value for the inequality constraint at the end of the
    ** range (if any).
    */
    nConstraint = nEq;
    ప్రతిజ్ఞా( pLevel->p2==0 );
    ఒకవేళ( p_Range_చివర ){
      సమాసం *p_కుడి = p_Range_చివర->p_సమాసం->p_కుడి;
      ప్రతిజ్ఞా( addr_Seek_పరిశీలన==0 );
      సంకేతం_సమాసం_Or_సదిశ(pParse, p_కుడి, reg_పునాది+nEq, n_శిఖరం);
      whereLikeOptimizationStringFixup(v, pLevel, p_Range_చివర);
      ఒకవేళ( (p_Range_చివర->wtFlags & పదం_VNULL)==0
       && sqlite3_సమాసం_Can_Be_Null(p_కుడి)
      ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, reg_పునాది+nEq, addrNxt);
        VdbeCoverage(v);
      }
      ఒకవేళ( z_చివర_Aff ){
        updateRangeAffinityStr(p_కుడి, n_శిఖరం, z_చివర_Aff);
        సంకేతం_Apply_Affinity(pParse, reg_పునాది+nEq, n_శిఖరం, z_చివర_Aff);
      }లేకపోతే{
        ప్రతిజ్ఞా( pParse->db->mallocFailed );
      }
      nConstraint += n_శిఖరం;
      testcase( p_Range_చివర->wtFlags & పదం_VIRTUAL );

      ఒకవేళ( sqlite3_సమాసం_Is_సదిశ(p_కుడి)==0 ){
        disable_పదం(pLevel, p_Range_చివర);
      }లేకపోతే{
        చివర_Eq = 1;
      }
    }లేకపోతే ఒకవేళ( bStopAtNull ){
      ఒకవేళ( regBignull==0 ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, reg_పునాది+nEq);
        చివర_Eq = 0;
      }
      nConstraint++;
    }
    ఒకవేళ( z_ప్రారంభం_Aff ) sqlite3DbNNFreeNN(db, z_ప్రారంభం_Aff);
    ఒకవేళ( z_చివర_Aff ) sqlite3DbNNFreeNN(db, z_చివర_Aff);

    /* Top of the loop body */
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    /* Check if the index cursor is past the end of the range. */
    ఒకవేళ( nConstraint ){
      ఒకవేళ( regBignull ){
        /* Except, skip the end-of-range check while doing the NULL-scan */
        sqlite3_Vdbe_కూడు_Op2(v, OP_IfNot, regBignull, sqlite3VdbeCurrentAddr(v)+3);
        VdbeComment((v, "If NULL-scan 2nd pass"));
        VdbeCoverage(v);
      }
      op = a_చివర_Op[bRev*2 + చివర_Eq];
      sqlite3_Vdbe_కూడు_Op4_Int(v, op, iIdxCur, addrNxt, reg_పునాది, nConstraint);
      testcase( op==OP_IdxGT );  VdbeCoverageIf(v, op==OP_IdxGT );
      testcase( op==OP_IdxGE );  VdbeCoverageIf(v, op==OP_IdxGE );
      testcase( op==OP_IdxLT );  VdbeCoverageIf(v, op==OP_IdxLT );
      testcase( op==OP_IdxLE );  VdbeCoverageIf(v, op==OP_IdxLE );
      ఒకవేళ( addr_Seek_పరిశీలన ) sqlite3_Vdbe_Jump_ఇక్కడ(v, addr_Seek_పరిశీలన);
    }
    ఒకవేళ( regBignull ){
      /* During a NULL-scan, check to see if we have reached the end of
      ** the NULLs */
      ప్రతిజ్ఞా( bSeekPastNull==!bStopAtNull );
      ప్రతిజ్ఞా( bSeekPastNull+bStopAtNull==1 );
      ప్రతిజ్ఞా( nConstraint+bSeekPastNull>0 );
      sqlite3_Vdbe_కూడు_Op2(v, OP_If, regBignull, sqlite3VdbeCurrentAddr(v)+2);
      VdbeComment((v, "If NULL-scan 1st pass"));
      VdbeCoverage(v);
      op = a_చివర_Op[bRev*2 + bSeekPastNull];
      sqlite3_Vdbe_కూడు_Op4_Int(v, op, iIdxCur, addrNxt, reg_పునాది,
                           nConstraint+bSeekPastNull);
      testcase( op==OP_IdxGT );  VdbeCoverageIf(v, op==OP_IdxGT );
      testcase( op==OP_IdxGE );  VdbeCoverageIf(v, op==OP_IdxGE );
      testcase( op==OP_IdxLT );  VdbeCoverageIf(v, op==OP_IdxLT );
      testcase( op==OP_IdxLE );  VdbeCoverageIf(v, op==OP_IdxLE );
    }

    ఒకవేళ( (pLoop->wsFlags & WHERE_లోపల_EARLYOUT)!=0 ){
      sqlite3_Vdbe_కూడు_Op3(v, OP_Seek_తగిలింది, iIdxCur, nEq, nEq);
    }

    /* Seek the table cursor, if required */
    omit_పట్టిక = (pLoop->wsFlags & WHERE_IDX_ONLY)!=0
           && (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_కుడి_JOIN))==0;
    ఒకవేళ( omit_పట్టిక ){
      /* pIdx is a covering index.  No need to access the main table. */
    }లేకపోతే ఒకవేళ( HasRowid(pIdx->p_పట్టిక) ){
      సంకేతం_Deferred_Seek(pWInfo, pIdx, iCur, iIdxCur);
    }లేకపోతే ఒకవేళ( iCur!=iIdxCur ){
      సూచీ *pPk = sqlite3_Primary_కీలకం_సూచీ(pIdx->p_పట్టిక);
      iRowidReg = sqlite3_Get_తాత్కాలిక_Range(pParse, pPk->n_కీలకం_నిలువు);
      కొరకు(j=0; j<pPk->n_కీలకం_నిలువు; j++){
        k = sqlite3_పట్టిక_స్తంభం_గమ్యం_సూచీ(pIdx, pPk->ai_స్తంభం[j]);
        sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, iIdxCur, k, iRowidReg+j);
      }
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Not_దొరికింది, iCur, addrCont,
                           iRowidReg, pPk->n_కీలకం_నిలువు); VdbeCoverage(v);
    }

    ఒకవేళ( pLevel->i_ఎడమ_Join==0 ){
      /* If a partial index is driving the loop, try to eliminate WHERE clause
      ** terms from the query that must be true due to the WHERE clause of
      ** the partial index.  This optimization does not work on an outer join,
      ** as shown by:
      **
      ** 2019-11-02 ticket 623eff57e76d45f6      (LEFT JOIN)
      ** 2025-05-29 forum post 7dee41d32506c4ae  (RIGHT JOIN)
      */
      ఒకవేళ( pIdx->pPartIdxWhere && pLevel->pRJ==0 ){
        where_Apply_Partial_సూచీ_Constraints(pIdx->pPartIdxWhere, iCur, pWC);
      }
    }లేకపోతే{
      testcase( pIdx->pPartIdxWhere );
      /* The following assert() is not a requirement, merely an observation:
      ** The OR-optimization doesn't work for the right hand table of
      ** a LEFT JOIN: */
      ప్రతిజ్ఞా( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_కుడి_JOIN))==0 );
    }

    /* Record the instruction used to terminate the loop. */
    ఒకవేళ( (pLoop->wsFlags & WHERE_ONEROW)
     || (pLevel->u.లోపల.n_లోపల && regBignull==0 && where_Loop_Is_One_అడ్డువరుస(pLoop))
    ){
      pLevel->op = OP_Noop;
    }లేకపోతే ఒకవేళ( bRev ){
      pLevel->op = OP_Prev;
    }లేకపోతే{
      pLevel->op = OP_తదుపరి;
    }
    pLevel->p1 = iIdxCur;
    pLevel->p3 = (pLoop->wsFlags&WHERE_UNQ_WANTED)!=0 ? 1:0;
    ఒకవేళ( (pLoop->wsFlags & WHERE_CONSTRAINT)==0 ){
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_అడుగు;
    }లేకపోతే{
      ప్రతిజ్ఞా( pLevel->p5==0 );
    }
    ఒకవేళ( omit_పట్టిక ) pIdx = 0;
  }లేకపోతే

#నిర్వచితం_కాకపోతే SQLITE_OMIT_OR_OPTIMIZATION
  ఒకవేళ( pLoop->wsFlags & WHERE_MULTI_OR ){
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
    సూచీ *pCov = 0;             /* Potential covering index (or NULL) */
    పూర్ణాంకం iCovCur = pParse->nTab++;  /* Cursor used for index scans (if any) */

    పూర్ణాంకం regReturn = ++pParse->nMem;           /* Register used with OP_Gosub */
    పూర్ణాంకం regRowset = 0;                        /* Register for RowSet object */
    పూర్ణాంకం regRowid = 0;                         /* Register holding rowid */
    పూర్ణాంకం iLoopBody = sqlite3VdbeMakeLabel(pParse);/* Start of loop body */
    పూర్ణాంకం iRetInit;                             /* Address of regReturn init */
    పూర్ణాంకం untestedTerms = 0;             /* Some terms not completely tested */
    పూర్ణాంకం ii;                            /* Loop counter */
    సమాసం *p_And_సమాసం = 0;                /* An ".. AND (...)" expression */
    పట్టిక *pTab = p_Tab_అంశం->pSTab;

    p_పదం = pLoop->aLTerm[0];
    ప్రతిజ్ఞా( p_పదం!=0 );
    ప్రతిజ్ఞా( p_పదం->eOperator & WO_OR );
    ప్రతిజ్ఞా( (p_పదం->wtFlags & పదం_ORINFO)!=0 );
    pOrWc = &p_పదం->u.pOrInfo->wc;
    pLevel->op = OP_Return;
    pLevel->p1 = regReturn;

    /* Set up a new SrcList in pOrTab containing the table being scanned
    ** by this loop in the a[0] slot and all notReady tables in a[1..] slots.
    ** This becomes the SrcList in the recursive call to sqlite3WhereBegin().
    */
    ఒకవేళ( pWInfo->nLevel>1 || p_Tab_అంశం->fg.fromExists ){
      పూర్ణాంకం nNotReady;                 /* The number of notReady tables */
      Src_అంశం *origSrc;              /* Original list of tables */
      nNotReady = pWInfo->nLevel - iLevel - 1;
      pOrTab = sqlite3DbMallocRawNN(db, SZ_SRCLIST(nNotReady+1));
      ఒకవేళ( pOrTab==0 ) తిరిగి notReady;
      pOrTab->nAlloc = (u8)(nNotReady + 1);
      pOrTab->nSrc = pOrTab->nAlloc;
      జ్ఞాపకం_నకలు(pOrTab->a, p_Tab_అంశం, పరిమాణం(*p_Tab_అంశం));
      origSrc = pWInfo->pTabList->a;
      కొరకు(k=1; k<=nNotReady; k++){
        జ్ఞాపకం_నకలు(&pOrTab->a[k], &origSrc[pLevel[k].iFrom], పరిమాణం(pOrTab->a[k]));
      }

      /* Clear the fromExists flag on the OR-optimized table entry so that
      ** the calls to sqlite3WhereEnd() do not code early-exits after the
      ** first row is visited. The early exit applies to this table's
      ** overall loop - including the multiple OR branches and any WHERE
      ** conditions not passed to the sub-loops - not to the sub-loops.  */
      pOrTab->a[0].fg.fromExists = 0;
    }లేకపోతే{
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
    ఒకవేళ( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
      ఒకవేళ( HasRowid(pTab) ){
        regRowset = ++pParse->nMem;
        sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, regRowset);
      }లేకపోతే{
        సూచీ *pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
        regRowset = pParse->nTab++;
        sqlite3_Vdbe_కూడు_Op2(v, OP_OpenEphemeral, regRowset, pPk->n_కీలకం_నిలువు);
        sqlite3_Vdbe_సమితి_P4_కీలకం_Info(pParse, pPk);
      }
      regRowid = ++pParse->nMem;
    }
    iRetInit = sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, regReturn);

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
    ఒకవేళ( pWC->n_పదం>1 ){
      పూర్ణాంకం i_పదం;
      కొరకు(i_పదం=0; i_పదం<pWC->n_పదం; i_పదం++){
        సమాసం *p_సమాసం = pWC->a[i_పదం].p_సమాసం;
        ఒకవేళ( &pWC->a[i_పదం] == p_పదం ) కొనసాగు;
        testcase( pWC->a[i_పదం].wtFlags & పదం_VIRTUAL );
        testcase( pWC->a[i_పదం].wtFlags & పదం_CODED );
        testcase( pWC->a[i_పదం].wtFlags & పదం_ముక్క );
        ఒకవేళ( (pWC->a[i_పదం].wtFlags & (పదం_VIRTUAL|పదం_CODED|పదం_ముక్క))!=0 ){
          కొనసాగు;
        }
        ఒకవేళ( (pWC->a[i_పదం].eOperator & WO_ALL)==0 ) కొనసాగు;
        ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Subquery) ) కొనసాగు;  /* tag-20220303a */
        ఒకవేళ( where_సమాసం_Is_Ready(pWInfo, pLevel, p_Tab_అంశం, p_సమాసం)==0 ) కొనసాగు;
        p_సమాసం = sqlite3_సమాసం_Dup(db, p_సమాసం, 0);
        p_And_సమాసం = sqlite3_సమాసం_And(pParse, p_And_సమాసం, p_సమాసం);
      }
      ఒకవేళ( p_And_సమాసం ){
        /* The extra 0x10000 bit on the opcode is masked off and does not
        ** become part of the new Expr.op.  However, it does make the
        ** op==TK_AND comparison inside of sqlite3PExpr() false, and this
        ** prevents sqlite3PExpr() from applying the AND short-circuit
        ** optimization, which we do not want here. */
        p_And_సమాసం = sqlite3PExpr(pParse, TK_AND|0x10000, 0, p_And_సమాసం);
      }
    }

    /* Run a separate WHERE clause for each term of the OR clause.  After
    ** eliminating duplicates from other WHERE clauses, the action for each
    ** sub-WHERE clause is to to invoke the main loop body as a subroutine.
    */
    ExplainQueryPlan((pParse, 1, "MULTI-INDEX OR"));
    కొరకు(ii=0; ii<pOrWc->n_పదం; ii++){
      Where_పదం *p_Or_పదం = &pOrWc->a[ii];
      ఒకవేళ( p_Or_పదం->ఎడమ_Cursor==iCur || (p_Or_పదం->eOperator & WO_AND)!=0 ){
        WhereInfo *pSubWInfo;           /* Info for single OR-term scan */
        సమాసం *p_Or_సమాసం = p_Or_పదం->p_సమాసం; /* Current OR clause term */
        సమాసం *pDelete;                  /* Local copy of OR clause term */
        పూర్ణాంకం jmp1 = 0;                   /* Address of jump operation */
        testcase( (p_Tab_అంశం[0].fg.jointype & JT_ఎడమ)!=0
               && !సమాసం_Has_Property(p_Or_సమాసం, EP_OuterON)
        ); /* See TH3 vtab25.400 and ticket 614b25314c766238 */
        pDelete = p_Or_సమాసం = sqlite3_సమాసం_Dup(db, p_Or_సమాసం, 0);
        ఒకవేళ( db->mallocFailed ){
          sqlite3_సమాసం_Delete(db, pDelete);
          కొనసాగు;
        }
        ఒకవేళ( p_And_సమాసం ){
          p_And_సమాసం->p_ఎడమ = p_Or_సమాసం;
          p_Or_సమాసం = p_And_సమాసం;
        }
        /* Loop through table entries that match term pOrTerm. */
        ExplainQueryPlan((pParse, 1, "INDEX %d", ii+1));
        WHERETRACE(0xffffffff, ("Subplan for OR-clause:\n"));
        pSubWInfo = sqlite3WhereBegin(pParse, pOrTab, p_Or_సమాసం, 0, 0, 0,
                                      WHERE_OR_SUBCLAUSE, iCovCur);
        ప్రతిజ్ఞా( pSubWInfo || pParse->nErr );
        ఒకవేళ( pSubWInfo ){
          WhereLoop *pSubLoop;
          పూర్ణాంకం addrExplain = sqlite3_Where_Explain_One_పరిశీలన(
              pParse, pOrTab, &pSubWInfo->a[0], 0
          );
          sqlite3_Where_కూడు_పరిశీలన_స్థితి(v, pOrTab, &pSubWInfo->a[0], addrExplain);

          /* This is the sub-WHERE clause body.  First skip over
          ** duplicate rows from prior sub-WHERE clauses, and record the
          ** rowid (or PRIMARY KEY) for the current row so that the same
          ** row will be skipped in subsequent sub-WHERE clauses.
          */
          ఒకవేళ( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
            పూర్ణాంకం i_సమితి = ((ii==pOrWc->n_పదం-1)?-1:ii);
            ఒకవేళ( HasRowid(pTab) ){
              sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, iCur, -1, regRowid);
              jmp1 = sqlite3_Vdbe_కూడు_Op4_Int(v, OP_అడ్డువరుస_సమితి_Test, regRowset, 0,
                                          regRowid, i_సమితి);
              VdbeCoverage(v);
            }లేకపోతే{
              సూచీ *pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
              పూర్ణాంకం nPk = pPk->n_కీలకం_నిలువు;
              పూర్ణాంకం iPk;
              పూర్ణాంకం r;

              /* Read the PK into an array of temp registers. */
              r = sqlite3_Get_తాత్కాలిక_Range(pParse, nPk);
              కొరకు(iPk=0; iPk<nPk; iPk++){
                పూర్ణాంకం i_నిలువు = pPk->ai_స్తంభం[iPk];
                sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, iCur, i_నిలువు,r+iPk);
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
              ఒకవేళ( i_సమితి ){
                jmp1 = sqlite3_Vdbe_కూడు_Op4_Int(v, OP_దొరికింది, regRowset, 0, r, nPk);
                VdbeCoverage(v);
              }
              ఒకవేళ( i_సమితి>=0 ){
                sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, r, nPk, regRowid);
                sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, regRowset, regRowid,
                                     r, nPk);
                ఒకవేళ( i_సమితి ) sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
              }

              /* Release the array of temp registers */
              sqlite3_Release_తాత్కాలిక_Range(pParse, r, nPk);
            }
          }

          /* Invoke the main loop body as a subroutine */
          sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, regReturn, iLoopBody);

          /* Jump here (skipping the main loop body subroutine) if the
          ** current sub-WHERE row is a duplicate from prior sub-WHEREs. */
          ఒకవేళ( jmp1 ) sqlite3_Vdbe_Jump_ఇక్కడ(v, jmp1);

          /* The pSubWInfo->untestedTerms flag means that this OR term
          ** contained one or more AND term from a notReady table.  The
          ** terms from the notReady table could not be tested and will
          ** need to be tested later.
          */
          ఒకవేళ( pSubWInfo->untestedTerms ) untestedTerms = 1;

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
          ప్రతిజ్ఞా( (pSubLoop->wsFlags & WHERE_AUTO_సూచీ)==0 );
          ఒకవేళ( (pSubLoop->wsFlags & WHERE_సూచీకృత)!=0
           && (ii==0 || pSubLoop->u.btree.p_సూచీ==pCov)
           && (HasRowid(pTab) || !Is_Primary_కీలకం_సూచీ(pSubLoop->u.btree.p_సూచీ))
          ){
            ప్రతిజ్ఞా( pSubWInfo->a[0].iIdxCur==iCovCur );
            pCov = pSubLoop->u.btree.p_సూచీ;
          }లేకపోతే{
            pCov = 0;
          }
          ఒకవేళ( sqlite3_Where_ఉపయోగిస్తుంది_Deferred_Seek(pSubWInfo) ){
            pWInfo->bDeferredSeek = 1;
          }

          /* Finish the loop through table entries that match term pOrTerm. */
          sqlite3_Where_చివర(pSubWInfo);
          Explain_Query_Plan_తీయి(pParse);
        }
        sqlite3_సమాసం_Delete(db, pDelete);
      }
    }
    Explain_Query_Plan_తీయి(pParse);
    ప్రతిజ్ఞా( pLevel->pWLoop==pLoop );
    ప్రతిజ్ఞా( (pLoop->wsFlags & WHERE_MULTI_OR)!=0 );
    ప్రతిజ్ఞా( (pLoop->wsFlags & WHERE_లోపల_ABLE)==0 );
    pLevel->u.pCoveringIdx = pCov;
    ఒకవేళ( pCov ) pLevel->iIdxCur = iCovCur;
    ఒకవేళ( p_And_సమాసం ){
      p_And_సమాసం->p_ఎడమ = 0;
      sqlite3_సమాసం_Delete(db, p_And_సమాసం);
    }
    sqlite3VdbeChangeP1(v, iRetInit, sqlite3VdbeCurrentAddr(v));
    sqlite3VdbeGoto(v, pLevel->addrBrk);
    sqlite3VdbeResolveLabel(v, iLoopBody);

    /* Set the P2 operand of the OP_Return opcode that will end the current
    ** loop to point to this spot, which is the top of the next containing
    ** loop.  The byte-code formatter will use that P2 value as a hint to
    ** indent everything in between the this point and the final OP_Return.
    ** See tag-20220407a in vdbe.c and shell.c */
    ప్రతిజ్ఞా( pLevel->op==OP_Return );
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    ఒకవేళ( pWInfo->pTabList!=pOrTab ){ sqlite3DbFreeNN(db, pOrTab); }
    ఒకవేళ( !untestedTerms ) disable_పదం(pLevel, p_పదం);
  }లేకపోతే
#అయితే_ముగింపు /* SQLITE_OMIT_OR_OPTIMIZATION */

  {
    /* Case 6:  There is no usable index.  We must do a complete
    **          scan of the entire table.
    */
    స్థిర స్థిరరాశి u8 a_అడుగు[] = { OP_తదుపరి, OP_Prev };
    స్థిర స్థిరరాశి u8 a_ప్రారంభం[] = { OP_Rewind, OP_చివరి };
    ప్రతిజ్ఞా( bRev==0 || bRev==1 );
    ఒకవేళ( p_Tab_అంశం->fg.isRecursive ){
      /* Tables marked isRecursive have only a single row that is stored in
      ** a pseudo-cursor.  No need to Rewind or Next such cursors. */
      pLevel->op = OP_Noop;
    }లేకపోతే{
      సంకేతం_Cursor_Hint(p_Tab_అంశం, pWInfo, pLevel, 0);
      pLevel->op = a_అడుగు[bRev];
      pLevel->p1 = iCur;
      pLevel->p2 = 1 + sqlite3_Vdbe_కూడు_Op2(v, a_ప్రారంభం[bRev],iCur,pLevel->addrHalt);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_అడుగు;
    }
  }

#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
  pLevel->addr_సందర్శించు = sqlite3VdbeCurrentAddr(v);
#అయితే_ముగింపు

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
  చేయి{
    పూర్ణాంకం i_తదుపరి = 0;                /* Next value for iLoop */
    కొరకు(p_పదం=pWC->a, j=pWC->n_పదం; j>0; j--, p_పదం++){
      సమాసం *pE;
      పూర్ణాంకం skipLikeAddr = 0;
      testcase( p_పదం->wtFlags & పదం_VIRTUAL );
      testcase( p_పదం->wtFlags & పదం_CODED );
      ఒకవేళ( p_పదం->wtFlags & (పదం_VIRTUAL|పదం_CODED) ) కొనసాగు;
      ఒకవేళ( (p_పదం->prereqAll & pLevel->notReady)!=0 ){
        testcase( pWInfo->untestedTerms==0
            && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 );
        pWInfo->untestedTerms = 1;
        కొనసాగు;
      }
      pE = p_పదం->p_సమాసం;
      ప్రతిజ్ఞా( pE!=0 );
      ఒకవేళ( where_సమాసం_Is_Ready(pWInfo, pLevel, p_Tab_అంశం, pE)==0 ) కొనసాగు;
      ఒకవేళ( iLoop==1 && !sqlite3_సమాసం_Covered_By_సూచీ(pE, pLevel->iTabCur, pIdx) ){
        i_తదుపరి = 2;
        కొనసాగు;
      }
      ఒకవేళ( iLoop<3 && (p_పదం->wtFlags & పదం_VARSELECT) ){
        ఒకవేళ( i_తదుపరి==0 ) i_తదుపరి = 3;
        కొనసాగు;
      }

      ఒకవేళ( (p_పదం->wtFlags & పదం_LIKECOND)!=0 ){
        /* If the TERM_LIKECOND flag is set, that means that the range search
        ** is sufficient to guarantee that the LIKE operator is true, so we
        ** can skip the call to the like(A,B) function.  But this only works
        ** for strings.  So do not skip the call to the function on the pass
        ** that compares BLOBs. */
#నిర్వచితమైతే SQLITE_LIKE_DOESNT_సరిపోలిక_BLOBS
        కొనసాగు;
#లేకపోతే
        u32 x = pLevel->iLikeRepCntr;
        ఒకవేళ( x>0 ){
          skipLikeAddr = sqlite3_Vdbe_కూడు_Op1(v, (x&1)?OP_IfNot:OP_If,(పూర్ణాంకం)(x>>1));
          VdbeCoverageIf(v, (x&1)==1);
          VdbeCoverageIf(v, (x&1)==0);
        }
#అయితే_ముగింపు
      }
#నిర్వచితమైతే WHERETRACE_ENABLED /* 0xffffffff */
      ఒకవేళ( sqlite3WhereTrace ){
        VdbeNoopComment((v, "WhereTerm[%d] (%p) priority=%d",
                         pWC->n_పదం-j, p_పదం, iLoop));
      }
      ఒకవేళ( sqlite3WhereTrace & 0x4000 ){
        sqlite3DebugPrintf("Coding auxiliary constraint:\n");
        sqlite3_Where_పదం_Print(p_పదం, pWC->n_పదం-j);
      }
#అయితే_ముగింపు
      sqlite3_సమాసం_If_False(pParse, pE, addrCont, SQLITE_JUMPIFNULL);
      ఒకవేళ( skipLikeAddr ) sqlite3_Vdbe_Jump_ఇక్కడ(v, skipLikeAddr);
      p_పదం->wtFlags |= పదం_CODED;
    }
    iLoop = i_తదుపరి;
  }అయితే( iLoop>0 );

  /* Insert code to test for implied constraints based on transitivity
  ** of the "==" operator.
  **
  ** Example: If the WHERE clause contains "t1.a=t2.b" and "t2.b=123"
  ** and we are coding the t1 loop and the t2 loop has not yet coded,
  ** then we cannot use the "t1.a=t2.b" constraint, but we can code
  ** the implied "t1.a=123" constraint.
  */
  కొరకు(p_పదం=pWC->a, j=pWC->n_పునాది; j>0; j--, p_పదం++){
    సమాసం *pE, sEAlt;
    Where_పదం *pAlt;
    ఒకవేళ( p_పదం->wtFlags & (పదం_VIRTUAL|పదం_CODED) ) కొనసాగు;
    ఒకవేళ( (p_పదం->eOperator & (WO_EQ|WO_IS))==0 ) కొనసాగు;
    ఒకవేళ( (p_పదం->eOperator & WO_EQUIV)==0 ) కొనసాగు;
    ఒకవేళ( p_పదం->ఎడమ_Cursor!=iCur ) కొనసాగు;
    ఒకవేళ( p_Tab_అంశం->fg.jointype & (JT_ఎడమ|JT_LTORJ|JT_కుడి) ) కొనసాగు;
    pE = p_పదం->p_సమాసం;
#నిర్వచితమైతే WHERETRACE_ENABLED /* 0x4001 */
    ఒకవేళ( (sqlite3WhereTrace & 0x4001)==0x4001 ){
      sqlite3DebugPrintf("Coding transitive constraint:\n");
      sqlite3_Where_పదం_Print(p_పదం, pWC->n_పదం-j);
    }
#అయితే_ముగింపు
    ప్రతిజ్ఞా( !సమాసం_Has_Property(pE, EP_OuterON) );
    ప్రతిజ్ఞా( (p_పదం->prereq_కుడి & pLevel->notReady)!=0 );
    ప్రతిజ్ఞా( (p_పదం->eOperator & (WO_OR|WO_AND))==0 );
    pAlt = sqlite3_Where_కనుగొను_పదం(pWC, iCur, p_పదం->u.x.ఎడమ_స్తంభం, notReady,
                    WO_EQ|WO_లోపల|WO_IS, 0);
    ఒకవేళ( pAlt==0 ) కొనసాగు;
    ఒకవేళ( pAlt->wtFlags & (పదం_CODED) ) కొనసాగు;
    ఒకవేళ( సమాసం_Has_Property(pAlt->p_సమాసం, EP_Collate) ) కొనసాగు;
    ఒకవేళ( (pAlt->eOperator & WO_లోపల)
     && సమాసం_Use_XSelect(pAlt->p_సమాసం)
     && (pAlt->p_సమాసం->x.pSelect->pEList->n_సమాసం>1)
    ){
      కొనసాగు;
    }
    testcase( pAlt->eOperator & WO_EQ );
    testcase( pAlt->eOperator & WO_IS );
    testcase( pAlt->eOperator & WO_లోపల );
    VdbeModuleComment((v, "begin transitive constraint"));
    sEAlt = *pAlt->p_సమాసం;
    sEAlt.p_ఎడమ = pE->p_ఎడమ;
    sqlite3_సమాసం_If_False(pParse, &sEAlt, addrCont, SQLITE_JUMPIFNULL);
    pAlt->wtFlags |= పదం_CODED;
  }

  /* For a RIGHT OUTER JOIN, record the fact that the current row has
  ** been matched at least once.
  */
  ఒకవేళ( pLevel->pRJ ){
    పట్టిక *pTab;
    పూర్ణాంకం nPk;
    పూర్ణాంకం r;
    పూర్ణాంకం jmp1 = 0;
    Where_కుడి_Join *pRJ = pLevel->pRJ;

    /* pTab is the right-hand table of the RIGHT JOIN.  Generate code that
    ** will record that the current row of that table has been matched at
    ** least once.  This is accomplished by storing the PK for the row in
    ** both the iMatch index and the regBloom Bloom filter.
    */
    pTab = pWInfo->pTabList->a[pLevel->iFrom].pSTab;
    ఒకవేళ( HasRowid(pTab) ){
      r = sqlite3_Get_తాత్కాలిక_Range(pParse, 2);
      sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, pLevel->iTabCur, -1, r+1);
      nPk = 1;
    }లేకపోతే{
      పూర్ణాంకం iPk;
      సూచీ *pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
      nPk = pPk->n_కీలకం_నిలువు;
      r = sqlite3_Get_తాత్కాలిక_Range(pParse, nPk+1);
      కొరకు(iPk=0; iPk<nPk; iPk++){
        పూర్ణాంకం i_నిలువు = pPk->ai_స్తంభం[iPk];
        sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, iCur, i_నిలువు,r+1+iPk);
      }
    }
    jmp1 = sqlite3_Vdbe_కూడు_Op4_Int(v, OP_దొరికింది, pRJ->i_సరిపోలిక, 0, r+1, nPk);
    VdbeCoverage(v);
    VdbeComment((v, "match against %s", pTab->zName));
    sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, r+1, nPk, r);
    sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, pRJ->i_సరిపోలిక, r, r+1, nPk);
    ఒకవేళ( pRJ->regBloom ){
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Filter_కూడు, pRJ->regBloom, 0, r+1, nPk);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
    }
    sqlite3_Vdbe_Jump_ఇక్కడ(v, jmp1);
    sqlite3_Release_తాత్కాలిక_Range(pParse, r, nPk+1);
  }

  /* For a LEFT OUTER JOIN, generate code that will record the fact that
  ** at least one row of the right table has matched the left table.
  */
  ఒకవేళ( pLevel->i_ఎడమ_Join ){
    pLevel->addr_మొదటి = sqlite3VdbeCurrentAddr(v);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 1, pLevel->i_ఎడమ_Join);
    VdbeComment((v, "record LEFT JOIN hit"));
    ఒకవేళ( pLevel->pRJ==0 ){
      వెళ్ళు సంకేతం_outer_join_constraints; /* WHERE clause constraints */
    }
  }

  ఒకవేళ( pLevel->pRJ ){
    /* Create a subroutine used to process all interior loops and code
    ** of the RIGHT JOIN.  During normal operation, the subroutine will
    ** be in-line with the rest of the code.  But at the end, a separate
    ** loop will run that invokes this subroutine for unmatched rows
    ** of pTab, with all tables to left begin set to NULL.
    */
    {
      Where_కుడి_Join *pRJ = pLevel->pRJ;
      sqlite3_Vdbe_కూడు_Op2(v, OP_BeginSubrtn, 0, pRJ->regReturn);
      pRJ->addrSubrtn = sqlite3VdbeCurrentAddr(v);
    }
    ప్రతిజ్ఞా( pParse->withinRJSubrtn < 255 );
    pParse->withinRJSubrtn++;

    /* WHERE clause constraints must be deferred until after outer join
    ** row elimination has completed, since WHERE clause constraints apply
    ** to the results of the OUTER JOIN.  The following loop generates the
    ** appropriate WHERE clause constraint checks.  tag-20220513a.
    */
  సంకేతం_outer_join_constraints:
    కొరకు(p_పదం=pWC->a, j=0; j<pWC->n_పునాది; j++, p_పదం++){
      testcase( p_పదం->wtFlags & పదం_VIRTUAL );
      testcase( p_పదం->wtFlags & పదం_CODED );
      ఒకవేళ( p_పదం->wtFlags & (పదం_VIRTUAL|పదం_CODED) ) కొనసాగు;
      ఒకవేళ( (p_పదం->prereqAll & pLevel->notReady)!=0 ){
        ప్రతిజ్ఞా( pWInfo->untestedTerms );
        కొనసాగు;
      }
      ఒకవేళ( p_Tab_అంశం->fg.jointype & JT_LTORJ ) కొనసాగు;
      ప్రతిజ్ఞా( p_పదం->p_సమాసం );
      sqlite3_సమాసం_If_False(pParse, p_పదం->p_సమాసం, addrCont, SQLITE_JUMPIFNULL);
      p_పదం->wtFlags |= పదం_CODED;
    }
  }

#ఒకవేళ WHERETRACE_ENABLED /* 0x4001 */
  ఒకవేళ( sqlite3WhereTrace & 0x4000 ){
    sqlite3DebugPrintf("All WHERE-clause terms after coding level %d:\n",
                       iLevel);
    sqlite3WhereClausePrint(pWC);
  }
  ఒకవేళ( sqlite3WhereTrace & 0x1 ){
    sqlite3DebugPrintf("End Coding level %d:  notReady=%llx\n",
       iLevel, (u64)pLevel->notReady);
  }
#అయితే_ముగింపు
  తిరిగి pLevel->notReady;
}

/*
** Generate the code for the loop that finds all non-matched terms
** for a RIGHT JOIN.
*/
SQLITE_NOINLINE శూన్య sqlite3_Where_కుడి_Join_Loop(
  WhereInfo *pWInfo,
  పూర్ణాంకం iLevel,
  WhereLevel *pLevel
){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  Where_కుడి_Join *pRJ = pLevel->pRJ;
  సమాసం *pSubWhere = 0;
  WhereClause *pWC = &pWInfo->sWC;
  WhereInfo *pSubWInfo;
  WhereLoop *pLoop = pLevel->pWLoop;
  Src_అంశం *p_Tab_అంశం = &pWInfo->pTabList->a[pLevel->iFrom];
  SrcList *pFrom;
  సంఘం {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;
  Bitmask mAll = 0;
  పూర్ణాంకం k;

  ExplainQueryPlan((pParse, 1, "RIGHT-JOIN %s", p_Tab_అంశం->pSTab->zName));
  sqlite3VdbeNoJumpsOutsideSubrtn(v, pRJ->addrSubrtn, pRJ->చివర_Subrtn,
                                  pRJ->regReturn);
  కొరకు(k=0; k<iLevel; k++){
    పూర్ణాంకం iIdxCur;
    Src_అంశం *p_కుడి;
    ప్రతిజ్ఞా( pWInfo->a[k].pWLoop->iTab == pWInfo->a[k].iFrom );
    p_కుడి = &pWInfo->pTabList->a[pWInfo->a[k].iFrom];
    mAll |= pWInfo->a[k].pWLoop->ముసుగు_Self;
    ఒకవేళ( p_కుడి->fg.ద్వారా_Coroutine ){
      Subquery *pSubq;
      ప్రతిజ్ఞా( p_కుడి->fg.isSubquery && p_కుడి->u4.pSubq!=0 );
      pSubq = p_కుడి->u4.pSubq;
      ప్రతిజ్ఞా( pSubq->pSelect!=0 && pSubq->pSelect->pEList!=0 );
      sqlite3_Vdbe_కూడు_Op3(
          v, OP_Null, 0, pSubq->reg_ఫలితం,
          pSubq->reg_ఫలితం + pSubq->pSelect->pEList->n_సమాసం-1
      );
    }
    sqlite3_Vdbe_కూడు_Op1(v, OP_Null_అడ్డువరుస, pWInfo->a[k].iTabCur);
    iIdxCur = pWInfo->a[k].iIdxCur;
    ఒకవేళ( iIdxCur ){
      sqlite3_Vdbe_కూడు_Op1(v, OP_Null_అడ్డువరుస, iIdxCur);
    }
  }
  ఒకవేళ( (p_Tab_అంశం->fg.jointype & JT_LTORJ)==0 ){
    mAll |= pLoop->ముసుగు_Self;
    కొరకు(k=0; k<pWC->n_పదం; k++){
      Where_పదం *p_పదం = &pWC->a[k];
      ఒకవేళ( (p_పదం->wtFlags & (పదం_VIRTUAL|పదం_ముక్క))!=0
       && p_పదం->eOperator!=WO_ROWVAL
      ){
        ఆపు;
      }
      ఒకవేళ( p_పదం->prereqAll & ~mAll ) కొనసాగు;
      ఒకవేళ( సమాసం_Has_Property(p_పదం->p_సమాసం, EP_OuterON|EP_InnerON) ) కొనసాగు;
      pSubWhere = sqlite3_సమాసం_And(pParse, pSubWhere,
                                 sqlite3_సమాసం_Dup(pParse->db, p_పదం->p_సమాసం, 0));
    }
  }
  ఒకవేళ( pLevel->iIdxCur ){
    /* pSubWhere may contain expressions that read from an index on the
    ** table on the RHS of the right join. All such expressions first test
    ** if the index is pointing at a NULL row, and if so, read from the
    ** table cursor instead. So ensure that the index cursor really is 
    ** pointing at a NULL row here, so that no values are read from it during
    ** the scan of the RHS of the RIGHT join below.  */
    sqlite3_Vdbe_కూడు_Op1(v, OP_Null_అడ్డువరుస, pLevel->iIdxCur);
  }
  pFrom = &uSrc.sSrc;
  pFrom->nSrc = 1;
  pFrom->nAlloc = 1;
  జ్ఞాపకం_నకలు(&pFrom->a[0], p_Tab_అంశం, పరిమాణం(Src_అంశం));
  pFrom->a[0].fg.jointype = 0;
  ఒకవేళ( pParse->withinRJSubrtn >= 100 ){
    /* This limit --------------^^^
    ** is based on an historical assert().  It is not compile-time or
    ** run-time configurable.  It could perhaps be raised as high as 254,
    ** but only an attack robot would ever do even 100 RIGHT JOINS within
    ** a single query, so we'll just leave it as it is. */
    sqlite3_Error_సందేశం(pParse, "too many RIGHT JOINs");
    తిరిగి;
  }
  pParse->withinRJSubrtn++;
  pSubWInfo = sqlite3WhereBegin(pParse, pFrom, pSubWhere, 0, 0, 0,
                                WHERE_కుడి_JOIN, 0);
  ఒకవేళ( pSubWInfo ){
    పూర్ణాంకం iCur = pLevel->iTabCur;  /* Table on RHS of RIGHT JOIN &*/
    పూర్ణాంకం r = ++pParse->nMem;      /* Register range to hold primary key */
    పూర్ణాంకం nPk;                     /* Number of values in the primary key */
    పూర్ణాంకం r2;                      /* Register holding record for primary key */
    పూర్ణాంకం addrCont = sqlite3WhereContinueLabel(pSubWInfo);
    పట్టిక *pTab = p_Tab_అంశం->pSTab;  /* Table on RHS of RIGHT JOIN */
    ఒకవేళ( HasRowid(pTab) ){
      sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, iCur, -1, r);
      nPk = 1;
    }లేకపోతే{
      పూర్ణాంకం iPk;
      సూచీ *pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
      nPk = pPk->n_కీలకం_నిలువు;
      pParse->nMem += nPk - 1;
      కొరకు(iPk=0; iPk<nPk; iPk++){
        పూర్ణాంకం i_నిలువు = pPk->ai_స్తంభం[iPk];
        sqlite3_సమాసం_సంకేతం_Get_స్తంభం_యొక్క_పట్టిక(v, pTab, iCur, i_నిలువు,r+iPk);
      }
    }

    /* Generate code that checks to see if the current row of the RHS table
    ** has appeared in any prior output row. */
    ఒకవేళ( pRJ->regBloom ){
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Filter, pRJ->regBloom, 
                 sqlite3VdbeCurrentAddr(v)+2, r, nPk);
      VdbeCoverage(v);
    }
    sqlite3_Vdbe_కూడు_Op4_Int(v, OP_దొరికింది, pRJ->i_సరిపోలిక, addrCont, r, nPk);
    VdbeCoverage(v);
    r2 = sqlite3_Get_తాత్కాలిక_Reg(pParse);

    /* Generate code that inserts the PK of the RHS table into the
    ** pRH->iMatch index to indicate that the current row has appeared
    ** in the output set. */
    sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, r, nPk, r2);
    sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, pRJ->i_సరిపోలిక, r2, r, nPk);
    sqlite3_Release_తాత్కాలిక_Reg(pParse, r2);
    ఒకవేళ( pRJ->regBloom ){
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Filter_కూడు, pRJ->regBloom, 0, r, nPk);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
    }

    /* Invoke the subroutine that actually puts the current RHS table row
    ** into the output set, with NULLs for the LHS. */
    sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, pRJ->regReturn, pRJ->addrSubrtn);
    sqlite3_Where_చివర(pSubWInfo);
  }
  sqlite3_సమాసం_Delete(pParse->db, pSubWhere);
  Explain_Query_Plan_తీయి(pParse);
  ప్రతిజ్ఞా( pParse->withinRJSubrtn>0 );
  pParse->withinRJSubrtn--;
}
