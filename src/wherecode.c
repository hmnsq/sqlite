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
#ضمن "sqliteInt.h"
#ضمن "whereInt.h"

#اگر_تعریف_نشده SQLITE_OMIT_EXPLAIN

/*
** Return the name of the i-th column of the pIdx index.
*/
ساكن ثابت حرف *explain_شاخص_ستونی_Name(شاخص *pIdx, صحيح i){
  i = pIdx->ai_ستونی[i];
  إذا( i==XN_عبارت ) أرجع "<expr>";
  إذا( i==XN_ROWID ) أرجع "rowid";
  أرجع pIdx->p_جدول->a_ستون[i].zCnName;
}

/*
** This routine is a helper for explainIndexRange() below
**
** pStr holds the text of an expression that we are building up one term
** at a time.  This routine adds a new term to the end of the expression.
** Terms are separated by AND so add the "AND" text for second and subsequent
** terms only.
*/
ساكن فراغ explain_Append_جمله(
  StrAccum *pStr,             /* The text expression being built */
  شاخص *pIdx,                /* Index to read column names from */
  صحيح n_جمله,                  /* Number of terms */
  صحيح i_جمله,                  /* Zero-based index of first term. */
  صحيح bAnd,                   /* Non-zero to append " AND " */
  ثابت حرف *zOp             /* Name of the operator */
){
  صحيح i;

  أكد( n_جمله>=1 );
  إذا( bAnd ) sqlite3_str_append(pStr, " AND ", 5);

  إذا( n_جمله>1 ) sqlite3_str_append(pStr, "(", 1);
  لكل(i=0; i<n_جمله; i++){
    إذا( i ) sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_appendall(pStr, explain_شاخص_ستونی_Name(pIdx, i_جمله+i));
  }
  إذا( n_جمله>1 ) sqlite3_str_append(pStr, ")", 1);

  sqlite3_str_append(pStr, zOp, 1);

  إذا( n_جمله>1 ) sqlite3_str_append(pStr, "(", 1);
  لكل(i=0; i<n_جمله; i++){
    إذا( i ) sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_append(pStr, "?", 1);
  }
  إذا( n_جمله>1 ) sqlite3_str_append(pStr, ")", 1);
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
ساكن فراغ explain_شاخص_Range(StrAccum *pStr, WhereLoop *pLoop){
  شاخص *p_شاخص = pLoop->u.btree.p_شاخص;
  u16 nEq = pLoop->u.btree.nEq;
  u16 nSkip = pLoop->nSkip;
  صحيح i, j;

  إذا( nEq==0 && (pLoop->wsFlags&(WHERE_BTM_حد|WHERE_بالاترین_حد))==0 ) أرجع;
  sqlite3_str_append(pStr, " (", 2);
  لكل(i=0; i<nEq; i++){
    ثابت حرف *z = explain_شاخص_ستونی_Name(p_شاخص, i);
    إذا( i ) sqlite3_str_append(pStr, " AND ", 5);
    sqlite3_str_appendf(pStr, i>=nSkip ? "%s=?" : "ANY(%s)", z);
  }

  j = i;
  إذا( pLoop->wsFlags&WHERE_BTM_حد ){
    explain_Append_جمله(pStr, p_شاخص, pLoop->u.btree.nBtm, j, i, ">");
    i = 1;
  }
  إذا( pLoop->wsFlags&WHERE_بالاترین_حد ){
    explain_Append_جمله(pStr, p_شاخص, pLoop->u.btree.n_بالاترین, j, i, "<");
  }
  sqlite3_str_append(pStr, ")", 1);
}

/*
** This function sets the P4 value of an existing OP_Explain opcode to
** text describing the loop in pLevel. If the OP_Explain opcode already has
** a P4 value, it is freed before it is overwritten.
*/
فراغ sqlite3_Where_جمع_کن_Explain_متن(
  Parse *pParse,                  /* Parse context */
  صحيح addr,                       /* Address of OP_Explain opcode */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
#إذا !defined(SQLITE_DEBUG)
  إذا( sqlite3ParseToplevel(pParse)->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#پایان_اگر
  {
    VdbeOp *pOp = sqlite3VdbeGetOp(pParse->pVdbe, addr);
    Src_قلم *p_قلم = &pTabList->a[pLevel->iFrom];
    sqlite3 *db = pParse->db;     /* Database handle */
    صحيح is_جستجو;                 /* True for a SEARCH. False for SCAN. */
    WhereLoop *pLoop;             /* The controlling WhereLoop object */
    u32 flags;                    /* Flags that describe this loop */
#إذا defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_EXPLAIN)
    حرف *z_پیام;                   /* Text to add to EQP output */
#پایان_اگر
    StrAccum str;                 /* EQP output string */
    حرف zBuf[100];               /* Initial space for EQP output string */

    إذا( db->mallocFailed ) أرجع;

    pLoop = pLevel->pWLoop;
    flags = pLoop->wsFlags;

    is_جستجو = (flags&(WHERE_BTM_حد|WHERE_بالاترین_حد))!=0
            || ((flags&WHERE_VIRTUALTABLE)==0 && (pLoop->u.btree.nEq>0))
            || (wctrlFlags&(WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX));

    sqlite3StrAccumInit(&str, db, zBuf, حجم(zBuf), SQLITE_MAX_درازی);
    str.printfFlags = SQLITE_PRINTF_داخلی;
    sqlite3_str_appendf(&str, "%s %S%s",
       is_جستجو ? "SEARCH" : "SCAN",
       p_قلم,
       p_قلم->fg.fromExists ? " EXISTS" : "");
    إذا( (flags & (WHERE_IPK|WHERE_VIRTUALTABLE))==0 ){
      ثابت حرف *zFmt = 0;
      شاخص *pIdx;

      أكد( pLoop->u.btree.p_شاخص!=0 );
      pIdx = pLoop->u.btree.p_شاخص;
      أكد( !(flags&WHERE_AUTO_شاخص) || (flags&WHERE_IDX_ONLY) );
      إذا( !HasRowid(p_قلم->pSTab) && Is_Primary_کلید_شاخص(pIdx) ){
        إذا( is_جستجو ){
          zFmt = "PRIMARY KEY";
        }
      }وإلا إذا( flags & WHERE_PARTIALIDX ){
        zFmt = "AUTOMATIC PARTIAL COVERING INDEX";
      }وإلا إذا( flags & WHERE_AUTO_شاخص ){
        zFmt = "AUTOMATIC COVERING INDEX";
      }وإلا إذا( flags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
        zFmt = "COVERING INDEX %s";
      }وإلا{
        zFmt = "INDEX %s";
      }
      إذا( zFmt ){
        sqlite3_str_append(&str, " USING ", 7);
        sqlite3_str_appendf(&str, zFmt, pIdx->zName);
        explain_شاخص_Range(&str, pLoop);
      }
    }وإلا إذا( (flags & WHERE_IPK)!=0 && (flags & WHERE_CONSTRAINT)!=0 ){
      حرف cRangeOp;
#إذا 0  /* Better output, but breaks many tests */
      ثابت جدول *pTab = p_قلم->pTab;
      ثابت حرف *zRowid = pTab->iPKey>=0 ? pTab->a_ستون[pTab->iPKey].zCnName:
                              "rowid";
#وإلا
      ثابت حرف *zRowid = "rowid";
#پایان_اگر
      sqlite3_str_appendf(&str, " USING INTEGER PRIMARY KEY (%s", zRowid);
      إذا( flags&(WHERE_ستونی_EQ|WHERE_ستونی_درون) ){
        cRangeOp = '=';
      }وإلا إذا( (flags&WHERE_BOTH_حد)==WHERE_BOTH_حد ){
        sqlite3_str_appendf(&str, ">? AND %s", zRowid);
        cRangeOp = '<';
      }وإلا إذا( flags&WHERE_BTM_حد ){
        cRangeOp = '>';
      }وإلا{
        أكد( flags&WHERE_بالاترین_حد);
        cRangeOp = '<';
      }
      sqlite3_str_appendf(&str, "%c?)", cRangeOp);
    }
#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE
    وإلا إذا( (flags & WHERE_VIRTUALTABLE)!=0 ){
      sqlite3_str_appendall(&str, " VIRTUAL TABLE INDEX ");
      sqlite3_str_appendf(&str,
                  pLoop->u.vtab.b_Idx_رقم_Hex ? "0x%x:%s" : "%d:%s",
                  pLoop->u.vtab.idx_رقم, pLoop->u.vtab.idxStr);
    }
#پایان_اگر
    إذا( p_قلم->fg.jointype & JT_چپ ){
      sqlite3_str_appendf(&str, " LEFT-JOIN");
    }
#اگر_تعریف_شده SQLITE_EXPLAIN_ESTIMATED_ردیفها
    إذا( pLoop->n_بیرون>=10 ){
      sqlite3_str_appendf(&str, " (~%llu rows)",
             sqlite3_Log_Est_مقصد_Int(pLoop->n_بیرون));
    }وإلا{
      sqlite3_str_append(&str, " (~1 row)", 9);
    }
#پایان_اگر
#إذا defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_EXPLAIN)
    z_پیام = sqlite3_Str_Accum_پایان(&str);
    sqlite3ExplainBreakpoint("",z_پیام);
#پایان_اگر

    أكد( pOp->opcode==OP_Explain );
    أكد( pOp->p4type==P4_DYNAMIC || pOp->p4.z==0 );
    sqlite3DbFree(db, pOp->p4.z);
    pOp->p4type = P4_DYNAMIC;
    pOp->p4.z = sqlite3_Str_Accum_پایان(&str);
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
صحيح sqlite3_Where_Explain_One_پیمایش(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
  صحيح ret = 0;
#إذا !defined(SQLITE_DEBUG)
  إذا( sqlite3ParseToplevel(pParse)->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#پایان_اگر
  {
    إذا( (pLevel->pWLoop->wsFlags & WHERE_MULTI_OR)==0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
    ){
      Vdbe *v = pParse->pVdbe;
      صحيح addr = sqlite3VdbeCurrentAddr(v);
      ret = sqlite3_Vdbe_جمع_کن_Op3(
          v, OP_Explain, addr, pParse->addrExplain, pLevel->pWLoop->r_پیاپی
      );
      sqlite3_Where_جمع_کن_Explain_متن(pParse, addr, pTabList, pLevel, wctrlFlags);
    }
  }
  أرجع ret;
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
صحيح sqlite3WhereExplainBloomFilter(
  ثابت Parse *pParse,               /* Parse context */
  ثابت WhereInfo *pWInfo,           /* WHERE clause */
  ثابت WhereLevel *pLevel           /* Bloom filter on this level */
){
  صحيح ret = 0;
  Src_قلم *p_قلم = &pWInfo->pTabList->a[pLevel->iFrom];
  Vdbe *v = pParse->pVdbe;      /* VM being constructed */
  sqlite3 *db = pParse->db;     /* Database handle */
  حرف *z_پیام;                   /* Text to add to EQP output */
  صحيح i;                        /* Loop counter */
  WhereLoop *pLoop;             /* The where loop */
  StrAccum str;                 /* EQP output string */
  حرف zBuf[100];               /* Initial space for EQP output string */

  sqlite3StrAccumInit(&str, db, zBuf, حجم(zBuf), SQLITE_MAX_درازی);
  str.printfFlags = SQLITE_PRINTF_داخلی;
  sqlite3_str_appendf(&str, "BLOOM FILTER ON %S (", p_قلم);
  pLoop = pLevel->pWLoop;
  إذا( pLoop->wsFlags & WHERE_IPK ){
    ثابت جدول *pTab = p_قلم->pSTab;
    إذا( pTab->iPKey>=0 ){
      sqlite3_str_appendf(&str, "%s=?", pTab->a_ستون[pTab->iPKey].zCnName);
    }وإلا{
      sqlite3_str_appendf(&str, "rowid=?");
    }
  }وإلا{
    لكل(i=pLoop->nSkip; i<pLoop->u.btree.nEq; i++){
      ثابت حرف *z = explain_شاخص_ستونی_Name(pLoop->u.btree.p_شاخص, i);
      إذا( i>pLoop->nSkip ) sqlite3_str_append(&str, " AND ", 5);
      sqlite3_str_appendf(&str, "%s=?", z);
    }
  }
  sqlite3_str_append(&str, ")", 1);
  z_پیام = sqlite3_Str_Accum_پایان(&str);
  ret = sqlite3_Vdbe_جمع_کن_Op4(v, OP_Explain, sqlite3VdbeCurrentAddr(v),
                          pParse->addrExplain, 0, z_پیام,P4_DYNAMIC);

  sqlite3_Vdbe_پیمایش_وضعیت(v, sqlite3VdbeCurrentAddr(v)-1, 0, 0, 0, 0);
  أرجع ret;
}
#پایان_اگر /* SQLITE_OMIT_EXPLAIN */

#اگر_تعریف_شده SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Configure the VM passed as the first argument with an
** sqlite3_stmt_scanstatus() entry corresponding to the scan used to
** implement level pLvl. Argument pSrclist is a pointer to the FROM
** clause that the scan reads data from.
**
** If argument addrExplain is not 0, it must be the address of an
** OP_Explain instruction that describes the same loop.
*/
فراغ sqlite3_Where_جمع_کن_پیمایش_وضعیت(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  صحيح addrExplain                 /* Address of OP_Explain (or 0) */
){
  إذا( IS_STMT_SCANSTATUS( sqlite3VdbeDb(v) ) ){
    ثابت حرف *zObj = 0;
    WhereLoop *pLoop = pLvl->pWLoop;
    صحيح wsFlags = pLoop->wsFlags;
    صحيح از_طریق_Coroutine = 0;

    إذا( (wsFlags & WHERE_VIRTUALTABLE)==0  &&  pLoop->u.btree.p_شاخص!=0 ){
      zObj = pLoop->u.btree.p_شاخص->zName;
    }وإلا{
      zObj = pSrclist->a[pLvl->iFrom].zName;
      از_طریق_Coroutine = pSrclist->a[pLvl->iFrom].fg.از_طریق_Coroutine;
    }
    sqlite3_Vdbe_پیمایش_وضعیت(
        v, addrExplain, pLvl->addrBody, pLvl->addr_بازدید, pLoop->n_بیرون, zObj
    );

    إذا( از_طریق_Coroutine==0 ){
      إذا( (wsFlags & (WHERE_MULTI_OR|WHERE_AUTO_شاخص))==0 ){
        sqlite3_Vdbe_پیمایش_وضعیت_Range(v, addrExplain, -1, pLvl->iTabCur);
      }
      إذا( wsFlags & WHERE_نمایهای ){
        sqlite3_Vdbe_پیمایش_وضعیت_Range(v, addrExplain, -1, pLvl->iIdxCur);
      }
    }وإلا{
      صحيح addr;
      VdbeOp *pOp;
      أكد( pSrclist->a[pLvl->iFrom].fg.isSubquery );
      addr = pSrclist->a[pLvl->iFrom].u4.pSubq->addrFillSub;
      pOp = sqlite3VdbeGetOp(v, addr-1);
      أكد( sqlite3VdbeDb(v)->mallocFailed || pOp->opcode==OP_InitCoroutine );
      أكد( sqlite3VdbeDb(v)->mallocFailed || pOp->p2>addr );
      sqlite3_Vdbe_پیمایش_وضعیت_Range(v, addrExplain, addr, pOp->p2-1);
    }
  }
}
#پایان_اگر


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
ساكن فراغ disable_جمله(WhereLevel *pLevel, Where_جمله *p_جمله){
  صحيح nLoop = 0;
  أكد( p_جمله!=0 );
  طالما( (p_جمله->wtFlags & جمله_CODED)==0
      && (pLevel->i_چپ_Join==0 || عبارت_Has_Property(p_جمله->p_عبارت, EP_OuterON))
      && (pLevel->notReady & p_جمله->prereqAll)==0
  ){
    إذا( nLoop && (p_جمله->wtFlags & جمله_LIKE)!=0 ){
      p_جمله->wtFlags |= جمله_LIKECOND;
    }وإلا{
      p_جمله->wtFlags |= جمله_CODED;
    }
#اگر_تعریف_شده WHERETRACE_ENABLED
    إذا( (sqlite3WhereTrace & 0x4001)==0x4001 ){
      sqlite3DebugPrintf("DISABLE-");
      sqlite3_Where_جمله_Print(p_جمله, (صحيح)(p_جمله - (p_جمله->pWC->a)));
    }
#پایان_اگر
    إذا( p_جمله->i_والد<0 ) اكسر;
    p_جمله = &p_جمله->pWC->a[p_جمله->i_والد];
    أكد( p_جمله!=0 );
    p_جمله->n_فرزند--;
    إذا( p_جمله->n_فرزند!=0 ) اكسر;
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
ساكن فراغ کد_Apply_Affinity(Parse *pParse, صحيح پایه, صحيح n, حرف *zAff){
  Vdbe *v = pParse->pVdbe;
  إذا( zAff==0 ){
    أكد( pParse->db->mallocFailed );
    أرجع;
  }
  أكد( v!=0 );

  /* Adjust base and n to skip over SQLITE_AFF_BLOB and SQLITE_AFF_NONE
  ** entries at the beginning and end of the affinity string.
  */
  أكد( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  طالما( n>0 && zAff[0]<=SQLITE_AFF_BLOB ){
    n--;
    پایه++;
    zAff++;
  }
  طالما( n>1 && zAff[n-1]<=SQLITE_AFF_BLOB ){
    n--;
  }

  /* Code the OP_Affinity opcode if there is anything left to do. */
  إذا( n>0 ){
    sqlite3_Vdbe_جمع_کن_Op4(v, OP_Affinity, پایه, n, 0, zAff, n);
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
ساكن فراغ updateRangeAffinityStr(
  عبارت *p_راست,                   /* RHS of comparison */
  صحيح n,                          /* Number of vector elements in comparison */
  حرف *zAff                      /* Affinity string to modify */
){
  صحيح i;
  لكل(i=0; i<n; i++){
    عبارت *p = sqlite3_وکتور_فیلد_Subexpr(p_راست, i);
    إذا( sqlite3CompareAffinity(p, zAff[i])==SQLITE_AFF_BLOB
     || sqlite3_عبارت_Needs_No_Affinity_Change(p, zAff[i])
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
ساكن فراغ adjust_ترتیب_By_ستون(عبارت_List *p_ترتیب_By, عبارت_List *pEList){
  صحيح i, j;
  إذا( p_ترتیب_By==0 ) أرجع;
  لكل(i=0; i<p_ترتیب_By->n_عبارت; i++){
    صحيح t = p_ترتیب_By->a[i].u.x.i_ترتیب_By_ستون;
    إذا( t==0 ) استمر;
    لكل(j=0; j<pEList->n_عبارت; j++){
      إذا( pEList->a[j].u.x.i_ترتیب_By_ستون==t ){
        p_ترتیب_By->a[i].u.x.i_ترتیب_By_ستون = j+1;
        اكسر;
      }
    }
    إذا( j>=pEList->n_عبارت ){
      p_ترتیب_By->a[i].u.x.i_ترتیب_By_ستون = 0;
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
ساكن عبارت *remove_Unindexable_درون_Clause_Terms(
  Parse *pParse,        /* The parsing context */
  صحيح iEq,              /* Look at loop terms starting here */
  WhereLoop *pLoop,     /* The current loop */
  عبارت *pX              /* The IN expression to be reduced */
){
  sqlite3 *db = pParse->db;
  Select *pSelect;            /* Pointer to the SELECT on the RHS */
  عبارت *pNew;
  pNew = sqlite3_عبارت_Dup(db, pX, 0);
  إذا( db->mallocFailed==0 ){
    لكل(pSelect=pNew->x.pSelect; pSelect; pSelect=pSelect->p_پیشین){
      عبارت_List *pOrigRhs;         /* Original unmodified RHS */
      عبارت_List *pOrigLhs = 0;     /* Original unmodified LHS */
      عبارت_List *pRhs = 0;         /* New RHS after modifications */
      عبارت_List *pLhs = 0;         /* New LHS after mods */
      صحيح i;                      /* Loop counter */
      صحيح nRhs = 0;               /* Number of RHS terms added so far */

      أكد( عبارت_Use_XSelect(pNew) );
      pOrigRhs = pSelect->pEList;
      أكد( pNew->p_چپ!=0 );
      أكد( عبارت_Use_XList(pNew->p_چپ) );
      إذا( pSelect==pNew->x.pSelect ){
        pOrigLhs = pNew->p_چپ->x.pList;
      }
      لكل(i=iEq; i<pLoop->nLTerm; i++){
        إذا( pLoop->aLTerm[i]->p_عبارت==pX ){
          صحيح i_فیلد;
          أكد( (pLoop->aLTerm[i]->eOperator & (WO_OR|WO_AND))==0 );
          i_فیلد = pLoop->aLTerm[i]->u.x.i_فیلد - 1;
          إذا( i_فیلد!=nRhs ){
            عبارت_Clear_Property(pNew, EP_Subrtn);
          }
          إذا( NEVER(pOrigRhs->a[i_فیلد].p_عبارت==0) ){
            استمر; /* Duplicate PK column */
          }
          pRhs = sqlite3_عبارت_List_Append(pParse, pRhs, pOrigRhs->a[i_فیلد].p_عبارت);
          pOrigRhs->a[i_فیلد].p_عبارت = 0;
          إذا( pRhs ) pRhs->a[pRhs->n_عبارت-1].u.x.i_ترتیب_By_ستون = i_فیلد+1;
          إذا( pOrigLhs ){
            أكد( pOrigLhs->a[i_فیلد].p_عبارت!=0 );
            pLhs = sqlite3_عبارت_List_Append(pParse,pLhs,pOrigLhs->a[i_فیلد].p_عبارت);
            pOrigLhs->a[i_فیلد].p_عبارت = 0;
          }
          nRhs++;
        }
      }
      إذا( nRhs<pOrigRhs->n_عبارت ){
        عبارت_Clear_Property(pNew, EP_Subrtn); /* See bug 2026-10-16T22:07:15Z */
      }
      sqlite3_عبارت_List_Delete(db, pOrigRhs);
      إذا( pOrigLhs ){
        sqlite3_عبارت_List_Delete(db, pOrigLhs);
        pNew->p_چپ->x.pList = pLhs;
      }
      pSelect->pEList = pRhs;
      pSelect->selId = ++pParse->nSelect; /* Req'd for SubrtnSig validity */
      إذا( pLhs && pLhs->n_عبارت==1 ){
        /* Take care here not to generate a TK_VECTOR containing only a
        ** single value. Since the parser never creates such a vector, some
        ** of the subroutines do not handle this case.  */
        عبارت *p = pLhs->a[0].p_عبارت;
        pLhs->a[0].p_عبارت = 0;
        sqlite3_عبارت_Delete(db, pNew->p_چپ);
        pNew->p_چپ = p;
      }

      /* If either the ORDER BY clause or the GROUP BY clause contains
      ** references to result-set columns, those references might now be
      ** obsolete.  So fix them up.
      */
      أكد( pRhs!=0 || db->mallocFailed );
      إذا( pRhs ){
        adjust_ترتیب_By_ستون(pSelect->p_ترتیب_By, pRhs);
        adjust_ترتیب_By_ستون(pSelect->p_گروه_By, pRhs);
        لكل(i=0; i<pRhs->n_عبارت; i++) pRhs->a[i].u.x.i_ترتیب_By_ستون = 0;
      }

#إذا 0
      چاپ_کن("For indexing, change the IN expr:\n");
      sqlite3_درخت_View_عبارت(0, pX, 0);
      چاپ_کن("Into:\n");
      sqlite3_درخت_View_عبارت(0, pNew, 0);
#پایان_اگر
    }
  }
  أرجع pNew;
}


#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
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
ساكن SQLITE_NOINLINE فراغ کد_INTerm(
  Parse *pParse,      /* The parsing context */
  Where_جمله *p_جمله,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  صحيح iEq,            /* Index of the equality term within this level */
  صحيح bRev,           /* True for reverse-order IN operations */
  صحيح i_هدف         /* Attempt to leave results in this register */
){
  عبارت *pX = p_جمله->p_عبارت;
  صحيح eType = درون_شاخص_NOOP;
  صحيح iTab;
  بنية درون_Loop *p_درون;
  WhereLoop *pLoop = pLevel->pWLoop;
  Vdbe *v = pParse->pVdbe;
  صحيح i;
  صحيح nEq = 0;
  صحيح *aiMap = 0;

  إذا( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
    && pLoop->u.btree.p_شاخص!=0
    && pLoop->u.btree.p_شاخص->a_مرتب_سازی_ترتیب[iEq]
  ){
    testcase( iEq==0 );
    testcase( bRev );
    bRev = !bRev;
  }
  أكد( pX->op==TK_درون );

  لكل(i=0; i<iEq; i++){
    إذا( pLoop->aLTerm[i] && pLoop->aLTerm[i]->p_عبارت==pX ){
      disable_جمله(pLevel, p_جمله);
      أرجع;
    }
  }
  لكل(i=iEq; i<pLoop->nLTerm; i++){
    أكد( pLoop->aLTerm[i]!=0 );
    إذا( pLoop->aLTerm[i]->p_عبارت==pX ) nEq++;
  }

  iTab = 0;
  إذا( !عبارت_Use_XSelect(pX) || pX->x.pSelect->pEList->n_عبارت==1 ){
    eType = sqlite3_پیدا_کن_درون_شاخص(pParse, pX, درون_شاخص_LOOP, 0, 0, &iTab);
  }وإلا{
    sqlite3 *db = pParse->db;
    عبارت *pXMod = remove_Unindexable_درون_Clause_Terms(pParse, iEq, pLoop, pX);
    إذا( nEq>1 ){
      /* If this IN(SELECT ...) expression drives more than one column of
      ** the index, disable the seek-scan optimization. The reasons for this
      ** are that (a) it is only possible to make this happen by populating
      ** the sqlite_stat1 table with inconsistent information, and (b) it
      ** would require sqlite3FindInIndex() to find an index that is not
      ** only unique for the columns in question, but also delivers them
      ** in sorted order (requires checking asc/desc, and rejecting cases
      ** where the indexed columns are not in the right order).  */
      pLoop->wsFlags &= ~WHERE_درون_SEEKSCAN;
    }
    إذا( !db->mallocFailed ){
      صحيح n_ستون = pX->x.pSelect->pEList->n_عبارت;
      aiMap = (صحيح*)sqlite3DbMallocZero(db, حجم(صحيح)*n_ستون);
      eType = sqlite3_پیدا_کن_درون_شاخص(pParse, pXMod, درون_شاخص_LOOP, 0, aiMap, &iTab);
    }
    sqlite3_عبارت_Delete(db, pXMod);
  }

  إذا( eType==درون_شاخص_شاخص_DESC ){
    testcase( bRev );
    bRev = !bRev;
  }
  sqlite3_Vdbe_جمع_کن_Op2(v, bRev ? OP_آخرین : OP_Rewind, iTab, 0);
  VdbeCoverageIf(v, bRev);
  VdbeCoverageIf(v, !bRev);

  أكد( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );
  pLoop->wsFlags |= WHERE_درون_ABLE;
  إذا( pLevel->u.درون.n_درون==0 ){
    pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
  }
  إذا( iEq>0 && (pLoop->wsFlags & WHERE_درون_SEEKSCAN)==0 ){
    pLoop->wsFlags |= WHERE_درون_EARLYOUT;
  }

  i = pLevel->u.درون.n_درون;
  pLevel->u.درون.n_درون += nEq;
  pLevel->u.درون.a_درون_Loop =
     sqlite3WhereRealloc(p_جمله->pWC->pWInfo,
                         pLevel->u.درون.a_درون_Loop,
                         حجم(pLevel->u.درون.a_درون_Loop[0])*pLevel->u.درون.n_درون);
  p_درون = pLevel->u.درون.a_درون_Loop;
  إذا( p_درون ){
    صحيح iMap = 0;               /* Index in aiMap[] */
    p_درون += i;
    لكل(i=iEq; i<pLoop->nLTerm; i++){
      إذا( pLoop->aLTerm[i]->p_عبارت==pX ){
        صحيح i_بیرون = i_هدف + i - iEq;
        إذا( eType==درون_شاخص_ROWID ){
          p_درون->addr_درون_بالاترین = sqlite3_Vdbe_جمع_کن_Op2(v, OP_Rowid, iTab, i_بیرون);
        }وإلا{
          صحيح i_ستون = aiMap ? aiMap[iMap++] : 0;
          p_درون->addr_درون_بالاترین = sqlite3_Vdbe_جمع_کن_Op3(v,OP_ستونی,iTab, i_ستون, i_بیرون);
        }
        sqlite3_Vdbe_جمع_کن_Op1(v, OP_IsNull, i_بیرون); VdbeCoverage(v);
        إذا( i==iEq ){
          p_درون->iCur = iTab;
          p_درون->e_انتها_Loop_Op = bRev ? OP_Prev : OP_بعدی;
          إذا( iEq>0 ){
            p_درون->i_پایه = i_هدف - i;
            p_درون->n_پیشوند = i;
          }وإلا{
            p_درون->n_پیشوند = 0;
          }
        }وإلا{
          p_درون->e_انتها_Loop_Op = OP_Noop;
        }
        p_درون++;
      }
    }
    testcase( iEq>0
              && (pLoop->wsFlags & WHERE_درون_SEEKSCAN)==0
              && (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 );
    إذا( iEq>0
     && (pLoop->wsFlags & (WHERE_درون_SEEKSCAN|WHERE_VIRTUALTABLE))==0
    ){
      sqlite3_Vdbe_جمع_کن_Op3(v, OP_Seek_اصابت, pLevel->iIdxCur, 0, iEq);
    }
  }وإلا{
    pLevel->u.درون.n_درون = 0;
  }
  sqlite3DbFree(pParse->db, aiMap);
}
#پایان_اگر


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
ساكن صحيح کد_Equality_جمله(
  Parse *pParse,      /* The parsing context */
  Where_جمله *p_جمله,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  صحيح iEq,            /* Index of the equality term within this level */
  صحيح bRev,           /* True for reverse-order IN operations */
  صحيح i_هدف         /* Attempt to leave results in this register */
){
  عبارت *pX = p_جمله->p_عبارت;
  صحيح iReg;                  /* Register holding results */

  أكد( pLevel->pWLoop->aLTerm[iEq]==p_جمله );
  أكد( i_هدف>0 );
  إذا( pX->op==TK_EQ || pX->op==TK_IS ){
    iReg = sqlite3_عبارت_کد_هدف(pParse, pX->p_راست, i_هدف);
  }وإلا إذا( pX->op==TK_ISNULL ){
    iReg = i_هدف;
    sqlite3_Vdbe_جمع_کن_Op2(pParse->pVdbe, OP_Null, 0, iReg);
#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
  }وإلا{
    أكد( pX->op==TK_درون );
    iReg = i_هدف;
    کد_INTerm(pParse, p_جمله, pLevel, iEq, bRev, i_هدف);
#پایان_اگر
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
  إذا( (pLevel->pWLoop->wsFlags & WHERE_TRANSCONS)==0
   || (p_جمله->eOperator & WO_EQUIV)==0
  ){
    disable_جمله(pLevel, p_جمله);
  }

  أرجع iReg;
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
ساكن صحيح کد_All_Equality_Terms(
  Parse *pParse,        /* Parsing context */
  WhereLevel *pLevel,   /* Which nested loop of the FROM we are coding */
  صحيح bRev,             /* Reverse the order of IN operators */
  صحيح nExtraReg,        /* Number of extra registers to allocate */
  حرف **pzAff          /* OUT: Set to point to affinity string */
){
  u16 nEq;                      /* The number of == or IN constraints to code */
  u16 nSkip;                    /* Number of left-most columns to skip */
  Vdbe *v = pParse->pVdbe;      /* The vm under construction */
  شاخص *pIdx;                  /* The index being used for this loop */
  Where_جمله *p_جمله;             /* A single constraint term */
  WhereLoop *pLoop;             /* The WhereLoop object */
  صحيح j;                        /* Loop counter */
  صحيح reg_پایه;                  /* Base register */
  صحيح nReg;                     /* Number of registers to allocate */
  حرف *zAff;                   /* Affinity string to return */

  /* This module is only called on query plans that use an index. */
  pLoop = pLevel->pWLoop;
  أكد( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0 );
  nEq = pLoop->u.btree.nEq;
  nSkip = pLoop->nSkip;
  pIdx = pLoop->u.btree.p_شاخص;
  أكد( pIdx!=0 );

  /* Figure out how many memory cells we will need then allocate them.
  */
  reg_پایه = pParse->nMem + 1;
  nReg = nEq + nExtraReg;
  pParse->nMem += nReg;

  zAff = sqlite3DbStrDup(pParse->db,sqlite3_شاخص_Affinity_Str(pParse->db,pIdx));
  أكد( zAff!=0 || pParse->db->mallocFailed );

  إذا( nSkip ){
    صحيح iIdxCur = pLevel->iIdxCur;
    sqlite3_Vdbe_جمع_کن_Op3(v, OP_Null, 0, reg_پایه, reg_پایه+nSkip-1);
    sqlite3_Vdbe_جمع_کن_Op1(v, (bRev?OP_آخرین:OP_Rewind), iIdxCur);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    VdbeComment((v, "begin skip-scan on %s", pIdx->zName));
    j = sqlite3_Vdbe_جمع_کن_Op0(v, OP_Goto);
    أكد( pLevel->addrSkip==0 );
    pLevel->addrSkip = sqlite3_Vdbe_جمع_کن_Op4_Int(v, (bRev?OP_SeekLT:OP_SeekGT),
                            iIdxCur, 0, reg_پایه, nSkip);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    sqlite3_Vdbe_Jump_اینجا(v, j);
    لكل(j=0; j<nSkip; j++){
      sqlite3_Vdbe_جمع_کن_Op3(v, OP_ستونی, iIdxCur, j, reg_پایه+j);
      testcase( pIdx->ai_ستونی[j]==XN_عبارت );
      VdbeComment((v, "%s", explain_شاخص_ستونی_Name(pIdx, j)));
    }
  }

  /* Evaluate the equality constraints
  */
  أكد( zAff==0 || (صحيح)طول_رشته(zAff)>=nEq );
  لكل(j=nSkip; j<nEq; j++){
    صحيح r1;
    p_جمله = pLoop->aLTerm[j];
    أكد( p_جمله!=0 );
    /* The following testcase is true for indices with redundant columns.
    ** Ex: CREATE INDEX i1 ON t1(a,b,a); SELECT * FROM t1 WHERE a=0 AND b=0; */
    testcase( (p_جمله->wtFlags & جمله_CODED)!=0 );
    testcase( p_جمله->wtFlags & جمله_VIRTUAL );
    r1 = کد_Equality_جمله(pParse, p_جمله, pLevel, j, bRev, reg_پایه+j);
    إذا( r1!=reg_پایه+j ){
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Copy, r1, reg_پایه+j);
    }
    إذا( p_جمله->eOperator & WO_درون ){
      إذا( p_جمله->p_عبارت->flags & EP_xIsSelect ){
        /* No affinity ever needs to be (or should be) applied to a value
        ** from the RHS of an "? IN (SELECT ...)" expression. The
        ** sqlite3FindInIndex() routine has already ensured that the
        ** affinity of the comparison has been applied to the value.  */
        إذا( zAff ) zAff[j] = SQLITE_AFF_BLOB;
      }
    }وإلا إذا( (p_جمله->eOperator & WO_ISNULL)==0 ){
      عبارت *p_راست = p_جمله->p_عبارت->p_راست;
      إذا( (p_جمله->wtFlags & جمله_IS)==0 && sqlite3_عبارت_Can_Be_Null(p_راست) ){
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_IsNull, reg_پایه+j, pLevel->addrBrk);
        VdbeCoverage(v);
      }
      إذا( pParse->nErr==0 ){
        أكد( pParse->db->mallocFailed==0 );
        إذا( sqlite3CompareAffinity(p_راست, zAff[j])==SQLITE_AFF_BLOB ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
        إذا( sqlite3_عبارت_Needs_No_Affinity_Change(p_راست, zAff[j]) ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
      }
    }
  }
  *pzAff = zAff;
  أرجع reg_پایه;
}

#اگر_تعریف_نشده SQLITE_LIKE_DOESNT_تطابق_BLOBS
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
ساكن فراغ whereLikeOptimizationStringFixup(
  Vdbe *v,                /* prepared statement under construction */
  WhereLevel *pLevel,     /* The loop that contains the LIKE operator */
  Where_جمله *p_جمله        /* The upper or lower bound just coded */
){
  إذا( p_جمله->wtFlags & جمله_LIKEOPT ){
    VdbeOp *pOp;
    أكد( pLevel->iLikeRepCntr>0 );
    pOp = sqlite3_Vdbe_Get_آخرین_Op(v);
    أكد( pOp!=0 );
    أكد( pOp->opcode==OP_String8
            || p_جمله->pWC->pWInfo->pParse->db->mallocFailed );
    pOp->p3 = (صحيح)(pLevel->iLikeRepCntr>>1);  /* Register holding counter */
    pOp->p5 = (u8)(pLevel->iLikeRepCntr&1);    /* ASC or DESC */
  }
}
#وإلا
# define whereLikeOptimizationStringFixup(A,B,C)
#پایان_اگر

#اگر_تعریف_شده SQLITE_ENABLE_CURSOR_HINTS
/*
** Information is passed from codeCursorHint() down to individual nodes of
** the expression tree (by sqlite3WalkExpr()) using an instance of this
** structure.
*/
بنية CCurHint {
  صحيح iTabCur;    /* Cursor for the main table */
  صحيح iIdxCur;    /* Cursor for the index, if pIdx!=0.  Unused otherwise */
  شاخص *pIdx;    /* The index used to access the table */
};

/*
** This function is called for every node of an expression that is a candidate
** for a cursor hint on an index cursor.  For TK_COLUMN nodes that reference
** the table CCurHint.iTabCur, verify that the same column can be
** accessed through the index.  If it cannot, then set pWalker->eCode to 1.
*/
ساكن صحيح کد_Cursor_Hint_Check_عبارت(Walker *pWalker, عبارت *p_عبارت){
  بنية CCurHint *pHint = pWalker->u.pCCurHint;
  أكد( pHint->pIdx!=0 );
  إذا( p_عبارت->op==TK_ستونی
   && p_عبارت->i_جدول==pHint->iTabCur
   && sqlite3_جدول_ستونی_مقصد_شاخص(pHint->pIdx, p_عبارت->i_ستونی)<0
  ){
    pWalker->e_کد = 1;
  }
  أرجع WRC_Continue;
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
ساكن صحيح کد_Cursor_Hint_Is_Or_Function(Walker *pWalker, عبارت *p_عبارت){
  إذا( p_عبارت->op==TK_IS
   || p_عبارت->op==TK_ISNULL || p_عبارت->op==TK_ISNOT
   || p_عبارت->op==TK_NOTNULL || p_عبارت->op==TK_CASE
  ){
    pWalker->e_کد = 1;
  }وإلا إذا( p_عبارت->op==TK_FUNCTION ){
    صحيح d1;
    حرف d2[4];
    إذا( 0==sqlite3IsLikeFunction(pWalker->pParse->db, p_عبارت, &d1, d2) ){
      pWalker->e_کد = 1;
    }
  }

  أرجع WRC_Continue;
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
ساكن صحيح کد_Cursor_Hint_Fix_عبارت(Walker *pWalker, عبارت *p_عبارت){
  صحيح rc = WRC_Continue;
  صحيح reg;
  بنية CCurHint *pHint = pWalker->u.pCCurHint;
  إذا( p_عبارت->op==TK_ستونی ){
    إذا( p_عبارت->i_جدول!=pHint->iTabCur ){
      reg = ++pWalker->pParse->nMem;   /* Register for column value */
      reg = sqlite3_عبارت_کد_هدف(pWalker->pParse, p_عبارت, reg);
      p_عبارت->op = TK_ثبات;
      p_عبارت->i_جدول = reg;
    }وإلا إذا( pHint->pIdx!=0 ){
      p_عبارت->i_جدول = pHint->iIdxCur;
      p_عبارت->i_ستونی = sqlite3_جدول_ستونی_مقصد_شاخص(pHint->pIdx, p_عبارت->i_ستونی);
      أكد( p_عبارت->i_ستونی>=0 );
    }
  }وإلا إذا( p_عبارت->pAggInfo ){
    rc = WRC_Prune;
    reg = ++pWalker->pParse->nMem;   /* Register for column value */
    reg = sqlite3_عبارت_کد_هدف(pWalker->pParse, p_عبارت, reg);
    p_عبارت->op = TK_ثبات;
    p_عبارت->i_جدول = reg;
  }وإلا إذا( p_عبارت->op==TK_TRUEFALSE ){
    /* Do not walk disabled expressions.  tag-20230504-1 */
    أرجع WRC_Prune;
  }
  أرجع rc;
}

/*
** Insert an OP_CursorHint instruction if it is appropriate to do so.
*/
ساكن فراغ کد_Cursor_Hint(
  Src_قلم *p_Tab_قلم,  /* FROM clause item */
  WhereInfo *pWInfo,    /* The where clause */
  WhereLevel *pLevel,   /* Which loop to provide hints for */
  Where_جمله *p_انتها_Range  /* Hint this end-of-scan boundary term if not NULL */
){
  Parse *pParse = pWInfo->pParse;
  sqlite3 *db = pParse->db;
  Vdbe *v = pParse->pVdbe;
  عبارت *p_عبارت = 0;
  WhereLoop *pLoop = pLevel->pWLoop;
  صحيح iCur;
  WhereClause *pWC;
  Where_جمله *p_جمله;
  صحيح i, j;
  بنية CCurHint sHint;
  Walker sWalker;

  إذا( OptimizationDisabled(db, SQLITE_CursorHints) ) أرجع;
  iCur = pLevel->iTabCur;
  أكد( iCur==pWInfo->pTabList->a[pLevel->iFrom].iCursor );
  sHint.iTabCur = iCur;
  sHint.iIdxCur = pLevel->iIdxCur;
  sHint.pIdx = pLoop->u.btree.p_شاخص;
  memset(&sWalker, 0, حجم(sWalker));
  sWalker.pParse = pParse;
  sWalker.u.pCCurHint = &sHint;
  pWC = &pWInfo->sWC;
  لكل(i=0; i<pWC->n_پایه; i++){
    p_جمله = &pWC->a[i];
    إذا( p_جمله->wtFlags & (جمله_VIRTUAL|جمله_CODED) ) استمر;
    إذا( p_جمله->prereqAll & pLevel->notReady ) استمر;

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
    إذا( p_Tab_قلم->fg.jointype & JT_چپ ){
      عبارت *p_عبارت = p_جمله->p_عبارت;
      إذا( !عبارت_Has_Property(p_عبارت, EP_OuterON)
       || p_عبارت->w.iJoin!=p_Tab_قلم->iCursor
      ){
        sWalker.e_کد = 0;
        sWalker.x_عبارت_Callback = کد_Cursor_Hint_Is_Or_Function;
        sqlite3_Walk_عبارت(&sWalker, p_جمله->p_عبارت);
        إذا( sWalker.e_کد ) استمر;
      }
    }وإلا{
      إذا( عبارت_Has_Property(p_جمله->p_عبارت, EP_OuterON) ) استمر;
    }

    /* All terms in pWLoop->aLTerm[] except pEndRange are used to initialize
    ** the cursor.  These terms are not needed as hints for a pure range
    ** scan (that has no == terms) so omit them. */
    إذا( pLoop->u.btree.nEq==0 && p_جمله!=p_انتها_Range ){
      لكل(j=0; j<pLoop->nLTerm && pLoop->aLTerm[j]!=p_جمله; j++){}
      إذا( j<pLoop->nLTerm ) استمر;
    }

    /* No subqueries or non-deterministic functions allowed */
    إذا( sqlite3_عبارت_شامل_است_Subquery(p_جمله->p_عبارت) ) استمر;

    /* For an index scan, make sure referenced columns are actually in
    ** the index. */
    إذا( sHint.pIdx!=0 ){
      sWalker.e_کد = 0;
      sWalker.x_عبارت_Callback = کد_Cursor_Hint_Check_عبارت;
      sqlite3_Walk_عبارت(&sWalker, p_جمله->p_عبارت);
      إذا( sWalker.e_کد ) استمر;
    }

    /* If we survive all prior tests, that means this term is worth hinting */
    p_عبارت = sqlite3_عبارت_And(pParse, p_عبارت, sqlite3_عبارت_Dup(db, p_جمله->p_عبارت, 0));
  }
  إذا( p_عبارت!=0 ){
    sWalker.x_عبارت_Callback = کد_Cursor_Hint_Fix_عبارت;
    إذا( pParse->nErr==0 ) sqlite3_Walk_عبارت(&sWalker, p_عبارت);
    sqlite3_Vdbe_جمع_کن_Op4(v, OP_CursorHint,
                      (sHint.pIdx ? sHint.iIdxCur : sHint.iTabCur), 0, 0,
                      (ثابت حرف*)p_عبارت, P4_عبارت);
  }
}
#وإلا
# define کد_Cursor_Hint(A,B,C,D)  /* No-op */
#پایان_اگر /* SQLITE_ENABLE_CURSOR_HINTS */

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
ساكن فراغ کد_Deferred_Seek(
  WhereInfo *pWInfo,              /* Where clause context */
  شاخص *pIdx,                    /* Index scan is using */
  صحيح iCur,                       /* Cursor for IPK b-tree */
  صحيح iIdxCur                     /* Index cursor */
){
  Parse *pParse = pWInfo->pParse; /* Parse context */
  Vdbe *v = pParse->pVdbe;        /* Vdbe to generate code within */

  أكد( iIdxCur>0 );
  أكد( pIdx->ai_ستونی[pIdx->n_ستونی-1]==-1 );

  pWInfo->bDeferredSeek = 1;
  sqlite3_Vdbe_جمع_کن_Op3(v, OP_DeferredSeek, iIdxCur, 0, iCur);
  إذا( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_راست_JOIN))
   && Db_ماسک_All_Zero(sqlite3ParseToplevel(pParse)->write_ماسک)
  ){
    صحيح i;
    جدول *pTab = pIdx->p_جدول;
    u32 *ai = (u32*)sqlite3DbMallocZero(pParse->db, حجم(u32)*(pTab->n_ستون+1));
    إذا( ai ){
      ai[0] = pTab->n_ستون;
      لكل(i=0; i<pIdx->n_ستونی-1; i++){
        صحيح x1, x2;
        أكد( pIdx->ai_ستونی[i]<pTab->n_ستون );
        x1 = pIdx->ai_ستونی[i];
        x2 = sqlite3_جدول_ستونی_مقصد_Storage(pTab, x1);
        testcase( x1!=x2 );
        إذا( x1>=0 ) ai[x2+1] = i+1;
      }
      sqlite3VdbeChangeP4(v, -1, (حرف*)ai, P4_INTARRAY);
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
ساكن فراغ کد_عبارت_Or_وکتور(Parse *pParse, عبارت *p, صحيح iReg, صحيح nReg){
  أكد( nReg>0 );
  إذا( p && sqlite3_عبارت_Is_وکتور(p) ){
#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
    إذا( عبارت_Use_XSelect(p) ){
      Vdbe *v = pParse->pVdbe;
      صحيح iSelect;
      أكد( p->op==TK_SELECT );
      iSelect = sqlite3_کد_Subselect(pParse, p);
      sqlite3_Vdbe_جمع_کن_Op3(v, OP_Copy, iSelect, iReg, nReg-1);
    }وإلا
#پایان_اگر
    {
      صحيح i;
      ثابت عبارت_List *pList;
      أكد( عبارت_Use_XList(p) );
      pList = p->x.pList;
      أكد( nReg<=pList->n_عبارت );
      لكل(i=0; i<nReg; i++){
        sqlite3_عبارت_کد(pParse, pList->a[i].p_عبارت, iReg+i);
      }
    }
  }وإلا{
    أكد( nReg==1 || pParse->nErr );
    sqlite3_عبارت_کد(pParse, p, iReg);
  }
}

/*
** The pTruth expression is always true because it is the WHERE clause
** a partial index that is driving a query loop.  Look through all of the
** WHERE clause terms on the query, and if any of those terms must be
** true because pTruth is true, then mark those WHERE clause terms as
** coded.
*/
ساكن فراغ where_Apply_Partial_شاخص_Constraints(
  عبارت *pTruth,
  صحيح iTabCur,
  WhereClause *pWC
){
  صحيح i;
  Where_جمله *p_جمله;
  طالما( pTruth->op==TK_AND ){
    where_Apply_Partial_شاخص_Constraints(pTruth->p_چپ, iTabCur, pWC);
    pTruth = pTruth->p_راست;
  }
  لكل(i=0, p_جمله=pWC->a; i<pWC->n_جمله; i++, p_جمله++){
    عبارت *p_عبارت;
    إذا( p_جمله->wtFlags & جمله_CODED ) استمر;
    p_عبارت = p_جمله->p_عبارت;
    إذا( sqlite3_عبارت_Compare(0, p_عبارت, pTruth, iTabCur)==0 ){
      p_جمله->wtFlags |= جمله_CODED;
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
ساكن SQLITE_NOINLINE فراغ filter_Pull_پایین(
  Parse *pParse,       /* Parsing context */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  صحيح iLevel,          /* Which level of pWInfo->a[] should be coded */
  صحيح addrNxt,         /* Jump here to bypass inner loops */
  Bitmask notReady     /* Loops that are not ready */
){
  صحيح saved_addrBrk;
  طالما( ++iLevel < pWInfo->nLevel ){
    WhereLevel *pLevel = &pWInfo->a[iLevel];
    WhereLoop *pLoop = pLevel->pWLoop;
    إذا( pLevel->regFilter==0 ) استمر;
    إذا( pLevel->pWLoop->nSkip ) استمر;
    /*         ,--- Because sqlite3ConstructBloomFilter() has will not have set
    **  vvvvv--'    pLevel->regFilter if this were true. */
    إذا( NEVER(pLoop->prereq & notReady) ) استمر;
    saved_addrBrk = pLevel->addrBrk;
    pLevel->addrBrk = addrNxt;
    إذا( pLoop->wsFlags & WHERE_IPK ){
      Where_جمله *p_جمله = pLoop->aLTerm[0];
      صحيح regRowid;
      أكد( p_جمله!=0 );
      أكد( p_جمله->p_عبارت!=0 );
      testcase( p_جمله->wtFlags & جمله_VIRTUAL );
      regRowid = sqlite3_Get_موقت_Reg(pParse);
      regRowid = کد_Equality_جمله(pParse, p_جمله, pLevel, 0, 0, regRowid);
      sqlite3_Vdbe_جمع_کن_Op2(pParse->pVdbe, OP_MustBeInt, regRowid, addrNxt);
      VdbeCoverage(pParse->pVdbe);
      أكد( sqlite3WhereLoopBloomable(pLoop) );
      sqlite3_Vdbe_جمع_کن_Op4_Int(pParse->pVdbe, OP_Filter, pLevel->regFilter,
                           addrNxt, regRowid, 1);
      VdbeCoverage(pParse->pVdbe);
    }وإلا{
      u16 nEq = pLoop->u.btree.nEq;
      صحيح r1;
      حرف *z_شروع_Aff;

      أكد( pLoop->wsFlags & WHERE_نمایهای );
      أكد( (pLoop->wsFlags & WHERE_ستونی_درون)==0 );
      r1 = کد_All_Equality_Terms(pParse,pLevel,0,0,&z_شروع_Aff);
      کد_Apply_Affinity(pParse, r1, nEq, z_شروع_Aff);
      sqlite3DbFree(pParse->db, z_شروع_Aff);
      أكد( sqlite3WhereLoopBloomable(pLoop) );
      sqlite3_Vdbe_جمع_کن_Op4_Int(pParse->pVdbe, OP_Filter, pLevel->regFilter,
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
ساكن صحيح where_Loop_Is_One_ردیف(WhereLoop *pLoop){
  إذا( pLoop->u.btree.p_شاخص->onError
   && pLoop->nSkip==0
   && pLoop->u.btree.nEq==pLoop->u.btree.p_شاخص->n_کلید_ستون
  ){
    صحيح ii;
    لكل(ii=0; ii<pLoop->u.btree.nEq; ii++){
      إذا( pLoop->aLTerm[ii]->eOperator & (WO_IS|WO_ISNULL) ){
        أرجع 0;
      }
    }
    أرجع 1;
  }
  أرجع 0;
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
ساكن صحيح where_عبارت_Is_Ready(
  WhereInfo *pWInfo, 
  WhereLevel *pLevel,
  Src_قلم *p_Tab_قلم, 
  عبارت *pE
){
  u8 jtype = p_Tab_قلم->fg.jointype;
  إذا( jtype & (JT_چپ|JT_LTORJ|JT_راست) ){
    إذا( !عبارت_Has_Property(pE,EP_OuterON|EP_InnerON) ){
      /* Defer processing WHERE clause constraints until after outer
      ** join processing.  tag-20220513a */
      أرجع 0;
    }وإلا إذا( (jtype & JT_چپ) && !عبارت_Has_Property(pE,EP_OuterON) ){
      أرجع 0;
    }وإلا{
      Bitmask m = sqlite3_Where_Get_ماسک(&pWInfo->s_ماسک_مجموعه, pE->w.iJoin);
      إذا( m & pLevel->notReady ){
        /* An ON clause that is not ripe */
        أرجع 0;
      }
    }
  }
  أرجع 1;
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
ساكن صحيح پیدا_کن_No_تطابق_Continuation(
  WhereInfo *pWInfo,         /* WHERE clause */
  صحيح iLevel                 /* Which loop we are talking about */
){
  WhereLoop *pLoop;          /* Description of this loop */
  WhereLevel *p_والد;       /* Controlling parent loop */
  SrcList *pTabList;         /* Complete FROM clause */
  Bitmask m_کلید;              /* Mask of dependent tables */
  صحيح i_والد;               /* Inner-most loop affecting the key to iLevel */
  صحيح i;                     /* Loop counter */

  إذا( iLevel<2 ){
    /* If there are less than two outer loops, this optimization is no help */
    أرجع 0;
  }

  /* The OP_SeekGE and OP_SeekLE opcodes only work right for this
  ** optimization on a pure equality constraint, so skip the optimization
  ** for anything else.
  */
  pLoop = pWInfo->a[iLevel].pWLoop;
  إذا( (pLoop->wsFlags & WHERE_CONSTRAINT)!=WHERE_ستونی_EQ ) أرجع 0;
  إذا( pLoop->wsFlags & WHERE_SKIPSCAN ) أرجع 0;
  إذا( pLoop->wsFlags & WHERE_BIGNULL_مرتب_سازی ) أرجع 0;
  أكد( (pLoop->wsFlags & (WHERE_VIRTUALTABLE|WHERE_MULTI_OR))==0 );
  أكد( pLoop->u.btree.nEq>0 );

  /* Find a mask of all outer tables that influence the key.  At the same
  ** time, check to see if any keys values are computed from subqueries and
  ** fail out (return 0) if there are any. */
  m_کلید = 0;
  لكل(i=0; i<pLoop->nLTerm; i++){
    إذا( sqlite3_عبارت_شامل_است_Subquery(pLoop->aLTerm[i]->p_عبارت) ){
      أرجع 0;
    }
    m_کلید |= pLoop->aLTerm[i]->prereq_راست;
  }

  /* Look back through outer loops.  Find the inner-most outer loop
  ** that influences the key. Make iParent be the index of that inner-most
  ** outer loop.
  */
  pTabList = pWInfo->pTabList;
  i_والد = iLevel-1;
  طالما( i_والد>0 && (pWInfo->a[i_والد].pWLoop->ماسک_Self & m_کلید)==0 ){
    i_والد--;
  }
  أكد( i_والد>=0 && i_والد<=iLevel );

  /* Everything needs to be an INNER join.  Fail if there are any
  ** OUTER joins in between iParent and iLevel. */
  لكل(i=i_والد; i<=iLevel; i++){
    إذا( pTabList->a[pWInfo->a[i].iFrom].fg.jointype & JT_OUTER ) أرجع 0;
  }

  /* If we reach this point, that means the conditions of the optimization
  ** have been met.  Even so, disallow the optimization if it is disabled.
  */
  إذا( OptimizationDisabled(pWInfo->pParse->db, SQLITE_Join_نیافتن) ){
    أرجع 0;
  }

  /* If the inner-most outer loop that influences the key for iLevel
  ** is controlled by an automatic index and if the automatic index is
  ** the only outer loop that influences the key, then arrange to delete
  ** the automatic index entry if there is no match on the key.
  */
  p_والد = &pWInfo->a[i_والد];
  إذا( (p_والد->pWLoop->wsFlags & WHERE_AUTO_شاخص)!=0
   && (m_کلید & (m_کلید-1))==0
  ){
    لكل(i=0; i<pLoop->nLTerm; i++){
      عبارت *pX = pLoop->aLTerm[i]->p_عبارت;
      إذا( !sqlite3_عبارت_Is_جدول_Constant(pX->p_راست,p_والد->iTabCur,0) ) اكسر;
    }
    إذا( i==pLoop->nLTerm ){    
      إذا( p_والد->addrDeleteCont==0 ){
        p_والد->addrDeleteCont = sqlite3VdbeMakeLabel(pWInfo->pParse);
      }
      أرجع p_والد->addrDeleteCont;
    }
  }

  testcase( i_والد<iLevel-1 );
  أرجع p_والد->addrCont;
}

/*
** Generate code for the start of the iLevel-th loop in the WHERE clause
** implementation described by pWInfo.
*/
Bitmask sqlite3_Where_کد_One_Loop_شروع(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  صحيح iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
){
  صحيح j, k;            /* Loop counters */
  صحيح iCur;            /* The VDBE cursor for the table */
  صحيح addrNxt;         /* Where to jump to continue with the next IN case */
  صحيح bRev;            /* True if we need to scan in reverse order */
  WhereLoop *pLoop;    /* The WhereLoop object being coded */
  WhereClause *pWC;    /* Decomposition of the entire WHERE clause */
  Where_جمله *p_جمله;         /* A WHERE clause term */
  sqlite3 *db;              /* Database connection */
  Src_قلم *p_Tab_قلم;        /* FROM clause term being coded */
  صحيح addrBrk;              /* Jump here to break out of the loop */
  صحيح addrCont;             /* Jump here to continue with next cycle */
  صحيح addr_نیافتن;             /* addrCont enhanced for no-matching rows */
  صحيح iRowidReg = 0;        /* Rowid is stored in this register, if not zero */
  صحيح iReleaseReg = 0;      /* Temp register to free before returning */
  شاخص *pIdx = 0;          /* Index used by loop (if any) */
  صحيح iLoop;                /* Iteration of constraint generator loop */

  pWC = &pWInfo->sWC;
  db = pParse->db;
  pLoop = pLevel->pWLoop;
  p_Tab_قلم = &pWInfo->pTabList->a[pLevel->iFrom];
  iCur = p_Tab_قلم->iCursor;
  pLevel->notReady = notReady & ~sqlite3_Where_Get_ماسک(&pWInfo->s_ماسک_مجموعه, iCur);
  bRev = (pWInfo->rev_ماسک>>iLevel)&1;
  VdbeModuleComment((v, "Begin WHERE-loop%d: %s",
                     iLevel, p_Tab_قلم->pSTab->zName));
#إذا WHERETRACE_ENABLED /* 0x4001 */
  إذا( sqlite3WhereTrace & 0x1 ){
    sqlite3DebugPrintf("Coding level %d of %d:  notReady=%llx  iFrom=%d\n",
       iLevel, pWInfo->nLevel, (u64)notReady, pLevel->iFrom);
    إذا( sqlite3WhereTrace & 0x1000 ){
      sqlite3WhereLoopPrint(pLoop, pWC);
    }
  }
  إذا( (sqlite3WhereTrace & 0x4001)==0x4001 ){
    إذا( iLevel==0 ){
      sqlite3DebugPrintf("WHERE clause being coded:\n");
      sqlite3_درخت_View_عبارت(0, pWInfo->pWhere, 0);
    }
    sqlite3DebugPrintf("All WHERE-clause terms before coding:\n");
    sqlite3WhereClausePrint(pWC);
  }
#پایان_اگر

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
  addr_نیافتن = پیدا_کن_No_تطابق_Continuation(pWInfo, iLevel);

  /* If this is the right table of a LEFT OUTER JOIN, allocate and
  ** initialize a memory cell that records if this table matches any
  ** row of the left table of the join.
  */
  أكد( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_راست_JOIN))
       || pLevel->iFrom>0 || (p_Tab_قلم[0].fg.jointype & JT_چپ)==0
  );
  إذا( pLevel->iFrom>0 && (p_Tab_قلم[0].fg.jointype & JT_چپ)!=0 ){
    pLevel->i_چپ_Join = ++pParse->nMem;
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 0, pLevel->i_چپ_Join);
    VdbeComment((v, "init LEFT JOIN match flag"));
    إذا( p_Tab_قلم->fg.از_طریق_Coroutine ){
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Null_ردیف, p_Tab_قلم->iCursor, 1);
    }
  }

  /* Special case of a FROM clause subquery implemented as a co-routine */
  إذا( p_Tab_قلم->fg.از_طریق_Coroutine ){
    صحيح regYield;
    Subquery *pSubq;
    أكد( p_Tab_قلم->fg.isSubquery && p_Tab_قلم->u4.pSubq!=0 );
    pSubq = p_Tab_قلم->u4.pSubq;
    regYield = pSubq->regReturn;
    sqlite3_Vdbe_جمع_کن_Op3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    pLevel->p2 =  sqlite3_Vdbe_جمع_کن_Op2(v, OP_Yield, regYield, addrBrk);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", p_Tab_قلم->pSTab->zName));
    pLevel->op = OP_Goto;
  }وإلا

#اگر_تعریف_نشده SQLITE_OMIT_VIRTUALTABLE
  إذا(  (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
    /* Case 1:  The table is a virtual-table.  Use the VFilter and VNext
    **          to access the data.
    */
    صحيح iReg;   /* P3 Value for OP_VFilter */
    صحيح addr_Not_یافت;
    صحيح nConstraint = pLoop->nLTerm;

    iReg = sqlite3_Get_موقت_Range(pParse, nConstraint+2);
    addr_Not_یافت = pLevel->addrBrk;
    لكل(j=0; j<nConstraint; j++){
      صحيح i_هدف = iReg+j+2;
      p_جمله = pLoop->aLTerm[j];
      إذا( NEVER(p_جمله==0) ) استمر;
      إذا( p_جمله->eOperator & WO_درون ){
        إذا( SMASKBIT32(j) & pLoop->u.vtab.m_Handle_درون ){
          صحيح iTab = pParse->nTab++;
          صحيح iCache = ++pParse->nMem;
          sqlite3_کد_Rhs_از_درون(pParse, p_جمله->p_عبارت, iTab, 0);
          sqlite3_Vdbe_جمع_کن_Op3(v, OP_VInit_درون, iTab, i_هدف, iCache);
        }وإلا{
          کد_Equality_جمله(pParse, p_جمله, pLevel, j, bRev, i_هدف);
          addr_Not_یافت = pLevel->addrNxt;
        }
      }وإلا{
        عبارت *p_راست = p_جمله->p_عبارت->p_راست;
        کد_عبارت_Or_وکتور(pParse, p_راست, i_هدف, 1);
        إذا( p_جمله->e_تطابق_Op==SQLITE_شاخص_CONSTRAINT_جابه_جایی
         && pLoop->u.vtab.b_Omit_جابه_جایی
        ){
          أكد( p_جمله->eOperator==WO_AUX );
          أكد( pWInfo->pSelect!=0 );
          أكد( pWInfo->pSelect->i_جابه_جایی>0 );
          sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 0, pWInfo->pSelect->i_جابه_جایی);
          VdbeComment((v,"Zero OFFSET counter"));
        }
      }
    }
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, pLoop->u.vtab.idx_رقم, iReg);
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, nConstraint, iReg+1);
    /* The instruction immediately prior to OP_VFilter must be an OP_Integer
    ** that sets the "argc" value for xVFilter.  This is necessary for
    ** resolveP2() to work correctly.  See tag-20250207a. */
    sqlite3_Vdbe_جمع_کن_Op4(v, OP_VFilter, iCur, addr_Not_یافت, iReg,
                      pLoop->u.vtab.idxStr,
                      pLoop->u.vtab.needFree ? P4_DYNAMIC : P4_STATIC);
    VdbeCoverage(v);
    pLoop->u.vtab.needFree = 0;
    /* An OOM inside of AddOp4(OP_VFilter) instruction above might have freed
    ** the u.vtab.idxStr.  NULL it out to prevent a use-after-free */
    إذا( db->mallocFailed ) pLoop->u.vtab.idxStr = 0;
    pLevel->p1 = iCur;
    pLevel->op = pWInfo->eOnePass ? OP_Noop : OP_VNext;
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);
    أكد( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );

    لكل(j=0; j<nConstraint; j++){
      p_جمله = pLoop->aLTerm[j];
      إذا( j<16 && (pLoop->u.vtab.omit_ماسک>>j)&1 ){
        disable_جمله(pLevel, p_جمله);
        استمر;
      }
      إذا( (p_جمله->eOperator & WO_درون)!=0
       && (SMASKBIT32(j) & pLoop->u.vtab.m_Handle_درون)==0
       && !db->mallocFailed
      ){
        عبارت *pCompare;  /* The comparison operator */
        عبارت *p_راست;    /* RHS of the comparison */
        VdbeOp *pOp;     /* Opcode to access the value of the IN constraint */
        صحيح i_درون;         /* IN loop corresponding to the j-th constraint */

        /* Reload the constraint value into reg[iReg+j+2].  The same value
        ** was loaded into the same register prior to the OP_VFilter, but
        ** the xFilter implementation might have changed the datatype or
        ** encoding of the value in the register, so it *must* be reloaded.
        */
        لكل(i_درون=0; ALWAYS(i_درون<pLevel->u.درون.n_درون); i_درون++){
          pOp = sqlite3VdbeGetOp(v, pLevel->u.درون.a_درون_Loop[i_درون].addr_درون_بالاترین);
          إذا( (pOp->opcode==OP_ستونی && pOp->p3==iReg+j+2)
           || (pOp->opcode==OP_Rowid && pOp->p2==iReg+j+2)
          ){
            testcase( pOp->opcode==OP_Rowid );
            sqlite3_Vdbe_جمع_کن_Op3(v, pOp->opcode, pOp->p1, pOp->p2, pOp->p3);
            اكسر;
          }
        }

        /* Generate code that will continue to the next row if
        ** the IN constraint is not satisfied
        */
        pCompare = sqlite3PExpr(pParse, TK_EQ, 0, 0);
        إذا( !db->mallocFailed ){
          صحيح iFld = p_جمله->u.x.i_فیلد;
          عبارت *p_چپ = p_جمله->p_عبارت->p_چپ;
          أكد( p_چپ!=0 );
          إذا( iFld>0 ){
            أكد( p_چپ->op==TK_وکتور );
            أكد( عبارت_Use_XList(p_چپ) );
            أكد( iFld<=p_چپ->x.pList->n_عبارت );
            pCompare->p_چپ = p_چپ->x.pList->a[iFld-1].p_عبارت;
          }وإلا{
            pCompare->p_چپ = p_چپ;
          }
          pCompare->p_راست = p_راست = sqlite3_عبارت(db, TK_ثبات, 0);
          إذا( p_راست ){
            p_راست->i_جدول = iReg+j+2;
            sqlite3_عبارت_If_False(
                pParse, pCompare, pLevel->addrCont, SQLITE_JUMPIFNULL
            );
          }
          pCompare->p_چپ = 0;
        }
        sqlite3_عبارت_Delete(db, pCompare);
      }
    }

    /* These registers need to be preserved in case there is an IN operator
    ** loop.  So we could deallocate the registers here (and potentially
    ** reuse them later) if (pLoop->wsFlags & WHERE_IN_ABLE)==0.  But it seems
    ** simpler and safer to simply not reuse the registers.
    **
    **    sqlite3ReleaseTempRange(pParse, iReg, nConstraint+2);
    */
  }وإلا
#پایان_اگر /* SQLITE_OMIT_VIRTUALTABLE */

  إذا( (pLoop->wsFlags & WHERE_IPK)!=0
   && (pLoop->wsFlags & (WHERE_ستونی_درون|WHERE_ستونی_EQ))!=0
  ){
    /* Case 2:  We can directly reference a single row using an
    **          equality comparison against the ROWID field.  Or
    **          we reference multiple rows using a "rowid IN (...)"
    **          construct.
    */
    أكد( pLoop->u.btree.nEq==1 );
    p_جمله = pLoop->aLTerm[0];
    أكد( p_جمله!=0 );
    أكد( p_جمله->p_عبارت!=0 );
    testcase( p_جمله->wtFlags & جمله_VIRTUAL );
    iReleaseReg = ++pParse->nMem;
    iRowidReg = کد_Equality_جمله(pParse, p_جمله, pLevel, 0, bRev, iReleaseReg);
    إذا( iRowidReg!=iReleaseReg ) sqlite3_Release_موقت_Reg(pParse, iReleaseReg);
    addrNxt = pLevel->addrNxt;
    إذا( addr_نیافتن==0 ) addr_نیافتن = addrNxt;
    إذا( pLevel->regFilter ){
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_MustBeInt, iRowidReg, addr_نیافتن);
      VdbeCoverage(v);
      sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Filter, pLevel->regFilter, addr_نیافتن,
                           iRowidReg, 1);
      VdbeCoverage(v);
      filter_Pull_پایین(pParse, pWInfo, iLevel, addrNxt, notReady);
    }
    sqlite3_Vdbe_جمع_کن_Op3(v, OP_SeekRowid, iCur, addr_نیافتن, iRowidReg);
    VdbeCoverage(v);
    pLevel->op = OP_Noop;
  }وإلا إذا( (pLoop->wsFlags & WHERE_IPK)!=0
         && (pLoop->wsFlags & WHERE_ستونی_RANGE)!=0
  ){
    /* Case 3:  We have an inequality comparison against the ROWID field.
    */
    صحيح testOp = OP_Noop;
    صحيح شروع;
    صحيح mem_انتها_مقدار = 0;
    Where_جمله *p_شروع, *p_انتها;

    j = 0;
    p_شروع = p_انتها = 0;
    إذا( pLoop->wsFlags & WHERE_BTM_حد ) p_شروع = pLoop->aLTerm[j++];
    إذا( pLoop->wsFlags & WHERE_بالاترین_حد ) p_انتها = pLoop->aLTerm[j++];
    أكد( p_شروع!=0 || p_انتها!=0 );
    إذا( bRev ){
      p_جمله = p_شروع;
      p_شروع = p_انتها;
      p_انتها = p_جمله;
    }
    کد_Cursor_Hint(p_Tab_قلم, pWInfo, pLevel, p_انتها);
    إذا( p_شروع ){
      عبارت *pX;             /* The expression that defines the start bound */
      صحيح r1, r_موقت;        /* Registers for holding the start boundary */
      صحيح op;               /* Cursor seek operation */

      /* The following constant maps TK_xx codes into corresponding
      ** seek opcodes.  It depends on a particular ordering of TK_xx
      */
      ثابت u8 aMoveOp[] = {
           /* TK_GT */  OP_SeekGT,
           /* TK_LE */  OP_SeekLE,
           /* TK_LT */  OP_SeekLT,
           /* TK_GE */  OP_SeekGE
      };
      أكد( TK_LE==TK_GT+1 );      /* Make sure the ordering.. */
      أكد( TK_LT==TK_GT+2 );      /*  ... of the TK_xx values... */
      أكد( TK_GE==TK_GT+3 );      /*  ... is correct. */

      أكد( (p_شروع->wtFlags & جمله_VNULL)==0 );
      testcase( p_شروع->wtFlags & جمله_VIRTUAL );
      pX = p_شروع->p_عبارت;
      أكد( pX!=0 );
      testcase( p_شروع->چپ_Cursor!=iCur ); /* transitive constraints */
      إذا( sqlite3_عبارت_Is_وکتور(pX->p_راست) ){
        r1 = r_موقت = sqlite3_Get_موقت_Reg(pParse);
        کد_عبارت_Or_وکتور(pParse, pX->p_راست, r1, 1);
        testcase( pX->op==TK_GT );
        testcase( pX->op==TK_GE );
        testcase( pX->op==TK_LT );
        testcase( pX->op==TK_LE );
        op = aMoveOp[((pX->op - TK_GT - 1) & 0x3) | 0x1];
        أكد( pX->op!=TK_GT || op==OP_SeekGE );
        أكد( pX->op!=TK_GE || op==OP_SeekGE );
        أكد( pX->op!=TK_LT || op==OP_SeekLE );
        أكد( pX->op!=TK_LE || op==OP_SeekLE );
      }وإلا{
        r1 = sqlite3_عبارت_کد_موقت(pParse, pX->p_راست, &r_موقت);
        disable_جمله(pLevel, p_شروع);
        op = aMoveOp[(pX->op - TK_GT)];
      }
      sqlite3_Vdbe_جمع_کن_Op3(v, op, iCur, addrBrk, r1);
      VdbeComment((v, "pk"));
      VdbeCoverageIf(v, pX->op==TK_GT);
      VdbeCoverageIf(v, pX->op==TK_LE);
      VdbeCoverageIf(v, pX->op==TK_LT);
      VdbeCoverageIf(v, pX->op==TK_GE);
      sqlite3_Release_موقت_Reg(pParse, r_موقت);
    }وإلا{
      sqlite3_Vdbe_جمع_کن_Op2(v, bRev ? OP_آخرین : OP_Rewind, iCur, pLevel->addrHalt);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
    }
    إذا( p_انتها ){
      عبارت *pX;
      pX = p_انتها->p_عبارت;
      أكد( pX!=0 );
      أكد( (p_انتها->wtFlags & جمله_VNULL)==0 );
      testcase( p_انتها->چپ_Cursor!=iCur ); /* Transitive constraints */
      testcase( p_انتها->wtFlags & جمله_VIRTUAL );
      mem_انتها_مقدار = ++pParse->nMem;
      کد_عبارت_Or_وکتور(pParse, pX->p_راست, mem_انتها_مقدار, 1);
      إذا( 0==sqlite3_عبارت_Is_وکتور(pX->p_راست)
       && (pX->op==TK_LT || pX->op==TK_GT)
      ){
        testOp = bRev ? OP_Le : OP_Ge;
      }وإلا{
        testOp = bRev ? OP_Lt : OP_Gt;
      }
      إذا( 0==sqlite3_عبارت_Is_وکتور(pX->p_راست) ){
        disable_جمله(pLevel, p_انتها);
      }
    }
    شروع = sqlite3VdbeCurrentAddr(v);
    pLevel->op = bRev ? OP_Prev : OP_بعدی;
    pLevel->p1 = iCur;
    pLevel->p2 = شروع;
    أكد( pLevel->p5==0 );
    إذا( testOp!=OP_Noop ){
      iRowidReg = ++pParse->nMem;
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Rowid, iCur, iRowidReg);
      sqlite3_Vdbe_جمع_کن_Op3(v, testOp, mem_انتها_مقدار, addrBrk, iRowidReg);
      VdbeCoverageIf(v, testOp==OP_Le);
      VdbeCoverageIf(v, testOp==OP_Lt);
      VdbeCoverageIf(v, testOp==OP_Ge);
      VdbeCoverageIf(v, testOp==OP_Gt);
      sqlite3VdbeChangeP5(v, SQLITE_AFF_NUMERIC | SQLITE_JUMPIFNULL);
    }
  }وإلا إذا( pLoop->wsFlags & WHERE_نمایهای ){
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
    ساكن ثابت u8 a_شروع_Op[] = {
      0,
      0,
      OP_Rewind,           /* 2: (!start_constraints && startEq &&  !bRev) */
      OP_آخرین,             /* 3: (!start_constraints && startEq &&   bRev) */
      OP_SeekGT,           /* 4: (start_constraints  && !startEq && !bRev) */
      OP_SeekLT,           /* 5: (start_constraints  && !startEq &&  bRev) */
      OP_SeekGE,           /* 6: (start_constraints  &&  startEq && !bRev) */
      OP_SeekLE            /* 7: (start_constraints  &&  startEq &&  bRev) */
    };
    ساكن ثابت u8 a_انتها_Op[] = {
      OP_IdxGE,            /* 0: (end_constraints && !bRev && !endEq) */
      OP_IdxGT,            /* 1: (end_constraints && !bRev &&  endEq) */
      OP_IdxLE,            /* 2: (end_constraints &&  bRev && !endEq) */
      OP_IdxLT,            /* 3: (end_constraints &&  bRev &&  endEq) */
    };
    u16 nEq = pLoop->u.btree.nEq;     /* Number of == or IN terms */
    u16 nBtm = pLoop->u.btree.nBtm;   /* Length of BTM vector */
    u16 n_بالاترین = pLoop->u.btree.n_بالاترین;   /* Length of TOP vector */
    صحيح reg_پایه;                 /* Base register holding constraint values */
    Where_جمله *p_Range_شروع = 0;  /* Inequality constraint at range start */
    Where_جمله *p_Range_انتها = 0;    /* Inequality constraint at range end */
    صحيح شروع_Eq;                 /* True if range start uses ==, >= or <= */
    صحيح انتها_Eq;                   /* True if range end uses ==, >= or <= */
    صحيح شروع_constraints;       /* Start of range is constrained */
    صحيح nConstraint;             /* Number of constraint terms */
    صحيح iIdxCur;                 /* The VDBE cursor for the index */
    صحيح nExtraReg = 0;           /* Number of extra registers needed */
    صحيح op;                      /* Instruction opcode */
    حرف *z_شروع_Aff;             /* Affinity for start of range constraint */
    حرف *z_انتها_Aff = 0;           /* Affinity for end of range constraint */
    u8 bSeekPastNull = 0;        /* True to seek past initial nulls */
    u8 bStopAtNull = 0;          /* Add condition to terminate at NULLs */
    صحيح omit_جدول;               /* True if we use the index only */
    صحيح regBignull = 0;          /* big-null flag register */
    صحيح addr_Seek_پیمایش = 0;        /* Opcode of the OP_SeekScan, if any */

    pIdx = pLoop->u.btree.p_شاخص;
    iIdxCur = pLevel->iIdxCur;
    أكد( nEq>=pLoop->nSkip );

    /* Find any inequality constraint terms for the start and end
    ** of the range.
    */
    j = nEq;
    إذا( pLoop->wsFlags & WHERE_BTM_حد ){
      p_Range_شروع = pLoop->aLTerm[j++];
      nExtraReg = MAX(nExtraReg, pLoop->u.btree.nBtm);
      /* Like optimization range constraints always occur in pairs */
      أكد( (p_Range_شروع->wtFlags & جمله_LIKEOPT)==0 ||
              (pLoop->wsFlags & WHERE_بالاترین_حد)!=0 );
    }
    إذا( pLoop->wsFlags & WHERE_بالاترین_حد ){
      p_Range_انتها = pLoop->aLTerm[j++];
      nExtraReg = MAX(nExtraReg, pLoop->u.btree.n_بالاترین);
#اگر_تعریف_نشده SQLITE_LIKE_DOESNT_تطابق_BLOBS
      إذا( (p_Range_انتها->wtFlags & جمله_LIKEOPT)!=0 ){
        أكد( p_Range_شروع!=0 );                     /* LIKE opt constraints */
        أكد( p_Range_شروع->wtFlags & جمله_LIKEOPT );   /* occur in pairs */
        pLevel->iLikeRepCntr = (u32)++pParse->nMem;
        /* iLikeRepCntr actually stores 2x the counter register number.  The
        ** bottom bit indicates whether the search order is ASC or DESC. */
        testcase( bRev );
        testcase( pIdx->a_مرتب_سازی_ترتیب[nEq]==SQLITE_SO_DESC );
        أكد( (bRev & ~1)==0 );
        pLevel->iLikeRepCntr <<=1;
        pLevel->iLikeRepCntr |= bRev ^ (pIdx->a_مرتب_سازی_ترتیب[nEq]==SQLITE_SO_DESC);
      }
#پایان_اگر
      إذا( p_Range_شروع==0 ){
        j = pIdx->ai_ستونی[nEq];
        إذا( (j>=0 && pIdx->p_جدول->a_ستون[j].notNull==0) || j==XN_عبارت ){
          bSeekPastNull = 1;
        }
      }
    }
    أكد( p_Range_انتها==0 || (p_Range_انتها->wtFlags & جمله_VNULL)==0 );

    /* If the WHERE_BIGNULL_SORT flag is set, then index column nEq uses
    ** a non-default "big-null" sort (either ASC NULLS LAST or DESC NULLS
    ** FIRST). In both cases separate ordered scans are made of those
    ** index entries for which the column is null and for those for which
    ** it is not. For an ASC sort, the non-NULL entries are scanned first.
    ** For DESC, NULL entries are scanned first.
    */
    إذا( (pLoop->wsFlags & (WHERE_بالاترین_حد|WHERE_BTM_حد))==0
     && (pLoop->wsFlags & WHERE_BIGNULL_مرتب_سازی)!=0
    ){
      أكد( bSeekPastNull==0 && nExtraReg==0 && nBtm==0 && n_بالاترین==0 );
      أكد( p_Range_انتها==0 && p_Range_شروع==0 );
      testcase( pLoop->nSkip>0 );
      nExtraReg = 1;
      bSeekPastNull = 1;
      pLevel->regBignull = regBignull = ++pParse->nMem;
      إذا( pLevel->i_چپ_Join ){
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 0, regBignull);
      }
      pLevel->addrBignull = sqlite3VdbeMakeLabel(pParse);
    }

    /* If we are doing a reverse order scan on an ascending index, or
    ** a forward order scan on a descending index, interchange the
    ** start and end terms (pRangeStart and pRangeEnd).
    */
    إذا( (nEq<pIdx->n_ستونی && bRev==(pIdx->a_مرتب_سازی_ترتیب[nEq]==SQLITE_SO_ASC)) ){
      جابجا(Where_جمله *, p_Range_انتها, p_Range_شروع);
      جابجا(u8, bSeekPastNull, bStopAtNull);
      جابجا(u8, nBtm, n_بالاترین);
    }

    إذا( iLevel>0 && (pLoop->wsFlags & WHERE_درون_SEEKSCAN)!=0 ){
      /* In case OP_SeekScan is used, ensure that the index cursor does not
      ** point to a valid row for the first iteration of this loop. */
      sqlite3_Vdbe_جمع_کن_Op1(v, OP_Null_ردیف, iIdxCur);
    }

    /* Generate code to evaluate all constraint terms using == or IN
    ** and store the values of those terms in an array of registers
    ** starting at regBase.
    */
    کد_Cursor_Hint(p_Tab_قلم, pWInfo, pLevel, p_Range_انتها);
    reg_پایه = کد_All_Equality_Terms(pParse,pLevel,bRev,nExtraReg,&z_شروع_Aff);
    أكد( z_شروع_Aff==0 || sqlite3Strlen30(z_شروع_Aff)>=nEq );
#اگر_تعریف_نشده SQLITE_LIKE_DOESNT_تطابق_BLOBS
    إذا( pLevel->iLikeRepCntr ){
      /* The LIKE optimization loop must be the innermost of any IN(...)
      ** or skip-scan loops. Otherwise rows may not be visited in index 
      ** order. This means it needs an addrNxt label for the same reason
      ** IN(...) scans do.  */
      إذا( pLevel->addrNxt==pLevel->addrBrk ){
        pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 1, (صحيح)(pLevel->iLikeRepCntr>>1));
      VdbeComment((v, "LIKE loop counter"));
      pLevel->addrLikeRep = sqlite3VdbeCurrentAddr(v);
    }
#پایان_اگر
    إذا( z_شروع_Aff && n_بالاترین ){
      z_انتها_Aff = sqlite3DbStrDup(db, &z_شروع_Aff[nEq]);
    }
    addrNxt = (regBignull ? pLevel->addrBignull : pLevel->addrNxt);
    إذا( addr_نیافتن==0 ) addr_نیافتن = addrNxt;

    testcase( p_Range_شروع && (p_Range_شروع->eOperator & WO_LE)!=0 );
    testcase( p_Range_شروع && (p_Range_شروع->eOperator & WO_GE)!=0 );
    testcase( p_Range_انتها && (p_Range_انتها->eOperator & WO_LE)!=0 );
    testcase( p_Range_انتها && (p_Range_انتها->eOperator & WO_GE)!=0 );
    شروع_Eq = !p_Range_شروع || p_Range_شروع->eOperator & (WO_LE|WO_GE);
    انتها_Eq =   !p_Range_انتها || p_Range_انتها->eOperator & (WO_LE|WO_GE);
    شروع_constraints = p_Range_شروع || nEq>0;

    /* Seek the index cursor to the start of the range. */
    nConstraint = nEq;
    إذا( p_Range_شروع ){
      عبارت *p_راست = p_Range_شروع->p_عبارت->p_راست;
      کد_عبارت_Or_وکتور(pParse, p_راست, reg_پایه+nEq, nBtm);
      whereLikeOptimizationStringFixup(v, pLevel, p_Range_شروع);
      إذا( (p_Range_شروع->wtFlags & جمله_VNULL)==0
       && sqlite3_عبارت_Can_Be_Null(p_راست)
      ){
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_IsNull, reg_پایه+nEq, addrNxt);
        VdbeCoverage(v);
      }
      إذا( z_شروع_Aff ){
        updateRangeAffinityStr(p_راست, nBtm, &z_شروع_Aff[nEq]);
      }
      nConstraint += nBtm;
      testcase( p_Range_شروع->wtFlags & جمله_VIRTUAL );
      إذا( sqlite3_عبارت_Is_وکتور(p_راست)==0 ){
        disable_جمله(pLevel, p_Range_شروع);
      }وإلا{
        شروع_Eq = 1;
      }
      bSeekPastNull = 0;
    }وإلا إذا( bSeekPastNull ){
      شروع_Eq = 0;
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Null, 0, reg_پایه+nEq);
      شروع_constraints = 1;
      nConstraint++;
    }وإلا إذا( regBignull ){
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_Null, 0, reg_پایه+nEq);
      شروع_constraints = 1;
      nConstraint++;
    }
    کد_Apply_Affinity(pParse, reg_پایه, nConstraint - bSeekPastNull, z_شروع_Aff);
    إذا( pLoop->nSkip>0 && nConstraint==pLoop->nSkip ){
      /* The skip-scan logic inside the call to codeAllEqualityConstraints()
      ** above has already left the cursor sitting on the correct row,
      ** so no further seeking is needed */
    }وإلا{
      إذا( regBignull ){
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 1, regBignull);
        VdbeComment((v, "NULL-scan pass ctr"));
      }
      إذا( pLevel->regFilter ){
        أكد( sqlite3WhereLoopBloomable(pLoop) );
        sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Filter, pLevel->regFilter, addr_نیافتن,
                             reg_پایه, nEq);
        VdbeCoverage(v);
        filter_Pull_پایین(pParse, pWInfo, iLevel, addrNxt, notReady);
      }

      op = a_شروع_Op[(شروع_constraints<<2) + (شروع_Eq<<1) + bRev];
      أكد( op!=0 );
      إذا( (pLoop->wsFlags & WHERE_درون_SEEKSCAN)!=0 && op==OP_SeekGE ){
        أكد( regBignull==0 );
        /* TUNING:  The OP_SeekScan opcode seeks to reduce the number
        ** of expensive seek operations by replacing a single seek with
        ** 1 or more step operations.  The question is, how many steps
        ** should we try before giving up and going with a seek.  The cost
        ** of a seek is proportional to the logarithm of the of the number
        ** of entries in the tree, so basing the number of steps to try
        ** on the estimated number of rows in the btree seems like a good
        ** guess. */
        addr_Seek_پیمایش = sqlite3_Vdbe_جمع_کن_Op1(v, OP_Seek_پیمایش,
                                         (pIdx->ai_ردیف_Log_Est[0]+9)/10);
        إذا( p_Range_شروع || p_Range_انتها ){
          sqlite3VdbeChangeP5(v, 1);
          sqlite3VdbeChangeP2(v, addr_Seek_پیمایش, sqlite3VdbeCurrentAddr(v)+1);
          addr_Seek_پیمایش = 0;
        }
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_جمع_کن_Op4_Int(v, op, iIdxCur, addr_نیافتن, reg_پایه, nConstraint);
      VdbeCoverage(v);
      VdbeCoverageIf(v, op==OP_Rewind);  testcase( op==OP_Rewind );
      VdbeCoverageIf(v, op==OP_آخرین);    testcase( op==OP_آخرین );
      VdbeCoverageIf(v, op==OP_SeekGT);  testcase( op==OP_SeekGT );
      VdbeCoverageIf(v, op==OP_SeekGE);  testcase( op==OP_SeekGE );
      VdbeCoverageIf(v, op==OP_SeekLE);  testcase( op==OP_SeekLE );
      VdbeCoverageIf(v, op==OP_SeekLT);  testcase( op==OP_SeekLT );

      أكد( bSeekPastNull==0 || bStopAtNull==0 );
      إذا( regBignull ){
        أكد( bSeekPastNull==1 || bStopAtNull==1 );
        أكد( bSeekPastNull==!bStopAtNull );
        أكد( bStopAtNull==شروع_Eq );
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        op = a_شروع_Op[(nConstraint>1)*4 + 2 + bRev];
        sqlite3_Vdbe_جمع_کن_Op4_Int(v, op, iIdxCur, addrNxt, reg_پایه,
                             nConstraint-شروع_Eq);
        VdbeCoverage(v);
        VdbeCoverageIf(v, op==OP_Rewind);  testcase( op==OP_Rewind );
        VdbeCoverageIf(v, op==OP_آخرین);    testcase( op==OP_آخرین );
        VdbeCoverageIf(v, op==OP_SeekGE);  testcase( op==OP_SeekGE );
        VdbeCoverageIf(v, op==OP_SeekLE);  testcase( op==OP_SeekLE );
        أكد( op==OP_Rewind || op==OP_آخرین || op==OP_SeekGE || op==OP_SeekLE);
      }
    }

    /* Load the value for the inequality constraint at the end of the
    ** range (if any).
    */
    nConstraint = nEq;
    أكد( pLevel->p2==0 );
    إذا( p_Range_انتها ){
      عبارت *p_راست = p_Range_انتها->p_عبارت->p_راست;
      أكد( addr_Seek_پیمایش==0 );
      کد_عبارت_Or_وکتور(pParse, p_راست, reg_پایه+nEq, n_بالاترین);
      whereLikeOptimizationStringFixup(v, pLevel, p_Range_انتها);
      إذا( (p_Range_انتها->wtFlags & جمله_VNULL)==0
       && sqlite3_عبارت_Can_Be_Null(p_راست)
      ){
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_IsNull, reg_پایه+nEq, addrNxt);
        VdbeCoverage(v);
      }
      إذا( z_انتها_Aff ){
        updateRangeAffinityStr(p_راست, n_بالاترین, z_انتها_Aff);
        کد_Apply_Affinity(pParse, reg_پایه+nEq, n_بالاترین, z_انتها_Aff);
      }وإلا{
        أكد( pParse->db->mallocFailed );
      }
      nConstraint += n_بالاترین;
      testcase( p_Range_انتها->wtFlags & جمله_VIRTUAL );

      إذا( sqlite3_عبارت_Is_وکتور(p_راست)==0 ){
        disable_جمله(pLevel, p_Range_انتها);
      }وإلا{
        انتها_Eq = 1;
      }
    }وإلا إذا( bStopAtNull ){
      إذا( regBignull==0 ){
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Null, 0, reg_پایه+nEq);
        انتها_Eq = 0;
      }
      nConstraint++;
    }
    إذا( z_شروع_Aff ) sqlite3DbNNFreeNN(db, z_شروع_Aff);
    إذا( z_انتها_Aff ) sqlite3DbNNFreeNN(db, z_انتها_Aff);

    /* Top of the loop body */
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    /* Check if the index cursor is past the end of the range. */
    إذا( nConstraint ){
      إذا( regBignull ){
        /* Except, skip the end-of-range check while doing the NULL-scan */
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_IfNot, regBignull, sqlite3VdbeCurrentAddr(v)+3);
        VdbeComment((v, "If NULL-scan 2nd pass"));
        VdbeCoverage(v);
      }
      op = a_انتها_Op[bRev*2 + انتها_Eq];
      sqlite3_Vdbe_جمع_کن_Op4_Int(v, op, iIdxCur, addrNxt, reg_پایه, nConstraint);
      testcase( op==OP_IdxGT );  VdbeCoverageIf(v, op==OP_IdxGT );
      testcase( op==OP_IdxGE );  VdbeCoverageIf(v, op==OP_IdxGE );
      testcase( op==OP_IdxLT );  VdbeCoverageIf(v, op==OP_IdxLT );
      testcase( op==OP_IdxLE );  VdbeCoverageIf(v, op==OP_IdxLE );
      إذا( addr_Seek_پیمایش ) sqlite3_Vdbe_Jump_اینجا(v, addr_Seek_پیمایش);
    }
    إذا( regBignull ){
      /* During a NULL-scan, check to see if we have reached the end of
      ** the NULLs */
      أكد( bSeekPastNull==!bStopAtNull );
      أكد( bSeekPastNull+bStopAtNull==1 );
      أكد( nConstraint+bSeekPastNull>0 );
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_If, regBignull, sqlite3VdbeCurrentAddr(v)+2);
      VdbeComment((v, "If NULL-scan 1st pass"));
      VdbeCoverage(v);
      op = a_انتها_Op[bRev*2 + bSeekPastNull];
      sqlite3_Vdbe_جمع_کن_Op4_Int(v, op, iIdxCur, addrNxt, reg_پایه,
                           nConstraint+bSeekPastNull);
      testcase( op==OP_IdxGT );  VdbeCoverageIf(v, op==OP_IdxGT );
      testcase( op==OP_IdxGE );  VdbeCoverageIf(v, op==OP_IdxGE );
      testcase( op==OP_IdxLT );  VdbeCoverageIf(v, op==OP_IdxLT );
      testcase( op==OP_IdxLE );  VdbeCoverageIf(v, op==OP_IdxLE );
    }

    إذا( (pLoop->wsFlags & WHERE_درون_EARLYOUT)!=0 ){
      sqlite3_Vdbe_جمع_کن_Op3(v, OP_Seek_اصابت, iIdxCur, nEq, nEq);
    }

    /* Seek the table cursor, if required */
    omit_جدول = (pLoop->wsFlags & WHERE_IDX_ONLY)!=0
           && (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_راست_JOIN))==0;
    إذا( omit_جدول ){
      /* pIdx is a covering index.  No need to access the main table. */
    }وإلا إذا( HasRowid(pIdx->p_جدول) ){
      کد_Deferred_Seek(pWInfo, pIdx, iCur, iIdxCur);
    }وإلا إذا( iCur!=iIdxCur ){
      شاخص *pPk = sqlite3_Primary_کلید_شاخص(pIdx->p_جدول);
      iRowidReg = sqlite3_Get_موقت_Range(pParse, pPk->n_کلید_ستون);
      لكل(j=0; j<pPk->n_کلید_ستون; j++){
        k = sqlite3_جدول_ستونی_مقصد_شاخص(pIdx, pPk->ai_ستونی[j]);
        sqlite3_Vdbe_جمع_کن_Op3(v, OP_ستونی, iIdxCur, k, iRowidReg+j);
      }
      sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Not_یافت, iCur, addrCont,
                           iRowidReg, pPk->n_کلید_ستون); VdbeCoverage(v);
    }

    إذا( pLevel->i_چپ_Join==0 ){
      /* If a partial index is driving the loop, try to eliminate WHERE clause
      ** terms from the query that must be true due to the WHERE clause of
      ** the partial index.  This optimization does not work on an outer join,
      ** as shown by:
      **
      ** 2019-11-02 ticket 623eff57e76d45f6      (LEFT JOIN)
      ** 2025-05-29 forum post 7dee41d32506c4ae  (RIGHT JOIN)
      */
      إذا( pIdx->pPartIdxWhere && pLevel->pRJ==0 ){
        where_Apply_Partial_شاخص_Constraints(pIdx->pPartIdxWhere, iCur, pWC);
      }
    }وإلا{
      testcase( pIdx->pPartIdxWhere );
      /* The following assert() is not a requirement, merely an observation:
      ** The OR-optimization doesn't work for the right hand table of
      ** a LEFT JOIN: */
      أكد( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_راست_JOIN))==0 );
    }

    /* Record the instruction used to terminate the loop. */
    إذا( (pLoop->wsFlags & WHERE_ONEROW)
     || (pLevel->u.درون.n_درون && regBignull==0 && where_Loop_Is_One_ردیف(pLoop))
    ){
      pLevel->op = OP_Noop;
    }وإلا إذا( bRev ){
      pLevel->op = OP_Prev;
    }وإلا{
      pLevel->op = OP_بعدی;
    }
    pLevel->p1 = iIdxCur;
    pLevel->p3 = (pLoop->wsFlags&WHERE_UNQ_WANTED)!=0 ? 1:0;
    إذا( (pLoop->wsFlags & WHERE_CONSTRAINT)==0 ){
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_گام;
    }وإلا{
      أكد( pLevel->p5==0 );
    }
    إذا( omit_جدول ) pIdx = 0;
  }وإلا

#اگر_تعریف_نشده SQLITE_OMIT_OR_OPTIMIZATION
  إذا( pLoop->wsFlags & WHERE_MULTI_OR ){
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
    شاخص *pCov = 0;             /* Potential covering index (or NULL) */
    صحيح iCovCur = pParse->nTab++;  /* Cursor used for index scans (if any) */

    صحيح regReturn = ++pParse->nMem;           /* Register used with OP_Gosub */
    صحيح regRowset = 0;                        /* Register for RowSet object */
    صحيح regRowid = 0;                         /* Register holding rowid */
    صحيح iLoopBody = sqlite3VdbeMakeLabel(pParse);/* Start of loop body */
    صحيح iRetInit;                             /* Address of regReturn init */
    صحيح untestedTerms = 0;             /* Some terms not completely tested */
    صحيح ii;                            /* Loop counter */
    عبارت *p_And_عبارت = 0;                /* An ".. AND (...)" expression */
    جدول *pTab = p_Tab_قلم->pSTab;

    p_جمله = pLoop->aLTerm[0];
    أكد( p_جمله!=0 );
    أكد( p_جمله->eOperator & WO_OR );
    أكد( (p_جمله->wtFlags & جمله_ORINFO)!=0 );
    pOrWc = &p_جمله->u.pOrInfo->wc;
    pLevel->op = OP_Return;
    pLevel->p1 = regReturn;

    /* Set up a new SrcList in pOrTab containing the table being scanned
    ** by this loop in the a[0] slot and all notReady tables in a[1..] slots.
    ** This becomes the SrcList in the recursive call to sqlite3WhereBegin().
    */
    إذا( pWInfo->nLevel>1 || p_Tab_قلم->fg.fromExists ){
      صحيح nNotReady;                 /* The number of notReady tables */
      Src_قلم *origSrc;              /* Original list of tables */
      nNotReady = pWInfo->nLevel - iLevel - 1;
      pOrTab = sqlite3DbMallocRawNN(db, SZ_SRCLIST(nNotReady+1));
      إذا( pOrTab==0 ) أرجع notReady;
      pOrTab->nAlloc = (u8)(nNotReady + 1);
      pOrTab->nSrc = pOrTab->nAlloc;
      کپی_حافظه(pOrTab->a, p_Tab_قلم, حجم(*p_Tab_قلم));
      origSrc = pWInfo->pTabList->a;
      لكل(k=1; k<=nNotReady; k++){
        کپی_حافظه(&pOrTab->a[k], &origSrc[pLevel[k].iFrom], حجم(pOrTab->a[k]));
      }

      /* Clear the fromExists flag on the OR-optimized table entry so that
      ** the calls to sqlite3WhereEnd() do not code early-exits after the
      ** first row is visited. The early exit applies to this table's
      ** overall loop - including the multiple OR branches and any WHERE
      ** conditions not passed to the sub-loops - not to the sub-loops.  */
      pOrTab->a[0].fg.fromExists = 0;
    }وإلا{
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
    إذا( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
      إذا( HasRowid(pTab) ){
        regRowset = ++pParse->nMem;
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_Null, 0, regRowset);
      }وإلا{
        شاخص *pPk = sqlite3_Primary_کلید_شاخص(pTab);
        regRowset = pParse->nTab++;
        sqlite3_Vdbe_جمع_کن_Op2(v, OP_OpenEphemeral, regRowset, pPk->n_کلید_ستون);
        sqlite3_Vdbe_مجموعه_P4_کلید_Info(pParse, pPk);
      }
      regRowid = ++pParse->nMem;
    }
    iRetInit = sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 0, regReturn);

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
    إذا( pWC->n_جمله>1 ){
      صحيح i_جمله;
      لكل(i_جمله=0; i_جمله<pWC->n_جمله; i_جمله++){
        عبارت *p_عبارت = pWC->a[i_جمله].p_عبارت;
        إذا( &pWC->a[i_جمله] == p_جمله ) استمر;
        testcase( pWC->a[i_جمله].wtFlags & جمله_VIRTUAL );
        testcase( pWC->a[i_جمله].wtFlags & جمله_CODED );
        testcase( pWC->a[i_جمله].wtFlags & جمله_برش );
        إذا( (pWC->a[i_جمله].wtFlags & (جمله_VIRTUAL|جمله_CODED|جمله_برش))!=0 ){
          استمر;
        }
        إذا( (pWC->a[i_جمله].eOperator & WO_ALL)==0 ) استمر;
        إذا( عبارت_Has_Property(p_عبارت, EP_Subquery) ) استمر;  /* tag-20220303a */
        إذا( where_عبارت_Is_Ready(pWInfo, pLevel, p_Tab_قلم, p_عبارت)==0 ) استمر;
        p_عبارت = sqlite3_عبارت_Dup(db, p_عبارت, 0);
        p_And_عبارت = sqlite3_عبارت_And(pParse, p_And_عبارت, p_عبارت);
      }
      إذا( p_And_عبارت ){
        /* The extra 0x10000 bit on the opcode is masked off and does not
        ** become part of the new Expr.op.  However, it does make the
        ** op==TK_AND comparison inside of sqlite3PExpr() false, and this
        ** prevents sqlite3PExpr() from applying the AND short-circuit
        ** optimization, which we do not want here. */
        p_And_عبارت = sqlite3PExpr(pParse, TK_AND|0x10000, 0, p_And_عبارت);
      }
    }

    /* Run a separate WHERE clause for each term of the OR clause.  After
    ** eliminating duplicates from other WHERE clauses, the action for each
    ** sub-WHERE clause is to to invoke the main loop body as a subroutine.
    */
    ExplainQueryPlan((pParse, 1, "MULTI-INDEX OR"));
    لكل(ii=0; ii<pOrWc->n_جمله; ii++){
      Where_جمله *p_Or_جمله = &pOrWc->a[ii];
      إذا( p_Or_جمله->چپ_Cursor==iCur || (p_Or_جمله->eOperator & WO_AND)!=0 ){
        WhereInfo *pSubWInfo;           /* Info for single OR-term scan */
        عبارت *p_Or_عبارت = p_Or_جمله->p_عبارت; /* Current OR clause term */
        عبارت *pDelete;                  /* Local copy of OR clause term */
        صحيح jmp1 = 0;                   /* Address of jump operation */
        testcase( (p_Tab_قلم[0].fg.jointype & JT_چپ)!=0
               && !عبارت_Has_Property(p_Or_عبارت, EP_OuterON)
        ); /* See TH3 vtab25.400 and ticket 614b25314c766238 */
        pDelete = p_Or_عبارت = sqlite3_عبارت_Dup(db, p_Or_عبارت, 0);
        إذا( db->mallocFailed ){
          sqlite3_عبارت_Delete(db, pDelete);
          استمر;
        }
        إذا( p_And_عبارت ){
          p_And_عبارت->p_چپ = p_Or_عبارت;
          p_Or_عبارت = p_And_عبارت;
        }
        /* Loop through table entries that match term pOrTerm. */
        ExplainQueryPlan((pParse, 1, "INDEX %d", ii+1));
        WHERETRACE(0xffffffff, ("Subplan for OR-clause:\n"));
        pSubWInfo = sqlite3WhereBegin(pParse, pOrTab, p_Or_عبارت, 0, 0, 0,
                                      WHERE_OR_SUBCLAUSE, iCovCur);
        أكد( pSubWInfo || pParse->nErr );
        إذا( pSubWInfo ){
          WhereLoop *pSubLoop;
          صحيح addrExplain = sqlite3_Where_Explain_One_پیمایش(
              pParse, pOrTab, &pSubWInfo->a[0], 0
          );
          sqlite3_Where_جمع_کن_پیمایش_وضعیت(v, pOrTab, &pSubWInfo->a[0], addrExplain);

          /* This is the sub-WHERE clause body.  First skip over
          ** duplicate rows from prior sub-WHERE clauses, and record the
          ** rowid (or PRIMARY KEY) for the current row so that the same
          ** row will be skipped in subsequent sub-WHERE clauses.
          */
          إذا( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
            صحيح i_مجموعه = ((ii==pOrWc->n_جمله-1)?-1:ii);
            إذا( HasRowid(pTab) ){
              sqlite3_عبارت_کد_Get_ستونی_از_جدول(v, pTab, iCur, -1, regRowid);
              jmp1 = sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_ردیف_مجموعه_Test, regRowset, 0,
                                          regRowid, i_مجموعه);
              VdbeCoverage(v);
            }وإلا{
              شاخص *pPk = sqlite3_Primary_کلید_شاخص(pTab);
              صحيح nPk = pPk->n_کلید_ستون;
              صحيح iPk;
              صحيح r;

              /* Read the PK into an array of temp registers. */
              r = sqlite3_Get_موقت_Range(pParse, nPk);
              لكل(iPk=0; iPk<nPk; iPk++){
                صحيح i_ستون = pPk->ai_ستونی[iPk];
                sqlite3_عبارت_کد_Get_ستونی_از_جدول(v, pTab, iCur, i_ستون,r+iPk);
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
              إذا( i_مجموعه ){
                jmp1 = sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_یافت, regRowset, 0, r, nPk);
                VdbeCoverage(v);
              }
              إذا( i_مجموعه>=0 ){
                sqlite3_Vdbe_جمع_کن_Op3(v, OP_Make_رکورد, r, nPk, regRowid);
                sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Idx_درج, regRowset, regRowid,
                                     r, nPk);
                إذا( i_مجموعه ) sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
              }

              /* Release the array of temp registers */
              sqlite3_Release_موقت_Range(pParse, r, nPk);
            }
          }

          /* Invoke the main loop body as a subroutine */
          sqlite3_Vdbe_جمع_کن_Op2(v, OP_Gosub, regReturn, iLoopBody);

          /* Jump here (skipping the main loop body subroutine) if the
          ** current sub-WHERE row is a duplicate from prior sub-WHEREs. */
          إذا( jmp1 ) sqlite3_Vdbe_Jump_اینجا(v, jmp1);

          /* The pSubWInfo->untestedTerms flag means that this OR term
          ** contained one or more AND term from a notReady table.  The
          ** terms from the notReady table could not be tested and will
          ** need to be tested later.
          */
          إذا( pSubWInfo->untestedTerms ) untestedTerms = 1;

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
          أكد( (pSubLoop->wsFlags & WHERE_AUTO_شاخص)==0 );
          إذا( (pSubLoop->wsFlags & WHERE_نمایهای)!=0
           && (ii==0 || pSubLoop->u.btree.p_شاخص==pCov)
           && (HasRowid(pTab) || !Is_Primary_کلید_شاخص(pSubLoop->u.btree.p_شاخص))
          ){
            أكد( pSubWInfo->a[0].iIdxCur==iCovCur );
            pCov = pSubLoop->u.btree.p_شاخص;
          }وإلا{
            pCov = 0;
          }
          إذا( sqlite3_Where_استفاده_میکند_Deferred_Seek(pSubWInfo) ){
            pWInfo->bDeferredSeek = 1;
          }

          /* Finish the loop through table entries that match term pOrTerm. */
          sqlite3_Where_انتها(pSubWInfo);
          Explain_Query_Plan_برداشت(pParse);
        }
        sqlite3_عبارت_Delete(db, pDelete);
      }
    }
    Explain_Query_Plan_برداشت(pParse);
    أكد( pLevel->pWLoop==pLoop );
    أكد( (pLoop->wsFlags & WHERE_MULTI_OR)!=0 );
    أكد( (pLoop->wsFlags & WHERE_درون_ABLE)==0 );
    pLevel->u.pCoveringIdx = pCov;
    إذا( pCov ) pLevel->iIdxCur = iCovCur;
    إذا( p_And_عبارت ){
      p_And_عبارت->p_چپ = 0;
      sqlite3_عبارت_Delete(db, p_And_عبارت);
    }
    sqlite3VdbeChangeP1(v, iRetInit, sqlite3VdbeCurrentAddr(v));
    sqlite3VdbeGoto(v, pLevel->addrBrk);
    sqlite3VdbeResolveLabel(v, iLoopBody);

    /* Set the P2 operand of the OP_Return opcode that will end the current
    ** loop to point to this spot, which is the top of the next containing
    ** loop.  The byte-code formatter will use that P2 value as a hint to
    ** indent everything in between the this point and the final OP_Return.
    ** See tag-20220407a in vdbe.c and shell.c */
    أكد( pLevel->op==OP_Return );
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    إذا( pWInfo->pTabList!=pOrTab ){ sqlite3DbFreeNN(db, pOrTab); }
    إذا( !untestedTerms ) disable_جمله(pLevel, p_جمله);
  }وإلا
#پایان_اگر /* SQLITE_OMIT_OR_OPTIMIZATION */

  {
    /* Case 6:  There is no usable index.  We must do a complete
    **          scan of the entire table.
    */
    ساكن ثابت u8 a_گام[] = { OP_بعدی, OP_Prev };
    ساكن ثابت u8 a_شروع[] = { OP_Rewind, OP_آخرین };
    أكد( bRev==0 || bRev==1 );
    إذا( p_Tab_قلم->fg.isRecursive ){
      /* Tables marked isRecursive have only a single row that is stored in
      ** a pseudo-cursor.  No need to Rewind or Next such cursors. */
      pLevel->op = OP_Noop;
    }وإلا{
      کد_Cursor_Hint(p_Tab_قلم, pWInfo, pLevel, 0);
      pLevel->op = a_گام[bRev];
      pLevel->p1 = iCur;
      pLevel->p2 = 1 + sqlite3_Vdbe_جمع_کن_Op2(v, a_شروع[bRev],iCur,pLevel->addrHalt);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_گام;
    }
  }

#اگر_تعریف_شده SQLITE_ENABLE_STMT_SCANSTATUS
  pLevel->addr_بازدید = sqlite3VdbeCurrentAddr(v);
#پایان_اگر

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
  افعل{
    صحيح i_بعدی = 0;                /* Next value for iLoop */
    لكل(p_جمله=pWC->a, j=pWC->n_جمله; j>0; j--, p_جمله++){
      عبارت *pE;
      صحيح skipLikeAddr = 0;
      testcase( p_جمله->wtFlags & جمله_VIRTUAL );
      testcase( p_جمله->wtFlags & جمله_CODED );
      إذا( p_جمله->wtFlags & (جمله_VIRTUAL|جمله_CODED) ) استمر;
      إذا( (p_جمله->prereqAll & pLevel->notReady)!=0 ){
        testcase( pWInfo->untestedTerms==0
            && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 );
        pWInfo->untestedTerms = 1;
        استمر;
      }
      pE = p_جمله->p_عبارت;
      أكد( pE!=0 );
      إذا( where_عبارت_Is_Ready(pWInfo, pLevel, p_Tab_قلم, pE)==0 ) استمر;
      إذا( iLoop==1 && !sqlite3_عبارت_Covered_By_شاخص(pE, pLevel->iTabCur, pIdx) ){
        i_بعدی = 2;
        استمر;
      }
      إذا( iLoop<3 && (p_جمله->wtFlags & جمله_VARSELECT) ){
        إذا( i_بعدی==0 ) i_بعدی = 3;
        استمر;
      }

      إذا( (p_جمله->wtFlags & جمله_LIKECOND)!=0 ){
        /* If the TERM_LIKECOND flag is set, that means that the range search
        ** is sufficient to guarantee that the LIKE operator is true, so we
        ** can skip the call to the like(A,B) function.  But this only works
        ** for strings.  So do not skip the call to the function on the pass
        ** that compares BLOBs. */
#اگر_تعریف_شده SQLITE_LIKE_DOESNT_تطابق_BLOBS
        استمر;
#وإلا
        u32 x = pLevel->iLikeRepCntr;
        إذا( x>0 ){
          skipLikeAddr = sqlite3_Vdbe_جمع_کن_Op1(v, (x&1)?OP_IfNot:OP_If,(صحيح)(x>>1));
          VdbeCoverageIf(v, (x&1)==1);
          VdbeCoverageIf(v, (x&1)==0);
        }
#پایان_اگر
      }
#اگر_تعریف_شده WHERETRACE_ENABLED /* 0xffffffff */
      إذا( sqlite3WhereTrace ){
        VdbeNoopComment((v, "WhereTerm[%d] (%p) priority=%d",
                         pWC->n_جمله-j, p_جمله, iLoop));
      }
      إذا( sqlite3WhereTrace & 0x4000 ){
        sqlite3DebugPrintf("Coding auxiliary constraint:\n");
        sqlite3_Where_جمله_Print(p_جمله, pWC->n_جمله-j);
      }
#پایان_اگر
      sqlite3_عبارت_If_False(pParse, pE, addrCont, SQLITE_JUMPIFNULL);
      إذا( skipLikeAddr ) sqlite3_Vdbe_Jump_اینجا(v, skipLikeAddr);
      p_جمله->wtFlags |= جمله_CODED;
    }
    iLoop = i_بعدی;
  }طالما( iLoop>0 );

  /* Insert code to test for implied constraints based on transitivity
  ** of the "==" operator.
  **
  ** Example: If the WHERE clause contains "t1.a=t2.b" and "t2.b=123"
  ** and we are coding the t1 loop and the t2 loop has not yet coded,
  ** then we cannot use the "t1.a=t2.b" constraint, but we can code
  ** the implied "t1.a=123" constraint.
  */
  لكل(p_جمله=pWC->a, j=pWC->n_پایه; j>0; j--, p_جمله++){
    عبارت *pE, sEAlt;
    Where_جمله *pAlt;
    إذا( p_جمله->wtFlags & (جمله_VIRTUAL|جمله_CODED) ) استمر;
    إذا( (p_جمله->eOperator & (WO_EQ|WO_IS))==0 ) استمر;
    إذا( (p_جمله->eOperator & WO_EQUIV)==0 ) استمر;
    إذا( p_جمله->چپ_Cursor!=iCur ) استمر;
    إذا( p_Tab_قلم->fg.jointype & (JT_چپ|JT_LTORJ|JT_راست) ) استمر;
    pE = p_جمله->p_عبارت;
#اگر_تعریف_شده WHERETRACE_ENABLED /* 0x4001 */
    إذا( (sqlite3WhereTrace & 0x4001)==0x4001 ){
      sqlite3DebugPrintf("Coding transitive constraint:\n");
      sqlite3_Where_جمله_Print(p_جمله, pWC->n_جمله-j);
    }
#پایان_اگر
    أكد( !عبارت_Has_Property(pE, EP_OuterON) );
    أكد( (p_جمله->prereq_راست & pLevel->notReady)!=0 );
    أكد( (p_جمله->eOperator & (WO_OR|WO_AND))==0 );
    pAlt = sqlite3_Where_پیدا_کن_جمله(pWC, iCur, p_جمله->u.x.چپ_ستونی, notReady,
                    WO_EQ|WO_درون|WO_IS, 0);
    إذا( pAlt==0 ) استمر;
    إذا( pAlt->wtFlags & (جمله_CODED) ) استمر;
    إذا( عبارت_Has_Property(pAlt->p_عبارت, EP_Collate) ) استمر;
    إذا( (pAlt->eOperator & WO_درون)
     && عبارت_Use_XSelect(pAlt->p_عبارت)
     && (pAlt->p_عبارت->x.pSelect->pEList->n_عبارت>1)
    ){
      استمر;
    }
    testcase( pAlt->eOperator & WO_EQ );
    testcase( pAlt->eOperator & WO_IS );
    testcase( pAlt->eOperator & WO_درون );
    VdbeModuleComment((v, "begin transitive constraint"));
    sEAlt = *pAlt->p_عبارت;
    sEAlt.p_چپ = pE->p_چپ;
    sqlite3_عبارت_If_False(pParse, &sEAlt, addrCont, SQLITE_JUMPIFNULL);
    pAlt->wtFlags |= جمله_CODED;
  }

  /* For a RIGHT OUTER JOIN, record the fact that the current row has
  ** been matched at least once.
  */
  إذا( pLevel->pRJ ){
    جدول *pTab;
    صحيح nPk;
    صحيح r;
    صحيح jmp1 = 0;
    Where_راست_Join *pRJ = pLevel->pRJ;

    /* pTab is the right-hand table of the RIGHT JOIN.  Generate code that
    ** will record that the current row of that table has been matched at
    ** least once.  This is accomplished by storing the PK for the row in
    ** both the iMatch index and the regBloom Bloom filter.
    */
    pTab = pWInfo->pTabList->a[pLevel->iFrom].pSTab;
    إذا( HasRowid(pTab) ){
      r = sqlite3_Get_موقت_Range(pParse, 2);
      sqlite3_عبارت_کد_Get_ستونی_از_جدول(v, pTab, pLevel->iTabCur, -1, r+1);
      nPk = 1;
    }وإلا{
      صحيح iPk;
      شاخص *pPk = sqlite3_Primary_کلید_شاخص(pTab);
      nPk = pPk->n_کلید_ستون;
      r = sqlite3_Get_موقت_Range(pParse, nPk+1);
      لكل(iPk=0; iPk<nPk; iPk++){
        صحيح i_ستون = pPk->ai_ستونی[iPk];
        sqlite3_عبارت_کد_Get_ستونی_از_جدول(v, pTab, iCur, i_ستون,r+1+iPk);
      }
    }
    jmp1 = sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_یافت, pRJ->i_تطابق, 0, r+1, nPk);
    VdbeCoverage(v);
    VdbeComment((v, "match against %s", pTab->zName));
    sqlite3_Vdbe_جمع_کن_Op3(v, OP_Make_رکورد, r+1, nPk, r);
    sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Idx_درج, pRJ->i_تطابق, r, r+1, nPk);
    إذا( pRJ->regBloom ){
      sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Filter_جمع_کن, pRJ->regBloom, 0, r+1, nPk);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
    }
    sqlite3_Vdbe_Jump_اینجا(v, jmp1);
    sqlite3_Release_موقت_Range(pParse, r, nPk+1);
  }

  /* For a LEFT OUTER JOIN, generate code that will record the fact that
  ** at least one row of the right table has matched the left table.
  */
  إذا( pLevel->i_چپ_Join ){
    pLevel->addr_اولی = sqlite3VdbeCurrentAddr(v);
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_Integer, 1, pLevel->i_چپ_Join);
    VdbeComment((v, "record LEFT JOIN hit"));
    إذا( pLevel->pRJ==0 ){
      اذهب کد_outer_join_constraints; /* WHERE clause constraints */
    }
  }

  إذا( pLevel->pRJ ){
    /* Create a subroutine used to process all interior loops and code
    ** of the RIGHT JOIN.  During normal operation, the subroutine will
    ** be in-line with the rest of the code.  But at the end, a separate
    ** loop will run that invokes this subroutine for unmatched rows
    ** of pTab, with all tables to left begin set to NULL.
    */
    {
      Where_راست_Join *pRJ = pLevel->pRJ;
      sqlite3_Vdbe_جمع_کن_Op2(v, OP_BeginSubrtn, 0, pRJ->regReturn);
      pRJ->addrSubrtn = sqlite3VdbeCurrentAddr(v);
    }
    أكد( pParse->withinRJSubrtn < 255 );
    pParse->withinRJSubrtn++;

    /* WHERE clause constraints must be deferred until after outer join
    ** row elimination has completed, since WHERE clause constraints apply
    ** to the results of the OUTER JOIN.  The following loop generates the
    ** appropriate WHERE clause constraint checks.  tag-20220513a.
    */
  کد_outer_join_constraints:
    لكل(p_جمله=pWC->a, j=0; j<pWC->n_پایه; j++, p_جمله++){
      testcase( p_جمله->wtFlags & جمله_VIRTUAL );
      testcase( p_جمله->wtFlags & جمله_CODED );
      إذا( p_جمله->wtFlags & (جمله_VIRTUAL|جمله_CODED) ) استمر;
      إذا( (p_جمله->prereqAll & pLevel->notReady)!=0 ){
        أكد( pWInfo->untestedTerms );
        استمر;
      }
      إذا( p_Tab_قلم->fg.jointype & JT_LTORJ ) استمر;
      أكد( p_جمله->p_عبارت );
      sqlite3_عبارت_If_False(pParse, p_جمله->p_عبارت, addrCont, SQLITE_JUMPIFNULL);
      p_جمله->wtFlags |= جمله_CODED;
    }
  }

#إذا WHERETRACE_ENABLED /* 0x4001 */
  إذا( sqlite3WhereTrace & 0x4000 ){
    sqlite3DebugPrintf("All WHERE-clause terms after coding level %d:\n",
                       iLevel);
    sqlite3WhereClausePrint(pWC);
  }
  إذا( sqlite3WhereTrace & 0x1 ){
    sqlite3DebugPrintf("End Coding level %d:  notReady=%llx\n",
       iLevel, (u64)pLevel->notReady);
  }
#پایان_اگر
  أرجع pLevel->notReady;
}

/*
** Generate the code for the loop that finds all non-matched terms
** for a RIGHT JOIN.
*/
SQLITE_NOINLINE فراغ sqlite3_Where_راست_Join_Loop(
  WhereInfo *pWInfo,
  صحيح iLevel,
  WhereLevel *pLevel
){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  Where_راست_Join *pRJ = pLevel->pRJ;
  عبارت *pSubWhere = 0;
  WhereClause *pWC = &pWInfo->sWC;
  WhereInfo *pSubWInfo;
  WhereLoop *pLoop = pLevel->pWLoop;
  Src_قلم *p_Tab_قلم = &pWInfo->pTabList->a[pLevel->iFrom];
  SrcList *pFrom;
  اتحاد {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;
  Bitmask mAll = 0;
  صحيح k;

  ExplainQueryPlan((pParse, 1, "RIGHT-JOIN %s", p_Tab_قلم->pSTab->zName));
  sqlite3VdbeNoJumpsOutsideSubrtn(v, pRJ->addrSubrtn, pRJ->انتها_Subrtn,
                                  pRJ->regReturn);
  لكل(k=0; k<iLevel; k++){
    صحيح iIdxCur;
    Src_قلم *p_راست;
    أكد( pWInfo->a[k].pWLoop->iTab == pWInfo->a[k].iFrom );
    p_راست = &pWInfo->pTabList->a[pWInfo->a[k].iFrom];
    mAll |= pWInfo->a[k].pWLoop->ماسک_Self;
    إذا( p_راست->fg.از_طریق_Coroutine ){
      Subquery *pSubq;
      أكد( p_راست->fg.isSubquery && p_راست->u4.pSubq!=0 );
      pSubq = p_راست->u4.pSubq;
      أكد( pSubq->pSelect!=0 && pSubq->pSelect->pEList!=0 );
      sqlite3_Vdbe_جمع_کن_Op3(
          v, OP_Null, 0, pSubq->reg_نتیجه,
          pSubq->reg_نتیجه + pSubq->pSelect->pEList->n_عبارت-1
      );
    }
    sqlite3_Vdbe_جمع_کن_Op1(v, OP_Null_ردیف, pWInfo->a[k].iTabCur);
    iIdxCur = pWInfo->a[k].iIdxCur;
    إذا( iIdxCur ){
      sqlite3_Vdbe_جمع_کن_Op1(v, OP_Null_ردیف, iIdxCur);
    }
  }
  إذا( (p_Tab_قلم->fg.jointype & JT_LTORJ)==0 ){
    mAll |= pLoop->ماسک_Self;
    لكل(k=0; k<pWC->n_جمله; k++){
      Where_جمله *p_جمله = &pWC->a[k];
      إذا( (p_جمله->wtFlags & (جمله_VIRTUAL|جمله_برش))!=0
       && p_جمله->eOperator!=WO_ROWVAL
      ){
        اكسر;
      }
      إذا( p_جمله->prereqAll & ~mAll ) استمر;
      إذا( عبارت_Has_Property(p_جمله->p_عبارت, EP_OuterON|EP_InnerON) ) استمر;
      pSubWhere = sqlite3_عبارت_And(pParse, pSubWhere,
                                 sqlite3_عبارت_Dup(pParse->db, p_جمله->p_عبارت, 0));
    }
  }
  إذا( pLevel->iIdxCur ){
    /* pSubWhere may contain expressions that read from an index on the
    ** table on the RHS of the right join. All such expressions first test
    ** if the index is pointing at a NULL row, and if so, read from the
    ** table cursor instead. So ensure that the index cursor really is 
    ** pointing at a NULL row here, so that no values are read from it during
    ** the scan of the RHS of the RIGHT join below.  */
    sqlite3_Vdbe_جمع_کن_Op1(v, OP_Null_ردیف, pLevel->iIdxCur);
  }
  pFrom = &uSrc.sSrc;
  pFrom->nSrc = 1;
  pFrom->nAlloc = 1;
  کپی_حافظه(&pFrom->a[0], p_Tab_قلم, حجم(Src_قلم));
  pFrom->a[0].fg.jointype = 0;
  إذا( pParse->withinRJSubrtn >= 100 ){
    /* This limit --------------^^^
    ** is based on an historical assert().  It is not compile-time or
    ** run-time configurable.  It could perhaps be raised as high as 254,
    ** but only an attack robot would ever do even 100 RIGHT JOINS within
    ** a single query, so we'll just leave it as it is. */
    sqlite3_Error_پیام(pParse, "too many RIGHT JOINs");
    أرجع;
  }
  pParse->withinRJSubrtn++;
  pSubWInfo = sqlite3WhereBegin(pParse, pFrom, pSubWhere, 0, 0, 0,
                                WHERE_راست_JOIN, 0);
  إذا( pSubWInfo ){
    صحيح iCur = pLevel->iTabCur;  /* Table on RHS of RIGHT JOIN &*/
    صحيح r = ++pParse->nMem;      /* Register range to hold primary key */
    صحيح nPk;                     /* Number of values in the primary key */
    صحيح r2;                      /* Register holding record for primary key */
    صحيح addrCont = sqlite3WhereContinueLabel(pSubWInfo);
    جدول *pTab = p_Tab_قلم->pSTab;  /* Table on RHS of RIGHT JOIN */
    إذا( HasRowid(pTab) ){
      sqlite3_عبارت_کد_Get_ستونی_از_جدول(v, pTab, iCur, -1, r);
      nPk = 1;
    }وإلا{
      صحيح iPk;
      شاخص *pPk = sqlite3_Primary_کلید_شاخص(pTab);
      nPk = pPk->n_کلید_ستون;
      pParse->nMem += nPk - 1;
      لكل(iPk=0; iPk<nPk; iPk++){
        صحيح i_ستون = pPk->ai_ستونی[iPk];
        sqlite3_عبارت_کد_Get_ستونی_از_جدول(v, pTab, iCur, i_ستون,r+iPk);
      }
    }

    /* Generate code that checks to see if the current row of the RHS table
    ** has appeared in any prior output row. */
    إذا( pRJ->regBloom ){
      sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Filter, pRJ->regBloom, 
                 sqlite3VdbeCurrentAddr(v)+2, r, nPk);
      VdbeCoverage(v);
    }
    sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_یافت, pRJ->i_تطابق, addrCont, r, nPk);
    VdbeCoverage(v);
    r2 = sqlite3_Get_موقت_Reg(pParse);

    /* Generate code that inserts the PK of the RHS table into the
    ** pRH->iMatch index to indicate that the current row has appeared
    ** in the output set. */
    sqlite3_Vdbe_جمع_کن_Op3(v, OP_Make_رکورد, r, nPk, r2);
    sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Idx_درج, pRJ->i_تطابق, r2, r, nPk);
    sqlite3_Release_موقت_Reg(pParse, r2);
    إذا( pRJ->regBloom ){
      sqlite3_Vdbe_جمع_کن_Op4_Int(v, OP_Filter_جمع_کن, pRJ->regBloom, 0, r, nPk);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
    }

    /* Invoke the subroutine that actually puts the current RHS table row
    ** into the output set, with NULLs for the LHS. */
    sqlite3_Vdbe_جمع_کن_Op2(v, OP_Gosub, pRJ->regReturn, pRJ->addrSubrtn);
    sqlite3_Where_انتها(pSubWInfo);
  }
  sqlite3_عبارت_Delete(pParse->db, pSubWhere);
  Explain_Query_Plan_برداشت(pParse);
  أكد( pParse->withinRJSubrtn>0 );
  pParse->withinRJSubrtn--;
}
