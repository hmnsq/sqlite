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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "whereInt.h"

#यदि_अपरिभाषितम् SQLITE_OMIT_EXPLAIN

/*
** Return the name of the i-th column of the pIdx index.
*/
स्थिर ध्रुव वर्ण *explain_सूचकांक_Column_Name(सूचकांक *pIdx, पूर्णाङ्क i){
  i = pIdx->aiColumn[i];
  यदि( i==XN_व्यञ्जकम् ) प्रत्यागम "<expr>";
  यदि( i==XN_ROWID ) प्रत्यागम "rowid";
  प्रत्यागम pIdx->p_सारणी->aCol[i].zCnName;
}

/*
** This routine is a helper for explainIndexRange() below
**
** pStr holds the text of an expression that we are building up one term
** at a time.  This routine adds a new term to the end of the expression.
** Terms are separated by AND so add the "AND" text for second and subsequent
** terms only.
*/
स्थिर शून्य explain_Append_पदांशः(
  StrAccum *pStr,             /* The text expression being built */
  सूचकांक *pIdx,                /* Index to read column names from */
  पूर्णाङ्क n_पदांशः,                  /* Number of terms */
  पूर्णाङ्क i_पदांशः,                  /* Zero-based index of first term. */
  पूर्णाङ्क bAnd,                   /* Non-zero to append " AND " */
  ध्रुव वर्ण *zOp             /* Name of the operator */
){
  पूर्णाङ्क i;

  प्रतिज्ञा( n_पदांशः>=1 );
  यदि( bAnd ) sqlite3_str_append(pStr, " AND ", 5);

  यदि( n_पदांशः>1 ) sqlite3_str_append(pStr, "(", 1);
  प्रति(i=0; i<n_पदांशः; i++){
    यदि( i ) sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_appendall(pStr, explain_सूचकांक_Column_Name(pIdx, i_पदांशः+i));
  }
  यदि( n_पदांशः>1 ) sqlite3_str_append(pStr, ")", 1);

  sqlite3_str_append(pStr, zOp, 1);

  यदि( n_पदांशः>1 ) sqlite3_str_append(pStr, "(", 1);
  प्रति(i=0; i<n_पदांशः; i++){
    यदि( i ) sqlite3_str_append(pStr, ",", 1);
    sqlite3_str_append(pStr, "?", 1);
  }
  यदि( n_पदांशः>1 ) sqlite3_str_append(pStr, ")", 1);
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
स्थिर शून्य explain_सूचकांक_Range(StrAccum *pStr, WhereLoop *pLoop){
  सूचकांक *p_सूचकांक = pLoop->u.btree.p_सूचकांक;
  u16 nEq = pLoop->u.btree.nEq;
  u16 nSkip = pLoop->nSkip;
  पूर्णाङ्क i, j;

  यदि( nEq==0 && (pLoop->wsFlags&(WHERE_BTM_सीमा|WHERE_शिखरम्_सीमा))==0 ) प्रत्यागम;
  sqlite3_str_append(pStr, " (", 2);
  प्रति(i=0; i<nEq; i++){
    ध्रुव वर्ण *z = explain_सूचकांक_Column_Name(p_सूचकांक, i);
    यदि( i ) sqlite3_str_append(pStr, " AND ", 5);
    sqlite3_str_appendf(pStr, i>=nSkip ? "%s=?" : "ANY(%s)", z);
  }

  j = i;
  यदि( pLoop->wsFlags&WHERE_BTM_सीमा ){
    explain_Append_पदांशः(pStr, p_सूचकांक, pLoop->u.btree.nBtm, j, i, ">");
    i = 1;
  }
  यदि( pLoop->wsFlags&WHERE_शिखरम्_सीमा ){
    explain_Append_पदांशः(pStr, p_सूचकांक, pLoop->u.btree.n_शिखरम्, j, i, "<");
  }
  sqlite3_str_append(pStr, ")", 1);
}

/*
** This function sets the P4 value of an existing OP_Explain opcode to
** text describing the loop in pLevel. If the OP_Explain opcode already has
** a P4 value, it is freed before it is overwritten.
*/
शून्य sqlite3_Where_सङ्कलय_Explain_Text(
  Parse *pParse,                  /* Parse context */
  पूर्णाङ्क addr,                       /* Address of OP_Explain opcode */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
#यदि !defined(SQLITE_DEBUG)
  यदि( sqlite3ParseToplevel(pParse)->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#यदि_अन्तः
  {
    VdbeOp *pOp = sqlite3VdbeGetOp(pParse->pVdbe, addr);
    SrcItem *pItem = &pTabList->a[pLevel->iFrom];
    sqlite3 *db = pParse->db;     /* Database handle */
    पूर्णाङ्क is_अन्वेषणम्;                 /* True for a SEARCH. False for SCAN. */
    WhereLoop *pLoop;             /* The controlling WhereLoop object */
    u32 flags;                    /* Flags that describe this loop */
#यदि defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_EXPLAIN)
    वर्ण *z_सन्देशः;                   /* Text to add to EQP output */
#यदि_अन्तः
    StrAccum str;                 /* EQP output string */
    वर्ण zBuf[100];               /* Initial space for EQP output string */

    यदि( db->mallocFailed ) प्रत्यागम;

    pLoop = pLevel->pWLoop;
    flags = pLoop->wsFlags;

    is_अन्वेषणम् = (flags&(WHERE_BTM_सीमा|WHERE_शिखरम्_सीमा))!=0
            || ((flags&WHERE_VIRTUALTABLE)==0 && (pLoop->u.btree.nEq>0))
            || (wctrlFlags&(WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX));

    sqlite3StrAccumInit(&str, db, zBuf, परिमाण(zBuf), SQLITE_MAX_दीर्घता);
    str.printfFlags = SQLITE_PRINTF_आन्तरिकम्;
    sqlite3_str_appendf(&str, "%s %S%s",
       is_अन्वेषणम् ? "SEARCH" : "SCAN",
       pItem,
       pItem->fg.fromExists ? " EXISTS" : "");
    यदि( (flags & (WHERE_IPK|WHERE_VIRTUALTABLE))==0 ){
      ध्रुव वर्ण *zFmt = 0;
      सूचकांक *pIdx;

      प्रतिज्ञा( pLoop->u.btree.p_सूचकांक!=0 );
      pIdx = pLoop->u.btree.p_सूचकांक;
      प्रतिज्ञा( !(flags&WHERE_AUTO_सूचकांक) || (flags&WHERE_IDX_ONLY) );
      यदि( !HasRowid(pItem->pSTab) && Is_Primary_कुञ्चिका_सूचकांक(pIdx) ){
        यदि( is_अन्वेषणम् ){
          zFmt = "PRIMARY KEY";
        }
      }अन्यथा यदि( flags & WHERE_PARTIALIDX ){
        zFmt = "AUTOMATIC PARTIAL COVERING INDEX";
      }अन्यथा यदि( flags & WHERE_AUTO_सूचकांक ){
        zFmt = "AUTOMATIC COVERING INDEX";
      }अन्यथा यदि( flags & (WHERE_IDX_ONLY|WHERE_EXPRIDX) ){
        zFmt = "COVERING INDEX %s";
      }अन्यथा{
        zFmt = "INDEX %s";
      }
      यदि( zFmt ){
        sqlite3_str_append(&str, " USING ", 7);
        sqlite3_str_appendf(&str, zFmt, pIdx->zName);
        explain_सूचकांक_Range(&str, pLoop);
      }
    }अन्यथा यदि( (flags & WHERE_IPK)!=0 && (flags & WHERE_CONSTRAINT)!=0 ){
      वर्ण cRangeOp;
#यदि 0  /* Better output, but breaks many tests */
      ध्रुव सारणी *pTab = pItem->pTab;
      ध्रुव वर्ण *zRowid = pTab->iPKey>=0 ? pTab->aCol[pTab->iPKey].zCnName:
                              "rowid";
#अन्यथा
      ध्रुव वर्ण *zRowid = "rowid";
#यदि_अन्तः
      sqlite3_str_appendf(&str, " USING INTEGER PRIMARY KEY (%s", zRowid);
      यदि( flags&(WHERE_COLUMN_EQ|WHERE_COLUMN_अन्तर्) ){
        cRangeOp = '=';
      }अन्यथा यदि( (flags&WHERE_BOTH_सीमा)==WHERE_BOTH_सीमा ){
        sqlite3_str_appendf(&str, ">? AND %s", zRowid);
        cRangeOp = '<';
      }अन्यथा यदि( flags&WHERE_BTM_सीमा ){
        cRangeOp = '>';
      }अन्यथा{
        प्रतिज्ञा( flags&WHERE_शिखरम्_सीमा);
        cRangeOp = '<';
      }
      sqlite3_str_appendf(&str, "%c?)", cRangeOp);
    }
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
    अन्यथा यदि( (flags & WHERE_VIRTUALTABLE)!=0 ){
      sqlite3_str_appendall(&str, " VIRTUAL TABLE INDEX ");
      sqlite3_str_appendf(&str,
                  pLoop->u.vtab.b_Idx_अङ्कः_Hex ? "0x%x:%s" : "%d:%s",
                  pLoop->u.vtab.idx_अङ्कः, pLoop->u.vtab.idxStr);
    }
#यदि_अन्तः
    यदि( pItem->fg.jointype & JT_वामः ){
      sqlite3_str_appendf(&str, " LEFT-JOIN");
    }
#यदि_परिभाषितम् SQLITE_EXPLAIN_ESTIMATED_पङ्क्तिकाः
    यदि( pLoop->n_बहिर्गमनम्>=10 ){
      sqlite3_str_appendf(&str, " (~%llu rows)",
             sqlite3_Log_Est_गन्तव्यम्_Int(pLoop->n_बहिर्गमनम्));
    }अन्यथा{
      sqlite3_str_append(&str, " (~1 row)", 9);
    }
#यदि_अन्तः
#यदि defined(SQLITE_DEBUG) && !defined(SQLITE_OMIT_EXPLAIN)
    z_सन्देशः = sqlite3StrAccumFinish(&str);
    sqlite3ExplainBreakpoint("",z_सन्देशः);
#यदि_अन्तः

    प्रतिज्ञा( pOp->opcode==OP_Explain );
    प्रतिज्ञा( pOp->p4type==P4_DYNAMIC || pOp->p4.z==0 );
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
पूर्णाङ्क sqlite3_Where_Explain_One_अवलोकनम्(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
  पूर्णाङ्क ret = 0;
#यदि !defined(SQLITE_DEBUG)
  यदि( sqlite3ParseToplevel(pParse)->explain==2 || IS_STMT_SCANSTATUS(pParse->db) )
#यदि_अन्तः
  {
    यदि( (pLevel->pWLoop->wsFlags & WHERE_MULTI_OR)==0
     && (wctrlFlags & WHERE_OR_SUBCLAUSE)==0
    ){
      Vdbe *v = pParse->pVdbe;
      पूर्णाङ्क addr = sqlite3VdbeCurrentAddr(v);
      ret = sqlite3_Vdbe_सङ्कलय_Op3(
          v, OP_Explain, addr, pParse->addrExplain, pLevel->pWLoop->r_सततम्
      );
      sqlite3_Where_सङ्कलय_Explain_Text(pParse, addr, pTabList, pLevel, wctrlFlags);
    }
  }
  प्रत्यागम ret;
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
पूर्णाङ्क sqlite3WhereExplainBloomFilter(
  ध्रुव Parse *pParse,               /* Parse context */
  ध्रुव WhereInfo *pWInfo,           /* WHERE clause */
  ध्रुव WhereLevel *pLevel           /* Bloom filter on this level */
){
  पूर्णाङ्क ret = 0;
  SrcItem *pItem = &pWInfo->pTabList->a[pLevel->iFrom];
  Vdbe *v = pParse->pVdbe;      /* VM being constructed */
  sqlite3 *db = pParse->db;     /* Database handle */
  वर्ण *z_सन्देशः;                   /* Text to add to EQP output */
  पूर्णाङ्क i;                        /* Loop counter */
  WhereLoop *pLoop;             /* The where loop */
  StrAccum str;                 /* EQP output string */
  वर्ण zBuf[100];               /* Initial space for EQP output string */

  sqlite3StrAccumInit(&str, db, zBuf, परिमाण(zBuf), SQLITE_MAX_दीर्घता);
  str.printfFlags = SQLITE_PRINTF_आन्तरिकम्;
  sqlite3_str_appendf(&str, "BLOOM FILTER ON %S (", pItem);
  pLoop = pLevel->pWLoop;
  यदि( pLoop->wsFlags & WHERE_IPK ){
    ध्रुव सारणी *pTab = pItem->pSTab;
    यदि( pTab->iPKey>=0 ){
      sqlite3_str_appendf(&str, "%s=?", pTab->aCol[pTab->iPKey].zCnName);
    }अन्यथा{
      sqlite3_str_appendf(&str, "rowid=?");
    }
  }अन्यथा{
    प्रति(i=pLoop->nSkip; i<pLoop->u.btree.nEq; i++){
      ध्रुव वर्ण *z = explain_सूचकांक_Column_Name(pLoop->u.btree.p_सूचकांक, i);
      यदि( i>pLoop->nSkip ) sqlite3_str_append(&str, " AND ", 5);
      sqlite3_str_appendf(&str, "%s=?", z);
    }
  }
  sqlite3_str_append(&str, ")", 1);
  z_सन्देशः = sqlite3StrAccumFinish(&str);
  ret = sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Explain, sqlite3VdbeCurrentAddr(v),
                          pParse->addrExplain, 0, z_सन्देशः,P4_DYNAMIC);

  sqlite3_Vdbe_अवलोकनम्_स्थितिः(v, sqlite3VdbeCurrentAddr(v)-1, 0, 0, 0, 0);
  प्रत्यागम ret;
}
#यदि_अन्तः /* SQLITE_OMIT_EXPLAIN */

#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Configure the VM passed as the first argument with an
** sqlite3_stmt_scanstatus() entry corresponding to the scan used to
** implement level pLvl. Argument pSrclist is a pointer to the FROM
** clause that the scan reads data from.
**
** If argument addrExplain is not 0, it must be the address of an
** OP_Explain instruction that describes the same loop.
*/
शून्य sqlite3_Where_सङ्कलय_अवलोकनम्_स्थितिः(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  पूर्णाङ्क addrExplain                 /* Address of OP_Explain (or 0) */
){
  यदि( IS_STMT_SCANSTATUS( sqlite3VdbeDb(v) ) ){
    ध्रुव वर्ण *zObj = 0;
    WhereLoop *pLoop = pLvl->pWLoop;
    पूर्णाङ्क wsFlags = pLoop->wsFlags;
    पूर्णाङ्क द्वारा_Coroutine = 0;

    यदि( (wsFlags & WHERE_VIRTUALTABLE)==0  &&  pLoop->u.btree.p_सूचकांक!=0 ){
      zObj = pLoop->u.btree.p_सूचकांक->zName;
    }अन्यथा{
      zObj = pSrclist->a[pLvl->iFrom].zName;
      द्वारा_Coroutine = pSrclist->a[pLvl->iFrom].fg.द्वारा_Coroutine;
    }
    sqlite3_Vdbe_अवलोकनम्_स्थितिः(
        v, addrExplain, pLvl->addrBody, pLvl->addr_दर्शय, pLoop->n_बहिर्गमनम्, zObj
    );

    यदि( द्वारा_Coroutine==0 ){
      यदि( (wsFlags & (WHERE_MULTI_OR|WHERE_AUTO_सूचकांक))==0 ){
        sqlite3_Vdbe_अवलोकनम्_स्थितिः_Range(v, addrExplain, -1, pLvl->iTabCur);
      }
      यदि( wsFlags & WHERE_अनुक्रमितम् ){
        sqlite3_Vdbe_अवलोकनम्_स्थितिः_Range(v, addrExplain, -1, pLvl->iIdxCur);
      }
    }अन्यथा{
      पूर्णाङ्क addr;
      VdbeOp *pOp;
      प्रतिज्ञा( pSrclist->a[pLvl->iFrom].fg.isSubquery );
      addr = pSrclist->a[pLvl->iFrom].u4.pSubq->addrFillSub;
      pOp = sqlite3VdbeGetOp(v, addr-1);
      प्रतिज्ञा( sqlite3VdbeDb(v)->mallocFailed || pOp->opcode==OP_InitCoroutine );
      प्रतिज्ञा( sqlite3VdbeDb(v)->mallocFailed || pOp->p2>addr );
      sqlite3_Vdbe_अवलोकनम्_स्थितिः_Range(v, addrExplain, addr, pOp->p2-1);
    }
  }
}
#यदि_अन्तः


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
स्थिर शून्य disable_पदांशः(WhereLevel *pLevel, Where_पदांशः *p_पदांशः){
  पूर्णाङ्क nLoop = 0;
  प्रतिज्ञा( p_पदांशः!=0 );
  यावत्( (p_पदांशः->wtFlags & पदांशः_CODED)==0
      && (pLevel->i_वामः_Join==0 || व्यञ्जकम्_Has_Property(p_पदांशः->p_व्यञ्जकम्, EP_OuterON))
      && (pLevel->notReady & p_पदांशः->prereqAll)==0
  ){
    यदि( nLoop && (p_पदांशः->wtFlags & पदांशः_LIKE)!=0 ){
      p_पदांशः->wtFlags |= पदांशः_LIKECOND;
    }अन्यथा{
      p_पदांशः->wtFlags |= पदांशः_CODED;
    }
#यदि_परिभाषितम् WHERETRACE_ENABLED
    यदि( (sqlite3WhereTrace & 0x4001)==0x4001 ){
      sqlite3DebugPrintf("DISABLE-");
      sqlite3_Where_पदांशः_Print(p_पदांशः, (पूर्णाङ्क)(p_पदांशः - (p_पदांशः->pWC->a)));
    }
#यदि_अन्तः
    यदि( p_पदांशः->iParent<0 ) भङ्ग;
    p_पदांशः = &p_पदांशः->pWC->a[p_पदांशः->iParent];
    प्रतिज्ञा( p_पदांशः!=0 );
    p_पदांशः->nChild--;
    यदि( p_पदांशः->nChild!=0 ) भङ्ग;
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
स्थिर शून्य संहिता_Apply_Affinity(Parse *pParse, पूर्णाङ्क आधारः, पूर्णाङ्क n, वर्ण *zAff){
  Vdbe *v = pParse->pVdbe;
  यदि( zAff==0 ){
    प्रतिज्ञा( pParse->db->mallocFailed );
    प्रत्यागम;
  }
  प्रतिज्ञा( v!=0 );

  /* Adjust base and n to skip over SQLITE_AFF_BLOB and SQLITE_AFF_NONE
  ** entries at the beginning and end of the affinity string.
  */
  प्रतिज्ञा( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  यावत्( n>0 && zAff[0]<=SQLITE_AFF_BLOB ){
    n--;
    आधारः++;
    zAff++;
  }
  यावत्( n>1 && zAff[n-1]<=SQLITE_AFF_BLOB ){
    n--;
  }

  /* Code the OP_Affinity opcode if there is anything left to do. */
  यदि( n>0 ){
    sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Affinity, आधारः, n, 0, zAff, n);
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
स्थिर शून्य updateRangeAffinityStr(
  व्यञ्जकम् *p_दक्षिणः,                   /* RHS of comparison */
  पूर्णाङ्क n,                          /* Number of vector elements in comparison */
  वर्ण *zAff                      /* Affinity string to modify */
){
  पूर्णाङ्क i;
  प्रति(i=0; i<n; i++){
    व्यञ्जकम् *p = sqlite3_सदिशम्_क्षेत्रम्_Subexpr(p_दक्षिणः, i);
    यदि( sqlite3CompareAffinity(p, zAff[i])==SQLITE_AFF_BLOB
     || sqlite3_व्यञ्जकम्_Needs_No_Affinity_Change(p, zAff[i])
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
स्थिर शून्य adjust_क्रमः_By_Col(व्यञ्जकम्_List *p_क्रमः_By, व्यञ्जकम्_List *pEList){
  पूर्णाङ्क i, j;
  यदि( p_क्रमः_By==0 ) प्रत्यागम;
  प्रति(i=0; i<p_क्रमः_By->n_व्यञ्जकम्; i++){
    पूर्णाङ्क t = p_क्रमः_By->a[i].u.x.i_क्रमः_By_Col;
    यदि( t==0 ) अनुवर्त;
    प्रति(j=0; j<pEList->n_व्यञ्जकम्; j++){
      यदि( pEList->a[j].u.x.i_क्रमः_By_Col==t ){
        p_क्रमः_By->a[i].u.x.i_क्रमः_By_Col = j+1;
        भङ्ग;
      }
    }
    यदि( j>=pEList->n_व्यञ्जकम् ){
      p_क्रमः_By->a[i].u.x.i_क्रमः_By_Col = 0;
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
स्थिर व्यञ्जकम् *remove_Unindexable_अन्तर्_Clause_Terms(
  Parse *pParse,        /* The parsing context */
  पूर्णाङ्क iEq,              /* Look at loop terms starting here */
  WhereLoop *pLoop,     /* The current loop */
  व्यञ्जकम् *pX              /* The IN expression to be reduced */
){
  sqlite3 *db = pParse->db;
  Select *pSelect;            /* Pointer to the SELECT on the RHS */
  व्यञ्जकम् *pNew;
  pNew = sqlite3_व्यञ्जकम्_Dup(db, pX, 0);
  यदि( db->mallocFailed==0 ){
    प्रति(pSelect=pNew->x.pSelect; pSelect; pSelect=pSelect->p_प्राक्){
      व्यञ्जकम्_List *pOrigRhs;         /* Original unmodified RHS */
      व्यञ्जकम्_List *pOrigLhs = 0;     /* Original unmodified LHS */
      व्यञ्जकम्_List *pRhs = 0;         /* New RHS after modifications */
      व्यञ्जकम्_List *pLhs = 0;         /* New LHS after mods */
      पूर्णाङ्क i;                      /* Loop counter */
      पूर्णाङ्क nRhs = 0;               /* Number of RHS terms added so far */

      प्रतिज्ञा( व्यञ्जकम्_Use_XSelect(pNew) );
      pOrigRhs = pSelect->pEList;
      प्रतिज्ञा( pNew->p_वामः!=0 );
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(pNew->p_वामः) );
      यदि( pSelect==pNew->x.pSelect ){
        pOrigLhs = pNew->p_वामः->x.pList;
      }
      प्रति(i=iEq; i<pLoop->nLTerm; i++){
        यदि( pLoop->aLTerm[i]->p_व्यञ्जकम्==pX ){
          पूर्णाङ्क i_क्षेत्रम्;
          प्रतिज्ञा( (pLoop->aLTerm[i]->eOperator & (WO_OR|WO_AND))==0 );
          i_क्षेत्रम् = pLoop->aLTerm[i]->u.x.i_क्षेत्रम् - 1;
          यदि( i_क्षेत्रम्!=nRhs ){
            व्यञ्जकम्_Clear_Property(pNew, EP_Subrtn);
          }
          यदि( NEVER(pOrigRhs->a[i_क्षेत्रम्].p_व्यञ्जकम्==0) ){
            अनुवर्त; /* Duplicate PK column */
          }
          pRhs = sqlite3_व्यञ्जकम्_List_Append(pParse, pRhs, pOrigRhs->a[i_क्षेत्रम्].p_व्यञ्जकम्);
          pOrigRhs->a[i_क्षेत्रम्].p_व्यञ्जकम् = 0;
          यदि( pRhs ) pRhs->a[pRhs->n_व्यञ्जकम्-1].u.x.i_क्रमः_By_Col = i_क्षेत्रम्+1;
          यदि( pOrigLhs ){
            प्रतिज्ञा( pOrigLhs->a[i_क्षेत्रम्].p_व्यञ्जकम्!=0 );
            pLhs = sqlite3_व्यञ्जकम्_List_Append(pParse,pLhs,pOrigLhs->a[i_क्षेत्रम्].p_व्यञ्जकम्);
            pOrigLhs->a[i_क्षेत्रम्].p_व्यञ्जकम् = 0;
          }
          nRhs++;
        }
      }
      यदि( nRhs<pOrigRhs->n_व्यञ्जकम् ){
        व्यञ्जकम्_Clear_Property(pNew, EP_Subrtn); /* See bug 2026-10-16T22:07:15Z */
      }
      sqlite3_व्यञ्जकम्_List_Delete(db, pOrigRhs);
      यदि( pOrigLhs ){
        sqlite3_व्यञ्जकम्_List_Delete(db, pOrigLhs);
        pNew->p_वामः->x.pList = pLhs;
      }
      pSelect->pEList = pRhs;
      pSelect->selId = ++pParse->nSelect; /* Req'd for SubrtnSig validity */
      यदि( pLhs && pLhs->n_व्यञ्जकम्==1 ){
        /* Take care here not to generate a TK_VECTOR containing only a
        ** single value. Since the parser never creates such a vector, some
        ** of the subroutines do not handle this case.  */
        व्यञ्जकम् *p = pLhs->a[0].p_व्यञ्जकम्;
        pLhs->a[0].p_व्यञ्जकम् = 0;
        sqlite3_व्यञ्जकम्_Delete(db, pNew->p_वामः);
        pNew->p_वामः = p;
      }

      /* If either the ORDER BY clause or the GROUP BY clause contains
      ** references to result-set columns, those references might now be
      ** obsolete.  So fix them up.
      */
      प्रतिज्ञा( pRhs!=0 || db->mallocFailed );
      यदि( pRhs ){
        adjust_क्रमः_By_Col(pSelect->p_क्रमः_By, pRhs);
        adjust_क्रमः_By_Col(pSelect->p_गणः_By, pRhs);
        प्रति(i=0; i<pRhs->n_व्यञ्जकम्; i++) pRhs->a[i].u.x.i_क्रमः_By_Col = 0;
      }

#यदि 0
      प्रारूप_लिख("For indexing, change the IN expr:\n");
      sqlite3_वृक्षः_View_व्यञ्जकम्(0, pX, 0);
      प्रारूप_लिख("Into:\n");
      sqlite3_वृक्षः_View_व्यञ्जकम्(0, pNew, 0);
#यदि_अन्तः
    }
  }
  प्रत्यागम pNew;
}


#यदि_अपरिभाषितम् SQLITE_OMIT_SUBQUERY
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
स्थिर SQLITE_NOINLINE शून्य संहिता_INTerm(
  Parse *pParse,      /* The parsing context */
  Where_पदांशः *p_पदांशः,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  पूर्णाङ्क iEq,            /* Index of the equality term within this level */
  पूर्णाङ्क bRev,           /* True for reverse-order IN operations */
  पूर्णाङ्क iTarget         /* Attempt to leave results in this register */
){
  व्यञ्जकम् *pX = p_पदांशः->p_व्यञ्जकम्;
  पूर्णाङ्क eType = अन्तर्_सूचकांक_NOOP;
  पूर्णाङ्क iTab;
  संरचना अन्तर्_Loop *p_अन्तर्;
  WhereLoop *pLoop = pLevel->pWLoop;
  Vdbe *v = pParse->pVdbe;
  पूर्णाङ्क i;
  पूर्णाङ्क nEq = 0;
  पूर्णाङ्क *aiMap = 0;

  यदि( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
    && pLoop->u.btree.p_सूचकांक!=0
    && pLoop->u.btree.p_सूचकांक->a_क्रमय_क्रमः[iEq]
  ){
    testcase( iEq==0 );
    testcase( bRev );
    bRev = !bRev;
  }
  प्रतिज्ञा( pX->op==TK_अन्तर् );

  प्रति(i=0; i<iEq; i++){
    यदि( pLoop->aLTerm[i] && pLoop->aLTerm[i]->p_व्यञ्जकम्==pX ){
      disable_पदांशः(pLevel, p_पदांशः);
      प्रत्यागम;
    }
  }
  प्रति(i=iEq; i<pLoop->nLTerm; i++){
    प्रतिज्ञा( pLoop->aLTerm[i]!=0 );
    यदि( pLoop->aLTerm[i]->p_व्यञ्जकम्==pX ) nEq++;
  }

  iTab = 0;
  यदि( !व्यञ्जकम्_Use_XSelect(pX) || pX->x.pSelect->pEList->n_व्यञ्जकम्==1 ){
    eType = sqlite3_Find_अन्तर्_सूचकांक(pParse, pX, अन्तर्_सूचकांक_LOOP, 0, 0, &iTab);
  }अन्यथा{
    sqlite3 *db = pParse->db;
    व्यञ्जकम् *pXMod = remove_Unindexable_अन्तर्_Clause_Terms(pParse, iEq, pLoop, pX);
    यदि( nEq>1 ){
      /* If this IN(SELECT ...) expression drives more than one column of
      ** the index, disable the seek-scan optimization. The reasons for this
      ** are that (a) it is only possible to make this happen by populating
      ** the sqlite_stat1 table with inconsistent information, and (b) it
      ** would require sqlite3FindInIndex() to find an index that is not
      ** only unique for the columns in question, but also delivers them
      ** in sorted order (requires checking asc/desc, and rejecting cases
      ** where the indexed columns are not in the right order).  */
      pLoop->wsFlags &= ~WHERE_अन्तर्_SEEKSCAN;
    }
    यदि( !db->mallocFailed ){
      पूर्णाङ्क nCol = pX->x.pSelect->pEList->n_व्यञ्जकम्;
      aiMap = (पूर्णाङ्क*)sqlite3DbMallocZero(db, परिमाण(पूर्णाङ्क)*nCol);
      eType = sqlite3_Find_अन्तर्_सूचकांक(pParse, pXMod, अन्तर्_सूचकांक_LOOP, 0, aiMap, &iTab);
    }
    sqlite3_व्यञ्जकम्_Delete(db, pXMod);
  }

  यदि( eType==अन्तर्_सूचकांक_सूचकांक_DESC ){
    testcase( bRev );
    bRev = !bRev;
  }
  sqlite3_Vdbe_सङ्कलय_Op2(v, bRev ? OP_चरमः : OP_Rewind, iTab, 0);
  VdbeCoverageIf(v, bRev);
  VdbeCoverageIf(v, !bRev);

  प्रतिज्ञा( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );
  pLoop->wsFlags |= WHERE_अन्तर्_ABLE;
  यदि( pLevel->u.अन्तर्.n_अन्तर्==0 ){
    pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
  }
  यदि( iEq>0 && (pLoop->wsFlags & WHERE_अन्तर्_SEEKSCAN)==0 ){
    pLoop->wsFlags |= WHERE_अन्तर्_EARLYOUT;
  }

  i = pLevel->u.अन्तर्.n_अन्तर्;
  pLevel->u.अन्तर्.n_अन्तर् += nEq;
  pLevel->u.अन्तर्.a_अन्तर्_Loop =
     sqlite3WhereRealloc(p_पदांशः->pWC->pWInfo,
                         pLevel->u.अन्तर्.a_अन्तर्_Loop,
                         परिमाण(pLevel->u.अन्तर्.a_अन्तर्_Loop[0])*pLevel->u.अन्तर्.n_अन्तर्);
  p_अन्तर् = pLevel->u.अन्तर्.a_अन्तर्_Loop;
  यदि( p_अन्तर् ){
    पूर्णाङ्क iMap = 0;               /* Index in aiMap[] */
    p_अन्तर् += i;
    प्रति(i=iEq; i<pLoop->nLTerm; i++){
      यदि( pLoop->aLTerm[i]->p_व्यञ्जकम्==pX ){
        पूर्णाङ्क i_बहिर्गमनम् = iTarget + i - iEq;
        यदि( eType==अन्तर्_सूचकांक_ROWID ){
          p_अन्तर्->addr_अन्तर्_शिखरम् = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rowid, iTab, i_बहिर्गमनम्);
        }अन्यथा{
          पूर्णाङ्क iCol = aiMap ? aiMap[iMap++] : 0;
          p_अन्तर्->addr_अन्तर्_शिखरम् = sqlite3_Vdbe_सङ्कलय_Op3(v,OP_Column,iTab, iCol, i_बहिर्गमनम्);
        }
        sqlite3_Vdbe_सङ्कलय_Op1(v, OP_IsNull, i_बहिर्गमनम्); VdbeCoverage(v);
        यदि( i==iEq ){
          p_अन्तर्->iCur = iTab;
          p_अन्तर्->e_अन्तः_Loop_Op = bRev ? OP_पूर्व : OP_अग्र;
          यदि( iEq>0 ){
            p_अन्तर्->i_आधारः = iTarget - i;
            p_अन्तर्->n_उपसर्गः = i;
          }अन्यथा{
            p_अन्तर्->n_उपसर्गः = 0;
          }
        }अन्यथा{
          p_अन्तर्->e_अन्तः_Loop_Op = OP_Noop;
        }
        p_अन्तर्++;
      }
    }
    testcase( iEq>0
              && (pLoop->wsFlags & WHERE_अन्तर्_SEEKSCAN)==0
              && (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 );
    यदि( iEq>0
     && (pLoop->wsFlags & (WHERE_अन्तर्_SEEKSCAN|WHERE_VIRTUALTABLE))==0
    ){
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Seek_लब्धम्, pLevel->iIdxCur, 0, iEq);
    }
  }अन्यथा{
    pLevel->u.अन्तर्.n_अन्तर् = 0;
  }
  sqlite3DbFree(pParse->db, aiMap);
}
#यदि_अन्तः


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
स्थिर पूर्णाङ्क संहिता_Equality_पदांशः(
  Parse *pParse,      /* The parsing context */
  Where_पदांशः *p_पदांशः,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  पूर्णाङ्क iEq,            /* Index of the equality term within this level */
  पूर्णाङ्क bRev,           /* True for reverse-order IN operations */
  पूर्णाङ्क iTarget         /* Attempt to leave results in this register */
){
  व्यञ्जकम् *pX = p_पदांशः->p_व्यञ्जकम्;
  पूर्णाङ्क iReg;                  /* Register holding results */

  प्रतिज्ञा( pLevel->pWLoop->aLTerm[iEq]==p_पदांशः );
  प्रतिज्ञा( iTarget>0 );
  यदि( pX->op==TK_EQ || pX->op==TK_IS ){
    iReg = sqlite3_व्यञ्जकम्_संहिता_Target(pParse, pX->p_दक्षिणः, iTarget);
  }अन्यथा यदि( pX->op==TK_ISNULL ){
    iReg = iTarget;
    sqlite3_Vdbe_सङ्कलय_Op2(pParse->pVdbe, OP_Null, 0, iReg);
#यदि_अपरिभाषितम् SQLITE_OMIT_SUBQUERY
  }अन्यथा{
    प्रतिज्ञा( pX->op==TK_अन्तर् );
    iReg = iTarget;
    संहिता_INTerm(pParse, p_पदांशः, pLevel, iEq, bRev, iTarget);
#यदि_अन्तः
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
  यदि( (pLevel->pWLoop->wsFlags & WHERE_TRANSCONS)==0
   || (p_पदांशः->eOperator & WO_EQUIV)==0
  ){
    disable_पदांशः(pLevel, p_पदांशः);
  }

  प्रत्यागम iReg;
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
स्थिर पूर्णाङ्क संहिता_All_Equality_Terms(
  Parse *pParse,        /* Parsing context */
  WhereLevel *pLevel,   /* Which nested loop of the FROM we are coding */
  पूर्णाङ्क bRev,             /* Reverse the order of IN operators */
  पूर्णाङ्क nExtraReg,        /* Number of extra registers to allocate */
  वर्ण **pzAff          /* OUT: Set to point to affinity string */
){
  u16 nEq;                      /* The number of == or IN constraints to code */
  u16 nSkip;                    /* Number of left-most columns to skip */
  Vdbe *v = pParse->pVdbe;      /* The vm under construction */
  सूचकांक *pIdx;                  /* The index being used for this loop */
  Where_पदांशः *p_पदांशः;             /* A single constraint term */
  WhereLoop *pLoop;             /* The WhereLoop object */
  पूर्णाङ्क j;                        /* Loop counter */
  पूर्णाङ्क reg_आधारः;                  /* Base register */
  पूर्णाङ्क nReg;                     /* Number of registers to allocate */
  वर्ण *zAff;                   /* Affinity string to return */

  /* This module is only called on query plans that use an index. */
  pLoop = pLevel->pWLoop;
  प्रतिज्ञा( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0 );
  nEq = pLoop->u.btree.nEq;
  nSkip = pLoop->nSkip;
  pIdx = pLoop->u.btree.p_सूचकांक;
  प्रतिज्ञा( pIdx!=0 );

  /* Figure out how many memory cells we will need then allocate them.
  */
  reg_आधारः = pParse->nMem + 1;
  nReg = nEq + nExtraReg;
  pParse->nMem += nReg;

  zAff = sqlite3DbStrDup(pParse->db,sqlite3_सूचकांक_Affinity_Str(pParse->db,pIdx));
  प्रतिज्ञा( zAff!=0 || pParse->db->mallocFailed );

  यदि( nSkip ){
    पूर्णाङ्क iIdxCur = pLevel->iIdxCur;
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Null, 0, reg_आधारः, reg_आधारः+nSkip-1);
    sqlite3_Vdbe_सङ्कलय_Op1(v, (bRev?OP_चरमः:OP_Rewind), iIdxCur);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    VdbeComment((v, "begin skip-scan on %s", pIdx->zName));
    j = sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Goto);
    प्रतिज्ञा( pLevel->addrSkip==0 );
    pLevel->addrSkip = sqlite3_Vdbe_सङ्कलय_Op4_Int(v, (bRev?OP_SeekLT:OP_SeekGT),
                            iIdxCur, 0, reg_आधारः, nSkip);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    sqlite3_Vdbe_Jump_अत्र(v, j);
    प्रति(j=0; j<nSkip; j++){
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, iIdxCur, j, reg_आधारः+j);
      testcase( pIdx->aiColumn[j]==XN_व्यञ्जकम् );
      VdbeComment((v, "%s", explain_सूचकांक_Column_Name(pIdx, j)));
    }
  }

  /* Evaluate the equality constraints
  */
  प्रतिज्ञा( zAff==0 || (पूर्णाङ्क)वाक्यदैर्घ्यम्(zAff)>=nEq );
  प्रति(j=nSkip; j<nEq; j++){
    पूर्णाङ्क r1;
    p_पदांशः = pLoop->aLTerm[j];
    प्रतिज्ञा( p_पदांशः!=0 );
    /* The following testcase is true for indices with redundant columns.
    ** Ex: CREATE INDEX i1 ON t1(a,b,a); SELECT * FROM t1 WHERE a=0 AND b=0; */
    testcase( (p_पदांशः->wtFlags & पदांशः_CODED)!=0 );
    testcase( p_पदांशः->wtFlags & पदांशः_VIRTUAL );
    r1 = संहिता_Equality_पदांशः(pParse, p_पदांशः, pLevel, j, bRev, reg_आधारः+j);
    यदि( r1!=reg_आधारः+j ){
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Copy, r1, reg_आधारः+j);
    }
    यदि( p_पदांशः->eOperator & WO_अन्तर् ){
      यदि( p_पदांशः->p_व्यञ्जकम्->flags & EP_xIsSelect ){
        /* No affinity ever needs to be (or should be) applied to a value
        ** from the RHS of an "? IN (SELECT ...)" expression. The
        ** sqlite3FindInIndex() routine has already ensured that the
        ** affinity of the comparison has been applied to the value.  */
        यदि( zAff ) zAff[j] = SQLITE_AFF_BLOB;
      }
    }अन्यथा यदि( (p_पदांशः->eOperator & WO_ISNULL)==0 ){
      व्यञ्जकम् *p_दक्षिणः = p_पदांशः->p_व्यञ्जकम्->p_दक्षिणः;
      यदि( (p_पदांशः->wtFlags & पदांशः_IS)==0 && sqlite3_व्यञ्जकम्_Can_Be_Null(p_दक्षिणः) ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IsNull, reg_आधारः+j, pLevel->addrBrk);
        VdbeCoverage(v);
      }
      यदि( pParse->nErr==0 ){
        प्रतिज्ञा( pParse->db->mallocFailed==0 );
        यदि( sqlite3CompareAffinity(p_दक्षिणः, zAff[j])==SQLITE_AFF_BLOB ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
        यदि( sqlite3_व्यञ्जकम्_Needs_No_Affinity_Change(p_दक्षिणः, zAff[j]) ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
      }
    }
  }
  *pzAff = zAff;
  प्रत्यागम reg_आधारः;
}

#यदि_अपरिभाषितम् SQLITE_LIKE_DOESNT_मेलनम्_BLOBS
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
स्थिर शून्य whereLikeOptimizationStringFixup(
  Vdbe *v,                /* prepared statement under construction */
  WhereLevel *pLevel,     /* The loop that contains the LIKE operator */
  Where_पदांशः *p_पदांशः        /* The upper or lower bound just coded */
){
  यदि( p_पदांशः->wtFlags & पदांशः_LIKEOPT ){
    VdbeOp *pOp;
    प्रतिज्ञा( pLevel->iLikeRepCntr>0 );
    pOp = sqlite3_Vdbe_Get_चरमः_Op(v);
    प्रतिज्ञा( pOp!=0 );
    प्रतिज्ञा( pOp->opcode==OP_String8
            || p_पदांशः->pWC->pWInfo->pParse->db->mallocFailed );
    pOp->p3 = (पूर्णाङ्क)(pLevel->iLikeRepCntr>>1);  /* Register holding counter */
    pOp->p5 = (u8)(pLevel->iLikeRepCntr&1);    /* ASC or DESC */
  }
}
#अन्यथा
# define whereLikeOptimizationStringFixup(A,B,C)
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_ENABLE_CURSOR_HINTS
/*
** Information is passed from codeCursorHint() down to individual nodes of
** the expression tree (by sqlite3WalkExpr()) using an instance of this
** structure.
*/
संरचना CCurHint {
  पूर्णाङ्क iTabCur;    /* Cursor for the main table */
  पूर्णाङ्क iIdxCur;    /* Cursor for the index, if pIdx!=0.  Unused otherwise */
  सूचकांक *pIdx;    /* The index used to access the table */
};

/*
** This function is called for every node of an expression that is a candidate
** for a cursor hint on an index cursor.  For TK_COLUMN nodes that reference
** the table CCurHint.iTabCur, verify that the same column can be
** accessed through the index.  If it cannot, then set pWalker->eCode to 1.
*/
स्थिर पूर्णाङ्क संहिता_Cursor_Hint_Check_व्यञ्जकम्(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  संरचना CCurHint *pHint = pWalker->u.pCCurHint;
  प्रतिज्ञा( pHint->pIdx!=0 );
  यदि( p_व्यञ्जकम्->op==TK_COLUMN
   && p_व्यञ्जकम्->i_सारणी==pHint->iTabCur
   && sqlite3_सारणी_Column_गन्तव्यम्_सूचकांक(pHint->pIdx, p_व्यञ्जकम्->iColumn)<0
  ){
    pWalker->e_संहिता = 1;
  }
  प्रत्यागम WRC_Continue;
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
स्थिर पूर्णाङ्क संहिता_Cursor_Hint_Is_Or_Function(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  यदि( p_व्यञ्जकम्->op==TK_IS
   || p_व्यञ्जकम्->op==TK_ISNULL || p_व्यञ्जकम्->op==TK_ISNOT
   || p_व्यञ्जकम्->op==TK_NOTNULL || p_व्यञ्जकम्->op==TK_CASE
  ){
    pWalker->e_संहिता = 1;
  }अन्यथा यदि( p_व्यञ्जकम्->op==TK_FUNCTION ){
    पूर्णाङ्क d1;
    वर्ण d2[4];
    यदि( 0==sqlite3IsLikeFunction(pWalker->pParse->db, p_व्यञ्जकम्, &d1, d2) ){
      pWalker->e_संहिता = 1;
    }
  }

  प्रत्यागम WRC_Continue;
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
स्थिर पूर्णाङ्क संहिता_Cursor_Hint_Fix_व्यञ्जकम्(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  पूर्णाङ्क rc = WRC_Continue;
  पूर्णाङ्क reg;
  संरचना CCurHint *pHint = pWalker->u.pCCurHint;
  यदि( p_व्यञ्जकम्->op==TK_COLUMN ){
    यदि( p_व्यञ्जकम्->i_सारणी!=pHint->iTabCur ){
      reg = ++pWalker->pParse->nMem;   /* Register for column value */
      reg = sqlite3_व्यञ्जकम्_संहिता_Target(pWalker->pParse, p_व्यञ्जकम्, reg);
      p_व्यञ्जकम्->op = TK_पञ्जिकाकोशः;
      p_व्यञ्जकम्->i_सारणी = reg;
    }अन्यथा यदि( pHint->pIdx!=0 ){
      p_व्यञ्जकम्->i_सारणी = pHint->iIdxCur;
      p_व्यञ्जकम्->iColumn = sqlite3_सारणी_Column_गन्तव्यम्_सूचकांक(pHint->pIdx, p_व्यञ्जकम्->iColumn);
      प्रतिज्ञा( p_व्यञ्जकम्->iColumn>=0 );
    }
  }अन्यथा यदि( p_व्यञ्जकम्->pAggInfo ){
    rc = WRC_Prune;
    reg = ++pWalker->pParse->nMem;   /* Register for column value */
    reg = sqlite3_व्यञ्जकम्_संहिता_Target(pWalker->pParse, p_व्यञ्जकम्, reg);
    p_व्यञ्जकम्->op = TK_पञ्जिकाकोशः;
    p_व्यञ्जकम्->i_सारणी = reg;
  }अन्यथा यदि( p_व्यञ्जकम्->op==TK_TRUEFALSE ){
    /* Do not walk disabled expressions.  tag-20230504-1 */
    प्रत्यागम WRC_Prune;
  }
  प्रत्यागम rc;
}

/*
** Insert an OP_CursorHint instruction if it is appropriate to do so.
*/
स्थिर शून्य संहिता_Cursor_Hint(
  SrcItem *pTabItem,  /* FROM clause item */
  WhereInfo *pWInfo,    /* The where clause */
  WhereLevel *pLevel,   /* Which loop to provide hints for */
  Where_पदांशः *p_अन्तः_Range  /* Hint this end-of-scan boundary term if not NULL */
){
  Parse *pParse = pWInfo->pParse;
  sqlite3 *db = pParse->db;
  Vdbe *v = pParse->pVdbe;
  व्यञ्जकम् *p_व्यञ्जकम् = 0;
  WhereLoop *pLoop = pLevel->pWLoop;
  पूर्णाङ्क iCur;
  WhereClause *pWC;
  Where_पदांशः *p_पदांशः;
  पूर्णाङ्क i, j;
  संरचना CCurHint sHint;
  Walker sWalker;

  यदि( OptimizationDisabled(db, SQLITE_CursorHints) ) प्रत्यागम;
  iCur = pLevel->iTabCur;
  प्रतिज्ञा( iCur==pWInfo->pTabList->a[pLevel->iFrom].iCursor );
  sHint.iTabCur = iCur;
  sHint.iIdxCur = pLevel->iIdxCur;
  sHint.pIdx = pLoop->u.btree.p_सूचकांक;
  स्मृतिं_पूरय(&sWalker, 0, परिमाण(sWalker));
  sWalker.pParse = pParse;
  sWalker.u.pCCurHint = &sHint;
  pWC = &pWInfo->sWC;
  प्रति(i=0; i<pWC->n_आधारः; i++){
    p_पदांशः = &pWC->a[i];
    यदि( p_पदांशः->wtFlags & (पदांशः_VIRTUAL|पदांशः_CODED) ) अनुवर्त;
    यदि( p_पदांशः->prereqAll & pLevel->notReady ) अनुवर्त;

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
    यदि( pTabItem->fg.jointype & JT_वामः ){
      व्यञ्जकम् *p_व्यञ्जकम् = p_पदांशः->p_व्यञ्जकम्;
      यदि( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON)
       || p_व्यञ्जकम्->w.iJoin!=pTabItem->iCursor
      ){
        sWalker.e_संहिता = 0;
        sWalker.x_व्यञ्जकम्_Callback = संहिता_Cursor_Hint_Is_Or_Function;
        sqlite3_Walk_व्यञ्जकम्(&sWalker, p_पदांशः->p_व्यञ्जकम्);
        यदि( sWalker.e_संहिता ) अनुवर्त;
      }
    }अन्यथा{
      यदि( व्यञ्जकम्_Has_Property(p_पदांशः->p_व्यञ्जकम्, EP_OuterON) ) अनुवर्त;
    }

    /* All terms in pWLoop->aLTerm[] except pEndRange are used to initialize
    ** the cursor.  These terms are not needed as hints for a pure range
    ** scan (that has no == terms) so omit them. */
    यदि( pLoop->u.btree.nEq==0 && p_पदांशः!=p_अन्तः_Range ){
      प्रति(j=0; j<pLoop->nLTerm && pLoop->aLTerm[j]!=p_पदांशः; j++){}
      यदि( j<pLoop->nLTerm ) अनुवर्त;
    }

    /* No subqueries or non-deterministic functions allowed */
    यदि( sqlite3_व्यञ्जकम्_अन्तर्भवति_Subquery(p_पदांशः->p_व्यञ्जकम्) ) अनुवर्त;

    /* For an index scan, make sure referenced columns are actually in
    ** the index. */
    यदि( sHint.pIdx!=0 ){
      sWalker.e_संहिता = 0;
      sWalker.x_व्यञ्जकम्_Callback = संहिता_Cursor_Hint_Check_व्यञ्जकम्;
      sqlite3_Walk_व्यञ्जकम्(&sWalker, p_पदांशः->p_व्यञ्जकम्);
      यदि( sWalker.e_संहिता ) अनुवर्त;
    }

    /* If we survive all prior tests, that means this term is worth hinting */
    p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_And(pParse, p_व्यञ्जकम्, sqlite3_व्यञ्जकम्_Dup(db, p_पदांशः->p_व्यञ्जकम्, 0));
  }
  यदि( p_व्यञ्जकम्!=0 ){
    sWalker.x_व्यञ्जकम्_Callback = संहिता_Cursor_Hint_Fix_व्यञ्जकम्;
    यदि( pParse->nErr==0 ) sqlite3_Walk_व्यञ्जकम्(&sWalker, p_व्यञ्जकम्);
    sqlite3_Vdbe_सङ्कलय_Op4(v, OP_CursorHint,
                      (sHint.pIdx ? sHint.iIdxCur : sHint.iTabCur), 0, 0,
                      (ध्रुव वर्ण*)p_व्यञ्जकम्, P4_व्यञ्जकम्);
  }
}
#अन्यथा
# define संहिता_Cursor_Hint(A,B,C,D)  /* No-op */
#यदि_अन्तः /* SQLITE_ENABLE_CURSOR_HINTS */

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
स्थिर शून्य संहिता_Deferred_Seek(
  WhereInfo *pWInfo,              /* Where clause context */
  सूचकांक *pIdx,                    /* Index scan is using */
  पूर्णाङ्क iCur,                       /* Cursor for IPK b-tree */
  पूर्णाङ्क iIdxCur                     /* Index cursor */
){
  Parse *pParse = pWInfo->pParse; /* Parse context */
  Vdbe *v = pParse->pVdbe;        /* Vdbe to generate code within */

  प्रतिज्ञा( iIdxCur>0 );
  प्रतिज्ञा( pIdx->aiColumn[pIdx->nColumn-1]==-1 );

  pWInfo->bDeferredSeek = 1;
  sqlite3_Vdbe_सङ्कलय_Op3(v, OP_DeferredSeek, iIdxCur, 0, iCur);
  यदि( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_दक्षिणः_JOIN))
   && Db_आवरणम्_All_Zero(sqlite3ParseToplevel(pParse)->write_आवरणम्)
  ){
    पूर्णाङ्क i;
    सारणी *pTab = pIdx->p_सारणी;
    u32 *ai = (u32*)sqlite3DbMallocZero(pParse->db, परिमाण(u32)*(pTab->nCol+1));
    यदि( ai ){
      ai[0] = pTab->nCol;
      प्रति(i=0; i<pIdx->nColumn-1; i++){
        पूर्णाङ्क x1, x2;
        प्रतिज्ञा( pIdx->aiColumn[i]<pTab->nCol );
        x1 = pIdx->aiColumn[i];
        x2 = sqlite3_सारणी_Column_गन्तव्यम्_Storage(pTab, x1);
        testcase( x1!=x2 );
        यदि( x1>=0 ) ai[x2+1] = i+1;
      }
      sqlite3VdbeChangeP4(v, -1, (वर्ण*)ai, P4_INTARRAY);
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
स्थिर शून्य संहिता_व्यञ्जकम्_Or_सदिशम्(Parse *pParse, व्यञ्जकम् *p, पूर्णाङ्क iReg, पूर्णाङ्क nReg){
  प्रतिज्ञा( nReg>0 );
  यदि( p && sqlite3_व्यञ्जकम्_Is_सदिशम्(p) ){
#यदि_अपरिभाषितम् SQLITE_OMIT_SUBQUERY
    यदि( व्यञ्जकम्_Use_XSelect(p) ){
      Vdbe *v = pParse->pVdbe;
      पूर्णाङ्क iSelect;
      प्रतिज्ञा( p->op==TK_SELECT );
      iSelect = sqlite3_संहिता_Subselect(pParse, p);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Copy, iSelect, iReg, nReg-1);
    }अन्यथा
#यदि_अन्तः
    {
      पूर्णाङ्क i;
      ध्रुव व्यञ्जकम्_List *pList;
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p) );
      pList = p->x.pList;
      प्रतिज्ञा( nReg<=pList->n_व्यञ्जकम् );
      प्रति(i=0; i<nReg; i++){
        sqlite3_व्यञ्जकम्_संहिता(pParse, pList->a[i].p_व्यञ्जकम्, iReg+i);
      }
    }
  }अन्यथा{
    प्रतिज्ञा( nReg==1 || pParse->nErr );
    sqlite3_व्यञ्जकम्_संहिता(pParse, p, iReg);
  }
}

/*
** The pTruth expression is always true because it is the WHERE clause
** a partial index that is driving a query loop.  Look through all of the
** WHERE clause terms on the query, and if any of those terms must be
** true because pTruth is true, then mark those WHERE clause terms as
** coded.
*/
स्थिर शून्य where_Apply_Partial_सूचकांक_Constraints(
  व्यञ्जकम् *pTruth,
  पूर्णाङ्क iTabCur,
  WhereClause *pWC
){
  पूर्णाङ्क i;
  Where_पदांशः *p_पदांशः;
  यावत्( pTruth->op==TK_AND ){
    where_Apply_Partial_सूचकांक_Constraints(pTruth->p_वामः, iTabCur, pWC);
    pTruth = pTruth->p_दक्षिणः;
  }
  प्रति(i=0, p_पदांशः=pWC->a; i<pWC->n_पदांशः; i++, p_पदांशः++){
    व्यञ्जकम् *p_व्यञ्जकम्;
    यदि( p_पदांशः->wtFlags & पदांशः_CODED ) अनुवर्त;
    p_व्यञ्जकम् = p_पदांशः->p_व्यञ्जकम्;
    यदि( sqlite3_व्यञ्जकम्_Compare(0, p_व्यञ्जकम्, pTruth, iTabCur)==0 ){
      p_पदांशः->wtFlags |= पदांशः_CODED;
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
स्थिर SQLITE_NOINLINE शून्य filter_Pull_अधस्तात्(
  Parse *pParse,       /* Parsing context */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  पूर्णाङ्क iLevel,          /* Which level of pWInfo->a[] should be coded */
  पूर्णाङ्क addrNxt,         /* Jump here to bypass inner loops */
  Bitmask notReady     /* Loops that are not ready */
){
  पूर्णाङ्क saved_addrBrk;
  यावत्( ++iLevel < pWInfo->nLevel ){
    WhereLevel *pLevel = &pWInfo->a[iLevel];
    WhereLoop *pLoop = pLevel->pWLoop;
    यदि( pLevel->regFilter==0 ) अनुवर्त;
    यदि( pLevel->pWLoop->nSkip ) अनुवर्त;
    /*         ,--- Because sqlite3ConstructBloomFilter() has will not have set
    **  vvvvv--'    pLevel->regFilter if this were true. */
    यदि( NEVER(pLoop->prereq & notReady) ) अनुवर्त;
    saved_addrBrk = pLevel->addrBrk;
    pLevel->addrBrk = addrNxt;
    यदि( pLoop->wsFlags & WHERE_IPK ){
      Where_पदांशः *p_पदांशः = pLoop->aLTerm[0];
      पूर्णाङ्क regRowid;
      प्रतिज्ञा( p_पदांशः!=0 );
      प्रतिज्ञा( p_पदांशः->p_व्यञ्जकम्!=0 );
      testcase( p_पदांशः->wtFlags & पदांशः_VIRTUAL );
      regRowid = sqlite3_Get_अस्थायी_Reg(pParse);
      regRowid = संहिता_Equality_पदांशः(pParse, p_पदांशः, pLevel, 0, 0, regRowid);
      sqlite3_Vdbe_सङ्कलय_Op2(pParse->pVdbe, OP_MustBeInt, regRowid, addrNxt);
      VdbeCoverage(pParse->pVdbe);
      प्रतिज्ञा( sqlite3WhereLoopBloomable(pLoop) );
      sqlite3_Vdbe_सङ्कलय_Op4_Int(pParse->pVdbe, OP_Filter, pLevel->regFilter,
                           addrNxt, regRowid, 1);
      VdbeCoverage(pParse->pVdbe);
    }अन्यथा{
      u16 nEq = pLoop->u.btree.nEq;
      पूर्णाङ्क r1;
      वर्ण *zStartAff;

      प्रतिज्ञा( pLoop->wsFlags & WHERE_अनुक्रमितम् );
      प्रतिज्ञा( (pLoop->wsFlags & WHERE_COLUMN_अन्तर्)==0 );
      r1 = संहिता_All_Equality_Terms(pParse,pLevel,0,0,&zStartAff);
      संहिता_Apply_Affinity(pParse, r1, nEq, zStartAff);
      sqlite3DbFree(pParse->db, zStartAff);
      प्रतिज्ञा( sqlite3WhereLoopBloomable(pLoop) );
      sqlite3_Vdbe_सङ्कलय_Op4_Int(pParse->pVdbe, OP_Filter, pLevel->regFilter,
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
स्थिर पूर्णाङ्क whereLoopIsOneRow(WhereLoop *pLoop){
  यदि( pLoop->u.btree.p_सूचकांक->onError
   && pLoop->nSkip==0
   && pLoop->u.btree.nEq==pLoop->u.btree.p_सूचकांक->n_कुञ्चिका_Col
  ){
    पूर्णाङ्क ii;
    प्रति(ii=0; ii<pLoop->u.btree.nEq; ii++){
      यदि( pLoop->aLTerm[ii]->eOperator & (WO_IS|WO_ISNULL) ){
        प्रत्यागम 0;
      }
    }
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
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
स्थिर पूर्णाङ्क where_व्यञ्जकम्_Is_Ready(
  WhereInfo *pWInfo, 
  WhereLevel *pLevel,
  SrcItem *pTabItem, 
  व्यञ्जकम् *pE
){
  u8 jtype = pTabItem->fg.jointype;
  यदि( jtype & (JT_वामः|JT_LTORJ|JT_दक्षिणः) ){
    यदि( !व्यञ्जकम्_Has_Property(pE,EP_OuterON|EP_InnerON) ){
      /* Defer processing WHERE clause constraints until after outer
      ** join processing.  tag-20220513a */
      प्रत्यागम 0;
    }अन्यथा यदि( (jtype & JT_वामः) && !व्यञ्जकम्_Has_Property(pE,EP_OuterON) ){
      प्रत्यागम 0;
    }अन्यथा{
      Bitmask m = sqlite3_Where_Get_आवरणम्(&pWInfo->s_आवरणम्_समुच्चयः, pE->w.iJoin);
      यदि( m & pLevel->notReady ){
        /* An ON clause that is not ripe */
        प्रत्यागम 0;
      }
    }
  }
  प्रत्यागम 1;
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
स्थिर पूर्णाङ्क find_No_मेलनम्_Continuation(
  WhereInfo *pWInfo,         /* WHERE clause */
  पूर्णाङ्क iLevel                 /* Which loop we are talking about */
){
  WhereLoop *pLoop;          /* Description of this loop */
  WhereLevel *pParent;       /* Controlling parent loop */
  SrcList *pTabList;         /* Complete FROM clause */
  Bitmask m_कुञ्चिका;              /* Mask of dependent tables */
  पूर्णाङ्क iParent;               /* Inner-most loop affecting the key to iLevel */
  पूर्णाङ्क i;                     /* Loop counter */

  यदि( iLevel<2 ){
    /* If there are less than two outer loops, this optimization is no help */
    प्रत्यागम 0;
  }

  /* The OP_SeekGE and OP_SeekLE opcodes only work right for this
  ** optimization on a pure equality constraint, so skip the optimization
  ** for anything else.
  */
  pLoop = pWInfo->a[iLevel].pWLoop;
  यदि( (pLoop->wsFlags & WHERE_CONSTRAINT)!=WHERE_COLUMN_EQ ) प्रत्यागम 0;
  यदि( pLoop->wsFlags & WHERE_SKIPSCAN ) प्रत्यागम 0;
  यदि( pLoop->wsFlags & WHERE_BIGNULL_क्रमय ) प्रत्यागम 0;
  प्रतिज्ञा( (pLoop->wsFlags & (WHERE_VIRTUALTABLE|WHERE_MULTI_OR))==0 );
  प्रतिज्ञा( pLoop->u.btree.nEq>0 );

  /* Find a mask of all outer tables that influence the key.  At the same
  ** time, check to see if any keys values are computed from subqueries and
  ** fail out (return 0) if there are any. */
  m_कुञ्चिका = 0;
  प्रति(i=0; i<pLoop->nLTerm; i++){
    यदि( sqlite3_व्यञ्जकम्_अन्तर्भवति_Subquery(pLoop->aLTerm[i]->p_व्यञ्जकम्) ){
      प्रत्यागम 0;
    }
    m_कुञ्चिका |= pLoop->aLTerm[i]->prereq_दक्षिणः;
  }

  /* Look back through outer loops.  Find the inner-most outer loop
  ** that influences the key. Make iParent be the index of that inner-most
  ** outer loop.
  */
  pTabList = pWInfo->pTabList;
  iParent = iLevel-1;
  यावत्( iParent>0 && (pWInfo->a[iParent].pWLoop->आवरणम्_Self & m_कुञ्चिका)==0 ){
    iParent--;
  }
  प्रतिज्ञा( iParent>=0 && iParent<=iLevel );

  /* Everything needs to be an INNER join.  Fail if there are any
  ** OUTER joins in between iParent and iLevel. */
  प्रति(i=iParent; i<=iLevel; i++){
    यदि( pTabList->a[pWInfo->a[i].iFrom].fg.jointype & JT_OUTER ) प्रत्यागम 0;
  }

  /* If we reach this point, that means the conditions of the optimization
  ** have been met.  Even so, disallow the optimization if it is disabled.
  */
  यदि( OptimizationDisabled(pWInfo->pParse->db, SQLITE_Join_अलब्धिः) ){
    प्रत्यागम 0;
  }

  /* If the inner-most outer loop that influences the key for iLevel
  ** is controlled by an automatic index and if the automatic index is
  ** the only outer loop that influences the key, then arrange to delete
  ** the automatic index entry if there is no match on the key.
  */
  pParent = &pWInfo->a[iParent];
  यदि( (pParent->pWLoop->wsFlags & WHERE_AUTO_सूचकांक)!=0
   && (m_कुञ्चिका & (m_कुञ्चिका-1))==0
  ){
    प्रति(i=0; i<pLoop->nLTerm; i++){
      व्यञ्जकम् *pX = pLoop->aLTerm[i]->p_व्यञ्जकम्;
      यदि( !sqlite3_व्यञ्जकम्_Is_सारणी_Constant(pX->p_दक्षिणः,pParent->iTabCur,0) ) भङ्ग;
    }
    यदि( i==pLoop->nLTerm ){    
      यदि( pParent->addrDeleteCont==0 ){
        pParent->addrDeleteCont = sqlite3VdbeMakeLabel(pWInfo->pParse);
      }
      प्रत्यागम pParent->addrDeleteCont;
    }
  }

  testcase( iParent<iLevel-1 );
  प्रत्यागम pParent->addrCont;
}

/*
** Generate code for the start of the iLevel-th loop in the WHERE clause
** implementation described by pWInfo.
*/
Bitmask sqlite3_Where_संहिता_One_Loop_Start(
  Parse *pParse,       /* Parsing context */
  Vdbe *v,             /* Prepared statement under construction */
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  पूर्णाङ्क iLevel,          /* Which level of pWInfo->a[] should be coded */
  WhereLevel *pLevel,  /* The current level pointer */
  Bitmask notReady     /* Which tables are currently available */
){
  पूर्णाङ्क j, k;            /* Loop counters */
  पूर्णाङ्क iCur;            /* The VDBE cursor for the table */
  पूर्णाङ्क addrNxt;         /* Where to jump to continue with the next IN case */
  पूर्णाङ्क bRev;            /* True if we need to scan in reverse order */
  WhereLoop *pLoop;    /* The WhereLoop object being coded */
  WhereClause *pWC;    /* Decomposition of the entire WHERE clause */
  Where_पदांशः *p_पदांशः;         /* A WHERE clause term */
  sqlite3 *db;              /* Database connection */
  SrcItem *pTabItem;        /* FROM clause term being coded */
  पूर्णाङ्क addrBrk;              /* Jump here to break out of the loop */
  पूर्णाङ्क addrCont;             /* Jump here to continue with next cycle */
  पूर्णाङ्क addr_अलब्धिः;             /* addrCont enhanced for no-matching rows */
  पूर्णाङ्क iRowidReg = 0;        /* Rowid is stored in this register, if not zero */
  पूर्णाङ्क iReleaseReg = 0;      /* Temp register to free before returning */
  सूचकांक *pIdx = 0;          /* Index used by loop (if any) */
  पूर्णाङ्क iLoop;                /* Iteration of constraint generator loop */

  pWC = &pWInfo->sWC;
  db = pParse->db;
  pLoop = pLevel->pWLoop;
  pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
  iCur = pTabItem->iCursor;
  pLevel->notReady = notReady & ~sqlite3_Where_Get_आवरणम्(&pWInfo->s_आवरणम्_समुच्चयः, iCur);
  bRev = (pWInfo->rev_आवरणम्>>iLevel)&1;
  VdbeModuleComment((v, "Begin WHERE-loop%d: %s",
                     iLevel, pTabItem->pSTab->zName));
#यदि WHERETRACE_ENABLED /* 0x4001 */
  यदि( sqlite3WhereTrace & 0x1 ){
    sqlite3DebugPrintf("Coding level %d of %d:  notReady=%llx  iFrom=%d\n",
       iLevel, pWInfo->nLevel, (u64)notReady, pLevel->iFrom);
    यदि( sqlite3WhereTrace & 0x1000 ){
      sqlite3WhereLoopPrint(pLoop, pWC);
    }
  }
  यदि( (sqlite3WhereTrace & 0x4001)==0x4001 ){
    यदि( iLevel==0 ){
      sqlite3DebugPrintf("WHERE clause being coded:\n");
      sqlite3_वृक्षः_View_व्यञ्जकम्(0, pWInfo->pWhere, 0);
    }
    sqlite3DebugPrintf("All WHERE-clause terms before coding:\n");
    sqlite3WhereClausePrint(pWC);
  }
#यदि_अन्तः

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
  addr_अलब्धिः = find_No_मेलनम्_Continuation(pWInfo, iLevel);

  /* If this is the right table of a LEFT OUTER JOIN, allocate and
  ** initialize a memory cell that records if this table matches any
  ** row of the left table of the join.
  */
  प्रतिज्ञा( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_दक्षिणः_JOIN))
       || pLevel->iFrom>0 || (pTabItem[0].fg.jointype & JT_वामः)==0
  );
  यदि( pLevel->iFrom>0 && (pTabItem[0].fg.jointype & JT_वामः)!=0 ){
    pLevel->i_वामः_Join = ++pParse->nMem;
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 0, pLevel->i_वामः_Join);
    VdbeComment((v, "init LEFT JOIN match flag"));
    यदि( pTabItem->fg.द्वारा_Coroutine ){
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_NullRow, pTabItem->iCursor, 1);
    }
  }

  /* Special case of a FROM clause subquery implemented as a co-routine */
  यदि( pTabItem->fg.द्वारा_Coroutine ){
    पूर्णाङ्क regYield;
    Subquery *pSubq;
    प्रतिज्ञा( pTabItem->fg.isSubquery && pTabItem->u4.pSubq!=0 );
    pSubq = pTabItem->u4.pSubq;
    regYield = pSubq->regReturn;
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_InitCoroutine, regYield, 0, pSubq->addrFillSub);
    pLevel->p2 =  sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Yield, regYield, addrBrk);
    VdbeCoverage(v);
    VdbeComment((v, "next row of %s", pTabItem->pSTab->zName));
    pLevel->op = OP_Goto;
  }अन्यथा

#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
  यदि(  (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
    /* Case 1:  The table is a virtual-table.  Use the VFilter and VNext
    **          to access the data.
    */
    पूर्णाङ्क iReg;   /* P3 Value for OP_VFilter */
    पूर्णाङ्क addr_Not_प्राप्तम्;
    पूर्णाङ्क nConstraint = pLoop->nLTerm;

    iReg = sqlite3_Get_अस्थायी_Range(pParse, nConstraint+2);
    addr_Not_प्राप्तम् = pLevel->addrBrk;
    प्रति(j=0; j<nConstraint; j++){
      पूर्णाङ्क iTarget = iReg+j+2;
      p_पदांशः = pLoop->aLTerm[j];
      यदि( NEVER(p_पदांशः==0) ) अनुवर्त;
      यदि( p_पदांशः->eOperator & WO_अन्तर् ){
        यदि( SMASKBIT32(j) & pLoop->u.vtab.m_Handle_अन्तर् ){
          पूर्णाङ्क iTab = pParse->nTab++;
          पूर्णाङ्क iCache = ++pParse->nMem;
          sqlite3_संहिता_Rhs_सम्बन्धि_अन्तर्(pParse, p_पदांशः->p_व्यञ्जकम्, iTab, 0);
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_VInit_अन्तर्, iTab, iTarget, iCache);
        }अन्यथा{
          संहिता_Equality_पदांशः(pParse, p_पदांशः, pLevel, j, bRev, iTarget);
          addr_Not_प्राप्तम् = pLevel->addrNxt;
        }
      }अन्यथा{
        व्यञ्जकम् *p_दक्षिणः = p_पदांशः->p_व्यञ्जकम्->p_दक्षिणः;
        संहिता_व्यञ्जकम्_Or_सदिशम्(pParse, p_दक्षिणः, iTarget, 1);
        यदि( p_पदांशः->e_मेलनम्_Op==SQLITE_सूचकांक_CONSTRAINT_विस्थापनम्
         && pLoop->u.vtab.b_Omit_विस्थापनम्
        ){
          प्रतिज्ञा( p_पदांशः->eOperator==WO_AUX );
          प्रतिज्ञा( pWInfo->pSelect!=0 );
          प्रतिज्ञा( pWInfo->pSelect->i_विस्थापनम्>0 );
          sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 0, pWInfo->pSelect->i_विस्थापनम्);
          VdbeComment((v,"Zero OFFSET counter"));
        }
      }
    }
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, pLoop->u.vtab.idx_अङ्कः, iReg);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, nConstraint, iReg+1);
    /* The instruction immediately prior to OP_VFilter must be an OP_Integer
    ** that sets the "argc" value for xVFilter.  This is necessary for
    ** resolveP2() to work correctly.  See tag-20250207a. */
    sqlite3_Vdbe_सङ्कलय_Op4(v, OP_VFilter, iCur, addr_Not_प्राप्तम्, iReg,
                      pLoop->u.vtab.idxStr,
                      pLoop->u.vtab.needFree ? P4_DYNAMIC : P4_STATIC);
    VdbeCoverage(v);
    pLoop->u.vtab.needFree = 0;
    /* An OOM inside of AddOp4(OP_VFilter) instruction above might have freed
    ** the u.vtab.idxStr.  NULL it out to prevent a use-after-free */
    यदि( db->mallocFailed ) pLoop->u.vtab.idxStr = 0;
    pLevel->p1 = iCur;
    pLevel->op = pWInfo->eOnePass ? OP_Noop : OP_VNext;
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);
    प्रतिज्ञा( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );

    प्रति(j=0; j<nConstraint; j++){
      p_पदांशः = pLoop->aLTerm[j];
      यदि( j<16 && (pLoop->u.vtab.omit_आवरणम्>>j)&1 ){
        disable_पदांशः(pLevel, p_पदांशः);
        अनुवर्त;
      }
      यदि( (p_पदांशः->eOperator & WO_अन्तर्)!=0
       && (SMASKBIT32(j) & pLoop->u.vtab.m_Handle_अन्तर्)==0
       && !db->mallocFailed
      ){
        व्यञ्जकम् *pCompare;  /* The comparison operator */
        व्यञ्जकम् *p_दक्षिणः;    /* RHS of the comparison */
        VdbeOp *pOp;     /* Opcode to access the value of the IN constraint */
        पूर्णाङ्क i_अन्तर्;         /* IN loop corresponding to the j-th constraint */

        /* Reload the constraint value into reg[iReg+j+2].  The same value
        ** was loaded into the same register prior to the OP_VFilter, but
        ** the xFilter implementation might have changed the datatype or
        ** encoding of the value in the register, so it *must* be reloaded.
        */
        प्रति(i_अन्तर्=0; ALWAYS(i_अन्तर्<pLevel->u.अन्तर्.n_अन्तर्); i_अन्तर्++){
          pOp = sqlite3VdbeGetOp(v, pLevel->u.अन्तर्.a_अन्तर्_Loop[i_अन्तर्].addr_अन्तर्_शिखरम्);
          यदि( (pOp->opcode==OP_Column && pOp->p3==iReg+j+2)
           || (pOp->opcode==OP_Rowid && pOp->p2==iReg+j+2)
          ){
            testcase( pOp->opcode==OP_Rowid );
            sqlite3_Vdbe_सङ्कलय_Op3(v, pOp->opcode, pOp->p1, pOp->p2, pOp->p3);
            भङ्ग;
          }
        }

        /* Generate code that will continue to the next row if
        ** the IN constraint is not satisfied
        */
        pCompare = sqlite3PExpr(pParse, TK_EQ, 0, 0);
        यदि( !db->mallocFailed ){
          पूर्णाङ्क iFld = p_पदांशः->u.x.i_क्षेत्रम्;
          व्यञ्जकम् *p_वामः = p_पदांशः->p_व्यञ्जकम्->p_वामः;
          प्रतिज्ञा( p_वामः!=0 );
          यदि( iFld>0 ){
            प्रतिज्ञा( p_वामः->op==TK_सदिशम् );
            प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_वामः) );
            प्रतिज्ञा( iFld<=p_वामः->x.pList->n_व्यञ्जकम् );
            pCompare->p_वामः = p_वामः->x.pList->a[iFld-1].p_व्यञ्जकम्;
          }अन्यथा{
            pCompare->p_वामः = p_वामः;
          }
          pCompare->p_दक्षिणः = p_दक्षिणः = sqlite3_व्यञ्जकम्(db, TK_पञ्जिकाकोशः, 0);
          यदि( p_दक्षिणः ){
            p_दक्षिणः->i_सारणी = iReg+j+2;
            sqlite3_व्यञ्जकम्_If_False(
                pParse, pCompare, pLevel->addrCont, SQLITE_JUMPIFNULL
            );
          }
          pCompare->p_वामः = 0;
        }
        sqlite3_व्यञ्जकम्_Delete(db, pCompare);
      }
    }

    /* These registers need to be preserved in case there is an IN operator
    ** loop.  So we could deallocate the registers here (and potentially
    ** reuse them later) if (pLoop->wsFlags & WHERE_IN_ABLE)==0.  But it seems
    ** simpler and safer to simply not reuse the registers.
    **
    **    sqlite3ReleaseTempRange(pParse, iReg, nConstraint+2);
    */
  }अन्यथा
#यदि_अन्तः /* SQLITE_OMIT_VIRTUALTABLE */

  यदि( (pLoop->wsFlags & WHERE_IPK)!=0
   && (pLoop->wsFlags & (WHERE_COLUMN_अन्तर्|WHERE_COLUMN_EQ))!=0
  ){
    /* Case 2:  We can directly reference a single row using an
    **          equality comparison against the ROWID field.  Or
    **          we reference multiple rows using a "rowid IN (...)"
    **          construct.
    */
    प्रतिज्ञा( pLoop->u.btree.nEq==1 );
    p_पदांशः = pLoop->aLTerm[0];
    प्रतिज्ञा( p_पदांशः!=0 );
    प्रतिज्ञा( p_पदांशः->p_व्यञ्जकम्!=0 );
    testcase( p_पदांशः->wtFlags & पदांशः_VIRTUAL );
    iReleaseReg = ++pParse->nMem;
    iRowidReg = संहिता_Equality_पदांशः(pParse, p_पदांशः, pLevel, 0, bRev, iReleaseReg);
    यदि( iRowidReg!=iReleaseReg ) sqlite3_Release_अस्थायी_Reg(pParse, iReleaseReg);
    addrNxt = pLevel->addrNxt;
    यदि( addr_अलब्धिः==0 ) addr_अलब्धिः = addrNxt;
    यदि( pLevel->regFilter ){
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_MustBeInt, iRowidReg, addr_अलब्धिः);
      VdbeCoverage(v);
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Filter, pLevel->regFilter, addr_अलब्धिः,
                           iRowidReg, 1);
      VdbeCoverage(v);
      filter_Pull_अधस्तात्(pParse, pWInfo, iLevel, addrNxt, notReady);
    }
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_SeekRowid, iCur, addr_अलब्धिः, iRowidReg);
    VdbeCoverage(v);
    pLevel->op = OP_Noop;
  }अन्यथा यदि( (pLoop->wsFlags & WHERE_IPK)!=0
         && (pLoop->wsFlags & WHERE_COLUMN_RANGE)!=0
  ){
    /* Case 3:  We have an inequality comparison against the ROWID field.
    */
    पूर्णाङ्क testOp = OP_Noop;
    पूर्णाङ्क start;
    पूर्णाङ्क mem_अन्तः_मानम् = 0;
    Where_पदांशः *pStart, *p_अन्तः;

    j = 0;
    pStart = p_अन्तः = 0;
    यदि( pLoop->wsFlags & WHERE_BTM_सीमा ) pStart = pLoop->aLTerm[j++];
    यदि( pLoop->wsFlags & WHERE_शिखरम्_सीमा ) p_अन्तः = pLoop->aLTerm[j++];
    प्रतिज्ञा( pStart!=0 || p_अन्तः!=0 );
    यदि( bRev ){
      p_पदांशः = pStart;
      pStart = p_अन्तः;
      p_अन्तः = p_पदांशः;
    }
    संहिता_Cursor_Hint(pTabItem, pWInfo, pLevel, p_अन्तः);
    यदि( pStart ){
      व्यञ्जकम् *pX;             /* The expression that defines the start bound */
      पूर्णाङ्क r1, r_अस्थायी;        /* Registers for holding the start boundary */
      पूर्णाङ्क op;               /* Cursor seek operation */

      /* The following constant maps TK_xx codes into corresponding
      ** seek opcodes.  It depends on a particular ordering of TK_xx
      */
      ध्रुव u8 aMoveOp[] = {
           /* TK_GT */  OP_SeekGT,
           /* TK_LE */  OP_SeekLE,
           /* TK_LT */  OP_SeekLT,
           /* TK_GE */  OP_SeekGE
      };
      प्रतिज्ञा( TK_LE==TK_GT+1 );      /* Make sure the ordering.. */
      प्रतिज्ञा( TK_LT==TK_GT+2 );      /*  ... of the TK_xx values... */
      प्रतिज्ञा( TK_GE==TK_GT+3 );      /*  ... is correct. */

      प्रतिज्ञा( (pStart->wtFlags & पदांशः_VNULL)==0 );
      testcase( pStart->wtFlags & पदांशः_VIRTUAL );
      pX = pStart->p_व्यञ्जकम्;
      प्रतिज्ञा( pX!=0 );
      testcase( pStart->वामः_Cursor!=iCur ); /* transitive constraints */
      यदि( sqlite3_व्यञ्जकम्_Is_सदिशम्(pX->p_दक्षिणः) ){
        r1 = r_अस्थायी = sqlite3_Get_अस्थायी_Reg(pParse);
        संहिता_व्यञ्जकम्_Or_सदिशम्(pParse, pX->p_दक्षिणः, r1, 1);
        testcase( pX->op==TK_GT );
        testcase( pX->op==TK_GE );
        testcase( pX->op==TK_LT );
        testcase( pX->op==TK_LE );
        op = aMoveOp[((pX->op - TK_GT - 1) & 0x3) | 0x1];
        प्रतिज्ञा( pX->op!=TK_GT || op==OP_SeekGE );
        प्रतिज्ञा( pX->op!=TK_GE || op==OP_SeekGE );
        प्रतिज्ञा( pX->op!=TK_LT || op==OP_SeekLE );
        प्रतिज्ञा( pX->op!=TK_LE || op==OP_SeekLE );
      }अन्यथा{
        r1 = sqlite3_व्यञ्जकम्_संहिता_अस्थायी(pParse, pX->p_दक्षिणः, &r_अस्थायी);
        disable_पदांशः(pLevel, pStart);
        op = aMoveOp[(pX->op - TK_GT)];
      }
      sqlite3_Vdbe_सङ्कलय_Op3(v, op, iCur, addrBrk, r1);
      VdbeComment((v, "pk"));
      VdbeCoverageIf(v, pX->op==TK_GT);
      VdbeCoverageIf(v, pX->op==TK_LE);
      VdbeCoverageIf(v, pX->op==TK_LT);
      VdbeCoverageIf(v, pX->op==TK_GE);
      sqlite3_Release_अस्थायी_Reg(pParse, r_अस्थायी);
    }अन्यथा{
      sqlite3_Vdbe_सङ्कलय_Op2(v, bRev ? OP_चरमः : OP_Rewind, iCur, pLevel->addrHalt);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
    }
    यदि( p_अन्तः ){
      व्यञ्जकम् *pX;
      pX = p_अन्तः->p_व्यञ्जकम्;
      प्रतिज्ञा( pX!=0 );
      प्रतिज्ञा( (p_अन्तः->wtFlags & पदांशः_VNULL)==0 );
      testcase( p_अन्तः->वामः_Cursor!=iCur ); /* Transitive constraints */
      testcase( p_अन्तः->wtFlags & पदांशः_VIRTUAL );
      mem_अन्तः_मानम् = ++pParse->nMem;
      संहिता_व्यञ्जकम्_Or_सदिशम्(pParse, pX->p_दक्षिणः, mem_अन्तः_मानम्, 1);
      यदि( 0==sqlite3_व्यञ्जकम्_Is_सदिशम्(pX->p_दक्षिणः)
       && (pX->op==TK_LT || pX->op==TK_GT)
      ){
        testOp = bRev ? OP_Le : OP_Ge;
      }अन्यथा{
        testOp = bRev ? OP_Lt : OP_Gt;
      }
      यदि( 0==sqlite3_व्यञ्जकम्_Is_सदिशम्(pX->p_दक्षिणः) ){
        disable_पदांशः(pLevel, p_अन्तः);
      }
    }
    start = sqlite3VdbeCurrentAddr(v);
    pLevel->op = bRev ? OP_पूर्व : OP_अग्र;
    pLevel->p1 = iCur;
    pLevel->p2 = start;
    प्रतिज्ञा( pLevel->p5==0 );
    यदि( testOp!=OP_Noop ){
      iRowidReg = ++pParse->nMem;
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rowid, iCur, iRowidReg);
      sqlite3_Vdbe_सङ्कलय_Op3(v, testOp, mem_अन्तः_मानम्, addrBrk, iRowidReg);
      VdbeCoverageIf(v, testOp==OP_Le);
      VdbeCoverageIf(v, testOp==OP_Lt);
      VdbeCoverageIf(v, testOp==OP_Ge);
      VdbeCoverageIf(v, testOp==OP_Gt);
      sqlite3VdbeChangeP5(v, SQLITE_AFF_NUMERIC | SQLITE_JUMPIFNULL);
    }
  }अन्यथा यदि( pLoop->wsFlags & WHERE_अनुक्रमितम् ){
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
    स्थिर ध्रुव u8 aStartOp[] = {
      0,
      0,
      OP_Rewind,           /* 2: (!start_constraints && startEq &&  !bRev) */
      OP_चरमः,             /* 3: (!start_constraints && startEq &&   bRev) */
      OP_SeekGT,           /* 4: (start_constraints  && !startEq && !bRev) */
      OP_SeekLT,           /* 5: (start_constraints  && !startEq &&  bRev) */
      OP_SeekGE,           /* 6: (start_constraints  &&  startEq && !bRev) */
      OP_SeekLE            /* 7: (start_constraints  &&  startEq &&  bRev) */
    };
    स्थिर ध्रुव u8 a_अन्तः_Op[] = {
      OP_IdxGE,            /* 0: (end_constraints && !bRev && !endEq) */
      OP_IdxGT,            /* 1: (end_constraints && !bRev &&  endEq) */
      OP_IdxLE,            /* 2: (end_constraints &&  bRev && !endEq) */
      OP_IdxLT,            /* 3: (end_constraints &&  bRev &&  endEq) */
    };
    u16 nEq = pLoop->u.btree.nEq;     /* Number of == or IN terms */
    u16 nBtm = pLoop->u.btree.nBtm;   /* Length of BTM vector */
    u16 n_शिखरम् = pLoop->u.btree.n_शिखरम्;   /* Length of TOP vector */
    पूर्णाङ्क reg_आधारः;                 /* Base register holding constraint values */
    Where_पदांशः *pRangeStart = 0;  /* Inequality constraint at range start */
    Where_पदांशः *p_Range_अन्तः = 0;    /* Inequality constraint at range end */
    पूर्णाङ्क startEq;                 /* True if range start uses ==, >= or <= */
    पूर्णाङ्क अन्तः_Eq;                   /* True if range end uses ==, >= or <= */
    पूर्णाङ्क start_constraints;       /* Start of range is constrained */
    पूर्णाङ्क nConstraint;             /* Number of constraint terms */
    पूर्णाङ्क iIdxCur;                 /* The VDBE cursor for the index */
    पूर्णाङ्क nExtraReg = 0;           /* Number of extra registers needed */
    पूर्णाङ्क op;                      /* Instruction opcode */
    वर्ण *zStartAff;             /* Affinity for start of range constraint */
    वर्ण *z_अन्तः_Aff = 0;           /* Affinity for end of range constraint */
    u8 bSeekPastNull = 0;        /* True to seek past initial nulls */
    u8 bStopAtNull = 0;          /* Add condition to terminate at NULLs */
    पूर्णाङ्क omit_सारणी;               /* True if we use the index only */
    पूर्णाङ्क regBignull = 0;          /* big-null flag register */
    पूर्णाङ्क addr_Seek_अवलोकनम् = 0;        /* Opcode of the OP_SeekScan, if any */

    pIdx = pLoop->u.btree.p_सूचकांक;
    iIdxCur = pLevel->iIdxCur;
    प्रतिज्ञा( nEq>=pLoop->nSkip );

    /* Find any inequality constraint terms for the start and end
    ** of the range.
    */
    j = nEq;
    यदि( pLoop->wsFlags & WHERE_BTM_सीमा ){
      pRangeStart = pLoop->aLTerm[j++];
      nExtraReg = MAX(nExtraReg, pLoop->u.btree.nBtm);
      /* Like optimization range constraints always occur in pairs */
      प्रतिज्ञा( (pRangeStart->wtFlags & पदांशः_LIKEOPT)==0 ||
              (pLoop->wsFlags & WHERE_शिखरम्_सीमा)!=0 );
    }
    यदि( pLoop->wsFlags & WHERE_शिखरम्_सीमा ){
      p_Range_अन्तः = pLoop->aLTerm[j++];
      nExtraReg = MAX(nExtraReg, pLoop->u.btree.n_शिखरम्);
#यदि_अपरिभाषितम् SQLITE_LIKE_DOESNT_मेलनम्_BLOBS
      यदि( (p_Range_अन्तः->wtFlags & पदांशः_LIKEOPT)!=0 ){
        प्रतिज्ञा( pRangeStart!=0 );                     /* LIKE opt constraints */
        प्रतिज्ञा( pRangeStart->wtFlags & पदांशः_LIKEOPT );   /* occur in pairs */
        pLevel->iLikeRepCntr = (u32)++pParse->nMem;
        /* iLikeRepCntr actually stores 2x the counter register number.  The
        ** bottom bit indicates whether the search order is ASC or DESC. */
        testcase( bRev );
        testcase( pIdx->a_क्रमय_क्रमः[nEq]==SQLITE_SO_DESC );
        प्रतिज्ञा( (bRev & ~1)==0 );
        pLevel->iLikeRepCntr <<=1;
        pLevel->iLikeRepCntr |= bRev ^ (pIdx->a_क्रमय_क्रमः[nEq]==SQLITE_SO_DESC);
      }
#यदि_अन्तः
      यदि( pRangeStart==0 ){
        j = pIdx->aiColumn[nEq];
        यदि( (j>=0 && pIdx->p_सारणी->aCol[j].notNull==0) || j==XN_व्यञ्जकम् ){
          bSeekPastNull = 1;
        }
      }
    }
    प्रतिज्ञा( p_Range_अन्तः==0 || (p_Range_अन्तः->wtFlags & पदांशः_VNULL)==0 );

    /* If the WHERE_BIGNULL_SORT flag is set, then index column nEq uses
    ** a non-default "big-null" sort (either ASC NULLS LAST or DESC NULLS
    ** FIRST). In both cases separate ordered scans are made of those
    ** index entries for which the column is null and for those for which
    ** it is not. For an ASC sort, the non-NULL entries are scanned first.
    ** For DESC, NULL entries are scanned first.
    */
    यदि( (pLoop->wsFlags & (WHERE_शिखरम्_सीमा|WHERE_BTM_सीमा))==0
     && (pLoop->wsFlags & WHERE_BIGNULL_क्रमय)!=0
    ){
      प्रतिज्ञा( bSeekPastNull==0 && nExtraReg==0 && nBtm==0 && n_शिखरम्==0 );
      प्रतिज्ञा( p_Range_अन्तः==0 && pRangeStart==0 );
      testcase( pLoop->nSkip>0 );
      nExtraReg = 1;
      bSeekPastNull = 1;
      pLevel->regBignull = regBignull = ++pParse->nMem;
      यदि( pLevel->i_वामः_Join ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 0, regBignull);
      }
      pLevel->addrBignull = sqlite3VdbeMakeLabel(pParse);
    }

    /* If we are doing a reverse order scan on an ascending index, or
    ** a forward order scan on a descending index, interchange the
    ** start and end terms (pRangeStart and pRangeEnd).
    */
    यदि( (nEq<pIdx->nColumn && bRev==(pIdx->a_क्रमय_क्रमः[nEq]==SQLITE_SO_ASC)) ){
      SWAP(Where_पदांशः *, p_Range_अन्तः, pRangeStart);
      SWAP(u8, bSeekPastNull, bStopAtNull);
      SWAP(u8, nBtm, n_शिखरम्);
    }

    यदि( iLevel>0 && (pLoop->wsFlags & WHERE_अन्तर्_SEEKSCAN)!=0 ){
      /* In case OP_SeekScan is used, ensure that the index cursor does not
      ** point to a valid row for the first iteration of this loop. */
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_NullRow, iIdxCur);
    }

    /* Generate code to evaluate all constraint terms using == or IN
    ** and store the values of those terms in an array of registers
    ** starting at regBase.
    */
    संहिता_Cursor_Hint(pTabItem, pWInfo, pLevel, p_Range_अन्तः);
    reg_आधारः = संहिता_All_Equality_Terms(pParse,pLevel,bRev,nExtraReg,&zStartAff);
    प्रतिज्ञा( zStartAff==0 || sqlite3Strlen30(zStartAff)>=nEq );
#यदि_अपरिभाषितम् SQLITE_LIKE_DOESNT_मेलनम्_BLOBS
    यदि( pLevel->iLikeRepCntr ){
      /* The LIKE optimization loop must be the innermost of any IN(...)
      ** or skip-scan loops. Otherwise rows may not be visited in index 
      ** order. This means it needs an addrNxt label for the same reason
      ** IN(...) scans do.  */
      यदि( pLevel->addrNxt==pLevel->addrBrk ){
        pLevel->addrNxt = sqlite3VdbeMakeLabel(pParse);
      }
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 1, (पूर्णाङ्क)(pLevel->iLikeRepCntr>>1));
      VdbeComment((v, "LIKE loop counter"));
      pLevel->addrLikeRep = sqlite3VdbeCurrentAddr(v);
    }
#यदि_अन्तः
    यदि( zStartAff && n_शिखरम् ){
      z_अन्तः_Aff = sqlite3DbStrDup(db, &zStartAff[nEq]);
    }
    addrNxt = (regBignull ? pLevel->addrBignull : pLevel->addrNxt);
    यदि( addr_अलब्धिः==0 ) addr_अलब्धिः = addrNxt;

    testcase( pRangeStart && (pRangeStart->eOperator & WO_LE)!=0 );
    testcase( pRangeStart && (pRangeStart->eOperator & WO_GE)!=0 );
    testcase( p_Range_अन्तः && (p_Range_अन्तः->eOperator & WO_LE)!=0 );
    testcase( p_Range_अन्तः && (p_Range_अन्तः->eOperator & WO_GE)!=0 );
    startEq = !pRangeStart || pRangeStart->eOperator & (WO_LE|WO_GE);
    अन्तः_Eq =   !p_Range_अन्तः || p_Range_अन्तः->eOperator & (WO_LE|WO_GE);
    start_constraints = pRangeStart || nEq>0;

    /* Seek the index cursor to the start of the range. */
    nConstraint = nEq;
    यदि( pRangeStart ){
      व्यञ्जकम् *p_दक्षिणः = pRangeStart->p_व्यञ्जकम्->p_दक्षिणः;
      संहिता_व्यञ्जकम्_Or_सदिशम्(pParse, p_दक्षिणः, reg_आधारः+nEq, nBtm);
      whereLikeOptimizationStringFixup(v, pLevel, pRangeStart);
      यदि( (pRangeStart->wtFlags & पदांशः_VNULL)==0
       && sqlite3_व्यञ्जकम्_Can_Be_Null(p_दक्षिणः)
      ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IsNull, reg_आधारः+nEq, addrNxt);
        VdbeCoverage(v);
      }
      यदि( zStartAff ){
        updateRangeAffinityStr(p_दक्षिणः, nBtm, &zStartAff[nEq]);
      }
      nConstraint += nBtm;
      testcase( pRangeStart->wtFlags & पदांशः_VIRTUAL );
      यदि( sqlite3_व्यञ्जकम्_Is_सदिशम्(p_दक्षिणः)==0 ){
        disable_पदांशः(pLevel, pRangeStart);
      }अन्यथा{
        startEq = 1;
      }
      bSeekPastNull = 0;
    }अन्यथा यदि( bSeekPastNull ){
      startEq = 0;
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Null, 0, reg_आधारः+nEq);
      start_constraints = 1;
      nConstraint++;
    }अन्यथा यदि( regBignull ){
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Null, 0, reg_आधारः+nEq);
      start_constraints = 1;
      nConstraint++;
    }
    संहिता_Apply_Affinity(pParse, reg_आधारः, nConstraint - bSeekPastNull, zStartAff);
    यदि( pLoop->nSkip>0 && nConstraint==pLoop->nSkip ){
      /* The skip-scan logic inside the call to codeAllEqualityConstraints()
      ** above has already left the cursor sitting on the correct row,
      ** so no further seeking is needed */
    }अन्यथा{
      यदि( regBignull ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 1, regBignull);
        VdbeComment((v, "NULL-scan pass ctr"));
      }
      यदि( pLevel->regFilter ){
        प्रतिज्ञा( sqlite3WhereLoopBloomable(pLoop) );
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Filter, pLevel->regFilter, addr_अलब्धिः,
                             reg_आधारः, nEq);
        VdbeCoverage(v);
        filter_Pull_अधस्तात्(pParse, pWInfo, iLevel, addrNxt, notReady);
      }

      op = aStartOp[(start_constraints<<2) + (startEq<<1) + bRev];
      प्रतिज्ञा( op!=0 );
      यदि( (pLoop->wsFlags & WHERE_अन्तर्_SEEKSCAN)!=0 && op==OP_SeekGE ){
        प्रतिज्ञा( regBignull==0 );
        /* TUNING:  The OP_SeekScan opcode seeks to reduce the number
        ** of expensive seek operations by replacing a single seek with
        ** 1 or more step operations.  The question is, how many steps
        ** should we try before giving up and going with a seek.  The cost
        ** of a seek is proportional to the logarithm of the of the number
        ** of entries in the tree, so basing the number of steps to try
        ** on the estimated number of rows in the btree seems like a good
        ** guess. */
        addr_Seek_अवलोकनम् = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Seek_अवलोकनम्,
                                         (pIdx->aiRowLogEst[0]+9)/10);
        यदि( pRangeStart || p_Range_अन्तः ){
          sqlite3VdbeChangeP5(v, 1);
          sqlite3VdbeChangeP2(v, addr_Seek_अवलोकनम्, sqlite3VdbeCurrentAddr(v)+1);
          addr_Seek_अवलोकनम् = 0;
        }
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, op, iIdxCur, addr_अलब्धिः, reg_आधारः, nConstraint);
      VdbeCoverage(v);
      VdbeCoverageIf(v, op==OP_Rewind);  testcase( op==OP_Rewind );
      VdbeCoverageIf(v, op==OP_चरमः);    testcase( op==OP_चरमः );
      VdbeCoverageIf(v, op==OP_SeekGT);  testcase( op==OP_SeekGT );
      VdbeCoverageIf(v, op==OP_SeekGE);  testcase( op==OP_SeekGE );
      VdbeCoverageIf(v, op==OP_SeekLE);  testcase( op==OP_SeekLE );
      VdbeCoverageIf(v, op==OP_SeekLT);  testcase( op==OP_SeekLT );

      प्रतिज्ञा( bSeekPastNull==0 || bStopAtNull==0 );
      यदि( regBignull ){
        प्रतिज्ञा( bSeekPastNull==1 || bStopAtNull==1 );
        प्रतिज्ञा( bSeekPastNull==!bStopAtNull );
        प्रतिज्ञा( bStopAtNull==startEq );
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Goto, 0, sqlite3VdbeCurrentAddr(v)+2);
        op = aStartOp[(nConstraint>1)*4 + 2 + bRev];
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, op, iIdxCur, addrNxt, reg_आधारः,
                             nConstraint-startEq);
        VdbeCoverage(v);
        VdbeCoverageIf(v, op==OP_Rewind);  testcase( op==OP_Rewind );
        VdbeCoverageIf(v, op==OP_चरमः);    testcase( op==OP_चरमः );
        VdbeCoverageIf(v, op==OP_SeekGE);  testcase( op==OP_SeekGE );
        VdbeCoverageIf(v, op==OP_SeekLE);  testcase( op==OP_SeekLE );
        प्रतिज्ञा( op==OP_Rewind || op==OP_चरमः || op==OP_SeekGE || op==OP_SeekLE);
      }
    }

    /* Load the value for the inequality constraint at the end of the
    ** range (if any).
    */
    nConstraint = nEq;
    प्रतिज्ञा( pLevel->p2==0 );
    यदि( p_Range_अन्तः ){
      व्यञ्जकम् *p_दक्षिणः = p_Range_अन्तः->p_व्यञ्जकम्->p_दक्षिणः;
      प्रतिज्ञा( addr_Seek_अवलोकनम्==0 );
      संहिता_व्यञ्जकम्_Or_सदिशम्(pParse, p_दक्षिणः, reg_आधारः+nEq, n_शिखरम्);
      whereLikeOptimizationStringFixup(v, pLevel, p_Range_अन्तः);
      यदि( (p_Range_अन्तः->wtFlags & पदांशः_VNULL)==0
       && sqlite3_व्यञ्जकम्_Can_Be_Null(p_दक्षिणः)
      ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IsNull, reg_आधारः+nEq, addrNxt);
        VdbeCoverage(v);
      }
      यदि( z_अन्तः_Aff ){
        updateRangeAffinityStr(p_दक्षिणः, n_शिखरम्, z_अन्तः_Aff);
        संहिता_Apply_Affinity(pParse, reg_आधारः+nEq, n_शिखरम्, z_अन्तः_Aff);
      }अन्यथा{
        प्रतिज्ञा( pParse->db->mallocFailed );
      }
      nConstraint += n_शिखरम्;
      testcase( p_Range_अन्तः->wtFlags & पदांशः_VIRTUAL );

      यदि( sqlite3_व्यञ्जकम्_Is_सदिशम्(p_दक्षिणः)==0 ){
        disable_पदांशः(pLevel, p_Range_अन्तः);
      }अन्यथा{
        अन्तः_Eq = 1;
      }
    }अन्यथा यदि( bStopAtNull ){
      यदि( regBignull==0 ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Null, 0, reg_आधारः+nEq);
        अन्तः_Eq = 0;
      }
      nConstraint++;
    }
    यदि( zStartAff ) sqlite3DbNNFreeNN(db, zStartAff);
    यदि( z_अन्तः_Aff ) sqlite3DbNNFreeNN(db, z_अन्तः_Aff);

    /* Top of the loop body */
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    /* Check if the index cursor is past the end of the range. */
    यदि( nConstraint ){
      यदि( regBignull ){
        /* Except, skip the end-of-range check while doing the NULL-scan */
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IfNot, regBignull, sqlite3VdbeCurrentAddr(v)+3);
        VdbeComment((v, "If NULL-scan 2nd pass"));
        VdbeCoverage(v);
      }
      op = a_अन्तः_Op[bRev*2 + अन्तः_Eq];
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, op, iIdxCur, addrNxt, reg_आधारः, nConstraint);
      testcase( op==OP_IdxGT );  VdbeCoverageIf(v, op==OP_IdxGT );
      testcase( op==OP_IdxGE );  VdbeCoverageIf(v, op==OP_IdxGE );
      testcase( op==OP_IdxLT );  VdbeCoverageIf(v, op==OP_IdxLT );
      testcase( op==OP_IdxLE );  VdbeCoverageIf(v, op==OP_IdxLE );
      यदि( addr_Seek_अवलोकनम् ) sqlite3_Vdbe_Jump_अत्र(v, addr_Seek_अवलोकनम्);
    }
    यदि( regBignull ){
      /* During a NULL-scan, check to see if we have reached the end of
      ** the NULLs */
      प्रतिज्ञा( bSeekPastNull==!bStopAtNull );
      प्रतिज्ञा( bSeekPastNull+bStopAtNull==1 );
      प्रतिज्ञा( nConstraint+bSeekPastNull>0 );
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_If, regBignull, sqlite3VdbeCurrentAddr(v)+2);
      VdbeComment((v, "If NULL-scan 1st pass"));
      VdbeCoverage(v);
      op = a_अन्तः_Op[bRev*2 + bSeekPastNull];
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, op, iIdxCur, addrNxt, reg_आधारः,
                           nConstraint+bSeekPastNull);
      testcase( op==OP_IdxGT );  VdbeCoverageIf(v, op==OP_IdxGT );
      testcase( op==OP_IdxGE );  VdbeCoverageIf(v, op==OP_IdxGE );
      testcase( op==OP_IdxLT );  VdbeCoverageIf(v, op==OP_IdxLT );
      testcase( op==OP_IdxLE );  VdbeCoverageIf(v, op==OP_IdxLE );
    }

    यदि( (pLoop->wsFlags & WHERE_अन्तर्_EARLYOUT)!=0 ){
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Seek_लब्धम्, iIdxCur, nEq, nEq);
    }

    /* Seek the table cursor, if required */
    omit_सारणी = (pLoop->wsFlags & WHERE_IDX_ONLY)!=0
           && (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_दक्षिणः_JOIN))==0;
    यदि( omit_सारणी ){
      /* pIdx is a covering index.  No need to access the main table. */
    }अन्यथा यदि( HasRowid(pIdx->p_सारणी) ){
      संहिता_Deferred_Seek(pWInfo, pIdx, iCur, iIdxCur);
    }अन्यथा यदि( iCur!=iIdxCur ){
      सूचकांक *pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pIdx->p_सारणी);
      iRowidReg = sqlite3_Get_अस्थायी_Range(pParse, pPk->n_कुञ्चिका_Col);
      प्रति(j=0; j<pPk->n_कुञ्चिका_Col; j++){
        k = sqlite3_सारणी_Column_गन्तव्यम्_सूचकांक(pIdx, pPk->aiColumn[j]);
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, iIdxCur, k, iRowidReg+j);
      }
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Not_प्राप्तम्, iCur, addrCont,
                           iRowidReg, pPk->n_कुञ्चिका_Col); VdbeCoverage(v);
    }

    यदि( pLevel->i_वामः_Join==0 ){
      /* If a partial index is driving the loop, try to eliminate WHERE clause
      ** terms from the query that must be true due to the WHERE clause of
      ** the partial index.  This optimization does not work on an outer join,
      ** as shown by:
      **
      ** 2019-11-02 ticket 623eff57e76d45f6      (LEFT JOIN)
      ** 2025-05-29 forum post 7dee41d32506c4ae  (RIGHT JOIN)
      */
      यदि( pIdx->pPartIdxWhere && pLevel->pRJ==0 ){
        where_Apply_Partial_सूचकांक_Constraints(pIdx->pPartIdxWhere, iCur, pWC);
      }
    }अन्यथा{
      testcase( pIdx->pPartIdxWhere );
      /* The following assert() is not a requirement, merely an observation:
      ** The OR-optimization doesn't work for the right hand table of
      ** a LEFT JOIN: */
      प्रतिज्ञा( (pWInfo->wctrlFlags & (WHERE_OR_SUBCLAUSE|WHERE_दक्षिणः_JOIN))==0 );
    }

    /* Record the instruction used to terminate the loop. */
    यदि( (pLoop->wsFlags & WHERE_ONEROW)
     || (pLevel->u.अन्तर्.n_अन्तर् && regBignull==0 && whereLoopIsOneRow(pLoop))
    ){
      pLevel->op = OP_Noop;
    }अन्यथा यदि( bRev ){
      pLevel->op = OP_पूर्व;
    }अन्यथा{
      pLevel->op = OP_अग्र;
    }
    pLevel->p1 = iIdxCur;
    pLevel->p3 = (pLoop->wsFlags&WHERE_UNQ_WANTED)!=0 ? 1:0;
    यदि( (pLoop->wsFlags & WHERE_CONSTRAINT)==0 ){
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_चरणः;
    }अन्यथा{
      प्रतिज्ञा( pLevel->p5==0 );
    }
    यदि( omit_सारणी ) pIdx = 0;
  }अन्यथा

