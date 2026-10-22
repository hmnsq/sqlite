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
#અન્તર્ભાવ "sqliteInt.h"

/*
** An instance of the following object is used to record information about
** how to process the DISTINCT keyword, to simplify passing that information
** into the selectInnerLoop() routine.
*/
પ્રકારવ્યાખ્યા સંરચના DistinctCtx DistinctCtx;
સંરચના DistinctCtx {
  u8 isTnct;      /* 0: Not distinct. 1: DISTINCT  2: DISTINCT and ORDER BY */
  u8 eTnctType;   /* One of the WHERE_DISTINCT_* operators */
  પૂર્ણાંક tabTnct;    /* Ephemeral table used for DISTINCT processing */
  પૂર્ણાંક addrTnct;   /* Address of OP_OpenEphemeral opcode for tabTnct */
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
પ્રકારવ્યાખ્યા સંરચના SortCtx SortCtx;
સંરચના SortCtx {
  પદાવલિ_List *pOrderBy;   /* The ORDER BY (or GROUP BY clause) */
  પૂર્ણાંક nOBSat;           /* Number of ORDER BY terms satisfied by indices */
  પૂર્ણાંક iECursor;         /* Cursor number for the sorter */
  પૂર્ણાંક regReturn;        /* Register holding block-output return address */
  પૂર્ણાંક labelBkOut;       /* Start label for the block-output subroutine */
  પૂર્ણાંક addrSortIndex;    /* Address of the OP_SorterOpen or OP_OpenEphemeral */
  પૂર્ણાંક labelDone;        /* Jump here when done, ex: LIMIT reached */
  પૂર્ણાંક labelOBLopt;      /* Jump here when sorter is full */
  u8 sortFlags;         /* Zero or more SORTFLAG_* bits */
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  u8 nDefer;            /* Number of valid entries in aDefer[] */
  સંરચના DeferredCsr {
    Table *pTab;        /* Table definition */
    પૂર્ણાંક iCsr;           /* Cursor number for table */
    પૂર્ણાંક n_ચાવી;           /* Number of PK columns for table pTab (>=1) */
  } aDefer[4];
#endif
  સંરચના RowLoadInfo *pDeferredRowLoad;  /* Deferred row loading info or NULL */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  પૂર્ણાંક addr_ધકેલો;         /* First instruction to push data into sorter */
  પૂર્ણાંક addr_ધકેલો_End;      /* Last instruction that pushes data into sorter */
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
સ્થિર શૂન્ય clearSelect(sqlite3 *db, Select *p, પૂર્ણાંક bFree){
  પ્રતિજ્ઞા( db!=0 );
  જ્યારે( p ){
    Select *pPrior = p->pPrior;
    sqlite3_પદાવલિ_List_Delete(db, p->pEList);
    sqlite3SrcListDelete(db, p->pSrc);
    sqlite3_પદાવલિ_Delete(db, p->pWhere);
    sqlite3_પદાવલિ_List_Delete(db, p->pGroupBy);
    sqlite3_પદાવલિ_Delete(db, p->pHaving);
    sqlite3_પદાવલિ_List_Delete(db, p->pOrderBy);
    sqlite3_પદાવલિ_Delete(db, p->pLimit);
    જો( OK_IF_ALWAYS_TRUE(p->pWith) ) sqlite3WithDelete(db, p->pWith);
#ifndef SQLITE_OMIT_WINDOWFUNC
    જો( OK_IF_ALWAYS_TRUE(p->pWinDefn) ){
      sqlite3WindowListDelete(db, p->pWinDefn);
    }
    જ્યારે( p->pWin ){
      પ્રતિજ્ઞા( p->pWin->ppThis==&p->pWin );
      sqlite3WindowUnlinkFromSelect(p->pWin);
    }
#endif
    જો( bFree ) sqlite3DbNNFreeNN(db, p);
    p = pPrior;
    bFree = 1;
  }
}

/*
** Initialize a SelectDest structure.
*/
શૂન્ય sqlite3SelectDestInit(SelectDest *pDest, પૂર્ણાંક eDest, પૂર્ણાંક iParm){
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
  પદાવલિ_List *pEList,     /* which columns to include in the result */
  SrcList *pSrc,        /* the FROM clause -- which tables to scan */
  પદાવલિ *pWhere,         /* the WHERE clause */
  પદાવલિ_List *pGroupBy,   /* the GROUP BY clause */
  પદાવલિ *pHaving,        /* the HAVING clause */
  પદાવલિ_List *pOrderBy,   /* the ORDER BY clause */
  u32 selFlags,         /* Flag parameters, such as SF_Distinct */
  પદાવલિ *pLimit          /* LIMIT value.  NULL means not used */
){
  Select *pNew, *pAllocated;
  Select standin;
  pAllocated = pNew = sqlite3DbMallocRawNN(pParse->db, કદ(*pNew) );
  જો( pNew==0 ){
    પ્રતિજ્ઞા( pParse->db->mallocFailed );
    pNew = &standin;
  }
  જો( pEList==0 ){
    pEList = sqlite3_પદાવલિ_List_Append(pParse, 0,
                                   sqlite3_પદાવલિ(pParse->db,TK_ASTERISK,0));
  }
  pNew->pEList = pEList;
  pNew->op = TK_SELECT;
  pNew->selFlags = selFlags;
  pNew->iLimit = 0;
  pNew->iOffset = 0;
  pNew->selId = ++pParse->nSelect;
  pNew->nSelectRow = 0;
  જો( pSrc==0 ) pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
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
  જો( pParse->db->mallocFailed ) {
    clearSelect(pParse->db, pNew, pNew!=&standin);
    pAllocated = 0;
  }નહીંતર{
    પ્રતિજ્ઞા( pNew->pSrc!=0 || pParse->nErr>0 );
  }
  પાછા pAllocated;
}


/*
** Delete the given Select structure and all of its substructures.
*/
શૂન્ય sqlite3SelectDelete(sqlite3 *db, Select *p){
  જો( OK_IF_ALWAYS_TRUE(p) ) clearSelect(db, p, 1);
}
શૂન્ય sqlite3SelectDeleteGeneric(sqlite3 *db, શૂન્ય *p){
  જો( ALWAYS(p) ) clearSelect(db, (Select*)p, 1);
}

/*
** Return a pointer to the right-most SELECT statement in a compound.
*/
સ્થિર Select *findRightmost(Select *p){
  જ્યારે( p->pNext ) p = p->pNext;
  પાછા p;
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
પૂર્ણાંક sqlite3JoinType(Parse *pParse, Token *pA, Token *pB, Token *pC){
  પૂર્ણાંક jointype = 0;
  Token *apAll[3];
  Token *p;
                             /*   0123456789 123456789 123456789 123 */
  સ્થિર અચલ અક્ષર z_ચાવી_Text[] = "naturaleftouterightfullinnercross";
  સ્થિર અચલ સંરચના {
    u8 i;        /* Beginning of keyword text in zKeyText[] */
    u8 nChar;    /* Length of the keyword in characters */
    u8 code;     /* Join type mask */
  } aKeyword[] = {
    /* (0) natural */ { 0,  7, JT_NATURAL                },
    /* (1) left    */ { 6,  4, JT_ડાબું|JT_OUTER          },
    /* (2) outer   */ { 10, 5, JT_OUTER                  },
    /* (3) right   */ { 14, 5, JT_જમણું|JT_OUTER         },
    /* (4) full    */ { 19, 4, JT_ડાબું|JT_જમણું|JT_OUTER },
    /* (5) inner   */ { 23, 5, JT_INNER                  },
    /* (6) cross   */ { 28, 5, JT_INNER|JT_CROSS         },
  };
  પૂર્ણાંક i, j;
  apAll[0] = pA;
  apAll[1] = pB;
  apAll[2] = pC;
  માટે(i=0; i<3 && apAll[i]; i++){
    p = apAll[i];
    માટે(j=0; j<ArraySize(aKeyword); j++){
      જો( p->n==aKeyword[j].nChar
          && sqlite3StrNICmp((અક્ષર*)p->z, &z_ચાવી_Text[aKeyword[j].i], p->n)==0 ){
        jointype |= aKeyword[j].code;
        તોડો;
      }
    }
    testcase( j==0 || j==1 || j==2 || j==3 || j==4 || j==5 || j==6 );
    જો( j>=ArraySize(aKeyword) ){
      jointype |= JT_ERROR;
      તોડો;
    }
  }
  જો(
     (jointype & (JT_INNER|JT_OUTER))==(JT_INNER|JT_OUTER) ||
     (jointype & JT_ERROR)!=0 ||
     (jointype & (JT_OUTER|JT_ડાબું|JT_જમણું))==JT_OUTER
  ){
    અચલ અક્ષર *zSp1 = " ";
    અચલ અક્ષર *zSp2 = " ";
    જો( pB==0 ){ zSp1++; }
    જો( pC==0 ){ zSp2++; }
    sqlite3_Error_સંદેશ(pParse, "unknown join type: "
       "%T%s%T%s%T", pA, zSp1, pB, zSp2, pC);
    jointype = JT_INNER;
  }
  પાછા jointype;
}

/*
** Return the index of a column in a table.  Return -1 if the column
** is not contained in the table.
*/
પૂર્ણાંક sqlite3ColumnIndex(Table *pTab, અચલ અક્ષર *zCol){
  પૂર્ણાંક i;
  u8 h;
  અચલ Column *aCol;
  પૂર્ણાંક nCol;

  h = sqlite3StrIHash(zCol);
  aCol = pTab->aCol;
  nCol = pTab->nCol;

  /* See if the aHx gives us a lucky match */
  i = pTab->aHx[h % કદ(pTab->aHx)];
  પ્રતિજ્ઞા( i<nCol );
  જો( aCol[i].hName==h
   && sqlite3StrICmp(aCol[i].zCnName, zCol)==0
  ){
    પાછા i;
  }

  /* No lucky match from the hash table.  Do a full search. */
  i = 0;
  જ્યારે( 1 /*exit-by-break*/ ){
    જો( aCol[i].hName==h
     && sqlite3StrICmp(aCol[i].zCnName, zCol)==0
    ){
      પાછા i;
    }
    i++;
    જો( i>=nCol ) તોડો;
  }
  પાછા -1;
}

/*
** Mark a subquery result column as having been used.
*/
શૂન્ય sqlite3SrcItemColumnUsed(SrcItem *pItem, પૂર્ણાંક iCol){
  પ્રતિજ્ઞા( pItem!=0 );
  પ્રતિજ્ઞા( (પૂર્ણાંક)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
  જો( pItem->fg.isNestedFrom ){
    પદાવલિ_List *pResults;
    પ્રતિજ્ઞા( pItem->fg.isSubquery );
    પ્રતિજ્ઞા( pItem->u4.pSubq!=0 );
    પ્રતિજ્ઞા( pItem->u4.pSubq->pSelect!=0 );
    pResults = pItem->u4.pSubq->pSelect->pEList;
    પ્રતિજ્ઞા( pResults!=0 );
    પ્રતિજ્ઞા( iCol>=0 && iCol<pResults->n_પદાવલિ );
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
સ્થિર પૂર્ણાંક tableAndColumnIndex(
  SrcList *pSrc,       /* Array of tables to search */
  પૂર્ણાંક iStart,          /* First member of pSrc->a[] to check */
  પૂર્ણાંક iEnd,            /* Last member of pSrc->a[] to check */
  અચલ અક્ષર *zCol,    /* Name of the column we are looking for */
  પૂર્ણાંક *piTab,          /* Write index of pSrc->a[] here */
  પૂર્ણાંક *piCol,          /* Write index of pSrc->a[*piTab].pSTab->aCol[] here */
  પૂર્ણાંક bIgnoreHidden    /* Ignore hidden columns */
){
  પૂર્ણાંક i;               /* For looping over tables in pSrc */
  પૂર્ણાંક iCol;            /* Index of column matching zCol */

  પ્રતિજ્ઞા( iEnd<pSrc->nSrc );
  પ્રતિજ્ઞા( iStart>=0 );
  પ્રતિજ્ઞા( (piTab==0)==(piCol==0) );  /* Both or neither are NULL */

  માટે(i=iStart; i<=iEnd; i++){
    iCol = sqlite3ColumnIndex(pSrc->a[i].pSTab, zCol);
    જો( iCol>=0
     && (bIgnoreHidden==0 || IsHiddenColumn(&pSrc->a[i].pSTab->aCol[iCol])==0)
    ){
      જો( piTab ){
        sqlite3SrcItemColumnUsed(&pSrc->a[i], iCol);
        *piTab = i;
        *piCol = iCol;
      }
      પાછા 1;
    }
  }
  પાછા 0;
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
શૂન્ય sqlite3_Set_Join_પદાવલિ(પદાવલિ *p, પૂર્ણાંક iTable, u32 joinFlag){
  પ્રતિજ્ઞા( joinFlag==EP_OuterON || joinFlag==EP_InnerON );
  જ્યારે( p ){
    પદાવલિ_Set_Property(p, joinFlag);
    પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    પદાવલિ_Set_VVAProperty(p, EP_NoReduce);
    p->w.iJoin = iTable;
    જો( પદાવલિ_Use_XList(p) ){
      જો( p->x.pList ){
        પૂર્ણાંક i;
        માટે(i=0; i<p->x.pList->n_પદાવલિ; i++){
          sqlite3_Set_Join_પદાવલિ(p->x.pList->a[i].p_પદાવલિ, iTable, joinFlag);
        }
      }
    }
    sqlite3_Set_Join_પદાવલિ(p->p_ડાબું, iTable, joinFlag);
    p = p->p_જમણું;
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
સ્થિર શૂન્ય unset_Join_પદાવલિ(પદાવલિ *p, પૂર્ણાંક iTable, પૂર્ણાંક nullable){
  જ્યારે( p ){
    જો( iTable<0 || (પદાવલિ_Has_Property(p, EP_OuterON) && p->w.iJoin==iTable) ){
      પદાવલિ_Clear_Property(p, EP_OuterON|EP_InnerON);
      જો( iTable>=0 ) પદાવલિ_Set_Property(p, EP_InnerON);
    }
    જો( p->op==TK_COLUMN && p->iTable==iTable && !nullable ){
      પદાવલિ_Clear_Property(p, EP_CanBeNull);
    }
    જો( p->op==TK_FUNCTION ){
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p) );
      પ્રતિજ્ઞા( p->p_ડાબું==0 );
      જો( p->x.pList ){
        પૂર્ણાંક i;
        માટે(i=0; i<p->x.pList->n_પદાવલિ; i++){
          unset_Join_પદાવલિ(p->x.pList->a[i].p_પદાવલિ, iTable, nullable);
        }
      }
    }
    unset_Join_પદાવલિ(p->p_ડાબું, iTable, nullable);
    p = p->p_જમણું;
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
સ્થિર પૂર્ણાંક sqlite3ProcessJoin(Parse *pParse, Select *p){
  SrcList *pSrc;                  /* All tables in the FROM clause */
  પૂર્ણાંક i, j;                       /* Loop counters */
  SrcItem *p_ડાબું;                 /* Left table being joined */
  SrcItem *p_જમણું;                /* Right table being joined */
  પૂર્ણાંક bSeenTFunc = 0;             /* True if have seen table-valued function */

  pSrc = p->pSrc;
  p_ડાબું = &pSrc->a[0];
  p_જમણું = &p_ડાબું[1];
  માટે(i=0; i<pSrc->nSrc-1; i++, p_જમણું++, p_ડાબું++){
    Table *p_જમણું_Tab = p_જમણું->pSTab;
    u32 joinType;

    જો( NEVER(p_ડાબું->pSTab==0 || p_જમણું_Tab==0) ) ચાલુ;
    joinType = (p_જમણું->fg.jointype & JT_OUTER)!=0 ? EP_OuterON : EP_InnerON;

    /* If this is a NATURAL join, synthesize an appropriate USING clause
    ** to specify which columns should be joined.
    */
    જો( p_જમણું->fg.jointype & JT_NATURAL ){
      IdList *pUsing = 0;
      જો( p_જમણું->fg.isUsing || p_જમણું->u3.pOn ){
        sqlite3_Error_સંદેશ(pParse, "a NATURAL join may not have "
           "an ON or USING clause", 0);
        પાછા 1;
      }
      માટે(j=0; j<p_જમણું_Tab->nCol; j++){
        અક્ષર *zName;   /* Name of column in the right table */

        જો( IsHiddenColumn(&p_જમણું_Tab->aCol[j]) ) ચાલુ;
        zName = p_જમણું_Tab->aCol[j].zCnName;
        જો( tableAndColumnIndex(pSrc, 0, i, zName, 0, 0, 1) ){
          pUsing = sqlite3IdListAppend(pParse, pUsing, 0);
          જો( pUsing ){
            પ્રતિજ્ઞા( pUsing->nId>0 );
            પ્રતિજ્ઞા( pUsing->a[pUsing->nId-1].zName==0 );
            pUsing->a[pUsing->nId-1].zName = sqlite3DbStrDup(pParse->db, zName);
          }
        }
      }
      જો( pUsing ){
        p_જમણું->fg.isUsing = 1;
        p_જમણું->fg.isSynthUsing = 1;
        p_જમણું->u3.pUsing = pUsing;
      }
      જો( pParse->nErr ) પાછા 1;
    }

    /* Create extra terms on the WHERE clause for each column named
    ** in the USING clause.  Example: If the two tables to be joined are
    ** A and B and the USING clause names X, Y, and Z, then add this
    ** to the WHERE clause:    A.X=B.X AND A.Y=B.Y AND A.Z=B.Z
    ** Report an error if any column mentioned in the USING clause is
    ** not contained in both tables to be joined.
    */
    જો( p_જમણું->fg.isUsing ){
      IdList *pList = p_જમણું->u3.pUsing;
      sqlite3 *db = pParse->db;
      પ્રતિજ્ઞા( pList!=0 );
      માટે(j=0; j<pList->nId; j++){
        અક્ષર *zName;     /* Name of the term in the USING clause */
        પૂર્ણાંક i_ડાબું;       /* Table on the left with matching column name */
        પૂર્ણાંક i_ડાબું_Col;    /* Column number of matching column on the left */
        પૂર્ણાંક i_જમણું_Col;   /* Column number of matching column on the right */
        પદાવલિ *pE1;       /* Reference to the column on the LEFT of the join */
        પદાવલિ *pE2;       /* Reference to the column on the RIGHT of the join */
        પદાવલિ *pEq;       /* Equality constraint.  pE1 == pE2 */

        zName = pList->a[j].zName;
        i_જમણું_Col = sqlite3ColumnIndex(p_જમણું_Tab, zName);
        જો( i_જમણું_Col<0
         || tableAndColumnIndex(pSrc, 0, i, zName, &i_ડાબું, &i_ડાબું_Col,
                                p_જમણું->fg.isSynthUsing)==0
        ){
          sqlite3_Error_સંદેશ(pParse, "cannot join using column %s - column "
            "not present in both tables", zName);
          પાછા 1;
        }
        pE1 = sqlite3_Create_Column_પદાવલિ(db, pSrc, i_ડાબું, i_ડાબું_Col);
        sqlite3SrcItemColumnUsed(&pSrc->a[i_ડાબું], i_ડાબું_Col);
        જો( (pSrc->a[0].fg.jointype & JT_LTORJ)!=0 && pParse->nErr==0 ){
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
          પદાવલિ_List *pFuncArgs = 0;   /* Arguments to the coalesce() */
          સ્થિર અચલ Token tkCoalesce = { "coalesce", 8 };
          પ્રતિજ્ઞા( pE1!=0 );
          પદાવલિ_Set_Property(pE1, EP_CanBeNull);
          જ્યારે( tableAndColumnIndex(pSrc, i_ડાબું+1, i, zName, &i_ડાબું, &i_ડાબું_Col,
                                     p_જમણું->fg.isSynthUsing)!=0 ){
            જો( pSrc->a[i_ડાબું].fg.isUsing==0
             || sqlite3IdListIndex(pSrc->a[i_ડાબું].u3.pUsing, zName)<0
            ){
              sqlite3_Error_સંદેશ(pParse, "ambiguous reference to %s in USING()",
                              zName);
              તોડો;
            }
            pFuncArgs = sqlite3_પદાવલિ_List_Append(pParse, pFuncArgs, pE1);
            pE1 = sqlite3_Create_Column_પદાવલિ(db, pSrc, i_ડાબું, i_ડાબું_Col);
            sqlite3SrcItemColumnUsed(&pSrc->a[i_ડાબું], i_ડાબું_Col);
          }
          જો( pFuncArgs ){
            pFuncArgs = sqlite3_પદાવલિ_List_Append(pParse, pFuncArgs, pE1);
            pE1 = sqlite3_પદાવલિ_Function(pParse, pFuncArgs, &tkCoalesce, 0);
            જો( pE1 ){
              pE1->aff_પદાવલિ = SQLITE_AFF_DEFER;
            }
          }
        }નહીંતર જો( (pSrc->a[i+1].fg.jointype & JT_ડાબું)!=0 && pParse->nErr==0 ){
          પ્રતિજ્ઞા( pE1!=0 );
          પદાવલિ_Set_Property(pE1, EP_CanBeNull);
        }
        pE2 = sqlite3_Create_Column_પદાવલિ(db, pSrc, i+1, i_જમણું_Col);
        sqlite3SrcItemColumnUsed(p_જમણું, i_જમણું_Col);
        pEq = sqlite3PExpr(pParse, TK_EQ, pE1, pE2);
        પ્રતિજ્ઞા( pE2!=0 || pEq==0 );
        જો( pEq ){
          પદાવલિ_Set_Property(pEq, joinType);
          પ્રતિજ્ઞા( !પદાવલિ_Has_Property(pEq, EP_TokenOnly|EP_Reduced) );
          પદાવલિ_Set_VVAProperty(pEq, EP_NoReduce);
          pEq->w.iJoin = pE2->iTable;
        }
        p->pWhere = sqlite3_પદાવલિ_And(pParse, p->pWhere, pEq);
      }
    }

    /* Add the ON clause to the end of the WHERE clause, connected by
    ** an AND operator.
    */
    નહીંતર જો( p_જમણું->u3.pOn ){
      sqlite3_Set_Join_પદાવલિ(p_જમણું->u3.pOn, p_જમણું->iCursor, joinType);
      p->pWhere = sqlite3_પદાવલિ_And(pParse, p->pWhere, p_જમણું->u3.pOn);
      p_જમણું->u3.pOn = 0;
      p_જમણું->fg.isOn = 1;
      p->selFlags |= SF_OnToWhere;
    }

    જો( bSeenTFunc==0 ){
      bSeenTFunc = (p_ડાબું->fg.isTabFunc && p_ડાબું->u1.pFuncArg);
    }

    /* We also need to call sqlite3SelectCheckOnClauses() to verify that
    ** table-valued function arguments do not illegally refer to any tables to
    ** their right. This test is required if either (a) there is a RIGHT JOIN
    ** in the SrcList, or (b) there is a LEFT JOIN to the right of a 
    ** table-valued function.  */
    જો( (p_જમણું->fg.isTabFunc && joinType==EP_OuterON && p_જમણું->u1.pFuncArg)
     || (bSeenTFunc && joinType==EP_OuterON)
    ){
      p->selFlags |= SF_OnToWhere;
    }
  }
  પાછા 0;
}

/*
** An instance of this object holds information (beyond pParse and pSelect)
** needed to load the next result row that is to be added to the sorter.
*/
પ્રકારવ્યાખ્યા સંરચના RowLoadInfo RowLoadInfo;
સંરચના RowLoadInfo {
  પૂર્ણાંક regResult;               /* Store results in array of registers here */
  u8 ecelFlags;                /* Flag argument to ExprCodeExprList() */
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  પદાવલિ_List *pExtra;            /* Extra columns needed by sorter refs */
  પૂર્ણાંક regExtraResult;          /* Where to load the extra columns */
#endif
};

/*
** This routine does the work of loading query data into an array of
** registers so that it can be added to the sorter.
*/
સ્થિર શૂન્ય innerLoopLoadRow(
  Parse *pParse,             /* Statement under construction */
  Select *pSelect,           /* The query being coded */
  RowLoadInfo *pInfo         /* Info needed to complete the row load */
){
  sqlite3_પદાવલિ_Code_પદાવલિ_List(pParse, pSelect->pEList, pInfo->regResult,
                          0, pInfo->ecelFlags);
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  જો( pInfo->pExtra ){
    sqlite3_પદાવલિ_Code_પદાવલિ_List(pParse, pInfo->pExtra, pInfo->regExtraResult, 0, 0);
    sqlite3_પદાવલિ_List_Delete(pParse->db, pInfo->pExtra);
  }
#endif
}

/*
** Code the OP_MakeRecord instruction that generates the entry to be
** added into the sorter.
**
** Return the register in which the result is stored.
*/
સ્થિર પૂર્ણાંક makeSorterRecord(
  Parse *pParse,
  SortCtx *pSort,
  Select *pSelect,
  પૂર્ણાંક regBase,
  પૂર્ણાંક nBase
){
  પૂર્ણાંક nOBSat = pSort->nOBSat;
  Vdbe *v = pParse->pVdbe;
  પૂર્ણાંક regOut = ++pParse->nMem;
  જો( pSort->pDeferredRowLoad ){
    innerLoopLoadRow(pParse, pSelect, pSort->pDeferredRowLoad);
  }
  sqlite3VdbeAddOp3(v, OP_MakeRecord, regBase+nOBSat, nBase-nOBSat, regOut);
  પાછા regOut;
}

/*
** Generate code that will push the record in registers regData
** through regData+nData-1 onto the sorter.
*/
સ્થિર શૂન્ય ધકેલો_Onto_Sorter(
  Parse *pParse,         /* Parser context */
  SortCtx *pSort,        /* Information about the ORDER BY clause */
  Select *pSelect,       /* The whole SELECT statement */
  પૂર્ણાંક regData,           /* First register holding data to be sorted */
  પૂર્ણાંક regOrigData,       /* First register holding data before packing */
  પૂર્ણાંક nData,             /* Number of elements in the regData data array */
  પૂર્ણાંક nPrefixReg         /* No. of reg prior to regData available for use */
){
  Vdbe *v = pParse->pVdbe;                         /* Stmt under construction */
  પૂર્ણાંક bSeq = ((pSort->sortFlags & SORTFLAG_UseSorter)==0);
  પૂર્ણાંક n_પદાવલિ = pSort->pOrderBy->n_પદાવલિ;              /* No. of ORDER BY terms */
  પૂર્ણાંક nBase = n_પદાવલિ + bSeq + nData;                /* Fields in sorter record */
  પૂર્ણાંક regBase;                                     /* Regs for sorter record */
  પૂર્ણાંક regRecord = 0;                               /* Assembled sorter record */
  પૂર્ણાંક nOBSat = pSort->nOBSat;                      /* ORDER BY terms to skip */
  પૂર્ણાંક op;                            /* Opcode to add sorter record to sorter */
  પૂર્ણાંક iLimit;                        /* LIMIT counter */
  પૂર્ણાંક iSkip = 0;                     /* End of the sorter insert loop */

  પ્રતિજ્ઞા( bSeq==0 || bSeq==1 );

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
  પ્રતિજ્ઞા( nData==1 || regData==regOrigData || regOrigData==0 );

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  pSort->addr_ધકેલો = sqlite3VdbeCurrentAddr(v);
#endif

  જો( nPrefixReg ){
    પ્રતિજ્ઞા( nPrefixReg==n_પદાવલિ+bSeq );
    regBase = regData - nPrefixReg;
  }નહીંતર{
    regBase = pParse->nMem + 1;
    pParse->nMem += nBase;
  }
  પ્રતિજ્ઞા( pSelect->iOffset==0 || pSelect->iLimit!=0 );
  iLimit = pSelect->iOffset ? pSelect->iOffset+1 : pSelect->iLimit;
  pSort->labelDone = sqlite3VdbeMakeLabel(pParse);
  sqlite3_પદાવલિ_Code_પદાવલિ_List(pParse, pSort->pOrderBy, regBase, regOrigData,
                          SQLITE_ECEL_DUP | (regOrigData? SQLITE_ECEL_REF : 0));
  જો( bSeq ){
    sqlite3VdbeAddOp2(v, OP_Sequence, pSort->iECursor, regBase+n_પદાવલિ);
  }
  જો( nPrefixReg==0 && nData>0 ){
    sqlite3_પદાવલિ_Code_Move(pParse, regData, regBase+n_પદાવલિ+bSeq, nData);
  }
  જો( nOBSat>0 ){
    પૂર્ણાંક reg_Prev_ચાવી;   /* The first nOBSat columns of the previous row */
    પૂર્ણાંક addrFirst;    /* Address of the OP_IfNot opcode */
    પૂર્ણાંક addrJmp;      /* Address of the OP_Jump opcode */
    VdbeOp *pOp;      /* Opcode that opens the sorter */
    પૂર્ણાંક n_ચાવી;         /* Number of sorting key columns, including OP_Sequence */
    ચાવી_Info *pKI;     /* Original KeyInfo on the sorter table */

    regRecord = makeSorterRecord(pParse, pSort, pSelect, regBase, nBase);
    reg_Prev_ચાવી = pParse->nMem+1;
    pParse->nMem += pSort->nOBSat;
    n_ચાવી = n_પદાવલિ - pSort->nOBSat + bSeq;
    જો( bSeq ){
      addrFirst = sqlite3VdbeAddOp1(v, OP_IfNot, regBase+n_પદાવલિ);
    }નહીંતર{
      addrFirst = sqlite3VdbeAddOp1(v, OP_SequenceTest, pSort->iECursor);
    }
    VdbeCoverage(v);
    sqlite3VdbeAddOp3(v, OP_Compare, reg_Prev_ચાવી, regBase, pSort->nOBSat);
    pOp = sqlite3VdbeGetOp(v, pSort->addrSortIndex);
    જો( pParse->db->mallocFailed ) પાછા;
    pOp->p2 = n_ચાવી + nData;
    pKI = pOp->p4.p_ચાવી_Info;
    memset(pKI->aSortFlags, 0, pKI->n_ચાવી_Field); /* Makes OP_Jump testable */
    sqlite3VdbeChangeP4(v, -1, (અક્ષર*)pKI, P4_KEYINFO);
    testcase( pKI->nAllField > pKI->n_ચાવી_Field+2 );
    pOp->p4.p_ચાવી_Info = sqlite3_ચાવી_Info_From_પદાવલિ_List(pParse,pSort->pOrderBy,nOBSat,
                                           pKI->nAllField-pKI->n_ચાવી_Field-1);
    pOp = 0; /* Ensure pOp not used after sqlite3VdbeAddOp3() */
    addrJmp = sqlite3VdbeCurrentAddr(v);
    sqlite3VdbeAddOp3(v, OP_Jump, addrJmp+1, 0, addrJmp+1); VdbeCoverage(v);
    pSort->labelBkOut = sqlite3VdbeMakeLabel(pParse);
    pSort->regReturn = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Gosub, pSort->regReturn, pSort->labelBkOut);
    sqlite3VdbeAddOp1(v, OP_ResetSorter, pSort->iECursor);
    જો( iLimit ){
      sqlite3VdbeAddOp2(v, OP_IfNot, iLimit, pSort->labelDone);
      VdbeCoverage(v);
    }
    sqlite3VdbeJumpHere(v, addrFirst);
    sqlite3_પદાવલિ_Code_Move(pParse, regBase, reg_Prev_ચાવી, pSort->nOBSat);
    sqlite3VdbeJumpHere(v, addrJmp);
  }
  જો( iLimit ){
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
    પૂર્ણાંક iCsr = pSort->iECursor;
    sqlite3VdbeAddOp2(v, OP_IfNotZero, iLimit, sqlite3VdbeCurrentAddr(v)+4);
    VdbeCoverage(v);
    sqlite3VdbeAddOp2(v, OP_Last, iCsr, 0);
    iSkip = sqlite3VdbeAddOp4Int(v, OP_IdxLE,
                                 iCsr, 0, regBase+nOBSat, n_પદાવલિ-nOBSat);
    VdbeCoverage(v);
    sqlite3VdbeAddOp1(v, OP_Delete, iCsr);
  }
  જો( regRecord==0 ){
    regRecord = makeSorterRecord(pParse, pSort, pSelect, regBase, nBase);
  }
  જો( pSort->sortFlags & SORTFLAG_UseSorter ){
    op = OP_Sorter_ઉમેરો;
  }નહીંતર{
    op = OP_Idx_ઉમેરો;
  }
  sqlite3VdbeAddOp4Int(v, op, pSort->iECursor, regRecord,
                       regBase+nOBSat, nBase-nOBSat);
  જો( iSkip ){
    sqlite3VdbeChangeP2(v, iSkip,
         pSort->labelOBLopt ? pSort->labelOBLopt : sqlite3VdbeCurrentAddr(v));
  }
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  pSort->addr_ધકેલો_End = sqlite3VdbeCurrentAddr(v)-1;
#endif
}

/*
** Add code to implement the OFFSET
*/
સ્થિર શૂન્ય codeOffset(
  Vdbe *v,          /* Generate code into this VM */
  પૂર્ણાંક iOffset,      /* Register holding the offset counter */
  પૂર્ણાંક iContinue     /* Jump here to skip the current record */
){
  જો( iOffset>0 ){
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
સ્થિર પૂર્ણાંક codeDistinct(
  Parse *pParse,     /* Parsing and code generating context */
  પૂર્ણાંક eTnctType,     /* WHERE_DISTINCT_* value */
  પૂર્ણાંક iTab,          /* A sorting index used to test for distinctness */
  પૂર્ણાંક addrRepeat,    /* Jump to here if not distinct */
  પદાવલિ_List *pEList,  /* Expression for each element */
  પૂર્ણાંક regElem        /* First element */
){
  પૂર્ણાંક iRet = 0;
  પૂર્ણાંક nResultCol = pEList->n_પદાવલિ;
  Vdbe *v = pParse->pVdbe;

  પસંદગી( eTnctType ){
    સ્થિતિ WHERE_DISTINCT_ORDERED: {
      પૂર્ણાંક i;
      પૂર્ણાંક iJump;              /* Jump destination */
      પૂર્ણાંક regPrev;            /* Previous row content */

      /* Allocate space for the previous row */
      iRet = regPrev = pParse->nMem+1;
      pParse->nMem += nResultCol;

      iJump = sqlite3VdbeCurrentAddr(v) + nResultCol;
      માટે(i=0; i<nResultCol; i++){
        CollSeq *pColl = sqlite3_પદાવલિ_Coll_Seq(pParse, pEList->a[i].p_પદાવલિ);
        જો( i<nResultCol-1 ){
          sqlite3VdbeAddOp3(v, OP_Ne, regElem+i, iJump, regPrev+i);
          VdbeCoverage(v);
        }નહીંતર{
          sqlite3VdbeAddOp3(v, OP_Eq, regElem+i, addrRepeat, regPrev+i);
          VdbeCoverage(v);
         }
        sqlite3VdbeChangeP4(v, -1, (અચલ અક્ષર *)pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
      }
      પ્રતિજ્ઞા( sqlite3VdbeCurrentAddr(v)==iJump || pParse->db->mallocFailed );
      sqlite3VdbeAddOp3(v, OP_Copy, regElem, regPrev, nResultCol-1);
      તોડો;
    }

    સ્થિતિ WHERE_DISTINCT_UNIQUE: {
      /* nothing to do */
      તોડો;
    }

    મૂળભૂત: {
      પૂર્ણાંક r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp4Int(v, OP_Found, iTab, addrRepeat, regElem, nResultCol);
      VdbeCoverage(v);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regElem, nResultCol, r1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, iTab, r1, regElem, nResultCol);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      sqlite3ReleaseTempReg(pParse, r1);
      iRet = iTab;
      તોડો;
    }
  }

  પાછા iRet;
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
સ્થિર શૂન્ય fixDistinctOpenEph(
  Parse *pParse,     /* Parsing and code generating context */
  પૂર્ણાંક eTnctType,     /* WHERE_DISTINCT_* value */
  પૂર્ણાંક iVal,          /* Value returned by codeDistinct() */
  પૂર્ણાંક iOpenEphAddr   /* Address of OP_OpenEphemeral instruction for iTab */
){
  જો( pParse->nErr==0
   && (eTnctType==WHERE_DISTINCT_UNIQUE || eTnctType==WHERE_DISTINCT_ORDERED)
  ){
    Vdbe *v = pParse->pVdbe;
    sqlite3VdbeChangeToNoop(v, iOpenEphAddr);
    જો( sqlite3VdbeGetOp(v, iOpenEphAddr+1)->opcode==OP_Explain ){
      sqlite3VdbeChangeToNoop(v, iOpenEphAddr+1);
    }
    જો( eTnctType==WHERE_DISTINCT_ORDERED ){
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
સ્થિર શૂન્ય select_પદાવલિ_Defer(
  Parse *pParse,                  /* Leave any error here */
  SortCtx *pSort,                 /* Sorter context */
  પદાવલિ_List *pEList,               /* Expressions destined for sorter */
  પદાવલિ_List **ppExtra              /* Expressions to append to sorter record */
){
  પૂર્ણાંક i;
  પૂર્ણાંક nDefer = 0;
  પદાવલિ_List *pExtra = 0;
  માટે(i=0; i<pEList->n_પદાવલિ; i++){
    સંરચના પદાવલિ_List_item *pItem = &pEList->a[i];
    જો( pItem->u.x.iOrderByCol==0 ){
      પદાવલિ *p_પદાવલિ = pItem->p_પદાવલિ;
      Table *pTab;
      જો( p_પદાવલિ->op==TK_COLUMN
       && p_પદાવલિ->iColumn>=0
       && ALWAYS( પદાવલિ_Use_YTab(p_પદાવલિ) )
       && (pTab = p_પદાવલિ->y.pTab)!=0
       && IsOrdinaryTable(pTab)
       && (pTab->aCol[p_પદાવલિ->iColumn].colFlags & COLFLAG_SORTERREF)!=0
      ){
        પૂર્ણાંક j;
        માટે(j=0; j<nDefer; j++){
          જો( pSort->aDefer[j].iCsr==p_પદાવલિ->iTable ) તોડો;
        }
        જો( j==nDefer ){
          જો( nDefer==ArraySize(pSort->aDefer) ){
            ચાલુ;
          }નહીંતર{
            પૂર્ણાંક n_ચાવી = 1;
            પૂર્ણાંક k;
            Index *pPk = 0;
            જો( !HasRowid(pTab) ){
              pPk = sqlite3_Primary_ચાવી_Index(pTab);
              n_ચાવી = pPk->n_ચાવી_Col;
            }
            માટે(k=0; k<n_ચાવી; k++){
              પદાવલિ *pNew = sqlite3PExpr(pParse, TK_COLUMN, 0, 0);
              જો( pNew ){
                pNew->iTable = p_પદાવલિ->iTable;
                પ્રતિજ્ઞા( પદાવલિ_Use_YTab(pNew) );
                pNew->y.pTab = p_પદાવલિ->y.pTab;
                pNew->iColumn = pPk ? pPk->aiColumn[k] : -1;
                pExtra = sqlite3_પદાવલિ_List_Append(pParse, pExtra, pNew);
              }
            }
            pSort->aDefer[nDefer].pTab = p_પદાવલિ->y.pTab;
            pSort->aDefer[nDefer].iCsr = p_પદાવલિ->iTable;
            pSort->aDefer[nDefer].n_ચાવી = n_ચાવી;
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
સ્થિર પૂર્ણાંક sqlite3CursorBloomable(Parse *pParse, પૂર્ણાંક iCur, પૂર્ણાંક nCol){
  પૂર્ણાંક i,k;
  Vdbe *v = pParse->pVdbe;
  જો( pParse->nErr ) પાછા 1;
  પ્રતિજ્ઞા( v );
  માટે(k=sqlite3VdbeCurrentAddr(v)-1; k>0; k--){
    અચલ VdbeOp *pOp = sqlite3VdbeGetOp(v, k);
    અચલ ચાવી_Info *p_ચાવી_Info;
    જો( pOp->p1!=iCur ) ચાલુ;
    જો( pOp->opcode!=OP_OpenRead
     && pOp->opcode!=OP_OpenWrite
     && pOp->opcode!=OP_OpenEphemeral
    ){
      ચાલુ;
    }
    પ્રતિજ્ઞા( pOp->p4type==P4_KEYINFO );
    p_ચાવી_Info = (અચલ ચાવી_Info*)pOp->p4.p_ચાવી_Info;
    પ્રતિજ્ઞા( p_ચાવી_Info!=0 );
    માટે(i=0; i<nCol; i++){
      પ્રતિજ્ઞા( sqlite3_Is_દ્વિઅંકી(p_ચાવી_Info->aColl[i]) );
    }
    પાછા 1;
  }
  પાછા 0;
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
સ્થિર શૂન્ય selectInnerLoop(
  Parse *pParse,          /* The parser context */
  Select *p,              /* The complete select statement being coded */
  પૂર્ણાંક srcTab,             /* Pull data from this table if non-negative */
  SortCtx *pSort,         /* If not NULL, info on how to process ORDER BY */
  DistinctCtx *pDistinct, /* If not NULL, info on how to process DISTINCT */
  SelectDest *pDest,      /* How to dispose of the results */
  પૂર્ણાંક iContinue,          /* Jump here to continue with next row */
  પૂર્ણાંક iBreak              /* Jump here to break out of the inner loop */
){
  Vdbe *v = pParse->pVdbe;
  પૂર્ણાંક i;
  પૂર્ણાંક hasDistinct;            /* True if the DISTINCT keyword is present */
  પૂર્ણાંક eDest = pDest->eDest;   /* How to dispose of results */
  પૂર્ણાંક iParm = pDest->iSDParm; /* First argument to disposal method */
  પૂર્ણાંક nResultCol;             /* Number of result columns */
  પૂર્ણાંક nPrefixReg = 0;         /* Number of extra registers before regResult */
  RowLoadInfo sRowLoadInfo;   /* Info for deferred row loading */

  /* Usually, regResult is the first cell in an array of memory cells
  ** containing the current result row. In this case regOrig is set to the
  ** same value. However, if the results are being sent to the sorter, the
  ** values for any expressions that are also part of the sort-key are omitted
  ** from this array. In this case regOrig is set to zero.  */
  પૂર્ણાંક regResult;              /* Start of memory holding current results */
  પૂર્ણાંક regOrig;                /* Start of memory holding full result (or 0) */

  પ્રતિજ્ઞા( v );
  પ્રતિજ્ઞા( p->pEList!=0 );
  hasDistinct = pDistinct ? pDistinct->eTnctType : WHERE_DISTINCT_NOOP;
  જો( pSort && pSort->pOrderBy==0 ) pSort = 0;
  જો( pSort==0 && !hasDistinct ){
    પ્રતિજ્ઞા( iContinue!=0 );
    codeOffset(v, p->iOffset, iContinue);
  }

  /* Pull the requested columns.
  */
  nResultCol = p->pEList->n_પદાવલિ;

  જો( pDest->iSdst==0 ){
    જો( pSort ){
      nPrefixReg = pSort->pOrderBy->n_પદાવલિ;
      જો( !(pSort->sortFlags & SORTFLAG_UseSorter) ) nPrefixReg++;
      pParse->nMem += nPrefixReg;
    }
    pDest->iSdst = pParse->nMem+1;
    pParse->nMem += nResultCol;
  }નહીંતર જો( pDest->iSdst+nResultCol > pParse->nMem ){
    /* This is an error condition that can result, for example, when a SELECT
    ** on the right-hand side of an INSERT contains more result columns than
    ** there are columns in the table on the left.  The error will be caught
    ** and reported later.  But we need to make sure enough memory is allocated
    ** to avoid other spurious errors in the meantime. */
    pParse->nMem += nResultCol;
  }
  pDest->nSdst = nResultCol;
  regOrig = regResult = pDest->iSdst;
  જો( srcTab>=0 ){
    માટે(i=0; i<nResultCol; i++){
      sqlite3VdbeAddOp3(v, OP_Column, srcTab, i, regResult+i);
      VdbeComment((v, "%s", p->pEList->a[i].zEName));
    }
  }નહીંતર જો( eDest!=SRT_Exists ){
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    પદાવલિ_List *pExtra = 0;
#endif
    /* If the destination is an EXISTS(...) expression, the actual
    ** values returned by the SELECT are not required.
    */
    u8 ecelFlags;    /* "ecel" is an abbreviation of "ExprCodeExprList" */
    પદાવલિ_List *pEList;
    જો( eDest==SRT_Mem || eDest==SRT_Output || eDest==SRT_Coroutine ){
      ecelFlags = SQLITE_ECEL_DUP;
    }નહીંતર{
      ecelFlags = 0;
    }
    જો( pSort && hasDistinct==0 && eDest!=SRT_EphemTab && eDest!=SRT_Table ){
      /* For each expression in p->pEList that is a copy of an expression in
      ** the ORDER BY clause (pSort->pOrderBy), set the associated
      ** iOrderByCol value to one more than the index of the ORDER BY
      ** expression within the sort-key that pushOntoSorter() will generate.
      ** This allows the p->pEList field to be omitted from the sorted record,
      ** saving space and CPU cycles.  */
      ecelFlags |= (SQLITE_ECEL_OMITREF|SQLITE_ECEL_REF);

      માટે(i=pSort->nOBSat; i<pSort->pOrderBy->n_પદાવલિ; i++){
        પૂર્ણાંક j;
        જો( (j = pSort->pOrderBy->a[i].u.x.iOrderByCol)>0 ){
          p->pEList->a[j-1].u.x.iOrderByCol = i+1-pSort->nOBSat;
        }
      }
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
      select_પદાવલિ_Defer(pParse, pSort, p->pEList, &pExtra);
      જો( pExtra && pParse->db->mallocFailed==0 ){
        /* If there are any extra PK columns to add to the sorter records,
        ** allocate extra memory cells and adjust the OpenEphemeral
        ** instruction to account for the larger records. This is only
        ** required if there are one or more WITHOUT ROWID tables with
        ** composite primary keys in the SortCtx.aDefer[] array.  */
        VdbeOp *pOp = sqlite3VdbeGetOp(v, pSort->addrSortIndex);
        pOp->p2 += (pExtra->n_પદાવલિ - pSort->nDefer);
        pOp->p4.p_ચાવી_Info->nAllField += (pExtra->n_પદાવલિ - pSort->nDefer);
        pParse->nMem += pExtra->n_પદાવલિ;
      }
#endif

      /* Adjust nResultCol to account for columns that are omitted
      ** from the sorter by the optimizations in this branch */
      pEList = p->pEList;
      માટે(i=0; i<pEList->n_પદાવલિ; i++){
        જો( pEList->a[i].u.x.iOrderByCol>0
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
      પ્રતિજ્ઞા( eDest==SRT_Set || eDest==SRT_Mem
           || eDest==SRT_Coroutine || eDest==SRT_Output
           || eDest==SRT_Upfrom );
    }
    sRowLoadInfo.regResult = regResult;
    sRowLoadInfo.ecelFlags = ecelFlags;
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    sRowLoadInfo.pExtra = pExtra;
    sRowLoadInfo.regExtraResult = regResult + nResultCol;
    જો( pExtra ) nResultCol += pExtra->n_પદાવલિ;
#endif
    જો( p->iLimit
     && (ecelFlags & SQLITE_ECEL_OMITREF)!=0
     && nPrefixReg>0
    ){
      પ્રતિજ્ઞા( pSort!=0 );
      પ્રતિજ્ઞા( hasDistinct==0 );
      pSort->pDeferredRowLoad = &sRowLoadInfo;
      regOrig = 0;
    }નહીંતર{
      innerLoopLoadRow(pParse, p, &sRowLoadInfo);
    }
  }

  /* If the DISTINCT keyword was present on the SELECT statement
  ** and this row has been seen before, then do not make this row
  ** part of the result.
  */
  જો( hasDistinct ){
    પૂર્ણાંક eType = pDistinct->eTnctType;
    પૂર્ણાંક iTab = pDistinct->tabTnct;
    પ્રતિજ્ઞા( nResultCol==p->pEList->n_પદાવલિ );
    iTab = codeDistinct(pParse, eType, iTab, iContinue, p->pEList, regResult);
    fixDistinctOpenEph(pParse, eType, iTab, pDistinct->addrTnct);
    જો( pSort==0 ){
      codeOffset(v, p->iOffset, iContinue);
    }
  }

  પસંદગી( eDest ){
    /* Store the result as data using a unique key.
    */
    સ્થિતિ SRT_Fifo:
    સ્થિતિ SRT_DistFifo:
    સ્થિતિ SRT_Table:
    સ્થિતિ SRT_EphemTab: {
      પૂર્ણાંક r1 = sqlite3GetTempRange(pParse, nPrefixReg+1);
      testcase( eDest==SRT_Table );
      testcase( eDest==SRT_EphemTab );
      testcase( eDest==SRT_Fifo );
      testcase( eDest==SRT_DistFifo );
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regResult, nResultCol, r1+nPrefixReg);
#જો !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      જો( eDest==SRT_Table && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#endif
#ifndef SQLITE_OMIT_CTE
      જો( eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index. If the current row is already present
        ** in the index, do not write it to the output. If not, add the
        ** current row to the index and proceed with writing it to the
        ** output table as well.  */
        પૂર્ણાંક addr = sqlite3VdbeCurrentAddr(v) + 4;
        sqlite3VdbeAddOp4Int(v, OP_Found, iParm+1, addr, r1, 0);
        VdbeCoverage(v);
        sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, iParm+1, r1,regResult,nResultCol);
        પ્રતિજ્ઞા( pSort==0 );
      }
#endif
      જો( pSort ){
        પ્રતિજ્ઞા( regResult==regOrig );
        ધકેલો_Onto_Sorter(pParse, pSort, p, r1+nPrefixReg, regOrig, 1, nPrefixReg);
      }નહીંતર{
        પૂર્ણાંક r2 = sqlite3GetTempReg(pParse);
        sqlite3VdbeAddOp2(v, OP_NewRowid, iParm, r2);
        sqlite3VdbeAddOp3(v, OP_ઉમેરો, iParm, r1, r2);
        sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
        sqlite3ReleaseTempReg(pParse, r2);
      }
      sqlite3ReleaseTempRange(pParse, r1, nPrefixReg+1);
      તોડો;
    }

    સ્થિતિ SRT_Upfrom: {
      જો( pSort ){
        ધકેલો_Onto_Sorter(
            pParse, pSort, p, regResult, regOrig, nResultCol, nPrefixReg);
      }નહીંતર{
        પૂર્ણાંક i2 = pDest->iSDParm2;
        પૂર્ણાંક r1 = sqlite3GetTempReg(pParse);

        /* If the UPDATE FROM join is an aggregate that matches no rows, it
        ** might still be trying to return one row, because that is what
        ** aggregates do.  Don't record that empty row in the output table. */
        sqlite3VdbeAddOp2(v, OP_IsNull, regResult, iBreak); VdbeCoverage(v);

        sqlite3VdbeAddOp3(v, OP_MakeRecord,
                          regResult+(i2<0), nResultCol-(i2<0), r1);
        જો( i2<0 ){
          sqlite3VdbeAddOp3(v, OP_ઉમેરો, iParm, r1, regResult);
        }નહીંતર{
          sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, iParm, r1, regResult, i2);
        }
      }
      તોડો;
    }

#ifndef SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)" construct,
    ** then there should be a single item on the stack.  Write this
    ** item into the set table with bogus data.
    */
    સ્થિતિ SRT_Set: {
      જો( pSort ){
        /* At first glance you would think we could optimize out the
        ** ORDER BY in this case since the order of entries in the set
        ** does not matter.  But there might be a LIMIT clause, in which
        ** case the order does matter */
        ધકેલો_Onto_Sorter(
            pParse, pSort, p, regResult, regOrig, nResultCol, nPrefixReg);
        pDest->iSDParm2 = 0; /* Signal that any Bloom filter is unpopulated */
      }નહીંતર{
        પૂર્ણાંક r1 = sqlite3GetTempReg(pParse);
        પ્રતિજ્ઞા( sqlite3Strlen30(pDest->zAffSdst)==nResultCol );
        sqlite3VdbeAddOp4(v, OP_MakeRecord, regResult, nResultCol,
            r1, pDest->zAffSdst, nResultCol);
        sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, iParm, r1, regResult, nResultCol);
        જો( pDest->iSDParm2 ){
          પ્રતિજ્ઞા( sqlite3CursorBloomable(pParse,iParm,nResultCol) );
          sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pDest->iSDParm2, 0,
                               regResult, nResultCol);
          ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
        }
        sqlite3ReleaseTempReg(pParse, r1);
      }
      તોડો;
    }


    /* If any row exist in the result set, record that fact and abort.
    */
    સ્થિતિ SRT_Exists: {
      sqlite3VdbeAddOp2(v, OP_Integer, 1, iParm);
      /* The LIMIT clause will terminate the loop for us */
      તોડો;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell or array of
    ** memory cells and break out of the scan loop.
    */
    સ્થિતિ SRT_Mem: {
      જો( pSort ){
        પ્રતિજ્ઞા( nResultCol<=pDest->nSdst );
        ધકેલો_Onto_Sorter(
            pParse, pSort, p, regResult, regOrig, nResultCol, nPrefixReg);
        pDest->iSDParm = regResult;
      }નહીંતર{
        પ્રતિજ્ઞા( nResultCol==pDest->nSdst );
        જો( regResult!=iParm ){
          /* This occurs in cases where the SELECT had both a DISTINCT and
          ** an OFFSET clause.  */
          sqlite3VdbeAddOp3(v, OP_Copy, regResult, iParm, nResultCol-1);
        }
        /* The LIMIT clause will jump out of the loop for us */
      }
      તોડો;
    }
#endif /* #ifndef SQLITE_OMIT_SUBQUERY */

    સ્થિતિ SRT_Coroutine:       /* Send data to a co-routine */
    સ્થિતિ SRT_Output: {        /* Return the results */
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      જો( pSort ){
        ધકેલો_Onto_Sorter(pParse, pSort, p, regResult, regOrig, nResultCol,
                       nPrefixReg);
      }નહીંતર જો( eDest==SRT_Coroutine ){
        sqlite3VdbeAddOp1(v, OP_Yield, pDest->iSDParm);
      }નહીંતર{
        sqlite3VdbeAddOp2(v, OP_ResultRow, regResult, nResultCol);
      }
      તોડો;
    }

#ifndef SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    સ્થિતિ SRT_DistQueue:
    સ્થિતિ SRT_Queue: {
      પૂર્ણાંક n_ચાવી;
      પૂર્ણાંક r1, r2, r3;
      પૂર્ણાંક addrTest = 0;
      પદાવલિ_List *pSO;
      pSO = pDest->pOrderBy;
      પ્રતિજ્ઞા( pSO );
      n_ચાવી = pSO->n_પદાવલિ;
      r1 = sqlite3GetTempReg(pParse);
      r2 = sqlite3GetTempRange(pParse, n_ચાવી+2);
      r3 = r2+n_ચાવી+1;
      જો( eDest==SRT_DistQueue ){
        /* If the destination is DistQueue, then cursor (iParm+1) is open
        ** on a second ephemeral index that holds all values every previously
        ** added to the queue. */
        addrTest = sqlite3VdbeAddOp4Int(v, OP_Found, iParm+1, 0,
                                        regResult, nResultCol);
        VdbeCoverage(v);
      }
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regResult, nResultCol, r3);
      જો( eDest==SRT_DistQueue ){
        sqlite3VdbeAddOp2(v, OP_Idx_ઉમેરો, iParm+1, r3);
        sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      }
      માટે(i=0; i<n_ચાવી; i++){
        sqlite3VdbeAddOp2(v, OP_SCopy,
                          regResult + pSO->a[i].u.x.iOrderByCol - 1,
                          r2+i);
      }
      sqlite3VdbeAddOp2(v, OP_Sequence, iParm, r2+n_ચાવી);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, r2, n_ચાવી+2, r1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, iParm, r1, r2, n_ચાવી+2);
      જો( addrTest ) sqlite3VdbeJumpHere(v, addrTest);
      sqlite3ReleaseTempReg(pParse, r1);
      sqlite3ReleaseTempRange(pParse, r2, n_ચાવી+2);
      તોડો;
    }
#endif /* SQLITE_OMIT_CTE */



#જો !defined(SQLITE_OMIT_TRIGGER)
    /* Discard the results.  This is used for SELECT statements inside
    ** the body of a TRIGGER.  The purpose of such selects is to call
    ** user-defined functions that have side effects.  We do not care
    ** about the actual results of the select.
    */
    મૂળભૂત: {
      પ્રતિજ્ઞા( eDest==SRT_Discard );
      તોડો;
    }
#endif
  }

  /* Jump to the end of the loop if the LIMIT is reached.  Except, if
  ** there is a sorter, in which case the sorter has already limited
  ** the output for us.
  */
  જો( pSort==0 && p->iLimit ){
    sqlite3VdbeAddOp2(v, OP_DecrJumpZero, p->iLimit, iBreak); VdbeCoverage(v);
  }
}

/*
** Allocate a KeyInfo object sufficient for an index of N key columns and
** X extra columns.
*/
ચાવી_Info *sqlite3_ચાવી_Info_Alloc(sqlite3 *db, પૂર્ણાંક N, પૂર્ણાંક X){
  પૂર્ણાંક nExtra = (N+X)*(કદ(CollSeq*)+1);
  ચાવી_Info *p;
  પ્રતિજ્ઞા( X>=0 );
  જો( NEVER(N+X>0xffff) ) પાછા (ચાવી_Info*)sqlite3OomFault(db);
  p = sqlite3DbMallocRawNN(db, SZ_KEYINFO(0) + nExtra);
  જો( p ){
    p->aSortFlags = (u8*)&p->aColl[N+X];
    p->n_ચાવી_Field = (u16)N;
    p->nAllField = (u16)(N+X);
    p->enc = ENC(db);
    p->db = db;
    p->nRef = 1;
    memset(p->aColl, 0, nExtra);
  }નહીંતર{
    પાછા (ચાવી_Info*)sqlite3OomFault(db);
  }
  પાછા p;
}

/*
** Deallocate a KeyInfo object
*/
શૂન્ય sqlite3_ચાવી_Info_Unref(ચાવી_Info *p){
  જો( p ){
    પ્રતિજ્ઞા( p->db!=0 );
    પ્રતિજ્ઞા( p->nRef>0 );
    p->nRef--;
    જો( p->nRef==0 ) sqlite3DbNNFreeNN(p->db, p);
  }
}

/*
** Make a new pointer to a KeyInfo object
*/
ચાવી_Info *sqlite3_ચાવી_Info_Ref(ચાવી_Info *p){
  જો( p ){
    પ્રતિજ્ઞા( p->nRef>0 );
    p->nRef++;
  }
  પાછા p;
}

#ifdef SQLITE_DEBUG
/*
** Return TRUE if a KeyInfo object can be change.  The KeyInfo object
** can only be changed if this is just a single reference to the object.
**
** This routine is used only inside of assert() statements.
*/
પૂર્ણાંક sqlite3_ચાવી_Info_Is_Writeable(ચાવી_Info *p){ પાછા p->nRef==1; }
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
ચાવી_Info *sqlite3_ચાવી_Info_From_પદાવલિ_List(
  Parse *pParse,       /* Parsing context */
  પદાવલિ_List *pList,     /* Form the KeyInfo object from this ExprList */
  પૂર્ણાંક iStart,          /* Begin with this column of pList */
  પૂર્ણાંક nExtra           /* Add this many extra columns to the end */
){
  પૂર્ણાંક n_પદાવલિ;
  ચાવી_Info *pInfo;
  સંરચના પદાવલિ_List_item *pItem;
  sqlite3 *db = pParse->db;
  પૂર્ણાંક i;

  n_પદાવલિ = pList->n_પદાવલિ;
  pInfo = sqlite3_ચાવી_Info_Alloc(db, n_પદાવલિ-iStart, nExtra+1);
  જો( pInfo ){
    પ્રતિજ્ઞા( sqlite3_ચાવી_Info_Is_Writeable(pInfo) );
    માટે(i=iStart, pItem=pList->a+iStart; i<n_પદાવલિ; i++, pItem++){
      pInfo->aColl[i-iStart] = sqlite3_પદાવલિ_NNColl_Seq(pParse, pItem->p_પદાવલિ);
      pInfo->aSortFlags[i-iStart] = pItem->fg.sortFlags;
    }
  }
  પાછા pInfo;
}

/*
** Name of the connection operator, used for error messages.
*/
અચલ અક્ષર *sqlite3SelectOpName(પૂર્ણાંક id){
  અક્ષર *z;
  પસંદગી( id ){
    સ્થિતિ TK_ALL:       z = "UNION ALL";   તોડો;
    સ્થિતિ TK_INTERSECT: z = "INTERSECT";   તોડો;
    સ્થિતિ TK_EXCEPT:    z = "EXCEPT";      તોડો;
    મૂળભૂત:           z = "UNION";       તોડો;
  }
  પાછા z;
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
સ્થિર શૂન્ય explainTempTable(Parse *pParse, અચલ અક્ષર *zUsage){
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

#નહીંતર
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
સ્થિર શૂન્ય generateSortTail(
  Parse *pParse,    /* Parsing context */
  Select *p,        /* The SELECT statement */
  SortCtx *pSort,   /* Information on the ORDER BY clause */
  પૂર્ણાંક nColumn,      /* Number of columns of data */
  SelectDest *pDest /* Write the sorted results here */
){
  Vdbe *v = pParse->pVdbe;                     /* The prepared statement */
  પૂર્ણાંક addrBreak = pSort->labelDone;            /* Jump here to exit loop */
  પૂર્ણાંક addrContinue = sqlite3VdbeMakeLabel(pParse);/* Jump here for next cycle */
  પૂર્ણાંક addr;                       /* Top of output loop. Jump for Next. */
  પૂર્ણાંક addrOnce = 0;
  પૂર્ણાંક iTab;
  પદાવલિ_List *pOrderBy = pSort->pOrderBy;
  પૂર્ણાંક eDest = pDest->eDest;
  પૂર્ણાંક iParm = pDest->iSDParm;
  પૂર્ણાંક regRow;
  પૂર્ણાંક regRowid;
  પૂર્ણાંક iCol;
  પૂર્ણાંક n_ચાવી;                       /* Number of key columns in sorter record */
  પૂર્ણાંક iSortTab;                   /* Sorter cursor to read from */
  પૂર્ણાંક i;
  પૂર્ણાંક bSeq;                       /* True if sorter record includes seq. no. */
  પૂર્ણાંક n_Ref_ચાવી = 0;
  સંરચના પદાવલિ_List_item *aOutEx = p->pEList->a;
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  પૂર્ણાંક addrExplain;                /* Address of OP_Explain instruction */
#endif

  n_ચાવી = pOrderBy->n_પદાવલિ - pSort->nOBSat;
  જો( pSort->nOBSat==0 || n_ચાવી==1 ){
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR %sORDER BY", pSort->nOBSat?"LAST TERM OF ":""
    ));
  }નહીંતર{
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR LAST %d TERMS OF ORDER BY", n_ચાવી
    ));
  }
  sqlite3VdbeScanStatusRange(v, addrExplain,pSort->addr_ધકેલો,pSort->addr_ધકેલો_End);
  sqlite3VdbeScanStatusCounters(v, addrExplain, addrExplain, pSort->addr_ધકેલો);


  પ્રતિજ્ઞા( addrBreak<0 );
  જો( pSort->labelBkOut ){
    sqlite3VdbeAddOp2(v, OP_Gosub, pSort->regReturn, pSort->labelBkOut);
    sqlite3VdbeGoto(v, addrBreak);
    sqlite3VdbeResolveLabel(v, pSort->labelBkOut);
  }

#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  /* Open any cursors needed for sorter-reference expressions */
  માટે(i=0; i<pSort->nDefer; i++){
    Table *pTab = pSort->aDefer[i].pTab;
    પૂર્ણાંક iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
    sqlite3OpenTable(pParse, pSort->aDefer[i].iCsr, iDb, pTab, OP_OpenRead);
    sqlite3VdbeAddOp2(v, OP_Rewind, pSort->aDefer[i].iCsr,
                      sqlite3VdbeCurrentAddr(v)+1);
    VdbeCoverage(v);
    n_Ref_ચાવી = MAX(n_Ref_ચાવી, pSort->aDefer[i].n_ચાવી);
  }
#endif

  iTab = pSort->iECursor;
  જો( eDest==SRT_Output || eDest==SRT_Coroutine || eDest==SRT_Mem ){
    જો( eDest==SRT_Mem && p->iOffset ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, pDest->iSdst);
    }
    regRowid = 0;
    regRow = pDest->iSdst;
  }નહીંતર{
    regRowid = sqlite3GetTempReg(pParse);
    જો( eDest==SRT_EphemTab || eDest==SRT_Table ){
      regRow = sqlite3GetTempReg(pParse);
      nColumn = 0;
    }નહીંતર{
      regRow = sqlite3GetTempRange(pParse, nColumn);
    }
  }
  જો( pSort->sortFlags & SORTFLAG_UseSorter ){
    પૂર્ણાંક regSortOut = ++pParse->nMem;
    iSortTab = pParse->nTab++;
    જો( pSort->labelBkOut ){
      addrOnce = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
    }
    sqlite3VdbeAddOp3(v, OP_OpenPseudo, iSortTab, regSortOut,
        n_ચાવી+1+nColumn+n_Ref_ચાવી);
    જો( addrOnce ) sqlite3VdbeJumpHere(v, addrOnce);
    addr = 1 + sqlite3VdbeAddOp2(v, OP_SorterSort, iTab, addrBreak);
    VdbeCoverage(v);
    પ્રતિજ્ઞા( p->iLimit==0 && p->iOffset==0 );
    sqlite3VdbeAddOp3(v, OP_SorterData, iTab, regSortOut, iSortTab);
    bSeq = 0;
  }નહીંતર{
    addr = 1 + sqlite3VdbeAddOp2(v, OP_Sort, iTab, addrBreak); VdbeCoverage(v);
    codeOffset(v, p->iOffset, addrContinue);
    iSortTab = iTab;
    bSeq = 1;
    જો( p->iOffset>0 ){
      sqlite3VdbeAddOp2(v, OP_AddImm, p->iLimit, -1);
    }
  }
  માટે(i=0, iCol=n_ચાવી+bSeq-1; i<nColumn; i++){
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    જો( aOutEx[i].fg.bSorterRef ) ચાલુ;
#endif
    જો( aOutEx[i].u.x.iOrderByCol==0 ) iCol++;
  }
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
  જો( pSort->nDefer ){
    પૂર્ણાંક i_ચાવી = iCol+1;
    પૂર્ણાંક reg_ચાવી = sqlite3GetTempRange(pParse, n_Ref_ચાવી);

    માટે(i=0; i<pSort->nDefer; i++){
      પૂર્ણાંક iCsr = pSort->aDefer[i].iCsr;
      Table *pTab = pSort->aDefer[i].pTab;
      પૂર્ણાંક n_ચાવી = pSort->aDefer[i].n_ચાવી;

      sqlite3VdbeAddOp1(v, OP_NullRow, iCsr);
      જો( HasRowid(pTab) ){
        sqlite3VdbeAddOp3(v, OP_Column, iSortTab, i_ચાવી++, reg_ચાવી);
        sqlite3VdbeAddOp3(v, OP_SeekRowid, iCsr,
            sqlite3VdbeCurrentAddr(v)+1, reg_ચાવી);
      }નહીંતર{
        પૂર્ણાંક k;
        પૂર્ણાંક iJmp;
        પ્રતિજ્ઞા( sqlite3_Primary_ચાવી_Index(pTab)->n_ચાવી_Col==n_ચાવી );
        માટે(k=0; k<n_ચાવી; k++){
          sqlite3VdbeAddOp3(v, OP_Column, iSortTab, i_ચાવી++, reg_ચાવી+k);
        }
        iJmp = sqlite3VdbeCurrentAddr(v);
        sqlite3VdbeAddOp4Int(v, OP_SeekGE, iCsr, iJmp+2, reg_ચાવી, n_ચાવી);
        sqlite3VdbeAddOp4Int(v, OP_IdxLE, iCsr, iJmp+3, reg_ચાવી, n_ચાવી);
        sqlite3VdbeAddOp1(v, OP_NullRow, iCsr);
      }
    }
    sqlite3ReleaseTempRange(pParse, reg_ચાવી, n_Ref_ચાવી);
  }
#endif
  માટે(i=nColumn-1; i>=0; i--){
#ifdef SQLITE_ENABLE_SORTER_REFERENCES
    જો( aOutEx[i].fg.bSorterRef ){
      sqlite3_પદાવલિ_Code(pParse, aOutEx[i].p_પદાવલિ, regRow+i);
    }નહીંતર
#endif
    {
      પૂર્ણાંક iRead;
      જો( aOutEx[i].u.x.iOrderByCol ){
        iRead = aOutEx[i].u.x.iOrderByCol-1;
      }નહીંતર{
        iRead = iCol--;
      }
      sqlite3VdbeAddOp3(v, OP_Column, iSortTab, iRead, regRow+i);
      VdbeComment((v, "%s", aOutEx[i].zEName));
    }
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);
  પસંદગી( eDest ){
    સ્થિતિ SRT_Table:
    સ્થિતિ SRT_EphemTab: {
      sqlite3VdbeAddOp3(v, OP_Column, iSortTab, n_ચાવી+bSeq, regRow);
      sqlite3VdbeAddOp2(v, OP_NewRowid, iParm, regRowid);
      sqlite3VdbeAddOp3(v, OP_ઉમેરો, iParm, regRow, regRowid);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      તોડો;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    સ્થિતિ SRT_Set: {
      પ્રતિજ્ઞા( nColumn==sqlite3Strlen30(pDest->zAffSdst) );
      sqlite3VdbeAddOp4(v, OP_MakeRecord, regRow, nColumn, regRowid,
                        pDest->zAffSdst, nColumn);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, iParm, regRowid, regRow, nColumn);
      તોડો;
    }
    સ્થિતિ SRT_Mem: {
      /* The LIMIT clause will terminate the loop for us */
      તોડો;
    }
#endif
    સ્થિતિ SRT_Upfrom: {
      પૂર્ણાંક i2 = pDest->iSDParm2;
      પૂર્ણાંક r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp3(v, OP_MakeRecord,regRow+(i2<0),nColumn-(i2<0),r1);
      જો( i2<0 ){
        sqlite3VdbeAddOp3(v, OP_ઉમેરો, iParm, r1, regRow);
      }નહીંતર{
        sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, iParm, r1, regRow, i2);
      }
      તોડો;
    }
    મૂળભૂત: {
      પ્રતિજ્ઞા( eDest==SRT_Output || eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      testcase( eDest==SRT_Coroutine );
      જો( eDest==SRT_Output ){
        sqlite3VdbeAddOp2(v, OP_ResultRow, pDest->iSdst, nColumn);
      }નહીંતર{
        sqlite3VdbeAddOp1(v, OP_Yield, pDest->iSDParm);
      }
      તોડો;
    }
  }
  જો( regRowid ){
    જો( eDest==SRT_Set ){
      sqlite3ReleaseTempRange(pParse, regRow, nColumn);
    }નહીંતર{
      sqlite3ReleaseTempReg(pParse, regRow);
    }
    sqlite3ReleaseTempReg(pParse, regRowid);
  }
  /* The bottom of the loop
  */
  sqlite3VdbeResolveLabel(v, addrContinue);
  જો( pSort->sortFlags & SORTFLAG_UseSorter ){
    sqlite3VdbeAddOp2(v, OP_SorterNext, iTab, addr); VdbeCoverage(v);
  }નહીંતર{
    sqlite3VdbeAddOp2(v, OP_Next, iTab, addr); VdbeCoverage(v);
  }
  sqlite3VdbeScanStatusRange(v, addrExplain, sqlite3VdbeCurrentAddr(v)-1, -1);
  જો( pSort->regReturn ) sqlite3VdbeAddOp1(v, OP_Return, pSort->regReturn);
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
#નહીંતર /* if !defined(SQLITE_ENABLE_COLUMN_METADATA) */
# define columnType(A,B,C,D,E) columnTypeImpl(A,B)
#endif
સ્થિર અચલ અક્ષર *columnTypeImpl(
  NameContext *pNC,
#ifndef SQLITE_ENABLE_COLUMN_METADATA
  પદાવલિ *p_પદાવલિ
#નહીંતર
  પદાવલિ *p_પદાવલિ,
  અચલ અક્ષર **pzOrigDb,
  અચલ અક્ષર **pzOrigTab,
  અચલ અક્ષર **pzOrigCol
#endif
){
  અક્ષર અચલ *zType = 0;
  પૂર્ણાંક j;
#ifdef SQLITE_ENABLE_COLUMN_METADATA
  અક્ષર અચલ *zOrigDb = 0;
  અક્ષર અચલ *zOrigTab = 0;
  અક્ષર અચલ *zOrigCol = 0;
#endif

  પ્રતિજ્ઞા( p_પદાવલિ!=0 );
  પ્રતિજ્ઞા( pNC->pSrcList!=0 );
  પસંદગી( p_પદાવલિ->op ){
    સ્થિતિ TK_COLUMN: {
      /* The expression is a column. Locate the table the column is being
      ** extracted from in NameContext.pSrcList. This table may be real
      ** database table or a subquery.
      */
      Table *pTab = 0;            /* Table structure column is extracted from */
      Select *pS = 0;             /* Select the column is extracted from */
      પૂર્ણાંક iCol = p_પદાવલિ->iColumn;  /* Index of column in pTab */
      જ્યારે( pNC && !pTab ){
        SrcList *pTabList = pNC->pSrcList;
        માટે(j=0;j<pTabList->nSrc && pTabList->a[j].iCursor!=p_પદાવલિ->iTable;j++);
        જો( j<pTabList->nSrc ){
          pTab = pTabList->a[j].pSTab;
          જો( pTabList->a[j].fg.isSubquery ){
            pS = pTabList->a[j].u4.pSubq->pSelect;
          }નહીંતર{
            pS = 0;
          }
        }નહીંતર{
          pNC = pNC->pNext;
        }
      }

      જો( pTab==0 ){
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
        તોડો;
      }

      પ્રતિજ્ઞા( pTab && પદાવલિ_Use_YTab(p_પદાવલિ) && p_પદાવલિ->y.pTab==pTab );
      જો( pS ){
        /* The "table" is actually a sub-select or a view in the FROM clause
        ** of the SELECT statement. Return the declaration type and origin
        ** data for the result-set column of the sub-select.
        */
        જો( iCol<pS->pEList->n_પદાવલિ
         && (!ViewCanHaveRowid || iCol>=0)
        ){
          /* If iCol is less than zero, then the expression requests the
          ** rowid of the sub-select or view. This expression is legal (see
          ** test case misc2.2.2) - it always evaluates to NULL.
          */
          NameContext sNC;
          પદાવલિ *p = pS->pEList->a[iCol].p_પદાવલિ;
          sNC.pSrcList = pS->pSrc;
          sNC.pNext = pNC;
          sNC.pParse = pNC->pParse;
          zType = columnType(&sNC, p,&zOrigDb,&zOrigTab,&zOrigCol);
        }
      }નહીંતર{
        /* A real table or a CTE table */
        પ્રતિજ્ઞા( !pS );
#ifdef SQLITE_ENABLE_COLUMN_METADATA
        જો( iCol<0 ) iCol = pTab->iPKey;
        પ્રતિજ્ઞા( iCol==XN_ROWID || (iCol>=0 && iCol<pTab->nCol) );
        જો( iCol<0 ){
          zType = "INTEGER";
          zOrigCol = "rowid";
        }નહીંતર{
          zOrigCol = pTab->aCol[iCol].zCnName;
          zType = sqlite3ColumnType(&pTab->aCol[iCol],0);
        }
        zOrigTab = pTab->zName;
        જો( pNC->pParse && pTab->pSchema ){
          પૂર્ણાંક iDb = sqlite3SchemaToIndex(pNC->pParse->db, pTab->pSchema);
          zOrigDb = pNC->pParse->db->aDb[iDb].zDbSName;
        }
#નહીંતર
        પ્રતિજ્ઞા( iCol==XN_ROWID || (iCol>=0 && iCol<pTab->nCol) );
        જો( iCol<0 ){
          zType = "INTEGER";
        }નહીંતર{
          zType = sqlite3ColumnType(&pTab->aCol[iCol],0);
        }
#endif
      }
      તોડો;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    સ્થિતિ TK_SELECT: {
      /* The expression is a sub-select. Return the declaration type and
      ** origin info for the single column in the result set of the SELECT
      ** statement.
      */
      NameContext sNC;
      Select *pS;
      પદાવલિ *p;
      પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(p_પદાવલિ) );
      pS = p_પદાવલિ->x.pSelect;
      p = pS->pEList->a[0].p_પદાવલિ;
      sNC.pSrcList = pS->pSrc;
      sNC.pNext = pNC;
      sNC.pParse = pNC->pParse;
      zType = columnType(&sNC, p, &zOrigDb, &zOrigTab, &zOrigCol);
      તોડો;
    }
#endif
  }

#ifdef SQLITE_ENABLE_COLUMN_METADATA 
  જો( pzOrigDb ){
    પ્રતિજ્ઞા( pzOrigTab && pzOrigCol );
    *pzOrigDb = zOrigDb;
    *pzOrigTab = zOrigTab;
    *pzOrigCol = zOrigCol;
  }
#endif
  પાછા zType;
}

/*
** Generate code that will tell the VDBE the declaration types of columns
** in the result set.
*/
સ્થિર શૂન્ય generateColumnTypes(
  Parse *pParse,      /* Parser context */
  SrcList *pTabList,  /* List of tables */
  પદાવલિ_List *pEList    /* Expressions defining the result set */
){
#ifndef SQLITE_OMIT_DECLTYPE
  Vdbe *v = pParse->pVdbe;
  પૂર્ણાંક i;
  NameContext sNC;
  sNC.pSrcList = pTabList;
  sNC.pParse = pParse;
  sNC.pNext = 0;
  માટે(i=0; i<pEList->n_પદાવલિ; i++){
    પદાવલિ *p = pEList->a[i].p_પદાવલિ;
    અચલ અક્ષર *zType;
#ifdef SQLITE_ENABLE_COLUMN_METADATA
    અચલ અક્ષર *zOrigDb = 0;
    અચલ અક્ષર *zOrigTab = 0;
    અચલ અક્ષર *zOrigCol = 0;
    zType = columnType(&sNC, p, &zOrigDb, &zOrigTab, &zOrigCol);

    /* The vdbe must make its own copy of the column-type and other
    ** column specific strings, in case the schema is reset before this
    ** virtual machine is deleted.
    */
    sqlite3VdbeSetColName(v, i, COLNAME_DATABASE, zOrigDb, SQLITE_TRANSIENT);
    sqlite3VdbeSetColName(v, i, COLNAME_TABLE, zOrigTab, SQLITE_TRANSIENT);
    sqlite3VdbeSetColName(v, i, COLNAME_COLUMN, zOrigCol, SQLITE_TRANSIENT);
#નહીંતર
    zType = columnType(&sNC, p, 0, 0, 0);
#endif
    sqlite3VdbeSetColName(v, i, COLNAME_DECLTYPE, zType, SQLITE_TRANSIENT);
  }
#નહીંતર
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
શૂન્ય sqlite3GenerateColumnNames(
  Parse *pParse,      /* Parser context */
  Select *pSelect     /* Generate column names for this SELECT statement */
){
  Vdbe *v = pParse->pVdbe;
  પૂર્ણાંક i;
  Table *pTab;
  SrcList *pTabList;
  પદાવલિ_List *pEList;
  sqlite3 *db = pParse->db;
  પૂર્ણાંક fullName;    /* TABLE.COLUMN if no AS clause and is a direct table ref */
  પૂર્ણાંક srcName;     /* COLUMN or TABLE.COLUMN if no AS clause and is direct */

  જો( pParse->colNamesSet ) પાછા;
  /* Column names are determined by the left-most term of a compound select */
  જ્યારે( pSelect->pPrior ) pSelect = pSelect->pPrior;
  TREETRACE(0x80,pParse,pSelect,("generating column names\n"));
  pTabList = pSelect->pSrc;
  pEList = pSelect->pEList;
  પ્રતિજ્ઞા( v!=0 );
  પ્રતિજ્ઞા( pTabList!=0 );
  pParse->colNamesSet = 1;
  fullName = (db->flags & SQLITE_FullColNames)!=0;
  srcName = (db->flags & SQLITE_ShortColNames)!=0 || fullName;
  sqlite3_Vdbe_Set_અંક_Cols(v, pEList->n_પદાવલિ);
  માટે(i=0; i<pEList->n_પદાવલિ; i++){
    પદાવલિ *p = pEList->a[i].p_પદાવલિ;

    પ્રતિજ્ઞા( p!=0 );
    પ્રતિજ્ઞા( p->op!=TK_AGG_COLUMN );  /* Agg processing has not run yet */
    પ્રતિજ્ઞા( p->op!=TK_COLUMN
        || (પદાવલિ_Use_YTab(p) && p->y.pTab!=0) ); /* Covering idx not yet coded */
    જો( pEList->a[i].zEName && pEList->a[i].fg.eEName==ENAME_NAME ){
      /* An AS clause always takes first priority */
      અક્ષર *zName = pEList->a[i].zEName;
      sqlite3VdbeSetColName(v, i, COLNAME_NAME, zName, SQLITE_TRANSIENT);
    }નહીંતર જો( srcName && p->op==TK_COLUMN ){
      અક્ષર *zCol;
      પૂર્ણાંક iCol = p->iColumn;
      pTab = p->y.pTab;
      પ્રતિજ્ઞા( pTab!=0 );
      જો( iCol<0 ) iCol = pTab->iPKey;
      પ્રતિજ્ઞા( iCol==-1 || (iCol>=0 && iCol<pTab->nCol) );
      જો( iCol<0 ){
        zCol = "rowid";
      }નહીંતર{
        zCol = pTab->aCol[iCol].zCnName;
      }
      જો( fullName ){
        અક્ષર *zName = 0;
        zName = sqlite3MPrintf(db, "%s.%s", pTab->zName, zCol);
        sqlite3VdbeSetColName(v, i, COLNAME_NAME, zName, SQLITE_DYNAMIC);
      }નહીંતર{
        sqlite3VdbeSetColName(v, i, COLNAME_NAME, zCol, SQLITE_TRANSIENT);
      }
    }નહીંતર{
      અચલ અક્ષર *z = pEList->a[i].zEName;
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
પૂર્ણાંક sqlite3_Columns_From_પદાવલિ_List(
  Parse *pParse,          /* Parsing context */
  પદાવલિ_List *pEList,       /* Expr list from which to derive column names */
  i16 *pnCol,             /* Write the number of columns here */
  Column **paCol          /* Write the new column list here */
){
  sqlite3 *db = pParse->db;   /* Database connection */
  પૂર્ણાંક i, j;                   /* Loop counters */
  u32 cnt;                    /* Index added to make the name unique */
  Column *aCol, *pCol;        /* For looping over result columns */
  પૂર્ણાંક nCol;                   /* Number of columns in the result set */
  અક્ષર *zName;                /* Column name */
  પૂર્ણાંક nName;                  /* Size of name in zName[] */
  Hash ht;                    /* Hash table of column names */
  Table *pTab;

  sqlite3HashInit(&ht);
  જો( pEList ){
    nCol = pEList->n_પદાવલિ;
    aCol = sqlite3DbMallocZero(db, કદ(aCol[0])*nCol);
    testcase( aCol==0 );
    જો( nCol>32767 ) nCol = 32767;
  }નહીંતર{
    nCol = 0;
    aCol = 0;
  }
  પ્રતિજ્ઞા( nCol==(i16)nCol );
  *pnCol = nCol;
  *paCol = aCol;

  માટે(i=0, pCol=aCol; i<nCol && !pParse->nErr; i++, pCol++){
    સંરચના પદાવલિ_List_item *pX = &pEList->a[i];
    સંરચના પદાવલિ_List_item *pCollide;
    /* Get an appropriate name for the column
    */
    જો( (zName = pX->zEName)!=0 && pX->fg.eEName==ENAME_NAME ){
      /* If the column contains an "AS <name>" phrase, use <name> as the name */
    }નહીંતર{
      પદાવલિ *p_Col_પદાવલિ = sqlite3_પદાવલિ_Skip_Collate_And_Likely(pX->p_પદાવલિ);
      જ્યારે( ALWAYS(p_Col_પદાવલિ!=0) && p_Col_પદાવલિ->op==TK_DOT ){
        p_Col_પદાવલિ = p_Col_પદાવલિ->p_જમણું;
        પ્રતિજ્ઞા( p_Col_પદાવલિ!=0 );
      }
      જો( p_Col_પદાવલિ->op==TK_COLUMN
       && ALWAYS( પદાવલિ_Use_YTab(p_Col_પદાવલિ) )
       && ALWAYS( p_Col_પદાવલિ->y.pTab!=0 )
      ){
        /* For columns use the column name name */
        પૂર્ણાંક iCol = p_Col_પદાવલિ->iColumn;
        pTab = p_Col_પદાવલિ->y.pTab;
        જો( iCol<0 ) iCol = pTab->iPKey;
        zName = iCol>=0 ? pTab->aCol[iCol].zCnName : "rowid";
      }નહીંતર જો( p_Col_પદાવલિ->op==TK_ID ){
        પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_Col_પદાવલિ, EP_Int_કિંમત) );
        zName = p_Col_પદાવલિ->u.zToken;
      }નહીંતર{
        /* Use the original text of the column expression as its name */
        પ્રતિજ્ઞા( zName==pX->zEName );  /* pointer comparison intended */
      }
    }
    જો( zName && !sqlite3IsTrueOrFalse(zName) ){
      zName = sqlite3DbStrDup(db, zName);
    }નહીંતર{
      zName = sqlite3MPrintf(db,"column%d",i+1);
    }

    /* Make sure the column name is unique.  If the name is not unique,
    ** append an integer to the name so that it becomes unique.
    */
    cnt = 0;
    જ્યારે( zName && (pCollide = sqlite3HashFind(&ht, zName))!=0 ){
      જો( pCollide->fg.bUsingTerm ){
        pCol->colFlags |= COLFLAG_NOEXPAND;
      }
      nName = sqlite3Strlen30(zName);
      જો( nName>0 ){
        માટે(j=nName-1; j>0 && sqlite3Isdigit(zName[j]); j--){}
        જો( zName[j]==':' ) nName = j;
      }
      zName = sqlite3MPrintf(db, "%.*z:%u", nName, zName, ++cnt);
      sqlite3ProgressCheck(pParse);
      જો( cnt>3 ){
        sqlite3_randomness(કદ(cnt), &cnt);
      }
    }
    pCol->zCnName = zName;
    pCol->hName = sqlite3StrIHash(zName);
    જો( pX->fg.bNoExpand ){
      pCol->colFlags |= COLFLAG_NOEXPAND;
    }
    sqlite3ColumnPropertiesFromName(0, pCol);
    જો( zName && sqlite3_Hash_ઉમેરો(&ht, zName, pX)==pX ){
      sqlite3OomFault(db);
    }
  }
  sqlite3HashClear(&ht);
  જો( pParse->nErr ){
    માટે(j=0; j<i; j++){
      sqlite3DbFree(db, aCol[j].zCnName);
    }
    sqlite3DbFree(db, aCol);
    *paCol = 0;
    *pnCol = 0;
    પાછા pParse->rc;
  }
  પાછા SQLITE_OK;
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
શૂન્ય sqlite3SubqueryColumnTypes(
  Parse *pParse,      /* Parsing contexts */
  Table *pTab,        /* Add column type information to this table */
  Select *pSelect,    /* SELECT used to determine types and collations */
  અક્ષર aff            /* Default affinity. */
){
  sqlite3 *db = pParse->db;
  Column *pCol;
  CollSeq *pColl;
  પૂર્ણાંક i,j;
  પદાવલિ *p;
  સંરચના પદાવલિ_List_item *a;
  NameContext sNC;

  પ્રતિજ્ઞા( pSelect!=0 );
  પ્રતિજ્ઞા( (pSelect->selFlags & SF_Resolved)!=0 );
  પ્રતિજ્ઞા( pTab->nCol==pSelect->pEList->n_પદાવલિ || pParse->nErr>0 );
  પ્રતિજ્ઞા( aff==SQLITE_AFF_NONE || aff==SQLITE_AFF_BLOB );
  જો( db->mallocFailed || IN_RENAME_OBJECT ) પાછા;
  જ્યારે( pSelect->pPrior ) pSelect = pSelect->pPrior;
  a = pSelect->pEList->a;
  memset(&sNC, 0, કદ(sNC));
  sNC.pSrcList = pSelect->pSrc;
  માટે(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
    અચલ અક્ષર *zType;
    i64 n;
    પૂર્ણાંક m = 0;
    Select *pS2 = pSelect;
    pTab->tabFlags |= (pCol->colFlags & COLFLAG_NOINSERT);
    p = a[i].p_પદાવલિ;
    /* pCol->szEst = ... // Column size est for SELECT tables never used */
    pCol->affinity = sqlite3_પદાવલિ_Affinity(p);
    જ્યારે( pCol->affinity<=SQLITE_AFF_NONE && pS2->pNext!=0 ){
      m |= sqlite3_પદાવલિ_Data_Type(pS2->pEList->a[i].p_પદાવલિ);
      pS2 = pS2->pNext;
      pCol->affinity = sqlite3_પદાવલિ_Affinity(pS2->pEList->a[i].p_પદાવલિ);
    }
    જો( pCol->affinity<=SQLITE_AFF_NONE ){
      pCol->affinity = aff;
    }
    જો( pCol->affinity>=SQLITE_AFF_TEXT && (pS2->pNext || pS2!=pSelect) ){
      માટે(pS2=pS2->pNext; pS2; pS2=pS2->pNext){
        m |= sqlite3_પદાવલિ_Data_Type(pS2->pEList->a[i].p_પદાવલિ);
      }
      જો( pCol->affinity==SQLITE_AFF_TEXT && (m&0x01)!=0 ){
        pCol->affinity = SQLITE_AFF_BLOB;
      }નહીંતર
      જો( pCol->affinity>=SQLITE_AFF_NUMERIC && (m&0x02)!=0 ){
        pCol->affinity = SQLITE_AFF_BLOB;
      }
      જો( pCol->affinity>=SQLITE_AFF_NUMERIC && p->op==TK_CAST ){
        pCol->affinity = SQLITE_AFF_FLEXNUM;
      }
    }
    zType = columnType(&sNC, p, 0, 0, 0);
    જો( zType==0 || pCol->affinity!=sqlite3AffinityType(zType, 0) ){
      જો( pCol->affinity==SQLITE_AFF_NUMERIC
       || pCol->affinity==SQLITE_AFF_FLEXNUM
      ){
        zType = "NUM";
      }નહીંતર{
        zType = 0;
        માટે(j=1; j<SQLITE_N_STDTYPE; j++){
          જો( sqlite3StdTypeAffinity[j]==pCol->affinity ){
            zType = sqlite3StdType[j];
            તોડો;
          }
        }
      }
    }
    જો( zType ){
      અચલ i64 k = strlen(zType);
      n = strlen(pCol->zCnName);
      pCol->zCnName = sqlite3DbReallocOrFree(db, pCol->zCnName, n+k+2);
      pCol->colFlags &= ~(COLFLAG_HASTYPE|COLFLAG_HASCOLL);
      જો( pCol->zCnName ){
        memcpy(&pCol->zCnName[n+1], zType, k+1);
        pCol->colFlags |= COLFLAG_HASTYPE;
      }
    }
    pColl = sqlite3_પદાવલિ_Coll_Seq(pParse, p);
    જો( pColl ){
      પ્રતિજ્ઞા( pTab->pIndex==0 );
      sqlite3ColumnSetColl(db, pCol, pColl->zName);
    }
  }
  pTab->szTabRow = 1; /* Any non-zero value works */
}

/*
** Check the current subquery nesting depth.  Return true and set an
** error if it has gone too deep.
*/
સ્થિર પૂર્ણાંક checkSubqueryNestingDepth(Parse *pParse){
#જો SQLITE_MAX_પદાવલિ_DEPTH>0
  જો( pParse->nNestSel >= pParse->db->aLimit[SQLITE_LIMIT_પદાવલિ_DEPTH] ){
    sqlite3_Error_સંદેશ(pParse, "VIEWs and/or subqueries nested too deep");
    પાછા 1;
  }
#endif
  પાછા 0;
}

/*
** Given a SELECT statement, generate a Table structure that describes
** the result set of that SELECT.
*/
Table *sqlite3ResultSetOfSelect(Parse *pParse, Select *pSelect, અક્ષર aff){
  Table *pTab;
  sqlite3 *db = pParse->db;
  u64 savedFlags;

  pParse->nNestSel++;
  જો( checkSubqueryNestingDepth(pParse) ) પાછા 0;
  savedFlags = db->flags;
  db->flags &= ~(u64)SQLITE_FullColNames;
  db->flags |= SQLITE_ShortColNames;
  sqlite3SelectPrep(pParse, pSelect, 0);
  db->flags = savedFlags;
  જો( pParse->nErr ) પાછા 0;
  જ્યારે( pSelect->pPrior ) pSelect = pSelect->pPrior;
  pTab = sqlite3DbMallocZero(db, કદ(Table) );
  જો( pTab==0 ){
    પાછા 0;
  }
  pTab->nTabRef = 1;
  pTab->zName = 0;
  pTab->nRowLogEst = 200; પ્રતિજ્ઞા( 200==sqlite3LogEst(1048576) );
  sqlite3_Columns_From_પદાવલિ_List(pParse, pSelect->pEList, &pTab->nCol, &pTab->aCol);
  sqlite3SubqueryColumnTypes(pParse, pTab, pSelect, aff);
  pTab->iPKey = -1;
  જો( db->mallocFailed ){
    sqlite3DeleteTable(db, pTab);
    પાછા 0;
  }
  pParse->nNestSel--;
  પ્રતિજ્ઞા( pParse->nNestSel>=0 );
  પાછા pTab;
}

/*
** Get a VDBE for the given parser context.  Create a new one if necessary.
** If an error occurs, return NULL and leave a message in pParse.
*/
Vdbe *sqlite3GetVdbe(Parse *pParse){
  જો( pParse->pVdbe ){
    પાછા pParse->pVdbe;
  }
  જો( pParse->pToplevel==0 ){
    pParse->okConstFactor = 1;
  }
  પાછા sqlite3VdbeCreate(pParse);
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
સ્થિર શૂન્ય computeLimitRegisters(Parse *pParse, Select *p, પૂર્ણાંક iBreak){
  Vdbe *v = 0;
  પૂર્ણાંક iLimit = 0;
  પૂર્ણાંક iOffset;
  પૂર્ણાંક n;
  પદાવલિ *pLimit = p->pLimit;

  જો( p->iLimit ) પાછા;

  /*
  ** "LIMIT -1" always shows all rows.  There is some
  ** controversy about what the correct behavior should be.
  ** The current implementation interprets "LIMIT 0" to mean
  ** no rows.
  */
  જો( pLimit ){
    પ્રતિજ્ઞા( pLimit->op==TK_LIMIT );
    પ્રતિજ્ઞા( pLimit->p_ડાબું!=0 );
    p->iLimit = iLimit = ++pParse->nMem;
    v = sqlite3GetVdbe(pParse);
    પ્રતિજ્ઞા( v!=0 );
    જો( sqlite3_પદાવલિ_Is_Integer(pLimit->p_ડાબું, &n, pParse, 0) ){
      sqlite3VdbeAddOp2(v, OP_Integer, n, iLimit);
      VdbeComment((v, "LIMIT counter"));
      જો( n==0 ){
        sqlite3VdbeGoto(v, iBreak);
      }નહીંતર જો( n>=0 && p->nSelectRow>sqlite3LogEst((u64)n) ){
        p->nSelectRow = sqlite3LogEst((u64)n);
        p->selFlags |= SF_FixedLimit;
      }
    }નહીંતર{
      sqlite3_પદાવલિ_Code(pParse, pLimit->p_ડાબું, iLimit);
      sqlite3VdbeAddOp1(v, OP_MustBeInt, iLimit); VdbeCoverage(v);
      VdbeComment((v, "LIMIT counter"));
      sqlite3MayAbort(pParse);
      sqlite3VdbeAddOp2(v, OP_IfNot, iLimit, iBreak); VdbeCoverage(v);
    }
    જો( pLimit->p_જમણું ){
      p->iOffset = iOffset = ++pParse->nMem;
      pParse->nMem++;   /* Allocate an extra register for limit+offset */
      sqlite3_પદાવલિ_Code(pParse, pLimit->p_જમણું, iOffset);
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
સ્થિર CollSeq *multiSelectCollSeq(Parse *pParse, Select *p, પૂર્ણાંક iCol){
  CollSeq *pRet;
  જો( p->pPrior ){
    pRet = multiSelectCollSeq(pParse, p->pPrior, iCol);
  }નહીંતર{
    pRet = 0;
  }
  પ્રતિજ્ઞા( iCol>=0 );
  /* iCol must be less than p->pEList->nExpr.  Otherwise an error would
  ** have been thrown during name resolution and we would not have gotten
  ** this far */
  જો( pRet==0 && ALWAYS(iCol<p->pEList->n_પદાવલિ) ){
    pRet = sqlite3_પદાવલિ_Coll_Seq(pParse, p->pEList->a[iCol].p_પદાવલિ);
  }
  પાછા pRet;
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
સ્થિર ચાવી_Info *multi_Select_By_Merge_ચાવી_Info(Parse *pParse, Select *p, પૂર્ણાંક nExtra){
  પદાવલિ_List *pOrderBy = p->pOrderBy;
  પૂર્ણાંક nOrderBy = (pOrderBy!=0) ? pOrderBy->n_પદાવલિ : 0;
  sqlite3 *db = pParse->db;
  ચાવી_Info *pRet = sqlite3_ચાવી_Info_Alloc(db, nOrderBy+nExtra, 1);
  જો( pRet ){
    પૂર્ણાંક i;
    માટે(i=0; i<nOrderBy; i++){
      સંરચના પદાવલિ_List_item *pItem = &pOrderBy->a[i];
      પદાવલિ *pTerm = pItem->p_પદાવલિ;
      CollSeq *pColl;

      જો( pTerm->flags & EP_Collate ){
        pColl = sqlite3_પદાવલિ_Coll_Seq(pParse, pTerm);
      }નહીંતર{
        pColl = multiSelectCollSeq(pParse, p, pItem->u.x.iOrderByCol-1);
        જો( pColl==0 ) pColl = db->pDfltColl;
        pOrderBy->a[i].p_પદાવલિ =
          sqlite3_પદાવલિ_Add_Collate_String(pParse, pTerm, pColl->zName);
      }
      પ્રતિજ્ઞા( sqlite3_ચાવી_Info_Is_Writeable(pRet) );
      pRet->aColl[i] = pColl;
      pRet->aSortFlags[i] = pOrderBy->a[i].fg.sortFlags;
    }
  }

  પાછા pRet;
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
સ્થિર શૂન્ય generateWithRecursiveQuery(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The recursive SELECT to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  SrcList *pSrc = p->pSrc;      /* The FROM clause of the recursive query */
  પૂર્ણાંક nCol = p->pEList->n_પદાવલિ;  /* Number of columns in the recursive table */
  Vdbe *v = pParse->pVdbe;      /* The prepared statement under construction */
  Select *pSetup;               /* The setup query */
  Select *pFirstRec;            /* Left-most recursive term */
  પૂર્ણાંક addr_ટોચ;                  /* Top of the loop */
  પૂર્ણાંક addrCont, addrBreak;      /* CONTINUE and BREAK addresses */
  પૂર્ણાંક iCurrent = 0;             /* The Current table */
  પૂર્ણાંક regCurrent;               /* Register holding Current table */
  પૂર્ણાંક iQueue;                   /* The Queue table */
  પૂર્ણાંક iDistinct = 0;            /* To ensure unique results if UNION */
  પૂર્ણાંક eDest = SRT_Fifo;         /* How to write to Queue */
  SelectDest destQueue;         /* SelectDest targeting the Queue table */
  પૂર્ણાંક i;                        /* Loop counter */
  પૂર્ણાંક rc;                       /* Result code */
  પદાવલિ_List *pOrderBy;           /* The ORDER BY clause */
  પદાવલિ *pLimit;                 /* Saved LIMIT and OFFSET */
  પૂર્ણાંક regLimit, regOffset;      /* Registers used by LIMIT and OFFSET */

#ifndef SQLITE_OMIT_WINDOWFUNC
  જો( p->pWin ){
    sqlite3_Error_સંદેશ(pParse, "cannot use window functions in recursive queries");
    પાછા;
  }
#endif

  /* Obtain authorization to do a recursive query */
  જો( sqlite3AuthCheck(pParse, SQLITE_RECURSIVE, 0, 0, 0) ) પાછા;

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
  માટે(i=0; ALWAYS(i<pSrc->nSrc); i++){
    જો( pSrc->a[i].fg.isRecursive ){
      iCurrent = pSrc->a[i].iCursor;
      તોડો;
    }
  }

  /* Allocate cursors numbers for Queue and Distinct.  The cursor number for
  ** the Distinct table must be exactly one greater than Queue in order
  ** for the SRT_DistFifo and SRT_DistQueue destinations to work. */
  iQueue = pParse->nTab++;
  જો( p->op==TK_UNION ){
    eDest = pOrderBy ? SRT_DistQueue : SRT_DistFifo;
    iDistinct = pParse->nTab++;
  }નહીંતર{
    eDest = pOrderBy ? SRT_Queue : SRT_Fifo;
  }
  sqlite3SelectDestInit(&destQueue, eDest, iQueue);

  /* Allocate cursors for Current, Queue, and Distinct. */
  regCurrent = ++pParse->nMem;
  sqlite3VdbeAddOp3(v, OP_OpenPseudo, iCurrent, regCurrent, nCol);
  જો( pOrderBy ){
    ચાવી_Info *p_ચાવી_Info = multi_Select_By_Merge_ચાવી_Info(pParse, p, 1);
    sqlite3VdbeAddOp4(v, OP_OpenEphemeral, iQueue, pOrderBy->n_પદાવલિ+2, 0,
                      (અક્ષર*)p_ચાવી_Info, P4_KEYINFO);
    destQueue.pOrderBy = pOrderBy;
  }નહીંતર{
    sqlite3VdbeAddOp2(v, OP_OpenEphemeral, iQueue, nCol);
  }
  VdbeComment((v, "Queue table"));
  જો( iDistinct ){
    /* Generate an ephemeral table used to enforce distinctness on the
    ** output of the recursive part of the CTE.
    */
    ચાવી_Info *p_ચાવી_Info;            /* Collating sequence for the result set */
    CollSeq **apColl;             /* For looping through pKeyInfo->aColl[] */

    પ્રતિજ્ઞા( p->pNext==0 );
    પ્રતિજ્ઞા( p->pEList!=0 );
    nCol = p->pEList->n_પદાવલિ;
    p_ચાવી_Info = sqlite3_ચાવી_Info_Alloc(pParse->db, nCol, 1);
    જો( p_ચાવી_Info ){
      માટે(i=0, apColl=p_ચાવી_Info->aColl; i<nCol; i++, apColl++){
        *apColl = multiSelectCollSeq(pParse, p, i);
        જો( 0==*apColl ){
          *apColl = pParse->db->pDfltColl;
        }
      }
      sqlite3VdbeAddOp4(v, OP_OpenEphemeral, iDistinct, nCol, 0,
                           (શૂન્ય*)p_ચાવી_Info, P4_KEYINFO);
    }નહીંતર{
      પ્રતિજ્ઞા( pParse->nErr>0 );
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
  માટે(pFirstRec=p; ALWAYS(pFirstRec!=0); pFirstRec=pFirstRec->pPrior){
    જો( pFirstRec->selFlags & SF_Aggregate ){
      sqlite3_Error_સંદેશ(pParse, "recursive aggregate queries not supported");
      જાઓ end_of_recursive_query;
    }
    pFirstRec->op = TK_ALL;
    જો( (pFirstRec->pPrior->selFlags & SF_Recursive)==0 ) તોડો;
  }

  /* Store the results of the setup-query in Queue. */
  pSetup = pFirstRec->pPrior;
  pSetup->pNext = 0;
  ExplainQueryPlan((pParse, 1, "SETUP"));
  rc = sqlite3Select(pParse, pSetup, &destQueue);
  pSetup->pNext = p;
  જો( rc ) જાઓ end_of_recursive_query;

  /* Find the next row in the Queue and output that row */
  addr_ટોચ = sqlite3VdbeAddOp2(v, OP_Rewind, iQueue, addrBreak); VdbeCoverage(v);

  /* Transfer the next row in Queue over to Current */
  sqlite3VdbeAddOp1(v, OP_NullRow, iCurrent); /* To reset column cache */
  જો( pOrderBy ){
    sqlite3VdbeAddOp3(v, OP_Column, iQueue, pOrderBy->n_પદાવલિ+1, regCurrent);
  }નહીંતર{
    sqlite3VdbeAddOp2(v, OP_RowData, iQueue, regCurrent);
  }
  sqlite3VdbeAddOp1(v, OP_Delete, iQueue);

  /* Output the single row in Current */
  addrCont = sqlite3VdbeMakeLabel(pParse);
  codeOffset(v, regOffset, addrCont);
  selectInnerLoop(pParse, p, iCurrent,
      0, 0, pDest, addrCont, addrBreak);
  જો( regLimit ){
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
  પ્રતિજ્ઞા( pFirstRec->pPrior==0 );
  pFirstRec->pPrior = pSetup;

  /* Keep running the loop until the Queue is empty */
  sqlite3VdbeGoto(v, addr_ટોચ);
  sqlite3VdbeResolveLabel(v, addrBreak);

end_of_recursive_query:
  sqlite3_પદાવલિ_List_Delete(pParse->db, p->pOrderBy);
  p->pOrderBy = pOrderBy;
  p->pLimit = pLimit;
  પાછા;
}
#endif /* SQLITE_OMIT_CTE */

/* Forward references */
સ્થિર પૂર્ણાંક multiSelectByMerge(
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
સ્થિર પૂર્ણાંક multi_Select_કિંમતો(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  પૂર્ણાંક nRow = 1;
  પૂર્ણાંક rc = 0;
  પૂર્ણાંક bShowAll = p->pLimit==0;
  પ્રતિજ્ઞા( p->selFlags & SF_Multi_કિંમત );
  કરો{
    પ્રતિજ્ઞા( p->selFlags & SF_કિંમતો );
    પ્રતિજ્ઞા( p->op==TK_ALL || (p->op==TK_SELECT && p->pPrior==0) );
    પ્રતિજ્ઞા( p->pNext==0 || p->pEList->n_પદાવલિ==p->pNext->pEList->n_પદાવલિ );
#ifndef SQLITE_OMIT_WINDOWFUNC
    જો( p->pWin ) પાછા -1;
#endif
    જો( p->pPrior==0 ) તોડો;
    પ્રતિજ્ઞા( p->pPrior->pNext==p );
    p = p->pPrior;
    nRow += bShowAll;
  }જ્યારે(1);
  ExplainQueryPlan((pParse, 0, "SCAN %d CONSTANT ROW%s", nRow,
                    nRow==1 ? "" : "S"));
  જ્યારે( p ){
    selectInnerLoop(pParse, p, -1, 0, 0, pDest, 1, 1);
    જો( !bShowAll ) તોડો;
    p->nSelectRow = nRow;
    p = p->pNext;
  }
  પાછા rc;
}

/*
** Return true if the SELECT statement which is known to be the recursive
** part of a recursive CTE still has its anchor terms attached.  If the
** anchor terms have already been removed, then return false.
*/
સ્થિર પૂર્ણાંક hasAnchor(Select *p){
  જ્યારે( p && (p->selFlags & SF_Recursive)!=0 ){ p = p->pPrior; }
  પાછા p!=0;
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
સ્થિર પૂર્ણાંક unionWithLimitOne(sqlite3 *db, Select *p){
  પૂર્ણાંક v;
  જો( p->op!=TK_UNION ) પાછા 0;
  જો( p->pLimit==0 ) પાછા 0;
  જો( p->pLimit->p_જમણું ) પાછા 0;  /* No OFFSET */
  v = 0;
  પ્રતિજ્ઞા( p->pLimit->p_ડાબું!=0 );
  જો( sqlite3_પદાવલિ_Is_Integer(p->pLimit->p_ડાબું, &v, 0, 0)==0 ) પાછા 0;
  જો( v!=1 ) પાછા 0;
  જો( OptimizationDisabled(db, SQLITE_UnionLimit) ) પાછા 0;
  પાછા 1;
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
સ્થિર પૂર્ણાંક multiSelect(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  પૂર્ણાંક rc = SQLITE_OK;   /* Success code from a subroutine */
  Select *pPrior;       /* Another SELECT immediately to our left */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest dest;      /* Alternative data destination */
  Select *pDelete = 0;  /* Chain of simple selects to delete */
  sqlite3 *db;          /* Database connection */

  /* Make sure there is no ORDER BY or LIMIT clause on prior SELECTs.  Only
  ** the last (right-most) SELECT in the series may have an ORDER BY or LIMIT.
  */
  પ્રતિજ્ઞા( p && p->pPrior );  /* Calling function guarantees this much */
  પ્રતિજ્ઞા( (p->selFlags & SF_Recursive)==0 || p->op==TK_ALL || p->op==TK_UNION );
  પ્રતિજ્ઞા( p->selFlags & SF_Compound );
  db = pParse->db;
  pPrior = p->pPrior;
  dest = *pDest;
  પ્રતિજ્ઞા( pPrior->pOrderBy==0 );
  પ્રતિજ્ઞા( pPrior->pLimit==0 );

  v = sqlite3GetVdbe(pParse);
  પ્રતિજ્ઞા( v!=0 );  /* The VDBE already created by calling function */

  /* Create the destination temporary table if necessary
  */
  જો( dest.eDest==SRT_EphemTab ){
    પ્રતિજ્ઞા( p->pEList );
    sqlite3VdbeAddOp2(v, OP_OpenEphemeral, dest.iSDParm, p->pEList->n_પદાવલિ);
    dest.eDest = SRT_Table;
  }

  /* Special handling for a compound-select that originates as a VALUES clause.
  */
  જો( p->selFlags & SF_Multi_કિંમત ){
    rc = multi_Select_કિંમતો(pParse, p, &dest);
    જો( rc>=0 ) જાઓ multi_select_end;
    rc = SQLITE_OK;
  }

  /* Make sure all SELECTs in the statement have the same number of elements
  ** in their result sets.
  */
  પ્રતિજ્ઞા( p->pEList && pPrior->pEList );
  પ્રતિજ્ઞા( p->pEList->n_પદાવલિ==pPrior->pEList->n_પદાવલિ );

#ifndef SQLITE_OMIT_CTE
  જો( (p->selFlags & SF_Recursive)!=0 && hasAnchor(p) ){
    generateWithRecursiveQuery(pParse, p, &dest);
  }નહીંતર
#endif
  જો( p->pOrderBy ){
    /* If the compound has an ORDER BY clause, then always use the merge
    ** algorithm. */
    પાછા multiSelectByMerge(pParse, p, pDest);
  }નહીંતર જો( p->op!=TK_ALL && !unionWithLimitOne(db,p) ){
    /* If the compound is EXCEPT, INTERSECT, or UNION (anything other than
    ** UNION ALL) then also always use the merge algorithm.  However, the
    ** multiSelectByMerge() routine requires that the compound have an
    ** ORDER BY clause, and it doesn't right now.  So invent one first. */
    પદાવલિ *pOne = sqlite3_પદાવલિ_Int32(db, 1);
    p->pOrderBy = sqlite3_પદાવલિ_List_Append(pParse, 0, pOne);
    જો( pParse->nErr ) જાઓ multi_select_end;
    પ્રતિજ્ઞા( p->pOrderBy!=0 );
    p->pOrderBy->a[0].u.x.iOrderByCol = 1;
    પાછા multiSelectByMerge(pParse, p, pDest);
  }નહીંતર{
    /* For a UNION ALL compound without ORDER BY, or for a UNION with a 
    ** LIMIT of exactly 1 and no OFFSET and no ORDER BY, simply run the left
    ** query, then run the right query */
    પૂર્ણાંક addr = 0;
    પૂર્ણાંક nLimit = 0;  /* Initialize to suppress harmless compiler warning */

#ifndef SQLITE_OMIT_EXPLAIN
    જો( pPrior->pPrior==0 ){
      ExplainQueryPlan((pParse, 1, "COMPOUND QUERY"));
      ExplainQueryPlan((pParse, 1, "LEFT-MOST SUBQUERY"));
    }
#endif
    પ્રતિજ્ઞા( !pPrior->pLimit );
    pPrior->iLimit = p->iLimit;
    pPrior->iOffset = p->iOffset;
    pPrior->pLimit = sqlite3_પદાવલિ_Dup(db, p->pLimit, 0);
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL left...\n"));
    rc = sqlite3Select(pParse, pPrior, &dest);
    sqlite3_પદાવલિ_Delete(db, pPrior->pLimit);
    pPrior->pLimit = 0;
    જો( rc ){
      જાઓ multi_select_end;
    }
    p->pPrior = 0;
    p->iLimit = pPrior->iLimit;
    p->iOffset = pPrior->iOffset;
    જો( p->iLimit ){
      addr = sqlite3VdbeAddOp1(v, OP_IfNot, p->iLimit); VdbeCoverage(v);
      VdbeComment((v, "Jump ahead if LIMIT reached"));
      જો( p->iOffset ){
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
    જો( p->pLimit
     && sqlite3_પદાવલિ_Is_Integer(p->pLimit->p_ડાબું, &nLimit, pParse, 0)
     && nLimit>0 && p->nSelectRow > sqlite3LogEst((u64)nLimit)
    ){
      p->nSelectRow = sqlite3LogEst((u64)nLimit);
    }
    જો( addr ){
      sqlite3VdbeJumpHere(v, addr);
    }
#ifndef SQLITE_OMIT_EXPLAIN
    જો( p->pNext==0 ){
      Explain_Query_Plan_કાઢો(pParse);
    }
#endif
  }

multi_select_end:
  pDest->iSdst = dest.iSdst;
  pDest->nSdst = dest.nSdst;
  pDest->iSDParm2 = dest.iSDParm2;
  જો( pDelete ){
    sqlite3ParserAddCleanup(pParse, sqlite3SelectDeleteGeneric, pDelete);
  }
  પાછા rc;
}
#endif /* SQLITE_OMIT_COMPOUND_SELECT */

/*
** Error message for when two or more terms of a compound select have different
** size result sets.
*/
શૂન્ય sqlite3_Select_Wrong_અંક_Terms_Error(Parse *pParse, Select *p){
  જો( p->selFlags & SF_કિંમતો ){
    sqlite3_Error_સંદેશ(pParse, "all VALUES must have the same number of terms");
  }નહીંતર{
    sqlite3_Error_સંદેશ(pParse, "SELECTs to the left and right of %s"
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
સ્થિર પૂર્ણાંક generateOutputSubroutine(
  Parse *pParse,          /* Parsing context */
  Select *p,              /* The SELECT statement */
  SelectDest *pIn,        /* Coroutine supplying data */
  SelectDest *pDest,      /* Where to send the data */
  પૂર્ણાંક regReturn,          /* The return address register */
  પૂર્ણાંક regPrev,            /* Previous result register.  No uniqueness if 0 */
  ચાવી_Info *p_ચાવી_Info,      /* For comparing with previous entry */
  પૂર્ણાંક iBreak              /* Jump here if we hit the LIMIT */
){
  Vdbe *v = pParse->pVdbe;
  પૂર્ણાંક iContinue;
  પૂર્ણાંક addr;

  પ્રતિજ્ઞા( pIn->eDest==SRT_Coroutine );

  addr = sqlite3VdbeCurrentAddr(v);
  iContinue = sqlite3VdbeMakeLabel(pParse);

  /* Suppress duplicates for UNION, EXCEPT, and INTERSECT
  */
  જો( regPrev ){
    પૂર્ણાંક addr1, addr2;
    addr1 = sqlite3VdbeAddOp1(v, OP_IfNot, regPrev); VdbeCoverage(v);
    addr2 = sqlite3VdbeAddOp4(v, OP_Compare, pIn->iSdst, regPrev+1, pIn->nSdst,
                              (અક્ષર*)sqlite3_ચાવી_Info_Ref(p_ચાવી_Info), P4_KEYINFO);
    sqlite3VdbeAddOp3(v, OP_Jump, addr2+2, iContinue, addr2+2); VdbeCoverage(v);
    sqlite3VdbeJumpHere(v, addr1);
    sqlite3VdbeAddOp3(v, OP_Copy, pIn->iSdst, regPrev+1, pIn->nSdst-1);
    sqlite3VdbeAddOp2(v, OP_Integer, 1, regPrev);
  }
  જો( pParse->db->mallocFailed ) પાછા 0;

  /* Suppress the first OFFSET entries if there is an OFFSET clause
  */
  codeOffset(v, p->iOffset, iContinue);

  પસંદગી( pDest->eDest ){
    /* Store the result as data using a unique key.
    */
    સ્થિતિ SRT_Fifo:
    સ્થિતિ SRT_DistFifo:
    સ્થિતિ SRT_Table:
    સ્થિતિ SRT_EphemTab: {
      પૂર્ણાંક r1 = sqlite3GetTempReg(pParse);
      પૂર્ણાંક r2 = sqlite3GetTempReg(pParse);
      પૂર્ણાંક iParm = pDest->iSDParm;
      testcase( pDest->eDest==SRT_Table );
      testcase( pDest->eDest==SRT_EphemTab );
      testcase( pDest->eDest==SRT_Fifo );
      testcase( pDest->eDest==SRT_DistFifo );
      sqlite3VdbeAddOp3(v, OP_MakeRecord, pIn->iSdst, pIn->nSdst, r1);
#જો !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      જો( pDest->eDest==SRT_Table && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#endif
#ifndef SQLITE_OMIT_CTE
      જો( pDest->eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index that is used to enforce uniqueness on the
        ** total result.  At this point, we are processing the setup portion
        ** of the recursive CTE using the merge algorithm, so the results are
        ** guaranteed to be unique anyhow.  But we still need to populate the
        ** (iParm+1) cursor for use by the subsequent recursive phase.
        */
        sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, iParm+1, r1,
                             pIn->iSdst, pIn->nSdst);
      }
#endif
      sqlite3VdbeAddOp2(v, OP_NewRowid, iParm, r2);
      sqlite3VdbeAddOp3(v, OP_ઉમેરો, iParm, r1, r2);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      sqlite3ReleaseTempReg(pParse, r2);
      sqlite3ReleaseTempReg(pParse, r1);
      તોડો;
    }

    /* If any row exist in the result set, record that fact and abort.
    */
    સ્થિતિ SRT_Exists: {
      sqlite3VdbeAddOp2(v, OP_Integer, 1, pDest->iSDParm);
      /* The LIMIT clause will terminate the loop for us */
      તોડો;
    }

#ifndef SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)".
    */
    સ્થિતિ SRT_Set: {
      પૂર્ણાંક r1;
      testcase( pIn->nSdst>1 );
      r1 = sqlite3GetTempReg(pParse);
      sqlite3VdbeAddOp4(v, OP_MakeRecord, pIn->iSdst, pIn->nSdst,
          r1, pDest->zAffSdst, pIn->nSdst);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, pDest->iSDParm, r1,
                           pIn->iSdst, pIn->nSdst);
      જો( pDest->iSDParm2>0 ){
        પ્રતિજ્ઞા( sqlite3CursorBloomable(pParse, pDest->iSDParm, pIn->nSdst) );
        sqlite3VdbeAddOp4Int(v, OP_FilterAdd, pDest->iSDParm2, 0,
                             pIn->iSdst, pIn->nSdst);
        ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
      }
      sqlite3ReleaseTempReg(pParse, r1);
      તોડો;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell and break out
    ** of the scan loop.  Note that the select might return multiple columns
    ** if it is the RHS of a row-value IN operator.
    */
    સ્થિતિ SRT_Mem: {
      testcase( pIn->nSdst>1 );
      sqlite3_પદાવલિ_Code_Move(pParse, pIn->iSdst, pDest->iSDParm, pIn->nSdst);
      /* The LIMIT clause will jump out of the loop for us */
      તોડો;
    }
#endif /* #ifndef SQLITE_OMIT_SUBQUERY */

    /* The results are stored in a sequence of registers
    ** starting at pDest->iSdst.  Then the co-routine yields.
    */
    સ્થિતિ SRT_Coroutine: {
      જો( pDest->iSdst==0 ){
        pDest->iSdst = sqlite3GetTempRange(pParse, pIn->nSdst);
        pDest->nSdst = pIn->nSdst;
      }
      sqlite3_પદાવલિ_Code_Move(pParse, pIn->iSdst, pDest->iSdst, pIn->nSdst);
      sqlite3VdbeAddOp1(v, OP_Yield, pDest->iSDParm);
      તોડો;
    }

#ifndef SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    સ્થિતિ SRT_DistQueue:
    સ્થિતિ SRT_Queue: {
      પૂર્ણાંક n_ચાવી;
      પૂર્ણાંક r1, r2, r3, ii;
      પદાવલિ_List *pSO;
      પૂર્ણાંક iParm = pDest->iSDParm;
      pSO = pDest->pOrderBy;
      પ્રતિજ્ઞા( pSO );
      n_ચાવી = pSO->n_પદાવલિ;
      r1 = sqlite3GetTempReg(pParse);
      r2 = sqlite3GetTempRange(pParse, n_ચાવી+2);
      r3 = r2+n_ચાવી+1;

      sqlite3VdbeAddOp3(v, OP_MakeRecord, pIn->iSdst, pIn->nSdst, r3);
      જો( pDest->eDest==SRT_DistQueue ){
        sqlite3VdbeAddOp2(v, OP_Idx_ઉમેરો, iParm+1, r3);
      }
      માટે(ii=0; ii<n_ચાવી; ii++){
        sqlite3VdbeAddOp2(v, OP_SCopy,
                          pIn->iSdst + pSO->a[ii].u.x.iOrderByCol - 1,
                          r2+ii);
      }
      sqlite3VdbeAddOp2(v, OP_Sequence, iParm, r2+n_ચાવી);
      sqlite3VdbeAddOp3(v, OP_MakeRecord, r2, n_ચાવી+2, r1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, iParm, r1, r2, n_ચાવી+2);
      sqlite3ReleaseTempReg(pParse, r1);
      sqlite3ReleaseTempRange(pParse, r2, n_ચાવી+2);
      તોડો;
    }
#endif /* SQLITE_OMIT_CTE */

    /* Ignore the output */
    સ્થિતિ SRT_Discard: {
      તોડો;
    }

    /* If none of the above, then the result destination must be
    ** SRT_Output.
    **
    ** For SRT_Output, results are stored in a sequence of registers. 
    ** Then the OP_ResultRow opcode is used to cause sqlite3_step() to
    ** return the next row of result.
    */
    મૂળભૂત: {
      પ્રતિજ્ઞા( pDest->eDest==SRT_Output );
      sqlite3VdbeAddOp2(v, OP_ResultRow, pIn->iSdst, pIn->nSdst);
      તોડો;
    }
  }

  /* Jump to the end of the loop if the LIMIT is reached.
  */
  જો( p->iLimit ){
    sqlite3VdbeAddOp2(v, OP_DecrJumpZero, p->iLimit, iBreak); VdbeCoverage(v);
  }

  /* Generate the subroutine return
  */
  sqlite3VdbeResolveLabel(v, iContinue);
  sqlite3VdbeAddOp1(v, OP_Return, regReturn);

  પાછા addr;
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
સ્થિર પૂર્ણાંક multiSelectByMerge(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  પૂર્ણાંક i, j;             /* Loop counters */
  Select *pPrior;       /* Another SELECT immediately to our left */
  Select *pSplit;       /* Left-most SELECT in the right-hand group */
  પૂર્ણાંક nSelect;          /* Number of SELECT statements in the compound */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest destA;     /* Destination for coroutine A */
  SelectDest destB;     /* Destination for coroutine B */
  પૂર્ણાંક regAddrA;         /* Address register for select-A coroutine */
  પૂર્ણાંક regAddrB;         /* Address register for select-B coroutine */
  પૂર્ણાંક addrSelectA;      /* Address of the select-A coroutine */
  પૂર્ણાંક addrSelectB;      /* Address of the select-B coroutine */
  પૂર્ણાંક regOutA;          /* Address register for the output-A subroutine */
  પૂર્ણાંક regOutB;          /* Address register for the output-B subroutine */
  પૂર્ણાંક addrOutA;         /* Address of the output-A subroutine */
  પૂર્ણાંક addrOutB = 0;     /* Address of the output-B subroutine */
  પૂર્ણાંક addrEofA;         /* Address of the select-A-exhausted subroutine */
  પૂર્ણાંક addrEofA_noB;     /* Alternate addrEofA if B is uninitialized */
  પૂર્ણાંક addrEofB;         /* Address of the select-B-exhausted subroutine */
  પૂર્ણાંક addrAltB;         /* Address of the A<B subroutine */
  પૂર્ણાંક addrAeqB;         /* Address of the A==B subroutine */
  પૂર્ણાંક addrAgtB;         /* Address of the A>B subroutine */
  પૂર્ણાંક regLimitA;        /* Limit register for select-A */
  પૂર્ણાંક regLimitB;        /* Limit register for select-A */
  પૂર્ણાંક regPrev;          /* A range of registers to hold previous output */
  પૂર્ણાંક savedLimit;       /* Saved value of p->iLimit */
  પૂર્ણાંક savedOffset;      /* Saved value of p->iOffset */
  પૂર્ણાંક labelCmpr;        /* Label for the start of the merge algorithm */
  પૂર્ણાંક labelEnd;         /* Label for the end of the overall SELECT stmt */
  પૂર્ણાંક addr1;            /* Jump instructions that get retargeted */
  પૂર્ણાંક op;               /* One of TK_ALL, TK_UNION, TK_EXCEPT, TK_INTERSECT */
  ચાવી_Info *p_ચાવી_Dup = 0; /* Comparison information for duplicate removal */
  ચાવી_Info *p_ચાવી_Merge;   /* Comparison information for merging rows */
  sqlite3 *db;          /* Database connection */
  પદાવલિ_List *pOrderBy;   /* The ORDER BY clause */
  પૂર્ણાંક nOrderBy;         /* Number of terms in the ORDER BY clause */
  u32 *aPermute;        /* Mapping from ORDER BY terms to result set columns */

  પ્રતિજ્ઞા( p->pOrderBy!=0 );
  પ્રતિજ્ઞા( p_ચાવી_Dup==0 ); /* "Managed" code needs this.  Ticket #3382. */
  db = pParse->db;
  v = pParse->pVdbe;
  પ્રતિજ્ઞા( v!=0 );       /* Already thrown the error if VDBE alloc failed */
  labelEnd = sqlite3VdbeMakeLabel(pParse);
  labelCmpr = sqlite3VdbeMakeLabel(pParse);


  /* Patch up the ORDER BY clause
  */
  op = p->op; 
  પ્રતિજ્ઞા( p->pPrior->pOrderBy==0 );
  pOrderBy = p->pOrderBy;
  પ્રતિજ્ઞા( pOrderBy );
  nOrderBy = pOrderBy->n_પદાવલિ;

  /* For operators other than UNION ALL we have to make sure that
  ** the ORDER BY clause covers every term of the result set.  Add
  ** terms to the ORDER BY clause as necessary.  Do the same for UNION ALL
  ** if any operator to the left is not UNION ALL, so that the ORDER BY
  ** carries the collation of every column into that sub-compound.
  */
  માટે(pPrior=p; pPrior && pPrior->op==TK_ALL; pPrior=pPrior->pPrior){}
  જો( pPrior && pPrior->pPrior ){
    માટે(i=1; db->mallocFailed==0 && i<=p->pEList->n_પદાવલિ; i++){
      સંરચના પદાવલિ_List_item *pItem;
      માટે(j=0, pItem=pOrderBy->a; j<nOrderBy; j++, pItem++){
        પ્રતિજ્ઞા( pItem!=0 );
        પ્રતિજ્ઞા( pItem->u.x.iOrderByCol>0 );
        જો( pItem->u.x.iOrderByCol==i ) તોડો;
      }
      જો( j==nOrderBy ){
        પદાવલિ *pNew = sqlite3_પદાવલિ_Int32(db, i);
        જો( pNew==0 ) પાછા SQLITE_NOMEM_BKPT;
        p->pOrderBy = pOrderBy = sqlite3_પદાવલિ_List_Append(pParse, pOrderBy, pNew);
        જો( pOrderBy ) pOrderBy->a[nOrderBy++].u.x.iOrderByCol = (u16)i;
      }
    }
  }

  /* Compute the comparison permutation and keyinfo that is used with
  ** the permutation to determine if the next row of results comes
  ** from selectA or selectB.  Also add literal collations to the
  ** ORDER BY clause terms so that when selectA and selectB are
  ** evaluated, they use the correct collation.
  */
  aPermute = sqlite3DbMallocRawNN(db, કદ(u32)*(nOrderBy + 1));
  જો( aPermute ){
    સંરચના પદાવલિ_List_item *pItem;
    પૂર્ણાંક bKeep = 0;
    aPermute[0] = nOrderBy;
    માટે(i=1, pItem=pOrderBy->a; i<=nOrderBy; i++, pItem++){
      પ્રતિજ્ઞા( pItem!=0 );
      પ્રતિજ્ઞા( pItem->u.x.iOrderByCol>0 );
      પ્રતિજ્ઞા( pItem->u.x.iOrderByCol<=p->pEList->n_પદાવલિ );
      aPermute[i] = pItem->u.x.iOrderByCol - 1;
      જો( aPermute[i]!=(u32)i-1 ) bKeep = 1;
    }
     જો( bKeep==0 ){
      sqlite3DbFreeNN(db, aPermute);
      aPermute = 0;
    }
  }
  p_ચાવી_Merge = multi_Select_By_Merge_ચાવી_Info(pParse, p, 1);

  /* Allocate a range of temporary registers and the KeyInfo needed
  ** for the logic that removes duplicate result rows when the
  ** operator is UNION, EXCEPT, or INTERSECT (but not UNION ALL).
  */
  જો( op==TK_ALL ){
    regPrev = 0;
  }નહીંતર{
    પૂર્ણાંક n_પદાવલિ = p->pEList->n_પદાવલિ;
    પ્રતિજ્ઞા( nOrderBy>=n_પદાવલિ || db->mallocFailed );
    regPrev = pParse->nMem+1;
    pParse->nMem += n_પદાવલિ+1;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, regPrev);
    p_ચાવી_Dup = sqlite3_ચાવી_Info_Alloc(db, n_પદાવલિ, 1);
    જો( p_ચાવી_Dup ){
      પ્રતિજ્ઞા( sqlite3_ચાવી_Info_Is_Writeable(p_ચાવી_Dup) );
      /* Use the ORDER BY collations, not multiSelectCollSeq(p), because
      ** p might be just one part of a larger compound, in which case
      ** the ORDER BY carries the COLLATE of the larger compound. */
      માટે(i=0; ALWAYS(p_ચાવી_Merge) && i<nOrderBy; i++){
        p_ચાવી_Dup->aColl[pOrderBy->a[i].u.x.iOrderByCol-1] = p_ચાવી_Merge->aColl[i];
      }
    }
  }

  /* Separate the left and the right query from one another
  */
  nSelect = 1;
  જો( (op==TK_ALL || op==TK_UNION)
   && OptimizationEnabled(db, SQLITE_BalancedMerge)
  ){
    માટે(pSplit=p; pSplit->pPrior!=0 && pSplit->op==op; pSplit=pSplit->pPrior){
      nSelect++;
      પ્રતિજ્ઞા( pSplit->pPrior->pNext==pSplit );
    }
  }
  જો( nSelect<=3 ){
    pSplit = p;
  }નહીંતર{
    pSplit = p;
    માટે(i=2; i<nSelect; i+=2){ pSplit = pSplit->pPrior; }
  }
  pPrior = pSplit->pPrior;
  પ્રતિજ્ઞા( pPrior!=0 );
  pSplit->pPrior = 0;
  pPrior->pNext = 0;
  પ્રતિજ્ઞા( p->pOrderBy == pOrderBy );
  પ્રતિજ્ઞા( pOrderBy!=0 || db->mallocFailed );
  pPrior->pOrderBy = sqlite3_પદાવલિ_List_Dup(pParse->db, pOrderBy, 0);
  sqlite3ResolveOrderGroupBy(pParse, p, p->pOrderBy, "ORDER");
  sqlite3ResolveOrderGroupBy(pParse, pPrior, pPrior->pOrderBy, "ORDER");

  /* Compute the limit registers */
  computeLimitRegisters(pParse, p, labelEnd);
  જો( p->iLimit && (op==TK_ALL || unionWithLimitOne(db,p)) ){
    regLimitA = ++pParse->nMem;
    regLimitB = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Copy, p->iOffset ? p->iOffset+1 : p->iLimit,
                                  regLimitA);
    sqlite3VdbeAddOp2(v, OP_Copy, regLimitA, regLimitB);
  }નહીંતર{
    regLimitA = regLimitB = 0;
  }
  sqlite3_પદાવલિ_Delete(db, p->pLimit);
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
                 regPrev, p_ચાવી_Dup, labelEnd);
 
  /* Generate a subroutine that outputs the current row of the B
  ** select as the next output row of the compound select.
  */
  જો( op==TK_ALL || op==TK_UNION ){
    VdbeNoopComment((v, "SUBR: out-B"));
    addrOutB = generateOutputSubroutine(pParse,
                 p, &destB, pDest, regOutB,
                 regPrev, p_ચાવી_Dup, labelEnd);
  }
  sqlite3_ચાવી_Info_Unref(p_ચાવી_Dup);

  /* Generate a subroutine to run when the results from select A
  ** are exhausted and only data in select B remains.
  */
  જો( op==TK_EXCEPT || op==TK_INTERSECT ){
    addrEofA_noB = addrEofA = labelEnd;
  }નહીંતર{ 
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
  જો( op==TK_INTERSECT ){
    addrEofB = addrEofA;
    જો( p->nSelectRow > pPrior->nSelectRow ) p->nSelectRow = pPrior->nSelectRow;
  }નહીંતર{ 
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
  જો( op==TK_ALL ){
    addrAeqB = addrAltB;
  }નહીંતર જો( op==TK_INTERSECT ){
    addrAeqB = addrAltB;
    addrAltB++;
  }નહીંતર{
    addrAeqB = addrAltB + 1;
  }

  /* Generate code to handle the case of A>B
  */
  addrAgtB = sqlite3VdbeCurrentAddr(v);
  જો( op==TK_ALL || op==TK_UNION ){
    sqlite3VdbeAddOp2(v, OP_Gosub, regOutB, addrOutB);
    VdbeComment((v, "out-B"));
    sqlite3VdbeAddOp2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, labelCmpr);
  }નહીંતર{
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
  જો( aPermute!=0 ){
    sqlite3VdbeAddOp4(v, OP_Permutation, 0, 0, 0, (અક્ષર*)aPermute, P4_INTARRAY);
  }
  sqlite3VdbeResolveLabel(v, labelCmpr);
  sqlite3VdbeAddOp4(v, OP_Compare, destA.iSdst, destB.iSdst, nOrderBy,
                         (અક્ષર*)p_ચાવી_Merge, P4_KEYINFO);
  જો( aPermute!=0 ){
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
  જો( pSplit->pPrior ){
    sqlite3ParserAddCleanup(pParse, sqlite3SelectDeleteGeneric, pSplit->pPrior);
  }
  pSplit->pPrior = pPrior;
  pPrior->pNext = pSplit;
  sqlite3_પદાવલિ_List_Delete(db, pPrior->pOrderBy);
  pPrior->pOrderBy = 0;

  /*** TBD:  Insert subroutine calls to close cursors on incomplete
  **** subqueries ****/
  Explain_Query_Plan_કાઢો(pParse);
  પાછા pParse->nErr!=0;
}
#endif

#જો !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)

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
પ્રકારવ્યાખ્યા સંરચના SubstContext {
  Parse *pParse;            /* The parsing context */
  પૂર્ણાંક iTable;               /* Replace references to this table */
  પૂર્ણાંક iNewTable;            /* New table number */
  પૂર્ણાંક isOuterJoin;          /* Add TK_IF_NULL_ROW opcodes on each replacement */
  પૂર્ણાંક nSelDepth;            /* Depth of sub-query recursion.  Top==1 */
  પદાવલિ_List *pEList;         /* Replacement expressions */
  પદાવલિ_List *pCList;         /* Collation sequences for replacement expr */
} SubstContext;

/* Forward Declarations */
સ્થિર શૂન્ય subst_પદાવલિ_List(SubstContext*, પદાવલિ_List*);
સ્થિર શૂન્ય substSelect(SubstContext*, Select*, પૂર્ણાંક);

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
સ્થિર પદાવલિ *subst_પદાવલિ(
  SubstContext *pSubst,  /* Description of the substitution */
  પદાવલિ *p_પદાવલિ            /* Expr in which substitution occurs */
){
  જો( p_પદાવલિ==0 ) પાછા 0;
  જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON|EP_InnerON)
   && p_પદાવલિ->w.iJoin==pSubst->iTable
  ){
    testcase( પદાવલિ_Has_Property(p_પદાવલિ, EP_InnerON) );
    p_પદાવલિ->w.iJoin = pSubst->iNewTable;
  }
  જો( p_પદાવલિ->op==TK_COLUMN
   && p_પદાવલિ->iTable==pSubst->iTable
   && !પદાવલિ_Has_Property(p_પદાવલિ, EP_FixedCol)
  ){
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
    જો( p_પદાવલિ->iColumn<0 ){
      p_પદાવલિ->op = TK_NULL;
    }નહીંતર
#endif
    {
      પદાવલિ *pNew;
      પૂર્ણાંક iColumn;
      પદાવલિ *pCopy;
      પદાવલિ ifNullRow;
      iColumn = p_પદાવલિ->iColumn;
      પ્રતિજ્ઞા( iColumn>=0 );
      પ્રતિજ્ઞા( pSubst->pEList!=0 && iColumn<pSubst->pEList->n_પદાવલિ );
      પ્રતિજ્ઞા( p_પદાવલિ->p_જમણું==0 );
      pCopy = pSubst->pEList->a[iColumn].p_પદાવલિ;
      જો( sqlite3_પદાવલિ_Is_Vector(pCopy) ){
        sqlite3_Vector_Error_સંદેશ(pSubst->pParse, pCopy);
      }નહીંતર{
        sqlite3 *db = pSubst->pParse->db;
        જો( pSubst->isOuterJoin
         && (pCopy->op!=TK_COLUMN || pCopy->iTable!=pSubst->iNewTable)
        ){
          memset(&ifNullRow, 0, કદ(ifNullRow));
          ifNullRow.op = TK_IF_NULL_ROW;
          ifNullRow.p_ડાબું = pCopy;
          ifNullRow.iTable = pSubst->iNewTable;
          ifNullRow.iColumn = -99;
          ifNullRow.flags = EP_IfNullRow;
          pCopy = &ifNullRow;
        }
        testcase( પદાવલિ_Has_Property(pCopy, EP_Subquery) );
        pNew = sqlite3_પદાવલિ_Dup(db, pCopy, 0);
        જો( db->mallocFailed ){
          sqlite3_પદાવલિ_Delete(db, pNew);
          પાછા p_પદાવલિ;
        }
        જો( pSubst->isOuterJoin ){
          પદાવલિ_Set_Property(pNew, EP_CanBeNull);
        }
        જો( pNew->op==TK_TRUEFALSE ){
          pNew->u.i_કિંમત = sqlite3_પદાવલિ_Truth_કિંમત(pNew);
          pNew->op = TK_INTEGER;
          પદાવલિ_Set_Property(pNew, EP_Int_કિંમત);
        }

        /* Ensure that the expression now has an implicit collation sequence,
        ** just as it did when it was a column of a view or sub-query. */
        {
          CollSeq *pNat = sqlite3_પદાવલિ_Coll_Seq(pSubst->pParse, pNew);
          CollSeq *pColl = sqlite3_પદાવલિ_Coll_Seq(pSubst->pParse,
                pSubst->pCList->a[iColumn].p_પદાવલિ
          );
          જો( pNat!=pColl || (pNew->op!=TK_COLUMN && pNew->op!=TK_COLLATE) ){
            pNew = sqlite3_પદાવલિ_Add_Collate_String(pSubst->pParse, pNew,
                (pColl ? pColl->zName : "BINARY")
            );
          }
        }
        પદાવલિ_Clear_Property(pNew, EP_Collate);
        જો( પદાવલિ_Has_Property(p_પદાવલિ,EP_OuterON|EP_InnerON) ){
          sqlite3_Set_Join_પદાવલિ(pNew, p_પદાવલિ->w.iJoin,
                             p_પદાવલિ->flags & (EP_OuterON|EP_InnerON));
        }
        sqlite3_પદાવલિ_Delete(db, p_પદાવલિ);
        p_પદાવલિ = pNew;
      }
    }
  }નહીંતર{
    જો( p_પદાવલિ->op==TK_IF_NULL_ROW && p_પદાવલિ->iTable==pSubst->iTable ){
      p_પદાવલિ->iTable = pSubst->iNewTable;
    }
    જો( p_પદાવલિ->op==TK_AGG_FUNCTION && p_પદાવલિ->op2>=pSubst->nSelDepth ){
      p_પદાવલિ->op2--;
    }
    p_પદાવલિ->p_ડાબું = subst_પદાવલિ(pSubst, p_પદાવલિ->p_ડાબું);
    p_પદાવલિ->p_જમણું = subst_પદાવલિ(pSubst, p_પદાવલિ->p_જમણું);
    જો( પદાવલિ_Use_XSelect(p_પદાવલિ) ){
      substSelect(pSubst, p_પદાવલિ->x.pSelect, 1);
    }નહીંતર{
      subst_પદાવલિ_List(pSubst, p_પદાવલિ->x.pList);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) ){
      Window *pWin = p_પદાવલિ->y.pWin;
      pWin->pFilter = subst_પદાવલિ(pSubst, pWin->pFilter);
      subst_પદાવલિ_List(pSubst, pWin->pPartition);
      subst_પદાવલિ_List(pSubst, pWin->pOrderBy);
    }
#endif
  }
  પાછા p_પદાવલિ;
}
સ્થિર શૂન્ય subst_પદાવલિ_List(
  SubstContext *pSubst, /* Description of the substitution */
  પદાવલિ_List *pList       /* List to scan and in which to make substitutes */
){
  પૂર્ણાંક i;
  જો( pList==0 ) પાછા;
  માટે(i=0; i<pList->n_પદાવલિ; i++){
    pList->a[i].p_પદાવલિ = subst_પદાવલિ(pSubst, pList->a[i].p_પદાવલિ);
  }
}
સ્થિર શૂન્ય substSelect(
  SubstContext *pSubst, /* Description of the substitution */
  Select *p,            /* SELECT statement in which to make substitutions */
  પૂર્ણાંક doPrior           /* Do substitutes on p->pPrior too */
){
  SrcList *pSrc;
  SrcItem *pItem;
  પૂર્ણાંક i;
  જો( !p ) પાછા;
  pSubst->nSelDepth++;
  કરો{
    subst_પદાવલિ_List(pSubst, p->pEList);
    subst_પદાવલિ_List(pSubst, p->pGroupBy);
    subst_પદાવલિ_List(pSubst, p->pOrderBy);
    p->pHaving = subst_પદાવલિ(pSubst, p->pHaving);
    p->pWhere = subst_પદાવલિ(pSubst, p->pWhere);
    pSrc = p->pSrc;
    પ્રતિજ્ઞા( pSrc!=0 );
    માટે(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      જો( pItem->fg.isSubquery ){
        substSelect(pSubst, pItem->u4.pSubq->pSelect, 1);
      }
      જો( pItem->fg.isTabFunc ){
        subst_પદાવલિ_List(pSubst, pItem->u1.pFuncArg);
      }
    }
  }જ્યારે( doPrior && (p = p->pPrior)!=0 );
  pSubst->nSelDepth--;
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#જો !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
/*
** pSelect is a SELECT statement and pSrcItem is one item in the FROM
** clause of that SELECT.
**
** This routine scans the entire SELECT statement and recomputes the
** pSrcItem->colUsed mask.
*/
સ્થિર પૂર્ણાંક recompute_Columns_Used_પદાવલિ(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  SrcItem *pItem;
  જો( p_પદાવલિ->op!=TK_COLUMN ) પાછા WRC_Continue;
  pItem = pWalker->u.pSrcItem;
  જો( pItem->iCursor!=p_પદાવલિ->iTable ) પાછા WRC_Continue;
  જો( p_પદાવલિ->iColumn<0 ) પાછા WRC_Continue;
  pItem->colUsed |= sqlite3_પદાવલિ_Col_Used(p_પદાવલિ);
  પાછા WRC_Continue;
}
સ્થિર શૂન્ય recomputeColumnsUsed(
  Select *pSelect,                 /* The complete SELECT statement */
  SrcItem *pSrcItem                /* Which FROM clause item to recompute */
){
  Walker w;
  જો( NEVER(pSrcItem->pSTab==0) ) પાછા;
  memset(&w, 0, કદ(w));
  w.x_પદાવલિ_Callback = recompute_Columns_Used_પદાવલિ;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pSrcItem = pSrcItem;
  pSrcItem->colUsed = 0;
  sqlite3WalkSelect(&w, pSelect);
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#જો !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
સ્થિર શૂન્ય srclistRenumberCursors(
  Parse *pParse,                  /* Parse context */
  પૂર્ણાંક *aCsrMap,                   /* Array to store cursor mappings in */
  SrcList *pSrc,                  /* FROM clause to renumber */
  પૂર્ણાંક iExcept                     /* FROM clause item to skip */
){
  પૂર્ણાંક i;
  SrcItem *pItem;
  માટે(i=0, pItem=pSrc->a; i<pSrc->nSrc; i++, pItem++){
    જો( i!=iExcept ){
      Select *p;
      પ્રતિજ્ઞા( pItem->iCursor < aCsrMap[0] );
      જો( !pItem->fg.isRecursive || aCsrMap[pItem->iCursor+1]==0 ){
        aCsrMap[pItem->iCursor+1] = pParse->nTab++;
      }
      pItem->iCursor = aCsrMap[pItem->iCursor+1];
      જો( pItem->fg.isSubquery ){
        માટે(p=pItem->u4.pSubq->pSelect; p; p=p->pPrior){
          srclistRenumberCursors(pParse, aCsrMap, p->pSrc, -1);
        }
      }
    }
  }
}

/*
** *piCursor is a cursor number.  Change it if it needs to be mapped.
*/
સ્થિર શૂન્ય renumberCursorDoMapping(Walker *pWalker, પૂર્ણાંક *piCursor){
  પૂર્ણાંક *aCsrMap = pWalker->u.aiCol;
  પૂર્ણાંક iCsr = *piCursor;
  જો( iCsr < aCsrMap[0] && aCsrMap[iCsr+1]>0 ){
    *piCursor = aCsrMap[iCsr+1];
  }
}

/*
** Expression walker callback used by renumberCursors() to update
** Expr objects to match newly assigned cursor numbers.
*/
સ્થિર પૂર્ણાંક renumberCursorsCb(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  પૂર્ણાંક op = p_પદાવલિ->op;
  જો( op==TK_COLUMN || op==TK_IF_NULL_ROW ){
    renumberCursorDoMapping(pWalker, &p_પદાવલિ->iTable);
  }
  જો( પદાવલિ_Has_Property(p_પદાવલિ, (EP_OuterON|EP_InnerON)) ){
    testcase( પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON) );
    testcase( પદાવલિ_Has_Property(p_પદાવલિ, EP_InnerON) );
    renumberCursorDoMapping(pWalker, &p_પદાવલિ->w.iJoin);
  }
  પાછા WRC_Continue;
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
સ્થિર શૂન્ય renumberCursors(
  Parse *pParse,                  /* Parse context */
  Select *p,                      /* Select to renumber cursors within */
  પૂર્ણાંક iExcept,                    /* FROM clause item to skip */
  પૂર્ણાંક *aCsrMap                    /* Working space */
){
  Walker w;
  srclistRenumberCursors(pParse, aCsrMap, p->pSrc, iExcept);
  memset(&w, 0, કદ(w));
  w.u.aiCol = aCsrMap;
  w.x_પદાવલિ_Callback = renumberCursorsCb;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  sqlite3WalkSelect(&w, p);
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** If pSel is not part of a compound SELECT, return a pointer to its
** expression list. Otherwise, return a pointer to the expression list
** of the leftmost SELECT in the compound.
*/
સ્થિર પદાવલિ_List *findLeftmostExprlist(Select *pSel){
  જ્યારે( pSel->pPrior ){
    pSel = pSel->pPrior;
  }
  પાછા pSel->pEList;
}

/*
** Return true if any of the result-set columns in the compound query
** have incompatible affinities on one or more arms of the compound.
*/
સ્થિર પૂર્ણાંક compoundHasDifferentAffinities(Select *p){
  પૂર્ણાંક ii;
  પદાવલિ_List *pList;
  પ્રતિજ્ઞા( p!=0 );
  પ્રતિજ્ઞા( p->pEList!=0 );
  પ્રતિજ્ઞા( p->pPrior!=0 );
  pList = p->pEList;
  માટે(ii=0; ii<pList->n_પદાવલિ; ii++){
    અક્ષર aff;
    Select *pSub1;
    પ્રતિજ્ઞા( pList->a[ii].p_પદાવલિ!=0 );
    aff = sqlite3_પદાવલિ_Affinity(pList->a[ii].p_પદાવલિ);
    માટે(pSub1=p->pPrior; pSub1; pSub1=pSub1->pPrior){
      પ્રતિજ્ઞા( pSub1->pEList!=0 );
      પ્રતિજ્ઞા( pSub1->pEList->n_પદાવલિ>ii );
      પ્રતિજ્ઞા( pSub1->pEList->a[ii].p_પદાવલિ!=0 );
      જો( sqlite3_પદાવલિ_Affinity(pSub1->pEList->a[ii].p_પદાવલિ)!=aff ){
        પાછા 1;
      }
    }
  }
  પાછા 0;
}

#જો !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
સ્થિર પૂર્ણાંક flattenSubquery(
  Parse *pParse,       /* Parsing context */
  Select *p,           /* The parent or outer SELECT statement */
  પૂર્ણાંક iFrom,           /* Index in p->pSrc->a[] of the inner subquery */
  પૂર્ણાંક isAgg            /* True if outer SELECT uses aggregate functions */
){
  અચલ અક્ષર *zSavedAuthContext = pParse->zAuthContext;
  Select *pParent;    /* Current UNION ALL term of the other query */
  Select *pSub;       /* The inner query or "subquery" */
  Select *pSub1;      /* Pointer to the rightmost select in sub-query */
  SrcList *pSrc;      /* The FROM clause of the outer query */
  SrcList *pSubSrc;   /* The FROM clause of the subquery */
  પૂર્ણાંક iParent;        /* VDBE cursor number of the pSub result set temp table */
  પૂર્ણાંક iNewParent = -1;/* Replacement table for iParent */
  પૂર્ણાંક isOuterJoin = 0; /* True if pSub is the right side of a LEFT JOIN */   
  પૂર્ણાંક i;              /* Loop counter */
  પદાવલિ *pWhere;                    /* The WHERE clause */
  SrcItem *pSubitem;               /* The subquery */
  sqlite3 *db = pParse->db;
  Walker w;                        /* Walker to persist agginfo data */
  પૂર્ણાંક *aCsrMap = 0;

  /* Check to see if flattening is permitted.  Return 0 if not.
  */
  પ્રતિજ્ઞા( p!=0 );
  પ્રતિજ્ઞા( p->pPrior==0 );
  જો( OptimizationDisabled(db, SQLITE_QueryFlattener) ) પાછા 0;
  pSrc = p->pSrc;
  પ્રતિજ્ઞા( pSrc && iFrom>=0 && iFrom<pSrc->nSrc );
  pSubitem = &pSrc->a[iFrom];
  iParent = pSubitem->iCursor;
  પ્રતિજ્ઞા( pSubitem->fg.isSubquery );
  pSub = pSubitem->u4.pSubq->pSelect;
  પ્રતિજ્ઞા( pSub!=0 );

#ifndef SQLITE_OMIT_WINDOWFUNC
  જો( p->pWin || pSub->pWin ) પાછા 0;                  /* Restriction (25) */
#endif

  pSubSrc = pSub->pSrc;
  પ્રતિજ્ઞા( pSubSrc );
  /* Prior to version 3.1.2, when LIMIT and OFFSET had to be simple constants,
  ** not arbitrary expressions, we allowed some combining of LIMIT and OFFSET
  ** because they could be computed at compile-time.  But when LIMIT and OFFSET
  ** became arbitrary expressions, we were forced to add restrictions (13)
  ** and (14). */
  જો( pSub->pLimit && p->pLimit ) પાછા 0;              /* Restriction (13) */
  જો( pSub->pLimit && pSub->pLimit->p_જમણું ) પાછા 0;   /* Restriction (14) */
  જો( (p->selFlags & SF_Compound)!=0 && pSub->pLimit ){
    પાછા 0;                                            /* Restriction (15) */
  }
  જો( pSubSrc->nSrc==0 ) પાછા 0;                       /* Restriction (7)  */
  જો( pSub->selFlags & SF_Distinct ) પાછા 0;           /* Restriction (4)  */
  જો( pSub->pLimit && (pSrc->nSrc>1 || isAgg) ){
     પાછા 0;         /* Restrictions (8)(9) */
  }
  જો( p->pOrderBy && pSub->pOrderBy ){
     પાછા 0;                                           /* Restriction (11) */
  }
  જો( isAgg && pSub->pOrderBy ) પાછા 0;                /* Restriction (16) */
  જો( pSub->pLimit && p->pWhere ) પાછા 0;              /* Restriction (19) */
  જો( pSub->pLimit && (p->selFlags & SF_Distinct)!=0 ){
     પાછા 0;         /* Restriction (21) */
  }
  જો( pSub->selFlags & (SF_Recursive) ){
    પાછા 0; /* Restrictions (22) */
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
  જો( (pSubitem->fg.jointype & (JT_OUTER|JT_LTORJ))!=0 ){
    જો( pSubSrc->nSrc>1                        /* (3a) */
     /**** || IsVirtual(pSubSrc->a[0].pSTab)      (3b)-omitted */
     || (p->selFlags & SF_Distinct)!=0         /* (3d) */
     || (pSubitem->fg.jointype & JT_જમણું)!=0  /* (26) */
    ){
      પાછા 0;
    }
    isOuterJoin = 1;
  }

  પ્રતિજ્ઞા( pSubSrc->nSrc>0 );  /* True by restriction (7) */
  જો( iFrom>0 && (pSubSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
    પાછા 0;   /* Restriction (27a) */
  }

  /* Condition (28) is blocked by the caller */
  પ્રતિજ્ઞા( !pSubitem->fg.isCte || pSubitem->u2.pCteUse->eM10d!=M10d_Yes );

  /* Restriction (17): If the sub-query is a compound SELECT, then it must
  ** use only the UNION ALL operator. And none of the simple select queries
  ** that make up the compound SELECT are allowed to be aggregate or distinct
  ** queries.
  */
  જો( pSub->pPrior ){
    પૂર્ણાંક ii;
    જો( pSub->pOrderBy ){
      પાછા 0;  /* Restriction (20) */
    }
    જો( isAgg || (p->selFlags & SF_Distinct)!=0 || isOuterJoin>0 ){
      પાછા 0; /* (17d1), (17d2), or (17f) */
    }
    માટે(pSub1=pSub; pSub1; pSub1=pSub1->pPrior){
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct );
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Aggregate );
      પ્રતિજ્ઞા( pSub->pSrc!=0 );
      પ્રતિજ્ઞા( (pSub->selFlags & SF_Recursive)==0 );
      પ્રતિજ્ઞા( pSub->pEList->n_પદાવલિ==pSub1->pEList->n_પદાવલિ );
      જો( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))!=0    /* (17b) */
       || (pSub1->pPrior && pSub1->op!=TK_ALL)                 /* (17a) */
       || pSub1->pSrc->nSrc<1                                  /* (17c) */
#ifndef SQLITE_OMIT_WINDOWFUNC
       || pSub1->pWin                                          /* (17e) */
#endif
      ){
        પાછા 0;
      }
      જો( iFrom>0 && (pSub1->pSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
        /* Without this restriction, the JT_LTORJ flag would end up being
        ** omitted on left-hand tables of the right join that is being
        ** flattened. */
        પાછા 0;   /* Restrictions (17g), (27b) */
      }
      testcase( pSub1->pSrc->nSrc>1 );
    }

    /* Restriction (18). */
    જો( p->pOrderBy ){
      માટે(ii=0; ii<p->pOrderBy->n_પદાવલિ; ii++){
        જો( p->pOrderBy->a[ii].u.x.iOrderByCol==0 ) પાછા 0;
      }
    }

    /* Restriction (23) */
    જો( (p->selFlags & SF_Recursive) ) પાછા 0;

    /* Restriction (17h) */
    જો( compoundHasDifferentAffinities(pSub) ) પાછા 0;

    જો( pSrc->nSrc>1 ){
      જો( pParse->nSelect>500 ) પાછા 0;
      જો( OptimizationDisabled(db, SQLITE_FlttnUnionAll) ) પાછા 0;
      aCsrMap = sqlite3DbMallocZero(db, ((i64)pParse->nTab+1)*કદ(પૂર્ણાંક));
      જો( aCsrMap ) aCsrMap[0] = pParse->nTab;
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
  
  જો( ALWAYS(pSubitem->fg.isSubquery) ){
    pSub1 = sqlite3SubqueryDetach(db, pSubitem);
  }નહીંતર{
    pSub1 = 0;
  }
  પ્રતિજ્ઞા( pSubitem->fg.isSubquery==0 );
  પ્રતિજ્ઞા( pSubitem->fg.fixedSchema==0 );
  sqlite3DbFree(db, pSubitem->zName);
  sqlite3DbFree(db, pSubitem->zAlias);
  pSubitem->zName = 0;
  pSubitem->zAlias = 0;
  પ્રતિજ્ઞા( pSubitem->fg.isUsing!=0 || pSubitem->u3.pOn==0 );

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
  માટે(pSub=pSub->pPrior; pSub; pSub=pSub->pPrior){
    Select *pNew;
    પદાવલિ_List *pOrderBy = p->pOrderBy;
    પદાવલિ *pLimit = p->pLimit;
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
    જો( pNew==0 ){
      p->pPrior = pPrior;
    }નહીંતર{
      pNew->selId = ++pParse->nSelect;
      જો( aCsrMap && ALWAYS(db->mallocFailed==0) ){
        renumberCursors(pParse, pNew, iFrom, aCsrMap);
      }
      pNew->pPrior = pPrior;
      જો( pPrior ) pPrior->pNext = pNew;
      pNew->pNext = p;
      p->pPrior = pNew;
      TREETRACE(0x4,pParse,p,("compound-subquery flattener"
                              " creates %u as peer\n",pNew->selId));
    }
    પ્રતિજ્ઞા( pSubitem->fg.isSubquery==0 );
  }
  sqlite3DbFree(db, aCsrMap);
  જો( db->mallocFailed ){
    પ્રતિજ્ઞા( pSubitem->fg.fixedSchema==0 );
    પ્રતિજ્ઞા( pSubitem->fg.isSubquery==0 );
    પ્રતિજ્ઞા( pSubitem->u4.zDatabase==0 );
    sqlite3SrcItemAttachSubquery(pParse, pSubitem, pSub1, 0);
    પાછા 1;
  }

  /* Defer deleting the Table object associated with the
  ** subquery until code generation is
  ** complete, since there may still exist Expr.pTab entries that
  ** refer to the subquery even after flattening.  Ticket #3346.
  **
  ** pSubitem->pSTab is always non-NULL by test restrictions and tests above.
  */
  જો( ALWAYS(pSubitem->pSTab!=0) ){
    Table *pTabToDel = pSubitem->pSTab;
    જો( pTabToDel->nTabRef==1 ){
      Parse *pToplevel = sqlite3ParseToplevel(pParse);
      sqlite3ParserAddCleanup(pToplevel, sqlite3DeleteTableGeneric, pTabToDel);
      testcase( pToplevel->earlyCleanup );
    }નહીંતર{
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
  માટે(pParent=p; pParent; pParent=pParent->pPrior, pSub=pSub->pPrior){
    પૂર્ણાંક nSubSrc;
    u8 jointype = pParent->pSrc->a[iFrom].fg.jointype;
    પ્રતિજ્ઞા( pSub!=0 );
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
    જો( nSubSrc>1 ){
      pSrc = sqlite3SrcListEnlarge(pParse, pSrc, nSubSrc-1,iFrom+1);
      જો( pSrc==0 ) તોડો;
      pParent->pSrc = pSrc;
    }

    /* Transfer the FROM clause terms from the subquery into the
    ** outer query.
    */
    iNewParent = pSubSrc->a[0].iCursor;
    માટે(i=0; i<nSubSrc; i++){
      SrcItem *pItem = &pSrc->a[i+iFrom];
      પ્રતિજ્ઞા( pItem->fg.isTabFunc==0 );
      પ્રતિજ્ઞા( pItem->fg.isSubquery
           || pItem->fg.fixedSchema
           || pItem->u4.zDatabase==0 );
      જો( pItem->fg.isUsing ) sqlite3IdListDelete(db, pItem->u3.pUsing);
      *pItem = pSubSrc->a[i];
      pItem->fg.jointype |= (jointype & JT_LTORJ);
      memset(&pSubSrc->a[i], 0, કદ(pSubSrc->a[i]));
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
    જો( pSub->pOrderBy ){
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
      પદાવલિ_List *pOrderBy = pSub->pOrderBy;
      માટે(i=0; i<pOrderBy->n_પદાવલિ; i++){
        pOrderBy->a[i].u.x.iOrderByCol = 0;
      }
      પ્રતિજ્ઞા( pParent->pOrderBy==0 );
      pParent->pOrderBy = pOrderBy;
      pSub->pOrderBy = 0;
    }
    pWhere = pSub->pWhere;
    pSub->pWhere = 0;
    જો( isOuterJoin>0 ){
      પ્રતિજ્ઞા( pSubSrc->nSrc==1 );
      sqlite3_Set_Join_પદાવલિ(pWhere, iNewParent, EP_OuterON);
    }
    જો( pWhere ){
      જો( pParent->pWhere ){
        pParent->pWhere = sqlite3PExpr(pParse, TK_AND, pWhere, pParent->pWhere);
      }નહીંતર{
        pParent->pWhere = pWhere;
      }
    }
    જો( db->mallocFailed==0 ){
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
    પ્રતિજ્ઞા( (pSub->selFlags & SF_Distinct)==0 ); /* restriction (17b) */
 
    /*
    ** SELECT ... FROM (SELECT ... LIMIT a OFFSET b) LIMIT x OFFSET y;
    **
    ** One is tempted to try to add a and b to combine the limits.  But this
    ** does not work if either limit is negative.
    */
    જો( pSub->pLimit ){
      pParent->pLimit = pSub->pLimit;
      pSub->pLimit = 0;
    }

    /* Recompute the SrcItem.colUsed masks for the flattened
    ** tables. */
    માટે(i=0; i<nSubSrc; i++){
      recomputeColumnsUsed(pParent, &pSrc->a[i+iFrom]);
    }
  }

  /* Finally, delete what is left of the subquery and return success.
  */
  sqlite3AggInfoPersistWalkerInit(&w, pParse);
  sqlite3WalkSelect(&w,pSub1);
  sqlite3SelectDelete(db, pSub1);

#જો TREETRACE_ENABLED
  જો( sqlite3_વૃક્ષ_Trace & 0x4 ){
    TREETRACE(0x4,pParse,p,("After flattening:\n"));
    sqlite3_વૃક્ષ_View_Select(0, p, 0);
  }
#endif

  પાછા 1;
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** A structure to keep track of all of the column values that are fixed to
** a known value due to WHERE clause constraints of the form COLUMN=VALUE.
*/
પ્રકારવ્યાખ્યા સંરચના WhereConst WhereConst;
સંરચના WhereConst {
  Parse *pParse;   /* Parsing context */
  u8 *pOomFault;   /* Pointer to pParse->db->mallocFailed */
  પૂર્ણાંક nConst;      /* Number for COLUMN=CONSTANT terms */
  પૂર્ણાંક nChng;       /* Number of times a constant is propagated */
  પૂર્ણાંક bHasAffBlob; /* At least one column in apExpr[] as affinity BLOB */
  u32 mExcludeOn;  /* Which ON expressions to exclude from considertion.
                   ** Either EP_OuterON or EP_InnerON|EP_OuterON */
  પદાવલિ **ap_પદાવલિ;   /* [i*2] is COLUMN and [i*2+1] is VALUE */
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
સ્થિર શૂન્ય const_ઉમેરો(
  WhereConst *pConst,  /* The WhereConst into which we are inserting */
  પદાવલિ *pColumn,       /* The COLUMN part of the constraint */
  પદાવલિ *p_કિંમત,        /* The VALUE part of the constraint */
  પદાવલિ *p_પદાવલિ          /* Overall expression: COLUMN=VALUE or VALUE=COLUMN */
){
  પૂર્ણાંક i;
  પ્રતિજ્ઞા( pColumn->op==TK_COLUMN );
  પ્રતિજ્ઞા( sqlite3_પદાવલિ_Is_Constant(pConst->pParse, p_કિંમત) );
  જો( પદાવલિ_Has_Property(pColumn, EP_FixedCol) ) પાછા;
  જો( sqlite3_પદાવલિ_Affinity(p_કિંમત)!=0 ) પાછા;
  જો( !sqlite3_Is_દ્વિઅંકી(sqlite3_પદાવલિ_Compare_Coll_Seq(pConst->pParse,p_પદાવલિ)) ){
    પાછા;
  }
  જો( sqlite3_પદાવલિ_Can_Return_Subtype(pConst->pParse, p_કિંમત) ) પાછા;

  /* 2018-10-25 ticket [cf5ed20f]
  ** Make sure the same pColumn is not inserted more than once */
  માટે(i=0; i<pConst->nConst; i++){
    અચલ પદાવલિ *pE2 = pConst->ap_પદાવલિ[i*2];
    પ્રતિજ્ઞા( pE2->op==TK_COLUMN );
    જો( pE2->iTable==pColumn->iTable
     && pE2->iColumn==pColumn->iColumn
    ){
      પાછા;  /* Already present.  Return without doing anything. */
    }
  }
  પ્રતિજ્ઞા( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  જો( sqlite3_પદાવલિ_Affinity(pColumn)<=SQLITE_AFF_BLOB ){
    pConst->bHasAffBlob = 1;
  }

  pConst->nConst++;
  pConst->ap_પદાવલિ = sqlite3DbReallocOrFree(pConst->pParse->db, pConst->ap_પદાવલિ,
                         pConst->nConst*2*કદ(પદાવલિ*));
  જો( pConst->ap_પદાવલિ==0 ){
    pConst->nConst = 0;
  }નહીંતર{
    pConst->ap_પદાવલિ[pConst->nConst*2-2] = pColumn;
    pConst->ap_પદાવલિ[pConst->nConst*2-1] = p_કિંમત;
  }
}

/*
** Find all terms of COLUMN=VALUE or VALUE=COLUMN in pExpr where VALUE
** is a constant expression and where the term must be true because it
** is part of the AND-connected terms of the expression.  For each term
** found, add it to the pConst structure.
*/
સ્થિર શૂન્ય findConstInWhere(WhereConst *pConst, પદાવલિ *p_પદાવલિ){
  પદાવલિ *p_જમણું, *p_ડાબું;
  જો( NEVER(p_પદાવલિ==0) ) પાછા;
  જો( પદાવલિ_Has_Property(p_પદાવલિ, pConst->mExcludeOn) ){
    testcase( પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON) );
    testcase( પદાવલિ_Has_Property(p_પદાવલિ, EP_InnerON) );
    પાછા;
  }
  જો( p_પદાવલિ->op==TK_AND ){
    findConstInWhere(pConst, p_પદાવલિ->p_જમણું);
    findConstInWhere(pConst, p_પદાવલિ->p_ડાબું);
    પાછા;
  }
  જો( p_પદાવલિ->op!=TK_EQ ) પાછા;
  p_જમણું = p_પદાવલિ->p_જમણું;
  p_ડાબું = p_પદાવલિ->p_ડાબું;
  પ્રતિજ્ઞા( p_જમણું!=0 );
  પ્રતિજ્ઞા( p_ડાબું!=0 );
  જો( p_જમણું->op==TK_COLUMN && sqlite3_પદાવલિ_Is_Constant(pConst->pParse, p_ડાબું) ){
    const_ઉમેરો(pConst,p_જમણું,p_ડાબું,p_પદાવલિ);
  }
  જો( p_ડાબું->op==TK_COLUMN && sqlite3_પદાવલિ_Is_Constant(pConst->pParse, p_જમણું) ){
    const_ઉમેરો(pConst,p_ડાબું,p_જમણું,p_પદાવલિ);
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
સ્થિર પૂર્ણાંક propagate_Constant_પદાવલિ_Rewrite_One(
  WhereConst *pConst,
  પદાવલિ *p_પદાવલિ,
  પૂર્ણાંક bIgnoreAffBlob
){
  પૂર્ણાંક i;
  જો( pConst->pOomFault[0] ) પાછા WRC_Prune;
  જો( p_પદાવલિ->op!=TK_COLUMN ) પાછા WRC_Continue;
  જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_FixedCol|pConst->mExcludeOn) ){
    testcase( પદાવલિ_Has_Property(p_પદાવલિ, EP_FixedCol) );
    testcase( પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON) );
    testcase( પદાવલિ_Has_Property(p_પદાવલિ, EP_InnerON) );
    પાછા WRC_Continue;
  }
  માટે(i=0; i<pConst->nConst; i++){
    પદાવલિ *pColumn = pConst->ap_પદાવલિ[i*2];
    જો( pColumn==p_પદાવલિ ) ચાલુ;
    જો( pColumn->iTable!=p_પદાવલિ->iTable ) ચાલુ;
    જો( pColumn->iColumn!=p_પદાવલિ->iColumn ) ચાલુ;
    પ્રતિજ્ઞા( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
    જો( bIgnoreAffBlob && sqlite3_પદાવલિ_Affinity(pColumn)<=SQLITE_AFF_BLOB ){
      તોડો;
    }
    /* A match is found.  Add the EP_FixedCol property */
    pConst->nChng++;
    પદાવલિ_Clear_Property(p_પદાવલિ, EP_Leaf);
    પદાવલિ_Set_Property(p_પદાવલિ, EP_FixedCol);
    પ્રતિજ્ઞા( p_પદાવલિ->p_ડાબું==0 );
    p_પદાવલિ->p_ડાબું = sqlite3_પદાવલિ_Dup(pConst->pParse->db, pConst->ap_પદાવલિ[i*2+1], 0);
    જો( pConst->pParse->db->mallocFailed ) પાછા WRC_Prune;
    તોડો;
  }
  પાછા WRC_Prune;
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
સ્થિર પૂર્ણાંક propagate_Constant_પદાવલિ_Rewrite(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  WhereConst *pConst = pWalker->u.pConst;
  પ્રતિજ્ઞા( TK_GT==TK_EQ+1 );
  પ્રતિજ્ઞા( TK_LE==TK_EQ+2 );
  પ્રતિજ્ઞા( TK_LT==TK_EQ+3 );
  પ્રતિજ્ઞા( TK_GE==TK_EQ+4 );
  જો( pConst->bHasAffBlob ){
    જો( (p_પદાવલિ->op>=TK_EQ && p_પદાવલિ->op<=TK_GE)
     || p_પદાવલિ->op==TK_IS
    ){
      propagate_Constant_પદાવલિ_Rewrite_One(pConst, p_પદાવલિ->p_ડાબું, 0);
      જો( pConst->pOomFault[0] ) પાછા WRC_Prune;
      જો( sqlite3_પદાવલિ_Affinity(p_પદાવલિ->p_ડાબું)!=SQLITE_AFF_TEXT ){
        propagate_Constant_પદાવલિ_Rewrite_One(pConst, p_પદાવલિ->p_જમણું, 0);
      }
    }
  }
  પાછા propagate_Constant_પદાવલિ_Rewrite_One(pConst, p_પદાવલિ, pConst->bHasAffBlob);
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
સ્થિર પૂર્ણાંક propagateConstants(
  Parse *pParse,   /* The parsing context */
  Select *p        /* The query in which to propagate constants */
){
  WhereConst x;
  Walker w;
  પૂર્ણાંક nChng = 0;
  x.pParse = pParse;
  x.pOomFault = &pParse->db->mallocFailed;
  કરો{
    x.nConst = 0;
    x.nChng = 0;
    x.ap_પદાવલિ = 0;
    x.bHasAffBlob = 0;
    જો( ALWAYS(p->pSrc!=0)
     && p->pSrc->nSrc>0
     && (p->pSrc->a[0].fg.jointype & JT_LTORJ)!=0
    ){
      /* Do not propagate constants on any ON clause if there is a
      ** RIGHT JOIN anywhere in the query */
      x.mExcludeOn = EP_InnerON | EP_OuterON;
    }નહીંતર{
      /* Do not propagate constants through the ON clause of a LEFT JOIN */
      x.mExcludeOn = EP_OuterON;
    }
    findConstInWhere(&x, p->pWhere);
    જો( x.nConst ){
      memset(&w, 0, કદ(w));
      w.pParse = pParse;
      w.x_પદાવલિ_Callback = propagate_Constant_પદાવલિ_Rewrite;
      w.xSelectCallback = sqlite3SelectWalkNoop;
      w.xSelectCallback2 = 0;
      w.walkerDepth = 0;
      w.u.pConst = &x;
      sqlite3_Walk_પદાવલિ(&w, p->pWhere);
      sqlite3DbFree(x.pParse->db, x.ap_પદાવલિ);
      nChng += x.nChng;
    }
  }જ્યારે( x.nChng ); 
  પાછા nChng;
}

#જો !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
# જો !defined(SQLITE_OMIT_WINDOWFUNC)
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
સ્થિર પૂર્ણાંક ધકેલો_Down_Window_Check(Parse *pParse, Select *pSubq, પદાવલિ *p_પદાવલિ){
  પ્રતિજ્ઞા( pSubq->pWin->pPartition );
  પ્રતિજ્ઞા( (pSubq->selFlags & SF_MultiPart)==0 );
  પ્રતિજ્ઞા( pSubq->pPrior==0 );
  પાછા sqlite3_પદાવલિ_Is_Constant_Or_Group_By(pParse, p_પદાવલિ, pSubq->pWin->pPartition);
}
# endif /* SQLITE_OMIT_WINDOWFUNC */
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#જો !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
સ્થિર પૂર્ણાંક ધકેલો_Down_Where_Terms(
  Parse *pParse,        /* Parse context (for malloc() and error reporting) */
  Select *pSubq,        /* The subquery whose WHERE clause is to be augmented */
  પદાવલિ *pWhere,         /* The WHERE clause of the outer query */
  SrcList *pSrcList,    /* The complete from clause of the outer query */
  પૂર્ણાંક iSrc              /* Which FROM clause term to try to push into  */
){
  પદાવલિ *pNew;
  SrcItem *pSrc;        /* The subquery FROM term into which WHERE is pushed */
  પૂર્ણાંક nChng = 0;
  pSrc = &pSrcList->a[iSrc];
  જો( pWhere==0 ) પાછા 0;
  જો( pSubq->selFlags & (SF_Recursive|SF_MultiPart) ){
    પાછા 0;           /* restrictions (2) and (11) */
  }
  જો( pSrc->fg.jointype & (JT_LTORJ|JT_જમણું) ){
    પાછા 0;           /* restrictions (10) */
  }

  જો( pSubq->pPrior ){
    Select *pSel;
    પૂર્ણાંક notUnionAll = 0;
    માટે(pSel=pSubq; pSel; pSel=pSel->pPrior){
      u8 op = pSel->op;
      પ્રતિજ્ઞા( op==TK_ALL || op==TK_SELECT
           || op==TK_UNION || op==TK_INTERSECT || op==TK_EXCEPT );
      જો( op!=TK_ALL && op!=TK_SELECT ){
        notUnionAll = 1;
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      જો( pSel->pWin ) પાછા 0;    /* restriction (6b) */
#endif
    }
    જો( notUnionAll ){
      /* If any of the compound arms are connected using UNION, INTERSECT,
      ** or EXCEPT, then we must ensure that none of the columns use a
      ** non-BINARY collating sequence. */
      માટે(pSel=pSubq; pSel; pSel=pSel->pPrior){
        પૂર્ણાંક ii;
        અચલ પદાવલિ_List *pList = pSel->pEList;
        પ્રતિજ્ઞા( pList!=0 );
        માટે(ii=0; ii<pList->n_પદાવલિ; ii++){
          CollSeq *pColl = sqlite3_પદાવલિ_Coll_Seq(pParse, pList->a[ii].p_પદાવલિ);
          જો( !sqlite3_Is_દ્વિઅંકી(pColl) ){
            પાછા 0;  /* Restriction (8) */
          }
        }
      }
    }
  }નહીંતર{
#ifndef SQLITE_OMIT_WINDOWFUNC
    જો( pSubq->pWin && pSubq->pWin->pPartition==0 ) પાછા 0;
#endif
  }

#ifdef SQLITE_DEBUG
  /* Only the first term of a compound can have a WITH clause.  But make
  ** sure no other terms are marked SF_Recursive in case something changes
  ** in the future.
  */
  {
    Select *pX; 
    માટે(pX=pSubq; pX; pX=pX->pPrior){
      પ્રતિજ્ઞા( (pX->selFlags & (SF_Recursive))==0 );
    }
  }
#endif

  જો( pSubq->pLimit!=0 ){
    પાછા 0; /* restriction (3) */
  }
  જ્યારે( pWhere->op==TK_AND ){
    nChng += ધકેલો_Down_Where_Terms(pParse, pSubq, pWhere->p_જમણું, pSrcList, iSrc);
    pWhere = pWhere->p_ડાબું;
  }

#જો 0 /* These checks now done by sqlite3ExprIsSingleTableConstraint() */
  જો( પદાવલિ_Has_Property(pWhere, EP_OuterON|EP_InnerON) /* (9a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (9c) */
  ){
    પૂર્ણાંક jj;
    માટે(jj=0; jj<iSrc; jj++){
      જો( pWhere->w.iJoin==pSrcList->a[jj].iCursor ){
        /* If we reach this point, both (9a) and (9b) are satisfied.
        ** The following loop checks (9c):
        */
        માટે(jj++; jj<iSrc; jj++){
          જો( (pSrcList->a[jj].fg.jointype & JT_જમણું)!=0 ){
            પાછા 0;  /* restriction (9) */
          }
        }
      }
    }
  }
  જો( is_ડાબું_Join
   && (પદાવલિ_Has_Property(pWhere,EP_OuterON)==0
         || pWhere->w.iJoin!=iCursor)
  ){
    પાછા 0; /* restriction (4) */
  }
  જો( પદાવલિ_Has_Property(pWhere,EP_OuterON)
   && pWhere->w.iJoin!=iCursor
  ){
    પાછા 0; /* restriction (5) */
  }
#endif

#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
  જો( ViewCanHaveRowid && (pWhere->op==TK_ISNULL || pWhere->op==TK_NOTNULL) ){
    પદાવલિ *p_ડાબું = pWhere->p_ડાબું;
    જો( ALWAYS(p_ડાબું) 
     && p_ડાબું->op==TK_COLUMN
     && p_ડાબું->iColumn < 0
    ){
      પાછા 0;  /* Restriction (12) */
    }
  }
#endif

  જો( sqlite3_પદાવલિ_Is_Single_Table_Constraint(pWhere, pSrcList, iSrc, 1) ){
    nChng++;
    pSubq->selFlags |= SF_ધકેલો_Down;
    જ્યારે( pSubq ){
      SubstContext x;
      pNew = sqlite3_પદાવલિ_Dup(pParse->db, pWhere, 0);
      unset_Join_પદાવલિ(pNew, -1, 1);
      x.pParse = pParse;
      x.iTable = pSrc->iCursor;
      x.iNewTable = pSrc->iCursor;
      x.isOuterJoin = 0;
      x.nSelDepth = 0;
      x.pEList = pSubq->pEList;
      x.pCList = findLeftmostExprlist(pSubq);
      pNew = subst_પદાવલિ(&x, pNew);
      પ્રતિજ્ઞા( pNew!=0 || pParse->nErr!=0 );
      જો( pParse->nErr==0 && pNew->op==TK_IN && પદાવલિ_Use_XSelect(pNew) ){
        પ્રતિજ્ઞા( pNew->x.pSelect!=0 );
        pNew->x.pSelect->selFlags |= SF_ClonedRhsIn;
        પ્રતિજ્ઞા( pWhere!=0 );
        પ્રતિજ્ઞા( pWhere->op==TK_IN );
        પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(pWhere) );
        પ્રતિજ્ઞા( pWhere->x.pSelect!=0 );
        pWhere->x.pSelect->selFlags |= SF_ClonedRhsIn;
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      જો( pSubq->pWin && 0==ધકેલો_Down_Window_Check(pParse, pSubq, pNew) ){
        /* Restriction 6c has prevented push-down in this case */
        sqlite3_પદાવલિ_Delete(pParse->db, pNew);
        nChng--;
        તોડો;
      }
#endif
      જો( pSubq->selFlags & SF_Aggregate ){
        pSubq->pHaving = sqlite3_પદાવલિ_And(pParse, pSubq->pHaving, pNew);
      }નહીંતર{
        pSubq->pWhere = sqlite3_પદાવલિ_And(pParse, pSubq->pWhere, pNew);
      }
      pSubq = pSubq->pPrior;
    }
  }
  પાછા nChng;
}
#endif /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** Check to see if a subquery contains result-set columns that are
** never used.  If it does, change the value of those result-set columns
** to NULL so that they do not cause unnecessary work to compute.
**
** Return the number of column that were changed to NULL.
*/
સ્થિર પૂર્ણાંક disableUnusedSubqueryResultColumns(SrcItem *pItem){
  પૂર્ણાંક nCol;
  Select *pSub;      /* The subquery to be simplified */
  Select *pX;        /* For looping over compound elements of pSub */
  Table *pTab;       /* The table that describes the subquery */
  પૂર્ણાંક j;             /* Column number */
  પૂર્ણાંક nChng = 0;     /* Number of columns converted to NULL */
  Bitmask colUsed;   /* Columns that may not be NULLed out */

  પ્રતિજ્ઞા( pItem!=0 );
  જો( pItem->fg.isCorrelated || pItem->fg.isCte ){
    પાછા 0;
  }
  પ્રતિજ્ઞા( pItem->pSTab!=0 );
  pTab = pItem->pSTab;
  પ્રતિજ્ઞા( pItem->fg.isSubquery );
  pSub = pItem->u4.pSubq->pSelect;
  પ્રતિજ્ઞા( pSub->pEList->n_પદાવલિ==pTab->nCol );
  માટે(pX=pSub; pX; pX=pX->pPrior){
    જો( (pX->selFlags & (SF_Distinct|SF_Aggregate))!=0 ){
      testcase( pX->selFlags & SF_Distinct );
      testcase( pX->selFlags & SF_Aggregate );
      પાછા 0;
    }
    જો( pX->pPrior && pX->op!=TK_ALL ){
      /* This optimization does not work for compound subqueries that
      ** use UNION, INTERSECT, or EXCEPT.  Only UNION ALL is allowed. */
      પાછા 0;
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    જો( pX->pWin ){
      /* This optimization does not work for subqueries that use window
      ** functions. */
      પાછા 0;
    }
#endif
  }
  colUsed = pItem->colUsed;
  જો( pSub->pOrderBy ){
    પદાવલિ_List *pList = pSub->pOrderBy;
    માટે(j=0; j<pList->n_પદાવલિ; j++){
      u16 iCol = pList->a[j].u.x.iOrderByCol;
      જો( iCol>0 ){
        iCol--;
        colUsed |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
      }
    }
  }
  nCol = pTab->nCol;
  માટે(j=0; j<nCol; j++){
    Bitmask m = j<BMS-1 ? MASKBIT(j) : TOPBIT;
    જો( (m & colUsed)!=0 ) ચાલુ;
    માટે(pX=pSub; pX; pX=pX->pPrior) {
      પદાવલિ *pY = pX->pEList->a[j].p_પદાવલિ;
      જો( pY->op==TK_NULL ) ચાલુ;
      pY->op = TK_NULL;
      પદાવલિ_Clear_Property(pY, EP_Skip|EP_Unlikely);
      pX->selFlags |= SF_ધકેલો_Down;
      nChng++;
    }
  }
  પાછા nChng;
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
સ્થિર u8 minMaxQuery(sqlite3 *db, પદાવલિ *pFunc, પદાવલિ_List **ppMinMax){
  પૂર્ણાંક eRet = WHERE_ORDERBY_NORMAL;      /* Return value */
  પદાવલિ_List *pEList;                     /* Arguments to agg function */
  અચલ અક્ષર *zFunc;                    /* Name of aggregate function pFunc */
  પદાવલિ_List *pOrderBy;
  u8 sortFlags = 0;

  પ્રતિજ્ઞા( *ppMinMax==0 );
  પ્રતિજ્ઞા( pFunc->op==TK_AGG_FUNCTION );
  પ્રતિજ્ઞા( !IsWindowFunc(pFunc) );
  પ્રતિજ્ઞા( પદાવલિ_Use_XList(pFunc) );
  pEList = pFunc->x.pList;
  જો( pEList==0
   || pEList->n_પદાવલિ!=1
   || પદાવલિ_Has_Property(pFunc, EP_WinFunc)
   || OptimizationDisabled(db, SQLITE_MinMaxOpt)
  ){
    પાછા eRet;
  }
  પ્રતિજ્ઞા( !પદાવલિ_Has_Property(pFunc, EP_Int_કિંમત) );
  zFunc = pFunc->u.zToken;
  જો( sqlite3StrICmp(zFunc, "min")==0 ){
    eRet = WHERE_ORDERBY_MIN;
    જો( sqlite3_પદાવલિ_Can_Be_Null(pEList->a[0].p_પદાવલિ) ){
      sortFlags = KEYINFO_ORDER_BIGNULL;
    }
  }નહીંતર જો( sqlite3StrICmp(zFunc, "max")==0 ){
    eRet = WHERE_ORDERBY_MAX;
    sortFlags = KEYINFO_ORDER_DESC;
  }નહીંતર{
    પાછા eRet;
  }
  *ppMinMax = pOrderBy = sqlite3_પદાવલિ_List_Dup(db, pEList, 0);
  પ્રતિજ્ઞા( pOrderBy!=0 || db->mallocFailed );
  જો( pOrderBy ) pOrderBy->a[0].fg.sortFlags = sortFlags;
  પાછા eRet;
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
સ્થિર Table *is_Simple_ગણતરી(Select *p, AggInfo *pAggInfo){
  Table *pTab;
  પદાવલિ *p_પદાવલિ;

  પ્રતિજ્ઞા( !p->pGroupBy );

  જો( p->pWhere
   || p->pEList->n_પદાવલિ!=1
   || p->pSrc->nSrc!=1
   || p->pSrc->a[0].fg.isSubquery
   || pAggInfo->nFunc!=1
   || p->pHaving
  ){
    પાછા 0;
  }
  pTab = p->pSrc->a[0].pSTab;
  પ્રતિજ્ઞા( pTab!=0 );
  પ્રતિજ્ઞા( !IsView(pTab) );
  જો( !IsOrdinaryTable(pTab) ) પાછા 0;
  p_પદાવલિ = p->pEList->a[0].p_પદાવલિ;
  પ્રતિજ્ઞા( p_પદાવલિ!=0 );
  જો( p_પદાવલિ->op!=TK_AGG_FUNCTION ) પાછા 0;
  જો( p_પદાવલિ->pAggInfo!=pAggInfo ) પાછા 0;
  જો( (pAggInfo->aFunc[0].pFunc->funcFlags&SQLITE_FUNC_ગણતરી)==0 ) પાછા 0;
  પ્રતિજ્ઞા( pAggInfo->aFunc[0].pFExpr==p_પદાવલિ );
  testcase( પદાવલિ_Has_Property(p_પદાવલિ, EP_Distinct) );
  testcase( પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) );
  જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_Distinct|EP_WinFunc) ) પાછા 0;

  પાછા pTab;
}

/*
** If the source-list item passed as an argument was augmented with an
** INDEXED BY clause, then try to locate the specified index. If there
** was such a clause and the named index cannot be found, return
** SQLITE_ERROR and leave an error in pParse. Otherwise, populate
** pFrom->pIndex and return SQLITE_OK.
*/
પૂર્ણાંક sqlite3IndexedByLookup(Parse *pParse, SrcItem *pFrom){
  Table *pTab = pFrom->pSTab;
  અક્ષર *zIndexedBy = pFrom->u1.zIndexedBy;
  Index *pIdx;
  પ્રતિજ્ઞા( pTab!=0 );
  પ્રતિજ્ઞા( pFrom->fg.isIndexedBy!=0 );

  માટે(pIdx=pTab->pIndex;
      pIdx && sqlite3StrICmp(pIdx->zName, zIndexedBy);
      pIdx=pIdx->pNext
  );
  જો( !pIdx ){
    sqlite3_Error_સંદેશ(pParse, "no such index: %s", zIndexedBy, 0);
    pParse->checkSchema = 1;
    પાછા SQLITE_ERROR;
  }
  પ્રતિજ્ઞા( pFrom->fg.isCte==0 );
  pFrom->u2.pIBIndex = pIdx;
  પાછા SQLITE_OK;
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
સ્થિર પૂર્ણાંક convertCompoundSelectToSubquery(Walker *pWalker, Select *p){
  પૂર્ણાંક i;
  Select *pNew;
  Select *pX;
  sqlite3 *db;
  સંરચના પદાવલિ_List_item *a;
  SrcList *pNewSrc;
  Parse *pParse;
  Token dummy;

  જો( p->pPrior==0 ) પાછા WRC_Continue;
  જો( p->pOrderBy==0 ) પાછા WRC_Continue;
  માટે(pX=p; pX && (pX->op==TK_ALL || pX->op==TK_SELECT); pX=pX->pPrior){}
  જો( pX==0 ) પાછા WRC_Continue;
  a = p->pOrderBy->a;
#ifndef SQLITE_OMIT_WINDOWFUNC
  /* If iOrderByCol is already non-zero, then it has already been matched
  ** to a result column of the SELECT statement. This occurs when the
  ** SELECT is rewritten for window-functions processing and then passed
  ** to sqlite3SelectPrep() and similar a second time. The rewriting done
  ** by this function is not required in this case. */
  જો( a[0].u.x.iOrderByCol ) પાછા WRC_Continue;
#endif
  માટે(i=p->pOrderBy->n_પદાવલિ-1; i>=0; i--){
    જો( a[i].p_પદાવલિ->flags & EP_Collate ) તોડો;
  }
  જો( i<0 ) પાછા WRC_Continue;

  /* If we reach this point, that means the transformation is required. */

  pParse = pWalker->pParse;
  db = pParse->db;
  pNew = sqlite3DbMallocZero(db, કદ(*pNew) );
  જો( pNew==0 ) પાછા WRC_Abort;
  memset(&dummy, 0, કદ(dummy));
  pNewSrc = sqlite3SrcListAppendFromTerm(pParse,0,0,0,&dummy,pNew,0);
  પ્રતિજ્ઞા( pNewSrc!=0 || pParse->nErr );
  જો( pParse->nErr ){
    sqlite3SrcListDelete(db, pNewSrc);
    પાછા WRC_Abort;
  }
  *pNew = *p;
  p->pSrc = pNewSrc;
  p->pEList = sqlite3_પદાવલિ_List_Append(pParse, 0, sqlite3_પદાવલિ(db, TK_ASTERISK, 0));
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
  પ્રતિજ્ઞા( (p->selFlags & SF_Converted)==0 );
  p->selFlags |= SF_Converted;
  પ્રતિજ્ઞા( pNew->pPrior!=0 );
  pNew->pPrior->pNext = pNew;
  pNew->pLimit = 0;
  પાછા WRC_Continue;
}

/*
** Check to see if the FROM clause term pFrom has table-valued function
** arguments.  If it does, leave an error message in pParse and return
** non-zero, since pFrom is not allowed to be a table-valued function.
*/
સ્થિર પૂર્ણાંક cannotBeFunction(Parse *pParse, SrcItem *pFrom){
  જો( pFrom->fg.isTabFunc ){
    sqlite3_Error_સંદેશ(pParse, "'%s' is not a function", pFrom->zName);
    પાછા 1;
  }
  પાછા 0;
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
સ્થિર સંરચના Cte *શોધ_With(
  With *pWith,                    /* Current innermost WITH clause */
  SrcItem *pItem,                 /* FROM clause element to resolve */
  With **ppContext                /* OUT: WITH clause return value belongs to */
){
  અચલ અક્ષર *zName = pItem->zName;
  With *p;
  પ્રતિજ્ઞા( pItem->fg.fixedSchema || pItem->u4.zDatabase==0 );
  પ્રતિજ્ઞા( zName!=0 );
  માટે(p=pWith; p; p=p->pOuter){
    પૂર્ણાંક i;
    માટે(i=0; i<p->nCte; i++){
      જો( sqlite3StrICmp(zName, p->a[i].zName)==0 ){
        *ppContext = p;
        પાછા &p->a[i];
      }
    }
    જો( p->bView ) તોડો;
  }
  પાછા 0;
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
With *sqlite3_With_ધકેલો(Parse *pParse, With *pWith, u8 bFree){
  જો( pWith ){
    જો( bFree ){
      pWith = (With*)sqlite3ParserAddCleanup(pParse, sqlite3WithDeleteGeneric,
                      pWith);
      જો( pWith==0 ) પાછા 0;
    }
    જો( pParse->nErr==0 ){
      પ્રતિજ્ઞા( pParse->pWith!=pWith );
      pWith->pOuter = pParse->pWith;
      pParse->pWith = pWith;
    }
  }
  પાછા pWith;
}

સ્થિર પૂર્ણાંક selectExpanderWalk(Walker*, Select*);

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
સ્થિર પૂર્ણાંક resolveFromTermToCte(
  Parse *pParse,                  /* The parsing context */
  Walker *pWalker,                /* Current tree walker */
  SrcItem *pFrom                  /* The FROM clause term to check */
){
  Cte *pCte;               /* Matched CTE (or NULL if no match) */
  With *pWith;             /* The matching WITH */

  પ્રતિજ્ઞા( pFrom->pSTab==0 );
  જો( pParse->pWith==0 ){
    /* There are no WITH clauses in the stack.  No match is possible */
    પાછા 0;
  }
  જો( pParse->nErr ){
    /* Prior errors might have left pParse->pWith in a goofy state, so
    ** go no further. */
    પાછા 0;
  }
  પ્રતિજ્ઞા( pFrom->fg.hadSchema==0 || pFrom->fg.notCte!=0 );
  જો( pFrom->fg.fixedSchema==0 && pFrom->u4.zDatabase!=0 ){
    /* The FROM term contains a schema qualifier (ex: main.t1) and so
    ** it cannot possibly be a CTE reference. */
    પાછા 0;
  }
  જો( pFrom->fg.notCte ){
    /* The FROM term is specifically excluded from matching a CTE.
    **   (1)  It is part of a trigger that used to have zDatabase but had
    **        zDatabase removed by sqlite3FixTriggerStep().
    **   (2)  This is the first term in the FROM clause of an UPDATE.
    */
    પાછા 0;
  }
  pCte = શોધ_With(pParse->pWith, pFrom, &pWith);
  જો( pCte ){
    sqlite3 *db = pParse->db;
    Table *pTab;
    પદાવલિ_List *pEList;
    Select *pSel;
    Select *p_ડાબું;                /* Left-most SELECT statement */
    Select *pRecTerm;             /* Left-most recursive term */
    પૂર્ણાંક bMayRecursive;            /* True if compound joined by UNION [ALL] */
    With *pSavedWith;             /* Initial value of pParse->pWith */
    પૂર્ણાંક iRecTab = -1;             /* Cursor for recursive table */
    CteUse *pCteUse;

    /* If pCte->zCteErr is non-NULL at this point, then this is an illegal
    ** recursive reference to CTE pCte. Leave an error in pParse and return
    ** early. If pCte->zCteErr is NULL, then this is not a recursive reference.
    ** In this case, proceed.  */
    જો( pCte->zCteErr ){
      sqlite3_Error_સંદેશ(pParse, pCte->zCteErr, pCte->zName);
      પાછા 2;
    }
    જો( cannotBeFunction(pParse, pFrom) ) પાછા 2;

    પ્રતિજ્ઞા( pFrom->pSTab==0 );
    pTab = sqlite3DbMallocZero(db, કદ(Table));
    જો( pTab==0 ) પાછા 2;
    pCteUse = pCte->pUse;
    જો( pCteUse==0 ){
      pCte->pUse = pCteUse = sqlite3DbMallocZero(db, કદ(pCteUse[0]));
      જો( pCteUse==0
       || sqlite3ParserAddCleanup(pParse,sqlite3DbFree,pCteUse)==0
      ){
        sqlite3DbFree(db, pTab);
        પાછા 2;
      }
      pCteUse->eM10d = pCte->eM10d;
    }
    pFrom->pSTab = pTab;
    pTab->nTabRef = 1;
    pTab->zName = sqlite3DbStrDup(db, pCte->zName);
    pTab->iPKey = -1;
    pTab->nRowLogEst = 200; પ્રતિજ્ઞા( 200==sqlite3LogEst(1048576) );
    pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
    sqlite3SrcItemAttachSubquery(pParse, pFrom, pCte->pSelect, 1);
    જો( db->mallocFailed ) પાછા 2;
    પ્રતિજ્ઞા( pFrom->fg.isSubquery && pFrom->u4.pSubq );
    pSel = pFrom->u4.pSubq->pSelect;
    પ્રતિજ્ઞા( pSel!=0 );
    pSel->selFlags |= SF_CopyCte;
    જો( pFrom->fg.isIndexedBy ){
      sqlite3_Error_સંદેશ(pParse, "no such index: \"%s\"", pFrom->u1.zIndexedBy);
      પાછા 2;
    }
    પ્રતિજ્ઞા( !pFrom->fg.isIndexedBy );
    pFrom->fg.isCte = 1;
    pFrom->u2.pCteUse = pCteUse;
    pCteUse->nUse++;

    /* Check if this is a recursive CTE. */
    pRecTerm = pSel;
    bMayRecursive = ( pSel->op==TK_ALL || pSel->op==TK_UNION );
    જ્યારે( bMayRecursive && pRecTerm->op==pSel->op ){
      પૂર્ણાંક i;
      SrcList *pSrc = pRecTerm->pSrc;
      પ્રતિજ્ઞા( pRecTerm->pPrior!=0 );
      માટે(i=0; i<pSrc->nSrc; i++){
        SrcItem *pItem = &pSrc->a[i];
        જો( pItem->zName!=0
         && !pItem->fg.hadSchema
         && ALWAYS( !pItem->fg.isSubquery )
         && (pItem->fg.fixedSchema || pItem->u4.zDatabase==0)
         && 0==sqlite3StrICmp(pItem->zName, pCte->zName)
        ){
          pItem->pSTab = pTab;
          pTab->nTabRef++;
          pItem->fg.isRecursive = 1;
          જો( pRecTerm->selFlags & SF_Recursive ){
            sqlite3_Error_સંદેશ(pParse,
               "multiple references to recursive table: %s", pCte->zName
            );
            પાછા 2;
          }
          pRecTerm->selFlags |= SF_Recursive;
          જો( iRecTab<0 ) iRecTab = pParse->nTab++;
          pItem->iCursor = iRecTab;
        }
      }
      જો( (pRecTerm->selFlags & SF_Recursive)==0 ) તોડો;
      pRecTerm = pRecTerm->pPrior;
    }

    જો( pParse->nTab >= db->aLimit[SQLITE_LIMIT_CTES]
     && db->aLimit[SQLITE_LIMIT_CTES]>0
    ){
      sqlite3_Error_સંદેશ(pParse, "too many CTEs");
      પાછા 2;
    }
#જો SQLITE_MAX_પદાવલિ_DEPTH>0
    pParse->nNestSel++;
    જો( checkSubqueryNestingDepth(pParse) ) પાછા 2;
#endif
    pCte->zCteErr = "circular reference: %s";
    pSavedWith = pParse->pWith;
    pParse->pWith = pWith;
    જો( pSel->selFlags & SF_Recursive ){
      પૂર્ણાંક rc;
      પ્રતિજ્ઞા( pRecTerm!=0 );
      પ્રતિજ્ઞા( (pRecTerm->selFlags & SF_Recursive)==0 );
      પ્રતિજ્ઞા( pRecTerm->pNext!=0 );
      પ્રતિજ્ઞા( (pRecTerm->pNext->selFlags & SF_Recursive)!=0 );
      પ્રતિજ્ઞા( pRecTerm->pWith==0 );
      pRecTerm->pWith = pSel->pWith;
      rc = sqlite3WalkSelect(pWalker, pRecTerm);
      pRecTerm->pWith = 0;
      જો( rc ){
        pParse->pWith = pSavedWith;
        પાછા 2;
      }
    }નહીંતર{
      જો( selectExpanderWalk(pWalker, pSel) ){
        pParse->pWith = pSavedWith;
        પાછા 2;
      }
    }
    pParse->pWith = pWith;
#જો SQLITE_MAX_પદાવલિ_DEPTH>0
    pParse->nNestSel--;
    પ્રતિજ્ઞા( pParse->nNestSel>=0 );
#endif

    માટે(p_ડાબું=pSel; p_ડાબું->pPrior; p_ડાબું=p_ડાબું->pPrior);
    pEList = p_ડાબું->pEList;
    જો( pCte->pCols ){
      જો( pEList && pEList->n_પદાવલિ!=pCte->pCols->n_પદાવલિ ){
        sqlite3_Error_સંદેશ(pParse, "table %s has %d values for %d columns",
            pCte->zName, pEList->n_પદાવલિ, pCte->pCols->n_પદાવલિ
        );
        pParse->pWith = pSavedWith;
        પાછા 2;
      }
      pEList = pCte->pCols;
    }

    sqlite3_Columns_From_પદાવલિ_List(pParse, pEList, &pTab->nCol, &pTab->aCol);
    જો( bMayRecursive ){
      જો( pSel->selFlags & SF_Recursive ){
        pCte->zCteErr = "multiple recursive references: %s";
      }નહીંતર{
        pCte->zCteErr = "recursive reference in a subquery: %s";
      }
      sqlite3WalkSelect(pWalker, pSel);
    }
    pCte->zCteErr = 0;
    pParse->pWith = pSavedWith;
    પાછા 1;  /* Success */
  }
  પાછા 0;  /* No match */
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
શૂન્ય sqlite3_Select_કાઢો_With(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  જો( OK_IF_ALWAYS_TRUE(pParse->pWith) && p->pPrior==0 ){
    With *pWith = findRightmost(p)->pWith;
    જો( pWith!=0 ){
      પ્રતિજ્ઞા( pParse->pWith==pWith || pParse->nErr );
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
પૂર્ણાંક sqlite3ExpandSubquery(Parse *pParse, SrcItem *pFrom){
  Select *pSel;
  Table *pTab;

  પ્રતિજ્ઞા( pFrom->fg.isSubquery );
  પ્રતિજ્ઞા( pFrom->u4.pSubq!=0 );
  pSel = pFrom->u4.pSubq->pSelect;
  પ્રતિજ્ઞા( pSel );
  pFrom->pSTab = pTab = sqlite3DbMallocZero(pParse->db, કદ(Table));
  જો( pTab==0 ) પાછા SQLITE_NOMEM;
  pTab->nTabRef = 1;
  જો( pFrom->zAlias ){
    pTab->zName = sqlite3DbStrDup(pParse->db, pFrom->zAlias);
  }નહીંતર{
    pTab->zName = sqlite3MPrintf(pParse->db, "%!S", pFrom);
  }
  જ્યારે( pSel->pPrior ){ pSel = pSel->pPrior; }
  sqlite3_Columns_From_પદાવલિ_List(pParse, pSel->pEList,&pTab->nCol,&pTab->aCol);
  pTab->iPKey = -1;
  pTab->eTabType = TABTYP_VIEW;
  pTab->nRowLogEst = 200; પ્રતિજ્ઞા( 200==sqlite3LogEst(1048576) );
#ifndef SQLITE_ALLOW_ROWID_IN_VIEW
  /* The usual case - do not allow ROWID on a subquery */
  pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
#નહીંતર
  /* Legacy compatibility mode */
  pTab->tabFlags |= TF_Ephemeral | sqlite3Config.mNoVisibleRowid;
#endif
  પાછા pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
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
સ્થિર પૂર્ણાંક inAnyUsingClause(
  અચલ અક્ષર *zName, /* Name we are looking for */
  SrcItem *pBase,    /* The base SrcItem.  Looking at pBase[1] and following */
  પૂર્ણાંક N              /* How many SrcItems to check */
){
  જ્યારે( N>0 ){
    N--;
    pBase++;
    જો( pBase->fg.isUsing==0 ) ચાલુ;
    જો( NEVER(pBase->u3.pUsing==0) ) ચાલુ;
    જો( sqlite3IdListIndex(pBase->u3.pUsing, zName)>=0 ) પાછા 1;
  }
  પાછા 0;
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
સ્થિર પૂર્ણાંક selectExpander(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  પૂર્ણાંક i, j, k, rc;
  SrcList *pTabList;
  પદાવલિ_List *pEList;
  SrcItem *pFrom;
  sqlite3 *db = pParse->db;
  પદાવલિ *pE, *p_જમણું, *p_પદાવલિ;
  u16 selFlags = p->selFlags;
  u32 elistFlags = 0;

  p->selFlags |= SF_Expanded;
  જો( db->mallocFailed  ){
    પાછા WRC_Abort;
  }
  પ્રતિજ્ઞા( p->pSrc!=0 );
  જો( (selFlags & SF_Expanded)!=0 ){
    પાછા WRC_Prune;
  }
  જો( pWalker->eCode ){
    /* Renumber selId because it has been copied from a view */
    p->selId = ++pParse->nSelect;
  }
  pTabList = p->pSrc;
  pEList = p->pEList;
  જો( pParse->pWith && (p->selFlags & SF_View) ){
    જો( p->pWith==0 ){
      p->pWith = (With*)sqlite3DbMallocZero(db, SZ_WITH(1) );
      જો( p->pWith==0 ){
        પાછા WRC_Abort;
      }
    }
    p->pWith->bView = 1;
  }
  sqlite3_With_ધકેલો(pParse, p->pWith, 0);

  /* Make sure cursor numbers have been assigned to all entries in
  ** the FROM clause of the SELECT statement.
  */
  sqlite3SrcListAssignCursors(pParse, pTabList);

  /* Look up every table named in the FROM clause of the select.  If
  ** an entry of the FROM clause is a subquery instead of a table or view,
  ** then create a transient table structure to describe the subquery.
  */
  માટે(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    Table *pTab;
    પ્રતિજ્ઞા( pFrom->fg.isRecursive==0 || pFrom->pSTab!=0 );
    જો( pFrom->pSTab ) ચાલુ;
    પ્રતિજ્ઞા( pFrom->fg.isRecursive==0 );
    જો( pFrom->zName==0 ){
#ifndef SQLITE_OMIT_SUBQUERY
      Select *pSel;
      પ્રતિજ્ઞા( pFrom->fg.isSubquery && pFrom->u4.pSubq!=0 );
      pSel = pFrom->u4.pSubq->pSelect;
      /* A sub-query in the FROM clause of a SELECT */
      પ્રતિજ્ઞા( pSel!=0 );
      પ્રતિજ્ઞા( pFrom->pSTab==0 );
      જો( sqlite3WalkSelect(pWalker, pSel) ) પાછા WRC_Abort;
      જો( sqlite3ExpandSubquery(pParse, pFrom) ) પાછા WRC_Abort;
#endif
#ifndef SQLITE_OMIT_CTE
    }નહીંતર જો( (rc = resolveFromTermToCte(pParse, pWalker, pFrom))!=0 ){
      જો( rc>1 ) પાછા WRC_Abort;
      pTab = pFrom->pSTab;
      પ્રતિજ્ઞા( pTab!=0 );
#endif
    }નહીંતર{
      /* An ordinary table or view name in the FROM clause */
      પ્રતિજ્ઞા( pFrom->pSTab==0 );
      pFrom->pSTab = pTab = sqlite3LocateTableItem(pParse, 0, pFrom);
      જો( pTab==0 ) પાછા WRC_Abort;
      જો( pTab->nTabRef>=0xffff ){
        sqlite3_Error_સંદેશ(pParse, "too many references to \"%s\": max 65535",
           pTab->zName);
        pFrom->pSTab = 0;
        પાછા WRC_Abort;
      }
      pTab->nTabRef++;
      જો( !IsVirtual(pTab) && cannotBeFunction(pParse, pFrom) ){
        પાછા WRC_Abort;
      }
#જો !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
      જો( !IsOrdinaryTable(pTab) ){
        i16 nCol;
        u8 eCodeOrig = pWalker->eCode;
        જો( sqlite3ViewGetColumnNames(pParse, pTab) ) પાછા WRC_Abort;
        પ્રતિજ્ઞા( pFrom->fg.isSubquery==0 );
        જો( IsView(pTab) ){
          જો( (db->flags & SQLITE_EnableView)==0
           && pTab->pSchema!=db->aDb[1].pSchema
          ){
            sqlite3_Error_સંદેશ(pParse, "access to view \"%s\" prohibited",
              pTab->zName);
          }
          sqlite3SrcItemAttachSubquery(pParse, pFrom, pTab->u.view.pSelect, 1);
        }
#ifndef SQLITE_OMIT_VIRTUALTABLE
        નહીંતર જો( ALWAYS(IsVirtual(pTab))
         && (pFrom->fg.fromDDL || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
         && ALWAYS(pTab->u.vtab.p!=0)
         && pTab->u.vtab.p->eVtabRisk > ((db->flags & SQLITE_TrustedSchema)!=0)
        ){
          sqlite3_Error_સંદેશ(pParse, "unsafe use of virtual table \"%s\"",
                                  pTab->zName);
        }
        પ્રતિજ્ઞા( SQLITE_VTABRISK_Normal==1 && SQLITE_VTABRISK_ઉપલું==2 );
#endif
        nCol = pTab->nCol;
        pTab->nCol = -1;
        pWalker->eCode = 1;  /* Turn on Select.selId renumbering */
        જો( pFrom->fg.isSubquery ){
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
    જો( pFrom->fg.isIndexedBy && sqlite3IndexedByLookup(pParse, pFrom) ){
      પાછા WRC_Abort;
    }
  }

  /* Process NATURAL keywords, and ON and USING clauses of joins.
  */
  પ્રતિજ્ઞા( db->mallocFailed==0 || pParse->nErr!=0 );
  જો( pParse->nErr || sqlite3ProcessJoin(pParse, p) ){
    પાછા WRC_Abort;
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
  માટે(k=0; k<pEList->n_પદાવલિ; k++){
    pE = pEList->a[k].p_પદાવલિ;
    જો( pE->op==TK_ASTERISK ) તોડો;
    પ્રતિજ્ઞા( pE->op!=TK_DOT || pE->p_જમણું!=0 );
    પ્રતિજ્ઞા( pE->op!=TK_DOT || (pE->p_ડાબું!=0 && pE->p_ડાબું->op==TK_ID) );
    જો( pE->op==TK_DOT && pE->p_જમણું->op==TK_ASTERISK ) તોડો;
    elistFlags |= pE->flags;
  }
  જો( k<pEList->n_પદાવલિ ){
    /*
    ** If we get here it means the result set contains one or more "*"
    ** operators that need to be expanded.  Loop through each expression
    ** in the result set and expand them one by one.
    */
    સંરચના પદાવલિ_List_item *a = pEList->a;
    પદાવલિ_List *pNew = 0;
    પૂર્ણાંક flags = pParse->db->flags;
    પૂર્ણાંક longNames = (flags & SQLITE_FullColNames)!=0
                      && (flags & SQLITE_ShortColNames)==0;

    માટે(k=0; k<pEList->n_પદાવલિ; k++){
      pE = a[k].p_પદાવલિ;
      elistFlags |= pE->flags;
      p_જમણું = pE->p_જમણું;
      પ્રતિજ્ઞા( pE->op!=TK_DOT || p_જમણું!=0 );
      જો( pE->op!=TK_ASTERISK
       && (pE->op!=TK_DOT || p_જમણું->op!=TK_ASTERISK)
      ){
        /* This particular expression does not need to be expanded.
        */
        pNew = sqlite3_પદાવલિ_List_Append(pParse, pNew, a[k].p_પદાવલિ);
        જો( pNew ){
          pNew->a[pNew->n_પદાવલિ-1].zEName = a[k].zEName;
          pNew->a[pNew->n_પદાવલિ-1].fg.eEName = a[k].fg.eEName;
          a[k].zEName = 0;
        }
        a[k].p_પદાવલિ = 0;
      }નહીંતર{
        /* This expression is a "*" or a "TABLE.*" and needs to be
        ** expanded. */
        પૂર્ણાંક tableSeen = 0;      /* Set to 1 when TABLE matches */
        અક્ષર *zTName = 0;       /* text of name of TABLE */
        પૂર્ણાંક iErrOfst;
        જો( pE->op==TK_DOT ){
          પ્રતિજ્ઞા( (selFlags & SF_NestedFrom)==0 );
          પ્રતિજ્ઞા( pE->p_ડાબું!=0 );
          પ્રતિજ્ઞા( !પદાવલિ_Has_Property(pE->p_ડાબું, EP_Int_કિંમત) );
          zTName = pE->p_ડાબું->u.zToken;
          પ્રતિજ્ઞા( પદાવલિ_Use_WOfst(pE->p_ડાબું) );
          iErrOfst = pE->p_જમણું->w.iOfst;
        }નહીંતર{
          પ્રતિજ્ઞા( pE->p_ડાબું==0 );
          પ્રતિજ્ઞા( પદાવલિ_Use_WOfst(pE) );
          iErrOfst = pE->w.iOfst;
        }
        માટે(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
          પૂર્ણાંક nAdd;                    /* Number of cols including rowid */
          Table *pTab = pFrom->pSTab;  /* Table for this data source */
          પદાવલિ_List *pNestedFrom;       /* Result-set of a nested FROM clause */
          અક્ષર *zTabName;              /* AS name for this data source */
          અચલ અક્ષર *zSchemaName = 0; /* Schema name for this data source */
          પૂર્ણાંક iDb;                     /* Schema index for this data src */
          IdList *pUsing;              /* USING clause for pFrom[1] */

          જો( (zTabName = pFrom->zAlias)==0 ){
            zTabName = pTab->zName;
          }
          જો( db->mallocFailed ) તોડો;
          પ્રતિજ્ઞા( (પૂર્ણાંક)pFrom->fg.isNestedFrom == IsNestedFrom(pFrom) );
          જો( pFrom->fg.isNestedFrom ){
            પ્રતિજ્ઞા( pFrom->fg.isSubquery && pFrom->u4.pSubq );
            પ્રતિજ્ઞા( pFrom->u4.pSubq->pSelect!=0 );
            pNestedFrom = pFrom->u4.pSubq->pSelect->pEList;
            પ્રતિજ્ઞા( pNestedFrom!=0 );
            પ્રતિજ્ઞા( pNestedFrom->n_પદાવલિ==pTab->nCol );
            પ્રતિજ્ઞા( VisibleRowid(pTab)==0 || ViewCanHaveRowid );
          }નહીંતર{
            જો( zTName && sqlite3StrICmp(zTName, zTabName)!=0 ){
              ચાલુ;
            }
            pNestedFrom = 0;
            iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
            zSchemaName = iDb>=0 ? db->aDb[iDb].zDbSName : "*";
          }
          જો( i+1<pTabList->nSrc
           && pFrom[1].fg.isUsing
           && (selFlags & SF_NestedFrom)!=0
          ){
            પૂર્ણાંક ii;
            pUsing = pFrom[1].u3.pUsing;
            માટે(ii=0; ii<pUsing->nId; ii++){
              અચલ અક્ષર *zUName = pUsing->a[ii].zName;
              p_જમણું = sqlite3_પદાવલિ(db, TK_ID, zUName);
              sqlite3_પદાવલિ_Set_Error_Offset(p_જમણું, iErrOfst);
              pNew = sqlite3_પદાવલિ_List_Append(pParse, pNew, p_જમણું);
              જો( pNew ){
                સંરચના પદાવલિ_List_item *pX = &pNew->a[pNew->n_પદાવલિ-1];
                પ્રતિજ્ઞા( pX->zEName==0 );
                pX->zEName = sqlite3MPrintf(db,"..%s", zUName);
                pX->fg.eEName = ENAME_TAB;
                pX->fg.bUsingTerm = 1;
              }
            }
          }નહીંતર{
            pUsing = 0;
          }

          nAdd = pTab->nCol;
          જો( VisibleRowid(pTab) && (selFlags & SF_NestedFrom)!=0 ) nAdd++;
          માટે(j=0; j<nAdd; j++){
            અચલ અક્ષર *zName; 
            સંરચના પદાવલિ_List_item *pX; /* Newly added ExprList term */

            જો( j==pTab->nCol ){
              zName = sqlite3RowidAlias(pTab);
              જો( zName==0 ) ચાલુ;
            }નહીંતર{
              zName = pTab->aCol[j].zCnName;

              /* If pTab is actually an SF_NestedFrom sub-select, do not
              ** expand any ENAME_ROWID columns.  */
              જો( pNestedFrom && pNestedFrom->a[j].fg.eEName==ENAME_ROWID ){
                ચાલુ;
              }

              જો( zTName
               && pNestedFrom
               && sqlite3MatchEName(&pNestedFrom->a[j], 0, zTName, 0, 0)==0
              ){
                ચાલુ;
              }

              /* If a column is marked as 'hidden', omit it from the expanded
              ** result-set list unless the SELECT has the SF_IncludeHidden
              ** bit set.
              */
              જો( (p->selFlags & SF_IncludeHidden)==0
                && IsHiddenColumn(&pTab->aCol[j])
              ){
                ચાલુ;
              }
              જો( (pTab->aCol[j].colFlags & COLFLAG_NOEXPAND)!=0
               && zTName==0
               && (selFlags & (SF_NestedFrom))==0
              ){
                ચાલુ;
              }
            }
            પ્રતિજ્ઞા( zName );
            tableSeen = 1;

            જો( i>0 && zTName==0 && (selFlags & SF_NestedFrom)==0 ){
              જો( pFrom->fg.isUsing
               && sqlite3IdListIndex(pFrom->u3.pUsing, zName)>=0
              ){
                /* In a join with a USING clause, omit columns in the
                ** using clause from the table on the right. */
                ચાલુ;
              }
            }
            p_જમણું = sqlite3_પદાવલિ(db, TK_ID, zName);
            જો( (pTabList->nSrc>1
                 && (  (pFrom->fg.jointype & JT_LTORJ)==0
                     || zTName!=0
                     || (selFlags & SF_NestedFrom)!=0
                     || !inAnyUsingClause(zName,pFrom,pTabList->nSrc-i-1)
                    )
                )
             || IN_RENAME_OBJECT
            ){
              પદાવલિ *p_ડાબું;
              p_ડાબું = sqlite3_પદાવલિ(db, TK_ID, zTabName);
              p_પદાવલિ = sqlite3PExpr(pParse, TK_DOT, p_ડાબું, p_જમણું);
              જો( IN_RENAME_OBJECT && pE->p_ડાબું ){
                પ્રતિજ્ઞા( pE->op==TK_DOT && pE->p_જમણું->op==TK_ASTERISK );
                sqlite3RenameTokenRemap(pParse, p_ડાબું, pE->p_ડાબું);
              }
              જો( zSchemaName ){
                p_ડાબું = sqlite3_પદાવલિ(db, TK_ID, zSchemaName);
                p_પદાવલિ = sqlite3PExpr(pParse, TK_DOT, p_ડાબું, p_પદાવલિ);
              }
            }નહીંતર{
              p_પદાવલિ = p_જમણું;
            }
            sqlite3_પદાવલિ_Set_Error_Offset(p_પદાવલિ, iErrOfst);
            pNew = sqlite3_પદાવલિ_List_Append(pParse, pNew, p_પદાવલિ);
            જો( pNew==0 ){
              તોડો;  /* OOM */
            }
            pX = &pNew->a[pNew->n_પદાવલિ-1];
            પ્રતિજ્ઞા( pX->zEName==0 );
            જો( (selFlags & SF_NestedFrom)!=0 && !IN_RENAME_OBJECT ){
              જો( pNestedFrom && (!ViewCanHaveRowid || j<pNestedFrom->n_પદાવલિ) ){
                પ્રતિજ્ઞા( j<pNestedFrom->n_પદાવલિ );
                pX->zEName = sqlite3DbStrDup(db, pNestedFrom->a[j].zEName);
                testcase( pX->zEName==0 );
              }નહીંતર{
                pX->zEName = sqlite3MPrintf(db, "%s.%s.%s",
                                           zSchemaName, zTabName, zName);
                testcase( pX->zEName==0 );
              }
              pX->fg.eEName = (j==pTab->nCol ? ENAME_ROWID : ENAME_TAB);
              જો( (pFrom->fg.isUsing
                   && sqlite3IdListIndex(pFrom->u3.pUsing, zName)>=0)
               || (pUsing && sqlite3IdListIndex(pUsing, zName)>=0)
               || (j<pTab->nCol && (pTab->aCol[j].colFlags & COLFLAG_NOEXPAND))
              ){
                pX->fg.bNoExpand = 1;
              }
            }નહીંતર જો( longNames ){
              pX->zEName = sqlite3MPrintf(db, "%s.%s", zTabName, zName);
              pX->fg.eEName = ENAME_NAME;
            }નહીંતર{
              pX->zEName = sqlite3DbStrDup(db, zName);
              pX->fg.eEName = ENAME_NAME;
            }
          }
        }
        જો( !tableSeen ){
          જો( zTName ){
            sqlite3_Error_સંદેશ(pParse, "no such table: %s", zTName);
          }નહીંતર{
            sqlite3_Error_સંદેશ(pParse, "no tables specified");
          }
        }
      }
    }
    sqlite3_પદાવલિ_List_Delete(db, pEList);
    p->pEList = pNew;
  }
  જો( p->pEList ){
    જો( p->pEList->n_પદાવલિ>db->aLimit[SQLITE_LIMIT_COLUMN] ){
      sqlite3_Error_સંદેશ(pParse, "too many columns in result set");
      પાછા WRC_Abort;
    }
    જો( (elistFlags & (EP_HasFunc|EP_Subquery))!=0 ){
      p->selFlags |= SF_ComplexResult;
    }
  }
#જો TREETRACE_ENABLED
  જો( sqlite3_વૃક્ષ_Trace & 0x8 ){
    TREETRACE(0x8,pParse,p,("After result-set wildcard expansion:\n"));
    sqlite3_વૃક્ષ_View_Select(0, p, 0);
  }
#endif
  પાછા WRC_Continue;
}

#જો SQLITE_DEBUG
/*
** Always assert.  This xSelectCallback2 implementation proves that the
** xSelectCallback2 is never invoked.
*/
શૂન્ય sqlite3SelectWalkAssert2(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  પ્રતિજ્ઞા( 0 );
}
#endif

/*
** The first argument is a Walker object created by sqlite3SelectExpand().
** This function uses the walker to expand the SELECT statement passed
** as the second argument. If necessary, it also converts any compound
** SELECT statements that have COLLATE clauses on ORDER BY expressions
** to sub-queries. Return the return value of sqlite3WalkSelect().
*/
સ્થિર પૂર્ણાંક selectExpanderWalk(Walker *pWalker, Select *pSelect){
  પ્રતિજ્ઞા( pWalker->x_પદાવલિ_Callback==sqlite3_પદાવલિ_Walk_Noop );
  પ્રતિજ્ઞા( pWalker->pParse!=0 );
  જો( OK_IF_ALWAYS_TRUE(pWalker->pParse->hasCompound) ){
    pWalker->xSelectCallback = convertCompoundSelectToSubquery;
    pWalker->xSelectCallback2 = 0;
    sqlite3WalkSelect(pWalker, pSelect);
  }
  pWalker->xSelectCallback = selectExpander;
  pWalker->xSelectCallback2 = sqlite3_Select_કાઢો_With;
  પાછા sqlite3WalkSelect(pWalker, pSelect);
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
સ્થિર શૂન્ય sqlite3SelectExpand(Parse *pParse, Select *pSelect){
  Walker w;
  w.x_પદાવલિ_Callback = sqlite3_પદાવલિ_Walk_Noop;
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
સ્થિર શૂન્ય selectAddSubqueryTypeInfo(Walker *pWalker, Select *p){
  Parse *pParse;
  પૂર્ણાંક i;
  SrcList *pTabList;
  SrcItem *pFrom;

  જો( p->selFlags & SF_HasTypeInfo ) પાછા;
  p->selFlags |= SF_HasTypeInfo;
  pParse = pWalker->pParse;
  પ્રતિજ્ઞા( (p->selFlags & SF_Resolved) );
  pTabList = p->pSrc;
  માટે(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    Table *pTab = pFrom->pSTab;
    પ્રતિજ્ઞા( pTab!=0 );
    જો( (pTab->tabFlags & TF_Ephemeral)!=0 && pFrom->fg.isSubquery ){
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
સ્થિર શૂન્ય sqlite3SelectAddTypeInfo(Parse *pParse, Select *pSelect){
#ifndef SQLITE_OMIT_SUBQUERY
  Walker w;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.xSelectCallback2 = selectAddSubqueryTypeInfo;
  w.x_પદાવલિ_Callback = sqlite3_પદાવલિ_Walk_Noop;
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
શૂન્ય sqlite3SelectPrep(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for container */
){
  પ્રતિજ્ઞા( p!=0 || pParse->db->mallocFailed );
  પ્રતિજ્ઞા( pParse->db->pParse==pParse );
  જો( pParse->db->mallocFailed ) પાછા;
  જો( p->selFlags & SF_HasTypeInfo ) પાછા;
  sqlite3SelectExpand(pParse, p);
  જો( pParse->nErr ) પાછા;
  sqlite3ResolveSelectNames(pParse, p, pOuterNC);
  જો( pParse->nErr ) પાછા;
  sqlite3SelectAddTypeInfo(pParse, p);
}

#જો TREETRACE_ENABLED
/*
** Display all information about an AggInfo object
*/
સ્થિર શૂન્ય printAggInfo(AggInfo *pAggInfo){
  પૂર્ણાંક ii;
  sqlite3DebugPrintf("AggInfo %d/%p:\n",
     pAggInfo->selId, pAggInfo);
  માટે(ii=0; ii<pAggInfo->nColumn; ii++){
    સંરચના AggInfo_col *pCol = &pAggInfo->aCol[ii];
    sqlite3DebugPrintf(
       "agg-column[%d] pTab=%s iTable=%d iColumn=%d iMem=%d"
       " iSorterColumn=%d %s\n",
       ii, pCol->pTab ? pCol->pTab->zName : "NULL",
       pCol->iTable, pCol->iColumn, pAggInfo->iFirstReg+ii,
       pCol->iSorterColumn,
       ii>=pAggInfo->nAccumulator ? "" : " Accumulator");
    sqlite3_વૃક્ષ_View_પદાવલિ(0, pAggInfo->aCol[ii].pCExpr, 0);
  }
  માટે(ii=0; ii<pAggInfo->nFunc; ii++){
    sqlite3DebugPrintf("agg-func[%d]: iMem=%d\n",
        ii, pAggInfo->iFirstReg+pAggInfo->nColumn+ii);
    sqlite3_વૃક્ષ_View_પદાવલિ(0, pAggInfo->aFunc[ii].pFExpr, 0);
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
સ્થિર શૂન્ય analyzeAggFuncArgs(
  AggInfo *pAggInfo,
  NameContext *pNC
){
  પૂર્ણાંક i;
  પ્રતિજ્ઞા( pAggInfo!=0 );
  પ્રતિજ્ઞા( pAggInfo->iFirstReg==0 );
  pNC->ncFlags |= NC_InAggFunc;
  માટે(i=0; i<pAggInfo->nFunc; i++){
    પદાવલિ *p_પદાવલિ = pAggInfo->aFunc[i].pFExpr;
    પ્રતિજ્ઞા( p_પદાવલિ->op==TK_FUNCTION || p_પદાવલિ->op==TK_AGG_FUNCTION );
    પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
    sqlite3_પદાવલિ_Analyze_Agg_List(pNC, p_પદાવલિ->x.pList);
    જો( p_પદાવલિ->p_ડાબું ){
      પ્રતિજ્ઞા( p_પદાવલિ->p_ડાબું->op==TK_ORDER );
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ->p_ડાબું) );
      sqlite3_પદાવલિ_Analyze_Agg_List(pNC, p_પદાવલિ->p_ડાબું->x.pList);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    પ્રતિજ્ઞા( !IsWindowFunc(p_પદાવલિ) );
    જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) ){
      sqlite3_પદાવલિ_Analyze_Aggregates(pNC, p_પદાવલિ->y.pWin->pFilter);
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
સ્થિર શૂન્ય optimize_Aggregate_Use_Of_Indexed_પદાવલિ(
  Parse *pParse,          /* Parsing context */
  Select *pSelect,        /* The SELECT statement being processed */
  AggInfo *pAggInfo,      /* The aggregate info */
  NameContext *pNC        /* Name context used to resolve agg-func args */
){
  પ્રતિજ્ઞા( pAggInfo->iFirstReg==0 );
  પ્રતિજ્ઞા( pSelect!=0 );
  પ્રતિજ્ઞા( pSelect->pGroupBy!=0 );
  pAggInfo->nColumn = pAggInfo->nAccumulator;
  જો( ALWAYS(pAggInfo->nSortingColumn>0) ){
    પૂર્ણાંક mx = pSelect->pGroupBy->n_પદાવલિ - 1;
    પૂર્ણાંક j, k;
    માટે(j=0; j<pAggInfo->nColumn; j++){
      k = pAggInfo->aCol[j].iSorterColumn;
      જો( k>mx ) mx = k;
    }
    pAggInfo->nSortingColumn = mx+1;
  }
  analyzeAggFuncArgs(pAggInfo, pNC);
#જો TREETRACE_ENABLED
  જો( sqlite3_વૃક્ષ_Trace & 0x20 ){
    Indexed_પદાવલિ *pIEpr;
    TREETRACE(0x20, pParse, pSelect,
        ("AggInfo (possibly) adjusted for Indexed Exprs\n"));
    sqlite3_વૃક્ષ_View_Select(0, pSelect, 0);
    માટે(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
      printf("data-cursor=%d index={%d,%d}\n",
          pIEpr->iDataCur, pIEpr->iIdxCur, pIEpr->iIdxCol);
      sqlite3_વૃક્ષ_View_પદાવલિ(0, pIEpr->p_પદાવલિ, 0);
    }
    printAggInfo(pAggInfo);
  }
#નહીંતર
  UNUSED_PARAMETER(pSelect);
  UNUSED_PARAMETER(pParse);
#endif
}

/*
** Walker callback for aggregateConvertIndexedExprRefToColumn().
*/
સ્થિર પૂર્ણાંક aggregateIdxEprRefToColCallback(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  AggInfo *pAggInfo;
  સંરચના AggInfo_col *pCol;
  UNUSED_PARAMETER(pWalker);
  જો( p_પદાવલિ->pAggInfo==0 ) પાછા WRC_Continue;
  જો( p_પદાવલિ->op==TK_AGG_COLUMN ) પાછા WRC_Continue;
  જો( p_પદાવલિ->op==TK_AGG_FUNCTION ) પાછા WRC_Continue;
  જો( p_પદાવલિ->op==TK_IF_NULL_ROW ) પાછા WRC_Continue;
  pAggInfo = p_પદાવલિ->pAggInfo;
  જો( NEVER(p_પદાવલિ->iAgg>=pAggInfo->nColumn) ) પાછા WRC_Continue;
  પ્રતિજ્ઞા( p_પદાવલિ->iAgg>=0 );
  pCol = &pAggInfo->aCol[p_પદાવલિ->iAgg];
  p_પદાવલિ->op = TK_AGG_COLUMN;
  p_પદાવલિ->iTable = pCol->iTable;
  p_પદાવલિ->iColumn = pCol->iColumn;
  પદાવલિ_Clear_Property(p_પદાવલિ, EP_Skip|EP_Collate|EP_Unlikely);
  પાછા WRC_Prune;
}

/*
** Convert every pAggInfo->aFunc[].pExpr such that any node within
** those expressions that has pAppInfo set is changed into a TK_AGG_COLUMN
** opcode.
*/
સ્થિર શૂન્ય aggregate_Convert_Indexed_પદાવલિ_Ref_To_Column(AggInfo *pAggInfo){
  પૂર્ણાંક i;
  Walker w;
  memset(&w, 0, કદ(w));
  w.x_પદાવલિ_Callback = aggregateIdxEprRefToColCallback;
  માટે(i=0; i<pAggInfo->nFunc; i++){
    sqlite3_Walk_પદાવલિ(&w, pAggInfo->aFunc[i].pFExpr);
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
સ્થિર શૂન્ય assignAggregateRegisters(Parse *pParse, AggInfo *pAggInfo){
  પ્રતિજ્ઞા( pAggInfo!=0 );
  પ્રતિજ્ઞા( pAggInfo->iFirstReg==0 );
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
સ્થિર શૂન્ય resetAccumulator(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  પૂર્ણાંક i;
  સંરચના AggInfo_func *pFunc;
  પૂર્ણાંક nReg = pAggInfo->nFunc + pAggInfo->nColumn;
  પ્રતિજ્ઞા( pAggInfo->iFirstReg>0 );
  પ્રતિજ્ઞા( pParse->db->pParse==pParse );
  પ્રતિજ્ઞા( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  જો( nReg==0 ) પાછા;
  જો( pParse->nErr ) પાછા;
  sqlite3VdbeAddOp3(v, OP_Null, 0, pAggInfo->iFirstReg,
                    pAggInfo->iFirstReg+nReg-1);
  માટે(pFunc=pAggInfo->aFunc, i=0; i<pAggInfo->nFunc; i++, pFunc++){
    જો( pFunc->iDistinct>=0 ){
      પદાવલિ *pE = pFunc->pFExpr;
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(pE) );
      જો( pE->x.pList==0 || pE->x.pList->n_પદાવલિ!=1 ){
        sqlite3_Error_સંદેશ(pParse, "DISTINCT aggregates must have exactly one "
           "argument");
        pFunc->iDistinct = -1;
      }નહીંતર{
        ચાવી_Info *p_ચાવી_Info = sqlite3_ચાવી_Info_From_પદાવલિ_List(pParse, pE->x.pList,0,0);
        pFunc->iDistAddr = sqlite3VdbeAddOp4(v, OP_OpenEphemeral,
            pFunc->iDistinct, 0, 0, (અક્ષર*)p_ચાવી_Info, P4_KEYINFO);
        ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(DISTINCT)",
                          pFunc->pFunc->zName));
      }
    }
    જો( pFunc->iOBTab>=0 ){
      પદાવલિ_List *pOBList;
      ચાવી_Info *p_ચાવી_Info;
      પૂર્ણાંક nExtra = 0;
      પ્રતિજ્ઞા( pFunc->pFExpr->p_ડાબું!=0 );
      પ્રતિજ્ઞા( pFunc->pFExpr->p_ડાબું->op==TK_ORDER );
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(pFunc->pFExpr->p_ડાબું) );
      પ્રતિજ્ઞા( pFunc->pFunc!=0 );
      pOBList = pFunc->pFExpr->p_ડાબું->x.pList;
      જો( !pFunc->bOBUnique ){
        nExtra++;  /* One extra column for the OP_Sequence */
      }
      જો( pFunc->bOBPayload ){
        /* extra columns for the function arguments */
        પ્રતિજ્ઞા( પદાવલિ_Use_XList(pFunc->pFExpr) );
        પ્રતિજ્ઞા( pFunc->pFExpr->x.pList!=0 );
        nExtra += pFunc->pFExpr->x.pList->n_પદાવલિ;
      }
      જો( pFunc->bUseSubtype ){
        nExtra += pFunc->pFExpr->x.pList->n_પદાવલિ;
      }
      p_ચાવી_Info = sqlite3_ચાવી_Info_From_પદાવલિ_List(pParse, pOBList, 0, nExtra);
      જો( !pFunc->bOBUnique && pParse->nErr==0 ){
        p_ચાવી_Info->n_ચાવી_Field++;
      }
      sqlite3VdbeAddOp4(v, OP_OpenEphemeral,
            pFunc->iOBTab, pOBList->n_પદાવલિ+nExtra, 0,
            (અક્ષર*)p_ચાવી_Info, P4_KEYINFO);
      ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(ORDER BY)",
                          pFunc->pFunc->zName));
    }
  }
}

/*
** Invoke the OP_AggFinalize opcode for every aggregate function
** in the AggInfo structure.
*/
સ્થિર શૂન્ય finalizeAggFunctions(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  પૂર્ણાંક i;
  સંરચના AggInfo_func *pF;
  માટે(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    પદાવલિ_List *pList;
    પ્રતિજ્ઞા( પદાવલિ_Use_XList(pF->pFExpr) );
    જો( pParse->nErr ) પાછા;
    pList = pF->pFExpr->x.pList;
    જો( pF->iOBTab>=0 ){
      /* For an ORDER BY aggregate, calls to OP_AggStep were deferred.  Inputs
      ** were stored in emphermal table pF->iOBTab.  Here, we extract those
      ** inputs (in ORDER BY order) and make all calls to OP_AggStep
      ** before doing the OP_AggFinal call. */
      પૂર્ણાંક i_ટોચ;        /* Start of loop for extracting columns */
      પૂર્ણાંક nArg;        /* Number of columns to extract */
      પૂર્ણાંક n_ચાવી;        /* Key columns to be skipped */
      પૂર્ણાંક regAgg;      /* Extract into this array */
      પૂર્ણાંક j;           /* Loop counter */
     
      પ્રતિજ્ઞા( pF->pFunc!=0 );
      nArg = pList->n_પદાવલિ;
      regAgg = sqlite3GetTempRange(pParse, nArg);

      જો( pF->bOBPayload==0 ){
        n_ચાવી = 0;
      }નહીંતર{
        પ્રતિજ્ઞા( pF->pFExpr->p_ડાબું!=0 );
        પ્રતિજ્ઞા( પદાવલિ_Use_XList(pF->pFExpr->p_ડાબું) );
        પ્રતિજ્ઞા( pF->pFExpr->p_ડાબું->x.pList!=0 );
        n_ચાવી = pF->pFExpr->p_ડાબું->x.pList->n_પદાવલિ;
        જો( ALWAYS(!pF->bOBUnique) ) n_ચાવી++;
      }
      i_ટોચ = sqlite3VdbeAddOp1(v, OP_Rewind, pF->iOBTab); VdbeCoverage(v);
      માટે(j=nArg-1; j>=0; j--){
        sqlite3VdbeAddOp3(v, OP_Column, pF->iOBTab, n_ચાવી+j, regAgg+j);
      }
      જો( pF->bUseSubtype ){
        પૂર્ણાંક regSubtype = sqlite3GetTempReg(pParse);
        પૂર્ણાંક iBaseCol = n_ચાવી + nArg + (pF->bOBPayload==0 && pF->bOBUnique==0);
        માટે(j=nArg-1; j>=0; j--){
          sqlite3VdbeAddOp3(v, OP_Column, pF->iOBTab, iBaseCol+j, regSubtype);
          sqlite3VdbeAddOp2(v, OP_SetSubtype, regSubtype, regAgg+j);
        }
        sqlite3ReleaseTempReg(pParse, regSubtype);
      }
      sqlite3VdbeAddOp3(v, OP_AggStep, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3VdbeAddOp2(v, OP_Next, pF->iOBTab, i_ટોચ+1); VdbeCoverage(v);
      sqlite3VdbeJumpHere(v, i_ટોચ);
      sqlite3ReleaseTempRange(pParse, regAgg, nArg);
    }
    sqlite3VdbeAddOp2(v, OP_AggFinal, AggInfoFuncReg(pAggInfo,i),
                      pList ? pList->n_પદાવલિ : 0);
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
સ્થિર શૂન્ય updateAccumulator(
  Parse *pParse,
  પૂર્ણાંક regAcc,
  AggInfo *pAggInfo,
  પૂર્ણાંક eDistinctType
){
  Vdbe *v = pParse->pVdbe;
  પૂર્ણાંક i;
  પૂર્ણાંક regHit = 0;
  પૂર્ણાંક addrHitTest = 0;
  સંરચના AggInfo_func *pF;
  સંરચના AggInfo_col *pC;

  પ્રતિજ્ઞા( pAggInfo->iFirstReg>0 );
  જો( pParse->nErr ) પાછા;
  pAggInfo->directMode = 1;
  માટે(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    પૂર્ણાંક nArg;
    પૂર્ણાંક addrNext = 0;
    પૂર્ણાંક regAgg;
    પૂર્ણાંક regAggSz = 0;
    પૂર્ણાંક regDistinct = 0;
    પદાવલિ_List *pList;
    પ્રતિજ્ઞા( પદાવલિ_Use_XList(pF->pFExpr) );
    પ્રતિજ્ઞા( !IsWindowFunc(pF->pFExpr) );
    પ્રતિજ્ઞા( pF->pFunc!=0 );
    pList = pF->pFExpr->x.pList;
    જો( પદાવલિ_Has_Property(pF->pFExpr, EP_WinFunc) ){
      પદાવલિ *pFilter = pF->pFExpr->y.pWin->pFilter;
      જો( pAggInfo->nAccumulator
       && (pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)
       && regAcc
      ){
        /* If regAcc==0, there there exists some min() or max() function
        ** without a FILTER clause that will ensure the magnet registers
        ** are populated. */
        જો( regHit==0 ) regHit = ++pParse->nMem;
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
      sqlite3_પદાવલિ_If_False(pParse, pFilter, addrNext, SQLITE_JUMPIFNULL);
    }
    જો( pF->iOBTab>=0 ){
      /* Instead of invoking AggStep, we must push the arguments that would
      ** have been passed to AggStep onto the sorting table. */
      પૂર્ણાંક jj;                /* Registered used so far in building the record */
      પદાવલિ_List *pOBList;     /* The ORDER BY clause */
      પ્રતિજ્ઞા( pList!=0 );
      nArg = pList->n_પદાવલિ;
      પ્રતિજ્ઞા( nArg>0 );
      પ્રતિજ્ઞા( pF->pFExpr->p_ડાબું!=0 );
      પ્રતિજ્ઞા( pF->pFExpr->p_ડાબું->op==TK_ORDER );
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(pF->pFExpr->p_ડાબું) );
      pOBList = pF->pFExpr->p_ડાબું->x.pList;
      પ્રતિજ્ઞા( pOBList!=0 );
      પ્રતિજ્ઞા( pOBList->n_પદાવલિ>0 );
      regAggSz = pOBList->n_પદાવલિ;
      જો( !pF->bOBUnique ){
        regAggSz++;   /* One register for OP_Sequence */
      }
      જો( pF->bOBPayload ){
        regAggSz += nArg;
      }
      જો( pF->bUseSubtype ){
        regAggSz += nArg;
      }
      regAggSz++;  /* One extra register to hold result of MakeRecord */
      regAgg = sqlite3GetTempRange(pParse, regAggSz);
      regDistinct = regAgg;
      sqlite3_પદાવલિ_Code_પદાવલિ_List(pParse, pOBList, regAgg, 0, SQLITE_ECEL_DUP);
      jj = pOBList->n_પદાવલિ;
      જો( !pF->bOBUnique ){
        sqlite3VdbeAddOp2(v, OP_Sequence, pF->iOBTab, regAgg+jj);
        jj++;
      }
      જો( pF->bOBPayload ){
        regDistinct = regAgg+jj;
        sqlite3_પદાવલિ_Code_પદાવલિ_List(pParse, pList, regDistinct, 0, SQLITE_ECEL_DUP);
        jj += nArg;
      }
      જો( pF->bUseSubtype ){
        પૂર્ણાંક kk;
        પૂર્ણાંક regBase = pF->bOBPayload ? regDistinct : regAgg;
        માટે(kk=0; kk<nArg; kk++, jj++){
          sqlite3VdbeAddOp2(v, OP_GetSubtype, regBase+kk, regAgg+jj);
        }
      }
    }નહીંતર જો( pList ){
      nArg = pList->n_પદાવલિ;
      regAgg = sqlite3GetTempRange(pParse, nArg);
      regDistinct = regAgg;
      sqlite3_પદાવલિ_Code_પદાવલિ_List(pParse, pList, regAgg, 0, SQLITE_ECEL_DUP);
    }નહીંતર{
      nArg = 0;
      regAgg = 0;
    }
    જો( pF->iDistinct>=0 && pList ){
      જો( addrNext==0 ){
        addrNext = sqlite3VdbeMakeLabel(pParse);
      }
      pF->iDistinct = codeDistinct(pParse, eDistinctType,
          pF->iDistinct, addrNext, pList, regDistinct);
    }
    જો( pF->iOBTab>=0 ){
      /* Insert a new record into the ORDER BY table */
      sqlite3VdbeAddOp3(v, OP_MakeRecord, regAgg, regAggSz-1,
                        regAgg+regAggSz-1);
      sqlite3VdbeAddOp4Int(v, OP_Idx_ઉમેરો, pF->iOBTab, regAgg+regAggSz-1,
                           regAgg, regAggSz-1);
      sqlite3ReleaseTempRange(pParse, regAgg, regAggSz);
    }નહીંતર{
      /* Invoke the AggStep function */
      જો( pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        CollSeq *pColl = 0;
        સંરચના પદાવલિ_List_item *pItem;
        પૂર્ણાંક j;
        પ્રતિજ્ઞા( pList!=0 );  /* pList!=0 if pF->pFunc has NEEDCOLL */
        માટે(j=0, pItem=pList->a; !pColl && j<nArg; j++, pItem++){
          pColl = sqlite3_પદાવલિ_Coll_Seq(pParse, pItem->p_પદાવલિ);
        }
        જો( !pColl ){
          pColl = pParse->db->pDfltColl;
        }
        જો( regHit==0 && pAggInfo->nAccumulator ) regHit = ++pParse->nMem;
        sqlite3VdbeAddOp4(v, OP_CollSeq, regHit, 0, 0,
                         (અક્ષર *)pColl, P4_COLLSEQ);
      }
      sqlite3VdbeAddOp3(v, OP_AggStep, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3ReleaseTempRange(pParse, regAgg, nArg);
    }
    જો( addrNext ){
      sqlite3VdbeResolveLabel(v, addrNext);
    }
    જો( pParse->nErr ) પાછા;
  }
  જો( regHit==0 && pAggInfo->nAccumulator ){
    regHit = regAcc;
  }
  જો( regHit ){
    addrHitTest = sqlite3VdbeAddOp1(v, OP_If, regHit); VdbeCoverage(v);
  }
  માટે(i=0, pC=pAggInfo->aCol; i<pAggInfo->nAccumulator; i++, pC++){
    sqlite3_પદાવલિ_Code(pParse, pC->pCExpr, AggInfoColumnReg(pAggInfo,i));
    જો( pParse->nErr ) પાછા;
  }

  pAggInfo->directMode = 0;
  જો( addrHitTest ){
    sqlite3_Vdbe_Jump_Here_Or_કાઢો_Inst(v, addrHitTest);
  }
}

/*
** Add a single OP_Explain instruction to the VDBE to explain a simple
** count(*) query ("SELECT count(*) FROM pTab").
*/
#ifndef SQLITE_OMIT_EXPLAIN
સ્થિર શૂન્ય explain_Simple_ગણતરી(
  Parse *pParse,                  /* Parse context */
  Table *pTab,                    /* Table being queried */
  Index *pIdx                     /* Index used to optimize scan, or NULL */
){
  જો( pParse->explain==2 ){
    પૂર્ણાંક bCover = (pIdx!=0 && (HasRowid(pTab) || !Is_Primary_ચાવી_Index(pIdx)));
    sqlite3VdbeExplain(pParse, 0, "SCAN %s%s%s",
        pTab->zName,
        bCover ? " USING COVERING INDEX " : "",
        bCover ? pIdx->zName : ""
    );
  }
}
#નહીંતર
# define explain_Simple_ગણતરી(a,b,c)
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
સ્થિર પૂર્ણાંક having_To_Where_પદાવલિ_Cb(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  જો( p_પદાવલિ->op!=TK_AND ){
    Select *pS = pWalker->u.pSelect;
    /* This routine is called before the HAVING clause of the current
    ** SELECT is analyzed for aggregates. So if pExpr->pAggInfo is set
    ** here, it indicates that the expression is a correlated reference to a
    ** column from an outer aggregate query, or an aggregate function that
    ** belongs to an outer query. Do not move the expression to the WHERE
    ** clause in this obscure case, as doing so may corrupt the outer Select
    ** statements AggInfo structure.  */
    જો( sqlite3_પદાવલિ_Is_Constant_Or_Group_By(pWalker->pParse, p_પદાવલિ, pS->pGroupBy)
     && પદાવલિ_Always_False(p_પદાવલિ)==0
     && p_પદાવલિ->pAggInfo==0
    ){
      sqlite3 *db = pWalker->pParse->db;
      પદાવલિ *pNew = sqlite3_પદાવલિ_Int32(db, 1);
      જો( pNew ){
        પદાવલિ *pWhere = pS->pWhere;
        SWAP(પદાવલિ, *pNew, *p_પદાવલિ);
        pNew = sqlite3_પદાવલિ_And(pWalker->pParse, pWhere, pNew);
        pS->pWhere = pNew;
        pWalker->eCode = 1;
      }
    }
    પાછા WRC_Prune;
  }
  પાછા WRC_Continue;
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
સ્થિર શૂન્ય havingToWhere(Parse *pParse, Select *p){
  Walker sWalker;
  memset(&sWalker, 0, કદ(sWalker));
  sWalker.pParse = pParse;
  sWalker.x_પદાવલિ_Callback = having_To_Where_પદાવલિ_Cb;
  sWalker.u.pSelect = p;
  sqlite3_Walk_પદાવલિ(&sWalker, p->pHaving);
#જો TREETRACE_ENABLED
  જો( sWalker.eCode && (sqlite3_વૃક્ષ_Trace & 0x100)!=0 ){
    TREETRACE(0x100,pParse,p,("Move HAVING terms into WHERE:\n"));
    sqlite3_વૃક્ષ_View_Select(0, p, 0);
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
સ્થિર SrcItem *isSelfJoinView(
  SrcList *pTabList,           /* Search for self-joins in this FROM clause */
  SrcItem *pThis,              /* Search for prior reference to this subquery */
  પૂર્ણાંક iFirst, પૂર્ણાંક iEnd        /* Range of FROM-clause entries to search. */
){
  SrcItem *pItem;
  Select *pSel;
  પ્રતિજ્ઞા( pThis->fg.isSubquery );
  pSel = pThis->u4.pSubq->pSelect;
  પ્રતિજ્ઞા( pSel!=0 );
  જો( pSel->selFlags & SF_ધકેલો_Down ) પાછા 0;
  જ્યારે( iFirst<iEnd ){
    Select *pS1;
    pItem = &pTabList->a[iFirst++];
    જો( !pItem->fg.isSubquery ) ચાલુ;
    જો( pItem->fg.viaCoroutine ) ચાલુ;
    જો( pItem->zName==0 ) ચાલુ;
    પ્રતિજ્ઞા( pItem->pSTab!=0 );
    પ્રતિજ્ઞા( pThis->pSTab!=0 );
    જો( pItem->pSTab->pSchema!=pThis->pSTab->pSchema ) ચાલુ;
    જો( sqlite3_stricmp(pItem->zName, pThis->zName)!=0 ) ચાલુ;
    pS1 = pItem->u4.pSubq->pSelect;
    જો( pItem->pSTab->pSchema==0 && pSel->selId!=pS1->selId ){
      /* The query flattener left two different CTE tables with identical
      ** names in the same FROM clause. */
      ચાલુ;
    }
    જો( pS1->selFlags & SF_ધકેલો_Down ){
      /* The view was modified by some other optimization such as
      ** pushDownWhereTerms() */
      ચાલુ;
    }
    પાછા pItem;
  }
  પાછા 0;
}

/*
** Deallocate a single AggInfo object
*/
સ્થિર શૂન્ય agginfoFree(sqlite3 *db, શૂન્ય *pArg){
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
સ્થિર પૂર્ણાંક ગણતરી_Of_View_Optimization(Parse *pParse, Select *p){
  Select *pSub, *pPrior;
  પદાવલિ *p_પદાવલિ;
  પદાવલિ *p_ગણતરી;
  sqlite3 *db;
  SrcItem *pFrom;
  જો( (p->selFlags & SF_Aggregate)==0 ) પાછા 0;   /* This is an aggregate */
  જો( p->pEList->n_પદાવલિ!=1 ) પાછા 0;               /* Single result column */
  જો( p->pWhere ) પાછા 0;
  જો( p->pHaving ) પાછા 0;
  જો( p->pGroupBy ) પાછા 0;
  જો( p->pOrderBy ) પાછા 0;
  p_પદાવલિ = p->pEList->a[0].p_પદાવલિ;
  જો( p_પદાવલિ->op!=TK_AGG_FUNCTION ) પાછા 0;        /* Result is an aggregate */
  પ્રતિજ્ઞા( પદાવલિ_Use_UToken(p_પદાવલિ) );
  જો( sqlite3_stricmp(p_પદાવલિ->u.zToken,"count") ) પાછા 0;  /* Is count() */
  પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
  જો( p_પદાવલિ->x.pList!=0 ) પાછા 0;                 /* Must be count(*) */
  જો( p->pSrc->nSrc!=1 ) પાછા 0;                  /* One table in FROM  */
  જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) ) પાછા 0;/* Not a window function */
  pFrom = p->pSrc->a;
  જો( pFrom->fg.isSubquery==0 ) પાછા 0;           /* FROM is a subquery */
  જો( (p->selFlags & SF_Correlated)!=0 ) પાછા 0;  /* Not a correlated subq */
  pSub = pFrom->u4.pSubq->pSelect;
  જો( pSub->pPrior==0 ) પાછા 0;                   /* Must be a compound */
  જો( pSub->selFlags & SF_CopyCte ) પાછા 0;       /* Not a CTE */
  કરો{
    જો( pSub->op!=TK_ALL && pSub->pPrior ) પાછા 0;  /* Must be UNION ALL */
    જો( pSub->pWhere ) પાછા 0;                      /* No WHERE clause */
    જો( pSub->pLimit ) પાછા 0;                      /* No LIMIT clause */
    જો( pSub->selFlags & (SF_Aggregate|SF_Distinct) ){
       testcase( pSub->selFlags & SF_Aggregate );
       testcase( pSub->selFlags & SF_Distinct );
       પાછા 0;     /* Not an aggregate nor DISTINCT */
    }
    પ્રતિજ્ઞા( pSub->pHaving==0 );  /* Due to the previous */
    pSub = pSub->pPrior;                              /* Repeat over compound */
  }જ્યારે( pSub );

  /* If we reach this point then it is OK to perform the transformation */

  db = pParse->db;
  p_ગણતરી = p_પદાવલિ;
  p_પદાવલિ = 0;
  pSub = sqlite3SubqueryDetach(db, pFrom);
  sqlite3SrcListDelete(db, p->pSrc);
  p->pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  જ્યારે( pSub ){
    પદાવલિ *pTerm;
    pPrior = pSub->pPrior;
    pSub->pPrior = 0;
    pSub->pNext = 0;
    pSub->selFlags |= SF_Aggregate;
    pSub->selFlags &= ~(u32)SF_Compound;
    pSub->nSelectRow = 0;
    sqlite3ParserAddCleanup(pParse, sqlite3_પદાવલિ_List_Delete_Generic, pSub->pEList);
    pTerm = pPrior ? sqlite3_પદાવલિ_Dup(db, p_ગણતરી, 0) : p_ગણતરી;
    pSub->pEList = sqlite3_પદાવલિ_List_Append(pParse, 0, pTerm);
    pTerm = sqlite3PExpr(pParse, TK_SELECT, 0, 0);
    sqlite3PExprAddSelect(pParse, pTerm, pSub);
    જો( p_પદાવલિ==0 ){
      p_પદાવલિ = pTerm;
    }નહીંતર{
      p_પદાવલિ = sqlite3PExpr(pParse, TK_PLUS, pTerm, p_પદાવલિ);
    }
    જો( pParse->nErr ){
      sqlite3_પદાવલિ_Delete(db, p_પદાવલિ);
      p_પદાવલિ = 0;
    }
    pSub = pPrior;
  }
  p->pEList->a[0].p_પદાવલિ = p_પદાવલિ;
  p->selFlags &= ~(u32)SF_Aggregate;

#જો TREETRACE_ENABLED
  જો( sqlite3_વૃક્ષ_Trace & 0x200 ){
    TREETRACE(0x200,pParse,p,("After count-of-view optimization:\n"));
    sqlite3_વૃક્ષ_View_Select(0, p, 0);
  }
#endif
  પાછા 1;
}

/*
** If any term of pSrc, or any SF_NestedFrom sub-query, is not the same
** as pSrcItem but has the same alias as p0, then return true.
** Otherwise return false.
*/
સ્થિર પૂર્ણાંક sameSrcAlias(SrcItem *p0, SrcList *pSrc){
  પૂર્ણાંક i;
  માટે(i=0; i<pSrc->nSrc; i++){
    SrcItem *p1 = &pSrc->a[i];
    જો( p1==p0 ) ચાલુ;
    જો( p0->pSTab==p1->pSTab && 0==sqlite3_stricmp(p0->zAlias, p1->zAlias) ){
      પાછા 1;
    }
    જો( p1->fg.isSubquery
     && (p1->u4.pSubq->pSelect->selFlags & SF_NestedFrom)!=0
     && sameSrcAlias(p0, p1->u4.pSubq->pSelect->pSrc)
    ){
      પાછા 1;
    }
  }
  પાછા 0;
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
સ્થિર પૂર્ણાંક fromClauseTermCanBeCoroutine(
  Parse *pParse,          /* Parsing context */
  SrcList *pTabList,      /* FROM clause */
  પૂર્ણાંક i,                  /* Which term of the FROM clause holds the subquery */
  પૂર્ણાંક selFlags            /* Flags on the SELECT statement */
){
  SrcItem *pItem = &pTabList->a[i];
  જો( pItem->fg.isCte ){
    અચલ CteUse *pCteUse = pItem->u2.pCteUse;
    જો( pCteUse->eM10d==M10d_Yes ) પાછા 0;                          /* (2a) */
    જો( pCteUse->nUse>=2 && pCteUse->eM10d!=M10d_No ) પાછા 0;       /* (2b) */
  }
  જો( pTabList->a[0].fg.jointype & JT_LTORJ ) પાછા 0;               /* (3)  */
  જો( OptimizationDisabled(pParse->db, SQLITE_Coroutines) ) પાછા 0; /* (4)  */
  જો( isSelfJoinView(pTabList, pItem, i+1, pTabList->nSrc)!=0 ){
    પાછા 0;                                                          /* (5) */
  }
  જો( i==0 ){
    જો( pTabList->nSrc==1 ) પાછા 1;                             /* (1a) */
    જો( pTabList->a[1].fg.jointype & JT_CROSS ) પાછા 1;         /* (1b) */
    જો( selFlags & SF_UpdateFrom )              પાછા 0;         /* (1c-iii) */
    પાછા 1;
  }
  જો( selFlags & SF_UpdateFrom ) પાછા 0;                        /* (1c-iii) */
  જ્યારે( 1 /*exit-by-break*/ ){
    જો( pItem->fg.jointype & (JT_OUTER|JT_CROSS)  ) પાછા 0;     /* (1c-ii) */
    જો( i==0 ) તોડો;
    i--;
    pItem--;
    જો( pItem->fg.isSubquery ) પાછા 0;                          /* (1c-i) */
  }
  પાછા 1;
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
સ્થિર SQLITE_NOINLINE શૂન્ય existsToJoin(
  Parse *pParse,  /* Parsing context */
  Select *p,      /* The SELECT statement being optimized */
  પદાવલિ *pWhere    /* part of the WHERE clause currently being examined */
){
  જો( pParse->nErr==0
   && pWhere!=0
   && !પદાવલિ_Has_Property(pWhere, EP_OuterON|EP_InnerON) 
   && ALWAYS(p->pSrc!=0)
   && p->pSrc->nSrc<BMS
   && (p->pLimit==0 || p->pLimit->p_જમણું==0)
  ){
    જો( pWhere->op==TK_AND ){
      પદાવલિ *p_જમણું = pWhere->p_જમણું;
      existsToJoin(pParse, p, pWhere->p_ડાબું);
      existsToJoin(pParse, p, p_જમણું);
    }
    નહીંતર જો( pWhere->op==TK_EXISTS ){
      Select *pSub = pWhere->x.pSelect;
      પદાવલિ *pSubWhere = pSub->pWhere;
      જો( pSub->pSrc->nSrc==1
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
        પૂર્ણાંક *aCsrMap = sqlite3DbMallocZero(db, (pParse->nTab+2)*કદ(પૂર્ણાંક));
        જો( aCsrMap==0 ) પાછા;
        aCsrMap[0] = (pParse->nTab+1);
        renumberCursors(pParse, pSub, -1, aCsrMap);
        sqlite3DbFree(db, aCsrMap);

        memset(pWhere, 0, કદ(*pWhere));
        pWhere->op = TK_INTEGER;
        pWhere->u.i_કિંમત = 1;
        પદાવલિ_Set_Property(pWhere, EP_Int_કિંમત);
        પ્રતિજ્ઞા( p->pWhere!=0 );
        pSub->pSrc->a[0].fg.fromExists = 1;
        p->pSrc = sqlite3SrcListAppendList(pParse, p->pSrc, pSub->pSrc);
        જો( pSubWhere ){
          p->pWhere = sqlite3PExpr(pParse, TK_AND, p->pWhere, pSubWhere);
          pSub->pWhere = 0;
        }
        pSub->pSrc = 0;
        sqlite3ParserAddCleanup(pParse, sqlite3SelectDeleteGeneric, pSub);
        recomputeColumnsUsed(p, &p->pSrc->a[p->pSrc->nSrc-1]);
#જો TREETRACE_ENABLED
        જો( sqlite3_વૃક્ષ_Trace & 0x100000 ){
          TREETRACE(0x100000,pParse,p,
                    ("After EXISTS-to-JOIN optimization:\n"));
          sqlite3_વૃક્ષ_View_Select(0, p, 0);
        }
#endif
      }
    }
  }
}

/*
** Type used for Walker callbacks by selectCheckOnClauses().
*/
પ્રકારવ્યાખ્યા સંરચના CheckOnCtx CheckOnCtx;
સંરચના CheckOnCtx {
  SrcList *pSrc;       /* SrcList for this context */
  પૂર્ણાંક iJoin;           /* Cursors must be left of this one, if not negative */
  પૂર્ણાંક bFuncArg;        /* True for table-function arg */
  CheckOnCtx *pParent; /* Parent context */
};

/*
** True if the SrcList passed as the only argument contains at least
** one RIGHT or FULL JOIN. False otherwise.  
*/
#define has_જમણું_Join(pSrc) (((pSrc)->a[0].fg.jointype & JT_LTORJ)!=0)

/*
** The xExpr callback for the search of invalid ON clause terms.
*/
સ્થિર પૂર્ણાંક select_Check_On_Clauses_પદાવલિ(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;

  /* Check if pExpr is root or near-root of an ON clause constraint that needs
  ** to be checked to ensure that it does not refer to tables in its FROM
  ** clause to the right of itself. i.e. it is either:
  **
  **   + an ON clause on an OUTER join, or
  **   + an ON clause on an INNER join within a FROM that features at
  **     least one RIGHT or FULL join.
  */
  જો( (પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON))
   || (પદાવલિ_Has_Property(p_પદાવલિ, EP_InnerON) && has_જમણું_Join(pCtx->pSrc))
  ){
    /* If CheckOnCtx.iJoin is already set, then fall through and process
    ** this expression node as normal. Or, if CheckOnCtx.iJoin is still 0,
    ** set it to the cursor number of the RHS of the join to which this
    ** ON expression was attached and then iterate through the entire 
    ** expression.  */
    પ્રતિજ્ઞા( pCtx->iJoin<0 || pCtx->iJoin==p_પદાવલિ->w.iJoin );
    જો( pCtx->iJoin<0 ){
      pCtx->iJoin = p_પદાવલિ->w.iJoin;
      sqlite3_Walk_પદાવલિ_NN(pWalker, p_પદાવલિ);
      pCtx->iJoin = -1;
      પાછા WRC_Prune;
    }
  }

  જો( p_પદાવલિ->op==TK_COLUMN ){
    /* A column expression. Find the SrcList (if any) to which it refers.
    ** Then, if CheckOnCtx.iJoin indicates that this expression is part of an
    ** ON clause from that SrcList (i.e. if iJoin is non-zero), check that it
    ** does not refer to a table to the right of CheckOnCtx.iJoin. */
    પૂર્ણાંક iTab = p_પદાવલિ->iTable;
    કરો {
      SrcList *pSrc = pCtx->pSrc;
      પૂર્ણાંક nSrc = pSrc->nSrc;
      પૂર્ણાંક ii;
      માટે(ii=0; ii<nSrc && pSrc->a[ii].iCursor!=iTab; ii++){}
      જો( ii<nSrc ){
        /* pSrc is the FROM clause that contains iTab */
        જો( pCtx->iJoin>=0 ){
          માટે(ii--; ii>=0 && pSrc->a[ii].iCursor!=pCtx->iJoin; ii--){}
          જો( ii>=0 ){
            /* Table iJoin appears to the left of table iTab in the SrcList.
            ** Therefore the expression refers to a table to its right. */
            sqlite3_Error_સંદેશ(pWalker->pParse, 
                "%s references tables to its right",
                (pCtx->bFuncArg ? "table-function argument" : "ON clause")
            );
            પાછા WRC_Abort;
          }
        }
        તોડો;
      }
      pCtx = pCtx->pParent;
    }જ્યારે( pCtx );
  }
  પાછા WRC_Continue;
}

/*
** The xSelect callback for the search of invalid ON clause terms.
*/
સ્થિર પૂર્ણાંક selectCheckOnClausesSelect(Walker *pWalker, Select *pSelect){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;
  જો( pSelect->pSrc==pCtx->pSrc || pSelect->pSrc->nSrc==0 ){
    પાછા WRC_Continue;
  }નહીંતર{
    CheckOnCtx sCtx;
    memset(&sCtx, 0, કદ(sCtx));
    sCtx.pSrc = pSelect->pSrc;
    sCtx.pParent = pCtx;
    sCtx.iJoin = -1;
    pWalker->u.pCheckOnCtx = &sCtx;
    sqlite3WalkSelect(pWalker, pSelect);
    pWalker->u.pCheckOnCtx = pCtx;
    pSelect->selFlags &= ~SF_OnToWhere;
    પાછા WRC_Prune;
  }
}

/*
** Check all ON clauses in pSelect to verify that they do not reference
** columns to the right.
*/
શૂન્ય sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect){
  Walker w;
  CheckOnCtx sCtx;
  પૂર્ણાંક ii;
  પ્રતિજ્ઞા( pSelect->selFlags & SF_OnToWhere );
  પ્રતિજ્ઞા( pSelect->pSrc!=0 && pSelect->pSrc->nSrc>=2 );
  memset(&w, 0, કદ(w));
  w.pParse = pParse;
  w.x_પદાવલિ_Callback = select_Check_On_Clauses_પદાવલિ;
  w.xSelectCallback = selectCheckOnClausesSelect;
  w.u.pCheckOnCtx = &sCtx;
  memset(&sCtx, 0, કદ(sCtx));
  sCtx.pSrc = pSelect->pSrc;
  sCtx.iJoin = -1;
  sqlite3_Walk_પદાવલિ(&w, pSelect->pWhere);
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
  માટે(ii=0; ii<pSelect->pSrc->nSrc; ii++){
    SrcItem *pItem = &pSelect->pSrc->a[ii];
    જો( pItem->fg.isTabFunc ){
      sCtx.iJoin = -1;
      જો( (pItem->fg.jointype & (JT_LTORJ|JT_જમણું)) ){
        sCtx.iJoin = pItem->iCursor;
      }નહીંતર{
        પૂર્ણાંક jj;
        માટે(jj=ii+1; jj<pSelect->pSrc->nSrc; jj++){
          SrcItem *pItem2 = &pSelect->pSrc->a[jj];
          જો( pItem2->fg.jointype & JT_OUTER ){
            sCtx.iJoin = pItem2[-1].iCursor;
            તોડો;
          }
        }
      }
      જો( sCtx.iJoin>=0 ){
        sqlite3_Walk_પદાવલિ_List(&w, pItem->u1.pFuncArg);
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
સ્થિર પૂર્ણાંક sqlite3CopySortOrder(પદાવલિ_List *p1, પદાવલિ_List *p2){
  પ્રતિજ્ઞા( p1 );
  જો( p2 && p1->n_પદાવલિ==p2->n_પદાવલિ ){
    પૂર્ણાંક ii;
    માટે(ii=0; ii<p1->n_પદાવલિ; ii++){
      u8 sortFlags;
      sortFlags = p2->a[ii].fg.sortFlags & KEYINFO_ORDER_DESC;
      p1->a[ii].fg.sortFlags = sortFlags;
    }
    પાછા 1;
  }નહીંતર{
    પાછા 0;
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
પૂર્ણાંક sqlite3Select(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  SelectDest *pDest      /* What to do with the query results */
){
  પૂર્ણાંક i, j;              /* Loop counters */
  WhereInfo *pWInfo;     /* Return from sqlite3WhereBegin() */
  Vdbe *v;               /* The virtual machine under construction */
  પૂર્ણાંક isAgg;             /* True for select lists like "count(*)" */
  પદાવલિ_List *pEList = 0;  /* List of columns to extract. */
  SrcList *pTabList;     /* List of tables to select from */
  પદાવલિ *pWhere;          /* The WHERE clause.  May be NULL */
  પદાવલિ_List *pGroupBy;    /* The GROUP BY clause.  May be NULL */
  પદાવલિ *pHaving;         /* The HAVING clause.  May be NULL */
  AggInfo *pAggInfo = 0; /* Aggregate information */
  પૂર્ણાંક rc = 1;            /* Value to return from this function */
  DistinctCtx sDistinct; /* Info on how to code the DISTINCT keyword */
  SortCtx sSort;         /* Info on how to code the ORDER BY clause */
  પૂર્ણાંક iEnd;              /* Address of the end of the query */
  sqlite3 *db;           /* The database connection */
  પદાવલિ_List *pMinMaxOrderBy = 0;  /* Added ORDER BY for min/max queries */
  u8 minMaxFlag;                 /* Flag for min/max queries */

  db = pParse->db;
  પ્રતિજ્ઞા( pParse==db->pParse );
  v = sqlite3GetVdbe(pParse);
  જો( p==0 || pParse->nErr ){
    પાછા 1;
  }
  પ્રતિજ્ઞા( db->mallocFailed==0 );
  જો( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) પાછા 1;
#જો TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p, ("begin processing:\n", pParse->addrExplain));
  જો( sqlite3_વૃક્ષ_Trace & 0x10000 ){
    જો( (sqlite3_વૃક્ષ_Trace & 0x10001)==0x10000 ){
      sqlite3_વૃક્ષ_View_લીટી(0, "In sqlite3Select() at %s:%d",
                           __FILE__, __LINE__);
    }
    sqlite3ShowSelect(p);
  }
#endif

  /* tag-select-0100 */
  પ્રતિજ્ઞા( p->pOrderBy==0 || pDest->eDest!=SRT_DistFifo );
  પ્રતિજ્ઞા( p->pOrderBy==0 || pDest->eDest!=SRT_Fifo );
  પ્રતિજ્ઞા( p->pOrderBy==0 || pDest->eDest!=SRT_DistQueue );
  પ્રતિજ્ઞા( p->pOrderBy==0 || pDest->eDest!=SRT_Queue );
  જો( IgnorableDistinct(pDest) ){
    પ્રતિજ્ઞા(pDest->eDest==SRT_Exists     || pDest->eDest==SRT_Discard ||
           pDest->eDest==SRT_DistQueue  || pDest->eDest==SRT_DistFifo );
    /* All of these destinations are also able to ignore the ORDER BY clause */
    જો( p->pOrderBy ){
#જો TREETRACE_ENABLED
      TREETRACE(0x800,pParse,p, ("dropping superfluous ORDER BY:\n"));
      જો( sqlite3_વૃક્ષ_Trace & 0x800 ){
        sqlite3_વૃક્ષ_View_પદાવલિ_List(0, p->pOrderBy, 0, "ORDERBY");
      }
#endif   
      sqlite3ParserAddCleanup(pParse, sqlite3_પદાવલિ_List_Delete_Generic,
                              p->pOrderBy);
      testcase( pParse->earlyCleanup );
      p->pOrderBy = 0;
    }
    p->selFlags &= ~(u32)SF_Distinct;
  }
  sqlite3SelectPrep(pParse, p, 0);
  જો( pParse->nErr ){
    જાઓ select_end;
  }
  પ્રતિજ્ઞા( db->mallocFailed==0 );
  પ્રતિજ્ઞા( p->pEList!=0 );
#જો TREETRACE_ENABLED
  જો( sqlite3_વૃક્ષ_Trace & 0x10 ){
    TREETRACE(0x10,pParse,p, ("after name resolution:\n"));
    sqlite3_વૃક્ષ_View_Select(0, p, 0);
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
  જો( p->selFlags & SF_UFSrcCheck ){
    SrcItem *p0 = &p->pSrc->a[0];
    જો( sameSrcAlias(p0, p->pSrc) ){
      sqlite3_Error_સંદેશ(pParse,
          "target object/alias may not appear in FROM clause: %s",
          p0->zAlias ? p0->zAlias : p0->pSTab->zName
      );
      જાઓ select_end;
    }

    /* Clear the SF_UFSrcCheck flag. The check has already been performed,
    ** and leaving this flag set can cause errors if a compound sub-query
    ** in p->pSrc is flattened into this query and this function called
    ** again as part of compound SELECT processing.  */
    p->selFlags &= ~(u32)SF_UFSrcCheck;
  }

  જો( pDest->eDest==SRT_Output ){
    sqlite3GenerateColumnNames(pParse, p);
  }

#ifndef SQLITE_OMIT_WINDOWFUNC
  જો( sqlite3WindowRewrite(pParse, p) ){
    પ્રતિજ્ઞા( pParse->nErr );
    જાઓ select_end;
  }
#જો TREETRACE_ENABLED
  જો( p->pWin && (sqlite3_વૃક્ષ_Trace & 0x40)!=0 ){
    TREETRACE(0x40,pParse,p, ("after window rewrite:\n"));
    sqlite3_વૃક્ષ_View_Select(0, p, 0);
  }
#endif
#endif /* SQLITE_OMIT_WINDOWFUNC */
  pTabList = p->pSrc;
  isAgg = (p->selFlags & SF_Aggregate)!=0;
  memset(&sSort, 0, કદ(sSort));
  sSort.pOrderBy = p->pOrderBy;

  /* Try to do various optimizations (flattening subqueries, and strength
  ** reduction of join operators) in the FROM clause up into the main query
  ** tag-select-0200
  */
#જો !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
  માટે(i=0; !p->pPrior && i<pTabList->nSrc; i++){
    SrcItem *pItem = &pTabList->a[i];
    Select *pSub = pItem->fg.isSubquery ? pItem->u4.pSubq->pSelect : 0;
    Table *pTab = pItem->pSTab;

    /* The expander should have already created transient Table objects
    ** even for FROM clause elements such as subqueries that do not correspond
    ** to a real table */
    પ્રતિજ્ઞા( pTab!=0 );

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
    જો( (pItem->fg.jointype & (JT_ડાબું|JT_LTORJ))!=0
     && sqlite3_પદાવલિ_Implies_Non_Null_Row(p->pWhere, pItem->iCursor,
                                     pItem->fg.jointype & JT_LTORJ)
     && OptimizationEnabled(db, SQLITE_SimplifyJoin)
    ){
      જો( pItem->fg.jointype & JT_ડાબું ){
        જો( pItem->fg.jointype & JT_જમણું ){
          TREETRACE(0x1000,pParse,p,
                    ("FULL-JOIN simplifies to RIGHT-JOIN on term %d\n",i));
          pItem->fg.jointype &= ~JT_ડાબું;
        }નહીંતર{
          TREETRACE(0x1000,pParse,p,
                    ("LEFT-JOIN simplifies to JOIN on term %d\n",i));
          pItem->fg.jointype &= ~(JT_ડાબું|JT_OUTER);
          unset_Join_પદાવલિ(p->pWhere, pItem->iCursor, 0);
        }
      }
      જો( pItem->fg.jointype & JT_LTORJ ){
        માટે(j=i+1; j<pTabList->nSrc; j++){
          SrcItem *pI2 = &pTabList->a[j];
          જો( pI2->fg.jointype & JT_જમણું ){
            જો( pI2->fg.jointype & JT_ડાબું ){
              TREETRACE(0x1000,pParse,p,
                        ("FULL-JOIN simplifies to LEFT-JOIN on term %d\n",j));
              pI2->fg.jointype &= ~JT_જમણું;
            }નહીંતર{
              TREETRACE(0x1000,pParse,p,
                        ("RIGHT-JOIN simplifies to JOIN on term %d\n",j));
              pI2->fg.jointype &= ~(JT_જમણું|JT_OUTER);
              unset_Join_પદાવલિ(p->pWhere, pI2->iCursor, 1);
            }
          }
        }
        માટે(j=pTabList->nSrc-1; j>=0; j--){
          pTabList->a[j].fg.jointype &= ~JT_LTORJ;
          જો( pTabList->a[j].fg.jointype & JT_જમણું ) તોડો;
        }
      }
    }

    /* No further action if this term of the FROM clause is not a subquery */
    જો( pSub==0 ) ચાલુ;

    /* Catch mismatch in the declared columns of a view and the number of
    ** columns in the SELECT on the RHS */
    જો( pTab->nCol!=pSub->pEList->n_પદાવલિ ){
      sqlite3_Error_સંદેશ(pParse, "expected %d columns for '%s' but got %d",
                      pTab->nCol, pTab->zName, pSub->pEList->n_પદાવલિ);
      જાઓ select_end;
    }

    /* Do not attempt the usual optimizations (flattening and ORDER BY
    ** elimination) on a MATERIALIZED common table expression because
    ** a MATERIALIZED common table expression is an optimization fence.
    */
    જો( pItem->fg.isCte && pItem->u2.pCteUse->eM10d==M10d_Yes ){
      ચાલુ;
    }

    /* Do not try to flatten an aggregate subquery.
    **
    ** Flattening an aggregate subquery is only possible if the outer query
    ** is not a join.  But if the outer query is not a join, then the subquery
    ** will be implemented as a co-routine and there is no advantage to
    ** flattening in that case.
    */
    જો( (pSub->selFlags & SF_Aggregate)!=0 ) ચાલુ;
    પ્રતિજ્ઞા( pSub->pGroupBy==0 );

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
    જો( pSub->pOrderBy!=0
     && (p->pOrderBy!=0 || pTabList->nSrc>1)      /* Condition (5) */
     && pSub->pLimit==0                           /* Condition (1) */
     && (pSub->selFlags & (SF_OrderByReqd|SF_Recursive))==0  /* (2) and (6) */
     && (p->selFlags & SF_OrderByReqd)==0         /* Condition (3) and (4) */
     && OptimizationEnabled(db, SQLITE_OmitOrderBy)
    ){
      TREETRACE(0x800,pParse,p,
                ("omit superfluous ORDER BY on %r FROM-clause subquery\n",i+1));
      sqlite3ParserAddCleanup(pParse, sqlite3_પદાવલિ_List_Delete_Generic,
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
    જો( pSub->pOrderBy!=0
     && i==0
     && (p->selFlags & SF_ComplexResult)!=0
     && (pTabList->nSrc==1
         || (pTabList->a[1].fg.jointype&(JT_OUTER|JT_CROSS))!=0)
    ){
      ચાલુ;
    }

    /* tag-select-0240 */
    જો( flattenSubquery(pParse, p, i, isAgg) ){
      જો( pParse->nErr ) જાઓ select_end;
      /* This subquery can be absorbed into its parent. */
      i = -1;
    }
    pTabList = p->pSrc;
    જો( db->mallocFailed ) જાઓ select_end;
    જો( !IgnorableOrderby(pDest) ){
      sSort.pOrderBy = p->pOrderBy;
    }
  }
#endif

#ifndef SQLITE_OMIT_COMPOUND_SELECT
  /* Handle compound SELECT statements using the separate multiSelect()
  ** procedure.  tag-select-0300
  */
  જો( p->pPrior ){
    rc = multiSelect(pParse, p, pDest);
#જો TREETRACE_ENABLED
    TREETRACE(0x400,pParse,p,("end compound-select processing\n"));
    જો( (sqlite3_વૃક્ષ_Trace & 0x400)!=0 && ExplainQueryPlanParent(pParse)==0 ){
      sqlite3_વૃક્ષ_View_Select(0, p, 0);
    }
#endif
    જો( p->pNext==0 ) Explain_Query_Plan_કાઢો(pParse);
    પાછા rc;
  }
#endif

  /* If there may be an "EXISTS (SELECT ...)" in the WHERE clause, attempt
  ** to change it into a join.  */
  જો( pParse->bHasExists && OptimizationEnabled(db,SQLITE_ExistsToJoin) ){
    existsToJoin(pParse, p, p->pWhere);
    pTabList = p->pSrc;
  }

  /* Do the WHERE-clause constant propagation optimization if this is
  ** a join.  No need to spend time on this operation for non-join queries
  ** as the equivalent optimization will be handled by query planner in
  ** sqlite3WhereBegin().  tag-select-0330
  */
  જો( p->pWhere!=0
   && p->pWhere->op==TK_AND
   && OptimizationEnabled(db, SQLITE_PropagateConst)
   && propagateConstants(pParse, p)
  ){
#જો TREETRACE_ENABLED
    જો( sqlite3_વૃક્ષ_Trace & 0x2000 ){
      TREETRACE(0x2000,pParse,p,("After constant propagation:\n"));
      sqlite3_વૃક્ષ_View_Select(0, p, 0);
    }
#endif
  }નહીંતર{
    TREETRACE(0x2000,pParse,p,("Constant propagation not helpful\n"));
  }

  /* tag-select-0350 */
  જો( OptimizationEnabled(db, SQLITE_QueryFlattener|SQLITE_ગણતરી_Of_View)
   && ગણતરી_Of_View_Optimization(pParse, p)
  ){
    જો( db->mallocFailed ) જાઓ select_end;
    pTabList = p->pSrc;
  }

  /* Loop over all terms in the FROM clause and do two things for each term:
  **
  **   (1) Authorize unreferenced tables
  **   (2) Generate code for all sub-queries
  **
  ** tag-select-0400
  */
  માટે(i=0; i<pTabList->nSrc; i++){
    SrcItem *pItem = &pTabList->a[i];
    SrcItem *pPrior;
    SelectDest dest;
    Subquery *pSubq;
    Select *pSub;
#જો !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    અચલ અક્ષર *zSavedAuthContext;
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
    જો( pItem->colUsed==0 && pItem->zName!=0 ){
      અચલ અક્ષર *zDb;
      જો( pItem->fg.fixedSchema ){
        પૂર્ણાંક iDb = sqlite3SchemaToIndex(pParse->db, pItem->u4.pSchema);
        zDb = db->aDb[iDb].zDbSName;
      }નહીંતર જો( pItem->fg.isSubquery ){
        zDb = 0;
      }નહીંતર{
        zDb = pItem->u4.zDatabase;
      }
      sqlite3AuthCheck(pParse, SQLITE_READ, pItem->zName, "", zDb);
    }

#જો !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    /* Generate code for all sub-queries in the FROM clause
    */
    જો( pItem->fg.isSubquery==0 ) ચાલુ;
    pSubq = pItem->u4.pSubq;
    પ્રતિજ્ઞા( pSubq!=0 );
    pSub = pSubq->pSelect;

    /* The code for a subquery should only be generated once. */
    જો( pSubq->addrFillSub!=0 ) ચાલુ;

    /* Increment Parse.nHeight by the height of the largest expression
    ** tree referred to by this, the parent select. The child select
    ** may contain expression trees of at most
    ** (SQLITE_MAX_EXPR_DEPTH-Parse.nHeight) height. This is a bit
    ** more conservative than necessary, but much easier than enforcing
    ** an exact limit.
    */
    pParse->nHeight += sqlite3_Select_પદાવલિ_Height(p);

    /* Make copies of constant WHERE-clause terms in the outer query down
    ** inside the subquery.  This can help the subquery to run more efficiently.
    ** This is the "predicate push-down optimization".  tag-select-0420
    */
    જો( OptimizationEnabled(db, SQLITE_ધકેલો_Down)
     && (pItem->fg.isCte==0
         || (pItem->u2.pCteUse->eM10d!=M10d_Yes && pItem->u2.pCteUse->nUse<2))
     && ધકેલો_Down_Where_Terms(pParse, pSub, p->pWhere, pTabList, i)
    ){
#જો TREETRACE_ENABLED
      જો( sqlite3_વૃક્ષ_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("After WHERE-clause push-down into subquery %d:\n", pSub->selId));
        sqlite3_વૃક્ષ_View_Select(0, p, 0);
      }
#endif
      પ્રતિજ્ઞા( pSubq->pSelect && (pSub->selFlags & SF_ધકેલો_Down)!=0 );
    }નહીંતર{
      TREETRACE(0x4000,pParse,p,("WHERE-clause push-down not possible\n"));
    }

    /* Convert unused result columns of the subquery into simple NULL
    ** expressions, to avoid unneeded searching and computation.
    ** tag-select-0440
    */
    જો( OptimizationEnabled(db, SQLITE_NullUnusedCols)
     && disableUnusedSubqueryResultColumns(pItem)
    ){
#જો TREETRACE_ENABLED
      જો( sqlite3_વૃક્ષ_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("Change unused result columns to NULL for subquery %d:\n",
             pSub->selId));
        sqlite3_વૃક્ષ_View_Select(0, p, 0);
      }
#endif
    }

    zSavedAuthContext = pParse->zAuthContext;
    pParse->zAuthContext = pItem->zName;

    /* Generate byte-code to implement the subquery  tag-select-0480
    */
    જો( fromClauseTermCanBeCoroutine(pParse, pTabList, i, p->selFlags) ){
      /* Implement a co-routine that will return a single row of the result
      ** set on each invocation.  tag-select-0482
      */
      પૂર્ણાંક addr_ટોચ = sqlite3VdbeCurrentAddr(v)+1;
    
      pSubq->regReturn = ++pParse->nMem;
      sqlite3VdbeAddOp3(v, OP_InitCoroutine, pSubq->regReturn, 0, addr_ટોચ);
      VdbeComment((v, "%!S", pItem));
      pSubq->addrFillSub = addr_ટોચ;
      sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);
      ExplainQueryPlan((pParse, 1, "CO-ROUTINE %!S", pItem));
      sqlite3Select(pParse, pSub, &dest);
      pItem->pSTab->nRowLogEst = pSub->nSelectRow;
      pItem->fg.viaCoroutine = 1;
      pSubq->regResult = dest.iSdst;
      sqlite3VdbeEndCoroutine(v, pSubq->regReturn);
      VdbeComment((v, "end %!S", pItem));
      sqlite3VdbeJumpHere(v, addr_ટોચ-1);
      sqlite3ClearTempRegCache(pParse);
    }નહીંતર જો( pItem->fg.isCte && pItem->u2.pCteUse->addrM9e>0 ){
      /* This is a CTE for which materialization code has already been
      ** generated.  Invoke the subroutine to compute the materialization,
      ** then make the pItem->iCursor be a copy of the ephemeral table that
      ** holds the result of the materialization. tag-select-0484 */
      CteUse *pCteUse = pItem->u2.pCteUse;
      sqlite3VdbeAddOp2(v, OP_Gosub, pCteUse->regRtn, pCteUse->addrM9e);
      જો( pItem->iCursor!=pCteUse->iCur ){
        sqlite3VdbeAddOp2(v, OP_OpenDup, pItem->iCursor, pCteUse->iCur);
        VdbeComment((v, "%!S", pItem));
      }
      pSub->nSelectRow = pCteUse->nRowEst;
    }નહીંતર જો( (pPrior = isSelfJoinView(pTabList, pItem, 0, i))!=0 ){
      /* This view has already been materialized by a prior entry in
      ** this same FROM clause.  Reuse it.  tag-select-0486 */
      Subquery *pPriorSubq;
      પ્રતિજ્ઞા( pPrior->fg.isSubquery );
      pPriorSubq = pPrior->u4.pSubq;
      પ્રતિજ્ઞા( pPriorSubq!=0 );
      જો( pPriorSubq->addrFillSub ){
        sqlite3VdbeAddOp2(v, OP_Gosub, pPriorSubq->regReturn,
                                       pPriorSubq->addrFillSub);
      }
      sqlite3VdbeAddOp2(v, OP_OpenDup, pItem->iCursor, pPrior->iCursor);
      pSub->nSelectRow = pPriorSubq->pSelect->nSelectRow;
    }નહીંતર{
      /* Materialize the view.  If the view is not correlated, generate a
      ** subroutine to do the materialization so that subsequent uses of
      ** the same view can reuse the materialization.  tag-select-0488 */
      પૂર્ણાંક ટોચ_Addr;
      પૂર્ણાંક onceAddr = 0;
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
      પૂર્ણાંક addrExplain;
#endif

      pSubq->regReturn = ++pParse->nMem;
      ટોચ_Addr = sqlite3VdbeAddOp0(v, OP_Goto);
      pSubq->addrFillSub = ટોચ_Addr+1;
      pItem->fg.isMaterialized = 1;
      જો( pItem->fg.isCorrelated==0 ){
        /* If the subquery is not correlated and if we are not inside of
        ** a trigger, then we only need to compute the value of the subquery
        ** once. */
        onceAddr = sqlite3VdbeAddOp0(v, OP_Once); VdbeCoverage(v);
        VdbeComment((v, "materialize %!S", pItem));
      }નહીંતર{
        VdbeNoopComment((v, "materialize %!S", pItem));
      }
      sqlite3SelectDestInit(&dest, SRT_EphemTab, pItem->iCursor);

      ExplainQueryPlan2(addrExplain, (pParse, 1, "MATERIALIZE %!S", pItem));
      sqlite3Select(pParse, pSub, &dest);
      pItem->pSTab->nRowLogEst = pSub->nSelectRow;
      જો( onceAddr ) sqlite3VdbeJumpHere(v, onceAddr);
      sqlite3VdbeAddOp2(v, OP_Return, pSubq->regReturn, ટોચ_Addr+1);
      VdbeComment((v, "end %!S", pItem));
      sqlite3VdbeScanStatusRange(v, addrExplain, addrExplain, -1);
      sqlite3VdbeJumpHere(v, ટોચ_Addr);
      sqlite3ClearTempRegCache(pParse);
      જો( pItem->fg.isCte && pItem->fg.isCorrelated==0 ){
        CteUse *pCteUse = pItem->u2.pCteUse;
        pCteUse->addrM9e = pSubq->addrFillSub;
        pCteUse->regRtn = pSubq->regReturn;
        pCteUse->iCur = pItem->iCursor;
        pCteUse->nRowEst = pSub->nSelectRow;
      }
    }
    જો( db->mallocFailed ) જાઓ select_end;
    pParse->nHeight -= sqlite3_Select_પદાવલિ_Height(p);
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

#જો TREETRACE_ENABLED
  જો( sqlite3_વૃક્ષ_Trace & 0x8000 ){
    TREETRACE(0x8000,pParse,p,("After all FROM-clause analysis:\n"));
    sqlite3_વૃક્ષ_View_Select(0, p, 0);
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
  જો( (p->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct
   && sqlite3CopySortOrder(pEList, sSort.pOrderBy)
   && sqlite3_પદાવલિ_List_Compare(pEList, sSort.pOrderBy, -1)==0
   && OptimizationEnabled(db, SQLITE_GroupByOrder)
#ifndef SQLITE_OMIT_WINDOWFUNC
   && p->pWin==0
#endif
  ){
    p->selFlags &= ~(u32)SF_Distinct;
    pGroupBy = p->pGroupBy = sqlite3_પદાવલિ_List_Dup(db, pEList, 0);
    જો( pGroupBy ){
      માટે(i=0; i<pGroupBy->n_પદાવલિ; i++){
        pGroupBy->a[i].u.x.iOrderByCol = i+1;
      }
    }
    p->selFlags |= SF_Aggregate;
    /* Notice that even thought SF_Distinct has been cleared from p->selFlags,
    ** the sDistinct.isTnct is still set.  Hence, isTnct represents the
    ** original setting of the SF_Distinct flag, not the current setting */
    પ્રતિજ્ઞા( sDistinct.isTnct );
    sDistinct.isTnct = 2;

#જો TREETRACE_ENABLED
    જો( sqlite3_વૃક્ષ_Trace & 0x20000 ){
      TREETRACE(0x20000,pParse,p,("Transform DISTINCT into GROUP BY:\n"));
      sqlite3_વૃક્ષ_View_Select(0, p, 0);
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
  જો( sSort.pOrderBy ){
    ચાવી_Info *p_ચાવી_Info;
    p_ચાવી_Info = sqlite3_ચાવી_Info_From_પદાવલિ_List(
        pParse, sSort.pOrderBy, 0, pEList->n_પદાવલિ);
    sSort.iECursor = pParse->nTab++;
    sSort.addrSortIndex =
      sqlite3VdbeAddOp4(v, OP_OpenEphemeral,
          sSort.iECursor, sSort.pOrderBy->n_પદાવલિ+1+pEList->n_પદાવલિ, 0,
          (અક્ષર*)p_ચાવી_Info, P4_KEYINFO
      );
  }નહીંતર{
    sSort.addrSortIndex = -1;
  }

  /* If the output is destined for a temporary table, open that table.
  ** tag-select-0630
  */
  જો( pDest->eDest==SRT_EphemTab ){
    sqlite3VdbeAddOp2(v, OP_OpenEphemeral, pDest->iSDParm, pEList->n_પદાવલિ);
    જો( p->selFlags & SF_NestedFrom ){
      /* Delete or NULL-out result columns that will never be used */
      પૂર્ણાંક ii;
      માટે(ii=pEList->n_પદાવલિ-1; ii>0 && pEList->a[ii].fg.bUsed==0; ii--){
        sqlite3_પદાવલિ_Delete(db, pEList->a[ii].p_પદાવલિ);
        sqlite3DbFree(db, pEList->a[ii].zEName);
        pEList->n_પદાવલિ--;
      }
      માટે(ii=0; ii<pEList->n_પદાવલિ; ii++){
        જો( pEList->a[ii].fg.bUsed==0 ) pEList->a[ii].p_પદાવલિ->op = TK_NULL;
      }
    }
  }

  /* Set the limiter.  tag-select-0650
  */
  iEnd = sqlite3VdbeMakeLabel(pParse);
  જો( (p->selFlags & SF_FixedLimit)==0 ){
    p->nSelectRow = 320;  /* 4 billion rows */
  }
  જો( p->pLimit ) computeLimitRegisters(pParse, p, iEnd);
  જો( p->iLimit==0 && sSort.addrSortIndex>=0 ){
    sqlite3VdbeChangeOpcode(v, sSort.addrSortIndex, OP_SorterOpen);
    sSort.sortFlags |= SORTFLAG_UseSorter;
  }

  /* Open an ephemeral index to use for the distinct set. tag-select-0680
  */
  જો( p->selFlags & SF_Distinct ){
    sDistinct.tabTnct = pParse->nTab++;
    sDistinct.addrTnct = sqlite3VdbeAddOp4(v, OP_OpenEphemeral,
                       sDistinct.tabTnct, 0, 0,
                       (અક્ષર*)sqlite3_ચાવી_Info_From_પદાવલિ_List(pParse, p->pEList,0,0),
                       P4_KEYINFO);
    sqlite3VdbeChangeP5(v, BTREE_UNORDERED);
    sDistinct.eTnctType = WHERE_DISTINCT_UNORDERED;
  }નહીંતર{
    sDistinct.eTnctType = WHERE_DISTINCT_NOOP;
  }

  જો( !isAgg && pGroupBy==0 ){
    /* No aggregate functions and no GROUP BY clause.  tag-select-0700 */
    u16 wctrlFlags = (sDistinct.isTnct ? WHERE_WANT_DISTINCT : 0)
                   | (p->selFlags & SF_FixedLimit);
#ifndef SQLITE_OMIT_WINDOWFUNC
    Window *pWin = p->pWin;      /* Main window object (or NULL) */
    જો( pWin ){
      sqlite3WindowCodeInit(pParse, p);
    }
#endif
    પ્રતિજ્ઞા( WHERE_USE_LIMIT==SF_FixedLimit );


    /* Begin the database scan. */
    TREETRACE(0x2,pParse,p,("WhereBegin\n"));
    pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, sSort.pOrderBy,
                               p->pEList, p, wctrlFlags, p->nSelectRow);
    જો( pWInfo==0 ) જાઓ select_end;
    જો( sqlite3_Where_Output_Row_ગણતરી(pWInfo) < p->nSelectRow ){
      p->nSelectRow = sqlite3_Where_Output_Row_ગણતરી(pWInfo);
      જો( pDest->eDest<=SRT_DistQueue && pDest->eDest>=SRT_DistFifo ){
        /* TUNING: For a UNION CTE, because UNION is implies DISTINCT,
        ** reduce the estimated output row count by 8 (LogEst 30). 
        ** Search for tag-20250414a to see other cases */
        p->nSelectRow -= 30;
      }
    }
    જો( sDistinct.isTnct && sqlite3WhereIsDistinct(pWInfo) ){
      sDistinct.eTnctType = sqlite3WhereIsDistinct(pWInfo);
    }
    જો( sSort.pOrderBy ){
      sSort.nOBSat = sqlite3WhereIsOrdered(pWInfo);
      sSort.labelOBLopt = sqlite3WhereOrderByLimitOptLabel(pWInfo);
      જો( sSort.nOBSat==sSort.pOrderBy->n_પદાવલિ ){
        sSort.pOrderBy = 0;
      }
    }
    TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));

    /* If sorting index that was created by a prior OP_OpenEphemeral
    ** instruction ended up not being needed, then change the OP_OpenEphemeral
    ** into an OP_Noop.
    */
    જો( sSort.addrSortIndex>=0 && sSort.pOrderBy==0 ){
      sqlite3VdbeChangeToNoop(v, sSort.addrSortIndex);
    }

    પ્રતિજ્ઞા( p->pEList==pEList );
#ifndef SQLITE_OMIT_WINDOWFUNC
    જો( pWin ){
      પૂર્ણાંક addrGosub = sqlite3VdbeMakeLabel(pParse);
      પૂર્ણાંક iCont = sqlite3VdbeMakeLabel(pParse);
      પૂર્ણાંક iBreak = sqlite3VdbeMakeLabel(pParse);
      પૂર્ણાંક regGosub = ++pParse->nMem;

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
    }નહીંતર
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
  }નહીંતર{
    /* This case is for when there exist aggregate functions or a GROUP BY
    ** clause or both.  tag-select-0800 */
    NameContext sNC;    /* Name context for processing aggregate information */
    પૂર્ણાંક iAMem;          /* First Mem address for storing current GROUP BY */
    પૂર્ણાંક iBMem;          /* First Mem address for previous GROUP BY */
    પૂર્ણાંક iUseFlag;       /* Mem address holding flag indicating that at least
                        ** one row of the input to the aggregator has been
                        ** processed */
    પૂર્ણાંક iAbortFlag;     /* Mem address which causes query abort if positive */
    પૂર્ણાંક groupBySort;    /* Rows come from source in GROUP BY order */
    પૂર્ણાંક addrEnd;        /* End of processing for this SELECT */
    પૂર્ણાંક sortPTab = 0;   /* Pseudotable used to decode sorting results */
    પૂર્ણાંક sortOut = 0;    /* Output register from the sorter */
    પૂર્ણાંક orderByGrp = 0; /* True if the GROUP BY and ORDER BY are the same */

    /* Remove any and all aliases between the result set and the
    ** GROUP BY clause.
    */
    જો( pGroupBy ){
      પૂર્ણાંક k;                        /* Loop counter */
      સંરચના પદાવલિ_List_item *pItem;  /* For looping over expression in a list */

      માટે(k=p->pEList->n_પદાવલિ, pItem=p->pEList->a; k>0; k--, pItem++){
        pItem->u.x.iAlias = 0;
      }
      માટે(k=pGroupBy->n_પદાવલિ, pItem=pGroupBy->a; k>0; k--, pItem++){
        pItem->u.x.iAlias = 0;
      }
      પ્રતિજ્ઞા( 66==sqlite3LogEst(100) );
      જો( p->nSelectRow>66 ) p->nSelectRow = 66;

      /* If there is both a GROUP BY and an ORDER BY clause and they are
      ** identical, then it may be possible to disable the ORDER BY clause
      ** on the grounds that the GROUP BY will cause elements to come out
      ** in the correct order. It also may not - the GROUP BY might use a
      ** database index that causes rows to be grouped together as required
      ** but not actually sorted. Either way, record the fact that the
      ** ORDER BY and GROUP BY clauses are the same by setting the orderByGrp
      ** variable.  */
      જો( sqlite3CopySortOrder(pGroupBy, sSort.pOrderBy)
       && sqlite3_પદાવલિ_List_Compare(pGroupBy, sSort.pOrderBy, -1)==0
      ){
        orderByGrp = 1;
      }
    }નહીંતર{
      પ્રતિજ્ઞા( 0==sqlite3LogEst(1) );
      p->nSelectRow = 0;
    }

    /* Create a label to jump to when we want to abort the query */
    addrEnd = sqlite3VdbeMakeLabel(pParse);

    /* Convert TK_COLUMN nodes into TK_AGG_COLUMN and make entries in
    ** sAggInfo for all TK_AGG_FUNCTION nodes in expressions of the
    ** SELECT statement.
    */
    pAggInfo = sqlite3DbMallocZero(db, કદ(*pAggInfo) );
    જો( pAggInfo ){
      sqlite3ParserAddCleanup(pParse, agginfoFree, pAggInfo);
      testcase( pParse->earlyCleanup );
    }
    જો( db->mallocFailed ){
      જાઓ select_end;
    }
    pAggInfo->selId = p->selId;
#ifdef SQLITE_DEBUG
    pAggInfo->pSelect = p;
#endif
    memset(&sNC, 0, કદ(sNC));
    sNC.pParse = pParse;
    sNC.pSrcList = pTabList;
    sNC.uNC.pAggInfo = pAggInfo;
    VVA_ONLY( sNC.ncFlags = NC_UAggInfo; )
    pAggInfo->nSortingColumn = pGroupBy ? pGroupBy->n_પદાવલિ : 0;
    pAggInfo->pGroupBy = pGroupBy;
    sqlite3_પદાવલિ_Analyze_Agg_List(&sNC, pEList);
    sqlite3_પદાવલિ_Analyze_Agg_List(&sNC, sSort.pOrderBy);
    જો( pHaving ){
      જો( pGroupBy ){
        પ્રતિજ્ઞા( pWhere==p->pWhere );
        પ્રતિજ્ઞા( pHaving==p->pHaving );
        પ્રતિજ્ઞા( pGroupBy==p->pGroupBy );
        havingToWhere(pParse, p);
        pWhere = p->pWhere;
      }
      sqlite3_પદાવલિ_Analyze_Aggregates(&sNC, pHaving);
    }
    pAggInfo->nAccumulator = pAggInfo->nColumn;
    જો( p->pGroupBy==0 && p->pHaving==0 && pAggInfo->nFunc==1 ){
      minMaxFlag = minMaxQuery(db, pAggInfo->aFunc[0].pFExpr, &pMinMaxOrderBy);
    }નહીંતર{
      minMaxFlag = WHERE_ORDERBY_NORMAL;
    }
    analyzeAggFuncArgs(pAggInfo, &sNC);
    જો( db->mallocFailed ) જાઓ select_end;
#જો TREETRACE_ENABLED
    જો( sqlite3_વૃક્ષ_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("After aggregate analysis %p:\n", pAggInfo));
      sqlite3_વૃક્ષ_View_Select(0, p, 0);
      જો( minMaxFlag ){
        sqlite3DebugPrintf("MIN/MAX Optimization (0x%02x) adds:\n", minMaxFlag);
        sqlite3_વૃક્ષ_View_પદાવલિ_List(0, pMinMaxOrderBy, 0, "ORDERBY");
      }
      printAggInfo(pAggInfo);
    }
#endif


    /* Processing for aggregates with GROUP BY is very different and
    ** much more complex than aggregates without a GROUP BY.  tag-select-0810
    */
    જો( pGroupBy ){
      ચાવી_Info *p_ચાવી_Info;  /* Keying information for the group by clause */
      પૂર્ણાંક addr1;          /* A-vs-B comparison jump */
      પૂર્ણાંક addrOutputRow;  /* Start of subroutine that outputs a result row */
      પૂર્ણાંક regOutputRow;   /* Return address register for output subroutine */
      પૂર્ણાંક addrSetAbort;   /* Set the abort flag and return */
      પૂર્ણાંક addr_ટોચ_Of_Loop;  /* Top of the input loop */
      પૂર્ણાંક addrSortingIdx; /* The OP_OpenEphemeral for the sorting index */
      પૂર્ણાંક addrReset;      /* Subroutine for resetting the accumulator */
      પૂર્ણાંક regReset;       /* Return address register for reset subroutine */
      પદાવલિ_List *pDistinct = 0;
      u16 distFlag = 0;
      પૂર્ણાંક eDist = WHERE_DISTINCT_NOOP;

      જો( pAggInfo->nFunc==1
       && pAggInfo->aFunc[0].iDistinct>=0
       && ALWAYS(pAggInfo->aFunc[0].pFExpr!=0)
       && ALWAYS(પદાવલિ_Use_XList(pAggInfo->aFunc[0].pFExpr))
       && pAggInfo->aFunc[0].pFExpr->x.pList!=0
      ){
        પદાવલિ *p_પદાવલિ = pAggInfo->aFunc[0].pFExpr->x.pList->a[0].p_પદાવલિ;
        p_પદાવલિ = sqlite3_પદાવલિ_Dup(db, p_પદાવલિ, 0);
        pDistinct = sqlite3_પદાવલિ_List_Dup(db, pGroupBy, 0);
        pDistinct = sqlite3_પદાવલિ_List_Append(pParse, pDistinct, p_પદાવલિ);
        distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
      }

      /* If there is a GROUP BY clause we might need a sorting index to
      ** implement it.  Allocate that sorting index now.  If it turns out
      ** that we do not need it after all, the OP_SorterOpen instruction
      ** will be converted into a Noop. 
      */
      pAggInfo->sortingIdx = pParse->nTab++;
      p_ચાવી_Info = sqlite3_ચાવી_Info_From_પદાવલિ_List(pParse, pGroupBy,
                                            0, pAggInfo->nColumn);
      addrSortingIdx = sqlite3VdbeAddOp4(v, OP_SorterOpen,
          pAggInfo->sortingIdx, pAggInfo->nSortingColumn,
          0, (અક્ષર*)p_ચાવી_Info, P4_KEYINFO);

      /* Initialize memory locations used by GROUP BY aggregate processing
      */
      iUseFlag = ++pParse->nMem;
      iAbortFlag = ++pParse->nMem;
      regOutputRow = ++pParse->nMem;
      addrOutputRow = sqlite3VdbeMakeLabel(pParse);
      regReset = ++pParse->nMem;
      addrReset = sqlite3VdbeMakeLabel(pParse);
      iAMem = pParse->nMem + 1;
      pParse->nMem += pGroupBy->n_પદાવલિ;
      iBMem = pParse->nMem + 1;
      pParse->nMem += pGroupBy->n_પદાવલિ;
      sqlite3VdbeAddOp2(v, OP_Integer, 0, iAbortFlag);
      VdbeComment((v, "clear abort flag"));
      sqlite3VdbeAddOp3(v, OP_Null, 0, iAMem, iAMem+pGroupBy->n_પદાવલિ-1);
      sqlite3_પદાવલિ_Null_Register_Range(pParse, iAMem, pGroupBy->n_પદાવલિ);

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
      જો( pWInfo==0 ){
        sqlite3_પદાવલિ_List_Delete(db, pDistinct);
        જાઓ select_end;
      }
      જો( pParse->pIdxEpr ){
        optimize_Aggregate_Use_Of_Indexed_પદાવલિ(pParse, p, pAggInfo, &sNC);
      }
      assignAggregateRegisters(pParse, pAggInfo);
      eDist = sqlite3WhereIsDistinct(pWInfo);
      TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
      જો( sqlite3WhereIsOrdered(pWInfo)==pGroupBy->n_પદાવલિ ){
        /* The optimizer is able to deliver rows in group by order so
        ** we do not have to sort.  The OP_OpenEphemeral table will be
        ** cancelled later because we still need to use the pKeyInfo
        */
        groupBySort = 0;
      }નહીંતર{
        /* Rows are coming out in undetermined order.  We have to push
        ** each row into a sorting index, terminate the first loop,
        ** then loop over the sorting index in order to get the output
        ** in sorted order
        */
        પૂર્ણાંક regBase;
        પૂર્ણાંક regRecord;
        પૂર્ણાંક nCol;
        પૂર્ણાંક nGroupBy;

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
        પૂર્ણાંક addrExp;              /* Address of OP_Explain instruction */
#endif
        ExplainQueryPlan2(addrExp, (pParse, 0, "USE TEMP B-TREE FOR %s",
            (sDistinct.isTnct && (p->selFlags&SF_Distinct)==0) ?
                    "DISTINCT" : "GROUP BY"
        ));

        groupBySort = 1;
        nGroupBy = pGroupBy->n_પદાવલિ;
        nCol = nGroupBy;
        j = nGroupBy;
        માટે(i=0; i<pAggInfo->nColumn; i++){
          જો( pAggInfo->aCol[i].iSorterColumn>=j ){
            nCol++;
            j++;
          }
        }
        regBase = sqlite3GetTempRange(pParse, nCol);
        sqlite3_પદાવલિ_Code_પદાવલિ_List(pParse, pGroupBy, regBase, 0, 0);
        j = nGroupBy;
        pAggInfo->directMode = 1;
        માટે(i=0; i<pAggInfo->nColumn; i++){
          સંરચના AggInfo_col *pCol = &pAggInfo->aCol[i];
          જો( pCol->iSorterColumn>=j ){
            sqlite3_પદાવલિ_Code(pParse, pCol->pCExpr, j + regBase);
            j++;
          }
        }
        pAggInfo->directMode = 0;
        regRecord = sqlite3GetTempReg(pParse);
        sqlite3VdbeScanStatusCounters(v, addrExp, 0, sqlite3VdbeCurrentAddr(v));
        sqlite3VdbeAddOp3(v, OP_MakeRecord, regBase, nCol, regRecord);
        sqlite3VdbeAddOp2(v, OP_Sorter_ઉમેરો, pAggInfo->sortingIdx, regRecord);
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
      જો( pParse->pIdxEpr ){
        aggregate_Convert_Indexed_પદાવલિ_Ref_To_Column(pAggInfo);
#જો TREETRACE_ENABLED
        જો( sqlite3_વૃક્ષ_Trace & 0x20 ){
          TREETRACE(0x20, pParse, p,
             ("AggInfo function expressions converted to reference index\n"));
          sqlite3_વૃક્ષ_View_Select(0, p, 0);
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
      જો( orderByGrp && OptimizationEnabled(db, SQLITE_GroupByOrder)
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
      addr_ટોચ_Of_Loop = sqlite3VdbeCurrentAddr(v);
      જો( groupBySort ){
        sqlite3VdbeAddOp3(v, OP_SorterData, pAggInfo->sortingIdx,
                          sortOut, sortPTab);
      }
      માટે(j=0; j<pGroupBy->n_પદાવલિ; j++){
        પૂર્ણાંક iOrderByCol = pGroupBy->a[j].u.x.iOrderByCol;
        
        જો( groupBySort ){
          sqlite3VdbeAddOp3(v, OP_Column, sortPTab, j, iBMem+j);
        }નહીંતર{
          pAggInfo->directMode = 1;
          sqlite3_પદાવલિ_Code(pParse, pGroupBy->a[j].p_પદાવલિ, iBMem+j);
        }

        જો( iOrderByCol ){
          પદાવલિ *pX = p->pEList->a[iOrderByCol-1].p_પદાવલિ;
          પદાવલિ *pBase = sqlite3_પદાવલિ_Skip_Collate_And_Likely(pX);
          જ્યારે( ALWAYS(pBase!=0) && pBase->op==TK_IF_NULL_ROW ){
            pX = pBase->p_ડાબું;
            pBase = sqlite3_પદાવલિ_Skip_Collate_And_Likely(pX);
          }
          જો( ALWAYS(pBase!=0)
           && pBase->op!=TK_AGG_COLUMN
           && pBase->op!=TK_REGISTER
          ){
            sqlite3_પદાવલિ_To_Register(pX, iAMem+j);
          }
        }
      }
      sqlite3VdbeAddOp4(v, OP_Compare, iAMem, iBMem, pGroupBy->n_પદાવલિ,
                          (અક્ષર*)sqlite3_ચાવી_Info_Ref(p_ચાવી_Info), P4_KEYINFO);
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
      sqlite3_પદાવલિ_Code_Move(pParse, iBMem, iAMem, pGroupBy->n_પદાવલિ);
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
      જો( groupBySort ){
        sqlite3VdbeAddOp2(v, OP_SorterNext, pAggInfo->sortingIdx,addr_ટોચ_Of_Loop);
        VdbeCoverage(v);
      }નહીંતર{
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3WhereEnd(pWInfo);
        sqlite3VdbeChangeToNoop(v, addrSortingIdx);
      }
      sqlite3_પદાવલિ_List_Delete(db, pDistinct);

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
      sqlite3_પદાવલિ_If_False(pParse, pHaving, addrOutputRow+1, SQLITE_JUMPIFNULL);
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

      જો( distFlag!=0 && eDist!=WHERE_DISTINCT_NOOP ){
        સંરચના AggInfo_func *pF = &pAggInfo->aFunc[0];
        fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
      }
    } /* endif pGroupBy.  Begin aggregate queries without GROUP BY: */
    નહીંતર {
      /* Aggregate functions without GROUP BY. tag-select-0820 */
      Table *pTab;
      જો( (pTab = is_Simple_ગણતરી(p, pAggInfo))!=0 ){
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
        અચલ પૂર્ણાંક iDb = sqlite3SchemaToIndex(pParse->db, pTab->pSchema);
        અચલ પૂર્ણાંક iCsr = pParse->nTab++;     /* Cursor to scan b-tree */
        Index *pIdx;                         /* Iterator variable */
        ચાવી_Info *p_ચાવી_Info = 0;               /* Keyinfo for scanned index */
        Index *pBest = 0;                    /* Best index found so far */
        Pgno i_મૂળ = pTab->tnum;             /* Root page of scanned b-tree */

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
        જો( !HasRowid(pTab) ) pBest = sqlite3_Primary_ચાવી_Index(pTab);
        જો( !p->pSrc->a[0].fg.notIndexed ){
          માટે(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
            જો( pIdx->bUnordered==0
             && pIdx->szIdxRow<pTab->szTabRow
             && pIdx->pPartIdxWhere==0
             && (!pBest || pIdx->szIdxRow<pBest->szIdxRow)
            ){
              pBest = pIdx;
            }
          }
        }
        જો( pBest ){
          i_મૂળ = pBest->tnum;
          p_ચાવી_Info = sqlite3_ચાવી_Info_Of_Index(pParse, pBest);
        }

        /* Open a read-only cursor, execute the OP_Count, close the cursor. */
        sqlite3VdbeAddOp4Int(v, OP_OpenRead, iCsr, (પૂર્ણાંક)i_મૂળ, iDb, 1);
        જો( p_ચાવી_Info ){
          sqlite3VdbeChangeP4(v, -1, (અક્ષર *)p_ચાવી_Info, P4_KEYINFO);
        }
        assignAggregateRegisters(pParse, pAggInfo);
        sqlite3VdbeAddOp2(v, OP_ગણતરી, iCsr, AggInfoFuncReg(pAggInfo,0));
        sqlite3VdbeAddOp1(v, OP_Close, iCsr);
        explain_Simple_ગણતરી(pParse, pTab, pBest);
      }નહીંતર{
        /* The general case of an aggregate query without GROUP BY
        ** tag-select-0822 */
        પૂર્ણાંક regAcc = 0;           /* "populate accumulators" flag */
        પદાવલિ_List *pDistinct = 0;
        u16 distFlag = 0;
        પૂર્ણાંક eDist;

        /* If there are accumulator registers but no min() or max() functions
        ** without FILTER clauses, allocate register regAcc. Register regAcc
        ** will contain 0 the first time the inner loop runs, and 1 thereafter.
        ** The code generated by updateAccumulator() uses this to ensure
        ** that the accumulator registers are (a) updated only once if
        ** there are no min() or max functions or (b) always updated for the
        ** first row visited by the aggregate, so that they are updated at
        ** least once even if the FILTER clause means the min() or max()
        ** function visits zero rows.  */
        જો( pAggInfo->nAccumulator ){
          માટે(i=0; i<pAggInfo->nFunc; i++){
            જો( પદાવલિ_Has_Property(pAggInfo->aFunc[i].pFExpr, EP_WinFunc) ){
              ચાલુ;
            }
            જો( pAggInfo->aFunc[i].pFunc->funcFlags&SQLITE_FUNC_NEEDCOLL ){
              તોડો;
            }
          }
          જો( i==pAggInfo->nFunc ){
            regAcc = ++pParse->nMem;
            sqlite3VdbeAddOp2(v, OP_Integer, 0, regAcc);
          }
        }નહીંતર જો( pAggInfo->nFunc==1 && pAggInfo->aFunc[0].iDistinct>=0 ){
          પ્રતિજ્ઞા( પદાવલિ_Use_XList(pAggInfo->aFunc[0].pFExpr) );
          pDistinct = pAggInfo->aFunc[0].pFExpr->x.pList;
          distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
        }
        assignAggregateRegisters(pParse, pAggInfo);

        /* This case runs if the aggregate has no GROUP BY clause.  The
        ** processing is much simpler since there is only a single row
        ** of output.
        */
        પ્રતિજ્ઞા( p->pGroupBy==0 );
        resetAccumulator(pParse, pAggInfo);

        /* If this query is a candidate for the min/max optimization, then
        ** minMaxFlag will have been previously set to either
        ** WHERE_ORDERBY_MIN or WHERE_ORDERBY_MAX and pMinMaxOrderBy will
        ** be an appropriate ORDER BY expression for the optimization.
        */
        પ્રતિજ્ઞા( minMaxFlag==WHERE_ORDERBY_NORMAL || pMinMaxOrderBy!=0 );
        પ્રતિજ્ઞા( pMinMaxOrderBy==0 || pMinMaxOrderBy->n_પદાવલિ==1 );

        TREETRACE(0x2,pParse,p,("WhereBegin\n"));
        pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, pMinMaxOrderBy,
                                   pDistinct, p, minMaxFlag|distFlag, 0);
        જો( pWInfo==0 ){
          જાઓ select_end;
        }
        TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
        eDist = sqlite3WhereIsDistinct(pWInfo);
        updateAccumulator(pParse, regAcc, pAggInfo, eDist);
        જો( eDist!=WHERE_DISTINCT_NOOP ){
          સંરચના AggInfo_func *pF = pAggInfo->aFunc;
          જો( pF ){
            fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
          }
        }

        જો( regAcc ) sqlite3VdbeAddOp2(v, OP_Integer, 1, regAcc);
        જો( minMaxFlag ){
          sqlite3WhereMinMaxOptEarlyOut(v, pWInfo);
        }
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3WhereEnd(pWInfo);
        finalizeAggFunctions(pParse, pAggInfo);
      }

      sSort.pOrderBy = 0;
      sqlite3_પદાવલિ_If_False(pParse, pHaving, addrEnd, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, 0, 0,
                      pDest, addrEnd, addrEnd);
    }
    sqlite3VdbeResolveLabel(v, addrEnd);

  } /* endif aggregate query */

  જો( sDistinct.eTnctType==WHERE_DISTINCT_UNORDERED ){
    explainTempTable(pParse, "DISTINCT");
  }

  /* If there is an ORDER BY clause, then we need to sort the results
  ** and send them to the callback one by one.  tag-select-0900
  */
  જો( sSort.pOrderBy ){
    પ્રતિજ્ઞા( p->pEList==pEList );
    generateSortTail(pParse, p, &sSort, pEList->n_પદાવલિ, pDest);
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
  પ્રતિજ્ઞા( db->mallocFailed==0 || db->mallocFailed==1 );
  પ્રતિજ્ઞા( db->mallocFailed==0 || pParse->nErr!=0 );
  sqlite3_પદાવલિ_List_Delete(db, pMinMaxOrderBy);
#ifdef SQLITE_DEBUG
  /* Internal self-checks.  tag-select-1000 */
  જો( pAggInfo && !db->mallocFailed ){
#જો TREETRACE_ENABLED
    જો( sqlite3_વૃક્ષ_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("Finished with AggInfo\n"));
      printAggInfo(pAggInfo);
    }
#endif
    માટે(i=0; i<pAggInfo->nColumn; i++){
      પદાવલિ *p_પદાવલિ = pAggInfo->aCol[i].pCExpr;
      જો( p_પદાવલિ==0 ) ચાલુ;
      પ્રતિજ્ઞા( p_પદાવલિ->pAggInfo==pAggInfo );
      પ્રતિજ્ઞા( p_પદાવલિ->iAgg==i );
    }
    માટે(i=0; i<pAggInfo->nFunc; i++){
      પદાવલિ *p_પદાવલિ = pAggInfo->aFunc[i].pFExpr;
      પ્રતિજ્ઞા( p_પદાવલિ!=0 );
      પ્રતિજ્ઞા( p_પદાવલિ->pAggInfo==pAggInfo );
      પ્રતિજ્ઞા( p_પદાવલિ->iAgg==i );
    }
  }
#endif

#જો TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p,("end processing\n"));
  જો( (sqlite3_વૃક્ષ_Trace & 0x40000)!=0 && ExplainQueryPlanParent(pParse)==0 ){
    sqlite3_વૃક્ષ_View_Select(0, p, 0);
  }
#endif
  Explain_Query_Plan_કાઢો(pParse);
  પાછા rc;
}
