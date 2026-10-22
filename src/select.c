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
#అన్తర్భావ "sqliteInt.h"

/*
** An instance of the following object is used to record information about
** how to process the DISTINCT keyword, to simplify passing that information
** into the selectInnerLoop() routine.
*/
రకనిర్వచనం నిర్మాణం DistinctCtx DistinctCtx;
నిర్మాణం DistinctCtx {
  u8 isTnct;      /* 0: Not distinct. 1: DISTINCT  2: DISTINCT and ORDER BY */
  u8 eTnctType;   /* One of the WHERE_DISTINCT_* operators */
  పూర్ణాంకం tabTnct;    /* Ephemeral table used for DISTINCT processing */
  పూర్ణాంకం addrTnct;   /* Address of OP_OpenEphemeral opcode for tabTnct */
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
రకనిర్వచనం నిర్మాణం క్రమీకరించు_Ctx క్రమీకరించు_Ctx;
నిర్మాణం క్రమీకరించు_Ctx {
  సమాసం_List *p_క్రమం_By;   /* The ORDER BY (or GROUP BY clause) */
  పూర్ణాంకం nOBSat;           /* Number of ORDER BY terms satisfied by indices */
  పూర్ణాంకం iECursor;         /* Cursor number for the sorter */
  పూర్ణాంకం regReturn;        /* Register holding block-output return address */
  పూర్ణాంకం label_Bk_బహిర్గతం;       /* Start label for the block-output subroutine */
  పూర్ణాంకం addr_క్రమీకరించు_సూచీ;    /* Address of the OP_SorterOpen or OP_OpenEphemeral */
  పూర్ణాంకం label_పూర్తి;        /* Jump here when done, ex: LIMIT reached */
  పూర్ణాంకం labelOBLopt;      /* Jump here when sorter is full */
  u8 క్రమీకరించు_Flags;         /* Zero or more SORTFLAG_* bits */
#నిర్వచితమైతే SQLITE_ENABLE_SORTER_REFERENCES
  u8 nDefer;            /* Number of valid entries in aDefer[] */
  నిర్మాణం DeferredCsr {
    పట్టిక *pTab;        /* Table definition */
    పూర్ణాంకం iCsr;           /* Cursor number for table */
    పూర్ణాంకం n_కీలకం;           /* Number of PK columns for table pTab (>=1) */
  } aDefer[4];
#అయితే_ముగింపు
  నిర్మాణం అడ్డువరుస_ఎక్కించు_Info *p_Deferred_అడ్డువరుస_ఎక్కించు;  /* Deferred row loading info or NULL */
#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
  పూర్ణాంకం addr_నెట్టు;         /* First instruction to push data into sorter */
  పూర్ణాంకం addr_నెట్టు_చివర;      /* Last instruction that pushes data into sorter */
#అయితే_ముగింపు
};
#నిర్వచించు SORTFLAG_UseSorter  0x01   /* Use SorterOpen instead of OpenEphemeral */

/*
** Delete all the content of a Select structure.  Deallocate the structure
** itself depending on the value of bFree
**
** If bFree==1, call sqlite3DbFree() on the p object.
** If bFree==0, Leave the first Select object unfreed
*/
స్థిర శూన్య clearSelect(sqlite3 *db, Select *p, పూర్ణాంకం bFree){
  ప్రతిజ్ఞా( db!=0 );
  అయితే( p ){
    Select *p_పూర్వ = p->p_పూర్వ;
    sqlite3_సమాసం_List_Delete(db, p->pEList);
    sqlite3SrcListDelete(db, p->pSrc);
    sqlite3_సమాసం_Delete(db, p->pWhere);
    sqlite3_సమాసం_List_Delete(db, p->p_సమూహం_By);
    sqlite3_సమాసం_Delete(db, p->pHaving);
    sqlite3_సమాసం_List_Delete(db, p->p_క్రమం_By);
    sqlite3_సమాసం_Delete(db, p->p_హద్దు);
    ఒకవేళ( OK_IF_ALWAYS_TRUE(p->pWith) ) sqlite3WithDelete(db, p->pWith);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    ఒకవేళ( OK_IF_ALWAYS_TRUE(p->pWinDefn) ){
      sqlite3_కిటికీ_List_Delete(db, p->pWinDefn);
    }
    అయితే( p->pWin ){
      ప్రతిజ్ఞా( p->pWin->ppThis==&p->pWin );
      sqlite3_కిటికీ_Unlink_From_Select(p->pWin);
    }
#అయితే_ముగింపు
    ఒకవేళ( bFree ) sqlite3DbNNFreeNN(db, p);
    p = p_పూర్వ;
    bFree = 1;
  }
}

/*
** Initialize a SelectDest structure.
*/
శూన్య sqlite3SelectDestInit(SelectDest *pDest, పూర్ణాంకం eDest, పూర్ణాంకం iParm){
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
  సమాసం_List *pEList,     /* which columns to include in the result */
  SrcList *pSrc,        /* the FROM clause -- which tables to scan */
  సమాసం *pWhere,         /* the WHERE clause */
  సమాసం_List *p_సమూహం_By,   /* the GROUP BY clause */
  సమాసం *pHaving,        /* the HAVING clause */
  సమాసం_List *p_క్రమం_By,   /* the ORDER BY clause */
  u32 selFlags,         /* Flag parameters, such as SF_Distinct */
  సమాసం *p_హద్దు          /* LIMIT value.  NULL means not used */
){
  Select *pNew, *pAllocated;
  Select standin;
  pAllocated = pNew = sqlite3DbMallocRawNN(pParse->db, పరిమాణం(*pNew) );
  ఒకవేళ( pNew==0 ){
    ప్రతిజ్ఞా( pParse->db->mallocFailed );
    pNew = &standin;
  }
  ఒకవేళ( pEList==0 ){
    pEList = sqlite3_సమాసం_List_Append(pParse, 0,
                                   sqlite3_సమాసం(pParse->db,TK_ASTERISK,0));
  }
  pNew->pEList = pEList;
  pNew->op = TK_SELECT;
  pNew->selFlags = selFlags;
  pNew->i_హద్దు = 0;
  pNew->i_స్థానభ్రంశం = 0;
  pNew->selId = ++pParse->nSelect;
  pNew->n_Select_అడ్డువరుస = 0;
  ఒకవేళ( pSrc==0 ) pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  pNew->pSrc = pSrc;
  pNew->pWhere = pWhere;
  pNew->p_సమూహం_By = p_సమూహం_By;
  pNew->pHaving = pHaving;
  pNew->p_క్రమం_By = p_క్రమం_By;
  pNew->p_పూర్వ = 0;
  pNew->p_తదుపరి = 0;
  pNew->p_హద్దు = p_హద్దు;
  pNew->pWith = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
  pNew->pWin = 0;
  pNew->pWinDefn = 0;
#అయితే_ముగింపు
  ఒకవేళ( pParse->db->mallocFailed ) {
    clearSelect(pParse->db, pNew, pNew!=&standin);
    pAllocated = 0;
  }లేకపోతే{
    ప్రతిజ్ఞా( pNew->pSrc!=0 || pParse->nErr>0 );
  }
  తిరిగి pAllocated;
}


/*
** Delete the given Select structure and all of its substructures.
*/
శూన్య sqlite3SelectDelete(sqlite3 *db, Select *p){
  ఒకవేళ( OK_IF_ALWAYS_TRUE(p) ) clearSelect(db, p, 1);
}
శూన్య sqlite3SelectDeleteGeneric(sqlite3 *db, శూన్య *p){
  ఒకవేళ( ALWAYS(p) ) clearSelect(db, (Select*)p, 1);
}

/*
** Return a pointer to the right-most SELECT statement in a compound.
*/
స్థిర Select *కనుగొను_Rightmost(Select *p){
  అయితే( p->p_తదుపరి ) p = p->p_తదుపరి;
  తిరిగి p;
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
పూర్ణాంకం sqlite3JoinType(Parse *pParse, Token *pA, Token *pB, Token *pC){
  పూర్ణాంకం jointype = 0;
  Token *apAll[3];
  Token *p;
                             /*   0123456789 123456789 123456789 123 */
  స్థిర స్థిరరాశి అక్షరం z_కీలకం_పాఠ్యం[] = "naturaleftouterightfullinnercross";
  స్థిర స్థిరరాశి నిర్మాణం {
    u8 i;        /* Beginning of keyword text in zKeyText[] */
    u8 nChar;    /* Length of the keyword in characters */
    u8 సంకేతం_2;     /* Join type mask */
  } aKeyword[] = {
    /* (0) natural */ { 0,  7, JT_NATURAL                },
    /* (1) left    */ { 6,  4, JT_ఎడమ|JT_OUTER          },
    /* (2) outer   */ { 10, 5, JT_OUTER                  },
    /* (3) right   */ { 14, 5, JT_కుడి|JT_OUTER         },
    /* (4) full    */ { 19, 4, JT_ఎడమ|JT_కుడి|JT_OUTER },
    /* (5) inner   */ { 23, 5, JT_INNER                  },
    /* (6) cross   */ { 28, 5, JT_INNER|JT_CROSS         },
  };
  పూర్ణాంకం i, j;
  apAll[0] = pA;
  apAll[1] = pB;
  apAll[2] = pC;
  కొరకు(i=0; i<3 && apAll[i]; i++){
    p = apAll[i];
    కొరకు(j=0; j<Array_కొలత(aKeyword); j++){
      ఒకవేళ( p->n==aKeyword[j].nChar
          && sqlite3StrNICmp((అక్షరం*)p->z, &z_కీలకం_పాఠ్యం[aKeyword[j].i], p->n)==0 ){
        jointype |= aKeyword[j].సంకేతం_2;
        ఆపు;
      }
    }
    testcase( j==0 || j==1 || j==2 || j==3 || j==4 || j==5 || j==6 );
    ఒకవేళ( j>=Array_కొలత(aKeyword) ){
      jointype |= JT_ERROR;
      ఆపు;
    }
  }
  ఒకవేళ(
     (jointype & (JT_INNER|JT_OUTER))==(JT_INNER|JT_OUTER) ||
     (jointype & JT_ERROR)!=0 ||
     (jointype & (JT_OUTER|JT_ఎడమ|JT_కుడి))==JT_OUTER
  ){
    స్థిరరాశి అక్షరం *zSp1 = " ";
    స్థిరరాశి అక్షరం *zSp2 = " ";
    ఒకవేళ( pB==0 ){ zSp1++; }
    ఒకవేళ( pC==0 ){ zSp2++; }
    sqlite3_Error_సందేశం(pParse, "unknown join type: "
       "%T%s%T%s%T", pA, zSp1, pB, zSp2, pC);
    jointype = JT_INNER;
  }
  తిరిగి jointype;
}

/*
** Return the index of a column in a table.  Return -1 if the column
** is not contained in the table.
*/
పూర్ణాంకం sqlite3_స్తంభం_సూచీ(పట్టిక *pTab, స్థిరరాశి అక్షరం *z_నిలువు){
  పూర్ణాంకం i;
  u8 h;
  స్థిరరాశి స్తంభం *a_నిలువు;
  పూర్ణాంకం n_నిలువు;

  h = sqlite3StrIHash(z_నిలువు);
  a_నిలువు = pTab->a_నిలువు;
  n_నిలువు = pTab->n_నిలువు;

  /* See if the aHx gives us a lucky match */
  i = pTab->aHx[h % పరిమాణం(pTab->aHx)];
  ప్రతిజ్ఞా( i<n_నిలువు );
  ఒకవేళ( a_నిలువు[i].hName==h
   && sqlite3StrICmp(a_నిలువు[i].zCnName, z_నిలువు)==0
  ){
    తిరిగి i;
  }

  /* No lucky match from the hash table.  Do a full search. */
  i = 0;
  అయితే( 1 /*exit-by-break*/ ){
    ఒకవేళ( a_నిలువు[i].hName==h
     && sqlite3StrICmp(a_నిలువు[i].zCnName, z_నిలువు)==0
    ){
      తిరిగి i;
    }
    i++;
    ఒకవేళ( i>=n_నిలువు ) ఆపు;
  }
  తిరిగి -1;
}

/*
** Mark a subquery result column as having been used.
*/
శూన్య sqlite3_Src_అంశం_స్తంభం_వాడిన(Src_అంశం *p_అంశం, పూర్ణాంకం i_నిలువు){
  ప్రతిజ్ఞా( p_అంశం!=0 );
  ప్రతిజ్ఞా( (పూర్ణాంకం)p_అంశం->fg.isNestedFrom == IsNestedFrom(p_అంశం) );
  ఒకవేళ( p_అంశం->fg.isNestedFrom ){
    సమాసం_List *pResults;
    ప్రతిజ్ఞా( p_అంశం->fg.isSubquery );
    ప్రతిజ్ఞా( p_అంశం->u4.pSubq!=0 );
    ప్రతిజ్ఞా( p_అంశం->u4.pSubq->pSelect!=0 );
    pResults = p_అంశం->u4.pSubq->pSelect->pEList;
    ప్రతిజ్ఞా( pResults!=0 );
    ప్రతిజ్ఞా( i_నిలువు>=0 && i_నిలువు<pResults->n_సమాసం );
    pResults->a[i_నిలువు].fg.b_వాడిన = 1;
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
స్థిర పూర్ణాంకం పట్టిక_And_స్తంభం_సూచీ(
  SrcList *pSrc,       /* Array of tables to search */
  పూర్ణాంకం i_ప్రారంభం,          /* First member of pSrc->a[] to check */
  పూర్ణాంకం i_చివర,            /* Last member of pSrc->a[] to check */
  స్థిరరాశి అక్షరం *z_నిలువు,    /* Name of the column we are looking for */
  పూర్ణాంకం *piTab,          /* Write index of pSrc->a[] here */
  పూర్ణాంకం *pi_నిలువు,          /* Write index of pSrc->a[*piTab].pSTab->aCol[] here */
  పూర్ణాంకం bIgnoreHidden    /* Ignore hidden columns */
){
  పూర్ణాంకం i;               /* For looping over tables in pSrc */
  పూర్ణాంకం i_నిలువు;            /* Index of column matching zCol */

  ప్రతిజ్ఞా( i_చివర<pSrc->nSrc );
  ప్రతిజ్ఞా( i_ప్రారంభం>=0 );
  ప్రతిజ్ఞా( (piTab==0)==(pi_నిలువు==0) );  /* Both or neither are NULL */

  కొరకు(i=i_ప్రారంభం; i<=i_చివర; i++){
    i_నిలువు = sqlite3_స్తంభం_సూచీ(pSrc->a[i].pSTab, z_నిలువు);
    ఒకవేళ( i_నిలువు>=0
     && (bIgnoreHidden==0 || Is_Hidden_స్తంభం(&pSrc->a[i].pSTab->a_నిలువు[i_నిలువు])==0)
    ){
      ఒకవేళ( piTab ){
        sqlite3_Src_అంశం_స్తంభం_వాడిన(&pSrc->a[i], i_నిలువు);
        *piTab = i;
        *pi_నిలువు = i_నిలువు;
      }
      తిరిగి 1;
    }
  }
  తిరిగి 0;
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
శూన్య sqlite3_సమితి_Join_సమాసం(సమాసం *p, పూర్ణాంకం i_పట్టిక, u32 joinFlag){
  ప్రతిజ్ఞా( joinFlag==EP_OuterON || joinFlag==EP_InnerON );
  అయితే( p ){
    సమాసం_సమితి_Property(p, joinFlag);
    ప్రతిజ్ఞా( !సమాసం_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    సమాసం_సమితి_VVAProperty(p, EP_NoReduce);
    p->w.iJoin = i_పట్టిక;
    ఒకవేళ( సమాసం_Use_XList(p) ){
      ఒకవేళ( p->x.pList ){
        పూర్ణాంకం i;
        కొరకు(i=0; i<p->x.pList->n_సమాసం; i++){
          sqlite3_సమితి_Join_సమాసం(p->x.pList->a[i].p_సమాసం, i_పట్టిక, joinFlag);
        }
      }
    }
    sqlite3_సమితి_Join_సమాసం(p->p_ఎడమ, i_పట్టిక, joinFlag);
    p = p->p_కుడి;
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
స్థిర శూన్య unset_Join_సమాసం(సమాసం *p, పూర్ణాంకం i_పట్టిక, పూర్ణాంకం nullable){
  అయితే( p ){
    ఒకవేళ( i_పట్టిక<0 || (సమాసం_Has_Property(p, EP_OuterON) && p->w.iJoin==i_పట్టిక) ){
      సమాసం_Clear_Property(p, EP_OuterON|EP_InnerON);
      ఒకవేళ( i_పట్టిక>=0 ) సమాసం_సమితి_Property(p, EP_InnerON);
    }
    ఒకవేళ( p->op==TK_స్తంభం && p->i_పట్టిక==i_పట్టిక && !nullable ){
      సమాసం_Clear_Property(p, EP_CanBeNull);
    }
    ఒకవేళ( p->op==TK_FUNCTION ){
      ప్రతిజ్ఞా( సమాసం_Use_XList(p) );
      ప్రతిజ్ఞా( p->p_ఎడమ==0 );
      ఒకవేళ( p->x.pList ){
        పూర్ణాంకం i;
        కొరకు(i=0; i<p->x.pList->n_సమాసం; i++){
          unset_Join_సమాసం(p->x.pList->a[i].p_సమాసం, i_పట్టిక, nullable);
        }
      }
    }
    unset_Join_సమాసం(p->p_ఎడమ, i_పట్టిక, nullable);
    p = p->p_కుడి;
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
స్థిర పూర్ణాంకం sqlite3ProcessJoin(Parse *pParse, Select *p){
  SrcList *pSrc;                  /* All tables in the FROM clause */
  పూర్ణాంకం i, j;                       /* Loop counters */
  Src_అంశం *p_ఎడమ;                 /* Left table being joined */
  Src_అంశం *p_కుడి;                /* Right table being joined */
  పూర్ణాంకం b_చూసిన_TFunc = 0;             /* True if have seen table-valued function */

  pSrc = p->pSrc;
  p_ఎడమ = &pSrc->a[0];
  p_కుడి = &p_ఎడమ[1];
  కొరకు(i=0; i<pSrc->nSrc-1; i++, p_కుడి++, p_ఎడమ++){
    పట్టిక *p_కుడి_Tab = p_కుడి->pSTab;
    u32 joinType;

    ఒకవేళ( NEVER(p_ఎడమ->pSTab==0 || p_కుడి_Tab==0) ) కొనసాగు;
    joinType = (p_కుడి->fg.jointype & JT_OUTER)!=0 ? EP_OuterON : EP_InnerON;

    /* If this is a NATURAL join, synthesize an appropriate USING clause
    ** to specify which columns should be joined.
    */
    ఒకవేళ( p_కుడి->fg.jointype & JT_NATURAL ){
      IdList *pUsing = 0;
      ఒకవేళ( p_కుడి->fg.isUsing || p_కుడి->u3.pOn ){
        sqlite3_Error_సందేశం(pParse, "a NATURAL join may not have "
           "an ON or USING clause", 0);
        తిరిగి 1;
      }
      కొరకు(j=0; j<p_కుడి_Tab->n_నిలువు; j++){
        అక్షరం *zName;   /* Name of column in the right table */

        ఒకవేళ( Is_Hidden_స్తంభం(&p_కుడి_Tab->a_నిలువు[j]) ) కొనసాగు;
        zName = p_కుడి_Tab->a_నిలువు[j].zCnName;
        ఒకవేళ( పట్టిక_And_స్తంభం_సూచీ(pSrc, 0, i, zName, 0, 0, 1) ){
          pUsing = sqlite3IdListAppend(pParse, pUsing, 0);
          ఒకవేళ( pUsing ){
            ప్రతిజ్ఞా( pUsing->nId>0 );
            ప్రతిజ్ఞా( pUsing->a[pUsing->nId-1].zName==0 );
            pUsing->a[pUsing->nId-1].zName = sqlite3DbStrDup(pParse->db, zName);
          }
        }
      }
      ఒకవేళ( pUsing ){
        p_కుడి->fg.isUsing = 1;
        p_కుడి->fg.isSynthUsing = 1;
        p_కుడి->u3.pUsing = pUsing;
      }
      ఒకవేళ( pParse->nErr ) తిరిగి 1;
    }

    /* Create extra terms on the WHERE clause for each column named
    ** in the USING clause.  Example: If the two tables to be joined are
    ** A and B and the USING clause names X, Y, and Z, then add this
    ** to the WHERE clause:    A.X=B.X AND A.Y=B.Y AND A.Z=B.Z
    ** Report an error if any column mentioned in the USING clause is
    ** not contained in both tables to be joined.
    */
    ఒకవేళ( p_కుడి->fg.isUsing ){
      IdList *pList = p_కుడి->u3.pUsing;
      sqlite3 *db = pParse->db;
      ప్రతిజ్ఞా( pList!=0 );
      కొరకు(j=0; j<pList->nId; j++){
        అక్షరం *zName;     /* Name of the term in the USING clause */
        పూర్ణాంకం i_ఎడమ;       /* Table on the left with matching column name */
        పూర్ణాంకం i_ఎడమ_నిలువు;    /* Column number of matching column on the left */
        పూర్ణాంకం i_కుడి_నిలువు;   /* Column number of matching column on the right */
        సమాసం *pE1;       /* Reference to the column on the LEFT of the join */
        సమాసం *pE2;       /* Reference to the column on the RIGHT of the join */
        సమాసం *pEq;       /* Equality constraint.  pE1 == pE2 */

        zName = pList->a[j].zName;
        i_కుడి_నిలువు = sqlite3_స్తంభం_సూచీ(p_కుడి_Tab, zName);
        ఒకవేళ( i_కుడి_నిలువు<0
         || పట్టిక_And_స్తంభం_సూచీ(pSrc, 0, i, zName, &i_ఎడమ, &i_ఎడమ_నిలువు,
                                p_కుడి->fg.isSynthUsing)==0
        ){
          sqlite3_Error_సందేశం(pParse, "cannot join using column %s - column "
            "not present in both tables", zName);
          తిరిగి 1;
        }
        pE1 = sqlite3_Create_స్తంభం_సమాసం(db, pSrc, i_ఎడమ, i_ఎడమ_నిలువు);
        sqlite3_Src_అంశం_స్తంభం_వాడిన(&pSrc->a[i_ఎడమ], i_ఎడమ_నిలువు);
        ఒకవేళ( (pSrc->a[0].fg.jointype & JT_LTORJ)!=0 && pParse->nErr==0 ){
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
          సమాసం_List *p_Func_వాదనలు = 0;   /* Arguments to the coalesce() */
          స్థిర స్థిరరాశి Token tkCoalesce = { "coalesce", 8 };
          ప్రతిజ్ఞా( pE1!=0 );
          సమాసం_సమితి_Property(pE1, EP_CanBeNull);
          అయితే( పట్టిక_And_స్తంభం_సూచీ(pSrc, i_ఎడమ+1, i, zName, &i_ఎడమ, &i_ఎడమ_నిలువు,
                                     p_కుడి->fg.isSynthUsing)!=0 ){
            ఒకవేళ( pSrc->a[i_ఎడమ].fg.isUsing==0
             || sqlite3_Id_List_సూచీ(pSrc->a[i_ఎడమ].u3.pUsing, zName)<0
            ){
              sqlite3_Error_సందేశం(pParse, "ambiguous reference to %s in USING()",
                              zName);
              ఆపు;
            }
            p_Func_వాదనలు = sqlite3_సమాసం_List_Append(pParse, p_Func_వాదనలు, pE1);
            pE1 = sqlite3_Create_స్తంభం_సమాసం(db, pSrc, i_ఎడమ, i_ఎడమ_నిలువు);
            sqlite3_Src_అంశం_స్తంభం_వాడిన(&pSrc->a[i_ఎడమ], i_ఎడమ_నిలువు);
          }
          ఒకవేళ( p_Func_వాదనలు ){
            p_Func_వాదనలు = sqlite3_సమాసం_List_Append(pParse, p_Func_వాదనలు, pE1);
            pE1 = sqlite3_సమాసం_Function(pParse, p_Func_వాదనలు, &tkCoalesce, 0);
            ఒకవేళ( pE1 ){
              pE1->aff_సమాసం = SQLITE_AFF_DEFER;
            }
          }
        }లేకపోతే ఒకవేళ( (pSrc->a[i+1].fg.jointype & JT_ఎడమ)!=0 && pParse->nErr==0 ){
          ప్రతిజ్ఞా( pE1!=0 );
          సమాసం_సమితి_Property(pE1, EP_CanBeNull);
        }
        pE2 = sqlite3_Create_స్తంభం_సమాసం(db, pSrc, i+1, i_కుడి_నిలువు);
        sqlite3_Src_అంశం_స్తంభం_వాడిన(p_కుడి, i_కుడి_నిలువు);
        pEq = sqlite3PExpr(pParse, TK_EQ, pE1, pE2);
        ప్రతిజ్ఞా( pE2!=0 || pEq==0 );
        ఒకవేళ( pEq ){
          సమాసం_సమితి_Property(pEq, joinType);
          ప్రతిజ్ఞా( !సమాసం_Has_Property(pEq, EP_TokenOnly|EP_Reduced) );
          సమాసం_సమితి_VVAProperty(pEq, EP_NoReduce);
          pEq->w.iJoin = pE2->i_పట్టిక;
        }
        p->pWhere = sqlite3_సమాసం_And(pParse, p->pWhere, pEq);
      }
    }

    /* Add the ON clause to the end of the WHERE clause, connected by
    ** an AND operator.
    */
    లేకపోతే ఒకవేళ( p_కుడి->u3.pOn ){
      sqlite3_సమితి_Join_సమాసం(p_కుడి->u3.pOn, p_కుడి->iCursor, joinType);
      p->pWhere = sqlite3_సమాసం_And(pParse, p->pWhere, p_కుడి->u3.pOn);
      p_కుడి->u3.pOn = 0;
      p_కుడి->fg.isOn = 1;
      p->selFlags |= SF_On_గమ్యం_Where;
    }

    ఒకవేళ( b_చూసిన_TFunc==0 ){
      b_చూసిన_TFunc = (p_ఎడమ->fg.isTabFunc && p_ఎడమ->u1.pFuncArg);
    }

    /* We also need to call sqlite3SelectCheckOnClauses() to verify that
    ** table-valued function arguments do not illegally refer to any tables to
    ** their right. This test is required if either (a) there is a RIGHT JOIN
    ** in the SrcList, or (b) there is a LEFT JOIN to the right of a 
    ** table-valued function.  */
    ఒకవేళ( (p_కుడి->fg.isTabFunc && joinType==EP_OuterON && p_కుడి->u1.pFuncArg)
     || (b_చూసిన_TFunc && joinType==EP_OuterON)
    ){
      p->selFlags |= SF_On_గమ్యం_Where;
    }
  }
  తిరిగి 0;
}

/*
** An instance of this object holds information (beyond pParse and pSelect)
** needed to load the next result row that is to be added to the sorter.
*/
రకనిర్వచనం నిర్మాణం అడ్డువరుస_ఎక్కించు_Info అడ్డువరుస_ఎక్కించు_Info;
నిర్మాణం అడ్డువరుస_ఎక్కించు_Info {
  పూర్ణాంకం reg_ఫలితం;               /* Store results in array of registers here */
  u8 ecelFlags;                /* Flag argument to ExprCodeExprList() */
#నిర్వచితమైతే SQLITE_ENABLE_SORTER_REFERENCES
  సమాసం_List *pExtra;            /* Extra columns needed by sorter refs */
  పూర్ణాంకం reg_Extra_ఫలితం;          /* Where to load the extra columns */
#అయితే_ముగింపు
};

/*
** This routine does the work of loading query data into an array of
** registers so that it can be added to the sorter.
*/
స్థిర శూన్య inner_Loop_ఎక్కించు_అడ్డువరుస(
  Parse *pParse,             /* Statement under construction */
  Select *pSelect,           /* The query being coded */
  అడ్డువరుస_ఎక్కించు_Info *pInfo         /* Info needed to complete the row load */
){
  sqlite3_సమాసం_సంకేతం_సమాసం_List(pParse, pSelect->pEList, pInfo->reg_ఫలితం,
                          0, pInfo->ecelFlags);
#నిర్వచితమైతే SQLITE_ENABLE_SORTER_REFERENCES
  ఒకవేళ( pInfo->pExtra ){
    sqlite3_సమాసం_సంకేతం_సమాసం_List(pParse, pInfo->pExtra, pInfo->reg_Extra_ఫలితం, 0, 0);
    sqlite3_సమాసం_List_Delete(pParse->db, pInfo->pExtra);
  }
#అయితే_ముగింపు
}

/*
** Code the OP_MakeRecord instruction that generates the entry to be
** added into the sorter.
**
** Return the register in which the result is stored.
*/
స్థిర పూర్ణాంకం make_Sorter_రికార్డు(
  Parse *pParse,
  క్రమీకరించు_Ctx *p_క్రమీకరించు,
  Select *pSelect,
  పూర్ణాంకం reg_పునాది,
  పూర్ణాంకం n_పునాది
){
  పూర్ణాంకం nOBSat = p_క్రమీకరించు->nOBSat;
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం reg_బహిర్గతం = ++pParse->nMem;
  ఒకవేళ( p_క్రమీకరించు->p_Deferred_అడ్డువరుస_ఎక్కించు ){
    inner_Loop_ఎక్కించు_అడ్డువరుస(pParse, pSelect, p_క్రమీకరించు->p_Deferred_అడ్డువరుస_ఎక్కించు);
  }
  sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, reg_పునాది+nOBSat, n_పునాది-nOBSat, reg_బహిర్గతం);
  తిరిగి reg_బహిర్గతం;
}

/*
** Generate code that will push the record in registers regData
** through regData+nData-1 onto the sorter.
*/
స్థిర శూన్య నెట్టు_Onto_Sorter(
  Parse *pParse,         /* Parser context */
  క్రమీకరించు_Ctx *p_క్రమీకరించు,        /* Information about the ORDER BY clause */
  Select *pSelect,       /* The whole SELECT statement */
  పూర్ణాంకం reg_దత్తాంశం,           /* First register holding data to be sorted */
  పూర్ణాంకం reg_Orig_దత్తాంశం,       /* First register holding data before packing */
  పూర్ణాంకం n_దత్తాంశం,             /* Number of elements in the regData data array */
  పూర్ణాంకం n_ఉపసర్గ_Reg         /* No. of reg prior to regData available for use */
){
  Vdbe *v = pParse->pVdbe;                         /* Stmt under construction */
  పూర్ణాంకం bSeq = ((p_క్రమీకరించు->క్రమీకరించు_Flags & SORTFLAG_UseSorter)==0);
  పూర్ణాంకం n_సమాసం = p_క్రమీకరించు->p_క్రమం_By->n_సమాసం;              /* No. of ORDER BY terms */
  పూర్ణాంకం n_పునాది = n_సమాసం + bSeq + n_దత్తాంశం;                /* Fields in sorter record */
  పూర్ణాంకం reg_పునాది;                                     /* Regs for sorter record */
  పూర్ణాంకం reg_రికార్డు = 0;                               /* Assembled sorter record */
  పూర్ణాంకం nOBSat = p_క్రమీకరించు->nOBSat;                      /* ORDER BY terms to skip */
  పూర్ణాంకం op;                            /* Opcode to add sorter record to sorter */
  పూర్ణాంకం i_హద్దు;                        /* LIMIT counter */
  పూర్ణాంకం iSkip = 0;                     /* End of the sorter insert loop */

  ప్రతిజ్ఞా( bSeq==0 || bSeq==1 );

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
  ప్రతిజ్ఞా( n_దత్తాంశం==1 || reg_దత్తాంశం==reg_Orig_దత్తాంశం || reg_Orig_దత్తాంశం==0 );

#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
  p_క్రమీకరించు->addr_నెట్టు = sqlite3VdbeCurrentAddr(v);
#అయితే_ముగింపు

  ఒకవేళ( n_ఉపసర్గ_Reg ){
    ప్రతిజ్ఞా( n_ఉపసర్గ_Reg==n_సమాసం+bSeq );
    reg_పునాది = reg_దత్తాంశం - n_ఉపసర్గ_Reg;
  }లేకపోతే{
    reg_పునాది = pParse->nMem + 1;
    pParse->nMem += n_పునాది;
  }
  ప్రతిజ్ఞా( pSelect->i_స్థానభ్రంశం==0 || pSelect->i_హద్దు!=0 );
  i_హద్దు = pSelect->i_స్థానభ్రంశం ? pSelect->i_స్థానభ్రంశం+1 : pSelect->i_హద్దు;
  p_క్రమీకరించు->label_పూర్తి = sqlite3VdbeMakeLabel(pParse);
  sqlite3_సమాసం_సంకేతం_సమాసం_List(pParse, p_క్రమీకరించు->p_క్రమం_By, reg_పునాది, reg_Orig_దత్తాంశం,
                          SQLITE_ECEL_DUP | (reg_Orig_దత్తాంశం? SQLITE_ECEL_REF : 0));
  ఒకవేళ( bSeq ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_Sequence, p_క్రమీకరించు->iECursor, reg_పునాది+n_సమాసం);
  }
  ఒకవేళ( n_ఉపసర్గ_Reg==0 && n_దత్తాంశం>0 ){
    sqlite3_సమాసం_సంకేతం_Move(pParse, reg_దత్తాంశం, reg_పునాది+n_సమాసం+bSeq, n_దత్తాంశం);
  }
  ఒకవేళ( nOBSat>0 ){
    పూర్ణాంకం reg_Prev_కీలకం;   /* The first nOBSat columns of the previous row */
    పూర్ణాంకం addr_మొదటి;    /* Address of the OP_IfNot opcode */
    పూర్ణాంకం addrJmp;      /* Address of the OP_Jump opcode */
    VdbeOp *pOp;      /* Opcode that opens the sorter */
    పూర్ణాంకం n_కీలకం;         /* Number of sorting key columns, including OP_Sequence */
    కీలకం_Info *pKI;     /* Original KeyInfo on the sorter table */

    reg_రికార్డు = make_Sorter_రికార్డు(pParse, p_క్రమీకరించు, pSelect, reg_పునాది, n_పునాది);
    reg_Prev_కీలకం = pParse->nMem+1;
    pParse->nMem += p_క్రమీకరించు->nOBSat;
    n_కీలకం = n_సమాసం - p_క్రమీకరించు->nOBSat + bSeq;
    ఒకవేళ( bSeq ){
      addr_మొదటి = sqlite3_Vdbe_కూడు_Op1(v, OP_IfNot, reg_పునాది+n_సమాసం);
    }లేకపోతే{
      addr_మొదటి = sqlite3_Vdbe_కూడు_Op1(v, OP_SequenceTest, p_క్రమీకరించు->iECursor);
    }
    VdbeCoverage(v);
    sqlite3_Vdbe_కూడు_Op3(v, OP_Compare, reg_Prev_కీలకం, reg_పునాది, p_క్రమీకరించు->nOBSat);
    pOp = sqlite3VdbeGetOp(v, p_క్రమీకరించు->addr_క్రమీకరించు_సూచీ);
    ఒకవేళ( pParse->db->mallocFailed ) తిరిగి;
    pOp->p2 = n_కీలకం + n_దత్తాంశం;
    pKI = pOp->p4.p_కీలకం_Info;
    memset(pKI->a_క్రమీకరించు_Flags, 0, pKI->n_కీలకం_క్షేత్రం); /* Makes OP_Jump testable */
    sqlite3VdbeChangeP4(v, -1, (అక్షరం*)pKI, P4_KEYINFO);
    testcase( pKI->n_All_క్షేత్రం > pKI->n_కీలకం_క్షేత్రం+2 );
    pOp->p4.p_కీలకం_Info = sqlite3_కీలకం_Info_From_సమాసం_List(pParse,p_క్రమీకరించు->p_క్రమం_By,nOBSat,
                                           pKI->n_All_క్షేత్రం-pKI->n_కీలకం_క్షేత్రం-1);
    pOp = 0; /* Ensure pOp not used after sqlite3VdbeAddOp3() */
    addrJmp = sqlite3VdbeCurrentAddr(v);
    sqlite3_Vdbe_కూడు_Op3(v, OP_Jump, addrJmp+1, 0, addrJmp+1); VdbeCoverage(v);
    p_క్రమీకరించు->label_Bk_బహిర్గతం = sqlite3VdbeMakeLabel(pParse);
    p_క్రమీకరించు->regReturn = ++pParse->nMem;
    sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, p_క్రమీకరించు->regReturn, p_క్రమీకరించు->label_Bk_బహిర్గతం);
    sqlite3_Vdbe_కూడు_Op1(v, OP_ResetSorter, p_క్రమీకరించు->iECursor);
    ఒకవేళ( i_హద్దు ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_IfNot, i_హద్దు, p_క్రమీకరించు->label_పూర్తి);
      VdbeCoverage(v);
    }
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addr_మొదటి);
    sqlite3_సమాసం_సంకేతం_Move(pParse, reg_పునాది, reg_Prev_కీలకం, p_క్రమీకరించు->nOBSat);
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addrJmp);
  }
  ఒకవేళ( i_హద్దు ){
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
    పూర్ణాంకం iCsr = p_క్రమీకరించు->iECursor;
    sqlite3_Vdbe_కూడు_Op2(v, OP_IfNotZero, i_హద్దు, sqlite3VdbeCurrentAddr(v)+4);
    VdbeCoverage(v);
    sqlite3_Vdbe_కూడు_Op2(v, OP_చివరి, iCsr, 0);
    iSkip = sqlite3_Vdbe_కూడు_Op4_Int(v, OP_IdxLE,
                                 iCsr, 0, reg_పునాది+nOBSat, n_సమాసం-nOBSat);
    VdbeCoverage(v);
    sqlite3_Vdbe_కూడు_Op1(v, OP_Delete, iCsr);
  }
  ఒకవేళ( reg_రికార్డు==0 ){
    reg_రికార్డు = make_Sorter_రికార్డు(pParse, p_క్రమీకరించు, pSelect, reg_పునాది, n_పునాది);
  }
  ఒకవేళ( p_క్రమీకరించు->క్రమీకరించు_Flags & SORTFLAG_UseSorter ){
    op = OP_Sorter_చొప్పించు;
  }లేకపోతే{
    op = OP_Idx_చొప్పించు;
  }
  sqlite3_Vdbe_కూడు_Op4_Int(v, op, p_క్రమీకరించు->iECursor, reg_రికార్డు,
                       reg_పునాది+nOBSat, n_పునాది-nOBSat);
  ఒకవేళ( iSkip ){
    sqlite3VdbeChangeP2(v, iSkip,
         p_క్రమీకరించు->labelOBLopt ? p_క్రమీకరించు->labelOBLopt : sqlite3VdbeCurrentAddr(v));
  }
#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
  p_క్రమీకరించు->addr_నెట్టు_చివర = sqlite3VdbeCurrentAddr(v)-1;
#అయితే_ముగింపు
}

/*
** Add code to implement the OFFSET
*/
స్థిర శూన్య సంకేతం_స్థానభ్రంశం(
  Vdbe *v,          /* Generate code into this VM */
  పూర్ణాంకం i_స్థానభ్రంశం,      /* Register holding the offset counter */
  పూర్ణాంకం iContinue     /* Jump here to skip the current record */
){
  ఒకవేళ( i_స్థానభ్రంశం>0 ){
    sqlite3_Vdbe_కూడు_Op3(v, OP_IfPos, i_స్థానభ్రంశం, iContinue, 1); VdbeCoverage(v);
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
స్థిర పూర్ణాంకం సంకేతం_Distinct(
  Parse *pParse,     /* Parsing and code generating context */
  పూర్ణాంకం eTnctType,     /* WHERE_DISTINCT_* value */
  పూర్ణాంకం iTab,          /* A sorting index used to test for distinctness */
  పూర్ణాంకం addrRepeat,    /* Jump to here if not distinct */
  సమాసం_List *pEList,  /* Expression for each element */
  పూర్ణాంకం regElem        /* First element */
){
  పూర్ణాంకం iRet = 0;
  పూర్ణాంకం n_ఫలితం_నిలువు = pEList->n_సమాసం;
  Vdbe *v = pParse->pVdbe;

  ఎంపిక( eTnctType ){
    స్థితి WHERE_DISTINCT_ORDERED: {
      పూర్ణాంకం i;
      పూర్ణాంకం iJump;              /* Jump destination */
      పూర్ణాంకం regPrev;            /* Previous row content */

      /* Allocate space for the previous row */
      iRet = regPrev = pParse->nMem+1;
      pParse->nMem += n_ఫలితం_నిలువు;

      iJump = sqlite3VdbeCurrentAddr(v) + n_ఫలితం_నిలువు;
      కొరకు(i=0; i<n_ఫలితం_నిలువు; i++){
        CollSeq *pColl = sqlite3_సమాసం_Coll_Seq(pParse, pEList->a[i].p_సమాసం);
        ఒకవేళ( i<n_ఫలితం_నిలువు-1 ){
          sqlite3_Vdbe_కూడు_Op3(v, OP_Ne, regElem+i, iJump, regPrev+i);
          VdbeCoverage(v);
        }లేకపోతే{
          sqlite3_Vdbe_కూడు_Op3(v, OP_Eq, regElem+i, addrRepeat, regPrev+i);
          VdbeCoverage(v);
         }
        sqlite3VdbeChangeP4(v, -1, (స్థిరరాశి అక్షరం *)pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
      }
      ప్రతిజ్ఞా( sqlite3VdbeCurrentAddr(v)==iJump || pParse->db->mallocFailed );
      sqlite3_Vdbe_కూడు_Op3(v, OP_Copy, regElem, regPrev, n_ఫలితం_నిలువు-1);
      ఆపు;
    }

    స్థితి WHERE_DISTINCT_UNIQUE: {
      /* nothing to do */
      ఆపు;
    }

    అప్రమేయ: {
      పూర్ణాంకం r1 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_దొరికింది, iTab, addrRepeat, regElem, n_ఫలితం_నిలువు);
      VdbeCoverage(v);
      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, regElem, n_ఫలితం_నిలువు, r1);
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, iTab, r1, regElem, n_ఫలితం_నిలువు);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      sqlite3_Release_తాత్కాలిక_Reg(pParse, r1);
      iRet = iTab;
      ఆపు;
    }
  }

  తిరిగి iRet;
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
స్థిర శూన్య fixDistinctOpenEph(
  Parse *pParse,     /* Parsing and code generating context */
  పూర్ణాంకం eTnctType,     /* WHERE_DISTINCT_* value */
  పూర్ణాంకం iVal,          /* Value returned by codeDistinct() */
  పూర్ణాంకం iOpenEphAddr   /* Address of OP_OpenEphemeral instruction for iTab */
){
  ఒకవేళ( pParse->nErr==0
   && (eTnctType==WHERE_DISTINCT_UNIQUE || eTnctType==WHERE_DISTINCT_ORDERED)
  ){
    Vdbe *v = pParse->pVdbe;
    sqlite3_Vdbe_Change_గమ్యం_Noop(v, iOpenEphAddr);
    ఒకవేళ( sqlite3VdbeGetOp(v, iOpenEphAddr+1)->opcode==OP_Explain ){
      sqlite3_Vdbe_Change_గమ్యం_Noop(v, iOpenEphAddr+1);
    }
    ఒకవేళ( eTnctType==WHERE_DISTINCT_ORDERED ){
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

#నిర్వచితమైతే SQLITE_ENABLE_SORTER_REFERENCES
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
స్థిర శూన్య select_సమాసం_Defer(
  Parse *pParse,                  /* Leave any error here */
  క్రమీకరించు_Ctx *p_క్రమీకరించు,                 /* Sorter context */
  సమాసం_List *pEList,               /* Expressions destined for sorter */
  సమాసం_List **ppExtra              /* Expressions to append to sorter record */
){
  పూర్ణాంకం i;
  పూర్ణాంకం nDefer = 0;
  సమాసం_List *pExtra = 0;
  కొరకు(i=0; i<pEList->n_సమాసం; i++){
    నిర్మాణం సమాసం_List_అంశం *p_అంశం = &pEList->a[i];
    ఒకవేళ( p_అంశం->u.x.i_క్రమం_By_నిలువు==0 ){
      సమాసం *p_సమాసం = p_అంశం->p_సమాసం;
      పట్టిక *pTab;
      ఒకవేళ( p_సమాసం->op==TK_స్తంభం
       && p_సమాసం->i_స్తంభం>=0
       && ALWAYS( సమాసం_Use_YTab(p_సమాసం) )
       && (pTab = p_సమాసం->y.pTab)!=0
       && Is_Ordinary_పట్టిక(pTab)
       && (pTab->a_నిలువు[p_సమాసం->i_స్తంభం].నిలువు_Flags & COLFLAG_SORTERREF)!=0
      ){
        పూర్ణాంకం j;
        కొరకు(j=0; j<nDefer; j++){
          ఒకవేళ( p_క్రమీకరించు->aDefer[j].iCsr==p_సమాసం->i_పట్టిక ) ఆపు;
        }
        ఒకవేళ( j==nDefer ){
          ఒకవేళ( nDefer==Array_కొలత(p_క్రమీకరించు->aDefer) ){
            కొనసాగు;
          }లేకపోతే{
            పూర్ణాంకం n_కీలకం = 1;
            పూర్ణాంకం k;
            సూచీ *pPk = 0;
            ఒకవేళ( !HasRowid(pTab) ){
              pPk = sqlite3_Primary_కీలకం_సూచీ(pTab);
              n_కీలకం = pPk->n_కీలకం_నిలువు;
            }
            కొరకు(k=0; k<n_కీలకం; k++){
              సమాసం *pNew = sqlite3PExpr(pParse, TK_స్తంభం, 0, 0);
              ఒకవేళ( pNew ){
                pNew->i_పట్టిక = p_సమాసం->i_పట్టిక;
                ప్రతిజ్ఞా( సమాసం_Use_YTab(pNew) );
                pNew->y.pTab = p_సమాసం->y.pTab;
                pNew->i_స్తంభం = pPk ? pPk->ai_స్తంభం[k] : -1;
                pExtra = sqlite3_సమాసం_List_Append(pParse, pExtra, pNew);
              }
            }
            p_క్రమీకరించు->aDefer[nDefer].pTab = p_సమాసం->y.pTab;
            p_క్రమీకరించు->aDefer[nDefer].iCsr = p_సమాసం->i_పట్టిక;
            p_క్రమీకరించు->aDefer[nDefer].n_కీలకం = n_కీలకం;
            nDefer++;
          }
        }
        p_అంశం->fg.bSorterRef = 1;
      }
    }
  }
  p_క్రమీకరించు->nDefer = (u8)nDefer;
  *ppExtra = pExtra;
}
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_DEBUG
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
స్థిర పూర్ణాంకం sqlite3CursorBloomable(Parse *pParse, పూర్ణాంకం iCur, పూర్ణాంకం n_నిలువు){
  పూర్ణాంకం i,k;
  Vdbe *v = pParse->pVdbe;
  ఒకవేళ( pParse->nErr ) తిరిగి 1;
  ప్రతిజ్ఞా( v );
  కొరకు(k=sqlite3VdbeCurrentAddr(v)-1; k>0; k--){
    స్థిరరాశి VdbeOp *pOp = sqlite3VdbeGetOp(v, k);
    స్థిరరాశి కీలకం_Info *p_కీలకం_Info;
    ఒకవేళ( pOp->p1!=iCur ) కొనసాగు;
    ఒకవేళ( pOp->opcode!=OP_OpenRead
     && pOp->opcode!=OP_OpenWrite
     && pOp->opcode!=OP_OpenEphemeral
    ){
      కొనసాగు;
    }
    ప్రతిజ్ఞా( pOp->p4type==P4_KEYINFO );
    p_కీలకం_Info = (స్థిరరాశి కీలకం_Info*)pOp->p4.p_కీలకం_Info;
    ప్రతిజ్ఞా( p_కీలకం_Info!=0 );
    కొరకు(i=0; i<n_నిలువు; i++){
      ప్రతిజ్ఞా( sqlite3_Is_ద్విమాన(p_కీలకం_Info->aColl[i]) );
    }
    తిరిగి 1;
  }
  తిరిగి 0;
}
#అయితే_ముగింపు /* SQLITE_DEBUG */


/*
** This routine generates the code for the inside of the inner loop
** of a SELECT.
**
** If srcTab is negative, then the p->pEList expressions
** are evaluated in order to get the data for this row.  If srcTab is
** zero or more, then data is pulled from srcTab and p->pEList is used only
** to get the number of columns and the collation sequence for each column.
*/
స్థిర శూన్య selectInnerLoop(
  Parse *pParse,          /* The parser context */
  Select *p,              /* The complete select statement being coded */
  పూర్ణాంకం srcTab,             /* Pull data from this table if non-negative */
  క్రమీకరించు_Ctx *p_క్రమీకరించు,         /* If not NULL, info on how to process ORDER BY */
  DistinctCtx *pDistinct, /* If not NULL, info on how to process DISTINCT */
  SelectDest *pDest,      /* How to dispose of the results */
  పూర్ణాంకం iContinue,          /* Jump here to continue with next row */
  పూర్ణాంకం iBreak              /* Jump here to break out of the inner loop */
){
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం i;
  పూర్ణాంకం hasDistinct;            /* True if the DISTINCT keyword is present */
  పూర్ణాంకం eDest = pDest->eDest;   /* How to dispose of results */
  పూర్ణాంకం iParm = pDest->iSDParm; /* First argument to disposal method */
  పూర్ణాంకం n_ఫలితం_నిలువు;             /* Number of result columns */
  పూర్ణాంకం n_ఉపసర్గ_Reg = 0;         /* Number of extra registers before regResult */
  అడ్డువరుస_ఎక్కించు_Info s_అడ్డువరుస_ఎక్కించు_Info;   /* Info for deferred row loading */

  /* Usually, regResult is the first cell in an array of memory cells
  ** containing the current result row. In this case regOrig is set to the
  ** same value. However, if the results are being sent to the sorter, the
  ** values for any expressions that are also part of the sort-key are omitted
  ** from this array. In this case regOrig is set to zero.  */
  పూర్ణాంకం reg_ఫలితం;              /* Start of memory holding current results */
  పూర్ణాంకం regOrig;                /* Start of memory holding full result (or 0) */

  ప్రతిజ్ఞా( v );
  ప్రతిజ్ఞా( p->pEList!=0 );
  hasDistinct = pDistinct ? pDistinct->eTnctType : WHERE_DISTINCT_NOOP;
  ఒకవేళ( p_క్రమీకరించు && p_క్రమీకరించు->p_క్రమం_By==0 ) p_క్రమీకరించు = 0;
  ఒకవేళ( p_క్రమీకరించు==0 && !hasDistinct ){
    ప్రతిజ్ఞా( iContinue!=0 );
    సంకేతం_స్థానభ్రంశం(v, p->i_స్థానభ్రంశం, iContinue);
  }

  /* Pull the requested columns.
  */
  n_ఫలితం_నిలువు = p->pEList->n_సమాసం;

  ఒకవేళ( pDest->iSdst==0 ){
    ఒకవేళ( p_క్రమీకరించు ){
      n_ఉపసర్గ_Reg = p_క్రమీకరించు->p_క్రమం_By->n_సమాసం;
      ఒకవేళ( !(p_క్రమీకరించు->క్రమీకరించు_Flags & SORTFLAG_UseSorter) ) n_ఉపసర్గ_Reg++;
      pParse->nMem += n_ఉపసర్గ_Reg;
    }
    pDest->iSdst = pParse->nMem+1;
    pParse->nMem += n_ఫలితం_నిలువు;
  }లేకపోతే ఒకవేళ( pDest->iSdst+n_ఫలితం_నిలువు > pParse->nMem ){
    /* This is an error condition that can result, for example, when a SELECT
    ** on the right-hand side of an INSERT contains more result columns than
    ** there are columns in the table on the left.  The error will be caught
    ** and reported later.  But we need to make sure enough memory is allocated
    ** to avoid other spurious errors in the meantime. */
    pParse->nMem += n_ఫలితం_నిలువు;
  }
  pDest->nSdst = n_ఫలితం_నిలువు;
  regOrig = reg_ఫలితం = pDest->iSdst;
  ఒకవేళ( srcTab>=0 ){
    కొరకు(i=0; i<n_ఫలితం_నిలువు; i++){
      sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, srcTab, i, reg_ఫలితం+i);
      VdbeComment((v, "%s", p->pEList->a[i].zEName));
    }
  }లేకపోతే ఒకవేళ( eDest!=SRT_Exists ){
#నిర్వచితమైతే SQLITE_ENABLE_SORTER_REFERENCES
    సమాసం_List *pExtra = 0;
#అయితే_ముగింపు
    /* If the destination is an EXISTS(...) expression, the actual
    ** values returned by the SELECT are not required.
    */
    u8 ecelFlags;    /* "ecel" is an abbreviation of "ExprCodeExprList" */
    సమాసం_List *pEList;
    ఒకవేళ( eDest==SRT_Mem || eDest==SRT_Output || eDest==SRT_Coroutine ){
      ecelFlags = SQLITE_ECEL_DUP;
    }లేకపోతే{
      ecelFlags = 0;
    }
    ఒకవేళ( p_క్రమీకరించు && hasDistinct==0 && eDest!=SRT_EphemTab && eDest!=SRT_పట్టిక ){
      /* For each expression in p->pEList that is a copy of an expression in
      ** the ORDER BY clause (pSort->pOrderBy), set the associated
      ** iOrderByCol value to one more than the index of the ORDER BY
      ** expression within the sort-key that pushOntoSorter() will generate.
      ** This allows the p->pEList field to be omitted from the sorted record,
      ** saving space and CPU cycles.  */
      ecelFlags |= (SQLITE_ECEL_OMITREF|SQLITE_ECEL_REF);

      కొరకు(i=p_క్రమీకరించు->nOBSat; i<p_క్రమీకరించు->p_క్రమం_By->n_సమాసం; i++){
        పూర్ణాంకం j;
        ఒకవేళ( (j = p_క్రమీకరించు->p_క్రమం_By->a[i].u.x.i_క్రమం_By_నిలువు)>0 ){
          p->pEList->a[j-1].u.x.i_క్రమం_By_నిలువు = i+1-p_క్రమీకరించు->nOBSat;
        }
      }
#నిర్వచితమైతే SQLITE_ENABLE_SORTER_REFERENCES
      select_సమాసం_Defer(pParse, p_క్రమీకరించు, p->pEList, &pExtra);
      ఒకవేళ( pExtra && pParse->db->mallocFailed==0 ){
        /* If there are any extra PK columns to add to the sorter records,
        ** allocate extra memory cells and adjust the OpenEphemeral
        ** instruction to account for the larger records. This is only
        ** required if there are one or more WITHOUT ROWID tables with
        ** composite primary keys in the SortCtx.aDefer[] array.  */
        VdbeOp *pOp = sqlite3VdbeGetOp(v, p_క్రమీకరించు->addr_క్రమీకరించు_సూచీ);
        pOp->p2 += (pExtra->n_సమాసం - p_క్రమీకరించు->nDefer);
        pOp->p4.p_కీలకం_Info->n_All_క్షేత్రం += (pExtra->n_సమాసం - p_క్రమీకరించు->nDefer);
        pParse->nMem += pExtra->n_సమాసం;
      }
#అయితే_ముగింపు

      /* Adjust nResultCol to account for columns that are omitted
      ** from the sorter by the optimizations in this branch */
      pEList = p->pEList;
      కొరకు(i=0; i<pEList->n_సమాసం; i++){
        ఒకవేళ( pEList->a[i].u.x.i_క్రమం_By_నిలువు>0
#నిర్వచితమైతే SQLITE_ENABLE_SORTER_REFERENCES
         || pEList->a[i].fg.bSorterRef
#అయితే_ముగింపు
        ){
          n_ఫలితం_నిలువు--;
          regOrig = 0;
        }
      }

      testcase( regOrig );
      testcase( eDest==SRT_సమితి );
      testcase( eDest==SRT_Mem );
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      ప్రతిజ్ఞా( eDest==SRT_సమితి || eDest==SRT_Mem
           || eDest==SRT_Coroutine || eDest==SRT_Output
           || eDest==SRT_Upfrom );
    }
    s_అడ్డువరుస_ఎక్కించు_Info.reg_ఫలితం = reg_ఫలితం;
    s_అడ్డువరుస_ఎక్కించు_Info.ecelFlags = ecelFlags;
#నిర్వచితమైతే SQLITE_ENABLE_SORTER_REFERENCES
    s_అడ్డువరుస_ఎక్కించు_Info.pExtra = pExtra;
    s_అడ్డువరుస_ఎక్కించు_Info.reg_Extra_ఫలితం = reg_ఫలితం + n_ఫలితం_నిలువు;
    ఒకవేళ( pExtra ) n_ఫలితం_నిలువు += pExtra->n_సమాసం;
#అయితే_ముగింపు
    ఒకవేళ( p->i_హద్దు
     && (ecelFlags & SQLITE_ECEL_OMITREF)!=0
     && n_ఉపసర్గ_Reg>0
    ){
      ప్రతిజ్ఞా( p_క్రమీకరించు!=0 );
      ప్రతిజ్ఞా( hasDistinct==0 );
      p_క్రమీకరించు->p_Deferred_అడ్డువరుస_ఎక్కించు = &s_అడ్డువరుస_ఎక్కించు_Info;
      regOrig = 0;
    }లేకపోతే{
      inner_Loop_ఎక్కించు_అడ్డువరుస(pParse, p, &s_అడ్డువరుస_ఎక్కించు_Info);
    }
  }

  /* If the DISTINCT keyword was present on the SELECT statement
  ** and this row has been seen before, then do not make this row
  ** part of the result.
  */
  ఒకవేళ( hasDistinct ){
    పూర్ణాంకం eType = pDistinct->eTnctType;
    పూర్ణాంకం iTab = pDistinct->tabTnct;
    ప్రతిజ్ఞా( n_ఫలితం_నిలువు==p->pEList->n_సమాసం );
    iTab = సంకేతం_Distinct(pParse, eType, iTab, iContinue, p->pEList, reg_ఫలితం);
    fixDistinctOpenEph(pParse, eType, iTab, pDistinct->addrTnct);
    ఒకవేళ( p_క్రమీకరించు==0 ){
      సంకేతం_స్థానభ్రంశం(v, p->i_స్థానభ్రంశం, iContinue);
    }
  }

  ఎంపిక( eDest ){
    /* Store the result as data using a unique key.
    */
    స్థితి SRT_Fifo:
    స్థితి SRT_DistFifo:
    స్థితి SRT_పట్టిక:
    స్థితి SRT_EphemTab: {
      పూర్ణాంకం r1 = sqlite3_Get_తాత్కాలిక_Range(pParse, n_ఉపసర్గ_Reg+1);
      testcase( eDest==SRT_పట్టిక );
      testcase( eDest==SRT_EphemTab );
      testcase( eDest==SRT_Fifo );
      testcase( eDest==SRT_DistFifo );
      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, reg_ఫలితం, n_ఫలితం_నిలువు, r1+n_ఉపసర్గ_Reg);
#ఒకవేళ !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      ఒకవేళ( eDest==SRT_పట్టిక && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_CTE
      ఒకవేళ( eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index. If the current row is already present
        ** in the index, do not write it to the output. If not, add the
        ** current row to the index and proceed with writing it to the
        ** output table as well.  */
        పూర్ణాంకం addr = sqlite3VdbeCurrentAddr(v) + 4;
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_దొరికింది, iParm+1, addr, r1, 0);
        VdbeCoverage(v);
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, iParm+1, r1,reg_ఫలితం,n_ఫలితం_నిలువు);
        ప్రతిజ్ఞా( p_క్రమీకరించు==0 );
      }
#అయితే_ముగింపు
      ఒకవేళ( p_క్రమీకరించు ){
        ప్రతిజ్ఞా( reg_ఫలితం==regOrig );
        నెట్టు_Onto_Sorter(pParse, p_క్రమీకరించు, p, r1+n_ఉపసర్గ_Reg, regOrig, 1, n_ఉపసర్గ_Reg);
      }లేకపోతే{
        పూర్ణాంకం r2 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
        sqlite3_Vdbe_కూడు_Op2(v, OP_NewRowid, iParm, r2);
        sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, iParm, r1, r2);
        sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
        sqlite3_Release_తాత్కాలిక_Reg(pParse, r2);
      }
      sqlite3_Release_తాత్కాలిక_Range(pParse, r1, n_ఉపసర్గ_Reg+1);
      ఆపు;
    }

    స్థితి SRT_Upfrom: {
      ఒకవేళ( p_క్రమీకరించు ){
        నెట్టు_Onto_Sorter(
            pParse, p_క్రమీకరించు, p, reg_ఫలితం, regOrig, n_ఫలితం_నిలువు, n_ఉపసర్గ_Reg);
      }లేకపోతే{
        పూర్ణాంకం i2 = pDest->iSDParm2;
        పూర్ణాంకం r1 = sqlite3_Get_తాత్కాలిక_Reg(pParse);

        /* If the UPDATE FROM join is an aggregate that matches no rows, it
        ** might still be trying to return one row, because that is what
        ** aggregates do.  Don't record that empty row in the output table. */
        sqlite3_Vdbe_కూడు_Op2(v, OP_IsNull, reg_ఫలితం, iBreak); VdbeCoverage(v);

        sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు,
                          reg_ఫలితం+(i2<0), n_ఫలితం_నిలువు-(i2<0), r1);
        ఒకవేళ( i2<0 ){
          sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, iParm, r1, reg_ఫలితం);
        }లేకపోతే{
          sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, iParm, r1, reg_ఫలితం, i2);
        }
      }
      ఆపు;
    }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)" construct,
    ** then there should be a single item on the stack.  Write this
    ** item into the set table with bogus data.
    */
    స్థితి SRT_సమితి: {
      ఒకవేళ( p_క్రమీకరించు ){
        /* At first glance you would think we could optimize out the
        ** ORDER BY in this case since the order of entries in the set
        ** does not matter.  But there might be a LIMIT clause, in which
        ** case the order does matter */
        నెట్టు_Onto_Sorter(
            pParse, p_క్రమీకరించు, p, reg_ఫలితం, regOrig, n_ఫలితం_నిలువు, n_ఉపసర్గ_Reg);
        pDest->iSDParm2 = 0; /* Signal that any Bloom filter is unpopulated */
      }లేకపోతే{
        పూర్ణాంకం r1 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
        ప్రతిజ్ఞా( sqlite3Strlen30(pDest->zAffSdst)==n_ఫలితం_నిలువు );
        sqlite3_Vdbe_కూడు_Op4(v, OP_Make_రికార్డు, reg_ఫలితం, n_ఫలితం_నిలువు,
            r1, pDest->zAffSdst, n_ఫలితం_నిలువు);
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, iParm, r1, reg_ఫలితం, n_ఫలితం_నిలువు);
        ఒకవేళ( pDest->iSDParm2 ){
          ప్రతిజ్ఞా( sqlite3CursorBloomable(pParse,iParm,n_ఫలితం_నిలువు) );
          sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Filter_కూడు, pDest->iSDParm2, 0,
                               reg_ఫలితం, n_ఫలితం_నిలువు);
          ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
        }
        sqlite3_Release_తాత్కాలిక_Reg(pParse, r1);
      }
      ఆపు;
    }


    /* If any row exist in the result set, record that fact and abort.
    */
    స్థితి SRT_Exists: {
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 1, iParm);
      /* The LIMIT clause will terminate the loop for us */
      ఆపు;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell or array of
    ** memory cells and break out of the scan loop.
    */
    స్థితి SRT_Mem: {
      ఒకవేళ( p_క్రమీకరించు ){
        ప్రతిజ్ఞా( n_ఫలితం_నిలువు<=pDest->nSdst );
        నెట్టు_Onto_Sorter(
            pParse, p_క్రమీకరించు, p, reg_ఫలితం, regOrig, n_ఫలితం_నిలువు, n_ఉపసర్గ_Reg);
        pDest->iSDParm = reg_ఫలితం;
      }లేకపోతే{
        ప్రతిజ్ఞా( n_ఫలితం_నిలువు==pDest->nSdst );
        ఒకవేళ( reg_ఫలితం!=iParm ){
          /* This occurs in cases where the SELECT had both a DISTINCT and
          ** an OFFSET clause.  */
          sqlite3_Vdbe_కూడు_Op3(v, OP_Copy, reg_ఫలితం, iParm, n_ఫలితం_నిలువు-1);
        }
        /* The LIMIT clause will jump out of the loop for us */
      }
      ఆపు;
    }
#అయితే_ముగింపు /* #ifndef SQLITE_OMIT_SUBQUERY */

    స్థితి SRT_Coroutine:       /* Send data to a co-routine */
    స్థితి SRT_Output: {        /* Return the results */
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      ఒకవేళ( p_క్రమీకరించు ){
        నెట్టు_Onto_Sorter(pParse, p_క్రమీకరించు, p, reg_ఫలితం, regOrig, n_ఫలితం_నిలువు,
                       n_ఉపసర్గ_Reg);
      }లేకపోతే ఒకవేళ( eDest==SRT_Coroutine ){
        sqlite3_Vdbe_కూడు_Op1(v, OP_Yield, pDest->iSDParm);
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, reg_ఫలితం, n_ఫలితం_నిలువు);
      }
      ఆపు;
    }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    స్థితి SRT_Dist_వరుస:
    స్థితి SRT_వరుస: {
      పూర్ణాంకం n_కీలకం;
      పూర్ణాంకం r1, r2, r3;
      పూర్ణాంకం addrTest = 0;
      సమాసం_List *pSO;
      pSO = pDest->p_క్రమం_By;
      ప్రతిజ్ఞా( pSO );
      n_కీలకం = pSO->n_సమాసం;
      r1 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
      r2 = sqlite3_Get_తాత్కాలిక_Range(pParse, n_కీలకం+2);
      r3 = r2+n_కీలకం+1;
      ఒకవేళ( eDest==SRT_Dist_వరుస ){
        /* If the destination is DistQueue, then cursor (iParm+1) is open
        ** on a second ephemeral index that holds all values every previously
        ** added to the queue. */
        addrTest = sqlite3_Vdbe_కూడు_Op4_Int(v, OP_దొరికింది, iParm+1, 0,
                                        reg_ఫలితం, n_ఫలితం_నిలువు);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, reg_ఫలితం, n_ఫలితం_నిలువు, r3);
      ఒకవేళ( eDest==SRT_Dist_వరుస ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_Idx_చొప్పించు, iParm+1, r3);
        sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      }
      కొరకు(i=0; i<n_కీలకం; i++){
        sqlite3_Vdbe_కూడు_Op2(v, OP_SCopy,
                          reg_ఫలితం + pSO->a[i].u.x.i_క్రమం_By_నిలువు - 1,
                          r2+i);
      }
      sqlite3_Vdbe_కూడు_Op2(v, OP_Sequence, iParm, r2+n_కీలకం);
      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, r2, n_కీలకం+2, r1);
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, iParm, r1, r2, n_కీలకం+2);
      ఒకవేళ( addrTest ) sqlite3_Vdbe_Jump_ఇక్కడ(v, addrTest);
      sqlite3_Release_తాత్కాలిక_Reg(pParse, r1);
      sqlite3_Release_తాత్కాలిక_Range(pParse, r2, n_కీలకం+2);
      ఆపు;
    }
#అయితే_ముగింపు /* SQLITE_OMIT_CTE */



#ఒకవేళ !defined(SQLITE_OMIT_TRIGGER)
    /* Discard the results.  This is used for SELECT statements inside
    ** the body of a TRIGGER.  The purpose of such selects is to call
    ** user-defined functions that have side effects.  We do not care
    ** about the actual results of the select.
    */
    అప్రమేయ: {
      ప్రతిజ్ఞా( eDest==SRT_Discard );
      ఆపు;
    }
#అయితే_ముగింపు
  }

  /* Jump to the end of the loop if the LIMIT is reached.  Except, if
  ** there is a sorter, in which case the sorter has already limited
  ** the output for us.
  */
  ఒకవేళ( p_క్రమీకరించు==0 && p->i_హద్దు ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_DecrJumpZero, p->i_హద్దు, iBreak); VdbeCoverage(v);
  }
}

/*
** Allocate a KeyInfo object sufficient for an index of N key columns and
** X extra columns.
*/
కీలకం_Info *sqlite3_కీలకం_Info_Alloc(sqlite3 *db, పూర్ణాంకం N, పూర్ణాంకం X){
  పూర్ణాంకం nExtra = (N+X)*(పరిమాణం(CollSeq*)+1);
  కీలకం_Info *p;
  ప్రతిజ్ఞా( X>=0 );
  ఒకవేళ( NEVER(N+X>0xffff) ) తిరిగి (కీలకం_Info*)sqlite3OomFault(db);
  p = sqlite3DbMallocRawNN(db, SZ_KEYINFO(0) + nExtra);
  ఒకవేళ( p ){
    p->a_క్రమీకరించు_Flags = (u8*)&p->aColl[N+X];
    p->n_కీలకం_క్షేత్రం = (u16)N;
    p->n_All_క్షేత్రం = (u16)(N+X);
    p->enc = ENC(db);
    p->db = db;
    p->nRef = 1;
    memset(p->aColl, 0, nExtra);
  }లేకపోతే{
    తిరిగి (కీలకం_Info*)sqlite3OomFault(db);
  }
  తిరిగి p;
}

/*
** Deallocate a KeyInfo object
*/
శూన్య sqlite3_కీలకం_Info_Unref(కీలకం_Info *p){
  ఒకవేళ( p ){
    ప్రతిజ్ఞా( p->db!=0 );
    ప్రతిజ్ఞా( p->nRef>0 );
    p->nRef--;
    ఒకవేళ( p->nRef==0 ) sqlite3DbNNFreeNN(p->db, p);
  }
}

/*
** Make a new pointer to a KeyInfo object
*/
కీలకం_Info *sqlite3_కీలకం_Info_Ref(కీలకం_Info *p){
  ఒకవేళ( p ){
    ప్రతిజ్ఞా( p->nRef>0 );
    p->nRef++;
  }
  తిరిగి p;
}

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Return TRUE if a KeyInfo object can be change.  The KeyInfo object
** can only be changed if this is just a single reference to the object.
**
** This routine is used only inside of assert() statements.
*/
పూర్ణాంకం sqlite3_కీలకం_Info_Is_Writeable(కీలకం_Info *p){ తిరిగి p->nRef==1; }
#అయితే_ముగింపు /* SQLITE_DEBUG */

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
కీలకం_Info *sqlite3_కీలకం_Info_From_సమాసం_List(
  Parse *pParse,       /* Parsing context */
  సమాసం_List *pList,     /* Form the KeyInfo object from this ExprList */
  పూర్ణాంకం i_ప్రారంభం,          /* Begin with this column of pList */
  పూర్ణాంకం nExtra           /* Add this many extra columns to the end */
){
  పూర్ణాంకం n_సమాసం;
  కీలకం_Info *pInfo;
  నిర్మాణం సమాసం_List_అంశం *p_అంశం;
  sqlite3 *db = pParse->db;
  పూర్ణాంకం i;

  n_సమాసం = pList->n_సమాసం;
  pInfo = sqlite3_కీలకం_Info_Alloc(db, n_సమాసం-i_ప్రారంభం, nExtra+1);
  ఒకవేళ( pInfo ){
    ప్రతిజ్ఞా( sqlite3_కీలకం_Info_Is_Writeable(pInfo) );
    కొరకు(i=i_ప్రారంభం, p_అంశం=pList->a+i_ప్రారంభం; i<n_సమాసం; i++, p_అంశం++){
      pInfo->aColl[i-i_ప్రారంభం] = sqlite3_సమాసం_NNColl_Seq(pParse, p_అంశం->p_సమాసం);
      pInfo->a_క్రమీకరించు_Flags[i-i_ప్రారంభం] = p_అంశం->fg.క్రమీకరించు_Flags;
    }
  }
  తిరిగి pInfo;
}

/*
** Name of the connection operator, used for error messages.
*/
స్థిరరాశి అక్షరం *sqlite3SelectOpName(పూర్ణాంకం id){
  అక్షరం *z;
  ఎంపిక( id ){
    స్థితి TK_ALL:       z = "UNION ALL";   ఆపు;
    స్థితి TK_INTERSECT: z = "INTERSECT";   ఆపు;
    స్థితి TK_EXCEPT:    z = "EXCEPT";      ఆపు;
    అప్రమేయ:           z = "UNION";       ఆపు;
  }
  తిరిగి z;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_EXPLAIN
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
స్థిర శూన్య explain_తాత్కాలిక_పట్టిక(Parse *pParse, స్థిరరాశి అక్షరం *zUsage){
  ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s", zUsage));
}

/*
** Assign expression b to lvalue a. A second, no-op, version of this macro
** is provided when SQLITE_OMIT_EXPLAIN is defined. This allows the code
** in sqlite3Select() to assign values to structure member variables that
** only exist if SQLITE_OMIT_EXPLAIN is not defined without polluting the
** code with #ifndef directives.
*/
# define explain_సమితి_Integer(a, b) a = b

#లేకపోతే
/* No-op versions of the explainXXX() functions and macros. */
# define explain_తాత్కాలిక_పట్టిక(y,z)
# define explain_సమితి_Integer(y,z)
#అయితే_ముగింపు


/*
** If the inner loop was generated using a non-null pOrderBy argument,
** then the results were placed in a sorter.  After the loop is terminated
** we need to run the sorter and output the results.  The following
** routine generates the code needed to do that.
*/
స్థిర శూన్య generate_క్రమీకరించు_తోక(
  Parse *pParse,    /* Parsing context */
  Select *p,        /* The SELECT statement */
  క్రమీకరించు_Ctx *p_క్రమీకరించు,   /* Information on the ORDER BY clause */
  పూర్ణాంకం n_స్తంభం,      /* Number of columns of data */
  SelectDest *pDest /* Write the sorted results here */
){
  Vdbe *v = pParse->pVdbe;                     /* The prepared statement */
  పూర్ణాంకం addrBreak = p_క్రమీకరించు->label_పూర్తి;            /* Jump here to exit loop */
  పూర్ణాంకం addrContinue = sqlite3VdbeMakeLabel(pParse);/* Jump here for next cycle */
  పూర్ణాంకం addr;                       /* Top of output loop. Jump for Next. */
  పూర్ణాంకం addrOnce = 0;
  పూర్ణాంకం iTab;
  సమాసం_List *p_క్రమం_By = p_క్రమీకరించు->p_క్రమం_By;
  పూర్ణాంకం eDest = pDest->eDest;
  పూర్ణాంకం iParm = pDest->iSDParm;
  పూర్ణాంకం reg_అడ్డువరుస;
  పూర్ణాంకం regRowid;
  పూర్ణాంకం i_నిలువు;
  పూర్ణాంకం n_కీలకం;                       /* Number of key columns in sorter record */
  పూర్ణాంకం i_క్రమీకరించు_Tab;                   /* Sorter cursor to read from */
  పూర్ణాంకం i;
  పూర్ణాంకం bSeq;                       /* True if sorter record includes seq. no. */
  పూర్ణాంకం n_Ref_కీలకం = 0;
  నిర్మాణం సమాసం_List_అంశం *a_బహిర్గతం_Ex = p->pEList->a;
#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
  పూర్ణాంకం addrExplain;                /* Address of OP_Explain instruction */
#అయితే_ముగింపు

  n_కీలకం = p_క్రమం_By->n_సమాసం - p_క్రమీకరించు->nOBSat;
  ఒకవేళ( p_క్రమీకరించు->nOBSat==0 || n_కీలకం==1 ){
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR %sORDER BY", p_క్రమీకరించు->nOBSat?"LAST TERM OF ":""
    ));
  }లేకపోతే{
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR LAST %d TERMS OF ORDER BY", n_కీలకం
    ));
  }
  sqlite3_Vdbe_పరిశీలన_స్థితి_Range(v, addrExplain,p_క్రమీకరించు->addr_నెట్టు,p_క్రమీకరించు->addr_నెట్టు_చివర);
  sqlite3_Vdbe_పరిశీలన_స్థితి_Counters(v, addrExplain, addrExplain, p_క్రమీకరించు->addr_నెట్టు);


  ప్రతిజ్ఞా( addrBreak<0 );
  ఒకవేళ( p_క్రమీకరించు->label_Bk_బహిర్గతం ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, p_క్రమీకరించు->regReturn, p_క్రమీకరించు->label_Bk_బహిర్గతం);
    sqlite3VdbeGoto(v, addrBreak);
    sqlite3VdbeResolveLabel(v, p_క్రమీకరించు->label_Bk_బహిర్గతం);
  }

#నిర్వచితమైతే SQLITE_ENABLE_SORTER_REFERENCES
  /* Open any cursors needed for sorter-reference expressions */
  కొరకు(i=0; i<p_క్రమీకరించు->nDefer; i++){
    పట్టిక *pTab = p_క్రమీకరించు->aDefer[i].pTab;
    పూర్ణాంకం iDb = sqlite3_Schema_గమ్యం_సూచీ(pParse->db, pTab->pSchema);
    sqlite3_Open_పట్టిక(pParse, p_క్రమీకరించు->aDefer[i].iCsr, iDb, pTab, OP_OpenRead);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Rewind, p_క్రమీకరించు->aDefer[i].iCsr,
                      sqlite3VdbeCurrentAddr(v)+1);
    VdbeCoverage(v);
    n_Ref_కీలకం = MAX(n_Ref_కీలకం, p_క్రమీకరించు->aDefer[i].n_కీలకం);
  }
#అయితే_ముగింపు

  iTab = p_క్రమీకరించు->iECursor;
  ఒకవేళ( eDest==SRT_Output || eDest==SRT_Coroutine || eDest==SRT_Mem ){
    ఒకవేళ( eDest==SRT_Mem && p->i_స్థానభ్రంశం ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_Null, 0, pDest->iSdst);
    }
    regRowid = 0;
    reg_అడ్డువరుస = pDest->iSdst;
  }లేకపోతే{
    regRowid = sqlite3_Get_తాత్కాలిక_Reg(pParse);
    ఒకవేళ( eDest==SRT_EphemTab || eDest==SRT_పట్టిక ){
      reg_అడ్డువరుస = sqlite3_Get_తాత్కాలిక_Reg(pParse);
      n_స్తంభం = 0;
    }లేకపోతే{
      reg_అడ్డువరుస = sqlite3_Get_తాత్కాలిక_Range(pParse, n_స్తంభం);
    }
  }
  ఒకవేళ( p_క్రమీకరించు->క్రమీకరించు_Flags & SORTFLAG_UseSorter ){
    పూర్ణాంకం reg_క్రమీకరించు_బహిర్గతం = ++pParse->nMem;
    i_క్రమీకరించు_Tab = pParse->nTab++;
    ఒకవేళ( p_క్రమీకరించు->label_Bk_బహిర్గతం ){
      addrOnce = sqlite3_Vdbe_కూడు_Op0(v, OP_Once); VdbeCoverage(v);
    }
    sqlite3_Vdbe_కూడు_Op3(v, OP_OpenPseudo, i_క్రమీకరించు_Tab, reg_క్రమీకరించు_బహిర్గతం,
        n_కీలకం+1+n_స్తంభం+n_Ref_కీలకం);
    ఒకవేళ( addrOnce ) sqlite3_Vdbe_Jump_ఇక్కడ(v, addrOnce);
    addr = 1 + sqlite3_Vdbe_కూడు_Op2(v, OP_Sorter_క్రమీకరించు, iTab, addrBreak);
    VdbeCoverage(v);
    ప్రతిజ్ఞా( p->i_హద్దు==0 && p->i_స్థానభ్రంశం==0 );
    sqlite3_Vdbe_కూడు_Op3(v, OP_Sorter_దత్తాంశం, iTab, reg_క్రమీకరించు_బహిర్గతం, i_క్రమీకరించు_Tab);
    bSeq = 0;
  }లేకపోతే{
    addr = 1 + sqlite3_Vdbe_కూడు_Op2(v, OP_క్రమీకరించు, iTab, addrBreak); VdbeCoverage(v);
    సంకేతం_స్థానభ్రంశం(v, p->i_స్థానభ్రంశం, addrContinue);
    i_క్రమీకరించు_Tab = iTab;
    bSeq = 1;
    ఒకవేళ( p->i_స్థానభ్రంశం>0 ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_కూడు_Imm, p->i_హద్దు, -1);
    }
  }
  కొరకు(i=0, i_నిలువు=n_కీలకం+bSeq-1; i<n_స్తంభం; i++){
#నిర్వచితమైతే SQLITE_ENABLE_SORTER_REFERENCES
    ఒకవేళ( a_బహిర్గతం_Ex[i].fg.bSorterRef ) కొనసాగు;
#అయితే_ముగింపు
    ఒకవేళ( a_బహిర్గతం_Ex[i].u.x.i_క్రమం_By_నిలువు==0 ) i_నిలువు++;
  }
#నిర్వచితమైతే SQLITE_ENABLE_SORTER_REFERENCES
  ఒకవేళ( p_క్రమీకరించు->nDefer ){
    పూర్ణాంకం i_కీలకం = i_నిలువు+1;
    పూర్ణాంకం reg_కీలకం = sqlite3_Get_తాత్కాలిక_Range(pParse, n_Ref_కీలకం);

    కొరకు(i=0; i<p_క్రమీకరించు->nDefer; i++){
      పూర్ణాంకం iCsr = p_క్రమీకరించు->aDefer[i].iCsr;
      పట్టిక *pTab = p_క్రమీకరించు->aDefer[i].pTab;
      పూర్ణాంకం n_కీలకం = p_క్రమీకరించు->aDefer[i].n_కీలకం;

      sqlite3_Vdbe_కూడు_Op1(v, OP_Null_అడ్డువరుస, iCsr);
      ఒకవేళ( HasRowid(pTab) ){
        sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, i_క్రమీకరించు_Tab, i_కీలకం++, reg_కీలకం);
        sqlite3_Vdbe_కూడు_Op3(v, OP_SeekRowid, iCsr,
            sqlite3VdbeCurrentAddr(v)+1, reg_కీలకం);
      }లేకపోతే{
        పూర్ణాంకం k;
        పూర్ణాంకం iJmp;
        ప్రతిజ్ఞా( sqlite3_Primary_కీలకం_సూచీ(pTab)->n_కీలకం_నిలువు==n_కీలకం );
        కొరకు(k=0; k<n_కీలకం; k++){
          sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, i_క్రమీకరించు_Tab, i_కీలకం++, reg_కీలకం+k);
        }
        iJmp = sqlite3VdbeCurrentAddr(v);
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_SeekGE, iCsr, iJmp+2, reg_కీలకం, n_కీలకం);
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_IdxLE, iCsr, iJmp+3, reg_కీలకం, n_కీలకం);
        sqlite3_Vdbe_కూడు_Op1(v, OP_Null_అడ్డువరుస, iCsr);
      }
    }
    sqlite3_Release_తాత్కాలిక_Range(pParse, reg_కీలకం, n_Ref_కీలకం);
  }
#అయితే_ముగింపు
  కొరకు(i=n_స్తంభం-1; i>=0; i--){
#నిర్వచితమైతే SQLITE_ENABLE_SORTER_REFERENCES
    ఒకవేళ( a_బహిర్గతం_Ex[i].fg.bSorterRef ){
      sqlite3_సమాసం_సంకేతం(pParse, a_బహిర్గతం_Ex[i].p_సమాసం, reg_అడ్డువరుస+i);
    }లేకపోతే
#అయితే_ముగింపు
    {
      పూర్ణాంకం iRead;
      ఒకవేళ( a_బహిర్గతం_Ex[i].u.x.i_క్రమం_By_నిలువు ){
        iRead = a_బహిర్గతం_Ex[i].u.x.i_క్రమం_By_నిలువు-1;
      }లేకపోతే{
        iRead = i_నిలువు--;
      }
      sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, i_క్రమీకరించు_Tab, iRead, reg_అడ్డువరుస+i);
      VdbeComment((v, "%s", a_బహిర్గతం_Ex[i].zEName));
    }
  }
  sqlite3_Vdbe_పరిశీలన_స్థితి_Range(v, addrExplain, addrExplain, -1);
  ఎంపిక( eDest ){
    స్థితి SRT_పట్టిక:
    స్థితి SRT_EphemTab: {
      sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, i_క్రమీకరించు_Tab, n_కీలకం+bSeq, reg_అడ్డువరుస);
      sqlite3_Vdbe_కూడు_Op2(v, OP_NewRowid, iParm, regRowid);
      sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, iParm, reg_అడ్డువరుస, regRowid);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      ఆపు;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
    స్థితి SRT_సమితి: {
      ప్రతిజ్ఞా( n_స్తంభం==sqlite3Strlen30(pDest->zAffSdst) );
      sqlite3_Vdbe_కూడు_Op4(v, OP_Make_రికార్డు, reg_అడ్డువరుస, n_స్తంభం, regRowid,
                        pDest->zAffSdst, n_స్తంభం);
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, iParm, regRowid, reg_అడ్డువరుస, n_స్తంభం);
      ఆపు;
    }
    స్థితి SRT_Mem: {
      /* The LIMIT clause will terminate the loop for us */
      ఆపు;
    }
#అయితే_ముగింపు
    స్థితి SRT_Upfrom: {
      పూర్ణాంకం i2 = pDest->iSDParm2;
      పూర్ణాంకం r1 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు,reg_అడ్డువరుస+(i2<0),n_స్తంభం-(i2<0),r1);
      ఒకవేళ( i2<0 ){
        sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, iParm, r1, reg_అడ్డువరుస);
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, iParm, r1, reg_అడ్డువరుస, i2);
      }
      ఆపు;
    }
    అప్రమేయ: {
      ప్రతిజ్ఞా( eDest==SRT_Output || eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      testcase( eDest==SRT_Coroutine );
      ఒకవేళ( eDest==SRT_Output ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, pDest->iSdst, n_స్తంభం);
      }లేకపోతే{
        sqlite3_Vdbe_కూడు_Op1(v, OP_Yield, pDest->iSDParm);
      }
      ఆపు;
    }
  }
  ఒకవేళ( regRowid ){
    ఒకవేళ( eDest==SRT_సమితి ){
      sqlite3_Release_తాత్కాలిక_Range(pParse, reg_అడ్డువరుస, n_స్తంభం);
    }లేకపోతే{
      sqlite3_Release_తాత్కాలిక_Reg(pParse, reg_అడ్డువరుస);
    }
    sqlite3_Release_తాత్కాలిక_Reg(pParse, regRowid);
  }
  /* The bottom of the loop
  */
  sqlite3VdbeResolveLabel(v, addrContinue);
  ఒకవేళ( p_క్రమీకరించు->క్రమీకరించు_Flags & SORTFLAG_UseSorter ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_Sorter_తదుపరి, iTab, addr); VdbeCoverage(v);
  }లేకపోతే{
    sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, iTab, addr); VdbeCoverage(v);
  }
  sqlite3_Vdbe_పరిశీలన_స్థితి_Range(v, addrExplain, sqlite3VdbeCurrentAddr(v)-1, -1);
  ఒకవేళ( p_క్రమీకరించు->regReturn ) sqlite3_Vdbe_కూడు_Op1(v, OP_Return, p_క్రమీకరించు->regReturn);
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
#నిర్వచితమైతే SQLITE_ENABLE_స్తంభం_METADATA
# define స్తంభం_Type(A,B,C,D,E) స్తంభం_Type_Impl(A,B,C,D,E)
#లేకపోతే /* if !defined(SQLITE_ENABLE_COLUMN_METADATA) */
# define స్తంభం_Type(A,B,C,D,E) స్తంభం_Type_Impl(A,B)
#అయితే_ముగింపు
స్థిర స్థిరరాశి అక్షరం *స్తంభం_Type_Impl(
  NameContext *pNC,
#నిర్వచితం_కాకపోతే SQLITE_ENABLE_స్తంభం_METADATA
  సమాసం *p_సమాసం
#లేకపోతే
  సమాసం *p_సమాసం,
  స్థిరరాశి అక్షరం **pzOrigDb,
  స్థిరరాశి అక్షరం **pzOrigTab,
  స్థిరరాశి అక్షరం **pz_Orig_నిలువు
#అయితే_ముగింపు
){
  అక్షరం స్థిరరాశి *zType = 0;
  పూర్ణాంకం j;
#నిర్వచితమైతే SQLITE_ENABLE_స్తంభం_METADATA
  అక్షరం స్థిరరాశి *zOrigDb = 0;
  అక్షరం స్థిరరాశి *zOrigTab = 0;
  అక్షరం స్థిరరాశి *z_Orig_నిలువు = 0;
#అయితే_ముగింపు

  ప్రతిజ్ఞా( p_సమాసం!=0 );
  ప్రతిజ్ఞా( pNC->pSrcList!=0 );
  ఎంపిక( p_సమాసం->op ){
    స్థితి TK_స్తంభం: {
      /* The expression is a column. Locate the table the column is being
      ** extracted from in NameContext.pSrcList. This table may be real
      ** database table or a subquery.
      */
      పట్టిక *pTab = 0;            /* Table structure column is extracted from */
      Select *pS = 0;             /* Select the column is extracted from */
      పూర్ణాంకం i_నిలువు = p_సమాసం->i_స్తంభం;  /* Index of column in pTab */
      అయితే( pNC && !pTab ){
        SrcList *pTabList = pNC->pSrcList;
        కొరకు(j=0;j<pTabList->nSrc && pTabList->a[j].iCursor!=p_సమాసం->i_పట్టిక;j++);
        ఒకవేళ( j<pTabList->nSrc ){
          pTab = pTabList->a[j].pSTab;
          ఒకవేళ( pTabList->a[j].fg.isSubquery ){
            pS = pTabList->a[j].u4.pSubq->pSelect;
          }లేకపోతే{
            pS = 0;
          }
        }లేకపోతే{
          pNC = pNC->p_తదుపరి;
        }
      }

      ఒకవేళ( pTab==0 ){
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
        ఆపు;
      }

      ప్రతిజ్ఞా( pTab && సమాసం_Use_YTab(p_సమాసం) && p_సమాసం->y.pTab==pTab );
      ఒకవేళ( pS ){
        /* The "table" is actually a sub-select or a view in the FROM clause
        ** of the SELECT statement. Return the declaration type and origin
        ** data for the result-set column of the sub-select.
        */
        ఒకవేళ( i_నిలువు<pS->pEList->n_సమాసం
         && (!ViewCanHaveRowid || i_నిలువు>=0)
        ){
          /* If iCol is less than zero, then the expression requests the
          ** rowid of the sub-select or view. This expression is legal (see
          ** test case misc2.2.2) - it always evaluates to NULL.
          */
          NameContext sNC;
          సమాసం *p = pS->pEList->a[i_నిలువు].p_సమాసం;
          sNC.pSrcList = pS->pSrc;
          sNC.p_తదుపరి = pNC;
          sNC.pParse = pNC->pParse;
          zType = స్తంభం_Type(&sNC, p,&zOrigDb,&zOrigTab,&z_Orig_నిలువు);
        }
      }లేకపోతే{
        /* A real table or a CTE table */
        ప్రతిజ్ఞా( !pS );
#నిర్వచితమైతే SQLITE_ENABLE_స్తంభం_METADATA
        ఒకవేళ( i_నిలువు<0 ) i_నిలువు = pTab->iPKey;
        ప్రతిజ్ఞా( i_నిలువు==XN_ROWID || (i_నిలువు>=0 && i_నిలువు<pTab->n_నిలువు) );
        ఒకవేళ( i_నిలువు<0 ){
          zType = "INTEGER";
          z_Orig_నిలువు = "rowid";
        }లేకపోతే{
          z_Orig_నిలువు = pTab->a_నిలువు[i_నిలువు].zCnName;
          zType = sqlite3_స్తంభం_Type(&pTab->a_నిలువు[i_నిలువు],0);
        }
        zOrigTab = pTab->zName;
        ఒకవేళ( pNC->pParse && pTab->pSchema ){
          పూర్ణాంకం iDb = sqlite3_Schema_గమ్యం_సూచీ(pNC->pParse->db, pTab->pSchema);
          zOrigDb = pNC->pParse->db->aDb[iDb].zDbSName;
        }
#లేకపోతే
        ప్రతిజ్ఞా( i_నిలువు==XN_ROWID || (i_నిలువు>=0 && i_నిలువు<pTab->n_నిలువు) );
        ఒకవేళ( i_నిలువు<0 ){
          zType = "INTEGER";
        }లేకపోతే{
          zType = sqlite3_స్తంభం_Type(&pTab->a_నిలువు[i_నిలువు],0);
        }
#అయితే_ముగింపు
      }
      ఆపు;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
    స్థితి TK_SELECT: {
      /* The expression is a sub-select. Return the declaration type and
      ** origin info for the single column in the result set of the SELECT
      ** statement.
      */
      NameContext sNC;
      Select *pS;
      సమాసం *p;
      ప్రతిజ్ఞా( సమాసం_Use_XSelect(p_సమాసం) );
      pS = p_సమాసం->x.pSelect;
      p = pS->pEList->a[0].p_సమాసం;
      sNC.pSrcList = pS->pSrc;
      sNC.p_తదుపరి = pNC;
      sNC.pParse = pNC->pParse;
      zType = స్తంభం_Type(&sNC, p, &zOrigDb, &zOrigTab, &z_Orig_నిలువు);
      ఆపు;
    }
#అయితే_ముగింపు
  }

#నిర్వచితమైతే SQLITE_ENABLE_స్తంభం_METADATA 
  ఒకవేళ( pzOrigDb ){
    ప్రతిజ్ఞా( pzOrigTab && pz_Orig_నిలువు );
    *pzOrigDb = zOrigDb;
    *pzOrigTab = zOrigTab;
    *pz_Orig_నిలువు = z_Orig_నిలువు;
  }
#అయితే_ముగింపు
  తిరిగి zType;
}

/*
** Generate code that will tell the VDBE the declaration types of columns
** in the result set.
*/
స్థిర శూన్య generate_స్తంభం_Types(
  Parse *pParse,      /* Parser context */
  SrcList *pTabList,  /* List of tables */
  సమాసం_List *pEList    /* Expressions defining the result set */
){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DECLTYPE
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం i;
  NameContext sNC;
  sNC.pSrcList = pTabList;
  sNC.pParse = pParse;
  sNC.p_తదుపరి = 0;
  కొరకు(i=0; i<pEList->n_సమాసం; i++){
    సమాసం *p = pEList->a[i].p_సమాసం;
    స్థిరరాశి అక్షరం *zType;
#నిర్వచితమైతే SQLITE_ENABLE_స్తంభం_METADATA
    స్థిరరాశి అక్షరం *zOrigDb = 0;
    స్థిరరాశి అక్షరం *zOrigTab = 0;
    స్థిరరాశి అక్షరం *z_Orig_నిలువు = 0;
    zType = స్తంభం_Type(&sNC, p, &zOrigDb, &zOrigTab, &z_Orig_నిలువు);

    /* The vdbe must make its own copy of the column-type and other
    ** column specific strings, in case the schema is reset before this
    ** virtual machine is deleted.
    */
    sqlite3_Vdbe_సమితి_నిలువు_Name(v, i, COLNAME_DATABASE, zOrigDb, SQLITE_TRANSIENT);
    sqlite3_Vdbe_సమితి_నిలువు_Name(v, i, COLNAME_పట్టిక, zOrigTab, SQLITE_TRANSIENT);
    sqlite3_Vdbe_సమితి_నిలువు_Name(v, i, COLNAME_స్తంభం, z_Orig_నిలువు, SQLITE_TRANSIENT);
#లేకపోతే
    zType = స్తంభం_Type(&sNC, p, 0, 0, 0);
#అయితే_ముగింపు
    sqlite3_Vdbe_సమితి_నిలువు_Name(v, i, COLNAME_DECLTYPE, zType, SQLITE_TRANSIENT);
  }
#లేకపోతే
  వాడని_PARAMETER(pParse);
  వాడని_PARAMETER(pTabList);
  వాడని_PARAMETER(pEList);
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_DECLTYPE) */
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
శూన్య sqlite3_Generate_స్తంభం_పేర్లు(
  Parse *pParse,      /* Parser context */
  Select *pSelect     /* Generate column names for this SELECT statement */
){
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం i;
  పట్టిక *pTab;
  SrcList *pTabList;
  సమాసం_List *pEList;
  sqlite3 *db = pParse->db;
  పూర్ణాంకం నిండిన_Name;    /* TABLE.COLUMN if no AS clause and is a direct table ref */
  పూర్ణాంకం srcName;     /* COLUMN or TABLE.COLUMN if no AS clause and is direct */

  ఒకవేళ( pParse->నిలువు_పేర్లు_సమితి ) తిరిగి;
  /* Column names are determined by the left-most term of a compound select */
  అయితే( pSelect->p_పూర్వ ) pSelect = pSelect->p_పూర్వ;
  TREETRACE(0x80,pParse,pSelect,("generating column names\n"));
  pTabList = pSelect->pSrc;
  pEList = pSelect->pEList;
  ప్రతిజ్ఞా( v!=0 );
  ప్రతిజ్ఞా( pTabList!=0 );
  pParse->నిలువు_పేర్లు_సమితి = 1;
  నిండిన_Name = (db->flags & SQLITE_నిండిన_నిలువు_పేర్లు)!=0;
  srcName = (db->flags & SQLITE_Short_నిలువు_పేర్లు)!=0 || నిండిన_Name;
  sqlite3_Vdbe_సమితి_అంకె_నిలువుల_సంఖ్య(v, pEList->n_సమాసం);
  కొరకు(i=0; i<pEList->n_సమాసం; i++){
    సమాసం *p = pEList->a[i].p_సమాసం;

    ప్రతిజ్ఞా( p!=0 );
    ప్రతిజ్ఞా( p->op!=TK_AGG_స్తంభం );  /* Agg processing has not run yet */
    ప్రతిజ్ఞా( p->op!=TK_స్తంభం
        || (సమాసం_Use_YTab(p) && p->y.pTab!=0) ); /* Covering idx not yet coded */
    ఒకవేళ( pEList->a[i].zEName && pEList->a[i].fg.eEName==ENAME_NAME ){
      /* An AS clause always takes first priority */
      అక్షరం *zName = pEList->a[i].zEName;
      sqlite3_Vdbe_సమితి_నిలువు_Name(v, i, COLNAME_NAME, zName, SQLITE_TRANSIENT);
    }లేకపోతే ఒకవేళ( srcName && p->op==TK_స్తంభం ){
      అక్షరం *z_నిలువు;
      పూర్ణాంకం i_నిలువు = p->i_స్తంభం;
      pTab = p->y.pTab;
      ప్రతిజ్ఞా( pTab!=0 );
      ఒకవేళ( i_నిలువు<0 ) i_నిలువు = pTab->iPKey;
      ప్రతిజ్ఞా( i_నిలువు==-1 || (i_నిలువు>=0 && i_నిలువు<pTab->n_నిలువు) );
      ఒకవేళ( i_నిలువు<0 ){
        z_నిలువు = "rowid";
      }లేకపోతే{
        z_నిలువు = pTab->a_నిలువు[i_నిలువు].zCnName;
      }
      ఒకవేళ( నిండిన_Name ){
        అక్షరం *zName = 0;
        zName = sqlite3MPrintf(db, "%s.%s", pTab->zName, z_నిలువు);
        sqlite3_Vdbe_సమితి_నిలువు_Name(v, i, COLNAME_NAME, zName, SQLITE_DYNAMIC);
      }లేకపోతే{
        sqlite3_Vdbe_సమితి_నిలువు_Name(v, i, COLNAME_NAME, z_నిలువు, SQLITE_TRANSIENT);
      }
    }లేకపోతే{
      స్థిరరాశి అక్షరం *z = pEList->a[i].zEName;
      z = z==0 ? sqlite3MPrintf(db, "column%d", i+1) : sqlite3DbStrDup(db, z);
      sqlite3_Vdbe_సమితి_నిలువు_Name(v, i, COLNAME_NAME, z, SQLITE_DYNAMIC);
    }
  }
  generate_స్తంభం_Types(pParse, pTabList, pEList);
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
పూర్ణాంకం sqlite3_Columns_From_సమాసం_List(
  Parse *pParse,          /* Parsing context */
  సమాసం_List *pEList,       /* Expr list from which to derive column names */
  i16 *pn_నిలువు,             /* Write the number of columns here */
  స్తంభం **pa_నిలువు          /* Write the new column list here */
){
  sqlite3 *db = pParse->db;   /* Database connection */
  పూర్ణాంకం i, j;                   /* Loop counters */
  u32 cnt;                    /* Index added to make the name unique */
  స్తంభం *a_నిలువు, *p_నిలువు;        /* For looping over result columns */
  పూర్ణాంకం n_నిలువు;                   /* Number of columns in the result set */
  అక్షరం *zName;                /* Column name */
  పూర్ణాంకం nName;                  /* Size of name in zName[] */
  Hash ht;                    /* Hash table of column names */
  పట్టిక *pTab;

  sqlite3HashInit(&ht);
  ఒకవేళ( pEList ){
    n_నిలువు = pEList->n_సమాసం;
    a_నిలువు = sqlite3DbMallocZero(db, పరిమాణం(a_నిలువు[0])*n_నిలువు);
    testcase( a_నిలువు==0 );
    ఒకవేళ( n_నిలువు>32767 ) n_నిలువు = 32767;
  }లేకపోతే{
    n_నిలువు = 0;
    a_నిలువు = 0;
  }
  ప్రతిజ్ఞా( n_నిలువు==(i16)n_నిలువు );
  *pn_నిలువు = n_నిలువు;
  *pa_నిలువు = a_నిలువు;

  కొరకు(i=0, p_నిలువు=a_నిలువు; i<n_నిలువు && !pParse->nErr; i++, p_నిలువు++){
    నిర్మాణం సమాసం_List_అంశం *pX = &pEList->a[i];
    నిర్మాణం సమాసం_List_అంశం *pCollide;
    /* Get an appropriate name for the column
    */
    ఒకవేళ( (zName = pX->zEName)!=0 && pX->fg.eEName==ENAME_NAME ){
      /* If the column contains an "AS <name>" phrase, use <name> as the name */
    }లేకపోతే{
      సమాసం *p_నిలువు_సమాసం = sqlite3_సమాసం_Skip_Collate_And_Likely(pX->p_సమాసం);
      అయితే( ALWAYS(p_నిలువు_సమాసం!=0) && p_నిలువు_సమాసం->op==TK_DOT ){
        p_నిలువు_సమాసం = p_నిలువు_సమాసం->p_కుడి;
        ప్రతిజ్ఞా( p_నిలువు_సమాసం!=0 );
      }
      ఒకవేళ( p_నిలువు_సమాసం->op==TK_స్తంభం
       && ALWAYS( సమాసం_Use_YTab(p_నిలువు_సమాసం) )
       && ALWAYS( p_నిలువు_సమాసం->y.pTab!=0 )
      ){
        /* For columns use the column name name */
        పూర్ణాంకం i_నిలువు = p_నిలువు_సమాసం->i_స్తంభం;
        pTab = p_నిలువు_సమాసం->y.pTab;
        ఒకవేళ( i_నిలువు<0 ) i_నిలువు = pTab->iPKey;
        zName = i_నిలువు>=0 ? pTab->a_నిలువు[i_నిలువు].zCnName : "rowid";
      }లేకపోతే ఒకవేళ( p_నిలువు_సమాసం->op==TK_ID ){
        ప్రతిజ్ఞా( !సమాసం_Has_Property(p_నిలువు_సమాసం, EP_Int_విలువ) );
        zName = p_నిలువు_సమాసం->u.zToken;
      }లేకపోతే{
        /* Use the original text of the column expression as its name */
        ప్రతిజ్ఞా( zName==pX->zEName );  /* pointer comparison intended */
      }
    }
    ఒకవేళ( zName && !sqlite3IsTrueOrFalse(zName) ){
      zName = sqlite3DbStrDup(db, zName);
    }లేకపోతే{
      zName = sqlite3MPrintf(db,"column%d",i+1);
    }

    /* Make sure the column name is unique.  If the name is not unique,
    ** append an integer to the name so that it becomes unique.
    */
    cnt = 0;
    అయితే( zName && (pCollide = sqlite3_Hash_కనుగొను(&ht, zName))!=0 ){
      ఒకవేళ( pCollide->fg.b_Using_పదం ){
        p_నిలువు->నిలువు_Flags |= COLFLAG_NOEXPAND;
      }
      nName = sqlite3Strlen30(zName);
      ఒకవేళ( nName>0 ){
        కొరకు(j=nName-1; j>0 && sqlite3Isdigit(zName[j]); j--){}
        ఒకవేళ( zName[j]==':' ) nName = j;
      }
      zName = sqlite3MPrintf(db, "%.*z:%u", nName, zName, ++cnt);
      sqlite3_పురోగతి_Check(pParse);
      ఒకవేళ( cnt>3 ){
        sqlite3_randomness(పరిమాణం(cnt), &cnt);
      }
    }
    p_నిలువు->zCnName = zName;
    p_నిలువు->hName = sqlite3StrIHash(zName);
    ఒకవేళ( pX->fg.bNoExpand ){
      p_నిలువు->నిలువు_Flags |= COLFLAG_NOEXPAND;
    }
    sqlite3_స్తంభం_Properties_From_Name(0, p_నిలువు);
    ఒకవేళ( zName && sqlite3_Hash_చొప్పించు(&ht, zName, pX)==pX ){
      sqlite3OomFault(db);
    }
  }
  sqlite3HashClear(&ht);
  ఒకవేళ( pParse->nErr ){
    కొరకు(j=0; j<i; j++){
      sqlite3DbFree(db, a_నిలువు[j].zCnName);
    }
    sqlite3DbFree(db, a_నిలువు);
    *pa_నిలువు = 0;
    *pn_నిలువు = 0;
    తిరిగి pParse->rc;
  }
  తిరిగి SQLITE_OK;
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
శూన్య sqlite3_Subquery_స్తంభం_Types(
  Parse *pParse,      /* Parsing contexts */
  పట్టిక *pTab,        /* Add column type information to this table */
  Select *pSelect,    /* SELECT used to determine types and collations */
  అక్షరం aff            /* Default affinity. */
){
  sqlite3 *db = pParse->db;
  స్తంభం *p_నిలువు;
  CollSeq *pColl;
  పూర్ణాంకం i,j;
  సమాసం *p;
  నిర్మాణం సమాసం_List_అంశం *a;
  NameContext sNC;

  ప్రతిజ్ఞా( pSelect!=0 );
  ప్రతిజ్ఞా( (pSelect->selFlags & SF_Resolved)!=0 );
  ప్రతిజ్ఞా( pTab->n_నిలువు==pSelect->pEList->n_సమాసం || pParse->nErr>0 );
  ప్రతిజ్ఞా( aff==SQLITE_AFF_NONE || aff==SQLITE_AFF_BLOB );
  ఒకవేళ( db->mallocFailed || లోపల_RENAME_OBJECT ) తిరిగి;
  అయితే( pSelect->p_పూర్వ ) pSelect = pSelect->p_పూర్వ;
  a = pSelect->pEList->a;
  memset(&sNC, 0, పరిమాణం(sNC));
  sNC.pSrcList = pSelect->pSrc;
  కొరకు(i=0, p_నిలువు=pTab->a_నిలువు; i<pTab->n_నిలువు; i++, p_నిలువు++){
    స్థిరరాశి అక్షరం *zType;
    i64 n;
    పూర్ణాంకం m = 0;
    Select *pS2 = pSelect;
    pTab->tabFlags |= (p_నిలువు->నిలువు_Flags & COLFLAG_NOINSERT);
    p = a[i].p_సమాసం;
    /* pCol->szEst = ... // Column size est for SELECT tables never used */
    p_నిలువు->affinity = sqlite3_సమాసం_Affinity(p);
    అయితే( p_నిలువు->affinity<=SQLITE_AFF_NONE && pS2->p_తదుపరి!=0 ){
      m |= sqlite3_సమాసం_దత్తాంశం_Type(pS2->pEList->a[i].p_సమాసం);
      pS2 = pS2->p_తదుపరి;
      p_నిలువు->affinity = sqlite3_సమాసం_Affinity(pS2->pEList->a[i].p_సమాసం);
    }
    ఒకవేళ( p_నిలువు->affinity<=SQLITE_AFF_NONE ){
      p_నిలువు->affinity = aff;
    }
    ఒకవేళ( p_నిలువు->affinity>=SQLITE_AFF_పాఠ్యం && (pS2->p_తదుపరి || pS2!=pSelect) ){
      కొరకు(pS2=pS2->p_తదుపరి; pS2; pS2=pS2->p_తదుపరి){
        m |= sqlite3_సమాసం_దత్తాంశం_Type(pS2->pEList->a[i].p_సమాసం);
      }
      ఒకవేళ( p_నిలువు->affinity==SQLITE_AFF_పాఠ్యం && (m&0x01)!=0 ){
        p_నిలువు->affinity = SQLITE_AFF_BLOB;
      }లేకపోతే
      ఒకవేళ( p_నిలువు->affinity>=SQLITE_AFF_NUMERIC && (m&0x02)!=0 ){
        p_నిలువు->affinity = SQLITE_AFF_BLOB;
      }
      ఒకవేళ( p_నిలువు->affinity>=SQLITE_AFF_NUMERIC && p->op==TK_CAST ){
        p_నిలువు->affinity = SQLITE_AFF_FLEXNUM;
      }
    }
    zType = స్తంభం_Type(&sNC, p, 0, 0, 0);
    ఒకవేళ( zType==0 || p_నిలువు->affinity!=sqlite3AffinityType(zType, 0) ){
      ఒకవేళ( p_నిలువు->affinity==SQLITE_AFF_NUMERIC
       || p_నిలువు->affinity==SQLITE_AFF_FLEXNUM
      ){
        zType = "NUM";
      }లేకపోతే{
        zType = 0;
        కొరకు(j=1; j<SQLITE_N_STDTYPE; j++){
          ఒకవేళ( sqlite3StdTypeAffinity[j]==p_నిలువు->affinity ){
            zType = sqlite3StdType[j];
            ఆపు;
          }
        }
      }
    }
    ఒకవేళ( zType ){
      స్థిరరాశి i64 k = అక్షరశ్రేణి_పొడవు(zType);
      n = అక్షరశ్రేణి_పొడవు(p_నిలువు->zCnName);
      p_నిలువు->zCnName = sqlite3DbReallocOrFree(db, p_నిలువు->zCnName, n+k+2);
      p_నిలువు->నిలువు_Flags &= ~(COLFLAG_HASTYPE|COLFLAG_HASCOLL);
      ఒకవేళ( p_నిలువు->zCnName ){
        జ్ఞాపకం_నకలు(&p_నిలువు->zCnName[n+1], zType, k+1);
        p_నిలువు->నిలువు_Flags |= COLFLAG_HASTYPE;
      }
    }
    pColl = sqlite3_సమాసం_Coll_Seq(pParse, p);
    ఒకవేళ( pColl ){
      ప్రతిజ్ఞా( pTab->p_సూచీ==0 );
      sqlite3_స్తంభం_సమితి_Coll(db, p_నిలువు, pColl->zName);
    }
  }
  pTab->sz_Tab_అడ్డువరుస = 1; /* Any non-zero value works */
}

/*
** Check the current subquery nesting depth.  Return true and set an
** error if it has gone too deep.
*/
స్థిర పూర్ణాంకం check_Subquery_Nesting_లోతు(Parse *pParse){
#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
  ఒకవేళ( pParse->nNestSel >= pParse->db->a_హద్దు[SQLITE_హద్దు_సమాసం_లోతు] ){
    sqlite3_Error_సందేశం(pParse, "VIEWs and/or subqueries nested too deep");
    తిరిగి 1;
  }
#అయితే_ముగింపు
  తిరిగి 0;
}

/*
** Given a SELECT statement, generate a Table structure that describes
** the result set of that SELECT.
*/
పట్టిక *sqlite3_ఫలితం_సమితి_యొక్క_Select(Parse *pParse, Select *pSelect, అక్షరం aff){
  పట్టిక *pTab;
  sqlite3 *db = pParse->db;
  u64 savedFlags;

  pParse->nNestSel++;
  ఒకవేళ( check_Subquery_Nesting_లోతు(pParse) ) తిరిగి 0;
  savedFlags = db->flags;
  db->flags &= ~(u64)SQLITE_నిండిన_నిలువు_పేర్లు;
  db->flags |= SQLITE_Short_నిలువు_పేర్లు;
  sqlite3SelectPrep(pParse, pSelect, 0);
  db->flags = savedFlags;
  ఒకవేళ( pParse->nErr ) తిరిగి 0;
  అయితే( pSelect->p_పూర్వ ) pSelect = pSelect->p_పూర్వ;
  pTab = sqlite3DbMallocZero(db, పరిమాణం(పట్టిక) );
  ఒకవేళ( pTab==0 ){
    తిరిగి 0;
  }
  pTab->nTabRef = 1;
  pTab->zName = 0;
  pTab->n_అడ్డువరుస_Log_Est = 200; ప్రతిజ్ఞా( 200==sqlite3LogEst(1048576) );
  sqlite3_Columns_From_సమాసం_List(pParse, pSelect->pEList, &pTab->n_నిలువు, &pTab->a_నిలువు);
  sqlite3_Subquery_స్తంభం_Types(pParse, pTab, pSelect, aff);
  pTab->iPKey = -1;
  ఒకవేళ( db->mallocFailed ){
    sqlite3_Delete_పట్టిక(db, pTab);
    తిరిగి 0;
  }
  pParse->nNestSel--;
  ప్రతిజ్ఞా( pParse->nNestSel>=0 );
  తిరిగి pTab;
}

/*
** Get a VDBE for the given parser context.  Create a new one if necessary.
** If an error occurs, return NULL and leave a message in pParse.
*/
Vdbe *sqlite3GetVdbe(Parse *pParse){
  ఒకవేళ( pParse->pVdbe ){
    తిరిగి pParse->pVdbe;
  }
  ఒకవేళ( pParse->pToplevel==0 ){
    pParse->ok_Const_గుణకం = 1;
  }
  తిరిగి sqlite3VdbeCreate(pParse);
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
స్థిర శూన్య compute_హద్దు_Registers(Parse *pParse, Select *p, పూర్ణాంకం iBreak){
  Vdbe *v = 0;
  పూర్ణాంకం i_హద్దు = 0;
  పూర్ణాంకం i_స్థానభ్రంశం;
  పూర్ణాంకం n;
  సమాసం *p_హద్దు = p->p_హద్దు;

  ఒకవేళ( p->i_హద్దు ) తిరిగి;

  /*
  ** "LIMIT -1" always shows all rows.  There is some
  ** controversy about what the correct behavior should be.
  ** The current implementation interprets "LIMIT 0" to mean
  ** no rows.
  */
  ఒకవేళ( p_హద్దు ){
    ప్రతిజ్ఞా( p_హద్దు->op==TK_హద్దు );
    ప్రతిజ్ఞా( p_హద్దు->p_ఎడమ!=0 );
    p->i_హద్దు = i_హద్దు = ++pParse->nMem;
    v = sqlite3GetVdbe(pParse);
    ప్రతిజ్ఞా( v!=0 );
    ఒకవేళ( sqlite3_సమాసం_Is_Integer(p_హద్దు->p_ఎడమ, &n, pParse, 0) ){
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, n, i_హద్దు);
      VdbeComment((v, "LIMIT counter"));
      ఒకవేళ( n==0 ){
        sqlite3VdbeGoto(v, iBreak);
      }లేకపోతే ఒకవేళ( n>=0 && p->n_Select_అడ్డువరుస>sqlite3LogEst((u64)n) ){
        p->n_Select_అడ్డువరుస = sqlite3LogEst((u64)n);
        p->selFlags |= SF_Fixed_హద్దు;
      }
    }లేకపోతే{
      sqlite3_సమాసం_సంకేతం(pParse, p_హద్దు->p_ఎడమ, i_హద్దు);
      sqlite3_Vdbe_కూడు_Op1(v, OP_MustBeInt, i_హద్దు); VdbeCoverage(v);
      VdbeComment((v, "LIMIT counter"));
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_కూడు_Op2(v, OP_IfNot, i_హద్దు, iBreak); VdbeCoverage(v);
    }
    ఒకవేళ( p_హద్దు->p_కుడి ){
      p->i_స్థానభ్రంశం = i_స్థానభ్రంశం = ++pParse->nMem;
      pParse->nMem++;   /* Allocate an extra register for limit+offset */
      sqlite3_సమాసం_సంకేతం(pParse, p_హద్దు->p_కుడి, i_స్థానభ్రంశం);
      sqlite3_Vdbe_కూడు_Op1(v, OP_MustBeInt, i_స్థానభ్రంశం); VdbeCoverage(v);
      sqlite3MayAbort(pParse);
      VdbeComment((v, "OFFSET counter"));
      sqlite3_Vdbe_కూడు_Op3(v, OP_స్థానభ్రంశం_హద్దు, i_హద్దు, i_స్థానభ్రంశం+1, i_స్థానభ్రంశం);
      VdbeComment((v, "LIMIT+OFFSET"));
    }
  }
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_COMPOUND_SELECT
/*
** Return the appropriate collating sequence for the iCol-th column of
** the result set for the compound-select statement "p".  Return NULL if
** the column has no default collating sequence.
**
** The collating sequence for the compound select is taken from the
** left-most term of the select that has a collating sequence.
*/
స్థిర CollSeq *multiSelectCollSeq(Parse *pParse, Select *p, పూర్ణాంకం i_నిలువు){
  CollSeq *pRet;
  ఒకవేళ( p->p_పూర్వ ){
    pRet = multiSelectCollSeq(pParse, p->p_పూర్వ, i_నిలువు);
  }లేకపోతే{
    pRet = 0;
  }
  ప్రతిజ్ఞా( i_నిలువు>=0 );
  /* iCol must be less than p->pEList->nExpr.  Otherwise an error would
  ** have been thrown during name resolution and we would not have gotten
  ** this far */
  ఒకవేళ( pRet==0 && ALWAYS(i_నిలువు<p->pEList->n_సమాసం) ){
    pRet = sqlite3_సమాసం_Coll_Seq(pParse, p->pEList->a[i_నిలువు].p_సమాసం);
  }
  తిరిగి pRet;
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
స్థిర కీలకం_Info *multi_Select_By_విలీనం_కీలకం_Info(Parse *pParse, Select *p, పూర్ణాంకం nExtra){
  సమాసం_List *p_క్రమం_By = p->p_క్రమం_By;
  పూర్ణాంకం n_క్రమం_By = (p_క్రమం_By!=0) ? p_క్రమం_By->n_సమాసం : 0;
  sqlite3 *db = pParse->db;
  కీలకం_Info *pRet = sqlite3_కీలకం_Info_Alloc(db, n_క్రమం_By+nExtra, 1);
  ఒకవేళ( pRet ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<n_క్రమం_By; i++){
      నిర్మాణం సమాసం_List_అంశం *p_అంశం = &p_క్రమం_By->a[i];
      సమాసం *p_పదం = p_అంశం->p_సమాసం;
      CollSeq *pColl;

      ఒకవేళ( p_పదం->flags & EP_Collate ){
        pColl = sqlite3_సమాసం_Coll_Seq(pParse, p_పదం);
      }లేకపోతే{
        pColl = multiSelectCollSeq(pParse, p, p_అంశం->u.x.i_క్రమం_By_నిలువు-1);
        ఒకవేళ( pColl==0 ) pColl = db->pDfltColl;
        p_క్రమం_By->a[i].p_సమాసం =
          sqlite3_సమాసం_కూడు_Collate_String(pParse, p_పదం, pColl->zName);
      }
      ప్రతిజ్ఞా( sqlite3_కీలకం_Info_Is_Writeable(pRet) );
      pRet->aColl[i] = pColl;
      pRet->a_క్రమీకరించు_Flags[i] = p_క్రమం_By->a[i].fg.క్రమీకరించు_Flags;
    }
  }

  తిరిగి pRet;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_CTE
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
స్థిర శూన్య generateWithRecursiveQuery(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The recursive SELECT to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  SrcList *pSrc = p->pSrc;      /* The FROM clause of the recursive query */
  పూర్ణాంకం n_నిలువు = p->pEList->n_సమాసం;  /* Number of columns in the recursive table */
  Vdbe *v = pParse->pVdbe;      /* The prepared statement under construction */
  Select *p_అమరిక;               /* The setup query */
  Select *p_మొదటి_Rec;            /* Left-most recursive term */
  పూర్ణాంకం addr_శిఖరం;                  /* Top of the loop */
  పూర్ణాంకం addrCont, addrBreak;      /* CONTINUE and BREAK addresses */
  పూర్ణాంకం iCurrent = 0;             /* The Current table */
  పూర్ణాంకం regCurrent;               /* Register holding Current table */
  పూర్ణాంకం i_వరుస;                   /* The Queue table */
  పూర్ణాంకం iDistinct = 0;            /* To ensure unique results if UNION */
  పూర్ణాంకం eDest = SRT_Fifo;         /* How to write to Queue */
  SelectDest dest_వరుస;         /* SelectDest targeting the Queue table */
  పూర్ణాంకం i;                        /* Loop counter */
  పూర్ణాంకం rc;                       /* Result code */
  సమాసం_List *p_క్రమం_By;           /* The ORDER BY clause */
  సమాసం *p_హద్దు;                 /* Saved LIMIT and OFFSET */
  పూర్ణాంకం reg_హద్దు, reg_స్థానభ్రంశం;      /* Registers used by LIMIT and OFFSET */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
  ఒకవేళ( p->pWin ){
    sqlite3_Error_సందేశం(pParse, "cannot use window functions in recursive queries");
    తిరిగి;
  }
#అయితే_ముగింపు

  /* Obtain authorization to do a recursive query */
  ఒకవేళ( sqlite3AuthCheck(pParse, SQLITE_RECURSIVE, 0, 0, 0) ) తిరిగి;

  /* Process the LIMIT and OFFSET clauses, if they exist */
  addrBreak = sqlite3VdbeMakeLabel(pParse);
  p->n_Select_అడ్డువరుస = 320;  /* 4 billion rows */
  compute_హద్దు_Registers(pParse, p, addrBreak);
  p_హద్దు = p->p_హద్దు;
  reg_హద్దు = p->i_హద్దు;
  reg_స్థానభ్రంశం = p->i_స్థానభ్రంశం;
  p->p_హద్దు = 0;
  p->i_హద్దు = p->i_స్థానభ్రంశం = 0;
  p_క్రమం_By = p->p_క్రమం_By;

  /* Locate the cursor number of the Current table */
  కొరకు(i=0; ALWAYS(i<pSrc->nSrc); i++){
    ఒకవేళ( pSrc->a[i].fg.isRecursive ){
      iCurrent = pSrc->a[i].iCursor;
      ఆపు;
    }
  }

  /* Allocate cursors numbers for Queue and Distinct.  The cursor number for
  ** the Distinct table must be exactly one greater than Queue in order
  ** for the SRT_DistFifo and SRT_DistQueue destinations to work. */
  i_వరుస = pParse->nTab++;
  ఒకవేళ( p->op==TK_UNION ){
    eDest = p_క్రమం_By ? SRT_Dist_వరుస : SRT_DistFifo;
    iDistinct = pParse->nTab++;
  }లేకపోతే{
    eDest = p_క్రమం_By ? SRT_వరుస : SRT_Fifo;
  }
  sqlite3SelectDestInit(&dest_వరుస, eDest, i_వరుస);

  /* Allocate cursors for Current, Queue, and Distinct. */
  regCurrent = ++pParse->nMem;
  sqlite3_Vdbe_కూడు_Op3(v, OP_OpenPseudo, iCurrent, regCurrent, n_నిలువు);
  ఒకవేళ( p_క్రమం_By ){
    కీలకం_Info *p_కీలకం_Info = multi_Select_By_విలీనం_కీలకం_Info(pParse, p, 1);
    sqlite3_Vdbe_కూడు_Op4(v, OP_OpenEphemeral, i_వరుస, p_క్రమం_By->n_సమాసం+2, 0,
                      (అక్షరం*)p_కీలకం_Info, P4_KEYINFO);
    dest_వరుస.p_క్రమం_By = p_క్రమం_By;
  }లేకపోతే{
    sqlite3_Vdbe_కూడు_Op2(v, OP_OpenEphemeral, i_వరుస, n_నిలువు);
  }
  VdbeComment((v, "Queue table"));
  ఒకవేళ( iDistinct ){
    /* Generate an ephemeral table used to enforce distinctness on the
    ** output of the recursive part of the CTE.
    */
    కీలకం_Info *p_కీలకం_Info;            /* Collating sequence for the result set */
    CollSeq **apColl;             /* For looping through pKeyInfo->aColl[] */

    ప్రతిజ్ఞా( p->p_తదుపరి==0 );
    ప్రతిజ్ఞా( p->pEList!=0 );
    n_నిలువు = p->pEList->n_సమాసం;
    p_కీలకం_Info = sqlite3_కీలకం_Info_Alloc(pParse->db, n_నిలువు, 1);
    ఒకవేళ( p_కీలకం_Info ){
      కొరకు(i=0, apColl=p_కీలకం_Info->aColl; i<n_నిలువు; i++, apColl++){
        *apColl = multiSelectCollSeq(pParse, p, i);
        ఒకవేళ( 0==*apColl ){
          *apColl = pParse->db->pDfltColl;
        }
      }
      sqlite3_Vdbe_కూడు_Op4(v, OP_OpenEphemeral, iDistinct, n_నిలువు, 0,
                           (శూన్య*)p_కీలకం_Info, P4_KEYINFO);
    }లేకపోతే{
      ప్రతిజ్ఞా( pParse->nErr>0 );
    }
  }

  /* Detach the ORDER BY clause from the compound SELECT */
  p->p_క్రమం_By = 0;

  /* Figure out how many elements of the compound SELECT are part of the
  ** recursive query.  Make sure no recursive elements use aggregate
  ** functions.  Mark the recursive elements as UNION ALL even if they
  ** are really UNION because the distinctness will be enforced by the
  ** iDistinct table.  pFirstRec is left pointing to the left-most
  ** recursive term of the CTE.
  */
  కొరకు(p_మొదటి_Rec=p; ALWAYS(p_మొదటి_Rec!=0); p_మొదటి_Rec=p_మొదటి_Rec->p_పూర్వ){
    ఒకవేళ( p_మొదటి_Rec->selFlags & SF_Aggregate ){
      sqlite3_Error_సందేశం(pParse, "recursive aggregate queries not supported");
      వెళ్ళు చివర_యొక్క_recursive_query;
    }
    p_మొదటి_Rec->op = TK_ALL;
    ఒకవేళ( (p_మొదటి_Rec->p_పూర్వ->selFlags & SF_Recursive)==0 ) ఆపు;
  }

  /* Store the results of the setup-query in Queue. */
  p_అమరిక = p_మొదటి_Rec->p_పూర్వ;
  p_అమరిక->p_తదుపరి = 0;
  ExplainQueryPlan((pParse, 1, "SETUP"));
  rc = sqlite3Select(pParse, p_అమరిక, &dest_వరుస);
  p_అమరిక->p_తదుపరి = p;
  ఒకవేళ( rc ) వెళ్ళు చివర_యొక్క_recursive_query;

  /* Find the next row in the Queue and output that row */
  addr_శిఖరం = sqlite3_Vdbe_కూడు_Op2(v, OP_Rewind, i_వరుస, addrBreak); VdbeCoverage(v);

  /* Transfer the next row in Queue over to Current */
  sqlite3_Vdbe_కూడు_Op1(v, OP_Null_అడ్డువరుస, iCurrent); /* To reset column cache */
  ఒకవేళ( p_క్రమం_By ){
    sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, i_వరుస, p_క్రమం_By->n_సమాసం+1, regCurrent);
  }లేకపోతే{
    sqlite3_Vdbe_కూడు_Op2(v, OP_అడ్డువరుస_దత్తాంశం, i_వరుస, regCurrent);
  }
  sqlite3_Vdbe_కూడు_Op1(v, OP_Delete, i_వరుస);

  /* Output the single row in Current */
  addrCont = sqlite3VdbeMakeLabel(pParse);
  సంకేతం_స్థానభ్రంశం(v, reg_స్థానభ్రంశం, addrCont);
  selectInnerLoop(pParse, p, iCurrent,
      0, 0, pDest, addrCont, addrBreak);
  ఒకవేళ( reg_హద్దు ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_DecrJumpZero, reg_హద్దు, addrBreak);
    VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, addrCont);

  /* Execute the recursive SELECT taking the single row in Current as
  ** the value for the recursive-table. Store the results in the Queue.
  */
  p_మొదటి_Rec->p_పూర్వ = 0;
  ExplainQueryPlan((pParse, 1, "RECURSIVE STEP"));
  sqlite3Select(pParse, p, &dest_వరుస);
  ప్రతిజ్ఞా( p_మొదటి_Rec->p_పూర్వ==0 );
  p_మొదటి_Rec->p_పూర్వ = p_అమరిక;

  /* Keep running the loop until the Queue is empty */
  sqlite3VdbeGoto(v, addr_శిఖరం);
  sqlite3VdbeResolveLabel(v, addrBreak);

చివర_యొక్క_recursive_query:
  sqlite3_సమాసం_List_Delete(pParse->db, p->p_క్రమం_By);
  p->p_క్రమం_By = p_క్రమం_By;
  p->p_హద్దు = p_హద్దు;
  తిరిగి;
}
#అయితే_ముగింపు /* SQLITE_OMIT_CTE */

/* Forward references */
స్థిర పూర్ణాంకం multi_Select_By_విలీనం(
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
స్థిర పూర్ణాంకం multi_Select_విలువలు(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  పూర్ణాంకం n_అడ్డువరుస = 1;
  పూర్ణాంకం rc = 0;
  పూర్ణాంకం bShowAll = p->p_హద్దు==0;
  ప్రతిజ్ఞా( p->selFlags & SF_Multi_విలువ );
  చేయి{
    ప్రతిజ్ఞా( p->selFlags & SF_విలువలు );
    ప్రతిజ్ఞా( p->op==TK_ALL || (p->op==TK_SELECT && p->p_పూర్వ==0) );
    ప్రతిజ్ఞా( p->p_తదుపరి==0 || p->pEList->n_సమాసం==p->p_తదుపరి->pEList->n_సమాసం );
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    ఒకవేళ( p->pWin ) తిరిగి -1;
#అయితే_ముగింపు
    ఒకవేళ( p->p_పూర్వ==0 ) ఆపు;
    ప్రతిజ్ఞా( p->p_పూర్వ->p_తదుపరి==p );
    p = p->p_పూర్వ;
    n_అడ్డువరుస += bShowAll;
  }అయితే(1);
  ExplainQueryPlan((pParse, 0, "SCAN %d CONSTANT ROW%s", n_అడ్డువరుస,
                    n_అడ్డువరుస==1 ? "" : "S"));
  అయితే( p ){
    selectInnerLoop(pParse, p, -1, 0, 0, pDest, 1, 1);
    ఒకవేళ( !bShowAll ) ఆపు;
    p->n_Select_అడ్డువరుస = n_అడ్డువరుస;
    p = p->p_తదుపరి;
  }
  తిరిగి rc;
}

/*
** Return true if the SELECT statement which is known to be the recursive
** part of a recursive CTE still has its anchor terms attached.  If the
** anchor terms have already been removed, then return false.
*/
స్థిర పూర్ణాంకం hasAnchor(Select *p){
  అయితే( p && (p->selFlags & SF_Recursive)!=0 ){ p = p->p_పూర్వ; }
  తిరిగి p!=0;
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
స్థిర పూర్ణాంకం union_With_హద్దు_One(sqlite3 *db, Select *p){
  పూర్ణాంకం v;
  ఒకవేళ( p->op!=TK_UNION ) తిరిగి 0;
  ఒకవేళ( p->p_హద్దు==0 ) తిరిగి 0;
  ఒకవేళ( p->p_హద్దు->p_కుడి ) తిరిగి 0;  /* No OFFSET */
  v = 0;
  ప్రతిజ్ఞా( p->p_హద్దు->p_ఎడమ!=0 );
  ఒకవేళ( sqlite3_సమాసం_Is_Integer(p->p_హద్దు->p_ఎడమ, &v, 0, 0)==0 ) తిరిగి 0;
  ఒకవేళ( v!=1 ) తిరిగి 0;
  ఒకవేళ( OptimizationDisabled(db, SQLITE_Union_హద్దు) ) తిరిగి 0;
  తిరిగి 1;
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
స్థిర పూర్ణాంకం multiSelect(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  పూర్ణాంకం rc = SQLITE_OK;   /* Success code from a subroutine */
  Select *p_పూర్వ;       /* Another SELECT immediately to our left */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest dest;      /* Alternative data destination */
  Select *pDelete = 0;  /* Chain of simple selects to delete */
  sqlite3 *db;          /* Database connection */

  /* Make sure there is no ORDER BY or LIMIT clause on prior SELECTs.  Only
  ** the last (right-most) SELECT in the series may have an ORDER BY or LIMIT.
  */
  ప్రతిజ్ఞా( p && p->p_పూర్వ );  /* Calling function guarantees this much */
  ప్రతిజ్ఞా( (p->selFlags & SF_Recursive)==0 || p->op==TK_ALL || p->op==TK_UNION );
  ప్రతిజ్ఞా( p->selFlags & SF_Compound );
  db = pParse->db;
  p_పూర్వ = p->p_పూర్వ;
  dest = *pDest;
  ప్రతిజ్ఞా( p_పూర్వ->p_క్రమం_By==0 );
  ప్రతిజ్ఞా( p_పూర్వ->p_హద్దు==0 );

  v = sqlite3GetVdbe(pParse);
  ప్రతిజ్ఞా( v!=0 );  /* The VDBE already created by calling function */

  /* Create the destination temporary table if necessary
  */
  ఒకవేళ( dest.eDest==SRT_EphemTab ){
    ప్రతిజ్ఞా( p->pEList );
    sqlite3_Vdbe_కూడు_Op2(v, OP_OpenEphemeral, dest.iSDParm, p->pEList->n_సమాసం);
    dest.eDest = SRT_పట్టిక;
  }

  /* Special handling for a compound-select that originates as a VALUES clause.
  */
  ఒకవేళ( p->selFlags & SF_Multi_విలువ ){
    rc = multi_Select_విలువలు(pParse, p, &dest);
    ఒకవేళ( rc>=0 ) వెళ్ళు multi_select_చివర;
    rc = SQLITE_OK;
  }

  /* Make sure all SELECTs in the statement have the same number of elements
  ** in their result sets.
  */
  ప్రతిజ్ఞా( p->pEList && p_పూర్వ->pEList );
  ప్రతిజ్ఞా( p->pEList->n_సమాసం==p_పూర్వ->pEList->n_సమాసం );

#నిర్వచితం_కాకపోతే SQLITE_OMIT_CTE
  ఒకవేళ( (p->selFlags & SF_Recursive)!=0 && hasAnchor(p) ){
    generateWithRecursiveQuery(pParse, p, &dest);
  }లేకపోతే
#అయితే_ముగింపు
  ఒకవేళ( p->p_క్రమం_By ){
    /* If the compound has an ORDER BY clause, then always use the merge
    ** algorithm. */
    తిరిగి multi_Select_By_విలీనం(pParse, p, pDest);
  }లేకపోతే ఒకవేళ( p->op!=TK_ALL && !union_With_హద్దు_One(db,p) ){
    /* If the compound is EXCEPT, INTERSECT, or UNION (anything other than
    ** UNION ALL) then also always use the merge algorithm.  However, the
    ** multiSelectByMerge() routine requires that the compound have an
    ** ORDER BY clause, and it doesn't right now.  So invent one first. */
    సమాసం *pOne = sqlite3_సమాసం_Int32(db, 1);
    p->p_క్రమం_By = sqlite3_సమాసం_List_Append(pParse, 0, pOne);
    ఒకవేళ( pParse->nErr ) వెళ్ళు multi_select_చివర;
    ప్రతిజ్ఞా( p->p_క్రమం_By!=0 );
    p->p_క్రమం_By->a[0].u.x.i_క్రమం_By_నిలువు = 1;
    తిరిగి multi_Select_By_విలీనం(pParse, p, pDest);
  }లేకపోతే{
    /* For a UNION ALL compound without ORDER BY, or for a UNION with a 
    ** LIMIT of exactly 1 and no OFFSET and no ORDER BY, simply run the left
    ** query, then run the right query */
    పూర్ణాంకం addr = 0;
    పూర్ణాంకం n_హద్దు = 0;  /* Initialize to suppress harmless compiler warning */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_EXPLAIN
    ఒకవేళ( p_పూర్వ->p_పూర్వ==0 ){
      ExplainQueryPlan((pParse, 1, "COMPOUND QUERY"));
      ExplainQueryPlan((pParse, 1, "LEFT-MOST SUBQUERY"));
    }
#అయితే_ముగింపు
    ప్రతిజ్ఞా( !p_పూర్వ->p_హద్దు );
    p_పూర్వ->i_హద్దు = p->i_హద్దు;
    p_పూర్వ->i_స్థానభ్రంశం = p->i_స్థానభ్రంశం;
    p_పూర్వ->p_హద్దు = sqlite3_సమాసం_Dup(db, p->p_హద్దు, 0);
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL left...\n"));
    rc = sqlite3Select(pParse, p_పూర్వ, &dest);
    sqlite3_సమాసం_Delete(db, p_పూర్వ->p_హద్దు);
    p_పూర్వ->p_హద్దు = 0;
    ఒకవేళ( rc ){
      వెళ్ళు multi_select_చివర;
    }
    p->p_పూర్వ = 0;
    p->i_హద్దు = p_పూర్వ->i_హద్దు;
    p->i_స్థానభ్రంశం = p_పూర్వ->i_స్థానభ్రంశం;
    ఒకవేళ( p->i_హద్దు ){
      addr = sqlite3_Vdbe_కూడు_Op1(v, OP_IfNot, p->i_హద్దు); VdbeCoverage(v);
      VdbeComment((v, "Jump ahead if LIMIT reached"));
      ఒకవేళ( p->i_స్థానభ్రంశం ){
        sqlite3_Vdbe_కూడు_Op3(v, OP_స్థానభ్రంశం_హద్దు,
                          p->i_హద్దు, p->i_స్థానభ్రంశం+1, p->i_స్థానభ్రంశం);
      }
    }
    ExplainQueryPlan((pParse, 1, "UNION ALL"));
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL right...\n"));
    rc = sqlite3Select(pParse, p, &dest);
    testcase( rc!=SQLITE_OK );
    pDelete = p->p_పూర్వ;
    p->p_పూర్వ = p_పూర్వ;
    p->n_Select_అడ్డువరుస = sqlite3_Log_Est_కూడు(p->n_Select_అడ్డువరుస, p_పూర్వ->n_Select_అడ్డువరుస);
    ఒకవేళ( p->p_హద్దు
     && sqlite3_సమాసం_Is_Integer(p->p_హద్దు->p_ఎడమ, &n_హద్దు, pParse, 0)
     && n_హద్దు>0 && p->n_Select_అడ్డువరుస > sqlite3LogEst((u64)n_హద్దు)
    ){
      p->n_Select_అడ్డువరుస = sqlite3LogEst((u64)n_హద్దు);
    }
    ఒకవేళ( addr ){
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addr);
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_EXPLAIN
    ఒకవేళ( p->p_తదుపరి==0 ){
      Explain_Query_Plan_తీయి(pParse);
    }
#అయితే_ముగింపు
  }

multi_select_చివర:
  pDest->iSdst = dest.iSdst;
  pDest->nSdst = dest.nSdst;
  pDest->iSDParm2 = dest.iSDParm2;
  ఒకవేళ( pDelete ){
    sqlite3_Parser_కూడు_Cleanup(pParse, sqlite3SelectDeleteGeneric, pDelete);
  }
  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_OMIT_COMPOUND_SELECT */

/*
** Error message for when two or more terms of a compound select have different
** size result sets.
*/
శూన్య sqlite3_Select_Wrong_అంకె_Terms_Error(Parse *pParse, Select *p){
  ఒకవేళ( p->selFlags & SF_విలువలు ){
    sqlite3_Error_సందేశం(pParse, "all VALUES must have the same number of terms");
  }లేకపోతే{
    sqlite3_Error_సందేశం(pParse, "SELECTs to the left and right of %s"
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
స్థిర పూర్ణాంకం generateOutputSubroutine(
  Parse *pParse,          /* Parsing context */
  Select *p,              /* The SELECT statement */
  SelectDest *p_లోపల,        /* Coroutine supplying data */
  SelectDest *pDest,      /* Where to send the data */
  పూర్ణాంకం regReturn,          /* The return address register */
  పూర్ణాంకం regPrev,            /* Previous result register.  No uniqueness if 0 */
  కీలకం_Info *p_కీలకం_Info,      /* For comparing with previous entry */
  పూర్ణాంకం iBreak              /* Jump here if we hit the LIMIT */
){
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం iContinue;
  పూర్ణాంకం addr;

  ప్రతిజ్ఞా( p_లోపల->eDest==SRT_Coroutine );

  addr = sqlite3VdbeCurrentAddr(v);
  iContinue = sqlite3VdbeMakeLabel(pParse);

  /* Suppress duplicates for UNION, EXCEPT, and INTERSECT
  */
  ఒకవేళ( regPrev ){
    పూర్ణాంకం addr1, addr2;
    addr1 = sqlite3_Vdbe_కూడు_Op1(v, OP_IfNot, regPrev); VdbeCoverage(v);
    addr2 = sqlite3_Vdbe_కూడు_Op4(v, OP_Compare, p_లోపల->iSdst, regPrev+1, p_లోపల->nSdst,
                              (అక్షరం*)sqlite3_కీలకం_Info_Ref(p_కీలకం_Info), P4_KEYINFO);
    sqlite3_Vdbe_కూడు_Op3(v, OP_Jump, addr2+2, iContinue, addr2+2); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ఇక్కడ(v, addr1);
    sqlite3_Vdbe_కూడు_Op3(v, OP_Copy, p_లోపల->iSdst, regPrev+1, p_లోపల->nSdst-1);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 1, regPrev);
  }
  ఒకవేళ( pParse->db->mallocFailed ) తిరిగి 0;

  /* Suppress the first OFFSET entries if there is an OFFSET clause
  */
  సంకేతం_స్థానభ్రంశం(v, p->i_స్థానభ్రంశం, iContinue);

  ఎంపిక( pDest->eDest ){
    /* Store the result as data using a unique key.
    */
    స్థితి SRT_Fifo:
    స్థితి SRT_DistFifo:
    స్థితి SRT_పట్టిక:
    స్థితి SRT_EphemTab: {
      పూర్ణాంకం r1 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
      పూర్ణాంకం r2 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
      పూర్ణాంకం iParm = pDest->iSDParm;
      testcase( pDest->eDest==SRT_పట్టిక );
      testcase( pDest->eDest==SRT_EphemTab );
      testcase( pDest->eDest==SRT_Fifo );
      testcase( pDest->eDest==SRT_DistFifo );
      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, p_లోపల->iSdst, p_లోపల->nSdst, r1);
#ఒకవేళ !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      ఒకవేళ( pDest->eDest==SRT_పట్టిక && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_CTE
      ఒకవేళ( pDest->eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index that is used to enforce uniqueness on the
        ** total result.  At this point, we are processing the setup portion
        ** of the recursive CTE using the merge algorithm, so the results are
        ** guaranteed to be unique anyhow.  But we still need to populate the
        ** (iParm+1) cursor for use by the subsequent recursive phase.
        */
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, iParm+1, r1,
                             p_లోపల->iSdst, p_లోపల->nSdst);
      }
#అయితే_ముగింపు
      sqlite3_Vdbe_కూడు_Op2(v, OP_NewRowid, iParm, r2);
      sqlite3_Vdbe_కూడు_Op3(v, OP_చొప్పించు, iParm, r1, r2);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      sqlite3_Release_తాత్కాలిక_Reg(pParse, r2);
      sqlite3_Release_తాత్కాలిక_Reg(pParse, r1);
      ఆపు;
    }

    /* If any row exist in the result set, record that fact and abort.
    */
    స్థితి SRT_Exists: {
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 1, pDest->iSDParm);
      /* The LIMIT clause will terminate the loop for us */
      ఆపు;
    }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)".
    */
    స్థితి SRT_సమితి: {
      పూర్ణాంకం r1;
      testcase( p_లోపల->nSdst>1 );
      r1 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
      sqlite3_Vdbe_కూడు_Op4(v, OP_Make_రికార్డు, p_లోపల->iSdst, p_లోపల->nSdst,
          r1, pDest->zAffSdst, p_లోపల->nSdst);
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, pDest->iSDParm, r1,
                           p_లోపల->iSdst, p_లోపల->nSdst);
      ఒకవేళ( pDest->iSDParm2>0 ){
        ప్రతిజ్ఞా( sqlite3CursorBloomable(pParse, pDest->iSDParm, p_లోపల->nSdst) );
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Filter_కూడు, pDest->iSDParm2, 0,
                             p_లోపల->iSdst, p_లోపల->nSdst);
        ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
      }
      sqlite3_Release_తాత్కాలిక_Reg(pParse, r1);
      ఆపు;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell and break out
    ** of the scan loop.  Note that the select might return multiple columns
    ** if it is the RHS of a row-value IN operator.
    */
    స్థితి SRT_Mem: {
      testcase( p_లోపల->nSdst>1 );
      sqlite3_సమాసం_సంకేతం_Move(pParse, p_లోపల->iSdst, pDest->iSDParm, p_లోపల->nSdst);
      /* The LIMIT clause will jump out of the loop for us */
      ఆపు;
    }
#అయితే_ముగింపు /* #ifndef SQLITE_OMIT_SUBQUERY */

    /* The results are stored in a sequence of registers
    ** starting at pDest->iSdst.  Then the co-routine yields.
    */
    స్థితి SRT_Coroutine: {
      ఒకవేళ( pDest->iSdst==0 ){
        pDest->iSdst = sqlite3_Get_తాత్కాలిక_Range(pParse, p_లోపల->nSdst);
        pDest->nSdst = p_లోపల->nSdst;
      }
      sqlite3_సమాసం_సంకేతం_Move(pParse, p_లోపల->iSdst, pDest->iSdst, p_లోపల->nSdst);
      sqlite3_Vdbe_కూడు_Op1(v, OP_Yield, pDest->iSDParm);
      ఆపు;
    }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    స్థితి SRT_Dist_వరుస:
    స్థితి SRT_వరుస: {
      పూర్ణాంకం n_కీలకం;
      పూర్ణాంకం r1, r2, r3, ii;
      సమాసం_List *pSO;
      పూర్ణాంకం iParm = pDest->iSDParm;
      pSO = pDest->p_క్రమం_By;
      ప్రతిజ్ఞా( pSO );
      n_కీలకం = pSO->n_సమాసం;
      r1 = sqlite3_Get_తాత్కాలిక_Reg(pParse);
      r2 = sqlite3_Get_తాత్కాలిక_Range(pParse, n_కీలకం+2);
      r3 = r2+n_కీలకం+1;

      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, p_లోపల->iSdst, p_లోపల->nSdst, r3);
      ఒకవేళ( pDest->eDest==SRT_Dist_వరుస ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_Idx_చొప్పించు, iParm+1, r3);
      }
      కొరకు(ii=0; ii<n_కీలకం; ii++){
        sqlite3_Vdbe_కూడు_Op2(v, OP_SCopy,
                          p_లోపల->iSdst + pSO->a[ii].u.x.i_క్రమం_By_నిలువు - 1,
                          r2+ii);
      }
      sqlite3_Vdbe_కూడు_Op2(v, OP_Sequence, iParm, r2+n_కీలకం);
      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, r2, n_కీలకం+2, r1);
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, iParm, r1, r2, n_కీలకం+2);
      sqlite3_Release_తాత్కాలిక_Reg(pParse, r1);
      sqlite3_Release_తాత్కాలిక_Range(pParse, r2, n_కీలకం+2);
      ఆపు;
    }
#అయితే_ముగింపు /* SQLITE_OMIT_CTE */

    /* Ignore the output */
    స్థితి SRT_Discard: {
      ఆపు;
    }

    /* If none of the above, then the result destination must be
    ** SRT_Output.
    **
    ** For SRT_Output, results are stored in a sequence of registers. 
    ** Then the OP_ResultRow opcode is used to cause sqlite3_step() to
    ** return the next row of result.
    */
    అప్రమేయ: {
      ప్రతిజ్ఞా( pDest->eDest==SRT_Output );
      sqlite3_Vdbe_కూడు_Op2(v, OP_ఫలితం_అడ్డువరుస, p_లోపల->iSdst, p_లోపల->nSdst);
      ఆపు;
    }
  }

  /* Jump to the end of the loop if the LIMIT is reached.
  */
  ఒకవేళ( p->i_హద్దు ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_DecrJumpZero, p->i_హద్దు, iBreak); VdbeCoverage(v);
  }

  /* Generate the subroutine return
  */
  sqlite3VdbeResolveLabel(v, iContinue);
  sqlite3_Vdbe_కూడు_Op1(v, OP_Return, regReturn);

  తిరిగి addr;
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
#నిర్వచితం_కాకపోతే SQLITE_OMIT_COMPOUND_SELECT
స్థిర పూర్ణాంకం multi_Select_By_విలీనం(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  పూర్ణాంకం i, j;             /* Loop counters */
  Select *p_పూర్వ;       /* Another SELECT immediately to our left */
  Select *p_విభజించు;       /* Left-most SELECT in the right-hand group */
  పూర్ణాంకం nSelect;          /* Number of SELECT statements in the compound */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest destA;     /* Destination for coroutine A */
  SelectDest destB;     /* Destination for coroutine B */
  పూర్ణాంకం regAddrA;         /* Address register for select-A coroutine */
  పూర్ణాంకం regAddrB;         /* Address register for select-B coroutine */
  పూర్ణాంకం addrSelectA;      /* Address of the select-A coroutine */
  పూర్ణాంకం addrSelectB;      /* Address of the select-B coroutine */
  పూర్ణాంకం reg_బహిర్గతం_A;          /* Address register for the output-A subroutine */
  పూర్ణాంకం reg_బహిర్గతం_B;          /* Address register for the output-B subroutine */
  పూర్ణాంకం addr_బహిర్గతం_A;         /* Address of the output-A subroutine */
  పూర్ణాంకం addr_బహిర్గతం_B = 0;     /* Address of the output-B subroutine */
  పూర్ణాంకం addrEofA;         /* Address of the select-A-exhausted subroutine */
  పూర్ణాంకం addrEofA_noB;     /* Alternate addrEofA if B is uninitialized */
  పూర్ణాంకం addrEofB;         /* Address of the select-B-exhausted subroutine */
  పూర్ణాంకం addrAltB;         /* Address of the A<B subroutine */
  పూర్ణాంకం addrAeqB;         /* Address of the A==B subroutine */
  పూర్ణాంకం addrAgtB;         /* Address of the A>B subroutine */
  పూర్ణాంకం reg_హద్దు_A;        /* Limit register for select-A */
  పూర్ణాంకం reg_హద్దు_B;        /* Limit register for select-A */
  పూర్ణాంకం regPrev;          /* A range of registers to hold previous output */
  పూర్ణాంకం saved_హద్దు;       /* Saved value of p->iLimit */
  పూర్ణాంకం saved_స్థానభ్రంశం;      /* Saved value of p->iOffset */
  పూర్ణాంకం labelCmpr;        /* Label for the start of the merge algorithm */
  పూర్ణాంకం label_చివర;         /* Label for the end of the overall SELECT stmt */
  పూర్ణాంకం addr1;            /* Jump instructions that get retargeted */
  పూర్ణాంకం op;               /* One of TK_ALL, TK_UNION, TK_EXCEPT, TK_INTERSECT */
  కీలకం_Info *p_కీలకం_Dup = 0; /* Comparison information for duplicate removal */
  కీలకం_Info *p_కీలకం_విలీనం;   /* Comparison information for merging rows */
  sqlite3 *db;          /* Database connection */
  సమాసం_List *p_క్రమం_By;   /* The ORDER BY clause */
  పూర్ణాంకం n_క్రమం_By;         /* Number of terms in the ORDER BY clause */
  u32 *a_క్రమమార్పు;        /* Mapping from ORDER BY terms to result set columns */

  ప్రతిజ్ఞా( p->p_క్రమం_By!=0 );
  ప్రతిజ్ఞా( p_కీలకం_Dup==0 ); /* "Managed" code needs this.  Ticket #3382. */
  db = pParse->db;
  v = pParse->pVdbe;
  ప్రతిజ్ఞా( v!=0 );       /* Already thrown the error if VDBE alloc failed */
  label_చివర = sqlite3VdbeMakeLabel(pParse);
  labelCmpr = sqlite3VdbeMakeLabel(pParse);


  /* Patch up the ORDER BY clause
  */
  op = p->op; 
  ప్రతిజ్ఞా( p->p_పూర్వ->p_క్రమం_By==0 );
  p_క్రమం_By = p->p_క్రమం_By;
  ప్రతిజ్ఞా( p_క్రమం_By );
  n_క్రమం_By = p_క్రమం_By->n_సమాసం;

  /* For operators other than UNION ALL we have to make sure that
  ** the ORDER BY clause covers every term of the result set.  Add
  ** terms to the ORDER BY clause as necessary.  Do the same for UNION ALL
  ** if any operator to the left is not UNION ALL, so that the ORDER BY
  ** carries the collation of every column into that sub-compound.
  */
  కొరకు(p_పూర్వ=p; p_పూర్వ && p_పూర్వ->op==TK_ALL; p_పూర్వ=p_పూర్వ->p_పూర్వ){}
  ఒకవేళ( p_పూర్వ && p_పూర్వ->p_పూర్వ ){
    కొరకు(i=1; db->mallocFailed==0 && i<=p->pEList->n_సమాసం; i++){
      నిర్మాణం సమాసం_List_అంశం *p_అంశం;
      కొరకు(j=0, p_అంశం=p_క్రమం_By->a; j<n_క్రమం_By; j++, p_అంశం++){
        ప్రతిజ్ఞా( p_అంశం!=0 );
        ప్రతిజ్ఞా( p_అంశం->u.x.i_క్రమం_By_నిలువు>0 );
        ఒకవేళ( p_అంశం->u.x.i_క్రమం_By_నిలువు==i ) ఆపు;
      }
      ఒకవేళ( j==n_క్రమం_By ){
        సమాసం *pNew = sqlite3_సమాసం_Int32(db, i);
        ఒకవేళ( pNew==0 ) తిరిగి SQLITE_NOMEM_BKPT;
        p->p_క్రమం_By = p_క్రమం_By = sqlite3_సమాసం_List_Append(pParse, p_క్రమం_By, pNew);
        ఒకవేళ( p_క్రమం_By ) p_క్రమం_By->a[n_క్రమం_By++].u.x.i_క్రమం_By_నిలువు = (u16)i;
      }
    }
  }

  /* Compute the comparison permutation and keyinfo that is used with
  ** the permutation to determine if the next row of results comes
  ** from selectA or selectB.  Also add literal collations to the
  ** ORDER BY clause terms so that when selectA and selectB are
  ** evaluated, they use the correct collation.
  */
  a_క్రమమార్పు = sqlite3DbMallocRawNN(db, పరిమాణం(u32)*(n_క్రమం_By + 1));
  ఒకవేళ( a_క్రమమార్పు ){
    నిర్మాణం సమాసం_List_అంశం *p_అంశం;
    పూర్ణాంకం bKeep = 0;
    a_క్రమమార్పు[0] = n_క్రమం_By;
    కొరకు(i=1, p_అంశం=p_క్రమం_By->a; i<=n_క్రమం_By; i++, p_అంశం++){
      ప్రతిజ్ఞా( p_అంశం!=0 );
      ప్రతిజ్ఞా( p_అంశం->u.x.i_క్రమం_By_నిలువు>0 );
      ప్రతిజ్ఞా( p_అంశం->u.x.i_క్రమం_By_నిలువు<=p->pEList->n_సమాసం );
      a_క్రమమార్పు[i] = p_అంశం->u.x.i_క్రమం_By_నిలువు - 1;
      ఒకవేళ( a_క్రమమార్పు[i]!=(u32)i-1 ) bKeep = 1;
    }
     ఒకవేళ( bKeep==0 ){
      sqlite3DbFreeNN(db, a_క్రమమార్పు);
      a_క్రమమార్పు = 0;
    }
  }
  p_కీలకం_విలీనం = multi_Select_By_విలీనం_కీలకం_Info(pParse, p, 1);

  /* Allocate a range of temporary registers and the KeyInfo needed
  ** for the logic that removes duplicate result rows when the
  ** operator is UNION, EXCEPT, or INTERSECT (but not UNION ALL).
  */
  ఒకవేళ( op==TK_ALL ){
    regPrev = 0;
  }లేకపోతే{
    పూర్ణాంకం n_సమాసం = p->pEList->n_సమాసం;
    ప్రతిజ్ఞా( n_క్రమం_By>=n_సమాసం || db->mallocFailed );
    regPrev = pParse->nMem+1;
    pParse->nMem += n_సమాసం+1;
    sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, regPrev);
    p_కీలకం_Dup = sqlite3_కీలకం_Info_Alloc(db, n_సమాసం, 1);
    ఒకవేళ( p_కీలకం_Dup ){
      ప్రతిజ్ఞా( sqlite3_కీలకం_Info_Is_Writeable(p_కీలకం_Dup) );
      /* Use the ORDER BY collations, not multiSelectCollSeq(p), because
      ** p might be just one part of a larger compound, in which case
      ** the ORDER BY carries the COLLATE of the larger compound. */
      కొరకు(i=0; ALWAYS(p_కీలకం_విలీనం) && i<n_క్రమం_By; i++){
        p_కీలకం_Dup->aColl[p_క్రమం_By->a[i].u.x.i_క్రమం_By_నిలువు-1] = p_కీలకం_విలీనం->aColl[i];
      }
    }
  }

  /* Separate the left and the right query from one another
  */
  nSelect = 1;
  ఒకవేళ( (op==TK_ALL || op==TK_UNION)
   && OptimizationEnabled(db, SQLITE_సమతుల్య_విలీనం)
  ){
    కొరకు(p_విభజించు=p; p_విభజించు->p_పూర్వ!=0 && p_విభజించు->op==op; p_విభజించు=p_విభజించు->p_పూర్వ){
      nSelect++;
      ప్రతిజ్ఞా( p_విభజించు->p_పూర్వ->p_తదుపరి==p_విభజించు );
    }
  }
  ఒకవేళ( nSelect<=3 ){
    p_విభజించు = p;
  }లేకపోతే{
    p_విభజించు = p;
    కొరకు(i=2; i<nSelect; i+=2){ p_విభజించు = p_విభజించు->p_పూర్వ; }
  }
  p_పూర్వ = p_విభజించు->p_పూర్వ;
  ప్రతిజ్ఞా( p_పూర్వ!=0 );
  p_విభజించు->p_పూర్వ = 0;
  p_పూర్వ->p_తదుపరి = 0;
  ప్రతిజ్ఞా( p->p_క్రమం_By == p_క్రమం_By );
  ప్రతిజ్ఞా( p_క్రమం_By!=0 || db->mallocFailed );
  p_పూర్వ->p_క్రమం_By = sqlite3_సమాసం_List_Dup(pParse->db, p_క్రమం_By, 0);
  sqlite3_Resolve_క్రమం_సమూహం_By(pParse, p, p->p_క్రమం_By, "ORDER");
  sqlite3_Resolve_క్రమం_సమూహం_By(pParse, p_పూర్వ, p_పూర్వ->p_క్రమం_By, "ORDER");

  /* Compute the limit registers */
  compute_హద్దు_Registers(pParse, p, label_చివర);
  ఒకవేళ( p->i_హద్దు && (op==TK_ALL || union_With_హద్దు_One(db,p)) ){
    reg_హద్దు_A = ++pParse->nMem;
    reg_హద్దు_B = ++pParse->nMem;
    sqlite3_Vdbe_కూడు_Op2(v, OP_Copy, p->i_స్థానభ్రంశం ? p->i_స్థానభ్రంశం+1 : p->i_హద్దు,
                                  reg_హద్దు_A);
    sqlite3_Vdbe_కూడు_Op2(v, OP_Copy, reg_హద్దు_A, reg_హద్దు_B);
  }లేకపోతే{
    reg_హద్దు_A = reg_హద్దు_B = 0;
  }
  sqlite3_సమాసం_Delete(db, p->p_హద్దు);
  p->p_హద్దు = 0;

  regAddrA = ++pParse->nMem;
  regAddrB = ++pParse->nMem;
  reg_బహిర్గతం_A = ++pParse->nMem;
  reg_బహిర్గతం_B = ++pParse->nMem;
  sqlite3SelectDestInit(&destA, SRT_Coroutine, regAddrA);
  sqlite3SelectDestInit(&destB, SRT_Coroutine, regAddrB);

  ExplainQueryPlan((pParse, 1, "MERGE (%s)", sqlite3SelectOpName(p->op)));

  /* Generate a coroutine to evaluate the SELECT statement to the
  ** left of the compound operator - the "A" select.
  */
  addrSelectA = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3_Vdbe_కూడు_Op3(v, OP_InitCoroutine, regAddrA, 0, addrSelectA);
  VdbeComment((v, "SUBR: next-A"));
  p_పూర్వ->i_హద్దు = reg_హద్దు_A;
  ExplainQueryPlan((pParse, 1, "LEFT"));
  sqlite3Select(pParse, p_పూర్వ, &destA);
  sqlite3_Vdbe_చివర_Coroutine(v, regAddrA);
  sqlite3_Vdbe_Jump_ఇక్కడ(v, addr1);

  /* Generate a coroutine to evaluate the SELECT statement on
  ** the right - the "B" select
  */
  addrSelectB = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3_Vdbe_కూడు_Op3(v, OP_InitCoroutine, regAddrB, 0, addrSelectB);
  VdbeComment((v, "SUBR: next-B"));
  saved_హద్దు = p->i_హద్దు;
  saved_స్థానభ్రంశం = p->i_స్థానభ్రంశం;
  p->i_హద్దు = reg_హద్దు_B;
  p->i_స్థానభ్రంశం = 0; 
  ExplainQueryPlan((pParse, 1, "RIGHT"));
  sqlite3Select(pParse, p, &destB);
  p->i_హద్దు = saved_హద్దు;
  p->i_స్థానభ్రంశం = saved_స్థానభ్రంశం;
  sqlite3_Vdbe_చివర_Coroutine(v, regAddrB);

  /* Generate a subroutine that outputs the current row of the A
  ** select as the next output row of the compound select.
  */
  VdbeNoopComment((v, "SUBR: out-A"));
  addr_బహిర్గతం_A = generateOutputSubroutine(pParse,
                 p, &destA, pDest, reg_బహిర్గతం_A,
                 regPrev, p_కీలకం_Dup, label_చివర);
 
  /* Generate a subroutine that outputs the current row of the B
  ** select as the next output row of the compound select.
  */
  ఒకవేళ( op==TK_ALL || op==TK_UNION ){
    VdbeNoopComment((v, "SUBR: out-B"));
    addr_బహిర్గతం_B = generateOutputSubroutine(pParse,
                 p, &destB, pDest, reg_బహిర్గతం_B,
                 regPrev, p_కీలకం_Dup, label_చివర);
  }
  sqlite3_కీలకం_Info_Unref(p_కీలకం_Dup);

  /* Generate a subroutine to run when the results from select A
  ** are exhausted and only data in select B remains.
  */
  ఒకవేళ( op==TK_EXCEPT || op==TK_INTERSECT ){
    addrEofA_noB = addrEofA = label_చివర;
  }లేకపోతే{ 
    VdbeNoopComment((v, "SUBR: eof-A"));
    addrEofA = sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, reg_బహిర్గతం_B, addr_బహిర్గతం_B);
    VdbeComment((v, "out-B"));
    addrEofA_noB = sqlite3_Vdbe_కూడు_Op2(v, OP_Yield, regAddrB, label_చివర);
                                     VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, addrEofA);
    p->n_Select_అడ్డువరుస = sqlite3_Log_Est_కూడు(p->n_Select_అడ్డువరుస, p_పూర్వ->n_Select_అడ్డువరుస);
  }

  /* Generate a subroutine to run when the results from select B
  ** are exhausted and only data in select A remains.
  */
  ఒకవేళ( op==TK_INTERSECT ){
    addrEofB = addrEofA;
    ఒకవేళ( p->n_Select_అడ్డువరుస > p_పూర్వ->n_Select_అడ్డువరుస ) p->n_Select_అడ్డువరుస = p_పూర్వ->n_Select_అడ్డువరుస;
  }లేకపోతే{ 
    VdbeNoopComment((v, "SUBR: eof-B"));
    addrEofB = sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, reg_బహిర్గతం_A, addr_బహిర్గతం_A);
    VdbeComment((v, "out-A"));
    sqlite3_Vdbe_కూడు_Op2(v, OP_Yield, regAddrA, label_చివర); VdbeCoverage(v);
    VdbeComment((v, "next-A"));
    sqlite3VdbeGoto(v, addrEofB);
  }

  /* Generate code to handle the case of A<B
  */
  addrAltB = sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, reg_బహిర్గతం_A, addr_బహిర్గతం_A);
  VdbeComment((v, "out-A"));
  sqlite3_Vdbe_కూడు_Op2(v, OP_Yield, regAddrA, addrEofA); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  sqlite3VdbeGoto(v, labelCmpr);

  /* Generate code to handle the case of A==B
  */
  ఒకవేళ( op==TK_ALL ){
    addrAeqB = addrAltB;
  }లేకపోతే ఒకవేళ( op==TK_INTERSECT ){
    addrAeqB = addrAltB;
    addrAltB++;
  }లేకపోతే{
    addrAeqB = addrAltB + 1;
  }

  /* Generate code to handle the case of A>B
  */
  addrAgtB = sqlite3VdbeCurrentAddr(v);
  ఒకవేళ( op==TK_ALL || op==TK_UNION ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, reg_బహిర్గతం_B, addr_బహిర్గతం_B);
    VdbeComment((v, "out-B"));
    sqlite3_Vdbe_కూడు_Op2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, labelCmpr);
  }లేకపోతే{
    addrAgtB++; /* Just do next-B.  Might as well use the next-B call
                ** in the next code block */
  }

  /* This code runs once to initialize everything.
  */
  sqlite3_Vdbe_Jump_ఇక్కడ(v, addr1);
  sqlite3_Vdbe_కూడు_Op2(v, OP_Yield, regAddrA, addrEofA_noB); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  /* v---  Also the A>B case for EXCEPT and INTERSECT */
  sqlite3_Vdbe_కూడు_Op2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
  VdbeComment((v, "next-B"));

  /* Implement the main merge loop
  */
  ఒకవేళ( a_క్రమమార్పు!=0 ){
    sqlite3_Vdbe_కూడు_Op4(v, OP_Permutation, 0, 0, 0, (అక్షరం*)a_క్రమమార్పు, P4_INTARRAY);
  }
  sqlite3VdbeResolveLabel(v, labelCmpr);
  sqlite3_Vdbe_కూడు_Op4(v, OP_Compare, destA.iSdst, destB.iSdst, n_క్రమం_By,
                         (అక్షరం*)p_కీలకం_విలీనం, P4_KEYINFO);
  ఒకవేళ( a_క్రమమార్పు!=0 ){
    sqlite3VdbeChangeP5(v, OPFLAG_క్రమమార్పు);
  }
  sqlite3_Vdbe_కూడు_Op3(v, OP_Jump, addrAltB, addrAeqB, addrAgtB);
  VdbeCoverageIf(v, op==TK_ALL);
  VdbeCoverageIf(v, op==TK_UNION);
  VdbeCoverageIf(v, op==TK_EXCEPT);
  VdbeCoverageIf(v, op==TK_INTERSECT);

  /* Jump to the this point in order to terminate the query.
  */
  sqlite3VdbeResolveLabel(v, label_చివర);

  /* Make arrangements to free the 2nd and subsequent arms of the compound
  ** after the parse has finished */
  ఒకవేళ( p_విభజించు->p_పూర్వ ){
    sqlite3_Parser_కూడు_Cleanup(pParse, sqlite3SelectDeleteGeneric, p_విభజించు->p_పూర్వ);
  }
  p_విభజించు->p_పూర్వ = p_పూర్వ;
  p_పూర్వ->p_తదుపరి = p_విభజించు;
  sqlite3_సమాసం_List_Delete(db, p_పూర్వ->p_క్రమం_By);
  p_పూర్వ->p_క్రమం_By = 0;

  /*** TBD:  Insert subroutine calls to close cursors on incomplete
  **** subqueries ****/
  Explain_Query_Plan_తీయి(pParse);
  తిరిగి pParse->nErr!=0;
}
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)

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
రకనిర్వచనం నిర్మాణం SubstContext {
  Parse *pParse;            /* The parsing context */
  పూర్ణాంకం i_పట్టిక;               /* Replace references to this table */
  పూర్ణాంకం i_New_పట్టిక;            /* New table number */
  పూర్ణాంకం isOuterJoin;          /* Add TK_IF_NULL_ROW opcodes on each replacement */
  పూర్ణాంకం n_Sel_లోతు;            /* Depth of sub-query recursion.  Top==1 */
  సమాసం_List *pEList;         /* Replacement expressions */
  సమాసం_List *pCList;         /* Collation sequences for replacement expr */
} SubstContext;

/* Forward Declarations */
స్థిర శూన్య subst_సమాసం_List(SubstContext*, సమాసం_List*);
స్థిర శూన్య substSelect(SubstContext*, Select*, పూర్ణాంకం);

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
స్థిర సమాసం *subst_సమాసం(
  SubstContext *pSubst,  /* Description of the substitution */
  సమాసం *p_సమాసం            /* Expr in which substitution occurs */
){
  ఒకవేళ( p_సమాసం==0 ) తిరిగి 0;
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_OuterON|EP_InnerON)
   && p_సమాసం->w.iJoin==pSubst->i_పట్టిక
  ){
    testcase( సమాసం_Has_Property(p_సమాసం, EP_InnerON) );
    p_సమాసం->w.iJoin = pSubst->i_New_పట్టిక;
  }
  ఒకవేళ( p_సమాసం->op==TK_స్తంభం
   && p_సమాసం->i_పట్టిక==pSubst->i_పట్టిక
   && !సమాసం_Has_Property(p_సమాసం, EP_Fixed_నిలువు)
  ){
#నిర్వచితమైతే SQLITE_ALLOW_ROWID_లోపల_VIEW
    ఒకవేళ( p_సమాసం->i_స్తంభం<0 ){
      p_సమాసం->op = TK_NULL;
    }లేకపోతే
#అయితే_ముగింపు
    {
      సమాసం *pNew;
      పూర్ణాంకం i_స్తంభం;
      సమాసం *pCopy;
      సమాసం if_Null_అడ్డువరుస;
      i_స్తంభం = p_సమాసం->i_స్తంభం;
      ప్రతిజ్ఞా( i_స్తంభం>=0 );
      ప్రతిజ్ఞా( pSubst->pEList!=0 && i_స్తంభం<pSubst->pEList->n_సమాసం );
      ప్రతిజ్ఞా( p_సమాసం->p_కుడి==0 );
      pCopy = pSubst->pEList->a[i_స్తంభం].p_సమాసం;
      ఒకవేళ( sqlite3_సమాసం_Is_సదిశ(pCopy) ){
        sqlite3_సదిశ_Error_సందేశం(pSubst->pParse, pCopy);
      }లేకపోతే{
        sqlite3 *db = pSubst->pParse->db;
        ఒకవేళ( pSubst->isOuterJoin
         && (pCopy->op!=TK_స్తంభం || pCopy->i_పట్టిక!=pSubst->i_New_పట్టిక)
        ){
          memset(&if_Null_అడ్డువరుస, 0, పరిమాణం(if_Null_అడ్డువరుస));
          if_Null_అడ్డువరుస.op = TK_IF_NULL_అడ్డువరుస;
          if_Null_అడ్డువరుస.p_ఎడమ = pCopy;
          if_Null_అడ్డువరుస.i_పట్టిక = pSubst->i_New_పట్టిక;
          if_Null_అడ్డువరుస.i_స్తంభం = -99;
          if_Null_అడ్డువరుస.flags = EP_If_Null_అడ్డువరుస;
          pCopy = &if_Null_అడ్డువరుస;
        }
        testcase( సమాసం_Has_Property(pCopy, EP_Subquery) );
        pNew = sqlite3_సమాసం_Dup(db, pCopy, 0);
        ఒకవేళ( db->mallocFailed ){
          sqlite3_సమాసం_Delete(db, pNew);
          తిరిగి p_సమాసం;
        }
        ఒకవేళ( pSubst->isOuterJoin ){
          సమాసం_సమితి_Property(pNew, EP_CanBeNull);
        }
        ఒకవేళ( pNew->op==TK_TRUEFALSE ){
          pNew->u.i_విలువ = sqlite3_సమాసం_Truth_విలువ(pNew);
          pNew->op = TK_INTEGER;
          సమాసం_సమితి_Property(pNew, EP_Int_విలువ);
        }

        /* Ensure that the expression now has an implicit collation sequence,
        ** just as it did when it was a column of a view or sub-query. */
        {
          CollSeq *pNat = sqlite3_సమాసం_Coll_Seq(pSubst->pParse, pNew);
          CollSeq *pColl = sqlite3_సమాసం_Coll_Seq(pSubst->pParse,
                pSubst->pCList->a[i_స్తంభం].p_సమాసం
          );
          ఒకవేళ( pNat!=pColl || (pNew->op!=TK_స్తంభం && pNew->op!=TK_COLLATE) ){
            pNew = sqlite3_సమాసం_కూడు_Collate_String(pSubst->pParse, pNew,
                (pColl ? pColl->zName : "BINARY")
            );
          }
        }
        సమాసం_Clear_Property(pNew, EP_Collate);
        ఒకవేళ( సమాసం_Has_Property(p_సమాసం,EP_OuterON|EP_InnerON) ){
          sqlite3_సమితి_Join_సమాసం(pNew, p_సమాసం->w.iJoin,
                             p_సమాసం->flags & (EP_OuterON|EP_InnerON));
        }
        sqlite3_సమాసం_Delete(db, p_సమాసం);
        p_సమాసం = pNew;
      }
    }
  }లేకపోతే{
    ఒకవేళ( p_సమాసం->op==TK_IF_NULL_అడ్డువరుస && p_సమాసం->i_పట్టిక==pSubst->i_పట్టిక ){
      p_సమాసం->i_పట్టిక = pSubst->i_New_పట్టిక;
    }
    ఒకవేళ( p_సమాసం->op==TK_AGG_FUNCTION && p_సమాసం->op2>=pSubst->n_Sel_లోతు ){
      p_సమాసం->op2--;
    }
    p_సమాసం->p_ఎడమ = subst_సమాసం(pSubst, p_సమాసం->p_ఎడమ);
    p_సమాసం->p_కుడి = subst_సమాసం(pSubst, p_సమాసం->p_కుడి);
    ఒకవేళ( సమాసం_Use_XSelect(p_సమాసం) ){
      substSelect(pSubst, p_సమాసం->x.pSelect, 1);
    }లేకపోతే{
      subst_సమాసం_List(pSubst, p_సమాసం->x.pList);
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_WinFunc) ){
      కిటికీ *pWin = p_సమాసం->y.pWin;
      pWin->pFilter = subst_సమాసం(pSubst, pWin->pFilter);
      subst_సమాసం_List(pSubst, pWin->p_విభజన);
      subst_సమాసం_List(pSubst, pWin->p_క్రమం_By);
    }
#అయితే_ముగింపు
  }
  తిరిగి p_సమాసం;
}
స్థిర శూన్య subst_సమాసం_List(
  SubstContext *pSubst, /* Description of the substitution */
  సమాసం_List *pList       /* List to scan and in which to make substitutes */
){
  పూర్ణాంకం i;
  ఒకవేళ( pList==0 ) తిరిగి;
  కొరకు(i=0; i<pList->n_సమాసం; i++){
    pList->a[i].p_సమాసం = subst_సమాసం(pSubst, pList->a[i].p_సమాసం);
  }
}
స్థిర శూన్య substSelect(
  SubstContext *pSubst, /* Description of the substitution */
  Select *p,            /* SELECT statement in which to make substitutions */
  పూర్ణాంకం do_పూర్వ           /* Do substitutes on p->pPrior too */
){
  SrcList *pSrc;
  Src_అంశం *p_అంశం;
  పూర్ణాంకం i;
  ఒకవేళ( !p ) తిరిగి;
  pSubst->n_Sel_లోతు++;
  చేయి{
    subst_సమాసం_List(pSubst, p->pEList);
    subst_సమాసం_List(pSubst, p->p_సమూహం_By);
    subst_సమాసం_List(pSubst, p->p_క్రమం_By);
    p->pHaving = subst_సమాసం(pSubst, p->pHaving);
    p->pWhere = subst_సమాసం(pSubst, p->pWhere);
    pSrc = p->pSrc;
    ప్రతిజ్ఞా( pSrc!=0 );
    కొరకు(i=pSrc->nSrc, p_అంశం=pSrc->a; i>0; i--, p_అంశం++){
      ఒకవేళ( p_అంశం->fg.isSubquery ){
        substSelect(pSubst, p_అంశం->u4.pSubq->pSelect, 1);
      }
      ఒకవేళ( p_అంశం->fg.isTabFunc ){
        subst_సమాసం_List(pSubst, p_అంశం->u1.pFuncArg);
      }
    }
  }అయితే( do_పూర్వ && (p = p->p_పూర్వ)!=0 );
  pSubst->n_Sel_లోతు--;
}
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ఒకవేళ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
/*
** pSelect is a SELECT statement and pSrcItem is one item in the FROM
** clause of that SELECT.
**
** This routine scans the entire SELECT statement and recomputes the
** pSrcItem->colUsed mask.
*/
స్థిర పూర్ణాంకం recompute_Columns_వాడిన_సమాసం(Walker *pWalker, సమాసం *p_సమాసం){
  Src_అంశం *p_అంశం;
  ఒకవేళ( p_సమాసం->op!=TK_స్తంభం ) తిరిగి WRC_Continue;
  p_అంశం = pWalker->u.p_Src_అంశం;
  ఒకవేళ( p_అంశం->iCursor!=p_సమాసం->i_పట్టిక ) తిరిగి WRC_Continue;
  ఒకవేళ( p_సమాసం->i_స్తంభం<0 ) తిరిగి WRC_Continue;
  p_అంశం->నిలువు_వాడిన |= sqlite3_సమాసం_నిలువు_వాడిన(p_సమాసం);
  తిరిగి WRC_Continue;
}
స్థిర శూన్య recompute_Columns_వాడిన(
  Select *pSelect,                 /* The complete SELECT statement */
  Src_అంశం *p_Src_అంశం                /* Which FROM clause item to recompute */
){
  Walker w;
  ఒకవేళ( NEVER(p_Src_అంశం->pSTab==0) ) తిరిగి;
  memset(&w, 0, పరిమాణం(w));
  w.x_సమాసం_Callback = recompute_Columns_వాడిన_సమాసం;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.p_Src_అంశం = p_Src_అంశం;
  p_Src_అంశం->నిలువు_వాడిన = 0;
  sqlite3WalkSelect(&w, pSelect);
}
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ఒకవేళ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
స్థిర శూన్య srclistRenumberCursors(
  Parse *pParse,                  /* Parse context */
  పూర్ణాంకం *aCsrMap,                   /* Array to store cursor mappings in */
  SrcList *pSrc,                  /* FROM clause to renumber */
  పూర్ణాంకం iExcept                     /* FROM clause item to skip */
){
  పూర్ణాంకం i;
  Src_అంశం *p_అంశం;
  కొరకు(i=0, p_అంశం=pSrc->a; i<pSrc->nSrc; i++, p_అంశం++){
    ఒకవేళ( i!=iExcept ){
      Select *p;
      ప్రతిజ్ఞా( p_అంశం->iCursor < aCsrMap[0] );
      ఒకవేళ( !p_అంశం->fg.isRecursive || aCsrMap[p_అంశం->iCursor+1]==0 ){
        aCsrMap[p_అంశం->iCursor+1] = pParse->nTab++;
      }
      p_అంశం->iCursor = aCsrMap[p_అంశం->iCursor+1];
      ఒకవేళ( p_అంశం->fg.isSubquery ){
        కొరకు(p=p_అంశం->u4.pSubq->pSelect; p; p=p->p_పూర్వ){
          srclistRenumberCursors(pParse, aCsrMap, p->pSrc, -1);
        }
      }
    }
  }
}

/*
** *piCursor is a cursor number.  Change it if it needs to be mapped.
*/
స్థిర శూన్య renumberCursorDoMapping(Walker *pWalker, పూర్ణాంకం *piCursor){
  పూర్ణాంకం *aCsrMap = pWalker->u.ai_నిలువు;
  పూర్ణాంకం iCsr = *piCursor;
  ఒకవేళ( iCsr < aCsrMap[0] && aCsrMap[iCsr+1]>0 ){
    *piCursor = aCsrMap[iCsr+1];
  }
}

/*
** Expression walker callback used by renumberCursors() to update
** Expr objects to match newly assigned cursor numbers.
*/
స్థిర పూర్ణాంకం renumberCursorsCb(Walker *pWalker, సమాసం *p_సమాసం){
  పూర్ణాంకం op = p_సమాసం->op;
  ఒకవేళ( op==TK_స్తంభం || op==TK_IF_NULL_అడ్డువరుస ){
    renumberCursorDoMapping(pWalker, &p_సమాసం->i_పట్టిక);
  }
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, (EP_OuterON|EP_InnerON)) ){
    testcase( సమాసం_Has_Property(p_సమాసం, EP_OuterON) );
    testcase( సమాసం_Has_Property(p_సమాసం, EP_InnerON) );
    renumberCursorDoMapping(pWalker, &p_సమాసం->w.iJoin);
  }
  తిరిగి WRC_Continue;
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
స్థిర శూన్య renumberCursors(
  Parse *pParse,                  /* Parse context */
  Select *p,                      /* Select to renumber cursors within */
  పూర్ణాంకం iExcept,                    /* FROM clause item to skip */
  పూర్ణాంకం *aCsrMap                    /* Working space */
){
  Walker w;
  srclistRenumberCursors(pParse, aCsrMap, p->pSrc, iExcept);
  memset(&w, 0, పరిమాణం(w));
  w.u.ai_నిలువు = aCsrMap;
  w.x_సమాసం_Callback = renumberCursorsCb;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  sqlite3WalkSelect(&w, p);
}
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** If pSel is not part of a compound SELECT, return a pointer to its
** expression list. Otherwise, return a pointer to the expression list
** of the leftmost SELECT in the compound.
*/
స్థిర సమాసం_List *కనుగొను_Leftmost_Exprlist(Select *pSel){
  అయితే( pSel->p_పూర్వ ){
    pSel = pSel->p_పూర్వ;
  }
  తిరిగి pSel->pEList;
}

/*
** Return true if any of the result-set columns in the compound query
** have incompatible affinities on one or more arms of the compound.
*/
స్థిర పూర్ణాంకం compoundHasDifferentAffinities(Select *p){
  పూర్ణాంకం ii;
  సమాసం_List *pList;
  ప్రతిజ్ఞా( p!=0 );
  ప్రతిజ్ఞా( p->pEList!=0 );
  ప్రతిజ్ఞా( p->p_పూర్వ!=0 );
  pList = p->pEList;
  కొరకు(ii=0; ii<pList->n_సమాసం; ii++){
    అక్షరం aff;
    Select *pSub1;
    ప్రతిజ్ఞా( pList->a[ii].p_సమాసం!=0 );
    aff = sqlite3_సమాసం_Affinity(pList->a[ii].p_సమాసం);
    కొరకు(pSub1=p->p_పూర్వ; pSub1; pSub1=pSub1->p_పూర్వ){
      ప్రతిజ్ఞా( pSub1->pEList!=0 );
      ప్రతిజ్ఞా( pSub1->pEList->n_సమాసం>ii );
      ప్రతిజ్ఞా( pSub1->pEList->a[ii].p_సమాసం!=0 );
      ఒకవేళ( sqlite3_సమాసం_Affinity(pSub1->pEList->a[ii].p_సమాసం)!=aff ){
        తిరిగి 1;
      }
    }
  }
  తిరిగి 0;
}

#ఒకవేళ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
స్థిర పూర్ణాంకం flattenSubquery(
  Parse *pParse,       /* Parsing context */
  Select *p,           /* The parent or outer SELECT statement */
  పూర్ణాంకం iFrom,           /* Index in p->pSrc->a[] of the inner subquery */
  పూర్ణాంకం isAgg            /* True if outer SELECT uses aggregate functions */
){
  స్థిరరాశి అక్షరం *zSavedAuthContext = pParse->zAuthContext;
  Select *p_మాతృ;    /* Current UNION ALL term of the other query */
  Select *pSub;       /* The inner query or "subquery" */
  Select *pSub1;      /* Pointer to the rightmost select in sub-query */
  SrcList *pSrc;      /* The FROM clause of the outer query */
  SrcList *pSubSrc;   /* The FROM clause of the subquery */
  పూర్ణాంకం i_మాతృ;        /* VDBE cursor number of the pSub result set temp table */
  పూర్ణాంకం i_New_మాతృ = -1;/* Replacement table for iParent */
  పూర్ణాంకం isOuterJoin = 0; /* True if pSub is the right side of a LEFT JOIN */   
  పూర్ణాంకం i;              /* Loop counter */
  సమాసం *pWhere;                    /* The WHERE clause */
  Src_అంశం *pSubitem;               /* The subquery */
  sqlite3 *db = pParse->db;
  Walker w;                        /* Walker to persist agginfo data */
  పూర్ణాంకం *aCsrMap = 0;

  /* Check to see if flattening is permitted.  Return 0 if not.
  */
  ప్రతిజ్ఞా( p!=0 );
  ప్రతిజ్ఞా( p->p_పూర్వ==0 );
  ఒకవేళ( OptimizationDisabled(db, SQLITE_QueryFlattener) ) తిరిగి 0;
  pSrc = p->pSrc;
  ప్రతిజ్ఞా( pSrc && iFrom>=0 && iFrom<pSrc->nSrc );
  pSubitem = &pSrc->a[iFrom];
  i_మాతృ = pSubitem->iCursor;
  ప్రతిజ్ఞా( pSubitem->fg.isSubquery );
  pSub = pSubitem->u4.pSubq->pSelect;
  ప్రతిజ్ఞా( pSub!=0 );

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
  ఒకవేళ( p->pWin || pSub->pWin ) తిరిగి 0;                  /* Restriction (25) */
#అయితే_ముగింపు

  pSubSrc = pSub->pSrc;
  ప్రతిజ్ఞా( pSubSrc );
  /* Prior to version 3.1.2, when LIMIT and OFFSET had to be simple constants,
  ** not arbitrary expressions, we allowed some combining of LIMIT and OFFSET
  ** because they could be computed at compile-time.  But when LIMIT and OFFSET
  ** became arbitrary expressions, we were forced to add restrictions (13)
  ** and (14). */
  ఒకవేళ( pSub->p_హద్దు && p->p_హద్దు ) తిరిగి 0;              /* Restriction (13) */
  ఒకవేళ( pSub->p_హద్దు && pSub->p_హద్దు->p_కుడి ) తిరిగి 0;   /* Restriction (14) */
  ఒకవేళ( (p->selFlags & SF_Compound)!=0 && pSub->p_హద్దు ){
    తిరిగి 0;                                            /* Restriction (15) */
  }
  ఒకవేళ( pSubSrc->nSrc==0 ) తిరిగి 0;                       /* Restriction (7)  */
  ఒకవేళ( pSub->selFlags & SF_Distinct ) తిరిగి 0;           /* Restriction (4)  */
  ఒకవేళ( pSub->p_హద్దు && (pSrc->nSrc>1 || isAgg) ){
     తిరిగి 0;         /* Restrictions (8)(9) */
  }
  ఒకవేళ( p->p_క్రమం_By && pSub->p_క్రమం_By ){
     తిరిగి 0;                                           /* Restriction (11) */
  }
  ఒకవేళ( isAgg && pSub->p_క్రమం_By ) తిరిగి 0;                /* Restriction (16) */
  ఒకవేళ( pSub->p_హద్దు && p->pWhere ) తిరిగి 0;              /* Restriction (19) */
  ఒకవేళ( pSub->p_హద్దు && (p->selFlags & SF_Distinct)!=0 ){
     తిరిగి 0;         /* Restriction (21) */
  }
  ఒకవేళ( pSub->selFlags & (SF_Recursive) ){
    తిరిగి 0; /* Restrictions (22) */
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
  ఒకవేళ( (pSubitem->fg.jointype & (JT_OUTER|JT_LTORJ))!=0 ){
    ఒకవేళ( pSubSrc->nSrc>1                        /* (3a) */
     /**** || IsVirtual(pSubSrc->a[0].pSTab)      (3b)-omitted */
     || (p->selFlags & SF_Distinct)!=0         /* (3d) */
     || (pSubitem->fg.jointype & JT_కుడి)!=0  /* (26) */
    ){
      తిరిగి 0;
    }
    isOuterJoin = 1;
  }

  ప్రతిజ్ఞా( pSubSrc->nSrc>0 );  /* True by restriction (7) */
  ఒకవేళ( iFrom>0 && (pSubSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
    తిరిగి 0;   /* Restriction (27a) */
  }

  /* Condition (28) is blocked by the caller */
  ప్రతిజ్ఞా( !pSubitem->fg.isCte || pSubitem->u2.pCteUse->eM10d!=M10d_Yes );

  /* Restriction (17): If the sub-query is a compound SELECT, then it must
  ** use only the UNION ALL operator. And none of the simple select queries
  ** that make up the compound SELECT are allowed to be aggregate or distinct
  ** queries.
  */
  ఒకవేళ( pSub->p_పూర్వ ){
    పూర్ణాంకం ii;
    ఒకవేళ( pSub->p_క్రమం_By ){
      తిరిగి 0;  /* Restriction (20) */
    }
    ఒకవేళ( isAgg || (p->selFlags & SF_Distinct)!=0 || isOuterJoin>0 ){
      తిరిగి 0; /* (17d1), (17d2), or (17f) */
    }
    కొరకు(pSub1=pSub; pSub1; pSub1=pSub1->p_పూర్వ){
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct );
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Aggregate );
      ప్రతిజ్ఞా( pSub->pSrc!=0 );
      ప్రతిజ్ఞా( (pSub->selFlags & SF_Recursive)==0 );
      ప్రతిజ్ఞా( pSub->pEList->n_సమాసం==pSub1->pEList->n_సమాసం );
      ఒకవేళ( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))!=0    /* (17b) */
       || (pSub1->p_పూర్వ && pSub1->op!=TK_ALL)                 /* (17a) */
       || pSub1->pSrc->nSrc<1                                  /* (17c) */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
       || pSub1->pWin                                          /* (17e) */
#అయితే_ముగింపు
      ){
        తిరిగి 0;
      }
      ఒకవేళ( iFrom>0 && (pSub1->pSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
        /* Without this restriction, the JT_LTORJ flag would end up being
        ** omitted on left-hand tables of the right join that is being
        ** flattened. */
        తిరిగి 0;   /* Restrictions (17g), (27b) */
      }
      testcase( pSub1->pSrc->nSrc>1 );
    }

    /* Restriction (18). */
    ఒకవేళ( p->p_క్రమం_By ){
      కొరకు(ii=0; ii<p->p_క్రమం_By->n_సమాసం; ii++){
        ఒకవేళ( p->p_క్రమం_By->a[ii].u.x.i_క్రమం_By_నిలువు==0 ) తిరిగి 0;
      }
    }

    /* Restriction (23) */
    ఒకవేళ( (p->selFlags & SF_Recursive) ) తిరిగి 0;

    /* Restriction (17h) */
    ఒకవేళ( compoundHasDifferentAffinities(pSub) ) తిరిగి 0;

    ఒకవేళ( pSrc->nSrc>1 ){
      ఒకవేళ( pParse->nSelect>500 ) తిరిగి 0;
      ఒకవేళ( OptimizationDisabled(db, SQLITE_FlttnUnionAll) ) తిరిగి 0;
      aCsrMap = sqlite3DbMallocZero(db, ((i64)pParse->nTab+1)*పరిమాణం(పూర్ణాంకం));
      ఒకవేళ( aCsrMap ) aCsrMap[0] = pParse->nTab;
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
  
  ఒకవేళ( ALWAYS(pSubitem->fg.isSubquery) ){
    pSub1 = sqlite3SubqueryDetach(db, pSubitem);
  }లేకపోతే{
    pSub1 = 0;
  }
  ప్రతిజ్ఞా( pSubitem->fg.isSubquery==0 );
  ప్రతిజ్ఞా( pSubitem->fg.fixedSchema==0 );
  sqlite3DbFree(db, pSubitem->zName);
  sqlite3DbFree(db, pSubitem->zAlias);
  pSubitem->zName = 0;
  pSubitem->zAlias = 0;
  ప్రతిజ్ఞా( pSubitem->fg.isUsing!=0 || pSubitem->u3.pOn==0 );

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
  కొరకు(pSub=pSub->p_పూర్వ; pSub; pSub=pSub->p_పూర్వ){
    Select *pNew;
    సమాసం_List *p_క్రమం_By = p->p_క్రమం_By;
    సమాసం *p_హద్దు = p->p_హద్దు;
    Select *p_పూర్వ = p->p_పూర్వ;
    పట్టిక *p_అంశం_Tab = pSubitem->pSTab;
    pSubitem->pSTab = 0;
    p->p_క్రమం_By = 0;
    p->p_పూర్వ = 0;
    p->p_హద్దు = 0;
    pNew = sqlite3SelectDup(db, p, 0);
    p->p_హద్దు = p_హద్దు;
    p->p_క్రమం_By = p_క్రమం_By;
    p->op = TK_ALL;
    pSubitem->pSTab = p_అంశం_Tab;
    ఒకవేళ( pNew==0 ){
      p->p_పూర్వ = p_పూర్వ;
    }లేకపోతే{
      pNew->selId = ++pParse->nSelect;
      ఒకవేళ( aCsrMap && ALWAYS(db->mallocFailed==0) ){
        renumberCursors(pParse, pNew, iFrom, aCsrMap);
      }
      pNew->p_పూర్వ = p_పూర్వ;
      ఒకవేళ( p_పూర్వ ) p_పూర్వ->p_తదుపరి = pNew;
      pNew->p_తదుపరి = p;
      p->p_పూర్వ = pNew;
      TREETRACE(0x4,pParse,p,("compound-subquery flattener"
                              " creates %u as peer\n",pNew->selId));
    }
    ప్రతిజ్ఞా( pSubitem->fg.isSubquery==0 );
  }
  sqlite3DbFree(db, aCsrMap);
  ఒకవేళ( db->mallocFailed ){
    ప్రతిజ్ఞా( pSubitem->fg.fixedSchema==0 );
    ప్రతిజ్ఞా( pSubitem->fg.isSubquery==0 );
    ప్రతిజ్ఞా( pSubitem->u4.zDatabase==0 );
    sqlite3_Src_అంశం_Attach_Subquery(pParse, pSubitem, pSub1, 0);
    తిరిగి 1;
  }

  /* Defer deleting the Table object associated with the
  ** subquery until code generation is
  ** complete, since there may still exist Expr.pTab entries that
  ** refer to the subquery even after flattening.  Ticket #3346.
  **
  ** pSubitem->pSTab is always non-NULL by test restrictions and tests above.
  */
  ఒకవేళ( ALWAYS(pSubitem->pSTab!=0) ){
    పట్టిక *p_Tab_గమ్యం_Del = pSubitem->pSTab;
    ఒకవేళ( p_Tab_గమ్యం_Del->nTabRef==1 ){
      Parse *pToplevel = sqlite3ParseToplevel(pParse);
      sqlite3_Parser_కూడు_Cleanup(pToplevel, sqlite3_Delete_పట్టిక_Generic, p_Tab_గమ్యం_Del);
      testcase( pToplevel->earlyCleanup );
    }లేకపోతే{
      p_Tab_గమ్యం_Del->nTabRef--;
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
  కొరకు(p_మాతృ=p; p_మాతృ; p_మాతృ=p_మాతృ->p_పూర్వ, pSub=pSub->p_పూర్వ){
    పూర్ణాంకం nSubSrc;
    u8 jointype = p_మాతృ->pSrc->a[iFrom].fg.jointype;
    ప్రతిజ్ఞా( pSub!=0 );
    pSubSrc = pSub->pSrc;     /* FROM clause of subquery */
    nSubSrc = pSubSrc->nSrc;  /* Number of terms in subquery FROM clause */
    pSrc = p_మాతృ->pSrc;     /* FROM clause of the outer query */

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
    ఒకవేళ( nSubSrc>1 ){
      pSrc = sqlite3SrcListEnlarge(pParse, pSrc, nSubSrc-1,iFrom+1);
      ఒకవేళ( pSrc==0 ) ఆపు;
      p_మాతృ->pSrc = pSrc;
    }

    /* Transfer the FROM clause terms from the subquery into the
    ** outer query.
    */
    i_New_మాతృ = pSubSrc->a[0].iCursor;
    కొరకు(i=0; i<nSubSrc; i++){
      Src_అంశం *p_అంశం = &pSrc->a[i+iFrom];
      ప్రతిజ్ఞా( p_అంశం->fg.isTabFunc==0 );
      ప్రతిజ్ఞా( p_అంశం->fg.isSubquery
           || p_అంశం->fg.fixedSchema
           || p_అంశం->u4.zDatabase==0 );
      ఒకవేళ( p_అంశం->fg.isUsing ) sqlite3IdListDelete(db, p_అంశం->u3.pUsing);
      *p_అంశం = pSubSrc->a[i];
      p_అంశం->fg.jointype |= (jointype & JT_LTORJ);
      memset(&pSubSrc->a[i], 0, పరిమాణం(pSubSrc->a[i]));
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
    ఒకవేళ( pSub->p_క్రమం_By ){
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
      సమాసం_List *p_క్రమం_By = pSub->p_క్రమం_By;
      కొరకు(i=0; i<p_క్రమం_By->n_సమాసం; i++){
        p_క్రమం_By->a[i].u.x.i_క్రమం_By_నిలువు = 0;
      }
      ప్రతిజ్ఞా( p_మాతృ->p_క్రమం_By==0 );
      p_మాతృ->p_క్రమం_By = p_క్రమం_By;
      pSub->p_క్రమం_By = 0;
    }
    pWhere = pSub->pWhere;
    pSub->pWhere = 0;
    ఒకవేళ( isOuterJoin>0 ){
      ప్రతిజ్ఞా( pSubSrc->nSrc==1 );
      sqlite3_సమితి_Join_సమాసం(pWhere, i_New_మాతృ, EP_OuterON);
    }
    ఒకవేళ( pWhere ){
      ఒకవేళ( p_మాతృ->pWhere ){
        p_మాతృ->pWhere = sqlite3PExpr(pParse, TK_AND, pWhere, p_మాతృ->pWhere);
      }లేకపోతే{
        p_మాతృ->pWhere = pWhere;
      }
    }
    ఒకవేళ( db->mallocFailed==0 ){
      SubstContext x;
      x.pParse = pParse;
      x.i_పట్టిక = i_మాతృ;
      x.i_New_పట్టిక = i_New_మాతృ;
      x.isOuterJoin = isOuterJoin;
      x.n_Sel_లోతు = 0;
      x.pEList = pSub->pEList;
      x.pCList = కనుగొను_Leftmost_Exprlist(pSub);
      substSelect(&x, p_మాతృ, 0);
    }
 
    /* The flattened query is a compound if either the inner or the
    ** outer query is a compound. */
    p_మాతృ->selFlags |= pSub->selFlags & SF_Compound;
    ప్రతిజ్ఞా( (pSub->selFlags & SF_Distinct)==0 ); /* restriction (17b) */
 
    /*
    ** SELECT ... FROM (SELECT ... LIMIT a OFFSET b) LIMIT x OFFSET y;
    **
    ** One is tempted to try to add a and b to combine the limits.  But this
    ** does not work if either limit is negative.
    */
    ఒకవేళ( pSub->p_హద్దు ){
      p_మాతృ->p_హద్దు = pSub->p_హద్దు;
      pSub->p_హద్దు = 0;
    }

    /* Recompute the SrcItem.colUsed masks for the flattened
    ** tables. */
    కొరకు(i=0; i<nSubSrc; i++){
      recompute_Columns_వాడిన(p_మాతృ, &pSrc->a[i+iFrom]);
    }
  }

  /* Finally, delete what is left of the subquery and return success.
  */
  sqlite3AggInfoPersistWalkerInit(&w, pParse);
  sqlite3WalkSelect(&w,pSub1);
  sqlite3SelectDelete(db, pSub1);

#ఒకవేళ TREETRACE_ENABLED
  ఒకవేళ( sqlite3_వృక్షం_Trace & 0x4 ){
    TREETRACE(0x4,pParse,p,("After flattening:\n"));
    sqlite3_వృక్షం_View_Select(0, p, 0);
  }
#అయితే_ముగింపు

  తిరిగి 1;
}
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** A structure to keep track of all of the column values that are fixed to
** a known value due to WHERE clause constraints of the form COLUMN=VALUE.
*/
రకనిర్వచనం నిర్మాణం WhereConst WhereConst;
నిర్మాణం WhereConst {
  Parse *pParse;   /* Parsing context */
  u8 *pOomFault;   /* Pointer to pParse->db->mallocFailed */
  పూర్ణాంకం nConst;      /* Number for COLUMN=CONSTANT terms */
  పూర్ణాంకం nChng;       /* Number of times a constant is propagated */
  పూర్ణాంకం bHasAffBlob; /* At least one column in apExpr[] as affinity BLOB */
  u32 mExcludeOn;  /* Which ON expressions to exclude from considertion.
                   ** Either EP_OuterON or EP_InnerON|EP_OuterON */
  సమాసం **ap_సమాసం;   /* [i*2] is COLUMN and [i*2+1] is VALUE */
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
స్థిర శూన్య const_చొప్పించు(
  WhereConst *pConst,  /* The WhereConst into which we are inserting */
  సమాసం *p_స్తంభం,       /* The COLUMN part of the constraint */
  సమాసం *p_విలువ,        /* The VALUE part of the constraint */
  సమాసం *p_సమాసం          /* Overall expression: COLUMN=VALUE or VALUE=COLUMN */
){
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( p_స్తంభం->op==TK_స్తంభం );
  ప్రతిజ్ఞా( sqlite3_సమాసం_Is_Constant(pConst->pParse, p_విలువ) );
  ఒకవేళ( సమాసం_Has_Property(p_స్తంభం, EP_Fixed_నిలువు) ) తిరిగి;
  ఒకవేళ( sqlite3_సమాసం_Affinity(p_విలువ)!=0 ) తిరిగి;
  ఒకవేళ( !sqlite3_Is_ద్విమాన(sqlite3_సమాసం_Compare_Coll_Seq(pConst->pParse,p_సమాసం)) ){
    తిరిగి;
  }
  ఒకవేళ( sqlite3_సమాసం_Can_Return_Subtype(pConst->pParse, p_విలువ) ) తిరిగి;

  /* 2018-10-25 ticket [cf5ed20f]
  ** Make sure the same pColumn is not inserted more than once */
  కొరకు(i=0; i<pConst->nConst; i++){
    స్థిరరాశి సమాసం *pE2 = pConst->ap_సమాసం[i*2];
    ప్రతిజ్ఞా( pE2->op==TK_స్తంభం );
    ఒకవేళ( pE2->i_పట్టిక==p_స్తంభం->i_పట్టిక
     && pE2->i_స్తంభం==p_స్తంభం->i_స్తంభం
    ){
      తిరిగి;  /* Already present.  Return without doing anything. */
    }
  }
  ప్రతిజ్ఞా( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  ఒకవేళ( sqlite3_సమాసం_Affinity(p_స్తంభం)<=SQLITE_AFF_BLOB ){
    pConst->bHasAffBlob = 1;
  }

  pConst->nConst++;
  pConst->ap_సమాసం = sqlite3DbReallocOrFree(pConst->pParse->db, pConst->ap_సమాసం,
                         pConst->nConst*2*పరిమాణం(సమాసం*));
  ఒకవేళ( pConst->ap_సమాసం==0 ){
    pConst->nConst = 0;
  }లేకపోతే{
    pConst->ap_సమాసం[pConst->nConst*2-2] = p_స్తంభం;
    pConst->ap_సమాసం[pConst->nConst*2-1] = p_విలువ;
  }
}

/*
** Find all terms of COLUMN=VALUE or VALUE=COLUMN in pExpr where VALUE
** is a constant expression and where the term must be true because it
** is part of the AND-connected terms of the expression.  For each term
** found, add it to the pConst structure.
*/
స్థిర శూన్య కనుగొను_Const_లోపల_Where(WhereConst *pConst, సమాసం *p_సమాసం){
  సమాసం *p_కుడి, *p_ఎడమ;
  ఒకవేళ( NEVER(p_సమాసం==0) ) తిరిగి;
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, pConst->mExcludeOn) ){
    testcase( సమాసం_Has_Property(p_సమాసం, EP_OuterON) );
    testcase( సమాసం_Has_Property(p_సమాసం, EP_InnerON) );
    తిరిగి;
  }
  ఒకవేళ( p_సమాసం->op==TK_AND ){
    కనుగొను_Const_లోపల_Where(pConst, p_సమాసం->p_కుడి);
    కనుగొను_Const_లోపల_Where(pConst, p_సమాసం->p_ఎడమ);
    తిరిగి;
  }
  ఒకవేళ( p_సమాసం->op!=TK_EQ ) తిరిగి;
  p_కుడి = p_సమాసం->p_కుడి;
  p_ఎడమ = p_సమాసం->p_ఎడమ;
  ప్రతిజ్ఞా( p_కుడి!=0 );
  ప్రతిజ్ఞా( p_ఎడమ!=0 );
  ఒకవేళ( p_కుడి->op==TK_స్తంభం && sqlite3_సమాసం_Is_Constant(pConst->pParse, p_ఎడమ) ){
    const_చొప్పించు(pConst,p_కుడి,p_ఎడమ,p_సమాసం);
  }
  ఒకవేళ( p_ఎడమ->op==TK_స్తంభం && sqlite3_సమాసం_Is_Constant(pConst->pParse, p_కుడి) ){
    const_చొప్పించు(pConst,p_ఎడమ,p_కుడి,p_సమాసం);
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
స్థిర పూర్ణాంకం propagate_Constant_సమాసం_Rewrite_One(
  WhereConst *pConst,
  సమాసం *p_సమాసం,
  పూర్ణాంకం bIgnoreAffBlob
){
  పూర్ణాంకం i;
  ఒకవేళ( pConst->pOomFault[0] ) తిరిగి WRC_Prune;
  ఒకవేళ( p_సమాసం->op!=TK_స్తంభం ) తిరిగి WRC_Continue;
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Fixed_నిలువు|pConst->mExcludeOn) ){
    testcase( సమాసం_Has_Property(p_సమాసం, EP_Fixed_నిలువు) );
    testcase( సమాసం_Has_Property(p_సమాసం, EP_OuterON) );
    testcase( సమాసం_Has_Property(p_సమాసం, EP_InnerON) );
    తిరిగి WRC_Continue;
  }
  కొరకు(i=0; i<pConst->nConst; i++){
    సమాసం *p_స్తంభం = pConst->ap_సమాసం[i*2];
    ఒకవేళ( p_స్తంభం==p_సమాసం ) కొనసాగు;
    ఒకవేళ( p_స్తంభం->i_పట్టిక!=p_సమాసం->i_పట్టిక ) కొనసాగు;
    ఒకవేళ( p_స్తంభం->i_స్తంభం!=p_సమాసం->i_స్తంభం ) కొనసాగు;
    ప్రతిజ్ఞా( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
    ఒకవేళ( bIgnoreAffBlob && sqlite3_సమాసం_Affinity(p_స్తంభం)<=SQLITE_AFF_BLOB ){
      ఆపు;
    }
    /* A match is found.  Add the EP_FixedCol property */
    pConst->nChng++;
    సమాసం_Clear_Property(p_సమాసం, EP_ఆకు);
    సమాసం_సమితి_Property(p_సమాసం, EP_Fixed_నిలువు);
    ప్రతిజ్ఞా( p_సమాసం->p_ఎడమ==0 );
    p_సమాసం->p_ఎడమ = sqlite3_సమాసం_Dup(pConst->pParse->db, pConst->ap_సమాసం[i*2+1], 0);
    ఒకవేళ( pConst->pParse->db->mallocFailed ) తిరిగి WRC_Prune;
    ఆపు;
  }
  తిరిగి WRC_Prune;
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
స్థిర పూర్ణాంకం propagate_Constant_సమాసం_Rewrite(Walker *pWalker, సమాసం *p_సమాసం){
  WhereConst *pConst = pWalker->u.pConst;
  ప్రతిజ్ఞా( TK_GT==TK_EQ+1 );
  ప్రతిజ్ఞా( TK_LE==TK_EQ+2 );
  ప్రతిజ్ఞా( TK_LT==TK_EQ+3 );
  ప్రతిజ్ఞా( TK_GE==TK_EQ+4 );
  ఒకవేళ( pConst->bHasAffBlob ){
    ఒకవేళ( (p_సమాసం->op>=TK_EQ && p_సమాసం->op<=TK_GE)
     || p_సమాసం->op==TK_IS
    ){
      propagate_Constant_సమాసం_Rewrite_One(pConst, p_సమాసం->p_ఎడమ, 0);
      ఒకవేళ( pConst->pOomFault[0] ) తిరిగి WRC_Prune;
      ఒకవేళ( sqlite3_సమాసం_Affinity(p_సమాసం->p_ఎడమ)!=SQLITE_AFF_పాఠ్యం ){
        propagate_Constant_సమాసం_Rewrite_One(pConst, p_సమాసం->p_కుడి, 0);
      }
    }
  }
  తిరిగి propagate_Constant_సమాసం_Rewrite_One(pConst, p_సమాసం, pConst->bHasAffBlob);
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
స్థిర పూర్ణాంకం propagateConstants(
  Parse *pParse,   /* The parsing context */
  Select *p        /* The query in which to propagate constants */
){
  WhereConst x;
  Walker w;
  పూర్ణాంకం nChng = 0;
  x.pParse = pParse;
  x.pOomFault = &pParse->db->mallocFailed;
  చేయి{
    x.nConst = 0;
    x.nChng = 0;
    x.ap_సమాసం = 0;
    x.bHasAffBlob = 0;
    ఒకవేళ( ALWAYS(p->pSrc!=0)
     && p->pSrc->nSrc>0
     && (p->pSrc->a[0].fg.jointype & JT_LTORJ)!=0
    ){
      /* Do not propagate constants on any ON clause if there is a
      ** RIGHT JOIN anywhere in the query */
      x.mExcludeOn = EP_InnerON | EP_OuterON;
    }లేకపోతే{
      /* Do not propagate constants through the ON clause of a LEFT JOIN */
      x.mExcludeOn = EP_OuterON;
    }
    కనుగొను_Const_లోపల_Where(&x, p->pWhere);
    ఒకవేళ( x.nConst ){
      memset(&w, 0, పరిమాణం(w));
      w.pParse = pParse;
      w.x_సమాసం_Callback = propagate_Constant_సమాసం_Rewrite;
      w.xSelectCallback = sqlite3SelectWalkNoop;
      w.xSelectCallback2 = 0;
      w.walker_లోతు = 0;
      w.u.pConst = &x;
      sqlite3_Walk_సమాసం(&w, p->pWhere);
      sqlite3DbFree(x.pParse->db, x.ap_సమాసం);
      nChng += x.nChng;
    }
  }అయితే( x.nChng ); 
  తిరిగి nChng;
}

#ఒకవేళ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
# ఒకవేళ !defined(SQLITE_OMIT_WINDOWFUNC)
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
స్థిర పూర్ణాంకం నెట్టు_కిందికి_కిటికీ_Check(Parse *pParse, Select *pSubq, సమాసం *p_సమాసం){
  ప్రతిజ్ఞా( pSubq->pWin->p_విభజన );
  ప్రతిజ్ఞా( (pSubq->selFlags & SF_MultiPart)==0 );
  ప్రతిజ్ఞా( pSubq->p_పూర్వ==0 );
  తిరిగి sqlite3_సమాసం_Is_Constant_Or_సమూహం_By(pParse, p_సమాసం, pSubq->pWin->p_విభజన);
}
# endif /* SQLITE_OMIT_WINDOWFUNC */
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ఒకవేళ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
స్థిర పూర్ణాంకం నెట్టు_కిందికి_Where_Terms(
  Parse *pParse,        /* Parse context (for malloc() and error reporting) */
  Select *pSubq,        /* The subquery whose WHERE clause is to be augmented */
  సమాసం *pWhere,         /* The WHERE clause of the outer query */
  SrcList *pSrcList,    /* The complete from clause of the outer query */
  పూర్ణాంకం iSrc              /* Which FROM clause term to try to push into  */
){
  సమాసం *pNew;
  Src_అంశం *pSrc;        /* The subquery FROM term into which WHERE is pushed */
  పూర్ణాంకం nChng = 0;
  pSrc = &pSrcList->a[iSrc];
  ఒకవేళ( pWhere==0 ) తిరిగి 0;
  ఒకవేళ( pSubq->selFlags & (SF_Recursive|SF_MultiPart) ){
    తిరిగి 0;           /* restrictions (2) and (11) */
  }
  ఒకవేళ( pSrc->fg.jointype & (JT_LTORJ|JT_కుడి) ){
    తిరిగి 0;           /* restrictions (10) */
  }

  ఒకవేళ( pSubq->p_పూర్వ ){
    Select *pSel;
    పూర్ణాంకం notUnionAll = 0;
    కొరకు(pSel=pSubq; pSel; pSel=pSel->p_పూర్వ){
      u8 op = pSel->op;
      ప్రతిజ్ఞా( op==TK_ALL || op==TK_SELECT
           || op==TK_UNION || op==TK_INTERSECT || op==TK_EXCEPT );
      ఒకవేళ( op!=TK_ALL && op!=TK_SELECT ){
        notUnionAll = 1;
      }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
      ఒకవేళ( pSel->pWin ) తిరిగి 0;    /* restriction (6b) */
#అయితే_ముగింపు
    }
    ఒకవేళ( notUnionAll ){
      /* If any of the compound arms are connected using UNION, INTERSECT,
      ** or EXCEPT, then we must ensure that none of the columns use a
      ** non-BINARY collating sequence. */
      కొరకు(pSel=pSubq; pSel; pSel=pSel->p_పూర్వ){
        పూర్ణాంకం ii;
        స్థిరరాశి సమాసం_List *pList = pSel->pEList;
        ప్రతిజ్ఞా( pList!=0 );
        కొరకు(ii=0; ii<pList->n_సమాసం; ii++){
          CollSeq *pColl = sqlite3_సమాసం_Coll_Seq(pParse, pList->a[ii].p_సమాసం);
          ఒకవేళ( !sqlite3_Is_ద్విమాన(pColl) ){
            తిరిగి 0;  /* Restriction (8) */
          }
        }
      }
    }
  }లేకపోతే{
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    ఒకవేళ( pSubq->pWin && pSubq->pWin->p_విభజన==0 ) తిరిగి 0;
#అయితే_ముగింపు
  }

#నిర్వచితమైతే SQLITE_DEBUG
  /* Only the first term of a compound can have a WITH clause.  But make
  ** sure no other terms are marked SF_Recursive in case something changes
  ** in the future.
  */
  {
    Select *pX; 
    కొరకు(pX=pSubq; pX; pX=pX->p_పూర్వ){
      ప్రతిజ్ఞా( (pX->selFlags & (SF_Recursive))==0 );
    }
  }
#అయితే_ముగింపు

  ఒకవేళ( pSubq->p_హద్దు!=0 ){
    తిరిగి 0; /* restriction (3) */
  }
  అయితే( pWhere->op==TK_AND ){
    nChng += నెట్టు_కిందికి_Where_Terms(pParse, pSubq, pWhere->p_కుడి, pSrcList, iSrc);
    pWhere = pWhere->p_ఎడమ;
  }

#ఒకవేళ 0 /* These checks now done by sqlite3ExprIsSingleTableConstraint() */
  ఒకవేళ( సమాసం_Has_Property(pWhere, EP_OuterON|EP_InnerON) /* (9a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (9c) */
  ){
    పూర్ణాంకం jj;
    కొరకు(jj=0; jj<iSrc; jj++){
      ఒకవేళ( pWhere->w.iJoin==pSrcList->a[jj].iCursor ){
        /* If we reach this point, both (9a) and (9b) are satisfied.
        ** The following loop checks (9c):
        */
        కొరకు(jj++; jj<iSrc; jj++){
          ఒకవేళ( (pSrcList->a[jj].fg.jointype & JT_కుడి)!=0 ){
            తిరిగి 0;  /* restriction (9) */
          }
        }
      }
    }
  }
  ఒకవేళ( is_ఎడమ_Join
   && (సమాసం_Has_Property(pWhere,EP_OuterON)==0
         || pWhere->w.iJoin!=iCursor)
  ){
    తిరిగి 0; /* restriction (4) */
  }
  ఒకవేళ( సమాసం_Has_Property(pWhere,EP_OuterON)
   && pWhere->w.iJoin!=iCursor
  ){
    తిరిగి 0; /* restriction (5) */
  }
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ALLOW_ROWID_లోపల_VIEW
  ఒకవేళ( ViewCanHaveRowid && (pWhere->op==TK_ISNULL || pWhere->op==TK_NOTNULL) ){
    సమాసం *p_ఎడమ = pWhere->p_ఎడమ;
    ఒకవేళ( ALWAYS(p_ఎడమ) 
     && p_ఎడమ->op==TK_స్తంభం
     && p_ఎడమ->i_స్తంభం < 0
    ){
      తిరిగి 0;  /* Restriction (12) */
    }
  }
#అయితే_ముగింపు

  ఒకవేళ( sqlite3_సమాసం_Is_Single_పట్టిక_Constraint(pWhere, pSrcList, iSrc, 1) ){
    nChng++;
    pSubq->selFlags |= SF_నెట్టు_కిందికి;
    అయితే( pSubq ){
      SubstContext x;
      pNew = sqlite3_సమాసం_Dup(pParse->db, pWhere, 0);
      unset_Join_సమాసం(pNew, -1, 1);
      x.pParse = pParse;
      x.i_పట్టిక = pSrc->iCursor;
      x.i_New_పట్టిక = pSrc->iCursor;
      x.isOuterJoin = 0;
      x.n_Sel_లోతు = 0;
      x.pEList = pSubq->pEList;
      x.pCList = కనుగొను_Leftmost_Exprlist(pSubq);
      pNew = subst_సమాసం(&x, pNew);
      ప్రతిజ్ఞా( pNew!=0 || pParse->nErr!=0 );
      ఒకవేళ( pParse->nErr==0 && pNew->op==TK_లోపల && సమాసం_Use_XSelect(pNew) ){
        ప్రతిజ్ఞా( pNew->x.pSelect!=0 );
        pNew->x.pSelect->selFlags |= SF_Cloned_Rhs_లోపల;
        ప్రతిజ్ఞా( pWhere!=0 );
        ప్రతిజ్ఞా( pWhere->op==TK_లోపల );
        ప్రతిజ్ఞా( సమాసం_Use_XSelect(pWhere) );
        ప్రతిజ్ఞా( pWhere->x.pSelect!=0 );
        pWhere->x.pSelect->selFlags |= SF_Cloned_Rhs_లోపల;
      }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
      ఒకవేళ( pSubq->pWin && 0==నెట్టు_కిందికి_కిటికీ_Check(pParse, pSubq, pNew) ){
        /* Restriction 6c has prevented push-down in this case */
        sqlite3_సమాసం_Delete(pParse->db, pNew);
        nChng--;
        ఆపు;
      }
#అయితే_ముగింపు
      ఒకవేళ( pSubq->selFlags & SF_Aggregate ){
        pSubq->pHaving = sqlite3_సమాసం_And(pParse, pSubq->pHaving, pNew);
      }లేకపోతే{
        pSubq->pWhere = sqlite3_సమాసం_And(pParse, pSubq->pWhere, pNew);
      }
      pSubq = pSubq->p_పూర్వ;
    }
  }
  తిరిగి nChng;
}
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** Check to see if a subquery contains result-set columns that are
** never used.  If it does, change the value of those result-set columns
** to NULL so that they do not cause unnecessary work to compute.
**
** Return the number of column that were changed to NULL.
*/
స్థిర పూర్ణాంకం disable_వాడని_Subquery_ఫలితం_Columns(Src_అంశం *p_అంశం){
  పూర్ణాంకం n_నిలువు;
  Select *pSub;      /* The subquery to be simplified */
  Select *pX;        /* For looping over compound elements of pSub */
  పట్టిక *pTab;       /* The table that describes the subquery */
  పూర్ణాంకం j;             /* Column number */
  పూర్ణాంకం nChng = 0;     /* Number of columns converted to NULL */
  Bitmask నిలువు_వాడిన;   /* Columns that may not be NULLed out */

  ప్రతిజ్ఞా( p_అంశం!=0 );
  ఒకవేళ( p_అంశం->fg.isCorrelated || p_అంశం->fg.isCte ){
    తిరిగి 0;
  }
  ప్రతిజ్ఞా( p_అంశం->pSTab!=0 );
  pTab = p_అంశం->pSTab;
  ప్రతిజ్ఞా( p_అంశం->fg.isSubquery );
  pSub = p_అంశం->u4.pSubq->pSelect;
  ప్రతిజ్ఞా( pSub->pEList->n_సమాసం==pTab->n_నిలువు );
  కొరకు(pX=pSub; pX; pX=pX->p_పూర్వ){
    ఒకవేళ( (pX->selFlags & (SF_Distinct|SF_Aggregate))!=0 ){
      testcase( pX->selFlags & SF_Distinct );
      testcase( pX->selFlags & SF_Aggregate );
      తిరిగి 0;
    }
    ఒకవేళ( pX->p_పూర్వ && pX->op!=TK_ALL ){
      /* This optimization does not work for compound subqueries that
      ** use UNION, INTERSECT, or EXCEPT.  Only UNION ALL is allowed. */
      తిరిగి 0;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    ఒకవేళ( pX->pWin ){
      /* This optimization does not work for subqueries that use window
      ** functions. */
      తిరిగి 0;
    }
#అయితే_ముగింపు
  }
  నిలువు_వాడిన = p_అంశం->నిలువు_వాడిన;
  ఒకవేళ( pSub->p_క్రమం_By ){
    సమాసం_List *pList = pSub->p_క్రమం_By;
    కొరకు(j=0; j<pList->n_సమాసం; j++){
      u16 i_నిలువు = pList->a[j].u.x.i_క్రమం_By_నిలువు;
      ఒకవేళ( i_నిలువు>0 ){
        i_నిలువు--;
        నిలువు_వాడిన |= ((Bitmask)1)<<(i_నిలువు>=BMS ? BMS-1 : i_నిలువు);
      }
    }
  }
  n_నిలువు = pTab->n_నిలువు;
  కొరకు(j=0; j<n_నిలువు; j++){
    Bitmask m = j<BMS-1 ? MASKBIT(j) : TOPBIT;
    ఒకవేళ( (m & నిలువు_వాడిన)!=0 ) కొనసాగు;
    కొరకు(pX=pSub; pX; pX=pX->p_పూర్వ) {
      సమాసం *pY = pX->pEList->a[j].p_సమాసం;
      ఒకవేళ( pY->op==TK_NULL ) కొనసాగు;
      pY->op = TK_NULL;
      సమాసం_Clear_Property(pY, EP_Skip|EP_Unlikely);
      pX->selFlags |= SF_నెట్టు_కిందికి;
      nChng++;
    }
  }
  తిరిగి nChng;
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
స్థిర u8 minMaxQuery(sqlite3 *db, సమాసం *pFunc, సమాసం_List **ppMinMax){
  పూర్ణాంకం eRet = WHERE_ORDERBY_సాధారణ;      /* Return value */
  సమాసం_List *pEList;                     /* Arguments to agg function */
  స్థిరరాశి అక్షరం *zFunc;                    /* Name of aggregate function pFunc */
  సమాసం_List *p_క్రమం_By;
  u8 క్రమీకరించు_Flags = 0;

  ప్రతిజ్ఞా( *ppMinMax==0 );
  ప్రతిజ్ఞా( pFunc->op==TK_AGG_FUNCTION );
  ప్రతిజ్ఞా( !Is_కిటికీ_Func(pFunc) );
  ప్రతిజ్ఞా( సమాసం_Use_XList(pFunc) );
  pEList = pFunc->x.pList;
  ఒకవేళ( pEList==0
   || pEList->n_సమాసం!=1
   || సమాసం_Has_Property(pFunc, EP_WinFunc)
   || OptimizationDisabled(db, SQLITE_MinMaxOpt)
  ){
    తిరిగి eRet;
  }
  ప్రతిజ్ఞా( !సమాసం_Has_Property(pFunc, EP_Int_విలువ) );
  zFunc = pFunc->u.zToken;
  ఒకవేళ( sqlite3StrICmp(zFunc, "min")==0 ){
    eRet = WHERE_ORDERBY_MIN;
    ఒకవేళ( sqlite3_సమాసం_Can_Be_Null(pEList->a[0].p_సమాసం) ){
      క్రమీకరించు_Flags = KEYINFO_క్రమం_BIGNULL;
    }
  }లేకపోతే ఒకవేళ( sqlite3StrICmp(zFunc, "max")==0 ){
    eRet = WHERE_ORDERBY_MAX;
    క్రమీకరించు_Flags = KEYINFO_క్రమం_DESC;
  }లేకపోతే{
    తిరిగి eRet;
  }
  *ppMinMax = p_క్రమం_By = sqlite3_సమాసం_List_Dup(db, pEList, 0);
  ప్రతిజ్ఞా( p_క్రమం_By!=0 || db->mallocFailed );
  ఒకవేళ( p_క్రమం_By ) p_క్రమం_By->a[0].fg.క్రమీకరించు_Flags = క్రమీకరించు_Flags;
  తిరిగి eRet;
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
స్థిర పట్టిక *is_Simple_లెక్క(Select *p, AggInfo *pAggInfo){
  పట్టిక *pTab;
  సమాసం *p_సమాసం;

  ప్రతిజ్ఞా( !p->p_సమూహం_By );

  ఒకవేళ( p->pWhere
   || p->pEList->n_సమాసం!=1
   || p->pSrc->nSrc!=1
   || p->pSrc->a[0].fg.isSubquery
   || pAggInfo->nFunc!=1
   || p->pHaving
  ){
    తిరిగి 0;
  }
  pTab = p->pSrc->a[0].pSTab;
  ప్రతిజ్ఞా( pTab!=0 );
  ప్రతిజ్ఞా( !IsView(pTab) );
  ఒకవేళ( !Is_Ordinary_పట్టిక(pTab) ) తిరిగి 0;
  p_సమాసం = p->pEList->a[0].p_సమాసం;
  ప్రతిజ్ఞా( p_సమాసం!=0 );
  ఒకవేళ( p_సమాసం->op!=TK_AGG_FUNCTION ) తిరిగి 0;
  ఒకవేళ( p_సమాసం->pAggInfo!=pAggInfo ) తిరిగి 0;
  ఒకవేళ( (pAggInfo->aFunc[0].pFunc->funcFlags&SQLITE_FUNC_లెక్క)==0 ) తిరిగి 0;
  ప్రతిజ్ఞా( pAggInfo->aFunc[0].pFExpr==p_సమాసం );
  testcase( సమాసం_Has_Property(p_సమాసం, EP_Distinct) );
  testcase( సమాసం_Has_Property(p_సమాసం, EP_WinFunc) );
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Distinct|EP_WinFunc) ) తిరిగి 0;

  తిరిగి pTab;
}

/*
** If the source-list item passed as an argument was augmented with an
** INDEXED BY clause, then try to locate the specified index. If there
** was such a clause and the named index cannot be found, return
** SQLITE_ERROR and leave an error in pParse. Otherwise, populate
** pFrom->pIndex and return SQLITE_OK.
*/
పూర్ణాంకం sqlite3_సూచీకృత_By_Lookup(Parse *pParse, Src_అంశం *pFrom){
  పట్టిక *pTab = pFrom->pSTab;
  అక్షరం *z_సూచీకృత_By = pFrom->u1.z_సూచీకృత_By;
  సూచీ *pIdx;
  ప్రతిజ్ఞా( pTab!=0 );
  ప్రతిజ్ఞా( pFrom->fg.is_సూచీకృత_By!=0 );

  కొరకు(pIdx=pTab->p_సూచీ;
      pIdx && sqlite3StrICmp(pIdx->zName, z_సూచీకృత_By);
      pIdx=pIdx->p_తదుపరి
  );
  ఒకవేళ( !pIdx ){
    sqlite3_Error_సందేశం(pParse, "no such index: %s", z_సూచీకృత_By, 0);
    pParse->checkSchema = 1;
    తిరిగి SQLITE_ERROR;
  }
  ప్రతిజ్ఞా( pFrom->fg.isCte==0 );
  pFrom->u2.pIBIndex = pIdx;
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం convert_Compound_Select_గమ్యం_Subquery(Walker *pWalker, Select *p){
  పూర్ణాంకం i;
  Select *pNew;
  Select *pX;
  sqlite3 *db;
  నిర్మాణం సమాసం_List_అంశం *a;
  SrcList *pNewSrc;
  Parse *pParse;
  Token dummy;

  ఒకవేళ( p->p_పూర్వ==0 ) తిరిగి WRC_Continue;
  ఒకవేళ( p->p_క్రమం_By==0 ) తిరిగి WRC_Continue;
  కొరకు(pX=p; pX && (pX->op==TK_ALL || pX->op==TK_SELECT); pX=pX->p_పూర్వ){}
  ఒకవేళ( pX==0 ) తిరిగి WRC_Continue;
  a = p->p_క్రమం_By->a;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
  /* If iOrderByCol is already non-zero, then it has already been matched
  ** to a result column of the SELECT statement. This occurs when the
  ** SELECT is rewritten for window-functions processing and then passed
  ** to sqlite3SelectPrep() and similar a second time. The rewriting done
  ** by this function is not required in this case. */
  ఒకవేళ( a[0].u.x.i_క్రమం_By_నిలువు ) తిరిగి WRC_Continue;
#అయితే_ముగింపు
  కొరకు(i=p->p_క్రమం_By->n_సమాసం-1; i>=0; i--){
    ఒకవేళ( a[i].p_సమాసం->flags & EP_Collate ) ఆపు;
  }
  ఒకవేళ( i<0 ) తిరిగి WRC_Continue;

  /* If we reach this point, that means the transformation is required. */

  pParse = pWalker->pParse;
  db = pParse->db;
  pNew = sqlite3DbMallocZero(db, పరిమాణం(*pNew) );
  ఒకవేళ( pNew==0 ) తిరిగి WRC_Abort;
  memset(&dummy, 0, పరిమాణం(dummy));
  pNewSrc = sqlite3_Src_List_Append_From_పదం(pParse,0,0,0,&dummy,pNew,0);
  ప్రతిజ్ఞా( pNewSrc!=0 || pParse->nErr );
  ఒకవేళ( pParse->nErr ){
    sqlite3SrcListDelete(db, pNewSrc);
    తిరిగి WRC_Abort;
  }
  *pNew = *p;
  p->pSrc = pNewSrc;
  p->pEList = sqlite3_సమాసం_List_Append(pParse, 0, sqlite3_సమాసం(db, TK_ASTERISK, 0));
  p->op = TK_SELECT;
  p->pWhere = 0;
  p->p_సమూహం_By = 0;
  p->pHaving = 0;
  pNew->p_క్రమం_By = 0;
  p->p_పూర్వ = 0;
  p->p_తదుపరి = 0;
  p->pWith = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
  p->pWinDefn = 0;
#అయితే_ముగింపు
  p->selFlags &= ~(u32)(SF_Compound|SF_Distinct);
  ప్రతిజ్ఞా( (p->selFlags & SF_Converted)==0 );
  p->selFlags |= SF_Converted;
  ప్రతిజ్ఞా( pNew->p_పూర్వ!=0 );
  pNew->p_పూర్వ->p_తదుపరి = pNew;
  pNew->p_హద్దు = 0;
  తిరిగి WRC_Continue;
}

/*
** Check to see if the FROM clause term pFrom has table-valued function
** arguments.  If it does, leave an error message in pParse and return
** non-zero, since pFrom is not allowed to be a table-valued function.
*/
స్థిర పూర్ణాంకం cannotBeFunction(Parse *pParse, Src_అంశం *pFrom){
  ఒకవేళ( pFrom->fg.isTabFunc ){
    sqlite3_Error_సందేశం(pParse, "'%s' is not a function", pFrom->zName);
    తిరిగి 1;
  }
  తిరిగి 0;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_CTE
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
స్థిర నిర్మాణం Cte *శోధన_With(
  With *pWith,                    /* Current innermost WITH clause */
  Src_అంశం *p_అంశం,                 /* FROM clause element to resolve */
  With **ppContext                /* OUT: WITH clause return value belongs to */
){
  స్థిరరాశి అక్షరం *zName = p_అంశం->zName;
  With *p;
  ప్రతిజ్ఞా( p_అంశం->fg.fixedSchema || p_అంశం->u4.zDatabase==0 );
  ప్రతిజ్ఞా( zName!=0 );
  కొరకు(p=pWith; p; p=p->pOuter){
    పూర్ణాంకం i;
    కొరకు(i=0; i<p->nCte; i++){
      ఒకవేళ( sqlite3StrICmp(zName, p->a[i].zName)==0 ){
        *ppContext = p;
        తిరిగి &p->a[i];
      }
    }
    ఒకవేళ( p->bView ) ఆపు;
  }
  తిరిగి 0;
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
With *sqlite3_With_నెట్టు(Parse *pParse, With *pWith, u8 bFree){
  ఒకవేళ( pWith ){
    ఒకవేళ( bFree ){
      pWith = (With*)sqlite3_Parser_కూడు_Cleanup(pParse, sqlite3WithDeleteGeneric,
                      pWith);
      ఒకవేళ( pWith==0 ) తిరిగి 0;
    }
    ఒకవేళ( pParse->nErr==0 ){
      ప్రతిజ్ఞా( pParse->pWith!=pWith );
      pWith->pOuter = pParse->pWith;
      pParse->pWith = pWith;
    }
  }
  తిరిగి pWith;
}

స్థిర పూర్ణాంకం selectExpanderWalk(Walker*, Select*);

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
స్థిర పూర్ణాంకం resolve_From_పదం_గమ్యం_Cte(
  Parse *pParse,                  /* The parsing context */
  Walker *pWalker,                /* Current tree walker */
  Src_అంశం *pFrom                  /* The FROM clause term to check */
){
  Cte *pCte;               /* Matched CTE (or NULL if no match) */
  With *pWith;             /* The matching WITH */

  ప్రతిజ్ఞా( pFrom->pSTab==0 );
  ఒకవేళ( pParse->pWith==0 ){
    /* There are no WITH clauses in the stack.  No match is possible */
    తిరిగి 0;
  }
  ఒకవేళ( pParse->nErr ){
    /* Prior errors might have left pParse->pWith in a goofy state, so
    ** go no further. */
    తిరిగి 0;
  }
  ప్రతిజ్ఞా( pFrom->fg.hadSchema==0 || pFrom->fg.notCte!=0 );
  ఒకవేళ( pFrom->fg.fixedSchema==0 && pFrom->u4.zDatabase!=0 ){
    /* The FROM term contains a schema qualifier (ex: main.t1) and so
    ** it cannot possibly be a CTE reference. */
    తిరిగి 0;
  }
  ఒకవేళ( pFrom->fg.notCte ){
    /* The FROM term is specifically excluded from matching a CTE.
    **   (1)  It is part of a trigger that used to have zDatabase but had
    **        zDatabase removed by sqlite3FixTriggerStep().
    **   (2)  This is the first term in the FROM clause of an UPDATE.
    */
    తిరిగి 0;
  }
  pCte = శోధన_With(pParse->pWith, pFrom, &pWith);
  ఒకవేళ( pCte ){
    sqlite3 *db = pParse->db;
    పట్టిక *pTab;
    సమాసం_List *pEList;
    Select *pSel;
    Select *p_ఎడమ;                /* Left-most SELECT statement */
    Select *p_Rec_పదం;             /* Left-most recursive term */
    పూర్ణాంకం bMayRecursive;            /* True if compound joined by UNION [ALL] */
    With *pSavedWith;             /* Initial value of pParse->pWith */
    పూర్ణాంకం iRecTab = -1;             /* Cursor for recursive table */
    CteUse *pCteUse;

    /* If pCte->zCteErr is non-NULL at this point, then this is an illegal
    ** recursive reference to CTE pCte. Leave an error in pParse and return
    ** early. If pCte->zCteErr is NULL, then this is not a recursive reference.
    ** In this case, proceed.  */
    ఒకవేళ( pCte->zCteErr ){
      sqlite3_Error_సందేశం(pParse, pCte->zCteErr, pCte->zName);
      తిరిగి 2;
    }
    ఒకవేళ( cannotBeFunction(pParse, pFrom) ) తిరిగి 2;

    ప్రతిజ్ఞా( pFrom->pSTab==0 );
    pTab = sqlite3DbMallocZero(db, పరిమాణం(పట్టిక));
    ఒకవేళ( pTab==0 ) తిరిగి 2;
    pCteUse = pCte->pUse;
    ఒకవేళ( pCteUse==0 ){
      pCte->pUse = pCteUse = sqlite3DbMallocZero(db, పరిమాణం(pCteUse[0]));
      ఒకవేళ( pCteUse==0
       || sqlite3_Parser_కూడు_Cleanup(pParse,sqlite3DbFree,pCteUse)==0
      ){
        sqlite3DbFree(db, pTab);
        తిరిగి 2;
      }
      pCteUse->eM10d = pCte->eM10d;
    }
    pFrom->pSTab = pTab;
    pTab->nTabRef = 1;
    pTab->zName = sqlite3DbStrDup(db, pCte->zName);
    pTab->iPKey = -1;
    pTab->n_అడ్డువరుస_Log_Est = 200; ప్రతిజ్ఞా( 200==sqlite3LogEst(1048576) );
    pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
    sqlite3_Src_అంశం_Attach_Subquery(pParse, pFrom, pCte->pSelect, 1);
    ఒకవేళ( db->mallocFailed ) తిరిగి 2;
    ప్రతిజ్ఞా( pFrom->fg.isSubquery && pFrom->u4.pSubq );
    pSel = pFrom->u4.pSubq->pSelect;
    ప్రతిజ్ఞా( pSel!=0 );
    pSel->selFlags |= SF_CopyCte;
    ఒకవేళ( pFrom->fg.is_సూచీకృత_By ){
      sqlite3_Error_సందేశం(pParse, "no such index: \"%s\"", pFrom->u1.z_సూచీకృత_By);
      తిరిగి 2;
    }
    ప్రతిజ్ఞా( !pFrom->fg.is_సూచీకృత_By );
    pFrom->fg.isCte = 1;
    pFrom->u2.pCteUse = pCteUse;
    pCteUse->nUse++;

    /* Check if this is a recursive CTE. */
    p_Rec_పదం = pSel;
    bMayRecursive = ( pSel->op==TK_ALL || pSel->op==TK_UNION );
    అయితే( bMayRecursive && p_Rec_పదం->op==pSel->op ){
      పూర్ణాంకం i;
      SrcList *pSrc = p_Rec_పదం->pSrc;
      ప్రతిజ్ఞా( p_Rec_పదం->p_పూర్వ!=0 );
      కొరకు(i=0; i<pSrc->nSrc; i++){
        Src_అంశం *p_అంశం = &pSrc->a[i];
        ఒకవేళ( p_అంశం->zName!=0
         && !p_అంశం->fg.hadSchema
         && ALWAYS( !p_అంశం->fg.isSubquery )
         && (p_అంశం->fg.fixedSchema || p_అంశం->u4.zDatabase==0)
         && 0==sqlite3StrICmp(p_అంశం->zName, pCte->zName)
        ){
          p_అంశం->pSTab = pTab;
          pTab->nTabRef++;
          p_అంశం->fg.isRecursive = 1;
          ఒకవేళ( p_Rec_పదం->selFlags & SF_Recursive ){
            sqlite3_Error_సందేశం(pParse,
               "multiple references to recursive table: %s", pCte->zName
            );
            తిరిగి 2;
          }
          p_Rec_పదం->selFlags |= SF_Recursive;
          ఒకవేళ( iRecTab<0 ) iRecTab = pParse->nTab++;
          p_అంశం->iCursor = iRecTab;
        }
      }
      ఒకవేళ( (p_Rec_పదం->selFlags & SF_Recursive)==0 ) ఆపు;
      p_Rec_పదం = p_Rec_పదం->p_పూర్వ;
    }

    ఒకవేళ( pParse->nTab >= db->a_హద్దు[SQLITE_హద్దు_CTES]
     && db->a_హద్దు[SQLITE_హద్దు_CTES]>0
    ){
      sqlite3_Error_సందేశం(pParse, "too many CTEs");
      తిరిగి 2;
    }
#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
    pParse->nNestSel++;
    ఒకవేళ( check_Subquery_Nesting_లోతు(pParse) ) తిరిగి 2;
#అయితే_ముగింపు
    pCte->zCteErr = "circular reference: %s";
    pSavedWith = pParse->pWith;
    pParse->pWith = pWith;
    ఒకవేళ( pSel->selFlags & SF_Recursive ){
      పూర్ణాంకం rc;
      ప్రతిజ్ఞా( p_Rec_పదం!=0 );
      ప్రతిజ్ఞా( (p_Rec_పదం->selFlags & SF_Recursive)==0 );
      ప్రతిజ్ఞా( p_Rec_పదం->p_తదుపరి!=0 );
      ప్రతిజ్ఞా( (p_Rec_పదం->p_తదుపరి->selFlags & SF_Recursive)!=0 );
      ప్రతిజ్ఞా( p_Rec_పదం->pWith==0 );
      p_Rec_పదం->pWith = pSel->pWith;
      rc = sqlite3WalkSelect(pWalker, p_Rec_పదం);
      p_Rec_పదం->pWith = 0;
      ఒకవేళ( rc ){
        pParse->pWith = pSavedWith;
        తిరిగి 2;
      }
    }లేకపోతే{
      ఒకవేళ( selectExpanderWalk(pWalker, pSel) ){
        pParse->pWith = pSavedWith;
        తిరిగి 2;
      }
    }
    pParse->pWith = pWith;
#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
    pParse->nNestSel--;
    ప్రతిజ్ఞా( pParse->nNestSel>=0 );
#అయితే_ముగింపు

    కొరకు(p_ఎడమ=pSel; p_ఎడమ->p_పూర్వ; p_ఎడమ=p_ఎడమ->p_పూర్వ);
    pEList = p_ఎడమ->pEList;
    ఒకవేళ( pCte->p_నిలువుల_సంఖ్య ){
      ఒకవేళ( pEList && pEList->n_సమాసం!=pCte->p_నిలువుల_సంఖ్య->n_సమాసం ){
        sqlite3_Error_సందేశం(pParse, "table %s has %d values for %d columns",
            pCte->zName, pEList->n_సమాసం, pCte->p_నిలువుల_సంఖ్య->n_సమాసం
        );
        pParse->pWith = pSavedWith;
        తిరిగి 2;
      }
      pEList = pCte->p_నిలువుల_సంఖ్య;
    }

    sqlite3_Columns_From_సమాసం_List(pParse, pEList, &pTab->n_నిలువు, &pTab->a_నిలువు);
    ఒకవేళ( bMayRecursive ){
      ఒకవేళ( pSel->selFlags & SF_Recursive ){
        pCte->zCteErr = "multiple recursive references: %s";
      }లేకపోతే{
        pCte->zCteErr = "recursive reference in a subquery: %s";
      }
      sqlite3WalkSelect(pWalker, pSel);
    }
    pCte->zCteErr = 0;
    pParse->pWith = pSavedWith;
    తిరిగి 1;  /* Success */
  }
  తిరిగి 0;  /* No match */
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_CTE
/*
** If the SELECT passed as the second argument has an associated WITH
** clause, pop it from the stack stored as part of the Parse object.
**
** This function is used as the xSelectCallback2() callback by
** sqlite3SelectExpand() when walking a SELECT tree to resolve table
** names and other FROM clause elements.
*/
శూన్య sqlite3_Select_తీయి_With(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  ఒకవేళ( OK_IF_ALWAYS_TRUE(pParse->pWith) && p->p_పూర్వ==0 ){
    With *pWith = కనుగొను_Rightmost(p)->pWith;
    ఒకవేళ( pWith!=0 ){
      ప్రతిజ్ఞా( pParse->pWith==pWith || pParse->nErr );
      pParse->pWith = pWith->pOuter;
    }
  }
}
#అయితే_ముగింపు

/*
** The SrcItem structure passed as the second argument represents a
** sub-query in the FROM clause of a SELECT statement. This function
** allocates and populates the SrcItem.pTab object. If successful,
** SQLITE_OK is returned. Otherwise, if an OOM error is encountered,
** SQLITE_NOMEM.
*/
పూర్ణాంకం sqlite3ExpandSubquery(Parse *pParse, Src_అంశం *pFrom){
  Select *pSel;
  పట్టిక *pTab;

  ప్రతిజ్ఞా( pFrom->fg.isSubquery );
  ప్రతిజ్ఞా( pFrom->u4.pSubq!=0 );
  pSel = pFrom->u4.pSubq->pSelect;
  ప్రతిజ్ఞా( pSel );
  pFrom->pSTab = pTab = sqlite3DbMallocZero(pParse->db, పరిమాణం(పట్టిక));
  ఒకవేళ( pTab==0 ) తిరిగి SQLITE_NOMEM;
  pTab->nTabRef = 1;
  ఒకవేళ( pFrom->zAlias ){
    pTab->zName = sqlite3DbStrDup(pParse->db, pFrom->zAlias);
  }లేకపోతే{
    pTab->zName = sqlite3MPrintf(pParse->db, "%!S", pFrom);
  }
  అయితే( pSel->p_పూర్వ ){ pSel = pSel->p_పూర్వ; }
  sqlite3_Columns_From_సమాసం_List(pParse, pSel->pEList,&pTab->n_నిలువు,&pTab->a_నిలువు);
  pTab->iPKey = -1;
  pTab->eTabType = TABTYP_VIEW;
  pTab->n_అడ్డువరుస_Log_Est = 200; ప్రతిజ్ఞా( 200==sqlite3LogEst(1048576) );
#నిర్వచితం_కాకపోతే SQLITE_ALLOW_ROWID_లోపల_VIEW
  /* The usual case - do not allow ROWID on a subquery */
  pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
#లేకపోతే
  /* Legacy compatibility mode */
  pTab->tabFlags |= TF_Ephemeral | sqlite3Config.mNoVisibleRowid;
#అయితే_ముగింపు
  తిరిగి pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
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
స్థిర పూర్ణాంకం లోపల_Any_Using_Clause(
  స్థిరరాశి అక్షరం *zName, /* Name we are looking for */
  Src_అంశం *p_పునాది,    /* The base SrcItem.  Looking at pBase[1] and following */
  పూర్ణాంకం N              /* How many SrcItems to check */
){
  అయితే( N>0 ){
    N--;
    p_పునాది++;
    ఒకవేళ( p_పునాది->fg.isUsing==0 ) కొనసాగు;
    ఒకవేళ( NEVER(p_పునాది->u3.pUsing==0) ) కొనసాగు;
    ఒకవేళ( sqlite3_Id_List_సూచీ(p_పునాది->u3.pUsing, zName)>=0 ) తిరిగి 1;
  }
  తిరిగి 0;
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
స్థిర పూర్ణాంకం selectExpander(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  పూర్ణాంకం i, j, k, rc;
  SrcList *pTabList;
  సమాసం_List *pEList;
  Src_అంశం *pFrom;
  sqlite3 *db = pParse->db;
  సమాసం *pE, *p_కుడి, *p_సమాసం;
  u16 selFlags = p->selFlags;
  u32 elistFlags = 0;

  p->selFlags |= SF_Expanded;
  ఒకవేళ( db->mallocFailed  ){
    తిరిగి WRC_Abort;
  }
  ప్రతిజ్ఞా( p->pSrc!=0 );
  ఒకవేళ( (selFlags & SF_Expanded)!=0 ){
    తిరిగి WRC_Prune;
  }
  ఒకవేళ( pWalker->e_సంకేతం ){
    /* Renumber selId because it has been copied from a view */
    p->selId = ++pParse->nSelect;
  }
  pTabList = p->pSrc;
  pEList = p->pEList;
  ఒకవేళ( pParse->pWith && (p->selFlags & SF_View) ){
    ఒకవేళ( p->pWith==0 ){
      p->pWith = (With*)sqlite3DbMallocZero(db, SZ_WITH(1) );
      ఒకవేళ( p->pWith==0 ){
        తిరిగి WRC_Abort;
      }
    }
    p->pWith->bView = 1;
  }
  sqlite3_With_నెట్టు(pParse, p->pWith, 0);

  /* Make sure cursor numbers have been assigned to all entries in
  ** the FROM clause of the SELECT statement.
  */
  sqlite3_Src_List_కేటాయించు_Cursors(pParse, pTabList);

  /* Look up every table named in the FROM clause of the select.  If
  ** an entry of the FROM clause is a subquery instead of a table or view,
  ** then create a transient table structure to describe the subquery.
  */
  కొరకు(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    పట్టిక *pTab;
    ప్రతిజ్ఞా( pFrom->fg.isRecursive==0 || pFrom->pSTab!=0 );
    ఒకవేళ( pFrom->pSTab ) కొనసాగు;
    ప్రతిజ్ఞా( pFrom->fg.isRecursive==0 );
    ఒకవేళ( pFrom->zName==0 ){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
      Select *pSel;
      ప్రతిజ్ఞా( pFrom->fg.isSubquery && pFrom->u4.pSubq!=0 );
      pSel = pFrom->u4.pSubq->pSelect;
      /* A sub-query in the FROM clause of a SELECT */
      ప్రతిజ్ఞా( pSel!=0 );
      ప్రతిజ్ఞా( pFrom->pSTab==0 );
      ఒకవేళ( sqlite3WalkSelect(pWalker, pSel) ) తిరిగి WRC_Abort;
      ఒకవేళ( sqlite3ExpandSubquery(pParse, pFrom) ) తిరిగి WRC_Abort;
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_CTE
    }లేకపోతే ఒకవేళ( (rc = resolve_From_పదం_గమ్యం_Cte(pParse, pWalker, pFrom))!=0 ){
      ఒకవేళ( rc>1 ) తిరిగి WRC_Abort;
      pTab = pFrom->pSTab;
      ప్రతిజ్ఞా( pTab!=0 );
#అయితే_ముగింపు
    }లేకపోతే{
      /* An ordinary table or view name in the FROM clause */
      ప్రతిజ్ఞా( pFrom->pSTab==0 );
      pFrom->pSTab = pTab = sqlite3_Locate_పట్టిక_అంశం(pParse, 0, pFrom);
      ఒకవేళ( pTab==0 ) తిరిగి WRC_Abort;
      ఒకవేళ( pTab->nTabRef>=0xffff ){
        sqlite3_Error_సందేశం(pParse, "too many references to \"%s\": max 65535",
           pTab->zName);
        pFrom->pSTab = 0;
        తిరిగి WRC_Abort;
      }
      pTab->nTabRef++;
      ఒకవేళ( !IsVirtual(pTab) && cannotBeFunction(pParse, pFrom) ){
        తిరిగి WRC_Abort;
      }
#ఒకవేళ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
      ఒకవేళ( !Is_Ordinary_పట్టిక(pTab) ){
        i16 n_నిలువు;
        u8 e_సంకేతం_Orig = pWalker->e_సంకేతం;
        ఒకవేళ( sqlite3_View_Get_స్తంభం_పేర్లు(pParse, pTab) ) తిరిగి WRC_Abort;
        ప్రతిజ్ఞా( pFrom->fg.isSubquery==0 );
        ఒకవేళ( IsView(pTab) ){
          ఒకవేళ( (db->flags & SQLITE_EnableView)==0
           && pTab->pSchema!=db->aDb[1].pSchema
          ){
            sqlite3_Error_సందేశం(pParse, "access to view \"%s\" prohibited",
              pTab->zName);
          }
          sqlite3_Src_అంశం_Attach_Subquery(pParse, pFrom, pTab->u.view.pSelect, 1);
        }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_VIRTUALTABLE
        లేకపోతే ఒకవేళ( ALWAYS(IsVirtual(pTab))
         && (pFrom->fg.fromDDL || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
         && ALWAYS(pTab->u.vtab.p!=0)
         && pTab->u.vtab.p->eVtabRisk > ((db->flags & SQLITE_TrustedSchema)!=0)
        ){
          sqlite3_Error_సందేశం(pParse, "unsafe use of virtual table \"%s\"",
                                  pTab->zName);
        }
        ప్రతిజ్ఞా( SQLITE_VTABRISK_సాధారణ==1 && SQLITE_VTABRISK_ఎగువ==2 );
#అయితే_ముగింపు
        n_నిలువు = pTab->n_నిలువు;
        pTab->n_నిలువు = -1;
        pWalker->e_సంకేతం = 1;  /* Turn on Select.selId renumbering */
        ఒకవేళ( pFrom->fg.isSubquery ){
          bft hasCompound = pParse->hasCompound;
          pParse->hasCompound = 1;
          selectExpanderWalk(pWalker, pFrom->u4.pSubq->pSelect);
          pParse->hasCompound = hasCompound;
        }
        pWalker->e_సంకేతం = e_సంకేతం_Orig;
        pTab->n_నిలువు = n_నిలువు;
      }
#అయితే_ముగింపు
    }

    /* Locate the index named by the INDEXED BY clause, if any. */
    ఒకవేళ( pFrom->fg.is_సూచీకృత_By && sqlite3_సూచీకృత_By_Lookup(pParse, pFrom) ){
      తిరిగి WRC_Abort;
    }
  }

  /* Process NATURAL keywords, and ON and USING clauses of joins.
  */
  ప్రతిజ్ఞా( db->mallocFailed==0 || pParse->nErr!=0 );
  ఒకవేళ( pParse->nErr || sqlite3ProcessJoin(pParse, p) ){
    తిరిగి WRC_Abort;
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
  కొరకు(k=0; k<pEList->n_సమాసం; k++){
    pE = pEList->a[k].p_సమాసం;
    ఒకవేళ( pE->op==TK_ASTERISK ) ఆపు;
    ప్రతిజ్ఞా( pE->op!=TK_DOT || pE->p_కుడి!=0 );
    ప్రతిజ్ఞా( pE->op!=TK_DOT || (pE->p_ఎడమ!=0 && pE->p_ఎడమ->op==TK_ID) );
    ఒకవేళ( pE->op==TK_DOT && pE->p_కుడి->op==TK_ASTERISK ) ఆపు;
    elistFlags |= pE->flags;
  }
  ఒకవేళ( k<pEList->n_సమాసం ){
    /*
    ** If we get here it means the result set contains one or more "*"
    ** operators that need to be expanded.  Loop through each expression
    ** in the result set and expand them one by one.
    */
    నిర్మాణం సమాసం_List_అంశం *a = pEList->a;
    సమాసం_List *pNew = 0;
    పూర్ణాంకం flags = pParse->db->flags;
    పూర్ణాంకం long_పేర్లు = (flags & SQLITE_నిండిన_నిలువు_పేర్లు)!=0
                      && (flags & SQLITE_Short_నిలువు_పేర్లు)==0;

    కొరకు(k=0; k<pEList->n_సమాసం; k++){
      pE = a[k].p_సమాసం;
      elistFlags |= pE->flags;
      p_కుడి = pE->p_కుడి;
      ప్రతిజ్ఞా( pE->op!=TK_DOT || p_కుడి!=0 );
      ఒకవేళ( pE->op!=TK_ASTERISK
       && (pE->op!=TK_DOT || p_కుడి->op!=TK_ASTERISK)
      ){
        /* This particular expression does not need to be expanded.
        */
        pNew = sqlite3_సమాసం_List_Append(pParse, pNew, a[k].p_సమాసం);
        ఒకవేళ( pNew ){
          pNew->a[pNew->n_సమాసం-1].zEName = a[k].zEName;
          pNew->a[pNew->n_సమాసం-1].fg.eEName = a[k].fg.eEName;
          a[k].zEName = 0;
        }
        a[k].p_సమాసం = 0;
      }లేకపోతే{
        /* This expression is a "*" or a "TABLE.*" and needs to be
        ** expanded. */
        పూర్ణాంకం పట్టిక_చూసిన = 0;      /* Set to 1 when TABLE matches */
        అక్షరం *zTName = 0;       /* text of name of TABLE */
        పూర్ణాంకం iErrOfst;
        ఒకవేళ( pE->op==TK_DOT ){
          ప్రతిజ్ఞా( (selFlags & SF_NestedFrom)==0 );
          ప్రతిజ్ఞా( pE->p_ఎడమ!=0 );
          ప్రతిజ్ఞా( !సమాసం_Has_Property(pE->p_ఎడమ, EP_Int_విలువ) );
          zTName = pE->p_ఎడమ->u.zToken;
          ప్రతిజ్ఞా( సమాసం_Use_WOfst(pE->p_ఎడమ) );
          iErrOfst = pE->p_కుడి->w.iOfst;
        }లేకపోతే{
          ప్రతిజ్ఞా( pE->p_ఎడమ==0 );
          ప్రతిజ్ఞా( సమాసం_Use_WOfst(pE) );
          iErrOfst = pE->w.iOfst;
        }
        కొరకు(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
          పూర్ణాంకం n_కూడు;                    /* Number of cols including rowid */
          పట్టిక *pTab = pFrom->pSTab;  /* Table for this data source */
          సమాసం_List *pNestedFrom;       /* Result-set of a nested FROM clause */
          అక్షరం *zTabName;              /* AS name for this data source */
          స్థిరరాశి అక్షరం *zSchemaName = 0; /* Schema name for this data source */
          పూర్ణాంకం iDb;                     /* Schema index for this data src */
          IdList *pUsing;              /* USING clause for pFrom[1] */

          ఒకవేళ( (zTabName = pFrom->zAlias)==0 ){
            zTabName = pTab->zName;
          }
          ఒకవేళ( db->mallocFailed ) ఆపు;
          ప్రతిజ్ఞా( (పూర్ణాంకం)pFrom->fg.isNestedFrom == IsNestedFrom(pFrom) );
          ఒకవేళ( pFrom->fg.isNestedFrom ){
            ప్రతిజ్ఞా( pFrom->fg.isSubquery && pFrom->u4.pSubq );
            ప్రతిజ్ఞా( pFrom->u4.pSubq->pSelect!=0 );
            pNestedFrom = pFrom->u4.pSubq->pSelect->pEList;
            ప్రతిజ్ఞా( pNestedFrom!=0 );
            ప్రతిజ్ఞా( pNestedFrom->n_సమాసం==pTab->n_నిలువు );
            ప్రతిజ్ఞా( VisibleRowid(pTab)==0 || ViewCanHaveRowid );
          }లేకపోతే{
            ఒకవేళ( zTName && sqlite3StrICmp(zTName, zTabName)!=0 ){
              కొనసాగు;
            }
            pNestedFrom = 0;
            iDb = sqlite3_Schema_గమ్యం_సూచీ(db, pTab->pSchema);
            zSchemaName = iDb>=0 ? db->aDb[iDb].zDbSName : "*";
          }
          ఒకవేళ( i+1<pTabList->nSrc
           && pFrom[1].fg.isUsing
           && (selFlags & SF_NestedFrom)!=0
          ){
            పూర్ణాంకం ii;
            pUsing = pFrom[1].u3.pUsing;
            కొరకు(ii=0; ii<pUsing->nId; ii++){
              స్థిరరాశి అక్షరం *zUName = pUsing->a[ii].zName;
              p_కుడి = sqlite3_సమాసం(db, TK_ID, zUName);
              sqlite3_సమాసం_సమితి_Error_స్థానభ్రంశం(p_కుడి, iErrOfst);
              pNew = sqlite3_సమాసం_List_Append(pParse, pNew, p_కుడి);
              ఒకవేళ( pNew ){
                నిర్మాణం సమాసం_List_అంశం *pX = &pNew->a[pNew->n_సమాసం-1];
                ప్రతిజ్ఞా( pX->zEName==0 );
                pX->zEName = sqlite3MPrintf(db,"..%s", zUName);
                pX->fg.eEName = ENAME_TAB;
                pX->fg.b_Using_పదం = 1;
              }
            }
          }లేకపోతే{
            pUsing = 0;
          }

          n_కూడు = pTab->n_నిలువు;
          ఒకవేళ( VisibleRowid(pTab) && (selFlags & SF_NestedFrom)!=0 ) n_కూడు++;
          కొరకు(j=0; j<n_కూడు; j++){
            స్థిరరాశి అక్షరం *zName; 
            నిర్మాణం సమాసం_List_అంశం *pX; /* Newly added ExprList term */

            ఒకవేళ( j==pTab->n_నిలువు ){
              zName = sqlite3RowidAlias(pTab);
              ఒకవేళ( zName==0 ) కొనసాగు;
            }లేకపోతే{
              zName = pTab->a_నిలువు[j].zCnName;

              /* If pTab is actually an SF_NestedFrom sub-select, do not
              ** expand any ENAME_ROWID columns.  */
              ఒకవేళ( pNestedFrom && pNestedFrom->a[j].fg.eEName==ENAME_ROWID ){
                కొనసాగు;
              }

              ఒకవేళ( zTName
               && pNestedFrom
               && sqlite3_సరిపోలిక_EName(&pNestedFrom->a[j], 0, zTName, 0, 0)==0
              ){
                కొనసాగు;
              }

              /* If a column is marked as 'hidden', omit it from the expanded
              ** result-set list unless the SELECT has the SF_IncludeHidden
              ** bit set.
              */
              ఒకవేళ( (p->selFlags & SF_IncludeHidden)==0
                && Is_Hidden_స్తంభం(&pTab->a_నిలువు[j])
              ){
                కొనసాగు;
              }
              ఒకవేళ( (pTab->a_నిలువు[j].నిలువు_Flags & COLFLAG_NOEXPAND)!=0
               && zTName==0
               && (selFlags & (SF_NestedFrom))==0
              ){
                కొనసాగు;
              }
            }
            ప్రతిజ్ఞా( zName );
            పట్టిక_చూసిన = 1;

            ఒకవేళ( i>0 && zTName==0 && (selFlags & SF_NestedFrom)==0 ){
              ఒకవేళ( pFrom->fg.isUsing
               && sqlite3_Id_List_సూచీ(pFrom->u3.pUsing, zName)>=0
              ){
                /* In a join with a USING clause, omit columns in the
                ** using clause from the table on the right. */
                కొనసాగు;
              }
            }
            p_కుడి = sqlite3_సమాసం(db, TK_ID, zName);
            ఒకవేళ( (pTabList->nSrc>1
                 && (  (pFrom->fg.jointype & JT_LTORJ)==0
                     || zTName!=0
                     || (selFlags & SF_NestedFrom)!=0
                     || !లోపల_Any_Using_Clause(zName,pFrom,pTabList->nSrc-i-1)
                    )
                )
             || లోపల_RENAME_OBJECT
            ){
              సమాసం *p_ఎడమ;
              p_ఎడమ = sqlite3_సమాసం(db, TK_ID, zTabName);
              p_సమాసం = sqlite3PExpr(pParse, TK_DOT, p_ఎడమ, p_కుడి);
              ఒకవేళ( లోపల_RENAME_OBJECT && pE->p_ఎడమ ){
                ప్రతిజ్ఞా( pE->op==TK_DOT && pE->p_కుడి->op==TK_ASTERISK );
                sqlite3RenameTokenRemap(pParse, p_ఎడమ, pE->p_ఎడమ);
              }
              ఒకవేళ( zSchemaName ){
                p_ఎడమ = sqlite3_సమాసం(db, TK_ID, zSchemaName);
                p_సమాసం = sqlite3PExpr(pParse, TK_DOT, p_ఎడమ, p_సమాసం);
              }
            }లేకపోతే{
              p_సమాసం = p_కుడి;
            }
            sqlite3_సమాసం_సమితి_Error_స్థానభ్రంశం(p_సమాసం, iErrOfst);
            pNew = sqlite3_సమాసం_List_Append(pParse, pNew, p_సమాసం);
            ఒకవేళ( pNew==0 ){
              ఆపు;  /* OOM */
            }
            pX = &pNew->a[pNew->n_సమాసం-1];
            ప్రతిజ్ఞా( pX->zEName==0 );
            ఒకవేళ( (selFlags & SF_NestedFrom)!=0 && !లోపల_RENAME_OBJECT ){
              ఒకవేళ( pNestedFrom && (!ViewCanHaveRowid || j<pNestedFrom->n_సమాసం) ){
                ప్రతిజ్ఞా( j<pNestedFrom->n_సమాసం );
                pX->zEName = sqlite3DbStrDup(db, pNestedFrom->a[j].zEName);
                testcase( pX->zEName==0 );
              }లేకపోతే{
                pX->zEName = sqlite3MPrintf(db, "%s.%s.%s",
                                           zSchemaName, zTabName, zName);
                testcase( pX->zEName==0 );
              }
              pX->fg.eEName = (j==pTab->n_నిలువు ? ENAME_ROWID : ENAME_TAB);
              ఒకవేళ( (pFrom->fg.isUsing
                   && sqlite3_Id_List_సూచీ(pFrom->u3.pUsing, zName)>=0)
               || (pUsing && sqlite3_Id_List_సూచీ(pUsing, zName)>=0)
               || (j<pTab->n_నిలువు && (pTab->a_నిలువు[j].నిలువు_Flags & COLFLAG_NOEXPAND))
              ){
                pX->fg.bNoExpand = 1;
              }
            }లేకపోతే ఒకవేళ( long_పేర్లు ){
              pX->zEName = sqlite3MPrintf(db, "%s.%s", zTabName, zName);
              pX->fg.eEName = ENAME_NAME;
            }లేకపోతే{
              pX->zEName = sqlite3DbStrDup(db, zName);
              pX->fg.eEName = ENAME_NAME;
            }
          }
        }
        ఒకవేళ( !పట్టిక_చూసిన ){
          ఒకవేళ( zTName ){
            sqlite3_Error_సందేశం(pParse, "no such table: %s", zTName);
          }లేకపోతే{
            sqlite3_Error_సందేశం(pParse, "no tables specified");
          }
        }
      }
    }
    sqlite3_సమాసం_List_Delete(db, pEList);
    p->pEList = pNew;
  }
  ఒకవేళ( p->pEList ){
    ఒకవేళ( p->pEList->n_సమాసం>db->a_హద్దు[SQLITE_హద్దు_స్తంభం] ){
      sqlite3_Error_సందేశం(pParse, "too many columns in result set");
      తిరిగి WRC_Abort;
    }
    ఒకవేళ( (elistFlags & (EP_HasFunc|EP_Subquery))!=0 ){
      p->selFlags |= SF_Complex_ఫలితం;
    }
  }
#ఒకవేళ TREETRACE_ENABLED
  ఒకవేళ( sqlite3_వృక్షం_Trace & 0x8 ){
    TREETRACE(0x8,pParse,p,("After result-set wildcard expansion:\n"));
    sqlite3_వృక్షం_View_Select(0, p, 0);
  }
#అయితే_ముగింపు
  తిరిగి WRC_Continue;
}

#ఒకవేళ SQLITE_DEBUG
/*
** Always assert.  This xSelectCallback2 implementation proves that the
** xSelectCallback2 is never invoked.
*/
శూన్య sqlite3SelectWalkAssert2(Walker *Not_వాడిన, Select *NotUsed2){
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
  ప్రతిజ్ఞా( 0 );
}
#అయితే_ముగింపు

/*
** The first argument is a Walker object created by sqlite3SelectExpand().
** This function uses the walker to expand the SELECT statement passed
** as the second argument. If necessary, it also converts any compound
** SELECT statements that have COLLATE clauses on ORDER BY expressions
** to sub-queries. Return the return value of sqlite3WalkSelect().
*/
స్థిర పూర్ణాంకం selectExpanderWalk(Walker *pWalker, Select *pSelect){
  ప్రతిజ్ఞా( pWalker->x_సమాసం_Callback==sqlite3_సమాసం_Walk_Noop );
  ప్రతిజ్ఞా( pWalker->pParse!=0 );
  ఒకవేళ( OK_IF_ALWAYS_TRUE(pWalker->pParse->hasCompound) ){
    pWalker->xSelectCallback = convert_Compound_Select_గమ్యం_Subquery;
    pWalker->xSelectCallback2 = 0;
    sqlite3WalkSelect(pWalker, pSelect);
  }
  pWalker->xSelectCallback = selectExpander;
  pWalker->xSelectCallback2 = sqlite3_Select_తీయి_With;
  తిరిగి sqlite3WalkSelect(pWalker, pSelect);
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
స్థిర శూన్య sqlite3SelectExpand(Parse *pParse, Select *pSelect){
  Walker w;
  w.x_సమాసం_Callback = sqlite3_సమాసం_Walk_Noop;
  w.pParse = pParse;
  w.e_సంకేతం = 0;
  selectExpanderWalk(&w, pSelect);
}


#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
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
స్థిర శూన్య select_కూడు_Subquery_Type_Info(Walker *pWalker, Select *p){
  Parse *pParse;
  పూర్ణాంకం i;
  SrcList *pTabList;
  Src_అంశం *pFrom;

  ఒకవేళ( p->selFlags & SF_HasTypeInfo ) తిరిగి;
  p->selFlags |= SF_HasTypeInfo;
  pParse = pWalker->pParse;
  ప్రతిజ్ఞా( (p->selFlags & SF_Resolved) );
  pTabList = p->pSrc;
  కొరకు(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    పట్టిక *pTab = pFrom->pSTab;
    ప్రతిజ్ఞా( pTab!=0 );
    ఒకవేళ( (pTab->tabFlags & TF_Ephemeral)!=0 && pFrom->fg.isSubquery ){
      /* A sub-query in the FROM clause of a SELECT */
      Select *pSel = pFrom->u4.pSubq->pSelect;
      sqlite3_Subquery_స్తంభం_Types(pParse, pTab, pSel, SQLITE_AFF_NONE);
    }
  }
}
#అయితే_ముగింపు


/*
** This routine adds datatype and collating sequence information to
** the Table structures of all FROM-clause subqueries in a
** SELECT statement.
**
** Use this routine after name resolution.
*/
స్థిర శూన్య sqlite3_Select_కూడు_Type_Info(Parse *pParse, Select *pSelect){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
  Walker w;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.xSelectCallback2 = select_కూడు_Subquery_Type_Info;
  w.x_సమాసం_Callback = sqlite3_సమాసం_Walk_Noop;
  w.pParse = pParse;
  sqlite3WalkSelect(&w, pSelect);
#అయితే_ముగింపు
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
శూన్య sqlite3SelectPrep(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for container */
){
  ప్రతిజ్ఞా( p!=0 || pParse->db->mallocFailed );
  ప్రతిజ్ఞా( pParse->db->pParse==pParse );
  ఒకవేళ( pParse->db->mallocFailed ) తిరిగి;
  ఒకవేళ( p->selFlags & SF_HasTypeInfo ) తిరిగి;
  sqlite3SelectExpand(pParse, p);
  ఒకవేళ( pParse->nErr ) తిరిగి;
  sqlite3_Resolve_Select_పేర్లు(pParse, p, pOuterNC);
  ఒకవేళ( pParse->nErr ) తిరిగి;
  sqlite3_Select_కూడు_Type_Info(pParse, p);
}

#ఒకవేళ TREETRACE_ENABLED
/*
** Display all information about an AggInfo object
*/
స్థిర శూన్య printAggInfo(AggInfo *pAggInfo){
  పూర్ణాంకం ii;
  sqlite3DebugPrintf("AggInfo %d/%p:\n",
     pAggInfo->selId, pAggInfo);
  కొరకు(ii=0; ii<pAggInfo->n_స్తంభం; ii++){
    నిర్మాణం Agg_Info_నిలువు *p_నిలువు = &pAggInfo->a_నిలువు[ii];
    sqlite3DebugPrintf(
       "agg-column[%d] pTab=%s iTable=%d iColumn=%d iMem=%d"
       " iSorterColumn=%d %s\n",
       ii, p_నిలువు->pTab ? p_నిలువు->pTab->zName : "NULL",
       p_నిలువు->i_పట్టిక, p_నిలువు->i_స్తంభం, pAggInfo->i_మొదటి_Reg+ii,
       p_నిలువు->i_Sorter_స్తంభం,
       ii>=pAggInfo->n_సంచాయకం ? "" : " Accumulator");
    sqlite3_వృక్షం_View_సమాసం(0, pAggInfo->a_నిలువు[ii].pCExpr, 0);
  }
  కొరకు(ii=0; ii<pAggInfo->nFunc; ii++){
    sqlite3DebugPrintf("agg-func[%d]: iMem=%d\n",
        ii, pAggInfo->i_మొదటి_Reg+pAggInfo->n_స్తంభం+ii);
    sqlite3_వృక్షం_View_సమాసం(0, pAggInfo->aFunc[ii].pFExpr, 0);
  }
}
#అయితే_ముగింపు /* TREETRACE_ENABLED */

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
స్థిర శూన్య analyze_Agg_Func_వాదనలు(
  AggInfo *pAggInfo,
  NameContext *pNC
){
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( pAggInfo!=0 );
  ప్రతిజ్ఞా( pAggInfo->i_మొదటి_Reg==0 );
  pNC->ncFlags |= NC_లోపల_Agg_Func;
  కొరకు(i=0; i<pAggInfo->nFunc; i++){
    సమాసం *p_సమాసం = pAggInfo->aFunc[i].pFExpr;
    ప్రతిజ్ఞా( p_సమాసం->op==TK_FUNCTION || p_సమాసం->op==TK_AGG_FUNCTION );
    ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
    sqlite3_సమాసం_Analyze_Agg_List(pNC, p_సమాసం->x.pList);
    ఒకవేళ( p_సమాసం->p_ఎడమ ){
      ప్రతిజ్ఞా( p_సమాసం->p_ఎడమ->op==TK_క్రమం );
      ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం->p_ఎడమ) );
      sqlite3_సమాసం_Analyze_Agg_List(pNC, p_సమాసం->p_ఎడమ->x.pList);
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    ప్రతిజ్ఞా( !Is_కిటికీ_Func(p_సమాసం) );
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_WinFunc) ){
      sqlite3_సమాసం_Analyze_Aggregates(pNC, p_సమాసం->y.pWin->pFilter);
    }
#అయితే_ముగింపు
  }
  pNC->ncFlags &= ~NC_లోపల_Agg_Func;
}

/*
** An index on expressions is being used in the inner loop of an
** aggregate query with a GROUP BY clause.  This routine attempts
** to adjust the AggInfo object to take advantage of index and to
** perhaps use the index as a covering index.
**
*/
స్థిర శూన్య optimize_Aggregate_Use_యొక్క_సూచీకృత_సమాసం(
  Parse *pParse,          /* Parsing context */
  Select *pSelect,        /* The SELECT statement being processed */
  AggInfo *pAggInfo,      /* The aggregate info */
  NameContext *pNC        /* Name context used to resolve agg-func args */
){
  ప్రతిజ్ఞా( pAggInfo->i_మొదటి_Reg==0 );
  ప్రతిజ్ఞా( pSelect!=0 );
  ప్రతిజ్ఞా( pSelect->p_సమూహం_By!=0 );
  pAggInfo->n_స్తంభం = pAggInfo->n_సంచాయకం;
  ఒకవేళ( ALWAYS(pAggInfo->n_క్రమీకరణ_స్తంభం>0) ){
    పూర్ణాంకం mx = pSelect->p_సమూహం_By->n_సమాసం - 1;
    పూర్ణాంకం j, k;
    కొరకు(j=0; j<pAggInfo->n_స్తంభం; j++){
      k = pAggInfo->a_నిలువు[j].i_Sorter_స్తంభం;
      ఒకవేళ( k>mx ) mx = k;
    }
    pAggInfo->n_క్రమీకరణ_స్తంభం = mx+1;
  }
  analyze_Agg_Func_వాదనలు(pAggInfo, pNC);
#ఒకవేళ TREETRACE_ENABLED
  ఒకవేళ( sqlite3_వృక్షం_Trace & 0x20 ){
    సూచీకృత_సమాసం *pIEpr;
    TREETRACE(0x20, pParse, pSelect,
        ("AggInfo (possibly) adjusted for Indexed Exprs\n"));
    sqlite3_వృక్షం_View_Select(0, pSelect, 0);
    కొరకు(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
      రాయి("data-cursor=%d index={%d,%d}\n",
          pIEpr->i_దత్తాంశం_Cur, pIEpr->iIdxCur, pIEpr->i_Idx_నిలువు);
      sqlite3_వృక్షం_View_సమాసం(0, pIEpr->p_సమాసం, 0);
    }
    printAggInfo(pAggInfo);
  }
#లేకపోతే
  వాడని_PARAMETER(pSelect);
  వాడని_PARAMETER(pParse);
#అయితే_ముగింపు
}

/*
** Walker callback for aggregateConvertIndexedExprRefToColumn().
*/
స్థిర పూర్ణాంకం aggregate_Idx_Epr_Ref_గమ్యం_నిలువు_Callback(Walker *pWalker, సమాసం *p_సమాసం){
  AggInfo *pAggInfo;
  నిర్మాణం Agg_Info_నిలువు *p_నిలువు;
  వాడని_PARAMETER(pWalker);
  ఒకవేళ( p_సమాసం->pAggInfo==0 ) తిరిగి WRC_Continue;
  ఒకవేళ( p_సమాసం->op==TK_AGG_స్తంభం ) తిరిగి WRC_Continue;
  ఒకవేళ( p_సమాసం->op==TK_AGG_FUNCTION ) తిరిగి WRC_Continue;
  ఒకవేళ( p_సమాసం->op==TK_IF_NULL_అడ్డువరుస ) తిరిగి WRC_Continue;
  pAggInfo = p_సమాసం->pAggInfo;
  ఒకవేళ( NEVER(p_సమాసం->iAgg>=pAggInfo->n_స్తంభం) ) తిరిగి WRC_Continue;
  ప్రతిజ్ఞా( p_సమాసం->iAgg>=0 );
  p_నిలువు = &pAggInfo->a_నిలువు[p_సమాసం->iAgg];
  p_సమాసం->op = TK_AGG_స్తంభం;
  p_సమాసం->i_పట్టిక = p_నిలువు->i_పట్టిక;
  p_సమాసం->i_స్తంభం = p_నిలువు->i_స్తంభం;
  సమాసం_Clear_Property(p_సమాసం, EP_Skip|EP_Collate|EP_Unlikely);
  తిరిగి WRC_Prune;
}

/*
** Convert every pAggInfo->aFunc[].pExpr such that any node within
** those expressions that has pAppInfo set is changed into a TK_AGG_COLUMN
** opcode.
*/
స్థిర శూన్య aggregate_Convert_సూచీకృత_సమాసం_Ref_గమ్యం_స్తంభం(AggInfo *pAggInfo){
  పూర్ణాంకం i;
  Walker w;
  memset(&w, 0, పరిమాణం(w));
  w.x_సమాసం_Callback = aggregate_Idx_Epr_Ref_గమ్యం_నిలువు_Callback;
  కొరకు(i=0; i<pAggInfo->nFunc; i++){
    sqlite3_Walk_సమాసం(&w, pAggInfo->aFunc[i].pFExpr);
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
స్థిర శూన్య కేటాయించు_Aggregate_Registers(Parse *pParse, AggInfo *pAggInfo){
  ప్రతిజ్ఞా( pAggInfo!=0 );
  ప్రతిజ్ఞా( pAggInfo->i_మొదటి_Reg==0 );
  pAggInfo->i_మొదటి_Reg = pParse->nMem + 1;
  pParse->nMem += pAggInfo->n_స్తంభం + pAggInfo->nFunc;
}

/*
** Reset the aggregate accumulator.
**
** The aggregate accumulator is a set of memory cells that hold
** intermediate results while calculating an aggregate.  This
** routine generates code that stores NULLs in all of those memory
** cells.
*/
స్థిర శూన్య reset_సంచాయకం(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం i;
  నిర్మాణం AggInfo_func *pFunc;
  పూర్ణాంకం nReg = pAggInfo->nFunc + pAggInfo->n_స్తంభం;
  ప్రతిజ్ఞా( pAggInfo->i_మొదటి_Reg>0 );
  ప్రతిజ్ఞా( pParse->db->pParse==pParse );
  ప్రతిజ్ఞా( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ఒకవేళ( nReg==0 ) తిరిగి;
  ఒకవేళ( pParse->nErr ) తిరిగి;
  sqlite3_Vdbe_కూడు_Op3(v, OP_Null, 0, pAggInfo->i_మొదటి_Reg,
                    pAggInfo->i_మొదటి_Reg+nReg-1);
  కొరకు(pFunc=pAggInfo->aFunc, i=0; i<pAggInfo->nFunc; i++, pFunc++){
    ఒకవేళ( pFunc->iDistinct>=0 ){
      సమాసం *pE = pFunc->pFExpr;
      ప్రతిజ్ఞా( సమాసం_Use_XList(pE) );
      ఒకవేళ( pE->x.pList==0 || pE->x.pList->n_సమాసం!=1 ){
        sqlite3_Error_సందేశం(pParse, "DISTINCT aggregates must have exactly one "
           "argument");
        pFunc->iDistinct = -1;
      }లేకపోతే{
        కీలకం_Info *p_కీలకం_Info = sqlite3_కీలకం_Info_From_సమాసం_List(pParse, pE->x.pList,0,0);
        pFunc->iDistAddr = sqlite3_Vdbe_కూడు_Op4(v, OP_OpenEphemeral,
            pFunc->iDistinct, 0, 0, (అక్షరం*)p_కీలకం_Info, P4_KEYINFO);
        ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(DISTINCT)",
                          pFunc->pFunc->zName));
      }
    }
    ఒకవేళ( pFunc->iOBTab>=0 ){
      సమాసం_List *pOBList;
      కీలకం_Info *p_కీలకం_Info;
      పూర్ణాంకం nExtra = 0;
      ప్రతిజ్ఞా( pFunc->pFExpr->p_ఎడమ!=0 );
      ప్రతిజ్ఞా( pFunc->pFExpr->p_ఎడమ->op==TK_క్రమం );
      ప్రతిజ్ఞా( సమాసం_Use_XList(pFunc->pFExpr->p_ఎడమ) );
      ప్రతిజ్ఞా( pFunc->pFunc!=0 );
      pOBList = pFunc->pFExpr->p_ఎడమ->x.pList;
      ఒకవేళ( !pFunc->bOBUnique ){
        nExtra++;  /* One extra column for the OP_Sequence */
      }
      ఒకవేళ( pFunc->bOBPayload ){
        /* extra columns for the function arguments */
        ప్రతిజ్ఞా( సమాసం_Use_XList(pFunc->pFExpr) );
        ప్రతిజ్ఞా( pFunc->pFExpr->x.pList!=0 );
        nExtra += pFunc->pFExpr->x.pList->n_సమాసం;
      }
      ఒకవేళ( pFunc->bUseSubtype ){
        nExtra += pFunc->pFExpr->x.pList->n_సమాసం;
      }
      p_కీలకం_Info = sqlite3_కీలకం_Info_From_సమాసం_List(pParse, pOBList, 0, nExtra);
      ఒకవేళ( !pFunc->bOBUnique && pParse->nErr==0 ){
        p_కీలకం_Info->n_కీలకం_క్షేత్రం++;
      }
      sqlite3_Vdbe_కూడు_Op4(v, OP_OpenEphemeral,
            pFunc->iOBTab, pOBList->n_సమాసం+nExtra, 0,
            (అక్షరం*)p_కీలకం_Info, P4_KEYINFO);
      ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(ORDER BY)",
                          pFunc->pFunc->zName));
    }
  }
}

/*
** Invoke the OP_AggFinalize opcode for every aggregate function
** in the AggInfo structure.
*/
స్థిర శూన్య finalizeAggFunctions(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం i;
  నిర్మాణం AggInfo_func *pF;
  కొరకు(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    సమాసం_List *pList;
    ప్రతిజ్ఞా( సమాసం_Use_XList(pF->pFExpr) );
    ఒకవేళ( pParse->nErr ) తిరిగి;
    pList = pF->pFExpr->x.pList;
    ఒకవేళ( pF->iOBTab>=0 ){
      /* For an ORDER BY aggregate, calls to OP_AggStep were deferred.  Inputs
      ** were stored in emphermal table pF->iOBTab.  Here, we extract those
      ** inputs (in ORDER BY order) and make all calls to OP_AggStep
      ** before doing the OP_AggFinal call. */
      పూర్ణాంకం i_శిఖరం;        /* Start of loop for extracting columns */
      పూర్ణాంకం nArg;        /* Number of columns to extract */
      పూర్ణాంకం n_కీలకం;        /* Key columns to be skipped */
      పూర్ణాంకం regAgg;      /* Extract into this array */
      పూర్ణాంకం j;           /* Loop counter */
     
      ప్రతిజ్ఞా( pF->pFunc!=0 );
      nArg = pList->n_సమాసం;
      regAgg = sqlite3_Get_తాత్కాలిక_Range(pParse, nArg);

      ఒకవేళ( pF->bOBPayload==0 ){
        n_కీలకం = 0;
      }లేకపోతే{
        ప్రతిజ్ఞా( pF->pFExpr->p_ఎడమ!=0 );
        ప్రతిజ్ఞా( సమాసం_Use_XList(pF->pFExpr->p_ఎడమ) );
        ప్రతిజ్ఞా( pF->pFExpr->p_ఎడమ->x.pList!=0 );
        n_కీలకం = pF->pFExpr->p_ఎడమ->x.pList->n_సమాసం;
        ఒకవేళ( ALWAYS(!pF->bOBUnique) ) n_కీలకం++;
      }
      i_శిఖరం = sqlite3_Vdbe_కూడు_Op1(v, OP_Rewind, pF->iOBTab); VdbeCoverage(v);
      కొరకు(j=nArg-1; j>=0; j--){
        sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, pF->iOBTab, n_కీలకం+j, regAgg+j);
      }
      ఒకవేళ( pF->bUseSubtype ){
        పూర్ణాంకం regSubtype = sqlite3_Get_తాత్కాలిక_Reg(pParse);
        పూర్ణాంకం i_పునాది_నిలువు = n_కీలకం + nArg + (pF->bOBPayload==0 && pF->bOBUnique==0);
        కొరకు(j=nArg-1; j>=0; j--){
          sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, pF->iOBTab, i_పునాది_నిలువు+j, regSubtype);
          sqlite3_Vdbe_కూడు_Op2(v, OP_సమితి_Subtype, regSubtype, regAgg+j);
        }
        sqlite3_Release_తాత్కాలిక_Reg(pParse, regSubtype);
      }
      sqlite3_Vdbe_కూడు_Op3(v, OP_Agg_అడుగు, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3_Vdbe_కూడు_Op2(v, OP_తదుపరి, pF->iOBTab, i_శిఖరం+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, i_శిఖరం);
      sqlite3_Release_తాత్కాలిక_Range(pParse, regAgg, nArg);
    }
    sqlite3_Vdbe_కూడు_Op2(v, OP_AggFinal, AggInfoFuncReg(pAggInfo,i),
                      pList ? pList->n_సమాసం : 0);
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
స్థిర శూన్య update_సంచాయకం(
  Parse *pParse,
  పూర్ణాంకం regAcc,
  AggInfo *pAggInfo,
  పూర్ణాంకం eDistinctType
){
  Vdbe *v = pParse->pVdbe;
  పూర్ణాంకం i;
  పూర్ణాంకం reg_తగిలింది = 0;
  పూర్ణాంకం addr_తగిలింది_Test = 0;
  నిర్మాణం AggInfo_func *pF;
  నిర్మాణం Agg_Info_నిలువు *pC;

  ప్రతిజ్ఞా( pAggInfo->i_మొదటి_Reg>0 );
  ఒకవేళ( pParse->nErr ) తిరిగి;
  pAggInfo->ప్రత్యక్ష_బహుళకం = 1;
  కొరకు(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    పూర్ణాంకం nArg;
    పూర్ణాంకం addr_తదుపరి = 0;
    పూర్ణాంకం regAgg;
    పూర్ణాంకం regAggSz = 0;
    పూర్ణాంకం regDistinct = 0;
    సమాసం_List *pList;
    ప్రతిజ్ఞా( సమాసం_Use_XList(pF->pFExpr) );
    ప్రతిజ్ఞా( !Is_కిటికీ_Func(pF->pFExpr) );
    ప్రతిజ్ఞా( pF->pFunc!=0 );
    pList = pF->pFExpr->x.pList;
    ఒకవేళ( సమాసం_Has_Property(pF->pFExpr, EP_WinFunc) ){
      సమాసం *pFilter = pF->pFExpr->y.pWin->pFilter;
      ఒకవేళ( pAggInfo->n_సంచాయకం
       && (pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)
       && regAcc
      ){
        /* If regAcc==0, there there exists some min() or max() function
        ** without a FILTER clause that will ensure the magnet registers
        ** are populated. */
        ఒకవేళ( reg_తగిలింది==0 ) reg_తగిలింది = ++pParse->nMem;
        /* If this is the first row of the group (regAcc contains 0), clear the
        ** "magnet" register regHit so that the accumulator registers
        ** are populated if the FILTER clause jumps over the the
        ** invocation of min() or max() altogether. Or, if this is not
        ** the first row (regAcc contains 1), set the magnet register so that
        ** the accumulators are not populated unless the min()/max() is invoked
        ** and indicates that they should be.  */
        sqlite3_Vdbe_కూడు_Op2(v, OP_Copy, regAcc, reg_తగిలింది);
      }
      addr_తదుపరి = sqlite3VdbeMakeLabel(pParse);
      sqlite3_సమాసం_If_False(pParse, pFilter, addr_తదుపరి, SQLITE_JUMPIFNULL);
    }
    ఒకవేళ( pF->iOBTab>=0 ){
      /* Instead of invoking AggStep, we must push the arguments that would
      ** have been passed to AggStep onto the sorting table. */
      పూర్ణాంకం jj;                /* Registered used so far in building the record */
      సమాసం_List *pOBList;     /* The ORDER BY clause */
      ప్రతిజ్ఞా( pList!=0 );
      nArg = pList->n_సమాసం;
      ప్రతిజ్ఞా( nArg>0 );
      ప్రతిజ్ఞా( pF->pFExpr->p_ఎడమ!=0 );
      ప్రతిజ్ఞా( pF->pFExpr->p_ఎడమ->op==TK_క్రమం );
      ప్రతిజ్ఞా( సమాసం_Use_XList(pF->pFExpr->p_ఎడమ) );
      pOBList = pF->pFExpr->p_ఎడమ->x.pList;
      ప్రతిజ్ఞా( pOBList!=0 );
      ప్రతిజ్ఞా( pOBList->n_సమాసం>0 );
      regAggSz = pOBList->n_సమాసం;
      ఒకవేళ( !pF->bOBUnique ){
        regAggSz++;   /* One register for OP_Sequence */
      }
      ఒకవేళ( pF->bOBPayload ){
        regAggSz += nArg;
      }
      ఒకవేళ( pF->bUseSubtype ){
        regAggSz += nArg;
      }
      regAggSz++;  /* One extra register to hold result of MakeRecord */
      regAgg = sqlite3_Get_తాత్కాలిక_Range(pParse, regAggSz);
      regDistinct = regAgg;
      sqlite3_సమాసం_సంకేతం_సమాసం_List(pParse, pOBList, regAgg, 0, SQLITE_ECEL_DUP);
      jj = pOBList->n_సమాసం;
      ఒకవేళ( !pF->bOBUnique ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_Sequence, pF->iOBTab, regAgg+jj);
        jj++;
      }
      ఒకవేళ( pF->bOBPayload ){
        regDistinct = regAgg+jj;
        sqlite3_సమాసం_సంకేతం_సమాసం_List(pParse, pList, regDistinct, 0, SQLITE_ECEL_DUP);
        jj += nArg;
      }
      ఒకవేళ( pF->bUseSubtype ){
        పూర్ణాంకం kk;
        పూర్ణాంకం reg_పునాది = pF->bOBPayload ? regDistinct : regAgg;
        కొరకు(kk=0; kk<nArg; kk++, jj++){
          sqlite3_Vdbe_కూడు_Op2(v, OP_GetSubtype, reg_పునాది+kk, regAgg+jj);
        }
      }
    }లేకపోతే ఒకవేళ( pList ){
      nArg = pList->n_సమాసం;
      regAgg = sqlite3_Get_తాత్కాలిక_Range(pParse, nArg);
      regDistinct = regAgg;
      sqlite3_సమాసం_సంకేతం_సమాసం_List(pParse, pList, regAgg, 0, SQLITE_ECEL_DUP);
    }లేకపోతే{
      nArg = 0;
      regAgg = 0;
    }
    ఒకవేళ( pF->iDistinct>=0 && pList ){
      ఒకవేళ( addr_తదుపరి==0 ){
        addr_తదుపరి = sqlite3VdbeMakeLabel(pParse);
      }
      pF->iDistinct = సంకేతం_Distinct(pParse, eDistinctType,
          pF->iDistinct, addr_తదుపరి, pList, regDistinct);
    }
    ఒకవేళ( pF->iOBTab>=0 ){
      /* Insert a new record into the ORDER BY table */
      sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, regAgg, regAggSz-1,
                        regAgg+regAggSz-1);
      sqlite3_Vdbe_కూడు_Op4_Int(v, OP_Idx_చొప్పించు, pF->iOBTab, regAgg+regAggSz-1,
                           regAgg, regAggSz-1);
      sqlite3_Release_తాత్కాలిక_Range(pParse, regAgg, regAggSz);
    }లేకపోతే{
      /* Invoke the AggStep function */
      ఒకవేళ( pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        CollSeq *pColl = 0;
        నిర్మాణం సమాసం_List_అంశం *p_అంశం;
        పూర్ణాంకం j;
        ప్రతిజ్ఞా( pList!=0 );  /* pList!=0 if pF->pFunc has NEEDCOLL */
        కొరకు(j=0, p_అంశం=pList->a; !pColl && j<nArg; j++, p_అంశం++){
          pColl = sqlite3_సమాసం_Coll_Seq(pParse, p_అంశం->p_సమాసం);
        }
        ఒకవేళ( !pColl ){
          pColl = pParse->db->pDfltColl;
        }
        ఒకవేళ( reg_తగిలింది==0 && pAggInfo->n_సంచాయకం ) reg_తగిలింది = ++pParse->nMem;
        sqlite3_Vdbe_కూడు_Op4(v, OP_CollSeq, reg_తగిలింది, 0, 0,
                         (అక్షరం *)pColl, P4_COLLSEQ);
      }
      sqlite3_Vdbe_కూడు_Op3(v, OP_Agg_అడుగు, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3_Release_తాత్కాలిక_Range(pParse, regAgg, nArg);
    }
    ఒకవేళ( addr_తదుపరి ){
      sqlite3VdbeResolveLabel(v, addr_తదుపరి);
    }
    ఒకవేళ( pParse->nErr ) తిరిగి;
  }
  ఒకవేళ( reg_తగిలింది==0 && pAggInfo->n_సంచాయకం ){
    reg_తగిలింది = regAcc;
  }
  ఒకవేళ( reg_తగిలింది ){
    addr_తగిలింది_Test = sqlite3_Vdbe_కూడు_Op1(v, OP_If, reg_తగిలింది); VdbeCoverage(v);
  }
  కొరకు(i=0, pC=pAggInfo->a_నిలువు; i<pAggInfo->n_సంచాయకం; i++, pC++){
    sqlite3_సమాసం_సంకేతం(pParse, pC->pCExpr, Agg_Info_స్తంభం_Reg(pAggInfo,i));
    ఒకవేళ( pParse->nErr ) తిరిగి;
  }

  pAggInfo->ప్రత్యక్ష_బహుళకం = 0;
  ఒకవేళ( addr_తగిలింది_Test ){
    sqlite3_Vdbe_Jump_ఇక్కడ_Or_తీయి_Inst(v, addr_తగిలింది_Test);
  }
}

/*
** Add a single OP_Explain instruction to the VDBE to explain a simple
** count(*) query ("SELECT count(*) FROM pTab").
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_EXPLAIN
స్థిర శూన్య explain_Simple_లెక్క(
  Parse *pParse,                  /* Parse context */
  పట్టిక *pTab,                    /* Table being queried */
  సూచీ *pIdx                     /* Index used to optimize scan, or NULL */
){
  ఒకవేళ( pParse->explain==2 ){
    పూర్ణాంకం bCover = (pIdx!=0 && (HasRowid(pTab) || !Is_Primary_కీలకం_సూచీ(pIdx)));
    sqlite3VdbeExplain(pParse, 0, "SCAN %s%s%s",
        pTab->zName,
        bCover ? " USING COVERING INDEX " : "",
        bCover ? pIdx->zName : ""
    );
  }
}
#లేకపోతే
# define explain_Simple_లెక్క(a,b,c)
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం having_గమ్యం_Where_సమాసం_Cb(Walker *pWalker, సమాసం *p_సమాసం){
  ఒకవేళ( p_సమాసం->op!=TK_AND ){
    Select *pS = pWalker->u.pSelect;
    /* This routine is called before the HAVING clause of the current
    ** SELECT is analyzed for aggregates. So if pExpr->pAggInfo is set
    ** here, it indicates that the expression is a correlated reference to a
    ** column from an outer aggregate query, or an aggregate function that
    ** belongs to an outer query. Do not move the expression to the WHERE
    ** clause in this obscure case, as doing so may corrupt the outer Select
    ** statements AggInfo structure.  */
    ఒకవేళ( sqlite3_సమాసం_Is_Constant_Or_సమూహం_By(pWalker->pParse, p_సమాసం, pS->p_సమూహం_By)
     && సమాసం_Always_False(p_సమాసం)==0
     && p_సమాసం->pAggInfo==0
    ){
      sqlite3 *db = pWalker->pParse->db;
      సమాసం *pNew = sqlite3_సమాసం_Int32(db, 1);
      ఒకవేళ( pNew ){
        సమాసం *pWhere = pS->pWhere;
        మార్పిడి(సమాసం, *pNew, *p_సమాసం);
        pNew = sqlite3_సమాసం_And(pWalker->pParse, pWhere, pNew);
        pS->pWhere = pNew;
        pWalker->e_సంకేతం = 1;
      }
    }
    తిరిగి WRC_Prune;
  }
  తిరిగి WRC_Continue;
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
స్థిర శూన్య having_గమ్యం_Where(Parse *pParse, Select *p){
  Walker sWalker;
  memset(&sWalker, 0, పరిమాణం(sWalker));
  sWalker.pParse = pParse;
  sWalker.x_సమాసం_Callback = having_గమ్యం_Where_సమాసం_Cb;
  sWalker.u.pSelect = p;
  sqlite3_Walk_సమాసం(&sWalker, p->pHaving);
#ఒకవేళ TREETRACE_ENABLED
  ఒకవేళ( sWalker.e_సంకేతం && (sqlite3_వృక్షం_Trace & 0x100)!=0 ){
    TREETRACE(0x100,pParse,p,("Move HAVING terms into WHERE:\n"));
    sqlite3_వృక్షం_View_Select(0, p, 0);
  }
#అయితే_ముగింపు
}

/*
** Check to see if the pThis entry of pTabList is a self-join of another view.
** Search FROM-clause entries in the range of iFirst..iEnd, including iFirst
** but stopping before iEnd.
**
** If pThis is a self-join, then return the SrcItem for the first other
** instance of that view found.  If pThis is not a self-join then return 0.
*/
స్థిర Src_అంశం *isSelfJoinView(
  SrcList *pTabList,           /* Search for self-joins in this FROM clause */
  Src_అంశం *pThis,              /* Search for prior reference to this subquery */
  పూర్ణాంకం i_మొదటి, పూర్ణాంకం i_చివర        /* Range of FROM-clause entries to search. */
){
  Src_అంశం *p_అంశం;
  Select *pSel;
  ప్రతిజ్ఞా( pThis->fg.isSubquery );
  pSel = pThis->u4.pSubq->pSelect;
  ప్రతిజ్ఞా( pSel!=0 );
  ఒకవేళ( pSel->selFlags & SF_నెట్టు_కిందికి ) తిరిగి 0;
  అయితే( i_మొదటి<i_చివర ){
    Select *pS1;
    p_అంశం = &pTabList->a[i_మొదటి++];
    ఒకవేళ( !p_అంశం->fg.isSubquery ) కొనసాగు;
    ఒకవేళ( p_అంశం->fg.ద్వారా_Coroutine ) కొనసాగు;
    ఒకవేళ( p_అంశం->zName==0 ) కొనసాగు;
    ప్రతిజ్ఞా( p_అంశం->pSTab!=0 );
    ప్రతిజ్ఞా( pThis->pSTab!=0 );
    ఒకవేళ( p_అంశం->pSTab->pSchema!=pThis->pSTab->pSchema ) కొనసాగు;
    ఒకవేళ( sqlite3_stricmp(p_అంశం->zName, pThis->zName)!=0 ) కొనసాగు;
    pS1 = p_అంశం->u4.pSubq->pSelect;
    ఒకవేళ( p_అంశం->pSTab->pSchema==0 && pSel->selId!=pS1->selId ){
      /* The query flattener left two different CTE tables with identical
      ** names in the same FROM clause. */
      కొనసాగు;
    }
    ఒకవేళ( pS1->selFlags & SF_నెట్టు_కిందికి ){
      /* The view was modified by some other optimization such as
      ** pushDownWhereTerms() */
      కొనసాగు;
    }
    తిరిగి p_అంశం;
  }
  తిరిగి 0;
}

/*
** Deallocate a single AggInfo object
*/
స్థిర శూన్య agginfoFree(sqlite3 *db, శూన్య *pArg){
  AggInfo *p = (AggInfo*)pArg;
  sqlite3DbFree(db, p->a_నిలువు);
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
స్థిర పూర్ణాంకం లెక్క_యొక్క_View_Optimization(Parse *pParse, Select *p){
  Select *pSub, *p_పూర్వ;
  సమాసం *p_సమాసం;
  సమాసం *p_లెక్క;
  sqlite3 *db;
  Src_అంశం *pFrom;
  ఒకవేళ( (p->selFlags & SF_Aggregate)==0 ) తిరిగి 0;   /* This is an aggregate */
  ఒకవేళ( p->pEList->n_సమాసం!=1 ) తిరిగి 0;               /* Single result column */
  ఒకవేళ( p->pWhere ) తిరిగి 0;
  ఒకవేళ( p->pHaving ) తిరిగి 0;
  ఒకవేళ( p->p_సమూహం_By ) తిరిగి 0;
  ఒకవేళ( p->p_క్రమం_By ) తిరిగి 0;
  p_సమాసం = p->pEList->a[0].p_సమాసం;
  ఒకవేళ( p_సమాసం->op!=TK_AGG_FUNCTION ) తిరిగి 0;        /* Result is an aggregate */
  ప్రతిజ్ఞా( సమాసం_Use_UToken(p_సమాసం) );
  ఒకవేళ( sqlite3_stricmp(p_సమాసం->u.zToken,"count") ) తిరిగి 0;  /* Is count() */
  ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
  ఒకవేళ( p_సమాసం->x.pList!=0 ) తిరిగి 0;                 /* Must be count(*) */
  ఒకవేళ( p->pSrc->nSrc!=1 ) తిరిగి 0;                  /* One table in FROM  */
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_WinFunc) ) తిరిగి 0;/* Not a window function */
  pFrom = p->pSrc->a;
  ఒకవేళ( pFrom->fg.isSubquery==0 ) తిరిగి 0;           /* FROM is a subquery */
  ఒకవేళ( (p->selFlags & SF_Correlated)!=0 ) తిరిగి 0;  /* Not a correlated subq */
  pSub = pFrom->u4.pSubq->pSelect;
  ఒకవేళ( pSub->p_పూర్వ==0 ) తిరిగి 0;                   /* Must be a compound */
  ఒకవేళ( pSub->selFlags & SF_CopyCte ) తిరిగి 0;       /* Not a CTE */
  చేయి{
    ఒకవేళ( pSub->op!=TK_ALL && pSub->p_పూర్వ ) తిరిగి 0;  /* Must be UNION ALL */
    ఒకవేళ( pSub->pWhere ) తిరిగి 0;                      /* No WHERE clause */
    ఒకవేళ( pSub->p_హద్దు ) తిరిగి 0;                      /* No LIMIT clause */
    ఒకవేళ( pSub->selFlags & (SF_Aggregate|SF_Distinct) ){
       testcase( pSub->selFlags & SF_Aggregate );
       testcase( pSub->selFlags & SF_Distinct );
       తిరిగి 0;     /* Not an aggregate nor DISTINCT */
    }
    ప్రతిజ్ఞా( pSub->pHaving==0 );  /* Due to the previous */
    pSub = pSub->p_పూర్వ;                              /* Repeat over compound */
  }అయితే( pSub );

  /* If we reach this point then it is OK to perform the transformation */

  db = pParse->db;
  p_లెక్క = p_సమాసం;
  p_సమాసం = 0;
  pSub = sqlite3SubqueryDetach(db, pFrom);
  sqlite3SrcListDelete(db, p->pSrc);
  p->pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  అయితే( pSub ){
    సమాసం *p_పదం;
    p_పూర్వ = pSub->p_పూర్వ;
    pSub->p_పూర్వ = 0;
    pSub->p_తదుపరి = 0;
    pSub->selFlags |= SF_Aggregate;
    pSub->selFlags &= ~(u32)SF_Compound;
    pSub->n_Select_అడ్డువరుస = 0;
    sqlite3_Parser_కూడు_Cleanup(pParse, sqlite3_సమాసం_List_Delete_Generic, pSub->pEList);
    p_పదం = p_పూర్వ ? sqlite3_సమాసం_Dup(db, p_లెక్క, 0) : p_లెక్క;
    pSub->pEList = sqlite3_సమాసం_List_Append(pParse, 0, p_పదం);
    p_పదం = sqlite3PExpr(pParse, TK_SELECT, 0, 0);
    sqlite3_PExpr_కూడు_Select(pParse, p_పదం, pSub);
    ఒకవేళ( p_సమాసం==0 ){
      p_సమాసం = p_పదం;
    }లేకపోతే{
      p_సమాసం = sqlite3PExpr(pParse, TK_PLUS, p_పదం, p_సమాసం);
    }
    ఒకవేళ( pParse->nErr ){
      sqlite3_సమాసం_Delete(db, p_సమాసం);
      p_సమాసం = 0;
    }
    pSub = p_పూర్వ;
  }
  p->pEList->a[0].p_సమాసం = p_సమాసం;
  p->selFlags &= ~(u32)SF_Aggregate;

#ఒకవేళ TREETRACE_ENABLED
  ఒకవేళ( sqlite3_వృక్షం_Trace & 0x200 ){
    TREETRACE(0x200,pParse,p,("After count-of-view optimization:\n"));
    sqlite3_వృక్షం_View_Select(0, p, 0);
  }
#అయితే_ముగింపు
  తిరిగి 1;
}

/*
** If any term of pSrc, or any SF_NestedFrom sub-query, is not the same
** as pSrcItem but has the same alias as p0, then return true.
** Otherwise return false.
*/
స్థిర పూర్ణాంకం sameSrcAlias(Src_అంశం *p0, SrcList *pSrc){
  పూర్ణాంకం i;
  కొరకు(i=0; i<pSrc->nSrc; i++){
    Src_అంశం *p1 = &pSrc->a[i];
    ఒకవేళ( p1==p0 ) కొనసాగు;
    ఒకవేళ( p0->pSTab==p1->pSTab && 0==sqlite3_stricmp(p0->zAlias, p1->zAlias) ){
      తిరిగి 1;
    }
    ఒకవేళ( p1->fg.isSubquery
     && (p1->u4.pSubq->pSelect->selFlags & SF_NestedFrom)!=0
     && sameSrcAlias(p0, p1->u4.pSubq->pSelect->pSrc)
    ){
      తిరిగి 1;
    }
  }
  తిరిగి 0;
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
స్థిర పూర్ణాంకం from_Clause_పదం_Can_Be_Coroutine(
  Parse *pParse,          /* Parsing context */
  SrcList *pTabList,      /* FROM clause */
  పూర్ణాంకం i,                  /* Which term of the FROM clause holds the subquery */
  పూర్ణాంకం selFlags            /* Flags on the SELECT statement */
){
  Src_అంశం *p_అంశం = &pTabList->a[i];
  ఒకవేళ( p_అంశం->fg.isCte ){
    స్థిరరాశి CteUse *pCteUse = p_అంశం->u2.pCteUse;
    ఒకవేళ( pCteUse->eM10d==M10d_Yes ) తిరిగి 0;                          /* (2a) */
    ఒకవేళ( pCteUse->nUse>=2 && pCteUse->eM10d!=M10d_No ) తిరిగి 0;       /* (2b) */
  }
  ఒకవేళ( pTabList->a[0].fg.jointype & JT_LTORJ ) తిరిగి 0;               /* (3)  */
  ఒకవేళ( OptimizationDisabled(pParse->db, SQLITE_Coroutines) ) తిరిగి 0; /* (4)  */
  ఒకవేళ( isSelfJoinView(pTabList, p_అంశం, i+1, pTabList->nSrc)!=0 ){
    తిరిగి 0;                                                          /* (5) */
  }
  ఒకవేళ( i==0 ){
    ఒకవేళ( pTabList->nSrc==1 ) తిరిగి 1;                             /* (1a) */
    ఒకవేళ( pTabList->a[1].fg.jointype & JT_CROSS ) తిరిగి 1;         /* (1b) */
    ఒకవేళ( selFlags & SF_UpdateFrom )              తిరిగి 0;         /* (1c-iii) */
    తిరిగి 1;
  }
  ఒకవేళ( selFlags & SF_UpdateFrom ) తిరిగి 0;                        /* (1c-iii) */
  అయితే( 1 /*exit-by-break*/ ){
    ఒకవేళ( p_అంశం->fg.jointype & (JT_OUTER|JT_CROSS)  ) తిరిగి 0;     /* (1c-ii) */
    ఒకవేళ( i==0 ) ఆపు;
    i--;
    p_అంశం--;
    ఒకవేళ( p_అంశం->fg.isSubquery ) తిరిగి 0;                          /* (1c-i) */
  }
  తిరిగి 1;
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
స్థిర SQLITE_NOINLINE శూన్య exists_గమ్యం_Join(
  Parse *pParse,  /* Parsing context */
  Select *p,      /* The SELECT statement being optimized */
  సమాసం *pWhere    /* part of the WHERE clause currently being examined */
){
  ఒకవేళ( pParse->nErr==0
   && pWhere!=0
   && !సమాసం_Has_Property(pWhere, EP_OuterON|EP_InnerON) 
   && ALWAYS(p->pSrc!=0)
   && p->pSrc->nSrc<BMS
   && (p->p_హద్దు==0 || p->p_హద్దు->p_కుడి==0)
  ){
    ఒకవేళ( pWhere->op==TK_AND ){
      సమాసం *p_కుడి = pWhere->p_కుడి;
      exists_గమ్యం_Join(pParse, p, pWhere->p_ఎడమ);
      exists_గమ్యం_Join(pParse, p, p_కుడి);
    }
    లేకపోతే ఒకవేళ( pWhere->op==TK_EXISTS ){
      Select *pSub = pWhere->x.pSelect;
      సమాసం *pSubWhere = pSub->pWhere;
      ఒకవేళ( pSub->pSrc->nSrc==1
       && (pSub->selFlags & SF_Aggregate)==0
       && !pSub->pSrc->a[0].fg.isSubquery
       && pSub->p_హద్దు==0
       && pSub->p_పూర్వ==0
      ){
        /* Before combining the sub-select with the parent, renumber the 
        ** cursor used by the subselect. This is because the EXISTS expression
        ** might be a copy of another EXISTS expression from somewhere
        ** else in the tree, and in this case it is important that it use
        ** a unique cursor number.  */
        sqlite3 *db = pParse->db;
        పూర్ణాంకం *aCsrMap = sqlite3DbMallocZero(db, (pParse->nTab+2)*పరిమాణం(పూర్ణాంకం));
        ఒకవేళ( aCsrMap==0 ) తిరిగి;
        aCsrMap[0] = (pParse->nTab+1);
        renumberCursors(pParse, pSub, -1, aCsrMap);
        sqlite3DbFree(db, aCsrMap);

        memset(pWhere, 0, పరిమాణం(*pWhere));
        pWhere->op = TK_INTEGER;
        pWhere->u.i_విలువ = 1;
        సమాసం_సమితి_Property(pWhere, EP_Int_విలువ);
        ప్రతిజ్ఞా( p->pWhere!=0 );
        pSub->pSrc->a[0].fg.fromExists = 1;
        p->pSrc = sqlite3SrcListAppendList(pParse, p->pSrc, pSub->pSrc);
        ఒకవేళ( pSubWhere ){
          p->pWhere = sqlite3PExpr(pParse, TK_AND, p->pWhere, pSubWhere);
          pSub->pWhere = 0;
        }
        pSub->pSrc = 0;
        sqlite3_Parser_కూడు_Cleanup(pParse, sqlite3SelectDeleteGeneric, pSub);
        recompute_Columns_వాడిన(p, &p->pSrc->a[p->pSrc->nSrc-1]);
#ఒకవేళ TREETRACE_ENABLED
        ఒకవేళ( sqlite3_వృక్షం_Trace & 0x100000 ){
          TREETRACE(0x100000,pParse,p,
                    ("After EXISTS-to-JOIN optimization:\n"));
          sqlite3_వృక్షం_View_Select(0, p, 0);
        }
#అయితే_ముగింపు
      }
    }
  }
}

/*
** Type used for Walker callbacks by selectCheckOnClauses().
*/
రకనిర్వచనం నిర్మాణం CheckOnCtx CheckOnCtx;
నిర్మాణం CheckOnCtx {
  SrcList *pSrc;       /* SrcList for this context */
  పూర్ణాంకం iJoin;           /* Cursors must be left of this one, if not negative */
  పూర్ణాంకం bFuncArg;        /* True for table-function arg */
  CheckOnCtx *p_మాతృ; /* Parent context */
};

/*
** True if the SrcList passed as the only argument contains at least
** one RIGHT or FULL JOIN. False otherwise.  
*/
#నిర్వచించు has_కుడి_Join(pSrc) (((pSrc)->a[0].fg.jointype & JT_LTORJ)!=0)

/*
** The xExpr callback for the search of invalid ON clause terms.
*/
స్థిర పూర్ణాంకం select_Check_On_Clauses_సమాసం(Walker *pWalker, సమాసం *p_సమాసం){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;

  /* Check if pExpr is root or near-root of an ON clause constraint that needs
  ** to be checked to ensure that it does not refer to tables in its FROM
  ** clause to the right of itself. i.e. it is either:
  **
  **   + an ON clause on an OUTER join, or
  **   + an ON clause on an INNER join within a FROM that features at
  **     least one RIGHT or FULL join.
  */
  ఒకవేళ( (సమాసం_Has_Property(p_సమాసం, EP_OuterON))
   || (సమాసం_Has_Property(p_సమాసం, EP_InnerON) && has_కుడి_Join(pCtx->pSrc))
  ){
    /* If CheckOnCtx.iJoin is already set, then fall through and process
    ** this expression node as normal. Or, if CheckOnCtx.iJoin is still 0,
    ** set it to the cursor number of the RHS of the join to which this
    ** ON expression was attached and then iterate through the entire 
    ** expression.  */
    ప్రతిజ్ఞా( pCtx->iJoin<0 || pCtx->iJoin==p_సమాసం->w.iJoin );
    ఒకవేళ( pCtx->iJoin<0 ){
      pCtx->iJoin = p_సమాసం->w.iJoin;
      sqlite3_Walk_సమాసం_NN(pWalker, p_సమాసం);
      pCtx->iJoin = -1;
      తిరిగి WRC_Prune;
    }
  }

  ఒకవేళ( p_సమాసం->op==TK_స్తంభం ){
    /* A column expression. Find the SrcList (if any) to which it refers.
    ** Then, if CheckOnCtx.iJoin indicates that this expression is part of an
    ** ON clause from that SrcList (i.e. if iJoin is non-zero), check that it
    ** does not refer to a table to the right of CheckOnCtx.iJoin. */
    పూర్ణాంకం iTab = p_సమాసం->i_పట్టిక;
    చేయి {
      SrcList *pSrc = pCtx->pSrc;
      పూర్ణాంకం nSrc = pSrc->nSrc;
      పూర్ణాంకం ii;
      కొరకు(ii=0; ii<nSrc && pSrc->a[ii].iCursor!=iTab; ii++){}
      ఒకవేళ( ii<nSrc ){
        /* pSrc is the FROM clause that contains iTab */
        ఒకవేళ( pCtx->iJoin>=0 ){
          కొరకు(ii--; ii>=0 && pSrc->a[ii].iCursor!=pCtx->iJoin; ii--){}
          ఒకవేళ( ii>=0 ){
            /* Table iJoin appears to the left of table iTab in the SrcList.
            ** Therefore the expression refers to a table to its right. */
            sqlite3_Error_సందేశం(pWalker->pParse, 
                "%s references tables to its right",
                (pCtx->bFuncArg ? "table-function argument" : "ON clause")
            );
            తిరిగి WRC_Abort;
          }
        }
        ఆపు;
      }
      pCtx = pCtx->p_మాతృ;
    }అయితే( pCtx );
  }
  తిరిగి WRC_Continue;
}

/*
** The xSelect callback for the search of invalid ON clause terms.
*/
స్థిర పూర్ణాంకం selectCheckOnClausesSelect(Walker *pWalker, Select *pSelect){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;
  ఒకవేళ( pSelect->pSrc==pCtx->pSrc || pSelect->pSrc->nSrc==0 ){
    తిరిగి WRC_Continue;
  }లేకపోతే{
    CheckOnCtx sCtx;
    memset(&sCtx, 0, పరిమాణం(sCtx));
    sCtx.pSrc = pSelect->pSrc;
    sCtx.p_మాతృ = pCtx;
    sCtx.iJoin = -1;
    pWalker->u.pCheckOnCtx = &sCtx;
    sqlite3WalkSelect(pWalker, pSelect);
    pWalker->u.pCheckOnCtx = pCtx;
    pSelect->selFlags &= ~SF_On_గమ్యం_Where;
    తిరిగి WRC_Prune;
  }
}

/*
** Check all ON clauses in pSelect to verify that they do not reference
** columns to the right.
*/
శూన్య sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect){
  Walker w;
  CheckOnCtx sCtx;
  పూర్ణాంకం ii;
  ప్రతిజ్ఞా( pSelect->selFlags & SF_On_గమ్యం_Where );
  ప్రతిజ్ఞా( pSelect->pSrc!=0 && pSelect->pSrc->nSrc>=2 );
  memset(&w, 0, పరిమాణం(w));
  w.pParse = pParse;
  w.x_సమాసం_Callback = select_Check_On_Clauses_సమాసం;
  w.xSelectCallback = selectCheckOnClausesSelect;
  w.u.pCheckOnCtx = &sCtx;
  memset(&sCtx, 0, పరిమాణం(sCtx));
  sCtx.pSrc = pSelect->pSrc;
  sCtx.iJoin = -1;
  sqlite3_Walk_సమాసం(&w, pSelect->pWhere);
  pSelect->selFlags &= ~SF_On_గమ్యం_Where;

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
  కొరకు(ii=0; ii<pSelect->pSrc->nSrc; ii++){
    Src_అంశం *p_అంశం = &pSelect->pSrc->a[ii];
    ఒకవేళ( p_అంశం->fg.isTabFunc ){
      sCtx.iJoin = -1;
      ఒకవేళ( (p_అంశం->fg.jointype & (JT_LTORJ|JT_కుడి)) ){
        sCtx.iJoin = p_అంశం->iCursor;
      }లేకపోతే{
        పూర్ణాంకం jj;
        కొరకు(jj=ii+1; jj<pSelect->pSrc->nSrc; jj++){
          Src_అంశం *pItem2 = &pSelect->pSrc->a[jj];
          ఒకవేళ( pItem2->fg.jointype & JT_OUTER ){
            sCtx.iJoin = pItem2[-1].iCursor;
            ఆపు;
          }
        }
      }
      ఒకవేళ( sCtx.iJoin>=0 ){
        sqlite3_Walk_సమాసం_List(&w, p_అంశం->u1.pFuncArg);
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
స్థిర పూర్ణాంకం sqlite3_Copy_క్రమీకరించు_క్రమం(సమాసం_List *p1, సమాసం_List *p2){
  ప్రతిజ్ఞా( p1 );
  ఒకవేళ( p2 && p1->n_సమాసం==p2->n_సమాసం ){
    పూర్ణాంకం ii;
    కొరకు(ii=0; ii<p1->n_సమాసం; ii++){
      u8 క్రమీకరించు_Flags;
      క్రమీకరించు_Flags = p2->a[ii].fg.క్రమీకరించు_Flags & KEYINFO_క్రమం_DESC;
      p1->a[ii].fg.క్రమీకరించు_Flags = క్రమీకరించు_Flags;
    }
    తిరిగి 1;
  }లేకపోతే{
    తిరిగి 0;
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
పూర్ణాంకం sqlite3Select(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  SelectDest *pDest      /* What to do with the query results */
){
  పూర్ణాంకం i, j;              /* Loop counters */
  WhereInfo *pWInfo;     /* Return from sqlite3WhereBegin() */
  Vdbe *v;               /* The virtual machine under construction */
  పూర్ణాంకం isAgg;             /* True for select lists like "count(*)" */
  సమాసం_List *pEList = 0;  /* List of columns to extract. */
  SrcList *pTabList;     /* List of tables to select from */
  సమాసం *pWhere;          /* The WHERE clause.  May be NULL */
  సమాసం_List *p_సమూహం_By;    /* The GROUP BY clause.  May be NULL */
  సమాసం *pHaving;         /* The HAVING clause.  May be NULL */
  AggInfo *pAggInfo = 0; /* Aggregate information */
  పూర్ణాంకం rc = 1;            /* Value to return from this function */
  DistinctCtx sDistinct; /* Info on how to code the DISTINCT keyword */
  క్రమీకరించు_Ctx s_క్రమీకరించు;         /* Info on how to code the ORDER BY clause */
  పూర్ణాంకం i_చివర;              /* Address of the end of the query */
  sqlite3 *db;           /* The database connection */
  సమాసం_List *p_Min_Max_క్రమం_By = 0;  /* Added ORDER BY for min/max queries */
  u8 minMaxFlag;                 /* Flag for min/max queries */

  db = pParse->db;
  ప్రతిజ్ఞా( pParse==db->pParse );
  v = sqlite3GetVdbe(pParse);
  ఒకవేళ( p==0 || pParse->nErr ){
    తిరిగి 1;
  }
  ప్రతిజ్ఞా( db->mallocFailed==0 );
  ఒకవేళ( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) తిరిగి 1;
#ఒకవేళ TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p, ("begin processing:\n", pParse->addrExplain));
  ఒకవేళ( sqlite3_వృక్షం_Trace & 0x10000 ){
    ఒకవేళ( (sqlite3_వృక్షం_Trace & 0x10001)==0x10000 ){
      sqlite3_వృక్షం_View_పంక్తి(0, "In sqlite3Select() at %s:%d",
                           __FILE__, __LINE__);
    }
    sqlite3ShowSelect(p);
  }
#అయితే_ముగింపు

  /* tag-select-0100 */
  ప్రతిజ్ఞా( p->p_క్రమం_By==0 || pDest->eDest!=SRT_DistFifo );
  ప్రతిజ్ఞా( p->p_క్రమం_By==0 || pDest->eDest!=SRT_Fifo );
  ప్రతిజ్ఞా( p->p_క్రమం_By==0 || pDest->eDest!=SRT_Dist_వరుస );
  ప్రతిజ్ఞా( p->p_క్రమం_By==0 || pDest->eDest!=SRT_వరుస );
  ఒకవేళ( IgnorableDistinct(pDest) ){
    ప్రతిజ్ఞా(pDest->eDest==SRT_Exists     || pDest->eDest==SRT_Discard ||
           pDest->eDest==SRT_Dist_వరుస  || pDest->eDest==SRT_DistFifo );
    /* All of these destinations are also able to ignore the ORDER BY clause */
    ఒకవేళ( p->p_క్రమం_By ){
#ఒకవేళ TREETRACE_ENABLED
      TREETRACE(0x800,pParse,p, ("dropping superfluous ORDER BY:\n"));
      ఒకవేళ( sqlite3_వృక్షం_Trace & 0x800 ){
        sqlite3_వృక్షం_View_సమాసం_List(0, p->p_క్రమం_By, 0, "ORDERBY");
      }
#అయితే_ముగింపు   
      sqlite3_Parser_కూడు_Cleanup(pParse, sqlite3_సమాసం_List_Delete_Generic,
                              p->p_క్రమం_By);
      testcase( pParse->earlyCleanup );
      p->p_క్రమం_By = 0;
    }
    p->selFlags &= ~(u32)SF_Distinct;
  }
  sqlite3SelectPrep(pParse, p, 0);
  ఒకవేళ( pParse->nErr ){
    వెళ్ళు select_చివర;
  }
  ప్రతిజ్ఞా( db->mallocFailed==0 );
  ప్రతిజ్ఞా( p->pEList!=0 );
#ఒకవేళ TREETRACE_ENABLED
  ఒకవేళ( sqlite3_వృక్షం_Trace & 0x10 ){
    TREETRACE(0x10,pParse,p, ("after name resolution:\n"));
    sqlite3_వృక్షం_View_Select(0, p, 0);
  }
#అయితే_ముగింపు

  /* If the SF_UFSrcCheck flag is set, then this function is being called
  ** as part of populating the temp table for an UPDATE...FROM statement.
  ** In this case, it is an error if the target object (pSrc->a[0]) name
  ** or alias is duplicated within FROM clause (pSrc->a[1..n]). 
  **
  ** Postgres disallows this case too. The reason is that some other
  ** systems handle this case differently, and not all the same way,
  ** which is just confusing. To avoid this, we follow PG's lead and
  ** disallow it altogether.  */
  ఒకవేళ( p->selFlags & SF_UFSrcCheck ){
    Src_అంశం *p0 = &p->pSrc->a[0];
    ఒకవేళ( sameSrcAlias(p0, p->pSrc) ){
      sqlite3_Error_సందేశం(pParse,
          "target object/alias may not appear in FROM clause: %s",
          p0->zAlias ? p0->zAlias : p0->pSTab->zName
      );
      వెళ్ళు select_చివర;
    }

    /* Clear the SF_UFSrcCheck flag. The check has already been performed,
    ** and leaving this flag set can cause errors if a compound sub-query
    ** in p->pSrc is flattened into this query and this function called
    ** again as part of compound SELECT processing.  */
    p->selFlags &= ~(u32)SF_UFSrcCheck;
  }

  ఒకవేళ( pDest->eDest==SRT_Output ){
    sqlite3_Generate_స్తంభం_పేర్లు(pParse, p);
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
  ఒకవేళ( sqlite3_కిటికీ_Rewrite(pParse, p) ){
    ప్రతిజ్ఞా( pParse->nErr );
    వెళ్ళు select_చివర;
  }
#ఒకవేళ TREETRACE_ENABLED
  ఒకవేళ( p->pWin && (sqlite3_వృక్షం_Trace & 0x40)!=0 ){
    TREETRACE(0x40,pParse,p, ("after window rewrite:\n"));
    sqlite3_వృక్షం_View_Select(0, p, 0);
  }
#అయితే_ముగింపు
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */
  pTabList = p->pSrc;
  isAgg = (p->selFlags & SF_Aggregate)!=0;
  memset(&s_క్రమీకరించు, 0, పరిమాణం(s_క్రమీకరించు));
  s_క్రమీకరించు.p_క్రమం_By = p->p_క్రమం_By;

  /* Try to do various optimizations (flattening subqueries, and strength
  ** reduction of join operators) in the FROM clause up into the main query
  ** tag-select-0200
  */
#ఒకవేళ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
  కొరకు(i=0; !p->p_పూర్వ && i<pTabList->nSrc; i++){
    Src_అంశం *p_అంశం = &pTabList->a[i];
    Select *pSub = p_అంశం->fg.isSubquery ? p_అంశం->u4.pSubq->pSelect : 0;
    పట్టిక *pTab = p_అంశం->pSTab;

    /* The expander should have already created transient Table objects
    ** even for FROM clause elements such as subqueries that do not correspond
    ** to a real table */
    ప్రతిజ్ఞా( pTab!=0 );

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
    ఒకవేళ( (p_అంశం->fg.jointype & (JT_ఎడమ|JT_LTORJ))!=0
     && sqlite3_సమాసం_సూచిస్తుంది_Non_Null_అడ్డువరుస(p->pWhere, p_అంశం->iCursor,
                                     p_అంశం->fg.jointype & JT_LTORJ)
     && OptimizationEnabled(db, SQLITE_SimplifyJoin)
    ){
      ఒకవేళ( p_అంశం->fg.jointype & JT_ఎడమ ){
        ఒకవేళ( p_అంశం->fg.jointype & JT_కుడి ){
          TREETRACE(0x1000,pParse,p,
                    ("FULL-JOIN simplifies to RIGHT-JOIN on term %d\n",i));
          p_అంశం->fg.jointype &= ~JT_ఎడమ;
        }లేకపోతే{
          TREETRACE(0x1000,pParse,p,
                    ("LEFT-JOIN simplifies to JOIN on term %d\n",i));
          p_అంశం->fg.jointype &= ~(JT_ఎడమ|JT_OUTER);
          unset_Join_సమాసం(p->pWhere, p_అంశం->iCursor, 0);
        }
      }
      ఒకవేళ( p_అంశం->fg.jointype & JT_LTORJ ){
        కొరకు(j=i+1; j<pTabList->nSrc; j++){
          Src_అంశం *pI2 = &pTabList->a[j];
          ఒకవేళ( pI2->fg.jointype & JT_కుడి ){
            ఒకవేళ( pI2->fg.jointype & JT_ఎడమ ){
              TREETRACE(0x1000,pParse,p,
                        ("FULL-JOIN simplifies to LEFT-JOIN on term %d\n",j));
              pI2->fg.jointype &= ~JT_కుడి;
            }లేకపోతే{
              TREETRACE(0x1000,pParse,p,
                        ("RIGHT-JOIN simplifies to JOIN on term %d\n",j));
              pI2->fg.jointype &= ~(JT_కుడి|JT_OUTER);
              unset_Join_సమాసం(p->pWhere, pI2->iCursor, 1);
            }
          }
        }
        కొరకు(j=pTabList->nSrc-1; j>=0; j--){
          pTabList->a[j].fg.jointype &= ~JT_LTORJ;
          ఒకవేళ( pTabList->a[j].fg.jointype & JT_కుడి ) ఆపు;
        }
      }
    }

    /* No further action if this term of the FROM clause is not a subquery */
    ఒకవేళ( pSub==0 ) కొనసాగు;

    /* Catch mismatch in the declared columns of a view and the number of
    ** columns in the SELECT on the RHS */
    ఒకవేళ( pTab->n_నిలువు!=pSub->pEList->n_సమాసం ){
      sqlite3_Error_సందేశం(pParse, "expected %d columns for '%s' but got %d",
                      pTab->n_నిలువు, pTab->zName, pSub->pEList->n_సమాసం);
      వెళ్ళు select_చివర;
    }

    /* Do not attempt the usual optimizations (flattening and ORDER BY
    ** elimination) on a MATERIALIZED common table expression because
    ** a MATERIALIZED common table expression is an optimization fence.
    */
    ఒకవేళ( p_అంశం->fg.isCte && p_అంశం->u2.pCteUse->eM10d==M10d_Yes ){
      కొనసాగు;
    }

    /* Do not try to flatten an aggregate subquery.
    **
    ** Flattening an aggregate subquery is only possible if the outer query
    ** is not a join.  But if the outer query is not a join, then the subquery
    ** will be implemented as a co-routine and there is no advantage to
    ** flattening in that case.
    */
    ఒకవేళ( (pSub->selFlags & SF_Aggregate)!=0 ) కొనసాగు;
    ప్రతిజ్ఞా( pSub->p_సమూహం_By==0 );

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
    ఒకవేళ( pSub->p_క్రమం_By!=0
     && (p->p_క్రమం_By!=0 || pTabList->nSrc>1)      /* Condition (5) */
     && pSub->p_హద్దు==0                           /* Condition (1) */
     && (pSub->selFlags & (SF_క్రమం_By_Reqd|SF_Recursive))==0  /* (2) and (6) */
     && (p->selFlags & SF_క్రమం_By_Reqd)==0         /* Condition (3) and (4) */
     && OptimizationEnabled(db, SQLITE_Omit_క్రమం_By)
    ){
      TREETRACE(0x800,pParse,p,
                ("omit superfluous ORDER BY on %r FROM-clause subquery\n",i+1));
      sqlite3_Parser_కూడు_Cleanup(pParse, sqlite3_సమాసం_List_Delete_Generic,
                              pSub->p_క్రమం_By);
      pSub->p_క్రమం_By = 0;
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
    ఒకవేళ( pSub->p_క్రమం_By!=0
     && i==0
     && (p->selFlags & SF_Complex_ఫలితం)!=0
     && (pTabList->nSrc==1
         || (pTabList->a[1].fg.jointype&(JT_OUTER|JT_CROSS))!=0)
    ){
      కొనసాగు;
    }

    /* tag-select-0240 */
    ఒకవేళ( flattenSubquery(pParse, p, i, isAgg) ){
      ఒకవేళ( pParse->nErr ) వెళ్ళు select_చివర;
      /* This subquery can be absorbed into its parent. */
      i = -1;
    }
    pTabList = p->pSrc;
    ఒకవేళ( db->mallocFailed ) వెళ్ళు select_చివర;
    ఒకవేళ( !IgnorableOrderby(pDest) ){
      s_క్రమీకరించు.p_క్రమం_By = p->p_క్రమం_By;
    }
  }
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_COMPOUND_SELECT
  /* Handle compound SELECT statements using the separate multiSelect()
  ** procedure.  tag-select-0300
  */
  ఒకవేళ( p->p_పూర్వ ){
    rc = multiSelect(pParse, p, pDest);
#ఒకవేళ TREETRACE_ENABLED
    TREETRACE(0x400,pParse,p,("end compound-select processing\n"));
    ఒకవేళ( (sqlite3_వృక్షం_Trace & 0x400)!=0 && Explain_Query_Plan_మాతృ(pParse)==0 ){
      sqlite3_వృక్షం_View_Select(0, p, 0);
    }
#అయితే_ముగింపు
    ఒకవేళ( p->p_తదుపరి==0 ) Explain_Query_Plan_తీయి(pParse);
    తిరిగి rc;
  }
#అయితే_ముగింపు

  /* If there may be an "EXISTS (SELECT ...)" in the WHERE clause, attempt
  ** to change it into a join.  */
  ఒకవేళ( pParse->bHasExists && OptimizationEnabled(db,SQLITE_Exists_గమ్యం_Join) ){
    exists_గమ్యం_Join(pParse, p, p->pWhere);
    pTabList = p->pSrc;
  }

  /* Do the WHERE-clause constant propagation optimization if this is
  ** a join.  No need to spend time on this operation for non-join queries
  ** as the equivalent optimization will be handled by query planner in
  ** sqlite3WhereBegin().  tag-select-0330
  */
  ఒకవేళ( p->pWhere!=0
   && p->pWhere->op==TK_AND
   && OptimizationEnabled(db, SQLITE_PropagateConst)
   && propagateConstants(pParse, p)
  ){
#ఒకవేళ TREETRACE_ENABLED
    ఒకవేళ( sqlite3_వృక్షం_Trace & 0x2000 ){
      TREETRACE(0x2000,pParse,p,("After constant propagation:\n"));
      sqlite3_వృక్షం_View_Select(0, p, 0);
    }
#అయితే_ముగింపు
  }లేకపోతే{
    TREETRACE(0x2000,pParse,p,("Constant propagation not helpful\n"));
  }

  /* tag-select-0350 */
  ఒకవేళ( OptimizationEnabled(db, SQLITE_QueryFlattener|SQLITE_లెక్క_యొక్క_View)
   && లెక్క_యొక్క_View_Optimization(pParse, p)
  ){
    ఒకవేళ( db->mallocFailed ) వెళ్ళు select_చివర;
    pTabList = p->pSrc;
  }

  /* Loop over all terms in the FROM clause and do two things for each term:
  **
  **   (1) Authorize unreferenced tables
  **   (2) Generate code for all sub-queries
  **
  ** tag-select-0400
  */
  కొరకు(i=0; i<pTabList->nSrc; i++){
    Src_అంశం *p_అంశం = &pTabList->a[i];
    Src_అంశం *p_పూర్వ;
    SelectDest dest;
    Subquery *pSubq;
    Select *pSub;
#ఒకవేళ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    స్థిరరాశి అక్షరం *zSavedAuthContext;
#అయితే_ముగింపు

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
    ఒకవేళ( p_అంశం->నిలువు_వాడిన==0 && p_అంశం->zName!=0 ){
      స్థిరరాశి అక్షరం *zDb;
      ఒకవేళ( p_అంశం->fg.fixedSchema ){
        పూర్ణాంకం iDb = sqlite3_Schema_గమ్యం_సూచీ(pParse->db, p_అంశం->u4.pSchema);
        zDb = db->aDb[iDb].zDbSName;
      }లేకపోతే ఒకవేళ( p_అంశం->fg.isSubquery ){
        zDb = 0;
      }లేకపోతే{
        zDb = p_అంశం->u4.zDatabase;
      }
      sqlite3AuthCheck(pParse, SQLITE_READ, p_అంశం->zName, "", zDb);
    }

#ఒకవేళ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    /* Generate code for all sub-queries in the FROM clause
    */
    ఒకవేళ( p_అంశం->fg.isSubquery==0 ) కొనసాగు;
    pSubq = p_అంశం->u4.pSubq;
    ప్రతిజ్ఞా( pSubq!=0 );
    pSub = pSubq->pSelect;

    /* The code for a subquery should only be generated once. */
    ఒకవేళ( pSubq->addrFillSub!=0 ) కొనసాగు;

    /* Increment Parse.nHeight by the height of the largest expression
    ** tree referred to by this, the parent select. The child select
    ** may contain expression trees of at most
    ** (SQLITE_MAX_EXPR_DEPTH-Parse.nHeight) height. This is a bit
    ** more conservative than necessary, but much easier than enforcing
    ** an exact limit.
    */
    pParse->n_ఎత్తు += sqlite3_Select_సమాసం_ఎత్తు(p);

    /* Make copies of constant WHERE-clause terms in the outer query down
    ** inside the subquery.  This can help the subquery to run more efficiently.
    ** This is the "predicate push-down optimization".  tag-select-0420
    */
    ఒకవేళ( OptimizationEnabled(db, SQLITE_నెట్టు_కిందికి)
     && (p_అంశం->fg.isCte==0
         || (p_అంశం->u2.pCteUse->eM10d!=M10d_Yes && p_అంశం->u2.pCteUse->nUse<2))
     && నెట్టు_కిందికి_Where_Terms(pParse, pSub, p->pWhere, pTabList, i)
    ){
#ఒకవేళ TREETRACE_ENABLED
      ఒకవేళ( sqlite3_వృక్షం_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("After WHERE-clause push-down into subquery %d:\n", pSub->selId));
        sqlite3_వృక్షం_View_Select(0, p, 0);
      }
#అయితే_ముగింపు
      ప్రతిజ్ఞా( pSubq->pSelect && (pSub->selFlags & SF_నెట్టు_కిందికి)!=0 );
    }లేకపోతే{
      TREETRACE(0x4000,pParse,p,("WHERE-clause push-down not possible\n"));
    }

    /* Convert unused result columns of the subquery into simple NULL
    ** expressions, to avoid unneeded searching and computation.
    ** tag-select-0440
    */
    ఒకవేళ( OptimizationEnabled(db, SQLITE_Null_వాడని_నిలువుల_సంఖ్య)
     && disable_వాడని_Subquery_ఫలితం_Columns(p_అంశం)
    ){
#ఒకవేళ TREETRACE_ENABLED
      ఒకవేళ( sqlite3_వృక్షం_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("Change unused result columns to NULL for subquery %d:\n",
             pSub->selId));
        sqlite3_వృక్షం_View_Select(0, p, 0);
      }
#అయితే_ముగింపు
    }

    zSavedAuthContext = pParse->zAuthContext;
    pParse->zAuthContext = p_అంశం->zName;

    /* Generate byte-code to implement the subquery  tag-select-0480
    */
    ఒకవేళ( from_Clause_పదం_Can_Be_Coroutine(pParse, pTabList, i, p->selFlags) ){
      /* Implement a co-routine that will return a single row of the result
      ** set on each invocation.  tag-select-0482
      */
      పూర్ణాంకం addr_శిఖరం = sqlite3VdbeCurrentAddr(v)+1;
    
      pSubq->regReturn = ++pParse->nMem;
      sqlite3_Vdbe_కూడు_Op3(v, OP_InitCoroutine, pSubq->regReturn, 0, addr_శిఖరం);
      VdbeComment((v, "%!S", p_అంశం));
      pSubq->addrFillSub = addr_శిఖరం;
      sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);
      ExplainQueryPlan((pParse, 1, "CO-ROUTINE %!S", p_అంశం));
      sqlite3Select(pParse, pSub, &dest);
      p_అంశం->pSTab->n_అడ్డువరుస_Log_Est = pSub->n_Select_అడ్డువరుస;
      p_అంశం->fg.ద్వారా_Coroutine = 1;
      pSubq->reg_ఫలితం = dest.iSdst;
      sqlite3_Vdbe_చివర_Coroutine(v, pSubq->regReturn);
      VdbeComment((v, "end %!S", p_అంశం));
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addr_శిఖరం-1);
      sqlite3_Clear_తాత్కాలిక_Reg_Cache(pParse);
    }లేకపోతే ఒకవేళ( p_అంశం->fg.isCte && p_అంశం->u2.pCteUse->addrM9e>0 ){
      /* This is a CTE for which materialization code has already been
      ** generated.  Invoke the subroutine to compute the materialization,
      ** then make the pItem->iCursor be a copy of the ephemeral table that
      ** holds the result of the materialization. tag-select-0484 */
      CteUse *pCteUse = p_అంశం->u2.pCteUse;
      sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, pCteUse->regRtn, pCteUse->addrM9e);
      ఒకవేళ( p_అంశం->iCursor!=pCteUse->iCur ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_OpenDup, p_అంశం->iCursor, pCteUse->iCur);
        VdbeComment((v, "%!S", p_అంశం));
      }
      pSub->n_Select_అడ్డువరుస = pCteUse->n_అడ్డువరుస_Est;
    }లేకపోతే ఒకవేళ( (p_పూర్వ = isSelfJoinView(pTabList, p_అంశం, 0, i))!=0 ){
      /* This view has already been materialized by a prior entry in
      ** this same FROM clause.  Reuse it.  tag-select-0486 */
      Subquery *p_పూర్వ_Subq;
      ప్రతిజ్ఞా( p_పూర్వ->fg.isSubquery );
      p_పూర్వ_Subq = p_పూర్వ->u4.pSubq;
      ప్రతిజ్ఞా( p_పూర్వ_Subq!=0 );
      ఒకవేళ( p_పూర్వ_Subq->addrFillSub ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, p_పూర్వ_Subq->regReturn,
                                       p_పూర్వ_Subq->addrFillSub);
      }
      sqlite3_Vdbe_కూడు_Op2(v, OP_OpenDup, p_అంశం->iCursor, p_పూర్వ->iCursor);
      pSub->n_Select_అడ్డువరుస = p_పూర్వ_Subq->pSelect->n_Select_అడ్డువరుస;
    }లేకపోతే{
      /* Materialize the view.  If the view is not correlated, generate a
      ** subroutine to do the materialization so that subsequent uses of
      ** the same view can reuse the materialization.  tag-select-0488 */
      పూర్ణాంకం శిఖరం_Addr;
      పూర్ణాంకం onceAddr = 0;
#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
      పూర్ణాంకం addrExplain;
#అయితే_ముగింపు

      pSubq->regReturn = ++pParse->nMem;
      శిఖరం_Addr = sqlite3_Vdbe_కూడు_Op0(v, OP_Goto);
      pSubq->addrFillSub = శిఖరం_Addr+1;
      p_అంశం->fg.isMaterialized = 1;
      ఒకవేళ( p_అంశం->fg.isCorrelated==0 ){
        /* If the subquery is not correlated and if we are not inside of
        ** a trigger, then we only need to compute the value of the subquery
        ** once. */
        onceAddr = sqlite3_Vdbe_కూడు_Op0(v, OP_Once); VdbeCoverage(v);
        VdbeComment((v, "materialize %!S", p_అంశం));
      }లేకపోతే{
        VdbeNoopComment((v, "materialize %!S", p_అంశం));
      }
      sqlite3SelectDestInit(&dest, SRT_EphemTab, p_అంశం->iCursor);

      ExplainQueryPlan2(addrExplain, (pParse, 1, "MATERIALIZE %!S", p_అంశం));
      sqlite3Select(pParse, pSub, &dest);
      p_అంశం->pSTab->n_అడ్డువరుస_Log_Est = pSub->n_Select_అడ్డువరుస;
      ఒకవేళ( onceAddr ) sqlite3_Vdbe_Jump_ఇక్కడ(v, onceAddr);
      sqlite3_Vdbe_కూడు_Op2(v, OP_Return, pSubq->regReturn, శిఖరం_Addr+1);
      VdbeComment((v, "end %!S", p_అంశం));
      sqlite3_Vdbe_పరిశీలన_స్థితి_Range(v, addrExplain, addrExplain, -1);
      sqlite3_Vdbe_Jump_ఇక్కడ(v, శిఖరం_Addr);
      sqlite3_Clear_తాత్కాలిక_Reg_Cache(pParse);
      ఒకవేళ( p_అంశం->fg.isCte && p_అంశం->fg.isCorrelated==0 ){
        CteUse *pCteUse = p_అంశం->u2.pCteUse;
        pCteUse->addrM9e = pSubq->addrFillSub;
        pCteUse->regRtn = pSubq->regReturn;
        pCteUse->iCur = p_అంశం->iCursor;
        pCteUse->n_అడ్డువరుస_Est = pSub->n_Select_అడ్డువరుస;
      }
    }
    ఒకవేళ( db->mallocFailed ) వెళ్ళు select_చివర;
    pParse->n_ఎత్తు -= sqlite3_Select_సమాసం_ఎత్తు(p);
    pParse->zAuthContext = zSavedAuthContext;
#అయితే_ముగింపు
  }

  /* Various elements of the SELECT copied into local variables for
  ** convenience */
  pEList = p->pEList;
  pWhere = p->pWhere;
  p_సమూహం_By = p->p_సమూహం_By;
  pHaving = p->pHaving;
  sDistinct.isTnct = (p->selFlags & SF_Distinct)!=0;

#ఒకవేళ TREETRACE_ENABLED
  ఒకవేళ( sqlite3_వృక్షం_Trace & 0x8000 ){
    TREETRACE(0x8000,pParse,p,("After all FROM-clause analysis:\n"));
    sqlite3_వృక్షం_View_Select(0, p, 0);
  }
#అయితే_ముగింపు

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
  ఒకవేళ( (p->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct
   && sqlite3_Copy_క్రమీకరించు_క్రమం(pEList, s_క్రమీకరించు.p_క్రమం_By)
   && sqlite3_సమాసం_List_Compare(pEList, s_క్రమీకరించు.p_క్రమం_By, -1)==0
   && OptimizationEnabled(db, SQLITE_సమూహం_By_క్రమం)
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
   && p->pWin==0
#అయితే_ముగింపు
  ){
    p->selFlags &= ~(u32)SF_Distinct;
    p_సమూహం_By = p->p_సమూహం_By = sqlite3_సమాసం_List_Dup(db, pEList, 0);
    ఒకవేళ( p_సమూహం_By ){
      కొరకు(i=0; i<p_సమూహం_By->n_సమాసం; i++){
        p_సమూహం_By->a[i].u.x.i_క్రమం_By_నిలువు = i+1;
      }
    }
    p->selFlags |= SF_Aggregate;
    /* Notice that even thought SF_Distinct has been cleared from p->selFlags,
    ** the sDistinct.isTnct is still set.  Hence, isTnct represents the
    ** original setting of the SF_Distinct flag, not the current setting */
    ప్రతిజ్ఞా( sDistinct.isTnct );
    sDistinct.isTnct = 2;

#ఒకవేళ TREETRACE_ENABLED
    ఒకవేళ( sqlite3_వృక్షం_Trace & 0x20000 ){
      TREETRACE(0x20000,pParse,p,("Transform DISTINCT into GROUP BY:\n"));
      sqlite3_వృక్షం_View_Select(0, p, 0);
    }
#అయితే_ముగింపు
  }

  /* If there is an ORDER BY clause, then create an ephemeral index to
  ** do the sorting.  But this sorting ephemeral index might end up
  ** being unused if the data can be extracted in pre-sorted order.
  ** If that is the case, then the OP_OpenEphemeral instruction will be
  ** changed to an OP_Noop once we figure out that the sorting index is
  ** not needed.  The sSort.addrSortIndex variable is used to facilitate
  ** that change.  tag-select-0600
  */
  ఒకవేళ( s_క్రమీకరించు.p_క్రమం_By ){
    కీలకం_Info *p_కీలకం_Info;
    p_కీలకం_Info = sqlite3_కీలకం_Info_From_సమాసం_List(
        pParse, s_క్రమీకరించు.p_క్రమం_By, 0, pEList->n_సమాసం);
    s_క్రమీకరించు.iECursor = pParse->nTab++;
    s_క్రమీకరించు.addr_క్రమీకరించు_సూచీ =
      sqlite3_Vdbe_కూడు_Op4(v, OP_OpenEphemeral,
          s_క్రమీకరించు.iECursor, s_క్రమీకరించు.p_క్రమం_By->n_సమాసం+1+pEList->n_సమాసం, 0,
          (అక్షరం*)p_కీలకం_Info, P4_KEYINFO
      );
  }లేకపోతే{
    s_క్రమీకరించు.addr_క్రమీకరించు_సూచీ = -1;
  }

  /* If the output is destined for a temporary table, open that table.
  ** tag-select-0630
  */
  ఒకవేళ( pDest->eDest==SRT_EphemTab ){
    sqlite3_Vdbe_కూడు_Op2(v, OP_OpenEphemeral, pDest->iSDParm, pEList->n_సమాసం);
    ఒకవేళ( p->selFlags & SF_NestedFrom ){
      /* Delete or NULL-out result columns that will never be used */
      పూర్ణాంకం ii;
      కొరకు(ii=pEList->n_సమాసం-1; ii>0 && pEList->a[ii].fg.b_వాడిన==0; ii--){
        sqlite3_సమాసం_Delete(db, pEList->a[ii].p_సమాసం);
        sqlite3DbFree(db, pEList->a[ii].zEName);
        pEList->n_సమాసం--;
      }
      కొరకు(ii=0; ii<pEList->n_సమాసం; ii++){
        ఒకవేళ( pEList->a[ii].fg.b_వాడిన==0 ) pEList->a[ii].p_సమాసం->op = TK_NULL;
      }
    }
  }

  /* Set the limiter.  tag-select-0650
  */
  i_చివర = sqlite3VdbeMakeLabel(pParse);
  ఒకవేళ( (p->selFlags & SF_Fixed_హద్దు)==0 ){
    p->n_Select_అడ్డువరుస = 320;  /* 4 billion rows */
  }
  ఒకవేళ( p->p_హద్దు ) compute_హద్దు_Registers(pParse, p, i_చివర);
  ఒకవేళ( p->i_హద్దు==0 && s_క్రమీకరించు.addr_క్రమీకరించు_సూచీ>=0 ){
    sqlite3VdbeChangeOpcode(v, s_క్రమీకరించు.addr_క్రమీకరించు_సూచీ, OP_SorterOpen);
    s_క్రమీకరించు.క్రమీకరించు_Flags |= SORTFLAG_UseSorter;
  }

  /* Open an ephemeral index to use for the distinct set. tag-select-0680
  */
  ఒకవేళ( p->selFlags & SF_Distinct ){
    sDistinct.tabTnct = pParse->nTab++;
    sDistinct.addrTnct = sqlite3_Vdbe_కూడు_Op4(v, OP_OpenEphemeral,
                       sDistinct.tabTnct, 0, 0,
                       (అక్షరం*)sqlite3_కీలకం_Info_From_సమాసం_List(pParse, p->pEList,0,0),
                       P4_KEYINFO);
    sqlite3VdbeChangeP5(v, BTREE_UNORDERED);
    sDistinct.eTnctType = WHERE_DISTINCT_UNORDERED;
  }లేకపోతే{
    sDistinct.eTnctType = WHERE_DISTINCT_NOOP;
  }

  ఒకవేళ( !isAgg && p_సమూహం_By==0 ){
    /* No aggregate functions and no GROUP BY clause.  tag-select-0700 */
    u16 wctrlFlags = (sDistinct.isTnct ? WHERE_WANT_DISTINCT : 0)
                   | (p->selFlags & SF_Fixed_హద్దు);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    కిటికీ *pWin = p->pWin;      /* Main window object (or NULL) */
    ఒకవేళ( pWin ){
      sqlite3_కిటికీ_సంకేతం_Init(pParse, p);
    }
#అయితే_ముగింపు
    ప్రతిజ్ఞా( WHERE_USE_హద్దు==SF_Fixed_హద్దు );


    /* Begin the database scan. */
    TREETRACE(0x2,pParse,p,("WhereBegin\n"));
    pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, s_క్రమీకరించు.p_క్రమం_By,
                               p->pEList, p, wctrlFlags, p->n_Select_అడ్డువరుస);
    ఒకవేళ( pWInfo==0 ) వెళ్ళు select_చివర;
    ఒకవేళ( sqlite3_Where_Output_అడ్డువరుస_లెక్క(pWInfo) < p->n_Select_అడ్డువరుస ){
      p->n_Select_అడ్డువరుస = sqlite3_Where_Output_అడ్డువరుస_లెక్క(pWInfo);
      ఒకవేళ( pDest->eDest<=SRT_Dist_వరుస && pDest->eDest>=SRT_DistFifo ){
        /* TUNING: For a UNION CTE, because UNION is implies DISTINCT,
        ** reduce the estimated output row count by 8 (LogEst 30). 
        ** Search for tag-20250414a to see other cases */
        p->n_Select_అడ్డువరుస -= 30;
      }
    }
    ఒకవేళ( sDistinct.isTnct && sqlite3WhereIsDistinct(pWInfo) ){
      sDistinct.eTnctType = sqlite3WhereIsDistinct(pWInfo);
    }
    ఒకవేళ( s_క్రమీకరించు.p_క్రమం_By ){
      s_క్రమీకరించు.nOBSat = sqlite3WhereIsOrdered(pWInfo);
      s_క్రమీకరించు.labelOBLopt = sqlite3_Where_క్రమం_By_హద్దు_Opt_Label(pWInfo);
      ఒకవేళ( s_క్రమీకరించు.nOBSat==s_క్రమీకరించు.p_క్రమం_By->n_సమాసం ){
        s_క్రమీకరించు.p_క్రమం_By = 0;
      }
    }
    TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));

    /* If sorting index that was created by a prior OP_OpenEphemeral
    ** instruction ended up not being needed, then change the OP_OpenEphemeral
    ** into an OP_Noop.
    */
    ఒకవేళ( s_క్రమీకరించు.addr_క్రమీకరించు_సూచీ>=0 && s_క్రమీకరించు.p_క్రమం_By==0 ){
      sqlite3_Vdbe_Change_గమ్యం_Noop(v, s_క్రమీకరించు.addr_క్రమీకరించు_సూచీ);
    }

    ప్రతిజ్ఞా( p->pEList==pEList );
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    ఒకవేళ( pWin ){
      పూర్ణాంకం addrGosub = sqlite3VdbeMakeLabel(pParse);
      పూర్ణాంకం iCont = sqlite3VdbeMakeLabel(pParse);
      పూర్ణాంకం iBreak = sqlite3VdbeMakeLabel(pParse);
      పూర్ణాంకం regGosub = ++pParse->nMem;

      sqlite3_కిటికీ_సంకేతం_అడుగు(pParse, p, pWInfo, regGosub, addrGosub);

      sqlite3_Vdbe_కూడు_Op2(v, OP_Goto, 0, iBreak);
      sqlite3VdbeResolveLabel(v, addrGosub);
      VdbeNoopComment((v, "inner-loop subroutine"));
      s_క్రమీకరించు.labelOBLopt = 0;
      selectInnerLoop(pParse, p, -1, &s_క్రమీకరించు, &sDistinct, pDest, iCont, iBreak);
      sqlite3VdbeResolveLabel(v, iCont);
      sqlite3_Vdbe_కూడు_Op1(v, OP_Return, regGosub);
      VdbeComment((v, "end inner-loop subroutine"));
      sqlite3VdbeResolveLabel(v, iBreak);
    }లేకపోతే
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */
    {
      /* Use the standard inner loop. */
      selectInnerLoop(pParse, p, -1, &s_క్రమీకరించు, &sDistinct, pDest,
          sqlite3WhereContinueLabel(pWInfo),
          sqlite3WhereBreakLabel(pWInfo));

      /* End the database scan loop.
      */
      TREETRACE(0x2,pParse,p,("WhereEnd\n"));
      sqlite3_Where_చివర(pWInfo);
    }
  }లేకపోతే{
    /* This case is for when there exist aggregate functions or a GROUP BY
    ** clause or both.  tag-select-0800 */
    NameContext sNC;    /* Name context for processing aggregate information */
    పూర్ణాంకం iAMem;          /* First Mem address for storing current GROUP BY */
    పూర్ణాంకం iBMem;          /* First Mem address for previous GROUP BY */
    పూర్ణాంకం iUseFlag;       /* Mem address holding flag indicating that at least
                        ** one row of the input to the aggregator has been
                        ** processed */
    పూర్ణాంకం iAbortFlag;     /* Mem address which causes query abort if positive */
    పూర్ణాంకం సమూహం_By_క్రమీకరించు;    /* Rows come from source in GROUP BY order */
    పూర్ణాంకం addr_చివర;        /* End of processing for this SELECT */
    పూర్ణాంకం క్రమీకరించు_PTab = 0;   /* Pseudotable used to decode sorting results */
    పూర్ణాంకం క్రమీకరించు_బహిర్గతం = 0;    /* Output register from the sorter */
    పూర్ణాంకం క్రమం_By_Grp = 0; /* True if the GROUP BY and ORDER BY are the same */

    /* Remove any and all aliases between the result set and the
    ** GROUP BY clause.
    */
    ఒకవేళ( p_సమూహం_By ){
      పూర్ణాంకం k;                        /* Loop counter */
      నిర్మాణం సమాసం_List_అంశం *p_అంశం;  /* For looping over expression in a list */

      కొరకు(k=p->pEList->n_సమాసం, p_అంశం=p->pEList->a; k>0; k--, p_అంశం++){
        p_అంశం->u.x.iAlias = 0;
      }
      కొరకు(k=p_సమూహం_By->n_సమాసం, p_అంశం=p_సమూహం_By->a; k>0; k--, p_అంశం++){
        p_అంశం->u.x.iAlias = 0;
      }
      ప్రతిజ్ఞా( 66==sqlite3LogEst(100) );
      ఒకవేళ( p->n_Select_అడ్డువరుస>66 ) p->n_Select_అడ్డువరుస = 66;

      /* If there is both a GROUP BY and an ORDER BY clause and they are
      ** identical, then it may be possible to disable the ORDER BY clause
      ** on the grounds that the GROUP BY will cause elements to come out
      ** in the correct order. It also may not - the GROUP BY might use a
      ** database index that causes rows to be grouped together as required
      ** but not actually sorted. Either way, record the fact that the
      ** ORDER BY and GROUP BY clauses are the same by setting the orderByGrp
      ** variable.  */
      ఒకవేళ( sqlite3_Copy_క్రమీకరించు_క్రమం(p_సమూహం_By, s_క్రమీకరించు.p_క్రమం_By)
       && sqlite3_సమాసం_List_Compare(p_సమూహం_By, s_క్రమీకరించు.p_క్రమం_By, -1)==0
      ){
        క్రమం_By_Grp = 1;
      }
    }లేకపోతే{
      ప్రతిజ్ఞా( 0==sqlite3LogEst(1) );
      p->n_Select_అడ్డువరుస = 0;
    }

    /* Create a label to jump to when we want to abort the query */
    addr_చివర = sqlite3VdbeMakeLabel(pParse);

    /* Convert TK_COLUMN nodes into TK_AGG_COLUMN and make entries in
    ** sAggInfo for all TK_AGG_FUNCTION nodes in expressions of the
    ** SELECT statement.
    */
    pAggInfo = sqlite3DbMallocZero(db, పరిమాణం(*pAggInfo) );
    ఒకవేళ( pAggInfo ){
      sqlite3_Parser_కూడు_Cleanup(pParse, agginfoFree, pAggInfo);
      testcase( pParse->earlyCleanup );
    }
    ఒకవేళ( db->mallocFailed ){
      వెళ్ళు select_చివర;
    }
    pAggInfo->selId = p->selId;
#నిర్వచితమైతే SQLITE_DEBUG
    pAggInfo->pSelect = p;
#అయితే_ముగింపు
    memset(&sNC, 0, పరిమాణం(sNC));
    sNC.pParse = pParse;
    sNC.pSrcList = pTabList;
    sNC.uNC.pAggInfo = pAggInfo;
    VVA_ONLY( sNC.ncFlags = NC_UAggInfo; )
    pAggInfo->n_క్రమీకరణ_స్తంభం = p_సమూహం_By ? p_సమూహం_By->n_సమాసం : 0;
    pAggInfo->p_సమూహం_By = p_సమూహం_By;
    sqlite3_సమాసం_Analyze_Agg_List(&sNC, pEList);
    sqlite3_సమాసం_Analyze_Agg_List(&sNC, s_క్రమీకరించు.p_క్రమం_By);
    ఒకవేళ( pHaving ){
      ఒకవేళ( p_సమూహం_By ){
        ప్రతిజ్ఞా( pWhere==p->pWhere );
        ప్రతిజ్ఞా( pHaving==p->pHaving );
        ప్రతిజ్ఞా( p_సమూహం_By==p->p_సమూహం_By );
        having_గమ్యం_Where(pParse, p);
        pWhere = p->pWhere;
      }
      sqlite3_సమాసం_Analyze_Aggregates(&sNC, pHaving);
    }
    pAggInfo->n_సంచాయకం = pAggInfo->n_స్తంభం;
    ఒకవేళ( p->p_సమూహం_By==0 && p->pHaving==0 && pAggInfo->nFunc==1 ){
      minMaxFlag = minMaxQuery(db, pAggInfo->aFunc[0].pFExpr, &p_Min_Max_క్రమం_By);
    }లేకపోతే{
      minMaxFlag = WHERE_ORDERBY_సాధారణ;
    }
    analyze_Agg_Func_వాదనలు(pAggInfo, &sNC);
    ఒకవేళ( db->mallocFailed ) వెళ్ళు select_చివర;
#ఒకవేళ TREETRACE_ENABLED
    ఒకవేళ( sqlite3_వృక్షం_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("After aggregate analysis %p:\n", pAggInfo));
      sqlite3_వృక్షం_View_Select(0, p, 0);
      ఒకవేళ( minMaxFlag ){
        sqlite3DebugPrintf("MIN/MAX Optimization (0x%02x) adds:\n", minMaxFlag);
        sqlite3_వృక్షం_View_సమాసం_List(0, p_Min_Max_క్రమం_By, 0, "ORDERBY");
      }
      printAggInfo(pAggInfo);
    }
#అయితే_ముగింపు


    /* Processing for aggregates with GROUP BY is very different and
    ** much more complex than aggregates without a GROUP BY.  tag-select-0810
    */
    ఒకవేళ( p_సమూహం_By ){
      కీలకం_Info *p_కీలకం_Info;  /* Keying information for the group by clause */
      పూర్ణాంకం addr1;          /* A-vs-B comparison jump */
      పూర్ణాంకం addr_Output_అడ్డువరుస;  /* Start of subroutine that outputs a result row */
      పూర్ణాంకం reg_Output_అడ్డువరుస;   /* Return address register for output subroutine */
      పూర్ణాంకం addr_సమితి_Abort;   /* Set the abort flag and return */
      పూర్ణాంకం addr_శిఖరం_యొక్క_Loop;  /* Top of the input loop */
      పూర్ణాంకం addr_క్రమీకరణ_Idx; /* The OP_OpenEphemeral for the sorting index */
      పూర్ణాంకం addrReset;      /* Subroutine for resetting the accumulator */
      పూర్ణాంకం regReset;       /* Return address register for reset subroutine */
      సమాసం_List *pDistinct = 0;
      u16 distFlag = 0;
      పూర్ణాంకం eDist = WHERE_DISTINCT_NOOP;

      ఒకవేళ( pAggInfo->nFunc==1
       && pAggInfo->aFunc[0].iDistinct>=0
       && ALWAYS(pAggInfo->aFunc[0].pFExpr!=0)
       && ALWAYS(సమాసం_Use_XList(pAggInfo->aFunc[0].pFExpr))
       && pAggInfo->aFunc[0].pFExpr->x.pList!=0
      ){
        సమాసం *p_సమాసం = pAggInfo->aFunc[0].pFExpr->x.pList->a[0].p_సమాసం;
        p_సమాసం = sqlite3_సమాసం_Dup(db, p_సమాసం, 0);
        pDistinct = sqlite3_సమాసం_List_Dup(db, p_సమూహం_By, 0);
        pDistinct = sqlite3_సమాసం_List_Append(pParse, pDistinct, p_సమాసం);
        distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
      }

      /* If there is a GROUP BY clause we might need a sorting index to
      ** implement it.  Allocate that sorting index now.  If it turns out
      ** that we do not need it after all, the OP_SorterOpen instruction
      ** will be converted into a Noop. 
      */
      pAggInfo->క్రమీకరణ_Idx = pParse->nTab++;
      p_కీలకం_Info = sqlite3_కీలకం_Info_From_సమాసం_List(pParse, p_సమూహం_By,
                                            0, pAggInfo->n_స్తంభం);
      addr_క్రమీకరణ_Idx = sqlite3_Vdbe_కూడు_Op4(v, OP_SorterOpen,
          pAggInfo->క్రమీకరణ_Idx, pAggInfo->n_క్రమీకరణ_స్తంభం,
          0, (అక్షరం*)p_కీలకం_Info, P4_KEYINFO);

      /* Initialize memory locations used by GROUP BY aggregate processing
      */
      iUseFlag = ++pParse->nMem;
      iAbortFlag = ++pParse->nMem;
      reg_Output_అడ్డువరుస = ++pParse->nMem;
      addr_Output_అడ్డువరుస = sqlite3VdbeMakeLabel(pParse);
      regReset = ++pParse->nMem;
      addrReset = sqlite3VdbeMakeLabel(pParse);
      iAMem = pParse->nMem + 1;
      pParse->nMem += p_సమూహం_By->n_సమాసం;
      iBMem = pParse->nMem + 1;
      pParse->nMem += p_సమూహం_By->n_సమాసం;
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, iAbortFlag);
      VdbeComment((v, "clear abort flag"));
      sqlite3_Vdbe_కూడు_Op3(v, OP_Null, 0, iAMem, iAMem+p_సమూహం_By->n_సమాసం-1);
      sqlite3_సమాసం_Null_రిజిస్టరు_Range(pParse, iAMem, p_సమూహం_By->n_సమాసం);

      /* Begin a loop that will extract all source rows in GROUP BY order.
      ** This might involve two separate loops with an OP_Sort in between, or
      ** it might be a single loop that uses an index to extract information
      ** in the right order to begin with.
      */
      sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, regReset, addrReset);
      TREETRACE(0x2,pParse,p,("WhereBegin\n"));
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, p_సమూహం_By, pDistinct,
          p, (sDistinct.isTnct==2 ? WHERE_DISTINCTBY : WHERE_GROUPBY)
          |  (క్రమం_By_Grp ? WHERE_SORTBYGROUP : 0) | distFlag, 0
      );
      ఒకవేళ( pWInfo==0 ){
        sqlite3_సమాసం_List_Delete(db, pDistinct);
        వెళ్ళు select_చివర;
      }
      ఒకవేళ( pParse->pIdxEpr ){
        optimize_Aggregate_Use_యొక్క_సూచీకృత_సమాసం(pParse, p, pAggInfo, &sNC);
      }
      కేటాయించు_Aggregate_Registers(pParse, pAggInfo);
      eDist = sqlite3WhereIsDistinct(pWInfo);
      TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
      ఒకవేళ( sqlite3WhereIsOrdered(pWInfo)==p_సమూహం_By->n_సమాసం ){
        /* The optimizer is able to deliver rows in group by order so
        ** we do not have to sort.  The OP_OpenEphemeral table will be
        ** cancelled later because we still need to use the pKeyInfo
        */
        సమూహం_By_క్రమీకరించు = 0;
      }లేకపోతే{
        /* Rows are coming out in undetermined order.  We have to push
        ** each row into a sorting index, terminate the first loop,
        ** then loop over the sorting index in order to get the output
        ** in sorted order
        */
        పూర్ణాంకం reg_పునాది;
        పూర్ణాంకం reg_రికార్డు;
        పూర్ణాంకం n_నిలువు;
        పూర్ణాంకం n_సమూహం_By;

#నిర్వచితమైతే SQLITE_ENABLE_STMT_SCANSTATUS
        పూర్ణాంకం addrExp;              /* Address of OP_Explain instruction */
#అయితే_ముగింపు
        ExplainQueryPlan2(addrExp, (pParse, 0, "USE TEMP B-TREE FOR %s",
            (sDistinct.isTnct && (p->selFlags&SF_Distinct)==0) ?
                    "DISTINCT" : "GROUP BY"
        ));

        సమూహం_By_క్రమీకరించు = 1;
        n_సమూహం_By = p_సమూహం_By->n_సమాసం;
        n_నిలువు = n_సమూహం_By;
        j = n_సమూహం_By;
        కొరకు(i=0; i<pAggInfo->n_స్తంభం; i++){
          ఒకవేళ( pAggInfo->a_నిలువు[i].i_Sorter_స్తంభం>=j ){
            n_నిలువు++;
            j++;
          }
        }
        reg_పునాది = sqlite3_Get_తాత్కాలిక_Range(pParse, n_నిలువు);
        sqlite3_సమాసం_సంకేతం_సమాసం_List(pParse, p_సమూహం_By, reg_పునాది, 0, 0);
        j = n_సమూహం_By;
        pAggInfo->ప్రత్యక్ష_బహుళకం = 1;
        కొరకు(i=0; i<pAggInfo->n_స్తంభం; i++){
          నిర్మాణం Agg_Info_నిలువు *p_నిలువు = &pAggInfo->a_నిలువు[i];
          ఒకవేళ( p_నిలువు->i_Sorter_స్తంభం>=j ){
            sqlite3_సమాసం_సంకేతం(pParse, p_నిలువు->pCExpr, j + reg_పునాది);
            j++;
          }
        }
        pAggInfo->ప్రత్యక్ష_బహుళకం = 0;
        reg_రికార్డు = sqlite3_Get_తాత్కాలిక_Reg(pParse);
        sqlite3_Vdbe_పరిశీలన_స్థితి_Counters(v, addrExp, 0, sqlite3VdbeCurrentAddr(v));
        sqlite3_Vdbe_కూడు_Op3(v, OP_Make_రికార్డు, reg_పునాది, n_నిలువు, reg_రికార్డు);
        sqlite3_Vdbe_కూడు_Op2(v, OP_Sorter_చొప్పించు, pAggInfo->క్రమీకరణ_Idx, reg_రికార్డు);
        sqlite3_Vdbe_పరిశీలన_స్థితి_Range(v, addrExp, sqlite3VdbeCurrentAddr(v)-2, -1);
        sqlite3_Release_తాత్కాలిక_Reg(pParse, reg_రికార్డు);
        sqlite3_Release_తాత్కాలిక_Range(pParse, reg_పునాది, n_నిలువు);
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_చివర(pWInfo);
        pAggInfo->క్రమీకరణ_Idx_PTab = క్రమీకరించు_PTab = pParse->nTab++;
        క్రమీకరించు_బహిర్గతం = sqlite3_Get_తాత్కాలిక_Reg(pParse);
        sqlite3_Vdbe_పరిశీలన_స్థితి_Counters(v, addrExp, sqlite3VdbeCurrentAddr(v), 0);
        sqlite3_Vdbe_కూడు_Op3(v, OP_OpenPseudo, క్రమీకరించు_PTab, క్రమీకరించు_బహిర్గతం, n_నిలువు);
        sqlite3_Vdbe_కూడు_Op2(v, OP_Sorter_క్రమీకరించు, pAggInfo->క్రమీకరణ_Idx, addr_చివర);
        VdbeComment((v, "GROUP BY sort")); VdbeCoverage(v);
        pAggInfo->use_క్రమీకరణ_Idx = 1;
        sqlite3_Vdbe_పరిశీలన_స్థితి_Range(v, addrExp, -1, క్రమీకరించు_PTab);
        sqlite3_Vdbe_పరిశీలన_స్థితి_Range(v, addrExp, -1, pAggInfo->క్రమీకరణ_Idx);
      }

      /* If there are entries in pAgggInfo->aFunc[] that contain subexpressions
      ** that are indexed (and that were previously identified and tagged
      ** in optimizeAggregateUseOfIndexedExpr()) then those subexpressions
      ** must now be converted into a TK_AGG_COLUMN node so that the value
      ** is correctly pulled from the index rather than being recomputed. */
      ఒకవేళ( pParse->pIdxEpr ){
        aggregate_Convert_సూచీకృత_సమాసం_Ref_గమ్యం_స్తంభం(pAggInfo);
#ఒకవేళ TREETRACE_ENABLED
        ఒకవేళ( sqlite3_వృక్షం_Trace & 0x20 ){
          TREETRACE(0x20, pParse, p,
             ("AggInfo function expressions converted to reference index\n"));
          sqlite3_వృక్షం_View_Select(0, p, 0);
          printAggInfo(pAggInfo);
        }
#అయితే_ముగింపు
      }

      /* If the index or temporary table used by the GROUP BY sort
      ** will naturally deliver rows in the order required by the ORDER BY
      ** clause, cancel the ephemeral table open coded earlier.
      **
      ** This is an optimization - the correct answer should result regardless.
      ** Use the SQLITE_GroupByOrder flag with SQLITE_TESTCTRL_OPTIMIZER to
      ** disable this optimization for testing purposes.  */
      ఒకవేళ( క్రమం_By_Grp && OptimizationEnabled(db, SQLITE_సమూహం_By_క్రమం)
       && (సమూహం_By_క్రమీకరించు || sqlite3WhereIsSorted(pWInfo))
      ){
        s_క్రమీకరించు.p_క్రమం_By = 0;
        sqlite3_Vdbe_Change_గమ్యం_Noop(v, s_క్రమీకరించు.addr_క్రమీకరించు_సూచీ);
      }

      /* Evaluate the current GROUP BY terms and store in b0, b1, b2...
      ** (b0 is memory location iBMem+0, b1 is iBMem+1, and so forth)
      ** Then compare the current GROUP BY terms against the GROUP BY terms
      ** from the previous row currently stored in a0, a1, a2...
      */
      addr_శిఖరం_యొక్క_Loop = sqlite3VdbeCurrentAddr(v);
      ఒకవేళ( సమూహం_By_క్రమీకరించు ){
        sqlite3_Vdbe_కూడు_Op3(v, OP_Sorter_దత్తాంశం, pAggInfo->క్రమీకరణ_Idx,
                          క్రమీకరించు_బహిర్గతం, క్రమీకరించు_PTab);
      }
      కొరకు(j=0; j<p_సమూహం_By->n_సమాసం; j++){
        పూర్ణాంకం i_క్రమం_By_నిలువు = p_సమూహం_By->a[j].u.x.i_క్రమం_By_నిలువు;
        
        ఒకవేళ( సమూహం_By_క్రమీకరించు ){
          sqlite3_Vdbe_కూడు_Op3(v, OP_స్తంభం, క్రమీకరించు_PTab, j, iBMem+j);
        }లేకపోతే{
          pAggInfo->ప్రత్యక్ష_బహుళకం = 1;
          sqlite3_సమాసం_సంకేతం(pParse, p_సమూహం_By->a[j].p_సమాసం, iBMem+j);
        }

        ఒకవేళ( i_క్రమం_By_నిలువు ){
          సమాసం *pX = p->pEList->a[i_క్రమం_By_నిలువు-1].p_సమాసం;
          సమాసం *p_పునాది = sqlite3_సమాసం_Skip_Collate_And_Likely(pX);
          అయితే( ALWAYS(p_పునాది!=0) && p_పునాది->op==TK_IF_NULL_అడ్డువరుస ){
            pX = p_పునాది->p_ఎడమ;
            p_పునాది = sqlite3_సమాసం_Skip_Collate_And_Likely(pX);
          }
          ఒకవేళ( ALWAYS(p_పునాది!=0)
           && p_పునాది->op!=TK_AGG_స్తంభం
           && p_పునాది->op!=TK_రిజిస్టరు
          ){
            sqlite3_సమాసం_గమ్యం_రిజిస్టరు(pX, iAMem+j);
          }
        }
      }
      sqlite3_Vdbe_కూడు_Op4(v, OP_Compare, iAMem, iBMem, p_సమూహం_By->n_సమాసం,
                          (అక్షరం*)sqlite3_కీలకం_Info_Ref(p_కీలకం_Info), P4_KEYINFO);
      addr1 = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_కూడు_Op3(v, OP_Jump, addr1+1, 0, addr1+1); VdbeCoverage(v);

      /* Generate code that runs whenever the GROUP BY changes.
      ** Changes in the GROUP BY are detected by the previous code
      ** block.  If there were no changes, this block is skipped.
      **
      ** This code copies current group by terms in b0,b1,b2,...
      ** over to a0,a1,a2.  It then calls the output subroutine
      ** and resets the aggregate accumulator registers in preparation
      ** for the next GROUP BY batch.
      */
      sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, reg_Output_అడ్డువరుస, addr_Output_అడ్డువరుస);
      VdbeComment((v, "output one row of %d", p->selId));
      sqlite3_సమాసం_సంకేతం_Move(pParse, iBMem, iAMem, p_సమూహం_By->n_సమాసం);
      sqlite3_Vdbe_కూడు_Op2(v, OP_IfPos, iAbortFlag, addr_చివర); VdbeCoverage(v);
      VdbeComment((v, "check abort flag"));
      sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, regReset, addrReset);
      VdbeComment((v, "reset accumulator %d", p->selId));

      /* Update the aggregate accumulators based on the content of
      ** the current row
      */
      sqlite3_Vdbe_Jump_ఇక్కడ(v, addr1);
      update_సంచాయకం(pParse, iUseFlag, pAggInfo, eDist);
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 1, iUseFlag);
      VdbeComment((v, "indicate data in accumulator %d", p->selId));

      /* End of the loop
      */
      ఒకవేళ( సమూహం_By_క్రమీకరించు ){
        sqlite3_Vdbe_కూడు_Op2(v, OP_Sorter_తదుపరి, pAggInfo->క్రమీకరణ_Idx,addr_శిఖరం_యొక్క_Loop);
        VdbeCoverage(v);
      }లేకపోతే{
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_చివర(pWInfo);
        sqlite3_Vdbe_Change_గమ్యం_Noop(v, addr_క్రమీకరణ_Idx);
      }
      sqlite3_సమాసం_List_Delete(db, pDistinct);

      /* Output the final row of result
      */
      sqlite3_Vdbe_కూడు_Op2(v, OP_Gosub, reg_Output_అడ్డువరుస, addr_Output_అడ్డువరుస);
      VdbeComment((v, "output final row of %d", p->selId));

      /* Jump over the subroutines
      */
      sqlite3VdbeGoto(v, addr_చివర);

      /* Generate a subroutine that outputs a single row of the result
      ** set.  This subroutine first looks at the iUseFlag.  If iUseFlag
      ** is less than or equal to zero, the subroutine is a no-op.  If
      ** the processing calls for the query to abort, this subroutine
      ** increments the iAbortFlag memory location before returning in
      ** order to signal the caller to abort.
      */
      addr_సమితి_Abort = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 1, iAbortFlag);
      VdbeComment((v, "set abort flag"));
      sqlite3_Vdbe_కూడు_Op1(v, OP_Return, reg_Output_అడ్డువరుస);
      sqlite3VdbeResolveLabel(v, addr_Output_అడ్డువరుస);
      addr_Output_అడ్డువరుస = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_కూడు_Op2(v, OP_IfPos, iUseFlag, addr_Output_అడ్డువరుస+2);
      VdbeCoverage(v);
      VdbeComment((v, "Groupby result generator entry point %d", p->selId));
      sqlite3_Vdbe_కూడు_Op1(v, OP_Return, reg_Output_అడ్డువరుస);
      finalizeAggFunctions(pParse, pAggInfo);
      sqlite3_సమాసం_If_False(pParse, pHaving, addr_Output_అడ్డువరుస+1, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, &s_క్రమీకరించు,
                      &sDistinct, pDest,
                      addr_Output_అడ్డువరుస+1, addr_సమితి_Abort);
      sqlite3_Vdbe_కూడు_Op1(v, OP_Return, reg_Output_అడ్డువరుస);
      VdbeComment((v, "end groupby result generator %d", p->selId));

      /* Generate a subroutine that will reset the group-by accumulator
      */
      sqlite3VdbeResolveLabel(v, addrReset);
      reset_సంచాయకం(pParse, pAggInfo);
      sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, iUseFlag);
      VdbeComment((v, "indicate accumulator %d empty", p->selId));
      sqlite3_Vdbe_కూడు_Op1(v, OP_Return, regReset);

      ఒకవేళ( distFlag!=0 && eDist!=WHERE_DISTINCT_NOOP ){
        నిర్మాణం AggInfo_func *pF = &pAggInfo->aFunc[0];
        fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
      }
    } /* endif pGroupBy.  Begin aggregate queries without GROUP BY: */
    లేకపోతే {
      /* Aggregate functions without GROUP BY. tag-select-0820 */
      పట్టిక *pTab;
      ఒకవేళ( (pTab = is_Simple_లెక్క(p, pAggInfo))!=0 ){
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
        స్థిరరాశి పూర్ణాంకం iDb = sqlite3_Schema_గమ్యం_సూచీ(pParse->db, pTab->pSchema);
        స్థిరరాశి పూర్ణాంకం iCsr = pParse->nTab++;     /* Cursor to scan b-tree */
        సూచీ *pIdx;                         /* Iterator variable */
        కీలకం_Info *p_కీలకం_Info = 0;               /* Keyinfo for scanned index */
        సూచీ *p_ఉత్తమ = 0;                    /* Best index found so far */
        Pgno i_మూలం = pTab->tnum;             /* Root page of scanned b-tree */

        sqlite3_సంకేతం_Verify_Schema(pParse, iDb);
        sqlite3_పట్టిక_తాళం(pParse, iDb, pTab->tnum, 0, pTab->zName);

        /* Search for the index that has the lowest scan cost.
        **
        ** (2011-04-15) Do not do a full scan of an unordered index.
        **
        ** (2013-10-03) Do not count the entries in a partial index.
        **
        ** In practice the KeyInfo structure will not be used. It is only
        ** passed to keep OP_OpenRead happy.
        */
        ఒకవేళ( !HasRowid(pTab) ) p_ఉత్తమ = sqlite3_Primary_కీలకం_సూచీ(pTab);
        ఒకవేళ( !p->pSrc->a[0].fg.not_సూచీకృత ){
          కొరకు(pIdx=pTab->p_సూచీ; pIdx; pIdx=pIdx->p_తదుపరి){
            ఒకవేళ( pIdx->bUnordered==0
             && pIdx->sz_Idx_అడ్డువరుస<pTab->sz_Tab_అడ్డువరుస
             && pIdx->pPartIdxWhere==0
             && (!p_ఉత్తమ || pIdx->sz_Idx_అడ్డువరుస<p_ఉత్తమ->sz_Idx_అడ్డువరుస)
            ){
              p_ఉత్తమ = pIdx;
            }
          }
        }
        ఒకవేళ( p_ఉత్తమ ){
          i_మూలం = p_ఉత్తమ->tnum;
          p_కీలకం_Info = sqlite3_కీలకం_Info_యొక్క_సూచీ(pParse, p_ఉత్తమ);
        }

        /* Open a read-only cursor, execute the OP_Count, close the cursor. */
        sqlite3_Vdbe_కూడు_Op4_Int(v, OP_OpenRead, iCsr, (పూర్ణాంకం)i_మూలం, iDb, 1);
        ఒకవేళ( p_కీలకం_Info ){
          sqlite3VdbeChangeP4(v, -1, (అక్షరం *)p_కీలకం_Info, P4_KEYINFO);
        }
        కేటాయించు_Aggregate_Registers(pParse, pAggInfo);
        sqlite3_Vdbe_కూడు_Op2(v, OP_లెక్క, iCsr, AggInfoFuncReg(pAggInfo,0));
        sqlite3_Vdbe_కూడు_Op1(v, OP_Close, iCsr);
        explain_Simple_లెక్క(pParse, pTab, p_ఉత్తమ);
      }లేకపోతే{
        /* The general case of an aggregate query without GROUP BY
        ** tag-select-0822 */
        పూర్ణాంకం regAcc = 0;           /* "populate accumulators" flag */
        సమాసం_List *pDistinct = 0;
        u16 distFlag = 0;
        పూర్ణాంకం eDist;

        /* If there are accumulator registers but no min() or max() functions
        ** without FILTER clauses, allocate register regAcc. Register regAcc
        ** will contain 0 the first time the inner loop runs, and 1 thereafter.
        ** The code generated by updateAccumulator() uses this to ensure
        ** that the accumulator registers are (a) updated only once if
        ** there are no min() or max functions or (b) always updated for the
        ** first row visited by the aggregate, so that they are updated at
        ** least once even if the FILTER clause means the min() or max()
        ** function visits zero rows.  */
        ఒకవేళ( pAggInfo->n_సంచాయకం ){
          కొరకు(i=0; i<pAggInfo->nFunc; i++){
            ఒకవేళ( సమాసం_Has_Property(pAggInfo->aFunc[i].pFExpr, EP_WinFunc) ){
              కొనసాగు;
            }
            ఒకవేళ( pAggInfo->aFunc[i].pFunc->funcFlags&SQLITE_FUNC_NEEDCOLL ){
              ఆపు;
            }
          }
          ఒకవేళ( i==pAggInfo->nFunc ){
            regAcc = ++pParse->nMem;
            sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 0, regAcc);
          }
        }లేకపోతే ఒకవేళ( pAggInfo->nFunc==1 && pAggInfo->aFunc[0].iDistinct>=0 ){
          ప్రతిజ్ఞా( సమాసం_Use_XList(pAggInfo->aFunc[0].pFExpr) );
          pDistinct = pAggInfo->aFunc[0].pFExpr->x.pList;
          distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
        }
        కేటాయించు_Aggregate_Registers(pParse, pAggInfo);

        /* This case runs if the aggregate has no GROUP BY clause.  The
        ** processing is much simpler since there is only a single row
        ** of output.
        */
        ప్రతిజ్ఞా( p->p_సమూహం_By==0 );
        reset_సంచాయకం(pParse, pAggInfo);

        /* If this query is a candidate for the min/max optimization, then
        ** minMaxFlag will have been previously set to either
        ** WHERE_ORDERBY_MIN or WHERE_ORDERBY_MAX and pMinMaxOrderBy will
        ** be an appropriate ORDER BY expression for the optimization.
        */
        ప్రతిజ్ఞా( minMaxFlag==WHERE_ORDERBY_సాధారణ || p_Min_Max_క్రమం_By!=0 );
        ప్రతిజ్ఞా( p_Min_Max_క్రమం_By==0 || p_Min_Max_క్రమం_By->n_సమాసం==1 );

        TREETRACE(0x2,pParse,p,("WhereBegin\n"));
        pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, p_Min_Max_క్రమం_By,
                                   pDistinct, p, minMaxFlag|distFlag, 0);
        ఒకవేళ( pWInfo==0 ){
          వెళ్ళు select_చివర;
        }
        TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
        eDist = sqlite3WhereIsDistinct(pWInfo);
        update_సంచాయకం(pParse, regAcc, pAggInfo, eDist);
        ఒకవేళ( eDist!=WHERE_DISTINCT_NOOP ){
          నిర్మాణం AggInfo_func *pF = pAggInfo->aFunc;
          ఒకవేళ( pF ){
            fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
          }
        }

        ఒకవేళ( regAcc ) sqlite3_Vdbe_కూడు_Op2(v, OP_Integer, 1, regAcc);
        ఒకవేళ( minMaxFlag ){
          sqlite3_Where_Min_Max_Opt_Early_బహిర్గతం(v, pWInfo);
        }
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_చివర(pWInfo);
        finalizeAggFunctions(pParse, pAggInfo);
      }

      s_క్రమీకరించు.p_క్రమం_By = 0;
      sqlite3_సమాసం_If_False(pParse, pHaving, addr_చివర, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, 0, 0,
                      pDest, addr_చివర, addr_చివర);
    }
    sqlite3VdbeResolveLabel(v, addr_చివర);

  } /* endif aggregate query */

  ఒకవేళ( sDistinct.eTnctType==WHERE_DISTINCT_UNORDERED ){
    explain_తాత్కాలిక_పట్టిక(pParse, "DISTINCT");
  }

  /* If there is an ORDER BY clause, then we need to sort the results
  ** and send them to the callback one by one.  tag-select-0900
  */
  ఒకవేళ( s_క్రమీకరించు.p_క్రమం_By ){
    ప్రతిజ్ఞా( p->pEList==pEList );
    generate_క్రమీకరించు_తోక(pParse, p, &s_క్రమీకరించు, pEList->n_సమాసం, pDest);
  }

  /* Jump here to skip this query
  */
  sqlite3VdbeResolveLabel(v, i_చివర);

  /* The SELECT has been coded. If there is an error in the Parse structure,
  ** set the return code to 1. Otherwise 0. */
  rc = (pParse->nErr>0);

  /* Control jumps to here if an error is encountered above, or upon
  ** successful coding of the SELECT.
  */
select_చివర:
  ప్రతిజ్ఞా( db->mallocFailed==0 || db->mallocFailed==1 );
  ప్రతిజ్ఞా( db->mallocFailed==0 || pParse->nErr!=0 );
  sqlite3_సమాసం_List_Delete(db, p_Min_Max_క్రమం_By);
#నిర్వచితమైతే SQLITE_DEBUG
  /* Internal self-checks.  tag-select-1000 */
  ఒకవేళ( pAggInfo && !db->mallocFailed ){
#ఒకవేళ TREETRACE_ENABLED
    ఒకవేళ( sqlite3_వృక్షం_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("Finished with AggInfo\n"));
      printAggInfo(pAggInfo);
    }
#అయితే_ముగింపు
    కొరకు(i=0; i<pAggInfo->n_స్తంభం; i++){
      సమాసం *p_సమాసం = pAggInfo->a_నిలువు[i].pCExpr;
      ఒకవేళ( p_సమాసం==0 ) కొనసాగు;
      ప్రతిజ్ఞా( p_సమాసం->pAggInfo==pAggInfo );
      ప్రతిజ్ఞా( p_సమాసం->iAgg==i );
    }
    కొరకు(i=0; i<pAggInfo->nFunc; i++){
      సమాసం *p_సమాసం = pAggInfo->aFunc[i].pFExpr;
      ప్రతిజ్ఞా( p_సమాసం!=0 );
      ప్రతిజ్ఞా( p_సమాసం->pAggInfo==pAggInfo );
      ప్రతిజ్ఞా( p_సమాసం->iAgg==i );
    }
  }
#అయితే_ముగింపు

#ఒకవేళ TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p,("end processing\n"));
  ఒకవేళ( (sqlite3_వృక్షం_Trace & 0x40000)!=0 && Explain_Query_Plan_మాతృ(pParse)==0 ){
    sqlite3_వృక్షం_View_Select(0, p, 0);
  }
#అయితే_ముగింపు
  Explain_Query_Plan_తీయి(pParse);
  తిరిగి rc;
}
