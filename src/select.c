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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** An instance of the following object is used to record information about
** how to process the DISTINCT keyword, to simplify passing that information
** into the selectInnerLoop() routine.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ DistinctCtx DistinctCtx;
ಸಂರಚನಾ DistinctCtx {
  u8 isTnct;      /* 0: Not distinct. 1: DISTINCT  2: DISTINCT and ORDER BY */
  u8 eTnctType;   /* One of the WHERE_DISTINCT_* operators */
  ಪೂರ್ಣಾಙ್ಕ tabTnct;    /* Ephemeral table used for DISTINCT processing */
  ಪೂರ್ಣಾಙ್ಕ addrTnct;   /* Address of OP_OpenEphemeral opcode for tabTnct */
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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ SortCtx SortCtx;
ಸಂರಚನಾ SortCtx {
  ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy;   /* The ORDER BY (or GROUP BY clause) */
  ಪೂರ್ಣಾಙ್ಕ nOBSat;           /* Number of ORDER BY terms satisfied by indices */
  ಪೂರ್ಣಾಙ್ಕ iECursor;         /* Cursor number for the sorter */
  ಪೂರ್ಣಾಙ್ಕ regReturn;        /* Register holding block-output return address */
  ಪೂರ್ಣಾಙ್ಕ labelBkOut;       /* Start label for the block-output subroutine */
  ಪೂರ್ಣಾಙ್ಕ addrSortIndex;    /* Address of the OP_SorterOpen or OP_OpenEphemeral */
  ಪೂರ್ಣಾಙ್ಕ labelDone;        /* Jump here when done, ex: LIMIT reached */
  ಪೂರ್ಣಾಙ್ಕ labelOBLopt;      /* Jump here when sorter is full */
  u8 sortFlags;         /* Zero or more SORTFLAG_* bits */
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  u8 nDefer;            /* Number of valid entries in aDefer[] */
  ಸಂರಚನಾ DeferredCsr {
    Table *pTab;        /* Table definition */
    ಪೂರ್ಣಾಙ್ಕ iCsr;           /* Cursor number for table */
    ಪೂರ್ಣಾಙ್ಕ n_ಕೀಲಿ;           /* Number of PK columns for table pTab (>=1) */
  } aDefer[4];
#endif
  ಸಂರಚನಾ RowLoadInfo *pDeferredRowLoad;  /* Deferred row loading info or NULL */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  ಪೂರ್ಣಾಙ್ಕ addr_ತಳ್ಳು;         /* First instruction to push data into sorter */
  ಪೂರ್ಣಾಙ್ಕ addr_ತಳ್ಳು_End;      /* Last instruction that pushes data into sorter */
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
ಸ್ಥಿರ ಶೂನ್ಯ clearSelect(sqlite3 *db, Select *p, ಪೂರ್ಣಾಙ್ಕ bFree){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಯಾವತ್( p ){
    Select *pPrior = p->pPrior;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p->pEList);
    sqlite3SrcListDelete(db, p->pSrc);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p->pWhere);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p->pGroupBy);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p->pHaving);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p->pOrderBy);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p->pLimit);
    ಯದಿ( OK_IF_ALWAYS_TRUE(p->pWith) ) sqlite3WithDelete(db, p->pWith);
#ifndef SQLITE_OMIT_WINDOWFUNC
    ಯದಿ( OK_IF_ALWAYS_TRUE(p->pWinDefn) ){
      sqlite3WindowListDelete(db, p->pWinDefn);
    }
    ಯಾವತ್( p->pWin ){
      ಪ್ರತಿಜ್ಞಾ( p->pWin->ppThis==&p->pWin );
      sqlite3WindowUnlinkFromSelect(p->pWin);
    }
#endif
    ಯದಿ( bFree ) sqlite3DbNNFreeNN(db, p);
    p = pPrior;
    bFree = 1;
  }
}

/*
** Initialize a SelectDest structure.
*/
ಶೂನ್ಯ sqlite3SelectDestInit(SelectDest *pDest, ಪೂರ್ಣಾಙ್ಕ eDest, ಪೂರ್ಣಾಙ್ಕ iParm){
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
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,     /* which columns to include in the result */
  SrcList *pSrc,        /* the FROM clause -- which tables to scan */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,         /* the WHERE clause */
  ಅಭಿವ್ಯಕ್ತಿ_List *pGroupBy,   /* the GROUP BY clause */
  ಅಭಿವ್ಯಕ್ತಿ *pHaving,        /* the HAVING clause */
  ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy,   /* the ORDER BY clause */
  u32 selFlags,         /* Flag parameters, such as SF_Distinct */
  ಅಭಿವ್ಯಕ್ತಿ *pLimit          /* LIMIT value.  NULL means not used */
){
  Select *pNew, *pAllocated;
  Select standin;
  pAllocated = pNew = sqlite3DbMallocRawNN(pParse->db, ಪರಿಮಾಣ(*pNew) );
  ಯದಿ( pNew==0 ){
    ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed );
    pNew = &standin;
  }
  ಯದಿ( pEList==0 ){
    pEList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, 0,
                                   sqlite3_ಅಭಿವ್ಯಕ್ತಿ(pParse->db,TK_ASTERISK,0));
  }
  pNew->pEList = pEList;
  pNew->op = TK_SELECT;
  pNew->selFlags = selFlags;
  pNew->iLimit = 0;
  pNew->iOffset = 0;
  pNew->selId = ++pParse->nSelect;
  pNew->nSelectRow = 0;
  ಯದಿ( pSrc==0 ) pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
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
  ಯದಿ( pParse->db->mallocFailed ) {
    clearSelect(pParse->db, pNew, pNew!=&standin);
    pAllocated = 0;
  }ಅನ್ಯಥಾ{
    ಪ್ರತಿಜ್ಞಾ( pNew->pSrc!=0 || pParse->nErr>0 );
  }
  ಪ್ರತ್ಯಾಗಮ pAllocated;
}


/*
** Delete the given Select structure and all of its substructures.
*/
ಶೂನ್ಯ sqlite3SelectDelete(sqlite3 *db, Select *p){
  ಯದಿ( OK_IF_ALWAYS_TRUE(p) ) clearSelect(db, p, 1);
}
ಶೂನ್ಯ sqlite3SelectDeleteGeneric(sqlite3 *db, ಶೂನ್ಯ *p){
  ಯದಿ( ALWAYS(p) ) clearSelect(db, (Select*)p, 1);
}

/*
** Return a pointer to the right-most SELECT statement in a compound.
*/
ಸ್ಥಿರ Select *findRightmost(Select *p){
  ಯಾವತ್( p->pNext ) p = p->pNext;
  ಪ್ರತ್ಯಾಗಮ p;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3JoinType(Parse *pParse, Token *pA, Token *pB, Token *pC){
  ಪೂರ್ಣಾಙ್ಕ jointype = 0;
  Token *apAll[3];
  Token *p;
                             /*   0123456789 123456789 123456789 123 */
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ z_ಕೀಲಿ_Text[] = "naturaleftouterightfullinnercross";
  ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ {
    u8 i;        /* Beginning of keyword text in zKeyText[] */
    u8 nChar;    /* Length of the keyword in characters */
    u8 code;     /* Join type mask */
  } aKeyword[] = {
    /* (0) natural */ { 0,  7, JT_NATURAL                },
    /* (1) left    */ { 6,  4, JT_ಎಡ|JT_OUTER          },
    /* (2) outer   */ { 10, 5, JT_OUTER                  },
    /* (3) right   */ { 14, 5, JT_ಬಲ|JT_OUTER         },
    /* (4) full    */ { 19, 4, JT_ಎಡ|JT_ಬಲ|JT_OUTER },
    /* (5) inner   */ { 23, 5, JT_INNER                  },
    /* (6) cross   */ { 28, 5, JT_INNER|JT_CROSS         },
  };
  ಪೂರ್ಣಾಙ್ಕ i, j;
  apAll[0] = pA;
  apAll[1] = pB;
  apAll[2] = pC;
  ಪ್ರತಿ(i=0; i<3 && apAll[i]; i++){
    p = apAll[i];
    ಪ್ರತಿ(j=0; j<ArraySize(aKeyword); j++){
      ಯದಿ( p->n==aKeyword[j].nChar
          && sqlite3StrNICmp((ವರ್ಣ*)p->z, &z_ಕೀಲಿ_Text[aKeyword[j].i], p->n)==0 ){
        jointype |= aKeyword[j].code;
        ಭಙ್ಗ;
      }
    }
    testcase( j==0 || j==1 || j==2 || j==3 || j==4 || j==5 || j==6 );
    ಯದಿ( j>=ArraySize(aKeyword) ){
      jointype |= JT_ERROR;
      ಭಙ್ಗ;
    }
  }
  ಯದಿ(
     (jointype & (JT_INNER|JT_OUTER))==(JT_INNER|JT_OUTER) ||
     (jointype & JT_ERROR)!=0 ||
     (jointype & (JT_OUTER|JT_ಎಡ|JT_ಬಲ))==JT_OUTER
  ){
    ಧ್ರುವ ವರ್ಣ *zSp1 = " ";
    ಧ್ರುವ ವರ್ಣ *zSp2 = " ";
    ಯದಿ( pB==0 ){ zSp1++; }
    ಯದಿ( pC==0 ){ zSp2++; }
    sqlite3_Error_ಸಂದೇಶ(pParse, "unknown join type: "
       "%T%s%T%s%T", pA, zSp1, pB, zSp2, pC);
    jointype = JT_INNER;
  }
  ಪ್ರತ್ಯಾಗಮ jointype;
}

/*
** Return the index of a column in a table.  Return -1 if the column
** is not contained in the table.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3ColumnIndex(Table *pTab, ಧ್ರುವ ವರ್ಣ *zCol){
  ಪೂರ್ಣಾಙ್ಕ i;
  u8 h;
  ಧ್ರುವ Column *aCol;
  ಪೂರ್ಣಾಙ್ಕ nCol;

  h = sqlite3StrIHash(zCol);
  aCol = pTab->aCol;
  nCol = pTab->nCol;

  /* See if the aHx gives us a lucky match */
  i = pTab->aHx[h % ಪರಿಮಾಣ(pTab->aHx)];
  ಪ್ರತಿಜ್ಞಾ( i<nCol );
  ಯದಿ( aCol[i].hName==h
   && sqlite3StrICmp(aCol[i].zCnName, zCol)==0
  ){
    ಪ್ರತ್ಯಾಗಮ i;
  }

  /* No lucky match from the hash table.  Do a full search. */
  i = 0;
  ಯಾವತ್( 1 /*exit-by-break*/ ){
    ಯದಿ( aCol[i].hName==h
     && sqlite3StrICmp(aCol[i].zCnName, zCol)==0
    ){
      ಪ್ರತ್ಯಾಗಮ i;
    }
    i++;
    ಯದಿ( i>=nCol ) ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ -1;
}

/*
** Mark a subquery result column as having been used.
*/
ಶೂನ್ಯ sqlite3SrcItemColumnUsed(SrcItem *pItem, ಪೂರ್ಣಾಙ್ಕ iCol){
  ಪ್ರತಿಜ್ಞಾ( pItem!=0 );
  ಪ್ರತಿಜ್ಞಾ( (ಪೂರ್ಣಾಙ್ಕ)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
  ಯದಿ( pItem->fg.isNestedFrom ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pResults;
    ಪ್ರತಿಜ್ಞಾ( pItem->fg.isSubquery );
    ಪ್ರತಿಜ್ಞಾ( pItem->u4.pSubq!=0 );
    ಪ್ರತಿಜ್ಞಾ( pItem->u4.pSubq->pSelect!=0 );
    pResults = pItem->u4.pSubq->pSelect->pEList;
    ಪ್ರತಿಜ್ಞಾ( pResults!=0 );
    ಪ್ರತಿಜ್ಞಾ( iCol>=0 && iCol<pResults->n_ಅಭಿವ್ಯಕ್ತಿ );
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tableAndColumnIndex(
  SrcList *pSrc,       /* Array of tables to search */
  ಪೂರ್ಣಾಙ್ಕ iStart,          /* First member of pSrc->a[] to check */
  ಪೂರ್ಣಾಙ್ಕ iEnd,            /* Last member of pSrc->a[] to check */
  ಧ್ರುವ ವರ್ಣ *zCol,    /* Name of the column we are looking for */
  ಪೂರ್ಣಾಙ್ಕ *piTab,          /* Write index of pSrc->a[] here */
  ಪೂರ್ಣಾಙ್ಕ *piCol,          /* Write index of pSrc->a[*piTab].pSTab->aCol[] here */
  ಪೂರ್ಣಾಙ್ಕ bIgnoreHidden    /* Ignore hidden columns */
){
  ಪೂರ್ಣಾಙ್ಕ i;               /* For looping over tables in pSrc */
  ಪೂರ್ಣಾಙ್ಕ iCol;            /* Index of column matching zCol */

  ಪ್ರತಿಜ್ಞಾ( iEnd<pSrc->nSrc );
  ಪ್ರತಿಜ್ಞಾ( iStart>=0 );
  ಪ್ರತಿಜ್ಞಾ( (piTab==0)==(piCol==0) );  /* Both or neither are NULL */

  ಪ್ರತಿ(i=iStart; i<=iEnd; i++){
    iCol = sqlite3ColumnIndex(pSrc->a[i].pSTab, zCol);
    ಯದಿ( iCol>=0
     && (bIgnoreHidden==0 || IsHiddenColumn(&pSrc->a[i].pSTab->aCol[iCol])==0)
    ){
      ಯದಿ( piTab ){
        sqlite3SrcItemColumnUsed(&pSrc->a[i], iCol);
        *piTab = i;
        *piCol = iCol;
      }
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಶೂನ್ಯ sqlite3_Set_Join_ಅಭಿವ್ಯಕ್ತಿ(ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಙ್ಕ iTable, u32 joinFlag){
  ಪ್ರತಿಜ್ಞಾ( joinFlag==EP_OuterON || joinFlag==EP_InnerON );
  ಯಾವತ್( p ){
    ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p, joinFlag);
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    ಅಭಿವ್ಯಕ್ತಿ_Set_VVAProperty(p, EP_NoReduce);
    p->w.iJoin = iTable;
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) ){
      ಯದಿ( p->x.pList ){
        ಪೂರ್ಣಾಙ್ಕ i;
        ಪ್ರತಿ(i=0; i<p->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
          sqlite3_Set_Join_ಅಭಿವ್ಯಕ್ತಿ(p->x.pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, iTable, joinFlag);
        }
      }
    }
    sqlite3_Set_Join_ಅಭಿವ್ಯಕ್ತಿ(p->p_ಎಡ, iTable, joinFlag);
    p = p->p_ಬಲ;
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
ಸ್ಥಿರ ಶೂನ್ಯ unset_Join_ಅಭಿವ್ಯಕ್ತಿ(ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಙ್ಕ iTable, ಪೂರ್ಣಾಙ್ಕ nullable){
  ಯಾವತ್( p ){
    ಯದಿ( iTable<0 || (ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_OuterON) && p->w.iJoin==iTable) ){
      ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p, EP_OuterON|EP_InnerON);
      ಯದಿ( iTable>=0 ) ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p, EP_InnerON);
    }
    ಯದಿ( p->op==TK_COLUMN && p->iTable==iTable && !nullable ){
      ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p, EP_CanBeNull);
    }
    ಯದಿ( p->op==TK_FUNCTION ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) );
      ಪ್ರತಿಜ್ಞಾ( p->p_ಎಡ==0 );
      ಯದಿ( p->x.pList ){
        ಪೂರ್ಣಾಙ್ಕ i;
        ಪ್ರತಿ(i=0; i<p->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
          unset_Join_ಅಭಿವ್ಯಕ್ತಿ(p->x.pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, iTable, nullable);
        }
      }
    }
    unset_Join_ಅಭಿವ್ಯಕ್ತಿ(p->p_ಎಡ, iTable, nullable);
    p = p->p_ಬಲ;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3ProcessJoin(Parse *pParse, Select *p){
  SrcList *pSrc;                  /* All tables in the FROM clause */
  ಪೂರ್ಣಾಙ್ಕ i, j;                       /* Loop counters */
  SrcItem *p_ಎಡ;                 /* Left table being joined */
  SrcItem *p_ಬಲ;                /* Right table being joined */
  ಪೂರ್ಣಾಙ್ಕ bSeenTFunc = 0;             /* True if have seen table-valued function */

  pSrc = p->pSrc;
  p_ಎಡ = &pSrc->a[0];
  p_ಬಲ = &p_ಎಡ[1];
  ಪ್ರತಿ(i=0; i<pSrc->nSrc-1; i++, p_ಬಲ++, p_ಎಡ++){
    Table *p_ಬಲ_Tab = p_ಬಲ->pSTab;
    u32 joinType;

    ಯದಿ( NEVER(p_ಎಡ->pSTab==0 || p_ಬಲ_Tab==0) ) ಅನುವರ್ತ;
    joinType = (p_ಬಲ->fg.jointype & JT_OUTER)!=0 ? EP_OuterON : EP_InnerON;

    /* If this is a NATURAL join, synthesize an appropriate USING clause
    ** to specify which columns should be joined.
    */
    ಯದಿ( p_ಬಲ->fg.jointype & JT_NATURAL ){
      IdList *pUsing = 0;
      ಯದಿ( p_ಬಲ->fg.isUsing || p_ಬಲ->u3.pOn ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "a NATURAL join may not have "
           "an ON or USING clause", 0);
        ಪ್ರತ್ಯಾಗಮ 1;
      }
      ಪ್ರತಿ(j=0; j<p_ಬಲ_Tab->nCol; j++){
        ವರ್ಣ *zName;   /* Name of column in the right table */

        ಯದಿ( IsHiddenColumn(&p_ಬಲ_Tab->aCol[j]) ) ಅನುವರ್ತ;
        zName = p_ಬಲ_Tab->aCol[j].zCnName;
        ಯದಿ( tableAndColumnIndex(pSrc, 0, i, zName, 0, 0, 1) ){
          pUsing = sqlite3IdListAppend(pParse, pUsing, 0);
          ಯದಿ( pUsing ){
            ಪ್ರತಿಜ್ಞಾ( pUsing->nId>0 );
            ಪ್ರತಿಜ್ಞಾ( pUsing->a[pUsing->nId-1].zName==0 );
            pUsing->a[pUsing->nId-1].zName = sqlite3DbStrDup(pParse->db, zName);
          }
        }
      }
      ಯದಿ( pUsing ){
        p_ಬಲ->fg.isUsing = 1;
        p_ಬಲ->fg.isSynthUsing = 1;
        p_ಬಲ->u3.pUsing = pUsing;
      }
      ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ 1;
    }

    /* Create extra terms on the WHERE clause for each column named
    ** in the USING clause.  Example: If the two tables to be joined are
    ** A and B and the USING clause names X, Y, and Z, then add this
    ** to the WHERE clause:    A.X=B.X AND A.Y=B.Y AND A.Z=B.Z
    ** Report an error if any column mentioned in the USING clause is
    ** not contained in both tables to be joined.
    */
    ಯದಿ( p_ಬಲ->fg.isUsing ){
      IdList *pList = p_ಬಲ->u3.pUsing;
      sqlite3 *db = pParse->db;
      ಪ್ರತಿಜ್ಞಾ( pList!=0 );
      ಪ್ರತಿ(j=0; j<pList->nId; j++){
        ವರ್ಣ *zName;     /* Name of the term in the USING clause */
        ಪೂರ್ಣಾಙ್ಕ i_ಎಡ;       /* Table on the left with matching column name */
        ಪೂರ್ಣಾಙ್ಕ i_ಎಡ_Col;    /* Column number of matching column on the left */
        ಪೂರ್ಣಾಙ್ಕ i_ಬಲ_Col;   /* Column number of matching column on the right */
        ಅಭಿವ್ಯಕ್ತಿ *pE1;       /* Reference to the column on the LEFT of the join */
        ಅಭಿವ್ಯಕ್ತಿ *pE2;       /* Reference to the column on the RIGHT of the join */
        ಅಭಿವ್ಯಕ್ತಿ *pEq;       /* Equality constraint.  pE1 == pE2 */

        zName = pList->a[j].zName;
        i_ಬಲ_Col = sqlite3ColumnIndex(p_ಬಲ_Tab, zName);
        ಯದಿ( i_ಬಲ_Col<0
         || tableAndColumnIndex(pSrc, 0, i, zName, &i_ಎಡ, &i_ಎಡ_Col,
                                p_ಬಲ->fg.isSynthUsing)==0
        ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "cannot join using column %s - column "
            "not present in both tables", zName);
          ಪ್ರತ್ಯಾಗಮ 1;
        }
        pE1 = sqlite3_Create_Column_ಅಭಿವ್ಯಕ್ತಿ(db, pSrc, i_ಎಡ, i_ಎಡ_Col);
        sqlite3SrcItemColumnUsed(&pSrc->a[i_ಎಡ], i_ಎಡ_Col);
        ಯದಿ( (pSrc->a[0].fg.jointype & JT_LTORJ)!=0 && pParse->nErr==0 ){
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
          ಅಭಿವ್ಯಕ್ತಿ_List *pFuncArgs = 0;   /* Arguments to the coalesce() */
          ಸ್ಥಿರ ಧ್ರುವ Token tkCoalesce = { "coalesce", 8 };
          ಪ್ರತಿಜ್ಞಾ( pE1!=0 );
          ಅಭಿವ್ಯಕ್ತಿ_Set_Property(pE1, EP_CanBeNull);
          ಯಾವತ್( tableAndColumnIndex(pSrc, i_ಎಡ+1, i, zName, &i_ಎಡ, &i_ಎಡ_Col,
                                     p_ಬಲ->fg.isSynthUsing)!=0 ){
            ಯದಿ( pSrc->a[i_ಎಡ].fg.isUsing==0
             || sqlite3IdListIndex(pSrc->a[i_ಎಡ].u3.pUsing, zName)<0
            ){
              sqlite3_Error_ಸಂದೇಶ(pParse, "ambiguous reference to %s in USING()",
                              zName);
              ಭಙ್ಗ;
            }
            pFuncArgs = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pFuncArgs, pE1);
            pE1 = sqlite3_Create_Column_ಅಭಿವ್ಯಕ್ತಿ(db, pSrc, i_ಎಡ, i_ಎಡ_Col);
            sqlite3SrcItemColumnUsed(&pSrc->a[i_ಎಡ], i_ಎಡ_Col);
          }
          ಯದಿ( pFuncArgs ){
            pFuncArgs = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pFuncArgs, pE1);
            pE1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Function(pParse, pFuncArgs, &tkCoalesce, 0);
            ಯದಿ( pE1 ){
              pE1->aff_ಅಭಿವ್ಯಕ್ತಿ = SQLITE_AFF_DEFER;
            }
          }
        }ಅನ್ಯಥಾ ಯದಿ( (pSrc->a[i+1].fg.jointype & JT_ಎಡ)!=0 && pParse->nErr==0 ){
          ಪ್ರತಿಜ್ಞಾ( pE1!=0 );
          ಅಭಿವ್ಯಕ್ತಿ_Set_Property(pE1, EP_CanBeNull);
        }
        pE2 = sqlite3_Create_Column_ಅಭಿವ್ಯಕ್ತಿ(db, pSrc, i+1, i_ಬಲ_Col);
        sqlite3SrcItemColumnUsed(p_ಬಲ, i_ಬಲ_Col);
        pEq = sqlite3PExpr(pParse, TK_EQ, pE1, pE2);
        ಪ್ರತಿಜ್ಞಾ( pE2!=0 || pEq==0 );
        ಯದಿ( pEq ){
          ಅಭಿವ್ಯಕ್ತಿ_Set_Property(pEq, joinType);
          ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pEq, EP_TokenOnly|EP_Reduced) );
          ಅಭಿವ್ಯಕ್ತಿ_Set_VVAProperty(pEq, EP_NoReduce);
          pEq->w.iJoin = pE2->iTable;
        }
        p->pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, p->pWhere, pEq);
      }
    }

    /* Add the ON clause to the end of the WHERE clause, connected by
    ** an AND operator.
    */
    ಅನ್ಯಥಾ ಯದಿ( p_ಬಲ->u3.pOn ){
      sqlite3_Set_Join_ಅಭಿವ್ಯಕ್ತಿ(p_ಬಲ->u3.pOn, p_ಬಲ->iCursor, joinType);
      p->pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, p->pWhere, p_ಬಲ->u3.pOn);
      p_ಬಲ->u3.pOn = 0;
      p_ಬಲ->fg.isOn = 1;
      p->selFlags |= SF_OnToWhere;
    }

    ಯದಿ( bSeenTFunc==0 ){
      bSeenTFunc = (p_ಎಡ->fg.isTabFunc && p_ಎಡ->u1.pFuncArg);
    }

    /* We also need to call sqlite3SelectCheckOnClauses() to verify that
    ** table-valued function arguments do not illegally refer to any tables to
    ** their right. This test is required if either (a) there is a RIGHT JOIN
    ** in the SrcList, or (b) there is a LEFT JOIN to the right of a 
    ** table-valued function.  */
    ಯದಿ( (p_ಬಲ->fg.isTabFunc && joinType==EP_OuterON && p_ಬಲ->u1.pFuncArg)
     || (bSeenTFunc && joinType==EP_OuterON)
    ){
      p->selFlags |= SF_OnToWhere;
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** An instance of this object holds information (beyond pParse and pSelect)
** needed to load the next result row that is to be added to the sorter.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ RowLoadInfo RowLoadInfo;
ಸಂರಚನಾ RowLoadInfo {
  ಪೂರ್ಣಾಙ್ಕ regResult;               /* Store results in array of registers here */
  u8 ecelFlags;                /* Flag argument to ExprCodeExprList() */
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  ಅಭಿವ್ಯಕ್ತಿ_List *pExtra;            /* Extra columns needed by sorter refs */
  ಪೂರ್ಣಾಙ್ಕ regExtraResult;          /* Where to load the extra columns */
#endif
};

/*
** This routine does the work of loading query data into an array of
** registers so that it can be added to the sorter.
*/
ಸ್ಥಿರ ಶೂನ್ಯ innerLoopLoadRow(
  Parse *pParse,             /* Statement under construction */
  Select *pSelect,           /* The query being coded */
  RowLoadInfo *pInfo         /* Info needed to complete the row load */
){
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pSelect->pEList, pInfo->regResult,
                          0, pInfo->ecelFlags);
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  ಯದಿ( pInfo->pExtra ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pInfo->pExtra, pInfo->regExtraResult, 0, 0);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(pParse->db, pInfo->pExtra);
  }
#endif
}

/*
** Code the OP_MakeRecord instruction that generates the entry to be
** added into the sorter.
**
** Return the register in which the result is stored.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ makeSorterRecord(
  Parse *pParse,
  SortCtx *pSort,
  Select *pSelect,
  ಪೂರ್ಣಾಙ್ಕ regBase,
  ಪೂರ್ಣಾಙ್ಕ nBase
){
  ಪೂರ್ಣಾಙ್ಕ nOBSat = pSort->nOBSat;
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಙ್ಕ regOut = ++pParse->nMem;
  ಯದಿ( pSort->pDeferredRowLoad ){
    innerLoopLoadRow(pParse, pSelect, pSort->pDeferredRowLoad);
  }
  sqlite3VdbeAddOp3(v, OP_MakeRecord, regBase+nOBSat, nBase-nOBSat, regOut);
  ಪ್ರತ್ಯಾಗಮ regOut;
}

/*
** Generate code that will push the record in registers regData
** through regData+nData-1 onto the sorter.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ತಳ್ಳು_Onto_Sorter(
  Parse *pParse,         /* Parser context */
  SortCtx *pSort,        /* Information about the ORDER BY clause */
  Select *pSelect,       /* The whole SELECT statement */
  ಪೂರ್ಣಾಙ್ಕ regData,           /* First register holding data to be sorted */
  ಪೂರ್ಣಾಙ್ಕ regOrigData,       /* First register holding data before packing */
  ಪೂರ್ಣಾಙ್ಕ nData,             /* Number of elements in the regData data array */
  ಪೂರ್ಣಾಙ್ಕ nPrefixReg         /* No. of reg prior to regData available for use */
){
  Vdbe *v = pParse->pVdbe;                         /* Stmt under construction */
  ಪೂರ್ಣಾಙ್ಕ bSeq = ((pSort->sortFlags & SORTFLAG_UseSorter)==0);
  ಪೂರ್ಣಾಙ್ಕ n_ಅಭಿವ್ಯಕ್ತಿ = pSort->pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ;              /* No. of ORDER BY terms */
  ಪೂರ್ಣಾಙ್ಕ nBase = n_ಅಭಿವ್ಯಕ್ತಿ + bSeq + nData;                /* Fields in sorter record */
  ಪೂರ್ಣಾಙ್ಕ regBase;                                     /* Regs for sorter record */
  ಪೂರ್ಣಾಙ್ಕ regRecord = 0;                               /* Assembled sorter record */
  ಪೂರ್ಣಾಙ್ಕ nOBSat = pSort->nOBSat;                      /* ORDER BY terms to skip */
  ಪೂರ್ಣಾಙ್ಕ op;                            /* Opcode to add sorter record to sorter */
  ಪೂರ್ಣಾಙ್ಕ iLimit;                        /* LIMIT counter */
  ಪೂರ್ಣಾಙ್ಕ iSkip = 0;                     /* End of the sorter insert loop */

  ಪ್ರತಿಜ್ಞಾ( bSeq==0 || bSeq==1 );

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
  ಪ್ರತಿಜ್ಞಾ( nData==1 || regData==regOrigData || regOrigData==0 );

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  pSort->addr_ತಳ್ಳು = sqlite3VdbeCurrentAddr(v);
#endif

  ಯದಿ( nPrefixReg ){
    ಪ್ರತಿಜ್ಞಾ( nPrefixReg==n_ಅಭಿವ್ಯಕ್ತಿ+bSeq );
    regBase = regData - nPrefixReg;
  }ಅನ್ಯಥಾ{
    regBase = pParse->nMem + 1;
    pParse->nMem += nBase;
  }
  ಪ್ರತಿಜ್ಞಾ( pSelect->iOffset==0 || pSelect->iLimit!=0 );
  iLimit = pSelect->iOffset ? pSelect->iOffset+1 : pSelect->iLimit;
  pSort->labelDone = sqlite3VdbeMakeLabel(pParse);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pSort->pOrderBy, regBase, regOrigData,
                          SQLITE_ECEL_DUP | (regOrigData? SQLITE_ECEL_REF : 0));
  ಯದಿ( bSeq ){
    sqlite3VdbeAddOp2(v, OP_Sequence, pSort->iECursor, regBase+n_ಅಭಿವ್ಯಕ್ತಿ);
  }
  ಯದಿ( nPrefixReg==0 && nData>0 ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Move(pParse, regData, regBase+n_ಅಭಿವ್ಯಕ್ತಿ+bSeq, nData);
  }
  ಯದಿ( nOBSat>0 ){
    ಪೂರ್ಣಾಙ್ಕ reg_Prev_ಕೀಲಿ;   /* The first nOBSat columns of the previous row */
    ಪೂರ್ಣಾಙ್ಕ addrFirst;    /* Address of the OP_IfNot opcode */
    ಪೂರ್ಣಾಙ್ಕ addrJmp;      /* Address of the OP_Jump opcode */
    VdbeOp *pOp;      /* Opcode that opens the sorter */
    ಪೂರ್ಣಾಙ್ಕ n_ಕೀಲಿ;         /* Number of sorting key columns, including OP_Sequence */
    ಕೀಲಿ_Info *pKI;     /* Original KeyInfo on the sorter table */

    regRecord = makeSorterRecord(pParse, pSort, pSelect, regBase, nBase);
    reg_Prev_ಕೀಲಿ = pParse->nMem+1;
    pParse->nMem += pSort->nOBSat;
    n_ಕೀಲಿ = n_ಅಭಿವ್ಯಕ್ತಿ - pSort->nOBSat + bSeq;
    ಯದಿ( bSeq ){
      addrFirst = sqlite3VdbeAddOp1(v, OP_IfNot, regBase+n_ಅಭಿವ್ಯಕ್ತಿ);
    }ಅನ್ಯಥಾ{
      addrFirst = sqlite3VdbeAddOp1(v, OP_SequenceTest, pSort->iECursor);
    }
    VdbeCoverage(v);
    sqlite3VdbeAddOp3(v, OP_Compare, reg_Prev_ಕೀಲಿ, regBase, pSort->nOBSat);
    pOp = sqlite3VdbeGetOp(v, pSort->addrSortIndex);
    ಯದಿ( pParse->db->mallocFailed ) ಪ್ರತ್ಯಾಗಮ;
    pOp->p2 = n_ಕೀಲಿ + nData;
    pKI = pOp->p4.p_ಕೀಲಿ_Info;
    memset(pKI->aSortFlags, 0, pKI->n_ಕೀಲಿ_Field); /* Makes OP_Jump testable */
    sqlite3VdbeChangeP4(v, -1, (ವರ್ಣ*)pKI, P4_KEYINFO);
    testcase( pKI->nAllField > pKI->n_ಕೀಲಿ_Field+2 );
    pOp->p4.p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse,pSort->pOrderBy,nOBSat,
                                           pKI->nAllField-pKI->n_ಕೀಲಿ_Field-1);
    pOp = 0; /* Ensure pOp not used after sqlite3VdbeAddOp3() */
    addrJmp = sqlite3VdbeCurrentAddr(v);
    sqlite3VdbeAddOp3(v, OP_Jump, addrJmp+1, 0, addrJmp+1); VdbeCoverage(v);
    pSort->labelBkOut = sqlite3VdbeMakeLabel(pParse);
    pSort->regReturn = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Gosub, pSort->regReturn, pSort->labelBkOut);
    sqlite3VdbeAddOp1(v, OP_ResetSorter, pSort->iECursor);
    ಯದಿ( iLimit ){
      sqlite3VdbeAddOp2(v, OP_IfNot, iLimit, pSort->labelDone);
      VdbeCoverage(v);
    }
    sqlite3VdbeJumpHere(v, addrFirst);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Move(pParse, regBase, reg_Prev_ಕೀಲಿ, pSort->nOBSat);
    sqlite3VdbeJumpHere(v, addrJmp);
  }
  ಯದಿ( iLimit ){
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
    ಪೂರ್ಣಾಙ್ಕ iCsr = pSort->iECursor;
    sqlite3VdbeAddOp2(v, OP_IfNotZero, iLimit, sqlite3VdbeCurrentAddr(v)+4);
    VdbeCoverage(v);
    sqlite3VdbeAddOp2(v, OP_Last, iCsr, 0);
    iSkip = sqlite3VdbeAddOp4Int(v, OP_IdxLE,
                                 iCsr, 0, regBase+nOBSat, n_ಅಭಿವ್ಯಕ್ತಿ-nOBSat);
    VdbeCoverage(v);
    sqlite3VdbeAddOp1(v, OP_Delete, iCsr);
  }
  ಯದಿ( regRecord==0 ){
    regRecord = makeSorterRecord(pParse, pSort, pSelect, regBase, nBase);
  }
  ಯದಿ( pSort->sortFlags & SORTFLAG_UseSorter ){
    op = OP_Sorter_ಸೇರಿಸು;
  }ಅನ್ಯಥಾ{
    op = OP_Idx_ಸೇರಿಸು;
  }
  sqlite3VdbeAddOp4Int(v, op, pSort->iECursor, regRecord,
                       regBase+nOBSat, nBase-nOBSat);
  ಯದಿ( iSkip ){
    sqlite3VdbeChangeP2(v, iSkip,
         pSort->labelOBLopt ? pSort->labelOBLopt : sqlite3VdbeCurrentAddr(v));
  }
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  pSort->addr_ತಳ್ಳು_End = sqlite3VdbeCurrentAddr(v)-1;
#endif
}

/*
** Add code to implement the OFFSET
*/
ಸ್ಥಿರ ಶೂನ್ಯ codeOffset(
  Vdbe *v,          /* Generate code into this VM */
  ಪೂರ್ಣಾಙ್ಕ iOffset,      /* Register holding the offset counter */
  ಪೂರ್ಣಾಙ್ಕ iContinue     /* Jump here to skip the current record */
){
  ಯದಿ( iOffset>0 ){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ codeDistinct(
  Parse *pParse,     /* Parsing and code generating context */
  ಪೂರ್ಣಾಙ್ಕ eTnctType,     /* WHERE_DISTINCT_* value */
  ಪೂರ್ಣಾಙ್ಕ iTab,          /* A sorting index used to test for distinctness */
  ಪೂರ್ಣಾಙ್ಕ addrRepeat,    /* Jump to here if not distinct */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,  /* Expression for each element */
  ಪೂರ್ಣಾಙ್ಕ regElem        /* First element */
){
  ಪೂರ್ಣಾಙ್ಕ iRet = 0;
  ಪೂರ್ಣಾಙ್ಕ nResultCol = pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
  Vdbe *v = pParse->pVdbe;

  ವಿಕಲ್ಪ( eTnctType ){
    ಪಕ್ಷ WHERE_DISTINCT_ORDERED: {
      ಪೂರ್ಣಾಙ್ಕ i;
      ಪೂರ್ಣಾಙ್ಕ iJump;              /* Jump destination */
      ಪೂರ್ಣಾಙ್ಕ regPrev;            /* Previous row content */

      /* Allocate space for the previous row */
      iRet = regPrev = pParse->nMem+1;
      pParse->nMem += nResultCol;

      iJump = sqlite3VdbeCurrentAddr(v) + nResultCol;
      ಪ್ರತಿ(i=0; i<nResultCol; i++){
        CollSeq *pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ);
        ಯದಿ( i<nResultCol-1 ){
          sqlite3VdbeAddOp3(v, OP_Ne, regElem+i, iJump, regPrev+i);
          VdbeCoverage(v);
        }ಅನ್ಯಥಾ{
          sqlite3VdbeAddOp3(v, OP_Eq, regElem+i, addrRepeat, regPrev+i);
          VdbeCoverage(v);
         }
        sqlite3VdbeChangeP4(v, -1, (ಧ್ರುವ ವರ್ಣ *)pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
      }
      ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeCurrentAddr(v)==iJump || pParse->db->mallocFailed );
      sqlite3VdbeAddOp3(v, OP_Copy, regElem, regPrev, nResultCol-1);
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ WHERE_DISTINCT_UNIQUE: {
      /* nothing to do */
      ಭಙ್ಗ;
    }

    ಸ್ವಭಾವ: {
      ಪೂರ್ಣಾಙ್ಕ r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp4Int(v, OP_Found, iTab, addrRepeat, regElem, nResultCol);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regElem, nResultCol, r1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ಸೇರಿಸು, iTab, r1, regElem, nResultCol);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      sqlite3ReleaseTempReg(pParse, r1);
      iRet = iTab;
      ಭಙ್ಗ;
    }
  }

  ಪ್ರತ್ಯಾಗಮ iRet;
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
ಸ್ಥಿರ ಶೂನ್ಯ fixDistinctOpenEph(
  Parse *pParse,     /* Parsing and code generating context */
  ಪೂರ್ಣಾಙ್ಕ eTnctType,     /* WHERE_DISTINCT_* value */
  ಪೂರ್ಣಾಙ್ಕ iVal,          /* Value returned by codeDistinct() */
  ಪೂರ್ಣಾಙ್ಕ iOpenEphAddr   /* Address of OP_OpenEphemeral instruction for iTab */
){
  ಯದಿ( pParse->nErr==0
   && (eTnctType==WHERE_DISTINCT_UNIQUE || eTnctType==WHERE_DISTINCT_ORDERED)
  ){
    Vdbe *v = pParse->pVdbe;
    sqlite3VdbeChangeToNoop(v, iOpenEphAddr);
    ಯದಿ( sqlite3VdbeGetOp(v, iOpenEphAddr+1)->opcode==OP_Explain ){
      sqlite3VdbeChangeToNoop(v, iOpenEphAddr+1);
    }
    ಯದಿ( eTnctType==WHERE_DISTINCT_ORDERED ){
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
ಸ್ಥಿರ ಶೂನ್ಯ select_ಅಭಿವ್ಯಕ್ತಿ_Defer(
  Parse *pParse,                  /* Leave any error here */
  SortCtx *pSort,                 /* Sorter context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,               /* Expressions destined for sorter */
  ಅಭಿವ್ಯಕ್ತಿ_List **ppExtra              /* Expressions to append to sorter record */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ nDefer = 0;
  ಅಭಿವ್ಯಕ್ತಿ_List *pExtra = 0;
  ಪ್ರತಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem = &pEList->a[i];
    ಯದಿ( pItem->u.x.iOrderByCol==0 ){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pItem->p_ಅಭಿವ್ಯಕ್ತಿ;
      Table *pTab;
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN
       && p_ಅಭಿವ್ಯಕ್ತಿ->iColumn>=0
       && ALWAYS( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) )
       && (pTab = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab)!=0
       && IsOrdinaryTable(pTab)
       && (pTab->aCol[p_ಅಭಿವ್ಯಕ್ತಿ->iColumn].colFlags & COLFLAG_SORTERREF)!=0
      ){
        ಪೂರ್ಣಾಙ್ಕ j;
        ಪ್ರತಿ(j=0; j<nDefer; j++){
          ಯದಿ( pSort->aDefer[j].iCsr==p_ಅಭಿವ್ಯಕ್ತಿ->iTable ) ಭಙ್ಗ;
        }
        ಯದಿ( j==nDefer ){
          ಯದಿ( nDefer==ArraySize(pSort->aDefer) ){
            ಅನುವರ್ತ;
          }ಅನ್ಯಥಾ{
            ಪೂರ್ಣಾಙ್ಕ n_ಕೀಲಿ = 1;
            ಪೂರ್ಣಾಙ್ಕ k;
            Index *pPk = 0;
            ಯದಿ( !HasRowid(pTab) ){
              pPk = sqlite3_Primary_ಕೀಲಿ_Index(pTab);
              n_ಕೀಲಿ = pPk->n_ಕೀಲಿ_Col;
            }
            ಪ್ರತಿ(k=0; k<n_ಕೀಲಿ; k++){
              ಅಭಿವ್ಯಕ್ತಿ *pNew = sqlite3PExpr(pParse, TK_COLUMN, 0, 0);
              ಯದಿ( pNew ){
                pNew->iTable = p_ಅಭಿವ್ಯಕ್ತಿ->iTable;
                ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(pNew) );
                pNew->y.pTab = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab;
                pNew->iColumn = pPk ? pPk->aiColumn[k] : -1;
                pExtra = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pExtra, pNew);
              }
            }
            pSort->aDefer[nDefer].pTab = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab;
            pSort->aDefer[nDefer].iCsr = p_ಅಭಿವ್ಯಕ್ತಿ->iTable;
            pSort->aDefer[nDefer].n_ಕೀಲಿ = n_ಕೀಲಿ;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3CursorBloomable(Parse *pParse, ಪೂರ್ಣಾಙ್ಕ iCur, ಪೂರ್ಣಾಙ್ಕ nCol){
  ಪೂರ್ಣಾಙ್ಕ i,k;
  Vdbe *v = pParse->pVdbe;
  ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ 1;
  ಪ್ರತಿಜ್ಞಾ( v );
  ಪ್ರತಿ(k=sqlite3VdbeCurrentAddr(v)-1; k>0; k--){
    ಧ್ರುವ VdbeOp *pOp = sqlite3VdbeGetOp(v, k);
    ಧ್ರುವ ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;
    ಯದಿ( pOp->p1!=iCur ) ಅನುವರ್ತ;
    ಯದಿ( pOp->opcode!=OP_OpenRead
     && pOp->opcode!=OP_OpenWrite
     && pOp->opcode!=OP_OpenEphemeral
    ){
      ಅನುವರ್ತ;
    }
    ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_KEYINFO );
    p_ಕೀಲಿ_Info = (ಧ್ರುವ ಕೀಲಿ_Info*)pOp->p4.p_ಕೀಲಿ_Info;
    ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info!=0 );
    ಪ್ರತಿ(i=0; i<nCol; i++){
      ಪ್ರತಿಜ್ಞಾ( sqlite3_Is_ದ್ವಿಮಾನ(p_ಕೀಲಿ_Info->aColl[i]) );
    }
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಶೂನ್ಯ selectInnerLoop(
  Parse *pParse,          /* The parser context */
  Select *p,              /* The complete select statement being coded */
  ಪೂರ್ಣಾಙ್ಕ srcTab,             /* Pull data from this table if non-negative */
  SortCtx *pSort,         /* If not NULL, info on how to process ORDER BY */
  DistinctCtx *pDistinct, /* If not NULL, info on how to process DISTINCT */
  SelectDest *pDest,      /* How to dispose of the results */
  ಪೂರ್ಣಾಙ್ಕ iContinue,          /* Jump here to continue with next row */
  ಪೂರ್ಣಾಙ್ಕ iBreak              /* Jump here to break out of the inner loop */
){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ hasDistinct;            /* True if the DISTINCT keyword is present */
  ಪೂರ್ಣಾಙ್ಕ eDest = pDest->eDest;   /* How to dispose of results */
  ಪೂರ್ಣಾಙ್ಕ iParm = pDest->iSDParm; /* First argument to disposal method */
  ಪೂರ್ಣಾಙ್ಕ nResultCol;             /* Number of result columns */
  ಪೂರ್ಣಾಙ್ಕ nPrefixReg = 0;         /* Number of extra registers before regResult */
  RowLoadInfo sRowLoadInfo;   /* Info for deferred row loading */

  /* Usually, regResult is the first cell in an array of memory cells
  ** containing the current result row. In this case regOrig is set to the
  ** same value. However, if the results are being sent to the sorter, the
  ** values for any expressions that are also part of the sort-key are omitted
  ** from this array. In this case regOrig is set to zero.  */
  ಪೂರ್ಣಾಙ್ಕ regResult;              /* Start of memory holding current results */
  ಪೂರ್ಣಾಙ್ಕ regOrig;                /* Start of memory holding full result (or 0) */

  ಪ್ರತಿಜ್ಞಾ( v );
  ಪ್ರತಿಜ್ಞಾ( p->pEList!=0 );
  hasDistinct = pDistinct ? pDistinct->eTnctType : WHERE_DISTINCT_NOOP;
  ಯದಿ( pSort && pSort->pOrderBy==0 ) pSort = 0;
  ಯದಿ( pSort==0 && !hasDistinct ){
    ಪ್ರತಿಜ್ಞಾ( iContinue!=0 );
    codeOffset(v, p->iOffset, iContinue);
  }

  /* Pull the requested columns.
  */
  nResultCol = p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;

  ಯದಿ( pDest->iSdst==0 ){
    ಯದಿ( pSort ){
      nPrefixReg = pSort->pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ;
      ಯದಿ( !(pSort->sortFlags & SORTFLAG_UseSorter) ) nPrefixReg++;
      pParse->nMem += nPrefixReg;
    }
    pDest->iSdst = pParse->nMem+1;
    pParse->nMem += nResultCol;
  }ಅನ್ಯಥಾ ಯದಿ( pDest->iSdst+nResultCol > pParse->nMem ){
    /* This is an error condition that can result, for example, when a SELECT
    ** on the right-hand side of an INSERT contains more result columns than
    ** there are columns in the table on the left.  The error will be caught
    ** and reported later.  But we need to make sure enough memory is allocated
    ** to avoid other spurious errors in the meantime. */
    pParse->nMem += nResultCol;
  }
  pDest->nSdst = nResultCol;
  regOrig = regResult = pDest->iSdst;
  ಯದಿ( srcTab>=0 ){
    ಪ್ರತಿ(i=0; i<nResultCol; i++){
      sqlite3VdbeAddOp3(v, OP_Column, srcTab, i, regResult+i);
      VdbeComment((v, "%s", p->pEList->a[i].zEName));
    }
  }ಅನ್ಯಥಾ ಯದಿ( eDest!=SRT_Exists ){
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    ಅಭಿವ್ಯಕ್ತಿ_List *pExtra = 0;
#endif
    /* If the destination is an EXISTS(...) expression, the actual
    ** values returned by the SELECT are not required.
    */
    u8 ecelFlags;    /* "ecel" is an abbreviation of "ExprCodeExprList" */
    ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
    ಯದಿ( eDest==SRT_Mem || eDest==SRT_Output || eDest==SRT_Coroutine ){
      ecelFlags = SQLITE_ECEL_DUP;
    }ಅನ್ಯಥಾ{
      ecelFlags = 0;
    }
    ಯದಿ( pSort && hasDistinct==0 && eDest!=SRT_EphemTab && eDest!=SRT_Table ){
      /* For each expression in p->pEList that is a copy of an expression in
      ** the ORDER BY clause (pSort->pOrderBy), set the associated
      ** iOrderByCol value to one more than the index of the ORDER BY
      ** expression within the sort-key that pushOntoSorter() will generate.
      ** This allows the p->pEList field to be omitted from the sorted record,
      ** saving space and CPU cycles.  */
      ecelFlags |= (SQLITE_ECEL_OMITREF|SQLITE_ECEL_REF);

      ಪ್ರತಿ(i=pSort->nOBSat; i<pSort->pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
        ಪೂರ್ಣಾಙ್ಕ j;
        ಯದಿ( (j = pSort->pOrderBy->a[i].u.x.iOrderByCol)>0 ){
          p->pEList->a[j-1].u.x.iOrderByCol = i+1-pSort->nOBSat;
        }
      }
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
      select_ಅಭಿವ್ಯಕ್ತಿ_Defer(pParse, pSort, p->pEList, &pExtra);
      ಯದಿ( pExtra && pParse->db->mallocFailed==0 ){
        /* If there are any extra PK columns to add to the sorter records,
        ** allocate extra memory cells and adjust the OpenEphemeral
        ** instruction to account for the larger records. This is only
        ** required if there are one or more WITHOUT ROWID tables with
        ** composite primary keys in the SortCtx.aDefer[] array.  */
        VdbeOp *pOp = sqlite3VdbeGetOp(v, pSort->addrSortIndex);
        pOp->p2 += (pExtra->n_ಅಭಿವ್ಯಕ್ತಿ - pSort->nDefer);
        pOp->p4.p_ಕೀಲಿ_Info->nAllField += (pExtra->n_ಅಭಿವ್ಯಕ್ತಿ - pSort->nDefer);
        pParse->nMem += pExtra->n_ಅಭಿವ್ಯಕ್ತಿ;
      }
#endif

      /* Adjust nResultCol to account for columns that are omitted
      ** from the sorter by the optimizations in this branch */
      pEList = p->pEList;
      ಪ್ರತಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
        ಯದಿ( pEList->a[i].u.x.iOrderByCol>0
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
      ಪ್ರತಿಜ್ಞಾ( eDest==SRT_Set || eDest==SRT_Mem
           || eDest==SRT_Coroutine || eDest==SRT_Output
           || eDest==SRT_Upfrom );
    }
    sRowLoadInfo.regResult = regResult;
    sRowLoadInfo.ecelFlags = ecelFlags;
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    sRowLoadInfo.pExtra = pExtra;
    sRowLoadInfo.regExtraResult = regResult + nResultCol;
    ಯದಿ( pExtra ) nResultCol += pExtra->n_ಅಭಿವ್ಯಕ್ತಿ;
#endif
    ಯದಿ( p->iLimit
     && (ecelFlags & SQLITE_ECEL_OMITREF)!=0
     && nPrefixReg>0
    ){
      ಪ್ರತಿಜ್ಞಾ( pSort!=0 );
      ಪ್ರತಿಜ್ಞಾ( hasDistinct==0 );
      pSort->pDeferredRowLoad = &sRowLoadInfo;
      regOrig = 0;
    }ಅನ್ಯಥಾ{
      innerLoopLoadRow(pParse, p, &sRowLoadInfo);
    }
  }

  /* If the DISTINCT keyword was present on the SELECT statement
  ** and this row has been seen before, then do not make this row
  ** part of the result.
  */
  ಯದಿ( hasDistinct ){
    ಪೂರ್ಣಾಙ್ಕ eType = pDistinct->eTnctType;
    ಪೂರ್ಣಾಙ್ಕ iTab = pDistinct->tabTnct;
    ಪ್ರತಿಜ್ಞಾ( nResultCol==p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ );
    iTab = codeDistinct(pParse, eType, iTab, iContinue, p->pEList, regResult);
    fixDistinctOpenEph(pParse, eType, iTab, pDistinct->addrTnct);
    ಯದಿ( pSort==0 ){
      codeOffset(v, p->iOffset, iContinue);
    }
  }

  ವಿಕಲ್ಪ( eDest ){
    /* Store the result as data using a unique key.
    */
    ಪಕ್ಷ SRT_Fifo:
    ಪಕ್ಷ SRT_DistFifo:
    ಪಕ್ಷ SRT_Table:
    ಪಕ್ಷ SRT_EphemTab: {
      ಪೂರ್ಣಾಙ್ಕ r1 = sqlite3GetTempRange(pParse, nPrefixReg+1);
      testcase( eDest==SRT_Table );
      testcase( eDest==SRT_EphemTab );
      testcase( eDest==SRT_Fifo );
      testcase( eDest==SRT_DistFifo );
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regResult, nResultCol, r1+nPrefixReg);
#ಯದಿ !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      ಯದಿ( eDest==SRT_Table && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#endif
#ifndef SQLITE_OMIT_CTE
      ಯದಿ( eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index. If the current row is already present
        ** in the index, do not write it to the output. If not, add the
        ** current row to the index and proceed with writing it to the
        ** output table as well.  */
        ಪೂರ್ಣಾಙ್ಕ addr = sqlite3VdbeCurrentAddr(v) + 4;
        sqlite3VdbeAddOp4Int(v, OP_Found, iParm+1, addr, r1, 0);
        VdbeCoverage(v);
        sqlite3VdbeAddOp4Int(v, OP_Idx_ಸೇರಿಸು, iParm+1, r1,regResult,nResultCol);
        ಪ್ರತಿಜ್ಞಾ( pSort==0 );
      }
#endif
      ಯದಿ( pSort ){
        ಪ್ರತಿಜ್ಞಾ( regResult==regOrig );
        ತಳ್ಳು_Onto_Sorter(pParse, pSort, p, r1+nPrefixReg, regOrig, 1, nPrefixReg);
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ r2 = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp2(v, OP_NewRowid, iParm, r2);
        sqlite3VdbeAddOp3(v, OP_ಸೇರಿಸು, iParm, r1, r2);
        sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
        sqlite3ReleaseTempReg(pParse, r2);
      }
      sqlite3ReleaseTempRange(pParse, r1, nPrefixReg+1);
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ SRT_Upfrom: {
      ಯದಿ( pSort ){
        ತಳ್ಳು_Onto_Sorter(
            pParse, pSort, p, regResult, regOrig, nResultCol, nPrefixReg);
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ i2 = pDest->iSDParm2;
        ಪೂರ್ಣಾಙ್ಕ r1 = sqlite3GetTempReg(pParse);

        /* If the UPDATE FROM join is an aggregate that matches no rows, it
        ** might still be trying to return one row, because that is what
        ** aggregates do.  Don't record that empty row in the output table. */
        sqlite3VdbeAddOp2(v, OP_IsNull, regResult, iBreak); VdbeCoverage(v);

        sqlite3VdbeAddOp3(v, OP_MakeRecord,
                          regResult+(i2<0), nResultCol-(i2<0), r1);
        ಯದಿ( i2<0 ){
          sqlite3VdbeAddOp3(v, OP_ಸೇರಿಸು, iParm, r1, regResult);
        }ಅನ್ಯಥಾ{
          sqlite3VdbeAddOp4Int(v, OP_Idx_ಸೇರಿಸು, iParm, r1, regResult, i2);
        }
      }
      ಭಙ್ಗ;
    }

#ifndef SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)" construct,
    ** then there should be a single item on the stack.  Write this
    ** item into the set table with bogus data.
    */
    ಪಕ್ಷ SRT_Set: {
      ಯದಿ( pSort ){
        /* At first glance you would think we could optimize out the
        ** ORDER BY in this case since the order of entries in the set
        ** does not matter.  But there might be a LIMIT clause, in which
        ** case the order does matter */
        ತಳ್ಳು_Onto_Sorter(
            pParse, pSort, p, regResult, regOrig, nResultCol, nPrefixReg);
        pDest->iSDParm2 = 0; /* Signal that any Bloom filter is unpopulated */
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ r1 = sqlite3GetTempReg(pParse);
        ಪ್ರತಿಜ್ಞಾ( sqlite3Strlen30(pDest->zAffSdst)==nResultCol );
        sqlite3VdbeAddOp4(v, OP_MakeRecord, regResult, nResultCol,
            r1, pDest->zAffSdst, nResultCol);
        sqlite3VdbeAddOp4Int(v, OP_Idx_ಸೇರಿಸು, iParm, r1, regResult, nResultCol);
        ಯದಿ( pDest->iSDParm2 ){
          ಪ್ರತಿಜ್ಞಾ( sqlite3CursorBloomable(pParse,iParm,nResultCol) );
          sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pDest->iSDParm2, 0,
                               regResult, nResultCol);
          ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
        }
        sqlite3ReleaseTempReg(pParse, r1);
      }
      ಭಙ್ಗ;
    }


    /* If any row exist in the result set, record that fact and abort.
    */
    ಪಕ್ಷ SRT_Exists: {
      sqlite3VdbeAddOp2(v, OP_Integer, 1, iParm);
      /* The LIMIT clause will terminate the loop for us */
      ಭಙ್ಗ;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell or array of
    ** memory cells and break out of the scan loop.
    */
    ಪಕ್ಷ SRT_Mem: {
      ಯದಿ( pSort ){
        ಪ್ರತಿಜ್ಞಾ( nResultCol<=pDest->nSdst );
        ತಳ್ಳು_Onto_Sorter(
            pParse, pSort, p, regResult, regOrig, nResultCol, nPrefixReg);
        pDest->iSDParm = regResult;
      }ಅನ್ಯಥಾ{
        ಪ್ರತಿಜ್ಞಾ( nResultCol==pDest->nSdst );
        ಯದಿ( regResult!=iParm ){
          /* This occurs in cases where the SELECT had both a DISTINCT and
          ** an OFFSET clause.  */
          sqlite3VdbeAddOp3(v, OP_Copy, regResult, iParm, nResultCol-1);
        }
        /* The LIMIT clause will jump out of the loop for us */
      }
      ಭಙ್ಗ;
    }
#endif /* #ifndef SQLITE_OMIT_SUBQUERY */

    ಪಕ್ಷ SRT_Coroutine:       /* Send data to a co-routine */
    ಪಕ್ಷ SRT_Output: {        /* Return the results */
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      ಯದಿ( pSort ){
        ತಳ್ಳು_Onto_Sorter(pParse, pSort, p, regResult, regOrig, nResultCol,
                       nPrefixReg);
      }ಅನ್ಯಥಾ ಯದಿ( eDest==SRT_Coroutine ){
        sqlite3VdbeAddOp1(v, OP_Yield, pDest->iSDParm);
      }ಅನ್ಯಥಾ{
        sqlite3VdbeAddOp2(v, OP_ResultRow, regResult, nResultCol);
      }
      ಭಙ್ಗ;
    }

#ifndef SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    ಪಕ್ಷ SRT_DistQueue:
    ಪಕ್ಷ SRT_Queue: {
      ಪೂರ್ಣಾಙ್ಕ n_ಕೀಲಿ;
      ಪೂರ್ಣಾಙ್ಕ r1, r2, r3;
      ಪೂರ್ಣಾಙ್ಕ addrTest = 0;
      ಅಭಿವ್ಯಕ್ತಿ_List *pSO;
      pSO = pDest->pOrderBy;
      ಪ್ರತಿಜ್ಞಾ( pSO );
      n_ಕೀಲಿ = pSO->n_ಅಭಿವ್ಯಕ್ತಿ;
      r1 = sqlite3GetTempReg(pParse);
      r2 = sqlite3GetTempRange(pParse, n_ಕೀಲಿ+2);
      r3 = r2+n_ಕೀಲಿ+1;
      ಯದಿ( eDest==SRT_DistQueue ){
        /* If the destination is DistQueue, then cursor (iParm+1) is open
        ** on a second ephemeral index that holds all values every previously
        ** added to the queue. */
        addrTest = sqlite3VdbeAddOp4Int(v, OP_Found, iParm+1, 0,
                                        regResult, nResultCol);
        VdbeCoverage(v);
      }
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regResult, nResultCol, r3);
      ಯದಿ( eDest==SRT_DistQueue ){
        sqlite3VdbeAddOp2(v, OP_Idx_ಸೇರಿಸು, iParm+1, r3);
        sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      }
      ಪ್ರತಿ(i=0; i<n_ಕೀಲಿ; i++){
        sqlite3VdbeAddOp2(v, OP_SCopy,
                          regResult + pSO->a[i].u.x.iOrderByCol - 1,
                          r2+i);
      }
      sqlite3VdbeAddOp2(v, OP_Sequence, iParm, r2+n_ಕೀಲಿ);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, r2, n_ಕೀಲಿ+2, r1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ಸೇರಿಸು, iParm, r1, r2, n_ಕೀಲಿ+2);
      ಯದಿ( addrTest ) sqlite3VdbeJumpHere(v, addrTest);
      sqlite3ReleaseTempReg(pParse, r1);
      sqlite3ReleaseTempRange(pParse, r2, n_ಕೀಲಿ+2);
      ಭಙ್ಗ;
    }
#endif /* SQLITE_OMIT_CTE */



#ಯದಿ !defined(SQLITE_OMIT_TRIGGER)
    /* Discard the results.  This is used for SELECT statements inside
    ** the body of a TRIGGER.  The purpose of such selects is to call
    ** user-defined functions that have side effects.  We do not care
    ** about the actual results of the select.
    */
    ಸ್ವಭಾವ: {
      ಪ್ರತಿಜ್ಞಾ( eDest==SRT_Discard );
      ಭಙ್ಗ;
    }
#endif
  }

  /* Jump to the end of the loop if the LIMIT is reached.  Except, if
  ** there is a sorter, in which case the sorter has already limited
  ** the output for us.
  */
  ಯದಿ( pSort==0 && p->iLimit ){
    sqlite3VdbeAddOp2(v, OP_DecrJumpZero, p->iLimit, iBreak); VdbeCoverage(v);
  }
}

/*
** Allocate a KeyInfo object sufficient for an index of N key columns and
** X extra columns.
*/
ಕೀಲಿ_Info *sqlite3_ಕೀಲಿ_Info_Alloc(sqlite3 *db, ಪೂರ್ಣಾಙ್ಕ N, ಪೂರ್ಣಾಙ್ಕ X){
  ಪೂರ್ಣಾಙ್ಕ nExtra = (N+X)*(ಪರಿಮಾಣ(CollSeq*)+1);
  ಕೀಲಿ_Info *p;
  ಪ್ರತಿಜ್ಞಾ( X>=0 );
  ಯದಿ( NEVER(N+X>0xffff) ) ಪ್ರತ್ಯಾಗಮ (ಕೀಲಿ_Info*)sqlite3OomFault(db);
  p = sqlite3DbMallocRawNN(db, SZ_KEYINFO(0) + nExtra);
  ಯದಿ( p ){
    p->aSortFlags = (u8*)&p->aColl[N+X];
    p->n_ಕೀಲಿ_Field = (u16)N;
    p->nAllField = (u16)(N+X);
    p->enc = ENC(db);
    p->db = db;
    p->nRef = 1;
    memset(p->aColl, 0, nExtra);
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ (ಕೀಲಿ_Info*)sqlite3OomFault(db);
  }
  ಪ್ರತ್ಯಾಗಮ p;
}

/*
** Deallocate a KeyInfo object
*/
ಶೂನ್ಯ sqlite3_ಕೀಲಿ_Info_Unref(ಕೀಲಿ_Info *p){
  ಯದಿ( p ){
    ಪ್ರತಿಜ್ಞಾ( p->db!=0 );
    ಪ್ರತಿಜ್ಞಾ( p->nRef>0 );
    p->nRef--;
    ಯದಿ( p->nRef==0 ) sqlite3DbNNFreeNN(p->db, p);
  }
}

/*
** Make a new pointer to a KeyInfo object
*/
ಕೀಲಿ_Info *sqlite3_ಕೀಲಿ_Info_Ref(ಕೀಲಿ_Info *p){
  ಯದಿ( p ){
    ಪ್ರತಿಜ್ಞಾ( p->nRef>0 );
    p->nRef++;
  }
  ಪ್ರತ್ಯಾಗಮ p;
}

#ifdef SQLITE_DEBUG
/*
** Return TRUE if a KeyInfo object can be change.  The KeyInfo object
** can only be changed if this is just a single reference to the object.
**
** This routine is used only inside of assert() statements.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಕೀಲಿ_Info_Is_Writeable(ಕೀಲಿ_Info *p){ ಪ್ರತ್ಯಾಗಮ p->nRef==1; }
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
ಕೀಲಿ_Info *sqlite3_ಕೀಲಿ_Info_From_ಅಭಿವ್ಯಕ್ತಿ_List(
  Parse *pParse,       /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList,     /* Form the KeyInfo object from this ExprList */
  ಪೂರ್ಣಾಙ್ಕ iStart,          /* Begin with this column of pList */
  ಪೂರ್ಣಾಙ್ಕ nExtra           /* Add this many extra columns to the end */
){
  ಪೂರ್ಣಾಙ್ಕ n_ಅಭಿವ್ಯಕ್ತಿ;
  ಕೀಲಿ_Info *pInfo;
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಙ್ಕ i;

  n_ಅಭಿವ್ಯಕ್ತಿ = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
  pInfo = sqlite3_ಕೀಲಿ_Info_Alloc(db, n_ಅಭಿವ್ಯಕ್ತಿ-iStart, nExtra+1);
  ಯದಿ( pInfo ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3_ಕೀಲಿ_Info_Is_Writeable(pInfo) );
    ಪ್ರತಿ(i=iStart, pItem=pList->a+iStart; i<n_ಅಭಿವ್ಯಕ್ತಿ; i++, pItem++){
      pInfo->aColl[i-iStart] = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_NNColl_Seq(pParse, pItem->p_ಅಭಿವ್ಯಕ್ತಿ);
      pInfo->aSortFlags[i-iStart] = pItem->fg.sortFlags;
    }
  }
  ಪ್ರತ್ಯಾಗಮ pInfo;
}

/*
** Name of the connection operator, used for error messages.
*/
ಧ್ರುವ ವರ್ಣ *sqlite3SelectOpName(ಪೂರ್ಣಾಙ್ಕ id){
  ವರ್ಣ *z;
  ವಿಕಲ್ಪ( id ){
    ಪಕ್ಷ TK_ALL:       z = "UNION ALL";   ಭಙ್ಗ;
    ಪಕ್ಷ TK_INTERSECT: z = "INTERSECT";   ಭಙ್ಗ;
    ಪಕ್ಷ TK_EXCEPT:    z = "EXCEPT";      ಭಙ್ಗ;
    ಸ್ವಭಾವ:           z = "UNION";       ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ z;
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
ಸ್ಥಿರ ಶೂನ್ಯ explainTempTable(Parse *pParse, ಧ್ರುವ ವರ್ಣ *zUsage){
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

#ಅನ್ಯಥಾ
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
ಸ್ಥಿರ ಶೂನ್ಯ generateSortTail(
  Parse *pParse,    /* Parsing context */
  Select *p,        /* The SELECT statement */
  SortCtx *pSort,   /* Information on the ORDER BY clause */
  ಪೂರ್ಣಾಙ್ಕ nColumn,      /* Number of columns of data */
  SelectDest *pDest /* Write the sorted results here */
){
  Vdbe *v = pParse->pVdbe;                     /* The prepared statement */
  ಪೂರ್ಣಾಙ್ಕ addrBreak = pSort->labelDone;            /* Jump here to exit loop */
  ಪೂರ್ಣಾಙ್ಕ addrContinue = sqlite3VdbeMakeLabel(pParse);/* Jump here for next cycle */
  ಪೂರ್ಣಾಙ್ಕ addr;                       /* Top of output loop. Jump for Next. */
  ಪೂರ್ಣಾಙ್ಕ addrOnce = 0;
  ಪೂರ್ಣಾಙ್ಕ iTab;
  ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy = pSort->pOrderBy;
  ಪೂರ್ಣಾಙ್ಕ eDest = pDest->eDest;
  ಪೂರ್ಣಾಙ್ಕ iParm = pDest->iSDParm;
  ಪೂರ್ಣಾಙ್ಕ regRow;
  ಪೂರ್ಣಾಙ್ಕ regRowid;
  ಪೂರ್ಣಾಙ್ಕ iCol;
  ಪೂರ್ಣಾಙ್ಕ n_ಕೀಲಿ;                       /* Number of key columns in sorter record */
  ಪೂರ್ಣಾಙ್ಕ iSortTab;                   /* Sorter cursor to read from */
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ bSeq;                       /* True if sorter record includes seq. no. */
  ಪೂರ್ಣಾಙ್ಕ n_Ref_ಕೀಲಿ = 0;
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *aOutEx = p->pEList->a;
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  ಪೂರ್ಣಾಙ್ಕ addrExplain;                /* Address of OP_Explain instruction */
#endif

  n_ಕೀಲಿ = pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ - pSort->nOBSat;
  ಯದಿ( pSort->nOBSat==0 || n_ಕೀಲಿ==1 ){
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR %sORDER BY", pSort->nOBSat?"LAST TERM OF ":""
    ));
  }ಅನ್ಯಥಾ{
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR LAST %d TERMS OF ORDER BY", n_ಕೀಲಿ
    ));
  }
  sqlite3VdbeScanStatusRange(v, addrExplain,pSort->addr_ತಳ್ಳು,pSort->addr_ತಳ್ಳು_End);
  sqlite3VdbeScanStatusCounters(v, addrExplain, addrExplain, pSort->addr_ತಳ್ಳು);


  ಪ್ರತಿಜ್ಞಾ( addrBreak<0 );
  ಯದಿ( pSort->labelBkOut ){
    sqlite3VdbeAddOp2(v, OP_Gosub, pSort->regReturn, pSort->labelBkOut);
    sqlite3VdbeGoto(v, addrBreak);
    sqlite3VdbeResolveLabel(v, pSort->labelBkOut);
  }

#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  /* Open any cursors needed for sorter-reference expressions */
  ಪ್ರತಿ(i=0; i<pSort->nDefer; i++){
    Table *pTab = pSort->aDefer[i].pTab;
    ಪೂರ್ಣಾಙ್ಕ iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
    sqlite3OpenTable(pParse, pSort->aDefer[i].iCsr, iDb, pTab, OP_OpenRead);
    sqlite3VdbeAddOp2(v, OP_Rewind, pSort->aDefer[i].iCsr,
                      sqlite3VdbeCurrentAddr(v)+1);
    VdbeCoverage(v);
    n_Ref_ಕೀಲಿ = MAX(n_Ref_ಕೀಲಿ, pSort->aDefer[i].n_ಕೀಲಿ);
  }
#endif

  iTab = pSort->iECursor;
  ಯದಿ( eDest==SRT_Output || eDest==SRT_Coroutine || eDest==SRT_Mem ){
    ಯದಿ( eDest==SRT_Mem && p->iOffset ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, pDest->iSdst);
    }
    regRowid = 0;
    regRow = pDest->iSdst;
  }ಅನ್ಯಥಾ{
    regRowid = sqlite3GetTempReg(pParse);
    ಯದಿ( eDest==SRT_EphemTab || eDest==SRT_Table ){
      regRow = sqlite3GetTempReg(pParse);
      nColumn = 0;
    }ಅನ್ಯಥಾ{
      regRow = sqlite3GetTempRange(pParse, nColumn);
    }
  }
  ಯದಿ( pSort->sortFlags & SORTFLAG_UseSorter ){
    ಪೂರ್ಣಾಙ್ಕ regSortOut = ++pParse->nMem;
    iSortTab = pParse->nTab++;
    ಯದಿ( pSort->labelBkOut ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    }
    sqlite3VdbeAddOp3(v, OP_OpenPseudo, iSortTab, regSortOut,
        n_ಕೀಲಿ+1+nColumn+n_Ref_ಕೀಲಿ);
    ಯದಿ( addrOnce ) sqlite3VdbeJumpHere(v, addrOnce);
    addr = 1 + sqlite3VdbeAddOp2(v, OP_SorterSort, iTab, addrBreak);
    VdbeCoverage(v);
    ಪ್ರತಿಜ್ಞಾ( p->iLimit==0 && p->iOffset==0 );
    sqlite3VdbeAddOp3(v, OP_SorterData, iTab, regSortOut, iSortTab);
    bSeq = 0;
  }ಅನ್ಯಥಾ{
    addr = 1 + sqlite3VdbeAddOp2(v, OP_Sort, iTab, addrBreak); VdbeCoverage(v);
    codeOffset(v, p->iOffset, addrContinue);
    iSortTab = iTab;
    bSeq = 1;
    ಯದಿ( p->iOffset>0 ){
      sqlite3VdbeAddOp2(v, OP_AddImm, p->iLimit, -1);
    }
  }
  ಪ್ರತಿ(i=0, iCol=n_ಕೀಲಿ+bSeq-1; i<nColumn; i++){
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    ಯದಿ( aOutEx[i].fg.bSorterRef ) ಅನುವರ್ತ;
#endif
    ಯದಿ( aOutEx[i].u.x.iOrderByCol==0 ) iCol++;
  }
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  ಯದಿ( pSort->nDefer ){
    ಪೂರ್ಣಾಙ್ಕ i_ಕೀಲಿ = iCol+1;
    ಪೂರ್ಣಾಙ್ಕ reg_ಕೀಲಿ = sqlite3GetTempRange(pParse, n_Ref_ಕೀಲಿ);

    ಪ್ರತಿ(i=0; i<pSort->nDefer; i++){
      ಪೂರ್ಣಾಙ್ಕ iCsr = pSort->aDefer[i].iCsr;
      Table *pTab = pSort->aDefer[i].pTab;
      ಪೂರ್ಣಾಙ್ಕ n_ಕೀಲಿ = pSort->aDefer[i].n_ಕೀಲಿ;

      sqlite3VdbeAddOp1(v, OP_NullRow, iCsr);
      ಯದಿ( HasRowid(pTab) ){
        sqlite3VdbeAddOp3(v, OP_Column, iSortTab, i_ಕೀಲಿ++, reg_ಕೀಲಿ);
        sqlite3VdbeAddOp3(v, OP_SeekRowid, iCsr,
            sqlite3VdbeCurrentAddr(v)+1, reg_ಕೀಲಿ);
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ k;
        ಪೂರ್ಣಾಙ್ಕ iJmp;
        ಪ್ರತಿಜ್ಞಾ( sqlite3_Primary_ಕೀಲಿ_Index(pTab)->n_ಕೀಲಿ_Col==n_ಕೀಲಿ );
        ಪ್ರತಿ(k=0; k<n_ಕೀಲಿ; k++){
          sqlite3VdbeAddOp3(v, OP_Column, iSortTab, i_ಕೀಲಿ++, reg_ಕೀಲಿ+k);
        }
        iJmp = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeAddOp4Int(v, OP_SeekGE, iCsr, iJmp+2, reg_ಕೀಲಿ, n_ಕೀಲಿ);
        sqlite3VdbeAddOp4Int(v, OP_IdxLE, iCsr, iJmp+3, reg_ಕೀಲಿ, n_ಕೀಲಿ);
        sqlite3VdbeAddOp1(v, OP_NullRow, iCsr);
      }
    }
    sqlite3ReleaseTempRange(pParse, reg_ಕೀಲಿ, n_Ref_ಕೀಲಿ);
  }
#endif
  ಪ್ರತಿ(i=nColumn-1; i>=0; i--){
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    ಯದಿ( aOutEx[i].fg.bSorterRef ){
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, aOutEx[i].p_ಅಭಿವ್ಯಕ್ತಿ, regRow+i);
    }ಅನ್ಯಥಾ
#endif
    {
      ಪೂರ್ಣಾಙ್ಕ iRead;
      ಯದಿ( aOutEx[i].u.x.iOrderByCol ){
        iRead = aOutEx[i].u.x.iOrderByCol-1;
      }ಅನ್ಯಥಾ{
        iRead = iCol--;
      }
      sqlite3VdbeAddOp3(v, OP_Column, iSortTab, iRead, regRow+i);
      VdbeComment((v, "%s", aOutEx[i].zEName));
    }
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);
  ವಿಕಲ್ಪ( eDest ){
    ಪಕ್ಷ SRT_Table:
    ಪಕ್ಷ SRT_EphemTab: {
      sqlite3VdbeAddOp3(v, OP_Column, iSortTab, n_ಕೀಲಿ+bSeq, regRow);
      sqlite3VdbeAddOp2(v, OP_NewRowid, iParm, regRowid);
      sqlite3VdbeAddOp3(v, OP_ಸೇರಿಸು, iParm, regRow, regRowid);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      ಭಙ್ಗ;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ಪಕ್ಷ SRT_Set: {
      ಪ್ರತಿಜ್ಞಾ( nColumn==sqlite3Strlen30(pDest->zAffSdst) );
      sqlite3VdbeAddOp4(v, OP_MakeRecord, regRow, nColumn, regRowid,
                        pDest->zAffSdst, nColumn);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ಸೇರಿಸು, iParm, regRowid, regRow, nColumn);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ SRT_Mem: {
      /* The LIMIT clause will terminate the loop for us */
      ಭಙ್ಗ;
    }
#endif
    ಪಕ್ಷ SRT_Upfrom: {
      ಪೂರ್ಣಾಙ್ಕ i2 = pDest->iSDParm2;
      ಪೂರ್ಣಾಙ್ಕ r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, OP_MakeRecord,regRow+(i2<0),nColumn-(i2<0),r1);
      ಯದಿ( i2<0 ){
        sqlite3VdbeAddOp3(v, OP_ಸೇರಿಸು, iParm, r1, regRow);
      }ಅನ್ಯಥಾ{
        sqlite3VdbeAddOp4Int(v, OP_Idx_ಸೇರಿಸು, iParm, r1, regRow, i2);
      }
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      ಪ್ರತಿಜ್ಞಾ( eDest==SRT_Output || eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      testcase( eDest==SRT_Coroutine );
      ಯದಿ( eDest==SRT_Output ){
        sqlite3VdbeAddOp2(v, OP_ResultRow, pDest->iSdst, nColumn);
      }ಅನ್ಯಥಾ{
        sqlite3VdbeAddOp1(v, OP_Yield, pDest->iSDParm);
      }
      ಭಙ್ಗ;
    }
  }
  ಯದಿ( regRowid ){
    ಯದಿ( eDest==SRT_Set ){
      sqlite3ReleaseTempRange(pParse, regRow, nColumn);
    }ಅನ್ಯಥಾ{
      sqlite3ReleaseTempReg(pParse, regRow);
    }
    sqlite3ReleaseTempReg(pParse, regRowid);
  }
  /* The bottom of the loop
  */
  sqlite3VdbeResolveLabel(v, addrContinue);
  ಯದಿ( pSort->sortFlags & SORTFLAG_UseSorter ){
    sqlite3VdbeAddOp2(v, OP_SorterNext, iTab, addr); VdbeCoverage(v);
  }ಅನ್ಯಥಾ{
    sqlite3VdbeAddOp2(v, OP_Next, iTab, addr); VdbeCoverage(v);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, sqlite3VdbeCurrentAddr(v)-1, -1);
  ಯದಿ( pSort->regReturn ) sqlite3VdbeAddOp1(v, OP_Return, pSort->regReturn);
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
#ಅನ್ಯಥಾ /* if !defined(SQLITE_ENABLE_COLUMN_METADATA) */
# define columnType(A,B,C,D,E) columnTypeImpl(A,B)
#endif
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *columnTypeImpl(
  NameContext *pNC,
#ifndef SQLITE_ENABLE_COLUMN_METADATA
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ
#ಅನ್ಯಥಾ
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,
  ಧ್ರುವ ವರ್ಣ **pzOrigDb,
  ಧ್ರುವ ವರ್ಣ **pzOrigTab,
  ಧ್ರುವ ವರ್ಣ **pzOrigCol
#endif
){
  ವರ್ಣ ಧ್ರುವ *zType = 0;
  ಪೂರ್ಣಾಙ್ಕ j;
#ifdef SQLITE_ENABLE_COLUMN_METADATA
  ವರ್ಣ ಧ್ರುವ *zOrigDb = 0;
  ವರ್ಣ ಧ್ರುವ *zOrigTab = 0;
  ವರ್ಣ ಧ್ರುವ *zOrigCol = 0;
#endif

  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
  ಪ್ರತಿಜ್ಞಾ( pNC->pSrcList!=0 );
  ವಿಕಲ್ಪ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
    ಪಕ್ಷ TK_COLUMN: {
      /* The expression is a column. Locate the table the column is being
      ** extracted from in NameContext.pSrcList. This table may be real
      ** database table or a subquery.
      */
      Table *pTab = 0;            /* Table structure column is extracted from */
      Select *pS = 0;             /* Select the column is extracted from */
      ಪೂರ್ಣಾಙ್ಕ iCol = p_ಅಭಿವ್ಯಕ್ತಿ->iColumn;  /* Index of column in pTab */
      ಯಾವತ್( pNC && !pTab ){
        SrcList *pTabList = pNC->pSrcList;
        ಪ್ರತಿ(j=0;j<pTabList->nSrc && pTabList->a[j].iCursor!=p_ಅಭಿವ್ಯಕ್ತಿ->iTable;j++);
        ಯದಿ( j<pTabList->nSrc ){
          pTab = pTabList->a[j].pSTab;
          ಯದಿ( pTabList->a[j].fg.isSubquery ){
            pS = pTabList->a[j].u4.pSubq->pSelect;
          }ಅನ್ಯಥಾ{
            pS = 0;
          }
        }ಅನ್ಯಥಾ{
          pNC = pNC->pNext;
        }
      }

      ಯದಿ( pTab==0 ){
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
        ಭಙ್ಗ;
      }

      ಪ್ರತಿಜ್ಞಾ( pTab && ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) && p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab==pTab );
      ಯದಿ( pS ){
        /* The "table" is actually a sub-select or a view in the FROM clause
        ** of the SELECT statement. Return the declaration type and origin
        ** data for the result-set column of the sub-select.
        */
        ಯದಿ( iCol<pS->pEList->n_ಅಭಿವ್ಯಕ್ತಿ
         && (!ViewCanHaveRowid || iCol>=0)
        ){
          /* If iCol is less than zero, then the expression requests the
          ** rowid of the sub-select or view. This expression is legal (see
          ** test case misc2.2.2) - it always evaluates to NULL.
          */
          NameContext sNC;
          ಅಭಿವ್ಯಕ್ತಿ *p = pS->pEList->a[iCol].p_ಅಭಿವ್ಯಕ್ತಿ;
          sNC.pSrcList = pS->pSrc;
          sNC.pNext = pNC;
          sNC.pParse = pNC->pParse;
          zType = columnType(&sNC, p,&zOrigDb,&zOrigTab,&zOrigCol);
        }
      }ಅನ್ಯಥಾ{
        /* A real table or a CTE table */
        ಪ್ರತಿಜ್ಞಾ( !pS );
#ifdef SQLITE_ENABLE_COLUMN_METADATA
        ಯದಿ( iCol<0 ) iCol = pTab->iPKey;
        ಪ್ರತಿಜ್ಞಾ( iCol==XN_ROWID || (iCol>=0 && iCol<pTab->nCol) );
        ಯದಿ( iCol<0 ){
          zType = "INTEGER";
          zOrigCol = "rowid";
        }ಅನ್ಯಥಾ{
          zOrigCol = pTab->aCol[iCol].zCnName;
          zType = sqlite3ColumnType(&pTab->aCol[iCol],0);
        }
        zOrigTab = pTab->zName;
        ಯದಿ( pNC->pParse && pTab->pSchema ){
          ಪೂರ್ಣಾಙ್ಕ iDb = sqlite3SchemaToIndex(pNC->pParse->db, pTab->pSchema);
          zOrigDb = pNC->pParse->db->aDb[iDb].zDbSName;
        }
#ಅನ್ಯಥಾ
        ಪ್ರತಿಜ್ಞಾ( iCol==XN_ROWID || (iCol>=0 && iCol<pTab->nCol) );
        ಯದಿ( iCol<0 ){
          zType = "INTEGER";
        }ಅನ್ಯಥಾ{
          zType = sqlite3ColumnType(&pTab->aCol[iCol],0);
        }
#endif
      }
      ಭಙ್ಗ;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ಪಕ್ಷ TK_SELECT: {
      /* The expression is a sub-select. Return the declaration type and
      ** origin info for the single column in the result set of the SELECT
      ** statement.
      */
      NameContext sNC;
      Select *pS;
      ಅಭಿವ್ಯಕ್ತಿ *p;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
      pS = p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect;
      p = pS->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
      sNC.pSrcList = pS->pSrc;
      sNC.pNext = pNC;
      sNC.pParse = pNC->pParse;
      zType = columnType(&sNC, p, &zOrigDb, &zOrigTab, &zOrigCol);
      ಭಙ್ಗ;
    }
#endif
  }

#ifdef SQLITE_ENABLE_COLUMN_METADATA 
  ಯದಿ( pzOrigDb ){
    ಪ್ರತಿಜ್ಞಾ( pzOrigTab && pzOrigCol );
    *pzOrigDb = zOrigDb;
    *pzOrigTab = zOrigTab;
    *pzOrigCol = zOrigCol;
  }
#endif
  ಪ್ರತ್ಯಾಗಮ zType;
}

/*
** Generate code that will tell the VDBE the declaration types of columns
** in the result set.
*/
ಸ್ಥಿರ ಶೂನ್ಯ generateColumnTypes(
  Parse *pParse,      /* Parser context */
  SrcList *pTabList,  /* List of tables */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList    /* Expressions defining the result set */
){
#ifndef SQLITE_OMIT_DECLTYPE
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಙ್ಕ i;
  NameContext sNC;
  sNC.pSrcList = pTabList;
  sNC.pParse = pParse;
  sNC.pNext = 0;
  ಪ್ರತಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p = pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಧ್ರುವ ವರ್ಣ *zType;
#ifdef SQLITE_ENABLE_COLUMN_METADATA
    ಧ್ರುವ ವರ್ಣ *zOrigDb = 0;
    ಧ್ರುವ ವರ್ಣ *zOrigTab = 0;
    ಧ್ರುವ ವರ್ಣ *zOrigCol = 0;
    zType = columnType(&sNC, p, &zOrigDb, &zOrigTab, &zOrigCol);

    /* The vdbe must make its own copy of the column-type and other
    ** column specific strings, in case the schema is reset before this
    ** virtual machine is deleted.
    */
    sqlite3VdbeSetColName(v, i, COLNAME_DATABASE, zOrigDb, SQLITE_TRANSIENT);
    sqlite3VdbeSetColName(v, i, COLNAME_TABLE, zOrigTab, SQLITE_TRANSIENT);
    sqlite3VdbeSetColName(v, i, COLNAME_COLUMN, zOrigCol, SQLITE_TRANSIENT);
#ಅನ್ಯಥಾ
    zType = columnType(&sNC, p, 0, 0, 0);
#endif
    sqlite3VdbeSetColName(v, i, COLNAME_DECLTYPE, zType, SQLITE_TRANSIENT);
  }
#ಅನ್ಯಥಾ
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
ಶೂನ್ಯ sqlite3GenerateColumnNames(
  Parse *pParse,      /* Parser context */
  Select *pSelect     /* Generate column names for this SELECT statement */
){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಙ್ಕ i;
  Table *pTab;
  SrcList *pTabList;
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಙ್ಕ fullName;    /* TABLE.COLUMN if no AS clause and is a direct table ref */
  ಪೂರ್ಣಾಙ್ಕ srcName;     /* COLUMN or TABLE.COLUMN if no AS clause and is direct */

  ಯದಿ( pParse->colNamesSet ) ಪ್ರತ್ಯಾಗಮ;
  /* Column names are determined by the left-most term of a compound select */
  ಯಾವತ್( pSelect->pPrior ) pSelect = pSelect->pPrior;
  TREETRACE(0x80,pParse,pSelect,("generating column names\n"));
  pTabList = pSelect->pSrc;
  pEList = pSelect->pEList;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಪ್ರತಿಜ್ಞಾ( pTabList!=0 );
  pParse->colNamesSet = 1;
  fullName = (db->flags & SQLITE_FullColNames)!=0;
  srcName = (db->flags & SQLITE_ShortColNames)!=0 || fullName;
  sqlite3_Vdbe_Set_ಅಂಕೆ_Cols(v, pEList->n_ಅಭಿವ್ಯಕ್ತಿ);
  ಪ್ರತಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p = pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;

    ಪ್ರತಿಜ್ಞಾ( p!=0 );
    ಪ್ರತಿಜ್ಞಾ( p->op!=TK_AGG_COLUMN );  /* Agg processing has not run yet */
    ಪ್ರತಿಜ್ಞಾ( p->op!=TK_COLUMN
        || (ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p) && p->y.pTab!=0) ); /* Covering idx not yet coded */
    ಯದಿ( pEList->a[i].zEName && pEList->a[i].fg.eEName==ENAME_NAME ){
      /* An AS clause always takes first priority */
      ವರ್ಣ *zName = pEList->a[i].zEName;
      sqlite3VdbeSetColName(v, i, COLNAME_NAME, zName, SQLITE_TRANSIENT);
    }ಅನ್ಯಥಾ ಯದಿ( srcName && p->op==TK_COLUMN ){
      ವರ್ಣ *zCol;
      ಪೂರ್ಣಾಙ್ಕ iCol = p->iColumn;
      pTab = p->y.pTab;
      ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
      ಯದಿ( iCol<0 ) iCol = pTab->iPKey;
      ಪ್ರತಿಜ್ಞಾ( iCol==-1 || (iCol>=0 && iCol<pTab->nCol) );
      ಯದಿ( iCol<0 ){
        zCol = "rowid";
      }ಅನ್ಯಥಾ{
        zCol = pTab->aCol[iCol].zCnName;
      }
      ಯದಿ( fullName ){
        ವರ್ಣ *zName = 0;
        zName = sqlite3MPrintf(db, "%s.%s", pTab->zName, zCol);
        sqlite3VdbeSetColName(v, i, COLNAME_NAME, zName, SQLITE_DYNAMIC);
      }ಅನ್ಯಥಾ{
        sqlite3VdbeSetColName(v, i, COLNAME_NAME, zCol, SQLITE_TRANSIENT);
      }
    }ಅನ್ಯಥಾ{
      ಧ್ರುವ ವರ್ಣ *z = pEList->a[i].zEName;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_Columns_From_ಅಭಿವ್ಯಕ್ತಿ_List(
  Parse *pParse,          /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,       /* Expr list from which to derive column names */
  i16 *pnCol,             /* Write the number of columns here */
  Column **paCol          /* Write the new column list here */
){
  sqlite3 *db = pParse->db;   /* Database connection */
  ಪೂರ್ಣಾಙ್ಕ i, j;                   /* Loop counters */
  u32 cnt;                    /* Index added to make the name unique */
  Column *aCol, *pCol;        /* For looping over result columns */
  ಪೂರ್ಣಾಙ್ಕ nCol;                   /* Number of columns in the result set */
  ವರ್ಣ *zName;                /* Column name */
  ಪೂರ್ಣಾಙ್ಕ nName;                  /* Size of name in zName[] */
  Hash ht;                    /* Hash table of column names */
  Table *pTab;

  sqlite3HashInit(&ht);
  ಯದಿ( pEList ){
    nCol = pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
    aCol = sqlite3DbMallocZero(db, ಪರಿಮಾಣ(aCol[0])*nCol);
    testcase( aCol==0 );
    ಯದಿ( nCol>32767 ) nCol = 32767;
  }ಅನ್ಯಥಾ{
    nCol = 0;
    aCol = 0;
  }
  ಪ್ರತಿಜ್ಞಾ( nCol==(i16)nCol );
  *pnCol = nCol;
  *paCol = aCol;

  ಪ್ರತಿ(i=0, pCol=aCol; i<nCol && !pParse->nErr; i++, pCol++){
    ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pX = &pEList->a[i];
    ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pCollide;
    /* Get an appropriate name for the column
    */
    ಯದಿ( (zName = pX->zEName)!=0 && pX->fg.eEName==ENAME_NAME ){
      /* If the column contains an "AS <name>" phrase, use <name> as the name */
    }ಅನ್ಯಥಾ{
      ಅಭಿವ್ಯಕ್ತಿ *p_Col_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(pX->p_ಅಭಿವ್ಯಕ್ತಿ);
      ಯಾವತ್( ALWAYS(p_Col_ಅಭಿವ್ಯಕ್ತಿ!=0) && p_Col_ಅಭಿವ್ಯಕ್ತಿ->op==TK_DOT ){
        p_Col_ಅಭಿವ್ಯಕ್ತಿ = p_Col_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        ಪ್ರತಿಜ್ಞಾ( p_Col_ಅಭಿವ್ಯಕ್ತಿ!=0 );
      }
      ಯದಿ( p_Col_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN
       && ALWAYS( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_Col_ಅಭಿವ್ಯಕ್ತಿ) )
       && ALWAYS( p_Col_ಅಭಿವ್ಯಕ್ತಿ->y.pTab!=0 )
      ){
        /* For columns use the column name name */
        ಪೂರ್ಣಾಙ್ಕ iCol = p_Col_ಅಭಿವ್ಯಕ್ತಿ->iColumn;
        pTab = p_Col_ಅಭಿವ್ಯಕ್ತಿ->y.pTab;
        ಯದಿ( iCol<0 ) iCol = pTab->iPKey;
        zName = iCol>=0 ? pTab->aCol[iCol].zCnName : "rowid";
      }ಅನ್ಯಥಾ ಯದಿ( p_Col_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ID ){
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_Col_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
        zName = p_Col_ಅಭಿವ್ಯಕ್ತಿ->u.zToken;
      }ಅನ್ಯಥಾ{
        /* Use the original text of the column expression as its name */
        ಪ್ರತಿಜ್ಞಾ( zName==pX->zEName );  /* pointer comparison intended */
      }
    }
    ಯದಿ( zName && !sqlite3IsTrueOrFalse(zName) ){
      zName = sqlite3DbStrDup(db, zName);
    }ಅನ್ಯಥಾ{
      zName = sqlite3MPrintf(db,"column%d",i+1);
    }

    /* Make sure the column name is unique.  If the name is not unique,
    ** append an integer to the name so that it becomes unique.
    */
    cnt = 0;
    ಯಾವತ್( zName && (pCollide = sqlite3HashFind(&ht, zName))!=0 ){
      ಯದಿ( pCollide->fg.bUsingTerm ){
        pCol->colFlags |= COLFLAG_NOEXPAND;
      }
      nName = sqlite3Strlen30(zName);
      ಯದಿ( nName>0 ){
        ಪ್ರತಿ(j=nName-1; j>0 && sqlite3Isdigit(zName[j]); j--){}
        ಯದಿ( zName[j]==':' ) nName = j;
      }
      zName = sqlite3MPrintf(db, "%.*z:%u", nName, zName, ++cnt);
      sqlite3ProgressCheck(pParse);
      ಯದಿ( cnt>3 ){
        sqlite3_randomness(ಪರಿಮಾಣ(cnt), &cnt);
      }
    }
    pCol->zCnName = zName;
    pCol->hName = sqlite3StrIHash(zName);
    ಯದಿ( pX->fg.bNoExpand ){
      pCol->colFlags |= COLFLAG_NOEXPAND;
    }
    sqlite3ColumnPropertiesFromName(0, pCol);
    ಯದಿ( zName && sqlite3_Hash_ಸೇರಿಸು(&ht, zName, pX)==pX ){
      sqlite3OomFault(db);
    }
  }
  sqlite3HashClear(&ht);
  ಯದಿ( pParse->nErr ){
    ಪ್ರತಿ(j=0; j<i; j++){
      sqlite3DbFree(db, aCol[j].zCnName);
    }
    sqlite3DbFree(db, aCol);
    *paCol = 0;
    *pnCol = 0;
    ಪ್ರತ್ಯಾಗಮ pParse->rc;
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಶೂನ್ಯ sqlite3SubqueryColumnTypes(
  Parse *pParse,      /* Parsing contexts */
  Table *pTab,        /* Add column type information to this table */
  Select *pSelect,    /* SELECT used to determine types and collations */
  ವರ್ಣ aff            /* Default affinity. */
){
  sqlite3 *db = pParse->db;
  Column *pCol;
  CollSeq *pColl;
  ಪೂರ್ಣಾಙ್ಕ i,j;
  ಅಭಿವ್ಯಕ್ತಿ *p;
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *a;
  NameContext sNC;

  ಪ್ರತಿಜ್ಞಾ( pSelect!=0 );
  ಪ್ರತಿಜ್ಞಾ( (pSelect->selFlags & SF_Resolved)!=0 );
  ಪ್ರತಿಜ್ಞಾ( pTab->nCol==pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ || pParse->nErr>0 );
  ಪ್ರತಿಜ್ಞಾ( aff==SQLITE_AFF_NONE || aff==SQLITE_AFF_BLOB );
  ಯದಿ( db->mallocFailed || IN_RENAME_OBJECT ) ಪ್ರತ್ಯಾಗಮ;
  ಯಾವತ್( pSelect->pPrior ) pSelect = pSelect->pPrior;
  a = pSelect->pEList->a;
  memset(&sNC, 0, ಪರಿಮಾಣ(sNC));
  sNC.pSrcList = pSelect->pSrc;
  ಪ್ರತಿ(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
    ಧ್ರುವ ವರ್ಣ *zType;
    i64 n;
    ಪೂರ್ಣಾಙ್ಕ m = 0;
    Select *pS2 = pSelect;
    pTab->tabFlags |= (pCol->colFlags & COLFLAG_NOINSERT);
    p = a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    /* pCol->szEst = ... // Column size est for SELECT tables never used */
    pCol->affinity = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p);
    ಯಾವತ್( pCol->affinity<=SQLITE_AFF_NONE && pS2->pNext!=0 ){
      m |= sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Data_Type(pS2->pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ);
      pS2 = pS2->pNext;
      pCol->affinity = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pS2->pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ);
    }
    ಯದಿ( pCol->affinity<=SQLITE_AFF_NONE ){
      pCol->affinity = aff;
    }
    ಯದಿ( pCol->affinity>=SQLITE_AFF_TEXT && (pS2->pNext || pS2!=pSelect) ){
      ಪ್ರತಿ(pS2=pS2->pNext; pS2; pS2=pS2->pNext){
        m |= sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Data_Type(pS2->pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ);
      }
      ಯದಿ( pCol->affinity==SQLITE_AFF_TEXT && (m&0x01)!=0 ){
        pCol->affinity = SQLITE_AFF_BLOB;
      }ಅನ್ಯಥಾ
      ಯದಿ( pCol->affinity>=SQLITE_AFF_NUMERIC && (m&0x02)!=0 ){
        pCol->affinity = SQLITE_AFF_BLOB;
      }
      ಯದಿ( pCol->affinity>=SQLITE_AFF_NUMERIC && p->op==TK_CAST ){
        pCol->affinity = SQLITE_AFF_FLEXNUM;
      }
    }
    zType = columnType(&sNC, p, 0, 0, 0);
    ಯದಿ( zType==0 || pCol->affinity!=sqlite3AffinityType(zType, 0) ){
      ಯದಿ( pCol->affinity==SQLITE_AFF_NUMERIC
       || pCol->affinity==SQLITE_AFF_FLEXNUM
      ){
        zType = "NUM";
      }ಅನ್ಯಥಾ{
        zType = 0;
        ಪ್ರತಿ(j=1; j<SQLITE_N_STDTYPE; j++){
          ಯದಿ( sqlite3StdTypeAffinity[j]==pCol->affinity ){
            zType = sqlite3StdType[j];
            ಭಙ್ಗ;
          }
        }
      }
    }
    ಯದಿ( zType ){
      ಧ್ರುವ i64 k = strlen(zType);
      n = strlen(pCol->zCnName);
      pCol->zCnName = sqlite3DbReallocOrFree(db, pCol->zCnName, n+k+2);
      pCol->colFlags &= ~(COLFLAG_HASTYPE|COLFLAG_HASCOLL);
      ಯದಿ( pCol->zCnName ){
        memcpy(&pCol->zCnName[n+1], zType, k+1);
        pCol->colFlags |= COLFLAG_HASTYPE;
      }
    }
    pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p);
    ಯದಿ( pColl ){
      ಪ್ರತಿಜ್ಞಾ( pTab->pIndex==0 );
      sqlite3ColumnSetColl(db, pCol, pColl->zName);
    }
  }
  pTab->szTabRow = 1; /* Any non-zero value works */
}

/*
** Check the current subquery nesting depth.  Return true and set an
** error if it has gone too deep.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ checkSubqueryNestingDepth(Parse *pParse){
#ಯದಿ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_DEPTH>0
  ಯದಿ( pParse->nNestSel >= pParse->db->aLimit[SQLITE_LIMIT_ಅಭಿವ್ಯಕ್ತಿ_DEPTH] ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "VIEWs and/or subqueries nested too deep");
    ಪ್ರತ್ಯಾಗಮ 1;
  }
#endif
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Given a SELECT statement, generate a Table structure that describes
** the result set of that SELECT.
*/
Table *sqlite3ResultSetOfSelect(Parse *pParse, Select *pSelect, ವರ್ಣ aff){
  Table *pTab;
  sqlite3 *db = pParse->db;
  u64 savedFlags;

  pParse->nNestSel++;
  ಯದಿ( checkSubqueryNestingDepth(pParse) ) ಪ್ರತ್ಯಾಗಮ 0;
  savedFlags = db->flags;
  db->flags &= ~(u64)SQLITE_FullColNames;
  db->flags |= SQLITE_ShortColNames;
  sqlite3SelectPrep(pParse, pSelect, 0);
  db->flags = savedFlags;
  ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯಾವತ್( pSelect->pPrior ) pSelect = pSelect->pPrior;
  pTab = sqlite3DbMallocZero(db, ಪರಿಮಾಣ(Table) );
  ಯದಿ( pTab==0 ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  pTab->nTabRef = 1;
  pTab->zName = 0;
  pTab->nRowLogEst = 200; ಪ್ರತಿಜ್ಞಾ( 200==sqlite3LogEst(1048576) );
  sqlite3_Columns_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pSelect->pEList, &pTab->nCol, &pTab->aCol);
  sqlite3SubqueryColumnTypes(pParse, pTab, pSelect, aff);
  pTab->iPKey = -1;
  ಯದಿ( db->mallocFailed ){
    sqlite3DeleteTable(db, pTab);
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  pParse->nNestSel--;
  ಪ್ರತಿಜ್ಞಾ( pParse->nNestSel>=0 );
  ಪ್ರತ್ಯಾಗಮ pTab;
}

/*
** Get a VDBE for the given parser context.  Create a new one if necessary.
** If an error occurs, return NULL and leave a message in pParse.
*/
Vdbe *sqlite3GetVdbe(Parse *pParse){
  ಯದಿ( pParse->pVdbe ){
    ಪ್ರತ್ಯಾಗಮ pParse->pVdbe;
  }
  ಯದಿ( pParse->pToplevel==0 ){
    pParse->okConstFactor = 1;
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3VdbeCreate(pParse);
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
ಸ್ಥಿರ ಶೂನ್ಯ computeLimitRegisters(Parse *pParse, Select *p, ಪೂರ್ಣಾಙ್ಕ iBreak){
  Vdbe *v = 0;
  ಪೂರ್ಣಾಙ್ಕ iLimit = 0;
  ಪೂರ್ಣಾಙ್ಕ iOffset;
  ಪೂರ್ಣಾಙ್ಕ n;
  ಅಭಿವ್ಯಕ್ತಿ *pLimit = p->pLimit;

  ಯದಿ( p->iLimit ) ಪ್ರತ್ಯಾಗಮ;

  /*
  ** "LIMIT -1" always shows all rows.  There is some
  ** controversy about what the correct behavior should be.
  ** The current implementation interprets "LIMIT 0" to mean
  ** no rows.
  */
  ಯದಿ( pLimit ){
    ಪ್ರತಿಜ್ಞಾ( pLimit->op==TK_LIMIT );
    ಪ್ರತಿಜ್ಞಾ( pLimit->p_ಎಡ!=0 );
    p->iLimit = iLimit = ++pParse->nMem;
    v = sqlite3GetVdbe(pParse);
    ಪ್ರತಿಜ್ಞಾ( v!=0 );
    ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(pLimit->p_ಎಡ, &n, pParse, 0) ){
      sqlite3VdbeAddOp2(v, OP_Integer, n, iLimit);
      VdbeComment((v, "LIMIT counter"));
      ಯದಿ( n==0 ){
        sqlite3VdbeGoto(v, iBreak);
      }ಅನ್ಯಥಾ ಯದಿ( n>=0 && p->nSelectRow>sqlite3LogEst((u64)n) ){
        p->nSelectRow = sqlite3LogEst((u64)n);
        p->selFlags |= SF_FixedLimit;
      }
    }ಅನ್ಯಥಾ{
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, pLimit->p_ಎಡ, iLimit);
      sqlite3VdbeAddOp1(v, OP_MustBeInt, iLimit); VdbeCoverage(v);
      VdbeComment((v, "LIMIT counter"));
      sqlite3MayAbort(pParse);
      sqlite3VdbeAddOp2(v, OP_IfNot, iLimit, iBreak); VdbeCoverage(v);
    }
    ಯದಿ( pLimit->p_ಬಲ ){
      p->iOffset = iOffset = ++pParse->nMem;
      pParse->nMem++;   /* Allocate an extra register for limit+offset */
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, pLimit->p_ಬಲ, iOffset);
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
ಸ್ಥಿರ CollSeq *multiSelectCollSeq(Parse *pParse, Select *p, ಪೂರ್ಣಾಙ್ಕ iCol){
  CollSeq *pRet;
  ಯದಿ( p->pPrior ){
    pRet = multiSelectCollSeq(pParse, p->pPrior, iCol);
  }ಅನ್ಯಥಾ{
    pRet = 0;
  }
  ಪ್ರತಿಜ್ಞಾ( iCol>=0 );
  /* iCol must be less than p->pEList->nExpr.  Otherwise an error would
  ** have been thrown during name resolution and we would not have gotten
  ** this far */
  ಯದಿ( pRet==0 && ALWAYS(iCol<p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ) ){
    pRet = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p->pEList->a[iCol].p_ಅಭಿವ್ಯಕ್ತಿ);
  }
  ಪ್ರತ್ಯಾಗಮ pRet;
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
ಸ್ಥಿರ ಕೀಲಿ_Info *multi_Select_By_Merge_ಕೀಲಿ_Info(Parse *pParse, Select *p, ಪೂರ್ಣಾಙ್ಕ nExtra){
  ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy = p->pOrderBy;
  ಪೂರ್ಣಾಙ್ಕ nOrderBy = (pOrderBy!=0) ? pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
  sqlite3 *db = pParse->db;
  ಕೀಲಿ_Info *pRet = sqlite3_ಕೀಲಿ_Info_Alloc(db, nOrderBy+nExtra, 1);
  ಯದಿ( pRet ){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<nOrderBy; i++){
      ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem = &pOrderBy->a[i];
      ಅಭಿವ್ಯಕ್ತಿ *pTerm = pItem->p_ಅಭಿವ್ಯಕ್ತಿ;
      CollSeq *pColl;

      ಯದಿ( pTerm->flags & EP_Collate ){
        pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, pTerm);
      }ಅನ್ಯಥಾ{
        pColl = multiSelectCollSeq(pParse, p, pItem->u.x.iOrderByCol-1);
        ಯದಿ( pColl==0 ) pColl = db->pDfltColl;
        pOrderBy->a[i].p_ಅಭಿವ್ಯಕ್ತಿ =
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Add_Collate_String(pParse, pTerm, pColl->zName);
      }
      ಪ್ರತಿಜ್ಞಾ( sqlite3_ಕೀಲಿ_Info_Is_Writeable(pRet) );
      pRet->aColl[i] = pColl;
      pRet->aSortFlags[i] = pOrderBy->a[i].fg.sortFlags;
    }
  }

  ಪ್ರತ್ಯಾಗಮ pRet;
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
ಸ್ಥಿರ ಶೂನ್ಯ generateWithRecursiveQuery(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The recursive SELECT to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  SrcList *pSrc = p->pSrc;      /* The FROM clause of the recursive query */
  ಪೂರ್ಣಾಙ್ಕ nCol = p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;  /* Number of columns in the recursive table */
  Vdbe *v = pParse->pVdbe;      /* The prepared statement under construction */
  Select *pSetup;               /* The setup query */
  Select *pFirstRec;            /* Left-most recursive term */
  ಪೂರ್ಣಾಙ್ಕ addr_ತುದಿ;                  /* Top of the loop */
  ಪೂರ್ಣಾಙ್ಕ addrCont, addrBreak;      /* CONTINUE and BREAK addresses */
  ಪೂರ್ಣಾಙ್ಕ iCurrent = 0;             /* The Current table */
  ಪೂರ್ಣಾಙ್ಕ regCurrent;               /* Register holding Current table */
  ಪೂರ್ಣಾಙ್ಕ iQueue;                   /* The Queue table */
  ಪೂರ್ಣಾಙ್ಕ iDistinct = 0;            /* To ensure unique results if UNION */
  ಪೂರ್ಣಾಙ್ಕ eDest = SRT_Fifo;         /* How to write to Queue */
  SelectDest destQueue;         /* SelectDest targeting the Queue table */
  ಪೂರ್ಣಾಙ್ಕ i;                        /* Loop counter */
  ಪೂರ್ಣಾಙ್ಕ rc;                       /* Result code */
  ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy;           /* The ORDER BY clause */
  ಅಭಿವ್ಯಕ್ತಿ *pLimit;                 /* Saved LIMIT and OFFSET */
  ಪೂರ್ಣಾಙ್ಕ regLimit, regOffset;      /* Registers used by LIMIT and OFFSET */

#ifndef SQLITE_OMIT_WINDOWFUNC
  ಯದಿ( p->pWin ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot use window functions in recursive queries");
    ಪ್ರತ್ಯಾಗಮ;
  }
#endif

  /* Obtain authorization to do a recursive query */
  ಯದಿ( sqlite3AuthCheck(pParse, SQLITE_RECURSIVE, 0, 0, 0) ) ಪ್ರತ್ಯಾಗಮ;

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
  ಪ್ರತಿ(i=0; ALWAYS(i<pSrc->nSrc); i++){
    ಯದಿ( pSrc->a[i].fg.isRecursive ){
      iCurrent = pSrc->a[i].iCursor;
      ಭಙ್ಗ;
    }
  }

  /* Allocate cursors numbers for Queue and Distinct.  The cursor number for
  ** the Distinct table must be exactly one greater than Queue in order
  ** for the SRT_DistFifo and SRT_DistQueue destinations to work. */
  iQueue = pParse->nTab++;
  ಯದಿ( p->op==TK_UNION ){
    eDest = pOrderBy ? SRT_DistQueue : SRT_DistFifo;
    iDistinct = pParse->nTab++;
  }ಅನ್ಯಥಾ{
    eDest = pOrderBy ? SRT_Queue : SRT_Fifo;
  }
  sqlite3SelectDestInit(&destQueue, eDest, iQueue);

  /* Allocate cursors for Current, Queue, and Distinct. */
  regCurrent = ++pParse->nMem;
  sqlite3VdbeAddOp3(v, OP_OpenPseudo, iCurrent, regCurrent, nCol);
  ಯದಿ( pOrderBy ){
    ಕೀಲಿ_Info *p_ಕೀಲಿ_Info = multi_Select_By_Merge_ಕೀಲಿ_Info(pParse, p, 1);
    sqlite3VdbeAddOp4(v, OP_OpenEphemeral, iQueue, pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ+2, 0,
                      (ವರ್ಣ*)p_ಕೀಲಿ_Info, P4_KEYINFO);
    destQueue.pOrderBy = pOrderBy;
  }ಅನ್ಯಥಾ{
    sqlite3VdbeAddOp2(v, OP_OpenEphemeral, iQueue, nCol);
  }
  VdbeComment((v, "Queue table"));
  ಯದಿ( iDistinct ){
    /* Generate an ephemeral table used to enforce distinctness on the
    ** output of the recursive part of the CTE.
    */
    ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;            /* Collating sequence for the result set */
    CollSeq **apColl;             /* For looping through pKeyInfo->aColl[] */

    ಪ್ರತಿಜ್ಞಾ( p->pNext==0 );
    ಪ್ರತಿಜ್ಞಾ( p->pEList!=0 );
    nCol = p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
    p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_Alloc(pParse->db, nCol, 1);
    ಯದಿ( p_ಕೀಲಿ_Info ){
      ಪ್ರತಿ(i=0, apColl=p_ಕೀಲಿ_Info->aColl; i<nCol; i++, apColl++){
        *apColl = multiSelectCollSeq(pParse, p, i);
        ಯದಿ( 0==*apColl ){
          *apColl = pParse->db->pDfltColl;
        }
      }
      sqlite3VdbeAddOp4(v, OP_OpenEphemeral, iDistinct, nCol, 0,
                           (ಶೂನ್ಯ*)p_ಕೀಲಿ_Info, P4_KEYINFO);
    }ಅನ್ಯಥಾ{
      ಪ್ರತಿಜ್ಞಾ( pParse->nErr>0 );
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
  ಪ್ರತಿ(pFirstRec=p; ALWAYS(pFirstRec!=0); pFirstRec=pFirstRec->pPrior){
    ಯದಿ( pFirstRec->selFlags & SF_Aggregate ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "recursive aggregate queries not supported");
      ಗಚ್ಛ end_of_recursive_query;
    }
    pFirstRec->op = TK_ALL;
    ಯದಿ( (pFirstRec->pPrior->selFlags & SF_Recursive)==0 ) ಭಙ್ಗ;
  }

  /* Store the results of the setup-query in Queue. */
  pSetup = pFirstRec->pPrior;
  pSetup->pNext = 0;
  ExplainQueryPlan((pParse, 1, "SETUP"));
  rc = sqlite3Select(pParse, pSetup, &destQueue);
  pSetup->pNext = p;
  ಯದಿ( rc ) ಗಚ್ಛ end_of_recursive_query;

  /* Find the next row in the Queue and output that row */
  addr_ತುದಿ = sqlite3VdbeAddOp2(v, OP_Rewind, iQueue, addrBreak); VdbeCoverage(v);

  /* Transfer the next row in Queue over to Current */
  sqlite3VdbeAddOp1(v, OP_NullRow, iCurrent); /* To reset column cache */
  ಯದಿ( pOrderBy ){
    sqlite3VdbeAddOp3(v, OP_Column, iQueue, pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ+1, regCurrent);
  }ಅನ್ಯಥಾ{
    sqlite3VdbeAddOp2(v, OP_RowData, iQueue, regCurrent);
  }
  sqlite3VdbeAddOp1(v, OP_Delete, iQueue);

  /* Output the single row in Current */
  addrCont = sqlite3VdbeMakeLabel(pParse);
  codeOffset(v, regOffset, addrCont);
  selectInnerLoop(pParse, p, iCurrent,
      0, 0, pDest, addrCont, addrBreak);
  ಯದಿ( regLimit ){
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
  ಪ್ರತಿಜ್ಞಾ( pFirstRec->pPrior==0 );
  pFirstRec->pPrior = pSetup;

  /* Keep running the loop until the Queue is empty */
  sqlite3VdbeGoto(v, addr_ತುದಿ);
  sqlite3VdbeResolveLabel(v, addrBreak);

end_of_recursive_query:
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(pParse->db, p->pOrderBy);
  p->pOrderBy = pOrderBy;
  p->pLimit = pLimit;
  ಪ್ರತ್ಯಾಗಮ;
}
#endif /* SQLITE_OMIT_CTE */

/* Forward references */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiSelectByMerge(
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multi_Select_ಮೌಲ್ಯಗಳು(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  ಪೂರ್ಣಾಙ್ಕ nRow = 1;
  ಪೂರ್ಣಾಙ್ಕ rc = 0;
  ಪೂರ್ಣಾಙ್ಕ bShowAll = p->pLimit==0;
  ಪ್ರತಿಜ್ಞಾ( p->selFlags & SF_Multi_ಮೌಲ್ಯ );
  ಕುರು{
    ಪ್ರತಿಜ್ಞಾ( p->selFlags & SF_ಮೌಲ್ಯಗಳು );
    ಪ್ರತಿಜ್ಞಾ( p->op==TK_ALL || (p->op==TK_SELECT && p->pPrior==0) );
    ಪ್ರತಿಜ್ಞಾ( p->pNext==0 || p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ==p->pNext->pEList->n_ಅಭಿವ್ಯಕ್ತಿ );
#ifndef SQLITE_OMIT_WINDOWFUNC
    ಯದಿ( p->pWin ) ಪ್ರತ್ಯಾಗಮ -1;
#endif
    ಯದಿ( p->pPrior==0 ) ಭಙ್ಗ;
    ಪ್ರತಿಜ್ಞಾ( p->pPrior->pNext==p );
    p = p->pPrior;
    nRow += bShowAll;
  }ಯಾವತ್(1);
  ExplainQueryPlan((pParse, 0, "SCAN %d CONSTANT ROW%s", nRow,
                    nRow==1 ? "" : "S"));
  ಯಾವತ್( p ){
    selectInnerLoop(pParse, p, -1, 0, 0, pDest, 1, 1);
    ಯದಿ( !bShowAll ) ಭಙ್ಗ;
    p->nSelectRow = nRow;
    p = p->pNext;
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Return true if the SELECT statement which is known to be the recursive
** part of a recursive CTE still has its anchor terms attached.  If the
** anchor terms have already been removed, then return false.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ hasAnchor(Select *p){
  ಯಾವತ್( p && (p->selFlags & SF_Recursive)!=0 ){ p = p->pPrior; }
  ಪ್ರತ್ಯಾಗಮ p!=0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ unionWithLimitOne(sqlite3 *db, Select *p){
  ಪೂರ್ಣಾಙ್ಕ v;
  ಯದಿ( p->op!=TK_UNION ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( p->pLimit==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( p->pLimit->p_ಬಲ ) ಪ್ರತ್ಯಾಗಮ 0;  /* No OFFSET */
  v = 0;
  ಪ್ರತಿಜ್ಞಾ( p->pLimit->p_ಎಡ!=0 );
  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(p->pLimit->p_ಎಡ, &v, 0, 0)==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( v!=1 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( OptimizationDisabled(db, SQLITE_UnionLimit) ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ 1;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiSelect(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;   /* Success code from a subroutine */
  Select *pPrior;       /* Another SELECT immediately to our left */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest dest;      /* Alternative data destination */
  Select *pDelete = 0;  /* Chain of simple selects to delete */
  sqlite3 *db;          /* Database connection */

  /* Make sure there is no ORDER BY or LIMIT clause on prior SELECTs.  Only
  ** the last (right-most) SELECT in the series may have an ORDER BY or LIMIT.
  */
  ಪ್ರತಿಜ್ಞಾ( p && p->pPrior );  /* Calling function guarantees this much */
  ಪ್ರತಿಜ್ಞಾ( (p->selFlags & SF_Recursive)==0 || p->op==TK_ALL || p->op==TK_UNION );
  ಪ್ರತಿಜ್ಞಾ( p->selFlags & SF_Compound );
  db = pParse->db;
  pPrior = p->pPrior;
  dest = *pDest;
  ಪ್ರತಿಜ್ಞಾ( pPrior->pOrderBy==0 );
  ಪ್ರತಿಜ್ಞಾ( pPrior->pLimit==0 );

  v = sqlite3GetVdbe(pParse);
  ಪ್ರತಿಜ್ಞಾ( v!=0 );  /* The VDBE already created by calling function */

  /* Create the destination temporary table if necessary
  */
  ಯದಿ( dest.eDest==SRT_EphemTab ){
    ಪ್ರತಿಜ್ಞಾ( p->pEList );
    sqlite3VdbeAddOp2(v, OP_OpenEphemeral, dest.iSDParm, p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ);
    dest.eDest = SRT_Table;
  }

  /* Special handling for a compound-select that originates as a VALUES clause.
  */
  ಯದಿ( p->selFlags & SF_Multi_ಮೌಲ್ಯ ){
    rc = multi_Select_ಮೌಲ್ಯಗಳು(pParse, p, &dest);
    ಯದಿ( rc>=0 ) ಗಚ್ಛ multi_select_end;
    rc = SQLITE_OK;
  }

  /* Make sure all SELECTs in the statement have the same number of elements
  ** in their result sets.
  */
  ಪ್ರತಿಜ್ಞಾ( p->pEList && pPrior->pEList );
  ಪ್ರತಿಜ್ಞಾ( p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ==pPrior->pEList->n_ಅಭಿವ್ಯಕ್ತಿ );

#ifndef SQLITE_OMIT_CTE
  ಯದಿ( (p->selFlags & SF_Recursive)!=0 && hasAnchor(p) ){
    generateWithRecursiveQuery(pParse, p, &dest);
  }ಅನ್ಯಥಾ
#endif
  ಯದಿ( p->pOrderBy ){
    /* If the compound has an ORDER BY clause, then always use the merge
    ** algorithm. */
    ಪ್ರತ್ಯಾಗಮ multiSelectByMerge(pParse, p, pDest);
  }ಅನ್ಯಥಾ ಯದಿ( p->op!=TK_ALL && !unionWithLimitOne(db,p) ){
    /* If the compound is EXCEPT, INTERSECT, or UNION (anything other than
    ** UNION ALL) then also always use the merge algorithm.  However, the
    ** multiSelectByMerge() routine requires that the compound have an
    ** ORDER BY clause, and it doesn't right now.  So invent one first. */
    ಅಭಿವ್ಯಕ್ತಿ *pOne = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(db, 1);
    p->pOrderBy = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, 0, pOne);
    ಯದಿ( pParse->nErr ) ಗಚ್ಛ multi_select_end;
    ಪ್ರತಿಜ್ಞಾ( p->pOrderBy!=0 );
    p->pOrderBy->a[0].u.x.iOrderByCol = 1;
    ಪ್ರತ್ಯಾಗಮ multiSelectByMerge(pParse, p, pDest);
  }ಅನ್ಯಥಾ{
    /* For a UNION ALL compound without ORDER BY, or for a UNION with a 
    ** LIMIT of exactly 1 and no OFFSET and no ORDER BY, simply run the left
    ** query, then run the right query */
    ಪೂರ್ಣಾಙ್ಕ addr = 0;
    ಪೂರ್ಣಾಙ್ಕ nLimit = 0;  /* Initialize to suppress harmless compiler warning */

#ifndef SQLITE_OMIT_EXPLAIN
    ಯದಿ( pPrior->pPrior==0 ){
      ExplainQueryPlan((pParse, 1, "COMPOUND QUERY"));
      ExplainQueryPlan((pParse, 1, "LEFT-MOST SUBQUERY"));
    }
#endif
    ಪ್ರತಿಜ್ಞಾ( !pPrior->pLimit );
    pPrior->iLimit = p->iLimit;
    pPrior->iOffset = p->iOffset;
    pPrior->pLimit = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->pLimit, 0);
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL left...\n"));
    rc = sqlite3Select(pParse, pPrior, &dest);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pPrior->pLimit);
    pPrior->pLimit = 0;
    ಯದಿ( rc ){
      ಗಚ್ಛ multi_select_end;
    }
    p->pPrior = 0;
    p->iLimit = pPrior->iLimit;
    p->iOffset = pPrior->iOffset;
    ಯದಿ( p->iLimit ){
      addr = sqlite3VdbeAddOp1(v, OP_IfNot, p->iLimit); VdbeCoverage(v);
      VdbeComment((v, "Jump ahead if LIMIT reached"));
      ಯದಿ( p->iOffset ){
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
    ಯದಿ( p->pLimit
     && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(p->pLimit->p_ಎಡ, &nLimit, pParse, 0)
     && nLimit>0 && p->nSelectRow > sqlite3LogEst((u64)nLimit)
    ){
      p->nSelectRow = sqlite3LogEst((u64)nLimit);
    }
    ಯದಿ( addr ){
      sqlite3VdbeJumpHere(v, addr);
    }
#ifndef SQLITE_OMIT_EXPLAIN
    ಯದಿ( p->pNext==0 ){
      Explain_Query_Plan_ತೆಗೆ(pParse);
    }
#endif
  }

multi_select_end:
  pDest->iSdst = dest.iSdst;
  pDest->nSdst = dest.nSdst;
  pDest->iSDParm2 = dest.iSDParm2;
  ಯದಿ( pDelete ){
    sqlite3ParserAddCleanup(pParse, sqlite3SelectDeleteGeneric, pDelete);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}
#endif /* SQLITE_OMIT_COMPOUND_SELECT */

/*
** Error message for when two or more terms of a compound select have different
** size result sets.
*/
ಶೂನ್ಯ sqlite3_Select_Wrong_ಅಂಕೆ_Terms_Error(Parse *pParse, Select *p){
  ಯದಿ( p->selFlags & SF_ಮೌಲ್ಯಗಳು ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "all VALUES must have the same number of terms");
  }ಅನ್ಯಥಾ{
    sqlite3_Error_ಸಂದೇಶ(pParse, "SELECTs to the left and right of %s"
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ generateOutputSubroutine(
  Parse *pParse,          /* Parsing context */
  Select *p,              /* The SELECT statement */
  SelectDest *pIn,        /* Coroutine supplying data */
  SelectDest *pDest,      /* Where to send the data */
  ಪೂರ್ಣಾಙ್ಕ regReturn,          /* The return address register */
  ಪೂರ್ಣಾಙ್ಕ regPrev,            /* Previous result register.  No uniqueness if 0 */
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info,      /* For comparing with previous entry */
  ಪೂರ್ಣಾಙ್ಕ iBreak              /* Jump here if we hit the LIMIT */
){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಙ್ಕ iContinue;
  ಪೂರ್ಣಾಙ್ಕ addr;

  ಪ್ರತಿಜ್ಞಾ( pIn->eDest==SRT_Coroutine );

  addr = sqlite3VdbeCurrentAddr(v);
  iContinue = sqlite3VdbeMakeLabel(pParse);

  /* Suppress duplicates for UNION, EXCEPT, and INTERSECT
  */
  ಯದಿ( regPrev ){
    ಪೂರ್ಣಾಙ್ಕ addr1, addr2;
    addr1 = sqlite3VdbeAddOp1(v, OP_IfNot, regPrev); VdbeCoverage(v);
    addr2 = sqlite3VdbeAddOp4(v, OP_Compare, pIn->iSdst, regPrev+1, pIn->nSdst,
                              (ವರ್ಣ*)sqlite3_ಕೀಲಿ_Info_Ref(p_ಕೀಲಿ_Info), P4_KEYINFO);
    sqlite3VdbeAddOp3(v, OP_Jump, addr2+2, iContinue, addr2+2); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr1);
    sqlite3VdbeAddOp3(v, OP_Copy, pIn->iSdst, regPrev+1, pIn->nSdst-1);
    sqlite3VdbeAddOp2(v, OP_Integer, 1, regPrev);
  }
  ಯದಿ( pParse->db->mallocFailed ) ಪ್ರತ್ಯಾಗಮ 0;

  /* Suppress the first OFFSET entries if there is an OFFSET clause
  */
  codeOffset(v, p->iOffset, iContinue);

  ವಿಕಲ್ಪ( pDest->eDest ){
    /* Store the result as data using a unique key.
    */
    ಪಕ್ಷ SRT_Fifo:
    ಪಕ್ಷ SRT_DistFifo:
    ಪಕ್ಷ SRT_Table:
    ಪಕ್ಷ SRT_EphemTab: {
      ಪೂರ್ಣಾಙ್ಕ r1 = sqlite3GetTempReg(pParse);
      ಪೂರ್ಣಾಙ್ಕ r2 = sqlite3GetTempReg(pParse);
      ಪೂರ್ಣಾಙ್ಕ iParm = pDest->iSDParm;
      testcase( pDest->eDest==SRT_Table );
      testcase( pDest->eDest==SRT_EphemTab );
      testcase( pDest->eDest==SRT_Fifo );
      testcase( pDest->eDest==SRT_DistFifo );
      sqlite3VdbeAddOp3(v, OP_MakeRecord, pIn->iSdst, pIn->nSdst, r1);
#ಯದಿ !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      ಯದಿ( pDest->eDest==SRT_Table && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#endif
#ifndef SQLITE_OMIT_CTE
      ಯದಿ( pDest->eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index that is used to enforce uniqueness on the
        ** total result.  At this point, we are processing the setup portion
        ** of the recursive CTE using the merge algorithm, so the results are
        ** guaranteed to be unique anyhow.  But we still need to populate the
        ** (iParm+1) cursor for use by the subsequent recursive phase.
        */
        sqlite3VdbeAddOp4Int(v, OP_Idx_ಸೇರಿಸು, iParm+1, r1,
                             pIn->iSdst, pIn->nSdst);
      }
#endif
      sqlite3VdbeAddOp2(v, OP_NewRowid, iParm, r2);
      sqlite3VdbeAddOp3(v, OP_ಸೇರಿಸು, iParm, r1, r2);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      sqlite3ReleaseTempReg(pParse, r2);
      sqlite3ReleaseTempReg(pParse, r1);
      ಭಙ್ಗ;
    }

    /* If any row exist in the result set, record that fact and abort.
    */
    ಪಕ್ಷ SRT_Exists: {
      sqlite3VdbeAddOp2(v, OP_Integer, 1, pDest->iSDParm);
      /* The LIMIT clause will terminate the loop for us */
      ಭಙ್ಗ;
    }

#ifndef SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)".
    */
    ಪಕ್ಷ SRT_Set: {
      ಪೂರ್ಣಾಙ್ಕ r1;
      testcase( pIn->nSdst>1 );
      r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp4(v, OP_MakeRecord, pIn->iSdst, pIn->nSdst,
          r1, pDest->zAffSdst, pIn->nSdst);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ಸೇರಿಸು, pDest->iSDParm, r1,
                           pIn->iSdst, pIn->nSdst);
      ಯದಿ( pDest->iSDParm2>0 ){
        ಪ್ರತಿಜ್ಞಾ( sqlite3CursorBloomable(pParse, pDest->iSDParm, pIn->nSdst) );
        sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pDest->iSDParm2, 0,
                             pIn->iSdst, pIn->nSdst);
        ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
      }
      sqlite3ReleaseTempReg(pParse, r1);
      ಭಙ್ಗ;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell and break out
    ** of the scan loop.  Note that the select might return multiple columns
    ** if it is the RHS of a row-value IN operator.
    */
    ಪಕ್ಷ SRT_Mem: {
      testcase( pIn->nSdst>1 );
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Move(pParse, pIn->iSdst, pDest->iSDParm, pIn->nSdst);
      /* The LIMIT clause will jump out of the loop for us */
      ಭಙ್ಗ;
    }
#endif /* #ifndef SQLITE_OMIT_SUBQUERY */

    /* The results are stored in a sequence of registers
    ** starting at pDest->iSdst.  Then the co-routine yields.
    */
    ಪಕ್ಷ SRT_Coroutine: {
      ಯದಿ( pDest->iSdst==0 ){
        pDest->iSdst = sqlite3GetTempRange(pParse, pIn->nSdst);
        pDest->nSdst = pIn->nSdst;
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Move(pParse, pIn->iSdst, pDest->iSdst, pIn->nSdst);
      sqlite3VdbeAddOp1(v, OP_Yield, pDest->iSDParm);
      ಭಙ್ಗ;
    }

#ifndef SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    ಪಕ್ಷ SRT_DistQueue:
    ಪಕ್ಷ SRT_Queue: {
      ಪೂರ್ಣಾಙ್ಕ n_ಕೀಲಿ;
      ಪೂರ್ಣಾಙ್ಕ r1, r2, r3, ii;
      ಅಭಿವ್ಯಕ್ತಿ_List *pSO;
      ಪೂರ್ಣಾಙ್ಕ iParm = pDest->iSDParm;
      pSO = pDest->pOrderBy;
      ಪ್ರತಿಜ್ಞಾ( pSO );
      n_ಕೀಲಿ = pSO->n_ಅಭಿವ್ಯಕ್ತಿ;
      r1 = sqlite3GetTempReg(pParse);
      r2 = sqlite3GetTempRange(pParse, n_ಕೀಲಿ+2);
      r3 = r2+n_ಕೀಲಿ+1;

      sqlite3VdbeAddOp3(v, OP_MakeRecord, pIn->iSdst, pIn->nSdst, r3);
      ಯದಿ( pDest->eDest==SRT_DistQueue ){
        sqlite3VdbeAddOp2(v, OP_Idx_ಸೇರಿಸು, iParm+1, r3);
      }
      ಪ್ರತಿ(ii=0; ii<n_ಕೀಲಿ; ii++){
        sqlite3VdbeAddOp2(v, OP_SCopy,
                          pIn->iSdst + pSO->a[ii].u.x.iOrderByCol - 1,
                          r2+ii);
      }
      sqlite3VdbeAddOp2(v, OP_Sequence, iParm, r2+n_ಕೀಲಿ);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, r2, n_ಕೀಲಿ+2, r1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ಸೇರಿಸು, iParm, r1, r2, n_ಕೀಲಿ+2);
      sqlite3ReleaseTempReg(pParse, r1);
      sqlite3ReleaseTempRange(pParse, r2, n_ಕೀಲಿ+2);
      ಭಙ್ಗ;
    }
#endif /* SQLITE_OMIT_CTE */

    /* Ignore the output */
    ಪಕ್ಷ SRT_Discard: {
      ಭಙ್ಗ;
    }

    /* If none of the above, then the result destination must be
    ** SRT_Output.
    **
    ** For SRT_Output, results are stored in a sequence of registers. 
    ** Then the OP_ResultRow opcode is used to cause sqlite3_step() to
    ** return the next row of result.
    */
    ಸ್ವಭಾವ: {
      ಪ್ರತಿಜ್ಞಾ( pDest->eDest==SRT_Output );
      sqlite3VdbeAddOp2(v, OP_ResultRow, pIn->iSdst, pIn->nSdst);
      ಭಙ್ಗ;
    }
  }

  /* Jump to the end of the loop if the LIMIT is reached.
  */
  ಯದಿ( p->iLimit ){
    sqlite3VdbeAddOp2(v, OP_DecrJumpZero, p->iLimit, iBreak); VdbeCoverage(v);
  }

  /* Generate the subroutine return
  */
  sqlite3VdbeResolveLabel(v, iContinue);
  sqlite3VdbeAddOp1(v, OP_Return, regReturn);

  ಪ್ರತ್ಯಾಗಮ addr;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ multiSelectByMerge(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  ಪೂರ್ಣಾಙ್ಕ i, j;             /* Loop counters */
  Select *pPrior;       /* Another SELECT immediately to our left */
  Select *pSplit;       /* Left-most SELECT in the right-hand group */
  ಪೂರ್ಣಾಙ್ಕ nSelect;          /* Number of SELECT statements in the compound */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest destA;     /* Destination for coroutine A */
  SelectDest destB;     /* Destination for coroutine B */
  ಪೂರ್ಣಾಙ್ಕ regAddrA;         /* Address register for select-A coroutine */
  ಪೂರ್ಣಾಙ್ಕ regAddrB;         /* Address register for select-B coroutine */
  ಪೂರ್ಣಾಙ್ಕ addrSelectA;      /* Address of the select-A coroutine */
  ಪೂರ್ಣಾಙ್ಕ addrSelectB;      /* Address of the select-B coroutine */
  ಪೂರ್ಣಾಙ್ಕ regOutA;          /* Address register for the output-A subroutine */
  ಪೂರ್ಣಾಙ್ಕ regOutB;          /* Address register for the output-B subroutine */
  ಪೂರ್ಣಾಙ್ಕ addrOutA;         /* Address of the output-A subroutine */
  ಪೂರ್ಣಾಙ್ಕ addrOutB = 0;     /* Address of the output-B subroutine */
  ಪೂರ್ಣಾಙ್ಕ addrEofA;         /* Address of the select-A-exhausted subroutine */
  ಪೂರ್ಣಾಙ್ಕ addrEofA_noB;     /* Alternate addrEofA if B is uninitialized */
  ಪೂರ್ಣಾಙ್ಕ addrEofB;         /* Address of the select-B-exhausted subroutine */
  ಪೂರ್ಣಾಙ್ಕ addrAltB;         /* Address of the A<B subroutine */
  ಪೂರ್ಣಾಙ್ಕ addrAeqB;         /* Address of the A==B subroutine */
  ಪೂರ್ಣಾಙ್ಕ addrAgtB;         /* Address of the A>B subroutine */
  ಪೂರ್ಣಾಙ್ಕ regLimitA;        /* Limit register for select-A */
  ಪೂರ್ಣಾಙ್ಕ regLimitB;        /* Limit register for select-A */
  ಪೂರ್ಣಾಙ್ಕ regPrev;          /* A range of registers to hold previous output */
  ಪೂರ್ಣಾಙ್ಕ savedLimit;       /* Saved value of p->iLimit */
  ಪೂರ್ಣಾಙ್ಕ savedOffset;      /* Saved value of p->iOffset */
  ಪೂರ್ಣಾಙ್ಕ labelCmpr;        /* Label for the start of the merge algorithm */
  ಪೂರ್ಣಾಙ್ಕ labelEnd;         /* Label for the end of the overall SELECT stmt */
  ಪೂರ್ಣಾಙ್ಕ addr1;            /* Jump instructions that get retargeted */
  ಪೂರ್ಣಾಙ್ಕ op;               /* One of TK_ALL, TK_UNION, TK_EXCEPT, TK_INTERSECT */
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Dup = 0; /* Comparison information for duplicate removal */
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Merge;   /* Comparison information for merging rows */
  sqlite3 *db;          /* Database connection */
  ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy;   /* The ORDER BY clause */
  ಪೂರ್ಣಾಙ್ಕ nOrderBy;         /* Number of terms in the ORDER BY clause */
  u32 *aPermute;        /* Mapping from ORDER BY terms to result set columns */

  ಪ್ರತಿಜ್ಞಾ( p->pOrderBy!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Dup==0 ); /* "Managed" code needs this.  Ticket #3382. */
  db = pParse->db;
  v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );       /* Already thrown the error if VDBE alloc failed */
  labelEnd = sqlite3VdbeMakeLabel(pParse);
  labelCmpr = sqlite3VdbeMakeLabel(pParse);


  /* Patch up the ORDER BY clause
  */
  op = p->op; 
  ಪ್ರತಿಜ್ಞಾ( p->pPrior->pOrderBy==0 );
  pOrderBy = p->pOrderBy;
  ಪ್ರತಿಜ್ಞಾ( pOrderBy );
  nOrderBy = pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ;

  /* For operators other than UNION ALL we have to make sure that
  ** the ORDER BY clause covers every term of the result set.  Add
  ** terms to the ORDER BY clause as necessary.  Do the same for UNION ALL
  ** if any operator to the left is not UNION ALL, so that the ORDER BY
  ** carries the collation of every column into that sub-compound.
  */
  ಪ್ರತಿ(pPrior=p; pPrior && pPrior->op==TK_ALL; pPrior=pPrior->pPrior){}
  ಯದಿ( pPrior && pPrior->pPrior ){
    ಪ್ರತಿ(i=1; db->mallocFailed==0 && i<=p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
      ಪ್ರತಿ(j=0, pItem=pOrderBy->a; j<nOrderBy; j++, pItem++){
        ಪ್ರತಿಜ್ಞಾ( pItem!=0 );
        ಪ್ರತಿಜ್ಞಾ( pItem->u.x.iOrderByCol>0 );
        ಯದಿ( pItem->u.x.iOrderByCol==i ) ಭಙ್ಗ;
      }
      ಯದಿ( j==nOrderBy ){
        ಅಭಿವ್ಯಕ್ತಿ *pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(db, i);
        ಯದಿ( pNew==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM_BKPT;
        p->pOrderBy = pOrderBy = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pOrderBy, pNew);
        ಯದಿ( pOrderBy ) pOrderBy->a[nOrderBy++].u.x.iOrderByCol = (u16)i;
      }
    }
  }

  /* Compute the comparison permutation and keyinfo that is used with
  ** the permutation to determine if the next row of results comes
  ** from selectA or selectB.  Also add literal collations to the
  ** ORDER BY clause terms so that when selectA and selectB are
  ** evaluated, they use the correct collation.
  */
  aPermute = sqlite3DbMallocRawNN(db, ಪರಿಮಾಣ(u32)*(nOrderBy + 1));
  ಯದಿ( aPermute ){
    ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
    ಪೂರ್ಣಾಙ್ಕ bKeep = 0;
    aPermute[0] = nOrderBy;
    ಪ್ರತಿ(i=1, pItem=pOrderBy->a; i<=nOrderBy; i++, pItem++){
      ಪ್ರತಿಜ್ಞಾ( pItem!=0 );
      ಪ್ರತಿಜ್ಞಾ( pItem->u.x.iOrderByCol>0 );
      ಪ್ರತಿಜ್ಞಾ( pItem->u.x.iOrderByCol<=p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ );
      aPermute[i] = pItem->u.x.iOrderByCol - 1;
      ಯದಿ( aPermute[i]!=(u32)i-1 ) bKeep = 1;
    }
     ಯದಿ( bKeep==0 ){
      sqlite3DbFreeNN(db, aPermute);
      aPermute = 0;
    }
  }
  p_ಕೀಲಿ_Merge = multi_Select_By_Merge_ಕೀಲಿ_Info(pParse, p, 1);

  /* Allocate a range of temporary registers and the KeyInfo needed
  ** for the logic that removes duplicate result rows when the
  ** operator is UNION, EXCEPT, or INTERSECT (but not UNION ALL).
  */
  ಯದಿ( op==TK_ALL ){
    regPrev = 0;
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ n_ಅಭಿವ್ಯಕ್ತಿ = p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
    ಪ್ರತಿಜ್ಞಾ( nOrderBy>=n_ಅಭಿವ್ಯಕ್ತಿ || db->mallocFailed );
    regPrev = pParse->nMem+1;
    pParse->nMem += n_ಅಭಿವ್ಯಕ್ತಿ+1;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, regPrev);
    p_ಕೀಲಿ_Dup = sqlite3_ಕೀಲಿ_Info_Alloc(db, n_ಅಭಿವ್ಯಕ್ತಿ, 1);
    ಯದಿ( p_ಕೀಲಿ_Dup ){
      ಪ್ರತಿಜ್ಞಾ( sqlite3_ಕೀಲಿ_Info_Is_Writeable(p_ಕೀಲಿ_Dup) );
      /* Use the ORDER BY collations, not multiSelectCollSeq(p), because
      ** p might be just one part of a larger compound, in which case
      ** the ORDER BY carries the COLLATE of the larger compound. */
      ಪ್ರತಿ(i=0; ALWAYS(p_ಕೀಲಿ_Merge) && i<nOrderBy; i++){
        p_ಕೀಲಿ_Dup->aColl[pOrderBy->a[i].u.x.iOrderByCol-1] = p_ಕೀಲಿ_Merge->aColl[i];
      }
    }
  }

  /* Separate the left and the right query from one another
  */
  nSelect = 1;
  ಯದಿ( (op==TK_ALL || op==TK_UNION)
   && OptimizationEnabled(db, SQLITE_BalancedMerge)
  ){
    ಪ್ರತಿ(pSplit=p; pSplit->pPrior!=0 && pSplit->op==op; pSplit=pSplit->pPrior){
      nSelect++;
      ಪ್ರತಿಜ್ಞಾ( pSplit->pPrior->pNext==pSplit );
    }
  }
  ಯದಿ( nSelect<=3 ){
    pSplit = p;
  }ಅನ್ಯಥಾ{
    pSplit = p;
    ಪ್ರತಿ(i=2; i<nSelect; i+=2){ pSplit = pSplit->pPrior; }
  }
  pPrior = pSplit->pPrior;
  ಪ್ರತಿಜ್ಞಾ( pPrior!=0 );
  pSplit->pPrior = 0;
  pPrior->pNext = 0;
  ಪ್ರತಿಜ್ಞಾ( p->pOrderBy == pOrderBy );
  ಪ್ರತಿಜ್ಞಾ( pOrderBy!=0 || db->mallocFailed );
  pPrior->pOrderBy = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(pParse->db, pOrderBy, 0);
  sqlite3ResolveOrderGroupBy(pParse, p, p->pOrderBy, "ORDER");
  sqlite3ResolveOrderGroupBy(pParse, pPrior, pPrior->pOrderBy, "ORDER");

  /* Compute the limit registers */
  computeLimitRegisters(pParse, p, labelEnd);
  ಯದಿ( p->iLimit && (op==TK_ALL || unionWithLimitOne(db,p)) ){
    regLimitA = ++pParse->nMem;
    regLimitB = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Copy, p->iOffset ? p->iOffset+1 : p->iLimit,
                                  regLimitA);
    sqlite3VdbeAddOp2(v, OP_Copy, regLimitA, regLimitB);
  }ಅನ್ಯಥಾ{
    regLimitA = regLimitB = 0;
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p->pLimit);
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
                 regPrev, p_ಕೀಲಿ_Dup, labelEnd);
 
  /* Generate a subroutine that outputs the current row of the B
  ** select as the next output row of the compound select.
  */
  ಯದಿ( op==TK_ALL || op==TK_UNION ){
    VdbeNoopComment((v, "SUBR: out-B"));
    addrOutB = generateOutputSubroutine(pParse,
                 p, &destB, pDest, regOutB,
                 regPrev, p_ಕೀಲಿ_Dup, labelEnd);
  }
  sqlite3_ಕೀಲಿ_Info_Unref(p_ಕೀಲಿ_Dup);

  /* Generate a subroutine to run when the results from select A
  ** are exhausted and only data in select B remains.
  */
  ಯದಿ( op==TK_EXCEPT || op==TK_INTERSECT ){
    addrEofA_noB = addrEofA = labelEnd;
  }ಅನ್ಯಥಾ{ 
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
  ಯದಿ( op==TK_INTERSECT ){
    addrEofB = addrEofA;
    ಯದಿ( p->nSelectRow > pPrior->nSelectRow ) p->nSelectRow = pPrior->nSelectRow;
  }ಅನ್ಯಥಾ{ 
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
  ಯದಿ( op==TK_ALL ){
    addrAeqB = addrAltB;
  }ಅನ್ಯಥಾ ಯದಿ( op==TK_INTERSECT ){
    addrAeqB = addrAltB;
    addrAltB++;
  }ಅನ್ಯಥಾ{
    addrAeqB = addrAltB + 1;
  }

  /* Generate code to handle the case of A>B
  */
  addrAgtB = sqlite3VdbeCurrentAddr(v);
  ಯದಿ( op==TK_ALL || op==TK_UNION ){
    sqlite3VdbeAddOp2(v, OP_Gosub, regOutB, addrOutB);
    VdbeComment((v, "out-B"));
    sqlite3VdbeAddOp2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, labelCmpr);
  }ಅನ್ಯಥಾ{
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
  ಯದಿ( aPermute!=0 ){
    sqlite3VdbeAddOp4(v, OP_Permutation, 0, 0, 0, (ವರ್ಣ*)aPermute, P4_INTARRAY);
  }
  sqlite3VdbeResolveLabel(v, labelCmpr);
  sqlite3VdbeAddOp4(v, OP_Compare, destA.iSdst, destB.iSdst, nOrderBy,
                         (ವರ್ಣ*)p_ಕೀಲಿ_Merge, P4_KEYINFO);
  ಯದಿ( aPermute!=0 ){
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
  ಯದಿ( pSplit->pPrior ){
    sqlite3ParserAddCleanup(pParse, sqlite3SelectDeleteGeneric, pSplit->pPrior);
  }
  pSplit->pPrior = pPrior;
  pPrior->pNext = pSplit;
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pPrior->pOrderBy);
  pPrior->pOrderBy = 0;

  /*** TBD:  Insert subroutine calls to close cursors on incomplete
  **** subqueries ****/
  Explain_Query_Plan_ತೆಗೆ(pParse);
  ಪ್ರತ್ಯಾಗಮ pParse->nErr!=0;
}
#endif

#ಯದಿ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)

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
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ SubstContext {
  Parse *pParse;            /* The parsing context */
  ಪೂರ್ಣಾಙ್ಕ iTable;               /* Replace references to this table */
  ಪೂರ್ಣಾಙ್ಕ iNewTable;            /* New table number */
  ಪೂರ್ಣಾಙ್ಕ isOuterJoin;          /* Add TK_IF_NULL_ROW opcodes on each replacement */
  ಪೂರ್ಣಾಙ್ಕ nSelDepth;            /* Depth of sub-query recursion.  Top==1 */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;         /* Replacement expressions */
  ಅಭಿವ್ಯಕ್ತಿ_List *pCList;         /* Collation sequences for replacement expr */
} SubstContext;

/* Forward Declarations */
ಸ್ಥಿರ ಶೂನ್ಯ subst_ಅಭಿವ್ಯಕ್ತಿ_List(SubstContext*, ಅಭಿವ್ಯಕ್ತಿ_List*);
ಸ್ಥಿರ ಶೂನ್ಯ substSelect(SubstContext*, Select*, ಪೂರ್ಣಾಙ್ಕ);

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
ಸ್ಥಿರ ಅಭಿವ್ಯಕ್ತಿ *subst_ಅಭಿವ್ಯಕ್ತಿ(
  SubstContext *pSubst,  /* Description of the substitution */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ            /* Expr in which substitution occurs */
){
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON|EP_InnerON)
   && p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin==pSubst->iTable
  ){
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON) );
    p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin = pSubst->iNewTable;
  }
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN
   && p_ಅಭಿವ್ಯಕ್ತಿ->iTable==pSubst->iTable
   && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FixedCol)
  ){
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iColumn<0 ){
      p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_NULL;
    }ಅನ್ಯಥಾ
#endif
    {
      ಅಭಿವ್ಯಕ್ತಿ *pNew;
      ಪೂರ್ಣಾಙ್ಕ iColumn;
      ಅಭಿವ್ಯಕ್ತಿ *pCopy;
      ಅಭಿವ್ಯಕ್ತಿ ifNullRow;
      iColumn = p_ಅಭಿವ್ಯಕ್ತಿ->iColumn;
      ಪ್ರತಿಜ್ಞಾ( iColumn>=0 );
      ಪ್ರತಿಜ್ಞಾ( pSubst->pEList!=0 && iColumn<pSubst->pEList->n_ಅಭಿವ್ಯಕ್ತಿ );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ==0 );
      pCopy = pSubst->pEList->a[iColumn].p_ಅಭಿವ್ಯಕ್ತಿ;
      ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Vector(pCopy) ){
        sqlite3_Vector_Error_ಸಂದೇಶ(pSubst->pParse, pCopy);
      }ಅನ್ಯಥಾ{
        sqlite3 *db = pSubst->pParse->db;
        ಯದಿ( pSubst->isOuterJoin
         && (pCopy->op!=TK_COLUMN || pCopy->iTable!=pSubst->iNewTable)
        ){
          memset(&ifNullRow, 0, ಪರಿಮಾಣ(ifNullRow));
          ifNullRow.op = TK_IF_NULL_ROW;
          ifNullRow.p_ಎಡ = pCopy;
          ifNullRow.iTable = pSubst->iNewTable;
          ifNullRow.iColumn = -99;
          ifNullRow.flags = EP_IfNullRow;
          pCopy = &ifNullRow;
        }
        testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pCopy, EP_Subquery) );
        pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pCopy, 0);
        ಯದಿ( db->mallocFailed ){
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pNew);
          ಪ್ರತ್ಯಾಗಮ p_ಅಭಿವ್ಯಕ್ತಿ;
        }
        ಯದಿ( pSubst->isOuterJoin ){
          ಅಭಿವ್ಯಕ್ತಿ_Set_Property(pNew, EP_CanBeNull);
        }
        ಯದಿ( pNew->op==TK_TRUEFALSE ){
          pNew->u.i_ಮೌಲ್ಯ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(pNew);
          pNew->op = TK_INTEGER;
          ಅಭಿವ್ಯಕ್ತಿ_Set_Property(pNew, EP_Int_ಮೌಲ್ಯ);
        }

        /* Ensure that the expression now has an implicit collation sequence,
        ** just as it did when it was a column of a view or sub-query. */
        {
          CollSeq *pNat = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pSubst->pParse, pNew);
          CollSeq *pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pSubst->pParse,
                pSubst->pCList->a[iColumn].p_ಅಭಿವ್ಯಕ್ತಿ
          );
          ಯದಿ( pNat!=pColl || (pNew->op!=TK_COLUMN && pNew->op!=TK_COLLATE) ){
            pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Add_Collate_String(pSubst->pParse, pNew,
                (pColl ? pColl->zName : "BINARY")
            );
          }
        }
        ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(pNew, EP_Collate);
        ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_OuterON|EP_InnerON) ){
          sqlite3_Set_Join_ಅಭಿವ್ಯಕ್ತಿ(pNew, p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin,
                             p_ಅಭಿವ್ಯಕ್ತಿ->flags & (EP_OuterON|EP_InnerON));
        }
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ);
        p_ಅಭಿವ್ಯಕ್ತಿ = pNew;
      }
    }
  }ಅನ್ಯಥಾ{
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IF_NULL_ROW && p_ಅಭಿವ್ಯಕ್ತಿ->iTable==pSubst->iTable ){
      p_ಅಭಿವ್ಯಕ್ತಿ->iTable = pSubst->iNewTable;
    }
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION && p_ಅಭಿವ್ಯಕ್ತಿ->op2>=pSubst->nSelDepth ){
      p_ಅಭಿವ್ಯಕ್ತಿ->op2--;
    }
    p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = subst_ಅಭಿವ್ಯಕ್ತಿ(pSubst, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
    p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ = subst_ಅಭಿವ್ಯಕ್ತಿ(pSubst, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
      substSelect(pSubst, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect, 1);
    }ಅನ್ಯಥಾ{
      subst_ಅಭಿವ್ಯಕ್ತಿ_List(pSubst, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
      Window *pWin = p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin;
      pWin->pFilter = subst_ಅಭಿವ್ಯಕ್ತಿ(pSubst, pWin->pFilter);
      subst_ಅಭಿವ್ಯಕ್ತಿ_List(pSubst, pWin->pPartition);
      subst_ಅಭಿವ್ಯಕ್ತಿ_List(pSubst, pWin->pOrderBy);
    }
#endif
  }
  ಪ್ರತ್ಯಾಗಮ p_ಅಭಿವ್ಯಕ್ತಿ;
}
ಸ್ಥಿರ ಶೂನ್ಯ subst_ಅಭಿವ್ಯಕ್ತಿ_List(
  SubstContext *pSubst, /* Description of the substitution */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList       /* List to scan and in which to make substitutes */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( pList==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ = subst_ಅಭಿವ್ಯಕ್ತಿ(pSubst, pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ);
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ substSelect(
  SubstContext *pSubst, /* Description of the substitution */
  Select *p,            /* SELECT statement in which to make substitutions */
  ಪೂರ್ಣಾಙ್ಕ doPrior           /* Do substitutes on p->pPrior too */
){
  SrcList *pSrc;
  SrcItem *pItem;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( !p ) ಪ್ರತ್ಯಾಗಮ;
  pSubst->nSelDepth++;
  ಕುರು{
    subst_ಅಭಿವ್ಯಕ್ತಿ_List(pSubst, p->pEList);
    subst_ಅಭಿವ್ಯಕ್ತಿ_List(pSubst, p->pGroupBy);
    subst_ಅಭಿವ್ಯಕ್ತಿ_List(pSubst, p->pOrderBy);
    p->pHaving = subst_ಅಭಿವ್ಯಕ್ತಿ(pSubst, p->pHaving);
    p->pWhere = subst_ಅಭಿವ್ಯಕ್ತಿ(pSubst, p->pWhere);
    pSrc = p->pSrc;
    ಪ್ರತಿಜ್ಞಾ( pSrc!=0 );
    ಪ್ರತಿ(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      ಯದಿ( pItem->fg.isSubquery ){
        substSelect(pSubst, pItem->u4.pSubq->pSelect, 1);
      }
      ಯದಿ( pItem->fg.isTabFunc ){
        subst_ಅಭಿವ್ಯಕ್ತಿ_List(pSubst, pItem->u1.pFuncArg);
      }
    }
  }ಯಾವತ್( doPrior && (p = p->pPrior)!=0 );
  pSubst->nSelDepth--;
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ಯದಿ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
/*
** pSelect is a SELECT statement and pSrcItem is one item in the FROM
** clause of that SELECT.
**
** This routine scans the entire SELECT statement and recomputes the
** pSrcItem->colUsed mask.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ recompute_Columns_Used_ಅಭಿವ್ಯಕ್ತಿ(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  SrcItem *pItem;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_COLUMN ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  pItem = pWalker->u.pSrcItem;
  ಯದಿ( pItem->iCursor!=p_ಅಭಿವ್ಯಕ್ತಿ->iTable ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iColumn<0 ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  pItem->colUsed |= sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Col_Used(p_ಅಭಿವ್ಯಕ್ತಿ);
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}
ಸ್ಥಿರ ಶೂನ್ಯ recomputeColumnsUsed(
  Select *pSelect,                 /* The complete SELECT statement */
  SrcItem *pSrcItem                /* Which FROM clause item to recompute */
){
  Walker w;
  ಯದಿ( NEVER(pSrcItem->pSTab==0) ) ಪ್ರತ್ಯಾಗಮ;
  memset(&w, 0, ಪರಿಮಾಣ(w));
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = recompute_Columns_Used_ಅಭಿವ್ಯಕ್ತಿ;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pSrcItem = pSrcItem;
  pSrcItem->colUsed = 0;
  sqlite3WalkSelect(&w, pSelect);
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ಯದಿ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
ಸ್ಥಿರ ಶೂನ್ಯ srclistRenumberCursors(
  Parse *pParse,                  /* Parse context */
  ಪೂರ್ಣಾಙ್ಕ *aCsrMap,                   /* Array to store cursor mappings in */
  SrcList *pSrc,                  /* FROM clause to renumber */
  ಪೂರ್ಣಾಙ್ಕ iExcept                     /* FROM clause item to skip */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  SrcItem *pItem;
  ಪ್ರತಿ(i=0, pItem=pSrc->a; i<pSrc->nSrc; i++, pItem++){
    ಯದಿ( i!=iExcept ){
      Select *p;
      ಪ್ರತಿಜ್ಞಾ( pItem->iCursor < aCsrMap[0] );
      ಯದಿ( !pItem->fg.isRecursive || aCsrMap[pItem->iCursor+1]==0 ){
        aCsrMap[pItem->iCursor+1] = pParse->nTab++;
      }
      pItem->iCursor = aCsrMap[pItem->iCursor+1];
      ಯದಿ( pItem->fg.isSubquery ){
        ಪ್ರತಿ(p=pItem->u4.pSubq->pSelect; p; p=p->pPrior){
          srclistRenumberCursors(pParse, aCsrMap, p->pSrc, -1);
        }
      }
    }
  }
}

/*
** *piCursor is a cursor number.  Change it if it needs to be mapped.
*/
ಸ್ಥಿರ ಶೂನ್ಯ renumberCursorDoMapping(Walker *pWalker, ಪೂರ್ಣಾಙ್ಕ *piCursor){
  ಪೂರ್ಣಾಙ್ಕ *aCsrMap = pWalker->u.aiCol;
  ಪೂರ್ಣಾಙ್ಕ iCsr = *piCursor;
  ಯದಿ( iCsr < aCsrMap[0] && aCsrMap[iCsr+1]>0 ){
    *piCursor = aCsrMap[iCsr+1];
  }
}

/*
** Expression walker callback used by renumberCursors() to update
** Expr objects to match newly assigned cursor numbers.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ renumberCursorsCb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಙ್ಕ op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
  ಯದಿ( op==TK_COLUMN || op==TK_IF_NULL_ROW ){
    renumberCursorDoMapping(pWalker, &p_ಅಭಿವ್ಯಕ್ತಿ->iTable);
  }
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, (EP_OuterON|EP_InnerON)) ){
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) );
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON) );
    renumberCursorDoMapping(pWalker, &p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin);
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
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
ಸ್ಥಿರ ಶೂನ್ಯ renumberCursors(
  Parse *pParse,                  /* Parse context */
  Select *p,                      /* Select to renumber cursors within */
  ಪೂರ್ಣಾಙ್ಕ iExcept,                    /* FROM clause item to skip */
  ಪೂರ್ಣಾಙ್ಕ *aCsrMap                    /* Working space */
){
  Walker w;
  srclistRenumberCursors(pParse, aCsrMap, p->pSrc, iExcept);
  memset(&w, 0, ಪರಿಮಾಣ(w));
  w.u.aiCol = aCsrMap;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = renumberCursorsCb;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  sqlite3WalkSelect(&w, p);
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** If pSel is not part of a compound SELECT, return a pointer to its
** expression list. Otherwise, return a pointer to the expression list
** of the leftmost SELECT in the compound.
*/
ಸ್ಥಿರ ಅಭಿವ್ಯಕ್ತಿ_List *findLeftmostExprlist(Select *pSel){
  ಯಾವತ್( pSel->pPrior ){
    pSel = pSel->pPrior;
  }
  ಪ್ರತ್ಯಾಗಮ pSel->pEList;
}

/*
** Return true if any of the result-set columns in the compound query
** have incompatible affinities on one or more arms of the compound.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ compoundHasDifferentAffinities(Select *p){
  ಪೂರ್ಣಾಙ್ಕ ii;
  ಅಭಿವ್ಯಕ್ತಿ_List *pList;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಪ್ರತಿಜ್ಞಾ( p->pEList!=0 );
  ಪ್ರತಿಜ್ಞಾ( p->pPrior!=0 );
  pList = p->pEList;
  ಪ್ರತಿ(ii=0; ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
    ವರ್ಣ aff;
    Select *pSub1;
    ಪ್ರತಿಜ್ಞಾ( pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
    aff = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ);
    ಪ್ರತಿ(pSub1=p->pPrior; pSub1; pSub1=pSub1->pPrior){
      ಪ್ರತಿಜ್ಞಾ( pSub1->pEList!=0 );
      ಪ್ರತಿಜ್ಞಾ( pSub1->pEList->n_ಅಭಿವ್ಯಕ್ತಿ>ii );
      ಪ್ರತಿಜ್ಞಾ( pSub1->pEList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
      ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pSub1->pEList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ)!=aff ){
        ಪ್ರತ್ಯಾಗಮ 1;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

#ಯದಿ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ flattenSubquery(
  Parse *pParse,       /* Parsing context */
  Select *p,           /* The parent or outer SELECT statement */
  ಪೂರ್ಣಾಙ್ಕ iFrom,           /* Index in p->pSrc->a[] of the inner subquery */
  ಪೂರ್ಣಾಙ್ಕ isAgg            /* True if outer SELECT uses aggregate functions */
){
  ಧ್ರುವ ವರ್ಣ *zSavedAuthContext = pParse->zAuthContext;
  Select *pParent;    /* Current UNION ALL term of the other query */
  Select *pSub;       /* The inner query or "subquery" */
  Select *pSub1;      /* Pointer to the rightmost select in sub-query */
  SrcList *pSrc;      /* The FROM clause of the outer query */
  SrcList *pSubSrc;   /* The FROM clause of the subquery */
  ಪೂರ್ಣಾಙ್ಕ iParent;        /* VDBE cursor number of the pSub result set temp table */
  ಪೂರ್ಣಾಙ್ಕ iNewParent = -1;/* Replacement table for iParent */
  ಪೂರ್ಣಾಙ್ಕ isOuterJoin = 0; /* True if pSub is the right side of a LEFT JOIN */   
  ಪೂರ್ಣಾಙ್ಕ i;              /* Loop counter */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere;                    /* The WHERE clause */
  SrcItem *pSubitem;               /* The subquery */
  sqlite3 *db = pParse->db;
  Walker w;                        /* Walker to persist agginfo data */
  ಪೂರ್ಣಾಙ್ಕ *aCsrMap = 0;

  /* Check to see if flattening is permitted.  Return 0 if not.
  */
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಪ್ರತಿಜ್ಞಾ( p->pPrior==0 );
  ಯದಿ( OptimizationDisabled(db, SQLITE_QueryFlattener) ) ಪ್ರತ್ಯಾಗಮ 0;
  pSrc = p->pSrc;
  ಪ್ರತಿಜ್ಞಾ( pSrc && iFrom>=0 && iFrom<pSrc->nSrc );
  pSubitem = &pSrc->a[iFrom];
  iParent = pSubitem->iCursor;
  ಪ್ರತಿಜ್ಞಾ( pSubitem->fg.isSubquery );
  pSub = pSubitem->u4.pSubq->pSelect;
  ಪ್ರತಿಜ್ಞಾ( pSub!=0 );

#ifndef SQLITE_OMIT_WINDOWFUNC
  ಯದಿ( p->pWin || pSub->pWin ) ಪ್ರತ್ಯಾಗಮ 0;                  /* Restriction (25) */
#endif

  pSubSrc = pSub->pSrc;
  ಪ್ರತಿಜ್ಞಾ( pSubSrc );
  /* Prior to version 3.1.2, when LIMIT and OFFSET had to be simple constants,
  ** not arbitrary expressions, we allowed some combining of LIMIT and OFFSET
  ** because they could be computed at compile-time.  But when LIMIT and OFFSET
  ** became arbitrary expressions, we were forced to add restrictions (13)
  ** and (14). */
  ಯದಿ( pSub->pLimit && p->pLimit ) ಪ್ರತ್ಯಾಗಮ 0;              /* Restriction (13) */
  ಯದಿ( pSub->pLimit && pSub->pLimit->p_ಬಲ ) ಪ್ರತ್ಯಾಗಮ 0;   /* Restriction (14) */
  ಯದಿ( (p->selFlags & SF_Compound)!=0 && pSub->pLimit ){
    ಪ್ರತ್ಯಾಗಮ 0;                                            /* Restriction (15) */
  }
  ಯದಿ( pSubSrc->nSrc==0 ) ಪ್ರತ್ಯಾಗಮ 0;                       /* Restriction (7)  */
  ಯದಿ( pSub->selFlags & SF_Distinct ) ಪ್ರತ್ಯಾಗಮ 0;           /* Restriction (4)  */
  ಯದಿ( pSub->pLimit && (pSrc->nSrc>1 || isAgg) ){
     ಪ್ರತ್ಯಾಗಮ 0;         /* Restrictions (8)(9) */
  }
  ಯದಿ( p->pOrderBy && pSub->pOrderBy ){
     ಪ್ರತ್ಯಾಗಮ 0;                                           /* Restriction (11) */
  }
  ಯದಿ( isAgg && pSub->pOrderBy ) ಪ್ರತ್ಯಾಗಮ 0;                /* Restriction (16) */
  ಯದಿ( pSub->pLimit && p->pWhere ) ಪ್ರತ್ಯಾಗಮ 0;              /* Restriction (19) */
  ಯದಿ( pSub->pLimit && (p->selFlags & SF_Distinct)!=0 ){
     ಪ್ರತ್ಯಾಗಮ 0;         /* Restriction (21) */
  }
  ಯದಿ( pSub->selFlags & (SF_Recursive) ){
    ಪ್ರತ್ಯಾಗಮ 0; /* Restrictions (22) */
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
  ಯದಿ( (pSubitem->fg.jointype & (JT_OUTER|JT_LTORJ))!=0 ){
    ಯದಿ( pSubSrc->nSrc>1                        /* (3a) */
     /**** || IsVirtual(pSubSrc->a[0].pSTab)      (3b)-omitted */
     || (p->selFlags & SF_Distinct)!=0         /* (3d) */
     || (pSubitem->fg.jointype & JT_ಬಲ)!=0  /* (26) */
    ){
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    isOuterJoin = 1;
  }

  ಪ್ರತಿಜ್ಞಾ( pSubSrc->nSrc>0 );  /* True by restriction (7) */
  ಯದಿ( iFrom>0 && (pSubSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
    ಪ್ರತ್ಯಾಗಮ 0;   /* Restriction (27a) */
  }

  /* Condition (28) is blocked by the caller */
  ಪ್ರತಿಜ್ಞಾ( !pSubitem->fg.isCte || pSubitem->u2.pCteUse->eM10d!=M10d_Yes );

  /* Restriction (17): If the sub-query is a compound SELECT, then it must
  ** use only the UNION ALL operator. And none of the simple select queries
  ** that make up the compound SELECT are allowed to be aggregate or distinct
  ** queries.
  */
  ಯದಿ( pSub->pPrior ){
    ಪೂರ್ಣಾಙ್ಕ ii;
    ಯದಿ( pSub->pOrderBy ){
      ಪ್ರತ್ಯಾಗಮ 0;  /* Restriction (20) */
    }
    ಯದಿ( isAgg || (p->selFlags & SF_Distinct)!=0 || isOuterJoin>0 ){
      ಪ್ರತ್ಯಾಗಮ 0; /* (17d1), (17d2), or (17f) */
    }
    ಪ್ರತಿ(pSub1=pSub; pSub1; pSub1=pSub1->pPrior){
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct );
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Aggregate );
      ಪ್ರತಿಜ್ಞಾ( pSub->pSrc!=0 );
      ಪ್ರತಿಜ್ಞಾ( (pSub->selFlags & SF_Recursive)==0 );
      ಪ್ರತಿಜ್ಞಾ( pSub->pEList->n_ಅಭಿವ್ಯಕ್ತಿ==pSub1->pEList->n_ಅಭಿವ್ಯಕ್ತಿ );
      ಯದಿ( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))!=0    /* (17b) */
       || (pSub1->pPrior && pSub1->op!=TK_ALL)                 /* (17a) */
       || pSub1->pSrc->nSrc<1                                  /* (17c) */
#ifndef SQLITE_OMIT_WINDOWFUNC
       || pSub1->pWin                                          /* (17e) */
#endif
      ){
        ಪ್ರತ್ಯಾಗಮ 0;
      }
      ಯದಿ( iFrom>0 && (pSub1->pSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
        /* Without this restriction, the JT_LTORJ flag would end up being
        ** omitted on left-hand tables of the right join that is being
        ** flattened. */
        ಪ್ರತ್ಯಾಗಮ 0;   /* Restrictions (17g), (27b) */
      }
      testcase( pSub1->pSrc->nSrc>1 );
    }

    /* Restriction (18). */
    ಯದಿ( p->pOrderBy ){
      ಪ್ರತಿ(ii=0; ii<p->pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
        ಯದಿ( p->pOrderBy->a[ii].u.x.iOrderByCol==0 ) ಪ್ರತ್ಯಾಗಮ 0;
      }
    }

    /* Restriction (23) */
    ಯದಿ( (p->selFlags & SF_Recursive) ) ಪ್ರತ್ಯಾಗಮ 0;

    /* Restriction (17h) */
    ಯದಿ( compoundHasDifferentAffinities(pSub) ) ಪ್ರತ್ಯಾಗಮ 0;

    ಯದಿ( pSrc->nSrc>1 ){
      ಯದಿ( pParse->nSelect>500 ) ಪ್ರತ್ಯಾಗಮ 0;
      ಯದಿ( OptimizationDisabled(db, SQLITE_FlttnUnionAll) ) ಪ್ರತ್ಯಾಗಮ 0;
      aCsrMap = sqlite3DbMallocZero(db, ((i64)pParse->nTab+1)*ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
      ಯದಿ( aCsrMap ) aCsrMap[0] = pParse->nTab;
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
  
  ಯದಿ( ALWAYS(pSubitem->fg.isSubquery) ){
    pSub1 = sqlite3SubqueryDetach(db, pSubitem);
  }ಅನ್ಯಥಾ{
    pSub1 = 0;
  }
  ಪ್ರತಿಜ್ಞಾ( pSubitem->fg.isSubquery==0 );
  ಪ್ರತಿಜ್ಞಾ( pSubitem->fg.fixedSchema==0 );
  sqlite3DbFree(db, pSubitem->zName);
  sqlite3DbFree(db, pSubitem->zAlias);
  pSubitem->zName = 0;
  pSubitem->zAlias = 0;
  ಪ್ರತಿಜ್ಞಾ( pSubitem->fg.isUsing!=0 || pSubitem->u3.pOn==0 );

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
  ಪ್ರತಿ(pSub=pSub->pPrior; pSub; pSub=pSub->pPrior){
    Select *pNew;
    ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy = p->pOrderBy;
    ಅಭಿವ್ಯಕ್ತಿ *pLimit = p->pLimit;
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
    ಯದಿ( pNew==0 ){
      p->pPrior = pPrior;
    }ಅನ್ಯಥಾ{
      pNew->selId = ++pParse->nSelect;
      ಯದಿ( aCsrMap && ALWAYS(db->mallocFailed==0) ){
        renumberCursors(pParse, pNew, iFrom, aCsrMap);
      }
      pNew->pPrior = pPrior;
      ಯದಿ( pPrior ) pPrior->pNext = pNew;
      pNew->pNext = p;
      p->pPrior = pNew;
      TREETRACE(0x4,pParse,p,("compound-subquery flattener"
                              " creates %u as peer\n",pNew->selId));
    }
    ಪ್ರತಿಜ್ಞಾ( pSubitem->fg.isSubquery==0 );
  }
  sqlite3DbFree(db, aCsrMap);
  ಯದಿ( db->mallocFailed ){
    ಪ್ರತಿಜ್ಞಾ( pSubitem->fg.fixedSchema==0 );
    ಪ್ರತಿಜ್ಞಾ( pSubitem->fg.isSubquery==0 );
    ಪ್ರತಿಜ್ಞಾ( pSubitem->u4.zDatabase==0 );
    sqlite3SrcItemAttachSubquery(pParse, pSubitem, pSub1, 0);
    ಪ್ರತ್ಯಾಗಮ 1;
  }

  /* Defer deleting the Table object associated with the
  ** subquery until code generation is
  ** complete, since there may still exist Expr.pTab entries that
  ** refer to the subquery even after flattening.  Ticket #3346.
  **
  ** pSubitem->pSTab is always non-NULL by test restrictions and tests above.
  */
  ಯದಿ( ALWAYS(pSubitem->pSTab!=0) ){
    Table *pTabToDel = pSubitem->pSTab;
    ಯದಿ( pTabToDel->nTabRef==1 ){
      Parse *pToplevel = sqlite3ParseToplevel(pParse);
      sqlite3ParserAddCleanup(pToplevel, sqlite3DeleteTableGeneric, pTabToDel);
      testcase( pToplevel->earlyCleanup );
    }ಅನ್ಯಥಾ{
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
  ಪ್ರತಿ(pParent=p; pParent; pParent=pParent->pPrior, pSub=pSub->pPrior){
    ಪೂರ್ಣಾಙ್ಕ nSubSrc;
    u8 jointype = pParent->pSrc->a[iFrom].fg.jointype;
    ಪ್ರತಿಜ್ಞಾ( pSub!=0 );
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
    ಯದಿ( nSubSrc>1 ){
      pSrc = sqlite3SrcListEnlarge(pParse, pSrc, nSubSrc-1,iFrom+1);
      ಯದಿ( pSrc==0 ) ಭಙ್ಗ;
      pParent->pSrc = pSrc;
    }

    /* Transfer the FROM clause terms from the subquery into the
    ** outer query.
    */
    iNewParent = pSubSrc->a[0].iCursor;
    ಪ್ರತಿ(i=0; i<nSubSrc; i++){
      SrcItem *pItem = &pSrc->a[i+iFrom];
      ಪ್ರತಿಜ್ಞಾ( pItem->fg.isTabFunc==0 );
      ಪ್ರತಿಜ್ಞಾ( pItem->fg.isSubquery
           || pItem->fg.fixedSchema
           || pItem->u4.zDatabase==0 );
      ಯದಿ( pItem->fg.isUsing ) sqlite3IdListDelete(db, pItem->u3.pUsing);
      *pItem = pSubSrc->a[i];
      pItem->fg.jointype |= (jointype & JT_LTORJ);
      memset(&pSubSrc->a[i], 0, ಪರಿಮಾಣ(pSubSrc->a[i]));
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
    ಯದಿ( pSub->pOrderBy ){
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
      ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy = pSub->pOrderBy;
      ಪ್ರತಿ(i=0; i<pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
        pOrderBy->a[i].u.x.iOrderByCol = 0;
      }
      ಪ್ರತಿಜ್ಞಾ( pParent->pOrderBy==0 );
      pParent->pOrderBy = pOrderBy;
      pSub->pOrderBy = 0;
    }
    pWhere = pSub->pWhere;
    pSub->pWhere = 0;
    ಯದಿ( isOuterJoin>0 ){
      ಪ್ರತಿಜ್ಞಾ( pSubSrc->nSrc==1 );
      sqlite3_Set_Join_ಅಭಿವ್ಯಕ್ತಿ(pWhere, iNewParent, EP_OuterON);
    }
    ಯದಿ( pWhere ){
      ಯದಿ( pParent->pWhere ){
        pParent->pWhere = sqlite3PExpr(pParse, TK_AND, pWhere, pParent->pWhere);
      }ಅನ್ಯಥಾ{
        pParent->pWhere = pWhere;
      }
    }
    ಯದಿ( db->mallocFailed==0 ){
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
    ಪ್ರತಿಜ್ಞಾ( (pSub->selFlags & SF_Distinct)==0 ); /* restriction (17b) */
 
    /*
    ** SELECT ... FROM (SELECT ... LIMIT a OFFSET b) LIMIT x OFFSET y;
    **
    ** One is tempted to try to add a and b to combine the limits.  But this
    ** does not work if either limit is negative.
    */
    ಯದಿ( pSub->pLimit ){
      pParent->pLimit = pSub->pLimit;
      pSub->pLimit = 0;
    }

    /* Recompute the SrcItem.colUsed masks for the flattened
    ** tables. */
    ಪ್ರತಿ(i=0; i<nSubSrc; i++){
      recomputeColumnsUsed(pParent, &pSrc->a[i+iFrom]);
    }
  }

  /* Finally, delete what is left of the subquery and return success.
  */
  sqlite3AggInfoPersistWalkerInit(&w, pParse);
  sqlite3WalkSelect(&w,pSub1);
  sqlite3SelectDelete(db, pSub1);

#ಯದಿ TREETRACE_ENABLED
  ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x4 ){
    TREETRACE(0x4,pParse,p,("After flattening:\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
  }
#endif

  ಪ್ರತ್ಯಾಗಮ 1;
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** A structure to keep track of all of the column values that are fixed to
** a known value due to WHERE clause constraints of the form COLUMN=VALUE.
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ WhereConst WhereConst;
ಸಂರಚನಾ WhereConst {
  Parse *pParse;   /* Parsing context */
  u8 *pOomFault;   /* Pointer to pParse->db->mallocFailed */
  ಪೂರ್ಣಾಙ್ಕ nConst;      /* Number for COLUMN=CONSTANT terms */
  ಪೂರ್ಣಾಙ್ಕ nChng;       /* Number of times a constant is propagated */
  ಪೂರ್ಣಾಙ್ಕ bHasAffBlob; /* At least one column in apExpr[] as affinity BLOB */
  u32 mExcludeOn;  /* Which ON expressions to exclude from considertion.
                   ** Either EP_OuterON or EP_InnerON|EP_OuterON */
  ಅಭಿವ್ಯಕ್ತಿ **ap_ಅಭಿವ್ಯಕ್ತಿ;   /* [i*2] is COLUMN and [i*2+1] is VALUE */
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
ಸ್ಥಿರ ಶೂನ್ಯ const_ಸೇರಿಸು(
  WhereConst *pConst,  /* The WhereConst into which we are inserting */
  ಅಭಿವ್ಯಕ್ತಿ *pColumn,       /* The COLUMN part of the constraint */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಮೌಲ್ಯ,        /* The VALUE part of the constraint */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ          /* Overall expression: COLUMN=VALUE or VALUE=COLUMN */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿಜ್ಞಾ( pColumn->op==TK_COLUMN );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(pConst->pParse, p_ಮೌಲ್ಯ) );
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pColumn, EP_FixedCol) ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಮೌಲ್ಯ)!=0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( !sqlite3_Is_ದ್ವಿಮಾನ(sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare_Coll_Seq(pConst->pParse,p_ಅಭಿವ್ಯಕ್ತಿ)) ){
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Return_Subtype(pConst->pParse, p_ಮೌಲ್ಯ) ) ಪ್ರತ್ಯಾಗಮ;

  /* 2018-10-25 ticket [cf5ed20f]
  ** Make sure the same pColumn is not inserted more than once */
  ಪ್ರತಿ(i=0; i<pConst->nConst; i++){
    ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pE2 = pConst->ap_ಅಭಿವ್ಯಕ್ತಿ[i*2];
    ಪ್ರತಿಜ್ಞಾ( pE2->op==TK_COLUMN );
    ಯದಿ( pE2->iTable==pColumn->iTable
     && pE2->iColumn==pColumn->iColumn
    ){
      ಪ್ರತ್ಯಾಗಮ;  /* Already present.  Return without doing anything. */
    }
  }
  ಪ್ರತಿಜ್ಞಾ( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pColumn)<=SQLITE_AFF_BLOB ){
    pConst->bHasAffBlob = 1;
  }

  pConst->nConst++;
  pConst->ap_ಅಭಿವ್ಯಕ್ತಿ = sqlite3DbReallocOrFree(pConst->pParse->db, pConst->ap_ಅಭಿವ್ಯಕ್ತಿ,
                         pConst->nConst*2*ಪರಿಮಾಣ(ಅಭಿವ್ಯಕ್ತಿ*));
  ಯದಿ( pConst->ap_ಅಭಿವ್ಯಕ್ತಿ==0 ){
    pConst->nConst = 0;
  }ಅನ್ಯಥಾ{
    pConst->ap_ಅಭಿವ್ಯಕ್ತಿ[pConst->nConst*2-2] = pColumn;
    pConst->ap_ಅಭಿವ್ಯಕ್ತಿ[pConst->nConst*2-1] = p_ಮೌಲ್ಯ;
  }
}

/*
** Find all terms of COLUMN=VALUE or VALUE=COLUMN in pExpr where VALUE
** is a constant expression and where the term must be true because it
** is part of the AND-connected terms of the expression.  For each term
** found, add it to the pConst structure.
*/
ಸ್ಥಿರ ಶೂನ್ಯ findConstInWhere(WhereConst *pConst, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ, *p_ಎಡ;
  ಯದಿ( NEVER(p_ಅಭಿವ್ಯಕ್ತಿ==0) ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, pConst->mExcludeOn) ){
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) );
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON) );
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AND ){
    findConstInWhere(pConst, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
    findConstInWhere(pConst, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_EQ ) ಪ್ರತ್ಯಾಗಮ;
  p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
  p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
  ಪ್ರತಿಜ್ಞಾ( p_ಬಲ!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಎಡ!=0 );
  ಯದಿ( p_ಬಲ->op==TK_COLUMN && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(pConst->pParse, p_ಎಡ) ){
    const_ಸೇರಿಸು(pConst,p_ಬಲ,p_ಎಡ,p_ಅಭಿವ್ಯಕ್ತಿ);
  }
  ಯದಿ( p_ಎಡ->op==TK_COLUMN && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(pConst->pParse, p_ಬಲ) ){
    const_ಸೇರಿಸು(pConst,p_ಎಡ,p_ಬಲ,p_ಅಭಿವ್ಯಕ್ತಿ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ propagate_Constant_ಅಭಿವ್ಯಕ್ತಿ_Rewrite_One(
  WhereConst *pConst,
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,
  ಪೂರ್ಣಾಙ್ಕ bIgnoreAffBlob
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( pConst->pOomFault[0] ) ಪ್ರತ್ಯಾಗಮ WRC_Prune;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_COLUMN ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FixedCol|pConst->mExcludeOn) ){
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FixedCol) );
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) );
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON) );
    ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  }
  ಪ್ರತಿ(i=0; i<pConst->nConst; i++){
    ಅಭಿವ್ಯಕ್ತಿ *pColumn = pConst->ap_ಅಭಿವ್ಯಕ್ತಿ[i*2];
    ಯದಿ( pColumn==p_ಅಭಿವ್ಯಕ್ತಿ ) ಅನುವರ್ತ;
    ಯದಿ( pColumn->iTable!=p_ಅಭಿವ್ಯಕ್ತಿ->iTable ) ಅನುವರ್ತ;
    ಯದಿ( pColumn->iColumn!=p_ಅಭಿವ್ಯಕ್ತಿ->iColumn ) ಅನುವರ್ತ;
    ಪ್ರತಿಜ್ಞಾ( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
    ಯದಿ( bIgnoreAffBlob && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pColumn)<=SQLITE_AFF_BLOB ){
      ಭಙ್ಗ;
    }
    /* A match is found.  Add the EP_FixedCol property */
    pConst->nChng++;
    ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Leaf);
    ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FixedCol);
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ==0 );
    p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(pConst->pParse->db, pConst->ap_ಅಭಿವ್ಯಕ್ತಿ[i*2+1], 0);
    ಯದಿ( pConst->pParse->db->mallocFailed ) ಪ್ರತ್ಯಾಗಮ WRC_Prune;
    ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Prune;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ propagate_Constant_ಅಭಿವ್ಯಕ್ತಿ_Rewrite(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  WhereConst *pConst = pWalker->u.pConst;
  ಪ್ರತಿಜ್ಞಾ( TK_GT==TK_EQ+1 );
  ಪ್ರತಿಜ್ಞಾ( TK_LE==TK_EQ+2 );
  ಪ್ರತಿಜ್ಞಾ( TK_LT==TK_EQ+3 );
  ಪ್ರತಿಜ್ಞಾ( TK_GE==TK_EQ+4 );
  ಯದಿ( pConst->bHasAffBlob ){
    ಯದಿ( (p_ಅಭಿವ್ಯಕ್ತಿ->op>=TK_EQ && p_ಅಭಿವ್ಯಕ್ತಿ->op<=TK_GE)
     || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS
    ){
      propagate_Constant_ಅಭಿವ್ಯಕ್ತಿ_Rewrite_One(pConst, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
      ಯದಿ( pConst->pOomFault[0] ) ಪ್ರತ್ಯಾಗಮ WRC_Prune;
      ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ)!=SQLITE_AFF_TEXT ){
        propagate_Constant_ಅಭಿವ್ಯಕ್ತಿ_Rewrite_One(pConst, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, 0);
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ propagate_Constant_ಅಭಿವ್ಯಕ್ತಿ_Rewrite_One(pConst, p_ಅಭಿವ್ಯಕ್ತಿ, pConst->bHasAffBlob);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ propagateConstants(
  Parse *pParse,   /* The parsing context */
  Select *p        /* The query in which to propagate constants */
){
  WhereConst x;
  Walker w;
  ಪೂರ್ಣಾಙ್ಕ nChng = 0;
  x.pParse = pParse;
  x.pOomFault = &pParse->db->mallocFailed;
  ಕುರು{
    x.nConst = 0;
    x.nChng = 0;
    x.ap_ಅಭಿವ್ಯಕ್ತಿ = 0;
    x.bHasAffBlob = 0;
    ಯದಿ( ALWAYS(p->pSrc!=0)
     && p->pSrc->nSrc>0
     && (p->pSrc->a[0].fg.jointype & JT_LTORJ)!=0
    ){
      /* Do not propagate constants on any ON clause if there is a
      ** RIGHT JOIN anywhere in the query */
      x.mExcludeOn = EP_InnerON | EP_OuterON;
    }ಅನ್ಯಥಾ{
      /* Do not propagate constants through the ON clause of a LEFT JOIN */
      x.mExcludeOn = EP_OuterON;
    }
    findConstInWhere(&x, p->pWhere);
    ಯದಿ( x.nConst ){
      memset(&w, 0, ಪರಿಮಾಣ(w));
      w.pParse = pParse;
      w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = propagate_Constant_ಅಭಿವ್ಯಕ್ತಿ_Rewrite;
      w.xSelectCallback = sqlite3SelectWalkNoop;
      w.xSelectCallback2 = 0;
      w.walkerDepth = 0;
      w.u.pConst = &x;
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p->pWhere);
      sqlite3DbFree(x.pParse->db, x.ap_ಅಭಿವ್ಯಕ್ತಿ);
      nChng += x.nChng;
    }
  }ಯಾವತ್( x.nChng ); 
  ಪ್ರತ್ಯಾಗಮ nChng;
}

#ಯದಿ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
# ಯದಿ !defined(SQLITE_OMIT_WINDOWFUNC)
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ತಳ್ಳು_Down_Window_Check(Parse *pParse, Select *pSubq, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪ್ರತಿಜ್ಞಾ( pSubq->pWin->pPartition );
  ಪ್ರತಿಜ್ಞಾ( (pSubq->selFlags & SF_MultiPart)==0 );
  ಪ್ರತಿಜ್ಞಾ( pSubq->pPrior==0 );
  ಪ್ರತ್ಯಾಗಮ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Or_Group_By(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, pSubq->pWin->pPartition);
}
# endif /* SQLITE_OMIT_WINDOWFUNC */
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ಯದಿ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ತಳ್ಳು_Down_Where_Terms(
  Parse *pParse,        /* Parse context (for malloc() and error reporting) */
  Select *pSubq,        /* The subquery whose WHERE clause is to be augmented */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,         /* The WHERE clause of the outer query */
  SrcList *pSrcList,    /* The complete from clause of the outer query */
  ಪೂರ್ಣಾಙ್ಕ iSrc              /* Which FROM clause term to try to push into  */
){
  ಅಭಿವ್ಯಕ್ತಿ *pNew;
  SrcItem *pSrc;        /* The subquery FROM term into which WHERE is pushed */
  ಪೂರ್ಣಾಙ್ಕ nChng = 0;
  pSrc = &pSrcList->a[iSrc];
  ಯದಿ( pWhere==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( pSubq->selFlags & (SF_Recursive|SF_MultiPart) ){
    ಪ್ರತ್ಯಾಗಮ 0;           /* restrictions (2) and (11) */
  }
  ಯದಿ( pSrc->fg.jointype & (JT_LTORJ|JT_ಬಲ) ){
    ಪ್ರತ್ಯಾಗಮ 0;           /* restrictions (10) */
  }

  ಯದಿ( pSubq->pPrior ){
    Select *pSel;
    ಪೂರ್ಣಾಙ್ಕ notUnionAll = 0;
    ಪ್ರತಿ(pSel=pSubq; pSel; pSel=pSel->pPrior){
      u8 op = pSel->op;
      ಪ್ರತಿಜ್ಞಾ( op==TK_ALL || op==TK_SELECT
           || op==TK_UNION || op==TK_INTERSECT || op==TK_EXCEPT );
      ಯದಿ( op!=TK_ALL && op!=TK_SELECT ){
        notUnionAll = 1;
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      ಯದಿ( pSel->pWin ) ಪ್ರತ್ಯಾಗಮ 0;    /* restriction (6b) */
#endif
    }
    ಯದಿ( notUnionAll ){
      /* If any of the compound arms are connected using UNION, INTERSECT,
      ** or EXCEPT, then we must ensure that none of the columns use a
      ** non-BINARY collating sequence. */
      ಪ್ರತಿ(pSel=pSubq; pSel; pSel=pSel->pPrior){
        ಪೂರ್ಣಾಙ್ಕ ii;
        ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *pList = pSel->pEList;
        ಪ್ರತಿಜ್ಞಾ( pList!=0 );
        ಪ್ರತಿ(ii=0; ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
          CollSeq *pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ);
          ಯದಿ( !sqlite3_Is_ದ್ವಿಮಾನ(pColl) ){
            ಪ್ರತ್ಯಾಗಮ 0;  /* Restriction (8) */
          }
        }
      }
    }
  }ಅನ್ಯಥಾ{
#ifndef SQLITE_OMIT_WINDOWFUNC
    ಯದಿ( pSubq->pWin && pSubq->pWin->pPartition==0 ) ಪ್ರತ್ಯಾಗಮ 0;
#endif
  }

#ifdef SQLITE_DEBUG
  /* Only the first term of a compound can have a WITH clause.  But make
  ** sure no other terms are marked SF_Recursive in case something changes
  ** in the future.
  */
  {
    Select *pX; 
    ಪ್ರತಿ(pX=pSubq; pX; pX=pX->pPrior){
      ಪ್ರತಿಜ್ಞಾ( (pX->selFlags & (SF_Recursive))==0 );
    }
  }
#endif

  ಯದಿ( pSubq->pLimit!=0 ){
    ಪ್ರತ್ಯಾಗಮ 0; /* restriction (3) */
  }
  ಯಾವತ್( pWhere->op==TK_AND ){
    nChng += ತಳ್ಳು_Down_Where_Terms(pParse, pSubq, pWhere->p_ಬಲ, pSrcList, iSrc);
    pWhere = pWhere->p_ಎಡ;
  }

#ಯದಿ 0 /* These checks now done by sqlite3ExprIsSingleTableConstraint() */
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pWhere, EP_OuterON|EP_InnerON) /* (9a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (9c) */
  ){
    ಪೂರ್ಣಾಙ್ಕ jj;
    ಪ್ರತಿ(jj=0; jj<iSrc; jj++){
      ಯದಿ( pWhere->w.iJoin==pSrcList->a[jj].iCursor ){
        /* If we reach this point, both (9a) and (9b) are satisfied.
        ** The following loop checks (9c):
        */
        ಪ್ರತಿ(jj++; jj<iSrc; jj++){
          ಯದಿ( (pSrcList->a[jj].fg.jointype & JT_ಬಲ)!=0 ){
            ಪ್ರತ್ಯಾಗಮ 0;  /* restriction (9) */
          }
        }
      }
    }
  }
  ಯದಿ( is_ಎಡ_Join
   && (ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pWhere,EP_OuterON)==0
         || pWhere->w.iJoin!=iCursor)
  ){
    ಪ್ರತ್ಯಾಗಮ 0; /* restriction (4) */
  }
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pWhere,EP_OuterON)
   && pWhere->w.iJoin!=iCursor
  ){
    ಪ್ರತ್ಯಾಗಮ 0; /* restriction (5) */
  }
#endif

#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
  ಯದಿ( ViewCanHaveRowid && (pWhere->op==TK_ISNULL || pWhere->op==TK_NOTNULL) ){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = pWhere->p_ಎಡ;
    ಯದಿ( ALWAYS(p_ಎಡ) 
     && p_ಎಡ->op==TK_COLUMN
     && p_ಎಡ->iColumn < 0
    ){
      ಪ್ರತ್ಯಾಗಮ 0;  /* Restriction (12) */
    }
  }
#endif

  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Single_Table_Constraint(pWhere, pSrcList, iSrc, 1) ){
    nChng++;
    pSubq->selFlags |= SF_ತಳ್ಳು_Down;
    ಯಾವತ್( pSubq ){
      SubstContext x;
      pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(pParse->db, pWhere, 0);
      unset_Join_ಅಭಿವ್ಯಕ್ತಿ(pNew, -1, 1);
      x.pParse = pParse;
      x.iTable = pSrc->iCursor;
      x.iNewTable = pSrc->iCursor;
      x.isOuterJoin = 0;
      x.nSelDepth = 0;
      x.pEList = pSubq->pEList;
      x.pCList = findLeftmostExprlist(pSubq);
      pNew = subst_ಅಭಿವ್ಯಕ್ತಿ(&x, pNew);
      ಪ್ರತಿಜ್ಞಾ( pNew!=0 || pParse->nErr!=0 );
      ಯದಿ( pParse->nErr==0 && pNew->op==TK_IN && ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pNew) ){
        ಪ್ರತಿಜ್ಞಾ( pNew->x.pSelect!=0 );
        pNew->x.pSelect->selFlags |= SF_ClonedRhsIn;
        ಪ್ರತಿಜ್ಞಾ( pWhere!=0 );
        ಪ್ರತಿಜ್ಞಾ( pWhere->op==TK_IN );
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pWhere) );
        ಪ್ರತಿಜ್ಞಾ( pWhere->x.pSelect!=0 );
        pWhere->x.pSelect->selFlags |= SF_ClonedRhsIn;
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      ಯದಿ( pSubq->pWin && 0==ತಳ್ಳು_Down_Window_Check(pParse, pSubq, pNew) ){
        /* Restriction 6c has prevented push-down in this case */
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, pNew);
        nChng--;
        ಭಙ್ಗ;
      }
#endif
      ಯದಿ( pSubq->selFlags & SF_Aggregate ){
        pSubq->pHaving = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, pSubq->pHaving, pNew);
      }ಅನ್ಯಥಾ{
        pSubq->pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, pSubq->pWhere, pNew);
      }
      pSubq = pSubq->pPrior;
    }
  }
  ಪ್ರತ್ಯಾಗಮ nChng;
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** Check to see if a subquery contains result-set columns that are
** never used.  If it does, change the value of those result-set columns
** to NULL so that they do not cause unnecessary work to compute.
**
** Return the number of column that were changed to NULL.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ disableUnusedSubqueryResultColumns(SrcItem *pItem){
  ಪೂರ್ಣಾಙ್ಕ nCol;
  Select *pSub;      /* The subquery to be simplified */
  Select *pX;        /* For looping over compound elements of pSub */
  Table *pTab;       /* The table that describes the subquery */
  ಪೂರ್ಣಾಙ್ಕ j;             /* Column number */
  ಪೂರ್ಣಾಙ್ಕ nChng = 0;     /* Number of columns converted to NULL */
  Bitmask colUsed;   /* Columns that may not be NULLed out */

  ಪ್ರತಿಜ್ಞಾ( pItem!=0 );
  ಯದಿ( pItem->fg.isCorrelated || pItem->fg.isCte ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತಿಜ್ಞಾ( pItem->pSTab!=0 );
  pTab = pItem->pSTab;
  ಪ್ರತಿಜ್ಞಾ( pItem->fg.isSubquery );
  pSub = pItem->u4.pSubq->pSelect;
  ಪ್ರತಿಜ್ಞಾ( pSub->pEList->n_ಅಭಿವ್ಯಕ್ತಿ==pTab->nCol );
  ಪ್ರತಿ(pX=pSub; pX; pX=pX->pPrior){
    ಯದಿ( (pX->selFlags & (SF_Distinct|SF_Aggregate))!=0 ){
      testcase( pX->selFlags & SF_Distinct );
      testcase( pX->selFlags & SF_Aggregate );
      ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಯದಿ( pX->pPrior && pX->op!=TK_ALL ){
      /* This optimization does not work for compound subqueries that
      ** use UNION, INTERSECT, or EXCEPT.  Only UNION ALL is allowed. */
      ಪ್ರತ್ಯಾಗಮ 0;
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    ಯದಿ( pX->pWin ){
      /* This optimization does not work for subqueries that use window
      ** functions. */
      ಪ್ರತ್ಯಾಗಮ 0;
    }
#endif
  }
  colUsed = pItem->colUsed;
  ಯದಿ( pSub->pOrderBy ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pList = pSub->pOrderBy;
    ಪ್ರತಿ(j=0; j<pList->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
      u16 iCol = pList->a[j].u.x.iOrderByCol;
      ಯದಿ( iCol>0 ){
        iCol--;
        colUsed |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
      }
    }
  }
  nCol = pTab->nCol;
  ಪ್ರತಿ(j=0; j<nCol; j++){
    Bitmask m = j<BMS-1 ? MASKBIT(j) : TOPBIT;
    ಯದಿ( (m & colUsed)!=0 ) ಅನುವರ್ತ;
    ಪ್ರತಿ(pX=pSub; pX; pX=pX->pPrior) {
      ಅಭಿವ್ಯಕ್ತಿ *pY = pX->pEList->a[j].p_ಅಭಿವ್ಯಕ್ತಿ;
      ಯದಿ( pY->op==TK_NULL ) ಅನುವರ್ತ;
      pY->op = TK_NULL;
      ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(pY, EP_Skip|EP_Unlikely);
      pX->selFlags |= SF_ತಳ್ಳು_Down;
      nChng++;
    }
  }
  ಪ್ರತ್ಯಾಗಮ nChng;
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
ಸ್ಥಿರ u8 minMaxQuery(sqlite3 *db, ಅಭಿವ್ಯಕ್ತಿ *pFunc, ಅಭಿವ್ಯಕ್ತಿ_List **ppMinMax){
  ಪೂರ್ಣಾಙ್ಕ eRet = WHERE_ORDERBY_NORMAL;      /* Return value */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;                     /* Arguments to agg function */
  ಧ್ರುವ ವರ್ಣ *zFunc;                    /* Name of aggregate function pFunc */
  ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy;
  u8 sortFlags = 0;

  ಪ್ರತಿಜ್ಞಾ( *ppMinMax==0 );
  ಪ್ರತಿಜ್ಞಾ( pFunc->op==TK_AGG_FUNCTION );
  ಪ್ರತಿಜ್ಞಾ( !IsWindowFunc(pFunc) );
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pFunc) );
  pEList = pFunc->x.pList;
  ಯದಿ( pEList==0
   || pEList->n_ಅಭಿವ್ಯಕ್ತಿ!=1
   || ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pFunc, EP_WinFunc)
   || OptimizationDisabled(db, SQLITE_MinMaxOpt)
  ){
    ಪ್ರತ್ಯಾಗಮ eRet;
  }
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pFunc, EP_Int_ಮೌಲ್ಯ) );
  zFunc = pFunc->u.zToken;
  ಯದಿ( sqlite3StrICmp(zFunc, "min")==0 ){
    eRet = WHERE_ORDERBY_MIN;
    ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ) ){
      sortFlags = KEYINFO_ORDER_BIGNULL;
    }
  }ಅನ್ಯಥಾ ಯದಿ( sqlite3StrICmp(zFunc, "max")==0 ){
    eRet = WHERE_ORDERBY_MAX;
    sortFlags = KEYINFO_ORDER_DESC;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ eRet;
  }
  *ppMinMax = pOrderBy = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pEList, 0);
  ಪ್ರತಿಜ್ಞಾ( pOrderBy!=0 || db->mallocFailed );
  ಯದಿ( pOrderBy ) pOrderBy->a[0].fg.sortFlags = sortFlags;
  ಪ್ರತ್ಯಾಗಮ eRet;
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
ಸ್ಥಿರ Table *is_Simple_ಎಣಿಕೆ(Select *p, AggInfo *pAggInfo){
  Table *pTab;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ;

  ಪ್ರತಿಜ್ಞಾ( !p->pGroupBy );

  ಯದಿ( p->pWhere
   || p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ!=1
   || p->pSrc->nSrc!=1
   || p->pSrc->a[0].fg.isSubquery
   || pAggInfo->nFunc!=1
   || p->pHaving
  ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  pTab = p->pSrc->a[0].pSTab;
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( !IsView(pTab) );
  ಯದಿ( !IsOrdinaryTable(pTab) ) ಪ್ರತ್ಯಾಗಮ 0;
  p_ಅಭಿವ್ಯಕ್ತಿ = p->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_AGG_FUNCTION ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo!=pAggInfo ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( (pAggInfo->aFunc[0].pFunc->funcFlags&SQLITE_FUNC_ಎಣಿಕೆ)==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತಿಜ್ಞಾ( pAggInfo->aFunc[0].pFExpr==p_ಅಭಿವ್ಯಕ್ತಿ );
  testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Distinct) );
  testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) );
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Distinct|EP_WinFunc) ) ಪ್ರತ್ಯಾಗಮ 0;

  ಪ್ರತ್ಯಾಗಮ pTab;
}

/*
** If the source-list item passed as an argument was augmented with an
** INDEXED BY clause, then try to locate the specified index. If there
** was such a clause and the named index cannot be found, return
** SQLITE_ERROR and leave an error in pParse. Otherwise, populate
** pFrom->pIndex and return SQLITE_OK.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3IndexedByLookup(Parse *pParse, SrcItem *pFrom){
  Table *pTab = pFrom->pSTab;
  ವರ್ಣ *zIndexedBy = pFrom->u1.zIndexedBy;
  Index *pIdx;
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isIndexedBy!=0 );

  ಪ್ರತಿ(pIdx=pTab->pIndex;
      pIdx && sqlite3StrICmp(pIdx->zName, zIndexedBy);
      pIdx=pIdx->pNext
  );
  ಯದಿ( !pIdx ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "no such index: %s", zIndexedBy, 0);
    pParse->checkSchema = 1;
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }
  ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isCte==0 );
  pFrom->u2.pIBIndex = pIdx;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ convertCompoundSelectToSubquery(Walker *pWalker, Select *p){
  ಪೂರ್ಣಾಙ್ಕ i;
  Select *pNew;
  Select *pX;
  sqlite3 *db;
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *a;
  SrcList *pNewSrc;
  Parse *pParse;
  Token dummy;

  ಯದಿ( p->pPrior==0 ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  ಯದಿ( p->pOrderBy==0 ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  ಪ್ರತಿ(pX=p; pX && (pX->op==TK_ALL || pX->op==TK_SELECT); pX=pX->pPrior){}
  ಯದಿ( pX==0 ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  a = p->pOrderBy->a;
#ifndef SQLITE_OMIT_WINDOWFUNC
  /* If iOrderByCol is already non-zero, then it has already been matched
  ** to a result column of the SELECT statement. This occurs when the
  ** SELECT is rewritten for window-functions processing and then passed
  ** to sqlite3SelectPrep() and similar a second time. The rewriting done
  ** by this function is not required in this case. */
  ಯದಿ( a[0].u.x.iOrderByCol ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
#endif
  ಪ್ರತಿ(i=p->pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ-1; i>=0; i--){
    ಯದಿ( a[i].p_ಅಭಿವ್ಯಕ್ತಿ->flags & EP_Collate ) ಭಙ್ಗ;
  }
  ಯದಿ( i<0 ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;

  /* If we reach this point, that means the transformation is required. */

  pParse = pWalker->pParse;
  db = pParse->db;
  pNew = sqlite3DbMallocZero(db, ಪರಿಮಾಣ(*pNew) );
  ಯದಿ( pNew==0 ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  memset(&dummy, 0, ಪರಿಮಾಣ(dummy));
  pNewSrc = sqlite3SrcListAppendFromTerm(pParse,0,0,0,&dummy,pNew,0);
  ಪ್ರತಿಜ್ಞಾ( pNewSrc!=0 || pParse->nErr );
  ಯದಿ( pParse->nErr ){
    sqlite3SrcListDelete(db, pNewSrc);
    ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  }
  *pNew = *p;
  p->pSrc = pNewSrc;
  p->pEList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, 0, sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ASTERISK, 0));
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
  ಪ್ರತಿಜ್ಞಾ( (p->selFlags & SF_Converted)==0 );
  p->selFlags |= SF_Converted;
  ಪ್ರತಿಜ್ಞಾ( pNew->pPrior!=0 );
  pNew->pPrior->pNext = pNew;
  pNew->pLimit = 0;
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}

/*
** Check to see if the FROM clause term pFrom has table-valued function
** arguments.  If it does, leave an error message in pParse and return
** non-zero, since pFrom is not allowed to be a table-valued function.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cannotBeFunction(Parse *pParse, SrcItem *pFrom){
  ಯದಿ( pFrom->fg.isTabFunc ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "'%s' is not a function", pFrom->zName);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಸಂರಚನಾ Cte *ಹುಡುಕು_With(
  With *pWith,                    /* Current innermost WITH clause */
  SrcItem *pItem,                 /* FROM clause element to resolve */
  With **ppContext                /* OUT: WITH clause return value belongs to */
){
  ಧ್ರುವ ವರ್ಣ *zName = pItem->zName;
  With *p;
  ಪ್ರತಿಜ್ಞಾ( pItem->fg.fixedSchema || pItem->u4.zDatabase==0 );
  ಪ್ರತಿಜ್ಞಾ( zName!=0 );
  ಪ್ರತಿ(p=pWith; p; p=p->pOuter){
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<p->nCte; i++){
      ಯದಿ( sqlite3StrICmp(zName, p->a[i].zName)==0 ){
        *ppContext = p;
        ಪ್ರತ್ಯಾಗಮ &p->a[i];
      }
    }
    ಯದಿ( p->bView ) ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
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
With *sqlite3_With_ತಳ್ಳು(Parse *pParse, With *pWith, u8 bFree){
  ಯದಿ( pWith ){
    ಯದಿ( bFree ){
      pWith = (With*)sqlite3ParserAddCleanup(pParse, sqlite3WithDeleteGeneric,
                      pWith);
      ಯದಿ( pWith==0 ) ಪ್ರತ್ಯಾಗಮ 0;
    }
    ಯದಿ( pParse->nErr==0 ){
      ಪ್ರತಿಜ್ಞಾ( pParse->pWith!=pWith );
      pWith->pOuter = pParse->pWith;
      pParse->pWith = pWith;
    }
  }
  ಪ್ರತ್ಯಾಗಮ pWith;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ selectExpanderWalk(Walker*, Select*);

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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ resolveFromTermToCte(
  Parse *pParse,                  /* The parsing context */
  Walker *pWalker,                /* Current tree walker */
  SrcItem *pFrom                  /* The FROM clause term to check */
){
  Cte *pCte;               /* Matched CTE (or NULL if no match) */
  With *pWith;             /* The matching WITH */

  ಪ್ರತಿಜ್ಞಾ( pFrom->pSTab==0 );
  ಯದಿ( pParse->pWith==0 ){
    /* There are no WITH clauses in the stack.  No match is possible */
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ( pParse->nErr ){
    /* Prior errors might have left pParse->pWith in a goofy state, so
    ** go no further. */
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತಿಜ್ಞಾ( pFrom->fg.hadSchema==0 || pFrom->fg.notCte!=0 );
  ಯದಿ( pFrom->fg.fixedSchema==0 && pFrom->u4.zDatabase!=0 ){
    /* The FROM term contains a schema qualifier (ex: main.t1) and so
    ** it cannot possibly be a CTE reference. */
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ( pFrom->fg.notCte ){
    /* The FROM term is specifically excluded from matching a CTE.
    **   (1)  It is part of a trigger that used to have zDatabase but had
    **        zDatabase removed by sqlite3FixTriggerStep().
    **   (2)  This is the first term in the FROM clause of an UPDATE.
    */
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  pCte = ಹುಡುಕು_With(pParse->pWith, pFrom, &pWith);
  ಯದಿ( pCte ){
    sqlite3 *db = pParse->db;
    Table *pTab;
    ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
    Select *pSel;
    Select *p_ಎಡ;                /* Left-most SELECT statement */
    Select *pRecTerm;             /* Left-most recursive term */
    ಪೂರ್ಣಾಙ್ಕ bMayRecursive;            /* True if compound joined by UNION [ALL] */
    With *pSavedWith;             /* Initial value of pParse->pWith */
    ಪೂರ್ಣಾಙ್ಕ iRecTab = -1;             /* Cursor for recursive table */
    CteUse *pCteUse;

    /* If pCte->zCteErr is non-NULL at this point, then this is an illegal
    ** recursive reference to CTE pCte. Leave an error in pParse and return
    ** early. If pCte->zCteErr is NULL, then this is not a recursive reference.
    ** In this case, proceed.  */
    ಯದಿ( pCte->zCteErr ){
      sqlite3_Error_ಸಂದೇಶ(pParse, pCte->zCteErr, pCte->zName);
      ಪ್ರತ್ಯಾಗಮ 2;
    }
    ಯದಿ( cannotBeFunction(pParse, pFrom) ) ಪ್ರತ್ಯಾಗಮ 2;

    ಪ್ರತಿಜ್ಞಾ( pFrom->pSTab==0 );
    pTab = sqlite3DbMallocZero(db, ಪರಿಮಾಣ(Table));
    ಯದಿ( pTab==0 ) ಪ್ರತ್ಯಾಗಮ 2;
    pCteUse = pCte->pUse;
    ಯದಿ( pCteUse==0 ){
      pCte->pUse = pCteUse = sqlite3DbMallocZero(db, ಪರಿಮಾಣ(pCteUse[0]));
      ಯದಿ( pCteUse==0
       || sqlite3ParserAddCleanup(pParse,sqlite3DbFree,pCteUse)==0
      ){
        sqlite3DbFree(db, pTab);
        ಪ್ರತ್ಯಾಗಮ 2;
      }
      pCteUse->eM10d = pCte->eM10d;
    }
    pFrom->pSTab = pTab;
    pTab->nTabRef = 1;
    pTab->zName = sqlite3DbStrDup(db, pCte->zName);
    pTab->iPKey = -1;
    pTab->nRowLogEst = 200; ಪ್ರತಿಜ್ಞಾ( 200==sqlite3LogEst(1048576) );
    pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
    sqlite3SrcItemAttachSubquery(pParse, pFrom, pCte->pSelect, 1);
    ಯದಿ( db->mallocFailed ) ಪ್ರತ್ಯಾಗಮ 2;
    ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isSubquery && pFrom->u4.pSubq );
    pSel = pFrom->u4.pSubq->pSelect;
    ಪ್ರತಿಜ್ಞಾ( pSel!=0 );
    pSel->selFlags |= SF_CopyCte;
    ಯದಿ( pFrom->fg.isIndexedBy ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "no such index: \"%s\"", pFrom->u1.zIndexedBy);
      ಪ್ರತ್ಯಾಗಮ 2;
    }
    ಪ್ರತಿಜ್ಞಾ( !pFrom->fg.isIndexedBy );
    pFrom->fg.isCte = 1;
    pFrom->u2.pCteUse = pCteUse;
    pCteUse->nUse++;

    /* Check if this is a recursive CTE. */
    pRecTerm = pSel;
    bMayRecursive = ( pSel->op==TK_ALL || pSel->op==TK_UNION );
    ಯಾವತ್( bMayRecursive && pRecTerm->op==pSel->op ){
      ಪೂರ್ಣಾಙ್ಕ i;
      SrcList *pSrc = pRecTerm->pSrc;
      ಪ್ರತಿಜ್ಞಾ( pRecTerm->pPrior!=0 );
      ಪ್ರತಿ(i=0; i<pSrc->nSrc; i++){
        SrcItem *pItem = &pSrc->a[i];
        ಯದಿ( pItem->zName!=0
         && !pItem->fg.hadSchema
         && ALWAYS( !pItem->fg.isSubquery )
         && (pItem->fg.fixedSchema || pItem->u4.zDatabase==0)
         && 0==sqlite3StrICmp(pItem->zName, pCte->zName)
        ){
          pItem->pSTab = pTab;
          pTab->nTabRef++;
          pItem->fg.isRecursive = 1;
          ಯದಿ( pRecTerm->selFlags & SF_Recursive ){
            sqlite3_Error_ಸಂದೇಶ(pParse,
               "multiple references to recursive table: %s", pCte->zName
            );
            ಪ್ರತ್ಯಾಗಮ 2;
          }
          pRecTerm->selFlags |= SF_Recursive;
          ಯದಿ( iRecTab<0 ) iRecTab = pParse->nTab++;
          pItem->iCursor = iRecTab;
        }
      }
      ಯದಿ( (pRecTerm->selFlags & SF_Recursive)==0 ) ಭಙ್ಗ;
      pRecTerm = pRecTerm->pPrior;
    }

    ಯದಿ( pParse->nTab >= db->aLimit[SQLITE_LIMIT_CTES]
     && db->aLimit[SQLITE_LIMIT_CTES]>0
    ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "too many CTEs");
      ಪ್ರತ್ಯಾಗಮ 2;
    }
#ಯದಿ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_DEPTH>0
    pParse->nNestSel++;
    ಯದಿ( checkSubqueryNestingDepth(pParse) ) ಪ್ರತ್ಯಾಗಮ 2;
#endif
    pCte->zCteErr = "circular reference: %s";
    pSavedWith = pParse->pWith;
    pParse->pWith = pWith;
    ಯದಿ( pSel->selFlags & SF_Recursive ){
      ಪೂರ್ಣಾಙ್ಕ rc;
      ಪ್ರತಿಜ್ಞಾ( pRecTerm!=0 );
      ಪ್ರತಿಜ್ಞಾ( (pRecTerm->selFlags & SF_Recursive)==0 );
      ಪ್ರತಿಜ್ಞಾ( pRecTerm->pNext!=0 );
      ಪ್ರತಿಜ್ಞಾ( (pRecTerm->pNext->selFlags & SF_Recursive)!=0 );
      ಪ್ರತಿಜ್ಞಾ( pRecTerm->pWith==0 );
      pRecTerm->pWith = pSel->pWith;
      rc = sqlite3WalkSelect(pWalker, pRecTerm);
      pRecTerm->pWith = 0;
      ಯದಿ( rc ){
        pParse->pWith = pSavedWith;
        ಪ್ರತ್ಯಾಗಮ 2;
      }
    }ಅನ್ಯಥಾ{
      ಯದಿ( selectExpanderWalk(pWalker, pSel) ){
        pParse->pWith = pSavedWith;
        ಪ್ರತ್ಯಾಗಮ 2;
      }
    }
    pParse->pWith = pWith;
#ಯದಿ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_DEPTH>0
    pParse->nNestSel--;
    ಪ್ರತಿಜ್ಞಾ( pParse->nNestSel>=0 );
#endif

    ಪ್ರತಿ(p_ಎಡ=pSel; p_ಎಡ->pPrior; p_ಎಡ=p_ಎಡ->pPrior);
    pEList = p_ಎಡ->pEList;
    ಯದಿ( pCte->pCols ){
      ಯದಿ( pEList && pEList->n_ಅಭಿವ್ಯಕ್ತಿ!=pCte->pCols->n_ಅಭಿವ್ಯಕ್ತಿ ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "table %s has %d values for %d columns",
            pCte->zName, pEList->n_ಅಭಿವ್ಯಕ್ತಿ, pCte->pCols->n_ಅಭಿವ್ಯಕ್ತಿ
        );
        pParse->pWith = pSavedWith;
        ಪ್ರತ್ಯಾಗಮ 2;
      }
      pEList = pCte->pCols;
    }

    sqlite3_Columns_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pEList, &pTab->nCol, &pTab->aCol);
    ಯದಿ( bMayRecursive ){
      ಯದಿ( pSel->selFlags & SF_Recursive ){
        pCte->zCteErr = "multiple recursive references: %s";
      }ಅನ್ಯಥಾ{
        pCte->zCteErr = "recursive reference in a subquery: %s";
      }
      sqlite3WalkSelect(pWalker, pSel);
    }
    pCte->zCteErr = 0;
    pParse->pWith = pSavedWith;
    ಪ್ರತ್ಯಾಗಮ 1;  /* Success */
  }
  ಪ್ರತ್ಯಾಗಮ 0;  /* No match */
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
ಶೂನ್ಯ sqlite3_Select_ತೆಗೆ_With(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  ಯದಿ( OK_IF_ALWAYS_TRUE(pParse->pWith) && p->pPrior==0 ){
    With *pWith = findRightmost(p)->pWith;
    ಯದಿ( pWith!=0 ){
      ಪ್ರತಿಜ್ಞಾ( pParse->pWith==pWith || pParse->nErr );
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
ಪೂರ್ಣಾಙ್ಕ sqlite3ExpandSubquery(Parse *pParse, SrcItem *pFrom){
  Select *pSel;
  Table *pTab;

  ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isSubquery );
  ಪ್ರತಿಜ್ಞಾ( pFrom->u4.pSubq!=0 );
  pSel = pFrom->u4.pSubq->pSelect;
  ಪ್ರತಿಜ್ಞಾ( pSel );
  pFrom->pSTab = pTab = sqlite3DbMallocZero(pParse->db, ಪರಿಮಾಣ(Table));
  ಯದಿ( pTab==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
  pTab->nTabRef = 1;
  ಯದಿ( pFrom->zAlias ){
    pTab->zName = sqlite3DbStrDup(pParse->db, pFrom->zAlias);
  }ಅನ್ಯಥಾ{
    pTab->zName = sqlite3MPrintf(pParse->db, "%!S", pFrom);
  }
  ಯಾವತ್( pSel->pPrior ){ pSel = pSel->pPrior; }
  sqlite3_Columns_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pSel->pEList,&pTab->nCol,&pTab->aCol);
  pTab->iPKey = -1;
  pTab->eTabType = TABTYP_VIEW;
  pTab->nRowLogEst = 200; ಪ್ರತಿಜ್ಞಾ( 200==sqlite3LogEst(1048576) );
#ifndef SQLITE_ALLOW_ROWID_IN_VIEW
  /* The usual case - do not allow ROWID on a subquery */
  pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
#ಅನ್ಯಥಾ
  /* Legacy compatibility mode */
  pTab->tabFlags |= TF_Ephemeral | sqlite3Config.mNoVisibleRowid;
#endif
  ಪ್ರತ್ಯಾಗಮ pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ inAnyUsingClause(
  ಧ್ರುವ ವರ್ಣ *zName, /* Name we are looking for */
  SrcItem *pBase,    /* The base SrcItem.  Looking at pBase[1] and following */
  ಪೂರ್ಣಾಙ್ಕ N              /* How many SrcItems to check */
){
  ಯಾವತ್( N>0 ){
    N--;
    pBase++;
    ಯದಿ( pBase->fg.isUsing==0 ) ಅನುವರ್ತ;
    ಯದಿ( NEVER(pBase->u3.pUsing==0) ) ಅನುವರ್ತ;
    ಯದಿ( sqlite3IdListIndex(pBase->u3.pUsing, zName)>=0 ) ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ selectExpander(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  ಪೂರ್ಣಾಙ್ಕ i, j, k, rc;
  SrcList *pTabList;
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
  SrcItem *pFrom;
  sqlite3 *db = pParse->db;
  ಅಭಿವ್ಯಕ್ತಿ *pE, *p_ಬಲ, *p_ಅಭಿವ್ಯಕ್ತಿ;
  u16 selFlags = p->selFlags;
  u32 elistFlags = 0;

  p->selFlags |= SF_Expanded;
  ಯದಿ( db->mallocFailed  ){
    ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  }
  ಪ್ರತಿಜ್ಞಾ( p->pSrc!=0 );
  ಯದಿ( (selFlags & SF_Expanded)!=0 ){
    ಪ್ರತ್ಯಾಗಮ WRC_Prune;
  }
  ಯದಿ( pWalker->eCode ){
    /* Renumber selId because it has been copied from a view */
    p->selId = ++pParse->nSelect;
  }
  pTabList = p->pSrc;
  pEList = p->pEList;
  ಯದಿ( pParse->pWith && (p->selFlags & SF_View) ){
    ಯದಿ( p->pWith==0 ){
      p->pWith = (With*)sqlite3DbMallocZero(db, SZ_WITH(1) );
      ಯದಿ( p->pWith==0 ){
        ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }
    }
    p->pWith->bView = 1;
  }
  sqlite3_With_ತಳ್ಳು(pParse, p->pWith, 0);

  /* Make sure cursor numbers have been assigned to all entries in
  ** the FROM clause of the SELECT statement.
  */
  sqlite3SrcListAssignCursors(pParse, pTabList);

  /* Look up every table named in the FROM clause of the select.  If
  ** an entry of the FROM clause is a subquery instead of a table or view,
  ** then create a transient table structure to describe the subquery.
  */
  ಪ್ರತಿ(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    Table *pTab;
    ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isRecursive==0 || pFrom->pSTab!=0 );
    ಯದಿ( pFrom->pSTab ) ಅನುವರ್ತ;
    ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isRecursive==0 );
    ಯದಿ( pFrom->zName==0 ){
#ifndef SQLITE_OMIT_SUBQUERY
      Select *pSel;
      ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isSubquery && pFrom->u4.pSubq!=0 );
      pSel = pFrom->u4.pSubq->pSelect;
      /* A sub-query in the FROM clause of a SELECT */
      ಪ್ರತಿಜ್ಞಾ( pSel!=0 );
      ಪ್ರತಿಜ್ಞಾ( pFrom->pSTab==0 );
      ಯದಿ( sqlite3WalkSelect(pWalker, pSel) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      ಯದಿ( sqlite3ExpandSubquery(pParse, pFrom) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
#endif
#ifndef SQLITE_OMIT_CTE
    }ಅನ್ಯಥಾ ಯದಿ( (rc = resolveFromTermToCte(pParse, pWalker, pFrom))!=0 ){
      ಯದಿ( rc>1 ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      pTab = pFrom->pSTab;
      ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
#endif
    }ಅನ್ಯಥಾ{
      /* An ordinary table or view name in the FROM clause */
      ಪ್ರತಿಜ್ಞಾ( pFrom->pSTab==0 );
      pFrom->pSTab = pTab = sqlite3LocateTableItem(pParse, 0, pFrom);
      ಯದಿ( pTab==0 ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      ಯದಿ( pTab->nTabRef>=0xffff ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "too many references to \"%s\": max 65535",
           pTab->zName);
        pFrom->pSTab = 0;
        ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }
      pTab->nTabRef++;
      ಯದಿ( !IsVirtual(pTab) && cannotBeFunction(pParse, pFrom) ){
        ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }
#ಯದಿ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
      ಯದಿ( !IsOrdinaryTable(pTab) ){
        i16 nCol;
        u8 eCodeOrig = pWalker->eCode;
        ಯದಿ( sqlite3ViewGetColumnNames(pParse, pTab) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
        ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isSubquery==0 );
        ಯದಿ( IsView(pTab) ){
          ಯದಿ( (db->flags & SQLITE_EnableView)==0
           && pTab->pSchema!=db->aDb[1].pSchema
          ){
            sqlite3_Error_ಸಂದೇಶ(pParse, "access to view \"%s\" prohibited",
              pTab->zName);
          }
          sqlite3SrcItemAttachSubquery(pParse, pFrom, pTab->u.view.pSelect, 1);
        }
#ifndef SQLITE_OMIT_VIRTUALTABLE
        ಅನ್ಯಥಾ ಯದಿ( ALWAYS(IsVirtual(pTab))
         && (pFrom->fg.fromDDL || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
         && ALWAYS(pTab->u.vtab.p!=0)
         && pTab->u.vtab.p->eVtabRisk > ((db->flags & SQLITE_TrustedSchema)!=0)
        ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "unsafe use of virtual table \"%s\"",
                                  pTab->zName);
        }
        ಪ್ರತಿಜ್ಞಾ( SQLITE_VTABRISK_Normal==1 && SQLITE_VTABRISK_ಮೇಲಿನ==2 );
#endif
        nCol = pTab->nCol;
        pTab->nCol = -1;
        pWalker->eCode = 1;  /* Turn on Select.selId renumbering */
        ಯದಿ( pFrom->fg.isSubquery ){
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
    ಯದಿ( pFrom->fg.isIndexedBy && sqlite3IndexedByLookup(pParse, pFrom) ){
      ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    }
  }

  /* Process NATURAL keywords, and ON and USING clauses of joins.
  */
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 || pParse->nErr!=0 );
  ಯದಿ( pParse->nErr || sqlite3ProcessJoin(pParse, p) ){
    ಪ್ರತ್ಯಾಗಮ WRC_Abort;
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
  ಪ್ರತಿ(k=0; k<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; k++){
    pE = pEList->a[k].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಯದಿ( pE->op==TK_ASTERISK ) ಭಙ್ಗ;
    ಪ್ರತಿಜ್ಞಾ( pE->op!=TK_DOT || pE->p_ಬಲ!=0 );
    ಪ್ರತಿಜ್ಞಾ( pE->op!=TK_DOT || (pE->p_ಎಡ!=0 && pE->p_ಎಡ->op==TK_ID) );
    ಯದಿ( pE->op==TK_DOT && pE->p_ಬಲ->op==TK_ASTERISK ) ಭಙ್ಗ;
    elistFlags |= pE->flags;
  }
  ಯದಿ( k<pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
    /*
    ** If we get here it means the result set contains one or more "*"
    ** operators that need to be expanded.  Loop through each expression
    ** in the result set and expand them one by one.
    */
    ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *a = pEList->a;
    ಅಭಿವ್ಯಕ್ತಿ_List *pNew = 0;
    ಪೂರ್ಣಾಙ್ಕ flags = pParse->db->flags;
    ಪೂರ್ಣಾಙ್ಕ longNames = (flags & SQLITE_FullColNames)!=0
                      && (flags & SQLITE_ShortColNames)==0;

    ಪ್ರತಿ(k=0; k<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; k++){
      pE = a[k].p_ಅಭಿವ್ಯಕ್ತಿ;
      elistFlags |= pE->flags;
      p_ಬಲ = pE->p_ಬಲ;
      ಪ್ರತಿಜ್ಞಾ( pE->op!=TK_DOT || p_ಬಲ!=0 );
      ಯದಿ( pE->op!=TK_ASTERISK
       && (pE->op!=TK_DOT || p_ಬಲ->op!=TK_ASTERISK)
      ){
        /* This particular expression does not need to be expanded.
        */
        pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pNew, a[k].p_ಅಭಿವ್ಯಕ್ತಿ);
        ಯದಿ( pNew ){
          pNew->a[pNew->n_ಅಭಿವ್ಯಕ್ತಿ-1].zEName = a[k].zEName;
          pNew->a[pNew->n_ಅಭಿವ್ಯಕ್ತಿ-1].fg.eEName = a[k].fg.eEName;
          a[k].zEName = 0;
        }
        a[k].p_ಅಭಿವ್ಯಕ್ತಿ = 0;
      }ಅನ್ಯಥಾ{
        /* This expression is a "*" or a "TABLE.*" and needs to be
        ** expanded. */
        ಪೂರ್ಣಾಙ್ಕ tableSeen = 0;      /* Set to 1 when TABLE matches */
        ವರ್ಣ *zTName = 0;       /* text of name of TABLE */
        ಪೂರ್ಣಾಙ್ಕ iErrOfst;
        ಯದಿ( pE->op==TK_DOT ){
          ಪ್ರತಿಜ್ಞಾ( (selFlags & SF_NestedFrom)==0 );
          ಪ್ರತಿಜ್ಞಾ( pE->p_ಎಡ!=0 );
          ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pE->p_ಎಡ, EP_Int_ಮೌಲ್ಯ) );
          zTName = pE->p_ಎಡ->u.zToken;
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_WOfst(pE->p_ಎಡ) );
          iErrOfst = pE->p_ಬಲ->w.iOfst;
        }ಅನ್ಯಥಾ{
          ಪ್ರತಿಜ್ಞಾ( pE->p_ಎಡ==0 );
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_WOfst(pE) );
          iErrOfst = pE->w.iOfst;
        }
        ಪ್ರತಿ(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
          ಪೂರ್ಣಾಙ್ಕ nAdd;                    /* Number of cols including rowid */
          Table *pTab = pFrom->pSTab;  /* Table for this data source */
          ಅಭಿವ್ಯಕ್ತಿ_List *pNestedFrom;       /* Result-set of a nested FROM clause */
          ವರ್ಣ *zTabName;              /* AS name for this data source */
          ಧ್ರುವ ವರ್ಣ *zSchemaName = 0; /* Schema name for this data source */
          ಪೂರ್ಣಾಙ್ಕ iDb;                     /* Schema index for this data src */
          IdList *pUsing;              /* USING clause for pFrom[1] */

          ಯದಿ( (zTabName = pFrom->zAlias)==0 ){
            zTabName = pTab->zName;
          }
          ಯದಿ( db->mallocFailed ) ಭಙ್ಗ;
          ಪ್ರತಿಜ್ಞಾ( (ಪೂರ್ಣಾಙ್ಕ)pFrom->fg.isNestedFrom == IsNestedFrom(pFrom) );
          ಯದಿ( pFrom->fg.isNestedFrom ){
            ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isSubquery && pFrom->u4.pSubq );
            ಪ್ರತಿಜ್ಞಾ( pFrom->u4.pSubq->pSelect!=0 );
            pNestedFrom = pFrom->u4.pSubq->pSelect->pEList;
            ಪ್ರತಿಜ್ಞಾ( pNestedFrom!=0 );
            ಪ್ರತಿಜ್ಞಾ( pNestedFrom->n_ಅಭಿವ್ಯಕ್ತಿ==pTab->nCol );
            ಪ್ರತಿಜ್ಞಾ( VisibleRowid(pTab)==0 || ViewCanHaveRowid );
          }ಅನ್ಯಥಾ{
            ಯದಿ( zTName && sqlite3StrICmp(zTName, zTabName)!=0 ){
              ಅನುವರ್ತ;
            }
            pNestedFrom = 0;
            iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
            zSchemaName = iDb>=0 ? db->aDb[iDb].zDbSName : "*";
          }
          ಯದಿ( i+1<pTabList->nSrc
           && pFrom[1].fg.isUsing
           && (selFlags & SF_NestedFrom)!=0
          ){
            ಪೂರ್ಣಾಙ್ಕ ii;
            pUsing = pFrom[1].u3.pUsing;
            ಪ್ರತಿ(ii=0; ii<pUsing->nId; ii++){
              ಧ್ರುವ ವರ್ಣ *zUName = pUsing->a[ii].zName;
              p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, zUName);
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Set_Error_Offset(p_ಬಲ, iErrOfst);
              pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pNew, p_ಬಲ);
              ಯದಿ( pNew ){
                ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pX = &pNew->a[pNew->n_ಅಭಿವ್ಯಕ್ತಿ-1];
                ಪ್ರತಿಜ್ಞಾ( pX->zEName==0 );
                pX->zEName = sqlite3MPrintf(db,"..%s", zUName);
                pX->fg.eEName = ENAME_TAB;
                pX->fg.bUsingTerm = 1;
              }
            }
          }ಅನ್ಯಥಾ{
            pUsing = 0;
          }

          nAdd = pTab->nCol;
          ಯದಿ( VisibleRowid(pTab) && (selFlags & SF_NestedFrom)!=0 ) nAdd++;
          ಪ್ರತಿ(j=0; j<nAdd; j++){
            ಧ್ರುವ ವರ್ಣ *zName; 
            ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pX; /* Newly added ExprList term */

            ಯದಿ( j==pTab->nCol ){
              zName = sqlite3RowidAlias(pTab);
              ಯದಿ( zName==0 ) ಅನುವರ್ತ;
            }ಅನ್ಯಥಾ{
              zName = pTab->aCol[j].zCnName;

              /* If pTab is actually an SF_NestedFrom sub-select, do not
              ** expand any ENAME_ROWID columns.  */
              ಯದಿ( pNestedFrom && pNestedFrom->a[j].fg.eEName==ENAME_ROWID ){
                ಅನುವರ್ತ;
              }

              ಯದಿ( zTName
               && pNestedFrom
               && sqlite3MatchEName(&pNestedFrom->a[j], 0, zTName, 0, 0)==0
              ){
                ಅನುವರ್ತ;
              }

              /* If a column is marked as 'hidden', omit it from the expanded
              ** result-set list unless the SELECT has the SF_IncludeHidden
              ** bit set.
              */
              ಯದಿ( (p->selFlags & SF_IncludeHidden)==0
                && IsHiddenColumn(&pTab->aCol[j])
              ){
                ಅನುವರ್ತ;
              }
              ಯದಿ( (pTab->aCol[j].colFlags & COLFLAG_NOEXPAND)!=0
               && zTName==0
               && (selFlags & (SF_NestedFrom))==0
              ){
                ಅನುವರ್ತ;
              }
            }
            ಪ್ರತಿಜ್ಞಾ( zName );
            tableSeen = 1;

            ಯದಿ( i>0 && zTName==0 && (selFlags & SF_NestedFrom)==0 ){
              ಯದಿ( pFrom->fg.isUsing
               && sqlite3IdListIndex(pFrom->u3.pUsing, zName)>=0
              ){
                /* In a join with a USING clause, omit columns in the
                ** using clause from the table on the right. */
                ಅನುವರ್ತ;
              }
            }
            p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, zName);
            ಯದಿ( (pTabList->nSrc>1
                 && (  (pFrom->fg.jointype & JT_LTORJ)==0
                     || zTName!=0
                     || (selFlags & SF_NestedFrom)!=0
                     || !inAnyUsingClause(zName,pFrom,pTabList->nSrc-i-1)
                    )
                )
             || IN_RENAME_OBJECT
            ){
              ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ;
              p_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, zTabName);
              p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3PExpr(pParse, TK_DOT, p_ಎಡ, p_ಬಲ);
              ಯದಿ( IN_RENAME_OBJECT && pE->p_ಎಡ ){
                ಪ್ರತಿಜ್ಞಾ( pE->op==TK_DOT && pE->p_ಬಲ->op==TK_ASTERISK );
                sqlite3RenameTokenRemap(pParse, p_ಎಡ, pE->p_ಎಡ);
              }
              ಯದಿ( zSchemaName ){
                p_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, zSchemaName);
                p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3PExpr(pParse, TK_DOT, p_ಎಡ, p_ಅಭಿವ್ಯಕ್ತಿ);
              }
            }ಅನ್ಯಥಾ{
              p_ಅಭಿವ್ಯಕ್ತಿ = p_ಬಲ;
            }
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Set_Error_Offset(p_ಅಭಿವ್ಯಕ್ತಿ, iErrOfst);
            pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pNew, p_ಅಭಿವ್ಯಕ್ತಿ);
            ಯದಿ( pNew==0 ){
              ಭಙ್ಗ;  /* OOM */
            }
            pX = &pNew->a[pNew->n_ಅಭಿವ್ಯಕ್ತಿ-1];
            ಪ್ರತಿಜ್ಞಾ( pX->zEName==0 );
            ಯದಿ( (selFlags & SF_NestedFrom)!=0 && !IN_RENAME_OBJECT ){
              ಯದಿ( pNestedFrom && (!ViewCanHaveRowid || j<pNestedFrom->n_ಅಭಿವ್ಯಕ್ತಿ) ){
                ಪ್ರತಿಜ್ಞಾ( j<pNestedFrom->n_ಅಭಿವ್ಯಕ್ತಿ );
                pX->zEName = sqlite3DbStrDup(db, pNestedFrom->a[j].zEName);
                testcase( pX->zEName==0 );
              }ಅನ್ಯಥಾ{
                pX->zEName = sqlite3MPrintf(db, "%s.%s.%s",
                                           zSchemaName, zTabName, zName);
                testcase( pX->zEName==0 );
              }
              pX->fg.eEName = (j==pTab->nCol ? ENAME_ROWID : ENAME_TAB);
              ಯದಿ( (pFrom->fg.isUsing
                   && sqlite3IdListIndex(pFrom->u3.pUsing, zName)>=0)
               || (pUsing && sqlite3IdListIndex(pUsing, zName)>=0)
               || (j<pTab->nCol && (pTab->aCol[j].colFlags & COLFLAG_NOEXPAND))
              ){
                pX->fg.bNoExpand = 1;
              }
            }ಅನ್ಯಥಾ ಯದಿ( longNames ){
              pX->zEName = sqlite3MPrintf(db, "%s.%s", zTabName, zName);
              pX->fg.eEName = ENAME_NAME;
            }ಅನ್ಯಥಾ{
              pX->zEName = sqlite3DbStrDup(db, zName);
              pX->fg.eEName = ENAME_NAME;
            }
          }
        }
        ಯದಿ( !tableSeen ){
          ಯದಿ( zTName ){
            sqlite3_Error_ಸಂದೇಶ(pParse, "no such table: %s", zTName);
          }ಅನ್ಯಥಾ{
            sqlite3_Error_ಸಂದೇಶ(pParse, "no tables specified");
          }
        }
      }
    }
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pEList);
    p->pEList = pNew;
  }
  ಯದಿ( p->pEList ){
    ಯದಿ( p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ>db->aLimit[SQLITE_LIMIT_COLUMN] ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "too many columns in result set");
      ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    }
    ಯದಿ( (elistFlags & (EP_HasFunc|EP_Subquery))!=0 ){
      p->selFlags |= SF_ComplexResult;
    }
  }
#ಯದಿ TREETRACE_ENABLED
  ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x8 ){
    TREETRACE(0x8,pParse,p,("After result-set wildcard expansion:\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}

#ಯದಿ SQLITE_DEBUG
/*
** Always assert.  This xSelectCallback2 implementation proves that the
** xSelectCallback2 is never invoked.
*/
ಶೂನ್ಯ sqlite3SelectWalkAssert2(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ಪ್ರತಿಜ್ಞಾ( 0 );
}
#endif

/*
** The first argument is a Walker object created by sqlite3SelectExpand().
** This function uses the walker to expand the SELECT statement passed
** as the second argument. If necessary, it also converts any compound
** SELECT statements that have COLLATE clauses on ORDER BY expressions
** to sub-queries. Return the return value of sqlite3WalkSelect().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ selectExpanderWalk(Walker *pWalker, Select *pSelect){
  ಪ್ರತಿಜ್ಞಾ( pWalker->x_ಅಭಿವ್ಯಕ್ತಿ_Callback==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Walk_Noop );
  ಪ್ರತಿಜ್ಞಾ( pWalker->pParse!=0 );
  ಯದಿ( OK_IF_ALWAYS_TRUE(pWalker->pParse->hasCompound) ){
    pWalker->xSelectCallback = convertCompoundSelectToSubquery;
    pWalker->xSelectCallback2 = 0;
    sqlite3WalkSelect(pWalker, pSelect);
  }
  pWalker->xSelectCallback = selectExpander;
  pWalker->xSelectCallback2 = sqlite3_Select_ತೆಗೆ_With;
  ಪ್ರತ್ಯಾಗಮ sqlite3WalkSelect(pWalker, pSelect);
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
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3SelectExpand(Parse *pParse, Select *pSelect){
  Walker w;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Walk_Noop;
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
ಸ್ಥಿರ ಶೂನ್ಯ selectAddSubqueryTypeInfo(Walker *pWalker, Select *p){
  Parse *pParse;
  ಪೂರ್ಣಾಙ್ಕ i;
  SrcList *pTabList;
  SrcItem *pFrom;

  ಯದಿ( p->selFlags & SF_HasTypeInfo ) ಪ್ರತ್ಯಾಗಮ;
  p->selFlags |= SF_HasTypeInfo;
  pParse = pWalker->pParse;
  ಪ್ರತಿಜ್ಞಾ( (p->selFlags & SF_Resolved) );
  pTabList = p->pSrc;
  ಪ್ರತಿ(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    Table *pTab = pFrom->pSTab;
    ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
    ಯದಿ( (pTab->tabFlags & TF_Ephemeral)!=0 && pFrom->fg.isSubquery ){
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
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3SelectAddTypeInfo(Parse *pParse, Select *pSelect){
#ifndef SQLITE_OMIT_SUBQUERY
  Walker w;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.xSelectCallback2 = selectAddSubqueryTypeInfo;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Walk_Noop;
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
ಶೂನ್ಯ sqlite3SelectPrep(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for container */
){
  ಪ್ರತಿಜ್ಞಾ( p!=0 || pParse->db->mallocFailed );
  ಪ್ರತಿಜ್ಞಾ( pParse->db->pParse==pParse );
  ಯದಿ( pParse->db->mallocFailed ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( p->selFlags & SF_HasTypeInfo ) ಪ್ರತ್ಯಾಗಮ;
  sqlite3SelectExpand(pParse, p);
  ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ;
  sqlite3ResolveSelectNames(pParse, p, pOuterNC);
  ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ;
  sqlite3SelectAddTypeInfo(pParse, p);
}

#ಯದಿ TREETRACE_ENABLED
/*
** Display all information about an AggInfo object
*/
ಸ್ಥಿರ ಶೂನ್ಯ printAggInfo(AggInfo *pAggInfo){
  ಪೂರ್ಣಾಙ್ಕ ii;
  sqlite3DebugPrintf("AggInfo %d/%p:\n",
     pAggInfo->selId, pAggInfo);
  ಪ್ರತಿ(ii=0; ii<pAggInfo->nColumn; ii++){
    ಸಂರಚನಾ AggInfo_col *pCol = &pAggInfo->aCol[ii];
    sqlite3DebugPrintf(
       "agg-column[%d] pTab=%s iTable=%d iColumn=%d iMem=%d"
       " iSorterColumn=%d %s\n",
       ii, pCol->pTab ? pCol->pTab->zName : "NULL",
       pCol->iTable, pCol->iColumn, pAggInfo->iFirstReg+ii,
       pCol->iSorterColumn,
       ii>=pAggInfo->nAccumulator ? "" : " Accumulator");
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(0, pAggInfo->aCol[ii].pCExpr, 0);
  }
  ಪ್ರತಿ(ii=0; ii<pAggInfo->nFunc; ii++){
    sqlite3DebugPrintf("agg-func[%d]: iMem=%d\n",
        ii, pAggInfo->iFirstReg+pAggInfo->nColumn+ii);
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(0, pAggInfo->aFunc[ii].pFExpr, 0);
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
ಸ್ಥಿರ ಶೂನ್ಯ analyzeAggFuncArgs(
  AggInfo *pAggInfo,
  NameContext *pNC
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿಜ್ಞಾ( pAggInfo!=0 );
  ಪ್ರತಿಜ್ಞಾ( pAggInfo->iFirstReg==0 );
  pNC->ncFlags |= NC_InAggFunc;
  ಪ್ರತಿ(i=0; i<pAggInfo->nFunc; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pAggInfo->aFunc[i].pFExpr;
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION );
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Agg_List(pNC, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList);
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ ){
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_ORDER );
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) );
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Agg_List(pNC, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pList);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    ಪ್ರತಿಜ್ಞಾ( !IsWindowFunc(p_ಅಭಿವ್ಯಕ್ತಿ) );
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Aggregates(pNC, p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin->pFilter);
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
ಸ್ಥಿರ ಶೂನ್ಯ optimize_Aggregate_Use_Of_Indexed_ಅಭಿವ್ಯಕ್ತಿ(
  Parse *pParse,          /* Parsing context */
  Select *pSelect,        /* The SELECT statement being processed */
  AggInfo *pAggInfo,      /* The aggregate info */
  NameContext *pNC        /* Name context used to resolve agg-func args */
){
  ಪ್ರತಿಜ್ಞಾ( pAggInfo->iFirstReg==0 );
  ಪ್ರತಿಜ್ಞಾ( pSelect!=0 );
  ಪ್ರತಿಜ್ಞಾ( pSelect->pGroupBy!=0 );
  pAggInfo->nColumn = pAggInfo->nAccumulator;
  ಯದಿ( ALWAYS(pAggInfo->nSortingColumn>0) ){
    ಪೂರ್ಣಾಙ್ಕ mx = pSelect->pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ - 1;
    ಪೂರ್ಣಾಙ್ಕ j, k;
    ಪ್ರತಿ(j=0; j<pAggInfo->nColumn; j++){
      k = pAggInfo->aCol[j].iSorterColumn;
      ಯದಿ( k>mx ) mx = k;
    }
    pAggInfo->nSortingColumn = mx+1;
  }
  analyzeAggFuncArgs(pAggInfo, pNC);
#ಯದಿ TREETRACE_ENABLED
  ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x20 ){
    Indexed_ಅಭಿವ್ಯಕ್ತಿ *pIEpr;
    TREETRACE(0x20, pParse, pSelect,
        ("AggInfo (possibly) adjusted for Indexed Exprs\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, pSelect, 0);
    ಪ್ರತಿ(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
      printf("data-cursor=%d index={%d,%d}\n",
          pIEpr->iDataCur, pIEpr->iIdxCur, pIEpr->iIdxCol);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(0, pIEpr->p_ಅಭಿವ್ಯಕ್ತಿ, 0);
    }
    printAggInfo(pAggInfo);
  }
#ಅನ್ಯಥಾ
  UNUSED_PARAMETER(pSelect);
  UNUSED_PARAMETER(pParse);
#endif
}

/*
** Walker callback for aggregateConvertIndexedExprRefToColumn().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ aggregateIdxEprRefToColCallback(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  AggInfo *pAggInfo;
  ಸಂರಚನಾ AggInfo_col *pCol;
  UNUSED_PARAMETER(pWalker);
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo==0 ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_COLUMN ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IF_NULL_ROW ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  pAggInfo = p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo;
  ಯದಿ( NEVER(p_ಅಭಿವ್ಯಕ್ತಿ->iAgg>=pAggInfo->nColumn) ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->iAgg>=0 );
  pCol = &pAggInfo->aCol[p_ಅಭಿವ್ಯಕ್ತಿ->iAgg];
  p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_AGG_COLUMN;
  p_ಅಭಿವ್ಯಕ್ತಿ->iTable = pCol->iTable;
  p_ಅಭಿವ್ಯಕ್ತಿ->iColumn = pCol->iColumn;
  ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Skip|EP_Collate|EP_Unlikely);
  ಪ್ರತ್ಯಾಗಮ WRC_Prune;
}

/*
** Convert every pAggInfo->aFunc[].pExpr such that any node within
** those expressions that has pAppInfo set is changed into a TK_AGG_COLUMN
** opcode.
*/
ಸ್ಥಿರ ಶೂನ್ಯ aggregate_Convert_Indexed_ಅಭಿವ್ಯಕ್ತಿ_Ref_To_Column(AggInfo *pAggInfo){
  ಪೂರ್ಣಾಙ್ಕ i;
  Walker w;
  memset(&w, 0, ಪರಿಮಾಣ(w));
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = aggregateIdxEprRefToColCallback;
  ಪ್ರತಿ(i=0; i<pAggInfo->nFunc; i++){
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, pAggInfo->aFunc[i].pFExpr);
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
ಸ್ಥಿರ ಶೂನ್ಯ assignAggregateRegisters(Parse *pParse, AggInfo *pAggInfo){
  ಪ್ರತಿಜ್ಞಾ( pAggInfo!=0 );
  ಪ್ರತಿಜ್ಞಾ( pAggInfo->iFirstReg==0 );
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
ಸ್ಥಿರ ಶೂನ್ಯ resetAccumulator(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಸಂರಚನಾ AggInfo_func *pFunc;
  ಪೂರ್ಣಾಙ್ಕ nReg = pAggInfo->nFunc + pAggInfo->nColumn;
  ಪ್ರತಿಜ್ಞಾ( pAggInfo->iFirstReg>0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->db->pParse==pParse );
  ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ಯದಿ( nReg==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ;
  sqlite3VdbeAddOp3(v, OP_Null, 0, pAggInfo->iFirstReg,
                    pAggInfo->iFirstReg+nReg-1);
  ಪ್ರತಿ(pFunc=pAggInfo->aFunc, i=0; i<pAggInfo->nFunc; i++, pFunc++){
    ಯದಿ( pFunc->iDistinct>=0 ){
      ಅಭಿವ್ಯಕ್ತಿ *pE = pFunc->pFExpr;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pE) );
      ಯದಿ( pE->x.pList==0 || pE->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ!=1 ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "DISTINCT aggregates must have exactly one "
           "argument");
        pFunc->iDistinct = -1;
      }ಅನ್ಯಥಾ{
        ಕೀಲಿ_Info *p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pE->x.pList,0,0);
        pFunc->iDistAddr = sqlite3VdbeAddOp4(v, OP_OpenEphemeral,
            pFunc->iDistinct, 0, 0, (ವರ್ಣ*)p_ಕೀಲಿ_Info, P4_KEYINFO);
        ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(DISTINCT)",
                          pFunc->pFunc->zName));
      }
    }
    ಯದಿ( pFunc->iOBTab>=0 ){
      ಅಭಿವ್ಯಕ್ತಿ_List *pOBList;
      ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;
      ಪೂರ್ಣಾಙ್ಕ nExtra = 0;
      ಪ್ರತಿಜ್ಞಾ( pFunc->pFExpr->p_ಎಡ!=0 );
      ಪ್ರತಿಜ್ಞಾ( pFunc->pFExpr->p_ಎಡ->op==TK_ORDER );
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pFunc->pFExpr->p_ಎಡ) );
      ಪ್ರತಿಜ್ಞಾ( pFunc->pFunc!=0 );
      pOBList = pFunc->pFExpr->p_ಎಡ->x.pList;
      ಯದಿ( !pFunc->bOBUnique ){
        nExtra++;  /* One extra column for the OP_Sequence */
      }
      ಯದಿ( pFunc->bOBPayload ){
        /* extra columns for the function arguments */
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pFunc->pFExpr) );
        ಪ್ರತಿಜ್ಞಾ( pFunc->pFExpr->x.pList!=0 );
        nExtra += pFunc->pFExpr->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ;
      }
      ಯದಿ( pFunc->bUseSubtype ){
        nExtra += pFunc->pFExpr->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ;
      }
      p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pOBList, 0, nExtra);
      ಯದಿ( !pFunc->bOBUnique && pParse->nErr==0 ){
        p_ಕೀಲಿ_Info->n_ಕೀಲಿ_Field++;
      }
      sqlite3VdbeAddOp4(v, OP_OpenEphemeral,
            pFunc->iOBTab, pOBList->n_ಅಭಿವ್ಯಕ್ತಿ+nExtra, 0,
            (ವರ್ಣ*)p_ಕೀಲಿ_Info, P4_KEYINFO);
      ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(ORDER BY)",
                          pFunc->pFunc->zName));
    }
  }
}

/*
** Invoke the OP_AggFinalize opcode for every aggregate function
** in the AggInfo structure.
*/
ಸ್ಥಿರ ಶೂನ್ಯ finalizeAggFunctions(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಸಂರಚನಾ AggInfo_func *pF;
  ಪ್ರತಿ(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    ಅಭಿವ್ಯಕ್ತಿ_List *pList;
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pF->pFExpr) );
    ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ;
    pList = pF->pFExpr->x.pList;
    ಯದಿ( pF->iOBTab>=0 ){
      /* For an ORDER BY aggregate, calls to OP_AggStep were deferred.  Inputs
      ** were stored in emphermal table pF->iOBTab.  Here, we extract those
      ** inputs (in ORDER BY order) and make all calls to OP_AggStep
      ** before doing the OP_AggFinal call. */
      ಪೂರ್ಣಾಙ್ಕ i_ತುದಿ;        /* Start of loop for extracting columns */
      ಪೂರ್ಣಾಙ್ಕ nArg;        /* Number of columns to extract */
      ಪೂರ್ಣಾಙ್ಕ n_ಕೀಲಿ;        /* Key columns to be skipped */
      ಪೂರ್ಣಾಙ್ಕ regAgg;      /* Extract into this array */
      ಪೂರ್ಣಾಙ್ಕ j;           /* Loop counter */
     
      ಪ್ರತಿಜ್ಞಾ( pF->pFunc!=0 );
      nArg = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
      regAgg = sqlite3GetTempRange(pParse, nArg);

      ಯದಿ( pF->bOBPayload==0 ){
        n_ಕೀಲಿ = 0;
      }ಅನ್ಯಥಾ{
        ಪ್ರತಿಜ್ಞಾ( pF->pFExpr->p_ಎಡ!=0 );
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pF->pFExpr->p_ಎಡ) );
        ಪ್ರತಿಜ್ಞಾ( pF->pFExpr->p_ಎಡ->x.pList!=0 );
        n_ಕೀಲಿ = pF->pFExpr->p_ಎಡ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ;
        ಯದಿ( ALWAYS(!pF->bOBUnique) ) n_ಕೀಲಿ++;
      }
      i_ತುದಿ = sqlite3VdbeAddOp1(v, OP_Rewind, pF->iOBTab); VdbeCoverage(v);
      ಪ್ರತಿ(j=nArg-1; j>=0; j--){
        sqlite3VdbeAddOp3(v, OP_Column, pF->iOBTab, n_ಕೀಲಿ+j, regAgg+j);
      }
      ಯದಿ( pF->bUseSubtype ){
        ಪೂರ್ಣಾಙ್ಕ regSubtype = sqlite3GetTempReg(pParse);
        ಪೂರ್ಣಾಙ್ಕ iBaseCol = n_ಕೀಲಿ + nArg + (pF->bOBPayload==0 && pF->bOBUnique==0);
        ಪ್ರತಿ(j=nArg-1; j>=0; j--){
          sqlite3VdbeAddOp3(v, OP_Column, pF->iOBTab, iBaseCol+j, regSubtype);
          sqlite3VdbeAddOp2(v, OP_SetSubtype, regSubtype, regAgg+j);
        }
        sqlite3ReleaseTempReg(pParse, regSubtype);
      }
      sqlite3VdbeAddOp3(v, OP_AggStep, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3VdbeAddOp2(v, OP_Next, pF->iOBTab, i_ತುದಿ+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, i_ತುದಿ);
      sqlite3ReleaseTempRange(pParse, regAgg, nArg);
    }
    sqlite3VdbeAddOp2(v, OP_AggFinal, AggInfoFuncReg(pAggInfo,i),
                      pList ? pList->n_ಅಭಿವ್ಯಕ್ತಿ : 0);
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
ಸ್ಥಿರ ಶೂನ್ಯ updateAccumulator(
  Parse *pParse,
  ಪೂರ್ಣಾಙ್ಕ regAcc,
  AggInfo *pAggInfo,
  ಪೂರ್ಣಾಙ್ಕ eDistinctType
){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ regHit = 0;
  ಪೂರ್ಣಾಙ್ಕ addrHitTest = 0;
  ಸಂರಚನಾ AggInfo_func *pF;
  ಸಂರಚನಾ AggInfo_col *pC;

  ಪ್ರತಿಜ್ಞಾ( pAggInfo->iFirstReg>0 );
  ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ;
  pAggInfo->directMode = 1;
  ಪ್ರತಿ(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    ಪೂರ್ಣಾಙ್ಕ nArg;
    ಪೂರ್ಣಾಙ್ಕ addrNext = 0;
    ಪೂರ್ಣಾಙ್ಕ regAgg;
    ಪೂರ್ಣಾಙ್ಕ regAggSz = 0;
    ಪೂರ್ಣಾಙ್ಕ regDistinct = 0;
    ಅಭಿವ್ಯಕ್ತಿ_List *pList;
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pF->pFExpr) );
    ಪ್ರತಿಜ್ಞಾ( !IsWindowFunc(pF->pFExpr) );
    ಪ್ರತಿಜ್ಞಾ( pF->pFunc!=0 );
    pList = pF->pFExpr->x.pList;
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pF->pFExpr, EP_WinFunc) ){
      ಅಭಿವ್ಯಕ್ತಿ *pFilter = pF->pFExpr->y.pWin->pFilter;
      ಯದಿ( pAggInfo->nAccumulator
       && (pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)
       && regAcc
      ){
        /* If regAcc==0, there there exists some min() or max() function
        ** without a FILTER clause that will ensure the magnet registers
        ** are populated. */
        ಯದಿ( regHit==0 ) regHit = ++pParse->nMem;
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
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pFilter, addrNext, SQLITE_JUMPIFNULL);
    }
    ಯದಿ( pF->iOBTab>=0 ){
      /* Instead of invoking AggStep, we must push the arguments that would
      ** have been passed to AggStep onto the sorting table. */
      ಪೂರ್ಣಾಙ್ಕ jj;                /* Registered used so far in building the record */
      ಅಭಿವ್ಯಕ್ತಿ_List *pOBList;     /* The ORDER BY clause */
      ಪ್ರತಿಜ್ಞಾ( pList!=0 );
      nArg = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
      ಪ್ರತಿಜ್ಞಾ( nArg>0 );
      ಪ್ರತಿಜ್ಞಾ( pF->pFExpr->p_ಎಡ!=0 );
      ಪ್ರತಿಜ್ಞಾ( pF->pFExpr->p_ಎಡ->op==TK_ORDER );
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pF->pFExpr->p_ಎಡ) );
      pOBList = pF->pFExpr->p_ಎಡ->x.pList;
      ಪ್ರತಿಜ್ಞಾ( pOBList!=0 );
      ಪ್ರತಿಜ್ಞಾ( pOBList->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
      regAggSz = pOBList->n_ಅಭಿವ್ಯಕ್ತಿ;
      ಯದಿ( !pF->bOBUnique ){
        regAggSz++;   /* One register for OP_Sequence */
      }
      ಯದಿ( pF->bOBPayload ){
        regAggSz += nArg;
      }
      ಯದಿ( pF->bUseSubtype ){
        regAggSz += nArg;
      }
      regAggSz++;  /* One extra register to hold result of MakeRecord */
      regAgg = sqlite3GetTempRange(pParse, regAggSz);
      regDistinct = regAgg;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pOBList, regAgg, 0, SQLITE_ECEL_DUP);
      jj = pOBList->n_ಅಭಿವ್ಯಕ್ತಿ;
      ಯದಿ( !pF->bOBUnique ){
        sqlite3VdbeAddOp2(v, OP_Sequence, pF->iOBTab, regAgg+jj);
        jj++;
      }
      ಯದಿ( pF->bOBPayload ){
        regDistinct = regAgg+jj;
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pList, regDistinct, 0, SQLITE_ECEL_DUP);
        jj += nArg;
      }
      ಯದಿ( pF->bUseSubtype ){
        ಪೂರ್ಣಾಙ್ಕ kk;
        ಪೂರ್ಣಾಙ್ಕ regBase = pF->bOBPayload ? regDistinct : regAgg;
        ಪ್ರತಿ(kk=0; kk<nArg; kk++, jj++){
          sqlite3VdbeAddOp2(v, OP_GetSubtype, regBase+kk, regAgg+jj);
        }
      }
    }ಅನ್ಯಥಾ ಯದಿ( pList ){
      nArg = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
      regAgg = sqlite3GetTempRange(pParse, nArg);
      regDistinct = regAgg;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pList, regAgg, 0, SQLITE_ECEL_DUP);
    }ಅನ್ಯಥಾ{
      nArg = 0;
      regAgg = 0;
    }
    ಯದಿ( pF->iDistinct>=0 && pList ){
      ಯದಿ( addrNext==0 ){
        addrNext = sqlite3VdbeMakeLabel(pParse);
      }
      pF->iDistinct = codeDistinct(pParse, eDistinctType,
          pF->iDistinct, addrNext, pList, regDistinct);
    }
    ಯದಿ( pF->iOBTab>=0 ){
      /* Insert a new record into the ORDER BY table */
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regAgg, regAggSz-1,
                        regAgg+regAggSz-1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ಸೇರಿಸು, pF->iOBTab, regAgg+regAggSz-1,
                           regAgg, regAggSz-1);
      sqlite3ReleaseTempRange(pParse, regAgg, regAggSz);
    }ಅನ್ಯಥಾ{
      /* Invoke the AggStep function */
      ಯದಿ( pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        CollSeq *pColl = 0;
        ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
        ಪೂರ್ಣಾಙ್ಕ j;
        ಪ್ರತಿಜ್ಞಾ( pList!=0 );  /* pList!=0 if pF->pFunc has NEEDCOLL */
        ಪ್ರತಿ(j=0, pItem=pList->a; !pColl && j<nArg; j++, pItem++){
          pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, pItem->p_ಅಭಿವ್ಯಕ್ತಿ);
        }
        ಯದಿ( !pColl ){
          pColl = pParse->db->pDfltColl;
        }
        ಯದಿ( regHit==0 && pAggInfo->nAccumulator ) regHit = ++pParse->nMem;
        sqlite3VdbeAddOp4(v, OP_CollSeq, regHit, 0, 0,
                         (ವರ್ಣ *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddOp3(v, OP_AggStep, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3ReleaseTempRange(pParse, regAgg, nArg);
    }
    ಯದಿ( addrNext ){
      sqlite3VdbeResolveLabel(v, addrNext);
    }
    ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( regHit==0 && pAggInfo->nAccumulator ){
    regHit = regAcc;
  }
  ಯದಿ( regHit ){
    addrHitTest = sqlite3VdbeAddOp1(v, OP_If, regHit); VdbeCoverage(v);
  }
  ಪ್ರತಿ(i=0, pC=pAggInfo->aCol; i<pAggInfo->nAccumulator; i++, pC++){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, pC->pCExpr, AggInfoColumnReg(pAggInfo,i));
    ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ;
  }

  pAggInfo->directMode = 0;
  ಯದಿ( addrHitTest ){
    sqlite3_Vdbe_Jump_Here_Or_ತೆಗೆ_Inst(v, addrHitTest);
  }
}

/*
** Add a single OP_Explain instruction to the VDBE to explain a simple
** count(*) query ("SELECT count(*) FROM pTab").
*/
#ifndef SQLITE_OMIT_EXPLAIN
ಸ್ಥಿರ ಶೂನ್ಯ explain_Simple_ಎಣಿಕೆ(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Table being queried */
  Index *pIdx                     /* Index used to optimize scan, or NULL */
){
  ಯದಿ( pParse->explain==2 ){
    ಪೂರ್ಣಾಙ್ಕ bCover = (pIdx!=0 && (HasRowid(pTab) || !Is_Primary_ಕೀಲಿ_Index(pIdx)));
    sqlite3VdbeExplain(pParse, 0, "SCAN %s%s%s",
        pTab->zName,
        bCover ? " USING COVERING INDEX " : "",
        bCover ? pIdx->zName : ""
    );
  }
}
#ಅನ್ಯಥಾ
# define explain_Simple_ಎಣಿಕೆ(a,b,c)
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ having_To_Where_ಅಭಿವ್ಯಕ್ತಿ_Cb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_AND ){
    Select *pS = pWalker->u.pSelect;
    /* This routine is called before the HAVING clause of the current
    ** SELECT is analyzed for aggregates. So if pExpr->pAggInfo is set
    ** here, it indicates that the expression is a correlated reference to a
    ** column from an outer aggregate query, or an aggregate function that
    ** belongs to an outer query. Do not move the expression to the WHERE
    ** clause in this obscure case, as doing so may corrupt the outer Select
    ** statements AggInfo structure.  */
    ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Or_Group_By(pWalker->pParse, p_ಅಭಿವ್ಯಕ್ತಿ, pS->pGroupBy)
     && ಅಭಿವ್ಯಕ್ತಿ_Always_False(p_ಅಭಿವ್ಯಕ್ತಿ)==0
     && p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo==0
    ){
      sqlite3 *db = pWalker->pParse->db;
      ಅಭಿವ್ಯಕ್ತಿ *pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(db, 1);
      ಯದಿ( pNew ){
        ಅಭಿವ್ಯಕ್ತಿ *pWhere = pS->pWhere;
        SWAP(ಅಭಿವ್ಯಕ್ತಿ, *pNew, *p_ಅಭಿವ್ಯಕ್ತಿ);
        pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pWalker->pParse, pWhere, pNew);
        pS->pWhere = pNew;
        pWalker->eCode = 1;
      }
    }
    ಪ್ರತ್ಯಾಗಮ WRC_Prune;
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
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
ಸ್ಥಿರ ಶೂನ್ಯ havingToWhere(Parse *pParse, Select *p){
  Walker sWalker;
  memset(&sWalker, 0, ಪರಿಮಾಣ(sWalker));
  sWalker.pParse = pParse;
  sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = having_To_Where_ಅಭಿವ್ಯಕ್ತಿ_Cb;
  sWalker.u.pSelect = p;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, p->pHaving);
#ಯದಿ TREETRACE_ENABLED
  ಯದಿ( sWalker.eCode && (sqlite3_ವೃಕ್ಷ_Trace & 0x100)!=0 ){
    TREETRACE(0x100,pParse,p,("Move HAVING terms into WHERE:\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
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
ಸ್ಥಿರ SrcItem *isSelfJoinView(
  SrcList *pTabList,           /* Search for self-joins in this FROM clause */
  SrcItem *pThis,              /* Search for prior reference to this subquery */
  ಪೂರ್ಣಾಙ್ಕ iFirst, ಪೂರ್ಣಾಙ್ಕ iEnd        /* Range of FROM-clause entries to search. */
){
  SrcItem *pItem;
  Select *pSel;
  ಪ್ರತಿಜ್ಞಾ( pThis->fg.isSubquery );
  pSel = pThis->u4.pSubq->pSelect;
  ಪ್ರತಿಜ್ಞಾ( pSel!=0 );
  ಯದಿ( pSel->selFlags & SF_ತಳ್ಳು_Down ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯಾವತ್( iFirst<iEnd ){
    Select *pS1;
    pItem = &pTabList->a[iFirst++];
    ಯದಿ( !pItem->fg.isSubquery ) ಅನುವರ್ತ;
    ಯದಿ( pItem->fg.viaCoroutine ) ಅನುವರ್ತ;
    ಯದಿ( pItem->zName==0 ) ಅನುವರ್ತ;
    ಪ್ರತಿಜ್ಞಾ( pItem->pSTab!=0 );
    ಪ್ರತಿಜ್ಞಾ( pThis->pSTab!=0 );
    ಯದಿ( pItem->pSTab->pSchema!=pThis->pSTab->pSchema ) ಅನುವರ್ತ;
    ಯದಿ( sqlite3_stricmp(pItem->zName, pThis->zName)!=0 ) ಅನುವರ್ತ;
    pS1 = pItem->u4.pSubq->pSelect;
    ಯದಿ( pItem->pSTab->pSchema==0 && pSel->selId!=pS1->selId ){
      /* The query flattener left two different CTE tables with identical
      ** names in the same FROM clause. */
      ಅನುವರ್ತ;
    }
    ಯದಿ( pS1->selFlags & SF_ತಳ್ಳು_Down ){
      /* The view was modified by some other optimization such as
      ** pushDownWhereTerms() */
      ಅನುವರ್ತ;
    }
    ಪ್ರತ್ಯಾಗಮ pItem;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Deallocate a single AggInfo object
*/
ಸ್ಥಿರ ಶೂನ್ಯ agginfoFree(sqlite3 *db, ಶೂನ್ಯ *pArg){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ_Of_View_Optimization(Parse *pParse, Select *p){
  Select *pSub, *pPrior;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಣಿಕೆ;
  sqlite3 *db;
  SrcItem *pFrom;
  ಯದಿ( (p->selFlags & SF_Aggregate)==0 ) ಪ್ರತ್ಯಾಗಮ 0;   /* This is an aggregate */
  ಯದಿ( p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ!=1 ) ಪ್ರತ್ಯಾಗಮ 0;               /* Single result column */
  ಯದಿ( p->pWhere ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( p->pHaving ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( p->pGroupBy ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( p->pOrderBy ) ಪ್ರತ್ಯಾಗಮ 0;
  p_ಅಭಿವ್ಯಕ್ತಿ = p->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_AGG_FUNCTION ) ಪ್ರತ್ಯಾಗಮ 0;        /* Result is an aggregate */
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_UToken(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಯದಿ( sqlite3_stricmp(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken,"count") ) ಪ್ರತ್ಯಾಗಮ 0;  /* Is count() */
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList!=0 ) ಪ್ರತ್ಯಾಗಮ 0;                 /* Must be count(*) */
  ಯದಿ( p->pSrc->nSrc!=1 ) ಪ್ರತ್ಯಾಗಮ 0;                  /* One table in FROM  */
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ) ಪ್ರತ್ಯಾಗಮ 0;/* Not a window function */
  pFrom = p->pSrc->a;
  ಯದಿ( pFrom->fg.isSubquery==0 ) ಪ್ರತ್ಯಾಗಮ 0;           /* FROM is a subquery */
  ಯದಿ( (p->selFlags & SF_Correlated)!=0 ) ಪ್ರತ್ಯಾಗಮ 0;  /* Not a correlated subq */
  pSub = pFrom->u4.pSubq->pSelect;
  ಯದಿ( pSub->pPrior==0 ) ಪ್ರತ್ಯಾಗಮ 0;                   /* Must be a compound */
  ಯದಿ( pSub->selFlags & SF_CopyCte ) ಪ್ರತ್ಯಾಗಮ 0;       /* Not a CTE */
  ಕುರು{
    ಯದಿ( pSub->op!=TK_ALL && pSub->pPrior ) ಪ್ರತ್ಯಾಗಮ 0;  /* Must be UNION ALL */
    ಯದಿ( pSub->pWhere ) ಪ್ರತ್ಯಾಗಮ 0;                      /* No WHERE clause */
    ಯದಿ( pSub->pLimit ) ಪ್ರತ್ಯಾಗಮ 0;                      /* No LIMIT clause */
    ಯದಿ( pSub->selFlags & (SF_Aggregate|SF_Distinct) ){
       testcase( pSub->selFlags & SF_Aggregate );
       testcase( pSub->selFlags & SF_Distinct );
       ಪ್ರತ್ಯಾಗಮ 0;     /* Not an aggregate nor DISTINCT */
    }
    ಪ್ರತಿಜ್ಞಾ( pSub->pHaving==0 );  /* Due to the previous */
    pSub = pSub->pPrior;                              /* Repeat over compound */
  }ಯಾವತ್( pSub );

  /* If we reach this point then it is OK to perform the transformation */

  db = pParse->db;
  p_ಎಣಿಕೆ = p_ಅಭಿವ್ಯಕ್ತಿ;
  p_ಅಭಿವ್ಯಕ್ತಿ = 0;
  pSub = sqlite3SubqueryDetach(db, pFrom);
  sqlite3SrcListDelete(db, p->pSrc);
  p->pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  ಯಾವತ್( pSub ){
    ಅಭಿವ್ಯಕ್ತಿ *pTerm;
    pPrior = pSub->pPrior;
    pSub->pPrior = 0;
    pSub->pNext = 0;
    pSub->selFlags |= SF_Aggregate;
    pSub->selFlags &= ~(u32)SF_Compound;
    pSub->nSelectRow = 0;
    sqlite3ParserAddCleanup(pParse, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete_Generic, pSub->pEList);
    pTerm = pPrior ? sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಣಿಕೆ, 0) : p_ಎಣಿಕೆ;
    pSub->pEList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, 0, pTerm);
    pTerm = sqlite3PExpr(pParse, TK_SELECT, 0, 0);
    sqlite3PExprAddSelect(pParse, pTerm, pSub);
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ){
      p_ಅಭಿವ್ಯಕ್ತಿ = pTerm;
    }ಅನ್ಯಥಾ{
      p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3PExpr(pParse, TK_PLUS, pTerm, p_ಅಭಿವ್ಯಕ್ತಿ);
    }
    ಯದಿ( pParse->nErr ){
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ);
      p_ಅಭಿವ್ಯಕ್ತಿ = 0;
    }
    pSub = pPrior;
  }
  p->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ;
  p->selFlags &= ~(u32)SF_Aggregate;

#ಯದಿ TREETRACE_ENABLED
  ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x200 ){
    TREETRACE(0x200,pParse,p,("After count-of-view optimization:\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
  }
#endif
  ಪ್ರತ್ಯಾಗಮ 1;
}

/*
** If any term of pSrc, or any SF_NestedFrom sub-query, is not the same
** as pSrcItem but has the same alias as p0, then return true.
** Otherwise return false.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sameSrcAlias(SrcItem *p0, SrcList *pSrc){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<pSrc->nSrc; i++){
    SrcItem *p1 = &pSrc->a[i];
    ಯದಿ( p1==p0 ) ಅನುವರ್ತ;
    ಯದಿ( p0->pSTab==p1->pSTab && 0==sqlite3_stricmp(p0->zAlias, p1->zAlias) ){
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಯದಿ( p1->fg.isSubquery
     && (p1->u4.pSubq->pSelect->selFlags & SF_NestedFrom)!=0
     && sameSrcAlias(p0, p1->u4.pSubq->pSelect->pSrc)
    ){
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fromClauseTermCanBeCoroutine(
  Parse *pParse,          /* Parsing context */
  SrcList *pTabList,      /* FROM clause */
  ಪೂರ್ಣಾಙ್ಕ i,                  /* Which term of the FROM clause holds the subquery */
  ಪೂರ್ಣಾಙ್ಕ selFlags            /* Flags on the SELECT statement */
){
  SrcItem *pItem = &pTabList->a[i];
  ಯದಿ( pItem->fg.isCte ){
    ಧ್ರುವ CteUse *pCteUse = pItem->u2.pCteUse;
    ಯದಿ( pCteUse->eM10d==M10d_Yes ) ಪ್ರತ್ಯಾಗಮ 0;                          /* (2a) */
    ಯದಿ( pCteUse->nUse>=2 && pCteUse->eM10d!=M10d_No ) ಪ್ರತ್ಯಾಗಮ 0;       /* (2b) */
  }
  ಯದಿ( pTabList->a[0].fg.jointype & JT_LTORJ ) ಪ್ರತ್ಯಾಗಮ 0;               /* (3)  */
  ಯದಿ( OptimizationDisabled(pParse->db, SQLITE_Coroutines) ) ಪ್ರತ್ಯಾಗಮ 0; /* (4)  */
  ಯದಿ( isSelfJoinView(pTabList, pItem, i+1, pTabList->nSrc)!=0 ){
    ಪ್ರತ್ಯಾಗಮ 0;                                                          /* (5) */
  }
  ಯದಿ( i==0 ){
    ಯದಿ( pTabList->nSrc==1 ) ಪ್ರತ್ಯಾಗಮ 1;                             /* (1a) */
    ಯದಿ( pTabList->a[1].fg.jointype & JT_CROSS ) ಪ್ರತ್ಯಾಗಮ 1;         /* (1b) */
    ಯದಿ( selFlags & SF_UpdateFrom )              ಪ್ರತ್ಯಾಗಮ 0;         /* (1c-iii) */
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಯದಿ( selFlags & SF_UpdateFrom ) ಪ್ರತ್ಯಾಗಮ 0;                        /* (1c-iii) */
  ಯಾವತ್( 1 /*exit-by-break*/ ){
    ಯದಿ( pItem->fg.jointype & (JT_OUTER|JT_CROSS)  ) ಪ್ರತ್ಯಾಗಮ 0;     /* (1c-ii) */
    ಯದಿ( i==0 ) ಭಙ್ಗ;
    i--;
    pItem--;
    ಯದಿ( pItem->fg.isSubquery ) ಪ್ರತ್ಯಾಗಮ 0;                          /* (1c-i) */
  }
  ಪ್ರತ್ಯಾಗಮ 1;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ existsToJoin(
  Parse *pParse,  /* Parsing context */
  Select *p,      /* The SELECT statement being optimized */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere    /* part of the WHERE clause currently being examined */
){
  ಯದಿ( pParse->nErr==0
   && pWhere!=0
   && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pWhere, EP_OuterON|EP_InnerON) 
   && ALWAYS(p->pSrc!=0)
   && p->pSrc->nSrc<BMS
   && (p->pLimit==0 || p->pLimit->p_ಬಲ==0)
  ){
    ಯದಿ( pWhere->op==TK_AND ){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = pWhere->p_ಬಲ;
      existsToJoin(pParse, p, pWhere->p_ಎಡ);
      existsToJoin(pParse, p, p_ಬಲ);
    }
    ಅನ್ಯಥಾ ಯದಿ( pWhere->op==TK_EXISTS ){
      Select *pSub = pWhere->x.pSelect;
      ಅಭಿವ್ಯಕ್ತಿ *pSubWhere = pSub->pWhere;
      ಯದಿ( pSub->pSrc->nSrc==1
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
        ಪೂರ್ಣಾಙ್ಕ *aCsrMap = sqlite3DbMallocZero(db, (pParse->nTab+2)*ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
        ಯದಿ( aCsrMap==0 ) ಪ್ರತ್ಯಾಗಮ;
        aCsrMap[0] = (pParse->nTab+1);
        renumberCursors(pParse, pSub, -1, aCsrMap);
        sqlite3DbFree(db, aCsrMap);

        memset(pWhere, 0, ಪರಿಮಾಣ(*pWhere));
        pWhere->op = TK_INTEGER;
        pWhere->u.i_ಮೌಲ್ಯ = 1;
        ಅಭಿವ್ಯಕ್ತಿ_Set_Property(pWhere, EP_Int_ಮೌಲ್ಯ);
        ಪ್ರತಿಜ್ಞಾ( p->pWhere!=0 );
        pSub->pSrc->a[0].fg.fromExists = 1;
        p->pSrc = sqlite3SrcListAppendList(pParse, p->pSrc, pSub->pSrc);
        ಯದಿ( pSubWhere ){
          p->pWhere = sqlite3PExpr(pParse, TK_AND, p->pWhere, pSubWhere);
          pSub->pWhere = 0;
        }
        pSub->pSrc = 0;
        sqlite3ParserAddCleanup(pParse, sqlite3SelectDeleteGeneric, pSub);
        recomputeColumnsUsed(p, &p->pSrc->a[p->pSrc->nSrc-1]);
#ಯದಿ TREETRACE_ENABLED
        ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x100000 ){
          TREETRACE(0x100000,pParse,p,
                    ("After EXISTS-to-JOIN optimization:\n"));
          sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
        }
#endif
      }
    }
  }
}

/*
** Type used for Walker callbacks by selectCheckOnClauses().
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ CheckOnCtx CheckOnCtx;
ಸಂರಚನಾ CheckOnCtx {
  SrcList *pSrc;       /* SrcList for this context */
  ಪೂರ್ಣಾಙ್ಕ iJoin;           /* Cursors must be left of this one, if not negative */
  ಪೂರ್ಣಾಙ್ಕ bFuncArg;        /* True for table-function arg */
  CheckOnCtx *pParent; /* Parent context */
};

/*
** True if the SrcList passed as the only argument contains at least
** one RIGHT or FULL JOIN. False otherwise.  
*/
#define has_ಬಲ_Join(pSrc) (((pSrc)->a[0].fg.jointype & JT_LTORJ)!=0)

/*
** The xExpr callback for the search of invalid ON clause terms.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ select_Check_On_Clauses_ಅಭಿವ್ಯಕ್ತಿ(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;

  /* Check if pExpr is root or near-root of an ON clause constraint that needs
  ** to be checked to ensure that it does not refer to tables in its FROM
  ** clause to the right of itself. i.e. it is either:
  **
  **   + an ON clause on an OUTER join, or
  **   + an ON clause on an INNER join within a FROM that features at
  **     least one RIGHT or FULL join.
  */
  ಯದಿ( (ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON))
   || (ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON) && has_ಬಲ_Join(pCtx->pSrc))
  ){
    /* If CheckOnCtx.iJoin is already set, then fall through and process
    ** this expression node as normal. Or, if CheckOnCtx.iJoin is still 0,
    ** set it to the cursor number of the RHS of the join to which this
    ** ON expression was attached and then iterate through the entire 
    ** expression.  */
    ಪ್ರತಿಜ್ಞಾ( pCtx->iJoin<0 || pCtx->iJoin==p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin );
    ಯದಿ( pCtx->iJoin<0 ){
      pCtx->iJoin = p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin;
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_NN(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
      pCtx->iJoin = -1;
      ಪ್ರತ್ಯಾಗಮ WRC_Prune;
    }
  }

  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN ){
    /* A column expression. Find the SrcList (if any) to which it refers.
    ** Then, if CheckOnCtx.iJoin indicates that this expression is part of an
    ** ON clause from that SrcList (i.e. if iJoin is non-zero), check that it
    ** does not refer to a table to the right of CheckOnCtx.iJoin. */
    ಪೂರ್ಣಾಙ್ಕ iTab = p_ಅಭಿವ್ಯಕ್ತಿ->iTable;
    ಕುರು {
      SrcList *pSrc = pCtx->pSrc;
      ಪೂರ್ಣಾಙ್ಕ nSrc = pSrc->nSrc;
      ಪೂರ್ಣಾಙ್ಕ ii;
      ಪ್ರತಿ(ii=0; ii<nSrc && pSrc->a[ii].iCursor!=iTab; ii++){}
      ಯದಿ( ii<nSrc ){
        /* pSrc is the FROM clause that contains iTab */
        ಯದಿ( pCtx->iJoin>=0 ){
          ಪ್ರತಿ(ii--; ii>=0 && pSrc->a[ii].iCursor!=pCtx->iJoin; ii--){}
          ಯದಿ( ii>=0 ){
            /* Table iJoin appears to the left of table iTab in the SrcList.
            ** Therefore the expression refers to a table to its right. */
            sqlite3_Error_ಸಂದೇಶ(pWalker->pParse, 
                "%s references tables to its right",
                (pCtx->bFuncArg ? "table-function argument" : "ON clause")
            );
            ಪ್ರತ್ಯಾಗಮ WRC_Abort;
          }
        }
        ಭಙ್ಗ;
      }
      pCtx = pCtx->pParent;
    }ಯಾವತ್( pCtx );
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}

/*
** The xSelect callback for the search of invalid ON clause terms.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ selectCheckOnClausesSelect(Walker *pWalker, Select *pSelect){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;
  ಯದಿ( pSelect->pSrc==pCtx->pSrc || pSelect->pSrc->nSrc==0 ){
    ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  }ಅನ್ಯಥಾ{
    CheckOnCtx sCtx;
    memset(&sCtx, 0, ಪರಿಮಾಣ(sCtx));
    sCtx.pSrc = pSelect->pSrc;
    sCtx.pParent = pCtx;
    sCtx.iJoin = -1;
    pWalker->u.pCheckOnCtx = &sCtx;
    sqlite3WalkSelect(pWalker, pSelect);
    pWalker->u.pCheckOnCtx = pCtx;
    pSelect->selFlags &= ~SF_OnToWhere;
    ಪ್ರತ್ಯಾಗಮ WRC_Prune;
  }
}

/*
** Check all ON clauses in pSelect to verify that they do not reference
** columns to the right.
*/
ಶೂನ್ಯ sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect){
  Walker w;
  CheckOnCtx sCtx;
  ಪೂರ್ಣಾಙ್ಕ ii;
  ಪ್ರತಿಜ್ಞಾ( pSelect->selFlags & SF_OnToWhere );
  ಪ್ರತಿಜ್ಞಾ( pSelect->pSrc!=0 && pSelect->pSrc->nSrc>=2 );
  memset(&w, 0, ಪರಿಮಾಣ(w));
  w.pParse = pParse;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = select_Check_On_Clauses_ಅಭಿವ್ಯಕ್ತಿ;
  w.xSelectCallback = selectCheckOnClausesSelect;
  w.u.pCheckOnCtx = &sCtx;
  memset(&sCtx, 0, ಪರಿಮಾಣ(sCtx));
  sCtx.pSrc = pSelect->pSrc;
  sCtx.iJoin = -1;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, pSelect->pWhere);
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
  ಪ್ರತಿ(ii=0; ii<pSelect->pSrc->nSrc; ii++){
    SrcItem *pItem = &pSelect->pSrc->a[ii];
    ಯದಿ( pItem->fg.isTabFunc ){
      sCtx.iJoin = -1;
      ಯದಿ( (pItem->fg.jointype & (JT_LTORJ|JT_ಬಲ)) ){
        sCtx.iJoin = pItem->iCursor;
      }ಅನ್ಯಥಾ{
        ಪೂರ್ಣಾಙ್ಕ jj;
        ಪ್ರತಿ(jj=ii+1; jj<pSelect->pSrc->nSrc; jj++){
          SrcItem *pItem2 = &pSelect->pSrc->a[jj];
          ಯದಿ( pItem2->fg.jointype & JT_OUTER ){
            sCtx.iJoin = pItem2[-1].iCursor;
            ಭಙ್ಗ;
          }
        }
      }
      ಯದಿ( sCtx.iJoin>=0 ){
        sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&w, pItem->u1.pFuncArg);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sqlite3CopySortOrder(ಅಭಿವ್ಯಕ್ತಿ_List *p1, ಅಭಿವ್ಯಕ್ತಿ_List *p2){
  ಪ್ರತಿಜ್ಞಾ( p1 );
  ಯದಿ( p2 && p1->n_ಅಭಿವ್ಯಕ್ತಿ==p2->n_ಅಭಿವ್ಯಕ್ತಿ ){
    ಪೂರ್ಣಾಙ್ಕ ii;
    ಪ್ರತಿ(ii=0; ii<p1->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
      u8 sortFlags;
      sortFlags = p2->a[ii].fg.sortFlags & KEYINFO_ORDER_DESC;
      p1->a[ii].fg.sortFlags = sortFlags;
    }
    ಪ್ರತ್ಯಾಗಮ 1;
  }ಅನ್ಯಥಾ{
    ಪ್ರತ್ಯಾಗಮ 0;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3Select(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  SelectDest *pDest      /* What to do with the query results */
){
  ಪೂರ್ಣಾಙ್ಕ i, j;              /* Loop counters */
  WhereInfo *pWInfo;     /* Return from sqlite3WhereBegin() */
  Vdbe *v;               /* The virtual machine under construction */
  ಪೂರ್ಣಾಙ್ಕ isAgg;             /* True for select lists like "count(*)" */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList = 0;  /* List of columns to extract. */
  SrcList *pTabList;     /* List of tables to select from */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere;          /* The WHERE clause.  May be NULL */
  ಅಭಿವ್ಯಕ್ತಿ_List *pGroupBy;    /* The GROUP BY clause.  May be NULL */
  ಅಭಿವ್ಯಕ್ತಿ *pHaving;         /* The HAVING clause.  May be NULL */
  AggInfo *pAggInfo = 0; /* Aggregate information */
  ಪೂರ್ಣಾಙ್ಕ rc = 1;            /* Value to return from this function */
  DistinctCtx sDistinct; /* Info on how to code the DISTINCT keyword */
  SortCtx sSort;         /* Info on how to code the ORDER BY clause */
  ಪೂರ್ಣಾಙ್ಕ iEnd;              /* Address of the end of the query */
  sqlite3 *db;           /* The database connection */
  ಅಭಿವ್ಯಕ್ತಿ_List *pMinMaxOrderBy = 0;  /* Added ORDER BY for min/max queries */
  u8 minMaxFlag;                 /* Flag for min/max queries */

  db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( pParse==db->pParse );
  v = sqlite3GetVdbe(pParse);
  ಯದಿ( p==0 || pParse->nErr ){
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
  ಯದಿ( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) ಪ್ರತ್ಯಾಗಮ 1;
#ಯದಿ TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p, ("begin processing:\n", pParse->addrExplain));
  ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x10000 ){
    ಯದಿ( (sqlite3_ವೃಕ್ಷ_Trace & 0x10001)==0x10000 ){
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(0, "In sqlite3Select() at %s:%d",
                           __FILE__, __LINE__);
    }
    sqlite3ShowSelect(p);
  }
#endif

  /* tag-select-0100 */
  ಪ್ರತಿಜ್ಞಾ( p->pOrderBy==0 || pDest->eDest!=SRT_DistFifo );
  ಪ್ರತಿಜ್ಞಾ( p->pOrderBy==0 || pDest->eDest!=SRT_Fifo );
  ಪ್ರತಿಜ್ಞಾ( p->pOrderBy==0 || pDest->eDest!=SRT_DistQueue );
  ಪ್ರತಿಜ್ಞಾ( p->pOrderBy==0 || pDest->eDest!=SRT_Queue );
  ಯದಿ( IgnorableDistinct(pDest) ){
    ಪ್ರತಿಜ್ಞಾ(pDest->eDest==SRT_Exists     || pDest->eDest==SRT_Discard ||
           pDest->eDest==SRT_DistQueue  || pDest->eDest==SRT_DistFifo );
    /* All of these destinations are also able to ignore the ORDER BY clause */
    ಯದಿ( p->pOrderBy ){
#ಯದಿ TREETRACE_ENABLED
      TREETRACE(0x800,pParse,p, ("dropping superfluous ORDER BY:\n"));
      ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x800 ){
        sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(0, p->pOrderBy, 0, "ORDERBY");
      }
#endif   
      sqlite3ParserAddCleanup(pParse, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete_Generic,
                              p->pOrderBy);
      testcase( pParse->earlyCleanup );
      p->pOrderBy = 0;
    }
    p->selFlags &= ~(u32)SF_Distinct;
  }
  sqlite3SelectPrep(pParse, p, 0);
  ಯದಿ( pParse->nErr ){
    ಗಚ್ಛ select_end;
  }
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
  ಪ್ರತಿಜ್ಞಾ( p->pEList!=0 );
#ಯದಿ TREETRACE_ENABLED
  ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x10 ){
    TREETRACE(0x10,pParse,p, ("after name resolution:\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
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
  ಯದಿ( p->selFlags & SF_UFSrcCheck ){
    SrcItem *p0 = &p->pSrc->a[0];
    ಯದಿ( sameSrcAlias(p0, p->pSrc) ){
      sqlite3_Error_ಸಂದೇಶ(pParse,
          "target object/alias may not appear in FROM clause: %s",
          p0->zAlias ? p0->zAlias : p0->pSTab->zName
      );
      ಗಚ್ಛ select_end;
    }

    /* Clear the SF_UFSrcCheck flag. The check has already been performed,
    ** and leaving this flag set can cause errors if a compound sub-query
    ** in p->pSrc is flattened into this query and this function called
    ** again as part of compound SELECT processing.  */
    p->selFlags &= ~(u32)SF_UFSrcCheck;
  }

  ಯದಿ( pDest->eDest==SRT_Output ){
    sqlite3GenerateColumnNames(pParse, p);
  }

#ifndef SQLITE_OMIT_WINDOWFUNC
  ಯದಿ( sqlite3WindowRewrite(pParse, p) ){
    ಪ್ರತಿಜ್ಞಾ( pParse->nErr );
    ಗಚ್ಛ select_end;
  }
#ಯದಿ TREETRACE_ENABLED
  ಯದಿ( p->pWin && (sqlite3_ವೃಕ್ಷ_Trace & 0x40)!=0 ){
    TREETRACE(0x40,pParse,p, ("after window rewrite:\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
  }
#endif
#endif /* SQLITE_OMIT_WINDOWFUNC */
  pTabList = p->pSrc;
  isAgg = (p->selFlags & SF_Aggregate)!=0;
  memset(&sSort, 0, ಪರಿಮಾಣ(sSort));
  sSort.pOrderBy = p->pOrderBy;

  /* Try to do various optimizations (flattening subqueries, and strength
  ** reduction of join operators) in the FROM clause up into the main query
  ** tag-select-0200
  */
#ಯದಿ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
  ಪ್ರತಿ(i=0; !p->pPrior && i<pTabList->nSrc; i++){
    SrcItem *pItem = &pTabList->a[i];
    Select *pSub = pItem->fg.isSubquery ? pItem->u4.pSubq->pSelect : 0;
    Table *pTab = pItem->pSTab;

    /* The expander should have already created transient Table objects
    ** even for FROM clause elements such as subqueries that do not correspond
    ** to a real table */
    ಪ್ರತಿಜ್ಞಾ( pTab!=0 );

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
    ಯದಿ( (pItem->fg.jointype & (JT_ಎಡ|JT_LTORJ))!=0
     && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Implies_Non_Null_Row(p->pWhere, pItem->iCursor,
                                     pItem->fg.jointype & JT_LTORJ)
     && OptimizationEnabled(db, SQLITE_SimplifyJoin)
    ){
      ಯದಿ( pItem->fg.jointype & JT_ಎಡ ){
        ಯದಿ( pItem->fg.jointype & JT_ಬಲ ){
          TREETRACE(0x1000,pParse,p,
                    ("FULL-JOIN simplifies to RIGHT-JOIN on term %d\n",i));
          pItem->fg.jointype &= ~JT_ಎಡ;
        }ಅನ್ಯಥಾ{
          TREETRACE(0x1000,pParse,p,
                    ("LEFT-JOIN simplifies to JOIN on term %d\n",i));
          pItem->fg.jointype &= ~(JT_ಎಡ|JT_OUTER);
          unset_Join_ಅಭಿವ್ಯಕ್ತಿ(p->pWhere, pItem->iCursor, 0);
        }
      }
      ಯದಿ( pItem->fg.jointype & JT_LTORJ ){
        ಪ್ರತಿ(j=i+1; j<pTabList->nSrc; j++){
          SrcItem *pI2 = &pTabList->a[j];
          ಯದಿ( pI2->fg.jointype & JT_ಬಲ ){
            ಯದಿ( pI2->fg.jointype & JT_ಎಡ ){
              TREETRACE(0x1000,pParse,p,
                        ("FULL-JOIN simplifies to LEFT-JOIN on term %d\n",j));
              pI2->fg.jointype &= ~JT_ಬಲ;
            }ಅನ್ಯಥಾ{
              TREETRACE(0x1000,pParse,p,
                        ("RIGHT-JOIN simplifies to JOIN on term %d\n",j));
              pI2->fg.jointype &= ~(JT_ಬಲ|JT_OUTER);
              unset_Join_ಅಭಿವ್ಯಕ್ತಿ(p->pWhere, pI2->iCursor, 1);
            }
          }
        }
        ಪ್ರತಿ(j=pTabList->nSrc-1; j>=0; j--){
          pTabList->a[j].fg.jointype &= ~JT_LTORJ;
          ಯದಿ( pTabList->a[j].fg.jointype & JT_ಬಲ ) ಭಙ್ಗ;
        }
      }
    }

    /* No further action if this term of the FROM clause is not a subquery */
    ಯದಿ( pSub==0 ) ಅನುವರ್ತ;

    /* Catch mismatch in the declared columns of a view and the number of
    ** columns in the SELECT on the RHS */
    ಯದಿ( pTab->nCol!=pSub->pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "expected %d columns for '%s' but got %d",
                      pTab->nCol, pTab->zName, pSub->pEList->n_ಅಭಿವ್ಯಕ್ತಿ);
      ಗಚ್ಛ select_end;
    }

    /* Do not attempt the usual optimizations (flattening and ORDER BY
    ** elimination) on a MATERIALIZED common table expression because
    ** a MATERIALIZED common table expression is an optimization fence.
    */
    ಯದಿ( pItem->fg.isCte && pItem->u2.pCteUse->eM10d==M10d_Yes ){
      ಅನುವರ್ತ;
    }

    /* Do not try to flatten an aggregate subquery.
    **
    ** Flattening an aggregate subquery is only possible if the outer query
    ** is not a join.  But if the outer query is not a join, then the subquery
    ** will be implemented as a co-routine and there is no advantage to
    ** flattening in that case.
    */
    ಯದಿ( (pSub->selFlags & SF_Aggregate)!=0 ) ಅನುವರ್ತ;
    ಪ್ರತಿಜ್ಞಾ( pSub->pGroupBy==0 );

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
    ಯದಿ( pSub->pOrderBy!=0
     && (p->pOrderBy!=0 || pTabList->nSrc>1)      /* Condition (5) */
     && pSub->pLimit==0                           /* Condition (1) */
     && (pSub->selFlags & (SF_OrderByReqd|SF_Recursive))==0  /* (2) and (6) */
     && (p->selFlags & SF_OrderByReqd)==0         /* Condition (3) and (4) */
     && OptimizationEnabled(db, SQLITE_OmitOrderBy)
    ){
      TREETRACE(0x800,pParse,p,
                ("omit superfluous ORDER BY on %r FROM-clause subquery\n",i+1));
      sqlite3ParserAddCleanup(pParse, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete_Generic,
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
    ಯದಿ( pSub->pOrderBy!=0
     && i==0
     && (p->selFlags & SF_ComplexResult)!=0
     && (pTabList->nSrc==1
         || (pTabList->a[1].fg.jointype&(JT_OUTER|JT_CROSS))!=0)
    ){
      ಅನುವರ್ತ;
    }

    /* tag-select-0240 */
    ಯದಿ( flattenSubquery(pParse, p, i, isAgg) ){
      ಯದಿ( pParse->nErr ) ಗಚ್ಛ select_end;
      /* This subquery can be absorbed into its parent. */
      i = -1;
    }
    pTabList = p->pSrc;
    ಯದಿ( db->mallocFailed ) ಗಚ್ಛ select_end;
    ಯದಿ( !IgnorableOrderby(pDest) ){
      sSort.pOrderBy = p->pOrderBy;
    }
  }
#endif

#ifndef SQLITE_OMIT_COMPOUND_SELECT
  /* Handle compound SELECT statements using the separate multiSelect()
  ** procedure.  tag-select-0300
  */
  ಯದಿ( p->pPrior ){
    rc = multiSelect(pParse, p, pDest);
#ಯದಿ TREETRACE_ENABLED
    TREETRACE(0x400,pParse,p,("end compound-select processing\n"));
    ಯದಿ( (sqlite3_ವೃಕ್ಷ_Trace & 0x400)!=0 && ExplainQueryPlanParent(pParse)==0 ){
      sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
    }
#endif
    ಯದಿ( p->pNext==0 ) Explain_Query_Plan_ತೆಗೆ(pParse);
    ಪ್ರತ್ಯಾಗಮ rc;
  }
#endif

  /* If there may be an "EXISTS (SELECT ...)" in the WHERE clause, attempt
  ** to change it into a join.  */
  ಯದಿ( pParse->bHasExists && OptimizationEnabled(db,SQLITE_ExistsToJoin) ){
    existsToJoin(pParse, p, p->pWhere);
    pTabList = p->pSrc;
  }

  /* Do the WHERE-clause constant propagation optimization if this is
  ** a join.  No need to spend time on this operation for non-join queries
  ** as the equivalent optimization will be handled by query planner in
  ** sqlite3WhereBegin().  tag-select-0330
  */
  ಯದಿ( p->pWhere!=0
   && p->pWhere->op==TK_AND
   && OptimizationEnabled(db, SQLITE_PropagateConst)
   && propagateConstants(pParse, p)
  ){
#ಯದಿ TREETRACE_ENABLED
    ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x2000 ){
      TREETRACE(0x2000,pParse,p,("After constant propagation:\n"));
      sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
    }
#endif
  }ಅನ್ಯಥಾ{
    TREETRACE(0x2000,pParse,p,("Constant propagation not helpful\n"));
  }

  /* tag-select-0350 */
  ಯದಿ( OptimizationEnabled(db, SQLITE_QueryFlattener|SQLITE_ಎಣಿಕೆ_Of_View)
   && ಎಣಿಕೆ_Of_View_Optimization(pParse, p)
  ){
    ಯದಿ( db->mallocFailed ) ಗಚ್ಛ select_end;
    pTabList = p->pSrc;
  }

  /* Loop over all terms in the FROM clause and do two things for each term:
  **
  **   (1) Authorize unreferenced tables
  **   (2) Generate code for all sub-queries
  **
  ** tag-select-0400
  */
  ಪ್ರತಿ(i=0; i<pTabList->nSrc; i++){
    SrcItem *pItem = &pTabList->a[i];
    SrcItem *pPrior;
    SelectDest dest;
    Subquery *pSubq;
    Select *pSub;
#ಯದಿ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    ಧ್ರುವ ವರ್ಣ *zSavedAuthContext;
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
    ಯದಿ( pItem->colUsed==0 && pItem->zName!=0 ){
      ಧ್ರುವ ವರ್ಣ *zDb;
      ಯದಿ( pItem->fg.fixedSchema ){
        ಪೂರ್ಣಾಙ್ಕ iDb = sqlite3SchemaToIndex(pParse->db, pItem->u4.pSchema);
        zDb = db->aDb[iDb].zDbSName;
      }ಅನ್ಯಥಾ ಯದಿ( pItem->fg.isSubquery ){
        zDb = 0;
      }ಅನ್ಯಥಾ{
        zDb = pItem->u4.zDatabase;
      }
      sqlite3AuthCheck(pParse, SQLITE_READ, pItem->zName, "", zDb);
    }

#ಯದಿ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    /* Generate code for all sub-queries in the FROM clause
    */
    ಯದಿ( pItem->fg.isSubquery==0 ) ಅನುವರ್ತ;
    pSubq = pItem->u4.pSubq;
    ಪ್ರತಿಜ್ಞಾ( pSubq!=0 );
    pSub = pSubq->pSelect;

    /* The code for a subquery should only be generated once. */
    ಯದಿ( pSubq->addrFillSub!=0 ) ಅನುವರ್ತ;

    /* Increment Parse.nHeight by the height of the largest expression
    ** tree referred to by this, the parent select. The child select
    ** may contain expression trees of at most
    ** (SQLITE_MAX_EXPR_DEPTH-Parse.nHeight) height. This is a bit
    ** more conservative than necessary, but much easier than enforcing
    ** an exact limit.
    */
    pParse->nHeight += sqlite3_Select_ಅಭಿವ್ಯಕ್ತಿ_Height(p);

    /* Make copies of constant WHERE-clause terms in the outer query down
    ** inside the subquery.  This can help the subquery to run more efficiently.
    ** This is the "predicate push-down optimization".  tag-select-0420
    */
    ಯದಿ( OptimizationEnabled(db, SQLITE_ತಳ್ಳು_Down)
     && (pItem->fg.isCte==0
         || (pItem->u2.pCteUse->eM10d!=M10d_Yes && pItem->u2.pCteUse->nUse<2))
     && ತಳ್ಳು_Down_Where_Terms(pParse, pSub, p->pWhere, pTabList, i)
    ){
#ಯದಿ TREETRACE_ENABLED
      ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("After WHERE-clause push-down into subquery %d:\n", pSub->selId));
        sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
      }
#endif
      ಪ್ರತಿಜ್ಞಾ( pSubq->pSelect && (pSub->selFlags & SF_ತಳ್ಳು_Down)!=0 );
    }ಅನ್ಯಥಾ{
      TREETRACE(0x4000,pParse,p,("WHERE-clause push-down not possible\n"));
    }

    /* Convert unused result columns of the subquery into simple NULL
    ** expressions, to avoid unneeded searching and computation.
    ** tag-select-0440
    */
    ಯದಿ( OptimizationEnabled(db, SQLITE_NullUnusedCols)
     && disableUnusedSubqueryResultColumns(pItem)
    ){
#ಯದಿ TREETRACE_ENABLED
      ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("Change unused result columns to NULL for subquery %d:\n",
             pSub->selId));
        sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
      }
#endif
    }

    zSavedAuthContext = pParse->zAuthContext;
    pParse->zAuthContext = pItem->zName;

    /* Generate byte-code to implement the subquery  tag-select-0480
    */
    ಯದಿ( fromClauseTermCanBeCoroutine(pParse, pTabList, i, p->selFlags) ){
      /* Implement a co-routine that will return a single row of the result
      ** set on each invocation.  tag-select-0482
      */
      ಪೂರ್ಣಾಙ್ಕ addr_ತುದಿ = sqlite3VdbeCurrentAddr(v)+1;
    
      pSubq->regReturn = ++pParse->nMem;
      sqlite3VdbeAddOp3(v, OP_InitCoroutine, pSubq->regReturn, 0, addr_ತುದಿ);
      VdbeComment((v, "%!S", pItem));
      pSubq->addrFillSub = addr_ತುದಿ;
      sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);
      ExplainQueryPlan((pParse, 1, "CO-ROUTINE %!S", pItem));
      sqlite3Select(pParse, pSub, &dest);
      pItem->pSTab->nRowLogEst = pSub->nSelectRow;
      pItem->fg.viaCoroutine = 1;
      pSubq->regResult = dest.iSdst;
      sqlite3VdbeEndCoroutine(v, pSubq->regReturn);
      VdbeComment((v, "end %!S", pItem));
      sqlite3VdbeJumpHere(v, addr_ತುದಿ-1);
      sqlite3ClearTempRegCache(pParse);
    }ಅನ್ಯಥಾ ಯದಿ( pItem->fg.isCte && pItem->u2.pCteUse->addrM9e>0 ){
      /* This is a CTE for which materialization code has already been
      ** generated.  Invoke the subroutine to compute the materialization,
      ** then make the pItem->iCursor be a copy of the ephemeral table that
      ** holds the result of the materialization. tag-select-0484 */
      CteUse *pCteUse = pItem->u2.pCteUse;
      sqlite3VdbeAddOp2(v, OP_Gosub, pCteUse->regRtn, pCteUse->addrM9e);
      ಯದಿ( pItem->iCursor!=pCteUse->iCur ){
        sqlite3VdbeAddOp2(v, OP_OpenDup, pItem->iCursor, pCteUse->iCur);
        VdbeComment((v, "%!S", pItem));
      }
      pSub->nSelectRow = pCteUse->nRowEst;
    }ಅನ್ಯಥಾ ಯದಿ( (pPrior = isSelfJoinView(pTabList, pItem, 0, i))!=0 ){
      /* This view has already been materialized by a prior entry in
      ** this same FROM clause.  Reuse it.  tag-select-0486 */
      Subquery *pPriorSubq;
      ಪ್ರತಿಜ್ಞಾ( pPrior->fg.isSubquery );
      pPriorSubq = pPrior->u4.pSubq;
      ಪ್ರತಿಜ್ಞಾ( pPriorSubq!=0 );
      ಯದಿ( pPriorSubq->addrFillSub ){
        sqlite3VdbeAddOp2(v, OP_Gosub, pPriorSubq->regReturn,
                                       pPriorSubq->addrFillSub);
      }
      sqlite3VdbeAddOp2(v, OP_OpenDup, pItem->iCursor, pPrior->iCursor);
      pSub->nSelectRow = pPriorSubq->pSelect->nSelectRow;
    }ಅನ್ಯಥಾ{
      /* Materialize the view.  If the view is not correlated, generate a
      ** subroutine to do the materialization so that subsequent uses of
      ** the same view can reuse the materialization.  tag-select-0488 */
      ಪೂರ್ಣಾಙ್ಕ ತುದಿ_Addr;
      ಪೂರ್ಣಾಙ್ಕ onceAddr = 0;
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
      ಪೂರ್ಣಾಙ್ಕ addrExplain;
#endif

      pSubq->regReturn = ++pParse->nMem;
      ತುದಿ_Addr = sqlite3VdbeAddOp0(v, OP_Goto);
      pSubq->addrFillSub = ತುದಿ_Addr+1;
      pItem->fg.isMaterialized = 1;
      ಯದಿ( pItem->fg.isCorrelated==0 ){
        /* If the subquery is not correlated and if we are not inside of
        ** a trigger, then we only need to compute the value of the subquery
        ** once. */
        onceAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
        VdbeComment((v, "materialize %!S", pItem));
      }ಅನ್ಯಥಾ{
        VdbeNoopComment((v, "materialize %!S", pItem));
      }
      sqlite3SelectDestInit(&dest, SRT_EphemTab, pItem->iCursor);

      ExplainQueryPlan2(addrExplain, (pParse, 1, "MATERIALIZE %!S", pItem));
      sqlite3Select(pParse, pSub, &dest);
      pItem->pSTab->nRowLogEst = pSub->nSelectRow;
      ಯದಿ( onceAddr ) sqlite3VdbeJumpHere(v, onceAddr);
      sqlite3VdbeAddOp2(v, OP_Return, pSubq->regReturn, ತುದಿ_Addr+1);
      VdbeComment((v, "end %!S", pItem));
      sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);
      sqlite3VdbeJumpHere(v, ತುದಿ_Addr);
      sqlite3ClearTempRegCache(pParse);
      ಯದಿ( pItem->fg.isCte && pItem->fg.isCorrelated==0 ){
        CteUse *pCteUse = pItem->u2.pCteUse;
        pCteUse->addrM9e = pSubq->addrFillSub;
        pCteUse->regRtn = pSubq->regReturn;
        pCteUse->iCur = pItem->iCursor;
        pCteUse->nRowEst = pSub->nSelectRow;
      }
    }
    ಯದಿ( db->mallocFailed ) ಗಚ್ಛ select_end;
    pParse->nHeight -= sqlite3_Select_ಅಭಿವ್ಯಕ್ತಿ_Height(p);
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

#ಯದಿ TREETRACE_ENABLED
  ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x8000 ){
    TREETRACE(0x8000,pParse,p,("After all FROM-clause analysis:\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
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
  ಯದಿ( (p->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct
   && sqlite3CopySortOrder(pEList, sSort.pOrderBy)
   && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Compare(pEList, sSort.pOrderBy, -1)==0
   && OptimizationEnabled(db, SQLITE_GroupByOrder)
#ifndef SQLITE_OMIT_WINDOWFUNC
   && p->pWin==0
#endif
  ){
    p->selFlags &= ~(u32)SF_Distinct;
    pGroupBy = p->pGroupBy = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pEList, 0);
    ಯದಿ( pGroupBy ){
      ಪ್ರತಿ(i=0; i<pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
        pGroupBy->a[i].u.x.iOrderByCol = i+1;
      }
    }
    p->selFlags |= SF_Aggregate;
    /* Notice that even thought SF_Distinct has been cleared from p->selFlags,
    ** the sDistinct.isTnct is still set.  Hence, isTnct represents the
    ** original setting of the SF_Distinct flag, not the current setting */
    ಪ್ರತಿಜ್ಞಾ( sDistinct.isTnct );
    sDistinct.isTnct = 2;

#ಯದಿ TREETRACE_ENABLED
    ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x20000 ){
      TREETRACE(0x20000,pParse,p,("Transform DISTINCT into GROUP BY:\n"));
      sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
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
  ಯದಿ( sSort.pOrderBy ){
    ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;
    p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_From_ಅಭಿವ್ಯಕ್ತಿ_List(
        pParse, sSort.pOrderBy, 0, pEList->n_ಅಭಿವ್ಯಕ್ತಿ);
    sSort.iECursor = pParse->nTab++;
    sSort.addrSortIndex =
      sqlite3VdbeAddOp4(v, OP_OpenEphemeral,
          sSort.iECursor, sSort.pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ+1+pEList->n_ಅಭಿವ್ಯಕ್ತಿ, 0,
          (ವರ್ಣ*)p_ಕೀಲಿ_Info, P4_KEYINFO
      );
  }ಅನ್ಯಥಾ{
    sSort.addrSortIndex = -1;
  }

  /* If the output is destined for a temporary table, open that table.
  ** tag-select-0630
  */
  ಯದಿ( pDest->eDest==SRT_EphemTab ){
    sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pDest->iSDParm, pEList->n_ಅಭಿವ್ಯಕ್ತಿ);
    ಯದಿ( p->selFlags & SF_NestedFrom ){
      /* Delete or NULL-out result columns that will never be used */
      ಪೂರ್ಣಾಙ್ಕ ii;
      ಪ್ರತಿ(ii=pEList->n_ಅಭಿವ್ಯಕ್ತಿ-1; ii>0 && pEList->a[ii].fg.bUsed==0; ii--){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pEList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ);
        sqlite3DbFree(db, pEList->a[ii].zEName);
        pEList->n_ಅಭಿವ್ಯಕ್ತಿ--;
      }
      ಪ್ರತಿ(ii=0; ii<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
        ಯದಿ( pEList->a[ii].fg.bUsed==0 ) pEList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_NULL;
      }
    }
  }

  /* Set the limiter.  tag-select-0650
  */
  iEnd = sqlite3VdbeMakeLabel(pParse);
  ಯದಿ( (p->selFlags & SF_FixedLimit)==0 ){
    p->nSelectRow = 320;  /* 4 billion rows */
  }
  ಯದಿ( p->pLimit ) computeLimitRegisters(pParse, p, iEnd);
  ಯದಿ( p->iLimit==0 && sSort.addrSortIndex>=0 ){
    sqlite3VdbeChangeOpcode(v, sSort.addrSortIndex, OP_SorterOpen);
    sSort.sortFlags |= SORTFLAG_UseSorter;
  }

  /* Open an ephemeral index to use for the distinct set. tag-select-0680
  */
  ಯದಿ( p->selFlags & SF_Distinct ){
    sDistinct.tabTnct = pParse->nTab++;
    sDistinct.addrTnct = sqlite3VdbeAddOp4(v, OP_OpenEphemeral,
                       sDistinct.tabTnct, 0, 0,
                       (ವರ್ಣ*)sqlite3_ಕೀಲಿ_Info_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, p->pEList,0,0),
                       P4_KEYINFO);
    sqlite3VdbeChangeP5(v, BTREE_UNORDERED);
    sDistinct.eTnctType = WHERE_DISTINCT_UNORDERED;
  }ಅನ್ಯಥಾ{
    sDistinct.eTnctType = WHERE_DISTINCT_NOOP;
  }

  ಯದಿ( !isAgg && pGroupBy==0 ){
    /* No aggregate functions and no GROUP BY clause.  tag-select-0700 */
    u16 wctrlFlags = (sDistinct.isTnct ? WHERE_WANT_DISTINCT : 0)
                   | (p->selFlags & SF_FixedLimit);
#ifndef SQLITE_OMIT_WINDOWFUNC
    Window *pWin = p->pWin;      /* Main window object (or NULL) */
    ಯದಿ( pWin ){
      sqlite3WindowCodeInit(pParse, p);
    }
#endif
    ಪ್ರತಿಜ್ಞಾ( WHERE_USE_LIMIT==SF_FixedLimit );


    /* Begin the database scan. */
    TREETRACE(0x2,pParse,p,("WhereBegin\n"));
    pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, sSort.pOrderBy,
                               p->pEList, p, wctrlFlags, p->nSelectRow);
    ಯದಿ( pWInfo==0 ) ಗಚ್ಛ select_end;
    ಯದಿ( sqlite3_Where_Output_Row_ಎಣಿಕೆ(pWInfo) < p->nSelectRow ){
      p->nSelectRow = sqlite3_Where_Output_Row_ಎಣಿಕೆ(pWInfo);
      ಯದಿ( pDest->eDest<=SRT_DistQueue && pDest->eDest>=SRT_DistFifo ){
        /* TUNING: For a UNION CTE, because UNION is implies DISTINCT,
        ** reduce the estimated output row count by 8 (LogEst 30). 
        ** Search for tag-20250414a to see other cases */
        p->nSelectRow -= 30;
      }
    }
    ಯದಿ( sDistinct.isTnct && sqlite3WhereIsDistinct(pWInfo) ){
      sDistinct.eTnctType = sqlite3WhereIsDistinct(pWInfo);
    }
    ಯದಿ( sSort.pOrderBy ){
      sSort.nOBSat = sqlite3WhereIsOrdered(pWInfo);
      sSort.labelOBLopt = sqlite3WhereOrderByLimitOptLabel(pWInfo);
      ಯದಿ( sSort.nOBSat==sSort.pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ ){
        sSort.pOrderBy = 0;
      }
    }
    TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));

    /* If sorting index that was created by a prior OP_OpenEphemeral
    ** instruction ended up not being needed, then change the OP_OpenEphemeral
    ** into an OP_Noop.
    */
    ಯದಿ( sSort.addrSortIndex>=0 && sSort.pOrderBy==0 ){
      sqlite3VdbeChangeToNoop(v, sSort.addrSortIndex);
    }

    ಪ್ರತಿಜ್ಞಾ( p->pEList==pEList );
#ifndef SQLITE_OMIT_WINDOWFUNC
    ಯದಿ( pWin ){
      ಪೂರ್ಣಾಙ್ಕ addrGosub = sqlite3VdbeMakeLabel(pParse);
      ಪೂರ್ಣಾಙ್ಕ iCont = sqlite3VdbeMakeLabel(pParse);
      ಪೂರ್ಣಾಙ್ಕ iBreak = sqlite3VdbeMakeLabel(pParse);
      ಪೂರ್ಣಾಙ್ಕ regGosub = ++pParse->nMem;

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
    }ಅನ್ಯಥಾ
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
  }ಅನ್ಯಥಾ{
    /* This case is for when there exist aggregate functions or a GROUP BY
    ** clause or both.  tag-select-0800 */
    NameContext sNC;    /* Name context for processing aggregate information */
    ಪೂರ್ಣಾಙ್ಕ iAMem;          /* First Mem address for storing current GROUP BY */
    ಪೂರ್ಣಾಙ್ಕ iBMem;          /* First Mem address for previous GROUP BY */
    ಪೂರ್ಣಾಙ್ಕ iUseFlag;       /* Mem address holding flag indicating that at least
                        ** one row of the input to the aggregator has been
                        ** processed */
    ಪೂರ್ಣಾಙ್ಕ iAbortFlag;     /* Mem address which causes query abort if positive */
    ಪೂರ್ಣಾಙ್ಕ groupBySort;    /* Rows come from source in GROUP BY order */
    ಪೂರ್ಣಾಙ್ಕ addrEnd;        /* End of processing for this SELECT */
    ಪೂರ್ಣಾಙ್ಕ sortPTab = 0;   /* Pseudotable used to decode sorting results */
    ಪೂರ್ಣಾಙ್ಕ sortOut = 0;    /* Output register from the sorter */
    ಪೂರ್ಣಾಙ್ಕ orderByGrp = 0; /* True if the GROUP BY and ORDER BY are the same */

    /* Remove any and all aliases between the result set and the
    ** GROUP BY clause.
    */
    ಯದಿ( pGroupBy ){
      ಪೂರ್ಣಾಙ್ಕ k;                        /* Loop counter */
      ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;  /* For looping over expression in a list */

      ಪ್ರತಿ(k=p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ, pItem=p->pEList->a; k>0; k--, pItem++){
        pItem->u.x.iAlias = 0;
      }
      ಪ್ರತಿ(k=pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ, pItem=pGroupBy->a; k>0; k--, pItem++){
        pItem->u.x.iAlias = 0;
      }
      ಪ್ರತಿಜ್ಞಾ( 66==sqlite3LogEst(100) );
      ಯದಿ( p->nSelectRow>66 ) p->nSelectRow = 66;

      /* If there is both a GROUP BY and an ORDER BY clause and they are
      ** identical, then it may be possible to disable the ORDER BY clause
      ** on the grounds that the GROUP BY will cause elements to come out
      ** in the correct order. It also may not - the GROUP BY might use a
      ** database index that causes rows to be grouped together as required
      ** but not actually sorted. Either way, record the fact that the
      ** ORDER BY and GROUP BY clauses are the same by setting the orderByGrp
      ** variable.  */
      ಯದಿ( sqlite3CopySortOrder(pGroupBy, sSort.pOrderBy)
       && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Compare(pGroupBy, sSort.pOrderBy, -1)==0
      ){
        orderByGrp = 1;
      }
    }ಅನ್ಯಥಾ{
      ಪ್ರತಿಜ್ಞಾ( 0==sqlite3LogEst(1) );
      p->nSelectRow = 0;
    }

    /* Create a label to jump to when we want to abort the query */
    addrEnd = sqlite3VdbeMakeLabel(pParse);

    /* Convert TK_COLUMN nodes into TK_AGG_COLUMN and make entries in
    ** sAggInfo for all TK_AGG_FUNCTION nodes in expressions of the
    ** SELECT statement.
    */
    pAggInfo = sqlite3DbMallocZero(db, ಪರಿಮಾಣ(*pAggInfo) );
    ಯದಿ( pAggInfo ){
      sqlite3ParserAddCleanup(pParse, agginfoFree, pAggInfo);
      testcase( pParse->earlyCleanup );
    }
    ಯದಿ( db->mallocFailed ){
      ಗಚ್ಛ select_end;
    }
    pAggInfo->selId = p->selId;
#ifdef SQLITE_DEBUG
    pAggInfo->pSelect = p;
#endif
    memset(&sNC, 0, ಪರಿಮಾಣ(sNC));
    sNC.pParse = pParse;
    sNC.pSrcList = pTabList;
    sNC.uNC.pAggInfo = pAggInfo;
    VVA_ONLY( sNC.ncFlags = NC_UAggInfo; )
    pAggInfo->nSortingColumn = pGroupBy ? pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
    pAggInfo->pGroupBy = pGroupBy;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Agg_List(&sNC, pEList);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Agg_List(&sNC, sSort.pOrderBy);
    ಯದಿ( pHaving ){
      ಯದಿ( pGroupBy ){
        ಪ್ರತಿಜ್ಞಾ( pWhere==p->pWhere );
        ಪ್ರತಿಜ್ಞಾ( pHaving==p->pHaving );
        ಪ್ರತಿಜ್ಞಾ( pGroupBy==p->pGroupBy );
        havingToWhere(pParse, p);
        pWhere = p->pWhere;
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Aggregates(&sNC, pHaving);
    }
    pAggInfo->nAccumulator = pAggInfo->nColumn;
    ಯದಿ( p->pGroupBy==0 && p->pHaving==0 && pAggInfo->nFunc==1 ){
      minMaxFlag = minMaxQuery(db, pAggInfo->aFunc[0].pFExpr, &pMinMaxOrderBy);
    }ಅನ್ಯಥಾ{
      minMaxFlag = WHERE_ORDERBY_NORMAL;
    }
    analyzeAggFuncArgs(pAggInfo, &sNC);
    ಯದಿ( db->mallocFailed ) ಗಚ್ಛ select_end;
#ಯದಿ TREETRACE_ENABLED
    ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("After aggregate analysis %p:\n", pAggInfo));
      sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
      ಯದಿ( minMaxFlag ){
        sqlite3DebugPrintf("MIN/MAX Optimization (0x%02x) adds:\n", minMaxFlag);
        sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(0, pMinMaxOrderBy, 0, "ORDERBY");
      }
      printAggInfo(pAggInfo);
    }
#endif


    /* Processing for aggregates with GROUP BY is very different and
    ** much more complex than aggregates without a GROUP BY.  tag-select-0810
    */
    ಯದಿ( pGroupBy ){
      ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;  /* Keying information for the group by clause */
      ಪೂರ್ಣಾಙ್ಕ addr1;          /* A-vs-B comparison jump */
      ಪೂರ್ಣಾಙ್ಕ addrOutputRow;  /* Start of subroutine that outputs a result row */
      ಪೂರ್ಣಾಙ್ಕ regOutputRow;   /* Return address register for output subroutine */
      ಪೂರ್ಣಾಙ್ಕ addrSetAbort;   /* Set the abort flag and return */
      ಪೂರ್ಣಾಙ್ಕ addr_ತುದಿ_Of_Loop;  /* Top of the input loop */
      ಪೂರ್ಣಾಙ್ಕ addrSortingIdx; /* The OP_OpenEphemeral for the sorting index */
      ಪೂರ್ಣಾಙ್ಕ addrReset;      /* Subroutine for resetting the accumulator */
      ಪೂರ್ಣಾಙ್ಕ regReset;       /* Return address register for reset subroutine */
      ಅಭಿವ್ಯಕ್ತಿ_List *pDistinct = 0;
      u16 distFlag = 0;
      ಪೂರ್ಣಾಙ್ಕ eDist = WHERE_DISTINCT_NOOP;

      ಯದಿ( pAggInfo->nFunc==1
       && pAggInfo->aFunc[0].iDistinct>=0
       && ALWAYS(pAggInfo->aFunc[0].pFExpr!=0)
       && ALWAYS(ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pAggInfo->aFunc[0].pFExpr))
       && pAggInfo->aFunc[0].pFExpr->x.pList!=0
      ){
        ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pAggInfo->aFunc[0].pFExpr->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
        p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
        pDistinct = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pGroupBy, 0);
        pDistinct = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pDistinct, p_ಅಭಿವ್ಯಕ್ತಿ);
        distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
      }

      /* If there is a GROUP BY clause we might need a sorting index to
      ** implement it.  Allocate that sorting index now.  If it turns out
      ** that we do not need it after all, the OP_SorterOpen instruction
      ** will be converted into a Noop. 
      */
      pAggInfo->sortingIdx = pParse->nTab++;
      p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pGroupBy,
                                            0, pAggInfo->nColumn);
      addrSortingIdx = sqlite3VdbeAddOp4(v, OP_SorterOpen,
          pAggInfo->sortingIdx, pAggInfo->nSortingColumn,
          0, (ವರ್ಣ*)p_ಕೀಲಿ_Info, P4_KEYINFO);

      /* Initialize memory locations used by GROUP BY aggregate processing
      */
      iUseFlag = ++pParse->nMem;
      iAbortFlag = ++pParse->nMem;
      regOutputRow = ++pParse->nMem;
      addrOutputRow = sqlite3VdbeMakeLabel(pParse);
      regReset = ++pParse->nMem;
      addrReset = sqlite3VdbeMakeLabel(pParse);
      iAMem = pParse->nMem + 1;
      pParse->nMem += pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ;
      iBMem = pParse->nMem + 1;
      pParse->nMem += pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ;
      sqlite3VdbeAddOp2(v, OP_Integer, 0, iAbortFlag);
      VdbeComment((v, "clear abort flag"));
      sqlite3VdbeAddOp3(v, OP_Null, 0, iAMem, iAMem+pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ-1);
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Null_Register_Range(pParse, iAMem, pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ);

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
      ಯದಿ( pWInfo==0 ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pDistinct);
        ಗಚ್ಛ select_end;
      }
      ಯದಿ( pParse->pIdxEpr ){
        optimize_Aggregate_Use_Of_Indexed_ಅಭಿವ್ಯಕ್ತಿ(pParse, p, pAggInfo, &sNC);
      }
      assignAggregateRegisters(pParse, pAggInfo);
      eDist = sqlite3WhereIsDistinct(pWInfo);
      TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
      ಯದಿ( sqlite3WhereIsOrdered(pWInfo)==pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ ){
        /* The optimizer is able to deliver rows in group by order so
        ** we do not have to sort.  The OP_OpenEphemeral table will be
        ** cancelled later because we still need to use the pKeyInfo
        */
        groupBySort = 0;
      }ಅನ್ಯಥಾ{
        /* Rows are coming out in undetermined order.  We have to push
        ** each row into a sorting index, terminate the first loop,
        ** then loop over the sorting index in order to get the output
        ** in sorted order
        */
        ಪೂರ್ಣಾಙ್ಕ regBase;
        ಪೂರ್ಣಾಙ್ಕ regRecord;
        ಪೂರ್ಣಾಙ್ಕ nCol;
        ಪೂರ್ಣಾಙ್ಕ nGroupBy;

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
        ಪೂರ್ಣಾಙ್ಕ addrExp;              /* Address of OP_Explain instruction */
#endif
        ExplainQueryPlan2(addrExp, (pParse, 0, "USE TEMP B-TREE FOR %s",
            (sDistinct.isTnct && (p->selFlags&SF_Distinct)==0) ?
                    "DISTINCT" : "GROUP BY"
        ));

        groupBySort = 1;
        nGroupBy = pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ;
        nCol = nGroupBy;
        j = nGroupBy;
        ಪ್ರತಿ(i=0; i<pAggInfo->nColumn; i++){
          ಯದಿ( pAggInfo->aCol[i].iSorterColumn>=j ){
            nCol++;
            j++;
          }
        }
        regBase = sqlite3GetTempRange(pParse, nCol);
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pGroupBy, regBase, 0, 0);
        j = nGroupBy;
        pAggInfo->directMode = 1;
        ಪ್ರತಿ(i=0; i<pAggInfo->nColumn; i++){
          ಸಂರಚನಾ AggInfo_col *pCol = &pAggInfo->aCol[i];
          ಯದಿ( pCol->iSorterColumn>=j ){
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, pCol->pCExpr, j + regBase);
            j++;
          }
        }
        pAggInfo->directMode = 0;
        regRecord = sqlite3GetTempReg(pParse);
        sqlite3VdbeScanStatusCounters(v, addrExp, 0, sqlite3VdbeCurrentAddr(v));
        sqlite3VdbeAddOp3(v, OP_MakeRecord, regBase, nCol, regRecord);
        sqlite3VdbeAddOp2(v, OP_Sorter_ಸೇರಿಸು, pAggInfo->sortingIdx, regRecord);
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
      ಯದಿ( pParse->pIdxEpr ){
        aggregate_Convert_Indexed_ಅಭಿವ್ಯಕ್ತಿ_Ref_To_Column(pAggInfo);
#ಯದಿ TREETRACE_ENABLED
        ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x20 ){
          TREETRACE(0x20, pParse, p,
             ("AggInfo function expressions converted to reference index\n"));
          sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
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
      ಯದಿ( orderByGrp && OptimizationEnabled(db, SQLITE_GroupByOrder)
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
      addr_ತುದಿ_Of_Loop = sqlite3VdbeCurrentAddr(v);
      ಯದಿ( groupBySort ){
        sqlite3VdbeAddOp3(v, OP_SorterData, pAggInfo->sortingIdx,
                          sortOut, sortPTab);
      }
      ಪ್ರತಿ(j=0; j<pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
        ಪೂರ್ಣಾಙ್ಕ iOrderByCol = pGroupBy->a[j].u.x.iOrderByCol;
        
        ಯದಿ( groupBySort ){
          sqlite3VdbeAddOp3(v, OP_Column, sortPTab, j, iBMem+j);
        }ಅನ್ಯಥಾ{
          pAggInfo->directMode = 1;
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code(pParse, pGroupBy->a[j].p_ಅಭಿವ್ಯಕ್ತಿ, iBMem+j);
        }

        ಯದಿ( iOrderByCol ){
          ಅಭಿವ್ಯಕ್ತಿ *pX = p->pEList->a[iOrderByCol-1].p_ಅಭಿವ್ಯಕ್ತಿ;
          ಅಭಿವ್ಯಕ್ತಿ *pBase = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(pX);
          ಯಾವತ್( ALWAYS(pBase!=0) && pBase->op==TK_IF_NULL_ROW ){
            pX = pBase->p_ಎಡ;
            pBase = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(pX);
          }
          ಯದಿ( ALWAYS(pBase!=0)
           && pBase->op!=TK_AGG_COLUMN
           && pBase->op!=TK_REGISTER
          ){
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_To_Register(pX, iAMem+j);
          }
        }
      }
      sqlite3VdbeAddOp4(v, OP_Compare, iAMem, iBMem, pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ,
                          (ವರ್ಣ*)sqlite3_ಕೀಲಿ_Info_Ref(p_ಕೀಲಿ_Info), P4_KEYINFO);
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
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Code_Move(pParse, iBMem, iAMem, pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ);
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
      ಯದಿ( groupBySort ){
        sqlite3VdbeAddOp2(v, OP_SorterNext, pAggInfo->sortingIdx,addr_ತುದಿ_Of_Loop);
        VdbeCoverage(v);
      }ಅನ್ಯಥಾ{
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3WhereEnd(pWInfo);
        sqlite3VdbeChangeToNoop(v, addrSortingIdx);
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pDistinct);

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
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pHaving, addrOutputRow+1, SQLITE_JUMPIFNULL);
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

      ಯದಿ( distFlag!=0 && eDist!=WHERE_DISTINCT_NOOP ){
        ಸಂರಚನಾ AggInfo_func *pF = &pAggInfo->aFunc[0];
        fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
      }
    } /* endif pGroupBy.  Begin aggregate queries without GROUP BY: */
    ಅನ್ಯಥಾ {
      /* Aggregate functions without GROUP BY. tag-select-0820 */
      Table *pTab;
      ಯದಿ( (pTab = is_Simple_ಎಣಿಕೆ(p, pAggInfo))!=0 ){
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
        ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
        ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ iCsr = pParse->nTab++;     /* Cursor to scan b-tree */
        Index *pIdx;                         /* Iterator variable */
        ಕೀಲಿ_Info *p_ಕೀಲಿ_Info = 0;               /* Keyinfo for scanned index */
        Index *pBest = 0;                    /* Best index found so far */
        Pgno i_ಬೇರು = pTab->tnum;             /* Root page of scanned b-tree */

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
        ಯದಿ( !HasRowid(pTab) ) pBest = sqlite3_Primary_ಕೀಲಿ_Index(pTab);
        ಯದಿ( !p->pSrc->a[0].fg.notIndexed ){
          ಪ್ರತಿ(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
            ಯದಿ( pIdx->bUnordered==0
             && pIdx->szIdxRow<pTab->szTabRow
             && pIdx->pPartIdxWhere==0
             && (!pBest || pIdx->szIdxRow<pBest->szIdxRow)
            ){
              pBest = pIdx;
            }
          }
        }
        ಯದಿ( pBest ){
          i_ಬೇರು = pBest->tnum;
          p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_Of_Index(pParse, pBest);
        }

        /* Open a read-only cursor, execute the OP_Count, close the cursor. */
        sqlite3VdbeAddOp4Int(v, OP_OpenRead, iCsr, (ಪೂರ್ಣಾಙ್ಕ)i_ಬೇರು, iDb, 1);
        ಯದಿ( p_ಕೀಲಿ_Info ){
          sqlite3VdbeChangeP4(v, -1, (ವರ್ಣ *)p_ಕೀಲಿ_Info, P4_KEYINFO);
        }
        assignAggregateRegisters(pParse, pAggInfo);
        sqlite3VdbeAddOp2(v, OP_ಎಣಿಕೆ, iCsr, AggInfoFuncReg(pAggInfo,0));
        sqlite3VdbeAddOp1(v, OP_Close, iCsr);
        explain_Simple_ಎಣಿಕೆ(pParse, pTab, pBest);
      }ಅನ್ಯಥಾ{
        /* The general case of an aggregate query without GROUP BY
        ** tag-select-0822 */
        ಪೂರ್ಣಾಙ್ಕ regAcc = 0;           /* "populate accumulators" flag */
        ಅಭಿವ್ಯಕ್ತಿ_List *pDistinct = 0;
        u16 distFlag = 0;
        ಪೂರ್ಣಾಙ್ಕ eDist;

        /* If there are accumulator registers but no min() or max() functions
        ** without FILTER clauses, allocate register regAcc. Register regAcc
        ** will contain 0 the first time the inner loop runs, and 1 thereafter.
        ** The code generated by updateAccumulator() uses this to ensure
        ** that the accumulator registers are (a) updated only once if
        ** there are no min() or max functions or (b) always updated for the
        ** first row visited by the aggregate, so that they are updated at
        ** least once even if the FILTER clause means the min() or max()
        ** function visits zero rows.  */
        ಯದಿ( pAggInfo->nAccumulator ){
          ಪ್ರತಿ(i=0; i<pAggInfo->nFunc; i++){
            ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pAggInfo->aFunc[i].pFExpr, EP_WinFunc) ){
              ಅನುವರ್ತ;
            }
            ಯದಿ( pAggInfo->aFunc[i].pFunc->funcFlags&SQLITE_FUNC_NEEDCOLL ){
              ಭಙ್ಗ;
            }
          }
          ಯದಿ( i==pAggInfo->nFunc ){
            regAcc = ++pParse->nMem;
            sqlite3VdbeAddOp2(v, OP_Integer, 0, regAcc);
          }
        }ಅನ್ಯಥಾ ಯದಿ( pAggInfo->nFunc==1 && pAggInfo->aFunc[0].iDistinct>=0 ){
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pAggInfo->aFunc[0].pFExpr) );
          pDistinct = pAggInfo->aFunc[0].pFExpr->x.pList;
          distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
        }
        assignAggregateRegisters(pParse, pAggInfo);

        /* This case runs if the aggregate has no GROUP BY clause.  The
        ** processing is much simpler since there is only a single row
        ** of output.
        */
        ಪ್ರತಿಜ್ಞಾ( p->pGroupBy==0 );
        resetAccumulator(pParse, pAggInfo);

        /* If this query is a candidate for the min/max optimization, then
        ** minMaxFlag will have been previously set to either
        ** WHERE_ORDERBY_MIN or WHERE_ORDERBY_MAX and pMinMaxOrderBy will
        ** be an appropriate ORDER BY expression for the optimization.
        */
        ಪ್ರತಿಜ್ಞಾ( minMaxFlag==WHERE_ORDERBY_NORMAL || pMinMaxOrderBy!=0 );
        ಪ್ರತಿಜ್ಞಾ( pMinMaxOrderBy==0 || pMinMaxOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ==1 );

        TREETRACE(0x2,pParse,p,("WhereBegin\n"));
        pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, pMinMaxOrderBy,
                                   pDistinct, p, minMaxFlag|distFlag, 0);
        ಯದಿ( pWInfo==0 ){
          ಗಚ್ಛ select_end;
        }
        TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
        eDist = sqlite3WhereIsDistinct(pWInfo);
        updateAccumulator(pParse, regAcc, pAggInfo, eDist);
        ಯದಿ( eDist!=WHERE_DISTINCT_NOOP ){
          ಸಂರಚನಾ AggInfo_func *pF = pAggInfo->aFunc;
          ಯದಿ( pF ){
            fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
          }
        }

        ಯದಿ( regAcc ) sqlite3VdbeAddOp2(v, OP_Integer, 1, regAcc);
        ಯದಿ( minMaxFlag ){
          sqlite3WhereMinMaxOptEarlyOut(v, pWInfo);
        }
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3WhereEnd(pWInfo);
        finalizeAggFunctions(pParse, pAggInfo);
      }

      sSort.pOrderBy = 0;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pHaving, addrEnd, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, 0, 0,
                      pDest, addrEnd, addrEnd);
    }
    sqlite3VdbeResolveLabel(v, addrEnd);

  } /* endif aggregate query */

  ಯದಿ( sDistinct.eTnctType==WHERE_DISTINCT_UNORDERED ){
    explainTempTable(pParse, "DISTINCT");
  }

  /* If there is an ORDER BY clause, then we need to sort the results
  ** and send them to the callback one by one.  tag-select-0900
  */
  ಯದಿ( sSort.pOrderBy ){
    ಪ್ರತಿಜ್ಞಾ( p->pEList==pEList );
    generateSortTail(pParse, p, &sSort, pEList->n_ಅಭಿವ್ಯಕ್ತಿ, pDest);
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
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 || db->mallocFailed==1 );
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 || pParse->nErr!=0 );
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pMinMaxOrderBy);
#ifdef SQLITE_DEBUG
  /* Internal self-checks.  tag-select-1000 */
  ಯದಿ( pAggInfo && !db->mallocFailed ){
#ಯದಿ TREETRACE_ENABLED
    ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("Finished with AggInfo\n"));
      printAggInfo(pAggInfo);
    }
#endif
    ಪ್ರತಿ(i=0; i<pAggInfo->nColumn; i++){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pAggInfo->aCol[i].pCExpr;
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಅನುವರ್ತ;
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo==pAggInfo );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->iAgg==i );
    }
    ಪ್ರತಿ(i=0; i<pAggInfo->nFunc; i++){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pAggInfo->aFunc[i].pFExpr;
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo==pAggInfo );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->iAgg==i );
    }
  }
#endif

#ಯದಿ TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p,("end processing\n"));
  ಯದಿ( (sqlite3_ವೃಕ್ಷ_Trace & 0x40000)!=0 && ExplainQueryPlanParent(pParse)==0 ){
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
  }
#endif
  Explain_Query_Plan_ತೆಗೆ(pParse);
  ಪ್ರತ್ಯಾಗಮ rc;
}
