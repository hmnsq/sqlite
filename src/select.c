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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

/*
** An instance of the following object is used to record information about
** how to process the DISTINCT keyword, to simplify passing that information
** into the selectInnerLoop() routine.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା DistinctCtx DistinctCtx;
ସଂରଚନା DistinctCtx {
  u8 isTnct;      /* 0: Not distinct. 1: DISTINCT  2: DISTINCT and ORDER BY */
  u8 eTnctType;   /* One of the WHERE_DISTINCT_* operators */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା tabTnct;    /* Ephemeral table used for DISTINCT processing */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrTnct;   /* Address of OP_OpenEphemeral opcode for tabTnct */
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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା SortCtx SortCtx;
ସଂରଚନା SortCtx {
  ପଦସମଷ୍ଟି_List *pOrderBy;   /* The ORDER BY (or GROUP BY clause) */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOBSat;           /* Number of ORDER BY terms satisfied by indices */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iECursor;         /* Cursor number for the sorter */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regReturn;        /* Register holding block-output return address */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା labelBkOut;       /* Start label for the block-output subroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrSortIndex;    /* Address of the OP_SorterOpen or OP_OpenEphemeral */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା labelDone;        /* Jump here when done, ex: LIMIT reached */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା labelOBLopt;      /* Jump here when sorter is full */
  u8 sortFlags;         /* Zero or more SORTFLAG_* bits */
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  u8 nDefer;            /* Number of valid entries in aDefer[] */
  ସଂରଚନା DeferredCsr {
    Table *pTab;        /* Table definition */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCsr;           /* Cursor number for table */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଚାବି;           /* Number of PK columns for table pTab (>=1) */
  } aDefer[4];
#endif
  ସଂରଚନା RowLoadInfo *pDeferredRowLoad;  /* Deferred row loading info or NULL */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr_ଠେଲ;         /* First instruction to push data into sorter */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr_ଠେଲ_End;      /* Last instruction that pushes data into sorter */
#endif
};
#define SORTFLAG_UseSorter  0x01   /* Use SorterOpen instead of OpenEphemeral */

/*
** Delete all the content of a Select structure.  Deallocate the structure
** itself depending on the value of bFree
**
** If bFree==1, call sqlite3DbFree() on the p object.
** If bFree==0, Leave the first Select object unfreed
*/
ସ୍ଥିର ଶୂନ୍ୟ clearSelect(sqlite3 *db, Select *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bFree){
  ପ୍ରତିଜ୍ଞା( db!=0 );
  ଯେତେବେଳେ( p ){
    Select *pPrior = p->pPrior;
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, p->pEList);
    sqlite3SrcListDelete(db, p->pSrc);
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, p->pWhere);
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, p->pGroupBy);
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, p->pHaving);
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, p->pOrderBy);
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, p->pLimit);
    ଯଦି( OK_IF_ALWAYS_TRUE(p->pWith) ) sqlite3WithDelete(db, p->pWith);
#ifndef SQLITE_OMIT_WINDOWFUNC
    ଯଦି( OK_IF_ALWAYS_TRUE(p->pWinDefn) ){
      sqlite3WindowListDelete(db, p->pWinDefn);
    }
    ଯେତେବେଳେ( p->pWin ){
      ପ୍ରତିଜ୍ଞା( p->pWin->ppThis==&p->pWin );
      sqlite3WindowUnlinkFromSelect(p->pWin);
    }
#endif
    ଯଦି( bFree ) sqlite3DbNNFreeNN(db, p);
    p = pPrior;
    bFree = 1;
  }
}

/*
** Initialize a SelectDest structure.
*/
ଶୂନ୍ୟ sqlite3SelectDestInit(SelectDest *pDest, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eDest, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iParm){
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
  ପଦସମଷ୍ଟି_List *pEList,     /* which columns to include in the result */
  SrcList *pSrc,        /* the FROM clause -- which tables to scan */
  ପଦସମଷ୍ଟି *pWhere,         /* the WHERE clause */
  ପଦସମଷ୍ଟି_List *pGroupBy,   /* the GROUP BY clause */
  ପଦସମଷ୍ଟି *pHaving,        /* the HAVING clause */
  ପଦସମଷ୍ଟି_List *pOrderBy,   /* the ORDER BY clause */
  u32 selFlags,         /* Flag parameters, such as SF_Distinct */
  ପଦସମଷ୍ଟି *pLimit          /* LIMIT value.  NULL means not used */
){
  Select *pNew, *pAllocated;
  Select standin;
  pAllocated = pNew = sqlite3DbMallocRawNN(pParse->db, ଆକାର(*pNew) );
  ଯଦି( pNew==0 ){
    ପ୍ରତିଜ୍ଞା( pParse->db->mallocFailed );
    pNew = &standin;
  }
  ଯଦି( pEList==0 ){
    pEList = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, 0,
                                   sqlite3_ପଦସମଷ୍ଟି(pParse->db,TK_ASTERISK,0));
  }
  pNew->pEList = pEList;
  pNew->op = TK_SELECT;
  pNew->selFlags = selFlags;
  pNew->iLimit = 0;
  pNew->iOffset = 0;
  pNew->selId = ++pParse->nSelect;
  pNew->nSelectRow = 0;
  ଯଦି( pSrc==0 ) pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  pNew->pSrc = pSrc;
  pNew->pWhere = pWhere;
  pNew->pGroupBy = pGroupBy;
  pNew->pHaving = pHaving;
  pNew->pOrderBy = pOrderBy;
  pNew->pPrior = 0;
  pNew->pNext = 0;
  pNew->pLimit = pLimit;
  pNew->pWith = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
  pNew->pWin = 0;
  pNew->pWinDefn = 0;
#endif
  ଯଦି( pParse->db->mallocFailed ) {
    clearSelect(pParse->db, pNew, pNew!=&standin);
    pAllocated = 0;
  }ଅନ୍ୟଥା{
    ପ୍ରତିଜ୍ଞା( pNew->pSrc!=0 || pParse->nErr>0 );
  }
  ଫେରନ୍ତୁ pAllocated;
}


/*
** Delete the given Select structure and all of its substructures.
*/
ଶୂନ୍ୟ sqlite3SelectDelete(sqlite3 *db, Select *p){
  ଯଦି( OK_IF_ALWAYS_TRUE(p) ) clearSelect(db, p, 1);
}
ଶୂନ୍ୟ sqlite3SelectDeleteGeneric(sqlite3 *db, ଶୂନ୍ୟ *p){
  ଯଦି( ALWAYS(p) ) clearSelect(db, (Select*)p, 1);
}

/*
** Return a pointer to the right-most SELECT statement in a compound.
*/
ସ୍ଥିର Select *findRightmost(Select *p){
  ଯେତେବେଳେ( p->pNext ) p = p->pNext;
  ଫେରନ୍ତୁ p;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3JoinType(Parse *pParse, Token *pA, Token *pB, Token *pC){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା jointype = 0;
  Token *apAll[3];
  Token *p;
                             /*   0123456789 123456789 123456789 123 */
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର z_ଚାବି_Text[] = "naturaleftouterightfullinnercross";
  ସ୍ଥିର ଅଚଳ ସଂରଚନା {
    u8 i;        /* Beginning of keyword text in zKeyText[] */
    u8 nChar;    /* Length of the keyword in characters */
    u8 code;     /* Join type mask */
  } aKeyword[] = {
    /* (0) natural */ { 0,  7, JT_NATURAL                },
    /* (1) left    */ { 6,  4, JT_ବାମ|JT_OUTER          },
    /* (2) outer   */ { 10, 5, JT_OUTER                  },
    /* (3) right   */ { 14, 5, JT_ଡାହାଣ|JT_OUTER         },
    /* (4) full    */ { 19, 4, JT_ବାମ|JT_ଡାହାଣ|JT_OUTER },
    /* (5) inner   */ { 23, 5, JT_INNER                  },
    /* (6) cross   */ { 28, 5, JT_INNER|JT_CROSS         },
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
  apAll[0] = pA;
  apAll[1] = pB;
  apAll[2] = pC;
  ପାଇଁ(i=0; i<3 && apAll[i]; i++){
    p = apAll[i];
    ପାଇଁ(j=0; j<ArraySize(aKeyword); j++){
      ଯଦି( p->n==aKeyword[j].nChar
          && sqlite3StrNICmp((ଅକ୍ଷର*)p->z, &z_ଚାବି_Text[aKeyword[j].i], p->n)==0 ){
        jointype |= aKeyword[j].code;
        ଭାଙ୍ଗ;
      }
    }
    testcase( j==0 || j==1 || j==2 || j==3 || j==4 || j==5 || j==6 );
    ଯଦି( j>=ArraySize(aKeyword) ){
      jointype |= JT_ERROR;
      ଭାଙ୍ଗ;
    }
  }
  ଯଦି(
     (jointype & (JT_INNER|JT_OUTER))==(JT_INNER|JT_OUTER) ||
     (jointype & JT_ERROR)!=0 ||
     (jointype & (JT_OUTER|JT_ବାମ|JT_ଡାହାଣ))==JT_OUTER
  ){
    ଅଚଳ ଅକ୍ଷର *zSp1 = " ";
    ଅଚଳ ଅକ୍ଷର *zSp2 = " ";
    ଯଦି( pB==0 ){ zSp1++; }
    ଯଦି( pC==0 ){ zSp2++; }
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "unknown join type: "
       "%T%s%T%s%T", pA, zSp1, pB, zSp2, pC);
    jointype = JT_INNER;
  }
  ଫେରନ୍ତୁ jointype;
}

/*
** Return the index of a column in a table.  Return -1 if the column
** is not contained in the table.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ColumnIndex(Table *pTab, ଅଚଳ ଅକ୍ଷର *zCol){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  u8 h;
  ଅଚଳ Column *aCol;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol;

  h = sqlite3StrIHash(zCol);
  aCol = pTab->aCol;
  nCol = pTab->nCol;

  /* See if the aHx gives us a lucky match */
  i = pTab->aHx[h % ଆକାର(pTab->aHx)];
  ପ୍ରତିଜ୍ଞା( i<nCol );
  ଯଦି( aCol[i].hName==h
   && sqlite3StrICmp(aCol[i].zCnName, zCol)==0
  ){
    ଫେରନ୍ତୁ i;
  }

  /* No lucky match from the hash table.  Do a full search. */
  i = 0;
  ଯେତେବେଳେ( 1 /*exit-by-break*/ ){
    ଯଦି( aCol[i].hName==h
     && sqlite3StrICmp(aCol[i].zCnName, zCol)==0
    ){
      ଫେରନ୍ତୁ i;
    }
    i++;
    ଯଦି( i>=nCol ) ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ -1;
}

/*
** Mark a subquery result column as having been used.
*/
ଶୂନ୍ୟ sqlite3SrcItemColumnUsed(SrcItem *pItem, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol){
  ପ୍ରତିଜ୍ଞା( pItem!=0 );
  ପ୍ରତିଜ୍ଞା( (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
  ଯଦି( pItem->fg.isNestedFrom ){
    ପଦସମଷ୍ଟି_List *pResults;
    ପ୍ରତିଜ୍ଞା( pItem->fg.isSubquery );
    ପ୍ରତିଜ୍ଞା( pItem->u4.pSubq!=0 );
    ପ୍ରତିଜ୍ଞା( pItem->u4.pSubq->pSelect!=0 );
    pResults = pItem->u4.pSubq->pSelect->pEList;
    ପ୍ରତିଜ୍ଞା( pResults!=0 );
    ପ୍ରତିଜ୍ଞା( iCol>=0 && iCol<pResults->n_ପଦସମଷ୍ଟି );
    pResults->a[iCol].fg.bUsed = 1;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tableAndColumnIndex(
  SrcList *pSrc,       /* Array of tables to search */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iStart,          /* First member of pSrc->a[] to check */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iEnd,            /* Last member of pSrc->a[] to check */
  ଅଚଳ ଅକ୍ଷର *zCol,    /* Name of the column we are looking for */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *piTab,          /* Write index of pSrc->a[] here */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *piCol,          /* Write index of pSrc->a[*piTab].pSTab->aCol[] here */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bIgnoreHidden    /* Ignore hidden columns */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;               /* For looping over tables in pSrc */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol;            /* Index of column matching zCol */

  ପ୍ରତିଜ୍ଞା( iEnd<pSrc->nSrc );
  ପ୍ରତିଜ୍ଞା( iStart>=0 );
  ପ୍ରତିଜ୍ଞା( (piTab==0)==(piCol==0) );  /* Both or neither are NULL */

  ପାଇଁ(i=iStart; i<=iEnd; i++){
    iCol = sqlite3ColumnIndex(pSrc->a[i].pSTab, zCol);
    ଯଦି( iCol>=0
     && (bIgnoreHidden==0 || IsHiddenColumn(&pSrc->a[i].pSTab->aCol[iCol])==0)
    ){
      ଯଦି( piTab ){
        sqlite3SrcItemColumnUsed(&pSrc->a[i], iCol);
        *piTab = i;
        *piCol = iCol;
      }
      ଫେରନ୍ତୁ 1;
    }
  }
  ଫେରନ୍ତୁ 0;
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
ଶୂନ୍ୟ sqlite3_Set_Join_ପଦସମଷ୍ଟି(ପଦସମଷ୍ଟି *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTable, u32 joinFlag){
  ପ୍ରତିଜ୍ଞା( joinFlag==EP_OuterON || joinFlag==EP_InnerON );
  ଯେତେବେଳେ( p ){
    ପଦସମଷ୍ଟି_Set_Property(p, joinFlag);
    ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    ପଦସମଷ୍ଟି_Set_VVAProperty(p, EP_NoReduce);
    p->w.iJoin = iTable;
    ଯଦି( ପଦସମଷ୍ଟି_Use_XList(p) ){
      ଯଦି( p->x.pList ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
        ପାଇଁ(i=0; i<p->x.pList->n_ପଦସମଷ୍ଟି; i++){
          sqlite3_Set_Join_ପଦସମଷ୍ଟି(p->x.pList->a[i].p_ପଦସମଷ୍ଟି, iTable, joinFlag);
        }
      }
    }
    sqlite3_Set_Join_ପଦସମଷ୍ଟି(p->p_ବାମ, iTable, joinFlag);
    p = p->p_ଡାହାଣ;
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
ସ୍ଥିର ଶୂନ୍ୟ unset_Join_ପଦସମଷ୍ଟି(ପଦସମଷ୍ଟି *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTable, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nullable){
  ଯେତେବେଳେ( p ){
    ଯଦି( iTable<0 || (ପଦସମଷ୍ଟି_Has_Property(p, EP_OuterON) && p->w.iJoin==iTable) ){
      ପଦସମଷ୍ଟି_Clear_Property(p, EP_OuterON|EP_InnerON);
      ଯଦି( iTable>=0 ) ପଦସମଷ୍ଟି_Set_Property(p, EP_InnerON);
    }
    ଯଦି( p->op==TK_COLUMN && p->iTable==iTable && !nullable ){
      ପଦସମଷ୍ଟି_Clear_Property(p, EP_CanBeNull);
    }
    ଯଦି( p->op==TK_FUNCTION ){
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p) );
      ପ୍ରତିଜ୍ଞା( p->p_ବାମ==0 );
      ଯଦି( p->x.pList ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
        ପାଇଁ(i=0; i<p->x.pList->n_ପଦସମଷ୍ଟି; i++){
          unset_Join_ପଦସମଷ୍ଟି(p->x.pList->a[i].p_ପଦସମଷ୍ଟି, iTable, nullable);
        }
      }
    }
    unset_Join_ପଦସମଷ୍ଟି(p->p_ବାମ, iTable, nullable);
    p = p->p_ଡାହାଣ;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ProcessJoin(Parse *pParse, Select *p){
  SrcList *pSrc;                  /* All tables in the FROM clause */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;                       /* Loop counters */
  SrcItem *p_ବାମ;                 /* Left table being joined */
  SrcItem *p_ଡାହାଣ;                /* Right table being joined */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bSeenTFunc = 0;             /* True if have seen table-valued function */

  pSrc = p->pSrc;
  p_ବାମ = &pSrc->a[0];
  p_ଡାହାଣ = &p_ବାମ[1];
  ପାଇଁ(i=0; i<pSrc->nSrc-1; i++, p_ଡାହାଣ++, p_ବାମ++){
    Table *p_ଡାହାଣ_Tab = p_ଡାହାଣ->pSTab;
    u32 joinType;

    ଯଦି( NEVER(p_ବାମ->pSTab==0 || p_ଡାହାଣ_Tab==0) ) ଜାରି;
    joinType = (p_ଡାହାଣ->fg.jointype & JT_OUTER)!=0 ? EP_OuterON : EP_InnerON;

    /* If this is a NATURAL join, synthesize an appropriate USING clause
    ** to specify which columns should be joined.
    */
    ଯଦି( p_ଡାହାଣ->fg.jointype & JT_NATURAL ){
      IdList *pUsing = 0;
      ଯଦି( p_ଡାହାଣ->fg.isUsing || p_ଡାହାଣ->u3.pOn ){
        sqlite3_Error_ସନ୍ଦେଶ(pParse, "a NATURAL join may not have "
           "an ON or USING clause", 0);
        ଫେରନ୍ତୁ 1;
      }
      ପାଇଁ(j=0; j<p_ଡାହାଣ_Tab->nCol; j++){
        ଅକ୍ଷର *zName;   /* Name of column in the right table */

        ଯଦି( IsHiddenColumn(&p_ଡାହାଣ_Tab->aCol[j]) ) ଜାରି;
        zName = p_ଡାହାଣ_Tab->aCol[j].zCnName;
        ଯଦି( tableAndColumnIndex(pSrc, 0, i, zName, 0, 0, 1) ){
          pUsing = sqlite3IdListAppend(pParse, pUsing, 0);
          ଯଦି( pUsing ){
            ପ୍ରତିଜ୍ଞା( pUsing->nId>0 );
            ପ୍ରତିଜ୍ଞା( pUsing->a[pUsing->nId-1].zName==0 );
            pUsing->a[pUsing->nId-1].zName = sqlite3DbStrDup(pParse->db, zName);
          }
        }
      }
      ଯଦି( pUsing ){
        p_ଡାହାଣ->fg.isUsing = 1;
        p_ଡାହାଣ->fg.isSynthUsing = 1;
        p_ଡାହାଣ->u3.pUsing = pUsing;
      }
      ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ 1;
    }

    /* Create extra terms on the WHERE clause for each column named
    ** in the USING clause.  Example: If the two tables to be joined are
    ** A and B and the USING clause names X, Y, and Z, then add this
    ** to the WHERE clause:    A.X=B.X AND A.Y=B.Y AND A.Z=B.Z
    ** Report an error if any column mentioned in the USING clause is
    ** not contained in both tables to be joined.
    */
    ଯଦି( p_ଡାହାଣ->fg.isUsing ){
      IdList *pList = p_ଡାହାଣ->u3.pUsing;
      sqlite3 *db = pParse->db;
      ପ୍ରତିଜ୍ଞା( pList!=0 );
      ପାଇଁ(j=0; j<pList->nId; j++){
        ଅକ୍ଷର *zName;     /* Name of the term in the USING clause */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_ବାମ;       /* Table on the left with matching column name */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_ବାମ_Col;    /* Column number of matching column on the left */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_ଡାହାଣ_Col;   /* Column number of matching column on the right */
        ପଦସମଷ୍ଟି *pE1;       /* Reference to the column on the LEFT of the join */
        ପଦସମଷ୍ଟି *pE2;       /* Reference to the column on the RIGHT of the join */
        ପଦସମଷ୍ଟି *pEq;       /* Equality constraint.  pE1 == pE2 */

        zName = pList->a[j].zName;
        i_ଡାହାଣ_Col = sqlite3ColumnIndex(p_ଡାହାଣ_Tab, zName);
        ଯଦି( i_ଡାହାଣ_Col<0
         || tableAndColumnIndex(pSrc, 0, i, zName, &i_ବାମ, &i_ବାମ_Col,
                                p_ଡାହାଣ->fg.isSynthUsing)==0
        ){
          sqlite3_Error_ସନ୍ଦେଶ(pParse, "cannot join using column %s - column "
            "not present in both tables", zName);
          ଫେରନ୍ତୁ 1;
        }
        pE1 = sqlite3_Create_Column_ପଦସମଷ୍ଟି(db, pSrc, i_ବାମ, i_ବାମ_Col);
        sqlite3SrcItemColumnUsed(&pSrc->a[i_ବାମ], i_ବାମ_Col);
        ଯଦି( (pSrc->a[0].fg.jointype & JT_LTORJ)!=0 && pParse->nErr==0 ){
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
          ପଦସମଷ୍ଟି_List *pFuncArgs = 0;   /* Arguments to the coalesce() */
          ସ୍ଥିର ଅଚଳ Token tkCoalesce = { "coalesce", 8 };
          ପ୍ରତିଜ୍ଞା( pE1!=0 );
          ପଦସମଷ୍ଟି_Set_Property(pE1, EP_CanBeNull);
          ଯେତେବେଳେ( tableAndColumnIndex(pSrc, i_ବାମ+1, i, zName, &i_ବାମ, &i_ବାମ_Col,
                                     p_ଡାହାଣ->fg.isSynthUsing)!=0 ){
            ଯଦି( pSrc->a[i_ବାମ].fg.isUsing==0
             || sqlite3IdListIndex(pSrc->a[i_ବାମ].u3.pUsing, zName)<0
            ){
              sqlite3_Error_ସନ୍ଦେଶ(pParse, "ambiguous reference to %s in USING()",
                              zName);
              ଭାଙ୍ଗ;
            }
            pFuncArgs = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, pFuncArgs, pE1);
            pE1 = sqlite3_Create_Column_ପଦସମଷ୍ଟି(db, pSrc, i_ବାମ, i_ବାମ_Col);
            sqlite3SrcItemColumnUsed(&pSrc->a[i_ବାମ], i_ବାମ_Col);
          }
          ଯଦି( pFuncArgs ){
            pFuncArgs = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, pFuncArgs, pE1);
            pE1 = sqlite3_ପଦସମଷ୍ଟି_Function(pParse, pFuncArgs, &tkCoalesce, 0);
            ଯଦି( pE1 ){
              pE1->aff_ପଦସମଷ୍ଟି = SQLITE_AFF_DEFER;
            }
          }
        }ଅନ୍ୟଥା ଯଦି( (pSrc->a[i+1].fg.jointype & JT_ବାମ)!=0 && pParse->nErr==0 ){
          ପ୍ରତିଜ୍ଞା( pE1!=0 );
          ପଦସମଷ୍ଟି_Set_Property(pE1, EP_CanBeNull);
        }
        pE2 = sqlite3_Create_Column_ପଦସମଷ୍ଟି(db, pSrc, i+1, i_ଡାହାଣ_Col);
        sqlite3SrcItemColumnUsed(p_ଡାହାଣ, i_ଡାହାଣ_Col);
        pEq = sqlite3PExpr(pParse, TK_EQ, pE1, pE2);
        ପ୍ରତିଜ୍ଞା( pE2!=0 || pEq==0 );
        ଯଦି( pEq ){
          ପଦସମଷ୍ଟି_Set_Property(pEq, joinType);
          ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(pEq, EP_TokenOnly|EP_Reduced) );
          ପଦସମଷ୍ଟି_Set_VVAProperty(pEq, EP_NoReduce);
          pEq->w.iJoin = pE2->iTable;
        }
        p->pWhere = sqlite3_ପଦସମଷ୍ଟି_And(pParse, p->pWhere, pEq);
      }
    }

    /* Add the ON clause to the end of the WHERE clause, connected by
    ** an AND operator.
    */
    ଅନ୍ୟଥା ଯଦି( p_ଡାହାଣ->u3.pOn ){
      sqlite3_Set_Join_ପଦସମଷ୍ଟି(p_ଡାହାଣ->u3.pOn, p_ଡାହାଣ->iCursor, joinType);
      p->pWhere = sqlite3_ପଦସମଷ୍ଟି_And(pParse, p->pWhere, p_ଡାହାଣ->u3.pOn);
      p_ଡାହାଣ->u3.pOn = 0;
      p_ଡାହାଣ->fg.isOn = 1;
      p->selFlags |= SF_OnToWhere;
    }

    ଯଦି( bSeenTFunc==0 ){
      bSeenTFunc = (p_ବାମ->fg.isTabFunc && p_ବାମ->u1.pFuncArg);
    }

    /* We also need to call sqlite3SelectCheckOnClauses() to verify that
    ** table-valued function arguments do not illegally refer to any tables to
    ** their right. This test is required if either (a) there is a RIGHT JOIN
    ** in the SrcList, or (b) there is a LEFT JOIN to the right of a 
    ** table-valued function.  */
    ଯଦି( (p_ଡାହାଣ->fg.isTabFunc && joinType==EP_OuterON && p_ଡାହାଣ->u1.pFuncArg)
     || (bSeenTFunc && joinType==EP_OuterON)
    ){
      p->selFlags |= SF_OnToWhere;
    }
  }
  ଫେରନ୍ତୁ 0;
}

/*
** An instance of this object holds information (beyond pParse and pSelect)
** needed to load the next result row that is to be added to the sorter.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା RowLoadInfo RowLoadInfo;
ସଂରଚନା RowLoadInfo {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regResult;               /* Store results in array of registers here */
  u8 ecelFlags;                /* Flag argument to ExprCodeExprList() */
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  ପଦସମଷ୍ଟି_List *pExtra;            /* Extra columns needed by sorter refs */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regExtraResult;          /* Where to load the extra columns */
#endif
};

/*
** This routine does the work of loading query data into an array of
** registers so that it can be added to the sorter.
*/
ସ୍ଥିର ଶୂନ୍ୟ innerLoopLoadRow(
  Parse *pParse,             /* Statement under construction */
  Select *pSelect,           /* The query being coded */
  RowLoadInfo *pInfo         /* Info needed to complete the row load */
){
  sqlite3_ପଦସମଷ୍ଟି_Code_ପଦସମଷ୍ଟି_List(pParse, pSelect->pEList, pInfo->regResult,
                          0, pInfo->ecelFlags);
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  ଯଦି( pInfo->pExtra ){
    sqlite3_ପଦସମଷ୍ଟି_Code_ପଦସମଷ୍ଟି_List(pParse, pInfo->pExtra, pInfo->regExtraResult, 0, 0);
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(pParse->db, pInfo->pExtra);
  }
#endif
}

/*
** Code the OP_MakeRecord instruction that generates the entry to be
** added into the sorter.
**
** Return the register in which the result is stored.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା makeSorterRecord(
  Parse *pParse,
  SortCtx *pSort,
  Select *pSelect,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regBase,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBase
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOBSat = pSort->nOBSat;
  Vdbe *v = pParse->pVdbe;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regOut = ++pParse->nMem;
  ଯଦି( pSort->pDeferredRowLoad ){
    innerLoopLoadRow(pParse, pSelect, pSort->pDeferredRowLoad);
  }
  sqlite3VdbeAddOp3(v, OP_MakeRecord, regBase+nOBSat, nBase-nOBSat, regOut);
  ଫେରନ୍ତୁ regOut;
}

/*
** Generate code that will push the record in registers regData
** through regData+nData-1 onto the sorter.
*/
ସ୍ଥିର ଶୂନ୍ୟ ଠେଲ_Onto_Sorter(
  Parse *pParse,         /* Parser context */
  SortCtx *pSort,        /* Information about the ORDER BY clause */
  Select *pSelect,       /* The whole SELECT statement */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regData,           /* First register holding data to be sorted */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regOrigData,       /* First register holding data before packing */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nData,             /* Number of elements in the regData data array */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPrefixReg         /* No. of reg prior to regData available for use */
){
  Vdbe *v = pParse->pVdbe;                         /* Stmt under construction */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bSeq = ((pSort->sortFlags & SORTFLAG_UseSorter)==0);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ପଦସମଷ୍ଟି = pSort->pOrderBy->n_ପଦସମଷ୍ଟି;              /* No. of ORDER BY terms */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nBase = n_ପଦସମଷ୍ଟି + bSeq + nData;                /* Fields in sorter record */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regBase;                                     /* Regs for sorter record */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regRecord = 0;                               /* Assembled sorter record */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOBSat = pSort->nOBSat;                      /* ORDER BY terms to skip */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;                            /* Opcode to add sorter record to sorter */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iLimit;                        /* LIMIT counter */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSkip = 0;                     /* End of the sorter insert loop */

  ପ୍ରତିଜ୍ଞା( bSeq==0 || bSeq==1 );

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
  ପ୍ରତିଜ୍ଞା( nData==1 || regData==regOrigData || regOrigData==0 );

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  pSort->addr_ଠେଲ = sqlite3VdbeCurrentAddr(v);
#endif

  ଯଦି( nPrefixReg ){
    ପ୍ରତିଜ୍ଞା( nPrefixReg==n_ପଦସମଷ୍ଟି+bSeq );
    regBase = regData - nPrefixReg;
  }ଅନ୍ୟଥା{
    regBase = pParse->nMem + 1;
    pParse->nMem += nBase;
  }
  ପ୍ରତିଜ୍ଞା( pSelect->iOffset==0 || pSelect->iLimit!=0 );
  iLimit = pSelect->iOffset ? pSelect->iOffset+1 : pSelect->iLimit;
  pSort->labelDone = sqlite3VdbeMakeLabel(pParse);
  sqlite3_ପଦସମଷ୍ଟି_Code_ପଦସମଷ୍ଟି_List(pParse, pSort->pOrderBy, regBase, regOrigData,
                          SQLITE_ECEL_DUP | (regOrigData? SQLITE_ECEL_REF : 0));
  ଯଦି( bSeq ){
    sqlite3VdbeAddOp2(v, OP_Sequence, pSort->iECursor, regBase+n_ପଦସମଷ୍ଟି);
  }
  ଯଦି( nPrefixReg==0 && nData>0 ){
    sqlite3_ପଦସମଷ୍ଟି_Code_Move(pParse, regData, regBase+n_ପଦସମଷ୍ଟି+bSeq, nData);
  }
  ଯଦି( nOBSat>0 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg_Prev_ଚାବି;   /* The first nOBSat columns of the previous row */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrFirst;    /* Address of the OP_IfNot opcode */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrJmp;      /* Address of the OP_Jump opcode */
    VdbeOp *pOp;      /* Opcode that opens the sorter */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଚାବି;         /* Number of sorting key columns, including OP_Sequence */
    ଚାବି_Info *pKI;     /* Original KeyInfo on the sorter table */

    regRecord = makeSorterRecord(pParse, pSort, pSelect, regBase, nBase);
    reg_Prev_ଚାବି = pParse->nMem+1;
    pParse->nMem += pSort->nOBSat;
    n_ଚାବି = n_ପଦସମଷ୍ଟି - pSort->nOBSat + bSeq;
    ଯଦି( bSeq ){
      addrFirst = sqlite3VdbeAddOp1(v, OP_IfNot, regBase+n_ପଦସମଷ୍ଟି);
    }ଅନ୍ୟଥା{
      addrFirst = sqlite3VdbeAddOp1(v, OP_SequenceTest, pSort->iECursor);
    }
    VdbeCoverage(v);
    sqlite3VdbeAddOp3(v, OP_Compare, reg_Prev_ଚାବି, regBase, pSort->nOBSat);
    pOp = sqlite3VdbeGetOp(v, pSort->addrSortIndex);
    ଯଦି( pParse->db->mallocFailed ) ଫେରନ୍ତୁ;
    pOp->p2 = n_ଚାବି + nData;
    pKI = pOp->p4.p_ଚାବି_Info;
    memset(pKI->aSortFlags, 0, pKI->n_ଚାବି_Field); /* Makes OP_Jump testable */
    sqlite3VdbeChangeP4(v, -1, (ଅକ୍ଷର*)pKI, P4_KEYINFO);
    testcase( pKI->nAllField > pKI->n_ଚାବି_Field+2 );
    pOp->p4.p_ଚାବି_Info = sqlite3_ଚାବି_Info_From_ପଦସମଷ୍ଟି_List(pParse,pSort->pOrderBy,nOBSat,
                                           pKI->nAllField-pKI->n_ଚାବି_Field-1);
    pOp = 0; /* Ensure pOp not used after sqlite3VdbeAddOp3() */
    addrJmp = sqlite3VdbeCurrentAddr(v);
    sqlite3VdbeAddOp3(v, OP_Jump, addrJmp+1, 0, addrJmp+1); VdbeCoverage(v);
    pSort->labelBkOut = sqlite3VdbeMakeLabel(pParse);
    pSort->regReturn = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Gosub, pSort->regReturn, pSort->labelBkOut);
    sqlite3VdbeAddOp1(v, OP_ResetSorter, pSort->iECursor);
    ଯଦି( iLimit ){
      sqlite3VdbeAddOp2(v, OP_IfNot, iLimit, pSort->labelDone);
      VdbeCoverage(v);
    }
    sqlite3VdbeJumpHere(v, addrFirst);
    sqlite3_ପଦସମଷ୍ଟି_Code_Move(pParse, regBase, reg_Prev_ଚାବି, pSort->nOBSat);
    sqlite3VdbeJumpHere(v, addrJmp);
  }
  ଯଦି( iLimit ){
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
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCsr = pSort->iECursor;
    sqlite3VdbeAddOp2(v, OP_IfNotZero, iLimit, sqlite3VdbeCurrentAddr(v)+4);
    VdbeCoverage(v);
    sqlite3VdbeAddOp2(v, OP_Last, iCsr, 0);
    iSkip = sqlite3VdbeAddOp4Int(v, OP_IdxLE,
                                 iCsr, 0, regBase+nOBSat, n_ପଦସମଷ୍ଟି-nOBSat);
    VdbeCoverage(v);
    sqlite3VdbeAddOp1(v, OP_Delete, iCsr);
  }
  ଯଦି( regRecord==0 ){
    regRecord = makeSorterRecord(pParse, pSort, pSelect, regBase, nBase);
  }
  ଯଦି( pSort->sortFlags & SORTFLAG_UseSorter ){
    op = OP_Sorter_ଭର୍ତ୍ତି;
  }ଅନ୍ୟଥା{
    op = OP_Idx_ଭର୍ତ୍ତି;
  }
  sqlite3VdbeAddOp4Int(v, op, pSort->iECursor, regRecord,
                       regBase+nOBSat, nBase-nOBSat);
  ଯଦି( iSkip ){
    sqlite3VdbeChangeP2(v, iSkip,
         pSort->labelOBLopt ? pSort->labelOBLopt : sqlite3VdbeCurrentAddr(v));
  }
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  pSort->addr_ଠେଲ_End = sqlite3VdbeCurrentAddr(v)-1;
#endif
}

/*
** Add code to implement the OFFSET
*/
ସ୍ଥିର ଶୂନ୍ୟ codeOffset(
  Vdbe *v,          /* Generate code into this VM */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOffset,      /* Register holding the offset counter */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iContinue     /* Jump here to skip the current record */
){
  ଯଦି( iOffset>0 ){
    sqlite3VdbeAddOp3(v, OP_IfPos, iOffset, iContinue, 1); VdbeCoverage(v);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା codeDistinct(
  Parse *pParse,     /* Parsing and code generating context */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eTnctType,     /* WHERE_DISTINCT_* value */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTab,          /* A sorting index used to test for distinctness */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrRepeat,    /* Jump to here if not distinct */
  ପଦସମଷ୍ଟି_List *pEList,  /* Expression for each element */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regElem        /* First element */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRet = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nResultCol = pEList->n_ପଦସମଷ୍ଟି;
  Vdbe *v = pParse->pVdbe;

  ବାଛ( eTnctType ){
    ସ୍ଥିତି WHERE_DISTINCT_ORDERED: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iJump;              /* Jump destination */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା regPrev;            /* Previous row content */

      /* Allocate space for the previous row */
      iRet = regPrev = pParse->nMem+1;
      pParse->nMem += nResultCol;

      iJump = sqlite3VdbeCurrentAddr(v) + nResultCol;
      ପାଇଁ(i=0; i<nResultCol; i++){
        CollSeq *pColl = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, pEList->a[i].p_ପଦସମଷ୍ଟି);
        ଯଦି( i<nResultCol-1 ){
          sqlite3VdbeAddOp3(v, OP_Ne, regElem+i, iJump, regPrev+i);
          VdbeCoverage(v);
        }ଅନ୍ୟଥା{
          sqlite3VdbeAddOp3(v, OP_Eq, regElem+i, addrRepeat, regPrev+i);
          VdbeCoverage(v);
         }
        sqlite3VdbeChangeP4(v, -1, (ଅଚଳ ଅକ୍ଷର *)pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
      }
      ପ୍ରତିଜ୍ଞା( sqlite3VdbeCurrentAddr(v)==iJump || pParse->db->mallocFailed );
      sqlite3VdbeAddOp3(v, OP_Copy, regElem, regPrev, nResultCol-1);
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି WHERE_DISTINCT_UNIQUE: {
      /* nothing to do */
      ଭାଙ୍ଗ;
    }

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp4Int(v, OP_Found, iTab, addrRepeat, regElem, nResultCol);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regElem, nResultCol, r1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ଭର୍ତ୍ତି, iTab, r1, regElem, nResultCol);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      sqlite3ReleaseTempReg(pParse, r1);
      iRet = iTab;
      ଭାଙ୍ଗ;
    }
  }

  ଫେରନ୍ତୁ iRet;
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
ସ୍ଥିର ଶୂନ୍ୟ fixDistinctOpenEph(
  Parse *pParse,     /* Parsing and code generating context */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eTnctType,     /* WHERE_DISTINCT_* value */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iVal,          /* Value returned by codeDistinct() */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOpenEphAddr   /* Address of OP_OpenEphemeral instruction for iTab */
){
  ଯଦି( pParse->nErr==0
   && (eTnctType==WHERE_DISTINCT_UNIQUE || eTnctType==WHERE_DISTINCT_ORDERED)
  ){
    Vdbe *v = pParse->pVdbe;
    sqlite3VdbeChangeToNoop(v, iOpenEphAddr);
    ଯଦି( sqlite3VdbeGetOp(v, iOpenEphAddr+1)->opcode==OP_Explain ){
      sqlite3VdbeChangeToNoop(v, iOpenEphAddr+1);
    }
    ଯଦି( eTnctType==WHERE_DISTINCT_ORDERED ){
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

#ifdef SQLITE_ENABLE_SORTER_REFERENCES
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
ସ୍ଥିର ଶୂନ୍ୟ select_ପଦସମଷ୍ଟି_Defer(
  Parse *pParse,                  /* Leave any error here */
  SortCtx *pSort,                 /* Sorter context */
  ପଦସମଷ୍ଟି_List *pEList,               /* Expressions destined for sorter */
  ପଦସମଷ୍ଟି_List **ppExtra              /* Expressions to append to sorter record */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nDefer = 0;
  ପଦସମଷ୍ଟି_List *pExtra = 0;
  ପାଇଁ(i=0; i<pEList->n_ପଦସମଷ୍ଟି; i++){
    ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem = &pEList->a[i];
    ଯଦି( pItem->u.x.iOrderByCol==0 ){
      ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି = pItem->p_ପଦସମଷ୍ଟି;
      Table *pTab;
      ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_COLUMN
       && p_ପଦସମଷ୍ଟି->iColumn>=0
       && ALWAYS( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) )
       && (pTab = p_ପଦସମଷ୍ଟି->y.pTab)!=0
       && IsOrdinaryTable(pTab)
       && (pTab->aCol[p_ପଦସମଷ୍ଟି->iColumn].colFlags & COLFLAG_SORTERREF)!=0
      ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
        ପାଇଁ(j=0; j<nDefer; j++){
          ଯଦି( pSort->aDefer[j].iCsr==p_ପଦସମଷ୍ଟି->iTable ) ଭାଙ୍ଗ;
        }
        ଯଦି( j==nDefer ){
          ଯଦି( nDefer==ArraySize(pSort->aDefer) ){
            ଜାରି;
          }ଅନ୍ୟଥା{
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଚାବି = 1;
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା k;
            Index *pPk = 0;
            ଯଦି( !HasRowid(pTab) ){
              pPk = sqlite3_Primary_ଚାବି_Index(pTab);
              n_ଚାବି = pPk->n_ଚାବି_Col;
            }
            ପାଇଁ(k=0; k<n_ଚାବି; k++){
              ପଦସମଷ୍ଟି *pNew = sqlite3PExpr(pParse, TK_COLUMN, 0, 0);
              ଯଦି( pNew ){
                pNew->iTable = p_ପଦସମଷ୍ଟି->iTable;
                ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(pNew) );
                pNew->y.pTab = p_ପଦସମଷ୍ଟି->y.pTab;
                pNew->iColumn = pPk ? pPk->aiColumn[k] : -1;
                pExtra = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, pExtra, pNew);
              }
            }
            pSort->aDefer[nDefer].pTab = p_ପଦସମଷ୍ଟି->y.pTab;
            pSort->aDefer[nDefer].iCsr = p_ପଦସମଷ୍ଟି->iTable;
            pSort->aDefer[nDefer].n_ଚାବି = n_ଚାବି;
            nDefer++;
          }
        }
        pItem->fg.bSorterRef = 1;
      }
    }
  }
  pSort->nDefer = (u8)nDefer;
  *ppExtra = pExtra;
}
#endif

#ifdef SQLITE_DEBUG
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3CursorBloomable(Parse *pParse, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCur, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i,k;
  Vdbe *v = pParse->pVdbe;
  ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ 1;
  ପ୍ରତିଜ୍ଞା( v );
  ପାଇଁ(k=sqlite3VdbeCurrentAddr(v)-1; k>0; k--){
    ଅଚଳ VdbeOp *pOp = sqlite3VdbeGetOp(v, k);
    ଅଚଳ ଚାବି_Info *p_ଚାବି_Info;
    ଯଦି( pOp->p1!=iCur ) ଜାରି;
    ଯଦି( pOp->opcode!=OP_OpenRead
     && pOp->opcode!=OP_OpenWrite
     && pOp->opcode!=OP_OpenEphemeral
    ){
      ଜାରି;
    }
    ପ୍ରତିଜ୍ଞା( pOp->p4type==P4_KEYINFO );
    p_ଚାବି_Info = (ଅଚଳ ଚାବି_Info*)pOp->p4.p_ଚାବି_Info;
    ପ୍ରତିଜ୍ଞା( p_ଚାବି_Info!=0 );
    ପାଇଁ(i=0; i<nCol; i++){
      ପ୍ରତିଜ୍ଞା( sqlite3_Is_ଦ୍ୱିମିକ(p_ଚାବି_Info->aColl[i]) );
    }
    ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
}
#endif /* SQLITE_DEBUG */


/*
** This routine generates the code for the inside of the inner loop
** of a SELECT.
**
** If srcTab is negative, then the p->pEList expressions
** are evaluated in order to get the data for this row.  If srcTab is
** zero or more, then data is pulled from srcTab and p->pEList is used only
** to get the number of columns and the collation sequence for each column.
*/
ସ୍ଥିର ଶୂନ୍ୟ selectInnerLoop(
  Parse *pParse,          /* The parser context */
  Select *p,              /* The complete select statement being coded */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା srcTab,             /* Pull data from this table if non-negative */
  SortCtx *pSort,         /* If not NULL, info on how to process ORDER BY */
  DistinctCtx *pDistinct, /* If not NULL, info on how to process DISTINCT */
  SelectDest *pDest,      /* How to dispose of the results */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iContinue,          /* Jump here to continue with next row */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iBreak              /* Jump here to break out of the inner loop */
){
  Vdbe *v = pParse->pVdbe;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା hasDistinct;            /* True if the DISTINCT keyword is present */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eDest = pDest->eDest;   /* How to dispose of results */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iParm = pDest->iSDParm; /* First argument to disposal method */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nResultCol;             /* Number of result columns */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPrefixReg = 0;         /* Number of extra registers before regResult */
  RowLoadInfo sRowLoadInfo;   /* Info for deferred row loading */

  /* Usually, regResult is the first cell in an array of memory cells
  ** containing the current result row. In this case regOrig is set to the
  ** same value. However, if the results are being sent to the sorter, the
  ** values for any expressions that are also part of the sort-key are omitted
  ** from this array. In this case regOrig is set to zero.  */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regResult;              /* Start of memory holding current results */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regOrig;                /* Start of memory holding full result (or 0) */

  ପ୍ରତିଜ୍ଞା( v );
  ପ୍ରତିଜ୍ଞା( p->pEList!=0 );
  hasDistinct = pDistinct ? pDistinct->eTnctType : WHERE_DISTINCT_NOOP;
  ଯଦି( pSort && pSort->pOrderBy==0 ) pSort = 0;
  ଯଦି( pSort==0 && !hasDistinct ){
    ପ୍ରତିଜ୍ଞା( iContinue!=0 );
    codeOffset(v, p->iOffset, iContinue);
  }

  /* Pull the requested columns.
  */
  nResultCol = p->pEList->n_ପଦସମଷ୍ଟି;

  ଯଦି( pDest->iSdst==0 ){
    ଯଦି( pSort ){
      nPrefixReg = pSort->pOrderBy->n_ପଦସମଷ୍ଟି;
      ଯଦି( !(pSort->sortFlags & SORTFLAG_UseSorter) ) nPrefixReg++;
      pParse->nMem += nPrefixReg;
    }
    pDest->iSdst = pParse->nMem+1;
    pParse->nMem += nResultCol;
  }ଅନ୍ୟଥା ଯଦି( pDest->iSdst+nResultCol > pParse->nMem ){
    /* This is an error condition that can result, for example, when a SELECT
    ** on the right-hand side of an INSERT contains more result columns than
    ** there are columns in the table on the left.  The error will be caught
    ** and reported later.  But we need to make sure enough memory is allocated
    ** to avoid other spurious errors in the meantime. */
    pParse->nMem += nResultCol;
  }
  pDest->nSdst = nResultCol;
  regOrig = regResult = pDest->iSdst;
  ଯଦି( srcTab>=0 ){
    ପାଇଁ(i=0; i<nResultCol; i++){
      sqlite3VdbeAddOp3(v, OP_Column, srcTab, i, regResult+i);
      VdbeComment((v, "%s", p->pEList->a[i].zEName));
    }
  }ଅନ୍ୟଥା ଯଦି( eDest!=SRT_Exists ){
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    ପଦସମଷ୍ଟି_List *pExtra = 0;
#endif
    /* If the destination is an EXISTS(...) expression, the actual
    ** values returned by the SELECT are not required.
    */
    u8 ecelFlags;    /* "ecel" is an abbreviation of "ExprCodeExprList" */
    ପଦସମଷ୍ଟି_List *pEList;
    ଯଦି( eDest==SRT_Mem || eDest==SRT_Output || eDest==SRT_Coroutine ){
      ecelFlags = SQLITE_ECEL_DUP;
    }ଅନ୍ୟଥା{
      ecelFlags = 0;
    }
    ଯଦି( pSort && hasDistinct==0 && eDest!=SRT_EphemTab && eDest!=SRT_Table ){
      /* For each expression in p->pEList that is a copy of an expression in
      ** the ORDER BY clause (pSort->pOrderBy), set the associated
      ** iOrderByCol value to one more than the index of the ORDER BY
      ** expression within the sort-key that pushOntoSorter() will generate.
      ** This allows the p->pEList field to be omitted from the sorted record,
      ** saving space and CPU cycles.  */
      ecelFlags |= (SQLITE_ECEL_OMITREF|SQLITE_ECEL_REF);

      ପାଇଁ(i=pSort->nOBSat; i<pSort->pOrderBy->n_ପଦସମଷ୍ଟି; i++){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
        ଯଦି( (j = pSort->pOrderBy->a[i].u.x.iOrderByCol)>0 ){
          p->pEList->a[j-1].u.x.iOrderByCol = i+1-pSort->nOBSat;
        }
      }
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
      select_ପଦସମଷ୍ଟି_Defer(pParse, pSort, p->pEList, &pExtra);
      ଯଦି( pExtra && pParse->db->mallocFailed==0 ){
        /* If there are any extra PK columns to add to the sorter records,
        ** allocate extra memory cells and adjust the OpenEphemeral
        ** instruction to account for the larger records. This is only
        ** required if there are one or more WITHOUT ROWID tables with
        ** composite primary keys in the SortCtx.aDefer[] array.  */
        VdbeOp *pOp = sqlite3VdbeGetOp(v, pSort->addrSortIndex);
        pOp->p2 += (pExtra->n_ପଦସମଷ୍ଟି - pSort->nDefer);
        pOp->p4.p_ଚାବି_Info->nAllField += (pExtra->n_ପଦସମଷ୍ଟି - pSort->nDefer);
        pParse->nMem += pExtra->n_ପଦସମଷ୍ଟି;
      }
#endif

      /* Adjust nResultCol to account for columns that are omitted
      ** from the sorter by the optimizations in this branch */
      pEList = p->pEList;
      ପାଇଁ(i=0; i<pEList->n_ପଦସମଷ୍ଟି; i++){
        ଯଦି( pEList->a[i].u.x.iOrderByCol>0
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
         || pEList->a[i].fg.bSorterRef
#endif
        ){
          nResultCol--;
          regOrig = 0;
        }
      }

      testcase( regOrig );
      testcase( eDest==SRT_Set );
      testcase( eDest==SRT_Mem );
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      ପ୍ରତିଜ୍ଞା( eDest==SRT_Set || eDest==SRT_Mem
           || eDest==SRT_Coroutine || eDest==SRT_Output
           || eDest==SRT_Upfrom );
    }
    sRowLoadInfo.regResult = regResult;
    sRowLoadInfo.ecelFlags = ecelFlags;
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    sRowLoadInfo.pExtra = pExtra;
    sRowLoadInfo.regExtraResult = regResult + nResultCol;
    ଯଦି( pExtra ) nResultCol += pExtra->n_ପଦସମଷ୍ଟି;
#endif
    ଯଦି( p->iLimit
     && (ecelFlags & SQLITE_ECEL_OMITREF)!=0
     && nPrefixReg>0
    ){
      ପ୍ରତିଜ୍ଞା( pSort!=0 );
      ପ୍ରତିଜ୍ଞା( hasDistinct==0 );
      pSort->pDeferredRowLoad = &sRowLoadInfo;
      regOrig = 0;
    }ଅନ୍ୟଥା{
      innerLoopLoadRow(pParse, p, &sRowLoadInfo);
    }
  }

  /* If the DISTINCT keyword was present on the SELECT statement
  ** and this row has been seen before, then do not make this row
  ** part of the result.
  */
  ଯଦି( hasDistinct ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା eType = pDistinct->eTnctType;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTab = pDistinct->tabTnct;
    ପ୍ରତିଜ୍ଞା( nResultCol==p->pEList->n_ପଦସମଷ୍ଟି );
    iTab = codeDistinct(pParse, eType, iTab, iContinue, p->pEList, regResult);
    fixDistinctOpenEph(pParse, eType, iTab, pDistinct->addrTnct);
    ଯଦି( pSort==0 ){
      codeOffset(v, p->iOffset, iContinue);
    }
  }

  ବାଛ( eDest ){
    /* Store the result as data using a unique key.
    */
    ସ୍ଥିତି SRT_Fifo:
    ସ୍ଥିତି SRT_DistFifo:
    ସ୍ଥିତି SRT_Table:
    ସ୍ଥିତି SRT_EphemTab: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1 = sqlite3GetTempRange(pParse, nPrefixReg+1);
      testcase( eDest==SRT_Table );
      testcase( eDest==SRT_EphemTab );
      testcase( eDest==SRT_Fifo );
      testcase( eDest==SRT_DistFifo );
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regResult, nResultCol, r1+nPrefixReg);
#ଯଦି !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      ଯଦି( eDest==SRT_Table && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#endif
#ifndef SQLITE_OMIT_CTE
      ଯଦି( eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index. If the current row is already present
        ** in the index, do not write it to the output. If not, add the
        ** current row to the index and proceed with writing it to the
        ** output table as well.  */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr = sqlite3VdbeCurrentAddr(v) + 4;
        sqlite3VdbeAddOp4Int(v, OP_Found, iParm+1, addr, r1, 0);
        VdbeCoverage(v);
        sqlite3VdbeAddOp4Int(v, OP_Idx_ଭର୍ତ୍ତି, iParm+1, r1,regResult,nResultCol);
        ପ୍ରତିଜ୍ଞା( pSort==0 );
      }
#endif
      ଯଦି( pSort ){
        ପ୍ରତିଜ୍ଞା( regResult==regOrig );
        ଠେଲ_Onto_Sorter(pParse, pSort, p, r1+nPrefixReg, regOrig, 1, nPrefixReg);
      }ଅନ୍ୟଥା{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା r2 = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp2(v, OP_NewRowid, iParm, r2);
        sqlite3VdbeAddOp3(v, OP_ଭର୍ତ୍ତି, iParm, r1, r2);
        sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
        sqlite3ReleaseTempReg(pParse, r2);
      }
      sqlite3ReleaseTempRange(pParse, r1, nPrefixReg+1);
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି SRT_Upfrom: {
      ଯଦି( pSort ){
        ଠେଲ_Onto_Sorter(
            pParse, pSort, p, regResult, regOrig, nResultCol, nPrefixReg);
      }ଅନ୍ୟଥା{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i2 = pDest->iSDParm2;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1 = sqlite3GetTempReg(pParse);

        /* If the UPDATE FROM join is an aggregate that matches no rows, it
        ** might still be trying to return one row, because that is what
        ** aggregates do.  Don't record that empty row in the output table. */
        sqlite3VdbeAddOp2(v, OP_IsNull, regResult, iBreak); VdbeCoverage(v);

        sqlite3VdbeAddOp3(v, OP_MakeRecord,
                          regResult+(i2<0), nResultCol-(i2<0), r1);
        ଯଦି( i2<0 ){
          sqlite3VdbeAddOp3(v, OP_ଭର୍ତ୍ତି, iParm, r1, regResult);
        }ଅନ୍ୟଥା{
          sqlite3VdbeAddOp4Int(v, OP_Idx_ଭର୍ତ୍ତି, iParm, r1, regResult, i2);
        }
      }
      ଭାଙ୍ଗ;
    }

#ifndef SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)" construct,
    ** then there should be a single item on the stack.  Write this
    ** item into the set table with bogus data.
    */
    ସ୍ଥିତି SRT_Set: {
      ଯଦି( pSort ){
        /* At first glance you would think we could optimize out the
        ** ORDER BY in this case since the order of entries in the set
        ** does not matter.  But there might be a LIMIT clause, in which
        ** case the order does matter */
        ଠେଲ_Onto_Sorter(
            pParse, pSort, p, regResult, regOrig, nResultCol, nPrefixReg);
        pDest->iSDParm2 = 0; /* Signal that any Bloom filter is unpopulated */
      }ଅନ୍ୟଥା{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1 = sqlite3GetTempReg(pParse);
        ପ୍ରତିଜ୍ଞା( sqlite3Strlen30(pDest->zAffSdst)==nResultCol );
        sqlite3VdbeAddOp4(v, OP_MakeRecord, regResult, nResultCol,
            r1, pDest->zAffSdst, nResultCol);
        sqlite3VdbeAddOp4Int(v, OP_Idx_ଭର୍ତ୍ତି, iParm, r1, regResult, nResultCol);
        ଯଦି( pDest->iSDParm2 ){
          ପ୍ରତିଜ୍ଞା( sqlite3CursorBloomable(pParse,iParm,nResultCol) );
          sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pDest->iSDParm2, 0,
                               regResult, nResultCol);
          ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
        }
        sqlite3ReleaseTempReg(pParse, r1);
      }
      ଭାଙ୍ଗ;
    }


    /* If any row exist in the result set, record that fact and abort.
    */
    ସ୍ଥିତି SRT_Exists: {
      sqlite3VdbeAddOp2(v, OP_Integer, 1, iParm);
      /* The LIMIT clause will terminate the loop for us */
      ଭାଙ୍ଗ;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell or array of
    ** memory cells and break out of the scan loop.
    */
    ସ୍ଥିତି SRT_Mem: {
      ଯଦି( pSort ){
        ପ୍ରତିଜ୍ଞା( nResultCol<=pDest->nSdst );
        ଠେଲ_Onto_Sorter(
            pParse, pSort, p, regResult, regOrig, nResultCol, nPrefixReg);
        pDest->iSDParm = regResult;
      }ଅନ୍ୟଥା{
        ପ୍ରତିଜ୍ଞା( nResultCol==pDest->nSdst );
        ଯଦି( regResult!=iParm ){
          /* This occurs in cases where the SELECT had both a DISTINCT and
          ** an OFFSET clause.  */
          sqlite3VdbeAddOp3(v, OP_Copy, regResult, iParm, nResultCol-1);
        }
        /* The LIMIT clause will jump out of the loop for us */
      }
      ଭାଙ୍ଗ;
    }
#endif /* #ifndef SQLITE_OMIT_SUBQUERY */

    ସ୍ଥିତି SRT_Coroutine:       /* Send data to a co-routine */
    ସ୍ଥିତି SRT_Output: {        /* Return the results */
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      ଯଦି( pSort ){
        ଠେଲ_Onto_Sorter(pParse, pSort, p, regResult, regOrig, nResultCol,
                       nPrefixReg);
      }ଅନ୍ୟଥା ଯଦି( eDest==SRT_Coroutine ){
        sqlite3VdbeAddOp1(v, OP_Yield, pDest->iSDParm);
      }ଅନ୍ୟଥା{
        sqlite3VdbeAddOp2(v, OP_ResultRow, regResult, nResultCol);
      }
      ଭାଙ୍ଗ;
    }

#ifndef SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    ସ୍ଥିତି SRT_DistQueue:
    ସ୍ଥିତି SRT_Queue: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଚାବି;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1, r2, r3;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrTest = 0;
      ପଦସମଷ୍ଟି_List *pSO;
      pSO = pDest->pOrderBy;
      ପ୍ରତିଜ୍ଞା( pSO );
      n_ଚାବି = pSO->n_ପଦସମଷ୍ଟି;
      r1 = sqlite3GetTempReg(pParse);
      r2 = sqlite3GetTempRange(pParse, n_ଚାବି+2);
      r3 = r2+n_ଚାବି+1;
      ଯଦି( eDest==SRT_DistQueue ){
        /* If the destination is DistQueue, then cursor (iParm+1) is open
        ** on a second ephemeral index that holds all values every previously
        ** added to the queue. */
        addrTest = sqlite3VdbeAddOp4Int(v, OP_Found, iParm+1, 0,
                                        regResult, nResultCol);
        VdbeCoverage(v);
      }
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regResult, nResultCol, r3);
      ଯଦି( eDest==SRT_DistQueue ){
        sqlite3VdbeAddOp2(v, OP_Idx_ଭର୍ତ୍ତି, iParm+1, r3);
        sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      }
      ପାଇଁ(i=0; i<n_ଚାବି; i++){
        sqlite3VdbeAddOp2(v, OP_SCopy,
                          regResult + pSO->a[i].u.x.iOrderByCol - 1,
                          r2+i);
      }
      sqlite3VdbeAddOp2(v, OP_Sequence, iParm, r2+n_ଚାବି);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, r2, n_ଚାବି+2, r1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ଭର୍ତ୍ତି, iParm, r1, r2, n_ଚାବି+2);
      ଯଦି( addrTest ) sqlite3VdbeJumpHere(v, addrTest);
      sqlite3ReleaseTempReg(pParse, r1);
      sqlite3ReleaseTempRange(pParse, r2, n_ଚାବି+2);
      ଭାଙ୍ଗ;
    }
#endif /* SQLITE_OMIT_CTE */



#ଯଦି !defined(SQLITE_OMIT_TRIGGER)
    /* Discard the results.  This is used for SELECT statements inside
    ** the body of a TRIGGER.  The purpose of such selects is to call
    ** user-defined functions that have side effects.  We do not care
    ** about the actual results of the select.
    */
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      ପ୍ରତିଜ୍ଞା( eDest==SRT_Discard );
      ଭାଙ୍ଗ;
    }
#endif
  }

  /* Jump to the end of the loop if the LIMIT is reached.  Except, if
  ** there is a sorter, in which case the sorter has already limited
  ** the output for us.
  */
  ଯଦି( pSort==0 && p->iLimit ){
    sqlite3VdbeAddOp2(v, OP_DecrJumpZero, p->iLimit, iBreak); VdbeCoverage(v);
  }
}

/*
** Allocate a KeyInfo object sufficient for an index of N key columns and
** X extra columns.
*/
ଚାବି_Info *sqlite3_ଚାବି_Info_Alloc(sqlite3 *db, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N, ପୂର୍ଣ୍ଣସଂଖ୍ୟା X){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nExtra = (N+X)*(ଆକାର(CollSeq*)+1);
  ଚାବି_Info *p;
  ପ୍ରତିଜ୍ଞା( X>=0 );
  ଯଦି( NEVER(N+X>0xffff) ) ଫେରନ୍ତୁ (ଚାବି_Info*)sqlite3OomFault(db);
  p = sqlite3DbMallocRawNN(db, SZ_KEYINFO(0) + nExtra);
  ଯଦି( p ){
    p->aSortFlags = (u8*)&p->aColl[N+X];
    p->n_ଚାବି_Field = (u16)N;
    p->nAllField = (u16)(N+X);
    p->enc = ENC(db);
    p->db = db;
    p->nRef = 1;
    memset(p->aColl, 0, nExtra);
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ (ଚାବି_Info*)sqlite3OomFault(db);
  }
  ଫେରନ୍ତୁ p;
}

/*
** Deallocate a KeyInfo object
*/
ଶୂନ୍ୟ sqlite3_ଚାବି_Info_Unref(ଚାବି_Info *p){
  ଯଦି( p ){
    ପ୍ରତିଜ୍ଞା( p->db!=0 );
    ପ୍ରତିଜ୍ଞା( p->nRef>0 );
    p->nRef--;
    ଯଦି( p->nRef==0 ) sqlite3DbNNFreeNN(p->db, p);
  }
}

/*
** Make a new pointer to a KeyInfo object
*/
ଚାବି_Info *sqlite3_ଚାବି_Info_Ref(ଚାବି_Info *p){
  ଯଦି( p ){
    ପ୍ରତିଜ୍ଞା( p->nRef>0 );
    p->nRef++;
  }
  ଫେରନ୍ତୁ p;
}

#ifdef SQLITE_DEBUG
/*
** Return TRUE if a KeyInfo object can be change.  The KeyInfo object
** can only be changed if this is just a single reference to the object.
**
** This routine is used only inside of assert() statements.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ଚାବି_Info_Is_Writeable(ଚାବି_Info *p){ ଫେରନ୍ତୁ p->nRef==1; }
#endif /* SQLITE_DEBUG */

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
ଚାବି_Info *sqlite3_ଚାବି_Info_From_ପଦସମଷ୍ଟି_List(
  Parse *pParse,       /* Parsing context */
  ପଦସମଷ୍ଟି_List *pList,     /* Form the KeyInfo object from this ExprList */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iStart,          /* Begin with this column of pList */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nExtra           /* Add this many extra columns to the end */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ପଦସମଷ୍ଟି;
  ଚାବି_Info *pInfo;
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
  sqlite3 *db = pParse->db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  n_ପଦସମଷ୍ଟି = pList->n_ପଦସମଷ୍ଟି;
  pInfo = sqlite3_ଚାବି_Info_Alloc(db, n_ପଦସମଷ୍ଟି-iStart, nExtra+1);
  ଯଦି( pInfo ){
    ପ୍ରତିଜ୍ଞା( sqlite3_ଚାବି_Info_Is_Writeable(pInfo) );
    ପାଇଁ(i=iStart, pItem=pList->a+iStart; i<n_ପଦସମଷ୍ଟି; i++, pItem++){
      pInfo->aColl[i-iStart] = sqlite3_ପଦସମଷ୍ଟି_NNColl_Seq(pParse, pItem->p_ପଦସମଷ୍ଟି);
      pInfo->aSortFlags[i-iStart] = pItem->fg.sortFlags;
    }
  }
  ଫେରନ୍ତୁ pInfo;
}

/*
** Name of the connection operator, used for error messages.
*/
ଅଚଳ ଅକ୍ଷର *sqlite3SelectOpName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା id){
  ଅକ୍ଷର *z;
  ବାଛ( id ){
    ସ୍ଥିତି TK_ALL:       z = "UNION ALL";   ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_INTERSECT: z = "INTERSECT";   ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_EXCEPT:    z = "EXCEPT";      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:           z = "UNION";       ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ z;
}

#ifndef SQLITE_OMIT_EXPLAIN
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
ସ୍ଥିର ଶୂନ୍ୟ explainTempTable(Parse *pParse, ଅଚଳ ଅକ୍ଷର *zUsage){
  ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s", zUsage));
}

/*
** Assign expression b to lvalue a. A second, no-op, version of this macro
** is provided when SQLITE_OMIT_EXPLAIN is defined. This allows the code
** in sqlite3Select() to assign values to structure member variables that
** only exist if SQLITE_OMIT_EXPLAIN is not defined without polluting the
** code with #ifndef directives.
*/
# define explainSetInteger(a, b) a = b

#ଅନ୍ୟଥା
/* No-op versions of the explainXXX() functions and macros. */
# define explainTempTable(y,z)
# define explainSetInteger(y,z)
#endif


/*
** If the inner loop was generated using a non-null pOrderBy argument,
** then the results were placed in a sorter.  After the loop is terminated
** we need to run the sorter and output the results.  The following
** routine generates the code needed to do that.
*/
ସ୍ଥିର ଶୂନ୍ୟ generateSortTail(
  Parse *pParse,    /* Parsing context */
  Select *p,        /* The SELECT statement */
  SortCtx *pSort,   /* Information on the ORDER BY clause */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nColumn,      /* Number of columns of data */
  SelectDest *pDest /* Write the sorted results here */
){
  Vdbe *v = pParse->pVdbe;                     /* The prepared statement */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrBreak = pSort->labelDone;            /* Jump here to exit loop */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrContinue = sqlite3VdbeMakeLabel(pParse);/* Jump here for next cycle */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr;                       /* Top of output loop. Jump for Next. */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrOnce = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTab;
  ପଦସମଷ୍ଟି_List *pOrderBy = pSort->pOrderBy;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eDest = pDest->eDest;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iParm = pDest->iSDParm;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regRow;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regRowid;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଚାବି;                       /* Number of key columns in sorter record */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSortTab;                   /* Sorter cursor to read from */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bSeq;                       /* True if sorter record includes seq. no. */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_Ref_ଚାବି = 0;
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *aOutEx = p->pEList->a;
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrExplain;                /* Address of OP_Explain instruction */
#endif

  n_ଚାବି = pOrderBy->n_ପଦସମଷ୍ଟି - pSort->nOBSat;
  ଯଦି( pSort->nOBSat==0 || n_ଚାବି==1 ){
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR %sORDER BY", pSort->nOBSat?"LAST TERM OF ":""
    ));
  }ଅନ୍ୟଥା{
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR LAST %d TERMS OF ORDER BY", n_ଚାବି
    ));
  }
  sqlite3VdbeScanStatusRange(v, addrExplain,pSort->addr_ଠେଲ,pSort->addr_ଠେଲ_End);
  sqlite3VdbeScanStatusCounters(v, addrExplain, addrExplain, pSort->addr_ଠେଲ);


  ପ୍ରତିଜ୍ଞା( addrBreak<0 );
  ଯଦି( pSort->labelBkOut ){
    sqlite3VdbeAddOp2(v, OP_Gosub, pSort->regReturn, pSort->labelBkOut);
    sqlite3VdbeGoto(v, addrBreak);
    sqlite3VdbeResolveLabel(v, pSort->labelBkOut);
  }

#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  /* Open any cursors needed for sorter-reference expressions */
  ପାଇଁ(i=0; i<pSort->nDefer; i++){
    Table *pTab = pSort->aDefer[i].pTab;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
    sqlite3OpenTable(pParse, pSort->aDefer[i].iCsr, iDb, pTab, OP_OpenRead);
    sqlite3VdbeAddOp2(v, OP_Rewind, pSort->aDefer[i].iCsr,
                      sqlite3VdbeCurrentAddr(v)+1);
    VdbeCoverage(v);
    n_Ref_ଚାବି = MAX(n_Ref_ଚାବି, pSort->aDefer[i].n_ଚାବି);
  }
#endif

  iTab = pSort->iECursor;
  ଯଦି( eDest==SRT_Output || eDest==SRT_Coroutine || eDest==SRT_Mem ){
    ଯଦି( eDest==SRT_Mem && p->iOffset ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, pDest->iSdst);
    }
    regRowid = 0;
    regRow = pDest->iSdst;
  }ଅନ୍ୟଥା{
    regRowid = sqlite3GetTempReg(pParse);
    ଯଦି( eDest==SRT_EphemTab || eDest==SRT_Table ){
      regRow = sqlite3GetTempReg(pParse);
      nColumn = 0;
    }ଅନ୍ୟଥା{
      regRow = sqlite3GetTempRange(pParse, nColumn);
    }
  }
  ଯଦି( pSort->sortFlags & SORTFLAG_UseSorter ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା regSortOut = ++pParse->nMem;
    iSortTab = pParse->nTab++;
    ଯଦି( pSort->labelBkOut ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    }
    sqlite3VdbeAddOp3(v, OP_OpenPseudo, iSortTab, regSortOut,
        n_ଚାବି+1+nColumn+n_Ref_ଚାବି);
    ଯଦି( addrOnce ) sqlite3VdbeJumpHere(v, addrOnce);
    addr = 1 + sqlite3VdbeAddOp2(v, OP_SorterSort, iTab, addrBreak);
    VdbeCoverage(v);
    ପ୍ରତିଜ୍ଞା( p->iLimit==0 && p->iOffset==0 );
    sqlite3VdbeAddOp3(v, OP_SorterData, iTab, regSortOut, iSortTab);
    bSeq = 0;
  }ଅନ୍ୟଥା{
    addr = 1 + sqlite3VdbeAddOp2(v, OP_Sort, iTab, addrBreak); VdbeCoverage(v);
    codeOffset(v, p->iOffset, addrContinue);
    iSortTab = iTab;
    bSeq = 1;
    ଯଦି( p->iOffset>0 ){
      sqlite3VdbeAddOp2(v, OP_AddImm, p->iLimit, -1);
    }
  }
  ପାଇଁ(i=0, iCol=n_ଚାବି+bSeq-1; i<nColumn; i++){
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    ଯଦି( aOutEx[i].fg.bSorterRef ) ଜାରି;
#endif
    ଯଦି( aOutEx[i].u.x.iOrderByCol==0 ) iCol++;
  }
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  ଯଦି( pSort->nDefer ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_ଚାବି = iCol+1;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg_ଚାବି = sqlite3GetTempRange(pParse, n_Ref_ଚାବି);

    ପାଇଁ(i=0; i<pSort->nDefer; i++){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCsr = pSort->aDefer[i].iCsr;
      Table *pTab = pSort->aDefer[i].pTab;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଚାବି = pSort->aDefer[i].n_ଚାବି;

      sqlite3VdbeAddOp1(v, OP_NullRow, iCsr);
      ଯଦି( HasRowid(pTab) ){
        sqlite3VdbeAddOp3(v, OP_Column, iSortTab, i_ଚାବି++, reg_ଚାବି);
        sqlite3VdbeAddOp3(v, OP_SeekRowid, iCsr,
            sqlite3VdbeCurrentAddr(v)+1, reg_ଚାବି);
      }ଅନ୍ୟଥା{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା k;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iJmp;
        ପ୍ରତିଜ୍ଞା( sqlite3_Primary_ଚାବି_Index(pTab)->n_ଚାବି_Col==n_ଚାବି );
        ପାଇଁ(k=0; k<n_ଚାବି; k++){
          sqlite3VdbeAddOp3(v, OP_Column, iSortTab, i_ଚାବି++, reg_ଚାବି+k);
        }
        iJmp = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeAddOp4Int(v, OP_SeekGE, iCsr, iJmp+2, reg_ଚାବି, n_ଚାବି);
        sqlite3VdbeAddOp4Int(v, OP_IdxLE, iCsr, iJmp+3, reg_ଚାବି, n_ଚାବି);
        sqlite3VdbeAddOp1(v, OP_NullRow, iCsr);
      }
    }
    sqlite3ReleaseTempRange(pParse, reg_ଚାବି, n_Ref_ଚାବି);
  }
#endif
  ପାଇଁ(i=nColumn-1; i>=0; i--){
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    ଯଦି( aOutEx[i].fg.bSorterRef ){
      sqlite3_ପଦସମଷ୍ଟି_Code(pParse, aOutEx[i].p_ପଦସମଷ୍ଟି, regRow+i);
    }ଅନ୍ୟଥା
#endif
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRead;
      ଯଦି( aOutEx[i].u.x.iOrderByCol ){
        iRead = aOutEx[i].u.x.iOrderByCol-1;
      }ଅନ୍ୟଥା{
        iRead = iCol--;
      }
      sqlite3VdbeAddOp3(v, OP_Column, iSortTab, iRead, regRow+i);
      VdbeComment((v, "%s", aOutEx[i].zEName));
    }
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);
  ବାଛ( eDest ){
    ସ୍ଥିତି SRT_Table:
    ସ୍ଥିତି SRT_EphemTab: {
      sqlite3VdbeAddOp3(v, OP_Column, iSortTab, n_ଚାବି+bSeq, regRow);
      sqlite3VdbeAddOp2(v, OP_NewRowid, iParm, regRowid);
      sqlite3VdbeAddOp3(v, OP_ଭର୍ତ୍ତି, iParm, regRow, regRowid);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      ଭାଙ୍ଗ;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ସ୍ଥିତି SRT_Set: {
      ପ୍ରତିଜ୍ଞା( nColumn==sqlite3Strlen30(pDest->zAffSdst) );
      sqlite3VdbeAddOp4(v, OP_MakeRecord, regRow, nColumn, regRowid,
                        pDest->zAffSdst, nColumn);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ଭର୍ତ୍ତି, iParm, regRowid, regRow, nColumn);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି SRT_Mem: {
      /* The LIMIT clause will terminate the loop for us */
      ଭାଙ୍ଗ;
    }
#endif
    ସ୍ଥିତି SRT_Upfrom: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i2 = pDest->iSDParm2;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, OP_MakeRecord,regRow+(i2<0),nColumn-(i2<0),r1);
      ଯଦି( i2<0 ){
        sqlite3VdbeAddOp3(v, OP_ଭର୍ତ୍ତି, iParm, r1, regRow);
      }ଅନ୍ୟଥା{
        sqlite3VdbeAddOp4Int(v, OP_Idx_ଭର୍ତ୍ତି, iParm, r1, regRow, i2);
      }
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      ପ୍ରତିଜ୍ଞା( eDest==SRT_Output || eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      testcase( eDest==SRT_Coroutine );
      ଯଦି( eDest==SRT_Output ){
        sqlite3VdbeAddOp2(v, OP_ResultRow, pDest->iSdst, nColumn);
      }ଅନ୍ୟଥା{
        sqlite3VdbeAddOp1(v, OP_Yield, pDest->iSDParm);
      }
      ଭାଙ୍ଗ;
    }
  }
  ଯଦି( regRowid ){
    ଯଦି( eDest==SRT_Set ){
      sqlite3ReleaseTempRange(pParse, regRow, nColumn);
    }ଅନ୍ୟଥା{
      sqlite3ReleaseTempReg(pParse, regRow);
    }
    sqlite3ReleaseTempReg(pParse, regRowid);
  }
  /* The bottom of the loop
  */
  sqlite3VdbeResolveLabel(v, addrContinue);
  ଯଦି( pSort->sortFlags & SORTFLAG_UseSorter ){
    sqlite3VdbeAddOp2(v, OP_SorterNext, iTab, addr); VdbeCoverage(v);
  }ଅନ୍ୟଥା{
    sqlite3VdbeAddOp2(v, OP_Next, iTab, addr); VdbeCoverage(v);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, sqlite3VdbeCurrentAddr(v)-1, -1);
  ଯଦି( pSort->regReturn ) sqlite3VdbeAddOp1(v, OP_Return, pSort->regReturn);
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
#ifdef SQLITE_ENABLE_COLUMN_METADATA
# define columnType(A,B,C,D,E) columnTypeImpl(A,B,C,D,E)
#ଅନ୍ୟଥା /* if !defined(SQLITE_ENABLE_COLUMN_METADATA) */
# define columnType(A,B,C,D,E) columnTypeImpl(A,B)
#endif
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *columnTypeImpl(
  NameContext *pNC,
#ifndef SQLITE_ENABLE_COLUMN_METADATA
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି
#ଅନ୍ୟଥା
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,
  ଅଚଳ ଅକ୍ଷର **pzOrigDb,
  ଅଚଳ ଅକ୍ଷର **pzOrigTab,
  ଅଚଳ ଅକ୍ଷର **pzOrigCol
#endif
){
  ଅକ୍ଷର ଅଚଳ *zType = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
#ifdef SQLITE_ENABLE_COLUMN_METADATA
  ଅକ୍ଷର ଅଚଳ *zOrigDb = 0;
  ଅକ୍ଷର ଅଚଳ *zOrigTab = 0;
  ଅକ୍ଷର ଅଚଳ *zOrigCol = 0;
#endif

  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି!=0 );
  ପ୍ରତିଜ୍ଞା( pNC->pSrcList!=0 );
  ବାଛ( p_ପଦସମଷ୍ଟି->op ){
    ସ୍ଥିତି TK_COLUMN: {
      /* The expression is a column. Locate the table the column is being
      ** extracted from in NameContext.pSrcList. This table may be real
      ** database table or a subquery.
      */
      Table *pTab = 0;            /* Table structure column is extracted from */
      Select *pS = 0;             /* Select the column is extracted from */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol = p_ପଦସମଷ୍ଟି->iColumn;  /* Index of column in pTab */
      ଯେତେବେଳେ( pNC && !pTab ){
        SrcList *pTabList = pNC->pSrcList;
        ପାଇଁ(j=0;j<pTabList->nSrc && pTabList->a[j].iCursor!=p_ପଦସମଷ୍ଟି->iTable;j++);
        ଯଦି( j<pTabList->nSrc ){
          pTab = pTabList->a[j].pSTab;
          ଯଦି( pTabList->a[j].fg.isSubquery ){
            pS = pTabList->a[j].u4.pSubq->pSelect;
          }ଅନ୍ୟଥା{
            pS = 0;
          }
        }ଅନ୍ୟଥା{
          pNC = pNC->pNext;
        }
      }

      ଯଦି( pTab==0 ){
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
        ଭାଙ୍ଗ;
      }

      ପ୍ରତିଜ୍ଞା( pTab && ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) && p_ପଦସମଷ୍ଟି->y.pTab==pTab );
      ଯଦି( pS ){
        /* The "table" is actually a sub-select or a view in the FROM clause
        ** of the SELECT statement. Return the declaration type and origin
        ** data for the result-set column of the sub-select.
        */
        ଯଦି( iCol<pS->pEList->n_ପଦସମଷ୍ଟି
         && (!ViewCanHaveRowid || iCol>=0)
        ){
          /* If iCol is less than zero, then the expression requests the
          ** rowid of the sub-select or view. This expression is legal (see
          ** test case misc2.2.2) - it always evaluates to NULL.
          */
          NameContext sNC;
          ପଦସମଷ୍ଟି *p = pS->pEList->a[iCol].p_ପଦସମଷ୍ଟି;
          sNC.pSrcList = pS->pSrc;
          sNC.pNext = pNC;
          sNC.pParse = pNC->pParse;
          zType = columnType(&sNC, p,&zOrigDb,&zOrigTab,&zOrigCol);
        }
      }ଅନ୍ୟଥା{
        /* A real table or a CTE table */
        ପ୍ରତିଜ୍ଞା( !pS );
#ifdef SQLITE_ENABLE_COLUMN_METADATA
        ଯଦି( iCol<0 ) iCol = pTab->iPKey;
        ପ୍ରତିଜ୍ଞା( iCol==XN_ROWID || (iCol>=0 && iCol<pTab->nCol) );
        ଯଦି( iCol<0 ){
          zType = "INTEGER";
          zOrigCol = "rowid";
        }ଅନ୍ୟଥା{
          zOrigCol = pTab->aCol[iCol].zCnName;
          zType = sqlite3ColumnType(&pTab->aCol[iCol],0);
        }
        zOrigTab = pTab->zName;
        ଯଦି( pNC->pParse && pTab->pSchema ){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = sqlite3SchemaToIndex(pNC->pParse->db, pTab->pSchema);
          zOrigDb = pNC->pParse->db->aDb[iDb].zDbSName;
        }
#ଅନ୍ୟଥା
        ପ୍ରତିଜ୍ଞା( iCol==XN_ROWID || (iCol>=0 && iCol<pTab->nCol) );
        ଯଦି( iCol<0 ){
          zType = "INTEGER";
        }ଅନ୍ୟଥା{
          zType = sqlite3ColumnType(&pTab->aCol[iCol],0);
        }
#endif
      }
      ଭାଙ୍ଗ;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ସ୍ଥିତି TK_SELECT: {
      /* The expression is a sub-select. Return the declaration type and
      ** origin info for the single column in the result set of the SELECT
      ** statement.
      */
      NameContext sNC;
      Select *pS;
      ପଦସମଷ୍ଟି *p;
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) );
      pS = p_ପଦସମଷ୍ଟି->x.pSelect;
      p = pS->pEList->a[0].p_ପଦସମଷ୍ଟି;
      sNC.pSrcList = pS->pSrc;
      sNC.pNext = pNC;
      sNC.pParse = pNC->pParse;
      zType = columnType(&sNC, p, &zOrigDb, &zOrigTab, &zOrigCol);
      ଭାଙ୍ଗ;
    }
#endif
  }

#ifdef SQLITE_ENABLE_COLUMN_METADATA 
  ଯଦି( pzOrigDb ){
    ପ୍ରତିଜ୍ଞା( pzOrigTab && pzOrigCol );
    *pzOrigDb = zOrigDb;
    *pzOrigTab = zOrigTab;
    *pzOrigCol = zOrigCol;
  }
#endif
  ଫେରନ୍ତୁ zType;
}

/*
** Generate code that will tell the VDBE the declaration types of columns
** in the result set.
*/
ସ୍ଥିର ଶୂନ୍ୟ generateColumnTypes(
  Parse *pParse,      /* Parser context */
  SrcList *pTabList,  /* List of tables */
  ପଦସମଷ୍ଟି_List *pEList    /* Expressions defining the result set */
){
#ifndef SQLITE_OMIT_DECLTYPE
  Vdbe *v = pParse->pVdbe;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  NameContext sNC;
  sNC.pSrcList = pTabList;
  sNC.pParse = pParse;
  sNC.pNext = 0;
  ପାଇଁ(i=0; i<pEList->n_ପଦସମଷ୍ଟି; i++){
    ପଦସମଷ୍ଟି *p = pEList->a[i].p_ପଦସମଷ୍ଟି;
    ଅଚଳ ଅକ୍ଷର *zType;
#ifdef SQLITE_ENABLE_COLUMN_METADATA
    ଅଚଳ ଅକ୍ଷର *zOrigDb = 0;
    ଅଚଳ ଅକ୍ଷର *zOrigTab = 0;
    ଅଚଳ ଅକ୍ଷର *zOrigCol = 0;
    zType = columnType(&sNC, p, &zOrigDb, &zOrigTab, &zOrigCol);

    /* The vdbe must make its own copy of the column-type and other
    ** column specific strings, in case the schema is reset before this
    ** virtual machine is deleted.
    */
    sqlite3VdbeSetColName(v, i, COLNAME_DATABASE, zOrigDb, SQLITE_TRANSIENT);
    sqlite3VdbeSetColName(v, i, COLNAME_TABLE, zOrigTab, SQLITE_TRANSIENT);
    sqlite3VdbeSetColName(v, i, COLNAME_COLUMN, zOrigCol, SQLITE_TRANSIENT);
#ଅନ୍ୟଥା
    zType = columnType(&sNC, p, 0, 0, 0);
#endif
    sqlite3VdbeSetColName(v, i, COLNAME_DECLTYPE, zType, SQLITE_TRANSIENT);
  }
#ଅନ୍ୟଥା
  UNUSED_PARAMETER(pParse);
  UNUSED_PARAMETER(pTabList);
  UNUSED_PARAMETER(pEList);
#endif /* !defined(SQLITE_OMIT_DECLTYPE) */
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
ଶୂନ୍ୟ sqlite3GenerateColumnNames(
  Parse *pParse,      /* Parser context */
  Select *pSelect     /* Generate column names for this SELECT statement */
){
  Vdbe *v = pParse->pVdbe;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  Table *pTab;
  SrcList *pTabList;
  ପଦସମଷ୍ଟି_List *pEList;
  sqlite3 *db = pParse->db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fullName;    /* TABLE.COLUMN if no AS clause and is a direct table ref */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା srcName;     /* COLUMN or TABLE.COLUMN if no AS clause and is direct */

  ଯଦି( pParse->colNamesSet ) ଫେରନ୍ତୁ;
  /* Column names are determined by the left-most term of a compound select */
  ଯେତେବେଳେ( pSelect->pPrior ) pSelect = pSelect->pPrior;
  TREETRACE(0x80,pParse,pSelect,("generating column names\n"));
  pTabList = pSelect->pSrc;
  pEList = pSelect->pEList;
  ପ୍ରତିଜ୍ଞା( v!=0 );
  ପ୍ରତିଜ୍ଞା( pTabList!=0 );
  pParse->colNamesSet = 1;
  fullName = (db->flags & SQLITE_FullColNames)!=0;
  srcName = (db->flags & SQLITE_ShortColNames)!=0 || fullName;
  sqlite3_Vdbe_Set_ଅଙ୍କ_Cols(v, pEList->n_ପଦସମଷ୍ଟି);
  ପାଇଁ(i=0; i<pEList->n_ପଦସମଷ୍ଟି; i++){
    ପଦସମଷ୍ଟି *p = pEList->a[i].p_ପଦସମଷ୍ଟି;

    ପ୍ରତିଜ୍ଞା( p!=0 );
    ପ୍ରତିଜ୍ଞା( p->op!=TK_AGG_COLUMN );  /* Agg processing has not run yet */
    ପ୍ରତିଜ୍ଞା( p->op!=TK_COLUMN
        || (ପଦସମଷ୍ଟି_Use_YTab(p) && p->y.pTab!=0) ); /* Covering idx not yet coded */
    ଯଦି( pEList->a[i].zEName && pEList->a[i].fg.eEName==ENAME_NAME ){
      /* An AS clause always takes first priority */
      ଅକ୍ଷର *zName = pEList->a[i].zEName;
      sqlite3VdbeSetColName(v, i, COLNAME_NAME, zName, SQLITE_TRANSIENT);
    }ଅନ୍ୟଥା ଯଦି( srcName && p->op==TK_COLUMN ){
      ଅକ୍ଷର *zCol;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol = p->iColumn;
      pTab = p->y.pTab;
      ପ୍ରତିଜ୍ଞା( pTab!=0 );
      ଯଦି( iCol<0 ) iCol = pTab->iPKey;
      ପ୍ରତିଜ୍ଞା( iCol==-1 || (iCol>=0 && iCol<pTab->nCol) );
      ଯଦି( iCol<0 ){
        zCol = "rowid";
      }ଅନ୍ୟଥା{
        zCol = pTab->aCol[iCol].zCnName;
      }
      ଯଦି( fullName ){
        ଅକ୍ଷର *zName = 0;
        zName = sqlite3MPrintf(db, "%s.%s", pTab->zName, zCol);
        sqlite3VdbeSetColName(v, i, COLNAME_NAME, zName, SQLITE_DYNAMIC);
      }ଅନ୍ୟଥା{
        sqlite3VdbeSetColName(v, i, COLNAME_NAME, zCol, SQLITE_TRANSIENT);
      }
    }ଅନ୍ୟଥା{
      ଅଚଳ ଅକ୍ଷର *z = pEList->a[i].zEName;
      z = z==0 ? sqlite3MPrintf(db, "column%d", i+1) : sqlite3DbStrDup(db, z);
      sqlite3VdbeSetColName(v, i, COLNAME_NAME, z, SQLITE_DYNAMIC);
    }
  }
  generateColumnTypes(pParse, pTabList, pEList);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_Columns_From_ପଦସମଷ୍ଟି_List(
  Parse *pParse,          /* Parsing context */
  ପଦସମଷ୍ଟି_List *pEList,       /* Expr list from which to derive column names */
  i16 *pnCol,             /* Write the number of columns here */
  Column **paCol          /* Write the new column list here */
){
  sqlite3 *db = pParse->db;   /* Database connection */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;                   /* Loop counters */
  u32 cnt;                    /* Index added to make the name unique */
  Column *aCol, *pCol;        /* For looping over result columns */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol;                   /* Number of columns in the result set */
  ଅକ୍ଷର *zName;                /* Column name */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nName;                  /* Size of name in zName[] */
  Hash ht;                    /* Hash table of column names */
  Table *pTab;

  sqlite3HashInit(&ht);
  ଯଦି( pEList ){
    nCol = pEList->n_ପଦସମଷ୍ଟି;
    aCol = sqlite3DbMallocZero(db, ଆକାର(aCol[0])*nCol);
    testcase( aCol==0 );
    ଯଦି( nCol>32767 ) nCol = 32767;
  }ଅନ୍ୟଥା{
    nCol = 0;
    aCol = 0;
  }
  ପ୍ରତିଜ୍ଞା( nCol==(i16)nCol );
  *pnCol = nCol;
  *paCol = aCol;

  ପାଇଁ(i=0, pCol=aCol; i<nCol && !pParse->nErr; i++, pCol++){
    ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pX = &pEList->a[i];
    ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pCollide;
    /* Get an appropriate name for the column
    */
    ଯଦି( (zName = pX->zEName)!=0 && pX->fg.eEName==ENAME_NAME ){
      /* If the column contains an "AS <name>" phrase, use <name> as the name */
    }ଅନ୍ୟଥା{
      ପଦସମଷ୍ଟି *p_Col_ପଦସମଷ୍ଟି = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(pX->p_ପଦସମଷ୍ଟି);
      ଯେତେବେଳେ( ALWAYS(p_Col_ପଦସମଷ୍ଟି!=0) && p_Col_ପଦସମଷ୍ଟି->op==TK_DOT ){
        p_Col_ପଦସମଷ୍ଟି = p_Col_ପଦସମଷ୍ଟି->p_ଡାହାଣ;
        ପ୍ରତିଜ୍ଞା( p_Col_ପଦସମଷ୍ଟି!=0 );
      }
      ଯଦି( p_Col_ପଦସମଷ୍ଟି->op==TK_COLUMN
       && ALWAYS( ପଦସମଷ୍ଟି_Use_YTab(p_Col_ପଦସମଷ୍ଟି) )
       && ALWAYS( p_Col_ପଦସମଷ୍ଟି->y.pTab!=0 )
      ){
        /* For columns use the column name name */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol = p_Col_ପଦସମଷ୍ଟି->iColumn;
        pTab = p_Col_ପଦସମଷ୍ଟି->y.pTab;
        ଯଦି( iCol<0 ) iCol = pTab->iPKey;
        zName = iCol>=0 ? pTab->aCol[iCol].zCnName : "rowid";
      }ଅନ୍ୟଥା ଯଦି( p_Col_ପଦସମଷ୍ଟି->op==TK_ID ){
        ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_Col_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
        zName = p_Col_ପଦସମଷ୍ଟି->u.zToken;
      }ଅନ୍ୟଥା{
        /* Use the original text of the column expression as its name */
        ପ୍ରତିଜ୍ଞା( zName==pX->zEName );  /* pointer comparison intended */
      }
    }
    ଯଦି( zName && !sqlite3IsTrueOrFalse(zName) ){
      zName = sqlite3DbStrDup(db, zName);
    }ଅନ୍ୟଥା{
      zName = sqlite3MPrintf(db,"column%d",i+1);
    }

    /* Make sure the column name is unique.  If the name is not unique,
    ** append an integer to the name so that it becomes unique.
    */
    cnt = 0;
    ଯେତେବେଳେ( zName && (pCollide = sqlite3HashFind(&ht, zName))!=0 ){
      ଯଦି( pCollide->fg.bUsingTerm ){
        pCol->colFlags |= COLFLAG_NOEXPAND;
      }
      nName = sqlite3Strlen30(zName);
      ଯଦି( nName>0 ){
        ପାଇଁ(j=nName-1; j>0 && sqlite3Isdigit(zName[j]); j--){}
        ଯଦି( zName[j]==':' ) nName = j;
      }
      zName = sqlite3MPrintf(db, "%.*z:%u", nName, zName, ++cnt);
      sqlite3ProgressCheck(pParse);
      ଯଦି( cnt>3 ){
        sqlite3_randomness(ଆକାର(cnt), &cnt);
      }
    }
    pCol->zCnName = zName;
    pCol->hName = sqlite3StrIHash(zName);
    ଯଦି( pX->fg.bNoExpand ){
      pCol->colFlags |= COLFLAG_NOEXPAND;
    }
    sqlite3ColumnPropertiesFromName(0, pCol);
    ଯଦି( zName && sqlite3_Hash_ଭର୍ତ୍ତି(&ht, zName, pX)==pX ){
      sqlite3OomFault(db);
    }
  }
  sqlite3HashClear(&ht);
  ଯଦି( pParse->nErr ){
    ପାଇଁ(j=0; j<i; j++){
      sqlite3DbFree(db, aCol[j].zCnName);
    }
    sqlite3DbFree(db, aCol);
    *paCol = 0;
    *pnCol = 0;
    ଫେରନ୍ତୁ pParse->rc;
  }
  ଫେରନ୍ତୁ SQLITE_OK;
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
ଶୂନ୍ୟ sqlite3SubqueryColumnTypes(
  Parse *pParse,      /* Parsing contexts */
  Table *pTab,        /* Add column type information to this table */
  Select *pSelect,    /* SELECT used to determine types and collations */
  ଅକ୍ଷର aff            /* Default affinity. */
){
  sqlite3 *db = pParse->db;
  Column *pCol;
  CollSeq *pColl;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i,j;
  ପଦସମଷ୍ଟି *p;
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *a;
  NameContext sNC;

  ପ୍ରତିଜ୍ଞା( pSelect!=0 );
  ପ୍ରତିଜ୍ଞା( (pSelect->selFlags & SF_Resolved)!=0 );
  ପ୍ରତିଜ୍ଞା( pTab->nCol==pSelect->pEList->n_ପଦସମଷ୍ଟି || pParse->nErr>0 );
  ପ୍ରତିଜ୍ଞା( aff==SQLITE_AFF_NONE || aff==SQLITE_AFF_BLOB );
  ଯଦି( db->mallocFailed || IN_RENAME_OBJECT ) ଫେରନ୍ତୁ;
  ଯେତେବେଳେ( pSelect->pPrior ) pSelect = pSelect->pPrior;
  a = pSelect->pEList->a;
  memset(&sNC, 0, ଆକାର(sNC));
  sNC.pSrcList = pSelect->pSrc;
  ପାଇଁ(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
    ଅଚଳ ଅକ୍ଷର *zType;
    i64 n;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା m = 0;
    Select *pS2 = pSelect;
    pTab->tabFlags |= (pCol->colFlags & COLFLAG_NOINSERT);
    p = a[i].p_ପଦସମଷ୍ଟି;
    /* pCol->szEst = ... // Column size est for SELECT tables never used */
    pCol->affinity = sqlite3_ପଦସମଷ୍ଟି_Affinity(p);
    ଯେତେବେଳେ( pCol->affinity<=SQLITE_AFF_NONE && pS2->pNext!=0 ){
      m |= sqlite3_ପଦସମଷ୍ଟି_Data_Type(pS2->pEList->a[i].p_ପଦସମଷ୍ଟି);
      pS2 = pS2->pNext;
      pCol->affinity = sqlite3_ପଦସମଷ୍ଟି_Affinity(pS2->pEList->a[i].p_ପଦସମଷ୍ଟି);
    }
    ଯଦି( pCol->affinity<=SQLITE_AFF_NONE ){
      pCol->affinity = aff;
    }
    ଯଦି( pCol->affinity>=SQLITE_AFF_TEXT && (pS2->pNext || pS2!=pSelect) ){
      ପାଇଁ(pS2=pS2->pNext; pS2; pS2=pS2->pNext){
        m |= sqlite3_ପଦସମଷ୍ଟି_Data_Type(pS2->pEList->a[i].p_ପଦସମଷ୍ଟି);
      }
      ଯଦି( pCol->affinity==SQLITE_AFF_TEXT && (m&0x01)!=0 ){
        pCol->affinity = SQLITE_AFF_BLOB;
      }ଅନ୍ୟଥା
      ଯଦି( pCol->affinity>=SQLITE_AFF_NUMERIC && (m&0x02)!=0 ){
        pCol->affinity = SQLITE_AFF_BLOB;
      }
      ଯଦି( pCol->affinity>=SQLITE_AFF_NUMERIC && p->op==TK_CAST ){
        pCol->affinity = SQLITE_AFF_FLEXNUM;
      }
    }
    zType = columnType(&sNC, p, 0, 0, 0);
    ଯଦି( zType==0 || pCol->affinity!=sqlite3AffinityType(zType, 0) ){
      ଯଦି( pCol->affinity==SQLITE_AFF_NUMERIC
       || pCol->affinity==SQLITE_AFF_FLEXNUM
      ){
        zType = "NUM";
      }ଅନ୍ୟଥା{
        zType = 0;
        ପାଇଁ(j=1; j<SQLITE_N_STDTYPE; j++){
          ଯଦି( sqlite3StdTypeAffinity[j]==pCol->affinity ){
            zType = sqlite3StdType[j];
            ଭାଙ୍ଗ;
          }
        }
      }
    }
    ଯଦି( zType ){
      ଅଚଳ i64 k = strlen(zType);
      n = strlen(pCol->zCnName);
      pCol->zCnName = sqlite3DbReallocOrFree(db, pCol->zCnName, n+k+2);
      pCol->colFlags &= ~(COLFLAG_HASTYPE|COLFLAG_HASCOLL);
      ଯଦି( pCol->zCnName ){
        memcpy(&pCol->zCnName[n+1], zType, k+1);
        pCol->colFlags |= COLFLAG_HASTYPE;
      }
    }
    pColl = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, p);
    ଯଦି( pColl ){
      ପ୍ରତିଜ୍ଞା( pTab->pIndex==0 );
      sqlite3ColumnSetColl(db, pCol, pColl->zName);
    }
  }
  pTab->szTabRow = 1; /* Any non-zero value works */
}

/*
** Check the current subquery nesting depth.  Return true and set an
** error if it has gone too deep.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା checkSubqueryNestingDepth(Parse *pParse){
#ଯଦି SQLITE_MAX_ପଦସମଷ୍ଟି_DEPTH>0
  ଯଦି( pParse->nNestSel >= pParse->db->aLimit[SQLITE_LIMIT_ପଦସମଷ୍ଟି_DEPTH] ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "VIEWs and/or subqueries nested too deep");
    ଫେରନ୍ତୁ 1;
  }
#endif
  ଫେରନ୍ତୁ 0;
}

/*
** Given a SELECT statement, generate a Table structure that describes
** the result set of that SELECT.
*/
Table *sqlite3ResultSetOfSelect(Parse *pParse, Select *pSelect, ଅକ୍ଷର aff){
  Table *pTab;
  sqlite3 *db = pParse->db;
  u64 savedFlags;

  pParse->nNestSel++;
  ଯଦି( checkSubqueryNestingDepth(pParse) ) ଫେରନ୍ତୁ 0;
  savedFlags = db->flags;
  db->flags &= ~(u64)SQLITE_FullColNames;
  db->flags |= SQLITE_ShortColNames;
  sqlite3SelectPrep(pParse, pSelect, 0);
  db->flags = savedFlags;
  ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ 0;
  ଯେତେବେଳେ( pSelect->pPrior ) pSelect = pSelect->pPrior;
  pTab = sqlite3DbMallocZero(db, ଆକାର(Table) );
  ଯଦି( pTab==0 ){
    ଫେରନ୍ତୁ 0;
  }
  pTab->nTabRef = 1;
  pTab->zName = 0;
  pTab->nRowLogEst = 200; ପ୍ରତିଜ୍ଞା( 200==sqlite3LogEst(1048576) );
  sqlite3_Columns_From_ପଦସମଷ୍ଟି_List(pParse, pSelect->pEList, &pTab->nCol, &pTab->aCol);
  sqlite3SubqueryColumnTypes(pParse, pTab, pSelect, aff);
  pTab->iPKey = -1;
  ଯଦି( db->mallocFailed ){
    sqlite3DeleteTable(db, pTab);
    ଫେରନ୍ତୁ 0;
  }
  pParse->nNestSel--;
  ପ୍ରତିଜ୍ଞା( pParse->nNestSel>=0 );
  ଫେରନ୍ତୁ pTab;
}

/*
** Get a VDBE for the given parser context.  Create a new one if necessary.
** If an error occurs, return NULL and leave a message in pParse.
*/
Vdbe *sqlite3GetVdbe(Parse *pParse){
  ଯଦି( pParse->pVdbe ){
    ଫେରନ୍ତୁ pParse->pVdbe;
  }
  ଯଦି( pParse->pToplevel==0 ){
    pParse->okConstFactor = 1;
  }
  ଫେରନ୍ତୁ sqlite3VdbeCreate(pParse);
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
ସ୍ଥିର ଶୂନ୍ୟ computeLimitRegisters(Parse *pParse, Select *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iBreak){
  Vdbe *v = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iLimit = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOffset;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
  ପଦସମଷ୍ଟି *pLimit = p->pLimit;

  ଯଦି( p->iLimit ) ଫେରନ୍ତୁ;

  /*
  ** "LIMIT -1" always shows all rows.  There is some
  ** controversy about what the correct behavior should be.
  ** The current implementation interprets "LIMIT 0" to mean
  ** no rows.
  */
  ଯଦି( pLimit ){
    ପ୍ରତିଜ୍ଞା( pLimit->op==TK_LIMIT );
    ପ୍ରତିଜ୍ଞା( pLimit->p_ବାମ!=0 );
    p->iLimit = iLimit = ++pParse->nMem;
    v = sqlite3GetVdbe(pParse);
    ପ୍ରତିଜ୍ଞା( v!=0 );
    ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Is_Integer(pLimit->p_ବାମ, &n, pParse, 0) ){
      sqlite3VdbeAddOp2(v, OP_Integer, n, iLimit);
      VdbeComment((v, "LIMIT counter"));
      ଯଦି( n==0 ){
        sqlite3VdbeGoto(v, iBreak);
      }ଅନ୍ୟଥା ଯଦି( n>=0 && p->nSelectRow>sqlite3LogEst((u64)n) ){
        p->nSelectRow = sqlite3LogEst((u64)n);
        p->selFlags |= SF_FixedLimit;
      }
    }ଅନ୍ୟଥା{
      sqlite3_ପଦସମଷ୍ଟି_Code(pParse, pLimit->p_ବାମ, iLimit);
      sqlite3VdbeAddOp1(v, OP_MustBeInt, iLimit); VdbeCoverage(v);
      VdbeComment((v, "LIMIT counter"));
      sqlite3MayAbort(pParse);
      sqlite3VdbeAddOp2(v, OP_IfNot, iLimit, iBreak); VdbeCoverage(v);
    }
    ଯଦି( pLimit->p_ଡାହାଣ ){
      p->iOffset = iOffset = ++pParse->nMem;
      pParse->nMem++;   /* Allocate an extra register for limit+offset */
      sqlite3_ପଦସମଷ୍ଟି_Code(pParse, pLimit->p_ଡାହାଣ, iOffset);
      sqlite3VdbeAddOp1(v, OP_MustBeInt, iOffset); VdbeCoverage(v);
      sqlite3MayAbort(pParse);
      VdbeComment((v, "OFFSET counter"));
      sqlite3VdbeAddOp3(v, OP_OffsetLimit, iLimit, iOffset+1, iOffset);
      VdbeComment((v, "LIMIT+OFFSET"));
    }
  }
}

#ifndef SQLITE_OMIT_COMPOUND_SELECT
/*
** Return the appropriate collating sequence for the iCol-th column of
** the result set for the compound-select statement "p".  Return NULL if
** the column has no default collating sequence.
**
** The collating sequence for the compound select is taken from the
** left-most term of the select that has a collating sequence.
*/
ସ୍ଥିର CollSeq *multiSelectCollSeq(Parse *pParse, Select *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol){
  CollSeq *pRet;
  ଯଦି( p->pPrior ){
    pRet = multiSelectCollSeq(pParse, p->pPrior, iCol);
  }ଅନ୍ୟଥା{
    pRet = 0;
  }
  ପ୍ରତିଜ୍ଞା( iCol>=0 );
  /* iCol must be less than p->pEList->nExpr.  Otherwise an error would
  ** have been thrown during name resolution and we would not have gotten
  ** this far */
  ଯଦି( pRet==0 && ALWAYS(iCol<p->pEList->n_ପଦସମଷ୍ଟି) ){
    pRet = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, p->pEList->a[iCol].p_ପଦସମଷ୍ଟି);
  }
  ଫେରନ୍ତୁ pRet;
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
ସ୍ଥିର ଚାବି_Info *multi_Select_By_Merge_ଚାବି_Info(Parse *pParse, Select *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nExtra){
  ପଦସମଷ୍ଟି_List *pOrderBy = p->pOrderBy;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOrderBy = (pOrderBy!=0) ? pOrderBy->n_ପଦସମଷ୍ଟି : 0;
  sqlite3 *db = pParse->db;
  ଚାବି_Info *pRet = sqlite3_ଚାବି_Info_Alloc(db, nOrderBy+nExtra, 1);
  ଯଦି( pRet ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<nOrderBy; i++){
      ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem = &pOrderBy->a[i];
      ପଦସମଷ୍ଟି *pTerm = pItem->p_ପଦସମଷ୍ଟି;
      CollSeq *pColl;

      ଯଦି( pTerm->flags & EP_Collate ){
        pColl = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, pTerm);
      }ଅନ୍ୟଥା{
        pColl = multiSelectCollSeq(pParse, p, pItem->u.x.iOrderByCol-1);
        ଯଦି( pColl==0 ) pColl = db->pDfltColl;
        pOrderBy->a[i].p_ପଦସମଷ୍ଟି =
          sqlite3_ପଦସମଷ୍ଟି_Add_Collate_String(pParse, pTerm, pColl->zName);
      }
      ପ୍ରତିଜ୍ଞା( sqlite3_ଚାବି_Info_Is_Writeable(pRet) );
      pRet->aColl[i] = pColl;
      pRet->aSortFlags[i] = pOrderBy->a[i].fg.sortFlags;
    }
  }

  ଫେରନ୍ତୁ pRet;
}

#ifndef SQLITE_OMIT_CTE
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
ସ୍ଥିର ଶୂନ୍ୟ generateWithRecursiveQuery(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The recursive SELECT to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  SrcList *pSrc = p->pSrc;      /* The FROM clause of the recursive query */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol = p->pEList->n_ପଦସମଷ୍ଟି;  /* Number of columns in the recursive table */
  Vdbe *v = pParse->pVdbe;      /* The prepared statement under construction */
  Select *pSetup;               /* The setup query */
  Select *pFirstRec;            /* Left-most recursive term */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr_ଶୀର୍ଷ;                  /* Top of the loop */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrCont, addrBreak;      /* CONTINUE and BREAK addresses */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCurrent = 0;             /* The Current table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regCurrent;               /* Register holding Current table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iQueue;                   /* The Queue table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDistinct = 0;            /* To ensure unique results if UNION */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eDest = SRT_Fifo;         /* How to write to Queue */
  SelectDest destQueue;         /* SelectDest targeting the Queue table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                        /* Loop counter */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                       /* Result code */
  ପଦସମଷ୍ଟି_List *pOrderBy;           /* The ORDER BY clause */
  ପଦସମଷ୍ଟି *pLimit;                 /* Saved LIMIT and OFFSET */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regLimit, regOffset;      /* Registers used by LIMIT and OFFSET */

#ifndef SQLITE_OMIT_WINDOWFUNC
  ଯଦି( p->pWin ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "cannot use window functions in recursive queries");
    ଫେରନ୍ତୁ;
  }
#endif

  /* Obtain authorization to do a recursive query */
  ଯଦି( sqlite3AuthCheck(pParse, SQLITE_RECURSIVE, 0, 0, 0) ) ଫେରନ୍ତୁ;

  /* Process the LIMIT and OFFSET clauses, if they exist */
  addrBreak = sqlite3VdbeMakeLabel(pParse);
  p->nSelectRow = 320;  /* 4 billion rows */
  computeLimitRegisters(pParse, p, addrBreak);
  pLimit = p->pLimit;
  regLimit = p->iLimit;
  regOffset = p->iOffset;
  p->pLimit = 0;
  p->iLimit = p->iOffset = 0;
  pOrderBy = p->pOrderBy;

  /* Locate the cursor number of the Current table */
  ପାଇଁ(i=0; ALWAYS(i<pSrc->nSrc); i++){
    ଯଦି( pSrc->a[i].fg.isRecursive ){
      iCurrent = pSrc->a[i].iCursor;
      ଭାଙ୍ଗ;
    }
  }

  /* Allocate cursors numbers for Queue and Distinct.  The cursor number for
  ** the Distinct table must be exactly one greater than Queue in order
  ** for the SRT_DistFifo and SRT_DistQueue destinations to work. */
  iQueue = pParse->nTab++;
  ଯଦି( p->op==TK_UNION ){
    eDest = pOrderBy ? SRT_DistQueue : SRT_DistFifo;
    iDistinct = pParse->nTab++;
  }ଅନ୍ୟଥା{
    eDest = pOrderBy ? SRT_Queue : SRT_Fifo;
  }
  sqlite3SelectDestInit(&destQueue, eDest, iQueue);

  /* Allocate cursors for Current, Queue, and Distinct. */
  regCurrent = ++pParse->nMem;
  sqlite3VdbeAddOp3(v, OP_OpenPseudo, iCurrent, regCurrent, nCol);
  ଯଦି( pOrderBy ){
    ଚାବି_Info *p_ଚାବି_Info = multi_Select_By_Merge_ଚାବି_Info(pParse, p, 1);
    sqlite3VdbeAddOp4(v, OP_OpenEphemeral, iQueue, pOrderBy->n_ପଦସମଷ୍ଟି+2, 0,
                      (ଅକ୍ଷର*)p_ଚାବି_Info, P4_KEYINFO);
    destQueue.pOrderBy = pOrderBy;
  }ଅନ୍ୟଥା{
    sqlite3VdbeAddOp2(v, OP_OpenEphemeral, iQueue, nCol);
  }
  VdbeComment((v, "Queue table"));
  ଯଦି( iDistinct ){
    /* Generate an ephemeral table used to enforce distinctness on the
    ** output of the recursive part of the CTE.
    */
    ଚାବି_Info *p_ଚାବି_Info;            /* Collating sequence for the result set */
    CollSeq **apColl;             /* For looping through pKeyInfo->aColl[] */

    ପ୍ରତିଜ୍ଞା( p->pNext==0 );
    ପ୍ରତିଜ୍ଞା( p->pEList!=0 );
    nCol = p->pEList->n_ପଦସମଷ୍ଟି;
    p_ଚାବି_Info = sqlite3_ଚାବି_Info_Alloc(pParse->db, nCol, 1);
    ଯଦି( p_ଚାବି_Info ){
      ପାଇଁ(i=0, apColl=p_ଚାବି_Info->aColl; i<nCol; i++, apColl++){
        *apColl = multiSelectCollSeq(pParse, p, i);
        ଯଦି( 0==*apColl ){
          *apColl = pParse->db->pDfltColl;
        }
      }
      sqlite3VdbeAddOp4(v, OP_OpenEphemeral, iDistinct, nCol, 0,
                           (ଶୂନ୍ୟ*)p_ଚାବି_Info, P4_KEYINFO);
    }ଅନ୍ୟଥା{
      ପ୍ରତିଜ୍ଞା( pParse->nErr>0 );
    }
  }

  /* Detach the ORDER BY clause from the compound SELECT */
  p->pOrderBy = 0;

  /* Figure out how many elements of the compound SELECT are part of the
  ** recursive query.  Make sure no recursive elements use aggregate
  ** functions.  Mark the recursive elements as UNION ALL even if they
  ** are really UNION because the distinctness will be enforced by the
  ** iDistinct table.  pFirstRec is left pointing to the left-most
  ** recursive term of the CTE.
  */
  ପାଇଁ(pFirstRec=p; ALWAYS(pFirstRec!=0); pFirstRec=pFirstRec->pPrior){
    ଯଦି( pFirstRec->selFlags & SF_Aggregate ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "recursive aggregate queries not supported");
      ଯାଅ end_of_recursive_query;
    }
    pFirstRec->op = TK_ALL;
    ଯଦି( (pFirstRec->pPrior->selFlags & SF_Recursive)==0 ) ଭାଙ୍ଗ;
  }

  /* Store the results of the setup-query in Queue. */
  pSetup = pFirstRec->pPrior;
  pSetup->pNext = 0;
  ExplainQueryPlan((pParse, 1, "SETUP"));
  rc = sqlite3Select(pParse, pSetup, &destQueue);
  pSetup->pNext = p;
  ଯଦି( rc ) ଯାଅ end_of_recursive_query;

  /* Find the next row in the Queue and output that row */
  addr_ଶୀର୍ଷ = sqlite3VdbeAddOp2(v, OP_Rewind, iQueue, addrBreak); VdbeCoverage(v);

  /* Transfer the next row in Queue over to Current */
  sqlite3VdbeAddOp1(v, OP_NullRow, iCurrent); /* To reset column cache */
  ଯଦି( pOrderBy ){
    sqlite3VdbeAddOp3(v, OP_Column, iQueue, pOrderBy->n_ପଦସମଷ୍ଟି+1, regCurrent);
  }ଅନ୍ୟଥା{
    sqlite3VdbeAddOp2(v, OP_RowData, iQueue, regCurrent);
  }
  sqlite3VdbeAddOp1(v, OP_Delete, iQueue);

  /* Output the single row in Current */
  addrCont = sqlite3VdbeMakeLabel(pParse);
  codeOffset(v, regOffset, addrCont);
  selectInnerLoop(pParse, p, iCurrent,
      0, 0, pDest, addrCont, addrBreak);
  ଯଦି( regLimit ){
    sqlite3VdbeAddOp2(v, OP_DecrJumpZero, regLimit, addrBreak);
    VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, addrCont);

  /* Execute the recursive SELECT taking the single row in Current as
  ** the value for the recursive-table. Store the results in the Queue.
  */
  pFirstRec->pPrior = 0;
  ExplainQueryPlan((pParse, 1, "RECURSIVE STEP"));
  sqlite3Select(pParse, p, &destQueue);
  ପ୍ରତିଜ୍ଞା( pFirstRec->pPrior==0 );
  pFirstRec->pPrior = pSetup;

  /* Keep running the loop until the Queue is empty */
  sqlite3VdbeGoto(v, addr_ଶୀର୍ଷ);
  sqlite3VdbeResolveLabel(v, addrBreak);

end_of_recursive_query:
  sqlite3_ପଦସମଷ୍ଟି_List_Delete(pParse->db, p->pOrderBy);
  p->pOrderBy = pOrderBy;
  p->pLimit = pLimit;
  ଫେରନ୍ତୁ;
}
#endif /* SQLITE_OMIT_CTE */

/* Forward references */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiSelectByMerge(
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multi_Select_ମୂଲ୍ୟଗୁଡ଼ିକ(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRow = 1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bShowAll = p->pLimit==0;
  ପ୍ରତିଜ୍ଞା( p->selFlags & SF_Multi_ମୂଲ୍ୟ );
  କର{
    ପ୍ରତିଜ୍ଞା( p->selFlags & SF_ମୂଲ୍ୟଗୁଡ଼ିକ );
    ପ୍ରତିଜ୍ଞା( p->op==TK_ALL || (p->op==TK_SELECT && p->pPrior==0) );
    ପ୍ରତିଜ୍ଞା( p->pNext==0 || p->pEList->n_ପଦସମଷ୍ଟି==p->pNext->pEList->n_ପଦସମଷ୍ଟି );
#ifndef SQLITE_OMIT_WINDOWFUNC
    ଯଦି( p->pWin ) ଫେରନ୍ତୁ -1;
#endif
    ଯଦି( p->pPrior==0 ) ଭାଙ୍ଗ;
    ପ୍ରତିଜ୍ଞା( p->pPrior->pNext==p );
    p = p->pPrior;
    nRow += bShowAll;
  }ଯେତେବେଳେ(1);
  ExplainQueryPlan((pParse, 0, "SCAN %d CONSTANT ROW%s", nRow,
                    nRow==1 ? "" : "S"));
  ଯେତେବେଳେ( p ){
    selectInnerLoop(pParse, p, -1, 0, 0, pDest, 1, 1);
    ଯଦି( !bShowAll ) ଭାଙ୍ଗ;
    p->nSelectRow = nRow;
    p = p->pNext;
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Return true if the SELECT statement which is known to be the recursive
** part of a recursive CTE still has its anchor terms attached.  If the
** anchor terms have already been removed, then return false.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା hasAnchor(Select *p){
  ଯେତେବେଳେ( p && (p->selFlags & SF_Recursive)!=0 ){ p = p->pPrior; }
  ଫେରନ୍ତୁ p!=0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା unionWithLimitOne(sqlite3 *db, Select *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା v;
  ଯଦି( p->op!=TK_UNION ) ଫେରନ୍ତୁ 0;
  ଯଦି( p->pLimit==0 ) ଫେରନ୍ତୁ 0;
  ଯଦି( p->pLimit->p_ଡାହାଣ ) ଫେରନ୍ତୁ 0;  /* No OFFSET */
  v = 0;
  ପ୍ରତିଜ୍ଞା( p->pLimit->p_ବାମ!=0 );
  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Is_Integer(p->pLimit->p_ବାମ, &v, 0, 0)==0 ) ଫେରନ୍ତୁ 0;
  ଯଦି( v!=1 ) ଫେରନ୍ତୁ 0;
  ଯଦି( OptimizationDisabled(db, SQLITE_UnionLimit) ) ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ 1;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiSelect(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;   /* Success code from a subroutine */
  Select *pPrior;       /* Another SELECT immediately to our left */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest dest;      /* Alternative data destination */
  Select *pDelete = 0;  /* Chain of simple selects to delete */
  sqlite3 *db;          /* Database connection */

  /* Make sure there is no ORDER BY or LIMIT clause on prior SELECTs.  Only
  ** the last (right-most) SELECT in the series may have an ORDER BY or LIMIT.
  */
  ପ୍ରତିଜ୍ଞା( p && p->pPrior );  /* Calling function guarantees this much */
  ପ୍ରତିଜ୍ଞା( (p->selFlags & SF_Recursive)==0 || p->op==TK_ALL || p->op==TK_UNION );
  ପ୍ରତିଜ୍ଞା( p->selFlags & SF_Compound );
  db = pParse->db;
  pPrior = p->pPrior;
  dest = *pDest;
  ପ୍ରତିଜ୍ଞା( pPrior->pOrderBy==0 );
  ପ୍ରତିଜ୍ଞା( pPrior->pLimit==0 );

  v = sqlite3GetVdbe(pParse);
  ପ୍ରତିଜ୍ଞା( v!=0 );  /* The VDBE already created by calling function */

  /* Create the destination temporary table if necessary
  */
  ଯଦି( dest.eDest==SRT_EphemTab ){
    ପ୍ରତିଜ୍ଞା( p->pEList );
    sqlite3VdbeAddOp2(v, OP_OpenEphemeral, dest.iSDParm, p->pEList->n_ପଦସମଷ୍ଟି);
    dest.eDest = SRT_Table;
  }

  /* Special handling for a compound-select that originates as a VALUES clause.
  */
  ଯଦି( p->selFlags & SF_Multi_ମୂଲ୍ୟ ){
    rc = multi_Select_ମୂଲ୍ୟଗୁଡ଼ିକ(pParse, p, &dest);
    ଯଦି( rc>=0 ) ଯାଅ multi_select_end;
    rc = SQLITE_OK;
  }

  /* Make sure all SELECTs in the statement have the same number of elements
  ** in their result sets.
  */
  ପ୍ରତିଜ୍ଞା( p->pEList && pPrior->pEList );
  ପ୍ରତିଜ୍ଞା( p->pEList->n_ପଦସମଷ୍ଟି==pPrior->pEList->n_ପଦସମଷ୍ଟି );

#ifndef SQLITE_OMIT_CTE
  ଯଦି( (p->selFlags & SF_Recursive)!=0 && hasAnchor(p) ){
    generateWithRecursiveQuery(pParse, p, &dest);
  }ଅନ୍ୟଥା
#endif
  ଯଦି( p->pOrderBy ){
    /* If the compound has an ORDER BY clause, then always use the merge
    ** algorithm. */
    ଫେରନ୍ତୁ multiSelectByMerge(pParse, p, pDest);
  }ଅନ୍ୟଥା ଯଦି( p->op!=TK_ALL && !unionWithLimitOne(db,p) ){
    /* If the compound is EXCEPT, INTERSECT, or UNION (anything other than
    ** UNION ALL) then also always use the merge algorithm.  However, the
    ** multiSelectByMerge() routine requires that the compound have an
    ** ORDER BY clause, and it doesn't right now.  So invent one first. */
    ପଦସମଷ୍ଟି *pOne = sqlite3_ପଦସମଷ୍ଟି_Int32(db, 1);
    p->pOrderBy = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, 0, pOne);
    ଯଦି( pParse->nErr ) ଯାଅ multi_select_end;
    ପ୍ରତିଜ୍ଞା( p->pOrderBy!=0 );
    p->pOrderBy->a[0].u.x.iOrderByCol = 1;
    ଫେରନ୍ତୁ multiSelectByMerge(pParse, p, pDest);
  }ଅନ୍ୟଥା{
    /* For a UNION ALL compound without ORDER BY, or for a UNION with a 
    ** LIMIT of exactly 1 and no OFFSET and no ORDER BY, simply run the left
    ** query, then run the right query */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nLimit = 0;  /* Initialize to suppress harmless compiler warning */

#ifndef SQLITE_OMIT_EXPLAIN
    ଯଦି( pPrior->pPrior==0 ){
      ExplainQueryPlan((pParse, 1, "COMPOUND QUERY"));
      ExplainQueryPlan((pParse, 1, "LEFT-MOST SUBQUERY"));
    }
#endif
    ପ୍ରତିଜ୍ଞା( !pPrior->pLimit );
    pPrior->iLimit = p->iLimit;
    pPrior->iOffset = p->iOffset;
    pPrior->pLimit = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p->pLimit, 0);
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL left...\n"));
    rc = sqlite3Select(pParse, pPrior, &dest);
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, pPrior->pLimit);
    pPrior->pLimit = 0;
    ଯଦି( rc ){
      ଯାଅ multi_select_end;
    }
    p->pPrior = 0;
    p->iLimit = pPrior->iLimit;
    p->iOffset = pPrior->iOffset;
    ଯଦି( p->iLimit ){
      addr = sqlite3VdbeAddOp1(v, OP_IfNot, p->iLimit); VdbeCoverage(v);
      VdbeComment((v, "Jump ahead if LIMIT reached"));
      ଯଦି( p->iOffset ){
        sqlite3VdbeAddOp3(v, OP_OffsetLimit,
                          p->iLimit, p->iOffset+1, p->iOffset);
      }
    }
    ExplainQueryPlan((pParse, 1, "UNION ALL"));
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL right...\n"));
    rc = sqlite3Select(pParse, p, &dest);
    testcase( rc!=SQLITE_OK );
    pDelete = p->pPrior;
    p->pPrior = pPrior;
    p->nSelectRow = sqlite3LogEstAdd(p->nSelectRow, pPrior->nSelectRow);
    ଯଦି( p->pLimit
     && sqlite3_ପଦସମଷ୍ଟି_Is_Integer(p->pLimit->p_ବାମ, &nLimit, pParse, 0)
     && nLimit>0 && p->nSelectRow > sqlite3LogEst((u64)nLimit)
    ){
      p->nSelectRow = sqlite3LogEst((u64)nLimit);
    }
    ଯଦି( addr ){
      sqlite3VdbeJumpHere(v, addr);
    }
#ifndef SQLITE_OMIT_EXPLAIN
    ଯଦି( p->pNext==0 ){
      Explain_Query_Plan_କାଢ଼(pParse);
    }
#endif
  }

multi_select_end:
  pDest->iSdst = dest.iSdst;
  pDest->nSdst = dest.nSdst;
  pDest->iSDParm2 = dest.iSDParm2;
  ଯଦି( pDelete ){
    sqlite3ParserAddCleanup(pParse, sqlite3SelectDeleteGeneric, pDelete);
  }
  ଫେରନ୍ତୁ rc;
}
#endif /* SQLITE_OMIT_COMPOUND_SELECT */

/*
** Error message for when two or more terms of a compound select have different
** size result sets.
*/
ଶୂନ୍ୟ sqlite3_Select_Wrong_ଅଙ୍କ_Terms_Error(Parse *pParse, Select *p){
  ଯଦି( p->selFlags & SF_ମୂଲ୍ୟଗୁଡ଼ିକ ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "all VALUES must have the same number of terms");
  }ଅନ୍ୟଥା{
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "SELECTs to the left and right of %s"
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା generateOutputSubroutine(
  Parse *pParse,          /* Parsing context */
  Select *p,              /* The SELECT statement */
  SelectDest *pIn,        /* Coroutine supplying data */
  SelectDest *pDest,      /* Where to send the data */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regReturn,          /* The return address register */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regPrev,            /* Previous result register.  No uniqueness if 0 */
  ଚାବି_Info *p_ଚାବି_Info,      /* For comparing with previous entry */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iBreak              /* Jump here if we hit the LIMIT */
){
  Vdbe *v = pParse->pVdbe;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iContinue;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr;

  ପ୍ରତିଜ୍ଞା( pIn->eDest==SRT_Coroutine );

  addr = sqlite3VdbeCurrentAddr(v);
  iContinue = sqlite3VdbeMakeLabel(pParse);

  /* Suppress duplicates for UNION, EXCEPT, and INTERSECT
  */
  ଯଦି( regPrev ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr1, addr2;
    addr1 = sqlite3VdbeAddOp1(v, OP_IfNot, regPrev); VdbeCoverage(v);
    addr2 = sqlite3VdbeAddOp4(v, OP_Compare, pIn->iSdst, regPrev+1, pIn->nSdst,
                              (ଅକ୍ଷର*)sqlite3_ଚାବି_Info_Ref(p_ଚାବି_Info), P4_KEYINFO);
    sqlite3VdbeAddOp3(v, OP_Jump, addr2+2, iContinue, addr2+2); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr1);
    sqlite3VdbeAddOp3(v, OP_Copy, pIn->iSdst, regPrev+1, pIn->nSdst-1);
    sqlite3VdbeAddOp2(v, OP_Integer, 1, regPrev);
  }
  ଯଦି( pParse->db->mallocFailed ) ଫେରନ୍ତୁ 0;

  /* Suppress the first OFFSET entries if there is an OFFSET clause
  */
  codeOffset(v, p->iOffset, iContinue);

  ବାଛ( pDest->eDest ){
    /* Store the result as data using a unique key.
    */
    ସ୍ଥିତି SRT_Fifo:
    ସ୍ଥିତି SRT_DistFifo:
    ସ୍ଥିତି SRT_Table:
    ସ୍ଥିତି SRT_EphemTab: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1 = sqlite3GetTempReg(pParse);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା r2 = sqlite3GetTempReg(pParse);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iParm = pDest->iSDParm;
      testcase( pDest->eDest==SRT_Table );
      testcase( pDest->eDest==SRT_EphemTab );
      testcase( pDest->eDest==SRT_Fifo );
      testcase( pDest->eDest==SRT_DistFifo );
      sqlite3VdbeAddOp3(v, OP_MakeRecord, pIn->iSdst, pIn->nSdst, r1);
#ଯଦି !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      ଯଦି( pDest->eDest==SRT_Table && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#endif
#ifndef SQLITE_OMIT_CTE
      ଯଦି( pDest->eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index that is used to enforce uniqueness on the
        ** total result.  At this point, we are processing the setup portion
        ** of the recursive CTE using the merge algorithm, so the results are
        ** guaranteed to be unique anyhow.  But we still need to populate the
        ** (iParm+1) cursor for use by the subsequent recursive phase.
        */
        sqlite3VdbeAddOp4Int(v, OP_Idx_ଭର୍ତ୍ତି, iParm+1, r1,
                             pIn->iSdst, pIn->nSdst);
      }
#endif
      sqlite3VdbeAddOp2(v, OP_NewRowid, iParm, r2);
      sqlite3VdbeAddOp3(v, OP_ଭର୍ତ୍ତି, iParm, r1, r2);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      sqlite3ReleaseTempReg(pParse, r2);
      sqlite3ReleaseTempReg(pParse, r1);
      ଭାଙ୍ଗ;
    }

    /* If any row exist in the result set, record that fact and abort.
    */
    ସ୍ଥିତି SRT_Exists: {
      sqlite3VdbeAddOp2(v, OP_Integer, 1, pDest->iSDParm);
      /* The LIMIT clause will terminate the loop for us */
      ଭାଙ୍ଗ;
    }

#ifndef SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)".
    */
    ସ୍ଥିତି SRT_Set: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1;
      testcase( pIn->nSdst>1 );
      r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp4(v, OP_MakeRecord, pIn->iSdst, pIn->nSdst,
          r1, pDest->zAffSdst, pIn->nSdst);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ଭର୍ତ୍ତି, pDest->iSDParm, r1,
                           pIn->iSdst, pIn->nSdst);
      ଯଦି( pDest->iSDParm2>0 ){
        ପ୍ରତିଜ୍ଞା( sqlite3CursorBloomable(pParse, pDest->iSDParm, pIn->nSdst) );
        sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pDest->iSDParm2, 0,
                             pIn->iSdst, pIn->nSdst);
        ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
      }
      sqlite3ReleaseTempReg(pParse, r1);
      ଭାଙ୍ଗ;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell and break out
    ** of the scan loop.  Note that the select might return multiple columns
    ** if it is the RHS of a row-value IN operator.
    */
    ସ୍ଥିତି SRT_Mem: {
      testcase( pIn->nSdst>1 );
      sqlite3_ପଦସମଷ୍ଟି_Code_Move(pParse, pIn->iSdst, pDest->iSDParm, pIn->nSdst);
      /* The LIMIT clause will jump out of the loop for us */
      ଭାଙ୍ଗ;
    }
#endif /* #ifndef SQLITE_OMIT_SUBQUERY */

    /* The results are stored in a sequence of registers
    ** starting at pDest->iSdst.  Then the co-routine yields.
    */
    ସ୍ଥିତି SRT_Coroutine: {
      ଯଦି( pDest->iSdst==0 ){
        pDest->iSdst = sqlite3GetTempRange(pParse, pIn->nSdst);
        pDest->nSdst = pIn->nSdst;
      }
      sqlite3_ପଦସମଷ୍ଟି_Code_Move(pParse, pIn->iSdst, pDest->iSdst, pIn->nSdst);
      sqlite3VdbeAddOp1(v, OP_Yield, pDest->iSDParm);
      ଭାଙ୍ଗ;
    }

#ifndef SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    ସ୍ଥିତି SRT_DistQueue:
    ସ୍ଥିତି SRT_Queue: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଚାବି;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା r1, r2, r3, ii;
      ପଦସମଷ୍ଟି_List *pSO;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iParm = pDest->iSDParm;
      pSO = pDest->pOrderBy;
      ପ୍ରତିଜ୍ଞା( pSO );
      n_ଚାବି = pSO->n_ପଦସମଷ୍ଟି;
      r1 = sqlite3GetTempReg(pParse);
      r2 = sqlite3GetTempRange(pParse, n_ଚାବି+2);
      r3 = r2+n_ଚାବି+1;

      sqlite3VdbeAddOp3(v, OP_MakeRecord, pIn->iSdst, pIn->nSdst, r3);
      ଯଦି( pDest->eDest==SRT_DistQueue ){
        sqlite3VdbeAddOp2(v, OP_Idx_ଭର୍ତ୍ତି, iParm+1, r3);
      }
      ପାଇଁ(ii=0; ii<n_ଚାବି; ii++){
        sqlite3VdbeAddOp2(v, OP_SCopy,
                          pIn->iSdst + pSO->a[ii].u.x.iOrderByCol - 1,
                          r2+ii);
      }
      sqlite3VdbeAddOp2(v, OP_Sequence, iParm, r2+n_ଚାବି);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, r2, n_ଚାବି+2, r1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ଭର୍ତ୍ତି, iParm, r1, r2, n_ଚାବି+2);
      sqlite3ReleaseTempReg(pParse, r1);
      sqlite3ReleaseTempRange(pParse, r2, n_ଚାବି+2);
      ଭାଙ୍ଗ;
    }
#endif /* SQLITE_OMIT_CTE */

    /* Ignore the output */
    ସ୍ଥିତି SRT_Discard: {
      ଭାଙ୍ଗ;
    }

    /* If none of the above, then the result destination must be
    ** SRT_Output.
    **
    ** For SRT_Output, results are stored in a sequence of registers. 
    ** Then the OP_ResultRow opcode is used to cause sqlite3_step() to
    ** return the next row of result.
    */
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      ପ୍ରତିଜ୍ଞା( pDest->eDest==SRT_Output );
      sqlite3VdbeAddOp2(v, OP_ResultRow, pIn->iSdst, pIn->nSdst);
      ଭାଙ୍ଗ;
    }
  }

  /* Jump to the end of the loop if the LIMIT is reached.
  */
  ଯଦି( p->iLimit ){
    sqlite3VdbeAddOp2(v, OP_DecrJumpZero, p->iLimit, iBreak); VdbeCoverage(v);
  }

  /* Generate the subroutine return
  */
  sqlite3VdbeResolveLabel(v, iContinue);
  sqlite3VdbeAddOp1(v, OP_Return, regReturn);

  ଫେରନ୍ତୁ addr;
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
#ifndef SQLITE_OMIT_COMPOUND_SELECT
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା multiSelectByMerge(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;             /* Loop counters */
  Select *pPrior;       /* Another SELECT immediately to our left */
  Select *pSplit;       /* Left-most SELECT in the right-hand group */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nSelect;          /* Number of SELECT statements in the compound */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest destA;     /* Destination for coroutine A */
  SelectDest destB;     /* Destination for coroutine B */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regAddrA;         /* Address register for select-A coroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regAddrB;         /* Address register for select-B coroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrSelectA;      /* Address of the select-A coroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrSelectB;      /* Address of the select-B coroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regOutA;          /* Address register for the output-A subroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regOutB;          /* Address register for the output-B subroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrOutA;         /* Address of the output-A subroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrOutB = 0;     /* Address of the output-B subroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrEofA;         /* Address of the select-A-exhausted subroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrEofA_noB;     /* Alternate addrEofA if B is uninitialized */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrEofB;         /* Address of the select-B-exhausted subroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrAltB;         /* Address of the A<B subroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrAeqB;         /* Address of the A==B subroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrAgtB;         /* Address of the A>B subroutine */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regLimitA;        /* Limit register for select-A */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regLimitB;        /* Limit register for select-A */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regPrev;          /* A range of registers to hold previous output */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା savedLimit;       /* Saved value of p->iLimit */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା savedOffset;      /* Saved value of p->iOffset */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା labelCmpr;        /* Label for the start of the merge algorithm */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା labelEnd;         /* Label for the end of the overall SELECT stmt */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr1;            /* Jump instructions that get retargeted */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op;               /* One of TK_ALL, TK_UNION, TK_EXCEPT, TK_INTERSECT */
  ଚାବି_Info *p_ଚାବି_Dup = 0; /* Comparison information for duplicate removal */
  ଚାବି_Info *p_ଚାବି_Merge;   /* Comparison information for merging rows */
  sqlite3 *db;          /* Database connection */
  ପଦସମଷ୍ଟି_List *pOrderBy;   /* The ORDER BY clause */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOrderBy;         /* Number of terms in the ORDER BY clause */
  u32 *aPermute;        /* Mapping from ORDER BY terms to result set columns */

  ପ୍ରତିଜ୍ଞା( p->pOrderBy!=0 );
  ପ୍ରତିଜ୍ଞା( p_ଚାବି_Dup==0 ); /* "Managed" code needs this.  Ticket #3382. */
  db = pParse->db;
  v = pParse->pVdbe;
  ପ୍ରତିଜ୍ଞା( v!=0 );       /* Already thrown the error if VDBE alloc failed */
  labelEnd = sqlite3VdbeMakeLabel(pParse);
  labelCmpr = sqlite3VdbeMakeLabel(pParse);


  /* Patch up the ORDER BY clause
  */
  op = p->op; 
  ପ୍ରତିଜ୍ଞା( p->pPrior->pOrderBy==0 );
  pOrderBy = p->pOrderBy;
  ପ୍ରତିଜ୍ଞା( pOrderBy );
  nOrderBy = pOrderBy->n_ପଦସମଷ୍ଟି;

  /* For operators other than UNION ALL we have to make sure that
  ** the ORDER BY clause covers every term of the result set.  Add
  ** terms to the ORDER BY clause as necessary.  Do the same for UNION ALL
  ** if any operator to the left is not UNION ALL, so that the ORDER BY
  ** carries the collation of every column into that sub-compound.
  */
  ପାଇଁ(pPrior=p; pPrior && pPrior->op==TK_ALL; pPrior=pPrior->pPrior){}
  ଯଦି( pPrior && pPrior->pPrior ){
    ପାଇଁ(i=1; db->mallocFailed==0 && i<=p->pEList->n_ପଦସମଷ୍ଟି; i++){
      ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
      ପାଇଁ(j=0, pItem=pOrderBy->a; j<nOrderBy; j++, pItem++){
        ପ୍ରତିଜ୍ଞା( pItem!=0 );
        ପ୍ରତିଜ୍ଞା( pItem->u.x.iOrderByCol>0 );
        ଯଦି( pItem->u.x.iOrderByCol==i ) ଭାଙ୍ଗ;
      }
      ଯଦି( j==nOrderBy ){
        ପଦସମଷ୍ଟି *pNew = sqlite3_ପଦସମଷ୍ଟି_Int32(db, i);
        ଯଦି( pNew==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM_BKPT;
        p->pOrderBy = pOrderBy = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, pOrderBy, pNew);
        ଯଦି( pOrderBy ) pOrderBy->a[nOrderBy++].u.x.iOrderByCol = (u16)i;
      }
    }
  }

  /* Compute the comparison permutation and keyinfo that is used with
  ** the permutation to determine if the next row of results comes
  ** from selectA or selectB.  Also add literal collations to the
  ** ORDER BY clause terms so that when selectA and selectB are
  ** evaluated, they use the correct collation.
  */
  aPermute = sqlite3DbMallocRawNN(db, ଆକାର(u32)*(nOrderBy + 1));
  ଯଦି( aPermute ){
    ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bKeep = 0;
    aPermute[0] = nOrderBy;
    ପାଇଁ(i=1, pItem=pOrderBy->a; i<=nOrderBy; i++, pItem++){
      ପ୍ରତିଜ୍ଞା( pItem!=0 );
      ପ୍ରତିଜ୍ଞା( pItem->u.x.iOrderByCol>0 );
      ପ୍ରତିଜ୍ଞା( pItem->u.x.iOrderByCol<=p->pEList->n_ପଦସମଷ୍ଟି );
      aPermute[i] = pItem->u.x.iOrderByCol - 1;
      ଯଦି( aPermute[i]!=(u32)i-1 ) bKeep = 1;
    }
     ଯଦି( bKeep==0 ){
      sqlite3DbFreeNN(db, aPermute);
      aPermute = 0;
    }
  }
  p_ଚାବି_Merge = multi_Select_By_Merge_ଚାବି_Info(pParse, p, 1);

  /* Allocate a range of temporary registers and the KeyInfo needed
  ** for the logic that removes duplicate result rows when the
  ** operator is UNION, EXCEPT, or INTERSECT (but not UNION ALL).
  */
  ଯଦି( op==TK_ALL ){
    regPrev = 0;
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ପଦସମଷ୍ଟି = p->pEList->n_ପଦସମଷ୍ଟି;
    ପ୍ରତିଜ୍ଞା( nOrderBy>=n_ପଦସମଷ୍ଟି || db->mallocFailed );
    regPrev = pParse->nMem+1;
    pParse->nMem += n_ପଦସମଷ୍ଟି+1;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, regPrev);
    p_ଚାବି_Dup = sqlite3_ଚାବି_Info_Alloc(db, n_ପଦସମଷ୍ଟି, 1);
    ଯଦି( p_ଚାବି_Dup ){
      ପ୍ରତିଜ୍ଞା( sqlite3_ଚାବି_Info_Is_Writeable(p_ଚାବି_Dup) );
      /* Use the ORDER BY collations, not multiSelectCollSeq(p), because
      ** p might be just one part of a larger compound, in which case
      ** the ORDER BY carries the COLLATE of the larger compound. */
      ପାଇଁ(i=0; ALWAYS(p_ଚାବି_Merge) && i<nOrderBy; i++){
        p_ଚାବି_Dup->aColl[pOrderBy->a[i].u.x.iOrderByCol-1] = p_ଚାବି_Merge->aColl[i];
      }
    }
  }

  /* Separate the left and the right query from one another
  */
  nSelect = 1;
  ଯଦି( (op==TK_ALL || op==TK_UNION)
   && OptimizationEnabled(db, SQLITE_BalancedMerge)
  ){
    ପାଇଁ(pSplit=p; pSplit->pPrior!=0 && pSplit->op==op; pSplit=pSplit->pPrior){
      nSelect++;
      ପ୍ରତିଜ୍ଞା( pSplit->pPrior->pNext==pSplit );
    }
  }
  ଯଦି( nSelect<=3 ){
    pSplit = p;
  }ଅନ୍ୟଥା{
    pSplit = p;
    ପାଇଁ(i=2; i<nSelect; i+=2){ pSplit = pSplit->pPrior; }
  }
  pPrior = pSplit->pPrior;
  ପ୍ରତିଜ୍ଞା( pPrior!=0 );
  pSplit->pPrior = 0;
  pPrior->pNext = 0;
  ପ୍ରତିଜ୍ଞା( p->pOrderBy == pOrderBy );
  ପ୍ରତିଜ୍ଞା( pOrderBy!=0 || db->mallocFailed );
  pPrior->pOrderBy = sqlite3_ପଦସମଷ୍ଟି_List_Dup(pParse->db, pOrderBy, 0);
  sqlite3ResolveOrderGroupBy(pParse, p, p->pOrderBy, "ORDER");
  sqlite3ResolveOrderGroupBy(pParse, pPrior, pPrior->pOrderBy, "ORDER");

  /* Compute the limit registers */
  computeLimitRegisters(pParse, p, labelEnd);
  ଯଦି( p->iLimit && (op==TK_ALL || unionWithLimitOne(db,p)) ){
    regLimitA = ++pParse->nMem;
    regLimitB = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Copy, p->iOffset ? p->iOffset+1 : p->iLimit,
                                  regLimitA);
    sqlite3VdbeAddOp2(v, OP_Copy, regLimitA, regLimitB);
  }ଅନ୍ୟଥା{
    regLimitA = regLimitB = 0;
  }
  sqlite3_ପଦସମଷ୍ଟି_Delete(db, p->pLimit);
  p->pLimit = 0;

  regAddrA = ++pParse->nMem;
  regAddrB = ++pParse->nMem;
  regOutA = ++pParse->nMem;
  regOutB = ++pParse->nMem;
  sqlite3SelectDestInit(&destA, SRT_Coroutine, regAddrA);
  sqlite3SelectDestInit(&destB, SRT_Coroutine, regAddrB);

  ExplainQueryPlan((pParse, 1, "MERGE (%s)", sqlite3SelectOpName(p->op)));

  /* Generate a coroutine to evaluate the SELECT statement to the
  ** left of the compound operator - the "A" select.
  */
  addrSelectA = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3VdbeAddOp3(v, OP_InitCoroutine, regAddrA, 0, addrSelectA);
  VdbeComment((v, "SUBR: next-A"));
  pPrior->iLimit = regLimitA;
  ExplainQueryPlan((pParse, 1, "LEFT"));
  sqlite3Select(pParse, pPrior, &destA);
  sqlite3VdbeEndCoroutine(v, regAddrA);
  sqlite3VdbeJumpHere(v, addr1);

  /* Generate a coroutine to evaluate the SELECT statement on
  ** the right - the "B" select
  */
  addrSelectB = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3VdbeAddOp3(v, OP_InitCoroutine, regAddrB, 0, addrSelectB);
  VdbeComment((v, "SUBR: next-B"));
  savedLimit = p->iLimit;
  savedOffset = p->iOffset;
  p->iLimit = regLimitB;
  p->iOffset = 0; 
  ExplainQueryPlan((pParse, 1, "RIGHT"));
  sqlite3Select(pParse, p, &destB);
  p->iLimit = savedLimit;
  p->iOffset = savedOffset;
  sqlite3VdbeEndCoroutine(v, regAddrB);

  /* Generate a subroutine that outputs the current row of the A
  ** select as the next output row of the compound select.
  */
  VdbeNoopComment((v, "SUBR: out-A"));
  addrOutA = generateOutputSubroutine(pParse,
                 p, &destA, pDest, regOutA,
                 regPrev, p_ଚାବି_Dup, labelEnd);
 
  /* Generate a subroutine that outputs the current row of the B
  ** select as the next output row of the compound select.
  */
  ଯଦି( op==TK_ALL || op==TK_UNION ){
    VdbeNoopComment((v, "SUBR: out-B"));
    addrOutB = generateOutputSubroutine(pParse,
                 p, &destB, pDest, regOutB,
                 regPrev, p_ଚାବି_Dup, labelEnd);
  }
  sqlite3_ଚାବି_Info_Unref(p_ଚାବି_Dup);

  /* Generate a subroutine to run when the results from select A
  ** are exhausted and only data in select B remains.
  */
  ଯଦି( op==TK_EXCEPT || op==TK_INTERSECT ){
    addrEofA_noB = addrEofA = labelEnd;
  }ଅନ୍ୟଥା{ 
    VdbeNoopComment((v, "SUBR: eof-A"));
    addrEofA = sqlite3VdbeAddOp2(v, OP_Gosub, regOutB, addrOutB);
    VdbeComment((v, "out-B"));
    addrEofA_noB = sqlite3VdbeAddOp2(v, OP_Yield, regAddrB, labelEnd);
                                     VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, addrEofA);
    p->nSelectRow = sqlite3LogEstAdd(p->nSelectRow, pPrior->nSelectRow);
  }

  /* Generate a subroutine to run when the results from select B
  ** are exhausted and only data in select A remains.
  */
  ଯଦି( op==TK_INTERSECT ){
    addrEofB = addrEofA;
    ଯଦି( p->nSelectRow > pPrior->nSelectRow ) p->nSelectRow = pPrior->nSelectRow;
  }ଅନ୍ୟଥା{ 
    VdbeNoopComment((v, "SUBR: eof-B"));
    addrEofB = sqlite3VdbeAddOp2(v, OP_Gosub, regOutA, addrOutA);
    VdbeComment((v, "out-A"));
    sqlite3VdbeAddOp2(v, OP_Yield, regAddrA, labelEnd); VdbeCoverage(v);
    VdbeComment((v, "next-A"));
    sqlite3VdbeGoto(v, addrEofB);
  }

  /* Generate code to handle the case of A<B
  */
  addrAltB = sqlite3VdbeAddOp2(v, OP_Gosub, regOutA, addrOutA);
  VdbeComment((v, "out-A"));
  sqlite3VdbeAddOp2(v, OP_Yield, regAddrA, addrEofA); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  sqlite3VdbeGoto(v, labelCmpr);

  /* Generate code to handle the case of A==B
  */
  ଯଦି( op==TK_ALL ){
    addrAeqB = addrAltB;
  }ଅନ୍ୟଥା ଯଦି( op==TK_INTERSECT ){
    addrAeqB = addrAltB;
    addrAltB++;
  }ଅନ୍ୟଥା{
    addrAeqB = addrAltB + 1;
  }

  /* Generate code to handle the case of A>B
  */
  addrAgtB = sqlite3VdbeCurrentAddr(v);
  ଯଦି( op==TK_ALL || op==TK_UNION ){
    sqlite3VdbeAddOp2(v, OP_Gosub, regOutB, addrOutB);
    VdbeComment((v, "out-B"));
    sqlite3VdbeAddOp2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, labelCmpr);
  }ଅନ୍ୟଥା{
    addrAgtB++; /* Just do next-B.  Might as well use the next-B call
                ** in the next code block */
  }

  /* This code runs once to initialize everything.
  */
  sqlite3VdbeJumpHere(v, addr1);
  sqlite3VdbeAddOp2(v, OP_Yield, regAddrA, addrEofA_noB); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  /* v---  Also the A>B case for EXCEPT and INTERSECT */
  sqlite3VdbeAddOp2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
  VdbeComment((v, "next-B"));

  /* Implement the main merge loop
  */
  ଯଦି( aPermute!=0 ){
    sqlite3VdbeAddOp4(v, OP_Permutation, 0, 0, 0, (ଅକ୍ଷର*)aPermute, P4_INTARRAY);
  }
  sqlite3VdbeResolveLabel(v, labelCmpr);
  sqlite3VdbeAddOp4(v, OP_Compare, destA.iSdst, destB.iSdst, nOrderBy,
                         (ଅକ୍ଷର*)p_ଚାବି_Merge, P4_KEYINFO);
  ଯଦି( aPermute!=0 ){
    sqlite3VdbeChangeP5(v, OPFLAG_PERMUTE);
  }
  sqlite3VdbeAddOp3(v, OP_Jump, addrAltB, addrAeqB, addrAgtB);
  VdbeCoverageIf(v, op==TK_ALL);
  VdbeCoverageIf(v, op==TK_UNION);
  VdbeCoverageIf(v, op==TK_EXCEPT);
  VdbeCoverageIf(v, op==TK_INTERSECT);

  /* Jump to the this point in order to terminate the query.
  */
  sqlite3VdbeResolveLabel(v, labelEnd);

  /* Make arrangements to free the 2nd and subsequent arms of the compound
  ** after the parse has finished */
  ଯଦି( pSplit->pPrior ){
    sqlite3ParserAddCleanup(pParse, sqlite3SelectDeleteGeneric, pSplit->pPrior);
  }
  pSplit->pPrior = pPrior;
  pPrior->pNext = pSplit;
  sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pPrior->pOrderBy);
  pPrior->pOrderBy = 0;

  /*** TBD:  Insert subroutine calls to close cursors on incomplete
  **** subqueries ****/
  Explain_Query_Plan_କାଢ଼(pParse);
  ଫେରନ୍ତୁ pParse->nErr!=0;
}
#endif

#ଯଦି !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)

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
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା SubstContext {
  Parse *pParse;            /* The parsing context */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTable;               /* Replace references to this table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iNewTable;            /* New table number */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isOuterJoin;          /* Add TK_IF_NULL_ROW opcodes on each replacement */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nSelDepth;            /* Depth of sub-query recursion.  Top==1 */
  ପଦସମଷ୍ଟି_List *pEList;         /* Replacement expressions */
  ପଦସମଷ୍ଟି_List *pCList;         /* Collation sequences for replacement expr */
} SubstContext;

/* Forward Declarations */
ସ୍ଥିର ଶୂନ୍ୟ subst_ପଦସମଷ୍ଟି_List(SubstContext*, ପଦସମଷ୍ଟି_List*);
ସ୍ଥିର ଶୂନ୍ୟ substSelect(SubstContext*, Select*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

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
ସ୍ଥିର ପଦସମଷ୍ଟି *subst_ପଦସମଷ୍ଟି(
  SubstContext *pSubst,  /* Description of the substitution */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି            /* Expr in which substitution occurs */
){
  ଯଦି( p_ପଦସମଷ୍ଟି==0 ) ଫେରନ୍ତୁ 0;
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON|EP_InnerON)
   && p_ପଦସମଷ୍ଟି->w.iJoin==pSubst->iTable
  ){
    testcase( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_InnerON) );
    p_ପଦସମଷ୍ଟି->w.iJoin = pSubst->iNewTable;
  }
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_COLUMN
   && p_ପଦସମଷ୍ଟି->iTable==pSubst->iTable
   && !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_FixedCol)
  ){
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
    ଯଦି( p_ପଦସମଷ୍ଟି->iColumn<0 ){
      p_ପଦସମଷ୍ଟି->op = TK_NULL;
    }ଅନ୍ୟଥା
#endif
    {
      ପଦସମଷ୍ଟି *pNew;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iColumn;
      ପଦସମଷ୍ଟି *pCopy;
      ପଦସମଷ୍ଟି ifNullRow;
      iColumn = p_ପଦସମଷ୍ଟି->iColumn;
      ପ୍ରତିଜ୍ଞା( iColumn>=0 );
      ପ୍ରତିଜ୍ଞା( pSubst->pEList!=0 && iColumn<pSubst->pEList->n_ପଦସମଷ୍ଟି );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ଡାହାଣ==0 );
      pCopy = pSubst->pEList->a[iColumn].p_ପଦସମଷ୍ଟି;
      ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Is_Vector(pCopy) ){
        sqlite3_Vector_Error_ସନ୍ଦେଶ(pSubst->pParse, pCopy);
      }ଅନ୍ୟଥା{
        sqlite3 *db = pSubst->pParse->db;
        ଯଦି( pSubst->isOuterJoin
         && (pCopy->op!=TK_COLUMN || pCopy->iTable!=pSubst->iNewTable)
        ){
          memset(&ifNullRow, 0, ଆକାର(ifNullRow));
          ifNullRow.op = TK_IF_NULL_ROW;
          ifNullRow.p_ବାମ = pCopy;
          ifNullRow.iTable = pSubst->iNewTable;
          ifNullRow.iColumn = -99;
          ifNullRow.flags = EP_IfNullRow;
          pCopy = &ifNullRow;
        }
        testcase( ପଦସମଷ୍ଟି_Has_Property(pCopy, EP_Subquery) );
        pNew = sqlite3_ପଦସମଷ୍ଟି_Dup(db, pCopy, 0);
        ଯଦି( db->mallocFailed ){
          sqlite3_ପଦସମଷ୍ଟି_Delete(db, pNew);
          ଫେରନ୍ତୁ p_ପଦସମଷ୍ଟି;
        }
        ଯଦି( pSubst->isOuterJoin ){
          ପଦସମଷ୍ଟି_Set_Property(pNew, EP_CanBeNull);
        }
        ଯଦି( pNew->op==TK_TRUEFALSE ){
          pNew->u.i_ମୂଲ୍ୟ = sqlite3_ପଦସମଷ୍ଟି_Truth_ମୂଲ୍ୟ(pNew);
          pNew->op = TK_INTEGER;
          ପଦସମଷ୍ଟି_Set_Property(pNew, EP_Int_ମୂଲ୍ୟ);
        }

        /* Ensure that the expression now has an implicit collation sequence,
        ** just as it did when it was a column of a view or sub-query. */
        {
          CollSeq *pNat = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pSubst->pParse, pNew);
          CollSeq *pColl = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pSubst->pParse,
                pSubst->pCList->a[iColumn].p_ପଦସମଷ୍ଟି
          );
          ଯଦି( pNat!=pColl || (pNew->op!=TK_COLUMN && pNew->op!=TK_COLLATE) ){
            pNew = sqlite3_ପଦସମଷ୍ଟି_Add_Collate_String(pSubst->pParse, pNew,
                (pColl ? pColl->zName : "BINARY")
            );
          }
        }
        ପଦସମଷ୍ଟି_Clear_Property(pNew, EP_Collate);
        ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି,EP_OuterON|EP_InnerON) ){
          sqlite3_Set_Join_ପଦସମଷ୍ଟି(pNew, p_ପଦସମଷ୍ଟି->w.iJoin,
                             p_ପଦସମଷ୍ଟି->flags & (EP_OuterON|EP_InnerON));
        }
        sqlite3_ପଦସମଷ୍ଟି_Delete(db, p_ପଦସମଷ୍ଟି);
        p_ପଦସମଷ୍ଟି = pNew;
      }
    }
  }ଅନ୍ୟଥା{
    ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_IF_NULL_ROW && p_ପଦସମଷ୍ଟି->iTable==pSubst->iTable ){
      p_ପଦସମଷ୍ଟି->iTable = pSubst->iNewTable;
    }
    ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_AGG_FUNCTION && p_ପଦସମଷ୍ଟି->op2>=pSubst->nSelDepth ){
      p_ପଦସମଷ୍ଟି->op2--;
    }
    p_ପଦସମଷ୍ଟି->p_ବାମ = subst_ପଦସମଷ୍ଟି(pSubst, p_ପଦସମଷ୍ଟି->p_ବାମ);
    p_ପଦସମଷ୍ଟି->p_ଡାହାଣ = subst_ପଦସମଷ୍ଟି(pSubst, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
    ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) ){
      substSelect(pSubst, p_ପଦସମଷ୍ଟି->x.pSelect, 1);
    }ଅନ୍ୟଥା{
      subst_ପଦସମଷ୍ଟି_List(pSubst, p_ପଦସମଷ୍ଟି->x.pList);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) ){
      Window *pWin = p_ପଦସମଷ୍ଟି->y.pWin;
      pWin->pFilter = subst_ପଦସମଷ୍ଟି(pSubst, pWin->pFilter);
      subst_ପଦସମଷ୍ଟି_List(pSubst, pWin->pPartition);
      subst_ପଦସମଷ୍ଟି_List(pSubst, pWin->pOrderBy);
    }
#endif
  }
  ଫେରନ୍ତୁ p_ପଦସମଷ୍ଟି;
}
ସ୍ଥିର ଶୂନ୍ୟ subst_ପଦସମଷ୍ଟି_List(
  SubstContext *pSubst, /* Description of the substitution */
  ପଦସମଷ୍ଟି_List *pList       /* List to scan and in which to make substitutes */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( pList==0 ) ଫେରନ୍ତୁ;
  ପାଇଁ(i=0; i<pList->n_ପଦସମଷ୍ଟି; i++){
    pList->a[i].p_ପଦସମଷ୍ଟି = subst_ପଦସମଷ୍ଟି(pSubst, pList->a[i].p_ପଦସମଷ୍ଟି);
  }
}
ସ୍ଥିର ଶୂନ୍ୟ substSelect(
  SubstContext *pSubst, /* Description of the substitution */
  Select *p,            /* SELECT statement in which to make substitutions */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା doPrior           /* Do substitutes on p->pPrior too */
){
  SrcList *pSrc;
  SrcItem *pItem;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( !p ) ଫେରନ୍ତୁ;
  pSubst->nSelDepth++;
  କର{
    subst_ପଦସମଷ୍ଟି_List(pSubst, p->pEList);
    subst_ପଦସମଷ୍ଟି_List(pSubst, p->pGroupBy);
    subst_ପଦସମଷ୍ଟି_List(pSubst, p->pOrderBy);
    p->pHaving = subst_ପଦସମଷ୍ଟି(pSubst, p->pHaving);
    p->pWhere = subst_ପଦସମଷ୍ଟି(pSubst, p->pWhere);
    pSrc = p->pSrc;
    ପ୍ରତିଜ୍ଞା( pSrc!=0 );
    ପାଇଁ(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      ଯଦି( pItem->fg.isSubquery ){
        substSelect(pSubst, pItem->u4.pSubq->pSelect, 1);
      }
      ଯଦି( pItem->fg.isTabFunc ){
        subst_ପଦସମଷ୍ଟି_List(pSubst, pItem->u1.pFuncArg);
      }
    }
  }ଯେତେବେଳେ( doPrior && (p = p->pPrior)!=0 );
  pSubst->nSelDepth--;
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ଯଦି !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
/*
** pSelect is a SELECT statement and pSrcItem is one item in the FROM
** clause of that SELECT.
**
** This routine scans the entire SELECT statement and recomputes the
** pSrcItem->colUsed mask.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା recompute_Columns_Used_ପଦସମଷ୍ଟି(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  SrcItem *pItem;
  ଯଦି( p_ପଦସମଷ୍ଟି->op!=TK_COLUMN ) ଫେରନ୍ତୁ WRC_Continue;
  pItem = pWalker->u.pSrcItem;
  ଯଦି( pItem->iCursor!=p_ପଦସମଷ୍ଟି->iTable ) ଫେରନ୍ତୁ WRC_Continue;
  ଯଦି( p_ପଦସମଷ୍ଟି->iColumn<0 ) ଫେରନ୍ତୁ WRC_Continue;
  pItem->colUsed |= sqlite3_ପଦସମଷ୍ଟି_Col_Used(p_ପଦସମଷ୍ଟି);
  ଫେରନ୍ତୁ WRC_Continue;
}
ସ୍ଥିର ଶୂନ୍ୟ recomputeColumnsUsed(
  Select *pSelect,                 /* The complete SELECT statement */
  SrcItem *pSrcItem                /* Which FROM clause item to recompute */
){
  Walker w;
  ଯଦି( NEVER(pSrcItem->pSTab==0) ) ଫେରନ୍ତୁ;
  memset(&w, 0, ଆକାର(w));
  w.x_ପଦସମଷ୍ଟି_Callback = recompute_Columns_Used_ପଦସମଷ୍ଟି;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pSrcItem = pSrcItem;
  pSrcItem->colUsed = 0;
  sqlite3WalkSelect(&w, pSelect);
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ଯଦି !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
ସ୍ଥିର ଶୂନ୍ୟ srclistRenumberCursors(
  Parse *pParse,                  /* Parse context */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aCsrMap,                   /* Array to store cursor mappings in */
  SrcList *pSrc,                  /* FROM clause to renumber */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iExcept                     /* FROM clause item to skip */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  SrcItem *pItem;
  ପାଇଁ(i=0, pItem=pSrc->a; i<pSrc->nSrc; i++, pItem++){
    ଯଦି( i!=iExcept ){
      Select *p;
      ପ୍ରତିଜ୍ଞା( pItem->iCursor < aCsrMap[0] );
      ଯଦି( !pItem->fg.isRecursive || aCsrMap[pItem->iCursor+1]==0 ){
        aCsrMap[pItem->iCursor+1] = pParse->nTab++;
      }
      pItem->iCursor = aCsrMap[pItem->iCursor+1];
      ଯଦି( pItem->fg.isSubquery ){
        ପାଇଁ(p=pItem->u4.pSubq->pSelect; p; p=p->pPrior){
          srclistRenumberCursors(pParse, aCsrMap, p->pSrc, -1);
        }
      }
    }
  }
}

/*
** *piCursor is a cursor number.  Change it if it needs to be mapped.
*/
ସ୍ଥିର ଶୂନ୍ୟ renumberCursorDoMapping(Walker *pWalker, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *piCursor){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aCsrMap = pWalker->u.aiCol;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCsr = *piCursor;
  ଯଦି( iCsr < aCsrMap[0] && aCsrMap[iCsr+1]>0 ){
    *piCursor = aCsrMap[iCsr+1];
  }
}

/*
** Expression walker callback used by renumberCursors() to update
** Expr objects to match newly assigned cursor numbers.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା renumberCursorsCb(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = p_ପଦସମଷ୍ଟି->op;
  ଯଦି( op==TK_COLUMN || op==TK_IF_NULL_ROW ){
    renumberCursorDoMapping(pWalker, &p_ପଦସମଷ୍ଟି->iTable);
  }
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, (EP_OuterON|EP_InnerON)) ){
    testcase( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON) );
    testcase( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_InnerON) );
    renumberCursorDoMapping(pWalker, &p_ପଦସମଷ୍ଟି->w.iJoin);
  }
  ଫେରନ୍ତୁ WRC_Continue;
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
ସ୍ଥିର ଶୂନ୍ୟ renumberCursors(
  Parse *pParse,                  /* Parse context */
  Select *p,                      /* Select to renumber cursors within */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iExcept,                    /* FROM clause item to skip */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aCsrMap                    /* Working space */
){
  Walker w;
  srclistRenumberCursors(pParse, aCsrMap, p->pSrc, iExcept);
  memset(&w, 0, ଆକାର(w));
  w.u.aiCol = aCsrMap;
  w.x_ପଦସମଷ୍ଟି_Callback = renumberCursorsCb;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  sqlite3WalkSelect(&w, p);
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** If pSel is not part of a compound SELECT, return a pointer to its
** expression list. Otherwise, return a pointer to the expression list
** of the leftmost SELECT in the compound.
*/
ସ୍ଥିର ପଦସମଷ୍ଟି_List *findLeftmostExprlist(Select *pSel){
  ଯେତେବେଳେ( pSel->pPrior ){
    pSel = pSel->pPrior;
  }
  ଫେରନ୍ତୁ pSel->pEList;
}

/*
** Return true if any of the result-set columns in the compound query
** have incompatible affinities on one or more arms of the compound.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା compoundHasDifferentAffinities(Select *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  ପଦସମଷ୍ଟି_List *pList;
  ପ୍ରତିଜ୍ଞା( p!=0 );
  ପ୍ରତିଜ୍ଞା( p->pEList!=0 );
  ପ୍ରତିଜ୍ଞା( p->pPrior!=0 );
  pList = p->pEList;
  ପାଇଁ(ii=0; ii<pList->n_ପଦସମଷ୍ଟି; ii++){
    ଅକ୍ଷର aff;
    Select *pSub1;
    ପ୍ରତିଜ୍ଞା( pList->a[ii].p_ପଦସମଷ୍ଟି!=0 );
    aff = sqlite3_ପଦସମଷ୍ଟି_Affinity(pList->a[ii].p_ପଦସମଷ୍ଟି);
    ପାଇଁ(pSub1=p->pPrior; pSub1; pSub1=pSub1->pPrior){
      ପ୍ରତିଜ୍ଞା( pSub1->pEList!=0 );
      ପ୍ରତିଜ୍ଞା( pSub1->pEList->n_ପଦସମଷ୍ଟି>ii );
      ପ୍ରତିଜ୍ଞା( pSub1->pEList->a[ii].p_ପଦସମଷ୍ଟି!=0 );
      ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Affinity(pSub1->pEList->a[ii].p_ପଦସମଷ୍ଟି)!=aff ){
        ଫେରନ୍ତୁ 1;
      }
    }
  }
  ଫେରନ୍ତୁ 0;
}

#ଯଦି !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା flattenSubquery(
  Parse *pParse,       /* Parsing context */
  Select *p,           /* The parent or outer SELECT statement */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFrom,           /* Index in p->pSrc->a[] of the inner subquery */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isAgg            /* True if outer SELECT uses aggregate functions */
){
  ଅଚଳ ଅକ୍ଷର *zSavedAuthContext = pParse->zAuthContext;
  Select *pParent;    /* Current UNION ALL term of the other query */
  Select *pSub;       /* The inner query or "subquery" */
  Select *pSub1;      /* Pointer to the rightmost select in sub-query */
  SrcList *pSrc;      /* The FROM clause of the outer query */
  SrcList *pSubSrc;   /* The FROM clause of the subquery */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iParent;        /* VDBE cursor number of the pSub result set temp table */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iNewParent = -1;/* Replacement table for iParent */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isOuterJoin = 0; /* True if pSub is the right side of a LEFT JOIN */   
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;              /* Loop counter */
  ପଦସମଷ୍ଟି *pWhere;                    /* The WHERE clause */
  SrcItem *pSubitem;               /* The subquery */
  sqlite3 *db = pParse->db;
  Walker w;                        /* Walker to persist agginfo data */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aCsrMap = 0;

  /* Check to see if flattening is permitted.  Return 0 if not.
  */
  ପ୍ରତିଜ୍ଞା( p!=0 );
  ପ୍ରତିଜ୍ଞା( p->pPrior==0 );
  ଯଦି( OptimizationDisabled(db, SQLITE_QueryFlattener) ) ଫେରନ୍ତୁ 0;
  pSrc = p->pSrc;
  ପ୍ରତିଜ୍ଞା( pSrc && iFrom>=0 && iFrom<pSrc->nSrc );
  pSubitem = &pSrc->a[iFrom];
  iParent = pSubitem->iCursor;
  ପ୍ରତିଜ୍ଞା( pSubitem->fg.isSubquery );
  pSub = pSubitem->u4.pSubq->pSelect;
  ପ୍ରତିଜ୍ଞା( pSub!=0 );

#ifndef SQLITE_OMIT_WINDOWFUNC
  ଯଦି( p->pWin || pSub->pWin ) ଫେରନ୍ତୁ 0;                  /* Restriction (25) */
#endif

  pSubSrc = pSub->pSrc;
  ପ୍ରତିଜ୍ଞା( pSubSrc );
  /* Prior to version 3.1.2, when LIMIT and OFFSET had to be simple constants,
  ** not arbitrary expressions, we allowed some combining of LIMIT and OFFSET
  ** because they could be computed at compile-time.  But when LIMIT and OFFSET
  ** became arbitrary expressions, we were forced to add restrictions (13)
  ** and (14). */
  ଯଦି( pSub->pLimit && p->pLimit ) ଫେରନ୍ତୁ 0;              /* Restriction (13) */
  ଯଦି( pSub->pLimit && pSub->pLimit->p_ଡାହାଣ ) ଫେରନ୍ତୁ 0;   /* Restriction (14) */
  ଯଦି( (p->selFlags & SF_Compound)!=0 && pSub->pLimit ){
    ଫେରନ୍ତୁ 0;                                            /* Restriction (15) */
  }
  ଯଦି( pSubSrc->nSrc==0 ) ଫେରନ୍ତୁ 0;                       /* Restriction (7)  */
  ଯଦି( pSub->selFlags & SF_Distinct ) ଫେରନ୍ତୁ 0;           /* Restriction (4)  */
  ଯଦି( pSub->pLimit && (pSrc->nSrc>1 || isAgg) ){
     ଫେରନ୍ତୁ 0;         /* Restrictions (8)(9) */
  }
  ଯଦି( p->pOrderBy && pSub->pOrderBy ){
     ଫେରନ୍ତୁ 0;                                           /* Restriction (11) */
  }
  ଯଦି( isAgg && pSub->pOrderBy ) ଫେରନ୍ତୁ 0;                /* Restriction (16) */
  ଯଦି( pSub->pLimit && p->pWhere ) ଫେରନ୍ତୁ 0;              /* Restriction (19) */
  ଯଦି( pSub->pLimit && (p->selFlags & SF_Distinct)!=0 ){
     ଫେରନ୍ତୁ 0;         /* Restriction (21) */
  }
  ଯଦି( pSub->selFlags & (SF_Recursive) ){
    ଫେରନ୍ତୁ 0; /* Restrictions (22) */
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
  ଯଦି( (pSubitem->fg.jointype & (JT_OUTER|JT_LTORJ))!=0 ){
    ଯଦି( pSubSrc->nSrc>1                        /* (3a) */
     /**** || IsVirtual(pSubSrc->a[0].pSTab)      (3b)-omitted */
     || (p->selFlags & SF_Distinct)!=0         /* (3d) */
     || (pSubitem->fg.jointype & JT_ଡାହାଣ)!=0  /* (26) */
    ){
      ଫେରନ୍ତୁ 0;
    }
    isOuterJoin = 1;
  }

  ପ୍ରତିଜ୍ଞା( pSubSrc->nSrc>0 );  /* True by restriction (7) */
  ଯଦି( iFrom>0 && (pSubSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
    ଫେରନ୍ତୁ 0;   /* Restriction (27a) */
  }

  /* Condition (28) is blocked by the caller */
  ପ୍ରତିଜ୍ଞା( !pSubitem->fg.isCte || pSubitem->u2.pCteUse->eM10d!=M10d_Yes );

  /* Restriction (17): If the sub-query is a compound SELECT, then it must
  ** use only the UNION ALL operator. And none of the simple select queries
  ** that make up the compound SELECT are allowed to be aggregate or distinct
  ** queries.
  */
  ଯଦି( pSub->pPrior ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
    ଯଦି( pSub->pOrderBy ){
      ଫେରନ୍ତୁ 0;  /* Restriction (20) */
    }
    ଯଦି( isAgg || (p->selFlags & SF_Distinct)!=0 || isOuterJoin>0 ){
      ଫେରନ୍ତୁ 0; /* (17d1), (17d2), or (17f) */
    }
    ପାଇଁ(pSub1=pSub; pSub1; pSub1=pSub1->pPrior){
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct );
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Aggregate );
      ପ୍ରତିଜ୍ଞା( pSub->pSrc!=0 );
      ପ୍ରତିଜ୍ଞା( (pSub->selFlags & SF_Recursive)==0 );
      ପ୍ରତିଜ୍ଞା( pSub->pEList->n_ପଦସମଷ୍ଟି==pSub1->pEList->n_ପଦସମଷ୍ଟି );
      ଯଦି( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))!=0    /* (17b) */
       || (pSub1->pPrior && pSub1->op!=TK_ALL)                 /* (17a) */
       || pSub1->pSrc->nSrc<1                                  /* (17c) */
#ifndef SQLITE_OMIT_WINDOWFUNC
       || pSub1->pWin                                          /* (17e) */
#endif
      ){
        ଫେରନ୍ତୁ 0;
      }
      ଯଦି( iFrom>0 && (pSub1->pSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
        /* Without this restriction, the JT_LTORJ flag would end up being
        ** omitted on left-hand tables of the right join that is being
        ** flattened. */
        ଫେରନ୍ତୁ 0;   /* Restrictions (17g), (27b) */
      }
      testcase( pSub1->pSrc->nSrc>1 );
    }

    /* Restriction (18). */
    ଯଦି( p->pOrderBy ){
      ପାଇଁ(ii=0; ii<p->pOrderBy->n_ପଦସମଷ୍ଟି; ii++){
        ଯଦି( p->pOrderBy->a[ii].u.x.iOrderByCol==0 ) ଫେରନ୍ତୁ 0;
      }
    }

    /* Restriction (23) */
    ଯଦି( (p->selFlags & SF_Recursive) ) ଫେରନ୍ତୁ 0;

    /* Restriction (17h) */
    ଯଦି( compoundHasDifferentAffinities(pSub) ) ଫେରନ୍ତୁ 0;

    ଯଦି( pSrc->nSrc>1 ){
      ଯଦି( pParse->nSelect>500 ) ଫେରନ୍ତୁ 0;
      ଯଦି( OptimizationDisabled(db, SQLITE_FlttnUnionAll) ) ଫେରନ୍ତୁ 0;
      aCsrMap = sqlite3DbMallocZero(db, ((i64)pParse->nTab+1)*ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା));
      ଯଦି( aCsrMap ) aCsrMap[0] = pParse->nTab;
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
  
  ଯଦି( ALWAYS(pSubitem->fg.isSubquery) ){
    pSub1 = sqlite3SubqueryDetach(db, pSubitem);
  }ଅନ୍ୟଥା{
    pSub1 = 0;
  }
  ପ୍ରତିଜ୍ଞା( pSubitem->fg.isSubquery==0 );
  ପ୍ରତିଜ୍ଞା( pSubitem->fg.fixedSchema==0 );
  sqlite3DbFree(db, pSubitem->zName);
  sqlite3DbFree(db, pSubitem->zAlias);
  pSubitem->zName = 0;
  pSubitem->zAlias = 0;
  ପ୍ରତିଜ୍ଞା( pSubitem->fg.isUsing!=0 || pSubitem->u3.pOn==0 );

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
  ପାଇଁ(pSub=pSub->pPrior; pSub; pSub=pSub->pPrior){
    Select *pNew;
    ପଦସମଷ୍ଟି_List *pOrderBy = p->pOrderBy;
    ପଦସମଷ୍ଟି *pLimit = p->pLimit;
    Select *pPrior = p->pPrior;
    Table *pItemTab = pSubitem->pSTab;
    pSubitem->pSTab = 0;
    p->pOrderBy = 0;
    p->pPrior = 0;
    p->pLimit = 0;
    pNew = sqlite3SelectDup(db, p, 0);
    p->pLimit = pLimit;
    p->pOrderBy = pOrderBy;
    p->op = TK_ALL;
    pSubitem->pSTab = pItemTab;
    ଯଦି( pNew==0 ){
      p->pPrior = pPrior;
    }ଅନ୍ୟଥା{
      pNew->selId = ++pParse->nSelect;
      ଯଦି( aCsrMap && ALWAYS(db->mallocFailed==0) ){
        renumberCursors(pParse, pNew, iFrom, aCsrMap);
      }
      pNew->pPrior = pPrior;
      ଯଦି( pPrior ) pPrior->pNext = pNew;
      pNew->pNext = p;
      p->pPrior = pNew;
      TREETRACE(0x4,pParse,p,("compound-subquery flattener"
                              " creates %u as peer\n",pNew->selId));
    }
    ପ୍ରତିଜ୍ଞା( pSubitem->fg.isSubquery==0 );
  }
  sqlite3DbFree(db, aCsrMap);
  ଯଦି( db->mallocFailed ){
    ପ୍ରତିଜ୍ଞା( pSubitem->fg.fixedSchema==0 );
    ପ୍ରତିଜ୍ଞା( pSubitem->fg.isSubquery==0 );
    ପ୍ରତିଜ୍ଞା( pSubitem->u4.zDatabase==0 );
    sqlite3SrcItemAttachSubquery(pParse, pSubitem, pSub1, 0);
    ଫେରନ୍ତୁ 1;
  }

  /* Defer deleting the Table object associated with the
  ** subquery until code generation is
  ** complete, since there may still exist Expr.pTab entries that
  ** refer to the subquery even after flattening.  Ticket #3346.
  **
  ** pSubitem->pSTab is always non-NULL by test restrictions and tests above.
  */
  ଯଦି( ALWAYS(pSubitem->pSTab!=0) ){
    Table *pTabToDel = pSubitem->pSTab;
    ଯଦି( pTabToDel->nTabRef==1 ){
      Parse *pToplevel = sqlite3ParseToplevel(pParse);
      sqlite3ParserAddCleanup(pToplevel, sqlite3DeleteTableGeneric, pTabToDel);
      testcase( pToplevel->earlyCleanup );
    }ଅନ୍ୟଥା{
      pTabToDel->nTabRef--;
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
  ପାଇଁ(pParent=p; pParent; pParent=pParent->pPrior, pSub=pSub->pPrior){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nSubSrc;
    u8 jointype = pParent->pSrc->a[iFrom].fg.jointype;
    ପ୍ରତିଜ୍ଞା( pSub!=0 );
    pSubSrc = pSub->pSrc;     /* FROM clause of subquery */
    nSubSrc = pSubSrc->nSrc;  /* Number of terms in subquery FROM clause */
    pSrc = pParent->pSrc;     /* FROM clause of the outer query */

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
    ଯଦି( nSubSrc>1 ){
      pSrc = sqlite3SrcListEnlarge(pParse, pSrc, nSubSrc-1,iFrom+1);
      ଯଦି( pSrc==0 ) ଭାଙ୍ଗ;
      pParent->pSrc = pSrc;
    }

    /* Transfer the FROM clause terms from the subquery into the
    ** outer query.
    */
    iNewParent = pSubSrc->a[0].iCursor;
    ପାଇଁ(i=0; i<nSubSrc; i++){
      SrcItem *pItem = &pSrc->a[i+iFrom];
      ପ୍ରତିଜ୍ଞା( pItem->fg.isTabFunc==0 );
      ପ୍ରତିଜ୍ଞା( pItem->fg.isSubquery
           || pItem->fg.fixedSchema
           || pItem->u4.zDatabase==0 );
      ଯଦି( pItem->fg.isUsing ) sqlite3IdListDelete(db, pItem->u3.pUsing);
      *pItem = pSubSrc->a[i];
      pItem->fg.jointype |= (jointype & JT_LTORJ);
      memset(&pSubSrc->a[i], 0, ଆକାର(pSubSrc->a[i]));
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
    ଯଦି( pSub->pOrderBy ){
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
      ପଦସମଷ୍ଟି_List *pOrderBy = pSub->pOrderBy;
      ପାଇଁ(i=0; i<pOrderBy->n_ପଦସମଷ୍ଟି; i++){
        pOrderBy->a[i].u.x.iOrderByCol = 0;
      }
      ପ୍ରତିଜ୍ଞା( pParent->pOrderBy==0 );
      pParent->pOrderBy = pOrderBy;
      pSub->pOrderBy = 0;
    }
    pWhere = pSub->pWhere;
    pSub->pWhere = 0;
    ଯଦି( isOuterJoin>0 ){
      ପ୍ରତିଜ୍ଞା( pSubSrc->nSrc==1 );
      sqlite3_Set_Join_ପଦସମଷ୍ଟି(pWhere, iNewParent, EP_OuterON);
    }
    ଯଦି( pWhere ){
      ଯଦି( pParent->pWhere ){
        pParent->pWhere = sqlite3PExpr(pParse, TK_AND, pWhere, pParent->pWhere);
      }ଅନ୍ୟଥା{
        pParent->pWhere = pWhere;
      }
    }
    ଯଦି( db->mallocFailed==0 ){
      SubstContext x;
      x.pParse = pParse;
      x.iTable = iParent;
      x.iNewTable = iNewParent;
      x.isOuterJoin = isOuterJoin;
      x.nSelDepth = 0;
      x.pEList = pSub->pEList;
      x.pCList = findLeftmostExprlist(pSub);
      substSelect(&x, pParent, 0);
    }
 
    /* The flattened query is a compound if either the inner or the
    ** outer query is a compound. */
    pParent->selFlags |= pSub->selFlags & SF_Compound;
    ପ୍ରତିଜ୍ଞା( (pSub->selFlags & SF_Distinct)==0 ); /* restriction (17b) */
 
    /*
    ** SELECT ... FROM (SELECT ... LIMIT a OFFSET b) LIMIT x OFFSET y;
    **
    ** One is tempted to try to add a and b to combine the limits.  But this
    ** does not work if either limit is negative.
    */
    ଯଦି( pSub->pLimit ){
      pParent->pLimit = pSub->pLimit;
      pSub->pLimit = 0;
    }

    /* Recompute the SrcItem.colUsed masks for the flattened
    ** tables. */
    ପାଇଁ(i=0; i<nSubSrc; i++){
      recomputeColumnsUsed(pParent, &pSrc->a[i+iFrom]);
    }
  }

  /* Finally, delete what is left of the subquery and return success.
  */
  sqlite3AggInfoPersistWalkerInit(&w, pParse);
  sqlite3WalkSelect(&w,pSub1);
  sqlite3SelectDelete(db, pSub1);

#ଯଦି TREETRACE_ENABLED
  ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x4 ){
    TREETRACE(0x4,pParse,p,("After flattening:\n"));
    sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
  }
#endif

  ଫେରନ୍ତୁ 1;
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** A structure to keep track of all of the column values that are fixed to
** a known value due to WHERE clause constraints of the form COLUMN=VALUE.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା WhereConst WhereConst;
ସଂରଚନା WhereConst {
  Parse *pParse;   /* Parsing context */
  u8 *pOomFault;   /* Pointer to pParse->db->mallocFailed */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nConst;      /* Number for COLUMN=CONSTANT terms */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nChng;       /* Number of times a constant is propagated */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bHasAffBlob; /* At least one column in apExpr[] as affinity BLOB */
  u32 mExcludeOn;  /* Which ON expressions to exclude from considertion.
                   ** Either EP_OuterON or EP_InnerON|EP_OuterON */
  ପଦସମଷ୍ଟି **ap_ପଦସମଷ୍ଟି;   /* [i*2] is COLUMN and [i*2+1] is VALUE */
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
ସ୍ଥିର ଶୂନ୍ୟ const_ଭର୍ତ୍ତି(
  WhereConst *pConst,  /* The WhereConst into which we are inserting */
  ପଦସମଷ୍ଟି *pColumn,       /* The COLUMN part of the constraint */
  ପଦସମଷ୍ଟି *p_ମୂଲ୍ୟ,        /* The VALUE part of the constraint */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି          /* Overall expression: COLUMN=VALUE or VALUE=COLUMN */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପ୍ରତିଜ୍ଞା( pColumn->op==TK_COLUMN );
  ପ୍ରତିଜ୍ଞା( sqlite3_ପଦସମଷ୍ଟି_Is_Constant(pConst->pParse, p_ମୂଲ୍ୟ) );
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(pColumn, EP_FixedCol) ) ଫେରନ୍ତୁ;
  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Affinity(p_ମୂଲ୍ୟ)!=0 ) ଫେରନ୍ତୁ;
  ଯଦି( !sqlite3_Is_ଦ୍ୱିମିକ(sqlite3_ପଦସମଷ୍ଟି_Compare_Coll_Seq(pConst->pParse,p_ପଦସମଷ୍ଟି)) ){
    ଫେରନ୍ତୁ;
  }
  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Can_Return_Subtype(pConst->pParse, p_ମୂଲ୍ୟ) ) ଫେରନ୍ତୁ;

  /* 2018-10-25 ticket [cf5ed20f]
  ** Make sure the same pColumn is not inserted more than once */
  ପାଇଁ(i=0; i<pConst->nConst; i++){
    ଅଚଳ ପଦସମଷ୍ଟି *pE2 = pConst->ap_ପଦସମଷ୍ଟି[i*2];
    ପ୍ରତିଜ୍ଞା( pE2->op==TK_COLUMN );
    ଯଦି( pE2->iTable==pColumn->iTable
     && pE2->iColumn==pColumn->iColumn
    ){
      ଫେରନ୍ତୁ;  /* Already present.  Return without doing anything. */
    }
  }
  ପ୍ରତିଜ୍ଞା( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Affinity(pColumn)<=SQLITE_AFF_BLOB ){
    pConst->bHasAffBlob = 1;
  }

  pConst->nConst++;
  pConst->ap_ପଦସମଷ୍ଟି = sqlite3DbReallocOrFree(pConst->pParse->db, pConst->ap_ପଦସମଷ୍ଟି,
                         pConst->nConst*2*ଆକାର(ପଦସମଷ୍ଟି*));
  ଯଦି( pConst->ap_ପଦସମଷ୍ଟି==0 ){
    pConst->nConst = 0;
  }ଅନ୍ୟଥା{
    pConst->ap_ପଦସମଷ୍ଟି[pConst->nConst*2-2] = pColumn;
    pConst->ap_ପଦସମଷ୍ଟି[pConst->nConst*2-1] = p_ମୂଲ୍ୟ;
  }
}

/*
** Find all terms of COLUMN=VALUE or VALUE=COLUMN in pExpr where VALUE
** is a constant expression and where the term must be true because it
** is part of the AND-connected terms of the expression.  For each term
** found, add it to the pConst structure.
*/
ସ୍ଥିର ଶୂନ୍ୟ findConstInWhere(WhereConst *pConst, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପଦସମଷ୍ଟି *p_ଡାହାଣ, *p_ବାମ;
  ଯଦି( NEVER(p_ପଦସମଷ୍ଟି==0) ) ଫେରନ୍ତୁ;
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, pConst->mExcludeOn) ){
    testcase( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON) );
    testcase( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_InnerON) );
    ଫେରନ୍ତୁ;
  }
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_AND ){
    findConstInWhere(pConst, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
    findConstInWhere(pConst, p_ପଦସମଷ୍ଟି->p_ବାମ);
    ଫେରନ୍ତୁ;
  }
  ଯଦି( p_ପଦସମଷ୍ଟି->op!=TK_EQ ) ଫେରନ୍ତୁ;
  p_ଡାହାଣ = p_ପଦସମଷ୍ଟି->p_ଡାହାଣ;
  p_ବାମ = p_ପଦସମଷ୍ଟି->p_ବାମ;
  ପ୍ରତିଜ୍ଞା( p_ଡାହାଣ!=0 );
  ପ୍ରତିଜ୍ଞା( p_ବାମ!=0 );
  ଯଦି( p_ଡାହାଣ->op==TK_COLUMN && sqlite3_ପଦସମଷ୍ଟି_Is_Constant(pConst->pParse, p_ବାମ) ){
    const_ଭର୍ତ୍ତି(pConst,p_ଡାହାଣ,p_ବାମ,p_ପଦସମଷ୍ଟି);
  }
  ଯଦି( p_ବାମ->op==TK_COLUMN && sqlite3_ପଦସମଷ୍ଟି_Is_Constant(pConst->pParse, p_ଡାହାଣ) ){
    const_ଭର୍ତ୍ତି(pConst,p_ବାମ,p_ଡାହାଣ,p_ପଦସମଷ୍ଟି);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା propagate_Constant_ପଦସମଷ୍ଟି_Rewrite_One(
  WhereConst *pConst,
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bIgnoreAffBlob
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( pConst->pOomFault[0] ) ଫେରନ୍ତୁ WRC_Prune;
  ଯଦି( p_ପଦସମଷ୍ଟି->op!=TK_COLUMN ) ଫେରନ୍ତୁ WRC_Continue;
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_FixedCol|pConst->mExcludeOn) ){
    testcase( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_FixedCol) );
    testcase( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON) );
    testcase( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_InnerON) );
    ଫେରନ୍ତୁ WRC_Continue;
  }
  ପାଇଁ(i=0; i<pConst->nConst; i++){
    ପଦସମଷ୍ଟି *pColumn = pConst->ap_ପଦସମଷ୍ଟି[i*2];
    ଯଦି( pColumn==p_ପଦସମଷ୍ଟି ) ଜାରି;
    ଯଦି( pColumn->iTable!=p_ପଦସମଷ୍ଟି->iTable ) ଜାରି;
    ଯଦି( pColumn->iColumn!=p_ପଦସମଷ୍ଟି->iColumn ) ଜାରି;
    ପ୍ରତିଜ୍ଞା( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
    ଯଦି( bIgnoreAffBlob && sqlite3_ପଦସମଷ୍ଟି_Affinity(pColumn)<=SQLITE_AFF_BLOB ){
      ଭାଙ୍ଗ;
    }
    /* A match is found.  Add the EP_FixedCol property */
    pConst->nChng++;
    ପଦସମଷ୍ଟି_Clear_Property(p_ପଦସମଷ୍ଟି, EP_Leaf);
    ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_FixedCol);
    ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ବାମ==0 );
    p_ପଦସମଷ୍ଟି->p_ବାମ = sqlite3_ପଦସମଷ୍ଟି_Dup(pConst->pParse->db, pConst->ap_ପଦସମଷ୍ଟି[i*2+1], 0);
    ଯଦି( pConst->pParse->db->mallocFailed ) ଫେରନ୍ତୁ WRC_Prune;
    ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ WRC_Prune;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା propagate_Constant_ପଦସମଷ୍ଟି_Rewrite(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  WhereConst *pConst = pWalker->u.pConst;
  ପ୍ରତିଜ୍ଞା( TK_GT==TK_EQ+1 );
  ପ୍ରତିଜ୍ଞା( TK_LE==TK_EQ+2 );
  ପ୍ରତିଜ୍ଞା( TK_LT==TK_EQ+3 );
  ପ୍ରତିଜ୍ଞା( TK_GE==TK_EQ+4 );
  ଯଦି( pConst->bHasAffBlob ){
    ଯଦି( (p_ପଦସମଷ୍ଟି->op>=TK_EQ && p_ପଦସମଷ୍ଟି->op<=TK_GE)
     || p_ପଦସମଷ୍ଟି->op==TK_IS
    ){
      propagate_Constant_ପଦସମଷ୍ଟି_Rewrite_One(pConst, p_ପଦସମଷ୍ଟି->p_ବାମ, 0);
      ଯଦି( pConst->pOomFault[0] ) ଫେରନ୍ତୁ WRC_Prune;
      ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Affinity(p_ପଦସମଷ୍ଟି->p_ବାମ)!=SQLITE_AFF_TEXT ){
        propagate_Constant_ପଦସମଷ୍ଟି_Rewrite_One(pConst, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, 0);
      }
    }
  }
  ଫେରନ୍ତୁ propagate_Constant_ପଦସମଷ୍ଟି_Rewrite_One(pConst, p_ପଦସମଷ୍ଟି, pConst->bHasAffBlob);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା propagateConstants(
  Parse *pParse,   /* The parsing context */
  Select *p        /* The query in which to propagate constants */
){
  WhereConst x;
  Walker w;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nChng = 0;
  x.pParse = pParse;
  x.pOomFault = &pParse->db->mallocFailed;
  କର{
    x.nConst = 0;
    x.nChng = 0;
    x.ap_ପଦସମଷ୍ଟି = 0;
    x.bHasAffBlob = 0;
    ଯଦି( ALWAYS(p->pSrc!=0)
     && p->pSrc->nSrc>0
     && (p->pSrc->a[0].fg.jointype & JT_LTORJ)!=0
    ){
      /* Do not propagate constants on any ON clause if there is a
      ** RIGHT JOIN anywhere in the query */
      x.mExcludeOn = EP_InnerON | EP_OuterON;
    }ଅନ୍ୟଥା{
      /* Do not propagate constants through the ON clause of a LEFT JOIN */
      x.mExcludeOn = EP_OuterON;
    }
    findConstInWhere(&x, p->pWhere);
    ଯଦି( x.nConst ){
      memset(&w, 0, ଆକାର(w));
      w.pParse = pParse;
      w.x_ପଦସମଷ୍ଟି_Callback = propagate_Constant_ପଦସମଷ୍ଟି_Rewrite;
      w.xSelectCallback = sqlite3SelectWalkNoop;
      w.xSelectCallback2 = 0;
      w.walkerDepth = 0;
      w.u.pConst = &x;
      sqlite3_Walk_ପଦସମଷ୍ଟି(&w, p->pWhere);
      sqlite3DbFree(x.pParse->db, x.ap_ପଦସମଷ୍ଟି);
      nChng += x.nChng;
    }
  }ଯେତେବେଳେ( x.nChng ); 
  ଫେରନ୍ତୁ nChng;
}

#ଯଦି !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
# ଯଦି !defined(SQLITE_OMIT_WINDOWFUNC)
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଠେଲ_Down_Window_Check(Parse *pParse, Select *pSubq, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପ୍ରତିଜ୍ଞା( pSubq->pWin->pPartition );
  ପ୍ରତିଜ୍ଞା( (pSubq->selFlags & SF_MultiPart)==0 );
  ପ୍ରତିଜ୍ଞା( pSubq->pPrior==0 );
  ଫେରନ୍ତୁ sqlite3_ପଦସମଷ୍ଟି_Is_Constant_Or_Group_By(pParse, p_ପଦସମଷ୍ଟି, pSubq->pWin->pPartition);
}
# endif /* SQLITE_OMIT_WINDOWFUNC */
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ଯଦି !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଠେଲ_Down_Where_Terms(
  Parse *pParse,        /* Parse context (for malloc() and error reporting) */
  Select *pSubq,        /* The subquery whose WHERE clause is to be augmented */
  ପଦସମଷ୍ଟି *pWhere,         /* The WHERE clause of the outer query */
  SrcList *pSrcList,    /* The complete from clause of the outer query */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSrc              /* Which FROM clause term to try to push into  */
){
  ପଦସମଷ୍ଟି *pNew;
  SrcItem *pSrc;        /* The subquery FROM term into which WHERE is pushed */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nChng = 0;
  pSrc = &pSrcList->a[iSrc];
  ଯଦି( pWhere==0 ) ଫେରନ୍ତୁ 0;
  ଯଦି( pSubq->selFlags & (SF_Recursive|SF_MultiPart) ){
    ଫେରନ୍ତୁ 0;           /* restrictions (2) and (11) */
  }
  ଯଦି( pSrc->fg.jointype & (JT_LTORJ|JT_ଡାହାଣ) ){
    ଫେରନ୍ତୁ 0;           /* restrictions (10) */
  }

  ଯଦି( pSubq->pPrior ){
    Select *pSel;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା notUnionAll = 0;
    ପାଇଁ(pSel=pSubq; pSel; pSel=pSel->pPrior){
      u8 op = pSel->op;
      ପ୍ରତିଜ୍ଞା( op==TK_ALL || op==TK_SELECT
           || op==TK_UNION || op==TK_INTERSECT || op==TK_EXCEPT );
      ଯଦି( op!=TK_ALL && op!=TK_SELECT ){
        notUnionAll = 1;
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      ଯଦି( pSel->pWin ) ଫେରନ୍ତୁ 0;    /* restriction (6b) */
#endif
    }
    ଯଦି( notUnionAll ){
      /* If any of the compound arms are connected using UNION, INTERSECT,
      ** or EXCEPT, then we must ensure that none of the columns use a
      ** non-BINARY collating sequence. */
      ପାଇଁ(pSel=pSubq; pSel; pSel=pSel->pPrior){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
        ଅଚଳ ପଦସମଷ୍ଟି_List *pList = pSel->pEList;
        ପ୍ରତିଜ୍ଞା( pList!=0 );
        ପାଇଁ(ii=0; ii<pList->n_ପଦସମଷ୍ଟି; ii++){
          CollSeq *pColl = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, pList->a[ii].p_ପଦସମଷ୍ଟି);
          ଯଦି( !sqlite3_Is_ଦ୍ୱିମିକ(pColl) ){
            ଫେରନ୍ତୁ 0;  /* Restriction (8) */
          }
        }
      }
    }
  }ଅନ୍ୟଥା{
#ifndef SQLITE_OMIT_WINDOWFUNC
    ଯଦି( pSubq->pWin && pSubq->pWin->pPartition==0 ) ଫେରନ୍ତୁ 0;
#endif
  }

#ifdef SQLITE_DEBUG
  /* Only the first term of a compound can have a WITH clause.  But make
  ** sure no other terms are marked SF_Recursive in case something changes
  ** in the future.
  */
  {
    Select *pX; 
    ପାଇଁ(pX=pSubq; pX; pX=pX->pPrior){
      ପ୍ରତିଜ୍ଞା( (pX->selFlags & (SF_Recursive))==0 );
    }
  }
#endif

  ଯଦି( pSubq->pLimit!=0 ){
    ଫେରନ୍ତୁ 0; /* restriction (3) */
  }
  ଯେତେବେଳେ( pWhere->op==TK_AND ){
    nChng += ଠେଲ_Down_Where_Terms(pParse, pSubq, pWhere->p_ଡାହାଣ, pSrcList, iSrc);
    pWhere = pWhere->p_ବାମ;
  }

#ଯଦି 0 /* These checks now done by sqlite3ExprIsSingleTableConstraint() */
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(pWhere, EP_OuterON|EP_InnerON) /* (9a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (9c) */
  ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା jj;
    ପାଇଁ(jj=0; jj<iSrc; jj++){
      ଯଦି( pWhere->w.iJoin==pSrcList->a[jj].iCursor ){
        /* If we reach this point, both (9a) and (9b) are satisfied.
        ** The following loop checks (9c):
        */
        ପାଇଁ(jj++; jj<iSrc; jj++){
          ଯଦି( (pSrcList->a[jj].fg.jointype & JT_ଡାହାଣ)!=0 ){
            ଫେରନ୍ତୁ 0;  /* restriction (9) */
          }
        }
      }
    }
  }
  ଯଦି( is_ବାମ_Join
   && (ପଦସମଷ୍ଟି_Has_Property(pWhere,EP_OuterON)==0
         || pWhere->w.iJoin!=iCursor)
  ){
    ଫେରନ୍ତୁ 0; /* restriction (4) */
  }
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(pWhere,EP_OuterON)
   && pWhere->w.iJoin!=iCursor
  ){
    ଫେରନ୍ତୁ 0; /* restriction (5) */
  }
#endif

#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
  ଯଦି( ViewCanHaveRowid && (pWhere->op==TK_ISNULL || pWhere->op==TK_NOTNULL) ){
    ପଦସମଷ୍ଟି *p_ବାମ = pWhere->p_ବାମ;
    ଯଦି( ALWAYS(p_ବାମ) 
     && p_ବାମ->op==TK_COLUMN
     && p_ବାମ->iColumn < 0
    ){
      ଫେରନ୍ତୁ 0;  /* Restriction (12) */
    }
  }
#endif

  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Is_Single_Table_Constraint(pWhere, pSrcList, iSrc, 1) ){
    nChng++;
    pSubq->selFlags |= SF_ଠେଲ_Down;
    ଯେତେବେଳେ( pSubq ){
      SubstContext x;
      pNew = sqlite3_ପଦସମଷ୍ଟି_Dup(pParse->db, pWhere, 0);
      unset_Join_ପଦସମଷ୍ଟି(pNew, -1, 1);
      x.pParse = pParse;
      x.iTable = pSrc->iCursor;
      x.iNewTable = pSrc->iCursor;
      x.isOuterJoin = 0;
      x.nSelDepth = 0;
      x.pEList = pSubq->pEList;
      x.pCList = findLeftmostExprlist(pSubq);
      pNew = subst_ପଦସମଷ୍ଟି(&x, pNew);
      ପ୍ରତିଜ୍ଞା( pNew!=0 || pParse->nErr!=0 );
      ଯଦି( pParse->nErr==0 && pNew->op==TK_IN && ପଦସମଷ୍ଟି_Use_XSelect(pNew) ){
        ପ୍ରତିଜ୍ଞା( pNew->x.pSelect!=0 );
        pNew->x.pSelect->selFlags |= SF_ClonedRhsIn;
        ପ୍ରତିଜ୍ଞା( pWhere!=0 );
        ପ୍ରତିଜ୍ଞା( pWhere->op==TK_IN );
        ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(pWhere) );
        ପ୍ରତିଜ୍ଞା( pWhere->x.pSelect!=0 );
        pWhere->x.pSelect->selFlags |= SF_ClonedRhsIn;
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      ଯଦି( pSubq->pWin && 0==ଠେଲ_Down_Window_Check(pParse, pSubq, pNew) ){
        /* Restriction 6c has prevented push-down in this case */
        sqlite3_ପଦସମଷ୍ଟି_Delete(pParse->db, pNew);
        nChng--;
        ଭାଙ୍ଗ;
      }
#endif
      ଯଦି( pSubq->selFlags & SF_Aggregate ){
        pSubq->pHaving = sqlite3_ପଦସମଷ୍ଟି_And(pParse, pSubq->pHaving, pNew);
      }ଅନ୍ୟଥା{
        pSubq->pWhere = sqlite3_ପଦସମଷ୍ଟି_And(pParse, pSubq->pWhere, pNew);
      }
      pSubq = pSubq->pPrior;
    }
  }
  ଫେରନ୍ତୁ nChng;
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** Check to see if a subquery contains result-set columns that are
** never used.  If it does, change the value of those result-set columns
** to NULL so that they do not cause unnecessary work to compute.
**
** Return the number of column that were changed to NULL.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା disableUnusedSubqueryResultColumns(SrcItem *pItem){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol;
  Select *pSub;      /* The subquery to be simplified */
  Select *pX;        /* For looping over compound elements of pSub */
  Table *pTab;       /* The table that describes the subquery */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;             /* Column number */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nChng = 0;     /* Number of columns converted to NULL */
  Bitmask colUsed;   /* Columns that may not be NULLed out */

  ପ୍ରତିଜ୍ଞା( pItem!=0 );
  ଯଦି( pItem->fg.isCorrelated || pItem->fg.isCte ){
    ଫେରନ୍ତୁ 0;
  }
  ପ୍ରତିଜ୍ଞା( pItem->pSTab!=0 );
  pTab = pItem->pSTab;
  ପ୍ରତିଜ୍ଞା( pItem->fg.isSubquery );
  pSub = pItem->u4.pSubq->pSelect;
  ପ୍ରତିଜ୍ଞା( pSub->pEList->n_ପଦସମଷ୍ଟି==pTab->nCol );
  ପାଇଁ(pX=pSub; pX; pX=pX->pPrior){
    ଯଦି( (pX->selFlags & (SF_Distinct|SF_Aggregate))!=0 ){
      testcase( pX->selFlags & SF_Distinct );
      testcase( pX->selFlags & SF_Aggregate );
      ଫେରନ୍ତୁ 0;
    }
    ଯଦି( pX->pPrior && pX->op!=TK_ALL ){
      /* This optimization does not work for compound subqueries that
      ** use UNION, INTERSECT, or EXCEPT.  Only UNION ALL is allowed. */
      ଫେରନ୍ତୁ 0;
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    ଯଦି( pX->pWin ){
      /* This optimization does not work for subqueries that use window
      ** functions. */
      ଫେରନ୍ତୁ 0;
    }
#endif
  }
  colUsed = pItem->colUsed;
  ଯଦି( pSub->pOrderBy ){
    ପଦସମଷ୍ଟି_List *pList = pSub->pOrderBy;
    ପାଇଁ(j=0; j<pList->n_ପଦସମଷ୍ଟି; j++){
      u16 iCol = pList->a[j].u.x.iOrderByCol;
      ଯଦି( iCol>0 ){
        iCol--;
        colUsed |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
      }
    }
  }
  nCol = pTab->nCol;
  ପାଇଁ(j=0; j<nCol; j++){
    Bitmask m = j<BMS-1 ? MASKBIT(j) : TOPBIT;
    ଯଦି( (m & colUsed)!=0 ) ଜାରି;
    ପାଇଁ(pX=pSub; pX; pX=pX->pPrior) {
      ପଦସମଷ୍ଟି *pY = pX->pEList->a[j].p_ପଦସମଷ୍ଟି;
      ଯଦି( pY->op==TK_NULL ) ଜାରି;
      pY->op = TK_NULL;
      ପଦସମଷ୍ଟି_Clear_Property(pY, EP_Skip|EP_Unlikely);
      pX->selFlags |= SF_ଠେଲ_Down;
      nChng++;
    }
  }
  ଫେରନ୍ତୁ nChng;
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
ସ୍ଥିର u8 minMaxQuery(sqlite3 *db, ପଦସମଷ୍ଟି *pFunc, ପଦସମଷ୍ଟି_List **ppMinMax){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eRet = WHERE_ORDERBY_NORMAL;      /* Return value */
  ପଦସମଷ୍ଟି_List *pEList;                     /* Arguments to agg function */
  ଅଚଳ ଅକ୍ଷର *zFunc;                    /* Name of aggregate function pFunc */
  ପଦସମଷ୍ଟି_List *pOrderBy;
  u8 sortFlags = 0;

  ପ୍ରତିଜ୍ଞା( *ppMinMax==0 );
  ପ୍ରତିଜ୍ଞା( pFunc->op==TK_AGG_FUNCTION );
  ପ୍ରତିଜ୍ଞା( !IsWindowFunc(pFunc) );
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pFunc) );
  pEList = pFunc->x.pList;
  ଯଦି( pEList==0
   || pEList->n_ପଦସମଷ୍ଟି!=1
   || ପଦସମଷ୍ଟି_Has_Property(pFunc, EP_WinFunc)
   || OptimizationDisabled(db, SQLITE_MinMaxOpt)
  ){
    ଫେରନ୍ତୁ eRet;
  }
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(pFunc, EP_Int_ମୂଲ୍ୟ) );
  zFunc = pFunc->u.zToken;
  ଯଦି( sqlite3StrICmp(zFunc, "min")==0 ){
    eRet = WHERE_ORDERBY_MIN;
    ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Can_Be_Null(pEList->a[0].p_ପଦସମଷ୍ଟି) ){
      sortFlags = KEYINFO_ORDER_BIGNULL;
    }
  }ଅନ୍ୟଥା ଯଦି( sqlite3StrICmp(zFunc, "max")==0 ){
    eRet = WHERE_ORDERBY_MAX;
    sortFlags = KEYINFO_ORDER_DESC;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ eRet;
  }
  *ppMinMax = pOrderBy = sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, pEList, 0);
  ପ୍ରତିଜ୍ଞା( pOrderBy!=0 || db->mallocFailed );
  ଯଦି( pOrderBy ) pOrderBy->a[0].fg.sortFlags = sortFlags;
  ଫେରନ୍ତୁ eRet;
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
ସ୍ଥିର Table *is_Simple_ଗଣନା(Select *p, AggInfo *pAggInfo){
  Table *pTab;
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି;

  ପ୍ରତିଜ୍ଞା( !p->pGroupBy );

  ଯଦି( p->pWhere
   || p->pEList->n_ପଦସମଷ୍ଟି!=1
   || p->pSrc->nSrc!=1
   || p->pSrc->a[0].fg.isSubquery
   || pAggInfo->nFunc!=1
   || p->pHaving
  ){
    ଫେରନ୍ତୁ 0;
  }
  pTab = p->pSrc->a[0].pSTab;
  ପ୍ରତିଜ୍ଞା( pTab!=0 );
  ପ୍ରତିଜ୍ଞା( !IsView(pTab) );
  ଯଦି( !IsOrdinaryTable(pTab) ) ଫେରନ୍ତୁ 0;
  p_ପଦସମଷ୍ଟି = p->pEList->a[0].p_ପଦସମଷ୍ଟି;
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି!=0 );
  ଯଦି( p_ପଦସମଷ୍ଟି->op!=TK_AGG_FUNCTION ) ଫେରନ୍ତୁ 0;
  ଯଦି( p_ପଦସମଷ୍ଟି->pAggInfo!=pAggInfo ) ଫେରନ୍ତୁ 0;
  ଯଦି( (pAggInfo->aFunc[0].pFunc->funcFlags&SQLITE_FUNC_ଗଣନା)==0 ) ଫେରନ୍ତୁ 0;
  ପ୍ରତିଜ୍ଞା( pAggInfo->aFunc[0].pFExpr==p_ପଦସମଷ୍ଟି );
  testcase( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Distinct) );
  testcase( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) );
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Distinct|EP_WinFunc) ) ଫେରନ୍ତୁ 0;

  ଫେରନ୍ତୁ pTab;
}

/*
** If the source-list item passed as an argument was augmented with an
** INDEXED BY clause, then try to locate the specified index. If there
** was such a clause and the named index cannot be found, return
** SQLITE_ERROR and leave an error in pParse. Otherwise, populate
** pFrom->pIndex and return SQLITE_OK.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3IndexedByLookup(Parse *pParse, SrcItem *pFrom){
  Table *pTab = pFrom->pSTab;
  ଅକ୍ଷର *zIndexedBy = pFrom->u1.zIndexedBy;
  Index *pIdx;
  ପ୍ରତିଜ୍ଞା( pTab!=0 );
  ପ୍ରତିଜ୍ଞା( pFrom->fg.isIndexedBy!=0 );

  ପାଇଁ(pIdx=pTab->pIndex;
      pIdx && sqlite3StrICmp(pIdx->zName, zIndexedBy);
      pIdx=pIdx->pNext
  );
  ଯଦି( !pIdx ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "no such index: %s", zIndexedBy, 0);
    pParse->checkSchema = 1;
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }
  ପ୍ରତିଜ୍ଞା( pFrom->fg.isCte==0 );
  pFrom->u2.pIBIndex = pIdx;
  ଫେରନ୍ତୁ SQLITE_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା convertCompoundSelectToSubquery(Walker *pWalker, Select *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  Select *pNew;
  Select *pX;
  sqlite3 *db;
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *a;
  SrcList *pNewSrc;
  Parse *pParse;
  Token dummy;

  ଯଦି( p->pPrior==0 ) ଫେରନ୍ତୁ WRC_Continue;
  ଯଦି( p->pOrderBy==0 ) ଫେରନ୍ତୁ WRC_Continue;
  ପାଇଁ(pX=p; pX && (pX->op==TK_ALL || pX->op==TK_SELECT); pX=pX->pPrior){}
  ଯଦି( pX==0 ) ଫେରନ୍ତୁ WRC_Continue;
  a = p->pOrderBy->a;
#ifndef SQLITE_OMIT_WINDOWFUNC
  /* If iOrderByCol is already non-zero, then it has already been matched
  ** to a result column of the SELECT statement. This occurs when the
  ** SELECT is rewritten for window-functions processing and then passed
  ** to sqlite3SelectPrep() and similar a second time. The rewriting done
  ** by this function is not required in this case. */
  ଯଦି( a[0].u.x.iOrderByCol ) ଫେରନ୍ତୁ WRC_Continue;
#endif
  ପାଇଁ(i=p->pOrderBy->n_ପଦସମଷ୍ଟି-1; i>=0; i--){
    ଯଦି( a[i].p_ପଦସମଷ୍ଟି->flags & EP_Collate ) ଭାଙ୍ଗ;
  }
  ଯଦି( i<0 ) ଫେରନ୍ତୁ WRC_Continue;

  /* If we reach this point, that means the transformation is required. */

  pParse = pWalker->pParse;
  db = pParse->db;
  pNew = sqlite3DbMallocZero(db, ଆକାର(*pNew) );
  ଯଦି( pNew==0 ) ଫେରନ୍ତୁ WRC_Abort;
  memset(&dummy, 0, ଆକାର(dummy));
  pNewSrc = sqlite3SrcListAppendFromTerm(pParse,0,0,0,&dummy,pNew,0);
  ପ୍ରତିଜ୍ଞା( pNewSrc!=0 || pParse->nErr );
  ଯଦି( pParse->nErr ){
    sqlite3SrcListDelete(db, pNewSrc);
    ଫେରନ୍ତୁ WRC_Abort;
  }
  *pNew = *p;
  p->pSrc = pNewSrc;
  p->pEList = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, 0, sqlite3_ପଦସମଷ୍ଟି(db, TK_ASTERISK, 0));
  p->op = TK_SELECT;
  p->pWhere = 0;
  p->pGroupBy = 0;
  p->pHaving = 0;
  pNew->pOrderBy = 0;
  p->pPrior = 0;
  p->pNext = 0;
  p->pWith = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
  p->pWinDefn = 0;
#endif
  p->selFlags &= ~(u32)(SF_Compound|SF_Distinct);
  ପ୍ରତିଜ୍ଞା( (p->selFlags & SF_Converted)==0 );
  p->selFlags |= SF_Converted;
  ପ୍ରତିଜ୍ଞା( pNew->pPrior!=0 );
  pNew->pPrior->pNext = pNew;
  pNew->pLimit = 0;
  ଫେରନ୍ତୁ WRC_Continue;
}

/*
** Check to see if the FROM clause term pFrom has table-valued function
** arguments.  If it does, leave an error message in pParse and return
** non-zero, since pFrom is not allowed to be a table-valued function.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cannotBeFunction(Parse *pParse, SrcItem *pFrom){
  ଯଦି( pFrom->fg.isTabFunc ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "'%s' is not a function", pFrom->zName);
    ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
}

#ifndef SQLITE_OMIT_CTE
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
ସ୍ଥିର ସଂରଚନା Cte *ସନ୍ଧାନ_With(
  With *pWith,                    /* Current innermost WITH clause */
  SrcItem *pItem,                 /* FROM clause element to resolve */
  With **ppContext                /* OUT: WITH clause return value belongs to */
){
  ଅଚଳ ଅକ୍ଷର *zName = pItem->zName;
  With *p;
  ପ୍ରତିଜ୍ଞା( pItem->fg.fixedSchema || pItem->u4.zDatabase==0 );
  ପ୍ରତିଜ୍ଞା( zName!=0 );
  ପାଇଁ(p=pWith; p; p=p->pOuter){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<p->nCte; i++){
      ଯଦି( sqlite3StrICmp(zName, p->a[i].zName)==0 ){
        *ppContext = p;
        ଫେରନ୍ତୁ &p->a[i];
      }
    }
    ଯଦି( p->bView ) ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ 0;
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
With *sqlite3_With_ଠେଲ(Parse *pParse, With *pWith, u8 bFree){
  ଯଦି( pWith ){
    ଯଦି( bFree ){
      pWith = (With*)sqlite3ParserAddCleanup(pParse, sqlite3WithDeleteGeneric,
                      pWith);
      ଯଦି( pWith==0 ) ଫେରନ୍ତୁ 0;
    }
    ଯଦି( pParse->nErr==0 ){
      ପ୍ରତିଜ୍ଞା( pParse->pWith!=pWith );
      pWith->pOuter = pParse->pWith;
      pParse->pWith = pWith;
    }
  }
  ଫେରନ୍ତୁ pWith;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା selectExpanderWalk(Walker*, Select*);

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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା resolveFromTermToCte(
  Parse *pParse,                  /* The parsing context */
  Walker *pWalker,                /* Current tree walker */
  SrcItem *pFrom                  /* The FROM clause term to check */
){
  Cte *pCte;               /* Matched CTE (or NULL if no match) */
  With *pWith;             /* The matching WITH */

  ପ୍ରତିଜ୍ଞା( pFrom->pSTab==0 );
  ଯଦି( pParse->pWith==0 ){
    /* There are no WITH clauses in the stack.  No match is possible */
    ଫେରନ୍ତୁ 0;
  }
  ଯଦି( pParse->nErr ){
    /* Prior errors might have left pParse->pWith in a goofy state, so
    ** go no further. */
    ଫେରନ୍ତୁ 0;
  }
  ପ୍ରତିଜ୍ଞା( pFrom->fg.hadSchema==0 || pFrom->fg.notCte!=0 );
  ଯଦି( pFrom->fg.fixedSchema==0 && pFrom->u4.zDatabase!=0 ){
    /* The FROM term contains a schema qualifier (ex: main.t1) and so
    ** it cannot possibly be a CTE reference. */
    ଫେରନ୍ତୁ 0;
  }
  ଯଦି( pFrom->fg.notCte ){
    /* The FROM term is specifically excluded from matching a CTE.
    **   (1)  It is part of a trigger that used to have zDatabase but had
    **        zDatabase removed by sqlite3FixTriggerStep().
    **   (2)  This is the first term in the FROM clause of an UPDATE.
    */
    ଫେରନ୍ତୁ 0;
  }
  pCte = ସନ୍ଧାନ_With(pParse->pWith, pFrom, &pWith);
  ଯଦି( pCte ){
    sqlite3 *db = pParse->db;
    Table *pTab;
    ପଦସମଷ୍ଟି_List *pEList;
    Select *pSel;
    Select *p_ବାମ;                /* Left-most SELECT statement */
    Select *pRecTerm;             /* Left-most recursive term */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bMayRecursive;            /* True if compound joined by UNION [ALL] */
    With *pSavedWith;             /* Initial value of pParse->pWith */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRecTab = -1;             /* Cursor for recursive table */
    CteUse *pCteUse;

    /* If pCte->zCteErr is non-NULL at this point, then this is an illegal
    ** recursive reference to CTE pCte. Leave an error in pParse and return
    ** early. If pCte->zCteErr is NULL, then this is not a recursive reference.
    ** In this case, proceed.  */
    ଯଦି( pCte->zCteErr ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, pCte->zCteErr, pCte->zName);
      ଫେରନ୍ତୁ 2;
    }
    ଯଦି( cannotBeFunction(pParse, pFrom) ) ଫେରନ୍ତୁ 2;

    ପ୍ରତିଜ୍ଞା( pFrom->pSTab==0 );
    pTab = sqlite3DbMallocZero(db, ଆକାର(Table));
    ଯଦି( pTab==0 ) ଫେରନ୍ତୁ 2;
    pCteUse = pCte->pUse;
    ଯଦି( pCteUse==0 ){
      pCte->pUse = pCteUse = sqlite3DbMallocZero(db, ଆକାର(pCteUse[0]));
      ଯଦି( pCteUse==0
       || sqlite3ParserAddCleanup(pParse,sqlite3DbFree,pCteUse)==0
      ){
        sqlite3DbFree(db, pTab);
        ଫେରନ୍ତୁ 2;
      }
      pCteUse->eM10d = pCte->eM10d;
    }
    pFrom->pSTab = pTab;
    pTab->nTabRef = 1;
    pTab->zName = sqlite3DbStrDup(db, pCte->zName);
    pTab->iPKey = -1;
    pTab->nRowLogEst = 200; ପ୍ରତିଜ୍ଞା( 200==sqlite3LogEst(1048576) );
    pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
    sqlite3SrcItemAttachSubquery(pParse, pFrom, pCte->pSelect, 1);
    ଯଦି( db->mallocFailed ) ଫେରନ୍ତୁ 2;
    ପ୍ରତିଜ୍ଞା( pFrom->fg.isSubquery && pFrom->u4.pSubq );
    pSel = pFrom->u4.pSubq->pSelect;
    ପ୍ରତିଜ୍ଞା( pSel!=0 );
    pSel->selFlags |= SF_CopyCte;
    ଯଦି( pFrom->fg.isIndexedBy ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "no such index: \"%s\"", pFrom->u1.zIndexedBy);
      ଫେରନ୍ତୁ 2;
    }
    ପ୍ରତିଜ୍ଞା( !pFrom->fg.isIndexedBy );
    pFrom->fg.isCte = 1;
    pFrom->u2.pCteUse = pCteUse;
    pCteUse->nUse++;

    /* Check if this is a recursive CTE. */
    pRecTerm = pSel;
    bMayRecursive = ( pSel->op==TK_ALL || pSel->op==TK_UNION );
    ଯେତେବେଳେ( bMayRecursive && pRecTerm->op==pSel->op ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      SrcList *pSrc = pRecTerm->pSrc;
      ପ୍ରତିଜ୍ଞା( pRecTerm->pPrior!=0 );
      ପାଇଁ(i=0; i<pSrc->nSrc; i++){
        SrcItem *pItem = &pSrc->a[i];
        ଯଦି( pItem->zName!=0
         && !pItem->fg.hadSchema
         && ALWAYS( !pItem->fg.isSubquery )
         && (pItem->fg.fixedSchema || pItem->u4.zDatabase==0)
         && 0==sqlite3StrICmp(pItem->zName, pCte->zName)
        ){
          pItem->pSTab = pTab;
          pTab->nTabRef++;
          pItem->fg.isRecursive = 1;
          ଯଦି( pRecTerm->selFlags & SF_Recursive ){
            sqlite3_Error_ସନ୍ଦେଶ(pParse,
               "multiple references to recursive table: %s", pCte->zName
            );
            ଫେରନ୍ତୁ 2;
          }
          pRecTerm->selFlags |= SF_Recursive;
          ଯଦି( iRecTab<0 ) iRecTab = pParse->nTab++;
          pItem->iCursor = iRecTab;
        }
      }
      ଯଦି( (pRecTerm->selFlags & SF_Recursive)==0 ) ଭାଙ୍ଗ;
      pRecTerm = pRecTerm->pPrior;
    }

    ଯଦି( pParse->nTab >= db->aLimit[SQLITE_LIMIT_CTES]
     && db->aLimit[SQLITE_LIMIT_CTES]>0
    ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "too many CTEs");
      ଫେରନ୍ତୁ 2;
    }
#ଯଦି SQLITE_MAX_ପଦସମଷ୍ଟି_DEPTH>0
    pParse->nNestSel++;
    ଯଦି( checkSubqueryNestingDepth(pParse) ) ଫେରନ୍ତୁ 2;
#endif
    pCte->zCteErr = "circular reference: %s";
    pSavedWith = pParse->pWith;
    pParse->pWith = pWith;
    ଯଦି( pSel->selFlags & SF_Recursive ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
      ପ୍ରତିଜ୍ଞା( pRecTerm!=0 );
      ପ୍ରତିଜ୍ଞା( (pRecTerm->selFlags & SF_Recursive)==0 );
      ପ୍ରତିଜ୍ଞା( pRecTerm->pNext!=0 );
      ପ୍ରତିଜ୍ଞା( (pRecTerm->pNext->selFlags & SF_Recursive)!=0 );
      ପ୍ରତିଜ୍ଞା( pRecTerm->pWith==0 );
      pRecTerm->pWith = pSel->pWith;
      rc = sqlite3WalkSelect(pWalker, pRecTerm);
      pRecTerm->pWith = 0;
      ଯଦି( rc ){
        pParse->pWith = pSavedWith;
        ଫେରନ୍ତୁ 2;
      }
    }ଅନ୍ୟଥା{
      ଯଦି( selectExpanderWalk(pWalker, pSel) ){
        pParse->pWith = pSavedWith;
        ଫେରନ୍ତୁ 2;
      }
    }
    pParse->pWith = pWith;
#ଯଦି SQLITE_MAX_ପଦସମଷ୍ଟି_DEPTH>0
    pParse->nNestSel--;
    ପ୍ରତିଜ୍ଞା( pParse->nNestSel>=0 );
#endif

    ପାଇଁ(p_ବାମ=pSel; p_ବାମ->pPrior; p_ବାମ=p_ବାମ->pPrior);
    pEList = p_ବାମ->pEList;
    ଯଦି( pCte->pCols ){
      ଯଦି( pEList && pEList->n_ପଦସମଷ୍ଟି!=pCte->pCols->n_ପଦସମଷ୍ଟି ){
        sqlite3_Error_ସନ୍ଦେଶ(pParse, "table %s has %d values for %d columns",
            pCte->zName, pEList->n_ପଦସମଷ୍ଟି, pCte->pCols->n_ପଦସମଷ୍ଟି
        );
        pParse->pWith = pSavedWith;
        ଫେରନ୍ତୁ 2;
      }
      pEList = pCte->pCols;
    }

    sqlite3_Columns_From_ପଦସମଷ୍ଟି_List(pParse, pEList, &pTab->nCol, &pTab->aCol);
    ଯଦି( bMayRecursive ){
      ଯଦି( pSel->selFlags & SF_Recursive ){
        pCte->zCteErr = "multiple recursive references: %s";
      }ଅନ୍ୟଥା{
        pCte->zCteErr = "recursive reference in a subquery: %s";
      }
      sqlite3WalkSelect(pWalker, pSel);
    }
    pCte->zCteErr = 0;
    pParse->pWith = pSavedWith;
    ଫେରନ୍ତୁ 1;  /* Success */
  }
  ଫେରନ୍ତୁ 0;  /* No match */
}
#endif

#ifndef SQLITE_OMIT_CTE
/*
** If the SELECT passed as the second argument has an associated WITH
** clause, pop it from the stack stored as part of the Parse object.
**
** This function is used as the xSelectCallback2() callback by
** sqlite3SelectExpand() when walking a SELECT tree to resolve table
** names and other FROM clause elements.
*/
ଶୂନ୍ୟ sqlite3_Select_କାଢ଼_With(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  ଯଦି( OK_IF_ALWAYS_TRUE(pParse->pWith) && p->pPrior==0 ){
    With *pWith = findRightmost(p)->pWith;
    ଯଦି( pWith!=0 ){
      ପ୍ରତିଜ୍ଞା( pParse->pWith==pWith || pParse->nErr );
      pParse->pWith = pWith->pOuter;
    }
  }
}
#endif

/*
** The SrcItem structure passed as the second argument represents a
** sub-query in the FROM clause of a SELECT statement. This function
** allocates and populates the SrcItem.pTab object. If successful,
** SQLITE_OK is returned. Otherwise, if an OOM error is encountered,
** SQLITE_NOMEM.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ExpandSubquery(Parse *pParse, SrcItem *pFrom){
  Select *pSel;
  Table *pTab;

  ପ୍ରତିଜ୍ଞା( pFrom->fg.isSubquery );
  ପ୍ରତିଜ୍ଞା( pFrom->u4.pSubq!=0 );
  pSel = pFrom->u4.pSubq->pSelect;
  ପ୍ରତିଜ୍ଞା( pSel );
  pFrom->pSTab = pTab = sqlite3DbMallocZero(pParse->db, ଆକାର(Table));
  ଯଦି( pTab==0 ) ଫେରନ୍ତୁ SQLITE_NOMEM;
  pTab->nTabRef = 1;
  ଯଦି( pFrom->zAlias ){
    pTab->zName = sqlite3DbStrDup(pParse->db, pFrom->zAlias);
  }ଅନ୍ୟଥା{
    pTab->zName = sqlite3MPrintf(pParse->db, "%!S", pFrom);
  }
  ଯେତେବେଳେ( pSel->pPrior ){ pSel = pSel->pPrior; }
  sqlite3_Columns_From_ପଦସମଷ୍ଟି_List(pParse, pSel->pEList,&pTab->nCol,&pTab->aCol);
  pTab->iPKey = -1;
  pTab->eTabType = TABTYP_VIEW;
  pTab->nRowLogEst = 200; ପ୍ରତିଜ୍ଞା( 200==sqlite3LogEst(1048576) );
#ifndef SQLITE_ALLOW_ROWID_IN_VIEW
  /* The usual case - do not allow ROWID on a subquery */
  pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
#ଅନ୍ୟଥା
  /* Legacy compatibility mode */
  pTab->tabFlags |= TF_Ephemeral | sqlite3Config.mNoVisibleRowid;
#endif
  ଫେରନ୍ତୁ pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା inAnyUsingClause(
  ଅଚଳ ଅକ୍ଷର *zName, /* Name we are looking for */
  SrcItem *pBase,    /* The base SrcItem.  Looking at pBase[1] and following */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା N              /* How many SrcItems to check */
){
  ଯେତେବେଳେ( N>0 ){
    N--;
    pBase++;
    ଯଦି( pBase->fg.isUsing==0 ) ଜାରି;
    ଯଦି( NEVER(pBase->u3.pUsing==0) ) ଜାରି;
    ଯଦି( sqlite3IdListIndex(pBase->u3.pUsing, zName)>=0 ) ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା selectExpander(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j, k, rc;
  SrcList *pTabList;
  ପଦସମଷ୍ଟି_List *pEList;
  SrcItem *pFrom;
  sqlite3 *db = pParse->db;
  ପଦସମଷ୍ଟି *pE, *p_ଡାହାଣ, *p_ପଦସମଷ୍ଟି;
  u16 selFlags = p->selFlags;
  u32 elistFlags = 0;

  p->selFlags |= SF_Expanded;
  ଯଦି( db->mallocFailed  ){
    ଫେରନ୍ତୁ WRC_Abort;
  }
  ପ୍ରତିଜ୍ଞା( p->pSrc!=0 );
  ଯଦି( (selFlags & SF_Expanded)!=0 ){
    ଫେରନ୍ତୁ WRC_Prune;
  }
  ଯଦି( pWalker->eCode ){
    /* Renumber selId because it has been copied from a view */
    p->selId = ++pParse->nSelect;
  }
  pTabList = p->pSrc;
  pEList = p->pEList;
  ଯଦି( pParse->pWith && (p->selFlags & SF_View) ){
    ଯଦି( p->pWith==0 ){
      p->pWith = (With*)sqlite3DbMallocZero(db, SZ_WITH(1) );
      ଯଦି( p->pWith==0 ){
        ଫେରନ୍ତୁ WRC_Abort;
      }
    }
    p->pWith->bView = 1;
  }
  sqlite3_With_ଠେଲ(pParse, p->pWith, 0);

  /* Make sure cursor numbers have been assigned to all entries in
  ** the FROM clause of the SELECT statement.
  */
  sqlite3SrcListAssignCursors(pParse, pTabList);

  /* Look up every table named in the FROM clause of the select.  If
  ** an entry of the FROM clause is a subquery instead of a table or view,
  ** then create a transient table structure to describe the subquery.
  */
  ପାଇଁ(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    Table *pTab;
    ପ୍ରତିଜ୍ଞା( pFrom->fg.isRecursive==0 || pFrom->pSTab!=0 );
    ଯଦି( pFrom->pSTab ) ଜାରି;
    ପ୍ରତିଜ୍ଞା( pFrom->fg.isRecursive==0 );
    ଯଦି( pFrom->zName==0 ){
#ifndef SQLITE_OMIT_SUBQUERY
      Select *pSel;
      ପ୍ରତିଜ୍ଞା( pFrom->fg.isSubquery && pFrom->u4.pSubq!=0 );
      pSel = pFrom->u4.pSubq->pSelect;
      /* A sub-query in the FROM clause of a SELECT */
      ପ୍ରତିଜ୍ଞା( pSel!=0 );
      ପ୍ରତିଜ୍ଞା( pFrom->pSTab==0 );
      ଯଦି( sqlite3WalkSelect(pWalker, pSel) ) ଫେରନ୍ତୁ WRC_Abort;
      ଯଦି( sqlite3ExpandSubquery(pParse, pFrom) ) ଫେରନ୍ତୁ WRC_Abort;
#endif
#ifndef SQLITE_OMIT_CTE
    }ଅନ୍ୟଥା ଯଦି( (rc = resolveFromTermToCte(pParse, pWalker, pFrom))!=0 ){
      ଯଦି( rc>1 ) ଫେରନ୍ତୁ WRC_Abort;
      pTab = pFrom->pSTab;
      ପ୍ରତିଜ୍ଞା( pTab!=0 );
#endif
    }ଅନ୍ୟଥା{
      /* An ordinary table or view name in the FROM clause */
      ପ୍ରତିଜ୍ଞା( pFrom->pSTab==0 );
      pFrom->pSTab = pTab = sqlite3LocateTableItem(pParse, 0, pFrom);
      ଯଦି( pTab==0 ) ଫେରନ୍ତୁ WRC_Abort;
      ଯଦି( pTab->nTabRef>=0xffff ){
        sqlite3_Error_ସନ୍ଦେଶ(pParse, "too many references to \"%s\": max 65535",
           pTab->zName);
        pFrom->pSTab = 0;
        ଫେରନ୍ତୁ WRC_Abort;
      }
      pTab->nTabRef++;
      ଯଦି( !IsVirtual(pTab) && cannotBeFunction(pParse, pFrom) ){
        ଫେରନ୍ତୁ WRC_Abort;
      }
#ଯଦି !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
      ଯଦି( !IsOrdinaryTable(pTab) ){
        i16 nCol;
        u8 eCodeOrig = pWalker->eCode;
        ଯଦି( sqlite3ViewGetColumnNames(pParse, pTab) ) ଫେରନ୍ତୁ WRC_Abort;
        ପ୍ରତିଜ୍ଞା( pFrom->fg.isSubquery==0 );
        ଯଦି( IsView(pTab) ){
          ଯଦି( (db->flags & SQLITE_EnableView)==0
           && pTab->pSchema!=db->aDb[1].pSchema
          ){
            sqlite3_Error_ସନ୍ଦେଶ(pParse, "access to view \"%s\" prohibited",
              pTab->zName);
          }
          sqlite3SrcItemAttachSubquery(pParse, pFrom, pTab->u.view.pSelect, 1);
        }
#ifndef SQLITE_OMIT_VIRTUALTABLE
        ଅନ୍ୟଥା ଯଦି( ALWAYS(IsVirtual(pTab))
         && (pFrom->fg.fromDDL || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
         && ALWAYS(pTab->u.vtab.p!=0)
         && pTab->u.vtab.p->eVtabRisk > ((db->flags & SQLITE_TrustedSchema)!=0)
        ){
          sqlite3_Error_ସନ୍ଦେଶ(pParse, "unsafe use of virtual table \"%s\"",
                                  pTab->zName);
        }
        ପ୍ରତିଜ୍ଞା( SQLITE_VTABRISK_Normal==1 && SQLITE_VTABRISK_ଉପର==2 );
#endif
        nCol = pTab->nCol;
        pTab->nCol = -1;
        pWalker->eCode = 1;  /* Turn on Select.selId renumbering */
        ଯଦି( pFrom->fg.isSubquery ){
          bft hasCompound = pParse->hasCompound;
          pParse->hasCompound = 1;
          selectExpanderWalk(pWalker, pFrom->u4.pSubq->pSelect);
          pParse->hasCompound = hasCompound;
        }
        pWalker->eCode = eCodeOrig;
        pTab->nCol = nCol;
      }
#endif
    }

    /* Locate the index named by the INDEXED BY clause, if any. */
    ଯଦି( pFrom->fg.isIndexedBy && sqlite3IndexedByLookup(pParse, pFrom) ){
      ଫେରନ୍ତୁ WRC_Abort;
    }
  }

  /* Process NATURAL keywords, and ON and USING clauses of joins.
  */
  ପ୍ରତିଜ୍ଞା( db->mallocFailed==0 || pParse->nErr!=0 );
  ଯଦି( pParse->nErr || sqlite3ProcessJoin(pParse, p) ){
    ଫେରନ୍ତୁ WRC_Abort;
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
  ପାଇଁ(k=0; k<pEList->n_ପଦସମଷ୍ଟି; k++){
    pE = pEList->a[k].p_ପଦସମଷ୍ଟି;
    ଯଦି( pE->op==TK_ASTERISK ) ଭାଙ୍ଗ;
    ପ୍ରତିଜ୍ଞା( pE->op!=TK_DOT || pE->p_ଡାହାଣ!=0 );
    ପ୍ରତିଜ୍ଞା( pE->op!=TK_DOT || (pE->p_ବାମ!=0 && pE->p_ବାମ->op==TK_ID) );
    ଯଦି( pE->op==TK_DOT && pE->p_ଡାହାଣ->op==TK_ASTERISK ) ଭାଙ୍ଗ;
    elistFlags |= pE->flags;
  }
  ଯଦି( k<pEList->n_ପଦସମଷ୍ଟି ){
    /*
    ** If we get here it means the result set contains one or more "*"
    ** operators that need to be expanded.  Loop through each expression
    ** in the result set and expand them one by one.
    */
    ସଂରଚନା ପଦସମଷ୍ଟି_List_item *a = pEList->a;
    ପଦସମଷ୍ଟି_List *pNew = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = pParse->db->flags;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା longNames = (flags & SQLITE_FullColNames)!=0
                      && (flags & SQLITE_ShortColNames)==0;

    ପାଇଁ(k=0; k<pEList->n_ପଦସମଷ୍ଟି; k++){
      pE = a[k].p_ପଦସମଷ୍ଟି;
      elistFlags |= pE->flags;
      p_ଡାହାଣ = pE->p_ଡାହାଣ;
      ପ୍ରତିଜ୍ଞା( pE->op!=TK_DOT || p_ଡାହାଣ!=0 );
      ଯଦି( pE->op!=TK_ASTERISK
       && (pE->op!=TK_DOT || p_ଡାହାଣ->op!=TK_ASTERISK)
      ){
        /* This particular expression does not need to be expanded.
        */
        pNew = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, pNew, a[k].p_ପଦସମଷ୍ଟି);
        ଯଦି( pNew ){
          pNew->a[pNew->n_ପଦସମଷ୍ଟି-1].zEName = a[k].zEName;
          pNew->a[pNew->n_ପଦସମଷ୍ଟି-1].fg.eEName = a[k].fg.eEName;
          a[k].zEName = 0;
        }
        a[k].p_ପଦସମଷ୍ଟି = 0;
      }ଅନ୍ୟଥା{
        /* This expression is a "*" or a "TABLE.*" and needs to be
        ** expanded. */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା tableSeen = 0;      /* Set to 1 when TABLE matches */
        ଅକ୍ଷର *zTName = 0;       /* text of name of TABLE */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iErrOfst;
        ଯଦି( pE->op==TK_DOT ){
          ପ୍ରତିଜ୍ଞା( (selFlags & SF_NestedFrom)==0 );
          ପ୍ରତିଜ୍ଞା( pE->p_ବାମ!=0 );
          ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(pE->p_ବାମ, EP_Int_ମୂଲ୍ୟ) );
          zTName = pE->p_ବାମ->u.zToken;
          ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_WOfst(pE->p_ବାମ) );
          iErrOfst = pE->p_ଡାହାଣ->w.iOfst;
        }ଅନ୍ୟଥା{
          ପ୍ରତିଜ୍ଞା( pE->p_ବାମ==0 );
          ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_WOfst(pE) );
          iErrOfst = pE->w.iOfst;
        }
        ପାଇଁ(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା nAdd;                    /* Number of cols including rowid */
          Table *pTab = pFrom->pSTab;  /* Table for this data source */
          ପଦସମଷ୍ଟି_List *pNestedFrom;       /* Result-set of a nested FROM clause */
          ଅକ୍ଷର *zTabName;              /* AS name for this data source */
          ଅଚଳ ଅକ୍ଷର *zSchemaName = 0; /* Schema name for this data source */
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb;                     /* Schema index for this data src */
          IdList *pUsing;              /* USING clause for pFrom[1] */

          ଯଦି( (zTabName = pFrom->zAlias)==0 ){
            zTabName = pTab->zName;
          }
          ଯଦି( db->mallocFailed ) ଭାଙ୍ଗ;
          ପ୍ରତିଜ୍ଞା( (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pFrom->fg.isNestedFrom == IsNestedFrom(pFrom) );
          ଯଦି( pFrom->fg.isNestedFrom ){
            ପ୍ରତିଜ୍ଞା( pFrom->fg.isSubquery && pFrom->u4.pSubq );
            ପ୍ରତିଜ୍ଞା( pFrom->u4.pSubq->pSelect!=0 );
            pNestedFrom = pFrom->u4.pSubq->pSelect->pEList;
            ପ୍ରତିଜ୍ଞା( pNestedFrom!=0 );
            ପ୍ରତିଜ୍ଞା( pNestedFrom->n_ପଦସମଷ୍ଟି==pTab->nCol );
            ପ୍ରତିଜ୍ଞା( VisibleRowid(pTab)==0 || ViewCanHaveRowid );
          }ଅନ୍ୟଥା{
            ଯଦି( zTName && sqlite3StrICmp(zTName, zTabName)!=0 ){
              ଜାରି;
            }
            pNestedFrom = 0;
            iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
            zSchemaName = iDb>=0 ? db->aDb[iDb].zDbSName : "*";
          }
          ଯଦି( i+1<pTabList->nSrc
           && pFrom[1].fg.isUsing
           && (selFlags & SF_NestedFrom)!=0
          ){
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
            pUsing = pFrom[1].u3.pUsing;
            ପାଇଁ(ii=0; ii<pUsing->nId; ii++){
              ଅଚଳ ଅକ୍ଷର *zUName = pUsing->a[ii].zName;
              p_ଡାହାଣ = sqlite3_ପଦସମଷ୍ଟି(db, TK_ID, zUName);
              sqlite3_ପଦସମଷ୍ଟି_Set_Error_Offset(p_ଡାହାଣ, iErrOfst);
              pNew = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, pNew, p_ଡାହାଣ);
              ଯଦି( pNew ){
                ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pX = &pNew->a[pNew->n_ପଦସମଷ୍ଟି-1];
                ପ୍ରତିଜ୍ଞା( pX->zEName==0 );
                pX->zEName = sqlite3MPrintf(db,"..%s", zUName);
                pX->fg.eEName = ENAME_TAB;
                pX->fg.bUsingTerm = 1;
              }
            }
          }ଅନ୍ୟଥା{
            pUsing = 0;
          }

          nAdd = pTab->nCol;
          ଯଦି( VisibleRowid(pTab) && (selFlags & SF_NestedFrom)!=0 ) nAdd++;
          ପାଇଁ(j=0; j<nAdd; j++){
            ଅଚଳ ଅକ୍ଷର *zName; 
            ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pX; /* Newly added ExprList term */

            ଯଦି( j==pTab->nCol ){
              zName = sqlite3RowidAlias(pTab);
              ଯଦି( zName==0 ) ଜାରି;
            }ଅନ୍ୟଥା{
              zName = pTab->aCol[j].zCnName;

              /* If pTab is actually an SF_NestedFrom sub-select, do not
              ** expand any ENAME_ROWID columns.  */
              ଯଦି( pNestedFrom && pNestedFrom->a[j].fg.eEName==ENAME_ROWID ){
                ଜାରି;
              }

              ଯଦି( zTName
               && pNestedFrom
               && sqlite3MatchEName(&pNestedFrom->a[j], 0, zTName, 0, 0)==0
              ){
                ଜାରି;
              }

              /* If a column is marked as 'hidden', omit it from the expanded
              ** result-set list unless the SELECT has the SF_IncludeHidden
              ** bit set.
              */
              ଯଦି( (p->selFlags & SF_IncludeHidden)==0
                && IsHiddenColumn(&pTab->aCol[j])
              ){
                ଜାରି;
              }
              ଯଦି( (pTab->aCol[j].colFlags & COLFLAG_NOEXPAND)!=0
               && zTName==0
               && (selFlags & (SF_NestedFrom))==0
              ){
                ଜାରି;
              }
            }
            ପ୍ରତିଜ୍ଞା( zName );
            tableSeen = 1;

            ଯଦି( i>0 && zTName==0 && (selFlags & SF_NestedFrom)==0 ){
              ଯଦି( pFrom->fg.isUsing
               && sqlite3IdListIndex(pFrom->u3.pUsing, zName)>=0
              ){
                /* In a join with a USING clause, omit columns in the
                ** using clause from the table on the right. */
                ଜାରି;
              }
            }
            p_ଡାହାଣ = sqlite3_ପଦସମଷ୍ଟି(db, TK_ID, zName);
            ଯଦି( (pTabList->nSrc>1
                 && (  (pFrom->fg.jointype & JT_LTORJ)==0
                     || zTName!=0
                     || (selFlags & SF_NestedFrom)!=0
                     || !inAnyUsingClause(zName,pFrom,pTabList->nSrc-i-1)
                    )
                )
             || IN_RENAME_OBJECT
            ){
              ପଦସମଷ୍ଟି *p_ବାମ;
              p_ବାମ = sqlite3_ପଦସମଷ୍ଟି(db, TK_ID, zTabName);
              p_ପଦସମଷ୍ଟି = sqlite3PExpr(pParse, TK_DOT, p_ବାମ, p_ଡାହାଣ);
              ଯଦି( IN_RENAME_OBJECT && pE->p_ବାମ ){
                ପ୍ରତିଜ୍ଞା( pE->op==TK_DOT && pE->p_ଡାହାଣ->op==TK_ASTERISK );
                sqlite3RenameTokenRemap(pParse, p_ବାମ, pE->p_ବାମ);
              }
              ଯଦି( zSchemaName ){
                p_ବାମ = sqlite3_ପଦସମଷ୍ଟି(db, TK_ID, zSchemaName);
                p_ପଦସମଷ୍ଟି = sqlite3PExpr(pParse, TK_DOT, p_ବାମ, p_ପଦସମଷ୍ଟି);
              }
            }ଅନ୍ୟଥା{
              p_ପଦସମଷ୍ଟି = p_ଡାହାଣ;
            }
            sqlite3_ପଦସମଷ୍ଟି_Set_Error_Offset(p_ପଦସମଷ୍ଟି, iErrOfst);
            pNew = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, pNew, p_ପଦସମଷ୍ଟି);
            ଯଦି( pNew==0 ){
              ଭାଙ୍ଗ;  /* OOM */
            }
            pX = &pNew->a[pNew->n_ପଦସମଷ୍ଟି-1];
            ପ୍ରତିଜ୍ଞା( pX->zEName==0 );
            ଯଦି( (selFlags & SF_NestedFrom)!=0 && !IN_RENAME_OBJECT ){
              ଯଦି( pNestedFrom && (!ViewCanHaveRowid || j<pNestedFrom->n_ପଦସମଷ୍ଟି) ){
                ପ୍ରତିଜ୍ଞା( j<pNestedFrom->n_ପଦସମଷ୍ଟି );
                pX->zEName = sqlite3DbStrDup(db, pNestedFrom->a[j].zEName);
                testcase( pX->zEName==0 );
              }ଅନ୍ୟଥା{
                pX->zEName = sqlite3MPrintf(db, "%s.%s.%s",
                                           zSchemaName, zTabName, zName);
                testcase( pX->zEName==0 );
              }
              pX->fg.eEName = (j==pTab->nCol ? ENAME_ROWID : ENAME_TAB);
              ଯଦି( (pFrom->fg.isUsing
                   && sqlite3IdListIndex(pFrom->u3.pUsing, zName)>=0)
               || (pUsing && sqlite3IdListIndex(pUsing, zName)>=0)
               || (j<pTab->nCol && (pTab->aCol[j].colFlags & COLFLAG_NOEXPAND))
              ){
                pX->fg.bNoExpand = 1;
              }
            }ଅନ୍ୟଥା ଯଦି( longNames ){
              pX->zEName = sqlite3MPrintf(db, "%s.%s", zTabName, zName);
              pX->fg.eEName = ENAME_NAME;
            }ଅନ୍ୟଥା{
              pX->zEName = sqlite3DbStrDup(db, zName);
              pX->fg.eEName = ENAME_NAME;
            }
          }
        }
        ଯଦି( !tableSeen ){
          ଯଦି( zTName ){
            sqlite3_Error_ସନ୍ଦେଶ(pParse, "no such table: %s", zTName);
          }ଅନ୍ୟଥା{
            sqlite3_Error_ସନ୍ଦେଶ(pParse, "no tables specified");
          }
        }
      }
    }
    sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pEList);
    p->pEList = pNew;
  }
  ଯଦି( p->pEList ){
    ଯଦି( p->pEList->n_ପଦସମଷ୍ଟି>db->aLimit[SQLITE_LIMIT_COLUMN] ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "too many columns in result set");
      ଫେରନ୍ତୁ WRC_Abort;
    }
    ଯଦି( (elistFlags & (EP_HasFunc|EP_Subquery))!=0 ){
      p->selFlags |= SF_ComplexResult;
    }
  }
#ଯଦି TREETRACE_ENABLED
  ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x8 ){
    TREETRACE(0x8,pParse,p,("After result-set wildcard expansion:\n"));
    sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
  }
#endif
  ଫେରନ୍ତୁ WRC_Continue;
}

#ଯଦି SQLITE_DEBUG
/*
** Always assert.  This xSelectCallback2 implementation proves that the
** xSelectCallback2 is never invoked.
*/
ଶୂନ୍ୟ sqlite3SelectWalkAssert2(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ପ୍ରତିଜ୍ଞା( 0 );
}
#endif

/*
** The first argument is a Walker object created by sqlite3SelectExpand().
** This function uses the walker to expand the SELECT statement passed
** as the second argument. If necessary, it also converts any compound
** SELECT statements that have COLLATE clauses on ORDER BY expressions
** to sub-queries. Return the return value of sqlite3WalkSelect().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା selectExpanderWalk(Walker *pWalker, Select *pSelect){
  ପ୍ରତିଜ୍ଞା( pWalker->x_ପଦସମଷ୍ଟି_Callback==sqlite3_ପଦସମଷ୍ଟି_Walk_Noop );
  ପ୍ରତିଜ୍ଞା( pWalker->pParse!=0 );
  ଯଦି( OK_IF_ALWAYS_TRUE(pWalker->pParse->hasCompound) ){
    pWalker->xSelectCallback = convertCompoundSelectToSubquery;
    pWalker->xSelectCallback2 = 0;
    sqlite3WalkSelect(pWalker, pSelect);
  }
  pWalker->xSelectCallback = selectExpander;
  pWalker->xSelectCallback2 = sqlite3_Select_କାଢ଼_With;
  ଫେରନ୍ତୁ sqlite3WalkSelect(pWalker, pSelect);
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
ସ୍ଥିର ଶୂନ୍ୟ sqlite3SelectExpand(Parse *pParse, Select *pSelect){
  Walker w;
  w.x_ପଦସମଷ୍ଟି_Callback = sqlite3_ପଦସମଷ୍ଟି_Walk_Noop;
  w.pParse = pParse;
  w.eCode = 0;
  selectExpanderWalk(&w, pSelect);
}


#ifndef SQLITE_OMIT_SUBQUERY
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
ସ୍ଥିର ଶୂନ୍ୟ selectAddSubqueryTypeInfo(Walker *pWalker, Select *p){
  Parse *pParse;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  SrcList *pTabList;
  SrcItem *pFrom;

  ଯଦି( p->selFlags & SF_HasTypeInfo ) ଫେରନ୍ତୁ;
  p->selFlags |= SF_HasTypeInfo;
  pParse = pWalker->pParse;
  ପ୍ରତିଜ୍ଞା( (p->selFlags & SF_Resolved) );
  pTabList = p->pSrc;
  ପାଇଁ(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    Table *pTab = pFrom->pSTab;
    ପ୍ରତିଜ୍ଞା( pTab!=0 );
    ଯଦି( (pTab->tabFlags & TF_Ephemeral)!=0 && pFrom->fg.isSubquery ){
      /* A sub-query in the FROM clause of a SELECT */
      Select *pSel = pFrom->u4.pSubq->pSelect;
      sqlite3SubqueryColumnTypes(pParse, pTab, pSel, SQLITE_AFF_NONE);
    }
  }
}
#endif


/*
** This routine adds datatype and collating sequence information to
** the Table structures of all FROM-clause subqueries in a
** SELECT statement.
**
** Use this routine after name resolution.
*/
ସ୍ଥିର ଶୂନ୍ୟ sqlite3SelectAddTypeInfo(Parse *pParse, Select *pSelect){
#ifndef SQLITE_OMIT_SUBQUERY
  Walker w;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.xSelectCallback2 = selectAddSubqueryTypeInfo;
  w.x_ପଦସମଷ୍ଟି_Callback = sqlite3_ପଦସମଷ୍ଟି_Walk_Noop;
  w.pParse = pParse;
  sqlite3WalkSelect(&w, pSelect);
#endif
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
ଶୂନ୍ୟ sqlite3SelectPrep(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for container */
){
  ପ୍ରତିଜ୍ଞା( p!=0 || pParse->db->mallocFailed );
  ପ୍ରତିଜ୍ଞା( pParse->db->pParse==pParse );
  ଯଦି( pParse->db->mallocFailed ) ଫେରନ୍ତୁ;
  ଯଦି( p->selFlags & SF_HasTypeInfo ) ଫେରନ୍ତୁ;
  sqlite3SelectExpand(pParse, p);
  ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ;
  sqlite3ResolveSelectNames(pParse, p, pOuterNC);
  ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ;
  sqlite3SelectAddTypeInfo(pParse, p);
}

#ଯଦି TREETRACE_ENABLED
/*
** Display all information about an AggInfo object
*/
ସ୍ଥିର ଶୂନ୍ୟ printAggInfo(AggInfo *pAggInfo){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  sqlite3DebugPrintf("AggInfo %d/%p:\n",
     pAggInfo->selId, pAggInfo);
  ପାଇଁ(ii=0; ii<pAggInfo->nColumn; ii++){
    ସଂରଚନା AggInfo_col *pCol = &pAggInfo->aCol[ii];
    sqlite3DebugPrintf(
       "agg-column[%d] pTab=%s iTable=%d iColumn=%d iMem=%d"
       " iSorterColumn=%d %s\n",
       ii, pCol->pTab ? pCol->pTab->zName : "NULL",
       pCol->iTable, pCol->iColumn, pAggInfo->iFirstReg+ii,
       pCol->iSorterColumn,
       ii>=pAggInfo->nAccumulator ? "" : " Accumulator");
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(0, pAggInfo->aCol[ii].pCExpr, 0);
  }
  ପାଇଁ(ii=0; ii<pAggInfo->nFunc; ii++){
    sqlite3DebugPrintf("agg-func[%d]: iMem=%d\n",
        ii, pAggInfo->iFirstReg+pAggInfo->nColumn+ii);
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(0, pAggInfo->aFunc[ii].pFExpr, 0);
  }
}
#endif /* TREETRACE_ENABLED */

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
ସ୍ଥିର ଶୂନ୍ୟ analyzeAggFuncArgs(
  AggInfo *pAggInfo,
  NameContext *pNC
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପ୍ରତିଜ୍ଞା( pAggInfo!=0 );
  ପ୍ରତିଜ୍ଞା( pAggInfo->iFirstReg==0 );
  pNC->ncFlags |= NC_InAggFunc;
  ପାଇଁ(i=0; i<pAggInfo->nFunc; i++){
    ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି = pAggInfo->aFunc[i].pFExpr;
    ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_FUNCTION || p_ପଦସମଷ୍ଟି->op==TK_AGG_FUNCTION );
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
    sqlite3_ପଦସମଷ୍ଟି_Analyze_Agg_List(pNC, p_ପଦସମଷ୍ଟି->x.pList);
    ଯଦି( p_ପଦସମଷ୍ଟି->p_ବାମ ){
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ବାମ->op==TK_ORDER );
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି->p_ବାମ) );
      sqlite3_ପଦସମଷ୍ଟି_Analyze_Agg_List(pNC, p_ପଦସମଷ୍ଟି->p_ବାମ->x.pList);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    ପ୍ରତିଜ୍ଞା( !IsWindowFunc(p_ପଦସମଷ୍ଟି) );
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) ){
      sqlite3_ପଦସମଷ୍ଟି_Analyze_Aggregates(pNC, p_ପଦସମଷ୍ଟି->y.pWin->pFilter);
    }
#endif
  }
  pNC->ncFlags &= ~NC_InAggFunc;
}

/*
** An index on expressions is being used in the inner loop of an
** aggregate query with a GROUP BY clause.  This routine attempts
** to adjust the AggInfo object to take advantage of index and to
** perhaps use the index as a covering index.
**
*/
ସ୍ଥିର ଶୂନ୍ୟ optimize_Aggregate_Use_Of_Indexed_ପଦସମଷ୍ଟି(
  Parse *pParse,          /* Parsing context */
  Select *pSelect,        /* The SELECT statement being processed */
  AggInfo *pAggInfo,      /* The aggregate info */
  NameContext *pNC        /* Name context used to resolve agg-func args */
){
  ପ୍ରତିଜ୍ଞା( pAggInfo->iFirstReg==0 );
  ପ୍ରତିଜ୍ଞା( pSelect!=0 );
  ପ୍ରତିଜ୍ଞା( pSelect->pGroupBy!=0 );
  pAggInfo->nColumn = pAggInfo->nAccumulator;
  ଯଦି( ALWAYS(pAggInfo->nSortingColumn>0) ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା mx = pSelect->pGroupBy->n_ପଦସମଷ୍ଟି - 1;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା j, k;
    ପାଇଁ(j=0; j<pAggInfo->nColumn; j++){
      k = pAggInfo->aCol[j].iSorterColumn;
      ଯଦି( k>mx ) mx = k;
    }
    pAggInfo->nSortingColumn = mx+1;
  }
  analyzeAggFuncArgs(pAggInfo, pNC);
#ଯଦି TREETRACE_ENABLED
  ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x20 ){
    Indexed_ପଦସମଷ୍ଟି *pIEpr;
    TREETRACE(0x20, pParse, pSelect,
        ("AggInfo (possibly) adjusted for Indexed Exprs\n"));
    sqlite3_ବୃକ୍ଷ_View_Select(0, pSelect, 0);
    ପାଇଁ(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
      printf("data-cursor=%d index={%d,%d}\n",
          pIEpr->iDataCur, pIEpr->iIdxCur, pIEpr->iIdxCol);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(0, pIEpr->p_ପଦସମଷ୍ଟି, 0);
    }
    printAggInfo(pAggInfo);
  }
#ଅନ୍ୟଥା
  UNUSED_PARAMETER(pSelect);
  UNUSED_PARAMETER(pParse);
#endif
}

/*
** Walker callback for aggregateConvertIndexedExprRefToColumn().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା aggregateIdxEprRefToColCallback(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  AggInfo *pAggInfo;
  ସଂରଚନା AggInfo_col *pCol;
  UNUSED_PARAMETER(pWalker);
  ଯଦି( p_ପଦସମଷ୍ଟି->pAggInfo==0 ) ଫେରନ୍ତୁ WRC_Continue;
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_AGG_COLUMN ) ଫେରନ୍ତୁ WRC_Continue;
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_AGG_FUNCTION ) ଫେରନ୍ତୁ WRC_Continue;
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_IF_NULL_ROW ) ଫେରନ୍ତୁ WRC_Continue;
  pAggInfo = p_ପଦସମଷ୍ଟି->pAggInfo;
  ଯଦି( NEVER(p_ପଦସମଷ୍ଟି->iAgg>=pAggInfo->nColumn) ) ଫେରନ୍ତୁ WRC_Continue;
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->iAgg>=0 );
  pCol = &pAggInfo->aCol[p_ପଦସମଷ୍ଟି->iAgg];
  p_ପଦସମଷ୍ଟି->op = TK_AGG_COLUMN;
  p_ପଦସମଷ୍ଟି->iTable = pCol->iTable;
  p_ପଦସମଷ୍ଟି->iColumn = pCol->iColumn;
  ପଦସମଷ୍ଟି_Clear_Property(p_ପଦସମଷ୍ଟି, EP_Skip|EP_Collate|EP_Unlikely);
  ଫେରନ୍ତୁ WRC_Prune;
}

/*
** Convert every pAggInfo->aFunc[].pExpr such that any node within
** those expressions that has pAppInfo set is changed into a TK_AGG_COLUMN
** opcode.
*/
ସ୍ଥିର ଶୂନ୍ୟ aggregate_Convert_Indexed_ପଦସମଷ୍ଟି_Ref_To_Column(AggInfo *pAggInfo){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  Walker w;
  memset(&w, 0, ଆକାର(w));
  w.x_ପଦସମଷ୍ଟି_Callback = aggregateIdxEprRefToColCallback;
  ପାଇଁ(i=0; i<pAggInfo->nFunc; i++){
    sqlite3_Walk_ପଦସମଷ୍ଟି(&w, pAggInfo->aFunc[i].pFExpr);
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
ସ୍ଥିର ଶୂନ୍ୟ assignAggregateRegisters(Parse *pParse, AggInfo *pAggInfo){
  ପ୍ରତିଜ୍ଞା( pAggInfo!=0 );
  ପ୍ରତିଜ୍ଞା( pAggInfo->iFirstReg==0 );
  pAggInfo->iFirstReg = pParse->nMem + 1;
  pParse->nMem += pAggInfo->nColumn + pAggInfo->nFunc;
}

/*
** Reset the aggregate accumulator.
**
** The aggregate accumulator is a set of memory cells that hold
** intermediate results while calculating an aggregate.  This
** routine generates code that stores NULLs in all of those memory
** cells.
*/
ସ୍ଥିର ଶୂନ୍ୟ resetAccumulator(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ସଂରଚନା AggInfo_func *pFunc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nReg = pAggInfo->nFunc + pAggInfo->nColumn;
  ପ୍ରତିଜ୍ଞା( pAggInfo->iFirstReg>0 );
  ପ୍ରତିଜ୍ଞା( pParse->db->pParse==pParse );
  ପ୍ରତିଜ୍ଞା( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ଯଦି( nReg==0 ) ଫେରନ୍ତୁ;
  ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ;
  sqlite3VdbeAddOp3(v, OP_Null, 0, pAggInfo->iFirstReg,
                    pAggInfo->iFirstReg+nReg-1);
  ପାଇଁ(pFunc=pAggInfo->aFunc, i=0; i<pAggInfo->nFunc; i++, pFunc++){
    ଯଦି( pFunc->iDistinct>=0 ){
      ପଦସମଷ୍ଟି *pE = pFunc->pFExpr;
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pE) );
      ଯଦି( pE->x.pList==0 || pE->x.pList->n_ପଦସମଷ୍ଟି!=1 ){
        sqlite3_Error_ସନ୍ଦେଶ(pParse, "DISTINCT aggregates must have exactly one "
           "argument");
        pFunc->iDistinct = -1;
      }ଅନ୍ୟଥା{
        ଚାବି_Info *p_ଚାବି_Info = sqlite3_ଚାବି_Info_From_ପଦସମଷ୍ଟି_List(pParse, pE->x.pList,0,0);
        pFunc->iDistAddr = sqlite3VdbeAddOp4(v, OP_OpenEphemeral,
            pFunc->iDistinct, 0, 0, (ଅକ୍ଷର*)p_ଚାବି_Info, P4_KEYINFO);
        ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(DISTINCT)",
                          pFunc->pFunc->zName));
      }
    }
    ଯଦି( pFunc->iOBTab>=0 ){
      ପଦସମଷ୍ଟି_List *pOBList;
      ଚାବି_Info *p_ଚାବି_Info;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nExtra = 0;
      ପ୍ରତିଜ୍ଞା( pFunc->pFExpr->p_ବାମ!=0 );
      ପ୍ରତିଜ୍ଞା( pFunc->pFExpr->p_ବାମ->op==TK_ORDER );
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pFunc->pFExpr->p_ବାମ) );
      ପ୍ରତିଜ୍ଞା( pFunc->pFunc!=0 );
      pOBList = pFunc->pFExpr->p_ବାମ->x.pList;
      ଯଦି( !pFunc->bOBUnique ){
        nExtra++;  /* One extra column for the OP_Sequence */
      }
      ଯଦି( pFunc->bOBPayload ){
        /* extra columns for the function arguments */
        ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pFunc->pFExpr) );
        ପ୍ରତିଜ୍ଞା( pFunc->pFExpr->x.pList!=0 );
        nExtra += pFunc->pFExpr->x.pList->n_ପଦସମଷ୍ଟି;
      }
      ଯଦି( pFunc->bUseSubtype ){
        nExtra += pFunc->pFExpr->x.pList->n_ପଦସମଷ୍ଟି;
      }
      p_ଚାବି_Info = sqlite3_ଚାବି_Info_From_ପଦସମଷ୍ଟି_List(pParse, pOBList, 0, nExtra);
      ଯଦି( !pFunc->bOBUnique && pParse->nErr==0 ){
        p_ଚାବି_Info->n_ଚାବି_Field++;
      }
      sqlite3VdbeAddOp4(v, OP_OpenEphemeral,
            pFunc->iOBTab, pOBList->n_ପଦସମଷ୍ଟି+nExtra, 0,
            (ଅକ୍ଷର*)p_ଚାବି_Info, P4_KEYINFO);
      ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(ORDER BY)",
                          pFunc->pFunc->zName));
    }
  }
}

/*
** Invoke the OP_AggFinalize opcode for every aggregate function
** in the AggInfo structure.
*/
ସ୍ଥିର ଶୂନ୍ୟ finalizeAggFunctions(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ସଂରଚନା AggInfo_func *pF;
  ପାଇଁ(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    ପଦସମଷ୍ଟି_List *pList;
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pF->pFExpr) );
    ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ;
    pList = pF->pFExpr->x.pList;
    ଯଦି( pF->iOBTab>=0 ){
      /* For an ORDER BY aggregate, calls to OP_AggStep were deferred.  Inputs
      ** were stored in emphermal table pF->iOBTab.  Here, we extract those
      ** inputs (in ORDER BY order) and make all calls to OP_AggStep
      ** before doing the OP_AggFinal call. */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_ଶୀର୍ଷ;        /* Start of loop for extracting columns */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg;        /* Number of columns to extract */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ଚାବି;        /* Key columns to be skipped */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା regAgg;      /* Extract into this array */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;           /* Loop counter */
     
      ପ୍ରତିଜ୍ଞା( pF->pFunc!=0 );
      nArg = pList->n_ପଦସମଷ୍ଟି;
      regAgg = sqlite3GetTempRange(pParse, nArg);

      ଯଦି( pF->bOBPayload==0 ){
        n_ଚାବି = 0;
      }ଅନ୍ୟଥା{
        ପ୍ରତିଜ୍ଞା( pF->pFExpr->p_ବାମ!=0 );
        ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pF->pFExpr->p_ବାମ) );
        ପ୍ରତିଜ୍ଞା( pF->pFExpr->p_ବାମ->x.pList!=0 );
        n_ଚାବି = pF->pFExpr->p_ବାମ->x.pList->n_ପଦସମଷ୍ଟି;
        ଯଦି( ALWAYS(!pF->bOBUnique) ) n_ଚାବି++;
      }
      i_ଶୀର୍ଷ = sqlite3VdbeAddOp1(v, OP_Rewind, pF->iOBTab); VdbeCoverage(v);
      ପାଇଁ(j=nArg-1; j>=0; j--){
        sqlite3VdbeAddOp3(v, OP_Column, pF->iOBTab, n_ଚାବି+j, regAgg+j);
      }
      ଯଦି( pF->bUseSubtype ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା regSubtype = sqlite3GetTempReg(pParse);
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iBaseCol = n_ଚାବି + nArg + (pF->bOBPayload==0 && pF->bOBUnique==0);
        ପାଇଁ(j=nArg-1; j>=0; j--){
          sqlite3VdbeAddOp3(v, OP_Column, pF->iOBTab, iBaseCol+j, regSubtype);
          sqlite3VdbeAddOp2(v, OP_SetSubtype, regSubtype, regAgg+j);
        }
        sqlite3ReleaseTempReg(pParse, regSubtype);
      }
      sqlite3VdbeAddOp3(v, OP_AggStep, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3VdbeAddOp2(v, OP_Next, pF->iOBTab, i_ଶୀର୍ଷ+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, i_ଶୀର୍ଷ);
      sqlite3ReleaseTempRange(pParse, regAgg, nArg);
    }
    sqlite3VdbeAddOp2(v, OP_AggFinal, AggInfoFuncReg(pAggInfo,i),
                      pList ? pList->n_ପଦସମଷ୍ଟି : 0);
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
ସ୍ଥିର ଶୂନ୍ୟ updateAccumulator(
  Parse *pParse,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regAcc,
  AggInfo *pAggInfo,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eDistinctType
){
  Vdbe *v = pParse->pVdbe;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା regHit = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrHitTest = 0;
  ସଂରଚନା AggInfo_func *pF;
  ସଂରଚନା AggInfo_col *pC;

  ପ୍ରତିଜ୍ଞା( pAggInfo->iFirstReg>0 );
  ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ;
  pAggInfo->directMode = 1;
  ପାଇଁ(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrNext = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା regAgg;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା regAggSz = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା regDistinct = 0;
    ପଦସମଷ୍ଟି_List *pList;
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pF->pFExpr) );
    ପ୍ରତିଜ୍ଞା( !IsWindowFunc(pF->pFExpr) );
    ପ୍ରତିଜ୍ଞା( pF->pFunc!=0 );
    pList = pF->pFExpr->x.pList;
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(pF->pFExpr, EP_WinFunc) ){
      ପଦସମଷ୍ଟି *pFilter = pF->pFExpr->y.pWin->pFilter;
      ଯଦି( pAggInfo->nAccumulator
       && (pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)
       && regAcc
      ){
        /* If regAcc==0, there there exists some min() or max() function
        ** without a FILTER clause that will ensure the magnet registers
        ** are populated. */
        ଯଦି( regHit==0 ) regHit = ++pParse->nMem;
        /* If this is the first row of the group (regAcc contains 0), clear the
        ** "magnet" register regHit so that the accumulator registers
        ** are populated if the FILTER clause jumps over the the
        ** invocation of min() or max() altogether. Or, if this is not
        ** the first row (regAcc contains 1), set the magnet register so that
        ** the accumulators are not populated unless the min()/max() is invoked
        ** and indicates that they should be.  */
        sqlite3VdbeAddOp2(v, OP_Copy, regAcc, regHit);
      }
      addrNext = sqlite3VdbeMakeLabel(pParse);
      sqlite3_ପଦସମଷ୍ଟି_If_False(pParse, pFilter, addrNext, SQLITE_JUMPIFNULL);
    }
    ଯଦି( pF->iOBTab>=0 ){
      /* Instead of invoking AggStep, we must push the arguments that would
      ** have been passed to AggStep onto the sorting table. */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା jj;                /* Registered used so far in building the record */
      ପଦସମଷ୍ଟି_List *pOBList;     /* The ORDER BY clause */
      ପ୍ରତିଜ୍ଞା( pList!=0 );
      nArg = pList->n_ପଦସମଷ୍ଟି;
      ପ୍ରତିଜ୍ଞା( nArg>0 );
      ପ୍ରତିଜ୍ଞା( pF->pFExpr->p_ବାମ!=0 );
      ପ୍ରତିଜ୍ଞା( pF->pFExpr->p_ବାମ->op==TK_ORDER );
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pF->pFExpr->p_ବାମ) );
      pOBList = pF->pFExpr->p_ବାମ->x.pList;
      ପ୍ରତିଜ୍ଞା( pOBList!=0 );
      ପ୍ରତିଜ୍ଞା( pOBList->n_ପଦସମଷ୍ଟି>0 );
      regAggSz = pOBList->n_ପଦସମଷ୍ଟି;
      ଯଦି( !pF->bOBUnique ){
        regAggSz++;   /* One register for OP_Sequence */
      }
      ଯଦି( pF->bOBPayload ){
        regAggSz += nArg;
      }
      ଯଦି( pF->bUseSubtype ){
        regAggSz += nArg;
      }
      regAggSz++;  /* One extra register to hold result of MakeRecord */
      regAgg = sqlite3GetTempRange(pParse, regAggSz);
      regDistinct = regAgg;
      sqlite3_ପଦସମଷ୍ଟି_Code_ପଦସମଷ୍ଟି_List(pParse, pOBList, regAgg, 0, SQLITE_ECEL_DUP);
      jj = pOBList->n_ପଦସମଷ୍ଟି;
      ଯଦି( !pF->bOBUnique ){
        sqlite3VdbeAddOp2(v, OP_Sequence, pF->iOBTab, regAgg+jj);
        jj++;
      }
      ଯଦି( pF->bOBPayload ){
        regDistinct = regAgg+jj;
        sqlite3_ପଦସମଷ୍ଟି_Code_ପଦସମଷ୍ଟି_List(pParse, pList, regDistinct, 0, SQLITE_ECEL_DUP);
        jj += nArg;
      }
      ଯଦି( pF->bUseSubtype ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା kk;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା regBase = pF->bOBPayload ? regDistinct : regAgg;
        ପାଇଁ(kk=0; kk<nArg; kk++, jj++){
          sqlite3VdbeAddOp2(v, OP_GetSubtype, regBase+kk, regAgg+jj);
        }
      }
    }ଅନ୍ୟଥା ଯଦି( pList ){
      nArg = pList->n_ପଦସମଷ୍ଟି;
      regAgg = sqlite3GetTempRange(pParse, nArg);
      regDistinct = regAgg;
      sqlite3_ପଦସମଷ୍ଟି_Code_ପଦସମଷ୍ଟି_List(pParse, pList, regAgg, 0, SQLITE_ECEL_DUP);
    }ଅନ୍ୟଥା{
      nArg = 0;
      regAgg = 0;
    }
    ଯଦି( pF->iDistinct>=0 && pList ){
      ଯଦି( addrNext==0 ){
        addrNext = sqlite3VdbeMakeLabel(pParse);
      }
      pF->iDistinct = codeDistinct(pParse, eDistinctType,
          pF->iDistinct, addrNext, pList, regDistinct);
    }
    ଯଦି( pF->iOBTab>=0 ){
      /* Insert a new record into the ORDER BY table */
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regAgg, regAggSz-1,
                        regAgg+regAggSz-1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ଭର୍ତ୍ତି, pF->iOBTab, regAgg+regAggSz-1,
                           regAgg, regAggSz-1);
      sqlite3ReleaseTempRange(pParse, regAgg, regAggSz);
    }ଅନ୍ୟଥା{
      /* Invoke the AggStep function */
      ଯଦି( pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        CollSeq *pColl = 0;
        ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
        ପ୍ରତିଜ୍ଞା( pList!=0 );  /* pList!=0 if pF->pFunc has NEEDCOLL */
        ପାଇଁ(j=0, pItem=pList->a; !pColl && j<nArg; j++, pItem++){
          pColl = sqlite3_ପଦସମଷ୍ଟି_Coll_Seq(pParse, pItem->p_ପଦସମଷ୍ଟି);
        }
        ଯଦି( !pColl ){
          pColl = pParse->db->pDfltColl;
        }
        ଯଦି( regHit==0 && pAggInfo->nAccumulator ) regHit = ++pParse->nMem;
        sqlite3VdbeAddOp4(v, OP_CollSeq, regHit, 0, 0,
                         (ଅକ୍ଷର *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddOp3(v, OP_AggStep, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3ReleaseTempRange(pParse, regAgg, nArg);
    }
    ଯଦି( addrNext ){
      sqlite3VdbeResolveLabel(v, addrNext);
    }
    ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ;
  }
  ଯଦି( regHit==0 && pAggInfo->nAccumulator ){
    regHit = regAcc;
  }
  ଯଦି( regHit ){
    addrHitTest = sqlite3VdbeAddOp1(v, OP_If, regHit); VdbeCoverage(v);
  }
  ପାଇଁ(i=0, pC=pAggInfo->aCol; i<pAggInfo->nAccumulator; i++, pC++){
    sqlite3_ପଦସମଷ୍ଟି_Code(pParse, pC->pCExpr, AggInfoColumnReg(pAggInfo,i));
    ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ;
  }

  pAggInfo->directMode = 0;
  ଯଦି( addrHitTest ){
    sqlite3_Vdbe_Jump_Here_Or_କାଢ଼_Inst(v, addrHitTest);
  }
}

/*
** Add a single OP_Explain instruction to the VDBE to explain a simple
** count(*) query ("SELECT count(*) FROM pTab").
*/
#ifndef SQLITE_OMIT_EXPLAIN
ସ୍ଥିର ଶୂନ୍ୟ explain_Simple_ଗଣନା(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Table being queried */
  Index *pIdx                     /* Index used to optimize scan, or NULL */
){
  ଯଦି( pParse->explain==2 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା bCover = (pIdx!=0 && (HasRowid(pTab) || !Is_Primary_ଚାବି_Index(pIdx)));
    sqlite3VdbeExplain(pParse, 0, "SCAN %s%s%s",
        pTab->zName,
        bCover ? " USING COVERING INDEX " : "",
        bCover ? pIdx->zName : ""
    );
  }
}
#ଅନ୍ୟଥା
# define explain_Simple_ଗଣନା(a,b,c)
#endif

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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା having_To_Where_ପଦସମଷ୍ଟି_Cb(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଯଦି( p_ପଦସମଷ୍ଟି->op!=TK_AND ){
    Select *pS = pWalker->u.pSelect;
    /* This routine is called before the HAVING clause of the current
    ** SELECT is analyzed for aggregates. So if pExpr->pAggInfo is set
    ** here, it indicates that the expression is a correlated reference to a
    ** column from an outer aggregate query, or an aggregate function that
    ** belongs to an outer query. Do not move the expression to the WHERE
    ** clause in this obscure case, as doing so may corrupt the outer Select
    ** statements AggInfo structure.  */
    ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Is_Constant_Or_Group_By(pWalker->pParse, p_ପଦସମଷ୍ଟି, pS->pGroupBy)
     && ପଦସମଷ୍ଟି_Always_False(p_ପଦସମଷ୍ଟି)==0
     && p_ପଦସମଷ୍ଟି->pAggInfo==0
    ){
      sqlite3 *db = pWalker->pParse->db;
      ପଦସମଷ୍ଟି *pNew = sqlite3_ପଦସମଷ୍ଟି_Int32(db, 1);
      ଯଦି( pNew ){
        ପଦସମଷ୍ଟି *pWhere = pS->pWhere;
        SWAP(ପଦସମଷ୍ଟି, *pNew, *p_ପଦସମଷ୍ଟି);
        pNew = sqlite3_ପଦସମଷ୍ଟି_And(pWalker->pParse, pWhere, pNew);
        pS->pWhere = pNew;
        pWalker->eCode = 1;
      }
    }
    ଫେରନ୍ତୁ WRC_Prune;
  }
  ଫେରନ୍ତୁ WRC_Continue;
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
ସ୍ଥିର ଶୂନ୍ୟ havingToWhere(Parse *pParse, Select *p){
  Walker sWalker;
  memset(&sWalker, 0, ଆକାର(sWalker));
  sWalker.pParse = pParse;
  sWalker.x_ପଦସମଷ୍ଟି_Callback = having_To_Where_ପଦସମଷ୍ଟି_Cb;
  sWalker.u.pSelect = p;
  sqlite3_Walk_ପଦସମଷ୍ଟି(&sWalker, p->pHaving);
#ଯଦି TREETRACE_ENABLED
  ଯଦି( sWalker.eCode && (sqlite3_ବୃକ୍ଷ_Trace & 0x100)!=0 ){
    TREETRACE(0x100,pParse,p,("Move HAVING terms into WHERE:\n"));
    sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
  }
#endif
}

/*
** Check to see if the pThis entry of pTabList is a self-join of another view.
** Search FROM-clause entries in the range of iFirst..iEnd, including iFirst
** but stopping before iEnd.
**
** If pThis is a self-join, then return the SrcItem for the first other
** instance of that view found.  If pThis is not a self-join then return 0.
*/
ସ୍ଥିର SrcItem *isSelfJoinView(
  SrcList *pTabList,           /* Search for self-joins in this FROM clause */
  SrcItem *pThis,              /* Search for prior reference to this subquery */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFirst, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iEnd        /* Range of FROM-clause entries to search. */
){
  SrcItem *pItem;
  Select *pSel;
  ପ୍ରତିଜ୍ଞା( pThis->fg.isSubquery );
  pSel = pThis->u4.pSubq->pSelect;
  ପ୍ରତିଜ୍ଞା( pSel!=0 );
  ଯଦି( pSel->selFlags & SF_ଠେଲ_Down ) ଫେରନ୍ତୁ 0;
  ଯେତେବେଳେ( iFirst<iEnd ){
    Select *pS1;
    pItem = &pTabList->a[iFirst++];
    ଯଦି( !pItem->fg.isSubquery ) ଜାରି;
    ଯଦି( pItem->fg.viaCoroutine ) ଜାରି;
    ଯଦି( pItem->zName==0 ) ଜାରି;
    ପ୍ରତିଜ୍ଞା( pItem->pSTab!=0 );
    ପ୍ରତିଜ୍ଞା( pThis->pSTab!=0 );
    ଯଦି( pItem->pSTab->pSchema!=pThis->pSTab->pSchema ) ଜାରି;
    ଯଦି( sqlite3_stricmp(pItem->zName, pThis->zName)!=0 ) ଜାରି;
    pS1 = pItem->u4.pSubq->pSelect;
    ଯଦି( pItem->pSTab->pSchema==0 && pSel->selId!=pS1->selId ){
      /* The query flattener left two different CTE tables with identical
      ** names in the same FROM clause. */
      ଜାରି;
    }
    ଯଦି( pS1->selFlags & SF_ଠେଲ_Down ){
      /* The view was modified by some other optimization such as
      ** pushDownWhereTerms() */
      ଜାରି;
    }
    ଫେରନ୍ତୁ pItem;
  }
  ଫେରନ୍ତୁ 0;
}

/*
** Deallocate a single AggInfo object
*/
ସ୍ଥିର ଶୂନ୍ୟ agginfoFree(sqlite3 *db, ଶୂନ୍ୟ *pArg){
  AggInfo *p = (AggInfo*)pArg;
  sqlite3DbFree(db, p->aCol);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_Of_View_Optimization(Parse *pParse, Select *p){
  Select *pSub, *pPrior;
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି;
  ପଦସମଷ୍ଟି *p_ଗଣନା;
  sqlite3 *db;
  SrcItem *pFrom;
  ଯଦି( (p->selFlags & SF_Aggregate)==0 ) ଫେରନ୍ତୁ 0;   /* This is an aggregate */
  ଯଦି( p->pEList->n_ପଦସମଷ୍ଟି!=1 ) ଫେରନ୍ତୁ 0;               /* Single result column */
  ଯଦି( p->pWhere ) ଫେରନ୍ତୁ 0;
  ଯଦି( p->pHaving ) ଫେରନ୍ତୁ 0;
  ଯଦି( p->pGroupBy ) ଫେରନ୍ତୁ 0;
  ଯଦି( p->pOrderBy ) ଫେରନ୍ତୁ 0;
  p_ପଦସମଷ୍ଟି = p->pEList->a[0].p_ପଦସମଷ୍ଟି;
  ଯଦି( p_ପଦସମଷ୍ଟି->op!=TK_AGG_FUNCTION ) ଫେରନ୍ତୁ 0;        /* Result is an aggregate */
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_UToken(p_ପଦସମଷ୍ଟି) );
  ଯଦି( sqlite3_stricmp(p_ପଦସମଷ୍ଟି->u.zToken,"count") ) ଫେରନ୍ତୁ 0;  /* Is count() */
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
  ଯଦି( p_ପଦସମଷ୍ଟି->x.pList!=0 ) ଫେରନ୍ତୁ 0;                 /* Must be count(*) */
  ଯଦି( p->pSrc->nSrc!=1 ) ଫେରନ୍ତୁ 0;                  /* One table in FROM  */
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) ) ଫେରନ୍ତୁ 0;/* Not a window function */
  pFrom = p->pSrc->a;
  ଯଦି( pFrom->fg.isSubquery==0 ) ଫେରନ୍ତୁ 0;           /* FROM is a subquery */
  ଯଦି( (p->selFlags & SF_Correlated)!=0 ) ଫେରନ୍ତୁ 0;  /* Not a correlated subq */
  pSub = pFrom->u4.pSubq->pSelect;
  ଯଦି( pSub->pPrior==0 ) ଫେରନ୍ତୁ 0;                   /* Must be a compound */
  ଯଦି( pSub->selFlags & SF_CopyCte ) ଫେରନ୍ତୁ 0;       /* Not a CTE */
  କର{
    ଯଦି( pSub->op!=TK_ALL && pSub->pPrior ) ଫେରନ୍ତୁ 0;  /* Must be UNION ALL */
    ଯଦି( pSub->pWhere ) ଫେରନ୍ତୁ 0;                      /* No WHERE clause */
    ଯଦି( pSub->pLimit ) ଫେରନ୍ତୁ 0;                      /* No LIMIT clause */
    ଯଦି( pSub->selFlags & (SF_Aggregate|SF_Distinct) ){
       testcase( pSub->selFlags & SF_Aggregate );
       testcase( pSub->selFlags & SF_Distinct );
       ଫେରନ୍ତୁ 0;     /* Not an aggregate nor DISTINCT */
    }
    ପ୍ରତିଜ୍ଞା( pSub->pHaving==0 );  /* Due to the previous */
    pSub = pSub->pPrior;                              /* Repeat over compound */
  }ଯେତେବେଳେ( pSub );

  /* If we reach this point then it is OK to perform the transformation */

  db = pParse->db;
  p_ଗଣନା = p_ପଦସମଷ୍ଟି;
  p_ପଦସମଷ୍ଟି = 0;
  pSub = sqlite3SubqueryDetach(db, pFrom);
  sqlite3SrcListDelete(db, p->pSrc);
  p->pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  ଯେତେବେଳେ( pSub ){
    ପଦସମଷ୍ଟି *pTerm;
    pPrior = pSub->pPrior;
    pSub->pPrior = 0;
    pSub->pNext = 0;
    pSub->selFlags |= SF_Aggregate;
    pSub->selFlags &= ~(u32)SF_Compound;
    pSub->nSelectRow = 0;
    sqlite3ParserAddCleanup(pParse, sqlite3_ପଦସମଷ୍ଟି_List_Delete_Generic, pSub->pEList);
    pTerm = pPrior ? sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ଗଣନା, 0) : p_ଗଣନା;
    pSub->pEList = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, 0, pTerm);
    pTerm = sqlite3PExpr(pParse, TK_SELECT, 0, 0);
    sqlite3PExprAddSelect(pParse, pTerm, pSub);
    ଯଦି( p_ପଦସମଷ୍ଟି==0 ){
      p_ପଦସମଷ୍ଟି = pTerm;
    }ଅନ୍ୟଥା{
      p_ପଦସମଷ୍ଟି = sqlite3PExpr(pParse, TK_PLUS, pTerm, p_ପଦସମଷ୍ଟି);
    }
    ଯଦି( pParse->nErr ){
      sqlite3_ପଦସମଷ୍ଟି_Delete(db, p_ପଦସମଷ୍ଟି);
      p_ପଦସମଷ୍ଟି = 0;
    }
    pSub = pPrior;
  }
  p->pEList->a[0].p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି;
  p->selFlags &= ~(u32)SF_Aggregate;

#ଯଦି TREETRACE_ENABLED
  ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x200 ){
    TREETRACE(0x200,pParse,p,("After count-of-view optimization:\n"));
    sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
  }
#endif
  ଫେରନ୍ତୁ 1;
}

/*
** If any term of pSrc, or any SF_NestedFrom sub-query, is not the same
** as pSrcItem but has the same alias as p0, then return true.
** Otherwise return false.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sameSrcAlias(SrcItem *p0, SrcList *pSrc){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<pSrc->nSrc; i++){
    SrcItem *p1 = &pSrc->a[i];
    ଯଦି( p1==p0 ) ଜାରି;
    ଯଦି( p0->pSTab==p1->pSTab && 0==sqlite3_stricmp(p0->zAlias, p1->zAlias) ){
      ଫେରନ୍ତୁ 1;
    }
    ଯଦି( p1->fg.isSubquery
     && (p1->u4.pSubq->pSelect->selFlags & SF_NestedFrom)!=0
     && sameSrcAlias(p0, p1->u4.pSubq->pSelect->pSrc)
    ){
      ଫେରନ୍ତୁ 1;
    }
  }
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fromClauseTermCanBeCoroutine(
  Parse *pParse,          /* Parsing context */
  SrcList *pTabList,      /* FROM clause */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i,                  /* Which term of the FROM clause holds the subquery */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା selFlags            /* Flags on the SELECT statement */
){
  SrcItem *pItem = &pTabList->a[i];
  ଯଦି( pItem->fg.isCte ){
    ଅଚଳ CteUse *pCteUse = pItem->u2.pCteUse;
    ଯଦି( pCteUse->eM10d==M10d_Yes ) ଫେରନ୍ତୁ 0;                          /* (2a) */
    ଯଦି( pCteUse->nUse>=2 && pCteUse->eM10d!=M10d_No ) ଫେରନ୍ତୁ 0;       /* (2b) */
  }
  ଯଦି( pTabList->a[0].fg.jointype & JT_LTORJ ) ଫେରନ୍ତୁ 0;               /* (3)  */
  ଯଦି( OptimizationDisabled(pParse->db, SQLITE_Coroutines) ) ଫେରନ୍ତୁ 0; /* (4)  */
  ଯଦି( isSelfJoinView(pTabList, pItem, i+1, pTabList->nSrc)!=0 ){
    ଫେରନ୍ତୁ 0;                                                          /* (5) */
  }
  ଯଦି( i==0 ){
    ଯଦି( pTabList->nSrc==1 ) ଫେରନ୍ତୁ 1;                             /* (1a) */
    ଯଦି( pTabList->a[1].fg.jointype & JT_CROSS ) ଫେରନ୍ତୁ 1;         /* (1b) */
    ଯଦି( selFlags & SF_UpdateFrom )              ଫେରନ୍ତୁ 0;         /* (1c-iii) */
    ଫେରନ୍ତୁ 1;
  }
  ଯଦି( selFlags & SF_UpdateFrom ) ଫେରନ୍ତୁ 0;                        /* (1c-iii) */
  ଯେତେବେଳେ( 1 /*exit-by-break*/ ){
    ଯଦି( pItem->fg.jointype & (JT_OUTER|JT_CROSS)  ) ଫେରନ୍ତୁ 0;     /* (1c-ii) */
    ଯଦି( i==0 ) ଭାଙ୍ଗ;
    i--;
    pItem--;
    ଯଦି( pItem->fg.isSubquery ) ଫେରନ୍ତୁ 0;                          /* (1c-i) */
  }
  ଫେରନ୍ତୁ 1;
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
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ existsToJoin(
  Parse *pParse,  /* Parsing context */
  Select *p,      /* The SELECT statement being optimized */
  ପଦସମଷ୍ଟି *pWhere    /* part of the WHERE clause currently being examined */
){
  ଯଦି( pParse->nErr==0
   && pWhere!=0
   && !ପଦସମଷ୍ଟି_Has_Property(pWhere, EP_OuterON|EP_InnerON) 
   && ALWAYS(p->pSrc!=0)
   && p->pSrc->nSrc<BMS
   && (p->pLimit==0 || p->pLimit->p_ଡାହାଣ==0)
  ){
    ଯଦି( pWhere->op==TK_AND ){
      ପଦସମଷ୍ଟି *p_ଡାହାଣ = pWhere->p_ଡାହାଣ;
      existsToJoin(pParse, p, pWhere->p_ବାମ);
      existsToJoin(pParse, p, p_ଡାହାଣ);
    }
    ଅନ୍ୟଥା ଯଦି( pWhere->op==TK_EXISTS ){
      Select *pSub = pWhere->x.pSelect;
      ପଦସମଷ୍ଟି *pSubWhere = pSub->pWhere;
      ଯଦି( pSub->pSrc->nSrc==1
       && (pSub->selFlags & SF_Aggregate)==0
       && !pSub->pSrc->a[0].fg.isSubquery
       && pSub->pLimit==0
       && pSub->pPrior==0
      ){
        /* Before combining the sub-select with the parent, renumber the 
        ** cursor used by the subselect. This is because the EXISTS expression
        ** might be a copy of another EXISTS expression from somewhere
        ** else in the tree, and in this case it is important that it use
        ** a unique cursor number.  */
        sqlite3 *db = pParse->db;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା *aCsrMap = sqlite3DbMallocZero(db, (pParse->nTab+2)*ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା));
        ଯଦି( aCsrMap==0 ) ଫେରନ୍ତୁ;
        aCsrMap[0] = (pParse->nTab+1);
        renumberCursors(pParse, pSub, -1, aCsrMap);
        sqlite3DbFree(db, aCsrMap);

        memset(pWhere, 0, ଆକାର(*pWhere));
        pWhere->op = TK_INTEGER;
        pWhere->u.i_ମୂଲ୍ୟ = 1;
        ପଦସମଷ୍ଟି_Set_Property(pWhere, EP_Int_ମୂଲ୍ୟ);
        ପ୍ରତିଜ୍ଞା( p->pWhere!=0 );
        pSub->pSrc->a[0].fg.fromExists = 1;
        p->pSrc = sqlite3SrcListAppendList(pParse, p->pSrc, pSub->pSrc);
        ଯଦି( pSubWhere ){
          p->pWhere = sqlite3PExpr(pParse, TK_AND, p->pWhere, pSubWhere);
          pSub->pWhere = 0;
        }
        pSub->pSrc = 0;
        sqlite3ParserAddCleanup(pParse, sqlite3SelectDeleteGeneric, pSub);
        recomputeColumnsUsed(p, &p->pSrc->a[p->pSrc->nSrc-1]);
#ଯଦି TREETRACE_ENABLED
        ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x100000 ){
          TREETRACE(0x100000,pParse,p,
                    ("After EXISTS-to-JOIN optimization:\n"));
          sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
        }
#endif
      }
    }
  }
}

/*
** Type used for Walker callbacks by selectCheckOnClauses().
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା CheckOnCtx CheckOnCtx;
ସଂରଚନା CheckOnCtx {
  SrcList *pSrc;       /* SrcList for this context */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iJoin;           /* Cursors must be left of this one, if not negative */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bFuncArg;        /* True for table-function arg */
  CheckOnCtx *pParent; /* Parent context */
};

/*
** True if the SrcList passed as the only argument contains at least
** one RIGHT or FULL JOIN. False otherwise.  
*/
#define has_ଡାହାଣ_Join(pSrc) (((pSrc)->a[0].fg.jointype & JT_LTORJ)!=0)

/*
** The xExpr callback for the search of invalid ON clause terms.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା select_Check_On_Clauses_ପଦସମଷ୍ଟି(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;

  /* Check if pExpr is root or near-root of an ON clause constraint that needs
  ** to be checked to ensure that it does not refer to tables in its FROM
  ** clause to the right of itself. i.e. it is either:
  **
  **   + an ON clause on an OUTER join, or
  **   + an ON clause on an INNER join within a FROM that features at
  **     least one RIGHT or FULL join.
  */
  ଯଦି( (ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON))
   || (ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_InnerON) && has_ଡାହାଣ_Join(pCtx->pSrc))
  ){
    /* If CheckOnCtx.iJoin is already set, then fall through and process
    ** this expression node as normal. Or, if CheckOnCtx.iJoin is still 0,
    ** set it to the cursor number of the RHS of the join to which this
    ** ON expression was attached and then iterate through the entire 
    ** expression.  */
    ପ୍ରତିଜ୍ଞା( pCtx->iJoin<0 || pCtx->iJoin==p_ପଦସମଷ୍ଟି->w.iJoin );
    ଯଦି( pCtx->iJoin<0 ){
      pCtx->iJoin = p_ପଦସମଷ୍ଟି->w.iJoin;
      sqlite3_Walk_ପଦସମଷ୍ଟି_NN(pWalker, p_ପଦସମଷ୍ଟି);
      pCtx->iJoin = -1;
      ଫେରନ୍ତୁ WRC_Prune;
    }
  }

  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_COLUMN ){
    /* A column expression. Find the SrcList (if any) to which it refers.
    ** Then, if CheckOnCtx.iJoin indicates that this expression is part of an
    ** ON clause from that SrcList (i.e. if iJoin is non-zero), check that it
    ** does not refer to a table to the right of CheckOnCtx.iJoin. */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iTab = p_ପଦସମଷ୍ଟି->iTable;
    କର {
      SrcList *pSrc = pCtx->pSrc;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nSrc = pSrc->nSrc;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
      ପାଇଁ(ii=0; ii<nSrc && pSrc->a[ii].iCursor!=iTab; ii++){}
      ଯଦି( ii<nSrc ){
        /* pSrc is the FROM clause that contains iTab */
        ଯଦି( pCtx->iJoin>=0 ){
          ପାଇଁ(ii--; ii>=0 && pSrc->a[ii].iCursor!=pCtx->iJoin; ii--){}
          ଯଦି( ii>=0 ){
            /* Table iJoin appears to the left of table iTab in the SrcList.
            ** Therefore the expression refers to a table to its right. */
            sqlite3_Error_ସନ୍ଦେଶ(pWalker->pParse, 
                "%s references tables to its right",
                (pCtx->bFuncArg ? "table-function argument" : "ON clause")
            );
            ଫେରନ୍ତୁ WRC_Abort;
          }
        }
        ଭାଙ୍ଗ;
      }
      pCtx = pCtx->pParent;
    }ଯେତେବେଳେ( pCtx );
  }
  ଫେରନ୍ତୁ WRC_Continue;
}

/*
** The xSelect callback for the search of invalid ON clause terms.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା selectCheckOnClausesSelect(Walker *pWalker, Select *pSelect){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;
  ଯଦି( pSelect->pSrc==pCtx->pSrc || pSelect->pSrc->nSrc==0 ){
    ଫେରନ୍ତୁ WRC_Continue;
  }ଅନ୍ୟଥା{
    CheckOnCtx sCtx;
    memset(&sCtx, 0, ଆକାର(sCtx));
    sCtx.pSrc = pSelect->pSrc;
    sCtx.pParent = pCtx;
    sCtx.iJoin = -1;
    pWalker->u.pCheckOnCtx = &sCtx;
    sqlite3WalkSelect(pWalker, pSelect);
    pWalker->u.pCheckOnCtx = pCtx;
    pSelect->selFlags &= ~SF_OnToWhere;
    ଫେରନ୍ତୁ WRC_Prune;
  }
}

/*
** Check all ON clauses in pSelect to verify that they do not reference
** columns to the right.
*/
ଶୂନ୍ୟ sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect){
  Walker w;
  CheckOnCtx sCtx;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
  ପ୍ରତିଜ୍ଞା( pSelect->selFlags & SF_OnToWhere );
  ପ୍ରତିଜ୍ଞା( pSelect->pSrc!=0 && pSelect->pSrc->nSrc>=2 );
  memset(&w, 0, ଆକାର(w));
  w.pParse = pParse;
  w.x_ପଦସମଷ୍ଟି_Callback = select_Check_On_Clauses_ପଦସମଷ୍ଟି;
  w.xSelectCallback = selectCheckOnClausesSelect;
  w.u.pCheckOnCtx = &sCtx;
  memset(&sCtx, 0, ଆକାର(sCtx));
  sCtx.pSrc = pSelect->pSrc;
  sCtx.iJoin = -1;
  sqlite3_Walk_ପଦସମଷ୍ଟି(&w, pSelect->pWhere);
  pSelect->selFlags &= ~SF_OnToWhere;

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
  ପାଇଁ(ii=0; ii<pSelect->pSrc->nSrc; ii++){
    SrcItem *pItem = &pSelect->pSrc->a[ii];
    ଯଦି( pItem->fg.isTabFunc ){
      sCtx.iJoin = -1;
      ଯଦି( (pItem->fg.jointype & (JT_LTORJ|JT_ଡାହାଣ)) ){
        sCtx.iJoin = pItem->iCursor;
      }ଅନ୍ୟଥା{
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା jj;
        ପାଇଁ(jj=ii+1; jj<pSelect->pSrc->nSrc; jj++){
          SrcItem *pItem2 = &pSelect->pSrc->a[jj];
          ଯଦି( pItem2->fg.jointype & JT_OUTER ){
            sCtx.iJoin = pItem2[-1].iCursor;
            ଭାଙ୍ଗ;
          }
        }
      }
      ଯଦି( sCtx.iJoin>=0 ){
        sqlite3_Walk_ପଦସମଷ୍ଟି_List(&w, pItem->u1.pFuncArg);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3CopySortOrder(ପଦସମଷ୍ଟି_List *p1, ପଦସମଷ୍ଟି_List *p2){
  ପ୍ରତିଜ୍ଞା( p1 );
  ଯଦି( p2 && p1->n_ପଦସମଷ୍ଟି==p2->n_ପଦସମଷ୍ଟି ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
    ପାଇଁ(ii=0; ii<p1->n_ପଦସମଷ୍ଟି; ii++){
      u8 sortFlags;
      sortFlags = p2->a[ii].fg.sortFlags & KEYINFO_ORDER_DESC;
      p1->a[ii].fg.sortFlags = sortFlags;
    }
    ଫେରନ୍ତୁ 1;
  }ଅନ୍ୟଥା{
    ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3Select(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  SelectDest *pDest      /* What to do with the query results */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;              /* Loop counters */
  WhereInfo *pWInfo;     /* Return from sqlite3WhereBegin() */
  Vdbe *v;               /* The virtual machine under construction */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isAgg;             /* True for select lists like "count(*)" */
  ପଦସମଷ୍ଟି_List *pEList = 0;  /* List of columns to extract. */
  SrcList *pTabList;     /* List of tables to select from */
  ପଦସମଷ୍ଟି *pWhere;          /* The WHERE clause.  May be NULL */
  ପଦସମଷ୍ଟି_List *pGroupBy;    /* The GROUP BY clause.  May be NULL */
  ପଦସମଷ୍ଟି *pHaving;         /* The HAVING clause.  May be NULL */
  AggInfo *pAggInfo = 0; /* Aggregate information */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 1;            /* Value to return from this function */
  DistinctCtx sDistinct; /* Info on how to code the DISTINCT keyword */
  SortCtx sSort;         /* Info on how to code the ORDER BY clause */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iEnd;              /* Address of the end of the query */
  sqlite3 *db;           /* The database connection */
  ପଦସମଷ୍ଟି_List *pMinMaxOrderBy = 0;  /* Added ORDER BY for min/max queries */
  u8 minMaxFlag;                 /* Flag for min/max queries */

  db = pParse->db;
  ପ୍ରତିଜ୍ଞା( pParse==db->pParse );
  v = sqlite3GetVdbe(pParse);
  ଯଦି( p==0 || pParse->nErr ){
    ଫେରନ୍ତୁ 1;
  }
  ପ୍ରତିଜ୍ଞା( db->mallocFailed==0 );
  ଯଦି( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) ଫେରନ୍ତୁ 1;
#ଯଦି TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p, ("begin processing:\n", pParse->addrExplain));
  ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x10000 ){
    ଯଦି( (sqlite3_ବୃକ୍ଷ_Trace & 0x10001)==0x10000 ){
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(0, "In sqlite3Select() at %s:%d",
                           __FILE__, __LINE__);
    }
    sqlite3ShowSelect(p);
  }
#endif

  /* tag-select-0100 */
  ପ୍ରତିଜ୍ଞା( p->pOrderBy==0 || pDest->eDest!=SRT_DistFifo );
  ପ୍ରତିଜ୍ଞା( p->pOrderBy==0 || pDest->eDest!=SRT_Fifo );
  ପ୍ରତିଜ୍ଞା( p->pOrderBy==0 || pDest->eDest!=SRT_DistQueue );
  ପ୍ରତିଜ୍ଞା( p->pOrderBy==0 || pDest->eDest!=SRT_Queue );
  ଯଦି( IgnorableDistinct(pDest) ){
    ପ୍ରତିଜ୍ଞା(pDest->eDest==SRT_Exists     || pDest->eDest==SRT_Discard ||
           pDest->eDest==SRT_DistQueue  || pDest->eDest==SRT_DistFifo );
    /* All of these destinations are also able to ignore the ORDER BY clause */
    ଯଦି( p->pOrderBy ){
#ଯଦି TREETRACE_ENABLED
      TREETRACE(0x800,pParse,p, ("dropping superfluous ORDER BY:\n"));
      ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x800 ){
        sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(0, p->pOrderBy, 0, "ORDERBY");
      }
#endif   
      sqlite3ParserAddCleanup(pParse, sqlite3_ପଦସମଷ୍ଟି_List_Delete_Generic,
                              p->pOrderBy);
      testcase( pParse->earlyCleanup );
      p->pOrderBy = 0;
    }
    p->selFlags &= ~(u32)SF_Distinct;
  }
  sqlite3SelectPrep(pParse, p, 0);
  ଯଦି( pParse->nErr ){
    ଯାଅ select_end;
  }
  ପ୍ରତିଜ୍ଞା( db->mallocFailed==0 );
  ପ୍ରତିଜ୍ଞା( p->pEList!=0 );
#ଯଦି TREETRACE_ENABLED
  ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x10 ){
    TREETRACE(0x10,pParse,p, ("after name resolution:\n"));
    sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
  }
#endif

  /* If the SF_UFSrcCheck flag is set, then this function is being called
  ** as part of populating the temp table for an UPDATE...FROM statement.
  ** In this case, it is an error if the target object (pSrc->a[0]) name
  ** or alias is duplicated within FROM clause (pSrc->a[1..n]). 
  **
  ** Postgres disallows this case too. The reason is that some other
  ** systems handle this case differently, and not all the same way,
  ** which is just confusing. To avoid this, we follow PG's lead and
  ** disallow it altogether.  */
  ଯଦି( p->selFlags & SF_UFSrcCheck ){
    SrcItem *p0 = &p->pSrc->a[0];
    ଯଦି( sameSrcAlias(p0, p->pSrc) ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse,
          "target object/alias may not appear in FROM clause: %s",
          p0->zAlias ? p0->zAlias : p0->pSTab->zName
      );
      ଯାଅ select_end;
    }

    /* Clear the SF_UFSrcCheck flag. The check has already been performed,
    ** and leaving this flag set can cause errors if a compound sub-query
    ** in p->pSrc is flattened into this query and this function called
    ** again as part of compound SELECT processing.  */
    p->selFlags &= ~(u32)SF_UFSrcCheck;
  }

  ଯଦି( pDest->eDest==SRT_Output ){
    sqlite3GenerateColumnNames(pParse, p);
  }

#ifndef SQLITE_OMIT_WINDOWFUNC
  ଯଦି( sqlite3WindowRewrite(pParse, p) ){
    ପ୍ରତିଜ୍ଞା( pParse->nErr );
    ଯାଅ select_end;
  }
#ଯଦି TREETRACE_ENABLED
  ଯଦି( p->pWin && (sqlite3_ବୃକ୍ଷ_Trace & 0x40)!=0 ){
    TREETRACE(0x40,pParse,p, ("after window rewrite:\n"));
    sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
  }
#endif
#endif /* SQLITE_OMIT_WINDOWFUNC */
  pTabList = p->pSrc;
  isAgg = (p->selFlags & SF_Aggregate)!=0;
  memset(&sSort, 0, ଆକାର(sSort));
  sSort.pOrderBy = p->pOrderBy;

  /* Try to do various optimizations (flattening subqueries, and strength
  ** reduction of join operators) in the FROM clause up into the main query
  ** tag-select-0200
  */
#ଯଦି !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
  ପାଇଁ(i=0; !p->pPrior && i<pTabList->nSrc; i++){
    SrcItem *pItem = &pTabList->a[i];
    Select *pSub = pItem->fg.isSubquery ? pItem->u4.pSubq->pSelect : 0;
    Table *pTab = pItem->pSTab;

    /* The expander should have already created transient Table objects
    ** even for FROM clause elements such as subqueries that do not correspond
    ** to a real table */
    ପ୍ରତିଜ୍ଞା( pTab!=0 );

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
    ଯଦି( (pItem->fg.jointype & (JT_ବାମ|JT_LTORJ))!=0
     && sqlite3_ପଦସମଷ୍ଟି_Implies_Non_Null_Row(p->pWhere, pItem->iCursor,
                                     pItem->fg.jointype & JT_LTORJ)
     && OptimizationEnabled(db, SQLITE_SimplifyJoin)
    ){
      ଯଦି( pItem->fg.jointype & JT_ବାମ ){
        ଯଦି( pItem->fg.jointype & JT_ଡାହାଣ ){
          TREETRACE(0x1000,pParse,p,
                    ("FULL-JOIN simplifies to RIGHT-JOIN on term %d\n",i));
          pItem->fg.jointype &= ~JT_ବାମ;
        }ଅନ୍ୟଥା{
          TREETRACE(0x1000,pParse,p,
                    ("LEFT-JOIN simplifies to JOIN on term %d\n",i));
          pItem->fg.jointype &= ~(JT_ବାମ|JT_OUTER);
          unset_Join_ପଦସମଷ୍ଟି(p->pWhere, pItem->iCursor, 0);
        }
      }
      ଯଦି( pItem->fg.jointype & JT_LTORJ ){
        ପାଇଁ(j=i+1; j<pTabList->nSrc; j++){
          SrcItem *pI2 = &pTabList->a[j];
          ଯଦି( pI2->fg.jointype & JT_ଡାହାଣ ){
            ଯଦି( pI2->fg.jointype & JT_ବାମ ){
              TREETRACE(0x1000,pParse,p,
                        ("FULL-JOIN simplifies to LEFT-JOIN on term %d\n",j));
              pI2->fg.jointype &= ~JT_ଡାହାଣ;
            }ଅନ୍ୟଥା{
              TREETRACE(0x1000,pParse,p,
                        ("RIGHT-JOIN simplifies to JOIN on term %d\n",j));
              pI2->fg.jointype &= ~(JT_ଡାହାଣ|JT_OUTER);
              unset_Join_ପଦସମଷ୍ଟି(p->pWhere, pI2->iCursor, 1);
            }
          }
        }
        ପାଇଁ(j=pTabList->nSrc-1; j>=0; j--){
          pTabList->a[j].fg.jointype &= ~JT_LTORJ;
          ଯଦି( pTabList->a[j].fg.jointype & JT_ଡାହାଣ ) ଭାଙ୍ଗ;
        }
      }
    }

    /* No further action if this term of the FROM clause is not a subquery */
    ଯଦି( pSub==0 ) ଜାରି;

    /* Catch mismatch in the declared columns of a view and the number of
    ** columns in the SELECT on the RHS */
    ଯଦି( pTab->nCol!=pSub->pEList->n_ପଦସମଷ୍ଟି ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "expected %d columns for '%s' but got %d",
                      pTab->nCol, pTab->zName, pSub->pEList->n_ପଦସମଷ୍ଟି);
      ଯାଅ select_end;
    }

    /* Do not attempt the usual optimizations (flattening and ORDER BY
    ** elimination) on a MATERIALIZED common table expression because
    ** a MATERIALIZED common table expression is an optimization fence.
    */
    ଯଦି( pItem->fg.isCte && pItem->u2.pCteUse->eM10d==M10d_Yes ){
      ଜାରି;
    }

    /* Do not try to flatten an aggregate subquery.
    **
    ** Flattening an aggregate subquery is only possible if the outer query
    ** is not a join.  But if the outer query is not a join, then the subquery
    ** will be implemented as a co-routine and there is no advantage to
    ** flattening in that case.
    */
    ଯଦି( (pSub->selFlags & SF_Aggregate)!=0 ) ଜାରି;
    ପ୍ରତିଜ୍ଞା( pSub->pGroupBy==0 );

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
    ଯଦି( pSub->pOrderBy!=0
     && (p->pOrderBy!=0 || pTabList->nSrc>1)      /* Condition (5) */
     && pSub->pLimit==0                           /* Condition (1) */
     && (pSub->selFlags & (SF_OrderByReqd|SF_Recursive))==0  /* (2) and (6) */
     && (p->selFlags & SF_OrderByReqd)==0         /* Condition (3) and (4) */
     && OptimizationEnabled(db, SQLITE_OmitOrderBy)
    ){
      TREETRACE(0x800,pParse,p,
                ("omit superfluous ORDER BY on %r FROM-clause subquery\n",i+1));
      sqlite3ParserAddCleanup(pParse, sqlite3_ପଦସମଷ୍ଟି_List_Delete_Generic,
                              pSub->pOrderBy);
      pSub->pOrderBy = 0;
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
    ଯଦି( pSub->pOrderBy!=0
     && i==0
     && (p->selFlags & SF_ComplexResult)!=0
     && (pTabList->nSrc==1
         || (pTabList->a[1].fg.jointype&(JT_OUTER|JT_CROSS))!=0)
    ){
      ଜାରି;
    }

    /* tag-select-0240 */
    ଯଦି( flattenSubquery(pParse, p, i, isAgg) ){
      ଯଦି( pParse->nErr ) ଯାଅ select_end;
      /* This subquery can be absorbed into its parent. */
      i = -1;
    }
    pTabList = p->pSrc;
    ଯଦି( db->mallocFailed ) ଯାଅ select_end;
    ଯଦି( !IgnorableOrderby(pDest) ){
      sSort.pOrderBy = p->pOrderBy;
    }
  }
#endif

#ifndef SQLITE_OMIT_COMPOUND_SELECT
  /* Handle compound SELECT statements using the separate multiSelect()
  ** procedure.  tag-select-0300
  */
  ଯଦି( p->pPrior ){
    rc = multiSelect(pParse, p, pDest);
#ଯଦି TREETRACE_ENABLED
    TREETRACE(0x400,pParse,p,("end compound-select processing\n"));
    ଯଦି( (sqlite3_ବୃକ୍ଷ_Trace & 0x400)!=0 && ExplainQueryPlanParent(pParse)==0 ){
      sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
    }
#endif
    ଯଦି( p->pNext==0 ) Explain_Query_Plan_କାଢ଼(pParse);
    ଫେରନ୍ତୁ rc;
  }
#endif

  /* If there may be an "EXISTS (SELECT ...)" in the WHERE clause, attempt
  ** to change it into a join.  */
  ଯଦି( pParse->bHasExists && OptimizationEnabled(db,SQLITE_ExistsToJoin) ){
    existsToJoin(pParse, p, p->pWhere);
    pTabList = p->pSrc;
  }

  /* Do the WHERE-clause constant propagation optimization if this is
  ** a join.  No need to spend time on this operation for non-join queries
  ** as the equivalent optimization will be handled by query planner in
  ** sqlite3WhereBegin().  tag-select-0330
  */
  ଯଦି( p->pWhere!=0
   && p->pWhere->op==TK_AND
   && OptimizationEnabled(db, SQLITE_PropagateConst)
   && propagateConstants(pParse, p)
  ){
#ଯଦି TREETRACE_ENABLED
    ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x2000 ){
      TREETRACE(0x2000,pParse,p,("After constant propagation:\n"));
      sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
    }
#endif
  }ଅନ୍ୟଥା{
    TREETRACE(0x2000,pParse,p,("Constant propagation not helpful\n"));
  }

  /* tag-select-0350 */
  ଯଦି( OptimizationEnabled(db, SQLITE_QueryFlattener|SQLITE_ଗଣନା_Of_View)
   && ଗଣନା_Of_View_Optimization(pParse, p)
  ){
    ଯଦି( db->mallocFailed ) ଯାଅ select_end;
    pTabList = p->pSrc;
  }

  /* Loop over all terms in the FROM clause and do two things for each term:
  **
  **   (1) Authorize unreferenced tables
  **   (2) Generate code for all sub-queries
  **
  ** tag-select-0400
  */
  ପାଇଁ(i=0; i<pTabList->nSrc; i++){
    SrcItem *pItem = &pTabList->a[i];
    SrcItem *pPrior;
    SelectDest dest;
    Subquery *pSubq;
    Select *pSub;
#ଯଦି !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    ଅଚଳ ଅକ୍ଷର *zSavedAuthContext;
#endif

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
    ଯଦି( pItem->colUsed==0 && pItem->zName!=0 ){
      ଅଚଳ ଅକ୍ଷର *zDb;
      ଯଦି( pItem->fg.fixedSchema ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = sqlite3SchemaToIndex(pParse->db, pItem->u4.pSchema);
        zDb = db->aDb[iDb].zDbSName;
      }ଅନ୍ୟଥା ଯଦି( pItem->fg.isSubquery ){
        zDb = 0;
      }ଅନ୍ୟଥା{
        zDb = pItem->u4.zDatabase;
      }
      sqlite3AuthCheck(pParse, SQLITE_READ, pItem->zName, "", zDb);
    }

#ଯଦି !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    /* Generate code for all sub-queries in the FROM clause
    */
    ଯଦି( pItem->fg.isSubquery==0 ) ଜାରି;
    pSubq = pItem->u4.pSubq;
    ପ୍ରତିଜ୍ଞା( pSubq!=0 );
    pSub = pSubq->pSelect;

    /* The code for a subquery should only be generated once. */
    ଯଦି( pSubq->addrFillSub!=0 ) ଜାରି;

    /* Increment Parse.nHeight by the height of the largest expression
    ** tree referred to by this, the parent select. The child select
    ** may contain expression trees of at most
    ** (SQLITE_MAX_EXPR_DEPTH-Parse.nHeight) height. This is a bit
    ** more conservative than necessary, but much easier than enforcing
    ** an exact limit.
    */
    pParse->nHeight += sqlite3_Select_ପଦସମଷ୍ଟି_Height(p);

    /* Make copies of constant WHERE-clause terms in the outer query down
    ** inside the subquery.  This can help the subquery to run more efficiently.
    ** This is the "predicate push-down optimization".  tag-select-0420
    */
    ଯଦି( OptimizationEnabled(db, SQLITE_ଠେଲ_Down)
     && (pItem->fg.isCte==0
         || (pItem->u2.pCteUse->eM10d!=M10d_Yes && pItem->u2.pCteUse->nUse<2))
     && ଠେଲ_Down_Where_Terms(pParse, pSub, p->pWhere, pTabList, i)
    ){
#ଯଦି TREETRACE_ENABLED
      ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("After WHERE-clause push-down into subquery %d:\n", pSub->selId));
        sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
      }
#endif
      ପ୍ରତିଜ୍ଞା( pSubq->pSelect && (pSub->selFlags & SF_ଠେଲ_Down)!=0 );
    }ଅନ୍ୟଥା{
      TREETRACE(0x4000,pParse,p,("WHERE-clause push-down not possible\n"));
    }

    /* Convert unused result columns of the subquery into simple NULL
    ** expressions, to avoid unneeded searching and computation.
    ** tag-select-0440
    */
    ଯଦି( OptimizationEnabled(db, SQLITE_NullUnusedCols)
     && disableUnusedSubqueryResultColumns(pItem)
    ){
#ଯଦି TREETRACE_ENABLED
      ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("Change unused result columns to NULL for subquery %d:\n",
             pSub->selId));
        sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
      }
#endif
    }

    zSavedAuthContext = pParse->zAuthContext;
    pParse->zAuthContext = pItem->zName;

    /* Generate byte-code to implement the subquery  tag-select-0480
    */
    ଯଦି( fromClauseTermCanBeCoroutine(pParse, pTabList, i, p->selFlags) ){
      /* Implement a co-routine that will return a single row of the result
      ** set on each invocation.  tag-select-0482
      */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr_ଶୀର୍ଷ = sqlite3VdbeCurrentAddr(v)+1;
    
      pSubq->regReturn = ++pParse->nMem;
      sqlite3VdbeAddOp3(v, OP_InitCoroutine, pSubq->regReturn, 0, addr_ଶୀର୍ଷ);
      VdbeComment((v, "%!S", pItem));
      pSubq->addrFillSub = addr_ଶୀର୍ଷ;
      sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);
      ExplainQueryPlan((pParse, 1, "CO-ROUTINE %!S", pItem));
      sqlite3Select(pParse, pSub, &dest);
      pItem->pSTab->nRowLogEst = pSub->nSelectRow;
      pItem->fg.viaCoroutine = 1;
      pSubq->regResult = dest.iSdst;
      sqlite3VdbeEndCoroutine(v, pSubq->regReturn);
      VdbeComment((v, "end %!S", pItem));
      sqlite3VdbeJumpHere(v, addr_ଶୀର୍ଷ-1);
      sqlite3ClearTempRegCache(pParse);
    }ଅନ୍ୟଥା ଯଦି( pItem->fg.isCte && pItem->u2.pCteUse->addrM9e>0 ){
      /* This is a CTE for which materialization code has already been
      ** generated.  Invoke the subroutine to compute the materialization,
      ** then make the pItem->iCursor be a copy of the ephemeral table that
      ** holds the result of the materialization. tag-select-0484 */
      CteUse *pCteUse = pItem->u2.pCteUse;
      sqlite3VdbeAddOp2(v, OP_Gosub, pCteUse->regRtn, pCteUse->addrM9e);
      ଯଦି( pItem->iCursor!=pCteUse->iCur ){
        sqlite3VdbeAddOp2(v, OP_OpenDup, pItem->iCursor, pCteUse->iCur);
        VdbeComment((v, "%!S", pItem));
      }
      pSub->nSelectRow = pCteUse->nRowEst;
    }ଅନ୍ୟଥା ଯଦି( (pPrior = isSelfJoinView(pTabList, pItem, 0, i))!=0 ){
      /* This view has already been materialized by a prior entry in
      ** this same FROM clause.  Reuse it.  tag-select-0486 */
      Subquery *pPriorSubq;
      ପ୍ରତିଜ୍ଞା( pPrior->fg.isSubquery );
      pPriorSubq = pPrior->u4.pSubq;
      ପ୍ରତିଜ୍ଞା( pPriorSubq!=0 );
      ଯଦି( pPriorSubq->addrFillSub ){
        sqlite3VdbeAddOp2(v, OP_Gosub, pPriorSubq->regReturn,
                                       pPriorSubq->addrFillSub);
      }
      sqlite3VdbeAddOp2(v, OP_OpenDup, pItem->iCursor, pPrior->iCursor);
      pSub->nSelectRow = pPriorSubq->pSelect->nSelectRow;
    }ଅନ୍ୟଥା{
      /* Materialize the view.  If the view is not correlated, generate a
      ** subroutine to do the materialization so that subsequent uses of
      ** the same view can reuse the materialization.  tag-select-0488 */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଶୀର୍ଷ_Addr;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା onceAddr = 0;
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrExplain;
#endif

      pSubq->regReturn = ++pParse->nMem;
      ଶୀର୍ଷ_Addr = sqlite3VdbeAddOp0(v, OP_Goto);
      pSubq->addrFillSub = ଶୀର୍ଷ_Addr+1;
      pItem->fg.isMaterialized = 1;
      ଯଦି( pItem->fg.isCorrelated==0 ){
        /* If the subquery is not correlated and if we are not inside of
        ** a trigger, then we only need to compute the value of the subquery
        ** once. */
        onceAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
        VdbeComment((v, "materialize %!S", pItem));
      }ଅନ୍ୟଥା{
        VdbeNoopComment((v, "materialize %!S", pItem));
      }
      sqlite3SelectDestInit(&dest, SRT_EphemTab, pItem->iCursor);

      ExplainQueryPlan2(addrExplain, (pParse, 1, "MATERIALIZE %!S", pItem));
      sqlite3Select(pParse, pSub, &dest);
      pItem->pSTab->nRowLogEst = pSub->nSelectRow;
      ଯଦି( onceAddr ) sqlite3VdbeJumpHere(v, onceAddr);
      sqlite3VdbeAddOp2(v, OP_Return, pSubq->regReturn, ଶୀର୍ଷ_Addr+1);
      VdbeComment((v, "end %!S", pItem));
      sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);
      sqlite3VdbeJumpHere(v, ଶୀର୍ଷ_Addr);
      sqlite3ClearTempRegCache(pParse);
      ଯଦି( pItem->fg.isCte && pItem->fg.isCorrelated==0 ){
        CteUse *pCteUse = pItem->u2.pCteUse;
        pCteUse->addrM9e = pSubq->addrFillSub;
        pCteUse->regRtn = pSubq->regReturn;
        pCteUse->iCur = pItem->iCursor;
        pCteUse->nRowEst = pSub->nSelectRow;
      }
    }
    ଯଦି( db->mallocFailed ) ଯାଅ select_end;
    pParse->nHeight -= sqlite3_Select_ପଦସମଷ୍ଟି_Height(p);
    pParse->zAuthContext = zSavedAuthContext;
#endif
  }

  /* Various elements of the SELECT copied into local variables for
  ** convenience */
  pEList = p->pEList;
  pWhere = p->pWhere;
  pGroupBy = p->pGroupBy;
  pHaving = p->pHaving;
  sDistinct.isTnct = (p->selFlags & SF_Distinct)!=0;

#ଯଦି TREETRACE_ENABLED
  ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x8000 ){
    TREETRACE(0x8000,pParse,p,("After all FROM-clause analysis:\n"));
    sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
  }
#endif

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
  ଯଦି( (p->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct
   && sqlite3CopySortOrder(pEList, sSort.pOrderBy)
   && sqlite3_ପଦସମଷ୍ଟି_List_Compare(pEList, sSort.pOrderBy, -1)==0
   && OptimizationEnabled(db, SQLITE_GroupByOrder)
#ifndef SQLITE_OMIT_WINDOWFUNC
   && p->pWin==0
#endif
  ){
    p->selFlags &= ~(u32)SF_Distinct;
    pGroupBy = p->pGroupBy = sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, pEList, 0);
    ଯଦି( pGroupBy ){
      ପାଇଁ(i=0; i<pGroupBy->n_ପଦସମଷ୍ଟି; i++){
        pGroupBy->a[i].u.x.iOrderByCol = i+1;
      }
    }
    p->selFlags |= SF_Aggregate;
    /* Notice that even thought SF_Distinct has been cleared from p->selFlags,
    ** the sDistinct.isTnct is still set.  Hence, isTnct represents the
    ** original setting of the SF_Distinct flag, not the current setting */
    ପ୍ରତିଜ୍ଞା( sDistinct.isTnct );
    sDistinct.isTnct = 2;

#ଯଦି TREETRACE_ENABLED
    ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x20000 ){
      TREETRACE(0x20000,pParse,p,("Transform DISTINCT into GROUP BY:\n"));
      sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
    }
#endif
  }

  /* If there is an ORDER BY clause, then create an ephemeral index to
  ** do the sorting.  But this sorting ephemeral index might end up
  ** being unused if the data can be extracted in pre-sorted order.
  ** If that is the case, then the OP_OpenEphemeral instruction will be
  ** changed to an OP_Noop once we figure out that the sorting index is
  ** not needed.  The sSort.addrSortIndex variable is used to facilitate
  ** that change.  tag-select-0600
  */
  ଯଦି( sSort.pOrderBy ){
    ଚାବି_Info *p_ଚାବି_Info;
    p_ଚାବି_Info = sqlite3_ଚାବି_Info_From_ପଦସମଷ୍ଟି_List(
        pParse, sSort.pOrderBy, 0, pEList->n_ପଦସମଷ୍ଟି);
    sSort.iECursor = pParse->nTab++;
    sSort.addrSortIndex =
      sqlite3VdbeAddOp4(v, OP_OpenEphemeral,
          sSort.iECursor, sSort.pOrderBy->n_ପଦସମଷ୍ଟି+1+pEList->n_ପଦସମଷ୍ଟି, 0,
          (ଅକ୍ଷର*)p_ଚାବି_Info, P4_KEYINFO
      );
  }ଅନ୍ୟଥା{
    sSort.addrSortIndex = -1;
  }

  /* If the output is destined for a temporary table, open that table.
  ** tag-select-0630
  */
  ଯଦି( pDest->eDest==SRT_EphemTab ){
    sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pDest->iSDParm, pEList->n_ପଦସମଷ୍ଟି);
    ଯଦି( p->selFlags & SF_NestedFrom ){
      /* Delete or NULL-out result columns that will never be used */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
      ପାଇଁ(ii=pEList->n_ପଦସମଷ୍ଟି-1; ii>0 && pEList->a[ii].fg.bUsed==0; ii--){
        sqlite3_ପଦସମଷ୍ଟି_Delete(db, pEList->a[ii].p_ପଦସମଷ୍ଟି);
        sqlite3DbFree(db, pEList->a[ii].zEName);
        pEList->n_ପଦସମଷ୍ଟି--;
      }
      ପାଇଁ(ii=0; ii<pEList->n_ପଦସମଷ୍ଟି; ii++){
        ଯଦି( pEList->a[ii].fg.bUsed==0 ) pEList->a[ii].p_ପଦସମଷ୍ଟି->op = TK_NULL;
      }
    }
  }

  /* Set the limiter.  tag-select-0650
  */
  iEnd = sqlite3VdbeMakeLabel(pParse);
  ଯଦି( (p->selFlags & SF_FixedLimit)==0 ){
    p->nSelectRow = 320;  /* 4 billion rows */
  }
  ଯଦି( p->pLimit ) computeLimitRegisters(pParse, p, iEnd);
  ଯଦି( p->iLimit==0 && sSort.addrSortIndex>=0 ){
    sqlite3VdbeChangeOpcode(v, sSort.addrSortIndex, OP_SorterOpen);
    sSort.sortFlags |= SORTFLAG_UseSorter;
  }

  /* Open an ephemeral index to use for the distinct set. tag-select-0680
  */
  ଯଦି( p->selFlags & SF_Distinct ){
    sDistinct.tabTnct = pParse->nTab++;
    sDistinct.addrTnct = sqlite3VdbeAddOp4(v, OP_OpenEphemeral,
                       sDistinct.tabTnct, 0, 0,
                       (ଅକ୍ଷର*)sqlite3_ଚାବି_Info_From_ପଦସମଷ୍ଟି_List(pParse, p->pEList,0,0),
                       P4_KEYINFO);
    sqlite3VdbeChangeP5(v, BTREE_UNORDERED);
    sDistinct.eTnctType = WHERE_DISTINCT_UNORDERED;
  }ଅନ୍ୟଥା{
    sDistinct.eTnctType = WHERE_DISTINCT_NOOP;
  }

  ଯଦି( !isAgg && pGroupBy==0 ){
    /* No aggregate functions and no GROUP BY clause.  tag-select-0700 */
    u16 wctrlFlags = (sDistinct.isTnct ? WHERE_WANT_DISTINCT : 0)
                   | (p->selFlags & SF_FixedLimit);
#ifndef SQLITE_OMIT_WINDOWFUNC
    Window *pWin = p->pWin;      /* Main window object (or NULL) */
    ଯଦି( pWin ){
      sqlite3WindowCodeInit(pParse, p);
    }
#endif
    ପ୍ରତିଜ୍ଞା( WHERE_USE_LIMIT==SF_FixedLimit );


    /* Begin the database scan. */
    TREETRACE(0x2,pParse,p,("WhereBegin\n"));
    pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, sSort.pOrderBy,
                               p->pEList, p, wctrlFlags, p->nSelectRow);
    ଯଦି( pWInfo==0 ) ଯାଅ select_end;
    ଯଦି( sqlite3_Where_Output_Row_ଗଣନା(pWInfo) < p->nSelectRow ){
      p->nSelectRow = sqlite3_Where_Output_Row_ଗଣନା(pWInfo);
      ଯଦି( pDest->eDest<=SRT_DistQueue && pDest->eDest>=SRT_DistFifo ){
        /* TUNING: For a UNION CTE, because UNION is implies DISTINCT,
        ** reduce the estimated output row count by 8 (LogEst 30). 
        ** Search for tag-20250414a to see other cases */
        p->nSelectRow -= 30;
      }
    }
    ଯଦି( sDistinct.isTnct && sqlite3WhereIsDistinct(pWInfo) ){
      sDistinct.eTnctType = sqlite3WhereIsDistinct(pWInfo);
    }
    ଯଦି( sSort.pOrderBy ){
      sSort.nOBSat = sqlite3WhereIsOrdered(pWInfo);
      sSort.labelOBLopt = sqlite3WhereOrderByLimitOptLabel(pWInfo);
      ଯଦି( sSort.nOBSat==sSort.pOrderBy->n_ପଦସମଷ୍ଟି ){
        sSort.pOrderBy = 0;
      }
    }
    TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));

    /* If sorting index that was created by a prior OP_OpenEphemeral
    ** instruction ended up not being needed, then change the OP_OpenEphemeral
    ** into an OP_Noop.
    */
    ଯଦି( sSort.addrSortIndex>=0 && sSort.pOrderBy==0 ){
      sqlite3VdbeChangeToNoop(v, sSort.addrSortIndex);
    }

    ପ୍ରତିଜ୍ଞା( p->pEList==pEList );
#ifndef SQLITE_OMIT_WINDOWFUNC
    ଯଦି( pWin ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrGosub = sqlite3VdbeMakeLabel(pParse);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCont = sqlite3VdbeMakeLabel(pParse);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iBreak = sqlite3VdbeMakeLabel(pParse);
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା regGosub = ++pParse->nMem;

      sqlite3WindowCodeStep(pParse, p, pWInfo, regGosub, addrGosub);

      sqlite3VdbeAddOp2(v, OP_Goto, 0, iBreak);
      sqlite3VdbeResolveLabel(v, addrGosub);
      VdbeNoopComment((v, "inner-loop subroutine"));
      sSort.labelOBLopt = 0;
      selectInnerLoop(pParse, p, -1, &sSort, &sDistinct, pDest, iCont, iBreak);
      sqlite3VdbeResolveLabel(v, iCont);
      sqlite3VdbeAddOp1(v, OP_Return, regGosub);
      VdbeComment((v, "end inner-loop subroutine"));
      sqlite3VdbeResolveLabel(v, iBreak);
    }ଅନ୍ୟଥା
#endif /* SQLITE_OMIT_WINDOWFUNC */
    {
      /* Use the standard inner loop. */
      selectInnerLoop(pParse, p, -1, &sSort, &sDistinct, pDest,
          sqlite3WhereContinueLabel(pWInfo),
          sqlite3WhereBreakLabel(pWInfo));

      /* End the database scan loop.
      */
      TREETRACE(0x2,pParse,p,("WhereEnd\n"));
      sqlite3WhereEnd(pWInfo);
    }
  }ଅନ୍ୟଥା{
    /* This case is for when there exist aggregate functions or a GROUP BY
    ** clause or both.  tag-select-0800 */
    NameContext sNC;    /* Name context for processing aggregate information */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAMem;          /* First Mem address for storing current GROUP BY */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iBMem;          /* First Mem address for previous GROUP BY */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iUseFlag;       /* Mem address holding flag indicating that at least
                        ** one row of the input to the aggregator has been
                        ** processed */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAbortFlag;     /* Mem address which causes query abort if positive */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା groupBySort;    /* Rows come from source in GROUP BY order */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrEnd;        /* End of processing for this SELECT */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା sortPTab = 0;   /* Pseudotable used to decode sorting results */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା sortOut = 0;    /* Output register from the sorter */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା orderByGrp = 0; /* True if the GROUP BY and ORDER BY are the same */

    /* Remove any and all aliases between the result set and the
    ** GROUP BY clause.
    */
    ଯଦି( pGroupBy ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା k;                        /* Loop counter */
      ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;  /* For looping over expression in a list */

      ପାଇଁ(k=p->pEList->n_ପଦସମଷ୍ଟି, pItem=p->pEList->a; k>0; k--, pItem++){
        pItem->u.x.iAlias = 0;
      }
      ପାଇଁ(k=pGroupBy->n_ପଦସମଷ୍ଟି, pItem=pGroupBy->a; k>0; k--, pItem++){
        pItem->u.x.iAlias = 0;
      }
      ପ୍ରତିଜ୍ଞା( 66==sqlite3LogEst(100) );
      ଯଦି( p->nSelectRow>66 ) p->nSelectRow = 66;

      /* If there is both a GROUP BY and an ORDER BY clause and they are
      ** identical, then it may be possible to disable the ORDER BY clause
      ** on the grounds that the GROUP BY will cause elements to come out
      ** in the correct order. It also may not - the GROUP BY might use a
      ** database index that causes rows to be grouped together as required
      ** but not actually sorted. Either way, record the fact that the
      ** ORDER BY and GROUP BY clauses are the same by setting the orderByGrp
      ** variable.  */
      ଯଦି( sqlite3CopySortOrder(pGroupBy, sSort.pOrderBy)
       && sqlite3_ପଦସମଷ୍ଟି_List_Compare(pGroupBy, sSort.pOrderBy, -1)==0
      ){
        orderByGrp = 1;
      }
    }ଅନ୍ୟଥା{
      ପ୍ରତିଜ୍ଞା( 0==sqlite3LogEst(1) );
      p->nSelectRow = 0;
    }

    /* Create a label to jump to when we want to abort the query */
    addrEnd = sqlite3VdbeMakeLabel(pParse);

    /* Convert TK_COLUMN nodes into TK_AGG_COLUMN and make entries in
    ** sAggInfo for all TK_AGG_FUNCTION nodes in expressions of the
    ** SELECT statement.
    */
    pAggInfo = sqlite3DbMallocZero(db, ଆକାର(*pAggInfo) );
    ଯଦି( pAggInfo ){
      sqlite3ParserAddCleanup(pParse, agginfoFree, pAggInfo);
      testcase( pParse->earlyCleanup );
    }
    ଯଦି( db->mallocFailed ){
      ଯାଅ select_end;
    }
    pAggInfo->selId = p->selId;
#ifdef SQLITE_DEBUG
    pAggInfo->pSelect = p;
#endif
    memset(&sNC, 0, ଆକାର(sNC));
    sNC.pParse = pParse;
    sNC.pSrcList = pTabList;
    sNC.uNC.pAggInfo = pAggInfo;
    VVA_ONLY( sNC.ncFlags = NC_UAggInfo; )
    pAggInfo->nSortingColumn = pGroupBy ? pGroupBy->n_ପଦସମଷ୍ଟି : 0;
    pAggInfo->pGroupBy = pGroupBy;
    sqlite3_ପଦସମଷ୍ଟି_Analyze_Agg_List(&sNC, pEList);
    sqlite3_ପଦସମଷ୍ଟି_Analyze_Agg_List(&sNC, sSort.pOrderBy);
    ଯଦି( pHaving ){
      ଯଦି( pGroupBy ){
        ପ୍ରତିଜ୍ଞା( pWhere==p->pWhere );
        ପ୍ରତିଜ୍ଞା( pHaving==p->pHaving );
        ପ୍ରତିଜ୍ଞା( pGroupBy==p->pGroupBy );
        havingToWhere(pParse, p);
        pWhere = p->pWhere;
      }
      sqlite3_ପଦସମଷ୍ଟି_Analyze_Aggregates(&sNC, pHaving);
    }
    pAggInfo->nAccumulator = pAggInfo->nColumn;
    ଯଦି( p->pGroupBy==0 && p->pHaving==0 && pAggInfo->nFunc==1 ){
      minMaxFlag = minMaxQuery(db, pAggInfo->aFunc[0].pFExpr, &pMinMaxOrderBy);
    }ଅନ୍ୟଥା{
      minMaxFlag = WHERE_ORDERBY_NORMAL;
    }
    analyzeAggFuncArgs(pAggInfo, &sNC);
    ଯଦି( db->mallocFailed ) ଯାଅ select_end;
#ଯଦି TREETRACE_ENABLED
    ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("After aggregate analysis %p:\n", pAggInfo));
      sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
      ଯଦି( minMaxFlag ){
        sqlite3DebugPrintf("MIN/MAX Optimization (0x%02x) adds:\n", minMaxFlag);
        sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(0, pMinMaxOrderBy, 0, "ORDERBY");
      }
      printAggInfo(pAggInfo);
    }
#endif


    /* Processing for aggregates with GROUP BY is very different and
    ** much more complex than aggregates without a GROUP BY.  tag-select-0810
    */
    ଯଦି( pGroupBy ){
      ଚାବି_Info *p_ଚାବି_Info;  /* Keying information for the group by clause */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr1;          /* A-vs-B comparison jump */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrOutputRow;  /* Start of subroutine that outputs a result row */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା regOutputRow;   /* Return address register for output subroutine */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrSetAbort;   /* Set the abort flag and return */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr_ଶୀର୍ଷ_Of_Loop;  /* Top of the input loop */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrSortingIdx; /* The OP_OpenEphemeral for the sorting index */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrReset;      /* Subroutine for resetting the accumulator */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା regReset;       /* Return address register for reset subroutine */
      ପଦସମଷ୍ଟି_List *pDistinct = 0;
      u16 distFlag = 0;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା eDist = WHERE_DISTINCT_NOOP;

      ଯଦି( pAggInfo->nFunc==1
       && pAggInfo->aFunc[0].iDistinct>=0
       && ALWAYS(pAggInfo->aFunc[0].pFExpr!=0)
       && ALWAYS(ପଦସମଷ୍ଟି_Use_XList(pAggInfo->aFunc[0].pFExpr))
       && pAggInfo->aFunc[0].pFExpr->x.pList!=0
      ){
        ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି = pAggInfo->aFunc[0].pFExpr->x.pList->a[0].p_ପଦସମଷ୍ଟି;
        p_ପଦସମଷ୍ଟି = sqlite3_ପଦସମଷ୍ଟି_Dup(db, p_ପଦସମଷ୍ଟି, 0);
        pDistinct = sqlite3_ପଦସମଷ୍ଟି_List_Dup(db, pGroupBy, 0);
        pDistinct = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, pDistinct, p_ପଦସମଷ୍ଟି);
        distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
      }

      /* If there is a GROUP BY clause we might need a sorting index to
      ** implement it.  Allocate that sorting index now.  If it turns out
      ** that we do not need it after all, the OP_SorterOpen instruction
      ** will be converted into a Noop. 
      */
      pAggInfo->sortingIdx = pParse->nTab++;
      p_ଚାବି_Info = sqlite3_ଚାବି_Info_From_ପଦସମଷ୍ଟି_List(pParse, pGroupBy,
                                            0, pAggInfo->nColumn);
      addrSortingIdx = sqlite3VdbeAddOp4(v, OP_SorterOpen,
          pAggInfo->sortingIdx, pAggInfo->nSortingColumn,
          0, (ଅକ୍ଷର*)p_ଚାବି_Info, P4_KEYINFO);

      /* Initialize memory locations used by GROUP BY aggregate processing
      */
      iUseFlag = ++pParse->nMem;
      iAbortFlag = ++pParse->nMem;
      regOutputRow = ++pParse->nMem;
      addrOutputRow = sqlite3VdbeMakeLabel(pParse);
      regReset = ++pParse->nMem;
      addrReset = sqlite3VdbeMakeLabel(pParse);
      iAMem = pParse->nMem + 1;
      pParse->nMem += pGroupBy->n_ପଦସମଷ୍ଟି;
      iBMem = pParse->nMem + 1;
      pParse->nMem += pGroupBy->n_ପଦସମଷ୍ଟି;
      sqlite3VdbeAddOp2(v, OP_Integer, 0, iAbortFlag);
      VdbeComment((v, "clear abort flag"));
      sqlite3VdbeAddOp3(v, OP_Null, 0, iAMem, iAMem+pGroupBy->n_ପଦସମଷ୍ଟି-1);
      sqlite3_ପଦସମଷ୍ଟି_Null_Register_Range(pParse, iAMem, pGroupBy->n_ପଦସମଷ୍ଟି);

      /* Begin a loop that will extract all source rows in GROUP BY order.
      ** This might involve two separate loops with an OP_Sort in between, or
      ** it might be a single loop that uses an index to extract information
      ** in the right order to begin with.
      */
      sqlite3VdbeAddOp2(v, OP_Gosub, regReset, addrReset);
      TREETRACE(0x2,pParse,p,("WhereBegin\n"));
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, pGroupBy, pDistinct,
          p, (sDistinct.isTnct==2 ? WHERE_DISTINCTBY : WHERE_GROUPBY)
          |  (orderByGrp ? WHERE_SORTBYGROUP : 0) | distFlag, 0
      );
      ଯଦି( pWInfo==0 ){
        sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pDistinct);
        ଯାଅ select_end;
      }
      ଯଦି( pParse->pIdxEpr ){
        optimize_Aggregate_Use_Of_Indexed_ପଦସମଷ୍ଟି(pParse, p, pAggInfo, &sNC);
      }
      assignAggregateRegisters(pParse, pAggInfo);
      eDist = sqlite3WhereIsDistinct(pWInfo);
      TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
      ଯଦି( sqlite3WhereIsOrdered(pWInfo)==pGroupBy->n_ପଦସମଷ୍ଟି ){
        /* The optimizer is able to deliver rows in group by order so
        ** we do not have to sort.  The OP_OpenEphemeral table will be
        ** cancelled later because we still need to use the pKeyInfo
        */
        groupBySort = 0;
      }ଅନ୍ୟଥା{
        /* Rows are coming out in undetermined order.  We have to push
        ** each row into a sorting index, terminate the first loop,
        ** then loop over the sorting index in order to get the output
        ** in sorted order
        */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା regBase;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା regRecord;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nGroupBy;

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrExp;              /* Address of OP_Explain instruction */
#endif
        ExplainQueryPlan2(addrExp, (pParse, 0, "USE TEMP B-TREE FOR %s",
            (sDistinct.isTnct && (p->selFlags&SF_Distinct)==0) ?
                    "DISTINCT" : "GROUP BY"
        ));

        groupBySort = 1;
        nGroupBy = pGroupBy->n_ପଦସମଷ୍ଟି;
        nCol = nGroupBy;
        j = nGroupBy;
        ପାଇଁ(i=0; i<pAggInfo->nColumn; i++){
          ଯଦି( pAggInfo->aCol[i].iSorterColumn>=j ){
            nCol++;
            j++;
          }
        }
        regBase = sqlite3GetTempRange(pParse, nCol);
        sqlite3_ପଦସମଷ୍ଟି_Code_ପଦସମଷ୍ଟି_List(pParse, pGroupBy, regBase, 0, 0);
        j = nGroupBy;
        pAggInfo->directMode = 1;
        ପାଇଁ(i=0; i<pAggInfo->nColumn; i++){
          ସଂରଚନା AggInfo_col *pCol = &pAggInfo->aCol[i];
          ଯଦି( pCol->iSorterColumn>=j ){
            sqlite3_ପଦସମଷ୍ଟି_Code(pParse, pCol->pCExpr, j + regBase);
            j++;
          }
        }
        pAggInfo->directMode = 0;
        regRecord = sqlite3GetTempReg(pParse);
        sqlite3VdbeScanStatusCounters(v, addrExp, 0, sqlite3VdbeCurrentAddr(v));
        sqlite3VdbeAddOp3(v, OP_MakeRecord, regBase, nCol, regRecord);
        sqlite3VdbeAddOp2(v, OP_Sorter_ଭର୍ତ୍ତି, pAggInfo->sortingIdx, regRecord);
        sqlite3VdbeScanStatusRange(v, addrExp, sqlite3VdbeCurrentAddr(v)-2, -1);
        sqlite3ReleaseTempReg(pParse, regRecord);
        sqlite3ReleaseTempRange(pParse, regBase, nCol);
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3WhereEnd(pWInfo);
        pAggInfo->sortingIdxPTab = sortPTab = pParse->nTab++;
        sortOut = sqlite3GetTempReg(pParse);
        sqlite3VdbeScanStatusCounters(v, addrExp, sqlite3VdbeCurrentAddr(v), 0);
        sqlite3VdbeAddOp3(v, OP_OpenPseudo, sortPTab, sortOut, nCol);
        sqlite3VdbeAddOp2(v, OP_SorterSort, pAggInfo->sortingIdx, addrEnd);
        VdbeComment((v, "GROUP BY sort")); VdbeCoverage(v);
        pAggInfo->useSortingIdx = 1;
        sqlite3VdbeScanStatusRange(v, addrExp, -1, sortPTab);
        sqlite3VdbeScanStatusRange(v, addrExp, -1, pAggInfo->sortingIdx);
      }

      /* If there are entries in pAgggInfo->aFunc[] that contain subexpressions
      ** that are indexed (and that were previously identified and tagged
      ** in optimizeAggregateUseOfIndexedExpr()) then those subexpressions
      ** must now be converted into a TK_AGG_COLUMN node so that the value
      ** is correctly pulled from the index rather than being recomputed. */
      ଯଦି( pParse->pIdxEpr ){
        aggregate_Convert_Indexed_ପଦସମଷ୍ଟି_Ref_To_Column(pAggInfo);
#ଯଦି TREETRACE_ENABLED
        ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x20 ){
          TREETRACE(0x20, pParse, p,
             ("AggInfo function expressions converted to reference index\n"));
          sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
          printAggInfo(pAggInfo);
        }
#endif
      }

      /* If the index or temporary table used by the GROUP BY sort
      ** will naturally deliver rows in the order required by the ORDER BY
      ** clause, cancel the ephemeral table open coded earlier.
      **
      ** This is an optimization - the correct answer should result regardless.
      ** Use the SQLITE_GroupByOrder flag with SQLITE_TESTCTRL_OPTIMIZER to
      ** disable this optimization for testing purposes.  */
      ଯଦି( orderByGrp && OptimizationEnabled(db, SQLITE_GroupByOrder)
       && (groupBySort || sqlite3WhereIsSorted(pWInfo))
      ){
        sSort.pOrderBy = 0;
        sqlite3VdbeChangeToNoop(v, sSort.addrSortIndex);
      }

      /* Evaluate the current GROUP BY terms and store in b0, b1, b2...
      ** (b0 is memory location iBMem+0, b1 is iBMem+1, and so forth)
      ** Then compare the current GROUP BY terms against the GROUP BY terms
      ** from the previous row currently stored in a0, a1, a2...
      */
      addr_ଶୀର୍ଷ_Of_Loop = sqlite3VdbeCurrentAddr(v);
      ଯଦି( groupBySort ){
        sqlite3VdbeAddOp3(v, OP_SorterData, pAggInfo->sortingIdx,
                          sortOut, sortPTab);
      }
      ପାଇଁ(j=0; j<pGroupBy->n_ପଦସମଷ୍ଟି; j++){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOrderByCol = pGroupBy->a[j].u.x.iOrderByCol;
        
        ଯଦି( groupBySort ){
          sqlite3VdbeAddOp3(v, OP_Column, sortPTab, j, iBMem+j);
        }ଅନ୍ୟଥା{
          pAggInfo->directMode = 1;
          sqlite3_ପଦସମଷ୍ଟି_Code(pParse, pGroupBy->a[j].p_ପଦସମଷ୍ଟି, iBMem+j);
        }

        ଯଦି( iOrderByCol ){
          ପଦସମଷ୍ଟି *pX = p->pEList->a[iOrderByCol-1].p_ପଦସମଷ୍ଟି;
          ପଦସମଷ୍ଟି *pBase = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(pX);
          ଯେତେବେଳେ( ALWAYS(pBase!=0) && pBase->op==TK_IF_NULL_ROW ){
            pX = pBase->p_ବାମ;
            pBase = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(pX);
          }
          ଯଦି( ALWAYS(pBase!=0)
           && pBase->op!=TK_AGG_COLUMN
           && pBase->op!=TK_REGISTER
          ){
            sqlite3_ପଦସମଷ୍ଟି_To_Register(pX, iAMem+j);
          }
        }
      }
      sqlite3VdbeAddOp4(v, OP_Compare, iAMem, iBMem, pGroupBy->n_ପଦସମଷ୍ଟି,
                          (ଅକ୍ଷର*)sqlite3_ଚାବି_Info_Ref(p_ଚାବି_Info), P4_KEYINFO);
      addr1 = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp3(v, OP_Jump, addr1+1, 0, addr1+1); VdbeCoverage(v);

      /* Generate code that runs whenever the GROUP BY changes.
      ** Changes in the GROUP BY are detected by the previous code
      ** block.  If there were no changes, this block is skipped.
      **
      ** This code copies current group by terms in b0,b1,b2,...
      ** over to a0,a1,a2.  It then calls the output subroutine
      ** and resets the aggregate accumulator registers in preparation
      ** for the next GROUP BY batch.
      */
      sqlite3VdbeAddOp2(v, OP_Gosub, regOutputRow, addrOutputRow);
      VdbeComment((v, "output one row of %d", p->selId));
      sqlite3_ପଦସମଷ୍ଟି_Code_Move(pParse, iBMem, iAMem, pGroupBy->n_ପଦସମଷ୍ଟି);
      sqlite3VdbeAddOp2(v, OP_IfPos, iAbortFlag, addrEnd); VdbeCoverage(v);
      VdbeComment((v, "check abort flag"));
      sqlite3VdbeAddOp2(v, OP_Gosub, regReset, addrReset);
      VdbeComment((v, "reset accumulator %d", p->selId));

      /* Update the aggregate accumulators based on the content of
      ** the current row
      */
      sqlite3VdbeJumpHere(v, addr1);
      updateAccumulator(pParse, iUseFlag, pAggInfo, eDist);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, iUseFlag);
      VdbeComment((v, "indicate data in accumulator %d", p->selId));

      /* End of the loop
      */
      ଯଦି( groupBySort ){
        sqlite3VdbeAddOp2(v, OP_SorterNext, pAggInfo->sortingIdx,addr_ଶୀର୍ଷ_Of_Loop);
        VdbeCoverage(v);
      }ଅନ୍ୟଥା{
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3WhereEnd(pWInfo);
        sqlite3VdbeChangeToNoop(v, addrSortingIdx);
      }
      sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pDistinct);

      /* Output the final row of result
      */
      sqlite3VdbeAddOp2(v, OP_Gosub, regOutputRow, addrOutputRow);
      VdbeComment((v, "output final row of %d", p->selId));

      /* Jump over the subroutines
      */
      sqlite3VdbeGoto(v, addrEnd);

      /* Generate a subroutine that outputs a single row of the result
      ** set.  This subroutine first looks at the iUseFlag.  If iUseFlag
      ** is less than or equal to zero, the subroutine is a no-op.  If
      ** the processing calls for the query to abort, this subroutine
      ** increments the iAbortFlag memory location before returning in
      ** order to signal the caller to abort.
      */
      addrSetAbort = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp2(v, OP_Integer, 1, iAbortFlag);
      VdbeComment((v, "set abort flag"));
      sqlite3VdbeAddOp1(v, OP_Return, regOutputRow);
      sqlite3VdbeResolveLabel(v, addrOutputRow);
      addrOutputRow = sqlite3VdbeCurrentAddr(v);
      sqlite3VdbeAddOp2(v, OP_IfPos, iUseFlag, addrOutputRow+2);
      VdbeCoverage(v);
      VdbeComment((v, "Groupby result generator entry point %d", p->selId));
      sqlite3VdbeAddOp1(v, OP_Return, regOutputRow);
      finalizeAggFunctions(pParse, pAggInfo);
      sqlite3_ପଦସମଷ୍ଟି_If_False(pParse, pHaving, addrOutputRow+1, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, &sSort,
                      &sDistinct, pDest,
                      addrOutputRow+1, addrSetAbort);
      sqlite3VdbeAddOp1(v, OP_Return, regOutputRow);
      VdbeComment((v, "end groupby result generator %d", p->selId));

      /* Generate a subroutine that will reset the group-by accumulator
      */
      sqlite3VdbeResolveLabel(v, addrReset);
      resetAccumulator(pParse, pAggInfo);
      sqlite3VdbeAddOp2(v, OP_Integer, 0, iUseFlag);
      VdbeComment((v, "indicate accumulator %d empty", p->selId));
      sqlite3VdbeAddOp1(v, OP_Return, regReset);

      ଯଦି( distFlag!=0 && eDist!=WHERE_DISTINCT_NOOP ){
        ସଂରଚନା AggInfo_func *pF = &pAggInfo->aFunc[0];
        fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
      }
    } /* endif pGroupBy.  Begin aggregate queries without GROUP BY: */
    ଅନ୍ୟଥା {
      /* Aggregate functions without GROUP BY. tag-select-0820 */
      Table *pTab;
      ଯଦି( (pTab = is_Simple_ଗଣନା(p, pAggInfo))!=0 ){
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
        ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
        ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCsr = pParse->nTab++;     /* Cursor to scan b-tree */
        Index *pIdx;                         /* Iterator variable */
        ଚାବି_Info *p_ଚାବି_Info = 0;               /* Keyinfo for scanned index */
        Index *pBest = 0;                    /* Best index found so far */
        Pgno i_ମୂଳ = pTab->tnum;             /* Root page of scanned b-tree */

        sqlite3CodeVerifySchema(pParse, iDb);
        sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);

        /* Search for the index that has the lowest scan cost.
        **
        ** (2011-04-15) Do not do a full scan of an unordered index.
        **
        ** (2013-10-03) Do not count the entries in a partial index.
        **
        ** In practice the KeyInfo structure will not be used. It is only
        ** passed to keep OP_OpenRead happy.
        */
        ଯଦି( !HasRowid(pTab) ) pBest = sqlite3_Primary_ଚାବି_Index(pTab);
        ଯଦି( !p->pSrc->a[0].fg.notIndexed ){
          ପାଇଁ(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
            ଯଦି( pIdx->bUnordered==0
             && pIdx->szIdxRow<pTab->szTabRow
             && pIdx->pPartIdxWhere==0
             && (!pBest || pIdx->szIdxRow<pBest->szIdxRow)
            ){
              pBest = pIdx;
            }
          }
        }
        ଯଦି( pBest ){
          i_ମୂଳ = pBest->tnum;
          p_ଚାବି_Info = sqlite3_ଚାବି_Info_Of_Index(pParse, pBest);
        }

        /* Open a read-only cursor, execute the OP_Count, close the cursor. */
        sqlite3VdbeAddOp4Int(v, OP_OpenRead, iCsr, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)i_ମୂଳ, iDb, 1);
        ଯଦି( p_ଚାବି_Info ){
          sqlite3VdbeChangeP4(v, -1, (ଅକ୍ଷର *)p_ଚାବି_Info, P4_KEYINFO);
        }
        assignAggregateRegisters(pParse, pAggInfo);
        sqlite3VdbeAddOp2(v, OP_ଗଣନା, iCsr, AggInfoFuncReg(pAggInfo,0));
        sqlite3VdbeAddOp1(v, OP_Close, iCsr);
        explain_Simple_ଗଣନା(pParse, pTab, pBest);
      }ଅନ୍ୟଥା{
        /* The general case of an aggregate query without GROUP BY
        ** tag-select-0822 */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା regAcc = 0;           /* "populate accumulators" flag */
        ପଦସମଷ୍ଟି_List *pDistinct = 0;
        u16 distFlag = 0;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା eDist;

        /* If there are accumulator registers but no min() or max() functions
        ** without FILTER clauses, allocate register regAcc. Register regAcc
        ** will contain 0 the first time the inner loop runs, and 1 thereafter.
        ** The code generated by updateAccumulator() uses this to ensure
        ** that the accumulator registers are (a) updated only once if
        ** there are no min() or max functions or (b) always updated for the
        ** first row visited by the aggregate, so that they are updated at
        ** least once even if the FILTER clause means the min() or max()
        ** function visits zero rows.  */
        ଯଦି( pAggInfo->nAccumulator ){
          ପାଇଁ(i=0; i<pAggInfo->nFunc; i++){
            ଯଦି( ପଦସମଷ୍ଟି_Has_Property(pAggInfo->aFunc[i].pFExpr, EP_WinFunc) ){
              ଜାରି;
            }
            ଯଦି( pAggInfo->aFunc[i].pFunc->funcFlags&SQLITE_FUNC_NEEDCOLL ){
              ଭାଙ୍ଗ;
            }
          }
          ଯଦି( i==pAggInfo->nFunc ){
            regAcc = ++pParse->nMem;
            sqlite3VdbeAddOp2(v, OP_Integer, 0, regAcc);
          }
        }ଅନ୍ୟଥା ଯଦି( pAggInfo->nFunc==1 && pAggInfo->aFunc[0].iDistinct>=0 ){
          ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pAggInfo->aFunc[0].pFExpr) );
          pDistinct = pAggInfo->aFunc[0].pFExpr->x.pList;
          distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
        }
        assignAggregateRegisters(pParse, pAggInfo);

        /* This case runs if the aggregate has no GROUP BY clause.  The
        ** processing is much simpler since there is only a single row
        ** of output.
        */
        ପ୍ରତିଜ୍ଞା( p->pGroupBy==0 );
        resetAccumulator(pParse, pAggInfo);

        /* If this query is a candidate for the min/max optimization, then
        ** minMaxFlag will have been previously set to either
        ** WHERE_ORDERBY_MIN or WHERE_ORDERBY_MAX and pMinMaxOrderBy will
        ** be an appropriate ORDER BY expression for the optimization.
        */
        ପ୍ରତିଜ୍ଞା( minMaxFlag==WHERE_ORDERBY_NORMAL || pMinMaxOrderBy!=0 );
        ପ୍ରତିଜ୍ଞା( pMinMaxOrderBy==0 || pMinMaxOrderBy->n_ପଦସମଷ୍ଟି==1 );

        TREETRACE(0x2,pParse,p,("WhereBegin\n"));
        pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, pMinMaxOrderBy,
                                   pDistinct, p, minMaxFlag|distFlag, 0);
        ଯଦି( pWInfo==0 ){
          ଯାଅ select_end;
        }
        TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
        eDist = sqlite3WhereIsDistinct(pWInfo);
        updateAccumulator(pParse, regAcc, pAggInfo, eDist);
        ଯଦି( eDist!=WHERE_DISTINCT_NOOP ){
          ସଂରଚନା AggInfo_func *pF = pAggInfo->aFunc;
          ଯଦି( pF ){
            fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
          }
        }

        ଯଦି( regAcc ) sqlite3VdbeAddOp2(v, OP_Integer, 1, regAcc);
        ଯଦି( minMaxFlag ){
          sqlite3WhereMinMaxOptEarlyOut(v, pWInfo);
        }
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3WhereEnd(pWInfo);
        finalizeAggFunctions(pParse, pAggInfo);
      }

      sSort.pOrderBy = 0;
      sqlite3_ପଦସମଷ୍ଟି_If_False(pParse, pHaving, addrEnd, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, 0, 0,
                      pDest, addrEnd, addrEnd);
    }
    sqlite3VdbeResolveLabel(v, addrEnd);

  } /* endif aggregate query */

  ଯଦି( sDistinct.eTnctType==WHERE_DISTINCT_UNORDERED ){
    explainTempTable(pParse, "DISTINCT");
  }

  /* If there is an ORDER BY clause, then we need to sort the results
  ** and send them to the callback one by one.  tag-select-0900
  */
  ଯଦି( sSort.pOrderBy ){
    ପ୍ରତିଜ୍ଞା( p->pEList==pEList );
    generateSortTail(pParse, p, &sSort, pEList->n_ପଦସମଷ୍ଟି, pDest);
  }

  /* Jump here to skip this query
  */
  sqlite3VdbeResolveLabel(v, iEnd);

  /* The SELECT has been coded. If there is an error in the Parse structure,
  ** set the return code to 1. Otherwise 0. */
  rc = (pParse->nErr>0);

  /* Control jumps to here if an error is encountered above, or upon
  ** successful coding of the SELECT.
  */
select_end:
  ପ୍ରତିଜ୍ଞା( db->mallocFailed==0 || db->mallocFailed==1 );
  ପ୍ରତିଜ୍ଞା( db->mallocFailed==0 || pParse->nErr!=0 );
  sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pMinMaxOrderBy);
#ifdef SQLITE_DEBUG
  /* Internal self-checks.  tag-select-1000 */
  ଯଦି( pAggInfo && !db->mallocFailed ){
#ଯଦି TREETRACE_ENABLED
    ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("Finished with AggInfo\n"));
      printAggInfo(pAggInfo);
    }
#endif
    ପାଇଁ(i=0; i<pAggInfo->nColumn; i++){
      ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି = pAggInfo->aCol[i].pCExpr;
      ଯଦି( p_ପଦସମଷ୍ଟି==0 ) ଜାରି;
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->pAggInfo==pAggInfo );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->iAgg==i );
    }
    ପାଇଁ(i=0; i<pAggInfo->nFunc; i++){
      ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି = pAggInfo->aFunc[i].pFExpr;
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି!=0 );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->pAggInfo==pAggInfo );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->iAgg==i );
    }
  }
#endif

#ଯଦି TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p,("end processing\n"));
  ଯଦି( (sqlite3_ବୃକ୍ଷ_Trace & 0x40000)!=0 && ExplainQueryPlanParent(pParse)==0 ){
    sqlite3_ବୃକ୍ଷ_View_Select(0, p, 0);
  }
#endif
  Explain_Query_Plan_କାଢ଼(pParse);
  ଫେରନ୍ତୁ rc;
}