#यदि_अपरिभाषितम् SQLITE_OMIT_OR_OPTIMIZATION
  यदि( pLoop->wsFlags & WHERE_MULTI_OR ){
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
    सूचकांक *pCov = 0;             /* Potential covering index (or NULL) */
    पूर्णाङ्क iCovCur = pParse->nTab++;  /* Cursor used for index scans (if any) */

    पूर्णाङ्क regReturn = ++pParse->nMem;           /* Register used with OP_Gosub */
    पूर्णाङ्क regRowset = 0;                        /* Register for RowSet object */
    पूर्णाङ्क regRowid = 0;                         /* Register holding rowid */
    पूर्णाङ्क iLoopBody = sqlite3VdbeMakeLabel(pParse);/* Start of loop body */
    पूर्णाङ्क iRetInit;                             /* Address of regReturn init */
    पूर्णाङ्क untestedTerms = 0;             /* Some terms not completely tested */
    पूर्णाङ्क ii;                            /* Loop counter */
    व्यञ्जकम् *p_And_व्यञ्जकम् = 0;                /* An ".. AND (...)" expression */
    सारणी *pTab = pTabItem->pSTab;

    p_पदांशः = pLoop->aLTerm[0];
    प्रतिज्ञा( p_पदांशः!=0 );
    प्रतिज्ञा( p_पदांशः->eOperator & WO_OR );
    प्रतिज्ञा( (p_पदांशः->wtFlags & पदांशः_ORINFO)!=0 );
    pOrWc = &p_पदांशः->u.pOrInfo->wc;
    pLevel->op = OP_Return;
    pLevel->p1 = regReturn;

    /* Set up a new SrcList in pOrTab containing the table being scanned
    ** by this loop in the a[0] slot and all notReady tables in a[1..] slots.
    ** This becomes the SrcList in the recursive call to sqlite3WhereBegin().
    */
    यदि( pWInfo->nLevel>1 || pTabItem->fg.fromExists ){
      पूर्णाङ्क nNotReady;                 /* The number of notReady tables */
      SrcItem *origSrc;              /* Original list of tables */
      nNotReady = pWInfo->nLevel - iLevel - 1;
      pOrTab = sqlite3DbMallocRawNN(db, SZ_SRCLIST(nNotReady+1));
      यदि( pOrTab==0 ) प्रत्यागम notReady;
      pOrTab->nAlloc = (u8)(nNotReady + 1);
      pOrTab->nSrc = pOrTab->nAlloc;
      स्मृतिं_प्रतिलिख(pOrTab->a, pTabItem, परिमाण(*pTabItem));
      origSrc = pWInfo->pTabList->a;
      प्रति(k=1; k<=nNotReady; k++){
        स्मृतिं_प्रतिलिख(&pOrTab->a[k], &origSrc[pLevel[k].iFrom], परिमाण(pOrTab->a[k]));
      }

      /* Clear the fromExists flag on the OR-optimized table entry so that
      ** the calls to sqlite3WhereEnd() do not code early-exits after the
      ** first row is visited. The early exit applies to this table's
      ** overall loop - including the multiple OR branches and any WHERE
      ** conditions not passed to the sub-loops - not to the sub-loops.  */
      pOrTab->a[0].fg.fromExists = 0;
    }अन्यथा{
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
    यदि( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
      यदि( HasRowid(pTab) ){
        regRowset = ++pParse->nMem;
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Null, 0, regRowset);
      }अन्यथा{
        सूचकांक *pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
        regRowset = pParse->nTab++;
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_OpenEphemeral, regRowset, pPk->n_कुञ्चिका_Col);
        sqlite3_Vdbe_समुच्चयः_P4_कुञ्चिका_Info(pParse, pPk);
      }
      regRowid = ++pParse->nMem;
    }
    iRetInit = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 0, regReturn);

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
    यदि( pWC->n_पदांशः>1 ){
      पूर्णाङ्क i_पदांशः;
      प्रति(i_पदांशः=0; i_पदांशः<pWC->n_पदांशः; i_पदांशः++){
        व्यञ्जकम् *p_व्यञ्जकम् = pWC->a[i_पदांशः].p_व्यञ्जकम्;
        यदि( &pWC->a[i_पदांशः] == p_पदांशः ) अनुवर्त;
        testcase( pWC->a[i_पदांशः].wtFlags & पदांशः_VIRTUAL );
        testcase( pWC->a[i_पदांशः].wtFlags & पदांशः_CODED );
        testcase( pWC->a[i_पदांशः].wtFlags & पदांशः_खण्डिका );
        यदि( (pWC->a[i_पदांशः].wtFlags & (पदांशः_VIRTUAL|पदांशः_CODED|पदांशः_खण्डिका))!=0 ){
          अनुवर्त;
        }
        यदि( (pWC->a[i_पदांशः].eOperator & WO_ALL)==0 ) अनुवर्त;
        यदि( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Subquery) ) अनुवर्त;  /* tag-20220303a */
        यदि( where_व्यञ्जकम्_Is_Ready(pWInfo, pLevel, pTabItem, p_व्यञ्जकम्)==0 ) अनुवर्त;
        p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Dup(db, p_व्यञ्जकम्, 0);
        p_And_व्यञ्जकम् = sqlite3_व्यञ्जकम्_And(pParse, p_And_व्यञ्जकम्, p_व्यञ्जकम्);
      }
      यदि( p_And_व्यञ्जकम् ){
        /* The extra 0x10000 bit on the opcode is masked off and does not
        ** become part of the new Expr.op.  However, it does make the
        ** op==TK_AND comparison inside of sqlite3PExpr() false, and this
        ** prevents sqlite3PExpr() from applying the AND short-circuit
        ** optimization, which we do not want here. */
        p_And_व्यञ्जकम् = sqlite3PExpr(pParse, TK_AND|0x10000, 0, p_And_व्यञ्जकम्);
      }
    }

    /* Run a separate WHERE clause for each term of the OR clause.  After
    ** eliminating duplicates from other WHERE clauses, the action for each
    ** sub-WHERE clause is to to invoke the main loop body as a subroutine.
    */
    ExplainQueryPlan((pParse, 1, "MULTI-INDEX OR"));
    प्रति(ii=0; ii<pOrWc->n_पदांशः; ii++){
      Where_पदांशः *p_Or_पदांशः = &pOrWc->a[ii];
      यदि( p_Or_पदांशः->वामः_Cursor==iCur || (p_Or_पदांशः->eOperator & WO_AND)!=0 ){
        WhereInfo *pSubWInfo;           /* Info for single OR-term scan */
        व्यञ्जकम् *p_Or_व्यञ्जकम् = p_Or_पदांशः->p_व्यञ्जकम्; /* Current OR clause term */
        व्यञ्जकम् *pDelete;                  /* Local copy of OR clause term */
        पूर्णाङ्क jmp1 = 0;                   /* Address of jump operation */
        testcase( (pTabItem[0].fg.jointype & JT_वामः)!=0
               && !व्यञ्जकम्_Has_Property(p_Or_व्यञ्जकम्, EP_OuterON)
        ); /* See TH3 vtab25.400 and ticket 614b25314c766238 */
        pDelete = p_Or_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Dup(db, p_Or_व्यञ्जकम्, 0);
        यदि( db->mallocFailed ){
          sqlite3_व्यञ्जकम्_Delete(db, pDelete);
          अनुवर्त;
        }
        यदि( p_And_व्यञ्जकम् ){
          p_And_व्यञ्जकम्->p_वामः = p_Or_व्यञ्जकम्;
          p_Or_व्यञ्जकम् = p_And_व्यञ्जकम्;
        }
        /* Loop through table entries that match term pOrTerm. */
        ExplainQueryPlan((pParse, 1, "INDEX %d", ii+1));
        WHERETRACE(0xffffffff, ("Subplan for OR-clause:\n"));
        pSubWInfo = sqlite3WhereBegin(pParse, pOrTab, p_Or_व्यञ्जकम्, 0, 0, 0,
                                      WHERE_OR_SUBCLAUSE, iCovCur);
        प्रतिज्ञा( pSubWInfo || pParse->nErr );
        यदि( pSubWInfo ){
          WhereLoop *pSubLoop;
          पूर्णाङ्क addrExplain = sqlite3_Where_Explain_One_अवलोकनम्(
              pParse, pOrTab, &pSubWInfo->a[0], 0
          );
          sqlite3_Where_सङ्कलय_अवलोकनम्_स्थितिः(v, pOrTab, &pSubWInfo->a[0], addrExplain);

          /* This is the sub-WHERE clause body.  First skip over
          ** duplicate rows from prior sub-WHERE clauses, and record the
          ** rowid (or PRIMARY KEY) for the current row so that the same
          ** row will be skipped in subsequent sub-WHERE clauses.
          */
          यदि( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
            पूर्णाङ्क i_समुच्चयः = ((ii==pOrWc->n_पदांशः-1)?-1:ii);
            यदि( HasRowid(pTab) ){
              sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, iCur, -1, regRowid);
              jmp1 = sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Row_समुच्चयः_Test, regRowset, 0,
                                          regRowid, i_समुच्चयः);
              VdbeCoverage(v);
            }अन्यथा{
              सूचकांक *pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
              पूर्णाङ्क nPk = pPk->n_कुञ्चिका_Col;
              पूर्णाङ्क iPk;
              पूर्णाङ्क r;

              /* Read the PK into an array of temp registers. */
              r = sqlite3_Get_अस्थायी_Range(pParse, nPk);
              प्रति(iPk=0; iPk<nPk; iPk++){
                पूर्णाङ्क iCol = pPk->aiColumn[iPk];
                sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, iCur, iCol,r+iPk);
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
              यदि( i_समुच्चयः ){
                jmp1 = sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_प्राप्तम्, regRowset, 0, r, nPk);
                VdbeCoverage(v);
              }
              यदि( i_समुच्चयः>=0 ){
                sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, r, nPk, regRowid);
                sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, regRowset, regRowid,
                                     r, nPk);
                यदि( i_समुच्चयः ) sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
              }

              /* Release the array of temp registers */
              sqlite3_Release_अस्थायी_Range(pParse, r, nPk);
            }
          }

          /* Invoke the main loop body as a subroutine */
          sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, regReturn, iLoopBody);

          /* Jump here (skipping the main loop body subroutine) if the
          ** current sub-WHERE row is a duplicate from prior sub-WHEREs. */
          यदि( jmp1 ) sqlite3_Vdbe_Jump_अत्र(v, jmp1);

          /* The pSubWInfo->untestedTerms flag means that this OR term
          ** contained one or more AND term from a notReady table.  The
          ** terms from the notReady table could not be tested and will
          ** need to be tested later.
          */
          यदि( pSubWInfo->untestedTerms ) untestedTerms = 1;

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
          प्रतिज्ञा( (pSubLoop->wsFlags & WHERE_AUTO_सूचकांक)==0 );
          यदि( (pSubLoop->wsFlags & WHERE_अनुक्रमितम्)!=0
           && (ii==0 || pSubLoop->u.btree.p_सूचकांक==pCov)
           && (HasRowid(pTab) || !Is_Primary_कुञ्चिका_सूचकांक(pSubLoop->u.btree.p_सूचकांक))
          ){
            प्रतिज्ञा( pSubWInfo->a[0].iIdxCur==iCovCur );
            pCov = pSubLoop->u.btree.p_सूचकांक;
          }अन्यथा{
            pCov = 0;
          }
          यदि( sqlite3_Where_उपयुङ्क्ते_Deferred_Seek(pSubWInfo) ){
            pWInfo->bDeferredSeek = 1;
          }

          /* Finish the loop through table entries that match term pOrTerm. */
          sqlite3_Where_अन्तः(pSubWInfo);
          Explain_Query_Plan_निष्कासय(pParse);
        }
        sqlite3_व्यञ्जकम्_Delete(db, pDelete);
      }
    }
    Explain_Query_Plan_निष्कासय(pParse);
    प्रतिज्ञा( pLevel->pWLoop==pLoop );
    प्रतिज्ञा( (pLoop->wsFlags & WHERE_MULTI_OR)!=0 );
    प्रतिज्ञा( (pLoop->wsFlags & WHERE_अन्तर्_ABLE)==0 );
    pLevel->u.pCoveringIdx = pCov;
    यदि( pCov ) pLevel->iIdxCur = iCovCur;
    यदि( p_And_व्यञ्जकम् ){
      p_And_व्यञ्जकम्->p_वामः = 0;
      sqlite3_व्यञ्जकम्_Delete(db, p_And_व्यञ्जकम्);
    }
    sqlite3VdbeChangeP1(v, iRetInit, sqlite3VdbeCurrentAddr(v));
    sqlite3VdbeGoto(v, pLevel->addrBrk);
    sqlite3VdbeResolveLabel(v, iLoopBody);

    /* Set the P2 operand of the OP_Return opcode that will end the current
    ** loop to point to this spot, which is the top of the next containing
    ** loop.  The byte-code formatter will use that P2 value as a hint to
    ** indent everything in between the this point and the final OP_Return.
    ** See tag-20220407a in vdbe.c and shell.c */
    प्रतिज्ञा( pLevel->op==OP_Return );
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    यदि( pWInfo->pTabList!=pOrTab ){ sqlite3DbFreeNN(db, pOrTab); }
    यदि( !untestedTerms ) disable_पदांशः(pLevel, p_पदांशः);
  }अन्यथा
