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
تعريفنوع بنية ترتيب_ڏيو_Ctx ترتيب_ڏيو_Ctx;
بنية ترتيب_ڏيو_Ctx {
  اظهار_List *p_ترتيب_By;   /* The ORDER BY (or GROUP BY clause) */
  صحيح nOBSat;           /* Number of ORDER BY terms satisfied by indices */
  صحيح iECursor;         /* Cursor number for the sorter */
  صحيح regReturn;        /* Register holding block-output return address */
  صحيح label_Bk_ٻاهريون;       /* Start label for the block-output subroutine */
  صحيح addr_ترتيب_ڏيو_اشاريو;    /* Address of the OP_SorterOpen or OP_OpenEphemeral */
  صحيح label_پورو;        /* Jump here when done, ex: LIMIT reached */
  صحيح labelOBLopt;      /* Jump here when sorter is full */
  u8 ترتيب_ڏيو_Flags;         /* Zero or more SORTFLAG_* bits */
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SORTER_REFERENCES
  u8 nDefer;            /* Number of valid entries in aDefer[] */
  بنية DeferredCsr {
    جدول *pTab;        /* Table definition */
    صحيح iCsr;           /* Cursor number for table */
    صحيح n_ڪنجي;           /* Number of PK columns for table pTab (>=1) */
  } aDefer[4];
#جيڪڏهن_پورو
  بنية صف_ڀريو_Info *p_Deferred_صف_ڀريو;  /* Deferred row loading info or NULL */
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STMT_SCANSTATUS
  صحيح addr_ڌڪو;         /* First instruction to push data into sorter */
  صحيح addr_ڌڪو_پڇاڙي;      /* Last instruction that pushes data into sorter */
#جيڪڏهن_پورو
};
#وصف_ڪريو SORTFLAG_UseSorter  0x01   /* Use SorterOpen instead of OpenEphemeral */

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
    Select *p_اڳواٽ = p->p_اڳواٽ;
    sqlite3_اظهار_List_Delete(db, p->pEList);
    sqlite3SrcListDelete(db, p->pSrc);
    sqlite3_اظهار_Delete(db, p->pWhere);
    sqlite3_اظهار_List_Delete(db, p->p_گروهه_By);
    sqlite3_اظهار_Delete(db, p->pHaving);
    sqlite3_اظهار_List_Delete(db, p->p_ترتيب_By);
    sqlite3_اظهار_Delete(db, p->p_حد_بندي);
    إذا( OK_IF_ALWAYS_TRUE(p->pWith) ) sqlite3WithDelete(db, p->pWith);
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
    إذا( OK_IF_ALWAYS_TRUE(p->pWinDefn) ){
      sqlite3_دري_List_Delete(db, p->pWinDefn);
    }
    طالما( p->pWin ){
      أكد( p->pWin->ppThis==&p->pWin );
      sqlite3_دري_Unlink_From_Select(p->pWin);
    }
#جيڪڏهن_پورو
    إذا( bFree ) sqlite3DbNNFreeNN(db, p);
    p = p_اڳواٽ;
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
  اظهار_List *pEList,     /* which columns to include in the result */
  SrcList *pSrc,        /* the FROM clause -- which tables to scan */
  اظهار *pWhere,         /* the WHERE clause */
  اظهار_List *p_گروهه_By,   /* the GROUP BY clause */
  اظهار *pHaving,        /* the HAVING clause */
  اظهار_List *p_ترتيب_By,   /* the ORDER BY clause */
  u32 selFlags,         /* Flag parameters, such as SF_Distinct */
  اظهار *p_حد_بندي          /* LIMIT value.  NULL means not used */
){
  Select *pNew, *pAllocated;
  Select standin;
  pAllocated = pNew = sqlite3DbMallocRawNN(pParse->db, حجم(*pNew) );
  إذا( pNew==0 ){
    أكد( pParse->db->mallocFailed );
    pNew = &standin;
  }
  إذا( pEList==0 ){
    pEList = sqlite3_اظهار_List_Append(pParse, 0,
                                   sqlite3_اظهار(pParse->db,TK_ASTERISK,0));
  }
  pNew->pEList = pEList;
  pNew->op = TK_SELECT;
  pNew->selFlags = selFlags;
  pNew->i_حد_بندي = 0;
  pNew->i_وٿي = 0;
  pNew->selId = ++pParse->nSelect;
  pNew->n_Select_صف = 0;
  إذا( pSrc==0 ) pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  pNew->pSrc = pSrc;
  pNew->pWhere = pWhere;
  pNew->p_گروهه_By = p_گروهه_By;
  pNew->pHaving = pHaving;
  pNew->p_ترتيب_By = p_ترتيب_By;
  pNew->p_اڳواٽ = 0;
  pNew->p_ايندڙ = 0;
  pNew->p_حد_بندي = p_حد_بندي;
  pNew->pWith = 0;
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
  pNew->pWin = 0;
  pNew->pWinDefn = 0;
#جيڪڏهن_پورو
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
ساكن Select *ڳوليو_Rightmost(Select *p){
  طالما( p->p_ايندڙ ) p = p->p_ايندڙ;
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
  ساكن ثابت حرف z_ڪنجي_متن[] = "naturaleftouterightfullinnercross";
  ساكن ثابت بنية {
    u8 i;        /* Beginning of keyword text in zKeyText[] */
    u8 nChar;    /* Length of the keyword in characters */
    u8 ڪوڊ;     /* Join type mask */
  } aKeyword[] = {
    /* (0) natural */ { 0,  7, JT_NATURAL                },
    /* (1) left    */ { 6,  4, JT_کاٻو|JT_OUTER          },
    /* (2) outer   */ { 10, 5, JT_OUTER                  },
    /* (3) right   */ { 14, 5, JT_ساڄو|JT_OUTER         },
    /* (4) full    */ { 19, 4, JT_کاٻو|JT_ساڄو|JT_OUTER },
    /* (5) inner   */ { 23, 5, JT_INNER                  },
    /* (6) cross   */ { 28, 5, JT_INNER|JT_CROSS         },
  };
  صحيح i, j;
  apAll[0] = pA;
  apAll[1] = pB;
  apAll[2] = pC;
  لكل(i=0; i<3 && apAll[i]; i++){
    p = apAll[i];
    لكل(j=0; j<Array_سائيز(aKeyword); j++){
      إذا( p->n==aKeyword[j].nChar
          && sqlite3StrNICmp((حرف*)p->z, &z_ڪنجي_متن[aKeyword[j].i], p->n)==0 ){
        jointype |= aKeyword[j].ڪوڊ;
        اكسر;
      }
    }
    testcase( j==0 || j==1 || j==2 || j==3 || j==4 || j==5 || j==6 );
    إذا( j>=Array_سائيز(aKeyword) ){
      jointype |= JT_ERROR;
      اكسر;
    }
  }
  إذا(
     (jointype & (JT_INNER|JT_OUTER))==(JT_INNER|JT_OUTER) ||
     (jointype & JT_ERROR)!=0 ||
     (jointype & (JT_OUTER|JT_کاٻو|JT_ساڄو))==JT_OUTER
  ){
    ثابت حرف *zSp1 = " ";
    ثابت حرف *zSp2 = " ";
    إذا( pB==0 ){ zSp1++; }
    إذا( pC==0 ){ zSp2++; }
    sqlite3_Error_نياپو(pParse, "unknown join type: "
       "%T%s%T%s%T", pA, zSp1, pB, zSp2, pC);
    jointype = JT_INNER;
  }
  أرجع jointype;
}

/*
** Return the index of a column in a table.  Return -1 if the column
** is not contained in the table.
*/
صحيح sqlite3_ڪالم_اشاريو(جدول *pTab, ثابت حرف *z_کانو){
  صحيح i;
  u8 h;
  ثابت ڪالم *a_کانو;
  صحيح n_کانو;

  h = sqlite3StrIHash(z_کانو);
  a_کانو = pTab->a_کانو;
  n_کانو = pTab->n_کانو;

  /* See if the aHx gives us a lucky match */
  i = pTab->aHx[h % حجم(pTab->aHx)];
  أكد( i<n_کانو );
  إذا( a_کانو[i].hName==h
   && sqlite3StrICmp(a_کانو[i].zCnName, z_کانو)==0
  ){
    أرجع i;
  }

  /* No lucky match from the hash table.  Do a full search. */
  i = 0;
  طالما( 1 /*exit-by-break*/ ){
    إذا( a_کانو[i].hName==h
     && sqlite3StrICmp(a_کانو[i].zCnName, z_کانو)==0
    ){
      أرجع i;
    }
    i++;
    إذا( i>=n_کانو ) اكسر;
  }
  أرجع -1;
}

/*
** Mark a subquery result column as having been used.
*/
فراغ sqlite3_Src_شي_ڪالم_استعمال_ٿيل(Src_شي *p_شي, صحيح i_کانو){
  أكد( p_شي!=0 );
  أكد( (صحيح)p_شي->fg.isNestedFrom == IsNestedFrom(p_شي) );
  إذا( p_شي->fg.isNestedFrom ){
    اظهار_List *pResults;
    أكد( p_شي->fg.isSubquery );
    أكد( p_شي->u4.pSubq!=0 );
    أكد( p_شي->u4.pSubq->pSelect!=0 );
    pResults = p_شي->u4.pSubq->pSelect->pEList;
    أكد( pResults!=0 );
    أكد( i_کانو>=0 && i_کانو<pResults->n_اظهار );
    pResults->a[i_کانو].fg.b_استعمال_ٿيل = 1;
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
ساكن صحيح جدول_And_ڪالم_اشاريو(
  SrcList *pSrc,       /* Array of tables to search */
  صحيح i_شروعات,          /* First member of pSrc->a[] to check */
  صحيح i_پڇاڙي,            /* Last member of pSrc->a[] to check */
  ثابت حرف *z_کانو,    /* Name of the column we are looking for */
  صحيح *piTab,          /* Write index of pSrc->a[] here */
  صحيح *pi_کانو,          /* Write index of pSrc->a[*piTab].pSTab->aCol[] here */
  صحيح bIgnoreHidden    /* Ignore hidden columns */
){
  صحيح i;               /* For looping over tables in pSrc */
  صحيح i_کانو;            /* Index of column matching zCol */

  أكد( i_پڇاڙي<pSrc->nSrc );
  أكد( i_شروعات>=0 );
  أكد( (piTab==0)==(pi_کانو==0) );  /* Both or neither are NULL */

  لكل(i=i_شروعات; i<=i_پڇاڙي; i++){
    i_کانو = sqlite3_ڪالم_اشاريو(pSrc->a[i].pSTab, z_کانو);
    إذا( i_کانو>=0
     && (bIgnoreHidden==0 || Is_Hidden_ڪالم(&pSrc->a[i].pSTab->a_کانو[i_کانو])==0)
    ){
      إذا( piTab ){
        sqlite3_Src_شي_ڪالم_استعمال_ٿيل(&pSrc->a[i], i_کانو);
        *piTab = i;
        *pi_کانو = i_کانو;
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
فراغ sqlite3_سيٽ_Join_اظهار(اظهار *p, صحيح i_جدول, u32 joinFlag){
  أكد( joinFlag==EP_OuterON || joinFlag==EP_InnerON );
  طالما( p ){
    اظهار_سيٽ_Property(p, joinFlag);
    أكد( !اظهار_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    اظهار_سيٽ_VVAProperty(p, EP_NoReduce);
    p->w.iJoin = i_جدول;
    إذا( اظهار_Use_XList(p) ){
      إذا( p->x.pList ){
        صحيح i;
        لكل(i=0; i<p->x.pList->n_اظهار; i++){
          sqlite3_سيٽ_Join_اظهار(p->x.pList->a[i].p_اظهار, i_جدول, joinFlag);
        }
      }
    }
    sqlite3_سيٽ_Join_اظهار(p->p_کاٻو, i_جدول, joinFlag);
    p = p->p_ساڄو;
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
ساكن فراغ unset_Join_اظهار(اظهار *p, صحيح i_جدول, صحيح nullable){
  طالما( p ){
    إذا( i_جدول<0 || (اظهار_Has_Property(p, EP_OuterON) && p->w.iJoin==i_جدول) ){
      اظهار_Clear_Property(p, EP_OuterON|EP_InnerON);
      إذا( i_جدول>=0 ) اظهار_سيٽ_Property(p, EP_InnerON);
    }
    إذا( p->op==TK_ڪالم && p->i_جدول==i_جدول && !nullable ){
      اظهار_Clear_Property(p, EP_CanBeNull);
    }
    إذا( p->op==TK_FUNCTION ){
      أكد( اظهار_Use_XList(p) );
      أكد( p->p_کاٻو==0 );
      إذا( p->x.pList ){
        صحيح i;
        لكل(i=0; i<p->x.pList->n_اظهار; i++){
          unset_Join_اظهار(p->x.pList->a[i].p_اظهار, i_جدول, nullable);
        }
      }
    }
    unset_Join_اظهار(p->p_کاٻو, i_جدول, nullable);
    p = p->p_ساڄو;
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
  Src_شي *p_کاٻو;                 /* Left table being joined */
  Src_شي *p_ساڄو;                /* Right table being joined */
  صحيح b_ڏٺل_TFunc = 0;             /* True if have seen table-valued function */

  pSrc = p->pSrc;
  p_کاٻو = &pSrc->a[0];
  p_ساڄو = &p_کاٻو[1];
  لكل(i=0; i<pSrc->nSrc-1; i++, p_ساڄو++, p_کاٻو++){
    جدول *p_ساڄو_Tab = p_ساڄو->pSTab;
    u32 joinType;

    إذا( NEVER(p_کاٻو->pSTab==0 || p_ساڄو_Tab==0) ) استمر;
    joinType = (p_ساڄو->fg.jointype & JT_OUTER)!=0 ? EP_OuterON : EP_InnerON;

    /* If this is a NATURAL join, synthesize an appropriate USING clause
    ** to specify which columns should be joined.
    */
    إذا( p_ساڄو->fg.jointype & JT_NATURAL ){
      IdList *pUsing = 0;
      إذا( p_ساڄو->fg.isUsing || p_ساڄو->u3.pOn ){
        sqlite3_Error_نياپو(pParse, "a NATURAL join may not have "
           "an ON or USING clause", 0);
        أرجع 1;
      }
      لكل(j=0; j<p_ساڄو_Tab->n_کانو; j++){
        حرف *zName;   /* Name of column in the right table */

        إذا( Is_Hidden_ڪالم(&p_ساڄو_Tab->a_کانو[j]) ) استمر;
        zName = p_ساڄو_Tab->a_کانو[j].zCnName;
        إذا( جدول_And_ڪالم_اشاريو(pSrc, 0, i, zName, 0, 0, 1) ){
          pUsing = sqlite3IdListAppend(pParse, pUsing, 0);
          إذا( pUsing ){
            أكد( pUsing->nId>0 );
            أكد( pUsing->a[pUsing->nId-1].zName==0 );
            pUsing->a[pUsing->nId-1].zName = sqlite3DbStrDup(pParse->db, zName);
          }
        }
      }
      إذا( pUsing ){
        p_ساڄو->fg.isUsing = 1;
        p_ساڄو->fg.isSynthUsing = 1;
        p_ساڄو->u3.pUsing = pUsing;
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
    إذا( p_ساڄو->fg.isUsing ){
      IdList *pList = p_ساڄو->u3.pUsing;
      sqlite3 *db = pParse->db;
      أكد( pList!=0 );
      لكل(j=0; j<pList->nId; j++){
        حرف *zName;     /* Name of the term in the USING clause */
        صحيح i_کاٻو;       /* Table on the left with matching column name */
        صحيح i_کاٻو_کانو;    /* Column number of matching column on the left */
        صحيح i_ساڄو_کانو;   /* Column number of matching column on the right */
        اظهار *pE1;       /* Reference to the column on the LEFT of the join */
        اظهار *pE2;       /* Reference to the column on the RIGHT of the join */
        اظهار *pEq;       /* Equality constraint.  pE1 == pE2 */

        zName = pList->a[j].zName;
        i_ساڄو_کانو = sqlite3_ڪالم_اشاريو(p_ساڄو_Tab, zName);
        إذا( i_ساڄو_کانو<0
         || جدول_And_ڪالم_اشاريو(pSrc, 0, i, zName, &i_کاٻو, &i_کاٻو_کانو,
                                p_ساڄو->fg.isSynthUsing)==0
        ){
          sqlite3_Error_نياپو(pParse, "cannot join using column %s - column "
            "not present in both tables", zName);
          أرجع 1;
        }
        pE1 = sqlite3_Create_ڪالم_اظهار(db, pSrc, i_کاٻو, i_کاٻو_کانو);
        sqlite3_Src_شي_ڪالم_استعمال_ٿيل(&pSrc->a[i_کاٻو], i_کاٻو_کانو);
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
          اظهار_List *p_Func_دليلون = 0;   /* Arguments to the coalesce() */
          ساكن ثابت Token tkCoalesce = { "coalesce", 8 };
          أكد( pE1!=0 );
          اظهار_سيٽ_Property(pE1, EP_CanBeNull);
          طالما( جدول_And_ڪالم_اشاريو(pSrc, i_کاٻو+1, i, zName, &i_کاٻو, &i_کاٻو_کانو,
                                     p_ساڄو->fg.isSynthUsing)!=0 ){
            إذا( pSrc->a[i_کاٻو].fg.isUsing==0
             || sqlite3_Id_List_اشاريو(pSrc->a[i_کاٻو].u3.pUsing, zName)<0
            ){
              sqlite3_Error_نياپو(pParse, "ambiguous reference to %s in USING()",
                              zName);
              اكسر;
            }
            p_Func_دليلون = sqlite3_اظهار_List_Append(pParse, p_Func_دليلون, pE1);
            pE1 = sqlite3_Create_ڪالم_اظهار(db, pSrc, i_کاٻو, i_کاٻو_کانو);
            sqlite3_Src_شي_ڪالم_استعمال_ٿيل(&pSrc->a[i_کاٻو], i_کاٻو_کانو);
          }
          إذا( p_Func_دليلون ){
            p_Func_دليلون = sqlite3_اظهار_List_Append(pParse, p_Func_دليلون, pE1);
            pE1 = sqlite3_اظهار_Function(pParse, p_Func_دليلون, &tkCoalesce, 0);
            إذا( pE1 ){
              pE1->aff_اظهار = SQLITE_AFF_DEFER;
            }
          }
        }وإلا إذا( (pSrc->a[i+1].fg.jointype & JT_کاٻو)!=0 && pParse->nErr==0 ){
          أكد( pE1!=0 );
          اظهار_سيٽ_Property(pE1, EP_CanBeNull);
        }
        pE2 = sqlite3_Create_ڪالم_اظهار(db, pSrc, i+1, i_ساڄو_کانو);
        sqlite3_Src_شي_ڪالم_استعمال_ٿيل(p_ساڄو, i_ساڄو_کانو);
        pEq = sqlite3PExpr(pParse, TK_EQ, pE1, pE2);
        أكد( pE2!=0 || pEq==0 );
        إذا( pEq ){
          اظهار_سيٽ_Property(pEq, joinType);
          أكد( !اظهار_Has_Property(pEq, EP_TokenOnly|EP_Reduced) );
          اظهار_سيٽ_VVAProperty(pEq, EP_NoReduce);
          pEq->w.iJoin = pE2->i_جدول;
        }
        p->pWhere = sqlite3_اظهار_And(pParse, p->pWhere, pEq);
      }
    }

    /* Add the ON clause to the end of the WHERE clause, connected by
    ** an AND operator.
    */
    وإلا إذا( p_ساڄو->u3.pOn ){
      sqlite3_سيٽ_Join_اظهار(p_ساڄو->u3.pOn, p_ساڄو->iCursor, joinType);
      p->pWhere = sqlite3_اظهار_And(pParse, p->pWhere, p_ساڄو->u3.pOn);
      p_ساڄو->u3.pOn = 0;
      p_ساڄو->fg.isOn = 1;
      p->selFlags |= SF_On_منزل_Where;
    }

    إذا( b_ڏٺل_TFunc==0 ){
      b_ڏٺل_TFunc = (p_کاٻو->fg.isTabFunc && p_کاٻو->u1.pFuncArg);
    }

    /* We also need to call sqlite3SelectCheckOnClauses() to verify that
    ** table-valued function arguments do not illegally refer to any tables to
    ** their right. This test is required if either (a) there is a RIGHT JOIN
    ** in the SrcList, or (b) there is a LEFT JOIN to the right of a 
    ** table-valued function.  */
    إذا( (p_ساڄو->fg.isTabFunc && joinType==EP_OuterON && p_ساڄو->u1.pFuncArg)
     || (b_ڏٺل_TFunc && joinType==EP_OuterON)
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
تعريفنوع بنية صف_ڀريو_Info صف_ڀريو_Info;
بنية صف_ڀريو_Info {
  صحيح reg_نتيجو;               /* Store results in array of registers here */
  u8 ecelFlags;                /* Flag argument to ExprCodeExprList() */
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SORTER_REFERENCES
  اظهار_List *pExtra;            /* Extra columns needed by sorter refs */
  صحيح reg_Extra_نتيجو;          /* Where to load the extra columns */
#جيڪڏهن_پورو
};

/*
** This routine does the work of loading query data into an array of
** registers so that it can be added to the sorter.
*/
ساكن فراغ inner_Loop_ڀريو_صف(
  Parse *pParse,             /* Statement under construction */
  Select *pSelect,           /* The query being coded */
  صف_ڀريو_Info *pInfo         /* Info needed to complete the row load */
){
  sqlite3_اظهار_ڪوڊ_اظهار_List(pParse, pSelect->pEList, pInfo->reg_نتيجو,
                          0, pInfo->ecelFlags);
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SORTER_REFERENCES
  إذا( pInfo->pExtra ){
    sqlite3_اظهار_ڪوڊ_اظهار_List(pParse, pInfo->pExtra, pInfo->reg_Extra_نتيجو, 0, 0);
    sqlite3_اظهار_List_Delete(pParse->db, pInfo->pExtra);
  }
#جيڪڏهن_پورو
}

/*
** Code the OP_MakeRecord instruction that generates the entry to be
** added into the sorter.
**
** Return the register in which the result is stored.
*/
ساكن صحيح make_Sorter_رڪارڊ(
  Parse *pParse,
  ترتيب_ڏيو_Ctx *p_ترتيب_ڏيو,
  Select *pSelect,
  صحيح reg_بنياد,
  صحيح n_بنياد
){
  صحيح nOBSat = p_ترتيب_ڏيو->nOBSat;
  Vdbe *v = pParse->pVdbe;
  صحيح reg_ٻاهريون = ++pParse->nMem;
  إذا( p_ترتيب_ڏيو->p_Deferred_صف_ڀريو ){
    inner_Loop_ڀريو_صف(pParse, pSelect, p_ترتيب_ڏيو->p_Deferred_صف_ڀريو);
  }
  sqlite3_Vdbe_جوڙيو_Op3(v, OP_Make_رڪارڊ, reg_بنياد+nOBSat, n_بنياد-nOBSat, reg_ٻاهريون);
  أرجع reg_ٻاهريون;
}

/*
** Generate code that will push the record in registers regData
** through regData+nData-1 onto the sorter.
*/
ساكن فراغ ڌڪو_Onto_Sorter(
  Parse *pParse,         /* Parser context */
  ترتيب_ڏيو_Ctx *p_ترتيب_ڏيو,        /* Information about the ORDER BY clause */
  Select *pSelect,       /* The whole SELECT statement */
  صحيح reg_معلومات,           /* First register holding data to be sorted */
  صحيح reg_Orig_معلومات,       /* First register holding data before packing */
  صحيح n_معلومات,             /* Number of elements in the regData data array */
  صحيح n_اڳياڙي_Reg         /* No. of reg prior to regData available for use */
){
  Vdbe *v = pParse->pVdbe;                         /* Stmt under construction */
  صحيح bSeq = ((p_ترتيب_ڏيو->ترتيب_ڏيو_Flags & SORTFLAG_UseSorter)==0);
  صحيح n_اظهار = p_ترتيب_ڏيو->p_ترتيب_By->n_اظهار;              /* No. of ORDER BY terms */
  صحيح n_بنياد = n_اظهار + bSeq + n_معلومات;                /* Fields in sorter record */
  صحيح reg_بنياد;                                     /* Regs for sorter record */
  صحيح reg_رڪارڊ = 0;                               /* Assembled sorter record */
  صحيح nOBSat = p_ترتيب_ڏيو->nOBSat;                      /* ORDER BY terms to skip */
  صحيح op;                            /* Opcode to add sorter record to sorter */
  صحيح i_حد_بندي;                        /* LIMIT counter */
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

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STMT_SCANSTATUS
  p_ترتيب_ڏيو->addr_ڌڪو = sqlite3VdbeCurrentAddr(v);
#جيڪڏهن_پورو

  إذا( n_اڳياڙي_Reg ){
    أكد( n_اڳياڙي_Reg==n_اظهار+bSeq );
    reg_بنياد = reg_معلومات - n_اڳياڙي_Reg;
  }وإلا{
    reg_بنياد = pParse->nMem + 1;
    pParse->nMem += n_بنياد;
  }
  أكد( pSelect->i_وٿي==0 || pSelect->i_حد_بندي!=0 );
  i_حد_بندي = pSelect->i_وٿي ? pSelect->i_وٿي+1 : pSelect->i_حد_بندي;
  p_ترتيب_ڏيو->label_پورو = sqlite3VdbeMakeLabel(pParse);
  sqlite3_اظهار_ڪوڊ_اظهار_List(pParse, p_ترتيب_ڏيو->p_ترتيب_By, reg_بنياد, reg_Orig_معلومات,
                          SQLITE_ECEL_DUP | (reg_Orig_معلومات? SQLITE_ECEL_REF : 0));
  إذا( bSeq ){
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_Sequence, p_ترتيب_ڏيو->iECursor, reg_بنياد+n_اظهار);
  }
  إذا( n_اڳياڙي_Reg==0 && n_معلومات>0 ){
    sqlite3_اظهار_ڪوڊ_Move(pParse, reg_معلومات, reg_بنياد+n_اظهار+bSeq, n_معلومات);
  }
  إذا( nOBSat>0 ){
    صحيح reg_Prev_ڪنجي;   /* The first nOBSat columns of the previous row */
    صحيح addr_پهريون;    /* Address of the OP_IfNot opcode */
    صحيح addrJmp;      /* Address of the OP_Jump opcode */
    VdbeOp *pOp;      /* Opcode that opens the sorter */
    صحيح n_ڪنجي;         /* Number of sorting key columns, including OP_Sequence */
    ڪنجي_Info *pKI;     /* Original KeyInfo on the sorter table */

    reg_رڪارڊ = make_Sorter_رڪارڊ(pParse, p_ترتيب_ڏيو, pSelect, reg_بنياد, n_بنياد);
    reg_Prev_ڪنجي = pParse->nMem+1;
    pParse->nMem += p_ترتيب_ڏيو->nOBSat;
    n_ڪنجي = n_اظهار - p_ترتيب_ڏيو->nOBSat + bSeq;
    إذا( bSeq ){
      addr_پهريون = sqlite3_Vdbe_جوڙيو_Op1(v, OP_IfNot, reg_بنياد+n_اظهار);
    }وإلا{
      addr_پهريون = sqlite3_Vdbe_جوڙيو_Op1(v, OP_SequenceTest, p_ترتيب_ڏيو->iECursor);
    }
    VdbeCoverage(v);
    sqlite3_Vdbe_جوڙيو_Op3(v, OP_Compare, reg_Prev_ڪنجي, reg_بنياد, p_ترتيب_ڏيو->nOBSat);
    pOp = sqlite3VdbeGetOp(v, p_ترتيب_ڏيو->addr_ترتيب_ڏيو_اشاريو);
    إذا( pParse->db->mallocFailed ) أرجع;
    pOp->p2 = n_ڪنجي + n_معلومات;
    pKI = pOp->p4.p_ڪنجي_Info;
    memset(pKI->a_ترتيب_ڏيو_Flags, 0, pKI->n_ڪنجي_ميدان); /* Makes OP_Jump testable */
    sqlite3VdbeChangeP4(v, -1, (حرف*)pKI, P4_KEYINFO);
    testcase( pKI->n_All_ميدان > pKI->n_ڪنجي_ميدان+2 );
    pOp->p4.p_ڪنجي_Info = sqlite3_ڪنجي_Info_From_اظهار_List(pParse,p_ترتيب_ڏيو->p_ترتيب_By,nOBSat,
                                           pKI->n_All_ميدان-pKI->n_ڪنجي_ميدان-1);
    pOp = 0; /* Ensure pOp not used after sqlite3VdbeAddOp3() */
    addrJmp = sqlite3VdbeCurrentAddr(v);
    sqlite3_Vdbe_جوڙيو_Op3(v, OP_Jump, addrJmp+1, 0, addrJmp+1); VdbeCoverage(v);
    p_ترتيب_ڏيو->label_Bk_ٻاهريون = sqlite3VdbeMakeLabel(pParse);
    p_ترتيب_ڏيو->regReturn = ++pParse->nMem;
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_Gosub, p_ترتيب_ڏيو->regReturn, p_ترتيب_ڏيو->label_Bk_ٻاهريون);
    sqlite3_Vdbe_جوڙيو_Op1(v, OP_ResetSorter, p_ترتيب_ڏيو->iECursor);
    إذا( i_حد_بندي ){
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_IfNot, i_حد_بندي, p_ترتيب_ڏيو->label_پورو);
      VdbeCoverage(v);
    }
    sqlite3_Vdbe_Jump_هتي(v, addr_پهريون);
    sqlite3_اظهار_ڪوڊ_Move(pParse, reg_بنياد, reg_Prev_ڪنجي, p_ترتيب_ڏيو->nOBSat);
    sqlite3_Vdbe_Jump_هتي(v, addrJmp);
  }
  إذا( i_حد_بندي ){
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
    صحيح iCsr = p_ترتيب_ڏيو->iECursor;
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_IfNotZero, i_حد_بندي, sqlite3VdbeCurrentAddr(v)+4);
    VdbeCoverage(v);
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_آخري, iCsr, 0);
    iSkip = sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_IdxLE,
                                 iCsr, 0, reg_بنياد+nOBSat, n_اظهار-nOBSat);
    VdbeCoverage(v);
    sqlite3_Vdbe_جوڙيو_Op1(v, OP_Delete, iCsr);
  }
  إذا( reg_رڪارڊ==0 ){
    reg_رڪارڊ = make_Sorter_رڪارڊ(pParse, p_ترتيب_ڏيو, pSelect, reg_بنياد, n_بنياد);
  }
  إذا( p_ترتيب_ڏيو->ترتيب_ڏيو_Flags & SORTFLAG_UseSorter ){
    op = OP_Sorter_وجهو;
  }وإلا{
    op = OP_Idx_وجهو;
  }
  sqlite3_Vdbe_جوڙيو_Op4_Int(v, op, p_ترتيب_ڏيو->iECursor, reg_رڪارڊ,
                       reg_بنياد+nOBSat, n_بنياد-nOBSat);
  إذا( iSkip ){
    sqlite3VdbeChangeP2(v, iSkip,
         p_ترتيب_ڏيو->labelOBLopt ? p_ترتيب_ڏيو->labelOBLopt : sqlite3VdbeCurrentAddr(v));
  }
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STMT_SCANSTATUS
  p_ترتيب_ڏيو->addr_ڌڪو_پڇاڙي = sqlite3VdbeCurrentAddr(v)-1;
#جيڪڏهن_پورو
}

/*
** Add code to implement the OFFSET
*/
ساكن فراغ ڪوڊ_وٿي(
  Vdbe *v,          /* Generate code into this VM */
  صحيح i_وٿي,      /* Register holding the offset counter */
  صحيح iContinue     /* Jump here to skip the current record */
){
  إذا( i_وٿي>0 ){
    sqlite3_Vdbe_جوڙيو_Op3(v, OP_IfPos, i_وٿي, iContinue, 1); VdbeCoverage(v);
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
ساكن صحيح ڪوڊ_Distinct(
  Parse *pParse,     /* Parsing and code generating context */
  صحيح eTnctType,     /* WHERE_DISTINCT_* value */
  صحيح iTab,          /* A sorting index used to test for distinctness */
  صحيح addrRepeat,    /* Jump to here if not distinct */
  اظهار_List *pEList,  /* Expression for each element */
  صحيح regElem        /* First element */
){
  صحيح iRet = 0;
  صحيح n_نتيجو_کانو = pEList->n_اظهار;
  Vdbe *v = pParse->pVdbe;

  بدل( eTnctType ){
    حالة WHERE_DISTINCT_ORDERED: {
      صحيح i;
      صحيح iJump;              /* Jump destination */
      صحيح regPrev;            /* Previous row content */

      /* Allocate space for the previous row */
      iRet = regPrev = pParse->nMem+1;
      pParse->nMem += n_نتيجو_کانو;

      iJump = sqlite3VdbeCurrentAddr(v) + n_نتيجو_کانو;
      لكل(i=0; i<n_نتيجو_کانو; i++){
        CollSeq *pColl = sqlite3_اظهار_Coll_Seq(pParse, pEList->a[i].p_اظهار);
        إذا( i<n_نتيجو_کانو-1 ){
          sqlite3_Vdbe_جوڙيو_Op3(v, OP_Ne, regElem+i, iJump, regPrev+i);
          VdbeCoverage(v);
        }وإلا{
          sqlite3_Vdbe_جوڙيو_Op3(v, OP_Eq, regElem+i, addrRepeat, regPrev+i);
          VdbeCoverage(v);
         }
        sqlite3VdbeChangeP4(v, -1, (ثابت حرف *)pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
      }
      أكد( sqlite3VdbeCurrentAddr(v)==iJump || pParse->db->mallocFailed );
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Copy, regElem, regPrev, n_نتيجو_کانو-1);
      اكسر;
    }

    حالة WHERE_DISTINCT_UNIQUE: {
      /* nothing to do */
      اكسر;
    }

    افتراضي: {
      صحيح r1 = sqlite3_Get_عارضي_Reg(pParse);
      sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_مليو, iTab, addrRepeat, regElem, n_نتيجو_کانو);
      VdbeCoverage(v);
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Make_رڪارڊ, regElem, n_نتيجو_کانو, r1);
      sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_Idx_وجهو, iTab, r1, regElem, n_نتيجو_کانو);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      sqlite3_Release_عارضي_Reg(pParse, r1);
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

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SORTER_REFERENCES
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
ساكن فراغ select_اظهار_Defer(
  Parse *pParse,                  /* Leave any error here */
  ترتيب_ڏيو_Ctx *p_ترتيب_ڏيو,                 /* Sorter context */
  اظهار_List *pEList,               /* Expressions destined for sorter */
  اظهار_List **ppExtra              /* Expressions to append to sorter record */
){
  صحيح i;
  صحيح nDefer = 0;
  اظهار_List *pExtra = 0;
  لكل(i=0; i<pEList->n_اظهار; i++){
    بنية اظهار_List_شي *p_شي = &pEList->a[i];
    إذا( p_شي->u.x.i_ترتيب_By_کانو==0 ){
      اظهار *p_اظهار = p_شي->p_اظهار;
      جدول *pTab;
      إذا( p_اظهار->op==TK_ڪالم
       && p_اظهار->i_ڪالم>=0
       && ALWAYS( اظهار_Use_YTab(p_اظهار) )
       && (pTab = p_اظهار->y.pTab)!=0
       && Is_Ordinary_جدول(pTab)
       && (pTab->a_کانو[p_اظهار->i_ڪالم].کانو_Flags & COLFLAG_SORTERREF)!=0
      ){
        صحيح j;
        لكل(j=0; j<nDefer; j++){
          إذا( p_ترتيب_ڏيو->aDefer[j].iCsr==p_اظهار->i_جدول ) اكسر;
        }
        إذا( j==nDefer ){
          إذا( nDefer==Array_سائيز(p_ترتيب_ڏيو->aDefer) ){
            استمر;
          }وإلا{
            صحيح n_ڪنجي = 1;
            صحيح k;
            اشاريو *pPk = 0;
            إذا( !HasRowid(pTab) ){
              pPk = sqlite3_Primary_ڪنجي_اشاريو(pTab);
              n_ڪنجي = pPk->n_ڪنجي_کانو;
            }
            لكل(k=0; k<n_ڪنجي; k++){
              اظهار *pNew = sqlite3PExpr(pParse, TK_ڪالم, 0, 0);
              إذا( pNew ){
                pNew->i_جدول = p_اظهار->i_جدول;
                أكد( اظهار_Use_YTab(pNew) );
                pNew->y.pTab = p_اظهار->y.pTab;
                pNew->i_ڪالم = pPk ? pPk->ai_ڪالم[k] : -1;
                pExtra = sqlite3_اظهار_List_Append(pParse, pExtra, pNew);
              }
            }
            p_ترتيب_ڏيو->aDefer[nDefer].pTab = p_اظهار->y.pTab;
            p_ترتيب_ڏيو->aDefer[nDefer].iCsr = p_اظهار->i_جدول;
            p_ترتيب_ڏيو->aDefer[nDefer].n_ڪنجي = n_ڪنجي;
            nDefer++;
          }
        }
        p_شي->fg.bSorterRef = 1;
      }
    }
  }
  p_ترتيب_ڏيو->nDefer = (u8)nDefer;
  *ppExtra = pExtra;
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
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
ساكن صحيح sqlite3CursorBloomable(Parse *pParse, صحيح iCur, صحيح n_کانو){
  صحيح i,k;
  Vdbe *v = pParse->pVdbe;
  إذا( pParse->nErr ) أرجع 1;
  أكد( v );
  لكل(k=sqlite3VdbeCurrentAddr(v)-1; k>0; k--){
    ثابت VdbeOp *pOp = sqlite3VdbeGetOp(v, k);
    ثابت ڪنجي_Info *p_ڪنجي_Info;
    إذا( pOp->p1!=iCur ) استمر;
    إذا( pOp->opcode!=OP_OpenRead
     && pOp->opcode!=OP_OpenWrite
     && pOp->opcode!=OP_OpenEphemeral
    ){
      استمر;
    }
    أكد( pOp->p4type==P4_KEYINFO );
    p_ڪنجي_Info = (ثابت ڪنجي_Info*)pOp->p4.p_ڪنجي_Info;
    أكد( p_ڪنجي_Info!=0 );
    لكل(i=0; i<n_کانو; i++){
      أكد( sqlite3_Is_ٻٽو(p_ڪنجي_Info->aColl[i]) );
    }
    أرجع 1;
  }
  أرجع 0;
}
#جيڪڏهن_پورو /* SQLITE_DEBUG */


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
  ترتيب_ڏيو_Ctx *p_ترتيب_ڏيو,         /* If not NULL, info on how to process ORDER BY */
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
  صحيح n_نتيجو_کانو;             /* Number of result columns */
  صحيح n_اڳياڙي_Reg = 0;         /* Number of extra registers before regResult */
  صف_ڀريو_Info s_صف_ڀريو_Info;   /* Info for deferred row loading */

  /* Usually, regResult is the first cell in an array of memory cells
  ** containing the current result row. In this case regOrig is set to the
  ** same value. However, if the results are being sent to the sorter, the
  ** values for any expressions that are also part of the sort-key are omitted
  ** from this array. In this case regOrig is set to zero.  */
  صحيح reg_نتيجو;              /* Start of memory holding current results */
  صحيح regOrig;                /* Start of memory holding full result (or 0) */

  أكد( v );
  أكد( p->pEList!=0 );
  hasDistinct = pDistinct ? pDistinct->eTnctType : WHERE_DISTINCT_NOOP;
  إذا( p_ترتيب_ڏيو && p_ترتيب_ڏيو->p_ترتيب_By==0 ) p_ترتيب_ڏيو = 0;
  إذا( p_ترتيب_ڏيو==0 && !hasDistinct ){
    أكد( iContinue!=0 );
    ڪوڊ_وٿي(v, p->i_وٿي, iContinue);
  }

  /* Pull the requested columns.
  */
  n_نتيجو_کانو = p->pEList->n_اظهار;

  إذا( pDest->iSdst==0 ){
    إذا( p_ترتيب_ڏيو ){
      n_اڳياڙي_Reg = p_ترتيب_ڏيو->p_ترتيب_By->n_اظهار;
      إذا( !(p_ترتيب_ڏيو->ترتيب_ڏيو_Flags & SORTFLAG_UseSorter) ) n_اڳياڙي_Reg++;
      pParse->nMem += n_اڳياڙي_Reg;
    }
    pDest->iSdst = pParse->nMem+1;
    pParse->nMem += n_نتيجو_کانو;
  }وإلا إذا( pDest->iSdst+n_نتيجو_کانو > pParse->nMem ){
    /* This is an error condition that can result, for example, when a SELECT
    ** on the right-hand side of an INSERT contains more result columns than
    ** there are columns in the table on the left.  The error will be caught
    ** and reported later.  But we need to make sure enough memory is allocated
    ** to avoid other spurious errors in the meantime. */
    pParse->nMem += n_نتيجو_کانو;
  }
  pDest->nSdst = n_نتيجو_کانو;
  regOrig = reg_نتيجو = pDest->iSdst;
  إذا( srcTab>=0 ){
    لكل(i=0; i<n_نتيجو_کانو; i++){
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_ڪالم, srcTab, i, reg_نتيجو+i);
      VdbeComment((v, "%s", p->pEList->a[i].zEName));
    }
  }وإلا إذا( eDest!=SRT_Exists ){
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SORTER_REFERENCES
    اظهار_List *pExtra = 0;
#جيڪڏهن_پورو
    /* If the destination is an EXISTS(...) expression, the actual
    ** values returned by the SELECT are not required.
    */
    u8 ecelFlags;    /* "ecel" is an abbreviation of "ExprCodeExprList" */
    اظهار_List *pEList;
    إذا( eDest==SRT_Mem || eDest==SRT_Output || eDest==SRT_Coroutine ){
      ecelFlags = SQLITE_ECEL_DUP;
    }وإلا{
      ecelFlags = 0;
    }
    إذا( p_ترتيب_ڏيو && hasDistinct==0 && eDest!=SRT_EphemTab && eDest!=SRT_جدول ){
      /* For each expression in p->pEList that is a copy of an expression in
      ** the ORDER BY clause (pSort->pOrderBy), set the associated
      ** iOrderByCol value to one more than the index of the ORDER BY
      ** expression within the sort-key that pushOntoSorter() will generate.
      ** This allows the p->pEList field to be omitted from the sorted record,
      ** saving space and CPU cycles.  */
      ecelFlags |= (SQLITE_ECEL_OMITREF|SQLITE_ECEL_REF);

      لكل(i=p_ترتيب_ڏيو->nOBSat; i<p_ترتيب_ڏيو->p_ترتيب_By->n_اظهار; i++){
        صحيح j;
        إذا( (j = p_ترتيب_ڏيو->p_ترتيب_By->a[i].u.x.i_ترتيب_By_کانو)>0 ){
          p->pEList->a[j-1].u.x.i_ترتيب_By_کانو = i+1-p_ترتيب_ڏيو->nOBSat;
        }
      }
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SORTER_REFERENCES
      select_اظهار_Defer(pParse, p_ترتيب_ڏيو, p->pEList, &pExtra);
      إذا( pExtra && pParse->db->mallocFailed==0 ){
        /* If there are any extra PK columns to add to the sorter records,
        ** allocate extra memory cells and adjust the OpenEphemeral
        ** instruction to account for the larger records. This is only
        ** required if there are one or more WITHOUT ROWID tables with
        ** composite primary keys in the SortCtx.aDefer[] array.  */
        VdbeOp *pOp = sqlite3VdbeGetOp(v, p_ترتيب_ڏيو->addr_ترتيب_ڏيو_اشاريو);
        pOp->p2 += (pExtra->n_اظهار - p_ترتيب_ڏيو->nDefer);
        pOp->p4.p_ڪنجي_Info->n_All_ميدان += (pExtra->n_اظهار - p_ترتيب_ڏيو->nDefer);
        pParse->nMem += pExtra->n_اظهار;
      }
#جيڪڏهن_پورو

      /* Adjust nResultCol to account for columns that are omitted
      ** from the sorter by the optimizations in this branch */
      pEList = p->pEList;
      لكل(i=0; i<pEList->n_اظهار; i++){
        إذا( pEList->a[i].u.x.i_ترتيب_By_کانو>0
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SORTER_REFERENCES
         || pEList->a[i].fg.bSorterRef
#جيڪڏهن_پورو
        ){
          n_نتيجو_کانو--;
          regOrig = 0;
        }
      }

      testcase( regOrig );
      testcase( eDest==SRT_سيٽ );
      testcase( eDest==SRT_Mem );
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      أكد( eDest==SRT_سيٽ || eDest==SRT_Mem
           || eDest==SRT_Coroutine || eDest==SRT_Output
           || eDest==SRT_Upfrom );
    }
    s_صف_ڀريو_Info.reg_نتيجو = reg_نتيجو;
    s_صف_ڀريو_Info.ecelFlags = ecelFlags;
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SORTER_REFERENCES
    s_صف_ڀريو_Info.pExtra = pExtra;
    s_صف_ڀريو_Info.reg_Extra_نتيجو = reg_نتيجو + n_نتيجو_کانو;
    إذا( pExtra ) n_نتيجو_کانو += pExtra->n_اظهار;
#جيڪڏهن_پورو
    إذا( p->i_حد_بندي
     && (ecelFlags & SQLITE_ECEL_OMITREF)!=0
     && n_اڳياڙي_Reg>0
    ){
      أكد( p_ترتيب_ڏيو!=0 );
      أكد( hasDistinct==0 );
      p_ترتيب_ڏيو->p_Deferred_صف_ڀريو = &s_صف_ڀريو_Info;
      regOrig = 0;
    }وإلا{
      inner_Loop_ڀريو_صف(pParse, p, &s_صف_ڀريو_Info);
    }
  }

  /* If the DISTINCT keyword was present on the SELECT statement
  ** and this row has been seen before, then do not make this row
  ** part of the result.
  */
  إذا( hasDistinct ){
    صحيح eType = pDistinct->eTnctType;
    صحيح iTab = pDistinct->tabTnct;
    أكد( n_نتيجو_کانو==p->pEList->n_اظهار );
    iTab = ڪوڊ_Distinct(pParse, eType, iTab, iContinue, p->pEList, reg_نتيجو);
    fixDistinctOpenEph(pParse, eType, iTab, pDistinct->addrTnct);
    إذا( p_ترتيب_ڏيو==0 ){
      ڪوڊ_وٿي(v, p->i_وٿي, iContinue);
    }
  }

  بدل( eDest ){
    /* Store the result as data using a unique key.
    */
    حالة SRT_Fifo:
    حالة SRT_DistFifo:
    حالة SRT_جدول:
    حالة SRT_EphemTab: {
      صحيح r1 = sqlite3_Get_عارضي_Range(pParse, n_اڳياڙي_Reg+1);
      testcase( eDest==SRT_جدول );
      testcase( eDest==SRT_EphemTab );
      testcase( eDest==SRT_Fifo );
      testcase( eDest==SRT_DistFifo );
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Make_رڪارڊ, reg_نتيجو, n_نتيجو_کانو, r1+n_اڳياڙي_Reg);
#إذا !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      إذا( eDest==SRT_جدول && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_CTE
      إذا( eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index. If the current row is already present
        ** in the index, do not write it to the output. If not, add the
        ** current row to the index and proceed with writing it to the
        ** output table as well.  */
        صحيح addr = sqlite3VdbeCurrentAddr(v) + 4;
        sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_مليو, iParm+1, addr, r1, 0);
        VdbeCoverage(v);
        sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_Idx_وجهو, iParm+1, r1,reg_نتيجو,n_نتيجو_کانو);
        أكد( p_ترتيب_ڏيو==0 );
      }
#جيڪڏهن_پورو
      إذا( p_ترتيب_ڏيو ){
        أكد( reg_نتيجو==regOrig );
        ڌڪو_Onto_Sorter(pParse, p_ترتيب_ڏيو, p, r1+n_اڳياڙي_Reg, regOrig, 1, n_اڳياڙي_Reg);
      }وإلا{
        صحيح r2 = sqlite3_Get_عارضي_Reg(pParse);
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_NewRowid, iParm, r2);
        sqlite3_Vdbe_جوڙيو_Op3(v, OP_وجهو, iParm, r1, r2);
        sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
        sqlite3_Release_عارضي_Reg(pParse, r2);
      }
      sqlite3_Release_عارضي_Range(pParse, r1, n_اڳياڙي_Reg+1);
      اكسر;
    }

    حالة SRT_Upfrom: {
      إذا( p_ترتيب_ڏيو ){
        ڌڪو_Onto_Sorter(
            pParse, p_ترتيب_ڏيو, p, reg_نتيجو, regOrig, n_نتيجو_کانو, n_اڳياڙي_Reg);
      }وإلا{
        صحيح i2 = pDest->iSDParm2;
        صحيح r1 = sqlite3_Get_عارضي_Reg(pParse);

        /* If the UPDATE FROM join is an aggregate that matches no rows, it
        ** might still be trying to return one row, because that is what
        ** aggregates do.  Don't record that empty row in the output table. */
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_IsNull, reg_نتيجو, iBreak); VdbeCoverage(v);

        sqlite3_Vdbe_جوڙيو_Op3(v, OP_Make_رڪارڊ,
                          reg_نتيجو+(i2<0), n_نتيجو_کانو-(i2<0), r1);
        إذا( i2<0 ){
          sqlite3_Vdbe_جوڙيو_Op3(v, OP_وجهو, iParm, r1, reg_نتيجو);
        }وإلا{
          sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_Idx_وجهو, iParm, r1, reg_نتيجو, i2);
        }
      }
      اكسر;
    }

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)" construct,
    ** then there should be a single item on the stack.  Write this
    ** item into the set table with bogus data.
    */
    حالة SRT_سيٽ: {
      إذا( p_ترتيب_ڏيو ){
        /* At first glance you would think we could optimize out the
        ** ORDER BY in this case since the order of entries in the set
        ** does not matter.  But there might be a LIMIT clause, in which
        ** case the order does matter */
        ڌڪو_Onto_Sorter(
            pParse, p_ترتيب_ڏيو, p, reg_نتيجو, regOrig, n_نتيجو_کانو, n_اڳياڙي_Reg);
        pDest->iSDParm2 = 0; /* Signal that any Bloom filter is unpopulated */
      }وإلا{
        صحيح r1 = sqlite3_Get_عارضي_Reg(pParse);
        أكد( sqlite3Strlen30(pDest->zAffSdst)==n_نتيجو_کانو );
        sqlite3_Vdbe_جوڙيو_Op4(v, OP_Make_رڪارڊ, reg_نتيجو, n_نتيجو_کانو,
            r1, pDest->zAffSdst, n_نتيجو_کانو);
        sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_Idx_وجهو, iParm, r1, reg_نتيجو, n_نتيجو_کانو);
        إذا( pDest->iSDParm2 ){
          أكد( sqlite3CursorBloomable(pParse,iParm,n_نتيجو_کانو) );
          sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_Filter_جوڙيو, pDest->iSDParm2, 0,
                               reg_نتيجو, n_نتيجو_کانو);
          ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
        }
        sqlite3_Release_عارضي_Reg(pParse, r1);
      }
      اكسر;
    }


    /* If any row exist in the result set, record that fact and abort.
    */
    حالة SRT_Exists: {
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, 1, iParm);
      /* The LIMIT clause will terminate the loop for us */
      اكسر;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell or array of
    ** memory cells and break out of the scan loop.
    */
    حالة SRT_Mem: {
      إذا( p_ترتيب_ڏيو ){
        أكد( n_نتيجو_کانو<=pDest->nSdst );
        ڌڪو_Onto_Sorter(
            pParse, p_ترتيب_ڏيو, p, reg_نتيجو, regOrig, n_نتيجو_کانو, n_اڳياڙي_Reg);
        pDest->iSDParm = reg_نتيجو;
      }وإلا{
        أكد( n_نتيجو_کانو==pDest->nSdst );
        إذا( reg_نتيجو!=iParm ){
          /* This occurs in cases where the SELECT had both a DISTINCT and
          ** an OFFSET clause.  */
          sqlite3_Vdbe_جوڙيو_Op3(v, OP_Copy, reg_نتيجو, iParm, n_نتيجو_کانو-1);
        }
        /* The LIMIT clause will jump out of the loop for us */
      }
      اكسر;
    }
#جيڪڏهن_پورو /* #ifndef SQLITE_OMIT_SUBQUERY */

    حالة SRT_Coroutine:       /* Send data to a co-routine */
    حالة SRT_Output: {        /* Return the results */
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      إذا( p_ترتيب_ڏيو ){
        ڌڪو_Onto_Sorter(pParse, p_ترتيب_ڏيو, p, reg_نتيجو, regOrig, n_نتيجو_کانو,
                       n_اڳياڙي_Reg);
      }وإلا إذا( eDest==SRT_Coroutine ){
        sqlite3_Vdbe_جوڙيو_Op1(v, OP_Yield, pDest->iSDParm);
      }وإلا{
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_نتيجو_صف, reg_نتيجو, n_نتيجو_کانو);
      }
      اكسر;
    }

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    حالة SRT_Dist_قطار:
    حالة SRT_قطار: {
      صحيح n_ڪنجي;
      صحيح r1, r2, r3;
      صحيح addrTest = 0;
      اظهار_List *pSO;
      pSO = pDest->p_ترتيب_By;
      أكد( pSO );
      n_ڪنجي = pSO->n_اظهار;
      r1 = sqlite3_Get_عارضي_Reg(pParse);
      r2 = sqlite3_Get_عارضي_Range(pParse, n_ڪنجي+2);
      r3 = r2+n_ڪنجي+1;
      إذا( eDest==SRT_Dist_قطار ){
        /* If the destination is DistQueue, then cursor (iParm+1) is open
        ** on a second ephemeral index that holds all values every previously
        ** added to the queue. */
        addrTest = sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_مليو, iParm+1, 0,
                                        reg_نتيجو, n_نتيجو_کانو);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Make_رڪارڊ, reg_نتيجو, n_نتيجو_کانو, r3);
      إذا( eDest==SRT_Dist_قطار ){
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_Idx_وجهو, iParm+1, r3);
        sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      }
      لكل(i=0; i<n_ڪنجي; i++){
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_SCopy,
                          reg_نتيجو + pSO->a[i].u.x.i_ترتيب_By_کانو - 1,
                          r2+i);
      }
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Sequence, iParm, r2+n_ڪنجي);
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Make_رڪارڊ, r2, n_ڪنجي+2, r1);
      sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_Idx_وجهو, iParm, r1, r2, n_ڪنجي+2);
      إذا( addrTest ) sqlite3_Vdbe_Jump_هتي(v, addrTest);
      sqlite3_Release_عارضي_Reg(pParse, r1);
      sqlite3_Release_عارضي_Range(pParse, r2, n_ڪنجي+2);
      اكسر;
    }
#جيڪڏهن_پورو /* SQLITE_OMIT_CTE */



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
#جيڪڏهن_پورو
  }

  /* Jump to the end of the loop if the LIMIT is reached.  Except, if
  ** there is a sorter, in which case the sorter has already limited
  ** the output for us.
  */
  إذا( p_ترتيب_ڏيو==0 && p->i_حد_بندي ){
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_DecrJumpZero, p->i_حد_بندي, iBreak); VdbeCoverage(v);
  }
}

/*
** Allocate a KeyInfo object sufficient for an index of N key columns and
** X extra columns.
*/
ڪنجي_Info *sqlite3_ڪنجي_Info_Alloc(sqlite3 *db, صحيح N, صحيح X){
  صحيح nExtra = (N+X)*(حجم(CollSeq*)+1);
  ڪنجي_Info *p;
  أكد( X>=0 );
  إذا( NEVER(N+X>0xffff) ) أرجع (ڪنجي_Info*)sqlite3OomFault(db);
  p = sqlite3DbMallocRawNN(db, SZ_KEYINFO(0) + nExtra);
  إذا( p ){
    p->a_ترتيب_ڏيو_Flags = (u8*)&p->aColl[N+X];
    p->n_ڪنجي_ميدان = (u16)N;
    p->n_All_ميدان = (u16)(N+X);
    p->enc = ENC(db);
    p->db = db;
    p->nRef = 1;
    memset(p->aColl, 0, nExtra);
  }وإلا{
    أرجع (ڪنجي_Info*)sqlite3OomFault(db);
  }
  أرجع p;
}

/*
** Deallocate a KeyInfo object
*/
فراغ sqlite3_ڪنجي_Info_Unref(ڪنجي_Info *p){
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
ڪنجي_Info *sqlite3_ڪنجي_Info_Ref(ڪنجي_Info *p){
  إذا( p ){
    أكد( p->nRef>0 );
    p->nRef++;
  }
  أرجع p;
}

#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
/*
** Return TRUE if a KeyInfo object can be change.  The KeyInfo object
** can only be changed if this is just a single reference to the object.
**
** This routine is used only inside of assert() statements.
*/
صحيح sqlite3_ڪنجي_Info_Is_Writeable(ڪنجي_Info *p){ أرجع p->nRef==1; }
#جيڪڏهن_پورو /* SQLITE_DEBUG */

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
ڪنجي_Info *sqlite3_ڪنجي_Info_From_اظهار_List(
  Parse *pParse,       /* Parsing context */
  اظهار_List *pList,     /* Form the KeyInfo object from this ExprList */
  صحيح i_شروعات,          /* Begin with this column of pList */
  صحيح nExtra           /* Add this many extra columns to the end */
){
  صحيح n_اظهار;
  ڪنجي_Info *pInfo;
  بنية اظهار_List_شي *p_شي;
  sqlite3 *db = pParse->db;
  صحيح i;

  n_اظهار = pList->n_اظهار;
  pInfo = sqlite3_ڪنجي_Info_Alloc(db, n_اظهار-i_شروعات, nExtra+1);
  إذا( pInfo ){
    أكد( sqlite3_ڪنجي_Info_Is_Writeable(pInfo) );
    لكل(i=i_شروعات, p_شي=pList->a+i_شروعات; i<n_اظهار; i++, p_شي++){
      pInfo->aColl[i-i_شروعات] = sqlite3_اظهار_NNColl_Seq(pParse, p_شي->p_اظهار);
      pInfo->a_ترتيب_ڏيو_Flags[i-i_شروعات] = p_شي->fg.ترتيب_ڏيو_Flags;
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

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_EXPLAIN
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
ساكن فراغ explain_عارضي_جدول(Parse *pParse, ثابت حرف *zUsage){
  ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s", zUsage));
}

/*
** Assign expression b to lvalue a. A second, no-op, version of this macro
** is provided when SQLITE_OMIT_EXPLAIN is defined. This allows the code
** in sqlite3Select() to assign values to structure member variables that
** only exist if SQLITE_OMIT_EXPLAIN is not defined without polluting the
** code with #ifndef directives.
*/
# define explain_سيٽ_Integer(a, b) a = b

#وإلا
/* No-op versions of the explainXXX() functions and macros. */
# define explain_عارضي_جدول(y,z)
# define explain_سيٽ_Integer(y,z)
#جيڪڏهن_پورو


/*
** If the inner loop was generated using a non-null pOrderBy argument,
** then the results were placed in a sorter.  After the loop is terminated
** we need to run the sorter and output the results.  The following
** routine generates the code needed to do that.
*/
ساكن فراغ generate_ترتيب_ڏيو_پڇ(
  Parse *pParse,    /* Parsing context */
  Select *p,        /* The SELECT statement */
  ترتيب_ڏيو_Ctx *p_ترتيب_ڏيو,   /* Information on the ORDER BY clause */
  صحيح n_ڪالم,      /* Number of columns of data */
  SelectDest *pDest /* Write the sorted results here */
){
  Vdbe *v = pParse->pVdbe;                     /* The prepared statement */
  صحيح addrBreak = p_ترتيب_ڏيو->label_پورو;            /* Jump here to exit loop */
  صحيح addrContinue = sqlite3VdbeMakeLabel(pParse);/* Jump here for next cycle */
  صحيح addr;                       /* Top of output loop. Jump for Next. */
  صحيح addrOnce = 0;
  صحيح iTab;
  اظهار_List *p_ترتيب_By = p_ترتيب_ڏيو->p_ترتيب_By;
  صحيح eDest = pDest->eDest;
  صحيح iParm = pDest->iSDParm;
  صحيح reg_صف;
  صحيح regRowid;
  صحيح i_کانو;
  صحيح n_ڪنجي;                       /* Number of key columns in sorter record */
  صحيح i_ترتيب_ڏيو_Tab;                   /* Sorter cursor to read from */
  صحيح i;
  صحيح bSeq;                       /* True if sorter record includes seq. no. */
  صحيح n_Ref_ڪنجي = 0;
  بنية اظهار_List_شي *a_ٻاهريون_Ex = p->pEList->a;
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STMT_SCANSTATUS
  صحيح addrExplain;                /* Address of OP_Explain instruction */
#جيڪڏهن_پورو

  n_ڪنجي = p_ترتيب_By->n_اظهار - p_ترتيب_ڏيو->nOBSat;
  إذا( p_ترتيب_ڏيو->nOBSat==0 || n_ڪنجي==1 ){
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR %sORDER BY", p_ترتيب_ڏيو->nOBSat?"LAST TERM OF ":""
    ));
  }وإلا{
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR LAST %d TERMS OF ORDER BY", n_ڪنجي
    ));
  }
  sqlite3_Vdbe_جاچ_حالت_Range(v, addrExplain,p_ترتيب_ڏيو->addr_ڌڪو,p_ترتيب_ڏيو->addr_ڌڪو_پڇاڙي);
  sqlite3_Vdbe_جاچ_حالت_Counters(v, addrExplain, addrExplain, p_ترتيب_ڏيو->addr_ڌڪو);


  أكد( addrBreak<0 );
  إذا( p_ترتيب_ڏيو->label_Bk_ٻاهريون ){
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_Gosub, p_ترتيب_ڏيو->regReturn, p_ترتيب_ڏيو->label_Bk_ٻاهريون);
    sqlite3VdbeGoto(v, addrBreak);
    sqlite3VdbeResolveLabel(v, p_ترتيب_ڏيو->label_Bk_ٻاهريون);
  }

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SORTER_REFERENCES
  /* Open any cursors needed for sorter-reference expressions */
  لكل(i=0; i<p_ترتيب_ڏيو->nDefer; i++){
    جدول *pTab = p_ترتيب_ڏيو->aDefer[i].pTab;
    صحيح iDb = sqlite3_Schema_منزل_اشاريو(pParse->db, pTab->pSchema);
    sqlite3_Open_جدول(pParse, p_ترتيب_ڏيو->aDefer[i].iCsr, iDb, pTab, OP_OpenRead);
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_Rewind, p_ترتيب_ڏيو->aDefer[i].iCsr,
                      sqlite3VdbeCurrentAddr(v)+1);
    VdbeCoverage(v);
    n_Ref_ڪنجي = MAX(n_Ref_ڪنجي, p_ترتيب_ڏيو->aDefer[i].n_ڪنجي);
  }
#جيڪڏهن_پورو

  iTab = p_ترتيب_ڏيو->iECursor;
  إذا( eDest==SRT_Output || eDest==SRT_Coroutine || eDest==SRT_Mem ){
    إذا( eDest==SRT_Mem && p->i_وٿي ){
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Null, 0, pDest->iSdst);
    }
    regRowid = 0;
    reg_صف = pDest->iSdst;
  }وإلا{
    regRowid = sqlite3_Get_عارضي_Reg(pParse);
    إذا( eDest==SRT_EphemTab || eDest==SRT_جدول ){
      reg_صف = sqlite3_Get_عارضي_Reg(pParse);
      n_ڪالم = 0;
    }وإلا{
      reg_صف = sqlite3_Get_عارضي_Range(pParse, n_ڪالم);
    }
  }
  إذا( p_ترتيب_ڏيو->ترتيب_ڏيو_Flags & SORTFLAG_UseSorter ){
    صحيح reg_ترتيب_ڏيو_ٻاهريون = ++pParse->nMem;
    i_ترتيب_ڏيو_Tab = pParse->nTab++;
    إذا( p_ترتيب_ڏيو->label_Bk_ٻاهريون ){
      addrOnce = sqlite3_Vdbe_جوڙيو_Op0(v, OP_Once); VdbeCoverage(v);
    }
    sqlite3_Vdbe_جوڙيو_Op3(v, OP_OpenPseudo, i_ترتيب_ڏيو_Tab, reg_ترتيب_ڏيو_ٻاهريون,
        n_ڪنجي+1+n_ڪالم+n_Ref_ڪنجي);
    إذا( addrOnce ) sqlite3_Vdbe_Jump_هتي(v, addrOnce);
    addr = 1 + sqlite3_Vdbe_جوڙيو_Op2(v, OP_Sorter_ترتيب_ڏيو, iTab, addrBreak);
    VdbeCoverage(v);
    أكد( p->i_حد_بندي==0 && p->i_وٿي==0 );
    sqlite3_Vdbe_جوڙيو_Op3(v, OP_Sorter_معلومات, iTab, reg_ترتيب_ڏيو_ٻاهريون, i_ترتيب_ڏيو_Tab);
    bSeq = 0;
  }وإلا{
    addr = 1 + sqlite3_Vdbe_جوڙيو_Op2(v, OP_ترتيب_ڏيو, iTab, addrBreak); VdbeCoverage(v);
    ڪوڊ_وٿي(v, p->i_وٿي, addrContinue);
    i_ترتيب_ڏيو_Tab = iTab;
    bSeq = 1;
    إذا( p->i_وٿي>0 ){
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_جوڙيو_Imm, p->i_حد_بندي, -1);
    }
  }
  لكل(i=0, i_کانو=n_ڪنجي+bSeq-1; i<n_ڪالم; i++){
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SORTER_REFERENCES
    إذا( a_ٻاهريون_Ex[i].fg.bSorterRef ) استمر;
#جيڪڏهن_پورو
    إذا( a_ٻاهريون_Ex[i].u.x.i_ترتيب_By_کانو==0 ) i_کانو++;
  }
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SORTER_REFERENCES
  إذا( p_ترتيب_ڏيو->nDefer ){
    صحيح i_ڪنجي = i_کانو+1;
    صحيح reg_ڪنجي = sqlite3_Get_عارضي_Range(pParse, n_Ref_ڪنجي);

    لكل(i=0; i<p_ترتيب_ڏيو->nDefer; i++){
      صحيح iCsr = p_ترتيب_ڏيو->aDefer[i].iCsr;
      جدول *pTab = p_ترتيب_ڏيو->aDefer[i].pTab;
      صحيح n_ڪنجي = p_ترتيب_ڏيو->aDefer[i].n_ڪنجي;

      sqlite3_Vdbe_جوڙيو_Op1(v, OP_Null_صف, iCsr);
      إذا( HasRowid(pTab) ){
        sqlite3_Vdbe_جوڙيو_Op3(v, OP_ڪالم, i_ترتيب_ڏيو_Tab, i_ڪنجي++, reg_ڪنجي);
        sqlite3_Vdbe_جوڙيو_Op3(v, OP_SeekRowid, iCsr,
            sqlite3VdbeCurrentAddr(v)+1, reg_ڪنجي);
      }وإلا{
        صحيح k;
        صحيح iJmp;
        أكد( sqlite3_Primary_ڪنجي_اشاريو(pTab)->n_ڪنجي_کانو==n_ڪنجي );
        لكل(k=0; k<n_ڪنجي; k++){
          sqlite3_Vdbe_جوڙيو_Op3(v, OP_ڪالم, i_ترتيب_ڏيو_Tab, i_ڪنجي++, reg_ڪنجي+k);
        }
        iJmp = sqlite3VdbeCurrentAddr(v);
        sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_SeekGE, iCsr, iJmp+2, reg_ڪنجي, n_ڪنجي);
        sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_IdxLE, iCsr, iJmp+3, reg_ڪنجي, n_ڪنجي);
        sqlite3_Vdbe_جوڙيو_Op1(v, OP_Null_صف, iCsr);
      }
    }
    sqlite3_Release_عارضي_Range(pParse, reg_ڪنجي, n_Ref_ڪنجي);
  }
#جيڪڏهن_پورو
  لكل(i=n_ڪالم-1; i>=0; i--){
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_SORTER_REFERENCES
    إذا( a_ٻاهريون_Ex[i].fg.bSorterRef ){
      sqlite3_اظهار_ڪوڊ(pParse, a_ٻاهريون_Ex[i].p_اظهار, reg_صف+i);
    }وإلا
#جيڪڏهن_پورو
    {
      صحيح iRead;
      إذا( a_ٻاهريون_Ex[i].u.x.i_ترتيب_By_کانو ){
        iRead = a_ٻاهريون_Ex[i].u.x.i_ترتيب_By_کانو-1;
      }وإلا{
        iRead = i_کانو--;
      }
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_ڪالم, i_ترتيب_ڏيو_Tab, iRead, reg_صف+i);
      VdbeComment((v, "%s", a_ٻاهريون_Ex[i].zEName));
    }
  }
  sqlite3_Vdbe_جاچ_حالت_Range(v, addrExplain, addrExplain, -1);
  بدل( eDest ){
    حالة SRT_جدول:
    حالة SRT_EphemTab: {
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_ڪالم, i_ترتيب_ڏيو_Tab, n_ڪنجي+bSeq, reg_صف);
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_NewRowid, iParm, regRowid);
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_وجهو, iParm, reg_صف, regRowid);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      اكسر;
    }
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_SUBQUERY
    حالة SRT_سيٽ: {
      أكد( n_ڪالم==sqlite3Strlen30(pDest->zAffSdst) );
      sqlite3_Vdbe_جوڙيو_Op4(v, OP_Make_رڪارڊ, reg_صف, n_ڪالم, regRowid,
                        pDest->zAffSdst, n_ڪالم);
      sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_Idx_وجهو, iParm, regRowid, reg_صف, n_ڪالم);
      اكسر;
    }
    حالة SRT_Mem: {
      /* The LIMIT clause will terminate the loop for us */
      اكسر;
    }
#جيڪڏهن_پورو
    حالة SRT_Upfrom: {
      صحيح i2 = pDest->iSDParm2;
      صحيح r1 = sqlite3_Get_عارضي_Reg(pParse);
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Make_رڪارڊ,reg_صف+(i2<0),n_ڪالم-(i2<0),r1);
      إذا( i2<0 ){
        sqlite3_Vdbe_جوڙيو_Op3(v, OP_وجهو, iParm, r1, reg_صف);
      }وإلا{
        sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_Idx_وجهو, iParm, r1, reg_صف, i2);
      }
      اكسر;
    }
    افتراضي: {
      أكد( eDest==SRT_Output || eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      testcase( eDest==SRT_Coroutine );
      إذا( eDest==SRT_Output ){
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_نتيجو_صف, pDest->iSdst, n_ڪالم);
      }وإلا{
        sqlite3_Vdbe_جوڙيو_Op1(v, OP_Yield, pDest->iSDParm);
      }
      اكسر;
    }
  }
  إذا( regRowid ){
    إذا( eDest==SRT_سيٽ ){
      sqlite3_Release_عارضي_Range(pParse, reg_صف, n_ڪالم);
    }وإلا{
      sqlite3_Release_عارضي_Reg(pParse, reg_صف);
    }
    sqlite3_Release_عارضي_Reg(pParse, regRowid);
  }
  /* The bottom of the loop
  */
  sqlite3VdbeResolveLabel(v, addrContinue);
  إذا( p_ترتيب_ڏيو->ترتيب_ڏيو_Flags & SORTFLAG_UseSorter ){
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_Sorter_ايندڙ, iTab, addr); VdbeCoverage(v);
  }وإلا{
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_ايندڙ, iTab, addr); VdbeCoverage(v);
  }
  sqlite3_Vdbe_جاچ_حالت_Range(v, addrExplain, sqlite3VdbeCurrentAddr(v)-1, -1);
  إذا( p_ترتيب_ڏيو->regReturn ) sqlite3_Vdbe_جوڙيو_Op1(v, OP_Return, p_ترتيب_ڏيو->regReturn);
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
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_ڪالم_METADATA
# define ڪالم_Type(A,B,C,D,E) ڪالم_Type_Impl(A,B,C,D,E)
#وإلا /* if !defined(SQLITE_ENABLE_COLUMN_METADATA) */
# define ڪالم_Type(A,B,C,D,E) ڪالم_Type_Impl(A,B)
#جيڪڏهن_پورو
ساكن ثابت حرف *ڪالم_Type_Impl(
  NameContext *pNC,
#جيڪڏهن_وصف_نه_ٿيل SQLITE_ENABLE_ڪالم_METADATA
  اظهار *p_اظهار
#وإلا
  اظهار *p_اظهار,
  ثابت حرف **pzOrigDb,
  ثابت حرف **pzOrigTab,
  ثابت حرف **pz_Orig_کانو
#جيڪڏهن_پورو
){
  حرف ثابت *zType = 0;
  صحيح j;
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_ڪالم_METADATA
  حرف ثابت *zOrigDb = 0;
  حرف ثابت *zOrigTab = 0;
  حرف ثابت *z_Orig_کانو = 0;
#جيڪڏهن_پورو

  أكد( p_اظهار!=0 );
  أكد( pNC->pSrcList!=0 );
  بدل( p_اظهار->op ){
    حالة TK_ڪالم: {
      /* The expression is a column. Locate the table the column is being
      ** extracted from in NameContext.pSrcList. This table may be real
      ** database table or a subquery.
      */
      جدول *pTab = 0;            /* Table structure column is extracted from */
      Select *pS = 0;             /* Select the column is extracted from */
      صحيح i_کانو = p_اظهار->i_ڪالم;  /* Index of column in pTab */
      طالما( pNC && !pTab ){
        SrcList *pTabList = pNC->pSrcList;
        لكل(j=0;j<pTabList->nSrc && pTabList->a[j].iCursor!=p_اظهار->i_جدول;j++);
        إذا( j<pTabList->nSrc ){
          pTab = pTabList->a[j].pSTab;
          إذا( pTabList->a[j].fg.isSubquery ){
            pS = pTabList->a[j].u4.pSubq->pSelect;
          }وإلا{
            pS = 0;
          }
        }وإلا{
          pNC = pNC->p_ايندڙ;
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

      أكد( pTab && اظهار_Use_YTab(p_اظهار) && p_اظهار->y.pTab==pTab );
      إذا( pS ){
        /* The "table" is actually a sub-select or a view in the FROM clause
        ** of the SELECT statement. Return the declaration type and origin
        ** data for the result-set column of the sub-select.
        */
        إذا( i_کانو<pS->pEList->n_اظهار
         && (!ViewCanHaveRowid || i_کانو>=0)
        ){
          /* If iCol is less than zero, then the expression requests the
          ** rowid of the sub-select or view. This expression is legal (see
          ** test case misc2.2.2) - it always evaluates to NULL.
          */
          NameContext sNC;
          اظهار *p = pS->pEList->a[i_کانو].p_اظهار;
          sNC.pSrcList = pS->pSrc;
          sNC.p_ايندڙ = pNC;
          sNC.pParse = pNC->pParse;
          zType = ڪالم_Type(&sNC, p,&zOrigDb,&zOrigTab,&z_Orig_کانو);
        }
      }وإلا{
        /* A real table or a CTE table */
        أكد( !pS );
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_ڪالم_METADATA
        إذا( i_کانو<0 ) i_کانو = pTab->iPKey;
        أكد( i_کانو==XN_ROWID || (i_کانو>=0 && i_کانو<pTab->n_کانو) );
        إذا( i_کانو<0 ){
          zType = "INTEGER";
          z_Orig_کانو = "rowid";
        }وإلا{
          z_Orig_کانو = pTab->a_کانو[i_کانو].zCnName;
          zType = sqlite3_ڪالم_Type(&pTab->a_کانو[i_کانو],0);
        }
        zOrigTab = pTab->zName;
        إذا( pNC->pParse && pTab->pSchema ){
          صحيح iDb = sqlite3_Schema_منزل_اشاريو(pNC->pParse->db, pTab->pSchema);
          zOrigDb = pNC->pParse->db->aDb[iDb].zDbSName;
        }
#وإلا
        أكد( i_کانو==XN_ROWID || (i_کانو>=0 && i_کانو<pTab->n_کانو) );
        إذا( i_کانو<0 ){
          zType = "INTEGER";
        }وإلا{
          zType = sqlite3_ڪالم_Type(&pTab->a_کانو[i_کانو],0);
        }
#جيڪڏهن_پورو
      }
      اكسر;
    }
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_SUBQUERY
    حالة TK_SELECT: {
      /* The expression is a sub-select. Return the declaration type and
      ** origin info for the single column in the result set of the SELECT
      ** statement.
      */
      NameContext sNC;
      Select *pS;
      اظهار *p;
      أكد( اظهار_Use_XSelect(p_اظهار) );
      pS = p_اظهار->x.pSelect;
      p = pS->pEList->a[0].p_اظهار;
      sNC.pSrcList = pS->pSrc;
      sNC.p_ايندڙ = pNC;
      sNC.pParse = pNC->pParse;
      zType = ڪالم_Type(&sNC, p, &zOrigDb, &zOrigTab, &z_Orig_کانو);
      اكسر;
    }
#جيڪڏهن_پورو
  }

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_ڪالم_METADATA 
  إذا( pzOrigDb ){
    أكد( pzOrigTab && pz_Orig_کانو );
    *pzOrigDb = zOrigDb;
    *pzOrigTab = zOrigTab;
    *pz_Orig_کانو = z_Orig_کانو;
  }
#جيڪڏهن_پورو
  أرجع zType;
}

/*
** Generate code that will tell the VDBE the declaration types of columns
** in the result set.
*/
ساكن فراغ generate_ڪالم_Types(
  Parse *pParse,      /* Parser context */
  SrcList *pTabList,  /* List of tables */
  اظهار_List *pEList    /* Expressions defining the result set */
){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_DECLTYPE
  Vdbe *v = pParse->pVdbe;
  صحيح i;
  NameContext sNC;
  sNC.pSrcList = pTabList;
  sNC.pParse = pParse;
  sNC.p_ايندڙ = 0;
  لكل(i=0; i<pEList->n_اظهار; i++){
    اظهار *p = pEList->a[i].p_اظهار;
    ثابت حرف *zType;
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_ڪالم_METADATA
    ثابت حرف *zOrigDb = 0;
    ثابت حرف *zOrigTab = 0;
    ثابت حرف *z_Orig_کانو = 0;
    zType = ڪالم_Type(&sNC, p, &zOrigDb, &zOrigTab, &z_Orig_کانو);

    /* The vdbe must make its own copy of the column-type and other
    ** column specific strings, in case the schema is reset before this
    ** virtual machine is deleted.
    */
    sqlite3_Vdbe_سيٽ_کانو_Name(v, i, COLNAME_DATABASE, zOrigDb, SQLITE_TRANSIENT);
    sqlite3_Vdbe_سيٽ_کانو_Name(v, i, COLNAME_جدول, zOrigTab, SQLITE_TRANSIENT);
    sqlite3_Vdbe_سيٽ_کانو_Name(v, i, COLNAME_ڪالم, z_Orig_کانو, SQLITE_TRANSIENT);
#وإلا
    zType = ڪالم_Type(&sNC, p, 0, 0, 0);
#جيڪڏهن_پورو
    sqlite3_Vdbe_سيٽ_کانو_Name(v, i, COLNAME_DECLTYPE, zType, SQLITE_TRANSIENT);
  }
#وإلا
  اڻ_استعمال_ٿيل_PARAMETER(pParse);
  اڻ_استعمال_ٿيل_PARAMETER(pTabList);
  اڻ_استعمال_ٿيل_PARAMETER(pEList);
#جيڪڏهن_پورو /* !defined(SQLITE_OMIT_DECLTYPE) */
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
فراغ sqlite3_Generate_ڪالم_نالا(
  Parse *pParse,      /* Parser context */
  Select *pSelect     /* Generate column names for this SELECT statement */
){
  Vdbe *v = pParse->pVdbe;
  صحيح i;
  جدول *pTab;
  SrcList *pTabList;
  اظهار_List *pEList;
  sqlite3 *db = pParse->db;
  صحيح ڀريل_Name;    /* TABLE.COLUMN if no AS clause and is a direct table ref */
  صحيح srcName;     /* COLUMN or TABLE.COLUMN if no AS clause and is direct */

  إذا( pParse->کانو_نالا_سيٽ ) أرجع;
  /* Column names are determined by the left-most term of a compound select */
  طالما( pSelect->p_اڳواٽ ) pSelect = pSelect->p_اڳواٽ;
  TREETRACE(0x80,pParse,pSelect,("generating column names\n"));
  pTabList = pSelect->pSrc;
  pEList = pSelect->pEList;
  أكد( v!=0 );
  أكد( pTabList!=0 );
  pParse->کانو_نالا_سيٽ = 1;
  ڀريل_Name = (db->flags & SQLITE_ڀريل_کانو_نالا)!=0;
  srcName = (db->flags & SQLITE_Short_کانو_نالا)!=0 || ڀريل_Name;
  sqlite3_Vdbe_سيٽ_انگ_کانن_ڳڻپ(v, pEList->n_اظهار);
  لكل(i=0; i<pEList->n_اظهار; i++){
    اظهار *p = pEList->a[i].p_اظهار;

    أكد( p!=0 );
    أكد( p->op!=TK_AGG_ڪالم );  /* Agg processing has not run yet */
    أكد( p->op!=TK_ڪالم
        || (اظهار_Use_YTab(p) && p->y.pTab!=0) ); /* Covering idx not yet coded */
    إذا( pEList->a[i].zEName && pEList->a[i].fg.eEName==ENAME_NAME ){
      /* An AS clause always takes first priority */
      حرف *zName = pEList->a[i].zEName;
      sqlite3_Vdbe_سيٽ_کانو_Name(v, i, COLNAME_NAME, zName, SQLITE_TRANSIENT);
    }وإلا إذا( srcName && p->op==TK_ڪالم ){
      حرف *z_کانو;
      صحيح i_کانو = p->i_ڪالم;
      pTab = p->y.pTab;
      أكد( pTab!=0 );
      إذا( i_کانو<0 ) i_کانو = pTab->iPKey;
      أكد( i_کانو==-1 || (i_کانو>=0 && i_کانو<pTab->n_کانو) );
      إذا( i_کانو<0 ){
        z_کانو = "rowid";
      }وإلا{
        z_کانو = pTab->a_کانو[i_کانو].zCnName;
      }
      إذا( ڀريل_Name ){
        حرف *zName = 0;
        zName = sqlite3MPrintf(db, "%s.%s", pTab->zName, z_کانو);
        sqlite3_Vdbe_سيٽ_کانو_Name(v, i, COLNAME_NAME, zName, SQLITE_DYNAMIC);
      }وإلا{
        sqlite3_Vdbe_سيٽ_کانو_Name(v, i, COLNAME_NAME, z_کانو, SQLITE_TRANSIENT);
      }
    }وإلا{
      ثابت حرف *z = pEList->a[i].zEName;
      z = z==0 ? sqlite3MPrintf(db, "column%d", i+1) : sqlite3DbStrDup(db, z);
      sqlite3_Vdbe_سيٽ_کانو_Name(v, i, COLNAME_NAME, z, SQLITE_DYNAMIC);
    }
  }
  generate_ڪالم_Types(pParse, pTabList, pEList);
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
صحيح sqlite3_Columns_From_اظهار_List(
  Parse *pParse,          /* Parsing context */
  اظهار_List *pEList,       /* Expr list from which to derive column names */
  i16 *pn_کانو,             /* Write the number of columns here */
  ڪالم **pa_کانو          /* Write the new column list here */
){
  sqlite3 *db = pParse->db;   /* Database connection */
  صحيح i, j;                   /* Loop counters */
  u32 cnt;                    /* Index added to make the name unique */
  ڪالم *a_کانو, *p_کانو;        /* For looping over result columns */
  صحيح n_کانو;                   /* Number of columns in the result set */
  حرف *zName;                /* Column name */
  صحيح nName;                  /* Size of name in zName[] */
  Hash ht;                    /* Hash table of column names */
  جدول *pTab;

  sqlite3HashInit(&ht);
  إذا( pEList ){
    n_کانو = pEList->n_اظهار;
    a_کانو = sqlite3DbMallocZero(db, حجم(a_کانو[0])*n_کانو);
    testcase( a_کانو==0 );
    إذا( n_کانو>32767 ) n_کانو = 32767;
  }وإلا{
    n_کانو = 0;
    a_کانو = 0;
  }
  أكد( n_کانو==(i16)n_کانو );
  *pn_کانو = n_کانو;
  *pa_کانو = a_کانو;

  لكل(i=0, p_کانو=a_کانو; i<n_کانو && !pParse->nErr; i++, p_کانو++){
    بنية اظهار_List_شي *pX = &pEList->a[i];
    بنية اظهار_List_شي *pCollide;
    /* Get an appropriate name for the column
    */
    إذا( (zName = pX->zEName)!=0 && pX->fg.eEName==ENAME_NAME ){
      /* If the column contains an "AS <name>" phrase, use <name> as the name */
    }وإلا{
      اظهار *p_کانو_اظهار = sqlite3_اظهار_Skip_Collate_And_Likely(pX->p_اظهار);
      طالما( ALWAYS(p_کانو_اظهار!=0) && p_کانو_اظهار->op==TK_DOT ){
        p_کانو_اظهار = p_کانو_اظهار->p_ساڄو;
        أكد( p_کانو_اظهار!=0 );
      }
      إذا( p_کانو_اظهار->op==TK_ڪالم
       && ALWAYS( اظهار_Use_YTab(p_کانو_اظهار) )
       && ALWAYS( p_کانو_اظهار->y.pTab!=0 )
      ){
        /* For columns use the column name name */
        صحيح i_کانو = p_کانو_اظهار->i_ڪالم;
        pTab = p_کانو_اظهار->y.pTab;
        إذا( i_کانو<0 ) i_کانو = pTab->iPKey;
        zName = i_کانو>=0 ? pTab->a_کانو[i_کانو].zCnName : "rowid";
      }وإلا إذا( p_کانو_اظهار->op==TK_ID ){
        أكد( !اظهار_Has_Property(p_کانو_اظهار, EP_Int_قدر) );
        zName = p_کانو_اظهار->u.zToken;
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
    طالما( zName && (pCollide = sqlite3_Hash_ڳوليو(&ht, zName))!=0 ){
      إذا( pCollide->fg.b_Using_رڪن ){
        p_کانو->کانو_Flags |= COLFLAG_NOEXPAND;
      }
      nName = sqlite3Strlen30(zName);
      إذا( nName>0 ){
        لكل(j=nName-1; j>0 && sqlite3Isdigit(zName[j]); j--){}
        إذا( zName[j]==':' ) nName = j;
      }
      zName = sqlite3MPrintf(db, "%.*z:%u", nName, zName, ++cnt);
      sqlite3_ترقي_Check(pParse);
      إذا( cnt>3 ){
        sqlite3_randomness(حجم(cnt), &cnt);
      }
    }
    p_کانو->zCnName = zName;
    p_کانو->hName = sqlite3StrIHash(zName);
    إذا( pX->fg.bNoExpand ){
      p_کانو->کانو_Flags |= COLFLAG_NOEXPAND;
    }
    sqlite3_ڪالم_Properties_From_Name(0, p_کانو);
    إذا( zName && sqlite3_Hash_وجهو(&ht, zName, pX)==pX ){
      sqlite3OomFault(db);
    }
  }
  sqlite3HashClear(&ht);
  إذا( pParse->nErr ){
    لكل(j=0; j<i; j++){
      sqlite3DbFree(db, a_کانو[j].zCnName);
    }
    sqlite3DbFree(db, a_کانو);
    *pa_کانو = 0;
    *pn_کانو = 0;
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
فراغ sqlite3_Subquery_ڪالم_Types(
  Parse *pParse,      /* Parsing contexts */
  جدول *pTab,        /* Add column type information to this table */
  Select *pSelect,    /* SELECT used to determine types and collations */
  حرف aff            /* Default affinity. */
){
  sqlite3 *db = pParse->db;
  ڪالم *p_کانو;
  CollSeq *pColl;
  صحيح i,j;
  اظهار *p;
  بنية اظهار_List_شي *a;
  NameContext sNC;

  أكد( pSelect!=0 );
  أكد( (pSelect->selFlags & SF_Resolved)!=0 );
  أكد( pTab->n_کانو==pSelect->pEList->n_اظهار || pParse->nErr>0 );
  أكد( aff==SQLITE_AFF_NONE || aff==SQLITE_AFF_BLOB );
  إذا( db->mallocFailed || اندريون_RENAME_OBJECT ) أرجع;
  طالما( pSelect->p_اڳواٽ ) pSelect = pSelect->p_اڳواٽ;
  a = pSelect->pEList->a;
  memset(&sNC, 0, حجم(sNC));
  sNC.pSrcList = pSelect->pSrc;
  لكل(i=0, p_کانو=pTab->a_کانو; i<pTab->n_کانو; i++, p_کانو++){
    ثابت حرف *zType;
    i64 n;
    صحيح m = 0;
    Select *pS2 = pSelect;
    pTab->tabFlags |= (p_کانو->کانو_Flags & COLFLAG_NOINSERT);
    p = a[i].p_اظهار;
    /* pCol->szEst = ... // Column size est for SELECT tables never used */
    p_کانو->affinity = sqlite3_اظهار_Affinity(p);
    طالما( p_کانو->affinity<=SQLITE_AFF_NONE && pS2->p_ايندڙ!=0 ){
      m |= sqlite3_اظهار_معلومات_Type(pS2->pEList->a[i].p_اظهار);
      pS2 = pS2->p_ايندڙ;
      p_کانو->affinity = sqlite3_اظهار_Affinity(pS2->pEList->a[i].p_اظهار);
    }
    إذا( p_کانو->affinity<=SQLITE_AFF_NONE ){
      p_کانو->affinity = aff;
    }
    إذا( p_کانو->affinity>=SQLITE_AFF_متن && (pS2->p_ايندڙ || pS2!=pSelect) ){
      لكل(pS2=pS2->p_ايندڙ; pS2; pS2=pS2->p_ايندڙ){
        m |= sqlite3_اظهار_معلومات_Type(pS2->pEList->a[i].p_اظهار);
      }
      إذا( p_کانو->affinity==SQLITE_AFF_متن && (m&0x01)!=0 ){
        p_کانو->affinity = SQLITE_AFF_BLOB;
      }وإلا
      إذا( p_کانو->affinity>=SQLITE_AFF_NUMERIC && (m&0x02)!=0 ){
        p_کانو->affinity = SQLITE_AFF_BLOB;
      }
      إذا( p_کانو->affinity>=SQLITE_AFF_NUMERIC && p->op==TK_CAST ){
        p_کانو->affinity = SQLITE_AFF_FLEXNUM;
      }
    }
    zType = ڪالم_Type(&sNC, p, 0, 0, 0);
    إذا( zType==0 || p_کانو->affinity!=sqlite3AffinityType(zType, 0) ){
      إذا( p_کانو->affinity==SQLITE_AFF_NUMERIC
       || p_کانو->affinity==SQLITE_AFF_FLEXNUM
      ){
        zType = "NUM";
      }وإلا{
        zType = 0;
        لكل(j=1; j<SQLITE_N_STDTYPE; j++){
          إذا( sqlite3StdTypeAffinity[j]==p_کانو->affinity ){
            zType = sqlite3StdType[j];
            اكسر;
          }
        }
      }
    }
    إذا( zType ){
      ثابت i64 k = لڙي_ڊيگهه(zType);
      n = لڙي_ڊيگهه(p_کانو->zCnName);
      p_کانو->zCnName = sqlite3DbReallocOrFree(db, p_کانو->zCnName, n+k+2);
      p_کانو->کانو_Flags &= ~(COLFLAG_HASTYPE|COLFLAG_HASCOLL);
      إذا( p_کانو->zCnName ){
        يادگيري_نقل(&p_کانو->zCnName[n+1], zType, k+1);
        p_کانو->کانو_Flags |= COLFLAG_HASTYPE;
      }
    }
    pColl = sqlite3_اظهار_Coll_Seq(pParse, p);
    إذا( pColl ){
      أكد( pTab->p_اشاريو==0 );
      sqlite3_ڪالم_سيٽ_Coll(db, p_کانو, pColl->zName);
    }
  }
  pTab->sz_Tab_صف = 1; /* Any non-zero value works */
}

/*
** Check the current subquery nesting depth.  Return true and set an
** error if it has gone too deep.
*/
ساكن صحيح check_Subquery_Nesting_اونهائي(Parse *pParse){
#إذا SQLITE_MAX_اظهار_اونهائي>0
  إذا( pParse->nNestSel >= pParse->db->a_حد_بندي[SQLITE_حد_بندي_اظهار_اونهائي] ){
    sqlite3_Error_نياپو(pParse, "VIEWs and/or subqueries nested too deep");
    أرجع 1;
  }
#جيڪڏهن_پورو
  أرجع 0;
}

/*
** Given a SELECT statement, generate a Table structure that describes
** the result set of that SELECT.
*/
جدول *sqlite3_نتيجو_سيٽ_جو_Select(Parse *pParse, Select *pSelect, حرف aff){
  جدول *pTab;
  sqlite3 *db = pParse->db;
  u64 savedFlags;

  pParse->nNestSel++;
  إذا( check_Subquery_Nesting_اونهائي(pParse) ) أرجع 0;
  savedFlags = db->flags;
  db->flags &= ~(u64)SQLITE_ڀريل_کانو_نالا;
  db->flags |= SQLITE_Short_کانو_نالا;
  sqlite3SelectPrep(pParse, pSelect, 0);
  db->flags = savedFlags;
  إذا( pParse->nErr ) أرجع 0;
  طالما( pSelect->p_اڳواٽ ) pSelect = pSelect->p_اڳواٽ;
  pTab = sqlite3DbMallocZero(db, حجم(جدول) );
  إذا( pTab==0 ){
    أرجع 0;
  }
  pTab->nTabRef = 1;
  pTab->zName = 0;
  pTab->n_صف_Log_Est = 200; أكد( 200==sqlite3LogEst(1048576) );
  sqlite3_Columns_From_اظهار_List(pParse, pSelect->pEList, &pTab->n_کانو, &pTab->a_کانو);
  sqlite3_Subquery_ڪالم_Types(pParse, pTab, pSelect, aff);
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
    pParse->ok_Const_ضرب_ڪندڙ = 1;
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
ساكن فراغ compute_حد_بندي_Registers(Parse *pParse, Select *p, صحيح iBreak){
  Vdbe *v = 0;
  صحيح i_حد_بندي = 0;
  صحيح i_وٿي;
  صحيح n;
  اظهار *p_حد_بندي = p->p_حد_بندي;

  إذا( p->i_حد_بندي ) أرجع;

  /*
  ** "LIMIT -1" always shows all rows.  There is some
  ** controversy about what the correct behavior should be.
  ** The current implementation interprets "LIMIT 0" to mean
  ** no rows.
  */
  إذا( p_حد_بندي ){
    أكد( p_حد_بندي->op==TK_حد_بندي );
    أكد( p_حد_بندي->p_کاٻو!=0 );
    p->i_حد_بندي = i_حد_بندي = ++pParse->nMem;
    v = sqlite3GetVdbe(pParse);
    أكد( v!=0 );
    إذا( sqlite3_اظهار_Is_Integer(p_حد_بندي->p_کاٻو, &n, pParse, 0) ){
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, n, i_حد_بندي);
      VdbeComment((v, "LIMIT counter"));
      إذا( n==0 ){
        sqlite3VdbeGoto(v, iBreak);
      }وإلا إذا( n>=0 && p->n_Select_صف>sqlite3LogEst((u64)n) ){
        p->n_Select_صف = sqlite3LogEst((u64)n);
        p->selFlags |= SF_Fixed_حد_بندي;
      }
    }وإلا{
      sqlite3_اظهار_ڪوڊ(pParse, p_حد_بندي->p_کاٻو, i_حد_بندي);
      sqlite3_Vdbe_جوڙيو_Op1(v, OP_MustBeInt, i_حد_بندي); VdbeCoverage(v);
      VdbeComment((v, "LIMIT counter"));
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_IfNot, i_حد_بندي, iBreak); VdbeCoverage(v);
    }
    إذا( p_حد_بندي->p_ساڄو ){
      p->i_وٿي = i_وٿي = ++pParse->nMem;
      pParse->nMem++;   /* Allocate an extra register for limit+offset */
      sqlite3_اظهار_ڪوڊ(pParse, p_حد_بندي->p_ساڄو, i_وٿي);
      sqlite3_Vdbe_جوڙيو_Op1(v, OP_MustBeInt, i_وٿي); VdbeCoverage(v);
      sqlite3MayAbort(pParse);
      VdbeComment((v, "OFFSET counter"));
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_وٿي_حد_بندي, i_حد_بندي, i_وٿي+1, i_وٿي);
      VdbeComment((v, "LIMIT+OFFSET"));
    }
  }
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_COMPOUND_SELECT
/*
** Return the appropriate collating sequence for the iCol-th column of
** the result set for the compound-select statement "p".  Return NULL if
** the column has no default collating sequence.
**
** The collating sequence for the compound select is taken from the
** left-most term of the select that has a collating sequence.
*/
ساكن CollSeq *multiSelectCollSeq(Parse *pParse, Select *p, صحيح i_کانو){
  CollSeq *pRet;
  إذا( p->p_اڳواٽ ){
    pRet = multiSelectCollSeq(pParse, p->p_اڳواٽ, i_کانو);
  }وإلا{
    pRet = 0;
  }
  أكد( i_کانو>=0 );
  /* iCol must be less than p->pEList->nExpr.  Otherwise an error would
  ** have been thrown during name resolution and we would not have gotten
  ** this far */
  إذا( pRet==0 && ALWAYS(i_کانو<p->pEList->n_اظهار) ){
    pRet = sqlite3_اظهار_Coll_Seq(pParse, p->pEList->a[i_کانو].p_اظهار);
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
ساكن ڪنجي_Info *multi_Select_By_ملايو_ڪنجي_Info(Parse *pParse, Select *p, صحيح nExtra){
  اظهار_List *p_ترتيب_By = p->p_ترتيب_By;
  صحيح n_ترتيب_By = (p_ترتيب_By!=0) ? p_ترتيب_By->n_اظهار : 0;
  sqlite3 *db = pParse->db;
  ڪنجي_Info *pRet = sqlite3_ڪنجي_Info_Alloc(db, n_ترتيب_By+nExtra, 1);
  إذا( pRet ){
    صحيح i;
    لكل(i=0; i<n_ترتيب_By; i++){
      بنية اظهار_List_شي *p_شي = &p_ترتيب_By->a[i];
      اظهار *p_رڪن = p_شي->p_اظهار;
      CollSeq *pColl;

      إذا( p_رڪن->flags & EP_Collate ){
        pColl = sqlite3_اظهار_Coll_Seq(pParse, p_رڪن);
      }وإلا{
        pColl = multiSelectCollSeq(pParse, p, p_شي->u.x.i_ترتيب_By_کانو-1);
        إذا( pColl==0 ) pColl = db->pDfltColl;
        p_ترتيب_By->a[i].p_اظهار =
          sqlite3_اظهار_جوڙيو_Collate_String(pParse, p_رڪن, pColl->zName);
      }
      أكد( sqlite3_ڪنجي_Info_Is_Writeable(pRet) );
      pRet->aColl[i] = pColl;
      pRet->a_ترتيب_ڏيو_Flags[i] = p_ترتيب_By->a[i].fg.ترتيب_ڏيو_Flags;
    }
  }

  أرجع pRet;
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_CTE
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
  صحيح n_کانو = p->pEList->n_اظهار;  /* Number of columns in the recursive table */
  Vdbe *v = pParse->pVdbe;      /* The prepared statement under construction */
  Select *p_تياري;               /* The setup query */
  Select *p_پهريون_Rec;            /* Left-most recursive term */
  صحيح addr_چوٽي;                  /* Top of the loop */
  صحيح addrCont, addrBreak;      /* CONTINUE and BREAK addresses */
  صحيح iCurrent = 0;             /* The Current table */
  صحيح regCurrent;               /* Register holding Current table */
  صحيح i_قطار;                   /* The Queue table */
  صحيح iDistinct = 0;            /* To ensure unique results if UNION */
  صحيح eDest = SRT_Fifo;         /* How to write to Queue */
  SelectDest dest_قطار;         /* SelectDest targeting the Queue table */
  صحيح i;                        /* Loop counter */
  صحيح rc;                       /* Result code */
  اظهار_List *p_ترتيب_By;           /* The ORDER BY clause */
  اظهار *p_حد_بندي;                 /* Saved LIMIT and OFFSET */
  صحيح reg_حد_بندي, reg_وٿي;      /* Registers used by LIMIT and OFFSET */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
  إذا( p->pWin ){
    sqlite3_Error_نياپو(pParse, "cannot use window functions in recursive queries");
    أرجع;
  }
#جيڪڏهن_پورو

  /* Obtain authorization to do a recursive query */
  إذا( sqlite3AuthCheck(pParse, SQLITE_RECURSIVE, 0, 0, 0) ) أرجع;

  /* Process the LIMIT and OFFSET clauses, if they exist */
  addrBreak = sqlite3VdbeMakeLabel(pParse);
  p->n_Select_صف = 320;  /* 4 billion rows */
  compute_حد_بندي_Registers(pParse, p, addrBreak);
  p_حد_بندي = p->p_حد_بندي;
  reg_حد_بندي = p->i_حد_بندي;
  reg_وٿي = p->i_وٿي;
  p->p_حد_بندي = 0;
  p->i_حد_بندي = p->i_وٿي = 0;
  p_ترتيب_By = p->p_ترتيب_By;

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
  i_قطار = pParse->nTab++;
  إذا( p->op==TK_UNION ){
    eDest = p_ترتيب_By ? SRT_Dist_قطار : SRT_DistFifo;
    iDistinct = pParse->nTab++;
  }وإلا{
    eDest = p_ترتيب_By ? SRT_قطار : SRT_Fifo;
  }
  sqlite3SelectDestInit(&dest_قطار, eDest, i_قطار);

  /* Allocate cursors for Current, Queue, and Distinct. */
  regCurrent = ++pParse->nMem;
  sqlite3_Vdbe_جوڙيو_Op3(v, OP_OpenPseudo, iCurrent, regCurrent, n_کانو);
  إذا( p_ترتيب_By ){
    ڪنجي_Info *p_ڪنجي_Info = multi_Select_By_ملايو_ڪنجي_Info(pParse, p, 1);
    sqlite3_Vdbe_جوڙيو_Op4(v, OP_OpenEphemeral, i_قطار, p_ترتيب_By->n_اظهار+2, 0,
                      (حرف*)p_ڪنجي_Info, P4_KEYINFO);
    dest_قطار.p_ترتيب_By = p_ترتيب_By;
  }وإلا{
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_OpenEphemeral, i_قطار, n_کانو);
  }
  VdbeComment((v, "Queue table"));
  إذا( iDistinct ){
    /* Generate an ephemeral table used to enforce distinctness on the
    ** output of the recursive part of the CTE.
    */
    ڪنجي_Info *p_ڪنجي_Info;            /* Collating sequence for the result set */
    CollSeq **apColl;             /* For looping through pKeyInfo->aColl[] */

    أكد( p->p_ايندڙ==0 );
    أكد( p->pEList!=0 );
    n_کانو = p->pEList->n_اظهار;
    p_ڪنجي_Info = sqlite3_ڪنجي_Info_Alloc(pParse->db, n_کانو, 1);
    إذا( p_ڪنجي_Info ){
      لكل(i=0, apColl=p_ڪنجي_Info->aColl; i<n_کانو; i++, apColl++){
        *apColl = multiSelectCollSeq(pParse, p, i);
        إذا( 0==*apColl ){
          *apColl = pParse->db->pDfltColl;
        }
      }
      sqlite3_Vdbe_جوڙيو_Op4(v, OP_OpenEphemeral, iDistinct, n_کانو, 0,
                           (فراغ*)p_ڪنجي_Info, P4_KEYINFO);
    }وإلا{
      أكد( pParse->nErr>0 );
    }
  }

  /* Detach the ORDER BY clause from the compound SELECT */
  p->p_ترتيب_By = 0;

  /* Figure out how many elements of the compound SELECT are part of the
  ** recursive query.  Make sure no recursive elements use aggregate
  ** functions.  Mark the recursive elements as UNION ALL even if they
  ** are really UNION because the distinctness will be enforced by the
  ** iDistinct table.  pFirstRec is left pointing to the left-most
  ** recursive term of the CTE.
  */
  لكل(p_پهريون_Rec=p; ALWAYS(p_پهريون_Rec!=0); p_پهريون_Rec=p_پهريون_Rec->p_اڳواٽ){
    إذا( p_پهريون_Rec->selFlags & SF_Aggregate ){
      sqlite3_Error_نياپو(pParse, "recursive aggregate queries not supported");
      اذهب پڇاڙي_جو_recursive_query;
    }
    p_پهريون_Rec->op = TK_ALL;
    إذا( (p_پهريون_Rec->p_اڳواٽ->selFlags & SF_Recursive)==0 ) اكسر;
  }

  /* Store the results of the setup-query in Queue. */
  p_تياري = p_پهريون_Rec->p_اڳواٽ;
  p_تياري->p_ايندڙ = 0;
  ExplainQueryPlan((pParse, 1, "SETUP"));
  rc = sqlite3Select(pParse, p_تياري, &dest_قطار);
  p_تياري->p_ايندڙ = p;
  إذا( rc ) اذهب پڇاڙي_جو_recursive_query;

  /* Find the next row in the Queue and output that row */
  addr_چوٽي = sqlite3_Vdbe_جوڙيو_Op2(v, OP_Rewind, i_قطار, addrBreak); VdbeCoverage(v);

  /* Transfer the next row in Queue over to Current */
  sqlite3_Vdbe_جوڙيو_Op1(v, OP_Null_صف, iCurrent); /* To reset column cache */
  إذا( p_ترتيب_By ){
    sqlite3_Vdbe_جوڙيو_Op3(v, OP_ڪالم, i_قطار, p_ترتيب_By->n_اظهار+1, regCurrent);
  }وإلا{
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_صف_معلومات, i_قطار, regCurrent);
  }
  sqlite3_Vdbe_جوڙيو_Op1(v, OP_Delete, i_قطار);

  /* Output the single row in Current */
  addrCont = sqlite3VdbeMakeLabel(pParse);
  ڪوڊ_وٿي(v, reg_وٿي, addrCont);
  selectInnerLoop(pParse, p, iCurrent,
      0, 0, pDest, addrCont, addrBreak);
  إذا( reg_حد_بندي ){
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_DecrJumpZero, reg_حد_بندي, addrBreak);
    VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, addrCont);

  /* Execute the recursive SELECT taking the single row in Current as
  ** the value for the recursive-table. Store the results in the Queue.
  */
  p_پهريون_Rec->p_اڳواٽ = 0;
  ExplainQueryPlan((pParse, 1, "RECURSIVE STEP"));
  sqlite3Select(pParse, p, &dest_قطار);
  أكد( p_پهريون_Rec->p_اڳواٽ==0 );
  p_پهريون_Rec->p_اڳواٽ = p_تياري;

  /* Keep running the loop until the Queue is empty */
  sqlite3VdbeGoto(v, addr_چوٽي);
  sqlite3VdbeResolveLabel(v, addrBreak);

پڇاڙي_جو_recursive_query:
  sqlite3_اظهار_List_Delete(pParse->db, p->p_ترتيب_By);
  p->p_ترتيب_By = p_ترتيب_By;
  p->p_حد_بندي = p_حد_بندي;
  أرجع;
}
#جيڪڏهن_پورو /* SQLITE_OMIT_CTE */

/* Forward references */
ساكن صحيح multi_Select_By_ملايو(
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
ساكن صحيح multi_Select_قدرون(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  صحيح n_صف = 1;
  صحيح rc = 0;
  صحيح bShowAll = p->p_حد_بندي==0;
  أكد( p->selFlags & SF_Multi_قدر );
  افعل{
    أكد( p->selFlags & SF_قدرون );
    أكد( p->op==TK_ALL || (p->op==TK_SELECT && p->p_اڳواٽ==0) );
    أكد( p->p_ايندڙ==0 || p->pEList->n_اظهار==p->p_ايندڙ->pEList->n_اظهار );
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
    إذا( p->pWin ) أرجع -1;
#جيڪڏهن_پورو
    إذا( p->p_اڳواٽ==0 ) اكسر;
    أكد( p->p_اڳواٽ->p_ايندڙ==p );
    p = p->p_اڳواٽ;
    n_صف += bShowAll;
  }طالما(1);
  ExplainQueryPlan((pParse, 0, "SCAN %d CONSTANT ROW%s", n_صف,
                    n_صف==1 ? "" : "S"));
  طالما( p ){
    selectInnerLoop(pParse, p, -1, 0, 0, pDest, 1, 1);
    إذا( !bShowAll ) اكسر;
    p->n_Select_صف = n_صف;
    p = p->p_ايندڙ;
  }
  أرجع rc;
}

/*
** Return true if the SELECT statement which is known to be the recursive
** part of a recursive CTE still has its anchor terms attached.  If the
** anchor terms have already been removed, then return false.
*/
ساكن صحيح hasAnchor(Select *p){
  طالما( p && (p->selFlags & SF_Recursive)!=0 ){ p = p->p_اڳواٽ; }
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
ساكن صحيح union_With_حد_بندي_One(sqlite3 *db, Select *p){
  صحيح v;
  إذا( p->op!=TK_UNION ) أرجع 0;
  إذا( p->p_حد_بندي==0 ) أرجع 0;
  إذا( p->p_حد_بندي->p_ساڄو ) أرجع 0;  /* No OFFSET */
  v = 0;
  أكد( p->p_حد_بندي->p_کاٻو!=0 );
  إذا( sqlite3_اظهار_Is_Integer(p->p_حد_بندي->p_کاٻو, &v, 0, 0)==0 ) أرجع 0;
  إذا( v!=1 ) أرجع 0;
  إذا( OptimizationDisabled(db, SQLITE_Union_حد_بندي) ) أرجع 0;
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
  Select *p_اڳواٽ;       /* Another SELECT immediately to our left */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest dest;      /* Alternative data destination */
  Select *pDelete = 0;  /* Chain of simple selects to delete */
  sqlite3 *db;          /* Database connection */

  /* Make sure there is no ORDER BY or LIMIT clause on prior SELECTs.  Only
  ** the last (right-most) SELECT in the series may have an ORDER BY or LIMIT.
  */
  أكد( p && p->p_اڳواٽ );  /* Calling function guarantees this much */
  أكد( (p->selFlags & SF_Recursive)==0 || p->op==TK_ALL || p->op==TK_UNION );
  أكد( p->selFlags & SF_Compound );
  db = pParse->db;
  p_اڳواٽ = p->p_اڳواٽ;
  dest = *pDest;
  أكد( p_اڳواٽ->p_ترتيب_By==0 );
  أكد( p_اڳواٽ->p_حد_بندي==0 );

  v = sqlite3GetVdbe(pParse);
  أكد( v!=0 );  /* The VDBE already created by calling function */

  /* Create the destination temporary table if necessary
  */
  إذا( dest.eDest==SRT_EphemTab ){
    أكد( p->pEList );
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_OpenEphemeral, dest.iSDParm, p->pEList->n_اظهار);
    dest.eDest = SRT_جدول;
  }

  /* Special handling for a compound-select that originates as a VALUES clause.
  */
  إذا( p->selFlags & SF_Multi_قدر ){
    rc = multi_Select_قدرون(pParse, p, &dest);
    إذا( rc>=0 ) اذهب multi_select_پڇاڙي;
    rc = SQLITE_OK;
  }

  /* Make sure all SELECTs in the statement have the same number of elements
  ** in their result sets.
  */
  أكد( p->pEList && p_اڳواٽ->pEList );
  أكد( p->pEList->n_اظهار==p_اڳواٽ->pEList->n_اظهار );

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_CTE
  إذا( (p->selFlags & SF_Recursive)!=0 && hasAnchor(p) ){
    generateWithRecursiveQuery(pParse, p, &dest);
  }وإلا
#جيڪڏهن_پورو
  إذا( p->p_ترتيب_By ){
    /* If the compound has an ORDER BY clause, then always use the merge
    ** algorithm. */
    أرجع multi_Select_By_ملايو(pParse, p, pDest);
  }وإلا إذا( p->op!=TK_ALL && !union_With_حد_بندي_One(db,p) ){
    /* If the compound is EXCEPT, INTERSECT, or UNION (anything other than
    ** UNION ALL) then also always use the merge algorithm.  However, the
    ** multiSelectByMerge() routine requires that the compound have an
    ** ORDER BY clause, and it doesn't right now.  So invent one first. */
    اظهار *pOne = sqlite3_اظهار_Int32(db, 1);
    p->p_ترتيب_By = sqlite3_اظهار_List_Append(pParse, 0, pOne);
    إذا( pParse->nErr ) اذهب multi_select_پڇاڙي;
    أكد( p->p_ترتيب_By!=0 );
    p->p_ترتيب_By->a[0].u.x.i_ترتيب_By_کانو = 1;
    أرجع multi_Select_By_ملايو(pParse, p, pDest);
  }وإلا{
    /* For a UNION ALL compound without ORDER BY, or for a UNION with a 
    ** LIMIT of exactly 1 and no OFFSET and no ORDER BY, simply run the left
    ** query, then run the right query */
    صحيح addr = 0;
    صحيح n_حد_بندي = 0;  /* Initialize to suppress harmless compiler warning */

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_EXPLAIN
    إذا( p_اڳواٽ->p_اڳواٽ==0 ){
      ExplainQueryPlan((pParse, 1, "COMPOUND QUERY"));
      ExplainQueryPlan((pParse, 1, "LEFT-MOST SUBQUERY"));
    }
#جيڪڏهن_پورو
    أكد( !p_اڳواٽ->p_حد_بندي );
    p_اڳواٽ->i_حد_بندي = p->i_حد_بندي;
    p_اڳواٽ->i_وٿي = p->i_وٿي;
    p_اڳواٽ->p_حد_بندي = sqlite3_اظهار_Dup(db, p->p_حد_بندي, 0);
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL left...\n"));
    rc = sqlite3Select(pParse, p_اڳواٽ, &dest);
    sqlite3_اظهار_Delete(db, p_اڳواٽ->p_حد_بندي);
    p_اڳواٽ->p_حد_بندي = 0;
    إذا( rc ){
      اذهب multi_select_پڇاڙي;
    }
    p->p_اڳواٽ = 0;
    p->i_حد_بندي = p_اڳواٽ->i_حد_بندي;
    p->i_وٿي = p_اڳواٽ->i_وٿي;
    إذا( p->i_حد_بندي ){
      addr = sqlite3_Vdbe_جوڙيو_Op1(v, OP_IfNot, p->i_حد_بندي); VdbeCoverage(v);
      VdbeComment((v, "Jump ahead if LIMIT reached"));
      إذا( p->i_وٿي ){
        sqlite3_Vdbe_جوڙيو_Op3(v, OP_وٿي_حد_بندي,
                          p->i_حد_بندي, p->i_وٿي+1, p->i_وٿي);
      }
    }
    ExplainQueryPlan((pParse, 1, "UNION ALL"));
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL right...\n"));
    rc = sqlite3Select(pParse, p, &dest);
    testcase( rc!=SQLITE_OK );
    pDelete = p->p_اڳواٽ;
    p->p_اڳواٽ = p_اڳواٽ;
    p->n_Select_صف = sqlite3_Log_Est_جوڙيو(p->n_Select_صف, p_اڳواٽ->n_Select_صف);
    إذا( p->p_حد_بندي
     && sqlite3_اظهار_Is_Integer(p->p_حد_بندي->p_کاٻو, &n_حد_بندي, pParse, 0)
     && n_حد_بندي>0 && p->n_Select_صف > sqlite3LogEst((u64)n_حد_بندي)
    ){
      p->n_Select_صف = sqlite3LogEst((u64)n_حد_بندي);
    }
    إذا( addr ){
      sqlite3_Vdbe_Jump_هتي(v, addr);
    }
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_EXPLAIN
    إذا( p->p_ايندڙ==0 ){
      Explain_Query_Plan_ڪڍو(pParse);
    }
#جيڪڏهن_پورو
  }

multi_select_پڇاڙي:
  pDest->iSdst = dest.iSdst;
  pDest->nSdst = dest.nSdst;
  pDest->iSDParm2 = dest.iSDParm2;
  إذا( pDelete ){
    sqlite3_Parser_جوڙيو_Cleanup(pParse, sqlite3SelectDeleteGeneric, pDelete);
  }
  أرجع rc;
}
#جيڪڏهن_پورو /* SQLITE_OMIT_COMPOUND_SELECT */

/*
** Error message for when two or more terms of a compound select have different
** size result sets.
*/
فراغ sqlite3_Select_Wrong_انگ_Terms_Error(Parse *pParse, Select *p){
  إذا( p->selFlags & SF_قدرون ){
    sqlite3_Error_نياپو(pParse, "all VALUES must have the same number of terms");
  }وإلا{
    sqlite3_Error_نياپو(pParse, "SELECTs to the left and right of %s"
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
  SelectDest *p_اندريون,        /* Coroutine supplying data */
  SelectDest *pDest,      /* Where to send the data */
  صحيح regReturn,          /* The return address register */
  صحيح regPrev,            /* Previous result register.  No uniqueness if 0 */
  ڪنجي_Info *p_ڪنجي_Info,      /* For comparing with previous entry */
  صحيح iBreak              /* Jump here if we hit the LIMIT */
){
  Vdbe *v = pParse->pVdbe;
  صحيح iContinue;
  صحيح addr;

  أكد( p_اندريون->eDest==SRT_Coroutine );

  addr = sqlite3VdbeCurrentAddr(v);
  iContinue = sqlite3VdbeMakeLabel(pParse);

  /* Suppress duplicates for UNION, EXCEPT, and INTERSECT
  */
  إذا( regPrev ){
    صحيح addr1, addr2;
    addr1 = sqlite3_Vdbe_جوڙيو_Op1(v, OP_IfNot, regPrev); VdbeCoverage(v);
    addr2 = sqlite3_Vdbe_جوڙيو_Op4(v, OP_Compare, p_اندريون->iSdst, regPrev+1, p_اندريون->nSdst,
                              (حرف*)sqlite3_ڪنجي_Info_Ref(p_ڪنجي_Info), P4_KEYINFO);
    sqlite3_Vdbe_جوڙيو_Op3(v, OP_Jump, addr2+2, iContinue, addr2+2); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_هتي(v, addr1);
    sqlite3_Vdbe_جوڙيو_Op3(v, OP_Copy, p_اندريون->iSdst, regPrev+1, p_اندريون->nSdst-1);
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, 1, regPrev);
  }
  إذا( pParse->db->mallocFailed ) أرجع 0;

  /* Suppress the first OFFSET entries if there is an OFFSET clause
  */
  ڪوڊ_وٿي(v, p->i_وٿي, iContinue);

  بدل( pDest->eDest ){
    /* Store the result as data using a unique key.
    */
    حالة SRT_Fifo:
    حالة SRT_DistFifo:
    حالة SRT_جدول:
    حالة SRT_EphemTab: {
      صحيح r1 = sqlite3_Get_عارضي_Reg(pParse);
      صحيح r2 = sqlite3_Get_عارضي_Reg(pParse);
      صحيح iParm = pDest->iSDParm;
      testcase( pDest->eDest==SRT_جدول );
      testcase( pDest->eDest==SRT_EphemTab );
      testcase( pDest->eDest==SRT_Fifo );
      testcase( pDest->eDest==SRT_DistFifo );
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Make_رڪارڊ, p_اندريون->iSdst, p_اندريون->nSdst, r1);
#إذا !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      إذا( pDest->eDest==SRT_جدول && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_CTE
      إذا( pDest->eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index that is used to enforce uniqueness on the
        ** total result.  At this point, we are processing the setup portion
        ** of the recursive CTE using the merge algorithm, so the results are
        ** guaranteed to be unique anyhow.  But we still need to populate the
        ** (iParm+1) cursor for use by the subsequent recursive phase.
        */
        sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_Idx_وجهو, iParm+1, r1,
                             p_اندريون->iSdst, p_اندريون->nSdst);
      }
#جيڪڏهن_پورو
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_NewRowid, iParm, r2);
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_وجهو, iParm, r1, r2);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      sqlite3_Release_عارضي_Reg(pParse, r2);
      sqlite3_Release_عارضي_Reg(pParse, r1);
      اكسر;
    }

    /* If any row exist in the result set, record that fact and abort.
    */
    حالة SRT_Exists: {
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, 1, pDest->iSDParm);
      /* The LIMIT clause will terminate the loop for us */
      اكسر;
    }

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)".
    */
    حالة SRT_سيٽ: {
      صحيح r1;
      testcase( p_اندريون->nSdst>1 );
      r1 = sqlite3_Get_عارضي_Reg(pParse);
      sqlite3_Vdbe_جوڙيو_Op4(v, OP_Make_رڪارڊ, p_اندريون->iSdst, p_اندريون->nSdst,
          r1, pDest->zAffSdst, p_اندريون->nSdst);
      sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_Idx_وجهو, pDest->iSDParm, r1,
                           p_اندريون->iSdst, p_اندريون->nSdst);
      إذا( pDest->iSDParm2>0 ){
        أكد( sqlite3CursorBloomable(pParse, pDest->iSDParm, p_اندريون->nSdst) );
        sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_Filter_جوڙيو, pDest->iSDParm2, 0,
                             p_اندريون->iSdst, p_اندريون->nSdst);
        ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
      }
      sqlite3_Release_عارضي_Reg(pParse, r1);
      اكسر;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell and break out
    ** of the scan loop.  Note that the select might return multiple columns
    ** if it is the RHS of a row-value IN operator.
    */
    حالة SRT_Mem: {
      testcase( p_اندريون->nSdst>1 );
      sqlite3_اظهار_ڪوڊ_Move(pParse, p_اندريون->iSdst, pDest->iSDParm, p_اندريون->nSdst);
      /* The LIMIT clause will jump out of the loop for us */
      اكسر;
    }
#جيڪڏهن_پورو /* #ifndef SQLITE_OMIT_SUBQUERY */

    /* The results are stored in a sequence of registers
    ** starting at pDest->iSdst.  Then the co-routine yields.
    */
    حالة SRT_Coroutine: {
      إذا( pDest->iSdst==0 ){
        pDest->iSdst = sqlite3_Get_عارضي_Range(pParse, p_اندريون->nSdst);
        pDest->nSdst = p_اندريون->nSdst;
      }
      sqlite3_اظهار_ڪوڊ_Move(pParse, p_اندريون->iSdst, pDest->iSdst, p_اندريون->nSdst);
      sqlite3_Vdbe_جوڙيو_Op1(v, OP_Yield, pDest->iSDParm);
      اكسر;
    }

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    حالة SRT_Dist_قطار:
    حالة SRT_قطار: {
      صحيح n_ڪنجي;
      صحيح r1, r2, r3, ii;
      اظهار_List *pSO;
      صحيح iParm = pDest->iSDParm;
      pSO = pDest->p_ترتيب_By;
      أكد( pSO );
      n_ڪنجي = pSO->n_اظهار;
      r1 = sqlite3_Get_عارضي_Reg(pParse);
      r2 = sqlite3_Get_عارضي_Range(pParse, n_ڪنجي+2);
      r3 = r2+n_ڪنجي+1;

      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Make_رڪارڊ, p_اندريون->iSdst, p_اندريون->nSdst, r3);
      إذا( pDest->eDest==SRT_Dist_قطار ){
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_Idx_وجهو, iParm+1, r3);
      }
      لكل(ii=0; ii<n_ڪنجي; ii++){
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_SCopy,
                          p_اندريون->iSdst + pSO->a[ii].u.x.i_ترتيب_By_کانو - 1,
                          r2+ii);
      }
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Sequence, iParm, r2+n_ڪنجي);
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Make_رڪارڊ, r2, n_ڪنجي+2, r1);
      sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_Idx_وجهو, iParm, r1, r2, n_ڪنجي+2);
      sqlite3_Release_عارضي_Reg(pParse, r1);
      sqlite3_Release_عارضي_Range(pParse, r2, n_ڪنجي+2);
      اكسر;
    }
#جيڪڏهن_پورو /* SQLITE_OMIT_CTE */

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
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_نتيجو_صف, p_اندريون->iSdst, p_اندريون->nSdst);
      اكسر;
    }
  }

  /* Jump to the end of the loop if the LIMIT is reached.
  */
  إذا( p->i_حد_بندي ){
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_DecrJumpZero, p->i_حد_بندي, iBreak); VdbeCoverage(v);
  }

  /* Generate the subroutine return
  */
  sqlite3VdbeResolveLabel(v, iContinue);
  sqlite3_Vdbe_جوڙيو_Op1(v, OP_Return, regReturn);

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
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_COMPOUND_SELECT
ساكن صحيح multi_Select_By_ملايو(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  صحيح i, j;             /* Loop counters */
  Select *p_اڳواٽ;       /* Another SELECT immediately to our left */
  Select *p_ورهايو;       /* Left-most SELECT in the right-hand group */
  صحيح nSelect;          /* Number of SELECT statements in the compound */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest destA;     /* Destination for coroutine A */
  SelectDest destB;     /* Destination for coroutine B */
  صحيح regAddrA;         /* Address register for select-A coroutine */
  صحيح regAddrB;         /* Address register for select-B coroutine */
  صحيح addrSelectA;      /* Address of the select-A coroutine */
  صحيح addrSelectB;      /* Address of the select-B coroutine */
  صحيح reg_ٻاهريون_A;          /* Address register for the output-A subroutine */
  صحيح reg_ٻاهريون_B;          /* Address register for the output-B subroutine */
  صحيح addr_ٻاهريون_A;         /* Address of the output-A subroutine */
  صحيح addr_ٻاهريون_B = 0;     /* Address of the output-B subroutine */
  صحيح addrEofA;         /* Address of the select-A-exhausted subroutine */
  صحيح addrEofA_noB;     /* Alternate addrEofA if B is uninitialized */
  صحيح addrEofB;         /* Address of the select-B-exhausted subroutine */
  صحيح addrAltB;         /* Address of the A<B subroutine */
  صحيح addrAeqB;         /* Address of the A==B subroutine */
  صحيح addrAgtB;         /* Address of the A>B subroutine */
  صحيح reg_حد_بندي_A;        /* Limit register for select-A */
  صحيح reg_حد_بندي_B;        /* Limit register for select-A */
  صحيح regPrev;          /* A range of registers to hold previous output */
  صحيح saved_حد_بندي;       /* Saved value of p->iLimit */
  صحيح saved_وٿي;      /* Saved value of p->iOffset */
  صحيح labelCmpr;        /* Label for the start of the merge algorithm */
  صحيح label_پڇاڙي;         /* Label for the end of the overall SELECT stmt */
  صحيح addr1;            /* Jump instructions that get retargeted */
  صحيح op;               /* One of TK_ALL, TK_UNION, TK_EXCEPT, TK_INTERSECT */
  ڪنجي_Info *p_ڪنجي_Dup = 0; /* Comparison information for duplicate removal */
  ڪنجي_Info *p_ڪنجي_ملايو;   /* Comparison information for merging rows */
  sqlite3 *db;          /* Database connection */
  اظهار_List *p_ترتيب_By;   /* The ORDER BY clause */
  صحيح n_ترتيب_By;         /* Number of terms in the ORDER BY clause */
  u32 *a_ترتيب_بدلايو;        /* Mapping from ORDER BY terms to result set columns */

  أكد( p->p_ترتيب_By!=0 );
  أكد( p_ڪنجي_Dup==0 ); /* "Managed" code needs this.  Ticket #3382. */
  db = pParse->db;
  v = pParse->pVdbe;
  أكد( v!=0 );       /* Already thrown the error if VDBE alloc failed */
  label_پڇاڙي = sqlite3VdbeMakeLabel(pParse);
  labelCmpr = sqlite3VdbeMakeLabel(pParse);


  /* Patch up the ORDER BY clause
  */
  op = p->op; 
  أكد( p->p_اڳواٽ->p_ترتيب_By==0 );
  p_ترتيب_By = p->p_ترتيب_By;
  أكد( p_ترتيب_By );
  n_ترتيب_By = p_ترتيب_By->n_اظهار;

  /* For operators other than UNION ALL we have to make sure that
  ** the ORDER BY clause covers every term of the result set.  Add
  ** terms to the ORDER BY clause as necessary.  Do the same for UNION ALL
  ** if any operator to the left is not UNION ALL, so that the ORDER BY
  ** carries the collation of every column into that sub-compound.
  */
  لكل(p_اڳواٽ=p; p_اڳواٽ && p_اڳواٽ->op==TK_ALL; p_اڳواٽ=p_اڳواٽ->p_اڳواٽ){}
  إذا( p_اڳواٽ && p_اڳواٽ->p_اڳواٽ ){
    لكل(i=1; db->mallocFailed==0 && i<=p->pEList->n_اظهار; i++){
      بنية اظهار_List_شي *p_شي;
      لكل(j=0, p_شي=p_ترتيب_By->a; j<n_ترتيب_By; j++, p_شي++){
        أكد( p_شي!=0 );
        أكد( p_شي->u.x.i_ترتيب_By_کانو>0 );
        إذا( p_شي->u.x.i_ترتيب_By_کانو==i ) اكسر;
      }
      إذا( j==n_ترتيب_By ){
        اظهار *pNew = sqlite3_اظهار_Int32(db, i);
        إذا( pNew==0 ) أرجع SQLITE_NOMEM_BKPT;
        p->p_ترتيب_By = p_ترتيب_By = sqlite3_اظهار_List_Append(pParse, p_ترتيب_By, pNew);
        إذا( p_ترتيب_By ) p_ترتيب_By->a[n_ترتيب_By++].u.x.i_ترتيب_By_کانو = (u16)i;
      }
    }
  }

  /* Compute the comparison permutation and keyinfo that is used with
  ** the permutation to determine if the next row of results comes
  ** from selectA or selectB.  Also add literal collations to the
  ** ORDER BY clause terms so that when selectA and selectB are
  ** evaluated, they use the correct collation.
  */
  a_ترتيب_بدلايو = sqlite3DbMallocRawNN(db, حجم(u32)*(n_ترتيب_By + 1));
  إذا( a_ترتيب_بدلايو ){
    بنية اظهار_List_شي *p_شي;
    صحيح bKeep = 0;
    a_ترتيب_بدلايو[0] = n_ترتيب_By;
    لكل(i=1, p_شي=p_ترتيب_By->a; i<=n_ترتيب_By; i++, p_شي++){
      أكد( p_شي!=0 );
      أكد( p_شي->u.x.i_ترتيب_By_کانو>0 );
      أكد( p_شي->u.x.i_ترتيب_By_کانو<=p->pEList->n_اظهار );
      a_ترتيب_بدلايو[i] = p_شي->u.x.i_ترتيب_By_کانو - 1;
      إذا( a_ترتيب_بدلايو[i]!=(u32)i-1 ) bKeep = 1;
    }
     إذا( bKeep==0 ){
      sqlite3DbFreeNN(db, a_ترتيب_بدلايو);
      a_ترتيب_بدلايو = 0;
    }
  }
  p_ڪنجي_ملايو = multi_Select_By_ملايو_ڪنجي_Info(pParse, p, 1);

  /* Allocate a range of temporary registers and the KeyInfo needed
  ** for the logic that removes duplicate result rows when the
  ** operator is UNION, EXCEPT, or INTERSECT (but not UNION ALL).
  */
  إذا( op==TK_ALL ){
    regPrev = 0;
  }وإلا{
    صحيح n_اظهار = p->pEList->n_اظهار;
    أكد( n_ترتيب_By>=n_اظهار || db->mallocFailed );
    regPrev = pParse->nMem+1;
    pParse->nMem += n_اظهار+1;
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, 0, regPrev);
    p_ڪنجي_Dup = sqlite3_ڪنجي_Info_Alloc(db, n_اظهار, 1);
    إذا( p_ڪنجي_Dup ){
      أكد( sqlite3_ڪنجي_Info_Is_Writeable(p_ڪنجي_Dup) );
      /* Use the ORDER BY collations, not multiSelectCollSeq(p), because
      ** p might be just one part of a larger compound, in which case
      ** the ORDER BY carries the COLLATE of the larger compound. */
      لكل(i=0; ALWAYS(p_ڪنجي_ملايو) && i<n_ترتيب_By; i++){
        p_ڪنجي_Dup->aColl[p_ترتيب_By->a[i].u.x.i_ترتيب_By_کانو-1] = p_ڪنجي_ملايو->aColl[i];
      }
    }
  }

  /* Separate the left and the right query from one another
  */
  nSelect = 1;
  إذا( (op==TK_ALL || op==TK_UNION)
   && OptimizationEnabled(db, SQLITE_متوازن_ملايو)
  ){
    لكل(p_ورهايو=p; p_ورهايو->p_اڳواٽ!=0 && p_ورهايو->op==op; p_ورهايو=p_ورهايو->p_اڳواٽ){
      nSelect++;
      أكد( p_ورهايو->p_اڳواٽ->p_ايندڙ==p_ورهايو );
    }
  }
  إذا( nSelect<=3 ){
    p_ورهايو = p;
  }وإلا{
    p_ورهايو = p;
    لكل(i=2; i<nSelect; i+=2){ p_ورهايو = p_ورهايو->p_اڳواٽ; }
  }
  p_اڳواٽ = p_ورهايو->p_اڳواٽ;
  أكد( p_اڳواٽ!=0 );
  p_ورهايو->p_اڳواٽ = 0;
  p_اڳواٽ->p_ايندڙ = 0;
  أكد( p->p_ترتيب_By == p_ترتيب_By );
  أكد( p_ترتيب_By!=0 || db->mallocFailed );
  p_اڳواٽ->p_ترتيب_By = sqlite3_اظهار_List_Dup(pParse->db, p_ترتيب_By, 0);
  sqlite3_Resolve_ترتيب_گروهه_By(pParse, p, p->p_ترتيب_By, "ORDER");
  sqlite3_Resolve_ترتيب_گروهه_By(pParse, p_اڳواٽ, p_اڳواٽ->p_ترتيب_By, "ORDER");

  /* Compute the limit registers */
  compute_حد_بندي_Registers(pParse, p, label_پڇاڙي);
  إذا( p->i_حد_بندي && (op==TK_ALL || union_With_حد_بندي_One(db,p)) ){
    reg_حد_بندي_A = ++pParse->nMem;
    reg_حد_بندي_B = ++pParse->nMem;
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_Copy, p->i_وٿي ? p->i_وٿي+1 : p->i_حد_بندي,
                                  reg_حد_بندي_A);
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_Copy, reg_حد_بندي_A, reg_حد_بندي_B);
  }وإلا{
    reg_حد_بندي_A = reg_حد_بندي_B = 0;
  }
  sqlite3_اظهار_Delete(db, p->p_حد_بندي);
  p->p_حد_بندي = 0;

  regAddrA = ++pParse->nMem;
  regAddrB = ++pParse->nMem;
  reg_ٻاهريون_A = ++pParse->nMem;
  reg_ٻاهريون_B = ++pParse->nMem;
  sqlite3SelectDestInit(&destA, SRT_Coroutine, regAddrA);
  sqlite3SelectDestInit(&destB, SRT_Coroutine, regAddrB);

  ExplainQueryPlan((pParse, 1, "MERGE (%s)", sqlite3SelectOpName(p->op)));

  /* Generate a coroutine to evaluate the SELECT statement to the
  ** left of the compound operator - the "A" select.
  */
  addrSelectA = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3_Vdbe_جوڙيو_Op3(v, OP_InitCoroutine, regAddrA, 0, addrSelectA);
  VdbeComment((v, "SUBR: next-A"));
  p_اڳواٽ->i_حد_بندي = reg_حد_بندي_A;
  ExplainQueryPlan((pParse, 1, "LEFT"));
  sqlite3Select(pParse, p_اڳواٽ, &destA);
  sqlite3_Vdbe_پڇاڙي_Coroutine(v, regAddrA);
  sqlite3_Vdbe_Jump_هتي(v, addr1);

  /* Generate a coroutine to evaluate the SELECT statement on
  ** the right - the "B" select
  */
  addrSelectB = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3_Vdbe_جوڙيو_Op3(v, OP_InitCoroutine, regAddrB, 0, addrSelectB);
  VdbeComment((v, "SUBR: next-B"));
  saved_حد_بندي = p->i_حد_بندي;
  saved_وٿي = p->i_وٿي;
  p->i_حد_بندي = reg_حد_بندي_B;
  p->i_وٿي = 0; 
  ExplainQueryPlan((pParse, 1, "RIGHT"));
  sqlite3Select(pParse, p, &destB);
  p->i_حد_بندي = saved_حد_بندي;
  p->i_وٿي = saved_وٿي;
  sqlite3_Vdbe_پڇاڙي_Coroutine(v, regAddrB);

  /* Generate a subroutine that outputs the current row of the A
  ** select as the next output row of the compound select.
  */
  VdbeNoopComment((v, "SUBR: out-A"));
  addr_ٻاهريون_A = generateOutputSubroutine(pParse,
                 p, &destA, pDest, reg_ٻاهريون_A,
                 regPrev, p_ڪنجي_Dup, label_پڇاڙي);
 
  /* Generate a subroutine that outputs the current row of the B
  ** select as the next output row of the compound select.
  */
  إذا( op==TK_ALL || op==TK_UNION ){
    VdbeNoopComment((v, "SUBR: out-B"));
    addr_ٻاهريون_B = generateOutputSubroutine(pParse,
                 p, &destB, pDest, reg_ٻاهريون_B,
                 regPrev, p_ڪنجي_Dup, label_پڇاڙي);
  }
  sqlite3_ڪنجي_Info_Unref(p_ڪنجي_Dup);

  /* Generate a subroutine to run when the results from select A
  ** are exhausted and only data in select B remains.
  */
  إذا( op==TK_EXCEPT || op==TK_INTERSECT ){
    addrEofA_noB = addrEofA = label_پڇاڙي;
  }وإلا{ 
    VdbeNoopComment((v, "SUBR: eof-A"));
    addrEofA = sqlite3_Vdbe_جوڙيو_Op2(v, OP_Gosub, reg_ٻاهريون_B, addr_ٻاهريون_B);
    VdbeComment((v, "out-B"));
    addrEofA_noB = sqlite3_Vdbe_جوڙيو_Op2(v, OP_Yield, regAddrB, label_پڇاڙي);
                                     VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, addrEofA);
    p->n_Select_صف = sqlite3_Log_Est_جوڙيو(p->n_Select_صف, p_اڳواٽ->n_Select_صف);
  }

  /* Generate a subroutine to run when the results from select B
  ** are exhausted and only data in select A remains.
  */
  إذا( op==TK_INTERSECT ){
    addrEofB = addrEofA;
    إذا( p->n_Select_صف > p_اڳواٽ->n_Select_صف ) p->n_Select_صف = p_اڳواٽ->n_Select_صف;
  }وإلا{ 
    VdbeNoopComment((v, "SUBR: eof-B"));
    addrEofB = sqlite3_Vdbe_جوڙيو_Op2(v, OP_Gosub, reg_ٻاهريون_A, addr_ٻاهريون_A);
    VdbeComment((v, "out-A"));
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_Yield, regAddrA, label_پڇاڙي); VdbeCoverage(v);
    VdbeComment((v, "next-A"));
    sqlite3VdbeGoto(v, addrEofB);
  }

  /* Generate code to handle the case of A<B
  */
  addrAltB = sqlite3_Vdbe_جوڙيو_Op2(v, OP_Gosub, reg_ٻاهريون_A, addr_ٻاهريون_A);
  VdbeComment((v, "out-A"));
  sqlite3_Vdbe_جوڙيو_Op2(v, OP_Yield, regAddrA, addrEofA); VdbeCoverage(v);
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
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_Gosub, reg_ٻاهريون_B, addr_ٻاهريون_B);
    VdbeComment((v, "out-B"));
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, labelCmpr);
  }وإلا{
    addrAgtB++; /* Just do next-B.  Might as well use the next-B call
                ** in the next code block */
  }

  /* This code runs once to initialize everything.
  */
  sqlite3_Vdbe_Jump_هتي(v, addr1);
  sqlite3_Vdbe_جوڙيو_Op2(v, OP_Yield, regAddrA, addrEofA_noB); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  /* v---  Also the A>B case for EXCEPT and INTERSECT */
  sqlite3_Vdbe_جوڙيو_Op2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
  VdbeComment((v, "next-B"));

  /* Implement the main merge loop
  */
  إذا( a_ترتيب_بدلايو!=0 ){
    sqlite3_Vdbe_جوڙيو_Op4(v, OP_Permutation, 0, 0, 0, (حرف*)a_ترتيب_بدلايو, P4_INTARRAY);
  }
  sqlite3VdbeResolveLabel(v, labelCmpr);
  sqlite3_Vdbe_جوڙيو_Op4(v, OP_Compare, destA.iSdst, destB.iSdst, n_ترتيب_By,
                         (حرف*)p_ڪنجي_ملايو, P4_KEYINFO);
  إذا( a_ترتيب_بدلايو!=0 ){
    sqlite3VdbeChangeP5(v, OPFLAG_ترتيب_بدلايو);
  }
  sqlite3_Vdbe_جوڙيو_Op3(v, OP_Jump, addrAltB, addrAeqB, addrAgtB);
  VdbeCoverageIf(v, op==TK_ALL);
  VdbeCoverageIf(v, op==TK_UNION);
  VdbeCoverageIf(v, op==TK_EXCEPT);
  VdbeCoverageIf(v, op==TK_INTERSECT);

  /* Jump to the this point in order to terminate the query.
  */
  sqlite3VdbeResolveLabel(v, label_پڇاڙي);

  /* Make arrangements to free the 2nd and subsequent arms of the compound
  ** after the parse has finished */
  إذا( p_ورهايو->p_اڳواٽ ){
    sqlite3_Parser_جوڙيو_Cleanup(pParse, sqlite3SelectDeleteGeneric, p_ورهايو->p_اڳواٽ);
  }
  p_ورهايو->p_اڳواٽ = p_اڳواٽ;
  p_اڳواٽ->p_ايندڙ = p_ورهايو;
  sqlite3_اظهار_List_Delete(db, p_اڳواٽ->p_ترتيب_By);
  p_اڳواٽ->p_ترتيب_By = 0;

  /*** TBD:  Insert subroutine calls to close cursors on incomplete
  **** subqueries ****/
  Explain_Query_Plan_ڪڍو(pParse);
  أرجع pParse->nErr!=0;
}
#جيڪڏهن_پورو

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
  صحيح n_Sel_اونهائي;            /* Depth of sub-query recursion.  Top==1 */
  اظهار_List *pEList;         /* Replacement expressions */
  اظهار_List *pCList;         /* Collation sequences for replacement expr */
} SubstContext;

/* Forward Declarations */
ساكن فراغ subst_اظهار_List(SubstContext*, اظهار_List*);
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
ساكن اظهار *subst_اظهار(
  SubstContext *pSubst,  /* Description of the substitution */
  اظهار *p_اظهار            /* Expr in which substitution occurs */
){
  إذا( p_اظهار==0 ) أرجع 0;
  إذا( اظهار_Has_Property(p_اظهار, EP_OuterON|EP_InnerON)
   && p_اظهار->w.iJoin==pSubst->i_جدول
  ){
    testcase( اظهار_Has_Property(p_اظهار, EP_InnerON) );
    p_اظهار->w.iJoin = pSubst->i_New_جدول;
  }
  إذا( p_اظهار->op==TK_ڪالم
   && p_اظهار->i_جدول==pSubst->i_جدول
   && !اظهار_Has_Property(p_اظهار, EP_Fixed_کانو)
  ){
#جيڪڏهن_وصف_ٿيل SQLITE_ALLOW_ROWID_اندريون_VIEW
    إذا( p_اظهار->i_ڪالم<0 ){
      p_اظهار->op = TK_NULL;
    }وإلا
#جيڪڏهن_پورو
    {
      اظهار *pNew;
      صحيح i_ڪالم;
      اظهار *pCopy;
      اظهار if_Null_صف;
      i_ڪالم = p_اظهار->i_ڪالم;
      أكد( i_ڪالم>=0 );
      أكد( pSubst->pEList!=0 && i_ڪالم<pSubst->pEList->n_اظهار );
      أكد( p_اظهار->p_ساڄو==0 );
      pCopy = pSubst->pEList->a[i_ڪالم].p_اظهار;
      إذا( sqlite3_اظهار_Is_ويڪٽر(pCopy) ){
        sqlite3_ويڪٽر_Error_نياپو(pSubst->pParse, pCopy);
      }وإلا{
        sqlite3 *db = pSubst->pParse->db;
        إذا( pSubst->isOuterJoin
         && (pCopy->op!=TK_ڪالم || pCopy->i_جدول!=pSubst->i_New_جدول)
        ){
          memset(&if_Null_صف, 0, حجم(if_Null_صف));
          if_Null_صف.op = TK_IF_NULL_صف;
          if_Null_صف.p_کاٻو = pCopy;
          if_Null_صف.i_جدول = pSubst->i_New_جدول;
          if_Null_صف.i_ڪالم = -99;
          if_Null_صف.flags = EP_If_Null_صف;
          pCopy = &if_Null_صف;
        }
        testcase( اظهار_Has_Property(pCopy, EP_Subquery) );
        pNew = sqlite3_اظهار_Dup(db, pCopy, 0);
        إذا( db->mallocFailed ){
          sqlite3_اظهار_Delete(db, pNew);
          أرجع p_اظهار;
        }
        إذا( pSubst->isOuterJoin ){
          اظهار_سيٽ_Property(pNew, EP_CanBeNull);
        }
        إذا( pNew->op==TK_TRUEFALSE ){
          pNew->u.i_قدر = sqlite3_اظهار_Truth_قدر(pNew);
          pNew->op = TK_INTEGER;
          اظهار_سيٽ_Property(pNew, EP_Int_قدر);
        }

        /* Ensure that the expression now has an implicit collation sequence,
        ** just as it did when it was a column of a view or sub-query. */
        {
          CollSeq *pNat = sqlite3_اظهار_Coll_Seq(pSubst->pParse, pNew);
          CollSeq *pColl = sqlite3_اظهار_Coll_Seq(pSubst->pParse,
                pSubst->pCList->a[i_ڪالم].p_اظهار
          );
          إذا( pNat!=pColl || (pNew->op!=TK_ڪالم && pNew->op!=TK_COLLATE) ){
            pNew = sqlite3_اظهار_جوڙيو_Collate_String(pSubst->pParse, pNew,
                (pColl ? pColl->zName : "BINARY")
            );
          }
        }
        اظهار_Clear_Property(pNew, EP_Collate);
        إذا( اظهار_Has_Property(p_اظهار,EP_OuterON|EP_InnerON) ){
          sqlite3_سيٽ_Join_اظهار(pNew, p_اظهار->w.iJoin,
                             p_اظهار->flags & (EP_OuterON|EP_InnerON));
        }
        sqlite3_اظهار_Delete(db, p_اظهار);
        p_اظهار = pNew;
      }
    }
  }وإلا{
    إذا( p_اظهار->op==TK_IF_NULL_صف && p_اظهار->i_جدول==pSubst->i_جدول ){
      p_اظهار->i_جدول = pSubst->i_New_جدول;
    }
    إذا( p_اظهار->op==TK_AGG_FUNCTION && p_اظهار->op2>=pSubst->n_Sel_اونهائي ){
      p_اظهار->op2--;
    }
    p_اظهار->p_کاٻو = subst_اظهار(pSubst, p_اظهار->p_کاٻو);
    p_اظهار->p_ساڄو = subst_اظهار(pSubst, p_اظهار->p_ساڄو);
    إذا( اظهار_Use_XSelect(p_اظهار) ){
      substSelect(pSubst, p_اظهار->x.pSelect, 1);
    }وإلا{
      subst_اظهار_List(pSubst, p_اظهار->x.pList);
    }
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
    إذا( اظهار_Has_Property(p_اظهار, EP_WinFunc) ){
      دري *pWin = p_اظهار->y.pWin;
      pWin->pFilter = subst_اظهار(pSubst, pWin->pFilter);
      subst_اظهار_List(pSubst, pWin->p_حصا_ڪريو);
      subst_اظهار_List(pSubst, pWin->p_ترتيب_By);
    }
#جيڪڏهن_پورو
  }
  أرجع p_اظهار;
}
ساكن فراغ subst_اظهار_List(
  SubstContext *pSubst, /* Description of the substitution */
  اظهار_List *pList       /* List to scan and in which to make substitutes */
){
  صحيح i;
  إذا( pList==0 ) أرجع;
  لكل(i=0; i<pList->n_اظهار; i++){
    pList->a[i].p_اظهار = subst_اظهار(pSubst, pList->a[i].p_اظهار);
  }
}
ساكن فراغ substSelect(
  SubstContext *pSubst, /* Description of the substitution */
  Select *p,            /* SELECT statement in which to make substitutions */
  صحيح do_اڳواٽ           /* Do substitutes on p->pPrior too */
){
  SrcList *pSrc;
  Src_شي *p_شي;
  صحيح i;
  إذا( !p ) أرجع;
  pSubst->n_Sel_اونهائي++;
  افعل{
    subst_اظهار_List(pSubst, p->pEList);
    subst_اظهار_List(pSubst, p->p_گروهه_By);
    subst_اظهار_List(pSubst, p->p_ترتيب_By);
    p->pHaving = subst_اظهار(pSubst, p->pHaving);
    p->pWhere = subst_اظهار(pSubst, p->pWhere);
    pSrc = p->pSrc;
    أكد( pSrc!=0 );
    لكل(i=pSrc->nSrc, p_شي=pSrc->a; i>0; i--, p_شي++){
      إذا( p_شي->fg.isSubquery ){
        substSelect(pSubst, p_شي->u4.pSubq->pSelect, 1);
      }
      إذا( p_شي->fg.isTabFunc ){
        subst_اظهار_List(pSubst, p_شي->u1.pFuncArg);
      }
    }
  }طالما( do_اڳواٽ && (p = p->p_اڳواٽ)!=0 );
  pSubst->n_Sel_اونهائي--;
}
#جيڪڏهن_پورو /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#إذا !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
/*
** pSelect is a SELECT statement and pSrcItem is one item in the FROM
** clause of that SELECT.
**
** This routine scans the entire SELECT statement and recomputes the
** pSrcItem->colUsed mask.
*/
ساكن صحيح recompute_Columns_استعمال_ٿيل_اظهار(Walker *pWalker, اظهار *p_اظهار){
  Src_شي *p_شي;
  إذا( p_اظهار->op!=TK_ڪالم ) أرجع WRC_Continue;
  p_شي = pWalker->u.p_Src_شي;
  إذا( p_شي->iCursor!=p_اظهار->i_جدول ) أرجع WRC_Continue;
  إذا( p_اظهار->i_ڪالم<0 ) أرجع WRC_Continue;
  p_شي->کانو_استعمال_ٿيل |= sqlite3_اظهار_کانو_استعمال_ٿيل(p_اظهار);
  أرجع WRC_Continue;
}
ساكن فراغ recompute_Columns_استعمال_ٿيل(
  Select *pSelect,                 /* The complete SELECT statement */
  Src_شي *p_Src_شي                /* Which FROM clause item to recompute */
){
  Walker w;
  إذا( NEVER(p_Src_شي->pSTab==0) ) أرجع;
  memset(&w, 0, حجم(w));
  w.x_اظهار_Callback = recompute_Columns_استعمال_ٿيل_اظهار;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.p_Src_شي = p_Src_شي;
  p_Src_شي->کانو_استعمال_ٿيل = 0;
  sqlite3WalkSelect(&w, pSelect);
}
#جيڪڏهن_پورو /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

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
  Src_شي *p_شي;
  لكل(i=0, p_شي=pSrc->a; i<pSrc->nSrc; i++, p_شي++){
    إذا( i!=iExcept ){
      Select *p;
      أكد( p_شي->iCursor < aCsrMap[0] );
      إذا( !p_شي->fg.isRecursive || aCsrMap[p_شي->iCursor+1]==0 ){
        aCsrMap[p_شي->iCursor+1] = pParse->nTab++;
      }
      p_شي->iCursor = aCsrMap[p_شي->iCursor+1];
      إذا( p_شي->fg.isSubquery ){
        لكل(p=p_شي->u4.pSubq->pSelect; p; p=p->p_اڳواٽ){
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
  صحيح *aCsrMap = pWalker->u.ai_کانو;
  صحيح iCsr = *piCursor;
  إذا( iCsr < aCsrMap[0] && aCsrMap[iCsr+1]>0 ){
    *piCursor = aCsrMap[iCsr+1];
  }
}

/*
** Expression walker callback used by renumberCursors() to update
** Expr objects to match newly assigned cursor numbers.
*/
ساكن صحيح renumberCursorsCb(Walker *pWalker, اظهار *p_اظهار){
  صحيح op = p_اظهار->op;
  إذا( op==TK_ڪالم || op==TK_IF_NULL_صف ){
    renumberCursorDoMapping(pWalker, &p_اظهار->i_جدول);
  }
  إذا( اظهار_Has_Property(p_اظهار, (EP_OuterON|EP_InnerON)) ){
    testcase( اظهار_Has_Property(p_اظهار, EP_OuterON) );
    testcase( اظهار_Has_Property(p_اظهار, EP_InnerON) );
    renumberCursorDoMapping(pWalker, &p_اظهار->w.iJoin);
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
  w.u.ai_کانو = aCsrMap;
  w.x_اظهار_Callback = renumberCursorsCb;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  sqlite3WalkSelect(&w, p);
}
#جيڪڏهن_پورو /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** If pSel is not part of a compound SELECT, return a pointer to its
** expression list. Otherwise, return a pointer to the expression list
** of the leftmost SELECT in the compound.
*/
ساكن اظهار_List *ڳوليو_Leftmost_Exprlist(Select *pSel){
  طالما( pSel->p_اڳواٽ ){
    pSel = pSel->p_اڳواٽ;
  }
  أرجع pSel->pEList;
}

/*
** Return true if any of the result-set columns in the compound query
** have incompatible affinities on one or more arms of the compound.
*/
ساكن صحيح compoundHasDifferentAffinities(Select *p){
  صحيح ii;
  اظهار_List *pList;
  أكد( p!=0 );
  أكد( p->pEList!=0 );
  أكد( p->p_اڳواٽ!=0 );
  pList = p->pEList;
  لكل(ii=0; ii<pList->n_اظهار; ii++){
    حرف aff;
    Select *pSub1;
    أكد( pList->a[ii].p_اظهار!=0 );
    aff = sqlite3_اظهار_Affinity(pList->a[ii].p_اظهار);
    لكل(pSub1=p->p_اڳواٽ; pSub1; pSub1=pSub1->p_اڳواٽ){
      أكد( pSub1->pEList!=0 );
      أكد( pSub1->pEList->n_اظهار>ii );
      أكد( pSub1->pEList->a[ii].p_اظهار!=0 );
      إذا( sqlite3_اظهار_Affinity(pSub1->pEList->a[ii].p_اظهار)!=aff ){
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
  Select *p_والد;    /* Current UNION ALL term of the other query */
  Select *pSub;       /* The inner query or "subquery" */
  Select *pSub1;      /* Pointer to the rightmost select in sub-query */
  SrcList *pSrc;      /* The FROM clause of the outer query */
  SrcList *pSubSrc;   /* The FROM clause of the subquery */
  صحيح i_والد;        /* VDBE cursor number of the pSub result set temp table */
  صحيح i_New_والد = -1;/* Replacement table for iParent */
  صحيح isOuterJoin = 0; /* True if pSub is the right side of a LEFT JOIN */   
  صحيح i;              /* Loop counter */
  اظهار *pWhere;                    /* The WHERE clause */
  Src_شي *pSubitem;               /* The subquery */
  sqlite3 *db = pParse->db;
  Walker w;                        /* Walker to persist agginfo data */
  صحيح *aCsrMap = 0;

  /* Check to see if flattening is permitted.  Return 0 if not.
  */
  أكد( p!=0 );
  أكد( p->p_اڳواٽ==0 );
  إذا( OptimizationDisabled(db, SQLITE_QueryFlattener) ) أرجع 0;
  pSrc = p->pSrc;
  أكد( pSrc && iFrom>=0 && iFrom<pSrc->nSrc );
  pSubitem = &pSrc->a[iFrom];
  i_والد = pSubitem->iCursor;
  أكد( pSubitem->fg.isSubquery );
  pSub = pSubitem->u4.pSubq->pSelect;
  أكد( pSub!=0 );

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
  إذا( p->pWin || pSub->pWin ) أرجع 0;                  /* Restriction (25) */
#جيڪڏهن_پورو

  pSubSrc = pSub->pSrc;
  أكد( pSubSrc );
  /* Prior to version 3.1.2, when LIMIT and OFFSET had to be simple constants,
  ** not arbitrary expressions, we allowed some combining of LIMIT and OFFSET
  ** because they could be computed at compile-time.  But when LIMIT and OFFSET
  ** became arbitrary expressions, we were forced to add restrictions (13)
  ** and (14). */
  إذا( pSub->p_حد_بندي && p->p_حد_بندي ) أرجع 0;              /* Restriction (13) */
  إذا( pSub->p_حد_بندي && pSub->p_حد_بندي->p_ساڄو ) أرجع 0;   /* Restriction (14) */
  إذا( (p->selFlags & SF_Compound)!=0 && pSub->p_حد_بندي ){
    أرجع 0;                                            /* Restriction (15) */
  }
  إذا( pSubSrc->nSrc==0 ) أرجع 0;                       /* Restriction (7)  */
  إذا( pSub->selFlags & SF_Distinct ) أرجع 0;           /* Restriction (4)  */
  إذا( pSub->p_حد_بندي && (pSrc->nSrc>1 || isAgg) ){
     أرجع 0;         /* Restrictions (8)(9) */
  }
  إذا( p->p_ترتيب_By && pSub->p_ترتيب_By ){
     أرجع 0;                                           /* Restriction (11) */
  }
  إذا( isAgg && pSub->p_ترتيب_By ) أرجع 0;                /* Restriction (16) */
  إذا( pSub->p_حد_بندي && p->pWhere ) أرجع 0;              /* Restriction (19) */
  إذا( pSub->p_حد_بندي && (p->selFlags & SF_Distinct)!=0 ){
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
     || (pSubitem->fg.jointype & JT_ساڄو)!=0  /* (26) */
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
  إذا( pSub->p_اڳواٽ ){
    صحيح ii;
    إذا( pSub->p_ترتيب_By ){
      أرجع 0;  /* Restriction (20) */
    }
    إذا( isAgg || (p->selFlags & SF_Distinct)!=0 || isOuterJoin>0 ){
      أرجع 0; /* (17d1), (17d2), or (17f) */
    }
    لكل(pSub1=pSub; pSub1; pSub1=pSub1->p_اڳواٽ){
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct );
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Aggregate );
      أكد( pSub->pSrc!=0 );
      أكد( (pSub->selFlags & SF_Recursive)==0 );
      أكد( pSub->pEList->n_اظهار==pSub1->pEList->n_اظهار );
      إذا( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))!=0    /* (17b) */
       || (pSub1->p_اڳواٽ && pSub1->op!=TK_ALL)                 /* (17a) */
       || pSub1->pSrc->nSrc<1                                  /* (17c) */
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
       || pSub1->pWin                                          /* (17e) */
#جيڪڏهن_پورو
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
    إذا( p->p_ترتيب_By ){
      لكل(ii=0; ii<p->p_ترتيب_By->n_اظهار; ii++){
        إذا( p->p_ترتيب_By->a[ii].u.x.i_ترتيب_By_کانو==0 ) أرجع 0;
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
  لكل(pSub=pSub->p_اڳواٽ; pSub; pSub=pSub->p_اڳواٽ){
    Select *pNew;
    اظهار_List *p_ترتيب_By = p->p_ترتيب_By;
    اظهار *p_حد_بندي = p->p_حد_بندي;
    Select *p_اڳواٽ = p->p_اڳواٽ;
    جدول *p_شي_Tab = pSubitem->pSTab;
    pSubitem->pSTab = 0;
    p->p_ترتيب_By = 0;
    p->p_اڳواٽ = 0;
    p->p_حد_بندي = 0;
    pNew = sqlite3SelectDup(db, p, 0);
    p->p_حد_بندي = p_حد_بندي;
    p->p_ترتيب_By = p_ترتيب_By;
    p->op = TK_ALL;
    pSubitem->pSTab = p_شي_Tab;
    إذا( pNew==0 ){
      p->p_اڳواٽ = p_اڳواٽ;
    }وإلا{
      pNew->selId = ++pParse->nSelect;
      إذا( aCsrMap && ALWAYS(db->mallocFailed==0) ){
        renumberCursors(pParse, pNew, iFrom, aCsrMap);
      }
      pNew->p_اڳواٽ = p_اڳواٽ;
      إذا( p_اڳواٽ ) p_اڳواٽ->p_ايندڙ = pNew;
      pNew->p_ايندڙ = p;
      p->p_اڳواٽ = pNew;
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
    sqlite3_Src_شي_Attach_Subquery(pParse, pSubitem, pSub1, 0);
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
      sqlite3_Parser_جوڙيو_Cleanup(pToplevel, sqlite3_Delete_جدول_Generic, p_Tab_منزل_Del);
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
  لكل(p_والد=p; p_والد; p_والد=p_والد->p_اڳواٽ, pSub=pSub->p_اڳواٽ){
    صحيح nSubSrc;
    u8 jointype = p_والد->pSrc->a[iFrom].fg.jointype;
    أكد( pSub!=0 );
    pSubSrc = pSub->pSrc;     /* FROM clause of subquery */
    nSubSrc = pSubSrc->nSrc;  /* Number of terms in subquery FROM clause */
    pSrc = p_والد->pSrc;     /* FROM clause of the outer query */

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
      p_والد->pSrc = pSrc;
    }

    /* Transfer the FROM clause terms from the subquery into the
    ** outer query.
    */
    i_New_والد = pSubSrc->a[0].iCursor;
    لكل(i=0; i<nSubSrc; i++){
      Src_شي *p_شي = &pSrc->a[i+iFrom];
      أكد( p_شي->fg.isTabFunc==0 );
      أكد( p_شي->fg.isSubquery
           || p_شي->fg.fixedSchema
           || p_شي->u4.zDatabase==0 );
      إذا( p_شي->fg.isUsing ) sqlite3IdListDelete(db, p_شي->u3.pUsing);
      *p_شي = pSubSrc->a[i];
      p_شي->fg.jointype |= (jointype & JT_LTORJ);
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
    إذا( pSub->p_ترتيب_By ){
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
      اظهار_List *p_ترتيب_By = pSub->p_ترتيب_By;
      لكل(i=0; i<p_ترتيب_By->n_اظهار; i++){
        p_ترتيب_By->a[i].u.x.i_ترتيب_By_کانو = 0;
      }
      أكد( p_والد->p_ترتيب_By==0 );
      p_والد->p_ترتيب_By = p_ترتيب_By;
      pSub->p_ترتيب_By = 0;
    }
    pWhere = pSub->pWhere;
    pSub->pWhere = 0;
    إذا( isOuterJoin>0 ){
      أكد( pSubSrc->nSrc==1 );
      sqlite3_سيٽ_Join_اظهار(pWhere, i_New_والد, EP_OuterON);
    }
    إذا( pWhere ){
      إذا( p_والد->pWhere ){
        p_والد->pWhere = sqlite3PExpr(pParse, TK_AND, pWhere, p_والد->pWhere);
      }وإلا{
        p_والد->pWhere = pWhere;
      }
    }
    إذا( db->mallocFailed==0 ){
      SubstContext x;
      x.pParse = pParse;
      x.i_جدول = i_والد;
      x.i_New_جدول = i_New_والد;
      x.isOuterJoin = isOuterJoin;
      x.n_Sel_اونهائي = 0;
      x.pEList = pSub->pEList;
      x.pCList = ڳوليو_Leftmost_Exprlist(pSub);
      substSelect(&x, p_والد, 0);
    }
 
    /* The flattened query is a compound if either the inner or the
    ** outer query is a compound. */
    p_والد->selFlags |= pSub->selFlags & SF_Compound;
    أكد( (pSub->selFlags & SF_Distinct)==0 ); /* restriction (17b) */
 
    /*
    ** SELECT ... FROM (SELECT ... LIMIT a OFFSET b) LIMIT x OFFSET y;
    **
    ** One is tempted to try to add a and b to combine the limits.  But this
    ** does not work if either limit is negative.
    */
    إذا( pSub->p_حد_بندي ){
      p_والد->p_حد_بندي = pSub->p_حد_بندي;
      pSub->p_حد_بندي = 0;
    }

    /* Recompute the SrcItem.colUsed masks for the flattened
    ** tables. */
    لكل(i=0; i<nSubSrc; i++){
      recompute_Columns_استعمال_ٿيل(p_والد, &pSrc->a[i+iFrom]);
    }
  }

  /* Finally, delete what is left of the subquery and return success.
  */
  sqlite3AggInfoPersistWalkerInit(&w, pParse);
  sqlite3WalkSelect(&w,pSub1);
  sqlite3SelectDelete(db, pSub1);

#إذا TREETRACE_ENABLED
  إذا( sqlite3_وڻ_Trace & 0x4 ){
    TREETRACE(0x4,pParse,p,("After flattening:\n"));
    sqlite3_وڻ_View_Select(0, p, 0);
  }
#جيڪڏهن_پورو

  أرجع 1;
}
#جيڪڏهن_پورو /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

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
  اظهار **ap_اظهار;   /* [i*2] is COLUMN and [i*2+1] is VALUE */
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
ساكن فراغ const_وجهو(
  WhereConst *pConst,  /* The WhereConst into which we are inserting */
  اظهار *p_ڪالم,       /* The COLUMN part of the constraint */
  اظهار *p_قدر,        /* The VALUE part of the constraint */
  اظهار *p_اظهار          /* Overall expression: COLUMN=VALUE or VALUE=COLUMN */
){
  صحيح i;
  أكد( p_ڪالم->op==TK_ڪالم );
  أكد( sqlite3_اظهار_Is_Constant(pConst->pParse, p_قدر) );
  إذا( اظهار_Has_Property(p_ڪالم, EP_Fixed_کانو) ) أرجع;
  إذا( sqlite3_اظهار_Affinity(p_قدر)!=0 ) أرجع;
  إذا( !sqlite3_Is_ٻٽو(sqlite3_اظهار_Compare_Coll_Seq(pConst->pParse,p_اظهار)) ){
    أرجع;
  }
  إذا( sqlite3_اظهار_Can_Return_Subtype(pConst->pParse, p_قدر) ) أرجع;

  /* 2018-10-25 ticket [cf5ed20f]
  ** Make sure the same pColumn is not inserted more than once */
  لكل(i=0; i<pConst->nConst; i++){
    ثابت اظهار *pE2 = pConst->ap_اظهار[i*2];
    أكد( pE2->op==TK_ڪالم );
    إذا( pE2->i_جدول==p_ڪالم->i_جدول
     && pE2->i_ڪالم==p_ڪالم->i_ڪالم
    ){
      أرجع;  /* Already present.  Return without doing anything. */
    }
  }
  أكد( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  إذا( sqlite3_اظهار_Affinity(p_ڪالم)<=SQLITE_AFF_BLOB ){
    pConst->bHasAffBlob = 1;
  }

  pConst->nConst++;
  pConst->ap_اظهار = sqlite3DbReallocOrFree(pConst->pParse->db, pConst->ap_اظهار,
                         pConst->nConst*2*حجم(اظهار*));
  إذا( pConst->ap_اظهار==0 ){
    pConst->nConst = 0;
  }وإلا{
    pConst->ap_اظهار[pConst->nConst*2-2] = p_ڪالم;
    pConst->ap_اظهار[pConst->nConst*2-1] = p_قدر;
  }
}

/*
** Find all terms of COLUMN=VALUE or VALUE=COLUMN in pExpr where VALUE
** is a constant expression and where the term must be true because it
** is part of the AND-connected terms of the expression.  For each term
** found, add it to the pConst structure.
*/
ساكن فراغ ڳوليو_Const_اندريون_Where(WhereConst *pConst, اظهار *p_اظهار){
  اظهار *p_ساڄو, *p_کاٻو;
  إذا( NEVER(p_اظهار==0) ) أرجع;
  إذا( اظهار_Has_Property(p_اظهار, pConst->mExcludeOn) ){
    testcase( اظهار_Has_Property(p_اظهار, EP_OuterON) );
    testcase( اظهار_Has_Property(p_اظهار, EP_InnerON) );
    أرجع;
  }
  إذا( p_اظهار->op==TK_AND ){
    ڳوليو_Const_اندريون_Where(pConst, p_اظهار->p_ساڄو);
    ڳوليو_Const_اندريون_Where(pConst, p_اظهار->p_کاٻو);
    أرجع;
  }
  إذا( p_اظهار->op!=TK_EQ ) أرجع;
  p_ساڄو = p_اظهار->p_ساڄو;
  p_کاٻو = p_اظهار->p_کاٻو;
  أكد( p_ساڄو!=0 );
  أكد( p_کاٻو!=0 );
  إذا( p_ساڄو->op==TK_ڪالم && sqlite3_اظهار_Is_Constant(pConst->pParse, p_کاٻو) ){
    const_وجهو(pConst,p_ساڄو,p_کاٻو,p_اظهار);
  }
  إذا( p_کاٻو->op==TK_ڪالم && sqlite3_اظهار_Is_Constant(pConst->pParse, p_ساڄو) ){
    const_وجهو(pConst,p_کاٻو,p_ساڄو,p_اظهار);
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
ساكن صحيح propagate_Constant_اظهار_Rewrite_One(
  WhereConst *pConst,
  اظهار *p_اظهار,
  صحيح bIgnoreAffBlob
){
  صحيح i;
  إذا( pConst->pOomFault[0] ) أرجع WRC_Prune;
  إذا( p_اظهار->op!=TK_ڪالم ) أرجع WRC_Continue;
  إذا( اظهار_Has_Property(p_اظهار, EP_Fixed_کانو|pConst->mExcludeOn) ){
    testcase( اظهار_Has_Property(p_اظهار, EP_Fixed_کانو) );
    testcase( اظهار_Has_Property(p_اظهار, EP_OuterON) );
    testcase( اظهار_Has_Property(p_اظهار, EP_InnerON) );
    أرجع WRC_Continue;
  }
  لكل(i=0; i<pConst->nConst; i++){
    اظهار *p_ڪالم = pConst->ap_اظهار[i*2];
    إذا( p_ڪالم==p_اظهار ) استمر;
    إذا( p_ڪالم->i_جدول!=p_اظهار->i_جدول ) استمر;
    إذا( p_ڪالم->i_ڪالم!=p_اظهار->i_ڪالم ) استمر;
    أكد( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
    إذا( bIgnoreAffBlob && sqlite3_اظهار_Affinity(p_ڪالم)<=SQLITE_AFF_BLOB ){
      اكسر;
    }
    /* A match is found.  Add the EP_FixedCol property */
    pConst->nChng++;
    اظهار_Clear_Property(p_اظهار, EP_پن);
    اظهار_سيٽ_Property(p_اظهار, EP_Fixed_کانو);
    أكد( p_اظهار->p_کاٻو==0 );
    p_اظهار->p_کاٻو = sqlite3_اظهار_Dup(pConst->pParse->db, pConst->ap_اظهار[i*2+1], 0);
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
ساكن صحيح propagate_Constant_اظهار_Rewrite(Walker *pWalker, اظهار *p_اظهار){
  WhereConst *pConst = pWalker->u.pConst;
  أكد( TK_GT==TK_EQ+1 );
  أكد( TK_LE==TK_EQ+2 );
  أكد( TK_LT==TK_EQ+3 );
  أكد( TK_GE==TK_EQ+4 );
  إذا( pConst->bHasAffBlob ){
    إذا( (p_اظهار->op>=TK_EQ && p_اظهار->op<=TK_GE)
     || p_اظهار->op==TK_IS
    ){
      propagate_Constant_اظهار_Rewrite_One(pConst, p_اظهار->p_کاٻو, 0);
      إذا( pConst->pOomFault[0] ) أرجع WRC_Prune;
      إذا( sqlite3_اظهار_Affinity(p_اظهار->p_کاٻو)!=SQLITE_AFF_متن ){
        propagate_Constant_اظهار_Rewrite_One(pConst, p_اظهار->p_ساڄو, 0);
      }
    }
  }
  أرجع propagate_Constant_اظهار_Rewrite_One(pConst, p_اظهار, pConst->bHasAffBlob);
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
    x.ap_اظهار = 0;
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
    ڳوليو_Const_اندريون_Where(&x, p->pWhere);
    إذا( x.nConst ){
      memset(&w, 0, حجم(w));
      w.pParse = pParse;
      w.x_اظهار_Callback = propagate_Constant_اظهار_Rewrite;
      w.xSelectCallback = sqlite3SelectWalkNoop;
      w.xSelectCallback2 = 0;
      w.walker_اونهائي = 0;
      w.u.pConst = &x;
      sqlite3_Walk_اظهار(&w, p->pWhere);
      sqlite3DbFree(x.pParse->db, x.ap_اظهار);
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
ساكن صحيح ڌڪو_هيٺ_ڏانهن_دري_Check(Parse *pParse, Select *pSubq, اظهار *p_اظهار){
  أكد( pSubq->pWin->p_حصا_ڪريو );
  أكد( (pSubq->selFlags & SF_MultiPart)==0 );
  أكد( pSubq->p_اڳواٽ==0 );
  أرجع sqlite3_اظهار_Is_Constant_Or_گروهه_By(pParse, p_اظهار, pSubq->pWin->p_حصا_ڪريو);
}
# endif /* SQLITE_OMIT_WINDOWFUNC */
#جيڪڏهن_پورو /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

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
ساكن صحيح ڌڪو_هيٺ_ڏانهن_Where_Terms(
  Parse *pParse,        /* Parse context (for malloc() and error reporting) */
  Select *pSubq,        /* The subquery whose WHERE clause is to be augmented */
  اظهار *pWhere,         /* The WHERE clause of the outer query */
  SrcList *pSrcList,    /* The complete from clause of the outer query */
  صحيح iSrc              /* Which FROM clause term to try to push into  */
){
  اظهار *pNew;
  Src_شي *pSrc;        /* The subquery FROM term into which WHERE is pushed */
  صحيح nChng = 0;
  pSrc = &pSrcList->a[iSrc];
  إذا( pWhere==0 ) أرجع 0;
  إذا( pSubq->selFlags & (SF_Recursive|SF_MultiPart) ){
    أرجع 0;           /* restrictions (2) and (11) */
  }
  إذا( pSrc->fg.jointype & (JT_LTORJ|JT_ساڄو) ){
    أرجع 0;           /* restrictions (10) */
  }

  إذا( pSubq->p_اڳواٽ ){
    Select *pSel;
    صحيح notUnionAll = 0;
    لكل(pSel=pSubq; pSel; pSel=pSel->p_اڳواٽ){
      u8 op = pSel->op;
      أكد( op==TK_ALL || op==TK_SELECT
           || op==TK_UNION || op==TK_INTERSECT || op==TK_EXCEPT );
      إذا( op!=TK_ALL && op!=TK_SELECT ){
        notUnionAll = 1;
      }
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
      إذا( pSel->pWin ) أرجع 0;    /* restriction (6b) */
#جيڪڏهن_پورو
    }
    إذا( notUnionAll ){
      /* If any of the compound arms are connected using UNION, INTERSECT,
      ** or EXCEPT, then we must ensure that none of the columns use a
      ** non-BINARY collating sequence. */
      لكل(pSel=pSubq; pSel; pSel=pSel->p_اڳواٽ){
        صحيح ii;
        ثابت اظهار_List *pList = pSel->pEList;
        أكد( pList!=0 );
        لكل(ii=0; ii<pList->n_اظهار; ii++){
          CollSeq *pColl = sqlite3_اظهار_Coll_Seq(pParse, pList->a[ii].p_اظهار);
          إذا( !sqlite3_Is_ٻٽو(pColl) ){
            أرجع 0;  /* Restriction (8) */
          }
        }
      }
    }
  }وإلا{
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
    إذا( pSubq->pWin && pSubq->pWin->p_حصا_ڪريو==0 ) أرجع 0;
#جيڪڏهن_پورو
  }

#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
  /* Only the first term of a compound can have a WITH clause.  But make
  ** sure no other terms are marked SF_Recursive in case something changes
  ** in the future.
  */
  {
    Select *pX; 
    لكل(pX=pSubq; pX; pX=pX->p_اڳواٽ){
      أكد( (pX->selFlags & (SF_Recursive))==0 );
    }
  }
#جيڪڏهن_پورو

  إذا( pSubq->p_حد_بندي!=0 ){
    أرجع 0; /* restriction (3) */
  }
  طالما( pWhere->op==TK_AND ){
    nChng += ڌڪو_هيٺ_ڏانهن_Where_Terms(pParse, pSubq, pWhere->p_ساڄو, pSrcList, iSrc);
    pWhere = pWhere->p_کاٻو;
  }

#إذا 0 /* These checks now done by sqlite3ExprIsSingleTableConstraint() */
  إذا( اظهار_Has_Property(pWhere, EP_OuterON|EP_InnerON) /* (9a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (9c) */
  ){
    صحيح jj;
    لكل(jj=0; jj<iSrc; jj++){
      إذا( pWhere->w.iJoin==pSrcList->a[jj].iCursor ){
        /* If we reach this point, both (9a) and (9b) are satisfied.
        ** The following loop checks (9c):
        */
        لكل(jj++; jj<iSrc; jj++){
          إذا( (pSrcList->a[jj].fg.jointype & JT_ساڄو)!=0 ){
            أرجع 0;  /* restriction (9) */
          }
        }
      }
    }
  }
  إذا( is_کاٻو_Join
   && (اظهار_Has_Property(pWhere,EP_OuterON)==0
         || pWhere->w.iJoin!=iCursor)
  ){
    أرجع 0; /* restriction (4) */
  }
  إذا( اظهار_Has_Property(pWhere,EP_OuterON)
   && pWhere->w.iJoin!=iCursor
  ){
    أرجع 0; /* restriction (5) */
  }
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل SQLITE_ALLOW_ROWID_اندريون_VIEW
  إذا( ViewCanHaveRowid && (pWhere->op==TK_ISNULL || pWhere->op==TK_NOTNULL) ){
    اظهار *p_کاٻو = pWhere->p_کاٻو;
    إذا( ALWAYS(p_کاٻو) 
     && p_کاٻو->op==TK_ڪالم
     && p_کاٻو->i_ڪالم < 0
    ){
      أرجع 0;  /* Restriction (12) */
    }
  }
#جيڪڏهن_پورو

  إذا( sqlite3_اظهار_Is_Single_جدول_Constraint(pWhere, pSrcList, iSrc, 1) ){
    nChng++;
    pSubq->selFlags |= SF_ڌڪو_هيٺ_ڏانهن;
    طالما( pSubq ){
      SubstContext x;
      pNew = sqlite3_اظهار_Dup(pParse->db, pWhere, 0);
      unset_Join_اظهار(pNew, -1, 1);
      x.pParse = pParse;
      x.i_جدول = pSrc->iCursor;
      x.i_New_جدول = pSrc->iCursor;
      x.isOuterJoin = 0;
      x.n_Sel_اونهائي = 0;
      x.pEList = pSubq->pEList;
      x.pCList = ڳوليو_Leftmost_Exprlist(pSubq);
      pNew = subst_اظهار(&x, pNew);
      أكد( pNew!=0 || pParse->nErr!=0 );
      إذا( pParse->nErr==0 && pNew->op==TK_اندريون && اظهار_Use_XSelect(pNew) ){
        أكد( pNew->x.pSelect!=0 );
        pNew->x.pSelect->selFlags |= SF_Cloned_Rhs_اندريون;
        أكد( pWhere!=0 );
        أكد( pWhere->op==TK_اندريون );
        أكد( اظهار_Use_XSelect(pWhere) );
        أكد( pWhere->x.pSelect!=0 );
        pWhere->x.pSelect->selFlags |= SF_Cloned_Rhs_اندريون;
      }
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
      إذا( pSubq->pWin && 0==ڌڪو_هيٺ_ڏانهن_دري_Check(pParse, pSubq, pNew) ){
        /* Restriction 6c has prevented push-down in this case */
        sqlite3_اظهار_Delete(pParse->db, pNew);
        nChng--;
        اكسر;
      }
#جيڪڏهن_پورو
      إذا( pSubq->selFlags & SF_Aggregate ){
        pSubq->pHaving = sqlite3_اظهار_And(pParse, pSubq->pHaving, pNew);
      }وإلا{
        pSubq->pWhere = sqlite3_اظهار_And(pParse, pSubq->pWhere, pNew);
      }
      pSubq = pSubq->p_اڳواٽ;
    }
  }
  أرجع nChng;
}
#جيڪڏهن_پورو /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** Check to see if a subquery contains result-set columns that are
** never used.  If it does, change the value of those result-set columns
** to NULL so that they do not cause unnecessary work to compute.
**
** Return the number of column that were changed to NULL.
*/
ساكن صحيح disable_اڻ_استعمال_ٿيل_Subquery_نتيجو_Columns(Src_شي *p_شي){
  صحيح n_کانو;
  Select *pSub;      /* The subquery to be simplified */
  Select *pX;        /* For looping over compound elements of pSub */
  جدول *pTab;       /* The table that describes the subquery */
  صحيح j;             /* Column number */
  صحيح nChng = 0;     /* Number of columns converted to NULL */
  Bitmask کانو_استعمال_ٿيل;   /* Columns that may not be NULLed out */

  أكد( p_شي!=0 );
  إذا( p_شي->fg.isCorrelated || p_شي->fg.isCte ){
    أرجع 0;
  }
  أكد( p_شي->pSTab!=0 );
  pTab = p_شي->pSTab;
  أكد( p_شي->fg.isSubquery );
  pSub = p_شي->u4.pSubq->pSelect;
  أكد( pSub->pEList->n_اظهار==pTab->n_کانو );
  لكل(pX=pSub; pX; pX=pX->p_اڳواٽ){
    إذا( (pX->selFlags & (SF_Distinct|SF_Aggregate))!=0 ){
      testcase( pX->selFlags & SF_Distinct );
      testcase( pX->selFlags & SF_Aggregate );
      أرجع 0;
    }
    إذا( pX->p_اڳواٽ && pX->op!=TK_ALL ){
      /* This optimization does not work for compound subqueries that
      ** use UNION, INTERSECT, or EXCEPT.  Only UNION ALL is allowed. */
      أرجع 0;
    }
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
    إذا( pX->pWin ){
      /* This optimization does not work for subqueries that use window
      ** functions. */
      أرجع 0;
    }
#جيڪڏهن_پورو
  }
  کانو_استعمال_ٿيل = p_شي->کانو_استعمال_ٿيل;
  إذا( pSub->p_ترتيب_By ){
    اظهار_List *pList = pSub->p_ترتيب_By;
    لكل(j=0; j<pList->n_اظهار; j++){
      u16 i_کانو = pList->a[j].u.x.i_ترتيب_By_کانو;
      إذا( i_کانو>0 ){
        i_کانو--;
        کانو_استعمال_ٿيل |= ((Bitmask)1)<<(i_کانو>=BMS ? BMS-1 : i_کانو);
      }
    }
  }
  n_کانو = pTab->n_کانو;
  لكل(j=0; j<n_کانو; j++){
    Bitmask m = j<BMS-1 ? MASKBIT(j) : TOPBIT;
    إذا( (m & کانو_استعمال_ٿيل)!=0 ) استمر;
    لكل(pX=pSub; pX; pX=pX->p_اڳواٽ) {
      اظهار *pY = pX->pEList->a[j].p_اظهار;
      إذا( pY->op==TK_NULL ) استمر;
      pY->op = TK_NULL;
      اظهار_Clear_Property(pY, EP_Skip|EP_Unlikely);
      pX->selFlags |= SF_ڌڪو_هيٺ_ڏانهن;
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
ساكن u8 minMaxQuery(sqlite3 *db, اظهار *pFunc, اظهار_List **ppMinMax){
  صحيح eRet = WHERE_ORDERBY_عام;      /* Return value */
  اظهار_List *pEList;                     /* Arguments to agg function */
  ثابت حرف *zFunc;                    /* Name of aggregate function pFunc */
  اظهار_List *p_ترتيب_By;
  u8 ترتيب_ڏيو_Flags = 0;

  أكد( *ppMinMax==0 );
  أكد( pFunc->op==TK_AGG_FUNCTION );
  أكد( !Is_دري_Func(pFunc) );
  أكد( اظهار_Use_XList(pFunc) );
  pEList = pFunc->x.pList;
  إذا( pEList==0
   || pEList->n_اظهار!=1
   || اظهار_Has_Property(pFunc, EP_WinFunc)
   || OptimizationDisabled(db, SQLITE_MinMaxOpt)
  ){
    أرجع eRet;
  }
  أكد( !اظهار_Has_Property(pFunc, EP_Int_قدر) );
  zFunc = pFunc->u.zToken;
  إذا( sqlite3StrICmp(zFunc, "min")==0 ){
    eRet = WHERE_ORDERBY_MIN;
    إذا( sqlite3_اظهار_Can_Be_Null(pEList->a[0].p_اظهار) ){
      ترتيب_ڏيو_Flags = KEYINFO_ترتيب_BIGNULL;
    }
  }وإلا إذا( sqlite3StrICmp(zFunc, "max")==0 ){
    eRet = WHERE_ORDERBY_MAX;
    ترتيب_ڏيو_Flags = KEYINFO_ترتيب_DESC;
  }وإلا{
    أرجع eRet;
  }
  *ppMinMax = p_ترتيب_By = sqlite3_اظهار_List_Dup(db, pEList, 0);
  أكد( p_ترتيب_By!=0 || db->mallocFailed );
  إذا( p_ترتيب_By ) p_ترتيب_By->a[0].fg.ترتيب_ڏيو_Flags = ترتيب_ڏيو_Flags;
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
ساكن جدول *is_Simple_ڳڻپ(Select *p, AggInfo *pAggInfo){
  جدول *pTab;
  اظهار *p_اظهار;

  أكد( !p->p_گروهه_By );

  إذا( p->pWhere
   || p->pEList->n_اظهار!=1
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
  p_اظهار = p->pEList->a[0].p_اظهار;
  أكد( p_اظهار!=0 );
  إذا( p_اظهار->op!=TK_AGG_FUNCTION ) أرجع 0;
  إذا( p_اظهار->pAggInfo!=pAggInfo ) أرجع 0;
  إذا( (pAggInfo->aFunc[0].pFunc->funcFlags&SQLITE_FUNC_ڳڻپ)==0 ) أرجع 0;
  أكد( pAggInfo->aFunc[0].pFExpr==p_اظهار );
  testcase( اظهار_Has_Property(p_اظهار, EP_Distinct) );
  testcase( اظهار_Has_Property(p_اظهار, EP_WinFunc) );
  إذا( اظهار_Has_Property(p_اظهار, EP_Distinct|EP_WinFunc) ) أرجع 0;

  أرجع pTab;
}

/*
** If the source-list item passed as an argument was augmented with an
** INDEXED BY clause, then try to locate the specified index. If there
** was such a clause and the named index cannot be found, return
** SQLITE_ERROR and leave an error in pParse. Otherwise, populate
** pFrom->pIndex and return SQLITE_OK.
*/
صحيح sqlite3_اشاريي_وارو_By_Lookup(Parse *pParse, Src_شي *pFrom){
  جدول *pTab = pFrom->pSTab;
  حرف *z_اشاريي_وارو_By = pFrom->u1.z_اشاريي_وارو_By;
  اشاريو *pIdx;
  أكد( pTab!=0 );
  أكد( pFrom->fg.is_اشاريي_وارو_By!=0 );

  لكل(pIdx=pTab->p_اشاريو;
      pIdx && sqlite3StrICmp(pIdx->zName, z_اشاريي_وارو_By);
      pIdx=pIdx->p_ايندڙ
  );
  إذا( !pIdx ){
    sqlite3_Error_نياپو(pParse, "no such index: %s", z_اشاريي_وارو_By, 0);
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
  بنية اظهار_List_شي *a;
  SrcList *pNewSrc;
  Parse *pParse;
  Token dummy;

  إذا( p->p_اڳواٽ==0 ) أرجع WRC_Continue;
  إذا( p->p_ترتيب_By==0 ) أرجع WRC_Continue;
  لكل(pX=p; pX && (pX->op==TK_ALL || pX->op==TK_SELECT); pX=pX->p_اڳواٽ){}
  إذا( pX==0 ) أرجع WRC_Continue;
  a = p->p_ترتيب_By->a;
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
  /* If iOrderByCol is already non-zero, then it has already been matched
  ** to a result column of the SELECT statement. This occurs when the
  ** SELECT is rewritten for window-functions processing and then passed
  ** to sqlite3SelectPrep() and similar a second time. The rewriting done
  ** by this function is not required in this case. */
  إذا( a[0].u.x.i_ترتيب_By_کانو ) أرجع WRC_Continue;
#جيڪڏهن_پورو
  لكل(i=p->p_ترتيب_By->n_اظهار-1; i>=0; i--){
    إذا( a[i].p_اظهار->flags & EP_Collate ) اكسر;
  }
  إذا( i<0 ) أرجع WRC_Continue;

  /* If we reach this point, that means the transformation is required. */

  pParse = pWalker->pParse;
  db = pParse->db;
  pNew = sqlite3DbMallocZero(db, حجم(*pNew) );
  إذا( pNew==0 ) أرجع WRC_Abort;
  memset(&dummy, 0, حجم(dummy));
  pNewSrc = sqlite3_Src_List_Append_From_رڪن(pParse,0,0,0,&dummy,pNew,0);
  أكد( pNewSrc!=0 || pParse->nErr );
  إذا( pParse->nErr ){
    sqlite3SrcListDelete(db, pNewSrc);
    أرجع WRC_Abort;
  }
  *pNew = *p;
  p->pSrc = pNewSrc;
  p->pEList = sqlite3_اظهار_List_Append(pParse, 0, sqlite3_اظهار(db, TK_ASTERISK, 0));
  p->op = TK_SELECT;
  p->pWhere = 0;
  p->p_گروهه_By = 0;
  p->pHaving = 0;
  pNew->p_ترتيب_By = 0;
  p->p_اڳواٽ = 0;
  p->p_ايندڙ = 0;
  p->pWith = 0;
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
  p->pWinDefn = 0;
#جيڪڏهن_پورو
  p->selFlags &= ~(u32)(SF_Compound|SF_Distinct);
  أكد( (p->selFlags & SF_Converted)==0 );
  p->selFlags |= SF_Converted;
  أكد( pNew->p_اڳواٽ!=0 );
  pNew->p_اڳواٽ->p_ايندڙ = pNew;
  pNew->p_حد_بندي = 0;
  أرجع WRC_Continue;
}

/*
** Check to see if the FROM clause term pFrom has table-valued function
** arguments.  If it does, leave an error message in pParse and return
** non-zero, since pFrom is not allowed to be a table-valued function.
*/
ساكن صحيح cannotBeFunction(Parse *pParse, Src_شي *pFrom){
  إذا( pFrom->fg.isTabFunc ){
    sqlite3_Error_نياپو(pParse, "'%s' is not a function", pFrom->zName);
    أرجع 1;
  }
  أرجع 0;
}

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_CTE
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
ساكن بنية Cte *ڳولا_With(
  With *pWith,                    /* Current innermost WITH clause */
  Src_شي *p_شي,                 /* FROM clause element to resolve */
  With **ppContext                /* OUT: WITH clause return value belongs to */
){
  ثابت حرف *zName = p_شي->zName;
  With *p;
  أكد( p_شي->fg.fixedSchema || p_شي->u4.zDatabase==0 );
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
With *sqlite3_With_ڌڪو(Parse *pParse, With *pWith, u8 bFree){
  إذا( pWith ){
    إذا( bFree ){
      pWith = (With*)sqlite3_Parser_جوڙيو_Cleanup(pParse, sqlite3WithDeleteGeneric,
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
ساكن صحيح resolve_From_رڪن_منزل_Cte(
  Parse *pParse,                  /* The parsing context */
  Walker *pWalker,                /* Current tree walker */
  Src_شي *pFrom                  /* The FROM clause term to check */
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
  pCte = ڳولا_With(pParse->pWith, pFrom, &pWith);
  إذا( pCte ){
    sqlite3 *db = pParse->db;
    جدول *pTab;
    اظهار_List *pEList;
    Select *pSel;
    Select *p_کاٻو;                /* Left-most SELECT statement */
    Select *p_Rec_رڪن;             /* Left-most recursive term */
    صحيح bMayRecursive;            /* True if compound joined by UNION [ALL] */
    With *pSavedWith;             /* Initial value of pParse->pWith */
    صحيح iRecTab = -1;             /* Cursor for recursive table */
    CteUse *pCteUse;

    /* If pCte->zCteErr is non-NULL at this point, then this is an illegal
    ** recursive reference to CTE pCte. Leave an error in pParse and return
    ** early. If pCte->zCteErr is NULL, then this is not a recursive reference.
    ** In this case, proceed.  */
    إذا( pCte->zCteErr ){
      sqlite3_Error_نياپو(pParse, pCte->zCteErr, pCte->zName);
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
       || sqlite3_Parser_جوڙيو_Cleanup(pParse,sqlite3DbFree,pCteUse)==0
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
    pTab->n_صف_Log_Est = 200; أكد( 200==sqlite3LogEst(1048576) );
    pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
    sqlite3_Src_شي_Attach_Subquery(pParse, pFrom, pCte->pSelect, 1);
    إذا( db->mallocFailed ) أرجع 2;
    أكد( pFrom->fg.isSubquery && pFrom->u4.pSubq );
    pSel = pFrom->u4.pSubq->pSelect;
    أكد( pSel!=0 );
    pSel->selFlags |= SF_CopyCte;
    إذا( pFrom->fg.is_اشاريي_وارو_By ){
      sqlite3_Error_نياپو(pParse, "no such index: \"%s\"", pFrom->u1.z_اشاريي_وارو_By);
      أرجع 2;
    }
    أكد( !pFrom->fg.is_اشاريي_وارو_By );
    pFrom->fg.isCte = 1;
    pFrom->u2.pCteUse = pCteUse;
    pCteUse->nUse++;

    /* Check if this is a recursive CTE. */
    p_Rec_رڪن = pSel;
    bMayRecursive = ( pSel->op==TK_ALL || pSel->op==TK_UNION );
    طالما( bMayRecursive && p_Rec_رڪن->op==pSel->op ){
      صحيح i;
      SrcList *pSrc = p_Rec_رڪن->pSrc;
      أكد( p_Rec_رڪن->p_اڳواٽ!=0 );
      لكل(i=0; i<pSrc->nSrc; i++){
        Src_شي *p_شي = &pSrc->a[i];
        إذا( p_شي->zName!=0
         && !p_شي->fg.hadSchema
         && ALWAYS( !p_شي->fg.isSubquery )
         && (p_شي->fg.fixedSchema || p_شي->u4.zDatabase==0)
         && 0==sqlite3StrICmp(p_شي->zName, pCte->zName)
        ){
          p_شي->pSTab = pTab;
          pTab->nTabRef++;
          p_شي->fg.isRecursive = 1;
          إذا( p_Rec_رڪن->selFlags & SF_Recursive ){
            sqlite3_Error_نياپو(pParse,
               "multiple references to recursive table: %s", pCte->zName
            );
            أرجع 2;
          }
          p_Rec_رڪن->selFlags |= SF_Recursive;
          إذا( iRecTab<0 ) iRecTab = pParse->nTab++;
          p_شي->iCursor = iRecTab;
        }
      }
      إذا( (p_Rec_رڪن->selFlags & SF_Recursive)==0 ) اكسر;
      p_Rec_رڪن = p_Rec_رڪن->p_اڳواٽ;
    }

    إذا( pParse->nTab >= db->a_حد_بندي[SQLITE_حد_بندي_CTES]
     && db->a_حد_بندي[SQLITE_حد_بندي_CTES]>0
    ){
      sqlite3_Error_نياپو(pParse, "too many CTEs");
      أرجع 2;
    }
#إذا SQLITE_MAX_اظهار_اونهائي>0
    pParse->nNestSel++;
    إذا( check_Subquery_Nesting_اونهائي(pParse) ) أرجع 2;
#جيڪڏهن_پورو
    pCte->zCteErr = "circular reference: %s";
    pSavedWith = pParse->pWith;
    pParse->pWith = pWith;
    إذا( pSel->selFlags & SF_Recursive ){
      صحيح rc;
      أكد( p_Rec_رڪن!=0 );
      أكد( (p_Rec_رڪن->selFlags & SF_Recursive)==0 );
      أكد( p_Rec_رڪن->p_ايندڙ!=0 );
      أكد( (p_Rec_رڪن->p_ايندڙ->selFlags & SF_Recursive)!=0 );
      أكد( p_Rec_رڪن->pWith==0 );
      p_Rec_رڪن->pWith = pSel->pWith;
      rc = sqlite3WalkSelect(pWalker, p_Rec_رڪن);
      p_Rec_رڪن->pWith = 0;
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
#إذا SQLITE_MAX_اظهار_اونهائي>0
    pParse->nNestSel--;
    أكد( pParse->nNestSel>=0 );
#جيڪڏهن_پورو

    لكل(p_کاٻو=pSel; p_کاٻو->p_اڳواٽ; p_کاٻو=p_کاٻو->p_اڳواٽ);
    pEList = p_کاٻو->pEList;
    إذا( pCte->p_کانن_ڳڻپ ){
      إذا( pEList && pEList->n_اظهار!=pCte->p_کانن_ڳڻپ->n_اظهار ){
        sqlite3_Error_نياپو(pParse, "table %s has %d values for %d columns",
            pCte->zName, pEList->n_اظهار, pCte->p_کانن_ڳڻپ->n_اظهار
        );
        pParse->pWith = pSavedWith;
        أرجع 2;
      }
      pEList = pCte->p_کانن_ڳڻپ;
    }

    sqlite3_Columns_From_اظهار_List(pParse, pEList, &pTab->n_کانو, &pTab->a_کانو);
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
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_CTE
/*
** If the SELECT passed as the second argument has an associated WITH
** clause, pop it from the stack stored as part of the Parse object.
**
** This function is used as the xSelectCallback2() callback by
** sqlite3SelectExpand() when walking a SELECT tree to resolve table
** names and other FROM clause elements.
*/
فراغ sqlite3_Select_ڪڍو_With(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  إذا( OK_IF_ALWAYS_TRUE(pParse->pWith) && p->p_اڳواٽ==0 ){
    With *pWith = ڳوليو_Rightmost(p)->pWith;
    إذا( pWith!=0 ){
      أكد( pParse->pWith==pWith || pParse->nErr );
      pParse->pWith = pWith->pOuter;
    }
  }
}
#جيڪڏهن_پورو

/*
** The SrcItem structure passed as the second argument represents a
** sub-query in the FROM clause of a SELECT statement. This function
** allocates and populates the SrcItem.pTab object. If successful,
** SQLITE_OK is returned. Otherwise, if an OOM error is encountered,
** SQLITE_NOMEM.
*/
صحيح sqlite3ExpandSubquery(Parse *pParse, Src_شي *pFrom){
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
  طالما( pSel->p_اڳواٽ ){ pSel = pSel->p_اڳواٽ; }
  sqlite3_Columns_From_اظهار_List(pParse, pSel->pEList,&pTab->n_کانو,&pTab->a_کانو);
  pTab->iPKey = -1;
  pTab->eTabType = TABTYP_VIEW;
  pTab->n_صف_Log_Est = 200; أكد( 200==sqlite3LogEst(1048576) );
#جيڪڏهن_وصف_نه_ٿيل SQLITE_ALLOW_ROWID_اندريون_VIEW
  /* The usual case - do not allow ROWID on a subquery */
  pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
#وإلا
  /* Legacy compatibility mode */
  pTab->tabFlags |= TF_Ephemeral | sqlite3Config.mNoVisibleRowid;
#جيڪڏهن_پورو
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
ساكن صحيح اندريون_Any_Using_Clause(
  ثابت حرف *zName, /* Name we are looking for */
  Src_شي *p_بنياد,    /* The base SrcItem.  Looking at pBase[1] and following */
  صحيح N              /* How many SrcItems to check */
){
  طالما( N>0 ){
    N--;
    p_بنياد++;
    إذا( p_بنياد->fg.isUsing==0 ) استمر;
    إذا( NEVER(p_بنياد->u3.pUsing==0) ) استمر;
    إذا( sqlite3_Id_List_اشاريو(p_بنياد->u3.pUsing, zName)>=0 ) أرجع 1;
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
  اظهار_List *pEList;
  Src_شي *pFrom;
  sqlite3 *db = pParse->db;
  اظهار *pE, *p_ساڄو, *p_اظهار;
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
  إذا( pWalker->e_ڪوڊ ){
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
  sqlite3_With_ڌڪو(pParse, p->pWith, 0);

  /* Make sure cursor numbers have been assigned to all entries in
  ** the FROM clause of the SELECT statement.
  */
  sqlite3_Src_List_سونپيو_Cursors(pParse, pTabList);

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
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_SUBQUERY
      Select *pSel;
      أكد( pFrom->fg.isSubquery && pFrom->u4.pSubq!=0 );
      pSel = pFrom->u4.pSubq->pSelect;
      /* A sub-query in the FROM clause of a SELECT */
      أكد( pSel!=0 );
      أكد( pFrom->pSTab==0 );
      إذا( sqlite3WalkSelect(pWalker, pSel) ) أرجع WRC_Abort;
      إذا( sqlite3ExpandSubquery(pParse, pFrom) ) أرجع WRC_Abort;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_CTE
    }وإلا إذا( (rc = resolve_From_رڪن_منزل_Cte(pParse, pWalker, pFrom))!=0 ){
      إذا( rc>1 ) أرجع WRC_Abort;
      pTab = pFrom->pSTab;
      أكد( pTab!=0 );
#جيڪڏهن_پورو
    }وإلا{
      /* An ordinary table or view name in the FROM clause */
      أكد( pFrom->pSTab==0 );
      pFrom->pSTab = pTab = sqlite3_Locate_جدول_شي(pParse, 0, pFrom);
      إذا( pTab==0 ) أرجع WRC_Abort;
      إذا( pTab->nTabRef>=0xffff ){
        sqlite3_Error_نياپو(pParse, "too many references to \"%s\": max 65535",
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
        i16 n_کانو;
        u8 e_ڪوڊ_Orig = pWalker->e_ڪوڊ;
        إذا( sqlite3_View_Get_ڪالم_نالا(pParse, pTab) ) أرجع WRC_Abort;
        أكد( pFrom->fg.isSubquery==0 );
        إذا( IsView(pTab) ){
          إذا( (db->flags & SQLITE_EnableView)==0
           && pTab->pSchema!=db->aDb[1].pSchema
          ){
            sqlite3_Error_نياپو(pParse, "access to view \"%s\" prohibited",
              pTab->zName);
          }
          sqlite3_Src_شي_Attach_Subquery(pParse, pFrom, pTab->u.view.pSelect, 1);
        }
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_VIRTUALTABLE
        وإلا إذا( ALWAYS(IsVirtual(pTab))
         && (pFrom->fg.fromDDL || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
         && ALWAYS(pTab->u.vtab.p!=0)
         && pTab->u.vtab.p->eVtabRisk > ((db->flags & SQLITE_TrustedSchema)!=0)
        ){
          sqlite3_Error_نياپو(pParse, "unsafe use of virtual table \"%s\"",
                                  pTab->zName);
        }
        أكد( SQLITE_VTABRISK_عام==1 && SQLITE_VTABRISK_مٿاهون==2 );
#جيڪڏهن_پورو
        n_کانو = pTab->n_کانو;
        pTab->n_کانو = -1;
        pWalker->e_ڪوڊ = 1;  /* Turn on Select.selId renumbering */
        إذا( pFrom->fg.isSubquery ){
          bft hasCompound = pParse->hasCompound;
          pParse->hasCompound = 1;
          selectExpanderWalk(pWalker, pFrom->u4.pSubq->pSelect);
          pParse->hasCompound = hasCompound;
        }
        pWalker->e_ڪوڊ = e_ڪوڊ_Orig;
        pTab->n_کانو = n_کانو;
      }
#جيڪڏهن_پورو
    }

    /* Locate the index named by the INDEXED BY clause, if any. */
    إذا( pFrom->fg.is_اشاريي_وارو_By && sqlite3_اشاريي_وارو_By_Lookup(pParse, pFrom) ){
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
  لكل(k=0; k<pEList->n_اظهار; k++){
    pE = pEList->a[k].p_اظهار;
    إذا( pE->op==TK_ASTERISK ) اكسر;
    أكد( pE->op!=TK_DOT || pE->p_ساڄو!=0 );
    أكد( pE->op!=TK_DOT || (pE->p_کاٻو!=0 && pE->p_کاٻو->op==TK_ID) );
    إذا( pE->op==TK_DOT && pE->p_ساڄو->op==TK_ASTERISK ) اكسر;
    elistFlags |= pE->flags;
  }
  إذا( k<pEList->n_اظهار ){
    /*
    ** If we get here it means the result set contains one or more "*"
    ** operators that need to be expanded.  Loop through each expression
    ** in the result set and expand them one by one.
    */
    بنية اظهار_List_شي *a = pEList->a;
    اظهار_List *pNew = 0;
    صحيح flags = pParse->db->flags;
    صحيح long_نالا = (flags & SQLITE_ڀريل_کانو_نالا)!=0
                      && (flags & SQLITE_Short_کانو_نالا)==0;

    لكل(k=0; k<pEList->n_اظهار; k++){
      pE = a[k].p_اظهار;
      elistFlags |= pE->flags;
      p_ساڄو = pE->p_ساڄو;
      أكد( pE->op!=TK_DOT || p_ساڄو!=0 );
      إذا( pE->op!=TK_ASTERISK
       && (pE->op!=TK_DOT || p_ساڄو->op!=TK_ASTERISK)
      ){
        /* This particular expression does not need to be expanded.
        */
        pNew = sqlite3_اظهار_List_Append(pParse, pNew, a[k].p_اظهار);
        إذا( pNew ){
          pNew->a[pNew->n_اظهار-1].zEName = a[k].zEName;
          pNew->a[pNew->n_اظهار-1].fg.eEName = a[k].fg.eEName;
          a[k].zEName = 0;
        }
        a[k].p_اظهار = 0;
      }وإلا{
        /* This expression is a "*" or a "TABLE.*" and needs to be
        ** expanded. */
        صحيح جدول_ڏٺل = 0;      /* Set to 1 when TABLE matches */
        حرف *zTName = 0;       /* text of name of TABLE */
        صحيح iErrOfst;
        إذا( pE->op==TK_DOT ){
          أكد( (selFlags & SF_NestedFrom)==0 );
          أكد( pE->p_کاٻو!=0 );
          أكد( !اظهار_Has_Property(pE->p_کاٻو, EP_Int_قدر) );
          zTName = pE->p_کاٻو->u.zToken;
          أكد( اظهار_Use_WOfst(pE->p_کاٻو) );
          iErrOfst = pE->p_ساڄو->w.iOfst;
        }وإلا{
          أكد( pE->p_کاٻو==0 );
          أكد( اظهار_Use_WOfst(pE) );
          iErrOfst = pE->w.iOfst;
        }
        لكل(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
          صحيح n_جوڙيو;                    /* Number of cols including rowid */
          جدول *pTab = pFrom->pSTab;  /* Table for this data source */
          اظهار_List *pNestedFrom;       /* Result-set of a nested FROM clause */
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
            أكد( pNestedFrom->n_اظهار==pTab->n_کانو );
            أكد( VisibleRowid(pTab)==0 || ViewCanHaveRowid );
          }وإلا{
            إذا( zTName && sqlite3StrICmp(zTName, zTabName)!=0 ){
              استمر;
            }
            pNestedFrom = 0;
            iDb = sqlite3_Schema_منزل_اشاريو(db, pTab->pSchema);
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
              p_ساڄو = sqlite3_اظهار(db, TK_ID, zUName);
              sqlite3_اظهار_سيٽ_Error_وٿي(p_ساڄو, iErrOfst);
              pNew = sqlite3_اظهار_List_Append(pParse, pNew, p_ساڄو);
              إذا( pNew ){
                بنية اظهار_List_شي *pX = &pNew->a[pNew->n_اظهار-1];
                أكد( pX->zEName==0 );
                pX->zEName = sqlite3MPrintf(db,"..%s", zUName);
                pX->fg.eEName = ENAME_TAB;
                pX->fg.b_Using_رڪن = 1;
              }
            }
          }وإلا{
            pUsing = 0;
          }

          n_جوڙيو = pTab->n_کانو;
          إذا( VisibleRowid(pTab) && (selFlags & SF_NestedFrom)!=0 ) n_جوڙيو++;
          لكل(j=0; j<n_جوڙيو; j++){
            ثابت حرف *zName; 
            بنية اظهار_List_شي *pX; /* Newly added ExprList term */

            إذا( j==pTab->n_کانو ){
              zName = sqlite3RowidAlias(pTab);
              إذا( zName==0 ) استمر;
            }وإلا{
              zName = pTab->a_کانو[j].zCnName;

              /* If pTab is actually an SF_NestedFrom sub-select, do not
              ** expand any ENAME_ROWID columns.  */
              إذا( pNestedFrom && pNestedFrom->a[j].fg.eEName==ENAME_ROWID ){
                استمر;
              }

              إذا( zTName
               && pNestedFrom
               && sqlite3_ميلاپ_EName(&pNestedFrom->a[j], 0, zTName, 0, 0)==0
              ){
                استمر;
              }

              /* If a column is marked as 'hidden', omit it from the expanded
              ** result-set list unless the SELECT has the SF_IncludeHidden
              ** bit set.
              */
              إذا( (p->selFlags & SF_IncludeHidden)==0
                && Is_Hidden_ڪالم(&pTab->a_کانو[j])
              ){
                استمر;
              }
              إذا( (pTab->a_کانو[j].کانو_Flags & COLFLAG_NOEXPAND)!=0
               && zTName==0
               && (selFlags & (SF_NestedFrom))==0
              ){
                استمر;
              }
            }
            أكد( zName );
            جدول_ڏٺل = 1;

            إذا( i>0 && zTName==0 && (selFlags & SF_NestedFrom)==0 ){
              إذا( pFrom->fg.isUsing
               && sqlite3_Id_List_اشاريو(pFrom->u3.pUsing, zName)>=0
              ){
                /* In a join with a USING clause, omit columns in the
                ** using clause from the table on the right. */
                استمر;
              }
            }
            p_ساڄو = sqlite3_اظهار(db, TK_ID, zName);
            إذا( (pTabList->nSrc>1
                 && (  (pFrom->fg.jointype & JT_LTORJ)==0
                     || zTName!=0
                     || (selFlags & SF_NestedFrom)!=0
                     || !اندريون_Any_Using_Clause(zName,pFrom,pTabList->nSrc-i-1)
                    )
                )
             || اندريون_RENAME_OBJECT
            ){
              اظهار *p_کاٻو;
              p_کاٻو = sqlite3_اظهار(db, TK_ID, zTabName);
              p_اظهار = sqlite3PExpr(pParse, TK_DOT, p_کاٻو, p_ساڄو);
              إذا( اندريون_RENAME_OBJECT && pE->p_کاٻو ){
                أكد( pE->op==TK_DOT && pE->p_ساڄو->op==TK_ASTERISK );
                sqlite3RenameTokenRemap(pParse, p_کاٻو, pE->p_کاٻو);
              }
              إذا( zSchemaName ){
                p_کاٻو = sqlite3_اظهار(db, TK_ID, zSchemaName);
                p_اظهار = sqlite3PExpr(pParse, TK_DOT, p_کاٻو, p_اظهار);
              }
            }وإلا{
              p_اظهار = p_ساڄو;
            }
            sqlite3_اظهار_سيٽ_Error_وٿي(p_اظهار, iErrOfst);
            pNew = sqlite3_اظهار_List_Append(pParse, pNew, p_اظهار);
            إذا( pNew==0 ){
              اكسر;  /* OOM */
            }
            pX = &pNew->a[pNew->n_اظهار-1];
            أكد( pX->zEName==0 );
            إذا( (selFlags & SF_NestedFrom)!=0 && !اندريون_RENAME_OBJECT ){
              إذا( pNestedFrom && (!ViewCanHaveRowid || j<pNestedFrom->n_اظهار) ){
                أكد( j<pNestedFrom->n_اظهار );
                pX->zEName = sqlite3DbStrDup(db, pNestedFrom->a[j].zEName);
                testcase( pX->zEName==0 );
              }وإلا{
                pX->zEName = sqlite3MPrintf(db, "%s.%s.%s",
                                           zSchemaName, zTabName, zName);
                testcase( pX->zEName==0 );
              }
              pX->fg.eEName = (j==pTab->n_کانو ? ENAME_ROWID : ENAME_TAB);
              إذا( (pFrom->fg.isUsing
                   && sqlite3_Id_List_اشاريو(pFrom->u3.pUsing, zName)>=0)
               || (pUsing && sqlite3_Id_List_اشاريو(pUsing, zName)>=0)
               || (j<pTab->n_کانو && (pTab->a_کانو[j].کانو_Flags & COLFLAG_NOEXPAND))
              ){
                pX->fg.bNoExpand = 1;
              }
            }وإلا إذا( long_نالا ){
              pX->zEName = sqlite3MPrintf(db, "%s.%s", zTabName, zName);
              pX->fg.eEName = ENAME_NAME;
            }وإلا{
              pX->zEName = sqlite3DbStrDup(db, zName);
              pX->fg.eEName = ENAME_NAME;
            }
          }
        }
        إذا( !جدول_ڏٺل ){
          إذا( zTName ){
            sqlite3_Error_نياپو(pParse, "no such table: %s", zTName);
          }وإلا{
            sqlite3_Error_نياپو(pParse, "no tables specified");
          }
        }
      }
    }
    sqlite3_اظهار_List_Delete(db, pEList);
    p->pEList = pNew;
  }
  إذا( p->pEList ){
    إذا( p->pEList->n_اظهار>db->a_حد_بندي[SQLITE_حد_بندي_ڪالم] ){
      sqlite3_Error_نياپو(pParse, "too many columns in result set");
      أرجع WRC_Abort;
    }
    إذا( (elistFlags & (EP_HasFunc|EP_Subquery))!=0 ){
      p->selFlags |= SF_Complex_نتيجو;
    }
  }
#إذا TREETRACE_ENABLED
  إذا( sqlite3_وڻ_Trace & 0x8 ){
    TREETRACE(0x8,pParse,p,("After result-set wildcard expansion:\n"));
    sqlite3_وڻ_View_Select(0, p, 0);
  }
#جيڪڏهن_پورو
  أرجع WRC_Continue;
}

#إذا SQLITE_DEBUG
/*
** Always assert.  This xSelectCallback2 implementation proves that the
** xSelectCallback2 is never invoked.
*/
فراغ sqlite3SelectWalkAssert2(Walker *Not_استعمال_ٿيل, Select *NotUsed2){
  اڻ_استعمال_ٿيل_PARAMETER2(Not_استعمال_ٿيل, NotUsed2);
  أكد( 0 );
}
#جيڪڏهن_پورو

/*
** The first argument is a Walker object created by sqlite3SelectExpand().
** This function uses the walker to expand the SELECT statement passed
** as the second argument. If necessary, it also converts any compound
** SELECT statements that have COLLATE clauses on ORDER BY expressions
** to sub-queries. Return the return value of sqlite3WalkSelect().
*/
ساكن صحيح selectExpanderWalk(Walker *pWalker, Select *pSelect){
  أكد( pWalker->x_اظهار_Callback==sqlite3_اظهار_Walk_Noop );
  أكد( pWalker->pParse!=0 );
  إذا( OK_IF_ALWAYS_TRUE(pWalker->pParse->hasCompound) ){
    pWalker->xSelectCallback = convert_Compound_Select_منزل_Subquery;
    pWalker->xSelectCallback2 = 0;
    sqlite3WalkSelect(pWalker, pSelect);
  }
  pWalker->xSelectCallback = selectExpander;
  pWalker->xSelectCallback2 = sqlite3_Select_ڪڍو_With;
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
  w.x_اظهار_Callback = sqlite3_اظهار_Walk_Noop;
  w.pParse = pParse;
  w.e_ڪوڊ = 0;
  selectExpanderWalk(&w, pSelect);
}


#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_SUBQUERY
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
ساكن فراغ select_جوڙيو_Subquery_Type_Info(Walker *pWalker, Select *p){
  Parse *pParse;
  صحيح i;
  SrcList *pTabList;
  Src_شي *pFrom;

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
      sqlite3_Subquery_ڪالم_Types(pParse, pTab, pSel, SQLITE_AFF_NONE);
    }
  }
}
#جيڪڏهن_پورو


/*
** This routine adds datatype and collating sequence information to
** the Table structures of all FROM-clause subqueries in a
** SELECT statement.
**
** Use this routine after name resolution.
*/
ساكن فراغ sqlite3_Select_جوڙيو_Type_Info(Parse *pParse, Select *pSelect){
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_SUBQUERY
  Walker w;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.xSelectCallback2 = select_جوڙيو_Subquery_Type_Info;
  w.x_اظهار_Callback = sqlite3_اظهار_Walk_Noop;
  w.pParse = pParse;
  sqlite3WalkSelect(&w, pSelect);
#جيڪڏهن_پورو
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
  sqlite3_Resolve_Select_نالا(pParse, p, pOuterNC);
  إذا( pParse->nErr ) أرجع;
  sqlite3_Select_جوڙيو_Type_Info(pParse, p);
}

#إذا TREETRACE_ENABLED
/*
** Display all information about an AggInfo object
*/
ساكن فراغ printAggInfo(AggInfo *pAggInfo){
  صحيح ii;
  sqlite3DebugPrintf("AggInfo %d/%p:\n",
     pAggInfo->selId, pAggInfo);
  لكل(ii=0; ii<pAggInfo->n_ڪالم; ii++){
    بنية Agg_Info_کانو *p_کانو = &pAggInfo->a_کانو[ii];
    sqlite3DebugPrintf(
       "agg-column[%d] pTab=%s iTable=%d iColumn=%d iMem=%d"
       " iSorterColumn=%d %s\n",
       ii, p_کانو->pTab ? p_کانو->pTab->zName : "NULL",
       p_کانو->i_جدول, p_کانو->i_ڪالم, pAggInfo->i_پهريون_Reg+ii,
       p_کانو->i_Sorter_ڪالم,
       ii>=pAggInfo->n_گڏ_ڪندڙ ? "" : " Accumulator");
    sqlite3_وڻ_View_اظهار(0, pAggInfo->a_کانو[ii].pCExpr, 0);
  }
  لكل(ii=0; ii<pAggInfo->nFunc; ii++){
    sqlite3DebugPrintf("agg-func[%d]: iMem=%d\n",
        ii, pAggInfo->i_پهريون_Reg+pAggInfo->n_ڪالم+ii);
    sqlite3_وڻ_View_اظهار(0, pAggInfo->aFunc[ii].pFExpr, 0);
  }
}
#جيڪڏهن_پورو /* TREETRACE_ENABLED */

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
ساكن فراغ analyze_Agg_Func_دليلون(
  AggInfo *pAggInfo,
  NameContext *pNC
){
  صحيح i;
  أكد( pAggInfo!=0 );
  أكد( pAggInfo->i_پهريون_Reg==0 );
  pNC->ncFlags |= NC_اندريون_Agg_Func;
  لكل(i=0; i<pAggInfo->nFunc; i++){
    اظهار *p_اظهار = pAggInfo->aFunc[i].pFExpr;
    أكد( p_اظهار->op==TK_FUNCTION || p_اظهار->op==TK_AGG_FUNCTION );
    أكد( اظهار_Use_XList(p_اظهار) );
    sqlite3_اظهار_Analyze_Agg_List(pNC, p_اظهار->x.pList);
    إذا( p_اظهار->p_کاٻو ){
      أكد( p_اظهار->p_کاٻو->op==TK_ترتيب );
      أكد( اظهار_Use_XList(p_اظهار->p_کاٻو) );
      sqlite3_اظهار_Analyze_Agg_List(pNC, p_اظهار->p_کاٻو->x.pList);
    }
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
    أكد( !Is_دري_Func(p_اظهار) );
    إذا( اظهار_Has_Property(p_اظهار, EP_WinFunc) ){
      sqlite3_اظهار_Analyze_Aggregates(pNC, p_اظهار->y.pWin->pFilter);
    }
#جيڪڏهن_پورو
  }
  pNC->ncFlags &= ~NC_اندريون_Agg_Func;
}

/*
** An index on expressions is being used in the inner loop of an
** aggregate query with a GROUP BY clause.  This routine attempts
** to adjust the AggInfo object to take advantage of index and to
** perhaps use the index as a covering index.
**
*/
ساكن فراغ optimize_Aggregate_Use_جو_اشاريي_وارو_اظهار(
  Parse *pParse,          /* Parsing context */
  Select *pSelect,        /* The SELECT statement being processed */
  AggInfo *pAggInfo,      /* The aggregate info */
  NameContext *pNC        /* Name context used to resolve agg-func args */
){
  أكد( pAggInfo->i_پهريون_Reg==0 );
  أكد( pSelect!=0 );
  أكد( pSelect->p_گروهه_By!=0 );
  pAggInfo->n_ڪالم = pAggInfo->n_گڏ_ڪندڙ;
  إذا( ALWAYS(pAggInfo->n_ترتيب_ڏيڻ_ڪالم>0) ){
    صحيح mx = pSelect->p_گروهه_By->n_اظهار - 1;
    صحيح j, k;
    لكل(j=0; j<pAggInfo->n_ڪالم; j++){
      k = pAggInfo->a_کانو[j].i_Sorter_ڪالم;
      إذا( k>mx ) mx = k;
    }
    pAggInfo->n_ترتيب_ڏيڻ_ڪالم = mx+1;
  }
  analyze_Agg_Func_دليلون(pAggInfo, pNC);
#إذا TREETRACE_ENABLED
  إذا( sqlite3_وڻ_Trace & 0x20 ){
    اشاريي_وارو_اظهار *pIEpr;
    TREETRACE(0x20, pParse, pSelect,
        ("AggInfo (possibly) adjusted for Indexed Exprs\n"));
    sqlite3_وڻ_View_Select(0, pSelect, 0);
    لكل(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
      لکو("data-cursor=%d index={%d,%d}\n",
          pIEpr->i_معلومات_Cur, pIEpr->iIdxCur, pIEpr->i_Idx_کانو);
      sqlite3_وڻ_View_اظهار(0, pIEpr->p_اظهار, 0);
    }
    printAggInfo(pAggInfo);
  }
#وإلا
  اڻ_استعمال_ٿيل_PARAMETER(pSelect);
  اڻ_استعمال_ٿيل_PARAMETER(pParse);
#جيڪڏهن_پورو
}

/*
** Walker callback for aggregateConvertIndexedExprRefToColumn().
*/
ساكن صحيح aggregate_Idx_Epr_Ref_منزل_کانو_Callback(Walker *pWalker, اظهار *p_اظهار){
  AggInfo *pAggInfo;
  بنية Agg_Info_کانو *p_کانو;
  اڻ_استعمال_ٿيل_PARAMETER(pWalker);
  إذا( p_اظهار->pAggInfo==0 ) أرجع WRC_Continue;
  إذا( p_اظهار->op==TK_AGG_ڪالم ) أرجع WRC_Continue;
  إذا( p_اظهار->op==TK_AGG_FUNCTION ) أرجع WRC_Continue;
  إذا( p_اظهار->op==TK_IF_NULL_صف ) أرجع WRC_Continue;
  pAggInfo = p_اظهار->pAggInfo;
  إذا( NEVER(p_اظهار->iAgg>=pAggInfo->n_ڪالم) ) أرجع WRC_Continue;
  أكد( p_اظهار->iAgg>=0 );
  p_کانو = &pAggInfo->a_کانو[p_اظهار->iAgg];
  p_اظهار->op = TK_AGG_ڪالم;
  p_اظهار->i_جدول = p_کانو->i_جدول;
  p_اظهار->i_ڪالم = p_کانو->i_ڪالم;
  اظهار_Clear_Property(p_اظهار, EP_Skip|EP_Collate|EP_Unlikely);
  أرجع WRC_Prune;
}

/*
** Convert every pAggInfo->aFunc[].pExpr such that any node within
** those expressions that has pAppInfo set is changed into a TK_AGG_COLUMN
** opcode.
*/
ساكن فراغ aggregate_Convert_اشاريي_وارو_اظهار_Ref_منزل_ڪالم(AggInfo *pAggInfo){
  صحيح i;
  Walker w;
  memset(&w, 0, حجم(w));
  w.x_اظهار_Callback = aggregate_Idx_Epr_Ref_منزل_کانو_Callback;
  لكل(i=0; i<pAggInfo->nFunc; i++){
    sqlite3_Walk_اظهار(&w, pAggInfo->aFunc[i].pFExpr);
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
ساكن فراغ سونپيو_Aggregate_Registers(Parse *pParse, AggInfo *pAggInfo){
  أكد( pAggInfo!=0 );
  أكد( pAggInfo->i_پهريون_Reg==0 );
  pAggInfo->i_پهريون_Reg = pParse->nMem + 1;
  pParse->nMem += pAggInfo->n_ڪالم + pAggInfo->nFunc;
}

/*
** Reset the aggregate accumulator.
**
** The aggregate accumulator is a set of memory cells that hold
** intermediate results while calculating an aggregate.  This
** routine generates code that stores NULLs in all of those memory
** cells.
*/
ساكن فراغ reset_گڏ_ڪندڙ(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  صحيح i;
  بنية AggInfo_func *pFunc;
  صحيح nReg = pAggInfo->nFunc + pAggInfo->n_ڪالم;
  أكد( pAggInfo->i_پهريون_Reg>0 );
  أكد( pParse->db->pParse==pParse );
  أكد( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  إذا( nReg==0 ) أرجع;
  إذا( pParse->nErr ) أرجع;
  sqlite3_Vdbe_جوڙيو_Op3(v, OP_Null, 0, pAggInfo->i_پهريون_Reg,
                    pAggInfo->i_پهريون_Reg+nReg-1);
  لكل(pFunc=pAggInfo->aFunc, i=0; i<pAggInfo->nFunc; i++, pFunc++){
    إذا( pFunc->iDistinct>=0 ){
      اظهار *pE = pFunc->pFExpr;
      أكد( اظهار_Use_XList(pE) );
      إذا( pE->x.pList==0 || pE->x.pList->n_اظهار!=1 ){
        sqlite3_Error_نياپو(pParse, "DISTINCT aggregates must have exactly one "
           "argument");
        pFunc->iDistinct = -1;
      }وإلا{
        ڪنجي_Info *p_ڪنجي_Info = sqlite3_ڪنجي_Info_From_اظهار_List(pParse, pE->x.pList,0,0);
        pFunc->iDistAddr = sqlite3_Vdbe_جوڙيو_Op4(v, OP_OpenEphemeral,
            pFunc->iDistinct, 0, 0, (حرف*)p_ڪنجي_Info, P4_KEYINFO);
        ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(DISTINCT)",
                          pFunc->pFunc->zName));
      }
    }
    إذا( pFunc->iOBTab>=0 ){
      اظهار_List *pOBList;
      ڪنجي_Info *p_ڪنجي_Info;
      صحيح nExtra = 0;
      أكد( pFunc->pFExpr->p_کاٻو!=0 );
      أكد( pFunc->pFExpr->p_کاٻو->op==TK_ترتيب );
      أكد( اظهار_Use_XList(pFunc->pFExpr->p_کاٻو) );
      أكد( pFunc->pFunc!=0 );
      pOBList = pFunc->pFExpr->p_کاٻو->x.pList;
      إذا( !pFunc->bOBUnique ){
        nExtra++;  /* One extra column for the OP_Sequence */
      }
      إذا( pFunc->bOBPayload ){
        /* extra columns for the function arguments */
        أكد( اظهار_Use_XList(pFunc->pFExpr) );
        أكد( pFunc->pFExpr->x.pList!=0 );
        nExtra += pFunc->pFExpr->x.pList->n_اظهار;
      }
      إذا( pFunc->bUseSubtype ){
        nExtra += pFunc->pFExpr->x.pList->n_اظهار;
      }
      p_ڪنجي_Info = sqlite3_ڪنجي_Info_From_اظهار_List(pParse, pOBList, 0, nExtra);
      إذا( !pFunc->bOBUnique && pParse->nErr==0 ){
        p_ڪنجي_Info->n_ڪنجي_ميدان++;
      }
      sqlite3_Vdbe_جوڙيو_Op4(v, OP_OpenEphemeral,
            pFunc->iOBTab, pOBList->n_اظهار+nExtra, 0,
            (حرف*)p_ڪنجي_Info, P4_KEYINFO);
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
    اظهار_List *pList;
    أكد( اظهار_Use_XList(pF->pFExpr) );
    إذا( pParse->nErr ) أرجع;
    pList = pF->pFExpr->x.pList;
    إذا( pF->iOBTab>=0 ){
      /* For an ORDER BY aggregate, calls to OP_AggStep were deferred.  Inputs
      ** were stored in emphermal table pF->iOBTab.  Here, we extract those
      ** inputs (in ORDER BY order) and make all calls to OP_AggStep
      ** before doing the OP_AggFinal call. */
      صحيح i_چوٽي;        /* Start of loop for extracting columns */
      صحيح nArg;        /* Number of columns to extract */
      صحيح n_ڪنجي;        /* Key columns to be skipped */
      صحيح regAgg;      /* Extract into this array */
      صحيح j;           /* Loop counter */
     
      أكد( pF->pFunc!=0 );
      nArg = pList->n_اظهار;
      regAgg = sqlite3_Get_عارضي_Range(pParse, nArg);

      إذا( pF->bOBPayload==0 ){
        n_ڪنجي = 0;
      }وإلا{
        أكد( pF->pFExpr->p_کاٻو!=0 );
        أكد( اظهار_Use_XList(pF->pFExpr->p_کاٻو) );
        أكد( pF->pFExpr->p_کاٻو->x.pList!=0 );
        n_ڪنجي = pF->pFExpr->p_کاٻو->x.pList->n_اظهار;
        إذا( ALWAYS(!pF->bOBUnique) ) n_ڪنجي++;
      }
      i_چوٽي = sqlite3_Vdbe_جوڙيو_Op1(v, OP_Rewind, pF->iOBTab); VdbeCoverage(v);
      لكل(j=nArg-1; j>=0; j--){
        sqlite3_Vdbe_جوڙيو_Op3(v, OP_ڪالم, pF->iOBTab, n_ڪنجي+j, regAgg+j);
      }
      إذا( pF->bUseSubtype ){
        صحيح regSubtype = sqlite3_Get_عارضي_Reg(pParse);
        صحيح i_بنياد_کانو = n_ڪنجي + nArg + (pF->bOBPayload==0 && pF->bOBUnique==0);
        لكل(j=nArg-1; j>=0; j--){
          sqlite3_Vdbe_جوڙيو_Op3(v, OP_ڪالم, pF->iOBTab, i_بنياد_کانو+j, regSubtype);
          sqlite3_Vdbe_جوڙيو_Op2(v, OP_سيٽ_Subtype, regSubtype, regAgg+j);
        }
        sqlite3_Release_عارضي_Reg(pParse, regSubtype);
      }
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Agg_قدم, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_ايندڙ, pF->iOBTab, i_چوٽي+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_هتي(v, i_چوٽي);
      sqlite3_Release_عارضي_Range(pParse, regAgg, nArg);
    }
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_AggFinal, AggInfoFuncReg(pAggInfo,i),
                      pList ? pList->n_اظهار : 0);
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
ساكن فراغ update_گڏ_ڪندڙ(
  Parse *pParse,
  صحيح regAcc,
  AggInfo *pAggInfo,
  صحيح eDistinctType
){
  Vdbe *v = pParse->pVdbe;
  صحيح i;
  صحيح reg_ڪاميابي = 0;
  صحيح addr_ڪاميابي_Test = 0;
  بنية AggInfo_func *pF;
  بنية Agg_Info_کانو *pC;

  أكد( pAggInfo->i_پهريون_Reg>0 );
  إذا( pParse->nErr ) أرجع;
  pAggInfo->سڌو_گهڻائي = 1;
  لكل(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    صحيح nArg;
    صحيح addr_ايندڙ = 0;
    صحيح regAgg;
    صحيح regAggSz = 0;
    صحيح regDistinct = 0;
    اظهار_List *pList;
    أكد( اظهار_Use_XList(pF->pFExpr) );
    أكد( !Is_دري_Func(pF->pFExpr) );
    أكد( pF->pFunc!=0 );
    pList = pF->pFExpr->x.pList;
    إذا( اظهار_Has_Property(pF->pFExpr, EP_WinFunc) ){
      اظهار *pFilter = pF->pFExpr->y.pWin->pFilter;
      إذا( pAggInfo->n_گڏ_ڪندڙ
       && (pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)
       && regAcc
      ){
        /* If regAcc==0, there there exists some min() or max() function
        ** without a FILTER clause that will ensure the magnet registers
        ** are populated. */
        إذا( reg_ڪاميابي==0 ) reg_ڪاميابي = ++pParse->nMem;
        /* If this is the first row of the group (regAcc contains 0), clear the
        ** "magnet" register regHit so that the accumulator registers
        ** are populated if the FILTER clause jumps over the the
        ** invocation of min() or max() altogether. Or, if this is not
        ** the first row (regAcc contains 1), set the magnet register so that
        ** the accumulators are not populated unless the min()/max() is invoked
        ** and indicates that they should be.  */
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_Copy, regAcc, reg_ڪاميابي);
      }
      addr_ايندڙ = sqlite3VdbeMakeLabel(pParse);
      sqlite3_اظهار_If_False(pParse, pFilter, addr_ايندڙ, SQLITE_JUMPIFNULL);
    }
    إذا( pF->iOBTab>=0 ){
      /* Instead of invoking AggStep, we must push the arguments that would
      ** have been passed to AggStep onto the sorting table. */
      صحيح jj;                /* Registered used so far in building the record */
      اظهار_List *pOBList;     /* The ORDER BY clause */
      أكد( pList!=0 );
      nArg = pList->n_اظهار;
      أكد( nArg>0 );
      أكد( pF->pFExpr->p_کاٻو!=0 );
      أكد( pF->pFExpr->p_کاٻو->op==TK_ترتيب );
      أكد( اظهار_Use_XList(pF->pFExpr->p_کاٻو) );
      pOBList = pF->pFExpr->p_کاٻو->x.pList;
      أكد( pOBList!=0 );
      أكد( pOBList->n_اظهار>0 );
      regAggSz = pOBList->n_اظهار;
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
      regAgg = sqlite3_Get_عارضي_Range(pParse, regAggSz);
      regDistinct = regAgg;
      sqlite3_اظهار_ڪوڊ_اظهار_List(pParse, pOBList, regAgg, 0, SQLITE_ECEL_DUP);
      jj = pOBList->n_اظهار;
      إذا( !pF->bOBUnique ){
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_Sequence, pF->iOBTab, regAgg+jj);
        jj++;
      }
      إذا( pF->bOBPayload ){
        regDistinct = regAgg+jj;
        sqlite3_اظهار_ڪوڊ_اظهار_List(pParse, pList, regDistinct, 0, SQLITE_ECEL_DUP);
        jj += nArg;
      }
      إذا( pF->bUseSubtype ){
        صحيح kk;
        صحيح reg_بنياد = pF->bOBPayload ? regDistinct : regAgg;
        لكل(kk=0; kk<nArg; kk++, jj++){
          sqlite3_Vdbe_جوڙيو_Op2(v, OP_GetSubtype, reg_بنياد+kk, regAgg+jj);
        }
      }
    }وإلا إذا( pList ){
      nArg = pList->n_اظهار;
      regAgg = sqlite3_Get_عارضي_Range(pParse, nArg);
      regDistinct = regAgg;
      sqlite3_اظهار_ڪوڊ_اظهار_List(pParse, pList, regAgg, 0, SQLITE_ECEL_DUP);
    }وإلا{
      nArg = 0;
      regAgg = 0;
    }
    إذا( pF->iDistinct>=0 && pList ){
      إذا( addr_ايندڙ==0 ){
        addr_ايندڙ = sqlite3VdbeMakeLabel(pParse);
      }
      pF->iDistinct = ڪوڊ_Distinct(pParse, eDistinctType,
          pF->iDistinct, addr_ايندڙ, pList, regDistinct);
    }
    إذا( pF->iOBTab>=0 ){
      /* Insert a new record into the ORDER BY table */
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Make_رڪارڊ, regAgg, regAggSz-1,
                        regAgg+regAggSz-1);
      sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_Idx_وجهو, pF->iOBTab, regAgg+regAggSz-1,
                           regAgg, regAggSz-1);
      sqlite3_Release_عارضي_Range(pParse, regAgg, regAggSz);
    }وإلا{
      /* Invoke the AggStep function */
      إذا( pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        CollSeq *pColl = 0;
        بنية اظهار_List_شي *p_شي;
        صحيح j;
        أكد( pList!=0 );  /* pList!=0 if pF->pFunc has NEEDCOLL */
        لكل(j=0, p_شي=pList->a; !pColl && j<nArg; j++, p_شي++){
          pColl = sqlite3_اظهار_Coll_Seq(pParse, p_شي->p_اظهار);
        }
        إذا( !pColl ){
          pColl = pParse->db->pDfltColl;
        }
        إذا( reg_ڪاميابي==0 && pAggInfo->n_گڏ_ڪندڙ ) reg_ڪاميابي = ++pParse->nMem;
        sqlite3_Vdbe_جوڙيو_Op4(v, OP_CollSeq, reg_ڪاميابي, 0, 0,
                         (حرف *)pColl, P4_COLLSEQ);
      }
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Agg_قدم, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3_Release_عارضي_Range(pParse, regAgg, nArg);
    }
    إذا( addr_ايندڙ ){
      sqlite3VdbeResolveLabel(v, addr_ايندڙ);
    }
    إذا( pParse->nErr ) أرجع;
  }
  إذا( reg_ڪاميابي==0 && pAggInfo->n_گڏ_ڪندڙ ){
    reg_ڪاميابي = regAcc;
  }
  إذا( reg_ڪاميابي ){
    addr_ڪاميابي_Test = sqlite3_Vdbe_جوڙيو_Op1(v, OP_If, reg_ڪاميابي); VdbeCoverage(v);
  }
  لكل(i=0, pC=pAggInfo->a_کانو; i<pAggInfo->n_گڏ_ڪندڙ; i++, pC++){
    sqlite3_اظهار_ڪوڊ(pParse, pC->pCExpr, Agg_Info_ڪالم_Reg(pAggInfo,i));
    إذا( pParse->nErr ) أرجع;
  }

  pAggInfo->سڌو_گهڻائي = 0;
  إذا( addr_ڪاميابي_Test ){
    sqlite3_Vdbe_Jump_هتي_Or_ڪڍو_Inst(v, addr_ڪاميابي_Test);
  }
}

/*
** Add a single OP_Explain instruction to the VDBE to explain a simple
** count(*) query ("SELECT count(*) FROM pTab").
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_EXPLAIN
ساكن فراغ explain_Simple_ڳڻپ(
  Parse *pParse,                  /* Parse context */
  جدول *pTab,                    /* Table being queried */
  اشاريو *pIdx                     /* Index used to optimize scan, or NULL */
){
  إذا( pParse->explain==2 ){
    صحيح bCover = (pIdx!=0 && (HasRowid(pTab) || !Is_Primary_ڪنجي_اشاريو(pIdx)));
    sqlite3VdbeExplain(pParse, 0, "SCAN %s%s%s",
        pTab->zName,
        bCover ? " USING COVERING INDEX " : "",
        bCover ? pIdx->zName : ""
    );
  }
}
#وإلا
# define explain_Simple_ڳڻپ(a,b,c)
#جيڪڏهن_پورو

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
ساكن صحيح having_منزل_Where_اظهار_Cb(Walker *pWalker, اظهار *p_اظهار){
  إذا( p_اظهار->op!=TK_AND ){
    Select *pS = pWalker->u.pSelect;
    /* This routine is called before the HAVING clause of the current
    ** SELECT is analyzed for aggregates. So if pExpr->pAggInfo is set
    ** here, it indicates that the expression is a correlated reference to a
    ** column from an outer aggregate query, or an aggregate function that
    ** belongs to an outer query. Do not move the expression to the WHERE
    ** clause in this obscure case, as doing so may corrupt the outer Select
    ** statements AggInfo structure.  */
    إذا( sqlite3_اظهار_Is_Constant_Or_گروهه_By(pWalker->pParse, p_اظهار, pS->p_گروهه_By)
     && اظهار_Always_False(p_اظهار)==0
     && p_اظهار->pAggInfo==0
    ){
      sqlite3 *db = pWalker->pParse->db;
      اظهار *pNew = sqlite3_اظهار_Int32(db, 1);
      إذا( pNew ){
        اظهار *pWhere = pS->pWhere;
        مٽا_سٽا(اظهار, *pNew, *p_اظهار);
        pNew = sqlite3_اظهار_And(pWalker->pParse, pWhere, pNew);
        pS->pWhere = pNew;
        pWalker->e_ڪوڊ = 1;
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
  sWalker.x_اظهار_Callback = having_منزل_Where_اظهار_Cb;
  sWalker.u.pSelect = p;
  sqlite3_Walk_اظهار(&sWalker, p->pHaving);
#إذا TREETRACE_ENABLED
  إذا( sWalker.e_ڪوڊ && (sqlite3_وڻ_Trace & 0x100)!=0 ){
    TREETRACE(0x100,pParse,p,("Move HAVING terms into WHERE:\n"));
    sqlite3_وڻ_View_Select(0, p, 0);
  }
#جيڪڏهن_پورو
}

/*
** Check to see if the pThis entry of pTabList is a self-join of another view.
** Search FROM-clause entries in the range of iFirst..iEnd, including iFirst
** but stopping before iEnd.
**
** If pThis is a self-join, then return the SrcItem for the first other
** instance of that view found.  If pThis is not a self-join then return 0.
*/
ساكن Src_شي *isSelfJoinView(
  SrcList *pTabList,           /* Search for self-joins in this FROM clause */
  Src_شي *pThis,              /* Search for prior reference to this subquery */
  صحيح i_پهريون, صحيح i_پڇاڙي        /* Range of FROM-clause entries to search. */
){
  Src_شي *p_شي;
  Select *pSel;
  أكد( pThis->fg.isSubquery );
  pSel = pThis->u4.pSubq->pSelect;
  أكد( pSel!=0 );
  إذا( pSel->selFlags & SF_ڌڪو_هيٺ_ڏانهن ) أرجع 0;
  طالما( i_پهريون<i_پڇاڙي ){
    Select *pS1;
    p_شي = &pTabList->a[i_پهريون++];
    إذا( !p_شي->fg.isSubquery ) استمر;
    إذا( p_شي->fg.ذريعي_Coroutine ) استمر;
    إذا( p_شي->zName==0 ) استمر;
    أكد( p_شي->pSTab!=0 );
    أكد( pThis->pSTab!=0 );
    إذا( p_شي->pSTab->pSchema!=pThis->pSTab->pSchema ) استمر;
    إذا( sqlite3_stricmp(p_شي->zName, pThis->zName)!=0 ) استمر;
    pS1 = p_شي->u4.pSubq->pSelect;
    إذا( p_شي->pSTab->pSchema==0 && pSel->selId!=pS1->selId ){
      /* The query flattener left two different CTE tables with identical
      ** names in the same FROM clause. */
      استمر;
    }
    إذا( pS1->selFlags & SF_ڌڪو_هيٺ_ڏانهن ){
      /* The view was modified by some other optimization such as
      ** pushDownWhereTerms() */
      استمر;
    }
    أرجع p_شي;
  }
  أرجع 0;
}

/*
** Deallocate a single AggInfo object
*/
ساكن فراغ agginfoFree(sqlite3 *db, فراغ *pArg){
  AggInfo *p = (AggInfo*)pArg;
  sqlite3DbFree(db, p->a_کانو);
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
ساكن صحيح ڳڻپ_جو_View_Optimization(Parse *pParse, Select *p){
  Select *pSub, *p_اڳواٽ;
  اظهار *p_اظهار;
  اظهار *p_ڳڻپ;
  sqlite3 *db;
  Src_شي *pFrom;
  إذا( (p->selFlags & SF_Aggregate)==0 ) أرجع 0;   /* This is an aggregate */
  إذا( p->pEList->n_اظهار!=1 ) أرجع 0;               /* Single result column */
  إذا( p->pWhere ) أرجع 0;
  إذا( p->pHaving ) أرجع 0;
  إذا( p->p_گروهه_By ) أرجع 0;
  إذا( p->p_ترتيب_By ) أرجع 0;
  p_اظهار = p->pEList->a[0].p_اظهار;
  إذا( p_اظهار->op!=TK_AGG_FUNCTION ) أرجع 0;        /* Result is an aggregate */
  أكد( اظهار_Use_UToken(p_اظهار) );
  إذا( sqlite3_stricmp(p_اظهار->u.zToken,"count") ) أرجع 0;  /* Is count() */
  أكد( اظهار_Use_XList(p_اظهار) );
  إذا( p_اظهار->x.pList!=0 ) أرجع 0;                 /* Must be count(*) */
  إذا( p->pSrc->nSrc!=1 ) أرجع 0;                  /* One table in FROM  */
  إذا( اظهار_Has_Property(p_اظهار, EP_WinFunc) ) أرجع 0;/* Not a window function */
  pFrom = p->pSrc->a;
  إذا( pFrom->fg.isSubquery==0 ) أرجع 0;           /* FROM is a subquery */
  إذا( (p->selFlags & SF_Correlated)!=0 ) أرجع 0;  /* Not a correlated subq */
  pSub = pFrom->u4.pSubq->pSelect;
  إذا( pSub->p_اڳواٽ==0 ) أرجع 0;                   /* Must be a compound */
  إذا( pSub->selFlags & SF_CopyCte ) أرجع 0;       /* Not a CTE */
  افعل{
    إذا( pSub->op!=TK_ALL && pSub->p_اڳواٽ ) أرجع 0;  /* Must be UNION ALL */
    إذا( pSub->pWhere ) أرجع 0;                      /* No WHERE clause */
    إذا( pSub->p_حد_بندي ) أرجع 0;                      /* No LIMIT clause */
    إذا( pSub->selFlags & (SF_Aggregate|SF_Distinct) ){
       testcase( pSub->selFlags & SF_Aggregate );
       testcase( pSub->selFlags & SF_Distinct );
       أرجع 0;     /* Not an aggregate nor DISTINCT */
    }
    أكد( pSub->pHaving==0 );  /* Due to the previous */
    pSub = pSub->p_اڳواٽ;                              /* Repeat over compound */
  }طالما( pSub );

  /* If we reach this point then it is OK to perform the transformation */

  db = pParse->db;
  p_ڳڻپ = p_اظهار;
  p_اظهار = 0;
  pSub = sqlite3SubqueryDetach(db, pFrom);
  sqlite3SrcListDelete(db, p->pSrc);
  p->pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  طالما( pSub ){
    اظهار *p_رڪن;
    p_اڳواٽ = pSub->p_اڳواٽ;
    pSub->p_اڳواٽ = 0;
    pSub->p_ايندڙ = 0;
    pSub->selFlags |= SF_Aggregate;
    pSub->selFlags &= ~(u32)SF_Compound;
    pSub->n_Select_صف = 0;
    sqlite3_Parser_جوڙيو_Cleanup(pParse, sqlite3_اظهار_List_Delete_Generic, pSub->pEList);
    p_رڪن = p_اڳواٽ ? sqlite3_اظهار_Dup(db, p_ڳڻپ, 0) : p_ڳڻپ;
    pSub->pEList = sqlite3_اظهار_List_Append(pParse, 0, p_رڪن);
    p_رڪن = sqlite3PExpr(pParse, TK_SELECT, 0, 0);
    sqlite3_PExpr_جوڙيو_Select(pParse, p_رڪن, pSub);
    إذا( p_اظهار==0 ){
      p_اظهار = p_رڪن;
    }وإلا{
      p_اظهار = sqlite3PExpr(pParse, TK_PLUS, p_رڪن, p_اظهار);
    }
    إذا( pParse->nErr ){
      sqlite3_اظهار_Delete(db, p_اظهار);
      p_اظهار = 0;
    }
    pSub = p_اڳواٽ;
  }
  p->pEList->a[0].p_اظهار = p_اظهار;
  p->selFlags &= ~(u32)SF_Aggregate;

#إذا TREETRACE_ENABLED
  إذا( sqlite3_وڻ_Trace & 0x200 ){
    TREETRACE(0x200,pParse,p,("After count-of-view optimization:\n"));
    sqlite3_وڻ_View_Select(0, p, 0);
  }
#جيڪڏهن_پورو
  أرجع 1;
}

/*
** If any term of pSrc, or any SF_NestedFrom sub-query, is not the same
** as pSrcItem but has the same alias as p0, then return true.
** Otherwise return false.
*/
ساكن صحيح sameSrcAlias(Src_شي *p0, SrcList *pSrc){
  صحيح i;
  لكل(i=0; i<pSrc->nSrc; i++){
    Src_شي *p1 = &pSrc->a[i];
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
ساكن صحيح from_Clause_رڪن_Can_Be_Coroutine(
  Parse *pParse,          /* Parsing context */
  SrcList *pTabList,      /* FROM clause */
  صحيح i,                  /* Which term of the FROM clause holds the subquery */
  صحيح selFlags            /* Flags on the SELECT statement */
){
  Src_شي *p_شي = &pTabList->a[i];
  إذا( p_شي->fg.isCte ){
    ثابت CteUse *pCteUse = p_شي->u2.pCteUse;
    إذا( pCteUse->eM10d==M10d_Yes ) أرجع 0;                          /* (2a) */
    إذا( pCteUse->nUse>=2 && pCteUse->eM10d!=M10d_No ) أرجع 0;       /* (2b) */
  }
  إذا( pTabList->a[0].fg.jointype & JT_LTORJ ) أرجع 0;               /* (3)  */
  إذا( OptimizationDisabled(pParse->db, SQLITE_Coroutines) ) أرجع 0; /* (4)  */
  إذا( isSelfJoinView(pTabList, p_شي, i+1, pTabList->nSrc)!=0 ){
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
    إذا( p_شي->fg.jointype & (JT_OUTER|JT_CROSS)  ) أرجع 0;     /* (1c-ii) */
    إذا( i==0 ) اكسر;
    i--;
    p_شي--;
    إذا( p_شي->fg.isSubquery ) أرجع 0;                          /* (1c-i) */
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
  اظهار *pWhere    /* part of the WHERE clause currently being examined */
){
  إذا( pParse->nErr==0
   && pWhere!=0
   && !اظهار_Has_Property(pWhere, EP_OuterON|EP_InnerON) 
   && ALWAYS(p->pSrc!=0)
   && p->pSrc->nSrc<BMS
   && (p->p_حد_بندي==0 || p->p_حد_بندي->p_ساڄو==0)
  ){
    إذا( pWhere->op==TK_AND ){
      اظهار *p_ساڄو = pWhere->p_ساڄو;
      exists_منزل_Join(pParse, p, pWhere->p_کاٻو);
      exists_منزل_Join(pParse, p, p_ساڄو);
    }
    وإلا إذا( pWhere->op==TK_EXISTS ){
      Select *pSub = pWhere->x.pSelect;
      اظهار *pSubWhere = pSub->pWhere;
      إذا( pSub->pSrc->nSrc==1
       && (pSub->selFlags & SF_Aggregate)==0
       && !pSub->pSrc->a[0].fg.isSubquery
       && pSub->p_حد_بندي==0
       && pSub->p_اڳواٽ==0
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
        pWhere->u.i_قدر = 1;
        اظهار_سيٽ_Property(pWhere, EP_Int_قدر);
        أكد( p->pWhere!=0 );
        pSub->pSrc->a[0].fg.fromExists = 1;
        p->pSrc = sqlite3SrcListAppendList(pParse, p->pSrc, pSub->pSrc);
        إذا( pSubWhere ){
          p->pWhere = sqlite3PExpr(pParse, TK_AND, p->pWhere, pSubWhere);
          pSub->pWhere = 0;
        }
        pSub->pSrc = 0;
        sqlite3_Parser_جوڙيو_Cleanup(pParse, sqlite3SelectDeleteGeneric, pSub);
        recompute_Columns_استعمال_ٿيل(p, &p->pSrc->a[p->pSrc->nSrc-1]);
#إذا TREETRACE_ENABLED
        إذا( sqlite3_وڻ_Trace & 0x100000 ){
          TREETRACE(0x100000,pParse,p,
                    ("After EXISTS-to-JOIN optimization:\n"));
          sqlite3_وڻ_View_Select(0, p, 0);
        }
#جيڪڏهن_پورو
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
  CheckOnCtx *p_والد; /* Parent context */
};

/*
** True if the SrcList passed as the only argument contains at least
** one RIGHT or FULL JOIN. False otherwise.  
*/
#وصف_ڪريو has_ساڄو_Join(pSrc) (((pSrc)->a[0].fg.jointype & JT_LTORJ)!=0)

/*
** The xExpr callback for the search of invalid ON clause terms.
*/
ساكن صحيح select_Check_On_Clauses_اظهار(Walker *pWalker, اظهار *p_اظهار){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;

  /* Check if pExpr is root or near-root of an ON clause constraint that needs
  ** to be checked to ensure that it does not refer to tables in its FROM
  ** clause to the right of itself. i.e. it is either:
  **
  **   + an ON clause on an OUTER join, or
  **   + an ON clause on an INNER join within a FROM that features at
  **     least one RIGHT or FULL join.
  */
  إذا( (اظهار_Has_Property(p_اظهار, EP_OuterON))
   || (اظهار_Has_Property(p_اظهار, EP_InnerON) && has_ساڄو_Join(pCtx->pSrc))
  ){
    /* If CheckOnCtx.iJoin is already set, then fall through and process
    ** this expression node as normal. Or, if CheckOnCtx.iJoin is still 0,
    ** set it to the cursor number of the RHS of the join to which this
    ** ON expression was attached and then iterate through the entire 
    ** expression.  */
    أكد( pCtx->iJoin<0 || pCtx->iJoin==p_اظهار->w.iJoin );
    إذا( pCtx->iJoin<0 ){
      pCtx->iJoin = p_اظهار->w.iJoin;
      sqlite3_Walk_اظهار_NN(pWalker, p_اظهار);
      pCtx->iJoin = -1;
      أرجع WRC_Prune;
    }
  }

  إذا( p_اظهار->op==TK_ڪالم ){
    /* A column expression. Find the SrcList (if any) to which it refers.
    ** Then, if CheckOnCtx.iJoin indicates that this expression is part of an
    ** ON clause from that SrcList (i.e. if iJoin is non-zero), check that it
    ** does not refer to a table to the right of CheckOnCtx.iJoin. */
    صحيح iTab = p_اظهار->i_جدول;
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
            sqlite3_Error_نياپو(pWalker->pParse, 
                "%s references tables to its right",
                (pCtx->bFuncArg ? "table-function argument" : "ON clause")
            );
            أرجع WRC_Abort;
          }
        }
        اكسر;
      }
      pCtx = pCtx->p_والد;
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
    sCtx.p_والد = pCtx;
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
  w.x_اظهار_Callback = select_Check_On_Clauses_اظهار;
  w.xSelectCallback = selectCheckOnClausesSelect;
  w.u.pCheckOnCtx = &sCtx;
  memset(&sCtx, 0, حجم(sCtx));
  sCtx.pSrc = pSelect->pSrc;
  sCtx.iJoin = -1;
  sqlite3_Walk_اظهار(&w, pSelect->pWhere);
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
    Src_شي *p_شي = &pSelect->pSrc->a[ii];
    إذا( p_شي->fg.isTabFunc ){
      sCtx.iJoin = -1;
      إذا( (p_شي->fg.jointype & (JT_LTORJ|JT_ساڄو)) ){
        sCtx.iJoin = p_شي->iCursor;
      }وإلا{
        صحيح jj;
        لكل(jj=ii+1; jj<pSelect->pSrc->nSrc; jj++){
          Src_شي *pItem2 = &pSelect->pSrc->a[jj];
          إذا( pItem2->fg.jointype & JT_OUTER ){
            sCtx.iJoin = pItem2[-1].iCursor;
            اكسر;
          }
        }
      }
      إذا( sCtx.iJoin>=0 ){
        sqlite3_Walk_اظهار_List(&w, p_شي->u1.pFuncArg);
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
ساكن صحيح sqlite3_Copy_ترتيب_ڏيو_ترتيب(اظهار_List *p1, اظهار_List *p2){
  أكد( p1 );
  إذا( p2 && p1->n_اظهار==p2->n_اظهار ){
    صحيح ii;
    لكل(ii=0; ii<p1->n_اظهار; ii++){
      u8 ترتيب_ڏيو_Flags;
      ترتيب_ڏيو_Flags = p2->a[ii].fg.ترتيب_ڏيو_Flags & KEYINFO_ترتيب_DESC;
      p1->a[ii].fg.ترتيب_ڏيو_Flags = ترتيب_ڏيو_Flags;
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
  اظهار_List *pEList = 0;  /* List of columns to extract. */
  SrcList *pTabList;     /* List of tables to select from */
  اظهار *pWhere;          /* The WHERE clause.  May be NULL */
  اظهار_List *p_گروهه_By;    /* The GROUP BY clause.  May be NULL */
  اظهار *pHaving;         /* The HAVING clause.  May be NULL */
  AggInfo *pAggInfo = 0; /* Aggregate information */
  صحيح rc = 1;            /* Value to return from this function */
  DistinctCtx sDistinct; /* Info on how to code the DISTINCT keyword */
  ترتيب_ڏيو_Ctx s_ترتيب_ڏيو;         /* Info on how to code the ORDER BY clause */
  صحيح i_پڇاڙي;              /* Address of the end of the query */
  sqlite3 *db;           /* The database connection */
  اظهار_List *p_Min_Max_ترتيب_By = 0;  /* Added ORDER BY for min/max queries */
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
  إذا( sqlite3_وڻ_Trace & 0x10000 ){
    إذا( (sqlite3_وڻ_Trace & 0x10001)==0x10000 ){
      sqlite3_وڻ_View_سٽ(0, "In sqlite3Select() at %s:%d",
                           __FILE__, __LINE__);
    }
    sqlite3ShowSelect(p);
  }
#جيڪڏهن_پورو

  /* tag-select-0100 */
  أكد( p->p_ترتيب_By==0 || pDest->eDest!=SRT_DistFifo );
  أكد( p->p_ترتيب_By==0 || pDest->eDest!=SRT_Fifo );
  أكد( p->p_ترتيب_By==0 || pDest->eDest!=SRT_Dist_قطار );
  أكد( p->p_ترتيب_By==0 || pDest->eDest!=SRT_قطار );
  إذا( IgnorableDistinct(pDest) ){
    أكد(pDest->eDest==SRT_Exists     || pDest->eDest==SRT_Discard ||
           pDest->eDest==SRT_Dist_قطار  || pDest->eDest==SRT_DistFifo );
    /* All of these destinations are also able to ignore the ORDER BY clause */
    إذا( p->p_ترتيب_By ){
#إذا TREETRACE_ENABLED
      TREETRACE(0x800,pParse,p, ("dropping superfluous ORDER BY:\n"));
      إذا( sqlite3_وڻ_Trace & 0x800 ){
        sqlite3_وڻ_View_اظهار_List(0, p->p_ترتيب_By, 0, "ORDERBY");
      }
#جيڪڏهن_پورو   
      sqlite3_Parser_جوڙيو_Cleanup(pParse, sqlite3_اظهار_List_Delete_Generic,
                              p->p_ترتيب_By);
      testcase( pParse->earlyCleanup );
      p->p_ترتيب_By = 0;
    }
    p->selFlags &= ~(u32)SF_Distinct;
  }
  sqlite3SelectPrep(pParse, p, 0);
  إذا( pParse->nErr ){
    اذهب select_پڇاڙي;
  }
  أكد( db->mallocFailed==0 );
  أكد( p->pEList!=0 );
#إذا TREETRACE_ENABLED
  إذا( sqlite3_وڻ_Trace & 0x10 ){
    TREETRACE(0x10,pParse,p, ("after name resolution:\n"));
    sqlite3_وڻ_View_Select(0, p, 0);
  }
#جيڪڏهن_پورو

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
    Src_شي *p0 = &p->pSrc->a[0];
    إذا( sameSrcAlias(p0, p->pSrc) ){
      sqlite3_Error_نياپو(pParse,
          "target object/alias may not appear in FROM clause: %s",
          p0->zAlias ? p0->zAlias : p0->pSTab->zName
      );
      اذهب select_پڇاڙي;
    }

    /* Clear the SF_UFSrcCheck flag. The check has already been performed,
    ** and leaving this flag set can cause errors if a compound sub-query
    ** in p->pSrc is flattened into this query and this function called
    ** again as part of compound SELECT processing.  */
    p->selFlags &= ~(u32)SF_UFSrcCheck;
  }

  إذا( pDest->eDest==SRT_Output ){
    sqlite3_Generate_ڪالم_نالا(pParse, p);
  }

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
  إذا( sqlite3_دري_Rewrite(pParse, p) ){
    أكد( pParse->nErr );
    اذهب select_پڇاڙي;
  }
#إذا TREETRACE_ENABLED
  إذا( p->pWin && (sqlite3_وڻ_Trace & 0x40)!=0 ){
    TREETRACE(0x40,pParse,p, ("after window rewrite:\n"));
    sqlite3_وڻ_View_Select(0, p, 0);
  }
#جيڪڏهن_پورو
#جيڪڏهن_پورو /* SQLITE_OMIT_WINDOWFUNC */
  pTabList = p->pSrc;
  isAgg = (p->selFlags & SF_Aggregate)!=0;
  memset(&s_ترتيب_ڏيو, 0, حجم(s_ترتيب_ڏيو));
  s_ترتيب_ڏيو.p_ترتيب_By = p->p_ترتيب_By;

  /* Try to do various optimizations (flattening subqueries, and strength
  ** reduction of join operators) in the FROM clause up into the main query
  ** tag-select-0200
  */
#إذا !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
  لكل(i=0; !p->p_اڳواٽ && i<pTabList->nSrc; i++){
    Src_شي *p_شي = &pTabList->a[i];
    Select *pSub = p_شي->fg.isSubquery ? p_شي->u4.pSubq->pSelect : 0;
    جدول *pTab = p_شي->pSTab;

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
    إذا( (p_شي->fg.jointype & (JT_کاٻو|JT_LTORJ))!=0
     && sqlite3_اظهار_نتيجو_ڏئي_Non_Null_صف(p->pWhere, p_شي->iCursor,
                                     p_شي->fg.jointype & JT_LTORJ)
     && OptimizationEnabled(db, SQLITE_SimplifyJoin)
    ){
      إذا( p_شي->fg.jointype & JT_کاٻو ){
        إذا( p_شي->fg.jointype & JT_ساڄو ){
          TREETRACE(0x1000,pParse,p,
                    ("FULL-JOIN simplifies to RIGHT-JOIN on term %d\n",i));
          p_شي->fg.jointype &= ~JT_کاٻو;
        }وإلا{
          TREETRACE(0x1000,pParse,p,
                    ("LEFT-JOIN simplifies to JOIN on term %d\n",i));
          p_شي->fg.jointype &= ~(JT_کاٻو|JT_OUTER);
          unset_Join_اظهار(p->pWhere, p_شي->iCursor, 0);
        }
      }
      إذا( p_شي->fg.jointype & JT_LTORJ ){
        لكل(j=i+1; j<pTabList->nSrc; j++){
          Src_شي *pI2 = &pTabList->a[j];
          إذا( pI2->fg.jointype & JT_ساڄو ){
            إذا( pI2->fg.jointype & JT_کاٻو ){
              TREETRACE(0x1000,pParse,p,
                        ("FULL-JOIN simplifies to LEFT-JOIN on term %d\n",j));
              pI2->fg.jointype &= ~JT_ساڄو;
            }وإلا{
              TREETRACE(0x1000,pParse,p,
                        ("RIGHT-JOIN simplifies to JOIN on term %d\n",j));
              pI2->fg.jointype &= ~(JT_ساڄو|JT_OUTER);
              unset_Join_اظهار(p->pWhere, pI2->iCursor, 1);
            }
          }
        }
        لكل(j=pTabList->nSrc-1; j>=0; j--){
          pTabList->a[j].fg.jointype &= ~JT_LTORJ;
          إذا( pTabList->a[j].fg.jointype & JT_ساڄو ) اكسر;
        }
      }
    }

    /* No further action if this term of the FROM clause is not a subquery */
    إذا( pSub==0 ) استمر;

    /* Catch mismatch in the declared columns of a view and the number of
    ** columns in the SELECT on the RHS */
    إذا( pTab->n_کانو!=pSub->pEList->n_اظهار ){
      sqlite3_Error_نياپو(pParse, "expected %d columns for '%s' but got %d",
                      pTab->n_کانو, pTab->zName, pSub->pEList->n_اظهار);
      اذهب select_پڇاڙي;
    }

    /* Do not attempt the usual optimizations (flattening and ORDER BY
    ** elimination) on a MATERIALIZED common table expression because
    ** a MATERIALIZED common table expression is an optimization fence.
    */
    إذا( p_شي->fg.isCte && p_شي->u2.pCteUse->eM10d==M10d_Yes ){
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
    أكد( pSub->p_گروهه_By==0 );

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
    إذا( pSub->p_ترتيب_By!=0
     && (p->p_ترتيب_By!=0 || pTabList->nSrc>1)      /* Condition (5) */
     && pSub->p_حد_بندي==0                           /* Condition (1) */
     && (pSub->selFlags & (SF_ترتيب_By_Reqd|SF_Recursive))==0  /* (2) and (6) */
     && (p->selFlags & SF_ترتيب_By_Reqd)==0         /* Condition (3) and (4) */
     && OptimizationEnabled(db, SQLITE_Omit_ترتيب_By)
    ){
      TREETRACE(0x800,pParse,p,
                ("omit superfluous ORDER BY on %r FROM-clause subquery\n",i+1));
      sqlite3_Parser_جوڙيو_Cleanup(pParse, sqlite3_اظهار_List_Delete_Generic,
                              pSub->p_ترتيب_By);
      pSub->p_ترتيب_By = 0;
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
    إذا( pSub->p_ترتيب_By!=0
     && i==0
     && (p->selFlags & SF_Complex_نتيجو)!=0
     && (pTabList->nSrc==1
         || (pTabList->a[1].fg.jointype&(JT_OUTER|JT_CROSS))!=0)
    ){
      استمر;
    }

    /* tag-select-0240 */
    إذا( flattenSubquery(pParse, p, i, isAgg) ){
      إذا( pParse->nErr ) اذهب select_پڇاڙي;
      /* This subquery can be absorbed into its parent. */
      i = -1;
    }
    pTabList = p->pSrc;
    إذا( db->mallocFailed ) اذهب select_پڇاڙي;
    إذا( !IgnorableOrderby(pDest) ){
      s_ترتيب_ڏيو.p_ترتيب_By = p->p_ترتيب_By;
    }
  }
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_COMPOUND_SELECT
  /* Handle compound SELECT statements using the separate multiSelect()
  ** procedure.  tag-select-0300
  */
  إذا( p->p_اڳواٽ ){
    rc = multiSelect(pParse, p, pDest);
#إذا TREETRACE_ENABLED
    TREETRACE(0x400,pParse,p,("end compound-select processing\n"));
    إذا( (sqlite3_وڻ_Trace & 0x400)!=0 && Explain_Query_Plan_والد(pParse)==0 ){
      sqlite3_وڻ_View_Select(0, p, 0);
    }
#جيڪڏهن_پورو
    إذا( p->p_ايندڙ==0 ) Explain_Query_Plan_ڪڍو(pParse);
    أرجع rc;
  }
#جيڪڏهن_پورو

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
    إذا( sqlite3_وڻ_Trace & 0x2000 ){
      TREETRACE(0x2000,pParse,p,("After constant propagation:\n"));
      sqlite3_وڻ_View_Select(0, p, 0);
    }
#جيڪڏهن_پورو
  }وإلا{
    TREETRACE(0x2000,pParse,p,("Constant propagation not helpful\n"));
  }

  /* tag-select-0350 */
  إذا( OptimizationEnabled(db, SQLITE_QueryFlattener|SQLITE_ڳڻپ_جو_View)
   && ڳڻپ_جو_View_Optimization(pParse, p)
  ){
    إذا( db->mallocFailed ) اذهب select_پڇاڙي;
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
    Src_شي *p_شي = &pTabList->a[i];
    Src_شي *p_اڳواٽ;
    SelectDest dest;
    Subquery *pSubq;
    Select *pSub;
#إذا !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    ثابت حرف *zSavedAuthContext;
#جيڪڏهن_پورو

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
    إذا( p_شي->کانو_استعمال_ٿيل==0 && p_شي->zName!=0 ){
      ثابت حرف *zDb;
      إذا( p_شي->fg.fixedSchema ){
        صحيح iDb = sqlite3_Schema_منزل_اشاريو(pParse->db, p_شي->u4.pSchema);
        zDb = db->aDb[iDb].zDbSName;
      }وإلا إذا( p_شي->fg.isSubquery ){
        zDb = 0;
      }وإلا{
        zDb = p_شي->u4.zDatabase;
      }
      sqlite3AuthCheck(pParse, SQLITE_READ, p_شي->zName, "", zDb);
    }

#إذا !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    /* Generate code for all sub-queries in the FROM clause
    */
    إذا( p_شي->fg.isSubquery==0 ) استمر;
    pSubq = p_شي->u4.pSubq;
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
    pParse->n_اوچائي += sqlite3_Select_اظهار_اوچائي(p);

    /* Make copies of constant WHERE-clause terms in the outer query down
    ** inside the subquery.  This can help the subquery to run more efficiently.
    ** This is the "predicate push-down optimization".  tag-select-0420
    */
    إذا( OptimizationEnabled(db, SQLITE_ڌڪو_هيٺ_ڏانهن)
     && (p_شي->fg.isCte==0
         || (p_شي->u2.pCteUse->eM10d!=M10d_Yes && p_شي->u2.pCteUse->nUse<2))
     && ڌڪو_هيٺ_ڏانهن_Where_Terms(pParse, pSub, p->pWhere, pTabList, i)
    ){
#إذا TREETRACE_ENABLED
      إذا( sqlite3_وڻ_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("After WHERE-clause push-down into subquery %d:\n", pSub->selId));
        sqlite3_وڻ_View_Select(0, p, 0);
      }
#جيڪڏهن_پورو
      أكد( pSubq->pSelect && (pSub->selFlags & SF_ڌڪو_هيٺ_ڏانهن)!=0 );
    }وإلا{
      TREETRACE(0x4000,pParse,p,("WHERE-clause push-down not possible\n"));
    }

    /* Convert unused result columns of the subquery into simple NULL
    ** expressions, to avoid unneeded searching and computation.
    ** tag-select-0440
    */
    إذا( OptimizationEnabled(db, SQLITE_Null_اڻ_استعمال_ٿيل_کانن_ڳڻپ)
     && disable_اڻ_استعمال_ٿيل_Subquery_نتيجو_Columns(p_شي)
    ){
#إذا TREETRACE_ENABLED
      إذا( sqlite3_وڻ_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("Change unused result columns to NULL for subquery %d:\n",
             pSub->selId));
        sqlite3_وڻ_View_Select(0, p, 0);
      }
#جيڪڏهن_پورو
    }

    zSavedAuthContext = pParse->zAuthContext;
    pParse->zAuthContext = p_شي->zName;

    /* Generate byte-code to implement the subquery  tag-select-0480
    */
    إذا( from_Clause_رڪن_Can_Be_Coroutine(pParse, pTabList, i, p->selFlags) ){
      /* Implement a co-routine that will return a single row of the result
      ** set on each invocation.  tag-select-0482
      */
      صحيح addr_چوٽي = sqlite3VdbeCurrentAddr(v)+1;
    
      pSubq->regReturn = ++pParse->nMem;
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_InitCoroutine, pSubq->regReturn, 0, addr_چوٽي);
      VdbeComment((v, "%!S", p_شي));
      pSubq->addrFillSub = addr_چوٽي;
      sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);
      ExplainQueryPlan((pParse, 1, "CO-ROUTINE %!S", p_شي));
      sqlite3Select(pParse, pSub, &dest);
      p_شي->pSTab->n_صف_Log_Est = pSub->n_Select_صف;
      p_شي->fg.ذريعي_Coroutine = 1;
      pSubq->reg_نتيجو = dest.iSdst;
      sqlite3_Vdbe_پڇاڙي_Coroutine(v, pSubq->regReturn);
      VdbeComment((v, "end %!S", p_شي));
      sqlite3_Vdbe_Jump_هتي(v, addr_چوٽي-1);
      sqlite3_Clear_عارضي_Reg_Cache(pParse);
    }وإلا إذا( p_شي->fg.isCte && p_شي->u2.pCteUse->addrM9e>0 ){
      /* This is a CTE for which materialization code has already been
      ** generated.  Invoke the subroutine to compute the materialization,
      ** then make the pItem->iCursor be a copy of the ephemeral table that
      ** holds the result of the materialization. tag-select-0484 */
      CteUse *pCteUse = p_شي->u2.pCteUse;
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Gosub, pCteUse->regRtn, pCteUse->addrM9e);
      إذا( p_شي->iCursor!=pCteUse->iCur ){
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_OpenDup, p_شي->iCursor, pCteUse->iCur);
        VdbeComment((v, "%!S", p_شي));
      }
      pSub->n_Select_صف = pCteUse->n_صف_Est;
    }وإلا إذا( (p_اڳواٽ = isSelfJoinView(pTabList, p_شي, 0, i))!=0 ){
      /* This view has already been materialized by a prior entry in
      ** this same FROM clause.  Reuse it.  tag-select-0486 */
      Subquery *p_اڳواٽ_Subq;
      أكد( p_اڳواٽ->fg.isSubquery );
      p_اڳواٽ_Subq = p_اڳواٽ->u4.pSubq;
      أكد( p_اڳواٽ_Subq!=0 );
      إذا( p_اڳواٽ_Subq->addrFillSub ){
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_Gosub, p_اڳواٽ_Subq->regReturn,
                                       p_اڳواٽ_Subq->addrFillSub);
      }
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_OpenDup, p_شي->iCursor, p_اڳواٽ->iCursor);
      pSub->n_Select_صف = p_اڳواٽ_Subq->pSelect->n_Select_صف;
    }وإلا{
      /* Materialize the view.  If the view is not correlated, generate a
      ** subroutine to do the materialization so that subsequent uses of
      ** the same view can reuse the materialization.  tag-select-0488 */
      صحيح چوٽي_Addr;
      صحيح onceAddr = 0;
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STMT_SCANSTATUS
      صحيح addrExplain;
#جيڪڏهن_پورو

      pSubq->regReturn = ++pParse->nMem;
      چوٽي_Addr = sqlite3_Vdbe_جوڙيو_Op0(v, OP_Goto);
      pSubq->addrFillSub = چوٽي_Addr+1;
      p_شي->fg.isMaterialized = 1;
      إذا( p_شي->fg.isCorrelated==0 ){
        /* If the subquery is not correlated and if we are not inside of
        ** a trigger, then we only need to compute the value of the subquery
        ** once. */
        onceAddr = sqlite3_Vdbe_جوڙيو_Op0(v, OP_Once); VdbeCoverage(v);
        VdbeComment((v, "materialize %!S", p_شي));
      }وإلا{
        VdbeNoopComment((v, "materialize %!S", p_شي));
      }
      sqlite3SelectDestInit(&dest, SRT_EphemTab, p_شي->iCursor);

      ExplainQueryPlan2(addrExplain, (pParse, 1, "MATERIALIZE %!S", p_شي));
      sqlite3Select(pParse, pSub, &dest);
      p_شي->pSTab->n_صف_Log_Est = pSub->n_Select_صف;
      إذا( onceAddr ) sqlite3_Vdbe_Jump_هتي(v, onceAddr);
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Return, pSubq->regReturn, چوٽي_Addr+1);
      VdbeComment((v, "end %!S", p_شي));
      sqlite3_Vdbe_جاچ_حالت_Range(v, addrExplain, addrExplain, -1);
      sqlite3_Vdbe_Jump_هتي(v, چوٽي_Addr);
      sqlite3_Clear_عارضي_Reg_Cache(pParse);
      إذا( p_شي->fg.isCte && p_شي->fg.isCorrelated==0 ){
        CteUse *pCteUse = p_شي->u2.pCteUse;
        pCteUse->addrM9e = pSubq->addrFillSub;
        pCteUse->regRtn = pSubq->regReturn;
        pCteUse->iCur = p_شي->iCursor;
        pCteUse->n_صف_Est = pSub->n_Select_صف;
      }
    }
    إذا( db->mallocFailed ) اذهب select_پڇاڙي;
    pParse->n_اوچائي -= sqlite3_Select_اظهار_اوچائي(p);
    pParse->zAuthContext = zSavedAuthContext;
#جيڪڏهن_پورو
  }

  /* Various elements of the SELECT copied into local variables for
  ** convenience */
  pEList = p->pEList;
  pWhere = p->pWhere;
  p_گروهه_By = p->p_گروهه_By;
  pHaving = p->pHaving;
  sDistinct.isTnct = (p->selFlags & SF_Distinct)!=0;

#إذا TREETRACE_ENABLED
  إذا( sqlite3_وڻ_Trace & 0x8000 ){
    TREETRACE(0x8000,pParse,p,("After all FROM-clause analysis:\n"));
    sqlite3_وڻ_View_Select(0, p, 0);
  }
#جيڪڏهن_پورو

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
   && sqlite3_Copy_ترتيب_ڏيو_ترتيب(pEList, s_ترتيب_ڏيو.p_ترتيب_By)
   && sqlite3_اظهار_List_Compare(pEList, s_ترتيب_ڏيو.p_ترتيب_By, -1)==0
   && OptimizationEnabled(db, SQLITE_گروهه_By_ترتيب)
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
   && p->pWin==0
#جيڪڏهن_پورو
  ){
    p->selFlags &= ~(u32)SF_Distinct;
    p_گروهه_By = p->p_گروهه_By = sqlite3_اظهار_List_Dup(db, pEList, 0);
    إذا( p_گروهه_By ){
      لكل(i=0; i<p_گروهه_By->n_اظهار; i++){
        p_گروهه_By->a[i].u.x.i_ترتيب_By_کانو = i+1;
      }
    }
    p->selFlags |= SF_Aggregate;
    /* Notice that even thought SF_Distinct has been cleared from p->selFlags,
    ** the sDistinct.isTnct is still set.  Hence, isTnct represents the
    ** original setting of the SF_Distinct flag, not the current setting */
    أكد( sDistinct.isTnct );
    sDistinct.isTnct = 2;

#إذا TREETRACE_ENABLED
    إذا( sqlite3_وڻ_Trace & 0x20000 ){
      TREETRACE(0x20000,pParse,p,("Transform DISTINCT into GROUP BY:\n"));
      sqlite3_وڻ_View_Select(0, p, 0);
    }
#جيڪڏهن_پورو
  }

  /* If there is an ORDER BY clause, then create an ephemeral index to
  ** do the sorting.  But this sorting ephemeral index might end up
  ** being unused if the data can be extracted in pre-sorted order.
  ** If that is the case, then the OP_OpenEphemeral instruction will be
  ** changed to an OP_Noop once we figure out that the sorting index is
  ** not needed.  The sSort.addrSortIndex variable is used to facilitate
  ** that change.  tag-select-0600
  */
  إذا( s_ترتيب_ڏيو.p_ترتيب_By ){
    ڪنجي_Info *p_ڪنجي_Info;
    p_ڪنجي_Info = sqlite3_ڪنجي_Info_From_اظهار_List(
        pParse, s_ترتيب_ڏيو.p_ترتيب_By, 0, pEList->n_اظهار);
    s_ترتيب_ڏيو.iECursor = pParse->nTab++;
    s_ترتيب_ڏيو.addr_ترتيب_ڏيو_اشاريو =
      sqlite3_Vdbe_جوڙيو_Op4(v, OP_OpenEphemeral,
          s_ترتيب_ڏيو.iECursor, s_ترتيب_ڏيو.p_ترتيب_By->n_اظهار+1+pEList->n_اظهار, 0,
          (حرف*)p_ڪنجي_Info, P4_KEYINFO
      );
  }وإلا{
    s_ترتيب_ڏيو.addr_ترتيب_ڏيو_اشاريو = -1;
  }

  /* If the output is destined for a temporary table, open that table.
  ** tag-select-0630
  */
  إذا( pDest->eDest==SRT_EphemTab ){
    sqlite3_Vdbe_جوڙيو_Op2(v, OP_OpenEphemeral, pDest->iSDParm, pEList->n_اظهار);
    إذا( p->selFlags & SF_NestedFrom ){
      /* Delete or NULL-out result columns that will never be used */
      صحيح ii;
      لكل(ii=pEList->n_اظهار-1; ii>0 && pEList->a[ii].fg.b_استعمال_ٿيل==0; ii--){
        sqlite3_اظهار_Delete(db, pEList->a[ii].p_اظهار);
        sqlite3DbFree(db, pEList->a[ii].zEName);
        pEList->n_اظهار--;
      }
      لكل(ii=0; ii<pEList->n_اظهار; ii++){
        إذا( pEList->a[ii].fg.b_استعمال_ٿيل==0 ) pEList->a[ii].p_اظهار->op = TK_NULL;
      }
    }
  }

  /* Set the limiter.  tag-select-0650
  */
  i_پڇاڙي = sqlite3VdbeMakeLabel(pParse);
  إذا( (p->selFlags & SF_Fixed_حد_بندي)==0 ){
    p->n_Select_صف = 320;  /* 4 billion rows */
  }
  إذا( p->p_حد_بندي ) compute_حد_بندي_Registers(pParse, p, i_پڇاڙي);
  إذا( p->i_حد_بندي==0 && s_ترتيب_ڏيو.addr_ترتيب_ڏيو_اشاريو>=0 ){
    sqlite3VdbeChangeOpcode(v, s_ترتيب_ڏيو.addr_ترتيب_ڏيو_اشاريو, OP_SorterOpen);
    s_ترتيب_ڏيو.ترتيب_ڏيو_Flags |= SORTFLAG_UseSorter;
  }

  /* Open an ephemeral index to use for the distinct set. tag-select-0680
  */
  إذا( p->selFlags & SF_Distinct ){
    sDistinct.tabTnct = pParse->nTab++;
    sDistinct.addrTnct = sqlite3_Vdbe_جوڙيو_Op4(v, OP_OpenEphemeral,
                       sDistinct.tabTnct, 0, 0,
                       (حرف*)sqlite3_ڪنجي_Info_From_اظهار_List(pParse, p->pEList,0,0),
                       P4_KEYINFO);
    sqlite3VdbeChangeP5(v, BTREE_UNORDERED);
    sDistinct.eTnctType = WHERE_DISTINCT_UNORDERED;
  }وإلا{
    sDistinct.eTnctType = WHERE_DISTINCT_NOOP;
  }

  إذا( !isAgg && p_گروهه_By==0 ){
    /* No aggregate functions and no GROUP BY clause.  tag-select-0700 */
    u16 wctrlFlags = (sDistinct.isTnct ? WHERE_WANT_DISTINCT : 0)
                   | (p->selFlags & SF_Fixed_حد_بندي);
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
    دري *pWin = p->pWin;      /* Main window object (or NULL) */
    إذا( pWin ){
      sqlite3_دري_ڪوڊ_Init(pParse, p);
    }
#جيڪڏهن_پورو
    أكد( WHERE_USE_حد_بندي==SF_Fixed_حد_بندي );


    /* Begin the database scan. */
    TREETRACE(0x2,pParse,p,("WhereBegin\n"));
    pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, s_ترتيب_ڏيو.p_ترتيب_By,
                               p->pEList, p, wctrlFlags, p->n_Select_صف);
    إذا( pWInfo==0 ) اذهب select_پڇاڙي;
    إذا( sqlite3_Where_Output_صف_ڳڻپ(pWInfo) < p->n_Select_صف ){
      p->n_Select_صف = sqlite3_Where_Output_صف_ڳڻپ(pWInfo);
      إذا( pDest->eDest<=SRT_Dist_قطار && pDest->eDest>=SRT_DistFifo ){
        /* TUNING: For a UNION CTE, because UNION is implies DISTINCT,
        ** reduce the estimated output row count by 8 (LogEst 30). 
        ** Search for tag-20250414a to see other cases */
        p->n_Select_صف -= 30;
      }
    }
    إذا( sDistinct.isTnct && sqlite3WhereIsDistinct(pWInfo) ){
      sDistinct.eTnctType = sqlite3WhereIsDistinct(pWInfo);
    }
    إذا( s_ترتيب_ڏيو.p_ترتيب_By ){
      s_ترتيب_ڏيو.nOBSat = sqlite3WhereIsOrdered(pWInfo);
      s_ترتيب_ڏيو.labelOBLopt = sqlite3_Where_ترتيب_By_حد_بندي_Opt_Label(pWInfo);
      إذا( s_ترتيب_ڏيو.nOBSat==s_ترتيب_ڏيو.p_ترتيب_By->n_اظهار ){
        s_ترتيب_ڏيو.p_ترتيب_By = 0;
      }
    }
    TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));

    /* If sorting index that was created by a prior OP_OpenEphemeral
    ** instruction ended up not being needed, then change the OP_OpenEphemeral
    ** into an OP_Noop.
    */
    إذا( s_ترتيب_ڏيو.addr_ترتيب_ڏيو_اشاريو>=0 && s_ترتيب_ڏيو.p_ترتيب_By==0 ){
      sqlite3_Vdbe_Change_منزل_Noop(v, s_ترتيب_ڏيو.addr_ترتيب_ڏيو_اشاريو);
    }

    أكد( p->pEList==pEList );
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WINDOWFUNC
    إذا( pWin ){
      صحيح addrGosub = sqlite3VdbeMakeLabel(pParse);
      صحيح iCont = sqlite3VdbeMakeLabel(pParse);
      صحيح iBreak = sqlite3VdbeMakeLabel(pParse);
      صحيح regGosub = ++pParse->nMem;

      sqlite3_دري_ڪوڊ_قدم(pParse, p, pWInfo, regGosub, addrGosub);

      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Goto, 0, iBreak);
      sqlite3VdbeResolveLabel(v, addrGosub);
      VdbeNoopComment((v, "inner-loop subroutine"));
      s_ترتيب_ڏيو.labelOBLopt = 0;
      selectInnerLoop(pParse, p, -1, &s_ترتيب_ڏيو, &sDistinct, pDest, iCont, iBreak);
      sqlite3VdbeResolveLabel(v, iCont);
      sqlite3_Vdbe_جوڙيو_Op1(v, OP_Return, regGosub);
      VdbeComment((v, "end inner-loop subroutine"));
      sqlite3VdbeResolveLabel(v, iBreak);
    }وإلا
#جيڪڏهن_پورو /* SQLITE_OMIT_WINDOWFUNC */
    {
      /* Use the standard inner loop. */
      selectInnerLoop(pParse, p, -1, &s_ترتيب_ڏيو, &sDistinct, pDest,
          sqlite3WhereContinueLabel(pWInfo),
          sqlite3WhereBreakLabel(pWInfo));

      /* End the database scan loop.
      */
      TREETRACE(0x2,pParse,p,("WhereEnd\n"));
      sqlite3_Where_پڇاڙي(pWInfo);
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
    صحيح گروهه_By_ترتيب_ڏيو;    /* Rows come from source in GROUP BY order */
    صحيح addr_پڇاڙي;        /* End of processing for this SELECT */
    صحيح ترتيب_ڏيو_PTab = 0;   /* Pseudotable used to decode sorting results */
    صحيح ترتيب_ڏيو_ٻاهريون = 0;    /* Output register from the sorter */
    صحيح ترتيب_By_Grp = 0; /* True if the GROUP BY and ORDER BY are the same */

    /* Remove any and all aliases between the result set and the
    ** GROUP BY clause.
    */
    إذا( p_گروهه_By ){
      صحيح k;                        /* Loop counter */
      بنية اظهار_List_شي *p_شي;  /* For looping over expression in a list */

      لكل(k=p->pEList->n_اظهار, p_شي=p->pEList->a; k>0; k--, p_شي++){
        p_شي->u.x.iAlias = 0;
      }
      لكل(k=p_گروهه_By->n_اظهار, p_شي=p_گروهه_By->a; k>0; k--, p_شي++){
        p_شي->u.x.iAlias = 0;
      }
      أكد( 66==sqlite3LogEst(100) );
      إذا( p->n_Select_صف>66 ) p->n_Select_صف = 66;

      /* If there is both a GROUP BY and an ORDER BY clause and they are
      ** identical, then it may be possible to disable the ORDER BY clause
      ** on the grounds that the GROUP BY will cause elements to come out
      ** in the correct order. It also may not - the GROUP BY might use a
      ** database index that causes rows to be grouped together as required
      ** but not actually sorted. Either way, record the fact that the
      ** ORDER BY and GROUP BY clauses are the same by setting the orderByGrp
      ** variable.  */
      إذا( sqlite3_Copy_ترتيب_ڏيو_ترتيب(p_گروهه_By, s_ترتيب_ڏيو.p_ترتيب_By)
       && sqlite3_اظهار_List_Compare(p_گروهه_By, s_ترتيب_ڏيو.p_ترتيب_By, -1)==0
      ){
        ترتيب_By_Grp = 1;
      }
    }وإلا{
      أكد( 0==sqlite3LogEst(1) );
      p->n_Select_صف = 0;
    }

    /* Create a label to jump to when we want to abort the query */
    addr_پڇاڙي = sqlite3VdbeMakeLabel(pParse);

    /* Convert TK_COLUMN nodes into TK_AGG_COLUMN and make entries in
    ** sAggInfo for all TK_AGG_FUNCTION nodes in expressions of the
    ** SELECT statement.
    */
    pAggInfo = sqlite3DbMallocZero(db, حجم(*pAggInfo) );
    إذا( pAggInfo ){
      sqlite3_Parser_جوڙيو_Cleanup(pParse, agginfoFree, pAggInfo);
      testcase( pParse->earlyCleanup );
    }
    إذا( db->mallocFailed ){
      اذهب select_پڇاڙي;
    }
    pAggInfo->selId = p->selId;
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
    pAggInfo->pSelect = p;
#جيڪڏهن_پورو
    memset(&sNC, 0, حجم(sNC));
    sNC.pParse = pParse;
    sNC.pSrcList = pTabList;
    sNC.uNC.pAggInfo = pAggInfo;
    VVA_ONLY( sNC.ncFlags = NC_UAggInfo; )
    pAggInfo->n_ترتيب_ڏيڻ_ڪالم = p_گروهه_By ? p_گروهه_By->n_اظهار : 0;
    pAggInfo->p_گروهه_By = p_گروهه_By;
    sqlite3_اظهار_Analyze_Agg_List(&sNC, pEList);
    sqlite3_اظهار_Analyze_Agg_List(&sNC, s_ترتيب_ڏيو.p_ترتيب_By);
    إذا( pHaving ){
      إذا( p_گروهه_By ){
        أكد( pWhere==p->pWhere );
        أكد( pHaving==p->pHaving );
        أكد( p_گروهه_By==p->p_گروهه_By );
        having_منزل_Where(pParse, p);
        pWhere = p->pWhere;
      }
      sqlite3_اظهار_Analyze_Aggregates(&sNC, pHaving);
    }
    pAggInfo->n_گڏ_ڪندڙ = pAggInfo->n_ڪالم;
    إذا( p->p_گروهه_By==0 && p->pHaving==0 && pAggInfo->nFunc==1 ){
      minMaxFlag = minMaxQuery(db, pAggInfo->aFunc[0].pFExpr, &p_Min_Max_ترتيب_By);
    }وإلا{
      minMaxFlag = WHERE_ORDERBY_عام;
    }
    analyze_Agg_Func_دليلون(pAggInfo, &sNC);
    إذا( db->mallocFailed ) اذهب select_پڇاڙي;
#إذا TREETRACE_ENABLED
    إذا( sqlite3_وڻ_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("After aggregate analysis %p:\n", pAggInfo));
      sqlite3_وڻ_View_Select(0, p, 0);
      إذا( minMaxFlag ){
        sqlite3DebugPrintf("MIN/MAX Optimization (0x%02x) adds:\n", minMaxFlag);
        sqlite3_وڻ_View_اظهار_List(0, p_Min_Max_ترتيب_By, 0, "ORDERBY");
      }
      printAggInfo(pAggInfo);
    }
#جيڪڏهن_پورو


    /* Processing for aggregates with GROUP BY is very different and
    ** much more complex than aggregates without a GROUP BY.  tag-select-0810
    */
    إذا( p_گروهه_By ){
      ڪنجي_Info *p_ڪنجي_Info;  /* Keying information for the group by clause */
      صحيح addr1;          /* A-vs-B comparison jump */
      صحيح addr_Output_صف;  /* Start of subroutine that outputs a result row */
      صحيح reg_Output_صف;   /* Return address register for output subroutine */
      صحيح addr_سيٽ_Abort;   /* Set the abort flag and return */
      صحيح addr_چوٽي_جو_Loop;  /* Top of the input loop */
      صحيح addr_ترتيب_ڏيڻ_Idx; /* The OP_OpenEphemeral for the sorting index */
      صحيح addrReset;      /* Subroutine for resetting the accumulator */
      صحيح regReset;       /* Return address register for reset subroutine */
      اظهار_List *pDistinct = 0;
      u16 distFlag = 0;
      صحيح eDist = WHERE_DISTINCT_NOOP;

      إذا( pAggInfo->nFunc==1
       && pAggInfo->aFunc[0].iDistinct>=0
       && ALWAYS(pAggInfo->aFunc[0].pFExpr!=0)
       && ALWAYS(اظهار_Use_XList(pAggInfo->aFunc[0].pFExpr))
       && pAggInfo->aFunc[0].pFExpr->x.pList!=0
      ){
        اظهار *p_اظهار = pAggInfo->aFunc[0].pFExpr->x.pList->a[0].p_اظهار;
        p_اظهار = sqlite3_اظهار_Dup(db, p_اظهار, 0);
        pDistinct = sqlite3_اظهار_List_Dup(db, p_گروهه_By, 0);
        pDistinct = sqlite3_اظهار_List_Append(pParse, pDistinct, p_اظهار);
        distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
      }

      /* If there is a GROUP BY clause we might need a sorting index to
      ** implement it.  Allocate that sorting index now.  If it turns out
      ** that we do not need it after all, the OP_SorterOpen instruction
      ** will be converted into a Noop. 
      */
      pAggInfo->ترتيب_ڏيڻ_Idx = pParse->nTab++;
      p_ڪنجي_Info = sqlite3_ڪنجي_Info_From_اظهار_List(pParse, p_گروهه_By,
                                            0, pAggInfo->n_ڪالم);
      addr_ترتيب_ڏيڻ_Idx = sqlite3_Vdbe_جوڙيو_Op4(v, OP_SorterOpen,
          pAggInfo->ترتيب_ڏيڻ_Idx, pAggInfo->n_ترتيب_ڏيڻ_ڪالم,
          0, (حرف*)p_ڪنجي_Info, P4_KEYINFO);

      /* Initialize memory locations used by GROUP BY aggregate processing
      */
      iUseFlag = ++pParse->nMem;
      iAbortFlag = ++pParse->nMem;
      reg_Output_صف = ++pParse->nMem;
      addr_Output_صف = sqlite3VdbeMakeLabel(pParse);
      regReset = ++pParse->nMem;
      addrReset = sqlite3VdbeMakeLabel(pParse);
      iAMem = pParse->nMem + 1;
      pParse->nMem += p_گروهه_By->n_اظهار;
      iBMem = pParse->nMem + 1;
      pParse->nMem += p_گروهه_By->n_اظهار;
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, 0, iAbortFlag);
      VdbeComment((v, "clear abort flag"));
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Null, 0, iAMem, iAMem+p_گروهه_By->n_اظهار-1);
      sqlite3_اظهار_Null_رجسٽر_Range(pParse, iAMem, p_گروهه_By->n_اظهار);

      /* Begin a loop that will extract all source rows in GROUP BY order.
      ** This might involve two separate loops with an OP_Sort in between, or
      ** it might be a single loop that uses an index to extract information
      ** in the right order to begin with.
      */
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Gosub, regReset, addrReset);
      TREETRACE(0x2,pParse,p,("WhereBegin\n"));
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, p_گروهه_By, pDistinct,
          p, (sDistinct.isTnct==2 ? WHERE_DISTINCTBY : WHERE_GROUPBY)
          |  (ترتيب_By_Grp ? WHERE_SORTBYGROUP : 0) | distFlag, 0
      );
      إذا( pWInfo==0 ){
        sqlite3_اظهار_List_Delete(db, pDistinct);
        اذهب select_پڇاڙي;
      }
      إذا( pParse->pIdxEpr ){
        optimize_Aggregate_Use_جو_اشاريي_وارو_اظهار(pParse, p, pAggInfo, &sNC);
      }
      سونپيو_Aggregate_Registers(pParse, pAggInfo);
      eDist = sqlite3WhereIsDistinct(pWInfo);
      TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
      إذا( sqlite3WhereIsOrdered(pWInfo)==p_گروهه_By->n_اظهار ){
        /* The optimizer is able to deliver rows in group by order so
        ** we do not have to sort.  The OP_OpenEphemeral table will be
        ** cancelled later because we still need to use the pKeyInfo
        */
        گروهه_By_ترتيب_ڏيو = 0;
      }وإلا{
        /* Rows are coming out in undetermined order.  We have to push
        ** each row into a sorting index, terminate the first loop,
        ** then loop over the sorting index in order to get the output
        ** in sorted order
        */
        صحيح reg_بنياد;
        صحيح reg_رڪارڊ;
        صحيح n_کانو;
        صحيح n_گروهه_By;

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_STMT_SCANSTATUS
        صحيح addrExp;              /* Address of OP_Explain instruction */
#جيڪڏهن_پورو
        ExplainQueryPlan2(addrExp, (pParse, 0, "USE TEMP B-TREE FOR %s",
            (sDistinct.isTnct && (p->selFlags&SF_Distinct)==0) ?
                    "DISTINCT" : "GROUP BY"
        ));

        گروهه_By_ترتيب_ڏيو = 1;
        n_گروهه_By = p_گروهه_By->n_اظهار;
        n_کانو = n_گروهه_By;
        j = n_گروهه_By;
        لكل(i=0; i<pAggInfo->n_ڪالم; i++){
          إذا( pAggInfo->a_کانو[i].i_Sorter_ڪالم>=j ){
            n_کانو++;
            j++;
          }
        }
        reg_بنياد = sqlite3_Get_عارضي_Range(pParse, n_کانو);
        sqlite3_اظهار_ڪوڊ_اظهار_List(pParse, p_گروهه_By, reg_بنياد, 0, 0);
        j = n_گروهه_By;
        pAggInfo->سڌو_گهڻائي = 1;
        لكل(i=0; i<pAggInfo->n_ڪالم; i++){
          بنية Agg_Info_کانو *p_کانو = &pAggInfo->a_کانو[i];
          إذا( p_کانو->i_Sorter_ڪالم>=j ){
            sqlite3_اظهار_ڪوڊ(pParse, p_کانو->pCExpr, j + reg_بنياد);
            j++;
          }
        }
        pAggInfo->سڌو_گهڻائي = 0;
        reg_رڪارڊ = sqlite3_Get_عارضي_Reg(pParse);
        sqlite3_Vdbe_جاچ_حالت_Counters(v, addrExp, 0, sqlite3VdbeCurrentAddr(v));
        sqlite3_Vdbe_جوڙيو_Op3(v, OP_Make_رڪارڊ, reg_بنياد, n_کانو, reg_رڪارڊ);
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_Sorter_وجهو, pAggInfo->ترتيب_ڏيڻ_Idx, reg_رڪارڊ);
        sqlite3_Vdbe_جاچ_حالت_Range(v, addrExp, sqlite3VdbeCurrentAddr(v)-2, -1);
        sqlite3_Release_عارضي_Reg(pParse, reg_رڪارڊ);
        sqlite3_Release_عارضي_Range(pParse, reg_بنياد, n_کانو);
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_پڇاڙي(pWInfo);
        pAggInfo->ترتيب_ڏيڻ_Idx_PTab = ترتيب_ڏيو_PTab = pParse->nTab++;
        ترتيب_ڏيو_ٻاهريون = sqlite3_Get_عارضي_Reg(pParse);
        sqlite3_Vdbe_جاچ_حالت_Counters(v, addrExp, sqlite3VdbeCurrentAddr(v), 0);
        sqlite3_Vdbe_جوڙيو_Op3(v, OP_OpenPseudo, ترتيب_ڏيو_PTab, ترتيب_ڏيو_ٻاهريون, n_کانو);
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_Sorter_ترتيب_ڏيو, pAggInfo->ترتيب_ڏيڻ_Idx, addr_پڇاڙي);
        VdbeComment((v, "GROUP BY sort")); VdbeCoverage(v);
        pAggInfo->use_ترتيب_ڏيڻ_Idx = 1;
        sqlite3_Vdbe_جاچ_حالت_Range(v, addrExp, -1, ترتيب_ڏيو_PTab);
        sqlite3_Vdbe_جاچ_حالت_Range(v, addrExp, -1, pAggInfo->ترتيب_ڏيڻ_Idx);
      }

      /* If there are entries in pAgggInfo->aFunc[] that contain subexpressions
      ** that are indexed (and that were previously identified and tagged
      ** in optimizeAggregateUseOfIndexedExpr()) then those subexpressions
      ** must now be converted into a TK_AGG_COLUMN node so that the value
      ** is correctly pulled from the index rather than being recomputed. */
      إذا( pParse->pIdxEpr ){
        aggregate_Convert_اشاريي_وارو_اظهار_Ref_منزل_ڪالم(pAggInfo);
#إذا TREETRACE_ENABLED
        إذا( sqlite3_وڻ_Trace & 0x20 ){
          TREETRACE(0x20, pParse, p,
             ("AggInfo function expressions converted to reference index\n"));
          sqlite3_وڻ_View_Select(0, p, 0);
          printAggInfo(pAggInfo);
        }
#جيڪڏهن_پورو
      }

      /* If the index or temporary table used by the GROUP BY sort
      ** will naturally deliver rows in the order required by the ORDER BY
      ** clause, cancel the ephemeral table open coded earlier.
      **
      ** This is an optimization - the correct answer should result regardless.
      ** Use the SQLITE_GroupByOrder flag with SQLITE_TESTCTRL_OPTIMIZER to
      ** disable this optimization for testing purposes.  */
      إذا( ترتيب_By_Grp && OptimizationEnabled(db, SQLITE_گروهه_By_ترتيب)
       && (گروهه_By_ترتيب_ڏيو || sqlite3WhereIsSorted(pWInfo))
      ){
        s_ترتيب_ڏيو.p_ترتيب_By = 0;
        sqlite3_Vdbe_Change_منزل_Noop(v, s_ترتيب_ڏيو.addr_ترتيب_ڏيو_اشاريو);
      }

      /* Evaluate the current GROUP BY terms and store in b0, b1, b2...
      ** (b0 is memory location iBMem+0, b1 is iBMem+1, and so forth)
      ** Then compare the current GROUP BY terms against the GROUP BY terms
      ** from the previous row currently stored in a0, a1, a2...
      */
      addr_چوٽي_جو_Loop = sqlite3VdbeCurrentAddr(v);
      إذا( گروهه_By_ترتيب_ڏيو ){
        sqlite3_Vdbe_جوڙيو_Op3(v, OP_Sorter_معلومات, pAggInfo->ترتيب_ڏيڻ_Idx,
                          ترتيب_ڏيو_ٻاهريون, ترتيب_ڏيو_PTab);
      }
      لكل(j=0; j<p_گروهه_By->n_اظهار; j++){
        صحيح i_ترتيب_By_کانو = p_گروهه_By->a[j].u.x.i_ترتيب_By_کانو;
        
        إذا( گروهه_By_ترتيب_ڏيو ){
          sqlite3_Vdbe_جوڙيو_Op3(v, OP_ڪالم, ترتيب_ڏيو_PTab, j, iBMem+j);
        }وإلا{
          pAggInfo->سڌو_گهڻائي = 1;
          sqlite3_اظهار_ڪوڊ(pParse, p_گروهه_By->a[j].p_اظهار, iBMem+j);
        }

        إذا( i_ترتيب_By_کانو ){
          اظهار *pX = p->pEList->a[i_ترتيب_By_کانو-1].p_اظهار;
          اظهار *p_بنياد = sqlite3_اظهار_Skip_Collate_And_Likely(pX);
          طالما( ALWAYS(p_بنياد!=0) && p_بنياد->op==TK_IF_NULL_صف ){
            pX = p_بنياد->p_کاٻو;
            p_بنياد = sqlite3_اظهار_Skip_Collate_And_Likely(pX);
          }
          إذا( ALWAYS(p_بنياد!=0)
           && p_بنياد->op!=TK_AGG_ڪالم
           && p_بنياد->op!=TK_رجسٽر
          ){
            sqlite3_اظهار_منزل_رجسٽر(pX, iAMem+j);
          }
        }
      }
      sqlite3_Vdbe_جوڙيو_Op4(v, OP_Compare, iAMem, iBMem, p_گروهه_By->n_اظهار,
                          (حرف*)sqlite3_ڪنجي_Info_Ref(p_ڪنجي_Info), P4_KEYINFO);
      addr1 = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_جوڙيو_Op3(v, OP_Jump, addr1+1, 0, addr1+1); VdbeCoverage(v);

      /* Generate code that runs whenever the GROUP BY changes.
      ** Changes in the GROUP BY are detected by the previous code
      ** block.  If there were no changes, this block is skipped.
      **
      ** This code copies current group by terms in b0,b1,b2,...
      ** over to a0,a1,a2.  It then calls the output subroutine
      ** and resets the aggregate accumulator registers in preparation
      ** for the next GROUP BY batch.
      */
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Gosub, reg_Output_صف, addr_Output_صف);
      VdbeComment((v, "output one row of %d", p->selId));
      sqlite3_اظهار_ڪوڊ_Move(pParse, iBMem, iAMem, p_گروهه_By->n_اظهار);
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_IfPos, iAbortFlag, addr_پڇاڙي); VdbeCoverage(v);
      VdbeComment((v, "check abort flag"));
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Gosub, regReset, addrReset);
      VdbeComment((v, "reset accumulator %d", p->selId));

      /* Update the aggregate accumulators based on the content of
      ** the current row
      */
      sqlite3_Vdbe_Jump_هتي(v, addr1);
      update_گڏ_ڪندڙ(pParse, iUseFlag, pAggInfo, eDist);
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, 1, iUseFlag);
      VdbeComment((v, "indicate data in accumulator %d", p->selId));

      /* End of the loop
      */
      إذا( گروهه_By_ترتيب_ڏيو ){
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_Sorter_ايندڙ, pAggInfo->ترتيب_ڏيڻ_Idx,addr_چوٽي_جو_Loop);
        VdbeCoverage(v);
      }وإلا{
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_پڇاڙي(pWInfo);
        sqlite3_Vdbe_Change_منزل_Noop(v, addr_ترتيب_ڏيڻ_Idx);
      }
      sqlite3_اظهار_List_Delete(db, pDistinct);

      /* Output the final row of result
      */
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Gosub, reg_Output_صف, addr_Output_صف);
      VdbeComment((v, "output final row of %d", p->selId));

      /* Jump over the subroutines
      */
      sqlite3VdbeGoto(v, addr_پڇاڙي);

      /* Generate a subroutine that outputs a single row of the result
      ** set.  This subroutine first looks at the iUseFlag.  If iUseFlag
      ** is less than or equal to zero, the subroutine is a no-op.  If
      ** the processing calls for the query to abort, this subroutine
      ** increments the iAbortFlag memory location before returning in
      ** order to signal the caller to abort.
      */
      addr_سيٽ_Abort = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, 1, iAbortFlag);
      VdbeComment((v, "set abort flag"));
      sqlite3_Vdbe_جوڙيو_Op1(v, OP_Return, reg_Output_صف);
      sqlite3VdbeResolveLabel(v, addr_Output_صف);
      addr_Output_صف = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_IfPos, iUseFlag, addr_Output_صف+2);
      VdbeCoverage(v);
      VdbeComment((v, "Groupby result generator entry point %d", p->selId));
      sqlite3_Vdbe_جوڙيو_Op1(v, OP_Return, reg_Output_صف);
      finalizeAggFunctions(pParse, pAggInfo);
      sqlite3_اظهار_If_False(pParse, pHaving, addr_Output_صف+1, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, &s_ترتيب_ڏيو,
                      &sDistinct, pDest,
                      addr_Output_صف+1, addr_سيٽ_Abort);
      sqlite3_Vdbe_جوڙيو_Op1(v, OP_Return, reg_Output_صف);
      VdbeComment((v, "end groupby result generator %d", p->selId));

      /* Generate a subroutine that will reset the group-by accumulator
      */
      sqlite3VdbeResolveLabel(v, addrReset);
      reset_گڏ_ڪندڙ(pParse, pAggInfo);
      sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, 0, iUseFlag);
      VdbeComment((v, "indicate accumulator %d empty", p->selId));
      sqlite3_Vdbe_جوڙيو_Op1(v, OP_Return, regReset);

      إذا( distFlag!=0 && eDist!=WHERE_DISTINCT_NOOP ){
        بنية AggInfo_func *pF = &pAggInfo->aFunc[0];
        fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
      }
    } /* endif pGroupBy.  Begin aggregate queries without GROUP BY: */
    وإلا {
      /* Aggregate functions without GROUP BY. tag-select-0820 */
      جدول *pTab;
      إذا( (pTab = is_Simple_ڳڻپ(p, pAggInfo))!=0 ){
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
        ثابت صحيح iDb = sqlite3_Schema_منزل_اشاريو(pParse->db, pTab->pSchema);
        ثابت صحيح iCsr = pParse->nTab++;     /* Cursor to scan b-tree */
        اشاريو *pIdx;                         /* Iterator variable */
        ڪنجي_Info *p_ڪنجي_Info = 0;               /* Keyinfo for scanned index */
        اشاريو *p_بهترين = 0;                    /* Best index found so far */
        Pgno i_پاڙ = pTab->tnum;             /* Root page of scanned b-tree */

        sqlite3_ڪوڊ_Verify_Schema(pParse, iDb);
        sqlite3_جدول_تالو(pParse, iDb, pTab->tnum, 0, pTab->zName);

        /* Search for the index that has the lowest scan cost.
        **
        ** (2011-04-15) Do not do a full scan of an unordered index.
        **
        ** (2013-10-03) Do not count the entries in a partial index.
        **
        ** In practice the KeyInfo structure will not be used. It is only
        ** passed to keep OP_OpenRead happy.
        */
        إذا( !HasRowid(pTab) ) p_بهترين = sqlite3_Primary_ڪنجي_اشاريو(pTab);
        إذا( !p->pSrc->a[0].fg.not_اشاريي_وارو ){
          لكل(pIdx=pTab->p_اشاريو; pIdx; pIdx=pIdx->p_ايندڙ){
            إذا( pIdx->bUnordered==0
             && pIdx->sz_Idx_صف<pTab->sz_Tab_صف
             && pIdx->pPartIdxWhere==0
             && (!p_بهترين || pIdx->sz_Idx_صف<p_بهترين->sz_Idx_صف)
            ){
              p_بهترين = pIdx;
            }
          }
        }
        إذا( p_بهترين ){
          i_پاڙ = p_بهترين->tnum;
          p_ڪنجي_Info = sqlite3_ڪنجي_Info_جو_اشاريو(pParse, p_بهترين);
        }

        /* Open a read-only cursor, execute the OP_Count, close the cursor. */
        sqlite3_Vdbe_جوڙيو_Op4_Int(v, OP_OpenRead, iCsr, (صحيح)i_پاڙ, iDb, 1);
        إذا( p_ڪنجي_Info ){
          sqlite3VdbeChangeP4(v, -1, (حرف *)p_ڪنجي_Info, P4_KEYINFO);
        }
        سونپيو_Aggregate_Registers(pParse, pAggInfo);
        sqlite3_Vdbe_جوڙيو_Op2(v, OP_ڳڻپ, iCsr, AggInfoFuncReg(pAggInfo,0));
        sqlite3_Vdbe_جوڙيو_Op1(v, OP_Close, iCsr);
        explain_Simple_ڳڻپ(pParse, pTab, p_بهترين);
      }وإلا{
        /* The general case of an aggregate query without GROUP BY
        ** tag-select-0822 */
        صحيح regAcc = 0;           /* "populate accumulators" flag */
        اظهار_List *pDistinct = 0;
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
        إذا( pAggInfo->n_گڏ_ڪندڙ ){
          لكل(i=0; i<pAggInfo->nFunc; i++){
            إذا( اظهار_Has_Property(pAggInfo->aFunc[i].pFExpr, EP_WinFunc) ){
              استمر;
            }
            إذا( pAggInfo->aFunc[i].pFunc->funcFlags&SQLITE_FUNC_NEEDCOLL ){
              اكسر;
            }
          }
          إذا( i==pAggInfo->nFunc ){
            regAcc = ++pParse->nMem;
            sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, 0, regAcc);
          }
        }وإلا إذا( pAggInfo->nFunc==1 && pAggInfo->aFunc[0].iDistinct>=0 ){
          أكد( اظهار_Use_XList(pAggInfo->aFunc[0].pFExpr) );
          pDistinct = pAggInfo->aFunc[0].pFExpr->x.pList;
          distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
        }
        سونپيو_Aggregate_Registers(pParse, pAggInfo);

        /* This case runs if the aggregate has no GROUP BY clause.  The
        ** processing is much simpler since there is only a single row
        ** of output.
        */
        أكد( p->p_گروهه_By==0 );
        reset_گڏ_ڪندڙ(pParse, pAggInfo);

        /* If this query is a candidate for the min/max optimization, then
        ** minMaxFlag will have been previously set to either
        ** WHERE_ORDERBY_MIN or WHERE_ORDERBY_MAX and pMinMaxOrderBy will
        ** be an appropriate ORDER BY expression for the optimization.
        */
        أكد( minMaxFlag==WHERE_ORDERBY_عام || p_Min_Max_ترتيب_By!=0 );
        أكد( p_Min_Max_ترتيب_By==0 || p_Min_Max_ترتيب_By->n_اظهار==1 );

        TREETRACE(0x2,pParse,p,("WhereBegin\n"));
        pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, p_Min_Max_ترتيب_By,
                                   pDistinct, p, minMaxFlag|distFlag, 0);
        إذا( pWInfo==0 ){
          اذهب select_پڇاڙي;
        }
        TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
        eDist = sqlite3WhereIsDistinct(pWInfo);
        update_گڏ_ڪندڙ(pParse, regAcc, pAggInfo, eDist);
        إذا( eDist!=WHERE_DISTINCT_NOOP ){
          بنية AggInfo_func *pF = pAggInfo->aFunc;
          إذا( pF ){
            fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
          }
        }

        إذا( regAcc ) sqlite3_Vdbe_جوڙيو_Op2(v, OP_Integer, 1, regAcc);
        إذا( minMaxFlag ){
          sqlite3_Where_Min_Max_Opt_Early_ٻاهريون(v, pWInfo);
        }
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_پڇاڙي(pWInfo);
        finalizeAggFunctions(pParse, pAggInfo);
      }

      s_ترتيب_ڏيو.p_ترتيب_By = 0;
      sqlite3_اظهار_If_False(pParse, pHaving, addr_پڇاڙي, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, 0, 0,
                      pDest, addr_پڇاڙي, addr_پڇاڙي);
    }
    sqlite3VdbeResolveLabel(v, addr_پڇاڙي);

  } /* endif aggregate query */

  إذا( sDistinct.eTnctType==WHERE_DISTINCT_UNORDERED ){
    explain_عارضي_جدول(pParse, "DISTINCT");
  }

  /* If there is an ORDER BY clause, then we need to sort the results
  ** and send them to the callback one by one.  tag-select-0900
  */
  إذا( s_ترتيب_ڏيو.p_ترتيب_By ){
    أكد( p->pEList==pEList );
    generate_ترتيب_ڏيو_پڇ(pParse, p, &s_ترتيب_ڏيو, pEList->n_اظهار, pDest);
  }

  /* Jump here to skip this query
  */
  sqlite3VdbeResolveLabel(v, i_پڇاڙي);

  /* The SELECT has been coded. If there is an error in the Parse structure,
  ** set the return code to 1. Otherwise 0. */
  rc = (pParse->nErr>0);

  /* Control jumps to here if an error is encountered above, or upon
  ** successful coding of the SELECT.
  */
select_پڇاڙي:
  أكد( db->mallocFailed==0 || db->mallocFailed==1 );
  أكد( db->mallocFailed==0 || pParse->nErr!=0 );
  sqlite3_اظهار_List_Delete(db, p_Min_Max_ترتيب_By);
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
  /* Internal self-checks.  tag-select-1000 */
  إذا( pAggInfo && !db->mallocFailed ){
#إذا TREETRACE_ENABLED
    إذا( sqlite3_وڻ_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("Finished with AggInfo\n"));
      printAggInfo(pAggInfo);
    }
#جيڪڏهن_پورو
    لكل(i=0; i<pAggInfo->n_ڪالم; i++){
      اظهار *p_اظهار = pAggInfo->a_کانو[i].pCExpr;
      إذا( p_اظهار==0 ) استمر;
      أكد( p_اظهار->pAggInfo==pAggInfo );
      أكد( p_اظهار->iAgg==i );
    }
    لكل(i=0; i<pAggInfo->nFunc; i++){
      اظهار *p_اظهار = pAggInfo->aFunc[i].pFExpr;
      أكد( p_اظهار!=0 );
      أكد( p_اظهار->pAggInfo==pAggInfo );
      أكد( p_اظهار->iAgg==i );
    }
  }
#جيڪڏهن_پورو

#إذا TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p,("end processing\n"));
  إذا( (sqlite3_وڻ_Trace & 0x40000)!=0 && Explain_Query_Plan_والد(pParse)==0 ){
    sqlite3_وڻ_View_Select(0, p, 0);
  }
#جيڪڏهن_پورو
  Explain_Query_Plan_ڪڍو(pParse);
  أرجع rc;
}
