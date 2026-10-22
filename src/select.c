/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains C code routines that are called by the parser
** to handle SELECT statements in SQLite.
*/
#ضمن "sqliteInt.h"

/*
** An instance of the following object is used to record information about
** how to process the DISTINCT keyword, to simplify passing that information
** into the selectInnerLoop() routine.
*/
تعريفنوع بنية DistinctCtx DistinctCtx;
بنية DistinctCtx {
  u8 isTnct;      /* 0: Not distinct. 1: DISTINCT  2: DISTINCT and ORDER BY */
  u8 eTnctType;   /* One of the WHERE_DISTINCT_* operators */
  صحيح tabTnct;    /* Ephemeral table used for DISTINCT processing */
  صحيح addrTnct;   /* Address of OP_OpenEphemeral opcode for tabTnct */
};

/*
** An instance of the following object is used to record information about
** the ORDER BY (or GROUP BY) clause of query is being coded.
**
** The aDefer[] array is used by the sorter-references optimization. For
** example, assuming there is no index that can be used for the ORDER BY,
** for the query:
**
**     SELECT a, bigblob FROM t1 ORDER BY a LIMIT 10;
**
** it may be more efficient to add just the "a" values to the sorter, and
** retrieve the associated "bigblob" values directly from table t1 as the
** 10 smallest "a" values are extracted from the sorter.
**
** When the sorter-reference optimization is used, there is one entry in the
** aDefer[] array for each database table that may be read as values are
** extracted from the sorter.
*/
تعريفنوع بنية ترتیبول_Ctx ترتیبول_Ctx;
بنية ترتیبول_Ctx {
  بیان_List *p_ترتیب_By;   /* The ORDER BY (or GROUP BY clause) */
  صحيح nOBSat;           /* Number of ORDER BY terms satisfied by indices */
  صحيح iECursor;         /* Cursor number for the sorter */
  صحيح regReturn;        /* Register holding block-output return address */
  صحيح label_Bk_بهر;       /* Start label for the block-output subroutine */
  صحيح addr_ترتیبول_شاخص;    /* Address of the OP_SorterOpen or OP_OpenEphemeral */
  صحيح label_بشپړ;        /* Jump here when done, ex: LIMIT reached */
  صحيح labelOBLopt;      /* Jump here when sorter is full */
  u8 ترتیبول_Flags;         /* Zero or more SORTFLAG_* bits */
#که_تعریف_شوی SQLITE_ENABLE_SORTER_REFERENCES
  u8 nDefer;            /* Number of valid entries in aDefer[] */
  بنية DeferredCsr {
    جدول *pTab;        /* Table definition */
    صحيح iCsr;           /* Cursor number for table */
    صحيح n_کلی;           /* Number of PK columns for table pTab (>=1) */
  } aDefer[4];
#د_که_پای
  بنية لیکه_بارول_Info *p_Deferred_لیکه_بارول;  /* Deferred row loading info or NULL */
#که_تعریف_شوی SQLITE_ENABLE_STMT_SCANSTATUS
  صحيح addr_ټېل;         /* First instruction to push data into sorter */
  صحيح addr_ټېل_پای_ټکی;      /* Last instruction that pushes data into sorter */
#د_که_پای
};
#تعریف_کړه SORTFLAG_UseSorter  0x01   /* Use SorterOpen instead of OpenEphemeral */

/*
** Delete all the content of a Select structure.  Deallocate the structure
** itself depending on the value of bFree
**
** If bFree==1, call sqlite3DbFree() on the p object.
** If bFree==0, Leave the first Select object unfreed
*/
ساكن فراغ clearSelect(sqlite3 *db, Select *p, صحيح bFree){
  أكد( db!=0 );
  طالما( p ){
    Select *p_مخکینی = p->p_مخکینی;
    sqlite3_بیان_List_Delete(db, p->pEList);
    sqlite3SrcListDelete(db, p->pSrc);
    sqlite3_بیان_Delete(db, p->pWhere);
    sqlite3_بیان_List_Delete(db, p->p_ډله_By);
    sqlite3_بیان_Delete(db, p->pHaving);
    sqlite3_بیان_List_Delete(db, p->p_ترتیب_By);
    sqlite3_بیان_Delete(db, p->p_حد);
    إذا( OK_IF_ALWAYS_TRUE(p->pWith) ) sqlite3WithDelete(db, p->pWith);
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
    إذا( OK_IF_ALWAYS_TRUE(p->pWinDefn) ){
      sqlite3_کړکۍ_List_Delete(db, p->pWinDefn);
    }
    طالما( p->pWin ){
      أكد( p->pWin->ppThis==&p->pWin );
      sqlite3_کړکۍ_Unlink_From_Select(p->pWin);
    }
#د_که_پای
    إذا( bFree ) sqlite3DbNNFreeNN(db, p);
    p = p_مخکینی;
    bFree = 1;
  }
}

/*
** Initialize a SelectDest structure.
*/
فراغ sqlite3SelectDestInit(SelectDest *pDest, صحيح eDest, صحيح iParm){
  pDest->eDest = (u8)eDest;
  pDest->iSDParm = iParm;
  pDest->iSDParm2 = 0;
  pDest->zAffSdst = 0;
  pDest->iSdst = 0;
  pDest->nSdst = 0;
}


/*
** Allocate a new Select structure and return a pointer to that
** structure.
*/
Select *sqlite3SelectNew(
  Parse *pParse,        /* Parsing context */
  بیان_List *pEList,     /* which columns to include in the result */
  SrcList *pSrc,        /* the FROM clause -- which tables to scan */
  بیان *pWhere,         /* the WHERE clause */
  بیان_List *p_ډله_By,   /* the GROUP BY clause */
  بیان *pHaving,        /* the HAVING clause */
  بیان_List *p_ترتیب_By,   /* the ORDER BY clause */
  u32 selFlags,         /* Flag parameters, such as SF_Distinct */
  بیان *p_حد          /* LIMIT value.  NULL means not used */
){
  Select *pNew, *pAllocated;
  Select standin;
  pAllocated = pNew = sqlite3DbMallocRawNN(pParse->db, حجم(*pNew) );
  إذا( pNew==0 ){
    أكد( pParse->db->mallocFailed );
    pNew = &standin;
  }
  إذا( pEList==0 ){
    pEList = sqlite3_بیان_List_Append(pParse, 0,
                                   sqlite3_بیان(pParse->db,TK_ASTERISK,0));
  }
  pNew->pEList = pEList;
  pNew->op = TK_SELECT;
  pNew->selFlags = selFlags;
  pNew->i_حد = 0;
  pNew->i_دننه_واټن = 0;
  pNew->selId = ++pParse->nSelect;
  pNew->n_Select_لیکه = 0;
  إذا( pSrc==0 ) pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  pNew->pSrc = pSrc;
  pNew->pWhere = pWhere;
  pNew->p_ډله_By = p_ډله_By;
  pNew->pHaving = pHaving;
  pNew->p_ترتیب_By = p_ترتیب_By;
  pNew->p_مخکینی = 0;
  pNew->p_بل = 0;
  pNew->p_حد = p_حد;
  pNew->pWith = 0;
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
  pNew->pWin = 0;
  pNew->pWinDefn = 0;
#د_که_پای
  إذا( pParse->db->mallocFailed ) {
    clearSelect(pParse->db, pNew, pNew!=&standin);
    pAllocated = 0;
  }وإلا{
    أكد( pNew->pSrc!=0 || pParse->nErr>0 );
  }
  أرجع pAllocated;
}


/*
** Delete the given Select structure and all of its substructures.
*/
فراغ sqlite3SelectDelete(sqlite3 *db, Select *p){
  إذا( OK_IF_ALWAYS_TRUE(p) ) clearSelect(db, p, 1);
}
فراغ sqlite3SelectDeleteGeneric(sqlite3 *db, فراغ *p){
  إذا( ALWAYS(p) ) clearSelect(db, (Select*)p, 1);
}

/*
** Return a pointer to the right-most SELECT statement in a compound.
*/
ساكن Select *پیدا_کړه_Rightmost(Select *p){
  طالما( p->p_بل ) p = p->p_بل;
  أرجع p;
}

/*
** Given 1 to 3 identifiers preceding the JOIN keyword, determine the
** type of join.  Return an integer constant that expresses that type
** in terms of the following bit values:
**
**     JT_INNER
**     JT_CROSS
**     JT_OUTER
**     JT_NATURAL
**     JT_LEFT
**     JT_RIGHT
**
** A full outer join is the combination of JT_LEFT and JT_RIGHT.
**
** If an illegal or unsupported join type is seen, then still return
** a join type, but put an error in the pParse structure.
**
** These are the valid join types:
**
**
**      pA       pB       pC               Return Value
**     -------  -----    -----             ------------
**     CROSS      -        -                 JT_CROSS
**     INNER      -        -                 JT_INNER
**     LEFT       -        -                 JT_LEFT|JT_OUTER
**     LEFT     OUTER      -                 JT_LEFT|JT_OUTER
**     RIGHT      -        -                 JT_RIGHT|JT_OUTER
**     RIGHT    OUTER      -                 JT_RIGHT|JT_OUTER
**     FULL       -        -                 JT_LEFT|JT_RIGHT|JT_OUTER
**     FULL     OUTER      -                 JT_LEFT|JT_RIGHT|JT_OUTER
**     NATURAL  INNER      -                 JT_NATURAL|JT_INNER
**     NATURAL  LEFT       -                 JT_NATURAL|JT_LEFT|JT_OUTER
**     NATURAL  LEFT     OUTER               JT_NATURAL|JT_LEFT|JT_OUTER
**     NATURAL  RIGHT      -                 JT_NATURAL|JT_RIGHT|JT_OUTER
**     NATURAL  RIGHT    OUTER               JT_NATURAL|JT_RIGHT|JT_OUTER
**     NATURAL  FULL       -                 JT_NATURAL|JT_LEFT|JT_RIGHT
**     NATURAL  FULL     OUTER               JT_NATRUAL|JT_LEFT|JT_RIGHT
**
** To preserve historical compatibly, SQLite also accepts a variety
** of other non-standard and in many cases nonsensical join types.
** This routine makes as much sense at it can from the nonsense join
** type and returns a result.  Examples of accepted nonsense join types
** include but are not limited to:
**
**          INNER CROSS JOIN        ->   same as JOIN
**          NATURAL CROSS JOIN      ->   same as NATURAL JOIN
**          OUTER LEFT JOIN         ->   same as LEFT JOIN
**          LEFT NATURAL JOIN       ->   same as NATURAL LEFT JOIN
**          LEFT RIGHT JOIN         ->   same as FULL JOIN
**          RIGHT OUTER FULL JOIN   ->   same as FULL JOIN
**          CROSS CROSS CROSS JOIN  ->   same as JOIN
**
** The only restrictions on the join type name are:
**
**    *   "INNER" cannot appear together with "OUTER", "LEFT", "RIGHT",
**        or "FULL".
**
**    *   "CROSS" cannot appear together with "OUTER", "LEFT", "RIGHT,
**        or "FULL".
**
**    *   If "OUTER" is present then there must also be one of
**        "LEFT", "RIGHT", or "FULL"
*/
صحيح sqlite3JoinType(Parse *pParse, Token *pA, Token *pB, Token *pC){
  صحيح jointype = 0;
  Token *apAll[3];
  Token *p;
                             /*   0123456789 123456789 123456789 123 */
  ساكن ثابت حرف z_کلی_متن[] = "naturaleftouterightfullinnercross";
  ساكن ثابت بنية {
    u8 i;        /* Beginning of keyword text in zKeyText[] */
    u8 nChar;    /* Length of the keyword in characters */
    u8 کوډ;     /* Join type mask */
  } aKeyword[] = {
    /* (0) natural */ { 0,  7, JT_NATURAL                },
    /* (1) left    */ { 6,  4, JT_کیڼ|JT_OUTER          },
    /* (2) outer   */ { 10, 5, JT_OUTER                  },
    /* (3) right   */ { 14, 5, JT_ښی|JT_OUTER         },
    /* (4) full    */ { 19, 4, JT_کیڼ|JT_ښی|JT_OUTER },
    /* (5) inner   */ { 23, 5, JT_INNER                  },
    /* (6) cross   */ { 28, 5, JT_INNER|JT_CROSS         },
  };
  صحيح i, j;
  apAll[0] = pA;
  apAll[1] = pB;
  apAll[2] = pC;
  لكل(i=0; i<3 && apAll[i]; i++){
    p = apAll[i];
    لكل(j=0; j<Array_اندازه(aKeyword); j++){
      إذا( p->n==aKeyword[j].nChar
          && sqlite3StrNICmp((حرف*)p->z, &z_کلی_متن[aKeyword[j].i], p->n)==0 ){
        jointype |= aKeyword[j].کوډ;
        اكسر;
      }
    }
    testcase( j==0 || j==1 || j==2 || j==3 || j==4 || j==5 || j==6 );
    إذا( j>=Array_اندازه(aKeyword) ){
      jointype |= JT_ERROR;
      اكسر;
    }
  }
  إذا(
     (jointype & (JT_INNER|JT_OUTER))==(JT_INNER|JT_OUTER) ||
     (jointype & JT_ERROR)!=0 ||
     (jointype & (JT_OUTER|JT_کیڼ|JT_ښی))==JT_OUTER
  ){
    ثابت حرف *zSp1 = " ";
    ثابت حرف *zSp2 = " ";
    إذا( pB==0 ){ zSp1++; }
    إذا( pC==0 ){ zSp2++; }
    sqlite3_Error_پیغام(pParse, "unknown join type: "
       "%T%s%T%s%T", pA, zSp1, pB, zSp2, pC);
    jointype = JT_INNER;
  }
  أرجع jointype;
}

/*
** Return the index of a column in a table.  Return -1 if the column
** is not contained in the table.
*/
صحيح sqlite3_ستنه_شاخص(جدول *pTab, ثابت حرف *z_ستون){
  صحيح i;
  u8 h;
  ثابت ستنه *a_ستون;
  صحيح n_ستون;

  h = sqlite3StrIHash(z_ستون);
  a_ستون = pTab->a_ستون;
  n_ستون = pTab->n_ستون;

  /* See if the aHx gives us a lucky match */
  i = pTab->aHx[h % حجم(pTab->aHx)];
  أكد( i<n_ستون );
  إذا( a_ستون[i].hName==h
   && sqlite3StrICmp(a_ستون[i].zCnName, z_ستون)==0
  ){
    أرجع i;
  }

  /* No lucky match from the hash table.  Do a full search. */
  i = 0;
  طالما( 1 /*exit-by-break*/ ){
    إذا( a_ستون[i].hName==h
     && sqlite3StrICmp(a_ستون[i].zCnName, z_ستون)==0
    ){
      أرجع i;
    }
    i++;
    إذا( i>=n_ستون ) اكسر;
  }
  أرجع -1;
}

/*
** Mark a subquery result column as having been used.
*/
فراغ sqlite3_Src_توکی_ستنه_کارول_شوی(Src_توکی *p_توکی, صحيح i_ستون){
  أكد( p_توکی!=0 );
  أكد( (صحيح)p_توکی->fg.isNestedFrom == IsNestedFrom(p_توکی) );
  إذا( p_توکی->fg.isNestedFrom ){
    بیان_List *pResults;
    أكد( p_توکی->fg.isSubquery );
    أكد( p_توکی->u4.pSubq!=0 );
    أكد( p_توکی->u4.pSubq->pSelect!=0 );
    pResults = p_توکی->u4.pSubq->pSelect->pEList;
    أكد( pResults!=0 );
    أكد( i_ستون>=0 && i_ستون<pResults->n_بیان );
    pResults->a[i_ستون].fg.b_کارول_شوی = 1;
  }
}

/*
** Search the tables iStart..iEnd (inclusive) in pSrc, looking for a
** table that has a column named zCol.  The search is left-to-right.
** The first match found is returned.
**
** When found, set *piTab and *piCol to the table index and column index
** of the matching column and return TRUE.
**
** If not found, return FALSE.
*/
ساكن صحيح جدول_And_ستنه_شاخص(
  SrcList *pSrc,       /* Array of tables to search */
  صحيح i_پیل,          /* First member of pSrc->a[] to check */
  صحيح i_پای_ټکی,            /* Last member of pSrc->a[] to check */
  ثابت حرف *z_ستون,    /* Name of the column we are looking for */
  صحيح *piTab,          /* Write index of pSrc->a[] here */
  صحيح *pi_ستون,          /* Write index of pSrc->a[*piTab].pSTab->aCol[] here */
  صحيح bIgnoreHidden    /* Ignore hidden columns */
){
  صحيح i;               /* For looping over tables in pSrc */
  صحيح i_ستون;            /* Index of column matching zCol */

  أكد( i_پای_ټکی<pSrc->nSrc );
  أكد( i_پیل>=0 );
  أكد( (piTab==0)==(pi_ستون==0) );  /* Both or neither are NULL */

  لكل(i=i_پیل; i<=i_پای_ټکی; i++){
    i_ستون = sqlite3_ستنه_شاخص(pSrc->a[i].pSTab, z_ستون);
    إذا( i_ستون>=0
     && (bIgnoreHidden==0 || Is_Hidden_ستنه(&pSrc->a[i].pSTab->a_ستون[i_ستون])==0)
    ){
      إذا( piTab ){
        sqlite3_Src_توکی_ستنه_کارول_شوی(&pSrc->a[i], i_ستون);
        *piTab = i;
        *pi_ستون = i_ستون;
      }
      أرجع 1;
    }
  }
  أرجع 0;
}

/*
** Set the EP_OuterON property on all terms of the given expression.
** And set the Expr.w.iJoin to iTable for every term in the
** expression.
**
** The EP_OuterON property is used on terms of an expression to tell
** the OUTER JOIN processing logic that this term is part of the
** join restriction specified in the ON or USING clause and not a part
** of the more general WHERE clause.  These terms are moved over to the
** WHERE clause during join processing but we need to remember that they
** originated in the ON or USING clause.
**
** The Expr.w.iJoin tells the WHERE clause processing that the
** expression depends on table w.iJoin even if that table is not
** explicitly mentioned in the expression.  That information is needed
** for cases like this:
**
**    SELECT * FROM t1 LEFT JOIN t2 ON t1.a=t2.b AND t1.x=5
**
** The where clause needs to defer the handling of the t1.x=5
** term until after the t2 loop of the join.  In that way, a
** NULL t2 row will be inserted whenever t1.x!=5.  If we do not
** defer the handling of t1.x=5, it will be processed immediately
** after the t1 loop and rows with t1.x!=5 will never appear in
** the output, which is incorrect.
*/
فراغ sqlite3_سېټ_Join_بیان(بیان *p, صحيح i_جدول, u32 joinFlag){
  أكد( joinFlag==EP_OuterON || joinFlag==EP_InnerON );
  طالما( p ){
    بیان_سېټ_Property(p, joinFlag);
    أكد( !بیان_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    بیان_سېټ_VVAProperty(p, EP_NoReduce);
    p->w.iJoin = i_جدول;
    إذا( بیان_Use_XList(p) ){
      إذا( p->x.pList ){
        صحيح i;
        لكل(i=0; i<p->x.pList->n_بیان; i++){
          sqlite3_سېټ_Join_بیان(p->x.pList->a[i].p_بیان, i_جدول, joinFlag);
        }
      }
    }
    sqlite3_سېټ_Join_بیان(p->p_کیڼ, i_جدول, joinFlag);
    p = p->p_ښی;
  }
}

/* Undo the work of sqlite3SetJoinExpr().  This is used when a LEFT JOIN
** is simplified into an ordinary JOIN, and when an ON expression is
** "pushed down" into the WHERE clause of a subquery.
**
** Convert every term that is marked with EP_OuterON and w.iJoin==iTable into
** an ordinary term that omits the EP_OuterON mark.  Or if iTable<0, then
** just clear every EP_OuterON and EP_InnerON mark from the expression tree.
**
** If nullable is true, that means that Expr p might evaluate to NULL even
** if it is a reference to a NOT NULL column.  This can happen, for example,
** if the table that p references is on the left side of a RIGHT JOIN.
** If nullable is true, then take care to not remove the EP_CanBeNull bit.
** See forum thread https://sqlite.org/forum/forumpost/b40696f50145d21c
*/
ساكن فراغ unset_Join_بیان(بیان *p, صحيح i_جدول, صحيح nullable){
  طالما( p ){
    إذا( i_جدول<0 || (بیان_Has_Property(p, EP_OuterON) && p->w.iJoin==i_جدول) ){
      بیان_Clear_Property(p, EP_OuterON|EP_InnerON);
      إذا( i_جدول>=0 ) بیان_سېټ_Property(p, EP_InnerON);
    }
    إذا( p->op==TK_ستنه && p->i_جدول==i_جدول && !nullable ){
      بیان_Clear_Property(p, EP_CanBeNull);
    }
    إذا( p->op==TK_FUNCTION ){
      أكد( بیان_Use_XList(p) );
      أكد( p->p_کیڼ==0 );
      إذا( p->x.pList ){
        صحيح i;
        لكل(i=0; i<p->x.pList->n_بیان; i++){
          unset_Join_بیان(p->x.pList->a[i].p_بیان, i_جدول, nullable);
        }
      }
    }
    unset_Join_بیان(p->p_کیڼ, i_جدول, nullable);
    p = p->p_ښی;
  }
}

/*
** This routine processes the join information for a SELECT statement.
**
**   *  A NATURAL join is converted into a USING join.  After that, we
**      do not need to be concerned with NATURAL joins and we only have
**      think about USING joins.
**
**   *  ON and USING clauses result in extra terms being added to the
**      WHERE clause to enforce the specified constraints.  The extra
**      WHERE clause terms will be tagged with EP_OuterON or
**      EP_InnerON so that we know that they originated in ON/USING.
**
** The terms of a FROM clause are contained in the Select.pSrc structure.
** The left most table is the first entry in Select.pSrc.  The right-most
** table is the last entry.  The join operator is held in the entry to
** the right.  Thus entry 1 contains the join operator for the join between
** entries 0 and 1.  Any ON or USING clauses associated with the join are
** also attached to the right entry.
**
** This routine returns the number of errors encountered.
*/
ساكن صحيح sqlite3ProcessJoin(Parse *pParse, Select *p){
  SrcList *pSrc;                  /* All tables in the FROM clause */
  صحيح i, j;                       /* Loop counters */
  Src_توکی *p_کیڼ;                 /* Left table being joined */
  Src_توکی *p_ښی;                /* Right table being joined */
  صحيح b_کتل_شوی_TFunc = 0;             /* True if have seen table-valued function */

  pSrc = p->pSrc;
  p_کیڼ = &pSrc->a[0];
  p_ښی = &p_کیڼ[1];
  لكل(i=0; i<pSrc->nSrc-1; i++, p_ښی++, p_کیڼ++){
    جدول *p_ښی_Tab = p_ښی->pSTab;
    u32 joinType;

    إذا( NEVER(p_کیڼ->pSTab==0 || p_ښی_Tab==0) ) استمر;
    joinType = (p_ښی->fg.jointype & JT_OUTER)!=0 ? EP_OuterON : EP_InnerON;

    /* If this is a NATURAL join, synthesize an appropriate USING clause
    ** to specify which columns should be joined.
    */
    إذا( p_ښی->fg.jointype & JT_NATURAL ){
      IdList *pUsing = 0;
      إذا( p_ښی->fg.isUsing || p_ښی->u3.pOn ){
        sqlite3_Error_پیغام(pParse, "a NATURAL join may not have "
           "an ON or USING clause", 0);
        أرجع 1;
      }
      لكل(j=0; j<p_ښی_Tab->n_ستون; j++){
        حرف *zName;   /* Name of column in the right table */

        إذا( Is_Hidden_ستنه(&p_ښی_Tab->a_ستون[j]) ) استمر;
        zName = p_ښی_Tab->a_ستون[j].zCnName;
        إذا( جدول_And_ستنه_شاخص(pSrc, 0, i, zName, 0, 0, 1) ){
          pUsing = sqlite3IdListAppend(pParse, pUsing, 0);
          إذا( pUsing ){
            أكد( pUsing->nId>0 );
            أكد( pUsing->a[pUsing->nId-1].zName==0 );
            pUsing->a[pUsing->nId-1].zName = sqlite3DbStrDup(pParse->db, zName);
          }
        }
      }
      إذا( pUsing ){
        p_ښی->fg.isUsing = 1;
        p_ښی->fg.isSynthUsing = 1;
        p_ښی->u3.pUsing = pUsing;
      }
      إذا( pParse->nErr ) أرجع 1;
    }

    /* Create extra terms on the WHERE clause for each column named
    ** in the USING clause.  Example: If the two tables to be joined are
    ** A and B and the USING clause names X, Y, and Z, then add this
    ** to the WHERE clause:    A.X=B.X AND A.Y=B.Y AND A.Z=B.Z
    ** Report an error if any column mentioned in the USING clause is
    ** not contained in both tables to be joined.
    */
    إذا( p_ښی->fg.isUsing ){
      IdList *pList = p_ښی->u3.pUsing;
      sqlite3 *db = pParse->db;
      أكد( pList!=0 );
      لكل(j=0; j<pList->nId; j++){
        حرف *zName;     /* Name of the term in the USING clause */
        صحيح i_کیڼ;       /* Table on the left with matching column name */
        صحيح i_کیڼ_ستون;    /* Column number of matching column on the left */
        صحيح i_ښی_ستون;   /* Column number of matching column on the right */
        بیان *pE1;       /* Reference to the column on the LEFT of the join */
        بیان *pE2;       /* Reference to the column on the RIGHT of the join */
        بیان *pEq;       /* Equality constraint.  pE1 == pE2 */

        zName = pList->a[j].zName;
        i_ښی_ستون = sqlite3_ستنه_شاخص(p_ښی_Tab, zName);
        إذا( i_ښی_ستون<0
         || جدول_And_ستنه_شاخص(pSrc, 0, i, zName, &i_کیڼ, &i_کیڼ_ستون,
                                p_ښی->fg.isSynthUsing)==0
        ){
          sqlite3_Error_پیغام(pParse, "cannot join using column %s - column "
            "not present in both tables", zName);
          أرجع 1;
        }
        pE1 = sqlite3_Create_ستنه_بیان(db, pSrc, i_کیڼ, i_کیڼ_ستون);
        sqlite3_Src_توکی_ستنه_کارول_شوی(&pSrc->a[i_کیڼ], i_کیڼ_ستون);
        إذا( (pSrc->a[0].fg.jointype & JT_LTORJ)!=0 && pParse->nErr==0 ){
          /* This branch runs if the query contains one or more RIGHT or FULL
          ** JOINs.  If only a single table on the left side of this join
          ** contains the zName column, then this branch is a no-op.
          ** But if there are two or more tables on the left side
          ** of the join, construct a coalesce() function that gathers all
          ** such tables.  Raise an error if more than one of those references
          ** to zName is not also within a prior USING clause.
          **
          ** We really ought to raise an error if there are two or more
          ** non-USING references to zName on the left of an INNER or LEFT
          ** JOIN.  But older versions of SQLite do not do that, so we avoid
          ** adding a new error so as to not break legacy applications.
          */
          بیان_List *p_Func_دلیلونه = 0;   /* Arguments to the coalesce() */
          ساكن ثابت Token tkCoalesce = { "coalesce", 8 };
          أكد( pE1!=0 );
          بیان_سېټ_Property(pE1, EP_CanBeNull);
          طالما( جدول_And_ستنه_شاخص(pSrc, i_کیڼ+1, i, zName, &i_کیڼ, &i_کیڼ_ستون,
                                     p_ښی->fg.isSynthUsing)!=0 ){
            إذا( pSrc->a[i_کیڼ].fg.isUsing==0
             || sqlite3_Id_List_شاخص(pSrc->a[i_کیڼ].u3.pUsing, zName)<0
            ){
              sqlite3_Error_پیغام(pParse, "ambiguous reference to %s in USING()",
                              zName);
              اكسر;
            }
            p_Func_دلیلونه = sqlite3_بیان_List_Append(pParse, p_Func_دلیلونه, pE1);
            pE1 = sqlite3_Create_ستنه_بیان(db, pSrc, i_کیڼ, i_کیڼ_ستون);
            sqlite3_Src_توکی_ستنه_کارول_شوی(&pSrc->a[i_کیڼ], i_کیڼ_ستون);
          }
          إذا( p_Func_دلیلونه ){
            p_Func_دلیلونه = sqlite3_بیان_List_Append(pParse, p_Func_دلیلونه, pE1);
            pE1 = sqlite3_بیان_Function(pParse, p_Func_دلیلونه, &tkCoalesce, 0);
            إذا( pE1 ){
              pE1->aff_بیان = SQLITE_AFF_DEFER;
            }
          }
        }وإلا إذا( (pSrc->a[i+1].fg.jointype & JT_کیڼ)!=0 && pParse->nErr==0 ){
          أكد( pE1!=0 );
          بیان_سېټ_Property(pE1, EP_CanBeNull);
        }
        pE2 = sqlite3_Create_ستنه_بیان(db, pSrc, i+1, i_ښی_ستون);
        sqlite3_Src_توکی_ستنه_کارول_شوی(p_ښی, i_ښی_ستون);
        pEq = sqlite3PExpr(pParse, TK_EQ, pE1, pE2);
        أكد( pE2!=0 || pEq==0 );
        إذا( pEq ){
          بیان_سېټ_Property(pEq, joinType);
          أكد( !بیان_Has_Property(pEq, EP_TokenOnly|EP_Reduced) );
          بیان_سېټ_VVAProperty(pEq, EP_NoReduce);
          pEq->w.iJoin = pE2->i_جدول;
        }
        p->pWhere = sqlite3_بیان_And(pParse, p->pWhere, pEq);
      }
    }

    /* Add the ON clause to the end of the WHERE clause, connected by
    ** an AND operator.
    */
    وإلا إذا( p_ښی->u3.pOn ){
      sqlite3_سېټ_Join_بیان(p_ښی->u3.pOn, p_ښی->iCursor, joinType);
      p->pWhere = sqlite3_بیان_And(pParse, p->pWhere, p_ښی->u3.pOn);
      p_ښی->u3.pOn = 0;
      p_ښی->fg.isOn = 1;
      p->selFlags |= SF_On_منزل_Where;
    }

    إذا( b_کتل_شوی_TFunc==0 ){
      b_کتل_شوی_TFunc = (p_کیڼ->fg.isTabFunc && p_کیڼ->u1.pFuncArg);
    }

    /* We also need to call sqlite3SelectCheckOnClauses() to verify that
    ** table-valued function arguments do not illegally refer to any tables to
    ** their right. This test is required if either (a) there is a RIGHT JOIN
    ** in the SrcList, or (b) there is a LEFT JOIN to the right of a 
    ** table-valued function.  */
    إذا( (p_ښی->fg.isTabFunc && joinType==EP_OuterON && p_ښی->u1.pFuncArg)
     || (b_کتل_شوی_TFunc && joinType==EP_OuterON)
    ){
      p->selFlags |= SF_On_منزل_Where;
    }
  }
  أرجع 0;
}

/*
** An instance of this object holds information (beyond pParse and pSelect)
** needed to load the next result row that is to be added to the sorter.
*/
تعريفنوع بنية لیکه_بارول_Info لیکه_بارول_Info;
بنية لیکه_بارول_Info {
  صحيح reg_پایله;               /* Store results in array of registers here */
  u8 ecelFlags;                /* Flag argument to ExprCodeExprList() */
#که_تعریف_شوی SQLITE_ENABLE_SORTER_REFERENCES
  بیان_List *pExtra;            /* Extra columns needed by sorter refs */
  صحيح reg_Extra_پایله;          /* Where to load the extra columns */
#د_که_پای
};

/*
** This routine does the work of loading query data into an array of
** registers so that it can be added to the sorter.
*/
ساكن فراغ inner_Loop_بارول_لیکه(
  Parse *pParse,             /* Statement under construction */
  Select *pSelect,           /* The query being coded */
  لیکه_بارول_Info *pInfo         /* Info needed to complete the row load */
){
  sqlite3_بیان_کوډ_بیان_List(pParse, pSelect->pEList, pInfo->reg_پایله,
                          0, pInfo->ecelFlags);
#که_تعریف_شوی SQLITE_ENABLE_SORTER_REFERENCES
  إذا( pInfo->pExtra ){
    sqlite3_بیان_کوډ_بیان_List(pParse, pInfo->pExtra, pInfo->reg_Extra_پایله, 0, 0);
    sqlite3_بیان_List_Delete(pParse->db, pInfo->pExtra);
  }
#د_که_پای
}

/*
** Code the OP_MakeRecord instruction that generates the entry to be
** added into the sorter.
**
** Return the register in which the result is stored.
*/
ساكن صحيح make_Sorter_ریکارډ(
  Parse *pParse,
  ترتیبول_Ctx *p_ترتیبول,
  Select *pSelect,
  صحيح reg_بنسټ,
  صحيح n_بنسټ
){
  صحيح nOBSat = p_ترتیبول->nOBSat;
  Vdbe *v = pParse->pVdbe;
  صحيح reg_بهر = ++pParse->nMem;
  إذا( p_ترتیبول->p_Deferred_لیکه_بارول ){
    inner_Loop_بارول_لیکه(pParse, pSelect, p_ترتیبول->p_Deferred_لیکه_بارول);
  }
  sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, reg_بنسټ+nOBSat, n_بنسټ-nOBSat, reg_بهر);
  أرجع reg_بهر;
}

/*
** Generate code that will push the record in registers regData
** through regData+nData-1 onto the sorter.
*/
ساكن فراغ ټېل_Onto_Sorter(
  Parse *pParse,         /* Parser context */
  ترتیبول_Ctx *p_ترتیبول,        /* Information about the ORDER BY clause */
  Select *pSelect,       /* The whole SELECT statement */
  صحيح reg_معلومات,           /* First register holding data to be sorted */
  صحيح reg_Orig_معلومات,       /* First register holding data before packing */
  صحيح n_معلومات,             /* Number of elements in the regData data array */
  صحيح n_مختاړی_Reg         /* No. of reg prior to regData available for use */
){
  Vdbe *v = pParse->pVdbe;                         /* Stmt under construction */
  صحيح bSeq = ((p_ترتیبول->ترتیبول_Flags & SORTFLAG_UseSorter)==0);
  صحيح n_بیان = p_ترتیبول->p_ترتیب_By->n_بیان;              /* No. of ORDER BY terms */
  صحيح n_بنسټ = n_بیان + bSeq + n_معلومات;                /* Fields in sorter record */
  صحيح reg_بنسټ;                                     /* Regs for sorter record */
  صحيح reg_ریکارډ = 0;                               /* Assembled sorter record */
  صحيح nOBSat = p_ترتیبول->nOBSat;                      /* ORDER BY terms to skip */
  صحيح op;                            /* Opcode to add sorter record to sorter */
  صحيح i_حد;                        /* LIMIT counter */
  صحيح iSkip = 0;                     /* End of the sorter insert loop */

  أكد( bSeq==0 || bSeq==1 );

  /* Three cases:
  **   (1) The data to be sorted has already been packed into a Record
  **       by a prior OP_MakeRecord.  In this case nData==1 and regData
  **       will be completely unrelated to regOrigData.
  **   (2) All output columns are included in the sort record.  In that
  **       case regData==regOrigData.
  **   (3) Some output columns are omitted from the sort record due to
  **       the SQLITE_ENABLE_SORTER_REFERENCES optimization, or due to the
  **       SQLITE_ECEL_OMITREF optimization, or due to the
  **       SortCtx.pDeferredRowLoad optimization.  In any of these cases
  **       regOrigData is 0 to prevent this routine from trying to copy
  **       values that might not yet exist.
  */
  أكد( n_معلومات==1 || reg_معلومات==reg_Orig_معلومات || reg_Orig_معلومات==0 );

#که_تعریف_شوی SQLITE_ENABLE_STMT_SCANSTATUS
  p_ترتیبول->addr_ټېل = sqlite3VdbeCurrentAddr(v);
#د_که_پای

  إذا( n_مختاړی_Reg ){
    أكد( n_مختاړی_Reg==n_بیان+bSeq );
    reg_بنسټ = reg_معلومات - n_مختاړی_Reg;
  }وإلا{
    reg_بنسټ = pParse->nMem + 1;
    pParse->nMem += n_بنسټ;
  }
  أكد( pSelect->i_دننه_واټن==0 || pSelect->i_حد!=0 );
  i_حد = pSelect->i_دننه_واټن ? pSelect->i_دننه_واټن+1 : pSelect->i_حد;
  p_ترتیبول->label_بشپړ = sqlite3VdbeMakeLabel(pParse);
  sqlite3_بیان_کوډ_بیان_List(pParse, p_ترتیبول->p_ترتیب_By, reg_بنسټ, reg_Orig_معلومات,
                          SQLITE_ECEL_DUP | (reg_Orig_معلومات? SQLITE_ECEL_REF : 0));
  إذا( bSeq ){
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Sequence, p_ترتیبول->iECursor, reg_بنسټ+n_بیان);
  }
  إذا( n_مختاړی_Reg==0 && n_معلومات>0 ){
    sqlite3_بیان_کوډ_Move(pParse, reg_معلومات, reg_بنسټ+n_بیان+bSeq, n_معلومات);
  }
  إذا( nOBSat>0 ){
    صحيح reg_Prev_کلی;   /* The first nOBSat columns of the previous row */
    صحيح addr_لومړی;    /* Address of the OP_IfNot opcode */
    صحيح addrJmp;      /* Address of the OP_Jump opcode */
    VdbeOp *pOp;      /* Opcode that opens the sorter */
    صحيح n_کلی;         /* Number of sorting key columns, including OP_Sequence */
    کلی_Info *pKI;     /* Original KeyInfo on the sorter table */

    reg_ریکارډ = make_Sorter_ریکارډ(pParse, p_ترتیبول, pSelect, reg_بنسټ, n_بنسټ);
    reg_Prev_کلی = pParse->nMem+1;
    pParse->nMem += p_ترتیبول->nOBSat;
    n_کلی = n_بیان - p_ترتیبول->nOBSat + bSeq;
    إذا( bSeq ){
      addr_لومړی = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_IfNot, reg_بنسټ+n_بیان);
    }وإلا{
      addr_لومړی = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_SequenceTest, p_ترتیبول->iECursor);
    }
    VdbeCoverage(v);
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Compare, reg_Prev_کلی, reg_بنسټ, p_ترتیبول->nOBSat);
    pOp = sqlite3VdbeGetOp(v, p_ترتیبول->addr_ترتیبول_شاخص);
    إذا( pParse->db->mallocFailed ) أرجع;
    pOp->p2 = n_کلی + n_معلومات;
    pKI = pOp->p4.p_کلی_Info;
    memset(pKI->a_ترتیبول_Flags, 0, pKI->n_کلی_ډګر); /* Makes OP_Jump testable */
    sqlite3VdbeChangeP4(v, -1, (حرف*)pKI, P4_KEYINFO);
    testcase( pKI->n_All_ډګر > pKI->n_کلی_ډګر+2 );
    pOp->p4.p_کلی_Info = sqlite3_کلی_Info_From_بیان_List(pParse,p_ترتیبول->p_ترتیب_By,nOBSat,
                                           pKI->n_All_ډګر-pKI->n_کلی_ډګر-1);
    pOp = 0; /* Ensure pOp not used after sqlite3VdbeAddOp3() */
    addrJmp = sqlite3VdbeCurrentAddr(v);
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Jump, addrJmp+1, 0, addrJmp+1); VdbeCoverage(v);
    p_ترتیبول->label_Bk_بهر = sqlite3VdbeMakeLabel(pParse);
    p_ترتیبول->regReturn = ++pParse->nMem;
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Gosub, p_ترتیبول->regReturn, p_ترتیبول->label_Bk_بهر);
    sqlite3_Vdbe_جمع_کړه_Op1(v, OP_ResetSorter, p_ترتیبول->iECursor);
    إذا( i_حد ){
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_IfNot, i_حد, p_ترتیبول->label_بشپړ);
      VdbeCoverage(v);
    }
    sqlite3_Vdbe_Jump_دلته(v, addr_لومړی);
    sqlite3_بیان_کوډ_Move(pParse, reg_بنسټ, reg_Prev_کلی, p_ترتیبول->nOBSat);
    sqlite3_Vdbe_Jump_دلته(v, addrJmp);
  }
  إذا( i_حد ){
    /* At this point the values for the new sorter entry are stored
    ** in an array of registers. They need to be composed into a record
    ** and inserted into the sorter if either (a) there are currently
    ** less than LIMIT+OFFSET items or (b) the new record is smaller than
    ** the largest record currently in the sorter. If (b) is true and there
    ** are already LIMIT+OFFSET items in the sorter, delete the largest
    ** entry before inserting the new one. This way there are never more
    ** than LIMIT+OFFSET items in the sorter.
    **
    ** If the new record does not need to be inserted into the sorter,
    ** jump to the next iteration of the loop. If the pSort->labelOBLopt
    ** value is not zero, then it is a label of where to jump.  Otherwise,
    ** just bypass the row insert logic.  See the header comment on the
    ** sqlite3WhereOrderByLimitOptLabel() function for additional info.
    */
    صحيح iCsr = p_ترتیبول->iECursor;
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_IfNotZero, i_حد, sqlite3VdbeCurrentAddr(v)+4);
    VdbeCoverage(v);
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_وروستی, iCsr, 0);
    iSkip = sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_IdxLE,
                                 iCsr, 0, reg_بنسټ+nOBSat, n_بیان-nOBSat);
    VdbeCoverage(v);
    sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Delete, iCsr);
  }
  إذا( reg_ریکارډ==0 ){
    reg_ریکارډ = make_Sorter_ریکارډ(pParse, p_ترتیبول, pSelect, reg_بنسټ, n_بنسټ);
  }
  إذا( p_ترتیبول->ترتیبول_Flags & SORTFLAG_UseSorter ){
    op = OP_Sorter_ورننباسه;
  }وإلا{
    op = OP_Idx_ورننباسه;
  }
  sqlite3_Vdbe_جمع_کړه_Op4_Int(v, op, p_ترتیبول->iECursor, reg_ریکارډ,
                       reg_بنسټ+nOBSat, n_بنسټ-nOBSat);
  إذا( iSkip ){
    sqlite3VdbeChangeP2(v, iSkip,
         p_ترتیبول->labelOBLopt ? p_ترتیبول->labelOBLopt : sqlite3VdbeCurrentAddr(v));
  }
#که_تعریف_شوی SQLITE_ENABLE_STMT_SCANSTATUS
  p_ترتیبول->addr_ټېل_پای_ټکی = sqlite3VdbeCurrentAddr(v)-1;
#د_که_پای
}

/*
** Add code to implement the OFFSET
*/
ساكن فراغ کوډ_دننه_واټن(
  Vdbe *v,          /* Generate code into this VM */
  صحيح i_دننه_واټن,      /* Register holding the offset counter */
  صحيح iContinue     /* Jump here to skip the current record */
){
  إذا( i_دننه_واټن>0 ){
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_IfPos, i_دننه_واټن, iContinue, 1); VdbeCoverage(v);
    VdbeComment((v, "OFFSET"));
  }
}

/*
** Add code that will check to make sure the array of registers starting at
** iMem form a distinct entry. This is used by both "SELECT DISTINCT ..." and
** distinct aggregates ("SELECT count(DISTINCT <expr>) ..."). Three strategies
** are available. Which is used depends on the value of parameter eTnctType,
** as follows:
**
**   WHERE_DISTINCT_UNORDERED/WHERE_DISTINCT_NOOP:
**     Build an ephemeral table that contains all entries seen before and
**     skip entries which have been seen before.
**
**     Parameter iTab is the cursor number of an ephemeral table that must
**     be opened before the VM code generated by this routine is executed.
**     The ephemeral cursor table is queried for a record identical to the
**     record formed by the current array of registers. If one is found,
**     jump to VM address addrRepeat. Otherwise, insert a new record into
**     the ephemeral cursor and proceed.
**
**     The returned value in this case is a copy of parameter iTab.
**
**   WHERE_DISTINCT_ORDERED:
**     In this case rows are being delivered sorted order. The ephemeral
**     table is not required. Instead, the current set of values
**     is compared against previous row. If they match, the new row
**     is not distinct and control jumps to VM address addrRepeat. Otherwise,
**     the VM program proceeds with processing the new row.
**
**     The returned value in this case is the register number of the first
**     in an array of registers used to store the previous result row so that
**     it can be compared to the next. The caller must ensure that this
**     register is initialized to NULL.  (The fixDistinctOpenEph() routine
**     will take care of this initialization.)
**
**   WHERE_DISTINCT_UNIQUE:
**     In this case it has already been determined that the rows are distinct.
**     No special action is required. The return value is zero.
**
** Parameter pEList is the list of expressions used to generated the
** contents of each row. It is used by this routine to determine (a)
** how many elements there are in the array of registers and (b) the
** collation sequences that should be used for the comparisons if
** eTnctType is WHERE_DISTINCT_ORDERED.
*/
ساكن صحيح کوډ_Distinct(
  Parse *pParse,     /* Parsing and code generating context */
  صحيح eTnctType,     /* WHERE_DISTINCT_* value */
  صحيح iTab,          /* A sorting index used to test for distinctness */
  صحيح addrRepeat,    /* Jump to here if not distinct */
  بیان_List *pEList,  /* Expression for each element */
  صحيح regElem        /* First element */
){
  صحيح iRet = 0;
  صحيح n_پایله_ستون = pEList->n_بیان;
  Vdbe *v = pParse->pVdbe;

  بدل( eTnctType ){
    حالة WHERE_DISTINCT_ORDERED: {
      صحيح i;
      صحيح iJump;              /* Jump destination */
      صحيح regPrev;            /* Previous row content */

      /* Allocate space for the previous row */
      iRet = regPrev = pParse->nMem+1;
      pParse->nMem += n_پایله_ستون;

      iJump = sqlite3VdbeCurrentAddr(v) + n_پایله_ستون;
      لكل(i=0; i<n_پایله_ستون; i++){
        CollSeq *pColl = sqlite3_بیان_Coll_Seq(pParse, pEList->a[i].p_بیان);
        إذا( i<n_پایله_ستون-1 ){
          sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Ne, regElem+i, iJump, regPrev+i);
          VdbeCoverage(v);
        }وإلا{
          sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Eq, regElem+i, addrRepeat, regPrev+i);
          VdbeCoverage(v);
         }
        sqlite3VdbeChangeP4(v, -1, (ثابت حرف *)pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
      }
      أكد( sqlite3VdbeCurrentAddr(v)==iJump || pParse->db->mallocFailed );
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Copy, regElem, regPrev, n_پایله_ستون-1);
      اكسر;
    }

    حالة WHERE_DISTINCT_UNIQUE: {
      /* nothing to do */
      اكسر;
    }

    افتراضي: {
      صحيح r1 = sqlite3_Get_لنډمهاله_Reg(pParse);
      sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_وموندل, iTab, addrRepeat, regElem, n_پایله_ستون);
      VdbeCoverage(v);
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, regElem, n_پایله_ستون, r1);
      sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Idx_ورننباسه, iTab, r1, regElem, n_پایله_ستون);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      sqlite3_Release_لنډمهاله_Reg(pParse, r1);
      iRet = iTab;
      اكسر;
    }
  }

  أرجع iRet;
}

/*
** This routine runs after codeDistinct().  It makes necessary
** adjustments to the OP_OpenEphemeral opcode that the codeDistinct()
** routine made use of.  This processing must be done separately since
** sometimes codeDistinct is called before the OP_OpenEphemeral is actually
** laid down.
**
** WHERE_DISTINCT_NOOP:
** WHERE_DISTINCT_UNORDERED:
**
**     No adjustments necessary.  This function is a no-op.
**
** WHERE_DISTINCT_UNIQUE:
**
**     The ephemeral table is not needed.  So change the
**     OP_OpenEphemeral opcode into an OP_Noop.
**
** WHERE_DISTINCT_ORDERED:
**
**     The ephemeral table is not needed.  But we do need register
**     iVal to be initialized to NULL.  So change the OP_OpenEphemeral
**     into an OP_Null on the iVal register.
*/
ساكن فراغ fixDistinctOpenEph(
  Parse *pParse,     /* Parsing and code generating context */
  صحيح eTnctType,     /* WHERE_DISTINCT_* value */
  صحيح iVal,          /* Value returned by codeDistinct() */
  صحيح iOpenEphAddr   /* Address of OP_OpenEphemeral instruction for iTab */
){
  إذا( pParse->nErr==0
   && (eTnctType==WHERE_DISTINCT_UNIQUE || eTnctType==WHERE_DISTINCT_ORDERED)
  ){
    Vdbe *v = pParse->pVdbe;
    sqlite3_Vdbe_Change_منزل_Noop(v, iOpenEphAddr);
    إذا( sqlite3VdbeGetOp(v, iOpenEphAddr+1)->opcode==OP_Explain ){
      sqlite3_Vdbe_Change_منزل_Noop(v, iOpenEphAddr+1);
    }
    إذا( eTnctType==WHERE_DISTINCT_ORDERED ){
      /* Change the OP_OpenEphemeral to an OP_Null that sets the MEM_Cleared
      ** bit on the first register of the previous value.  This will cause the
      ** OP_Ne added in codeDistinct() to always fail on the first iteration of
      ** the loop even if the first row is all NULLs.  */
      VdbeOp *pOp = sqlite3VdbeGetOp(v, iOpenEphAddr);
      pOp->opcode = OP_Null;
      pOp->p1 = 1;
      pOp->p2 = iVal;
    }
  }
}

#که_تعریف_شوی SQLITE_ENABLE_SORTER_REFERENCES
/*
** This function is called as part of inner-loop generation for a SELECT
** statement with an ORDER BY that is not optimized by an index. It
** determines the expressions, if any, that the sorter-reference
** optimization should be used for. The sorter-reference optimization
** is used for SELECT queries like:
**
**   SELECT a, bigblob FROM t1 ORDER BY a LIMIT 10
**
** If the optimization is used for expression "bigblob", then instead of
** storing values read from that column in the sorter records, the PK of
** the row from table t1 is stored instead. Then, as records are extracted from
** the sorter to return to the user, the required value of bigblob is
** retrieved directly from table t1. If the values are very large, this
** can be more efficient than storing them directly in the sorter records.
**
** The ExprList_item.fg.bSorterRef flag is set for each expression in pEList
** for which the sorter-reference optimization should be enabled.
** Additionally, the pSort->aDefer[] array is populated with entries
** for all cursors required to evaluate all selected expressions. Finally.
** output variable (*ppExtra) is set to an expression list containing
** expressions for all extra PK values that should be stored in the
** sorter records.
*/
ساكن فراغ select_بیان_Defer(
  Parse *pParse,                  /* Leave any error here */
  ترتیبول_Ctx *p_ترتیبول,                 /* Sorter context */
  بیان_List *pEList,               /* Expressions destined for sorter */
  بیان_List **ppExtra              /* Expressions to append to sorter record */
){
  صحيح i;
  صحيح nDefer = 0;
  بیان_List *pExtra = 0;
  لكل(i=0; i<pEList->n_بیان; i++){
    بنية بیان_List_توکی *p_توکی = &pEList->a[i];
    إذا( p_توکی->u.x.i_ترتیب_By_ستون==0 ){
      بیان *p_بیان = p_توکی->p_بیان;
      جدول *pTab;
      إذا( p_بیان->op==TK_ستنه
       && p_بیان->i_ستنه>=0
       && ALWAYS( بیان_Use_YTab(p_بیان) )
       && (pTab = p_بیان->y.pTab)!=0
       && Is_Ordinary_جدول(pTab)
       && (pTab->a_ستون[p_بیان->i_ستنه].ستون_Flags & COLFLAG_SORTERREF)!=0
      ){
        صحيح j;
        لكل(j=0; j<nDefer; j++){
          إذا( p_ترتیبول->aDefer[j].iCsr==p_بیان->i_جدول ) اكسر;
        }
        إذا( j==nDefer ){
          إذا( nDefer==Array_اندازه(p_ترتیبول->aDefer) ){
            استمر;
          }وإلا{
            صحيح n_کلی = 1;
            صحيح k;
            شاخص *pPk = 0;
            إذا( !HasRowid(pTab) ){
              pPk = sqlite3_Primary_کلی_شاخص(pTab);
              n_کلی = pPk->n_کلی_ستون;
            }
            لكل(k=0; k<n_کلی; k++){
              بیان *pNew = sqlite3PExpr(pParse, TK_ستنه, 0, 0);
              إذا( pNew ){
                pNew->i_جدول = p_بیان->i_جدول;
                أكد( بیان_Use_YTab(pNew) );
                pNew->y.pTab = p_بیان->y.pTab;
                pNew->i_ستنه = pPk ? pPk->ai_ستنه[k] : -1;
                pExtra = sqlite3_بیان_List_Append(pParse, pExtra, pNew);
              }
            }
            p_ترتیبول->aDefer[nDefer].pTab = p_بیان->y.pTab;
            p_ترتیبول->aDefer[nDefer].iCsr = p_بیان->i_جدول;
            p_ترتیبول->aDefer[nDefer].n_کلی = n_کلی;
            nDefer++;
          }
        }
        p_توکی->fg.bSorterRef = 1;
      }
    }
  }
  p_ترتیبول->nDefer = (u8)nDefer;
  *ppExtra = pExtra;
}
#د_که_پای

#که_تعریف_شوی SQLITE_DEBUG
/*
** This is a byte-code validation check that only runs when SQLITE_DEBUG
** is defined.  This routine looks backwards through the bytecode
** for the definition of cursor with index iCur.  It extracts the KeyInfo from
** that cursor (it must be an index cursor) and verifies that the cursor
** does not use any collating seqeuences other than BINARY for its first
** nCol columns.
**
** This routine is used inside of an assert().  So it should return true
** on success and false if the invariant is not satisfied.
**
** tag-202607231411
*/
ساكن صحيح sqlite3CursorBloomable(Parse *pParse, صحيح iCur, صحيح n_ستون){
  صحيح i,k;
  Vdbe *v = pParse->pVdbe;
  إذا( pParse->nErr ) أرجع 1;
  أكد( v );
  لكل(k=sqlite3VdbeCurrentAddr(v)-1; k>0; k--){
    ثابت VdbeOp *pOp = sqlite3VdbeGetOp(v, k);
    ثابت کلی_Info *p_کلی_Info;
    إذا( pOp->p1!=iCur ) استمر;
    إذا( pOp->opcode!=OP_OpenRead
     && pOp->opcode!=OP_OpenWrite
     && pOp->opcode!=OP_OpenEphemeral
    ){
      استمر;
    }
    أكد( pOp->p4type==P4_KEYINFO );
    p_کلی_Info = (ثابت کلی_Info*)pOp->p4.p_کلی_Info;
    أكد( p_کلی_Info!=0 );
    لكل(i=0; i<n_ستون; i++){
      أكد( sqlite3_Is_دوګونی(p_کلی_Info->aColl[i]) );
    }
    أرجع 1;
  }
  أرجع 0;
}
#د_که_پای /* SQLITE_DEBUG */


/*
** This routine generates the code for the inside of the inner loop
** of a SELECT.
**
** If srcTab is negative, then the p->pEList expressions
** are evaluated in order to get the data for this row.  If srcTab is
** zero or more, then data is pulled from srcTab and p->pEList is used only
** to get the number of columns and the collation sequence for each column.
*/
ساكن فراغ selectInnerLoop(
  Parse *pParse,          /* The parser context */
  Select *p,              /* The complete select statement being coded */
  صحيح srcTab,             /* Pull data from this table if non-negative */
  ترتیبول_Ctx *p_ترتیبول,         /* If not NULL, info on how to process ORDER BY */
  DistinctCtx *pDistinct, /* If not NULL, info on how to process DISTINCT */
  SelectDest *pDest,      /* How to dispose of the results */
  صحيح iContinue,          /* Jump here to continue with next row */
  صحيح iBreak              /* Jump here to break out of the inner loop */
){
  Vdbe *v = pParse->pVdbe;
  صحيح i;
  صحيح hasDistinct;            /* True if the DISTINCT keyword is present */
  صحيح eDest = pDest->eDest;   /* How to dispose of results */
  صحيح iParm = pDest->iSDParm; /* First argument to disposal method */
  صحيح n_پایله_ستون;             /* Number of result columns */
  صحيح n_مختاړی_Reg = 0;         /* Number of extra registers before regResult */
  لیکه_بارول_Info s_لیکه_بارول_Info;   /* Info for deferred row loading */

  /* Usually, regResult is the first cell in an array of memory cells
  ** containing the current result row. In this case regOrig is set to the
  ** same value. However, if the results are being sent to the sorter, the
  ** values for any expressions that are also part of the sort-key are omitted
  ** from this array. In this case regOrig is set to zero.  */
  صحيح reg_پایله;              /* Start of memory holding current results */
  صحيح regOrig;                /* Start of memory holding full result (or 0) */

  أكد( v );
  أكد( p->pEList!=0 );
  hasDistinct = pDistinct ? pDistinct->eTnctType : WHERE_DISTINCT_NOOP;
  إذا( p_ترتیبول && p_ترتیبول->p_ترتیب_By==0 ) p_ترتیبول = 0;
  إذا( p_ترتیبول==0 && !hasDistinct ){
    أكد( iContinue!=0 );
    کوډ_دننه_واټن(v, p->i_دننه_واټن, iContinue);
  }

  /* Pull the requested columns.
  */
  n_پایله_ستون = p->pEList->n_بیان;

  إذا( pDest->iSdst==0 ){
    إذا( p_ترتیبول ){
      n_مختاړی_Reg = p_ترتیبول->p_ترتیب_By->n_بیان;
      إذا( !(p_ترتیبول->ترتیبول_Flags & SORTFLAG_UseSorter) ) n_مختاړی_Reg++;
      pParse->nMem += n_مختاړی_Reg;
    }
    pDest->iSdst = pParse->nMem+1;
    pParse->nMem += n_پایله_ستون;
  }وإلا إذا( pDest->iSdst+n_پایله_ستون > pParse->nMem ){
    /* This is an error condition that can result, for example, when a SELECT
    ** on the right-hand side of an INSERT contains more result columns than
    ** there are columns in the table on the left.  The error will be caught
    ** and reported later.  But we need to make sure enough memory is allocated
    ** to avoid other spurious errors in the meantime. */
    pParse->nMem += n_پایله_ستون;
  }
  pDest->nSdst = n_پایله_ستون;
  regOrig = reg_پایله = pDest->iSdst;
  إذا( srcTab>=0 ){
    لكل(i=0; i<n_پایله_ستون; i++){
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, srcTab, i, reg_پایله+i);
      VdbeComment((v, "%s", p->pEList->a[i].zEName));
    }
  }وإلا إذا( eDest!=SRT_Exists ){
#که_تعریف_شوی SQLITE_ENABLE_SORTER_REFERENCES
    بیان_List *pExtra = 0;
#د_که_پای
    /* If the destination is an EXISTS(...) expression, the actual
    ** values returned by the SELECT are not required.
    */
    u8 ecelFlags;    /* "ecel" is an abbreviation of "ExprCodeExprList" */
    بیان_List *pEList;
    إذا( eDest==SRT_Mem || eDest==SRT_Output || eDest==SRT_Coroutine ){
      ecelFlags = SQLITE_ECEL_DUP;
    }وإلا{
      ecelFlags = 0;
    }
    إذا( p_ترتیبول && hasDistinct==0 && eDest!=SRT_EphemTab && eDest!=SRT_جدول ){
      /* For each expression in p->pEList that is a copy of an expression in
      ** the ORDER BY clause (pSort->pOrderBy), set the associated
      ** iOrderByCol value to one more than the index of the ORDER BY
      ** expression within the sort-key that pushOntoSorter() will generate.
      ** This allows the p->pEList field to be omitted from the sorted record,
      ** saving space and CPU cycles.  */
      ecelFlags |= (SQLITE_ECEL_OMITREF|SQLITE_ECEL_REF);

      لكل(i=p_ترتیبول->nOBSat; i<p_ترتیبول->p_ترتیب_By->n_بیان; i++){
        صحيح j;
        إذا( (j = p_ترتیبول->p_ترتیب_By->a[i].u.x.i_ترتیب_By_ستون)>0 ){
          p->pEList->a[j-1].u.x.i_ترتیب_By_ستون = i+1-p_ترتیبول->nOBSat;
        }
      }
#که_تعریف_شوی SQLITE_ENABLE_SORTER_REFERENCES
      select_بیان_Defer(pParse, p_ترتیبول, p->pEList, &pExtra);
      إذا( pExtra && pParse->db->mallocFailed==0 ){
        /* If there are any extra PK columns to add to the sorter records,
        ** allocate extra memory cells and adjust the OpenEphemeral
        ** instruction to account for the larger records. This is only
        ** required if there are one or more WITHOUT ROWID tables with
        ** composite primary keys in the SortCtx.aDefer[] array.  */
        VdbeOp *pOp = sqlite3VdbeGetOp(v, p_ترتیبول->addr_ترتیبول_شاخص);
        pOp->p2 += (pExtra->n_بیان - p_ترتیبول->nDefer);
        pOp->p4.p_کلی_Info->n_All_ډګر += (pExtra->n_بیان - p_ترتیبول->nDefer);
        pParse->nMem += pExtra->n_بیان;
      }
#د_که_پای

      /* Adjust nResultCol to account for columns that are omitted
      ** from the sorter by the optimizations in this branch */
      pEList = p->pEList;
      لكل(i=0; i<pEList->n_بیان; i++){
        إذا( pEList->a[i].u.x.i_ترتیب_By_ستون>0
#که_تعریف_شوی SQLITE_ENABLE_SORTER_REFERENCES
         || pEList->a[i].fg.bSorterRef
#د_که_پای
        ){
          n_پایله_ستون--;
          regOrig = 0;
        }
      }

      testcase( regOrig );
      testcase( eDest==SRT_سېټ );
      testcase( eDest==SRT_Mem );
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      أكد( eDest==SRT_سېټ || eDest==SRT_Mem
           || eDest==SRT_Coroutine || eDest==SRT_Output
           || eDest==SRT_Upfrom );
    }
    s_لیکه_بارول_Info.reg_پایله = reg_پایله;
    s_لیکه_بارول_Info.ecelFlags = ecelFlags;
#که_تعریف_شوی SQLITE_ENABLE_SORTER_REFERENCES
    s_لیکه_بارول_Info.pExtra = pExtra;
    s_لیکه_بارول_Info.reg_Extra_پایله = reg_پایله + n_پایله_ستون;
    إذا( pExtra ) n_پایله_ستون += pExtra->n_بیان;
#د_که_پای
    إذا( p->i_حد
     && (ecelFlags & SQLITE_ECEL_OMITREF)!=0
     && n_مختاړی_Reg>0
    ){
      أكد( p_ترتیبول!=0 );
      أكد( hasDistinct==0 );
      p_ترتیبول->p_Deferred_لیکه_بارول = &s_لیکه_بارول_Info;
      regOrig = 0;
    }وإلا{
      inner_Loop_بارول_لیکه(pParse, p, &s_لیکه_بارول_Info);
    }
  }

  /* If the DISTINCT keyword was present on the SELECT statement
  ** and this row has been seen before, then do not make this row
  ** part of the result.
  */
  إذا( hasDistinct ){
    صحيح eType = pDistinct->eTnctType;
    صحيح iTab = pDistinct->tabTnct;
    أكد( n_پایله_ستون==p->pEList->n_بیان );
    iTab = کوډ_Distinct(pParse, eType, iTab, iContinue, p->pEList, reg_پایله);
    fixDistinctOpenEph(pParse, eType, iTab, pDistinct->addrTnct);
    إذا( p_ترتیبول==0 ){
      کوډ_دننه_واټن(v, p->i_دننه_واټن, iContinue);
    }
  }

  بدل( eDest ){
    /* Store the result as data using a unique key.
    */
    حالة SRT_Fifo:
    حالة SRT_DistFifo:
    حالة SRT_جدول:
    حالة SRT_EphemTab: {
      صحيح r1 = sqlite3_Get_لنډمهاله_Range(pParse, n_مختاړی_Reg+1);
      testcase( eDest==SRT_جدول );
      testcase( eDest==SRT_EphemTab );
      testcase( eDest==SRT_Fifo );
      testcase( eDest==SRT_DistFifo );
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, reg_پایله, n_پایله_ستون, r1+n_مختاړی_Reg);
#إذا !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      إذا( eDest==SRT_جدول && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_CTE
      إذا( eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index. If the current row is already present
        ** in the index, do not write it to the output. If not, add the
        ** current row to the index and proceed with writing it to the
        ** output table as well.  */
        صحيح addr = sqlite3VdbeCurrentAddr(v) + 4;
        sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_وموندل, iParm+1, addr, r1, 0);
        VdbeCoverage(v);
        sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Idx_ورننباسه, iParm+1, r1,reg_پایله,n_پایله_ستون);
        أكد( p_ترتیبول==0 );
      }
#د_که_پای
      إذا( p_ترتیبول ){
        أكد( reg_پایله==regOrig );
        ټېل_Onto_Sorter(pParse, p_ترتیبول, p, r1+n_مختاړی_Reg, regOrig, 1, n_مختاړی_Reg);
      }وإلا{
        صحيح r2 = sqlite3_Get_لنډمهاله_Reg(pParse);
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_NewRowid, iParm, r2);
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ورننباسه, iParm, r1, r2);
        sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
        sqlite3_Release_لنډمهاله_Reg(pParse, r2);
      }
      sqlite3_Release_لنډمهاله_Range(pParse, r1, n_مختاړی_Reg+1);
      اكسر;
    }

    حالة SRT_Upfrom: {
      إذا( p_ترتیبول ){
        ټېل_Onto_Sorter(
            pParse, p_ترتیبول, p, reg_پایله, regOrig, n_پایله_ستون, n_مختاړی_Reg);
      }وإلا{
        صحيح i2 = pDest->iSDParm2;
        صحيح r1 = sqlite3_Get_لنډمهاله_Reg(pParse);

        /* If the UPDATE FROM join is an aggregate that matches no rows, it
        ** might still be trying to return one row, because that is what
        ** aggregates do.  Don't record that empty row in the output table. */
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_IsNull, reg_پایله, iBreak); VdbeCoverage(v);

        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ,
                          reg_پایله+(i2<0), n_پایله_ستون-(i2<0), r1);
        إذا( i2<0 ){
          sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ورننباسه, iParm, r1, reg_پایله);
        }وإلا{
          sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Idx_ورننباسه, iParm, r1, reg_پایله, i2);
        }
      }
      اكسر;
    }

#که_نه_تعریف_شوی SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)" construct,
    ** then there should be a single item on the stack.  Write this
    ** item into the set table with bogus data.
    */
    حالة SRT_سېټ: {
      إذا( p_ترتیبول ){
        /* At first glance you would think we could optimize out the
        ** ORDER BY in this case since the order of entries in the set
        ** does not matter.  But there might be a LIMIT clause, in which
        ** case the order does matter */
        ټېل_Onto_Sorter(
            pParse, p_ترتیبول, p, reg_پایله, regOrig, n_پایله_ستون, n_مختاړی_Reg);
        pDest->iSDParm2 = 0; /* Signal that any Bloom filter is unpopulated */
      }وإلا{
        صحيح r1 = sqlite3_Get_لنډمهاله_Reg(pParse);
        أكد( sqlite3Strlen30(pDest->zAffSdst)==n_پایله_ستون );
        sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Make_ریکارډ, reg_پایله, n_پایله_ستون,
            r1, pDest->zAffSdst, n_پایله_ستون);
        sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Idx_ورننباسه, iParm, r1, reg_پایله, n_پایله_ستون);
        إذا( pDest->iSDParm2 ){
          أكد( sqlite3CursorBloomable(pParse,iParm,n_پایله_ستون) );
          sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Filter_جمع_کړه, pDest->iSDParm2, 0,
                               reg_پایله, n_پایله_ستون);
          ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
        }
        sqlite3_Release_لنډمهاله_Reg(pParse, r1);
      }
      اكسر;
    }


    /* If any row exist in the result set, record that fact and abort.
    */
    حالة SRT_Exists: {
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, 1, iParm);
      /* The LIMIT clause will terminate the loop for us */
      اكسر;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell or array of
    ** memory cells and break out of the scan loop.
    */
    حالة SRT_Mem: {
      إذا( p_ترتیبول ){
        أكد( n_پایله_ستون<=pDest->nSdst );
        ټېل_Onto_Sorter(
            pParse, p_ترتیبول, p, reg_پایله, regOrig, n_پایله_ستون, n_مختاړی_Reg);
        pDest->iSDParm = reg_پایله;
      }وإلا{
        أكد( n_پایله_ستون==pDest->nSdst );
        إذا( reg_پایله!=iParm ){
          /* This occurs in cases where the SELECT had both a DISTINCT and
          ** an OFFSET clause.  */
          sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Copy, reg_پایله, iParm, n_پایله_ستون-1);
        }
        /* The LIMIT clause will jump out of the loop for us */
      }
      اكسر;
    }
#د_که_پای /* #ifndef SQLITE_OMIT_SUBQUERY */

    حالة SRT_Coroutine:       /* Send data to a co-routine */
    حالة SRT_Output: {        /* Return the results */
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      إذا( p_ترتیبول ){
        ټېل_Onto_Sorter(pParse, p_ترتیبول, p, reg_پایله, regOrig, n_پایله_ستون,
                       n_مختاړی_Reg);
      }وإلا إذا( eDest==SRT_Coroutine ){
        sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Yield, pDest->iSDParm);
      }وإلا{
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_پایله_لیکه, reg_پایله, n_پایله_ستون);
      }
      اكسر;
    }

#که_نه_تعریف_شوی SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    حالة SRT_Dist_کتار:
    حالة SRT_کتار: {
      صحيح n_کلی;
      صحيح r1, r2, r3;
      صحيح addrTest = 0;
      بیان_List *pSO;
      pSO = pDest->p_ترتیب_By;
      أكد( pSO );
      n_کلی = pSO->n_بیان;
      r1 = sqlite3_Get_لنډمهاله_Reg(pParse);
      r2 = sqlite3_Get_لنډمهاله_Range(pParse, n_کلی+2);
      r3 = r2+n_کلی+1;
      إذا( eDest==SRT_Dist_کتار ){
        /* If the destination is DistQueue, then cursor (iParm+1) is open
        ** on a second ephemeral index that holds all values every previously
        ** added to the queue. */
        addrTest = sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_وموندل, iParm+1, 0,
                                        reg_پایله, n_پایله_ستون);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, reg_پایله, n_پایله_ستون, r3);
      إذا( eDest==SRT_Dist_کتار ){
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Idx_ورننباسه, iParm+1, r3);
        sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      }
      لكل(i=0; i<n_کلی; i++){
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_SCopy,
                          reg_پایله + pSO->a[i].u.x.i_ترتیب_By_ستون - 1,
                          r2+i);
      }
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Sequence, iParm, r2+n_کلی);
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, r2, n_کلی+2, r1);
      sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Idx_ورننباسه, iParm, r1, r2, n_کلی+2);
      إذا( addrTest ) sqlite3_Vdbe_Jump_دلته(v, addrTest);
      sqlite3_Release_لنډمهاله_Reg(pParse, r1);
      sqlite3_Release_لنډمهاله_Range(pParse, r2, n_کلی+2);
      اكسر;
    }
#د_که_پای /* SQLITE_OMIT_CTE */



#إذا !defined(SQLITE_OMIT_TRIGGER)
    /* Discard the results.  This is used for SELECT statements inside
    ** the body of a TRIGGER.  The purpose of such selects is to call
    ** user-defined functions that have side effects.  We do not care
    ** about the actual results of the select.
    */
    افتراضي: {
      أكد( eDest==SRT_Discard );
      اكسر;
    }
#د_که_پای
  }

  /* Jump to the end of the loop if the LIMIT is reached.  Except, if
  ** there is a sorter, in which case the sorter has already limited
  ** the output for us.
  */
  إذا( p_ترتیبول==0 && p->i_حد ){
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_DecrJumpZero, p->i_حد, iBreak); VdbeCoverage(v);
  }
}

/*
** Allocate a KeyInfo object sufficient for an index of N key columns and
** X extra columns.
*/
کلی_Info *sqlite3_کلی_Info_Alloc(sqlite3 *db, صحيح N, صحيح X){
  صحيح nExtra = (N+X)*(حجم(CollSeq*)+1);
  کلی_Info *p;
  أكد( X>=0 );
  إذا( NEVER(N+X>0xffff) ) أرجع (کلی_Info*)sqlite3OomFault(db);
  p = sqlite3DbMallocRawNN(db, SZ_KEYINFO(0) + nExtra);
  إذا( p ){
    p->a_ترتیبول_Flags = (u8*)&p->aColl[N+X];
    p->n_کلی_ډګر = (u16)N;
    p->n_All_ډګر = (u16)(N+X);
    p->enc = ENC(db);
    p->db = db;
    p->nRef = 1;
    memset(p->aColl, 0, nExtra);
  }وإلا{
    أرجع (کلی_Info*)sqlite3OomFault(db);
  }
  أرجع p;
}

/*
** Deallocate a KeyInfo object
*/
فراغ sqlite3_کلی_Info_Unref(کلی_Info *p){
  إذا( p ){
    أكد( p->db!=0 );
    أكد( p->nRef>0 );
    p->nRef--;
    إذا( p->nRef==0 ) sqlite3DbNNFreeNN(p->db, p);
  }
}

/*
** Make a new pointer to a KeyInfo object
*/
کلی_Info *sqlite3_کلی_Info_Ref(کلی_Info *p){
  إذا( p ){
    أكد( p->nRef>0 );
    p->nRef++;
  }
  أرجع p;
}

#که_تعریف_شوی SQLITE_DEBUG
/*
** Return TRUE if a KeyInfo object can be change.  The KeyInfo object
** can only be changed if this is just a single reference to the object.
**
** This routine is used only inside of assert() statements.
*/
صحيح sqlite3_کلی_Info_Is_Writeable(کلی_Info *p){ أرجع p->nRef==1; }
#د_که_پای /* SQLITE_DEBUG */

/*
** Given an expression list, generate a KeyInfo structure that records
** the collating sequence for each expression in that expression list.
**
** If the ExprList is an ORDER BY or GROUP BY clause then the resulting
** KeyInfo structure is appropriate for initializing a virtual index to
** implement that clause.  If the ExprList is the result set of a SELECT
** then the KeyInfo structure is appropriate for initializing a virtual
** index to implement a DISTINCT test.
**
** Space to hold the KeyInfo structure is obtained from malloc.  The calling
** function is responsible for seeing that this structure is eventually
** freed.
*/
کلی_Info *sqlite3_کلی_Info_From_بیان_List(
  Parse *pParse,       /* Parsing context */
  بیان_List *pList,     /* Form the KeyInfo object from this ExprList */
  صحيح i_پیل,          /* Begin with this column of pList */
  صحيح nExtra           /* Add this many extra columns to the end */
){
  صحيح n_بیان;
  کلی_Info *pInfo;
  بنية بیان_List_توکی *p_توکی;
  sqlite3 *db = pParse->db;
  صحيح i;

  n_بیان = pList->n_بیان;
  pInfo = sqlite3_کلی_Info_Alloc(db, n_بیان-i_پیل, nExtra+1);
  إذا( pInfo ){
    أكد( sqlite3_کلی_Info_Is_Writeable(pInfo) );
    لكل(i=i_پیل, p_توکی=pList->a+i_پیل; i<n_بیان; i++, p_توکی++){
      pInfo->aColl[i-i_پیل] = sqlite3_بیان_NNColl_Seq(pParse, p_توکی->p_بیان);
      pInfo->a_ترتیبول_Flags[i-i_پیل] = p_توکی->fg.ترتیبول_Flags;
    }
  }
  أرجع pInfo;
}

/*
** Name of the connection operator, used for error messages.
*/
ثابت حرف *sqlite3SelectOpName(صحيح id){
  حرف *z;
  بدل( id ){
    حالة TK_ALL:       z = "UNION ALL";   اكسر;
    حالة TK_INTERSECT: z = "INTERSECT";   اكسر;
    حالة TK_EXCEPT:    z = "EXCEPT";      اكسر;
    افتراضي:           z = "UNION";       اكسر;
  }
  أرجع z;
}

#که_نه_تعریف_شوی SQLITE_OMIT_EXPLAIN
/*
** Unless an "EXPLAIN QUERY PLAN" command is being processed, this function
** is a no-op. Otherwise, it adds a single row of output to the EQP result,
** where the caption is of the form:
**
**   "USE TEMP B-TREE FOR xxx"
**
** where xxx is one of "DISTINCT", "ORDER BY" or "GROUP BY". Exactly which
** is determined by the zUsage argument.
*/
ساكن فراغ explain_لنډمهاله_جدول(Parse *pParse, ثابت حرف *zUsage){
  ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s", zUsage));
}

/*
** Assign expression b to lvalue a. A second, no-op, version of this macro
** is provided when SQLITE_OMIT_EXPLAIN is defined. This allows the code
** in sqlite3Select() to assign values to structure member variables that
** only exist if SQLITE_OMIT_EXPLAIN is not defined without polluting the
** code with #ifndef directives.
*/
# define explain_سېټ_Integer(a, b) a = b

#وإلا
/* No-op versions of the explainXXX() functions and macros. */
# define explain_لنډمهاله_جدول(y,z)
# define explain_سېټ_Integer(y,z)
#د_که_پای


/*
** If the inner loop was generated using a non-null pOrderBy argument,
** then the results were placed in a sorter.  After the loop is terminated
** we need to run the sorter and output the results.  The following
** routine generates the code needed to do that.
*/
ساكن فراغ generate_ترتیبول_لکۍ(
  Parse *pParse,    /* Parsing context */
  Select *p,        /* The SELECT statement */
  ترتیبول_Ctx *p_ترتیبول,   /* Information on the ORDER BY clause */
  صحيح n_ستنه,      /* Number of columns of data */
  SelectDest *pDest /* Write the sorted results here */
){
  Vdbe *v = pParse->pVdbe;                     /* The prepared statement */
  صحيح addrBreak = p_ترتیبول->label_بشپړ;            /* Jump here to exit loop */
  صحيح addrContinue = sqlite3VdbeMakeLabel(pParse);/* Jump here for next cycle */
  صحيح addr;                       /* Top of output loop. Jump for Next. */
  صحيح addrOnce = 0;
  صحيح iTab;
  بیان_List *p_ترتیب_By = p_ترتیبول->p_ترتیب_By;
  صحيح eDest = pDest->eDest;
  صحيح iParm = pDest->iSDParm;
  صحيح reg_لیکه;
  صحيح regRowid;
  صحيح i_ستون;
  صحيح n_کلی;                       /* Number of key columns in sorter record */
  صحيح i_ترتیبول_Tab;                   /* Sorter cursor to read from */
  صحيح i;
  صحيح bSeq;                       /* True if sorter record includes seq. no. */
  صحيح n_Ref_کلی = 0;
  بنية بیان_List_توکی *a_بهر_Ex = p->pEList->a;
#که_تعریف_شوی SQLITE_ENABLE_STMT_SCANSTATUS
  صحيح addrExplain;                /* Address of OP_Explain instruction */
#د_که_پای

  n_کلی = p_ترتیب_By->n_بیان - p_ترتیبول->nOBSat;
  إذا( p_ترتیبول->nOBSat==0 || n_کلی==1 ){
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR %sORDER BY", p_ترتیبول->nOBSat?"LAST TERM OF ":""
    ));
  }وإلا{
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR LAST %d TERMS OF ORDER BY", n_کلی
    ));
  }
  sqlite3_Vdbe_لټون_حالت_Range(v, addrExplain,p_ترتیبول->addr_ټېل,p_ترتیبول->addr_ټېل_پای_ټکی);
  sqlite3_Vdbe_لټون_حالت_Counters(v, addrExplain, addrExplain, p_ترتیبول->addr_ټېل);


  أكد( addrBreak<0 );
  إذا( p_ترتیبول->label_Bk_بهر ){
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Gosub, p_ترتیبول->regReturn, p_ترتیبول->label_Bk_بهر);
    sqlite3VdbeGoto(v, addrBreak);
    sqlite3VdbeResolveLabel(v, p_ترتیبول->label_Bk_بهر);
  }

#که_تعریف_شوی SQLITE_ENABLE_SORTER_REFERENCES
  /* Open any cursors needed for sorter-reference expressions */
  لكل(i=0; i<p_ترتیبول->nDefer; i++){
    جدول *pTab = p_ترتیبول->aDefer[i].pTab;
    صحيح iDb = sqlite3_Schema_منزل_شاخص(pParse->db, pTab->pSchema);
    sqlite3_Open_جدول(pParse, p_ترتیبول->aDefer[i].iCsr, iDb, pTab, OP_OpenRead);
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Rewind, p_ترتیبول->aDefer[i].iCsr,
                      sqlite3VdbeCurrentAddr(v)+1);
    VdbeCoverage(v);
    n_Ref_کلی = MAX(n_Ref_کلی, p_ترتیبول->aDefer[i].n_کلی);
  }
#د_که_پای

  iTab = p_ترتیبول->iECursor;
  إذا( eDest==SRT_Output || eDest==SRT_Coroutine || eDest==SRT_Mem ){
    إذا( eDest==SRT_Mem && p->i_دننه_واټن ){
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Null, 0, pDest->iSdst);
    }
    regRowid = 0;
    reg_لیکه = pDest->iSdst;
  }وإلا{
    regRowid = sqlite3_Get_لنډمهاله_Reg(pParse);
    إذا( eDest==SRT_EphemTab || eDest==SRT_جدول ){
      reg_لیکه = sqlite3_Get_لنډمهاله_Reg(pParse);
      n_ستنه = 0;
    }وإلا{
      reg_لیکه = sqlite3_Get_لنډمهاله_Range(pParse, n_ستنه);
    }
  }
  إذا( p_ترتیبول->ترتیبول_Flags & SORTFLAG_UseSorter ){
    صحيح reg_ترتیبول_بهر = ++pParse->nMem;
    i_ترتیبول_Tab = pParse->nTab++;
    إذا( p_ترتیبول->label_Bk_بهر ){
      addrOnce = sqlite3_Vdbe_جمع_کړه_Op0(v, OP_Once); VdbeCoverage(v);
    }
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_OpenPseudo, i_ترتیبول_Tab, reg_ترتیبول_بهر,
        n_کلی+1+n_ستنه+n_Ref_کلی);
    إذا( addrOnce ) sqlite3_Vdbe_Jump_دلته(v, addrOnce);
    addr = 1 + sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Sorter_ترتیبول, iTab, addrBreak);
    VdbeCoverage(v);
    أكد( p->i_حد==0 && p->i_دننه_واټن==0 );
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Sorter_معلومات, iTab, reg_ترتیبول_بهر, i_ترتیبول_Tab);
    bSeq = 0;
  }وإلا{
    addr = 1 + sqlite3_Vdbe_جمع_کړه_Op2(v, OP_ترتیبول, iTab, addrBreak); VdbeCoverage(v);
    کوډ_دننه_واټن(v, p->i_دننه_واټن, addrContinue);
    i_ترتیبول_Tab = iTab;
    bSeq = 1;
    إذا( p->i_دننه_واټن>0 ){
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_جمع_کړه_Imm, p->i_حد, -1);
    }
  }
  لكل(i=0, i_ستون=n_کلی+bSeq-1; i<n_ستنه; i++){
#که_تعریف_شوی SQLITE_ENABLE_SORTER_REFERENCES
    إذا( a_بهر_Ex[i].fg.bSorterRef ) استمر;
#د_که_پای
    إذا( a_بهر_Ex[i].u.x.i_ترتیب_By_ستون==0 ) i_ستون++;
  }
#که_تعریف_شوی SQLITE_ENABLE_SORTER_REFERENCES
  إذا( p_ترتیبول->nDefer ){
    صحيح i_کلی = i_ستون+1;
    صحيح reg_کلی = sqlite3_Get_لنډمهاله_Range(pParse, n_Ref_کلی);

    لكل(i=0; i<p_ترتیبول->nDefer; i++){
      صحيح iCsr = p_ترتیبول->aDefer[i].iCsr;
      جدول *pTab = p_ترتیبول->aDefer[i].pTab;
      صحيح n_کلی = p_ترتیبول->aDefer[i].n_کلی;

      sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Null_لیکه, iCsr);
      إذا( HasRowid(pTab) ){
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, i_ترتیبول_Tab, i_کلی++, reg_کلی);
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_SeekRowid, iCsr,
            sqlite3VdbeCurrentAddr(v)+1, reg_کلی);
      }وإلا{
        صحيح k;
        صحيح iJmp;
        أكد( sqlite3_Primary_کلی_شاخص(pTab)->n_کلی_ستون==n_کلی );
        لكل(k=0; k<n_کلی; k++){
          sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, i_ترتیبول_Tab, i_کلی++, reg_کلی+k);
        }
        iJmp = sqlite3VdbeCurrentAddr(v);
        sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_SeekGE, iCsr, iJmp+2, reg_کلی, n_کلی);
        sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_IdxLE, iCsr, iJmp+3, reg_کلی, n_کلی);
        sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Null_لیکه, iCsr);
      }
    }
    sqlite3_Release_لنډمهاله_Range(pParse, reg_کلی, n_Ref_کلی);
  }
#د_که_پای
  لكل(i=n_ستنه-1; i>=0; i--){
#که_تعریف_شوی SQLITE_ENABLE_SORTER_REFERENCES
    إذا( a_بهر_Ex[i].fg.bSorterRef ){
      sqlite3_بیان_کوډ(pParse, a_بهر_Ex[i].p_بیان, reg_لیکه+i);
    }وإلا
#د_که_پای
    {
      صحيح iRead;
      إذا( a_بهر_Ex[i].u.x.i_ترتیب_By_ستون ){
        iRead = a_بهر_Ex[i].u.x.i_ترتیب_By_ستون-1;
      }وإلا{
        iRead = i_ستون--;
      }
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, i_ترتیبول_Tab, iRead, reg_لیکه+i);
      VdbeComment((v, "%s", a_بهر_Ex[i].zEName));
    }
  }
  sqlite3_Vdbe_لټون_حالت_Range(v, addrExplain, addrExplain, -1);
  بدل( eDest ){
    حالة SRT_جدول:
    حالة SRT_EphemTab: {
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, i_ترتیبول_Tab, n_کلی+bSeq, reg_لیکه);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_NewRowid, iParm, regRowid);
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ورننباسه, iParm, reg_لیکه, regRowid);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      اكسر;
    }
#که_نه_تعریف_شوی SQLITE_OMIT_SUBQUERY
    حالة SRT_سېټ: {
      أكد( n_ستنه==sqlite3Strlen30(pDest->zAffSdst) );
      sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Make_ریکارډ, reg_لیکه, n_ستنه, regRowid,
                        pDest->zAffSdst, n_ستنه);
      sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Idx_ورننباسه, iParm, regRowid, reg_لیکه, n_ستنه);
      اكسر;
    }
    حالة SRT_Mem: {
      /* The LIMIT clause will terminate the loop for us */
      اكسر;
    }
#د_که_پای
    حالة SRT_Upfrom: {
      صحيح i2 = pDest->iSDParm2;
      صحيح r1 = sqlite3_Get_لنډمهاله_Reg(pParse);
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ,reg_لیکه+(i2<0),n_ستنه-(i2<0),r1);
      إذا( i2<0 ){
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ورننباسه, iParm, r1, reg_لیکه);
      }وإلا{
        sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Idx_ورننباسه, iParm, r1, reg_لیکه, i2);
      }
      اكسر;
    }
    افتراضي: {
      أكد( eDest==SRT_Output || eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      testcase( eDest==SRT_Coroutine );
      إذا( eDest==SRT_Output ){
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_پایله_لیکه, pDest->iSdst, n_ستنه);
      }وإلا{
        sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Yield, pDest->iSDParm);
      }
      اكسر;
    }
  }
  إذا( regRowid ){
    إذا( eDest==SRT_سېټ ){
      sqlite3_Release_لنډمهاله_Range(pParse, reg_لیکه, n_ستنه);
    }وإلا{
      sqlite3_Release_لنډمهاله_Reg(pParse, reg_لیکه);
    }
    sqlite3_Release_لنډمهاله_Reg(pParse, regRowid);
  }
  /* The bottom of the loop
  */
  sqlite3VdbeResolveLabel(v, addrContinue);
  إذا( p_ترتیبول->ترتیبول_Flags & SORTFLAG_UseSorter ){
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Sorter_بل, iTab, addr); VdbeCoverage(v);
  }وإلا{
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_بل, iTab, addr); VdbeCoverage(v);
  }
  sqlite3_Vdbe_لټون_حالت_Range(v, addrExplain, sqlite3VdbeCurrentAddr(v)-1, -1);
  إذا( p_ترتیبول->regReturn ) sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Return, p_ترتیبول->regReturn);
  sqlite3VdbeResolveLabel(v, addrBreak);
}

/*
** Return a pointer to a string containing the 'declaration type' of the
** expression pExpr. The string may be treated as static by the caller.
**
** The declaration type is the exact datatype definition extracted from the
** original CREATE TABLE statement if the expression is a column. The
** declaration type for a ROWID field is INTEGER. Exactly when an expression
** is considered a column can be complex in the presence of subqueries. The
** result-set expression in all of the following SELECT statements is
** considered a column by this function.
**
**   SELECT col FROM tbl;
**   SELECT (SELECT col FROM tbl;
**   SELECT (SELECT col FROM tbl);
**   SELECT abc FROM (SELECT col AS abc FROM tbl);
**
** The declaration type for any expression other than a column is NULL.
**
** This routine has either 3 or 6 parameters depending on whether or not
** the SQLITE_ENABLE_COLUMN_METADATA compile-time option is used.
*/
#که_تعریف_شوی SQLITE_ENABLE_ستنه_METADATA
# define ستنه_Type(A,B,C,D,E) ستنه_Type_Impl(A,B,C,D,E)
#وإلا /* if !defined(SQLITE_ENABLE_COLUMN_METADATA) */
# define ستنه_Type(A,B,C,D,E) ستنه_Type_Impl(A,B)
#د_که_پای
ساكن ثابت حرف *ستنه_Type_Impl(
  NameContext *pNC,
#که_نه_تعریف_شوی SQLITE_ENABLE_ستنه_METADATA
  بیان *p_بیان
#وإلا
  بیان *p_بیان,
  ثابت حرف **pzOrigDb,
  ثابت حرف **pzOrigTab,
  ثابت حرف **pz_Orig_ستون
#د_که_پای
){
  حرف ثابت *zType = 0;
  صحيح j;
#که_تعریف_شوی SQLITE_ENABLE_ستنه_METADATA
  حرف ثابت *zOrigDb = 0;
  حرف ثابت *zOrigTab = 0;
  حرف ثابت *z_Orig_ستون = 0;
#د_که_پای

  أكد( p_بیان!=0 );
  أكد( pNC->pSrcList!=0 );
  بدل( p_بیان->op ){
    حالة TK_ستنه: {
      /* The expression is a column. Locate the table the column is being
      ** extracted from in NameContext.pSrcList. This table may be real
      ** database table or a subquery.
      */
      جدول *pTab = 0;            /* Table structure column is extracted from */
      Select *pS = 0;             /* Select the column is extracted from */
      صحيح i_ستون = p_بیان->i_ستنه;  /* Index of column in pTab */
      طالما( pNC && !pTab ){
        SrcList *pTabList = pNC->pSrcList;
        لكل(j=0;j<pTabList->nSrc && pTabList->a[j].iCursor!=p_بیان->i_جدول;j++);
        إذا( j<pTabList->nSrc ){
          pTab = pTabList->a[j].pSTab;
          إذا( pTabList->a[j].fg.isSubquery ){
            pS = pTabList->a[j].u4.pSubq->pSelect;
          }وإلا{
            pS = 0;
          }
        }وإلا{
          pNC = pNC->p_بل;
        }
      }

      إذا( pTab==0 ){
        /* At one time, code such as "SELECT new.x" within a trigger would
        ** cause this condition to run.  Since then, we have restructured how
        ** trigger code is generated and so this condition is no longer
        ** possible. However, it can still be true for statements like
        ** the following:
        **
        **   CREATE TABLE t1(col INTEGER);
        **   SELECT (SELECT t1.col) FROM FROM t1;
        **
        ** when columnType() is called on the expression "t1.col" in the
        ** sub-select. In this case, set the column type to NULL, even
        ** though it should really be "INTEGER".
        **
        ** This is not a problem, as the column type of "t1.col" is never
        ** used. When columnType() is called on the expression
        ** "(SELECT t1.col)", the correct type is returned (see the TK_SELECT
        ** branch below.  */
        اكسر;
      }

      أكد( pTab && بیان_Use_YTab(p_بیان) && p_بیان->y.pTab==pTab );
      إذا( pS ){
        /* The "table" is actually a sub-select or a view in the FROM clause
        ** of the SELECT statement. Return the declaration type and origin
        ** data for the result-set column of the sub-select.
        */
        إذا( i_ستون<pS->pEList->n_بیان
         && (!ViewCanHaveRowid || i_ستون>=0)
        ){
          /* If iCol is less than zero, then the expression requests the
          ** rowid of the sub-select or view. This expression is legal (see
          ** test case misc2.2.2) - it always evaluates to NULL.
          */
          NameContext sNC;
          بیان *p = pS->pEList->a[i_ستون].p_بیان;
          sNC.pSrcList = pS->pSrc;
          sNC.p_بل = pNC;
          sNC.pParse = pNC->pParse;
          zType = ستنه_Type(&sNC, p,&zOrigDb,&zOrigTab,&z_Orig_ستون);
        }
      }وإلا{
        /* A real table or a CTE table */
        أكد( !pS );
#که_تعریف_شوی SQLITE_ENABLE_ستنه_METADATA
        إذا( i_ستون<0 ) i_ستون = pTab->iPKey;
        أكد( i_ستون==XN_ROWID || (i_ستون>=0 && i_ستون<pTab->n_ستون) );
        إذا( i_ستون<0 ){
          zType = "INTEGER";
          z_Orig_ستون = "rowid";
        }وإلا{
          z_Orig_ستون = pTab->a_ستون[i_ستون].zCnName;
          zType = sqlite3_ستنه_Type(&pTab->a_ستون[i_ستون],0);
        }
        zOrigTab = pTab->zName;
        إذا( pNC->pParse && pTab->pSchema ){
          صحيح iDb = sqlite3_Schema_منزل_شاخص(pNC->pParse->db, pTab->pSchema);
          zOrigDb = pNC->pParse->db->aDb[iDb].zDbSName;
        }
#وإلا
        أكد( i_ستون==XN_ROWID || (i_ستون>=0 && i_ستون<pTab->n_ستون) );
        إذا( i_ستون<0 ){
          zType = "INTEGER";
        }وإلا{
          zType = sqlite3_ستنه_Type(&pTab->a_ستون[i_ستون],0);
        }
#د_که_پای
      }
      اكسر;
    }
#که_نه_تعریف_شوی SQLITE_OMIT_SUBQUERY
    حالة TK_SELECT: {
      /* The expression is a sub-select. Return the declaration type and
      ** origin info for the single column in the result set of the SELECT
      ** statement.
      */
      NameContext sNC;
      Select *pS;
      بیان *p;
      أكد( بیان_Use_XSelect(p_بیان) );
      pS = p_بیان->x.pSelect;
      p = pS->pEList->a[0].p_بیان;
      sNC.pSrcList = pS->pSrc;
      sNC.p_بل = pNC;
      sNC.pParse = pNC->pParse;
      zType = ستنه_Type(&sNC, p, &zOrigDb, &zOrigTab, &z_Orig_ستون);
      اكسر;
    }
#د_که_پای
  }

#که_تعریف_شوی SQLITE_ENABLE_ستنه_METADATA 
  إذا( pzOrigDb ){
    أكد( pzOrigTab && pz_Orig_ستون );
    *pzOrigDb = zOrigDb;
    *pzOrigTab = zOrigTab;
    *pz_Orig_ستون = z_Orig_ستون;
  }
#د_که_پای
  أرجع zType;
}

/*
** Generate code that will tell the VDBE the declaration types of columns
** in the result set.
*/
ساكن فراغ generate_ستنه_Types(
  Parse *pParse,      /* Parser context */
  SrcList *pTabList,  /* List of tables */
  بیان_List *pEList    /* Expressions defining the result set */
){
#که_نه_تعریف_شوی SQLITE_OMIT_DECLTYPE
  Vdbe *v = pParse->pVdbe;
  صحيح i;
  NameContext sNC;
  sNC.pSrcList = pTabList;
  sNC.pParse = pParse;
  sNC.p_بل = 0;
  لكل(i=0; i<pEList->n_بیان; i++){
    بیان *p = pEList->a[i].p_بیان;
    ثابت حرف *zType;
#که_تعریف_شوی SQLITE_ENABLE_ستنه_METADATA
    ثابت حرف *zOrigDb = 0;
    ثابت حرف *zOrigTab = 0;
    ثابت حرف *z_Orig_ستون = 0;
    zType = ستنه_Type(&sNC, p, &zOrigDb, &zOrigTab, &z_Orig_ستون);

    /* The vdbe must make its own copy of the column-type and other
    ** column specific strings, in case the schema is reset before this
    ** virtual machine is deleted.
    */
    sqlite3_Vdbe_سېټ_ستون_Name(v, i, COLNAME_DATABASE, zOrigDb, SQLITE_TRANSIENT);
    sqlite3_Vdbe_سېټ_ستون_Name(v, i, COLNAME_جدول, zOrigTab, SQLITE_TRANSIENT);
    sqlite3_Vdbe_سېټ_ستون_Name(v, i, COLNAME_ستنه, z_Orig_ستون, SQLITE_TRANSIENT);
#وإلا
    zType = ستنه_Type(&sNC, p, 0, 0, 0);
#د_که_پای
    sqlite3_Vdbe_سېټ_ستون_Name(v, i, COLNAME_DECLTYPE, zType, SQLITE_TRANSIENT);
  }
#وإلا
  ناکارول_شوی_PARAMETER(pParse);
  ناکارول_شوی_PARAMETER(pTabList);
  ناکارول_شوی_PARAMETER(pEList);
#د_که_پای /* !defined(SQLITE_OMIT_DECLTYPE) */
}


/*
** Compute the column names for a SELECT statement.
**
** The only guarantee that SQLite makes about column names is that if the
** column has an AS clause assigning it a name, that will be the name used.
** That is the only documented guarantee.  However, countless applications
** developed over the years have made baseless assumptions about column names
** and will break if those assumptions changes.  Hence, use extreme caution
** when modifying this routine to avoid breaking legacy.
**
** See Also: sqlite3ColumnsFromExprList()
**
** The PRAGMA short_column_names and PRAGMA full_column_names settings are
** deprecated.  The default setting is short=ON, full=OFF.  99.9% of all
** applications should operate this way.  Nevertheless, we need to support the
** other modes for legacy:
**
**    short=OFF, full=OFF:      Column name is the text of the expression has it
**                              originally appears in the SELECT statement.  In
**                              other words, the zSpan of the result expression.
**
**    short=ON, full=OFF:       (This is the default setting).  If the result
**                              refers directly to a table column, then the
**                              result column name is just the table column
**                              name: COLUMN.  Otherwise use zSpan.
**
**    full=ON, short=ANY:       If the result refers directly to a table column,
**                              then the result column name with the table name
**                              prefix, ex: TABLE.COLUMN.  Otherwise use zSpan.
*/
فراغ sqlite3_Generate_ستنه_نومونه(
  Parse *pParse,      /* Parser context */
  Select *pSelect     /* Generate column names for this SELECT statement */
){
  Vdbe *v = pParse->pVdbe;
  صحيح i;
  جدول *pTab;
  SrcList *pTabList;
  بیان_List *pEList;
  sqlite3 *db = pParse->db;
  صحيح ډک_Name;    /* TABLE.COLUMN if no AS clause and is a direct table ref */
  صحيح srcName;     /* COLUMN or TABLE.COLUMN if no AS clause and is direct */

  إذا( pParse->ستون_نومونه_سېټ ) أرجع;
  /* Column names are determined by the left-most term of a compound select */
  طالما( pSelect->p_مخکینی ) pSelect = pSelect->p_مخکینی;
  TREETRACE(0x80,pParse,pSelect,("generating column names\n"));
  pTabList = pSelect->pSrc;
  pEList = pSelect->pEList;
  أكد( v!=0 );
  أكد( pTabList!=0 );
  pParse->ستون_نومونه_سېټ = 1;
  ډک_Name = (db->flags & SQLITE_ډک_ستون_نومونه)!=0;
  srcName = (db->flags & SQLITE_Short_ستون_نومونه)!=0 || ډک_Name;
  sqlite3_Vdbe_سېټ_عدد_ستون_شمېر(v, pEList->n_بیان);
  لكل(i=0; i<pEList->n_بیان; i++){
    بیان *p = pEList->a[i].p_بیان;

    أكد( p!=0 );
    أكد( p->op!=TK_AGG_ستنه );  /* Agg processing has not run yet */
    أكد( p->op!=TK_ستنه
        || (بیان_Use_YTab(p) && p->y.pTab!=0) ); /* Covering idx not yet coded */
    إذا( pEList->a[i].zEName && pEList->a[i].fg.eEName==ENAME_NAME ){
      /* An AS clause always takes first priority */
      حرف *zName = pEList->a[i].zEName;
      sqlite3_Vdbe_سېټ_ستون_Name(v, i, COLNAME_NAME, zName, SQLITE_TRANSIENT);
    }وإلا إذا( srcName && p->op==TK_ستنه ){
      حرف *z_ستون;
      صحيح i_ستون = p->i_ستنه;
      pTab = p->y.pTab;
      أكد( pTab!=0 );
      إذا( i_ستون<0 ) i_ستون = pTab->iPKey;
      أكد( i_ستون==-1 || (i_ستون>=0 && i_ستون<pTab->n_ستون) );
      إذا( i_ستون<0 ){
        z_ستون = "rowid";
      }وإلا{
        z_ستون = pTab->a_ستون[i_ستون].zCnName;
      }
      إذا( ډک_Name ){
        حرف *zName = 0;
        zName = sqlite3MPrintf(db, "%s.%s", pTab->zName, z_ستون);
        sqlite3_Vdbe_سېټ_ستون_Name(v, i, COLNAME_NAME, zName, SQLITE_DYNAMIC);
      }وإلا{
        sqlite3_Vdbe_سېټ_ستون_Name(v, i, COLNAME_NAME, z_ستون, SQLITE_TRANSIENT);
      }
    }وإلا{
      ثابت حرف *z = pEList->a[i].zEName;
      z = z==0 ? sqlite3MPrintf(db, "column%d", i+1) : sqlite3DbStrDup(db, z);
      sqlite3_Vdbe_سېټ_ستون_Name(v, i, COLNAME_NAME, z, SQLITE_DYNAMIC);
    }
  }
  generate_ستنه_Types(pParse, pTabList, pEList);
}

/*
** Given an expression list (which is really the list of expressions
** that form the result set of a SELECT statement) compute appropriate
** column names for a table that would hold the expression list.
**
** All column names will be unique.
**
** Only the column names are computed.  Column.zType, Column.zColl,
** and other fields of Column are zeroed.
**
** Return SQLITE_OK on success.  If a memory allocation error occurs,
** store NULL in *paCol and 0 in *pnCol and return SQLITE_NOMEM.
**
** The only guarantee that SQLite makes about column names is that if the
** column has an AS clause assigning it a name, that will be the name used.
** That is the only documented guarantee.  However, countless applications
** developed over the years have made baseless assumptions about column names
** and will break if those assumptions changes.  Hence, use extreme caution
** when modifying this routine to avoid breaking legacy.
**
** See Also: sqlite3GenerateColumnNames()
*/
صحيح sqlite3_Columns_From_بیان_List(
  Parse *pParse,          /* Parsing context */
  بیان_List *pEList,       /* Expr list from which to derive column names */
  i16 *pn_ستون,             /* Write the number of columns here */
  ستنه **pa_ستون          /* Write the new column list here */
){
  sqlite3 *db = pParse->db;   /* Database connection */
  صحيح i, j;                   /* Loop counters */
  u32 cnt;                    /* Index added to make the name unique */
  ستنه *a_ستون, *p_ستون;        /* For looping over result columns */
  صحيح n_ستون;                   /* Number of columns in the result set */
  حرف *zName;                /* Column name */
  صحيح nName;                  /* Size of name in zName[] */
  Hash ht;                    /* Hash table of column names */
  جدول *pTab;

  sqlite3HashInit(&ht);
  إذا( pEList ){
    n_ستون = pEList->n_بیان;
    a_ستون = sqlite3DbMallocZero(db, حجم(a_ستون[0])*n_ستون);
    testcase( a_ستون==0 );
    إذا( n_ستون>32767 ) n_ستون = 32767;
  }وإلا{
    n_ستون = 0;
    a_ستون = 0;
  }
  أكد( n_ستون==(i16)n_ستون );
  *pn_ستون = n_ستون;
  *pa_ستون = a_ستون;

  لكل(i=0, p_ستون=a_ستون; i<n_ستون && !pParse->nErr; i++, p_ستون++){
    بنية بیان_List_توکی *pX = &pEList->a[i];
    بنية بیان_List_توکی *pCollide;
    /* Get an appropriate name for the column
    */
    إذا( (zName = pX->zEName)!=0 && pX->fg.eEName==ENAME_NAME ){
      /* If the column contains an "AS <name>" phrase, use <name> as the name */
    }وإلا{
      بیان *p_ستون_بیان = sqlite3_بیان_Skip_Collate_And_Likely(pX->p_بیان);
      طالما( ALWAYS(p_ستون_بیان!=0) && p_ستون_بیان->op==TK_DOT ){
        p_ستون_بیان = p_ستون_بیان->p_ښی;
        أكد( p_ستون_بیان!=0 );
      }
      إذا( p_ستون_بیان->op==TK_ستنه
       && ALWAYS( بیان_Use_YTab(p_ستون_بیان) )
       && ALWAYS( p_ستون_بیان->y.pTab!=0 )
      ){
        /* For columns use the column name name */
        صحيح i_ستون = p_ستون_بیان->i_ستنه;
        pTab = p_ستون_بیان->y.pTab;
        إذا( i_ستون<0 ) i_ستون = pTab->iPKey;
        zName = i_ستون>=0 ? pTab->a_ستون[i_ستون].zCnName : "rowid";
      }وإلا إذا( p_ستون_بیان->op==TK_ID ){
        أكد( !بیان_Has_Property(p_ستون_بیان, EP_Int_ارزښت) );
        zName = p_ستون_بیان->u.zToken;
      }وإلا{
        /* Use the original text of the column expression as its name */
        أكد( zName==pX->zEName );  /* pointer comparison intended */
      }
    }
    إذا( zName && !sqlite3IsTrueOrFalse(zName) ){
      zName = sqlite3DbStrDup(db, zName);
    }وإلا{
      zName = sqlite3MPrintf(db,"column%d",i+1);
    }

    /* Make sure the column name is unique.  If the name is not unique,
    ** append an integer to the name so that it becomes unique.
    */
    cnt = 0;
    طالما( zName && (pCollide = sqlite3_Hash_پیدا_کړه(&ht, zName))!=0 ){
      إذا( pCollide->fg.b_Using_غړی ){
        p_ستون->ستون_Flags |= COLFLAG_NOEXPAND;
      }
      nName = sqlite3Strlen30(zName);
      إذا( nName>0 ){
        لكل(j=nName-1; j>0 && sqlite3Isdigit(zName[j]); j--){}
        إذا( zName[j]==':' ) nName = j;
      }
      zName = sqlite3MPrintf(db, "%.*z:%u", nName, zName, ++cnt);
      sqlite3_پرمختګ_Check(pParse);
      إذا( cnt>3 ){
        sqlite3_randomness(حجم(cnt), &cnt);
      }
    }
    p_ستون->zCnName = zName;
    p_ستون->hName = sqlite3StrIHash(zName);
    إذا( pX->fg.bNoExpand ){
      p_ستون->ستون_Flags |= COLFLAG_NOEXPAND;
    }
    sqlite3_ستنه_Properties_From_Name(0, p_ستون);
    إذا( zName && sqlite3_Hash_ورننباسه(&ht, zName, pX)==pX ){
      sqlite3OomFault(db);
    }
  }
  sqlite3HashClear(&ht);
  إذا( pParse->nErr ){
    لكل(j=0; j<i; j++){
      sqlite3DbFree(db, a_ستون[j].zCnName);
    }
    sqlite3DbFree(db, a_ستون);
    *pa_ستون = 0;
    *pn_ستون = 0;
    أرجع pParse->rc;
  }
  أرجع SQLITE_OK;
}

/*
** pTab is a transient Table object that represents a subquery of some
** kind (maybe a parenthesized subquery in the FROM clause of a larger
** query, or a VIEW, or a CTE).  This routine computes type information
** for that Table object based on the Select object that implements the
** subquery.  For the purposes of this routine, "type information" means:
**
**    *   The datatype name, as it might appear in a CREATE TABLE statement
**    *   Which collating sequence to use for the column
**    *   The affinity of the column
*/
فراغ sqlite3_Subquery_ستنه_Types(
  Parse *pParse,      /* Parsing contexts */
  جدول *pTab,        /* Add column type information to this table */
  Select *pSelect,    /* SELECT used to determine types and collations */
  حرف aff            /* Default affinity. */
){
  sqlite3 *db = pParse->db;
  ستنه *p_ستون;
  CollSeq *pColl;
  صحيح i,j;
  بیان *p;
  بنية بیان_List_توکی *a;
  NameContext sNC;

  أكد( pSelect!=0 );
  أكد( (pSelect->selFlags & SF_Resolved)!=0 );
  أكد( pTab->n_ستون==pSelect->pEList->n_بیان || pParse->nErr>0 );
  أكد( aff==SQLITE_AFF_NONE || aff==SQLITE_AFF_BLOB );
  إذا( db->mallocFailed || دننه_RENAME_OBJECT ) أرجع;
  طالما( pSelect->p_مخکینی ) pSelect = pSelect->p_مخکینی;
  a = pSelect->pEList->a;
  memset(&sNC, 0, حجم(sNC));
  sNC.pSrcList = pSelect->pSrc;
  لكل(i=0, p_ستون=pTab->a_ستون; i<pTab->n_ستون; i++, p_ستون++){
    ثابت حرف *zType;
    i64 n;
    صحيح m = 0;
    Select *pS2 = pSelect;
    pTab->tabFlags |= (p_ستون->ستون_Flags & COLFLAG_NOINSERT);
    p = a[i].p_بیان;
    /* pCol->szEst = ... // Column size est for SELECT tables never used */
    p_ستون->affinity = sqlite3_بیان_Affinity(p);
    طالما( p_ستون->affinity<=SQLITE_AFF_NONE && pS2->p_بل!=0 ){
      m |= sqlite3_بیان_معلومات_Type(pS2->pEList->a[i].p_بیان);
      pS2 = pS2->p_بل;
      p_ستون->affinity = sqlite3_بیان_Affinity(pS2->pEList->a[i].p_بیان);
    }
    إذا( p_ستون->affinity<=SQLITE_AFF_NONE ){
      p_ستون->affinity = aff;
    }
    إذا( p_ستون->affinity>=SQLITE_AFF_متن && (pS2->p_بل || pS2!=pSelect) ){
      لكل(pS2=pS2->p_بل; pS2; pS2=pS2->p_بل){
        m |= sqlite3_بیان_معلومات_Type(pS2->pEList->a[i].p_بیان);
      }
      إذا( p_ستون->affinity==SQLITE_AFF_متن && (m&0x01)!=0 ){
        p_ستون->affinity = SQLITE_AFF_BLOB;
      }وإلا
      إذا( p_ستون->affinity>=SQLITE_AFF_NUMERIC && (m&0x02)!=0 ){
        p_ستون->affinity = SQLITE_AFF_BLOB;
      }
      إذا( p_ستون->affinity>=SQLITE_AFF_NUMERIC && p->op==TK_CAST ){
        p_ستون->affinity = SQLITE_AFF_FLEXNUM;
      }
    }
    zType = ستنه_Type(&sNC, p, 0, 0, 0);
    إذا( zType==0 || p_ستون->affinity!=sqlite3AffinityType(zType, 0) ){
      إذا( p_ستون->affinity==SQLITE_AFF_NUMERIC
       || p_ستون->affinity==SQLITE_AFF_FLEXNUM
      ){
        zType = "NUM";
      }وإلا{
        zType = 0;
        لكل(j=1; j<SQLITE_N_STDTYPE; j++){
          إذا( sqlite3StdTypeAffinity[j]==p_ستون->affinity ){
            zType = sqlite3StdType[j];
            اكسر;
          }
        }
      }
    }
    إذا( zType ){
      ثابت i64 k = مزي_اوږدوالی(zType);
      n = مزي_اوږدوالی(p_ستون->zCnName);
      p_ستون->zCnName = sqlite3DbReallocOrFree(db, p_ستون->zCnName, n+k+2);
      p_ستون->ستون_Flags &= ~(COLFLAG_HASTYPE|COLFLAG_HASCOLL);
      إذا( p_ستون->zCnName ){
        حافظه_نقل(&p_ستون->zCnName[n+1], zType, k+1);
        p_ستون->ستون_Flags |= COLFLAG_HASTYPE;
      }
    }
    pColl = sqlite3_بیان_Coll_Seq(pParse, p);
    إذا( pColl ){
      أكد( pTab->p_شاخص==0 );
      sqlite3_ستنه_سېټ_Coll(db, p_ستون, pColl->zName);
    }
  }
  pTab->sz_Tab_لیکه = 1; /* Any non-zero value works */
}

/*
** Check the current subquery nesting depth.  Return true and set an
** error if it has gone too deep.
*/
ساكن صحيح check_Subquery_Nesting_ژوروالی(Parse *pParse){
#إذا SQLITE_MAX_بیان_ژوروالی>0
  إذا( pParse->nNestSel >= pParse->db->a_حد[SQLITE_حد_بیان_ژوروالی] ){
    sqlite3_Error_پیغام(pParse, "VIEWs and/or subqueries nested too deep");
    أرجع 1;
  }
#د_که_پای
  أرجع 0;
}

/*
** Given a SELECT statement, generate a Table structure that describes
** the result set of that SELECT.
*/
جدول *sqlite3_پایله_سېټ_د_Select(Parse *pParse, Select *pSelect, حرف aff){
  جدول *pTab;
  sqlite3 *db = pParse->db;
  u64 savedFlags;

  pParse->nNestSel++;
  إذا( check_Subquery_Nesting_ژوروالی(pParse) ) أرجع 0;
  savedFlags = db->flags;
  db->flags &= ~(u64)SQLITE_ډک_ستون_نومونه;
  db->flags |= SQLITE_Short_ستون_نومونه;
  sqlite3SelectPrep(pParse, pSelect, 0);
  db->flags = savedFlags;
  إذا( pParse->nErr ) أرجع 0;
  طالما( pSelect->p_مخکینی ) pSelect = pSelect->p_مخکینی;
  pTab = sqlite3DbMallocZero(db, حجم(جدول) );
  إذا( pTab==0 ){
    أرجع 0;
  }
  pTab->nTabRef = 1;
  pTab->zName = 0;
  pTab->n_لیکه_Log_Est = 200; أكد( 200==sqlite3LogEst(1048576) );
  sqlite3_Columns_From_بیان_List(pParse, pSelect->pEList, &pTab->n_ستون, &pTab->a_ستون);
  sqlite3_Subquery_ستنه_Types(pParse, pTab, pSelect, aff);
  pTab->iPKey = -1;
  إذا( db->mallocFailed ){
    sqlite3_Delete_جدول(db, pTab);
    أرجع 0;
  }
  pParse->nNestSel--;
  أكد( pParse->nNestSel>=0 );
  أرجع pTab;
}

/*
** Get a VDBE for the given parser context.  Create a new one if necessary.
** If an error occurs, return NULL and leave a message in pParse.
*/
Vdbe *sqlite3GetVdbe(Parse *pParse){
  إذا( pParse->pVdbe ){
    أرجع pParse->pVdbe;
  }
  إذا( pParse->pToplevel==0 ){
    pParse->ok_Const_ضریب = 1;
  }
  أرجع sqlite3VdbeCreate(pParse);
}


/*
** Compute the iLimit and iOffset fields of the SELECT based on the
** pLimit expressions.  pLimit->pLeft and pLimit->pRight hold the expressions
** that appear in the original SQL statement after the LIMIT and OFFSET
** keywords.  Or NULL if those keywords are omitted. iLimit and iOffset
** are the integer memory register numbers for counters used to compute
** the limit and offset.  If there is no limit and/or offset, then
** iLimit and iOffset are negative.
**
** This routine changes the values of iLimit and iOffset only if
** a limit or offset is defined by pLimit->pLeft and pLimit->pRight.  iLimit
** and iOffset should have been preset to appropriate default values (zero)
** prior to calling this routine.
**
** The iOffset register (if it exists) is initialized to the value
** of the OFFSET.  The iLimit register is initialized to LIMIT.  Register
** iOffset+1 is initialized to LIMIT+OFFSET.
**
** Only if pLimit->pLeft!=0 do the limit registers get
** redefined.  The UNION ALL operator uses this property to force
** the reuse of the same limit and offset registers across multiple
** SELECT statements.
*/
ساكن فراغ compute_حد_Registers(Parse *pParse, Select *p, صحيح iBreak){
  Vdbe *v = 0;
  صحيح i_حد = 0;
  صحيح i_دننه_واټن;
  صحيح n;
  بیان *p_حد = p->p_حد;

  إذا( p->i_حد ) أرجع;

  /*
  ** "LIMIT -1" always shows all rows.  There is some
  ** controversy about what the correct behavior should be.
  ** The current implementation interprets "LIMIT 0" to mean
  ** no rows.
  */
  إذا( p_حد ){
    أكد( p_حد->op==TK_حد );
    أكد( p_حد->p_کیڼ!=0 );
    p->i_حد = i_حد = ++pParse->nMem;
    v = sqlite3GetVdbe(pParse);
    أكد( v!=0 );
    إذا( sqlite3_بیان_Is_Integer(p_حد->p_کیڼ, &n, pParse, 0) ){
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, n, i_حد);
      VdbeComment((v, "LIMIT counter"));
      إذا( n==0 ){
        sqlite3VdbeGoto(v, iBreak);
      }وإلا إذا( n>=0 && p->n_Select_لیکه>sqlite3LogEst((u64)n) ){
        p->n_Select_لیکه = sqlite3LogEst((u64)n);
        p->selFlags |= SF_Fixed_حد;
      }
    }وإلا{
      sqlite3_بیان_کوډ(pParse, p_حد->p_کیڼ, i_حد);
      sqlite3_Vdbe_جمع_کړه_Op1(v, OP_MustBeInt, i_حد); VdbeCoverage(v);
      VdbeComment((v, "LIMIT counter"));
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_IfNot, i_حد, iBreak); VdbeCoverage(v);
    }
    إذا( p_حد->p_ښی ){
      p->i_دننه_واټن = i_دننه_واټن = ++pParse->nMem;
      pParse->nMem++;   /* Allocate an extra register for limit+offset */
      sqlite3_بیان_کوډ(pParse, p_حد->p_ښی, i_دننه_واټن);
      sqlite3_Vdbe_جمع_کړه_Op1(v, OP_MustBeInt, i_دننه_واټن); VdbeCoverage(v);
      sqlite3MayAbort(pParse);
      VdbeComment((v, "OFFSET counter"));
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_دننه_واټن_حد, i_حد, i_دننه_واټن+1, i_دننه_واټن);
      VdbeComment((v, "LIMIT+OFFSET"));
    }
  }
}

#که_نه_تعریف_شوی SQLITE_OMIT_COMPOUND_SELECT
/*
** Return the appropriate collating sequence for the iCol-th column of
** the result set for the compound-select statement "p".  Return NULL if
** the column has no default collating sequence.
**
** The collating sequence for the compound select is taken from the
** left-most term of the select that has a collating sequence.
*/
ساكن CollSeq *multiSelectCollSeq(Parse *pParse, Select *p, صحيح i_ستون){
  CollSeq *pRet;
  إذا( p->p_مخکینی ){
    pRet = multiSelectCollSeq(pParse, p->p_مخکینی, i_ستون);
  }وإلا{
    pRet = 0;
  }
  أكد( i_ستون>=0 );
  /* iCol must be less than p->pEList->nExpr.  Otherwise an error would
  ** have been thrown during name resolution and we would not have gotten
  ** this far */
  إذا( pRet==0 && ALWAYS(i_ستون<p->pEList->n_بیان) ){
    pRet = sqlite3_بیان_Coll_Seq(pParse, p->pEList->a[i_ستون].p_بیان);
  }
  أرجع pRet;
}

/*
** The select statement passed as the second parameter is a compound SELECT
** with an ORDER BY clause. This function allocates and returns a KeyInfo
** structure suitable for implementing the ORDER BY.
**
** Space to hold the KeyInfo structure is obtained from malloc. The calling
** function is responsible for ensuring that this structure is eventually
** freed.
*/
ساكن کلی_Info *multi_Select_By_یوځای_کلی_Info(Parse *pParse, Select *p, صحيح nExtra){
  بیان_List *p_ترتیب_By = p->p_ترتیب_By;
  صحيح n_ترتیب_By = (p_ترتیب_By!=0) ? p_ترتیب_By->n_بیان : 0;
  sqlite3 *db = pParse->db;
  کلی_Info *pRet = sqlite3_کلی_Info_Alloc(db, n_ترتیب_By+nExtra, 1);
  إذا( pRet ){
    صحيح i;
    لكل(i=0; i<n_ترتیب_By; i++){
      بنية بیان_List_توکی *p_توکی = &p_ترتیب_By->a[i];
      بیان *p_غړی = p_توکی->p_بیان;
      CollSeq *pColl;

      إذا( p_غړی->flags & EP_Collate ){
        pColl = sqlite3_بیان_Coll_Seq(pParse, p_غړی);
      }وإلا{
        pColl = multiSelectCollSeq(pParse, p, p_توکی->u.x.i_ترتیب_By_ستون-1);
        إذا( pColl==0 ) pColl = db->pDfltColl;
        p_ترتیب_By->a[i].p_بیان =
          sqlite3_بیان_جمع_کړه_Collate_String(pParse, p_غړی, pColl->zName);
      }
      أكد( sqlite3_کلی_Info_Is_Writeable(pRet) );
      pRet->aColl[i] = pColl;
      pRet->a_ترتیبول_Flags[i] = p_ترتیب_By->a[i].fg.ترتیبول_Flags;
    }
  }

  أرجع pRet;
}

#که_نه_تعریف_شوی SQLITE_OMIT_CTE
/*
** This routine generates VDBE code to compute the content of a WITH RECURSIVE
** query of the form:
**
**   <recursive-table> AS (<setup-query> UNION [ALL] <recursive-query>)
**                         \___________/             \_______________/
**                           p->pPrior                      p
**
**
** There is exactly one reference to the recursive-table in the FROM clause
** of recursive-query, marked with the SrcList->a[].fg.isRecursive flag.
**
** The setup-query runs once to generate an initial set of rows that go
** into a Queue table.  Rows are extracted from the Queue table one by
** one.  Each row extracted from Queue is output to pDest.  Then the single
** extracted row (now in the iCurrent table) becomes the content of the
** recursive-table for a recursive-query run.  The output of the recursive-query
** is added back into the Queue table.  Then another row is extracted from Queue
** and the iteration continues until the Queue table is empty.
**
** If the compound query operator is UNION then no duplicate rows are ever
** inserted into the Queue table.  The iDistinct table keeps a copy of all rows
** that have ever been inserted into Queue and causes duplicates to be
** discarded.  If the operator is UNION ALL, then duplicates are allowed.
**
** If the query has an ORDER BY, then entries in the Queue table are kept in
** ORDER BY order and the first entry is extracted for each cycle.  Without
** an ORDER BY, the Queue table is just a FIFO.
**
** If a LIMIT clause is provided, then the iteration stops after LIMIT rows
** have been output to pDest.  A LIMIT of zero means to output no rows and a
** negative LIMIT means to output all rows.  If there is also an OFFSET clause
** with a positive value, then the first OFFSET outputs are discarded rather
** than being sent to pDest.  The LIMIT count does not begin until after OFFSET
** rows have been skipped.
*/
ساكن فراغ generateWithRecursiveQuery(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The recursive SELECT to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  SrcList *pSrc = p->pSrc;      /* The FROM clause of the recursive query */
  صحيح n_ستون = p->pEList->n_بیان;  /* Number of columns in the recursive table */
  Vdbe *v = pParse->pVdbe;      /* The prepared statement under construction */
  Select *p_تیاری;               /* The setup query */
  Select *p_لومړی_Rec;            /* Left-most recursive term */
  صحيح addr_سر;                  /* Top of the loop */
  صحيح addrCont, addrBreak;      /* CONTINUE and BREAK addresses */
  صحيح iCurrent = 0;             /* The Current table */
  صحيح regCurrent;               /* Register holding Current table */
  صحيح i_کتار;                   /* The Queue table */
  صحيح iDistinct = 0;            /* To ensure unique results if UNION */
  صحيح eDest = SRT_Fifo;         /* How to write to Queue */
  SelectDest dest_کتار;         /* SelectDest targeting the Queue table */
  صحيح i;                        /* Loop counter */
  صحيح rc;                       /* Result code */
  بیان_List *p_ترتیب_By;           /* The ORDER BY clause */
  بیان *p_حد;                 /* Saved LIMIT and OFFSET */
  صحيح reg_حد, reg_دننه_واټن;      /* Registers used by LIMIT and OFFSET */

#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
  إذا( p->pWin ){
    sqlite3_Error_پیغام(pParse, "cannot use window functions in recursive queries");
    أرجع;
  }
#د_که_پای

  /* Obtain authorization to do a recursive query */
  إذا( sqlite3AuthCheck(pParse, SQLITE_RECURSIVE, 0, 0, 0) ) أرجع;

  /* Process the LIMIT and OFFSET clauses, if they exist */
  addrBreak = sqlite3VdbeMakeLabel(pParse);
  p->n_Select_لیکه = 320;  /* 4 billion rows */
  compute_حد_Registers(pParse, p, addrBreak);
  p_حد = p->p_حد;
  reg_حد = p->i_حد;
  reg_دننه_واټن = p->i_دننه_واټن;
  p->p_حد = 0;
  p->i_حد = p->i_دننه_واټن = 0;
  p_ترتیب_By = p->p_ترتیب_By;

  /* Locate the cursor number of the Current table */
  لكل(i=0; ALWAYS(i<pSrc->nSrc); i++){
    إذا( pSrc->a[i].fg.isRecursive ){
      iCurrent = pSrc->a[i].iCursor;
      اكسر;
    }
  }

  /* Allocate cursors numbers for Queue and Distinct.  The cursor number for
  ** the Distinct table must be exactly one greater than Queue in order
  ** for the SRT_DistFifo and SRT_DistQueue destinations to work. */
  i_کتار = pParse->nTab++;
  إذا( p->op==TK_UNION ){
    eDest = p_ترتیب_By ? SRT_Dist_کتار : SRT_DistFifo;
    iDistinct = pParse->nTab++;
  }وإلا{
    eDest = p_ترتیب_By ? SRT_کتار : SRT_Fifo;
  }
  sqlite3SelectDestInit(&dest_کتار, eDest, i_کتار);

  /* Allocate cursors for Current, Queue, and Distinct. */
  regCurrent = ++pParse->nMem;
  sqlite3_Vdbe_جمع_کړه_Op3(v, OP_OpenPseudo, iCurrent, regCurrent, n_ستون);
  إذا( p_ترتیب_By ){
    کلی_Info *p_کلی_Info = multi_Select_By_یوځای_کلی_Info(pParse, p, 1);
    sqlite3_Vdbe_جمع_کړه_Op4(v, OP_OpenEphemeral, i_کتار, p_ترتیب_By->n_بیان+2, 0,
                      (حرف*)p_کلی_Info, P4_KEYINFO);
    dest_کتار.p_ترتیب_By = p_ترتیب_By;
  }وإلا{
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_OpenEphemeral, i_کتار, n_ستون);
  }
  VdbeComment((v, "Queue table"));
  إذا( iDistinct ){
    /* Generate an ephemeral table used to enforce distinctness on the
    ** output of the recursive part of the CTE.
    */
    کلی_Info *p_کلی_Info;            /* Collating sequence for the result set */
    CollSeq **apColl;             /* For looping through pKeyInfo->aColl[] */

    أكد( p->p_بل==0 );
    أكد( p->pEList!=0 );
    n_ستون = p->pEList->n_بیان;
    p_کلی_Info = sqlite3_کلی_Info_Alloc(pParse->db, n_ستون, 1);
    إذا( p_کلی_Info ){
      لكل(i=0, apColl=p_کلی_Info->aColl; i<n_ستون; i++, apColl++){
        *apColl = multiSelectCollSeq(pParse, p, i);
        إذا( 0==*apColl ){
          *apColl = pParse->db->pDfltColl;
        }
      }
      sqlite3_Vdbe_جمع_کړه_Op4(v, OP_OpenEphemeral, iDistinct, n_ستون, 0,
                           (فراغ*)p_کلی_Info, P4_KEYINFO);
    }وإلا{
      أكد( pParse->nErr>0 );
    }
  }

  /* Detach the ORDER BY clause from the compound SELECT */
  p->p_ترتیب_By = 0;

  /* Figure out how many elements of the compound SELECT are part of the
  ** recursive query.  Make sure no recursive elements use aggregate
  ** functions.  Mark the recursive elements as UNION ALL even if they
  ** are really UNION because the distinctness will be enforced by the
  ** iDistinct table.  pFirstRec is left pointing to the left-most
  ** recursive term of the CTE.
  */
  لكل(p_لومړی_Rec=p; ALWAYS(p_لومړی_Rec!=0); p_لومړی_Rec=p_لومړی_Rec->p_مخکینی){
    إذا( p_لومړی_Rec->selFlags & SF_Aggregate ){
      sqlite3_Error_پیغام(pParse, "recursive aggregate queries not supported");
      اذهب پای_ټکی_د_recursive_query;
    }
    p_لومړی_Rec->op = TK_ALL;
    إذا( (p_لومړی_Rec->p_مخکینی->selFlags & SF_Recursive)==0 ) اكسر;
  }

  /* Store the results of the setup-query in Queue. */
  p_تیاری = p_لومړی_Rec->p_مخکینی;
  p_تیاری->p_بل = 0;
  ExplainQueryPlan((pParse, 1, "SETUP"));
  rc = sqlite3Select(pParse, p_تیاری, &dest_کتار);
  p_تیاری->p_بل = p;
  إذا( rc ) اذهب پای_ټکی_د_recursive_query;

  /* Find the next row in the Queue and output that row */
  addr_سر = sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Rewind, i_کتار, addrBreak); VdbeCoverage(v);

  /* Transfer the next row in Queue over to Current */
  sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Null_لیکه, iCurrent); /* To reset column cache */
  إذا( p_ترتیب_By ){
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, i_کتار, p_ترتیب_By->n_بیان+1, regCurrent);
  }وإلا{
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_لیکه_معلومات, i_کتار, regCurrent);
  }
  sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Delete, i_کتار);

  /* Output the single row in Current */
  addrCont = sqlite3VdbeMakeLabel(pParse);
  کوډ_دننه_واټن(v, reg_دننه_واټن, addrCont);
  selectInnerLoop(pParse, p, iCurrent,
      0, 0, pDest, addrCont, addrBreak);
  إذا( reg_حد ){
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_DecrJumpZero, reg_حد, addrBreak);
    VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, addrCont);

  /* Execute the recursive SELECT taking the single row in Current as
  ** the value for the recursive-table. Store the results in the Queue.
  */
  p_لومړی_Rec->p_مخکینی = 0;
  ExplainQueryPlan((pParse, 1, "RECURSIVE STEP"));
  sqlite3Select(pParse, p, &dest_کتار);
  أكد( p_لومړی_Rec->p_مخکینی==0 );
  p_لومړی_Rec->p_مخکینی = p_تیاری;

  /* Keep running the loop until the Queue is empty */
  sqlite3VdbeGoto(v, addr_سر);
  sqlite3VdbeResolveLabel(v, addrBreak);

پای_ټکی_د_recursive_query:
  sqlite3_بیان_List_Delete(pParse->db, p->p_ترتیب_By);
  p->p_ترتیب_By = p_ترتیب_By;
  p->p_حد = p_حد;
  أرجع;
}
#د_که_پای /* SQLITE_OMIT_CTE */

/* Forward references */
ساكن صحيح multi_Select_By_یوځای(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
);

/*
** Handle the special case of a compound-select that originates from a
** VALUES clause.  By handling this as a special case, we avoid deep
** recursion, and thus do not need to enforce the SQLITE_LIMIT_COMPOUND_SELECT
** on a VALUES clause.
**
** Because the Select object originates from a VALUES clause:
**   (1) There is no LIMIT or OFFSET or else there is a LIMIT of exactly 1
**   (2) All terms are UNION ALL
**   (3) There is no ORDER BY clause
**
** The "LIMIT of exactly 1" case of condition (1) comes about when a VALUES
** clause occurs within scalar expression (ex: "SELECT (VALUES(1),(2),(3))").
** The sqlite3CodeSubselect will have added the LIMIT 1 clause in tht case.
** Since the limit is exactly 1, we only need to evaluate the left-most VALUES.
*/
ساكن صحيح multi_Select_ارزښتونه(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  صحيح n_لیکه = 1;
  صحيح rc = 0;
  صحيح bShowAll = p->p_حد==0;
  أكد( p->selFlags & SF_Multi_ارزښت );
  افعل{
    أكد( p->selFlags & SF_ارزښتونه );
    أكد( p->op==TK_ALL || (p->op==TK_SELECT && p->p_مخکینی==0) );
    أكد( p->p_بل==0 || p->pEList->n_بیان==p->p_بل->pEList->n_بیان );
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
    إذا( p->pWin ) أرجع -1;
#د_که_پای
    إذا( p->p_مخکینی==0 ) اكسر;
    أكد( p->p_مخکینی->p_بل==p );
    p = p->p_مخکینی;
    n_لیکه += bShowAll;
  }طالما(1);
  ExplainQueryPlan((pParse, 0, "SCAN %d CONSTANT ROW%s", n_لیکه,
                    n_لیکه==1 ? "" : "S"));
  طالما( p ){
    selectInnerLoop(pParse, p, -1, 0, 0, pDest, 1, 1);
    إذا( !bShowAll ) اكسر;
    p->n_Select_لیکه = n_لیکه;
    p = p->p_بل;
  }
  أرجع rc;
}

/*
** Return true if the SELECT statement which is known to be the recursive
** part of a recursive CTE still has its anchor terms attached.  If the
** anchor terms have already been removed, then return false.
*/
ساكن صحيح hasAnchor(Select *p){
  طالما( p && (p->selFlags & SF_Recursive)!=0 ){ p = p->p_مخکینی; }
  أرجع p!=0;
}

/*
** Return TRUE if p is a UNION with a LIMIT of exactly 1 and no OFFSET
** clause.  This is a precondition for a couple of related optimizations.
**
** False negatives are harmless (apart from resulting in a slower query).
** False positives can result in incorrect answers, however.  To provoke
** false negatives for testing purposes, disable the SQLITE_UnionLimit
** optimization.
*/
ساكن صحيح union_With_حد_One(sqlite3 *db, Select *p){
  صحيح v;
  إذا( p->op!=TK_UNION ) أرجع 0;
  إذا( p->p_حد==0 ) أرجع 0;
  إذا( p->p_حد->p_ښی ) أرجع 0;  /* No OFFSET */
  v = 0;
  أكد( p->p_حد->p_کیڼ!=0 );
  إذا( sqlite3_بیان_Is_Integer(p->p_حد->p_کیڼ, &v, 0, 0)==0 ) أرجع 0;
  إذا( v!=1 ) أرجع 0;
  إذا( OptimizationDisabled(db, SQLITE_Union_حد) ) أرجع 0;
  أرجع 1;
}

/*
** This routine is called to process a compound query formed from
** two or more separate queries using UNION, UNION ALL, EXCEPT, or
** INTERSECT
**
** "p" points to the right-most of the two queries.  The query on the
** left is p->pPrior.  The left query could also be a compound query
** in which case this routine will be called recursively.
**
** The results of the total query are to be written into a destination
** of type eDest with parameter iParm.
**
** Example 1:  Consider a three-way compound SQL statement.
**
**     SELECT a FROM t1 UNION SELECT b FROM t2 UNION SELECT c FROM t3
**
** This statement is parsed up as follows:
**
**     SELECT c FROM t3
**      |
**      `----->  SELECT b FROM t2
**                |
**                `------>  SELECT a FROM t1
**
** The arrows in the diagram above represent the Select.pPrior pointer.
** So if this routine is called with p equal to the t3 query, then
** pPrior will be the t2 query.  p->op will be TK_UNION in this case.
**
** Notice that because of the way SQLite parses compound SELECTs, the
** individual selects always group from left to right.
*/
ساكن صحيح multiSelect(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  صحيح rc = SQLITE_OK;   /* Success code from a subroutine */
  Select *p_مخکینی;       /* Another SELECT immediately to our left */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest dest;      /* Alternative data destination */
  Select *pDelete = 0;  /* Chain of simple selects to delete */
  sqlite3 *db;          /* Database connection */

  /* Make sure there is no ORDER BY or LIMIT clause on prior SELECTs.  Only
  ** the last (right-most) SELECT in the series may have an ORDER BY or LIMIT.
  */
  أكد( p && p->p_مخکینی );  /* Calling function guarantees this much */
  أكد( (p->selFlags & SF_Recursive)==0 || p->op==TK_ALL || p->op==TK_UNION );
  أكد( p->selFlags & SF_Compound );
  db = pParse->db;
  p_مخکینی = p->p_مخکینی;
  dest = *pDest;
  أكد( p_مخکینی->p_ترتیب_By==0 );
  أكد( p_مخکینی->p_حد==0 );

  v = sqlite3GetVdbe(pParse);
  أكد( v!=0 );  /* The VDBE already created by calling function */

  /* Create the destination temporary table if necessary
  */
  إذا( dest.eDest==SRT_EphemTab ){
    أكد( p->pEList );
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_OpenEphemeral, dest.iSDParm, p->pEList->n_بیان);
    dest.eDest = SRT_جدول;
  }

  /* Special handling for a compound-select that originates as a VALUES clause.
  */
  إذا( p->selFlags & SF_Multi_ارزښت ){
    rc = multi_Select_ارزښتونه(pParse, p, &dest);
    إذا( rc>=0 ) اذهب multi_select_پای_ټکی;
    rc = SQLITE_OK;
  }

  /* Make sure all SELECTs in the statement have the same number of elements
  ** in their result sets.
  */
  أكد( p->pEList && p_مخکینی->pEList );
  أكد( p->pEList->n_بیان==p_مخکینی->pEList->n_بیان );

#که_نه_تعریف_شوی SQLITE_OMIT_CTE
  إذا( (p->selFlags & SF_Recursive)!=0 && hasAnchor(p) ){
    generateWithRecursiveQuery(pParse, p, &dest);
  }وإلا
#د_که_پای
  إذا( p->p_ترتیب_By ){
    /* If the compound has an ORDER BY clause, then always use the merge
    ** algorithm. */
    أرجع multi_Select_By_یوځای(pParse, p, pDest);
  }وإلا إذا( p->op!=TK_ALL && !union_With_حد_One(db,p) ){
    /* If the compound is EXCEPT, INTERSECT, or UNION (anything other than
    ** UNION ALL) then also always use the merge algorithm.  However, the
    ** multiSelectByMerge() routine requires that the compound have an
    ** ORDER BY clause, and it doesn't right now.  So invent one first. */
    بیان *pOne = sqlite3_بیان_Int32(db, 1);
    p->p_ترتیب_By = sqlite3_بیان_List_Append(pParse, 0, pOne);
    إذا( pParse->nErr ) اذهب multi_select_پای_ټکی;
    أكد( p->p_ترتیب_By!=0 );
    p->p_ترتیب_By->a[0].u.x.i_ترتیب_By_ستون = 1;
    أرجع multi_Select_By_یوځای(pParse, p, pDest);
  }وإلا{
    /* For a UNION ALL compound without ORDER BY, or for a UNION with a 
    ** LIMIT of exactly 1 and no OFFSET and no ORDER BY, simply run the left
    ** query, then run the right query */
    صحيح addr = 0;
    صحيح n_حد = 0;  /* Initialize to suppress harmless compiler warning */

#که_نه_تعریف_شوی SQLITE_OMIT_EXPLAIN
    إذا( p_مخکینی->p_مخکینی==0 ){
      ExplainQueryPlan((pParse, 1, "COMPOUND QUERY"));
      ExplainQueryPlan((pParse, 1, "LEFT-MOST SUBQUERY"));
    }
#د_که_پای
    أكد( !p_مخکینی->p_حد );
    p_مخکینی->i_حد = p->i_حد;
    p_مخکینی->i_دننه_واټن = p->i_دننه_واټن;
    p_مخکینی->p_حد = sqlite3_بیان_Dup(db, p->p_حد, 0);
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL left...\n"));
    rc = sqlite3Select(pParse, p_مخکینی, &dest);
    sqlite3_بیان_Delete(db, p_مخکینی->p_حد);
    p_مخکینی->p_حد = 0;
    إذا( rc ){
      اذهب multi_select_پای_ټکی;
    }
    p->p_مخکینی = 0;
    p->i_حد = p_مخکینی->i_حد;
    p->i_دننه_واټن = p_مخکینی->i_دننه_واټن;
    إذا( p->i_حد ){
      addr = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_IfNot, p->i_حد); VdbeCoverage(v);
      VdbeComment((v, "Jump ahead if LIMIT reached"));
      إذا( p->i_دننه_واټن ){
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_دننه_واټن_حد,
                          p->i_حد, p->i_دننه_واټن+1, p->i_دننه_واټن);
      }
    }
    ExplainQueryPlan((pParse, 1, "UNION ALL"));
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL right...\n"));
    rc = sqlite3Select(pParse, p, &dest);
    testcase( rc!=SQLITE_OK );
    pDelete = p->p_مخکینی;
    p->p_مخکینی = p_مخکینی;
    p->n_Select_لیکه = sqlite3_Log_Est_جمع_کړه(p->n_Select_لیکه, p_مخکینی->n_Select_لیکه);
    إذا( p->p_حد
     && sqlite3_بیان_Is_Integer(p->p_حد->p_کیڼ, &n_حد, pParse, 0)
     && n_حد>0 && p->n_Select_لیکه > sqlite3LogEst((u64)n_حد)
    ){
      p->n_Select_لیکه = sqlite3LogEst((u64)n_حد);
    }
    إذا( addr ){
      sqlite3_Vdbe_Jump_دلته(v, addr);
    }
#که_نه_تعریف_شوی SQLITE_OMIT_EXPLAIN
    إذا( p->p_بل==0 ){
      Explain_Query_Plan_راوباسه(pParse);
    }
#د_که_پای
  }

multi_select_پای_ټکی:
  pDest->iSdst = dest.iSdst;
  pDest->nSdst = dest.nSdst;
  pDest->iSDParm2 = dest.iSDParm2;
  إذا( pDelete ){
    sqlite3_Parser_جمع_کړه_Cleanup(pParse, sqlite3SelectDeleteGeneric, pDelete);
  }
  أرجع rc;
}
#د_که_پای /* SQLITE_OMIT_COMPOUND_SELECT */

/*
** Error message for when two or more terms of a compound select have different
** size result sets.
*/
فراغ sqlite3_Select_Wrong_عدد_Terms_Error(Parse *pParse, Select *p){
  إذا( p->selFlags & SF_ارزښتونه ){
    sqlite3_Error_پیغام(pParse, "all VALUES must have the same number of terms");
  }وإلا{
    sqlite3_Error_پیغام(pParse, "SELECTs to the left and right of %s"
      " do not have the same number of result columns",
      sqlite3SelectOpName(p->op));
  }
}

/*
** Code an output subroutine for a coroutine implementation of a
** SELECT statement.
**
** The data to be output is contained in an array of pIn->nSdst registers
** starting at register pIn->iSdst.  pDest is where the output should
** be sent.
**
** regReturn is the number of the register holding the subroutine
** return address.
**
** If regPrev>0 then it is the first register in a vector that
** records the previous output.  mem[regPrev] is a flag that is false
** if there has been no previous output.  If regPrev>0 then code is
** generated to suppress duplicates.  pKeyInfo is used for comparing
** keys.
**
** If the LIMIT found in p->iLimit is reached, jump immediately to
** iBreak.
*/
ساكن صحيح generateOutputSubroutine(
  Parse *pParse,          /* Parsing context */
  Select *p,              /* The SELECT statement */
  SelectDest *p_دننه,        /* Coroutine supplying data */
  SelectDest *pDest,      /* Where to send the data */
  صحيح regReturn,          /* The return address register */
  صحيح regPrev,            /* Previous result register.  No uniqueness if 0 */
  کلی_Info *p_کلی_Info,      /* For comparing with previous entry */
  صحيح iBreak              /* Jump here if we hit the LIMIT */
){
  Vdbe *v = pParse->pVdbe;
  صحيح iContinue;
  صحيح addr;

  أكد( p_دننه->eDest==SRT_Coroutine );

  addr = sqlite3VdbeCurrentAddr(v);
  iContinue = sqlite3VdbeMakeLabel(pParse);

  /* Suppress duplicates for UNION, EXCEPT, and INTERSECT
  */
  إذا( regPrev ){
    صحيح addr1, addr2;
    addr1 = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_IfNot, regPrev); VdbeCoverage(v);
    addr2 = sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Compare, p_دننه->iSdst, regPrev+1, p_دننه->nSdst,
                              (حرف*)sqlite3_کلی_Info_Ref(p_کلی_Info), P4_KEYINFO);
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Jump, addr2+2, iContinue, addr2+2); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_دلته(v, addr1);
    sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Copy, p_دننه->iSdst, regPrev+1, p_دننه->nSdst-1);
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, 1, regPrev);
  }
  إذا( pParse->db->mallocFailed ) أرجع 0;

  /* Suppress the first OFFSET entries if there is an OFFSET clause
  */
  کوډ_دننه_واټن(v, p->i_دننه_واټن, iContinue);

  بدل( pDest->eDest ){
    /* Store the result as data using a unique key.
    */
    حالة SRT_Fifo:
    حالة SRT_DistFifo:
    حالة SRT_جدول:
    حالة SRT_EphemTab: {
      صحيح r1 = sqlite3_Get_لنډمهاله_Reg(pParse);
      صحيح r2 = sqlite3_Get_لنډمهاله_Reg(pParse);
      صحيح iParm = pDest->iSDParm;
      testcase( pDest->eDest==SRT_جدول );
      testcase( pDest->eDest==SRT_EphemTab );
      testcase( pDest->eDest==SRT_Fifo );
      testcase( pDest->eDest==SRT_DistFifo );
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, p_دننه->iSdst, p_دننه->nSdst, r1);
#إذا !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      إذا( pDest->eDest==SRT_جدول && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_CTE
      إذا( pDest->eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index that is used to enforce uniqueness on the
        ** total result.  At this point, we are processing the setup portion
        ** of the recursive CTE using the merge algorithm, so the results are
        ** guaranteed to be unique anyhow.  But we still need to populate the
        ** (iParm+1) cursor for use by the subsequent recursive phase.
        */
        sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Idx_ورننباسه, iParm+1, r1,
                             p_دننه->iSdst, p_دننه->nSdst);
      }
#د_که_پای
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_NewRowid, iParm, r2);
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ورننباسه, iParm, r1, r2);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      sqlite3_Release_لنډمهاله_Reg(pParse, r2);
      sqlite3_Release_لنډمهاله_Reg(pParse, r1);
      اكسر;
    }

    /* If any row exist in the result set, record that fact and abort.
    */
    حالة SRT_Exists: {
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, 1, pDest->iSDParm);
      /* The LIMIT clause will terminate the loop for us */
      اكسر;
    }

#که_نه_تعریف_شوی SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)".
    */
    حالة SRT_سېټ: {
      صحيح r1;
      testcase( p_دننه->nSdst>1 );
      r1 = sqlite3_Get_لنډمهاله_Reg(pParse);
      sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Make_ریکارډ, p_دننه->iSdst, p_دننه->nSdst,
          r1, pDest->zAffSdst, p_دننه->nSdst);
      sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Idx_ورننباسه, pDest->iSDParm, r1,
                           p_دننه->iSdst, p_دننه->nSdst);
      إذا( pDest->iSDParm2>0 ){
        أكد( sqlite3CursorBloomable(pParse, pDest->iSDParm, p_دننه->nSdst) );
        sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Filter_جمع_کړه, pDest->iSDParm2, 0,
                             p_دننه->iSdst, p_دننه->nSdst);
        ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
      }
      sqlite3_Release_لنډمهاله_Reg(pParse, r1);
      اكسر;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell and break out
    ** of the scan loop.  Note that the select might return multiple columns
    ** if it is the RHS of a row-value IN operator.
    */
    حالة SRT_Mem: {
      testcase( p_دننه->nSdst>1 );
      sqlite3_بیان_کوډ_Move(pParse, p_دننه->iSdst, pDest->iSDParm, p_دننه->nSdst);
      /* The LIMIT clause will jump out of the loop for us */
      اكسر;
    }
#د_که_پای /* #ifndef SQLITE_OMIT_SUBQUERY */

    /* The results are stored in a sequence of registers
    ** starting at pDest->iSdst.  Then the co-routine yields.
    */
    حالة SRT_Coroutine: {
      إذا( pDest->iSdst==0 ){
        pDest->iSdst = sqlite3_Get_لنډمهاله_Range(pParse, p_دننه->nSdst);
        pDest->nSdst = p_دننه->nSdst;
      }
      sqlite3_بیان_کوډ_Move(pParse, p_دننه->iSdst, pDest->iSdst, p_دننه->nSdst);
      sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Yield, pDest->iSDParm);
      اكسر;
    }

#که_نه_تعریف_شوی SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    حالة SRT_Dist_کتار:
    حالة SRT_کتار: {
      صحيح n_کلی;
      صحيح r1, r2, r3, ii;
      بیان_List *pSO;
      صحيح iParm = pDest->iSDParm;
      pSO = pDest->p_ترتیب_By;
      أكد( pSO );
      n_کلی = pSO->n_بیان;
      r1 = sqlite3_Get_لنډمهاله_Reg(pParse);
      r2 = sqlite3_Get_لنډمهاله_Range(pParse, n_کلی+2);
      r3 = r2+n_کلی+1;

      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, p_دننه->iSdst, p_دننه->nSdst, r3);
      إذا( pDest->eDest==SRT_Dist_کتار ){
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Idx_ورننباسه, iParm+1, r3);
      }
      لكل(ii=0; ii<n_کلی; ii++){
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_SCopy,
                          p_دننه->iSdst + pSO->a[ii].u.x.i_ترتیب_By_ستون - 1,
                          r2+ii);
      }
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Sequence, iParm, r2+n_کلی);
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, r2, n_کلی+2, r1);
      sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Idx_ورننباسه, iParm, r1, r2, n_کلی+2);
      sqlite3_Release_لنډمهاله_Reg(pParse, r1);
      sqlite3_Release_لنډمهاله_Range(pParse, r2, n_کلی+2);
      اكسر;
    }
#د_که_پای /* SQLITE_OMIT_CTE */

    /* Ignore the output */
    حالة SRT_Discard: {
      اكسر;
    }

    /* If none of the above, then the result destination must be
    ** SRT_Output.
    **
    ** For SRT_Output, results are stored in a sequence of registers. 
    ** Then the OP_ResultRow opcode is used to cause sqlite3_step() to
    ** return the next row of result.
    */
    افتراضي: {
      أكد( pDest->eDest==SRT_Output );
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_پایله_لیکه, p_دننه->iSdst, p_دننه->nSdst);
      اكسر;
    }
  }

  /* Jump to the end of the loop if the LIMIT is reached.
  */
  إذا( p->i_حد ){
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_DecrJumpZero, p->i_حد, iBreak); VdbeCoverage(v);
  }

  /* Generate the subroutine return
  */
  sqlite3VdbeResolveLabel(v, iContinue);
  sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Return, regReturn);

  أرجع addr;
}

/*
** Generate code for a compound SELECT statement using a merge
** algorithm.  The compound must have an ORDER BY clause for this
** to work.
**
** We assume a query of the following form:
**
**      <selectA>  <operator>  <selectB>  ORDER BY <orderbylist>
**
** <operator> is one of UNION ALL, UNION, EXCEPT, or INTERSECT.  The idea
** is to code both <selectA> and <selectB> with the ORDER BY clause as
** co-routines.  Then run the co-routines in parallel and merge the results
** into the output.  In addition to the two coroutines (called selectA and
** selectB) there are 7 subroutines:
**
**    outA:    Move the output of the selectA coroutine into the output
**             of the compound query.
**
**    outB:    Move the output of the selectB coroutine into the output
**             of the compound query.  (Only generated for UNION and
**             UNION ALL.  EXCEPT and INTERSECT never output a row that
**             appears only in B.)
**
**    AltB:    Called when there is data from both coroutines and A<B.
**
**    AeqB:    Called when there is data from both coroutines and A==B.
**
**    AgtB:    Called when there is data from both coroutines and A>B.
**
**    EofA:    Called when data is exhausted from selectA.
**
**    EofB:    Called when data is exhausted from selectB.
**
** The implementation of the latter five subroutines depend on which
** <operator> is used:
**
**
**             UNION ALL         UNION            EXCEPT          INTERSECT
**          -------------  -----------------  --------------  -----------------
**   AltB:   outA, nextA      outA, nextA       outA, nextA         nextA
**
**   AeqB:   outA, nextA         nextA             nextA         outA, nextA
**
**   AgtB:   outB, nextB      outB, nextB          nextB            nextB
**
**   EofA:   outB, nextB      outB, nextB          halt             halt
**
**   EofB:   outA, nextA      outA, nextA       outA, nextA         halt
**
** In the AltB, AeqB, and AgtB subroutines, an EOF on A following nextA
** causes an immediate jump to EofA and an EOF on B following nextB causes
** an immediate jump to EofB.  Within EofA and EofB, and EOF on entry or
** following nextX causes a jump to the end of the select processing.
**
** Duplicate removal in the UNION, EXCEPT, and INTERSECT cases is handled
** within the output subroutine.  The regPrev register set holds the previously
** output value.  A comparison is made against this value and the output
** is skipped if the next results would be the same as the previous.
**
** The implementation plan is to implement the two coroutines and seven
** subroutines first, then put the control logic at the bottom.  Like this:
**
**          goto Init
**     coA: coroutine for left query (A)
**     coB: coroutine for right query (B)
**    outA: output one row of A
**    outB: output one row of B (UNION and UNION ALL only)
**    EofA: ...
**    EofB: ...
**    AltB: ...
**    AeqB: ...
**    AgtB: ...
**    Init: initialize coroutine registers
**          yield coA, on eof goto EofA
**          yield coB, on eof goto EofB
**    Cmpr: Compare A, B
**          Jump AltB, AeqB, AgtB
**     End: ...
**
** We call AltB, AeqB, AgtB, EofA, and EofB "subroutines" but they are not
** actually called using Gosub and they do not Return.  EofA and EofB loop
** until all data is exhausted then jump to the "end" label.  AltB, AeqB,
** and AgtB jump to either Cmpr or to one of EofA or EofB.
*/
#که_نه_تعریف_شوی SQLITE_OMIT_COMPOUND_SELECT
ساكن صحيح multi_Select_By_یوځای(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  صحيح i, j;             /* Loop counters */
  Select *p_مخکینی;       /* Another SELECT immediately to our left */
  Select *p_وېشل;       /* Left-most SELECT in the right-hand group */
  صحيح nSelect;          /* Number of SELECT statements in the compound */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest destA;     /* Destination for coroutine A */
  SelectDest destB;     /* Destination for coroutine B */
  صحيح regAddrA;         /* Address register for select-A coroutine */
  صحيح regAddrB;         /* Address register for select-B coroutine */
  صحيح addrSelectA;      /* Address of the select-A coroutine */
  صحيح addrSelectB;      /* Address of the select-B coroutine */
  صحيح reg_بهر_A;          /* Address register for the output-A subroutine */
  صحيح reg_بهر_B;          /* Address register for the output-B subroutine */
  صحيح addr_بهر_A;         /* Address of the output-A subroutine */
  صحيح addr_بهر_B = 0;     /* Address of the output-B subroutine */
  صحيح addrEofA;         /* Address of the select-A-exhausted subroutine */
  صحيح addrEofA_noB;     /* Alternate addrEofA if B is uninitialized */
  صحيح addrEofB;         /* Address of the select-B-exhausted subroutine */
  صحيح addrAltB;         /* Address of the A<B subroutine */
  صحيح addrAeqB;         /* Address of the A==B subroutine */
  صحيح addrAgtB;         /* Address of the A>B subroutine */
  صحيح reg_حد_A;        /* Limit register for select-A */
  صحيح reg_حد_B;        /* Limit register for select-A */
  صحيح regPrev;          /* A range of registers to hold previous output */
  صحيح saved_حد;       /* Saved value of p->iLimit */
  صحيح saved_دننه_واټن;      /* Saved value of p->iOffset */
  صحيح labelCmpr;        /* Label for the start of the merge algorithm */
  صحيح label_پای_ټکی;         /* Label for the end of the overall SELECT stmt */
  صحيح addr1;            /* Jump instructions that get retargeted */
  صحيح op;               /* One of TK_ALL, TK_UNION, TK_EXCEPT, TK_INTERSECT */
  کلی_Info *p_کلی_Dup = 0; /* Comparison information for duplicate removal */
  کلی_Info *p_کلی_یوځای;   /* Comparison information for merging rows */
  sqlite3 *db;          /* Database connection */
  بیان_List *p_ترتیب_By;   /* The ORDER BY clause */
  صحيح n_ترتیب_By;         /* Number of terms in the ORDER BY clause */
  u32 *a_ترتیب_بدلول;        /* Mapping from ORDER BY terms to result set columns */

  أكد( p->p_ترتیب_By!=0 );
  أكد( p_کلی_Dup==0 ); /* "Managed" code needs this.  Ticket #3382. */
  db = pParse->db;
  v = pParse->pVdbe;
  أكد( v!=0 );       /* Already thrown the error if VDBE alloc failed */
  label_پای_ټکی = sqlite3VdbeMakeLabel(pParse);
  labelCmpr = sqlite3VdbeMakeLabel(pParse);


  /* Patch up the ORDER BY clause
  */
  op = p->op; 
  أكد( p->p_مخکینی->p_ترتیب_By==0 );
  p_ترتیب_By = p->p_ترتیب_By;
  أكد( p_ترتیب_By );
  n_ترتیب_By = p_ترتیب_By->n_بیان;

  /* For operators other than UNION ALL we have to make sure that
  ** the ORDER BY clause covers every term of the result set.  Add
  ** terms to the ORDER BY clause as necessary.  Do the same for UNION ALL
  ** if any operator to the left is not UNION ALL, so that the ORDER BY
  ** carries the collation of every column into that sub-compound.
  */
  لكل(p_مخکینی=p; p_مخکینی && p_مخکینی->op==TK_ALL; p_مخکینی=p_مخکینی->p_مخکینی){}
  إذا( p_مخکینی && p_مخکینی->p_مخکینی ){
    لكل(i=1; db->mallocFailed==0 && i<=p->pEList->n_بیان; i++){
      بنية بیان_List_توکی *p_توکی;
      لكل(j=0, p_توکی=p_ترتیب_By->a; j<n_ترتیب_By; j++, p_توکی++){
        أكد( p_توکی!=0 );
        أكد( p_توکی->u.x.i_ترتیب_By_ستون>0 );
        إذا( p_توکی->u.x.i_ترتیب_By_ستون==i ) اكسر;
      }
      إذا( j==n_ترتیب_By ){
        بیان *pNew = sqlite3_بیان_Int32(db, i);
        إذا( pNew==0 ) أرجع SQLITE_NOMEM_BKPT;
        p->p_ترتیب_By = p_ترتیب_By = sqlite3_بیان_List_Append(pParse, p_ترتیب_By, pNew);
        إذا( p_ترتیب_By ) p_ترتیب_By->a[n_ترتیب_By++].u.x.i_ترتیب_By_ستون = (u16)i;
      }
    }
  }

  /* Compute the comparison permutation and keyinfo that is used with
  ** the permutation to determine if the next row of results comes
  ** from selectA or selectB.  Also add literal collations to the
  ** ORDER BY clause terms so that when selectA and selectB are
  ** evaluated, they use the correct collation.
  */
  a_ترتیب_بدلول = sqlite3DbMallocRawNN(db, حجم(u32)*(n_ترتیب_By + 1));
  إذا( a_ترتیب_بدلول ){
    بنية بیان_List_توکی *p_توکی;
    صحيح bKeep = 0;
    a_ترتیب_بدلول[0] = n_ترتیب_By;
    لكل(i=1, p_توکی=p_ترتیب_By->a; i<=n_ترتیب_By; i++, p_توکی++){
      أكد( p_توکی!=0 );
      أكد( p_توکی->u.x.i_ترتیب_By_ستون>0 );
      أكد( p_توکی->u.x.i_ترتیب_By_ستون<=p->pEList->n_بیان );
      a_ترتیب_بدلول[i] = p_توکی->u.x.i_ترتیب_By_ستون - 1;
      إذا( a_ترتیب_بدلول[i]!=(u32)i-1 ) bKeep = 1;
    }
     إذا( bKeep==0 ){
      sqlite3DbFreeNN(db, a_ترتیب_بدلول);
      a_ترتیب_بدلول = 0;
    }
  }
  p_کلی_یوځای = multi_Select_By_یوځای_کلی_Info(pParse, p, 1);

  /* Allocate a range of temporary registers and the KeyInfo needed
  ** for the logic that removes duplicate result rows when the
  ** operator is UNION, EXCEPT, or INTERSECT (but not UNION ALL).
  */
  إذا( op==TK_ALL ){
    regPrev = 0;
  }وإلا{
    صحيح n_بیان = p->pEList->n_بیان;
    أكد( n_ترتیب_By>=n_بیان || db->mallocFailed );
    regPrev = pParse->nMem+1;
    pParse->nMem += n_بیان+1;
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, 0, regPrev);
    p_کلی_Dup = sqlite3_کلی_Info_Alloc(db, n_بیان, 1);
    إذا( p_کلی_Dup ){
      أكد( sqlite3_کلی_Info_Is_Writeable(p_کلی_Dup) );
      /* Use the ORDER BY collations, not multiSelectCollSeq(p), because
      ** p might be just one part of a larger compound, in which case
      ** the ORDER BY carries the COLLATE of the larger compound. */
      لكل(i=0; ALWAYS(p_کلی_یوځای) && i<n_ترتیب_By; i++){
        p_کلی_Dup->aColl[p_ترتیب_By->a[i].u.x.i_ترتیب_By_ستون-1] = p_کلی_یوځای->aColl[i];
      }
    }
  }

  /* Separate the left and the right query from one another
  */
  nSelect = 1;
  إذا( (op==TK_ALL || op==TK_UNION)
   && OptimizationEnabled(db, SQLITE_متوازن_یوځای)
  ){
    لكل(p_وېشل=p; p_وېشل->p_مخکینی!=0 && p_وېشل->op==op; p_وېشل=p_وېشل->p_مخکینی){
      nSelect++;
      أكد( p_وېشل->p_مخکینی->p_بل==p_وېشل );
    }
  }
  إذا( nSelect<=3 ){
    p_وېشل = p;
  }وإلا{
    p_وېشل = p;
    لكل(i=2; i<nSelect; i+=2){ p_وېشل = p_وېشل->p_مخکینی; }
  }
  p_مخکینی = p_وېشل->p_مخکینی;
  أكد( p_مخکینی!=0 );
  p_وېشل->p_مخکینی = 0;
  p_مخکینی->p_بل = 0;
  أكد( p->p_ترتیب_By == p_ترتیب_By );
  أكد( p_ترتیب_By!=0 || db->mallocFailed );
  p_مخکینی->p_ترتیب_By = sqlite3_بیان_List_Dup(pParse->db, p_ترتیب_By, 0);
  sqlite3_Resolve_ترتیب_ډله_By(pParse, p, p->p_ترتیب_By, "ORDER");
  sqlite3_Resolve_ترتیب_ډله_By(pParse, p_مخکینی, p_مخکینی->p_ترتیب_By, "ORDER");

  /* Compute the limit registers */
  compute_حد_Registers(pParse, p, label_پای_ټکی);
  إذا( p->i_حد && (op==TK_ALL || union_With_حد_One(db,p)) ){
    reg_حد_A = ++pParse->nMem;
    reg_حد_B = ++pParse->nMem;
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Copy, p->i_دننه_واټن ? p->i_دننه_واټن+1 : p->i_حد,
                                  reg_حد_A);
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Copy, reg_حد_A, reg_حد_B);
  }وإلا{
    reg_حد_A = reg_حد_B = 0;
  }
  sqlite3_بیان_Delete(db, p->p_حد);
  p->p_حد = 0;

  regAddrA = ++pParse->nMem;
  regAddrB = ++pParse->nMem;
  reg_بهر_A = ++pParse->nMem;
  reg_بهر_B = ++pParse->nMem;
  sqlite3SelectDestInit(&destA, SRT_Coroutine, regAddrA);
  sqlite3SelectDestInit(&destB, SRT_Coroutine, regAddrB);

  ExplainQueryPlan((pParse, 1, "MERGE (%s)", sqlite3SelectOpName(p->op)));

  /* Generate a coroutine to evaluate the SELECT statement to the
  ** left of the compound operator - the "A" select.
  */
  addrSelectA = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3_Vdbe_جمع_کړه_Op3(v, OP_InitCoroutine, regAddrA, 0, addrSelectA);
  VdbeComment((v, "SUBR: next-A"));
  p_مخکینی->i_حد = reg_حد_A;
  ExplainQueryPlan((pParse, 1, "LEFT"));
  sqlite3Select(pParse, p_مخکینی, &destA);
  sqlite3_Vdbe_پای_ټکی_Coroutine(v, regAddrA);
  sqlite3_Vdbe_Jump_دلته(v, addr1);

  /* Generate a coroutine to evaluate the SELECT statement on
  ** the right - the "B" select
  */
  addrSelectB = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3_Vdbe_جمع_کړه_Op3(v, OP_InitCoroutine, regAddrB, 0, addrSelectB);
  VdbeComment((v, "SUBR: next-B"));
  saved_حد = p->i_حد;
  saved_دننه_واټن = p->i_دننه_واټن;
  p->i_حد = reg_حد_B;
  p->i_دننه_واټن = 0; 
  ExplainQueryPlan((pParse, 1, "RIGHT"));
  sqlite3Select(pParse, p, &destB);
  p->i_حد = saved_حد;
  p->i_دننه_واټن = saved_دننه_واټن;
  sqlite3_Vdbe_پای_ټکی_Coroutine(v, regAddrB);

  /* Generate a subroutine that outputs the current row of the A
  ** select as the next output row of the compound select.
  */
  VdbeNoopComment((v, "SUBR: out-A"));
  addr_بهر_A = generateOutputSubroutine(pParse,
                 p, &destA, pDest, reg_بهر_A,
                 regPrev, p_کلی_Dup, label_پای_ټکی);
 
  /* Generate a subroutine that outputs the current row of the B
  ** select as the next output row of the compound select.
  */
  إذا( op==TK_ALL || op==TK_UNION ){
    VdbeNoopComment((v, "SUBR: out-B"));
    addr_بهر_B = generateOutputSubroutine(pParse,
                 p, &destB, pDest, reg_بهر_B,
                 regPrev, p_کلی_Dup, label_پای_ټکی);
  }
  sqlite3_کلی_Info_Unref(p_کلی_Dup);

  /* Generate a subroutine to run when the results from select A
  ** are exhausted and only data in select B remains.
  */
  إذا( op==TK_EXCEPT || op==TK_INTERSECT ){
    addrEofA_noB = addrEofA = label_پای_ټکی;
  }وإلا{ 
    VdbeNoopComment((v, "SUBR: eof-A"));
    addrEofA = sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Gosub, reg_بهر_B, addr_بهر_B);
    VdbeComment((v, "out-B"));
    addrEofA_noB = sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Yield, regAddrB, label_پای_ټکی);
                                     VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, addrEofA);
    p->n_Select_لیکه = sqlite3_Log_Est_جمع_کړه(p->n_Select_لیکه, p_مخکینی->n_Select_لیکه);
  }

  /* Generate a subroutine to run when the results from select B
  ** are exhausted and only data in select A remains.
  */
  إذا( op==TK_INTERSECT ){
    addrEofB = addrEofA;
    إذا( p->n_Select_لیکه > p_مخکینی->n_Select_لیکه ) p->n_Select_لیکه = p_مخکینی->n_Select_لیکه;
  }وإلا{ 
    VdbeNoopComment((v, "SUBR: eof-B"));
    addrEofB = sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Gosub, reg_بهر_A, addr_بهر_A);
    VdbeComment((v, "out-A"));
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Yield, regAddrA, label_پای_ټکی); VdbeCoverage(v);
    VdbeComment((v, "next-A"));
    sqlite3VdbeGoto(v, addrEofB);
  }

  /* Generate code to handle the case of A<B
  */
  addrAltB = sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Gosub, reg_بهر_A, addr_بهر_A);
  VdbeComment((v, "out-A"));
  sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Yield, regAddrA, addrEofA); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  sqlite3VdbeGoto(v, labelCmpr);

  /* Generate code to handle the case of A==B
  */
  إذا( op==TK_ALL ){
    addrAeqB = addrAltB;
  }وإلا إذا( op==TK_INTERSECT ){
    addrAeqB = addrAltB;
    addrAltB++;
  }وإلا{
    addrAeqB = addrAltB + 1;
  }

  /* Generate code to handle the case of A>B
  */
  addrAgtB = sqlite3VdbeCurrentAddr(v);
  إذا( op==TK_ALL || op==TK_UNION ){
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Gosub, reg_بهر_B, addr_بهر_B);
    VdbeComment((v, "out-B"));
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, labelCmpr);
  }وإلا{
    addrAgtB++; /* Just do next-B.  Might as well use the next-B call
                ** in the next code block */
  }

  /* This code runs once to initialize everything.
  */
  sqlite3_Vdbe_Jump_دلته(v, addr1);
  sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Yield, regAddrA, addrEofA_noB); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  /* v---  Also the A>B case for EXCEPT and INTERSECT */
  sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
  VdbeComment((v, "next-B"));

  /* Implement the main merge loop
  */
  إذا( a_ترتیب_بدلول!=0 ){
    sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Permutation, 0, 0, 0, (حرف*)a_ترتیب_بدلول, P4_INTARRAY);
  }
  sqlite3VdbeResolveLabel(v, labelCmpr);
  sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Compare, destA.iSdst, destB.iSdst, n_ترتیب_By,
                         (حرف*)p_کلی_یوځای, P4_KEYINFO);
  إذا( a_ترتیب_بدلول!=0 ){
    sqlite3VdbeChangeP5(v, OPFLAG_ترتیب_بدلول);
  }
  sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Jump, addrAltB, addrAeqB, addrAgtB);
  VdbeCoverageIf(v, op==TK_ALL);
  VdbeCoverageIf(v, op==TK_UNION);
  VdbeCoverageIf(v, op==TK_EXCEPT);
  VdbeCoverageIf(v, op==TK_INTERSECT);

  /* Jump to the this point in order to terminate the query.
  */
  sqlite3VdbeResolveLabel(v, label_پای_ټکی);

  /* Make arrangements to free the 2nd and subsequent arms of the compound
  ** after the parse has finished */
  إذا( p_وېشل->p_مخکینی ){
    sqlite3_Parser_جمع_کړه_Cleanup(pParse, sqlite3SelectDeleteGeneric, p_وېشل->p_مخکینی);
  }
  p_وېشل->p_مخکینی = p_مخکینی;
  p_مخکینی->p_بل = p_وېشل;
  sqlite3_بیان_List_Delete(db, p_مخکینی->p_ترتیب_By);
  p_مخکینی->p_ترتیب_By = 0;

  /*** TBD:  Insert subroutine calls to close cursors on incomplete
  **** subqueries ****/
  Explain_Query_Plan_راوباسه(pParse);
  أرجع pParse->nErr!=0;
}
#د_که_پای

#إذا !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)

/* An instance of the SubstContext object describes an substitution edit
** to be performed on a parse tree.
**
** All references to columns in table iTable are to be replaced by corresponding
** expressions in pEList.
**
** ## About "isOuterJoin":
**
** The isOuterJoin column indicates that the replacement will occur into a
** position in the parent that is NULL-able due to an OUTER JOIN.  Either the
** target slot in the parent is the right operand of a LEFT JOIN, or one of
** the left operands of a RIGHT JOIN.  In either case, we need to potentially
** bypass the substituted expression with OP_IfNullRow.
**
** Suppose the original expression is an integer constant. Even though the table
** has the nullRow flag set, because the expression is an integer constant,
** it will not be NULLed out.  So instead, we insert an OP_IfNullRow opcode
** that checks to see if the nullRow flag is set on the table.  If the nullRow
** flag is set, then the value in the register is set to NULL and the original
** expression is bypassed.  If the nullRow flag is not set, then the original
** expression runs to populate the register.
**
** Example where this is needed:
**
**      CREATE TABLE t1(a INTEGER PRIMARY KEY, b INT);
**      CREATE TABLE t2(x INT UNIQUE);
**
**      SELECT a,b,m,x FROM t1 LEFT JOIN (SELECT 59 AS m,x FROM t2) ON b=x;
**
** When the subquery on the right side of the LEFT JOIN is flattened, we
** have to add OP_IfNullRow in front of the OP_Integer that implements the
** "m" value of the subquery so that a NULL will be loaded instead of 59
** when processing a non-matched row of the left.
*/
تعريفنوع بنية SubstContext {
  Parse *pParse;            /* The parsing context */
  صحيح i_جدول;               /* Replace references to this table */
  صحيح i_New_جدول;            /* New table number */
  صحيح isOuterJoin;          /* Add TK_IF_NULL_ROW opcodes on each replacement */
  صحيح n_Sel_ژوروالی;            /* Depth of sub-query recursion.  Top==1 */
  بیان_List *pEList;         /* Replacement expressions */
  بیان_List *pCList;         /* Collation sequences for replacement expr */
} SubstContext;

/* Forward Declarations */
ساكن فراغ subst_بیان_List(SubstContext*, بیان_List*);
ساكن فراغ substSelect(SubstContext*, Select*, صحيح);

/*
** Scan through the expression pExpr.  Replace every reference to
** a column in table number iTable with a copy of the iColumn-th
** entry in pEList.  (But leave references to the ROWID column
** unchanged.)
**
** This routine is part of the flattening procedure.  A subquery
** whose result set is defined by pEList appears as entry in the
** FROM clause of a SELECT such that the VDBE cursor assigned to that
** FORM clause entry is iTable.  This routine makes the necessary
** changes to pExpr so that it refers directly to the source table
** of the subquery rather the result set of the subquery.
*/
ساكن بیان *subst_بیان(
  SubstContext *pSubst,  /* Description of the substitution */
  بیان *p_بیان            /* Expr in which substitution occurs */
){
  إذا( p_بیان==0 ) أرجع 0;
  إذا( بیان_Has_Property(p_بیان, EP_OuterON|EP_InnerON)
   && p_بیان->w.iJoin==pSubst->i_جدول
  ){
    testcase( بیان_Has_Property(p_بیان, EP_InnerON) );
    p_بیان->w.iJoin = pSubst->i_New_جدول;
  }
  إذا( p_بیان->op==TK_ستنه
   && p_بیان->i_جدول==pSubst->i_جدول
   && !بیان_Has_Property(p_بیان, EP_Fixed_ستون)
  ){
#که_تعریف_شوی SQLITE_ALLOW_ROWID_دننه_VIEW
    إذا( p_بیان->i_ستنه<0 ){
      p_بیان->op = TK_NULL;
    }وإلا
#د_که_پای
    {
      بیان *pNew;
      صحيح i_ستنه;
      بیان *pCopy;
      بیان if_Null_لیکه;
      i_ستنه = p_بیان->i_ستنه;
      أكد( i_ستنه>=0 );
      أكد( pSubst->pEList!=0 && i_ستنه<pSubst->pEList->n_بیان );
      أكد( p_بیان->p_ښی==0 );
      pCopy = pSubst->pEList->a[i_ستنه].p_بیان;
      إذا( sqlite3_بیان_Is_ویکټور(pCopy) ){
        sqlite3_ویکټور_Error_پیغام(pSubst->pParse, pCopy);
      }وإلا{
        sqlite3 *db = pSubst->pParse->db;
        إذا( pSubst->isOuterJoin
         && (pCopy->op!=TK_ستنه || pCopy->i_جدول!=pSubst->i_New_جدول)
        ){
          memset(&if_Null_لیکه, 0, حجم(if_Null_لیکه));
          if_Null_لیکه.op = TK_IF_NULL_لیکه;
          if_Null_لیکه.p_کیڼ = pCopy;
          if_Null_لیکه.i_جدول = pSubst->i_New_جدول;
          if_Null_لیکه.i_ستنه = -99;
          if_Null_لیکه.flags = EP_If_Null_لیکه;
          pCopy = &if_Null_لیکه;
        }
        testcase( بیان_Has_Property(pCopy, EP_Subquery) );
        pNew = sqlite3_بیان_Dup(db, pCopy, 0);
        إذا( db->mallocFailed ){
          sqlite3_بیان_Delete(db, pNew);
          أرجع p_بیان;
        }
        إذا( pSubst->isOuterJoin ){
          بیان_سېټ_Property(pNew, EP_CanBeNull);
        }
        إذا( pNew->op==TK_TRUEFALSE ){
          pNew->u.i_ارزښت = sqlite3_بیان_Truth_ارزښت(pNew);
          pNew->op = TK_INTEGER;
          بیان_سېټ_Property(pNew, EP_Int_ارزښت);
        }

        /* Ensure that the expression now has an implicit collation sequence,
        ** just as it did when it was a column of a view or sub-query. */
        {
          CollSeq *pNat = sqlite3_بیان_Coll_Seq(pSubst->pParse, pNew);
          CollSeq *pColl = sqlite3_بیان_Coll_Seq(pSubst->pParse,
                pSubst->pCList->a[i_ستنه].p_بیان
          );
          إذا( pNat!=pColl || (pNew->op!=TK_ستنه && pNew->op!=TK_COLLATE) ){
            pNew = sqlite3_بیان_جمع_کړه_Collate_String(pSubst->pParse, pNew,
                (pColl ? pColl->zName : "BINARY")
            );
          }
        }
        بیان_Clear_Property(pNew, EP_Collate);
        إذا( بیان_Has_Property(p_بیان,EP_OuterON|EP_InnerON) ){
          sqlite3_سېټ_Join_بیان(pNew, p_بیان->w.iJoin,
                             p_بیان->flags & (EP_OuterON|EP_InnerON));
        }
        sqlite3_بیان_Delete(db, p_بیان);
        p_بیان = pNew;
      }
    }
  }وإلا{
    إذا( p_بیان->op==TK_IF_NULL_لیکه && p_بیان->i_جدول==pSubst->i_جدول ){
      p_بیان->i_جدول = pSubst->i_New_جدول;
    }
    إذا( p_بیان->op==TK_AGG_FUNCTION && p_بیان->op2>=pSubst->n_Sel_ژوروالی ){
      p_بیان->op2--;
    }
    p_بیان->p_کیڼ = subst_بیان(pSubst, p_بیان->p_کیڼ);
    p_بیان->p_ښی = subst_بیان(pSubst, p_بیان->p_ښی);
    إذا( بیان_Use_XSelect(p_بیان) ){
      substSelect(pSubst, p_بیان->x.pSelect, 1);
    }وإلا{
      subst_بیان_List(pSubst, p_بیان->x.pList);
    }
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
    إذا( بیان_Has_Property(p_بیان, EP_WinFunc) ){
      کړکۍ *pWin = p_بیان->y.pWin;
      pWin->pFilter = subst_بیان(pSubst, pWin->pFilter);
      subst_بیان_List(pSubst, pWin->p_برخه_کول);
      subst_بیان_List(pSubst, pWin->p_ترتیب_By);
    }
#د_که_پای
  }
  أرجع p_بیان;
}
ساكن فراغ subst_بیان_List(
  SubstContext *pSubst, /* Description of the substitution */
  بیان_List *pList       /* List to scan and in which to make substitutes */
){
  صحيح i;
  إذا( pList==0 ) أرجع;
  لكل(i=0; i<pList->n_بیان; i++){
    pList->a[i].p_بیان = subst_بیان(pSubst, pList->a[i].p_بیان);
  }
}
ساكن فراغ substSelect(
  SubstContext *pSubst, /* Description of the substitution */
  Select *p,            /* SELECT statement in which to make substitutions */
  صحيح do_مخکینی           /* Do substitutes on p->pPrior too */
){
  SrcList *pSrc;
  Src_توکی *p_توکی;
  صحيح i;
  إذا( !p ) أرجع;
  pSubst->n_Sel_ژوروالی++;
  افعل{
    subst_بیان_List(pSubst, p->pEList);
    subst_بیان_List(pSubst, p->p_ډله_By);
    subst_بیان_List(pSubst, p->p_ترتیب_By);
    p->pHaving = subst_بیان(pSubst, p->pHaving);
    p->pWhere = subst_بیان(pSubst, p->pWhere);
    pSrc = p->pSrc;
    أكد( pSrc!=0 );
    لكل(i=pSrc->nSrc, p_توکی=pSrc->a; i>0; i--, p_توکی++){
      إذا( p_توکی->fg.isSubquery ){
        substSelect(pSubst, p_توکی->u4.pSubq->pSelect, 1);
      }
      إذا( p_توکی->fg.isTabFunc ){
        subst_بیان_List(pSubst, p_توکی->u1.pFuncArg);
      }
    }
  }طالما( do_مخکینی && (p = p->p_مخکینی)!=0 );
  pSubst->n_Sel_ژوروالی--;
}
#د_که_پای /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#إذا !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
/*
** pSelect is a SELECT statement and pSrcItem is one item in the FROM
** clause of that SELECT.
**
** This routine scans the entire SELECT statement and recomputes the
** pSrcItem->colUsed mask.
*/
ساكن صحيح recompute_Columns_کارول_شوی_بیان(Walker *pWalker, بیان *p_بیان){
  Src_توکی *p_توکی;
  إذا( p_بیان->op!=TK_ستنه ) أرجع WRC_Continue;
  p_توکی = pWalker->u.p_Src_توکی;
  إذا( p_توکی->iCursor!=p_بیان->i_جدول ) أرجع WRC_Continue;
  إذا( p_بیان->i_ستنه<0 ) أرجع WRC_Continue;
  p_توکی->ستون_کارول_شوی |= sqlite3_بیان_ستون_کارول_شوی(p_بیان);
  أرجع WRC_Continue;
}
ساكن فراغ recompute_Columns_کارول_شوی(
  Select *pSelect,                 /* The complete SELECT statement */
  Src_توکی *p_Src_توکی                /* Which FROM clause item to recompute */
){
  Walker w;
  إذا( NEVER(p_Src_توکی->pSTab==0) ) أرجع;
  memset(&w, 0, حجم(w));
  w.x_بیان_Callback = recompute_Columns_کارول_شوی_بیان;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.p_Src_توکی = p_Src_توکی;
  p_Src_توکی->ستون_کارول_شوی = 0;
  sqlite3WalkSelect(&w, pSelect);
}
#د_که_پای /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#إذا !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
/*
** Assign new cursor numbers to each of the items in pSrc. For each
** new cursor number assigned, set an entry in the aCsrMap[] array
** to map the old cursor number to the new:
**
**     aCsrMap[iOld+1] = iNew;
**
** The array is guaranteed by the caller to be large enough for all
** existing cursor numbers in pSrc.  aCsrMap[0] is the array size.
**
** If pSrc contains any sub-selects, call this routine recursively
** on the FROM clause of each such sub-select, with iExcept set to -1.
*/
ساكن فراغ srclistRenumberCursors(
  Parse *pParse,                  /* Parse context */
  صحيح *aCsrMap,                   /* Array to store cursor mappings in */
  SrcList *pSrc,                  /* FROM clause to renumber */
  صحيح iExcept                     /* FROM clause item to skip */
){
  صحيح i;
  Src_توکی *p_توکی;
  لكل(i=0, p_توکی=pSrc->a; i<pSrc->nSrc; i++, p_توکی++){
    إذا( i!=iExcept ){
      Select *p;
      أكد( p_توکی->iCursor < aCsrMap[0] );
      إذا( !p_توکی->fg.isRecursive || aCsrMap[p_توکی->iCursor+1]==0 ){
        aCsrMap[p_توکی->iCursor+1] = pParse->nTab++;
      }
      p_توکی->iCursor = aCsrMap[p_توکی->iCursor+1];
      إذا( p_توکی->fg.isSubquery ){
        لكل(p=p_توکی->u4.pSubq->pSelect; p; p=p->p_مخکینی){
          srclistRenumberCursors(pParse, aCsrMap, p->pSrc, -1);
        }
      }
    }
  }
}

/*
** *piCursor is a cursor number.  Change it if it needs to be mapped.
*/
ساكن فراغ renumberCursorDoMapping(Walker *pWalker, صحيح *piCursor){
  صحيح *aCsrMap = pWalker->u.ai_ستون;
  صحيح iCsr = *piCursor;
  إذا( iCsr < aCsrMap[0] && aCsrMap[iCsr+1]>0 ){
    *piCursor = aCsrMap[iCsr+1];
  }
}

/*
** Expression walker callback used by renumberCursors() to update
** Expr objects to match newly assigned cursor numbers.
*/
ساكن صحيح renumberCursorsCb(Walker *pWalker, بیان *p_بیان){
  صحيح op = p_بیان->op;
  إذا( op==TK_ستنه || op==TK_IF_NULL_لیکه ){
    renumberCursorDoMapping(pWalker, &p_بیان->i_جدول);
  }
  إذا( بیان_Has_Property(p_بیان, (EP_OuterON|EP_InnerON)) ){
    testcase( بیان_Has_Property(p_بیان, EP_OuterON) );
    testcase( بیان_Has_Property(p_بیان, EP_InnerON) );
    renumberCursorDoMapping(pWalker, &p_بیان->w.iJoin);
  }
  أرجع WRC_Continue;
}

/*
** Assign a new cursor number to each cursor in the FROM clause (Select.pSrc)
** of the SELECT statement passed as the second argument, and to each
** cursor in the FROM clause of any FROM clause sub-selects, recursively.
** Except, do not assign a new cursor number to the iExcept'th element in
** the FROM clause of (*p). Update all expressions and other references
** to refer to the new cursor numbers.
**
** Argument aCsrMap is an array that may be used for temporary working
** space. Two guarantees are made by the caller:
**
**   * the array is larger than the largest cursor number used within the
**     select statement passed as an argument, and
**
**   * the array entries for all cursor numbers that do *not* appear in
**     FROM clauses of the select statement as described above are
**     initialized to zero.
*/
ساكن فراغ renumberCursors(
  Parse *pParse,                  /* Parse context */
  Select *p,                      /* Select to renumber cursors within */
  صحيح iExcept,                    /* FROM clause item to skip */
  صحيح *aCsrMap                    /* Working space */
){
  Walker w;
  srclistRenumberCursors(pParse, aCsrMap, p->pSrc, iExcept);
  memset(&w, 0, حجم(w));
  w.u.ai_ستون = aCsrMap;
  w.x_بیان_Callback = renumberCursorsCb;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  sqlite3WalkSelect(&w, p);
}
#د_که_پای /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** If pSel is not part of a compound SELECT, return a pointer to its
** expression list. Otherwise, return a pointer to the expression list
** of the leftmost SELECT in the compound.
*/
ساكن بیان_List *پیدا_کړه_Leftmost_Exprlist(Select *pSel){
  طالما( pSel->p_مخکینی ){
    pSel = pSel->p_مخکینی;
  }
  أرجع pSel->pEList;
}

/*
** Return true if any of the result-set columns in the compound query
** have incompatible affinities on one or more arms of the compound.
*/
ساكن صحيح compoundHasDifferentAffinities(Select *p){
  صحيح ii;
  بیان_List *pList;
  أكد( p!=0 );
  أكد( p->pEList!=0 );
  أكد( p->p_مخکینی!=0 );
  pList = p->pEList;
  لكل(ii=0; ii<pList->n_بیان; ii++){
    حرف aff;
    Select *pSub1;
    أكد( pList->a[ii].p_بیان!=0 );
    aff = sqlite3_بیان_Affinity(pList->a[ii].p_بیان);
    لكل(pSub1=p->p_مخکینی; pSub1; pSub1=pSub1->p_مخکینی){
      أكد( pSub1->pEList!=0 );
      أكد( pSub1->pEList->n_بیان>ii );
      أكد( pSub1->pEList->a[ii].p_بیان!=0 );
      إذا( sqlite3_بیان_Affinity(pSub1->pEList->a[ii].p_بیان)!=aff ){
        أرجع 1;
      }
    }
  }
  أرجع 0;
}

#إذا !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
/*
** This routine attempts to flatten subqueries as a performance optimization.
** This routine returns 1 if it makes changes and 0 if no flattening occurs.
**
** To understand the concept of flattening, consider the following
** query:
**
**     SELECT a FROM (SELECT x+y AS a FROM t1 WHERE z<100) WHERE a>5
**
** The default way of implementing this query is to execute the
** subquery first and store the results in a temporary table, then
** run the outer query on that temporary table.  This requires two
** passes over the data.  Furthermore, because the temporary table
** has no indices, the WHERE clause on the outer query cannot be
** optimized.
**
** This routine attempts to rewrite queries such as the above into
** a single flat select, like this:
**
**     SELECT x+y AS a FROM t1 WHERE z<100 AND a>5
**
** The code generated for this simplification gives the same result
** but only has to scan the data once.  And because indices might
** exist on the table t1, a complete scan of the data might be
** avoided.
**
** Flattening is subject to the following constraints:
**
**  (**)  We no longer attempt to flatten aggregate subqueries. Was:
**        The subquery and the outer query cannot both be aggregates.
**
**  (**)  We no longer attempt to flatten aggregate subqueries. Was:
**        (2) If the subquery is an aggregate then
**        (2a) the outer query must not be a join and
**        (2b) the outer query must not use subqueries
**             other than the one FROM-clause subquery that is a candidate
**             for flattening.  (This is due to ticket [2f7170d73bf9abf80]
**             from 2015-02-09.)
**
**   (3)  If the subquery is the right operand of a LEFT JOIN then
**        (3a) the subquery may not be a join
**        (**) Was (3b): "the FROM clause of the subquery may not contain
**             a virtual table"
**        (**) Was: "The outer query may not have a GROUP BY." This case
**             is now managed correctly
**        (3d) the outer query may not be DISTINCT.
**        See also (26) for restrictions on RIGHT JOIN.
**
**   (4)  The subquery can not be DISTINCT.
**
**  (**)  At one point restrictions (4) and (5) defined a subset of DISTINCT
**        sub-queries that were excluded from this optimization. Restriction
**        (4) has since been expanded to exclude all DISTINCT subqueries.
**
**  (**)  We no longer attempt to flatten aggregate subqueries.  Was:
**        If the subquery is aggregate, the outer query may not be DISTINCT.
**
**   (7)  The subquery must have a FROM clause.  TODO:  For subqueries without
**        A FROM clause, consider adding a FROM clause with the special
**        table sqlite_once that consists of a single row containing a
**        single NULL.
**
**   (8)  If the subquery uses LIMIT then the outer query may not be a join.
**
**   (9)  If the subquery uses LIMIT then the outer query may not be aggregate.
**
**  (**)  Restriction (10) was removed from the code on 2005-02-05 but we
**        accidentally carried the comment forward until 2014-09-15.  Original
**        constraint: "If the subquery is aggregate then the outer query
**        may not use LIMIT."
**
**  (11)  The subquery and the outer query may not both have ORDER BY clauses.
**
**  (**)  Not implemented.  Subsumed into restriction (3).  Was previously
**        a separate restriction deriving from ticket #350.
**
**  (13)  The subquery and outer query may not both use LIMIT.
**
**  (14)  The subquery may not use OFFSET.
**
**  (15)  If the outer query is part of a compound select, then the
**        subquery may not use LIMIT.
**        (See ticket #2339 and ticket [02a8e81d44]).
**
**  (16)  If the outer query is aggregate, then the subquery may not
**        use ORDER BY.  (Ticket #2942)  This used to not matter
**        until we introduced the group_concat() function. 
**
**  (17)  If the subquery is a compound select, then
**        (17a) all compound operators must be a UNION ALL, and
**        (17b) no terms within the subquery compound may be aggregate
**              or DISTINCT, and
**        (17c) every term within the subquery compound must have a FROM clause
**        (17d) the outer query may not be
**              (17d1) aggregate, or
**              (17d2) DISTINCT
**        (17e) the subquery may not contain window functions, and
**        (17f) the subquery must not be the RHS of a LEFT JOIN.
**        (17g) either the subquery is the first element of the outer
**              query or there are no RIGHT or FULL JOINs in any arm
**              of the subquery.  (This is a duplicate of condition (27b).)
**        (17h) The corresponding result set expressions in all arms of the
**              compound must have the same affinity.
**
**        The parent and sub-query may contain WHERE clauses. Subject to
**        rules (11), (13) and (14), they may also contain ORDER BY,
**        LIMIT and OFFSET clauses.  The subquery cannot use any compound
**        operator other than UNION ALL because all the other compound
**        operators have an implied DISTINCT which is disallowed by
**        restriction (4).
**
**        Also, each component of the sub-query must return the same number
**        of result columns. This is actually a requirement for any compound
**        SELECT statement, but all the code here does is make sure that no
**        such (illegal) sub-query is flattened. The caller will detect the
**        syntax error and return a detailed message.
**
**  (18)  If the sub-query is a compound select, then all terms of the
**        ORDER BY clause of the parent must be copies of a term returned
**        by the parent query.
**
**  (19)  If the subquery uses LIMIT then the outer query may not
**        have a WHERE clause.
**
**  (20)  If the sub-query is a compound select, then it must not use
**        an ORDER BY clause.  Ticket #3773.  We could relax this constraint
**        somewhat by saying that the terms of the ORDER BY clause must
**        appear as unmodified result columns in the outer query.  But we
**        have other optimizations in mind to deal with that case.
**
**  (21)  If the subquery uses LIMIT then the outer query may not be
**        DISTINCT.  (See ticket [752e1646fc]).
**
**  (22)  The subquery may not be a recursive CTE.
**
**  (23)  If the outer query is a recursive CTE, then the sub-query may not be
**        a compound query.  This restriction is because transforming the
**        parent to a compound query confuses the code that handles
**        recursive queries in multiSelect().
**
**  (**)  We no longer attempt to flatten aggregate subqueries.  Was:
**        The subquery may not be an aggregate that uses the built-in min() or
**        or max() functions.  (Without this restriction, a query like:
**        "SELECT x FROM (SELECT max(y), x FROM t1)" would not necessarily
**        return the value X for which Y was maximal.)
**
**  (25)  If either the subquery or the parent query contains a window
**        function in the select list or ORDER BY clause, flattening
**        is not attempted.
**
**  (26)  The subquery may not be the right operand of a RIGHT JOIN.
**        See also (3) for restrictions on LEFT JOIN.
**
**  (27)  The subquery may not contain a FULL or RIGHT JOIN unless it
**        is the first element of the parent query.  Two subcases:
**        (27a) the subquery is not a compound query.
**        (27b) the subquery is a compound query and the RIGHT JOIN occurs
**              in any arm of the compound query.  (See also (17g).)
**
**  (28)  The subquery is not a MATERIALIZED CTE.  (This is handled
**        in the caller before ever reaching this routine.)
**
**
** In this routine, the "p" parameter is a pointer to the outer query.
** The subquery is p->pSrc->a[iFrom].  isAgg is true if the outer query
** uses aggregates.
**
** If flattening is not attempted, this routine is a no-op and returns 0.
** If flattening is attempted this routine returns 1.
**
** All of the expression analysis must occur on both the outer query and
** the subquery before this routine runs.
*/
ساكن صحيح flattenSubquery(
  Parse *pParse,       /* Parsing context */
  Select *p,           /* The parent or outer SELECT statement */
  صحيح iFrom,           /* Index in p->pSrc->a[] of the inner subquery */
  صحيح isAgg            /* True if outer SELECT uses aggregate functions */
){
  ثابت حرف *zSavedAuthContext = pParse->zAuthContext;
  Select *p_پلار;    /* Current UNION ALL term of the other query */
  Select *pSub;       /* The inner query or "subquery" */
  Select *pSub1;      /* Pointer to the rightmost select in sub-query */
  SrcList *pSrc;      /* The FROM clause of the outer query */
  SrcList *pSubSrc;   /* The FROM clause of the subquery */
  صحيح i_پلار;        /* VDBE cursor number of the pSub result set temp table */
  صحيح i_New_پلار = -1;/* Replacement table for iParent */
  صحيح isOuterJoin = 0; /* True if pSub is the right side of a LEFT JOIN */   
  صحيح i;              /* Loop counter */
  بیان *pWhere;                    /* The WHERE clause */
  Src_توکی *pSubitem;               /* The subquery */
  sqlite3 *db = pParse->db;
  Walker w;                        /* Walker to persist agginfo data */
  صحيح *aCsrMap = 0;

  /* Check to see if flattening is permitted.  Return 0 if not.
  */
  أكد( p!=0 );
  أكد( p->p_مخکینی==0 );
  إذا( OptimizationDisabled(db, SQLITE_QueryFlattener) ) أرجع 0;
  pSrc = p->pSrc;
  أكد( pSrc && iFrom>=0 && iFrom<pSrc->nSrc );
  pSubitem = &pSrc->a[iFrom];
  i_پلار = pSubitem->iCursor;
  أكد( pSubitem->fg.isSubquery );
  pSub = pSubitem->u4.pSubq->pSelect;
  أكد( pSub!=0 );

#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
  إذا( p->pWin || pSub->pWin ) أرجع 0;                  /* Restriction (25) */
#د_که_پای

  pSubSrc = pSub->pSrc;
  أكد( pSubSrc );
  /* Prior to version 3.1.2, when LIMIT and OFFSET had to be simple constants,
  ** not arbitrary expressions, we allowed some combining of LIMIT and OFFSET
  ** because they could be computed at compile-time.  But when LIMIT and OFFSET
  ** became arbitrary expressions, we were forced to add restrictions (13)
  ** and (14). */
  إذا( pSub->p_حد && p->p_حد ) أرجع 0;              /* Restriction (13) */
  إذا( pSub->p_حد && pSub->p_حد->p_ښی ) أرجع 0;   /* Restriction (14) */
  إذا( (p->selFlags & SF_Compound)!=0 && pSub->p_حد ){
    أرجع 0;                                            /* Restriction (15) */
  }
  إذا( pSubSrc->nSrc==0 ) أرجع 0;                       /* Restriction (7)  */
  إذا( pSub->selFlags & SF_Distinct ) أرجع 0;           /* Restriction (4)  */
  إذا( pSub->p_حد && (pSrc->nSrc>1 || isAgg) ){
     أرجع 0;         /* Restrictions (8)(9) */
  }
  إذا( p->p_ترتیب_By && pSub->p_ترتیب_By ){
     أرجع 0;                                           /* Restriction (11) */
  }
  إذا( isAgg && pSub->p_ترتیب_By ) أرجع 0;                /* Restriction (16) */
  إذا( pSub->p_حد && p->pWhere ) أرجع 0;              /* Restriction (19) */
  إذا( pSub->p_حد && (p->selFlags & SF_Distinct)!=0 ){
     أرجع 0;         /* Restriction (21) */
  }
  إذا( pSub->selFlags & (SF_Recursive) ){
    أرجع 0; /* Restrictions (22) */
  }

  /*
  ** If the subquery is the right operand of a LEFT JOIN, then the
  ** subquery may not be a join itself (3a). Example of why this is not
  ** allowed:
  **
  **         t1 LEFT OUTER JOIN (t2 JOIN t3)
  **
  ** If we flatten the above, we would get
  **
  **         (t1 LEFT OUTER JOIN t2) JOIN t3
  **
  ** which is not at all the same thing.
  **
  ** See also tickets #306, #350, and #3300.
  */
  إذا( (pSubitem->fg.jointype & (JT_OUTER|JT_LTORJ))!=0 ){
    إذا( pSubSrc->nSrc>1                        /* (3a) */
     /**** || IsVirtual(pSubSrc->a[0].pSTab)      (3b)-omitted */
     || (p->selFlags & SF_Distinct)!=0         /* (3d) */
     || (pSubitem->fg.jointype & JT_ښی)!=0  /* (26) */
    ){
      أرجع 0;
    }
    isOuterJoin = 1;
  }

  أكد( pSubSrc->nSrc>0 );  /* True by restriction (7) */
  إذا( iFrom>0 && (pSubSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
    أرجع 0;   /* Restriction (27a) */
  }

  /* Condition (28) is blocked by the caller */
  أكد( !pSubitem->fg.isCte || pSubitem->u2.pCteUse->eM10d!=M10d_Yes );

  /* Restriction (17): If the sub-query is a compound SELECT, then it must
  ** use only the UNION ALL operator. And none of the simple select queries
  ** that make up the compound SELECT are allowed to be aggregate or distinct
  ** queries.
  */
  إذا( pSub->p_مخکینی ){
    صحيح ii;
    إذا( pSub->p_ترتیب_By ){
      أرجع 0;  /* Restriction (20) */
    }
    إذا( isAgg || (p->selFlags & SF_Distinct)!=0 || isOuterJoin>0 ){
      أرجع 0; /* (17d1), (17d2), or (17f) */
    }
    لكل(pSub1=pSub; pSub1; pSub1=pSub1->p_مخکینی){
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct );
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Aggregate );
      أكد( pSub->pSrc!=0 );
      أكد( (pSub->selFlags & SF_Recursive)==0 );
      أكد( pSub->pEList->n_بیان==pSub1->pEList->n_بیان );
      إذا( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))!=0    /* (17b) */
       || (pSub1->p_مخکینی && pSub1->op!=TK_ALL)                 /* (17a) */
       || pSub1->pSrc->nSrc<1                                  /* (17c) */
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
       || pSub1->pWin                                          /* (17e) */
#د_که_پای
      ){
        أرجع 0;
      }
      إذا( iFrom>0 && (pSub1->pSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
        /* Without this restriction, the JT_LTORJ flag would end up being
        ** omitted on left-hand tables of the right join that is being
        ** flattened. */
        أرجع 0;   /* Restrictions (17g), (27b) */
      }
      testcase( pSub1->pSrc->nSrc>1 );
    }

    /* Restriction (18). */
    إذا( p->p_ترتیب_By ){
      لكل(ii=0; ii<p->p_ترتیب_By->n_بیان; ii++){
        إذا( p->p_ترتیب_By->a[ii].u.x.i_ترتیب_By_ستون==0 ) أرجع 0;
      }
    }

    /* Restriction (23) */
    إذا( (p->selFlags & SF_Recursive) ) أرجع 0;

    /* Restriction (17h) */
    إذا( compoundHasDifferentAffinities(pSub) ) أرجع 0;

    إذا( pSrc->nSrc>1 ){
      إذا( pParse->nSelect>500 ) أرجع 0;
      إذا( OptimizationDisabled(db, SQLITE_FlttnUnionAll) ) أرجع 0;
      aCsrMap = sqlite3DbMallocZero(db, ((i64)pParse->nTab+1)*حجم(صحيح));
      إذا( aCsrMap ) aCsrMap[0] = pParse->nTab;
    }
  }

  /***** If we reach this point, flattening is permitted. *****/
  TREETRACE(0x4,pParse,p,("flatten %u.%p from term %d\n",
                   pSub->selId, pSub, iFrom));

  /* Authorize the subquery */
  pParse->zAuthContext = pSubitem->zName;
  TESTONLY(i =) sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0);
  testcase( i==SQLITE_DENY );
  pParse->zAuthContext = zSavedAuthContext;

  /* Delete the transient structures associated with the subquery */
  
  إذا( ALWAYS(pSubitem->fg.isSubquery) ){
    pSub1 = sqlite3SubqueryDetach(db, pSubitem);
  }وإلا{
    pSub1 = 0;
  }
  أكد( pSubitem->fg.isSubquery==0 );
  أكد( pSubitem->fg.fixedSchema==0 );
  sqlite3DbFree(db, pSubitem->zName);
  sqlite3DbFree(db, pSubitem->zAlias);
  pSubitem->zName = 0;
  pSubitem->zAlias = 0;
  أكد( pSubitem->fg.isUsing!=0 || pSubitem->u3.pOn==0 );

  /* If the sub-query is a compound SELECT statement, then (by restrictions
  ** 17 and 18 above) it must be a UNION ALL and the parent query must
  ** be of the form:
  **
  **     SELECT <expr-list> FROM (<sub-query>) <where-clause>
  **
  ** followed by any ORDER BY, LIMIT and/or OFFSET clauses. This block
  ** creates N-1 copies of the parent query without any ORDER BY, LIMIT or
  ** OFFSET clauses and joins them to the left-hand-side of the original
  ** using UNION ALL operators. In this case N is the number of simple
  ** select statements in the compound sub-query.
  **
  ** Example:
  **
  **     SELECT a+1 FROM (
  **        SELECT x FROM tab
  **        UNION ALL
  **        SELECT y FROM tab
  **        UNION ALL
  **        SELECT abs(z*2) FROM tab2
  **     ) WHERE a!=5 ORDER BY 1
  **
  ** Transformed into:
  **
  **     SELECT x+1 FROM tab WHERE x+1!=5
  **     UNION ALL
  **     SELECT y+1 FROM tab WHERE y+1!=5
  **     UNION ALL
  **     SELECT abs(z*2)+1 FROM tab2 WHERE abs(z*2)+1!=5
  **     ORDER BY 1
  **
  ** We call this the "compound-subquery flattening".
  */
  لكل(pSub=pSub->p_مخکینی; pSub; pSub=pSub->p_مخکینی){
    Select *pNew;
    بیان_List *p_ترتیب_By = p->p_ترتیب_By;
    بیان *p_حد = p->p_حد;
    Select *p_مخکینی = p->p_مخکینی;
    جدول *p_توکی_Tab = pSubitem->pSTab;
    pSubitem->pSTab = 0;
    p->p_ترتیب_By = 0;
    p->p_مخکینی = 0;
    p->p_حد = 0;
    pNew = sqlite3SelectDup(db, p, 0);
    p->p_حد = p_حد;
    p->p_ترتیب_By = p_ترتیب_By;
    p->op = TK_ALL;
    pSubitem->pSTab = p_توکی_Tab;
    إذا( pNew==0 ){
      p->p_مخکینی = p_مخکینی;
    }وإلا{
      pNew->selId = ++pParse->nSelect;
      إذا( aCsrMap && ALWAYS(db->mallocFailed==0) ){
        renumberCursors(pParse, pNew, iFrom, aCsrMap);
      }
      pNew->p_مخکینی = p_مخکینی;
      إذا( p_مخکینی ) p_مخکینی->p_بل = pNew;
      pNew->p_بل = p;
      p->p_مخکینی = pNew;
      TREETRACE(0x4,pParse,p,("compound-subquery flattener"
                              " creates %u as peer\n",pNew->selId));
    }
    أكد( pSubitem->fg.isSubquery==0 );
  }
  sqlite3DbFree(db, aCsrMap);
  إذا( db->mallocFailed ){
    أكد( pSubitem->fg.fixedSchema==0 );
    أكد( pSubitem->fg.isSubquery==0 );
    أكد( pSubitem->u4.zDatabase==0 );
    sqlite3_Src_توکی_Attach_Subquery(pParse, pSubitem, pSub1, 0);
    أرجع 1;
  }

  /* Defer deleting the Table object associated with the
  ** subquery until code generation is
  ** complete, since there may still exist Expr.pTab entries that
  ** refer to the subquery even after flattening.  Ticket #3346.
  **
  ** pSubitem->pSTab is always non-NULL by test restrictions and tests above.
  */
  إذا( ALWAYS(pSubitem->pSTab!=0) ){
    جدول *p_Tab_منزل_Del = pSubitem->pSTab;
    إذا( p_Tab_منزل_Del->nTabRef==1 ){
      Parse *pToplevel = sqlite3ParseToplevel(pParse);
      sqlite3_Parser_جمع_کړه_Cleanup(pToplevel, sqlite3_Delete_جدول_Generic, p_Tab_منزل_Del);
      testcase( pToplevel->earlyCleanup );
    }وإلا{
      p_Tab_منزل_Del->nTabRef--;
    }
    pSubitem->pSTab = 0;
  }

  /* The following loop runs once for each term in a compound-subquery
  ** flattening (as described above).  If we are doing a different kind
  ** of flattening - a flattening other than a compound-subquery flattening -
  ** then this loop only runs once.
  **
  ** This loop moves all of the FROM elements of the subquery into the
  ** the FROM clause of the outer query.  Before doing this, remember
  ** the cursor number for the original outer query FROM element in
  ** iParent.  The iParent cursor will never be used.  Subsequent code
  ** will scan expressions looking for iParent references and replace
  ** those references with expressions that resolve to the subquery FROM
  ** elements we are now copying in.
  */
  pSub = pSub1;
  لكل(p_پلار=p; p_پلار; p_پلار=p_پلار->p_مخکینی, pSub=pSub->p_مخکینی){
    صحيح nSubSrc;
    u8 jointype = p_پلار->pSrc->a[iFrom].fg.jointype;
    أكد( pSub!=0 );
    pSubSrc = pSub->pSrc;     /* FROM clause of subquery */
    nSubSrc = pSubSrc->nSrc;  /* Number of terms in subquery FROM clause */
    pSrc = p_پلار->pSrc;     /* FROM clause of the outer query */

    /* The subquery uses a single slot of the FROM clause of the outer
    ** query.  If the subquery has more than one element in its FROM clause,
    ** then expand the outer query to make space for it to hold all elements
    ** of the subquery.
    **
    ** Example:
    **
    **    SELECT * FROM tabA, (SELECT * FROM sub1, sub2), tabB;
    **
    ** The outer query has 3 slots in its FROM clause.  One slot of the
    ** outer query (the middle slot) is used by the subquery.  The next
    ** block of code will expand the outer query FROM clause to 4 slots.
    ** The middle slot is expanded to two slots in order to make space
    ** for the two elements in the FROM clause of the subquery.
    */
    إذا( nSubSrc>1 ){
      pSrc = sqlite3SrcListEnlarge(pParse, pSrc, nSubSrc-1,iFrom+1);
      إذا( pSrc==0 ) اكسر;
      p_پلار->pSrc = pSrc;
    }

    /* Transfer the FROM clause terms from the subquery into the
    ** outer query.
    */
    i_New_پلار = pSubSrc->a[0].iCursor;
    لكل(i=0; i<nSubSrc; i++){
      Src_توکی *p_توکی = &pSrc->a[i+iFrom];
      أكد( p_توکی->fg.isTabFunc==0 );
      أكد( p_توکی->fg.isSubquery
           || p_توکی->fg.fixedSchema
           || p_توکی->u4.zDatabase==0 );
      إذا( p_توکی->fg.isUsing ) sqlite3IdListDelete(db, p_توکی->u3.pUsing);
      *p_توکی = pSubSrc->a[i];
      p_توکی->fg.jointype |= (jointype & JT_LTORJ);
      memset(&pSubSrc->a[i], 0, حجم(pSubSrc->a[i]));
    }
    pSrc->a[iFrom].fg.jointype |= jointype;
 
    /* Begin substituting subquery result set expressions for
    ** references to the iParent in the outer query.
    **
    ** Example:
    **
    **   SELECT a+5, b*10 FROM (SELECT x*3 AS a, y+10 AS b FROM t1) WHERE a>b;
    **   \                     \_____________ subquery __________/          /
    **    \_____________________ outer query ______________________________/
    **
    ** We look at every expression in the outer query and every place we see
    ** "a" we substitute "x*3" and every place we see "b" we substitute "y+10".
    */
    إذا( pSub->p_ترتیب_By ){
      /* At this point, any non-zero iOrderByCol values indicate that the
      ** ORDER BY column expression is identical to the iOrderByCol'th
      ** expression returned by SELECT statement pSub. Since these values
      ** do not necessarily correspond to columns in SELECT statement pParent,
      ** zero them before transferring the ORDER BY clause.
      **
      ** Not doing this may cause an error if a subsequent call to this
      ** function attempts to flatten a compound sub-query into pParent.
      ** See ticket [d11a6e908f].
      */
      بیان_List *p_ترتیب_By = pSub->p_ترتیب_By;
      لكل(i=0; i<p_ترتیب_By->n_بیان; i++){
        p_ترتیب_By->a[i].u.x.i_ترتیب_By_ستون = 0;
      }
      أكد( p_پلار->p_ترتیب_By==0 );
      p_پلار->p_ترتیب_By = p_ترتیب_By;
      pSub->p_ترتیب_By = 0;
    }
    pWhere = pSub->pWhere;
    pSub->pWhere = 0;
    إذا( isOuterJoin>0 ){
      أكد( pSubSrc->nSrc==1 );
      sqlite3_سېټ_Join_بیان(pWhere, i_New_پلار, EP_OuterON);
    }
    إذا( pWhere ){
      إذا( p_پلار->pWhere ){
        p_پلار->pWhere = sqlite3PExpr(pParse, TK_AND, pWhere, p_پلار->pWhere);
      }وإلا{
        p_پلار->pWhere = pWhere;
      }
    }
    إذا( db->mallocFailed==0 ){
      SubstContext x;
      x.pParse = pParse;
      x.i_جدول = i_پلار;
      x.i_New_جدول = i_New_پلار;
      x.isOuterJoin = isOuterJoin;
      x.n_Sel_ژوروالی = 0;
      x.pEList = pSub->pEList;
      x.pCList = پیدا_کړه_Leftmost_Exprlist(pSub);
      substSelect(&x, p_پلار, 0);
    }
 
    /* The flattened query is a compound if either the inner or the
    ** outer query is a compound. */
    p_پلار->selFlags |= pSub->selFlags & SF_Compound;
    أكد( (pSub->selFlags & SF_Distinct)==0 ); /* restriction (17b) */
 
    /*
    ** SELECT ... FROM (SELECT ... LIMIT a OFFSET b) LIMIT x OFFSET y;
    **
    ** One is tempted to try to add a and b to combine the limits.  But this
    ** does not work if either limit is negative.
    */
    إذا( pSub->p_حد ){
      p_پلار->p_حد = pSub->p_حد;
      pSub->p_حد = 0;
    }

    /* Recompute the SrcItem.colUsed masks for the flattened
    ** tables. */
    لكل(i=0; i<nSubSrc; i++){
      recompute_Columns_کارول_شوی(p_پلار, &pSrc->a[i+iFrom]);
    }
  }

  /* Finally, delete what is left of the subquery and return success.
  */
  sqlite3AggInfoPersistWalkerInit(&w, pParse);
  sqlite3WalkSelect(&w,pSub1);
  sqlite3SelectDelete(db, pSub1);

#إذا TREETRACE_ENABLED
  إذا( sqlite3_ونه_Trace & 0x4 ){
    TREETRACE(0x4,pParse,p,("After flattening:\n"));
    sqlite3_ونه_View_Select(0, p, 0);
  }
#د_که_پای

  أرجع 1;
}
#د_که_پای /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** A structure to keep track of all of the column values that are fixed to
** a known value due to WHERE clause constraints of the form COLUMN=VALUE.
*/
تعريفنوع بنية WhereConst WhereConst;
بنية WhereConst {
  Parse *pParse;   /* Parsing context */
  u8 *pOomFault;   /* Pointer to pParse->db->mallocFailed */
  صحيح nConst;      /* Number for COLUMN=CONSTANT terms */
  صحيح nChng;       /* Number of times a constant is propagated */
  صحيح bHasAffBlob; /* At least one column in apExpr[] as affinity BLOB */
  u32 mExcludeOn;  /* Which ON expressions to exclude from considertion.
                   ** Either EP_OuterON or EP_InnerON|EP_OuterON */
  بیان **ap_بیان;   /* [i*2] is COLUMN and [i*2+1] is VALUE */
};

/*
** Add a new entry to the pConst object, if appropriate.
**
**    *   Do not add if pValue is not constant.  (This is enforced by
**        the caller)
**
**    *   Do not add duplicate pColumn entries
**
**    *   Do not add if pValue has an affinity
**
**    *   Do not add if the comparison uses a collating sequence other
**        than binary.
**
**    *   Do not add if pValue is a function that might return a subtype
**
** The caller guarantees the pColumn is a column.
*/
ساكن فراغ const_ورننباسه(
  WhereConst *pConst,  /* The WhereConst into which we are inserting */
  بیان *p_ستنه,       /* The COLUMN part of the constraint */
  بیان *p_ارزښت,        /* The VALUE part of the constraint */
  بیان *p_بیان          /* Overall expression: COLUMN=VALUE or VALUE=COLUMN */
){
  صحيح i;
  أكد( p_ستنه->op==TK_ستنه );
  أكد( sqlite3_بیان_Is_Constant(pConst->pParse, p_ارزښت) );
  إذا( بیان_Has_Property(p_ستنه, EP_Fixed_ستون) ) أرجع;
  إذا( sqlite3_بیان_Affinity(p_ارزښت)!=0 ) أرجع;
  إذا( !sqlite3_Is_دوګونی(sqlite3_بیان_Compare_Coll_Seq(pConst->pParse,p_بیان)) ){
    أرجع;
  }
  إذا( sqlite3_بیان_Can_Return_Subtype(pConst->pParse, p_ارزښت) ) أرجع;

  /* 2018-10-25 ticket [cf5ed20f]
  ** Make sure the same pColumn is not inserted more than once */
  لكل(i=0; i<pConst->nConst; i++){
    ثابت بیان *pE2 = pConst->ap_بیان[i*2];
    أكد( pE2->op==TK_ستنه );
    إذا( pE2->i_جدول==p_ستنه->i_جدول
     && pE2->i_ستنه==p_ستنه->i_ستنه
    ){
      أرجع;  /* Already present.  Return without doing anything. */
    }
  }
  أكد( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  إذا( sqlite3_بیان_Affinity(p_ستنه)<=SQLITE_AFF_BLOB ){
    pConst->bHasAffBlob = 1;
  }

  pConst->nConst++;
  pConst->ap_بیان = sqlite3DbReallocOrFree(pConst->pParse->db, pConst->ap_بیان,
                         pConst->nConst*2*حجم(بیان*));
  إذا( pConst->ap_بیان==0 ){
    pConst->nConst = 0;
  }وإلا{
    pConst->ap_بیان[pConst->nConst*2-2] = p_ستنه;
    pConst->ap_بیان[pConst->nConst*2-1] = p_ارزښت;
  }
}

/*
** Find all terms of COLUMN=VALUE or VALUE=COLUMN in pExpr where VALUE
** is a constant expression and where the term must be true because it
** is part of the AND-connected terms of the expression.  For each term
** found, add it to the pConst structure.
*/
ساكن فراغ پیدا_کړه_Const_دننه_Where(WhereConst *pConst, بیان *p_بیان){
  بیان *p_ښی, *p_کیڼ;
  إذا( NEVER(p_بیان==0) ) أرجع;
  إذا( بیان_Has_Property(p_بیان, pConst->mExcludeOn) ){
    testcase( بیان_Has_Property(p_بیان, EP_OuterON) );
    testcase( بیان_Has_Property(p_بیان, EP_InnerON) );
    أرجع;
  }
  إذا( p_بیان->op==TK_AND ){
    پیدا_کړه_Const_دننه_Where(pConst, p_بیان->p_ښی);
    پیدا_کړه_Const_دننه_Where(pConst, p_بیان->p_کیڼ);
    أرجع;
  }
  إذا( p_بیان->op!=TK_EQ ) أرجع;
  p_ښی = p_بیان->p_ښی;
  p_کیڼ = p_بیان->p_کیڼ;
  أكد( p_ښی!=0 );
  أكد( p_کیڼ!=0 );
  إذا( p_ښی->op==TK_ستنه && sqlite3_بیان_Is_Constant(pConst->pParse, p_کیڼ) ){
    const_ورننباسه(pConst,p_ښی,p_کیڼ,p_بیان);
  }
  إذا( p_کیڼ->op==TK_ستنه && sqlite3_بیان_Is_Constant(pConst->pParse, p_ښی) ){
    const_ورننباسه(pConst,p_کیڼ,p_ښی,p_بیان);
  }
}

/*
** This is a helper function for Walker callback propagateConstantExprRewrite().
**
** Argument pExpr is a candidate expression to be replaced by a value. If
** pExpr is equivalent to one of the columns named in pWalker->u.pConst,
** then overwrite it with the corresponding value. Except, do not do so
** if argument bIgnoreAffBlob is non-zero and the affinity of pExpr
** is SQLITE_AFF_BLOB.
*/
ساكن صحيح propagate_Constant_بیان_Rewrite_One(
  WhereConst *pConst,
  بیان *p_بیان,
  صحيح bIgnoreAffBlob
){
  صحيح i;
  إذا( pConst->pOomFault[0] ) أرجع WRC_Prune;
  إذا( p_بیان->op!=TK_ستنه ) أرجع WRC_Continue;
  إذا( بیان_Has_Property(p_بیان, EP_Fixed_ستون|pConst->mExcludeOn) ){
    testcase( بیان_Has_Property(p_بیان, EP_Fixed_ستون) );
    testcase( بیان_Has_Property(p_بیان, EP_OuterON) );
    testcase( بیان_Has_Property(p_بیان, EP_InnerON) );
    أرجع WRC_Continue;
  }
  لكل(i=0; i<pConst->nConst; i++){
    بیان *p_ستنه = pConst->ap_بیان[i*2];
    إذا( p_ستنه==p_بیان ) استمر;
    إذا( p_ستنه->i_جدول!=p_بیان->i_جدول ) استمر;
    إذا( p_ستنه->i_ستنه!=p_بیان->i_ستنه ) استمر;
    أكد( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
    إذا( bIgnoreAffBlob && sqlite3_بیان_Affinity(p_ستنه)<=SQLITE_AFF_BLOB ){
      اكسر;
    }
    /* A match is found.  Add the EP_FixedCol property */
    pConst->nChng++;
    بیان_Clear_Property(p_بیان, EP_ونې_پاڼه);
    بیان_سېټ_Property(p_بیان, EP_Fixed_ستون);
    أكد( p_بیان->p_کیڼ==0 );
    p_بیان->p_کیڼ = sqlite3_بیان_Dup(pConst->pParse->db, pConst->ap_بیان[i*2+1], 0);
    إذا( pConst->pParse->db->mallocFailed ) أرجع WRC_Prune;
    اكسر;
  }
  أرجع WRC_Prune;
}

/*
** This is a Walker expression callback. pExpr is a node from the WHERE
** clause of a SELECT statement. This function examines pExpr to see if
** any substitutions based on the contents of pWalker->u.pConst should
** be made to pExpr or its immediate children.
**
** A substitution is made if:
**
**   + pExpr is a column with an affinity other than BLOB that matches
**     one of the columns in pWalker->u.pConst, or
**
**   + pExpr is a binary comparison operator (=, <=, >=, <, >) that
**     uses an affinity other than TEXT and one of its immediate
**     children is a column that matches one of the columns in
**     pWalker->u.pConst.
*/
ساكن صحيح propagate_Constant_بیان_Rewrite(Walker *pWalker, بیان *p_بیان){
  WhereConst *pConst = pWalker->u.pConst;
  أكد( TK_GT==TK_EQ+1 );
  أكد( TK_LE==TK_EQ+2 );
  أكد( TK_LT==TK_EQ+3 );
  أكد( TK_GE==TK_EQ+4 );
  إذا( pConst->bHasAffBlob ){
    إذا( (p_بیان->op>=TK_EQ && p_بیان->op<=TK_GE)
     || p_بیان->op==TK_IS
    ){
      propagate_Constant_بیان_Rewrite_One(pConst, p_بیان->p_کیڼ, 0);
      إذا( pConst->pOomFault[0] ) أرجع WRC_Prune;
      إذا( sqlite3_بیان_Affinity(p_بیان->p_کیڼ)!=SQLITE_AFF_متن ){
        propagate_Constant_بیان_Rewrite_One(pConst, p_بیان->p_ښی, 0);
      }
    }
  }
  أرجع propagate_Constant_بیان_Rewrite_One(pConst, p_بیان, pConst->bHasAffBlob);
}

/*
** The WHERE-clause constant propagation optimization.
**
** If the WHERE clause contains terms of the form COLUMN=CONSTANT or
** CONSTANT=COLUMN that are top-level AND-connected terms that are not
** part of a ON clause from an OUTER JOIN, then throughout the query
** replace all other occurrences of COLUMN with CONSTANT.
**
** For example, the query:
**
**      SELECT * FROM t1, t2, t3 WHERE t1.a=39 AND t2.b=t1.a AND t3.c=t2.b
**
** Is transformed into
**
**      SELECT * FROM t1, t2, t3 WHERE t1.a=39 AND t2.b=39 AND t3.c=39
**
** Return true if any transformations where made and false if not.
**
** Implementation note:  Constant propagation is tricky due to affinity
** and collating sequence interactions.  Consider this example:
**
**    CREATE TABLE t1(a INT,b TEXT);
**    INSERT INTO t1 VALUES(123,'0123');
**    SELECT * FROM t1 WHERE a=123 AND b=a;
**    SELECT * FROM t1 WHERE a=123 AND b=123;
**
** The two SELECT statements above should return different answers.  b=a
** is always true because the comparison uses numeric affinity, but b=123
** is false because it uses text affinity and '0123' is not the same as '123'.
** To work around this, the expression tree is not actually changed from
** "b=a" to "b=123" but rather the "a" in "b=a" is tagged with EP_FixedCol
** and the "123" value is hung off of the pLeft pointer.  Code generator
** routines know to generate the constant "123" instead of looking up the
** column value.  Also, to avoid collation problems, this optimization is
** only attempted if the "a=123" term uses the default BINARY collation.
**
** 2021-05-25 forum post 6a06202608: Another troublesome case is...
**
**    CREATE TABLE t1(x);
**    INSERT INTO t1 VALUES(10.0);
**    SELECT 1 FROM t1 WHERE x=10 AND x LIKE 10;
**
** The query should return no rows, because the t1.x value is '10.0' not '10'
** and '10.0' is not LIKE '10'.  But if we are not careful, the first WHERE
** term "x=10" will cause the second WHERE term to become "10 LIKE 10",
** resulting in a false positive.  To avoid this, constant propagation for
** columns with BLOB affinity is only allowed if the constant is used with
** operators ==, <=, <, >=, >, or IS in a way that will cause the correct
** type conversions to occur.  See logic associated with the bHasAffBlob flag
** for details.
*/
ساكن صحيح propagateConstants(
  Parse *pParse,   /* The parsing context */
  Select *p        /* The query in which to propagate constants */
){
  WhereConst x;
  Walker w;
  صحيح nChng = 0;
  x.pParse = pParse;
  x.pOomFault = &pParse->db->mallocFailed;
  افعل{
    x.nConst = 0;
    x.nChng = 0;
    x.ap_بیان = 0;
    x.bHasAffBlob = 0;
    إذا( ALWAYS(p->pSrc!=0)
     && p->pSrc->nSrc>0
     && (p->pSrc->a[0].fg.jointype & JT_LTORJ)!=0
    ){
      /* Do not propagate constants on any ON clause if there is a
      ** RIGHT JOIN anywhere in the query */
      x.mExcludeOn = EP_InnerON | EP_OuterON;
    }وإلا{
      /* Do not propagate constants through the ON clause of a LEFT JOIN */
      x.mExcludeOn = EP_OuterON;
    }
    پیدا_کړه_Const_دننه_Where(&x, p->pWhere);
    إذا( x.nConst ){
      memset(&w, 0, حجم(w));
      w.pParse = pParse;
      w.x_بیان_Callback = propagate_Constant_بیان_Rewrite;
      w.xSelectCallback = sqlite3SelectWalkNoop;
      w.xSelectCallback2 = 0;
      w.walker_ژوروالی = 0;
      w.u.pConst = &x;
      sqlite3_Walk_بیان(&w, p->pWhere);
      sqlite3DbFree(x.pParse->db, x.ap_بیان);
      nChng += x.nChng;
    }
  }طالما( x.nChng ); 
  أرجع nChng;
}

#إذا !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
# إذا !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** This function is called to determine whether or not it is safe to
** push WHERE clause expression pExpr down to FROM clause sub-query
** pSubq, which contains at least one window function. Return 1
** if it is safe and the expression should be pushed down, or 0
** otherwise.
**
** It is only safe to push the expression down if it consists only
** of constants and copies of expressions that appear in the PARTITION
** BY clause of all window function used by the sub-query. It is safe
** to filter out entire partitions, but not rows within partitions, as
** this may change the results of the window functions.
**
** At the time this function is called it is guaranteed that
**
**   * the sub-query uses only one distinct window frame, and
**   * that the window frame has a PARTITION BY clause.
*/
ساكن صحيح ټېل_ښکته_کړکۍ_Check(Parse *pParse, Select *pSubq, بیان *p_بیان){
  أكد( pSubq->pWin->p_برخه_کول );
  أكد( (pSubq->selFlags & SF_MultiPart)==0 );
  أكد( pSubq->p_مخکینی==0 );
  أرجع sqlite3_بیان_Is_Constant_Or_ډله_By(pParse, p_بیان, pSubq->pWin->p_برخه_کول);
}
# endif /* SQLITE_OMIT_WINDOWFUNC */
#د_که_پای /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#إذا !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
/*
** Make copies of relevant WHERE clause terms of the outer query into
** the WHERE clause of subquery.  Example:
**
**    SELECT * FROM (SELECT a AS x, c-d AS y FROM t1) WHERE x=5 AND y=10;
**
** Transformed into:
**
**    SELECT * FROM (SELECT a AS x, c-d AS y FROM t1 WHERE a=5 AND c-d=10)
**     WHERE x=5 AND y=10;
**
** The hope is that the terms added to the inner query will make it more
** efficient.
**
** NAME AMBIGUITY
**
** This optimization is called the "WHERE-clause push-down optimization"
** or sometimes the "predicate push-down optimization".
**
** Do not confuse this optimization with another unrelated optimization
** with a similar name:  The "MySQL push-down optimization" causes WHERE
** clause terms that can be evaluated using only the index and without
** reference to the table are run first, so that if they are false,
** unnecessary table seeks are avoided.
**
** RULES
**
** Do not attempt this optimization if:
**
**   (1) (** This restriction was removed on 2017-09-29.  We used to
**           disallow this optimization for aggregate subqueries, but now
**           it is allowed by putting the extra terms on the HAVING clause.
**           The added HAVING clause is pointless if the subquery lacks
**           a GROUP BY clause.  But such a HAVING clause is also harmless
**           so there does not appear to be any reason to add extra logic
**           to suppress it. **)
**
**   (2) The inner query is the recursive part of a common table expression.
**
**   (3) The inner query has a LIMIT clause (since the changes to the WHERE
**       clause would change the meaning of the LIMIT).
**
**   (4) The inner query is the right operand of a LEFT JOIN and the
**       expression to be pushed down does not come from the ON clause
**       on that LEFT JOIN.
**
**   (5) The WHERE clause expression originates in the ON or USING clause
**       of a LEFT JOIN where iCursor is not the right-hand table of that
**       left join.  An example:
**
**           SELECT *
**           FROM (SELECT 1 AS a1 UNION ALL SELECT 2) AS aa
**           JOIN (SELECT 1 AS b2 UNION ALL SELECT 2) AS bb ON (a1=b2)
**           LEFT JOIN (SELECT 8 AS c3 UNION ALL SELECT 9) AS cc ON (b2=2);
**
**       The correct answer is three rows:  (1,1,NULL),(2,2,8),(2,2,9).
**       But if the (b2=2) term were to be pushed down into the bb subquery,
**       then the (1,1,NULL) row would be suppressed.
**
**   (6) Window functions make things tricky as changes to the WHERE clause
**       of the inner query could change the window over which window
**       functions are calculated. Therefore, do not attempt the optimization
**       if:
**
**     (6a) The inner query uses multiple incompatible window partitions.
**
**     (6b) The inner query is a compound and uses window-functions.
**
**     (6c) The WHERE clause does not consist entirely of constants and
**          copies of expressions found in the PARTITION BY clause of
**          all window-functions used by the sub-query. It is safe to
**          filter out entire partitions, as this does not change the
**          window over which any window-function is calculated.
**
**   (7) The inner query is a Common Table Expression (CTE) that should
**       be materialized.  (This restriction is implemented in the calling
**       routine.)
**
**   (8) If the subquery is a compound that uses UNION, INTERSECT,
**       or EXCEPT, then all of the result set columns for all arms of
**       the compound must use the BINARY collating sequence.
**
**   (9) All three of the following are true:
**
**       (9a) The WHERE clause expression originates in the ON or USING clause
**            of a join (either an INNER or an OUTER join), and
**
**       (9b) The subquery is to the right of the ON/USING clause
**
**       (9c) There is a RIGHT JOIN (or FULL JOIN) in between the ON/USING
**            clause and the subquery.
**
**       Without this restriction, the WHERE-clause push-down optimization
**       might move the ON/USING filter expression from the left side of a
**       RIGHT JOIN over to the right side, which leads to incorrect answers.
**       See also restriction (6) in sqlite3ExprIsSingleTableConstraint().
**
**  (10) The inner query is not the right-hand table of a RIGHT JOIN.
**
**  (11) The subquery is not a VALUES clause
**
**  (12) The WHERE clause is not "rowid ISNULL" or the equivalent.  This
**       case only comes up if SQLite is compiled using
**       SQLITE_ALLOW_ROWID_IN_VIEW.
**
** Return 0 if no changes are made and non-zero if one or more WHERE clause
** terms are duplicated into the subquery.
*/
ساكن صحيح ټېل_ښکته_Where_Terms(
  Parse *pParse,        /* Parse context (for malloc() and error reporting) */
  Select *pSubq,        /* The subquery whose WHERE clause is to be augmented */
  بیان *pWhere,         /* The WHERE clause of the outer query */
  SrcList *pSrcList,    /* The complete from clause of the outer query */
  صحيح iSrc              /* Which FROM clause term to try to push into  */
){
  بیان *pNew;
  Src_توکی *pSrc;        /* The subquery FROM term into which WHERE is pushed */
  صحيح nChng = 0;
  pSrc = &pSrcList->a[iSrc];
  إذا( pWhere==0 ) أرجع 0;
  إذا( pSubq->selFlags & (SF_Recursive|SF_MultiPart) ){
    أرجع 0;           /* restrictions (2) and (11) */
  }
  إذا( pSrc->fg.jointype & (JT_LTORJ|JT_ښی) ){
    أرجع 0;           /* restrictions (10) */
  }

  إذا( pSubq->p_مخکینی ){
    Select *pSel;
    صحيح notUnionAll = 0;
    لكل(pSel=pSubq; pSel; pSel=pSel->p_مخکینی){
      u8 op = pSel->op;
      أكد( op==TK_ALL || op==TK_SELECT
           || op==TK_UNION || op==TK_INTERSECT || op==TK_EXCEPT );
      إذا( op!=TK_ALL && op!=TK_SELECT ){
        notUnionAll = 1;
      }
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
      إذا( pSel->pWin ) أرجع 0;    /* restriction (6b) */
#د_که_پای
    }
    إذا( notUnionAll ){
      /* If any of the compound arms are connected using UNION, INTERSECT,
      ** or EXCEPT, then we must ensure that none of the columns use a
      ** non-BINARY collating sequence. */
      لكل(pSel=pSubq; pSel; pSel=pSel->p_مخکینی){
        صحيح ii;
        ثابت بیان_List *pList = pSel->pEList;
        أكد( pList!=0 );
        لكل(ii=0; ii<pList->n_بیان; ii++){
          CollSeq *pColl = sqlite3_بیان_Coll_Seq(pParse, pList->a[ii].p_بیان);
          إذا( !sqlite3_Is_دوګونی(pColl) ){
            أرجع 0;  /* Restriction (8) */
          }
        }
      }
    }
  }وإلا{
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
    إذا( pSubq->pWin && pSubq->pWin->p_برخه_کول==0 ) أرجع 0;
#د_که_پای
  }

#که_تعریف_شوی SQLITE_DEBUG
  /* Only the first term of a compound can have a WITH clause.  But make
  ** sure no other terms are marked SF_Recursive in case something changes
  ** in the future.
  */
  {
    Select *pX; 
    لكل(pX=pSubq; pX; pX=pX->p_مخکینی){
      أكد( (pX->selFlags & (SF_Recursive))==0 );
    }
  }
#د_که_پای

  إذا( pSubq->p_حد!=0 ){
    أرجع 0; /* restriction (3) */
  }
  طالما( pWhere->op==TK_AND ){
    nChng += ټېل_ښکته_Where_Terms(pParse, pSubq, pWhere->p_ښی, pSrcList, iSrc);
    pWhere = pWhere->p_کیڼ;
  }

#إذا 0 /* These checks now done by sqlite3ExprIsSingleTableConstraint() */
  إذا( بیان_Has_Property(pWhere, EP_OuterON|EP_InnerON) /* (9a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (9c) */
  ){
    صحيح jj;
    لكل(jj=0; jj<iSrc; jj++){
      إذا( pWhere->w.iJoin==pSrcList->a[jj].iCursor ){
        /* If we reach this point, both (9a) and (9b) are satisfied.
        ** The following loop checks (9c):
        */
        لكل(jj++; jj<iSrc; jj++){
          إذا( (pSrcList->a[jj].fg.jointype & JT_ښی)!=0 ){
            أرجع 0;  /* restriction (9) */
          }
        }
      }
    }
  }
  إذا( is_کیڼ_Join
   && (بیان_Has_Property(pWhere,EP_OuterON)==0
         || pWhere->w.iJoin!=iCursor)
  ){
    أرجع 0; /* restriction (4) */
  }
  إذا( بیان_Has_Property(pWhere,EP_OuterON)
   && pWhere->w.iJoin!=iCursor
  ){
    أرجع 0; /* restriction (5) */
  }
#د_که_پای

#که_تعریف_شوی SQLITE_ALLOW_ROWID_دننه_VIEW
  إذا( ViewCanHaveRowid && (pWhere->op==TK_ISNULL || pWhere->op==TK_NOTNULL) ){
    بیان *p_کیڼ = pWhere->p_کیڼ;
    إذا( ALWAYS(p_کیڼ) 
     && p_کیڼ->op==TK_ستنه
     && p_کیڼ->i_ستنه < 0
    ){
      أرجع 0;  /* Restriction (12) */
    }
  }
#د_که_پای

  إذا( sqlite3_بیان_Is_Single_جدول_Constraint(pWhere, pSrcList, iSrc, 1) ){
    nChng++;
    pSubq->selFlags |= SF_ټېل_ښکته;
    طالما( pSubq ){
      SubstContext x;
      pNew = sqlite3_بیان_Dup(pParse->db, pWhere, 0);
      unset_Join_بیان(pNew, -1, 1);
      x.pParse = pParse;
      x.i_جدول = pSrc->iCursor;
      x.i_New_جدول = pSrc->iCursor;
      x.isOuterJoin = 0;
      x.n_Sel_ژوروالی = 0;
      x.pEList = pSubq->pEList;
      x.pCList = پیدا_کړه_Leftmost_Exprlist(pSubq);
      pNew = subst_بیان(&x, pNew);
      أكد( pNew!=0 || pParse->nErr!=0 );
      إذا( pParse->nErr==0 && pNew->op==TK_دننه && بیان_Use_XSelect(pNew) ){
        أكد( pNew->x.pSelect!=0 );
        pNew->x.pSelect->selFlags |= SF_Cloned_Rhs_دننه;
        أكد( pWhere!=0 );
        أكد( pWhere->op==TK_دننه );
        أكد( بیان_Use_XSelect(pWhere) );
        أكد( pWhere->x.pSelect!=0 );
        pWhere->x.pSelect->selFlags |= SF_Cloned_Rhs_دننه;
      }
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
      إذا( pSubq->pWin && 0==ټېل_ښکته_کړکۍ_Check(pParse, pSubq, pNew) ){
        /* Restriction 6c has prevented push-down in this case */
        sqlite3_بیان_Delete(pParse->db, pNew);
        nChng--;
        اكسر;
      }
#د_که_پای
      إذا( pSubq->selFlags & SF_Aggregate ){
        pSubq->pHaving = sqlite3_بیان_And(pParse, pSubq->pHaving, pNew);
      }وإلا{
        pSubq->pWhere = sqlite3_بیان_And(pParse, pSubq->pWhere, pNew);
      }
      pSubq = pSubq->p_مخکینی;
    }
  }
  أرجع nChng;
}
#د_که_پای /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** Check to see if a subquery contains result-set columns that are
** never used.  If it does, change the value of those result-set columns
** to NULL so that they do not cause unnecessary work to compute.
**
** Return the number of column that were changed to NULL.
*/
ساكن صحيح disable_ناکارول_شوی_Subquery_پایله_Columns(Src_توکی *p_توکی){
  صحيح n_ستون;
  Select *pSub;      /* The subquery to be simplified */
  Select *pX;        /* For looping over compound elements of pSub */
  جدول *pTab;       /* The table that describes the subquery */
  صحيح j;             /* Column number */
  صحيح nChng = 0;     /* Number of columns converted to NULL */
  Bitmask ستون_کارول_شوی;   /* Columns that may not be NULLed out */

  أكد( p_توکی!=0 );
  إذا( p_توکی->fg.isCorrelated || p_توکی->fg.isCte ){
    أرجع 0;
  }
  أكد( p_توکی->pSTab!=0 );
  pTab = p_توکی->pSTab;
  أكد( p_توکی->fg.isSubquery );
  pSub = p_توکی->u4.pSubq->pSelect;
  أكد( pSub->pEList->n_بیان==pTab->n_ستون );
  لكل(pX=pSub; pX; pX=pX->p_مخکینی){
    إذا( (pX->selFlags & (SF_Distinct|SF_Aggregate))!=0 ){
      testcase( pX->selFlags & SF_Distinct );
      testcase( pX->selFlags & SF_Aggregate );
      أرجع 0;
    }
    إذا( pX->p_مخکینی && pX->op!=TK_ALL ){
      /* This optimization does not work for compound subqueries that
      ** use UNION, INTERSECT, or EXCEPT.  Only UNION ALL is allowed. */
      أرجع 0;
    }
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
    إذا( pX->pWin ){
      /* This optimization does not work for subqueries that use window
      ** functions. */
      أرجع 0;
    }
#د_که_پای
  }
  ستون_کارول_شوی = p_توکی->ستون_کارول_شوی;
  إذا( pSub->p_ترتیب_By ){
    بیان_List *pList = pSub->p_ترتیب_By;
    لكل(j=0; j<pList->n_بیان; j++){
      u16 i_ستون = pList->a[j].u.x.i_ترتیب_By_ستون;
      إذا( i_ستون>0 ){
        i_ستون--;
        ستون_کارول_شوی |= ((Bitmask)1)<<(i_ستون>=BMS ? BMS-1 : i_ستون);
      }
    }
  }
  n_ستون = pTab->n_ستون;
  لكل(j=0; j<n_ستون; j++){
    Bitmask m = j<BMS-1 ? MASKBIT(j) : TOPBIT;
    إذا( (m & ستون_کارول_شوی)!=0 ) استمر;
    لكل(pX=pSub; pX; pX=pX->p_مخکینی) {
      بیان *pY = pX->pEList->a[j].p_بیان;
      إذا( pY->op==TK_NULL ) استمر;
      pY->op = TK_NULL;
      بیان_Clear_Property(pY, EP_Skip|EP_Unlikely);
      pX->selFlags |= SF_ټېل_ښکته;
      nChng++;
    }
  }
  أرجع nChng;
}


/*
** The pFunc is the only aggregate function in the query.  Check to see
** if the query is a candidate for the min/max optimization.
**
** If the query is a candidate for the min/max optimization, then set
** *ppMinMax to be an ORDER BY clause to be used for the optimization
** and return either WHERE_ORDERBY_MIN or WHERE_ORDERBY_MAX depending on
** whether pFunc is a min() or max() function.
**
** If the query is not a candidate for the min/max optimization, return
** WHERE_ORDERBY_NORMAL (which must be zero).
**
** This routine must be called after aggregate functions have been
** located but before their arguments have been subjected to aggregate
** analysis.
*/
ساكن u8 minMaxQuery(sqlite3 *db, بیان *pFunc, بیان_List **ppMinMax){
  صحيح eRet = WHERE_ORDERBY_نورمال;      /* Return value */
  بیان_List *pEList;                     /* Arguments to agg function */
  ثابت حرف *zFunc;                    /* Name of aggregate function pFunc */
  بیان_List *p_ترتیب_By;
  u8 ترتیبول_Flags = 0;

  أكد( *ppMinMax==0 );
  أكد( pFunc->op==TK_AGG_FUNCTION );
  أكد( !Is_کړکۍ_Func(pFunc) );
  أكد( بیان_Use_XList(pFunc) );
  pEList = pFunc->x.pList;
  إذا( pEList==0
   || pEList->n_بیان!=1
   || بیان_Has_Property(pFunc, EP_WinFunc)
   || OptimizationDisabled(db, SQLITE_MinMaxOpt)
  ){
    أرجع eRet;
  }
  أكد( !بیان_Has_Property(pFunc, EP_Int_ارزښت) );
  zFunc = pFunc->u.zToken;
  إذا( sqlite3StrICmp(zFunc, "min")==0 ){
    eRet = WHERE_ORDERBY_MIN;
    إذا( sqlite3_بیان_Can_Be_Null(pEList->a[0].p_بیان) ){
      ترتیبول_Flags = KEYINFO_ترتیب_BIGNULL;
    }
  }وإلا إذا( sqlite3StrICmp(zFunc, "max")==0 ){
    eRet = WHERE_ORDERBY_MAX;
    ترتیبول_Flags = KEYINFO_ترتیب_DESC;
  }وإلا{
    أرجع eRet;
  }
  *ppMinMax = p_ترتیب_By = sqlite3_بیان_List_Dup(db, pEList, 0);
  أكد( p_ترتیب_By!=0 || db->mallocFailed );
  إذا( p_ترتیب_By ) p_ترتیب_By->a[0].fg.ترتیبول_Flags = ترتیبول_Flags;
  أرجع eRet;
}

/*
** The select statement passed as the first argument is an aggregate query.
** The second argument is the associated aggregate-info object. This
** function tests if the SELECT is of the form:
**
**   SELECT count(*) FROM <tbl>
**
** where table is a database table, not a sub-select or view. If the query
** does match this pattern, then a pointer to the Table object representing
** <tbl> is returned. Otherwise, NULL is returned.
**
** This routine checks to see if it is safe to use the count optimization.
** A correct answer is still obtained (though perhaps more slowly) if
** this routine returns NULL when it could have returned a table pointer.
** But returning the pointer when NULL should have been returned can
** result in incorrect answers and/or crashes.  So, when in doubt, return NULL.
*/
ساكن جدول *is_Simple_شمېر(Select *p, AggInfo *pAggInfo){
  جدول *pTab;
  بیان *p_بیان;

  أكد( !p->p_ډله_By );

  إذا( p->pWhere
   || p->pEList->n_بیان!=1
   || p->pSrc->nSrc!=1
   || p->pSrc->a[0].fg.isSubquery
   || pAggInfo->nFunc!=1
   || p->pHaving
  ){
    أرجع 0;
  }
  pTab = p->pSrc->a[0].pSTab;
  أكد( pTab!=0 );
  أكد( !IsView(pTab) );
  إذا( !Is_Ordinary_جدول(pTab) ) أرجع 0;
  p_بیان = p->pEList->a[0].p_بیان;
  أكد( p_بیان!=0 );
  إذا( p_بیان->op!=TK_AGG_FUNCTION ) أرجع 0;
  إذا( p_بیان->pAggInfo!=pAggInfo ) أرجع 0;
  إذا( (pAggInfo->aFunc[0].pFunc->funcFlags&SQLITE_FUNC_شمېر)==0 ) أرجع 0;
  أكد( pAggInfo->aFunc[0].pFExpr==p_بیان );
  testcase( بیان_Has_Property(p_بیان, EP_Distinct) );
  testcase( بیان_Has_Property(p_بیان, EP_WinFunc) );
  إذا( بیان_Has_Property(p_بیان, EP_Distinct|EP_WinFunc) ) أرجع 0;

  أرجع pTab;
}

/*
** If the source-list item passed as an argument was augmented with an
** INDEXED BY clause, then try to locate the specified index. If there
** was such a clause and the named index cannot be found, return
** SQLITE_ERROR and leave an error in pParse. Otherwise, populate
** pFrom->pIndex and return SQLITE_OK.
*/
صحيح sqlite3_شاخصي_By_Lookup(Parse *pParse, Src_توکی *pFrom){
  جدول *pTab = pFrom->pSTab;
  حرف *z_شاخصي_By = pFrom->u1.z_شاخصي_By;
  شاخص *pIdx;
  أكد( pTab!=0 );
  أكد( pFrom->fg.is_شاخصي_By!=0 );

  لكل(pIdx=pTab->p_شاخص;
      pIdx && sqlite3StrICmp(pIdx->zName, z_شاخصي_By);
      pIdx=pIdx->p_بل
  );
  إذا( !pIdx ){
    sqlite3_Error_پیغام(pParse, "no such index: %s", z_شاخصي_By, 0);
    pParse->checkSchema = 1;
    أرجع SQLITE_ERROR;
  }
  أكد( pFrom->fg.isCte==0 );
  pFrom->u2.pIBIndex = pIdx;
  أرجع SQLITE_OK;
}

/*
** Detect compound SELECT statements that use an ORDER BY clause with
** an alternative collating sequence.
**
**    SELECT ... FROM t1 EXCEPT SELECT ... FROM t2 ORDER BY .. COLLATE ...
**
** These are rewritten as a subquery:
**
**    SELECT * FROM (SELECT ... FROM t1 EXCEPT SELECT ... FROM t2)
**     ORDER BY ... COLLATE ...
**
** This transformation is necessary because the multiSelectByMerge() routine
** above that generates the code for a compound SELECT with an ORDER BY clause
** uses a merge algorithm that requires the same collating sequence on the
** result columns as on the ORDER BY clause.  See ticket
** http://sqlite.org/src/info/6709574d2a
**
** This transformation is only needed for EXCEPT, INTERSECT, and UNION.
** The UNION ALL operator works fine with multiSelectByMerge() even when
** there are COLLATE terms in the ORDER BY.
*/
ساكن صحيح convert_Compound_Select_منزل_Subquery(Walker *pWalker, Select *p){
  صحيح i;
  Select *pNew;
  Select *pX;
  sqlite3 *db;
  بنية بیان_List_توکی *a;
  SrcList *pNewSrc;
  Parse *pParse;
  Token dummy;

  إذا( p->p_مخکینی==0 ) أرجع WRC_Continue;
  إذا( p->p_ترتیب_By==0 ) أرجع WRC_Continue;
  لكل(pX=p; pX && (pX->op==TK_ALL || pX->op==TK_SELECT); pX=pX->p_مخکینی){}
  إذا( pX==0 ) أرجع WRC_Continue;
  a = p->p_ترتیب_By->a;
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
  /* If iOrderByCol is already non-zero, then it has already been matched
  ** to a result column of the SELECT statement. This occurs when the
  ** SELECT is rewritten for window-functions processing and then passed
  ** to sqlite3SelectPrep() and similar a second time. The rewriting done
  ** by this function is not required in this case. */
  إذا( a[0].u.x.i_ترتیب_By_ستون ) أرجع WRC_Continue;
#د_که_پای
  لكل(i=p->p_ترتیب_By->n_بیان-1; i>=0; i--){
    إذا( a[i].p_بیان->flags & EP_Collate ) اكسر;
  }
  إذا( i<0 ) أرجع WRC_Continue;

  /* If we reach this point, that means the transformation is required. */

  pParse = pWalker->pParse;
  db = pParse->db;
  pNew = sqlite3DbMallocZero(db, حجم(*pNew) );
  إذا( pNew==0 ) أرجع WRC_Abort;
  memset(&dummy, 0, حجم(dummy));
  pNewSrc = sqlite3_Src_List_Append_From_غړی(pParse,0,0,0,&dummy,pNew,0);
  أكد( pNewSrc!=0 || pParse->nErr );
  إذا( pParse->nErr ){
    sqlite3SrcListDelete(db, pNewSrc);
    أرجع WRC_Abort;
  }
  *pNew = *p;
  p->pSrc = pNewSrc;
  p->pEList = sqlite3_بیان_List_Append(pParse, 0, sqlite3_بیان(db, TK_ASTERISK, 0));
  p->op = TK_SELECT;
  p->pWhere = 0;
  p->p_ډله_By = 0;
  p->pHaving = 0;
  pNew->p_ترتیب_By = 0;
  p->p_مخکینی = 0;
  p->p_بل = 0;
  p->pWith = 0;
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
  p->pWinDefn = 0;
#د_که_پای
  p->selFlags &= ~(u32)(SF_Compound|SF_Distinct);
  أكد( (p->selFlags & SF_Converted)==0 );
  p->selFlags |= SF_Converted;
  أكد( pNew->p_مخکینی!=0 );
  pNew->p_مخکینی->p_بل = pNew;
  pNew->p_حد = 0;
  أرجع WRC_Continue;
}

/*
** Check to see if the FROM clause term pFrom has table-valued function
** arguments.  If it does, leave an error message in pParse and return
** non-zero, since pFrom is not allowed to be a table-valued function.
*/
ساكن صحيح cannotBeFunction(Parse *pParse, Src_توکی *pFrom){
  إذا( pFrom->fg.isTabFunc ){
    sqlite3_Error_پیغام(pParse, "'%s' is not a function", pFrom->zName);
    أرجع 1;
  }
  أرجع 0;
}

#که_نه_تعریف_شوی SQLITE_OMIT_CTE
/*
** Argument pWith (which may be NULL) points to a linked list of nested
** WITH contexts, from inner to outermost. If the table identified by
** FROM clause element pItem is really a common-table-expression (CTE)
** then return a pointer to the CTE definition for that table. Otherwise
** return NULL.
**
** If a non-NULL value is returned, set *ppContext to point to the With
** object that the returned CTE belongs to.
*/
ساكن بنية Cte *پلټنه_With(
  With *pWith,                    /* Current innermost WITH clause */
  Src_توکی *p_توکی,                 /* FROM clause element to resolve */
  With **ppContext                /* OUT: WITH clause return value belongs to */
){
  ثابت حرف *zName = p_توکی->zName;
  With *p;
  أكد( p_توکی->fg.fixedSchema || p_توکی->u4.zDatabase==0 );
  أكد( zName!=0 );
  لكل(p=pWith; p; p=p->pOuter){
    صحيح i;
    لكل(i=0; i<p->nCte; i++){
      إذا( sqlite3StrICmp(zName, p->a[i].zName)==0 ){
        *ppContext = p;
        أرجع &p->a[i];
      }
    }
    إذا( p->bView ) اكسر;
  }
  أرجع 0;
}

/* The code generator maintains a stack of active WITH clauses
** with the inner-most WITH clause being at the top of the stack.
**
** This routine pushes the WITH clause passed as the second argument
** onto the top of the stack. If argument bFree is true, then this
** WITH clause will never be popped from the stack but should instead
** be freed along with the Parse object. In other cases, when
** bFree==0, the With object will be freed along with the SELECT
** statement with which it is associated.
**
** This routine returns a copy of pWith.  Or, if bFree is true and
** the pWith object is destroyed immediately due to an OOM condition,
** then this routine return NULL.
**
** If bFree is true, do not continue to use the pWith pointer after
** calling this routine,  Instead, use only the return value.
*/
With *sqlite3_With_ټېل(Parse *pParse, With *pWith, u8 bFree){
  إذا( pWith ){
    إذا( bFree ){
      pWith = (With*)sqlite3_Parser_جمع_کړه_Cleanup(pParse, sqlite3WithDeleteGeneric,
                      pWith);
      إذا( pWith==0 ) أرجع 0;
    }
    إذا( pParse->nErr==0 ){
      أكد( pParse->pWith!=pWith );
      pWith->pOuter = pParse->pWith;
      pParse->pWith = pWith;
    }
  }
  أرجع pWith;
}

ساكن صحيح selectExpanderWalk(Walker*, Select*);

/*
** This function checks to see if argument pFrom refers to a CTE declared by
** a WITH clause on the stack currently maintained by the parser (on the
** pParse->pWith linked list).  And if currently processing a CTE expression,
** it also checks to see if the reference is a recursive reference to the CTE.
**
** If pFrom matches a CTE, pFrom->pSTab and other fields are populated
** accordingly.
**
** Return 0 if no match is found.
** Return 1 if a match is found.
** Return 2 if an error condition is detected.
*/
ساكن صحيح resolve_From_غړی_منزل_Cte(
  Parse *pParse,                  /* The parsing context */
  Walker *pWalker,                /* Current tree walker */
  Src_توکی *pFrom                  /* The FROM clause term to check */
){
  Cte *pCte;               /* Matched CTE (or NULL if no match) */
  With *pWith;             /* The matching WITH */

  أكد( pFrom->pSTab==0 );
  إذا( pParse->pWith==0 ){
    /* There are no WITH clauses in the stack.  No match is possible */
    أرجع 0;
  }
  إذا( pParse->nErr ){
    /* Prior errors might have left pParse->pWith in a goofy state, so
    ** go no further. */
    أرجع 0;
  }
  أكد( pFrom->fg.hadSchema==0 || pFrom->fg.notCte!=0 );
  إذا( pFrom->fg.fixedSchema==0 && pFrom->u4.zDatabase!=0 ){
    /* The FROM term contains a schema qualifier (ex: main.t1) and so
    ** it cannot possibly be a CTE reference. */
    أرجع 0;
  }
  إذا( pFrom->fg.notCte ){
    /* The FROM term is specifically excluded from matching a CTE.
    **   (1)  It is part of a trigger that used to have zDatabase but had
    **        zDatabase removed by sqlite3FixTriggerStep().
    **   (2)  This is the first term in the FROM clause of an UPDATE.
    */
    أرجع 0;
  }
  pCte = پلټنه_With(pParse->pWith, pFrom, &pWith);
  إذا( pCte ){
    sqlite3 *db = pParse->db;
    جدول *pTab;
    بیان_List *pEList;
    Select *pSel;
    Select *p_کیڼ;                /* Left-most SELECT statement */
    Select *p_Rec_غړی;             /* Left-most recursive term */
    صحيح bMayRecursive;            /* True if compound joined by UNION [ALL] */
    With *pSavedWith;             /* Initial value of pParse->pWith */
    صحيح iRecTab = -1;             /* Cursor for recursive table */
    CteUse *pCteUse;

    /* If pCte->zCteErr is non-NULL at this point, then this is an illegal
    ** recursive reference to CTE pCte. Leave an error in pParse and return
    ** early. If pCte->zCteErr is NULL, then this is not a recursive reference.
    ** In this case, proceed.  */
    إذا( pCte->zCteErr ){
      sqlite3_Error_پیغام(pParse, pCte->zCteErr, pCte->zName);
      أرجع 2;
    }
    إذا( cannotBeFunction(pParse, pFrom) ) أرجع 2;

    أكد( pFrom->pSTab==0 );
    pTab = sqlite3DbMallocZero(db, حجم(جدول));
    إذا( pTab==0 ) أرجع 2;
    pCteUse = pCte->pUse;
    إذا( pCteUse==0 ){
      pCte->pUse = pCteUse = sqlite3DbMallocZero(db, حجم(pCteUse[0]));
      إذا( pCteUse==0
       || sqlite3_Parser_جمع_کړه_Cleanup(pParse,sqlite3DbFree,pCteUse)==0
      ){
        sqlite3DbFree(db, pTab);
        أرجع 2;
      }
      pCteUse->eM10d = pCte->eM10d;
    }
    pFrom->pSTab = pTab;
    pTab->nTabRef = 1;
    pTab->zName = sqlite3DbStrDup(db, pCte->zName);
    pTab->iPKey = -1;
    pTab->n_لیکه_Log_Est = 200; أكد( 200==sqlite3LogEst(1048576) );
    pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
    sqlite3_Src_توکی_Attach_Subquery(pParse, pFrom, pCte->pSelect, 1);
    إذا( db->mallocFailed ) أرجع 2;
    أكد( pFrom->fg.isSubquery && pFrom->u4.pSubq );
    pSel = pFrom->u4.pSubq->pSelect;
    أكد( pSel!=0 );
    pSel->selFlags |= SF_CopyCte;
    إذا( pFrom->fg.is_شاخصي_By ){
      sqlite3_Error_پیغام(pParse, "no such index: \"%s\"", pFrom->u1.z_شاخصي_By);
      أرجع 2;
    }
    أكد( !pFrom->fg.is_شاخصي_By );
    pFrom->fg.isCte = 1;
    pFrom->u2.pCteUse = pCteUse;
    pCteUse->nUse++;

    /* Check if this is a recursive CTE. */
    p_Rec_غړی = pSel;
    bMayRecursive = ( pSel->op==TK_ALL || pSel->op==TK_UNION );
    طالما( bMayRecursive && p_Rec_غړی->op==pSel->op ){
      صحيح i;
      SrcList *pSrc = p_Rec_غړی->pSrc;
      أكد( p_Rec_غړی->p_مخکینی!=0 );
      لكل(i=0; i<pSrc->nSrc; i++){
        Src_توکی *p_توکی = &pSrc->a[i];
        إذا( p_توکی->zName!=0
         && !p_توکی->fg.hadSchema
         && ALWAYS( !p_توکی->fg.isSubquery )
         && (p_توکی->fg.fixedSchema || p_توکی->u4.zDatabase==0)
         && 0==sqlite3StrICmp(p_توکی->zName, pCte->zName)
        ){
          p_توکی->pSTab = pTab;
          pTab->nTabRef++;
          p_توکی->fg.isRecursive = 1;
          إذا( p_Rec_غړی->selFlags & SF_Recursive ){
            sqlite3_Error_پیغام(pParse,
               "multiple references to recursive table: %s", pCte->zName
            );
            أرجع 2;
          }
          p_Rec_غړی->selFlags |= SF_Recursive;
          إذا( iRecTab<0 ) iRecTab = pParse->nTab++;
          p_توکی->iCursor = iRecTab;
        }
      }
      إذا( (p_Rec_غړی->selFlags & SF_Recursive)==0 ) اكسر;
      p_Rec_غړی = p_Rec_غړی->p_مخکینی;
    }

    إذا( pParse->nTab >= db->a_حد[SQLITE_حد_CTES]
     && db->a_حد[SQLITE_حد_CTES]>0
    ){
      sqlite3_Error_پیغام(pParse, "too many CTEs");
      أرجع 2;
    }
#إذا SQLITE_MAX_بیان_ژوروالی>0
    pParse->nNestSel++;
    إذا( check_Subquery_Nesting_ژوروالی(pParse) ) أرجع 2;
#د_که_پای
    pCte->zCteErr = "circular reference: %s";
    pSavedWith = pParse->pWith;
    pParse->pWith = pWith;
    إذا( pSel->selFlags & SF_Recursive ){
      صحيح rc;
      أكد( p_Rec_غړی!=0 );
      أكد( (p_Rec_غړی->selFlags & SF_Recursive)==0 );
      أكد( p_Rec_غړی->p_بل!=0 );
      أكد( (p_Rec_غړی->p_بل->selFlags & SF_Recursive)!=0 );
      أكد( p_Rec_غړی->pWith==0 );
      p_Rec_غړی->pWith = pSel->pWith;
      rc = sqlite3WalkSelect(pWalker, p_Rec_غړی);
      p_Rec_غړی->pWith = 0;
      إذا( rc ){
        pParse->pWith = pSavedWith;
        أرجع 2;
      }
    }وإلا{
      إذا( selectExpanderWalk(pWalker, pSel) ){
        pParse->pWith = pSavedWith;
        أرجع 2;
      }
    }
    pParse->pWith = pWith;
#إذا SQLITE_MAX_بیان_ژوروالی>0
    pParse->nNestSel--;
    أكد( pParse->nNestSel>=0 );
#د_که_پای

    لكل(p_کیڼ=pSel; p_کیڼ->p_مخکینی; p_کیڼ=p_کیڼ->p_مخکینی);
    pEList = p_کیڼ->pEList;
    إذا( pCte->p_ستون_شمېر ){
      إذا( pEList && pEList->n_بیان!=pCte->p_ستون_شمېر->n_بیان ){
        sqlite3_Error_پیغام(pParse, "table %s has %d values for %d columns",
            pCte->zName, pEList->n_بیان, pCte->p_ستون_شمېر->n_بیان
        );
        pParse->pWith = pSavedWith;
        أرجع 2;
      }
      pEList = pCte->p_ستون_شمېر;
    }

    sqlite3_Columns_From_بیان_List(pParse, pEList, &pTab->n_ستون, &pTab->a_ستون);
    إذا( bMayRecursive ){
      إذا( pSel->selFlags & SF_Recursive ){
        pCte->zCteErr = "multiple recursive references: %s";
      }وإلا{
        pCte->zCteErr = "recursive reference in a subquery: %s";
      }
      sqlite3WalkSelect(pWalker, pSel);
    }
    pCte->zCteErr = 0;
    pParse->pWith = pSavedWith;
    أرجع 1;  /* Success */
  }
  أرجع 0;  /* No match */
}
#د_که_پای

#که_نه_تعریف_شوی SQLITE_OMIT_CTE
/*
** If the SELECT passed as the second argument has an associated WITH
** clause, pop it from the stack stored as part of the Parse object.
**
** This function is used as the xSelectCallback2() callback by
** sqlite3SelectExpand() when walking a SELECT tree to resolve table
** names and other FROM clause elements.
*/
فراغ sqlite3_Select_راوباسه_With(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  إذا( OK_IF_ALWAYS_TRUE(pParse->pWith) && p->p_مخکینی==0 ){
    With *pWith = پیدا_کړه_Rightmost(p)->pWith;
    إذا( pWith!=0 ){
      أكد( pParse->pWith==pWith || pParse->nErr );
      pParse->pWith = pWith->pOuter;
    }
  }
}
#د_که_پای

/*
** The SrcItem structure passed as the second argument represents a
** sub-query in the FROM clause of a SELECT statement. This function
** allocates and populates the SrcItem.pTab object. If successful,
** SQLITE_OK is returned. Otherwise, if an OOM error is encountered,
** SQLITE_NOMEM.
*/
صحيح sqlite3ExpandSubquery(Parse *pParse, Src_توکی *pFrom){
  Select *pSel;
  جدول *pTab;

  أكد( pFrom->fg.isSubquery );
  أكد( pFrom->u4.pSubq!=0 );
  pSel = pFrom->u4.pSubq->pSelect;
  أكد( pSel );
  pFrom->pSTab = pTab = sqlite3DbMallocZero(pParse->db, حجم(جدول));
  إذا( pTab==0 ) أرجع SQLITE_NOMEM;
  pTab->nTabRef = 1;
  إذا( pFrom->zAlias ){
    pTab->zName = sqlite3DbStrDup(pParse->db, pFrom->zAlias);
  }وإلا{
    pTab->zName = sqlite3MPrintf(pParse->db, "%!S", pFrom);
  }
  طالما( pSel->p_مخکینی ){ pSel = pSel->p_مخکینی; }
  sqlite3_Columns_From_بیان_List(pParse, pSel->pEList,&pTab->n_ستون,&pTab->a_ستون);
  pTab->iPKey = -1;
  pTab->eTabType = TABTYP_VIEW;
  pTab->n_لیکه_Log_Est = 200; أكد( 200==sqlite3LogEst(1048576) );
#که_نه_تعریف_شوی SQLITE_ALLOW_ROWID_دننه_VIEW
  /* The usual case - do not allow ROWID on a subquery */
  pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
#وإلا
  /* Legacy compatibility mode */
  pTab->tabFlags |= TF_Ephemeral | sqlite3Config.mNoVisibleRowid;
#د_که_پای
  أرجع pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
}


/*
** Check the N SrcItem objects to the right of pBase.  (N might be zero!)
** If any of those SrcItem objects have a USING clause containing zName
** then return true.
**
** If N is zero, or none of the N SrcItem objects to the right of pBase
** contains a USING clause, or if none of the USING clauses contain zName,
** then return false.
*/
ساكن صحيح دننه_Any_Using_Clause(
  ثابت حرف *zName, /* Name we are looking for */
  Src_توکی *p_بنسټ,    /* The base SrcItem.  Looking at pBase[1] and following */
  صحيح N              /* How many SrcItems to check */
){
  طالما( N>0 ){
    N--;
    p_بنسټ++;
    إذا( p_بنسټ->fg.isUsing==0 ) استمر;
    إذا( NEVER(p_بنسټ->u3.pUsing==0) ) استمر;
    إذا( sqlite3_Id_List_شاخص(p_بنسټ->u3.pUsing, zName)>=0 ) أرجع 1;
  }
  أرجع 0;
}


/*
** This routine is a Walker callback for "expanding" a SELECT statement.
** "Expanding" means to do the following:
**
**    (1)  Make sure VDBE cursor numbers have been assigned to every
**         element of the FROM clause.
**
**    (2)  Fill in the pTabList->a[].pTab fields in the SrcList that
**         defines FROM clause.  When views appear in the FROM clause,
**         fill pTabList->a[].pSelect with a copy of the SELECT statement
**         that implements the view.  A copy is made of the view's SELECT
**         statement so that we can freely modify or delete that statement
**         without worrying about messing up the persistent representation
**         of the view.
**
**    (3)  Add terms to the WHERE clause to accommodate the NATURAL keyword
**         on joins and the ON and USING clause of joins.
**
**    (4)  Scan the list of columns in the result set (pEList) looking
**         for instances of the "*" operator or the TABLE.* operator.
**         If found, expand each "*" to be every column in every table
**         and TABLE.* to be every column in TABLE.
**
*/
ساكن صحيح selectExpander(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  صحيح i, j, k, rc;
  SrcList *pTabList;
  بیان_List *pEList;
  Src_توکی *pFrom;
  sqlite3 *db = pParse->db;
  بیان *pE, *p_ښی, *p_بیان;
  u16 selFlags = p->selFlags;
  u32 elistFlags = 0;

  p->selFlags |= SF_Expanded;
  إذا( db->mallocFailed  ){
    أرجع WRC_Abort;
  }
  أكد( p->pSrc!=0 );
  إذا( (selFlags & SF_Expanded)!=0 ){
    أرجع WRC_Prune;
  }
  إذا( pWalker->e_کوډ ){
    /* Renumber selId because it has been copied from a view */
    p->selId = ++pParse->nSelect;
  }
  pTabList = p->pSrc;
  pEList = p->pEList;
  إذا( pParse->pWith && (p->selFlags & SF_View) ){
    إذا( p->pWith==0 ){
      p->pWith = (With*)sqlite3DbMallocZero(db, SZ_WITH(1) );
      إذا( p->pWith==0 ){
        أرجع WRC_Abort;
      }
    }
    p->pWith->bView = 1;
  }
  sqlite3_With_ټېل(pParse, p->pWith, 0);

  /* Make sure cursor numbers have been assigned to all entries in
  ** the FROM clause of the SELECT statement.
  */
  sqlite3_Src_List_ټاکل_Cursors(pParse, pTabList);

  /* Look up every table named in the FROM clause of the select.  If
  ** an entry of the FROM clause is a subquery instead of a table or view,
  ** then create a transient table structure to describe the subquery.
  */
  لكل(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    جدول *pTab;
    أكد( pFrom->fg.isRecursive==0 || pFrom->pSTab!=0 );
    إذا( pFrom->pSTab ) استمر;
    أكد( pFrom->fg.isRecursive==0 );
    إذا( pFrom->zName==0 ){
#که_نه_تعریف_شوی SQLITE_OMIT_SUBQUERY
      Select *pSel;
      أكد( pFrom->fg.isSubquery && pFrom->u4.pSubq!=0 );
      pSel = pFrom->u4.pSubq->pSelect;
      /* A sub-query in the FROM clause of a SELECT */
      أكد( pSel!=0 );
      أكد( pFrom->pSTab==0 );
      إذا( sqlite3WalkSelect(pWalker, pSel) ) أرجع WRC_Abort;
      إذا( sqlite3ExpandSubquery(pParse, pFrom) ) أرجع WRC_Abort;
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_CTE
    }وإلا إذا( (rc = resolve_From_غړی_منزل_Cte(pParse, pWalker, pFrom))!=0 ){
      إذا( rc>1 ) أرجع WRC_Abort;
      pTab = pFrom->pSTab;
      أكد( pTab!=0 );
#د_که_پای
    }وإلا{
      /* An ordinary table or view name in the FROM clause */
      أكد( pFrom->pSTab==0 );
      pFrom->pSTab = pTab = sqlite3_Locate_جدول_توکی(pParse, 0, pFrom);
      إذا( pTab==0 ) أرجع WRC_Abort;
      إذا( pTab->nTabRef>=0xffff ){
        sqlite3_Error_پیغام(pParse, "too many references to \"%s\": max 65535",
           pTab->zName);
        pFrom->pSTab = 0;
        أرجع WRC_Abort;
      }
      pTab->nTabRef++;
      إذا( !IsVirtual(pTab) && cannotBeFunction(pParse, pFrom) ){
        أرجع WRC_Abort;
      }
#إذا !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
      إذا( !Is_Ordinary_جدول(pTab) ){
        i16 n_ستون;
        u8 e_کوډ_Orig = pWalker->e_کوډ;
        إذا( sqlite3_View_Get_ستنه_نومونه(pParse, pTab) ) أرجع WRC_Abort;
        أكد( pFrom->fg.isSubquery==0 );
        إذا( IsView(pTab) ){
          إذا( (db->flags & SQLITE_EnableView)==0
           && pTab->pSchema!=db->aDb[1].pSchema
          ){
            sqlite3_Error_پیغام(pParse, "access to view \"%s\" prohibited",
              pTab->zName);
          }
          sqlite3_Src_توکی_Attach_Subquery(pParse, pFrom, pTab->u.view.pSelect, 1);
        }
#که_نه_تعریف_شوی SQLITE_OMIT_VIRTUALTABLE
        وإلا إذا( ALWAYS(IsVirtual(pTab))
         && (pFrom->fg.fromDDL || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
         && ALWAYS(pTab->u.vtab.p!=0)
         && pTab->u.vtab.p->eVtabRisk > ((db->flags & SQLITE_TrustedSchema)!=0)
        ){
          sqlite3_Error_پیغام(pParse, "unsafe use of virtual table \"%s\"",
                                  pTab->zName);
        }
        أكد( SQLITE_VTABRISK_نورمال==1 && SQLITE_VTABRISK_لوړ==2 );
#د_که_پای
        n_ستون = pTab->n_ستون;
        pTab->n_ستون = -1;
        pWalker->e_کوډ = 1;  /* Turn on Select.selId renumbering */
        إذا( pFrom->fg.isSubquery ){
          bft hasCompound = pParse->hasCompound;
          pParse->hasCompound = 1;
          selectExpanderWalk(pWalker, pFrom->u4.pSubq->pSelect);
          pParse->hasCompound = hasCompound;
        }
        pWalker->e_کوډ = e_کوډ_Orig;
        pTab->n_ستون = n_ستون;
      }
#د_که_پای
    }

    /* Locate the index named by the INDEXED BY clause, if any. */
    إذا( pFrom->fg.is_شاخصي_By && sqlite3_شاخصي_By_Lookup(pParse, pFrom) ){
      أرجع WRC_Abort;
    }
  }

  /* Process NATURAL keywords, and ON and USING clauses of joins.
  */
  أكد( db->mallocFailed==0 || pParse->nErr!=0 );
  إذا( pParse->nErr || sqlite3ProcessJoin(pParse, p) ){
    أرجع WRC_Abort;
  }

  /* For every "*" that occurs in the column list, insert the names of
  ** all columns in all tables.  And for every TABLE.* insert the names
  ** of all columns in TABLE.  The parser inserted a special expression
  ** with the TK_ASTERISK operator for each "*" that it found in the column
  ** list.  The following code just has to locate the TK_ASTERISK
  ** expressions and expand each one to the list of all columns in
  ** all tables.
  **
  ** The first loop just checks to see if there are any "*" operators
  ** that need expanding.
  */
  لكل(k=0; k<pEList->n_بیان; k++){
    pE = pEList->a[k].p_بیان;
    إذا( pE->op==TK_ASTERISK ) اكسر;
    أكد( pE->op!=TK_DOT || pE->p_ښی!=0 );
    أكد( pE->op!=TK_DOT || (pE->p_کیڼ!=0 && pE->p_کیڼ->op==TK_ID) );
    إذا( pE->op==TK_DOT && pE->p_ښی->op==TK_ASTERISK ) اكسر;
    elistFlags |= pE->flags;
  }
  إذا( k<pEList->n_بیان ){
    /*
    ** If we get here it means the result set contains one or more "*"
    ** operators that need to be expanded.  Loop through each expression
    ** in the result set and expand them one by one.
    */
    بنية بیان_List_توکی *a = pEList->a;
    بیان_List *pNew = 0;
    صحيح flags = pParse->db->flags;
    صحيح long_نومونه = (flags & SQLITE_ډک_ستون_نومونه)!=0
                      && (flags & SQLITE_Short_ستون_نومونه)==0;

    لكل(k=0; k<pEList->n_بیان; k++){
      pE = a[k].p_بیان;
      elistFlags |= pE->flags;
      p_ښی = pE->p_ښی;
      أكد( pE->op!=TK_DOT || p_ښی!=0 );
      إذا( pE->op!=TK_ASTERISK
       && (pE->op!=TK_DOT || p_ښی->op!=TK_ASTERISK)
      ){
        /* This particular expression does not need to be expanded.
        */
        pNew = sqlite3_بیان_List_Append(pParse, pNew, a[k].p_بیان);
        إذا( pNew ){
          pNew->a[pNew->n_بیان-1].zEName = a[k].zEName;
          pNew->a[pNew->n_بیان-1].fg.eEName = a[k].fg.eEName;
          a[k].zEName = 0;
        }
        a[k].p_بیان = 0;
      }وإلا{
        /* This expression is a "*" or a "TABLE.*" and needs to be
        ** expanded. */
        صحيح جدول_کتل_شوی = 0;      /* Set to 1 when TABLE matches */
        حرف *zTName = 0;       /* text of name of TABLE */
        صحيح iErrOfst;
        إذا( pE->op==TK_DOT ){
          أكد( (selFlags & SF_NestedFrom)==0 );
          أكد( pE->p_کیڼ!=0 );
          أكد( !بیان_Has_Property(pE->p_کیڼ, EP_Int_ارزښت) );
          zTName = pE->p_کیڼ->u.zToken;
          أكد( بیان_Use_WOfst(pE->p_کیڼ) );
          iErrOfst = pE->p_ښی->w.iOfst;
        }وإلا{
          أكد( pE->p_کیڼ==0 );
          أكد( بیان_Use_WOfst(pE) );
          iErrOfst = pE->w.iOfst;
        }
        لكل(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
          صحيح n_جمع_کړه;                    /* Number of cols including rowid */
          جدول *pTab = pFrom->pSTab;  /* Table for this data source */
          بیان_List *pNestedFrom;       /* Result-set of a nested FROM clause */
          حرف *zTabName;              /* AS name for this data source */
          ثابت حرف *zSchemaName = 0; /* Schema name for this data source */
          صحيح iDb;                     /* Schema index for this data src */
          IdList *pUsing;              /* USING clause for pFrom[1] */

          إذا( (zTabName = pFrom->zAlias)==0 ){
            zTabName = pTab->zName;
          }
          إذا( db->mallocFailed ) اكسر;
          أكد( (صحيح)pFrom->fg.isNestedFrom == IsNestedFrom(pFrom) );
          إذا( pFrom->fg.isNestedFrom ){
            أكد( pFrom->fg.isSubquery && pFrom->u4.pSubq );
            أكد( pFrom->u4.pSubq->pSelect!=0 );
            pNestedFrom = pFrom->u4.pSubq->pSelect->pEList;
            أكد( pNestedFrom!=0 );
            أكد( pNestedFrom->n_بیان==pTab->n_ستون );
            أكد( VisibleRowid(pTab)==0 || ViewCanHaveRowid );
          }وإلا{
            إذا( zTName && sqlite3StrICmp(zTName, zTabName)!=0 ){
              استمر;
            }
            pNestedFrom = 0;
            iDb = sqlite3_Schema_منزل_شاخص(db, pTab->pSchema);
            zSchemaName = iDb>=0 ? db->aDb[iDb].zDbSName : "*";
          }
          إذا( i+1<pTabList->nSrc
           && pFrom[1].fg.isUsing
           && (selFlags & SF_NestedFrom)!=0
          ){
            صحيح ii;
            pUsing = pFrom[1].u3.pUsing;
            لكل(ii=0; ii<pUsing->nId; ii++){
              ثابت حرف *zUName = pUsing->a[ii].zName;
              p_ښی = sqlite3_بیان(db, TK_ID, zUName);
              sqlite3_بیان_سېټ_Error_دننه_واټن(p_ښی, iErrOfst);
              pNew = sqlite3_بیان_List_Append(pParse, pNew, p_ښی);
              إذا( pNew ){
                بنية بیان_List_توکی *pX = &pNew->a[pNew->n_بیان-1];
                أكد( pX->zEName==0 );
                pX->zEName = sqlite3MPrintf(db,"..%s", zUName);
                pX->fg.eEName = ENAME_TAB;
                pX->fg.b_Using_غړی = 1;
              }
            }
          }وإلا{
            pUsing = 0;
          }

          n_جمع_کړه = pTab->n_ستون;
          إذا( VisibleRowid(pTab) && (selFlags & SF_NestedFrom)!=0 ) n_جمع_کړه++;
          لكل(j=0; j<n_جمع_کړه; j++){
            ثابت حرف *zName; 
            بنية بیان_List_توکی *pX; /* Newly added ExprList term */

            إذا( j==pTab->n_ستون ){
              zName = sqlite3RowidAlias(pTab);
              إذا( zName==0 ) استمر;
            }وإلا{
              zName = pTab->a_ستون[j].zCnName;

              /* If pTab is actually an SF_NestedFrom sub-select, do not
              ** expand any ENAME_ROWID columns.  */
              إذا( pNestedFrom && pNestedFrom->a[j].fg.eEName==ENAME_ROWID ){
                استمر;
              }

              إذا( zTName
               && pNestedFrom
               && sqlite3_سمون_EName(&pNestedFrom->a[j], 0, zTName, 0, 0)==0
              ){
                استمر;
              }

              /* If a column is marked as 'hidden', omit it from the expanded
              ** result-set list unless the SELECT has the SF_IncludeHidden
              ** bit set.
              */
              إذا( (p->selFlags & SF_IncludeHidden)==0
                && Is_Hidden_ستنه(&pTab->a_ستون[j])
              ){
                استمر;
              }
              إذا( (pTab->a_ستون[j].ستون_Flags & COLFLAG_NOEXPAND)!=0
               && zTName==0
               && (selFlags & (SF_NestedFrom))==0
              ){
                استمر;
              }
            }
            أكد( zName );
            جدول_کتل_شوی = 1;

            إذا( i>0 && zTName==0 && (selFlags & SF_NestedFrom)==0 ){
              إذا( pFrom->fg.isUsing
               && sqlite3_Id_List_شاخص(pFrom->u3.pUsing, zName)>=0
              ){
                /* In a join with a USING clause, omit columns in the
                ** using clause from the table on the right. */
                استمر;
              }
            }
            p_ښی = sqlite3_بیان(db, TK_ID, zName);
            إذا( (pTabList->nSrc>1
                 && (  (pFrom->fg.jointype & JT_LTORJ)==0
                     || zTName!=0
                     || (selFlags & SF_NestedFrom)!=0
                     || !دننه_Any_Using_Clause(zName,pFrom,pTabList->nSrc-i-1)
                    )
                )
             || دننه_RENAME_OBJECT
            ){
              بیان *p_کیڼ;
              p_کیڼ = sqlite3_بیان(db, TK_ID, zTabName);
              p_بیان = sqlite3PExpr(pParse, TK_DOT, p_کیڼ, p_ښی);
              إذا( دننه_RENAME_OBJECT && pE->p_کیڼ ){
                أكد( pE->op==TK_DOT && pE->p_ښی->op==TK_ASTERISK );
                sqlite3RenameTokenRemap(pParse, p_کیڼ, pE->p_کیڼ);
              }
              إذا( zSchemaName ){
                p_کیڼ = sqlite3_بیان(db, TK_ID, zSchemaName);
                p_بیان = sqlite3PExpr(pParse, TK_DOT, p_کیڼ, p_بیان);
              }
            }وإلا{
              p_بیان = p_ښی;
            }
            sqlite3_بیان_سېټ_Error_دننه_واټن(p_بیان, iErrOfst);
            pNew = sqlite3_بیان_List_Append(pParse, pNew, p_بیان);
            إذا( pNew==0 ){
              اكسر;  /* OOM */
            }
            pX = &pNew->a[pNew->n_بیان-1];
            أكد( pX->zEName==0 );
            إذا( (selFlags & SF_NestedFrom)!=0 && !دننه_RENAME_OBJECT ){
              إذا( pNestedFrom && (!ViewCanHaveRowid || j<pNestedFrom->n_بیان) ){
                أكد( j<pNestedFrom->n_بیان );
                pX->zEName = sqlite3DbStrDup(db, pNestedFrom->a[j].zEName);
                testcase( pX->zEName==0 );
              }وإلا{
                pX->zEName = sqlite3MPrintf(db, "%s.%s.%s",
                                           zSchemaName, zTabName, zName);
                testcase( pX->zEName==0 );
              }
              pX->fg.eEName = (j==pTab->n_ستون ? ENAME_ROWID : ENAME_TAB);
              إذا( (pFrom->fg.isUsing
                   && sqlite3_Id_List_شاخص(pFrom->u3.pUsing, zName)>=0)
               || (pUsing && sqlite3_Id_List_شاخص(pUsing, zName)>=0)
               || (j<pTab->n_ستون && (pTab->a_ستون[j].ستون_Flags & COLFLAG_NOEXPAND))
              ){
                pX->fg.bNoExpand = 1;
              }
            }وإلا إذا( long_نومونه ){
              pX->zEName = sqlite3MPrintf(db, "%s.%s", zTabName, zName);
              pX->fg.eEName = ENAME_NAME;
            }وإلا{
              pX->zEName = sqlite3DbStrDup(db, zName);
              pX->fg.eEName = ENAME_NAME;
            }
          }
        }
        إذا( !جدول_کتل_شوی ){
          إذا( zTName ){
            sqlite3_Error_پیغام(pParse, "no such table: %s", zTName);
          }وإلا{
            sqlite3_Error_پیغام(pParse, "no tables specified");
          }
        }
      }
    }
    sqlite3_بیان_List_Delete(db, pEList);
    p->pEList = pNew;
  }
  إذا( p->pEList ){
    إذا( p->pEList->n_بیان>db->a_حد[SQLITE_حد_ستنه] ){
      sqlite3_Error_پیغام(pParse, "too many columns in result set");
      أرجع WRC_Abort;
    }
    إذا( (elistFlags & (EP_HasFunc|EP_Subquery))!=0 ){
      p->selFlags |= SF_Complex_پایله;
    }
  }
#إذا TREETRACE_ENABLED
  إذا( sqlite3_ونه_Trace & 0x8 ){
    TREETRACE(0x8,pParse,p,("After result-set wildcard expansion:\n"));
    sqlite3_ونه_View_Select(0, p, 0);
  }
#د_که_پای
  أرجع WRC_Continue;
}

#إذا SQLITE_DEBUG
/*
** Always assert.  This xSelectCallback2 implementation proves that the
** xSelectCallback2 is never invoked.
*/
فراغ sqlite3SelectWalkAssert2(Walker *Not_کارول_شوی, Select *NotUsed2){
  ناکارول_شوی_PARAMETER2(Not_کارول_شوی, NotUsed2);
  أكد( 0 );
}
#د_که_پای

/*
** The first argument is a Walker object created by sqlite3SelectExpand().
** This function uses the walker to expand the SELECT statement passed
** as the second argument. If necessary, it also converts any compound
** SELECT statements that have COLLATE clauses on ORDER BY expressions
** to sub-queries. Return the return value of sqlite3WalkSelect().
*/
ساكن صحيح selectExpanderWalk(Walker *pWalker, Select *pSelect){
  أكد( pWalker->x_بیان_Callback==sqlite3_بیان_Walk_Noop );
  أكد( pWalker->pParse!=0 );
  إذا( OK_IF_ALWAYS_TRUE(pWalker->pParse->hasCompound) ){
    pWalker->xSelectCallback = convert_Compound_Select_منزل_Subquery;
    pWalker->xSelectCallback2 = 0;
    sqlite3WalkSelect(pWalker, pSelect);
  }
  pWalker->xSelectCallback = selectExpander;
  pWalker->xSelectCallback2 = sqlite3_Select_راوباسه_With;
  أرجع sqlite3WalkSelect(pWalker, pSelect);
}

/*
** This routine "expands" a SELECT statement and all of its subqueries.
** For additional information on what it means to "expand" a SELECT
** statement, see the comment on the selectExpand worker callback above.
**
** Expanding a SELECT statement is the first step in processing a
** SELECT statement.  The SELECT statement must be expanded before
** name resolution is performed.
**
** If anything goes wrong, an error message is written into pParse.
** The calling function can detect the problem by looking at pParse->nErr
** and/or pParse->db->mallocFailed.
*/
ساكن فراغ sqlite3SelectExpand(Parse *pParse, Select *pSelect){
  Walker w;
  w.x_بیان_Callback = sqlite3_بیان_Walk_Noop;
  w.pParse = pParse;
  w.e_کوډ = 0;
  selectExpanderWalk(&w, pSelect);
}


#که_نه_تعریف_شوی SQLITE_OMIT_SUBQUERY
/*
** This is a Walker.xSelectCallback callback for the sqlite3SelectTypeInfo()
** interface.
**
** For each FROM-clause subquery, add Column.zType, Column.zColl, and
** Column.affinity information to the Table structure that represents
** the result set of that subquery.
**
** The Table structure that represents the result set was constructed
** by selectExpander() but the type and collation and affinity information
** was omitted at that point because identifiers had not yet been resolved.
** This routine is called after identifier resolution.
*/
ساكن فراغ select_جمع_کړه_Subquery_Type_Info(Walker *pWalker, Select *p){
  Parse *pParse;
  صحيح i;
  SrcList *pTabList;
  Src_توکی *pFrom;

  إذا( p->selFlags & SF_HasTypeInfo ) أرجع;
  p->selFlags |= SF_HasTypeInfo;
  pParse = pWalker->pParse;
  أكد( (p->selFlags & SF_Resolved) );
  pTabList = p->pSrc;
  لكل(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    جدول *pTab = pFrom->pSTab;
    أكد( pTab!=0 );
    إذا( (pTab->tabFlags & TF_Ephemeral)!=0 && pFrom->fg.isSubquery ){
      /* A sub-query in the FROM clause of a SELECT */
      Select *pSel = pFrom->u4.pSubq->pSelect;
      sqlite3_Subquery_ستنه_Types(pParse, pTab, pSel, SQLITE_AFF_NONE);
    }
  }
}
#د_که_پای


/*
** This routine adds datatype and collating sequence information to
** the Table structures of all FROM-clause subqueries in a
** SELECT statement.
**
** Use this routine after name resolution.
*/
ساكن فراغ sqlite3_Select_جمع_کړه_Type_Info(Parse *pParse, Select *pSelect){
#که_نه_تعریف_شوی SQLITE_OMIT_SUBQUERY
  Walker w;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.xSelectCallback2 = select_جمع_کړه_Subquery_Type_Info;
  w.x_بیان_Callback = sqlite3_بیان_Walk_Noop;
  w.pParse = pParse;
  sqlite3WalkSelect(&w, pSelect);
#د_که_پای
}


/*
** This routine sets up a SELECT statement for processing.  The
** following is accomplished:
**
**     *  VDBE Cursor numbers are assigned to all FROM-clause terms.
**     *  Ephemeral Table objects are created for all FROM-clause subqueries.
**     *  ON and USING clauses are shifted into WHERE statements
**     *  Wildcards "*" and "TABLE.*" in result sets are expanded.
**     *  Identifiers in expression are matched to tables.
**
** This routine acts recursively on all subqueries within the SELECT.
*/
فراغ sqlite3SelectPrep(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for container */
){
  أكد( p!=0 || pParse->db->mallocFailed );
  أكد( pParse->db->pParse==pParse );
  إذا( pParse->db->mallocFailed ) أرجع;
  إذا( p->selFlags & SF_HasTypeInfo ) أرجع;
  sqlite3SelectExpand(pParse, p);
  إذا( pParse->nErr ) أرجع;
  sqlite3_Resolve_Select_نومونه(pParse, p, pOuterNC);
  إذا( pParse->nErr ) أرجع;
  sqlite3_Select_جمع_کړه_Type_Info(pParse, p);
}

#إذا TREETRACE_ENABLED
/*
** Display all information about an AggInfo object
*/
ساكن فراغ printAggInfo(AggInfo *pAggInfo){
  صحيح ii;
  sqlite3DebugPrintf("AggInfo %d/%p:\n",
     pAggInfo->selId, pAggInfo);
  لكل(ii=0; ii<pAggInfo->n_ستنه; ii++){
    بنية Agg_Info_ستون *p_ستون = &pAggInfo->a_ستون[ii];
    sqlite3DebugPrintf(
       "agg-column[%d] pTab=%s iTable=%d iColumn=%d iMem=%d"
       " iSorterColumn=%d %s\n",
       ii, p_ستون->pTab ? p_ستون->pTab->zName : "NULL",
       p_ستون->i_جدول, p_ستون->i_ستنه, pAggInfo->i_لومړی_Reg+ii,
       p_ستون->i_Sorter_ستنه,
       ii>=pAggInfo->n_راټولونکی ? "" : " Accumulator");
    sqlite3_ونه_View_بیان(0, pAggInfo->a_ستون[ii].pCExpr, 0);
  }
  لكل(ii=0; ii<pAggInfo->nFunc; ii++){
    sqlite3DebugPrintf("agg-func[%d]: iMem=%d\n",
        ii, pAggInfo->i_لومړی_Reg+pAggInfo->n_ستنه+ii);
    sqlite3_ونه_View_بیان(0, pAggInfo->aFunc[ii].pFExpr, 0);
  }
}
#د_که_پای /* TREETRACE_ENABLED */

/*
** Analyze the arguments to aggregate functions.  Create new pAggInfo->aCol[]
** entries for columns that are arguments to aggregate functions but which
** are not otherwise used.
**
** The aCol[] entries in AggInfo prior to nAccumulator are columns that
** are referenced outside of aggregate functions.  These might be columns
** that are part of the GROUP by clause, for example.  Other database engines
** would throw an error if there is a column reference that is not in the
** GROUP BY clause and that is not part of an aggregate function argument.
** But SQLite allows this.
**
** The aCol[] entries beginning with the aCol[nAccumulator] and following
** are column references that are used exclusively as arguments to
** aggregate functions.  This routine is responsible for computing
** (or recomputing) those aCol[] entries.
*/
ساكن فراغ analyze_Agg_Func_دلیلونه(
  AggInfo *pAggInfo,
  NameContext *pNC
){
  صحيح i;
  أكد( pAggInfo!=0 );
  أكد( pAggInfo->i_لومړی_Reg==0 );
  pNC->ncFlags |= NC_دننه_Agg_Func;
  لكل(i=0; i<pAggInfo->nFunc; i++){
    بیان *p_بیان = pAggInfo->aFunc[i].pFExpr;
    أكد( p_بیان->op==TK_FUNCTION || p_بیان->op==TK_AGG_FUNCTION );
    أكد( بیان_Use_XList(p_بیان) );
    sqlite3_بیان_Analyze_Agg_List(pNC, p_بیان->x.pList);
    إذا( p_بیان->p_کیڼ ){
      أكد( p_بیان->p_کیڼ->op==TK_ترتیب );
      أكد( بیان_Use_XList(p_بیان->p_کیڼ) );
      sqlite3_بیان_Analyze_Agg_List(pNC, p_بیان->p_کیڼ->x.pList);
    }
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
    أكد( !Is_کړکۍ_Func(p_بیان) );
    إذا( بیان_Has_Property(p_بیان, EP_WinFunc) ){
      sqlite3_بیان_Analyze_Aggregates(pNC, p_بیان->y.pWin->pFilter);
    }
#د_که_پای
  }
  pNC->ncFlags &= ~NC_دننه_Agg_Func;
}

/*
** An index on expressions is being used in the inner loop of an
** aggregate query with a GROUP BY clause.  This routine attempts
** to adjust the AggInfo object to take advantage of index and to
** perhaps use the index as a covering index.
**
*/
ساكن فراغ optimize_Aggregate_Use_د_شاخصي_بیان(
  Parse *pParse,          /* Parsing context */
  Select *pSelect,        /* The SELECT statement being processed */
  AggInfo *pAggInfo,      /* The aggregate info */
  NameContext *pNC        /* Name context used to resolve agg-func args */
){
  أكد( pAggInfo->i_لومړی_Reg==0 );
  أكد( pSelect!=0 );
  أكد( pSelect->p_ډله_By!=0 );
  pAggInfo->n_ستنه = pAggInfo->n_راټولونکی;
  إذا( ALWAYS(pAggInfo->n_ترتیبونه_ستنه>0) ){
    صحيح mx = pSelect->p_ډله_By->n_بیان - 1;
    صحيح j, k;
    لكل(j=0; j<pAggInfo->n_ستنه; j++){
      k = pAggInfo->a_ستون[j].i_Sorter_ستنه;
      إذا( k>mx ) mx = k;
    }
    pAggInfo->n_ترتیبونه_ستنه = mx+1;
  }
  analyze_Agg_Func_دلیلونه(pAggInfo, pNC);
#إذا TREETRACE_ENABLED
  إذا( sqlite3_ونه_Trace & 0x20 ){
    شاخصي_بیان *pIEpr;
    TREETRACE(0x20, pParse, pSelect,
        ("AggInfo (possibly) adjusted for Indexed Exprs\n"));
    sqlite3_ونه_View_Select(0, pSelect, 0);
    لكل(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
      چاپ_کړه("data-cursor=%d index={%d,%d}\n",
          pIEpr->i_معلومات_Cur, pIEpr->iIdxCur, pIEpr->i_Idx_ستون);
      sqlite3_ونه_View_بیان(0, pIEpr->p_بیان, 0);
    }
    printAggInfo(pAggInfo);
  }
#وإلا
  ناکارول_شوی_PARAMETER(pSelect);
  ناکارول_شوی_PARAMETER(pParse);
#د_که_پای
}

/*
** Walker callback for aggregateConvertIndexedExprRefToColumn().
*/
ساكن صحيح aggregate_Idx_Epr_Ref_منزل_ستون_Callback(Walker *pWalker, بیان *p_بیان){
  AggInfo *pAggInfo;
  بنية Agg_Info_ستون *p_ستون;
  ناکارول_شوی_PARAMETER(pWalker);
  إذا( p_بیان->pAggInfo==0 ) أرجع WRC_Continue;
  إذا( p_بیان->op==TK_AGG_ستنه ) أرجع WRC_Continue;
  إذا( p_بیان->op==TK_AGG_FUNCTION ) أرجع WRC_Continue;
  إذا( p_بیان->op==TK_IF_NULL_لیکه ) أرجع WRC_Continue;
  pAggInfo = p_بیان->pAggInfo;
  إذا( NEVER(p_بیان->iAgg>=pAggInfo->n_ستنه) ) أرجع WRC_Continue;
  أكد( p_بیان->iAgg>=0 );
  p_ستون = &pAggInfo->a_ستون[p_بیان->iAgg];
  p_بیان->op = TK_AGG_ستنه;
  p_بیان->i_جدول = p_ستون->i_جدول;
  p_بیان->i_ستنه = p_ستون->i_ستنه;
  بیان_Clear_Property(p_بیان, EP_Skip|EP_Collate|EP_Unlikely);
  أرجع WRC_Prune;
}

/*
** Convert every pAggInfo->aFunc[].pExpr such that any node within
** those expressions that has pAppInfo set is changed into a TK_AGG_COLUMN
** opcode.
*/
ساكن فراغ aggregate_Convert_شاخصي_بیان_Ref_منزل_ستنه(AggInfo *pAggInfo){
  صحيح i;
  Walker w;
  memset(&w, 0, حجم(w));
  w.x_بیان_Callback = aggregate_Idx_Epr_Ref_منزل_ستون_Callback;
  لكل(i=0; i<pAggInfo->nFunc; i++){
    sqlite3_Walk_بیان(&w, pAggInfo->aFunc[i].pFExpr);
  }
}


/*
** Allocate a block of registers so that there is one register for each
** pAggInfo->aCol[] and pAggInfo->aFunc[] entry in pAggInfo.  The first
** register in this block is stored in pAggInfo->iFirstReg.
**
** This routine may only be called once for each AggInfo object.  Prior
** to calling this routine:
**
**     *  The aCol[] and aFunc[] arrays may be modified
**     *  The AggInfoColumnReg() and AggInfoFuncReg() macros may not be used
**
** After calling this routine:
**
**     *  The aCol[] and aFunc[] arrays are fixed
**     *  The AggInfoColumnReg() and AggInfoFuncReg() macros may be used
**
*/
ساكن فراغ ټاکل_Aggregate_Registers(Parse *pParse, AggInfo *pAggInfo){
  أكد( pAggInfo!=0 );
  أكد( pAggInfo->i_لومړی_Reg==0 );
  pAggInfo->i_لومړی_Reg = pParse->nMem + 1;
  pParse->nMem += pAggInfo->n_ستنه + pAggInfo->nFunc;
}

/*
** Reset the aggregate accumulator.
**
** The aggregate accumulator is a set of memory cells that hold
** intermediate results while calculating an aggregate.  This
** routine generates code that stores NULLs in all of those memory
** cells.
*/
ساكن فراغ reset_راټولونکی(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  صحيح i;
  بنية AggInfo_func *pFunc;
  صحيح nReg = pAggInfo->nFunc + pAggInfo->n_ستنه;
  أكد( pAggInfo->i_لومړی_Reg>0 );
  أكد( pParse->db->pParse==pParse );
  أكد( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  إذا( nReg==0 ) أرجع;
  إذا( pParse->nErr ) أرجع;
  sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Null, 0, pAggInfo->i_لومړی_Reg,
                    pAggInfo->i_لومړی_Reg+nReg-1);
  لكل(pFunc=pAggInfo->aFunc, i=0; i<pAggInfo->nFunc; i++, pFunc++){
    إذا( pFunc->iDistinct>=0 ){
      بیان *pE = pFunc->pFExpr;
      أكد( بیان_Use_XList(pE) );
      إذا( pE->x.pList==0 || pE->x.pList->n_بیان!=1 ){
        sqlite3_Error_پیغام(pParse, "DISTINCT aggregates must have exactly one "
           "argument");
        pFunc->iDistinct = -1;
      }وإلا{
        کلی_Info *p_کلی_Info = sqlite3_کلی_Info_From_بیان_List(pParse, pE->x.pList,0,0);
        pFunc->iDistAddr = sqlite3_Vdbe_جمع_کړه_Op4(v, OP_OpenEphemeral,
            pFunc->iDistinct, 0, 0, (حرف*)p_کلی_Info, P4_KEYINFO);
        ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(DISTINCT)",
                          pFunc->pFunc->zName));
      }
    }
    إذا( pFunc->iOBTab>=0 ){
      بیان_List *pOBList;
      کلی_Info *p_کلی_Info;
      صحيح nExtra = 0;
      أكد( pFunc->pFExpr->p_کیڼ!=0 );
      أكد( pFunc->pFExpr->p_کیڼ->op==TK_ترتیب );
      أكد( بیان_Use_XList(pFunc->pFExpr->p_کیڼ) );
      أكد( pFunc->pFunc!=0 );
      pOBList = pFunc->pFExpr->p_کیڼ->x.pList;
      إذا( !pFunc->bOBUnique ){
        nExtra++;  /* One extra column for the OP_Sequence */
      }
      إذا( pFunc->bOBPayload ){
        /* extra columns for the function arguments */
        أكد( بیان_Use_XList(pFunc->pFExpr) );
        أكد( pFunc->pFExpr->x.pList!=0 );
        nExtra += pFunc->pFExpr->x.pList->n_بیان;
      }
      إذا( pFunc->bUseSubtype ){
        nExtra += pFunc->pFExpr->x.pList->n_بیان;
      }
      p_کلی_Info = sqlite3_کلی_Info_From_بیان_List(pParse, pOBList, 0, nExtra);
      إذا( !pFunc->bOBUnique && pParse->nErr==0 ){
        p_کلی_Info->n_کلی_ډګر++;
      }
      sqlite3_Vdbe_جمع_کړه_Op4(v, OP_OpenEphemeral,
            pFunc->iOBTab, pOBList->n_بیان+nExtra, 0,
            (حرف*)p_کلی_Info, P4_KEYINFO);
      ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(ORDER BY)",
                          pFunc->pFunc->zName));
    }
  }
}

/*
** Invoke the OP_AggFinalize opcode for every aggregate function
** in the AggInfo structure.
*/
ساكن فراغ finalizeAggFunctions(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  صحيح i;
  بنية AggInfo_func *pF;
  لكل(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    بیان_List *pList;
    أكد( بیان_Use_XList(pF->pFExpr) );
    إذا( pParse->nErr ) أرجع;
    pList = pF->pFExpr->x.pList;
    إذا( pF->iOBTab>=0 ){
      /* For an ORDER BY aggregate, calls to OP_AggStep were deferred.  Inputs
      ** were stored in emphermal table pF->iOBTab.  Here, we extract those
      ** inputs (in ORDER BY order) and make all calls to OP_AggStep
      ** before doing the OP_AggFinal call. */
      صحيح i_سر_2;        /* Start of loop for extracting columns */
      صحيح nArg;        /* Number of columns to extract */
      صحيح n_کلی;        /* Key columns to be skipped */
      صحيح regAgg;      /* Extract into this array */
      صحيح j;           /* Loop counter */
     
      أكد( pF->pFunc!=0 );
      nArg = pList->n_بیان;
      regAgg = sqlite3_Get_لنډمهاله_Range(pParse, nArg);

      إذا( pF->bOBPayload==0 ){
        n_کلی = 0;
      }وإلا{
        أكد( pF->pFExpr->p_کیڼ!=0 );
        أكد( بیان_Use_XList(pF->pFExpr->p_کیڼ) );
        أكد( pF->pFExpr->p_کیڼ->x.pList!=0 );
        n_کلی = pF->pFExpr->p_کیڼ->x.pList->n_بیان;
        إذا( ALWAYS(!pF->bOBUnique) ) n_کلی++;
      }
      i_سر_2 = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Rewind, pF->iOBTab); VdbeCoverage(v);
      لكل(j=nArg-1; j>=0; j--){
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, pF->iOBTab, n_کلی+j, regAgg+j);
      }
      إذا( pF->bUseSubtype ){
        صحيح regSubtype = sqlite3_Get_لنډمهاله_Reg(pParse);
        صحيح i_بنسټ_ستون = n_کلی + nArg + (pF->bOBPayload==0 && pF->bOBUnique==0);
        لكل(j=nArg-1; j>=0; j--){
          sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, pF->iOBTab, i_بنسټ_ستون+j, regSubtype);
          sqlite3_Vdbe_جمع_کړه_Op2(v, OP_سېټ_Subtype, regSubtype, regAgg+j);
        }
        sqlite3_Release_لنډمهاله_Reg(pParse, regSubtype);
      }
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Agg_ګام, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_بل, pF->iOBTab, i_سر_2+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_دلته(v, i_سر_2);
      sqlite3_Release_لنډمهاله_Range(pParse, regAgg, nArg);
    }
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_AggFinal, AggInfoFuncReg(pAggInfo,i),
                      pList ? pList->n_بیان : 0);
    sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
  }
}

/*
** Generate code that will update the accumulator memory cells for an
** aggregate based on the current cursor position.
**
** If regAcc is non-zero and there are no min() or max() aggregates
** in pAggInfo, then only populate the pAggInfo->nAccumulator accumulator
** registers if register regAcc contains 0. The caller will take care
** of setting and clearing regAcc.
**
** For an ORDER BY aggregate, the actual accumulator memory cell update
** is deferred until after all input rows have been received, so that they
** can be run in the requested order.  In that case, instead of invoking
** OP_AggStep to update the accumulator, just add the arguments that would
** have been passed into OP_AggStep into the sorting ephemeral table
** (along with the appropriate sort key).
*/
ساكن فراغ update_راټولونکی(
  Parse *pParse,
  صحيح regAcc,
  AggInfo *pAggInfo,
  صحيح eDistinctType
){
  Vdbe *v = pParse->pVdbe;
  صحيح i;
  صحيح reg_موندنه = 0;
  صحيح addr_موندنه_Test = 0;
  بنية AggInfo_func *pF;
  بنية Agg_Info_ستون *pC;

  أكد( pAggInfo->i_لومړی_Reg>0 );
  إذا( pParse->nErr ) أرجع;
  pAggInfo->مستقیم_ډېرپېښ = 1;
  لكل(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    صحيح nArg;
    صحيح addr_بل = 0;
    صحيح regAgg;
    صحيح regAggSz = 0;
    صحيح regDistinct = 0;
    بیان_List *pList;
    أكد( بیان_Use_XList(pF->pFExpr) );
    أكد( !Is_کړکۍ_Func(pF->pFExpr) );
    أكد( pF->pFunc!=0 );
    pList = pF->pFExpr->x.pList;
    إذا( بیان_Has_Property(pF->pFExpr, EP_WinFunc) ){
      بیان *pFilter = pF->pFExpr->y.pWin->pFilter;
      إذا( pAggInfo->n_راټولونکی
       && (pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)
       && regAcc
      ){
        /* If regAcc==0, there there exists some min() or max() function
        ** without a FILTER clause that will ensure the magnet registers
        ** are populated. */
        إذا( reg_موندنه==0 ) reg_موندنه = ++pParse->nMem;
        /* If this is the first row of the group (regAcc contains 0), clear the
        ** "magnet" register regHit so that the accumulator registers
        ** are populated if the FILTER clause jumps over the the
        ** invocation of min() or max() altogether. Or, if this is not
        ** the first row (regAcc contains 1), set the magnet register so that
        ** the accumulators are not populated unless the min()/max() is invoked
        ** and indicates that they should be.  */
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Copy, regAcc, reg_موندنه);
      }
      addr_بل = sqlite3VdbeMakeLabel(pParse);
      sqlite3_بیان_If_False(pParse, pFilter, addr_بل, SQLITE_JUMPIFNULL);
    }
    إذا( pF->iOBTab>=0 ){
      /* Instead of invoking AggStep, we must push the arguments that would
      ** have been passed to AggStep onto the sorting table. */
      صحيح jj;                /* Registered used so far in building the record */
      بیان_List *pOBList;     /* The ORDER BY clause */
      أكد( pList!=0 );
      nArg = pList->n_بیان;
      أكد( nArg>0 );
      أكد( pF->pFExpr->p_کیڼ!=0 );
      أكد( pF->pFExpr->p_کیڼ->op==TK_ترتیب );
      أكد( بیان_Use_XList(pF->pFExpr->p_کیڼ) );
      pOBList = pF->pFExpr->p_کیڼ->x.pList;
      أكد( pOBList!=0 );
      أكد( pOBList->n_بیان>0 );
      regAggSz = pOBList->n_بیان;
      إذا( !pF->bOBUnique ){
        regAggSz++;   /* One register for OP_Sequence */
      }
      إذا( pF->bOBPayload ){
        regAggSz += nArg;
      }
      إذا( pF->bUseSubtype ){
        regAggSz += nArg;
      }
      regAggSz++;  /* One extra register to hold result of MakeRecord */
      regAgg = sqlite3_Get_لنډمهاله_Range(pParse, regAggSz);
      regDistinct = regAgg;
      sqlite3_بیان_کوډ_بیان_List(pParse, pOBList, regAgg, 0, SQLITE_ECEL_DUP);
      jj = pOBList->n_بیان;
      إذا( !pF->bOBUnique ){
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Sequence, pF->iOBTab, regAgg+jj);
        jj++;
      }
      إذا( pF->bOBPayload ){
        regDistinct = regAgg+jj;
        sqlite3_بیان_کوډ_بیان_List(pParse, pList, regDistinct, 0, SQLITE_ECEL_DUP);
        jj += nArg;
      }
      إذا( pF->bUseSubtype ){
        صحيح kk;
        صحيح reg_بنسټ = pF->bOBPayload ? regDistinct : regAgg;
        لكل(kk=0; kk<nArg; kk++, jj++){
          sqlite3_Vdbe_جمع_کړه_Op2(v, OP_GetSubtype, reg_بنسټ+kk, regAgg+jj);
        }
      }
    }وإلا إذا( pList ){
      nArg = pList->n_بیان;
      regAgg = sqlite3_Get_لنډمهاله_Range(pParse, nArg);
      regDistinct = regAgg;
      sqlite3_بیان_کوډ_بیان_List(pParse, pList, regAgg, 0, SQLITE_ECEL_DUP);
    }وإلا{
      nArg = 0;
      regAgg = 0;
    }
    إذا( pF->iDistinct>=0 && pList ){
      إذا( addr_بل==0 ){
        addr_بل = sqlite3VdbeMakeLabel(pParse);
      }
      pF->iDistinct = کوډ_Distinct(pParse, eDistinctType,
          pF->iDistinct, addr_بل, pList, regDistinct);
    }
    إذا( pF->iOBTab>=0 ){
      /* Insert a new record into the ORDER BY table */
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, regAgg, regAggSz-1,
                        regAgg+regAggSz-1);
      sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_Idx_ورننباسه, pF->iOBTab, regAgg+regAggSz-1,
                           regAgg, regAggSz-1);
      sqlite3_Release_لنډمهاله_Range(pParse, regAgg, regAggSz);
    }وإلا{
      /* Invoke the AggStep function */
      إذا( pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        CollSeq *pColl = 0;
        بنية بیان_List_توکی *p_توکی;
        صحيح j;
        أكد( pList!=0 );  /* pList!=0 if pF->pFunc has NEEDCOLL */
        لكل(j=0, p_توکی=pList->a; !pColl && j<nArg; j++, p_توکی++){
          pColl = sqlite3_بیان_Coll_Seq(pParse, p_توکی->p_بیان);
        }
        إذا( !pColl ){
          pColl = pParse->db->pDfltColl;
        }
        إذا( reg_موندنه==0 && pAggInfo->n_راټولونکی ) reg_موندنه = ++pParse->nMem;
        sqlite3_Vdbe_جمع_کړه_Op4(v, OP_CollSeq, reg_موندنه, 0, 0,
                         (حرف *)pColl, P4_COLLSEQ);
      }
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Agg_ګام, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3_Release_لنډمهاله_Range(pParse, regAgg, nArg);
    }
    إذا( addr_بل ){
      sqlite3VdbeResolveLabel(v, addr_بل);
    }
    إذا( pParse->nErr ) أرجع;
  }
  إذا( reg_موندنه==0 && pAggInfo->n_راټولونکی ){
    reg_موندنه = regAcc;
  }
  إذا( reg_موندنه ){
    addr_موندنه_Test = sqlite3_Vdbe_جمع_کړه_Op1(v, OP_If, reg_موندنه); VdbeCoverage(v);
  }
  لكل(i=0, pC=pAggInfo->a_ستون; i<pAggInfo->n_راټولونکی; i++, pC++){
    sqlite3_بیان_کوډ(pParse, pC->pCExpr, Agg_Info_ستنه_Reg(pAggInfo,i));
    إذا( pParse->nErr ) أرجع;
  }

  pAggInfo->مستقیم_ډېرپېښ = 0;
  إذا( addr_موندنه_Test ){
    sqlite3_Vdbe_Jump_دلته_Or_راوباسه_Inst(v, addr_موندنه_Test);
  }
}

/*
** Add a single OP_Explain instruction to the VDBE to explain a simple
** count(*) query ("SELECT count(*) FROM pTab").
*/
#که_نه_تعریف_شوی SQLITE_OMIT_EXPLAIN
ساكن فراغ explain_Simple_شمېر(
  Parse *pParse,                  /* Parse context */
  جدول *pTab,                    /* Table being queried */
  شاخص *pIdx                     /* Index used to optimize scan, or NULL */
){
  إذا( pParse->explain==2 ){
    صحيح bCover = (pIdx!=0 && (HasRowid(pTab) || !Is_Primary_کلی_شاخص(pIdx)));
    sqlite3VdbeExplain(pParse, 0, "SCAN %s%s%s",
        pTab->zName,
        bCover ? " USING COVERING INDEX " : "",
        bCover ? pIdx->zName : ""
    );
  }
}
#وإلا
# define explain_Simple_شمېر(a,b,c)
#د_که_پای

/*
** sqlite3WalkExpr() callback used by havingToWhere().
**
** If the node passed to the callback is a TK_AND node, return
** WRC_Continue to tell sqlite3WalkExpr() to iterate through child nodes.
**
** Otherwise, return WRC_Prune. In this case, also check if the
** sub-expression matches the criteria for being moved to the WHERE
** clause. If so, add it to the WHERE clause and replace the sub-expression
** within the HAVING expression with a constant "1".
*/
ساكن صحيح having_منزل_Where_بیان_Cb(Walker *pWalker, بیان *p_بیان){
  إذا( p_بیان->op!=TK_AND ){
    Select *pS = pWalker->u.pSelect;
    /* This routine is called before the HAVING clause of the current
    ** SELECT is analyzed for aggregates. So if pExpr->pAggInfo is set
    ** here, it indicates that the expression is a correlated reference to a
    ** column from an outer aggregate query, or an aggregate function that
    ** belongs to an outer query. Do not move the expression to the WHERE
    ** clause in this obscure case, as doing so may corrupt the outer Select
    ** statements AggInfo structure.  */
    إذا( sqlite3_بیان_Is_Constant_Or_ډله_By(pWalker->pParse, p_بیان, pS->p_ډله_By)
     && بیان_Always_False(p_بیان)==0
     && p_بیان->pAggInfo==0
    ){
      sqlite3 *db = pWalker->pParse->db;
      بیان *pNew = sqlite3_بیان_Int32(db, 1);
      إذا( pNew ){
        بیان *pWhere = pS->pWhere;
        بدلون(بیان, *pNew, *p_بیان);
        pNew = sqlite3_بیان_And(pWalker->pParse, pWhere, pNew);
        pS->pWhere = pNew;
        pWalker->e_کوډ = 1;
      }
    }
    أرجع WRC_Prune;
  }
  أرجع WRC_Continue;
}

/*
** Transfer eligible terms from the HAVING clause of a query, which is
** processed after grouping, to the WHERE clause, which is processed before
** grouping. For example, the query:
**
**   SELECT * FROM <tables> WHERE a=? GROUP BY b HAVING b=? AND c=?
**
** can be rewritten as:
**
**   SELECT * FROM <tables> WHERE a=? AND b=? GROUP BY b HAVING c=?
**
** A term of the HAVING expression is eligible for transfer if it consists
** entirely of constants and expressions that are also GROUP BY terms that
** use the "BINARY" collation sequence.
*/
ساكن فراغ having_منزل_Where(Parse *pParse, Select *p){
  Walker sWalker;
  memset(&sWalker, 0, حجم(sWalker));
  sWalker.pParse = pParse;
  sWalker.x_بیان_Callback = having_منزل_Where_بیان_Cb;
  sWalker.u.pSelect = p;
  sqlite3_Walk_بیان(&sWalker, p->pHaving);
#إذا TREETRACE_ENABLED
  إذا( sWalker.e_کوډ && (sqlite3_ونه_Trace & 0x100)!=0 ){
    TREETRACE(0x100,pParse,p,("Move HAVING terms into WHERE:\n"));
    sqlite3_ونه_View_Select(0, p, 0);
  }
#د_که_پای
}

/*
** Check to see if the pThis entry of pTabList is a self-join of another view.
** Search FROM-clause entries in the range of iFirst..iEnd, including iFirst
** but stopping before iEnd.
**
** If pThis is a self-join, then return the SrcItem for the first other
** instance of that view found.  If pThis is not a self-join then return 0.
*/
ساكن Src_توکی *isSelfJoinView(
  SrcList *pTabList,           /* Search for self-joins in this FROM clause */
  Src_توکی *pThis,              /* Search for prior reference to this subquery */
  صحيح i_لومړی, صحيح i_پای_ټکی        /* Range of FROM-clause entries to search. */
){
  Src_توکی *p_توکی;
  Select *pSel;
  أكد( pThis->fg.isSubquery );
  pSel = pThis->u4.pSubq->pSelect;
  أكد( pSel!=0 );
  إذا( pSel->selFlags & SF_ټېل_ښکته ) أرجع 0;
  طالما( i_لومړی<i_پای_ټکی ){
    Select *pS1;
    p_توکی = &pTabList->a[i_لومړی++];
    إذا( !p_توکی->fg.isSubquery ) استمر;
    إذا( p_توکی->fg.له_لارې_Coroutine ) استمر;
    إذا( p_توکی->zName==0 ) استمر;
    أكد( p_توکی->pSTab!=0 );
    أكد( pThis->pSTab!=0 );
    إذا( p_توکی->pSTab->pSchema!=pThis->pSTab->pSchema ) استمر;
    إذا( sqlite3_stricmp(p_توکی->zName, pThis->zName)!=0 ) استمر;
    pS1 = p_توکی->u4.pSubq->pSelect;
    إذا( p_توکی->pSTab->pSchema==0 && pSel->selId!=pS1->selId ){
      /* The query flattener left two different CTE tables with identical
      ** names in the same FROM clause. */
      استمر;
    }
    إذا( pS1->selFlags & SF_ټېل_ښکته ){
      /* The view was modified by some other optimization such as
      ** pushDownWhereTerms() */
      استمر;
    }
    أرجع p_توکی;
  }
  أرجع 0;
}

/*
** Deallocate a single AggInfo object
*/
ساكن فراغ agginfoFree(sqlite3 *db, فراغ *pArg){
  AggInfo *p = (AggInfo*)pArg;
  sqlite3DbFree(db, p->a_ستون);
  sqlite3DbFree(db, p->aFunc);
  sqlite3DbFreeNN(db, p);
}

/*
** Attempt to transform a query of the form
**
**    SELECT count(*) FROM (SELECT x FROM t1 UNION ALL SELECT y FROM t2)
**
** Into this:
**
**    SELECT (SELECT count(*) FROM t1)+(SELECT count(*) FROM t2)
**
** The transformation only works if all of the following are true:
**
**   *  The subquery is a UNION ALL of two or more terms
**   *  The subquery does not have a LIMIT clause
**   *  There is no WHERE or GROUP BY or HAVING clauses on the subqueries
**   *  The outer query is a simple count(*) with no WHERE clause or other
**      extraneous syntax.
**   *  None of the subqueries are DISTINCT (forumpost/a860f5fb2e 2025-03-10)
**
** Return TRUE if the optimization is undertaken.
*/
ساكن صحيح شمېر_د_View_Optimization(Parse *pParse, Select *p){
  Select *pSub, *p_مخکینی;
  بیان *p_بیان;
  بیان *p_شمېر;
  sqlite3 *db;
  Src_توکی *pFrom;
  إذا( (p->selFlags & SF_Aggregate)==0 ) أرجع 0;   /* This is an aggregate */
  إذا( p->pEList->n_بیان!=1 ) أرجع 0;               /* Single result column */
  إذا( p->pWhere ) أرجع 0;
  إذا( p->pHaving ) أرجع 0;
  إذا( p->p_ډله_By ) أرجع 0;
  إذا( p->p_ترتیب_By ) أرجع 0;
  p_بیان = p->pEList->a[0].p_بیان;
  إذا( p_بیان->op!=TK_AGG_FUNCTION ) أرجع 0;        /* Result is an aggregate */
  أكد( بیان_Use_UToken(p_بیان) );
  إذا( sqlite3_stricmp(p_بیان->u.zToken,"count") ) أرجع 0;  /* Is count() */
  أكد( بیان_Use_XList(p_بیان) );
  إذا( p_بیان->x.pList!=0 ) أرجع 0;                 /* Must be count(*) */
  إذا( p->pSrc->nSrc!=1 ) أرجع 0;                  /* One table in FROM  */
  إذا( بیان_Has_Property(p_بیان, EP_WinFunc) ) أرجع 0;/* Not a window function */
  pFrom = p->pSrc->a;
  إذا( pFrom->fg.isSubquery==0 ) أرجع 0;           /* FROM is a subquery */
  إذا( (p->selFlags & SF_Correlated)!=0 ) أرجع 0;  /* Not a correlated subq */
  pSub = pFrom->u4.pSubq->pSelect;
  إذا( pSub->p_مخکینی==0 ) أرجع 0;                   /* Must be a compound */
  إذا( pSub->selFlags & SF_CopyCte ) أرجع 0;       /* Not a CTE */
  افعل{
    إذا( pSub->op!=TK_ALL && pSub->p_مخکینی ) أرجع 0;  /* Must be UNION ALL */
    إذا( pSub->pWhere ) أرجع 0;                      /* No WHERE clause */
    إذا( pSub->p_حد ) أرجع 0;                      /* No LIMIT clause */
    إذا( pSub->selFlags & (SF_Aggregate|SF_Distinct) ){
       testcase( pSub->selFlags & SF_Aggregate );
       testcase( pSub->selFlags & SF_Distinct );
       أرجع 0;     /* Not an aggregate nor DISTINCT */
    }
    أكد( pSub->pHaving==0 );  /* Due to the previous */
    pSub = pSub->p_مخکینی;                              /* Repeat over compound */
  }طالما( pSub );

  /* If we reach this point then it is OK to perform the transformation */

  db = pParse->db;
  p_شمېر = p_بیان;
  p_بیان = 0;
  pSub = sqlite3SubqueryDetach(db, pFrom);
  sqlite3SrcListDelete(db, p->pSrc);
  p->pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  طالما( pSub ){
    بیان *p_غړی;
    p_مخکینی = pSub->p_مخکینی;
    pSub->p_مخکینی = 0;
    pSub->p_بل = 0;
    pSub->selFlags |= SF_Aggregate;
    pSub->selFlags &= ~(u32)SF_Compound;
    pSub->n_Select_لیکه = 0;
    sqlite3_Parser_جمع_کړه_Cleanup(pParse, sqlite3_بیان_List_Delete_Generic, pSub->pEList);
    p_غړی = p_مخکینی ? sqlite3_بیان_Dup(db, p_شمېر, 0) : p_شمېر;
    pSub->pEList = sqlite3_بیان_List_Append(pParse, 0, p_غړی);
    p_غړی = sqlite3PExpr(pParse, TK_SELECT, 0, 0);
    sqlite3_PExpr_جمع_کړه_Select(pParse, p_غړی, pSub);
    إذا( p_بیان==0 ){
      p_بیان = p_غړی;
    }وإلا{
      p_بیان = sqlite3PExpr(pParse, TK_PLUS, p_غړی, p_بیان);
    }
    إذا( pParse->nErr ){
      sqlite3_بیان_Delete(db, p_بیان);
      p_بیان = 0;
    }
    pSub = p_مخکینی;
  }
  p->pEList->a[0].p_بیان = p_بیان;
  p->selFlags &= ~(u32)SF_Aggregate;

#إذا TREETRACE_ENABLED
  إذا( sqlite3_ونه_Trace & 0x200 ){
    TREETRACE(0x200,pParse,p,("After count-of-view optimization:\n"));
    sqlite3_ونه_View_Select(0, p, 0);
  }
#د_که_پای
  أرجع 1;
}

/*
** If any term of pSrc, or any SF_NestedFrom sub-query, is not the same
** as pSrcItem but has the same alias as p0, then return true.
** Otherwise return false.
*/
ساكن صحيح sameSrcAlias(Src_توکی *p0, SrcList *pSrc){
  صحيح i;
  لكل(i=0; i<pSrc->nSrc; i++){
    Src_توکی *p1 = &pSrc->a[i];
    إذا( p1==p0 ) استمر;
    إذا( p0->pSTab==p1->pSTab && 0==sqlite3_stricmp(p0->zAlias, p1->zAlias) ){
      أرجع 1;
    }
    إذا( p1->fg.isSubquery
     && (p1->u4.pSubq->pSelect->selFlags & SF_NestedFrom)!=0
     && sameSrcAlias(p0, p1->u4.pSubq->pSelect->pSrc)
    ){
      أرجع 1;
    }
  }
  أرجع 0;
}

/*
** Return TRUE (non-zero) if the i-th entry in the pTabList SrcList can
** be implemented as a co-routine.  The i-th entry is guaranteed to be
** a subquery.
**
** The subquery is implemented as a co-routine if all of the following are
** true:
**
**    (1)  The subquery will likely be implemented in the outer loop of
**         the query.  This will be the case if any one of the following
**         conditions hold:
**         (a)  The subquery is the only term in the FROM clause
**         (b)  The subquery is the left-most term and a CROSS JOIN or similar
**              requires it to be the outer loop
**         (c)  All of the following are true:
**                (i) The subquery is the left-most subquery in the FROM clause
**               (ii) There is nothing that would prevent the subquery from
**                    being used as the outer loop if the sqlite3WhereBegin()
**                    routine nominates it to that position.
**              (iii) The query is not a UPDATE ... FROM
**    (2)  The subquery is not a CTE that should be materialized because
**         (a) the AS MATERIALIZED keyword is used, or
**         (b) the CTE is used multiple times and does not have the
**             NOT MATERIALIZED keyword
**    (3)  The subquery is not part of a left operand for a RIGHT JOIN
**    (4)  The SQLITE_Coroutine optimization disable flag is not set
**    (5)  The subquery is not self-joined
*/
ساكن صحيح from_Clause_غړی_Can_Be_Coroutine(
  Parse *pParse,          /* Parsing context */
  SrcList *pTabList,      /* FROM clause */
  صحيح i,                  /* Which term of the FROM clause holds the subquery */
  صحيح selFlags            /* Flags on the SELECT statement */
){
  Src_توکی *p_توکی = &pTabList->a[i];
  إذا( p_توکی->fg.isCte ){
    ثابت CteUse *pCteUse = p_توکی->u2.pCteUse;
    إذا( pCteUse->eM10d==M10d_Yes ) أرجع 0;                          /* (2a) */
    إذا( pCteUse->nUse>=2 && pCteUse->eM10d!=M10d_No ) أرجع 0;       /* (2b) */
  }
  إذا( pTabList->a[0].fg.jointype & JT_LTORJ ) أرجع 0;               /* (3)  */
  إذا( OptimizationDisabled(pParse->db, SQLITE_Coroutines) ) أرجع 0; /* (4)  */
  إذا( isSelfJoinView(pTabList, p_توکی, i+1, pTabList->nSrc)!=0 ){
    أرجع 0;                                                          /* (5) */
  }
  إذا( i==0 ){
    إذا( pTabList->nSrc==1 ) أرجع 1;                             /* (1a) */
    إذا( pTabList->a[1].fg.jointype & JT_CROSS ) أرجع 1;         /* (1b) */
    إذا( selFlags & SF_UpdateFrom )              أرجع 0;         /* (1c-iii) */
    أرجع 1;
  }
  إذا( selFlags & SF_UpdateFrom ) أرجع 0;                        /* (1c-iii) */
  طالما( 1 /*exit-by-break*/ ){
    إذا( p_توکی->fg.jointype & (JT_OUTER|JT_CROSS)  ) أرجع 0;     /* (1c-ii) */
    إذا( i==0 ) اكسر;
    i--;
    p_توکی--;
    إذا( p_توکی->fg.isSubquery ) أرجع 0;                          /* (1c-i) */
  }
  أرجع 1;
}

/*
** Argument pWhere is the WHERE clause belonging to SELECT statement p. This
** function attempts to transform expressions of the form:
**
**     EXISTS (SELECT ...)
**
** into joins. For example, given
**
**    CREATE TABLE sailors(sid INTEGER PRIMARY KEY, name TEXT);
**    CREATE TABLE reserves(sid INT, day DATE, PRIMARY KEY(sid, day));
**
**    SELECT name FROM sailors AS S WHERE EXISTS (
**      SELECT * FROM reserves AS R WHERE S.sid = R.sid AND R.day = '2022-10-25'
**    );
**
** the SELECT statement may be transformed as follows:
**
**    SELECT name FROM sailors AS S, reserves AS R
**      WHERE S.sid = R.sid AND R.day = '2022-10-25';
**
** **Approximately**.  Really, we have to ensure that the FROM-clause term
** that was formerly inside the EXISTS is only executed once.  This is handled
** by setting the SrcItem.fg.fromExists flag, which then causes code in
** the where.c file to exit the corresponding loop after the first successful
** match (if any).
*/
ساكن SQLITE_NOINLINE فراغ exists_منزل_Join(
  Parse *pParse,  /* Parsing context */
  Select *p,      /* The SELECT statement being optimized */
  بیان *pWhere    /* part of the WHERE clause currently being examined */
){
  إذا( pParse->nErr==0
   && pWhere!=0
   && !بیان_Has_Property(pWhere, EP_OuterON|EP_InnerON) 
   && ALWAYS(p->pSrc!=0)
   && p->pSrc->nSrc<BMS
   && (p->p_حد==0 || p->p_حد->p_ښی==0)
  ){
    إذا( pWhere->op==TK_AND ){
      بیان *p_ښی = pWhere->p_ښی;
      exists_منزل_Join(pParse, p, pWhere->p_کیڼ);
      exists_منزل_Join(pParse, p, p_ښی);
    }
    وإلا إذا( pWhere->op==TK_EXISTS ){
      Select *pSub = pWhere->x.pSelect;
      بیان *pSubWhere = pSub->pWhere;
      إذا( pSub->pSrc->nSrc==1
       && (pSub->selFlags & SF_Aggregate)==0
       && !pSub->pSrc->a[0].fg.isSubquery
       && pSub->p_حد==0
       && pSub->p_مخکینی==0
      ){
        /* Before combining the sub-select with the parent, renumber the 
        ** cursor used by the subselect. This is because the EXISTS expression
        ** might be a copy of another EXISTS expression from somewhere
        ** else in the tree, and in this case it is important that it use
        ** a unique cursor number.  */
        sqlite3 *db = pParse->db;
        صحيح *aCsrMap = sqlite3DbMallocZero(db, (pParse->nTab+2)*حجم(صحيح));
        إذا( aCsrMap==0 ) أرجع;
        aCsrMap[0] = (pParse->nTab+1);
        renumberCursors(pParse, pSub, -1, aCsrMap);
        sqlite3DbFree(db, aCsrMap);

        memset(pWhere, 0, حجم(*pWhere));
        pWhere->op = TK_INTEGER;
        pWhere->u.i_ارزښت = 1;
        بیان_سېټ_Property(pWhere, EP_Int_ارزښت);
        أكد( p->pWhere!=0 );
        pSub->pSrc->a[0].fg.fromExists = 1;
        p->pSrc = sqlite3SrcListAppendList(pParse, p->pSrc, pSub->pSrc);
        إذا( pSubWhere ){
          p->pWhere = sqlite3PExpr(pParse, TK_AND, p->pWhere, pSubWhere);
          pSub->pWhere = 0;
        }
        pSub->pSrc = 0;
        sqlite3_Parser_جمع_کړه_Cleanup(pParse, sqlite3SelectDeleteGeneric, pSub);
        recompute_Columns_کارول_شوی(p, &p->pSrc->a[p->pSrc->nSrc-1]);
#إذا TREETRACE_ENABLED
        إذا( sqlite3_ونه_Trace & 0x100000 ){
          TREETRACE(0x100000,pParse,p,
                    ("After EXISTS-to-JOIN optimization:\n"));
          sqlite3_ونه_View_Select(0, p, 0);
        }
#د_که_پای
      }
    }
  }
}

/*
** Type used for Walker callbacks by selectCheckOnClauses().
*/
تعريفنوع بنية CheckOnCtx CheckOnCtx;
بنية CheckOnCtx {
  SrcList *pSrc;       /* SrcList for this context */
  صحيح iJoin;           /* Cursors must be left of this one, if not negative */
  صحيح bFuncArg;        /* True for table-function arg */
  CheckOnCtx *p_پلار; /* Parent context */
};

/*
** True if the SrcList passed as the only argument contains at least
** one RIGHT or FULL JOIN. False otherwise.  
*/
#تعریف_کړه has_ښی_Join(pSrc) (((pSrc)->a[0].fg.jointype & JT_LTORJ)!=0)

/*
** The xExpr callback for the search of invalid ON clause terms.
*/
ساكن صحيح select_Check_On_Clauses_بیان(Walker *pWalker, بیان *p_بیان){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;

  /* Check if pExpr is root or near-root of an ON clause constraint that needs
  ** to be checked to ensure that it does not refer to tables in its FROM
  ** clause to the right of itself. i.e. it is either:
  **
  **   + an ON clause on an OUTER join, or
  **   + an ON clause on an INNER join within a FROM that features at
  **     least one RIGHT or FULL join.
  */
  إذا( (بیان_Has_Property(p_بیان, EP_OuterON))
   || (بیان_Has_Property(p_بیان, EP_InnerON) && has_ښی_Join(pCtx->pSrc))
  ){
    /* If CheckOnCtx.iJoin is already set, then fall through and process
    ** this expression node as normal. Or, if CheckOnCtx.iJoin is still 0,
    ** set it to the cursor number of the RHS of the join to which this
    ** ON expression was attached and then iterate through the entire 
    ** expression.  */
    أكد( pCtx->iJoin<0 || pCtx->iJoin==p_بیان->w.iJoin );
    إذا( pCtx->iJoin<0 ){
      pCtx->iJoin = p_بیان->w.iJoin;
      sqlite3_Walk_بیان_NN(pWalker, p_بیان);
      pCtx->iJoin = -1;
      أرجع WRC_Prune;
    }
  }

  إذا( p_بیان->op==TK_ستنه ){
    /* A column expression. Find the SrcList (if any) to which it refers.
    ** Then, if CheckOnCtx.iJoin indicates that this expression is part of an
    ** ON clause from that SrcList (i.e. if iJoin is non-zero), check that it
    ** does not refer to a table to the right of CheckOnCtx.iJoin. */
    صحيح iTab = p_بیان->i_جدول;
    افعل {
      SrcList *pSrc = pCtx->pSrc;
      صحيح nSrc = pSrc->nSrc;
      صحيح ii;
      لكل(ii=0; ii<nSrc && pSrc->a[ii].iCursor!=iTab; ii++){}
      إذا( ii<nSrc ){
        /* pSrc is the FROM clause that contains iTab */
        إذا( pCtx->iJoin>=0 ){
          لكل(ii--; ii>=0 && pSrc->a[ii].iCursor!=pCtx->iJoin; ii--){}
          إذا( ii>=0 ){
            /* Table iJoin appears to the left of table iTab in the SrcList.
            ** Therefore the expression refers to a table to its right. */
            sqlite3_Error_پیغام(pWalker->pParse, 
                "%s references tables to its right",
                (pCtx->bFuncArg ? "table-function argument" : "ON clause")
            );
            أرجع WRC_Abort;
          }
        }
        اكسر;
      }
      pCtx = pCtx->p_پلار;
    }طالما( pCtx );
  }
  أرجع WRC_Continue;
}

/*
** The xSelect callback for the search of invalid ON clause terms.
*/
ساكن صحيح selectCheckOnClausesSelect(Walker *pWalker, Select *pSelect){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;
  إذا( pSelect->pSrc==pCtx->pSrc || pSelect->pSrc->nSrc==0 ){
    أرجع WRC_Continue;
  }وإلا{
    CheckOnCtx sCtx;
    memset(&sCtx, 0, حجم(sCtx));
    sCtx.pSrc = pSelect->pSrc;
    sCtx.p_پلار = pCtx;
    sCtx.iJoin = -1;
    pWalker->u.pCheckOnCtx = &sCtx;
    sqlite3WalkSelect(pWalker, pSelect);
    pWalker->u.pCheckOnCtx = pCtx;
    pSelect->selFlags &= ~SF_On_منزل_Where;
    أرجع WRC_Prune;
  }
}

/*
** Check all ON clauses in pSelect to verify that they do not reference
** columns to the right.
*/
فراغ sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect){
  Walker w;
  CheckOnCtx sCtx;
  صحيح ii;
  أكد( pSelect->selFlags & SF_On_منزل_Where );
  أكد( pSelect->pSrc!=0 && pSelect->pSrc->nSrc>=2 );
  memset(&w, 0, حجم(w));
  w.pParse = pParse;
  w.x_بیان_Callback = select_Check_On_Clauses_بیان;
  w.xSelectCallback = selectCheckOnClausesSelect;
  w.u.pCheckOnCtx = &sCtx;
  memset(&sCtx, 0, حجم(sCtx));
  sCtx.pSrc = pSelect->pSrc;
  sCtx.iJoin = -1;
  sqlite3_Walk_بیان(&w, pSelect->pWhere);
  pSelect->selFlags &= ~SF_On_منزل_Where;

  /* Check for any table-function args that are attached to virtual tables 
  ** on the RHS of an outer join. They are subject to similar constraints
  ** as ON clauses. Specifically:
  **
  ** * If the table-valued function is to the left of any RIGHT JOIN, or if
  **   it is the RHS of a RIGHT JOIN, then the table-valued function arguments
  **   may not refer to any tables to the right of this one.
  **
  ** * If the table-valued function is to the left of a LEFT JOIN, then it
  **   may not refer to any table that occurs to the right of the LEFT JOIN.
  */
  sCtx.bFuncArg = 1;
  لكل(ii=0; ii<pSelect->pSrc->nSrc; ii++){
    Src_توکی *p_توکی = &pSelect->pSrc->a[ii];
    إذا( p_توکی->fg.isTabFunc ){
      sCtx.iJoin = -1;
      إذا( (p_توکی->fg.jointype & (JT_LTORJ|JT_ښی)) ){
        sCtx.iJoin = p_توکی->iCursor;
      }وإلا{
        صحيح jj;
        لكل(jj=ii+1; jj<pSelect->pSrc->nSrc; jj++){
          Src_توکی *pItem2 = &pSelect->pSrc->a[jj];
          إذا( pItem2->fg.jointype & JT_OUTER ){
            sCtx.iJoin = pItem2[-1].iCursor;
            اكسر;
          }
        }
      }
      إذا( sCtx.iJoin>=0 ){
        sqlite3_Walk_بیان_List(&w, p_توکی->u1.pFuncArg);
      }
    }
  }
}

/*
** If p2 exists and p1 and p2 have the same number of terms, then change
** every term of p1 to have the same sort order as p2 and return true.
**
** If p2 is NULL or p1 and p2 are different lengths, then make no changes
** and return false.
**
** p1 must be non-NULL.
*/
ساكن صحيح sqlite3_Copy_ترتیبول_ترتیب(بیان_List *p1, بیان_List *p2){
  أكد( p1 );
  إذا( p2 && p1->n_بیان==p2->n_بیان ){
    صحيح ii;
    لكل(ii=0; ii<p1->n_بیان; ii++){
      u8 ترتیبول_Flags;
      ترتیبول_Flags = p2->a[ii].fg.ترتیبول_Flags & KEYINFO_ترتیب_DESC;
      p1->a[ii].fg.ترتیبول_Flags = ترتیبول_Flags;
    }
    أرجع 1;
  }وإلا{
    أرجع 0;
  }
}

/*
** Generate byte-code for the SELECT statement given in the p argument. 
**
** The results are returned according to the SelectDest structure.
** See comments in sqliteInt.h for further information.
**
** This routine returns the number of errors.  If any errors are
** encountered, then an appropriate error message is left in
** pParse->zErrMsg.
**
** This routine does NOT free the Select structure passed in.  The
** calling function needs to do that.
**
** This is a long function.  The following is an outline of the processing
** steps, with tags referencing various milestones:
**
**  *  Resolve names and similar preparation                tag-select-0100
**  *  Scan of the FROM clause                              tag-select-0200
**      +  OUTER JOIN strength reduction                      tag-select-0220
**      +  Sub-query ORDER BY removal                         tag-select-0230
**      +  Query flattening                                   tag-select-0240
**  *  Separate subroutine for compound-SELECT              tag-select-0300
**  *  WHERE-clause constant propagation                    tag-select-0330
**  *  Count()-of-VIEW optimization                         tag-select-0350
**  *  Scan of the FROM clause again                        tag-select-0400
**      +  Authorize unreferenced tables                      tag-select-0410
**      +  Predicate push-down optimization                   tag-select-0420
**      +  Omit unused subquery columns optimization          tag-select-0440
**      +  Generate code to implement subqueries              tag-select-0480
**         -  Co-routines                                       tag-select-0482
**         -  Reuse previously computed CTE                     tag-select-0484
**         -  REuse previously computed VIEW                    tag-select-0486
**         -  Materialize a VIEW or CTE                         tag-select-0488
**  *  DISTINCT ORDER BY -> GROUP BY optimization           tag-select-0500
**  *  Set up for ORDER BY                                  tag-select-0600
**  *  Create output table                                  tag-select-0630
**  *  Prepare registers for LIMIT                          tag-select-0650
**  *  Setup for DISTINCT                                   tag-select-0680
**  *  Generate code for non-aggregate and non-GROUP BY     tag-select-0700
**  *  Generate code for aggregate and/or GROUP BY          tag-select-0800
**      +  GROUP BY queries                                   tag-select-0810
**      +  non-GROUP BY queries                               tag-select-0820
**         -  Special case of count() w/o GROUP BY              tag-select-0821
**         -  General case of non-GROUP BY aggregates           tag-select-0822
**  *  Sort results, as needed                              tag-select-0900
**  *  Internal self-checks                                 tag-select-1000
*/
صحيح sqlite3Select(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  SelectDest *pDest      /* What to do with the query results */
){
  صحيح i, j;              /* Loop counters */
  WhereInfo *pWInfo;     /* Return from sqlite3WhereBegin() */
  Vdbe *v;               /* The virtual machine under construction */
  صحيح isAgg;             /* True for select lists like "count(*)" */
  بیان_List *pEList = 0;  /* List of columns to extract. */
  SrcList *pTabList;     /* List of tables to select from */
  بیان *pWhere;          /* The WHERE clause.  May be NULL */
  بیان_List *p_ډله_By;    /* The GROUP BY clause.  May be NULL */
  بیان *pHaving;         /* The HAVING clause.  May be NULL */
  AggInfo *pAggInfo = 0; /* Aggregate information */
  صحيح rc = 1;            /* Value to return from this function */
  DistinctCtx sDistinct; /* Info on how to code the DISTINCT keyword */
  ترتیبول_Ctx s_ترتیبول;         /* Info on how to code the ORDER BY clause */
  صحيح i_پای_ټکی;              /* Address of the end of the query */
  sqlite3 *db;           /* The database connection */
  بیان_List *p_Min_Max_ترتیب_By = 0;  /* Added ORDER BY for min/max queries */
  u8 minMaxFlag;                 /* Flag for min/max queries */

  db = pParse->db;
  أكد( pParse==db->pParse );
  v = sqlite3GetVdbe(pParse);
  إذا( p==0 || pParse->nErr ){
    أرجع 1;
  }
  أكد( db->mallocFailed==0 );
  إذا( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) أرجع 1;
#إذا TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p, ("begin processing:\n", pParse->addrExplain));
  إذا( sqlite3_ونه_Trace & 0x10000 ){
    إذا( (sqlite3_ونه_Trace & 0x10001)==0x10000 ){
      sqlite3_ونه_View_کرښه(0, "In sqlite3Select() at %s:%d",
                           __FILE__, __LINE__);
    }
    sqlite3ShowSelect(p);
  }
#د_که_پای

  /* tag-select-0100 */
  أكد( p->p_ترتیب_By==0 || pDest->eDest!=SRT_DistFifo );
  أكد( p->p_ترتیب_By==0 || pDest->eDest!=SRT_Fifo );
  أكد( p->p_ترتیب_By==0 || pDest->eDest!=SRT_Dist_کتار );
  أكد( p->p_ترتیب_By==0 || pDest->eDest!=SRT_کتار );
  إذا( IgnorableDistinct(pDest) ){
    أكد(pDest->eDest==SRT_Exists     || pDest->eDest==SRT_Discard ||
           pDest->eDest==SRT_Dist_کتار  || pDest->eDest==SRT_DistFifo );
    /* All of these destinations are also able to ignore the ORDER BY clause */
    إذا( p->p_ترتیب_By ){
#إذا TREETRACE_ENABLED
      TREETRACE(0x800,pParse,p, ("dropping superfluous ORDER BY:\n"));
      إذا( sqlite3_ونه_Trace & 0x800 ){
        sqlite3_ونه_View_بیان_List(0, p->p_ترتیب_By, 0, "ORDERBY");
      }
#د_که_پای   
      sqlite3_Parser_جمع_کړه_Cleanup(pParse, sqlite3_بیان_List_Delete_Generic,
                              p->p_ترتیب_By);
      testcase( pParse->earlyCleanup );
      p->p_ترتیب_By = 0;
    }
    p->selFlags &= ~(u32)SF_Distinct;
  }
  sqlite3SelectPrep(pParse, p, 0);
  إذا( pParse->nErr ){
    اذهب select_پای_ټکی;
  }
  أكد( db->mallocFailed==0 );
  أكد( p->pEList!=0 );
#إذا TREETRACE_ENABLED
  إذا( sqlite3_ونه_Trace & 0x10 ){
    TREETRACE(0x10,pParse,p, ("after name resolution:\n"));
    sqlite3_ونه_View_Select(0, p, 0);
  }
#د_که_پای

  /* If the SF_UFSrcCheck flag is set, then this function is being called
  ** as part of populating the temp table for an UPDATE...FROM statement.
  ** In this case, it is an error if the target object (pSrc->a[0]) name
  ** or alias is duplicated within FROM clause (pSrc->a[1..n]). 
  **
  ** Postgres disallows this case too. The reason is that some other
  ** systems handle this case differently, and not all the same way,
  ** which is just confusing. To avoid this, we follow PG's lead and
  ** disallow it altogether.  */
  إذا( p->selFlags & SF_UFSrcCheck ){
    Src_توکی *p0 = &p->pSrc->a[0];
    إذا( sameSrcAlias(p0, p->pSrc) ){
      sqlite3_Error_پیغام(pParse,
          "target object/alias may not appear in FROM clause: %s",
          p0->zAlias ? p0->zAlias : p0->pSTab->zName
      );
      اذهب select_پای_ټکی;
    }

    /* Clear the SF_UFSrcCheck flag. The check has already been performed,
    ** and leaving this flag set can cause errors if a compound sub-query
    ** in p->pSrc is flattened into this query and this function called
    ** again as part of compound SELECT processing.  */
    p->selFlags &= ~(u32)SF_UFSrcCheck;
  }

  إذا( pDest->eDest==SRT_Output ){
    sqlite3_Generate_ستنه_نومونه(pParse, p);
  }

#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
  إذا( sqlite3_کړکۍ_Rewrite(pParse, p) ){
    أكد( pParse->nErr );
    اذهب select_پای_ټکی;
  }
#إذا TREETRACE_ENABLED
  إذا( p->pWin && (sqlite3_ونه_Trace & 0x40)!=0 ){
    TREETRACE(0x40,pParse,p, ("after window rewrite:\n"));
    sqlite3_ونه_View_Select(0, p, 0);
  }
#د_که_پای
#د_که_پای /* SQLITE_OMIT_WINDOWFUNC */
  pTabList = p->pSrc;
  isAgg = (p->selFlags & SF_Aggregate)!=0;
  memset(&s_ترتیبول, 0, حجم(s_ترتیبول));
  s_ترتیبول.p_ترتیب_By = p->p_ترتیب_By;

  /* Try to do various optimizations (flattening subqueries, and strength
  ** reduction of join operators) in the FROM clause up into the main query
  ** tag-select-0200
  */
#إذا !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
  لكل(i=0; !p->p_مخکینی && i<pTabList->nSrc; i++){
    Src_توکی *p_توکی = &pTabList->a[i];
    Select *pSub = p_توکی->fg.isSubquery ? p_توکی->u4.pSubq->pSelect : 0;
    جدول *pTab = p_توکی->pSTab;

    /* The expander should have already created transient Table objects
    ** even for FROM clause elements such as subqueries that do not correspond
    ** to a real table */
    أكد( pTab!=0 );

    /* Try to simplify joins:
    **
    **      LEFT JOIN  ->  JOIN
    **     RIGHT JOIN  ->  JOIN
    **      FULL JOIN  ->  RIGHT JOIN
    **
    ** If terms of the i-th table are used in the WHERE clause in such a
    ** way that the i-th table cannot be the NULL row of a join, then
    ** perform the appropriate simplification. This is called
    ** "OUTER JOIN strength reduction" in the SQLite documentation.
    ** tag-select-0220
    */
    إذا( (p_توکی->fg.jointype & (JT_کیڼ|JT_LTORJ))!=0
     && sqlite3_بیان_پایله_ورکوي_Non_Null_لیکه(p->pWhere, p_توکی->iCursor,
                                     p_توکی->fg.jointype & JT_LTORJ)
     && OptimizationEnabled(db, SQLITE_SimplifyJoin)
    ){
      إذا( p_توکی->fg.jointype & JT_کیڼ ){
        إذا( p_توکی->fg.jointype & JT_ښی ){
          TREETRACE(0x1000,pParse,p,
                    ("FULL-JOIN simplifies to RIGHT-JOIN on term %d\n",i));
          p_توکی->fg.jointype &= ~JT_کیڼ;
        }وإلا{
          TREETRACE(0x1000,pParse,p,
                    ("LEFT-JOIN simplifies to JOIN on term %d\n",i));
          p_توکی->fg.jointype &= ~(JT_کیڼ|JT_OUTER);
          unset_Join_بیان(p->pWhere, p_توکی->iCursor, 0);
        }
      }
      إذا( p_توکی->fg.jointype & JT_LTORJ ){
        لكل(j=i+1; j<pTabList->nSrc; j++){
          Src_توکی *pI2 = &pTabList->a[j];
          إذا( pI2->fg.jointype & JT_ښی ){
            إذا( pI2->fg.jointype & JT_کیڼ ){
              TREETRACE(0x1000,pParse,p,
                        ("FULL-JOIN simplifies to LEFT-JOIN on term %d\n",j));
              pI2->fg.jointype &= ~JT_ښی;
            }وإلا{
              TREETRACE(0x1000,pParse,p,
                        ("RIGHT-JOIN simplifies to JOIN on term %d\n",j));
              pI2->fg.jointype &= ~(JT_ښی|JT_OUTER);
              unset_Join_بیان(p->pWhere, pI2->iCursor, 1);
            }
          }
        }
        لكل(j=pTabList->nSrc-1; j>=0; j--){
          pTabList->a[j].fg.jointype &= ~JT_LTORJ;
          إذا( pTabList->a[j].fg.jointype & JT_ښی ) اكسر;
        }
      }
    }

    /* No further action if this term of the FROM clause is not a subquery */
    إذا( pSub==0 ) استمر;

    /* Catch mismatch in the declared columns of a view and the number of
    ** columns in the SELECT on the RHS */
    إذا( pTab->n_ستون!=pSub->pEList->n_بیان ){
      sqlite3_Error_پیغام(pParse, "expected %d columns for '%s' but got %d",
                      pTab->n_ستون, pTab->zName, pSub->pEList->n_بیان);
      اذهب select_پای_ټکی;
    }

    /* Do not attempt the usual optimizations (flattening and ORDER BY
    ** elimination) on a MATERIALIZED common table expression because
    ** a MATERIALIZED common table expression is an optimization fence.
    */
    إذا( p_توکی->fg.isCte && p_توکی->u2.pCteUse->eM10d==M10d_Yes ){
      استمر;
    }

    /* Do not try to flatten an aggregate subquery.
    **
    ** Flattening an aggregate subquery is only possible if the outer query
    ** is not a join.  But if the outer query is not a join, then the subquery
    ** will be implemented as a co-routine and there is no advantage to
    ** flattening in that case.
    */
    إذا( (pSub->selFlags & SF_Aggregate)!=0 ) استمر;
    أكد( pSub->p_ډله_By==0 );

    /* tag-select-0230:
    ** If a FROM-clause subquery has an ORDER BY clause that is not
    ** really doing anything, then delete it now so that it does not
    ** interfere with query flattening.  See the discussion at
    ** https://sqlite.org/forum/forumpost/2d76f2bcf65d256a
    **
    ** Beware of these cases where the ORDER BY clause may not be safely
    ** omitted:
    **
    **    (1)   There is also a LIMIT clause
    **    (2)   The subquery was added to help with window-function
    **          processing
    **    (3)   The subquery is in the FROM clause of an UPDATE
    **    (4)   The outer query uses an aggregate function other than
    **          the built-in count(), min(), or max().
    **    (5)   The ORDER BY isn't going to accomplish anything because
    **          one of:
    **            (a)  The outer query has a different ORDER BY clause
    **            (b)  The subquery is part of a join
    **          See forum post 062d576715d277c8
    **    (6)   The subquery is not a recursive CTE.  ORDER BY has a different
    **          meaning for recursive CTEs and this optimization does not
    **          apply.
    **
    ** Also retain the ORDER BY if the OmitOrderBy optimization is disabled.
    */
    إذا( pSub->p_ترتیب_By!=0
     && (p->p_ترتیب_By!=0 || pTabList->nSrc>1)      /* Condition (5) */
     && pSub->p_حد==0                           /* Condition (1) */
     && (pSub->selFlags & (SF_ترتیب_By_Reqd|SF_Recursive))==0  /* (2) and (6) */
     && (p->selFlags & SF_ترتیب_By_Reqd)==0         /* Condition (3) and (4) */
     && OptimizationEnabled(db, SQLITE_Omit_ترتیب_By)
    ){
      TREETRACE(0x800,pParse,p,
                ("omit superfluous ORDER BY on %r FROM-clause subquery\n",i+1));
      sqlite3_Parser_جمع_کړه_Cleanup(pParse, sqlite3_بیان_List_Delete_Generic,
                              pSub->p_ترتیب_By);
      pSub->p_ترتیب_By = 0;
    }

    /* If the outer query contains a "complex" result set (that is,
    ** if the result set of the outer query uses functions or subqueries)
    ** and if the subquery contains an ORDER BY clause and if
    ** it will be implemented as a co-routine, then do not flatten.  This
    ** restriction allows SQL constructs like this:
    **
    **  SELECT expensive_function(x)
    **    FROM (SELECT x FROM tab ORDER BY y LIMIT 10);
    **
    ** The expensive_function() is only computed on the 10 rows that
    ** are output, rather than every row of the table.
    **
    ** The requirement that the outer query have a complex result set
    ** means that flattening does occur on simpler SQL constraints without
    ** the expensive_function() like:
    **
    **  SELECT x FROM (SELECT x FROM tab ORDER BY y LIMIT 10);
    */
    إذا( pSub->p_ترتیب_By!=0
     && i==0
     && (p->selFlags & SF_Complex_پایله)!=0
     && (pTabList->nSrc==1
         || (pTabList->a[1].fg.jointype&(JT_OUTER|JT_CROSS))!=0)
    ){
      استمر;
    }

    /* tag-select-0240 */
    إذا( flattenSubquery(pParse, p, i, isAgg) ){
      إذا( pParse->nErr ) اذهب select_پای_ټکی;
      /* This subquery can be absorbed into its parent. */
      i = -1;
    }
    pTabList = p->pSrc;
    إذا( db->mallocFailed ) اذهب select_پای_ټکی;
    إذا( !IgnorableOrderby(pDest) ){
      s_ترتیبول.p_ترتیب_By = p->p_ترتیب_By;
    }
  }
#د_که_پای

#که_نه_تعریف_شوی SQLITE_OMIT_COMPOUND_SELECT
  /* Handle compound SELECT statements using the separate multiSelect()
  ** procedure.  tag-select-0300
  */
  إذا( p->p_مخکینی ){
    rc = multiSelect(pParse, p, pDest);
#إذا TREETRACE_ENABLED
    TREETRACE(0x400,pParse,p,("end compound-select processing\n"));
    إذا( (sqlite3_ونه_Trace & 0x400)!=0 && Explain_Query_Plan_پلار(pParse)==0 ){
      sqlite3_ونه_View_Select(0, p, 0);
    }
#د_که_پای
    إذا( p->p_بل==0 ) Explain_Query_Plan_راوباسه(pParse);
    أرجع rc;
  }
#د_که_پای

  /* If there may be an "EXISTS (SELECT ...)" in the WHERE clause, attempt
  ** to change it into a join.  */
  إذا( pParse->bHasExists && OptimizationEnabled(db,SQLITE_Exists_منزل_Join) ){
    exists_منزل_Join(pParse, p, p->pWhere);
    pTabList = p->pSrc;
  }

  /* Do the WHERE-clause constant propagation optimization if this is
  ** a join.  No need to spend time on this operation for non-join queries
  ** as the equivalent optimization will be handled by query planner in
  ** sqlite3WhereBegin().  tag-select-0330
  */
  إذا( p->pWhere!=0
   && p->pWhere->op==TK_AND
   && OptimizationEnabled(db, SQLITE_PropagateConst)
   && propagateConstants(pParse, p)
  ){
#إذا TREETRACE_ENABLED
    إذا( sqlite3_ونه_Trace & 0x2000 ){
      TREETRACE(0x2000,pParse,p,("After constant propagation:\n"));
      sqlite3_ونه_View_Select(0, p, 0);
    }
#د_که_پای
  }وإلا{
    TREETRACE(0x2000,pParse,p,("Constant propagation not helpful\n"));
  }

  /* tag-select-0350 */
  إذا( OptimizationEnabled(db, SQLITE_QueryFlattener|SQLITE_شمېر_د_View)
   && شمېر_د_View_Optimization(pParse, p)
  ){
    إذا( db->mallocFailed ) اذهب select_پای_ټکی;
    pTabList = p->pSrc;
  }

  /* Loop over all terms in the FROM clause and do two things for each term:
  **
  **   (1) Authorize unreferenced tables
  **   (2) Generate code for all sub-queries
  **
  ** tag-select-0400
  */
  لكل(i=0; i<pTabList->nSrc; i++){
    Src_توکی *p_توکی = &pTabList->a[i];
    Src_توکی *p_مخکینی;
    SelectDest dest;
    Subquery *pSubq;
    Select *pSub;
#إذا !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    ثابت حرف *zSavedAuthContext;
#د_که_پای

    /* Authorized unreferenced tables.  tag-select-0410
    **
    ** Issue SQLITE_READ authorizations with a fake column name for any
    ** tables that are referenced but from which no values are extracted.
    ** Examples of where these kinds of null SQLITE_READ authorizations
    ** would occur:
    **
    **     SELECT count(*) FROM t1;   -- SQLITE_READ t1.""
    **     SELECT t1.* FROM t1, t2;   -- SQLITE_READ t2.""
    **
    ** The fake column name is an empty string.  It is possible for a table to
    ** have a column named by the empty string, in which case there is no way to
    ** distinguish between an unreferenced table and an actual reference to the
    ** "" column. The original design was for the fake column name to be a NULL,
    ** which would be unambiguous.  But legacy authorization callbacks might
    ** assume the column name is non-NULL and segfault.  The use of an empty
    ** string for the fake column name seems safer.
    */
    إذا( p_توکی->ستون_کارول_شوی==0 && p_توکی->zName!=0 ){
      ثابت حرف *zDb;
      إذا( p_توکی->fg.fixedSchema ){
        صحيح iDb = sqlite3_Schema_منزل_شاخص(pParse->db, p_توکی->u4.pSchema);
        zDb = db->aDb[iDb].zDbSName;
      }وإلا إذا( p_توکی->fg.isSubquery ){
        zDb = 0;
      }وإلا{
        zDb = p_توکی->u4.zDatabase;
      }
      sqlite3AuthCheck(pParse, SQLITE_READ, p_توکی->zName, "", zDb);
    }

#إذا !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    /* Generate code for all sub-queries in the FROM clause
    */
    إذا( p_توکی->fg.isSubquery==0 ) استمر;
    pSubq = p_توکی->u4.pSubq;
    أكد( pSubq!=0 );
    pSub = pSubq->pSelect;

    /* The code for a subquery should only be generated once. */
    إذا( pSubq->addrFillSub!=0 ) استمر;

    /* Increment Parse.nHeight by the height of the largest expression
    ** tree referred to by this, the parent select. The child select
    ** may contain expression trees of at most
    ** (SQLITE_MAX_EXPR_DEPTH-Parse.nHeight) height. This is a bit
    ** more conservative than necessary, but much easier than enforcing
    ** an exact limit.
    */
    pParse->n_لوړوالی += sqlite3_Select_بیان_لوړوالی(p);

    /* Make copies of constant WHERE-clause terms in the outer query down
    ** inside the subquery.  This can help the subquery to run more efficiently.
    ** This is the "predicate push-down optimization".  tag-select-0420
    */
    إذا( OptimizationEnabled(db, SQLITE_ټېل_ښکته)
     && (p_توکی->fg.isCte==0
         || (p_توکی->u2.pCteUse->eM10d!=M10d_Yes && p_توکی->u2.pCteUse->nUse<2))
     && ټېل_ښکته_Where_Terms(pParse, pSub, p->pWhere, pTabList, i)
    ){
#إذا TREETRACE_ENABLED
      إذا( sqlite3_ونه_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("After WHERE-clause push-down into subquery %d:\n", pSub->selId));
        sqlite3_ونه_View_Select(0, p, 0);
      }
#د_که_پای
      أكد( pSubq->pSelect && (pSub->selFlags & SF_ټېل_ښکته)!=0 );
    }وإلا{
      TREETRACE(0x4000,pParse,p,("WHERE-clause push-down not possible\n"));
    }

    /* Convert unused result columns of the subquery into simple NULL
    ** expressions, to avoid unneeded searching and computation.
    ** tag-select-0440
    */
    إذا( OptimizationEnabled(db, SQLITE_Null_ناکارول_شوی_ستون_شمېر)
     && disable_ناکارول_شوی_Subquery_پایله_Columns(p_توکی)
    ){
#إذا TREETRACE_ENABLED
      إذا( sqlite3_ونه_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("Change unused result columns to NULL for subquery %d:\n",
             pSub->selId));
        sqlite3_ونه_View_Select(0, p, 0);
      }
#د_که_پای
    }

    zSavedAuthContext = pParse->zAuthContext;
    pParse->zAuthContext = p_توکی->zName;

    /* Generate byte-code to implement the subquery  tag-select-0480
    */
    إذا( from_Clause_غړی_Can_Be_Coroutine(pParse, pTabList, i, p->selFlags) ){
      /* Implement a co-routine that will return a single row of the result
      ** set on each invocation.  tag-select-0482
      */
      صحيح addr_سر = sqlite3VdbeCurrentAddr(v)+1;
    
      pSubq->regReturn = ++pParse->nMem;
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_InitCoroutine, pSubq->regReturn, 0, addr_سر);
      VdbeComment((v, "%!S", p_توکی));
      pSubq->addrFillSub = addr_سر;
      sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);
      ExplainQueryPlan((pParse, 1, "CO-ROUTINE %!S", p_توکی));
      sqlite3Select(pParse, pSub, &dest);
      p_توکی->pSTab->n_لیکه_Log_Est = pSub->n_Select_لیکه;
      p_توکی->fg.له_لارې_Coroutine = 1;
      pSubq->reg_پایله = dest.iSdst;
      sqlite3_Vdbe_پای_ټکی_Coroutine(v, pSubq->regReturn);
      VdbeComment((v, "end %!S", p_توکی));
      sqlite3_Vdbe_Jump_دلته(v, addr_سر-1);
      sqlite3_Clear_لنډمهاله_Reg_Cache(pParse);
    }وإلا إذا( p_توکی->fg.isCte && p_توکی->u2.pCteUse->addrM9e>0 ){
      /* This is a CTE for which materialization code has already been
      ** generated.  Invoke the subroutine to compute the materialization,
      ** then make the pItem->iCursor be a copy of the ephemeral table that
      ** holds the result of the materialization. tag-select-0484 */
      CteUse *pCteUse = p_توکی->u2.pCteUse;
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Gosub, pCteUse->regRtn, pCteUse->addrM9e);
      إذا( p_توکی->iCursor!=pCteUse->iCur ){
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_OpenDup, p_توکی->iCursor, pCteUse->iCur);
        VdbeComment((v, "%!S", p_توکی));
      }
      pSub->n_Select_لیکه = pCteUse->n_لیکه_Est;
    }وإلا إذا( (p_مخکینی = isSelfJoinView(pTabList, p_توکی, 0, i))!=0 ){
      /* This view has already been materialized by a prior entry in
      ** this same FROM clause.  Reuse it.  tag-select-0486 */
      Subquery *p_مخکینی_Subq;
      أكد( p_مخکینی->fg.isSubquery );
      p_مخکینی_Subq = p_مخکینی->u4.pSubq;
      أكد( p_مخکینی_Subq!=0 );
      إذا( p_مخکینی_Subq->addrFillSub ){
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Gosub, p_مخکینی_Subq->regReturn,
                                       p_مخکینی_Subq->addrFillSub);
      }
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_OpenDup, p_توکی->iCursor, p_مخکینی->iCursor);
      pSub->n_Select_لیکه = p_مخکینی_Subq->pSelect->n_Select_لیکه;
    }وإلا{
      /* Materialize the view.  If the view is not correlated, generate a
      ** subroutine to do the materialization so that subsequent uses of
      ** the same view can reuse the materialization.  tag-select-0488 */
      صحيح سر_Addr;
      صحيح onceAddr = 0;
#که_تعریف_شوی SQLITE_ENABLE_STMT_SCANSTATUS
      صحيح addrExplain;
#د_که_پای

      pSubq->regReturn = ++pParse->nMem;
      سر_Addr = sqlite3_Vdbe_جمع_کړه_Op0(v, OP_Goto);
      pSubq->addrFillSub = سر_Addr+1;
      p_توکی->fg.isMaterialized = 1;
      إذا( p_توکی->fg.isCorrelated==0 ){
        /* If the subquery is not correlated and if we are not inside of
        ** a trigger, then we only need to compute the value of the subquery
        ** once. */
        onceAddr = sqlite3_Vdbe_جمع_کړه_Op0(v, OP_Once); VdbeCoverage(v);
        VdbeComment((v, "materialize %!S", p_توکی));
      }وإلا{
        VdbeNoopComment((v, "materialize %!S", p_توکی));
      }
      sqlite3SelectDestInit(&dest, SRT_EphemTab, p_توکی->iCursor);

      ExplainQueryPlan2(addrExplain, (pParse, 1, "MATERIALIZE %!S", p_توکی));
      sqlite3Select(pParse, pSub, &dest);
      p_توکی->pSTab->n_لیکه_Log_Est = pSub->n_Select_لیکه;
      إذا( onceAddr ) sqlite3_Vdbe_Jump_دلته(v, onceAddr);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Return, pSubq->regReturn, سر_Addr+1);
      VdbeComment((v, "end %!S", p_توکی));
      sqlite3_Vdbe_لټون_حالت_Range(v, addrExplain, addrExplain, -1);
      sqlite3_Vdbe_Jump_دلته(v, سر_Addr);
      sqlite3_Clear_لنډمهاله_Reg_Cache(pParse);
      إذا( p_توکی->fg.isCte && p_توکی->fg.isCorrelated==0 ){
        CteUse *pCteUse = p_توکی->u2.pCteUse;
        pCteUse->addrM9e = pSubq->addrFillSub;
        pCteUse->regRtn = pSubq->regReturn;
        pCteUse->iCur = p_توکی->iCursor;
        pCteUse->n_لیکه_Est = pSub->n_Select_لیکه;
      }
    }
    إذا( db->mallocFailed ) اذهب select_پای_ټکی;
    pParse->n_لوړوالی -= sqlite3_Select_بیان_لوړوالی(p);
    pParse->zAuthContext = zSavedAuthContext;
#د_که_پای
  }

  /* Various elements of the SELECT copied into local variables for
  ** convenience */
  pEList = p->pEList;
  pWhere = p->pWhere;
  p_ډله_By = p->p_ډله_By;
  pHaving = p->pHaving;
  sDistinct.isTnct = (p->selFlags & SF_Distinct)!=0;

#إذا TREETRACE_ENABLED
  إذا( sqlite3_ونه_Trace & 0x8000 ){
    TREETRACE(0x8000,pParse,p,("After all FROM-clause analysis:\n"));
    sqlite3_ونه_View_Select(0, p, 0);
  }
#د_که_پای

  /* tag-select-0500
  **
  ** If the query is DISTINCT with an ORDER BY but is not an aggregate, and
  ** if the select-list is the same as the ORDER BY list, then this query
  ** can be rewritten as a GROUP BY. In other words, this:
  **
  **     SELECT DISTINCT xyz FROM ... ORDER BY xyz
  **
  ** is transformed to:
  **
  **     SELECT xyz FROM ... GROUP BY xyz ORDER BY xyz
  **
  ** The second form is preferred as a single index (or temp-table) may be
  ** used for both the ORDER BY and DISTINCT processing. As originally
  ** written the query must use a temp-table for at least one of the ORDER
  ** BY and DISTINCT, and an index or separate temp-table for the other.
  */
  إذا( (p->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct
   && sqlite3_Copy_ترتیبول_ترتیب(pEList, s_ترتیبول.p_ترتیب_By)
   && sqlite3_بیان_List_Compare(pEList, s_ترتیبول.p_ترتیب_By, -1)==0
   && OptimizationEnabled(db, SQLITE_ډله_By_ترتیب)
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
   && p->pWin==0
#د_که_پای
  ){
    p->selFlags &= ~(u32)SF_Distinct;
    p_ډله_By = p->p_ډله_By = sqlite3_بیان_List_Dup(db, pEList, 0);
    إذا( p_ډله_By ){
      لكل(i=0; i<p_ډله_By->n_بیان; i++){
        p_ډله_By->a[i].u.x.i_ترتیب_By_ستون = i+1;
      }
    }
    p->selFlags |= SF_Aggregate;
    /* Notice that even thought SF_Distinct has been cleared from p->selFlags,
    ** the sDistinct.isTnct is still set.  Hence, isTnct represents the
    ** original setting of the SF_Distinct flag, not the current setting */
    أكد( sDistinct.isTnct );
    sDistinct.isTnct = 2;

#إذا TREETRACE_ENABLED
    إذا( sqlite3_ونه_Trace & 0x20000 ){
      TREETRACE(0x20000,pParse,p,("Transform DISTINCT into GROUP BY:\n"));
      sqlite3_ونه_View_Select(0, p, 0);
    }
#د_که_پای
  }

  /* If there is an ORDER BY clause, then create an ephemeral index to
  ** do the sorting.  But this sorting ephemeral index might end up
  ** being unused if the data can be extracted in pre-sorted order.
  ** If that is the case, then the OP_OpenEphemeral instruction will be
  ** changed to an OP_Noop once we figure out that the sorting index is
  ** not needed.  The sSort.addrSortIndex variable is used to facilitate
  ** that change.  tag-select-0600
  */
  إذا( s_ترتیبول.p_ترتیب_By ){
    کلی_Info *p_کلی_Info;
    p_کلی_Info = sqlite3_کلی_Info_From_بیان_List(
        pParse, s_ترتیبول.p_ترتیب_By, 0, pEList->n_بیان);
    s_ترتیبول.iECursor = pParse->nTab++;
    s_ترتیبول.addr_ترتیبول_شاخص =
      sqlite3_Vdbe_جمع_کړه_Op4(v, OP_OpenEphemeral,
          s_ترتیبول.iECursor, s_ترتیبول.p_ترتیب_By->n_بیان+1+pEList->n_بیان, 0,
          (حرف*)p_کلی_Info, P4_KEYINFO
      );
  }وإلا{
    s_ترتیبول.addr_ترتیبول_شاخص = -1;
  }

  /* If the output is destined for a temporary table, open that table.
  ** tag-select-0630
  */
  إذا( pDest->eDest==SRT_EphemTab ){
    sqlite3_Vdbe_جمع_کړه_Op2(v, OP_OpenEphemeral, pDest->iSDParm, pEList->n_بیان);
    إذا( p->selFlags & SF_NestedFrom ){
      /* Delete or NULL-out result columns that will never be used */
      صحيح ii;
      لكل(ii=pEList->n_بیان-1; ii>0 && pEList->a[ii].fg.b_کارول_شوی==0; ii--){
        sqlite3_بیان_Delete(db, pEList->a[ii].p_بیان);
        sqlite3DbFree(db, pEList->a[ii].zEName);
        pEList->n_بیان--;
      }
      لكل(ii=0; ii<pEList->n_بیان; ii++){
        إذا( pEList->a[ii].fg.b_کارول_شوی==0 ) pEList->a[ii].p_بیان->op = TK_NULL;
      }
    }
  }

  /* Set the limiter.  tag-select-0650
  */
  i_پای_ټکی = sqlite3VdbeMakeLabel(pParse);
  إذا( (p->selFlags & SF_Fixed_حد)==0 ){
    p->n_Select_لیکه = 320;  /* 4 billion rows */
  }
  إذا( p->p_حد ) compute_حد_Registers(pParse, p, i_پای_ټکی);
  إذا( p->i_حد==0 && s_ترتیبول.addr_ترتیبول_شاخص>=0 ){
    sqlite3VdbeChangeOpcode(v, s_ترتیبول.addr_ترتیبول_شاخص, OP_SorterOpen);
    s_ترتیبول.ترتیبول_Flags |= SORTFLAG_UseSorter;
  }

  /* Open an ephemeral index to use for the distinct set. tag-select-0680
  */
  إذا( p->selFlags & SF_Distinct ){
    sDistinct.tabTnct = pParse->nTab++;
    sDistinct.addrTnct = sqlite3_Vdbe_جمع_کړه_Op4(v, OP_OpenEphemeral,
                       sDistinct.tabTnct, 0, 0,
                       (حرف*)sqlite3_کلی_Info_From_بیان_List(pParse, p->pEList,0,0),
                       P4_KEYINFO);
    sqlite3VdbeChangeP5(v, BTREE_UNORDERED);
    sDistinct.eTnctType = WHERE_DISTINCT_UNORDERED;
  }وإلا{
    sDistinct.eTnctType = WHERE_DISTINCT_NOOP;
  }

  إذا( !isAgg && p_ډله_By==0 ){
    /* No aggregate functions and no GROUP BY clause.  tag-select-0700 */
    u16 wctrlFlags = (sDistinct.isTnct ? WHERE_WANT_DISTINCT : 0)
                   | (p->selFlags & SF_Fixed_حد);
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
    کړکۍ *pWin = p->pWin;      /* Main window object (or NULL) */
    إذا( pWin ){
      sqlite3_کړکۍ_کوډ_Init(pParse, p);
    }
#د_که_پای
    أكد( WHERE_USE_حد==SF_Fixed_حد );


    /* Begin the database scan. */
    TREETRACE(0x2,pParse,p,("WhereBegin\n"));
    pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, s_ترتیبول.p_ترتیب_By,
                               p->pEList, p, wctrlFlags, p->n_Select_لیکه);
    إذا( pWInfo==0 ) اذهب select_پای_ټکی;
    إذا( sqlite3_Where_Output_لیکه_شمېر(pWInfo) < p->n_Select_لیکه ){
      p->n_Select_لیکه = sqlite3_Where_Output_لیکه_شمېر(pWInfo);
      إذا( pDest->eDest<=SRT_Dist_کتار && pDest->eDest>=SRT_DistFifo ){
        /* TUNING: For a UNION CTE, because UNION is implies DISTINCT,
        ** reduce the estimated output row count by 8 (LogEst 30). 
        ** Search for tag-20250414a to see other cases */
        p->n_Select_لیکه -= 30;
      }
    }
    إذا( sDistinct.isTnct && sqlite3WhereIsDistinct(pWInfo) ){
      sDistinct.eTnctType = sqlite3WhereIsDistinct(pWInfo);
    }
    إذا( s_ترتیبول.p_ترتیب_By ){
      s_ترتیبول.nOBSat = sqlite3WhereIsOrdered(pWInfo);
      s_ترتیبول.labelOBLopt = sqlite3_Where_ترتیب_By_حد_Opt_Label(pWInfo);
      إذا( s_ترتیبول.nOBSat==s_ترتیبول.p_ترتیب_By->n_بیان ){
        s_ترتیبول.p_ترتیب_By = 0;
      }
    }
    TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));

    /* If sorting index that was created by a prior OP_OpenEphemeral
    ** instruction ended up not being needed, then change the OP_OpenEphemeral
    ** into an OP_Noop.
    */
    إذا( s_ترتیبول.addr_ترتیبول_شاخص>=0 && s_ترتیبول.p_ترتیب_By==0 ){
      sqlite3_Vdbe_Change_منزل_Noop(v, s_ترتیبول.addr_ترتیبول_شاخص);
    }

    أكد( p->pEList==pEList );
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
    إذا( pWin ){
      صحيح addrGosub = sqlite3VdbeMakeLabel(pParse);
      صحيح iCont = sqlite3VdbeMakeLabel(pParse);
      صحيح iBreak = sqlite3VdbeMakeLabel(pParse);
      صحيح regGosub = ++pParse->nMem;

      sqlite3_کړکۍ_کوډ_ګام(pParse, p, pWInfo, regGosub, addrGosub);

      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Goto, 0, iBreak);
      sqlite3VdbeResolveLabel(v, addrGosub);
      VdbeNoopComment((v, "inner-loop subroutine"));
      s_ترتیبول.labelOBLopt = 0;
      selectInnerLoop(pParse, p, -1, &s_ترتیبول, &sDistinct, pDest, iCont, iBreak);
      sqlite3VdbeResolveLabel(v, iCont);
      sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Return, regGosub);
      VdbeComment((v, "end inner-loop subroutine"));
      sqlite3VdbeResolveLabel(v, iBreak);
    }وإلا
#د_که_پای /* SQLITE_OMIT_WINDOWFUNC */
    {
      /* Use the standard inner loop. */
      selectInnerLoop(pParse, p, -1, &s_ترتیبول, &sDistinct, pDest,
          sqlite3WhereContinueLabel(pWInfo),
          sqlite3WhereBreakLabel(pWInfo));

      /* End the database scan loop.
      */
      TREETRACE(0x2,pParse,p,("WhereEnd\n"));
      sqlite3_Where_پای_ټکی(pWInfo);
    }
  }وإلا{
    /* This case is for when there exist aggregate functions or a GROUP BY
    ** clause or both.  tag-select-0800 */
    NameContext sNC;    /* Name context for processing aggregate information */
    صحيح iAMem;          /* First Mem address for storing current GROUP BY */
    صحيح iBMem;          /* First Mem address for previous GROUP BY */
    صحيح iUseFlag;       /* Mem address holding flag indicating that at least
                        ** one row of the input to the aggregator has been
                        ** processed */
    صحيح iAbortFlag;     /* Mem address which causes query abort if positive */
    صحيح ډله_By_ترتیبول;    /* Rows come from source in GROUP BY order */
    صحيح addr_پای_ټکی;        /* End of processing for this SELECT */
    صحيح ترتیبول_PTab = 0;   /* Pseudotable used to decode sorting results */
    صحيح ترتیبول_بهر = 0;    /* Output register from the sorter */
    صحيح ترتیب_By_Grp = 0; /* True if the GROUP BY and ORDER BY are the same */

    /* Remove any and all aliases between the result set and the
    ** GROUP BY clause.
    */
    إذا( p_ډله_By ){
      صحيح k;                        /* Loop counter */
      بنية بیان_List_توکی *p_توکی;  /* For looping over expression in a list */

      لكل(k=p->pEList->n_بیان, p_توکی=p->pEList->a; k>0; k--, p_توکی++){
        p_توکی->u.x.iAlias = 0;
      }
      لكل(k=p_ډله_By->n_بیان, p_توکی=p_ډله_By->a; k>0; k--, p_توکی++){
        p_توکی->u.x.iAlias = 0;
      }
      أكد( 66==sqlite3LogEst(100) );
      إذا( p->n_Select_لیکه>66 ) p->n_Select_لیکه = 66;

      /* If there is both a GROUP BY and an ORDER BY clause and they are
      ** identical, then it may be possible to disable the ORDER BY clause
      ** on the grounds that the GROUP BY will cause elements to come out
      ** in the correct order. It also may not - the GROUP BY might use a
      ** database index that causes rows to be grouped together as required
      ** but not actually sorted. Either way, record the fact that the
      ** ORDER BY and GROUP BY clauses are the same by setting the orderByGrp
      ** variable.  */
      إذا( sqlite3_Copy_ترتیبول_ترتیب(p_ډله_By, s_ترتیبول.p_ترتیب_By)
       && sqlite3_بیان_List_Compare(p_ډله_By, s_ترتیبول.p_ترتیب_By, -1)==0
      ){
        ترتیب_By_Grp = 1;
      }
    }وإلا{
      أكد( 0==sqlite3LogEst(1) );
      p->n_Select_لیکه = 0;
    }

    /* Create a label to jump to when we want to abort the query */
    addr_پای_ټکی = sqlite3VdbeMakeLabel(pParse);

    /* Convert TK_COLUMN nodes into TK_AGG_COLUMN and make entries in
    ** sAggInfo for all TK_AGG_FUNCTION nodes in expressions of the
    ** SELECT statement.
    */
    pAggInfo = sqlite3DbMallocZero(db, حجم(*pAggInfo) );
    إذا( pAggInfo ){
      sqlite3_Parser_جمع_کړه_Cleanup(pParse, agginfoFree, pAggInfo);
      testcase( pParse->earlyCleanup );
    }
    إذا( db->mallocFailed ){
      اذهب select_پای_ټکی;
    }
    pAggInfo->selId = p->selId;
#که_تعریف_شوی SQLITE_DEBUG
    pAggInfo->pSelect = p;
#د_که_پای
    memset(&sNC, 0, حجم(sNC));
    sNC.pParse = pParse;
    sNC.pSrcList = pTabList;
    sNC.uNC.pAggInfo = pAggInfo;
    VVA_ONLY( sNC.ncFlags = NC_UAggInfo; )
    pAggInfo->n_ترتیبونه_ستنه = p_ډله_By ? p_ډله_By->n_بیان : 0;
    pAggInfo->p_ډله_By = p_ډله_By;
    sqlite3_بیان_Analyze_Agg_List(&sNC, pEList);
    sqlite3_بیان_Analyze_Agg_List(&sNC, s_ترتیبول.p_ترتیب_By);
    إذا( pHaving ){
      إذا( p_ډله_By ){
        أكد( pWhere==p->pWhere );
        أكد( pHaving==p->pHaving );
        أكد( p_ډله_By==p->p_ډله_By );
        having_منزل_Where(pParse, p);
        pWhere = p->pWhere;
      }
      sqlite3_بیان_Analyze_Aggregates(&sNC, pHaving);
    }
    pAggInfo->n_راټولونکی = pAggInfo->n_ستنه;
    إذا( p->p_ډله_By==0 && p->pHaving==0 && pAggInfo->nFunc==1 ){
      minMaxFlag = minMaxQuery(db, pAggInfo->aFunc[0].pFExpr, &p_Min_Max_ترتیب_By);
    }وإلا{
      minMaxFlag = WHERE_ORDERBY_نورمال;
    }
    analyze_Agg_Func_دلیلونه(pAggInfo, &sNC);
    إذا( db->mallocFailed ) اذهب select_پای_ټکی;
#إذا TREETRACE_ENABLED
    إذا( sqlite3_ونه_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("After aggregate analysis %p:\n", pAggInfo));
      sqlite3_ونه_View_Select(0, p, 0);
      إذا( minMaxFlag ){
        sqlite3DebugPrintf("MIN/MAX Optimization (0x%02x) adds:\n", minMaxFlag);
        sqlite3_ونه_View_بیان_List(0, p_Min_Max_ترتیب_By, 0, "ORDERBY");
      }
      printAggInfo(pAggInfo);
    }
#د_که_پای


    /* Processing for aggregates with GROUP BY is very different and
    ** much more complex than aggregates without a GROUP BY.  tag-select-0810
    */
    إذا( p_ډله_By ){
      کلی_Info *p_کلی_Info;  /* Keying information for the group by clause */
      صحيح addr1;          /* A-vs-B comparison jump */
      صحيح addr_Output_لیکه;  /* Start of subroutine that outputs a result row */
      صحيح reg_Output_لیکه;   /* Return address register for output subroutine */
      صحيح addr_سېټ_Abort;   /* Set the abort flag and return */
      صحيح addr_سر_د_Loop;  /* Top of the input loop */
      صحيح addr_ترتیبونه_Idx; /* The OP_OpenEphemeral for the sorting index */
      صحيح addrReset;      /* Subroutine for resetting the accumulator */
      صحيح regReset;       /* Return address register for reset subroutine */
      بیان_List *pDistinct = 0;
      u16 distFlag = 0;
      صحيح eDist = WHERE_DISTINCT_NOOP;

      إذا( pAggInfo->nFunc==1
       && pAggInfo->aFunc[0].iDistinct>=0
       && ALWAYS(pAggInfo->aFunc[0].pFExpr!=0)
       && ALWAYS(بیان_Use_XList(pAggInfo->aFunc[0].pFExpr))
       && pAggInfo->aFunc[0].pFExpr->x.pList!=0
      ){
        بیان *p_بیان = pAggInfo->aFunc[0].pFExpr->x.pList->a[0].p_بیان;
        p_بیان = sqlite3_بیان_Dup(db, p_بیان, 0);
        pDistinct = sqlite3_بیان_List_Dup(db, p_ډله_By, 0);
        pDistinct = sqlite3_بیان_List_Append(pParse, pDistinct, p_بیان);
        distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
      }

      /* If there is a GROUP BY clause we might need a sorting index to
      ** implement it.  Allocate that sorting index now.  If it turns out
      ** that we do not need it after all, the OP_SorterOpen instruction
      ** will be converted into a Noop. 
      */
      pAggInfo->ترتیبونه_Idx = pParse->nTab++;
      p_کلی_Info = sqlite3_کلی_Info_From_بیان_List(pParse, p_ډله_By,
                                            0, pAggInfo->n_ستنه);
      addr_ترتیبونه_Idx = sqlite3_Vdbe_جمع_کړه_Op4(v, OP_SorterOpen,
          pAggInfo->ترتیبونه_Idx, pAggInfo->n_ترتیبونه_ستنه,
          0, (حرف*)p_کلی_Info, P4_KEYINFO);

      /* Initialize memory locations used by GROUP BY aggregate processing
      */
      iUseFlag = ++pParse->nMem;
      iAbortFlag = ++pParse->nMem;
      reg_Output_لیکه = ++pParse->nMem;
      addr_Output_لیکه = sqlite3VdbeMakeLabel(pParse);
      regReset = ++pParse->nMem;
      addrReset = sqlite3VdbeMakeLabel(pParse);
      iAMem = pParse->nMem + 1;
      pParse->nMem += p_ډله_By->n_بیان;
      iBMem = pParse->nMem + 1;
      pParse->nMem += p_ډله_By->n_بیان;
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, 0, iAbortFlag);
      VdbeComment((v, "clear abort flag"));
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Null, 0, iAMem, iAMem+p_ډله_By->n_بیان-1);
      sqlite3_بیان_Null_ثبتوونکی_Range(pParse, iAMem, p_ډله_By->n_بیان);

      /* Begin a loop that will extract all source rows in GROUP BY order.
      ** This might involve two separate loops with an OP_Sort in between, or
      ** it might be a single loop that uses an index to extract information
      ** in the right order to begin with.
      */
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Gosub, regReset, addrReset);
      TREETRACE(0x2,pParse,p,("WhereBegin\n"));
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, p_ډله_By, pDistinct,
          p, (sDistinct.isTnct==2 ? WHERE_DISTINCTBY : WHERE_GROUPBY)
          |  (ترتیب_By_Grp ? WHERE_SORTBYGROUP : 0) | distFlag, 0
      );
      إذا( pWInfo==0 ){
        sqlite3_بیان_List_Delete(db, pDistinct);
        اذهب select_پای_ټکی;
      }
      إذا( pParse->pIdxEpr ){
        optimize_Aggregate_Use_د_شاخصي_بیان(pParse, p, pAggInfo, &sNC);
      }
      ټاکل_Aggregate_Registers(pParse, pAggInfo);
      eDist = sqlite3WhereIsDistinct(pWInfo);
      TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
      إذا( sqlite3WhereIsOrdered(pWInfo)==p_ډله_By->n_بیان ){
        /* The optimizer is able to deliver rows in group by order so
        ** we do not have to sort.  The OP_OpenEphemeral table will be
        ** cancelled later because we still need to use the pKeyInfo
        */
        ډله_By_ترتیبول = 0;
      }وإلا{
        /* Rows are coming out in undetermined order.  We have to push
        ** each row into a sorting index, terminate the first loop,
        ** then loop over the sorting index in order to get the output
        ** in sorted order
        */
        صحيح reg_بنسټ;
        صحيح reg_ریکارډ;
        صحيح n_ستون;
        صحيح n_ډله_By;

#که_تعریف_شوی SQLITE_ENABLE_STMT_SCANSTATUS
        صحيح addrExp;              /* Address of OP_Explain instruction */
#د_که_پای
        ExplainQueryPlan2(addrExp, (pParse, 0, "USE TEMP B-TREE FOR %s",
            (sDistinct.isTnct && (p->selFlags&SF_Distinct)==0) ?
                    "DISTINCT" : "GROUP BY"
        ));

        ډله_By_ترتیبول = 1;
        n_ډله_By = p_ډله_By->n_بیان;
        n_ستون = n_ډله_By;
        j = n_ډله_By;
        لكل(i=0; i<pAggInfo->n_ستنه; i++){
          إذا( pAggInfo->a_ستون[i].i_Sorter_ستنه>=j ){
            n_ستون++;
            j++;
          }
        }
        reg_بنسټ = sqlite3_Get_لنډمهاله_Range(pParse, n_ستون);
        sqlite3_بیان_کوډ_بیان_List(pParse, p_ډله_By, reg_بنسټ, 0, 0);
        j = n_ډله_By;
        pAggInfo->مستقیم_ډېرپېښ = 1;
        لكل(i=0; i<pAggInfo->n_ستنه; i++){
          بنية Agg_Info_ستون *p_ستون = &pAggInfo->a_ستون[i];
          إذا( p_ستون->i_Sorter_ستنه>=j ){
            sqlite3_بیان_کوډ(pParse, p_ستون->pCExpr, j + reg_بنسټ);
            j++;
          }
        }
        pAggInfo->مستقیم_ډېرپېښ = 0;
        reg_ریکارډ = sqlite3_Get_لنډمهاله_Reg(pParse);
        sqlite3_Vdbe_لټون_حالت_Counters(v, addrExp, 0, sqlite3VdbeCurrentAddr(v));
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Make_ریکارډ, reg_بنسټ, n_ستون, reg_ریکارډ);
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Sorter_ورننباسه, pAggInfo->ترتیبونه_Idx, reg_ریکارډ);
        sqlite3_Vdbe_لټون_حالت_Range(v, addrExp, sqlite3VdbeCurrentAddr(v)-2, -1);
        sqlite3_Release_لنډمهاله_Reg(pParse, reg_ریکارډ);
        sqlite3_Release_لنډمهاله_Range(pParse, reg_بنسټ, n_ستون);
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_پای_ټکی(pWInfo);
        pAggInfo->ترتیبونه_Idx_PTab = ترتیبول_PTab = pParse->nTab++;
        ترتیبول_بهر = sqlite3_Get_لنډمهاله_Reg(pParse);
        sqlite3_Vdbe_لټون_حالت_Counters(v, addrExp, sqlite3VdbeCurrentAddr(v), 0);
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_OpenPseudo, ترتیبول_PTab, ترتیبول_بهر, n_ستون);
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Sorter_ترتیبول, pAggInfo->ترتیبونه_Idx, addr_پای_ټکی);
        VdbeComment((v, "GROUP BY sort")); VdbeCoverage(v);
        pAggInfo->use_ترتیبونه_Idx = 1;
        sqlite3_Vdbe_لټون_حالت_Range(v, addrExp, -1, ترتیبول_PTab);
        sqlite3_Vdbe_لټون_حالت_Range(v, addrExp, -1, pAggInfo->ترتیبونه_Idx);
      }

      /* If there are entries in pAgggInfo->aFunc[] that contain subexpressions
      ** that are indexed (and that were previously identified and tagged
      ** in optimizeAggregateUseOfIndexedExpr()) then those subexpressions
      ** must now be converted into a TK_AGG_COLUMN node so that the value
      ** is correctly pulled from the index rather than being recomputed. */
      إذا( pParse->pIdxEpr ){
        aggregate_Convert_شاخصي_بیان_Ref_منزل_ستنه(pAggInfo);
#إذا TREETRACE_ENABLED
        إذا( sqlite3_ونه_Trace & 0x20 ){
          TREETRACE(0x20, pParse, p,
             ("AggInfo function expressions converted to reference index\n"));
          sqlite3_ونه_View_Select(0, p, 0);
          printAggInfo(pAggInfo);
        }
#د_که_پای
      }

      /* If the index or temporary table used by the GROUP BY sort
      ** will naturally deliver rows in the order required by the ORDER BY
      ** clause, cancel the ephemeral table open coded earlier.
      **
      ** This is an optimization - the correct answer should result regardless.
      ** Use the SQLITE_GroupByOrder flag with SQLITE_TESTCTRL_OPTIMIZER to
      ** disable this optimization for testing purposes.  */
      إذا( ترتیب_By_Grp && OptimizationEnabled(db, SQLITE_ډله_By_ترتیب)
       && (ډله_By_ترتیبول || sqlite3WhereIsSorted(pWInfo))
      ){
        s_ترتیبول.p_ترتیب_By = 0;
        sqlite3_Vdbe_Change_منزل_Noop(v, s_ترتیبول.addr_ترتیبول_شاخص);
      }

      /* Evaluate the current GROUP BY terms and store in b0, b1, b2...
      ** (b0 is memory location iBMem+0, b1 is iBMem+1, and so forth)
      ** Then compare the current GROUP BY terms against the GROUP BY terms
      ** from the previous row currently stored in a0, a1, a2...
      */
      addr_سر_د_Loop = sqlite3VdbeCurrentAddr(v);
      إذا( ډله_By_ترتیبول ){
        sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Sorter_معلومات, pAggInfo->ترتیبونه_Idx,
                          ترتیبول_بهر, ترتیبول_PTab);
      }
      لكل(j=0; j<p_ډله_By->n_بیان; j++){
        صحيح i_ترتیب_By_ستون = p_ډله_By->a[j].u.x.i_ترتیب_By_ستون;
        
        إذا( ډله_By_ترتیبول ){
          sqlite3_Vdbe_جمع_کړه_Op3(v, OP_ستنه, ترتیبول_PTab, j, iBMem+j);
        }وإلا{
          pAggInfo->مستقیم_ډېرپېښ = 1;
          sqlite3_بیان_کوډ(pParse, p_ډله_By->a[j].p_بیان, iBMem+j);
        }

        إذا( i_ترتیب_By_ستون ){
          بیان *pX = p->pEList->a[i_ترتیب_By_ستون-1].p_بیان;
          بیان *p_بنسټ = sqlite3_بیان_Skip_Collate_And_Likely(pX);
          طالما( ALWAYS(p_بنسټ!=0) && p_بنسټ->op==TK_IF_NULL_لیکه ){
            pX = p_بنسټ->p_کیڼ;
            p_بنسټ = sqlite3_بیان_Skip_Collate_And_Likely(pX);
          }
          إذا( ALWAYS(p_بنسټ!=0)
           && p_بنسټ->op!=TK_AGG_ستنه
           && p_بنسټ->op!=TK_ثبتوونکی
          ){
            sqlite3_بیان_منزل_ثبتوونکی(pX, iAMem+j);
          }
        }
      }
      sqlite3_Vdbe_جمع_کړه_Op4(v, OP_Compare, iAMem, iBMem, p_ډله_By->n_بیان,
                          (حرف*)sqlite3_کلی_Info_Ref(p_کلی_Info), P4_KEYINFO);
      addr1 = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_جمع_کړه_Op3(v, OP_Jump, addr1+1, 0, addr1+1); VdbeCoverage(v);

      /* Generate code that runs whenever the GROUP BY changes.
      ** Changes in the GROUP BY are detected by the previous code
      ** block.  If there were no changes, this block is skipped.
      **
      ** This code copies current group by terms in b0,b1,b2,...
      ** over to a0,a1,a2.  It then calls the output subroutine
      ** and resets the aggregate accumulator registers in preparation
      ** for the next GROUP BY batch.
      */
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Gosub, reg_Output_لیکه, addr_Output_لیکه);
      VdbeComment((v, "output one row of %d", p->selId));
      sqlite3_بیان_کوډ_Move(pParse, iBMem, iAMem, p_ډله_By->n_بیان);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_IfPos, iAbortFlag, addr_پای_ټکی); VdbeCoverage(v);
      VdbeComment((v, "check abort flag"));
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Gosub, regReset, addrReset);
      VdbeComment((v, "reset accumulator %d", p->selId));

      /* Update the aggregate accumulators based on the content of
      ** the current row
      */
      sqlite3_Vdbe_Jump_دلته(v, addr1);
      update_راټولونکی(pParse, iUseFlag, pAggInfo, eDist);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, 1, iUseFlag);
      VdbeComment((v, "indicate data in accumulator %d", p->selId));

      /* End of the loop
      */
      إذا( ډله_By_ترتیبول ){
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Sorter_بل, pAggInfo->ترتیبونه_Idx,addr_سر_د_Loop);
        VdbeCoverage(v);
      }وإلا{
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_پای_ټکی(pWInfo);
        sqlite3_Vdbe_Change_منزل_Noop(v, addr_ترتیبونه_Idx);
      }
      sqlite3_بیان_List_Delete(db, pDistinct);

      /* Output the final row of result
      */
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Gosub, reg_Output_لیکه, addr_Output_لیکه);
      VdbeComment((v, "output final row of %d", p->selId));

      /* Jump over the subroutines
      */
      sqlite3VdbeGoto(v, addr_پای_ټکی);

      /* Generate a subroutine that outputs a single row of the result
      ** set.  This subroutine first looks at the iUseFlag.  If iUseFlag
      ** is less than or equal to zero, the subroutine is a no-op.  If
      ** the processing calls for the query to abort, this subroutine
      ** increments the iAbortFlag memory location before returning in
      ** order to signal the caller to abort.
      */
      addr_سېټ_Abort = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, 1, iAbortFlag);
      VdbeComment((v, "set abort flag"));
      sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Return, reg_Output_لیکه);
      sqlite3VdbeResolveLabel(v, addr_Output_لیکه);
      addr_Output_لیکه = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_IfPos, iUseFlag, addr_Output_لیکه+2);
      VdbeCoverage(v);
      VdbeComment((v, "Groupby result generator entry point %d", p->selId));
      sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Return, reg_Output_لیکه);
      finalizeAggFunctions(pParse, pAggInfo);
      sqlite3_بیان_If_False(pParse, pHaving, addr_Output_لیکه+1, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, &s_ترتیبول,
                      &sDistinct, pDest,
                      addr_Output_لیکه+1, addr_سېټ_Abort);
      sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Return, reg_Output_لیکه);
      VdbeComment((v, "end groupby result generator %d", p->selId));

      /* Generate a subroutine that will reset the group-by accumulator
      */
      sqlite3VdbeResolveLabel(v, addrReset);
      reset_راټولونکی(pParse, pAggInfo);
      sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, 0, iUseFlag);
      VdbeComment((v, "indicate accumulator %d empty", p->selId));
      sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Return, regReset);

      إذا( distFlag!=0 && eDist!=WHERE_DISTINCT_NOOP ){
        بنية AggInfo_func *pF = &pAggInfo->aFunc[0];
        fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
      }
    } /* endif pGroupBy.  Begin aggregate queries without GROUP BY: */
    وإلا {
      /* Aggregate functions without GROUP BY. tag-select-0820 */
      جدول *pTab;
      إذا( (pTab = is_Simple_شمېر(p, pAggInfo))!=0 ){
        /* tag-select-0821
        **
        ** If isSimpleCount() returns a pointer to a Table structure, then
        ** the SQL statement is of the form:
        **
        **   SELECT count(*) FROM <tbl>
        **
        ** where the Table structure returned represents table <tbl>.
        **
        ** This statement is so common that it is optimized specially. The
        ** OP_Count instruction is executed either on the intkey table that
        ** contains the data for table <tbl> or on one of its indexes. It
        ** is better to execute the op on an index, as indexes are almost
        ** always spread across less pages than their corresponding tables.
        */
        ثابت صحيح iDb = sqlite3_Schema_منزل_شاخص(pParse->db, pTab->pSchema);
        ثابت صحيح iCsr = pParse->nTab++;     /* Cursor to scan b-tree */
        شاخص *pIdx;                         /* Iterator variable */
        کلی_Info *p_کلی_Info = 0;               /* Keyinfo for scanned index */
        شاخص *p_غوره = 0;                    /* Best index found so far */
        Pgno i_ریښه = pTab->tnum;             /* Root page of scanned b-tree */

        sqlite3_کوډ_Verify_Schema(pParse, iDb);
        sqlite3_جدول_قلف(pParse, iDb, pTab->tnum, 0, pTab->zName);

        /* Search for the index that has the lowest scan cost.
        **
        ** (2011-04-15) Do not do a full scan of an unordered index.
        **
        ** (2013-10-03) Do not count the entries in a partial index.
        **
        ** In practice the KeyInfo structure will not be used. It is only
        ** passed to keep OP_OpenRead happy.
        */
        إذا( !HasRowid(pTab) ) p_غوره = sqlite3_Primary_کلی_شاخص(pTab);
        إذا( !p->pSrc->a[0].fg.not_شاخصي ){
          لكل(pIdx=pTab->p_شاخص; pIdx; pIdx=pIdx->p_بل){
            إذا( pIdx->bUnordered==0
             && pIdx->sz_Idx_لیکه<pTab->sz_Tab_لیکه
             && pIdx->pPartIdxWhere==0
             && (!p_غوره || pIdx->sz_Idx_لیکه<p_غوره->sz_Idx_لیکه)
            ){
              p_غوره = pIdx;
            }
          }
        }
        إذا( p_غوره ){
          i_ریښه = p_غوره->tnum;
          p_کلی_Info = sqlite3_کلی_Info_د_شاخص(pParse, p_غوره);
        }

        /* Open a read-only cursor, execute the OP_Count, close the cursor. */
        sqlite3_Vdbe_جمع_کړه_Op4_Int(v, OP_OpenRead, iCsr, (صحيح)i_ریښه, iDb, 1);
        إذا( p_کلی_Info ){
          sqlite3VdbeChangeP4(v, -1, (حرف *)p_کلی_Info, P4_KEYINFO);
        }
        ټاکل_Aggregate_Registers(pParse, pAggInfo);
        sqlite3_Vdbe_جمع_کړه_Op2(v, OP_شمېر, iCsr, AggInfoFuncReg(pAggInfo,0));
        sqlite3_Vdbe_جمع_کړه_Op1(v, OP_Close, iCsr);
        explain_Simple_شمېر(pParse, pTab, p_غوره);
      }وإلا{
        /* The general case of an aggregate query without GROUP BY
        ** tag-select-0822 */
        صحيح regAcc = 0;           /* "populate accumulators" flag */
        بیان_List *pDistinct = 0;
        u16 distFlag = 0;
        صحيح eDist;

        /* If there are accumulator registers but no min() or max() functions
        ** without FILTER clauses, allocate register regAcc. Register regAcc
        ** will contain 0 the first time the inner loop runs, and 1 thereafter.
        ** The code generated by updateAccumulator() uses this to ensure
        ** that the accumulator registers are (a) updated only once if
        ** there are no min() or max functions or (b) always updated for the
        ** first row visited by the aggregate, so that they are updated at
        ** least once even if the FILTER clause means the min() or max()
        ** function visits zero rows.  */
        إذا( pAggInfo->n_راټولونکی ){
          لكل(i=0; i<pAggInfo->nFunc; i++){
            إذا( بیان_Has_Property(pAggInfo->aFunc[i].pFExpr, EP_WinFunc) ){
              استمر;
            }
            إذا( pAggInfo->aFunc[i].pFunc->funcFlags&SQLITE_FUNC_NEEDCOLL ){
              اكسر;
            }
          }
          إذا( i==pAggInfo->nFunc ){
            regAcc = ++pParse->nMem;
            sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, 0, regAcc);
          }
        }وإلا إذا( pAggInfo->nFunc==1 && pAggInfo->aFunc[0].iDistinct>=0 ){
          أكد( بیان_Use_XList(pAggInfo->aFunc[0].pFExpr) );
          pDistinct = pAggInfo->aFunc[0].pFExpr->x.pList;
          distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
        }
        ټاکل_Aggregate_Registers(pParse, pAggInfo);

        /* This case runs if the aggregate has no GROUP BY clause.  The
        ** processing is much simpler since there is only a single row
        ** of output.
        */
        أكد( p->p_ډله_By==0 );
        reset_راټولونکی(pParse, pAggInfo);

        /* If this query is a candidate for the min/max optimization, then
        ** minMaxFlag will have been previously set to either
        ** WHERE_ORDERBY_MIN or WHERE_ORDERBY_MAX and pMinMaxOrderBy will
        ** be an appropriate ORDER BY expression for the optimization.
        */
        أكد( minMaxFlag==WHERE_ORDERBY_نورمال || p_Min_Max_ترتیب_By!=0 );
        أكد( p_Min_Max_ترتیب_By==0 || p_Min_Max_ترتیب_By->n_بیان==1 );

        TREETRACE(0x2,pParse,p,("WhereBegin\n"));
        pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, p_Min_Max_ترتیب_By,
                                   pDistinct, p, minMaxFlag|distFlag, 0);
        إذا( pWInfo==0 ){
          اذهب select_پای_ټکی;
        }
        TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
        eDist = sqlite3WhereIsDistinct(pWInfo);
        update_راټولونکی(pParse, regAcc, pAggInfo, eDist);
        إذا( eDist!=WHERE_DISTINCT_NOOP ){
          بنية AggInfo_func *pF = pAggInfo->aFunc;
          إذا( pF ){
            fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
          }
        }

        إذا( regAcc ) sqlite3_Vdbe_جمع_کړه_Op2(v, OP_Integer, 1, regAcc);
        إذا( minMaxFlag ){
          sqlite3_Where_Min_Max_Opt_Early_بهر(v, pWInfo);
        }
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_پای_ټکی(pWInfo);
        finalizeAggFunctions(pParse, pAggInfo);
      }

      s_ترتیبول.p_ترتیب_By = 0;
      sqlite3_بیان_If_False(pParse, pHaving, addr_پای_ټکی, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, 0, 0,
                      pDest, addr_پای_ټکی, addr_پای_ټکی);
    }
    sqlite3VdbeResolveLabel(v, addr_پای_ټکی);

  } /* endif aggregate query */

  إذا( sDistinct.eTnctType==WHERE_DISTINCT_UNORDERED ){
    explain_لنډمهاله_جدول(pParse, "DISTINCT");
  }

  /* If there is an ORDER BY clause, then we need to sort the results
  ** and send them to the callback one by one.  tag-select-0900
  */
  إذا( s_ترتیبول.p_ترتیب_By ){
    أكد( p->pEList==pEList );
    generate_ترتیبول_لکۍ(pParse, p, &s_ترتیبول, pEList->n_بیان, pDest);
  }

  /* Jump here to skip this query
  */
  sqlite3VdbeResolveLabel(v, i_پای_ټکی);

  /* The SELECT has been coded. If there is an error in the Parse structure,
  ** set the return code to 1. Otherwise 0. */
  rc = (pParse->nErr>0);

  /* Control jumps to here if an error is encountered above, or upon
  ** successful coding of the SELECT.
  */
select_پای_ټکی:
  أكد( db->mallocFailed==0 || db->mallocFailed==1 );
  أكد( db->mallocFailed==0 || pParse->nErr!=0 );
  sqlite3_بیان_List_Delete(db, p_Min_Max_ترتیب_By);
#که_تعریف_شوی SQLITE_DEBUG
  /* Internal self-checks.  tag-select-1000 */
  إذا( pAggInfo && !db->mallocFailed ){
#إذا TREETRACE_ENABLED
    إذا( sqlite3_ونه_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("Finished with AggInfo\n"));
      printAggInfo(pAggInfo);
    }
#د_که_پای
    لكل(i=0; i<pAggInfo->n_ستنه; i++){
      بیان *p_بیان = pAggInfo->a_ستون[i].pCExpr;
      إذا( p_بیان==0 ) استمر;
      أكد( p_بیان->pAggInfo==pAggInfo );
      أكد( p_بیان->iAgg==i );
    }
    لكل(i=0; i<pAggInfo->nFunc; i++){
      بیان *p_بیان = pAggInfo->aFunc[i].pFExpr;
      أكد( p_بیان!=0 );
      أكد( p_بیان->pAggInfo==pAggInfo );
      أكد( p_بیان->iAgg==i );
    }
  }
#د_که_پای

#إذا TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p,("end processing\n"));
  إذا( (sqlite3_ونه_Trace & 0x40000)!=0 && Explain_Query_Plan_پلار(pParse)==0 ){
    sqlite3_ونه_View_Select(0, p, 0);
  }
#د_که_پای
  Explain_Query_Plan_راوباسه(pParse);
  أرجع rc;
}
