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
#अन्तर्भाव "sqliteInt.h"

/*
** An instance of the following object is used to record information about
** how to process the DISTINCT keyword, to simplify passing that information
** into the selectInnerLoop() routine.
*/
प्रकारनिर्देश संरचना DistinctCtx DistinctCtx;
संरचना DistinctCtx {
  u8 isTnct;      /* 0: Not distinct. 1: DISTINCT  2: DISTINCT and ORDER BY */
  u8 eTnctType;   /* One of the WHERE_DISTINCT_* operators */
  पूर्णाङ्क tabTnct;    /* Ephemeral table used for DISTINCT processing */
  पूर्णाङ्क addrTnct;   /* Address of OP_OpenEphemeral opcode for tabTnct */
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
प्रकारनिर्देश संरचना क्रमय_Ctx क्रमय_Ctx;
संरचना क्रमय_Ctx {
  व्यञ्जकम्_List *p_क्रमः_By;   /* The ORDER BY (or GROUP BY clause) */
  पूर्णाङ्क nOBSat;           /* Number of ORDER BY terms satisfied by indices */
  पूर्णाङ्क iECursor;         /* Cursor number for the sorter */
  पूर्णाङ्क regReturn;        /* Register holding block-output return address */
  पूर्णाङ्क label_Bk_बहिर्गमनम्;       /* Start label for the block-output subroutine */
  पूर्णाङ्क addr_क्रमय_सूचकांक;    /* Address of the OP_SorterOpen or OP_OpenEphemeral */
  पूर्णाङ्क label_समाप्तम्;        /* Jump here when done, ex: LIMIT reached */
  पूर्णाङ्क labelOBLopt;      /* Jump here when sorter is full */
  u8 क्रमय_Flags;         /* Zero or more SORTFLAG_* bits */
#यदि_परिभाषितम् SQLITE_ENABLE_SORTER_REFERENCES
  u8 nDefer;            /* Number of valid entries in aDefer[] */
  संरचना DeferredCsr {
    सारणी *pTab;        /* Table definition */
    पूर्णाङ्क iCsr;           /* Cursor number for table */
    पूर्णाङ्क n_कुञ्चिका;           /* Number of PK columns for table pTab (>=1) */
  } aDefer[4];
#यदि_अन्तः
  संरचना Row_आरोपय_Info *p_Deferred_Row_आरोपय;  /* Deferred row loading info or NULL */
#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
  पूर्णाङ्क addr_प्रक्षिप;         /* First instruction to push data into sorter */
  पूर्णाङ्क addr_प्रक्षिप_अन्तः;      /* Last instruction that pushes data into sorter */
#यदि_अन्तः
};
#परिभाषय SORTFLAG_UseSorter  0x01   /* Use SorterOpen instead of OpenEphemeral */

/*
** Delete all the content of a Select structure.  Deallocate the structure
** itself depending on the value of bFree
**
** If bFree==1, call sqlite3DbFree() on the p object.
** If bFree==0, Leave the first Select object unfreed
*/
स्थिर शून्य clearSelect(sqlite3 *db, Select *p, पूर्णाङ्क bFree){
  प्रतिज्ञा( db!=0 );
  यावत्( p ){
    Select *p_प्राक् = p->p_प्राक्;
    sqlite3_व्यञ्जकम्_List_Delete(db, p->pEList);
    sqlite3SrcListDelete(db, p->pSrc);
    sqlite3_व्यञ्जकम्_Delete(db, p->pWhere);
    sqlite3_व्यञ्जकम्_List_Delete(db, p->p_गणः_By);
    sqlite3_व्यञ्जकम्_Delete(db, p->pHaving);
    sqlite3_व्यञ्जकम्_List_Delete(db, p->p_क्रमः_By);
    sqlite3_व्यञ्जकम्_Delete(db, p->p_सीमा);
    यदि( OK_IF_ALWAYS_TRUE(p->pWith) ) sqlite3WithDelete(db, p->pWith);
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
    यदि( OK_IF_ALWAYS_TRUE(p->pWinDefn) ){
      sqlite3_गवाक्षः_List_Delete(db, p->pWinDefn);
    }
    यावत्( p->pWin ){
      प्रतिज्ञा( p->pWin->ppThis==&p->pWin );
      sqlite3_गवाक्षः_Unlink_From_Select(p->pWin);
    }
#यदि_अन्तः
    यदि( bFree ) sqlite3DbNNFreeNN(db, p);
    p = p_प्राक्;
    bFree = 1;
  }
}

/*
** Initialize a SelectDest structure.
*/
शून्य sqlite3SelectDestInit(SelectDest *pDest, पूर्णाङ्क eDest, पूर्णाङ्क iParm){
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
  व्यञ्जकम्_List *pEList,     /* which columns to include in the result */
  SrcList *pSrc,        /* the FROM clause -- which tables to scan */
  व्यञ्जकम् *pWhere,         /* the WHERE clause */
  व्यञ्जकम्_List *p_गणः_By,   /* the GROUP BY clause */
  व्यञ्जकम् *pHaving,        /* the HAVING clause */
  व्यञ्जकम्_List *p_क्रमः_By,   /* the ORDER BY clause */
  u32 selFlags,         /* Flag parameters, such as SF_Distinct */
  व्यञ्जकम् *p_सीमा          /* LIMIT value.  NULL means not used */
){
  Select *pNew, *pAllocated;
  Select standin;
  pAllocated = pNew = sqlite3DbMallocRawNN(pParse->db, परिमाण(*pNew) );
  यदि( pNew==0 ){
    प्रतिज्ञा( pParse->db->mallocFailed );
    pNew = &standin;
  }
  यदि( pEList==0 ){
    pEList = sqlite3_व्यञ्जकम्_List_Append(pParse, 0,
                                   sqlite3_व्यञ्जकम्(pParse->db,TK_ASTERISK,0));
  }
  pNew->pEList = pEList;
  pNew->op = TK_SELECT;
  pNew->selFlags = selFlags;
  pNew->i_सीमा = 0;
  pNew->i_विस्थापनम् = 0;
  pNew->selId = ++pParse->nSelect;
  pNew->nSelectRow = 0;
  यदि( pSrc==0 ) pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  pNew->pSrc = pSrc;
  pNew->pWhere = pWhere;
  pNew->p_गणः_By = p_गणः_By;
  pNew->pHaving = pHaving;
  pNew->p_क्रमः_By = p_क्रमः_By;
  pNew->p_प्राक् = 0;
  pNew->p_अग्र = 0;
  pNew->p_सीमा = p_सीमा;
  pNew->pWith = 0;
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
  pNew->pWin = 0;
  pNew->pWinDefn = 0;
#यदि_अन्तः
  यदि( pParse->db->mallocFailed ) {
    clearSelect(pParse->db, pNew, pNew!=&standin);
    pAllocated = 0;
  }अन्यथा{
    प्रतिज्ञा( pNew->pSrc!=0 || pParse->nErr>0 );
  }
  प्रत्यागम pAllocated;
}


/*
** Delete the given Select structure and all of its substructures.
*/
शून्य sqlite3SelectDelete(sqlite3 *db, Select *p){
  यदि( OK_IF_ALWAYS_TRUE(p) ) clearSelect(db, p, 1);
}
शून्य sqlite3SelectDeleteGeneric(sqlite3 *db, शून्य *p){
  यदि( ALWAYS(p) ) clearSelect(db, (Select*)p, 1);
}

/*
** Return a pointer to the right-most SELECT statement in a compound.
*/
स्थिर Select *findRightmost(Select *p){
  यावत्( p->p_अग्र ) p = p->p_अग्र;
  प्रत्यागम p;
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
पूर्णाङ्क sqlite3JoinType(Parse *pParse, Token *pA, Token *pB, Token *pC){
  पूर्णाङ्क jointype = 0;
  Token *apAll[3];
  Token *p;
                             /*   0123456789 123456789 123456789 123 */
  स्थिर ध्रुव वर्ण z_कुञ्चिका_Text[] = "naturaleftouterightfullinnercross";
  स्थिर ध्रुव संरचना {
    u8 i;        /* Beginning of keyword text in zKeyText[] */
    u8 nChar;    /* Length of the keyword in characters */
    u8 संहिता;     /* Join type mask */
  } aKeyword[] = {
    /* (0) natural */ { 0,  7, JT_NATURAL                },
    /* (1) left    */ { 6,  4, JT_वामः|JT_OUTER          },
    /* (2) outer   */ { 10, 5, JT_OUTER                  },
    /* (3) right   */ { 14, 5, JT_दक्षिणः|JT_OUTER         },
    /* (4) full    */ { 19, 4, JT_वामः|JT_दक्षिणः|JT_OUTER },
    /* (5) inner   */ { 23, 5, JT_INNER                  },
    /* (6) cross   */ { 28, 5, JT_INNER|JT_CROSS         },
  };
  पूर्णाङ्क i, j;
  apAll[0] = pA;
  apAll[1] = pB;
  apAll[2] = pC;
  प्रति(i=0; i<3 && apAll[i]; i++){
    p = apAll[i];
    प्रति(j=0; j<Array_आकार(aKeyword); j++){
      यदि( p->n==aKeyword[j].nChar
          && sqlite3StrNICmp((वर्ण*)p->z, &z_कुञ्चिका_Text[aKeyword[j].i], p->n)==0 ){
        jointype |= aKeyword[j].संहिता;
        भङ्ग;
      }
    }
    testcase( j==0 || j==1 || j==2 || j==3 || j==4 || j==5 || j==6 );
    यदि( j>=Array_आकार(aKeyword) ){
      jointype |= JT_ERROR;
      भङ्ग;
    }
  }
  यदि(
     (jointype & (JT_INNER|JT_OUTER))==(JT_INNER|JT_OUTER) ||
     (jointype & JT_ERROR)!=0 ||
     (jointype & (JT_OUTER|JT_वामः|JT_दक्षिणः))==JT_OUTER
  ){
    ध्रुव वर्ण *zSp1 = " ";
    ध्रुव वर्ण *zSp2 = " ";
    यदि( pB==0 ){ zSp1++; }
    यदि( pC==0 ){ zSp2++; }
    sqlite3_Error_सन्देशः(pParse, "unknown join type: "
       "%T%s%T%s%T", pA, zSp1, pB, zSp2, pC);
    jointype = JT_INNER;
  }
  प्रत्यागम jointype;
}

/*
** Return the index of a column in a table.  Return -1 if the column
** is not contained in the table.
*/
पूर्णाङ्क sqlite3_Column_सूचकांक(सारणी *pTab, ध्रुव वर्ण *zCol){
  पूर्णाङ्क i;
  u8 h;
  ध्रुव Column *aCol;
  पूर्णाङ्क nCol;

  h = sqlite3StrIHash(zCol);
  aCol = pTab->aCol;
  nCol = pTab->nCol;

  /* See if the aHx gives us a lucky match */
  i = pTab->aHx[h % परिमाण(pTab->aHx)];
  प्रतिज्ञा( i<nCol );
  यदि( aCol[i].hName==h
   && sqlite3StrICmp(aCol[i].zCnName, zCol)==0
  ){
    प्रत्यागम i;
  }

  /* No lucky match from the hash table.  Do a full search. */
  i = 0;
  यावत्( 1 /*exit-by-break*/ ){
    यदि( aCol[i].hName==h
     && sqlite3StrICmp(aCol[i].zCnName, zCol)==0
    ){
      प्रत्यागम i;
    }
    i++;
    यदि( i>=nCol ) भङ्ग;
  }
  प्रत्यागम -1;
}

/*
** Mark a subquery result column as having been used.
*/
शून्य sqlite3_Src_Item_Column_प्रयुक्तम्(SrcItem *pItem, पूर्णाङ्क iCol){
  प्रतिज्ञा( pItem!=0 );
  प्रतिज्ञा( (पूर्णाङ्क)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
  यदि( pItem->fg.isNestedFrom ){
    व्यञ्जकम्_List *pResults;
    प्रतिज्ञा( pItem->fg.isSubquery );
    प्रतिज्ञा( pItem->u4.pSubq!=0 );
    प्रतिज्ञा( pItem->u4.pSubq->pSelect!=0 );
    pResults = pItem->u4.pSubq->pSelect->pEList;
    प्रतिज्ञा( pResults!=0 );
    प्रतिज्ञा( iCol>=0 && iCol<pResults->n_व्यञ्जकम् );
    pResults->a[iCol].fg.b_प्रयुक्तम् = 1;
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
स्थिर पूर्णाङ्क सारणी_And_Column_सूचकांक(
  SrcList *pSrc,       /* Array of tables to search */
  पूर्णाङ्क iStart,          /* First member of pSrc->a[] to check */
  पूर्णाङ्क i_अन्तः,            /* Last member of pSrc->a[] to check */
  ध्रुव वर्ण *zCol,    /* Name of the column we are looking for */
  पूर्णाङ्क *piTab,          /* Write index of pSrc->a[] here */
  पूर्णाङ्क *piCol,          /* Write index of pSrc->a[*piTab].pSTab->aCol[] here */
  पूर्णाङ्क bIgnoreHidden    /* Ignore hidden columns */
){
  पूर्णाङ्क i;               /* For looping over tables in pSrc */
  पूर्णाङ्क iCol;            /* Index of column matching zCol */

  प्रतिज्ञा( i_अन्तः<pSrc->nSrc );
  प्रतिज्ञा( iStart>=0 );
  प्रतिज्ञा( (piTab==0)==(piCol==0) );  /* Both or neither are NULL */

  प्रति(i=iStart; i<=i_अन्तः; i++){
    iCol = sqlite3_Column_सूचकांक(pSrc->a[i].pSTab, zCol);
    यदि( iCol>=0
     && (bIgnoreHidden==0 || IsHiddenColumn(&pSrc->a[i].pSTab->aCol[iCol])==0)
    ){
      यदि( piTab ){
        sqlite3_Src_Item_Column_प्रयुक्तम्(&pSrc->a[i], iCol);
        *piTab = i;
        *piCol = iCol;
      }
      प्रत्यागम 1;
    }
  }
  प्रत्यागम 0;
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
शून्य sqlite3_समुच्चयः_Join_व्यञ्जकम्(व्यञ्जकम् *p, पूर्णाङ्क i_सारणी, u32 joinFlag){
  प्रतिज्ञा( joinFlag==EP_OuterON || joinFlag==EP_InnerON );
  यावत्( p ){
    व्यञ्जकम्_समुच्चयः_Property(p, joinFlag);
    प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    व्यञ्जकम्_समुच्चयः_VVAProperty(p, EP_NoReduce);
    p->w.iJoin = i_सारणी;
    यदि( व्यञ्जकम्_Use_XList(p) ){
      यदि( p->x.pList ){
        पूर्णाङ्क i;
        प्रति(i=0; i<p->x.pList->n_व्यञ्जकम्; i++){
          sqlite3_समुच्चयः_Join_व्यञ्जकम्(p->x.pList->a[i].p_व्यञ्जकम्, i_सारणी, joinFlag);
        }
      }
    }
    sqlite3_समुच्चयः_Join_व्यञ्जकम्(p->p_वामः, i_सारणी, joinFlag);
    p = p->p_दक्षिणः;
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
स्थिर शून्य unset_Join_व्यञ्जकम्(व्यञ्जकम् *p, पूर्णाङ्क i_सारणी, पूर्णाङ्क nullable){
  यावत्( p ){
    यदि( i_सारणी<0 || (व्यञ्जकम्_Has_Property(p, EP_OuterON) && p->w.iJoin==i_सारणी) ){
      व्यञ्जकम्_Clear_Property(p, EP_OuterON|EP_InnerON);
      यदि( i_सारणी>=0 ) व्यञ्जकम्_समुच्चयः_Property(p, EP_InnerON);
    }
    यदि( p->op==TK_COLUMN && p->i_सारणी==i_सारणी && !nullable ){
      व्यञ्जकम्_Clear_Property(p, EP_CanBeNull);
    }
    यदि( p->op==TK_FUNCTION ){
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p) );
      प्रतिज्ञा( p->p_वामः==0 );
      यदि( p->x.pList ){
        पूर्णाङ्क i;
        प्रति(i=0; i<p->x.pList->n_व्यञ्जकम्; i++){
          unset_Join_व्यञ्जकम्(p->x.pList->a[i].p_व्यञ्जकम्, i_सारणी, nullable);
        }
      }
    }
    unset_Join_व्यञ्जकम्(p->p_वामः, i_सारणी, nullable);
    p = p->p_दक्षिणः;
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
स्थिर पूर्णाङ्क sqlite3ProcessJoin(Parse *pParse, Select *p){
  SrcList *pSrc;                  /* All tables in the FROM clause */
  पूर्णाङ्क i, j;                       /* Loop counters */
  SrcItem *p_वामः;                 /* Left table being joined */
  SrcItem *p_दक्षिणः;                /* Right table being joined */
  पूर्णाङ्क b_आलोकितम्_TFunc = 0;             /* True if have seen table-valued function */

  pSrc = p->pSrc;
  p_वामः = &pSrc->a[0];
  p_दक्षिणः = &p_वामः[1];
  प्रति(i=0; i<pSrc->nSrc-1; i++, p_दक्षिणः++, p_वामः++){
    सारणी *p_दक्षिणः_Tab = p_दक्षिणः->pSTab;
    u32 joinType;

    यदि( NEVER(p_वामः->pSTab==0 || p_दक्षिणः_Tab==0) ) अनुवर्त;
    joinType = (p_दक्षिणः->fg.jointype & JT_OUTER)!=0 ? EP_OuterON : EP_InnerON;

    /* If this is a NATURAL join, synthesize an appropriate USING clause
    ** to specify which columns should be joined.
    */
    यदि( p_दक्षिणः->fg.jointype & JT_NATURAL ){
      IdList *pUsing = 0;
      यदि( p_दक्षिणः->fg.isUsing || p_दक्षिणः->u3.pOn ){
        sqlite3_Error_सन्देशः(pParse, "a NATURAL join may not have "
           "an ON or USING clause", 0);
        प्रत्यागम 1;
      }
      प्रति(j=0; j<p_दक्षिणः_Tab->nCol; j++){
        वर्ण *zName;   /* Name of column in the right table */

        यदि( IsHiddenColumn(&p_दक्षिणः_Tab->aCol[j]) ) अनुवर्त;
        zName = p_दक्षिणः_Tab->aCol[j].zCnName;
        यदि( सारणी_And_Column_सूचकांक(pSrc, 0, i, zName, 0, 0, 1) ){
          pUsing = sqlite3IdListAppend(pParse, pUsing, 0);
          यदि( pUsing ){
            प्रतिज्ञा( pUsing->nId>0 );
            प्रतिज्ञा( pUsing->a[pUsing->nId-1].zName==0 );
            pUsing->a[pUsing->nId-1].zName = sqlite3DbStrDup(pParse->db, zName);
          }
        }
      }
      यदि( pUsing ){
        p_दक्षिणः->fg.isUsing = 1;
        p_दक्षिणः->fg.isSynthUsing = 1;
        p_दक्षिणः->u3.pUsing = pUsing;
      }
      यदि( pParse->nErr ) प्रत्यागम 1;
    }

    /* Create extra terms on the WHERE clause for each column named
    ** in the USING clause.  Example: If the two tables to be joined are
    ** A and B and the USING clause names X, Y, and Z, then add this
    ** to the WHERE clause:    A.X=B.X AND A.Y=B.Y AND A.Z=B.Z
    ** Report an error if any column mentioned in the USING clause is
    ** not contained in both tables to be joined.
    */
    यदि( p_दक्षिणः->fg.isUsing ){
      IdList *pList = p_दक्षिणः->u3.pUsing;
      sqlite3 *db = pParse->db;
      प्रतिज्ञा( pList!=0 );
      प्रति(j=0; j<pList->nId; j++){
        वर्ण *zName;     /* Name of the term in the USING clause */
        पूर्णाङ्क i_वामः;       /* Table on the left with matching column name */
        पूर्णाङ्क i_वामः_Col;    /* Column number of matching column on the left */
        पूर्णाङ्क i_दक्षिणः_Col;   /* Column number of matching column on the right */
        व्यञ्जकम् *pE1;       /* Reference to the column on the LEFT of the join */
        व्यञ्जकम् *pE2;       /* Reference to the column on the RIGHT of the join */
        व्यञ्जकम् *pEq;       /* Equality constraint.  pE1 == pE2 */

        zName = pList->a[j].zName;
        i_दक्षिणः_Col = sqlite3_Column_सूचकांक(p_दक्षिणः_Tab, zName);
        यदि( i_दक्षिणः_Col<0
         || सारणी_And_Column_सूचकांक(pSrc, 0, i, zName, &i_वामः, &i_वामः_Col,
                                p_दक्षिणः->fg.isSynthUsing)==0
        ){
          sqlite3_Error_सन्देशः(pParse, "cannot join using column %s - column "
            "not present in both tables", zName);
          प्रत्यागम 1;
        }
        pE1 = sqlite3_Create_Column_व्यञ्जकम्(db, pSrc, i_वामः, i_वामः_Col);
        sqlite3_Src_Item_Column_प्रयुक्तम्(&pSrc->a[i_वामः], i_वामः_Col);
        यदि( (pSrc->a[0].fg.jointype & JT_LTORJ)!=0 && pParse->nErr==0 ){
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
          व्यञ्जकम्_List *p_Func_तर्काः = 0;   /* Arguments to the coalesce() */
          स्थिर ध्रुव Token tkCoalesce = { "coalesce", 8 };
          प्रतिज्ञा( pE1!=0 );
          व्यञ्जकम्_समुच्चयः_Property(pE1, EP_CanBeNull);
          यावत्( सारणी_And_Column_सूचकांक(pSrc, i_वामः+1, i, zName, &i_वामः, &i_वामः_Col,
                                     p_दक्षिणः->fg.isSynthUsing)!=0 ){
            यदि( pSrc->a[i_वामः].fg.isUsing==0
             || sqlite3_Id_List_सूचकांक(pSrc->a[i_वामः].u3.pUsing, zName)<0
            ){
              sqlite3_Error_सन्देशः(pParse, "ambiguous reference to %s in USING()",
                              zName);
              भङ्ग;
            }
            p_Func_तर्काः = sqlite3_व्यञ्जकम्_List_Append(pParse, p_Func_तर्काः, pE1);
            pE1 = sqlite3_Create_Column_व्यञ्जकम्(db, pSrc, i_वामः, i_वामः_Col);
            sqlite3_Src_Item_Column_प्रयुक्तम्(&pSrc->a[i_वामः], i_वामः_Col);
          }
          यदि( p_Func_तर्काः ){
            p_Func_तर्काः = sqlite3_व्यञ्जकम्_List_Append(pParse, p_Func_तर्काः, pE1);
            pE1 = sqlite3_व्यञ्जकम्_Function(pParse, p_Func_तर्काः, &tkCoalesce, 0);
            यदि( pE1 ){
              pE1->aff_व्यञ्जकम् = SQLITE_AFF_DEFER;
            }
          }
        }अन्यथा यदि( (pSrc->a[i+1].fg.jointype & JT_वामः)!=0 && pParse->nErr==0 ){
          प्रतिज्ञा( pE1!=0 );
          व्यञ्जकम्_समुच्चयः_Property(pE1, EP_CanBeNull);
        }
        pE2 = sqlite3_Create_Column_व्यञ्जकम्(db, pSrc, i+1, i_दक्षिणः_Col);
        sqlite3_Src_Item_Column_प्रयुक्तम्(p_दक्षिणः, i_दक्षिणः_Col);
        pEq = sqlite3PExpr(pParse, TK_EQ, pE1, pE2);
        प्रतिज्ञा( pE2!=0 || pEq==0 );
        यदि( pEq ){
          व्यञ्जकम्_समुच्चयः_Property(pEq, joinType);
          प्रतिज्ञा( !व्यञ्जकम्_Has_Property(pEq, EP_TokenOnly|EP_Reduced) );
          व्यञ्जकम्_समुच्चयः_VVAProperty(pEq, EP_NoReduce);
          pEq->w.iJoin = pE2->i_सारणी;
        }
        p->pWhere = sqlite3_व्यञ्जकम्_And(pParse, p->pWhere, pEq);
      }
    }

    /* Add the ON clause to the end of the WHERE clause, connected by
    ** an AND operator.
    */
    अन्यथा यदि( p_दक्षिणः->u3.pOn ){
      sqlite3_समुच्चयः_Join_व्यञ्जकम्(p_दक्षिणः->u3.pOn, p_दक्षिणः->iCursor, joinType);
      p->pWhere = sqlite3_व्यञ्जकम्_And(pParse, p->pWhere, p_दक्षिणः->u3.pOn);
      p_दक्षिणः->u3.pOn = 0;
      p_दक्षिणः->fg.isOn = 1;
      p->selFlags |= SF_On_गन्तव्यम्_Where;
    }

    यदि( b_आलोकितम्_TFunc==0 ){
      b_आलोकितम्_TFunc = (p_वामः->fg.isTabFunc && p_वामः->u1.pFuncArg);
    }

    /* We also need to call sqlite3SelectCheckOnClauses() to verify that
    ** table-valued function arguments do not illegally refer to any tables to
    ** their right. This test is required if either (a) there is a RIGHT JOIN
    ** in the SrcList, or (b) there is a LEFT JOIN to the right of a 
    ** table-valued function.  */
    यदि( (p_दक्षिणः->fg.isTabFunc && joinType==EP_OuterON && p_दक्षिणः->u1.pFuncArg)
     || (b_आलोकितम्_TFunc && joinType==EP_OuterON)
    ){
      p->selFlags |= SF_On_गन्तव्यम्_Where;
    }
  }
  प्रत्यागम 0;
}

/*
** An instance of this object holds information (beyond pParse and pSelect)
** needed to load the next result row that is to be added to the sorter.
*/
प्रकारनिर्देश संरचना Row_आरोपय_Info Row_आरोपय_Info;
संरचना Row_आरोपय_Info {
  पूर्णाङ्क reg_परिणाम;               /* Store results in array of registers here */
  u8 ecelFlags;                /* Flag argument to ExprCodeExprList() */
#यदि_परिभाषितम् SQLITE_ENABLE_SORTER_REFERENCES
  व्यञ्जकम्_List *pExtra;            /* Extra columns needed by sorter refs */
  पूर्णाङ्क reg_Extra_परिणाम;          /* Where to load the extra columns */
#यदि_अन्तः
};

/*
** This routine does the work of loading query data into an array of
** registers so that it can be added to the sorter.
*/
स्थिर शून्य inner_Loop_आरोपय_Row(
  Parse *pParse,             /* Statement under construction */
  Select *pSelect,           /* The query being coded */
  Row_आरोपय_Info *pInfo         /* Info needed to complete the row load */
){
  sqlite3_व्यञ्जकम्_संहिता_व्यञ्जकम्_List(pParse, pSelect->pEList, pInfo->reg_परिणाम,
                          0, pInfo->ecelFlags);
#यदि_परिभाषितम् SQLITE_ENABLE_SORTER_REFERENCES
  यदि( pInfo->pExtra ){
    sqlite3_व्यञ्जकम्_संहिता_व्यञ्जकम्_List(pParse, pInfo->pExtra, pInfo->reg_Extra_परिणाम, 0, 0);
    sqlite3_व्यञ्जकम्_List_Delete(pParse->db, pInfo->pExtra);
  }
#यदि_अन्तः
}

/*
** Code the OP_MakeRecord instruction that generates the entry to be
** added into the sorter.
**
** Return the register in which the result is stored.
*/
स्थिर पूर्णाङ्क make_Sorter_लेखः(
  Parse *pParse,
  क्रमय_Ctx *p_क्रमय,
  Select *pSelect,
  पूर्णाङ्क reg_आधारः,
  पूर्णाङ्क n_आधारः
){
  पूर्णाङ्क nOBSat = p_क्रमय->nOBSat;
  Vdbe *v = pParse->pVdbe;
  पूर्णाङ्क reg_बहिर्गमनम् = ++pParse->nMem;
  यदि( p_क्रमय->p_Deferred_Row_आरोपय ){
    inner_Loop_आरोपय_Row(pParse, pSelect, p_क्रमय->p_Deferred_Row_आरोपय);
  }
  sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, reg_आधारः+nOBSat, n_आधारः-nOBSat, reg_बहिर्गमनम्);
  प्रत्यागम reg_बहिर्गमनम्;
}

/*
** Generate code that will push the record in registers regData
** through regData+nData-1 onto the sorter.
*/
स्थिर शून्य प्रक्षिप_Onto_Sorter(
  Parse *pParse,         /* Parser context */
  क्रमय_Ctx *p_क्रमय,        /* Information about the ORDER BY clause */
  Select *pSelect,       /* The whole SELECT statement */
  पूर्णाङ्क reg_दत्त,           /* First register holding data to be sorted */
  पूर्णाङ्क reg_Orig_दत्त,       /* First register holding data before packing */
  पूर्णाङ्क n_दत्त,             /* Number of elements in the regData data array */
  पूर्णाङ्क n_उपसर्गः_Reg         /* No. of reg prior to regData available for use */
){
  Vdbe *v = pParse->pVdbe;                         /* Stmt under construction */
  पूर्णाङ्क bSeq = ((p_क्रमय->क्रमय_Flags & SORTFLAG_UseSorter)==0);
  पूर्णाङ्क n_व्यञ्जकम् = p_क्रमय->p_क्रमः_By->n_व्यञ्जकम्;              /* No. of ORDER BY terms */
  पूर्णाङ्क n_आधारः = n_व्यञ्जकम् + bSeq + n_दत्त;                /* Fields in sorter record */
  पूर्णाङ्क reg_आधारः;                                     /* Regs for sorter record */
  पूर्णाङ्क reg_लेखः = 0;                               /* Assembled sorter record */
  पूर्णाङ्क nOBSat = p_क्रमय->nOBSat;                      /* ORDER BY terms to skip */
  पूर्णाङ्क op;                            /* Opcode to add sorter record to sorter */
  पूर्णाङ्क i_सीमा;                        /* LIMIT counter */
  पूर्णाङ्क iSkip = 0;                     /* End of the sorter insert loop */

  प्रतिज्ञा( bSeq==0 || bSeq==1 );

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
  प्रतिज्ञा( n_दत्त==1 || reg_दत्त==reg_Orig_दत्त || reg_Orig_दत्त==0 );

#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
  p_क्रमय->addr_प्रक्षिप = sqlite3VdbeCurrentAddr(v);
#यदि_अन्तः

  यदि( n_उपसर्गः_Reg ){
    प्रतिज्ञा( n_उपसर्गः_Reg==n_व्यञ्जकम्+bSeq );
    reg_आधारः = reg_दत्त - n_उपसर्गः_Reg;
  }अन्यथा{
    reg_आधारः = pParse->nMem + 1;
    pParse->nMem += n_आधारः;
  }
  प्रतिज्ञा( pSelect->i_विस्थापनम्==0 || pSelect->i_सीमा!=0 );
  i_सीमा = pSelect->i_विस्थापनम् ? pSelect->i_विस्थापनम्+1 : pSelect->i_सीमा;
  p_क्रमय->label_समाप्तम् = sqlite3VdbeMakeLabel(pParse);
  sqlite3_व्यञ्जकम्_संहिता_व्यञ्जकम्_List(pParse, p_क्रमय->p_क्रमः_By, reg_आधारः, reg_Orig_दत्त,
                          SQLITE_ECEL_DUP | (reg_Orig_दत्त? SQLITE_ECEL_REF : 0));
  यदि( bSeq ){
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Sequence, p_क्रमय->iECursor, reg_आधारः+n_व्यञ्जकम्);
  }
  यदि( n_उपसर्गः_Reg==0 && n_दत्त>0 ){
    sqlite3_व्यञ्जकम्_संहिता_Move(pParse, reg_दत्त, reg_आधारः+n_व्यञ्जकम्+bSeq, n_दत्त);
  }
  यदि( nOBSat>0 ){
    पूर्णाङ्क reg_पूर्व_कुञ्चिका;   /* The first nOBSat columns of the previous row */
    पूर्णाङ्क addrFirst;    /* Address of the OP_IfNot opcode */
    पूर्णाङ्क addrJmp;      /* Address of the OP_Jump opcode */
    VdbeOp *pOp;      /* Opcode that opens the sorter */
    पूर्णाङ्क n_कुञ्चिका;         /* Number of sorting key columns, including OP_Sequence */
    कुञ्चिका_Info *pKI;     /* Original KeyInfo on the sorter table */

    reg_लेखः = make_Sorter_लेखः(pParse, p_क्रमय, pSelect, reg_आधारः, n_आधारः);
    reg_पूर्व_कुञ्चिका = pParse->nMem+1;
    pParse->nMem += p_क्रमय->nOBSat;
    n_कुञ्चिका = n_व्यञ्जकम् - p_क्रमय->nOBSat + bSeq;
    यदि( bSeq ){
      addrFirst = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_IfNot, reg_आधारः+n_व्यञ्जकम्);
    }अन्यथा{
      addrFirst = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_SequenceTest, p_क्रमय->iECursor);
    }
    VdbeCoverage(v);
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Compare, reg_पूर्व_कुञ्चिका, reg_आधारः, p_क्रमय->nOBSat);
    pOp = sqlite3VdbeGetOp(v, p_क्रमय->addr_क्रमय_सूचकांक);
    यदि( pParse->db->mallocFailed ) प्रत्यागम;
    pOp->p2 = n_कुञ्चिका + n_दत्त;
    pKI = pOp->p4.p_कुञ्चिका_Info;
    स्मृतिं_पूरय(pKI->a_क्रमय_Flags, 0, pKI->n_कुञ्चिका_क्षेत्रम्); /* Makes OP_Jump testable */
    sqlite3VdbeChangeP4(v, -1, (वर्ण*)pKI, P4_KEYINFO);
    testcase( pKI->n_All_क्षेत्रम् > pKI->n_कुञ्चिका_क्षेत्रम्+2 );
    pOp->p4.p_कुञ्चिका_Info = sqlite3_कुञ्चिका_Info_From_व्यञ्जकम्_List(pParse,p_क्रमय->p_क्रमः_By,nOBSat,
                                           pKI->n_All_क्षेत्रम्-pKI->n_कुञ्चिका_क्षेत्रम्-1);
    pOp = 0; /* Ensure pOp not used after sqlite3VdbeAddOp3() */
    addrJmp = sqlite3VdbeCurrentAddr(v);
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Jump, addrJmp+1, 0, addrJmp+1); VdbeCoverage(v);
    p_क्रमय->label_Bk_बहिर्गमनम् = sqlite3VdbeMakeLabel(pParse);
    p_क्रमय->regReturn = ++pParse->nMem;
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, p_क्रमय->regReturn, p_क्रमय->label_Bk_बहिर्गमनम्);
    sqlite3_Vdbe_सङ्कलय_Op1(v, OP_ResetSorter, p_क्रमय->iECursor);
    यदि( i_सीमा ){
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IfNot, i_सीमा, p_क्रमय->label_समाप्तम्);
      VdbeCoverage(v);
    }
    sqlite3_Vdbe_Jump_अत्र(v, addrFirst);
    sqlite3_व्यञ्जकम्_संहिता_Move(pParse, reg_आधारः, reg_पूर्व_कुञ्चिका, p_क्रमय->nOBSat);
    sqlite3_Vdbe_Jump_अत्र(v, addrJmp);
  }
  यदि( i_सीमा ){
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
    पूर्णाङ्क iCsr = p_क्रमय->iECursor;
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IfNotZero, i_सीमा, sqlite3VdbeCurrentAddr(v)+4);
    VdbeCoverage(v);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_चरमः, iCsr, 0);
    iSkip = sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_IdxLE,
                                 iCsr, 0, reg_आधारः+nOBSat, n_व्यञ्जकम्-nOBSat);
    VdbeCoverage(v);
    sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Delete, iCsr);
  }
  यदि( reg_लेखः==0 ){
    reg_लेखः = make_Sorter_लेखः(pParse, p_क्रमय, pSelect, reg_आधारः, n_आधारः);
  }
  यदि( p_क्रमय->क्रमय_Flags & SORTFLAG_UseSorter ){
    op = OP_Sorter_निवेशय;
  }अन्यथा{
    op = OP_Idx_निवेशय;
  }
  sqlite3_Vdbe_सङ्कलय_Op4_Int(v, op, p_क्रमय->iECursor, reg_लेखः,
                       reg_आधारः+nOBSat, n_आधारः-nOBSat);
  यदि( iSkip ){
    sqlite3VdbeChangeP2(v, iSkip,
         p_क्रमय->labelOBLopt ? p_क्रमय->labelOBLopt : sqlite3VdbeCurrentAddr(v));
  }
#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
  p_क्रमय->addr_प्रक्षिप_अन्तः = sqlite3VdbeCurrentAddr(v)-1;
#यदि_अन्तः
}

/*
** Add code to implement the OFFSET
*/
स्थिर शून्य संहिता_विस्थापनम्(
  Vdbe *v,          /* Generate code into this VM */
  पूर्णाङ्क i_विस्थापनम्,      /* Register holding the offset counter */
  पूर्णाङ्क iContinue     /* Jump here to skip the current record */
){
  यदि( i_विस्थापनम्>0 ){
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_IfPos, i_विस्थापनम्, iContinue, 1); VdbeCoverage(v);
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
स्थिर पूर्णाङ्क संहिता_Distinct(
  Parse *pParse,     /* Parsing and code generating context */
  पूर्णाङ्क eTnctType,     /* WHERE_DISTINCT_* value */
  पूर्णाङ्क iTab,          /* A sorting index used to test for distinctness */
  पूर्णाङ्क addrRepeat,    /* Jump to here if not distinct */
  व्यञ्जकम्_List *pEList,  /* Expression for each element */
  पूर्णाङ्क regElem        /* First element */
){
  पूर्णाङ्क iRet = 0;
  पूर्णाङ्क n_परिणाम_Col = pEList->n_व्यञ्जकम्;
  Vdbe *v = pParse->pVdbe;

  विकल्प( eTnctType ){
    पक्ष WHERE_DISTINCT_ORDERED: {
      पूर्णाङ्क i;
      पूर्णाङ्क iJump;              /* Jump destination */
      पूर्णाङ्क reg_पूर्व;            /* Previous row content */

      /* Allocate space for the previous row */
      iRet = reg_पूर्व = pParse->nMem+1;
      pParse->nMem += n_परिणाम_Col;

      iJump = sqlite3VdbeCurrentAddr(v) + n_परिणाम_Col;
      प्रति(i=0; i<n_परिणाम_Col; i++){
        CollSeq *pColl = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, pEList->a[i].p_व्यञ्जकम्);
        यदि( i<n_परिणाम_Col-1 ){
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Ne, regElem+i, iJump, reg_पूर्व+i);
          VdbeCoverage(v);
        }अन्यथा{
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Eq, regElem+i, addrRepeat, reg_पूर्व+i);
          VdbeCoverage(v);
         }
        sqlite3VdbeChangeP4(v, -1, (ध्रुव वर्ण *)pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
      }
      प्रतिज्ञा( sqlite3VdbeCurrentAddr(v)==iJump || pParse->db->mallocFailed );
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Copy, regElem, reg_पूर्व, n_परिणाम_Col-1);
      भङ्ग;
    }

    पक्ष WHERE_DISTINCT_UNIQUE: {
      /* nothing to do */
      भङ्ग;
    }

    स्वभाव: {
      पूर्णाङ्क r1 = sqlite3_Get_अस्थायी_Reg(pParse);
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_प्राप्तम्, iTab, addrRepeat, regElem, n_परिणाम_Col);
      VdbeCoverage(v);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, regElem, n_परिणाम_Col, r1);
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, iTab, r1, regElem, n_परिणाम_Col);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      sqlite3_Release_अस्थायी_Reg(pParse, r1);
      iRet = iTab;
      भङ्ग;
    }
  }

  प्रत्यागम iRet;
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
स्थिर शून्य fixDistinctOpenEph(
  Parse *pParse,     /* Parsing and code generating context */
  पूर्णाङ्क eTnctType,     /* WHERE_DISTINCT_* value */
  पूर्णाङ्क iVal,          /* Value returned by codeDistinct() */
  पूर्णाङ्क iOpenEphAddr   /* Address of OP_OpenEphemeral instruction for iTab */
){
  यदि( pParse->nErr==0
   && (eTnctType==WHERE_DISTINCT_UNIQUE || eTnctType==WHERE_DISTINCT_ORDERED)
  ){
    Vdbe *v = pParse->pVdbe;
    sqlite3_Vdbe_Change_गन्तव्यम्_Noop(v, iOpenEphAddr);
    यदि( sqlite3VdbeGetOp(v, iOpenEphAddr+1)->opcode==OP_Explain ){
      sqlite3_Vdbe_Change_गन्तव्यम्_Noop(v, iOpenEphAddr+1);
    }
    यदि( eTnctType==WHERE_DISTINCT_ORDERED ){
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

#यदि_परिभाषितम् SQLITE_ENABLE_SORTER_REFERENCES
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
स्थिर शून्य select_व्यञ्जकम्_Defer(
  Parse *pParse,                  /* Leave any error here */
  क्रमय_Ctx *p_क्रमय,                 /* Sorter context */
  व्यञ्जकम्_List *pEList,               /* Expressions destined for sorter */
  व्यञ्जकम्_List **ppExtra              /* Expressions to append to sorter record */
){
  पूर्णाङ्क i;
  पूर्णाङ्क nDefer = 0;
  व्यञ्जकम्_List *pExtra = 0;
  प्रति(i=0; i<pEList->n_व्यञ्जकम्; i++){
    संरचना व्यञ्जकम्_List_item *pItem = &pEList->a[i];
    यदि( pItem->u.x.i_क्रमः_By_Col==0 ){
      व्यञ्जकम् *p_व्यञ्जकम् = pItem->p_व्यञ्जकम्;
      सारणी *pTab;
      यदि( p_व्यञ्जकम्->op==TK_COLUMN
       && p_व्यञ्जकम्->iColumn>=0
       && ALWAYS( व्यञ्जकम्_Use_YTab(p_व्यञ्जकम्) )
       && (pTab = p_व्यञ्जकम्->y.pTab)!=0
       && Is_Ordinary_सारणी(pTab)
       && (pTab->aCol[p_व्यञ्जकम्->iColumn].colFlags & COLFLAG_SORTERREF)!=0
      ){
        पूर्णाङ्क j;
        प्रति(j=0; j<nDefer; j++){
          यदि( p_क्रमय->aDefer[j].iCsr==p_व्यञ्जकम्->i_सारणी ) भङ्ग;
        }
        यदि( j==nDefer ){
          यदि( nDefer==Array_आकार(p_क्रमय->aDefer) ){
            अनुवर्त;
          }अन्यथा{
            पूर्णाङ्क n_कुञ्चिका = 1;
            पूर्णाङ्क k;
            सूचकांक *pPk = 0;
            यदि( !HasRowid(pTab) ){
              pPk = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
              n_कुञ्चिका = pPk->n_कुञ्चिका_Col;
            }
            प्रति(k=0; k<n_कुञ्चिका; k++){
              व्यञ्जकम् *pNew = sqlite3PExpr(pParse, TK_COLUMN, 0, 0);
              यदि( pNew ){
                pNew->i_सारणी = p_व्यञ्जकम्->i_सारणी;
                प्रतिज्ञा( व्यञ्जकम्_Use_YTab(pNew) );
                pNew->y.pTab = p_व्यञ्जकम्->y.pTab;
                pNew->iColumn = pPk ? pPk->aiColumn[k] : -1;
                pExtra = sqlite3_व्यञ्जकम्_List_Append(pParse, pExtra, pNew);
              }
            }
            p_क्रमय->aDefer[nDefer].pTab = p_व्यञ्जकम्->y.pTab;
            p_क्रमय->aDefer[nDefer].iCsr = p_व्यञ्जकम्->i_सारणी;
            p_क्रमय->aDefer[nDefer].n_कुञ्चिका = n_कुञ्चिका;
            nDefer++;
          }
        }
        pItem->fg.bSorterRef = 1;
      }
    }
  }
  p_क्रमय->nDefer = (u8)nDefer;
  *ppExtra = pExtra;
}
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_DEBUG
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
स्थिर पूर्णाङ्क sqlite3CursorBloomable(Parse *pParse, पूर्णाङ्क iCur, पूर्णाङ्क nCol){
  पूर्णाङ्क i,k;
  Vdbe *v = pParse->pVdbe;
  यदि( pParse->nErr ) प्रत्यागम 1;
  प्रतिज्ञा( v );
  प्रति(k=sqlite3VdbeCurrentAddr(v)-1; k>0; k--){
    ध्रुव VdbeOp *pOp = sqlite3VdbeGetOp(v, k);
    ध्रुव कुञ्चिका_Info *p_कुञ्चिका_Info;
    यदि( pOp->p1!=iCur ) अनुवर्त;
    यदि( pOp->opcode!=OP_OpenRead
     && pOp->opcode!=OP_OpenWrite
     && pOp->opcode!=OP_OpenEphemeral
    ){
      अनुवर्त;
    }
    प्रतिज्ञा( pOp->p4type==P4_KEYINFO );
    p_कुञ्चिका_Info = (ध्रुव कुञ्चिका_Info*)pOp->p4.p_कुञ्चिका_Info;
    प्रतिज्ञा( p_कुञ्चिका_Info!=0 );
    प्रति(i=0; i<nCol; i++){
      प्रतिज्ञा( sqlite3_Is_द्विमान(p_कुञ्चिका_Info->aColl[i]) );
    }
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
}
#यदि_अन्तः /* SQLITE_DEBUG */


/*
** This routine generates the code for the inside of the inner loop
** of a SELECT.
**
** If srcTab is negative, then the p->pEList expressions
** are evaluated in order to get the data for this row.  If srcTab is
** zero or more, then data is pulled from srcTab and p->pEList is used only
** to get the number of columns and the collation sequence for each column.
*/
स्थिर शून्य selectInnerLoop(
  Parse *pParse,          /* The parser context */
  Select *p,              /* The complete select statement being coded */
  पूर्णाङ्क srcTab,             /* Pull data from this table if non-negative */
  क्रमय_Ctx *p_क्रमय,         /* If not NULL, info on how to process ORDER BY */
  DistinctCtx *pDistinct, /* If not NULL, info on how to process DISTINCT */
  SelectDest *pDest,      /* How to dispose of the results */
  पूर्णाङ्क iContinue,          /* Jump here to continue with next row */
  पूर्णाङ्क iBreak              /* Jump here to break out of the inner loop */
){
  Vdbe *v = pParse->pVdbe;
  पूर्णाङ्क i;
  पूर्णाङ्क hasDistinct;            /* True if the DISTINCT keyword is present */
  पूर्णाङ्क eDest = pDest->eDest;   /* How to dispose of results */
  पूर्णाङ्क iParm = pDest->iSDParm; /* First argument to disposal method */
  पूर्णाङ्क n_परिणाम_Col;             /* Number of result columns */
  पूर्णाङ्क n_उपसर्गः_Reg = 0;         /* Number of extra registers before regResult */
  Row_आरोपय_Info s_Row_आरोपय_Info;   /* Info for deferred row loading */

  /* Usually, regResult is the first cell in an array of memory cells
  ** containing the current result row. In this case regOrig is set to the
  ** same value. However, if the results are being sent to the sorter, the
  ** values for any expressions that are also part of the sort-key are omitted
  ** from this array. In this case regOrig is set to zero.  */
  पूर्णाङ्क reg_परिणाम;              /* Start of memory holding current results */
  पूर्णाङ्क regOrig;                /* Start of memory holding full result (or 0) */

  प्रतिज्ञा( v );
  प्रतिज्ञा( p->pEList!=0 );
  hasDistinct = pDistinct ? pDistinct->eTnctType : WHERE_DISTINCT_NOOP;
  यदि( p_क्रमय && p_क्रमय->p_क्रमः_By==0 ) p_क्रमय = 0;
  यदि( p_क्रमय==0 && !hasDistinct ){
    प्रतिज्ञा( iContinue!=0 );
    संहिता_विस्थापनम्(v, p->i_विस्थापनम्, iContinue);
  }

  /* Pull the requested columns.
  */
  n_परिणाम_Col = p->pEList->n_व्यञ्जकम्;

  यदि( pDest->iSdst==0 ){
    यदि( p_क्रमय ){
      n_उपसर्गः_Reg = p_क्रमय->p_क्रमः_By->n_व्यञ्जकम्;
      यदि( !(p_क्रमय->क्रमय_Flags & SORTFLAG_UseSorter) ) n_उपसर्गः_Reg++;
      pParse->nMem += n_उपसर्गः_Reg;
    }
    pDest->iSdst = pParse->nMem+1;
    pParse->nMem += n_परिणाम_Col;
  }अन्यथा यदि( pDest->iSdst+n_परिणाम_Col > pParse->nMem ){
    /* This is an error condition that can result, for example, when a SELECT
    ** on the right-hand side of an INSERT contains more result columns than
    ** there are columns in the table on the left.  The error will be caught
    ** and reported later.  But we need to make sure enough memory is allocated
    ** to avoid other spurious errors in the meantime. */
    pParse->nMem += n_परिणाम_Col;
  }
  pDest->nSdst = n_परिणाम_Col;
  regOrig = reg_परिणाम = pDest->iSdst;
  यदि( srcTab>=0 ){
    प्रति(i=0; i<n_परिणाम_Col; i++){
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, srcTab, i, reg_परिणाम+i);
      VdbeComment((v, "%s", p->pEList->a[i].zEName));
    }
  }अन्यथा यदि( eDest!=SRT_Exists ){
#यदि_परिभाषितम् SQLITE_ENABLE_SORTER_REFERENCES
    व्यञ्जकम्_List *pExtra = 0;
#यदि_अन्तः
    /* If the destination is an EXISTS(...) expression, the actual
    ** values returned by the SELECT are not required.
    */
    u8 ecelFlags;    /* "ecel" is an abbreviation of "ExprCodeExprList" */
    व्यञ्जकम्_List *pEList;
    यदि( eDest==SRT_Mem || eDest==SRT_Output || eDest==SRT_Coroutine ){
      ecelFlags = SQLITE_ECEL_DUP;
    }अन्यथा{
      ecelFlags = 0;
    }
    यदि( p_क्रमय && hasDistinct==0 && eDest!=SRT_EphemTab && eDest!=SRT_सारणी ){
      /* For each expression in p->pEList that is a copy of an expression in
      ** the ORDER BY clause (pSort->pOrderBy), set the associated
      ** iOrderByCol value to one more than the index of the ORDER BY
      ** expression within the sort-key that pushOntoSorter() will generate.
      ** This allows the p->pEList field to be omitted from the sorted record,
      ** saving space and CPU cycles.  */
      ecelFlags |= (SQLITE_ECEL_OMITREF|SQLITE_ECEL_REF);

      प्रति(i=p_क्रमय->nOBSat; i<p_क्रमय->p_क्रमः_By->n_व्यञ्जकम्; i++){
        पूर्णाङ्क j;
        यदि( (j = p_क्रमय->p_क्रमः_By->a[i].u.x.i_क्रमः_By_Col)>0 ){
          p->pEList->a[j-1].u.x.i_क्रमः_By_Col = i+1-p_क्रमय->nOBSat;
        }
      }
#यदि_परिभाषितम् SQLITE_ENABLE_SORTER_REFERENCES
      select_व्यञ्जकम्_Defer(pParse, p_क्रमय, p->pEList, &pExtra);
      यदि( pExtra && pParse->db->mallocFailed==0 ){
        /* If there are any extra PK columns to add to the sorter records,
        ** allocate extra memory cells and adjust the OpenEphemeral
        ** instruction to account for the larger records. This is only
        ** required if there are one or more WITHOUT ROWID tables with
        ** composite primary keys in the SortCtx.aDefer[] array.  */
        VdbeOp *pOp = sqlite3VdbeGetOp(v, p_क्रमय->addr_क्रमय_सूचकांक);
        pOp->p2 += (pExtra->n_व्यञ्जकम् - p_क्रमय->nDefer);
        pOp->p4.p_कुञ्चिका_Info->n_All_क्षेत्रम् += (pExtra->n_व्यञ्जकम् - p_क्रमय->nDefer);
        pParse->nMem += pExtra->n_व्यञ्जकम्;
      }
#यदि_अन्तः

      /* Adjust nResultCol to account for columns that are omitted
      ** from the sorter by the optimizations in this branch */
      pEList = p->pEList;
      प्रति(i=0; i<pEList->n_व्यञ्जकम्; i++){
        यदि( pEList->a[i].u.x.i_क्रमः_By_Col>0
#यदि_परिभाषितम् SQLITE_ENABLE_SORTER_REFERENCES
         || pEList->a[i].fg.bSorterRef
#यदि_अन्तः
        ){
          n_परिणाम_Col--;
          regOrig = 0;
        }
      }

      testcase( regOrig );
      testcase( eDest==SRT_समुच्चयः );
      testcase( eDest==SRT_Mem );
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      प्रतिज्ञा( eDest==SRT_समुच्चयः || eDest==SRT_Mem
           || eDest==SRT_Coroutine || eDest==SRT_Output
           || eDest==SRT_Upfrom );
    }
    s_Row_आरोपय_Info.reg_परिणाम = reg_परिणाम;
    s_Row_आरोपय_Info.ecelFlags = ecelFlags;
#यदि_परिभाषितम् SQLITE_ENABLE_SORTER_REFERENCES
    s_Row_आरोपय_Info.pExtra = pExtra;
    s_Row_आरोपय_Info.reg_Extra_परिणाम = reg_परिणाम + n_परिणाम_Col;
    यदि( pExtra ) n_परिणाम_Col += pExtra->n_व्यञ्जकम्;
#यदि_अन्तः
    यदि( p->i_सीमा
     && (ecelFlags & SQLITE_ECEL_OMITREF)!=0
     && n_उपसर्गः_Reg>0
    ){
      प्रतिज्ञा( p_क्रमय!=0 );
      प्रतिज्ञा( hasDistinct==0 );
      p_क्रमय->p_Deferred_Row_आरोपय = &s_Row_आरोपय_Info;
      regOrig = 0;
    }अन्यथा{
      inner_Loop_आरोपय_Row(pParse, p, &s_Row_आरोपय_Info);
    }
  }

  /* If the DISTINCT keyword was present on the SELECT statement
  ** and this row has been seen before, then do not make this row
  ** part of the result.
  */
  यदि( hasDistinct ){
    पूर्णाङ्क eType = pDistinct->eTnctType;
    पूर्णाङ्क iTab = pDistinct->tabTnct;
    प्रतिज्ञा( n_परिणाम_Col==p->pEList->n_व्यञ्जकम् );
    iTab = संहिता_Distinct(pParse, eType, iTab, iContinue, p->pEList, reg_परिणाम);
    fixDistinctOpenEph(pParse, eType, iTab, pDistinct->addrTnct);
    यदि( p_क्रमय==0 ){
      संहिता_विस्थापनम्(v, p->i_विस्थापनम्, iContinue);
    }
  }

  विकल्प( eDest ){
    /* Store the result as data using a unique key.
    */
    पक्ष SRT_Fifo:
    पक्ष SRT_DistFifo:
    पक्ष SRT_सारणी:
    पक्ष SRT_EphemTab: {
      पूर्णाङ्क r1 = sqlite3_Get_अस्थायी_Range(pParse, n_उपसर्गः_Reg+1);
      testcase( eDest==SRT_सारणी );
      testcase( eDest==SRT_EphemTab );
      testcase( eDest==SRT_Fifo );
      testcase( eDest==SRT_DistFifo );
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, reg_परिणाम, n_परिणाम_Col, r1+n_उपसर्गः_Reg);
#यदि !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      यदि( eDest==SRT_सारणी && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_CTE
      यदि( eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index. If the current row is already present
        ** in the index, do not write it to the output. If not, add the
        ** current row to the index and proceed with writing it to the
        ** output table as well.  */
        पूर्णाङ्क addr = sqlite3VdbeCurrentAddr(v) + 4;
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_प्राप्तम्, iParm+1, addr, r1, 0);
        VdbeCoverage(v);
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, iParm+1, r1,reg_परिणाम,n_परिणाम_Col);
        प्रतिज्ञा( p_क्रमय==0 );
      }
#यदि_अन्तः
      यदि( p_क्रमय ){
        प्रतिज्ञा( reg_परिणाम==regOrig );
        प्रक्षिप_Onto_Sorter(pParse, p_क्रमय, p, r1+n_उपसर्गः_Reg, regOrig, 1, n_उपसर्गः_Reg);
      }अन्यथा{
        पूर्णाङ्क r2 = sqlite3_Get_अस्थायी_Reg(pParse);
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_NewRowid, iParm, r2);
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_निवेशय, iParm, r1, r2);
        sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
        sqlite3_Release_अस्थायी_Reg(pParse, r2);
      }
      sqlite3_Release_अस्थायी_Range(pParse, r1, n_उपसर्गः_Reg+1);
      भङ्ग;
    }

    पक्ष SRT_Upfrom: {
      यदि( p_क्रमय ){
        प्रक्षिप_Onto_Sorter(
            pParse, p_क्रमय, p, reg_परिणाम, regOrig, n_परिणाम_Col, n_उपसर्गः_Reg);
      }अन्यथा{
        पूर्णाङ्क i2 = pDest->iSDParm2;
        पूर्णाङ्क r1 = sqlite3_Get_अस्थायी_Reg(pParse);

        /* If the UPDATE FROM join is an aggregate that matches no rows, it
        ** might still be trying to return one row, because that is what
        ** aggregates do.  Don't record that empty row in the output table. */
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IsNull, reg_परिणाम, iBreak); VdbeCoverage(v);

        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः,
                          reg_परिणाम+(i2<0), n_परिणाम_Col-(i2<0), r1);
        यदि( i2<0 ){
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_निवेशय, iParm, r1, reg_परिणाम);
        }अन्यथा{
          sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, iParm, r1, reg_परिणाम, i2);
        }
      }
      भङ्ग;
    }

#यदि_अपरिभाषितम् SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)" construct,
    ** then there should be a single item on the stack.  Write this
    ** item into the set table with bogus data.
    */
    पक्ष SRT_समुच्चयः: {
      यदि( p_क्रमय ){
        /* At first glance you would think we could optimize out the
        ** ORDER BY in this case since the order of entries in the set
        ** does not matter.  But there might be a LIMIT clause, in which
        ** case the order does matter */
        प्रक्षिप_Onto_Sorter(
            pParse, p_क्रमय, p, reg_परिणाम, regOrig, n_परिणाम_Col, n_उपसर्गः_Reg);
        pDest->iSDParm2 = 0; /* Signal that any Bloom filter is unpopulated */
      }अन्यथा{
        पूर्णाङ्क r1 = sqlite3_Get_अस्थायी_Reg(pParse);
        प्रतिज्ञा( sqlite3Strlen30(pDest->zAffSdst)==n_परिणाम_Col );
        sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Make_लेखः, reg_परिणाम, n_परिणाम_Col,
            r1, pDest->zAffSdst, n_परिणाम_Col);
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, iParm, r1, reg_परिणाम, n_परिणाम_Col);
        यदि( pDest->iSDParm2 ){
          प्रतिज्ञा( sqlite3CursorBloomable(pParse,iParm,n_परिणाम_Col) );
          sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Filter_सङ्कलय, pDest->iSDParm2, 0,
                               reg_परिणाम, n_परिणाम_Col);
          ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
        }
        sqlite3_Release_अस्थायी_Reg(pParse, r1);
      }
      भङ्ग;
    }


    /* If any row exist in the result set, record that fact and abort.
    */
    पक्ष SRT_Exists: {
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 1, iParm);
      /* The LIMIT clause will terminate the loop for us */
      भङ्ग;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell or array of
    ** memory cells and break out of the scan loop.
    */
    पक्ष SRT_Mem: {
      यदि( p_क्रमय ){
        प्रतिज्ञा( n_परिणाम_Col<=pDest->nSdst );
        प्रक्षिप_Onto_Sorter(
            pParse, p_क्रमय, p, reg_परिणाम, regOrig, n_परिणाम_Col, n_उपसर्गः_Reg);
        pDest->iSDParm = reg_परिणाम;
      }अन्यथा{
        प्रतिज्ञा( n_परिणाम_Col==pDest->nSdst );
        यदि( reg_परिणाम!=iParm ){
          /* This occurs in cases where the SELECT had both a DISTINCT and
          ** an OFFSET clause.  */
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Copy, reg_परिणाम, iParm, n_परिणाम_Col-1);
        }
        /* The LIMIT clause will jump out of the loop for us */
      }
      भङ्ग;
    }
#यदि_अन्तः /* #ifndef SQLITE_OMIT_SUBQUERY */

    पक्ष SRT_Coroutine:       /* Send data to a co-routine */
    पक्ष SRT_Output: {        /* Return the results */
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      यदि( p_क्रमय ){
        प्रक्षिप_Onto_Sorter(pParse, p_क्रमय, p, reg_परिणाम, regOrig, n_परिणाम_Col,
                       n_उपसर्गः_Reg);
      }अन्यथा यदि( eDest==SRT_Coroutine ){
        sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Yield, pDest->iSDParm);
      }अन्यथा{
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, reg_परिणाम, n_परिणाम_Col);
      }
      भङ्ग;
    }

#यदि_अपरिभाषितम् SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    पक्ष SRT_DistQueue:
    पक्ष SRT_Queue: {
      पूर्णाङ्क n_कुञ्चिका;
      पूर्णाङ्क r1, r2, r3;
      पूर्णाङ्क addrTest = 0;
      व्यञ्जकम्_List *pSO;
      pSO = pDest->p_क्रमः_By;
      प्रतिज्ञा( pSO );
      n_कुञ्चिका = pSO->n_व्यञ्जकम्;
      r1 = sqlite3_Get_अस्थायी_Reg(pParse);
      r2 = sqlite3_Get_अस्थायी_Range(pParse, n_कुञ्चिका+2);
      r3 = r2+n_कुञ्चिका+1;
      यदि( eDest==SRT_DistQueue ){
        /* If the destination is DistQueue, then cursor (iParm+1) is open
        ** on a second ephemeral index that holds all values every previously
        ** added to the queue. */
        addrTest = sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_प्राप्तम्, iParm+1, 0,
                                        reg_परिणाम, n_परिणाम_Col);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, reg_परिणाम, n_परिणाम_Col, r3);
      यदि( eDest==SRT_DistQueue ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Idx_निवेशय, iParm+1, r3);
        sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      }
      प्रति(i=0; i<n_कुञ्चिका; i++){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_SCopy,
                          reg_परिणाम + pSO->a[i].u.x.i_क्रमः_By_Col - 1,
                          r2+i);
      }
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Sequence, iParm, r2+n_कुञ्चिका);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, r2, n_कुञ्चिका+2, r1);
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, iParm, r1, r2, n_कुञ्चिका+2);
      यदि( addrTest ) sqlite3_Vdbe_Jump_अत्र(v, addrTest);
      sqlite3_Release_अस्थायी_Reg(pParse, r1);
      sqlite3_Release_अस्थायी_Range(pParse, r2, n_कुञ्चिका+2);
      भङ्ग;
    }
#यदि_अन्तः /* SQLITE_OMIT_CTE */



#यदि !defined(SQLITE_OMIT_TRIGGER)
    /* Discard the results.  This is used for SELECT statements inside
    ** the body of a TRIGGER.  The purpose of such selects is to call
    ** user-defined functions that have side effects.  We do not care
    ** about the actual results of the select.
    */
    स्वभाव: {
      प्रतिज्ञा( eDest==SRT_Discard );
      भङ्ग;
    }
#यदि_अन्तः
  }

  /* Jump to the end of the loop if the LIMIT is reached.  Except, if
  ** there is a sorter, in which case the sorter has already limited
  ** the output for us.
  */
  यदि( p_क्रमय==0 && p->i_सीमा ){
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_DecrJumpZero, p->i_सीमा, iBreak); VdbeCoverage(v);
  }
}

/*
** Allocate a KeyInfo object sufficient for an index of N key columns and
** X extra columns.
*/
कुञ्चिका_Info *sqlite3_कुञ्चिका_Info_Alloc(sqlite3 *db, पूर्णाङ्क N, पूर्णाङ्क X){
  पूर्णाङ्क nExtra = (N+X)*(परिमाण(CollSeq*)+1);
  कुञ्चिका_Info *p;
  प्रतिज्ञा( X>=0 );
  यदि( NEVER(N+X>0xffff) ) प्रत्यागम (कुञ्चिका_Info*)sqlite3OomFault(db);
  p = sqlite3DbMallocRawNN(db, SZ_KEYINFO(0) + nExtra);
  यदि( p ){
    p->a_क्रमय_Flags = (u8*)&p->aColl[N+X];
    p->n_कुञ्चिका_क्षेत्रम् = (u16)N;
    p->n_All_क्षेत्रम् = (u16)(N+X);
    p->enc = ENC(db);
    p->db = db;
    p->nRef = 1;
    स्मृतिं_पूरय(p->aColl, 0, nExtra);
  }अन्यथा{
    प्रत्यागम (कुञ्चिका_Info*)sqlite3OomFault(db);
  }
  प्रत्यागम p;
}

/*
** Deallocate a KeyInfo object
*/
शून्य sqlite3_कुञ्चिका_Info_Unref(कुञ्चिका_Info *p){
  यदि( p ){
    प्रतिज्ञा( p->db!=0 );
    प्रतिज्ञा( p->nRef>0 );
    p->nRef--;
    यदि( p->nRef==0 ) sqlite3DbNNFreeNN(p->db, p);
  }
}

/*
** Make a new pointer to a KeyInfo object
*/
कुञ्चिका_Info *sqlite3_कुञ्चिका_Info_Ref(कुञ्चिका_Info *p){
  यदि( p ){
    प्रतिज्ञा( p->nRef>0 );
    p->nRef++;
  }
  प्रत्यागम p;
}

#यदि_परिभाषितम् SQLITE_DEBUG
/*
** Return TRUE if a KeyInfo object can be change.  The KeyInfo object
** can only be changed if this is just a single reference to the object.
**
** This routine is used only inside of assert() statements.
*/
पूर्णाङ्क sqlite3_कुञ्चिका_Info_Is_Writeable(कुञ्चिका_Info *p){ प्रत्यागम p->nRef==1; }
#यदि_अन्तः /* SQLITE_DEBUG */

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
कुञ्चिका_Info *sqlite3_कुञ्चिका_Info_From_व्यञ्जकम्_List(
  Parse *pParse,       /* Parsing context */
  व्यञ्जकम्_List *pList,     /* Form the KeyInfo object from this ExprList */
  पूर्णाङ्क iStart,          /* Begin with this column of pList */
  पूर्णाङ्क nExtra           /* Add this many extra columns to the end */
){
  पूर्णाङ्क n_व्यञ्जकम्;
  कुञ्चिका_Info *pInfo;
  संरचना व्यञ्जकम्_List_item *pItem;
  sqlite3 *db = pParse->db;
  पूर्णाङ्क i;

  n_व्यञ्जकम् = pList->n_व्यञ्जकम्;
  pInfo = sqlite3_कुञ्चिका_Info_Alloc(db, n_व्यञ्जकम्-iStart, nExtra+1);
  यदि( pInfo ){
    प्रतिज्ञा( sqlite3_कुञ्चिका_Info_Is_Writeable(pInfo) );
    प्रति(i=iStart, pItem=pList->a+iStart; i<n_व्यञ्जकम्; i++, pItem++){
      pInfo->aColl[i-iStart] = sqlite3_व्यञ्जकम्_NNColl_Seq(pParse, pItem->p_व्यञ्जकम्);
      pInfo->a_क्रमय_Flags[i-iStart] = pItem->fg.क्रमय_Flags;
    }
  }
  प्रत्यागम pInfo;
}

/*
** Name of the connection operator, used for error messages.
*/
ध्रुव वर्ण *sqlite3SelectOpName(पूर्णाङ्क id){
  वर्ण *z;
  विकल्प( id ){
    पक्ष TK_ALL:       z = "UNION ALL";   भङ्ग;
    पक्ष TK_INTERSECT: z = "INTERSECT";   भङ्ग;
    पक्ष TK_EXCEPT:    z = "EXCEPT";      भङ्ग;
    स्वभाव:           z = "UNION";       भङ्ग;
  }
  प्रत्यागम z;
}

#यदि_अपरिभाषितम् SQLITE_OMIT_EXPLAIN
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
स्थिर शून्य explain_अस्थायी_सारणी(Parse *pParse, ध्रुव वर्ण *zUsage){
  ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s", zUsage));
}

/*
** Assign expression b to lvalue a. A second, no-op, version of this macro
** is provided when SQLITE_OMIT_EXPLAIN is defined. This allows the code
** in sqlite3Select() to assign values to structure member variables that
** only exist if SQLITE_OMIT_EXPLAIN is not defined without polluting the
** code with #ifndef directives.
*/
# define explain_समुच्चयः_Integer(a, b) a = b

#अन्यथा
/* No-op versions of the explainXXX() functions and macros. */
# define explain_अस्थायी_सारणी(y,z)
# define explain_समुच्चयः_Integer(y,z)
#यदि_अन्तः


/*
** If the inner loop was generated using a non-null pOrderBy argument,
** then the results were placed in a sorter.  After the loop is terminated
** we need to run the sorter and output the results.  The following
** routine generates the code needed to do that.
*/
स्थिर शून्य generate_क्रमय_पुच्छ(
  Parse *pParse,    /* Parsing context */
  Select *p,        /* The SELECT statement */
  क्रमय_Ctx *p_क्रमय,   /* Information on the ORDER BY clause */
  पूर्णाङ्क nColumn,      /* Number of columns of data */
  SelectDest *pDest /* Write the sorted results here */
){
  Vdbe *v = pParse->pVdbe;                     /* The prepared statement */
  पूर्णाङ्क addrBreak = p_क्रमय->label_समाप्तम्;            /* Jump here to exit loop */
  पूर्णाङ्क addrContinue = sqlite3VdbeMakeLabel(pParse);/* Jump here for next cycle */
  पूर्णाङ्क addr;                       /* Top of output loop. Jump for Next. */
  पूर्णाङ्क addrOnce = 0;
  पूर्णाङ्क iTab;
  व्यञ्जकम्_List *p_क्रमः_By = p_क्रमय->p_क्रमः_By;
  पूर्णाङ्क eDest = pDest->eDest;
  पूर्णाङ्क iParm = pDest->iSDParm;
  पूर्णाङ्क regRow;
  पूर्णाङ्क regRowid;
  पूर्णाङ्क iCol;
  पूर्णाङ्क n_कुञ्चिका;                       /* Number of key columns in sorter record */
  पूर्णाङ्क i_क्रमय_Tab;                   /* Sorter cursor to read from */
  पूर्णाङ्क i;
  पूर्णाङ्क bSeq;                       /* True if sorter record includes seq. no. */
  पूर्णाङ्क n_Ref_कुञ्चिका = 0;
  संरचना व्यञ्जकम्_List_item *a_बहिर्गमनम्_Ex = p->pEList->a;
#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
  पूर्णाङ्क addrExplain;                /* Address of OP_Explain instruction */
#यदि_अन्तः

  n_कुञ्चिका = p_क्रमः_By->n_व्यञ्जकम् - p_क्रमय->nOBSat;
  यदि( p_क्रमय->nOBSat==0 || n_कुञ्चिका==1 ){
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR %sORDER BY", p_क्रमय->nOBSat?"LAST TERM OF ":""
    ));
  }अन्यथा{
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR LAST %d TERMS OF ORDER BY", n_कुञ्चिका
    ));
  }
  sqlite3_Vdbe_अवलोकनम्_स्थितिः_Range(v, addrExplain,p_क्रमय->addr_प्रक्षिप,p_क्रमय->addr_प्रक्षिप_अन्तः);
  sqlite3_Vdbe_अवलोकनम्_स्थितिः_Counters(v, addrExplain, addrExplain, p_क्रमय->addr_प्रक्षिप);


  प्रतिज्ञा( addrBreak<0 );
  यदि( p_क्रमय->label_Bk_बहिर्गमनम् ){
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, p_क्रमय->regReturn, p_क्रमय->label_Bk_बहिर्गमनम्);
    sqlite3VdbeGoto(v, addrBreak);
    sqlite3VdbeResolveLabel(v, p_क्रमय->label_Bk_बहिर्गमनम्);
  }

#यदि_परिभाषितम् SQLITE_ENABLE_SORTER_REFERENCES
  /* Open any cursors needed for sorter-reference expressions */
  प्रति(i=0; i<p_क्रमय->nDefer; i++){
    सारणी *pTab = p_क्रमय->aDefer[i].pTab;
    पूर्णाङ्क iDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(pParse->db, pTab->pSchema);
    sqlite3_Open_सारणी(pParse, p_क्रमय->aDefer[i].iCsr, iDb, pTab, OP_OpenRead);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rewind, p_क्रमय->aDefer[i].iCsr,
                      sqlite3VdbeCurrentAddr(v)+1);
    VdbeCoverage(v);
    n_Ref_कुञ्चिका = MAX(n_Ref_कुञ्चिका, p_क्रमय->aDefer[i].n_कुञ्चिका);
  }
#यदि_अन्तः

  iTab = p_क्रमय->iECursor;
  यदि( eDest==SRT_Output || eDest==SRT_Coroutine || eDest==SRT_Mem ){
    यदि( eDest==SRT_Mem && p->i_विस्थापनम् ){
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Null, 0, pDest->iSdst);
    }
    regRowid = 0;
    regRow = pDest->iSdst;
  }अन्यथा{
    regRowid = sqlite3_Get_अस्थायी_Reg(pParse);
    यदि( eDest==SRT_EphemTab || eDest==SRT_सारणी ){
      regRow = sqlite3_Get_अस्थायी_Reg(pParse);
      nColumn = 0;
    }अन्यथा{
      regRow = sqlite3_Get_अस्थायी_Range(pParse, nColumn);
    }
  }
  यदि( p_क्रमय->क्रमय_Flags & SORTFLAG_UseSorter ){
    पूर्णाङ्क reg_क्रमय_बहिर्गमनम् = ++pParse->nMem;
    i_क्रमय_Tab = pParse->nTab++;
    यदि( p_क्रमय->label_Bk_बहिर्गमनम् ){
      addrOnce = sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Once); VdbeCoverage(v);
    }
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_OpenPseudo, i_क्रमय_Tab, reg_क्रमय_बहिर्गमनम्,
        n_कुञ्चिका+1+nColumn+n_Ref_कुञ्चिका);
    यदि( addrOnce ) sqlite3_Vdbe_Jump_अत्र(v, addrOnce);
    addr = 1 + sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Sorter_क्रमय, iTab, addrBreak);
    VdbeCoverage(v);
    प्रतिज्ञा( p->i_सीमा==0 && p->i_विस्थापनम्==0 );
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Sorter_दत्त, iTab, reg_क्रमय_बहिर्गमनम्, i_क्रमय_Tab);
    bSeq = 0;
  }अन्यथा{
    addr = 1 + sqlite3_Vdbe_सङ्कलय_Op2(v, OP_क्रमय, iTab, addrBreak); VdbeCoverage(v);
    संहिता_विस्थापनम्(v, p->i_विस्थापनम्, addrContinue);
    i_क्रमय_Tab = iTab;
    bSeq = 1;
    यदि( p->i_विस्थापनम्>0 ){
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_सङ्कलय_Imm, p->i_सीमा, -1);
    }
  }
  प्रति(i=0, iCol=n_कुञ्चिका+bSeq-1; i<nColumn; i++){
#यदि_परिभाषितम् SQLITE_ENABLE_SORTER_REFERENCES
    यदि( a_बहिर्गमनम्_Ex[i].fg.bSorterRef ) अनुवर्त;
#यदि_अन्तः
    यदि( a_बहिर्गमनम्_Ex[i].u.x.i_क्रमः_By_Col==0 ) iCol++;
  }
#यदि_परिभाषितम् SQLITE_ENABLE_SORTER_REFERENCES
  यदि( p_क्रमय->nDefer ){
    पूर्णाङ्क i_कुञ्चिका = iCol+1;
    पूर्णाङ्क reg_कुञ्चिका = sqlite3_Get_अस्थायी_Range(pParse, n_Ref_कुञ्चिका);

    प्रति(i=0; i<p_क्रमय->nDefer; i++){
      पूर्णाङ्क iCsr = p_क्रमय->aDefer[i].iCsr;
      सारणी *pTab = p_क्रमय->aDefer[i].pTab;
      पूर्णाङ्क n_कुञ्चिका = p_क्रमय->aDefer[i].n_कुञ्चिका;

      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_NullRow, iCsr);
      यदि( HasRowid(pTab) ){
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, i_क्रमय_Tab, i_कुञ्चिका++, reg_कुञ्चिका);
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_SeekRowid, iCsr,
            sqlite3VdbeCurrentAddr(v)+1, reg_कुञ्चिका);
      }अन्यथा{
        पूर्णाङ्क k;
        पूर्णाङ्क iJmp;
        प्रतिज्ञा( sqlite3_Primary_कुञ्चिका_सूचकांक(pTab)->n_कुञ्चिका_Col==n_कुञ्चिका );
        प्रति(k=0; k<n_कुञ्चिका; k++){
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, i_क्रमय_Tab, i_कुञ्चिका++, reg_कुञ्चिका+k);
        }
        iJmp = sqlite3VdbeCurrentAddr(v);
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_SeekGE, iCsr, iJmp+2, reg_कुञ्चिका, n_कुञ्चिका);
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_IdxLE, iCsr, iJmp+3, reg_कुञ्चिका, n_कुञ्चिका);
        sqlite3_Vdbe_सङ्कलय_Op1(v, OP_NullRow, iCsr);
      }
    }
    sqlite3_Release_अस्थायी_Range(pParse, reg_कुञ्चिका, n_Ref_कुञ्चिका);
  }
#यदि_अन्तः
  प्रति(i=nColumn-1; i>=0; i--){
#यदि_परिभाषितम् SQLITE_ENABLE_SORTER_REFERENCES
    यदि( a_बहिर्गमनम्_Ex[i].fg.bSorterRef ){
      sqlite3_व्यञ्जकम्_संहिता(pParse, a_बहिर्गमनम्_Ex[i].p_व्यञ्जकम्, regRow+i);
    }अन्यथा
#यदि_अन्तः
    {
      पूर्णाङ्क iRead;
      यदि( a_बहिर्गमनम्_Ex[i].u.x.i_क्रमः_By_Col ){
        iRead = a_बहिर्गमनम्_Ex[i].u.x.i_क्रमः_By_Col-1;
      }अन्यथा{
        iRead = iCol--;
      }
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, i_क्रमय_Tab, iRead, regRow+i);
      VdbeComment((v, "%s", a_बहिर्गमनम्_Ex[i].zEName));
    }
  }
  sqlite3_Vdbe_अवलोकनम्_स्थितिः_Range(v, addrExplain, addrExplain, -1);
  विकल्प( eDest ){
    पक्ष SRT_सारणी:
    पक्ष SRT_EphemTab: {
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, i_क्रमय_Tab, n_कुञ्चिका+bSeq, regRow);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_NewRowid, iParm, regRowid);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_निवेशय, iParm, regRow, regRowid);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      भङ्ग;
    }
#यदि_अपरिभाषितम् SQLITE_OMIT_SUBQUERY
    पक्ष SRT_समुच्चयः: {
      प्रतिज्ञा( nColumn==sqlite3Strlen30(pDest->zAffSdst) );
      sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Make_लेखः, regRow, nColumn, regRowid,
                        pDest->zAffSdst, nColumn);
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, iParm, regRowid, regRow, nColumn);
      भङ्ग;
    }
    पक्ष SRT_Mem: {
      /* The LIMIT clause will terminate the loop for us */
      भङ्ग;
    }
#यदि_अन्तः
    पक्ष SRT_Upfrom: {
      पूर्णाङ्क i2 = pDest->iSDParm2;
      पूर्णाङ्क r1 = sqlite3_Get_अस्थायी_Reg(pParse);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः,regRow+(i2<0),nColumn-(i2<0),r1);
      यदि( i2<0 ){
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_निवेशय, iParm, r1, regRow);
      }अन्यथा{
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, iParm, r1, regRow, i2);
      }
      भङ्ग;
    }
    स्वभाव: {
      प्रतिज्ञा( eDest==SRT_Output || eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      testcase( eDest==SRT_Coroutine );
      यदि( eDest==SRT_Output ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, pDest->iSdst, nColumn);
      }अन्यथा{
        sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Yield, pDest->iSDParm);
      }
      भङ्ग;
    }
  }
  यदि( regRowid ){
    यदि( eDest==SRT_समुच्चयः ){
      sqlite3_Release_अस्थायी_Range(pParse, regRow, nColumn);
    }अन्यथा{
      sqlite3_Release_अस्थायी_Reg(pParse, regRow);
    }
    sqlite3_Release_अस्थायी_Reg(pParse, regRowid);
  }
  /* The bottom of the loop
  */
  sqlite3VdbeResolveLabel(v, addrContinue);
  यदि( p_क्रमय->क्रमय_Flags & SORTFLAG_UseSorter ){
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Sorter_अग्र, iTab, addr); VdbeCoverage(v);
  }अन्यथा{
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_अग्र, iTab, addr); VdbeCoverage(v);
  }
  sqlite3_Vdbe_अवलोकनम्_स्थितिः_Range(v, addrExplain, sqlite3VdbeCurrentAddr(v)-1, -1);
  यदि( p_क्रमय->regReturn ) sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Return, p_क्रमय->regReturn);
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
#यदि_परिभाषितम् SQLITE_ENABLE_COLUMN_METADATA
# define columnType(A,B,C,D,E) columnTypeImpl(A,B,C,D,E)
#अन्यथा /* if !defined(SQLITE_ENABLE_COLUMN_METADATA) */
# define columnType(A,B,C,D,E) columnTypeImpl(A,B)
#यदि_अन्तः
स्थिर ध्रुव वर्ण *columnTypeImpl(
  NameContext *pNC,
#यदि_अपरिभाषितम् SQLITE_ENABLE_COLUMN_METADATA
  व्यञ्जकम् *p_व्यञ्जकम्
#अन्यथा
  व्यञ्जकम् *p_व्यञ्जकम्,
  ध्रुव वर्ण **pzOrigDb,
  ध्रुव वर्ण **pzOrigTab,
  ध्रुव वर्ण **pzOrigCol
#यदि_अन्तः
){
  वर्ण ध्रुव *zType = 0;
  पूर्णाङ्क j;
#यदि_परिभाषितम् SQLITE_ENABLE_COLUMN_METADATA
  वर्ण ध्रुव *zOrigDb = 0;
  वर्ण ध्रुव *zOrigTab = 0;
  वर्ण ध्रुव *zOrigCol = 0;
#यदि_अन्तः

  प्रतिज्ञा( p_व्यञ्जकम्!=0 );
  प्रतिज्ञा( pNC->pSrcList!=0 );
  विकल्प( p_व्यञ्जकम्->op ){
    पक्ष TK_COLUMN: {
      /* The expression is a column. Locate the table the column is being
      ** extracted from in NameContext.pSrcList. This table may be real
      ** database table or a subquery.
      */
      सारणी *pTab = 0;            /* Table structure column is extracted from */
      Select *pS = 0;             /* Select the column is extracted from */
      पूर्णाङ्क iCol = p_व्यञ्जकम्->iColumn;  /* Index of column in pTab */
      यावत्( pNC && !pTab ){
        SrcList *pTabList = pNC->pSrcList;
        प्रति(j=0;j<pTabList->nSrc && pTabList->a[j].iCursor!=p_व्यञ्जकम्->i_सारणी;j++);
        यदि( j<pTabList->nSrc ){
          pTab = pTabList->a[j].pSTab;
          यदि( pTabList->a[j].fg.isSubquery ){
            pS = pTabList->a[j].u4.pSubq->pSelect;
          }अन्यथा{
            pS = 0;
          }
        }अन्यथा{
          pNC = pNC->p_अग्र;
        }
      }

      यदि( pTab==0 ){
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
        भङ्ग;
      }

      प्रतिज्ञा( pTab && व्यञ्जकम्_Use_YTab(p_व्यञ्जकम्) && p_व्यञ्जकम्->y.pTab==pTab );
      यदि( pS ){
        /* The "table" is actually a sub-select or a view in the FROM clause
        ** of the SELECT statement. Return the declaration type and origin
        ** data for the result-set column of the sub-select.
        */
        यदि( iCol<pS->pEList->n_व्यञ्जकम्
         && (!ViewCanHaveRowid || iCol>=0)
        ){
          /* If iCol is less than zero, then the expression requests the
          ** rowid of the sub-select or view. This expression is legal (see
          ** test case misc2.2.2) - it always evaluates to NULL.
          */
          NameContext sNC;
          व्यञ्जकम् *p = pS->pEList->a[iCol].p_व्यञ्जकम्;
          sNC.pSrcList = pS->pSrc;
          sNC.p_अग्र = pNC;
          sNC.pParse = pNC->pParse;
          zType = columnType(&sNC, p,&zOrigDb,&zOrigTab,&zOrigCol);
        }
      }अन्यथा{
        /* A real table or a CTE table */
        प्रतिज्ञा( !pS );
#यदि_परिभाषितम् SQLITE_ENABLE_COLUMN_METADATA
        यदि( iCol<0 ) iCol = pTab->iPKey;
        प्रतिज्ञा( iCol==XN_ROWID || (iCol>=0 && iCol<pTab->nCol) );
        यदि( iCol<0 ){
          zType = "INTEGER";
          zOrigCol = "rowid";
        }अन्यथा{
          zOrigCol = pTab->aCol[iCol].zCnName;
          zType = sqlite3ColumnType(&pTab->aCol[iCol],0);
        }
        zOrigTab = pTab->zName;
        यदि( pNC->pParse && pTab->pSchema ){
          पूर्णाङ्क iDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(pNC->pParse->db, pTab->pSchema);
          zOrigDb = pNC->pParse->db->aDb[iDb].zDbSName;
        }
#अन्यथा
        प्रतिज्ञा( iCol==XN_ROWID || (iCol>=0 && iCol<pTab->nCol) );
        यदि( iCol<0 ){
          zType = "INTEGER";
        }अन्यथा{
          zType = sqlite3ColumnType(&pTab->aCol[iCol],0);
        }
#यदि_अन्तः
      }
      भङ्ग;
    }
#यदि_अपरिभाषितम् SQLITE_OMIT_SUBQUERY
    पक्ष TK_SELECT: {
      /* The expression is a sub-select. Return the declaration type and
      ** origin info for the single column in the result set of the SELECT
      ** statement.
      */
      NameContext sNC;
      Select *pS;
      व्यञ्जकम् *p;
      प्रतिज्ञा( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) );
      pS = p_व्यञ्जकम्->x.pSelect;
      p = pS->pEList->a[0].p_व्यञ्जकम्;
      sNC.pSrcList = pS->pSrc;
      sNC.p_अग्र = pNC;
      sNC.pParse = pNC->pParse;
      zType = columnType(&sNC, p, &zOrigDb, &zOrigTab, &zOrigCol);
      भङ्ग;
    }
#यदि_अन्तः
  }

#यदि_परिभाषितम् SQLITE_ENABLE_COLUMN_METADATA 
  यदि( pzOrigDb ){
    प्रतिज्ञा( pzOrigTab && pzOrigCol );
    *pzOrigDb = zOrigDb;
    *pzOrigTab = zOrigTab;
    *pzOrigCol = zOrigCol;
  }
#यदि_अन्तः
  प्रत्यागम zType;
}

/*
** Generate code that will tell the VDBE the declaration types of columns
** in the result set.
*/
स्थिर शून्य generateColumnTypes(
  Parse *pParse,      /* Parser context */
  SrcList *pTabList,  /* List of tables */
  व्यञ्जकम्_List *pEList    /* Expressions defining the result set */
){
#यदि_अपरिभाषितम् SQLITE_OMIT_DECLTYPE
  Vdbe *v = pParse->pVdbe;
  पूर्णाङ्क i;
  NameContext sNC;
  sNC.pSrcList = pTabList;
  sNC.pParse = pParse;
  sNC.p_अग्र = 0;
  प्रति(i=0; i<pEList->n_व्यञ्जकम्; i++){
    व्यञ्जकम् *p = pEList->a[i].p_व्यञ्जकम्;
    ध्रुव वर्ण *zType;
#यदि_परिभाषितम् SQLITE_ENABLE_COLUMN_METADATA
    ध्रुव वर्ण *zOrigDb = 0;
    ध्रुव वर्ण *zOrigTab = 0;
    ध्रुव वर्ण *zOrigCol = 0;
    zType = columnType(&sNC, p, &zOrigDb, &zOrigTab, &zOrigCol);

    /* The vdbe must make its own copy of the column-type and other
    ** column specific strings, in case the schema is reset before this
    ** virtual machine is deleted.
    */
    sqlite3_Vdbe_समुच्चयः_Col_Name(v, i, COLNAME_DATABASE, zOrigDb, SQLITE_TRANSIENT);
    sqlite3_Vdbe_समुच्चयः_Col_Name(v, i, COLNAME_सारणी, zOrigTab, SQLITE_TRANSIENT);
    sqlite3_Vdbe_समुच्चयः_Col_Name(v, i, COLNAME_COLUMN, zOrigCol, SQLITE_TRANSIENT);
#अन्यथा
    zType = columnType(&sNC, p, 0, 0, 0);
#यदि_अन्तः
    sqlite3_Vdbe_समुच्चयः_Col_Name(v, i, COLNAME_DECLTYPE, zType, SQLITE_TRANSIENT);
  }
#अन्यथा
  अप्रयुक्तम्_PARAMETER(pParse);
  अप्रयुक्तम्_PARAMETER(pTabList);
  अप्रयुक्तम्_PARAMETER(pEList);
#यदि_अन्तः /* !defined(SQLITE_OMIT_DECLTYPE) */
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
शून्य sqlite3_Generate_Column_नामानि(
  Parse *pParse,      /* Parser context */
  Select *pSelect     /* Generate column names for this SELECT statement */
){
  Vdbe *v = pParse->pVdbe;
  पूर्णाङ्क i;
  सारणी *pTab;
  SrcList *pTabList;
  व्यञ्जकम्_List *pEList;
  sqlite3 *db = pParse->db;
  पूर्णाङ्क पूर्णम्_Name;    /* TABLE.COLUMN if no AS clause and is a direct table ref */
  पूर्णाङ्क srcName;     /* COLUMN or TABLE.COLUMN if no AS clause and is direct */

  यदि( pParse->col_नामानि_समुच्चयः ) प्रत्यागम;
  /* Column names are determined by the left-most term of a compound select */
  यावत्( pSelect->p_प्राक् ) pSelect = pSelect->p_प्राक्;
  TREETRACE(0x80,pParse,pSelect,("generating column names\n"));
  pTabList = pSelect->pSrc;
  pEList = pSelect->pEList;
  प्रतिज्ञा( v!=0 );
  प्रतिज्ञा( pTabList!=0 );
  pParse->col_नामानि_समुच्चयः = 1;
  पूर्णम्_Name = (db->flags & SQLITE_पूर्णम्_Col_नामानि)!=0;
  srcName = (db->flags & SQLITE_Short_Col_नामानि)!=0 || पूर्णम्_Name;
  sqlite3_Vdbe_समुच्चयः_अङ्कः_स्तम्भसङ्ख्या(v, pEList->n_व्यञ्जकम्);
  प्रति(i=0; i<pEList->n_व्यञ्जकम्; i++){
    व्यञ्जकम् *p = pEList->a[i].p_व्यञ्जकम्;

    प्रतिज्ञा( p!=0 );
    प्रतिज्ञा( p->op!=TK_AGG_COLUMN );  /* Agg processing has not run yet */
    प्रतिज्ञा( p->op!=TK_COLUMN
        || (व्यञ्जकम्_Use_YTab(p) && p->y.pTab!=0) ); /* Covering idx not yet coded */
    यदि( pEList->a[i].zEName && pEList->a[i].fg.eEName==ENAME_NAME ){
      /* An AS clause always takes first priority */
      वर्ण *zName = pEList->a[i].zEName;
      sqlite3_Vdbe_समुच्चयः_Col_Name(v, i, COLNAME_NAME, zName, SQLITE_TRANSIENT);
    }अन्यथा यदि( srcName && p->op==TK_COLUMN ){
      वर्ण *zCol;
      पूर्णाङ्क iCol = p->iColumn;
      pTab = p->y.pTab;
      प्रतिज्ञा( pTab!=0 );
      यदि( iCol<0 ) iCol = pTab->iPKey;
      प्रतिज्ञा( iCol==-1 || (iCol>=0 && iCol<pTab->nCol) );
      यदि( iCol<0 ){
        zCol = "rowid";
      }अन्यथा{
        zCol = pTab->aCol[iCol].zCnName;
      }
      यदि( पूर्णम्_Name ){
        वर्ण *zName = 0;
        zName = sqlite3MPrintf(db, "%s.%s", pTab->zName, zCol);
        sqlite3_Vdbe_समुच्चयः_Col_Name(v, i, COLNAME_NAME, zName, SQLITE_DYNAMIC);
      }अन्यथा{
        sqlite3_Vdbe_समुच्चयः_Col_Name(v, i, COLNAME_NAME, zCol, SQLITE_TRANSIENT);
      }
    }अन्यथा{
      ध्रुव वर्ण *z = pEList->a[i].zEName;
      z = z==0 ? sqlite3MPrintf(db, "column%d", i+1) : sqlite3DbStrDup(db, z);
      sqlite3_Vdbe_समुच्चयः_Col_Name(v, i, COLNAME_NAME, z, SQLITE_DYNAMIC);
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
पूर्णाङ्क sqlite3_Columns_From_व्यञ्जकम्_List(
  Parse *pParse,          /* Parsing context */
  व्यञ्जकम्_List *pEList,       /* Expr list from which to derive column names */
  i16 *pnCol,             /* Write the number of columns here */
  Column **paCol          /* Write the new column list here */
){
  sqlite3 *db = pParse->db;   /* Database connection */
  पूर्णाङ्क i, j;                   /* Loop counters */
  u32 cnt;                    /* Index added to make the name unique */
  Column *aCol, *pCol;        /* For looping over result columns */
  पूर्णाङ्क nCol;                   /* Number of columns in the result set */
  वर्ण *zName;                /* Column name */
  पूर्णाङ्क nName;                  /* Size of name in zName[] */
  Hash ht;                    /* Hash table of column names */
  सारणी *pTab;

  sqlite3HashInit(&ht);
  यदि( pEList ){
    nCol = pEList->n_व्यञ्जकम्;
    aCol = sqlite3DbMallocZero(db, परिमाण(aCol[0])*nCol);
    testcase( aCol==0 );
    यदि( nCol>32767 ) nCol = 32767;
  }अन्यथा{
    nCol = 0;
    aCol = 0;
  }
  प्रतिज्ञा( nCol==(i16)nCol );
  *pnCol = nCol;
  *paCol = aCol;

  प्रति(i=0, pCol=aCol; i<nCol && !pParse->nErr; i++, pCol++){
    संरचना व्यञ्जकम्_List_item *pX = &pEList->a[i];
    संरचना व्यञ्जकम्_List_item *pCollide;
    /* Get an appropriate name for the column
    */
    यदि( (zName = pX->zEName)!=0 && pX->fg.eEName==ENAME_NAME ){
      /* If the column contains an "AS <name>" phrase, use <name> as the name */
    }अन्यथा{
      व्यञ्जकम् *p_Col_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(pX->p_व्यञ्जकम्);
      यावत्( ALWAYS(p_Col_व्यञ्जकम्!=0) && p_Col_व्यञ्जकम्->op==TK_DOT ){
        p_Col_व्यञ्जकम् = p_Col_व्यञ्जकम्->p_दक्षिणः;
        प्रतिज्ञा( p_Col_व्यञ्जकम्!=0 );
      }
      यदि( p_Col_व्यञ्जकम्->op==TK_COLUMN
       && ALWAYS( व्यञ्जकम्_Use_YTab(p_Col_व्यञ्जकम्) )
       && ALWAYS( p_Col_व्यञ्जकम्->y.pTab!=0 )
      ){
        /* For columns use the column name name */
        पूर्णाङ्क iCol = p_Col_व्यञ्जकम्->iColumn;
        pTab = p_Col_व्यञ्जकम्->y.pTab;
        यदि( iCol<0 ) iCol = pTab->iPKey;
        zName = iCol>=0 ? pTab->aCol[iCol].zCnName : "rowid";
      }अन्यथा यदि( p_Col_व्यञ्जकम्->op==TK_ID ){
        प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_Col_व्यञ्जकम्, EP_Int_मानम्) );
        zName = p_Col_व्यञ्जकम्->u.zToken;
      }अन्यथा{
        /* Use the original text of the column expression as its name */
        प्रतिज्ञा( zName==pX->zEName );  /* pointer comparison intended */
      }
    }
    यदि( zName && !sqlite3IsTrueOrFalse(zName) ){
      zName = sqlite3DbStrDup(db, zName);
    }अन्यथा{
      zName = sqlite3MPrintf(db,"column%d",i+1);
    }

    /* Make sure the column name is unique.  If the name is not unique,
    ** append an integer to the name so that it becomes unique.
    */
    cnt = 0;
    यावत्( zName && (pCollide = sqlite3HashFind(&ht, zName))!=0 ){
      यदि( pCollide->fg.b_Using_पदांशः ){
        pCol->colFlags |= COLFLAG_NOEXPAND;
      }
      nName = sqlite3Strlen30(zName);
      यदि( nName>0 ){
        प्रति(j=nName-1; j>0 && sqlite3Isdigit(zName[j]); j--){}
        यदि( zName[j]==':' ) nName = j;
      }
      zName = sqlite3MPrintf(db, "%.*z:%u", nName, zName, ++cnt);
      sqlite3_प्रगतिः_Check(pParse);
      यदि( cnt>3 ){
        sqlite3_randomness(परिमाण(cnt), &cnt);
      }
    }
    pCol->zCnName = zName;
    pCol->hName = sqlite3StrIHash(zName);
    यदि( pX->fg.bNoExpand ){
      pCol->colFlags |= COLFLAG_NOEXPAND;
    }
    sqlite3ColumnPropertiesFromName(0, pCol);
    यदि( zName && sqlite3_Hash_निवेशय(&ht, zName, pX)==pX ){
      sqlite3OomFault(db);
    }
  }
  sqlite3HashClear(&ht);
  यदि( pParse->nErr ){
    प्रति(j=0; j<i; j++){
      sqlite3DbFree(db, aCol[j].zCnName);
    }
    sqlite3DbFree(db, aCol);
    *paCol = 0;
    *pnCol = 0;
    प्रत्यागम pParse->rc;
  }
  प्रत्यागम SQLITE_OK;
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
शून्य sqlite3SubqueryColumnTypes(
  Parse *pParse,      /* Parsing contexts */
  सारणी *pTab,        /* Add column type information to this table */
  Select *pSelect,    /* SELECT used to determine types and collations */
  वर्ण aff            /* Default affinity. */
){
  sqlite3 *db = pParse->db;
  Column *pCol;
  CollSeq *pColl;
  पूर्णाङ्क i,j;
  व्यञ्जकम् *p;
  संरचना व्यञ्जकम्_List_item *a;
  NameContext sNC;

  प्रतिज्ञा( pSelect!=0 );
  प्रतिज्ञा( (pSelect->selFlags & SF_Resolved)!=0 );
  प्रतिज्ञा( pTab->nCol==pSelect->pEList->n_व्यञ्जकम् || pParse->nErr>0 );
  प्रतिज्ञा( aff==SQLITE_AFF_NONE || aff==SQLITE_AFF_BLOB );
  यदि( db->mallocFailed || अन्तर्_RENAME_OBJECT ) प्रत्यागम;
  यावत्( pSelect->p_प्राक् ) pSelect = pSelect->p_प्राक्;
  a = pSelect->pEList->a;
  स्मृतिं_पूरय(&sNC, 0, परिमाण(sNC));
  sNC.pSrcList = pSelect->pSrc;
  प्रति(i=0, pCol=pTab->aCol; i<pTab->nCol; i++, pCol++){
    ध्रुव वर्ण *zType;
    i64 n;
    पूर्णाङ्क m = 0;
    Select *pS2 = pSelect;
    pTab->tabFlags |= (pCol->colFlags & COLFLAG_NOINSERT);
    p = a[i].p_व्यञ्जकम्;
    /* pCol->szEst = ... // Column size est for SELECT tables never used */
    pCol->affinity = sqlite3_व्यञ्जकम्_Affinity(p);
    यावत्( pCol->affinity<=SQLITE_AFF_NONE && pS2->p_अग्र!=0 ){
      m |= sqlite3_व्यञ्जकम्_दत्त_Type(pS2->pEList->a[i].p_व्यञ्जकम्);
      pS2 = pS2->p_अग्र;
      pCol->affinity = sqlite3_व्यञ्जकम्_Affinity(pS2->pEList->a[i].p_व्यञ्जकम्);
    }
    यदि( pCol->affinity<=SQLITE_AFF_NONE ){
      pCol->affinity = aff;
    }
    यदि( pCol->affinity>=SQLITE_AFF_TEXT && (pS2->p_अग्र || pS2!=pSelect) ){
      प्रति(pS2=pS2->p_अग्र; pS2; pS2=pS2->p_अग्र){
        m |= sqlite3_व्यञ्जकम्_दत्त_Type(pS2->pEList->a[i].p_व्यञ्जकम्);
      }
      यदि( pCol->affinity==SQLITE_AFF_TEXT && (m&0x01)!=0 ){
        pCol->affinity = SQLITE_AFF_BLOB;
      }अन्यथा
      यदि( pCol->affinity>=SQLITE_AFF_NUMERIC && (m&0x02)!=0 ){
        pCol->affinity = SQLITE_AFF_BLOB;
      }
      यदि( pCol->affinity>=SQLITE_AFF_NUMERIC && p->op==TK_CAST ){
        pCol->affinity = SQLITE_AFF_FLEXNUM;
      }
    }
    zType = columnType(&sNC, p, 0, 0, 0);
    यदि( zType==0 || pCol->affinity!=sqlite3AffinityType(zType, 0) ){
      यदि( pCol->affinity==SQLITE_AFF_NUMERIC
       || pCol->affinity==SQLITE_AFF_FLEXNUM
      ){
        zType = "NUM";
      }अन्यथा{
        zType = 0;
        प्रति(j=1; j<SQLITE_N_STDTYPE; j++){
          यदि( sqlite3StdTypeAffinity[j]==pCol->affinity ){
            zType = sqlite3StdType[j];
            भङ्ग;
          }
        }
      }
    }
    यदि( zType ){
      ध्रुव i64 k = वाक्यदैर्घ्यम्(zType);
      n = वाक्यदैर्घ्यम्(pCol->zCnName);
      pCol->zCnName = sqlite3DbReallocOrFree(db, pCol->zCnName, n+k+2);
      pCol->colFlags &= ~(COLFLAG_HASTYPE|COLFLAG_HASCOLL);
      यदि( pCol->zCnName ){
        स्मृतिं_प्रतिलिख(&pCol->zCnName[n+1], zType, k+1);
        pCol->colFlags |= COLFLAG_HASTYPE;
      }
    }
    pColl = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, p);
    यदि( pColl ){
      प्रतिज्ञा( pTab->p_सूचकांक==0 );
      sqlite3_Column_समुच्चयः_Coll(db, pCol, pColl->zName);
    }
  }
  pTab->szTabRow = 1; /* Any non-zero value works */
}

/*
** Check the current subquery nesting depth.  Return true and set an
** error if it has gone too deep.
*/
स्थिर पूर्णाङ्क check_Subquery_Nesting_गभीरता(Parse *pParse){
#यदि SQLITE_MAX_व्यञ्जकम्_गभीरता>0
  यदि( pParse->nNestSel >= pParse->db->a_सीमा[SQLITE_सीमा_व्यञ्जकम्_गभीरता] ){
    sqlite3_Error_सन्देशः(pParse, "VIEWs and/or subqueries nested too deep");
    प्रत्यागम 1;
  }
#यदि_अन्तः
  प्रत्यागम 0;
}

/*
** Given a SELECT statement, generate a Table structure that describes
** the result set of that SELECT.
*/
सारणी *sqlite3_परिणाम_समुच्चयः_सम्बन्धि_Select(Parse *pParse, Select *pSelect, वर्ण aff){
  सारणी *pTab;
  sqlite3 *db = pParse->db;
  u64 savedFlags;

  pParse->nNestSel++;
  यदि( check_Subquery_Nesting_गभीरता(pParse) ) प्रत्यागम 0;
  savedFlags = db->flags;
  db->flags &= ~(u64)SQLITE_पूर्णम्_Col_नामानि;
  db->flags |= SQLITE_Short_Col_नामानि;
  sqlite3SelectPrep(pParse, pSelect, 0);
  db->flags = savedFlags;
  यदि( pParse->nErr ) प्रत्यागम 0;
  यावत्( pSelect->p_प्राक् ) pSelect = pSelect->p_प्राक्;
  pTab = sqlite3DbMallocZero(db, परिमाण(सारणी) );
  यदि( pTab==0 ){
    प्रत्यागम 0;
  }
  pTab->nTabRef = 1;
  pTab->zName = 0;
  pTab->nRowLogEst = 200; प्रतिज्ञा( 200==sqlite3LogEst(1048576) );
  sqlite3_Columns_From_व्यञ्जकम्_List(pParse, pSelect->pEList, &pTab->nCol, &pTab->aCol);
  sqlite3SubqueryColumnTypes(pParse, pTab, pSelect, aff);
  pTab->iPKey = -1;
  यदि( db->mallocFailed ){
    sqlite3_Delete_सारणी(db, pTab);
    प्रत्यागम 0;
  }
  pParse->nNestSel--;
  प्रतिज्ञा( pParse->nNestSel>=0 );
  प्रत्यागम pTab;
}

/*
** Get a VDBE for the given parser context.  Create a new one if necessary.
** If an error occurs, return NULL and leave a message in pParse.
*/
Vdbe *sqlite3GetVdbe(Parse *pParse){
  यदि( pParse->pVdbe ){
    प्रत्यागम pParse->pVdbe;
  }
  यदि( pParse->pToplevel==0 ){
    pParse->ok_Const_गुणकः = 1;
  }
  प्रत्यागम sqlite3VdbeCreate(pParse);
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
स्थिर शून्य compute_सीमा_Registers(Parse *pParse, Select *p, पूर्णाङ्क iBreak){
  Vdbe *v = 0;
  पूर्णाङ्क i_सीमा = 0;
  पूर्णाङ्क i_विस्थापनम्;
  पूर्णाङ्क n;
  व्यञ्जकम् *p_सीमा = p->p_सीमा;

  यदि( p->i_सीमा ) प्रत्यागम;

  /*
  ** "LIMIT -1" always shows all rows.  There is some
  ** controversy about what the correct behavior should be.
  ** The current implementation interprets "LIMIT 0" to mean
  ** no rows.
  */
  यदि( p_सीमा ){
    प्रतिज्ञा( p_सीमा->op==TK_सीमा );
    प्रतिज्ञा( p_सीमा->p_वामः!=0 );
    p->i_सीमा = i_सीमा = ++pParse->nMem;
    v = sqlite3GetVdbe(pParse);
    प्रतिज्ञा( v!=0 );
    यदि( sqlite3_व्यञ्जकम्_Is_Integer(p_सीमा->p_वामः, &n, pParse, 0) ){
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, n, i_सीमा);
      VdbeComment((v, "LIMIT counter"));
      यदि( n==0 ){
        sqlite3VdbeGoto(v, iBreak);
      }अन्यथा यदि( n>=0 && p->nSelectRow>sqlite3LogEst((u64)n) ){
        p->nSelectRow = sqlite3LogEst((u64)n);
        p->selFlags |= SF_Fixed_सीमा;
      }
    }अन्यथा{
      sqlite3_व्यञ्जकम्_संहिता(pParse, p_सीमा->p_वामः, i_सीमा);
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_MustBeInt, i_सीमा); VdbeCoverage(v);
      VdbeComment((v, "LIMIT counter"));
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IfNot, i_सीमा, iBreak); VdbeCoverage(v);
    }
    यदि( p_सीमा->p_दक्षिणः ){
      p->i_विस्थापनम् = i_विस्थापनम् = ++pParse->nMem;
      pParse->nMem++;   /* Allocate an extra register for limit+offset */
      sqlite3_व्यञ्जकम्_संहिता(pParse, p_सीमा->p_दक्षिणः, i_विस्थापनम्);
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_MustBeInt, i_विस्थापनम्); VdbeCoverage(v);
      sqlite3MayAbort(pParse);
      VdbeComment((v, "OFFSET counter"));
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_विस्थापनम्_सीमा, i_सीमा, i_विस्थापनम्+1, i_विस्थापनम्);
      VdbeComment((v, "LIMIT+OFFSET"));
    }
  }
}

#यदि_अपरिभाषितम् SQLITE_OMIT_COMPOUND_SELECT
/*
** Return the appropriate collating sequence for the iCol-th column of
** the result set for the compound-select statement "p".  Return NULL if
** the column has no default collating sequence.
**
** The collating sequence for the compound select is taken from the
** left-most term of the select that has a collating sequence.
*/
स्थिर CollSeq *multiSelectCollSeq(Parse *pParse, Select *p, पूर्णाङ्क iCol){
  CollSeq *pRet;
  यदि( p->p_प्राक् ){
    pRet = multiSelectCollSeq(pParse, p->p_प्राक्, iCol);
  }अन्यथा{
    pRet = 0;
  }
  प्रतिज्ञा( iCol>=0 );
  /* iCol must be less than p->pEList->nExpr.  Otherwise an error would
  ** have been thrown during name resolution and we would not have gotten
  ** this far */
  यदि( pRet==0 && ALWAYS(iCol<p->pEList->n_व्यञ्जकम्) ){
    pRet = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, p->pEList->a[iCol].p_व्यञ्जकम्);
  }
  प्रत्यागम pRet;
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
स्थिर कुञ्चिका_Info *multi_Select_By_Merge_कुञ्चिका_Info(Parse *pParse, Select *p, पूर्णाङ्क nExtra){
  व्यञ्जकम्_List *p_क्रमः_By = p->p_क्रमः_By;
  पूर्णाङ्क n_क्रमः_By = (p_क्रमः_By!=0) ? p_क्रमः_By->n_व्यञ्जकम् : 0;
  sqlite3 *db = pParse->db;
  कुञ्चिका_Info *pRet = sqlite3_कुञ्चिका_Info_Alloc(db, n_क्रमः_By+nExtra, 1);
  यदि( pRet ){
    पूर्णाङ्क i;
    प्रति(i=0; i<n_क्रमः_By; i++){
      संरचना व्यञ्जकम्_List_item *pItem = &p_क्रमः_By->a[i];
      व्यञ्जकम् *p_पदांशः = pItem->p_व्यञ्जकम्;
      CollSeq *pColl;

      यदि( p_पदांशः->flags & EP_Collate ){
        pColl = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, p_पदांशः);
      }अन्यथा{
        pColl = multiSelectCollSeq(pParse, p, pItem->u.x.i_क्रमः_By_Col-1);
        यदि( pColl==0 ) pColl = db->pDfltColl;
        p_क्रमः_By->a[i].p_व्यञ्जकम् =
          sqlite3_व्यञ्जकम्_सङ्कलय_Collate_String(pParse, p_पदांशः, pColl->zName);
      }
      प्रतिज्ञा( sqlite3_कुञ्चिका_Info_Is_Writeable(pRet) );
      pRet->aColl[i] = pColl;
      pRet->a_क्रमय_Flags[i] = p_क्रमः_By->a[i].fg.क्रमय_Flags;
    }
  }

  प्रत्यागम pRet;
}

#यदि_अपरिभाषितम् SQLITE_OMIT_CTE
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
स्थिर शून्य generateWithRecursiveQuery(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The recursive SELECT to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  SrcList *pSrc = p->pSrc;      /* The FROM clause of the recursive query */
  पूर्णाङ्क nCol = p->pEList->n_व्यञ्जकम्;  /* Number of columns in the recursive table */
  Vdbe *v = pParse->pVdbe;      /* The prepared statement under construction */
  Select *p_सज्जता;               /* The setup query */
  Select *pFirstRec;            /* Left-most recursive term */
  पूर्णाङ्क addr_शिखरम्;                  /* Top of the loop */
  पूर्णाङ्क addrCont, addrBreak;      /* CONTINUE and BREAK addresses */
  पूर्णाङ्क iCurrent = 0;             /* The Current table */
  पूर्णाङ्क regCurrent;               /* Register holding Current table */
  पूर्णाङ्क iQueue;                   /* The Queue table */
  पूर्णाङ्क iDistinct = 0;            /* To ensure unique results if UNION */
  पूर्णाङ्क eDest = SRT_Fifo;         /* How to write to Queue */
  SelectDest destQueue;         /* SelectDest targeting the Queue table */
  पूर्णाङ्क i;                        /* Loop counter */
  पूर्णाङ्क rc;                       /* Result code */
  व्यञ्जकम्_List *p_क्रमः_By;           /* The ORDER BY clause */
  व्यञ्जकम् *p_सीमा;                 /* Saved LIMIT and OFFSET */
  पूर्णाङ्क reg_सीमा, reg_विस्थापनम्;      /* Registers used by LIMIT and OFFSET */

#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
  यदि( p->pWin ){
    sqlite3_Error_सन्देशः(pParse, "cannot use window functions in recursive queries");
    प्रत्यागम;
  }
#यदि_अन्तः

  /* Obtain authorization to do a recursive query */
  यदि( sqlite3AuthCheck(pParse, SQLITE_RECURSIVE, 0, 0, 0) ) प्रत्यागम;

  /* Process the LIMIT and OFFSET clauses, if they exist */
  addrBreak = sqlite3VdbeMakeLabel(pParse);
  p->nSelectRow = 320;  /* 4 billion rows */
  compute_सीमा_Registers(pParse, p, addrBreak);
  p_सीमा = p->p_सीमा;
  reg_सीमा = p->i_सीमा;
  reg_विस्थापनम् = p->i_विस्थापनम्;
  p->p_सीमा = 0;
  p->i_सीमा = p->i_विस्थापनम् = 0;
  p_क्रमः_By = p->p_क्रमः_By;

  /* Locate the cursor number of the Current table */
  प्रति(i=0; ALWAYS(i<pSrc->nSrc); i++){
    यदि( pSrc->a[i].fg.isRecursive ){
      iCurrent = pSrc->a[i].iCursor;
      भङ्ग;
    }
  }

  /* Allocate cursors numbers for Queue and Distinct.  The cursor number for
  ** the Distinct table must be exactly one greater than Queue in order
  ** for the SRT_DistFifo and SRT_DistQueue destinations to work. */
  iQueue = pParse->nTab++;
  यदि( p->op==TK_UNION ){
    eDest = p_क्रमः_By ? SRT_DistQueue : SRT_DistFifo;
    iDistinct = pParse->nTab++;
  }अन्यथा{
    eDest = p_क्रमः_By ? SRT_Queue : SRT_Fifo;
  }
  sqlite3SelectDestInit(&destQueue, eDest, iQueue);

  /* Allocate cursors for Current, Queue, and Distinct. */
  regCurrent = ++pParse->nMem;
  sqlite3_Vdbe_सङ्कलय_Op3(v, OP_OpenPseudo, iCurrent, regCurrent, nCol);
  यदि( p_क्रमः_By ){
    कुञ्चिका_Info *p_कुञ्चिका_Info = multi_Select_By_Merge_कुञ्चिका_Info(pParse, p, 1);
    sqlite3_Vdbe_सङ्कलय_Op4(v, OP_OpenEphemeral, iQueue, p_क्रमः_By->n_व्यञ्जकम्+2, 0,
                      (वर्ण*)p_कुञ्चिका_Info, P4_KEYINFO);
    destQueue.p_क्रमः_By = p_क्रमः_By;
  }अन्यथा{
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_OpenEphemeral, iQueue, nCol);
  }
  VdbeComment((v, "Queue table"));
  यदि( iDistinct ){
    /* Generate an ephemeral table used to enforce distinctness on the
    ** output of the recursive part of the CTE.
    */
    कुञ्चिका_Info *p_कुञ्चिका_Info;            /* Collating sequence for the result set */
    CollSeq **apColl;             /* For looping through pKeyInfo->aColl[] */

    प्रतिज्ञा( p->p_अग्र==0 );
    प्रतिज्ञा( p->pEList!=0 );
    nCol = p->pEList->n_व्यञ्जकम्;
    p_कुञ्चिका_Info = sqlite3_कुञ्चिका_Info_Alloc(pParse->db, nCol, 1);
    यदि( p_कुञ्चिका_Info ){
      प्रति(i=0, apColl=p_कुञ्चिका_Info->aColl; i<nCol; i++, apColl++){
        *apColl = multiSelectCollSeq(pParse, p, i);
        यदि( 0==*apColl ){
          *apColl = pParse->db->pDfltColl;
        }
      }
      sqlite3_Vdbe_सङ्कलय_Op4(v, OP_OpenEphemeral, iDistinct, nCol, 0,
                           (शून्य*)p_कुञ्चिका_Info, P4_KEYINFO);
    }अन्यथा{
      प्रतिज्ञा( pParse->nErr>0 );
    }
  }

  /* Detach the ORDER BY clause from the compound SELECT */
  p->p_क्रमः_By = 0;

  /* Figure out how many elements of the compound SELECT are part of the
  ** recursive query.  Make sure no recursive elements use aggregate
  ** functions.  Mark the recursive elements as UNION ALL even if they
  ** are really UNION because the distinctness will be enforced by the
  ** iDistinct table.  pFirstRec is left pointing to the left-most
  ** recursive term of the CTE.
  */
  प्रति(pFirstRec=p; ALWAYS(pFirstRec!=0); pFirstRec=pFirstRec->p_प्राक्){
    यदि( pFirstRec->selFlags & SF_Aggregate ){
      sqlite3_Error_सन्देशः(pParse, "recursive aggregate queries not supported");
      गच्छ अन्तः_सम्बन्धि_recursive_query;
    }
    pFirstRec->op = TK_ALL;
    यदि( (pFirstRec->p_प्राक्->selFlags & SF_Recursive)==0 ) भङ्ग;
  }

  /* Store the results of the setup-query in Queue. */
  p_सज्जता = pFirstRec->p_प्राक्;
  p_सज्जता->p_अग्र = 0;
  ExplainQueryPlan((pParse, 1, "SETUP"));
  rc = sqlite3Select(pParse, p_सज्जता, &destQueue);
  p_सज्जता->p_अग्र = p;
  यदि( rc ) गच्छ अन्तः_सम्बन्धि_recursive_query;

  /* Find the next row in the Queue and output that row */
  addr_शिखरम् = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Rewind, iQueue, addrBreak); VdbeCoverage(v);

  /* Transfer the next row in Queue over to Current */
  sqlite3_Vdbe_सङ्कलय_Op1(v, OP_NullRow, iCurrent); /* To reset column cache */
  यदि( p_क्रमः_By ){
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, iQueue, p_क्रमः_By->n_व्यञ्जकम्+1, regCurrent);
  }अन्यथा{
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Row_दत्त, iQueue, regCurrent);
  }
  sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Delete, iQueue);

  /* Output the single row in Current */
  addrCont = sqlite3VdbeMakeLabel(pParse);
  संहिता_विस्थापनम्(v, reg_विस्थापनम्, addrCont);
  selectInnerLoop(pParse, p, iCurrent,
      0, 0, pDest, addrCont, addrBreak);
  यदि( reg_सीमा ){
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_DecrJumpZero, reg_सीमा, addrBreak);
    VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, addrCont);

  /* Execute the recursive SELECT taking the single row in Current as
  ** the value for the recursive-table. Store the results in the Queue.
  */
  pFirstRec->p_प्राक् = 0;
  ExplainQueryPlan((pParse, 1, "RECURSIVE STEP"));
  sqlite3Select(pParse, p, &destQueue);
  प्रतिज्ञा( pFirstRec->p_प्राक्==0 );
  pFirstRec->p_प्राक् = p_सज्जता;

  /* Keep running the loop until the Queue is empty */
  sqlite3VdbeGoto(v, addr_शिखरम्);
  sqlite3VdbeResolveLabel(v, addrBreak);

अन्तः_सम्बन्धि_recursive_query:
  sqlite3_व्यञ्जकम्_List_Delete(pParse->db, p->p_क्रमः_By);
  p->p_क्रमः_By = p_क्रमः_By;
  p->p_सीमा = p_सीमा;
  प्रत्यागम;
}
#यदि_अन्तः /* SQLITE_OMIT_CTE */

/* Forward references */
स्थिर पूर्णाङ्क multiSelectByMerge(
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
स्थिर पूर्णाङ्क multi_Select_मानानि(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  पूर्णाङ्क nRow = 1;
  पूर्णाङ्क rc = 0;
  पूर्णाङ्क bShowAll = p->p_सीमा==0;
  प्रतिज्ञा( p->selFlags & SF_Multi_मानम् );
  कुरु{
    प्रतिज्ञा( p->selFlags & SF_मानानि );
    प्रतिज्ञा( p->op==TK_ALL || (p->op==TK_SELECT && p->p_प्राक्==0) );
    प्रतिज्ञा( p->p_अग्र==0 || p->pEList->n_व्यञ्जकम्==p->p_अग्र->pEList->n_व्यञ्जकम् );
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
    यदि( p->pWin ) प्रत्यागम -1;
#यदि_अन्तः
    यदि( p->p_प्राक्==0 ) भङ्ग;
    प्रतिज्ञा( p->p_प्राक्->p_अग्र==p );
    p = p->p_प्राक्;
    nRow += bShowAll;
  }यावत्(1);
  ExplainQueryPlan((pParse, 0, "SCAN %d CONSTANT ROW%s", nRow,
                    nRow==1 ? "" : "S"));
  यावत्( p ){
    selectInnerLoop(pParse, p, -1, 0, 0, pDest, 1, 1);
    यदि( !bShowAll ) भङ्ग;
    p->nSelectRow = nRow;
    p = p->p_अग्र;
  }
  प्रत्यागम rc;
}

/*
** Return true if the SELECT statement which is known to be the recursive
** part of a recursive CTE still has its anchor terms attached.  If the
** anchor terms have already been removed, then return false.
*/
स्थिर पूर्णाङ्क hasAnchor(Select *p){
  यावत्( p && (p->selFlags & SF_Recursive)!=0 ){ p = p->p_प्राक्; }
  प्रत्यागम p!=0;
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
स्थिर पूर्णाङ्क union_With_सीमा_One(sqlite3 *db, Select *p){
  पूर्णाङ्क v;
  यदि( p->op!=TK_UNION ) प्रत्यागम 0;
  यदि( p->p_सीमा==0 ) प्रत्यागम 0;
  यदि( p->p_सीमा->p_दक्षिणः ) प्रत्यागम 0;  /* No OFFSET */
  v = 0;
  प्रतिज्ञा( p->p_सीमा->p_वामः!=0 );
  यदि( sqlite3_व्यञ्जकम्_Is_Integer(p->p_सीमा->p_वामः, &v, 0, 0)==0 ) प्रत्यागम 0;
  यदि( v!=1 ) प्रत्यागम 0;
  यदि( OptimizationDisabled(db, SQLITE_Union_सीमा) ) प्रत्यागम 0;
  प्रत्यागम 1;
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
स्थिर पूर्णाङ्क multiSelect(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  पूर्णाङ्क rc = SQLITE_OK;   /* Success code from a subroutine */
  Select *p_प्राक्;       /* Another SELECT immediately to our left */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest dest;      /* Alternative data destination */
  Select *pDelete = 0;  /* Chain of simple selects to delete */
  sqlite3 *db;          /* Database connection */

  /* Make sure there is no ORDER BY or LIMIT clause on prior SELECTs.  Only
  ** the last (right-most) SELECT in the series may have an ORDER BY or LIMIT.
  */
  प्रतिज्ञा( p && p->p_प्राक् );  /* Calling function guarantees this much */
  प्रतिज्ञा( (p->selFlags & SF_Recursive)==0 || p->op==TK_ALL || p->op==TK_UNION );
  प्रतिज्ञा( p->selFlags & SF_Compound );
  db = pParse->db;
  p_प्राक् = p->p_प्राक्;
  dest = *pDest;
  प्रतिज्ञा( p_प्राक्->p_क्रमः_By==0 );
  प्रतिज्ञा( p_प्राक्->p_सीमा==0 );

  v = sqlite3GetVdbe(pParse);
  प्रतिज्ञा( v!=0 );  /* The VDBE already created by calling function */

  /* Create the destination temporary table if necessary
  */
  यदि( dest.eDest==SRT_EphemTab ){
    प्रतिज्ञा( p->pEList );
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_OpenEphemeral, dest.iSDParm, p->pEList->n_व्यञ्जकम्);
    dest.eDest = SRT_सारणी;
  }

  /* Special handling for a compound-select that originates as a VALUES clause.
  */
  यदि( p->selFlags & SF_Multi_मानम् ){
    rc = multi_Select_मानानि(pParse, p, &dest);
    यदि( rc>=0 ) गच्छ multi_select_अन्तः;
    rc = SQLITE_OK;
  }

  /* Make sure all SELECTs in the statement have the same number of elements
  ** in their result sets.
  */
  प्रतिज्ञा( p->pEList && p_प्राक्->pEList );
  प्रतिज्ञा( p->pEList->n_व्यञ्जकम्==p_प्राक्->pEList->n_व्यञ्जकम् );

#यदि_अपरिभाषितम् SQLITE_OMIT_CTE
  यदि( (p->selFlags & SF_Recursive)!=0 && hasAnchor(p) ){
    generateWithRecursiveQuery(pParse, p, &dest);
  }अन्यथा
#यदि_अन्तः
  यदि( p->p_क्रमः_By ){
    /* If the compound has an ORDER BY clause, then always use the merge
    ** algorithm. */
    प्रत्यागम multiSelectByMerge(pParse, p, pDest);
  }अन्यथा यदि( p->op!=TK_ALL && !union_With_सीमा_One(db,p) ){
    /* If the compound is EXCEPT, INTERSECT, or UNION (anything other than
    ** UNION ALL) then also always use the merge algorithm.  However, the
    ** multiSelectByMerge() routine requires that the compound have an
    ** ORDER BY clause, and it doesn't right now.  So invent one first. */
    व्यञ्जकम् *pOne = sqlite3_व्यञ्जकम्_Int32(db, 1);
    p->p_क्रमः_By = sqlite3_व्यञ्जकम्_List_Append(pParse, 0, pOne);
    यदि( pParse->nErr ) गच्छ multi_select_अन्तः;
    प्रतिज्ञा( p->p_क्रमः_By!=0 );
    p->p_क्रमः_By->a[0].u.x.i_क्रमः_By_Col = 1;
    प्रत्यागम multiSelectByMerge(pParse, p, pDest);
  }अन्यथा{
    /* For a UNION ALL compound without ORDER BY, or for a UNION with a 
    ** LIMIT of exactly 1 and no OFFSET and no ORDER BY, simply run the left
    ** query, then run the right query */
    पूर्णाङ्क addr = 0;
    पूर्णाङ्क n_सीमा = 0;  /* Initialize to suppress harmless compiler warning */

#यदि_अपरिभाषितम् SQLITE_OMIT_EXPLAIN
    यदि( p_प्राक्->p_प्राक्==0 ){
      ExplainQueryPlan((pParse, 1, "COMPOUND QUERY"));
      ExplainQueryPlan((pParse, 1, "LEFT-MOST SUBQUERY"));
    }
#यदि_अन्तः
    प्रतिज्ञा( !p_प्राक्->p_सीमा );
    p_प्राक्->i_सीमा = p->i_सीमा;
    p_प्राक्->i_विस्थापनम् = p->i_विस्थापनम्;
    p_प्राक्->p_सीमा = sqlite3_व्यञ्जकम्_Dup(db, p->p_सीमा, 0);
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL left...\n"));
    rc = sqlite3Select(pParse, p_प्राक्, &dest);
    sqlite3_व्यञ्जकम्_Delete(db, p_प्राक्->p_सीमा);
    p_प्राक्->p_सीमा = 0;
    यदि( rc ){
      गच्छ multi_select_अन्तः;
    }
    p->p_प्राक् = 0;
    p->i_सीमा = p_प्राक्->i_सीमा;
    p->i_विस्थापनम् = p_प्राक्->i_विस्थापनम्;
    यदि( p->i_सीमा ){
      addr = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_IfNot, p->i_सीमा); VdbeCoverage(v);
      VdbeComment((v, "Jump ahead if LIMIT reached"));
      यदि( p->i_विस्थापनम् ){
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_विस्थापनम्_सीमा,
                          p->i_सीमा, p->i_विस्थापनम्+1, p->i_विस्थापनम्);
      }
    }
    ExplainQueryPlan((pParse, 1, "UNION ALL"));
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL right...\n"));
    rc = sqlite3Select(pParse, p, &dest);
    testcase( rc!=SQLITE_OK );
    pDelete = p->p_प्राक्;
    p->p_प्राक् = p_प्राक्;
    p->nSelectRow = sqlite3_Log_Est_सङ्कलय(p->nSelectRow, p_प्राक्->nSelectRow);
    यदि( p->p_सीमा
     && sqlite3_व्यञ्जकम्_Is_Integer(p->p_सीमा->p_वामः, &n_सीमा, pParse, 0)
     && n_सीमा>0 && p->nSelectRow > sqlite3LogEst((u64)n_सीमा)
    ){
      p->nSelectRow = sqlite3LogEst((u64)n_सीमा);
    }
    यदि( addr ){
      sqlite3_Vdbe_Jump_अत्र(v, addr);
    }
#यदि_अपरिभाषितम् SQLITE_OMIT_EXPLAIN
    यदि( p->p_अग्र==0 ){
      Explain_Query_Plan_निष्कासय(pParse);
    }
#यदि_अन्तः
  }

multi_select_अन्तः:
  pDest->iSdst = dest.iSdst;
  pDest->nSdst = dest.nSdst;
  pDest->iSDParm2 = dest.iSDParm2;
  यदि( pDelete ){
    sqlite3_Parser_सङ्कलय_Cleanup(pParse, sqlite3SelectDeleteGeneric, pDelete);
  }
  प्रत्यागम rc;
}
#यदि_अन्तः /* SQLITE_OMIT_COMPOUND_SELECT */

/*
** Error message for when two or more terms of a compound select have different
** size result sets.
*/
शून्य sqlite3_Select_Wrong_अङ्कः_Terms_Error(Parse *pParse, Select *p){
  यदि( p->selFlags & SF_मानानि ){
    sqlite3_Error_सन्देशः(pParse, "all VALUES must have the same number of terms");
  }अन्यथा{
    sqlite3_Error_सन्देशः(pParse, "SELECTs to the left and right of %s"
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
स्थिर पूर्णाङ्क generateOutputSubroutine(
  Parse *pParse,          /* Parsing context */
  Select *p,              /* The SELECT statement */
  SelectDest *p_अन्तर्,        /* Coroutine supplying data */
  SelectDest *pDest,      /* Where to send the data */
  पूर्णाङ्क regReturn,          /* The return address register */
  पूर्णाङ्क reg_पूर्व,            /* Previous result register.  No uniqueness if 0 */
  कुञ्चिका_Info *p_कुञ्चिका_Info,      /* For comparing with previous entry */
  पूर्णाङ्क iBreak              /* Jump here if we hit the LIMIT */
){
  Vdbe *v = pParse->pVdbe;
  पूर्णाङ्क iContinue;
  पूर्णाङ्क addr;

  प्रतिज्ञा( p_अन्तर्->eDest==SRT_Coroutine );

  addr = sqlite3VdbeCurrentAddr(v);
  iContinue = sqlite3VdbeMakeLabel(pParse);

  /* Suppress duplicates for UNION, EXCEPT, and INTERSECT
  */
  यदि( reg_पूर्व ){
    पूर्णाङ्क addr1, addr2;
    addr1 = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_IfNot, reg_पूर्व); VdbeCoverage(v);
    addr2 = sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Compare, p_अन्तर्->iSdst, reg_पूर्व+1, p_अन्तर्->nSdst,
                              (वर्ण*)sqlite3_कुञ्चिका_Info_Ref(p_कुञ्चिका_Info), P4_KEYINFO);
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Jump, addr2+2, iContinue, addr2+2); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_अत्र(v, addr1);
    sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Copy, p_अन्तर्->iSdst, reg_पूर्व+1, p_अन्तर्->nSdst-1);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 1, reg_पूर्व);
  }
  यदि( pParse->db->mallocFailed ) प्रत्यागम 0;

  /* Suppress the first OFFSET entries if there is an OFFSET clause
  */
  संहिता_विस्थापनम्(v, p->i_विस्थापनम्, iContinue);

  विकल्प( pDest->eDest ){
    /* Store the result as data using a unique key.
    */
    पक्ष SRT_Fifo:
    पक्ष SRT_DistFifo:
    पक्ष SRT_सारणी:
    पक्ष SRT_EphemTab: {
      पूर्णाङ्क r1 = sqlite3_Get_अस्थायी_Reg(pParse);
      पूर्णाङ्क r2 = sqlite3_Get_अस्थायी_Reg(pParse);
      पूर्णाङ्क iParm = pDest->iSDParm;
      testcase( pDest->eDest==SRT_सारणी );
      testcase( pDest->eDest==SRT_EphemTab );
      testcase( pDest->eDest==SRT_Fifo );
      testcase( pDest->eDest==SRT_DistFifo );
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, p_अन्तर्->iSdst, p_अन्तर्->nSdst, r1);
#यदि !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      यदि( pDest->eDest==SRT_सारणी && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_CTE
      यदि( pDest->eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index that is used to enforce uniqueness on the
        ** total result.  At this point, we are processing the setup portion
        ** of the recursive CTE using the merge algorithm, so the results are
        ** guaranteed to be unique anyhow.  But we still need to populate the
        ** (iParm+1) cursor for use by the subsequent recursive phase.
        */
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, iParm+1, r1,
                             p_अन्तर्->iSdst, p_अन्तर्->nSdst);
      }
#यदि_अन्तः
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_NewRowid, iParm, r2);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_निवेशय, iParm, r1, r2);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      sqlite3_Release_अस्थायी_Reg(pParse, r2);
      sqlite3_Release_अस्थायी_Reg(pParse, r1);
      भङ्ग;
    }

    /* If any row exist in the result set, record that fact and abort.
    */
    पक्ष SRT_Exists: {
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 1, pDest->iSDParm);
      /* The LIMIT clause will terminate the loop for us */
      भङ्ग;
    }

#यदि_अपरिभाषितम् SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)".
    */
    पक्ष SRT_समुच्चयः: {
      पूर्णाङ्क r1;
      testcase( p_अन्तर्->nSdst>1 );
      r1 = sqlite3_Get_अस्थायी_Reg(pParse);
      sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Make_लेखः, p_अन्तर्->iSdst, p_अन्तर्->nSdst,
          r1, pDest->zAffSdst, p_अन्तर्->nSdst);
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, pDest->iSDParm, r1,
                           p_अन्तर्->iSdst, p_अन्तर्->nSdst);
      यदि( pDest->iSDParm2>0 ){
        प्रतिज्ञा( sqlite3CursorBloomable(pParse, pDest->iSDParm, p_अन्तर्->nSdst) );
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Filter_सङ्कलय, pDest->iSDParm2, 0,
                             p_अन्तर्->iSdst, p_अन्तर्->nSdst);
        ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
      }
      sqlite3_Release_अस्थायी_Reg(pParse, r1);
      भङ्ग;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell and break out
    ** of the scan loop.  Note that the select might return multiple columns
    ** if it is the RHS of a row-value IN operator.
    */
    पक्ष SRT_Mem: {
      testcase( p_अन्तर्->nSdst>1 );
      sqlite3_व्यञ्जकम्_संहिता_Move(pParse, p_अन्तर्->iSdst, pDest->iSDParm, p_अन्तर्->nSdst);
      /* The LIMIT clause will jump out of the loop for us */
      भङ्ग;
    }
#यदि_अन्तः /* #ifndef SQLITE_OMIT_SUBQUERY */

    /* The results are stored in a sequence of registers
    ** starting at pDest->iSdst.  Then the co-routine yields.
    */
    पक्ष SRT_Coroutine: {
      यदि( pDest->iSdst==0 ){
        pDest->iSdst = sqlite3_Get_अस्थायी_Range(pParse, p_अन्तर्->nSdst);
        pDest->nSdst = p_अन्तर्->nSdst;
      }
      sqlite3_व्यञ्जकम्_संहिता_Move(pParse, p_अन्तर्->iSdst, pDest->iSdst, p_अन्तर्->nSdst);
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Yield, pDest->iSDParm);
      भङ्ग;
    }

#यदि_अपरिभाषितम् SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    पक्ष SRT_DistQueue:
    पक्ष SRT_Queue: {
      पूर्णाङ्क n_कुञ्चिका;
      पूर्णाङ्क r1, r2, r3, ii;
      व्यञ्जकम्_List *pSO;
      पूर्णाङ्क iParm = pDest->iSDParm;
      pSO = pDest->p_क्रमः_By;
      प्रतिज्ञा( pSO );
      n_कुञ्चिका = pSO->n_व्यञ्जकम्;
      r1 = sqlite3_Get_अस्थायी_Reg(pParse);
      r2 = sqlite3_Get_अस्थायी_Range(pParse, n_कुञ्चिका+2);
      r3 = r2+n_कुञ्चिका+1;

      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, p_अन्तर्->iSdst, p_अन्तर्->nSdst, r3);
      यदि( pDest->eDest==SRT_DistQueue ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Idx_निवेशय, iParm+1, r3);
      }
      प्रति(ii=0; ii<n_कुञ्चिका; ii++){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_SCopy,
                          p_अन्तर्->iSdst + pSO->a[ii].u.x.i_क्रमः_By_Col - 1,
                          r2+ii);
      }
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Sequence, iParm, r2+n_कुञ्चिका);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, r2, n_कुञ्चिका+2, r1);
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, iParm, r1, r2, n_कुञ्चिका+2);
      sqlite3_Release_अस्थायी_Reg(pParse, r1);
      sqlite3_Release_अस्थायी_Range(pParse, r2, n_कुञ्चिका+2);
      भङ्ग;
    }
#यदि_अन्तः /* SQLITE_OMIT_CTE */

    /* Ignore the output */
    पक्ष SRT_Discard: {
      भङ्ग;
    }

    /* If none of the above, then the result destination must be
    ** SRT_Output.
    **
    ** For SRT_Output, results are stored in a sequence of registers. 
    ** Then the OP_ResultRow opcode is used to cause sqlite3_step() to
    ** return the next row of result.
    */
    स्वभाव: {
      प्रतिज्ञा( pDest->eDest==SRT_Output );
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_परिणाम_Row, p_अन्तर्->iSdst, p_अन्तर्->nSdst);
      भङ्ग;
    }
  }

  /* Jump to the end of the loop if the LIMIT is reached.
  */
  यदि( p->i_सीमा ){
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_DecrJumpZero, p->i_सीमा, iBreak); VdbeCoverage(v);
  }

  /* Generate the subroutine return
  */
  sqlite3VdbeResolveLabel(v, iContinue);
  sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Return, regReturn);

  प्रत्यागम addr;
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
#यदि_अपरिभाषितम् SQLITE_OMIT_COMPOUND_SELECT
स्थिर पूर्णाङ्क multiSelectByMerge(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  पूर्णाङ्क i, j;             /* Loop counters */
  Select *p_प्राक्;       /* Another SELECT immediately to our left */
  Select *p_विभज;       /* Left-most SELECT in the right-hand group */
  पूर्णाङ्क nSelect;          /* Number of SELECT statements in the compound */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest destA;     /* Destination for coroutine A */
  SelectDest destB;     /* Destination for coroutine B */
  पूर्णाङ्क regAddrA;         /* Address register for select-A coroutine */
  पूर्णाङ्क regAddrB;         /* Address register for select-B coroutine */
  पूर्णाङ्क addrSelectA;      /* Address of the select-A coroutine */
  पूर्णाङ्क addrSelectB;      /* Address of the select-B coroutine */
  पूर्णाङ्क reg_बहिर्गमनम्_A;          /* Address register for the output-A subroutine */
  पूर्णाङ्क reg_बहिर्गमनम्_B;          /* Address register for the output-B subroutine */
  पूर्णाङ्क addr_बहिर्गमनम्_A;         /* Address of the output-A subroutine */
  पूर्णाङ्क addr_बहिर्गमनम्_B = 0;     /* Address of the output-B subroutine */
  पूर्णाङ्क addrEofA;         /* Address of the select-A-exhausted subroutine */
  पूर्णाङ्क addrEofA_noB;     /* Alternate addrEofA if B is uninitialized */
  पूर्णाङ्क addrEofB;         /* Address of the select-B-exhausted subroutine */
  पूर्णाङ्क addrAltB;         /* Address of the A<B subroutine */
  पूर्णाङ्क addrAeqB;         /* Address of the A==B subroutine */
  पूर्णाङ्क addrAgtB;         /* Address of the A>B subroutine */
  पूर्णाङ्क reg_सीमा_A;        /* Limit register for select-A */
  पूर्णाङ्क reg_सीमा_B;        /* Limit register for select-A */
  पूर्णाङ्क reg_पूर्व;          /* A range of registers to hold previous output */
  पूर्णाङ्क saved_सीमा;       /* Saved value of p->iLimit */
  पूर्णाङ्क saved_विस्थापनम्;      /* Saved value of p->iOffset */
  पूर्णाङ्क labelCmpr;        /* Label for the start of the merge algorithm */
  पूर्णाङ्क label_अन्तः;         /* Label for the end of the overall SELECT stmt */
  पूर्णाङ्क addr1;            /* Jump instructions that get retargeted */
  पूर्णाङ्क op;               /* One of TK_ALL, TK_UNION, TK_EXCEPT, TK_INTERSECT */
  कुञ्चिका_Info *p_कुञ्चिका_Dup = 0; /* Comparison information for duplicate removal */
  कुञ्चिका_Info *p_कुञ्चिका_Merge;   /* Comparison information for merging rows */
  sqlite3 *db;          /* Database connection */
  व्यञ्जकम्_List *p_क्रमः_By;   /* The ORDER BY clause */
  पूर्णाङ्क n_क्रमः_By;         /* Number of terms in the ORDER BY clause */
  u32 *aPermute;        /* Mapping from ORDER BY terms to result set columns */

  प्रतिज्ञा( p->p_क्रमः_By!=0 );
  प्रतिज्ञा( p_कुञ्चिका_Dup==0 ); /* "Managed" code needs this.  Ticket #3382. */
  db = pParse->db;
  v = pParse->pVdbe;
  प्रतिज्ञा( v!=0 );       /* Already thrown the error if VDBE alloc failed */
  label_अन्तः = sqlite3VdbeMakeLabel(pParse);
  labelCmpr = sqlite3VdbeMakeLabel(pParse);


  /* Patch up the ORDER BY clause
  */
  op = p->op; 
  प्रतिज्ञा( p->p_प्राक्->p_क्रमः_By==0 );
  p_क्रमः_By = p->p_क्रमः_By;
  प्रतिज्ञा( p_क्रमः_By );
  n_क्रमः_By = p_क्रमः_By->n_व्यञ्जकम्;

  /* For operators other than UNION ALL we have to make sure that
  ** the ORDER BY clause covers every term of the result set.  Add
  ** terms to the ORDER BY clause as necessary.  Do the same for UNION ALL
  ** if any operator to the left is not UNION ALL, so that the ORDER BY
  ** carries the collation of every column into that sub-compound.
  */
  प्रति(p_प्राक्=p; p_प्राक् && p_प्राक्->op==TK_ALL; p_प्राक्=p_प्राक्->p_प्राक्){}
  यदि( p_प्राक् && p_प्राक्->p_प्राक् ){
    प्रति(i=1; db->mallocFailed==0 && i<=p->pEList->n_व्यञ्जकम्; i++){
      संरचना व्यञ्जकम्_List_item *pItem;
      प्रति(j=0, pItem=p_क्रमः_By->a; j<n_क्रमः_By; j++, pItem++){
        प्रतिज्ञा( pItem!=0 );
        प्रतिज्ञा( pItem->u.x.i_क्रमः_By_Col>0 );
        यदि( pItem->u.x.i_क्रमः_By_Col==i ) भङ्ग;
      }
      यदि( j==n_क्रमः_By ){
        व्यञ्जकम् *pNew = sqlite3_व्यञ्जकम्_Int32(db, i);
        यदि( pNew==0 ) प्रत्यागम SQLITE_NOMEM_BKPT;
        p->p_क्रमः_By = p_क्रमः_By = sqlite3_व्यञ्जकम्_List_Append(pParse, p_क्रमः_By, pNew);
        यदि( p_क्रमः_By ) p_क्रमः_By->a[n_क्रमः_By++].u.x.i_क्रमः_By_Col = (u16)i;
      }
    }
  }

  /* Compute the comparison permutation and keyinfo that is used with
  ** the permutation to determine if the next row of results comes
  ** from selectA or selectB.  Also add literal collations to the
  ** ORDER BY clause terms so that when selectA and selectB are
  ** evaluated, they use the correct collation.
  */
  aPermute = sqlite3DbMallocRawNN(db, परिमाण(u32)*(n_क्रमः_By + 1));
  यदि( aPermute ){
    संरचना व्यञ्जकम्_List_item *pItem;
    पूर्णाङ्क bKeep = 0;
    aPermute[0] = n_क्रमः_By;
    प्रति(i=1, pItem=p_क्रमः_By->a; i<=n_क्रमः_By; i++, pItem++){
      प्रतिज्ञा( pItem!=0 );
      प्रतिज्ञा( pItem->u.x.i_क्रमः_By_Col>0 );
      प्रतिज्ञा( pItem->u.x.i_क्रमः_By_Col<=p->pEList->n_व्यञ्जकम् );
      aPermute[i] = pItem->u.x.i_क्रमः_By_Col - 1;
      यदि( aPermute[i]!=(u32)i-1 ) bKeep = 1;
    }
     यदि( bKeep==0 ){
      sqlite3DbFreeNN(db, aPermute);
      aPermute = 0;
    }
  }
  p_कुञ्चिका_Merge = multi_Select_By_Merge_कुञ्चिका_Info(pParse, p, 1);

  /* Allocate a range of temporary registers and the KeyInfo needed
  ** for the logic that removes duplicate result rows when the
  ** operator is UNION, EXCEPT, or INTERSECT (but not UNION ALL).
  */
  यदि( op==TK_ALL ){
    reg_पूर्व = 0;
  }अन्यथा{
    पूर्णाङ्क n_व्यञ्जकम् = p->pEList->n_व्यञ्जकम्;
    प्रतिज्ञा( n_क्रमः_By>=n_व्यञ्जकम् || db->mallocFailed );
    reg_पूर्व = pParse->nMem+1;
    pParse->nMem += n_व्यञ्जकम्+1;
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 0, reg_पूर्व);
    p_कुञ्चिका_Dup = sqlite3_कुञ्चिका_Info_Alloc(db, n_व्यञ्जकम्, 1);
    यदि( p_कुञ्चिका_Dup ){
      प्रतिज्ञा( sqlite3_कुञ्चिका_Info_Is_Writeable(p_कुञ्चिका_Dup) );
      /* Use the ORDER BY collations, not multiSelectCollSeq(p), because
      ** p might be just one part of a larger compound, in which case
      ** the ORDER BY carries the COLLATE of the larger compound. */
      प्रति(i=0; ALWAYS(p_कुञ्चिका_Merge) && i<n_क्रमः_By; i++){
        p_कुञ्चिका_Dup->aColl[p_क्रमः_By->a[i].u.x.i_क्रमः_By_Col-1] = p_कुञ्चिका_Merge->aColl[i];
      }
    }
  }

  /* Separate the left and the right query from one another
  */
  nSelect = 1;
  यदि( (op==TK_ALL || op==TK_UNION)
   && OptimizationEnabled(db, SQLITE_BalancedMerge)
  ){
    प्रति(p_विभज=p; p_विभज->p_प्राक्!=0 && p_विभज->op==op; p_विभज=p_विभज->p_प्राक्){
      nSelect++;
      प्रतिज्ञा( p_विभज->p_प्राक्->p_अग्र==p_विभज );
    }
  }
  यदि( nSelect<=3 ){
    p_विभज = p;
  }अन्यथा{
    p_विभज = p;
    प्रति(i=2; i<nSelect; i+=2){ p_विभज = p_विभज->p_प्राक्; }
  }
  p_प्राक् = p_विभज->p_प्राक्;
  प्रतिज्ञा( p_प्राक्!=0 );
  p_विभज->p_प्राक् = 0;
  p_प्राक्->p_अग्र = 0;
  प्रतिज्ञा( p->p_क्रमः_By == p_क्रमः_By );
  प्रतिज्ञा( p_क्रमः_By!=0 || db->mallocFailed );
  p_प्राक्->p_क्रमः_By = sqlite3_व्यञ्जकम्_List_Dup(pParse->db, p_क्रमः_By, 0);
  sqlite3_Resolve_क्रमः_गणः_By(pParse, p, p->p_क्रमः_By, "ORDER");
  sqlite3_Resolve_क्रमः_गणः_By(pParse, p_प्राक्, p_प्राक्->p_क्रमः_By, "ORDER");

  /* Compute the limit registers */
  compute_सीमा_Registers(pParse, p, label_अन्तः);
  यदि( p->i_सीमा && (op==TK_ALL || union_With_सीमा_One(db,p)) ){
    reg_सीमा_A = ++pParse->nMem;
    reg_सीमा_B = ++pParse->nMem;
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Copy, p->i_विस्थापनम् ? p->i_विस्थापनम्+1 : p->i_सीमा,
                                  reg_सीमा_A);
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Copy, reg_सीमा_A, reg_सीमा_B);
  }अन्यथा{
    reg_सीमा_A = reg_सीमा_B = 0;
  }
  sqlite3_व्यञ्जकम्_Delete(db, p->p_सीमा);
  p->p_सीमा = 0;

  regAddrA = ++pParse->nMem;
  regAddrB = ++pParse->nMem;
  reg_बहिर्गमनम्_A = ++pParse->nMem;
  reg_बहिर्गमनम्_B = ++pParse->nMem;
  sqlite3SelectDestInit(&destA, SRT_Coroutine, regAddrA);
  sqlite3SelectDestInit(&destB, SRT_Coroutine, regAddrB);

  ExplainQueryPlan((pParse, 1, "MERGE (%s)", sqlite3SelectOpName(p->op)));

  /* Generate a coroutine to evaluate the SELECT statement to the
  ** left of the compound operator - the "A" select.
  */
  addrSelectA = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3_Vdbe_सङ्कलय_Op3(v, OP_InitCoroutine, regAddrA, 0, addrSelectA);
  VdbeComment((v, "SUBR: next-A"));
  p_प्राक्->i_सीमा = reg_सीमा_A;
  ExplainQueryPlan((pParse, 1, "LEFT"));
  sqlite3Select(pParse, p_प्राक्, &destA);
  sqlite3_Vdbe_अन्तः_Coroutine(v, regAddrA);
  sqlite3_Vdbe_Jump_अत्र(v, addr1);

  /* Generate a coroutine to evaluate the SELECT statement on
  ** the right - the "B" select
  */
  addrSelectB = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3_Vdbe_सङ्कलय_Op3(v, OP_InitCoroutine, regAddrB, 0, addrSelectB);
  VdbeComment((v, "SUBR: next-B"));
  saved_सीमा = p->i_सीमा;
  saved_विस्थापनम् = p->i_विस्थापनम्;
  p->i_सीमा = reg_सीमा_B;
  p->i_विस्थापनम् = 0; 
  ExplainQueryPlan((pParse, 1, "RIGHT"));
  sqlite3Select(pParse, p, &destB);
  p->i_सीमा = saved_सीमा;
  p->i_विस्थापनम् = saved_विस्थापनम्;
  sqlite3_Vdbe_अन्तः_Coroutine(v, regAddrB);

  /* Generate a subroutine that outputs the current row of the A
  ** select as the next output row of the compound select.
  */
  VdbeNoopComment((v, "SUBR: out-A"));
  addr_बहिर्गमनम्_A = generateOutputSubroutine(pParse,
                 p, &destA, pDest, reg_बहिर्गमनम्_A,
                 reg_पूर्व, p_कुञ्चिका_Dup, label_अन्तः);
 
  /* Generate a subroutine that outputs the current row of the B
  ** select as the next output row of the compound select.
  */
  यदि( op==TK_ALL || op==TK_UNION ){
    VdbeNoopComment((v, "SUBR: out-B"));
    addr_बहिर्गमनम्_B = generateOutputSubroutine(pParse,
                 p, &destB, pDest, reg_बहिर्गमनम्_B,
                 reg_पूर्व, p_कुञ्चिका_Dup, label_अन्तः);
  }
  sqlite3_कुञ्चिका_Info_Unref(p_कुञ्चिका_Dup);

  /* Generate a subroutine to run when the results from select A
  ** are exhausted and only data in select B remains.
  */
  यदि( op==TK_EXCEPT || op==TK_INTERSECT ){
    addrEofA_noB = addrEofA = label_अन्तः;
  }अन्यथा{ 
    VdbeNoopComment((v, "SUBR: eof-A"));
    addrEofA = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, reg_बहिर्गमनम्_B, addr_बहिर्गमनम्_B);
    VdbeComment((v, "out-B"));
    addrEofA_noB = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Yield, regAddrB, label_अन्तः);
                                     VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, addrEofA);
    p->nSelectRow = sqlite3_Log_Est_सङ्कलय(p->nSelectRow, p_प्राक्->nSelectRow);
  }

  /* Generate a subroutine to run when the results from select B
  ** are exhausted and only data in select A remains.
  */
  यदि( op==TK_INTERSECT ){
    addrEofB = addrEofA;
    यदि( p->nSelectRow > p_प्राक्->nSelectRow ) p->nSelectRow = p_प्राक्->nSelectRow;
  }अन्यथा{ 
    VdbeNoopComment((v, "SUBR: eof-B"));
    addrEofB = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, reg_बहिर्गमनम्_A, addr_बहिर्गमनम्_A);
    VdbeComment((v, "out-A"));
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Yield, regAddrA, label_अन्तः); VdbeCoverage(v);
    VdbeComment((v, "next-A"));
    sqlite3VdbeGoto(v, addrEofB);
  }

  /* Generate code to handle the case of A<B
  */
  addrAltB = sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, reg_बहिर्गमनम्_A, addr_बहिर्गमनम्_A);
  VdbeComment((v, "out-A"));
  sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Yield, regAddrA, addrEofA); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  sqlite3VdbeGoto(v, labelCmpr);

  /* Generate code to handle the case of A==B
  */
  यदि( op==TK_ALL ){
    addrAeqB = addrAltB;
  }अन्यथा यदि( op==TK_INTERSECT ){
    addrAeqB = addrAltB;
    addrAltB++;
  }अन्यथा{
    addrAeqB = addrAltB + 1;
  }

  /* Generate code to handle the case of A>B
  */
  addrAgtB = sqlite3VdbeCurrentAddr(v);
  यदि( op==TK_ALL || op==TK_UNION ){
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, reg_बहिर्गमनम्_B, addr_बहिर्गमनम्_B);
    VdbeComment((v, "out-B"));
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, labelCmpr);
  }अन्यथा{
    addrAgtB++; /* Just do next-B.  Might as well use the next-B call
                ** in the next code block */
  }

  /* This code runs once to initialize everything.
  */
  sqlite3_Vdbe_Jump_अत्र(v, addr1);
  sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Yield, regAddrA, addrEofA_noB); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  /* v---  Also the A>B case for EXCEPT and INTERSECT */
  sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
  VdbeComment((v, "next-B"));

  /* Implement the main merge loop
  */
  यदि( aPermute!=0 ){
    sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Permutation, 0, 0, 0, (वर्ण*)aPermute, P4_INTARRAY);
  }
  sqlite3VdbeResolveLabel(v, labelCmpr);
  sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Compare, destA.iSdst, destB.iSdst, n_क्रमः_By,
                         (वर्ण*)p_कुञ्चिका_Merge, P4_KEYINFO);
  यदि( aPermute!=0 ){
    sqlite3VdbeChangeP5(v, OPFLAG_PERMUTE);
  }
  sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Jump, addrAltB, addrAeqB, addrAgtB);
  VdbeCoverageIf(v, op==TK_ALL);
  VdbeCoverageIf(v, op==TK_UNION);
  VdbeCoverageIf(v, op==TK_EXCEPT);
  VdbeCoverageIf(v, op==TK_INTERSECT);

  /* Jump to the this point in order to terminate the query.
  */
  sqlite3VdbeResolveLabel(v, label_अन्तः);

  /* Make arrangements to free the 2nd and subsequent arms of the compound
  ** after the parse has finished */
  यदि( p_विभज->p_प्राक् ){
    sqlite3_Parser_सङ्कलय_Cleanup(pParse, sqlite3SelectDeleteGeneric, p_विभज->p_प्राक्);
  }
  p_विभज->p_प्राक् = p_प्राक्;
  p_प्राक्->p_अग्र = p_विभज;
  sqlite3_व्यञ्जकम्_List_Delete(db, p_प्राक्->p_क्रमः_By);
  p_प्राक्->p_क्रमः_By = 0;

  /*** TBD:  Insert subroutine calls to close cursors on incomplete
  **** subqueries ****/
  Explain_Query_Plan_निष्कासय(pParse);
  प्रत्यागम pParse->nErr!=0;
}
#यदि_अन्तः

#यदि !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)

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
प्रकारनिर्देश संरचना SubstContext {
  Parse *pParse;            /* The parsing context */
  पूर्णाङ्क i_सारणी;               /* Replace references to this table */
  पूर्णाङ्क i_New_सारणी;            /* New table number */
  पूर्णाङ्क isOuterJoin;          /* Add TK_IF_NULL_ROW opcodes on each replacement */
  पूर्णाङ्क n_Sel_गभीरता;            /* Depth of sub-query recursion.  Top==1 */
  व्यञ्जकम्_List *pEList;         /* Replacement expressions */
  व्यञ्जकम्_List *pCList;         /* Collation sequences for replacement expr */
} SubstContext;

/* Forward Declarations */
स्थिर शून्य subst_व्यञ्जकम्_List(SubstContext*, व्यञ्जकम्_List*);
स्थिर शून्य substSelect(SubstContext*, Select*, पूर्णाङ्क);

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
स्थिर व्यञ्जकम् *subst_व्यञ्जकम्(
  SubstContext *pSubst,  /* Description of the substitution */
  व्यञ्जकम् *p_व्यञ्जकम्            /* Expr in which substitution occurs */
){
  यदि( p_व्यञ्जकम्==0 ) प्रत्यागम 0;
  यदि( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON|EP_InnerON)
   && p_व्यञ्जकम्->w.iJoin==pSubst->i_सारणी
  ){
    testcase( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_InnerON) );
    p_व्यञ्जकम्->w.iJoin = pSubst->i_New_सारणी;
  }
  यदि( p_व्यञ्जकम्->op==TK_COLUMN
   && p_व्यञ्जकम्->i_सारणी==pSubst->i_सारणी
   && !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_FixedCol)
  ){
#यदि_परिभाषितम् SQLITE_ALLOW_ROWID_अन्तर्_VIEW
    यदि( p_व्यञ्जकम्->iColumn<0 ){
      p_व्यञ्जकम्->op = TK_NULL;
    }अन्यथा
#यदि_अन्तः
    {
      व्यञ्जकम् *pNew;
      पूर्णाङ्क iColumn;
      व्यञ्जकम् *pCopy;
      व्यञ्जकम् ifNullRow;
      iColumn = p_व्यञ्जकम्->iColumn;
      प्रतिज्ञा( iColumn>=0 );
      प्रतिज्ञा( pSubst->pEList!=0 && iColumn<pSubst->pEList->n_व्यञ्जकम् );
      प्रतिज्ञा( p_व्यञ्जकम्->p_दक्षिणः==0 );
      pCopy = pSubst->pEList->a[iColumn].p_व्यञ्जकम्;
      यदि( sqlite3_व्यञ्जकम्_Is_सदिशम्(pCopy) ){
        sqlite3_सदिशम्_Error_सन्देशः(pSubst->pParse, pCopy);
      }अन्यथा{
        sqlite3 *db = pSubst->pParse->db;
        यदि( pSubst->isOuterJoin
         && (pCopy->op!=TK_COLUMN || pCopy->i_सारणी!=pSubst->i_New_सारणी)
        ){
          स्मृतिं_पूरय(&ifNullRow, 0, परिमाण(ifNullRow));
          ifNullRow.op = TK_IF_NULL_ROW;
          ifNullRow.p_वामः = pCopy;
          ifNullRow.i_सारणी = pSubst->i_New_सारणी;
          ifNullRow.iColumn = -99;
          ifNullRow.flags = EP_IfNullRow;
          pCopy = &ifNullRow;
        }
        testcase( व्यञ्जकम्_Has_Property(pCopy, EP_Subquery) );
        pNew = sqlite3_व्यञ्जकम्_Dup(db, pCopy, 0);
        यदि( db->mallocFailed ){
          sqlite3_व्यञ्जकम्_Delete(db, pNew);
          प्रत्यागम p_व्यञ्जकम्;
        }
        यदि( pSubst->isOuterJoin ){
          व्यञ्जकम्_समुच्चयः_Property(pNew, EP_CanBeNull);
        }
        यदि( pNew->op==TK_TRUEFALSE ){
          pNew->u.i_मानम् = sqlite3_व्यञ्जकम्_Truth_मानम्(pNew);
          pNew->op = TK_INTEGER;
          व्यञ्जकम्_समुच्चयः_Property(pNew, EP_Int_मानम्);
        }

        /* Ensure that the expression now has an implicit collation sequence,
        ** just as it did when it was a column of a view or sub-query. */
        {
          CollSeq *pNat = sqlite3_व्यञ्जकम्_Coll_Seq(pSubst->pParse, pNew);
          CollSeq *pColl = sqlite3_व्यञ्जकम्_Coll_Seq(pSubst->pParse,
                pSubst->pCList->a[iColumn].p_व्यञ्जकम्
          );
          यदि( pNat!=pColl || (pNew->op!=TK_COLUMN && pNew->op!=TK_COLLATE) ){
            pNew = sqlite3_व्यञ्जकम्_सङ्कलय_Collate_String(pSubst->pParse, pNew,
                (pColl ? pColl->zName : "BINARY")
            );
          }
        }
        व्यञ्जकम्_Clear_Property(pNew, EP_Collate);
        यदि( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्,EP_OuterON|EP_InnerON) ){
          sqlite3_समुच्चयः_Join_व्यञ्जकम्(pNew, p_व्यञ्जकम्->w.iJoin,
                             p_व्यञ्जकम्->flags & (EP_OuterON|EP_InnerON));
        }
        sqlite3_व्यञ्जकम्_Delete(db, p_व्यञ्जकम्);
        p_व्यञ्जकम् = pNew;
      }
    }
  }अन्यथा{
    यदि( p_व्यञ्जकम्->op==TK_IF_NULL_ROW && p_व्यञ्जकम्->i_सारणी==pSubst->i_सारणी ){
      p_व्यञ्जकम्->i_सारणी = pSubst->i_New_सारणी;
    }
    यदि( p_व्यञ्जकम्->op==TK_AGG_FUNCTION && p_व्यञ्जकम्->op2>=pSubst->n_Sel_गभीरता ){
      p_व्यञ्जकम्->op2--;
    }
    p_व्यञ्जकम्->p_वामः = subst_व्यञ्जकम्(pSubst, p_व्यञ्जकम्->p_वामः);
    p_व्यञ्जकम्->p_दक्षिणः = subst_व्यञ्जकम्(pSubst, p_व्यञ्जकम्->p_दक्षिणः);
    यदि( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) ){
      substSelect(pSubst, p_व्यञ्जकम्->x.pSelect, 1);
    }अन्यथा{
      subst_व्यञ्जकम्_List(pSubst, p_व्यञ्जकम्->x.pList);
    }
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
    यदि( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_WinFunc) ){
      गवाक्षः *pWin = p_व्यञ्जकम्->y.pWin;
      pWin->pFilter = subst_व्यञ्जकम्(pSubst, pWin->pFilter);
      subst_व्यञ्जकम्_List(pSubst, pWin->pPartition);
      subst_व्यञ्जकम्_List(pSubst, pWin->p_क्रमः_By);
    }
#यदि_अन्तः
  }
  प्रत्यागम p_व्यञ्जकम्;
}
स्थिर शून्य subst_व्यञ्जकम्_List(
  SubstContext *pSubst, /* Description of the substitution */
  व्यञ्जकम्_List *pList       /* List to scan and in which to make substitutes */
){
  पूर्णाङ्क i;
  यदि( pList==0 ) प्रत्यागम;
  प्रति(i=0; i<pList->n_व्यञ्जकम्; i++){
    pList->a[i].p_व्यञ्जकम् = subst_व्यञ्जकम्(pSubst, pList->a[i].p_व्यञ्जकम्);
  }
}
स्थिर शून्य substSelect(
  SubstContext *pSubst, /* Description of the substitution */
  Select *p,            /* SELECT statement in which to make substitutions */
  पूर्णाङ्क do_प्राक्           /* Do substitutes on p->pPrior too */
){
  SrcList *pSrc;
  SrcItem *pItem;
  पूर्णाङ्क i;
  यदि( !p ) प्रत्यागम;
  pSubst->n_Sel_गभीरता++;
  कुरु{
    subst_व्यञ्जकम्_List(pSubst, p->pEList);
    subst_व्यञ्जकम्_List(pSubst, p->p_गणः_By);
    subst_व्यञ्जकम्_List(pSubst, p->p_क्रमः_By);
    p->pHaving = subst_व्यञ्जकम्(pSubst, p->pHaving);
    p->pWhere = subst_व्यञ्जकम्(pSubst, p->pWhere);
    pSrc = p->pSrc;
    प्रतिज्ञा( pSrc!=0 );
    प्रति(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      यदि( pItem->fg.isSubquery ){
        substSelect(pSubst, pItem->u4.pSubq->pSelect, 1);
      }
      यदि( pItem->fg.isTabFunc ){
        subst_व्यञ्जकम्_List(pSubst, pItem->u1.pFuncArg);
      }
    }
  }यावत्( do_प्राक् && (p = p->p_प्राक्)!=0 );
  pSubst->n_Sel_गभीरता--;
}
#यदि_अन्तः /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#यदि !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
/*
** pSelect is a SELECT statement and pSrcItem is one item in the FROM
** clause of that SELECT.
**
** This routine scans the entire SELECT statement and recomputes the
** pSrcItem->colUsed mask.
*/
स्थिर पूर्णाङ्क recompute_Columns_प्रयुक्तम्_व्यञ्जकम्(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  SrcItem *pItem;
  यदि( p_व्यञ्जकम्->op!=TK_COLUMN ) प्रत्यागम WRC_Continue;
  pItem = pWalker->u.pSrcItem;
  यदि( pItem->iCursor!=p_व्यञ्जकम्->i_सारणी ) प्रत्यागम WRC_Continue;
  यदि( p_व्यञ्जकम्->iColumn<0 ) प्रत्यागम WRC_Continue;
  pItem->col_प्रयुक्तम् |= sqlite3_व्यञ्जकम्_Col_प्रयुक्तम्(p_व्यञ्जकम्);
  प्रत्यागम WRC_Continue;
}
स्थिर शून्य recompute_Columns_प्रयुक्तम्(
  Select *pSelect,                 /* The complete SELECT statement */
  SrcItem *pSrcItem                /* Which FROM clause item to recompute */
){
  Walker w;
  यदि( NEVER(pSrcItem->pSTab==0) ) प्रत्यागम;
  स्मृतिं_पूरय(&w, 0, परिमाण(w));
  w.x_व्यञ्जकम्_Callback = recompute_Columns_प्रयुक्तम्_व्यञ्जकम्;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.pSrcItem = pSrcItem;
  pSrcItem->col_प्रयुक्तम् = 0;
  sqlite3WalkSelect(&w, pSelect);
}
#यदि_अन्तः /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#यदि !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
स्थिर शून्य srclistRenumberCursors(
  Parse *pParse,                  /* Parse context */
  पूर्णाङ्क *aCsrMap,                   /* Array to store cursor mappings in */
  SrcList *pSrc,                  /* FROM clause to renumber */
  पूर्णाङ्क iExcept                     /* FROM clause item to skip */
){
  पूर्णाङ्क i;
  SrcItem *pItem;
  प्रति(i=0, pItem=pSrc->a; i<pSrc->nSrc; i++, pItem++){
    यदि( i!=iExcept ){
      Select *p;
      प्रतिज्ञा( pItem->iCursor < aCsrMap[0] );
      यदि( !pItem->fg.isRecursive || aCsrMap[pItem->iCursor+1]==0 ){
        aCsrMap[pItem->iCursor+1] = pParse->nTab++;
      }
      pItem->iCursor = aCsrMap[pItem->iCursor+1];
      यदि( pItem->fg.isSubquery ){
        प्रति(p=pItem->u4.pSubq->pSelect; p; p=p->p_प्राक्){
          srclistRenumberCursors(pParse, aCsrMap, p->pSrc, -1);
        }
      }
    }
  }
}

/*
** *piCursor is a cursor number.  Change it if it needs to be mapped.
*/
स्थिर शून्य renumberCursorDoMapping(Walker *pWalker, पूर्णाङ्क *piCursor){
  पूर्णाङ्क *aCsrMap = pWalker->u.aiCol;
  पूर्णाङ्क iCsr = *piCursor;
  यदि( iCsr < aCsrMap[0] && aCsrMap[iCsr+1]>0 ){
    *piCursor = aCsrMap[iCsr+1];
  }
}

/*
** Expression walker callback used by renumberCursors() to update
** Expr objects to match newly assigned cursor numbers.
*/
स्थिर पूर्णाङ्क renumberCursorsCb(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  पूर्णाङ्क op = p_व्यञ्जकम्->op;
  यदि( op==TK_COLUMN || op==TK_IF_NULL_ROW ){
    renumberCursorDoMapping(pWalker, &p_व्यञ्जकम्->i_सारणी);
  }
  यदि( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, (EP_OuterON|EP_InnerON)) ){
    testcase( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON) );
    testcase( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_InnerON) );
    renumberCursorDoMapping(pWalker, &p_व्यञ्जकम्->w.iJoin);
  }
  प्रत्यागम WRC_Continue;
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
स्थिर शून्य renumberCursors(
  Parse *pParse,                  /* Parse context */
  Select *p,                      /* Select to renumber cursors within */
  पूर्णाङ्क iExcept,                    /* FROM clause item to skip */
  पूर्णाङ्क *aCsrMap                    /* Working space */
){
  Walker w;
  srclistRenumberCursors(pParse, aCsrMap, p->pSrc, iExcept);
  स्मृतिं_पूरय(&w, 0, परिमाण(w));
  w.u.aiCol = aCsrMap;
  w.x_व्यञ्जकम्_Callback = renumberCursorsCb;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  sqlite3WalkSelect(&w, p);
}
#यदि_अन्तः /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** If pSel is not part of a compound SELECT, return a pointer to its
** expression list. Otherwise, return a pointer to the expression list
** of the leftmost SELECT in the compound.
*/
स्थिर व्यञ्जकम्_List *findLeftmostExprlist(Select *pSel){
  यावत्( pSel->p_प्राक् ){
    pSel = pSel->p_प्राक्;
  }
  प्रत्यागम pSel->pEList;
}

/*
** Return true if any of the result-set columns in the compound query
** have incompatible affinities on one or more arms of the compound.
*/
स्थिर पूर्णाङ्क compoundHasDifferentAffinities(Select *p){
  पूर्णाङ्क ii;
  व्यञ्जकम्_List *pList;
  प्रतिज्ञा( p!=0 );
  प्रतिज्ञा( p->pEList!=0 );
  प्रतिज्ञा( p->p_प्राक्!=0 );
  pList = p->pEList;
  प्रति(ii=0; ii<pList->n_व्यञ्जकम्; ii++){
    वर्ण aff;
    Select *pSub1;
    प्रतिज्ञा( pList->a[ii].p_व्यञ्जकम्!=0 );
    aff = sqlite3_व्यञ्जकम्_Affinity(pList->a[ii].p_व्यञ्जकम्);
    प्रति(pSub1=p->p_प्राक्; pSub1; pSub1=pSub1->p_प्राक्){
      प्रतिज्ञा( pSub1->pEList!=0 );
      प्रतिज्ञा( pSub1->pEList->n_व्यञ्जकम्>ii );
      प्रतिज्ञा( pSub1->pEList->a[ii].p_व्यञ्जकम्!=0 );
      यदि( sqlite3_व्यञ्जकम्_Affinity(pSub1->pEList->a[ii].p_व्यञ्जकम्)!=aff ){
        प्रत्यागम 1;
      }
    }
  }
  प्रत्यागम 0;
}

#यदि !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
स्थिर पूर्णाङ्क flattenSubquery(
  Parse *pParse,       /* Parsing context */
  Select *p,           /* The parent or outer SELECT statement */
  पूर्णाङ्क iFrom,           /* Index in p->pSrc->a[] of the inner subquery */
  पूर्णाङ्क isAgg            /* True if outer SELECT uses aggregate functions */
){
  ध्रुव वर्ण *zSavedAuthContext = pParse->zAuthContext;
  Select *pParent;    /* Current UNION ALL term of the other query */
  Select *pSub;       /* The inner query or "subquery" */
  Select *pSub1;      /* Pointer to the rightmost select in sub-query */
  SrcList *pSrc;      /* The FROM clause of the outer query */
  SrcList *pSubSrc;   /* The FROM clause of the subquery */
  पूर्णाङ्क iParent;        /* VDBE cursor number of the pSub result set temp table */
  पूर्णाङ्क iNewParent = -1;/* Replacement table for iParent */
  पूर्णाङ्क isOuterJoin = 0; /* True if pSub is the right side of a LEFT JOIN */   
  पूर्णाङ्क i;              /* Loop counter */
  व्यञ्जकम् *pWhere;                    /* The WHERE clause */
  SrcItem *pSubitem;               /* The subquery */
  sqlite3 *db = pParse->db;
  Walker w;                        /* Walker to persist agginfo data */
  पूर्णाङ्क *aCsrMap = 0;

  /* Check to see if flattening is permitted.  Return 0 if not.
  */
  प्रतिज्ञा( p!=0 );
  प्रतिज्ञा( p->p_प्राक्==0 );
  यदि( OptimizationDisabled(db, SQLITE_QueryFlattener) ) प्रत्यागम 0;
  pSrc = p->pSrc;
  प्रतिज्ञा( pSrc && iFrom>=0 && iFrom<pSrc->nSrc );
  pSubitem = &pSrc->a[iFrom];
  iParent = pSubitem->iCursor;
  प्रतिज्ञा( pSubitem->fg.isSubquery );
  pSub = pSubitem->u4.pSubq->pSelect;
  प्रतिज्ञा( pSub!=0 );

#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
  यदि( p->pWin || pSub->pWin ) प्रत्यागम 0;                  /* Restriction (25) */
#यदि_अन्तः

  pSubSrc = pSub->pSrc;
  प्रतिज्ञा( pSubSrc );
  /* Prior to version 3.1.2, when LIMIT and OFFSET had to be simple constants,
  ** not arbitrary expressions, we allowed some combining of LIMIT and OFFSET
  ** because they could be computed at compile-time.  But when LIMIT and OFFSET
  ** became arbitrary expressions, we were forced to add restrictions (13)
  ** and (14). */
  यदि( pSub->p_सीमा && p->p_सीमा ) प्रत्यागम 0;              /* Restriction (13) */
  यदि( pSub->p_सीमा && pSub->p_सीमा->p_दक्षिणः ) प्रत्यागम 0;   /* Restriction (14) */
  यदि( (p->selFlags & SF_Compound)!=0 && pSub->p_सीमा ){
    प्रत्यागम 0;                                            /* Restriction (15) */
  }
  यदि( pSubSrc->nSrc==0 ) प्रत्यागम 0;                       /* Restriction (7)  */
  यदि( pSub->selFlags & SF_Distinct ) प्रत्यागम 0;           /* Restriction (4)  */
  यदि( pSub->p_सीमा && (pSrc->nSrc>1 || isAgg) ){
     प्रत्यागम 0;         /* Restrictions (8)(9) */
  }
  यदि( p->p_क्रमः_By && pSub->p_क्रमः_By ){
     प्रत्यागम 0;                                           /* Restriction (11) */
  }
  यदि( isAgg && pSub->p_क्रमः_By ) प्रत्यागम 0;                /* Restriction (16) */
  यदि( pSub->p_सीमा && p->pWhere ) प्रत्यागम 0;              /* Restriction (19) */
  यदि( pSub->p_सीमा && (p->selFlags & SF_Distinct)!=0 ){
     प्रत्यागम 0;         /* Restriction (21) */
  }
  यदि( pSub->selFlags & (SF_Recursive) ){
    प्रत्यागम 0; /* Restrictions (22) */
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
  यदि( (pSubitem->fg.jointype & (JT_OUTER|JT_LTORJ))!=0 ){
    यदि( pSubSrc->nSrc>1                        /* (3a) */
     /**** || IsVirtual(pSubSrc->a[0].pSTab)      (3b)-omitted */
     || (p->selFlags & SF_Distinct)!=0         /* (3d) */
     || (pSubitem->fg.jointype & JT_दक्षिणः)!=0  /* (26) */
    ){
      प्रत्यागम 0;
    }
    isOuterJoin = 1;
  }

  प्रतिज्ञा( pSubSrc->nSrc>0 );  /* True by restriction (7) */
  यदि( iFrom>0 && (pSubSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
    प्रत्यागम 0;   /* Restriction (27a) */
  }

  /* Condition (28) is blocked by the caller */
  प्रतिज्ञा( !pSubitem->fg.isCte || pSubitem->u2.pCteUse->eM10d!=M10d_Yes );

  /* Restriction (17): If the sub-query is a compound SELECT, then it must
  ** use only the UNION ALL operator. And none of the simple select queries
  ** that make up the compound SELECT are allowed to be aggregate or distinct
  ** queries.
  */
  यदि( pSub->p_प्राक् ){
    पूर्णाङ्क ii;
    यदि( pSub->p_क्रमः_By ){
      प्रत्यागम 0;  /* Restriction (20) */
    }
    यदि( isAgg || (p->selFlags & SF_Distinct)!=0 || isOuterJoin>0 ){
      प्रत्यागम 0; /* (17d1), (17d2), or (17f) */
    }
    प्रति(pSub1=pSub; pSub1; pSub1=pSub1->p_प्राक्){
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct );
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Aggregate );
      प्रतिज्ञा( pSub->pSrc!=0 );
      प्रतिज्ञा( (pSub->selFlags & SF_Recursive)==0 );
      प्रतिज्ञा( pSub->pEList->n_व्यञ्जकम्==pSub1->pEList->n_व्यञ्जकम् );
      यदि( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))!=0    /* (17b) */
       || (pSub1->p_प्राक् && pSub1->op!=TK_ALL)                 /* (17a) */
       || pSub1->pSrc->nSrc<1                                  /* (17c) */
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
       || pSub1->pWin                                          /* (17e) */
#यदि_अन्तः
      ){
        प्रत्यागम 0;
      }
      यदि( iFrom>0 && (pSub1->pSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
        /* Without this restriction, the JT_LTORJ flag would end up being
        ** omitted on left-hand tables of the right join that is being
        ** flattened. */
        प्रत्यागम 0;   /* Restrictions (17g), (27b) */
      }
      testcase( pSub1->pSrc->nSrc>1 );
    }

    /* Restriction (18). */
    यदि( p->p_क्रमः_By ){
      प्रति(ii=0; ii<p->p_क्रमः_By->n_व्यञ्जकम्; ii++){
        यदि( p->p_क्रमः_By->a[ii].u.x.i_क्रमः_By_Col==0 ) प्रत्यागम 0;
      }
    }

    /* Restriction (23) */
    यदि( (p->selFlags & SF_Recursive) ) प्रत्यागम 0;

    /* Restriction (17h) */
    यदि( compoundHasDifferentAffinities(pSub) ) प्रत्यागम 0;

    यदि( pSrc->nSrc>1 ){
      यदि( pParse->nSelect>500 ) प्रत्यागम 0;
      यदि( OptimizationDisabled(db, SQLITE_FlttnUnionAll) ) प्रत्यागम 0;
      aCsrMap = sqlite3DbMallocZero(db, ((i64)pParse->nTab+1)*परिमाण(पूर्णाङ्क));
      यदि( aCsrMap ) aCsrMap[0] = pParse->nTab;
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
  
  यदि( ALWAYS(pSubitem->fg.isSubquery) ){
    pSub1 = sqlite3SubqueryDetach(db, pSubitem);
  }अन्यथा{
    pSub1 = 0;
  }
  प्रतिज्ञा( pSubitem->fg.isSubquery==0 );
  प्रतिज्ञा( pSubitem->fg.fixedSchema==0 );
  sqlite3DbFree(db, pSubitem->zName);
  sqlite3DbFree(db, pSubitem->zAlias);
  pSubitem->zName = 0;
  pSubitem->zAlias = 0;
  प्रतिज्ञा( pSubitem->fg.isUsing!=0 || pSubitem->u3.pOn==0 );

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
  प्रति(pSub=pSub->p_प्राक्; pSub; pSub=pSub->p_प्राक्){
    Select *pNew;
    व्यञ्जकम्_List *p_क्रमः_By = p->p_क्रमः_By;
    व्यञ्जकम् *p_सीमा = p->p_सीमा;
    Select *p_प्राक् = p->p_प्राक्;
    सारणी *pItemTab = pSubitem->pSTab;
    pSubitem->pSTab = 0;
    p->p_क्रमः_By = 0;
    p->p_प्राक् = 0;
    p->p_सीमा = 0;
    pNew = sqlite3SelectDup(db, p, 0);
    p->p_सीमा = p_सीमा;
    p->p_क्रमः_By = p_क्रमः_By;
    p->op = TK_ALL;
    pSubitem->pSTab = pItemTab;
    यदि( pNew==0 ){
      p->p_प्राक् = p_प्राक्;
    }अन्यथा{
      pNew->selId = ++pParse->nSelect;
      यदि( aCsrMap && ALWAYS(db->mallocFailed==0) ){
        renumberCursors(pParse, pNew, iFrom, aCsrMap);
      }
      pNew->p_प्राक् = p_प्राक्;
      यदि( p_प्राक् ) p_प्राक्->p_अग्र = pNew;
      pNew->p_अग्र = p;
      p->p_प्राक् = pNew;
      TREETRACE(0x4,pParse,p,("compound-subquery flattener"
                              " creates %u as peer\n",pNew->selId));
    }
    प्रतिज्ञा( pSubitem->fg.isSubquery==0 );
  }
  sqlite3DbFree(db, aCsrMap);
  यदि( db->mallocFailed ){
    प्रतिज्ञा( pSubitem->fg.fixedSchema==0 );
    प्रतिज्ञा( pSubitem->fg.isSubquery==0 );
    प्रतिज्ञा( pSubitem->u4.zDatabase==0 );
    sqlite3SrcItemAttachSubquery(pParse, pSubitem, pSub1, 0);
    प्रत्यागम 1;
  }

  /* Defer deleting the Table object associated with the
  ** subquery until code generation is
  ** complete, since there may still exist Expr.pTab entries that
  ** refer to the subquery even after flattening.  Ticket #3346.
  **
  ** pSubitem->pSTab is always non-NULL by test restrictions and tests above.
  */
  यदि( ALWAYS(pSubitem->pSTab!=0) ){
    सारणी *p_Tab_गन्तव्यम्_Del = pSubitem->pSTab;
    यदि( p_Tab_गन्तव्यम्_Del->nTabRef==1 ){
      Parse *pToplevel = sqlite3ParseToplevel(pParse);
      sqlite3_Parser_सङ्कलय_Cleanup(pToplevel, sqlite3_Delete_सारणी_Generic, p_Tab_गन्तव्यम्_Del);
      testcase( pToplevel->earlyCleanup );
    }अन्यथा{
      p_Tab_गन्तव्यम्_Del->nTabRef--;
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
  प्रति(pParent=p; pParent; pParent=pParent->p_प्राक्, pSub=pSub->p_प्राक्){
    पूर्णाङ्क nSubSrc;
    u8 jointype = pParent->pSrc->a[iFrom].fg.jointype;
    प्रतिज्ञा( pSub!=0 );
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
    यदि( nSubSrc>1 ){
      pSrc = sqlite3SrcListEnlarge(pParse, pSrc, nSubSrc-1,iFrom+1);
      यदि( pSrc==0 ) भङ्ग;
      pParent->pSrc = pSrc;
    }

    /* Transfer the FROM clause terms from the subquery into the
    ** outer query.
    */
    iNewParent = pSubSrc->a[0].iCursor;
    प्रति(i=0; i<nSubSrc; i++){
      SrcItem *pItem = &pSrc->a[i+iFrom];
      प्रतिज्ञा( pItem->fg.isTabFunc==0 );
      प्रतिज्ञा( pItem->fg.isSubquery
           || pItem->fg.fixedSchema
           || pItem->u4.zDatabase==0 );
      यदि( pItem->fg.isUsing ) sqlite3IdListDelete(db, pItem->u3.pUsing);
      *pItem = pSubSrc->a[i];
      pItem->fg.jointype |= (jointype & JT_LTORJ);
      स्मृतिं_पूरय(&pSubSrc->a[i], 0, परिमाण(pSubSrc->a[i]));
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
    यदि( pSub->p_क्रमः_By ){
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
      व्यञ्जकम्_List *p_क्रमः_By = pSub->p_क्रमः_By;
      प्रति(i=0; i<p_क्रमः_By->n_व्यञ्जकम्; i++){
        p_क्रमः_By->a[i].u.x.i_क्रमः_By_Col = 0;
      }
      प्रतिज्ञा( pParent->p_क्रमः_By==0 );
      pParent->p_क्रमः_By = p_क्रमः_By;
      pSub->p_क्रमः_By = 0;
    }
    pWhere = pSub->pWhere;
    pSub->pWhere = 0;
    यदि( isOuterJoin>0 ){
      प्रतिज्ञा( pSubSrc->nSrc==1 );
      sqlite3_समुच्चयः_Join_व्यञ्जकम्(pWhere, iNewParent, EP_OuterON);
    }
    यदि( pWhere ){
      यदि( pParent->pWhere ){
        pParent->pWhere = sqlite3PExpr(pParse, TK_AND, pWhere, pParent->pWhere);
      }अन्यथा{
        pParent->pWhere = pWhere;
      }
    }
    यदि( db->mallocFailed==0 ){
      SubstContext x;
      x.pParse = pParse;
      x.i_सारणी = iParent;
      x.i_New_सारणी = iNewParent;
      x.isOuterJoin = isOuterJoin;
      x.n_Sel_गभीरता = 0;
      x.pEList = pSub->pEList;
      x.pCList = findLeftmostExprlist(pSub);
      substSelect(&x, pParent, 0);
    }
 
    /* The flattened query is a compound if either the inner or the
    ** outer query is a compound. */
    pParent->selFlags |= pSub->selFlags & SF_Compound;
    प्रतिज्ञा( (pSub->selFlags & SF_Distinct)==0 ); /* restriction (17b) */
 
    /*
    ** SELECT ... FROM (SELECT ... LIMIT a OFFSET b) LIMIT x OFFSET y;
    **
    ** One is tempted to try to add a and b to combine the limits.  But this
    ** does not work if either limit is negative.
    */
    यदि( pSub->p_सीमा ){
      pParent->p_सीमा = pSub->p_सीमा;
      pSub->p_सीमा = 0;
    }

    /* Recompute the SrcItem.colUsed masks for the flattened
    ** tables. */
    प्रति(i=0; i<nSubSrc; i++){
      recompute_Columns_प्रयुक्तम्(pParent, &pSrc->a[i+iFrom]);
    }
  }

  /* Finally, delete what is left of the subquery and return success.
  */
  sqlite3AggInfoPersistWalkerInit(&w, pParse);
  sqlite3WalkSelect(&w,pSub1);
  sqlite3SelectDelete(db, pSub1);

#यदि TREETRACE_ENABLED
  यदि( sqlite3_वृक्षः_Trace & 0x4 ){
    TREETRACE(0x4,pParse,p,("After flattening:\n"));
    sqlite3_वृक्षः_View_Select(0, p, 0);
  }
#यदि_अन्तः

  प्रत्यागम 1;
}
#यदि_अन्तः /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** A structure to keep track of all of the column values that are fixed to
** a known value due to WHERE clause constraints of the form COLUMN=VALUE.
*/
प्रकारनिर्देश संरचना WhereConst WhereConst;
संरचना WhereConst {
  Parse *pParse;   /* Parsing context */
  u8 *pOomFault;   /* Pointer to pParse->db->mallocFailed */
  पूर्णाङ्क nConst;      /* Number for COLUMN=CONSTANT terms */
  पूर्णाङ्क nChng;       /* Number of times a constant is propagated */
  पूर्णाङ्क bHasAffBlob; /* At least one column in apExpr[] as affinity BLOB */
  u32 mExcludeOn;  /* Which ON expressions to exclude from considertion.
                   ** Either EP_OuterON or EP_InnerON|EP_OuterON */
  व्यञ्जकम् **ap_व्यञ्जकम्;   /* [i*2] is COLUMN and [i*2+1] is VALUE */
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
स्थिर शून्य const_निवेशय(
  WhereConst *pConst,  /* The WhereConst into which we are inserting */
  व्यञ्जकम् *pColumn,       /* The COLUMN part of the constraint */
  व्यञ्जकम् *p_मानम्,        /* The VALUE part of the constraint */
  व्यञ्जकम् *p_व्यञ्जकम्          /* Overall expression: COLUMN=VALUE or VALUE=COLUMN */
){
  पूर्णाङ्क i;
  प्रतिज्ञा( pColumn->op==TK_COLUMN );
  प्रतिज्ञा( sqlite3_व्यञ्जकम्_Is_Constant(pConst->pParse, p_मानम्) );
  यदि( व्यञ्जकम्_Has_Property(pColumn, EP_FixedCol) ) प्रत्यागम;
  यदि( sqlite3_व्यञ्जकम्_Affinity(p_मानम्)!=0 ) प्रत्यागम;
  यदि( !sqlite3_Is_द्विमान(sqlite3_व्यञ्जकम्_Compare_Coll_Seq(pConst->pParse,p_व्यञ्जकम्)) ){
    प्रत्यागम;
  }
  यदि( sqlite3_व्यञ्जकम्_Can_Return_Subtype(pConst->pParse, p_मानम्) ) प्रत्यागम;

  /* 2018-10-25 ticket [cf5ed20f]
  ** Make sure the same pColumn is not inserted more than once */
  प्रति(i=0; i<pConst->nConst; i++){
    ध्रुव व्यञ्जकम् *pE2 = pConst->ap_व्यञ्जकम्[i*2];
    प्रतिज्ञा( pE2->op==TK_COLUMN );
    यदि( pE2->i_सारणी==pColumn->i_सारणी
     && pE2->iColumn==pColumn->iColumn
    ){
      प्रत्यागम;  /* Already present.  Return without doing anything. */
    }
  }
  प्रतिज्ञा( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  यदि( sqlite3_व्यञ्जकम्_Affinity(pColumn)<=SQLITE_AFF_BLOB ){
    pConst->bHasAffBlob = 1;
  }

  pConst->nConst++;
  pConst->ap_व्यञ्जकम् = sqlite3DbReallocOrFree(pConst->pParse->db, pConst->ap_व्यञ्जकम्,
                         pConst->nConst*2*परिमाण(व्यञ्जकम्*));
  यदि( pConst->ap_व्यञ्जकम्==0 ){
    pConst->nConst = 0;
  }अन्यथा{
    pConst->ap_व्यञ्जकम्[pConst->nConst*2-2] = pColumn;
    pConst->ap_व्यञ्जकम्[pConst->nConst*2-1] = p_मानम्;
  }
}

/*
** Find all terms of COLUMN=VALUE or VALUE=COLUMN in pExpr where VALUE
** is a constant expression and where the term must be true because it
** is part of the AND-connected terms of the expression.  For each term
** found, add it to the pConst structure.
*/
स्थिर शून्य find_Const_अन्तर्_Where(WhereConst *pConst, व्यञ्जकम् *p_व्यञ्जकम्){
  व्यञ्जकम् *p_दक्षिणः, *p_वामः;
  यदि( NEVER(p_व्यञ्जकम्==0) ) प्रत्यागम;
  यदि( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, pConst->mExcludeOn) ){
    testcase( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON) );
    testcase( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_InnerON) );
    प्रत्यागम;
  }
  यदि( p_व्यञ्जकम्->op==TK_AND ){
    find_Const_अन्तर्_Where(pConst, p_व्यञ्जकम्->p_दक्षिणः);
    find_Const_अन्तर्_Where(pConst, p_व्यञ्जकम्->p_वामः);
    प्रत्यागम;
  }
  यदि( p_व्यञ्जकम्->op!=TK_EQ ) प्रत्यागम;
  p_दक्षिणः = p_व्यञ्जकम्->p_दक्षिणः;
  p_वामः = p_व्यञ्जकम्->p_वामः;
  प्रतिज्ञा( p_दक्षिणः!=0 );
  प्रतिज्ञा( p_वामः!=0 );
  यदि( p_दक्षिणः->op==TK_COLUMN && sqlite3_व्यञ्जकम्_Is_Constant(pConst->pParse, p_वामः) ){
    const_निवेशय(pConst,p_दक्षिणः,p_वामः,p_व्यञ्जकम्);
  }
  यदि( p_वामः->op==TK_COLUMN && sqlite3_व्यञ्जकम्_Is_Constant(pConst->pParse, p_दक्षिणः) ){
    const_निवेशय(pConst,p_वामः,p_दक्षिणः,p_व्यञ्जकम्);
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
स्थिर पूर्णाङ्क propagate_Constant_व्यञ्जकम्_Rewrite_One(
  WhereConst *pConst,
  व्यञ्जकम् *p_व्यञ्जकम्,
  पूर्णाङ्क bIgnoreAffBlob
){
  पूर्णाङ्क i;
  यदि( pConst->pOomFault[0] ) प्रत्यागम WRC_Prune;
  यदि( p_व्यञ्जकम्->op!=TK_COLUMN ) प्रत्यागम WRC_Continue;
  यदि( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_FixedCol|pConst->mExcludeOn) ){
    testcase( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_FixedCol) );
    testcase( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON) );
    testcase( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_InnerON) );
    प्रत्यागम WRC_Continue;
  }
  प्रति(i=0; i<pConst->nConst; i++){
    व्यञ्जकम् *pColumn = pConst->ap_व्यञ्जकम्[i*2];
    यदि( pColumn==p_व्यञ्जकम् ) अनुवर्त;
    यदि( pColumn->i_सारणी!=p_व्यञ्जकम्->i_सारणी ) अनुवर्त;
    यदि( pColumn->iColumn!=p_व्यञ्जकम्->iColumn ) अनुवर्त;
    प्रतिज्ञा( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
    यदि( bIgnoreAffBlob && sqlite3_व्यञ्जकम्_Affinity(pColumn)<=SQLITE_AFF_BLOB ){
      भङ्ग;
    }
    /* A match is found.  Add the EP_FixedCol property */
    pConst->nChng++;
    व्यञ्जकम्_Clear_Property(p_व्यञ्जकम्, EP_पर्णम्);
    व्यञ्जकम्_समुच्चयः_Property(p_व्यञ्जकम्, EP_FixedCol);
    प्रतिज्ञा( p_व्यञ्जकम्->p_वामः==0 );
    p_व्यञ्जकम्->p_वामः = sqlite3_व्यञ्जकम्_Dup(pConst->pParse->db, pConst->ap_व्यञ्जकम्[i*2+1], 0);
    यदि( pConst->pParse->db->mallocFailed ) प्रत्यागम WRC_Prune;
    भङ्ग;
  }
  प्रत्यागम WRC_Prune;
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
स्थिर पूर्णाङ्क propagate_Constant_व्यञ्जकम्_Rewrite(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  WhereConst *pConst = pWalker->u.pConst;
  प्रतिज्ञा( TK_GT==TK_EQ+1 );
  प्रतिज्ञा( TK_LE==TK_EQ+2 );
  प्रतिज्ञा( TK_LT==TK_EQ+3 );
  प्रतिज्ञा( TK_GE==TK_EQ+4 );
  यदि( pConst->bHasAffBlob ){
    यदि( (p_व्यञ्जकम्->op>=TK_EQ && p_व्यञ्जकम्->op<=TK_GE)
     || p_व्यञ्जकम्->op==TK_IS
    ){
      propagate_Constant_व्यञ्जकम्_Rewrite_One(pConst, p_व्यञ्जकम्->p_वामः, 0);
      यदि( pConst->pOomFault[0] ) प्रत्यागम WRC_Prune;
      यदि( sqlite3_व्यञ्जकम्_Affinity(p_व्यञ्जकम्->p_वामः)!=SQLITE_AFF_TEXT ){
        propagate_Constant_व्यञ्जकम्_Rewrite_One(pConst, p_व्यञ्जकम्->p_दक्षिणः, 0);
      }
    }
  }
  प्रत्यागम propagate_Constant_व्यञ्जकम्_Rewrite_One(pConst, p_व्यञ्जकम्, pConst->bHasAffBlob);
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
स्थिर पूर्णाङ्क propagateConstants(
  Parse *pParse,   /* The parsing context */
  Select *p        /* The query in which to propagate constants */
){
  WhereConst x;
  Walker w;
  पूर्णाङ्क nChng = 0;
  x.pParse = pParse;
  x.pOomFault = &pParse->db->mallocFailed;
  कुरु{
    x.nConst = 0;
    x.nChng = 0;
    x.ap_व्यञ्जकम् = 0;
    x.bHasAffBlob = 0;
    यदि( ALWAYS(p->pSrc!=0)
     && p->pSrc->nSrc>0
     && (p->pSrc->a[0].fg.jointype & JT_LTORJ)!=0
    ){
      /* Do not propagate constants on any ON clause if there is a
      ** RIGHT JOIN anywhere in the query */
      x.mExcludeOn = EP_InnerON | EP_OuterON;
    }अन्यथा{
      /* Do not propagate constants through the ON clause of a LEFT JOIN */
      x.mExcludeOn = EP_OuterON;
    }
    find_Const_अन्तर्_Where(&x, p->pWhere);
    यदि( x.nConst ){
      स्मृतिं_पूरय(&w, 0, परिमाण(w));
      w.pParse = pParse;
      w.x_व्यञ्जकम्_Callback = propagate_Constant_व्यञ्जकम्_Rewrite;
      w.xSelectCallback = sqlite3SelectWalkNoop;
      w.xSelectCallback2 = 0;
      w.walker_गभीरता = 0;
      w.u.pConst = &x;
      sqlite3_Walk_व्यञ्जकम्(&w, p->pWhere);
      sqlite3DbFree(x.pParse->db, x.ap_व्यञ्जकम्);
      nChng += x.nChng;
    }
  }यावत्( x.nChng ); 
  प्रत्यागम nChng;
}

#यदि !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
# यदि !defined(SQLITE_OMIT_WINDOWFUNC)
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
स्थिर पूर्णाङ्क प्रक्षिप_अधस्तात्_गवाक्षः_Check(Parse *pParse, Select *pSubq, व्यञ्जकम् *p_व्यञ्जकम्){
  प्रतिज्ञा( pSubq->pWin->pPartition );
  प्रतिज्ञा( (pSubq->selFlags & SF_MultiPart)==0 );
  प्रतिज्ञा( pSubq->p_प्राक्==0 );
  प्रत्यागम sqlite3_व्यञ्जकम्_Is_Constant_Or_गणः_By(pParse, p_व्यञ्जकम्, pSubq->pWin->pPartition);
}
# endif /* SQLITE_OMIT_WINDOWFUNC */
#यदि_अन्तः /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#यदि !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
स्थिर पूर्णाङ्क प्रक्षिप_अधस्तात्_Where_Terms(
  Parse *pParse,        /* Parse context (for malloc() and error reporting) */
  Select *pSubq,        /* The subquery whose WHERE clause is to be augmented */
  व्यञ्जकम् *pWhere,         /* The WHERE clause of the outer query */
  SrcList *pSrcList,    /* The complete from clause of the outer query */
  पूर्णाङ्क iSrc              /* Which FROM clause term to try to push into  */
){
  व्यञ्जकम् *pNew;
  SrcItem *pSrc;        /* The subquery FROM term into which WHERE is pushed */
  पूर्णाङ्क nChng = 0;
  pSrc = &pSrcList->a[iSrc];
  यदि( pWhere==0 ) प्रत्यागम 0;
  यदि( pSubq->selFlags & (SF_Recursive|SF_MultiPart) ){
    प्रत्यागम 0;           /* restrictions (2) and (11) */
  }
  यदि( pSrc->fg.jointype & (JT_LTORJ|JT_दक्षिणः) ){
    प्रत्यागम 0;           /* restrictions (10) */
  }

  यदि( pSubq->p_प्राक् ){
    Select *pSel;
    पूर्णाङ्क notUnionAll = 0;
    प्रति(pSel=pSubq; pSel; pSel=pSel->p_प्राक्){
      u8 op = pSel->op;
      प्रतिज्ञा( op==TK_ALL || op==TK_SELECT
           || op==TK_UNION || op==TK_INTERSECT || op==TK_EXCEPT );
      यदि( op!=TK_ALL && op!=TK_SELECT ){
        notUnionAll = 1;
      }
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
      यदि( pSel->pWin ) प्रत्यागम 0;    /* restriction (6b) */
#यदि_अन्तः
    }
    यदि( notUnionAll ){
      /* If any of the compound arms are connected using UNION, INTERSECT,
      ** or EXCEPT, then we must ensure that none of the columns use a
      ** non-BINARY collating sequence. */
      प्रति(pSel=pSubq; pSel; pSel=pSel->p_प्राक्){
        पूर्णाङ्क ii;
        ध्रुव व्यञ्जकम्_List *pList = pSel->pEList;
        प्रतिज्ञा( pList!=0 );
        प्रति(ii=0; ii<pList->n_व्यञ्जकम्; ii++){
          CollSeq *pColl = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, pList->a[ii].p_व्यञ्जकम्);
          यदि( !sqlite3_Is_द्विमान(pColl) ){
            प्रत्यागम 0;  /* Restriction (8) */
          }
        }
      }
    }
  }अन्यथा{
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
    यदि( pSubq->pWin && pSubq->pWin->pPartition==0 ) प्रत्यागम 0;
#यदि_अन्तः
  }

#यदि_परिभाषितम् SQLITE_DEBUG
  /* Only the first term of a compound can have a WITH clause.  But make
  ** sure no other terms are marked SF_Recursive in case something changes
  ** in the future.
  */
  {
    Select *pX; 
    प्रति(pX=pSubq; pX; pX=pX->p_प्राक्){
      प्रतिज्ञा( (pX->selFlags & (SF_Recursive))==0 );
    }
  }
#यदि_अन्तः

  यदि( pSubq->p_सीमा!=0 ){
    प्रत्यागम 0; /* restriction (3) */
  }
  यावत्( pWhere->op==TK_AND ){
    nChng += प्रक्षिप_अधस्तात्_Where_Terms(pParse, pSubq, pWhere->p_दक्षिणः, pSrcList, iSrc);
    pWhere = pWhere->p_वामः;
  }

#यदि 0 /* These checks now done by sqlite3ExprIsSingleTableConstraint() */
  यदि( व्यञ्जकम्_Has_Property(pWhere, EP_OuterON|EP_InnerON) /* (9a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (9c) */
  ){
    पूर्णाङ्क jj;
    प्रति(jj=0; jj<iSrc; jj++){
      यदि( pWhere->w.iJoin==pSrcList->a[jj].iCursor ){
        /* If we reach this point, both (9a) and (9b) are satisfied.
        ** The following loop checks (9c):
        */
        प्रति(jj++; jj<iSrc; jj++){
          यदि( (pSrcList->a[jj].fg.jointype & JT_दक्षिणः)!=0 ){
            प्रत्यागम 0;  /* restriction (9) */
          }
        }
      }
    }
  }
  यदि( is_वामः_Join
   && (व्यञ्जकम्_Has_Property(pWhere,EP_OuterON)==0
         || pWhere->w.iJoin!=iCursor)
  ){
    प्रत्यागम 0; /* restriction (4) */
  }
  यदि( व्यञ्जकम्_Has_Property(pWhere,EP_OuterON)
   && pWhere->w.iJoin!=iCursor
  ){
    प्रत्यागम 0; /* restriction (5) */
  }
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_ALLOW_ROWID_अन्तर्_VIEW
  यदि( ViewCanHaveRowid && (pWhere->op==TK_ISNULL || pWhere->op==TK_NOTNULL) ){
    व्यञ्जकम् *p_वामः = pWhere->p_वामः;
    यदि( ALWAYS(p_वामः) 
     && p_वामः->op==TK_COLUMN
     && p_वामः->iColumn < 0
    ){
      प्रत्यागम 0;  /* Restriction (12) */
    }
  }
#यदि_अन्तः

  यदि( sqlite3_व्यञ्जकम्_Is_Single_सारणी_Constraint(pWhere, pSrcList, iSrc, 1) ){
    nChng++;
    pSubq->selFlags |= SF_प्रक्षिप_अधस्तात्;
    यावत्( pSubq ){
      SubstContext x;
      pNew = sqlite3_व्यञ्जकम्_Dup(pParse->db, pWhere, 0);
      unset_Join_व्यञ्जकम्(pNew, -1, 1);
      x.pParse = pParse;
      x.i_सारणी = pSrc->iCursor;
      x.i_New_सारणी = pSrc->iCursor;
      x.isOuterJoin = 0;
      x.n_Sel_गभीरता = 0;
      x.pEList = pSubq->pEList;
      x.pCList = findLeftmostExprlist(pSubq);
      pNew = subst_व्यञ्जकम्(&x, pNew);
      प्रतिज्ञा( pNew!=0 || pParse->nErr!=0 );
      यदि( pParse->nErr==0 && pNew->op==TK_अन्तर् && व्यञ्जकम्_Use_XSelect(pNew) ){
        प्रतिज्ञा( pNew->x.pSelect!=0 );
        pNew->x.pSelect->selFlags |= SF_Cloned_Rhs_अन्तर्;
        प्रतिज्ञा( pWhere!=0 );
        प्रतिज्ञा( pWhere->op==TK_अन्तर् );
        प्रतिज्ञा( व्यञ्जकम्_Use_XSelect(pWhere) );
        प्रतिज्ञा( pWhere->x.pSelect!=0 );
        pWhere->x.pSelect->selFlags |= SF_Cloned_Rhs_अन्तर्;
      }
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
      यदि( pSubq->pWin && 0==प्रक्षिप_अधस्तात्_गवाक्षः_Check(pParse, pSubq, pNew) ){
        /* Restriction 6c has prevented push-down in this case */
        sqlite3_व्यञ्जकम्_Delete(pParse->db, pNew);
        nChng--;
        भङ्ग;
      }
#यदि_अन्तः
      यदि( pSubq->selFlags & SF_Aggregate ){
        pSubq->pHaving = sqlite3_व्यञ्जकम्_And(pParse, pSubq->pHaving, pNew);
      }अन्यथा{
        pSubq->pWhere = sqlite3_व्यञ्जकम्_And(pParse, pSubq->pWhere, pNew);
      }
      pSubq = pSubq->p_प्राक्;
    }
  }
  प्रत्यागम nChng;
}
#यदि_अन्तः /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** Check to see if a subquery contains result-set columns that are
** never used.  If it does, change the value of those result-set columns
** to NULL so that they do not cause unnecessary work to compute.
**
** Return the number of column that were changed to NULL.
*/
स्थिर पूर्णाङ्क disable_अप्रयुक्तम्_Subquery_परिणाम_Columns(SrcItem *pItem){
  पूर्णाङ्क nCol;
  Select *pSub;      /* The subquery to be simplified */
  Select *pX;        /* For looping over compound elements of pSub */
  सारणी *pTab;       /* The table that describes the subquery */
  पूर्णाङ्क j;             /* Column number */
  पूर्णाङ्क nChng = 0;     /* Number of columns converted to NULL */
  Bitmask col_प्रयुक्तम्;   /* Columns that may not be NULLed out */

  प्रतिज्ञा( pItem!=0 );
  यदि( pItem->fg.isCorrelated || pItem->fg.isCte ){
    प्रत्यागम 0;
  }
  प्रतिज्ञा( pItem->pSTab!=0 );
  pTab = pItem->pSTab;
  प्रतिज्ञा( pItem->fg.isSubquery );
  pSub = pItem->u4.pSubq->pSelect;
  प्रतिज्ञा( pSub->pEList->n_व्यञ्जकम्==pTab->nCol );
  प्रति(pX=pSub; pX; pX=pX->p_प्राक्){
    यदि( (pX->selFlags & (SF_Distinct|SF_Aggregate))!=0 ){
      testcase( pX->selFlags & SF_Distinct );
      testcase( pX->selFlags & SF_Aggregate );
      प्रत्यागम 0;
    }
    यदि( pX->p_प्राक् && pX->op!=TK_ALL ){
      /* This optimization does not work for compound subqueries that
      ** use UNION, INTERSECT, or EXCEPT.  Only UNION ALL is allowed. */
      प्रत्यागम 0;
    }
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
    यदि( pX->pWin ){
      /* This optimization does not work for subqueries that use window
      ** functions. */
      प्रत्यागम 0;
    }
#यदि_अन्तः
  }
  col_प्रयुक्तम् = pItem->col_प्रयुक्तम्;
  यदि( pSub->p_क्रमः_By ){
    व्यञ्जकम्_List *pList = pSub->p_क्रमः_By;
    प्रति(j=0; j<pList->n_व्यञ्जकम्; j++){
      u16 iCol = pList->a[j].u.x.i_क्रमः_By_Col;
      यदि( iCol>0 ){
        iCol--;
        col_प्रयुक्तम् |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
      }
    }
  }
  nCol = pTab->nCol;
  प्रति(j=0; j<nCol; j++){
    Bitmask m = j<BMS-1 ? MASKBIT(j) : TOPBIT;
    यदि( (m & col_प्रयुक्तम्)!=0 ) अनुवर्त;
    प्रति(pX=pSub; pX; pX=pX->p_प्राक्) {
      व्यञ्जकम् *pY = pX->pEList->a[j].p_व्यञ्जकम्;
      यदि( pY->op==TK_NULL ) अनुवर्त;
      pY->op = TK_NULL;
      व्यञ्जकम्_Clear_Property(pY, EP_Skip|EP_Unlikely);
      pX->selFlags |= SF_प्रक्षिप_अधस्तात्;
      nChng++;
    }
  }
  प्रत्यागम nChng;
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
स्थिर u8 minMaxQuery(sqlite3 *db, व्यञ्जकम् *pFunc, व्यञ्जकम्_List **ppMinMax){
  पूर्णाङ्क eRet = WHERE_ORDERBY_प्रसामान्यम्;      /* Return value */
  व्यञ्जकम्_List *pEList;                     /* Arguments to agg function */
  ध्रुव वर्ण *zFunc;                    /* Name of aggregate function pFunc */
  व्यञ्जकम्_List *p_क्रमः_By;
  u8 क्रमय_Flags = 0;

  प्रतिज्ञा( *ppMinMax==0 );
  प्रतिज्ञा( pFunc->op==TK_AGG_FUNCTION );
  प्रतिज्ञा( !Is_गवाक्षः_Func(pFunc) );
  प्रतिज्ञा( व्यञ्जकम्_Use_XList(pFunc) );
  pEList = pFunc->x.pList;
  यदि( pEList==0
   || pEList->n_व्यञ्जकम्!=1
   || व्यञ्जकम्_Has_Property(pFunc, EP_WinFunc)
   || OptimizationDisabled(db, SQLITE_MinMaxOpt)
  ){
    प्रत्यागम eRet;
  }
  प्रतिज्ञा( !व्यञ्जकम्_Has_Property(pFunc, EP_Int_मानम्) );
  zFunc = pFunc->u.zToken;
  यदि( sqlite3StrICmp(zFunc, "min")==0 ){
    eRet = WHERE_ORDERBY_MIN;
    यदि( sqlite3_व्यञ्जकम्_Can_Be_Null(pEList->a[0].p_व्यञ्जकम्) ){
      क्रमय_Flags = KEYINFO_क्रमः_BIGNULL;
    }
  }अन्यथा यदि( sqlite3StrICmp(zFunc, "max")==0 ){
    eRet = WHERE_ORDERBY_MAX;
    क्रमय_Flags = KEYINFO_क्रमः_DESC;
  }अन्यथा{
    प्रत्यागम eRet;
  }
  *ppMinMax = p_क्रमः_By = sqlite3_व्यञ्जकम्_List_Dup(db, pEList, 0);
  प्रतिज्ञा( p_क्रमः_By!=0 || db->mallocFailed );
  यदि( p_क्रमः_By ) p_क्रमः_By->a[0].fg.क्रमय_Flags = क्रमय_Flags;
  प्रत्यागम eRet;
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
स्थिर सारणी *is_Simple_गणनम्(Select *p, AggInfo *pAggInfo){
  सारणी *pTab;
  व्यञ्जकम् *p_व्यञ्जकम्;

  प्रतिज्ञा( !p->p_गणः_By );

  यदि( p->pWhere
   || p->pEList->n_व्यञ्जकम्!=1
   || p->pSrc->nSrc!=1
   || p->pSrc->a[0].fg.isSubquery
   || pAggInfo->nFunc!=1
   || p->pHaving
  ){
    प्रत्यागम 0;
  }
  pTab = p->pSrc->a[0].pSTab;
  प्रतिज्ञा( pTab!=0 );
  प्रतिज्ञा( !IsView(pTab) );
  यदि( !Is_Ordinary_सारणी(pTab) ) प्रत्यागम 0;
  p_व्यञ्जकम् = p->pEList->a[0].p_व्यञ्जकम्;
  प्रतिज्ञा( p_व्यञ्जकम्!=0 );
  यदि( p_व्यञ्जकम्->op!=TK_AGG_FUNCTION ) प्रत्यागम 0;
  यदि( p_व्यञ्जकम्->pAggInfo!=pAggInfo ) प्रत्यागम 0;
  यदि( (pAggInfo->aFunc[0].pFunc->funcFlags&SQLITE_FUNC_गणनम्)==0 ) प्रत्यागम 0;
  प्रतिज्ञा( pAggInfo->aFunc[0].pFExpr==p_व्यञ्जकम् );
  testcase( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Distinct) );
  testcase( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_WinFunc) );
  यदि( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Distinct|EP_WinFunc) ) प्रत्यागम 0;

  प्रत्यागम pTab;
}

/*
** If the source-list item passed as an argument was augmented with an
** INDEXED BY clause, then try to locate the specified index. If there
** was such a clause and the named index cannot be found, return
** SQLITE_ERROR and leave an error in pParse. Otherwise, populate
** pFrom->pIndex and return SQLITE_OK.
*/
पूर्णाङ्क sqlite3_अनुक्रमितम्_By_Lookup(Parse *pParse, SrcItem *pFrom){
  सारणी *pTab = pFrom->pSTab;
  वर्ण *z_अनुक्रमितम्_By = pFrom->u1.z_अनुक्रमितम्_By;
  सूचकांक *pIdx;
  प्रतिज्ञा( pTab!=0 );
  प्रतिज्ञा( pFrom->fg.is_अनुक्रमितम्_By!=0 );

  प्रति(pIdx=pTab->p_सूचकांक;
      pIdx && sqlite3StrICmp(pIdx->zName, z_अनुक्रमितम्_By);
      pIdx=pIdx->p_अग्र
  );
  यदि( !pIdx ){
    sqlite3_Error_सन्देशः(pParse, "no such index: %s", z_अनुक्रमितम्_By, 0);
    pParse->checkSchema = 1;
    प्रत्यागम SQLITE_ERROR;
  }
  प्रतिज्ञा( pFrom->fg.isCte==0 );
  pFrom->u2.pIBIndex = pIdx;
  प्रत्यागम SQLITE_OK;
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
स्थिर पूर्णाङ्क convert_Compound_Select_गन्तव्यम्_Subquery(Walker *pWalker, Select *p){
  पूर्णाङ्क i;
  Select *pNew;
  Select *pX;
  sqlite3 *db;
  संरचना व्यञ्जकम्_List_item *a;
  SrcList *pNewSrc;
  Parse *pParse;
  Token dummy;

  यदि( p->p_प्राक्==0 ) प्रत्यागम WRC_Continue;
  यदि( p->p_क्रमः_By==0 ) प्रत्यागम WRC_Continue;
  प्रति(pX=p; pX && (pX->op==TK_ALL || pX->op==TK_SELECT); pX=pX->p_प्राक्){}
  यदि( pX==0 ) प्रत्यागम WRC_Continue;
  a = p->p_क्रमः_By->a;
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
  /* If iOrderByCol is already non-zero, then it has already been matched
  ** to a result column of the SELECT statement. This occurs when the
  ** SELECT is rewritten for window-functions processing and then passed
  ** to sqlite3SelectPrep() and similar a second time. The rewriting done
  ** by this function is not required in this case. */
  यदि( a[0].u.x.i_क्रमः_By_Col ) प्रत्यागम WRC_Continue;
#यदि_अन्तः
  प्रति(i=p->p_क्रमः_By->n_व्यञ्जकम्-1; i>=0; i--){
    यदि( a[i].p_व्यञ्जकम्->flags & EP_Collate ) भङ्ग;
  }
  यदि( i<0 ) प्रत्यागम WRC_Continue;

  /* If we reach this point, that means the transformation is required. */

  pParse = pWalker->pParse;
  db = pParse->db;
  pNew = sqlite3DbMallocZero(db, परिमाण(*pNew) );
  यदि( pNew==0 ) प्रत्यागम WRC_Abort;
  स्मृतिं_पूरय(&dummy, 0, परिमाण(dummy));
  pNewSrc = sqlite3_Src_List_Append_From_पदांशः(pParse,0,0,0,&dummy,pNew,0);
  प्रतिज्ञा( pNewSrc!=0 || pParse->nErr );
  यदि( pParse->nErr ){
    sqlite3SrcListDelete(db, pNewSrc);
    प्रत्यागम WRC_Abort;
  }
  *pNew = *p;
  p->pSrc = pNewSrc;
  p->pEList = sqlite3_व्यञ्जकम्_List_Append(pParse, 0, sqlite3_व्यञ्जकम्(db, TK_ASTERISK, 0));
  p->op = TK_SELECT;
  p->pWhere = 0;
  p->p_गणः_By = 0;
  p->pHaving = 0;
  pNew->p_क्रमः_By = 0;
  p->p_प्राक् = 0;
  p->p_अग्र = 0;
  p->pWith = 0;
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
  p->pWinDefn = 0;
#यदि_अन्तः
  p->selFlags &= ~(u32)(SF_Compound|SF_Distinct);
  प्रतिज्ञा( (p->selFlags & SF_Converted)==0 );
  p->selFlags |= SF_Converted;
  प्रतिज्ञा( pNew->p_प्राक्!=0 );
  pNew->p_प्राक्->p_अग्र = pNew;
  pNew->p_सीमा = 0;
  प्रत्यागम WRC_Continue;
}

/*
** Check to see if the FROM clause term pFrom has table-valued function
** arguments.  If it does, leave an error message in pParse and return
** non-zero, since pFrom is not allowed to be a table-valued function.
*/
स्थिर पूर्णाङ्क cannotBeFunction(Parse *pParse, SrcItem *pFrom){
  यदि( pFrom->fg.isTabFunc ){
    sqlite3_Error_सन्देशः(pParse, "'%s' is not a function", pFrom->zName);
    प्रत्यागम 1;
  }
  प्रत्यागम 0;
}

#यदि_अपरिभाषितम् SQLITE_OMIT_CTE
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
स्थिर संरचना Cte *अन्वेषणम्_With(
  With *pWith,                    /* Current innermost WITH clause */
  SrcItem *pItem,                 /* FROM clause element to resolve */
  With **ppContext                /* OUT: WITH clause return value belongs to */
){
  ध्रुव वर्ण *zName = pItem->zName;
  With *p;
  प्रतिज्ञा( pItem->fg.fixedSchema || pItem->u4.zDatabase==0 );
  प्रतिज्ञा( zName!=0 );
  प्रति(p=pWith; p; p=p->pOuter){
    पूर्णाङ्क i;
    प्रति(i=0; i<p->nCte; i++){
      यदि( sqlite3StrICmp(zName, p->a[i].zName)==0 ){
        *ppContext = p;
        प्रत्यागम &p->a[i];
      }
    }
    यदि( p->bView ) भङ्ग;
  }
  प्रत्यागम 0;
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
With *sqlite3_With_प्रक्षिप(Parse *pParse, With *pWith, u8 bFree){
  यदि( pWith ){
    यदि( bFree ){
      pWith = (With*)sqlite3_Parser_सङ्कलय_Cleanup(pParse, sqlite3WithDeleteGeneric,
                      pWith);
      यदि( pWith==0 ) प्रत्यागम 0;
    }
    यदि( pParse->nErr==0 ){
      प्रतिज्ञा( pParse->pWith!=pWith );
      pWith->pOuter = pParse->pWith;
      pParse->pWith = pWith;
    }
  }
  प्रत्यागम pWith;
}

स्थिर पूर्णाङ्क selectExpanderWalk(Walker*, Select*);

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
स्थिर पूर्णाङ्क resolve_From_पदांशः_गन्तव्यम्_Cte(
  Parse *pParse,                  /* The parsing context */
  Walker *pWalker,                /* Current tree walker */
  SrcItem *pFrom                  /* The FROM clause term to check */
){
  Cte *pCte;               /* Matched CTE (or NULL if no match) */
  With *pWith;             /* The matching WITH */

  प्रतिज्ञा( pFrom->pSTab==0 );
  यदि( pParse->pWith==0 ){
    /* There are no WITH clauses in the stack.  No match is possible */
    प्रत्यागम 0;
  }
  यदि( pParse->nErr ){
    /* Prior errors might have left pParse->pWith in a goofy state, so
    ** go no further. */
    प्रत्यागम 0;
  }
  प्रतिज्ञा( pFrom->fg.hadSchema==0 || pFrom->fg.notCte!=0 );
  यदि( pFrom->fg.fixedSchema==0 && pFrom->u4.zDatabase!=0 ){
    /* The FROM term contains a schema qualifier (ex: main.t1) and so
    ** it cannot possibly be a CTE reference. */
    प्रत्यागम 0;
  }
  यदि( pFrom->fg.notCte ){
    /* The FROM term is specifically excluded from matching a CTE.
    **   (1)  It is part of a trigger that used to have zDatabase but had
    **        zDatabase removed by sqlite3FixTriggerStep().
    **   (2)  This is the first term in the FROM clause of an UPDATE.
    */
    प्रत्यागम 0;
  }
  pCte = अन्वेषणम्_With(pParse->pWith, pFrom, &pWith);
  यदि( pCte ){
    sqlite3 *db = pParse->db;
    सारणी *pTab;
    व्यञ्जकम्_List *pEList;
    Select *pSel;
    Select *p_वामः;                /* Left-most SELECT statement */
    Select *p_Rec_पदांशः;             /* Left-most recursive term */
    पूर्णाङ्क bMayRecursive;            /* True if compound joined by UNION [ALL] */
    With *pSavedWith;             /* Initial value of pParse->pWith */
    पूर्णाङ्क iRecTab = -1;             /* Cursor for recursive table */
    CteUse *pCteUse;

    /* If pCte->zCteErr is non-NULL at this point, then this is an illegal
    ** recursive reference to CTE pCte. Leave an error in pParse and return
    ** early. If pCte->zCteErr is NULL, then this is not a recursive reference.
    ** In this case, proceed.  */
    यदि( pCte->zCteErr ){
      sqlite3_Error_सन्देशः(pParse, pCte->zCteErr, pCte->zName);
      प्रत्यागम 2;
    }
    यदि( cannotBeFunction(pParse, pFrom) ) प्रत्यागम 2;

    प्रतिज्ञा( pFrom->pSTab==0 );
    pTab = sqlite3DbMallocZero(db, परिमाण(सारणी));
    यदि( pTab==0 ) प्रत्यागम 2;
    pCteUse = pCte->pUse;
    यदि( pCteUse==0 ){
      pCte->pUse = pCteUse = sqlite3DbMallocZero(db, परिमाण(pCteUse[0]));
      यदि( pCteUse==0
       || sqlite3_Parser_सङ्कलय_Cleanup(pParse,sqlite3DbFree,pCteUse)==0
      ){
        sqlite3DbFree(db, pTab);
        प्रत्यागम 2;
      }
      pCteUse->eM10d = pCte->eM10d;
    }
    pFrom->pSTab = pTab;
    pTab->nTabRef = 1;
    pTab->zName = sqlite3DbStrDup(db, pCte->zName);
    pTab->iPKey = -1;
    pTab->nRowLogEst = 200; प्रतिज्ञा( 200==sqlite3LogEst(1048576) );
    pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
    sqlite3SrcItemAttachSubquery(pParse, pFrom, pCte->pSelect, 1);
    यदि( db->mallocFailed ) प्रत्यागम 2;
    प्रतिज्ञा( pFrom->fg.isSubquery && pFrom->u4.pSubq );
    pSel = pFrom->u4.pSubq->pSelect;
    प्रतिज्ञा( pSel!=0 );
    pSel->selFlags |= SF_CopyCte;
    यदि( pFrom->fg.is_अनुक्रमितम्_By ){
      sqlite3_Error_सन्देशः(pParse, "no such index: \"%s\"", pFrom->u1.z_अनुक्रमितम्_By);
      प्रत्यागम 2;
    }
    प्रतिज्ञा( !pFrom->fg.is_अनुक्रमितम्_By );
    pFrom->fg.isCte = 1;
    pFrom->u2.pCteUse = pCteUse;
    pCteUse->nUse++;

    /* Check if this is a recursive CTE. */
    p_Rec_पदांशः = pSel;
    bMayRecursive = ( pSel->op==TK_ALL || pSel->op==TK_UNION );
    यावत्( bMayRecursive && p_Rec_पदांशः->op==pSel->op ){
      पूर्णाङ्क i;
      SrcList *pSrc = p_Rec_पदांशः->pSrc;
      प्रतिज्ञा( p_Rec_पदांशः->p_प्राक्!=0 );
      प्रति(i=0; i<pSrc->nSrc; i++){
        SrcItem *pItem = &pSrc->a[i];
        यदि( pItem->zName!=0
         && !pItem->fg.hadSchema
         && ALWAYS( !pItem->fg.isSubquery )
         && (pItem->fg.fixedSchema || pItem->u4.zDatabase==0)
         && 0==sqlite3StrICmp(pItem->zName, pCte->zName)
        ){
          pItem->pSTab = pTab;
          pTab->nTabRef++;
          pItem->fg.isRecursive = 1;
          यदि( p_Rec_पदांशः->selFlags & SF_Recursive ){
            sqlite3_Error_सन्देशः(pParse,
               "multiple references to recursive table: %s", pCte->zName
            );
            प्रत्यागम 2;
          }
          p_Rec_पदांशः->selFlags |= SF_Recursive;
          यदि( iRecTab<0 ) iRecTab = pParse->nTab++;
          pItem->iCursor = iRecTab;
        }
      }
      यदि( (p_Rec_पदांशः->selFlags & SF_Recursive)==0 ) भङ्ग;
      p_Rec_पदांशः = p_Rec_पदांशः->p_प्राक्;
    }

    यदि( pParse->nTab >= db->a_सीमा[SQLITE_सीमा_CTES]
     && db->a_सीमा[SQLITE_सीमा_CTES]>0
    ){
      sqlite3_Error_सन्देशः(pParse, "too many CTEs");
      प्रत्यागम 2;
    }
#यदि SQLITE_MAX_व्यञ्जकम्_गभीरता>0
    pParse->nNestSel++;
    यदि( check_Subquery_Nesting_गभीरता(pParse) ) प्रत्यागम 2;
#यदि_अन्तः
    pCte->zCteErr = "circular reference: %s";
    pSavedWith = pParse->pWith;
    pParse->pWith = pWith;
    यदि( pSel->selFlags & SF_Recursive ){
      पूर्णाङ्क rc;
      प्रतिज्ञा( p_Rec_पदांशः!=0 );
      प्रतिज्ञा( (p_Rec_पदांशः->selFlags & SF_Recursive)==0 );
      प्रतिज्ञा( p_Rec_पदांशः->p_अग्र!=0 );
      प्रतिज्ञा( (p_Rec_पदांशः->p_अग्र->selFlags & SF_Recursive)!=0 );
      प्रतिज्ञा( p_Rec_पदांशः->pWith==0 );
      p_Rec_पदांशः->pWith = pSel->pWith;
      rc = sqlite3WalkSelect(pWalker, p_Rec_पदांशः);
      p_Rec_पदांशः->pWith = 0;
      यदि( rc ){
        pParse->pWith = pSavedWith;
        प्रत्यागम 2;
      }
    }अन्यथा{
      यदि( selectExpanderWalk(pWalker, pSel) ){
        pParse->pWith = pSavedWith;
        प्रत्यागम 2;
      }
    }
    pParse->pWith = pWith;
#यदि SQLITE_MAX_व्यञ्जकम्_गभीरता>0
    pParse->nNestSel--;
    प्रतिज्ञा( pParse->nNestSel>=0 );
#यदि_अन्तः

    प्रति(p_वामः=pSel; p_वामः->p_प्राक्; p_वामः=p_वामः->p_प्राक्);
    pEList = p_वामः->pEList;
    यदि( pCte->p_स्तम्भसङ्ख्या ){
      यदि( pEList && pEList->n_व्यञ्जकम्!=pCte->p_स्तम्भसङ्ख्या->n_व्यञ्जकम् ){
        sqlite3_Error_सन्देशः(pParse, "table %s has %d values for %d columns",
            pCte->zName, pEList->n_व्यञ्जकम्, pCte->p_स्तम्भसङ्ख्या->n_व्यञ्जकम्
        );
        pParse->pWith = pSavedWith;
        प्रत्यागम 2;
      }
      pEList = pCte->p_स्तम्भसङ्ख्या;
    }

    sqlite3_Columns_From_व्यञ्जकम्_List(pParse, pEList, &pTab->nCol, &pTab->aCol);
    यदि( bMayRecursive ){
      यदि( pSel->selFlags & SF_Recursive ){
        pCte->zCteErr = "multiple recursive references: %s";
      }अन्यथा{
        pCte->zCteErr = "recursive reference in a subquery: %s";
      }
      sqlite3WalkSelect(pWalker, pSel);
    }
    pCte->zCteErr = 0;
    pParse->pWith = pSavedWith;
    प्रत्यागम 1;  /* Success */
  }
  प्रत्यागम 0;  /* No match */
}
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_CTE
/*
** If the SELECT passed as the second argument has an associated WITH
** clause, pop it from the stack stored as part of the Parse object.
**
** This function is used as the xSelectCallback2() callback by
** sqlite3SelectExpand() when walking a SELECT tree to resolve table
** names and other FROM clause elements.
*/
शून्य sqlite3_Select_निष्कासय_With(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  यदि( OK_IF_ALWAYS_TRUE(pParse->pWith) && p->p_प्राक्==0 ){
    With *pWith = findRightmost(p)->pWith;
    यदि( pWith!=0 ){
      प्रतिज्ञा( pParse->pWith==pWith || pParse->nErr );
      pParse->pWith = pWith->pOuter;
    }
  }
}
#यदि_अन्तः

/*
** The SrcItem structure passed as the second argument represents a
** sub-query in the FROM clause of a SELECT statement. This function
** allocates and populates the SrcItem.pTab object. If successful,
** SQLITE_OK is returned. Otherwise, if an OOM error is encountered,
** SQLITE_NOMEM.
*/
पूर्णाङ्क sqlite3ExpandSubquery(Parse *pParse, SrcItem *pFrom){
  Select *pSel;
  सारणी *pTab;

  प्रतिज्ञा( pFrom->fg.isSubquery );
  प्रतिज्ञा( pFrom->u4.pSubq!=0 );
  pSel = pFrom->u4.pSubq->pSelect;
  प्रतिज्ञा( pSel );
  pFrom->pSTab = pTab = sqlite3DbMallocZero(pParse->db, परिमाण(सारणी));
  यदि( pTab==0 ) प्रत्यागम SQLITE_NOMEM;
  pTab->nTabRef = 1;
  यदि( pFrom->zAlias ){
    pTab->zName = sqlite3DbStrDup(pParse->db, pFrom->zAlias);
  }अन्यथा{
    pTab->zName = sqlite3MPrintf(pParse->db, "%!S", pFrom);
  }
  यावत्( pSel->p_प्राक् ){ pSel = pSel->p_प्राक्; }
  sqlite3_Columns_From_व्यञ्जकम्_List(pParse, pSel->pEList,&pTab->nCol,&pTab->aCol);
  pTab->iPKey = -1;
  pTab->eTabType = TABTYP_VIEW;
  pTab->nRowLogEst = 200; प्रतिज्ञा( 200==sqlite3LogEst(1048576) );
#यदि_अपरिभाषितम् SQLITE_ALLOW_ROWID_अन्तर्_VIEW
  /* The usual case - do not allow ROWID on a subquery */
  pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
#अन्यथा
  /* Legacy compatibility mode */
  pTab->tabFlags |= TF_Ephemeral | sqlite3Config.mNoVisibleRowid;
#यदि_अन्तः
  प्रत्यागम pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
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
स्थिर पूर्णाङ्क अन्तर्_Any_Using_Clause(
  ध्रुव वर्ण *zName, /* Name we are looking for */
  SrcItem *p_आधारः,    /* The base SrcItem.  Looking at pBase[1] and following */
  पूर्णाङ्क N              /* How many SrcItems to check */
){
  यावत्( N>0 ){
    N--;
    p_आधारः++;
    यदि( p_आधारः->fg.isUsing==0 ) अनुवर्त;
    यदि( NEVER(p_आधारः->u3.pUsing==0) ) अनुवर्त;
    यदि( sqlite3_Id_List_सूचकांक(p_आधारः->u3.pUsing, zName)>=0 ) प्रत्यागम 1;
  }
  प्रत्यागम 0;
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
स्थिर पूर्णाङ्क selectExpander(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  पूर्णाङ्क i, j, k, rc;
  SrcList *pTabList;
  व्यञ्जकम्_List *pEList;
  SrcItem *pFrom;
  sqlite3 *db = pParse->db;
  व्यञ्जकम् *pE, *p_दक्षिणः, *p_व्यञ्जकम्;
  u16 selFlags = p->selFlags;
  u32 elistFlags = 0;

  p->selFlags |= SF_Expanded;
  यदि( db->mallocFailed  ){
    प्रत्यागम WRC_Abort;
  }
  प्रतिज्ञा( p->pSrc!=0 );
  यदि( (selFlags & SF_Expanded)!=0 ){
    प्रत्यागम WRC_Prune;
  }
  यदि( pWalker->e_संहिता ){
    /* Renumber selId because it has been copied from a view */
    p->selId = ++pParse->nSelect;
  }
  pTabList = p->pSrc;
  pEList = p->pEList;
  यदि( pParse->pWith && (p->selFlags & SF_View) ){
    यदि( p->pWith==0 ){
      p->pWith = (With*)sqlite3DbMallocZero(db, SZ_WITH(1) );
      यदि( p->pWith==0 ){
        प्रत्यागम WRC_Abort;
      }
    }
    p->pWith->bView = 1;
  }
  sqlite3_With_प्रक्षिप(pParse, p->pWith, 0);

  /* Make sure cursor numbers have been assigned to all entries in
  ** the FROM clause of the SELECT statement.
  */
  sqlite3_Src_List_नियोजय_Cursors(pParse, pTabList);

  /* Look up every table named in the FROM clause of the select.  If
  ** an entry of the FROM clause is a subquery instead of a table or view,
  ** then create a transient table structure to describe the subquery.
  */
  प्रति(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    सारणी *pTab;
    प्रतिज्ञा( pFrom->fg.isRecursive==0 || pFrom->pSTab!=0 );
    यदि( pFrom->pSTab ) अनुवर्त;
    प्रतिज्ञा( pFrom->fg.isRecursive==0 );
    यदि( pFrom->zName==0 ){
#यदि_अपरिभाषितम् SQLITE_OMIT_SUBQUERY
      Select *pSel;
      प्रतिज्ञा( pFrom->fg.isSubquery && pFrom->u4.pSubq!=0 );
      pSel = pFrom->u4.pSubq->pSelect;
      /* A sub-query in the FROM clause of a SELECT */
      प्रतिज्ञा( pSel!=0 );
      प्रतिज्ञा( pFrom->pSTab==0 );
      यदि( sqlite3WalkSelect(pWalker, pSel) ) प्रत्यागम WRC_Abort;
      यदि( sqlite3ExpandSubquery(pParse, pFrom) ) प्रत्यागम WRC_Abort;
#यदि_अन्तः
#यदि_अपरिभाषितम् SQLITE_OMIT_CTE
    }अन्यथा यदि( (rc = resolve_From_पदांशः_गन्तव्यम्_Cte(pParse, pWalker, pFrom))!=0 ){
      यदि( rc>1 ) प्रत्यागम WRC_Abort;
      pTab = pFrom->pSTab;
      प्रतिज्ञा( pTab!=0 );
#यदि_अन्तः
    }अन्यथा{
      /* An ordinary table or view name in the FROM clause */
      प्रतिज्ञा( pFrom->pSTab==0 );
      pFrom->pSTab = pTab = sqlite3_Locate_सारणी_Item(pParse, 0, pFrom);
      यदि( pTab==0 ) प्रत्यागम WRC_Abort;
      यदि( pTab->nTabRef>=0xffff ){
        sqlite3_Error_सन्देशः(pParse, "too many references to \"%s\": max 65535",
           pTab->zName);
        pFrom->pSTab = 0;
        प्रत्यागम WRC_Abort;
      }
      pTab->nTabRef++;
      यदि( !IsVirtual(pTab) && cannotBeFunction(pParse, pFrom) ){
        प्रत्यागम WRC_Abort;
      }
#यदि !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
      यदि( !Is_Ordinary_सारणी(pTab) ){
        i16 nCol;
        u8 e_संहिता_Orig = pWalker->e_संहिता;
        यदि( sqlite3_View_Get_Column_नामानि(pParse, pTab) ) प्रत्यागम WRC_Abort;
        प्रतिज्ञा( pFrom->fg.isSubquery==0 );
        यदि( IsView(pTab) ){
          यदि( (db->flags & SQLITE_EnableView)==0
           && pTab->pSchema!=db->aDb[1].pSchema
          ){
            sqlite3_Error_सन्देशः(pParse, "access to view \"%s\" prohibited",
              pTab->zName);
          }
          sqlite3SrcItemAttachSubquery(pParse, pFrom, pTab->u.view.pSelect, 1);
        }
#यदि_अपरिभाषितम् SQLITE_OMIT_VIRTUALTABLE
        अन्यथा यदि( ALWAYS(IsVirtual(pTab))
         && (pFrom->fg.fromDDL || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
         && ALWAYS(pTab->u.vtab.p!=0)
         && pTab->u.vtab.p->eVtabRisk > ((db->flags & SQLITE_TrustedSchema)!=0)
        ){
          sqlite3_Error_सन्देशः(pParse, "unsafe use of virtual table \"%s\"",
                                  pTab->zName);
        }
        प्रतिज्ञा( SQLITE_VTABRISK_प्रसामान्यम्==1 && SQLITE_VTABRISK_उच्चः==2 );
#यदि_अन्तः
        nCol = pTab->nCol;
        pTab->nCol = -1;
        pWalker->e_संहिता = 1;  /* Turn on Select.selId renumbering */
        यदि( pFrom->fg.isSubquery ){
          bft hasCompound = pParse->hasCompound;
          pParse->hasCompound = 1;
          selectExpanderWalk(pWalker, pFrom->u4.pSubq->pSelect);
          pParse->hasCompound = hasCompound;
        }
        pWalker->e_संहिता = e_संहिता_Orig;
        pTab->nCol = nCol;
      }
#यदि_अन्तः
    }

    /* Locate the index named by the INDEXED BY clause, if any. */
    यदि( pFrom->fg.is_अनुक्रमितम्_By && sqlite3_अनुक्रमितम्_By_Lookup(pParse, pFrom) ){
      प्रत्यागम WRC_Abort;
    }
  }

  /* Process NATURAL keywords, and ON and USING clauses of joins.
  */
  प्रतिज्ञा( db->mallocFailed==0 || pParse->nErr!=0 );
  यदि( pParse->nErr || sqlite3ProcessJoin(pParse, p) ){
    प्रत्यागम WRC_Abort;
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
  प्रति(k=0; k<pEList->n_व्यञ्जकम्; k++){
    pE = pEList->a[k].p_व्यञ्जकम्;
    यदि( pE->op==TK_ASTERISK ) भङ्ग;
    प्रतिज्ञा( pE->op!=TK_DOT || pE->p_दक्षिणः!=0 );
    प्रतिज्ञा( pE->op!=TK_DOT || (pE->p_वामः!=0 && pE->p_वामः->op==TK_ID) );
    यदि( pE->op==TK_DOT && pE->p_दक्षिणः->op==TK_ASTERISK ) भङ्ग;
    elistFlags |= pE->flags;
  }
  यदि( k<pEList->n_व्यञ्जकम् ){
    /*
    ** If we get here it means the result set contains one or more "*"
    ** operators that need to be expanded.  Loop through each expression
    ** in the result set and expand them one by one.
    */
    संरचना व्यञ्जकम्_List_item *a = pEList->a;
    व्यञ्जकम्_List *pNew = 0;
    पूर्णाङ्क flags = pParse->db->flags;
    पूर्णाङ्क long_नामानि = (flags & SQLITE_पूर्णम्_Col_नामानि)!=0
                      && (flags & SQLITE_Short_Col_नामानि)==0;

    प्रति(k=0; k<pEList->n_व्यञ्जकम्; k++){
      pE = a[k].p_व्यञ्जकम्;
      elistFlags |= pE->flags;
      p_दक्षिणः = pE->p_दक्षिणः;
      प्रतिज्ञा( pE->op!=TK_DOT || p_दक्षिणः!=0 );
      यदि( pE->op!=TK_ASTERISK
       && (pE->op!=TK_DOT || p_दक्षिणः->op!=TK_ASTERISK)
      ){
        /* This particular expression does not need to be expanded.
        */
        pNew = sqlite3_व्यञ्जकम्_List_Append(pParse, pNew, a[k].p_व्यञ्जकम्);
        यदि( pNew ){
          pNew->a[pNew->n_व्यञ्जकम्-1].zEName = a[k].zEName;
          pNew->a[pNew->n_व्यञ्जकम्-1].fg.eEName = a[k].fg.eEName;
          a[k].zEName = 0;
        }
        a[k].p_व्यञ्जकम् = 0;
      }अन्यथा{
        /* This expression is a "*" or a "TABLE.*" and needs to be
        ** expanded. */
        पूर्णाङ्क सारणी_आलोकितम् = 0;      /* Set to 1 when TABLE matches */
        वर्ण *zTName = 0;       /* text of name of TABLE */
        पूर्णाङ्क iErrOfst;
        यदि( pE->op==TK_DOT ){
          प्रतिज्ञा( (selFlags & SF_NestedFrom)==0 );
          प्रतिज्ञा( pE->p_वामः!=0 );
          प्रतिज्ञा( !व्यञ्जकम्_Has_Property(pE->p_वामः, EP_Int_मानम्) );
          zTName = pE->p_वामः->u.zToken;
          प्रतिज्ञा( व्यञ्जकम्_Use_WOfst(pE->p_वामः) );
          iErrOfst = pE->p_दक्षिणः->w.iOfst;
        }अन्यथा{
          प्रतिज्ञा( pE->p_वामः==0 );
          प्रतिज्ञा( व्यञ्जकम्_Use_WOfst(pE) );
          iErrOfst = pE->w.iOfst;
        }
        प्रति(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
          पूर्णाङ्क n_सङ्कलय;                    /* Number of cols including rowid */
          सारणी *pTab = pFrom->pSTab;  /* Table for this data source */
          व्यञ्जकम्_List *pNestedFrom;       /* Result-set of a nested FROM clause */
          वर्ण *zTabName;              /* AS name for this data source */
          ध्रुव वर्ण *zSchemaName = 0; /* Schema name for this data source */
          पूर्णाङ्क iDb;                     /* Schema index for this data src */
          IdList *pUsing;              /* USING clause for pFrom[1] */

          यदि( (zTabName = pFrom->zAlias)==0 ){
            zTabName = pTab->zName;
          }
          यदि( db->mallocFailed ) भङ्ग;
          प्रतिज्ञा( (पूर्णाङ्क)pFrom->fg.isNestedFrom == IsNestedFrom(pFrom) );
          यदि( pFrom->fg.isNestedFrom ){
            प्रतिज्ञा( pFrom->fg.isSubquery && pFrom->u4.pSubq );
            प्रतिज्ञा( pFrom->u4.pSubq->pSelect!=0 );
            pNestedFrom = pFrom->u4.pSubq->pSelect->pEList;
            प्रतिज्ञा( pNestedFrom!=0 );
            प्रतिज्ञा( pNestedFrom->n_व्यञ्जकम्==pTab->nCol );
            प्रतिज्ञा( VisibleRowid(pTab)==0 || ViewCanHaveRowid );
          }अन्यथा{
            यदि( zTName && sqlite3StrICmp(zTName, zTabName)!=0 ){
              अनुवर्त;
            }
            pNestedFrom = 0;
            iDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(db, pTab->pSchema);
            zSchemaName = iDb>=0 ? db->aDb[iDb].zDbSName : "*";
          }
          यदि( i+1<pTabList->nSrc
           && pFrom[1].fg.isUsing
           && (selFlags & SF_NestedFrom)!=0
          ){
            पूर्णाङ्क ii;
            pUsing = pFrom[1].u3.pUsing;
            प्रति(ii=0; ii<pUsing->nId; ii++){
              ध्रुव वर्ण *zUName = pUsing->a[ii].zName;
              p_दक्षिणः = sqlite3_व्यञ्जकम्(db, TK_ID, zUName);
              sqlite3_व्यञ्जकम्_समुच्चयः_Error_विस्थापनम्(p_दक्षिणः, iErrOfst);
              pNew = sqlite3_व्यञ्जकम्_List_Append(pParse, pNew, p_दक्षिणः);
              यदि( pNew ){
                संरचना व्यञ्जकम्_List_item *pX = &pNew->a[pNew->n_व्यञ्जकम्-1];
                प्रतिज्ञा( pX->zEName==0 );
                pX->zEName = sqlite3MPrintf(db,"..%s", zUName);
                pX->fg.eEName = ENAME_TAB;
                pX->fg.b_Using_पदांशः = 1;
              }
            }
          }अन्यथा{
            pUsing = 0;
          }

          n_सङ्कलय = pTab->nCol;
          यदि( VisibleRowid(pTab) && (selFlags & SF_NestedFrom)!=0 ) n_सङ्कलय++;
          प्रति(j=0; j<n_सङ्कलय; j++){
            ध्रुव वर्ण *zName; 
            संरचना व्यञ्जकम्_List_item *pX; /* Newly added ExprList term */

            यदि( j==pTab->nCol ){
              zName = sqlite3RowidAlias(pTab);
              यदि( zName==0 ) अनुवर्त;
            }अन्यथा{
              zName = pTab->aCol[j].zCnName;

              /* If pTab is actually an SF_NestedFrom sub-select, do not
              ** expand any ENAME_ROWID columns.  */
              यदि( pNestedFrom && pNestedFrom->a[j].fg.eEName==ENAME_ROWID ){
                अनुवर्त;
              }

              यदि( zTName
               && pNestedFrom
               && sqlite3_मेलनम्_EName(&pNestedFrom->a[j], 0, zTName, 0, 0)==0
              ){
                अनुवर्त;
              }

              /* If a column is marked as 'hidden', omit it from the expanded
              ** result-set list unless the SELECT has the SF_IncludeHidden
              ** bit set.
              */
              यदि( (p->selFlags & SF_IncludeHidden)==0
                && IsHiddenColumn(&pTab->aCol[j])
              ){
                अनुवर्त;
              }
              यदि( (pTab->aCol[j].colFlags & COLFLAG_NOEXPAND)!=0
               && zTName==0
               && (selFlags & (SF_NestedFrom))==0
              ){
                अनुवर्त;
              }
            }
            प्रतिज्ञा( zName );
            सारणी_आलोकितम् = 1;

            यदि( i>0 && zTName==0 && (selFlags & SF_NestedFrom)==0 ){
              यदि( pFrom->fg.isUsing
               && sqlite3_Id_List_सूचकांक(pFrom->u3.pUsing, zName)>=0
              ){
                /* In a join with a USING clause, omit columns in the
                ** using clause from the table on the right. */
                अनुवर्त;
              }
            }
            p_दक्षिणः = sqlite3_व्यञ्जकम्(db, TK_ID, zName);
            यदि( (pTabList->nSrc>1
                 && (  (pFrom->fg.jointype & JT_LTORJ)==0
                     || zTName!=0
                     || (selFlags & SF_NestedFrom)!=0
                     || !अन्तर्_Any_Using_Clause(zName,pFrom,pTabList->nSrc-i-1)
                    )
                )
             || अन्तर्_RENAME_OBJECT
            ){
              व्यञ्जकम् *p_वामः;
              p_वामः = sqlite3_व्यञ्जकम्(db, TK_ID, zTabName);
              p_व्यञ्जकम् = sqlite3PExpr(pParse, TK_DOT, p_वामः, p_दक्षिणः);
              यदि( अन्तर्_RENAME_OBJECT && pE->p_वामः ){
                प्रतिज्ञा( pE->op==TK_DOT && pE->p_दक्षिणः->op==TK_ASTERISK );
                sqlite3RenameTokenRemap(pParse, p_वामः, pE->p_वामः);
              }
              यदि( zSchemaName ){
                p_वामः = sqlite3_व्यञ्जकम्(db, TK_ID, zSchemaName);
                p_व्यञ्जकम् = sqlite3PExpr(pParse, TK_DOT, p_वामः, p_व्यञ्जकम्);
              }
            }अन्यथा{
              p_व्यञ्जकम् = p_दक्षिणः;
            }
            sqlite3_व्यञ्जकम्_समुच्चयः_Error_विस्थापनम्(p_व्यञ्जकम्, iErrOfst);
            pNew = sqlite3_व्यञ्जकम्_List_Append(pParse, pNew, p_व्यञ्जकम्);
            यदि( pNew==0 ){
              भङ्ग;  /* OOM */
            }
            pX = &pNew->a[pNew->n_व्यञ्जकम्-1];
            प्रतिज्ञा( pX->zEName==0 );
            यदि( (selFlags & SF_NestedFrom)!=0 && !अन्तर्_RENAME_OBJECT ){
              यदि( pNestedFrom && (!ViewCanHaveRowid || j<pNestedFrom->n_व्यञ्जकम्) ){
                प्रतिज्ञा( j<pNestedFrom->n_व्यञ्जकम् );
                pX->zEName = sqlite3DbStrDup(db, pNestedFrom->a[j].zEName);
                testcase( pX->zEName==0 );
              }अन्यथा{
                pX->zEName = sqlite3MPrintf(db, "%s.%s.%s",
                                           zSchemaName, zTabName, zName);
                testcase( pX->zEName==0 );
              }
              pX->fg.eEName = (j==pTab->nCol ? ENAME_ROWID : ENAME_TAB);
              यदि( (pFrom->fg.isUsing
                   && sqlite3_Id_List_सूचकांक(pFrom->u3.pUsing, zName)>=0)
               || (pUsing && sqlite3_Id_List_सूचकांक(pUsing, zName)>=0)
               || (j<pTab->nCol && (pTab->aCol[j].colFlags & COLFLAG_NOEXPAND))
              ){
                pX->fg.bNoExpand = 1;
              }
            }अन्यथा यदि( long_नामानि ){
              pX->zEName = sqlite3MPrintf(db, "%s.%s", zTabName, zName);
              pX->fg.eEName = ENAME_NAME;
            }अन्यथा{
              pX->zEName = sqlite3DbStrDup(db, zName);
              pX->fg.eEName = ENAME_NAME;
            }
          }
        }
        यदि( !सारणी_आलोकितम् ){
          यदि( zTName ){
            sqlite3_Error_सन्देशः(pParse, "no such table: %s", zTName);
          }अन्यथा{
            sqlite3_Error_सन्देशः(pParse, "no tables specified");
          }
        }
      }
    }
    sqlite3_व्यञ्जकम्_List_Delete(db, pEList);
    p->pEList = pNew;
  }
  यदि( p->pEList ){
    यदि( p->pEList->n_व्यञ्जकम्>db->a_सीमा[SQLITE_सीमा_COLUMN] ){
      sqlite3_Error_सन्देशः(pParse, "too many columns in result set");
      प्रत्यागम WRC_Abort;
    }
    यदि( (elistFlags & (EP_HasFunc|EP_Subquery))!=0 ){
      p->selFlags |= SF_Complex_परिणाम;
    }
  }
#यदि TREETRACE_ENABLED
  यदि( sqlite3_वृक्षः_Trace & 0x8 ){
    TREETRACE(0x8,pParse,p,("After result-set wildcard expansion:\n"));
    sqlite3_वृक्षः_View_Select(0, p, 0);
  }
#यदि_अन्तः
  प्रत्यागम WRC_Continue;
}

#यदि SQLITE_DEBUG
/*
** Always assert.  This xSelectCallback2 implementation proves that the
** xSelectCallback2 is never invoked.
*/
शून्य sqlite3SelectWalkAssert2(Walker *Not_प्रयुक्तम्, Select *NotUsed2){
  अप्रयुक्तम्_PARAMETER2(Not_प्रयुक्तम्, NotUsed2);
  प्रतिज्ञा( 0 );
}
#यदि_अन्तः

/*
** The first argument is a Walker object created by sqlite3SelectExpand().
** This function uses the walker to expand the SELECT statement passed
** as the second argument. If necessary, it also converts any compound
** SELECT statements that have COLLATE clauses on ORDER BY expressions
** to sub-queries. Return the return value of sqlite3WalkSelect().
*/
स्थिर पूर्णाङ्क selectExpanderWalk(Walker *pWalker, Select *pSelect){
  प्रतिज्ञा( pWalker->x_व्यञ्जकम्_Callback==sqlite3_व्यञ्जकम्_Walk_Noop );
  प्रतिज्ञा( pWalker->pParse!=0 );
  यदि( OK_IF_ALWAYS_TRUE(pWalker->pParse->hasCompound) ){
    pWalker->xSelectCallback = convert_Compound_Select_गन्तव्यम्_Subquery;
    pWalker->xSelectCallback2 = 0;
    sqlite3WalkSelect(pWalker, pSelect);
  }
  pWalker->xSelectCallback = selectExpander;
  pWalker->xSelectCallback2 = sqlite3_Select_निष्कासय_With;
  प्रत्यागम sqlite3WalkSelect(pWalker, pSelect);
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
स्थिर शून्य sqlite3SelectExpand(Parse *pParse, Select *pSelect){
  Walker w;
  w.x_व्यञ्जकम्_Callback = sqlite3_व्यञ्जकम्_Walk_Noop;
  w.pParse = pParse;
  w.e_संहिता = 0;
  selectExpanderWalk(&w, pSelect);
}


#यदि_अपरिभाषितम् SQLITE_OMIT_SUBQUERY
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
स्थिर शून्य select_सङ्कलय_Subquery_Type_Info(Walker *pWalker, Select *p){
  Parse *pParse;
  पूर्णाङ्क i;
  SrcList *pTabList;
  SrcItem *pFrom;

  यदि( p->selFlags & SF_HasTypeInfo ) प्रत्यागम;
  p->selFlags |= SF_HasTypeInfo;
  pParse = pWalker->pParse;
  प्रतिज्ञा( (p->selFlags & SF_Resolved) );
  pTabList = p->pSrc;
  प्रति(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    सारणी *pTab = pFrom->pSTab;
    प्रतिज्ञा( pTab!=0 );
    यदि( (pTab->tabFlags & TF_Ephemeral)!=0 && pFrom->fg.isSubquery ){
      /* A sub-query in the FROM clause of a SELECT */
      Select *pSel = pFrom->u4.pSubq->pSelect;
      sqlite3SubqueryColumnTypes(pParse, pTab, pSel, SQLITE_AFF_NONE);
    }
  }
}
#यदि_अन्तः


/*
** This routine adds datatype and collating sequence information to
** the Table structures of all FROM-clause subqueries in a
** SELECT statement.
**
** Use this routine after name resolution.
*/
स्थिर शून्य sqlite3_Select_सङ्कलय_Type_Info(Parse *pParse, Select *pSelect){
#यदि_अपरिभाषितम् SQLITE_OMIT_SUBQUERY
  Walker w;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.xSelectCallback2 = select_सङ्कलय_Subquery_Type_Info;
  w.x_व्यञ्जकम्_Callback = sqlite3_व्यञ्जकम्_Walk_Noop;
  w.pParse = pParse;
  sqlite3WalkSelect(&w, pSelect);
#यदि_अन्तः
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
शून्य sqlite3SelectPrep(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for container */
){
  प्रतिज्ञा( p!=0 || pParse->db->mallocFailed );
  प्रतिज्ञा( pParse->db->pParse==pParse );
  यदि( pParse->db->mallocFailed ) प्रत्यागम;
  यदि( p->selFlags & SF_HasTypeInfo ) प्रत्यागम;
  sqlite3SelectExpand(pParse, p);
  यदि( pParse->nErr ) प्रत्यागम;
  sqlite3_Resolve_Select_नामानि(pParse, p, pOuterNC);
  यदि( pParse->nErr ) प्रत्यागम;
  sqlite3_Select_सङ्कलय_Type_Info(pParse, p);
}

#यदि TREETRACE_ENABLED
/*
** Display all information about an AggInfo object
*/
स्थिर शून्य printAggInfo(AggInfo *pAggInfo){
  पूर्णाङ्क ii;
  sqlite3DebugPrintf("AggInfo %d/%p:\n",
     pAggInfo->selId, pAggInfo);
  प्रति(ii=0; ii<pAggInfo->nColumn; ii++){
    संरचना AggInfo_col *pCol = &pAggInfo->aCol[ii];
    sqlite3DebugPrintf(
       "agg-column[%d] pTab=%s iTable=%d iColumn=%d iMem=%d"
       " iSorterColumn=%d %s\n",
       ii, pCol->pTab ? pCol->pTab->zName : "NULL",
       pCol->i_सारणी, pCol->iColumn, pAggInfo->iFirstReg+ii,
       pCol->iSorterColumn,
       ii>=pAggInfo->n_सञ्चायकः ? "" : " Accumulator");
    sqlite3_वृक्षः_View_व्यञ्जकम्(0, pAggInfo->aCol[ii].pCExpr, 0);
  }
  प्रति(ii=0; ii<pAggInfo->nFunc; ii++){
    sqlite3DebugPrintf("agg-func[%d]: iMem=%d\n",
        ii, pAggInfo->iFirstReg+pAggInfo->nColumn+ii);
    sqlite3_वृक्षः_View_व्यञ्जकम्(0, pAggInfo->aFunc[ii].pFExpr, 0);
  }
}
#यदि_अन्तः /* TREETRACE_ENABLED */

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
स्थिर शून्य analyze_Agg_Func_तर्काः(
  AggInfo *pAggInfo,
  NameContext *pNC
){
  पूर्णाङ्क i;
  प्रतिज्ञा( pAggInfo!=0 );
  प्रतिज्ञा( pAggInfo->iFirstReg==0 );
  pNC->ncFlags |= NC_अन्तर्_Agg_Func;
  प्रति(i=0; i<pAggInfo->nFunc; i++){
    व्यञ्जकम् *p_व्यञ्जकम् = pAggInfo->aFunc[i].pFExpr;
    प्रतिज्ञा( p_व्यञ्जकम्->op==TK_FUNCTION || p_व्यञ्जकम्->op==TK_AGG_FUNCTION );
    प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
    sqlite3_व्यञ्जकम्_Analyze_Agg_List(pNC, p_व्यञ्जकम्->x.pList);
    यदि( p_व्यञ्जकम्->p_वामः ){
      प्रतिज्ञा( p_व्यञ्जकम्->p_वामः->op==TK_क्रमः );
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्->p_वामः) );
      sqlite3_व्यञ्जकम्_Analyze_Agg_List(pNC, p_व्यञ्जकम्->p_वामः->x.pList);
    }
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
    प्रतिज्ञा( !Is_गवाक्षः_Func(p_व्यञ्जकम्) );
    यदि( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_WinFunc) ){
      sqlite3_व्यञ्जकम्_Analyze_Aggregates(pNC, p_व्यञ्जकम्->y.pWin->pFilter);
    }
#यदि_अन्तः
  }
  pNC->ncFlags &= ~NC_अन्तर्_Agg_Func;
}

/*
** An index on expressions is being used in the inner loop of an
** aggregate query with a GROUP BY clause.  This routine attempts
** to adjust the AggInfo object to take advantage of index and to
** perhaps use the index as a covering index.
**
*/
स्थिर शून्य optimize_Aggregate_Use_सम्बन्धि_अनुक्रमितम्_व्यञ्जकम्(
  Parse *pParse,          /* Parsing context */
  Select *pSelect,        /* The SELECT statement being processed */
  AggInfo *pAggInfo,      /* The aggregate info */
  NameContext *pNC        /* Name context used to resolve agg-func args */
){
  प्रतिज्ञा( pAggInfo->iFirstReg==0 );
  प्रतिज्ञा( pSelect!=0 );
  प्रतिज्ञा( pSelect->p_गणः_By!=0 );
  pAggInfo->nColumn = pAggInfo->n_सञ्चायकः;
  यदि( ALWAYS(pAggInfo->n_क्रमणम्_Column>0) ){
    पूर्णाङ्क mx = pSelect->p_गणः_By->n_व्यञ्जकम् - 1;
    पूर्णाङ्क j, k;
    प्रति(j=0; j<pAggInfo->nColumn; j++){
      k = pAggInfo->aCol[j].iSorterColumn;
      यदि( k>mx ) mx = k;
    }
    pAggInfo->n_क्रमणम्_Column = mx+1;
  }
  analyze_Agg_Func_तर्काः(pAggInfo, pNC);
#यदि TREETRACE_ENABLED
  यदि( sqlite3_वृक्षः_Trace & 0x20 ){
    अनुक्रमितम्_व्यञ्जकम् *pIEpr;
    TREETRACE(0x20, pParse, pSelect,
        ("AggInfo (possibly) adjusted for Indexed Exprs\n"));
    sqlite3_वृक्षः_View_Select(0, pSelect, 0);
    प्रति(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
      प्रारूप_लिख("data-cursor=%d index={%d,%d}\n",
          pIEpr->i_दत्त_Cur, pIEpr->iIdxCur, pIEpr->iIdxCol);
      sqlite3_वृक्षः_View_व्यञ्जकम्(0, pIEpr->p_व्यञ्जकम्, 0);
    }
    printAggInfo(pAggInfo);
  }
#अन्यथा
  अप्रयुक्तम्_PARAMETER(pSelect);
  अप्रयुक्तम्_PARAMETER(pParse);
#यदि_अन्तः
}

/*
** Walker callback for aggregateConvertIndexedExprRefToColumn().
*/
स्थिर पूर्णाङ्क aggregate_Idx_Epr_Ref_गन्तव्यम्_Col_Callback(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  AggInfo *pAggInfo;
  संरचना AggInfo_col *pCol;
  अप्रयुक्तम्_PARAMETER(pWalker);
  यदि( p_व्यञ्जकम्->pAggInfo==0 ) प्रत्यागम WRC_Continue;
  यदि( p_व्यञ्जकम्->op==TK_AGG_COLUMN ) प्रत्यागम WRC_Continue;
  यदि( p_व्यञ्जकम्->op==TK_AGG_FUNCTION ) प्रत्यागम WRC_Continue;
  यदि( p_व्यञ्जकम्->op==TK_IF_NULL_ROW ) प्रत्यागम WRC_Continue;
  pAggInfo = p_व्यञ्जकम्->pAggInfo;
  यदि( NEVER(p_व्यञ्जकम्->iAgg>=pAggInfo->nColumn) ) प्रत्यागम WRC_Continue;
  प्रतिज्ञा( p_व्यञ्जकम्->iAgg>=0 );
  pCol = &pAggInfo->aCol[p_व्यञ्जकम्->iAgg];
  p_व्यञ्जकम्->op = TK_AGG_COLUMN;
  p_व्यञ्जकम्->i_सारणी = pCol->i_सारणी;
  p_व्यञ्जकम्->iColumn = pCol->iColumn;
  व्यञ्जकम्_Clear_Property(p_व्यञ्जकम्, EP_Skip|EP_Collate|EP_Unlikely);
  प्रत्यागम WRC_Prune;
}

/*
** Convert every pAggInfo->aFunc[].pExpr such that any node within
** those expressions that has pAppInfo set is changed into a TK_AGG_COLUMN
** opcode.
*/
स्थिर शून्य aggregate_Convert_अनुक्रमितम्_व्यञ्जकम्_Ref_गन्तव्यम्_Column(AggInfo *pAggInfo){
  पूर्णाङ्क i;
  Walker w;
  स्मृतिं_पूरय(&w, 0, परिमाण(w));
  w.x_व्यञ्जकम्_Callback = aggregate_Idx_Epr_Ref_गन्तव्यम्_Col_Callback;
  प्रति(i=0; i<pAggInfo->nFunc; i++){
    sqlite3_Walk_व्यञ्जकम्(&w, pAggInfo->aFunc[i].pFExpr);
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
स्थिर शून्य नियोजय_Aggregate_Registers(Parse *pParse, AggInfo *pAggInfo){
  प्रतिज्ञा( pAggInfo!=0 );
  प्रतिज्ञा( pAggInfo->iFirstReg==0 );
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
स्थिर शून्य reset_सञ्चायकः(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  पूर्णाङ्क i;
  संरचना AggInfo_func *pFunc;
  पूर्णाङ्क nReg = pAggInfo->nFunc + pAggInfo->nColumn;
  प्रतिज्ञा( pAggInfo->iFirstReg>0 );
  प्रतिज्ञा( pParse->db->pParse==pParse );
  प्रतिज्ञा( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  यदि( nReg==0 ) प्रत्यागम;
  यदि( pParse->nErr ) प्रत्यागम;
  sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Null, 0, pAggInfo->iFirstReg,
                    pAggInfo->iFirstReg+nReg-1);
  प्रति(pFunc=pAggInfo->aFunc, i=0; i<pAggInfo->nFunc; i++, pFunc++){
    यदि( pFunc->iDistinct>=0 ){
      व्यञ्जकम् *pE = pFunc->pFExpr;
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(pE) );
      यदि( pE->x.pList==0 || pE->x.pList->n_व्यञ्जकम्!=1 ){
        sqlite3_Error_सन्देशः(pParse, "DISTINCT aggregates must have exactly one "
           "argument");
        pFunc->iDistinct = -1;
      }अन्यथा{
        कुञ्चिका_Info *p_कुञ्चिका_Info = sqlite3_कुञ्चिका_Info_From_व्यञ्जकम्_List(pParse, pE->x.pList,0,0);
        pFunc->iDistAddr = sqlite3_Vdbe_सङ्कलय_Op4(v, OP_OpenEphemeral,
            pFunc->iDistinct, 0, 0, (वर्ण*)p_कुञ्चिका_Info, P4_KEYINFO);
        ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(DISTINCT)",
                          pFunc->pFunc->zName));
      }
    }
    यदि( pFunc->iOBTab>=0 ){
      व्यञ्जकम्_List *pOBList;
      कुञ्चिका_Info *p_कुञ्चिका_Info;
      पूर्णाङ्क nExtra = 0;
      प्रतिज्ञा( pFunc->pFExpr->p_वामः!=0 );
      प्रतिज्ञा( pFunc->pFExpr->p_वामः->op==TK_क्रमः );
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(pFunc->pFExpr->p_वामः) );
      प्रतिज्ञा( pFunc->pFunc!=0 );
      pOBList = pFunc->pFExpr->p_वामः->x.pList;
      यदि( !pFunc->bOBUnique ){
        nExtra++;  /* One extra column for the OP_Sequence */
      }
      यदि( pFunc->bOBPayload ){
        /* extra columns for the function arguments */
        प्रतिज्ञा( व्यञ्जकम्_Use_XList(pFunc->pFExpr) );
        प्रतिज्ञा( pFunc->pFExpr->x.pList!=0 );
        nExtra += pFunc->pFExpr->x.pList->n_व्यञ्जकम्;
      }
      यदि( pFunc->bUseSubtype ){
        nExtra += pFunc->pFExpr->x.pList->n_व्यञ्जकम्;
      }
      p_कुञ्चिका_Info = sqlite3_कुञ्चिका_Info_From_व्यञ्जकम्_List(pParse, pOBList, 0, nExtra);
      यदि( !pFunc->bOBUnique && pParse->nErr==0 ){
        p_कुञ्चिका_Info->n_कुञ्चिका_क्षेत्रम्++;
      }
      sqlite3_Vdbe_सङ्कलय_Op4(v, OP_OpenEphemeral,
            pFunc->iOBTab, pOBList->n_व्यञ्जकम्+nExtra, 0,
            (वर्ण*)p_कुञ्चिका_Info, P4_KEYINFO);
      ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(ORDER BY)",
                          pFunc->pFunc->zName));
    }
  }
}

/*
** Invoke the OP_AggFinalize opcode for every aggregate function
** in the AggInfo structure.
*/
स्थिर शून्य finalizeAggFunctions(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  पूर्णाङ्क i;
  संरचना AggInfo_func *pF;
  प्रति(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    व्यञ्जकम्_List *pList;
    प्रतिज्ञा( व्यञ्जकम्_Use_XList(pF->pFExpr) );
    यदि( pParse->nErr ) प्रत्यागम;
    pList = pF->pFExpr->x.pList;
    यदि( pF->iOBTab>=0 ){
      /* For an ORDER BY aggregate, calls to OP_AggStep were deferred.  Inputs
      ** were stored in emphermal table pF->iOBTab.  Here, we extract those
      ** inputs (in ORDER BY order) and make all calls to OP_AggStep
      ** before doing the OP_AggFinal call. */
      पूर्णाङ्क i_शिखरम्;        /* Start of loop for extracting columns */
      पूर्णाङ्क nArg;        /* Number of columns to extract */
      पूर्णाङ्क n_कुञ्चिका;        /* Key columns to be skipped */
      पूर्णाङ्क regAgg;      /* Extract into this array */
      पूर्णाङ्क j;           /* Loop counter */
     
      प्रतिज्ञा( pF->pFunc!=0 );
      nArg = pList->n_व्यञ्जकम्;
      regAgg = sqlite3_Get_अस्थायी_Range(pParse, nArg);

      यदि( pF->bOBPayload==0 ){
        n_कुञ्चिका = 0;
      }अन्यथा{
        प्रतिज्ञा( pF->pFExpr->p_वामः!=0 );
        प्रतिज्ञा( व्यञ्जकम्_Use_XList(pF->pFExpr->p_वामः) );
        प्रतिज्ञा( pF->pFExpr->p_वामः->x.pList!=0 );
        n_कुञ्चिका = pF->pFExpr->p_वामः->x.pList->n_व्यञ्जकम्;
        यदि( ALWAYS(!pF->bOBUnique) ) n_कुञ्चिका++;
      }
      i_शिखरम् = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Rewind, pF->iOBTab); VdbeCoverage(v);
      प्रति(j=nArg-1; j>=0; j--){
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, pF->iOBTab, n_कुञ्चिका+j, regAgg+j);
      }
      यदि( pF->bUseSubtype ){
        पूर्णाङ्क regSubtype = sqlite3_Get_अस्थायी_Reg(pParse);
        पूर्णाङ्क i_आधारः_Col = n_कुञ्चिका + nArg + (pF->bOBPayload==0 && pF->bOBUnique==0);
        प्रति(j=nArg-1; j>=0; j--){
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, pF->iOBTab, i_आधारः_Col+j, regSubtype);
          sqlite3_Vdbe_सङ्कलय_Op2(v, OP_समुच्चयः_Subtype, regSubtype, regAgg+j);
        }
        sqlite3_Release_अस्थायी_Reg(pParse, regSubtype);
      }
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Agg_चरणः, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_अग्र, pF->iOBTab, i_शिखरम्+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_अत्र(v, i_शिखरम्);
      sqlite3_Release_अस्थायी_Range(pParse, regAgg, nArg);
    }
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_AggFinal, AggInfoFuncReg(pAggInfo,i),
                      pList ? pList->n_व्यञ्जकम् : 0);
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
स्थिर शून्य update_सञ्चायकः(
  Parse *pParse,
  पूर्णाङ्क regAcc,
  AggInfo *pAggInfo,
  पूर्णाङ्क eDistinctType
){
  Vdbe *v = pParse->pVdbe;
  पूर्णाङ्क i;
  पूर्णाङ्क reg_लब्धम् = 0;
  पूर्णाङ्क addr_लब्धम्_Test = 0;
  संरचना AggInfo_func *pF;
  संरचना AggInfo_col *pC;

  प्रतिज्ञा( pAggInfo->iFirstReg>0 );
  यदि( pParse->nErr ) प्रत्यागम;
  pAggInfo->प्रत्यक्षम्_बहुलकम् = 1;
  प्रति(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    पूर्णाङ्क nArg;
    पूर्णाङ्क addr_अग्र = 0;
    पूर्णाङ्क regAgg;
    पूर्णाङ्क regAggSz = 0;
    पूर्णाङ्क regDistinct = 0;
    व्यञ्जकम्_List *pList;
    प्रतिज्ञा( व्यञ्जकम्_Use_XList(pF->pFExpr) );
    प्रतिज्ञा( !Is_गवाक्षः_Func(pF->pFExpr) );
    प्रतिज्ञा( pF->pFunc!=0 );
    pList = pF->pFExpr->x.pList;
    यदि( व्यञ्जकम्_Has_Property(pF->pFExpr, EP_WinFunc) ){
      व्यञ्जकम् *pFilter = pF->pFExpr->y.pWin->pFilter;
      यदि( pAggInfo->n_सञ्चायकः
       && (pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)
       && regAcc
      ){
        /* If regAcc==0, there there exists some min() or max() function
        ** without a FILTER clause that will ensure the magnet registers
        ** are populated. */
        यदि( reg_लब्धम्==0 ) reg_लब्धम् = ++pParse->nMem;
        /* If this is the first row of the group (regAcc contains 0), clear the
        ** "magnet" register regHit so that the accumulator registers
        ** are populated if the FILTER clause jumps over the the
        ** invocation of min() or max() altogether. Or, if this is not
        ** the first row (regAcc contains 1), set the magnet register so that
        ** the accumulators are not populated unless the min()/max() is invoked
        ** and indicates that they should be.  */
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Copy, regAcc, reg_लब्धम्);
      }
      addr_अग्र = sqlite3VdbeMakeLabel(pParse);
      sqlite3_व्यञ्जकम्_If_False(pParse, pFilter, addr_अग्र, SQLITE_JUMPIFNULL);
    }
    यदि( pF->iOBTab>=0 ){
      /* Instead of invoking AggStep, we must push the arguments that would
      ** have been passed to AggStep onto the sorting table. */
      पूर्णाङ्क jj;                /* Registered used so far in building the record */
      व्यञ्जकम्_List *pOBList;     /* The ORDER BY clause */
      प्रतिज्ञा( pList!=0 );
      nArg = pList->n_व्यञ्जकम्;
      प्रतिज्ञा( nArg>0 );
      प्रतिज्ञा( pF->pFExpr->p_वामः!=0 );
      प्रतिज्ञा( pF->pFExpr->p_वामः->op==TK_क्रमः );
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(pF->pFExpr->p_वामः) );
      pOBList = pF->pFExpr->p_वामः->x.pList;
      प्रतिज्ञा( pOBList!=0 );
      प्रतिज्ञा( pOBList->n_व्यञ्जकम्>0 );
      regAggSz = pOBList->n_व्यञ्जकम्;
      यदि( !pF->bOBUnique ){
        regAggSz++;   /* One register for OP_Sequence */
      }
      यदि( pF->bOBPayload ){
        regAggSz += nArg;
      }
      यदि( pF->bUseSubtype ){
        regAggSz += nArg;
      }
      regAggSz++;  /* One extra register to hold result of MakeRecord */
      regAgg = sqlite3_Get_अस्थायी_Range(pParse, regAggSz);
      regDistinct = regAgg;
      sqlite3_व्यञ्जकम्_संहिता_व्यञ्जकम्_List(pParse, pOBList, regAgg, 0, SQLITE_ECEL_DUP);
      jj = pOBList->n_व्यञ्जकम्;
      यदि( !pF->bOBUnique ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Sequence, pF->iOBTab, regAgg+jj);
        jj++;
      }
      यदि( pF->bOBPayload ){
        regDistinct = regAgg+jj;
        sqlite3_व्यञ्जकम्_संहिता_व्यञ्जकम्_List(pParse, pList, regDistinct, 0, SQLITE_ECEL_DUP);
        jj += nArg;
      }
      यदि( pF->bUseSubtype ){
        पूर्णाङ्क kk;
        पूर्णाङ्क reg_आधारः = pF->bOBPayload ? regDistinct : regAgg;
        प्रति(kk=0; kk<nArg; kk++, jj++){
          sqlite3_Vdbe_सङ्कलय_Op2(v, OP_GetSubtype, reg_आधारः+kk, regAgg+jj);
        }
      }
    }अन्यथा यदि( pList ){
      nArg = pList->n_व्यञ्जकम्;
      regAgg = sqlite3_Get_अस्थायी_Range(pParse, nArg);
      regDistinct = regAgg;
      sqlite3_व्यञ्जकम्_संहिता_व्यञ्जकम्_List(pParse, pList, regAgg, 0, SQLITE_ECEL_DUP);
    }अन्यथा{
      nArg = 0;
      regAgg = 0;
    }
    यदि( pF->iDistinct>=0 && pList ){
      यदि( addr_अग्र==0 ){
        addr_अग्र = sqlite3VdbeMakeLabel(pParse);
      }
      pF->iDistinct = संहिता_Distinct(pParse, eDistinctType,
          pF->iDistinct, addr_अग्र, pList, regDistinct);
    }
    यदि( pF->iOBTab>=0 ){
      /* Insert a new record into the ORDER BY table */
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, regAgg, regAggSz-1,
                        regAgg+regAggSz-1);
      sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_Idx_निवेशय, pF->iOBTab, regAgg+regAggSz-1,
                           regAgg, regAggSz-1);
      sqlite3_Release_अस्थायी_Range(pParse, regAgg, regAggSz);
    }अन्यथा{
      /* Invoke the AggStep function */
      यदि( pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        CollSeq *pColl = 0;
        संरचना व्यञ्जकम्_List_item *pItem;
        पूर्णाङ्क j;
        प्रतिज्ञा( pList!=0 );  /* pList!=0 if pF->pFunc has NEEDCOLL */
        प्रति(j=0, pItem=pList->a; !pColl && j<nArg; j++, pItem++){
          pColl = sqlite3_व्यञ्जकम्_Coll_Seq(pParse, pItem->p_व्यञ्जकम्);
        }
        यदि( !pColl ){
          pColl = pParse->db->pDfltColl;
        }
        यदि( reg_लब्धम्==0 && pAggInfo->n_सञ्चायकः ) reg_लब्धम् = ++pParse->nMem;
        sqlite3_Vdbe_सङ्कलय_Op4(v, OP_CollSeq, reg_लब्धम्, 0, 0,
                         (वर्ण *)pColl, P4_COLLSEQ);
      }
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Agg_चरणः, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3_Release_अस्थायी_Range(pParse, regAgg, nArg);
    }
    यदि( addr_अग्र ){
      sqlite3VdbeResolveLabel(v, addr_अग्र);
    }
    यदि( pParse->nErr ) प्रत्यागम;
  }
  यदि( reg_लब्धम्==0 && pAggInfo->n_सञ्चायकः ){
    reg_लब्धम् = regAcc;
  }
  यदि( reg_लब्धम् ){
    addr_लब्धम्_Test = sqlite3_Vdbe_सङ्कलय_Op1(v, OP_If, reg_लब्धम्); VdbeCoverage(v);
  }
  प्रति(i=0, pC=pAggInfo->aCol; i<pAggInfo->n_सञ्चायकः; i++, pC++){
    sqlite3_व्यञ्जकम्_संहिता(pParse, pC->pCExpr, AggInfoColumnReg(pAggInfo,i));
    यदि( pParse->nErr ) प्रत्यागम;
  }

  pAggInfo->प्रत्यक्षम्_बहुलकम् = 0;
  यदि( addr_लब्धम्_Test ){
    sqlite3_Vdbe_Jump_अत्र_Or_निष्कासय_Inst(v, addr_लब्धम्_Test);
  }
}

/*
** Add a single OP_Explain instruction to the VDBE to explain a simple
** count(*) query ("SELECT count(*) FROM pTab").
*/
#यदि_अपरिभाषितम् SQLITE_OMIT_EXPLAIN
स्थिर शून्य explain_Simple_गणनम्(
  Parse *pParse,                  /* Parse context */
  सारणी *pTab,                    /* Table being queried */
  सूचकांक *pIdx                     /* Index used to optimize scan, or NULL */
){
  यदि( pParse->explain==2 ){
    पूर्णाङ्क bCover = (pIdx!=0 && (HasRowid(pTab) || !Is_Primary_कुञ्चिका_सूचकांक(pIdx)));
    sqlite3VdbeExplain(pParse, 0, "SCAN %s%s%s",
        pTab->zName,
        bCover ? " USING COVERING INDEX " : "",
        bCover ? pIdx->zName : ""
    );
  }
}
#अन्यथा
# define explain_Simple_गणनम्(a,b,c)
#यदि_अन्तः

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
स्थिर पूर्णाङ्क having_गन्तव्यम्_Where_व्यञ्जकम्_Cb(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  यदि( p_व्यञ्जकम्->op!=TK_AND ){
    Select *pS = pWalker->u.pSelect;
    /* This routine is called before the HAVING clause of the current
    ** SELECT is analyzed for aggregates. So if pExpr->pAggInfo is set
    ** here, it indicates that the expression is a correlated reference to a
    ** column from an outer aggregate query, or an aggregate function that
    ** belongs to an outer query. Do not move the expression to the WHERE
    ** clause in this obscure case, as doing so may corrupt the outer Select
    ** statements AggInfo structure.  */
    यदि( sqlite3_व्यञ्जकम्_Is_Constant_Or_गणः_By(pWalker->pParse, p_व्यञ्जकम्, pS->p_गणः_By)
     && व्यञ्जकम्_Always_False(p_व्यञ्जकम्)==0
     && p_व्यञ्जकम्->pAggInfo==0
    ){
      sqlite3 *db = pWalker->pParse->db;
      व्यञ्जकम् *pNew = sqlite3_व्यञ्जकम्_Int32(db, 1);
      यदि( pNew ){
        व्यञ्जकम् *pWhere = pS->pWhere;
        SWAP(व्यञ्जकम्, *pNew, *p_व्यञ्जकम्);
        pNew = sqlite3_व्यञ्जकम्_And(pWalker->pParse, pWhere, pNew);
        pS->pWhere = pNew;
        pWalker->e_संहिता = 1;
      }
    }
    प्रत्यागम WRC_Prune;
  }
  प्रत्यागम WRC_Continue;
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
स्थिर शून्य having_गन्तव्यम्_Where(Parse *pParse, Select *p){
  Walker sWalker;
  स्मृतिं_पूरय(&sWalker, 0, परिमाण(sWalker));
  sWalker.pParse = pParse;
  sWalker.x_व्यञ्जकम्_Callback = having_गन्तव्यम्_Where_व्यञ्जकम्_Cb;
  sWalker.u.pSelect = p;
  sqlite3_Walk_व्यञ्जकम्(&sWalker, p->pHaving);
#यदि TREETRACE_ENABLED
  यदि( sWalker.e_संहिता && (sqlite3_वृक्षः_Trace & 0x100)!=0 ){
    TREETRACE(0x100,pParse,p,("Move HAVING terms into WHERE:\n"));
    sqlite3_वृक्षः_View_Select(0, p, 0);
  }
#यदि_अन्तः
}

/*
** Check to see if the pThis entry of pTabList is a self-join of another view.
** Search FROM-clause entries in the range of iFirst..iEnd, including iFirst
** but stopping before iEnd.
**
** If pThis is a self-join, then return the SrcItem for the first other
** instance of that view found.  If pThis is not a self-join then return 0.
*/
स्थिर SrcItem *isSelfJoinView(
  SrcList *pTabList,           /* Search for self-joins in this FROM clause */
  SrcItem *pThis,              /* Search for prior reference to this subquery */
  पूर्णाङ्क iFirst, पूर्णाङ्क i_अन्तः        /* Range of FROM-clause entries to search. */
){
  SrcItem *pItem;
  Select *pSel;
  प्रतिज्ञा( pThis->fg.isSubquery );
  pSel = pThis->u4.pSubq->pSelect;
  प्रतिज्ञा( pSel!=0 );
  यदि( pSel->selFlags & SF_प्रक्षिप_अधस्तात् ) प्रत्यागम 0;
  यावत्( iFirst<i_अन्तः ){
    Select *pS1;
    pItem = &pTabList->a[iFirst++];
    यदि( !pItem->fg.isSubquery ) अनुवर्त;
    यदि( pItem->fg.द्वारा_Coroutine ) अनुवर्त;
    यदि( pItem->zName==0 ) अनुवर्त;
    प्रतिज्ञा( pItem->pSTab!=0 );
    प्रतिज्ञा( pThis->pSTab!=0 );
    यदि( pItem->pSTab->pSchema!=pThis->pSTab->pSchema ) अनुवर्त;
    यदि( sqlite3_stricmp(pItem->zName, pThis->zName)!=0 ) अनुवर्त;
    pS1 = pItem->u4.pSubq->pSelect;
    यदि( pItem->pSTab->pSchema==0 && pSel->selId!=pS1->selId ){
      /* The query flattener left two different CTE tables with identical
      ** names in the same FROM clause. */
      अनुवर्त;
    }
    यदि( pS1->selFlags & SF_प्रक्षिप_अधस्तात् ){
      /* The view was modified by some other optimization such as
      ** pushDownWhereTerms() */
      अनुवर्त;
    }
    प्रत्यागम pItem;
  }
  प्रत्यागम 0;
}

/*
** Deallocate a single AggInfo object
*/
स्थिर शून्य agginfoFree(sqlite3 *db, शून्य *pArg){
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
स्थिर पूर्णाङ्क गणनम्_सम्बन्धि_View_Optimization(Parse *pParse, Select *p){
  Select *pSub, *p_प्राक्;
  व्यञ्जकम् *p_व्यञ्जकम्;
  व्यञ्जकम् *p_गणनम्;
  sqlite3 *db;
  SrcItem *pFrom;
  यदि( (p->selFlags & SF_Aggregate)==0 ) प्रत्यागम 0;   /* This is an aggregate */
  यदि( p->pEList->n_व्यञ्जकम्!=1 ) प्रत्यागम 0;               /* Single result column */
  यदि( p->pWhere ) प्रत्यागम 0;
  यदि( p->pHaving ) प्रत्यागम 0;
  यदि( p->p_गणः_By ) प्रत्यागम 0;
  यदि( p->p_क्रमः_By ) प्रत्यागम 0;
  p_व्यञ्जकम् = p->pEList->a[0].p_व्यञ्जकम्;
  यदि( p_व्यञ्जकम्->op!=TK_AGG_FUNCTION ) प्रत्यागम 0;        /* Result is an aggregate */
  प्रतिज्ञा( व्यञ्जकम्_Use_UToken(p_व्यञ्जकम्) );
  यदि( sqlite3_stricmp(p_व्यञ्जकम्->u.zToken,"count") ) प्रत्यागम 0;  /* Is count() */
  प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
  यदि( p_व्यञ्जकम्->x.pList!=0 ) प्रत्यागम 0;                 /* Must be count(*) */
  यदि( p->pSrc->nSrc!=1 ) प्रत्यागम 0;                  /* One table in FROM  */
  यदि( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_WinFunc) ) प्रत्यागम 0;/* Not a window function */
  pFrom = p->pSrc->a;
  यदि( pFrom->fg.isSubquery==0 ) प्रत्यागम 0;           /* FROM is a subquery */
  यदि( (p->selFlags & SF_Correlated)!=0 ) प्रत्यागम 0;  /* Not a correlated subq */
  pSub = pFrom->u4.pSubq->pSelect;
  यदि( pSub->p_प्राक्==0 ) प्रत्यागम 0;                   /* Must be a compound */
  यदि( pSub->selFlags & SF_CopyCte ) प्रत्यागम 0;       /* Not a CTE */
  कुरु{
    यदि( pSub->op!=TK_ALL && pSub->p_प्राक् ) प्रत्यागम 0;  /* Must be UNION ALL */
    यदि( pSub->pWhere ) प्रत्यागम 0;                      /* No WHERE clause */
    यदि( pSub->p_सीमा ) प्रत्यागम 0;                      /* No LIMIT clause */
    यदि( pSub->selFlags & (SF_Aggregate|SF_Distinct) ){
       testcase( pSub->selFlags & SF_Aggregate );
       testcase( pSub->selFlags & SF_Distinct );
       प्रत्यागम 0;     /* Not an aggregate nor DISTINCT */
    }
    प्रतिज्ञा( pSub->pHaving==0 );  /* Due to the previous */
    pSub = pSub->p_प्राक्;                              /* Repeat over compound */
  }यावत्( pSub );

  /* If we reach this point then it is OK to perform the transformation */

  db = pParse->db;
  p_गणनम् = p_व्यञ्जकम्;
  p_व्यञ्जकम् = 0;
  pSub = sqlite3SubqueryDetach(db, pFrom);
  sqlite3SrcListDelete(db, p->pSrc);
  p->pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  यावत्( pSub ){
    व्यञ्जकम् *p_पदांशः;
    p_प्राक् = pSub->p_प्राक्;
    pSub->p_प्राक् = 0;
    pSub->p_अग्र = 0;
    pSub->selFlags |= SF_Aggregate;
    pSub->selFlags &= ~(u32)SF_Compound;
    pSub->nSelectRow = 0;
    sqlite3_Parser_सङ्कलय_Cleanup(pParse, sqlite3_व्यञ्जकम्_List_Delete_Generic, pSub->pEList);
    p_पदांशः = p_प्राक् ? sqlite3_व्यञ्जकम्_Dup(db, p_गणनम्, 0) : p_गणनम्;
    pSub->pEList = sqlite3_व्यञ्जकम्_List_Append(pParse, 0, p_पदांशः);
    p_पदांशः = sqlite3PExpr(pParse, TK_SELECT, 0, 0);
    sqlite3_PExpr_सङ्कलय_Select(pParse, p_पदांशः, pSub);
    यदि( p_व्यञ्जकम्==0 ){
      p_व्यञ्जकम् = p_पदांशः;
    }अन्यथा{
      p_व्यञ्जकम् = sqlite3PExpr(pParse, TK_PLUS, p_पदांशः, p_व्यञ्जकम्);
    }
    यदि( pParse->nErr ){
      sqlite3_व्यञ्जकम्_Delete(db, p_व्यञ्जकम्);
      p_व्यञ्जकम् = 0;
    }
    pSub = p_प्राक्;
  }
  p->pEList->a[0].p_व्यञ्जकम् = p_व्यञ्जकम्;
  p->selFlags &= ~(u32)SF_Aggregate;

#यदि TREETRACE_ENABLED
  यदि( sqlite3_वृक्षः_Trace & 0x200 ){
    TREETRACE(0x200,pParse,p,("After count-of-view optimization:\n"));
    sqlite3_वृक्षः_View_Select(0, p, 0);
  }
#यदि_अन्तः
  प्रत्यागम 1;
}

/*
** If any term of pSrc, or any SF_NestedFrom sub-query, is not the same
** as pSrcItem but has the same alias as p0, then return true.
** Otherwise return false.
*/
स्थिर पूर्णाङ्क sameSrcAlias(SrcItem *p0, SrcList *pSrc){
  पूर्णाङ्क i;
  प्रति(i=0; i<pSrc->nSrc; i++){
    SrcItem *p1 = &pSrc->a[i];
    यदि( p1==p0 ) अनुवर्त;
    यदि( p0->pSTab==p1->pSTab && 0==sqlite3_stricmp(p0->zAlias, p1->zAlias) ){
      प्रत्यागम 1;
    }
    यदि( p1->fg.isSubquery
     && (p1->u4.pSubq->pSelect->selFlags & SF_NestedFrom)!=0
     && sameSrcAlias(p0, p1->u4.pSubq->pSelect->pSrc)
    ){
      प्रत्यागम 1;
    }
  }
  प्रत्यागम 0;
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
स्थिर पूर्णाङ्क from_Clause_पदांशः_Can_Be_Coroutine(
  Parse *pParse,          /* Parsing context */
  SrcList *pTabList,      /* FROM clause */
  पूर्णाङ्क i,                  /* Which term of the FROM clause holds the subquery */
  पूर्णाङ्क selFlags            /* Flags on the SELECT statement */
){
  SrcItem *pItem = &pTabList->a[i];
  यदि( pItem->fg.isCte ){
    ध्रुव CteUse *pCteUse = pItem->u2.pCteUse;
    यदि( pCteUse->eM10d==M10d_Yes ) प्रत्यागम 0;                          /* (2a) */
    यदि( pCteUse->nUse>=2 && pCteUse->eM10d!=M10d_No ) प्रत्यागम 0;       /* (2b) */
  }
  यदि( pTabList->a[0].fg.jointype & JT_LTORJ ) प्रत्यागम 0;               /* (3)  */
  यदि( OptimizationDisabled(pParse->db, SQLITE_Coroutines) ) प्रत्यागम 0; /* (4)  */
  यदि( isSelfJoinView(pTabList, pItem, i+1, pTabList->nSrc)!=0 ){
    प्रत्यागम 0;                                                          /* (5) */
  }
  यदि( i==0 ){
    यदि( pTabList->nSrc==1 ) प्रत्यागम 1;                             /* (1a) */
    यदि( pTabList->a[1].fg.jointype & JT_CROSS ) प्रत्यागम 1;         /* (1b) */
    यदि( selFlags & SF_UpdateFrom )              प्रत्यागम 0;         /* (1c-iii) */
    प्रत्यागम 1;
  }
  यदि( selFlags & SF_UpdateFrom ) प्रत्यागम 0;                        /* (1c-iii) */
  यावत्( 1 /*exit-by-break*/ ){
    यदि( pItem->fg.jointype & (JT_OUTER|JT_CROSS)  ) प्रत्यागम 0;     /* (1c-ii) */
    यदि( i==0 ) भङ्ग;
    i--;
    pItem--;
    यदि( pItem->fg.isSubquery ) प्रत्यागम 0;                          /* (1c-i) */
  }
  प्रत्यागम 1;
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
स्थिर SQLITE_NOINLINE शून्य exists_गन्तव्यम्_Join(
  Parse *pParse,  /* Parsing context */
  Select *p,      /* The SELECT statement being optimized */
  व्यञ्जकम् *pWhere    /* part of the WHERE clause currently being examined */
){
  यदि( pParse->nErr==0
   && pWhere!=0
   && !व्यञ्जकम्_Has_Property(pWhere, EP_OuterON|EP_InnerON) 
   && ALWAYS(p->pSrc!=0)
   && p->pSrc->nSrc<BMS
   && (p->p_सीमा==0 || p->p_सीमा->p_दक्षिणः==0)
  ){
    यदि( pWhere->op==TK_AND ){
      व्यञ्जकम् *p_दक्षिणः = pWhere->p_दक्षिणः;
      exists_गन्तव्यम्_Join(pParse, p, pWhere->p_वामः);
      exists_गन्तव्यम्_Join(pParse, p, p_दक्षिणः);
    }
    अन्यथा यदि( pWhere->op==TK_EXISTS ){
      Select *pSub = pWhere->x.pSelect;
      व्यञ्जकम् *pSubWhere = pSub->pWhere;
      यदि( pSub->pSrc->nSrc==1
       && (pSub->selFlags & SF_Aggregate)==0
       && !pSub->pSrc->a[0].fg.isSubquery
       && pSub->p_सीमा==0
       && pSub->p_प्राक्==0
      ){
        /* Before combining the sub-select with the parent, renumber the 
        ** cursor used by the subselect. This is because the EXISTS expression
        ** might be a copy of another EXISTS expression from somewhere
        ** else in the tree, and in this case it is important that it use
        ** a unique cursor number.  */
        sqlite3 *db = pParse->db;
        पूर्णाङ्क *aCsrMap = sqlite3DbMallocZero(db, (pParse->nTab+2)*परिमाण(पूर्णाङ्क));
        यदि( aCsrMap==0 ) प्रत्यागम;
        aCsrMap[0] = (pParse->nTab+1);
        renumberCursors(pParse, pSub, -1, aCsrMap);
        sqlite3DbFree(db, aCsrMap);

        स्मृतिं_पूरय(pWhere, 0, परिमाण(*pWhere));
        pWhere->op = TK_INTEGER;
        pWhere->u.i_मानम् = 1;
        व्यञ्जकम्_समुच्चयः_Property(pWhere, EP_Int_मानम्);
        प्रतिज्ञा( p->pWhere!=0 );
        pSub->pSrc->a[0].fg.fromExists = 1;
        p->pSrc = sqlite3SrcListAppendList(pParse, p->pSrc, pSub->pSrc);
        यदि( pSubWhere ){
          p->pWhere = sqlite3PExpr(pParse, TK_AND, p->pWhere, pSubWhere);
          pSub->pWhere = 0;
        }
        pSub->pSrc = 0;
        sqlite3_Parser_सङ्कलय_Cleanup(pParse, sqlite3SelectDeleteGeneric, pSub);
        recompute_Columns_प्रयुक्तम्(p, &p->pSrc->a[p->pSrc->nSrc-1]);
#यदि TREETRACE_ENABLED
        यदि( sqlite3_वृक्षः_Trace & 0x100000 ){
          TREETRACE(0x100000,pParse,p,
                    ("After EXISTS-to-JOIN optimization:\n"));
          sqlite3_वृक्षः_View_Select(0, p, 0);
        }
#यदि_अन्तः
      }
    }
  }
}

/*
** Type used for Walker callbacks by selectCheckOnClauses().
*/
प्रकारनिर्देश संरचना CheckOnCtx CheckOnCtx;
संरचना CheckOnCtx {
  SrcList *pSrc;       /* SrcList for this context */
  पूर्णाङ्क iJoin;           /* Cursors must be left of this one, if not negative */
  पूर्णाङ्क bFuncArg;        /* True for table-function arg */
  CheckOnCtx *pParent; /* Parent context */
};

/*
** True if the SrcList passed as the only argument contains at least
** one RIGHT or FULL JOIN. False otherwise.  
*/
#परिभाषय has_दक्षिणः_Join(pSrc) (((pSrc)->a[0].fg.jointype & JT_LTORJ)!=0)

/*
** The xExpr callback for the search of invalid ON clause terms.
*/
स्थिर पूर्णाङ्क select_Check_On_Clauses_व्यञ्जकम्(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;

  /* Check if pExpr is root or near-root of an ON clause constraint that needs
  ** to be checked to ensure that it does not refer to tables in its FROM
  ** clause to the right of itself. i.e. it is either:
  **
  **   + an ON clause on an OUTER join, or
  **   + an ON clause on an INNER join within a FROM that features at
  **     least one RIGHT or FULL join.
  */
  यदि( (व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON))
   || (व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_InnerON) && has_दक्षिणः_Join(pCtx->pSrc))
  ){
    /* If CheckOnCtx.iJoin is already set, then fall through and process
    ** this expression node as normal. Or, if CheckOnCtx.iJoin is still 0,
    ** set it to the cursor number of the RHS of the join to which this
    ** ON expression was attached and then iterate through the entire 
    ** expression.  */
    प्रतिज्ञा( pCtx->iJoin<0 || pCtx->iJoin==p_व्यञ्जकम्->w.iJoin );
    यदि( pCtx->iJoin<0 ){
      pCtx->iJoin = p_व्यञ्जकम्->w.iJoin;
      sqlite3_Walk_व्यञ्जकम्_NN(pWalker, p_व्यञ्जकम्);
      pCtx->iJoin = -1;
      प्रत्यागम WRC_Prune;
    }
  }

  यदि( p_व्यञ्जकम्->op==TK_COLUMN ){
    /* A column expression. Find the SrcList (if any) to which it refers.
    ** Then, if CheckOnCtx.iJoin indicates that this expression is part of an
    ** ON clause from that SrcList (i.e. if iJoin is non-zero), check that it
    ** does not refer to a table to the right of CheckOnCtx.iJoin. */
    पूर्णाङ्क iTab = p_व्यञ्जकम्->i_सारणी;
    कुरु {
      SrcList *pSrc = pCtx->pSrc;
      पूर्णाङ्क nSrc = pSrc->nSrc;
      पूर्णाङ्क ii;
      प्रति(ii=0; ii<nSrc && pSrc->a[ii].iCursor!=iTab; ii++){}
      यदि( ii<nSrc ){
        /* pSrc is the FROM clause that contains iTab */
        यदि( pCtx->iJoin>=0 ){
          प्रति(ii--; ii>=0 && pSrc->a[ii].iCursor!=pCtx->iJoin; ii--){}
          यदि( ii>=0 ){
            /* Table iJoin appears to the left of table iTab in the SrcList.
            ** Therefore the expression refers to a table to its right. */
            sqlite3_Error_सन्देशः(pWalker->pParse, 
                "%s references tables to its right",
                (pCtx->bFuncArg ? "table-function argument" : "ON clause")
            );
            प्रत्यागम WRC_Abort;
          }
        }
        भङ्ग;
      }
      pCtx = pCtx->pParent;
    }यावत्( pCtx );
  }
  प्रत्यागम WRC_Continue;
}

/*
** The xSelect callback for the search of invalid ON clause terms.
*/
स्थिर पूर्णाङ्क selectCheckOnClausesSelect(Walker *pWalker, Select *pSelect){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;
  यदि( pSelect->pSrc==pCtx->pSrc || pSelect->pSrc->nSrc==0 ){
    प्रत्यागम WRC_Continue;
  }अन्यथा{
    CheckOnCtx sCtx;
    स्मृतिं_पूरय(&sCtx, 0, परिमाण(sCtx));
    sCtx.pSrc = pSelect->pSrc;
    sCtx.pParent = pCtx;
    sCtx.iJoin = -1;
    pWalker->u.pCheckOnCtx = &sCtx;
    sqlite3WalkSelect(pWalker, pSelect);
    pWalker->u.pCheckOnCtx = pCtx;
    pSelect->selFlags &= ~SF_On_गन्तव्यम्_Where;
    प्रत्यागम WRC_Prune;
  }
}

/*
** Check all ON clauses in pSelect to verify that they do not reference
** columns to the right.
*/
शून्य sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect){
  Walker w;
  CheckOnCtx sCtx;
  पूर्णाङ्क ii;
  प्रतिज्ञा( pSelect->selFlags & SF_On_गन्तव्यम्_Where );
  प्रतिज्ञा( pSelect->pSrc!=0 && pSelect->pSrc->nSrc>=2 );
  स्मृतिं_पूरय(&w, 0, परिमाण(w));
  w.pParse = pParse;
  w.x_व्यञ्जकम्_Callback = select_Check_On_Clauses_व्यञ्जकम्;
  w.xSelectCallback = selectCheckOnClausesSelect;
  w.u.pCheckOnCtx = &sCtx;
  स्मृतिं_पूरय(&sCtx, 0, परिमाण(sCtx));
  sCtx.pSrc = pSelect->pSrc;
  sCtx.iJoin = -1;
  sqlite3_Walk_व्यञ्जकम्(&w, pSelect->pWhere);
  pSelect->selFlags &= ~SF_On_गन्तव्यम्_Where;

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
  प्रति(ii=0; ii<pSelect->pSrc->nSrc; ii++){
    SrcItem *pItem = &pSelect->pSrc->a[ii];
    यदि( pItem->fg.isTabFunc ){
      sCtx.iJoin = -1;
      यदि( (pItem->fg.jointype & (JT_LTORJ|JT_दक्षिणः)) ){
        sCtx.iJoin = pItem->iCursor;
      }अन्यथा{
        पूर्णाङ्क jj;
        प्रति(jj=ii+1; jj<pSelect->pSrc->nSrc; jj++){
          SrcItem *pItem2 = &pSelect->pSrc->a[jj];
          यदि( pItem2->fg.jointype & JT_OUTER ){
            sCtx.iJoin = pItem2[-1].iCursor;
            भङ्ग;
          }
        }
      }
      यदि( sCtx.iJoin>=0 ){
        sqlite3_Walk_व्यञ्जकम्_List(&w, pItem->u1.pFuncArg);
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
स्थिर पूर्णाङ्क sqlite3_Copy_क्रमय_क्रमः(व्यञ्जकम्_List *p1, व्यञ्जकम्_List *p2){
  प्रतिज्ञा( p1 );
  यदि( p2 && p1->n_व्यञ्जकम्==p2->n_व्यञ्जकम् ){
    पूर्णाङ्क ii;
    प्रति(ii=0; ii<p1->n_व्यञ्जकम्; ii++){
      u8 क्रमय_Flags;
      क्रमय_Flags = p2->a[ii].fg.क्रमय_Flags & KEYINFO_क्रमः_DESC;
      p1->a[ii].fg.क्रमय_Flags = क्रमय_Flags;
    }
    प्रत्यागम 1;
  }अन्यथा{
    प्रत्यागम 0;
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
पूर्णाङ्क sqlite3Select(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  SelectDest *pDest      /* What to do with the query results */
){
  पूर्णाङ्क i, j;              /* Loop counters */
  WhereInfo *pWInfo;     /* Return from sqlite3WhereBegin() */
  Vdbe *v;               /* The virtual machine under construction */
  पूर्णाङ्क isAgg;             /* True for select lists like "count(*)" */
  व्यञ्जकम्_List *pEList = 0;  /* List of columns to extract. */
  SrcList *pTabList;     /* List of tables to select from */
  व्यञ्जकम् *pWhere;          /* The WHERE clause.  May be NULL */
  व्यञ्जकम्_List *p_गणः_By;    /* The GROUP BY clause.  May be NULL */
  व्यञ्जकम् *pHaving;         /* The HAVING clause.  May be NULL */
  AggInfo *pAggInfo = 0; /* Aggregate information */
  पूर्णाङ्क rc = 1;            /* Value to return from this function */
  DistinctCtx sDistinct; /* Info on how to code the DISTINCT keyword */
  क्रमय_Ctx s_क्रमय;         /* Info on how to code the ORDER BY clause */
  पूर्णाङ्क i_अन्तः;              /* Address of the end of the query */
  sqlite3 *db;           /* The database connection */
  व्यञ्जकम्_List *p_Min_Max_क्रमः_By = 0;  /* Added ORDER BY for min/max queries */
  u8 minMaxFlag;                 /* Flag for min/max queries */

  db = pParse->db;
  प्रतिज्ञा( pParse==db->pParse );
  v = sqlite3GetVdbe(pParse);
  यदि( p==0 || pParse->nErr ){
    प्रत्यागम 1;
  }
  प्रतिज्ञा( db->mallocFailed==0 );
  यदि( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) प्रत्यागम 1;
#यदि TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p, ("begin processing:\n", pParse->addrExplain));
  यदि( sqlite3_वृक्षः_Trace & 0x10000 ){
    यदि( (sqlite3_वृक्षः_Trace & 0x10001)==0x10000 ){
      sqlite3_वृक्षः_View_पङ्क्तिः(0, "In sqlite3Select() at %s:%d",
                           __FILE__, __LINE__);
    }
    sqlite3ShowSelect(p);
  }
#यदि_अन्तः

  /* tag-select-0100 */
  प्रतिज्ञा( p->p_क्रमः_By==0 || pDest->eDest!=SRT_DistFifo );
  प्रतिज्ञा( p->p_क्रमः_By==0 || pDest->eDest!=SRT_Fifo );
  प्रतिज्ञा( p->p_क्रमः_By==0 || pDest->eDest!=SRT_DistQueue );
  प्रतिज्ञा( p->p_क्रमः_By==0 || pDest->eDest!=SRT_Queue );
  यदि( IgnorableDistinct(pDest) ){
    प्रतिज्ञा(pDest->eDest==SRT_Exists     || pDest->eDest==SRT_Discard ||
           pDest->eDest==SRT_DistQueue  || pDest->eDest==SRT_DistFifo );
    /* All of these destinations are also able to ignore the ORDER BY clause */
    यदि( p->p_क्रमः_By ){
#यदि TREETRACE_ENABLED
      TREETRACE(0x800,pParse,p, ("dropping superfluous ORDER BY:\n"));
      यदि( sqlite3_वृक्षः_Trace & 0x800 ){
        sqlite3_वृक्षः_View_व्यञ्जकम्_List(0, p->p_क्रमः_By, 0, "ORDERBY");
      }
#यदि_अन्तः   
      sqlite3_Parser_सङ्कलय_Cleanup(pParse, sqlite3_व्यञ्जकम्_List_Delete_Generic,
                              p->p_क्रमः_By);
      testcase( pParse->earlyCleanup );
      p->p_क्रमः_By = 0;
    }
    p->selFlags &= ~(u32)SF_Distinct;
  }
  sqlite3SelectPrep(pParse, p, 0);
  यदि( pParse->nErr ){
    गच्छ select_अन्तः;
  }
  प्रतिज्ञा( db->mallocFailed==0 );
  प्रतिज्ञा( p->pEList!=0 );
#यदि TREETRACE_ENABLED
  यदि( sqlite3_वृक्षः_Trace & 0x10 ){
    TREETRACE(0x10,pParse,p, ("after name resolution:\n"));
    sqlite3_वृक्षः_View_Select(0, p, 0);
  }
#यदि_अन्तः

  /* If the SF_UFSrcCheck flag is set, then this function is being called
  ** as part of populating the temp table for an UPDATE...FROM statement.
  ** In this case, it is an error if the target object (pSrc->a[0]) name
  ** or alias is duplicated within FROM clause (pSrc->a[1..n]). 
  **
  ** Postgres disallows this case too. The reason is that some other
  ** systems handle this case differently, and not all the same way,
  ** which is just confusing. To avoid this, we follow PG's lead and
  ** disallow it altogether.  */
  यदि( p->selFlags & SF_UFSrcCheck ){
    SrcItem *p0 = &p->pSrc->a[0];
    यदि( sameSrcAlias(p0, p->pSrc) ){
      sqlite3_Error_सन्देशः(pParse,
          "target object/alias may not appear in FROM clause: %s",
          p0->zAlias ? p0->zAlias : p0->pSTab->zName
      );
      गच्छ select_अन्तः;
    }

    /* Clear the SF_UFSrcCheck flag. The check has already been performed,
    ** and leaving this flag set can cause errors if a compound sub-query
    ** in p->pSrc is flattened into this query and this function called
    ** again as part of compound SELECT processing.  */
    p->selFlags &= ~(u32)SF_UFSrcCheck;
  }

  यदि( pDest->eDest==SRT_Output ){
    sqlite3_Generate_Column_नामानि(pParse, p);
  }

#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
  यदि( sqlite3_गवाक्षः_Rewrite(pParse, p) ){
    प्रतिज्ञा( pParse->nErr );
    गच्छ select_अन्तः;
  }
#यदि TREETRACE_ENABLED
  यदि( p->pWin && (sqlite3_वृक्षः_Trace & 0x40)!=0 ){
    TREETRACE(0x40,pParse,p, ("after window rewrite:\n"));
    sqlite3_वृक्षः_View_Select(0, p, 0);
  }
#यदि_अन्तः
#यदि_अन्तः /* SQLITE_OMIT_WINDOWFUNC */
  pTabList = p->pSrc;
  isAgg = (p->selFlags & SF_Aggregate)!=0;
  स्मृतिं_पूरय(&s_क्रमय, 0, परिमाण(s_क्रमय));
  s_क्रमय.p_क्रमः_By = p->p_क्रमः_By;

  /* Try to do various optimizations (flattening subqueries, and strength
  ** reduction of join operators) in the FROM clause up into the main query
  ** tag-select-0200
  */
#यदि !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
  प्रति(i=0; !p->p_प्राक् && i<pTabList->nSrc; i++){
    SrcItem *pItem = &pTabList->a[i];
    Select *pSub = pItem->fg.isSubquery ? pItem->u4.pSubq->pSelect : 0;
    सारणी *pTab = pItem->pSTab;

    /* The expander should have already created transient Table objects
    ** even for FROM clause elements such as subqueries that do not correspond
    ** to a real table */
    प्रतिज्ञा( pTab!=0 );

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
    यदि( (pItem->fg.jointype & (JT_वामः|JT_LTORJ))!=0
     && sqlite3_व्यञ्जकम्_उपपादयति_Non_Null_Row(p->pWhere, pItem->iCursor,
                                     pItem->fg.jointype & JT_LTORJ)
     && OptimizationEnabled(db, SQLITE_SimplifyJoin)
    ){
      यदि( pItem->fg.jointype & JT_वामः ){
        यदि( pItem->fg.jointype & JT_दक्षिणः ){
          TREETRACE(0x1000,pParse,p,
                    ("FULL-JOIN simplifies to RIGHT-JOIN on term %d\n",i));
          pItem->fg.jointype &= ~JT_वामः;
        }अन्यथा{
          TREETRACE(0x1000,pParse,p,
                    ("LEFT-JOIN simplifies to JOIN on term %d\n",i));
          pItem->fg.jointype &= ~(JT_वामः|JT_OUTER);
          unset_Join_व्यञ्जकम्(p->pWhere, pItem->iCursor, 0);
        }
      }
      यदि( pItem->fg.jointype & JT_LTORJ ){
        प्रति(j=i+1; j<pTabList->nSrc; j++){
          SrcItem *pI2 = &pTabList->a[j];
          यदि( pI2->fg.jointype & JT_दक्षिणः ){
            यदि( pI2->fg.jointype & JT_वामः ){
              TREETRACE(0x1000,pParse,p,
                        ("FULL-JOIN simplifies to LEFT-JOIN on term %d\n",j));
              pI2->fg.jointype &= ~JT_दक्षिणः;
            }अन्यथा{
              TREETRACE(0x1000,pParse,p,
                        ("RIGHT-JOIN simplifies to JOIN on term %d\n",j));
              pI2->fg.jointype &= ~(JT_दक्षिणः|JT_OUTER);
              unset_Join_व्यञ्जकम्(p->pWhere, pI2->iCursor, 1);
            }
          }
        }
        प्रति(j=pTabList->nSrc-1; j>=0; j--){
          pTabList->a[j].fg.jointype &= ~JT_LTORJ;
          यदि( pTabList->a[j].fg.jointype & JT_दक्षिणः ) भङ्ग;
        }
      }
    }

    /* No further action if this term of the FROM clause is not a subquery */
    यदि( pSub==0 ) अनुवर्त;

    /* Catch mismatch in the declared columns of a view and the number of
    ** columns in the SELECT on the RHS */
    यदि( pTab->nCol!=pSub->pEList->n_व्यञ्जकम् ){
      sqlite3_Error_सन्देशः(pParse, "expected %d columns for '%s' but got %d",
                      pTab->nCol, pTab->zName, pSub->pEList->n_व्यञ्जकम्);
      गच्छ select_अन्तः;
    }

    /* Do not attempt the usual optimizations (flattening and ORDER BY
    ** elimination) on a MATERIALIZED common table expression because
    ** a MATERIALIZED common table expression is an optimization fence.
    */
    यदि( pItem->fg.isCte && pItem->u2.pCteUse->eM10d==M10d_Yes ){
      अनुवर्त;
    }

    /* Do not try to flatten an aggregate subquery.
    **
    ** Flattening an aggregate subquery is only possible if the outer query
    ** is not a join.  But if the outer query is not a join, then the subquery
    ** will be implemented as a co-routine and there is no advantage to
    ** flattening in that case.
    */
    यदि( (pSub->selFlags & SF_Aggregate)!=0 ) अनुवर्त;
    प्रतिज्ञा( pSub->p_गणः_By==0 );

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
    यदि( pSub->p_क्रमः_By!=0
     && (p->p_क्रमः_By!=0 || pTabList->nSrc>1)      /* Condition (5) */
     && pSub->p_सीमा==0                           /* Condition (1) */
     && (pSub->selFlags & (SF_क्रमः_By_Reqd|SF_Recursive))==0  /* (2) and (6) */
     && (p->selFlags & SF_क्रमः_By_Reqd)==0         /* Condition (3) and (4) */
     && OptimizationEnabled(db, SQLITE_Omit_क्रमः_By)
    ){
      TREETRACE(0x800,pParse,p,
                ("omit superfluous ORDER BY on %r FROM-clause subquery\n",i+1));
      sqlite3_Parser_सङ्कलय_Cleanup(pParse, sqlite3_व्यञ्जकम्_List_Delete_Generic,
                              pSub->p_क्रमः_By);
      pSub->p_क्रमः_By = 0;
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
    यदि( pSub->p_क्रमः_By!=0
     && i==0
     && (p->selFlags & SF_Complex_परिणाम)!=0
     && (pTabList->nSrc==1
         || (pTabList->a[1].fg.jointype&(JT_OUTER|JT_CROSS))!=0)
    ){
      अनुवर्त;
    }

    /* tag-select-0240 */
    यदि( flattenSubquery(pParse, p, i, isAgg) ){
      यदि( pParse->nErr ) गच्छ select_अन्तः;
      /* This subquery can be absorbed into its parent. */
      i = -1;
    }
    pTabList = p->pSrc;
    यदि( db->mallocFailed ) गच्छ select_अन्तः;
    यदि( !IgnorableOrderby(pDest) ){
      s_क्रमय.p_क्रमः_By = p->p_क्रमः_By;
    }
  }
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_COMPOUND_SELECT
  /* Handle compound SELECT statements using the separate multiSelect()
  ** procedure.  tag-select-0300
  */
  यदि( p->p_प्राक् ){
    rc = multiSelect(pParse, p, pDest);
#यदि TREETRACE_ENABLED
    TREETRACE(0x400,pParse,p,("end compound-select processing\n"));
    यदि( (sqlite3_वृक्षः_Trace & 0x400)!=0 && ExplainQueryPlanParent(pParse)==0 ){
      sqlite3_वृक्षः_View_Select(0, p, 0);
    }
#यदि_अन्तः
    यदि( p->p_अग्र==0 ) Explain_Query_Plan_निष्कासय(pParse);
    प्रत्यागम rc;
  }
#यदि_अन्तः

  /* If there may be an "EXISTS (SELECT ...)" in the WHERE clause, attempt
  ** to change it into a join.  */
  यदि( pParse->bHasExists && OptimizationEnabled(db,SQLITE_Exists_गन्तव्यम्_Join) ){
    exists_गन्तव्यम्_Join(pParse, p, p->pWhere);
    pTabList = p->pSrc;
  }

  /* Do the WHERE-clause constant propagation optimization if this is
  ** a join.  No need to spend time on this operation for non-join queries
  ** as the equivalent optimization will be handled by query planner in
  ** sqlite3WhereBegin().  tag-select-0330
  */
  यदि( p->pWhere!=0
   && p->pWhere->op==TK_AND
   && OptimizationEnabled(db, SQLITE_PropagateConst)
   && propagateConstants(pParse, p)
  ){
#यदि TREETRACE_ENABLED
    यदि( sqlite3_वृक्षः_Trace & 0x2000 ){
      TREETRACE(0x2000,pParse,p,("After constant propagation:\n"));
      sqlite3_वृक्षः_View_Select(0, p, 0);
    }
#यदि_अन्तः
  }अन्यथा{
    TREETRACE(0x2000,pParse,p,("Constant propagation not helpful\n"));
  }

  /* tag-select-0350 */
  यदि( OptimizationEnabled(db, SQLITE_QueryFlattener|SQLITE_गणनम्_सम्बन्धि_View)
   && गणनम्_सम्बन्धि_View_Optimization(pParse, p)
  ){
    यदि( db->mallocFailed ) गच्छ select_अन्तः;
    pTabList = p->pSrc;
  }

  /* Loop over all terms in the FROM clause and do two things for each term:
  **
  **   (1) Authorize unreferenced tables
  **   (2) Generate code for all sub-queries
  **
  ** tag-select-0400
  */
  प्रति(i=0; i<pTabList->nSrc; i++){
    SrcItem *pItem = &pTabList->a[i];
    SrcItem *p_प्राक्;
    SelectDest dest;
    Subquery *pSubq;
    Select *pSub;
#यदि !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    ध्रुव वर्ण *zSavedAuthContext;
#यदि_अन्तः

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
    यदि( pItem->col_प्रयुक्तम्==0 && pItem->zName!=0 ){
      ध्रुव वर्ण *zDb;
      यदि( pItem->fg.fixedSchema ){
        पूर्णाङ्क iDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(pParse->db, pItem->u4.pSchema);
        zDb = db->aDb[iDb].zDbSName;
      }अन्यथा यदि( pItem->fg.isSubquery ){
        zDb = 0;
      }अन्यथा{
        zDb = pItem->u4.zDatabase;
      }
      sqlite3AuthCheck(pParse, SQLITE_READ, pItem->zName, "", zDb);
    }

#यदि !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    /* Generate code for all sub-queries in the FROM clause
    */
    यदि( pItem->fg.isSubquery==0 ) अनुवर्त;
    pSubq = pItem->u4.pSubq;
    प्रतिज्ञा( pSubq!=0 );
    pSub = pSubq->pSelect;

    /* The code for a subquery should only be generated once. */
    यदि( pSubq->addrFillSub!=0 ) अनुवर्त;

    /* Increment Parse.nHeight by the height of the largest expression
    ** tree referred to by this, the parent select. The child select
    ** may contain expression trees of at most
    ** (SQLITE_MAX_EXPR_DEPTH-Parse.nHeight) height. This is a bit
    ** more conservative than necessary, but much easier than enforcing
    ** an exact limit.
    */
    pParse->nHeight += sqlite3_Select_व्यञ्जकम्_Height(p);

    /* Make copies of constant WHERE-clause terms in the outer query down
    ** inside the subquery.  This can help the subquery to run more efficiently.
    ** This is the "predicate push-down optimization".  tag-select-0420
    */
    यदि( OptimizationEnabled(db, SQLITE_प्रक्षिप_अधस्तात्)
     && (pItem->fg.isCte==0
         || (pItem->u2.pCteUse->eM10d!=M10d_Yes && pItem->u2.pCteUse->nUse<2))
     && प्रक्षिप_अधस्तात्_Where_Terms(pParse, pSub, p->pWhere, pTabList, i)
    ){
#यदि TREETRACE_ENABLED
      यदि( sqlite3_वृक्षः_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("After WHERE-clause push-down into subquery %d:\n", pSub->selId));
        sqlite3_वृक्षः_View_Select(0, p, 0);
      }
#यदि_अन्तः
      प्रतिज्ञा( pSubq->pSelect && (pSub->selFlags & SF_प्रक्षिप_अधस्तात्)!=0 );
    }अन्यथा{
      TREETRACE(0x4000,pParse,p,("WHERE-clause push-down not possible\n"));
    }

    /* Convert unused result columns of the subquery into simple NULL
    ** expressions, to avoid unneeded searching and computation.
    ** tag-select-0440
    */
    यदि( OptimizationEnabled(db, SQLITE_Null_अप्रयुक्तम्_स्तम्भसङ्ख्या)
     && disable_अप्रयुक्तम्_Subquery_परिणाम_Columns(pItem)
    ){
#यदि TREETRACE_ENABLED
      यदि( sqlite3_वृक्षः_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("Change unused result columns to NULL for subquery %d:\n",
             pSub->selId));
        sqlite3_वृक्षः_View_Select(0, p, 0);
      }
#यदि_अन्तः
    }

    zSavedAuthContext = pParse->zAuthContext;
    pParse->zAuthContext = pItem->zName;

    /* Generate byte-code to implement the subquery  tag-select-0480
    */
    यदि( from_Clause_पदांशः_Can_Be_Coroutine(pParse, pTabList, i, p->selFlags) ){
      /* Implement a co-routine that will return a single row of the result
      ** set on each invocation.  tag-select-0482
      */
      पूर्णाङ्क addr_शिखरम् = sqlite3VdbeCurrentAddr(v)+1;
    
      pSubq->regReturn = ++pParse->nMem;
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_InitCoroutine, pSubq->regReturn, 0, addr_शिखरम्);
      VdbeComment((v, "%!S", pItem));
      pSubq->addrFillSub = addr_शिखरम्;
      sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);
      ExplainQueryPlan((pParse, 1, "CO-ROUTINE %!S", pItem));
      sqlite3Select(pParse, pSub, &dest);
      pItem->pSTab->nRowLogEst = pSub->nSelectRow;
      pItem->fg.द्वारा_Coroutine = 1;
      pSubq->reg_परिणाम = dest.iSdst;
      sqlite3_Vdbe_अन्तः_Coroutine(v, pSubq->regReturn);
      VdbeComment((v, "end %!S", pItem));
      sqlite3_Vdbe_Jump_अत्र(v, addr_शिखरम्-1);
      sqlite3_Clear_अस्थायी_Reg_Cache(pParse);
    }अन्यथा यदि( pItem->fg.isCte && pItem->u2.pCteUse->addrM9e>0 ){
      /* This is a CTE for which materialization code has already been
      ** generated.  Invoke the subroutine to compute the materialization,
      ** then make the pItem->iCursor be a copy of the ephemeral table that
      ** holds the result of the materialization. tag-select-0484 */
      CteUse *pCteUse = pItem->u2.pCteUse;
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, pCteUse->regRtn, pCteUse->addrM9e);
      यदि( pItem->iCursor!=pCteUse->iCur ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_OpenDup, pItem->iCursor, pCteUse->iCur);
        VdbeComment((v, "%!S", pItem));
      }
      pSub->nSelectRow = pCteUse->nRowEst;
    }अन्यथा यदि( (p_प्राक् = isSelfJoinView(pTabList, pItem, 0, i))!=0 ){
      /* This view has already been materialized by a prior entry in
      ** this same FROM clause.  Reuse it.  tag-select-0486 */
      Subquery *p_प्राक्_Subq;
      प्रतिज्ञा( p_प्राक्->fg.isSubquery );
      p_प्राक्_Subq = p_प्राक्->u4.pSubq;
      प्रतिज्ञा( p_प्राक्_Subq!=0 );
      यदि( p_प्राक्_Subq->addrFillSub ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, p_प्राक्_Subq->regReturn,
                                       p_प्राक्_Subq->addrFillSub);
      }
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_OpenDup, pItem->iCursor, p_प्राक्->iCursor);
      pSub->nSelectRow = p_प्राक्_Subq->pSelect->nSelectRow;
    }अन्यथा{
      /* Materialize the view.  If the view is not correlated, generate a
      ** subroutine to do the materialization so that subsequent uses of
      ** the same view can reuse the materialization.  tag-select-0488 */
      पूर्णाङ्क शिखरम्_Addr;
      पूर्णाङ्क onceAddr = 0;
#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
      पूर्णाङ्क addrExplain;
#यदि_अन्तः

      pSubq->regReturn = ++pParse->nMem;
      शिखरम्_Addr = sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Goto);
      pSubq->addrFillSub = शिखरम्_Addr+1;
      pItem->fg.isMaterialized = 1;
      यदि( pItem->fg.isCorrelated==0 ){
        /* If the subquery is not correlated and if we are not inside of
        ** a trigger, then we only need to compute the value of the subquery
        ** once. */
        onceAddr = sqlite3_Vdbe_सङ्कलय_Op0(v, OP_Once); VdbeCoverage(v);
        VdbeComment((v, "materialize %!S", pItem));
      }अन्यथा{
        VdbeNoopComment((v, "materialize %!S", pItem));
      }
      sqlite3SelectDestInit(&dest, SRT_EphemTab, pItem->iCursor);

      ExplainQueryPlan2(addrExplain, (pParse, 1, "MATERIALIZE %!S", pItem));
      sqlite3Select(pParse, pSub, &dest);
      pItem->pSTab->nRowLogEst = pSub->nSelectRow;
      यदि( onceAddr ) sqlite3_Vdbe_Jump_अत्र(v, onceAddr);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Return, pSubq->regReturn, शिखरम्_Addr+1);
      VdbeComment((v, "end %!S", pItem));
      sqlite3_Vdbe_अवलोकनम्_स्थितिः_Range(v, addrExplain, addrExplain, -1);
      sqlite3_Vdbe_Jump_अत्र(v, शिखरम्_Addr);
      sqlite3_Clear_अस्थायी_Reg_Cache(pParse);
      यदि( pItem->fg.isCte && pItem->fg.isCorrelated==0 ){
        CteUse *pCteUse = pItem->u2.pCteUse;
        pCteUse->addrM9e = pSubq->addrFillSub;
        pCteUse->regRtn = pSubq->regReturn;
        pCteUse->iCur = pItem->iCursor;
        pCteUse->nRowEst = pSub->nSelectRow;
      }
    }
    यदि( db->mallocFailed ) गच्छ select_अन्तः;
    pParse->nHeight -= sqlite3_Select_व्यञ्जकम्_Height(p);
    pParse->zAuthContext = zSavedAuthContext;
#यदि_अन्तः
  }

  /* Various elements of the SELECT copied into local variables for
  ** convenience */
  pEList = p->pEList;
  pWhere = p->pWhere;
  p_गणः_By = p->p_गणः_By;
  pHaving = p->pHaving;
  sDistinct.isTnct = (p->selFlags & SF_Distinct)!=0;

#यदि TREETRACE_ENABLED
  यदि( sqlite3_वृक्षः_Trace & 0x8000 ){
    TREETRACE(0x8000,pParse,p,("After all FROM-clause analysis:\n"));
    sqlite3_वृक्षः_View_Select(0, p, 0);
  }
#यदि_अन्तः

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
  यदि( (p->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct
   && sqlite3_Copy_क्रमय_क्रमः(pEList, s_क्रमय.p_क्रमः_By)
   && sqlite3_व्यञ्जकम्_List_Compare(pEList, s_क्रमय.p_क्रमः_By, -1)==0
   && OptimizationEnabled(db, SQLITE_गणः_By_क्रमः)
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
   && p->pWin==0
#यदि_अन्तः
  ){
    p->selFlags &= ~(u32)SF_Distinct;
    p_गणः_By = p->p_गणः_By = sqlite3_व्यञ्जकम्_List_Dup(db, pEList, 0);
    यदि( p_गणः_By ){
      प्रति(i=0; i<p_गणः_By->n_व्यञ्जकम्; i++){
        p_गणः_By->a[i].u.x.i_क्रमः_By_Col = i+1;
      }
    }
    p->selFlags |= SF_Aggregate;
    /* Notice that even thought SF_Distinct has been cleared from p->selFlags,
    ** the sDistinct.isTnct is still set.  Hence, isTnct represents the
    ** original setting of the SF_Distinct flag, not the current setting */
    प्रतिज्ञा( sDistinct.isTnct );
    sDistinct.isTnct = 2;

#यदि TREETRACE_ENABLED
    यदि( sqlite3_वृक्षः_Trace & 0x20000 ){
      TREETRACE(0x20000,pParse,p,("Transform DISTINCT into GROUP BY:\n"));
      sqlite3_वृक्षः_View_Select(0, p, 0);
    }
#यदि_अन्तः
  }

  /* If there is an ORDER BY clause, then create an ephemeral index to
  ** do the sorting.  But this sorting ephemeral index might end up
  ** being unused if the data can be extracted in pre-sorted order.
  ** If that is the case, then the OP_OpenEphemeral instruction will be
  ** changed to an OP_Noop once we figure out that the sorting index is
  ** not needed.  The sSort.addrSortIndex variable is used to facilitate
  ** that change.  tag-select-0600
  */
  यदि( s_क्रमय.p_क्रमः_By ){
    कुञ्चिका_Info *p_कुञ्चिका_Info;
    p_कुञ्चिका_Info = sqlite3_कुञ्चिका_Info_From_व्यञ्जकम्_List(
        pParse, s_क्रमय.p_क्रमः_By, 0, pEList->n_व्यञ्जकम्);
    s_क्रमय.iECursor = pParse->nTab++;
    s_क्रमय.addr_क्रमय_सूचकांक =
      sqlite3_Vdbe_सङ्कलय_Op4(v, OP_OpenEphemeral,
          s_क्रमय.iECursor, s_क्रमय.p_क्रमः_By->n_व्यञ्जकम्+1+pEList->n_व्यञ्जकम्, 0,
          (वर्ण*)p_कुञ्चिका_Info, P4_KEYINFO
      );
  }अन्यथा{
    s_क्रमय.addr_क्रमय_सूचकांक = -1;
  }

  /* If the output is destined for a temporary table, open that table.
  ** tag-select-0630
  */
  यदि( pDest->eDest==SRT_EphemTab ){
    sqlite3_Vdbe_सङ्कलय_Op2(v, OP_OpenEphemeral, pDest->iSDParm, pEList->n_व्यञ्जकम्);
    यदि( p->selFlags & SF_NestedFrom ){
      /* Delete or NULL-out result columns that will never be used */
      पूर्णाङ्क ii;
      प्रति(ii=pEList->n_व्यञ्जकम्-1; ii>0 && pEList->a[ii].fg.b_प्रयुक्तम्==0; ii--){
        sqlite3_व्यञ्जकम्_Delete(db, pEList->a[ii].p_व्यञ्जकम्);
        sqlite3DbFree(db, pEList->a[ii].zEName);
        pEList->n_व्यञ्जकम्--;
      }
      प्रति(ii=0; ii<pEList->n_व्यञ्जकम्; ii++){
        यदि( pEList->a[ii].fg.b_प्रयुक्तम्==0 ) pEList->a[ii].p_व्यञ्जकम्->op = TK_NULL;
      }
    }
  }

  /* Set the limiter.  tag-select-0650
  */
  i_अन्तः = sqlite3VdbeMakeLabel(pParse);
  यदि( (p->selFlags & SF_Fixed_सीमा)==0 ){
    p->nSelectRow = 320;  /* 4 billion rows */
  }
  यदि( p->p_सीमा ) compute_सीमा_Registers(pParse, p, i_अन्तः);
  यदि( p->i_सीमा==0 && s_क्रमय.addr_क्रमय_सूचकांक>=0 ){
    sqlite3VdbeChangeOpcode(v, s_क्रमय.addr_क्रमय_सूचकांक, OP_SorterOpen);
    s_क्रमय.क्रमय_Flags |= SORTFLAG_UseSorter;
  }

  /* Open an ephemeral index to use for the distinct set. tag-select-0680
  */
  यदि( p->selFlags & SF_Distinct ){
    sDistinct.tabTnct = pParse->nTab++;
    sDistinct.addrTnct = sqlite3_Vdbe_सङ्कलय_Op4(v, OP_OpenEphemeral,
                       sDistinct.tabTnct, 0, 0,
                       (वर्ण*)sqlite3_कुञ्चिका_Info_From_व्यञ्जकम्_List(pParse, p->pEList,0,0),
                       P4_KEYINFO);
    sqlite3VdbeChangeP5(v, BTREE_UNORDERED);
    sDistinct.eTnctType = WHERE_DISTINCT_UNORDERED;
  }अन्यथा{
    sDistinct.eTnctType = WHERE_DISTINCT_NOOP;
  }

  यदि( !isAgg && p_गणः_By==0 ){
    /* No aggregate functions and no GROUP BY clause.  tag-select-0700 */
    u16 wctrlFlags = (sDistinct.isTnct ? WHERE_WANT_DISTINCT : 0)
                   | (p->selFlags & SF_Fixed_सीमा);
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
    गवाक्षः *pWin = p->pWin;      /* Main window object (or NULL) */
    यदि( pWin ){
      sqlite3_गवाक्षः_संहिता_Init(pParse, p);
    }
#यदि_अन्तः
    प्रतिज्ञा( WHERE_USE_सीमा==SF_Fixed_सीमा );


    /* Begin the database scan. */
    TREETRACE(0x2,pParse,p,("WhereBegin\n"));
    pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, s_क्रमय.p_क्रमः_By,
                               p->pEList, p, wctrlFlags, p->nSelectRow);
    यदि( pWInfo==0 ) गच्छ select_अन्तः;
    यदि( sqlite3_Where_Output_Row_गणनम्(pWInfo) < p->nSelectRow ){
      p->nSelectRow = sqlite3_Where_Output_Row_गणनम्(pWInfo);
      यदि( pDest->eDest<=SRT_DistQueue && pDest->eDest>=SRT_DistFifo ){
        /* TUNING: For a UNION CTE, because UNION is implies DISTINCT,
        ** reduce the estimated output row count by 8 (LogEst 30). 
        ** Search for tag-20250414a to see other cases */
        p->nSelectRow -= 30;
      }
    }
    यदि( sDistinct.isTnct && sqlite3WhereIsDistinct(pWInfo) ){
      sDistinct.eTnctType = sqlite3WhereIsDistinct(pWInfo);
    }
    यदि( s_क्रमय.p_क्रमः_By ){
      s_क्रमय.nOBSat = sqlite3WhereIsOrdered(pWInfo);
      s_क्रमय.labelOBLopt = sqlite3_Where_क्रमः_By_सीमा_Opt_Label(pWInfo);
      यदि( s_क्रमय.nOBSat==s_क्रमय.p_क्रमः_By->n_व्यञ्जकम् ){
        s_क्रमय.p_क्रमः_By = 0;
      }
    }
    TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));

    /* If sorting index that was created by a prior OP_OpenEphemeral
    ** instruction ended up not being needed, then change the OP_OpenEphemeral
    ** into an OP_Noop.
    */
    यदि( s_क्रमय.addr_क्रमय_सूचकांक>=0 && s_क्रमय.p_क्रमः_By==0 ){
      sqlite3_Vdbe_Change_गन्तव्यम्_Noop(v, s_क्रमय.addr_क्रमय_सूचकांक);
    }

    प्रतिज्ञा( p->pEList==pEList );
#यदि_अपरिभाषितम् SQLITE_OMIT_WINDOWFUNC
    यदि( pWin ){
      पूर्णाङ्क addrGosub = sqlite3VdbeMakeLabel(pParse);
      पूर्णाङ्क iCont = sqlite3VdbeMakeLabel(pParse);
      पूर्णाङ्क iBreak = sqlite3VdbeMakeLabel(pParse);
      पूर्णाङ्क regGosub = ++pParse->nMem;

      sqlite3_गवाक्षः_संहिता_चरणः(pParse, p, pWInfo, regGosub, addrGosub);

      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Goto, 0, iBreak);
      sqlite3VdbeResolveLabel(v, addrGosub);
      VdbeNoopComment((v, "inner-loop subroutine"));
      s_क्रमय.labelOBLopt = 0;
      selectInnerLoop(pParse, p, -1, &s_क्रमय, &sDistinct, pDest, iCont, iBreak);
      sqlite3VdbeResolveLabel(v, iCont);
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Return, regGosub);
      VdbeComment((v, "end inner-loop subroutine"));
      sqlite3VdbeResolveLabel(v, iBreak);
    }अन्यथा
#यदि_अन्तः /* SQLITE_OMIT_WINDOWFUNC */
    {
      /* Use the standard inner loop. */
      selectInnerLoop(pParse, p, -1, &s_क्रमय, &sDistinct, pDest,
          sqlite3WhereContinueLabel(pWInfo),
          sqlite3WhereBreakLabel(pWInfo));

      /* End the database scan loop.
      */
      TREETRACE(0x2,pParse,p,("WhereEnd\n"));
      sqlite3_Where_अन्तः(pWInfo);
    }
  }अन्यथा{
    /* This case is for when there exist aggregate functions or a GROUP BY
    ** clause or both.  tag-select-0800 */
    NameContext sNC;    /* Name context for processing aggregate information */
    पूर्णाङ्क iAMem;          /* First Mem address for storing current GROUP BY */
    पूर्णाङ्क iBMem;          /* First Mem address for previous GROUP BY */
    पूर्णाङ्क iUseFlag;       /* Mem address holding flag indicating that at least
                        ** one row of the input to the aggregator has been
                        ** processed */
    पूर्णाङ्क iAbortFlag;     /* Mem address which causes query abort if positive */
    पूर्णाङ्क गणः_By_क्रमय;    /* Rows come from source in GROUP BY order */
    पूर्णाङ्क addr_अन्तः;        /* End of processing for this SELECT */
    पूर्णाङ्क क्रमय_PTab = 0;   /* Pseudotable used to decode sorting results */
    पूर्णाङ्क क्रमय_बहिर्गमनम् = 0;    /* Output register from the sorter */
    पूर्णाङ्क क्रमः_By_Grp = 0; /* True if the GROUP BY and ORDER BY are the same */

    /* Remove any and all aliases between the result set and the
    ** GROUP BY clause.
    */
    यदि( p_गणः_By ){
      पूर्णाङ्क k;                        /* Loop counter */
      संरचना व्यञ्जकम्_List_item *pItem;  /* For looping over expression in a list */

      प्रति(k=p->pEList->n_व्यञ्जकम्, pItem=p->pEList->a; k>0; k--, pItem++){
        pItem->u.x.iAlias = 0;
      }
      प्रति(k=p_गणः_By->n_व्यञ्जकम्, pItem=p_गणः_By->a; k>0; k--, pItem++){
        pItem->u.x.iAlias = 0;
      }
      प्रतिज्ञा( 66==sqlite3LogEst(100) );
      यदि( p->nSelectRow>66 ) p->nSelectRow = 66;

      /* If there is both a GROUP BY and an ORDER BY clause and they are
      ** identical, then it may be possible to disable the ORDER BY clause
      ** on the grounds that the GROUP BY will cause elements to come out
      ** in the correct order. It also may not - the GROUP BY might use a
      ** database index that causes rows to be grouped together as required
      ** but not actually sorted. Either way, record the fact that the
      ** ORDER BY and GROUP BY clauses are the same by setting the orderByGrp
      ** variable.  */
      यदि( sqlite3_Copy_क्रमय_क्रमः(p_गणः_By, s_क्रमय.p_क्रमः_By)
       && sqlite3_व्यञ्जकम्_List_Compare(p_गणः_By, s_क्रमय.p_क्रमः_By, -1)==0
      ){
        क्रमः_By_Grp = 1;
      }
    }अन्यथा{
      प्रतिज्ञा( 0==sqlite3LogEst(1) );
      p->nSelectRow = 0;
    }

    /* Create a label to jump to when we want to abort the query */
    addr_अन्तः = sqlite3VdbeMakeLabel(pParse);

    /* Convert TK_COLUMN nodes into TK_AGG_COLUMN and make entries in
    ** sAggInfo for all TK_AGG_FUNCTION nodes in expressions of the
    ** SELECT statement.
    */
    pAggInfo = sqlite3DbMallocZero(db, परिमाण(*pAggInfo) );
    यदि( pAggInfo ){
      sqlite3_Parser_सङ्कलय_Cleanup(pParse, agginfoFree, pAggInfo);
      testcase( pParse->earlyCleanup );
    }
    यदि( db->mallocFailed ){
      गच्छ select_अन्तः;
    }
    pAggInfo->selId = p->selId;
#यदि_परिभाषितम् SQLITE_DEBUG
    pAggInfo->pSelect = p;
#यदि_अन्तः
    स्मृतिं_पूरय(&sNC, 0, परिमाण(sNC));
    sNC.pParse = pParse;
    sNC.pSrcList = pTabList;
    sNC.uNC.pAggInfo = pAggInfo;
    VVA_ONLY( sNC.ncFlags = NC_UAggInfo; )
    pAggInfo->n_क्रमणम्_Column = p_गणः_By ? p_गणः_By->n_व्यञ्जकम् : 0;
    pAggInfo->p_गणः_By = p_गणः_By;
    sqlite3_व्यञ्जकम्_Analyze_Agg_List(&sNC, pEList);
    sqlite3_व्यञ्जकम्_Analyze_Agg_List(&sNC, s_क्रमय.p_क्रमः_By);
    यदि( pHaving ){
      यदि( p_गणः_By ){
        प्रतिज्ञा( pWhere==p->pWhere );
        प्रतिज्ञा( pHaving==p->pHaving );
        प्रतिज्ञा( p_गणः_By==p->p_गणः_By );
        having_गन्तव्यम्_Where(pParse, p);
        pWhere = p->pWhere;
      }
      sqlite3_व्यञ्जकम्_Analyze_Aggregates(&sNC, pHaving);
    }
    pAggInfo->n_सञ्चायकः = pAggInfo->nColumn;
    यदि( p->p_गणः_By==0 && p->pHaving==0 && pAggInfo->nFunc==1 ){
      minMaxFlag = minMaxQuery(db, pAggInfo->aFunc[0].pFExpr, &p_Min_Max_क्रमः_By);
    }अन्यथा{
      minMaxFlag = WHERE_ORDERBY_प्रसामान्यम्;
    }
    analyze_Agg_Func_तर्काः(pAggInfo, &sNC);
    यदि( db->mallocFailed ) गच्छ select_अन्तः;
#यदि TREETRACE_ENABLED
    यदि( sqlite3_वृक्षः_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("After aggregate analysis %p:\n", pAggInfo));
      sqlite3_वृक्षः_View_Select(0, p, 0);
      यदि( minMaxFlag ){
        sqlite3DebugPrintf("MIN/MAX Optimization (0x%02x) adds:\n", minMaxFlag);
        sqlite3_वृक्षः_View_व्यञ्जकम्_List(0, p_Min_Max_क्रमः_By, 0, "ORDERBY");
      }
      printAggInfo(pAggInfo);
    }
#यदि_अन्तः


    /* Processing for aggregates with GROUP BY is very different and
    ** much more complex than aggregates without a GROUP BY.  tag-select-0810
    */
    यदि( p_गणः_By ){
      कुञ्चिका_Info *p_कुञ्चिका_Info;  /* Keying information for the group by clause */
      पूर्णाङ्क addr1;          /* A-vs-B comparison jump */
      पूर्णाङ्क addrOutputRow;  /* Start of subroutine that outputs a result row */
      पूर्णाङ्क regOutputRow;   /* Return address register for output subroutine */
      पूर्णाङ्क addr_समुच्चयः_Abort;   /* Set the abort flag and return */
      पूर्णाङ्क addr_शिखरम्_सम्बन्धि_Loop;  /* Top of the input loop */
      पूर्णाङ्क addr_क्रमणम्_Idx; /* The OP_OpenEphemeral for the sorting index */
      पूर्णाङ्क addrReset;      /* Subroutine for resetting the accumulator */
      पूर्णाङ्क regReset;       /* Return address register for reset subroutine */
      व्यञ्जकम्_List *pDistinct = 0;
      u16 distFlag = 0;
      पूर्णाङ्क eDist = WHERE_DISTINCT_NOOP;

      यदि( pAggInfo->nFunc==1
       && pAggInfo->aFunc[0].iDistinct>=0
       && ALWAYS(pAggInfo->aFunc[0].pFExpr!=0)
       && ALWAYS(व्यञ्जकम्_Use_XList(pAggInfo->aFunc[0].pFExpr))
       && pAggInfo->aFunc[0].pFExpr->x.pList!=0
      ){
        व्यञ्जकम् *p_व्यञ्जकम् = pAggInfo->aFunc[0].pFExpr->x.pList->a[0].p_व्यञ्जकम्;
        p_व्यञ्जकम् = sqlite3_व्यञ्जकम्_Dup(db, p_व्यञ्जकम्, 0);
        pDistinct = sqlite3_व्यञ्जकम्_List_Dup(db, p_गणः_By, 0);
        pDistinct = sqlite3_व्यञ्जकम्_List_Append(pParse, pDistinct, p_व्यञ्जकम्);
        distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
      }

      /* If there is a GROUP BY clause we might need a sorting index to
      ** implement it.  Allocate that sorting index now.  If it turns out
      ** that we do not need it after all, the OP_SorterOpen instruction
      ** will be converted into a Noop. 
      */
      pAggInfo->क्रमणम्_Idx = pParse->nTab++;
      p_कुञ्चिका_Info = sqlite3_कुञ्चिका_Info_From_व्यञ्जकम्_List(pParse, p_गणः_By,
                                            0, pAggInfo->nColumn);
      addr_क्रमणम्_Idx = sqlite3_Vdbe_सङ्कलय_Op4(v, OP_SorterOpen,
          pAggInfo->क्रमणम्_Idx, pAggInfo->n_क्रमणम्_Column,
          0, (वर्ण*)p_कुञ्चिका_Info, P4_KEYINFO);

      /* Initialize memory locations used by GROUP BY aggregate processing
      */
      iUseFlag = ++pParse->nMem;
      iAbortFlag = ++pParse->nMem;
      regOutputRow = ++pParse->nMem;
      addrOutputRow = sqlite3VdbeMakeLabel(pParse);
      regReset = ++pParse->nMem;
      addrReset = sqlite3VdbeMakeLabel(pParse);
      iAMem = pParse->nMem + 1;
      pParse->nMem += p_गणः_By->n_व्यञ्जकम्;
      iBMem = pParse->nMem + 1;
      pParse->nMem += p_गणः_By->n_व्यञ्जकम्;
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 0, iAbortFlag);
      VdbeComment((v, "clear abort flag"));
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Null, 0, iAMem, iAMem+p_गणः_By->n_व्यञ्जकम्-1);
      sqlite3_व्यञ्जकम्_Null_पञ्जिकाकोशः_Range(pParse, iAMem, p_गणः_By->n_व्यञ्जकम्);

      /* Begin a loop that will extract all source rows in GROUP BY order.
      ** This might involve two separate loops with an OP_Sort in between, or
      ** it might be a single loop that uses an index to extract information
      ** in the right order to begin with.
      */
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, regReset, addrReset);
      TREETRACE(0x2,pParse,p,("WhereBegin\n"));
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, p_गणः_By, pDistinct,
          p, (sDistinct.isTnct==2 ? WHERE_DISTINCTBY : WHERE_GROUPBY)
          |  (क्रमः_By_Grp ? WHERE_SORTBYGROUP : 0) | distFlag, 0
      );
      यदि( pWInfo==0 ){
        sqlite3_व्यञ्जकम्_List_Delete(db, pDistinct);
        गच्छ select_अन्तः;
      }
      यदि( pParse->pIdxEpr ){
        optimize_Aggregate_Use_सम्बन्धि_अनुक्रमितम्_व्यञ्जकम्(pParse, p, pAggInfo, &sNC);
      }
      नियोजय_Aggregate_Registers(pParse, pAggInfo);
      eDist = sqlite3WhereIsDistinct(pWInfo);
      TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
      यदि( sqlite3WhereIsOrdered(pWInfo)==p_गणः_By->n_व्यञ्जकम् ){
        /* The optimizer is able to deliver rows in group by order so
        ** we do not have to sort.  The OP_OpenEphemeral table will be
        ** cancelled later because we still need to use the pKeyInfo
        */
        गणः_By_क्रमय = 0;
      }अन्यथा{
        /* Rows are coming out in undetermined order.  We have to push
        ** each row into a sorting index, terminate the first loop,
        ** then loop over the sorting index in order to get the output
        ** in sorted order
        */
        पूर्णाङ्क reg_आधारः;
        पूर्णाङ्क reg_लेखः;
        पूर्णाङ्क nCol;
        पूर्णाङ्क n_गणः_By;

#यदि_परिभाषितम् SQLITE_ENABLE_STMT_SCANSTATUS
        पूर्णाङ्क addrExp;              /* Address of OP_Explain instruction */
#यदि_अन्तः
        ExplainQueryPlan2(addrExp, (pParse, 0, "USE TEMP B-TREE FOR %s",
            (sDistinct.isTnct && (p->selFlags&SF_Distinct)==0) ?
                    "DISTINCT" : "GROUP BY"
        ));

        गणः_By_क्रमय = 1;
        n_गणः_By = p_गणः_By->n_व्यञ्जकम्;
        nCol = n_गणः_By;
        j = n_गणः_By;
        प्रति(i=0; i<pAggInfo->nColumn; i++){
          यदि( pAggInfo->aCol[i].iSorterColumn>=j ){
            nCol++;
            j++;
          }
        }
        reg_आधारः = sqlite3_Get_अस्थायी_Range(pParse, nCol);
        sqlite3_व्यञ्जकम्_संहिता_व्यञ्जकम्_List(pParse, p_गणः_By, reg_आधारः, 0, 0);
        j = n_गणः_By;
        pAggInfo->प्रत्यक्षम्_बहुलकम् = 1;
        प्रति(i=0; i<pAggInfo->nColumn; i++){
          संरचना AggInfo_col *pCol = &pAggInfo->aCol[i];
          यदि( pCol->iSorterColumn>=j ){
            sqlite3_व्यञ्जकम्_संहिता(pParse, pCol->pCExpr, j + reg_आधारः);
            j++;
          }
        }
        pAggInfo->प्रत्यक्षम्_बहुलकम् = 0;
        reg_लेखः = sqlite3_Get_अस्थायी_Reg(pParse);
        sqlite3_Vdbe_अवलोकनम्_स्थितिः_Counters(v, addrExp, 0, sqlite3VdbeCurrentAddr(v));
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Make_लेखः, reg_आधारः, nCol, reg_लेखः);
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Sorter_निवेशय, pAggInfo->क्रमणम्_Idx, reg_लेखः);
        sqlite3_Vdbe_अवलोकनम्_स्थितिः_Range(v, addrExp, sqlite3VdbeCurrentAddr(v)-2, -1);
        sqlite3_Release_अस्थायी_Reg(pParse, reg_लेखः);
        sqlite3_Release_अस्थायी_Range(pParse, reg_आधारः, nCol);
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_अन्तः(pWInfo);
        pAggInfo->क्रमणम्_Idx_PTab = क्रमय_PTab = pParse->nTab++;
        क्रमय_बहिर्गमनम् = sqlite3_Get_अस्थायी_Reg(pParse);
        sqlite3_Vdbe_अवलोकनम्_स्थितिः_Counters(v, addrExp, sqlite3VdbeCurrentAddr(v), 0);
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_OpenPseudo, क्रमय_PTab, क्रमय_बहिर्गमनम्, nCol);
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Sorter_क्रमय, pAggInfo->क्रमणम्_Idx, addr_अन्तः);
        VdbeComment((v, "GROUP BY sort")); VdbeCoverage(v);
        pAggInfo->use_क्रमणम्_Idx = 1;
        sqlite3_Vdbe_अवलोकनम्_स्थितिः_Range(v, addrExp, -1, क्रमय_PTab);
        sqlite3_Vdbe_अवलोकनम्_स्थितिः_Range(v, addrExp, -1, pAggInfo->क्रमणम्_Idx);
      }

      /* If there are entries in pAgggInfo->aFunc[] that contain subexpressions
      ** that are indexed (and that were previously identified and tagged
      ** in optimizeAggregateUseOfIndexedExpr()) then those subexpressions
      ** must now be converted into a TK_AGG_COLUMN node so that the value
      ** is correctly pulled from the index rather than being recomputed. */
      यदि( pParse->pIdxEpr ){
        aggregate_Convert_अनुक्रमितम्_व्यञ्जकम्_Ref_गन्तव्यम्_Column(pAggInfo);
#यदि TREETRACE_ENABLED
        यदि( sqlite3_वृक्षः_Trace & 0x20 ){
          TREETRACE(0x20, pParse, p,
             ("AggInfo function expressions converted to reference index\n"));
          sqlite3_वृक्षः_View_Select(0, p, 0);
          printAggInfo(pAggInfo);
        }
#यदि_अन्तः
      }

      /* If the index or temporary table used by the GROUP BY sort
      ** will naturally deliver rows in the order required by the ORDER BY
      ** clause, cancel the ephemeral table open coded earlier.
      **
      ** This is an optimization - the correct answer should result regardless.
      ** Use the SQLITE_GroupByOrder flag with SQLITE_TESTCTRL_OPTIMIZER to
      ** disable this optimization for testing purposes.  */
      यदि( क्रमः_By_Grp && OptimizationEnabled(db, SQLITE_गणः_By_क्रमः)
       && (गणः_By_क्रमय || sqlite3WhereIsSorted(pWInfo))
      ){
        s_क्रमय.p_क्रमः_By = 0;
        sqlite3_Vdbe_Change_गन्तव्यम्_Noop(v, s_क्रमय.addr_क्रमय_सूचकांक);
      }

      /* Evaluate the current GROUP BY terms and store in b0, b1, b2...
      ** (b0 is memory location iBMem+0, b1 is iBMem+1, and so forth)
      ** Then compare the current GROUP BY terms against the GROUP BY terms
      ** from the previous row currently stored in a0, a1, a2...
      */
      addr_शिखरम्_सम्बन्धि_Loop = sqlite3VdbeCurrentAddr(v);
      यदि( गणः_By_क्रमय ){
        sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Sorter_दत्त, pAggInfo->क्रमणम्_Idx,
                          क्रमय_बहिर्गमनम्, क्रमय_PTab);
      }
      प्रति(j=0; j<p_गणः_By->n_व्यञ्जकम्; j++){
        पूर्णाङ्क i_क्रमः_By_Col = p_गणः_By->a[j].u.x.i_क्रमः_By_Col;
        
        यदि( गणः_By_क्रमय ){
          sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Column, क्रमय_PTab, j, iBMem+j);
        }अन्यथा{
          pAggInfo->प्रत्यक्षम्_बहुलकम् = 1;
          sqlite3_व्यञ्जकम्_संहिता(pParse, p_गणः_By->a[j].p_व्यञ्जकम्, iBMem+j);
        }

        यदि( i_क्रमः_By_Col ){
          व्यञ्जकम् *pX = p->pEList->a[i_क्रमः_By_Col-1].p_व्यञ्जकम्;
          व्यञ्जकम् *p_आधारः = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(pX);
          यावत्( ALWAYS(p_आधारः!=0) && p_आधारः->op==TK_IF_NULL_ROW ){
            pX = p_आधारः->p_वामः;
            p_आधारः = sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(pX);
          }
          यदि( ALWAYS(p_आधारः!=0)
           && p_आधारः->op!=TK_AGG_COLUMN
           && p_आधारः->op!=TK_पञ्जिकाकोशः
          ){
            sqlite3_व्यञ्जकम्_गन्तव्यम्_पञ्जिकाकोशः(pX, iAMem+j);
          }
        }
      }
      sqlite3_Vdbe_सङ्कलय_Op4(v, OP_Compare, iAMem, iBMem, p_गणः_By->n_व्यञ्जकम्,
                          (वर्ण*)sqlite3_कुञ्चिका_Info_Ref(p_कुञ्चिका_Info), P4_KEYINFO);
      addr1 = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_सङ्कलय_Op3(v, OP_Jump, addr1+1, 0, addr1+1); VdbeCoverage(v);

      /* Generate code that runs whenever the GROUP BY changes.
      ** Changes in the GROUP BY are detected by the previous code
      ** block.  If there were no changes, this block is skipped.
      **
      ** This code copies current group by terms in b0,b1,b2,...
      ** over to a0,a1,a2.  It then calls the output subroutine
      ** and resets the aggregate accumulator registers in preparation
      ** for the next GROUP BY batch.
      */
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, regOutputRow, addrOutputRow);
      VdbeComment((v, "output one row of %d", p->selId));
      sqlite3_व्यञ्जकम्_संहिता_Move(pParse, iBMem, iAMem, p_गणः_By->n_व्यञ्जकम्);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IfPos, iAbortFlag, addr_अन्तः); VdbeCoverage(v);
      VdbeComment((v, "check abort flag"));
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, regReset, addrReset);
      VdbeComment((v, "reset accumulator %d", p->selId));

      /* Update the aggregate accumulators based on the content of
      ** the current row
      */
      sqlite3_Vdbe_Jump_अत्र(v, addr1);
      update_सञ्चायकः(pParse, iUseFlag, pAggInfo, eDist);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 1, iUseFlag);
      VdbeComment((v, "indicate data in accumulator %d", p->selId));

      /* End of the loop
      */
      यदि( गणः_By_क्रमय ){
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Sorter_अग्र, pAggInfo->क्रमणम्_Idx,addr_शिखरम्_सम्बन्धि_Loop);
        VdbeCoverage(v);
      }अन्यथा{
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_अन्तः(pWInfo);
        sqlite3_Vdbe_Change_गन्तव्यम्_Noop(v, addr_क्रमणम्_Idx);
      }
      sqlite3_व्यञ्जकम्_List_Delete(db, pDistinct);

      /* Output the final row of result
      */
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Gosub, regOutputRow, addrOutputRow);
      VdbeComment((v, "output final row of %d", p->selId));

      /* Jump over the subroutines
      */
      sqlite3VdbeGoto(v, addr_अन्तः);

      /* Generate a subroutine that outputs a single row of the result
      ** set.  This subroutine first looks at the iUseFlag.  If iUseFlag
      ** is less than or equal to zero, the subroutine is a no-op.  If
      ** the processing calls for the query to abort, this subroutine
      ** increments the iAbortFlag memory location before returning in
      ** order to signal the caller to abort.
      */
      addr_समुच्चयः_Abort = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 1, iAbortFlag);
      VdbeComment((v, "set abort flag"));
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Return, regOutputRow);
      sqlite3VdbeResolveLabel(v, addrOutputRow);
      addrOutputRow = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_IfPos, iUseFlag, addrOutputRow+2);
      VdbeCoverage(v);
      VdbeComment((v, "Groupby result generator entry point %d", p->selId));
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Return, regOutputRow);
      finalizeAggFunctions(pParse, pAggInfo);
      sqlite3_व्यञ्जकम्_If_False(pParse, pHaving, addrOutputRow+1, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, &s_क्रमय,
                      &sDistinct, pDest,
                      addrOutputRow+1, addr_समुच्चयः_Abort);
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Return, regOutputRow);
      VdbeComment((v, "end groupby result generator %d", p->selId));

      /* Generate a subroutine that will reset the group-by accumulator
      */
      sqlite3VdbeResolveLabel(v, addrReset);
      reset_सञ्चायकः(pParse, pAggInfo);
      sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 0, iUseFlag);
      VdbeComment((v, "indicate accumulator %d empty", p->selId));
      sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Return, regReset);

      यदि( distFlag!=0 && eDist!=WHERE_DISTINCT_NOOP ){
        संरचना AggInfo_func *pF = &pAggInfo->aFunc[0];
        fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
      }
    } /* endif pGroupBy.  Begin aggregate queries without GROUP BY: */
    अन्यथा {
      /* Aggregate functions without GROUP BY. tag-select-0820 */
      सारणी *pTab;
      यदि( (pTab = is_Simple_गणनम्(p, pAggInfo))!=0 ){
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
        ध्रुव पूर्णाङ्क iDb = sqlite3_Schema_गन्तव्यम्_सूचकांक(pParse->db, pTab->pSchema);
        ध्रुव पूर्णाङ्क iCsr = pParse->nTab++;     /* Cursor to scan b-tree */
        सूचकांक *pIdx;                         /* Iterator variable */
        कुञ्चिका_Info *p_कुञ्चिका_Info = 0;               /* Keyinfo for scanned index */
        सूचकांक *pBest = 0;                    /* Best index found so far */
        Pgno i_मूलम् = pTab->tnum;             /* Root page of scanned b-tree */

        sqlite3_संहिता_Verify_Schema(pParse, iDb);
        sqlite3_सारणी_तालकम्(pParse, iDb, pTab->tnum, 0, pTab->zName);

        /* Search for the index that has the lowest scan cost.
        **
        ** (2011-04-15) Do not do a full scan of an unordered index.
        **
        ** (2013-10-03) Do not count the entries in a partial index.
        **
        ** In practice the KeyInfo structure will not be used. It is only
        ** passed to keep OP_OpenRead happy.
        */
        यदि( !HasRowid(pTab) ) pBest = sqlite3_Primary_कुञ्चिका_सूचकांक(pTab);
        यदि( !p->pSrc->a[0].fg.not_अनुक्रमितम् ){
          प्रति(pIdx=pTab->p_सूचकांक; pIdx; pIdx=pIdx->p_अग्र){
            यदि( pIdx->bUnordered==0
             && pIdx->szIdxRow<pTab->szTabRow
             && pIdx->pPartIdxWhere==0
             && (!pBest || pIdx->szIdxRow<pBest->szIdxRow)
            ){
              pBest = pIdx;
            }
          }
        }
        यदि( pBest ){
          i_मूलम् = pBest->tnum;
          p_कुञ्चिका_Info = sqlite3_कुञ्चिका_Info_सम्बन्धि_सूचकांक(pParse, pBest);
        }

        /* Open a read-only cursor, execute the OP_Count, close the cursor. */
        sqlite3_Vdbe_सङ्कलय_Op4_Int(v, OP_OpenRead, iCsr, (पूर्णाङ्क)i_मूलम्, iDb, 1);
        यदि( p_कुञ्चिका_Info ){
          sqlite3VdbeChangeP4(v, -1, (वर्ण *)p_कुञ्चिका_Info, P4_KEYINFO);
        }
        नियोजय_Aggregate_Registers(pParse, pAggInfo);
        sqlite3_Vdbe_सङ्कलय_Op2(v, OP_गणनम्, iCsr, AggInfoFuncReg(pAggInfo,0));
        sqlite3_Vdbe_सङ्कलय_Op1(v, OP_Close, iCsr);
        explain_Simple_गणनम्(pParse, pTab, pBest);
      }अन्यथा{
        /* The general case of an aggregate query without GROUP BY
        ** tag-select-0822 */
        पूर्णाङ्क regAcc = 0;           /* "populate accumulators" flag */
        व्यञ्जकम्_List *pDistinct = 0;
        u16 distFlag = 0;
        पूर्णाङ्क eDist;

        /* If there are accumulator registers but no min() or max() functions
        ** without FILTER clauses, allocate register regAcc. Register regAcc
        ** will contain 0 the first time the inner loop runs, and 1 thereafter.
        ** The code generated by updateAccumulator() uses this to ensure
        ** that the accumulator registers are (a) updated only once if
        ** there are no min() or max functions or (b) always updated for the
        ** first row visited by the aggregate, so that they are updated at
        ** least once even if the FILTER clause means the min() or max()
        ** function visits zero rows.  */
        यदि( pAggInfo->n_सञ्चायकः ){
          प्रति(i=0; i<pAggInfo->nFunc; i++){
            यदि( व्यञ्जकम्_Has_Property(pAggInfo->aFunc[i].pFExpr, EP_WinFunc) ){
              अनुवर्त;
            }
            यदि( pAggInfo->aFunc[i].pFunc->funcFlags&SQLITE_FUNC_NEEDCOLL ){
              भङ्ग;
            }
          }
          यदि( i==pAggInfo->nFunc ){
            regAcc = ++pParse->nMem;
            sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 0, regAcc);
          }
        }अन्यथा यदि( pAggInfo->nFunc==1 && pAggInfo->aFunc[0].iDistinct>=0 ){
          प्रतिज्ञा( व्यञ्जकम्_Use_XList(pAggInfo->aFunc[0].pFExpr) );
          pDistinct = pAggInfo->aFunc[0].pFExpr->x.pList;
          distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
        }
        नियोजय_Aggregate_Registers(pParse, pAggInfo);

        /* This case runs if the aggregate has no GROUP BY clause.  The
        ** processing is much simpler since there is only a single row
        ** of output.
        */
        प्रतिज्ञा( p->p_गणः_By==0 );
        reset_सञ्चायकः(pParse, pAggInfo);

        /* If this query is a candidate for the min/max optimization, then
        ** minMaxFlag will have been previously set to either
        ** WHERE_ORDERBY_MIN or WHERE_ORDERBY_MAX and pMinMaxOrderBy will
        ** be an appropriate ORDER BY expression for the optimization.
        */
        प्रतिज्ञा( minMaxFlag==WHERE_ORDERBY_प्रसामान्यम् || p_Min_Max_क्रमः_By!=0 );
        प्रतिज्ञा( p_Min_Max_क्रमः_By==0 || p_Min_Max_क्रमः_By->n_व्यञ्जकम्==1 );

        TREETRACE(0x2,pParse,p,("WhereBegin\n"));
        pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, p_Min_Max_क्रमः_By,
                                   pDistinct, p, minMaxFlag|distFlag, 0);
        यदि( pWInfo==0 ){
          गच्छ select_अन्तः;
        }
        TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
        eDist = sqlite3WhereIsDistinct(pWInfo);
        update_सञ्चायकः(pParse, regAcc, pAggInfo, eDist);
        यदि( eDist!=WHERE_DISTINCT_NOOP ){
          संरचना AggInfo_func *pF = pAggInfo->aFunc;
          यदि( pF ){
            fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
          }
        }

        यदि( regAcc ) sqlite3_Vdbe_सङ्कलय_Op2(v, OP_Integer, 1, regAcc);
        यदि( minMaxFlag ){
          sqlite3_Where_Min_Max_Opt_Early_बहिर्गमनम्(v, pWInfo);
        }
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_अन्तः(pWInfo);
        finalizeAggFunctions(pParse, pAggInfo);
      }

      s_क्रमय.p_क्रमः_By = 0;
      sqlite3_व्यञ्जकम्_If_False(pParse, pHaving, addr_अन्तः, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, 0, 0,
                      pDest, addr_अन्तः, addr_अन्तः);
    }
    sqlite3VdbeResolveLabel(v, addr_अन्तः);

  } /* endif aggregate query */

  यदि( sDistinct.eTnctType==WHERE_DISTINCT_UNORDERED ){
    explain_अस्थायी_सारणी(pParse, "DISTINCT");
  }

  /* If there is an ORDER BY clause, then we need to sort the results
  ** and send them to the callback one by one.  tag-select-0900
  */
  यदि( s_क्रमय.p_क्रमः_By ){
    प्रतिज्ञा( p->pEList==pEList );
    generate_क्रमय_पुच्छ(pParse, p, &s_क्रमय, pEList->n_व्यञ्जकम्, pDest);
  }

  /* Jump here to skip this query
  */
  sqlite3VdbeResolveLabel(v, i_अन्तः);

  /* The SELECT has been coded. If there is an error in the Parse structure,
  ** set the return code to 1. Otherwise 0. */
  rc = (pParse->nErr>0);

  /* Control jumps to here if an error is encountered above, or upon
  ** successful coding of the SELECT.
  */
select_अन्तः:
  प्रतिज्ञा( db->mallocFailed==0 || db->mallocFailed==1 );
  प्रतिज्ञा( db->mallocFailed==0 || pParse->nErr!=0 );
  sqlite3_व्यञ्जकम्_List_Delete(db, p_Min_Max_क्रमः_By);
#यदि_परिभाषितम् SQLITE_DEBUG
  /* Internal self-checks.  tag-select-1000 */
  यदि( pAggInfo && !db->mallocFailed ){
#यदि TREETRACE_ENABLED
    यदि( sqlite3_वृक्षः_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("Finished with AggInfo\n"));
      printAggInfo(pAggInfo);
    }
#यदि_अन्तः
    प्रति(i=0; i<pAggInfo->nColumn; i++){
      व्यञ्जकम् *p_व्यञ्जकम् = pAggInfo->aCol[i].pCExpr;
      यदि( p_व्यञ्जकम्==0 ) अनुवर्त;
      प्रतिज्ञा( p_व्यञ्जकम्->pAggInfo==pAggInfo );
      प्रतिज्ञा( p_व्यञ्जकम्->iAgg==i );
    }
    प्रति(i=0; i<pAggInfo->nFunc; i++){
      व्यञ्जकम् *p_व्यञ्जकम् = pAggInfo->aFunc[i].pFExpr;
      प्रतिज्ञा( p_व्यञ्जकम्!=0 );
      प्रतिज्ञा( p_व्यञ्जकम्->pAggInfo==pAggInfo );
      प्रतिज्ञा( p_व्यञ्जकम्->iAgg==i );
    }
  }
#यदि_अन्तः

#यदि TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p,("end processing\n"));
  यदि( (sqlite3_वृक्षः_Trace & 0x40000)!=0 && ExplainQueryPlanParent(pParse)==0 ){
    sqlite3_वृक्षः_View_Select(0, p, 0);
  }
#यदि_अन्तः
  Explain_Query_Plan_निष्कासय(pParse);
  प्रत्यागम rc;
}
