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
#شامل "sqliteInt.h"

/*
** An instance of the following object is used to record information about
** how to process the DISTINCT keyword, to simplify passing that information
** into the selectInnerLoop() routine.
*/
قسمتعریف ساخت DistinctCtx DistinctCtx;
ساخت DistinctCtx {
  u8 isTnct;      /* 0: Not distinct. 1: DISTINCT  2: DISTINCT and ORDER BY */
  u8 eTnctType;   /* One of the WHERE_DISTINCT_* operators */
  صحیح tabTnct;    /* Ephemeral table used for DISTINCT processing */
  صحیح addrTnct;   /* Address of OP_OpenEphemeral opcode for tabTnct */
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
قسمتعریف ساخت ترتیب_دو_Ctx ترتیب_دو_Ctx;
ساخت ترتیب_دو_Ctx {
  اظہاریہ_List *p_ترتیب_By;   /* The ORDER BY (or GROUP BY clause) */
  صحیح nOBSat;           /* Number of ORDER BY terms satisfied by indices */
  صحیح iECursor;         /* Cursor number for the sorter */
  صحیح regReturn;        /* Register holding block-output return address */
  صحیح label_Bk_برآمد;       /* Start label for the block-output subroutine */
  صحیح addr_ترتیب_دو_اشاریہ;    /* Address of the OP_SorterOpen or OP_OpenEphemeral */
  صحیح label_مکمل;        /* Jump here when done, ex: LIMIT reached */
  صحیح labelOBLopt;      /* Jump here when sorter is full */
  u8 ترتیب_دو_Flags;         /* Zero or more SORTFLAG_* bits */
#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
  u8 nDefer;            /* Number of valid entries in aDefer[] */
  ساخت DeferredCsr {
    جدول *pTab;        /* Table definition */
    صحیح iCsr;           /* Cursor number for table */
    صحیح n_کلید;           /* Number of PK columns for table pTab (>=1) */
  } aDefer[4];
#اگر_ختم
  ساخت صف_بھرو_Info *p_Deferred_صف_بھرو;  /* Deferred row loading info or NULL */
#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
  صحیح addr_ڈالو;         /* First instruction to push data into sorter */
  صحیح addr_ڈالو_انتہا;      /* Last instruction that pushes data into sorter */
#اگر_ختم
};
#تعریف_کرو SORTFLAG_UseSorter  0x01   /* Use SorterOpen instead of OpenEphemeral */

/*
** Delete all the content of a Select structure.  Deallocate the structure
** itself depending on the value of bFree
**
** If bFree==1, call sqlite3DbFree() on the p object.
** If bFree==0, Leave the first Select object unfreed
*/
ساکن خالی clearSelect(sqlite3 *db, Select *p, صحیح bFree){
  أكد( db!=0 );
  جب( p ){
    Select *p_پیشگی = p->p_پیشگی;
    sqlite3_اظہاریہ_List_Delete(db, p->pEList);
    sqlite3SrcListDelete(db, p->pSrc);
    sqlite3_اظہاریہ_Delete(db, p->pWhere);
    sqlite3_اظہاریہ_List_Delete(db, p->p_گروہ_By);
    sqlite3_اظہاریہ_Delete(db, p->pHaving);
    sqlite3_اظہاریہ_List_Delete(db, p->p_ترتیب_By);
    sqlite3_اظہاریہ_Delete(db, p->p_حد_بندی);
    اگر( OK_IF_ALWAYS_TRUE(p->pWith) ) sqlite3WithDelete(db, p->pWith);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    اگر( OK_IF_ALWAYS_TRUE(p->pWinDefn) ){
      sqlite3_کھڑکی_List_Delete(db, p->pWinDefn);
    }
    جب( p->pWin ){
      أكد( p->pWin->ppThis==&p->pWin );
      sqlite3_کھڑکی_Unlink_From_Select(p->pWin);
    }
#اگر_ختم
    اگر( bFree ) sqlite3DbNNFreeNN(db, p);
    p = p_پیشگی;
    bFree = 1;
  }
}

/*
** Initialize a SelectDest structure.
*/
خالی sqlite3SelectDestInit(SelectDest *pDest, صحیح eDest, صحیح iParm){
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
  اظہاریہ_List *pEList,     /* which columns to include in the result */
  SrcList *pSrc,        /* the FROM clause -- which tables to scan */
  اظہاریہ *pWhere,         /* the WHERE clause */
  اظہاریہ_List *p_گروہ_By,   /* the GROUP BY clause */
  اظہاریہ *pHaving,        /* the HAVING clause */
  اظہاریہ_List *p_ترتیب_By,   /* the ORDER BY clause */
  u32 selFlags,         /* Flag parameters, such as SF_Distinct */
  اظہاریہ *p_حد_بندی          /* LIMIT value.  NULL means not used */
){
  Select *pNew, *pAllocated;
  Select standin;
  pAllocated = pNew = sqlite3DbMallocRawNN(pParse->db, حجم(*pNew) );
  اگر( pNew==0 ){
    أكد( pParse->db->mallocFailed );
    pNew = &standin;
  }
  اگر( pEList==0 ){
    pEList = sqlite3_اظہاریہ_List_Append(pParse, 0,
                                   sqlite3_اظہاریہ(pParse->db,TK_ASTERISK,0));
  }
  pNew->pEList = pEList;
  pNew->op = TK_SELECT;
  pNew->selFlags = selFlags;
  pNew->i_حد_بندی = 0;
  pNew->i_انحراف = 0;
  pNew->selId = ++pParse->nSelect;
  pNew->n_Select_صف = 0;
  اگر( pSrc==0 ) pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  pNew->pSrc = pSrc;
  pNew->pWhere = pWhere;
  pNew->p_گروہ_By = p_گروہ_By;
  pNew->pHaving = pHaving;
  pNew->p_ترتیب_By = p_ترتیب_By;
  pNew->p_پیشگی = 0;
  pNew->p_اگلا = 0;
  pNew->p_حد_بندی = p_حد_بندی;
  pNew->pWith = 0;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
  pNew->pWin = 0;
  pNew->pWinDefn = 0;
#اگر_ختم
  اگر( pParse->db->mallocFailed ) {
    clearSelect(pParse->db, pNew, pNew!=&standin);
    pAllocated = 0;
  }ورنہ{
    أكد( pNew->pSrc!=0 || pParse->nErr>0 );
  }
  واپس pAllocated;
}


/*
** Delete the given Select structure and all of its substructures.
*/
خالی sqlite3SelectDelete(sqlite3 *db, Select *p){
  اگر( OK_IF_ALWAYS_TRUE(p) ) clearSelect(db, p, 1);
}
خالی sqlite3SelectDeleteGeneric(sqlite3 *db, خالی *p){
  اگر( ALWAYS(p) ) clearSelect(db, (Select*)p, 1);
}

/*
** Return a pointer to the right-most SELECT statement in a compound.
*/
ساکن Select *ڈھونڈو_Rightmost(Select *p){
  جب( p->p_اگلا ) p = p->p_اگلا;
  واپس p;
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
صحیح sqlite3JoinType(Parse *pParse, Token *pA, Token *pB, Token *pC){
  صحیح jointype = 0;
  Token *apAll[3];
  Token *p;
                             /*   0123456789 123456789 123456789 123 */
  ساکن ثابت حرف z_کلید_متن[] = "naturaleftouterightfullinnercross";
  ساکن ثابت ساخت {
    u8 i;        /* Beginning of keyword text in zKeyText[] */
    u8 nChar;    /* Length of the keyword in characters */
    u8 رمز;     /* Join type mask */
  } aKeyword[] = {
    /* (0) natural */ { 0,  7, JT_NATURAL                },
    /* (1) left    */ { 6,  4, JT_بایاں|JT_OUTER          },
    /* (2) outer   */ { 10, 5, JT_OUTER                  },
    /* (3) right   */ { 14, 5, JT_دایاں|JT_OUTER         },
    /* (4) full    */ { 19, 4, JT_بایاں|JT_دایاں|JT_OUTER },
    /* (5) inner   */ { 23, 5, JT_INNER                  },
    /* (6) cross   */ { 28, 5, JT_INNER|JT_CROSS         },
  };
  صحیح i, j;
  apAll[0] = pA;
  apAll[1] = pB;
  apAll[2] = pC;
  برائے(i=0; i<3 && apAll[i]; i++){
    p = apAll[i];
    برائے(j=0; j<Array_سائز(aKeyword); j++){
      اگر( p->n==aKeyword[j].nChar
          && sqlite3StrNICmp((حرف*)p->z, &z_کلید_متن[aKeyword[j].i], p->n)==0 ){
        jointype |= aKeyword[j].رمز;
        توڑو;
      }
    }
    testcase( j==0 || j==1 || j==2 || j==3 || j==4 || j==5 || j==6 );
    اگر( j>=Array_سائز(aKeyword) ){
      jointype |= JT_ERROR;
      توڑو;
    }
  }
  اگر(
     (jointype & (JT_INNER|JT_OUTER))==(JT_INNER|JT_OUTER) ||
     (jointype & JT_ERROR)!=0 ||
     (jointype & (JT_OUTER|JT_بایاں|JT_دایاں))==JT_OUTER
  ){
    ثابت حرف *zSp1 = " ";
    ثابت حرف *zSp2 = " ";
    اگر( pB==0 ){ zSp1++; }
    اگر( pC==0 ){ zSp2++; }
    sqlite3_Error_پیغام(pParse, "unknown join type: "
       "%T%s%T%s%T", pA, zSp1, pB, zSp2, pC);
    jointype = JT_INNER;
  }
  واپس jointype;
}

/*
** Return the index of a column in a table.  Return -1 if the column
** is not contained in the table.
*/
صحیح sqlite3_عمود_اشاریہ(جدول *pTab, ثابت حرف *z_ستون){
  صحیح i;
  u8 h;
  ثابت عمود *a_ستون;
  صحیح n_ستون;

  h = sqlite3StrIHash(z_ستون);
  a_ستون = pTab->a_ستون;
  n_ستون = pTab->n_ستون;

  /* See if the aHx gives us a lucky match */
  i = pTab->aHx[h % حجم(pTab->aHx)];
  أكد( i<n_ستون );
  اگر( a_ستون[i].hName==h
   && sqlite3StrICmp(a_ستون[i].zCnName, z_ستون)==0
  ){
    واپس i;
  }

  /* No lucky match from the hash table.  Do a full search. */
  i = 0;
  جب( 1 /*exit-by-break*/ ){
    اگر( a_ستون[i].hName==h
     && sqlite3StrICmp(a_ستون[i].zCnName, z_ستون)==0
    ){
      واپس i;
    }
    i++;
    اگر( i>=n_ستون ) توڑو;
  }
  واپس -1;
}

/*
** Mark a subquery result column as having been used.
*/
خالی sqlite3_Src_شے_عمود_مستعمل(Src_شے *p_شے, صحیح i_ستون){
  أكد( p_شے!=0 );
  أكد( (صحیح)p_شے->fg.isNestedFrom == IsNestedFrom(p_شے) );
  اگر( p_شے->fg.isNestedFrom ){
    اظہاریہ_List *pResults;
    أكد( p_شے->fg.isSubquery );
    أكد( p_شے->u4.pSubq!=0 );
    أكد( p_شے->u4.pSubq->pSelect!=0 );
    pResults = p_شے->u4.pSubq->pSelect->pEList;
    أكد( pResults!=0 );
    أكد( i_ستون>=0 && i_ستون<pResults->n_اظہاریہ );
    pResults->a[i_ستون].fg.b_مستعمل = 1;
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
ساکن صحیح جدول_And_عمود_اشاریہ(
  SrcList *pSrc,       /* Array of tables to search */
  صحیح i_آغاز,          /* First member of pSrc->a[] to check */
  صحیح i_انتہا,            /* Last member of pSrc->a[] to check */
  ثابت حرف *z_ستون,    /* Name of the column we are looking for */
  صحیح *piTab,          /* Write index of pSrc->a[] here */
  صحیح *pi_ستون,          /* Write index of pSrc->a[*piTab].pSTab->aCol[] here */
  صحیح bIgnoreHidden    /* Ignore hidden columns */
){
  صحیح i;               /* For looping over tables in pSrc */
  صحیح i_ستون;            /* Index of column matching zCol */

  أكد( i_انتہا<pSrc->nSrc );
  أكد( i_آغاز>=0 );
  أكد( (piTab==0)==(pi_ستون==0) );  /* Both or neither are NULL */

  برائے(i=i_آغاز; i<=i_انتہا; i++){
    i_ستون = sqlite3_عمود_اشاریہ(pSrc->a[i].pSTab, z_ستون);
    اگر( i_ستون>=0
     && (bIgnoreHidden==0 || Is_Hidden_عمود(&pSrc->a[i].pSTab->a_ستون[i_ستون])==0)
    ){
      اگر( piTab ){
        sqlite3_Src_شے_عمود_مستعمل(&pSrc->a[i], i_ستون);
        *piTab = i;
        *pi_ستون = i_ستون;
      }
      واپس 1;
    }
  }
  واپس 0;
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
خالی sqlite3_سیٹ_Join_اظہاریہ(اظہاریہ *p, صحیح i_جدول, u32 joinFlag){
  أكد( joinFlag==EP_OuterON || joinFlag==EP_InnerON );
  جب( p ){
    اظہاریہ_سیٹ_Property(p, joinFlag);
    أكد( !اظہاریہ_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    اظہاریہ_سیٹ_VVAProperty(p, EP_NoReduce);
    p->w.iJoin = i_جدول;
    اگر( اظہاریہ_Use_XList(p) ){
      اگر( p->x.pList ){
        صحیح i;
        برائے(i=0; i<p->x.pList->n_اظہاریہ; i++){
          sqlite3_سیٹ_Join_اظہاریہ(p->x.pList->a[i].p_اظہاریہ, i_جدول, joinFlag);
        }
      }
    }
    sqlite3_سیٹ_Join_اظہاریہ(p->p_بایاں, i_جدول, joinFlag);
    p = p->p_دایاں;
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
ساکن خالی unset_Join_اظہاریہ(اظہاریہ *p, صحیح i_جدول, صحیح nullable){
  جب( p ){
    اگر( i_جدول<0 || (اظہاریہ_Has_Property(p, EP_OuterON) && p->w.iJoin==i_جدول) ){
      اظہاریہ_Clear_Property(p, EP_OuterON|EP_InnerON);
      اگر( i_جدول>=0 ) اظہاریہ_سیٹ_Property(p, EP_InnerON);
    }
    اگر( p->op==TK_عمود && p->i_جدول==i_جدول && !nullable ){
      اظہاریہ_Clear_Property(p, EP_CanBeNull);
    }
    اگر( p->op==TK_FUNCTION ){
      أكد( اظہاریہ_Use_XList(p) );
      أكد( p->p_بایاں==0 );
      اگر( p->x.pList ){
        صحیح i;
        برائے(i=0; i<p->x.pList->n_اظہاریہ; i++){
          unset_Join_اظہاریہ(p->x.pList->a[i].p_اظہاریہ, i_جدول, nullable);
        }
      }
    }
    unset_Join_اظہاریہ(p->p_بایاں, i_جدول, nullable);
    p = p->p_دایاں;
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
ساکن صحیح sqlite3ProcessJoin(Parse *pParse, Select *p){
  SrcList *pSrc;                  /* All tables in the FROM clause */
  صحیح i, j;                       /* Loop counters */
  Src_شے *p_بایاں;                 /* Left table being joined */
  Src_شے *p_دایاں;                /* Right table being joined */
  صحیح b_نظر_آیا_TFunc = 0;             /* True if have seen table-valued function */

  pSrc = p->pSrc;
  p_بایاں = &pSrc->a[0];
  p_دایاں = &p_بایاں[1];
  برائے(i=0; i<pSrc->nSrc-1; i++, p_دایاں++, p_بایاں++){
    جدول *p_دایاں_Tab = p_دایاں->pSTab;
    u32 joinType;

    اگر( NEVER(p_بایاں->pSTab==0 || p_دایاں_Tab==0) ) جاری;
    joinType = (p_دایاں->fg.jointype & JT_OUTER)!=0 ? EP_OuterON : EP_InnerON;

    /* If this is a NATURAL join, synthesize an appropriate USING clause
    ** to specify which columns should be joined.
    */
    اگر( p_دایاں->fg.jointype & JT_NATURAL ){
      IdList *pUsing = 0;
      اگر( p_دایاں->fg.isUsing || p_دایاں->u3.pOn ){
        sqlite3_Error_پیغام(pParse, "a NATURAL join may not have "
           "an ON or USING clause", 0);
        واپس 1;
      }
      برائے(j=0; j<p_دایاں_Tab->n_ستون; j++){
        حرف *zName;   /* Name of column in the right table */

        اگر( Is_Hidden_عمود(&p_دایاں_Tab->a_ستون[j]) ) جاری;
        zName = p_دایاں_Tab->a_ستون[j].zCnName;
        اگر( جدول_And_عمود_اشاریہ(pSrc, 0, i, zName, 0, 0, 1) ){
          pUsing = sqlite3IdListAppend(pParse, pUsing, 0);
          اگر( pUsing ){
            أكد( pUsing->nId>0 );
            أكد( pUsing->a[pUsing->nId-1].zName==0 );
            pUsing->a[pUsing->nId-1].zName = sqlite3DbStrDup(pParse->db, zName);
          }
        }
      }
      اگر( pUsing ){
        p_دایاں->fg.isUsing = 1;
        p_دایاں->fg.isSynthUsing = 1;
        p_دایاں->u3.pUsing = pUsing;
      }
      اگر( pParse->nErr ) واپس 1;
    }

    /* Create extra terms on the WHERE clause for each column named
    ** in the USING clause.  Example: If the two tables to be joined are
    ** A and B and the USING clause names X, Y, and Z, then add this
    ** to the WHERE clause:    A.X=B.X AND A.Y=B.Y AND A.Z=B.Z
    ** Report an error if any column mentioned in the USING clause is
    ** not contained in both tables to be joined.
    */
    اگر( p_دایاں->fg.isUsing ){
      IdList *pList = p_دایاں->u3.pUsing;
      sqlite3 *db = pParse->db;
      أكد( pList!=0 );
      برائے(j=0; j<pList->nId; j++){
        حرف *zName;     /* Name of the term in the USING clause */
        صحیح i_بایاں;       /* Table on the left with matching column name */
        صحیح i_بایاں_ستون;    /* Column number of matching column on the left */
        صحیح i_دایاں_ستون;   /* Column number of matching column on the right */
        اظہاریہ *pE1;       /* Reference to the column on the LEFT of the join */
        اظہاریہ *pE2;       /* Reference to the column on the RIGHT of the join */
        اظہاریہ *pEq;       /* Equality constraint.  pE1 == pE2 */

        zName = pList->a[j].zName;
        i_دایاں_ستون = sqlite3_عمود_اشاریہ(p_دایاں_Tab, zName);
        اگر( i_دایاں_ستون<0
         || جدول_And_عمود_اشاریہ(pSrc, 0, i, zName, &i_بایاں, &i_بایاں_ستون,
                                p_دایاں->fg.isSynthUsing)==0
        ){
          sqlite3_Error_پیغام(pParse, "cannot join using column %s - column "
            "not present in both tables", zName);
          واپس 1;
        }
        pE1 = sqlite3_Create_عمود_اظہاریہ(db, pSrc, i_بایاں, i_بایاں_ستون);
        sqlite3_Src_شے_عمود_مستعمل(&pSrc->a[i_بایاں], i_بایاں_ستون);
        اگر( (pSrc->a[0].fg.jointype & JT_LTORJ)!=0 && pParse->nErr==0 ){
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
          اظہاریہ_List *p_Func_دلائل = 0;   /* Arguments to the coalesce() */
          ساکن ثابت Token tkCoalesce = { "coalesce", 8 };
          أكد( pE1!=0 );
          اظہاریہ_سیٹ_Property(pE1, EP_CanBeNull);
          جب( جدول_And_عمود_اشاریہ(pSrc, i_بایاں+1, i, zName, &i_بایاں, &i_بایاں_ستون,
                                     p_دایاں->fg.isSynthUsing)!=0 ){
            اگر( pSrc->a[i_بایاں].fg.isUsing==0
             || sqlite3_Id_List_اشاریہ(pSrc->a[i_بایاں].u3.pUsing, zName)<0
            ){
              sqlite3_Error_پیغام(pParse, "ambiguous reference to %s in USING()",
                              zName);
              توڑو;
            }
            p_Func_دلائل = sqlite3_اظہاریہ_List_Append(pParse, p_Func_دلائل, pE1);
            pE1 = sqlite3_Create_عمود_اظہاریہ(db, pSrc, i_بایاں, i_بایاں_ستون);
            sqlite3_Src_شے_عمود_مستعمل(&pSrc->a[i_بایاں], i_بایاں_ستون);
          }
          اگر( p_Func_دلائل ){
            p_Func_دلائل = sqlite3_اظہاریہ_List_Append(pParse, p_Func_دلائل, pE1);
            pE1 = sqlite3_اظہاریہ_Function(pParse, p_Func_دلائل, &tkCoalesce, 0);
            اگر( pE1 ){
              pE1->aff_اظہاریہ = SQLITE_AFF_DEFER;
            }
          }
        }ورنہ اگر( (pSrc->a[i+1].fg.jointype & JT_بایاں)!=0 && pParse->nErr==0 ){
          أكد( pE1!=0 );
          اظہاریہ_سیٹ_Property(pE1, EP_CanBeNull);
        }
        pE2 = sqlite3_Create_عمود_اظہاریہ(db, pSrc, i+1, i_دایاں_ستون);
        sqlite3_Src_شے_عمود_مستعمل(p_دایاں, i_دایاں_ستون);
        pEq = sqlite3PExpr(pParse, TK_EQ, pE1, pE2);
        أكد( pE2!=0 || pEq==0 );
        اگر( pEq ){
          اظہاریہ_سیٹ_Property(pEq, joinType);
          أكد( !اظہاریہ_Has_Property(pEq, EP_TokenOnly|EP_Reduced) );
          اظہاریہ_سیٹ_VVAProperty(pEq, EP_NoReduce);
          pEq->w.iJoin = pE2->i_جدول;
        }
        p->pWhere = sqlite3_اظہاریہ_And(pParse, p->pWhere, pEq);
      }
    }

    /* Add the ON clause to the end of the WHERE clause, connected by
    ** an AND operator.
    */
    ورنہ اگر( p_دایاں->u3.pOn ){
      sqlite3_سیٹ_Join_اظہاریہ(p_دایاں->u3.pOn, p_دایاں->iCursor, joinType);
      p->pWhere = sqlite3_اظہاریہ_And(pParse, p->pWhere, p_دایاں->u3.pOn);
      p_دایاں->u3.pOn = 0;
      p_دایاں->fg.isOn = 1;
      p->selFlags |= SF_On_مقصد_Where;
    }

    اگر( b_نظر_آیا_TFunc==0 ){
      b_نظر_آیا_TFunc = (p_بایاں->fg.isTabFunc && p_بایاں->u1.pFuncArg);
    }

    /* We also need to call sqlite3SelectCheckOnClauses() to verify that
    ** table-valued function arguments do not illegally refer to any tables to
    ** their right. This test is required if either (a) there is a RIGHT JOIN
    ** in the SrcList, or (b) there is a LEFT JOIN to the right of a 
    ** table-valued function.  */
    اگر( (p_دایاں->fg.isTabFunc && joinType==EP_OuterON && p_دایاں->u1.pFuncArg)
     || (b_نظر_آیا_TFunc && joinType==EP_OuterON)
    ){
      p->selFlags |= SF_On_مقصد_Where;
    }
  }
  واپس 0;
}

/*
** An instance of this object holds information (beyond pParse and pSelect)
** needed to load the next result row that is to be added to the sorter.
*/
قسمتعریف ساخت صف_بھرو_Info صف_بھرو_Info;
ساخت صف_بھرو_Info {
  صحیح reg_نتیجہ;               /* Store results in array of registers here */
  u8 ecelFlags;                /* Flag argument to ExprCodeExprList() */
#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
  اظہاریہ_List *pExtra;            /* Extra columns needed by sorter refs */
  صحیح reg_Extra_نتیجہ;          /* Where to load the extra columns */
#اگر_ختم
};

/*
** This routine does the work of loading query data into an array of
** registers so that it can be added to the sorter.
*/
ساکن خالی inner_Loop_بھرو_صف(
  Parse *pParse,             /* Statement under construction */
  Select *pSelect,           /* The query being coded */
  صف_بھرو_Info *pInfo         /* Info needed to complete the row load */
){
  sqlite3_اظہاریہ_رمز_اظہاریہ_List(pParse, pSelect->pEList, pInfo->reg_نتیجہ,
                          0, pInfo->ecelFlags);
#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
  اگر( pInfo->pExtra ){
    sqlite3_اظہاریہ_رمز_اظہاریہ_List(pParse, pInfo->pExtra, pInfo->reg_Extra_نتیجہ, 0, 0);
    sqlite3_اظہاریہ_List_Delete(pParse->db, pInfo->pExtra);
  }
#اگر_ختم
}

/*
** Code the OP_MakeRecord instruction that generates the entry to be
** added into the sorter.
**
** Return the register in which the result is stored.
*/
ساکن صحیح make_Sorter_ریکارڈ(
  Parse *pParse,
  ترتیب_دو_Ctx *p_ترتیب_دو,
  Select *pSelect,
  صحیح reg_بنیاد,
  صحیح n_بنیاد
){
  صحیح nOBSat = p_ترتیب_دو->nOBSat;
  Vdbe *v = pParse->pVdbe;
  صحیح reg_برآمد = ++pParse->nMem;
  اگر( p_ترتیب_دو->p_Deferred_صف_بھرو ){
    inner_Loop_بھرو_صف(pParse, pSelect, p_ترتیب_دو->p_Deferred_صف_بھرو);
  }
  sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, reg_بنیاد+nOBSat, n_بنیاد-nOBSat, reg_برآمد);
  واپس reg_برآمد;
}

/*
** Generate code that will push the record in registers regData
** through regData+nData-1 onto the sorter.
*/
ساکن خالی ڈالو_Onto_Sorter(
  Parse *pParse,         /* Parser context */
  ترتیب_دو_Ctx *p_ترتیب_دو,        /* Information about the ORDER BY clause */
  Select *pSelect,       /* The whole SELECT statement */
  صحیح reg_ڈیٹا,           /* First register holding data to be sorted */
  صحیح reg_Orig_ڈیٹا,       /* First register holding data before packing */
  صحیح n_ڈیٹا,             /* Number of elements in the regData data array */
  صحیح n_سابقہ_Reg         /* No. of reg prior to regData available for use */
){
  Vdbe *v = pParse->pVdbe;                         /* Stmt under construction */
  صحیح bSeq = ((p_ترتیب_دو->ترتیب_دو_Flags & SORTFLAG_UseSorter)==0);
  صحیح n_اظہاریہ = p_ترتیب_دو->p_ترتیب_By->n_اظہاریہ;              /* No. of ORDER BY terms */
  صحیح n_بنیاد = n_اظہاریہ + bSeq + n_ڈیٹا;                /* Fields in sorter record */
  صحیح reg_بنیاد;                                     /* Regs for sorter record */
  صحیح reg_ریکارڈ = 0;                               /* Assembled sorter record */
  صحیح nOBSat = p_ترتیب_دو->nOBSat;                      /* ORDER BY terms to skip */
  صحیح op;                            /* Opcode to add sorter record to sorter */
  صحیح i_حد_بندی;                        /* LIMIT counter */
  صحیح iSkip = 0;                     /* End of the sorter insert loop */

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
  أكد( n_ڈیٹا==1 || reg_ڈیٹا==reg_Orig_ڈیٹا || reg_Orig_ڈیٹا==0 );

#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
  p_ترتیب_دو->addr_ڈالو = sqlite3VdbeCurrentAddr(v);
#اگر_ختم

  اگر( n_سابقہ_Reg ){
    أكد( n_سابقہ_Reg==n_اظہاریہ+bSeq );
    reg_بنیاد = reg_ڈیٹا - n_سابقہ_Reg;
  }ورنہ{
    reg_بنیاد = pParse->nMem + 1;
    pParse->nMem += n_بنیاد;
  }
  أكد( pSelect->i_انحراف==0 || pSelect->i_حد_بندی!=0 );
  i_حد_بندی = pSelect->i_انحراف ? pSelect->i_انحراف+1 : pSelect->i_حد_بندی;
  p_ترتیب_دو->label_مکمل = sqlite3VdbeMakeLabel(pParse);
  sqlite3_اظہاریہ_رمز_اظہاریہ_List(pParse, p_ترتیب_دو->p_ترتیب_By, reg_بنیاد, reg_Orig_ڈیٹا,
                          SQLITE_ECEL_DUP | (reg_Orig_ڈیٹا? SQLITE_ECEL_REF : 0));
  اگر( bSeq ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Sequence, p_ترتیب_دو->iECursor, reg_بنیاد+n_اظہاریہ);
  }
  اگر( n_سابقہ_Reg==0 && n_ڈیٹا>0 ){
    sqlite3_اظہاریہ_رمز_Move(pParse, reg_ڈیٹا, reg_بنیاد+n_اظہاریہ+bSeq, n_ڈیٹا);
  }
  اگر( nOBSat>0 ){
    صحیح reg_پچھلا_کلید;   /* The first nOBSat columns of the previous row */
    صحیح addr_پہلا;    /* Address of the OP_IfNot opcode */
    صحیح addrJmp;      /* Address of the OP_Jump opcode */
    VdbeOp *pOp;      /* Opcode that opens the sorter */
    صحیح n_کلید;         /* Number of sorting key columns, including OP_Sequence */
    کلید_Info *pKI;     /* Original KeyInfo on the sorter table */

    reg_ریکارڈ = make_Sorter_ریکارڈ(pParse, p_ترتیب_دو, pSelect, reg_بنیاد, n_بنیاد);
    reg_پچھلا_کلید = pParse->nMem+1;
    pParse->nMem += p_ترتیب_دو->nOBSat;
    n_کلید = n_اظہاریہ - p_ترتیب_دو->nOBSat + bSeq;
    اگر( bSeq ){
      addr_پہلا = sqlite3_Vdbe_جوڑو_Op1(v, OP_IfNot, reg_بنیاد+n_اظہاریہ);
    }ورنہ{
      addr_پہلا = sqlite3_Vdbe_جوڑو_Op1(v, OP_SequenceTest, p_ترتیب_دو->iECursor);
    }
    VdbeCoverage(v);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Compare, reg_پچھلا_کلید, reg_بنیاد, p_ترتیب_دو->nOBSat);
    pOp = sqlite3VdbeGetOp(v, p_ترتیب_دو->addr_ترتیب_دو_اشاریہ);
    اگر( pParse->db->mallocFailed ) واپس;
    pOp->p2 = n_کلید + n_ڈیٹا;
    pKI = pOp->p4.p_کلید_Info;
    memset(pKI->a_ترتیب_دو_Flags, 0, pKI->n_کلید_میدان); /* Makes OP_Jump testable */
    sqlite3VdbeChangeP4(v, -1, (حرف*)pKI, P4_KEYINFO);
    testcase( pKI->n_All_میدان > pKI->n_کلید_میدان+2 );
    pOp->p4.p_کلید_Info = sqlite3_کلید_Info_From_اظہاریہ_List(pParse,p_ترتیب_دو->p_ترتیب_By,nOBSat,
                                           pKI->n_All_میدان-pKI->n_کلید_میدان-1);
    pOp = 0; /* Ensure pOp not used after sqlite3VdbeAddOp3() */
    addrJmp = sqlite3VdbeCurrentAddr(v);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Jump, addrJmp+1, 0, addrJmp+1); VdbeCoverage(v);
    p_ترتیب_دو->label_Bk_برآمد = sqlite3VdbeMakeLabel(pParse);
    p_ترتیب_دو->regReturn = ++pParse->nMem;
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, p_ترتیب_دو->regReturn, p_ترتیب_دو->label_Bk_برآمد);
    sqlite3_Vdbe_جوڑو_Op1(v, OP_ResetSorter, p_ترتیب_دو->iECursor);
    اگر( i_حد_بندی ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_IfNot, i_حد_بندی, p_ترتیب_دو->label_مکمل);
      VdbeCoverage(v);
    }
    sqlite3_Vdbe_Jump_یہاں(v, addr_پہلا);
    sqlite3_اظہاریہ_رمز_Move(pParse, reg_بنیاد, reg_پچھلا_کلید, p_ترتیب_دو->nOBSat);
    sqlite3_Vdbe_Jump_یہاں(v, addrJmp);
  }
  اگر( i_حد_بندی ){
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
    صحیح iCsr = p_ترتیب_دو->iECursor;
    sqlite3_Vdbe_جوڑو_Op2(v, OP_IfNotZero, i_حد_بندی, sqlite3VdbeCurrentAddr(v)+4);
    VdbeCoverage(v);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_آخری, iCsr, 0);
    iSkip = sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_IdxLE,
                                 iCsr, 0, reg_بنیاد+nOBSat, n_اظہاریہ-nOBSat);
    VdbeCoverage(v);
    sqlite3_Vdbe_جوڑو_Op1(v, OP_Delete, iCsr);
  }
  اگر( reg_ریکارڈ==0 ){
    reg_ریکارڈ = make_Sorter_ریکارڈ(pParse, p_ترتیب_دو, pSelect, reg_بنیاد, n_بنیاد);
  }
  اگر( p_ترتیب_دو->ترتیب_دو_Flags & SORTFLAG_UseSorter ){
    op = OP_Sorter_داخل_کرو;
  }ورنہ{
    op = OP_Idx_داخل_کرو;
  }
  sqlite3_Vdbe_جوڑو_Op4_Int(v, op, p_ترتیب_دو->iECursor, reg_ریکارڈ,
                       reg_بنیاد+nOBSat, n_بنیاد-nOBSat);
  اگر( iSkip ){
    sqlite3VdbeChangeP2(v, iSkip,
         p_ترتیب_دو->labelOBLopt ? p_ترتیب_دو->labelOBLopt : sqlite3VdbeCurrentAddr(v));
  }
#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
  p_ترتیب_دو->addr_ڈالو_انتہا = sqlite3VdbeCurrentAddr(v)-1;
#اگر_ختم
}

/*
** Add code to implement the OFFSET
*/
ساکن خالی رمز_انحراف(
  Vdbe *v,          /* Generate code into this VM */
  صحیح i_انحراف,      /* Register holding the offset counter */
  صحیح iContinue     /* Jump here to skip the current record */
){
  اگر( i_انحراف>0 ){
    sqlite3_Vdbe_جوڑو_Op3(v, OP_IfPos, i_انحراف, iContinue, 1); VdbeCoverage(v);
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
ساکن صحیح رمز_Distinct(
  Parse *pParse,     /* Parsing and code generating context */
  صحیح eTnctType,     /* WHERE_DISTINCT_* value */
  صحیح iTab,          /* A sorting index used to test for distinctness */
  صحیح addrRepeat,    /* Jump to here if not distinct */
  اظہاریہ_List *pEList,  /* Expression for each element */
  صحیح regElem        /* First element */
){
  صحیح iRet = 0;
  صحیح n_نتیجہ_ستون = pEList->n_اظہاریہ;
  Vdbe *v = pParse->pVdbe;

  انتخاب( eTnctType ){
    حالت WHERE_DISTINCT_ORDERED: {
      صحیح i;
      صحیح iJump;              /* Jump destination */
      صحیح reg_پچھلا;            /* Previous row content */

      /* Allocate space for the previous row */
      iRet = reg_پچھلا = pParse->nMem+1;
      pParse->nMem += n_نتیجہ_ستون;

      iJump = sqlite3VdbeCurrentAddr(v) + n_نتیجہ_ستون;
      برائے(i=0; i<n_نتیجہ_ستون; i++){
        CollSeq *pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, pEList->a[i].p_اظہاریہ);
        اگر( i<n_نتیجہ_ستون-1 ){
          sqlite3_Vdbe_جوڑو_Op3(v, OP_Ne, regElem+i, iJump, reg_پچھلا+i);
          VdbeCoverage(v);
        }ورنہ{
          sqlite3_Vdbe_جوڑو_Op3(v, OP_Eq, regElem+i, addrRepeat, reg_پچھلا+i);
          VdbeCoverage(v);
         }
        sqlite3VdbeChangeP4(v, -1, (ثابت حرف *)pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
      }
      أكد( sqlite3VdbeCurrentAddr(v)==iJump || pParse->db->mallocFailed );
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Copy, regElem, reg_پچھلا, n_نتیجہ_ستون-1);
      توڑو;
    }

    حالت WHERE_DISTINCT_UNIQUE: {
      /* nothing to do */
      توڑو;
    }

    طےشدہ: {
      صحیح r1 = sqlite3_Get_عارضی_Reg(pParse);
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_ملا, iTab, addrRepeat, regElem, n_نتیجہ_ستون);
      VdbeCoverage(v);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, regElem, n_نتیجہ_ستون, r1);
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iTab, r1, regElem, n_نتیجہ_ستون);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      sqlite3_Release_عارضی_Reg(pParse, r1);
      iRet = iTab;
      توڑو;
    }
  }

  واپس iRet;
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
ساکن خالی fixDistinctOpenEph(
  Parse *pParse,     /* Parsing and code generating context */
  صحیح eTnctType,     /* WHERE_DISTINCT_* value */
  صحیح iVal,          /* Value returned by codeDistinct() */
  صحیح iOpenEphAddr   /* Address of OP_OpenEphemeral instruction for iTab */
){
  اگر( pParse->nErr==0
   && (eTnctType==WHERE_DISTINCT_UNIQUE || eTnctType==WHERE_DISTINCT_ORDERED)
  ){
    Vdbe *v = pParse->pVdbe;
    sqlite3_Vdbe_Change_مقصد_Noop(v, iOpenEphAddr);
    اگر( sqlite3VdbeGetOp(v, iOpenEphAddr+1)->opcode==OP_Explain ){
      sqlite3_Vdbe_Change_مقصد_Noop(v, iOpenEphAddr+1);
    }
    اگر( eTnctType==WHERE_DISTINCT_ORDERED ){
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

#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
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
ساکن خالی select_اظہاریہ_Defer(
  Parse *pParse,                  /* Leave any error here */
  ترتیب_دو_Ctx *p_ترتیب_دو,                 /* Sorter context */
  اظہاریہ_List *pEList,               /* Expressions destined for sorter */
  اظہاریہ_List **ppExtra              /* Expressions to append to sorter record */
){
  صحیح i;
  صحیح nDefer = 0;
  اظہاریہ_List *pExtra = 0;
  برائے(i=0; i<pEList->n_اظہاریہ; i++){
    ساخت اظہاریہ_List_شے *p_شے = &pEList->a[i];
    اگر( p_شے->u.x.i_ترتیب_By_ستون==0 ){
      اظہاریہ *p_اظہاریہ = p_شے->p_اظہاریہ;
      جدول *pTab;
      اگر( p_اظہاریہ->op==TK_عمود
       && p_اظہاریہ->i_عمود>=0
       && ALWAYS( اظہاریہ_Use_YTab(p_اظہاریہ) )
       && (pTab = p_اظہاریہ->y.pTab)!=0
       && Is_Ordinary_جدول(pTab)
       && (pTab->a_ستون[p_اظہاریہ->i_عمود].ستون_Flags & COLFLAG_SORTERREF)!=0
      ){
        صحیح j;
        برائے(j=0; j<nDefer; j++){
          اگر( p_ترتیب_دو->aDefer[j].iCsr==p_اظہاریہ->i_جدول ) توڑو;
        }
        اگر( j==nDefer ){
          اگر( nDefer==Array_سائز(p_ترتیب_دو->aDefer) ){
            جاری;
          }ورنہ{
            صحیح n_کلید = 1;
            صحیح k;
            اشاریہ *pPk = 0;
            اگر( !HasRowid(pTab) ){
              pPk = sqlite3_Primary_کلید_اشاریہ(pTab);
              n_کلید = pPk->n_کلید_ستون;
            }
            برائے(k=0; k<n_کلید; k++){
              اظہاریہ *pNew = sqlite3PExpr(pParse, TK_عمود, 0, 0);
              اگر( pNew ){
                pNew->i_جدول = p_اظہاریہ->i_جدول;
                أكد( اظہاریہ_Use_YTab(pNew) );
                pNew->y.pTab = p_اظہاریہ->y.pTab;
                pNew->i_عمود = pPk ? pPk->ai_عمود[k] : -1;
                pExtra = sqlite3_اظہاریہ_List_Append(pParse, pExtra, pNew);
              }
            }
            p_ترتیب_دو->aDefer[nDefer].pTab = p_اظہاریہ->y.pTab;
            p_ترتیب_دو->aDefer[nDefer].iCsr = p_اظہاریہ->i_جدول;
            p_ترتیب_دو->aDefer[nDefer].n_کلید = n_کلید;
            nDefer++;
          }
        }
        p_شے->fg.bSorterRef = 1;
      }
    }
  }
  p_ترتیب_دو->nDefer = (u8)nDefer;
  *ppExtra = pExtra;
}
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DEBUG
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
ساکن صحیح sqlite3CursorBloomable(Parse *pParse, صحیح iCur, صحیح n_ستون){
  صحیح i,k;
  Vdbe *v = pParse->pVdbe;
  اگر( pParse->nErr ) واپس 1;
  أكد( v );
  برائے(k=sqlite3VdbeCurrentAddr(v)-1; k>0; k--){
    ثابت VdbeOp *pOp = sqlite3VdbeGetOp(v, k);
    ثابت کلید_Info *p_کلید_Info;
    اگر( pOp->p1!=iCur ) جاری;
    اگر( pOp->opcode!=OP_OpenRead
     && pOp->opcode!=OP_OpenWrite
     && pOp->opcode!=OP_OpenEphemeral
    ){
      جاری;
    }
    أكد( pOp->p4type==P4_KEYINFO );
    p_کلید_Info = (ثابت کلید_Info*)pOp->p4.p_کلید_Info;
    أكد( p_کلید_Info!=0 );
    برائے(i=0; i<n_ستون; i++){
      أكد( sqlite3_Is_ثنائی(p_کلید_Info->aColl[i]) );
    }
    واپس 1;
  }
  واپس 0;
}
#اگر_ختم /* SQLITE_DEBUG */


/*
** This routine generates the code for the inside of the inner loop
** of a SELECT.
**
** If srcTab is negative, then the p->pEList expressions
** are evaluated in order to get the data for this row.  If srcTab is
** zero or more, then data is pulled from srcTab and p->pEList is used only
** to get the number of columns and the collation sequence for each column.
*/
ساکن خالی selectInnerLoop(
  Parse *pParse,          /* The parser context */
  Select *p,              /* The complete select statement being coded */
  صحیح srcTab,             /* Pull data from this table if non-negative */
  ترتیب_دو_Ctx *p_ترتیب_دو,         /* If not NULL, info on how to process ORDER BY */
  DistinctCtx *pDistinct, /* If not NULL, info on how to process DISTINCT */
  SelectDest *pDest,      /* How to dispose of the results */
  صحیح iContinue,          /* Jump here to continue with next row */
  صحیح iBreak              /* Jump here to break out of the inner loop */
){
  Vdbe *v = pParse->pVdbe;
  صحیح i;
  صحیح hasDistinct;            /* True if the DISTINCT keyword is present */
  صحیح eDest = pDest->eDest;   /* How to dispose of results */
  صحیح iParm = pDest->iSDParm; /* First argument to disposal method */
  صحیح n_نتیجہ_ستون;             /* Number of result columns */
  صحیح n_سابقہ_Reg = 0;         /* Number of extra registers before regResult */
  صف_بھرو_Info s_صف_بھرو_Info;   /* Info for deferred row loading */

  /* Usually, regResult is the first cell in an array of memory cells
  ** containing the current result row. In this case regOrig is set to the
  ** same value. However, if the results are being sent to the sorter, the
  ** values for any expressions that are also part of the sort-key are omitted
  ** from this array. In this case regOrig is set to zero.  */
  صحیح reg_نتیجہ;              /* Start of memory holding current results */
  صحیح regOrig;                /* Start of memory holding full result (or 0) */

  أكد( v );
  أكد( p->pEList!=0 );
  hasDistinct = pDistinct ? pDistinct->eTnctType : WHERE_DISTINCT_NOOP;
  اگر( p_ترتیب_دو && p_ترتیب_دو->p_ترتیب_By==0 ) p_ترتیب_دو = 0;
  اگر( p_ترتیب_دو==0 && !hasDistinct ){
    أكد( iContinue!=0 );
    رمز_انحراف(v, p->i_انحراف, iContinue);
  }

  /* Pull the requested columns.
  */
  n_نتیجہ_ستون = p->pEList->n_اظہاریہ;

  اگر( pDest->iSdst==0 ){
    اگر( p_ترتیب_دو ){
      n_سابقہ_Reg = p_ترتیب_دو->p_ترتیب_By->n_اظہاریہ;
      اگر( !(p_ترتیب_دو->ترتیب_دو_Flags & SORTFLAG_UseSorter) ) n_سابقہ_Reg++;
      pParse->nMem += n_سابقہ_Reg;
    }
    pDest->iSdst = pParse->nMem+1;
    pParse->nMem += n_نتیجہ_ستون;
  }ورنہ اگر( pDest->iSdst+n_نتیجہ_ستون > pParse->nMem ){
    /* This is an error condition that can result, for example, when a SELECT
    ** on the right-hand side of an INSERT contains more result columns than
    ** there are columns in the table on the left.  The error will be caught
    ** and reported later.  But we need to make sure enough memory is allocated
    ** to avoid other spurious errors in the meantime. */
    pParse->nMem += n_نتیجہ_ستون;
  }
  pDest->nSdst = n_نتیجہ_ستون;
  regOrig = reg_نتیجہ = pDest->iSdst;
  اگر( srcTab>=0 ){
    برائے(i=0; i<n_نتیجہ_ستون; i++){
      sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, srcTab, i, reg_نتیجہ+i);
      VdbeComment((v, "%s", p->pEList->a[i].zEName));
    }
  }ورنہ اگر( eDest!=SRT_Exists ){
#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
    اظہاریہ_List *pExtra = 0;
#اگر_ختم
    /* If the destination is an EXISTS(...) expression, the actual
    ** values returned by the SELECT are not required.
    */
    u8 ecelFlags;    /* "ecel" is an abbreviation of "ExprCodeExprList" */
    اظہاریہ_List *pEList;
    اگر( eDest==SRT_Mem || eDest==SRT_Output || eDest==SRT_Coroutine ){
      ecelFlags = SQLITE_ECEL_DUP;
    }ورنہ{
      ecelFlags = 0;
    }
    اگر( p_ترتیب_دو && hasDistinct==0 && eDest!=SRT_EphemTab && eDest!=SRT_جدول ){
      /* For each expression in p->pEList that is a copy of an expression in
      ** the ORDER BY clause (pSort->pOrderBy), set the associated
      ** iOrderByCol value to one more than the index of the ORDER BY
      ** expression within the sort-key that pushOntoSorter() will generate.
      ** This allows the p->pEList field to be omitted from the sorted record,
      ** saving space and CPU cycles.  */
      ecelFlags |= (SQLITE_ECEL_OMITREF|SQLITE_ECEL_REF);

      برائے(i=p_ترتیب_دو->nOBSat; i<p_ترتیب_دو->p_ترتیب_By->n_اظہاریہ; i++){
        صحیح j;
        اگر( (j = p_ترتیب_دو->p_ترتیب_By->a[i].u.x.i_ترتیب_By_ستون)>0 ){
          p->pEList->a[j-1].u.x.i_ترتیب_By_ستون = i+1-p_ترتیب_دو->nOBSat;
        }
      }
#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
      select_اظہاریہ_Defer(pParse, p_ترتیب_دو, p->pEList, &pExtra);
      اگر( pExtra && pParse->db->mallocFailed==0 ){
        /* If there are any extra PK columns to add to the sorter records,
        ** allocate extra memory cells and adjust the OpenEphemeral
        ** instruction to account for the larger records. This is only
        ** required if there are one or more WITHOUT ROWID tables with
        ** composite primary keys in the SortCtx.aDefer[] array.  */
        VdbeOp *pOp = sqlite3VdbeGetOp(v, p_ترتیب_دو->addr_ترتیب_دو_اشاریہ);
        pOp->p2 += (pExtra->n_اظہاریہ - p_ترتیب_دو->nDefer);
        pOp->p4.p_کلید_Info->n_All_میدان += (pExtra->n_اظہاریہ - p_ترتیب_دو->nDefer);
        pParse->nMem += pExtra->n_اظہاریہ;
      }
#اگر_ختم

      /* Adjust nResultCol to account for columns that are omitted
      ** from the sorter by the optimizations in this branch */
      pEList = p->pEList;
      برائے(i=0; i<pEList->n_اظہاریہ; i++){
        اگر( pEList->a[i].u.x.i_ترتیب_By_ستون>0
#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
         || pEList->a[i].fg.bSorterRef
#اگر_ختم
        ){
          n_نتیجہ_ستون--;
          regOrig = 0;
        }
      }

      testcase( regOrig );
      testcase( eDest==SRT_سیٹ );
      testcase( eDest==SRT_Mem );
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      أكد( eDest==SRT_سیٹ || eDest==SRT_Mem
           || eDest==SRT_Coroutine || eDest==SRT_Output
           || eDest==SRT_Upfrom );
    }
    s_صف_بھرو_Info.reg_نتیجہ = reg_نتیجہ;
    s_صف_بھرو_Info.ecelFlags = ecelFlags;
#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
    s_صف_بھرو_Info.pExtra = pExtra;
    s_صف_بھرو_Info.reg_Extra_نتیجہ = reg_نتیجہ + n_نتیجہ_ستون;
    اگر( pExtra ) n_نتیجہ_ستون += pExtra->n_اظہاریہ;
#اگر_ختم
    اگر( p->i_حد_بندی
     && (ecelFlags & SQLITE_ECEL_OMITREF)!=0
     && n_سابقہ_Reg>0
    ){
      أكد( p_ترتیب_دو!=0 );
      أكد( hasDistinct==0 );
      p_ترتیب_دو->p_Deferred_صف_بھرو = &s_صف_بھرو_Info;
      regOrig = 0;
    }ورنہ{
      inner_Loop_بھرو_صف(pParse, p, &s_صف_بھرو_Info);
    }
  }

  /* If the DISTINCT keyword was present on the SELECT statement
  ** and this row has been seen before, then do not make this row
  ** part of the result.
  */
  اگر( hasDistinct ){
    صحیح eType = pDistinct->eTnctType;
    صحیح iTab = pDistinct->tabTnct;
    أكد( n_نتیجہ_ستون==p->pEList->n_اظہاریہ );
    iTab = رمز_Distinct(pParse, eType, iTab, iContinue, p->pEList, reg_نتیجہ);
    fixDistinctOpenEph(pParse, eType, iTab, pDistinct->addrTnct);
    اگر( p_ترتیب_دو==0 ){
      رمز_انحراف(v, p->i_انحراف, iContinue);
    }
  }

  انتخاب( eDest ){
    /* Store the result as data using a unique key.
    */
    حالت SRT_Fifo:
    حالت SRT_DistFifo:
    حالت SRT_جدول:
    حالت SRT_EphemTab: {
      صحیح r1 = sqlite3_Get_عارضی_Range(pParse, n_سابقہ_Reg+1);
      testcase( eDest==SRT_جدول );
      testcase( eDest==SRT_EphemTab );
      testcase( eDest==SRT_Fifo );
      testcase( eDest==SRT_DistFifo );
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, reg_نتیجہ, n_نتیجہ_ستون, r1+n_سابقہ_Reg);
#اگر !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      اگر( eDest==SRT_جدول && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CTE
      اگر( eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index. If the current row is already present
        ** in the index, do not write it to the output. If not, add the
        ** current row to the index and proceed with writing it to the
        ** output table as well.  */
        صحیح addr = sqlite3VdbeCurrentAddr(v) + 4;
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_ملا, iParm+1, addr, r1, 0);
        VdbeCoverage(v);
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iParm+1, r1,reg_نتیجہ,n_نتیجہ_ستون);
        أكد( p_ترتیب_دو==0 );
      }
#اگر_ختم
      اگر( p_ترتیب_دو ){
        أكد( reg_نتیجہ==regOrig );
        ڈالو_Onto_Sorter(pParse, p_ترتیب_دو, p, r1+n_سابقہ_Reg, regOrig, 1, n_سابقہ_Reg);
      }ورنہ{
        صحیح r2 = sqlite3_Get_عارضی_Reg(pParse);
        sqlite3_Vdbe_جوڑو_Op2(v, OP_NewRowid, iParm, r2);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, iParm, r1, r2);
        sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
        sqlite3_Release_عارضی_Reg(pParse, r2);
      }
      sqlite3_Release_عارضی_Range(pParse, r1, n_سابقہ_Reg+1);
      توڑو;
    }

    حالت SRT_Upfrom: {
      اگر( p_ترتیب_دو ){
        ڈالو_Onto_Sorter(
            pParse, p_ترتیب_دو, p, reg_نتیجہ, regOrig, n_نتیجہ_ستون, n_سابقہ_Reg);
      }ورنہ{
        صحیح i2 = pDest->iSDParm2;
        صحیح r1 = sqlite3_Get_عارضی_Reg(pParse);

        /* If the UPDATE FROM join is an aggregate that matches no rows, it
        ** might still be trying to return one row, because that is what
        ** aggregates do.  Don't record that empty row in the output table. */
        sqlite3_Vdbe_جوڑو_Op2(v, OP_IsNull, reg_نتیجہ, iBreak); VdbeCoverage(v);

        sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ,
                          reg_نتیجہ+(i2<0), n_نتیجہ_ستون-(i2<0), r1);
        اگر( i2<0 ){
          sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, iParm, r1, reg_نتیجہ);
        }ورنہ{
          sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iParm, r1, reg_نتیجہ, i2);
        }
      }
      توڑو;
    }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)" construct,
    ** then there should be a single item on the stack.  Write this
    ** item into the set table with bogus data.
    */
    حالت SRT_سیٹ: {
      اگر( p_ترتیب_دو ){
        /* At first glance you would think we could optimize out the
        ** ORDER BY in this case since the order of entries in the set
        ** does not matter.  But there might be a LIMIT clause, in which
        ** case the order does matter */
        ڈالو_Onto_Sorter(
            pParse, p_ترتیب_دو, p, reg_نتیجہ, regOrig, n_نتیجہ_ستون, n_سابقہ_Reg);
        pDest->iSDParm2 = 0; /* Signal that any Bloom filter is unpopulated */
      }ورنہ{
        صحیح r1 = sqlite3_Get_عارضی_Reg(pParse);
        أكد( sqlite3Strlen30(pDest->zAffSdst)==n_نتیجہ_ستون );
        sqlite3_Vdbe_جوڑو_Op4(v, OP_Make_ریکارڈ, reg_نتیجہ, n_نتیجہ_ستون,
            r1, pDest->zAffSdst, n_نتیجہ_ستون);
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iParm, r1, reg_نتیجہ, n_نتیجہ_ستون);
        اگر( pDest->iSDParm2 ){
          أكد( sqlite3CursorBloomable(pParse,iParm,n_نتیجہ_ستون) );
          sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Filter_جوڑو, pDest->iSDParm2, 0,
                               reg_نتیجہ, n_نتیجہ_ستون);
          ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
        }
        sqlite3_Release_عارضی_Reg(pParse, r1);
      }
      توڑو;
    }


    /* If any row exist in the result set, record that fact and abort.
    */
    حالت SRT_Exists: {
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 1, iParm);
      /* The LIMIT clause will terminate the loop for us */
      توڑو;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell or array of
    ** memory cells and break out of the scan loop.
    */
    حالت SRT_Mem: {
      اگر( p_ترتیب_دو ){
        أكد( n_نتیجہ_ستون<=pDest->nSdst );
        ڈالو_Onto_Sorter(
            pParse, p_ترتیب_دو, p, reg_نتیجہ, regOrig, n_نتیجہ_ستون, n_سابقہ_Reg);
        pDest->iSDParm = reg_نتیجہ;
      }ورنہ{
        أكد( n_نتیجہ_ستون==pDest->nSdst );
        اگر( reg_نتیجہ!=iParm ){
          /* This occurs in cases where the SELECT had both a DISTINCT and
          ** an OFFSET clause.  */
          sqlite3_Vdbe_جوڑو_Op3(v, OP_Copy, reg_نتیجہ, iParm, n_نتیجہ_ستون-1);
        }
        /* The LIMIT clause will jump out of the loop for us */
      }
      توڑو;
    }
#اگر_ختم /* #ifndef SQLITE_OMIT_SUBQUERY */

    حالت SRT_Coroutine:       /* Send data to a co-routine */
    حالت SRT_Output: {        /* Return the results */
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      اگر( p_ترتیب_دو ){
        ڈالو_Onto_Sorter(pParse, p_ترتیب_دو, p, reg_نتیجہ, regOrig, n_نتیجہ_ستون,
                       n_سابقہ_Reg);
      }ورنہ اگر( eDest==SRT_Coroutine ){
        sqlite3_Vdbe_جوڑو_Op1(v, OP_Yield, pDest->iSDParm);
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op2(v, OP_نتیجہ_صف, reg_نتیجہ, n_نتیجہ_ستون);
      }
      توڑو;
    }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    حالت SRT_Dist_قطار:
    حالت SRT_قطار: {
      صحیح n_کلید;
      صحیح r1, r2, r3;
      صحیح addrTest = 0;
      اظہاریہ_List *pSO;
      pSO = pDest->p_ترتیب_By;
      أكد( pSO );
      n_کلید = pSO->n_اظہاریہ;
      r1 = sqlite3_Get_عارضی_Reg(pParse);
      r2 = sqlite3_Get_عارضی_Range(pParse, n_کلید+2);
      r3 = r2+n_کلید+1;
      اگر( eDest==SRT_Dist_قطار ){
        /* If the destination is DistQueue, then cursor (iParm+1) is open
        ** on a second ephemeral index that holds all values every previously
        ** added to the queue. */
        addrTest = sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_ملا, iParm+1, 0,
                                        reg_نتیجہ, n_نتیجہ_ستون);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, reg_نتیجہ, n_نتیجہ_ستون, r3);
      اگر( eDest==SRT_Dist_قطار ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Idx_داخل_کرو, iParm+1, r3);
        sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      }
      برائے(i=0; i<n_کلید; i++){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_SCopy,
                          reg_نتیجہ + pSO->a[i].u.x.i_ترتیب_By_ستون - 1,
                          r2+i);
      }
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Sequence, iParm, r2+n_کلید);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, r2, n_کلید+2, r1);
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iParm, r1, r2, n_کلید+2);
      اگر( addrTest ) sqlite3_Vdbe_Jump_یہاں(v, addrTest);
      sqlite3_Release_عارضی_Reg(pParse, r1);
      sqlite3_Release_عارضی_Range(pParse, r2, n_کلید+2);
      توڑو;
    }
#اگر_ختم /* SQLITE_OMIT_CTE */



#اگر !defined(SQLITE_OMIT_TRIGGER)
    /* Discard the results.  This is used for SELECT statements inside
    ** the body of a TRIGGER.  The purpose of such selects is to call
    ** user-defined functions that have side effects.  We do not care
    ** about the actual results of the select.
    */
    طےشدہ: {
      أكد( eDest==SRT_Discard );
      توڑو;
    }
#اگر_ختم
  }

  /* Jump to the end of the loop if the LIMIT is reached.  Except, if
  ** there is a sorter, in which case the sorter has already limited
  ** the output for us.
  */
  اگر( p_ترتیب_دو==0 && p->i_حد_بندی ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_DecrJumpZero, p->i_حد_بندی, iBreak); VdbeCoverage(v);
  }
}

/*
** Allocate a KeyInfo object sufficient for an index of N key columns and
** X extra columns.
*/
کلید_Info *sqlite3_کلید_Info_Alloc(sqlite3 *db, صحیح N, صحیح X){
  صحیح nExtra = (N+X)*(حجم(CollSeq*)+1);
  کلید_Info *p;
  أكد( X>=0 );
  اگر( NEVER(N+X>0xffff) ) واپس (کلید_Info*)sqlite3OomFault(db);
  p = sqlite3DbMallocRawNN(db, SZ_KEYINFO(0) + nExtra);
  اگر( p ){
    p->a_ترتیب_دو_Flags = (u8*)&p->aColl[N+X];
    p->n_کلید_میدان = (u16)N;
    p->n_All_میدان = (u16)(N+X);
    p->enc = ENC(db);
    p->db = db;
    p->nRef = 1;
    memset(p->aColl, 0, nExtra);
  }ورنہ{
    واپس (کلید_Info*)sqlite3OomFault(db);
  }
  واپس p;
}

/*
** Deallocate a KeyInfo object
*/
خالی sqlite3_کلید_Info_Unref(کلید_Info *p){
  اگر( p ){
    أكد( p->db!=0 );
    أكد( p->nRef>0 );
    p->nRef--;
    اگر( p->nRef==0 ) sqlite3DbNNFreeNN(p->db, p);
  }
}

/*
** Make a new pointer to a KeyInfo object
*/
کلید_Info *sqlite3_کلید_Info_Ref(کلید_Info *p){
  اگر( p ){
    أكد( p->nRef>0 );
    p->nRef++;
  }
  واپس p;
}

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** Return TRUE if a KeyInfo object can be change.  The KeyInfo object
** can only be changed if this is just a single reference to the object.
**
** This routine is used only inside of assert() statements.
*/
صحیح sqlite3_کلید_Info_Is_Writeable(کلید_Info *p){ واپس p->nRef==1; }
#اگر_ختم /* SQLITE_DEBUG */

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
کلید_Info *sqlite3_کلید_Info_From_اظہاریہ_List(
  Parse *pParse,       /* Parsing context */
  اظہاریہ_List *pList,     /* Form the KeyInfo object from this ExprList */
  صحیح i_آغاز,          /* Begin with this column of pList */
  صحیح nExtra           /* Add this many extra columns to the end */
){
  صحیح n_اظہاریہ;
  کلید_Info *pInfo;
  ساخت اظہاریہ_List_شے *p_شے;
  sqlite3 *db = pParse->db;
  صحیح i;

  n_اظہاریہ = pList->n_اظہاریہ;
  pInfo = sqlite3_کلید_Info_Alloc(db, n_اظہاریہ-i_آغاز, nExtra+1);
  اگر( pInfo ){
    أكد( sqlite3_کلید_Info_Is_Writeable(pInfo) );
    برائے(i=i_آغاز, p_شے=pList->a+i_آغاز; i<n_اظہاریہ; i++, p_شے++){
      pInfo->aColl[i-i_آغاز] = sqlite3_اظہاریہ_NNColl_Seq(pParse, p_شے->p_اظہاریہ);
      pInfo->a_ترتیب_دو_Flags[i-i_آغاز] = p_شے->fg.ترتیب_دو_Flags;
    }
  }
  واپس pInfo;
}

/*
** Name of the connection operator, used for error messages.
*/
ثابت حرف *sqlite3SelectOpName(صحیح id){
  حرف *z;
  انتخاب( id ){
    حالت TK_ALL:       z = "UNION ALL";   توڑو;
    حالت TK_INTERSECT: z = "INTERSECT";   توڑو;
    حالت TK_EXCEPT:    z = "EXCEPT";      توڑو;
    طےشدہ:           z = "UNION";       توڑو;
  }
  واپس z;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_EXPLAIN
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
ساکن خالی explain_عارضی_جدول(Parse *pParse, ثابت حرف *zUsage){
  ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s", zUsage));
}

/*
** Assign expression b to lvalue a. A second, no-op, version of this macro
** is provided when SQLITE_OMIT_EXPLAIN is defined. This allows the code
** in sqlite3Select() to assign values to structure member variables that
** only exist if SQLITE_OMIT_EXPLAIN is not defined without polluting the
** code with #ifndef directives.
*/
# define explain_سیٹ_Integer(a, b) a = b

#ورنہ
/* No-op versions of the explainXXX() functions and macros. */
# define explain_عارضی_جدول(y,z)
# define explain_سیٹ_Integer(y,z)
#اگر_ختم


/*
** If the inner loop was generated using a non-null pOrderBy argument,
** then the results were placed in a sorter.  After the loop is terminated
** we need to run the sorter and output the results.  The following
** routine generates the code needed to do that.
*/
ساکن خالی generate_ترتیب_دو_دم(
  Parse *pParse,    /* Parsing context */
  Select *p,        /* The SELECT statement */
  ترتیب_دو_Ctx *p_ترتیب_دو,   /* Information on the ORDER BY clause */
  صحیح n_عمود,      /* Number of columns of data */
  SelectDest *pDest /* Write the sorted results here */
){
  Vdbe *v = pParse->pVdbe;                     /* The prepared statement */
  صحیح addrBreak = p_ترتیب_دو->label_مکمل;            /* Jump here to exit loop */
  صحیح addrContinue = sqlite3VdbeMakeLabel(pParse);/* Jump here for next cycle */
  صحیح addr;                       /* Top of output loop. Jump for Next. */
  صحیح addrOnce = 0;
  صحیح iTab;
  اظہاریہ_List *p_ترتیب_By = p_ترتیب_دو->p_ترتیب_By;
  صحیح eDest = pDest->eDest;
  صحیح iParm = pDest->iSDParm;
  صحیح reg_صف;
  صحیح regRowid;
  صحیح i_ستون;
  صحیح n_کلید;                       /* Number of key columns in sorter record */
  صحیح i_ترتیب_دو_Tab;                   /* Sorter cursor to read from */
  صحیح i;
  صحیح bSeq;                       /* True if sorter record includes seq. no. */
  صحیح n_Ref_کلید = 0;
  ساخت اظہاریہ_List_شے *a_برآمد_Ex = p->pEList->a;
#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
  صحیح addrExplain;                /* Address of OP_Explain instruction */
#اگر_ختم

  n_کلید = p_ترتیب_By->n_اظہاریہ - p_ترتیب_دو->nOBSat;
  اگر( p_ترتیب_دو->nOBSat==0 || n_کلید==1 ){
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR %sORDER BY", p_ترتیب_دو->nOBSat?"LAST TERM OF ":""
    ));
  }ورنہ{
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR LAST %d TERMS OF ORDER BY", n_کلید
    ));
  }
  sqlite3_Vdbe_جائزہ_حالت_Range(v, addrExplain,p_ترتیب_دو->addr_ڈالو,p_ترتیب_دو->addr_ڈالو_انتہا);
  sqlite3_Vdbe_جائزہ_حالت_Counters(v, addrExplain, addrExplain, p_ترتیب_دو->addr_ڈالو);


  أكد( addrBreak<0 );
  اگر( p_ترتیب_دو->label_Bk_برآمد ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, p_ترتیب_دو->regReturn, p_ترتیب_دو->label_Bk_برآمد);
    sqlite3VdbeGoto(v, addrBreak);
    sqlite3VdbeResolveLabel(v, p_ترتیب_دو->label_Bk_برآمد);
  }

#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
  /* Open any cursors needed for sorter-reference expressions */
  برائے(i=0; i<p_ترتیب_دو->nDefer; i++){
    جدول *pTab = p_ترتیب_دو->aDefer[i].pTab;
    صحیح iDb = sqlite3_Schema_مقصد_اشاریہ(pParse->db, pTab->pSchema);
    sqlite3_Open_جدول(pParse, p_ترتیب_دو->aDefer[i].iCsr, iDb, pTab, OP_OpenRead);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Rewind, p_ترتیب_دو->aDefer[i].iCsr,
                      sqlite3VdbeCurrentAddr(v)+1);
    VdbeCoverage(v);
    n_Ref_کلید = MAX(n_Ref_کلید, p_ترتیب_دو->aDefer[i].n_کلید);
  }
#اگر_ختم

  iTab = p_ترتیب_دو->iECursor;
  اگر( eDest==SRT_Output || eDest==SRT_Coroutine || eDest==SRT_Mem ){
    اگر( eDest==SRT_Mem && p->i_انحراف ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Null, 0, pDest->iSdst);
    }
    regRowid = 0;
    reg_صف = pDest->iSdst;
  }ورنہ{
    regRowid = sqlite3_Get_عارضی_Reg(pParse);
    اگر( eDest==SRT_EphemTab || eDest==SRT_جدول ){
      reg_صف = sqlite3_Get_عارضی_Reg(pParse);
      n_عمود = 0;
    }ورنہ{
      reg_صف = sqlite3_Get_عارضی_Range(pParse, n_عمود);
    }
  }
  اگر( p_ترتیب_دو->ترتیب_دو_Flags & SORTFLAG_UseSorter ){
    صحیح reg_ترتیب_دو_برآمد = ++pParse->nMem;
    i_ترتیب_دو_Tab = pParse->nTab++;
    اگر( p_ترتیب_دو->label_Bk_برآمد ){
      addrOnce = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);
    }
    sqlite3_Vdbe_جوڑو_Op3(v, OP_OpenPseudo, i_ترتیب_دو_Tab, reg_ترتیب_دو_برآمد,
        n_کلید+1+n_عمود+n_Ref_کلید);
    اگر( addrOnce ) sqlite3_Vdbe_Jump_یہاں(v, addrOnce);
    addr = 1 + sqlite3_Vdbe_جوڑو_Op2(v, OP_Sorter_ترتیب_دو, iTab, addrBreak);
    VdbeCoverage(v);
    أكد( p->i_حد_بندی==0 && p->i_انحراف==0 );
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Sorter_ڈیٹا, iTab, reg_ترتیب_دو_برآمد, i_ترتیب_دو_Tab);
    bSeq = 0;
  }ورنہ{
    addr = 1 + sqlite3_Vdbe_جوڑو_Op2(v, OP_ترتیب_دو, iTab, addrBreak); VdbeCoverage(v);
    رمز_انحراف(v, p->i_انحراف, addrContinue);
    i_ترتیب_دو_Tab = iTab;
    bSeq = 1;
    اگر( p->i_انحراف>0 ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_جوڑو_Imm, p->i_حد_بندی, -1);
    }
  }
  برائے(i=0, i_ستون=n_کلید+bSeq-1; i<n_عمود; i++){
#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
    اگر( a_برآمد_Ex[i].fg.bSorterRef ) جاری;
#اگر_ختم
    اگر( a_برآمد_Ex[i].u.x.i_ترتیب_By_ستون==0 ) i_ستون++;
  }
#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
  اگر( p_ترتیب_دو->nDefer ){
    صحیح i_کلید = i_ستون+1;
    صحیح reg_کلید = sqlite3_Get_عارضی_Range(pParse, n_Ref_کلید);

    برائے(i=0; i<p_ترتیب_دو->nDefer; i++){
      صحیح iCsr = p_ترتیب_دو->aDefer[i].iCsr;
      جدول *pTab = p_ترتیب_دو->aDefer[i].pTab;
      صحیح n_کلید = p_ترتیب_دو->aDefer[i].n_کلید;

      sqlite3_Vdbe_جوڑو_Op1(v, OP_Null_صف, iCsr);
      اگر( HasRowid(pTab) ){
        sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, i_ترتیب_دو_Tab, i_کلید++, reg_کلید);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_SeekRowid, iCsr,
            sqlite3VdbeCurrentAddr(v)+1, reg_کلید);
      }ورنہ{
        صحیح k;
        صحیح iJmp;
        أكد( sqlite3_Primary_کلید_اشاریہ(pTab)->n_کلید_ستون==n_کلید );
        برائے(k=0; k<n_کلید; k++){
          sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, i_ترتیب_دو_Tab, i_کلید++, reg_کلید+k);
        }
        iJmp = sqlite3VdbeCurrentAddr(v);
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_SeekGE, iCsr, iJmp+2, reg_کلید, n_کلید);
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_IdxLE, iCsr, iJmp+3, reg_کلید, n_کلید);
        sqlite3_Vdbe_جوڑو_Op1(v, OP_Null_صف, iCsr);
      }
    }
    sqlite3_Release_عارضی_Range(pParse, reg_کلید, n_Ref_کلید);
  }
#اگر_ختم
  برائے(i=n_عمود-1; i>=0; i--){
#اگر_تعریف_شدہ SQLITE_ENABLE_SORTER_REFERENCES
    اگر( a_برآمد_Ex[i].fg.bSorterRef ){
      sqlite3_اظہاریہ_رمز(pParse, a_برآمد_Ex[i].p_اظہاریہ, reg_صف+i);
    }ورنہ
#اگر_ختم
    {
      صحیح iRead;
      اگر( a_برآمد_Ex[i].u.x.i_ترتیب_By_ستون ){
        iRead = a_برآمد_Ex[i].u.x.i_ترتیب_By_ستون-1;
      }ورنہ{
        iRead = i_ستون--;
      }
      sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, i_ترتیب_دو_Tab, iRead, reg_صف+i);
      VdbeComment((v, "%s", a_برآمد_Ex[i].zEName));
    }
  }
  sqlite3_Vdbe_جائزہ_حالت_Range(v, addrExplain, addrExplain, -1);
  انتخاب( eDest ){
    حالت SRT_جدول:
    حالت SRT_EphemTab: {
      sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, i_ترتیب_دو_Tab, n_کلید+bSeq, reg_صف);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_NewRowid, iParm, regRowid);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, iParm, reg_صف, regRowid);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      توڑو;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
    حالت SRT_سیٹ: {
      أكد( n_عمود==sqlite3Strlen30(pDest->zAffSdst) );
      sqlite3_Vdbe_جوڑو_Op4(v, OP_Make_ریکارڈ, reg_صف, n_عمود, regRowid,
                        pDest->zAffSdst, n_عمود);
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iParm, regRowid, reg_صف, n_عمود);
      توڑو;
    }
    حالت SRT_Mem: {
      /* The LIMIT clause will terminate the loop for us */
      توڑو;
    }
#اگر_ختم
    حالت SRT_Upfrom: {
      صحیح i2 = pDest->iSDParm2;
      صحیح r1 = sqlite3_Get_عارضی_Reg(pParse);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ,reg_صف+(i2<0),n_عمود-(i2<0),r1);
      اگر( i2<0 ){
        sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, iParm, r1, reg_صف);
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iParm, r1, reg_صف, i2);
      }
      توڑو;
    }
    طےشدہ: {
      أكد( eDest==SRT_Output || eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      testcase( eDest==SRT_Coroutine );
      اگر( eDest==SRT_Output ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_نتیجہ_صف, pDest->iSdst, n_عمود);
      }ورنہ{
        sqlite3_Vdbe_جوڑو_Op1(v, OP_Yield, pDest->iSDParm);
      }
      توڑو;
    }
  }
  اگر( regRowid ){
    اگر( eDest==SRT_سیٹ ){
      sqlite3_Release_عارضی_Range(pParse, reg_صف, n_عمود);
    }ورنہ{
      sqlite3_Release_عارضی_Reg(pParse, reg_صف);
    }
    sqlite3_Release_عارضی_Reg(pParse, regRowid);
  }
  /* The bottom of the loop
  */
  sqlite3VdbeResolveLabel(v, addrContinue);
  اگر( p_ترتیب_دو->ترتیب_دو_Flags & SORTFLAG_UseSorter ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Sorter_اگلا, iTab, addr); VdbeCoverage(v);
  }ورنہ{
    sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, iTab, addr); VdbeCoverage(v);
  }
  sqlite3_Vdbe_جائزہ_حالت_Range(v, addrExplain, sqlite3VdbeCurrentAddr(v)-1, -1);
  اگر( p_ترتیب_دو->regReturn ) sqlite3_Vdbe_جوڑو_Op1(v, OP_Return, p_ترتیب_دو->regReturn);
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
#اگر_تعریف_شدہ SQLITE_ENABLE_عمود_METADATA
# define عمود_Type(A,B,C,D,E) عمود_Type_Impl(A,B,C,D,E)
#ورنہ /* if !defined(SQLITE_ENABLE_COLUMN_METADATA) */
# define عمود_Type(A,B,C,D,E) عمود_Type_Impl(A,B)
#اگر_ختم
ساکن ثابت حرف *عمود_Type_Impl(
  NameContext *pNC,
#اگر_غیر_تعریف_شدہ SQLITE_ENABLE_عمود_METADATA
  اظہاریہ *p_اظہاریہ
#ورنہ
  اظہاریہ *p_اظہاریہ,
  ثابت حرف **pzOrigDb,
  ثابت حرف **pzOrigTab,
  ثابت حرف **pz_Orig_ستون
#اگر_ختم
){
  حرف ثابت *zType = 0;
  صحیح j;
#اگر_تعریف_شدہ SQLITE_ENABLE_عمود_METADATA
  حرف ثابت *zOrigDb = 0;
  حرف ثابت *zOrigTab = 0;
  حرف ثابت *z_Orig_ستون = 0;
#اگر_ختم

  أكد( p_اظہاریہ!=0 );
  أكد( pNC->pSrcList!=0 );
  انتخاب( p_اظہاریہ->op ){
    حالت TK_عمود: {
      /* The expression is a column. Locate the table the column is being
      ** extracted from in NameContext.pSrcList. This table may be real
      ** database table or a subquery.
      */
      جدول *pTab = 0;            /* Table structure column is extracted from */
      Select *pS = 0;             /* Select the column is extracted from */
      صحیح i_ستون = p_اظہاریہ->i_عمود;  /* Index of column in pTab */
      جب( pNC && !pTab ){
        SrcList *pTabList = pNC->pSrcList;
        برائے(j=0;j<pTabList->nSrc && pTabList->a[j].iCursor!=p_اظہاریہ->i_جدول;j++);
        اگر( j<pTabList->nSrc ){
          pTab = pTabList->a[j].pSTab;
          اگر( pTabList->a[j].fg.isSubquery ){
            pS = pTabList->a[j].u4.pSubq->pSelect;
          }ورنہ{
            pS = 0;
          }
        }ورنہ{
          pNC = pNC->p_اگلا;
        }
      }

      اگر( pTab==0 ){
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
        توڑو;
      }

      أكد( pTab && اظہاریہ_Use_YTab(p_اظہاریہ) && p_اظہاریہ->y.pTab==pTab );
      اگر( pS ){
        /* The "table" is actually a sub-select or a view in the FROM clause
        ** of the SELECT statement. Return the declaration type and origin
        ** data for the result-set column of the sub-select.
        */
        اگر( i_ستون<pS->pEList->n_اظہاریہ
         && (!ViewCanHaveRowid || i_ستون>=0)
        ){
          /* If iCol is less than zero, then the expression requests the
          ** rowid of the sub-select or view. This expression is legal (see
          ** test case misc2.2.2) - it always evaluates to NULL.
          */
          NameContext sNC;
          اظہاریہ *p = pS->pEList->a[i_ستون].p_اظہاریہ;
          sNC.pSrcList = pS->pSrc;
          sNC.p_اگلا = pNC;
          sNC.pParse = pNC->pParse;
          zType = عمود_Type(&sNC, p,&zOrigDb,&zOrigTab,&z_Orig_ستون);
        }
      }ورنہ{
        /* A real table or a CTE table */
        أكد( !pS );
#اگر_تعریف_شدہ SQLITE_ENABLE_عمود_METADATA
        اگر( i_ستون<0 ) i_ستون = pTab->iPKey;
        أكد( i_ستون==XN_ROWID || (i_ستون>=0 && i_ستون<pTab->n_ستون) );
        اگر( i_ستون<0 ){
          zType = "INTEGER";
          z_Orig_ستون = "rowid";
        }ورنہ{
          z_Orig_ستون = pTab->a_ستون[i_ستون].zCnName;
          zType = sqlite3_عمود_Type(&pTab->a_ستون[i_ستون],0);
        }
        zOrigTab = pTab->zName;
        اگر( pNC->pParse && pTab->pSchema ){
          صحیح iDb = sqlite3_Schema_مقصد_اشاریہ(pNC->pParse->db, pTab->pSchema);
          zOrigDb = pNC->pParse->db->aDb[iDb].zDbSName;
        }
#ورنہ
        أكد( i_ستون==XN_ROWID || (i_ستون>=0 && i_ستون<pTab->n_ستون) );
        اگر( i_ستون<0 ){
          zType = "INTEGER";
        }ورنہ{
          zType = sqlite3_عمود_Type(&pTab->a_ستون[i_ستون],0);
        }
#اگر_ختم
      }
      توڑو;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
    حالت TK_SELECT: {
      /* The expression is a sub-select. Return the declaration type and
      ** origin info for the single column in the result set of the SELECT
      ** statement.
      */
      NameContext sNC;
      Select *pS;
      اظہاریہ *p;
      أكد( اظہاریہ_Use_XSelect(p_اظہاریہ) );
      pS = p_اظہاریہ->x.pSelect;
      p = pS->pEList->a[0].p_اظہاریہ;
      sNC.pSrcList = pS->pSrc;
      sNC.p_اگلا = pNC;
      sNC.pParse = pNC->pParse;
      zType = عمود_Type(&sNC, p, &zOrigDb, &zOrigTab, &z_Orig_ستون);
      توڑو;
    }
#اگر_ختم
  }

#اگر_تعریف_شدہ SQLITE_ENABLE_عمود_METADATA 
  اگر( pzOrigDb ){
    أكد( pzOrigTab && pz_Orig_ستون );
    *pzOrigDb = zOrigDb;
    *pzOrigTab = zOrigTab;
    *pz_Orig_ستون = z_Orig_ستون;
  }
#اگر_ختم
  واپس zType;
}

/*
** Generate code that will tell the VDBE the declaration types of columns
** in the result set.
*/
ساکن خالی generate_عمود_Types(
  Parse *pParse,      /* Parser context */
  SrcList *pTabList,  /* List of tables */
  اظہاریہ_List *pEList    /* Expressions defining the result set */
){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DECLTYPE
  Vdbe *v = pParse->pVdbe;
  صحیح i;
  NameContext sNC;
  sNC.pSrcList = pTabList;
  sNC.pParse = pParse;
  sNC.p_اگلا = 0;
  برائے(i=0; i<pEList->n_اظہاریہ; i++){
    اظہاریہ *p = pEList->a[i].p_اظہاریہ;
    ثابت حرف *zType;
#اگر_تعریف_شدہ SQLITE_ENABLE_عمود_METADATA
    ثابت حرف *zOrigDb = 0;
    ثابت حرف *zOrigTab = 0;
    ثابت حرف *z_Orig_ستون = 0;
    zType = عمود_Type(&sNC, p, &zOrigDb, &zOrigTab, &z_Orig_ستون);

    /* The vdbe must make its own copy of the column-type and other
    ** column specific strings, in case the schema is reset before this
    ** virtual machine is deleted.
    */
    sqlite3_Vdbe_سیٹ_ستون_Name(v, i, COLNAME_DATABASE, zOrigDb, SQLITE_TRANSIENT);
    sqlite3_Vdbe_سیٹ_ستون_Name(v, i, COLNAME_جدول, zOrigTab, SQLITE_TRANSIENT);
    sqlite3_Vdbe_سیٹ_ستون_Name(v, i, COLNAME_عمود, z_Orig_ستون, SQLITE_TRANSIENT);
#ورنہ
    zType = عمود_Type(&sNC, p, 0, 0, 0);
#اگر_ختم
    sqlite3_Vdbe_سیٹ_ستون_Name(v, i, COLNAME_DECLTYPE, zType, SQLITE_TRANSIENT);
  }
#ورنہ
  غیر_مستعمل_PARAMETER(pParse);
  غیر_مستعمل_PARAMETER(pTabList);
  غیر_مستعمل_PARAMETER(pEList);
#اگر_ختم /* !defined(SQLITE_OMIT_DECLTYPE) */
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
خالی sqlite3_Generate_عمود_نام(
  Parse *pParse,      /* Parser context */
  Select *pSelect     /* Generate column names for this SELECT statement */
){
  Vdbe *v = pParse->pVdbe;
  صحیح i;
  جدول *pTab;
  SrcList *pTabList;
  اظہاریہ_List *pEList;
  sqlite3 *db = pParse->db;
  صحیح بھرا_Name;    /* TABLE.COLUMN if no AS clause and is a direct table ref */
  صحیح srcName;     /* COLUMN or TABLE.COLUMN if no AS clause and is direct */

  اگر( pParse->ستون_نام_سیٹ ) واپس;
  /* Column names are determined by the left-most term of a compound select */
  جب( pSelect->p_پیشگی ) pSelect = pSelect->p_پیشگی;
  TREETRACE(0x80,pParse,pSelect,("generating column names\n"));
  pTabList = pSelect->pSrc;
  pEList = pSelect->pEList;
  أكد( v!=0 );
  أكد( pTabList!=0 );
  pParse->ستون_نام_سیٹ = 1;
  بھرا_Name = (db->flags & SQLITE_بھرا_ستون_نام)!=0;
  srcName = (db->flags & SQLITE_Short_ستون_نام)!=0 || بھرا_Name;
  sqlite3_Vdbe_سیٹ_عدد_ستون_تعداد(v, pEList->n_اظہاریہ);
  برائے(i=0; i<pEList->n_اظہاریہ; i++){
    اظہاریہ *p = pEList->a[i].p_اظہاریہ;

    أكد( p!=0 );
    أكد( p->op!=TK_AGG_عمود );  /* Agg processing has not run yet */
    أكد( p->op!=TK_عمود
        || (اظہاریہ_Use_YTab(p) && p->y.pTab!=0) ); /* Covering idx not yet coded */
    اگر( pEList->a[i].zEName && pEList->a[i].fg.eEName==ENAME_NAME ){
      /* An AS clause always takes first priority */
      حرف *zName = pEList->a[i].zEName;
      sqlite3_Vdbe_سیٹ_ستون_Name(v, i, COLNAME_NAME, zName, SQLITE_TRANSIENT);
    }ورنہ اگر( srcName && p->op==TK_عمود ){
      حرف *z_ستون;
      صحیح i_ستون = p->i_عمود;
      pTab = p->y.pTab;
      أكد( pTab!=0 );
      اگر( i_ستون<0 ) i_ستون = pTab->iPKey;
      أكد( i_ستون==-1 || (i_ستون>=0 && i_ستون<pTab->n_ستون) );
      اگر( i_ستون<0 ){
        z_ستون = "rowid";
      }ورنہ{
        z_ستون = pTab->a_ستون[i_ستون].zCnName;
      }
      اگر( بھرا_Name ){
        حرف *zName = 0;
        zName = sqlite3MPrintf(db, "%s.%s", pTab->zName, z_ستون);
        sqlite3_Vdbe_سیٹ_ستون_Name(v, i, COLNAME_NAME, zName, SQLITE_DYNAMIC);
      }ورنہ{
        sqlite3_Vdbe_سیٹ_ستون_Name(v, i, COLNAME_NAME, z_ستون, SQLITE_TRANSIENT);
      }
    }ورنہ{
      ثابت حرف *z = pEList->a[i].zEName;
      z = z==0 ? sqlite3MPrintf(db, "column%d", i+1) : sqlite3DbStrDup(db, z);
      sqlite3_Vdbe_سیٹ_ستون_Name(v, i, COLNAME_NAME, z, SQLITE_DYNAMIC);
    }
  }
  generate_عمود_Types(pParse, pTabList, pEList);
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
صحیح sqlite3_Columns_From_اظہاریہ_List(
  Parse *pParse,          /* Parsing context */
  اظہاریہ_List *pEList,       /* Expr list from which to derive column names */
  i16 *pn_ستون,             /* Write the number of columns here */
  عمود **pa_ستون          /* Write the new column list here */
){
  sqlite3 *db = pParse->db;   /* Database connection */
  صحیح i, j;                   /* Loop counters */
  u32 cnt;                    /* Index added to make the name unique */
  عمود *a_ستون, *p_ستون;        /* For looping over result columns */
  صحیح n_ستون;                   /* Number of columns in the result set */
  حرف *zName;                /* Column name */
  صحیح nName;                  /* Size of name in zName[] */
  Hash ht;                    /* Hash table of column names */
  جدول *pTab;

  sqlite3HashInit(&ht);
  اگر( pEList ){
    n_ستون = pEList->n_اظہاریہ;
    a_ستون = sqlite3DbMallocZero(db, حجم(a_ستون[0])*n_ستون);
    testcase( a_ستون==0 );
    اگر( n_ستون>32767 ) n_ستون = 32767;
  }ورنہ{
    n_ستون = 0;
    a_ستون = 0;
  }
  أكد( n_ستون==(i16)n_ستون );
  *pn_ستون = n_ستون;
  *pa_ستون = a_ستون;

  برائے(i=0, p_ستون=a_ستون; i<n_ستون && !pParse->nErr; i++, p_ستون++){
    ساخت اظہاریہ_List_شے *pX = &pEList->a[i];
    ساخت اظہاریہ_List_شے *pCollide;
    /* Get an appropriate name for the column
    */
    اگر( (zName = pX->zEName)!=0 && pX->fg.eEName==ENAME_NAME ){
      /* If the column contains an "AS <name>" phrase, use <name> as the name */
    }ورنہ{
      اظہاریہ *p_ستون_اظہاریہ = sqlite3_اظہاریہ_Skip_Collate_And_Likely(pX->p_اظہاریہ);
      جب( ALWAYS(p_ستون_اظہاریہ!=0) && p_ستون_اظہاریہ->op==TK_DOT ){
        p_ستون_اظہاریہ = p_ستون_اظہاریہ->p_دایاں;
        أكد( p_ستون_اظہاریہ!=0 );
      }
      اگر( p_ستون_اظہاریہ->op==TK_عمود
       && ALWAYS( اظہاریہ_Use_YTab(p_ستون_اظہاریہ) )
       && ALWAYS( p_ستون_اظہاریہ->y.pTab!=0 )
      ){
        /* For columns use the column name name */
        صحیح i_ستون = p_ستون_اظہاریہ->i_عمود;
        pTab = p_ستون_اظہاریہ->y.pTab;
        اگر( i_ستون<0 ) i_ستون = pTab->iPKey;
        zName = i_ستون>=0 ? pTab->a_ستون[i_ستون].zCnName : "rowid";
      }ورنہ اگر( p_ستون_اظہاریہ->op==TK_ID ){
        أكد( !اظہاریہ_Has_Property(p_ستون_اظہاریہ, EP_Int_قدر) );
        zName = p_ستون_اظہاریہ->u.zToken;
      }ورنہ{
        /* Use the original text of the column expression as its name */
        أكد( zName==pX->zEName );  /* pointer comparison intended */
      }
    }
    اگر( zName && !sqlite3IsTrueOrFalse(zName) ){
      zName = sqlite3DbStrDup(db, zName);
    }ورنہ{
      zName = sqlite3MPrintf(db,"column%d",i+1);
    }

    /* Make sure the column name is unique.  If the name is not unique,
    ** append an integer to the name so that it becomes unique.
    */
    cnt = 0;
    جب( zName && (pCollide = sqlite3_Hash_ڈھونڈو(&ht, zName))!=0 ){
      اگر( pCollide->fg.b_Using_رکن ){
        p_ستون->ستون_Flags |= COLFLAG_NOEXPAND;
      }
      nName = sqlite3Strlen30(zName);
      اگر( nName>0 ){
        برائے(j=nName-1; j>0 && sqlite3Isdigit(zName[j]); j--){}
        اگر( zName[j]==':' ) nName = j;
      }
      zName = sqlite3MPrintf(db, "%.*z:%u", nName, zName, ++cnt);
      sqlite3_پیش_رفت_Check(pParse);
      اگر( cnt>3 ){
        sqlite3_randomness(حجم(cnt), &cnt);
      }
    }
    p_ستون->zCnName = zName;
    p_ستون->hName = sqlite3StrIHash(zName);
    اگر( pX->fg.bNoExpand ){
      p_ستون->ستون_Flags |= COLFLAG_NOEXPAND;
    }
    sqlite3_عمود_Properties_From_Name(0, p_ستون);
    اگر( zName && sqlite3_Hash_داخل_کرو(&ht, zName, pX)==pX ){
      sqlite3OomFault(db);
    }
  }
  sqlite3HashClear(&ht);
  اگر( pParse->nErr ){
    برائے(j=0; j<i; j++){
      sqlite3DbFree(db, a_ستون[j].zCnName);
    }
    sqlite3DbFree(db, a_ستون);
    *pa_ستون = 0;
    *pn_ستون = 0;
    واپس pParse->rc;
  }
  واپس SQLITE_OK;
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
خالی sqlite3_Subquery_عمود_Types(
  Parse *pParse,      /* Parsing contexts */
  جدول *pTab,        /* Add column type information to this table */
  Select *pSelect,    /* SELECT used to determine types and collations */
  حرف aff            /* Default affinity. */
){
  sqlite3 *db = pParse->db;
  عمود *p_ستون;
  CollSeq *pColl;
  صحیح i,j;
  اظہاریہ *p;
  ساخت اظہاریہ_List_شے *a;
  NameContext sNC;

  أكد( pSelect!=0 );
  أكد( (pSelect->selFlags & SF_Resolved)!=0 );
  أكد( pTab->n_ستون==pSelect->pEList->n_اظہاریہ || pParse->nErr>0 );
  أكد( aff==SQLITE_AFF_NONE || aff==SQLITE_AFF_BLOB );
  اگر( db->mallocFailed || اندر_RENAME_OBJECT ) واپس;
  جب( pSelect->p_پیشگی ) pSelect = pSelect->p_پیشگی;
  a = pSelect->pEList->a;
  memset(&sNC, 0, حجم(sNC));
  sNC.pSrcList = pSelect->pSrc;
  برائے(i=0, p_ستون=pTab->a_ستون; i<pTab->n_ستون; i++, p_ستون++){
    ثابت حرف *zType;
    i64 n;
    صحیح m = 0;
    Select *pS2 = pSelect;
    pTab->tabFlags |= (p_ستون->ستون_Flags & COLFLAG_NOINSERT);
    p = a[i].p_اظہاریہ;
    /* pCol->szEst = ... // Column size est for SELECT tables never used */
    p_ستون->affinity = sqlite3_اظہاریہ_Affinity(p);
    جب( p_ستون->affinity<=SQLITE_AFF_NONE && pS2->p_اگلا!=0 ){
      m |= sqlite3_اظہاریہ_ڈیٹا_Type(pS2->pEList->a[i].p_اظہاریہ);
      pS2 = pS2->p_اگلا;
      p_ستون->affinity = sqlite3_اظہاریہ_Affinity(pS2->pEList->a[i].p_اظہاریہ);
    }
    اگر( p_ستون->affinity<=SQLITE_AFF_NONE ){
      p_ستون->affinity = aff;
    }
    اگر( p_ستون->affinity>=SQLITE_AFF_متن && (pS2->p_اگلا || pS2!=pSelect) ){
      برائے(pS2=pS2->p_اگلا; pS2; pS2=pS2->p_اگلا){
        m |= sqlite3_اظہاریہ_ڈیٹا_Type(pS2->pEList->a[i].p_اظہاریہ);
      }
      اگر( p_ستون->affinity==SQLITE_AFF_متن && (m&0x01)!=0 ){
        p_ستون->affinity = SQLITE_AFF_BLOB;
      }ورنہ
      اگر( p_ستون->affinity>=SQLITE_AFF_NUMERIC && (m&0x02)!=0 ){
        p_ستون->affinity = SQLITE_AFF_BLOB;
      }
      اگر( p_ستون->affinity>=SQLITE_AFF_NUMERIC && p->op==TK_CAST ){
        p_ستون->affinity = SQLITE_AFF_FLEXNUM;
      }
    }
    zType = عمود_Type(&sNC, p, 0, 0, 0);
    اگر( zType==0 || p_ستون->affinity!=sqlite3AffinityType(zType, 0) ){
      اگر( p_ستون->affinity==SQLITE_AFF_NUMERIC
       || p_ستون->affinity==SQLITE_AFF_FLEXNUM
      ){
        zType = "NUM";
      }ورنہ{
        zType = 0;
        برائے(j=1; j<SQLITE_N_STDTYPE; j++){
          اگر( sqlite3StdTypeAffinity[j]==p_ستون->affinity ){
            zType = sqlite3StdType[j];
            توڑو;
          }
        }
      }
    }
    اگر( zType ){
      ثابت i64 k = لڑی_لمبائی(zType);
      n = لڑی_لمبائی(p_ستون->zCnName);
      p_ستون->zCnName = sqlite3DbReallocOrFree(db, p_ستون->zCnName, n+k+2);
      p_ستون->ستون_Flags &= ~(COLFLAG_HASTYPE|COLFLAG_HASCOLL);
      اگر( p_ستون->zCnName ){
        حافظہ_نقل(&p_ستون->zCnName[n+1], zType, k+1);
        p_ستون->ستون_Flags |= COLFLAG_HASTYPE;
      }
    }
    pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p);
    اگر( pColl ){
      أكد( pTab->p_اشاریہ==0 );
      sqlite3_عمود_سیٹ_Coll(db, p_ستون, pColl->zName);
    }
  }
  pTab->sz_Tab_صف = 1; /* Any non-zero value works */
}

/*
** Check the current subquery nesting depth.  Return true and set an
** error if it has gone too deep.
*/
ساکن صحیح check_Subquery_Nesting_گہرائی(Parse *pParse){
#اگر SQLITE_MAX_اظہاریہ_گہرائی>0
  اگر( pParse->nNestSel >= pParse->db->a_حد_بندی[SQLITE_حد_بندی_اظہاریہ_گہرائی] ){
    sqlite3_Error_پیغام(pParse, "VIEWs and/or subqueries nested too deep");
    واپس 1;
  }
#اگر_ختم
  واپس 0;
}

/*
** Given a SELECT statement, generate a Table structure that describes
** the result set of that SELECT.
*/
جدول *sqlite3_نتیجہ_سیٹ_کا_Select(Parse *pParse, Select *pSelect, حرف aff){
  جدول *pTab;
  sqlite3 *db = pParse->db;
  u64 savedFlags;

  pParse->nNestSel++;
  اگر( check_Subquery_Nesting_گہرائی(pParse) ) واپس 0;
  savedFlags = db->flags;
  db->flags &= ~(u64)SQLITE_بھرا_ستون_نام;
  db->flags |= SQLITE_Short_ستون_نام;
  sqlite3SelectPrep(pParse, pSelect, 0);
  db->flags = savedFlags;
  اگر( pParse->nErr ) واپس 0;
  جب( pSelect->p_پیشگی ) pSelect = pSelect->p_پیشگی;
  pTab = sqlite3DbMallocZero(db, حجم(جدول) );
  اگر( pTab==0 ){
    واپس 0;
  }
  pTab->nTabRef = 1;
  pTab->zName = 0;
  pTab->n_صف_Log_Est = 200; أكد( 200==sqlite3LogEst(1048576) );
  sqlite3_Columns_From_اظہاریہ_List(pParse, pSelect->pEList, &pTab->n_ستون, &pTab->a_ستون);
  sqlite3_Subquery_عمود_Types(pParse, pTab, pSelect, aff);
  pTab->iPKey = -1;
  اگر( db->mallocFailed ){
    sqlite3_Delete_جدول(db, pTab);
    واپس 0;
  }
  pParse->nNestSel--;
  أكد( pParse->nNestSel>=0 );
  واپس pTab;
}

/*
** Get a VDBE for the given parser context.  Create a new one if necessary.
** If an error occurs, return NULL and leave a message in pParse.
*/
Vdbe *sqlite3GetVdbe(Parse *pParse){
  اگر( pParse->pVdbe ){
    واپس pParse->pVdbe;
  }
  اگر( pParse->pToplevel==0 ){
    pParse->ok_Const_ضربی_عدد = 1;
  }
  واپس sqlite3VdbeCreate(pParse);
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
ساکن خالی compute_حد_بندی_Registers(Parse *pParse, Select *p, صحیح iBreak){
  Vdbe *v = 0;
  صحیح i_حد_بندی = 0;
  صحیح i_انحراف;
  صحیح n;
  اظہاریہ *p_حد_بندی = p->p_حد_بندی;

  اگر( p->i_حد_بندی ) واپس;

  /*
  ** "LIMIT -1" always shows all rows.  There is some
  ** controversy about what the correct behavior should be.
  ** The current implementation interprets "LIMIT 0" to mean
  ** no rows.
  */
  اگر( p_حد_بندی ){
    أكد( p_حد_بندی->op==TK_حد_بندی );
    أكد( p_حد_بندی->p_بایاں!=0 );
    p->i_حد_بندی = i_حد_بندی = ++pParse->nMem;
    v = sqlite3GetVdbe(pParse);
    أكد( v!=0 );
    اگر( sqlite3_اظہاریہ_Is_Integer(p_حد_بندی->p_بایاں, &n, pParse, 0) ){
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, n, i_حد_بندی);
      VdbeComment((v, "LIMIT counter"));
      اگر( n==0 ){
        sqlite3VdbeGoto(v, iBreak);
      }ورنہ اگر( n>=0 && p->n_Select_صف>sqlite3LogEst((u64)n) ){
        p->n_Select_صف = sqlite3LogEst((u64)n);
        p->selFlags |= SF_Fixed_حد_بندی;
      }
    }ورنہ{
      sqlite3_اظہاریہ_رمز(pParse, p_حد_بندی->p_بایاں, i_حد_بندی);
      sqlite3_Vdbe_جوڑو_Op1(v, OP_MustBeInt, i_حد_بندی); VdbeCoverage(v);
      VdbeComment((v, "LIMIT counter"));
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_IfNot, i_حد_بندی, iBreak); VdbeCoverage(v);
    }
    اگر( p_حد_بندی->p_دایاں ){
      p->i_انحراف = i_انحراف = ++pParse->nMem;
      pParse->nMem++;   /* Allocate an extra register for limit+offset */
      sqlite3_اظہاریہ_رمز(pParse, p_حد_بندی->p_دایاں, i_انحراف);
      sqlite3_Vdbe_جوڑو_Op1(v, OP_MustBeInt, i_انحراف); VdbeCoverage(v);
      sqlite3MayAbort(pParse);
      VdbeComment((v, "OFFSET counter"));
      sqlite3_Vdbe_جوڑو_Op3(v, OP_انحراف_حد_بندی, i_حد_بندی, i_انحراف+1, i_انحراف);
      VdbeComment((v, "LIMIT+OFFSET"));
    }
  }
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_COMPOUND_SELECT
/*
** Return the appropriate collating sequence for the iCol-th column of
** the result set for the compound-select statement "p".  Return NULL if
** the column has no default collating sequence.
**
** The collating sequence for the compound select is taken from the
** left-most term of the select that has a collating sequence.
*/
ساکن CollSeq *multiSelectCollSeq(Parse *pParse, Select *p, صحیح i_ستون){
  CollSeq *pRet;
  اگر( p->p_پیشگی ){
    pRet = multiSelectCollSeq(pParse, p->p_پیشگی, i_ستون);
  }ورنہ{
    pRet = 0;
  }
  أكد( i_ستون>=0 );
  /* iCol must be less than p->pEList->nExpr.  Otherwise an error would
  ** have been thrown during name resolution and we would not have gotten
  ** this far */
  اگر( pRet==0 && ALWAYS(i_ستون<p->pEList->n_اظہاریہ) ){
    pRet = sqlite3_اظہاریہ_Coll_Seq(pParse, p->pEList->a[i_ستون].p_اظہاریہ);
  }
  واپس pRet;
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
ساکن کلید_Info *multi_Select_By_ملاؤ_کلید_Info(Parse *pParse, Select *p, صحیح nExtra){
  اظہاریہ_List *p_ترتیب_By = p->p_ترتیب_By;
  صحیح n_ترتیب_By = (p_ترتیب_By!=0) ? p_ترتیب_By->n_اظہاریہ : 0;
  sqlite3 *db = pParse->db;
  کلید_Info *pRet = sqlite3_کلید_Info_Alloc(db, n_ترتیب_By+nExtra, 1);
  اگر( pRet ){
    صحیح i;
    برائے(i=0; i<n_ترتیب_By; i++){
      ساخت اظہاریہ_List_شے *p_شے = &p_ترتیب_By->a[i];
      اظہاریہ *p_رکن = p_شے->p_اظہاریہ;
      CollSeq *pColl;

      اگر( p_رکن->flags & EP_Collate ){
        pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p_رکن);
      }ورنہ{
        pColl = multiSelectCollSeq(pParse, p, p_شے->u.x.i_ترتیب_By_ستون-1);
        اگر( pColl==0 ) pColl = db->pDfltColl;
        p_ترتیب_By->a[i].p_اظہاریہ =
          sqlite3_اظہاریہ_جوڑو_Collate_String(pParse, p_رکن, pColl->zName);
      }
      أكد( sqlite3_کلید_Info_Is_Writeable(pRet) );
      pRet->aColl[i] = pColl;
      pRet->a_ترتیب_دو_Flags[i] = p_ترتیب_By->a[i].fg.ترتیب_دو_Flags;
    }
  }

  واپس pRet;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CTE
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
ساکن خالی generateWithRecursiveQuery(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The recursive SELECT to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  SrcList *pSrc = p->pSrc;      /* The FROM clause of the recursive query */
  صحیح n_ستون = p->pEList->n_اظہاریہ;  /* Number of columns in the recursive table */
  Vdbe *v = pParse->pVdbe;      /* The prepared statement under construction */
  Select *p_تیاری;               /* The setup query */
  Select *p_پہلا_Rec;            /* Left-most recursive term */
  صحیح addr_چوٹی;                  /* Top of the loop */
  صحیح addrCont, addrBreak;      /* CONTINUE and BREAK addresses */
  صحیح iCurrent = 0;             /* The Current table */
  صحیح regCurrent;               /* Register holding Current table */
  صحیح i_قطار;                   /* The Queue table */
  صحیح iDistinct = 0;            /* To ensure unique results if UNION */
  صحیح eDest = SRT_Fifo;         /* How to write to Queue */
  SelectDest dest_قطار;         /* SelectDest targeting the Queue table */
  صحیح i;                        /* Loop counter */
  صحیح rc;                       /* Result code */
  اظہاریہ_List *p_ترتیب_By;           /* The ORDER BY clause */
  اظہاریہ *p_حد_بندی;                 /* Saved LIMIT and OFFSET */
  صحیح reg_حد_بندی, reg_انحراف;      /* Registers used by LIMIT and OFFSET */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
  اگر( p->pWin ){
    sqlite3_Error_پیغام(pParse, "cannot use window functions in recursive queries");
    واپس;
  }
#اگر_ختم

  /* Obtain authorization to do a recursive query */
  اگر( sqlite3AuthCheck(pParse, SQLITE_RECURSIVE, 0, 0, 0) ) واپس;

  /* Process the LIMIT and OFFSET clauses, if they exist */
  addrBreak = sqlite3VdbeMakeLabel(pParse);
  p->n_Select_صف = 320;  /* 4 billion rows */
  compute_حد_بندی_Registers(pParse, p, addrBreak);
  p_حد_بندی = p->p_حد_بندی;
  reg_حد_بندی = p->i_حد_بندی;
  reg_انحراف = p->i_انحراف;
  p->p_حد_بندی = 0;
  p->i_حد_بندی = p->i_انحراف = 0;
  p_ترتیب_By = p->p_ترتیب_By;

  /* Locate the cursor number of the Current table */
  برائے(i=0; ALWAYS(i<pSrc->nSrc); i++){
    اگر( pSrc->a[i].fg.isRecursive ){
      iCurrent = pSrc->a[i].iCursor;
      توڑو;
    }
  }

  /* Allocate cursors numbers for Queue and Distinct.  The cursor number for
  ** the Distinct table must be exactly one greater than Queue in order
  ** for the SRT_DistFifo and SRT_DistQueue destinations to work. */
  i_قطار = pParse->nTab++;
  اگر( p->op==TK_UNION ){
    eDest = p_ترتیب_By ? SRT_Dist_قطار : SRT_DistFifo;
    iDistinct = pParse->nTab++;
  }ورنہ{
    eDest = p_ترتیب_By ? SRT_قطار : SRT_Fifo;
  }
  sqlite3SelectDestInit(&dest_قطار, eDest, i_قطار);

  /* Allocate cursors for Current, Queue, and Distinct. */
  regCurrent = ++pParse->nMem;
  sqlite3_Vdbe_جوڑو_Op3(v, OP_OpenPseudo, iCurrent, regCurrent, n_ستون);
  اگر( p_ترتیب_By ){
    کلید_Info *p_کلید_Info = multi_Select_By_ملاؤ_کلید_Info(pParse, p, 1);
    sqlite3_Vdbe_جوڑو_Op4(v, OP_OpenEphemeral, i_قطار, p_ترتیب_By->n_اظہاریہ+2, 0,
                      (حرف*)p_کلید_Info, P4_KEYINFO);
    dest_قطار.p_ترتیب_By = p_ترتیب_By;
  }ورنہ{
    sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenEphemeral, i_قطار, n_ستون);
  }
  VdbeComment((v, "Queue table"));
  اگر( iDistinct ){
    /* Generate an ephemeral table used to enforce distinctness on the
    ** output of the recursive part of the CTE.
    */
    کلید_Info *p_کلید_Info;            /* Collating sequence for the result set */
    CollSeq **apColl;             /* For looping through pKeyInfo->aColl[] */

    أكد( p->p_اگلا==0 );
    أكد( p->pEList!=0 );
    n_ستون = p->pEList->n_اظہاریہ;
    p_کلید_Info = sqlite3_کلید_Info_Alloc(pParse->db, n_ستون, 1);
    اگر( p_کلید_Info ){
      برائے(i=0, apColl=p_کلید_Info->aColl; i<n_ستون; i++, apColl++){
        *apColl = multiSelectCollSeq(pParse, p, i);
        اگر( 0==*apColl ){
          *apColl = pParse->db->pDfltColl;
        }
      }
      sqlite3_Vdbe_جوڑو_Op4(v, OP_OpenEphemeral, iDistinct, n_ستون, 0,
                           (خالی*)p_کلید_Info, P4_KEYINFO);
    }ورنہ{
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
  برائے(p_پہلا_Rec=p; ALWAYS(p_پہلا_Rec!=0); p_پہلا_Rec=p_پہلا_Rec->p_پیشگی){
    اگر( p_پہلا_Rec->selFlags & SF_Aggregate ){
      sqlite3_Error_پیغام(pParse, "recursive aggregate queries not supported");
      جاؤ انتہا_کا_recursive_query;
    }
    p_پہلا_Rec->op = TK_ALL;
    اگر( (p_پہلا_Rec->p_پیشگی->selFlags & SF_Recursive)==0 ) توڑو;
  }

  /* Store the results of the setup-query in Queue. */
  p_تیاری = p_پہلا_Rec->p_پیشگی;
  p_تیاری->p_اگلا = 0;
  ExplainQueryPlan((pParse, 1, "SETUP"));
  rc = sqlite3Select(pParse, p_تیاری, &dest_قطار);
  p_تیاری->p_اگلا = p;
  اگر( rc ) جاؤ انتہا_کا_recursive_query;

  /* Find the next row in the Queue and output that row */
  addr_چوٹی = sqlite3_Vdbe_جوڑو_Op2(v, OP_Rewind, i_قطار, addrBreak); VdbeCoverage(v);

  /* Transfer the next row in Queue over to Current */
  sqlite3_Vdbe_جوڑو_Op1(v, OP_Null_صف, iCurrent); /* To reset column cache */
  اگر( p_ترتیب_By ){
    sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, i_قطار, p_ترتیب_By->n_اظہاریہ+1, regCurrent);
  }ورنہ{
    sqlite3_Vdbe_جوڑو_Op2(v, OP_صف_ڈیٹا, i_قطار, regCurrent);
  }
  sqlite3_Vdbe_جوڑو_Op1(v, OP_Delete, i_قطار);

  /* Output the single row in Current */
  addrCont = sqlite3VdbeMakeLabel(pParse);
  رمز_انحراف(v, reg_انحراف, addrCont);
  selectInnerLoop(pParse, p, iCurrent,
      0, 0, pDest, addrCont, addrBreak);
  اگر( reg_حد_بندی ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_DecrJumpZero, reg_حد_بندی, addrBreak);
    VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, addrCont);

  /* Execute the recursive SELECT taking the single row in Current as
  ** the value for the recursive-table. Store the results in the Queue.
  */
  p_پہلا_Rec->p_پیشگی = 0;
  ExplainQueryPlan((pParse, 1, "RECURSIVE STEP"));
  sqlite3Select(pParse, p, &dest_قطار);
  أكد( p_پہلا_Rec->p_پیشگی==0 );
  p_پہلا_Rec->p_پیشگی = p_تیاری;

  /* Keep running the loop until the Queue is empty */
  sqlite3VdbeGoto(v, addr_چوٹی);
  sqlite3VdbeResolveLabel(v, addrBreak);

انتہا_کا_recursive_query:
  sqlite3_اظہاریہ_List_Delete(pParse->db, p->p_ترتیب_By);
  p->p_ترتیب_By = p_ترتیب_By;
  p->p_حد_بندی = p_حد_بندی;
  واپس;
}
#اگر_ختم /* SQLITE_OMIT_CTE */

/* Forward references */
ساکن صحیح multi_Select_By_ملاؤ(
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
ساکن صحیح multi_Select_اقدار(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  صحیح n_صف = 1;
  صحیح rc = 0;
  صحیح bShowAll = p->p_حد_بندی==0;
  أكد( p->selFlags & SF_Multi_قدر );
  کرو{
    أكد( p->selFlags & SF_اقدار );
    أكد( p->op==TK_ALL || (p->op==TK_SELECT && p->p_پیشگی==0) );
    أكد( p->p_اگلا==0 || p->pEList->n_اظہاریہ==p->p_اگلا->pEList->n_اظہاریہ );
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    اگر( p->pWin ) واپس -1;
#اگر_ختم
    اگر( p->p_پیشگی==0 ) توڑو;
    أكد( p->p_پیشگی->p_اگلا==p );
    p = p->p_پیشگی;
    n_صف += bShowAll;
  }جب(1);
  ExplainQueryPlan((pParse, 0, "SCAN %d CONSTANT ROW%s", n_صف,
                    n_صف==1 ? "" : "S"));
  جب( p ){
    selectInnerLoop(pParse, p, -1, 0, 0, pDest, 1, 1);
    اگر( !bShowAll ) توڑو;
    p->n_Select_صف = n_صف;
    p = p->p_اگلا;
  }
  واپس rc;
}

/*
** Return true if the SELECT statement which is known to be the recursive
** part of a recursive CTE still has its anchor terms attached.  If the
** anchor terms have already been removed, then return false.
*/
ساکن صحیح hasAnchor(Select *p){
  جب( p && (p->selFlags & SF_Recursive)!=0 ){ p = p->p_پیشگی; }
  واپس p!=0;
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
ساکن صحیح union_With_حد_بندی_One(sqlite3 *db, Select *p){
  صحیح v;
  اگر( p->op!=TK_UNION ) واپس 0;
  اگر( p->p_حد_بندی==0 ) واپس 0;
  اگر( p->p_حد_بندی->p_دایاں ) واپس 0;  /* No OFFSET */
  v = 0;
  أكد( p->p_حد_بندی->p_بایاں!=0 );
  اگر( sqlite3_اظہاریہ_Is_Integer(p->p_حد_بندی->p_بایاں, &v, 0, 0)==0 ) واپس 0;
  اگر( v!=1 ) واپس 0;
  اگر( OptimizationDisabled(db, SQLITE_Union_حد_بندی) ) واپس 0;
  واپس 1;
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
ساکن صحیح multiSelect(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  صحیح rc = SQLITE_OK;   /* Success code from a subroutine */
  Select *p_پیشگی;       /* Another SELECT immediately to our left */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest dest;      /* Alternative data destination */
  Select *pDelete = 0;  /* Chain of simple selects to delete */
  sqlite3 *db;          /* Database connection */

  /* Make sure there is no ORDER BY or LIMIT clause on prior SELECTs.  Only
  ** the last (right-most) SELECT in the series may have an ORDER BY or LIMIT.
  */
  أكد( p && p->p_پیشگی );  /* Calling function guarantees this much */
  أكد( (p->selFlags & SF_Recursive)==0 || p->op==TK_ALL || p->op==TK_UNION );
  أكد( p->selFlags & SF_Compound );
  db = pParse->db;
  p_پیشگی = p->p_پیشگی;
  dest = *pDest;
  أكد( p_پیشگی->p_ترتیب_By==0 );
  أكد( p_پیشگی->p_حد_بندی==0 );

  v = sqlite3GetVdbe(pParse);
  أكد( v!=0 );  /* The VDBE already created by calling function */

  /* Create the destination temporary table if necessary
  */
  اگر( dest.eDest==SRT_EphemTab ){
    أكد( p->pEList );
    sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenEphemeral, dest.iSDParm, p->pEList->n_اظہاریہ);
    dest.eDest = SRT_جدول;
  }

  /* Special handling for a compound-select that originates as a VALUES clause.
  */
  اگر( p->selFlags & SF_Multi_قدر ){
    rc = multi_Select_اقدار(pParse, p, &dest);
    اگر( rc>=0 ) جاؤ multi_select_انتہا;
    rc = SQLITE_OK;
  }

  /* Make sure all SELECTs in the statement have the same number of elements
  ** in their result sets.
  */
  أكد( p->pEList && p_پیشگی->pEList );
  أكد( p->pEList->n_اظہاریہ==p_پیشگی->pEList->n_اظہاریہ );

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CTE
  اگر( (p->selFlags & SF_Recursive)!=0 && hasAnchor(p) ){
    generateWithRecursiveQuery(pParse, p, &dest);
  }ورنہ
#اگر_ختم
  اگر( p->p_ترتیب_By ){
    /* If the compound has an ORDER BY clause, then always use the merge
    ** algorithm. */
    واپس multi_Select_By_ملاؤ(pParse, p, pDest);
  }ورنہ اگر( p->op!=TK_ALL && !union_With_حد_بندی_One(db,p) ){
    /* If the compound is EXCEPT, INTERSECT, or UNION (anything other than
    ** UNION ALL) then also always use the merge algorithm.  However, the
    ** multiSelectByMerge() routine requires that the compound have an
    ** ORDER BY clause, and it doesn't right now.  So invent one first. */
    اظہاریہ *pOne = sqlite3_اظہاریہ_Int32(db, 1);
    p->p_ترتیب_By = sqlite3_اظہاریہ_List_Append(pParse, 0, pOne);
    اگر( pParse->nErr ) جاؤ multi_select_انتہا;
    أكد( p->p_ترتیب_By!=0 );
    p->p_ترتیب_By->a[0].u.x.i_ترتیب_By_ستون = 1;
    واپس multi_Select_By_ملاؤ(pParse, p, pDest);
  }ورنہ{
    /* For a UNION ALL compound without ORDER BY, or for a UNION with a 
    ** LIMIT of exactly 1 and no OFFSET and no ORDER BY, simply run the left
    ** query, then run the right query */
    صحیح addr = 0;
    صحیح n_حد_بندی = 0;  /* Initialize to suppress harmless compiler warning */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_EXPLAIN
    اگر( p_پیشگی->p_پیشگی==0 ){
      ExplainQueryPlan((pParse, 1, "COMPOUND QUERY"));
      ExplainQueryPlan((pParse, 1, "LEFT-MOST SUBQUERY"));
    }
#اگر_ختم
    أكد( !p_پیشگی->p_حد_بندی );
    p_پیشگی->i_حد_بندی = p->i_حد_بندی;
    p_پیشگی->i_انحراف = p->i_انحراف;
    p_پیشگی->p_حد_بندی = sqlite3_اظہاریہ_Dup(db, p->p_حد_بندی, 0);
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL left...\n"));
    rc = sqlite3Select(pParse, p_پیشگی, &dest);
    sqlite3_اظہاریہ_Delete(db, p_پیشگی->p_حد_بندی);
    p_پیشگی->p_حد_بندی = 0;
    اگر( rc ){
      جاؤ multi_select_انتہا;
    }
    p->p_پیشگی = 0;
    p->i_حد_بندی = p_پیشگی->i_حد_بندی;
    p->i_انحراف = p_پیشگی->i_انحراف;
    اگر( p->i_حد_بندی ){
      addr = sqlite3_Vdbe_جوڑو_Op1(v, OP_IfNot, p->i_حد_بندی); VdbeCoverage(v);
      VdbeComment((v, "Jump ahead if LIMIT reached"));
      اگر( p->i_انحراف ){
        sqlite3_Vdbe_جوڑو_Op3(v, OP_انحراف_حد_بندی,
                          p->i_حد_بندی, p->i_انحراف+1, p->i_انحراف);
      }
    }
    ExplainQueryPlan((pParse, 1, "UNION ALL"));
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL right...\n"));
    rc = sqlite3Select(pParse, p, &dest);
    testcase( rc!=SQLITE_OK );
    pDelete = p->p_پیشگی;
    p->p_پیشگی = p_پیشگی;
    p->n_Select_صف = sqlite3_Log_Est_جوڑو(p->n_Select_صف, p_پیشگی->n_Select_صف);
    اگر( p->p_حد_بندی
     && sqlite3_اظہاریہ_Is_Integer(p->p_حد_بندی->p_بایاں, &n_حد_بندی, pParse, 0)
     && n_حد_بندی>0 && p->n_Select_صف > sqlite3LogEst((u64)n_حد_بندی)
    ){
      p->n_Select_صف = sqlite3LogEst((u64)n_حد_بندی);
    }
    اگر( addr ){
      sqlite3_Vdbe_Jump_یہاں(v, addr);
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_EXPLAIN
    اگر( p->p_اگلا==0 ){
      Explain_Query_Plan_نکالو(pParse);
    }
#اگر_ختم
  }

multi_select_انتہا:
  pDest->iSdst = dest.iSdst;
  pDest->nSdst = dest.nSdst;
  pDest->iSDParm2 = dest.iSDParm2;
  اگر( pDelete ){
    sqlite3_Parser_جوڑو_Cleanup(pParse, sqlite3SelectDeleteGeneric, pDelete);
  }
  واپس rc;
}
#اگر_ختم /* SQLITE_OMIT_COMPOUND_SELECT */

/*
** Error message for when two or more terms of a compound select have different
** size result sets.
*/
خالی sqlite3_Select_Wrong_عدد_Terms_Error(Parse *pParse, Select *p){
  اگر( p->selFlags & SF_اقدار ){
    sqlite3_Error_پیغام(pParse, "all VALUES must have the same number of terms");
  }ورنہ{
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
ساکن صحیح generateOutputSubroutine(
  Parse *pParse,          /* Parsing context */
  Select *p,              /* The SELECT statement */
  SelectDest *p_اندر,        /* Coroutine supplying data */
  SelectDest *pDest,      /* Where to send the data */
  صحیح regReturn,          /* The return address register */
  صحیح reg_پچھلا,            /* Previous result register.  No uniqueness if 0 */
  کلید_Info *p_کلید_Info,      /* For comparing with previous entry */
  صحیح iBreak              /* Jump here if we hit the LIMIT */
){
  Vdbe *v = pParse->pVdbe;
  صحیح iContinue;
  صحیح addr;

  أكد( p_اندر->eDest==SRT_Coroutine );

  addr = sqlite3VdbeCurrentAddr(v);
  iContinue = sqlite3VdbeMakeLabel(pParse);

  /* Suppress duplicates for UNION, EXCEPT, and INTERSECT
  */
  اگر( reg_پچھلا ){
    صحیح addr1, addr2;
    addr1 = sqlite3_Vdbe_جوڑو_Op1(v, OP_IfNot, reg_پچھلا); VdbeCoverage(v);
    addr2 = sqlite3_Vdbe_جوڑو_Op4(v, OP_Compare, p_اندر->iSdst, reg_پچھلا+1, p_اندر->nSdst,
                              (حرف*)sqlite3_کلید_Info_Ref(p_کلید_Info), P4_KEYINFO);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Jump, addr2+2, iContinue, addr2+2); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_یہاں(v, addr1);
    sqlite3_Vdbe_جوڑو_Op3(v, OP_Copy, p_اندر->iSdst, reg_پچھلا+1, p_اندر->nSdst-1);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 1, reg_پچھلا);
  }
  اگر( pParse->db->mallocFailed ) واپس 0;

  /* Suppress the first OFFSET entries if there is an OFFSET clause
  */
  رمز_انحراف(v, p->i_انحراف, iContinue);

  انتخاب( pDest->eDest ){
    /* Store the result as data using a unique key.
    */
    حالت SRT_Fifo:
    حالت SRT_DistFifo:
    حالت SRT_جدول:
    حالت SRT_EphemTab: {
      صحیح r1 = sqlite3_Get_عارضی_Reg(pParse);
      صحیح r2 = sqlite3_Get_عارضی_Reg(pParse);
      صحیح iParm = pDest->iSDParm;
      testcase( pDest->eDest==SRT_جدول );
      testcase( pDest->eDest==SRT_EphemTab );
      testcase( pDest->eDest==SRT_Fifo );
      testcase( pDest->eDest==SRT_DistFifo );
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, p_اندر->iSdst, p_اندر->nSdst, r1);
#اگر !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      اگر( pDest->eDest==SRT_جدول && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CTE
      اگر( pDest->eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index that is used to enforce uniqueness on the
        ** total result.  At this point, we are processing the setup portion
        ** of the recursive CTE using the merge algorithm, so the results are
        ** guaranteed to be unique anyhow.  But we still need to populate the
        ** (iParm+1) cursor for use by the subsequent recursive phase.
        */
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iParm+1, r1,
                             p_اندر->iSdst, p_اندر->nSdst);
      }
#اگر_ختم
      sqlite3_Vdbe_جوڑو_Op2(v, OP_NewRowid, iParm, r2);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_داخل_کرو, iParm, r1, r2);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      sqlite3_Release_عارضی_Reg(pParse, r2);
      sqlite3_Release_عارضی_Reg(pParse, r1);
      توڑو;
    }

    /* If any row exist in the result set, record that fact and abort.
    */
    حالت SRT_Exists: {
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 1, pDest->iSDParm);
      /* The LIMIT clause will terminate the loop for us */
      توڑو;
    }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)".
    */
    حالت SRT_سیٹ: {
      صحیح r1;
      testcase( p_اندر->nSdst>1 );
      r1 = sqlite3_Get_عارضی_Reg(pParse);
      sqlite3_Vdbe_جوڑو_Op4(v, OP_Make_ریکارڈ, p_اندر->iSdst, p_اندر->nSdst,
          r1, pDest->zAffSdst, p_اندر->nSdst);
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, pDest->iSDParm, r1,
                           p_اندر->iSdst, p_اندر->nSdst);
      اگر( pDest->iSDParm2>0 ){
        أكد( sqlite3CursorBloomable(pParse, pDest->iSDParm, p_اندر->nSdst) );
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Filter_جوڑو, pDest->iSDParm2, 0,
                             p_اندر->iSdst, p_اندر->nSdst);
        ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
      }
      sqlite3_Release_عارضی_Reg(pParse, r1);
      توڑو;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell and break out
    ** of the scan loop.  Note that the select might return multiple columns
    ** if it is the RHS of a row-value IN operator.
    */
    حالت SRT_Mem: {
      testcase( p_اندر->nSdst>1 );
      sqlite3_اظہاریہ_رمز_Move(pParse, p_اندر->iSdst, pDest->iSDParm, p_اندر->nSdst);
      /* The LIMIT clause will jump out of the loop for us */
      توڑو;
    }
#اگر_ختم /* #ifndef SQLITE_OMIT_SUBQUERY */

    /* The results are stored in a sequence of registers
    ** starting at pDest->iSdst.  Then the co-routine yields.
    */
    حالت SRT_Coroutine: {
      اگر( pDest->iSdst==0 ){
        pDest->iSdst = sqlite3_Get_عارضی_Range(pParse, p_اندر->nSdst);
        pDest->nSdst = p_اندر->nSdst;
      }
      sqlite3_اظہاریہ_رمز_Move(pParse, p_اندر->iSdst, pDest->iSdst, p_اندر->nSdst);
      sqlite3_Vdbe_جوڑو_Op1(v, OP_Yield, pDest->iSDParm);
      توڑو;
    }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    حالت SRT_Dist_قطار:
    حالت SRT_قطار: {
      صحیح n_کلید;
      صحیح r1, r2, r3, ii;
      اظہاریہ_List *pSO;
      صحیح iParm = pDest->iSDParm;
      pSO = pDest->p_ترتیب_By;
      أكد( pSO );
      n_کلید = pSO->n_اظہاریہ;
      r1 = sqlite3_Get_عارضی_Reg(pParse);
      r2 = sqlite3_Get_عارضی_Range(pParse, n_کلید+2);
      r3 = r2+n_کلید+1;

      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, p_اندر->iSdst, p_اندر->nSdst, r3);
      اگر( pDest->eDest==SRT_Dist_قطار ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Idx_داخل_کرو, iParm+1, r3);
      }
      برائے(ii=0; ii<n_کلید; ii++){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_SCopy,
                          p_اندر->iSdst + pSO->a[ii].u.x.i_ترتیب_By_ستون - 1,
                          r2+ii);
      }
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Sequence, iParm, r2+n_کلید);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, r2, n_کلید+2, r1);
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, iParm, r1, r2, n_کلید+2);
      sqlite3_Release_عارضی_Reg(pParse, r1);
      sqlite3_Release_عارضی_Range(pParse, r2, n_کلید+2);
      توڑو;
    }
#اگر_ختم /* SQLITE_OMIT_CTE */

    /* Ignore the output */
    حالت SRT_Discard: {
      توڑو;
    }

    /* If none of the above, then the result destination must be
    ** SRT_Output.
    **
    ** For SRT_Output, results are stored in a sequence of registers. 
    ** Then the OP_ResultRow opcode is used to cause sqlite3_step() to
    ** return the next row of result.
    */
    طےشدہ: {
      أكد( pDest->eDest==SRT_Output );
      sqlite3_Vdbe_جوڑو_Op2(v, OP_نتیجہ_صف, p_اندر->iSdst, p_اندر->nSdst);
      توڑو;
    }
  }

  /* Jump to the end of the loop if the LIMIT is reached.
  */
  اگر( p->i_حد_بندی ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_DecrJumpZero, p->i_حد_بندی, iBreak); VdbeCoverage(v);
  }

  /* Generate the subroutine return
  */
  sqlite3VdbeResolveLabel(v, iContinue);
  sqlite3_Vdbe_جوڑو_Op1(v, OP_Return, regReturn);

  واپس addr;
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
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_COMPOUND_SELECT
ساکن صحیح multi_Select_By_ملاؤ(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  صحیح i, j;             /* Loop counters */
  Select *p_پیشگی;       /* Another SELECT immediately to our left */
  Select *p_بانٹو;       /* Left-most SELECT in the right-hand group */
  صحیح nSelect;          /* Number of SELECT statements in the compound */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest destA;     /* Destination for coroutine A */
  SelectDest destB;     /* Destination for coroutine B */
  صحیح regAddrA;         /* Address register for select-A coroutine */
  صحیح regAddrB;         /* Address register for select-B coroutine */
  صحیح addrSelectA;      /* Address of the select-A coroutine */
  صحیح addrSelectB;      /* Address of the select-B coroutine */
  صحیح reg_برآمد_A;          /* Address register for the output-A subroutine */
  صحیح reg_برآمد_B;          /* Address register for the output-B subroutine */
  صحیح addr_برآمد_A;         /* Address of the output-A subroutine */
  صحیح addr_برآمد_B = 0;     /* Address of the output-B subroutine */
  صحیح addrEofA;         /* Address of the select-A-exhausted subroutine */
  صحیح addrEofA_noB;     /* Alternate addrEofA if B is uninitialized */
  صحیح addrEofB;         /* Address of the select-B-exhausted subroutine */
  صحیح addrAltB;         /* Address of the A<B subroutine */
  صحیح addrAeqB;         /* Address of the A==B subroutine */
  صحیح addrAgtB;         /* Address of the A>B subroutine */
  صحیح reg_حد_بندی_A;        /* Limit register for select-A */
  صحیح reg_حد_بندی_B;        /* Limit register for select-A */
  صحیح reg_پچھلا;          /* A range of registers to hold previous output */
  صحیح saved_حد_بندی;       /* Saved value of p->iLimit */
  صحیح saved_انحراف;      /* Saved value of p->iOffset */
  صحیح labelCmpr;        /* Label for the start of the merge algorithm */
  صحیح label_انتہا;         /* Label for the end of the overall SELECT stmt */
  صحیح addr1;            /* Jump instructions that get retargeted */
  صحیح op;               /* One of TK_ALL, TK_UNION, TK_EXCEPT, TK_INTERSECT */
  کلید_Info *p_کلید_Dup = 0; /* Comparison information for duplicate removal */
  کلید_Info *p_کلید_ملاؤ;   /* Comparison information for merging rows */
  sqlite3 *db;          /* Database connection */
  اظہاریہ_List *p_ترتیب_By;   /* The ORDER BY clause */
  صحیح n_ترتیب_By;         /* Number of terms in the ORDER BY clause */
  u32 *a_ترتیب_بدلو;        /* Mapping from ORDER BY terms to result set columns */

  أكد( p->p_ترتیب_By!=0 );
  أكد( p_کلید_Dup==0 ); /* "Managed" code needs this.  Ticket #3382. */
  db = pParse->db;
  v = pParse->pVdbe;
  أكد( v!=0 );       /* Already thrown the error if VDBE alloc failed */
  label_انتہا = sqlite3VdbeMakeLabel(pParse);
  labelCmpr = sqlite3VdbeMakeLabel(pParse);


  /* Patch up the ORDER BY clause
  */
  op = p->op; 
  أكد( p->p_پیشگی->p_ترتیب_By==0 );
  p_ترتیب_By = p->p_ترتیب_By;
  أكد( p_ترتیب_By );
  n_ترتیب_By = p_ترتیب_By->n_اظہاریہ;

  /* For operators other than UNION ALL we have to make sure that
  ** the ORDER BY clause covers every term of the result set.  Add
  ** terms to the ORDER BY clause as necessary.  Do the same for UNION ALL
  ** if any operator to the left is not UNION ALL, so that the ORDER BY
  ** carries the collation of every column into that sub-compound.
  */
  برائے(p_پیشگی=p; p_پیشگی && p_پیشگی->op==TK_ALL; p_پیشگی=p_پیشگی->p_پیشگی){}
  اگر( p_پیشگی && p_پیشگی->p_پیشگی ){
    برائے(i=1; db->mallocFailed==0 && i<=p->pEList->n_اظہاریہ; i++){
      ساخت اظہاریہ_List_شے *p_شے;
      برائے(j=0, p_شے=p_ترتیب_By->a; j<n_ترتیب_By; j++, p_شے++){
        أكد( p_شے!=0 );
        أكد( p_شے->u.x.i_ترتیب_By_ستون>0 );
        اگر( p_شے->u.x.i_ترتیب_By_ستون==i ) توڑو;
      }
      اگر( j==n_ترتیب_By ){
        اظہاریہ *pNew = sqlite3_اظہاریہ_Int32(db, i);
        اگر( pNew==0 ) واپس SQLITE_NOMEM_BKPT;
        p->p_ترتیب_By = p_ترتیب_By = sqlite3_اظہاریہ_List_Append(pParse, p_ترتیب_By, pNew);
        اگر( p_ترتیب_By ) p_ترتیب_By->a[n_ترتیب_By++].u.x.i_ترتیب_By_ستون = (u16)i;
      }
    }
  }

  /* Compute the comparison permutation and keyinfo that is used with
  ** the permutation to determine if the next row of results comes
  ** from selectA or selectB.  Also add literal collations to the
  ** ORDER BY clause terms so that when selectA and selectB are
  ** evaluated, they use the correct collation.
  */
  a_ترتیب_بدلو = sqlite3DbMallocRawNN(db, حجم(u32)*(n_ترتیب_By + 1));
  اگر( a_ترتیب_بدلو ){
    ساخت اظہاریہ_List_شے *p_شے;
    صحیح bKeep = 0;
    a_ترتیب_بدلو[0] = n_ترتیب_By;
    برائے(i=1, p_شے=p_ترتیب_By->a; i<=n_ترتیب_By; i++, p_شے++){
      أكد( p_شے!=0 );
      أكد( p_شے->u.x.i_ترتیب_By_ستون>0 );
      أكد( p_شے->u.x.i_ترتیب_By_ستون<=p->pEList->n_اظہاریہ );
      a_ترتیب_بدلو[i] = p_شے->u.x.i_ترتیب_By_ستون - 1;
      اگر( a_ترتیب_بدلو[i]!=(u32)i-1 ) bKeep = 1;
    }
     اگر( bKeep==0 ){
      sqlite3DbFreeNN(db, a_ترتیب_بدلو);
      a_ترتیب_بدلو = 0;
    }
  }
  p_کلید_ملاؤ = multi_Select_By_ملاؤ_کلید_Info(pParse, p, 1);

  /* Allocate a range of temporary registers and the KeyInfo needed
  ** for the logic that removes duplicate result rows when the
  ** operator is UNION, EXCEPT, or INTERSECT (but not UNION ALL).
  */
  اگر( op==TK_ALL ){
    reg_پچھلا = 0;
  }ورنہ{
    صحیح n_اظہاریہ = p->pEList->n_اظہاریہ;
    أكد( n_ترتیب_By>=n_اظہاریہ || db->mallocFailed );
    reg_پچھلا = pParse->nMem+1;
    pParse->nMem += n_اظہاریہ+1;
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, reg_پچھلا);
    p_کلید_Dup = sqlite3_کلید_Info_Alloc(db, n_اظہاریہ, 1);
    اگر( p_کلید_Dup ){
      أكد( sqlite3_کلید_Info_Is_Writeable(p_کلید_Dup) );
      /* Use the ORDER BY collations, not multiSelectCollSeq(p), because
      ** p might be just one part of a larger compound, in which case
      ** the ORDER BY carries the COLLATE of the larger compound. */
      برائے(i=0; ALWAYS(p_کلید_ملاؤ) && i<n_ترتیب_By; i++){
        p_کلید_Dup->aColl[p_ترتیب_By->a[i].u.x.i_ترتیب_By_ستون-1] = p_کلید_ملاؤ->aColl[i];
      }
    }
  }

  /* Separate the left and the right query from one another
  */
  nSelect = 1;
  اگر( (op==TK_ALL || op==TK_UNION)
   && OptimizationEnabled(db, SQLITE_متوازن_ملاؤ)
  ){
    برائے(p_بانٹو=p; p_بانٹو->p_پیشگی!=0 && p_بانٹو->op==op; p_بانٹو=p_بانٹو->p_پیشگی){
      nSelect++;
      أكد( p_بانٹو->p_پیشگی->p_اگلا==p_بانٹو );
    }
  }
  اگر( nSelect<=3 ){
    p_بانٹو = p;
  }ورنہ{
    p_بانٹو = p;
    برائے(i=2; i<nSelect; i+=2){ p_بانٹو = p_بانٹو->p_پیشگی; }
  }
  p_پیشگی = p_بانٹو->p_پیشگی;
  أكد( p_پیشگی!=0 );
  p_بانٹو->p_پیشگی = 0;
  p_پیشگی->p_اگلا = 0;
  أكد( p->p_ترتیب_By == p_ترتیب_By );
  أكد( p_ترتیب_By!=0 || db->mallocFailed );
  p_پیشگی->p_ترتیب_By = sqlite3_اظہاریہ_List_Dup(pParse->db, p_ترتیب_By, 0);
  sqlite3_Resolve_ترتیب_گروہ_By(pParse, p, p->p_ترتیب_By, "ORDER");
  sqlite3_Resolve_ترتیب_گروہ_By(pParse, p_پیشگی, p_پیشگی->p_ترتیب_By, "ORDER");

  /* Compute the limit registers */
  compute_حد_بندی_Registers(pParse, p, label_انتہا);
  اگر( p->i_حد_بندی && (op==TK_ALL || union_With_حد_بندی_One(db,p)) ){
    reg_حد_بندی_A = ++pParse->nMem;
    reg_حد_بندی_B = ++pParse->nMem;
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Copy, p->i_انحراف ? p->i_انحراف+1 : p->i_حد_بندی,
                                  reg_حد_بندی_A);
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Copy, reg_حد_بندی_A, reg_حد_بندی_B);
  }ورنہ{
    reg_حد_بندی_A = reg_حد_بندی_B = 0;
  }
  sqlite3_اظہاریہ_Delete(db, p->p_حد_بندی);
  p->p_حد_بندی = 0;

  regAddrA = ++pParse->nMem;
  regAddrB = ++pParse->nMem;
  reg_برآمد_A = ++pParse->nMem;
  reg_برآمد_B = ++pParse->nMem;
  sqlite3SelectDestInit(&destA, SRT_Coroutine, regAddrA);
  sqlite3SelectDestInit(&destB, SRT_Coroutine, regAddrB);

  ExplainQueryPlan((pParse, 1, "MERGE (%s)", sqlite3SelectOpName(p->op)));

  /* Generate a coroutine to evaluate the SELECT statement to the
  ** left of the compound operator - the "A" select.
  */
  addrSelectA = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3_Vdbe_جوڑو_Op3(v, OP_InitCoroutine, regAddrA, 0, addrSelectA);
  VdbeComment((v, "SUBR: next-A"));
  p_پیشگی->i_حد_بندی = reg_حد_بندی_A;
  ExplainQueryPlan((pParse, 1, "LEFT"));
  sqlite3Select(pParse, p_پیشگی, &destA);
  sqlite3_Vdbe_انتہا_Coroutine(v, regAddrA);
  sqlite3_Vdbe_Jump_یہاں(v, addr1);

  /* Generate a coroutine to evaluate the SELECT statement on
  ** the right - the "B" select
  */
  addrSelectB = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3_Vdbe_جوڑو_Op3(v, OP_InitCoroutine, regAddrB, 0, addrSelectB);
  VdbeComment((v, "SUBR: next-B"));
  saved_حد_بندی = p->i_حد_بندی;
  saved_انحراف = p->i_انحراف;
  p->i_حد_بندی = reg_حد_بندی_B;
  p->i_انحراف = 0; 
  ExplainQueryPlan((pParse, 1, "RIGHT"));
  sqlite3Select(pParse, p, &destB);
  p->i_حد_بندی = saved_حد_بندی;
  p->i_انحراف = saved_انحراف;
  sqlite3_Vdbe_انتہا_Coroutine(v, regAddrB);

  /* Generate a subroutine that outputs the current row of the A
  ** select as the next output row of the compound select.
  */
  VdbeNoopComment((v, "SUBR: out-A"));
  addr_برآمد_A = generateOutputSubroutine(pParse,
                 p, &destA, pDest, reg_برآمد_A,
                 reg_پچھلا, p_کلید_Dup, label_انتہا);
 
  /* Generate a subroutine that outputs the current row of the B
  ** select as the next output row of the compound select.
  */
  اگر( op==TK_ALL || op==TK_UNION ){
    VdbeNoopComment((v, "SUBR: out-B"));
    addr_برآمد_B = generateOutputSubroutine(pParse,
                 p, &destB, pDest, reg_برآمد_B,
                 reg_پچھلا, p_کلید_Dup, label_انتہا);
  }
  sqlite3_کلید_Info_Unref(p_کلید_Dup);

  /* Generate a subroutine to run when the results from select A
  ** are exhausted and only data in select B remains.
  */
  اگر( op==TK_EXCEPT || op==TK_INTERSECT ){
    addrEofA_noB = addrEofA = label_انتہا;
  }ورنہ{ 
    VdbeNoopComment((v, "SUBR: eof-A"));
    addrEofA = sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, reg_برآمد_B, addr_برآمد_B);
    VdbeComment((v, "out-B"));
    addrEofA_noB = sqlite3_Vdbe_جوڑو_Op2(v, OP_Yield, regAddrB, label_انتہا);
                                     VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, addrEofA);
    p->n_Select_صف = sqlite3_Log_Est_جوڑو(p->n_Select_صف, p_پیشگی->n_Select_صف);
  }

  /* Generate a subroutine to run when the results from select B
  ** are exhausted and only data in select A remains.
  */
  اگر( op==TK_INTERSECT ){
    addrEofB = addrEofA;
    اگر( p->n_Select_صف > p_پیشگی->n_Select_صف ) p->n_Select_صف = p_پیشگی->n_Select_صف;
  }ورنہ{ 
    VdbeNoopComment((v, "SUBR: eof-B"));
    addrEofB = sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, reg_برآمد_A, addr_برآمد_A);
    VdbeComment((v, "out-A"));
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Yield, regAddrA, label_انتہا); VdbeCoverage(v);
    VdbeComment((v, "next-A"));
    sqlite3VdbeGoto(v, addrEofB);
  }

  /* Generate code to handle the case of A<B
  */
  addrAltB = sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, reg_برآمد_A, addr_برآمد_A);
  VdbeComment((v, "out-A"));
  sqlite3_Vdbe_جوڑو_Op2(v, OP_Yield, regAddrA, addrEofA); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  sqlite3VdbeGoto(v, labelCmpr);

  /* Generate code to handle the case of A==B
  */
  اگر( op==TK_ALL ){
    addrAeqB = addrAltB;
  }ورنہ اگر( op==TK_INTERSECT ){
    addrAeqB = addrAltB;
    addrAltB++;
  }ورنہ{
    addrAeqB = addrAltB + 1;
  }

  /* Generate code to handle the case of A>B
  */
  addrAgtB = sqlite3VdbeCurrentAddr(v);
  اگر( op==TK_ALL || op==TK_UNION ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, reg_برآمد_B, addr_برآمد_B);
    VdbeComment((v, "out-B"));
    sqlite3_Vdbe_جوڑو_Op2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, labelCmpr);
  }ورنہ{
    addrAgtB++; /* Just do next-B.  Might as well use the next-B call
                ** in the next code block */
  }

  /* This code runs once to initialize everything.
  */
  sqlite3_Vdbe_Jump_یہاں(v, addr1);
  sqlite3_Vdbe_جوڑو_Op2(v, OP_Yield, regAddrA, addrEofA_noB); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  /* v---  Also the A>B case for EXCEPT and INTERSECT */
  sqlite3_Vdbe_جوڑو_Op2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
  VdbeComment((v, "next-B"));

  /* Implement the main merge loop
  */
  اگر( a_ترتیب_بدلو!=0 ){
    sqlite3_Vdbe_جوڑو_Op4(v, OP_Permutation, 0, 0, 0, (حرف*)a_ترتیب_بدلو, P4_INTARRAY);
  }
  sqlite3VdbeResolveLabel(v, labelCmpr);
  sqlite3_Vdbe_جوڑو_Op4(v, OP_Compare, destA.iSdst, destB.iSdst, n_ترتیب_By,
                         (حرف*)p_کلید_ملاؤ, P4_KEYINFO);
  اگر( a_ترتیب_بدلو!=0 ){
    sqlite3VdbeChangeP5(v, OPFLAG_ترتیب_بدلو);
  }
  sqlite3_Vdbe_جوڑو_Op3(v, OP_Jump, addrAltB, addrAeqB, addrAgtB);
  VdbeCoverageIf(v, op==TK_ALL);
  VdbeCoverageIf(v, op==TK_UNION);
  VdbeCoverageIf(v, op==TK_EXCEPT);
  VdbeCoverageIf(v, op==TK_INTERSECT);

  /* Jump to the this point in order to terminate the query.
  */
  sqlite3VdbeResolveLabel(v, label_انتہا);

  /* Make arrangements to free the 2nd and subsequent arms of the compound
  ** after the parse has finished */
  اگر( p_بانٹو->p_پیشگی ){
    sqlite3_Parser_جوڑو_Cleanup(pParse, sqlite3SelectDeleteGeneric, p_بانٹو->p_پیشگی);
  }
  p_بانٹو->p_پیشگی = p_پیشگی;
  p_پیشگی->p_اگلا = p_بانٹو;
  sqlite3_اظہاریہ_List_Delete(db, p_پیشگی->p_ترتیب_By);
  p_پیشگی->p_ترتیب_By = 0;

  /*** TBD:  Insert subroutine calls to close cursors on incomplete
  **** subqueries ****/
  Explain_Query_Plan_نکالو(pParse);
  واپس pParse->nErr!=0;
}
#اگر_ختم

#اگر !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)

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
قسمتعریف ساخت SubstContext {
  Parse *pParse;            /* The parsing context */
  صحیح i_جدول;               /* Replace references to this table */
  صحیح i_New_جدول;            /* New table number */
  صحیح isOuterJoin;          /* Add TK_IF_NULL_ROW opcodes on each replacement */
  صحیح n_Sel_گہرائی;            /* Depth of sub-query recursion.  Top==1 */
  اظہاریہ_List *pEList;         /* Replacement expressions */
  اظہاریہ_List *pCList;         /* Collation sequences for replacement expr */
} SubstContext;

/* Forward Declarations */
ساکن خالی subst_اظہاریہ_List(SubstContext*, اظہاریہ_List*);
ساکن خالی substSelect(SubstContext*, Select*, صحیح);

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
ساکن اظہاریہ *subst_اظہاریہ(
  SubstContext *pSubst,  /* Description of the substitution */
  اظہاریہ *p_اظہاریہ            /* Expr in which substitution occurs */
){
  اگر( p_اظہاریہ==0 ) واپس 0;
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON|EP_InnerON)
   && p_اظہاریہ->w.iJoin==pSubst->i_جدول
  ){
    testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_InnerON) );
    p_اظہاریہ->w.iJoin = pSubst->i_New_جدول;
  }
  اگر( p_اظہاریہ->op==TK_عمود
   && p_اظہاریہ->i_جدول==pSubst->i_جدول
   && !اظہاریہ_Has_Property(p_اظہاریہ, EP_Fixed_ستون)
  ){
#اگر_تعریف_شدہ SQLITE_ALLOW_ROWID_اندر_VIEW
    اگر( p_اظہاریہ->i_عمود<0 ){
      p_اظہاریہ->op = TK_NULL;
    }ورنہ
#اگر_ختم
    {
      اظہاریہ *pNew;
      صحیح i_عمود;
      اظہاریہ *pCopy;
      اظہاریہ if_Null_صف;
      i_عمود = p_اظہاریہ->i_عمود;
      أكد( i_عمود>=0 );
      أكد( pSubst->pEList!=0 && i_عمود<pSubst->pEList->n_اظہاریہ );
      أكد( p_اظہاریہ->p_دایاں==0 );
      pCopy = pSubst->pEList->a[i_عمود].p_اظہاریہ;
      اگر( sqlite3_اظہاریہ_Is_سمتیہ(pCopy) ){
        sqlite3_سمتیہ_Error_پیغام(pSubst->pParse, pCopy);
      }ورنہ{
        sqlite3 *db = pSubst->pParse->db;
        اگر( pSubst->isOuterJoin
         && (pCopy->op!=TK_عمود || pCopy->i_جدول!=pSubst->i_New_جدول)
        ){
          memset(&if_Null_صف, 0, حجم(if_Null_صف));
          if_Null_صف.op = TK_IF_NULL_صف;
          if_Null_صف.p_بایاں = pCopy;
          if_Null_صف.i_جدول = pSubst->i_New_جدول;
          if_Null_صف.i_عمود = -99;
          if_Null_صف.flags = EP_If_Null_صف;
          pCopy = &if_Null_صف;
        }
        testcase( اظہاریہ_Has_Property(pCopy, EP_Subquery) );
        pNew = sqlite3_اظہاریہ_Dup(db, pCopy, 0);
        اگر( db->mallocFailed ){
          sqlite3_اظہاریہ_Delete(db, pNew);
          واپس p_اظہاریہ;
        }
        اگر( pSubst->isOuterJoin ){
          اظہاریہ_سیٹ_Property(pNew, EP_CanBeNull);
        }
        اگر( pNew->op==TK_TRUEFALSE ){
          pNew->u.i_قدر = sqlite3_اظہاریہ_Truth_قدر(pNew);
          pNew->op = TK_INTEGER;
          اظہاریہ_سیٹ_Property(pNew, EP_Int_قدر);
        }

        /* Ensure that the expression now has an implicit collation sequence,
        ** just as it did when it was a column of a view or sub-query. */
        {
          CollSeq *pNat = sqlite3_اظہاریہ_Coll_Seq(pSubst->pParse, pNew);
          CollSeq *pColl = sqlite3_اظہاریہ_Coll_Seq(pSubst->pParse,
                pSubst->pCList->a[i_عمود].p_اظہاریہ
          );
          اگر( pNat!=pColl || (pNew->op!=TK_عمود && pNew->op!=TK_COLLATE) ){
            pNew = sqlite3_اظہاریہ_جوڑو_Collate_String(pSubst->pParse, pNew,
                (pColl ? pColl->zName : "BINARY")
            );
          }
        }
        اظہاریہ_Clear_Property(pNew, EP_Collate);
        اگر( اظہاریہ_Has_Property(p_اظہاریہ,EP_OuterON|EP_InnerON) ){
          sqlite3_سیٹ_Join_اظہاریہ(pNew, p_اظہاریہ->w.iJoin,
                             p_اظہاریہ->flags & (EP_OuterON|EP_InnerON));
        }
        sqlite3_اظہاریہ_Delete(db, p_اظہاریہ);
        p_اظہاریہ = pNew;
      }
    }
  }ورنہ{
    اگر( p_اظہاریہ->op==TK_IF_NULL_صف && p_اظہاریہ->i_جدول==pSubst->i_جدول ){
      p_اظہاریہ->i_جدول = pSubst->i_New_جدول;
    }
    اگر( p_اظہاریہ->op==TK_AGG_FUNCTION && p_اظہاریہ->op2>=pSubst->n_Sel_گہرائی ){
      p_اظہاریہ->op2--;
    }
    p_اظہاریہ->p_بایاں = subst_اظہاریہ(pSubst, p_اظہاریہ->p_بایاں);
    p_اظہاریہ->p_دایاں = subst_اظہاریہ(pSubst, p_اظہاریہ->p_دایاں);
    اگر( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
      substSelect(pSubst, p_اظہاریہ->x.pSelect, 1);
    }ورنہ{
      subst_اظہاریہ_List(pSubst, p_اظہاریہ->x.pList);
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
      کھڑکی *pWin = p_اظہاریہ->y.pWin;
      pWin->pFilter = subst_اظہاریہ(pSubst, pWin->pFilter);
      subst_اظہاریہ_List(pSubst, pWin->p_تقسیم);
      subst_اظہاریہ_List(pSubst, pWin->p_ترتیب_By);
    }
#اگر_ختم
  }
  واپس p_اظہاریہ;
}
ساکن خالی subst_اظہاریہ_List(
  SubstContext *pSubst, /* Description of the substitution */
  اظہاریہ_List *pList       /* List to scan and in which to make substitutes */
){
  صحیح i;
  اگر( pList==0 ) واپس;
  برائے(i=0; i<pList->n_اظہاریہ; i++){
    pList->a[i].p_اظہاریہ = subst_اظہاریہ(pSubst, pList->a[i].p_اظہاریہ);
  }
}
ساکن خالی substSelect(
  SubstContext *pSubst, /* Description of the substitution */
  Select *p,            /* SELECT statement in which to make substitutions */
  صحیح do_پیشگی           /* Do substitutes on p->pPrior too */
){
  SrcList *pSrc;
  Src_شے *p_شے;
  صحیح i;
  اگر( !p ) واپس;
  pSubst->n_Sel_گہرائی++;
  کرو{
    subst_اظہاریہ_List(pSubst, p->pEList);
    subst_اظہاریہ_List(pSubst, p->p_گروہ_By);
    subst_اظہاریہ_List(pSubst, p->p_ترتیب_By);
    p->pHaving = subst_اظہاریہ(pSubst, p->pHaving);
    p->pWhere = subst_اظہاریہ(pSubst, p->pWhere);
    pSrc = p->pSrc;
    أكد( pSrc!=0 );
    برائے(i=pSrc->nSrc, p_شے=pSrc->a; i>0; i--, p_شے++){
      اگر( p_شے->fg.isSubquery ){
        substSelect(pSubst, p_شے->u4.pSubq->pSelect, 1);
      }
      اگر( p_شے->fg.isTabFunc ){
        subst_اظہاریہ_List(pSubst, p_شے->u1.pFuncArg);
      }
    }
  }جب( do_پیشگی && (p = p->p_پیشگی)!=0 );
  pSubst->n_Sel_گہرائی--;
}
#اگر_ختم /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#اگر !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
/*
** pSelect is a SELECT statement and pSrcItem is one item in the FROM
** clause of that SELECT.
**
** This routine scans the entire SELECT statement and recomputes the
** pSrcItem->colUsed mask.
*/
ساکن صحیح recompute_Columns_مستعمل_اظہاریہ(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  Src_شے *p_شے;
  اگر( p_اظہاریہ->op!=TK_عمود ) واپس WRC_Continue;
  p_شے = pWalker->u.p_Src_شے;
  اگر( p_شے->iCursor!=p_اظہاریہ->i_جدول ) واپس WRC_Continue;
  اگر( p_اظہاریہ->i_عمود<0 ) واپس WRC_Continue;
  p_شے->ستون_مستعمل |= sqlite3_اظہاریہ_ستون_مستعمل(p_اظہاریہ);
  واپس WRC_Continue;
}
ساکن خالی recompute_Columns_مستعمل(
  Select *pSelect,                 /* The complete SELECT statement */
  Src_شے *p_Src_شے                /* Which FROM clause item to recompute */
){
  Walker w;
  اگر( NEVER(p_Src_شے->pSTab==0) ) واپس;
  memset(&w, 0, حجم(w));
  w.x_اظہاریہ_Callback = recompute_Columns_مستعمل_اظہاریہ;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.p_Src_شے = p_Src_شے;
  p_Src_شے->ستون_مستعمل = 0;
  sqlite3WalkSelect(&w, pSelect);
}
#اگر_ختم /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#اگر !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
ساکن خالی srclistRenumberCursors(
  Parse *pParse,                  /* Parse context */
  صحیح *aCsrMap,                   /* Array to store cursor mappings in */
  SrcList *pSrc,                  /* FROM clause to renumber */
  صحیح iExcept                     /* FROM clause item to skip */
){
  صحیح i;
  Src_شے *p_شے;
  برائے(i=0, p_شے=pSrc->a; i<pSrc->nSrc; i++, p_شے++){
    اگر( i!=iExcept ){
      Select *p;
      أكد( p_شے->iCursor < aCsrMap[0] );
      اگر( !p_شے->fg.isRecursive || aCsrMap[p_شے->iCursor+1]==0 ){
        aCsrMap[p_شے->iCursor+1] = pParse->nTab++;
      }
      p_شے->iCursor = aCsrMap[p_شے->iCursor+1];
      اگر( p_شے->fg.isSubquery ){
        برائے(p=p_شے->u4.pSubq->pSelect; p; p=p->p_پیشگی){
          srclistRenumberCursors(pParse, aCsrMap, p->pSrc, -1);
        }
      }
    }
  }
}

/*
** *piCursor is a cursor number.  Change it if it needs to be mapped.
*/
ساکن خالی renumberCursorDoMapping(Walker *pWalker, صحیح *piCursor){
  صحیح *aCsrMap = pWalker->u.ai_ستون;
  صحیح iCsr = *piCursor;
  اگر( iCsr < aCsrMap[0] && aCsrMap[iCsr+1]>0 ){
    *piCursor = aCsrMap[iCsr+1];
  }
}

/*
** Expression walker callback used by renumberCursors() to update
** Expr objects to match newly assigned cursor numbers.
*/
ساکن صحیح renumberCursorsCb(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  صحیح op = p_اظہاریہ->op;
  اگر( op==TK_عمود || op==TK_IF_NULL_صف ){
    renumberCursorDoMapping(pWalker, &p_اظہاریہ->i_جدول);
  }
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, (EP_OuterON|EP_InnerON)) ){
    testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) );
    testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_InnerON) );
    renumberCursorDoMapping(pWalker, &p_اظہاریہ->w.iJoin);
  }
  واپس WRC_Continue;
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
ساکن خالی renumberCursors(
  Parse *pParse,                  /* Parse context */
  Select *p,                      /* Select to renumber cursors within */
  صحیح iExcept,                    /* FROM clause item to skip */
  صحیح *aCsrMap                    /* Working space */
){
  Walker w;
  srclistRenumberCursors(pParse, aCsrMap, p->pSrc, iExcept);
  memset(&w, 0, حجم(w));
  w.u.ai_ستون = aCsrMap;
  w.x_اظہاریہ_Callback = renumberCursorsCb;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  sqlite3WalkSelect(&w, p);
}
#اگر_ختم /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** If pSel is not part of a compound SELECT, return a pointer to its
** expression list. Otherwise, return a pointer to the expression list
** of the leftmost SELECT in the compound.
*/
ساکن اظہاریہ_List *ڈھونڈو_Leftmost_Exprlist(Select *pSel){
  جب( pSel->p_پیشگی ){
    pSel = pSel->p_پیشگی;
  }
  واپس pSel->pEList;
}

/*
** Return true if any of the result-set columns in the compound query
** have incompatible affinities on one or more arms of the compound.
*/
ساکن صحیح compoundHasDifferentAffinities(Select *p){
  صحیح ii;
  اظہاریہ_List *pList;
  أكد( p!=0 );
  أكد( p->pEList!=0 );
  أكد( p->p_پیشگی!=0 );
  pList = p->pEList;
  برائے(ii=0; ii<pList->n_اظہاریہ; ii++){
    حرف aff;
    Select *pSub1;
    أكد( pList->a[ii].p_اظہاریہ!=0 );
    aff = sqlite3_اظہاریہ_Affinity(pList->a[ii].p_اظہاریہ);
    برائے(pSub1=p->p_پیشگی; pSub1; pSub1=pSub1->p_پیشگی){
      أكد( pSub1->pEList!=0 );
      أكد( pSub1->pEList->n_اظہاریہ>ii );
      أكد( pSub1->pEList->a[ii].p_اظہاریہ!=0 );
      اگر( sqlite3_اظہاریہ_Affinity(pSub1->pEList->a[ii].p_اظہاریہ)!=aff ){
        واپس 1;
      }
    }
  }
  واپس 0;
}

#اگر !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
ساکن صحیح flattenSubquery(
  Parse *pParse,       /* Parsing context */
  Select *p,           /* The parent or outer SELECT statement */
  صحیح iFrom,           /* Index in p->pSrc->a[] of the inner subquery */
  صحیح isAgg            /* True if outer SELECT uses aggregate functions */
){
  ثابت حرف *zSavedAuthContext = pParse->zAuthContext;
  Select *p_والد;    /* Current UNION ALL term of the other query */
  Select *pSub;       /* The inner query or "subquery" */
  Select *pSub1;      /* Pointer to the rightmost select in sub-query */
  SrcList *pSrc;      /* The FROM clause of the outer query */
  SrcList *pSubSrc;   /* The FROM clause of the subquery */
  صحیح i_والد;        /* VDBE cursor number of the pSub result set temp table */
  صحیح i_New_والد = -1;/* Replacement table for iParent */
  صحیح isOuterJoin = 0; /* True if pSub is the right side of a LEFT JOIN */   
  صحیح i;              /* Loop counter */
  اظہاریہ *pWhere;                    /* The WHERE clause */
  Src_شے *pSubitem;               /* The subquery */
  sqlite3 *db = pParse->db;
  Walker w;                        /* Walker to persist agginfo data */
  صحیح *aCsrMap = 0;

  /* Check to see if flattening is permitted.  Return 0 if not.
  */
  أكد( p!=0 );
  أكد( p->p_پیشگی==0 );
  اگر( OptimizationDisabled(db, SQLITE_QueryFlattener) ) واپس 0;
  pSrc = p->pSrc;
  أكد( pSrc && iFrom>=0 && iFrom<pSrc->nSrc );
  pSubitem = &pSrc->a[iFrom];
  i_والد = pSubitem->iCursor;
  أكد( pSubitem->fg.isSubquery );
  pSub = pSubitem->u4.pSubq->pSelect;
  أكد( pSub!=0 );

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
  اگر( p->pWin || pSub->pWin ) واپس 0;                  /* Restriction (25) */
#اگر_ختم

  pSubSrc = pSub->pSrc;
  أكد( pSubSrc );
  /* Prior to version 3.1.2, when LIMIT and OFFSET had to be simple constants,
  ** not arbitrary expressions, we allowed some combining of LIMIT and OFFSET
  ** because they could be computed at compile-time.  But when LIMIT and OFFSET
  ** became arbitrary expressions, we were forced to add restrictions (13)
  ** and (14). */
  اگر( pSub->p_حد_بندی && p->p_حد_بندی ) واپس 0;              /* Restriction (13) */
  اگر( pSub->p_حد_بندی && pSub->p_حد_بندی->p_دایاں ) واپس 0;   /* Restriction (14) */
  اگر( (p->selFlags & SF_Compound)!=0 && pSub->p_حد_بندی ){
    واپس 0;                                            /* Restriction (15) */
  }
  اگر( pSubSrc->nSrc==0 ) واپس 0;                       /* Restriction (7)  */
  اگر( pSub->selFlags & SF_Distinct ) واپس 0;           /* Restriction (4)  */
  اگر( pSub->p_حد_بندی && (pSrc->nSrc>1 || isAgg) ){
     واپس 0;         /* Restrictions (8)(9) */
  }
  اگر( p->p_ترتیب_By && pSub->p_ترتیب_By ){
     واپس 0;                                           /* Restriction (11) */
  }
  اگر( isAgg && pSub->p_ترتیب_By ) واپس 0;                /* Restriction (16) */
  اگر( pSub->p_حد_بندی && p->pWhere ) واپس 0;              /* Restriction (19) */
  اگر( pSub->p_حد_بندی && (p->selFlags & SF_Distinct)!=0 ){
     واپس 0;         /* Restriction (21) */
  }
  اگر( pSub->selFlags & (SF_Recursive) ){
    واپس 0; /* Restrictions (22) */
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
  اگر( (pSubitem->fg.jointype & (JT_OUTER|JT_LTORJ))!=0 ){
    اگر( pSubSrc->nSrc>1                        /* (3a) */
     /**** || IsVirtual(pSubSrc->a[0].pSTab)      (3b)-omitted */
     || (p->selFlags & SF_Distinct)!=0         /* (3d) */
     || (pSubitem->fg.jointype & JT_دایاں)!=0  /* (26) */
    ){
      واپس 0;
    }
    isOuterJoin = 1;
  }

  أكد( pSubSrc->nSrc>0 );  /* True by restriction (7) */
  اگر( iFrom>0 && (pSubSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
    واپس 0;   /* Restriction (27a) */
  }

  /* Condition (28) is blocked by the caller */
  أكد( !pSubitem->fg.isCte || pSubitem->u2.pCteUse->eM10d!=M10d_Yes );

  /* Restriction (17): If the sub-query is a compound SELECT, then it must
  ** use only the UNION ALL operator. And none of the simple select queries
  ** that make up the compound SELECT are allowed to be aggregate or distinct
  ** queries.
  */
  اگر( pSub->p_پیشگی ){
    صحیح ii;
    اگر( pSub->p_ترتیب_By ){
      واپس 0;  /* Restriction (20) */
    }
    اگر( isAgg || (p->selFlags & SF_Distinct)!=0 || isOuterJoin>0 ){
      واپس 0; /* (17d1), (17d2), or (17f) */
    }
    برائے(pSub1=pSub; pSub1; pSub1=pSub1->p_پیشگی){
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct );
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Aggregate );
      أكد( pSub->pSrc!=0 );
      أكد( (pSub->selFlags & SF_Recursive)==0 );
      أكد( pSub->pEList->n_اظہاریہ==pSub1->pEList->n_اظہاریہ );
      اگر( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))!=0    /* (17b) */
       || (pSub1->p_پیشگی && pSub1->op!=TK_ALL)                 /* (17a) */
       || pSub1->pSrc->nSrc<1                                  /* (17c) */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
       || pSub1->pWin                                          /* (17e) */
#اگر_ختم
      ){
        واپس 0;
      }
      اگر( iFrom>0 && (pSub1->pSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
        /* Without this restriction, the JT_LTORJ flag would end up being
        ** omitted on left-hand tables of the right join that is being
        ** flattened. */
        واپس 0;   /* Restrictions (17g), (27b) */
      }
      testcase( pSub1->pSrc->nSrc>1 );
    }

    /* Restriction (18). */
    اگر( p->p_ترتیب_By ){
      برائے(ii=0; ii<p->p_ترتیب_By->n_اظہاریہ; ii++){
        اگر( p->p_ترتیب_By->a[ii].u.x.i_ترتیب_By_ستون==0 ) واپس 0;
      }
    }

    /* Restriction (23) */
    اگر( (p->selFlags & SF_Recursive) ) واپس 0;

    /* Restriction (17h) */
    اگر( compoundHasDifferentAffinities(pSub) ) واپس 0;

    اگر( pSrc->nSrc>1 ){
      اگر( pParse->nSelect>500 ) واپس 0;
      اگر( OptimizationDisabled(db, SQLITE_FlttnUnionAll) ) واپس 0;
      aCsrMap = sqlite3DbMallocZero(db, ((i64)pParse->nTab+1)*حجم(صحیح));
      اگر( aCsrMap ) aCsrMap[0] = pParse->nTab;
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
  
  اگر( ALWAYS(pSubitem->fg.isSubquery) ){
    pSub1 = sqlite3SubqueryDetach(db, pSubitem);
  }ورنہ{
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
  برائے(pSub=pSub->p_پیشگی; pSub; pSub=pSub->p_پیشگی){
    Select *pNew;
    اظہاریہ_List *p_ترتیب_By = p->p_ترتیب_By;
    اظہاریہ *p_حد_بندی = p->p_حد_بندی;
    Select *p_پیشگی = p->p_پیشگی;
    جدول *p_شے_Tab = pSubitem->pSTab;
    pSubitem->pSTab = 0;
    p->p_ترتیب_By = 0;
    p->p_پیشگی = 0;
    p->p_حد_بندی = 0;
    pNew = sqlite3SelectDup(db, p, 0);
    p->p_حد_بندی = p_حد_بندی;
    p->p_ترتیب_By = p_ترتیب_By;
    p->op = TK_ALL;
    pSubitem->pSTab = p_شے_Tab;
    اگر( pNew==0 ){
      p->p_پیشگی = p_پیشگی;
    }ورنہ{
      pNew->selId = ++pParse->nSelect;
      اگر( aCsrMap && ALWAYS(db->mallocFailed==0) ){
        renumberCursors(pParse, pNew, iFrom, aCsrMap);
      }
      pNew->p_پیشگی = p_پیشگی;
      اگر( p_پیشگی ) p_پیشگی->p_اگلا = pNew;
      pNew->p_اگلا = p;
      p->p_پیشگی = pNew;
      TREETRACE(0x4,pParse,p,("compound-subquery flattener"
                              " creates %u as peer\n",pNew->selId));
    }
    أكد( pSubitem->fg.isSubquery==0 );
  }
  sqlite3DbFree(db, aCsrMap);
  اگر( db->mallocFailed ){
    أكد( pSubitem->fg.fixedSchema==0 );
    أكد( pSubitem->fg.isSubquery==0 );
    أكد( pSubitem->u4.zDatabase==0 );
    sqlite3_Src_شے_Attach_Subquery(pParse, pSubitem, pSub1, 0);
    واپس 1;
  }

  /* Defer deleting the Table object associated with the
  ** subquery until code generation is
  ** complete, since there may still exist Expr.pTab entries that
  ** refer to the subquery even after flattening.  Ticket #3346.
  **
  ** pSubitem->pSTab is always non-NULL by test restrictions and tests above.
  */
  اگر( ALWAYS(pSubitem->pSTab!=0) ){
    جدول *p_Tab_مقصد_Del = pSubitem->pSTab;
    اگر( p_Tab_مقصد_Del->nTabRef==1 ){
      Parse *pToplevel = sqlite3ParseToplevel(pParse);
      sqlite3_Parser_جوڑو_Cleanup(pToplevel, sqlite3_Delete_جدول_Generic, p_Tab_مقصد_Del);
      testcase( pToplevel->earlyCleanup );
    }ورنہ{
      p_Tab_مقصد_Del->nTabRef--;
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
  برائے(p_والد=p; p_والد; p_والد=p_والد->p_پیشگی, pSub=pSub->p_پیشگی){
    صحیح nSubSrc;
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
    اگر( nSubSrc>1 ){
      pSrc = sqlite3SrcListEnlarge(pParse, pSrc, nSubSrc-1,iFrom+1);
      اگر( pSrc==0 ) توڑو;
      p_والد->pSrc = pSrc;
    }

    /* Transfer the FROM clause terms from the subquery into the
    ** outer query.
    */
    i_New_والد = pSubSrc->a[0].iCursor;
    برائے(i=0; i<nSubSrc; i++){
      Src_شے *p_شے = &pSrc->a[i+iFrom];
      أكد( p_شے->fg.isTabFunc==0 );
      أكد( p_شے->fg.isSubquery
           || p_شے->fg.fixedSchema
           || p_شے->u4.zDatabase==0 );
      اگر( p_شے->fg.isUsing ) sqlite3IdListDelete(db, p_شے->u3.pUsing);
      *p_شے = pSubSrc->a[i];
      p_شے->fg.jointype |= (jointype & JT_LTORJ);
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
    اگر( pSub->p_ترتیب_By ){
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
      اظہاریہ_List *p_ترتیب_By = pSub->p_ترتیب_By;
      برائے(i=0; i<p_ترتیب_By->n_اظہاریہ; i++){
        p_ترتیب_By->a[i].u.x.i_ترتیب_By_ستون = 0;
      }
      أكد( p_والد->p_ترتیب_By==0 );
      p_والد->p_ترتیب_By = p_ترتیب_By;
      pSub->p_ترتیب_By = 0;
    }
    pWhere = pSub->pWhere;
    pSub->pWhere = 0;
    اگر( isOuterJoin>0 ){
      أكد( pSubSrc->nSrc==1 );
      sqlite3_سیٹ_Join_اظہاریہ(pWhere, i_New_والد, EP_OuterON);
    }
    اگر( pWhere ){
      اگر( p_والد->pWhere ){
        p_والد->pWhere = sqlite3PExpr(pParse, TK_AND, pWhere, p_والد->pWhere);
      }ورنہ{
        p_والد->pWhere = pWhere;
      }
    }
    اگر( db->mallocFailed==0 ){
      SubstContext x;
      x.pParse = pParse;
      x.i_جدول = i_والد;
      x.i_New_جدول = i_New_والد;
      x.isOuterJoin = isOuterJoin;
      x.n_Sel_گہرائی = 0;
      x.pEList = pSub->pEList;
      x.pCList = ڈھونڈو_Leftmost_Exprlist(pSub);
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
    اگر( pSub->p_حد_بندی ){
      p_والد->p_حد_بندی = pSub->p_حد_بندی;
      pSub->p_حد_بندی = 0;
    }

    /* Recompute the SrcItem.colUsed masks for the flattened
    ** tables. */
    برائے(i=0; i<nSubSrc; i++){
      recompute_Columns_مستعمل(p_والد, &pSrc->a[i+iFrom]);
    }
  }

  /* Finally, delete what is left of the subquery and return success.
  */
  sqlite3AggInfoPersistWalkerInit(&w, pParse);
  sqlite3WalkSelect(&w,pSub1);
  sqlite3SelectDelete(db, pSub1);

#اگر TREETRACE_ENABLED
  اگر( sqlite3_درخت_Trace & 0x4 ){
    TREETRACE(0x4,pParse,p,("After flattening:\n"));
    sqlite3_درخت_View_Select(0, p, 0);
  }
#اگر_ختم

  واپس 1;
}
#اگر_ختم /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** A structure to keep track of all of the column values that are fixed to
** a known value due to WHERE clause constraints of the form COLUMN=VALUE.
*/
قسمتعریف ساخت WhereConst WhereConst;
ساخت WhereConst {
  Parse *pParse;   /* Parsing context */
  u8 *pOomFault;   /* Pointer to pParse->db->mallocFailed */
  صحیح nConst;      /* Number for COLUMN=CONSTANT terms */
  صحیح nChng;       /* Number of times a constant is propagated */
  صحیح bHasAffBlob; /* At least one column in apExpr[] as affinity BLOB */
  u32 mExcludeOn;  /* Which ON expressions to exclude from considertion.
                   ** Either EP_OuterON or EP_InnerON|EP_OuterON */
  اظہاریہ **ap_اظہاریہ;   /* [i*2] is COLUMN and [i*2+1] is VALUE */
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
ساکن خالی const_داخل_کرو(
  WhereConst *pConst,  /* The WhereConst into which we are inserting */
  اظہاریہ *p_عمود,       /* The COLUMN part of the constraint */
  اظہاریہ *p_قدر,        /* The VALUE part of the constraint */
  اظہاریہ *p_اظہاریہ          /* Overall expression: COLUMN=VALUE or VALUE=COLUMN */
){
  صحیح i;
  أكد( p_عمود->op==TK_عمود );
  أكد( sqlite3_اظہاریہ_Is_Constant(pConst->pParse, p_قدر) );
  اگر( اظہاریہ_Has_Property(p_عمود, EP_Fixed_ستون) ) واپس;
  اگر( sqlite3_اظہاریہ_Affinity(p_قدر)!=0 ) واپس;
  اگر( !sqlite3_Is_ثنائی(sqlite3_اظہاریہ_Compare_Coll_Seq(pConst->pParse,p_اظہاریہ)) ){
    واپس;
  }
  اگر( sqlite3_اظہاریہ_Can_Return_Subtype(pConst->pParse, p_قدر) ) واپس;

  /* 2018-10-25 ticket [cf5ed20f]
  ** Make sure the same pColumn is not inserted more than once */
  برائے(i=0; i<pConst->nConst; i++){
    ثابت اظہاریہ *pE2 = pConst->ap_اظہاریہ[i*2];
    أكد( pE2->op==TK_عمود );
    اگر( pE2->i_جدول==p_عمود->i_جدول
     && pE2->i_عمود==p_عمود->i_عمود
    ){
      واپس;  /* Already present.  Return without doing anything. */
    }
  }
  أكد( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  اگر( sqlite3_اظہاریہ_Affinity(p_عمود)<=SQLITE_AFF_BLOB ){
    pConst->bHasAffBlob = 1;
  }

  pConst->nConst++;
  pConst->ap_اظہاریہ = sqlite3DbReallocOrFree(pConst->pParse->db, pConst->ap_اظہاریہ,
                         pConst->nConst*2*حجم(اظہاریہ*));
  اگر( pConst->ap_اظہاریہ==0 ){
    pConst->nConst = 0;
  }ورنہ{
    pConst->ap_اظہاریہ[pConst->nConst*2-2] = p_عمود;
    pConst->ap_اظہاریہ[pConst->nConst*2-1] = p_قدر;
  }
}

/*
** Find all terms of COLUMN=VALUE or VALUE=COLUMN in pExpr where VALUE
** is a constant expression and where the term must be true because it
** is part of the AND-connected terms of the expression.  For each term
** found, add it to the pConst structure.
*/
ساکن خالی ڈھونڈو_Const_اندر_Where(WhereConst *pConst, اظہاریہ *p_اظہاریہ){
  اظہاریہ *p_دایاں, *p_بایاں;
  اگر( NEVER(p_اظہاریہ==0) ) واپس;
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, pConst->mExcludeOn) ){
    testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) );
    testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_InnerON) );
    واپس;
  }
  اگر( p_اظہاریہ->op==TK_AND ){
    ڈھونڈو_Const_اندر_Where(pConst, p_اظہاریہ->p_دایاں);
    ڈھونڈو_Const_اندر_Where(pConst, p_اظہاریہ->p_بایاں);
    واپس;
  }
  اگر( p_اظہاریہ->op!=TK_EQ ) واپس;
  p_دایاں = p_اظہاریہ->p_دایاں;
  p_بایاں = p_اظہاریہ->p_بایاں;
  أكد( p_دایاں!=0 );
  أكد( p_بایاں!=0 );
  اگر( p_دایاں->op==TK_عمود && sqlite3_اظہاریہ_Is_Constant(pConst->pParse, p_بایاں) ){
    const_داخل_کرو(pConst,p_دایاں,p_بایاں,p_اظہاریہ);
  }
  اگر( p_بایاں->op==TK_عمود && sqlite3_اظہاریہ_Is_Constant(pConst->pParse, p_دایاں) ){
    const_داخل_کرو(pConst,p_بایاں,p_دایاں,p_اظہاریہ);
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
ساکن صحیح propagate_Constant_اظہاریہ_Rewrite_One(
  WhereConst *pConst,
  اظہاریہ *p_اظہاریہ,
  صحیح bIgnoreAffBlob
){
  صحیح i;
  اگر( pConst->pOomFault[0] ) واپس WRC_Prune;
  اگر( p_اظہاریہ->op!=TK_عمود ) واپس WRC_Continue;
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Fixed_ستون|pConst->mExcludeOn) ){
    testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_Fixed_ستون) );
    testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) );
    testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_InnerON) );
    واپس WRC_Continue;
  }
  برائے(i=0; i<pConst->nConst; i++){
    اظہاریہ *p_عمود = pConst->ap_اظہاریہ[i*2];
    اگر( p_عمود==p_اظہاریہ ) جاری;
    اگر( p_عمود->i_جدول!=p_اظہاریہ->i_جدول ) جاری;
    اگر( p_عمود->i_عمود!=p_اظہاریہ->i_عمود ) جاری;
    أكد( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
    اگر( bIgnoreAffBlob && sqlite3_اظہاریہ_Affinity(p_عمود)<=SQLITE_AFF_BLOB ){
      توڑو;
    }
    /* A match is found.  Add the EP_FixedCol property */
    pConst->nChng++;
    اظہاریہ_Clear_Property(p_اظہاریہ, EP_برگ);
    اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_Fixed_ستون);
    أكد( p_اظہاریہ->p_بایاں==0 );
    p_اظہاریہ->p_بایاں = sqlite3_اظہاریہ_Dup(pConst->pParse->db, pConst->ap_اظہاریہ[i*2+1], 0);
    اگر( pConst->pParse->db->mallocFailed ) واپس WRC_Prune;
    توڑو;
  }
  واپس WRC_Prune;
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
ساکن صحیح propagate_Constant_اظہاریہ_Rewrite(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  WhereConst *pConst = pWalker->u.pConst;
  أكد( TK_GT==TK_EQ+1 );
  أكد( TK_LE==TK_EQ+2 );
  أكد( TK_LT==TK_EQ+3 );
  أكد( TK_GE==TK_EQ+4 );
  اگر( pConst->bHasAffBlob ){
    اگر( (p_اظہاریہ->op>=TK_EQ && p_اظہاریہ->op<=TK_GE)
     || p_اظہاریہ->op==TK_IS
    ){
      propagate_Constant_اظہاریہ_Rewrite_One(pConst, p_اظہاریہ->p_بایاں, 0);
      اگر( pConst->pOomFault[0] ) واپس WRC_Prune;
      اگر( sqlite3_اظہاریہ_Affinity(p_اظہاریہ->p_بایاں)!=SQLITE_AFF_متن ){
        propagate_Constant_اظہاریہ_Rewrite_One(pConst, p_اظہاریہ->p_دایاں, 0);
      }
    }
  }
  واپس propagate_Constant_اظہاریہ_Rewrite_One(pConst, p_اظہاریہ, pConst->bHasAffBlob);
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
ساکن صحیح propagateConstants(
  Parse *pParse,   /* The parsing context */
  Select *p        /* The query in which to propagate constants */
){
  WhereConst x;
  Walker w;
  صحیح nChng = 0;
  x.pParse = pParse;
  x.pOomFault = &pParse->db->mallocFailed;
  کرو{
    x.nConst = 0;
    x.nChng = 0;
    x.ap_اظہاریہ = 0;
    x.bHasAffBlob = 0;
    اگر( ALWAYS(p->pSrc!=0)
     && p->pSrc->nSrc>0
     && (p->pSrc->a[0].fg.jointype & JT_LTORJ)!=0
    ){
      /* Do not propagate constants on any ON clause if there is a
      ** RIGHT JOIN anywhere in the query */
      x.mExcludeOn = EP_InnerON | EP_OuterON;
    }ورنہ{
      /* Do not propagate constants through the ON clause of a LEFT JOIN */
      x.mExcludeOn = EP_OuterON;
    }
    ڈھونڈو_Const_اندر_Where(&x, p->pWhere);
    اگر( x.nConst ){
      memset(&w, 0, حجم(w));
      w.pParse = pParse;
      w.x_اظہاریہ_Callback = propagate_Constant_اظہاریہ_Rewrite;
      w.xSelectCallback = sqlite3SelectWalkNoop;
      w.xSelectCallback2 = 0;
      w.walker_گہرائی = 0;
      w.u.pConst = &x;
      sqlite3_Walk_اظہاریہ(&w, p->pWhere);
      sqlite3DbFree(x.pParse->db, x.ap_اظہاریہ);
      nChng += x.nChng;
    }
  }جب( x.nChng ); 
  واپس nChng;
}

#اگر !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
# اگر !defined(SQLITE_OMIT_WINDOWFUNC)
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
ساکن صحیح ڈالو_نیچے_کو_کھڑکی_Check(Parse *pParse, Select *pSubq, اظہاریہ *p_اظہاریہ){
  أكد( pSubq->pWin->p_تقسیم );
  أكد( (pSubq->selFlags & SF_MultiPart)==0 );
  أكد( pSubq->p_پیشگی==0 );
  واپس sqlite3_اظہاریہ_Is_Constant_Or_گروہ_By(pParse, p_اظہاریہ, pSubq->pWin->p_تقسیم);
}
# endif /* SQLITE_OMIT_WINDOWFUNC */
#اگر_ختم /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#اگر !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
ساکن صحیح ڈالو_نیچے_کو_Where_Terms(
  Parse *pParse,        /* Parse context (for malloc() and error reporting) */
  Select *pSubq,        /* The subquery whose WHERE clause is to be augmented */
  اظہاریہ *pWhere,         /* The WHERE clause of the outer query */
  SrcList *pSrcList,    /* The complete from clause of the outer query */
  صحیح iSrc              /* Which FROM clause term to try to push into  */
){
  اظہاریہ *pNew;
  Src_شے *pSrc;        /* The subquery FROM term into which WHERE is pushed */
  صحیح nChng = 0;
  pSrc = &pSrcList->a[iSrc];
  اگر( pWhere==0 ) واپس 0;
  اگر( pSubq->selFlags & (SF_Recursive|SF_MultiPart) ){
    واپس 0;           /* restrictions (2) and (11) */
  }
  اگر( pSrc->fg.jointype & (JT_LTORJ|JT_دایاں) ){
    واپس 0;           /* restrictions (10) */
  }

  اگر( pSubq->p_پیشگی ){
    Select *pSel;
    صحیح notUnionAll = 0;
    برائے(pSel=pSubq; pSel; pSel=pSel->p_پیشگی){
      u8 op = pSel->op;
      أكد( op==TK_ALL || op==TK_SELECT
           || op==TK_UNION || op==TK_INTERSECT || op==TK_EXCEPT );
      اگر( op!=TK_ALL && op!=TK_SELECT ){
        notUnionAll = 1;
      }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
      اگر( pSel->pWin ) واپس 0;    /* restriction (6b) */
#اگر_ختم
    }
    اگر( notUnionAll ){
      /* If any of the compound arms are connected using UNION, INTERSECT,
      ** or EXCEPT, then we must ensure that none of the columns use a
      ** non-BINARY collating sequence. */
      برائے(pSel=pSubq; pSel; pSel=pSel->p_پیشگی){
        صحیح ii;
        ثابت اظہاریہ_List *pList = pSel->pEList;
        أكد( pList!=0 );
        برائے(ii=0; ii<pList->n_اظہاریہ; ii++){
          CollSeq *pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, pList->a[ii].p_اظہاریہ);
          اگر( !sqlite3_Is_ثنائی(pColl) ){
            واپس 0;  /* Restriction (8) */
          }
        }
      }
    }
  }ورنہ{
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    اگر( pSubq->pWin && pSubq->pWin->p_تقسیم==0 ) واپس 0;
#اگر_ختم
  }

#اگر_تعریف_شدہ SQLITE_DEBUG
  /* Only the first term of a compound can have a WITH clause.  But make
  ** sure no other terms are marked SF_Recursive in case something changes
  ** in the future.
  */
  {
    Select *pX; 
    برائے(pX=pSubq; pX; pX=pX->p_پیشگی){
      أكد( (pX->selFlags & (SF_Recursive))==0 );
    }
  }
#اگر_ختم

  اگر( pSubq->p_حد_بندی!=0 ){
    واپس 0; /* restriction (3) */
  }
  جب( pWhere->op==TK_AND ){
    nChng += ڈالو_نیچے_کو_Where_Terms(pParse, pSubq, pWhere->p_دایاں, pSrcList, iSrc);
    pWhere = pWhere->p_بایاں;
  }

#اگر 0 /* These checks now done by sqlite3ExprIsSingleTableConstraint() */
  اگر( اظہاریہ_Has_Property(pWhere, EP_OuterON|EP_InnerON) /* (9a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (9c) */
  ){
    صحیح jj;
    برائے(jj=0; jj<iSrc; jj++){
      اگر( pWhere->w.iJoin==pSrcList->a[jj].iCursor ){
        /* If we reach this point, both (9a) and (9b) are satisfied.
        ** The following loop checks (9c):
        */
        برائے(jj++; jj<iSrc; jj++){
          اگر( (pSrcList->a[jj].fg.jointype & JT_دایاں)!=0 ){
            واپس 0;  /* restriction (9) */
          }
        }
      }
    }
  }
  اگر( is_بایاں_Join
   && (اظہاریہ_Has_Property(pWhere,EP_OuterON)==0
         || pWhere->w.iJoin!=iCursor)
  ){
    واپس 0; /* restriction (4) */
  }
  اگر( اظہاریہ_Has_Property(pWhere,EP_OuterON)
   && pWhere->w.iJoin!=iCursor
  ){
    واپس 0; /* restriction (5) */
  }
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ALLOW_ROWID_اندر_VIEW
  اگر( ViewCanHaveRowid && (pWhere->op==TK_ISNULL || pWhere->op==TK_NOTNULL) ){
    اظہاریہ *p_بایاں = pWhere->p_بایاں;
    اگر( ALWAYS(p_بایاں) 
     && p_بایاں->op==TK_عمود
     && p_بایاں->i_عمود < 0
    ){
      واپس 0;  /* Restriction (12) */
    }
  }
#اگر_ختم

  اگر( sqlite3_اظہاریہ_Is_Single_جدول_Constraint(pWhere, pSrcList, iSrc, 1) ){
    nChng++;
    pSubq->selFlags |= SF_ڈالو_نیچے_کو;
    جب( pSubq ){
      SubstContext x;
      pNew = sqlite3_اظہاریہ_Dup(pParse->db, pWhere, 0);
      unset_Join_اظہاریہ(pNew, -1, 1);
      x.pParse = pParse;
      x.i_جدول = pSrc->iCursor;
      x.i_New_جدول = pSrc->iCursor;
      x.isOuterJoin = 0;
      x.n_Sel_گہرائی = 0;
      x.pEList = pSubq->pEList;
      x.pCList = ڈھونڈو_Leftmost_Exprlist(pSubq);
      pNew = subst_اظہاریہ(&x, pNew);
      أكد( pNew!=0 || pParse->nErr!=0 );
      اگر( pParse->nErr==0 && pNew->op==TK_اندر && اظہاریہ_Use_XSelect(pNew) ){
        أكد( pNew->x.pSelect!=0 );
        pNew->x.pSelect->selFlags |= SF_Cloned_Rhs_اندر;
        أكد( pWhere!=0 );
        أكد( pWhere->op==TK_اندر );
        أكد( اظہاریہ_Use_XSelect(pWhere) );
        أكد( pWhere->x.pSelect!=0 );
        pWhere->x.pSelect->selFlags |= SF_Cloned_Rhs_اندر;
      }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
      اگر( pSubq->pWin && 0==ڈالو_نیچے_کو_کھڑکی_Check(pParse, pSubq, pNew) ){
        /* Restriction 6c has prevented push-down in this case */
        sqlite3_اظہاریہ_Delete(pParse->db, pNew);
        nChng--;
        توڑو;
      }
#اگر_ختم
      اگر( pSubq->selFlags & SF_Aggregate ){
        pSubq->pHaving = sqlite3_اظہاریہ_And(pParse, pSubq->pHaving, pNew);
      }ورنہ{
        pSubq->pWhere = sqlite3_اظہاریہ_And(pParse, pSubq->pWhere, pNew);
      }
      pSubq = pSubq->p_پیشگی;
    }
  }
  واپس nChng;
}
#اگر_ختم /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** Check to see if a subquery contains result-set columns that are
** never used.  If it does, change the value of those result-set columns
** to NULL so that they do not cause unnecessary work to compute.
**
** Return the number of column that were changed to NULL.
*/
ساکن صحیح disable_غیر_مستعمل_Subquery_نتیجہ_Columns(Src_شے *p_شے){
  صحیح n_ستون;
  Select *pSub;      /* The subquery to be simplified */
  Select *pX;        /* For looping over compound elements of pSub */
  جدول *pTab;       /* The table that describes the subquery */
  صحیح j;             /* Column number */
  صحیح nChng = 0;     /* Number of columns converted to NULL */
  Bitmask ستون_مستعمل;   /* Columns that may not be NULLed out */

  أكد( p_شے!=0 );
  اگر( p_شے->fg.isCorrelated || p_شے->fg.isCte ){
    واپس 0;
  }
  أكد( p_شے->pSTab!=0 );
  pTab = p_شے->pSTab;
  أكد( p_شے->fg.isSubquery );
  pSub = p_شے->u4.pSubq->pSelect;
  أكد( pSub->pEList->n_اظہاریہ==pTab->n_ستون );
  برائے(pX=pSub; pX; pX=pX->p_پیشگی){
    اگر( (pX->selFlags & (SF_Distinct|SF_Aggregate))!=0 ){
      testcase( pX->selFlags & SF_Distinct );
      testcase( pX->selFlags & SF_Aggregate );
      واپس 0;
    }
    اگر( pX->p_پیشگی && pX->op!=TK_ALL ){
      /* This optimization does not work for compound subqueries that
      ** use UNION, INTERSECT, or EXCEPT.  Only UNION ALL is allowed. */
      واپس 0;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    اگر( pX->pWin ){
      /* This optimization does not work for subqueries that use window
      ** functions. */
      واپس 0;
    }
#اگر_ختم
  }
  ستون_مستعمل = p_شے->ستون_مستعمل;
  اگر( pSub->p_ترتیب_By ){
    اظہاریہ_List *pList = pSub->p_ترتیب_By;
    برائے(j=0; j<pList->n_اظہاریہ; j++){
      u16 i_ستون = pList->a[j].u.x.i_ترتیب_By_ستون;
      اگر( i_ستون>0 ){
        i_ستون--;
        ستون_مستعمل |= ((Bitmask)1)<<(i_ستون>=BMS ? BMS-1 : i_ستون);
      }
    }
  }
  n_ستون = pTab->n_ستون;
  برائے(j=0; j<n_ستون; j++){
    Bitmask m = j<BMS-1 ? MASKBIT(j) : TOPBIT;
    اگر( (m & ستون_مستعمل)!=0 ) جاری;
    برائے(pX=pSub; pX; pX=pX->p_پیشگی) {
      اظہاریہ *pY = pX->pEList->a[j].p_اظہاریہ;
      اگر( pY->op==TK_NULL ) جاری;
      pY->op = TK_NULL;
      اظہاریہ_Clear_Property(pY, EP_Skip|EP_Unlikely);
      pX->selFlags |= SF_ڈالو_نیچے_کو;
      nChng++;
    }
  }
  واپس nChng;
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
ساکن u8 minMaxQuery(sqlite3 *db, اظہاریہ *pFunc, اظہاریہ_List **ppMinMax){
  صحیح eRet = WHERE_ORDERBY_معمول;      /* Return value */
  اظہاریہ_List *pEList;                     /* Arguments to agg function */
  ثابت حرف *zFunc;                    /* Name of aggregate function pFunc */
  اظہاریہ_List *p_ترتیب_By;
  u8 ترتیب_دو_Flags = 0;

  أكد( *ppMinMax==0 );
  أكد( pFunc->op==TK_AGG_FUNCTION );
  أكد( !Is_کھڑکی_Func(pFunc) );
  أكد( اظہاریہ_Use_XList(pFunc) );
  pEList = pFunc->x.pList;
  اگر( pEList==0
   || pEList->n_اظہاریہ!=1
   || اظہاریہ_Has_Property(pFunc, EP_WinFunc)
   || OptimizationDisabled(db, SQLITE_MinMaxOpt)
  ){
    واپس eRet;
  }
  أكد( !اظہاریہ_Has_Property(pFunc, EP_Int_قدر) );
  zFunc = pFunc->u.zToken;
  اگر( sqlite3StrICmp(zFunc, "min")==0 ){
    eRet = WHERE_ORDERBY_MIN;
    اگر( sqlite3_اظہاریہ_Can_Be_Null(pEList->a[0].p_اظہاریہ) ){
      ترتیب_دو_Flags = KEYINFO_ترتیب_BIGNULL;
    }
  }ورنہ اگر( sqlite3StrICmp(zFunc, "max")==0 ){
    eRet = WHERE_ORDERBY_MAX;
    ترتیب_دو_Flags = KEYINFO_ترتیب_DESC;
  }ورنہ{
    واپس eRet;
  }
  *ppMinMax = p_ترتیب_By = sqlite3_اظہاریہ_List_Dup(db, pEList, 0);
  أكد( p_ترتیب_By!=0 || db->mallocFailed );
  اگر( p_ترتیب_By ) p_ترتیب_By->a[0].fg.ترتیب_دو_Flags = ترتیب_دو_Flags;
  واپس eRet;
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
ساکن جدول *is_Simple_گنتی(Select *p, AggInfo *pAggInfo){
  جدول *pTab;
  اظہاریہ *p_اظہاریہ;

  أكد( !p->p_گروہ_By );

  اگر( p->pWhere
   || p->pEList->n_اظہاریہ!=1
   || p->pSrc->nSrc!=1
   || p->pSrc->a[0].fg.isSubquery
   || pAggInfo->nFunc!=1
   || p->pHaving
  ){
    واپس 0;
  }
  pTab = p->pSrc->a[0].pSTab;
  أكد( pTab!=0 );
  أكد( !IsView(pTab) );
  اگر( !Is_Ordinary_جدول(pTab) ) واپس 0;
  p_اظہاریہ = p->pEList->a[0].p_اظہاریہ;
  أكد( p_اظہاریہ!=0 );
  اگر( p_اظہاریہ->op!=TK_AGG_FUNCTION ) واپس 0;
  اگر( p_اظہاریہ->pAggInfo!=pAggInfo ) واپس 0;
  اگر( (pAggInfo->aFunc[0].pFunc->funcFlags&SQLITE_FUNC_گنتی)==0 ) واپس 0;
  أكد( pAggInfo->aFunc[0].pFExpr==p_اظہاریہ );
  testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_Distinct) );
  testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) );
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Distinct|EP_WinFunc) ) واپس 0;

  واپس pTab;
}

/*
** If the source-list item passed as an argument was augmented with an
** INDEXED BY clause, then try to locate the specified index. If there
** was such a clause and the named index cannot be found, return
** SQLITE_ERROR and leave an error in pParse. Otherwise, populate
** pFrom->pIndex and return SQLITE_OK.
*/
صحیح sqlite3_اشاریہ_دار_By_Lookup(Parse *pParse, Src_شے *pFrom){
  جدول *pTab = pFrom->pSTab;
  حرف *z_اشاریہ_دار_By = pFrom->u1.z_اشاریہ_دار_By;
  اشاریہ *pIdx;
  أكد( pTab!=0 );
  أكد( pFrom->fg.is_اشاریہ_دار_By!=0 );

  برائے(pIdx=pTab->p_اشاریہ;
      pIdx && sqlite3StrICmp(pIdx->zName, z_اشاریہ_دار_By);
      pIdx=pIdx->p_اگلا
  );
  اگر( !pIdx ){
    sqlite3_Error_پیغام(pParse, "no such index: %s", z_اشاریہ_دار_By, 0);
    pParse->checkSchema = 1;
    واپس SQLITE_ERROR;
  }
  أكد( pFrom->fg.isCte==0 );
  pFrom->u2.pIBIndex = pIdx;
  واپس SQLITE_OK;
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
ساکن صحیح convert_Compound_Select_مقصد_Subquery(Walker *pWalker, Select *p){
  صحیح i;
  Select *pNew;
  Select *pX;
  sqlite3 *db;
  ساخت اظہاریہ_List_شے *a;
  SrcList *pNewSrc;
  Parse *pParse;
  Token dummy;

  اگر( p->p_پیشگی==0 ) واپس WRC_Continue;
  اگر( p->p_ترتیب_By==0 ) واپس WRC_Continue;
  برائے(pX=p; pX && (pX->op==TK_ALL || pX->op==TK_SELECT); pX=pX->p_پیشگی){}
  اگر( pX==0 ) واپس WRC_Continue;
  a = p->p_ترتیب_By->a;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
  /* If iOrderByCol is already non-zero, then it has already been matched
  ** to a result column of the SELECT statement. This occurs when the
  ** SELECT is rewritten for window-functions processing and then passed
  ** to sqlite3SelectPrep() and similar a second time. The rewriting done
  ** by this function is not required in this case. */
  اگر( a[0].u.x.i_ترتیب_By_ستون ) واپس WRC_Continue;
#اگر_ختم
  برائے(i=p->p_ترتیب_By->n_اظہاریہ-1; i>=0; i--){
    اگر( a[i].p_اظہاریہ->flags & EP_Collate ) توڑو;
  }
  اگر( i<0 ) واپس WRC_Continue;

  /* If we reach this point, that means the transformation is required. */

  pParse = pWalker->pParse;
  db = pParse->db;
  pNew = sqlite3DbMallocZero(db, حجم(*pNew) );
  اگر( pNew==0 ) واپس WRC_Abort;
  memset(&dummy, 0, حجم(dummy));
  pNewSrc = sqlite3_Src_List_Append_From_رکن(pParse,0,0,0,&dummy,pNew,0);
  أكد( pNewSrc!=0 || pParse->nErr );
  اگر( pParse->nErr ){
    sqlite3SrcListDelete(db, pNewSrc);
    واپس WRC_Abort;
  }
  *pNew = *p;
  p->pSrc = pNewSrc;
  p->pEList = sqlite3_اظہاریہ_List_Append(pParse, 0, sqlite3_اظہاریہ(db, TK_ASTERISK, 0));
  p->op = TK_SELECT;
  p->pWhere = 0;
  p->p_گروہ_By = 0;
  p->pHaving = 0;
  pNew->p_ترتیب_By = 0;
  p->p_پیشگی = 0;
  p->p_اگلا = 0;
  p->pWith = 0;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
  p->pWinDefn = 0;
#اگر_ختم
  p->selFlags &= ~(u32)(SF_Compound|SF_Distinct);
  أكد( (p->selFlags & SF_Converted)==0 );
  p->selFlags |= SF_Converted;
  أكد( pNew->p_پیشگی!=0 );
  pNew->p_پیشگی->p_اگلا = pNew;
  pNew->p_حد_بندی = 0;
  واپس WRC_Continue;
}

/*
** Check to see if the FROM clause term pFrom has table-valued function
** arguments.  If it does, leave an error message in pParse and return
** non-zero, since pFrom is not allowed to be a table-valued function.
*/
ساکن صحیح cannotBeFunction(Parse *pParse, Src_شے *pFrom){
  اگر( pFrom->fg.isTabFunc ){
    sqlite3_Error_پیغام(pParse, "'%s' is not a function", pFrom->zName);
    واپس 1;
  }
  واپس 0;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CTE
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
ساکن ساخت Cte *تلاش_With(
  With *pWith,                    /* Current innermost WITH clause */
  Src_شے *p_شے,                 /* FROM clause element to resolve */
  With **ppContext                /* OUT: WITH clause return value belongs to */
){
  ثابت حرف *zName = p_شے->zName;
  With *p;
  أكد( p_شے->fg.fixedSchema || p_شے->u4.zDatabase==0 );
  أكد( zName!=0 );
  برائے(p=pWith; p; p=p->pOuter){
    صحیح i;
    برائے(i=0; i<p->nCte; i++){
      اگر( sqlite3StrICmp(zName, p->a[i].zName)==0 ){
        *ppContext = p;
        واپس &p->a[i];
      }
    }
    اگر( p->bView ) توڑو;
  }
  واپس 0;
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
With *sqlite3_With_ڈالو(Parse *pParse, With *pWith, u8 bFree){
  اگر( pWith ){
    اگر( bFree ){
      pWith = (With*)sqlite3_Parser_جوڑو_Cleanup(pParse, sqlite3WithDeleteGeneric,
                      pWith);
      اگر( pWith==0 ) واپس 0;
    }
    اگر( pParse->nErr==0 ){
      أكد( pParse->pWith!=pWith );
      pWith->pOuter = pParse->pWith;
      pParse->pWith = pWith;
    }
  }
  واپس pWith;
}

ساکن صحیح selectExpanderWalk(Walker*, Select*);

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
ساکن صحیح resolve_From_رکن_مقصد_Cte(
  Parse *pParse,                  /* The parsing context */
  Walker *pWalker,                /* Current tree walker */
  Src_شے *pFrom                  /* The FROM clause term to check */
){
  Cte *pCte;               /* Matched CTE (or NULL if no match) */
  With *pWith;             /* The matching WITH */

  أكد( pFrom->pSTab==0 );
  اگر( pParse->pWith==0 ){
    /* There are no WITH clauses in the stack.  No match is possible */
    واپس 0;
  }
  اگر( pParse->nErr ){
    /* Prior errors might have left pParse->pWith in a goofy state, so
    ** go no further. */
    واپس 0;
  }
  أكد( pFrom->fg.hadSchema==0 || pFrom->fg.notCte!=0 );
  اگر( pFrom->fg.fixedSchema==0 && pFrom->u4.zDatabase!=0 ){
    /* The FROM term contains a schema qualifier (ex: main.t1) and so
    ** it cannot possibly be a CTE reference. */
    واپس 0;
  }
  اگر( pFrom->fg.notCte ){
    /* The FROM term is specifically excluded from matching a CTE.
    **   (1)  It is part of a trigger that used to have zDatabase but had
    **        zDatabase removed by sqlite3FixTriggerStep().
    **   (2)  This is the first term in the FROM clause of an UPDATE.
    */
    واپس 0;
  }
  pCte = تلاش_With(pParse->pWith, pFrom, &pWith);
  اگر( pCte ){
    sqlite3 *db = pParse->db;
    جدول *pTab;
    اظہاریہ_List *pEList;
    Select *pSel;
    Select *p_بایاں;                /* Left-most SELECT statement */
    Select *p_Rec_رکن;             /* Left-most recursive term */
    صحیح bMayRecursive;            /* True if compound joined by UNION [ALL] */
    With *pSavedWith;             /* Initial value of pParse->pWith */
    صحیح iRecTab = -1;             /* Cursor for recursive table */
    CteUse *pCteUse;

    /* If pCte->zCteErr is non-NULL at this point, then this is an illegal
    ** recursive reference to CTE pCte. Leave an error in pParse and return
    ** early. If pCte->zCteErr is NULL, then this is not a recursive reference.
    ** In this case, proceed.  */
    اگر( pCte->zCteErr ){
      sqlite3_Error_پیغام(pParse, pCte->zCteErr, pCte->zName);
      واپس 2;
    }
    اگر( cannotBeFunction(pParse, pFrom) ) واپس 2;

    أكد( pFrom->pSTab==0 );
    pTab = sqlite3DbMallocZero(db, حجم(جدول));
    اگر( pTab==0 ) واپس 2;
    pCteUse = pCte->pUse;
    اگر( pCteUse==0 ){
      pCte->pUse = pCteUse = sqlite3DbMallocZero(db, حجم(pCteUse[0]));
      اگر( pCteUse==0
       || sqlite3_Parser_جوڑو_Cleanup(pParse,sqlite3DbFree,pCteUse)==0
      ){
        sqlite3DbFree(db, pTab);
        واپس 2;
      }
      pCteUse->eM10d = pCte->eM10d;
    }
    pFrom->pSTab = pTab;
    pTab->nTabRef = 1;
    pTab->zName = sqlite3DbStrDup(db, pCte->zName);
    pTab->iPKey = -1;
    pTab->n_صف_Log_Est = 200; أكد( 200==sqlite3LogEst(1048576) );
    pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
    sqlite3_Src_شے_Attach_Subquery(pParse, pFrom, pCte->pSelect, 1);
    اگر( db->mallocFailed ) واپس 2;
    أكد( pFrom->fg.isSubquery && pFrom->u4.pSubq );
    pSel = pFrom->u4.pSubq->pSelect;
    أكد( pSel!=0 );
    pSel->selFlags |= SF_CopyCte;
    اگر( pFrom->fg.is_اشاریہ_دار_By ){
      sqlite3_Error_پیغام(pParse, "no such index: \"%s\"", pFrom->u1.z_اشاریہ_دار_By);
      واپس 2;
    }
    أكد( !pFrom->fg.is_اشاریہ_دار_By );
    pFrom->fg.isCte = 1;
    pFrom->u2.pCteUse = pCteUse;
    pCteUse->nUse++;

    /* Check if this is a recursive CTE. */
    p_Rec_رکن = pSel;
    bMayRecursive = ( pSel->op==TK_ALL || pSel->op==TK_UNION );
    جب( bMayRecursive && p_Rec_رکن->op==pSel->op ){
      صحیح i;
      SrcList *pSrc = p_Rec_رکن->pSrc;
      أكد( p_Rec_رکن->p_پیشگی!=0 );
      برائے(i=0; i<pSrc->nSrc; i++){
        Src_شے *p_شے = &pSrc->a[i];
        اگر( p_شے->zName!=0
         && !p_شے->fg.hadSchema
         && ALWAYS( !p_شے->fg.isSubquery )
         && (p_شے->fg.fixedSchema || p_شے->u4.zDatabase==0)
         && 0==sqlite3StrICmp(p_شے->zName, pCte->zName)
        ){
          p_شے->pSTab = pTab;
          pTab->nTabRef++;
          p_شے->fg.isRecursive = 1;
          اگر( p_Rec_رکن->selFlags & SF_Recursive ){
            sqlite3_Error_پیغام(pParse,
               "multiple references to recursive table: %s", pCte->zName
            );
            واپس 2;
          }
          p_Rec_رکن->selFlags |= SF_Recursive;
          اگر( iRecTab<0 ) iRecTab = pParse->nTab++;
          p_شے->iCursor = iRecTab;
        }
      }
      اگر( (p_Rec_رکن->selFlags & SF_Recursive)==0 ) توڑو;
      p_Rec_رکن = p_Rec_رکن->p_پیشگی;
    }

    اگر( pParse->nTab >= db->a_حد_بندی[SQLITE_حد_بندی_CTES]
     && db->a_حد_بندی[SQLITE_حد_بندی_CTES]>0
    ){
      sqlite3_Error_پیغام(pParse, "too many CTEs");
      واپس 2;
    }
#اگر SQLITE_MAX_اظہاریہ_گہرائی>0
    pParse->nNestSel++;
    اگر( check_Subquery_Nesting_گہرائی(pParse) ) واپس 2;
#اگر_ختم
    pCte->zCteErr = "circular reference: %s";
    pSavedWith = pParse->pWith;
    pParse->pWith = pWith;
    اگر( pSel->selFlags & SF_Recursive ){
      صحیح rc;
      أكد( p_Rec_رکن!=0 );
      أكد( (p_Rec_رکن->selFlags & SF_Recursive)==0 );
      أكد( p_Rec_رکن->p_اگلا!=0 );
      أكد( (p_Rec_رکن->p_اگلا->selFlags & SF_Recursive)!=0 );
      أكد( p_Rec_رکن->pWith==0 );
      p_Rec_رکن->pWith = pSel->pWith;
      rc = sqlite3WalkSelect(pWalker, p_Rec_رکن);
      p_Rec_رکن->pWith = 0;
      اگر( rc ){
        pParse->pWith = pSavedWith;
        واپس 2;
      }
    }ورنہ{
      اگر( selectExpanderWalk(pWalker, pSel) ){
        pParse->pWith = pSavedWith;
        واپس 2;
      }
    }
    pParse->pWith = pWith;
#اگر SQLITE_MAX_اظہاریہ_گہرائی>0
    pParse->nNestSel--;
    أكد( pParse->nNestSel>=0 );
#اگر_ختم

    برائے(p_بایاں=pSel; p_بایاں->p_پیشگی; p_بایاں=p_بایاں->p_پیشگی);
    pEList = p_بایاں->pEList;
    اگر( pCte->p_ستون_تعداد ){
      اگر( pEList && pEList->n_اظہاریہ!=pCte->p_ستون_تعداد->n_اظہاریہ ){
        sqlite3_Error_پیغام(pParse, "table %s has %d values for %d columns",
            pCte->zName, pEList->n_اظہاریہ, pCte->p_ستون_تعداد->n_اظہاریہ
        );
        pParse->pWith = pSavedWith;
        واپس 2;
      }
      pEList = pCte->p_ستون_تعداد;
    }

    sqlite3_Columns_From_اظہاریہ_List(pParse, pEList, &pTab->n_ستون, &pTab->a_ستون);
    اگر( bMayRecursive ){
      اگر( pSel->selFlags & SF_Recursive ){
        pCte->zCteErr = "multiple recursive references: %s";
      }ورنہ{
        pCte->zCteErr = "recursive reference in a subquery: %s";
      }
      sqlite3WalkSelect(pWalker, pSel);
    }
    pCte->zCteErr = 0;
    pParse->pWith = pSavedWith;
    واپس 1;  /* Success */
  }
  واپس 0;  /* No match */
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CTE
/*
** If the SELECT passed as the second argument has an associated WITH
** clause, pop it from the stack stored as part of the Parse object.
**
** This function is used as the xSelectCallback2() callback by
** sqlite3SelectExpand() when walking a SELECT tree to resolve table
** names and other FROM clause elements.
*/
خالی sqlite3_Select_نکالو_With(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  اگر( OK_IF_ALWAYS_TRUE(pParse->pWith) && p->p_پیشگی==0 ){
    With *pWith = ڈھونڈو_Rightmost(p)->pWith;
    اگر( pWith!=0 ){
      أكد( pParse->pWith==pWith || pParse->nErr );
      pParse->pWith = pWith->pOuter;
    }
  }
}
#اگر_ختم

/*
** The SrcItem structure passed as the second argument represents a
** sub-query in the FROM clause of a SELECT statement. This function
** allocates and populates the SrcItem.pTab object. If successful,
** SQLITE_OK is returned. Otherwise, if an OOM error is encountered,
** SQLITE_NOMEM.
*/
صحیح sqlite3ExpandSubquery(Parse *pParse, Src_شے *pFrom){
  Select *pSel;
  جدول *pTab;

  أكد( pFrom->fg.isSubquery );
  أكد( pFrom->u4.pSubq!=0 );
  pSel = pFrom->u4.pSubq->pSelect;
  أكد( pSel );
  pFrom->pSTab = pTab = sqlite3DbMallocZero(pParse->db, حجم(جدول));
  اگر( pTab==0 ) واپس SQLITE_NOMEM;
  pTab->nTabRef = 1;
  اگر( pFrom->zAlias ){
    pTab->zName = sqlite3DbStrDup(pParse->db, pFrom->zAlias);
  }ورنہ{
    pTab->zName = sqlite3MPrintf(pParse->db, "%!S", pFrom);
  }
  جب( pSel->p_پیشگی ){ pSel = pSel->p_پیشگی; }
  sqlite3_Columns_From_اظہاریہ_List(pParse, pSel->pEList,&pTab->n_ستون,&pTab->a_ستون);
  pTab->iPKey = -1;
  pTab->eTabType = TABTYP_VIEW;
  pTab->n_صف_Log_Est = 200; أكد( 200==sqlite3LogEst(1048576) );
#اگر_غیر_تعریف_شدہ SQLITE_ALLOW_ROWID_اندر_VIEW
  /* The usual case - do not allow ROWID on a subquery */
  pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
#ورنہ
  /* Legacy compatibility mode */
  pTab->tabFlags |= TF_Ephemeral | sqlite3Config.mNoVisibleRowid;
#اگر_ختم
  واپس pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
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
ساکن صحیح اندر_Any_Using_Clause(
  ثابت حرف *zName, /* Name we are looking for */
  Src_شے *p_بنیاد,    /* The base SrcItem.  Looking at pBase[1] and following */
  صحیح N              /* How many SrcItems to check */
){
  جب( N>0 ){
    N--;
    p_بنیاد++;
    اگر( p_بنیاد->fg.isUsing==0 ) جاری;
    اگر( NEVER(p_بنیاد->u3.pUsing==0) ) جاری;
    اگر( sqlite3_Id_List_اشاریہ(p_بنیاد->u3.pUsing, zName)>=0 ) واپس 1;
  }
  واپس 0;
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
ساکن صحیح selectExpander(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  صحیح i, j, k, rc;
  SrcList *pTabList;
  اظہاریہ_List *pEList;
  Src_شے *pFrom;
  sqlite3 *db = pParse->db;
  اظہاریہ *pE, *p_دایاں, *p_اظہاریہ;
  u16 selFlags = p->selFlags;
  u32 elistFlags = 0;

  p->selFlags |= SF_Expanded;
  اگر( db->mallocFailed  ){
    واپس WRC_Abort;
  }
  أكد( p->pSrc!=0 );
  اگر( (selFlags & SF_Expanded)!=0 ){
    واپس WRC_Prune;
  }
  اگر( pWalker->e_رمز ){
    /* Renumber selId because it has been copied from a view */
    p->selId = ++pParse->nSelect;
  }
  pTabList = p->pSrc;
  pEList = p->pEList;
  اگر( pParse->pWith && (p->selFlags & SF_View) ){
    اگر( p->pWith==0 ){
      p->pWith = (With*)sqlite3DbMallocZero(db, SZ_WITH(1) );
      اگر( p->pWith==0 ){
        واپس WRC_Abort;
      }
    }
    p->pWith->bView = 1;
  }
  sqlite3_With_ڈالو(pParse, p->pWith, 0);

  /* Make sure cursor numbers have been assigned to all entries in
  ** the FROM clause of the SELECT statement.
  */
  sqlite3_Src_List_تفویض_Cursors(pParse, pTabList);

  /* Look up every table named in the FROM clause of the select.  If
  ** an entry of the FROM clause is a subquery instead of a table or view,
  ** then create a transient table structure to describe the subquery.
  */
  برائے(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    جدول *pTab;
    أكد( pFrom->fg.isRecursive==0 || pFrom->pSTab!=0 );
    اگر( pFrom->pSTab ) جاری;
    أكد( pFrom->fg.isRecursive==0 );
    اگر( pFrom->zName==0 ){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
      Select *pSel;
      أكد( pFrom->fg.isSubquery && pFrom->u4.pSubq!=0 );
      pSel = pFrom->u4.pSubq->pSelect;
      /* A sub-query in the FROM clause of a SELECT */
      أكد( pSel!=0 );
      أكد( pFrom->pSTab==0 );
      اگر( sqlite3WalkSelect(pWalker, pSel) ) واپس WRC_Abort;
      اگر( sqlite3ExpandSubquery(pParse, pFrom) ) واپس WRC_Abort;
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CTE
    }ورنہ اگر( (rc = resolve_From_رکن_مقصد_Cte(pParse, pWalker, pFrom))!=0 ){
      اگر( rc>1 ) واپس WRC_Abort;
      pTab = pFrom->pSTab;
      أكد( pTab!=0 );
#اگر_ختم
    }ورنہ{
      /* An ordinary table or view name in the FROM clause */
      أكد( pFrom->pSTab==0 );
      pFrom->pSTab = pTab = sqlite3_Locate_جدول_شے(pParse, 0, pFrom);
      اگر( pTab==0 ) واپس WRC_Abort;
      اگر( pTab->nTabRef>=0xffff ){
        sqlite3_Error_پیغام(pParse, "too many references to \"%s\": max 65535",
           pTab->zName);
        pFrom->pSTab = 0;
        واپس WRC_Abort;
      }
      pTab->nTabRef++;
      اگر( !IsVirtual(pTab) && cannotBeFunction(pParse, pFrom) ){
        واپس WRC_Abort;
      }
#اگر !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
      اگر( !Is_Ordinary_جدول(pTab) ){
        i16 n_ستون;
        u8 e_رمز_Orig = pWalker->e_رمز;
        اگر( sqlite3_View_Get_عمود_نام(pParse, pTab) ) واپس WRC_Abort;
        أكد( pFrom->fg.isSubquery==0 );
        اگر( IsView(pTab) ){
          اگر( (db->flags & SQLITE_EnableView)==0
           && pTab->pSchema!=db->aDb[1].pSchema
          ){
            sqlite3_Error_پیغام(pParse, "access to view \"%s\" prohibited",
              pTab->zName);
          }
          sqlite3_Src_شے_Attach_Subquery(pParse, pFrom, pTab->u.view.pSelect, 1);
        }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_VIRTUALTABLE
        ورنہ اگر( ALWAYS(IsVirtual(pTab))
         && (pFrom->fg.fromDDL || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
         && ALWAYS(pTab->u.vtab.p!=0)
         && pTab->u.vtab.p->eVtabRisk > ((db->flags & SQLITE_TrustedSchema)!=0)
        ){
          sqlite3_Error_پیغام(pParse, "unsafe use of virtual table \"%s\"",
                                  pTab->zName);
        }
        أكد( SQLITE_VTABRISK_معمول==1 && SQLITE_VTABRISK_بلند==2 );
#اگر_ختم
        n_ستون = pTab->n_ستون;
        pTab->n_ستون = -1;
        pWalker->e_رمز = 1;  /* Turn on Select.selId renumbering */
        اگر( pFrom->fg.isSubquery ){
          bft hasCompound = pParse->hasCompound;
          pParse->hasCompound = 1;
          selectExpanderWalk(pWalker, pFrom->u4.pSubq->pSelect);
          pParse->hasCompound = hasCompound;
        }
        pWalker->e_رمز = e_رمز_Orig;
        pTab->n_ستون = n_ستون;
      }
#اگر_ختم
    }

    /* Locate the index named by the INDEXED BY clause, if any. */
    اگر( pFrom->fg.is_اشاریہ_دار_By && sqlite3_اشاریہ_دار_By_Lookup(pParse, pFrom) ){
      واپس WRC_Abort;
    }
  }

  /* Process NATURAL keywords, and ON and USING clauses of joins.
  */
  أكد( db->mallocFailed==0 || pParse->nErr!=0 );
  اگر( pParse->nErr || sqlite3ProcessJoin(pParse, p) ){
    واپس WRC_Abort;
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
  برائے(k=0; k<pEList->n_اظہاریہ; k++){
    pE = pEList->a[k].p_اظہاریہ;
    اگر( pE->op==TK_ASTERISK ) توڑو;
    أكد( pE->op!=TK_DOT || pE->p_دایاں!=0 );
    أكد( pE->op!=TK_DOT || (pE->p_بایاں!=0 && pE->p_بایاں->op==TK_ID) );
    اگر( pE->op==TK_DOT && pE->p_دایاں->op==TK_ASTERISK ) توڑو;
    elistFlags |= pE->flags;
  }
  اگر( k<pEList->n_اظہاریہ ){
    /*
    ** If we get here it means the result set contains one or more "*"
    ** operators that need to be expanded.  Loop through each expression
    ** in the result set and expand them one by one.
    */
    ساخت اظہاریہ_List_شے *a = pEList->a;
    اظہاریہ_List *pNew = 0;
    صحیح flags = pParse->db->flags;
    صحیح long_نام = (flags & SQLITE_بھرا_ستون_نام)!=0
                      && (flags & SQLITE_Short_ستون_نام)==0;

    برائے(k=0; k<pEList->n_اظہاریہ; k++){
      pE = a[k].p_اظہاریہ;
      elistFlags |= pE->flags;
      p_دایاں = pE->p_دایاں;
      أكد( pE->op!=TK_DOT || p_دایاں!=0 );
      اگر( pE->op!=TK_ASTERISK
       && (pE->op!=TK_DOT || p_دایاں->op!=TK_ASTERISK)
      ){
        /* This particular expression does not need to be expanded.
        */
        pNew = sqlite3_اظہاریہ_List_Append(pParse, pNew, a[k].p_اظہاریہ);
        اگر( pNew ){
          pNew->a[pNew->n_اظہاریہ-1].zEName = a[k].zEName;
          pNew->a[pNew->n_اظہاریہ-1].fg.eEName = a[k].fg.eEName;
          a[k].zEName = 0;
        }
        a[k].p_اظہاریہ = 0;
      }ورنہ{
        /* This expression is a "*" or a "TABLE.*" and needs to be
        ** expanded. */
        صحیح جدول_نظر_آیا = 0;      /* Set to 1 when TABLE matches */
        حرف *zTName = 0;       /* text of name of TABLE */
        صحیح iErrOfst;
        اگر( pE->op==TK_DOT ){
          أكد( (selFlags & SF_NestedFrom)==0 );
          أكد( pE->p_بایاں!=0 );
          أكد( !اظہاریہ_Has_Property(pE->p_بایاں, EP_Int_قدر) );
          zTName = pE->p_بایاں->u.zToken;
          أكد( اظہاریہ_Use_WOfst(pE->p_بایاں) );
          iErrOfst = pE->p_دایاں->w.iOfst;
        }ورنہ{
          أكد( pE->p_بایاں==0 );
          أكد( اظہاریہ_Use_WOfst(pE) );
          iErrOfst = pE->w.iOfst;
        }
        برائے(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
          صحیح n_جوڑو;                    /* Number of cols including rowid */
          جدول *pTab = pFrom->pSTab;  /* Table for this data source */
          اظہاریہ_List *pNestedFrom;       /* Result-set of a nested FROM clause */
          حرف *zTabName;              /* AS name for this data source */
          ثابت حرف *zSchemaName = 0; /* Schema name for this data source */
          صحیح iDb;                     /* Schema index for this data src */
          IdList *pUsing;              /* USING clause for pFrom[1] */

          اگر( (zTabName = pFrom->zAlias)==0 ){
            zTabName = pTab->zName;
          }
          اگر( db->mallocFailed ) توڑو;
          أكد( (صحیح)pFrom->fg.isNestedFrom == IsNestedFrom(pFrom) );
          اگر( pFrom->fg.isNestedFrom ){
            أكد( pFrom->fg.isSubquery && pFrom->u4.pSubq );
            أكد( pFrom->u4.pSubq->pSelect!=0 );
            pNestedFrom = pFrom->u4.pSubq->pSelect->pEList;
            أكد( pNestedFrom!=0 );
            أكد( pNestedFrom->n_اظہاریہ==pTab->n_ستون );
            أكد( VisibleRowid(pTab)==0 || ViewCanHaveRowid );
          }ورنہ{
            اگر( zTName && sqlite3StrICmp(zTName, zTabName)!=0 ){
              جاری;
            }
            pNestedFrom = 0;
            iDb = sqlite3_Schema_مقصد_اشاریہ(db, pTab->pSchema);
            zSchemaName = iDb>=0 ? db->aDb[iDb].zDbSName : "*";
          }
          اگر( i+1<pTabList->nSrc
           && pFrom[1].fg.isUsing
           && (selFlags & SF_NestedFrom)!=0
          ){
            صحیح ii;
            pUsing = pFrom[1].u3.pUsing;
            برائے(ii=0; ii<pUsing->nId; ii++){
              ثابت حرف *zUName = pUsing->a[ii].zName;
              p_دایاں = sqlite3_اظہاریہ(db, TK_ID, zUName);
              sqlite3_اظہاریہ_سیٹ_Error_انحراف(p_دایاں, iErrOfst);
              pNew = sqlite3_اظہاریہ_List_Append(pParse, pNew, p_دایاں);
              اگر( pNew ){
                ساخت اظہاریہ_List_شے *pX = &pNew->a[pNew->n_اظہاریہ-1];
                أكد( pX->zEName==0 );
                pX->zEName = sqlite3MPrintf(db,"..%s", zUName);
                pX->fg.eEName = ENAME_TAB;
                pX->fg.b_Using_رکن = 1;
              }
            }
          }ورنہ{
            pUsing = 0;
          }

          n_جوڑو = pTab->n_ستون;
          اگر( VisibleRowid(pTab) && (selFlags & SF_NestedFrom)!=0 ) n_جوڑو++;
          برائے(j=0; j<n_جوڑو; j++){
            ثابت حرف *zName; 
            ساخت اظہاریہ_List_شے *pX; /* Newly added ExprList term */

            اگر( j==pTab->n_ستون ){
              zName = sqlite3RowidAlias(pTab);
              اگر( zName==0 ) جاری;
            }ورنہ{
              zName = pTab->a_ستون[j].zCnName;

              /* If pTab is actually an SF_NestedFrom sub-select, do not
              ** expand any ENAME_ROWID columns.  */
              اگر( pNestedFrom && pNestedFrom->a[j].fg.eEName==ENAME_ROWID ){
                جاری;
              }

              اگر( zTName
               && pNestedFrom
               && sqlite3_مطابقت_EName(&pNestedFrom->a[j], 0, zTName, 0, 0)==0
              ){
                جاری;
              }

              /* If a column is marked as 'hidden', omit it from the expanded
              ** result-set list unless the SELECT has the SF_IncludeHidden
              ** bit set.
              */
              اگر( (p->selFlags & SF_IncludeHidden)==0
                && Is_Hidden_عمود(&pTab->a_ستون[j])
              ){
                جاری;
              }
              اگر( (pTab->a_ستون[j].ستون_Flags & COLFLAG_NOEXPAND)!=0
               && zTName==0
               && (selFlags & (SF_NestedFrom))==0
              ){
                جاری;
              }
            }
            أكد( zName );
            جدول_نظر_آیا = 1;

            اگر( i>0 && zTName==0 && (selFlags & SF_NestedFrom)==0 ){
              اگر( pFrom->fg.isUsing
               && sqlite3_Id_List_اشاریہ(pFrom->u3.pUsing, zName)>=0
              ){
                /* In a join with a USING clause, omit columns in the
                ** using clause from the table on the right. */
                جاری;
              }
            }
            p_دایاں = sqlite3_اظہاریہ(db, TK_ID, zName);
            اگر( (pTabList->nSrc>1
                 && (  (pFrom->fg.jointype & JT_LTORJ)==0
                     || zTName!=0
                     || (selFlags & SF_NestedFrom)!=0
                     || !اندر_Any_Using_Clause(zName,pFrom,pTabList->nSrc-i-1)
                    )
                )
             || اندر_RENAME_OBJECT
            ){
              اظہاریہ *p_بایاں;
              p_بایاں = sqlite3_اظہاریہ(db, TK_ID, zTabName);
              p_اظہاریہ = sqlite3PExpr(pParse, TK_DOT, p_بایاں, p_دایاں);
              اگر( اندر_RENAME_OBJECT && pE->p_بایاں ){
                أكد( pE->op==TK_DOT && pE->p_دایاں->op==TK_ASTERISK );
                sqlite3RenameTokenRemap(pParse, p_بایاں, pE->p_بایاں);
              }
              اگر( zSchemaName ){
                p_بایاں = sqlite3_اظہاریہ(db, TK_ID, zSchemaName);
                p_اظہاریہ = sqlite3PExpr(pParse, TK_DOT, p_بایاں, p_اظہاریہ);
              }
            }ورنہ{
              p_اظہاریہ = p_دایاں;
            }
            sqlite3_اظہاریہ_سیٹ_Error_انحراف(p_اظہاریہ, iErrOfst);
            pNew = sqlite3_اظہاریہ_List_Append(pParse, pNew, p_اظہاریہ);
            اگر( pNew==0 ){
              توڑو;  /* OOM */
            }
            pX = &pNew->a[pNew->n_اظہاریہ-1];
            أكد( pX->zEName==0 );
            اگر( (selFlags & SF_NestedFrom)!=0 && !اندر_RENAME_OBJECT ){
              اگر( pNestedFrom && (!ViewCanHaveRowid || j<pNestedFrom->n_اظہاریہ) ){
                أكد( j<pNestedFrom->n_اظہاریہ );
                pX->zEName = sqlite3DbStrDup(db, pNestedFrom->a[j].zEName);
                testcase( pX->zEName==0 );
              }ورنہ{
                pX->zEName = sqlite3MPrintf(db, "%s.%s.%s",
                                           zSchemaName, zTabName, zName);
                testcase( pX->zEName==0 );
              }
              pX->fg.eEName = (j==pTab->n_ستون ? ENAME_ROWID : ENAME_TAB);
              اگر( (pFrom->fg.isUsing
                   && sqlite3_Id_List_اشاریہ(pFrom->u3.pUsing, zName)>=0)
               || (pUsing && sqlite3_Id_List_اشاریہ(pUsing, zName)>=0)
               || (j<pTab->n_ستون && (pTab->a_ستون[j].ستون_Flags & COLFLAG_NOEXPAND))
              ){
                pX->fg.bNoExpand = 1;
              }
            }ورنہ اگر( long_نام ){
              pX->zEName = sqlite3MPrintf(db, "%s.%s", zTabName, zName);
              pX->fg.eEName = ENAME_NAME;
            }ورنہ{
              pX->zEName = sqlite3DbStrDup(db, zName);
              pX->fg.eEName = ENAME_NAME;
            }
          }
        }
        اگر( !جدول_نظر_آیا ){
          اگر( zTName ){
            sqlite3_Error_پیغام(pParse, "no such table: %s", zTName);
          }ورنہ{
            sqlite3_Error_پیغام(pParse, "no tables specified");
          }
        }
      }
    }
    sqlite3_اظہاریہ_List_Delete(db, pEList);
    p->pEList = pNew;
  }
  اگر( p->pEList ){
    اگر( p->pEList->n_اظہاریہ>db->a_حد_بندی[SQLITE_حد_بندی_عمود] ){
      sqlite3_Error_پیغام(pParse, "too many columns in result set");
      واپس WRC_Abort;
    }
    اگر( (elistFlags & (EP_HasFunc|EP_Subquery))!=0 ){
      p->selFlags |= SF_Complex_نتیجہ;
    }
  }
#اگر TREETRACE_ENABLED
  اگر( sqlite3_درخت_Trace & 0x8 ){
    TREETRACE(0x8,pParse,p,("After result-set wildcard expansion:\n"));
    sqlite3_درخت_View_Select(0, p, 0);
  }
#اگر_ختم
  واپس WRC_Continue;
}

#اگر SQLITE_DEBUG
/*
** Always assert.  This xSelectCallback2 implementation proves that the
** xSelectCallback2 is never invoked.
*/
خالی sqlite3SelectWalkAssert2(Walker *Not_مستعمل, Select *NotUsed2){
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  أكد( 0 );
}
#اگر_ختم

/*
** The first argument is a Walker object created by sqlite3SelectExpand().
** This function uses the walker to expand the SELECT statement passed
** as the second argument. If necessary, it also converts any compound
** SELECT statements that have COLLATE clauses on ORDER BY expressions
** to sub-queries. Return the return value of sqlite3WalkSelect().
*/
ساکن صحیح selectExpanderWalk(Walker *pWalker, Select *pSelect){
  أكد( pWalker->x_اظہاریہ_Callback==sqlite3_اظہاریہ_Walk_Noop );
  أكد( pWalker->pParse!=0 );
  اگر( OK_IF_ALWAYS_TRUE(pWalker->pParse->hasCompound) ){
    pWalker->xSelectCallback = convert_Compound_Select_مقصد_Subquery;
    pWalker->xSelectCallback2 = 0;
    sqlite3WalkSelect(pWalker, pSelect);
  }
  pWalker->xSelectCallback = selectExpander;
  pWalker->xSelectCallback2 = sqlite3_Select_نکالو_With;
  واپس sqlite3WalkSelect(pWalker, pSelect);
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
ساکن خالی sqlite3SelectExpand(Parse *pParse, Select *pSelect){
  Walker w;
  w.x_اظہاریہ_Callback = sqlite3_اظہاریہ_Walk_Noop;
  w.pParse = pParse;
  w.e_رمز = 0;
  selectExpanderWalk(&w, pSelect);
}


#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
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
ساکن خالی select_جوڑو_Subquery_Type_Info(Walker *pWalker, Select *p){
  Parse *pParse;
  صحیح i;
  SrcList *pTabList;
  Src_شے *pFrom;

  اگر( p->selFlags & SF_HasTypeInfo ) واپس;
  p->selFlags |= SF_HasTypeInfo;
  pParse = pWalker->pParse;
  أكد( (p->selFlags & SF_Resolved) );
  pTabList = p->pSrc;
  برائے(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    جدول *pTab = pFrom->pSTab;
    أكد( pTab!=0 );
    اگر( (pTab->tabFlags & TF_Ephemeral)!=0 && pFrom->fg.isSubquery ){
      /* A sub-query in the FROM clause of a SELECT */
      Select *pSel = pFrom->u4.pSubq->pSelect;
      sqlite3_Subquery_عمود_Types(pParse, pTab, pSel, SQLITE_AFF_NONE);
    }
  }
}
#اگر_ختم


/*
** This routine adds datatype and collating sequence information to
** the Table structures of all FROM-clause subqueries in a
** SELECT statement.
**
** Use this routine after name resolution.
*/
ساکن خالی sqlite3_Select_جوڑو_Type_Info(Parse *pParse, Select *pSelect){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
  Walker w;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.xSelectCallback2 = select_جوڑو_Subquery_Type_Info;
  w.x_اظہاریہ_Callback = sqlite3_اظہاریہ_Walk_Noop;
  w.pParse = pParse;
  sqlite3WalkSelect(&w, pSelect);
#اگر_ختم
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
خالی sqlite3SelectPrep(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for container */
){
  أكد( p!=0 || pParse->db->mallocFailed );
  أكد( pParse->db->pParse==pParse );
  اگر( pParse->db->mallocFailed ) واپس;
  اگر( p->selFlags & SF_HasTypeInfo ) واپس;
  sqlite3SelectExpand(pParse, p);
  اگر( pParse->nErr ) واپس;
  sqlite3_Resolve_Select_نام(pParse, p, pOuterNC);
  اگر( pParse->nErr ) واپس;
  sqlite3_Select_جوڑو_Type_Info(pParse, p);
}

#اگر TREETRACE_ENABLED
/*
** Display all information about an AggInfo object
*/
ساکن خالی printAggInfo(AggInfo *pAggInfo){
  صحیح ii;
  sqlite3DebugPrintf("AggInfo %d/%p:\n",
     pAggInfo->selId, pAggInfo);
  برائے(ii=0; ii<pAggInfo->n_عمود; ii++){
    ساخت Agg_Info_ستون *p_ستون = &pAggInfo->a_ستون[ii];
    sqlite3DebugPrintf(
       "agg-column[%d] pTab=%s iTable=%d iColumn=%d iMem=%d"
       " iSorterColumn=%d %s\n",
       ii, p_ستون->pTab ? p_ستون->pTab->zName : "NULL",
       p_ستون->i_جدول, p_ستون->i_عمود, pAggInfo->i_پہلا_Reg+ii,
       p_ستون->i_Sorter_عمود,
       ii>=pAggInfo->n_مجمع ? "" : " Accumulator");
    sqlite3_درخت_View_اظہاریہ(0, pAggInfo->a_ستون[ii].pCExpr, 0);
  }
  برائے(ii=0; ii<pAggInfo->nFunc; ii++){
    sqlite3DebugPrintf("agg-func[%d]: iMem=%d\n",
        ii, pAggInfo->i_پہلا_Reg+pAggInfo->n_عمود+ii);
    sqlite3_درخت_View_اظہاریہ(0, pAggInfo->aFunc[ii].pFExpr, 0);
  }
}
#اگر_ختم /* TREETRACE_ENABLED */

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
ساکن خالی analyze_Agg_Func_دلائل(
  AggInfo *pAggInfo,
  NameContext *pNC
){
  صحیح i;
  أكد( pAggInfo!=0 );
  أكد( pAggInfo->i_پہلا_Reg==0 );
  pNC->ncFlags |= NC_اندر_Agg_Func;
  برائے(i=0; i<pAggInfo->nFunc; i++){
    اظہاریہ *p_اظہاریہ = pAggInfo->aFunc[i].pFExpr;
    أكد( p_اظہاریہ->op==TK_FUNCTION || p_اظہاریہ->op==TK_AGG_FUNCTION );
    أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
    sqlite3_اظہاریہ_Analyze_Agg_List(pNC, p_اظہاریہ->x.pList);
    اگر( p_اظہاریہ->p_بایاں ){
      أكد( p_اظہاریہ->p_بایاں->op==TK_ترتیب );
      أكد( اظہاریہ_Use_XList(p_اظہاریہ->p_بایاں) );
      sqlite3_اظہاریہ_Analyze_Agg_List(pNC, p_اظہاریہ->p_بایاں->x.pList);
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    أكد( !Is_کھڑکی_Func(p_اظہاریہ) );
    اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
      sqlite3_اظہاریہ_Analyze_Aggregates(pNC, p_اظہاریہ->y.pWin->pFilter);
    }
#اگر_ختم
  }
  pNC->ncFlags &= ~NC_اندر_Agg_Func;
}

/*
** An index on expressions is being used in the inner loop of an
** aggregate query with a GROUP BY clause.  This routine attempts
** to adjust the AggInfo object to take advantage of index and to
** perhaps use the index as a covering index.
**
*/
ساکن خالی optimize_Aggregate_Use_کا_اشاریہ_دار_اظہاریہ(
  Parse *pParse,          /* Parsing context */
  Select *pSelect,        /* The SELECT statement being processed */
  AggInfo *pAggInfo,      /* The aggregate info */
  NameContext *pNC        /* Name context used to resolve agg-func args */
){
  أكد( pAggInfo->i_پہلا_Reg==0 );
  أكد( pSelect!=0 );
  أكد( pSelect->p_گروہ_By!=0 );
  pAggInfo->n_عمود = pAggInfo->n_مجمع;
  اگر( ALWAYS(pAggInfo->n_ترتیب_کاری_عمود>0) ){
    صحیح mx = pSelect->p_گروہ_By->n_اظہاریہ - 1;
    صحیح j, k;
    برائے(j=0; j<pAggInfo->n_عمود; j++){
      k = pAggInfo->a_ستون[j].i_Sorter_عمود;
      اگر( k>mx ) mx = k;
    }
    pAggInfo->n_ترتیب_کاری_عمود = mx+1;
  }
  analyze_Agg_Func_دلائل(pAggInfo, pNC);
#اگر TREETRACE_ENABLED
  اگر( sqlite3_درخت_Trace & 0x20 ){
    اشاریہ_دار_اظہاریہ *pIEpr;
    TREETRACE(0x20, pParse, pSelect,
        ("AggInfo (possibly) adjusted for Indexed Exprs\n"));
    sqlite3_درخت_View_Select(0, pSelect, 0);
    برائے(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
      لکھو("data-cursor=%d index={%d,%d}\n",
          pIEpr->i_ڈیٹا_Cur, pIEpr->iIdxCur, pIEpr->i_Idx_ستون);
      sqlite3_درخت_View_اظہاریہ(0, pIEpr->p_اظہاریہ, 0);
    }
    printAggInfo(pAggInfo);
  }
#ورنہ
  غیر_مستعمل_PARAMETER(pSelect);
  غیر_مستعمل_PARAMETER(pParse);
#اگر_ختم
}

/*
** Walker callback for aggregateConvertIndexedExprRefToColumn().
*/
ساکن صحیح aggregate_Idx_Epr_Ref_مقصد_ستون_Callback(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  AggInfo *pAggInfo;
  ساخت Agg_Info_ستون *p_ستون;
  غیر_مستعمل_PARAMETER(pWalker);
  اگر( p_اظہاریہ->pAggInfo==0 ) واپس WRC_Continue;
  اگر( p_اظہاریہ->op==TK_AGG_عمود ) واپس WRC_Continue;
  اگر( p_اظہاریہ->op==TK_AGG_FUNCTION ) واپس WRC_Continue;
  اگر( p_اظہاریہ->op==TK_IF_NULL_صف ) واپس WRC_Continue;
  pAggInfo = p_اظہاریہ->pAggInfo;
  اگر( NEVER(p_اظہاریہ->iAgg>=pAggInfo->n_عمود) ) واپس WRC_Continue;
  أكد( p_اظہاریہ->iAgg>=0 );
  p_ستون = &pAggInfo->a_ستون[p_اظہاریہ->iAgg];
  p_اظہاریہ->op = TK_AGG_عمود;
  p_اظہاریہ->i_جدول = p_ستون->i_جدول;
  p_اظہاریہ->i_عمود = p_ستون->i_عمود;
  اظہاریہ_Clear_Property(p_اظہاریہ, EP_Skip|EP_Collate|EP_Unlikely);
  واپس WRC_Prune;
}

/*
** Convert every pAggInfo->aFunc[].pExpr such that any node within
** those expressions that has pAppInfo set is changed into a TK_AGG_COLUMN
** opcode.
*/
ساکن خالی aggregate_Convert_اشاریہ_دار_اظہاریہ_Ref_مقصد_عمود(AggInfo *pAggInfo){
  صحیح i;
  Walker w;
  memset(&w, 0, حجم(w));
  w.x_اظہاریہ_Callback = aggregate_Idx_Epr_Ref_مقصد_ستون_Callback;
  برائے(i=0; i<pAggInfo->nFunc; i++){
    sqlite3_Walk_اظہاریہ(&w, pAggInfo->aFunc[i].pFExpr);
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
ساکن خالی تفویض_Aggregate_Registers(Parse *pParse, AggInfo *pAggInfo){
  أكد( pAggInfo!=0 );
  أكد( pAggInfo->i_پہلا_Reg==0 );
  pAggInfo->i_پہلا_Reg = pParse->nMem + 1;
  pParse->nMem += pAggInfo->n_عمود + pAggInfo->nFunc;
}

/*
** Reset the aggregate accumulator.
**
** The aggregate accumulator is a set of memory cells that hold
** intermediate results while calculating an aggregate.  This
** routine generates code that stores NULLs in all of those memory
** cells.
*/
ساکن خالی reset_مجمع(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  صحیح i;
  ساخت AggInfo_func *pFunc;
  صحیح nReg = pAggInfo->nFunc + pAggInfo->n_عمود;
  أكد( pAggInfo->i_پہلا_Reg>0 );
  أكد( pParse->db->pParse==pParse );
  أكد( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  اگر( nReg==0 ) واپس;
  اگر( pParse->nErr ) واپس;
  sqlite3_Vdbe_جوڑو_Op3(v, OP_Null, 0, pAggInfo->i_پہلا_Reg,
                    pAggInfo->i_پہلا_Reg+nReg-1);
  برائے(pFunc=pAggInfo->aFunc, i=0; i<pAggInfo->nFunc; i++, pFunc++){
    اگر( pFunc->iDistinct>=0 ){
      اظہاریہ *pE = pFunc->pFExpr;
      أكد( اظہاریہ_Use_XList(pE) );
      اگر( pE->x.pList==0 || pE->x.pList->n_اظہاریہ!=1 ){
        sqlite3_Error_پیغام(pParse, "DISTINCT aggregates must have exactly one "
           "argument");
        pFunc->iDistinct = -1;
      }ورنہ{
        کلید_Info *p_کلید_Info = sqlite3_کلید_Info_From_اظہاریہ_List(pParse, pE->x.pList,0,0);
        pFunc->iDistAddr = sqlite3_Vdbe_جوڑو_Op4(v, OP_OpenEphemeral,
            pFunc->iDistinct, 0, 0, (حرف*)p_کلید_Info, P4_KEYINFO);
        ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(DISTINCT)",
                          pFunc->pFunc->zName));
      }
    }
    اگر( pFunc->iOBTab>=0 ){
      اظہاریہ_List *pOBList;
      کلید_Info *p_کلید_Info;
      صحیح nExtra = 0;
      أكد( pFunc->pFExpr->p_بایاں!=0 );
      أكد( pFunc->pFExpr->p_بایاں->op==TK_ترتیب );
      أكد( اظہاریہ_Use_XList(pFunc->pFExpr->p_بایاں) );
      أكد( pFunc->pFunc!=0 );
      pOBList = pFunc->pFExpr->p_بایاں->x.pList;
      اگر( !pFunc->bOBUnique ){
        nExtra++;  /* One extra column for the OP_Sequence */
      }
      اگر( pFunc->bOBPayload ){
        /* extra columns for the function arguments */
        أكد( اظہاریہ_Use_XList(pFunc->pFExpr) );
        أكد( pFunc->pFExpr->x.pList!=0 );
        nExtra += pFunc->pFExpr->x.pList->n_اظہاریہ;
      }
      اگر( pFunc->bUseSubtype ){
        nExtra += pFunc->pFExpr->x.pList->n_اظہاریہ;
      }
      p_کلید_Info = sqlite3_کلید_Info_From_اظہاریہ_List(pParse, pOBList, 0, nExtra);
      اگر( !pFunc->bOBUnique && pParse->nErr==0 ){
        p_کلید_Info->n_کلید_میدان++;
      }
      sqlite3_Vdbe_جوڑو_Op4(v, OP_OpenEphemeral,
            pFunc->iOBTab, pOBList->n_اظہاریہ+nExtra, 0,
            (حرف*)p_کلید_Info, P4_KEYINFO);
      ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(ORDER BY)",
                          pFunc->pFunc->zName));
    }
  }
}

/*
** Invoke the OP_AggFinalize opcode for every aggregate function
** in the AggInfo structure.
*/
ساکن خالی finalizeAggFunctions(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  صحیح i;
  ساخت AggInfo_func *pF;
  برائے(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    اظہاریہ_List *pList;
    أكد( اظہاریہ_Use_XList(pF->pFExpr) );
    اگر( pParse->nErr ) واپس;
    pList = pF->pFExpr->x.pList;
    اگر( pF->iOBTab>=0 ){
      /* For an ORDER BY aggregate, calls to OP_AggStep were deferred.  Inputs
      ** were stored in emphermal table pF->iOBTab.  Here, we extract those
      ** inputs (in ORDER BY order) and make all calls to OP_AggStep
      ** before doing the OP_AggFinal call. */
      صحیح i_چوٹی;        /* Start of loop for extracting columns */
      صحیح nArg;        /* Number of columns to extract */
      صحیح n_کلید;        /* Key columns to be skipped */
      صحیح regAgg;      /* Extract into this array */
      صحیح j;           /* Loop counter */
     
      أكد( pF->pFunc!=0 );
      nArg = pList->n_اظہاریہ;
      regAgg = sqlite3_Get_عارضی_Range(pParse, nArg);

      اگر( pF->bOBPayload==0 ){
        n_کلید = 0;
      }ورنہ{
        أكد( pF->pFExpr->p_بایاں!=0 );
        أكد( اظہاریہ_Use_XList(pF->pFExpr->p_بایاں) );
        أكد( pF->pFExpr->p_بایاں->x.pList!=0 );
        n_کلید = pF->pFExpr->p_بایاں->x.pList->n_اظہاریہ;
        اگر( ALWAYS(!pF->bOBUnique) ) n_کلید++;
      }
      i_چوٹی = sqlite3_Vdbe_جوڑو_Op1(v, OP_Rewind, pF->iOBTab); VdbeCoverage(v);
      برائے(j=nArg-1; j>=0; j--){
        sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, pF->iOBTab, n_کلید+j, regAgg+j);
      }
      اگر( pF->bUseSubtype ){
        صحیح regSubtype = sqlite3_Get_عارضی_Reg(pParse);
        صحیح i_بنیاد_ستون = n_کلید + nArg + (pF->bOBPayload==0 && pF->bOBUnique==0);
        برائے(j=nArg-1; j>=0; j--){
          sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, pF->iOBTab, i_بنیاد_ستون+j, regSubtype);
          sqlite3_Vdbe_جوڑو_Op2(v, OP_سیٹ_Subtype, regSubtype, regAgg+j);
        }
        sqlite3_Release_عارضی_Reg(pParse, regSubtype);
      }
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Agg_قدم, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_اگلا, pF->iOBTab, i_چوٹی+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_یہاں(v, i_چوٹی);
      sqlite3_Release_عارضی_Range(pParse, regAgg, nArg);
    }
    sqlite3_Vdbe_جوڑو_Op2(v, OP_AggFinal, AggInfoFuncReg(pAggInfo,i),
                      pList ? pList->n_اظہاریہ : 0);
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
ساکن خالی update_مجمع(
  Parse *pParse,
  صحیح regAcc,
  AggInfo *pAggInfo,
  صحیح eDistinctType
){
  Vdbe *v = pParse->pVdbe;
  صحیح i;
  صحیح reg_کامیابی = 0;
  صحیح addr_کامیابی_Test = 0;
  ساخت AggInfo_func *pF;
  ساخت Agg_Info_ستون *pC;

  أكد( pAggInfo->i_پہلا_Reg>0 );
  اگر( pParse->nErr ) واپس;
  pAggInfo->براہ_راست_منوال = 1;
  برائے(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    صحیح nArg;
    صحیح addr_اگلا = 0;
    صحیح regAgg;
    صحیح regAggSz = 0;
    صحیح regDistinct = 0;
    اظہاریہ_List *pList;
    أكد( اظہاریہ_Use_XList(pF->pFExpr) );
    أكد( !Is_کھڑکی_Func(pF->pFExpr) );
    أكد( pF->pFunc!=0 );
    pList = pF->pFExpr->x.pList;
    اگر( اظہاریہ_Has_Property(pF->pFExpr, EP_WinFunc) ){
      اظہاریہ *pFilter = pF->pFExpr->y.pWin->pFilter;
      اگر( pAggInfo->n_مجمع
       && (pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)
       && regAcc
      ){
        /* If regAcc==0, there there exists some min() or max() function
        ** without a FILTER clause that will ensure the magnet registers
        ** are populated. */
        اگر( reg_کامیابی==0 ) reg_کامیابی = ++pParse->nMem;
        /* If this is the first row of the group (regAcc contains 0), clear the
        ** "magnet" register regHit so that the accumulator registers
        ** are populated if the FILTER clause jumps over the the
        ** invocation of min() or max() altogether. Or, if this is not
        ** the first row (regAcc contains 1), set the magnet register so that
        ** the accumulators are not populated unless the min()/max() is invoked
        ** and indicates that they should be.  */
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Copy, regAcc, reg_کامیابی);
      }
      addr_اگلا = sqlite3VdbeMakeLabel(pParse);
      sqlite3_اظہاریہ_If_False(pParse, pFilter, addr_اگلا, SQLITE_JUMPIFNULL);
    }
    اگر( pF->iOBTab>=0 ){
      /* Instead of invoking AggStep, we must push the arguments that would
      ** have been passed to AggStep onto the sorting table. */
      صحیح jj;                /* Registered used so far in building the record */
      اظہاریہ_List *pOBList;     /* The ORDER BY clause */
      أكد( pList!=0 );
      nArg = pList->n_اظہاریہ;
      أكد( nArg>0 );
      أكد( pF->pFExpr->p_بایاں!=0 );
      أكد( pF->pFExpr->p_بایاں->op==TK_ترتیب );
      أكد( اظہاریہ_Use_XList(pF->pFExpr->p_بایاں) );
      pOBList = pF->pFExpr->p_بایاں->x.pList;
      أكد( pOBList!=0 );
      أكد( pOBList->n_اظہاریہ>0 );
      regAggSz = pOBList->n_اظہاریہ;
      اگر( !pF->bOBUnique ){
        regAggSz++;   /* One register for OP_Sequence */
      }
      اگر( pF->bOBPayload ){
        regAggSz += nArg;
      }
      اگر( pF->bUseSubtype ){
        regAggSz += nArg;
      }
      regAggSz++;  /* One extra register to hold result of MakeRecord */
      regAgg = sqlite3_Get_عارضی_Range(pParse, regAggSz);
      regDistinct = regAgg;
      sqlite3_اظہاریہ_رمز_اظہاریہ_List(pParse, pOBList, regAgg, 0, SQLITE_ECEL_DUP);
      jj = pOBList->n_اظہاریہ;
      اگر( !pF->bOBUnique ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Sequence, pF->iOBTab, regAgg+jj);
        jj++;
      }
      اگر( pF->bOBPayload ){
        regDistinct = regAgg+jj;
        sqlite3_اظہاریہ_رمز_اظہاریہ_List(pParse, pList, regDistinct, 0, SQLITE_ECEL_DUP);
        jj += nArg;
      }
      اگر( pF->bUseSubtype ){
        صحیح kk;
        صحیح reg_بنیاد = pF->bOBPayload ? regDistinct : regAgg;
        برائے(kk=0; kk<nArg; kk++, jj++){
          sqlite3_Vdbe_جوڑو_Op2(v, OP_GetSubtype, reg_بنیاد+kk, regAgg+jj);
        }
      }
    }ورنہ اگر( pList ){
      nArg = pList->n_اظہاریہ;
      regAgg = sqlite3_Get_عارضی_Range(pParse, nArg);
      regDistinct = regAgg;
      sqlite3_اظہاریہ_رمز_اظہاریہ_List(pParse, pList, regAgg, 0, SQLITE_ECEL_DUP);
    }ورنہ{
      nArg = 0;
      regAgg = 0;
    }
    اگر( pF->iDistinct>=0 && pList ){
      اگر( addr_اگلا==0 ){
        addr_اگلا = sqlite3VdbeMakeLabel(pParse);
      }
      pF->iDistinct = رمز_Distinct(pParse, eDistinctType,
          pF->iDistinct, addr_اگلا, pList, regDistinct);
    }
    اگر( pF->iOBTab>=0 ){
      /* Insert a new record into the ORDER BY table */
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, regAgg, regAggSz-1,
                        regAgg+regAggSz-1);
      sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_Idx_داخل_کرو, pF->iOBTab, regAgg+regAggSz-1,
                           regAgg, regAggSz-1);
      sqlite3_Release_عارضی_Range(pParse, regAgg, regAggSz);
    }ورنہ{
      /* Invoke the AggStep function */
      اگر( pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        CollSeq *pColl = 0;
        ساخت اظہاریہ_List_شے *p_شے;
        صحیح j;
        أكد( pList!=0 );  /* pList!=0 if pF->pFunc has NEEDCOLL */
        برائے(j=0, p_شے=pList->a; !pColl && j<nArg; j++, p_شے++){
          pColl = sqlite3_اظہاریہ_Coll_Seq(pParse, p_شے->p_اظہاریہ);
        }
        اگر( !pColl ){
          pColl = pParse->db->pDfltColl;
        }
        اگر( reg_کامیابی==0 && pAggInfo->n_مجمع ) reg_کامیابی = ++pParse->nMem;
        sqlite3_Vdbe_جوڑو_Op4(v, OP_CollSeq, reg_کامیابی, 0, 0,
                         (حرف *)pColl, P4_COLLSEQ);
      }
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Agg_قدم, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3_Release_عارضی_Range(pParse, regAgg, nArg);
    }
    اگر( addr_اگلا ){
      sqlite3VdbeResolveLabel(v, addr_اگلا);
    }
    اگر( pParse->nErr ) واپس;
  }
  اگر( reg_کامیابی==0 && pAggInfo->n_مجمع ){
    reg_کامیابی = regAcc;
  }
  اگر( reg_کامیابی ){
    addr_کامیابی_Test = sqlite3_Vdbe_جوڑو_Op1(v, OP_If, reg_کامیابی); VdbeCoverage(v);
  }
  برائے(i=0, pC=pAggInfo->a_ستون; i<pAggInfo->n_مجمع; i++, pC++){
    sqlite3_اظہاریہ_رمز(pParse, pC->pCExpr, Agg_Info_عمود_Reg(pAggInfo,i));
    اگر( pParse->nErr ) واپس;
  }

  pAggInfo->براہ_راست_منوال = 0;
  اگر( addr_کامیابی_Test ){
    sqlite3_Vdbe_Jump_یہاں_Or_نکالو_Inst(v, addr_کامیابی_Test);
  }
}

/*
** Add a single OP_Explain instruction to the VDBE to explain a simple
** count(*) query ("SELECT count(*) FROM pTab").
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_EXPLAIN
ساکن خالی explain_Simple_گنتی(
  Parse *pParse,                  /* Parse context */
  جدول *pTab,                    /* Table being queried */
  اشاریہ *pIdx                     /* Index used to optimize scan, or NULL */
){
  اگر( pParse->explain==2 ){
    صحیح bCover = (pIdx!=0 && (HasRowid(pTab) || !Is_Primary_کلید_اشاریہ(pIdx)));
    sqlite3VdbeExplain(pParse, 0, "SCAN %s%s%s",
        pTab->zName,
        bCover ? " USING COVERING INDEX " : "",
        bCover ? pIdx->zName : ""
    );
  }
}
#ورنہ
# define explain_Simple_گنتی(a,b,c)
#اگر_ختم

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
ساکن صحیح having_مقصد_Where_اظہاریہ_Cb(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  اگر( p_اظہاریہ->op!=TK_AND ){
    Select *pS = pWalker->u.pSelect;
    /* This routine is called before the HAVING clause of the current
    ** SELECT is analyzed for aggregates. So if pExpr->pAggInfo is set
    ** here, it indicates that the expression is a correlated reference to a
    ** column from an outer aggregate query, or an aggregate function that
    ** belongs to an outer query. Do not move the expression to the WHERE
    ** clause in this obscure case, as doing so may corrupt the outer Select
    ** statements AggInfo structure.  */
    اگر( sqlite3_اظہاریہ_Is_Constant_Or_گروہ_By(pWalker->pParse, p_اظہاریہ, pS->p_گروہ_By)
     && اظہاریہ_Always_False(p_اظہاریہ)==0
     && p_اظہاریہ->pAggInfo==0
    ){
      sqlite3 *db = pWalker->pParse->db;
      اظہاریہ *pNew = sqlite3_اظہاریہ_Int32(db, 1);
      اگر( pNew ){
        اظہاریہ *pWhere = pS->pWhere;
        تبادلہ(اظہاریہ, *pNew, *p_اظہاریہ);
        pNew = sqlite3_اظہاریہ_And(pWalker->pParse, pWhere, pNew);
        pS->pWhere = pNew;
        pWalker->e_رمز = 1;
      }
    }
    واپس WRC_Prune;
  }
  واپس WRC_Continue;
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
ساکن خالی having_مقصد_Where(Parse *pParse, Select *p){
  Walker sWalker;
  memset(&sWalker, 0, حجم(sWalker));
  sWalker.pParse = pParse;
  sWalker.x_اظہاریہ_Callback = having_مقصد_Where_اظہاریہ_Cb;
  sWalker.u.pSelect = p;
  sqlite3_Walk_اظہاریہ(&sWalker, p->pHaving);
#اگر TREETRACE_ENABLED
  اگر( sWalker.e_رمز && (sqlite3_درخت_Trace & 0x100)!=0 ){
    TREETRACE(0x100,pParse,p,("Move HAVING terms into WHERE:\n"));
    sqlite3_درخت_View_Select(0, p, 0);
  }
#اگر_ختم
}

/*
** Check to see if the pThis entry of pTabList is a self-join of another view.
** Search FROM-clause entries in the range of iFirst..iEnd, including iFirst
** but stopping before iEnd.
**
** If pThis is a self-join, then return the SrcItem for the first other
** instance of that view found.  If pThis is not a self-join then return 0.
*/
ساکن Src_شے *isSelfJoinView(
  SrcList *pTabList,           /* Search for self-joins in this FROM clause */
  Src_شے *pThis,              /* Search for prior reference to this subquery */
  صحیح i_پہلا, صحیح i_انتہا        /* Range of FROM-clause entries to search. */
){
  Src_شے *p_شے;
  Select *pSel;
  أكد( pThis->fg.isSubquery );
  pSel = pThis->u4.pSubq->pSelect;
  أكد( pSel!=0 );
  اگر( pSel->selFlags & SF_ڈالو_نیچے_کو ) واپس 0;
  جب( i_پہلا<i_انتہا ){
    Select *pS1;
    p_شے = &pTabList->a[i_پہلا++];
    اگر( !p_شے->fg.isSubquery ) جاری;
    اگر( p_شے->fg.بذریعہ_Coroutine ) جاری;
    اگر( p_شے->zName==0 ) جاری;
    أكد( p_شے->pSTab!=0 );
    أكد( pThis->pSTab!=0 );
    اگر( p_شے->pSTab->pSchema!=pThis->pSTab->pSchema ) جاری;
    اگر( sqlite3_stricmp(p_شے->zName, pThis->zName)!=0 ) جاری;
    pS1 = p_شے->u4.pSubq->pSelect;
    اگر( p_شے->pSTab->pSchema==0 && pSel->selId!=pS1->selId ){
      /* The query flattener left two different CTE tables with identical
      ** names in the same FROM clause. */
      جاری;
    }
    اگر( pS1->selFlags & SF_ڈالو_نیچے_کو ){
      /* The view was modified by some other optimization such as
      ** pushDownWhereTerms() */
      جاری;
    }
    واپس p_شے;
  }
  واپس 0;
}

/*
** Deallocate a single AggInfo object
*/
ساکن خالی agginfoFree(sqlite3 *db, خالی *pArg){
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
ساکن صحیح گنتی_کا_View_Optimization(Parse *pParse, Select *p){
  Select *pSub, *p_پیشگی;
  اظہاریہ *p_اظہاریہ;
  اظہاریہ *p_گنتی;
  sqlite3 *db;
  Src_شے *pFrom;
  اگر( (p->selFlags & SF_Aggregate)==0 ) واپس 0;   /* This is an aggregate */
  اگر( p->pEList->n_اظہاریہ!=1 ) واپس 0;               /* Single result column */
  اگر( p->pWhere ) واپس 0;
  اگر( p->pHaving ) واپس 0;
  اگر( p->p_گروہ_By ) واپس 0;
  اگر( p->p_ترتیب_By ) واپس 0;
  p_اظہاریہ = p->pEList->a[0].p_اظہاریہ;
  اگر( p_اظہاریہ->op!=TK_AGG_FUNCTION ) واپس 0;        /* Result is an aggregate */
  أكد( اظہاریہ_Use_UToken(p_اظہاریہ) );
  اگر( sqlite3_stricmp(p_اظہاریہ->u.zToken,"count") ) واپس 0;  /* Is count() */
  أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
  اگر( p_اظہاریہ->x.pList!=0 ) واپس 0;                 /* Must be count(*) */
  اگر( p->pSrc->nSrc!=1 ) واپس 0;                  /* One table in FROM  */
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ) واپس 0;/* Not a window function */
  pFrom = p->pSrc->a;
  اگر( pFrom->fg.isSubquery==0 ) واپس 0;           /* FROM is a subquery */
  اگر( (p->selFlags & SF_Correlated)!=0 ) واپس 0;  /* Not a correlated subq */
  pSub = pFrom->u4.pSubq->pSelect;
  اگر( pSub->p_پیشگی==0 ) واپس 0;                   /* Must be a compound */
  اگر( pSub->selFlags & SF_CopyCte ) واپس 0;       /* Not a CTE */
  کرو{
    اگر( pSub->op!=TK_ALL && pSub->p_پیشگی ) واپس 0;  /* Must be UNION ALL */
    اگر( pSub->pWhere ) واپس 0;                      /* No WHERE clause */
    اگر( pSub->p_حد_بندی ) واپس 0;                      /* No LIMIT clause */
    اگر( pSub->selFlags & (SF_Aggregate|SF_Distinct) ){
       testcase( pSub->selFlags & SF_Aggregate );
       testcase( pSub->selFlags & SF_Distinct );
       واپس 0;     /* Not an aggregate nor DISTINCT */
    }
    أكد( pSub->pHaving==0 );  /* Due to the previous */
    pSub = pSub->p_پیشگی;                              /* Repeat over compound */
  }جب( pSub );

  /* If we reach this point then it is OK to perform the transformation */

  db = pParse->db;
  p_گنتی = p_اظہاریہ;
  p_اظہاریہ = 0;
  pSub = sqlite3SubqueryDetach(db, pFrom);
  sqlite3SrcListDelete(db, p->pSrc);
  p->pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  جب( pSub ){
    اظہاریہ *p_رکن;
    p_پیشگی = pSub->p_پیشگی;
    pSub->p_پیشگی = 0;
    pSub->p_اگلا = 0;
    pSub->selFlags |= SF_Aggregate;
    pSub->selFlags &= ~(u32)SF_Compound;
    pSub->n_Select_صف = 0;
    sqlite3_Parser_جوڑو_Cleanup(pParse, sqlite3_اظہاریہ_List_Delete_Generic, pSub->pEList);
    p_رکن = p_پیشگی ? sqlite3_اظہاریہ_Dup(db, p_گنتی, 0) : p_گنتی;
    pSub->pEList = sqlite3_اظہاریہ_List_Append(pParse, 0, p_رکن);
    p_رکن = sqlite3PExpr(pParse, TK_SELECT, 0, 0);
    sqlite3_PExpr_جوڑو_Select(pParse, p_رکن, pSub);
    اگر( p_اظہاریہ==0 ){
      p_اظہاریہ = p_رکن;
    }ورنہ{
      p_اظہاریہ = sqlite3PExpr(pParse, TK_PLUS, p_رکن, p_اظہاریہ);
    }
    اگر( pParse->nErr ){
      sqlite3_اظہاریہ_Delete(db, p_اظہاریہ);
      p_اظہاریہ = 0;
    }
    pSub = p_پیشگی;
  }
  p->pEList->a[0].p_اظہاریہ = p_اظہاریہ;
  p->selFlags &= ~(u32)SF_Aggregate;

#اگر TREETRACE_ENABLED
  اگر( sqlite3_درخت_Trace & 0x200 ){
    TREETRACE(0x200,pParse,p,("After count-of-view optimization:\n"));
    sqlite3_درخت_View_Select(0, p, 0);
  }
#اگر_ختم
  واپس 1;
}

/*
** If any term of pSrc, or any SF_NestedFrom sub-query, is not the same
** as pSrcItem but has the same alias as p0, then return true.
** Otherwise return false.
*/
ساکن صحیح sameSrcAlias(Src_شے *p0, SrcList *pSrc){
  صحیح i;
  برائے(i=0; i<pSrc->nSrc; i++){
    Src_شے *p1 = &pSrc->a[i];
    اگر( p1==p0 ) جاری;
    اگر( p0->pSTab==p1->pSTab && 0==sqlite3_stricmp(p0->zAlias, p1->zAlias) ){
      واپس 1;
    }
    اگر( p1->fg.isSubquery
     && (p1->u4.pSubq->pSelect->selFlags & SF_NestedFrom)!=0
     && sameSrcAlias(p0, p1->u4.pSubq->pSelect->pSrc)
    ){
      واپس 1;
    }
  }
  واپس 0;
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
ساکن صحیح from_Clause_رکن_Can_Be_Coroutine(
  Parse *pParse,          /* Parsing context */
  SrcList *pTabList,      /* FROM clause */
  صحیح i,                  /* Which term of the FROM clause holds the subquery */
  صحیح selFlags            /* Flags on the SELECT statement */
){
  Src_شے *p_شے = &pTabList->a[i];
  اگر( p_شے->fg.isCte ){
    ثابت CteUse *pCteUse = p_شے->u2.pCteUse;
    اگر( pCteUse->eM10d==M10d_Yes ) واپس 0;                          /* (2a) */
    اگر( pCteUse->nUse>=2 && pCteUse->eM10d!=M10d_No ) واپس 0;       /* (2b) */
  }
  اگر( pTabList->a[0].fg.jointype & JT_LTORJ ) واپس 0;               /* (3)  */
  اگر( OptimizationDisabled(pParse->db, SQLITE_Coroutines) ) واپس 0; /* (4)  */
  اگر( isSelfJoinView(pTabList, p_شے, i+1, pTabList->nSrc)!=0 ){
    واپس 0;                                                          /* (5) */
  }
  اگر( i==0 ){
    اگر( pTabList->nSrc==1 ) واپس 1;                             /* (1a) */
    اگر( pTabList->a[1].fg.jointype & JT_CROSS ) واپس 1;         /* (1b) */
    اگر( selFlags & SF_UpdateFrom )              واپس 0;         /* (1c-iii) */
    واپس 1;
  }
  اگر( selFlags & SF_UpdateFrom ) واپس 0;                        /* (1c-iii) */
  جب( 1 /*exit-by-break*/ ){
    اگر( p_شے->fg.jointype & (JT_OUTER|JT_CROSS)  ) واپس 0;     /* (1c-ii) */
    اگر( i==0 ) توڑو;
    i--;
    p_شے--;
    اگر( p_شے->fg.isSubquery ) واپس 0;                          /* (1c-i) */
  }
  واپس 1;
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
ساکن SQLITE_NOINLINE خالی exists_مقصد_Join(
  Parse *pParse,  /* Parsing context */
  Select *p,      /* The SELECT statement being optimized */
  اظہاریہ *pWhere    /* part of the WHERE clause currently being examined */
){
  اگر( pParse->nErr==0
   && pWhere!=0
   && !اظہاریہ_Has_Property(pWhere, EP_OuterON|EP_InnerON) 
   && ALWAYS(p->pSrc!=0)
   && p->pSrc->nSrc<BMS
   && (p->p_حد_بندی==0 || p->p_حد_بندی->p_دایاں==0)
  ){
    اگر( pWhere->op==TK_AND ){
      اظہاریہ *p_دایاں = pWhere->p_دایاں;
      exists_مقصد_Join(pParse, p, pWhere->p_بایاں);
      exists_مقصد_Join(pParse, p, p_دایاں);
    }
    ورنہ اگر( pWhere->op==TK_EXISTS ){
      Select *pSub = pWhere->x.pSelect;
      اظہاریہ *pSubWhere = pSub->pWhere;
      اگر( pSub->pSrc->nSrc==1
       && (pSub->selFlags & SF_Aggregate)==0
       && !pSub->pSrc->a[0].fg.isSubquery
       && pSub->p_حد_بندی==0
       && pSub->p_پیشگی==0
      ){
        /* Before combining the sub-select with the parent, renumber the 
        ** cursor used by the subselect. This is because the EXISTS expression
        ** might be a copy of another EXISTS expression from somewhere
        ** else in the tree, and in this case it is important that it use
        ** a unique cursor number.  */
        sqlite3 *db = pParse->db;
        صحیح *aCsrMap = sqlite3DbMallocZero(db, (pParse->nTab+2)*حجم(صحیح));
        اگر( aCsrMap==0 ) واپس;
        aCsrMap[0] = (pParse->nTab+1);
        renumberCursors(pParse, pSub, -1, aCsrMap);
        sqlite3DbFree(db, aCsrMap);

        memset(pWhere, 0, حجم(*pWhere));
        pWhere->op = TK_INTEGER;
        pWhere->u.i_قدر = 1;
        اظہاریہ_سیٹ_Property(pWhere, EP_Int_قدر);
        أكد( p->pWhere!=0 );
        pSub->pSrc->a[0].fg.fromExists = 1;
        p->pSrc = sqlite3SrcListAppendList(pParse, p->pSrc, pSub->pSrc);
        اگر( pSubWhere ){
          p->pWhere = sqlite3PExpr(pParse, TK_AND, p->pWhere, pSubWhere);
          pSub->pWhere = 0;
        }
        pSub->pSrc = 0;
        sqlite3_Parser_جوڑو_Cleanup(pParse, sqlite3SelectDeleteGeneric, pSub);
        recompute_Columns_مستعمل(p, &p->pSrc->a[p->pSrc->nSrc-1]);
#اگر TREETRACE_ENABLED
        اگر( sqlite3_درخت_Trace & 0x100000 ){
          TREETRACE(0x100000,pParse,p,
                    ("After EXISTS-to-JOIN optimization:\n"));
          sqlite3_درخت_View_Select(0, p, 0);
        }
#اگر_ختم
      }
    }
  }
}

/*
** Type used for Walker callbacks by selectCheckOnClauses().
*/
قسمتعریف ساخت CheckOnCtx CheckOnCtx;
ساخت CheckOnCtx {
  SrcList *pSrc;       /* SrcList for this context */
  صحیح iJoin;           /* Cursors must be left of this one, if not negative */
  صحیح bFuncArg;        /* True for table-function arg */
  CheckOnCtx *p_والد; /* Parent context */
};

/*
** True if the SrcList passed as the only argument contains at least
** one RIGHT or FULL JOIN. False otherwise.  
*/
#تعریف_کرو has_دایاں_Join(pSrc) (((pSrc)->a[0].fg.jointype & JT_LTORJ)!=0)

/*
** The xExpr callback for the search of invalid ON clause terms.
*/
ساکن صحیح select_Check_On_Clauses_اظہاریہ(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;

  /* Check if pExpr is root or near-root of an ON clause constraint that needs
  ** to be checked to ensure that it does not refer to tables in its FROM
  ** clause to the right of itself. i.e. it is either:
  **
  **   + an ON clause on an OUTER join, or
  **   + an ON clause on an INNER join within a FROM that features at
  **     least one RIGHT or FULL join.
  */
  اگر( (اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON))
   || (اظہاریہ_Has_Property(p_اظہاریہ, EP_InnerON) && has_دایاں_Join(pCtx->pSrc))
  ){
    /* If CheckOnCtx.iJoin is already set, then fall through and process
    ** this expression node as normal. Or, if CheckOnCtx.iJoin is still 0,
    ** set it to the cursor number of the RHS of the join to which this
    ** ON expression was attached and then iterate through the entire 
    ** expression.  */
    أكد( pCtx->iJoin<0 || pCtx->iJoin==p_اظہاریہ->w.iJoin );
    اگر( pCtx->iJoin<0 ){
      pCtx->iJoin = p_اظہاریہ->w.iJoin;
      sqlite3_Walk_اظہاریہ_NN(pWalker, p_اظہاریہ);
      pCtx->iJoin = -1;
      واپس WRC_Prune;
    }
  }

  اگر( p_اظہاریہ->op==TK_عمود ){
    /* A column expression. Find the SrcList (if any) to which it refers.
    ** Then, if CheckOnCtx.iJoin indicates that this expression is part of an
    ** ON clause from that SrcList (i.e. if iJoin is non-zero), check that it
    ** does not refer to a table to the right of CheckOnCtx.iJoin. */
    صحیح iTab = p_اظہاریہ->i_جدول;
    کرو {
      SrcList *pSrc = pCtx->pSrc;
      صحیح nSrc = pSrc->nSrc;
      صحیح ii;
      برائے(ii=0; ii<nSrc && pSrc->a[ii].iCursor!=iTab; ii++){}
      اگر( ii<nSrc ){
        /* pSrc is the FROM clause that contains iTab */
        اگر( pCtx->iJoin>=0 ){
          برائے(ii--; ii>=0 && pSrc->a[ii].iCursor!=pCtx->iJoin; ii--){}
          اگر( ii>=0 ){
            /* Table iJoin appears to the left of table iTab in the SrcList.
            ** Therefore the expression refers to a table to its right. */
            sqlite3_Error_پیغام(pWalker->pParse, 
                "%s references tables to its right",
                (pCtx->bFuncArg ? "table-function argument" : "ON clause")
            );
            واپس WRC_Abort;
          }
        }
        توڑو;
      }
      pCtx = pCtx->p_والد;
    }جب( pCtx );
  }
  واپس WRC_Continue;
}

/*
** The xSelect callback for the search of invalid ON clause terms.
*/
ساکن صحیح selectCheckOnClausesSelect(Walker *pWalker, Select *pSelect){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;
  اگر( pSelect->pSrc==pCtx->pSrc || pSelect->pSrc->nSrc==0 ){
    واپس WRC_Continue;
  }ورنہ{
    CheckOnCtx sCtx;
    memset(&sCtx, 0, حجم(sCtx));
    sCtx.pSrc = pSelect->pSrc;
    sCtx.p_والد = pCtx;
    sCtx.iJoin = -1;
    pWalker->u.pCheckOnCtx = &sCtx;
    sqlite3WalkSelect(pWalker, pSelect);
    pWalker->u.pCheckOnCtx = pCtx;
    pSelect->selFlags &= ~SF_On_مقصد_Where;
    واپس WRC_Prune;
  }
}

/*
** Check all ON clauses in pSelect to verify that they do not reference
** columns to the right.
*/
خالی sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect){
  Walker w;
  CheckOnCtx sCtx;
  صحیح ii;
  أكد( pSelect->selFlags & SF_On_مقصد_Where );
  أكد( pSelect->pSrc!=0 && pSelect->pSrc->nSrc>=2 );
  memset(&w, 0, حجم(w));
  w.pParse = pParse;
  w.x_اظہاریہ_Callback = select_Check_On_Clauses_اظہاریہ;
  w.xSelectCallback = selectCheckOnClausesSelect;
  w.u.pCheckOnCtx = &sCtx;
  memset(&sCtx, 0, حجم(sCtx));
  sCtx.pSrc = pSelect->pSrc;
  sCtx.iJoin = -1;
  sqlite3_Walk_اظہاریہ(&w, pSelect->pWhere);
  pSelect->selFlags &= ~SF_On_مقصد_Where;

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
  برائے(ii=0; ii<pSelect->pSrc->nSrc; ii++){
    Src_شے *p_شے = &pSelect->pSrc->a[ii];
    اگر( p_شے->fg.isTabFunc ){
      sCtx.iJoin = -1;
      اگر( (p_شے->fg.jointype & (JT_LTORJ|JT_دایاں)) ){
        sCtx.iJoin = p_شے->iCursor;
      }ورنہ{
        صحیح jj;
        برائے(jj=ii+1; jj<pSelect->pSrc->nSrc; jj++){
          Src_شے *pItem2 = &pSelect->pSrc->a[jj];
          اگر( pItem2->fg.jointype & JT_OUTER ){
            sCtx.iJoin = pItem2[-1].iCursor;
            توڑو;
          }
        }
      }
      اگر( sCtx.iJoin>=0 ){
        sqlite3_Walk_اظہاریہ_List(&w, p_شے->u1.pFuncArg);
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
ساکن صحیح sqlite3_Copy_ترتیب_دو_ترتیب(اظہاریہ_List *p1, اظہاریہ_List *p2){
  أكد( p1 );
  اگر( p2 && p1->n_اظہاریہ==p2->n_اظہاریہ ){
    صحیح ii;
    برائے(ii=0; ii<p1->n_اظہاریہ; ii++){
      u8 ترتیب_دو_Flags;
      ترتیب_دو_Flags = p2->a[ii].fg.ترتیب_دو_Flags & KEYINFO_ترتیب_DESC;
      p1->a[ii].fg.ترتیب_دو_Flags = ترتیب_دو_Flags;
    }
    واپس 1;
  }ورنہ{
    واپس 0;
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
صحیح sqlite3Select(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  SelectDest *pDest      /* What to do with the query results */
){
  صحیح i, j;              /* Loop counters */
  WhereInfo *pWInfo;     /* Return from sqlite3WhereBegin() */
  Vdbe *v;               /* The virtual machine under construction */
  صحیح isAgg;             /* True for select lists like "count(*)" */
  اظہاریہ_List *pEList = 0;  /* List of columns to extract. */
  SrcList *pTabList;     /* List of tables to select from */
  اظہاریہ *pWhere;          /* The WHERE clause.  May be NULL */
  اظہاریہ_List *p_گروہ_By;    /* The GROUP BY clause.  May be NULL */
  اظہاریہ *pHaving;         /* The HAVING clause.  May be NULL */
  AggInfo *pAggInfo = 0; /* Aggregate information */
  صحیح rc = 1;            /* Value to return from this function */
  DistinctCtx sDistinct; /* Info on how to code the DISTINCT keyword */
  ترتیب_دو_Ctx s_ترتیب_دو;         /* Info on how to code the ORDER BY clause */
  صحیح i_انتہا;              /* Address of the end of the query */
  sqlite3 *db;           /* The database connection */
  اظہاریہ_List *p_Min_Max_ترتیب_By = 0;  /* Added ORDER BY for min/max queries */
  u8 minMaxFlag;                 /* Flag for min/max queries */

  db = pParse->db;
  أكد( pParse==db->pParse );
  v = sqlite3GetVdbe(pParse);
  اگر( p==0 || pParse->nErr ){
    واپس 1;
  }
  أكد( db->mallocFailed==0 );
  اگر( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) واپس 1;
#اگر TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p, ("begin processing:\n", pParse->addrExplain));
  اگر( sqlite3_درخت_Trace & 0x10000 ){
    اگر( (sqlite3_درخت_Trace & 0x10001)==0x10000 ){
      sqlite3_درخت_View_سطر(0, "In sqlite3Select() at %s:%d",
                           __FILE__, __LINE__);
    }
    sqlite3ShowSelect(p);
  }
#اگر_ختم

  /* tag-select-0100 */
  أكد( p->p_ترتیب_By==0 || pDest->eDest!=SRT_DistFifo );
  أكد( p->p_ترتیب_By==0 || pDest->eDest!=SRT_Fifo );
  أكد( p->p_ترتیب_By==0 || pDest->eDest!=SRT_Dist_قطار );
  أكد( p->p_ترتیب_By==0 || pDest->eDest!=SRT_قطار );
  اگر( IgnorableDistinct(pDest) ){
    أكد(pDest->eDest==SRT_Exists     || pDest->eDest==SRT_Discard ||
           pDest->eDest==SRT_Dist_قطار  || pDest->eDest==SRT_DistFifo );
    /* All of these destinations are also able to ignore the ORDER BY clause */
    اگر( p->p_ترتیب_By ){
#اگر TREETRACE_ENABLED
      TREETRACE(0x800,pParse,p, ("dropping superfluous ORDER BY:\n"));
      اگر( sqlite3_درخت_Trace & 0x800 ){
        sqlite3_درخت_View_اظہاریہ_List(0, p->p_ترتیب_By, 0, "ORDERBY");
      }
#اگر_ختم   
      sqlite3_Parser_جوڑو_Cleanup(pParse, sqlite3_اظہاریہ_List_Delete_Generic,
                              p->p_ترتیب_By);
      testcase( pParse->earlyCleanup );
      p->p_ترتیب_By = 0;
    }
    p->selFlags &= ~(u32)SF_Distinct;
  }
  sqlite3SelectPrep(pParse, p, 0);
  اگر( pParse->nErr ){
    جاؤ select_انتہا;
  }
  أكد( db->mallocFailed==0 );
  أكد( p->pEList!=0 );
#اگر TREETRACE_ENABLED
  اگر( sqlite3_درخت_Trace & 0x10 ){
    TREETRACE(0x10,pParse,p, ("after name resolution:\n"));
    sqlite3_درخت_View_Select(0, p, 0);
  }
#اگر_ختم

  /* If the SF_UFSrcCheck flag is set, then this function is being called
  ** as part of populating the temp table for an UPDATE...FROM statement.
  ** In this case, it is an error if the target object (pSrc->a[0]) name
  ** or alias is duplicated within FROM clause (pSrc->a[1..n]). 
  **
  ** Postgres disallows this case too. The reason is that some other
  ** systems handle this case differently, and not all the same way,
  ** which is just confusing. To avoid this, we follow PG's lead and
  ** disallow it altogether.  */
  اگر( p->selFlags & SF_UFSrcCheck ){
    Src_شے *p0 = &p->pSrc->a[0];
    اگر( sameSrcAlias(p0, p->pSrc) ){
      sqlite3_Error_پیغام(pParse,
          "target object/alias may not appear in FROM clause: %s",
          p0->zAlias ? p0->zAlias : p0->pSTab->zName
      );
      جاؤ select_انتہا;
    }

    /* Clear the SF_UFSrcCheck flag. The check has already been performed,
    ** and leaving this flag set can cause errors if a compound sub-query
    ** in p->pSrc is flattened into this query and this function called
    ** again as part of compound SELECT processing.  */
    p->selFlags &= ~(u32)SF_UFSrcCheck;
  }

  اگر( pDest->eDest==SRT_Output ){
    sqlite3_Generate_عمود_نام(pParse, p);
  }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
  اگر( sqlite3_کھڑکی_Rewrite(pParse, p) ){
    أكد( pParse->nErr );
    جاؤ select_انتہا;
  }
#اگر TREETRACE_ENABLED
  اگر( p->pWin && (sqlite3_درخت_Trace & 0x40)!=0 ){
    TREETRACE(0x40,pParse,p, ("after window rewrite:\n"));
    sqlite3_درخت_View_Select(0, p, 0);
  }
#اگر_ختم
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */
  pTabList = p->pSrc;
  isAgg = (p->selFlags & SF_Aggregate)!=0;
  memset(&s_ترتیب_دو, 0, حجم(s_ترتیب_دو));
  s_ترتیب_دو.p_ترتیب_By = p->p_ترتیب_By;

  /* Try to do various optimizations (flattening subqueries, and strength
  ** reduction of join operators) in the FROM clause up into the main query
  ** tag-select-0200
  */
#اگر !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
  برائے(i=0; !p->p_پیشگی && i<pTabList->nSrc; i++){
    Src_شے *p_شے = &pTabList->a[i];
    Select *pSub = p_شے->fg.isSubquery ? p_شے->u4.pSubq->pSelect : 0;
    جدول *pTab = p_شے->pSTab;

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
    اگر( (p_شے->fg.jointype & (JT_بایاں|JT_LTORJ))!=0
     && sqlite3_اظہاریہ_مستلزم_Non_Null_صف(p->pWhere, p_شے->iCursor,
                                     p_شے->fg.jointype & JT_LTORJ)
     && OptimizationEnabled(db, SQLITE_SimplifyJoin)
    ){
      اگر( p_شے->fg.jointype & JT_بایاں ){
        اگر( p_شے->fg.jointype & JT_دایاں ){
          TREETRACE(0x1000,pParse,p,
                    ("FULL-JOIN simplifies to RIGHT-JOIN on term %d\n",i));
          p_شے->fg.jointype &= ~JT_بایاں;
        }ورنہ{
          TREETRACE(0x1000,pParse,p,
                    ("LEFT-JOIN simplifies to JOIN on term %d\n",i));
          p_شے->fg.jointype &= ~(JT_بایاں|JT_OUTER);
          unset_Join_اظہاریہ(p->pWhere, p_شے->iCursor, 0);
        }
      }
      اگر( p_شے->fg.jointype & JT_LTORJ ){
        برائے(j=i+1; j<pTabList->nSrc; j++){
          Src_شے *pI2 = &pTabList->a[j];
          اگر( pI2->fg.jointype & JT_دایاں ){
            اگر( pI2->fg.jointype & JT_بایاں ){
              TREETRACE(0x1000,pParse,p,
                        ("FULL-JOIN simplifies to LEFT-JOIN on term %d\n",j));
              pI2->fg.jointype &= ~JT_دایاں;
            }ورنہ{
              TREETRACE(0x1000,pParse,p,
                        ("RIGHT-JOIN simplifies to JOIN on term %d\n",j));
              pI2->fg.jointype &= ~(JT_دایاں|JT_OUTER);
              unset_Join_اظہاریہ(p->pWhere, pI2->iCursor, 1);
            }
          }
        }
        برائے(j=pTabList->nSrc-1; j>=0; j--){
          pTabList->a[j].fg.jointype &= ~JT_LTORJ;
          اگر( pTabList->a[j].fg.jointype & JT_دایاں ) توڑو;
        }
      }
    }

    /* No further action if this term of the FROM clause is not a subquery */
    اگر( pSub==0 ) جاری;

    /* Catch mismatch in the declared columns of a view and the number of
    ** columns in the SELECT on the RHS */
    اگر( pTab->n_ستون!=pSub->pEList->n_اظہاریہ ){
      sqlite3_Error_پیغام(pParse, "expected %d columns for '%s' but got %d",
                      pTab->n_ستون, pTab->zName, pSub->pEList->n_اظہاریہ);
      جاؤ select_انتہا;
    }

    /* Do not attempt the usual optimizations (flattening and ORDER BY
    ** elimination) on a MATERIALIZED common table expression because
    ** a MATERIALIZED common table expression is an optimization fence.
    */
    اگر( p_شے->fg.isCte && p_شے->u2.pCteUse->eM10d==M10d_Yes ){
      جاری;
    }

    /* Do not try to flatten an aggregate subquery.
    **
    ** Flattening an aggregate subquery is only possible if the outer query
    ** is not a join.  But if the outer query is not a join, then the subquery
    ** will be implemented as a co-routine and there is no advantage to
    ** flattening in that case.
    */
    اگر( (pSub->selFlags & SF_Aggregate)!=0 ) جاری;
    أكد( pSub->p_گروہ_By==0 );

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
    اگر( pSub->p_ترتیب_By!=0
     && (p->p_ترتیب_By!=0 || pTabList->nSrc>1)      /* Condition (5) */
     && pSub->p_حد_بندی==0                           /* Condition (1) */
     && (pSub->selFlags & (SF_ترتیب_By_Reqd|SF_Recursive))==0  /* (2) and (6) */
     && (p->selFlags & SF_ترتیب_By_Reqd)==0         /* Condition (3) and (4) */
     && OptimizationEnabled(db, SQLITE_Omit_ترتیب_By)
    ){
      TREETRACE(0x800,pParse,p,
                ("omit superfluous ORDER BY on %r FROM-clause subquery\n",i+1));
      sqlite3_Parser_جوڑو_Cleanup(pParse, sqlite3_اظہاریہ_List_Delete_Generic,
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
    اگر( pSub->p_ترتیب_By!=0
     && i==0
     && (p->selFlags & SF_Complex_نتیجہ)!=0
     && (pTabList->nSrc==1
         || (pTabList->a[1].fg.jointype&(JT_OUTER|JT_CROSS))!=0)
    ){
      جاری;
    }

    /* tag-select-0240 */
    اگر( flattenSubquery(pParse, p, i, isAgg) ){
      اگر( pParse->nErr ) جاؤ select_انتہا;
      /* This subquery can be absorbed into its parent. */
      i = -1;
    }
    pTabList = p->pSrc;
    اگر( db->mallocFailed ) جاؤ select_انتہا;
    اگر( !IgnorableOrderby(pDest) ){
      s_ترتیب_دو.p_ترتیب_By = p->p_ترتیب_By;
    }
  }
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_COMPOUND_SELECT
  /* Handle compound SELECT statements using the separate multiSelect()
  ** procedure.  tag-select-0300
  */
  اگر( p->p_پیشگی ){
    rc = multiSelect(pParse, p, pDest);
#اگر TREETRACE_ENABLED
    TREETRACE(0x400,pParse,p,("end compound-select processing\n"));
    اگر( (sqlite3_درخت_Trace & 0x400)!=0 && Explain_Query_Plan_والد(pParse)==0 ){
      sqlite3_درخت_View_Select(0, p, 0);
    }
#اگر_ختم
    اگر( p->p_اگلا==0 ) Explain_Query_Plan_نکالو(pParse);
    واپس rc;
  }
#اگر_ختم

  /* If there may be an "EXISTS (SELECT ...)" in the WHERE clause, attempt
  ** to change it into a join.  */
  اگر( pParse->bHasExists && OptimizationEnabled(db,SQLITE_Exists_مقصد_Join) ){
    exists_مقصد_Join(pParse, p, p->pWhere);
    pTabList = p->pSrc;
  }

  /* Do the WHERE-clause constant propagation optimization if this is
  ** a join.  No need to spend time on this operation for non-join queries
  ** as the equivalent optimization will be handled by query planner in
  ** sqlite3WhereBegin().  tag-select-0330
  */
  اگر( p->pWhere!=0
   && p->pWhere->op==TK_AND
   && OptimizationEnabled(db, SQLITE_PropagateConst)
   && propagateConstants(pParse, p)
  ){
#اگر TREETRACE_ENABLED
    اگر( sqlite3_درخت_Trace & 0x2000 ){
      TREETRACE(0x2000,pParse,p,("After constant propagation:\n"));
      sqlite3_درخت_View_Select(0, p, 0);
    }
#اگر_ختم
  }ورنہ{
    TREETRACE(0x2000,pParse,p,("Constant propagation not helpful\n"));
  }

  /* tag-select-0350 */
  اگر( OptimizationEnabled(db, SQLITE_QueryFlattener|SQLITE_گنتی_کا_View)
   && گنتی_کا_View_Optimization(pParse, p)
  ){
    اگر( db->mallocFailed ) جاؤ select_انتہا;
    pTabList = p->pSrc;
  }

  /* Loop over all terms in the FROM clause and do two things for each term:
  **
  **   (1) Authorize unreferenced tables
  **   (2) Generate code for all sub-queries
  **
  ** tag-select-0400
  */
  برائے(i=0; i<pTabList->nSrc; i++){
    Src_شے *p_شے = &pTabList->a[i];
    Src_شے *p_پیشگی;
    SelectDest dest;
    Subquery *pSubq;
    Select *pSub;
#اگر !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    ثابت حرف *zSavedAuthContext;
#اگر_ختم

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
    اگر( p_شے->ستون_مستعمل==0 && p_شے->zName!=0 ){
      ثابت حرف *zDb;
      اگر( p_شے->fg.fixedSchema ){
        صحیح iDb = sqlite3_Schema_مقصد_اشاریہ(pParse->db, p_شے->u4.pSchema);
        zDb = db->aDb[iDb].zDbSName;
      }ورنہ اگر( p_شے->fg.isSubquery ){
        zDb = 0;
      }ورنہ{
        zDb = p_شے->u4.zDatabase;
      }
      sqlite3AuthCheck(pParse, SQLITE_READ, p_شے->zName, "", zDb);
    }

#اگر !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    /* Generate code for all sub-queries in the FROM clause
    */
    اگر( p_شے->fg.isSubquery==0 ) جاری;
    pSubq = p_شے->u4.pSubq;
    أكد( pSubq!=0 );
    pSub = pSubq->pSelect;

    /* The code for a subquery should only be generated once. */
    اگر( pSubq->addrFillSub!=0 ) جاری;

    /* Increment Parse.nHeight by the height of the largest expression
    ** tree referred to by this, the parent select. The child select
    ** may contain expression trees of at most
    ** (SQLITE_MAX_EXPR_DEPTH-Parse.nHeight) height. This is a bit
    ** more conservative than necessary, but much easier than enforcing
    ** an exact limit.
    */
    pParse->n_اونچائی += sqlite3_Select_اظہاریہ_اونچائی(p);

    /* Make copies of constant WHERE-clause terms in the outer query down
    ** inside the subquery.  This can help the subquery to run more efficiently.
    ** This is the "predicate push-down optimization".  tag-select-0420
    */
    اگر( OptimizationEnabled(db, SQLITE_ڈالو_نیچے_کو)
     && (p_شے->fg.isCte==0
         || (p_شے->u2.pCteUse->eM10d!=M10d_Yes && p_شے->u2.pCteUse->nUse<2))
     && ڈالو_نیچے_کو_Where_Terms(pParse, pSub, p->pWhere, pTabList, i)
    ){
#اگر TREETRACE_ENABLED
      اگر( sqlite3_درخت_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("After WHERE-clause push-down into subquery %d:\n", pSub->selId));
        sqlite3_درخت_View_Select(0, p, 0);
      }
#اگر_ختم
      أكد( pSubq->pSelect && (pSub->selFlags & SF_ڈالو_نیچے_کو)!=0 );
    }ورنہ{
      TREETRACE(0x4000,pParse,p,("WHERE-clause push-down not possible\n"));
    }

    /* Convert unused result columns of the subquery into simple NULL
    ** expressions, to avoid unneeded searching and computation.
    ** tag-select-0440
    */
    اگر( OptimizationEnabled(db, SQLITE_Null_غیر_مستعمل_ستون_تعداد)
     && disable_غیر_مستعمل_Subquery_نتیجہ_Columns(p_شے)
    ){
#اگر TREETRACE_ENABLED
      اگر( sqlite3_درخت_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("Change unused result columns to NULL for subquery %d:\n",
             pSub->selId));
        sqlite3_درخت_View_Select(0, p, 0);
      }
#اگر_ختم
    }

    zSavedAuthContext = pParse->zAuthContext;
    pParse->zAuthContext = p_شے->zName;

    /* Generate byte-code to implement the subquery  tag-select-0480
    */
    اگر( from_Clause_رکن_Can_Be_Coroutine(pParse, pTabList, i, p->selFlags) ){
      /* Implement a co-routine that will return a single row of the result
      ** set on each invocation.  tag-select-0482
      */
      صحیح addr_چوٹی = sqlite3VdbeCurrentAddr(v)+1;
    
      pSubq->regReturn = ++pParse->nMem;
      sqlite3_Vdbe_جوڑو_Op3(v, OP_InitCoroutine, pSubq->regReturn, 0, addr_چوٹی);
      VdbeComment((v, "%!S", p_شے));
      pSubq->addrFillSub = addr_چوٹی;
      sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);
      ExplainQueryPlan((pParse, 1, "CO-ROUTINE %!S", p_شے));
      sqlite3Select(pParse, pSub, &dest);
      p_شے->pSTab->n_صف_Log_Est = pSub->n_Select_صف;
      p_شے->fg.بذریعہ_Coroutine = 1;
      pSubq->reg_نتیجہ = dest.iSdst;
      sqlite3_Vdbe_انتہا_Coroutine(v, pSubq->regReturn);
      VdbeComment((v, "end %!S", p_شے));
      sqlite3_Vdbe_Jump_یہاں(v, addr_چوٹی-1);
      sqlite3_Clear_عارضی_Reg_Cache(pParse);
    }ورنہ اگر( p_شے->fg.isCte && p_شے->u2.pCteUse->addrM9e>0 ){
      /* This is a CTE for which materialization code has already been
      ** generated.  Invoke the subroutine to compute the materialization,
      ** then make the pItem->iCursor be a copy of the ephemeral table that
      ** holds the result of the materialization. tag-select-0484 */
      CteUse *pCteUse = p_شے->u2.pCteUse;
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, pCteUse->regRtn, pCteUse->addrM9e);
      اگر( p_شے->iCursor!=pCteUse->iCur ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenDup, p_شے->iCursor, pCteUse->iCur);
        VdbeComment((v, "%!S", p_شے));
      }
      pSub->n_Select_صف = pCteUse->n_صف_Est;
    }ورنہ اگر( (p_پیشگی = isSelfJoinView(pTabList, p_شے, 0, i))!=0 ){
      /* This view has already been materialized by a prior entry in
      ** this same FROM clause.  Reuse it.  tag-select-0486 */
      Subquery *p_پیشگی_Subq;
      أكد( p_پیشگی->fg.isSubquery );
      p_پیشگی_Subq = p_پیشگی->u4.pSubq;
      أكد( p_پیشگی_Subq!=0 );
      اگر( p_پیشگی_Subq->addrFillSub ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, p_پیشگی_Subq->regReturn,
                                       p_پیشگی_Subq->addrFillSub);
      }
      sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenDup, p_شے->iCursor, p_پیشگی->iCursor);
      pSub->n_Select_صف = p_پیشگی_Subq->pSelect->n_Select_صف;
    }ورنہ{
      /* Materialize the view.  If the view is not correlated, generate a
      ** subroutine to do the materialization so that subsequent uses of
      ** the same view can reuse the materialization.  tag-select-0488 */
      صحیح چوٹی_Addr;
      صحیح onceAddr = 0;
#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
      صحیح addrExplain;
#اگر_ختم

      pSubq->regReturn = ++pParse->nMem;
      چوٹی_Addr = sqlite3_Vdbe_جوڑو_Op0(v, OP_Goto);
      pSubq->addrFillSub = چوٹی_Addr+1;
      p_شے->fg.isMaterialized = 1;
      اگر( p_شے->fg.isCorrelated==0 ){
        /* If the subquery is not correlated and if we are not inside of
        ** a trigger, then we only need to compute the value of the subquery
        ** once. */
        onceAddr = sqlite3_Vdbe_جوڑو_Op0(v, OP_Once); VdbeCoverage(v);
        VdbeComment((v, "materialize %!S", p_شے));
      }ورنہ{
        VdbeNoopComment((v, "materialize %!S", p_شے));
      }
      sqlite3SelectDestInit(&dest, SRT_EphemTab, p_شے->iCursor);

      ExplainQueryPlan2(addrExplain, (pParse, 1, "MATERIALIZE %!S", p_شے));
      sqlite3Select(pParse, pSub, &dest);
      p_شے->pSTab->n_صف_Log_Est = pSub->n_Select_صف;
      اگر( onceAddr ) sqlite3_Vdbe_Jump_یہاں(v, onceAddr);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Return, pSubq->regReturn, چوٹی_Addr+1);
      VdbeComment((v, "end %!S", p_شے));
      sqlite3_Vdbe_جائزہ_حالت_Range(v, addrExplain, addrExplain, -1);
      sqlite3_Vdbe_Jump_یہاں(v, چوٹی_Addr);
      sqlite3_Clear_عارضی_Reg_Cache(pParse);
      اگر( p_شے->fg.isCte && p_شے->fg.isCorrelated==0 ){
        CteUse *pCteUse = p_شے->u2.pCteUse;
        pCteUse->addrM9e = pSubq->addrFillSub;
        pCteUse->regRtn = pSubq->regReturn;
        pCteUse->iCur = p_شے->iCursor;
        pCteUse->n_صف_Est = pSub->n_Select_صف;
      }
    }
    اگر( db->mallocFailed ) جاؤ select_انتہا;
    pParse->n_اونچائی -= sqlite3_Select_اظہاریہ_اونچائی(p);
    pParse->zAuthContext = zSavedAuthContext;
#اگر_ختم
  }

  /* Various elements of the SELECT copied into local variables for
  ** convenience */
  pEList = p->pEList;
  pWhere = p->pWhere;
  p_گروہ_By = p->p_گروہ_By;
  pHaving = p->pHaving;
  sDistinct.isTnct = (p->selFlags & SF_Distinct)!=0;

#اگر TREETRACE_ENABLED
  اگر( sqlite3_درخت_Trace & 0x8000 ){
    TREETRACE(0x8000,pParse,p,("After all FROM-clause analysis:\n"));
    sqlite3_درخت_View_Select(0, p, 0);
  }
#اگر_ختم

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
  اگر( (p->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct
   && sqlite3_Copy_ترتیب_دو_ترتیب(pEList, s_ترتیب_دو.p_ترتیب_By)
   && sqlite3_اظہاریہ_List_Compare(pEList, s_ترتیب_دو.p_ترتیب_By, -1)==0
   && OptimizationEnabled(db, SQLITE_گروہ_By_ترتیب)
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
   && p->pWin==0
#اگر_ختم
  ){
    p->selFlags &= ~(u32)SF_Distinct;
    p_گروہ_By = p->p_گروہ_By = sqlite3_اظہاریہ_List_Dup(db, pEList, 0);
    اگر( p_گروہ_By ){
      برائے(i=0; i<p_گروہ_By->n_اظہاریہ; i++){
        p_گروہ_By->a[i].u.x.i_ترتیب_By_ستون = i+1;
      }
    }
    p->selFlags |= SF_Aggregate;
    /* Notice that even thought SF_Distinct has been cleared from p->selFlags,
    ** the sDistinct.isTnct is still set.  Hence, isTnct represents the
    ** original setting of the SF_Distinct flag, not the current setting */
    أكد( sDistinct.isTnct );
    sDistinct.isTnct = 2;

#اگر TREETRACE_ENABLED
    اگر( sqlite3_درخت_Trace & 0x20000 ){
      TREETRACE(0x20000,pParse,p,("Transform DISTINCT into GROUP BY:\n"));
      sqlite3_درخت_View_Select(0, p, 0);
    }
#اگر_ختم
  }

  /* If there is an ORDER BY clause, then create an ephemeral index to
  ** do the sorting.  But this sorting ephemeral index might end up
  ** being unused if the data can be extracted in pre-sorted order.
  ** If that is the case, then the OP_OpenEphemeral instruction will be
  ** changed to an OP_Noop once we figure out that the sorting index is
  ** not needed.  The sSort.addrSortIndex variable is used to facilitate
  ** that change.  tag-select-0600
  */
  اگر( s_ترتیب_دو.p_ترتیب_By ){
    کلید_Info *p_کلید_Info;
    p_کلید_Info = sqlite3_کلید_Info_From_اظہاریہ_List(
        pParse, s_ترتیب_دو.p_ترتیب_By, 0, pEList->n_اظہاریہ);
    s_ترتیب_دو.iECursor = pParse->nTab++;
    s_ترتیب_دو.addr_ترتیب_دو_اشاریہ =
      sqlite3_Vdbe_جوڑو_Op4(v, OP_OpenEphemeral,
          s_ترتیب_دو.iECursor, s_ترتیب_دو.p_ترتیب_By->n_اظہاریہ+1+pEList->n_اظہاریہ, 0,
          (حرف*)p_کلید_Info, P4_KEYINFO
      );
  }ورنہ{
    s_ترتیب_دو.addr_ترتیب_دو_اشاریہ = -1;
  }

  /* If the output is destined for a temporary table, open that table.
  ** tag-select-0630
  */
  اگر( pDest->eDest==SRT_EphemTab ){
    sqlite3_Vdbe_جوڑو_Op2(v, OP_OpenEphemeral, pDest->iSDParm, pEList->n_اظہاریہ);
    اگر( p->selFlags & SF_NestedFrom ){
      /* Delete or NULL-out result columns that will never be used */
      صحیح ii;
      برائے(ii=pEList->n_اظہاریہ-1; ii>0 && pEList->a[ii].fg.b_مستعمل==0; ii--){
        sqlite3_اظہاریہ_Delete(db, pEList->a[ii].p_اظہاریہ);
        sqlite3DbFree(db, pEList->a[ii].zEName);
        pEList->n_اظہاریہ--;
      }
      برائے(ii=0; ii<pEList->n_اظہاریہ; ii++){
        اگر( pEList->a[ii].fg.b_مستعمل==0 ) pEList->a[ii].p_اظہاریہ->op = TK_NULL;
      }
    }
  }

  /* Set the limiter.  tag-select-0650
  */
  i_انتہا = sqlite3VdbeMakeLabel(pParse);
  اگر( (p->selFlags & SF_Fixed_حد_بندی)==0 ){
    p->n_Select_صف = 320;  /* 4 billion rows */
  }
  اگر( p->p_حد_بندی ) compute_حد_بندی_Registers(pParse, p, i_انتہا);
  اگر( p->i_حد_بندی==0 && s_ترتیب_دو.addr_ترتیب_دو_اشاریہ>=0 ){
    sqlite3VdbeChangeOpcode(v, s_ترتیب_دو.addr_ترتیب_دو_اشاریہ, OP_SorterOpen);
    s_ترتیب_دو.ترتیب_دو_Flags |= SORTFLAG_UseSorter;
  }

  /* Open an ephemeral index to use for the distinct set. tag-select-0680
  */
  اگر( p->selFlags & SF_Distinct ){
    sDistinct.tabTnct = pParse->nTab++;
    sDistinct.addrTnct = sqlite3_Vdbe_جوڑو_Op4(v, OP_OpenEphemeral,
                       sDistinct.tabTnct, 0, 0,
                       (حرف*)sqlite3_کلید_Info_From_اظہاریہ_List(pParse, p->pEList,0,0),
                       P4_KEYINFO);
    sqlite3VdbeChangeP5(v, BTREE_UNORDERED);
    sDistinct.eTnctType = WHERE_DISTINCT_UNORDERED;
  }ورنہ{
    sDistinct.eTnctType = WHERE_DISTINCT_NOOP;
  }

  اگر( !isAgg && p_گروہ_By==0 ){
    /* No aggregate functions and no GROUP BY clause.  tag-select-0700 */
    u16 wctrlFlags = (sDistinct.isTnct ? WHERE_WANT_DISTINCT : 0)
                   | (p->selFlags & SF_Fixed_حد_بندی);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    کھڑکی *pWin = p->pWin;      /* Main window object (or NULL) */
    اگر( pWin ){
      sqlite3_کھڑکی_رمز_Init(pParse, p);
    }
#اگر_ختم
    أكد( WHERE_USE_حد_بندی==SF_Fixed_حد_بندی );


    /* Begin the database scan. */
    TREETRACE(0x2,pParse,p,("WhereBegin\n"));
    pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, s_ترتیب_دو.p_ترتیب_By,
                               p->pEList, p, wctrlFlags, p->n_Select_صف);
    اگر( pWInfo==0 ) جاؤ select_انتہا;
    اگر( sqlite3_Where_Output_صف_گنتی(pWInfo) < p->n_Select_صف ){
      p->n_Select_صف = sqlite3_Where_Output_صف_گنتی(pWInfo);
      اگر( pDest->eDest<=SRT_Dist_قطار && pDest->eDest>=SRT_DistFifo ){
        /* TUNING: For a UNION CTE, because UNION is implies DISTINCT,
        ** reduce the estimated output row count by 8 (LogEst 30). 
        ** Search for tag-20250414a to see other cases */
        p->n_Select_صف -= 30;
      }
    }
    اگر( sDistinct.isTnct && sqlite3WhereIsDistinct(pWInfo) ){
      sDistinct.eTnctType = sqlite3WhereIsDistinct(pWInfo);
    }
    اگر( s_ترتیب_دو.p_ترتیب_By ){
      s_ترتیب_دو.nOBSat = sqlite3WhereIsOrdered(pWInfo);
      s_ترتیب_دو.labelOBLopt = sqlite3_Where_ترتیب_By_حد_بندی_Opt_Label(pWInfo);
      اگر( s_ترتیب_دو.nOBSat==s_ترتیب_دو.p_ترتیب_By->n_اظہاریہ ){
        s_ترتیب_دو.p_ترتیب_By = 0;
      }
    }
    TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));

    /* If sorting index that was created by a prior OP_OpenEphemeral
    ** instruction ended up not being needed, then change the OP_OpenEphemeral
    ** into an OP_Noop.
    */
    اگر( s_ترتیب_دو.addr_ترتیب_دو_اشاریہ>=0 && s_ترتیب_دو.p_ترتیب_By==0 ){
      sqlite3_Vdbe_Change_مقصد_Noop(v, s_ترتیب_دو.addr_ترتیب_دو_اشاریہ);
    }

    أكد( p->pEList==pEList );
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    اگر( pWin ){
      صحیح addrGosub = sqlite3VdbeMakeLabel(pParse);
      صحیح iCont = sqlite3VdbeMakeLabel(pParse);
      صحیح iBreak = sqlite3VdbeMakeLabel(pParse);
      صحیح regGosub = ++pParse->nMem;

      sqlite3_کھڑکی_رمز_قدم(pParse, p, pWInfo, regGosub, addrGosub);

      sqlite3_Vdbe_جوڑو_Op2(v, OP_Goto, 0, iBreak);
      sqlite3VdbeResolveLabel(v, addrGosub);
      VdbeNoopComment((v, "inner-loop subroutine"));
      s_ترتیب_دو.labelOBLopt = 0;
      selectInnerLoop(pParse, p, -1, &s_ترتیب_دو, &sDistinct, pDest, iCont, iBreak);
      sqlite3VdbeResolveLabel(v, iCont);
      sqlite3_Vdbe_جوڑو_Op1(v, OP_Return, regGosub);
      VdbeComment((v, "end inner-loop subroutine"));
      sqlite3VdbeResolveLabel(v, iBreak);
    }ورنہ
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */
    {
      /* Use the standard inner loop. */
      selectInnerLoop(pParse, p, -1, &s_ترتیب_دو, &sDistinct, pDest,
          sqlite3WhereContinueLabel(pWInfo),
          sqlite3WhereBreakLabel(pWInfo));

      /* End the database scan loop.
      */
      TREETRACE(0x2,pParse,p,("WhereEnd\n"));
      sqlite3_Where_انتہا(pWInfo);
    }
  }ورنہ{
    /* This case is for when there exist aggregate functions or a GROUP BY
    ** clause or both.  tag-select-0800 */
    NameContext sNC;    /* Name context for processing aggregate information */
    صحیح iAMem;          /* First Mem address for storing current GROUP BY */
    صحیح iBMem;          /* First Mem address for previous GROUP BY */
    صحیح iUseFlag;       /* Mem address holding flag indicating that at least
                        ** one row of the input to the aggregator has been
                        ** processed */
    صحیح iAbortFlag;     /* Mem address which causes query abort if positive */
    صحیح گروہ_By_ترتیب_دو;    /* Rows come from source in GROUP BY order */
    صحیح addr_انتہا;        /* End of processing for this SELECT */
    صحیح ترتیب_دو_PTab = 0;   /* Pseudotable used to decode sorting results */
    صحیح ترتیب_دو_برآمد = 0;    /* Output register from the sorter */
    صحیح ترتیب_By_Grp = 0; /* True if the GROUP BY and ORDER BY are the same */

    /* Remove any and all aliases between the result set and the
    ** GROUP BY clause.
    */
    اگر( p_گروہ_By ){
      صحیح k;                        /* Loop counter */
      ساخت اظہاریہ_List_شے *p_شے;  /* For looping over expression in a list */

      برائے(k=p->pEList->n_اظہاریہ, p_شے=p->pEList->a; k>0; k--, p_شے++){
        p_شے->u.x.iAlias = 0;
      }
      برائے(k=p_گروہ_By->n_اظہاریہ, p_شے=p_گروہ_By->a; k>0; k--, p_شے++){
        p_شے->u.x.iAlias = 0;
      }
      أكد( 66==sqlite3LogEst(100) );
      اگر( p->n_Select_صف>66 ) p->n_Select_صف = 66;

      /* If there is both a GROUP BY and an ORDER BY clause and they are
      ** identical, then it may be possible to disable the ORDER BY clause
      ** on the grounds that the GROUP BY will cause elements to come out
      ** in the correct order. It also may not - the GROUP BY might use a
      ** database index that causes rows to be grouped together as required
      ** but not actually sorted. Either way, record the fact that the
      ** ORDER BY and GROUP BY clauses are the same by setting the orderByGrp
      ** variable.  */
      اگر( sqlite3_Copy_ترتیب_دو_ترتیب(p_گروہ_By, s_ترتیب_دو.p_ترتیب_By)
       && sqlite3_اظہاریہ_List_Compare(p_گروہ_By, s_ترتیب_دو.p_ترتیب_By, -1)==0
      ){
        ترتیب_By_Grp = 1;
      }
    }ورنہ{
      أكد( 0==sqlite3LogEst(1) );
      p->n_Select_صف = 0;
    }

    /* Create a label to jump to when we want to abort the query */
    addr_انتہا = sqlite3VdbeMakeLabel(pParse);

    /* Convert TK_COLUMN nodes into TK_AGG_COLUMN and make entries in
    ** sAggInfo for all TK_AGG_FUNCTION nodes in expressions of the
    ** SELECT statement.
    */
    pAggInfo = sqlite3DbMallocZero(db, حجم(*pAggInfo) );
    اگر( pAggInfo ){
      sqlite3_Parser_جوڑو_Cleanup(pParse, agginfoFree, pAggInfo);
      testcase( pParse->earlyCleanup );
    }
    اگر( db->mallocFailed ){
      جاؤ select_انتہا;
    }
    pAggInfo->selId = p->selId;
#اگر_تعریف_شدہ SQLITE_DEBUG
    pAggInfo->pSelect = p;
#اگر_ختم
    memset(&sNC, 0, حجم(sNC));
    sNC.pParse = pParse;
    sNC.pSrcList = pTabList;
    sNC.uNC.pAggInfo = pAggInfo;
    VVA_ONLY( sNC.ncFlags = NC_UAggInfo; )
    pAggInfo->n_ترتیب_کاری_عمود = p_گروہ_By ? p_گروہ_By->n_اظہاریہ : 0;
    pAggInfo->p_گروہ_By = p_گروہ_By;
    sqlite3_اظہاریہ_Analyze_Agg_List(&sNC, pEList);
    sqlite3_اظہاریہ_Analyze_Agg_List(&sNC, s_ترتیب_دو.p_ترتیب_By);
    اگر( pHaving ){
      اگر( p_گروہ_By ){
        أكد( pWhere==p->pWhere );
        أكد( pHaving==p->pHaving );
        أكد( p_گروہ_By==p->p_گروہ_By );
        having_مقصد_Where(pParse, p);
        pWhere = p->pWhere;
      }
      sqlite3_اظہاریہ_Analyze_Aggregates(&sNC, pHaving);
    }
    pAggInfo->n_مجمع = pAggInfo->n_عمود;
    اگر( p->p_گروہ_By==0 && p->pHaving==0 && pAggInfo->nFunc==1 ){
      minMaxFlag = minMaxQuery(db, pAggInfo->aFunc[0].pFExpr, &p_Min_Max_ترتیب_By);
    }ورنہ{
      minMaxFlag = WHERE_ORDERBY_معمول;
    }
    analyze_Agg_Func_دلائل(pAggInfo, &sNC);
    اگر( db->mallocFailed ) جاؤ select_انتہا;
#اگر TREETRACE_ENABLED
    اگر( sqlite3_درخت_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("After aggregate analysis %p:\n", pAggInfo));
      sqlite3_درخت_View_Select(0, p, 0);
      اگر( minMaxFlag ){
        sqlite3DebugPrintf("MIN/MAX Optimization (0x%02x) adds:\n", minMaxFlag);
        sqlite3_درخت_View_اظہاریہ_List(0, p_Min_Max_ترتیب_By, 0, "ORDERBY");
      }
      printAggInfo(pAggInfo);
    }
#اگر_ختم


    /* Processing for aggregates with GROUP BY is very different and
    ** much more complex than aggregates without a GROUP BY.  tag-select-0810
    */
    اگر( p_گروہ_By ){
      کلید_Info *p_کلید_Info;  /* Keying information for the group by clause */
      صحیح addr1;          /* A-vs-B comparison jump */
      صحیح addr_Output_صف;  /* Start of subroutine that outputs a result row */
      صحیح reg_Output_صف;   /* Return address register for output subroutine */
      صحیح addr_سیٹ_Abort;   /* Set the abort flag and return */
      صحیح addr_چوٹی_کا_Loop;  /* Top of the input loop */
      صحیح addr_ترتیب_کاری_Idx; /* The OP_OpenEphemeral for the sorting index */
      صحیح addrReset;      /* Subroutine for resetting the accumulator */
      صحیح regReset;       /* Return address register for reset subroutine */
      اظہاریہ_List *pDistinct = 0;
      u16 distFlag = 0;
      صحیح eDist = WHERE_DISTINCT_NOOP;

      اگر( pAggInfo->nFunc==1
       && pAggInfo->aFunc[0].iDistinct>=0
       && ALWAYS(pAggInfo->aFunc[0].pFExpr!=0)
       && ALWAYS(اظہاریہ_Use_XList(pAggInfo->aFunc[0].pFExpr))
       && pAggInfo->aFunc[0].pFExpr->x.pList!=0
      ){
        اظہاریہ *p_اظہاریہ = pAggInfo->aFunc[0].pFExpr->x.pList->a[0].p_اظہاریہ;
        p_اظہاریہ = sqlite3_اظہاریہ_Dup(db, p_اظہاریہ, 0);
        pDistinct = sqlite3_اظہاریہ_List_Dup(db, p_گروہ_By, 0);
        pDistinct = sqlite3_اظہاریہ_List_Append(pParse, pDistinct, p_اظہاریہ);
        distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
      }

      /* If there is a GROUP BY clause we might need a sorting index to
      ** implement it.  Allocate that sorting index now.  If it turns out
      ** that we do not need it after all, the OP_SorterOpen instruction
      ** will be converted into a Noop. 
      */
      pAggInfo->ترتیب_کاری_Idx = pParse->nTab++;
      p_کلید_Info = sqlite3_کلید_Info_From_اظہاریہ_List(pParse, p_گروہ_By,
                                            0, pAggInfo->n_عمود);
      addr_ترتیب_کاری_Idx = sqlite3_Vdbe_جوڑو_Op4(v, OP_SorterOpen,
          pAggInfo->ترتیب_کاری_Idx, pAggInfo->n_ترتیب_کاری_عمود,
          0, (حرف*)p_کلید_Info, P4_KEYINFO);

      /* Initialize memory locations used by GROUP BY aggregate processing
      */
      iUseFlag = ++pParse->nMem;
      iAbortFlag = ++pParse->nMem;
      reg_Output_صف = ++pParse->nMem;
      addr_Output_صف = sqlite3VdbeMakeLabel(pParse);
      regReset = ++pParse->nMem;
      addrReset = sqlite3VdbeMakeLabel(pParse);
      iAMem = pParse->nMem + 1;
      pParse->nMem += p_گروہ_By->n_اظہاریہ;
      iBMem = pParse->nMem + 1;
      pParse->nMem += p_گروہ_By->n_اظہاریہ;
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, iAbortFlag);
      VdbeComment((v, "clear abort flag"));
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Null, 0, iAMem, iAMem+p_گروہ_By->n_اظہاریہ-1);
      sqlite3_اظہاریہ_Null_رجسٹر_Range(pParse, iAMem, p_گروہ_By->n_اظہاریہ);

      /* Begin a loop that will extract all source rows in GROUP BY order.
      ** This might involve two separate loops with an OP_Sort in between, or
      ** it might be a single loop that uses an index to extract information
      ** in the right order to begin with.
      */
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, regReset, addrReset);
      TREETRACE(0x2,pParse,p,("WhereBegin\n"));
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, p_گروہ_By, pDistinct,
          p, (sDistinct.isTnct==2 ? WHERE_DISTINCTBY : WHERE_GROUPBY)
          |  (ترتیب_By_Grp ? WHERE_SORTBYGROUP : 0) | distFlag, 0
      );
      اگر( pWInfo==0 ){
        sqlite3_اظہاریہ_List_Delete(db, pDistinct);
        جاؤ select_انتہا;
      }
      اگر( pParse->pIdxEpr ){
        optimize_Aggregate_Use_کا_اشاریہ_دار_اظہاریہ(pParse, p, pAggInfo, &sNC);
      }
      تفویض_Aggregate_Registers(pParse, pAggInfo);
      eDist = sqlite3WhereIsDistinct(pWInfo);
      TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
      اگر( sqlite3WhereIsOrdered(pWInfo)==p_گروہ_By->n_اظہاریہ ){
        /* The optimizer is able to deliver rows in group by order so
        ** we do not have to sort.  The OP_OpenEphemeral table will be
        ** cancelled later because we still need to use the pKeyInfo
        */
        گروہ_By_ترتیب_دو = 0;
      }ورنہ{
        /* Rows are coming out in undetermined order.  We have to push
        ** each row into a sorting index, terminate the first loop,
        ** then loop over the sorting index in order to get the output
        ** in sorted order
        */
        صحیح reg_بنیاد;
        صحیح reg_ریکارڈ;
        صحیح n_ستون;
        صحیح n_گروہ_By;

#اگر_تعریف_شدہ SQLITE_ENABLE_STMT_SCANSTATUS
        صحیح addrExp;              /* Address of OP_Explain instruction */
#اگر_ختم
        ExplainQueryPlan2(addrExp, (pParse, 0, "USE TEMP B-TREE FOR %s",
            (sDistinct.isTnct && (p->selFlags&SF_Distinct)==0) ?
                    "DISTINCT" : "GROUP BY"
        ));

        گروہ_By_ترتیب_دو = 1;
        n_گروہ_By = p_گروہ_By->n_اظہاریہ;
        n_ستون = n_گروہ_By;
        j = n_گروہ_By;
        برائے(i=0; i<pAggInfo->n_عمود; i++){
          اگر( pAggInfo->a_ستون[i].i_Sorter_عمود>=j ){
            n_ستون++;
            j++;
          }
        }
        reg_بنیاد = sqlite3_Get_عارضی_Range(pParse, n_ستون);
        sqlite3_اظہاریہ_رمز_اظہاریہ_List(pParse, p_گروہ_By, reg_بنیاد, 0, 0);
        j = n_گروہ_By;
        pAggInfo->براہ_راست_منوال = 1;
        برائے(i=0; i<pAggInfo->n_عمود; i++){
          ساخت Agg_Info_ستون *p_ستون = &pAggInfo->a_ستون[i];
          اگر( p_ستون->i_Sorter_عمود>=j ){
            sqlite3_اظہاریہ_رمز(pParse, p_ستون->pCExpr, j + reg_بنیاد);
            j++;
          }
        }
        pAggInfo->براہ_راست_منوال = 0;
        reg_ریکارڈ = sqlite3_Get_عارضی_Reg(pParse);
        sqlite3_Vdbe_جائزہ_حالت_Counters(v, addrExp, 0, sqlite3VdbeCurrentAddr(v));
        sqlite3_Vdbe_جوڑو_Op3(v, OP_Make_ریکارڈ, reg_بنیاد, n_ستون, reg_ریکارڈ);
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Sorter_داخل_کرو, pAggInfo->ترتیب_کاری_Idx, reg_ریکارڈ);
        sqlite3_Vdbe_جائزہ_حالت_Range(v, addrExp, sqlite3VdbeCurrentAddr(v)-2, -1);
        sqlite3_Release_عارضی_Reg(pParse, reg_ریکارڈ);
        sqlite3_Release_عارضی_Range(pParse, reg_بنیاد, n_ستون);
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_انتہا(pWInfo);
        pAggInfo->ترتیب_کاری_Idx_PTab = ترتیب_دو_PTab = pParse->nTab++;
        ترتیب_دو_برآمد = sqlite3_Get_عارضی_Reg(pParse);
        sqlite3_Vdbe_جائزہ_حالت_Counters(v, addrExp, sqlite3VdbeCurrentAddr(v), 0);
        sqlite3_Vdbe_جوڑو_Op3(v, OP_OpenPseudo, ترتیب_دو_PTab, ترتیب_دو_برآمد, n_ستون);
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Sorter_ترتیب_دو, pAggInfo->ترتیب_کاری_Idx, addr_انتہا);
        VdbeComment((v, "GROUP BY sort")); VdbeCoverage(v);
        pAggInfo->use_ترتیب_کاری_Idx = 1;
        sqlite3_Vdbe_جائزہ_حالت_Range(v, addrExp, -1, ترتیب_دو_PTab);
        sqlite3_Vdbe_جائزہ_حالت_Range(v, addrExp, -1, pAggInfo->ترتیب_کاری_Idx);
      }

      /* If there are entries in pAgggInfo->aFunc[] that contain subexpressions
      ** that are indexed (and that were previously identified and tagged
      ** in optimizeAggregateUseOfIndexedExpr()) then those subexpressions
      ** must now be converted into a TK_AGG_COLUMN node so that the value
      ** is correctly pulled from the index rather than being recomputed. */
      اگر( pParse->pIdxEpr ){
        aggregate_Convert_اشاریہ_دار_اظہاریہ_Ref_مقصد_عمود(pAggInfo);
#اگر TREETRACE_ENABLED
        اگر( sqlite3_درخت_Trace & 0x20 ){
          TREETRACE(0x20, pParse, p,
             ("AggInfo function expressions converted to reference index\n"));
          sqlite3_درخت_View_Select(0, p, 0);
          printAggInfo(pAggInfo);
        }
#اگر_ختم
      }

      /* If the index or temporary table used by the GROUP BY sort
      ** will naturally deliver rows in the order required by the ORDER BY
      ** clause, cancel the ephemeral table open coded earlier.
      **
      ** This is an optimization - the correct answer should result regardless.
      ** Use the SQLITE_GroupByOrder flag with SQLITE_TESTCTRL_OPTIMIZER to
      ** disable this optimization for testing purposes.  */
      اگر( ترتیب_By_Grp && OptimizationEnabled(db, SQLITE_گروہ_By_ترتیب)
       && (گروہ_By_ترتیب_دو || sqlite3WhereIsSorted(pWInfo))
      ){
        s_ترتیب_دو.p_ترتیب_By = 0;
        sqlite3_Vdbe_Change_مقصد_Noop(v, s_ترتیب_دو.addr_ترتیب_دو_اشاریہ);
      }

      /* Evaluate the current GROUP BY terms and store in b0, b1, b2...
      ** (b0 is memory location iBMem+0, b1 is iBMem+1, and so forth)
      ** Then compare the current GROUP BY terms against the GROUP BY terms
      ** from the previous row currently stored in a0, a1, a2...
      */
      addr_چوٹی_کا_Loop = sqlite3VdbeCurrentAddr(v);
      اگر( گروہ_By_ترتیب_دو ){
        sqlite3_Vdbe_جوڑو_Op3(v, OP_Sorter_ڈیٹا, pAggInfo->ترتیب_کاری_Idx,
                          ترتیب_دو_برآمد, ترتیب_دو_PTab);
      }
      برائے(j=0; j<p_گروہ_By->n_اظہاریہ; j++){
        صحیح i_ترتیب_By_ستون = p_گروہ_By->a[j].u.x.i_ترتیب_By_ستون;
        
        اگر( گروہ_By_ترتیب_دو ){
          sqlite3_Vdbe_جوڑو_Op3(v, OP_عمود, ترتیب_دو_PTab, j, iBMem+j);
        }ورنہ{
          pAggInfo->براہ_راست_منوال = 1;
          sqlite3_اظہاریہ_رمز(pParse, p_گروہ_By->a[j].p_اظہاریہ, iBMem+j);
        }

        اگر( i_ترتیب_By_ستون ){
          اظہاریہ *pX = p->pEList->a[i_ترتیب_By_ستون-1].p_اظہاریہ;
          اظہاریہ *p_بنیاد = sqlite3_اظہاریہ_Skip_Collate_And_Likely(pX);
          جب( ALWAYS(p_بنیاد!=0) && p_بنیاد->op==TK_IF_NULL_صف ){
            pX = p_بنیاد->p_بایاں;
            p_بنیاد = sqlite3_اظہاریہ_Skip_Collate_And_Likely(pX);
          }
          اگر( ALWAYS(p_بنیاد!=0)
           && p_بنیاد->op!=TK_AGG_عمود
           && p_بنیاد->op!=TK_رجسٹر
          ){
            sqlite3_اظہاریہ_مقصد_رجسٹر(pX, iAMem+j);
          }
        }
      }
      sqlite3_Vdbe_جوڑو_Op4(v, OP_Compare, iAMem, iBMem, p_گروہ_By->n_اظہاریہ,
                          (حرف*)sqlite3_کلید_Info_Ref(p_کلید_Info), P4_KEYINFO);
      addr1 = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_جوڑو_Op3(v, OP_Jump, addr1+1, 0, addr1+1); VdbeCoverage(v);

      /* Generate code that runs whenever the GROUP BY changes.
      ** Changes in the GROUP BY are detected by the previous code
      ** block.  If there were no changes, this block is skipped.
      **
      ** This code copies current group by terms in b0,b1,b2,...
      ** over to a0,a1,a2.  It then calls the output subroutine
      ** and resets the aggregate accumulator registers in preparation
      ** for the next GROUP BY batch.
      */
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, reg_Output_صف, addr_Output_صف);
      VdbeComment((v, "output one row of %d", p->selId));
      sqlite3_اظہاریہ_رمز_Move(pParse, iBMem, iAMem, p_گروہ_By->n_اظہاریہ);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_IfPos, iAbortFlag, addr_انتہا); VdbeCoverage(v);
      VdbeComment((v, "check abort flag"));
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, regReset, addrReset);
      VdbeComment((v, "reset accumulator %d", p->selId));

      /* Update the aggregate accumulators based on the content of
      ** the current row
      */
      sqlite3_Vdbe_Jump_یہاں(v, addr1);
      update_مجمع(pParse, iUseFlag, pAggInfo, eDist);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 1, iUseFlag);
      VdbeComment((v, "indicate data in accumulator %d", p->selId));

      /* End of the loop
      */
      اگر( گروہ_By_ترتیب_دو ){
        sqlite3_Vdbe_جوڑو_Op2(v, OP_Sorter_اگلا, pAggInfo->ترتیب_کاری_Idx,addr_چوٹی_کا_Loop);
        VdbeCoverage(v);
      }ورنہ{
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_انتہا(pWInfo);
        sqlite3_Vdbe_Change_مقصد_Noop(v, addr_ترتیب_کاری_Idx);
      }
      sqlite3_اظہاریہ_List_Delete(db, pDistinct);

      /* Output the final row of result
      */
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Gosub, reg_Output_صف, addr_Output_صف);
      VdbeComment((v, "output final row of %d", p->selId));

      /* Jump over the subroutines
      */
      sqlite3VdbeGoto(v, addr_انتہا);

      /* Generate a subroutine that outputs a single row of the result
      ** set.  This subroutine first looks at the iUseFlag.  If iUseFlag
      ** is less than or equal to zero, the subroutine is a no-op.  If
      ** the processing calls for the query to abort, this subroutine
      ** increments the iAbortFlag memory location before returning in
      ** order to signal the caller to abort.
      */
      addr_سیٹ_Abort = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 1, iAbortFlag);
      VdbeComment((v, "set abort flag"));
      sqlite3_Vdbe_جوڑو_Op1(v, OP_Return, reg_Output_صف);
      sqlite3VdbeResolveLabel(v, addr_Output_صف);
      addr_Output_صف = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_IfPos, iUseFlag, addr_Output_صف+2);
      VdbeCoverage(v);
      VdbeComment((v, "Groupby result generator entry point %d", p->selId));
      sqlite3_Vdbe_جوڑو_Op1(v, OP_Return, reg_Output_صف);
      finalizeAggFunctions(pParse, pAggInfo);
      sqlite3_اظہاریہ_If_False(pParse, pHaving, addr_Output_صف+1, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, &s_ترتیب_دو,
                      &sDistinct, pDest,
                      addr_Output_صف+1, addr_سیٹ_Abort);
      sqlite3_Vdbe_جوڑو_Op1(v, OP_Return, reg_Output_صف);
      VdbeComment((v, "end groupby result generator %d", p->selId));

      /* Generate a subroutine that will reset the group-by accumulator
      */
      sqlite3VdbeResolveLabel(v, addrReset);
      reset_مجمع(pParse, pAggInfo);
      sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, iUseFlag);
      VdbeComment((v, "indicate accumulator %d empty", p->selId));
      sqlite3_Vdbe_جوڑو_Op1(v, OP_Return, regReset);

      اگر( distFlag!=0 && eDist!=WHERE_DISTINCT_NOOP ){
        ساخت AggInfo_func *pF = &pAggInfo->aFunc[0];
        fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
      }
    } /* endif pGroupBy.  Begin aggregate queries without GROUP BY: */
    ورنہ {
      /* Aggregate functions without GROUP BY. tag-select-0820 */
      جدول *pTab;
      اگر( (pTab = is_Simple_گنتی(p, pAggInfo))!=0 ){
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
        ثابت صحیح iDb = sqlite3_Schema_مقصد_اشاریہ(pParse->db, pTab->pSchema);
        ثابت صحیح iCsr = pParse->nTab++;     /* Cursor to scan b-tree */
        اشاریہ *pIdx;                         /* Iterator variable */
        کلید_Info *p_کلید_Info = 0;               /* Keyinfo for scanned index */
        اشاریہ *p_بہترین = 0;                    /* Best index found so far */
        Pgno i_جڑ = pTab->tnum;             /* Root page of scanned b-tree */

        sqlite3_رمز_Verify_Schema(pParse, iDb);
        sqlite3_جدول_تالا(pParse, iDb, pTab->tnum, 0, pTab->zName);

        /* Search for the index that has the lowest scan cost.
        **
        ** (2011-04-15) Do not do a full scan of an unordered index.
        **
        ** (2013-10-03) Do not count the entries in a partial index.
        **
        ** In practice the KeyInfo structure will not be used. It is only
        ** passed to keep OP_OpenRead happy.
        */
        اگر( !HasRowid(pTab) ) p_بہترین = sqlite3_Primary_کلید_اشاریہ(pTab);
        اگر( !p->pSrc->a[0].fg.not_اشاریہ_دار ){
          برائے(pIdx=pTab->p_اشاریہ; pIdx; pIdx=pIdx->p_اگلا){
            اگر( pIdx->bUnordered==0
             && pIdx->sz_Idx_صف<pTab->sz_Tab_صف
             && pIdx->pPartIdxWhere==0
             && (!p_بہترین || pIdx->sz_Idx_صف<p_بہترین->sz_Idx_صف)
            ){
              p_بہترین = pIdx;
            }
          }
        }
        اگر( p_بہترین ){
          i_جڑ = p_بہترین->tnum;
          p_کلید_Info = sqlite3_کلید_Info_کا_اشاریہ(pParse, p_بہترین);
        }

        /* Open a read-only cursor, execute the OP_Count, close the cursor. */
        sqlite3_Vdbe_جوڑو_Op4_Int(v, OP_OpenRead, iCsr, (صحیح)i_جڑ, iDb, 1);
        اگر( p_کلید_Info ){
          sqlite3VdbeChangeP4(v, -1, (حرف *)p_کلید_Info, P4_KEYINFO);
        }
        تفویض_Aggregate_Registers(pParse, pAggInfo);
        sqlite3_Vdbe_جوڑو_Op2(v, OP_گنتی, iCsr, AggInfoFuncReg(pAggInfo,0));
        sqlite3_Vdbe_جوڑو_Op1(v, OP_Close, iCsr);
        explain_Simple_گنتی(pParse, pTab, p_بہترین);
      }ورنہ{
        /* The general case of an aggregate query without GROUP BY
        ** tag-select-0822 */
        صحیح regAcc = 0;           /* "populate accumulators" flag */
        اظہاریہ_List *pDistinct = 0;
        u16 distFlag = 0;
        صحیح eDist;

        /* If there are accumulator registers but no min() or max() functions
        ** without FILTER clauses, allocate register regAcc. Register regAcc
        ** will contain 0 the first time the inner loop runs, and 1 thereafter.
        ** The code generated by updateAccumulator() uses this to ensure
        ** that the accumulator registers are (a) updated only once if
        ** there are no min() or max functions or (b) always updated for the
        ** first row visited by the aggregate, so that they are updated at
        ** least once even if the FILTER clause means the min() or max()
        ** function visits zero rows.  */
        اگر( pAggInfo->n_مجمع ){
          برائے(i=0; i<pAggInfo->nFunc; i++){
            اگر( اظہاریہ_Has_Property(pAggInfo->aFunc[i].pFExpr, EP_WinFunc) ){
              جاری;
            }
            اگر( pAggInfo->aFunc[i].pFunc->funcFlags&SQLITE_FUNC_NEEDCOLL ){
              توڑو;
            }
          }
          اگر( i==pAggInfo->nFunc ){
            regAcc = ++pParse->nMem;
            sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 0, regAcc);
          }
        }ورنہ اگر( pAggInfo->nFunc==1 && pAggInfo->aFunc[0].iDistinct>=0 ){
          أكد( اظہاریہ_Use_XList(pAggInfo->aFunc[0].pFExpr) );
          pDistinct = pAggInfo->aFunc[0].pFExpr->x.pList;
          distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
        }
        تفویض_Aggregate_Registers(pParse, pAggInfo);

        /* This case runs if the aggregate has no GROUP BY clause.  The
        ** processing is much simpler since there is only a single row
        ** of output.
        */
        أكد( p->p_گروہ_By==0 );
        reset_مجمع(pParse, pAggInfo);

        /* If this query is a candidate for the min/max optimization, then
        ** minMaxFlag will have been previously set to either
        ** WHERE_ORDERBY_MIN or WHERE_ORDERBY_MAX and pMinMaxOrderBy will
        ** be an appropriate ORDER BY expression for the optimization.
        */
        أكد( minMaxFlag==WHERE_ORDERBY_معمول || p_Min_Max_ترتیب_By!=0 );
        أكد( p_Min_Max_ترتیب_By==0 || p_Min_Max_ترتیب_By->n_اظہاریہ==1 );

        TREETRACE(0x2,pParse,p,("WhereBegin\n"));
        pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, p_Min_Max_ترتیب_By,
                                   pDistinct, p, minMaxFlag|distFlag, 0);
        اگر( pWInfo==0 ){
          جاؤ select_انتہا;
        }
        TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
        eDist = sqlite3WhereIsDistinct(pWInfo);
        update_مجمع(pParse, regAcc, pAggInfo, eDist);
        اگر( eDist!=WHERE_DISTINCT_NOOP ){
          ساخت AggInfo_func *pF = pAggInfo->aFunc;
          اگر( pF ){
            fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
          }
        }

        اگر( regAcc ) sqlite3_Vdbe_جوڑو_Op2(v, OP_Integer, 1, regAcc);
        اگر( minMaxFlag ){
          sqlite3_Where_Min_Max_Opt_Early_برآمد(v, pWInfo);
        }
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_انتہا(pWInfo);
        finalizeAggFunctions(pParse, pAggInfo);
      }

      s_ترتیب_دو.p_ترتیب_By = 0;
      sqlite3_اظہاریہ_If_False(pParse, pHaving, addr_انتہا, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, 0, 0,
                      pDest, addr_انتہا, addr_انتہا);
    }
    sqlite3VdbeResolveLabel(v, addr_انتہا);

  } /* endif aggregate query */

  اگر( sDistinct.eTnctType==WHERE_DISTINCT_UNORDERED ){
    explain_عارضی_جدول(pParse, "DISTINCT");
  }

  /* If there is an ORDER BY clause, then we need to sort the results
  ** and send them to the callback one by one.  tag-select-0900
  */
  اگر( s_ترتیب_دو.p_ترتیب_By ){
    أكد( p->pEList==pEList );
    generate_ترتیب_دو_دم(pParse, p, &s_ترتیب_دو, pEList->n_اظہاریہ, pDest);
  }

  /* Jump here to skip this query
  */
  sqlite3VdbeResolveLabel(v, i_انتہا);

  /* The SELECT has been coded. If there is an error in the Parse structure,
  ** set the return code to 1. Otherwise 0. */
  rc = (pParse->nErr>0);

  /* Control jumps to here if an error is encountered above, or upon
  ** successful coding of the SELECT.
  */
select_انتہا:
  أكد( db->mallocFailed==0 || db->mallocFailed==1 );
  أكد( db->mallocFailed==0 || pParse->nErr!=0 );
  sqlite3_اظہاریہ_List_Delete(db, p_Min_Max_ترتیب_By);
#اگر_تعریف_شدہ SQLITE_DEBUG
  /* Internal self-checks.  tag-select-1000 */
  اگر( pAggInfo && !db->mallocFailed ){
#اگر TREETRACE_ENABLED
    اگر( sqlite3_درخت_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("Finished with AggInfo\n"));
      printAggInfo(pAggInfo);
    }
#اگر_ختم
    برائے(i=0; i<pAggInfo->n_عمود; i++){
      اظہاریہ *p_اظہاریہ = pAggInfo->a_ستون[i].pCExpr;
      اگر( p_اظہاریہ==0 ) جاری;
      أكد( p_اظہاریہ->pAggInfo==pAggInfo );
      أكد( p_اظہاریہ->iAgg==i );
    }
    برائے(i=0; i<pAggInfo->nFunc; i++){
      اظہاریہ *p_اظہاریہ = pAggInfo->aFunc[i].pFExpr;
      أكد( p_اظہاریہ!=0 );
      أكد( p_اظہاریہ->pAggInfo==pAggInfo );
      أكد( p_اظہاریہ->iAgg==i );
    }
  }
#اگر_ختم

#اگر TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p,("end processing\n"));
  اگر( (sqlite3_درخت_Trace & 0x40000)!=0 && Explain_Query_Plan_والد(pParse)==0 ){
    sqlite3_درخت_View_Select(0, p, 0);
  }
#اگر_ختم
  Explain_Query_Plan_نکالو(pParse);
  واپس rc;
}