#यदि_अन्तः /* SQLITE_OMIT_OR_OPTIMIZATION */

  {
    /* Case 6:  There is no usable index.  We must do a complete
    **          scan of the entire table.
    */
    स्थिर ध्रुव u8 a_चरणः[] = { OP_अग्र, OP_पूर्व };
    स्थिर ध्रुव u8 aStart[] = { OP_Rewind, OP_चरमः };
    प्रतिज्ञा( bRev==0 || bRev==1 );
    यदि( pTabItem->fg.isRecursive ){
      /* Tables marked isRecursive have only a single row that is stored in
      ** a pseudo-cursor.  No need to Rewind or Next such cursors. */
      pLevel->op = OP_Noop;
    }अन्यथा{
      संहिता_Cursor_Hint(pTabItem, pWInfo, pLevel, 0);
      pLevel->op = a_चरणः[bRev];
      pLevel->p1 = iCur;
      pLevel->p2 = 1 + sqlite3_Vdbe_सङ्कलय_Op2(v, aStart[bRev],iCur,pLevel->addrHalt);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_चरणः;
    }
  }

#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
  pLevel->addr_दर्शय = sqlite3VdbeCurrentAddr(v);
#यदि_अन्तः

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
  कुरु{
    पूर्णाङ्क i_अग्र = 0;                /* Next value for iLoop */
    प्रति(p_पदांशः=pWC->a, j=pWC->n_पदांशः; j>0; j--, p_पदांशः++){
      व्यञ्जकम् *pE;
      पूर्णाङ्क skipLikeAddr = 0;
      testcase( p_पदांशः->wtFlags & पदांशः_VIRTUAL );
      testcase( p_पदांशः->wtFlags & पदांशः_CODED );
      यदि( p_पदांशः->wtFlags & (पदांशः_VIRTUAL|पदांशः_CODED) ) अनुवर्त;
      यदि( (p_पदांशः->prereqAll & pLevel->notReady)!=0 ){
        testcase( pWInfo->untestedTerms==0
            && (pWInfo->wctrlFlags & WHERE_OR_SUBCLAUSE)!=0 );
        pWInfo->untestedTerms = 1;
        अनुवर्त;
      }
      pE = p_पदांशः->p_व्यञ्जकम्;
      प्रतिज्ञा( pE!=0 );
      यदि( where_व्यञ्जकम्_Is_Ready(pWInfo, pLevel, pTabItem, pE)==0 ) अनुवर्त;
      यदि( iLoop==1 && !sqlite3_व्यञ्जकम्_Covered_By_सूचकांक(pE, pLevel->iTabCur, pIdx) ){
        i_अग्र = 2;
        अनुवर्त;
      }
      यदि( iLoop<3 && (p_पदांशः->wtFlags & पदांशः_VARSELECT) ){
        यदि( i_अग्र==0 ) i_अग्र = 3;
        अनुवर्त;
      }

      यदि( (p_पदांशः->wtFlags & पदांशः_LIKECOND)!=0 ){
        /* If the TERM_LIKECOND flag is set, that means that the range search
        ** is sufficient to guarantee that the LIKE operator is true, so we
        ** can skip the call to the like(A,B) function.  But this only works
        ** for strings.  So do not skip the call to the function on the pass
        ** that compares BLOBs. */
#यदि_परिभाषितम् SQLITE_LIKE_DOESNT_मेलनम्_BLOBS
        अनुवर्त;
#अन्यथा
        u32 x = pLevel->iLikeRepCntr;
        यदि( x>0 ){
          skipLikeAddr = sqlite3_Vdbe_सङ्कलय_Op1(v, (x&1)?OP_IfNot:OP_If,(पूर्णाङ्क)(x>>1));
          VdbeCoverageIf(v, (x&1)==1);
          VdbeCoverageIf(v, (x&1)==0);
        }
#यदि_अन्तः
      }
#यदि_परिभाषितम् WHERETRACE_ENABLED /* 0xffffffff */
      यदि( sqlite3WhereTrace ){
        VdbeNoopComment((v, "WhereTerm[%d] (%p) priority=%d",
                         pWC->n_पदांशः-j, p_पदांशः, iLoop));
      }
      यदि( sqlite3WhereTrace & 0x4000 ){
        sqlite3DebugPrintf("Coding auxiliary constraint:\n");
        sqlite3_Where_पदांशः_Print(p_पदांशः, pWC->n_पदांशः-j);
      }
#यदि_अन्तः
      sqlite3_व्यञ्जकम्_If_False(pParse, pE, addrCont, SQLITE_JUMPIFNULL);
      यदि( skipLikeAddr ) sqlite3_Vdbe_Jump_अत्र(v, skipLikeAddr);
      p_पदांशः->wtFlags |= पदांशः_CODED;
    }
    iLoop = i_अग्र;
  }यावत्( iLoop>0 );

  /* Insert code to test for implied constraints based on transitivity
  ** of the "==" operator.
  **
  ** Example: If the WHERE clause contains "t1.a=t2.b" and "t2.b=123"
  ** and we are coding the t1 loop and the t2 loop has not yet coded,
  ** then we cannot use the "t1.a=t2.b" constraint, but we can code
  ** the implied "t1.a=123" constraint.
  */
  प्रति(p_पदांशः=pWC->a, j=pWC->n_आधारः; j>0; j--, p_पदांशः++){
    व्यञ्जकम् *pE, sEAlt;
    Where_पदांशः *pAlt;
    यदि( p_पदांशः->wtFlags & (पदांशः_VIRTUAL|पदांशः_CODED) ) अनुवर्त;
    यदि( (p_पदांशः->eOperator & (WO_EQ|WO_IS))==0 ) अनुवर्त;
    यदि( (p_पदांशः->eOperator & WO_EQUIV)==0 ) अनुवर्त;
    यदि( p_पदांशः->वामः_Cursor!=iCur ) अनुवर्त;
    यदि( pTabItem->fg.jointype & (JT_वामः|JT_LTORJ|JT_दक्षिणः) ) अनुवर्त;
    pE = p_पदांशः->p_व्यञ्जकम्;
#यदि_परिभाषितम् WHERETRACE_ENABLED /* 0x4001 */
    यदि( (sqlite3WhereTrace & 0x4001)==0x4001 ){
      sqlite3DebugPrintf("Coding transitive constraint:\n");
      sqlite3_Where_पदांशः_Print(p_पदांशः, pWC->n_पदांशः-j);
    }
#यदि_अन्तः
    प्रतिज्ञा( !व्यञ्जकम्_Has_Property(pE, EP_OuterON) );
    प्रतिज्ञा( (p_पदांशः->prereq_दक्षिणः & pLevel->notReady)!=0 );
    प्रतिज्ञा( (p_पदांशः->eOperator & (WO_OR|WO_AND))==0 );
    pAlt = sqlite3_Where_Find_पदांशः(pWC, iCur, p_पदांशः->u.x.वामः_Column, notReady,
                    WO_EQ|WO_अन्तर्|WO_IS, 0);
    यदि( pAlt==0 ) अनुवर्त;
    यदि( pAlt->wtFlags & (पदांशः_CODED) ) अनुवर्त;
    यदि( व्यञ्जकम्_Has_Property(pAlt->p_व्यञ्जकम्, EP_Collate) ) अनुवर्त;
    यदि( (pAlt->eOperator & WO_अन्तर्)
     && व्यञ्जकम्_Use_XSelect(pAlt->p_व्यञ्जकम्)
     && (pAlt->p_व्यञ्जकम्->x.pSelect->pEList->n_व्यञ्जकम्>1)
    ){
      अनुवर्त;
    }
    testcase( pAlt->eOperator & WO_EQ );
    testcase( pAlt->eOperator & WO_IS );
    testcase( pAlt->eOperator & WO_अन्तर् );
    VdbeModuleComment((v, "begin transitive constraint"));
    sEAlt = *pAlt->p_व्यञ्जकम्;
    sEAlt.p_वामः = pE->p_वामः;
    sqlite3_व्यञ्जकम्_If_False(pParse, &sEAlt, addrCont, SQLITE_JUMPIFNULL);
    pAlt->wtFlags |= पदांशः_CODED;
  }

  /* For a RIGHT OUTER JOIN, record the fact that the current row has
  ** been matched at least once.
  */
  यदि( pLevel->pRJ ){
    सारणी *pTab;
    पूर्णाङ्क nPk;
    पूर्णाङ्क r;
    पूर्णाङ्क jmp1 = 0;
    Where_दक्षिणः_Join *pRJ = pLevel->pRJ;

    /* pTab is the right-hand table of the RIGHT JOIN.  Generate code that
    ** will record that the current row of that table has been matched at
    ** least once.  This is accomplished by storing the PK for the row in
    ** both the iMatch index and the regBloom Bloom filter.
    */
    pTab = pWInfo->pTabList->a[pLevel->iFrom].pSTab;
    यदि( HasRowid(pTab) ){
      r = sqlite3_Get_अस्थायी_Range(pParse, 2);
      sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, pLevel->iTabCur, -1, r+1);
      nPk = 1;
    }अन्यथा{
      पूर्णाङ्क iPk;
      सूचकांक *pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
      nPk = pPk->n_कुञ्चिका_Col;
      r = sqlite3_Get_अस्थायी_Range(pParse, nPk+1);
      प्रति(iPk=0; iPk<nPk; iPk++){
        पूर्णाङ्क iCol = pPk->aiColumn[iPk];
        sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, iCur, iCol,r+1+iPk);
      }
    }
    jmp1 = sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_प्राप्तम्, pRJ->i_मेलनम्, 0, r+1, nPk);
    VdbeCoverage(v);
    VdbeComment((v, "match against %s", pTab->zName));
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, r+1, nPk, r);
    sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, pRJ->i_मेलनम्, r, r+1, nPk);
    यदि( pRJ->regBloom ){
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Filter_सङ्कलय, pRJ->regBloom, 0, r+1, nPk);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
    }
    sqlite3_Vdbe_Jump_अत्र(v, jmp1);
    sqlite3_Release_अस्थायी_Range(pParse, r, nPk+1);
  }

  /* For a LEFT OUTER JOIN, generate code that will record the fact that
  ** at least one row of the right table has matched the left table.
  */
  यदि( pLevel->i_वामः_Join ){
    pLevel->addrFirst = sqlite3VdbeCurrentAddr(v);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 1, pLevel->i_वामः_Join);
    VdbeComment((v, "record LEFT JOIN hit"));
    यदि( pLevel->pRJ==0 ){
      गच्छ संहिता_outer_join_constraints; /* WHERE clause constraints */
    }
  }

  यदि( pLevel->pRJ ){
    /* Create a subroutine used to process all interior loops and code
    ** of the RIGHT JOIN.  During normal operation, the subroutine will
    ** be in-line with the rest of the code.  But at the end, a separate
    ** loop will run that invokes this subroutine for unmatched rows
    ** of pTab, with all tables to left begin set to NULL.
    */
    {
      Where_दक्षिणः_Join *pRJ = pLevel->pRJ;
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_BeginSubrtn, 0, pRJ->regReturn);
      pRJ->addrSubrtn = sqlite3VdbeCurrentAddr(v);
    }
    प्रतिज्ञा( pParse->withinRJSubrtn < 255 );
    pParse->withinRJSubrtn++;

    /* WHERE clause constraints must be deferred until after outer join
    ** row elimination has completed, since WHERE clause constraints apply
    ** to the results of the OUTER JOIN.  The following loop generates the
    ** appropriate WHERE clause constraint checks.  tag-20220513a.
    */
  संहिता_outer_join_constraints:
    प्रति(p_पदांशः=pWC->a, j=0; j<pWC->n_आधारः; j++, p_पदांशः++){
      testcase( p_पदांशः->wtFlags & पदांशः_VIRTUAL );
      testcase( p_पदांशः->wtFlags & पदांशः_CODED );
      यदि( p_पदांशः->wtFlags & (पदांशः_VIRTUAL|पदांशः_CODED) ) अनुवर्त;
      यदि( (p_पदांशः->prereqAll & pLevel->notReady)!=0 ){
        प्रतिज्ञा( pWInfo->untestedTerms );
        अनुवर्त;
      }
      यदि( pTabItem->fg.jointype & JT_LTORJ ) अनुवर्त;
      प्रतिज्ञा( p_पदांशः->p_व्यञ्जकम् );
      sqlite3_व्यञ्जकम्_If_False(pParse, p_पदांशः->p_व्यञ्जकम्, addrCont, SQLITE_JUMPIFNULL);
      p_पदांशः->wtFlags |= पदांशः_CODED;
    }
  }

#यदि WHERETRACE_ENABLED /* 0x4001 */
  यदि( sqlite3WhereTrace & 0x4000 ){
    sqlite3DebugPrintf("All WHERE-clause terms after coding level %d:\n",
                       iLevel);
    sqlite3WhereClausePrint(pWC);
  }
  यदि( sqlite3WhereTrace & 0x1 ){
    sqlite3DebugPrintf("End Coding level %d:  notReady=%llx\n",
       iLevel, (u64)pLevel->notReady);
  }
#यदि_अन्तः
  प्रत्यागम pLevel->notReady;
}

/*
** Generate the code for the loop that finds all non-matched terms
** for a RIGHT JOIN.
*/
SQLITE_NOINLINE शून्य sqlite3_Where_दक्षिणः_Join_Loop(
  WhereInfo *pWInfo,
  पूर्णाङ्क iLevel,
  WhereLevel *pLevel
){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  Where_दक्षिणः_Join *pRJ = pLevel->pRJ;
  व्यञ्जकम् *pSubWhere = 0;
  WhereClause *pWC = &pWInfo->sWC;
  WhereInfo *pSubWInfo;
  WhereLoop *pLoop = pLevel->pWLoop;
  SrcItem *pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
  SrcList *pFrom;
  सङ्घ {
    SrcList sSrc;
    u8 fromSpace[SZ_SRCLIST_1];
  } uSrc;
  Bitmask mAll = 0;
  पूर्णाङ्क k;

  ExplainQueryPlan((pParse, 1, "RIGHT-JOIN %s", pTabItem->pSTab->zName));
  sqlite3VdbeNoJumpsOutsideSubrtn(v, pRJ->addrSubrtn, pRJ->अन्तः_Subrtn,
                                  pRJ->regReturn);
  प्रति(k=0; k<iLevel; k++){
    पूर्णाङ्क iIdxCur;
    SrcItem *p_दक्षिणः;
    प्रतिज्ञा( pWInfo->a[k].pWLoop->iTab == pWInfo->a[k].iFrom );
    p_दक्षिणः = &pWInfo->pTabList->a[pWInfo->a[k].iFrom];
    mAll |= pWInfo->a[k].pWLoop->आवरणम्_Self;
    यदि( p_दक्षिणः->fg.द्वारा_Coroutine ){
      Subquery *pSubq;
      प्रतिज्ञा( p_दक्षिणः->fg.isSubquery && p_दक्षिणः->u4.pSubq!=0 );
      pSubq = p_दक्षिणः->u4.pSubq;
      प्रतिज्ञा( pSubq->pSelect!=0 && pSubq->pSelect->pEList!=0 );
      sqlite3_Vdbe_सङ्कलय_Op3(
          v, OP_Null, 0, pSubq->reg_परिणाम,
          pSubq->reg_परिणाम + pSubq->pSelect->pEList->n_व्यञ्जकम्-1
      );
    }
    sqlite3_Vdbe_सङ्कलय_Op1(v, OP_NullRow, pWInfo->a[k].iTabCur);
    iIdxCur = pWInfo->a[k].iIdxCur;
    यदि( iIdxCur ){
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_NullRow, iIdxCur);
    }
  }
  यदि( (pTabItem->fg.jointype & JT_LTORJ)==0 ){
    mAll |= pLoop->आवरणम्_Self;
    प्रति(k=0; k<pWC->n_पदांशः; k++){
      Where_पदांशः *p_पदांशः = &pWC->a[k];
      यदि( (p_पदांशः->wtFlags & (पदांशः_VIRTUAL|पदांशः_खण्डिका))!=0
       && p_पदांशः->eOperator!=WO_ROWVAL
      ){
        भङ्ग;
      }
      यदि( p_पदांशः->prereqAll & ~mAll ) अनुवर्त;
      यदि( व्यञ्जकम्_Has_Property(p_पदांशः->p_व्यञ्जकम्, EP_OuterON|EP_InnerON) ) अनुवर्त;
      pSubWhere = sqlite3_व्यञ्जकम्_And(pParse, pSubWhere,
                                 sqlite3_व्यञ्जकम्_Dup(pParse->db, p_पदांशः->p_व्यञ्जकम्, 0));
    }
  }
  यदि( pLevel->iIdxCur ){
    /* pSubWhere may contain expressions that read from an index on the
    ** table on the RHS of the right join. All such expressions first test
    ** if the index is pointing at a NULL row, and if so, read from the
    ** table cursor instead. So ensure that the index cursor really is 
    ** pointing at a NULL row here, so that no values are read from it during
    ** the scan of the RHS of the RIGHT join below.  */
    sqlite3_Vdbe_सङ्कलय_Op1(v, OP_NullRow, pLevel->iIdxCur);
  }
  pFrom = &uSrc.sSrc;
  pFrom->nSrc = 1;
  pFrom->nAlloc = 1;
  स्मृतिं_प्रतिलिख(&pFrom->a[0], pTabItem, परिमाण(SrcItem));
  pFrom->a[0].fg.jointype = 0;
  यदि( pParse->withinRJSubrtn >= 100 ){
    /* This limit --------------^^^
    ** is based on an historical assert().  It is not compile-time or
    ** run-time configurable.  It could perhaps be raised as high as 254,
    ** but only an attack robot would ever do even 100 RIGHT JOINS within
    ** a single query, so we'll just leave it as it is. */
    sqlite3_Error_सन्देशः(pParse, "too many RIGHT JOINs");
    प्रत्यागम;
  }
  pParse->withinRJSubrtn++;
  pSubWInfo = sqlite3WhereBegin(pParse, pFrom, pSubWhere, 0, 0, 0,
                                WHERE_दक्षिणः_JOIN, 0);
  यदि( pSubWInfo ){
    पूर्णाङ्क iCur = pLevel->iTabCur;  /* Table on RHS of RIGHT JOIN &*/
    पूर्णाङ्क r = ++pParse->nMem;      /* Register range to hold primary key */
    पूर्णाङ्क nPk;                     /* Number of values in the primary key */
    पूर्णाङ्क r2;                      /* Register holding record for primary key */
    पूर्णाङ्क addrCont = sqlite3WhereContinueLabel(pSubWInfo);
    सारणी *pTab = pTabItem->pSTab;  /* Table on RHS of RIGHT JOIN */
    यदि( HasRowid(pTab) ){
      sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, iCur, -1, r);
      nPk = 1;
    }अन्यथा{
      पूर्णाङ्क iPk;
      सूचकांक *pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
      nPk = pPk->n_कुञ्चिका_Col;
      pParse->nMem += nPk - 1;
      प्रति(iPk=0; iPk<nPk; iPk++){
        पूर्णाङ्क iCol = pPk->aiColumn[iPk];
        sqlite3_व्यञ्जकम्_संहिता_Get_Column_सम्बन्धि_सारणी(v, pTab, iCur, iCol,r+iPk);
      }
    }

    /* Generate code that checks to see if the current row of the RHS table
    ** has appeared in any prior output row. */
    यदि( pRJ->regBloom ){
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Filter, pRJ->regBloom, 
                 sqlite3VdbeCurrentAddr(v)+2, r, nPk);
      VdbeCoverage(v);
    }
    sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_प्राप्तम्, pRJ->i_मेलनम्, addrCont, r, nPk);
    VdbeCoverage(v);
    r2 = sqlite3_Get_अस्थायी_Reg(pParse);

    /* Generate code that inserts the PK of the RHS table into the
    ** pRH->iMatch index to indicate that the current row has appeared
    ** in the output set. */
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, r, nPk, r2);
    sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, pRJ->i_मेलनम्, r2, r, nPk);
    sqlite3_Release_अस्थायी_Reg(pParse, r2);
    यदि( pRJ->regBloom ){
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Filter_सङ्कलय, pRJ->regBloom, 0, r, nPk);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
    }

    /* Invoke the subroutine that actually puts the current RHS table row
    ** into the output set, with NULLs for the LHS. */
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, pRJ->regReturn, pRJ->addrSubrtn);
    sqlite3_Where_अन्तः(pSubWInfo);
  }
  sqlite3_व्यञ्जकम्_Delete(pParse->db, pSubWhere);
  Explain_Query_Plan_निष्कासय(pParse);
  प्रतिज्ञा( pParse->withinRJSubrtn>0 );
  pParse->withinRJSubrtn--;
}
